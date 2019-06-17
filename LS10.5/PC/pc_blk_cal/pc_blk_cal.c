/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_blk_cal.c,v 5.4 2002/07/24 08:38:57 scott Exp $
|  Program Name  : (pc_blk_cal )                                   
|  Program Desc  : (Production Control Bulk Calendar Generation.)   
|---------------------------------------------------------------------|
|  Date Written  : (03/02/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: pc_blk_cal.c,v $
| Revision 5.4  2002/07/24 08:38:57  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:20:07  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_blk_cal.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_blk_cal/pc_blk_cal.c,v 5.4 2002/07/24 08:38:57 scott Exp $";

#ifdef TABLINES
#undef TABLINES
#endif

#define	TABLINES	7

#define	DAY_NUMB	0
#define	ACT_TIME	1
#define	DURATION	2

#define	WEEK_MINS	10080L
#define	DAY_MINS	1440L
#define	DATE_TIME(x, y)	 ((x * 24L * 60L) + y)

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

FILE	*fout;
	
char	*day_of_wk [] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", 
	"Thursday", "Friday", "Saturday", ""
};

char	*mth_name [] = {
	"January", "February", "March",     "April",   "May",      "June", 
	"July",    "August",   "September", "October", "November", "December",""
};

#include	"schema"

struct commRecord	comm_rec;
struct rgrsRecord	rgrs_rec;
struct pcclRecord	pccl_rec;

	char	*data	= "data";

int	clearOk;

struct	storeRec
{
	int		day;
	char	dayName [10];
	long	actualTime;
	long	duration;
} store [MAXLINES], order [MAXLINES];

struct	store2Rec
{
	long	excluded;
} store2 [MAXLINES];

int	tmpDmy [3];
int	deleteCalendar;
int	overwrite;
int	errorDay;
int	errorPage;
int	errorLine;
long	envPcTimeRes;

/*
 * Local & Screen Structures. 
 */
struct
{
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	resourceCode [9];
	char	resourceDesc [41];
	long	startDate;
	char	startDateDesc [31];
	long	endDate;
	char	endDateDesc [31];
	int		day;
	char	dayName [10];
	long	actualTime;
	long	duration;
	long	extendDate;
	char	extendDateDesc [31];
	char	tmpDateDesc [31];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "resourceCode",	 4, 2, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Resource       ", "Default for GLOBAL calendar generation.",
		YES, NO,  JUSTLEFT, "", "", local_rec.resourceCode},
	{1, LIN, "resourceDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.resourceDesc},
	{1, LIN, "startDate",	 5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date     ", "",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.startDate},
	{1, LIN, "startDateDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startDateDesc},
	{1, LIN, "endDate",	 6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date       ", "",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.endDate},
	{1, LIN, "endDateDesc", 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endDateDesc},

	{2, TAB, "day",	 MAXLINES, 1, INTTYPE,
		"N", "          ",
		" ", "1", "Day", " 1 = Sunday, 2 = Monday, 3 = Tuesday etc ",
		YES, NO,  JUSTRIGHT, "1", "7", (char *)&local_rec.day},
	{2, TAB, "dayName",	 7, 1, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", " ", " Day Of Week ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.dayName},
	{2, TAB, "actualTime",	 0, 6, TIMETYPE,
		"NN:NN", "          ",
		" ", " ", " Activation Time ","",
		YES, NO,  JUSTLEFT, "0", "1439", (char *)&local_rec.actualTime},
	{2, TAB, "duration",	 0, 2, TIMETYPE,
		"NNN:NN", "          ",
		" ", " ", " Duration ", "",
		YES, NO,  JUSTLEFT, "0", "10080", (char *)&local_rec.duration},

	{3, TAB, "extendDate",	 MAXLINES, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", " Date  ", "",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.extendDate},
	{3, TAB, "extendDateDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " Extended Date         ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.extendDateDesc},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
/*
 * function prototypes 
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
int 	AllLinesOk 			(void);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	InsertLine 			(void);
int 	DeleteLine 			(void);
int 	DeleteXLine 		(void);
void 	SrchRgrs 			(char *);
int 	CalcExtendedDate 	(long);
int 	Process 			(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char *argv [])
{
	int	i;
	char	*sptr = get_env ("PC_TIME_RES");

	envPcTimeRes = (sptr) ? atol (sptr) : 5L;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	TruePosition = TRUE;

	SETUP_SCR (vars);

    OpenDB ();

	ReadMisc ();

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
	SetSortArray (3, store2, sizeof (struct store2Rec));
#endif
	/*
	 * Beginning of input control loop . 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;

		overwrite	= FALSE;

		init_vars (1);

		/*
		 * Enter screen 1 linear input . 
		 */
		clearOk = TRUE;
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit day details. 
		 */
		tab_row = 9;
		tab_col = 16;
		clearOk = FALSE;
	
		init_vars (2);
		lcount [2] = 0;
		errorDay = -1;
		errorPage = -1;
		errorLine = -1;
		do 
		{
			heading (2);
			clearOk = TRUE;
			scn_display (2);
			if (errorPage == -1)
				edit (2);
			else
				_edit (2, errorLine, label ("duration"));

			if (restart)
				break;

			deleteCalendar = FALSE;
			if (lcount [2] == 0)
			{
				/*i = prmptmsg ("\007 Calendar Will Be Deleted For This Date Range. Continue Update ? ", "YyNn", 5, 2);*/
				i = prmptmsg (ML (mlPcMess001), "YyNn", 5, 2);
				move (0,2);
				cl_line ();
				if (i == 'N' || i == 'n')
					continue;
				else
				{
					deleteCalendar = TRUE;
					break;
				}
			}
		} while (!AllLinesOk ());
	
		if (restart)
			continue;

		if (!deleteCalendar)
		{
			/*
			 * Edit exclusion details. 
			 */
			tab_col = 19;
			init_vars (3);
			lcount [3] = 0;
			heading (3);
			edit (3);
			if (restart)
				continue;
		}

		Process ();

	}	/* end of input control loop	*/

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
	open_rec (pccl,  pccl_list, PCCL_NO_FIELDS, "pccl_id_no");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (rgrs);
	abc_fclose (pccl);
	abc_dbclose (data);
}

/*
 * Get common info from common database file. 
 */
void
ReadMisc (void)
{


	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
}

/*
 * Put lines in order and check for overlapping times        
 */
int
AllLinesOk (void)
{
	int	i, j, ins, next_entry; 
	int	no_lines, no_in_order;
	int	inserted;
	long	st_time, end_time, day_start;

	if (lcount [2] == 0)
		return (FALSE);

	if (lcount [2] == 1)
		return (TRUE);

	/*
	 * Put lines in order 
	 */
	no_lines = lcount [2];
	no_in_order = 0;
	for (i = 0; i < no_lines; i++)
	{
	    inserted = FALSE;
	    for (j = 0; j < no_in_order; j++)
	    {
			/*
			 * Insert 
			 */
			if (store [i].day < order [j].day ||
			   (store [i].day == order [j].day &&
				 store [i].actualTime <= order [j].actualTime))
			{
				/*
				 * Shuffle lines ready for insert 
				 */
				for (ins = no_in_order; ins > j; ins--)
				{
					order [ins].day = order [ins - 1].day;
					strcpy (order [ins].dayName, order [ins - 1].dayName);
					order [ins].actualTime = order [ins - 1].actualTime;
					order [ins].duration = order [ins - 1].duration;
				}
				no_in_order++;

				order [j].day = store [i].day;
				strcpy (order [j].dayName, store [i].dayName);
				order [j].actualTime = store [i].actualTime;
				order [j].duration = store [i].duration;

				inserted = TRUE;
				break;
			}
	    }
	    /*
	     * Append 
	     */
	    if (!inserted)
	    {
		    order [no_in_order].day = store [i].day;
		    strcpy (order [no_in_order].dayName, store [i].dayName);
		    order [no_in_order].actualTime = store [i].actualTime;
		    order [no_in_order++].duration = store [i].duration;
	    }
	}

	scn_set (2);
	lcount [2] = 0;
	for (i = 0; i < no_in_order; i++)
	{
		local_rec.day = order [i].day;
		strcpy (local_rec.dayName, order [i].dayName);
		local_rec.actualTime = order [i].actualTime;
		local_rec.duration = order [i].duration;
		putval (lcount [2]++);

		store [i].day = order [i].day;
		strcpy (store [i].dayName, order [i].dayName);
		store [i].actualTime = order [i].actualTime;
		store [i].duration = order [i].duration;
	}

	/*
	 * Check For Overlapping Times 
	 */
	for (i = 0; i < lcount [2]; i++)
	{
		next_entry = i + 1;
		if (next_entry >= lcount [2])
			next_entry = 0;

		day_start = (long) (store [next_entry].day - 1) * DAY_MINS;
		day_start += store [next_entry].actualTime;

		st_time = (long) (store [i].day - 1) * DAY_MINS;
		st_time += store [i].actualTime;

		end_time = st_time + store [i].duration;
		if (end_time > WEEK_MINS)
			end_time -= WEEK_MINS;

		if ((end_time <= st_time &&
		    ((day_start >= st_time && day_start <= WEEK_MINS) ||
		     (day_start >= 0 && day_start < end_time))) ||
		   (st_time < end_time &&
		     day_start >= st_time && day_start < end_time))
		{
			errorDay = store [i].day;
			errorPage = i / TABLINES;
			errorLine = i;
			return (FALSE);
		}
	}
	return (TRUE);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (clearOk)
			clear ();

		rv_pr (ML (mlPcMess063), 29, 0, 1);

		line_at (1,0,80);

		if (scn == 1)
		{
			box (0, 3, 80, 3);
		}

		if (scn == 2)
		{
			box (0, 3, 80, 3);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (16, 8, 48, 9);
			rv_pr (ML (mlPcMess137), 29, 8, 1);
			if (errorDay >= 0)
			{
				sprintf (err_str,ML (mlPcMess058),
					errorDay, errorPage + 1, 
					 (errorLine % TABLINES) + 1);
				rv_pr (err_str, (80 - strlen (err_str)) / 2, 20,1);
			}
		}

		if (scn == 3)
		{
			box (0, 3, 80, 3);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (19, 8, 43, 9);
			rv_pr (ML (mlPcMess064), 34, 8, 1);
		}

		line_at (20,0,80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0,err_str,
			comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22, 0,err_str,
			comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_set (scn);
		scn_write (scn);
	}
	return (TRUE);
}

int
spec_valid (
 int field)
{
	int	i; 

	if (LCHECK ("resourceCode"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.resourceCode, "GLOBAL  ");
			sprintf (local_rec.resourceDesc, 
				"%-40.40s", 
				"Generate System-Wide Calendar");
			DSP_FLD ("resourceDesc");
			rgrs_rec.hhrs_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Validate resource code 
		 */
		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.resourceCode);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPcMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Specific calendars may not be generated 
		 * for resources which are set up as using 
		 * the GLOBAL calendar.                    
		 */
		if (rgrs_rec.cal_sel [0] == 'G')
		{
			print_mess (ML (mlPcMess059));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.resourceDesc, "%-40.40s", rgrs_rec.desc);
		DSP_FLD ("resourceDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startDate"))
	{
		if (dflt_used)
			local_rec.startDate = local_rec.lsystemDate;

		CalcExtendedDate (local_rec.startDate);
		sprintf (local_rec.startDateDesc, "%-30.30s", local_rec.tmpDateDesc);
		DSP_FLD ("startDateDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate"))
	{
		if (local_rec.endDate < local_rec.startDate)
		{
			print_mess (ML (mlStdMess026));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (pccl_rec.co_no, comm_rec.co_no);
		strcpy (pccl_rec.br_no, comm_rec.est_no);
		pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
		pccl_rec.act_date = local_rec.startDate;
		pccl_rec.act_time = 0L;
		cc = find_rec (pccl, &pccl_rec, GTEQ, "r");
		if (!cc &&
		    !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
		    !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
		    pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash &&
		    pccl_rec.act_date <= local_rec.endDate)
		{
			i = prmptmsg (ML (mlPcMess060), "YyNn", 5, 2);
			move (0,2);
			cl_line ();
			if (i == 'Y' || i == 'y')
				overwrite = TRUE;
			else
				return (EXIT_FAILURE);
		}
		CalcExtendedDate (local_rec.endDate);
		sprintf (local_rec.endDateDesc, "%-30.30s", local_rec.tmpDateDesc);
		DSP_FLD ("endDateDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("day"))
	{
		if (last_char == INSLINE)
			return (InsertLine ());

		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		sprintf (local_rec.dayName, "%-9.9s", day_of_wk [local_rec.day - 1]);

		strcpy (store [line_cnt].dayName, local_rec.dayName);
		store [line_cnt].day = local_rec.day;
		DSP_FLD ("dayName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("actualTime"))
	{
		local_rec.actualTime = rnd_time (local_rec.actualTime, envPcTimeRes);
		store [line_cnt].actualTime = local_rec.actualTime;
	
		DSP_FLD ("actualTime");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("duration"))
	{
		local_rec.duration = rnd_time (local_rec.duration, envPcTimeRes);
		store [line_cnt].duration = local_rec.duration;
			
		DSP_FLD ("duration");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("extendDate"))
	{
		if (last_char == DELLINE || dflt_used)
		{
			return (DeleteXLine ());
		}

		if (local_rec.extendDate < local_rec.startDate ||
		    local_rec.extendDate > local_rec.endDate)
		{
			print_mess (ML (mlPcMess061));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check that date has not already been excluded.  
		 */
		for (i = 0; i < line_cnt; i++)
		{
			if (local_rec.extendDate == store2 [i].excluded)
			{
				print_mess (ML (mlPcMess062));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		CalcExtendedDate (local_rec.extendDate);
		sprintf (local_rec.extendDateDesc, "%-30.30s", local_rec.tmpDateDesc);
		DSP_FLD ("extendDateDesc");
		store2 [line_cnt].excluded = local_rec.extendDate;

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
InsertLine (void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	for (i = line_cnt, line_cnt = lcount [2]; line_cnt > i; line_cnt--)
	{
		getval (line_cnt - 1);
		store [line_cnt].day      = local_rec.day;
		store [line_cnt].actualTime = local_rec.actualTime;
		store [line_cnt].duration = local_rec.duration;

		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	local_rec.day = 0;
	local_rec.actualTime = 0L;
	local_rec.duration = 0L;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();
	init_ok = FALSE;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = TRUE;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		store [line_cnt].day = local_rec.day;
		strcpy (store [line_cnt].dayName, local_rec.dayName);
		store [line_cnt].actualTime = local_rec.actualTime;
		store [line_cnt].duration = local_rec.duration;

		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

int
DeleteXLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [3]--;

	for (i = line_cnt; line_cnt < lcount [3]; line_cnt++)
	{
		getval (line_cnt + 1);

		store2 [line_cnt].excluded = store2 [line_cnt + 1].excluded;

		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

void
SrchRgrs (
 char *key_val)
{
	_work_open (8,0,40);
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in rgrs During (DBFIND)", cc, PNAME);
}

/*
 * Construct date description 
 */
int
CalcExtendedDate (
	long	useDate)
{
	char	daySuffix [3];

	DateToDMY (useDate, &tmpDmy [0],&tmpDmy [1],&tmpDmy [2]);

	switch (tmpDmy [0])
	{
	case 1:
	case 21:
	case 31:
		strcpy (daySuffix, "st");
		break;

	case 2:
	case 22:
		strcpy (daySuffix, "nd");
		break;

	case 3:
	case 23:
		strcpy (daySuffix, "rd");
		break;

	default:
		strcpy (daySuffix, "th");
		break;
	}

	sprintf (local_rec.tmpDateDesc, "%-9.9s %2d%-2.2s %-9.9s %04d",
		day_of_wk [useDate % 7],
		tmpDmy [0],
		daySuffix,
		mth_name [tmpDmy [1] - 1],
		tmpDmy [2]);

	return (EXIT_SUCCESS);
}

int
Process (void)
{
	int		currentDay;
	int		excl_date_fnd;
	int		i, j;
	long	currentDate;
	long	absStartTime;
	long	absEndTime;
	long	currStartTime;
	long	currEndTime;
	long	excl_st_time = 0L;
	char	tmpDate [11];

	dsp_screen ("Creating Calendar", comm_rec.co_no, comm_rec.co_name);

	/*
	 * Delete any existing records within selected range 
	 */
	if (overwrite)
	{
	    strcpy (pccl_rec.co_no, comm_rec.co_no);
	    strcpy (pccl_rec.br_no, comm_rec.est_no);
	    pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
	    pccl_rec.act_date = local_rec.startDate;
	    pccl_rec.act_time = 0L;
	    cc = find_rec (pccl, &pccl_rec, GTEQ, "u");
	    while (!cc &&
		   !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
		   !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
		   pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash &&
		   pccl_rec.act_date <= local_rec.endDate)
	    {
		cc = abc_delete (pccl);
		if (cc)
		    file_err (cc, pccl, "DBDELETE");

		strcpy (pccl_rec.co_no, comm_rec.co_no);
		strcpy (pccl_rec.br_no, comm_rec.est_no);
		pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
		pccl_rec.act_date = local_rec.startDate;
		pccl_rec.act_time = 0L;
		cc = find_rec (pccl, &pccl_rec, GTEQ, "u");
	    }
	}

	/*
	 * Put excluded dates in order. 
	 */
	for (i = 0; i < lcount [3]; i++)
	{
	    if (i < (lcount [3] - 1))
	    {
			if (store2 [i].excluded > store2 [i + 1].excluded)
			{
				/*
				 * Swap dates and start again 
				 */
				currentDate 			= store2 [i].excluded;
				store2 [i].excluded 	= store2 [i + 1].excluded;
				store2 [i + 1].excluded = currentDate;

				i = -1;
				continue;
			}
	    }
	}

	currentDate = local_rec.startDate;
	currentDay = (local_rec.startDate % 7) + 1;

	/*
	 * Create records for specified duration. 
	 * There may be multiple records because 
	 * of excluded dates.                   
	 */
	while (currentDate <= local_rec.endDate)
	{
	    strcpy (tmpDate, DateToString (currentDate));
	    dsp_process ("Date :", tmpDate);

	    for (i = 0; i < lcount [2]; i++)
	    {
			/*
			 * Don't create records unless the DAY 
			 * has been specified in the calendar. 
			 */
			if (store [i].day != currentDay)
				continue;

			absStartTime = DATE_TIME (currentDate, store [i].actualTime);
			absEndTime = absStartTime + store [i].duration;
			currStartTime = absStartTime;
			currEndTime = currStartTime;

			/*
			 * Create records for specified duration. 
			 */
			while (currEndTime < absEndTime)
			{
				/*
				 * Find first (if any) excluded  
				 * date between currStartTime and
				 * absEndTime.               
				 */
				excl_date_fnd = FALSE;
				for (j = 0; j < lcount [3]; j++)
				{
					excl_st_time = DATE_TIME (store2 [j].excluded, 0L);
					if (excl_st_time >= currStartTime &&
						excl_st_time <= absEndTime)
					{
						excl_date_fnd = TRUE;
						break;
					}
				}

				/*
				 * absStartTime of excluded date  
				 * becomes currEndTime. Create 
				 * pccl record for this duration
				 */
				if (excl_date_fnd)
					currEndTime = excl_st_time;
				else
					currEndTime = absEndTime;

				strcpy (pccl_rec.co_no, comm_rec.co_no);
				strcpy (pccl_rec.br_no, comm_rec.est_no);
				pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
				pccl_rec.act_date = currStartTime / (24L * 60L);
				pccl_rec.act_time = currStartTime - (pccl_rec.act_date * 24L * 60L);
				pccl_rec.duration = (currEndTime - currStartTime);
				strcpy (pccl_rec.stat_flag, "0");

				if (pccl_rec.duration != 0L)
				{
					cc = abc_add (pccl, &pccl_rec);
					if (cc)
						file_err (cc, pccl, "DBADD");
				}

				/*
				 * currStartTime then becomes the 
				 * currEndTime + (24L * 60L).  
				 * currEndTime = currStartTime. 
				 */
				if (excl_date_fnd)
				{
					currStartTime 	= currEndTime + (24L * 60L);
					currEndTime 	= currStartTime;
				}
			}
	    }
	    currentDate++;
	    currentDay++;
	    if (currentDay > 7)
		currentDay = 1;
	}
	return (EXIT_SUCCESS);
}
