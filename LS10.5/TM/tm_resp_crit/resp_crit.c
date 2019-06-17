/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: resp_crit.c,v 5.3 2001/11/14 08:48:19 scott Exp $
|  Program Name  : (tm_resp_crit.c)
|  Program Desc  : (Maintain Response Criteria For Automatic)
|                 (Letters)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 07/08/91         |
|---------------------------------------------------------------------|
| $Log: resp_crit.c,v $
| Revision 5.3  2001/11/14 08:48:19  scott
| Updated to add app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: resp_crit.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_resp_crit/resp_crit.c,v 5.3 2001/11/14 08:48:19 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;
struct tmrcRecord	tmrc_rec;
struct tmcfRecord	tmcf_rec;
struct tmlhRecord	tmlh_rec;

int		*tmsl_rep_goto	=	&tmsl_rec.rep1_goto;

	char	*scn_desc [] = {
		"Script Header Screen.",
		"Prompt Objective.",
		"Prompt text."
	};

int	new_resp;
int	rec_locked;

extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	int		campgn;
	char	campgn_name [61];
	char	script_name [41];
	int		script;
	char	prmpt_desc [31];
	int		prmpt;
	char	rep_desc [21];
	int		rep_no;
	char	letter [11];
	char	letter_desc [41];
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "campgn", 4, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign number  ", " Enter Campaign. Default for ALL ", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.campgn}, 
	{1, LIN, "campgn_name", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name}, 
	{1, LIN, "script_no", 6, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Script Number    ", "Enter Script no.", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.script}, 
	{1, LIN, "script_desc", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Script Desc.     ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.script_name}, 
	{1, LIN, "prompt_no", 9, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Prompt Number    ", "Enter Prompt no.", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.prmpt}, 
	{1, LIN, "prompt_desc", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Desc.     ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.prmpt_desc}, 
	{1, LIN, "rep_no", 12, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Reply Number     ", "Enter Reply no.", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.rep_no}, 
	{1, LIN, "rep_desc", 13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Reply Desc.      ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc}, 
	{1, LIN, "letter", 15, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Letter To Produce ", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.letter}, 
	{1, LIN, "letter_desc", 16, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Letter Desc.     ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.letter_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 

};
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchTmcf 		(char *);
void 	SrchTmsh 		(char *);
void 	SrchTmsl 		(char *);
void 	SrchTmlh 		(char *);
void 	ShowReply 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB (); 	

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		rec_locked 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			if (rec_locked)
				abc_unlock (tmrc);
			continue;
		}

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			if (rec_locked)
				abc_unlock (tmrc);
			continue;
		}

		Update ();
	}
	CloseDB (); 
	FinishProgram ();
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmrc, tmrc_list, TMRC_NO_FIELDS, "tmrc_id_no");
	open_rec (tmlh, tmlh_list, TMLH_NO_FIELDS, "tmlh_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
	void)
{
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_fclose (tmcf);
	abc_fclose (tmrc);
	abc_fclose (tmlh);
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

		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		tmcf_rec.campaign_no = local_rec.campgn;
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			/* No Such Campaign */
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

	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("script_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmsh_rec.co_no,comm_rec.co_no);
		tmsh_rec.script_no = local_rec.script;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTmMess002));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.script_name, "%-40.40s", tmsh_rec.desc);
		DSP_FLD ("script_desc");
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Prompt Number. |
	-------------------------*/
	if (LCHECK ("prompt_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsl (temp_str);
			return (EXIT_SUCCESS);
		}
		tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
		tmsl_rec.prmpt_no = local_rec.prmpt;
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "w");
		if (cc)
		{
			errmess (ML (mlTmMess057));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.prmpt_desc, "%-30.30s", tmsl_rec.desc);
		DSP_FLD ("prompt_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rep_no"))
	{
		if (SRCH_KEY)
		{
			ShowReply ();
			return (EXIT_SUCCESS);
		}

		tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
		tmsl_rec.prmpt_no = local_rec.prmpt;
		save_rec ("#Reply No","#Reply Description.");
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
		if (!cc)
		{
			if (tmsl_rep_goto [local_rec.rep_no - 1] == 0)
			{
				print_mess (ML (mlTmMess058));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			tmrc_rec.hhcf_hash = tmcf_rec.hhcf_hash;
			tmrc_rec.hhsh_hash = tmsh_rec.hhsh_hash;
			tmrc_rec.prompt = local_rec.prmpt;
			tmrc_rec.response = local_rec.rep_no;
			cc = find_rec (tmrc, &tmrc_rec, COMPARISON, "u");
			if (cc)
				new_resp = TRUE;
			else
			{
				rec_locked = TRUE;
				abc_selfield (tmlh, "tmlh_hhlh_hash");
				if (find_hash (tmlh, &tmlh_rec, COMPARISON, "r", tmrc_rec.hhlh_hash))
					new_resp = TRUE;
				else
				{
					sprintf (local_rec.letter, "%-10.10s", tmlh_rec.let_code);
					sprintf (local_rec.letter_desc, "%-40.40s", tmlh_rec.let_desc);
					new_resp = FALSE;
					abc_selfield (tmlh, "tmlh_id_no");
					entry_exit = TRUE;
				}
			}
			switch (local_rec.rep_no)
			{
				case	1:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep1_desc);
				break;
				case	2:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep2_desc);
				break;
				case	3:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep3_desc);
				break;
				case	4:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep4_desc);
				break;
				case	5:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep5_desc);
				break;
				case	6:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep6_desc);
				break;
				case	7:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep7_desc);
				break;
				case	8:
					sprintf (local_rec.rep_desc,"%-20.20s", tmsl_rec.rep8_desc);
				break;
			}
			DSP_FLD ("rep_desc");
			return (EXIT_SUCCESS);
		}
		else
		{
			print_mess (ML (mlTmMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	/*-----------------------
	| Validate Letter Code  |
	-----------------------*/
	if (LCHECK ("letter"))
	{
		if (SRCH_KEY)
		{
			SrchTmlh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmlh_rec.co_no, comm_rec.co_no);
		tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		sprintf (tmlh_rec.let_code, "%-10.10s", local_rec.letter);
		cc = find_rec (tmlh, &tmlh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess109));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.letter_desc,"%-40.40s",tmlh_rec.let_desc);
		DSP_FLD ("letter_desc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	

/*----------------
| Update Record. |
----------------*/
void
Update (
	void)
{
	tmrc_rec.hhlh_hash = tmlh_rec.hhlh_hash;
	if (new_resp)
	{
		cc = abc_add (tmrc, &tmrc_rec);
		if (cc)
			file_err (cc, tmrc, "DBADD");
	}
	else
	{
		cc = abc_update (tmrc, &tmrc_rec);
		if (cc)
			file_err (cc, tmrc, "DBUPDATE");
	}

	return;
}

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmcf (
	char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.","#Campaign Description.");
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

	strcpy (tmcf_rec.co_no,comm_rec.co_no);
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
	_work_open (4,0,40);

	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#No.","#Script Description.");
	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
		tmsh_rec.script_no >= atoi (key_val))
	{
		sprintf (err_str, "%04d", tmsh_rec.script_no);
		cc = save_rec (err_str, tmsh_rec.desc);
		if (cc)
			break;
		cc = find_rec (tmsh, &tmsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = atoi (temp_str);
	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmsh, "DBFIND");
}

/*======================================
| Search routine for Script Line File. |
======================================*/
void
SrchTmsl (
	char *key_val)
{
	_work_open (4,0,40);

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = atoi (key_val);
	save_rec ("#No.","#Prompt Description.");
	cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
	while (!cc && tmsl_rec.hhsh_hash == tmsh_rec.hhsh_hash &&
		tmsl_rec.prmpt_no >= atoi (key_val))
	{
		sprintf (err_str, "%04d", tmsl_rec.prmpt_no);
		cc = save_rec (err_str, tmsl_rec.desc);
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
ShowReply (
	void)
{
	char	tmp_rep_no [6];
	int	i;

	_work_open (4,0,40);
	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = local_rec.prmpt;
	save_rec ("#No","#Reply Description.");
	cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	if (!cc)
	{
		for (i = 0; i < 8; i++)
		{
			if (tmsl_rep_goto [i] == 0)
				break;

			sprintf (tmp_rep_no, "%4d", i + 1);
			switch (i)
			{
				case	1:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep1_desc);
				break;
				case	2:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep2_desc);
				break;
				case	3:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep3_desc);
				break;
				case	4:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep4_desc);
				break;
				case	5:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep5_desc);
				break;
				case	6:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep6_desc);
				break;
				case	7:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep7_desc);
				break;
				case	8:
					cc = save_rec (tmp_rep_no, tmsl_rec.rep8_desc);
				break;
			}
			if (cc)
				break;
		}
	}
	cc = disp_srch ();
	work_close ();

	return;
}

/*=============================================
| Search routine for Letter master file. |
=============================================*/
void
SrchTmlh (
	char *key_val)
{
	work_open ();
	save_rec ("#Code","#Description");
	strcpy (tmlh_rec.co_no,comm_rec.co_no);
	tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	sprintf (tmlh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tmlh, &tmlh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp (tmlh_rec.co_no,comm_rec.co_no)     &&
		  tmlh_rec.hhcf_hash == tmcf_rec.hhcf_hash &&
	          !strncmp (tmlh_rec.let_code,key_val,strlen (key_val)))
	{
		cc = save_rec (tmlh_rec.let_code,tmlh_rec.let_desc);
		if (cc)
			break;

		cc = find_rec (tmlh, &tmlh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmlh_rec.co_no,comm_rec.co_no);
	tmlh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	sprintf (tmlh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tmlh, &tmlh_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in tmlh During (DBFIND)", cc, PNAME);

	sprintf (local_rec.letter_desc,"%-40.40s",tmlh_rec.let_desc);
}

int
heading (
	int scn)
{
	if (restart)
	{
		return (EXIT_FAILURE);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	box (0,3,80,13);

	line_at (8,1,79);
	line_at (11,1,79);
	line_at (14,1,79);

	rv_pr (ML (mlTmMess071),18,0,1);
	line_at (1,0,80);
	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
