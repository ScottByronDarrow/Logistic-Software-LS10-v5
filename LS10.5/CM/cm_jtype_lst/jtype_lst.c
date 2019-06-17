/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: jtype_lst.c,v 5.3 2002/07/17 09:57:01 scott Exp $
|  Program Name  : (jtype_lst.c)                                    |
|  Program Desc  : (Default G/L Acc Setup by Job Types Listing  )   |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : 17/03/93         |
|---------------------------------------------------------------------|
| $Log: jtype_lst.c,v $
| Revision 5.3  2002/07/17 09:57:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:22  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: jtype_lst.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_jtype_lst/jtype_lst.c,v 5.3 2002/07/17 09:57:01 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include	<string.h>
#include	<memory.h>
#include	<ml_std_mess.h>
#include	<ml_cm_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cmjtRecord	cmjt_rec;

/*======
 Globals
========*/
	int	bgmode = FALSE;

/*===========
 Table names
============*/
static char
	*data	= "data";

/*============================
| Local & Screen Structures. |
============================*/
#define	CODE_WIDTH	4
struct
{
	char	beg_jt [CODE_WIDTH + 1];
	char	beg_jt_desc [41];
	char	end_jt [CODE_WIDTH + 1];
	char	end_jt_desc [41];

	int		lpno;
	char	back [2];
	char	onight [2];

	int	dummy;

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "beg_cost", 	4, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", " Start Job Type  ", "Enter the starting job type code",
		YES, NO,  JUSTLEFT, "", "", local_rec.beg_jt},
	{1, LIN, "beg_desc", 	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.beg_jt_desc},
	{1, LIN, "end_cost", 	5, 16, CHARTYPE,
		"UUUU", "          ",
		" ", "~", " End Job Type    ", "Enter the ending costhead code",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_jt},
	{1, LIN, "end_desc", 	5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_jt_desc},

	{1, LIN, "lpno", 	 7, 16, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{1, LIN, "back", 	 8, 16, CHARTYPE,
		"U", "          ",
		" ", "N", " Background      ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "onight", 	 9, 16, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight       ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},

	{0, LIN, "", 	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.dummy},
};


/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int field);
void	RunProgram		(char *prog_name, char *prog_desc);
void	ProcessFile		(void);
void	StartReport		(FILE *pout, int prnt_no);
void	PrintLine		(FILE *pout);
void	EndReport		(FILE *pout);
int		heading			(int scn);
void	SrchCmjt		(char *key_val);
int		GetDescription	(char *key, char *dst);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 2 && argc != 4)
	{
		print_at (0,0,mlCmMess717, argv [0]);
		print_at (1,0,mlCmMess742, argv [0]);
		return (EXIT_FAILURE);
	}

	/*====================================
	| Open db and read in terminal record
	======================================*/
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (argc == 4)
	{
		sprintf (local_rec.beg_jt, "%-4.4s", argv [1]);
		sprintf (local_rec.end_jt, "%-4.4s", argv [2]);
		local_rec.lpno = atoi (argv [3]);

		bgmode = TRUE;
		ProcessFile ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	while (!prog_exit)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
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
	}

	/*========================
	| Program exit sequence. |
	========================*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*==============================
 Open and closes of db and files
================================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose (cmjt);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onight"))
	{
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Start Job Type. |
	--------------------------*/
	if (LCHECK ("beg_cost"))
	{
		if (dflt_used)
		{
			memset (local_rec.beg_jt, ' ', CODE_WIDTH);
			local_rec.beg_jt [CODE_WIDTH] = '\0';
			strcpy (local_rec.beg_jt_desc, "First Job Type");
			DSP_FLD ("beg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.beg_jt, local_rec.end_jt) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!GetDescription (local_rec.beg_jt, local_rec.beg_jt_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("beg_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate End Job Type. |
	------------------------*/
	if (LCHECK ("end_cost"))
	{
		if (dflt_used)
		{
			memset (local_rec.end_jt, '~', CODE_WIDTH);
			local_rec.end_jt [CODE_WIDTH] = '\0';
			strcpy (local_rec.end_jt_desc, "Last Job Type");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.beg_jt, local_rec.end_jt) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!GetDescription (local_rec.end_jt, local_rec.end_jt_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("end_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
 char *	prog_name,
 char *	prog_desc)
{
	char	lpstr [10];

	if (local_rec.onight [0] == 'Y' || local_rec.back [0] == 'Y')
	{
		CloseDB (); FinishProgram ();;
		snorm ();
		rset_tty ();
	}

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
		{
			sprintf (lpstr, "%d", local_rec.lpno);
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.beg_jt,
				local_rec.end_jt,
				lpstr,
				prog_desc,
				(char *) 0);
		}
		else
			return;
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
		{
			sprintf (lpstr, "%d", local_rec.lpno);
			execlp (prog_name,
				prog_name,
				local_rec.beg_jt,
				local_rec.end_jt,
				lpstr,
				(char *) 0);
		}
		else
			return;
	}
	else ProcessFile ();
}

/*-----------------------------
| The guts of the processing. |
-----------------------------*/
void
ProcessFile (
 void)
{
	FILE	*pout = popen ("pformat", "w");		/* printer output */

	if (!pout)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	dsp_screen ("Printing Job Type Master Listings.",
			comm_rec.co_no, comm_rec.co_name);

	StartReport (pout, local_rec.lpno);

	memset (&cmjt_rec, 0, sizeof (cmjt_rec));
	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", local_rec.beg_jt);

	cc = find_rec (cmjt, (char *) &cmjt_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmjt_rec.br_no, comm_rec.est_no) &&
	       strcmp (cmjt_rec.job_type, local_rec.end_jt) <= 0)
	{

		PrintLine (pout);

		dsp_process ("Job Type Code : ", clip (cmjt_rec.job_type));

		cc = find_rec (cmjt, (char *) &cmjt_rec, NEXT, "r");
	}

	EndReport (pout);
	pclose (pout);
}

void
StartReport (
 FILE *	pout,
 int	prnt_no)
{
	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pout, ".START%s <%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (pout, ".LP%d\n", prnt_no);
	fprintf (pout, ".PI12\n");

	fprintf (pout, ".14\n");	/* next 14 lines descs heading */
	fprintf (pout, ".L143\n");
	fprintf (pout, ".E%s\n", "G/L ACCS BY JOB TYPE MASTER FILE LISTING");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %-24.24s\n", SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ECompany %s : %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (pout, ".B1\n");
	fprintf (pout, "From : '%-4s' - %s\n",
		local_rec.beg_jt, local_rec.beg_jt_desc);
	fprintf (pout, "  To : '%-4s' - %s\n",
		local_rec.end_jt, local_rec.end_jt_desc);

	fprintf (pout, ".R===============================================================================================================================================\n");

	fprintf (pout, "===============================================================================================================================================\n");

	fprintf (pout, "| CODE | JOB TYPE DESCRIPTION           |      WIP       |     LABOUR     |    OVERHEAD    |     SALES      |      COG       |    EXPENSES    |\n");
	fprintf (pout, "|------|--------------------------------|----------------|----------------|----------------|----------------|----------------|----------------|\n");

}

/*===========================
| Validate and print lines. |
===========================*/
void
PrintLine (
 FILE *	pout)
{
   	fprintf (pout,
			 "| %s | %s |%-16.16s|%-16.16s|%-16.16s|%-16.16s|%-16.16s|%-16.16s|\n",
			 cmjt_rec.job_type,
			 cmjt_rec.desc,
			 cmjt_rec.wip_glacc,
			 cmjt_rec.lab_glacc,
			 cmjt_rec.o_h_glacc,
			 cmjt_rec.sal_glacc,
			 cmjt_rec.cog_glacc,
			 cmjt_rec.var_glacc);
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (
 FILE *	pout)
{
	fprintf (pout, ".EOF\n");
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		line_at (1,0,80);

		rv_pr (ML (mlCmMess126), (80 - strlen (ML (mlCmMess126))) / 2, 0, 1);

		box (0, 3, 80, 6);

		line_at (6,1,79);
		line_at (20,0,80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0, err_str, comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
SrchCmjt (
 char *	key_val)
{
	work_open ();
	save_rec ("#Type", "#Description");

	memset (&cmjt_rec, 0, sizeof (cmjt_rec));
	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", key_val);

	cc = find_rec (cmjt, (char *) &cmjt_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmjt_rec.br_no, comm_rec.est_no) &&
	       !strncmp (cmjt_rec.job_type, key_val, strlen (key_val)))
	{
		if (save_rec (cmjt_rec.job_type, cmjt_rec.desc))
			break;

		cc = find_rec (cmjt, (char *) &cmjt_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
}

int
GetDescription (
 char *	key,
 char *	dst)
{
	/*
	 * Set up match
	 */
	memset (&cmjt_rec, 0, sizeof (cmjt_rec));	
	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	strcpy (cmjt_rec.job_type, key);

	if (find_rec (cmjt, (char *) &cmjt_rec, EQUAL, "r"))
	{
		print_mess (ML (mlStdMess056));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmjt_rec.desc);
	return (TRUE);
}
