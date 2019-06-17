/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_trace.c,v 5.3 2002/07/17 09:57:29 scott Exp $
|  Program Name  : (pc_trace.c   )                                    |
|  Program Desc  : (Print/Display Traceability of Lot-Controlled)     |
|                  (Items                                       )     |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 24/11/93         |
|---------------------------------------------------------------------|
| $Log: pc_trace.c,v $
| Revision 5.3  2002/07/17 09:57:29  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:27  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2000/12/21 03:31:13  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 3.2  2000/12/19 04:23:51  scott
| Updated to use correct index on inlo during open as fields did not exist in
| app.schema
|
| Revision 3.1  2000/11/20 07:39:07  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:09  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:17  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.14  2000/06/13 05:02:13  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.13  2000/03/07 09:32:28  ramon
| For GVision compatibility, I moved the position of the description fields 3 chars to the right.
|
| Revision 1.12  1999/11/19 04:44:14  scott
| Updated as program has a datejul and we missed it.
|
| Revision 1.11  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.10  1999/09/29 10:11:39  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 08:26:26  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.8  1999/09/13 07:03:19  marlene
| *** empty log message ***
|
| Revision 1.7  1999/09/09 06:12:36  marlene
| *** empty log message ***
|
| Revision 1.6  1999/06/17 07:40:48  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_trace.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_trace/pc_trace.c,v 5.3 2002/07/17 09:57:29 scott Exp $";

#define	X_OFF		2
#define Y_OFF		5

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>
#include	<LocHeader.h>

#define	SLEEP_TIME	2
#ifdef PSIZE
#undef PSIZE
#endif
#define	PSIZE	11

#define	PRINTER		 (local_rec.prt_disp [0] == 'P')
#define	BRANCH		 (local_rec.cobrwh [0] == 'B')
#define	WAREHOUSE	 (local_rec.cobrwh [0] == 'W')

char	*endLine = "^^GGJGGJGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGG"; 

#include	"schema"

struct commRecord	comm_rec;
struct pcltRecord	pclt_rec;
struct pcwoRecord	pcwo_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;
struct inumRecord	inum_rec;
struct inloRecord	inlo_rec;

	char	*data	= "data";

	FILE	*fout,
			*fsort;

	int		printed;
	int		foundLot;

	struct {
		char	est_no [3];
		char	cc_no [3];
		char	order_no [8];
		char	batch_no [11];
		char	item_no [17];
		char	item_desc [41];
		char	lot_no [8];
		char	location [11];
		long	date;
		float	qty_used;
		char	uom [5];
	} store_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	st_item [17];
	char	st_desc [41];
	char	ed_item [17];
	char	ed_desc [41];

	char	st_lot [8];
	char	ed_lot [8];

	char	cobrwh [2];
	char	cobrwh_desc [10];

	char	prt_disp [2];
	char	prt_disp_desc [8];
	int		lp_no;

	char 	dummy [11];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_item",	 4, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", " ", "Start Item               : ", "Start Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_desc",	 5, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", "Start Item Description   : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "ed_item",	 6, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", "~~~~~~~~~~~~~~~~", "End Item                 : ", "End Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ed_item},
	{1, LIN, "ed_desc",	 7, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", "End Item Description     : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ed_desc},
	{1, LIN, "cobrwh",	 9, 30, CHARTYPE,
		"U", "          ",
		" ", "C", "Company/Branch/Warehouse : ", "Company/Branch/Warehouse - Default : Company",
		YES, NO,  JUSTLEFT, "BCW", "", local_rec.cobrwh},
	{1, LIN, "cobrwh_desc",	 9, 33, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cobrwh_desc},
	{1, LIN, "st_lot",	 11, 30, CHARTYPE,
		"UUUUUUU", "           ",
		" ", " ", "Start Lot Number         : ", "Start Item Lot Number - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_lot},
	{1, LIN, "ed_lot",	 12, 30, CHARTYPE,
		"UUUUUUU", "           ",
		" ", "~~~~~~~", "End Lot Number           : ", "End Item Lot Number - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ed_lot},
	{1, LIN, "prt_disp",	 14, 30, CHARTYPE,
		"U", "          ",
		" ", "D", "Display or Print         : ", "Display or Print - Default : Display",
		YES, NO,  JUSTLEFT, "DP", "", local_rec.prt_disp},
	{1, LIN, "prt_disp_desc",	 14, 33, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.prt_disp_desc},
	{1, LIN, "lp_no",	15, 30, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No               : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lp_no},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| function prototypes |
=====================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
int 	spec_valid 				(int);
void 	ProcReport 				(void);
int 	ValidRecord 			(long);
void 	AddToSortFile 			(void);
void 	PrintDetails 			(void);
void 	PrintLine 				(void);
int 	heading 				(int);
void 	InitOutput 				(void);
void 	PrintHeading 			(void);
void 	DisplayHeading 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc,
 char *argv [])
{
	SETUP_SCR (vars);

	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		printed = FALSE;

		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		crsr_on ();

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		/*============================
		| Process Orders in Database.|
		============================*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcReport ();

		if (PRINTER)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		else
		{
			Dsp_srch ();
			Dsp_close ();
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
	
/*=========================
| Program exit sequence	. |
=========================*/
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (pclt, pclt_list, PCLT_NO_FIELDS, "pclt_hhwo_hash");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_id_lot");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");
	OpenLocation (ccmr_rec.hhcc_hash);

	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pclt);
	abc_fclose (pcwo);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (inum);
	abc_fclose (inlo);
	CloseLocation ();

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("st_item"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.st_desc, "%-40.40s", "First item");
			DSP_FLD ("st_desc");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.st_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");

			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		strcpy (local_rec.st_item, inmr_rec.item_no);

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_item, local_rec.ed_item) > 0)
		{
/*
			sprintf (err_str,
				"Start Item %s Greater Than End Item %s",
				local_rec.st_item,
				local_rec.ed_item);
*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");

			return (EXIT_FAILURE);
		}

		strcpy (local_rec.st_desc, inmr_rec.description);
		DSP_FLD ("st_desc");

		abc_selfield (inmr, "inmr_hhbr_hash");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ed_item"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.ed_desc, "%-40.40s", "Last item");
			DSP_FLD ("ed_desc");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.ed_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.ed_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("ed_desc");

			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.ed_item, inmr_rec.item_no);

		if (strcmp (local_rec.st_item, local_rec.ed_item) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("ed_desc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ed_desc, inmr_rec.description);
		DSP_FLD ("ed_desc");

		abc_selfield (inmr, "inmr_hhbr_hash");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_lot"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchLOC (FALSE, 0L, temp_str);
			return (EXIT_SUCCESS);
		}

		abc_selfield (inlo, "inlo_id_lot");

		inlo_rec.hhwh_hash		=	0L;
		inlo_rec.loc_type [0]	=	' ';
		strcpy (inlo_rec.lot_no, "       ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		foundLot = FALSE;
		while (!cc)
		{
			if (!strcmp (inlo_rec.lot_no, local_rec.st_lot))
			{
				foundLot = TRUE;
				break;
			}
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
		}
		if (cc || !foundLot)
		{
			print_mess (ML (mlPcMess070));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_lot, local_rec.ed_lot) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ed_lot"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchLOC (FALSE, 0L, temp_str);
			return (EXIT_SUCCESS);
		}

		inlo_rec.hhwh_hash		=	0L;
		inlo_rec.loc_type [0]	=	' ';
		strcpy (inlo_rec.lot_no, "       ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		foundLot = FALSE;
		while (!cc)
		{
			if (!strcmp (inlo_rec.lot_no, local_rec.ed_lot))
			{
				foundLot = TRUE;
				break;
			}
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
		}
		if (cc || !foundLot)
		{
		/*	sprintf (err_str, "Lot %s Not Exist", local_rec.ed_lot);*/
			print_mess (ML (mlPcMess070));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_lot, local_rec.ed_lot) > 0)
		{
/*
			sprintf (err_str, "Start Lot %s Greater Than End Lot %s",
				local_rec.st_lot,
				local_rec.ed_lot);
*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cobrwh"))
	{
		strcpy (local_rec.cobrwh_desc, "         ");
		DSP_FLD ("cobrwh_desc");
		switch (local_rec.cobrwh [0])
		{
		case	'C':
			strcpy (local_rec.cobrwh_desc, "Company  ");
			break;
		case	'B':
			strcpy (local_rec.cobrwh_desc, "Branch   ");
			break;
		case	'W':
			strcpy (local_rec.cobrwh_desc, "Warehouse");
			break;
		}
		DSP_FLD ("cobrwh_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prt_disp"))
	{
		*local_rec.prt_disp_desc = '\0';
		DSP_FLD ("prt_disp_desc");

		if (local_rec.prt_disp [0] == 'D')
		{
			strcpy (local_rec.prt_disp_desc, "Display");
			DSP_FLD ("prt_disp_desc");
			FLD ("lp_no") = NA;
			local_rec.lp_no = 0;
			DSP_FLD ("lp_no");
		}
		else
		{
			strcpy (local_rec.prt_disp_desc, "Print  ");
			DSP_FLD ("prt_disp_desc");
			FLD ("lp_no") = YES;
			if (prog_status != ENTRY)
			{
/*
				print_mess ("Please ensure a printer number has been entered.");
*/
				print_mess (ML (mlPcMess052));
				sleep (sleepTime);
				clear_mess ();
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lp_no"))
	{
		if (FLD ("lp_no") != NA) 
		{
			if (SRCH_KEY)
			{
				local_rec.lp_no = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lp_no))
			{
				/*print_mess ("Invalid Printer");*/
				print_mess (ML (mlStdMess020));
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
ProcReport (
 void)
{
	int		sortOpen = FALSE;

	if (PRINTER)
		PrintHeading ();
	else
		DisplayHeading ();

	/*----------------------
	| Process all records. |
	----------------------*/
	pclt_rec.hhwo_hash = 0L;
	cc = find_rec (pclt, &pclt_rec, GTEQ, "r");
	while (!cc)
	{
		/*------------------------------
		| Validate record : within the |
		| lot number range.            |
		------------------------------*/
		if (strcmp (local_rec.st_lot, pclt_rec.lot_number) > 0 ||
			strcmp (pclt_rec.lot_number, local_rec.ed_lot) > 0)
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		/*------------------------------
		| Validate record : within the |
		| item number range.           |
		------------------------------*/
		inmr_rec.hhbr_hash = pclt_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		if (strcmp (local_rec.st_item, inmr_rec.item_no) > 0 ||
			strcmp (inmr_rec.item_no, local_rec.ed_item) > 0)
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		/*------------------
		| read uom details |
		------------------*/
		inum_rec.hhum_hash	=	pclt_rec.iss_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		/*-----------------------
		| read works order file |
		-----------------------*/
		pcwo_rec.hhwo_hash	=	pclt_rec.hhwo_hash;
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		if (!ValidRecord (pcwo_rec.hhcc_hash))
		{
			cc = find_rec (pclt, &pclt_rec, NEXT, "r");
			continue;
		}

		if (!sortOpen)
		{
			fsort = sort_open ("trace");
			sortOpen = TRUE;
		}

		AddToSortFile ();

		cc = find_rec (pclt, &pclt_rec, NEXT, "r");
	}

	if (sortOpen)
	{
		fsort = sort_sort (fsort,"trace");

		PrintDetails ();

		sort_delete (fsort, "trace");

		sortOpen = FALSE;
	}

	/*------------------------------
	| nothing printed so tell user |
	------------------------------*/
	if (!printed)
	{
		if (PRINTER)
			fprintf (fout, ".E NO TIMESHEET'S FOUND FOR ANY EMPLOYEES \n");
	}
}

int
ValidRecord (
	long	hhccHash)
{
	ccmr_rec.hhcc_hash	=	hhccHash;
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	if (!BRANCH && !WAREHOUSE &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no))
		return (TRUE);

	if (BRANCH &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, comm_rec.est_no))
		return (TRUE);

	if (WAREHOUSE &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, comm_rec.est_no) &&
		!strcmp (ccmr_rec.cc_no, comm_rec.cc_no))
		return (TRUE);

	return (FALSE);
}

void
AddToSortFile (
 void)
{
	char	workSortString [200];

	sprintf (workSortString, "%-2.2s %-2.2s %-7.7s %-10.10s %-16.16s %-40.40s %-7.7s %-10.10s %-10.10s %14.6f %-4.4s\n",
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		pcwo_rec.order_no,
		pcwo_rec.batch_no,
		inmr_rec.item_no,
		inmr_rec.description,
		pclt_rec.lot_number,
		pclt_rec.lot_location,
		DateToString (pclt_rec.issue_date),
		n_dec (pclt_rec.qty_used, inmr_rec.dec_pt),
		inum_rec.uom);
	sort_save (fsort, workSortString);
}

void
PrintDetails (
 void)
{
	char	*sptr;

	sptr = sort_read (fsort);

	while (sptr)
	{
		sprintf (store_rec.est_no,		"%-2.2s",	sptr);
		sprintf (store_rec.cc_no,		"%-2.2s",	sptr + 3);
		sprintf (store_rec.order_no,	"%-7.7s",	sptr + 6);
		sprintf (store_rec.batch_no,	"%-10.10s",	sptr + 14);
		sprintf (store_rec.item_no,		"%-16.16s",	sptr + 25);
		sprintf (store_rec.item_desc,	"%-40.40s",	sptr + 42);
		sprintf (store_rec.lot_no,		"%-7.7s",	sptr + 83);
		sprintf (store_rec.location,	"%-10.10s",	sptr + 91);
		store_rec.date					= StringToDate (sptr + 102);
		store_rec.qty_used				= (float)atof (sptr + 113);
		sprintf (store_rec.uom,			"%-4.4s",	sptr + 128);

		PrintLine ();

		sptr = sort_read (fsort);
	}
	if (printed && !PRINTER)
		Dsp_saverec (endLine);
}

void
PrintLine (
 void)
{
	if (!printed)
		printed = TRUE;

	if (PRINTER)
	{
		dsp_process ("Works Order :", store_rec.order_no);

		fprintf (fout, "| %-2.2s ",		store_rec.est_no);
		fprintf (fout, "| %-2.2s ",		store_rec.cc_no);
		fprintf (fout, "| %-7.7s ",		store_rec.order_no);
		fprintf (fout, "| %-10.10s ",	store_rec.batch_no);
		fprintf (fout, "| %-16.16s ",	store_rec.item_no);
		fprintf (fout, "| %-40.40s ",	store_rec.item_desc);
		fprintf (fout, "| %-7.7s ",		store_rec.lot_no);
		fprintf (fout, "| %-10.10s ",	store_rec.location);
		fprintf (fout, "|%-10.10s",		DateToString (store_rec.date));
		fprintf (fout, "| %14.6f ",		store_rec.qty_used);
		fprintf (fout, "| %-4.4s |\n",	store_rec.uom);
	}
	else
	{
		sprintf (err_str,
			"%-2.2s^E%-2.2s^E%-7.7s^E%-10.10s^E%-16.16s^E%-36.36s^E%-7.7s^E%-10.10s^E%-10.10s^E%14.6f^E %-4.4s",
			store_rec.est_no,
			store_rec.cc_no,
			store_rec.order_no,
			store_rec.batch_no,
			store_rec.item_no,
			store_rec.item_desc,
			store_rec.lot_no,
			store_rec.location,
			DateToString (store_rec.date),
			store_rec.qty_used,
			store_rec.uom);
		Dsp_saverec (err_str);
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		snorm ();
		
		rv_pr (ML (mlPcMess075), 20, 0, 1);

		move (0, 1);
		line (80);

		box (0, 3, 80, 12);
		move (1, 8);
		line (79);
		move (1, 10);
		line (79);
		move (1, 13);
		line (79);

		move (0, 20);
		line (80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0, err_str,
			comm_rec.co_no, clip (comm_rec.co_short));
		strcpy (err_str, ML (mlStdMess039));
		print_at (21,27, err_str,
			comm_rec.est_no, clip (comm_rec.est_short));
		strcpy (err_str, ML (mlStdMess099));
		print_at (21,57, err_str,
			comm_rec.cc_no, clip (comm_rec.cc_short));

		move (0, 22);
		line (80);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	if (PRINTER)
	{
		dsp_screen (" Printing Traceability of Lot-Controlled Items ",
				comm_rec.co_no, 
				comm_rec.co_name);

		/*----------------------
		| Open pipe to pformat | 
		----------------------*/
		if ( (fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.lp_no);
		fprintf (fout, ".16\n");
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L154\n");
	}
	else
	{
		/*------------------------------------------------------
		| DISPLAY SCREEN.                                      |
		| Display Heading at screen after clearing the screen. |
		------------------------------------------------------*/
		clear ();
		swide ();

		rv_pr (ML (mlPcMess076), 40, 0, 1);

		print_at (2, 2,ML (mlPcMess045), clip (comm_rec.co_name));

		print_at (3, 2,ML (mlPcMess071), local_rec.st_item);
		print_at (3, 35,ML (mlPcMess072), local_rec.ed_item);
		print_at (4, 2,ML (mlPcMess073), local_rec.st_lot);
		print_at (4, 36,ML (mlPcMess074), local_rec.ed_lot);
		print_at (5, 2,ML (mlPcMess048), local_rec.cobrwh_desc);
	}
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".E TRACEABILITY OF LOT-CONTROLLED ITEMS \n");

	fprintf (fout, ".B1");

	fprintf (fout, ".E COMPANY NAME : %s \n", clip (comm_rec.co_name));
	fprintf (fout, ".E BRANCH NAME : %s \n", clip (comm_rec.est_name));
	fprintf (fout, ".E WAREHOUSE NAME : %s \n", clip (comm_rec.cc_name));

	fprintf (fout,
			".E START ITEM : %s  END ITEM : %s \n",
			local_rec.st_item,
			local_rec.ed_item);

	fprintf (fout,
			".E START LOT NO : %s  END LOT NO : %s \n",
			local_rec.st_lot,
			local_rec.ed_lot);

	fprintf (fout, ".E LEVEL : %s \n", local_rec.cobrwh_desc);

	fprintf (fout, ".B1\n");

	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===========");
	fprintf (fout, "=================");
	fprintf (fout, "========\n");

	fprintf (fout, "| BR ");
	fprintf (fout, "| WH ");
	fprintf (fout, "|  WORKS  ");
	fprintf (fout, "|   BATCH    ");
	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|             ITEM DESCRIPTION             ");
	fprintf (fout, "|   LOT   ");
	fprintf (fout, "|    BIN     ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|    QTY USED    ");
	fprintf (fout, "| UOM  |\n");

	fprintf (fout, "| NO ");
	fprintf (fout, "| NO ");
	fprintf (fout, "|  ORDER  ");
	fprintf (fout, "|     NO     ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|   NO    ");
	fprintf (fout, "|  LOCATION  ");
	fprintf (fout, "|  ISSUED  ");
	fprintf (fout, "|                ");
	fprintf (fout, "|      |\n");

	fprintf (fout, "|----");
	fprintf (fout, "|----");
	fprintf (fout, "|---------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|---------");
	fprintf (fout, "|------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------|\n");

	fprintf (fout, ".R=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===========");
	fprintf (fout, "=================");
	fprintf (fout, "========\n");
}

void
DisplayHeading (
 void)
{
	Dsp_nc_prn_open (0, 6, PSIZE, err_str,
			comm_rec.co_no, comm_rec.co_name,
			comm_rec.est_no, comm_rec.est_name,
			 (char *)0, (char *)0);

	Dsp_saverec ("BR|WH| WORKS |  BATCH   |  ITEM  NUMBER  |    ITEM DESCRIPTION                |  LOT  |   BIN    |    DATE   |   QTY USED   | UOM ");
	Dsp_saverec ("NO|NO| ORDER |    NO    |                |                                    |  NO   | LOCATION |  ISSUED  |              |     ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");
}



