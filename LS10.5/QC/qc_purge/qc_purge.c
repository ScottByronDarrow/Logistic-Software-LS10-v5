/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: qc_purge.c,v 5.4 2002/07/17 09:57:41 scott Exp $
|  Program Name  : ( qc_purge.c    )                                  |
|  Program Desc  : ( Purge QC Records.                            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, qchr, qcln, inmr, sumr, prmr, exwo,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  qchr, qcln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 02/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (03/11/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (30/11/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      :                                                    |
|  (01/11/94)    : PSL 11299 - upgrade to version 9 - mfg cutover -   |
|                : no code changes                                    |
|  (03/11/94)    : PSL 11299 - check QC_APPLY                         |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                                                                     |
| $Log: qc_purge.c,v $
| Revision 5.4  2002/07/17 09:57:41  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/19 03:04:22  robert
| Updated to avoid overlapping of description
|
| Revision 5.2  2001/08/09 09:16:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:50  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:01  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/13 05:50:27  scott
| Updated to remove inlo_gr_number as not used.
|
| Revision 2.0  2000/07/15 09:08:50  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/11/17 06:40:34  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/11/04 08:44:16  scott
| Updated to fix warnings due to -Wall flag.
|
| Revision 1.11  1999/09/29 10:12:26  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/21 05:26:37  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/18 05:55:47  scott
| Updated to add log file for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_purge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_purge/qc_purge.c,v 5.4 2002/07/17 09:57:41 scott Exp $";

#define	X_OFF		2
#define Y_OFF		5

#include <pslscr.h>
#include <std_decs.h>
#include <ml_std_mess.h>
#include <ml_qc_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>

#define AUDIT		(local_rec.audit [0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct qchrRecord	qchr_rec;
struct qclnRecord	qcln_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct prmrRecord	prmr_rec;
struct exwoRecord	exwo_rec;
struct inloRecord	inlo_rec;

	char	*data	= "data";
 
	FILE	*fout;
	int 	printed,
			multLoc;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	long	date;
	char	audit [2];
	char	auditDesc [4];
	int		lpno;

	char 	dummy [11];
	char	sysDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "date",	 4, 22, EDATETYPE,
		"DD/DD/DD", "           ",
		" ", local_rec.sysDate, "Release Purge Date : ", "Purge All Released QC Records Upto & Including Date - Defaults To Today",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.date},
	{1, LIN, "audit",	 6, 22, CHARTYPE,
		"U", "          ",
		" ", "Y", "Audit Trial Reqd   : ", "Enter Yes or No - Default : No",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.audit},
	{1, LIN, "auditDesc",	 6, 26, CHARTYPE,
		"AAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.auditDesc},
	{1, LIN, "lpno",	8, 22, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No         : ", "Valid Printer - Defaults To 1",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=============================
| Function prototypes         |
=============================*/
void	OpenDB			(void);
void	CloseDB 		(void);
int		spec_valid 		(int);
void	ProcessRecords 	(void);
int		CheckQcln 		(void);
int		heading			(int);
void	PrintHeading 	(void);
void	PrintDetails 	(void);
void	PrintTail 		(void);
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		QC_APPLY;
	char	*sptr;

	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
	{
		print_at(0,0,"\n\nQC_APPLY not set for this program to be used\n\n");
		return (EXIT_SUCCESS);
	}

	/* Multi-Location Available */
	multLoc = atoi (get_env ("MULT_LOC"));

	strcpy (local_rec.sysDate, DateToString (TodaysDate()));

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		printed		= FALSE;

		init_vars (1);

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		/*----------------------------
		| Process Orders in Database.|
		----------------------------*/
		clear ();
		crsr_off ();
		fflush (stdout);

		if (AUDIT)
		{
			open_rec (inmr,	 inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
			open_rec (sumr,	 sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
			open_rec (prmr,	 prmr_list, PRMR_NO_FIELDS, "prmr_id_no");
			open_rec (exwo,	 exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
		}

		ProcessRecords ();

		if (AUDIT)
			PrintTail ();

		CloseDB (); 
		FinishProgram ();
		break;
	}
	return (EXIT_SUCCESS);
}
	
/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (qchr,	 qchr_list, QCHR_NO_FIELDS, "qchr_id_no");
	open_rec (qcln,	 qcln_list, QCLN_NO_FIELDS, "qcln_id_no");
	open_rec (inlo,	 inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (qchr);
	abc_fclose (qcln);
	abc_fclose (inlo);

	if (AUDIT)
	{
		abc_fclose (inmr);
		abc_fclose (sumr);
		abc_fclose (prmr);
		abc_fclose (exwo);
	}
	abc_dbclose (data);
}

int
spec_valid (int field)
{
	if (LCHECK ("audit"))
	{
		switch (local_rec.audit [0])
		{
		case 'Y' :
			strcpy (local_rec.auditDesc, "Yes");
			DSP_FLD ("auditDesc");
			FLD ("lpno") = YES;
			if (prog_status != ENTRY)
			{
				do
				{
					get_entry (label ("lpno"));
					cc = spec_valid (label ("lpno"));
				} while (cc && !restart);
				if (restart)
					return (EXIT_SUCCESS);
			}
			break;
		default :
			strcpy (local_rec.auditDesc, "No ");
			FLD ("lpno") = NA;
			local_rec.lpno = 0;
			DSP_FLD ("lpno");
			break;
		}
		DSP_FLD ("auditDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (FLD ("lpno") != NA) 
		{
			if (SRCH_KEY)
			{
				local_rec.lpno = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lpno))
			{
				/*print_mess ("Invalid Printer");*/
				print_mess(ML(mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
ProcessRecords (void)
{
	float	tmpQty;

	if (AUDIT)
		PrintHeading ();

	strcpy (qchr_rec.co_no, comm_rec.co_no);
	strcpy (qchr_rec.br_no, comm_rec.est_no);
	strcpy (qchr_rec.wh_no, comm_rec.cc_no);
	strcpy (qchr_rec.qc_centre, " ");
	qchr_rec.hhbr_hash = 0L;
	qchr_rec.exp_rel_dt = 0L;
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qchr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qchr_rec.br_no, comm_rec.est_no) &&
		!strcmp (qchr_rec.wh_no, comm_rec.cc_no))
	{
		/*-------------------------------------------------------
		| Validate record before deletion, delete only if record|
		| has been completely released.                         |
		-------------------------------------------------------*/
		tmpQty = qchr_rec.rel_qty + qchr_rec.rej_qty;
		if (tmpQty < qchr_rec.origin_qty)
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}

		/*-------------------------------------------------------
		| Check that all the released lines are within the date |
		| given.                                                |
		-------------------------------------------------------*/
		if (!CheckQcln ())
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}
		
		if (AUDIT)
		{
			if ((cc = find_hash (inmr, &inmr_rec, EQUAL,
				"r", qchr_rec.hhbr_hash)))
				sprintf (inmr_rec.description, "%ld", qchr_rec.hhbr_hash);

			if ((cc = find_hash (sumr, &sumr_rec, EQUAL,
				"r", qchr_rec.hhsu_hash)))
				sprintf (sumr_rec.crd_name, "%ld", qchr_rec.hhsu_hash);
		}

		qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
		qcln_rec.release_dt	= 0L;
		qcln_rec.line_no	= 0;
		cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
		while (!cc &&
			qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
		{
			if (AUDIT)
			{
				strcpy (prmr_rec.co_no, comm_rec.co_no);
				strcpy (prmr_rec.br_no, comm_rec.est_no);
				strcpy (prmr_rec.code,	qcln_rec.emp_code);
				if ((cc = find_rec (prmr, &prmr_rec, EQUAL, "r")))
					strcpy (prmr_rec.name, qcln_rec.emp_code);

				strcpy (exwo_rec.co_no, comm_rec.co_no);
				strcpy (exwo_rec.code,	qcln_rec.reason);
				if ((cc = find_rec (exwo, &exwo_rec, EQUAL, "r")))
					strcpy (exwo_rec.description, qcln_rec.reason);

				PrintDetails ();
			}

			/*-------------------------------------------------------
			| Delete qcln record.                                   |
			-------------------------------------------------------*/
			if ((cc = abc_delete (qcln)))
				file_err (cc, "qcln", "DBDELETE");

			cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
		}

		/*-------------------------------------------------------
		| Delete qchr record.                                   |
		-------------------------------------------------------*/
		if ((cc = abc_delete (qchr)))
			file_err (cc, "qchr", "DBDELETE");

		cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	}

	/*------------------------------
	| nothing printed so tell user |
	------------------------------*/
	if (!printed && AUDIT)
		fprintf (fout, ".E NO QC RECORS PURGED \n");
}

int
CheckQcln (void)
{
	qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
	qcln_rec.line_no	= 0;
	while (!cc &&
		qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
	{
		if (qcln_rec.release_dt > local_rec.date)
			return (FALSE);

		cc = find_rec (qcln, &qcln_rec, NEXT, "r");
	}

	return (TRUE);
}

int
heading (int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		snorm ();
		
		rv_pr ("      QC Purge      ", 30, 0, 1);
		line_at (1,0,80);

		box (0, 3, 80, 5);
		line_at (5,1,79);
		line_at (7,1,79);
		line_at (20,0,80);

		strcpy(err_str,ML(mlStdMess038));
		print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_short);
		strcpy(err_str,ML(mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no,comm_rec.est_short);
		strcpy(err_str,ML(mlStdMess099));
		print_at (22,40,err_str,comm_rec.cc_no, comm_rec.cc_short);


		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (void)
{
	clear ();

	dsp_screen (" Print Purged QC Records Report ",
			comm_rec.co_no, comm_rec.co_name);

	/* Open print file. */
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".10\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E PURGED QC RECORDS REPORT \n");
	fprintf (fout, ".E COMPANY : %s %s \n",
			clip (comm_rec.co_no), clip (comm_rec.co_name));
	fprintf (fout, ".E BRANCH : %s %s \n",
			clip (comm_rec.est_no), clip (comm_rec.est_name));
	fprintf (fout, ".E WAREHOUSE : %s %s \n",
			clip (comm_rec.cc_no), clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R=========");
	fprintf (fout, "=====================");
	fprintf (fout, "=================================================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "=============");
	fprintf (fout, "===============");
	fprintf (fout, "=\n");

	fprintf (fout, "=========");
	fprintf (fout, "=====================");
	fprintf (fout, "=================================================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "=============");
	fprintf (fout, "===============");
	fprintf (fout, "=\n");

	fprintf (fout, "|   QC   ");
	fprintf (fout, "|    ITEM  NUMBER    ");
	fprintf (fout, "|                ITEM DESCRIPTION                ");
	fprintf (fout, "|  RECEIPT   ");
	fprintf (fout, "|  EXPT REL  ");
	fprintf (fout, "|  LOT  NO  ");
	fprintf (fout, "|  SLOT NO  ");
	fprintf (fout, "|   EXPIRY   ");
	if (multLoc)
		fprintf (fout, "|   LOCATION   ");
	else
		fprintf (fout, "|              ");
	fprintf (fout, "|\n");
}

void
PrintDetails (void)
{
	char	sourceType [15];

	dsp_process ("Item No :", inmr_rec.item_no);

	if (!printed)
		printed = TRUE;

	if (qchr_rec.source_type [0] == 'P')
		strcpy (sourceType, "Purchase Order");
	else
	if (qchr_rec.source_type [0] == 'W')
		strcpy (sourceType, "Works Order");
	else
		strcpy (sourceType, "Manual Order");

	fprintf (fout, "|--------");
	fprintf (fout, "---------------------");
	fprintf (fout, "-------------------------------------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "------------");
	fprintf (fout, "------------");
	fprintf (fout, "-------------");
	fprintf (fout, "---------------");
	fprintf (fout, "|\n");

	fprintf (fout, "| ITEM CATEGORY : %-11.11s",	inmr_rec.category);
	fprintf (fout, "%-128.128s",					" ");
	fprintf (fout, "|\n");

	inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
	cc = find_rec ("inlo", &inlo_rec, COMPARISON, "r");
	if (cc)
		memset (&inlo_rec, 0, sizeof (inlo_rec));
	
	fprintf (fout, "|  %-4.4s  ",			qchr_rec.qc_centre);
	fprintf (fout, "|  %-16.16s  ",			inmr_rec.item_no);
	fprintf (fout, "|  %-40.40s      ",		inmr_rec.description);
	fprintf (fout, "| %-10.10s ",			DateToString (qchr_rec.receipt_dt));
	fprintf (fout, "| %-10.10s ",			DateToString (qchr_rec.exp_rel_dt));
	fprintf (fout, "|  %-7.7s  ",			inlo_rec.lot_no);
	fprintf (fout, "|  %-7.7s  ",			inlo_rec.slot_no);
	fprintf (fout, "| %-10.10s ",			DateToString (inlo_rec.expiry_date));
	if (multLoc)
		fprintf (fout, "|  %-10.10s  ",		inlo_rec.location);
	else
		fprintf (fout, "|%-14.14s",			" ");
	fprintf (fout, "|\n");

	fprintf (fout, "| SUPPLIER : %-32.32s ",	sumr_rec.crd_name);
	fprintf (fout, "SOURCE TYPE  : %14.14s ",	sourceType);
	fprintf (fout, "ORIGINAL REC : %11.3f ",
			qchr_rec.origin_qty);
	fprintf (fout, "TOT RELEASED : %11.3f ",	qchr_rec.rel_qty);
	fprintf (fout, "TOT REJECTED : %11.3f ",	qchr_rec.rej_qty);
	fprintf (fout, "|\n");

	fprintf (fout, "| EMPLOYEE : %-32.32s ",	prmr_rec.name);
	fprintf (fout, "RELEASE DATE : %-10.10s     ",
			DateToString (qcln_rec.release_dt));
	fprintf (fout, "NXT RE DATE  : %-10.10s  ",
			DateToString (qcln_rec.reassay_date));
	fprintf (fout, "C.O.A.       : %-3.3s%-8.8s ",
			(qcln_rec.coa [0] == 'Y') ? "Yes" : "No", " ");
	fprintf (fout, "%-27.27s",					" ");
	fprintf (fout, "|\n");

	fprintf (fout, "| REMARKS  : %-32.32s ",	qcln_rec.remarks);
	fprintf (fout, "RELEASED QTY : %11.3f    ",	qcln_rec.rel_qty);
	fprintf (fout, "REJECTED QTY : %11.3f ",	qcln_rec.rej_qty);
	fprintf (fout, "%-54.54s",					" ");
	fprintf (fout, "|\n");

	if (qcln_rec.rej_qty > 0.00)
	{
		double	value = qcln_rec.cost * qcln_rec.rej_qty;

		fprintf (fout, "| REASON   : %-2.2s %-20.20s%-9.9s ",
				qcln_rec.reason, exwo_rec.description, " ");
		fprintf (fout, "DBT ACCT : %-16.16s   ",	qcln_rec.wof_acc);
		fprintf (fout, "CRD ACCT : %-16.16s", 		qcln_rec.stk_wof_acc);
		fprintf (fout, "COST/UNIT    : %10.2f  ",
				DOLLARS (qcln_rec.cost));
		fprintf (fout, "TOTAL COST   : %10.2f  ",		DOLLARS (value));
		fprintf (fout, "|\n");
	}
}

void
PrintTail (void)
{
	/* Close print file */
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

