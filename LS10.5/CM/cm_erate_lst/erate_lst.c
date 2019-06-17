/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( erate_lst.c )                                    |
|  Program Desc  : ( Employee Rates Listing                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmem, cmer, cmcm, cmhr, cmcd,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : 17/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Jiggs A Veloz.    |
|                :                                                    |
|                :                                                    |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (03/09/97)    : SEL - Multilingual Conversion.                     |
|                :                                                    |
|                                                                     |
| $Log: erate_lst.c,v $
| Revision 5.3  2002/07/17 09:57:00  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:12  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:32  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:32:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/29 20:53:46  cam
| Changes for GVision compatibility
|
| Revision 1.10  1999/11/17 06:39:13  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/11/08 04:35:39  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.8  1999/09/29 10:10:20  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 04:40:10  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.6  1999/09/16 04:44:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/14 07:34:37  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME		= "cm_erate_lst.c",
	*PROG_VERSION	= "@(#) - (Logistic Release 9.9 & 9.10 - Program Version-9.3) 98/08/12 08:08:07";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include	<string.h>
#include	<memory.h>
#include	<malloc.h>
#include	<ml_cm_mess.h>
#include	<ml_std_mess.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"}
	};

	int comm_no_fields = 6;

	struct
	{
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		long	tcrd_date;
	} comm_rec;

	/*==========================================
	| Contract Management Employee Master File |
	==========================================*/
	struct dbview	cmem_list [] =
	{
		{"cmem_co_no"},
		{"cmem_emp_no"},
		{"cmem_emp_name"},
		{"cmem_hhem_hash"},
	};

	int	cmem_no_fields = 4;

	struct tag_cmemRecord
	{
		char	co_no [3];
		char	emp_no [11];
		char	emp_name [41];
		long	hhem_hash;
	}	cmem_rec;

	/*================================================
	| Contract Management Employee Rates Master File |
	================================================*/
	struct dbview	cmer_list [] =
	{
		{"cmer_hhem_hash"},
		{"cmer_hhhr_hash"},
		{"cmer_hhcm_hash"},
		{"cmer_lab_rate"},
		{"cmer_lab_pc"},
		{"cmer_o_h_rate"},
		{"cmer_o_h_pc"}
	};

	int	cmer_no_fields = 7;

	struct tag_cmerRecord
	{
		long	hhem_hash;
		long	hhhr_hash;
		long	hhcm_hash;
/* money */	double	lab_rate;
		float	lab_pc;
/* money */	double	o_h_rate;
		float	o_h_pc;
	} cmer_rec;

	/*========================================
	| cmhr - Contract Management Header File |
	========================================*/
	struct dbview	cmhr_list [] =
	{
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
	};

	int	cmhr_no_fields = 2;

	struct tag_cmhrRecord
	{
		char	cont_no [7];
		long	hhhr_hash;
	}	cmhr_rec;

	/*===========================================
	| Contract Management Costheads Master File |
	===========================================*/
	struct dbview	cmcm_list [] =
	{
		{"cmcm_ch_code"},
		{"cmcm_desc"},
		{"cmcm_hhcm_hash"},
	};

	int	cmcm_no_fields = 3;

	struct tag_cmcmRecord
	{
		char	ch_code [5];
		char	desc [41];
		long	hhcm_hash;
	}	cmcm_rec;

	/*===============================================
	| Contract Management Contract Description File |
	===============================================*/
	struct dbview	cmcd_list [] =
	{
		{"cmcd_hhhr_hash"},
		{"cmcd_line_no"},
		{"cmcd_text"},
		{"cmcd_stat_flag"}
	};

	int	cmcd_no_fields = 4;

	struct tag_cmcdRecord
	{
		long	hhhr_hash;
		int	line_no;
		char	text [71];
		char	stat_flag [2];
	}	cmcd_rec;

/*======
 Globals
========*/
	int	bgmode = FALSE;

/*===========
 Table names
============*/
static char
	*data	= "data",
	*cmcd	= "cmcd",
	*cmcm	= "cmcm",
	*cmem	= "cmem",
	*cmer	= "cmer",
	*cmhr	= "cmhr";

/*============================
| Local & Screen Structures. |
============================*/
#define	CODE_WIDTH	10
struct
{
	char	beg_ec [CODE_WIDTH + 1];
	char	beg_ec_desc [41];
	char	end_ec [CODE_WIDTH + 1];
	char	end_ec_desc [41];

	int	lpno;
	char	back [8];
	char	onight [8];

	int	dummy;

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "beg_cost", 	4, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Start Employee :", "Enter the starting employee code",
		YES, NO,  JUSTLEFT, "", "", local_rec.beg_ec},
	{1, LIN, "beg_desc", 	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.beg_ec_desc},
	{1, LIN, "end_cost", 	5, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "~", " End Employee   :", "Enter the ending employee code",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_ec},
	{1, LIN, "end_desc", 	5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_ec_desc},

	{1, LIN, "lpno", 	 7, 16, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{1, LIN, "back", 	 8, 16, CHARTYPE,
		"U", "          ",
		" ", "N", " Background :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "onight", 	 9, 16, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight  :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{0, LIN, "", 	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.dummy},
};

/*=====================
 Function declarations
======================*/
int		main				(int argc, char * argv []);
void	shutdown_prog		(void);
void	OpenDB				(void);
void	CloseDB			(void);
int		spec_valid			(int field);
void	run_prog			(char * prog_name, char * prog_desc);
void	proc_file			(void);
void	start_report		(FILE * pout, int prnt_no);
void	print_line			(FILE * pout, int brkEmpl, int brkCont);
void	end_report			(FILE * pout);
int		heading				(int scn);
void	srch_cmem			(char * key_val);
int		getDescription		(char * key, char * dst);
void	getContractDetails	(long hhhr_hash);
void	getCostheadDetails	(long hhcm_hash);

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
		print_at(0,0,ML(mlCmMess700), argv[0]);
		print_at(0,0,ML(mlCmMess719), argv[0]);
		return (argc);
	}

	/*====================================
	| Open db and read in terminal record
	======================================*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 4)
	{
		memset (&local_rec, 0, sizeof (local_rec));
		sprintf(local_rec.beg_ec, "%-10.10s", argv[1]);
		sprintf(local_rec.end_ec, "%-10.10s", argv[2]);
		local_rec.lpno = atoi (argv[3]);

		proc_file();
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
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = TRUE;
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
		run_prog (argv [0], argv[1]);
		prog_exit = 1;
	}

	/*========================
	| Program exit sequence. |
	========================*/
	shutdown_prog();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*==============================
 Open and closes of db and files
================================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (cmcd, cmcd_list, cmcd_no_fields, "cmcd_id_no");
	open_rec (cmer, cmer_list, cmer_no_fields, "cmer_id_no");
	open_rec (cmem, cmem_list, cmem_no_fields, "cmem_id_no");
	open_rec (cmcm, cmcm_list, cmcm_no_fields, "cmcm_hhcm_hash");
	open_rec (cmhr, cmhr_list, cmhr_no_fields, "cmhr_hhhr_hash");
}

void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmer);
	abc_fclose (cmem);
	abc_fclose (cmcm);
	abc_fclose (cmhr);
	abc_dbclose (data);
}

/** Trap lib routine
**/
int
spec_valid (
 int	field)
{
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp(local_rec.lpno))
		{
			print_mess( ML(mlStdMess020) );
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}
	}

	else if (LCHECK ("back"))
	{
		/*---------------------------------------------
		| Validate Field Selection background option. |
		---------------------------------------------*/
		strcpy (local_rec.back,
			*local_rec.back == 'N' ? "N (No) " : "Y (Yes)");
	}

	else if (LCHECK ("onight"))
	{
		/*--------------------------------------------
		| Validate Field Selection overnight option. |
		--------------------------------------------*/
		strcpy (local_rec.onight,
			*local_rec.onight == 'N' ? "N (No) " : "Y (Yes)");
	}

	else if (LCHECK ("beg_cost"))
	{
		/*--------------------------
		| Validate Start Employee Code. |
		--------------------------*/
		if (dflt_used)
		{
			memset (local_rec.beg_ec, ' ', CODE_WIDTH);
			local_rec.beg_ec [CODE_WIDTH] = '\0';
			strcpy (local_rec.beg_ec_desc, "First Employee");
			DSP_FLD ("beg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			srch_cmem (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.beg_ec, local_rec.end_ec) > 0)
		{
			/*--------------
			| Invalid Range |
			---------------*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.beg_ec, local_rec.beg_ec_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("beg_desc");
	}

	else if (LCHECK ("end_cost"))
	{
		/*------------------------
		| Validate End Employee Code. |
		------------------------*/
		if (dflt_used)
		{
			memset (local_rec.end_ec, '~', CODE_WIDTH);
			local_rec.end_ec [CODE_WIDTH] = '\0';
			strcpy (local_rec.end_ec_desc, "Last Employee");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			srch_cmem (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.beg_ec, local_rec.end_ec) > 0)
		{
			/*--------------
			| Invalid Range |
			---------------*/
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.end_ec, local_rec.end_ec_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("end_desc");
	}
	return (EXIT_SUCCESS);
}

void
run_prog (
 char *	prog_name,
 char *	prog_desc)
{
	char	lpstr [10];

	if (local_rec.onight[0] == 'Y' || local_rec.back[0] == 'Y')
	{
		CloseDB (); FinishProgram ();; FinishProgram ();;
		snorm();
		rset_tty();
	}

	sprintf (lpstr, "%d", local_rec.lpno);

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp ("/u/DB/BIN/MENU/ONIGHT",
				"/u/DB/BIN/MENU/ONIGHT",
				prog_name,
				local_rec.beg_ec,
				local_rec.end_ec,
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
	if (local_rec.back [0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp (prog_name,
				prog_name,
				local_rec.beg_ec,
				local_rec.end_ec,
				lpstr,
				(char *) 0);
		}
		else
			return;
	}
	else
		proc_file ();
}

/*-----------------------------
| The guts of the processing. |
-----------------------------*/
void
proc_file (void)
{
	FILE	*pout = popen ("pformat", "w");		/* printer output */

	if (!pout)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	dsp_screen ("Printing Contract Master Listings.",
			comm_rec.tco_no, comm_rec.tco_name);

	start_report (pout, local_rec.lpno);

	memset (&cmem_rec, 0, sizeof (cmem_rec));	/* scrub first */
	strcpy (cmem_rec.co_no, comm_rec.tco_no);
	sprintf(cmem_rec.emp_no, "%-10.10s", local_rec.beg_ec);
	cc = find_rec (cmem, (char *) &cmem_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmem_rec.co_no, comm_rec.tco_no) &&
	       strcmp (cmem_rec.emp_no, local_rec.end_ec) <= 0)
	{
		int	brkEmpl,	/* break indicator for Employee */
			brkCont;	/* break indicator for Contract */

		brkEmpl = TRUE;
		memset (&cmhr_rec, 0, sizeof (cmhr_rec));

		/* Dump out all the rates record for this employee
		*/
		memset (&cmer_rec, 0, sizeof (cmer_rec));
		cmer_rec.hhem_hash = cmem_rec.hhem_hash;
		cmer_rec.hhhr_hash = 0L;
		cmer_rec.hhcm_hash = 0L;
		cc = find_rec (cmer, (char *) &cmer_rec, GTEQ, "r");
		while (!cc && cmer_rec.hhem_hash == cmem_rec.hhem_hash)
		{

			/* Grab relevant Contract and description record out
			   (only if different from previous one)
			*/
			brkCont = cmer_rec.hhhr_hash != cmhr_rec.hhhr_hash;
			if (brkCont || !cmer_rec.hhhr_hash)
				getContractDetails (cmer_rec.hhhr_hash);

			/* Grab relevant Costhead record out
			*/
			getCostheadDetails (cmer_rec.hhcm_hash);

			print_line (pout, brkEmpl, brkCont);

			brkEmpl = FALSE;

			cc = find_rec (cmer, (char *) &cmer_rec, NEXT, "r");
		}

		dsp_process ("Employee Code : ", clip (cmem_rec.emp_no));

		cc = find_rec (cmem, (char *) &cmem_rec, NEXT, "r");
	}

	end_report (pout);
	pclose (pout);
}

#define	LINELEN	132

void
start_report (
 FILE *	pout,
 int	prnt_no)
{
	char	*ruler = (char *) malloc (LINELEN + 1U);

	memset (ruler, '=', LINELEN);
	ruler [LINELEN] = '\0';

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pout, ".START%s <%s>\n", DateToString (comm_rec.tcrd_date), PNAME);
	fprintf (pout, ".LP%d\n", prnt_no);

	fprintf (pout, ".15\n");	/* next 14 lines descs heading */
	fprintf (pout, ".L%d\n", LINELEN);
	fprintf (pout, ".E%s\n", "EMPLOYEE RATES MASTER FILE LISTING");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %-24.24s\n", SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ECompany %s : %s\n", comm_rec.tco_no, clip (comm_rec.tco_name));
	fprintf (pout, ".B1\n");
	fprintf (pout, "From : '%-4s' - %s\n",
		local_rec.beg_ec, local_rec.beg_ec_desc);
	fprintf (pout, "  To : '%-4s' - %s\n",
		local_rec.end_ec, local_rec.end_ec_desc);

        fprintf (pout, ".R%s\n", ruler);
        fprintf (pout, "%s\n", ruler);

	fprintf (pout, ".PI10\n");
        fprintf (pout, "| Emp Code   Employee Name           | Contract                        | Costhead                  |        Labour |      Overhead |\n");
        fprintf (pout, "|                                    | Number Description              | Code    Description       |    Rate     %% |    Rate     %% |\n");
        fprintf (pout, "+------------------------------------+---------------------------------+---------------------------+---------------+---------------+\n");

	free (ruler);
}

/*===========================
| Validate and print lines. |
===========================*/
void
print_line (
 FILE *	pout,
 int	brkEmpl,
 int	brkCont)
{
	char	*nullStr = "";

	/* Normalise break conditions
	*/
	if (brkEmpl)
		brkCont = TRUE;

       	fprintf (pout, "| %-10s %-23.23s | %-6s %-24.24s | %-4s %-20.20s | %7.2f %5.2f | %7.2f %5.2f |\n",
		brkEmpl ? cmem_rec.emp_no	: nullStr,
		brkEmpl ? cmem_rec.emp_name	: nullStr,
		brkCont ? cmhr_rec.cont_no	: nullStr,
		brkCont ? cmcd_rec.text		: nullStr,
		cmcm_rec.ch_code,
		cmcm_rec.desc,
		cmer_rec.lab_rate / 100,
		cmer_rec.lab_pc,
		cmer_rec.o_h_rate / 100,
		cmer_rec.o_h_pc);
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (
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
 int	scn)
{
	if (!restart)
	{

		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		move (0, 1);
		line (80);

		/*------------------------------
		| Employee Rates Master Listing |
		-------------------------------*/
		sprintf (err_str, ML(mlCmMess105) ); 
		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

		box (0, 3, 80, 6);

		move (1, 6);
		line (79);

		move (0, 20);
		line (80);
		print_at (21,0, ML(mlStdMess038), 
							comm_rec.tco_no, clip(comm_rec.tco_name));
		print_at (21,45, ML(mlStdMess039), 
							comm_rec.test_no, clip(comm_rec.test_name));

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
srch_cmem (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code", "#Name");

	/* Set up match codes
	*/
	memset (&cmem_rec, 0, sizeof (cmem_rec));	/* flush out */
	strcpy (cmem_rec.co_no, comm_rec.tco_no);
	sprintf(cmem_rec.emp_no, "%-10.10s", key_val);
	cc = find_rec (cmem, (char *) &cmem_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmem_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (cmem_rec.emp_no, key_val, strlen (key_val)))
	{
		if (save_rec (cmem_rec.emp_no, cmem_rec.emp_name))
			break;

		cc = find_rec (cmem, (char *) &cmem_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
}

int
getDescription (
 char *	key,
 char * dst)
{
	/* Set up match
	*/
	memset (&cmem_rec, 0, sizeof (cmem_rec));	/* flush */
	strcpy (cmem_rec.co_no, comm_rec.tco_no);
	strcpy (cmem_rec.emp_no, key);

	if (find_rec (cmem, (char *) &cmem_rec, EQUAL, "r"))
	{
		/*------------------------
		| Employee code not found |
		-------------------------*/
		print_mess (ML(mlCmMess002));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmem_rec.emp_name);
	return (TRUE);
}

void
getContractDetails (
 long	hhhr_hash)
{
	memset (&cmhr_rec, 0, sizeof (cmhr_rec));
	memset (&cmcd_rec, 0, sizeof (cmcd_rec));

	if (!hhhr_hash)
		return;

	/* Get contract record
	*/
	if ((cc = find_hash (cmhr, (char *) &cmhr_rec, EQUAL, "r", hhhr_hash)))
	{
		strcpy (cmhr_rec.cont_no, "*ERR*");
		strcpy (cmcd_rec.text, "**ERROR**");
		return;
	}

	/* Get first line of description
	*/
	cmcd_rec.hhhr_hash = hhhr_hash;			/* setup index */
	strcpy (cmcd_rec.stat_flag, "D");		/* for desc(?) */
	cmcd_rec.line_no = 0;

	if ((cc = find_rec (cmcd, (char *) &cmcd_rec, EQUAL, "r")))
		strcpy (cmcd_rec.text, "**ERROR**");
}

void
getCostheadDetails (
 long	hhcm_hash)
{
	if (hhcm_hash)
	{
		if ((cc = find_hash (cmcm, (char *) &cmcm_rec, EQUAL, "r", hhcm_hash)))
			strcpy (cmcm_rec.desc, "**DATA ERROR**");
	} else
		memset (&cmcm_rec, 0, sizeof (cmcm_rec));
}
