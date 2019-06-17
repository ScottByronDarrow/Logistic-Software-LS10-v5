/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( po_mthdet.i.c  )                                 |
|  Program Desc  : ( Input Program for Monthly Seasonal report   )    |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Vicki Seal      | Date Written  : 31/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (14/11/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (15/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      : Removed return from read_comm().                   |
|                : Include new scrgen                                 |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|                : (15/09/97) - Incorporate multilingual conversion.  |
|                :                                                    |
|                                                                     |
| $Log: mthdet.i.c,v $
| Revision 5.3  2002/07/17 09:57:38  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:15:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:49  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:23  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/11 06:43:15  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.7  1999/11/05 05:17:13  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.6  1999/09/29 10:12:03  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/21 04:38:05  scott
| Updated from Ansi project
|
| Revision 1.4  1999/06/17 10:06:29  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mthdet.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_mthdet.i/mthdet.i.c,v 5.3 2002/07/17 09:57:38 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int comm_no_fields = 3;

	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
	} comm_rec;

	int	use_stat = FALSE;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	prog_desc[61];
	char	back[2];
	char	onight[2];
	int	lpno;
	char	lp_str[3];
	char	s_date[11];
	char	e_date[11];
	long	start;
	long	end;
	char	order_detail[2];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "lpno", 4, 15, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 5, 15, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "start_period", 7, 15, EDATETYPE, 
		"DD/DD", "          ", 
		" ", "MM/YY", "Start Period. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start}, 
	{1, LIN, "end_period", 8, 15, EDATETYPE, 
		"DD/DD", "          ", 
		" ", "MM/YY", "End Period.", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end}, 
	{1, LIN, "tape_status", 10, 15, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Order Detail ", "Show Order detail (Y)es, (N)o.", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.order_detail}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void CloseDB (void);
int spec_valid (int field);
void OpenDB (void);
int heading (int scn);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	/*---------------------------------------
	|	parameters			                |
	|	1:	program name		            |
	|	2:	program description    	        |
	|	3:	status required 	            |
	---------------------------------------*/

	if (argc != 3 && argc != 4) 
	{
		print_at(0,0,mlStdMess037,argv[0]);
        return (EXIT_FAILURE);
	}
	sprintf(local_rec.prog_desc,"%s",clip(argv[2]));

	SETUP_SCR (vars);

	if (argc == 4 && argv[3][0] == '1')
		use_stat = TRUE;
	else
		vars[5].scn = 0;

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();

   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = 1;
	init_vars(1);

	/*-----------------------------
	| Enter screen 1 linear input |
	-----------------------------*/
	heading(1);
	entry(1);
    if (prog_exit) {
		shutdown_prog();
        return (EXIT_FAILURE);
    }

	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading(1);
	scn_display(1);
	edit(1);
	prog_exit = 1;
    if (restart) {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }
	
	strcpy(local_rec.s_date,DateToString(local_rec.start));
	strcpy(local_rec.e_date,DateToString(local_rec.end));
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);

	CloseDB (); FinishProgram ();
	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork() == 0)
		{
			if (use_stat)
				execlp("ONIGHT",
					"ONIGHT",
					argv[1],
					local_rec.lp_str,
					local_rec.s_date,
					local_rec.e_date,
					local_rec.order_detail,
					argv[2],0);
			else
				execlp("ONIGHT",
					"ONIGHT",
					argv[1],
					local_rec.lp_str,
					local_rec.s_date,
					local_rec.e_date,
					argv[2],0);
		}
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
		{
			if (use_stat)
				execlp(argv[1],
					argv[1],
					local_rec.lp_str,
					local_rec.s_date,
					local_rec.e_date,
					local_rec.order_detail,0);
			else
				execlp(argv[1],
					argv[1],
					local_rec.lp_str,
					local_rec.s_date,
					local_rec.e_date,0);
		}
	}
	else 
	{
		if (use_stat)
			execlp(argv[1],
				argv[1],
				local_rec.lp_str,
				local_rec.s_date,
				local_rec.e_date,
				local_rec.order_detail,0);
		else
			execlp(argv[1],
				argv[1],
				local_rec.lp_str,
				local_rec.s_date,
				local_rec.e_date,0);
	}

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
	FinishProgram ();
}

/*======================	
| Close database file. |
======================*/
void
CloseDB (
 void)
{
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (strcmp(FIELD.label,"lpno") == 0)
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			return(1);
		}
		return(0);
	}

	if (strcmp(FIELD.label,"start_period") == 0)
	{
		if (prog_status == ENTRY)
			return(0);

		if (local_rec.start > local_rec.end)
		{
			/*print_mess(" Error : Start Period <= End Period.");*/
			print_mess(ML(mlStdMess019));
			return(1);
		}
		return(0);
	}

	if (strcmp(FIELD.label,"end_period") == 0)
	{
		if (local_rec.start > local_rec.end)
		{
			/*print_mess(" Error : Start Period >= End Period.");*/
			print_mess(ML(mlStdMess026));
			return(1);
		}
		return(0);
	}
	return(0);
}

void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	int	y = (80 - strlen(local_rec.prog_desc))/2;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();

		rv_pr(local_rec.prog_desc,y,0,1);
		move(0,1);
		line(80);

		box(0,3,80,(use_stat) ? 7 : 5);

		move(1,6);
		line(79);

		if (use_stat)
		{
			move(1,9);
			line(79);
			move(1,11);
			line(79);
		}

		move(0,20);
		line(80);
		strcpy (err_str,ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,clip(comm_rec.tco_name));
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}

    return (EXIT_SUCCESS);
}
