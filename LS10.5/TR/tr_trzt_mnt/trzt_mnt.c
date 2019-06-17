/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( tr_trtz_mnt.c )                                  |
|  Program Desc  : ( Transport Time file maintenance.             )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (10/03/1999)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: trzt_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_trzt_mnt/trzt_mnt.c,v 5.3 2002/07/25 11:17:39 scott Exp $";

#define		MAXSCNS		1
#define		MAXLINES	12
#define		TABLINES	12
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <minimenu.h>

#define		MAX_HOURS	24

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tes_no [3];
		char	tes_name [41];
	} comm_rec;

	/*==================================+
	 | TRansport Zone Time Maintenance. |
	 +==================================*/
#define	TRZT_NO_FIELDS	4

	struct dbview	trzt_list [TRZT_NO_FIELDS] =
	{
		{"trzt_co_no"},
		{"trzt_time_code"},
		{"trzt_start_time"},
		{"trzt_end_time"}
	};

	struct tag_trztRecord
	{
		char	co_no [3];
		char	time_code [2];
		long	start_time;
		long	end_time;
	}	trzt_rec;


char	*data  = "data",
		*trzt  = "trzt";

#include	<tr_schedule.h>
	
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	timeSlotLeft [2],
			timeSlotRight [2];
	long	startTimeLeft,
			startTimeRight,
			endTimeLeft,
			endTimeRight;
} local_rec;

static	struct	var	vars[] =
{
	{1, TAB, "timeSlotLeft",	 MAXLINES, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Code", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotLeft},
	{1, TAB, "startTimeLeft",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start Time", " ",
		YES, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeLeft},
	{1, TAB, "endTimeLeft",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", " End Time ", " ",
		YES, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeLeft},
	{1, TAB, "timeSlotRight",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Code", " ",
		NA, NO,  JUSTLEFT, "", "", (char *)&local_rec.timeSlotRight},
	{1, TAB, "startTimeRight",	 0, 2, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start Time", " ",
		YES, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeRight},
	{1, TAB, "endTimeRight",	 0, 1, TIMETYPE,
		"NN:NN", "          ",
		" ", "", " End Time ", " ",
		YES, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeRight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},

};

typedef int BOOL;
extern	int	TruePosition;

/*=======================
| Function Declarations |
=======================*/
int 	spec_valid 			(int);
int 	CheckOverAll 		(void);
void	LoadDefaultTimes 	(void);
void 	shutdown_prog 		(void);
void	LoadTabularScreen 	(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Update 				(void);
void 	LoadDefaultTimes 	(void);
void 	LoadTabularScreen 	(void);
int 	heading 			(int);

/*==========================
| Main processing routine. |
==========================*/
int 
main (
 int argc,
 char * argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	TruePosition	=	TRUE;

	tab_row	=	5;
	tab_col	=	10;
	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		LoadDefaultTimes ();
		LoadTabularScreen ();

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			break;

		Update ();
		prog_exit = TRUE;
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

/*========================
| Open data base files . |
========================*/
void 
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (trzt, trzt_list, TRZT_NO_FIELDS, "trzt_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose (trzt);
	abc_dbclose (data);
}

int 
spec_valid (
 int field)
{
	if (CheckOverAll ())
	{
		print_mess (ML (mlTrMess058));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("startTimeLeft"))
	{
		if (local_rec.startTimeLeft > local_rec.endTimeLeft)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("startTimeRight"))
	{
		if (local_rec.startTimeRight > local_rec.endTimeRight)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endTimeLeft"))
	{
		if (local_rec.endTimeLeft < local_rec.startTimeLeft)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endTimeRight"))
	{
		if (local_rec.endTimeRight < local_rec.startTimeRight)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	return(0);
}

int 
CheckOverAll (
 void)
{
	int		holdLineCnt	=	line_cnt;
	int		i;
	long	startArray [MAX_HOURS];
	long	endArray [MAX_HOURS];

	putval (line_cnt);
	
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++)
	{
		getval (line_cnt);

		startArray [line_cnt]		=	local_rec.startTimeLeft;
		endArray [line_cnt]			=	local_rec.endTimeLeft;
		startArray [line_cnt+12]	=	local_rec.startTimeRight;
		endArray [line_cnt+12]		=	local_rec.endTimeRight;
	}
	line_cnt	=	holdLineCnt;
	getval (line_cnt);

	for (i = 1; i < MAX_HOURS; i++)
		if (startArray [i] < endArray [i - 1])
			return (EXIT_FAILURE);
	
	return (EXIT_SUCCESS);
}
/*==================
| Updated records. |
==================*/
void 
Update (
 void)
{
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++)
	{
		getval (line_cnt);

		strcpy (trzt_rec.co_no, comm_rec.tco_no);
		strcpy (trzt_rec.time_code, local_rec.timeSlotLeft);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "u");
		if (cc)
		{
			trzt_rec.start_time	=	local_rec.startTimeLeft;
			trzt_rec.end_time	=	local_rec.endTimeLeft;
			cc = abc_add (trzt, &trzt_rec);
			if (cc)
				file_err (cc, trzt, "DBADD");
		}
		else
		{
			trzt_rec.start_time	=	local_rec.startTimeLeft;
			trzt_rec.end_time	=	local_rec.endTimeLeft;
			cc = abc_update (trzt, &trzt_rec);
			if (cc)
				file_err (cc, trzt, "DBUPDATE");
		}
		strcpy (trzt_rec.co_no, comm_rec.tco_no);
		strcpy (trzt_rec.time_code, local_rec.timeSlotRight);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "u");
		if (cc)
		{
			trzt_rec.start_time	=	local_rec.startTimeRight;
			trzt_rec.end_time	=	local_rec.endTimeRight;
			cc = abc_add (trzt, &trzt_rec);
			if (cc)
				file_err (cc, trzt, "DBADD");
		}
		else
		{
			trzt_rec.start_time	=	local_rec.startTimeRight;
			trzt_rec.end_time	=	local_rec.endTimeRight;
			cc = abc_update (trzt, &trzt_rec);
			if (cc)
				file_err (cc, trzt, "DBUPDATE");
		}
	}
	abc_unlock (trzt);
}

/*========================================================
| Load default time records if they don't already exist. |
========================================================*/
void
LoadDefaultTimes (
 void)
{
	int		i;

	for (i = 0; strlen (TC [i].time_code); i++)
	{
		strcpy (trzt_rec.co_no, comm_rec.tco_no);
		strcpy (trzt_rec.time_code, TC [i].time_code);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
		{
			trzt_rec.start_time	=	rnd_time (atot (TC [i].time_start), 	1);
			trzt_rec.end_time	=	rnd_time (atot (TC [i].time_end), 	1);
			cc = abc_add (trzt, &trzt_rec);
			if (cc)
				file_err (cc, trzt, "DBADD");
		}
	}
}

void
LoadTabularScreen (
 void)
{
	int		i;

	scn_set (1);
	lcount [1]	=	0;

	for (i = 0; strlen (TC [i].time_code); i++)
	{
		strcpy (trzt_rec.co_no, comm_rec.tco_no);
		strcpy (trzt_rec.time_code, TC [i].time_code);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, trzt, "DBFIND");
		
		if (i % 2)
		{
			strcpy (local_rec.timeSlotRight, trzt_rec.time_code);
			local_rec.startTimeRight 	=	trzt_rec.start_time;
			local_rec.endTimeRight 	 	=	trzt_rec.end_time;
			putval (lcount[1]++);
		}
		else
		{
			strcpy (local_rec.timeSlotLeft, trzt_rec.time_code);
			local_rec.startTimeLeft 	=	trzt_rec.start_time;
			local_rec.endTimeLeft 	 	=	trzt_rec.end_time;
		}
	}
}
/*===========================
| edit () callback function |
===========================*/
int 
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML ("Transport Time Code Maintenance"), 23, 0, 1);
		move (0, 1); line (80);

		move (0, 21); line(80);
		print_at (22,0, mlStdMess038, comm_rec.tco_no, comm_rec.tco_name);
	
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
