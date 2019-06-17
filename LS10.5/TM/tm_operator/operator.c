/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: operator.c,v 5.5 2002/03/01 03:38:49 scott Exp $
|  Program Name  : (tm_operator.c)
|  Program Desc  : (Add / Change Tele-marketing Operator File)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21/06/91         |
|---------------------------------------------------------------------|
| $Log: operator.c,v $
| Revision 5.5  2002/03/01 03:38:49  scott
| .
|
| Revision 5.4  2001/11/12 07:37:24  scott
| Updated for sco error
|
| Revision 5.3  2001/11/12 07:34:33  scott
| Updated to add app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: operator.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_operator/operator.c,v 5.5 2002/03/01 03:38:49 scott Exp $";

#ifdef	LINUX
#define	_XOPEN_SOURCE
#include	<unistd.h>
#endif	// LINUX

#include <pslscr.h>
#include <ml_tm_mess.h>
#include <ml_std_mess.h>

#ifndef	SCO

#ifndef GVISION
#include <pwd.h>
#endif	/* GVISION */

#endif

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newOperator 	= 0,
			envDbCo 		= 0,
			envDbFind 		= 0;
	
	char	branchNo [3];

	char	*tmop2	=	"tmop2";

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmopRecord	tmop2_rec;
struct tmcfRecord	tmcf_rec;
struct exsfRecord	exsf_rec;
struct tmshRecord	tmsh_rec;

	long	lsystemDate;

	char	*scn_desc [] = {
		"Campaign Header.",
		"Campaign Details."
	};

	char	*currentUser;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [16];
	char	out_sr_desc [41];
	char	in_sr_desc [41];
	char	stat_flag [2];
	char	stat_flag_desc [7];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "op_id", 3, 21, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", " ", "Operator I.D.", "Enter Operator Code.", 
		NE, NO, JUSTLEFT, "", "", tmop_rec.op_id}, 
	{1, LIN, "op_name", 4, 21, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Operator Name.", "Enter Operator full name.", 
		YES, NO, JUSTLEFT, "", "", tmop_rec.op_name}, 
	{1, LIN, "passwd", 5, 21, CHARTYPE, 
		"________", "          ", 
		" ", "", "Password ", "Enter you password ", 
		NO, NO, JUSTLEFT, "", "", local_rec.dummy}, 
	{1, LIN, "short", 5, 64, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Short ID. ", "Enter users short code.", 
		YES, NO, JUSTRIGHT, "", "", tmop_rec.short_id}, 
	{1, LIN, "op_stat", 6, 21, CHARTYPE, 
		"U", "          ", 
		" ", "", "Status ", "Enter Operator Status S (uper) N(ormal)", 
		NO, NO, JUSTLEFT, "SN", "", local_rec.stat_flag}, 
	{1, LIN, "op_stat_desc", 6, 24, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.stat_flag_desc}, 
	{1, LIN, "campaign_no", 8, 21, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Campaign Number.", "Enter Campaign no.", 
		YES, NO, JUSTLEFT, "", "", (char *)&tmop_rec.campaign_no}, 
	{1, LIN, "camp_desc", 9, 21, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Desc.", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name1}, 
	{1, LIN, "in_script_no", 11, 21, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "In Script Number.", "Enter Script no.", 
		NO, NO, JUSTLEFT, "", "", (char *)&tmop_rec.in_scr_no}, 
	{1, LIN, "in_sr_desc", 12, 21, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Script Description.", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.in_sr_desc}, 
	{1, LIN, "out_script_no", 13, 21, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Out Script Number.", "Enter Script no.", 
		NO, NO, JUSTLEFT, "", "", (char *)&tmop_rec.out_scr_no}, 
	{1, LIN, "out_sr_desc", 14, 21, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Script Description.", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.out_sr_desc}, 
	{1, LIN, "sman", 16, 21, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Salesman code  ", "Enter Valid Salesman Number.", 
		YES, NO, JUSTRIGHT, "", "", tmop_rec.sman_code}, 
	{1, LIN, "sal_name", 17, 21, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		NA, NO, JUSTLEFT, "", "", exsf_rec.salesman}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchTmsh 		(char *);
void 	SrchTmcf 		(char *);
void 	SrchExsf 		(char *);
void 	SrchTmop 		(char *);
void 	FindMisc 		(void);
int 	SetPassword 	(void);
int 	spec_valid 		(int);
int 	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	currentUser = getenv ("LOGNAME");

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	_set_masks ("tm_operator.s");

	lsystemDate = TodaysDate ();

	OpenDB (); 	

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newOperator = FALSE;
		search_ok	= TRUE;
		abc_unlock (tmop);
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		/*-----------------------
		| Update debtor record. |
		-----------------------*/
		if (!restart)
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
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (tmop2, tmop);
	open_rec (tmop2,tmop_list, TMOP_NO_FIELDS, "tmop_pass_id");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (exsf);
	abc_fclose (tmop);
	abc_fclose (tmcf);
	abc_fclose (tmsh);
	abc_fclose (tmop2);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*------------------------
	| Validate Operator I.D. |
	------------------------*/
	if (LCHECK ("op_id"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmop_rec.co_no,comm_rec.co_no);
		newOperator = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (!newOperator)
		{
			FindMisc ();
			entry_exit = TRUE;
		}
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("op_stat"))
	{
		if (local_rec.stat_flag [0] == 'S')
			strcpy (local_rec.stat_flag_desc, ML ("Super "));
		else
			strcpy (local_rec.stat_flag_desc, ML ("Normal"));

		DSP_FLD ("op_stat_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campaign_no"))
	{
		if (dflt_used)
		{
			sprintf (tmcf_rec.c_name1, "%-40.40s", 
				ML ("Operator Not Assigned To A Campaign"));
			DSP_FLD ("camp_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		tmcf_rec.campaign_no = tmop_rec.campaign_no;
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Campaign not found. |
			---------------------*/
			errmess (ML (mlTmMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("camp_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("in_script_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.in_sr_desc, 
				"%-40.40s", 
				"Operator Not Assigned An In Script");
			DSP_FLD ("in_sr_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmsh_rec.co_no,comm_rec.co_no);
		tmsh_rec.script_no = tmop_rec.in_scr_no;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Script not found. |
			-------------------*/
			errmess (ML (mlTmMess002));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.in_sr_desc, tmsh_rec.desc);
		DSP_FLD ("in_sr_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("out_script_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.out_sr_desc, 
				"%-40.40s", 
				"Operator Not Assigned An Out Script");
			DSP_FLD ("out_sr_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmsh_rec.co_no,comm_rec.co_no);
		tmsh_rec.script_no = tmop_rec.out_scr_no;
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Script not found. |
			-------------------*/
			errmess (ML (mlTmMess002));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.out_sr_desc, tmsh_rec.desc);
		DSP_FLD ("out_sr_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate salesman code and allow search. |
	------------------------------------------*/
	if (LCHECK ("sman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,tmop_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Salesman not found. |
			---------------------*/
			errmess (ML (mlStdMess135));
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("sal_name");
		return (EXIT_SUCCESS);
	}
	if (!strcmp (FIELD.label,"passwd"))
		return (SetPassword ());
					
	return (EXIT_SUCCESS);
}	
/*----------------
| Update Record. |
----------------*/
void
Update (
	void)
{
	fflush (stdout);

	/*-------------------
	| Add a new script. |
	-------------------*/
	if (newOperator)
	{
		sprintf (tmop_rec.stat_flag, "%-1.1s", local_rec.stat_flag);
		strcpy (tmop_rec.unscripted, "N");
		cc = abc_add (tmop, &tmop_rec);
		if (cc)
			file_err (cc, tmop, "DBADD");
	}
	else
	{
		sprintf (tmop_rec.stat_flag, "%-1.1s", local_rec.stat_flag);
		strcpy (tmop_rec.unscripted, "N");
		cc = abc_update (tmop, &tmop_rec);
		if (cc)
			file_err (cc, tmop, "DBUPDATE");

	}
	return;
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

/*========================================
| Search routine for Script Header File. |
========================================*/
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

/*==========================================
| Search routine for Salesman Master File. |
==========================================*/
void
SrchExsf (
	char *key_val)
{
	_work_open (2,0,40);
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,key_val);
	save_rec ("#No.","#Salesman Description");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strncmp (exsf_rec.salesman_no, key_val,strlen (key_val)) && 
		      !strcmp (exsf_rec.co_no, comm_rec.co_no))
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
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
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

int
SetPassword (
	void)
{
#ifdef	SCO
	sprintf (tmop_rec.op_passwd, "%-13.13s", local_rec.dummy);
	return (EXIT_SUCCESS);
#else
#	ifdef GVISION

	sprintf (tmop_rec.op_passwd, "%-13.13s", local_rec.dummy);
	return (EXIT_SUCCESS);

#	else	/* GVISION */

	strcpy (tmop_rec.op_passwd, crypt (local_rec.dummy, "TM"));

	strcpy (tmop2_rec.co_no, comm_rec.co_no);
	strcpy (tmop2_rec.op_passwd, tmop_rec.op_passwd);
	if (!find_rec (tmop2, &tmop2_rec, COMPARISON, "r"))
	{
		/*---------------------------------------------
		| Password already exists in security system. |
		---------------------------------------------*/
		print_mess (ML (mlTmMess060));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);

#	endif	/* GVISION */
#endif
}

void
FindMisc (
	void)
{
	if (tmop_rec.stat_flag [0] == 'S')
	{
		strcpy (local_rec.stat_flag, "S");
		strcpy (local_rec.stat_flag_desc, ML ("Super "));
	}
	else
	{
		strcpy (local_rec.stat_flag, "N");
		strcpy (local_rec.stat_flag_desc, ML ("Normal"));
	}

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,tmop_rec.sman_code);
	if (find_rec (exsf, &exsf_rec, COMPARISON, "r"))
		strcpy (exsf_rec.salesman_no,"  ");

	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = tmop_rec.campaign_no;
	if (find_rec (tmcf, &tmcf_rec, COMPARISON, "r"))
	{
		tmcf_rec.campaign_no = 0;
		sprintf (tmcf_rec.c_name1, "%-40.40s", 
			ML ("Operator Not Assigned To A Campaign"));
	}

	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = tmop_rec.in_scr_no;
	if (find_rec (tmsh, &tmsh_rec, COMPARISON, "r"))
	{
		sprintf (local_rec.in_sr_desc, "%-40.40s", 
			ML ("Operator Not Assigned An In Script"));
	}
	else
		strcpy (local_rec.in_sr_desc, tmsh_rec.desc);

	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = tmop_rec.out_scr_no;
	if (find_rec (tmsh, &tmsh_rec, COMPARISON, "r"))
	{
		sprintf (local_rec.out_sr_desc, "%-40.40s", 
			ML ("Operator Not Assigned An Out Script"));
	}
	else
		strcpy (local_rec.out_sr_desc, tmsh_rec.desc);
}

int
heading (
	int scn)
{
	if (restart)
	{
		abc_unlock (tmop);
		return (EXIT_FAILURE);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	if (scn != cur_screen)
		scn_set (scn);

	pr_box_lines (scn);

	sprintf (err_str, " %s ", ML (mlTmMess061));
	rv_pr (err_str, 25,0,1);
	line_at (1,0,80);

	line_at (20,0,80);
	sprintf (err_str, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	print_at (21,1, "%s", err_str);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
