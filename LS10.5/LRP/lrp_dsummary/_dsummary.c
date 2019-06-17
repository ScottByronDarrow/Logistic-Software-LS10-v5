/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _dsummary.c,v 5.4 2002/11/25 03:16:36 scott Exp $
|  Program Name  : (lrp_dsummary.c)                                   |
|  Program Desc  : (Print Forecast Summary Report               )     |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _dsummary.c,v $
| Revision 5.4  2002/11/25 03:16:36  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.3  2001/08/28 08:46:14  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 09:29:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:25  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/23 22:13:19  scott
| Updated to use app.schema
|
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _dsummary.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_dsummary/_dsummary.c,v 5.4 2002/11/25 03:16:36 scott Exp $";

#include	<pslscr.h>
#include	<ml_lrp_mess.h>
#include	<ml_std_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<pr_format3.h>

#define	MAX_METHODS	26
#define	MAX_CCMR	100

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;


	int		byCompany = FALSE,
			byBranch = FALSE,
			byWarehouse = FALSE,
			printerNumber;

	char	validMethods [27];

	long	numberItems = 0L;
	long	hhccHash [MAX_CCMR],
			store [3] [MAX_METHODS];

	FILE	*fin,
			*fout;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	check_page 			(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	ProcessFile 		(void);
void 	Summarise 			(long);
void 	PrintOption 		(char *);
void 	LoadHhcc 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int	i;
	char	*sptr;

	if (argc != 2)
	{
		print_at (0,0,"Usage : %s <printerNumber>\007\n\r",argv [0]);
		return (EXIT_FAILURE);
	}

	if (!strncmp (argv [0],"lrp_cdsu",7))
		byCompany = TRUE;

	if (!strncmp (argv [0],"lrp_bdsu",7))
		byBranch = TRUE;

	if (!strncmp (argv [0],"lrp_dsum",7))
		byWarehouse = TRUE;

	sptr = chk_env ("LRP_METHODS");
	if (sptr == (char *) NULL)
	{
		print_at (0,0,ML ("No LRP_METHODS defined in Enironment."));
		return (EXIT_FAILURE);
	}

	sprintf (validMethods,"%.26s",sptr);

	for (i = 0;i < MAX_METHODS;i++)
	{
		store [0] [i] = 0L;
		store [1] [i] = 0L;
		store [2] [i] = 0L;
	}

	/*-------------------
	| Printer Number	|
	-------------------*/
	printerNumber = atoi (argv [1]);

	init_scr ();

	OpenDB ();

	dsp_screen ("Processing : LRP - Forecast Summary Report.", comm_rec.co_no, comm_rec.co_name);

	if ((fin = pr_open ("lrp_dsummary.p")) == NULL)
		file_err (errno, "lrp_dsummary.p", "PR_OPEN");

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat.p", "POPEN");
	
	HeadingOutput ();

	ProcessFile ();

	fprintf (fout, ".EOF\n");
	pclose (fout);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s\n",DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);

	fprintf (fout, ".%d\n", (byCompany) ? 11 : 13);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".B1\n");
	if (byCompany)
		fprintf (fout, ".EFORECAST SUMMARY REPORT BY COMPANY\n");
	if (byBranch)
		fprintf (fout, ".EFORECAST SUMMARY REPORT BY BRANCH\n");
	if (byWarehouse)
		fprintf (fout, ".EFORECAST SUMMARY REPORT BY WAREHOUSE\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	if (!byCompany)
	{
		fprintf (fout, ".B1\n");
		if (byBranch)
			fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
		else
			fprintf (fout, ".E%s-%s\n", clip (comm_rec.est_name),
						   clip (comm_rec.cc_name));
	}
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n",SystemTime ());

	pr_format (fin,fout,"SUM_RULER",0,0);
	pr_format (fin,fout,"SUM_HEAD0",0,0);
	pr_format (fin,fout,"SUM_HEAD1",0,0);
	fflush (fout);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

void
ProcessFile (void)
{
	LoadHhcc ();

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		dsp_process ("Item No : ",inmr_rec.item_no);
		Summarise (inmr_rec.hhbr_hash);
		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
	PrintOption ("A");
	PrintOption ("M");
	PrintOption ("P");
}

void
Summarise (
 long   hhbr_hash)
{
	int	indx;
	char	*sptr;

	for (indx = 0;indx < MAX_CCMR && hhccHash [indx] != 0L;indx++)
	{
		incc_rec.hhcc_hash = hhccHash [indx];
		incc_rec.hhbr_hash = hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");

		/*---------------------------
		| Item Exists in Warehouse	|
		---------------------------*/
		if (!cc)
		{
			sptr = strchr (validMethods,incc_rec.ff_method [0]);
			if (sptr != (char *) NULL)
			{
				switch (incc_rec.ff_option [0])
				{
				case	'A':
					store [0] [sptr - validMethods]++;
					numberItems++;
					break;

				case	'M':
					store [1] [sptr - validMethods]++;
					numberItems++;
					break;

				case	'P':
					store [2] [sptr - validMethods]++;
					numberItems++;
					break;

				default:
					break;
				}
			}
		}
	}
}

void
PrintOption (
 char*  option)
{
	register int	i;
	int		        indx = 0;
	long	        option_total;
	double	        percent;

	pr_format (fin,fout,"SUM_HEAD2",0,0);

	for (i = 0,option_total = 0L;i < strlen (validMethods);i++)
	{
		if (i == 0)
		{
			switch (option [0])
			{
			case	'A':
				pr_format (fin,fout,"SUM_LINE",1,"A");
				pr_format (fin,fout,"SUM_LINE",2,"(utomatic");
				indx = 0;
				break;

			case	'M':
				pr_format (fin,fout,"SUM_LINE",1,"M");
				pr_format (fin,fout,"SUM_LINE",2,"(anual");
				indx = 1;
				break;

			case	'P':
				pr_format (fin,fout,"SUM_LINE",1,"P");
				pr_format (fin,fout,"SUM_LINE",2,"(redetermined");
				indx = 2;
				break;

			default:
				pr_format (fin,fout,"SUM_LINE",1," ");
				pr_format (fin,fout,"SUM_LINE",2,"** ERROR **");
				indx = 0;
				break;
			}
		}
		else
		{
			pr_format (fin,fout,"SUM_LINE",1," ");
			pr_format (fin,fout,"SUM_LINE",2," ");
		}

		percent = (double) store [indx] [i];
		percent /= (double) numberItems;

		pr_format (fin,fout,"SUM_LINE",3,validMethods + i);
		pr_format (fin,fout,"SUM_LINE",4,store [indx] [i]);
		pr_format (fin,fout,"SUM_LINE",5,percent * 100);

		option_total += store [indx] [i];
	}
	percent = (double) option_total;
	percent /= (double) numberItems;

	pr_format (fin,fout,"SUM_BAR",0,0);

	pr_format (fin,fout,"SUM_TOT",1,option_total);
	pr_format (fin,fout,"SUM_TOT",2,percent * 100);
}

/*===============================================
| Load hhcc_hash table with valid warehouses	|
===============================================*/
void
LoadHhcc (void)
{
	int	indx;

	for (indx = 0;indx < MAX_CCMR;indx++)
		hhccHash [indx] = 0L;

	indx = 0;

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no, (byCompany) ? "  " : comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, (byWarehouse) ? comm_rec.cc_no : "  ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && indx < MAX_CCMR && 
		!strcmp (ccmr_rec.co_no,comm_rec.co_no))
	{
		if (ccmr_rec.lrp_ok  [0] == 'N')
		{
			cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
			continue;
		}
		if (byBranch)
		{
 			if (strcmp (ccmr_rec.est_no,comm_rec.est_no))
			{
				cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		if (!byWarehouse)
		{
 			if (strcmp (ccmr_rec.cc_no,comm_rec.cc_no))
			{
				cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		hhccHash [indx++] = ccmr_rec.hhcc_hash;
		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}

	abc_selfield (ccmr,"ccmr_hhcc_hash");
}
