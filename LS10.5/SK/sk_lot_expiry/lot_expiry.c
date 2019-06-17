/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lot_expiry.c,v 5.6 2001/11/05 01:40:44 scott Exp $
|  Program Name  : ( lot_expiry.c   )                                 |
|  Program Desc  : ( Lot Number and Expiry Date Amendment Program )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees | Date Written : 04/10/93          |
|---------------------------------------------------------------------|
| $Log: lot_expiry.c,v $
| Revision 5.6  2001/11/05 01:40:44  scott
| Updated from Testing.
|
| Revision 5.5  2001/10/22 07:14:14  scott
| Updated to fix looping problem when onny expiry date changed.
|
| Revision 5.4  2001/10/19 05:51:50  francis
| Checked in for Val.
|
| Revision 5.3  2001/10/16 02:54:56  robert
| Updated to fix memory dump on LS10-GUI
|
| Revision 5.2  2001/08/09 09:19:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:41  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 10:52:13  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:13  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:09  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  2000/06/13 05:03:02  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.7  1999/11/11 05:59:49  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.6  1999/11/03 07:32:08  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.5  1999/10/08 05:32:31  scott
| First Pass checkin by Scott.
|
| Revision 1.4  1999/06/20 05:20:13  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lot_expiry.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_lot_expiry/lot_expiry.c,v 5.6 2001/11/05 01:40:44 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;

	char	*data 	= "data",
			*inlo2 	= "inlo2";

/*============================ 
| Local & Screen Structures. |
============================*/
struct 
{
	char	item_no [17];
	char	lot_no [8];
	long	expiry_date;
	char	new_lot_no [8];
	long	new_expiry_date;
	char	dummy [11];
	char	prev_item [17];
	char	location [11];
	long	hhum_hash;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no", 3, 23, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Item no              : ", " Full Search Available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",	 4, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description          : ", "",
		 NA, NO, JUSTLEFT, "", "", inmr_rec.description},
	{2, LIN, "lot_no", 7, 23, CHARTYPE,
		"UUUUUUU", "          ",
		" ", "", " Existing Lot Number  : ", "Existing Lot Number for the Current Item - Search Available",
		YES, NO, JUSTLEFT, "", "", local_rec.lot_no},
	{2, LIN, "expiry_date", 8, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", " Existing Expiry Date : ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.expiry_date},
	{2, LIN, "new_lot_no", 10, 23, CHARTYPE,
		"UUUUUUU", "          ",
		" ", local_rec.lot_no, " New Lot Number       : ", "New Lot Number for the Current Item - Default : Existing Lot Number",
		YES, NO, JUSTLEFT, "", "", local_rec.new_lot_no},
	{2, LIN, "location", 10, 23, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.location},
	{2, LIN, "hhum_hash", 10, 23, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.hhum_hash},
	{2, LIN, "new_expiry_date", 11, 23, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", (char *)&local_rec.expiry_date, " New Expiry Date      : ", "Expiry Date for New Lot Number",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.new_expiry_date},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<LocHeader.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog 		(void);
void OpenDB 			(void);
void CloseDB 			(void);
void ReadCcmr 			(void);
int  spec_valid 		(int);
int  Update 			(void);
int  heading 			(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr ();	
	set_tty ();     
	set_masks ();

	OpenDB ();

	while (prog_exit == 0) 
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;

		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (2);
		entry (2);
		if (restart)
			continue;

		heading (2);
		scn_display (2);
		edit (2);

		if (restart)
			continue;

		while (TRUE)
		{
			cc = Update ();
			/*-------------------------------
			| if unsuccessful update, allow |
			| the user to re-enter data     |
			-------------------------------*/
			if (cc)
			{
				heading (2);
				scn_display (2);
				edit (2);
			}
			else
				break;

			if (restart)
				break;
		}

		if (!restart)
		{
			strcpy (local_rec.prev_item, local_rec.item_no);
			scn_set (1);
		}
	}
	shutdown_prog ();	
	return (EXIT_SUCCESS);
}

/*========================
| program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (inlo2, inlo);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	ReadCcmr ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_id_lot");
	open_rec (inlo2,inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inlo);
	abc_fclose (inlo2);
	CloseLocation();
	SearchFindClose ();
	abc_dbclose (data);
}

/*=========================================================
| Get ccmr record, for this company, branch and warehouse |
=========================================================*/
void
ReadCcmr (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
}

/*==========================================
| Primary validation and file access here. |
==========================================*/
int
spec_valid (
 int field)
{
	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (inmr_rec.lot_ctrl [0] != 'Y') /* Lot controlled items only */
		{
			print_mess (ML(mlSkMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return(0);
	}

	if (LCHECK ("lot_no"))
	{
		if (SRCH_KEY)
		{
			SearchLOC (FALSE, incc_rec.hhwh_hash, temp_str);
			return (EXIT_SUCCESS);
		}

		cc = 	FindLotNo 
				(
					incc_rec.hhwh_hash, 
					0L, 
					local_rec.lot_no, 
					&inlo2_rec.inlo_hash
				);
		if (cc)
		{
			sprintf (err_str, ML(mlSkMess202), local_rec.lot_no, local_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cc = find_rec (inlo2, &inlo2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inlo2, "DBFIND");

		local_rec.expiry_date = inlo2_rec.expiry_date;
		strcpy (local_rec.location, inlo2_rec.location);
		DSP_FLD ("lot_no");
		DSP_FLD ("expiry_date");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_lot_no"))
	{
		if (dflt_used || 
			!strcmp (local_rec.new_lot_no, local_rec.lot_no))
			return (EXIT_SUCCESS);

		cc = 	FindLotNo 
				(
					incc_rec.hhwh_hash, 
					0L, 
					local_rec.new_lot_no, 
					&inlo_rec.inlo_hash
				);
		if (!cc)
		{
			sprintf (err_str, ML(mlSkMess203), local_rec.new_lot_no,
				local_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		DSP_FLD ("new_lot_no");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_expiry_date"))
	{
		if (dflt_used)
			local_rec.new_expiry_date = local_rec.expiry_date;
		DSP_FLD ("new_expiry_date");
		return (EXIT_SUCCESS);
	}

	return(0);
}

/*===============================================
| Update inlo records. Return 1 if not updated, |
| 0 if records updated.                         |
===============================================*/
int
Update (void)
{
	if (strcmp (local_rec.lot_no, local_rec.new_lot_no) ||
		local_rec.expiry_date != local_rec.new_expiry_date)
	{
		clear ();
		print_at (0,0, ML(mlStdMess035));
		fflush (stdout);

		cc = 	FindLotNo 
				(
					incc_rec.hhwh_hash, 
					0L, 
					local_rec.lot_no, 
					&inlo_rec.inlo_hash
				);
		if (cc)
			return (EXIT_FAILURE);

		abc_selfield (inlo, "inlo_inlo_hash");

		cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inlo, "DBFIND");

		while (!cc && !strcmp (inlo_rec.lot_no, local_rec.lot_no) &&
				inlo_rec.expiry_date == local_rec.expiry_date)
		{

			strcpy (inlo_rec.lot_no, local_rec.new_lot_no);
			inlo_rec.expiry_date = local_rec.new_expiry_date;

			cc = abc_update (inlo, &inlo_rec); 
			if (cc)
				file_err (cc, inlo, "DBUPDATE");

			cc = 	FindLotNo 
					(
						incc_rec.hhwh_hash, 
						0L, 
						local_rec.lot_no, 
						&inlo_rec.inlo_hash
					);

			abc_selfield (inlo, "inlo_inlo_hash");

			cc = find_rec (inlo, &inlo_rec, EQUAL, "u");
			if (cc)
				file_err (cc, inlo, "DBFIND");
		}
		abc_unlock (inlo);
	}
	else
	{
		/*------------------------------
		| No Updating of inlo records. |
		| Allow user to re-edit.       |
		------------------------------*/
		print_mess (ML(mlSkMess204));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		clear ();
        print_at (0,56, ML(mlSkMess089), local_rec.prev_item);

		line_at (1,0,80);

		rv_pr (ML(mlSkMess205) , 20, 0, 1);
		box (0, 2, 80, 2);
		scn_write (1);
		scn_display (1);

		if (scn != cur_screen)
			scn_set (scn);

		if (scn == 2)
		{
			box (0, 6, 80, 5);
			line_at (9,1,79);
		}

		line_at (20,0,80);

		print_at(21,0, ML(mlStdMess038), 
			comm_rec.co_no,
			clip (comm_rec.co_name));

		line_at (22,0,80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
