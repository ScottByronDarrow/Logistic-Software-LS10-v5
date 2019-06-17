/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ld_sheet.c,v 5.4 2002/07/17 09:58:11 scott Exp $
|  Program Name  : (tm_ld_sheet.c)
|  Program Desc  : (Telemarketing Lead Sheet Print By Rep)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 05/08/91         |
|---------------------------------------------------------------------|
| $Log: ld_sheet.c,v $
| Revision 5.4  2002/07/17 09:58:11  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/14 06:04:12  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ld_sheet.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_ld_sheet/ld_sheet.c,v 5.4 2002/07/17 09:58:11 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_tm_mess.h>

FILE	*fout;
FILE	*fsort;
char	*sptr;

#include	"schema"

struct commRecord	comm_rec;
struct tmpmRecord	tmpm_rec;
struct exsfRecord	exsf_rec;
struct tmcfRecord	tmcf_rec;
struct tmchRecord	tmch_rec;
struct tmclRecord	tmcl_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;

extern	int		TruePosition;

	int		*tmsl_rep_goto	=	&tmsl_rec.rep1_goto;

	char	*tmch2	=	"tmch2";
int	data_found = FALSE;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	int		campgn;
	char	campgn_name [61];
	int		scpt_no;
	char	scpt_desc [41];
	int		prmpt_no;
	char	prmpt_desc [31];
	int		reply_no;
	char	reply_desc [21];
	long	st_date;
	long	end_date;
	int	lpno;
	char	lp_str [3];
	char	back [4];
	char	onight [4];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campgn", 3, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign No  ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.campgn}, 
	{1, LIN, "campgn_name", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name}, 
	{1, LIN, "scpt_no", 6, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Script No    ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.scpt_no}, 
	{1, LIN, "scpt_desc", 6, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.scpt_desc}, 
	{1, LIN, "prmpt_no", 8, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Prompt No    ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.prmpt_no}, 
	{1, LIN, "prmpt_desc", 8, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.prmpt_desc}, 
	{1, LIN, "reply_no", 10, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Reply No     ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.reply_no}, 
	{1, LIN, "reply_desc", 10, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reply_desc}, 
	{1, LIN, "st_date", 12, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Start Date   ", "", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.st_date}, 
	{1, LIN, "end_date", 12, 34, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "End Date     ", "", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.end_date}, 
	{1, LIN, "lpno", 14, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No   ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "",  "Background   ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.back}, 
	{1, LIN, "onight", 16, 2, CHARTYPE, 
		"U", "          ", 
		" ", "",  "Overnight    ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.onight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	RunProgram 		(char *, char *);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchTmcf 		(char *);
void 	SrchTmsh 		(char *);
void 	SrchTmsl 		(void);
void 	SrchTmsl2 		(void);
void 	Process 		(void);
void 	ProcessSort 	(void);
void 	HeadOutput 		(void);
int	 	heading 		(int);
int 	spec_valid 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;
	if (argc != 2 && argc != 8)
	{
		/*---------------------------
		| Usage: %s <description> 	| 
		|       %s <lpno> <campaign>| 
		| <script> <prompt> <reply>	| 
		| <st. date> <end date>   	|
		---------------------------*/
		print_at (0, 0, mlTmMess703, argv [0]);
		print_at (0, 0, mlTmMess705, argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();

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
	
			if (restart) 
				continue;
	
			RunProgram (argv [0], argv [1]);
			break;   
		}
	}
	else
	{
		local_rec.lpno 		= atoi (argv [1]);
		local_rec.campgn 	= atoi (argv [2]);
		local_rec.scpt_no 	= atoi (argv [3]);
		local_rec.prmpt_no 	= atoi (argv [4]);
		local_rec.reply_no 	= atoi (argv [5]);
		local_rec.st_date 	= StringToDate (argv [6]);
		local_rec.end_date 	= StringToDate (argv [7]);
	
		strcpy (tmcf_rec.co_no, comm_rec.co_no);
		tmcf_rec.campaign_no = local_rec.campgn;
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, tmcf, "DBFIND");

		strcpy (tmsh_rec.co_no, comm_rec.co_no);
		tmsh_rec.script_no = local_rec.scpt_no;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, tmsh, "DBFIND");

		Process ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
RunProgram (
	char *prog_name, 
	char *prog_desc)
{
	char	tmp_campgn [5];
	char	tmp_script [5];
	char	tmp_prompt [5];
	char	tmp_reply [5];
	char	tmp_st_date [11];
	char	tmp_end_date [11];

	sprintf (local_rec.lp_str, "%d", local_rec.lpno);
	
	CloseDB (); 
	FinishProgram ();

	sprintf (tmp_campgn,  "%d",  local_rec.campgn);
	sprintf (tmp_script,  "%d",  local_rec.scpt_no);
	sprintf (tmp_prompt,  "%d",  local_rec.prmpt_no);
	sprintf (tmp_reply,   "%d",  local_rec.reply_no);
	sprintf (tmp_st_date , "%10.10s", DateToString (local_rec.st_date));
	sprintf (tmp_end_date, "%10.10s", DateToString (local_rec.end_date));

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT", 
				"ONIGHT", 
				prog_name, 
				local_rec.lp_str, 
				tmp_campgn, 
				tmp_script, 
				tmp_prompt, 
				tmp_reply, 
				tmp_st_date, 
				tmp_end_date, 
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
				tmp_campgn, 
				tmp_script, 
				tmp_prompt, 
				tmp_reply, 
				tmp_st_date, 
				tmp_end_date, (char *)0);
		else
			return;
	}
	else 
	{
		execlp (prog_name, 
			prog_name, 
			local_rec.lp_str, 
			tmp_campgn, 
			tmp_script, 
			tmp_prompt, 
			tmp_reply, 
			tmp_st_date, 
			tmp_end_date, (char *)0);
	}
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias (tmch2, tmch);

	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, "tmpm_hhpm_hash");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_hhcf_hash");
	open_rec (tmch2,tmch_list, TMCH_NO_FIELDS, "tmch_id_no");
	open_rec (tmcl, tmcl_list, TMCL_NO_FIELDS, "tmcl_id_no");
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose (tmpm);
	abc_fclose (tmcf);
	abc_fclose (tmch);
	abc_fclose (tmch2);
	abc_fclose (tmcl);
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campgn"))
	{
		if (SRCH_KEY)
		{
			SrchTmcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no, comm_rec.co_no);
		tmcf_rec.campaign_no = local_rec.campgn;
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Campaign not found. |
			---------------------*/
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (err_str, "%s, ", clip (tmcf_rec.c_name1));
		strcat (err_str, clip (tmcf_rec.c_name2));
		sprintf (local_rec.campgn_name, "%-60.60s", err_str);
		DSP_FLD ("campgn_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("scpt_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmsh_rec.co_no, comm_rec.co_no);
		tmsh_rec.script_no = local_rec.scpt_no;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Script Not Found. |
			-------------------*/
			print_mess (ML (mlTmMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.scpt_desc, "%-40.40s", tmsh_rec.desc);
		
		DSP_FLD ("scpt_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prmpt_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsl ();
			return (EXIT_SUCCESS);
		}

		tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
		tmsl_rec.prmpt_no = local_rec.prmpt_no;
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Prompt not found. |
			-------------------*/
			print_mess (ML (mlTmMess057));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.scpt_desc, "%-30.30s", tmsl_rec.desc);
		DSP_FLD ("prmpt_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reply_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsl2 ();
			return (EXIT_SUCCESS);
		}

		tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
		tmsl_rec.prmpt_no = local_rec.prmpt_no;
		save_rec ("#Reply No", "#Reply Description.");
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
		if (!cc)
		{
			if (tmsl_rep_goto [local_rec.reply_no - 1] == 0)
			{
				/*----------------
				| Invalid reply. |
				----------------*/
				print_mess (ML (mlTmMess058));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			switch (local_rec.reply_no)
			{
				case	1:
					strcpy (local_rec.reply_desc, tmsl_rec.rep1_desc);
				break;
				case	2:
					strcpy (local_rec.reply_desc, tmsl_rec.rep2_desc);
				break;
				case	3:
					strcpy (local_rec.reply_desc, tmsl_rec.rep3_desc);
				break;
				case	4:
					strcpy (local_rec.reply_desc, tmsl_rec.rep4_desc);
				break;
				case	5:
					strcpy (local_rec.reply_desc, tmsl_rec.rep5_desc);
				break;
				case	6:
					strcpy (local_rec.reply_desc, tmsl_rec.rep6_desc);
				break;
				case	7:
					strcpy (local_rec.reply_desc, tmsl_rec.rep7_desc);
				break;
				case	8:
					strcpy (local_rec.reply_desc, tmsl_rec.rep8_desc);
				break;
			}
			DSP_FLD ("reply_desc");
			return (EXIT_SUCCESS);
		}
		else
		{
			/*----------------
			| Invalid reply. |
			----------------*/
			print_mess (ML (mlTmMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
			local_rec.lpno = get_lpno (0);

		return (EXIT_SUCCESS);
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

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmcf (
	char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.", "#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%4d", tmcf_rec.campaign_no);
		cc = save_rec (err_str, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmcf, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsh (
	char *key_val)
{
	char	tmp_scr_no [6];

	work_open ();
	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#Script No", "#Script Description.");
	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
		      tmsh_rec.script_no >= atoi (key_val))
	{
		sprintf (tmp_scr_no, "%4d", tmsh_rec.script_no);
		cc = save_rec (tmp_scr_no, tmsh_rec.desc);
		if (cc)
			break;

		cc = find_rec (tmsh, &tmsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (temp_str);
	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmsh, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsl (
	void)
{
	char	tmp_prmpt_no [6];

	work_open ();
	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = 0;
	save_rec ("#Prompt No", "#Prompt Description.");
	cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
	while (!cc && tmsl_rec.hhsh_hash == tmsh_rec.hhsh_hash)
	{
		sprintf (tmp_prmpt_no, "%4d", tmsl_rec.prmpt_no);
		cc = save_rec (tmp_prmpt_no, tmsl_rec.desc);
		if (cc)
			break;

		cc = find_rec (tmsl, &tmsl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = atoi (temp_str);
	cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmsl, "DBFIND");
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsl2 (
	void)
{
	char	tmp_reply_no [6];
	int	i;

	work_open ();
	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = local_rec.prmpt_no;
	save_rec ("#Reply No", "#Reply Description.");
	cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	if (!cc)
	{
		for (i = 0; i < 8; i++)
		{
			if (tmsl_rep_goto [i] == 0)
				break;

			sprintf (tmp_reply_no, "%4d", i + 1);

			switch (i)
			{
				case	1:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep1_desc);
				break;
				case	2:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep2_desc);
				break;
				case	3:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep3_desc);
				break;
				case	4:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep4_desc);
				break;
				case	5:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep5_desc);
				break;
				case	6:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep6_desc);
				break;
				case	7:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep7_desc);
				break;
				case	8:
					cc = save_rec (tmp_reply_no, tmsl_rec.rep8_desc);
				break;
			}
			if (cc)
				break;
		}
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	return;
}

void
Process (
	void)
{
	int	prompt_fnd;
	int	data_found = FALSE;

 	fsort = sort_open ("rep_sheet");

	cc = find_hash (tmch, &tmch_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		/*-----------------------------
		| Validate selection criteria |
		-----------------------------*/
		if (tmch_rec.hhcf_hash != tmcf_rec.hhcf_hash ||
		    tmch_rec.hhsh_hash != tmsh_rec.hhsh_hash ||
		    tmch_rec.call_date > local_rec.end_date ||
		    tmch_rec.call_date < local_rec.st_date)
		{
			cc = find_hash (tmch, &tmch_rec, NEXT, "r", 0L);
			continue;
		}

		prompt_fnd = FALSE;
		/*-----------------------------------
		| Validate call prompt and response |
		-----------------------------------*/
		tmcl_rec.hhcl_hash = tmch_rec.hhcl_hash;
		tmcl_rec.line_no = 0;
		cc = find_rec (tmcl, &tmcl_rec, GTEQ, "r");
		while (!cc && tmcl_rec.hhcl_hash == tmch_rec.hhcl_hash)
		{
			if (tmcl_rec.prmpt_no == local_rec.prmpt_no)
			{
				prompt_fnd = TRUE;
				break;
			}

			cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
		}
			
		if (!prompt_fnd || 
		   (prompt_fnd && tmcl_rec.rep_no != local_rec.reply_no) ||
		    find_hash (tmpm, &tmpm_rec, COMPARISON, "r", tmch_rec.hhpm_hash))
		{
			cc = find_hash (tmch, &tmch_rec, NEXT, "r", 0L);
			continue;
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", tmpm_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			sprintf (exsf_rec.salesman, "%-40.40s", " Salesman Not Found On File ");

		sprintf (err_str, 
			"%2.2s %-8.8s %08ld %08ld %-40.40s \n", 
			tmpm_rec.sman_code, 
			tmpm_rec.pro_no, 
			tmch_rec.call_no, 
			tmpm_rec.hhpm_hash, 
			exsf_rec.salesman);

		sort_save (fsort, err_str);

		data_found = TRUE;

		cc = find_hash (tmch, &tmch_rec, NEXT, "r", 0L);
	}

	if (data_found)
		ProcessSort ();

	return;
}

void
ProcessSort (
	void)
{
	char	*sptr;
	int	first_time = TRUE;
	int	first_rep = TRUE;
	char	curr_rep [3];
	char	prev_rep [3];
	char	tmp_rep_name [100];

	fsort = sort_sort (fsort, "rep_sheet");

	HeadOutput ();

	dsp_screen ("Printing Rep Lead Sheet", 
		comm_rec.co_no, 
		comm_rec.co_name);

	sptr = sort_read (fsort);
	while (sptr)
	{
		sprintf (curr_rep, "%2.2s", sptr);

		if (first_time)
			strcpy (prev_rep, curr_rep);

		cc = find_hash (tmpm, &tmpm_rec, COMPARISON, "r", atol (sptr + 18));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		strcpy (tmch_rec.co_no, comm_rec.co_no);
		tmch_rec.call_no = atol (sptr + 10);
		if (find_rec (tmch2, &tmch_rec, COMPARISON, "r"))
		{
			sptr = sort_read (fsort);
			continue;
		}

		if (strcmp (curr_rep, prev_rep) || first_time)
		{
			if (!first_time)
				fprintf (fout, ".PA\n");

			expand (tmp_rep_name, sptr + 27);
			fprintf (fout, 
				"| %2.2s %-80.80s%-50.50s|\n", 
				sptr, 
				tmp_rep_name, 
				" ");

			strcpy (prev_rep, curr_rep);
			first_time = FALSE;
			first_rep = TRUE;
		}

		dsp_process ("Prospect :", tmpm_rec.pro_no);
		if (!first_rep)
			fprintf (fout, "|--------------------------------------------------------------------------------------------------------------------------------------|\n");
		fprintf (fout, 
			"| %-8.8s | %-40.40s | %-15.15s | %-30.30s | %-14.14s | %06ld       |\n", 
			tmpm_rec.pro_no, 
			tmpm_rec.name, 
			tmpm_rec.phone_no, 
			tmpm_rec.cont_name1, 
			tmpm_rec.lst_op_code, 
			atol (sptr + 10));

		first_rep = FALSE;
		/*----------------------
		| Track call and print |
		----------------------*/
		tmcl_rec.hhcl_hash = tmch_rec.hhcl_hash;
		tmcl_rec.line_no = 0;
		cc = find_rec (tmcl, &tmcl_rec, GTEQ, "r");
		while (!cc && tmcl_rec.hhcl_hash == tmch_rec.hhcl_hash)
		{
			tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
			tmsl_rec.prmpt_no = tmcl_rec.prmpt_no;
			cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
				continue;
			}

			switch (tmcl_rec.rep_no)
			{
				case	1:
					strcpy (err_str, tmsl_rec.rep1_desc);
				break;
				case	2:
					strcpy (err_str, tmsl_rec.rep2_desc);
				break;
				case	3:
					strcpy (err_str, tmsl_rec.rep3_desc);
				break;
				case	4:
					strcpy (err_str, tmsl_rec.rep4_desc);
				break;
				case	5:
					strcpy (err_str, tmsl_rec.rep5_desc);
				break;
				case	6:
					strcpy (err_str, tmsl_rec.rep6_desc);
				break;
				case	7:
					strcpy (err_str, tmsl_rec.rep7_desc);
				break;
				case	8:
					strcpy (err_str, tmsl_rec.rep8_desc);
				break;
			}
			fprintf (fout, 
				"|        | %4d | %-30.30s    | %4d | %-20.20s | %-50.50s |\n", 
				tmsl_rec.prmpt_no, 
				tmsl_rec.desc, 
				tmcl_rec.rep_no, 
				err_str,
				tmcl_rec.rep1_text);

			cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
		}

		fflush (fout);

		sptr = sort_read (fsort);
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
	sort_delete (fsort, "rep_sheet");
}

void
HeadOutput (
	void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".13\n");
	fprintf (fout, ".L136\n");

	fprintf (fout, ".E Rep Lead Sheet\n");
	fprintf (fout, 
		".ECOMPANY : %s - %s\n", 
		comm_rec.co_no, 
		comm_rec.co_name);

	fprintf (fout, 
		".ECampaign : %d - %s\n", 
		tmcf_rec.campaign_no, 
		tmcf_rec.c_name1);

	fprintf (fout, 
		".C Campaign Objectives : %s\n", 
		tmcf_rec.c_obj1);

	fprintf (fout, 
		".C %s %s\n", 
		tmcf_rec.c_obj2, 
		tmcf_rec.c_obj3);

	fprintf (fout, 
		".C %s %s\n", 
		tmcf_rec.c_obj4, 
		tmcf_rec.c_obj5);

	fprintf (fout, "========================================================================================================================================\n");
	fprintf (fout, "|  Lead  |                Lead Name                 |  Phone Number   |           Contact              | Last Operator  | Call Number  |\n");
	fprintf (fout, "|--------------------------------------------------------------------------------------------------------------------------------------|\n");
	fprintf (fout, "|         Prompt| Prompt Description                | Reply| Reply Description    |     Comment                                        |\n");
	fprintf (fout, "|--------------------------------------------------------------------------------------------------------------------------------------|\n");

	fprintf (fout, ".R========================================================================================================================================\n");

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
	char	headerDate [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		strcpy (headerDate, DateToString (TodaysDate ()));
		rv_pr (headerDate, 70, 0, 0);

		line_at (1,0,80);

		/*-----------------------------------------
		| Telemarketing Lead Sheet By Rep Report. |
		-----------------------------------------*/
		sprintf (err_str, " %s ", ML (mlTmMess059));
		rv_pr (err_str, 20, 0, 1);

		box (0, 2, 80, 14);

		line_at (5, 1,79);
		line_at (7, 1,79);
		line_at (9, 1,79);
		line_at (11,1,79);
		line_at (13,1,79);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
