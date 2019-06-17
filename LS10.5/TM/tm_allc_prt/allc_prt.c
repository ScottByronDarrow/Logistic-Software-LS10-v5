/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: allc_prt.c,v 5.4 2002/07/17 09:58:11 scott Exp $
|  Program Name  : (tm_allc_prt.c) 
|  Program Desc  : (Telemarketing Lead Allocation Print)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 31/07/91         |
|---------------------------------------------------------------------|
| $Log: allc_prt.c,v $
| Revision 5.4  2002/07/17 09:58:11  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/14 02:54:02  scott
| Updated to add app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: allc_prt.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_allc_prt/allc_prt.c,v 5.4 2002/07/17 09:58:11 scott Exp $";

#define		X_OFF	20
#define		Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_tm_mess.h>
#include	<ml_std_mess.h>

FILE	*fout;
FILE	*fsort;
char	*sptr;

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmpmRecord	tmpm_rec;
struct exsfRecord	exsf_rec;

int		data_found = FALSE;

extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	systemDate [11];
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
	char	lp_str [3];
	char	back [4];
	char	onight [4];
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
	{1, LIN, "lpno", 7, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1","Printer Number   ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "", "Background       ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.back}, 
	{1, LIN, "onight", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "", "Overnight        ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.onight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void	RunProgram 			 (char *, char *);
void	shutdown_prog 		 (void);
void	OpenDB 				 (void);
void	CloseDB 			 (void);
int		spec_valid 			 (int);
void	SrchTmop 			 (char *);
void	SrchExsf 			 (char *);
void	Process 			 (void);
void	ProcessSorted 		 (void);
void	HeadingOutput 		 (void);
int		heading 			 (int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;

	if (argc != 2 && argc != 4)
	{
		/*--------------------------------------------------
		| Usage: %s <description> 						   |
		|        %s <lpno> <start operator> <end operator> |
		--------------------------------------------------*/
		print_at (0, 0, mlTmMess703, argv [0]);
		print_at (0, 0, mlTmMess704, argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.cur_ldate = TodaysDate ();

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (argc == 2)
	{
		SETUP_SCR (vars);
	
		init_scr ();		/*  sets terminal from termcap	*/
		set_tty ();              /*  get into raw mode		*/
		set_masks ();		/*  setup print using masks	*/
	
		init_vars (1);		/*  set default values		*/
	
		while (prog_exit == 0)
		{
			/*=====================
			| Reset control flags |
			=====================*/
			search_ok	= TRUE;
			entry_exit	= TRUE;
			prog_exit	= FALSE;
			restart		= FALSE;
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
	
			RunProgram (argv [0], argv [1]);
			break;   
		}
	}
	else
	{
		local_rec.lpno = atoi (argv [1]);
		sprintf (local_rec.st_op, "%-14.14s", argv [2]);
		sprintf (local_rec.end_op, "%-14.14s", argv [3]);
		Process ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
RunProgram (
	char *prog_name, 
	char *prog_desc)
{
	sprintf (local_rec.lp_str, "%d",local_rec.lpno);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT", 
				"ONIGHT", 
				prog_name, 
				local_rec.lp_str, 
				local_rec.st_op, 
				local_rec.end_op, 
				prog_desc, (char *)0);
		else
			return;
	}
	else
	if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name, 
				prog_name, 
				local_rec.lp_str, 
				local_rec.st_op, 
				local_rec.end_op, (char *)0);
		else
			return;
	}
	else 
	{
		execlp (prog_name, 
			prog_name, 
			local_rec.lp_str, 
			local_rec.st_op, 
			local_rec.end_op, (char *)0);
	}
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

void
OpenDB (
	void)
{
	abc_dbopen ("data");
	open_rec (tmop, tmop_list,TMOP_NO_FIELDS,"tmop_id_no");
	open_rec (tmpm, tmpm_list,TMPM_NO_FIELDS,"tmpm_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose (tmop);
	abc_fclose (tmpm);
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
			/*---------------------
			| Operator Not Found. |
			---------------------*/
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

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.end_op);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Operator Not Found. |
			---------------------*/
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.end_op_name, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("end_op_name");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
			local_rec.lpno = get_lpno (0);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.back, "Yes");
		else
			strcpy (local_rec.back, "No ");
	
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onight, "Yes");
		else
			strcpy (local_rec.onight, "No ");
	
		DSP_FLD ("onight");
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
	work_open ();
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%2.2s", key_val);
	save_rec ("#Salesman.", "#Salesman Full Name.");
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

 	fsort = sort_open ("ld_allc");

	strcpy (tmpm_rec.co_no, comm_rec.co_no);
	sprintf (tmpm_rec.pro_no, "%-8.8s", " ");
	cc = find_rec (tmpm, &tmpm_rec, GTEQ, "r");
	while (!cc && !strcmp (tmpm_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (tmpm_rec.op_code, local_rec.st_op) < 0 ||
		    strcmp (tmpm_rec.op_code, local_rec.end_op) > 0)
		{
			cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
			continue;
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s",tmpm_rec.op_code);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
			continue;
		}

		sprintf (sort_str, 
			"%05d %-14.14s %-8.8s %8ld %8ld %-40.40s \n", 
			tmop_rec.campaign_no, 
			tmop_rec.op_id, 
			tmpm_rec.pro_no, 
			tmpm_rec.n_phone_date, 
			tmpm_rec.hhpm_hash, 
			tmop_rec.op_name);

		sort_save (fsort, sort_str);

		data_found = TRUE;

		cc = find_rec (tmpm, &tmpm_rec, NEXT, "r");
	}

	if (data_found)
		ProcessSorted ();

	return;
}

void
ProcessSorted (
	void)
{
	char	*sptr;
	int		first_time = TRUE;
	char	curr_op [15];
	char	prev_op [15];

	abc_selfield (tmpm, "tmpm_hhpm_hash");

	fsort = sort_sort (fsort, "ld_allc");

	HeadingOutput ();

	dsp_screen ("Printing Allocated Leads", 
		comm_rec.co_no, 
		comm_rec.co_name);

	sptr = sort_read (fsort);
	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';

		sprintf (curr_op, "%-14.14s", sptr + 6);

		if (first_time)
			strcpy (prev_op, curr_op);

		cc = find_hash (tmpm, &tmpm_rec,COMPARISON,"r",atol (sptr + 37));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		if (strcmp (curr_op, prev_op) || first_time)
		{
			if (!first_time)
				fprintf (fout, ".PA\n");

			fprintf (fout, 
				"| %-14.14s %-40.40s             |                                |                |\n", 
				sptr + 6, 
				sptr + 46);

			strcpy (prev_op, curr_op);
			first_time = FALSE;
		}

		fprintf (fout, 
			"| %-8.8s | %-40.40s | %-15.15s | %-30.30s | %-14.14s |\n", 
			tmpm_rec.pro_no, 
			tmpm_rec.name, 
			tmpm_rec.phone_no, 
			tmpm_rec.cont_name1, 
			tmpm_rec.lst_op_code);

		dsp_process ("Prospect :", tmpm_rec.pro_no);

		sptr = sort_read (fsort);
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
	sort_delete (fsort, "ld_allc");
}

void
HeadingOutput (
	void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat during (POPEN)", errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.lpno);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".7\n");
	fprintf (fout, ".L121\n");

	fprintf (fout, ".E Lead Allocations\n");
	fprintf (fout, ".ECOMPANY : %s - %s\n", comm_rec.co_no, comm_rec.co_name);

	fprintf (fout, "=========================================================================================================================\n");
	fprintf (fout, "|  Lead  |                Lead Name                 |  Phone Number   |           Contact              | Last Operator  |\n");
	fprintf (fout, "|--------|------------------------------------------|-----------------|--------------------------------|----------------|\n");
	fprintf (fout, ".R=========================================================================================================================\n");

	return;
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
	int scn)
{
	char	showDate [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
	
		strcpy (showDate, DateToString (TodaysDate ()));
		rv_pr (showDate, 70,0,0);

		line_at (1, 0,80);

		/*---------------------------------------
		| Telemarketing Lead Allocation Report. |
		---------------------------------------*/
		sprintf (err_str, " %s ", ML (mlTmMess055));
		rv_pr (err_str, 25,0,1);

		box (0, 3,80,6);
		line_at (6, 1,79);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
