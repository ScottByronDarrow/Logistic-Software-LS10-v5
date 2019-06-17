/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( equip_lst.c )                                    |
|  Program Desc  : ( Plant Master Listing                         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmeq,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : 17/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Jiggs A. Veloz.   |
|                                                                     |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (03/09/97)    : SEL - Multilingual Conversion.                     |
|                :                                                    |
|                                                                     |
| $Log: equip_lst.c,v $
| Revision 5.3  2002/07/17 09:57:00  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:16  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:32  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:32:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/29 20:41:32  cam
| Changes for GVision compatibility
|
| Revision 1.10  1999/11/17 06:39:12  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/11/08 04:35:39  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.8  1999/09/29 10:10:19  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 04:40:10  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.6  1999/09/16 04:44:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/14 07:34:35  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME		= "cm_equip_lst.c",
	*PROG_VERSION	= "@(#) - (Logistic Release 9.9 & 9.10 - Program Version-9.3) 98/08/12 08:07:30";

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

	/*===========================================
	| Contract Management Equipment Master File |
	===========================================*/
	struct dbview	cmeq_list [] =
	{
		{"cmeq_co_no"},
		{"cmeq_eq_name"},
		{"cmeq_desc"},
		{"cmeq_rate"}
	};

	int	cmeq_no_fields = 4;

	struct tag_cmeqRecord
	{
		char	co_no [3];
		char	eq_name [11];
		char	desc [41];
/* money */	double	rate;
	}	cmeq_rec;

/*======
 Globals
========*/
	int	bgmode = FALSE;

/*===========
 Table names
============*/
static char
	*data	= "data",
	*cmeq	= "cmeq";

/*============================
| Local & Screen Structures. |
============================*/
#define	CODE_WIDTH	10
struct
{
	char	beg_eq [CODE_WIDTH + 1];
	char	beg_eq_desc [41];
	char	end_eq [CODE_WIDTH + 1];
	char	end_eq_desc [41];

	int	lpno;
	char	back [8];
	char	onight [8];

	int	dummy;

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "beg_cost", 	4, 18, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Start Plant Code :", "Enter the starting plant code",
		YES, NO,  JUSTLEFT, "", "", local_rec.beg_eq},
	{1, LIN, "beg_desc", 	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.beg_eq_desc},
	{1, LIN, "end_cost", 	5, 18, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "~", " End Plant Code   :", "Enter the ending plant code",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_eq},
	{1, LIN, "end_desc", 	5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_eq_desc},

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
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
void	run_prog		(char * prog_name, char * prog_desc);
void	proc_file		(void);
void	start_report	(FILE * pout, int prnt_no);
void	print_line		(FILE * pout);
void	end_report		(FILE * pout);
int		heading			(int scn);
void	srch_cmeq		(char * key_val);
int		getDescription	(char * key, char * dst);


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
		print_at(0,0, ML(mlCmMess700), argv [0]);
		print_at(1,0, ML(mlCmMess718), argv [0]);
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
		sprintf(local_rec.beg_eq, "%-10.10s", argv[1]);
		sprintf(local_rec.end_eq, "%-10.10s", argv[2]);
		local_rec.lpno = atoi (argv [3]);

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

		run_prog (argv[0], argv[1]);
		prog_exit = 1;
	}

	/*========================
	| Program exit sequence. |
	========================*/
	shutdown_prog ();
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
	open_rec (cmeq, cmeq_list, cmeq_no_fields, "cmeq_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cmeq);
	abc_dbclose (data);
}

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
			/*------------------
			| Invalid Printer. |
			------------------*/
			print_mess( ML(mlStdMess020) ); 
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		strcpy (local_rec.back,
			*local_rec.back == 'N' ? "N (No) " : "Y (Yes)");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight,
			*local_rec.onight == 'N' ? "N (No) " : "Y (Yes)");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Start Equipment Code. |
	--------------------------*/
	if (LCHECK ("beg_cost"))
	{
		if (dflt_used)
		{
			memset (local_rec.beg_eq, ' ', CODE_WIDTH);
			local_rec.beg_eq [CODE_WIDTH] = '\0';
			strcpy (local_rec.beg_eq_desc, "First Plant");
			DSP_FLD ("beg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			srch_cmeq (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.beg_eq, local_rec.end_eq) > 0)
		{
			/*----------------
			| Invalid Range. |
			----------------*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.beg_eq, local_rec.beg_eq_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("beg_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate End Equipment Code. |
	------------------------*/
	if (LCHECK ("end_cost"))
	{
		if (dflt_used)
		{
			memset (local_rec.end_eq, '~', CODE_WIDTH);
			local_rec.end_eq [CODE_WIDTH] = '\0';
			strcpy (local_rec.end_eq_desc, "Last Plant");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			srch_cmeq (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.beg_eq, local_rec.end_eq) > 0)
		{
			/*----------------
			| Invalid range. |
			----------------*/
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.end_eq, local_rec.end_eq_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("end_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
run_prog (
 char *	prog_name,
 char *	prog_desc)
{
	char	lpstr [10];

	if (local_rec.onight [0] == 'Y' || local_rec.back [0] == 'Y')
		shutdown_prog ();

	sprintf(lpstr, "%2d", local_rec.lpno);

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.beg_eq,
				local_rec.end_eq,
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
		if (fork() == 0)
		{
			execlp (prog_name,
				prog_name,
				local_rec.beg_eq,
				local_rec.end_eq,
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

	memset (&cmeq_rec, 0, sizeof (cmeq_rec));	/* scrub first */
	strcpy (cmeq_rec.co_no, comm_rec.tco_no);
	sprintf(cmeq_rec.eq_name, "%-10.10s", local_rec.beg_eq);
	cc = find_rec (cmeq, (char *) &cmeq_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmeq_rec.co_no, comm_rec.tco_no) &&
	       strcmp (cmeq_rec.eq_name, local_rec.end_eq) <= 0)
	{

		print_line (pout);

		dsp_process ("Equipment Code : ", clip (cmeq_rec.eq_name));

		cc = find_rec (cmeq, (char *) &cmeq_rec, NEXT, "r");
	}

	end_report (pout);
	pclose (pout);
}

#define	LINELEN	74

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

	fprintf (pout, ".14\n");	/* next 14 lines descs heading */
	fprintf (pout, ".L%d\n", LINELEN);
	fprintf (pout, ".E%s\n", "PLANT NUMBERS MASTER FILE LISTING");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %s\n", SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ECompany %s : %s\n", comm_rec.tco_no, clip (comm_rec.tco_name));
	fprintf (pout, ".B1\n");
	fprintf (pout, "From : '%-4s' - %s\n",
		local_rec.beg_eq, local_rec.beg_eq_desc);
	fprintf (pout, "  To : '%-4s' - %s\n",
		local_rec.end_eq, local_rec.end_eq_desc);

        fprintf (pout, ".R%s\n", ruler);
        fprintf (pout, "%s\n", ruler);

	fprintf (pout, ".PI10\n");
        fprintf (pout, "| Plant No.  | Equipment Code Description               |  Rate ($/unit) |\n");
        fprintf (pout, "+------------+------------------------------------------+----------------+\n");


	free (ruler);
}

/*===========================
| Validate and print lines. |
===========================*/
void
print_line (
 FILE *	pout)
{
       	fprintf (pout, "| %s | %s | %14.2f |\n",
		cmeq_rec.eq_name,
		cmeq_rec.desc,
		cmeq_rec.rate / 100);
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
	
		sprintf(err_str, ML(mlCmMess068) );

		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

		box (0, 3, 80, 6);

		move (1, 6);
		line (79);

		move (0, 20);
		line (80);
		print_at (21,0, ML(mlStdMess038), 
							comm_rec.tco_no, clip (comm_rec.tco_name));
		print_at (21,45, ML(mlStdMess039), 
							comm_rec.test_no, clip (comm_rec.test_name));

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
srch_cmeq (
 char *	key_val)
{
	work_open ();
	save_rec ("#Name", "#Description");

	/* Set up match codes
	*/
	memset (&cmeq_rec, 0, sizeof (cmeq_rec));	/* flush out */
	strcpy (cmeq_rec.co_no, comm_rec.tco_no);
	sprintf(cmeq_rec.eq_name, "%-10.10s", key_val);
	while (!cc &&
	       !strcmp (cmeq_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (cmeq_rec.eq_name, key_val, strlen (key_val)))
	{
		if (save_rec (cmeq_rec.eq_name, cmeq_rec.desc))
			break;

		cc = find_rec (cmeq, (char *) &cmeq_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
}

int
getDescription (
 char *	key,
 char *	dst)
{
	/* Set up match
	*/
	memset (&cmeq_rec, 0, sizeof (cmeq_rec));	/* flush */
	strcpy (cmeq_rec.co_no, comm_rec.tco_no);
	strcpy (cmeq_rec.eq_name, key);

	if (find_rec (cmeq, (char *) &cmeq_rec, EQUAL, "r"))
	{
		/*--------------------------
		| Equipment code not found. |
		---------------------------*/
		print_mess (ML(mlCmMess001));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmeq_rec.desc);
	return (TRUE);
}
