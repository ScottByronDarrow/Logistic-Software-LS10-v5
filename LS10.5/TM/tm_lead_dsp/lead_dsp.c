/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lead_dsp.c,v 5.3 2001/11/14 06:44:56 scott Exp $
|  Program Name  : (tm_lead_dsp.c)
|  Program Desc  : (Telemarketing Lead Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 29/07/91         |
|---------------------------------------------------------------------|
| $Log: lead_dsp.c,v $
| Revision 5.3  2001/11/14 06:44:56  scott
| Updated to convert to app.schema
| Updated to clean code
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lead_dsp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_lead_dsp/lead_dsp.c,v 5.3 2001/11/14 06:44:56 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_tm_mess.h>

#define	LEAD_REP	 (rep_type [0] == 'L')
#define	VALID_LEAD	 (strcmp (tmpm_rec.op_code, local_rec.st_op) >= 0 && \
					  strcmp (tmpm_rec.op_code, local_rec.end_op) <= 0 && \
			 		 (tmpm_rec.n_phone_date != 0L || \
					  strcmp (tmpm_rec.n_phone_time, "00:00")))

#define	VALID_VST	 (strcmp (tmpm_rec.sman_code, local_rec.st_rep) >= 0 &&\
	           		  strcmp (tmpm_rec.sman_code, local_rec.end_rep) <= 0 &&\
		   	 		 (tmpm_rec.n_visit_date != 0L || \
		   			 strcmp (tmpm_rec.n_visit_time, "00:00")))

FILE	*fin;
FILE	*fsort;
char	*sptr;
char	usr_fname [100];

char	*UNDER_LINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

int		by_user;

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmpmRecord	tmpm_rec;
struct exsfRecord	exsf_rec;

char	rep_type [2];

extern	int		TruePosition;
/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	st_op [15];
	char	st_op_name [41];
	char	end_op [15];
	char	end_op_name [41];
	char	st_rep [3];
	char	st_rep_name [41];
	char	end_rep [3];
	char	end_rep_name [41];
	long	prt_date;
	long	cur_ldate;
	int		lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "st_op", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Start Operator   ", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.st_op}, 
	{1, LIN, "st_op_name", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_op_name}, 
	{1, LIN, "end_op", 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "End Operator     ", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.end_op}, 
	{1, LIN, "end_op_name", 5, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_op_name}, 
	{1, LIN, "prt_date1", 7, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Next Phone Date  ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.prt_date}, 
	{2, LIN, "st_rep", 4, 12, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Start Rep.       ", "", 
		YES, NO, JUSTRIGHT, "", "", local_rec.st_rep}, 
	{2, LIN, "st_rep_name", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_rep_name}, 
	{2, LIN, "end_rep", 5, 12, CHARTYPE, 
		"UU", "          ", 
		" ", "", "End Rep.         ", "", 
		YES, NO, JUSTRIGHT, "", "", local_rec.end_rep}, 
	{2, LIN, "end_rep_name", 5, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_rep_name}, 
	{2, LIN, "prt_date2", 7, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Next Visit Date  ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.prt_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int 	spec_valid 		 (int);
void 	SrchTmop 		 (char *);
void 	SrchExsf 		 (char *);
void 	Process 		 (void);
void 	ProcessSorted 	 (void);
int 	heading 		 (int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	char	systemDate [11];
	int		i;

	TruePosition	=	TRUE;

	if (argc != 2 || (argc == 2 && argv [1][0] != 'L' && argv [1][0] != 'V'))
	{
		print_at (0, 0,mlTmMess701, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (rep_type, "%-1.1s", argv [1]);

	strcpy (systemDate, DateToString (TodaysDate ()));
	local_rec.cur_ldate = TodaysDate ();

	SETUP_SCR (vars);

	if (!LEAD_REP)
	{
		for (i = label ("st_op"); i <= label ("prt_date1"); i++)
			vars [i].scn = 2;

		for (i = label ("st_rep"); i <= label ("prt_date2"); i++)
			vars [i].scn = 1;
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok 	= TRUE;
		entry_exit 	= TRUE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
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
	if (LEAD_REP)
		open_rec (tmop, tmop_list,TMOP_NO_FIELDS,"tmop_id_no");
	else
		open_rec (exsf, exsf_list,EXSF_NO_FIELDS,"exsf_id_no");

	open_rec (tmpm, tmpm_list,TMPM_NO_FIELDS,"tmpm_id_no");
}

void
CloseDB (void)
{
	abc_fclose (tmpm);
	if (LEAD_REP)
		abc_fclose (tmop);
	else
		abc_fclose (exsf);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	if (LCHECK ("st_op"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.st_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.st_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("st_op_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_rep"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", local_rec.st_rep);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.st_rep_name, "%-40.40s", exsf_rec.salesman);
		
		DSP_FLD ("st_rep_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_rep"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", local_rec.end_rep);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.end_rep_name, "%-40.40s",exsf_rec.salesman);
		
		DSP_FLD ("end_rep_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_op"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.end_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.end_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("end_op_name");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("prt_date1") || LCHECK ("prt_date2"))
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
	_work_open (14,0,40);
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	save_rec ("#Operator I.D.", "#Operator Full Name.");
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

	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", temp_str);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmop, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchExsf (
	char *key_val)
{
	_work_open (2,0,40);
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%2.2s", key_val);
	save_rec ("#No", "#Salesman Full Name.");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no) &&
		      !strncmp (exsf_rec.salesman_no, key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

void
Process (
	void)
{
	char	sort_str [256];

	abc_selfield (tmpm, "tmpm_id_no3");

 	fsort = sort_open ("nxt_ph");

	if (LEAD_REP)
	{
		sprintf (local_rec.st_op, "%-14.14s", local_rec.st_op);
		sprintf (local_rec.end_op, "%-14.14s", local_rec.end_op);
	}
	else
	{
		sprintf (local_rec.st_op, "%2.2s", local_rec.st_op);
		sprintf (local_rec.end_op, "%2.2s", local_rec.end_op);
	}

	strcpy (tmpm_rec.co_no, comm_rec.co_no);
	sprintf (tmpm_rec.pro_no, "%-8.8s", " ");
	cc = find_rec (tmpm, &tmpm_rec, GTEQ, "r");
	while (!cc && !strcmp (tmpm_rec.co_no, comm_rec.co_no))
	{
		if ((LEAD_REP && VALID_LEAD) || (!LEAD_REP && VALID_VST))
		{
			if ((LEAD_REP && tmpm_rec.n_phone_date > local_rec.prt_date) ||
			    (!LEAD_REP && tmpm_rec.n_visit_date > local_rec.prt_date))
			{
				cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
				continue;
			}

			if (LEAD_REP)
			{
				sprintf (tmop_rec.op_id, "%-14.14s",tmpm_rec.op_code);
				cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
					continue;
				}
			}
			else
			{
				strcpy (exsf_rec.co_no, comm_rec.co_no);
				sprintf (exsf_rec.salesman_no, "%2.2s",tmpm_rec.sman_code);
				cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
					continue;
				}
			}

			if (LEAD_REP)
			{
				sprintf 
				(
					sort_str, 
					"%05d %-14.14s %8ld %8ld %-40.40s \n", 
					tmop_rec.campaign_no, 
					tmop_rec.op_id, 
					tmpm_rec.n_phone_date, 
					tmpm_rec.hhpm_hash, 
					tmop_rec.op_name
				);
			}
			else
			{
				sprintf 
				(
					sort_str, 
					"%2.2s %8ld %8ld %-40.40s \n", 
					exsf_rec.salesman_no, 
					tmpm_rec.n_visit_date, 
					tmpm_rec.hhpm_hash, 
					exsf_rec.salesman
				);
			}
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
		     "                        Telemarketing Lead Display                            ", (char *) 0,
	           (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);

	if (LEAD_REP)
		Dsp_saverec (" Lead  | Lead Name           | Phone No  | Contact name   | Next Phone        ");
	else
		Dsp_saverec (" Lead  | Lead Name           | Phone No  | Contact name   | Next Visit        ");

	Dsp_saverec ("");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");
	
	fsort = sort_sort (fsort, "nxt_ph");

	sptr = sort_read (fsort);

	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';

		if (LEAD_REP)
			sprintf (curr_op, "%-14.14s", sptr + 6);
		else
			sprintf (curr_op, "%2.2s", sptr);

		if (first_time)
			strcpy (prev_op, curr_op);

		cc = find_hash (tmpm, &tmpm_rec,COMPARISON,"r", (LEAD_REP) ? atol (sptr + 30) : atol (sptr + 12));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		if (strcmp (curr_op, prev_op) || first_time)
		{
			if (!first_time)
				Dsp_saverec (UNDER_LINE);

			if (LEAD_REP)
			{
				sprintf (data_str, 
					"^1 %-14.14s  %-40.40s ^6", 
					sptr + 6, 
					sptr + 39);
			}
			else
			{
				sprintf (data_str, 
					"^1 %2.2s  %-40.40s ^6", 
					sptr, 
					sptr + 21);
			}

			Dsp_saverec (data_str);

			strcpy (prev_op, curr_op);
			first_time = FALSE;
		}

		sprintf (data_str, 
			" %-6.6s^E %-20.20s^E %-10.10s^E %-15.15s^E%-10.10s^E %-5.5s ", 
			tmpm_rec.pro_no, 
			tmpm_rec.name, 
			tmpm_rec.phone_no, 
			tmpm_rec.cont_name1, 
			 (LEAD_REP) ? DateToString (tmpm_rec.n_phone_date) : DateToString (tmpm_rec.n_visit_date), 
			 (LEAD_REP) ? tmpm_rec.n_phone_time : tmpm_rec.n_visit_time);

		Dsp_saverec (data_str);

		sptr = sort_read (fsort);
	}

	if (!first_time)
		Dsp_saverec (UNDER_LINE);

	sort_delete (fsort, "nxt_ph");

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
	char	headerDate [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
	
		strcpy (headerDate, DateToString (TodaysDate ()));
		rv_pr (headerDate, 70,0,0);

		line_at (1,0,80);

		if (LEAD_REP)
			rv_pr (ML (mlTmMess030), 25,0,1);
		else
			rv_pr (ML (mlTmMess031), 20,0,1);

		box (0, 3,80,4);
		line_at (6,1,79);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
