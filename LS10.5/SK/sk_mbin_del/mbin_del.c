/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: mbin_del.c,v 5.2 2001/08/09 09:19:06 scott Exp $
|  Program Name  : (sk_mbin_del.c )                                   |
|  Program Desc  : (Stock Delete Of Multiple Bin Locations.     )     |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 03/04/89         |
|---------------------------------------------------------------------|
| $Log: mbin_del.c,v $
| Revision 5.2  2001/08/09 09:19:06  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:20  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:48  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 11:00:03  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:13  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:13  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/07/10 01:53:40  scott
| Updated to replace "@(" with "@(" to ensure psl_what works correctly
|
| Revision 1.14  2000/06/13 05:03:06  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.13  2000/02/14 07:23:43  scott
| Updated to correct programs that update database tables without locking the record. This does not cause a problem with the character based code but causes a problem with GVision.
|
| Revision 1.12  2000/01/21 02:21:38  cam
| Changes for GVision compatibility.  Fixed print_mess () calls.
|
| Revision 1.11  1999/11/11 05:59:50  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.10  1999/11/08 02:50:28  ana
| (08/11/1999) SC2030 Modified condition in inlo_qty for inlo deletion.
|
| Revision 1.9  1999/11/03 07:32:10  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.8  1999/10/13 02:42:02  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.7  1999/10/08 05:32:33  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/07/16 00:18:51  scott
| Updated for abc_delete
|
| Revision 1.5  1999/06/20 05:20:16  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|$Log: mbin_del.c,v $
|Revision 5.2  2001/08/09 09:19:06  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:45:20  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:16:32  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:37:48  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.2  2000/12/21 11:00:03  ramon
|Updated to correct the errors when compiled in LS10-GUI.
|
|Revision 3.1  2000/11/20 07:40:13  scott
|New features related to 3PL environment
|New features related to Number Plates
|All covered in release 3 notes
|
|Revision 3.0  2000/10/10 12:20:30  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 09:11:13  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.15  2000/07/10 01:53:40  scott
|Updated to replace "@(" with "@(" to ensure psl_what works correctly
|
|Revision 1.14  2000/06/13 05:03:06  scott
|New Search routine that allow multi level searches using
|Item number, description, Alpha code, alternate no, maker number, selling group,
|buying group using AND or OR searches.
|New search beings routines into the library to speed up GVision.
|See seperate release notes.
|
|Revision 1.13  2000/02/14 07:23:43  scott
|Updated to correct programs that update database tables without locking the record. This does not cause a problem with the character based code but causes a problem with GVision.
|
|Revision 1.12  2000/01/21 02:21:38  cam
|Changes for GVision compatibility.  Fixed print_mess () calls.
|
|Revision 1.11  1999/11/11 05:59:50  scott
|Updated to remove display of program name as ^P can be used.
|
|Revision 1.10  1999/11/08 02:50:28  ana
| (08/11/1999) SC2030 Modified condition in inlo_qty for inlo deletion.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mbin_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mbin_del/mbin_del.c,v 5.2 2001/08/09 09:19:06 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

int	upd_ok   = 0;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;
struct lomrRecord	lomr_rec;

	char	*date  	= "data",
			*inum2  = "inum2";

#include	<LocHeader.h>

char    comp_amt [15];
int     before, after;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	prev_item [17];
	char	idesc [41];
	char	ldesc [41];
	char	locn [11];
	char	lot_no [7];
	char	dflt_qty [15];
	char	rep_qty [10];
	char	comp_qty [10];
	char	UOM [5];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "item_no", 4, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "i_desc", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Description", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.idesc}, 
	{1, LIN, "UOM",	 7, 18, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.UOM},
	{1, LIN, "locn", 8, 18, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Location  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.locn}, 
	{1, LIN, "lot", 8, 18, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.lot_no}, 
	{1, LIN, "l_desc", 9, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description  ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.ldesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
int  	spec_valid 		(int);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadMisc 		(void);
void 	SrchInum 		(char *);
void 	Update 			(void);
int  	FindIncc 		(void);
int  	heading 		(int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	mult_loc = 0;

	char	*sptr;

	sptr = chk_env ("MULT_LOC");
	if (sptr != (char *)0)
		mult_loc = atoi (sptr);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		 strcpy (local_rec.dflt_qty, "NNNNNN.NNNNN");
	else
		 strcpy (local_rec.dflt_qty, sptr);

	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)         
	   after = (int) ( (sptr + strlen (sptr) - 1) - sptr);
	else                  
	   after = 0;
	if (after == 0)   
	  	sprintf (local_rec.rep_qty, "%%%df", before);
	else              
	 	sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	if (after == 0)     
		sprintf (local_rec.comp_qty, "%%%df", before); 
	else   
		sprintf (local_rec.comp_qty, "%%%d.%df", before, after + 1);
																					sprintf (comp_amt, local_rec.comp_qty, 1/pow (10,after) - 1/pow (10,after + 1));
	
	init_scr ();

	
	if (!mult_loc)
	{
		no_option ("MULT_LOC (Multi Bin Locations)");
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();


	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		upd_ok 		= 0;
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		init_vars (1);
		heading (1);
		entry (1);
		if (prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		if (upd_ok)
			Update ();
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
		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("i_desc");

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		if (FindIncc ())
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.idesc,inmr_rec.description);
		DSP_FLD ("i_desc");

		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Unit of Measure |
	--------------------------*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
			strcpy (local_rec.UOM, inmr_rec.sale_unit);

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("locn"))
	{
		char	str_amt [15];

		abc_selfield ("inlo", "inlo_inlo_hash");

		if (SRCH_KEY)
		{
			SearchLOC (TRUE, incc_rec.hhwh_hash, temp_str);
			return (EXIT_SUCCESS);
		}

		cc = FindLocation 
		 (
			incc_rec.hhwh_hash, 
			inum2_rec.hhum_hash,	
			local_rec.locn,
			ValidLocations,
			&inlo_rec.inlo_hash
		);
		if (cc)
		{
			sprintf (err_str,ML (mlSkMess534),local_rec.locn, inum2_rec.uom);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inlo, "DBFIND");

  		/*--------------------------------------------------------------
		| Used env var SK_QTY_MASK to coincide with the display of qty |
		| in the location in Stock Display. Used string because        |
		| comparison of two floats sometimes does not work.            |
		---------------------------------------------------------------*/
		sprintf (str_amt, local_rec.comp_qty, fabs (n_dec (inlo_rec.qty, after)));
		if (strcmp (str_amt, comp_amt) > 0)
		{
		    if (inlo_rec.cnv_fct == 0.00)
			  	inlo_rec.cnv_fct = 1.00;
		 	sprintf (str_amt, local_rec.rep_qty, inlo_rec.qty/inlo_rec.cnv_fct);
			sprintf (err_str,ML ("Cannot delete location since %s %s of items still on hand"),str_amt,inlo_rec.uom);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = CheckLocation 
			(
				ccmr_rec.hhcc_hash, 
				local_rec.locn, 
				lomr_rec.loc_type
			);
		if (cc)
			strcpy (local_rec.ldesc,"No Description found");
		else
		{
			lomr_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			strcpy (lomr_rec.location, local_rec.locn);
			cc = find_rec (lomr, &lomr_rec, COMPARISON, "r");
			if (!cc)
				strcpy (local_rec.ldesc,lomr_rec.desc);
		}
		strcpy (local_rec.lot_no, local_rec.locn);
		upd_ok = TRUE;

		DSP_FLD ("l_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	ReadMisc ();

	abc_alias (inum2, inum);

	open_rec ("inmr",inmr_list,INMR_NO_FIELDS,"inmr_id_no");
	open_rec ("incc",incc_list,INCC_NO_FIELDS,"incc_id_no");

	open_rec  (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec  (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec  (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec  (lomr,  lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	OpenLocation (ccmr_rec.hhcc_hash);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inum);
	abc_fclose ("inmr");
	abc_fclose ("insc");
	abc_fclose ("lomr");
	abc_fclose ("inlo");
	CloseLocation ();
	SearchFindClose ();
	abc_dbclose ("data");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("ccmr",ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose ("ccmr");
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char *key_val)
{
	work_open ();
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}
		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

void
Update (
 void)
{
	abc_selfield ("inlo", "inlo_mst_id");

	inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inlo_rec.hhum_hash	=	inum2_rec.hhum_hash;
	strcpy (inlo_rec.location, local_rec.locn);
	strcpy (inlo_rec.lot_no,   local_rec.lot_no);
	cc = find_rec ("inlo", &inlo_rec, COMPARISON, "u");
	if (!cc)
	{
		cc = abc_delete ("inlo");
		if (cc)
			file_err (cc, "inlo", "DBDELETE");
	}
	else
		abc_unlock ("inlo");
	strcpy (local_rec.prev_item,local_rec.item_no);
}

int
FindIncc (
 void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	return (find_rec ("incc",&incc_rec,COMPARISON,"r"));
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSkMess489),24,0,1);
		print_at (0,53,ML (mlSkMess089),clip (local_rec.prev_item));
		move (0,1);
		line (80);

		move (1,6);
		line (79);
		box (0,3,80,6);

		move (0,20);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,clip (comm_rec.co_short));
		strcpy (err_str,ML (mlStdMess039));
		print_at (21,40,err_str,comm_rec.est_no,clip (comm_rec.est_short));
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
