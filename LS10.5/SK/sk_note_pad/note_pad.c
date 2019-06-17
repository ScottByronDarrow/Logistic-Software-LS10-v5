/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: note_pad.c,v 5.2 2001/08/09 09:19:21 scott Exp $
|  Program Name  : (sk_note_pad.c)
|  Program Desc  : (Stock Note Pad Input Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/07/87         |
|---------------------------------------------------------------------|
| $Log: note_pad.c,v $
| Revision 5.2  2001/08/09 09:19:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:25  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: note_pad.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_note_pad/note_pad.c,v 5.2 2001/08/09 09:19:21 scott Exp $";

#define MAXSCNS 	2
#define MAXLINES	100

#define	NORMAL 		 (note_type [0] == 'N')
#define	SERIAL 		 (note_type [0] == 'S')

#define	SLEEP_TIME	2

#define	TXT_REQD
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <Costing.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		first_time,	
			wk_line = 0,
			f_flag = 0,
			new_note = 0,
			envDbCo = 0;

	double	c_left = 0.00;

	char	note_type [2];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct innhRecord	innh_rec;
struct inndRecord	innd_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;

/*===========================
| Local & Screen Structures |
===========================*/
struct {					/*---------------------------------------*/
	char	dummy [11];		/*| Dummy Used In Screen Generator.		|*/
	char	comment [61];    /*| Holds Comments for each line.       |*/
	char	ser_no [26];     /*| Holds Serial Number.                |*/
	char	dsp_desc [17];   /*| Holds Serial Number.                |*/
							/*|_____________________________________|*/
} local_rec;            
		
static	struct	var	vars []	={	

	{1, LIN, "item_no", 4, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number.", " ", 
		YES, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{1, LIN, "desc", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Description ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{1, LIN, "ser_no", 6, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", local_rec.dsp_desc, " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.ser_no}, 
	{2, TXT, "comment", 8, 9, 0, 
		"", "          ", 
		" ", " ", "                    C O M M E N T S                       ", " ", 
		10, 60, 100, "", "", local_rec.comment}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ReadMisc 		 (void);
int  	spec_valid 		 (int);
int  	GetInnh 		 (long);
void 	Update 			 (void);
void 	PrintCoStuff 	 (void);
int  	FindIncc 		 (void);
int  	heading 		 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);

	tab_row = 8;
	tab_col = 6;

	if (argc != 2)
	{
		print_at (0,0,mlSkMess630, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (note_type, "%1.1s", argv [1]);
	
	if (!NORMAL && !SERIAL)
	{
		print_at (0,0,mlSkMess630, argv [0]);
		return (EXIT_FAILURE);
	}
	if (NORMAL)
	{
		strcpy (local_rec.dsp_desc, "              ");
		FLD ("ser_no")	= ND;
	}
	else
	{
		strcpy (local_rec.dsp_desc, "Serial Number.");
		FLD ("ser_no")	= YES;
	}

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok 	= 1;
		entry_exit 	= 0;
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		first_time 	= 0;
		lcount [2] 	= 0;
		init_vars (1);	
		init_vars (2);	

		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		scn_display (2);

		if (new_note == 1) 
			entry (2);
		else
			edit (2);

		if (restart)
			continue;

		Update ();

	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	ReadMisc ();

	open_rec (innd, innd_list, INND_NO_FIELDS, "innd_id_no");
	open_rec (innh, innh_list, INNH_NO_FIELDS, "innh_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (innd);
	abc_fclose (innh);
	abc_fclose (incc);
	abc_fclose (insf);
	abc_fclose (inmr);
	SearchFindClose ();
	CloseCosting ();
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

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

	abc_fclose (ccmr);
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

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
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
		DSP_FLD ("desc");

		/*----------------------------------------------------------
		| Check if Item is a serial item and input type is serial. |
		----------------------------------------------------------*/
		if (inmr_rec.serial_item [0] == 'Y' && SERIAL)
		{
			if (FindIncc ())
			{
				print_mess (ML (mlSkMess192));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		/*--------------------------------------------------------------
		| Check if Item is not a serial item and input type is serial. |
		--------------------------------------------------------------*/
		if (inmr_rec.serial_item [0] != 'Y' && SERIAL)
		{
			sprintf (err_str, ML (mlSkMess560), inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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

		if (SERIAL)
			return (EXIT_SUCCESS);

		abc_selfield (innh, "innh_id_no");

		strcpy (innh_rec.co_no ,comm_rec.co_no);
		innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (innh_rec.serial_no, "%25.25s", " ");
		cc = find_rec (innh, &innh_rec, COMPARISON, "r");
		if (!cc)
		{
			entry_exit = 1;
			new_note = 0;
			cc = GetInnh (innh_rec.hhnh_hash);
			sprintf (innh_rec.serial_no, "%25.25s", " ");
			if (cc)
				return (EXIT_SUCCESS);
		}
		if (cc)
		new_note = 1;

		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Serial Number. |
	-------------------------*/
	if (LCHECK ("ser_no"))
	{
		if (!SERIAL)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchInsf (incc_rec.hhwh_hash, "F", temp_str);
			return (EXIT_SUCCESS);
		}

		cc = FindInsf (incc_rec.hhwh_hash, 0L, local_rec.ser_no,"F", "r");
		if (cc)
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	    
		abc_selfield (innh, "innh_id_no2");

		innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
		innh_rec.hhwh_hash = incc_rec.hhwh_hash;
		strcpy (innh_rec.serial_no, local_rec.ser_no);
		cc = find_rec (innh, &innh_rec, COMPARISON, "r");
		if (!cc)
		{
			entry_exit = 1;
			new_note = 0;
			cc = GetInnh (innh_rec.hhnh_hash);
			if (cc)
			{
				restart = 1;
				return (EXIT_SUCCESS);
			}
		}
		else
			new_note = 1;

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============================================
| Read detail lines from note-pad detail file. |
==============================================*/
int
GetInnh (
 long hhnh_hash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	innd_rec.hhnh_hash = hhnh_hash;
	innd_rec.line_no = 0; 

	cc = find_rec (innd, &innd_rec, GTEQ, "r");
	while (!cc && innd_rec.hhnh_hash == hhnh_hash)
	{
		strcpy (local_rec.comment, innd_rec.comments);

		putval (lcount [2]++);

		cc = find_rec (innd, &innd_rec, NEXT, "r");
	}
	scn_set (1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*===================
| Update all files. |
===================*/
void
Update (
 void)
{
	int 	add_item = FALSE;

	if (new_note == 1) 
	{
		/*rv_pr ("Now Creating New Note Pad Record.", 0,2,1);*/
		rv_pr (ML (mlStdMess035), 0,2,1);
		strcpy (innh_rec.co_no, comm_rec.co_no);
		innh_rec.hhbr_hash = inmr_rec.hhbr_hash;
		innh_rec.hhwh_hash = insfRec.hhwh_hash;
		innh_rec.hhnh_hash = 0L;
		strcpy (innh_rec.serial_no, local_rec.ser_no);
		strcpy (innh_rec.stat_flag, "0");
		cc = abc_add (innh,&innh_rec);
		if (cc) 
			file_err (cc, innh, "DBADD");

		cc = find_rec (innh, &innh_rec,COMPARISON, "r");
		if (cc)
			return;

	}
	else
		rv_pr (ML (mlStdMess035), 0,2,1);
		/*rv_pr ("Now Updating Note Pad Record.", 0,2,1);*/

	scn_set (2);
	for (wk_line = 0;wk_line < lcount [2];wk_line++) 
	{
		getval (wk_line);

		innd_rec.hhnh_hash = innh_rec.hhnh_hash;
		innd_rec.line_no = wk_line;
		cc = find_rec (innd, &innd_rec, COMPARISON, "u");
		if (cc)
		   	add_item = TRUE;
		else
		   	add_item = FALSE;

		strcpy (innd_rec.comments, local_rec.comment);
		if (add_item)
		{
			putchar ('.');
			fflush (stdout);
			cc = abc_add (innd,&innd_rec);
			if (cc) 
				file_err (cc, innd, "DBADD");
		      		
			abc_unlock (innd);
		}
		else
		{
			/*------------------------
			| Update existing order. |
			------------------------*/
			cc = abc_update (innd,&innd_rec);
			if (cc) 
				file_err (cc, innd, "DBUPDATE");
		}
	}
	for (wk_line = lcount [2];wk_line < MAXLINES; wk_line++)
	{
		innd_rec.hhnh_hash = innh_rec.hhnh_hash;
		innd_rec.line_no = wk_line;
		cc = find_rec (innd, &innd_rec, COMPARISON, "r");
		if (!cc)
			abc_delete (innd);
		else
		 	break;
	}
	/*-------------------------------
	| Update existing order header. |
	-------------------------------*/
	if (new_note == 0) 
	{	
		/*-------------------------
		| Delete cancelled order. |
		-------------------------*/
		if (lcount [2] == 0) 
		abc_delete (innh);

		abc_unlock (innh);
	}
	move (0,2);
	cl_line ();
}

/*========================
| Print Company Details. |
========================*/
void
PrintCoStuff (
 void)
{
	line_at (20,0,80);
	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0, err_str, comm_rec.co_no,comm_rec.co_name);
	line_at (22,0,80);
}

int
FindIncc (
 void)
{
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	return (find_rec (incc,&incc_rec,COMPARISON,"r"));
}

/*================
| Print Heading. |
================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML (mlSkMess354),25,0,1);
		
		line_at (1,0,80);

		switch (scn)
		{
		case	1:
			scn_set (2);
			scn_display (2);

			box (0,3,80,3);
			break;

		case	2:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (0,3,80,3);
		}
		
		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
