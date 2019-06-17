/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lett_prt.c,v 5.3 2001/11/14 07:53:18 scott Exp $
|  Program Name  : (tm_lett_prt.c)
|  Program Desc  : (Telemarketing Letter Print)
|               (Analyses Calls And Produces Letter (s))
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 18/07/91         |
|---------------------------------------------------------------------|
| $Log: lett_prt.c,v $
| Revision 5.3  2001/11/14 07:53:18  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lett_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_lett_prt/lett_prt.c,v 5.3 2001/11/14 07:53:18 scott Exp $";

#include <ml_tm_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

	/*=============================================================== 
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		envDbFind = 0,
			envDbCo = 0,
			partPrinted = FALSE,
			envTmLMargin = 10;

	FILE	*fout;

	char	envTmDir [51];

	/*-------------------------------------------------------------------
	| Set up Array to hold Months of Year used with mon in time struct. |
	-------------------------------------------------------------------*/
	static char *mth [] = {
			"January","February","March",
			"April",  "May",     "June",
	        "July",   "August",  "September",
	        "October","November","December"
	};

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct exclRecord	excl_rec;
struct tmofRecord	tmof_rec;
struct tmcfRecord	tmcf_rec;
struct tmxfRecord	tmxf_rec;
struct tmlhRecord	tmlh_rec;
struct tmlnRecord	tmln_rec;
struct tmrcRecord	tmrc_rec;
struct tmpmRecord	tmpm_rec;
struct tmopRecord	tmop_rec;
struct tmchRecord	tmch_rec;
struct tmclRecord	tmcl_rec;

	struct	tm *t;

	double	inv_price;

	char	branchNo [3];
	char	save_ser [26],
			save_part [17];

	long 	cur_ldate;

	char	*scn_desc [] = {
		"Quotation Header Screen.",
		"Quotation line Item Screen.",
		"Quotation Paragraph Screen."
	};

extern	int		TruePosition;

#include	<tm_commands.h>

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	int		campgn;
	char	campgn_name [2][41];
	char	st_lead [9];
	char	st_lead_name [41];
	char	end_lead [9];
	char	end_lead_name [41];
	char	st_op [15];
	char	st_op_name [41];
	char	end_op [15];
	char	end_op_name [41];
	long	st_date;
	long	end_date;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campgn", 3, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number  ", "Enter Campaign no:", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.campgn}, 
	{1, LIN, "campgn_name1", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Desc.   ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name [0]}, 
	{1, LIN, "campgn_name2", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name [1]}, 
	{1, LIN, "st_lead", 7, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "", "Start Lead       ", "Enter Start Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.st_lead}, 
	{1, LIN, "st_lead_name", 7, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_lead_name}, 
	{1, LIN, "end_lead", 8, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "", "End Lead         ", "Enter End Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.end_lead}, 
	{1, LIN, "end_lead_name", 8, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_lead_name}, 
	{1, LIN, "st_op", 10, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Start Operator   ", "Enter Start Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.st_op}, 
	{1, LIN, "st_op_name", 10, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_op_name}, 
	{1, LIN, "end_op", 11, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "End Operator     ", "Enter End Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.end_op}, 
	{1, LIN, "end_op_name", 11, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_op_name}, 
	{1, LIN, "st_date", 13, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Start Date       ", "Enter Start Date ", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.st_date}, 
	{1, LIN, "end_date", 14, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "End Date         ", "Enter End Date ", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.end_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadMisc 		(void);
int 	spec_valid 		(int);
void 	SrchTmcf 		(char *);
void 	SrchTmop 		(char *);
void 	proc_calls 		(void);
int 	get_details 	(void);
int 	CallInRange 	(void);
int 	CheckChiteria 	(void);
void 	PrintLetter 	(void);
void 	Parse 			(char *);
int 	ValidCommand 	(char *);
void 	SubstCommand 	(int, int);
void 	GetUserValue 	(int);
char 	*GetFDate		(long);
void 	PrintCo			(void);
int 	heading 		(int);
void 	SrchTmpm 		(char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	int		i;

	char	*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	tab_row = 6;
	init_scr ();
	set_tty (); 
	_set_masks (argv [0]);

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	ReadMisc ();

	/*-------------------------------
	| Get directory for Quotations. |
	-------------------------------*/
	sptr = chk_env ("TM_DIR");
	if (sptr == (char *)0)
		strcpy (envTmDir, "PRT/TM/t.");
	else
		strcpy (envTmDir, sptr);
	
	/*--------------------
	| Check Left Margin. |
	--------------------*/
	sptr = chk_env ("TM_LMARGIN");
	if (sptr == (char *)0)
		envTmLMargin = 10;
	else
		envTmLMargin = atoi (sptr);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));
	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);

	clear ();

	while (!prog_exit)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		skip_entry 	= FALSE;
		init_ok 	= 1;
		skip_tab 	= 0;
		init_vars (1);	

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (prog_exit || restart)
			continue;

		proc_calls ();

	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (tmxf, tmxf_list, TMXF_NO_FIELDS, "tmxf_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, "tmpm_id_no3");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmof, tmof_list, TMOF_NO_FIELDS, "tmof_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_id_no");
	open_rec (tmcl, tmcl_list, TMCL_NO_FIELDS, "tmcl_id_no");
	open_rec (tmlh, tmlh_list, TMLH_NO_FIELDS, "tmlh_hhlh_hash");
	open_rec (tmln, tmln_list, TMLN_NO_FIELDS, "tmln_id_no");
	open_rec (tmrc, tmrc_list, TMRC_NO_FIELDS, "tmrc_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (exsf);
	abc_fclose (exaf);
	abc_fclose (tmxf);
	abc_fclose (tmcf);
	abc_fclose (tmpm);
	abc_fclose (tmop);
	abc_fclose (tmof);
	abc_fclose (tmch);
	abc_fclose (tmcl);
	abc_fclose (tmlh);
	abc_fclose (tmln);
	abc_fclose (tmrc);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
	void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	if (find_rec (comr, &comr_rec, COMPARISON, "r"))
		sys_err ("Error in comm During (DBFIND)", cc, PNAME);

	abc_fclose (comr);
	return;
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
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.campgn_name [0], "%-40.40s", tmcf_rec.c_name1);
		DSP_FLD ("campgn_name1");
		sprintf (local_rec.campgn_name [1], "%-40.40s", tmcf_rec.c_name2);
		DSP_FLD ("campgn_name2");

		return (EXIT_SUCCESS);
	}

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
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.end_op_name, "%-40.40s", tmop_rec.op_name);
		DSP_FLD ("end_op_name");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("st_lead") || LCHECK ("end_lead"))
	{
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		if (LCHECK ("st_lead"))
			strcpy (tmpm_rec.pro_no,pad_num (local_rec.st_lead));
		else
			strcpy (tmpm_rec.pro_no,pad_num (local_rec.end_lead));
		cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess051));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("st_lead"))
			sprintf (local_rec.st_lead_name, "%-40.40s", tmpm_rec.name);
		else
			sprintf (local_rec.end_lead_name, "%-40.40s", tmpm_rec.name);

		DSP_FLD ("st_lead_name");
		DSP_FLD ("end_lead_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_date"))
	{
		if (dflt_used)
			local_rec.st_date = cur_ldate;
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("end_date"))
	{
		if (dflt_used)
			local_rec.end_date = cur_ldate;
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
	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.","#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%04d", tmcf_rec.campaign_no);
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
SrchTmop (
	char *key_val)
{
	_work_open (14,0,40);
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

/*------------------------------------------
| Process all calls within specified range |
------------------------------------------*/
void
proc_calls (
	void)
{
	int		first_time = TRUE;
	char	tmp_call [9];

	abc_selfield (tmcf, "tmcf_hhcf_hash");
	abc_selfield (tmpm, "tmpm_hhpm_hash");
	abc_selfield (tmop, "tmop_hhop_hash");

	strcpy (tmch_rec.co_no, comm_rec.co_no);
	tmch_rec.call_no = 0L;
	cc = find_rec (tmch, &tmch_rec, GTEQ, "r");
	while (!cc && !strcmp (tmch_rec.co_no, comm_rec.co_no))
	{
		if (!CallInRange () || !CheckChiteria () || !get_details ())
		{
			cc = find_rec (tmch, &tmch_rec, NEXT, "r");
			continue;
		}

		if (first_time)
		{
			dsp_screen ("Preparing Letters",
				comm_rec.co_no,
				comm_rec.co_name);
			first_time = FALSE;
		}

		sprintf (tmp_call, "%08ld", tmch_rec.call_no);
		dsp_process ("Call Number :", tmp_call);
		PrintLetter ();

		cc = find_rec (tmch, &tmch_rec, NEXT, "r");
	}

	abc_selfield (tmcf, "tmcf_id_no");
	abc_selfield (tmpm, "tmpm_id_no3");
	abc_selfield (tmop, "tmop_id_no");
	return;
}

/*---------------------------------------
| Read Letter Header And Sundry Details |
---------------------------------------*/
int
get_details (
	void)
{
	/*--------------------
	| Read Letter Header |
	--------------------*/
	if (find_hash (tmlh, &tmlh_rec, COMPARISON, "r", tmrc_rec.hhlh_hash))
		return (FALSE);

	/*------------------
	| Lookup Sman Code |
	------------------*/
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%2.2s", tmpm_rec.sman_code);
	if (find_rec (exsf, &exsf_rec, COMPARISON, "r"))
	{
		sprintf (exsf_rec.salesman_no, "%2.2s", tmpm_rec.sman_code);
		sprintf (exsf_rec.salesman, "%-40.40s", " ");
	}

	/*------------------
	| Lookup Area Code |
	------------------*/
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%2.2s", tmpm_rec.area_code);
	if (find_rec (exaf, &exaf_rec, COMPARISON, "r"))
	{
		sprintf (exaf_rec.area_code, "%2.2s", tmpm_rec.area_code);
		sprintf (exaf_rec.area, "%-40.40s", " ");
	}

	/*--------------------
	| Lookup Origin Code |
	--------------------*/
	strcpy (tmof_rec.co_no, comm_rec.co_no);
	sprintf (tmof_rec.o_code, "%-3.3s", tmpm_rec.origin);
	if (find_rec (tmof, &tmof_rec, COMPARISON, "r"))
	{
		sprintf (tmof_rec.o_code,"%-3.3s",tmpm_rec.origin);
		sprintf (tmof_rec.o_desc, "%-40.40s", " ");
	}

	return (TRUE);
}

/*--------------------------------------------
| Check That Call Matches Selection Criteria |
--------------------------------------------*/
int
CallInRange (
	void)
{
	if (find_hash (tmcf,&tmcf_rec, COMPARISON, "r", tmch_rec.hhcf_hash))
		return (FALSE);
	if (tmcf_rec.campaign_no != local_rec.campgn)
		return (FALSE);

	if (find_hash (tmpm,&tmpm_rec, COMPARISON, "r", tmch_rec.hhpm_hash))
		return (FALSE);
	if (strcmp (tmpm_rec.pro_no, local_rec.st_lead) < 0 ||
	   strcmp (tmpm_rec.pro_no, local_rec.end_lead) > 0)
		return (FALSE);

	if (find_hash (tmop,&tmop_rec, COMPARISON, "r", tmch_rec.hhop_hash))
		return (FALSE);
	if (strcmp (tmop_rec.op_id, local_rec.st_op) < 0 ||
	   strcmp (tmop_rec.op_id, local_rec.end_op) > 0)
		return (FALSE);

	if (tmch_rec.call_date < local_rec.st_date ||
	   tmch_rec.call_date > local_rec.end_date)
		return (FALSE);

	return (TRUE);
}

/*-------------------------------------------
| Check That Call Matches Response Criteria |
-------------------------------------------*/
int
CheckChiteria (
	void)
{
	tmcl_rec.hhcl_hash = tmch_rec.hhcl_hash;
	tmcl_rec.line_no = 0;
	cc = find_rec (tmcl, &tmcl_rec, GTEQ, "r");
	while (!cc && tmcl_rec.hhcl_hash == tmch_rec.hhcl_hash)
	{
		tmrc_rec.hhcf_hash = tmch_rec.hhcf_hash;
		tmrc_rec.hhsh_hash = tmch_rec.hhsh_hash;
		tmrc_rec.prompt = 0;
		tmrc_rec.response = 0;
		cc = find_rec (tmrc, &tmrc_rec, GTEQ, "r");
		while (!cc && 
		       tmrc_rec.hhcf_hash == tmch_rec.hhcf_hash &&
		       tmrc_rec.hhsh_hash == tmch_rec.hhsh_hash)
		{
			if (tmrc_rec.prompt == tmcl_rec.prmpt_no &&
			    tmrc_rec.response == tmcl_rec.rep_no)
				return (TRUE);
	
			cc = find_rec (tmrc, &tmrc_rec, NEXT, "r");
		}

		cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
	}
	
	return (FALSE);
}

/*=====================================
| Print letter determined by criteria |
=====================================*/
void
PrintLetter (
	void)
{
	char	filename [100],
		tm_number [9];

	char	parse_str [201];

	sprintf (tm_number,"%08ld",tmch_rec.hhcl_hash);
	sprintf (filename,"%s%s", (envTmDir == (char *)0) 
				? "PRT/TM/t." : envTmDir, clip (tm_number));

	if ((fout = fopen (filename,"w")) == 0) 
		sys_err ("Error in Letter Output During (FOPEN)", errno, PNAME);

	tmln_rec.hhlh_hash = tmlh_rec.hhlh_hash;
	tmln_rec.line_no = 0;

	cc = find_rec (tmln, &tmln_rec, GTEQ, "r");
	while (!cc && tmln_rec.hhlh_hash == tmlh_rec.hhlh_hash)
	{
		sprintf (parse_str,
			"%*.*s%s", 
			envTmLMargin,
			envTmLMargin,
			" ",
			clip (tmln_rec.desc));
						  
		Parse (parse_str);
		cc = find_rec (tmln, &tmln_rec, NEXT, "r");
	}

	fflush (stdout);
	fclose (fout);
}

void
Parse (
	char *wrk_prt)
{
	int		cmd;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = strdup (wrk_prt);
	
	partPrinted = TRUE;

	/*-------------------------------
	|	look for caret command	|
	-------------------------------*/
	cptr = strchr (wk_prt,'.');
	dptr = wk_prt;
	while (cptr)
	{
		partPrinted = FALSE;
		/*-------------------------------
		|	print line up to now	|
		-------------------------------*/
		*cptr = (char)NULL;

		if (cptr != wk_prt)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",dptr);
		}

		/*-------------------------------
		|	check if valid .command	|
		-------------------------------*/
		cmd = ValidCommand (cptr + 1);
		if (cmd >= FUL_DAT)
		{
			SubstCommand (cmd,cptr - wk_prt);
			dptr = cptr + 8;
		}
		else
		{
			fprintf (fout,".");
			partPrinted = TRUE;
			dptr = cptr + 1;
		}

		cptr = strchr (dptr,'.');
	}

	/*-------------------------------
	|	print rest of line	|
	-------------------------------*/
	if (partPrinted)
	{
		if (dptr)
			fprintf (fout,"%s\n",dptr);
		else
			fprintf (fout,"\n");
	}
	free (wk_prt);
}

/*=====================
| Validate .commands. |
=====================*/
int
ValidCommand (
	char *wk_str)
{
	int		i;

	/*----------------------------------------
	| Dot command is last character on line. |
	----------------------------------------*/
	if (!strlen (wk_str))
		return (-1);

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (wk_str,dot_cmds [i],7))
			return (i);

	return (-1);
}

/*==============================================
| Substitute valid .commands with actual data. |
==============================================*/
void
SubstCommand (
	int cmd,
	int offset)
{
	char	*pr_sptr;
	char	tmp_amt [21];

	switch (cmd)
	{
	/*-------------------------------
	| System Date, format dd/mm/yy. |
	-------------------------------*/
	case	CUR_DAT:
		partPrinted = TRUE;
		fprintf (fout,"%s",local_rec.systemDate);
		break;

	/*-------------------
	| Full system date. |
	-------------------*/
	case FUL_DAT:
		pr_sptr = GetFDate (cur_ldate);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*---------------
	| Company Name. |
	---------------*/
	case	CO_NAME:
		pr_sptr = clip (comr_rec.co_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 1. |
	--------------------*/
	case	CO_ADR1:
		pr_sptr = clip (comr_rec.co_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 2. |
	--------------------*/
	case	CO_ADR2:
		pr_sptr = clip (comr_rec.co_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 3. |
	--------------------*/
	case	CO_ADR3:
		pr_sptr = clip (comr_rec.co_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*------------------
	| Prospect Number. |
	------------------*/
	case	PS_NUMB:
		pr_sptr = clip (tmpm_rec.pro_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-8.8s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Acronym. |
	-------------------*/
	case	PS_ACRO:
		pr_sptr = clip (tmpm_rec.acronym);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-9.9s",pr_sptr);
		}
		break;

	/*----------------
	| Prospect Name. |
	----------------*/
	case	PS_NAME:
		pr_sptr = clip (tmpm_rec.name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	PS_ADR1:
		pr_sptr = clip (tmpm_rec.mail1_adr);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	PS_ADR2:
		pr_sptr = clip (tmpm_rec.mail2_adr);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	PS_ADR3:
		pr_sptr = clip (tmpm_rec.mail3_adr);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Name 1. |
	--------------------------*/
	case	PS_CNT1:
		pr_sptr = clip (tmpm_rec.cont_name1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-30.30s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Name 2. |
	--------------------------*/
	case	PS_CNT2:
		pr_sptr = clip (tmpm_rec.cont_name2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-30.30s",pr_sptr);
		}
		break;

	/*--------------
	| Salesman No. |
	--------------*/
	case	SM_NUMB:
		pr_sptr = clip (exsf_rec.salesman_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%2.2s",pr_sptr);
		}
		break;

	/*----------------
	| Salesman Name. |
	----------------*/
	case	SM_NAME:
		pr_sptr = clip (exsf_rec.salesman);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*----------
	| Area No. |
	----------*/
	case	AR_NUMB:
		pr_sptr = clip (exaf_rec.area_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%2.2s",pr_sptr);
		}
		break;

	/*------------
	| Area Name. |
	------------*/
	case	AR_NAME:
		pr_sptr = clip (exaf_rec.area);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*------------
	| Post Code. |
	------------*/
	case	PS_POST:
		pr_sptr = clip (tmpm_rec.post_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-4.4s",pr_sptr);
		}
		break;

	/*------------------
	| Business Sector. |
	------------------*/
	case	PS_BSEC:
		pr_sptr = clip (excl_rec.class_desc);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*---------------
	| Phone Number. |
	---------------*/
	case	PHN_NUM:
		pr_sptr = clip (tmpm_rec.phone_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-15.15s",pr_sptr);
		}
		break;

	/*-------------
	| Fax Number. |
	-------------*/
	case	FAX_NUM:
		pr_sptr = clip (tmpm_rec.fax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-15.15s",pr_sptr);
		}
		break;

	/*------------------
	| Next Phone Date. |
	------------------*/
	case	NX_PHDT:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tmpm_rec.n_phone_date));
		break;

	/*------------------
	| Next Phone Time. |
	------------------*/
	case	NX_PHTM:
		pr_sptr = clip (tmpm_rec.n_phone_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*------------------
	| Next Visit Date. |
	------------------*/
	case	NX_VSDT:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tmpm_rec.n_visit_date));
		break;

	/*------------------
	| Next Visit Time. |
	------------------*/
	case	NX_VSTM:
		pr_sptr = clip (tmpm_rec.n_visit_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*------------------
	| Visit Frequency. |
	------------------*/
	case	VSTFREQ:
		sprintf (tmp_amt, "%-d", tmpm_rec.visit_freq);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-------------------
	| Current Operator. |
	-------------------*/
	case	OPERATR:
		pr_sptr = clip (tmpm_rec.op_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*----------------
	| Last Operator. |
	----------------*/
	case	LST_OPR:
		pr_sptr = clip (tmpm_rec.lst_op_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-14.14s",pr_sptr);
		}
		break;

	/*-----------------
	| Call Back Date. |
	-----------------*/
	case	CLBKDAT:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tmpm_rec.call_bk_date));
		break;

	/*-----------------
	| Call Back Time. |
	-----------------*/
	case	CLBKTIM:
		pr_sptr = clip (tmpm_rec.call_bk_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*------------------
	| Last Phone Date. |
	------------------*/
	case	LST_PHN:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tmpm_rec.lphone_date));
		break;

	/*---------
	| Origin. |
	---------*/
	case	PS_ORGN:
		pr_sptr = clip (tmof_rec.o_desc);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*------------------
	| Best Phone Time. |
	------------------*/
	case	BST_PHN:
		pr_sptr = clip (tmpm_rec.best_ph_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*-------------------
	| User Defined One. |
	-------------------*/
	case	U_DEF01:
	case	U_DEF02:
	case	U_DEF03:
	case	U_DEF04:
	case	U_DEF05:
	case	U_DEF06:
	case	U_DEF07:
	case	U_DEF08:
	case	U_DEF09:
	case	U_DEF10:
	case	U_DEF11:
	case	U_DEF12:
		GetUserValue (cmd - U_OFFSET);
			pr_sptr = clip (tmxf_rec.desc);

		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	default:
		break;
	}

	fflush (fout);
}

void
GetUserValue (
	int line_no)
{
	tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, "U");
	tmxf_rec.line_no = line_no;

	cc = find_rec (tmxf, &tmxf_rec, COMPARISON, "r");
	if (cc)
		sprintf (tmxf_rec.desc, "%-60.60s", " ");

	return;
}

/*====================================================
| GetFDate (long-date) returns date in 23 January 1986 . |
====================================================*/
char *
GetFDate (
	long int cur_date)
{
	int		day,
			mon,
			year;

	DateToDMY (cur_date, &day, &mon, &year);
	sprintf (err_str, "%d %s %d", day, mth [mon -1], year);
	return (err_str);
}

void
PrintCo (
	void)
{
	line_at (21,0,80); 

	strcpy (err_str,ML (mlStdMess038));
	print_at (22,0,err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,45,err_str, comm_rec.est_no, comm_rec.est_short);

}

int
heading (
	int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlTmMess045),28,0,1);
		line_at (1,0,80);
		box (0,2,80,12);

		line_at (6,1,79);
		line_at (9,1,79);
		line_at (12,1,79);

		move (1,input_row);
		PrintCo ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
#include	<wild_search.h>

/*
 * Search for Prospect master file. 
 */
void
SrchTmpm (char *key_val)
{
	int		valid = 1;
	int		break_out;
	char	type_flag[2];
	char	*stype = chk_env ("DB_SER");
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	switch (search_key)
	{
	case	FN9:
		strcpy (type_flag,"N");
		break;

	case	FN10:
		strcpy (type_flag,"A");
		break;

	case	FN12:
		strcpy (type_flag,"D");
		break;

	default:
		sprintf (type_flag,"%-1.1s", (stype != (char *)0) ? stype : "N");
		break;
	}

	if (type_flag[0] == 'D' || type_flag[0] == 'd')
		sptr = (char *)0;

	work_open ();

	if (type_flag[0] == 'A' || type_flag[0] == 'a')
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no4" : "tmpm_id_no2");
		save_rec ("#Number","# Acronym   Prospect Name.");
	}
	else
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
		save_rec ("#Number","# Prospect Name.");
	}
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.acronym,"%-9.9s", (sptr != (char *)0) ? sptr : " ");
	sprintf (tmpm_rec.pro_no,"%-6.6s", (sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	while (!cc && !strcmp (tmpm_rec.co_no,comm_rec.co_no))
	{
		/*
		 * If Debtors Branch Owned && Correct Branch. 
		 */
		if (!envDbFind && strcmp (tmpm_rec.br_no,branchNo))
			break;

		switch (type_flag[0])
		{
		case	'A':
		case	'a':
			valid = check_search (tmpm_rec.acronym,key_val,&break_out);
			break;

		case	'D':
		case	'd':
			valid = check_search (tmpm_rec.name,key_val,&break_out);
			break_out = 0;
			break;

		default:
			valid = check_search (tmpm_rec.pro_no,key_val,&break_out);
			break;
		}

		if (valid)
		{
			sprintf (err_str," (%s) %-40.40s", tmpm_rec.acronym, tmpm_rec.name);
			cc = save_rec (tmpm_rec.pro_no,err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (tmpm,&tmpm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
	if (cc)
	{
		sprintf (tmpm_rec.acronym,"%-9.9s"," ");
		return;
	}
	
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.pro_no,"%-6.6s",temp_str);
	sprintf (tmpm_rec.acronym,"%-9.9s",temp_str);
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	if (cc)
		file_err (cc, tmpm, "DBFIND");
}
