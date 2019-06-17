/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ir_eod.c,v 5.4 2001/11/22 02:56:22 scott Exp $
|  Program Name  : (sk_ir_aud.c)                              
|  Program Desc  : (Issues / Receipts print program.) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.    | Date Written : 08/10/90         |
|---------------------------------------------------------------------|
| $Log: sk_ir_eod.c,v $
| Revision 5.4  2001/11/22 02:56:22  scott
| Updated for lineup
|
| Revision 5.3  2001/11/22 01:14:53  scott
| Updated to re-lineup reports as movement transaction reflected as 10 instead of 15
|
| Revision 5.2  2001/08/09 09:18:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:06  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/06 10:07:41  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ir_eod.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_eod/sk_ir_eod.c,v 5.4 2001/11/22 02:56:22 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>

#define	PRT_ISS		 (programType [0] == 'I')
#define	PRT_REC		 (programType [0] == 'R')
#define	PRT_PUR		 (programType [0] == 'P')

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inafRecord	inaf_rec;


	static char *tr_type [] =
	{
		"STK BAL ", 
		"STK REC ", 
		"STK ISS ", 
		"STK ADJ ", 
		"STK PUR ", 
		"INVOICES", 
		"CREDIT  ", 
		"PROD ISS", 
		"STK TRAN", 
		"PRD REC ", 
		"STK WOFF"
	};

	int		printerNumber = 1;
	int		auditDisplay = FALSE;

	char	programType [2];
	
	FILE	*fout;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	StartReport 		(void);
void 	ProcessTrans		(void);
void 	ProcessDisplay		(void);

    
/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "sk_ir_eod"))
	{
		/*============================
		| Setup required parameters. |	
		============================*/
		if (argc < 3)	
		{
			print_at (0, 0, mlSkMess508, argv [0]);
			return (EXIT_FAILURE);
		}
		printerNumber = atoi (argv [1]);
		sprintf (programType, "%-1.1s", argv [2]);
		auditDisplay = FALSE;
	}
	else
		auditDisplay = TRUE;

	OpenDB ();

	if (!auditDisplay)
	{
		if (PRT_ISS)
			sprintf (err_str, "PRINTING %s", "STOCK ISSUES.");

		if (PRT_REC)
			sprintf (err_str, "PRINTING %s", "STOCK RECEIPTS.");

		if (PRT_PUR)
			sprintf (err_str, "PRINTING %s", "STOCK PURCHASES.");

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
		StartReport ();

		ProcessTrans ();
	    fprintf (fout, ".EOF\n");
	    pclose (fout);
	    FinishProgram ();
        return (EXIT_SUCCESS);
	}
	else
		ProcessDisplay ();

	CloseDB (); 
	FinishProgram ();;
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
	open_rec (inaf, inaf_list, INAF_NO_FIELDS, "inaf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inaf);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

void
StartReport (void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".15\n");
	fprintf (fout, ".L158\n");

	if (PRT_ISS)
		fprintf (fout, ".EAUDIT TRAIL FOR STOCK ISSUES\n");

	if (PRT_REC)
		fprintf (fout, ".EAUDIT TRAIL FOR STOCK RECEIPTS\n");

	if (PRT_PUR)
		fprintf (fout, ".EAUDIT TRAIL FOR STOCK PURCHASES\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s : %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s : %s\n", comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s : %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT : %s\n", SystemTime ());

	fprintf (fout, ".R================");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "==================\n");

	fprintf (fout, "================");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===============================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "==================\n");

	fprintf (fout, "|   TRANSFER    ");
	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|   ITEM DESCRIPTION                       ");
	if (PRT_ISS)
	{	
		fprintf (fout, "|             ISSUE TO          ");
		fprintf (fout, "|   ISSUE    ");
		fprintf (fout, "|   ISSUE    ");
	}
	if (PRT_REC)
	{	
		fprintf (fout, "|          RECEIPT FROM         ");
		fprintf (fout, "|   RECEIPT  ");
		fprintf (fout, "|   RECEIPT  ");
	}
	if (PRT_PUR)
	{	
		fprintf (fout, "|            REFERENCE          ");
		fprintf (fout, "|  PURCHASE  ");
		fprintf (fout, "|  PURCHASE  ");
	}
	fprintf (fout, "|   EXTENDED.   |\n");

	fprintf (fout, "|       NO      ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|                               ");
	fprintf (fout, "|  QUANTITY  ");
	fprintf (fout, "|    COST    ");
	fprintf (fout, "|    VALUE.     |\n");
		
	fprintf (fout, "|---------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-------------------------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|---------------|\n");
	fflush (fout);

}

void
ProcessTrans (void)
{
	char	validTrans [2];
	double	tot_ext = 0.00;
	long	wk_date = 0L;

	wk_date = TodaysDate ();

	if (PRT_ISS)
		strcpy (validTrans, "I");

	if (PRT_REC)
		strcpy (validTrans, "R");

	if (PRT_PUR)
		strcpy (validTrans, "P");
	
	strcpy (inaf_rec.co_no, comm_rec.co_no);
	strcpy (inaf_rec.br_no, comm_rec.est_no);
	strcpy (inaf_rec.wh_no, comm_rec.cc_no);
	inaf_rec.sys_date = wk_date;
	strcpy (inaf_rec.ref1, "          ");
	cc = find_rec (inaf, &inaf_rec, GTEQ, "r");

	while (!cc && !strcmp (inaf_rec.co_no, comm_rec.co_no) &&
		          !strcmp (inaf_rec.br_no, comm_rec.est_no) &&
		          !strcmp (inaf_rec.wh_no, comm_rec.cc_no) &&
		          inaf_rec.sys_date == wk_date) 
	{
		dsp_process ("Processing Ref: ", inaf_rec.ref1);

		/*-------------------------
		| Check for Valid issues. |
		-------------------------*/
		if (PRT_ISS && inaf_rec.type != 3)
		{
			cc = find_rec (inaf, &inaf_rec, NEXT, "r");
			continue;
		}
		/*---------------------------
		| Check for Valid receipts. |
		---------------------------*/
		if (PRT_REC && inaf_rec.type != 2)
		{
			cc = find_rec (inaf, &inaf_rec, NEXT, "r");
			continue;
		}
		/*----------------------------
		| Check for Valid Purchases. |
		----------------------------*/
		if (PRT_PUR && inaf_rec.type != 5)
		{
			cc = find_rec (inaf, &inaf_rec, NEXT, "r");
			continue;
		}
		inmr_rec.hhbr_hash 	= inaf_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (inmr_rec.item_no, "%16.16s", " ");
			sprintf (inmr_rec.description, "%40.40s", " ");
		}

		fprintf (fout, "|%s",  inaf_rec.ref1);
		fprintf (fout, "| %16.16s ", inmr_rec.item_no);
		fprintf (fout, "| %40.40s ", inmr_rec.description);

		if (!PRT_PUR)
		{
			sprintf (ccmr_rec.co_no,  "%-2.2s", inaf_rec.ref2 + 2);
			sprintf (ccmr_rec.est_no, "%-2.2s", inaf_rec.ref2 + 5);
			sprintf (ccmr_rec.cc_no,  "%-2.2s", inaf_rec.ref2 + 8);

			cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
			if (cc)
			{
				fprintf (fout, "|%-15.15s|%-15.15s", 
							inaf_rec.ref1, inaf_rec.ref2);
			}
			else
			{
				fprintf (fout, "| %-2.2s - %-24.24s ", 
							ccmr_rec.cc_no, ccmr_rec.name);
			}
		}
		else
			fprintf (fout, "|%-15.15s|%-15.15s", inaf_rec.ref1, inaf_rec.ref2);

		fprintf (fout, "|%11.2f ", inaf_rec.qty);
		fprintf (fout, "|%11.2f ", DOLLARS (inaf_rec.cost_price));
		fprintf (fout, "|%14.2f |\n", 
				DOLLARS (inaf_rec.qty * inaf_rec.cost_price));
		tot_ext += inaf_rec.qty * inaf_rec.cost_price;
		cc = find_rec (inaf, &inaf_rec, NEXT, "r");
	}
	fprintf (fout, "|---------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-------------------------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|---------------|\n");

	fprintf (fout, "|               ");
	fprintf (fout, "|   *** TOTALS *** ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|                               ");
	fprintf (fout, "|            ");
	fprintf (fout, "|            ");
	fprintf (fout, "| %13.2f |\n", DOLLARS (tot_ext));
	return;
}

void
ProcessDisplay (void)
{
	char	disp_str [300];
	double	tot_ext = 0.00;

	set_tty ();
	init_scr ();
	clear ();
	swide ();

	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21, 40, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (22, 0, ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);

	Dsp_prn_open (0, 1, 14, err_str, comm_rec.co_no, comm_rec.co_name, 
					 (char *) 0, (char *) 0, 
					 (char *) 0, (char *) 0);

	Dsp_saverec (" TRANSACTION   |   TRANS  |  ITEM  NUMBER  |    TRANS DESCRIPTION.    | QUANTITY |     COST   |  EXTENDED   ");
	Dsp_saverec ("   REFERENCE   |   DATE   |                |                          |          |            |    VALUE    ");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");
	

	strcpy (inaf_rec.co_no, comm_rec.co_no);
	strcpy (inaf_rec.br_no, comm_rec.est_no);
	strcpy (inaf_rec.wh_no, comm_rec.cc_no);
	inaf_rec.sys_date = 0L;
	strcpy (inaf_rec.ref1, "          ");
	cc = find_rec (inaf, &inaf_rec, GTEQ, "r");

	while (!cc && !strcmp (inaf_rec.co_no, comm_rec.co_no) &&
		      !strcmp (inaf_rec.br_no, comm_rec.est_no) &&
		      !strcmp (inaf_rec.wh_no, comm_rec.cc_no))
	{
		inmr_rec.hhbr_hash 	= inaf_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (inmr_rec.item_no, "%16.16s", " ");
			sprintf (inmr_rec.description, "%40.40s", " ");
		}

		sprintf (disp_str, "%-15.15s^E%-10.10s^E%16.16s^E%8.8s - %-15.15s^E %8.2f ^E%11.2f ^E%12.2f ", 
				inaf_rec.ref1, 
				DateToString (inaf_rec.date), 
				inmr_rec.item_no, 
				tr_type [inaf_rec.type -1], 
				inaf_rec.ref2, 
				inaf_rec.qty, 
				DOLLARS (inaf_rec.cost_price), 
				DOLLARS (inaf_rec.qty * inaf_rec.cost_price));
				
		Dsp_saverec (disp_str);
		tot_ext += inaf_rec.qty * inaf_rec.cost_price;
		cc = find_rec (inaf, &inaf_rec, NEXT, "r");
	}
	Dsp_saverec ("^^GGGGGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGG");

	sprintf (disp_str, "               ^E          ^E                ^E                          ^E          ^E            ^E%12.2f ", 
			DOLLARS (tot_ext));
				
	Dsp_saverec (disp_str);
	

	Dsp_srch ();
	Dsp_close ();
	snorm ();
	rset_tty ();
}
