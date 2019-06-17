/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_ltd_inpt.c  )                                 |
|  Program Desc  : ( Life to Date Sales Maintenance               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 29/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (26/05/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (22/02/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/10/97)      | Modified  by  : Marnie Organo    |
|                                                                     |
|  Comments      : (03/08/90) - General Update for New Scrgen. S.B.D. |
|                : (22/02/91) - Added alternate code.                 |
|                : (01/10/97) - Updated for Multilingual Conversion . |
|                :                                                    |
|                                                                     |
| $Log: ltd_inpt.c,v $
| Revision 5.2  2001/08/09 09:19:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:10  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  2000/07/10 01:53:40  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.7  2000/06/13 05:03:05  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.6  1999/11/11 05:59:49  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.5  1999/11/03 07:32:08  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.4  1999/10/08 05:32:32  scott
| First Pass checkin by Scott.
|
| Revision 1.3  1999/06/20 05:20:14  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ltd_inpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ltd_inpt/ltd_inpt.c,v 5.2 2001/08/09 09:19:02 scott Exp $";


#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
				    
#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	item_no [17];
	char	prev_item [17];
	float	ltd_sales;
	char	dummy [11];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "item_no", 5, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Item Number.     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "desc", 6, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Description.", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{1, LIN, "ltd_sale", 8, 22, FLOATTYPE, 
		"NNNNNNNNN.NN", "          ", 
		"0", "0", "Life To Date Sales.", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.ltd_sales}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
int  spec_valid (int field);
void OpenDB (void);
void CloseDB (void);
void update (void);
int  heading (int scn);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		edit_exit = 0;

		init_vars (1);
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		update ();
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
	/*------------------------
	| Validate Item Number . |
	------------------------*/
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
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		if (inmr_rec.ltd_sales != 0.00)
		{
			entry_exit = 1;
			local_rec.ltd_sales = inmr_rec.ltd_sales;
			DSP_FLD ("ltd_sale");
		}

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

	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("inmr");
	SearchFindClose ();
	abc_dbclose ("data");
}

void
update (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,local_rec.item_no);
	cc = find_rec ("inmr", &inmr_rec, COMPARISON, "u");
	if (cc)
		sys_err ("Error in inmr During (DBFIND)",cc, PNAME);

	inmr_rec.ltd_sales = local_rec.ltd_sales;
	cc = abc_update ("inmr",&inmr_rec);
	if (cc)
		sys_err ("Error in inmr During (DBUPDATE)",cc, PNAME);

	abc_unlock ("inmr");
	strcpy (local_rec.prev_item,inmr_rec.item_no);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
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
		rv_pr (ML (mlSkMess357),24,0,1); 
		print_at (0,53, ML (mlSkMess096),clip (local_rec.prev_item));
		move (0,1);
		line (80);

		move (1,7);
		line (79);
		box (0,4,80,4);

		move (0,20);
		line (80);
		strcpy (err_str,  ML (mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no,clip (comm_rec.co_short));
		strcpy (err_str,  ML (mlStdMess039));
		print_at (21,45, err_str,comm_rec.est_no,clip (comm_rec.est_short));

		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
