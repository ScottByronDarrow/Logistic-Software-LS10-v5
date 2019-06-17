/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_odueprt.c,v 5.2 2001/08/09 09:15:56 scott Exp $
|  Program Name  : ( po_odueprt.c )                                   |
|  Program Desc  : ( Print Overdue Information.              	  )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, comr, inis, inmr, inum, pohr, poln          |
|                :  poli, posl, sumr, exsi,                           |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Basil Wood.     | Date Written  : 06/07/95         |
|---------------------------------------------------------------------|
|  Date Modified : (06/07/95)      | Created by  : Basil Wood         |
|  Date Modified : (18/08/95)      | Created by  : John Osborne       |
|  Date Modified : (12/03/96)      | Created by  : Anneliese Allen    |
|  Date Modified : (01/04/96)      | Created by  : Anneliese Allen    |
|  Date Modified : (11/11/1998)    | Created by  : Ronnel L. Amanca.  |
|  Date Modified : (24/08/1999)    | Modified by : Mars dela Cruz.    |
|                :                                                    |
|  Comments      :                                                    |
|  (06/07/95)    : ASL 12080. New program based on po_pofax           |
|  (18/08/95)    : ASL 12080. Text aligning.                          |
|  (12/03/96)    : ASL 12426. Contact person from sumr not pohr.      |
|  (01/04/96)    : ASL 12884. Exclude quantities already on shipments.|
|  (11/11/1998)  : ASL Modified to fit in standard version.           |
|                :                                                    |
| $Log: po_odueprt.c,v $
| Revision 5.2  2001/08/09 09:15:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:06  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:49  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:02  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:35  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:11  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:26  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/11/11 06:43:16  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.9  1999/11/05 05:17:14  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.8  1999/09/29 10:12:05  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 04:38:07  scott
| Updated from Ansi project
|
| Revision 1.6  1999/08/24 10:36:59  marlyn
| Resolved ASL issue SC#178 : When ETA date on the Purchase Order Note is less than the Date entered in Screen 1 in the field Overdue Date, then the Purchase Order Line should print.
|
| Revision 1.5  1999/06/17 10:06:32  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_odueprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_odueprt/po_odueprt.c,v 5.2 2001/08/09 09:15:56 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>

#define	PAGE_LEN		66
#define	HEAD_PAGE1		15
#define	HEAD_PAGE2		8
#define	PAGE1_LINES		40
#define	PAGE2_LINES		45

/*====================
| RECORD STRUCTURES. |
====================*/

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct poliRecord	poli_rec;
struct poslRecord	posl_rec;
struct exsiRecord	exsi_rec;

	char	*data	= "data";

/*===================
| GLOBAL VARIABLES. |
===================*/
	int		pageNumber		=	1,
			lineNumber		=	0,
			printerNumber	=	1,
			companyOwned	=	0,
			creditorsFind	=	0,
			automaticPrint	=	FALSE,
			totalPages		=	0;

	char	branchNumber[3];
	char	uomDesc[5];

	long	hhsuHash;

	float	stdCnvFct		=	0.00,
			purCnvFct		=	0.00,
			cnvFct			=	0.00;

	FILE	*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sysDate [11];
	char	sup_no [7];
	char	sup_name [41];
	Date	overdue_date;
	float	qtyOnShip;
	char	text [3][61];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "sup_no",	 4, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",	"Supplier No   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.sup_no},
	{1, LIN, "name",	 5, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",	"Supplier Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "overdue_date", 6, 16, EDATETYPE,
        "DD/DD/DD", "          ",
        " ", local_rec.sysDate, "Overdue Date  ", "Enter date used to determine if P/O is overdue",
        YES, NO, JUSTLEFT, "", "", (char *) &local_rec.overdue_date},
	{1, LIN, "text1",	 7, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",	"Text Line 1   ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.text[0]},
	{1, LIN, "text2",	 8, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",	"     Line 2   ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.text[1]},
	{1, LIN, "text3",	 9, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",	"     Line 3   ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.text[2]},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
float 	GetShipQty 			(long);
int 	FindInis 			(long);
int 	IsOutstandingLine 	(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CalcNumPages 		(void);
void 	CloseDB 			(void);
void 	HeadOutput 			(void);
void 	OpenDB 				(void);
void 	PageBreak 			(void);
void 	PageTrailer 		(int);
void 	PrintPoln 			(void);
void 	ProcessFile 		(void);
void 	ReadComr 			(void);
void 	RunReport 			(void);
void 	SubHeading 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	char *sptr;

	sptr = strrchr (argv[0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv[0];

	if (argc < 2)
	{
		print_at (0,0,"Usage : <LPNO> Optional [hhsuHash]");
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv[1]);

	automaticPrint = FALSE;
	if (argc == 3)
	{
		hhsuHash = atol (argv[2]);
		automaticPrint = TRUE;
	}
	companyOwned 	= atoi (get_env ("CR_CO"));
	creditorsFind 	= atoi (get_env ("CR_FIND"));

	strcpy (local_rec.sysDate, DateToString (TodaysDate()));

	OpenDB ();

	strcpy (branchNumber, (companyOwned) ? comm_rec.est_no : " 0");

	if (!automaticPrint)
	{
		SETUP_SCR (vars);

		/*---------------------------
		| Setup required parameters |
		---------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();
		init_vars (1);

		prog_exit = FALSE;
		while (!prog_exit)
		{
			/*---------------------
			| Reset control flags |
			---------------------*/
			entry_exit	=	FALSE;
			edit_exit	=	FALSE;
			restart		=	FALSE;
			search_ok	=	TRUE;
			init_vars (1);

			/*-----------------------------
			| Entry screen 1 linear input |
			-----------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;

			/*----------------------------
			| Edit screen 1 linear input |
			----------------------------*/
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
			RunReport ();
		}
	}
	else
	{
		/*--------------------
		| Validate hhpo hash |
		--------------------*/
		if (!find_hash (sumr, &sumr_rec, COMPARISON, "r", hhsuHash))
		{
			RunReport ();
		}
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadComr ();

	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");

	if (automaticPrint)
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	else
	{
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS,
				  (!creditorsFind) ? "sumr_id_no" : "sumr_id_no3");
	}
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poli);
	abc_fclose (posl);
	abc_fclose (exsi);
	abc_fclose (sumr);
	abc_dbclose (data);
}

void
ReadComr (
 void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in comr During (DBFIND)", cc, PNAME);
	abc_fclose (comr);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if (LCHECK ("sup_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.sup_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("name");

		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Overview At Date
	---------------------------*/
	if (LCHECK("overdue_date"))
	{
		return (local_rec.overdue_date <= 0L);
	}

	return (EXIT_SUCCESS);
}

void
RunReport (
 void)
{
	dsp_screen (ML ("Printing Overdue Purchase Order Items"),
				comm_rec.co_no,
				comm_rec.co_name);

	CalcNumPages ();

	HeadOutput ();
	ProcessFile ();

	PageTrailer (TRUE);

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*==============================================
| Return total shipment qty less received qty  |
==============================================*/
float
GetShipQty (
 long hhpl_hash)
{
	float	qty = 0.00;

	cc = find_hash (posl, &posl_rec, GTEQ, "r", hhpl_hash);
	while (!cc && posl_rec.hhpl_hash == hhpl_hash)
	{
		if (posl_rec.ship_qty == 0.00)
			qty += 0.00;
		else
			qty += (posl_rec.ship_qty - posl_rec.rec_qty);

		cc = find_hash (posl, &posl_rec, NEXT, "r", hhpl_hash);
	}
	return qty;
}

/*=============================================
| Returns TRUE if poln is outstanding 
|     a) an outstanding qty on order
| and b) outstanding order is not on a shipment
| and c) there is no ETA in poli file
=============================================*/
int
IsOutstandingLine (
 void)
{
	float qty = poln_rec.qty_ord - poln_rec.qty_rec;

	/*-----------------------
	| Check qty outstanding 
	-----------------------*/
	if (qty <= 0.0)
		return FALSE;	/* No qty outstanding */
	
	/*-----------------------
	| Check no ETA
	-----------------------*/
	cc = find_hash (poli, &poli_rec, EQUAL, "r", poln_rec.hhpl_hash);
	if (!cc && poli_rec.eta_date >= local_rec.overdue_date) 
	{
		return FALSE;	/* ETA exists */
    }

	/*-----------------------
	| Check not on a shipment
	-----------------------*/
	local_rec.qtyOnShip = GetShipQty(poln_rec.hhpl_hash);
	if (qty <= local_rec.qtyOnShip)
    {
       return FALSE;	/* Is on a shipment */
    }
	return TRUE;
}

/*================================
| Calculate number of pages that |
| will be generated by report.   |
================================*/
void
CalcNumPages (
 void)
{
	int cntLines = 0;
	int pohr_printed;

	memset (&pohr_rec, 0, sizeof pohr_rec);
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.pur_ord_no, "               ");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		   pohr_rec.hhsu_hash == sumr_rec.hhsu_hash &&
		   *pohr_rec.type == 'O')
	{
		if (*pohr_rec.status != 'D' &&
			pohr_rec.due_date <= local_rec.overdue_date)
		{
			pohr_printed = FALSE;
			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			poln_rec.line_no = 0;

			cc = find_rec (poln, &poln_rec, GTEQ, "r");
			while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
			{
				if (IsOutstandingLine())
				{
					if (!pohr_printed)
					{
						cntLines++;
						pohr_printed = TRUE;
					}
					cntLines++;
				}

				cc = find_rec (poln, &poln_rec, NEXT, "r");
			}
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}

	if (cntLines <= PAGE1_LINES)
		totalPages = 1;
	else
	{
		totalPages = 1;
		cntLines -= PAGE1_LINES;

		totalPages += (cntLines / PAGE2_LINES);
		if ((cntLines % PAGE2_LINES) != 0)
			totalPages++;
	}
}

/*===================================
| Start Output To Standard Print
===================================*/
void
HeadOutput (
 void)
{
	pageNumber = 1;

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".NC1\n");

	fprintf (fout, ".5\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".PL0\n");
	fprintf (fout, ".OP\n");
	fprintf (fout, ".L80\n");

	/*----------------------------
	| HEADING - first page only. |
	----------------------------*/
	fprintf (fout, "%s:   %-40.40s       ",	ML ("TO"),	 	sumr_rec.crd_name);
	fprintf (fout, "%s:   %-14.14s\n",  	ML ("FAX NO"),	sumr_rec.fax_no);

	if (strlen (clip (sumr_rec.cont_name)) == 0)
		fprintf (fout, "%-53.53s", " ");
	else
		fprintf (fout, "%s: %-20.20s%27.27s", ML ("ATTN"),sumr_rec.cont_name, " ");
	fprintf (fout, "%s:     %-10.10s\n", ML ("DATE"), local_rec.sysDate);
	fprintf (fout, ".B1\n");

	/*------------------------------------------
	| SUBHEADING - first and subsequent pages. |
	------------------------------------------*/
	SubHeading ();

	lineNumber = HEAD_PAGE1;
}

/*====================================
| Process Purchase order line items. |
====================================*/
void
ProcessFile (
 void)
{
	int pohr_printed;

	memset (&pohr_rec, 0, sizeof pohr_rec);
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		   pohr_rec.hhsu_hash == sumr_rec.hhsu_hash &&
		   *pohr_rec.type == 'O')
	{
		if (*pohr_rec.status != 'D' &&
			pohr_rec.due_date <= local_rec.overdue_date)
		{
			pohr_printed = FALSE;
			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			poln_rec.line_no = 0;

			cc = find_rec (poln, &poln_rec, GTEQ, "r");
			while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
			{
				if (IsOutstandingLine())
				{
					if (!pohr_printed)
					{
						PageBreak ();
						fprintf (fout, "|%s %-15.15s", 
											ML ("Purchase Order No"),
											pohr_rec.pur_ord_no);
						fprintf (fout, "|%8.8s", 	  " ");
						fprintf (fout, "|%4.4s", 	  " ");
						fprintf (fout, "|%30.30s|\n", " ");
						lineNumber++;
						pohr_printed = TRUE;
					}
					PageBreak ();
					PrintPoln ();
					lineNumber++;
				}
				cc = find_rec (poln, &poln_rec, NEXT, "r");
			}
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
}

/*=================================
| Find inventory Supplier Record. |
=================================*/
int
FindInis (
 long	hhbrHash)
{
	inis_rec.hhbr_hash = hhbrHash;
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");

	if (cc || inis_rec.hhbr_hash != hhbrHash || 
			  inis_rec.hhsu_hash != sumr_rec.hhsu_hash)
	{
		sprintf (inis_rec.sup_part, "%-16.16s", " ");
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Print Purchase Order Line items. |
==================================*/
void
PrintPoln (
 void)
{
	float	printQuantity	=	0.00;

	inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inmr", "DBFIND");

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (uomDesc, "%-4.4s", "    ");
		stdCnvFct = 1.00;
	}
	else
	{
		sprintf (uomDesc, "%-4.4s", inum_rec.uom);
		stdCnvFct = inum_rec.cnv_fct;
	}

	cc = FindInis (poln_rec.hhbr_hash);
	if (cc)
		purCnvFct = 1.00;
	else
	{
		inum_rec.hhum_hash	=	inis_rec.sup_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (uomDesc, "%-4.4s", inum_rec.uom);
			purCnvFct = inum_rec.cnv_fct;
		}
		else
			purCnvFct = 1.00;
	}
	cnvFct	= stdCnvFct / purCnvFct;
	if (poli_rec.hhpl_hash != poln_rec.hhpl_hash)
	{
		memset (&poli_rec, 0, sizeof poli_rec);
		poli_rec.hhpl_hash	=	poln_rec.hhpl_hash;
		cc = find_rec (poli, &poli_rec, COMPARISON, "r");
	}

	memset (&exsi_rec, 0, sizeof exsi_rec);
	if (poli_rec.inst_code)
	{
		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = poli_rec.inst_code;
		cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	}

	printQuantity	=	poln_rec.qty_ord - 
						poln_rec.qty_rec -
						local_rec.qtyOnShip;

	fprintf (fout, "|%16.16s", inis_rec.sup_part);
	fprintf (fout, "|%16.16s", inmr_rec.item_no);
	fprintf (fout, "|%8.2f",   printQuantity * cnvFct);
	fprintf (fout, "|%-4.4s", uomDesc);
	fprintf (fout, "|%-30.30s|\n", exsi_rec.inst_text);
}

void
PageBreak (
 void)
{
	int linesInTab;

	if (pageNumber == 1)
		linesInTab = lineNumber - HEAD_PAGE1 + 1;
	else
		linesInTab = lineNumber - HEAD_PAGE2 + 1;

	if ((pageNumber == 1 && linesInTab > PAGE1_LINES) ||
		(pageNumber != 1 && linesInTab > PAGE2_LINES))
	{
		/*---------------------
		| Print page trailer. |
		---------------------*/
		PageTrailer (FALSE);

		pageNumber++;

		/*-------------------------
		| Print subheading again. |
		-------------------------*/
		SubHeading ();
		lineNumber = HEAD_PAGE2;
	}
}

/*====================
| Print sub-heading. |
====================*/
void
SubHeading (
 void)
{
	fprintf (fout,
			 "FROM: %-40.40s       PAGE %4d OF %4d\n",
			 comr_rec.co_name,
			 pageNumber,
			 totalPages);

	fprintf (fout, "      %-40.40s\n", comr_rec.co_adr1);
	fprintf (fout, "      %-40.40s\n", comr_rec.co_adr2);
	fprintf (fout, "      %-40.40s\n", comr_rec.co_adr3);
	fprintf (fout, ".B1\n");

	if (pageNumber == 1)
	{
		fprintf (fout, "      %-60s\n", local_rec.text[0]);
		fprintf (fout, "      %-60s\n", local_rec.text[1]);
		fprintf (fout, "      %-60s\n", local_rec.text[2]);
		fprintf (fout, ".B1\n");
	}

	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=========");
	fprintf (fout, "=====");
	fprintf (fout, "================================\n");

	fprintf (fout, "|%s", 			ML ("SUPPLIER PART NO"));
	fprintf (fout, "|  %11.11s   ", ML ("ITEM NUMBER"));
	fprintf (fout, "|%8.8s",		ML ("QUANTITY"));
	fprintf (fout, "|%4.4s",		ML ("UOM "));
	fprintf (fout, "|                              |\n");

	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----");
	fprintf (fout, "|------------------------------|\n");
}

void
PageTrailer (
 int lastPage)
{
	int trailerLines;

	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=========");
	fprintf (fout, "=====");
	fprintf (fout, "================================\n");
	lineNumber++;

	if (lastPage)
	{
		fprintf (fout, ".B1\n");
		fprintf (fout, ".C***** %s *****\n", 
					ML ("PLEASE CONFIRM RECEIPT OF THIS FAX BY RETURN FAX"));
	}
	else
	{
		trailerLines = PAGE_LEN - lineNumber;
		fprintf (fout, ".B%d\n", trailerLines);
	}
}

int
heading (
 int scn)
{
	if (!restart)
	{
		clear ();
		centre_at (0, 80, ML ("%R Print Overdue Purchase Order Items "));
		move (0, 1);
		line (80);

		box (0, 3, 80, 6);

		move (0, 20);
		line (80);
		move (0, 21);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
