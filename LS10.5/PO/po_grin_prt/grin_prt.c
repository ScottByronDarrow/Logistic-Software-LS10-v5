/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: grin_prt.c,v 5.4 2002/07/17 09:57:36 scott Exp $
|  Program Name  : (po_grin_prt.c )                                   |
|  Program Desc  : (Goods Receipt Audit Print.                  )     |
|                  (                                            )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Updates files : See /usr/ver (x)/DOCS/Programs                     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (26/11/1996)    | Modified  by : Scott B Darrow.   |
|  Date Modified : (17/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                :                                                    |
|   Comments     :   SEL - Updated for problem with average cost.     |
|  (17/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
| $Log: grin_prt.c,v $
| Revision 5.4  2002/07/17 09:57:36  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/10/19 02:56:10  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:15:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:46  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:27  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:43  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/07/18 23:46:41  gerry
| Replaced FindSupercesion with IntFindSupercession
|
| Revision 2.0  2000/07/15 09:05:15  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/05/31 03:38:00  scott
| Updated to change FindInmr () to IntFindInmr () due t conflict with new routines
|
| Revision 1.16  1999/12/10 04:14:56  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.15  1999/12/06 01:32:34  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/11 06:43:14  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.13  1999/11/05 05:17:11  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.12  1999/10/14 03:04:22  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.11  1999/09/29 10:11:56  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/21 04:38:02  scott
| Updated from Ansi project
|
| Revision 1.9  1999/06/17 10:06:24  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
|                :                                                    |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: grin_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_grin_prt/grin_prt.c,v 5.4 2002/07/17 09:57:36 scott Exp $";

#include	<pslscr.h>
#include	<time.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include 	<get_lpno.h>
#include 	<ml_po_mess.h>
#include 	<ml_std_mess.h>

	/*---------------------------
	| Special fields and flags. |
	---------------------------*/
    FILE* fout;

	int		printerNumber = 1,
			envVarMultLoc = 0;

	char	systemDate [11];
	long	lsystemDate = 0L;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct poghRecord	pogh_rec;
struct poglRecord	pogl_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;

	char 	*inmr2 = "inmr2",
	    	*data = "data";

	double	receiptTotal = 0.00;

	double	value = 0.00;

/*===========================
| Local & Screen Structures |
===========================*/
struct {
							/*---------------------------------------*/
	char	dummy [11];		/*| Dummy Used In Screen Generator.	    |*/
	int		printerNumber;
							/*---------------------------------------*/
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "gr_no",4, 22, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Goods Received No.", " ",
		YES, NO,  JUSTLEFT, "", "", pogh_rec.gr_no},
	{1, LIN, "printerNumber",	5, 22, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
int 	IntFindInmr 		(long);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	IntFindSupercession (char *);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintAudit 			(double);
void 	PrintInex 			(void);
void 	ProcessPogl 		(void);
void 	ProcessStock 		(long, long);
void 	SrchPogh 			(char *);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	envVarMultLoc = (sptr = chk_env ("MULT_LOC")) ? atoi (sptr) : 0;

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	OpenDB ();

	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		init_vars (1);	

		receiptTotal = 0.00;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*------------------
		| Edit all screens.| 
		------------------*/
		edit_all ();

		if (restart)
			continue;

		OpenAudit ();
		ProcessPogl ();
	}

	shutdown_prog ();	
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (inmr2, inmr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (pogh,  pogh_list, POGH_NO_FIELDS, "pogh_id_no2");
	open_rec (pogl,  pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inex,  inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (incc);
	abc_fclose (inum);
	abc_fclose (inmr2);
		
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	char	*sptr;

	/*---------------------------------
	| Validate Goods Receipt Number.  |
	---------------------------------*/
	if (LCHECK ("gr_no"))
	{
		if (SRCH_KEY)
		{
			SrchPogh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (err_str,pogh_rec.gr_no);
		sptr = clip (err_str);

		while (strlen (sptr))
		{
			if (*sptr != ' ' && *sptr != '0')
				break;
			sptr++;
		}

		if (!strlen (sptr))
		{
			/*----------------
			| GRN not found. |
			----------------*/
			print_mess (ML (mlStdMess049));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pogh_rec.co_no,comm_rec.co_no);
		cc = find_rec (pogh,&pogh_rec,EQUAL,"r");
		if (cc)
		{
			/*----------------
			| GRN not found. |
			----------------*/
			print_mess (ML (mlStdMess049));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			/*------------------
			| Invalid printer. |
			------------------*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


/*==================================
| Search for goods receipt number. |
==================================*/
void
SrchPogh (
	char	*key_val)
{
	work_open ();
	save_rec ("#Goods Receipt  ","# Date Received");
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",key_val);
	cc = find_rec (pogh,&pogh_rec,GTEQ,"r");
	while (!cc && !strncmp (pogh_rec.gr_no,key_val,strlen (key_val)) &&
		      !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		sprintf (err_str, " %s ", DateToString (pogh_rec.date_raised));

		cc = save_rec (pogh_rec.gr_no,err_str);
		if (cc)
			break;

		cc = find_rec (pogh,&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pogh_rec.co_no,comm_rec.co_no);
	sprintf (pogh_rec.gr_no,"%-15.15s",temp_str);
	cc = find_rec (pogh,&pogh_rec,EQUAL,"r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");
}

void
ProcessPogl (
 void)
{
	OpenAudit ();

	pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
	pogl_rec.line_no = 0;

	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	while (!cc && pogl_rec.hhgr_hash == pogh_rec.hhgr_hash)
	{
		cc = IntFindInmr (pogl_rec.hhbr_hash);
		if (cc)
			continue;
				
		ProcessStock 
		(
			inmr_rec.hhbr_hash,
	   		pogl_rec.hhcc_hash
		);
		cc = find_rec (pogl, &pogl_rec, NEXT, "r");
	}
	CloseAudit ();
}

void
IntFindSupercession (
 char *item_no)
{
	if (!strcmp (item_no, "                "))
		return;

	strcpy (inmr2_rec.co_no, comm_rec.co_no);
	sprintf (inmr2_rec.item_no, "%-16.16s", item_no);
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (!cc)
		IntFindSupercession (inmr2_rec.supercession);
}

int
IntFindInmr (
 long hhbr_hash)
{
	cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", hhbr_hash);
	if (cc)
		return (cc);

	if (inmr_rec.hhsi_hash != 0L)
	{
		cc = find_hash (inmr, &inmr_rec, COMPARISON,"r", inmr_rec.hhsi_hash);
		if (cc)
			return (cc);
	}

	if (strcmp (inmr_rec.supercession, "                "))
	{
		IntFindSupercession (inmr_rec.supercession);
		cc = find_hash (inmr,&inmr_rec, COMPARISON,"r",inmr2_rec.hhbr_hash);
	}
	return (cc);
}

/*================
| Process Stock. |
================*/
void
ProcessStock (
 long	hhbr_hash,
 long	hhcc_hash)
{
	double	new_cost 	= 0.00;
	double	extend 		= 0.00;
	double	value 		= 0.00;

	ccmr_rec.hhcc_hash	=	hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	new_cost = pogl_rec.land_cst;
		
	value = twodec (pogl_rec.land_cst);
	value = out_cost (value, inmr_rec.outer_size);
	extend = value * (double) pogl_rec.qty_rec;
	extend = twodec (extend);
	
	dsp_process (" Item No. : ", inmr_rec.item_no);

	PrintAudit (extend);
	receiptTotal += extend;

	return;
}

/*================================
| Routine to print line item to  |
| the audit trail                |
================================*/
void
PrintAudit (
 double extend)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	fprintf (fout, "|%15.15s",			pogl_rec.po_number);
	fprintf (fout, "|%15.15s",			pogh_rec.gr_no);
	fprintf (fout, "|%2.2s", 			ccmr_rec.est_no);
	fprintf (fout, "|%2.2s - %9.9s",	ccmr_rec.cc_no,
									  	ccmr_rec.acronym);
	fprintf (fout, "|%16.16s",			inmr_rec.item_no);
	if (envVarMultLoc)
	{
		fprintf (fout, "|%-24.24s",		pogl_rec.item_desc);
		fprintf (fout, "|%10.10s",		pogl_rec.location);
	}
	else
		fprintf (fout, "|%-34.34s",		pogl_rec.item_desc);

	fprintf (fout, "|%10.10s",			DateToString (pogl_rec.rec_date));
	fprintf (fout, "|%4.4s",			inum_rec.uom);
	fprintf (fout, "|%11.2f",			pogl_rec.qty_rec * CnvFct);
	fprintf (fout, "|%12.2f",			pogl_rec.land_cst / CnvFct);
	fprintf (fout, "|%12.2f|\n",		extend);
	PrintInex ();

	if (strcmp (pogl_rec.serial_no, "                         "))
	{
		fprintf (fout, "|               ");
		fprintf (fout, "|               ");
		fprintf (fout, "|  ");
		fprintf (fout, "|              ");
		fprintf (fout, "|                ");
		if (envVarMultLoc)
			fprintf (fout, "|SER NO : %25.25s ", pogl_rec.serial_no);
		else
			fprintf (fout, "|SER NO : %25.25s", pogl_rec.serial_no);

		fprintf (fout, "|          ");
		fprintf (fout, "|    ");
		fprintf (fout, "|           ");
		fprintf (fout, "|            ");
		fprintf (fout, "|            |\n");
	}
	fflush (fout);
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ( (fout = popen ("pformat","w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	if (comm_rec.inv_date > lsystemDate)
		fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.inv_date),PNAME);
	else
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n",clip (comm_rec.co_name));
	fprintf (fout, ".E%s / %s\n",clip (comm_rec.est_name),comm_rec.cc_name);
	fprintf (fout, ".EPURCHASE ORDER GOODS RECEIPTS\n");
	fprintf (fout, ".Eas at %-24.24s\n",SystemTime ());

	fprintf (fout, ".R================");
	fprintf (fout, "================");
	fprintf (fout, "===");
	fprintf (fout, "===============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	if (envVarMultLoc)
	{
		fprintf (fout, "=========================");
		fprintf (fout, "===========");
	}
	else
		fprintf (fout, "===================================");

	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "=============");
	fprintf (fout, "==============\n");

	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "===");
	fprintf (fout, "===============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	if (envVarMultLoc)
	{
		fprintf (fout, "=========================");
		fprintf (fout, "===========");
	}
	else
		fprintf (fout, "===================================");

	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "=============");
	fprintf (fout, "==============\n");

	fprintf (fout, "|P/ORDER NUMBER ");
	fprintf (fout, "|  GRIN NUMBER  ");
	fprintf (fout, "|BR");
	fprintf (fout, "|  WAREHOUSE.  ");
	fprintf (fout, "|  ITEM NUMBER   ");
	if (envVarMultLoc)
	{
		fprintf (fout, "|   ITEM DESCRIPTION     ");
		fprintf (fout, "| LOCATION ");
	}
	else
		fprintf (fout, "|        ITEM DESCRIPTION          ");

	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|UOM.");
	fprintf (fout, "| QUANTITY  ");
	fprintf (fout, "| UNIT  COST ");
	fprintf (fout, "|EXTEND COST |\n");

	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|--");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------------");
	if (envVarMultLoc)
	{
		fprintf (fout, "|------------------------");
		fprintf (fout, "|----------");
	}
	else
		fprintf (fout, "|----------------------------------");

	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------|\n");

	fflush (fout);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|--");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------------");
	if (envVarMultLoc)
	{
		fprintf (fout, "|------------------------");
		fprintf (fout, "|----------");
	}
	else
	{
		fprintf (fout, "|----------------------------------");
	}

	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------|\n");

	fprintf (fout, "|               ");
	fprintf (fout, "|               ");
	fprintf (fout, "|  ");
	fprintf (fout, "|              ");
	fprintf (fout, "|                ");
	if (envVarMultLoc)
	{
		fprintf (fout, "| TOTAL VALUE RECEIPTED  ");
		fprintf (fout, "|          ");
	}
	else
		fprintf (fout, "| TOTAL VALUE RECEIPTED            ");

	fprintf (fout, "|          ");
	fprintf (fout, "|    ");
	fprintf (fout, "|           ");
	fprintf (fout, "|            ");
	fprintf (fout, "|%12.2f|\n",receiptTotal);

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
PrintInex (
 void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout, "|%15.15s",	" ");
		fprintf (fout, "|%15.15s",	" ");
		fprintf (fout, "|%2.2s", 	" ");
		fprintf (fout, "|%2.2s   %9.9s",	" ",	" ");
		fprintf (fout, "|%16.16s",	" ");
		if (envVarMultLoc)
		{
			fprintf (fout, "|%-24.24s",	inex_rec.desc);
			fprintf (fout, "|          ");
		}
		else
			fprintf (fout, "|%-34.34s",	inex_rec.desc);
		fprintf (fout, "|%10.10s",	" ");
		fprintf (fout, "|%4.4s",	" ");
		fprintf (fout, "|%11.2s",	" ");
		fprintf (fout, "|%12.2s",	" ");
		fprintf (fout, "|%12.2s|\n"," ");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	/*---------------------
	| Print Goods Receipt.|
	---------------------*/
	rv_pr (ML (mlPoMess120),30,0,1);
	move (0,1);
	line (80);

	box (0,3,80,2);

	move (0,20);
	line (80);
	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (21,1, "%s", err_str);
	move (0,22);
	line (80);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
