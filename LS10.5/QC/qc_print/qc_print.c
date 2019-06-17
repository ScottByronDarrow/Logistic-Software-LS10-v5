/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: qc_print.c,v 5.5 2002/07/25 11:17:31 scott Exp $
|  Program Name  : ( qc_dspprt.c    )                                 |
|  Program Desc  : ( Print QC Release/To Be Released Records Report  )|
|---------------------------------------------------------------------|
|  Access files  :  comm, qchr, qcln, inmr, qcmr, prmr, exwo, sumr,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : 15/08/94        | Author       : Aroha Merrilees.  |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (03/11/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (30/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (11/10/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (09/09/97)      | Modified  by : Marnie I Organo.  |
|  Date Modified : (17/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (01/11/94)    : PSL 11299 - upgrade to version 9 - mfg cutover -   |
|                : no code changes                                    |
|  (03/11/94)    : PSL 11299 - check QC_APPLY                         |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|                                                                     |
|  (11/10/96)    : SEL - Updated to cater for year 2000.              |
|  (09/09/97)    : Modified for Multilingual Conversion.              |
|  (17/09/1999)  : Ported to ANSI standards.                          |
| $Log: qc_print.c,v $
| Revision 5.5  2002/07/25 11:17:31  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.4  2002/07/17 09:57:40  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/24 03:45:21  robert
| Updated to avoid overlapping descriptions
|
| Revision 5.2  2001/08/09 09:16:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:53  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:49  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:00  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/13 05:49:31  scott
| Updated to remove inlo_gr_number as not used.
|
| Revision 2.0  2000/07/15 09:08:49  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/06/13 05:02:23  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.12  2000/02/18 02:25:49  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.11  1999/11/17 06:40:34  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/04 08:44:16  scott
| Updated to fix warnings due to -Wall flag.
|
| Revision 1.9  1999/09/29 10:12:25  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 05:26:37  scott
| Updated from Ansi Project
|                                                |
| Revision 1.7  1999/06/18 05:55:46  scott                            |
| Updated to add log file for cvs.                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_print.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_print/qc_print.c,v 5.5 2002/07/25 11:17:31 scott Exp $";

#define		MOD		3

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_qc_mess.h>

#define		RELEASED	(local_rec.type [0] == 'R')
#define		CATEGORY	'C'
#define		DATE		'D'
#define		SUPPLIER	'S'

#include	"schema"

struct commRecord	comm_rec;
struct qchrRecord	qchr_rec;
struct qclnRecord	qcln_rec;
struct inmrRecord	inmr_rec;
struct qcmrRecord	qcmr_rec;
struct prmrRecord	prmr_rec;
struct exwoRecord	exwo_rec;
struct sumrRecord	sumr_rec;
struct inloRecord	inlo_rec;

	char	*data	= "data";

	FILE	*fout,
			*fsort;
	
	int		multLoc;
	char	prevCategory [12];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sysDate [11];

	char	type [2];
	char	typeDesc [13];
	char	sourceType [2];
	char	sourceTypeDesc [15];
	char	stItemNo [17];
	char	edItemNo [17];
	long	stDate;
	long	edDate;
	char	stCentre [5];
	char	edCentre [5];
	char	sortBy [2];
	char	sortByDesc [16];
	int		lpno;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "type", 	 4, 17, CHARTYPE, 
		"U", "          ", 
		" ", "R", " Rel/Pre-Rel   :", "R(eleased Or P(re-Released - Defaults To Released", 
		YES, NO,  JUSTLEFT, "RP", "", local_rec.type}, 
	{1, LIN, "typeDesc", 	 4, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO,  JUSTLEFT, "RP", "", local_rec.typeDesc}, 
	{1, LIN, "sourceType", 	 5, 17, CHARTYPE, 
		"U", "          ", 
		" ", "A", " Source Type   :", "Source Type - A(ll P(urchase Orders W(orks Orders M(anual - Default To All", 
		YES, NO,  JUSTLEFT, "APWM", "", local_rec.sourceType}, 
	{1, LIN, "sourceTypeDesc", 	 5, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.sourceTypeDesc}, 
	{1, LIN, "stItemNo", 	 7, 17, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", " Start Item No :", "Start Item No - Search Available", 
		YES, NO,  JUSTLEFT, "", "", local_rec.stItemNo}, 
	{1, LIN, "edItemNo", 	 7, 52, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "~~~~~~~~~~~~~~~~", " End Item No   :", "End Item No - Search Available", 
		YES, NO,  JUSTLEFT, "", "", local_rec.edItemNo}, 
	{1, LIN, "stDate",	 8, 17, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", " Start Date    :", "Rel - Release Date Pre-Rel - Expected Release Date - Defaults To 00/00/00",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.stDate}, 
	{1, LIN, "edDate",	 8, 52, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.sysDate, " End Date      :", "Rel - Release Date Pre-Rel - Expected Release Date - Defaults To Today",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.edDate}, 
	{1, LIN, "stCentre", 	 9, 17, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", " Start QC Cntr :", "Start QC Centre - Search Available", 
		YES, NO,  JUSTLEFT, "", "", local_rec.stCentre}, 
	{1, LIN, "edCentre", 	 9, 52, CHARTYPE, 
		"UUUU", "          ", 
		" ", "~~~~", " End QC Cntr   :", "End QC Centre - Search Available", 
		YES, NO,  JUSTLEFT, "", "", local_rec.edCentre}, 
	{1, LIN, "sortBy", 	 11, 17, CHARTYPE, 
		"U", "          ", 
		" ", "C", " Sort By       :", "C(ategory/Date, D)ate/Category, S(upplier/Date - Defaults To Category", 
		YES, NO,  JUSTLEFT, "CDS", "", local_rec.sortBy}, 
	{1, LIN, "sortByDesc", 	 11, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.sortByDesc}, 
	{1, LIN, "lpno", 	 13, 17, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No    :", "Printer Number - Search Available", 
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.lpno}, 

	{0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			(void);
void	CloseDB			(void);
void	SrchQcmr		(char *);
void	ProcessRecords	(void);
void	PrintHeading	(void);
void	PrintDetails	(void);
void	PrintTail		(void);
int		heading 		(int);

extern	int	qualityControlSrch;

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		QC_APPLY;
	char *	sptr;

	qualityControlSrch	=	TRUE;

	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
	{
		print_at (0, 0, mlQcMess001);
		return (EXIT_FAILURE);
	}

	/* Multi-Location available. */
	multLoc = atoi (get_env ("MULT_LOC"));

	strcpy (local_rec.sysDate, DateToString (TodaysDate()));

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |	
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		abc_selfield (inmr, "inmr_hhbr_hash");
		ProcessRecords ();
		abc_selfield (inmr, "inmr_id_no");
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

	open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no2");
	open_rec (qcln,  qcln_list, QCLN_NO_FIELDS, "qcln_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec (qcmr,  qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	open_rec (prmr,  prmr_list, PRMR_NO_FIELDS, "prmr_id_no");
	open_rec (exwo,  exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (qchr);
	abc_fclose (qcln);
	abc_fclose (inmr);
	abc_fclose (inlo);
	abc_fclose (qcmr);
	abc_fclose (prmr);
	abc_fclose (exwo);
	abc_fclose (sumr);

	SearchFindClose ();
	abc_dbclose (data);
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (mlQcMess002), (80 - strlen (ML (mlQcMess002))) / 2, 0, 1);
		line_at (1,0,80);

		box (0, 3, 80, 10);
		line_at (6, 1,79);
		line_at (10,1,79);
		line_at (12,1,79);

		line_at (20,0,80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_short );
		strcpy (err_str, ML (mlStdMess039));
		print_at (22, 0, err_str, comm_rec.est_no, comm_rec.est_short );
		strcpy (err_str, ML (mlStdMess099));
		print_at (22, 40, err_str,
			comm_rec.cc_no, comm_rec.cc_short);

		/*  reset this variable for new screen NOT page	*/
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("type"))
	{
		if (local_rec.type [0] == 'R')
			strcpy (local_rec.typeDesc, "Released");
		else
			strcpy (local_rec.typeDesc, "Pre-Released");
		DSP_FLD ("typeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sourceType"))
	{
		switch (local_rec.sourceType [0])
		{
		case	'A':
			strcpy (local_rec.sourceTypeDesc, ML ("All Orders"));
			break;
		case	'P':
			strcpy (local_rec.sourceTypeDesc, ML ("Purchase Order"));
			break;
		case	'W':
			strcpy (local_rec.sourceTypeDesc, ML ("Works Order"));
			break;
		case	'M':
			strcpy (local_rec.sourceTypeDesc, ML ("Manual Entry"));
			break;
		}
		DSP_FLD ("sourceTypeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("stItemNo") ||
		LCHECK ("edItemNo"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_SUCCESS);

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		if (LCHECK ("stItemNo"))
		{
			cc = FindInmr (comm_rec.co_no, local_rec.stItemNo, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.stItemNo);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
		}
		else
		{
			cc = FindInmr (comm_rec.co_no, local_rec.edItemNo, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.edItemNo);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		if (inmr_rec.qc_reqd [0] != 'Y')
		{
			print_mess (ML (mlQcMess003));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (((LCHECK ("stItemNo") && prog_status != ENTRY) ||
			LCHECK ("edItemNo")) &&
			strcmp (local_rec.stItemNo, local_rec.edItemNo) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (LCHECK ("stItemNo"))
			DSP_FLD ("stItemNo");
		else
			DSP_FLD ("edItemNo");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("stDate") ||
		LCHECK ("edDate"))
	{
		if (((LCHECK ("stDate") && prog_status != ENTRY) ||
			LCHECK ("edDate")) &&
			local_rec.stDate > local_rec.edDate)
		{
			char	tempDate [11];

			sprintf (tempDate, "%-10.10s", DateToString (local_rec.stDate));
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("stCentre") ||
		LCHECK ("edCentre"))
	{
		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_SUCCESS);

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		if (LCHECK ("stCentre"))
			strcpy (qcmr_rec.centre, local_rec.stCentre);
		else
			strcpy (qcmr_rec.centre, local_rec.edCentre);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (((LCHECK ("stCentre") && prog_status != ENTRY) ||
			LCHECK ("edCentre")) &&
			strcmp (local_rec.stCentre, local_rec.edCentre) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sortBy"))
	{
		switch (local_rec.sortBy [0])
		{
		case	'C':
			strcpy (local_rec.sortByDesc, ML ("Category & Date"));
			break;
		case	'D':
			strcpy (local_rec.sortByDesc, ML ("Date & Category"));
			break;
		case	'S':
			if (local_rec.sourceType [0] == 'A' ||
				local_rec.sourceType [0] == 'W' ||
				local_rec.sourceType [0] == 'M')
			{
				sprintf (err_str, ML(mlQcMess004), BELL);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.sortByDesc, ML ("Supplier & Date"));
			break;
		}
		DSP_FLD ("sortByDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("lpno");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);             
}

/*======================
| Search for QC Centre |
======================*/
void
SrchQcmr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Cntr", "#Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", key_val);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, key_val, strlen (key_val)))
	{
		if (save_rec (qcmr_rec.centre, qcmr_rec.description))
			break;

		cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		file_err (cc, "qcmr", "DBFIND");
}

void
ProcessRecords (
 void)
{
	char	szSort [256];
	char *	sptr;
	int		HASH,
			LINE,
			DT = 0;

	/* Read and load records into a sort file. */
	fsort = sort_open ("qchr");

	strcpy (qchr_rec.co_no, comm_rec.co_no);
	strcpy (qchr_rec.br_no, comm_rec.est_no);
	strcpy (qchr_rec.wh_no, comm_rec.cc_no);
	qchr_rec.hhbr_hash = 0L;
	strcpy (qchr_rec.qc_centre, local_rec.stCentre);
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qchr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qchr_rec.br_no, comm_rec.est_no) &&
		!strcmp (qchr_rec.wh_no, comm_rec.cc_no) &&
		strcmp (qchr_rec.qc_centre, local_rec.edCentre) <= 0)
	{
		/*-------------------------------------------------------
		| Check if the source type is within the selection.     |
		-------------------------------------------------------*/
		if (local_rec.sourceType [0] != 'A')
		{
			if ((local_rec.sourceType [0] == 'P' &&
				qchr_rec.source_type [0] != 'P') ||
				(local_rec.sourceType [0] == 'W' &&
				qchr_rec.source_type [0] != 'W') ||
				(local_rec.sourceType [0] == 'M' &&
				qchr_rec.source_type [0] != 'M'))
			{
				cc = find_rec (qchr, &qchr_rec, NEXT, "r");
				continue;
			}
		}

		/*-------------------------------------------------------
		| Check if the QC item is within the selected range.    |
		-------------------------------------------------------*/
		if ((cc = find_hash (inmr, &inmr_rec, EQUAL, "r", qchr_rec.hhbr_hash)))
			file_err (cc, "inmr", "DBFIND");
		if (strcmp (inmr_rec.item_no, local_rec.stItemNo) < 0 ||
			strcmp (inmr_rec.item_no, local_rec.edItemNo) > 0)
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}

		if (RELEASED)
		{
			qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
			qcln_rec.line_no	= 0;
			cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
			while (!cc &&
				qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
			{
				/*-------------------------------------------------------
				| The release date is compared with the start and end   |
				| dates, if greater than or less than these dates, the  |
				| record is invalid.                                    |
				-------------------------------------------------------*/
				if (qcln_rec.release_dt < local_rec.stDate ||
					qcln_rec.release_dt > local_rec.edDate)
				{
					cc = find_rec (qcln, &qcln_rec, NEXT, "r");
					continue;
				}

				/*-------------------------------------------------------
				| Sort in selected order.                               |
				-------------------------------------------------------*/
				switch (local_rec.sortBy [0])
				{
				case	CATEGORY:
					sprintf (szSort,
							 "%-11.11s %10ld %-16.16s %-4.4s %10ld %3d\n",
							 inmr_rec.category,
							 qcln_rec.release_dt,
							 inmr_rec.item_no,
							 qchr_rec.qc_centre,
							 qchr_rec.hhqc_hash,
							 qcln_rec.line_no);
					sort_save (fsort, szSort);
					break;
				case	DATE:
					sprintf (szSort,
							 "%10ld %-11.11s %-16.16s %-4.4s %10ld %3d\n",
							 qcln_rec.release_dt,
							 inmr_rec.category,
							 inmr_rec.item_no,
							 qchr_rec.qc_centre,
							 qchr_rec.hhqc_hash,
							 qcln_rec.line_no);
					sort_save (fsort, szSort);
					break;
				case	SUPPLIER:
					if (!find_hash (sumr, &sumr_rec, EQUAL,
						"r", qchr_rec.hhsu_hash))
					{
						sprintf (szSort,
								 "%-6.6s %10ld %-11.11s %-16.16s %-4.4s %10ld %3d\n",
								 sumr_rec.crd_no,
								 qcln_rec.release_dt,
								 inmr_rec.category,
								 inmr_rec.item_no,
								 qchr_rec.qc_centre,
								 qchr_rec.hhqc_hash,
								 qcln_rec.line_no);
						sort_save (fsort, szSort);
					}
					break;
				}

				cc = find_rec (qcln, &qcln_rec, NEXT, "r");
			}
		}
		else
		{
			/*-------------------------------------------------------
			| The expected release date is compared with the start  |
			| and end dates, if greater than or less than these     |
			| dates, record is invalid.                             |
			-------------------------------------------------------*/
			if (qchr_rec.exp_rel_dt < local_rec.stDate ||
				qchr_rec.exp_rel_dt > local_rec.edDate)
			{
				cc = find_rec (qchr, &qchr_rec, NEXT, "r");
				continue;
			}


			/*-------------------------------------------------------
			| Sort in selected order.                               |
			-------------------------------------------------------*/
			switch (local_rec.sortBy [0])
			{
			case	CATEGORY:
				sprintf (szSort,
						 "%-11.11s %10ld %-16.16s %-4.4s %10ld\n",
						 inmr_rec.category,
						 qchr_rec.exp_rel_dt,
						 inmr_rec.item_no,
						 qchr_rec.qc_centre,
						 qchr_rec.hhqc_hash);
				sort_save (fsort, szSort);
				break;
			case	DATE:
				sprintf (szSort,
						 "%10ld  %-11.11s %-16.16s %-4.4s %10ld\n",
						 qchr_rec.exp_rel_dt,
						 inmr_rec.category,
						 inmr_rec.item_no,
						 qchr_rec.qc_centre,
						 qchr_rec.hhqc_hash);
				sort_save (fsort, szSort);
				break;
			case	SUPPLIER:
				if (!find_hash (sumr, &sumr_rec, EQUAL,
					"r", qchr_rec.hhsu_hash))
				{
					sprintf (szSort,
							 "%-6.6s %10ld %-11.11s %-16.16s %-4.4s %10ld\n",
							 sumr_rec.crd_no,
							 qchr_rec.exp_rel_dt,
							 inmr_rec.category,
							 inmr_rec.item_no,
							 qchr_rec.qc_centre,
							 qchr_rec.hhqc_hash);
					sort_save (fsort, szSort);
				}
				break;
			}
		}
			
		cc = find_rec (qchr, &qchr_rec, NEXT, "r");
	}

	/* Sort the sort file in the appropriate order. */
	fsort = sort_sort (fsort, "qchr");

	abc_selfield (qchr, "qchr_hhqc_hash");
	sprintf (prevCategory, "%-11.11s", " ");

	/* Display or Print the required data. */
	PrintHeading ();
	sptr = sort_read (fsort);
	while (sptr)
	{
		HASH	= 45;
		LINE	= 56;
		if (RELEASED)
			DT		= 0;
		if (local_rec.sortBy [0] == SUPPLIER)
		{
			HASH	= 52;
			LINE	= 63;
			if (RELEASED)
				DT		= 7;
		}
		if (RELEASED &&
			local_rec.sortBy [0] == CATEGORY)
			DT		= 12;

		if (!find_hash (qchr, &qchr_rec, EQUAL, "r", atol (sptr + HASH)))
		{
			if (find_hash (inmr, &inmr_rec, EQUAL, "r", qchr_rec.hhbr_hash))
			{
				strcpy (inmr_rec.item_no, " ");
				strcpy (inmr_rec.description, " ");
			}

			if (find_hash (sumr, &sumr_rec, EQUAL, "r", qchr_rec.hhsu_hash))
				strcpy (sumr_rec.crd_name, " ");

			if (RELEASED)
			{
				qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
				qcln_rec.release_dt	= atol (sptr + DT);
				qcln_rec.line_no	= atoi (sptr + LINE);
				if (find_rec (qcln, &qcln_rec, EQUAL, "r"))
				{
					sptr = sort_read (fsort);
					continue;
				}

				strcpy (prmr_rec.co_no, comm_rec.co_no);
				strcpy (prmr_rec.br_no, comm_rec.est_no);
				strcpy (prmr_rec.code, qcln_rec.emp_code);
				if (find_rec (prmr, &prmr_rec, EQUAL, "r"))
					strcpy (prmr_rec.name, qcln_rec.emp_code);

				if (qchr_rec.rej_qty > 0.00)
				{
					strcpy (exwo_rec.co_no, comm_rec.co_no);
					strcpy (exwo_rec.code, qcln_rec.reason);
					if ((cc = find_rec (exwo, &exwo_rec, EQUAL, "r")))
						strcpy (exwo_rec.description, qcln_rec.reason);
				}
			}

			PrintDetails ();
		}
		strcpy (prevCategory, inmr_rec.category);

		sptr = sort_read (fsort);
	}
	sort_delete (fsort, "qchr");

	abc_selfield (qchr, "qchr_id_no");

	PrintTail ();
}

void
PrintHeading (
 void)
{
	char	tempDate [11];

	sprintf (tempDate, "%-10.10s", DateToString (local_rec.stDate));
	clear ();

	if (RELEASED)
		dsp_screen (" Print Released QC Records Report ",
					comm_rec.co_no, comm_rec.co_name);
	else
		dsp_screen (" Print Pre-Released QC Records Report ",
					comm_rec.co_no, comm_rec.co_name);

	/* Open print file. */
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".15\n");
	fprintf (fout, ".L158\n");
	if (RELEASED)
		fprintf (fout, ".E RELEASED QC RECORDS REPORT \n");
	else
		fprintf (fout, ".E PRE-RELEASED QC RECORDS REPORT \n");

	fprintf (fout, ".E COMPANY : %s %s \n",
			clip (comm_rec.co_no), clip (comm_rec.co_name));
	fprintf (fout, ".E BRANCH : %s %s \n",
			clip (comm_rec.est_no), clip (comm_rec.est_name));
	fprintf (fout, ".E WAREHOUSE : %s %s \n",
			clip (comm_rec.cc_no), clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E START ITEM : %-16.16s END ITEM : %-16.16s \n",
			clip (local_rec.stItemNo), clip (local_rec.edItemNo));
	if (RELEASED)
		fprintf (fout, ".E START REL DATE : %s END REL DATE : %s \n",
				tempDate, DateToString (local_rec.edDate));
	else
		fprintf (fout, ".E START EXP REL DATE : %s END EXP REL DATE : %s \n",
				tempDate, DateToString (local_rec.edDate));
	fprintf (fout, ".E START QC CNTR : %-4.4s END QC CNTR : %-4.4s \n",
			clip (local_rec.stCentre), clip (local_rec.edCentre));
	fprintf (fout, ".E SOURCE TYPE : %s \n", clip (local_rec.sourceTypeDesc));
	fprintf (fout, ".E SORT BY : %s (%s) \n",
			clip (local_rec.sortByDesc),
			(RELEASED) ? "Released" : "Expected Release");
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
PrintDetails (
 void)
{
	char	sourceType [15];

	dsp_process ("Item No :", inmr_rec.item_no);

	if (qchr_rec.source_type [0] == 'P')
		strcpy (sourceType, ML ("Purchase Order"));
	else if (qchr_rec.source_type [0] == 'W')
		strcpy (sourceType, ML ("Works Order"));
	else
		strcpy (sourceType, ML ("Manual Entry"));

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

	if (strcmp (prevCategory, inmr_rec.category))
	{
		fprintf (fout, "| ITEM CATEGORY : %-11.11s",	inmr_rec.category);
		fprintf (fout, "%-128.128s",					" ");
		fprintf (fout, "|\n");
	}
		
	inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
	cc = find_rec ("inlo", &inlo_rec, COMPARISON, "r");
	if (cc)
		memset (&inlo_rec, 0, sizeof (inlo_rec));

	fprintf (fout, "|  %-4.4s  ",		qchr_rec.qc_centre);
	fprintf (fout, "|  %-16.16s  ",		inmr_rec.item_no);
	fprintf (fout, "|  %-40.40s      ",	inmr_rec.description);
	fprintf (fout, "| %-10.10s ",		DateToString (qchr_rec.receipt_dt));
	fprintf (fout, "| %-10.10s ",		DateToString (qchr_rec.exp_rel_dt));
	fprintf (fout, "|  %-7.7s  ",		inlo_rec.lot_no);
	fprintf (fout, "|  %-7.7s  ",		inlo_rec.slot_no);
	fprintf (fout, "| %-10.10s ",		DateToString (inlo_rec.expiry_date));
	fprintf (fout, "|  %-10.10s  ",		inlo_rec.location);
	fprintf (fout, "|\n");

	if (RELEASED)
	{
		fprintf (fout, "| SUPPLIER : %-32.32s ",	sumr_rec.crd_name);
		fprintf (fout, "SOURCE TYPE  : %-14.14s ",	sourceType);
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
		fprintf (fout, "LINE NO      : %3d%-8.8s ", qcln_rec.line_no, " ");
		fprintf (fout, "|\n");

		fprintf (fout, "| REMARKS  : %-30.30s   ",	qcln_rec.remarks);
		fprintf (fout, "RELEASED QTY : %11.3f    ",	qcln_rec.rel_qty);
		fprintf (fout, "REJECTED QTY : %11.3f ",	qcln_rec.rej_qty);
		fprintf (fout, "%-54.54s",					" ");
		fprintf (fout, "|\n");

		if (qcln_rec.rej_qty > 0.00)
		{
			double	value = qcln_rec.cost * qcln_rec.rej_qty;

			fprintf (fout, "| REASON   : %-2.2s %-20.20s%-9.9s ",
					 qcln_rec.reason, exwo_rec.description, " ");
			fprintf (fout, "DR WOF ACC:%-16.16s ",	qcln_rec.wof_acc);
			fprintf (fout, "CR STK ACC:%-16.16s",
					 qcln_rec.stk_wof_acc);
			fprintf (fout, "COST/UNIT    : %10.2f  ",
					 DOLLARS (qcln_rec.cost));
			fprintf (fout, "TOTAL COST   : %10.2f  ",		DOLLARS (value));
			fprintf (fout, "|\n");
		}
	}
	else
	{
		fprintf (fout, "| SUPPLIER : %-40.40s ",	sumr_rec.crd_name);
		fprintf (fout, "SOURCE TYPE : %-14.14s ",	sourceType);
		fprintf (fout, "ORIGINAL REC : %11.3f ",	qchr_rec.origin_qty);
		fprintf (fout, "%-47.47s",					" ");
		fprintf (fout, "|\n");
	}
}

void
PrintTail (
 void)
{
	/* Close print file */
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

