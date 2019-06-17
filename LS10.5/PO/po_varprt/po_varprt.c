/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_varprt.c,v 5.2 2001/08/09 09:16:20 scott Exp $
|  Program Name  : ( po_varprt.c     )                                |
|  Program Desc  : ( Print Costing Variances Reports.             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, poca,                             ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 02/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/87)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|  Date Modified : (09/09/97)      | Modified  by  : Marnie Organo    |
|                                                                     |
|  Comments      :  X : P.O. Cost of Sale Exceptions                  |
|                :  V : P.O. Costing Variances ( over 10% )           |
|                :  E : P.O. Estimated/Actual Cost Variances          |
|                :  (17/04/89) Changed MONEYTYPEs to DOUBLETYPEs.     |
|                :  Updated for Multilingual Conversion               |
|                                                                     |
| $Log: po_varprt.c,v $
| Revision 5.2  2001/08/09 09:16:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:24  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:59  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:14  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:44  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/02/15 21:45:34  cam
| Updated to correct programs that update database tables without locking the
| record. This does not cause a problem with the character based code when linked
| with the CISAM-DBIF interface, but causes a problem with GVision, and the
| character-based source linked with the Oracle or Informix Online DBIF.
|
| Revision 1.12  1999/12/06 01:32:54  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/10/20 02:06:59  nz
| Updated for final changes on date routines.
|
| Revision 1.10  1999/10/14 03:04:38  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/10/13 23:14:30  cam
| Removed max_work
|
| Revision 1.8  1999/09/29 10:12:22  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 04:38:17  scott
| Updated from Ansi project
|
| Revision 1.6  1999/06/17 10:06:45  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_varprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_varprt/po_varprt.c,v 5.2 2001/08/09 09:16:20 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct pocaRecord	poca_rec;

	int		printerNumber = 1,
			first_flag = FALSE,
			cat_line = 0;

	char	reportType [2],
			previousCategory [12];
	
	float	cost 		= 0.0,
			cost_pc 	= 0.0,
			tot_est 	= 0.0,
			tot_act 	= 0.0,
			tot_pc 		= 0.0,
			tot_cost_var = 0.0,
			grand_est 	= 0.0,
			grand_act 	= 0.0,
			grand_var 	= 0.0,
			grand_pc 	= 0.0;

	FILE	*pp;
	
	static char *rep_header [] = {
 		" PURCHASE ORDER COST OF SALES EXCEPTIONS ",
 		" PURCHASE ORDER COSTING VARIANCES (OVER 10%%) ",
 		" PURCHASE ORDER ESTIMATED/ACTUAL COST VARIANCE "
	};

	static char *rep_desc [] = {
 		" COST OF SALES EXCEPTIONS ",
 		" COSTING VARIANCES (OVER 10%) ",
 		" ESTIMATED/ACTUAL COST VARIANCE "
	};


/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	StartReport 	(void);
void 	PrintLine 		(void);
void 	ProcessFile 	(void);
void 	CalculateVars 	(void);
void 	PrintExcept 	(void);
void 	PrintTotalCat 	(char *);
void 	printGrandTot 	(void);
void 	UpdatePoca 		(void);
void 	EndReport 		(void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{

	if (argc < 3)
	{
		print_at (0,0,mlPoMess709,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	OpenDB ();

	/*-------------
	| Report Type |
	-------------*/
	switch (argv [2][0])
	{
	case	'X':
	case	'x':
		strcpy (reportType,"X");
		sprintf (err_str,"PRINTING %s",rep_desc [0]);
		break;

	case	'V':
	case	'v':
		strcpy (reportType,"V");
		sprintf (err_str,"PRINTING %s",rep_desc [1]);
		break;

	case	'E':
	case	'e':
		strcpy (reportType,"E");
		sprintf (err_str,"PRINTING %s",rep_desc [2]);
		break;

	default:
		print_at (1,0,mlPoMess709,argv [0]);
		print_at (2,0,mlPoMess710);
		print_at (3,0,mlPoMess711);
		print_at (4,0,mlPoMess712);
        return (EXIT_FAILURE);
	}

	dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

	StartReport ();

	ProcessFile ();

	EndReport ();
	pclose (pp);
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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("poca",poca_list,POCA_NO_FIELDS,"poca_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("poca");
	abc_dbclose ("data");
}

void
StartReport (
 void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ( (pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",printerNumber);

	fprintf (pp,".15\n");
	fprintf (pp,".PI12\n");
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s / %s\n",clip (comm_rec.est_name),comm_rec.cc_name);
	fprintf (pp,".B1\n");

	/*--------------------------------
	| Get the relevant report title. |
	--------------------------------*/
	switch (reportType [0])
	{
	case 'X':
		fprintf (pp,".E%s\n",rep_header [0]);
		break;

	case 'V':
		fprintf (pp,".E%s\n",rep_header [1]);
		break;

	case 'E':
		fprintf (pp,".E%s\n",rep_header [2]);
		break;
	}

	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n",SystemTime ());
	fprintf (pp,".B1\n");

	fprintf (pp,".R===============================");
	fprintf (pp,"======================================");
	fprintf (pp,"================================");
	fprintf (pp,"============================");
	fprintf (pp,"===========================\n");
  
	fprintf (pp,"===============================");
	fprintf (pp,"======================================");
	fprintf (pp,"================================");
	fprintf (pp,"============================");
	fprintf (pp,"===========================\n");
  
	fprintf (pp,"|  CATEGORY   |       ITEM     ");
	fprintf (pp,"|             DESCRIPTION             ");
	fprintf (pp,"|PURCHASES ORDER| GOODS RECEIPT ");

	if (reportType [0] == 'X' || reportType [0] == 'E')
		fprintf (pp,"|  ESTIMATED  |   ACTUAL    ");
	else if (reportType [0] == 'V')
		fprintf (pp,"|     OLD     |     NEW     ");
	fprintf (pp,"|    COST     |  PERCENT  |\n");
		
	fprintf (pp,"|   NUMBER    |     NUMBER     ");
	fprintf (pp,"|                                     ");
	fprintf (pp,"|     NUMBER    |     NUMBER    ");
	fprintf (pp,"|     COST    |     COST    ");
	fprintf (pp,"|  VARIANCE   |  VARIANCE |\n");

	PrintLine ();
	first_flag = TRUE;

	fflush (pp);
}

void
PrintLine (
 void)
{
	fprintf (pp,"|-------------|----------------");
	fprintf (pp,"|-------------------------------------");
	fprintf (pp,"|---------------|---------------");
	fprintf (pp,"|-------------|-------------");
	fprintf (pp,"|-------------|-----------|\n");
	fflush (pp);
}

/*=======================================
| Validate and print poca details.	|
=======================================*/
void
ProcessFile (
 void)
{
	strcpy (poca_rec.co_no,comm_rec.co_no);
	strcpy (poca_rec.br_no,comm_rec.est_no); 
	poca_rec.type [0] = reportType [0];
	strcpy (poca_rec.item_cat,"           "); 
	sprintf (poca_rec.item_no,"%16.16s"," "); 
        poca_rec.line_no = 0;
	strcpy (previousCategory,"           "); 

	cc = find_rec ("poca",&poca_rec,GTEQ,"u");
	while (!cc && !strcmp (poca_rec.co_no,comm_rec.co_no) && !strcmp (poca_rec.br_no,comm_rec.est_no) &&  poca_rec.type [0] == reportType [0])
	{
		if (poca_rec.status [0] != 'C')
		{
			abc_unlock ("poca");
		  	cc = find_rec ("poca",&poca_rec,NEXT,"u");
		  	continue;
		}
		if (strcmp (previousCategory,poca_rec.item_cat) != 0)
		{
			if (!first_flag)
			{
				if (cat_line > 1)
					PrintTotalCat (previousCategory);
				PrintLine ();
				fprintf (pp,"| %11.11s ",poca_rec.item_cat);
			}
			else
			{
				fprintf (pp,"| %11.11s ",poca_rec.item_cat);
				first_flag = FALSE;
			}
			cat_line = 0;
			tot_est = 0.0;
			tot_act = 0.0;
			tot_cost_var = 0.0;
			tot_pc = 0.0;
			strcpy (previousCategory,poca_rec.item_cat);
		}	
		else
			fprintf (pp,"|             ");

		dsp_process ("Item No. : ",poca_rec.item_no);
		CalculateVars ();
		PrintExcept ();

		abc_unlock ("poca");
		cc = find_rec ("poca",&poca_rec,NEXT,"u");
	}
	abc_unlock ("poca");

	if (cat_line > 1)
	{
		PrintTotalCat (previousCategory);
		printGrandTot ();
	}
}

void
CalculateVars (
 void)
{
	double	wk_value = 0.00;

	cost = (float) (poca_rec.est_cst - poca_rec.act_cst);

	wk_value = poca_rec.est_cst;
	if (wk_value != 0.00)
		cost_pc = (float) (cost/poca_rec.est_cst * 100);
	else
		cost_pc = 0.00;
	tot_est += (float) (poca_rec.est_cst);
	tot_act += (float) (poca_rec.act_cst);
	tot_cost_var += cost;
	grand_est += (float) (poca_rec.est_cst);
	grand_act += (float) (poca_rec.act_cst);
	grand_var += cost;
}

void
PrintExcept (
 void)
{
	fprintf (pp,"|%16.16s",poca_rec.item_no);
	fprintf (pp,"|%-37.37s",poca_rec.item_desc);
	fprintf (pp,"|%15.15s",poca_rec.po_no);
	fprintf (pp,"|%15.15s",poca_rec.gr_no);
	fprintf (pp,"|%12.2f ",poca_rec.est_cst);
	fprintf (pp,"|%12.2f ",poca_rec.act_cst);
	fprintf (pp,"|%12.2f ",cost);
	fprintf (pp,"| %9.2f |\n",cost_pc);
	fflush (pp);
	cat_line++;
	cost = 0.0;
	cost_pc = 0.0;
	UpdatePoca ();
}

void
PrintTotalCat (
 char *cat)
{
	if (tot_est != 0.00)
		tot_pc = tot_cost_var/tot_est * 100;

	fprintf (pp,"| ***** TOTAL FOR CATEGORY : %s ***** ",cat);
	fprintf (pp,"                      ");
	fprintf (pp,"|               |               ");
	fprintf (pp,"|%12.2f ",tot_est);
	fprintf (pp,"|%12.2f ",tot_act);
	fprintf (pp,"|%12.2f ",tot_cost_var);
	fprintf (pp,"| %9.2f |\n",tot_pc);
}

void
printGrandTot (
 void)
{
	if (grand_est != 0.00)
		grand_pc = grand_var/grand_est * 100;

	fprintf (pp,"| ***** TOTAL PURCHASE ORDER ***** ");
	fprintf (pp,"                                  ");
	fprintf (pp,"|            |            ");
	fprintf (pp,"|%12.2f ",grand_est);
	fprintf (pp,"|%12.2f ",grand_act);
	fprintf (pp,"|%12.2f ",grand_var);
	fprintf (pp,"| %9.2f |\n",grand_pc);
}

void
UpdatePoca (
 void)
{
	strcpy (poca_rec.co_no,comm_rec.co_no);
	strcpy (poca_rec.br_no,comm_rec.est_no);
	strcpy (poca_rec.status,"D");
	poca_rec.date_print = TodaysDate ();
	cc = abc_update ("poca",&poca_rec);
	if (cc)
		sys_err ("Error in poca during (DBUPDATE)",cc,PNAME);
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (
 void)
{
	fprintf (pp,".EOF\n");
}
