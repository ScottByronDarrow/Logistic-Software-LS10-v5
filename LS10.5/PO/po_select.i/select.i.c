/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( po_select.i.c  )                                 |
|  Program Desc  : ( Purchase order reports selection one.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
|  Date Modified : (05/09/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (16/11/88)      | Modified  by  : F.C.Yap.         |
|  Date Modified : (12/12/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : default start date to current date,only display    |
|                : end date field if prog_name is po_purlog.          |
|                :                                                    |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|                : (12/09/97) - Incorporated multilingual conversion  |
|                :              and DMY4 date.						  |
|                                                                     |
| $Log: select.i.c,v $
| Revision 5.3  2002/07/17 09:57:39  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:16:06  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:14  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:17  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
|
| Revision 1.10  2000/01/26 07:39:44  ramon
| For GVision compatibility, I added a description field for Background.
|
| Revision 1.9  1999/11/11 06:43:18  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.8  1999/11/05 05:17:16  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.7  1999/09/29 10:12:13  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/21 04:38:11  scott
| Updated from Ansi project
|
| Revision 1.5  1999/06/17 10:06:37  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: select.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_select.i/select.i.c,v 5.3 2002/07/17 09:57:39 scott Exp $";

#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <get_lpno.h>		

   	char 	progname[40],
		progdesc[100];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
	};

	int comm_no_fields = 4;
	
	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char tco_short[16];
	} comm_rec;

	char	systemDate[11];
	int	num_field = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	back_desc[5];
	char	lpno[2];
	long	start_dt;
	long	end_dt;
	int	lp_no;
} local_rec;
	
static	struct	var	vars[]	={	

	{1, LIN, "st_dt", 4, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Start Date", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start_dt}, 
	{1, LIN, "fin_dt", 5, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "End Date", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_dt}, 
	{1, LIN, "back", 6, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "back_desc", 6, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc}, 
	{1, LIN, "lpno", 7, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number ", " ", 
		YES, NO, JUSTRIGHT, "123456789", "", (char *)&local_rec.lp_no}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
int spec_valid (int field);
void CloseDB (void);
void OpenDB (void);
int heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char date1[11];
	char date2[11];

	if (argc != 3 && argc != 4)
	{
		print_at(0,0,mlPoMess734,argv[0]);
		return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	num_field = argc;

	strcpy(progname,argv[1]);
	strcpy(progdesc,argv[2]);

	if (argc == 4)
		vars[label("fin_dt")].required = YES;

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();          /*  get into raw mode		*/
	set_masks();		/*  setup print using masks	*/
	init_vars(1);		/*  set default values		*/
	
	OpenDB();

   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = 1;

	init_vars(1);

	strcpy (systemDate, DateToString (TodaysDate()));
	local_rec.start_dt 	= TodaysDate ();
	local_rec.end_dt 	= TodaysDate ();

	strcpy(local_rec.back, "N");
	strcpy(local_rec.back_desc,"No ");
	local_rec.lp_no = 1;

	heading(1);
	scn_display(1);
	edit(1);
    if (restart || prog_exit)  {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }
	
	strcpy(date1,DateToString(local_rec.start_dt));
	strcpy(date2,DateToString(local_rec.end_dt));
	sprintf(local_rec.lpno,"%2d",local_rec.lp_no);

	/*====================================
	| Test for forground or background . |
	====================================*/
	shutdown_prog ();

	if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp(progname,
					progname,
					local_rec.lpno,
					date1,
					date2,(char *)0);
	}
	else 
		execlp(progname,
				progname,
				local_rec.lpno,
				date1,
				date2,(char *)0);
	
	shutdown_prog();   
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
	if (LCHECK("back"))
	{
		strcpy(local_rec.back_desc, (local_rec.back[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("back_desc");
		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lp_no = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lp_no))
		{
			errmess(ML(mlStdMess020));
			return(1);
		}
		return(0);
	}

	if (num_field == 4)
	{
		if (LCHECK("st_dt"))
		{
			if (prog_status != ENTRY && local_rec.start_dt > local_rec.end_dt)
			{
/*
				sprintf(err_str,"Start Date %s May Not Be > End Date",DateToString(local_rec.start_dt));
				errmess(err_str);*/
				errmess(ML(mlStdMess019));
				return(1);
			}
			return(0);
		}

		if (LCHECK("fin_dt"))
		{
			if (local_rec.start_dt > local_rec.end_dt)
			{
/*
				sprintf(err_str,"End Date %s May Not Be < Start Date",DateToString(local_rec.end_dt));
				errmess(err_str);*/
				errmess(ML(mlStdMess019));
				return(1);
			}
			return(0);
		}
		return(0);
	}

	return(0);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_dbclose("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		sprintf(err_str, ML(mlPoMess219),clip(progdesc));
		rv_pr(err_str,(80 - strlen(clip(err_str))) / 2,0,1);
		move(0,1);
		line(80);

		move(1,input_row);
		if (scn == 1)
			box(0,3,80,4);
		
		move(0,20);
		line(80);
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);

		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
