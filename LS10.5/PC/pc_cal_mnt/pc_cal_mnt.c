/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_cal_mnt.c,v 5.3 2002/07/18 06:50:07 scott Exp $
|  Program Name  : (pc_cal_mnt )                                    |
|  Program Desc  : (Production Control Calendar Maintenance.    )   |
|---------------------------------------------------------------------|
|  Date Written  : (10/02/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: pc_cal_mnt.c,v $
| Revision 5.3  2002/07/18 06:50:07  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:14:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:08  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 00:04:28  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to use SQL calls that work with CISAM and SQL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_cal_mnt.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_cal_mnt/pc_cal_mnt.c,v 5.3 2002/07/18 06:50:07 scott Exp $";

#ifdef TABLINES
#undef TABLINES
#endif
#define	TABLINES	7

#define	DAY_NUMB	0
#define	ACT_TIME	1
#define	DURATION	2

#define	WEEK_MINS	10080L
#define	DAY_MINS	1440L

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>
#include <cal_select.h>
#include <twodec.h>

FILE	*fout;
	
struct	CAL_STORE *st_ptr    = CAL_NULL;
struct	CAL_INFO  *st_info   = INFO_NULL;
long	rslt_date;
long	date_overlap;

char	*day_of_wk [7];

char	*mth_name [] = {
	"January", "February", "March",     "April",   "May",      "June", 
	"July",    "August",   "September", "October", "November", "December",""
};

#include	"schema"

struct commRecord	comm_rec;
struct rgrsRecord	rgrs_rec;
struct pcclRecord	pccl_rec;

	char	*data	= "data";

int		firstTime;

struct
{
	int		day;
	char	dayName [10];
	long	actTime;
	long	duration;
} storeRec [MAXLINES], orderRec [MAXLINES];

int		tmpDMY [3],
		day_no,
		delete_cal,
		overwrite,
		errorDay,
		errorPage,
		errorLine,
		overlap;

long	timeRes;
char	dayName [10];

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	rsrc [9];
	char	rsrcDesc [41];
	long	startDate;
	long	endDate;
	int		day;
	char	dayName [10];
	long	actTime;
	long	duration;
	long	xDate;
	char	dateDesc [31];
	char	tempDtdesc [31];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "rsrc_code",	 3, 12, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", " Resource : ", "Default for GLOBAL calendar maintenance",
		YES, NO,  JUSTLEFT, "", "", local_rec.rsrc},
	{1, LIN, "rsrcDesc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.rsrcDesc},

	{2, TAB, "dayName",	 7, 1, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", " ", " Day Of Week ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.dayName},
	{2, TAB, "actTime",	 0, 6, TIMETYPE,
		"NN:NN", "          ",
		" ", " ", " Activation Time ","Time Resource Becomes Active",
		YES, NO,  JUSTLEFT, "0", "1439", (char *)&local_rec.actTime},
	{2, TAB, "duration",	 0, 2, TIMETYPE,
		"NNN:NN", "          ",
		" ", " ", " Duration ", "Duration Resource Stays Active",
		YES, NO,  JUSTLEFT, "0", "10080", (char *)&local_rec.duration},
	{2, TAB, "day",	 0, 0, INTTYPE,
		"N", "          ",
		" ", " ", "", "",
		ND, NO,  JUSTLEFT, "0123456789", "", (char *)&local_rec.day},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================
| function prototypes |
=====================*/
int	 	LoadCurrDay 		(void);
int 	AllLinesOK 			(void);
int 	CalcXtdDate 		(long);
int 	CheckNextRec 		(int);
int 	DeleteLine 			(void);
int 	InsertLine 			(void);
int 	ReadMisc 			(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	ProcessDay 			(long);
void 	SrchRgrs 			(char *);
void 	initML 				(void);
void 	shutdown_prog 		(void);
void 	tab_other 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int  argc, 
 char *argv [])
{
	int	curr_day;
	int	diff_mnths;
	char	*sptr = get_env ("PC_TIME_RES");

	timeRes = (sptr) ? atol (sptr) : 5L;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

    OpenDB ();

	ReadMisc ();

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	initML 		();
	init_scr 	();
	set_tty 	();
	set_masks 	();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		DateToDMY (local_rec.lsystemDate, &tmpDMY [0],&tmpDMY [0],&tmpDMY [2]);
		curr_day 	= tmpDMY [0];
		rslt_date 	= local_rec.lsystemDate;
		diff_mnths 	= 0;
		while (rslt_date != 0L)
		{
			/*--------------------
			| Calendar Selection |
			--------------------*/
			st_ptr = 	set_calendar 
						(
							st_ptr, 
							rslt_date, 
							diff_mnths, 
							(127 - diff_mnths)
						);
	
			rslt_date =	cal_select 
						(
							20, 
							5, 
							5, 
							2, 
							curr_day, 
							st_ptr, 
							st_info, 
							C_NORM
						);
	
			/*------------------------
			| Exit without selection |
			------------------------*/
			if (rslt_date == 0L)
				continue;

			/*----------------------
			| Check For Valid Date |
			----------------------*/
			if (rslt_date < local_rec.lsystemDate)
			{
				print_mess (ML (mlStdMess141));
				sleep (sleepTime);
				clear_mess ();
				continue;
			}
	
			ProcessDay (rslt_date);
	
			DateToDMY (rslt_date, &tmpDMY [0],&tmpDMY [0],&tmpDMY [2]);
			curr_day = tmpDMY [0];
			diff_mnths = (tmpDMY [2] * 12) + tmpDMY [1];

			DateToDMY (local_rec.lsystemDate, &tmpDMY [0],&tmpDMY [0],&tmpDMY [2]);
			diff_mnths -= (tmpDMY [2] * 12);
			diff_mnths -= tmpDMY [1];

			heading (1);
			scn_display (1);
		}

	}	/* end of input control loop	*/

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
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
	open_rec (pccl,  pccl_list, PCCL_NO_FIELDS, "pccl_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (rgrs);
	abc_fclose (pccl);
	abc_dbclose (data);
}

/*============================================
| Get common info from common database file. |
============================================*/
int
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	return (cc);
}

void
ProcessDay (
	long	selectDate)
{
	int	i, startValue;

	day_no = (int) (selectDate % 7L);
	sprintf (dayName, "%-9.9s", day_of_wk [day_no]);

	LoadCurrDay ();
	
	tab_row = 6;
	tab_col = 18;

	errorDay = -1;
	errorPage = -1;
	errorLine = -1;

	do 
	{
		heading (2);
		scn_display (2);
		if (errorPage == -1)
		{
			if (overlap)
				_edit (2, 0, label ("duration"));
			else
				edit (2);
		}
		else
			_edit (2, errorLine, label ("duration"));

		if (restart)
			break;

		delete_cal = FALSE;
		if (lcount [2] == 0)
		{
			i = prmptmsg (ML (mlPcMess001), "YyNn", 5, 20);
			move (0,20);
			cl_line ();
			if (i == 'N' || i == 'n')
				continue;
			else
			{
				delete_cal = TRUE;
				break;
			}
		}
	} while (!AllLinesOK ());

	if (!restart)
	{
		if (overlap)
		{
			getval (0);
			strcpy (pccl_rec.co_no, comm_rec.co_no);
			strcpy (pccl_rec.br_no, comm_rec.est_no);
			pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
			pccl_rec.act_date  = date_overlap;
			pccl_rec.act_time  = local_rec.actTime;
			cc = find_rec (pccl, &pccl_rec, COMPARISON, "u");
			if (!cc)
			{
				pccl_rec.duration = local_rec.duration;

				cc = abc_update (pccl, &pccl_rec);
				if (cc)
					file_err (cc, pccl, "DBUPDATE");
			}

			startValue = 1;
		}
		else
			startValue = 0;

		/*------------------------------
		| Delete Old Records For Today |
		------------------------------*/
		strcpy (pccl_rec.co_no, comm_rec.co_no);
		strcpy (pccl_rec.br_no, comm_rec.est_no);
		pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
		pccl_rec.act_date  = rslt_date;
		pccl_rec.act_time  = 0L;
		cc = find_rec (pccl, &pccl_rec, GTEQ, "u");
		while (!cc &&
		       !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
		       !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
		       pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash &&
		       pccl_rec.act_date == rslt_date)
		{
			cc = abc_delete (pccl);
			if (cc)
				file_err (cc, pccl, "DBDELETE");

			strcpy (pccl_rec.co_no, comm_rec.co_no);
			strcpy (pccl_rec.br_no, comm_rec.est_no);
			pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
			pccl_rec.act_date = rslt_date;
			pccl_rec.act_time = 0L;
			cc = find_rec (pccl, &pccl_rec, GTEQ, "u");
		}

		/*------------------
		| Update pccl file |
		------------------*/
		for (i = startValue; i < lcount [2]; i++)
		{
			getval (i);
			strcpy (pccl_rec.co_no, comm_rec.co_no);
			strcpy (pccl_rec.br_no, comm_rec.est_no);
			pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
			pccl_rec.act_date = rslt_date;
			pccl_rec.act_time = local_rec.actTime;
			pccl_rec.duration = local_rec.duration;

			cc = abc_add (pccl, &pccl_rec);
			if (cc)
				file_err (cc, pccl, "DBADD");
		}
	}

	restart = FALSE;
	return;
}

/*------------------------------
| Put lines in order and check |
| for overlapping times        |
------------------------------*/
int
AllLinesOK (
 void)
{
	int	i, j, ins, nextEntry; 
	int	no_lines, noInOrder;
	int	inserted;
	long	st_time, end_time, day_start;

	if (lcount [2] == 0)
		return (FALSE);

	if (lcount [2] == 1 && overlap)
		return (TRUE);

	/*--------------------
	| Put lines in order |
	--------------------*/
	no_lines = lcount [2];

	if (overlap)
	{
		noInOrder = 1;
		orderRec [0].day = storeRec [0].day;
		strcpy (orderRec [0].dayName, storeRec [0].dayName);
		orderRec [0].actTime = storeRec [0].actTime;
		orderRec [0].duration = storeRec [0].duration;
		i = 1;
	}
	else
	{
		noInOrder = 0;
		i = 0;
	}

	for (; i < no_lines; i++)
	{
	    inserted = FALSE;
	    for (j = 0; j < noInOrder; j++)
	    {
		/*--------
		| Insert |
		--------*/
		if (storeRec [i].day < orderRec [j].day ||
		    (storeRec [i].day == orderRec [j].day &&
		    storeRec [i].actTime <= orderRec [j].actTime))
		{
		    /*--------------------------------
		    | Shuffle lines ready for insert |
		    --------------------------------*/
		    for (ins = noInOrder; ins > j; ins--)
		    {
			orderRec [ins].day = orderRec [ins - 1].day;
			strcpy (orderRec [ins].dayName, orderRec [ins - 1].dayName);
			orderRec [ins].actTime = orderRec [ins - 1].actTime;
			orderRec [ins].duration = orderRec [ins - 1].duration;
		    }
		    noInOrder++;

		    orderRec [j].day = orderRec [i].day;
		    strcpy (orderRec [j].dayName, storeRec [i].dayName);
		    orderRec [j].actTime = storeRec [i].actTime;
		    orderRec [j].duration = storeRec [i].duration;

		    inserted = TRUE;
		    break;
		}
	    }

	    /*--------
	    | Append |
	    --------*/
	    if (!inserted)
	    {
		    strcpy (orderRec [noInOrder].dayName, storeRec [i].dayName);
		    orderRec [noInOrder].day 	 	= storeRec [i].day;
		    orderRec [noInOrder].actTime 	= storeRec [i].actTime;
		    orderRec [noInOrder++].duration = storeRec [i].duration;
	    }
	}

	scn_set (2);
	lcount [2] = 0;
	for (i = 0; i < noInOrder; i++)
	{
		strcpy (local_rec.dayName, orderRec [i].dayName);
		local_rec.day 			= orderRec [i].day;
		local_rec.actTime 		= orderRec [i].actTime;
		local_rec.duration 		= orderRec [i].duration;
		putval (lcount [2]++);

		strcpy (storeRec [i].dayName, orderRec [i].dayName);
		storeRec [i].day 		= orderRec [i].day;
		storeRec [i].actTime 	= orderRec [i].actTime;
		storeRec [i].duration 	= orderRec [i].duration;
	}

	/*-----------------------------
	| Check For Overlapping Times |
	-----------------------------*/
	for (i = 0; i < lcount [2]; i++)
	{
		nextEntry = i + 1;

		/*----------------------------------
		| Check For Overlap On Next Record |
		----------------------------------*/
		if (nextEntry >= lcount [2])
			return (CheckNextRec (i));

		day_start 	= (long) (storeRec [nextEntry].day - 1) * DAY_MINS;
		day_start 	+= storeRec [nextEntry].actTime;

		st_time 	= (long) (storeRec [i].day - 1) * DAY_MINS;
		st_time 	+= storeRec [i].actTime;

		end_time 	= st_time + storeRec [i].duration;

		if ((end_time <= st_time &&
		     ((day_start >= st_time && day_start <= WEEK_MINS) ||
		      (day_start >= 0 && day_start < end_time))) ||
		    (st_time < end_time &&
		     day_start >= st_time && day_start < end_time))
		{
			errorDay = storeRec [i].day;
			errorPage = i / TABLINES;
			errorLine = i % TABLINES;
			return (FALSE);
		}
	}

	return (TRUE);
}

/*----------------------------------
| Check For Overlap On Next Record |
----------------------------------*/
int
CheckNextRec (
 int line_no)
{
	long	end_time, day_start;

	strcpy (pccl_rec.co_no, comm_rec.co_no);
	strcpy (pccl_rec.br_no, comm_rec.est_no);
	pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
	pccl_rec.act_date = rslt_date;
	pccl_rec.act_time = DAY_MINS + 1;
	cc = find_rec (pccl, &pccl_rec, GTEQ, "r");
	if (!cc && 
	    !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
	    !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
	    pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash)
	{
		day_start = pccl_rec.act_time;
		day_start = day_start / 60L;
		day_start += (pccl_rec.act_date * 24L);

		end_time = storeRec [line_no].actTime;
		end_time += storeRec [line_no].duration;
		end_time = end_time / 60L;
		end_time += (rslt_date * 24L);

		if (end_time > day_start)
		{
			errorDay = storeRec [line_no].day;
			errorPage = line_no / TABLINES;
			errorLine = line_no;
			return (FALSE);
		}
	}

	return (TRUE);
}

/*-----------------------------
| Determine Amount Of Current |
| Day Already Allocated       |
-----------------------------*/
int
LoadCurrDay (
 void)
{
	int	day_found;
	int	curr_day;
	long	end_time;

	init_vars (2);
	lcount [2] = 0;

	overlap = FALSE;
	day_found = FALSE;

	strcpy (pccl_rec.co_no, comm_rec.co_no);
	strcpy (pccl_rec.br_no, comm_rec.est_no);
	pccl_rec.hhrs_hash 	= rgrs_rec.hhrs_hash;
	pccl_rec.act_date 	= rslt_date;
	pccl_rec.act_time 	= 0L;
	cc = find_rec (pccl, &pccl_rec, LTEQ, "r");

	if (!cc && 
	    !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
	    !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
	    pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash &&
	    pccl_rec.act_date < rslt_date)
	{
		end_time =  pccl_rec.duration;
		end_time += pccl_rec.act_time;
		end_time =  end_time / 60L;
		end_time += (pccl_rec.act_date * 24L);

		/*--------------------------------
		| Previous record overlaps today |
		| so add into table              |
		--------------------------------*/
		if (end_time > (rslt_date * 24L))
		{		
			overlap = TRUE;

			date_overlap = pccl_rec.act_date;
			curr_day = (int) (pccl_rec.act_date % 7L);
			sprintf (local_rec.dayName, "%-9.9s", day_of_wk [curr_day]);
			local_rec.actTime = pccl_rec.act_time;
			local_rec.duration = pccl_rec.duration;
			local_rec.day = curr_day;

			storeRec [lcount [2]].day = curr_day;
			sprintf (storeRec [lcount [2]].dayName, "%-9.9s", local_rec.dayName);

			storeRec [lcount [2]].actTime = local_rec.actTime;
			storeRec [lcount [2]].duration = local_rec.duration;

			putval (lcount [2]++);
		}
	}

	strcpy (pccl_rec.co_no, comm_rec.co_no);
	strcpy (pccl_rec.br_no, comm_rec.est_no);
	pccl_rec.hhrs_hash = rgrs_rec.hhrs_hash;
	pccl_rec.act_date = rslt_date;
	pccl_rec.act_time = 0L;
	cc = find_rec (pccl, &pccl_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (pccl_rec.co_no, comm_rec.co_no) &&
	       !strcmp (pccl_rec.br_no, comm_rec.est_no) &&
	       pccl_rec.hhrs_hash == rgrs_rec.hhrs_hash &&
	       pccl_rec.act_date == rslt_date)
	{
		day_found = TRUE;

		local_rec.day = day_no;
		sprintf (local_rec.dayName, "%-9.9s", dayName);

		local_rec.actTime = pccl_rec.act_time;
		local_rec.duration = pccl_rec.duration;

		storeRec [lcount [2]].day = day_no;
		sprintf (storeRec [lcount [2]].dayName,"%-9.9s",local_rec.dayName);
		storeRec [lcount [2]].actTime = local_rec.actTime;
		storeRec [lcount [2]].duration = local_rec.duration;

		putval (lcount [2]++);

		cc = find_rec (pccl, &pccl_rec, NEXT, "r");
	}

	return (day_found);
}

void
tab_other (
 int line_no)
{
	if (line_no == 0 && overlap == TRUE)
		FLD ("actTime") = NA;
	else
		FLD ("actTime") = YES;
	
	return;
}

int
heading (
 int scn)
{
	char	tmp_date [31];

	if (!restart)
	{
		clear ();

		rv_pr (ML (mlPcMess002), 29, 0, 1);

		move (0, 1);
		line (80);

		if (scn == 1)
		{
			box (0, 2, 80, 1);
		}

		if (scn == 2)
		{
			box (0, 2, 80, 1);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (18, 5, 44, 9);
			CalcXtdDate (rslt_date);
			sprintf (tmp_date, "%-30.30s", local_rec.tempDtdesc);
			clip (tmp_date);
			sprintf (err_str, " %s ", tmp_date);
			rv_pr (err_str, (80 - strlen (err_str)) / 2, 5, 1);
			if (errorDay >= 0)
			{
				sprintf 
				(
					err_str,
					ML (mlPcMess139),
					day_of_wk [errorDay], 
					errorPage + 1, 
					(errorLine % TABLINES) + 1
				);
				rv_pr (err_str, (80 - strlen (err_str)) / 2, 20, 1);
			}
		}

		line_at (21,0,80);
		print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,40, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_set (scn);
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("rsrc_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.rsrc, "GLOBAL  ");
			strcpy (local_rec.rsrcDesc, ML ("Generate System-Wide Calendar"));
			DSP_FLD ("rsrcDesc");
			rgrs_rec.hhrs_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------
		| Validate resource code |
		------------------------*/
		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.rsrc);
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPcMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (rgrs_rec.cal_sel [0] == 'G')
		{
			print_mess (ML (mlPcMess059));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.rsrcDesc, "%-40.40s", rgrs_rec.desc);
		DSP_FLD ("rsrcDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("actTime"))
	{
		if (last_char == INSLINE)
			return (InsertLine ());

		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		local_rec.actTime = rnd_time (local_rec.actTime, timeRes);

		storeRec [line_cnt].actTime = local_rec.actTime;
		storeRec [line_cnt].day = day_no;
		local_rec.day = day_no;

		sprintf (local_rec.dayName, "%-9.9s", dayName);
		strcpy (storeRec [line_cnt].dayName, local_rec.dayName);

		DSP_FLD ("dayName");
		DSP_FLD ("actTime");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("duration"))
	{
		local_rec.duration = rnd_time (local_rec.duration, timeRes);
		storeRec [line_cnt].duration = local_rec.duration;

		DSP_FLD ("duration");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("xDate"))
	{
		if (local_rec.xDate < local_rec.startDate ||
		    local_rec.xDate > local_rec.endDate)
		{
			print_mess (ML (mlPcMess061));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		CalcXtdDate (local_rec.xDate);
		sprintf (local_rec.dateDesc, "%-30.30s", local_rec.tempDtdesc);
		DSP_FLD ("dateDesc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
InsertLine (
 void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	for (i = line_cnt, line_cnt = lcount [2]; line_cnt > i; line_cnt--)
	{
		getval (line_cnt - 1);
		storeRec [line_cnt].day = local_rec.day;
		strcpy (storeRec [line_cnt].dayName, local_rec.dayName);
		storeRec [line_cnt].actTime = local_rec.actTime;
		storeRec [line_cnt].duration = local_rec.duration;

		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	strcpy (local_rec.dayName, "         ");
	local_rec.actTime = 0L;
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
DeleteLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		storeRec [line_cnt].day = local_rec.day;
		strcpy (storeRec [line_cnt].dayName, local_rec.dayName);
		storeRec [line_cnt].actTime = local_rec.actTime;
		storeRec [line_cnt].duration = local_rec.duration;

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
	work_open ();
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

/*----------------------------
| Construct date description |
----------------------------*/
int
CalcXtdDate (
 long int use_date)
{
	char	day_suffix [3];

	DateToDMY (use_date, &tmpDMY [0],&tmpDMY [1],&tmpDMY [2]);

	switch (tmpDMY [0])
	{
	case 1:
	case 21:
	case 31:
		strcpy (day_suffix, "st");
		break;

	case 2:
	case 22:
		strcpy (day_suffix, "nd");
		break;

	case 3:
	case 23:
		strcpy (day_suffix, "rd");
		break;

	default:
		strcpy (day_suffix, "th");
		break;
	}

	sprintf (local_rec.tempDtdesc, 
		"%s %2d%-2.2s %s %4d",
		day_of_wk [ (int) (use_date % 7L)],
		tmpDMY [0],
		day_suffix,
		mth_name [tmpDMY [1] - 1],
		tmpDMY [2]);

	return (EXIT_SUCCESS);
}

void
initML (
 void)
{
	day_of_wk [0] = strdup (ML (mlStdMess251)); 
	day_of_wk [1] = strdup (ML (mlStdMess252)); 
	day_of_wk [2] = strdup (ML (mlStdMess253)); 
	day_of_wk [3] = strdup (ML (mlStdMess254)); 
	day_of_wk [4] = strdup (ML (mlStdMess255)); 
	day_of_wk [5] = strdup (ML (mlStdMess256)); 
	day_of_wk [6] = strdup (ML (mlStdMess257)); 
}

