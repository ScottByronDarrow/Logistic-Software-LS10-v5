/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_lists.c,v 5.2 2001/08/09 08:52:02 scott Exp $
|  Program Name  : (cr_lists.c) 
|  Program Desc  : (Suppliers customer listings)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 27/01/87          |
|---------------------------------------------------------------------|
| $Log: cr_lists.c,v $
| Revision 5.2  2001/08/09 08:52:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:32  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_lists.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_lists/cr_lists.c,v 5.2 2001/08/09 08:52:02 scott Exp $";

#include    <ml_std_mess.h>
#include    <ml_cr_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;


/*
 * Table names
 */
static char
	*data	= "data";

/*
 * Globals
 */
	char	branchNumber [3];

	static struct
	{
		int	lpno;

		char	*fullShort,
				*listBy,
				*sortBy,
				*payType,
				*begStr,
				*endStr;

	} args;

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB			 (void);
FILE *	InitPrintOut	 (void);
char *	FmtAddr			 (char *, char *, char *);
void	ProcessFile		 (FILE *);
void	EndPrintOut		 (FILE *);
int		ListByCompany	 (void);
int		SortByNumber	 (void);
int		FullListing		 (void);


/*==============
 The real thing!
================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		envCrCo;

	/*===========
	 Handle args
	=============*/
	if (argc != 8)
	{
		print_at (0,0,mlCrMess071, argv [0]);
		return (EXIT_FAILURE);
	}

	args.lpno		= atoi (argv [1]);
	args.fullShort	= argv [2];
	args.listBy		= argv [3];
	args.sortBy		= argv [4];
	args.begStr		= argv [5];
	args.endStr		= argv [6];
	args.payType	= argv [7];

	/*
	 * Look at env
	 */
	envCrCo = atoi (get_env ("CR_CO"));

	/*
	 * Init db
	 */
	OpenDB ();

	strcpy (branchNumber, envCrCo ? comm_rec.est_no : " 0");

	dsp_screen ("Printing Supplier Listings.",
				comm_rec.co_no, comm_rec.co_name);

	memset (&sumr_rec, 0, sizeof (sumr_rec));
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no,
		 (!envCrCo || !ListByCompany ()) ? branchNumber : " 0");
	strcpy (SortByNumber () ? sumr_rec.crd_no : sumr_rec.acronym,
		args.begStr);

	if (! (cc = find_rec (sumr, (char *) &sumr_rec, GTEQ, "r")))
	{
		FILE	*pOut = NULL;

		while (!cc &&
			!strcmp (sumr_rec.co_no, comm_rec.co_no) &&
			 (ListByCompany () || !strcmp (sumr_rec.est_no, branchNumber)))
		{
			if (strcmp (SortByNumber () ? sumr_rec.crd_no : sumr_rec.acronym, args.endStr) > 0)
				break;

			if (*args.payType == 'A' ||
				*sumr_rec.pay_method == *args.payType)
			{
				dsp_process ("Supplier : ", sumr_rec.crd_no);

				if (!pOut)
					pOut = InitPrintOut ();
				ProcessFile (pOut);
			}

			cc = find_rec (sumr, (char *) &sumr_rec, NEXT, "r");
		}

		EndPrintOut (pOut);
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS,
		SortByNumber () ? "sumr_id_no" : "sumr_id_no2");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_dbclose (data);
}

FILE *
InitPrintOut (
 void)
{
	int	lineSize = FullListing () ? 151 : 142;

	FILE *	pOut = popen ("pformat", "w");
	
	char *	payMesg;

	/*----------------------
	| Open pipe to pformat |
 	----------------------*/
	if (!pOut)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/

	fprintf (pOut, ".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (pOut, ".LP%d\n", args.lpno);
	fprintf (pOut, ".%d\n", FullListing () ? 17 : 16);
	fprintf (pOut, ".PI12\n");
	fprintf (pOut, ".L%d\n", lineSize);
	fprintf (pOut, ".ESUPPLIER LISTING BY %s\n",
		ListByCompany () ? "COMPANY" : "BRANCH");
	fprintf (pOut, ".B1\n");
	fprintf (pOut, ".E%s\n", comm_rec.co_name);
	fprintf (pOut, ".B1\n");

	fprintf (pOut, ".CAnalysis Listing (%s Name And Address Listing)\n",
		FullListing () ? "Full" : "Short");
	fprintf (pOut, ".CSorted By Supplier %s ",
		SortByNumber () ? "Number" : "Acronym");
	fprintf (pOut, "From '%s' to '%s'\n", args.begStr, args.endStr);

	switch (*args.payType)
	{
	case 'C'	:
		payMesg = "Cheque";
		break;
	case 'D'	:
		payMesg = "Draft";
		break;
	case 'T'	:
		payMesg = "Transfer";
		break;
	default	:
		payMesg = "All";
	}
	fprintf (pOut, ".CPayment Method : %s\n", payMesg);

	fprintf (pOut, ".B1\n");
	fprintf (pOut, ".E");
	if (!ListByCompany ())
		fprintf (pOut, "%s ", clip (comm_rec.est_name));
	fprintf (pOut, "AS AT : %s\n", SystemTime ());

	if (FullListing ())
	{
		fprintf (pOut, "==========");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "====================");
		fprintf (pOut, "=====");
		fprintf (pOut, "=====================");
		fprintf (pOut, "=======");
		fprintf (pOut, "======");
		fprintf (pOut, "=====");
		fprintf (pOut, "==================");
		fprintf (pOut, "==================\n");

		fprintf (pOut, ".R==========");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "====================");
		fprintf (pOut, "=====");
		fprintf (pOut, "=====================");
		fprintf (pOut, "=======");
		fprintf (pOut, "======");
		fprintf (pOut, "=====");
		fprintf (pOut, "==================");
		fprintf (pOut, "==================\n");

		fprintf (pOut, "|SUPPLIER ");
		fprintf (pOut, "|       C R E D I T O R   N A M E        ");
		fprintf (pOut, "                    ");
		fprintf (pOut, "| ACC");
		fprintf (pOut, "|      CONTACT       ");
		fprintf (pOut, "|PAYMT ");
		fprintf (pOut, "|CURR/");
		fprintf (pOut, "|PYT/");
		fprintf (pOut, "|   PHONE  NO. /  ");
		fprintf (pOut, "|    CONTROL     |\n");

		fprintf (pOut, "| NUMBER /");
		fprintf (pOut, "|       A D D R E S S                    ");
		fprintf (pOut, "                    ");
		fprintf (pOut, "|TYPE");
		fprintf (pOut, "|        NAME        ");
		fprintf (pOut, "|TERMS ");
		fprintf (pOut, "|CTRY ");
		fprintf (pOut, "|H/P ");
		fprintf (pOut, "|   FAX NO.       ");
		fprintf (pOut, "|    ACCOUNT     |\n");

		fprintf (pOut, "| ACRONYM ");
		fprintf (pOut, "|                                        ");
		fprintf (pOut, "                    ");
		fprintf (pOut, "|    ");
		fprintf (pOut, "|                    ");
		fprintf (pOut, "|      ");
		fprintf (pOut, "|CODE ");
		fprintf (pOut, "|    ");
		fprintf (pOut, "|                 ");
		fprintf (pOut, "|                |\n");

		fprintf (pOut, "|---------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "--------------------");
		fprintf (pOut, "+----");
		fprintf (pOut, "+--------------------");
		fprintf (pOut, "+------");
		fprintf (pOut, "+-----");
		fprintf (pOut, "+----");
		fprintf (pOut, "+-----------------");
		fprintf (pOut, "+----------------|\n");
	} 
	else
	{
		fprintf (pOut, "=========");
		fprintf (pOut, "==========");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "=========================================\n");

		fprintf (pOut, ".R=========");
		fprintf (pOut, "==========");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "=========================================");
		fprintf (pOut, "=========================================\n");

		fprintf (pOut, "|SUPPLIER");
		fprintf (pOut, "|SUPPLIER ");
		fprintf (pOut, "|       S U P P L I E R   N A M E        ");
		fprintf (pOut, "|                     A D D R E S S  D E ");
		fprintf (pOut, "T A I L S                               |\n");

		fprintf (pOut, "| NUMBER ");
		fprintf (pOut, "| ACRONYM ");
		fprintf (pOut, "|                                        ");
		fprintf (pOut, "|                                        ");
		fprintf (pOut, "                                        |\n");

		fprintf (pOut, "|--------");
		fprintf (pOut, "+---------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "----------------------------------------|\n");
	}

	return (pOut);
}

/*=======================================
 Format addr (adding ',' when necessary)
=========================================*/
char *
FmtAddr (
 char *	buf,
 char *	adr1,
 char *	adr2)
{
	strcpy (buf, clip (adr1));
	if (*clip (adr2) && *buf)
		strcat (buf, ", ");

	return (strcat (buf, adr2));
}

/*===========================
| Validate and print lines. |
===========================*/
void
ProcessFile (
 FILE *	pOut)
{
	char	wk_desc [200];

	/*--------------------------------------
	|  Analysis (Full Master file listing) |
	--------------------------------------*/
	if (FullListing ())
	{
		fprintf (pOut, "| %-6.6s  ", sumr_rec.crd_no);
		fprintf (pOut, "|%-60.60s", sumr_rec.crd_name);

		fprintf (pOut, "|%s", sumr_rec.acc_type [0] == 'O' ? "OPEN" : "B/F.");

		fprintf (pOut, "|%-20.20s", sumr_rec.cont_name);
		fprintf (pOut, "| %-3.3s  ", sumr_rec.pay_terms);
		fprintf (pOut, "| %-3.3s ", sumr_rec.curr_code);
		fprintf (pOut, "|%-3.3s ", sumr_rec.pay_method);
		fprintf (pOut, "| %-15.15s ", sumr_rec.cont_no);
		fprintf (pOut, "|%-16.16s|\n", sumr_rec.gl_ctrl_acct);

		fprintf (pOut, "|%-9.9s", sumr_rec.acronym);

		fprintf (pOut, "|%-86.86s",
			FmtAddr (wk_desc, sumr_rec.adr1, sumr_rec.adr2));
		fprintf (pOut, "|      ");
		fprintf (pOut, "| %-3.3s ", sumr_rec.ctry_code);
		fprintf (pOut, "|%-3.3s ", sumr_rec.hold_payment);
		fprintf (pOut, "| %-14.14s  ", sumr_rec.fax_no);
		fprintf (pOut, "|%-16.16s|\n", " ");

		fprintf (pOut, "|         ");
		fprintf (pOut, "|%-86.86s",
			FmtAddr (wk_desc, sumr_rec.adr3, sumr_rec.adr4));
		fprintf (pOut, "|      ");
		fprintf (pOut, "|     ");
		fprintf (pOut, "|    ");
		fprintf (pOut, "|                 ");
		fprintf (pOut, "|                |\n");

		fprintf (pOut, "|---------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "--------------------");
		fprintf (pOut, "+----");
		fprintf (pOut, "+--------------------");
		fprintf (pOut, "+------");
		fprintf (pOut, "+-----");
		fprintf (pOut, "+----");
		fprintf (pOut, "+-----------------");
		fprintf (pOut, "+----------------|\n");
	}

	/*-------------------------------
	| This is for the rest of them. |
	-------------------------------*/
	else
	{
		fprintf (pOut, "| %-6.6s ", sumr_rec.crd_no);
		fprintf (pOut, "|%-9.9s", sumr_rec.acronym);
		fprintf (pOut, "|%40.40s", sumr_rec.crd_name);

		fprintf (pOut, "|%-80.80s|\n",
			FmtAddr (wk_desc, sumr_rec.adr1, sumr_rec.adr2));

		fprintf (pOut, "|        ");
		fprintf (pOut, "|         ");
		fprintf (pOut, "|%40.40s", " ");

		fprintf (pOut, "|%-80.80s|\n",
			FmtAddr (wk_desc, sumr_rec.adr3, sumr_rec.adr4));

		fprintf (pOut, "|--------");
		fprintf (pOut, "+---------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "+----------------------------------------");
		fprintf (pOut, "----------------------------------------|\n");
	}
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndPrintOut (
 FILE *	pOut)
{
	if (pOut)
	{
		fprintf (pOut, ".EOF\n");
		pclose (pOut);
	}
}

/*
 * Support utilities
 */
int
ListByCompany (void)
{
	return (*args.listBy == 'C');
}

int
SortByNumber (void)
{
	return (*args.sortBy == 'N');
}

int
FullListing (void)
{
	return (*args.fullShort == 'F');
}
