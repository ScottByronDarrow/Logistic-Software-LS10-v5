/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: try_scpt.c,v 5.3 2001/11/13 05:30:41 scott Exp $
|  Program Name  : (tm_try_scpt.c)
|  Program Desc  : (Test scripts)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 30/08/91         |
|---------------------------------------------------------------------|
| $Log: try_scpt.c,v $
| Revision 5.3  2001/11/13 05:30:41  scott
| Updated to convert to app.schema
| Update to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: try_scpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_try_scpt/try_scpt.c,v 5.3 2001/11/13 05:30:41 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <ml_tm_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>

/*=======================================
|   Local defines, constants and stuff  |
=======================================*/

#define	FIRST_PRMPT	 (nextPrompt == -1)
#define	END_CALL	 (local_rec.response [0] == 'E')

char *data = "data";



	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	envDbCo		= 0,
		 	envDbFind 	= 0,
			nextPrompt	= 0,
			response	= 0,
			clear_ok	= 0,
			cl_2_ok		= 0;

	extern	int		TruePosition;

	char	logName [15],
			branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct tmcfRecord	tmcf_rec;
struct tmxfRecord	tmxf_rec;
struct tmudRecord	tmud_rec;
struct tmopRecord	tmop_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;

int		*rep_goto	=	&tmsl_rec.rep1_goto;

	long    lsystemDate;

	char    *curr_user;


/*============================ 
| Local & Screen Structures. |
============================*/
struct 
{
	char    dummy [11];
	char    response [2];
	char    rep_desc [8][26];
	char    re_desc [81];
	char    ok_keys [21];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campaign_no", 2, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number       ", "Enter Campaign no:", 
		YES, NO, JUSTLEFT, "", "", (char *)&tmcf_rec.campaign_no}, 
	{1, LIN, "camp_desc1", 3, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Description  ", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name1}, 
	{1, LIN, "camp_desc2", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name2}, 
	{1, LIN, "script_no", 6, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Script Number         ", "Enter Script no:", 
		YES, NO, JUSTLEFT, "", "", (char *)&tmsh_rec.script_no}, 
	{1, LIN, "script_desc", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Script Description    ", "", 
		NA, NO, JUSTLEFT, "", "", tmsh_rec.desc}, 
	{2, LIN, "prmpt_line1", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text1}, 
	{2, LIN, "prmpt_line2", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text2}, 
	{2, LIN, "prmpt_line3", 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text3}, 
	{2, LIN, "prmpt_line4", 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text4}, 
	{2, LIN, "prmpt_line5", 13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text5}, 
	{2, LIN, "prmpt_line6", 14, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text6}, 
	{2, LIN, "prmpt_line7", 15, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text7}, 
	{2, LIN, "resp1", 17, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [0]}, 
	{2, LIN, "resp2", 17, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [1]}, 
	{2, LIN, "resp3", 18, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [2]}, 
	{2, LIN, "resp4", 18, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [3]}, 
	{2, LIN, "resp5", 18, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [4]}, 
	{2, LIN, "resp6", 19, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [5]}, 
	{2, LIN, "resp7", 19, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [6]}, 
	{2, LIN, "resp8", 19, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [7]}, 
	{2, LIN, "response", 17, 2, CHARTYPE, 
		"U", "          ", 
		" ", "", "Enter Response        ", local_rec.re_desc, 
		YES, NO, JUSTLEFT, local_rec.ok_keys, "", local_rec.response}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchTmpf 			(char *);
void 	SrchTmsh 			(char *);
int  	spec_valid 			(int);
int  	GetOperator 		(void);
int  	ProcessScript 		(void);
int  	LdScriptLine 		(int);
int  	heading 			(int);
int  	ClearScreen 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv [])
{
	char	*sptr = getenv ("LOGNAME");

	TruePosition	=	TRUE;

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();

	set_masks ();

	lsystemDate = TodaysDate ();

	OpenDB (); 	

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (sptr)
	{
		sprintf (logName, "%-14.14s", sptr);
		if (!GetOperator ())
		{
			errmess (ML (mlTmMess048));
			sleep (sleepTime);
			shutdown_prog ();
            return (EXIT_FAILURE);
		}
	}
	else
	{
		errmess (ML (mlTmMess049));
		sleep (sleepTime);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

    prog_exit = FALSE;
	while (!prog_exit)
	{
		restart   = FALSE;
		prog_exit = FALSE;
		search_ok = TRUE;
		init_vars (1);

		clear_ok = TRUE;
		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		ProcessScript ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB ();
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmxf, tmxf_list, TMXF_NO_FIELDS, "tmxf_id_no");
	open_rec (tmud, tmud_list, TMUD_NO_FIELDS, "tmud_id_no");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (tmcf);
	abc_fclose (tmxf);
	abc_fclose (tmud);
	abc_fclose (tmop);
	abc_fclose (comr);
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_dbclose (data);
}
int
spec_valid (
 int field)
{
	/*
	 * Validate Campaign Number
	 */
	if (LCHECK ("campaign_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}
		else
        {
			scn_display (1);
        }

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("script_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);

			return (EXIT_SUCCESS);
		}

		strcpy (tmsh_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess063));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}
		
		DSP_FLD ("script_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("response"))
	{
		if (END_CALL)
		{
			nextPrompt = 999;
			entry_exit = TRUE;

			return (EXIT_SUCCESS);
		}

		response = atoi (local_rec.response);
		if (rep_goto [response - 1] != 0)
        {
			nextPrompt = rep_goto [response - 1];
        }
		else
		{
			print_mess (ML (mlTmMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*
 * Lookup operator file.
 */
int
GetOperator (void)
{
	char	tmp_op [15];

	sprintf (tmp_op, "%-14.14s", logName);
	upshift (tmp_op);

	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", tmp_op);

    cc = find_rec (tmop, &tmop_rec, COMPARISON, "u");
	if (cc)
    {
		return (FALSE);
    }

	return (TRUE);
}

/*-----------------------
| Make a call to a lead |
-----------------------*/
int
ProcessScript (void)
{
	nextPrompt = -1;

	/*----------------------
	| Read script header.  |
	----------------------*/
	strcpy (tmsh_rec.co_no, comm_rec.co_no);

    cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
    {
		return (FALSE);
    }

	cl_2_ok = TRUE;

	/*----------------
	| Process script |
	----------------*/
	while (nextPrompt != 999)
	{
		init_vars (2);

		if (!LdScriptLine (nextPrompt))
		{
            errmess (ML (mlTmMess065));
            sleep (sleepTime);
            nextPrompt = 999;
            continue;
		}

		heading (2);
		scn_display (2);
		init_ok = FALSE;
		cl_2_ok = TRUE;
		
		entry (2);

		if (restart)
		{
			nextPrompt = 999;
			restart = FALSE;
			continue;
		}

		init_ok = TRUE;
	}

	return (TRUE);
}

/*----------------------------------------
| Load Required Line From Current Script |
----------------------------------------*/
int
LdScriptLine (
 int nextPrompt)
{
	int	i;

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = nextPrompt;

	if (FIRST_PRMPT)
    {
		cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
    }
	else 
    {
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
    }

	if (cc)
    {
		return (FALSE);
    }
	else
	{
		if (FIRST_PRMPT && 
		   (tmsl_rec.hhsh_hash != tmsh_rec.hhsh_hash))
        {
			return (FALSE);
        }
	}

	for (i = 0; i < 8; i++)
	{
		switch (i)
		{
			case	0:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep1_desc);
			break;

			case	1:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep2_desc);
			break;

			case	2:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep3_desc);
			break;

			case	3:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep4_desc);
			break;

			case	4:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep5_desc);
			break;

			case	5:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep6_desc);
			break;

			case	6:
			sprintf (local_rec.rep_desc [i], "%d : %-20.20s", i + 1, tmsl_rec.rep7_desc);
			break;
		}
		sprintf (err_str, "resp%1d", i + 1);
		if (rep_goto [i] == 0)
			FLD (err_str) = ND;
		else
			FLD (err_str) = NA;
	}
	return (TRUE);
}

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmpf (
	char	*key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.","#Campaign Description.");

	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
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
    {
		file_err (cc, tmcf, "DBFIND");
    }
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void 
SrchTmsh (
	char	*key_val)
{
	char	tmp_scr_no [6];

	_work_open (4,0,40);
	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#No","#Script Description.");

	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
           tmsh_rec.script_no >= atoi (key_val))
	{
		sprintf (tmp_scr_no, "%04d", tmsh_rec.script_no);
		cc = save_rec (tmp_scr_no, tmsh_rec.desc);
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

int
heading (
	int	scn)
{
	if (scn != cur_screen)
    {
		scn_set (scn);
    }

	if ((scn == 1 && clear_ok) || 
        scn == 0)
    {
		clear ();
    }

	if (scn == 1 || 
        scn == 2)
	{
		if (scn == 1)
		{
			box (0,1,80,6);
			line_at (5,1,79);
		}
		rv_pr (ML (mlTmMess047),30,0,1);
	}
		
	if (scn == 2)
	{
		if (cl_2_ok)
			ClearScreen (2);
		box (0, 1, 80, 18);

		sprintf (err_str, 
		    	 " %-*.*s ", 
			    (int) strlen (clip (tmsl_rec.desc)),
			    (int) strlen (clip (tmsl_rec.desc)),
			     tmsl_rec.desc);

		rv_pr (err_str, (80 - strlen (tmsl_rec.desc))/2 , 8, 1);

		line_at (16,1,79);
	}

	if (scn != 2)
		line_at (21,0,80);

	strcpy (err_str,ML (mlStdMess038));
	print_at (22,0,err_str,comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

int
ClearScreen (
 int scn_no)
{
	int	i;

	if (scn_no == 2)
	{
		for (i = 9; i < 21; i++)
		{
			move (0,i);
			cl_line ();
		}
	}

    /*  QUERY STD_RETURN
        assumed return value of 0 is EXIT_SUCCESS
    */
	return (EXIT_SUCCESS);
}
