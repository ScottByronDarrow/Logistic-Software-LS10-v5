/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: clbk_dsp.c,v 5.3 2001/11/14 04:08:19 scott Exp $
|  Program Name  : (tm_clbk_dsp.c) 
|  Program Desc  : (Telemarketing Call Back Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 19/07/91         |
|---------------------------------------------------------------------|
| $Log: clbk_dsp.c,v $
| Revision 5.3  2001/11/14 04:08:19  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: clbk_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_clbk_dsp/clbk_dsp.c,v 5.3 2001/11/14 04:08:19 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_tm_mess.h>
#include	<ml_std_mess.h>

FILE	*fin;
FILE	*fsort;
char	*sptr;
char	usr_fname [100];

char	*UNDER_LINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

int		by_user;

extern	int		TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmpmRecord	tmpm_rec;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	st_op [15];
	char	st_op_name [41];
	char	end_op [15];
	char	end_op_name [41];
	long	prt_date;
	long	cur_ldate;
	int		lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "st_op", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Start Operator   ", "Enter Start Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.st_op}, 
	{1, LIN, "st_op_name", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_op_name}, 
	{1, LIN, "end_op", 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "End Operator     ", "Enter End Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.end_op}, 
	{1, LIN, "end_op_name", 5, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_op_name}, 
	{1, LIN, "prt_date", 7, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Call Back Date   ", " Enter Call Back Date ", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.prt_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void SrchTmop (char *key_val);
void Process (void);
void ProcessSorted (void);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc,
	char *argv [])
{

	TruePosition	= TRUE;

	local_rec.cur_ldate = TodaysDate ();

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = 0;
		restart = 0;
		init_vars (1);	

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
			Process ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);   
}

void
OpenDB (
	void)
{
	abc_dbopen ("data");
	open_rec (tmop,tmop_list,TMOP_NO_FIELDS,"tmop_id_no");
	open_rec (tmpm,tmpm_list,TMPM_NO_FIELDS,"tmpm_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose (tmpm);
	abc_fclose (tmop);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	if (LCHECK ("st_op"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.st_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------
			| Operator Not Found |
			--------------------*/
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.st_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("st_op_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_op"))
	{

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.end_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------
			| Operator Not Found | 
			--------------------*/
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.end_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("end_op_name");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("prt_date"))
	{
		if (dflt_used)
			local_rec.prt_date = local_rec.cur_ldate;
		return (EXIT_SUCCESS);
	}
	
	return (EXIT_SUCCESS);
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmop (
	char *key_val)
{
	work_open ();
	strcpy (tmop_rec.co_no,comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	save_rec ("#Operator I.D.","#Operator Full Name.");
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
		      !strncmp (tmop_rec.op_id, key_val,strlen (key_val)))
	{
		cc = save_rec (tmop_rec.op_id, tmop_rec.op_name);
		if (cc)
			break;

		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmop_rec.co_no,comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", temp_str);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmop, "DBFIND");
}

void
Process (
	void)
{
	char	sort_str [256];

	abc_selfield (tmpm, "tmpm_id_no3");

 	fsort = sort_open ("call_bk");

	sprintf (local_rec.st_op, "%-14.14s", local_rec.st_op);
	sprintf (local_rec.end_op, "%-14.14s", local_rec.end_op);

	strcpy (tmpm_rec.co_no, comm_rec.co_no);
	sprintf (tmpm_rec.pro_no, "%-8.8s", " ");
	cc = find_rec (tmpm, &tmpm_rec, GTEQ, "r");
	while (!cc && !strcmp (tmpm_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (tmpm_rec.op_code, local_rec.st_op) >= 0 && 
	           strcmp (tmpm_rec.op_code, local_rec.end_op) <= 0 &&
		  (tmpm_rec.call_bk_date != 0L ||
		  (strcmp (tmpm_rec.call_bk_time, "00:00"))))
		{
			if (tmpm_rec.call_bk_date > local_rec.prt_date)
			{
				cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
				continue;
			}

			sprintf (tmop_rec.op_id, "%-14.14s",tmpm_rec.op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
				continue;
			}

			sprintf (sort_str, 
				"%05d %-14.14s %8ld %8ld %-40.40s \n", 
				tmop_rec.campaign_no,
				tmop_rec.op_id,
				tmpm_rec.call_bk_date,
				tmpm_rec.hhpm_hash,
				tmop_rec.op_name);

			sort_save (fsort, sort_str);
		}

		cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
	}

	ProcessSorted ();
	return;
}

void
ProcessSorted (
	void)
{
	char	*sptr;
	char	data_str [256];
	int		first_time = TRUE;
	char	curr_op [15];
	char	prev_op [15];

	abc_selfield (tmpm, "tmpm_hhpm_hash");
	/*-----------------
	| Open Dsp window |
	-----------------*/
	Dsp_prn_open (0, 3, 15, 
		     "                        Telemarketing Call Back Display                       ", (char *) 0,
	            (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);

	Dsp_saverec ("  Lead  | Lead Name           | Phone No  | Contact name   | Call Back        ");
	Dsp_saverec ("");
	Dsp_saverec (" [FN03][FN05][FN14][FN15][FN16]  ");
	
	fsort = sort_sort (fsort,"call_bk");

	sptr = sort_read (fsort);

	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';

		sprintf (curr_op, "%-14.14s", sptr + 8);

		if (first_time)
			strcpy (prev_op, curr_op);

		cc = find_hash (tmpm,&tmpm_rec,COMPARISON,"r",atol (sptr + 32));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		if (strcmp (curr_op, prev_op) || first_time)
		{
			if (!first_time)
				Dsp_saverec (UNDER_LINE);

			sprintf (data_str, 
				"^1 %-14.14s  %-40.40s ^6",
				sptr + 8,
				sptr + 41);

			Dsp_saverec (data_str);

			strcpy (prev_op, curr_op);
			first_time = FALSE;
		}

		sprintf (data_str, 
			" %-8.8s^E %-20.20s^E %-10.10s^E %-15.15s^E%-10.10s^E %-5.5s ",
			tmpm_rec.pro_no,
			tmpm_rec.name,
			tmpm_rec.phone_no,
			tmpm_rec.cont_name1,
			DateToString (tmpm_rec.call_bk_date),
			tmpm_rec.call_bk_time);

		Dsp_saverec (data_str);

		sptr = sort_read (fsort);
	}

	if (!first_time)
		Dsp_saverec (UNDER_LINE);

	sort_delete (fsort,"call_bk");

	Dsp_srch ();

	Dsp_close ();
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
	int scn)
{
	char	hdng_date [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		strcpy (hdng_date,DateToString (TodaysDate ()));
		rv_pr (hdng_date,70,0,0);

		line_at (1,0,80);

		/*----------------------------------
		| Telemarketing Call Back Display. |
		----------------------------------*/
		sprintf (err_str, " %s ", ML (mlTmMess056));
		rv_pr (err_str ,25,0,1);

		box (0,3,80,4);

		line_at (6,1,79);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
