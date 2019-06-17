/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( csthd_list.c )                                   |
|  Program Desc  : ( CostHeads Master File Listing                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmcm,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : 16/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (10/09/97)      | Modified  by : Ana Marie Tario.  |
|                :                                                    |
|                                                                     |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (10/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
| $Log: csthd_lst.c,v $
| Revision 5.3  2002/07/17 09:56:59  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:08  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:58  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:04  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:23  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:32:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/29 21:40:40  cam
| Changes for GVision compatibility
|
| Revision 1.11  1999/11/17 06:39:08  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/08 04:35:37  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.9  1999/10/01 07:48:19  scott
| Updated for standard function calls.
|
| Revision 1.8  1999/09/29 10:10:15  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 04:40:08  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.6  1999/09/16 04:44:41  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/14 07:34:14  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME		= "cm_csthd_lst.c",
	*PROG_VERSION	= "@(#) - (Logistic Release 9.9 & 9.10 - Program Version-9.3) 98/08/12 08:01:45";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>

#define	LINELEN	110

char	*ruler = "==============================================================================================================\n";

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
		{"comm_crd_date"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		long	tcrd_date;
	} comm_rec;

	/*===========================================+
	 | Contract Management Costheads Master File |
	 +===========================================*/
#define	CMCM_NO_FIELDS	10

	struct dbview	cmcm_list [CMCM_NO_FIELDS] =
	{
		{"cmcm_co_no"},
		{"cmcm_ch_code"},
		{"cmcm_desc"},
		{"cmcm_hhum_hash"},
		{"cmcm_rep_conv"},
		{"cmcm_usr_ref1"},
		{"cmcm_usr_ref2"},
		{"cmcm_usr_ref3"},
		{"cmcm_usr_ref4"},
		{"cmcm_usr_ref5"},
	};

	struct tag_cmcmRecord
	{
		char	co_no [3];
		char	ch_code [5];
		char	desc [41];
		long	hhum_hash;
		float	rep_conv;
		char	usr_ref[5] [5];
	}	cmcm_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview	inum_list [] =
	{
		{"inum_hhum_hash"},
		{"inum_uom"},
	};

	int	inum_no_fields = 2;

	struct tag_inumRecord
	{
		long	hhum_hash;
		char	uom [5];
	}	inum_rec;

	char 	*data	= "data",
			*cmcm	= "cmcm",
			*inum	= "inum";

/*======
 Globals
========*/
	int	bgmode = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	beg_ch [5];
	char	beg_ch_desc [41];
	char	end_ch [5];
	char	end_ch_desc [41];

	int		lpno;
	char	back [8];
	char	onight [8];

	char	dummy [11];

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "beg_cost", 	4, 21, CHARTYPE,
		"UUUU", "          ",
		" ", "    ", " Start Costhead Code :", "Enter the starting costhead code",
		YES, NO,  JUSTLEFT, "", "", local_rec.beg_ch},
	{1, LIN, "beg_desc", 	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.beg_ch_desc},
	{1, LIN, "end_cost", 	6, 21, CHARTYPE,
		"UUUU", "          ",
		" ", "~~~~", " End Costhead Type   :", "Enter the ending costhead code",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_ch},
	{1, LIN, "end_desc", 	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_ch_desc},

	{1, LIN, "lpno",  	8, 20, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No  :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{1, LIN, "back", 	9, 20, CHARTYPE,
		"U", "          ",
		" ", "N", " Background  :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", (char *) local_rec.back},
	{1, LIN, "onight", 	10, 20, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight   :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", (char *) local_rec.onight},

	{0, LIN, "", 	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
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
void	SrchCmcm		(char * key_val);
int		getDescription	(char * key, char * dst);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	if (argc != 2 && argc != 4)
	{
/*
		printf("\007Usage : %s <description>\n", argv[0]);
		printf(" OR   : %s <start costhead>\n", argv[0]);
		printf("           <end costhead>\n");
		printf("           <lpno>\n");*/
		print_at(0,0,ML(mlCmMess700),argv[0]);
		print_at(1,0,ML(mlCmMess745),argv[0]);
		print_at(2,0,ML(mlCmMess746));
		print_at(3,0,ML(mlCmMess727));

		return (argc);
	}

	/*====================================
	| Open db and read in terminal record
	======================================*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 4)
	{
		/* background mode */

		sprintf(local_rec.beg_ch, "%-4.4s", argv[1]);
		sprintf(local_rec.end_ch, "%-4.4s", argv[2]);
		local_rec.lpno = atoi(argv[3]);

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

	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (inum, inum_list, inum_no_fields, "inum_hhum_hash");
}

void
CloseDB (void)
{
	abc_fclose (cmcm);
	abc_fclose (inum);
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
			/*print_mess ("\007 Invalid Printer Number ");*/
			print_mess(ML(mlStdMess020));
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
	| Validate Start Job Type. |
	--------------------------*/
	if (LCHECK ("beg_cost"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.beg_ch, "    ");
			strcpy (local_rec.beg_ch_desc, "First Costhead");
			DSP_FLD ("beg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmcm (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.beg_ch, local_rec.end_ch) > 0)
		{
/*
			sprintf (err_str, "Start costhead code %s must be less than end costhead code %s\007",
				local_rec.beg_ch,
				local_rec.end_ch);
			print_mess (err_str);*/
			print_mess(ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.beg_ch, local_rec.beg_ch_desc))
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
			strcpy (local_rec.end_ch, "~~~~");
			strcpy (local_rec.end_ch_desc, "End Costhead");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchCmcm (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.beg_ch, local_rec.end_ch) > 0)
		{
/*
			sprintf (err_str,
				"End costhead code %s must be less than start costhead code %s\007",
				local_rec.end_ch,
				local_rec.beg_ch);
			print_mess (err_str);*/
			print_mess(ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!getDescription (local_rec.end_ch, local_rec.end_ch_desc))
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

	if (local_rec.onight [0] == 'Y' || local_rec.back[0] == 'Y')
	{
		CloseDB (); 
		FinishProgram ();
	}

	/*================================
	| Test for Overnight Processing. |
	================================*/
	else if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
		{
			sprintf (lpstr, "%d", local_rec.lpno);
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.beg_ch,
				local_rec.end_ch,
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
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
		{
			sprintf (lpstr, "%d", local_rec.lpno);
			execlp (prog_name,
				prog_name,
				local_rec.beg_ch,
				local_rec.end_ch,
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

	if (!bgmode)
		dsp_screen ("Printing Contract Master Listings.",
			comm_rec.tco_no, comm_rec.tco_name);

	start_report (pout, local_rec.lpno);

	/** Set up match conditions
	**/

	memset (&cmcm_rec, 0, sizeof (cmcm_rec));	/* scrub first */
	strcpy (cmcm_rec.co_no, comm_rec.tco_no);
	sprintf(cmcm_rec.ch_code, "%-4.4s", local_rec.beg_ch);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp(cmcm_rec.co_no, comm_rec.tco_no) &&
	       strcmp(cmcm_rec.ch_code, local_rec.end_ch) <= 0)
	{
		/* extract uom */
		cc = find_hash (inum, (char *) &inum_rec, EQUAL, "r", 
				cmcm_rec.hhum_hash);
		if (cc)
		{
			file_err (cc, inum, "FIND_HASH");
			break;
		}

		print_line (pout);

		dsp_process ("CostHead Code : ", clip (cmcm_rec.ch_code));
		cc = find_rec (cmcm, (char *) &cmcm_rec, NEXT, "r");
	}

	end_report (pout);
	pclose (pout);
}

void
start_report (
 FILE *	pout,
 int	prnt_no)
{
	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pout, ".START%s <%s>\n", DateToString (comm_rec.tcrd_date), PNAME);
	fprintf (pout, ".LP%d\n", prnt_no);

	fprintf (pout, ".14\n");	/* next 14 lines descs heading */
	fprintf (pout, ".L%d\n", LINELEN);
	fprintf (pout, ".E%s\n", "COSTHEADS MASTER FILE LISTING");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %-24.24s\n", SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ECompany %s : %s\n", comm_rec.tco_no, clip (comm_rec.tco_name));
	fprintf (pout, ".B1\n");
	fprintf (pout, "From : '%-4s' - %s\n",
		local_rec.beg_ch, local_rec.beg_ch_desc);
	fprintf (pout, "  To : '%-4s' - %s\n",
		local_rec.end_ch, local_rec.end_ch_desc);

        fprintf (pout, ".R%s\n", ruler);
        fprintf (pout, "%s\n", ruler);

	fprintf (pout, ".PI10\n");
        fprintf (pout, "| Code | Costhead Description                      | Analysis Codes 1 - 5     | UnitMeasure | ReptConvFactor |\n");
        fprintf (pout, "+------+-------------------------------------------+--------------------------+-------------+----------------+\n");


	free (ruler);
}

/*===========================
| Validate and print lines. |
===========================*/
void
print_line (
 FILE *	pout)
{
	/*---------------------------
	|  Full Master file listing |
	---------------------------*/
       	fprintf (pout, "| %s | %s  | %s %s %s %s %s |      %s   |  %10.2f    |\n",
		cmcm_rec.ch_code,
		cmcm_rec.desc,
		cmcm_rec.usr_ref[0],
		cmcm_rec.usr_ref[1],
		cmcm_rec.usr_ref[2],
		cmcm_rec.usr_ref[3],
		cmcm_rec.usr_ref[4],
		inum_rec.uom,
		cmcm_rec.rep_conv);
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
/*	char	*title = "CostHeads Master File Listings";*/
	char	*title = ML(mlCmMess172); /*"CostHeads Master File Listings";*/

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		move (0, 0);
		move(0, 1);
		line(79);

		rv_pr(title, (80 - strlen (title)) / 2, 0, 1);

		box(0, 3, 80, 7);

		move(1, 5);
		line(79);
		move(1, 7);
		line(79);

		move(0, 20);
		line(80);

		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,clip(comm_rec.tco_name));
		strcpy(err_str,ML(mlStdMess039));
		print_at(21,40,err_str,comm_rec.test_no,clip(comm_rec.test_name));

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
/*
		print_at (21,0," Co no : %s - %s  Br no : %s - %s ",
			comm_rec.tco_no,
			clip (comm_rec.tco_name),
			comm_rec.test_no,
			clip (comm_rec.test_name));*/
	}
	return (EXIT_SUCCESS);
}

void
SrchCmcm (
 char *	key_val)
{
	work_open ();
	save_rec ("#Type", "#Description");

	/* Set up match codes
	*/
	memset (&cmcm_rec, 0, sizeof (cmcm_rec));	/* flush out */
	strcpy (cmcm_rec.co_no, comm_rec.tco_no);
	strcpy (cmcm_rec.ch_code, key_val);

	for (cc = find_rec (cmcm, (char *) &cmcm_rec, GTEQ, "r");
		!cc &&
			!strcmp (cmcm_rec.co_no, comm_rec.tco_no) &&
			!strncmp (cmcm_rec.ch_code, key_val, strlen (key_val));
		cc = find_rec (cmcm, (char *) &cmcm_rec, NEXT, "r"))
	{
		if (save_rec (cmcm_rec.ch_code, cmcm_rec.desc))
			break;
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
	memset (&cmcm_rec, 0, sizeof (cmcm_rec));	/* flush */
	strcpy (cmcm_rec.co_no, comm_rec.tco_no);
	strcpy (cmcm_rec.ch_code, key);

	if (find_rec (cmcm, (char *) &cmcm_rec, EQUAL, "r"))
	{
		/*sprintf (err_str, "Costhead code '%s' not found\007", key);
		print_mess (err_str);*/
		print_mess(ML(mlStdMess055));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmcm_rec.desc);
	return (TRUE);
}
