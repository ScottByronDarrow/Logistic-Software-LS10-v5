/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: complaint.c,v 5.4 2002/07/17 09:58:16 scott Exp $
|  Program Name  : (ts_complaint.c)
|  Program Desc  : (Print Telesales Customer Complaints)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 15/06/92         |
|---------------------------------------------------------------------|
| $Log: complaint.c,v $
| Revision 5.4  2002/07/17 09:58:16  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/20 05:35:28  robert
| Updated to fix default value validation in LS10-GUI
|
| Revision 5.2  2001/08/09 09:23:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:55:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:06  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/27 09:50:10  scott
| Updated to fix core dump on some machines (always in windows).
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: complaint.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_complaint/complaint.c,v 5.4 2002/07/17 09:58:16 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <ml_std_mess.h>
#include <ml_ts_mess.h>
#include <dsp_process.h>

#define	LINE_LEN	128

FILE	*fout;

	extern	int	TruePosition;
	extern	int	EnvScreenOK;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	envVarDbCo 		= 0, 
			envVarDbFind 	= 0;
	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct exsfRecord	exsf_rec;
struct cumrRecord	cumr_rec;
struct tspmRecord	tspm_rec;
struct tsxdRecord	tsxd_rec;

	char	*data    = "data"; 

	int		firstTime;
	long	lsystemDate;
	char	systemDate [11];
	char	printLine [256];
	char	leftOver [256];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	startOperator [15];
	char	startOperatorName [41];
	char	endOperator [15];
	char	endOperatorName [41];
	char	startCust [7];
	char	startCustName [41];
	char	endCust [7];
	char	endCustName [41];
	char	startSman [3];
	char	startSmanName [41];
	char	endSman [3];
	char	endSmanName [41];
	char	back [2];
	char	backDesc [11];
	char	onight [2];
	char	onightDesc [11];
	int		printerNumber;
	char	printerString [3];
	char	noteType [2];
} local_rec;

static struct	var vars [] ={
	{1, LIN, "startOperator", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Start Operator    ", "Enter Start Operator.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startOperator}, 
	{1, LIN, "startOperatorName", 4, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startOperatorName}, 
	{1, LIN, "endOperator", 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "End Operator      ", "Enter End Operator.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endOperator}, 
	{1, LIN, "endOperatorName", 5, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endOperatorName}, 
	{1, LIN, "startCust", 7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Start Customer    ", "Enter Start Cust.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startCust}, 
	{1, LIN, "startCustName", 7, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startCustName}, 
	{1, LIN, "endCust", 8, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Customer      ", "Enter End Cust.", 	
		NO, NO, JUSTLEFT, "", "", local_rec.endCust}, 
	{1, LIN, "endCustName", 8, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endCustName}, 
	{1, LIN, "startSman", 10, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Start Salesperson ", "Enter Start SalesPerson.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.startSman}, 
	{1, LIN, "startSmanName", 10, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startSmanName}, 
	{1, LIN, "endSman", 11, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "End Salesperson   ", "Enter End SalesPerson.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.endSman}, 
	{1, LIN, "endSmanName", 11, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endSmanName}, 
	{1, LIN, "printerNumber", 13, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No        ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "back", 14, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N",  "Background        ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.back}, 
	{1, LIN, "backDesc", 14, 23, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", "",  "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N",  "Overnight         ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.onight}, 
	{1, LIN, "onightDesc", 15, 23, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", "",  "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};

#include <FindCumr.h>

/*----------------------------
| Local Function Prototypes  |
----------------------------*/
int     AppendComm 			(char *, char *);
int     HeadingOutput 		(void);
int     Process 			(void);
int     ProcessTxsd 		(long);
void    RunProgram 			(char *, char *);
int     TeleSalesCustomer 	(long, int);
int     heading 			(int);
int     spec_valid 			(int);
void    CloseDB 			(void);
void    OpenDB 				(void);
void    SrchExsf 			(char *);
void    SrchTmop 			(char *);
void    shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc, 
 char*  argv [])
{

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	if (argc != 3 && argc != 9)
	{
		print_at (0, 0, ML (mlTsMess700), argv [0]);
		print_at (1, 0, ML (mlTsMess701), argv [0]);
		print_at (2, 0, ML (mlTsMess702));
		print_at (3, 0, ML (mlTsMess703));
		print_at (4, 0, ML (mlTsMess704));
		print_at (5, 0, ML (mlTsMess705));
		print_at (6, 0, ML (mlTsMess706));
		print_at (7, 0, ML (mlTsMess707));
		print_at (8, 0, ML (mlTsMess708));
		return (EXIT_FAILURE);
	}

	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB (); 	
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 	
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	if (argc == 9)
	{
		local_rec.printerNumber = atoi (argv [1]);
		sprintf (local_rec.startOperator,	"%-14.14s", argv [2]);
		sprintf (local_rec.endOperator,   	"%-14.14s", argv [3]);
		sprintf (local_rec.startCust,  		"%-6.6s",   argv [4]);
		sprintf (local_rec.endCust, 		"%-6.6s",   argv [5]);
		sprintf (local_rec.startSman,   	"%2.2s",    argv [6]);
		sprintf (local_rec.endSman,  		"%2.2s",    argv [7]);
		sprintf (local_rec.noteType, 		"%-1.1s",   argv [8]);

		firstTime = TRUE;
		Process ();
	
		if (!firstTime)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	set_masks ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();
	sprintf (local_rec.noteType, "%-1.1s", argv [2]);

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
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
		if (restart)
			continue;

		RunProgram (argv [0], argv [1]);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}


/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envVarDbFind) ? "cumr_id_no3" 
							    : "cumr_id_no");
	open_rec (tspm, tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
	open_rec (tsxd, tsxd_list, TSXD_NO_FIELDS, "tsxd_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmop);
	abc_fclose (exsf);
	abc_fclose (cumr);
	abc_fclose (tspm);
	abc_fclose (tsxd);
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("startOperator"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.startOperator, "%-14.14s", " ");
			strcpy (local_rec.startOperatorName, ML ("First Operator"));
			DSP_FLD ("startOperatorName");
	
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.startOperator);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.startOperatorName, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("startOperatorName");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("endOperator"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.endOperator, "~~~~~~~~~~~~~~"))
		{
			strcpy (local_rec.endOperator, "~~~~~~~~~~~~~~");
			strcpy (local_rec.endOperatorName, ML ("Last Operator"));
			DSP_FLD ("endOperatorName");
	
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.endOperator);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.endOperatorName, "%-40.40s", tmop_rec.op_name);
		DSP_FLD ("endOperatorName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCust"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.startCust, "%-6.6s", " ");
			strcpy (local_rec.startCustName, ML ("First Customer"));
			DSP_FLD ("startCustName");
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no, "%-6.6s", zero_pad (local_rec.startCust,6));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!TeleSalesCustomer (cumr_rec.hhcu_hash, TRUE))
			return (EXIT_FAILURE);

		sprintf (local_rec.startCustName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("startCustName");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCust"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.endCust, "~~~~~~"))
		{
			strcpy (local_rec.endCust, "~~~~~~");
			strcpy (local_rec.endCustName, ML ("Last Customer"));
			DSP_FLD ("endCustName");
	
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no, "%-6.6s", zero_pad (local_rec.endCust, 6));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!TeleSalesCustomer (cumr_rec.hhcu_hash, TRUE))
			return (EXIT_FAILURE);

		sprintf (local_rec.endCustName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("endCustName");
		
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("startSman"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startSman, "  ");
			strcpy (local_rec.startSmanName, ML ("First Salesman"));
			DSP_FLD ("startSmanName");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", local_rec.startSman);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startSmanName, "%-40.40s", exsf_rec.salesman);
		DSP_FLD ("startSmanName");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endSman"))
	{
		if (dflt_used || !strcmp (local_rec.endSman, "~~"))
		{
			strcpy (local_rec.endSman, "~~");
			strcpy (local_rec.endSmanName, ML ("Last Salesman"));
			DSP_FLD ("endSmanName");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", local_rec.endSman);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess135));	
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endSmanName, "%-40.40s", exsf_rec.salesman);
		DSP_FLD ("endSmanName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.backDesc, ML ("Yes"));
		else
			strcpy (local_rec.backDesc, ML ("No "));
	
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		if (local_rec.onight [0] == 'Y')
			strcpy (local_rec.onightDesc, ML ("Yes"));
		else
			strcpy (local_rec.onightDesc, ML ("No "));
	
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------------------
| Check that debtor is active for telesales |
-------------------------------------------*/
int
TeleSalesCustomer (
	long   hhcuHash, 
	int    displayMessage)
{
	tspm_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
	if (cc)
	{
		if (displayMessage)
		{
			print_mess (ML (mlTsMess002));
			sleep (sleepTime);
			clear_mess ();
		}
		return (FALSE);
	}
	return (TRUE);
}

void
SrchTmop (
 char*  key_val)
{
	_work_open (14,0,40);
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	save_rec ("#Operator I.D.", "#Operator Full Name.");
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
		      !strncmp (tmop_rec.op_id, key_val, strlen (key_val)))
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
		file_err (cc, "tmop", "DBFIND");
}

void
SrchExsf (
 char*  key_val)
{
	_work_open (2,0,40);
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%2.2s", key_val);
	save_rec ("#Sm", "#Salesman Name.");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no) &&
		      !strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
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
RunProgram (
	char*  programName, 
	char*  programDesc)
{
	sprintf (local_rec.printerString, "%d", local_rec.printerNumber);
	
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				"ONIGHT", 
				"ONIGHT", 
				programName, 
				local_rec.printerString, 
				local_rec.startOperator, 
				local_rec.endOperator, 
				local_rec.startCust, 
				local_rec.endCust, 
				local_rec.startSman, 
				local_rec.endSman, 
				local_rec.noteType, 
				programDesc, (char *)0
			);
		}
	}
    else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				programName, 
				programName, 
				local_rec.printerString, 
				local_rec.startOperator, 
				local_rec.endOperator, 
				local_rec.startCust, 
				local_rec.endCust, 
				local_rec.startSman, 
				local_rec.endSman, 
				local_rec.noteType, (char *)0
			);
		}
	}
	else 
	{
		execlp 
		(
			programName, 
			programName, 
			local_rec.printerString, 
			local_rec.startOperator, 
			local_rec.endOperator, 
			local_rec.startCust, 
			local_rec.endCust, 
			local_rec.startSman, 
			local_rec.endSman, 
			local_rec.noteType, (char *)0
		);
	}
}

int
Process (void)
{
	abc_selfield (cumr, "cumr_id_no3");

	dsp_screen ("Processing", comm_rec.co_no, comm_rec.co_name);

	if (!strncmp (local_rec.endCust, "~~~~~~", 6))
		memset ((char *)local_rec.endCust,0xff,sizeof (local_rec.endCust));

	if (!strncmp (local_rec.endSman, "~~", 2))
		memset ((char *)local_rec.endSman,0xff,sizeof (local_rec.endSman));

	if (!strncmp (local_rec.endOperator, "~~~~~~~~~~", 10))
		memset ((char *)local_rec.endOperator,0xff,sizeof (local_rec.endOperator));
	
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	sprintf (cumr_rec.dbt_no, "%-6.6s", " ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*----------------------
		| Check customer range |
		----------------------*/
		if (strcmp (cumr_rec.dbt_no, local_rec.startCust) < 0 ||
		    strcmp (cumr_rec.dbt_no, local_rec.endCust) > 0)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
/*
		dsp_process ("Customer", cumr_rec.dbt_no);
*/

		/*-----------------
		| Check rep range |
		-----------------*/
		if (strcmp (cumr_rec.sman_code, local_rec.startSman) < 0 ||
		    strcmp (cumr_rec.sman_code, local_rec.endSman) > 0)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		/*------------------------
		| Check that customer is |
		| active for telesales.  |
		------------------------*/
		if (!TeleSalesCustomer (cumr_rec.hhcu_hash, FALSE))
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		/*----------------------
		| Check operator range |
		----------------------*/
		if (strcmp (tspm_rec.op_code, local_rec.startOperator) < 0 ||
		    strcmp (tspm_rec.op_code, local_rec.endOperator) > 0)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		/*--------------------
		| Process complaints |
		--------------------*/
		ProcessTxsd (cumr_rec.hhcu_hash);

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*--------------------
| Process complaints |
--------------------*/
int
ProcessTxsd (
	long   hhcuHash)
{
	int		lineToPrint;
	int		firstLine;
	char	salesmanName [41];

	sprintf (printLine, "%-*.*s", LINE_LEN, LINE_LEN, " ");
	sprintf (leftOver,  "%-*.*s", LINE_LEN, LINE_LEN, " ");

	firstLine = TRUE;
	tsxd_rec.hhcu_hash = hhcuHash;
	sprintf (tsxd_rec.type, "%-1.1s", local_rec.noteType);
	tsxd_rec.line_no = 0;
	cc = find_rec (tsxd, &tsxd_rec, GTEQ, "r");
	while (!cc &&
	       tsxd_rec.hhcu_hash == hhcuHash &&
	       !strncmp (tsxd_rec.type, local_rec.noteType, 1))
	{
		if (firstTime)
			HeadingOutput ();

		/*------------------------
		| Print customer details |
		------------------------*/
		if (firstLine)
		{
			/*----------------------------
			| Lookup op and sman details |
			----------------------------*/
			strcpy (exsf_rec.co_no, comm_rec.co_no);
			sprintf (exsf_rec.salesman_no, "%2.2s", cumr_rec.sman_code);
			cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
			strcpy (salesmanName,(cc) ? " " : exsf_rec.salesman);

			strcpy (tmop_rec.co_no, comm_rec.co_no);
			sprintf (tmop_rec.op_id, "%-14.14s", tspm_rec.op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (!cc)
				strcpy (salesmanName, tmop_rec.op_name);

			if (!firstTime)
				fprintf (fout, ".PA\n");

			fprintf 
			(
				fout, 
		 		"! Customer: %-6.6s %-26.26s   Operator: %-14.14s %-20.20s   Sales Rep: %2.2s %-20.20s !\n", 
		 		cumr_rec.dbt_no, 
				cumr_rec.dbt_name, 
		 		tspm_rec.op_code, 
				tmop_rec.op_name,
		 		cumr_rec.sman_code, 
				salesmanName
			);
			fprintf (fout, "! %*.*s !\n", LINE_LEN, LINE_LEN, " ");

			firstLine = FALSE;
		}

		firstTime = FALSE;

		lineToPrint = AppendComm (tsxd_rec.desc, leftOver);
		if (!lineToPrint)
		{
			cc = find_rec (tsxd, &tsxd_rec, NEXT, "r");
			continue;
		}

		fprintf (fout, "! %-*.*s !\n", LINE_LEN, LINE_LEN, printLine);

		sprintf (printLine, "%-*.*s", LINE_LEN, LINE_LEN, " ");

		cc = find_rec (tsxd, &tsxd_rec, NEXT, "r");
	}

	if (strlen (clip (leftOver)) > 0)
	{
		fprintf (fout, "! %-*.*s !\n", 	LINE_LEN, LINE_LEN, leftOver);
		sprintf (printLine, "%-*.*s", 	LINE_LEN, LINE_LEN, " ");
		sprintf (leftOver, "%-*.*s", 	LINE_LEN, LINE_LEN, " ");
	}
	fflush (fout);

	return (EXIT_SUCCESS);
}

int
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".6\n");
	fprintf (fout, ".L132\n");

	switch (local_rec.noteType [0])
	{
	case 'C':
		fprintf (fout, ".ECustomer Complaints\n");
		break;
	
	case 'L':
		fprintf (fout, ".ECustomer Last Call Notes\n");
		break;
	
	case 'N':
		fprintf (fout, ".ECustomer Call Notes\n");
		break;
	
	case 'V':
		fprintf (fout, ".ECustomer Next Visit Notes\n");
		break;
	}
	
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECOMPANY : %s - %s\n", comm_rec.co_no, comm_rec.co_name);

	fprintf (fout, "====================================");
	fprintf (fout, "====================================");
	fprintf (fout, "====================================");
	fprintf (fout, "============================\n");

	fprintf (fout, ".R===================================");
	fprintf (fout, "=====================================");
	fprintf (fout, "=====================================");
	fprintf (fout, "===========================\n");

	return (EXIT_SUCCESS);
}

int
heading (
 int    scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	box (0, 3, 80, 12);
	line_at (6,1,79);
	line_at (9,1,79);
	line_at (12,1,79);

	switch (local_rec.noteType [0])
	{
	case 'C':
		rv_pr (ML (mlTsMess003), 25, 0, 1);
		break;
		
	case 'L':
		rv_pr (ML (mlTsMess004), 25, 0, 1);
		break;
		
	case 'N':
		rv_pr (ML (mlTsMess005), 25, 0, 1);
		break;
		
	case 'V':
		rv_pr (ML (mlTsMess006), 25, 0, 1);
		break;
	}
		
	line_at (1,0,80);
	line_at (20,0,80);

	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

/*----------------------------------
| Build up a full line of comments |
| Return TRUE if line is full.     |
----------------------------------*/
int
AppendComm (
 char*  comment, 
 char*  leftOver)
{
	int		line_len;
	char	*sptr;
	char	*tptr;
	char	tmp_store [116 * 2 + 1];
	
	clip (comment);
	clip (leftOver);
	if (strlen (leftOver) > 0 && strlen (leftOver) <= LINE_LEN)
	{
		strcpy (tmp_store, leftOver);
		strcat (tmp_store, " ");
	}

	strcat (tmp_store, comment);
	line_len = strlen (tmp_store);

	if (line_len <= LINE_LEN)
	{
		sprintf (leftOver, "%-*.*s", LINE_LEN, LINE_LEN, tmp_store);
		return (FALSE);
	}

	tptr = tmp_store;
	sptr = (tptr + line_len - 1);
	while (line_len > LINE_LEN)
	{
		while (*sptr != ' ')
		{
			sptr--;
			line_len--;
		}

		if (line_len > LINE_LEN)
		{
			sptr--;
			line_len--;
		}
	}

	if (*sptr != ' ')
		sptr++;
	*sptr = '\0';

	sprintf (printLine, "%-*.*s", LINE_LEN, LINE_LEN, tptr);

	sprintf (leftOver, "%-*.*s", LINE_LEN, LINE_LEN, sptr + 1);

	return (TRUE);
}

