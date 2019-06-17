/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_cnt_set.c,v 5.3 2002/01/11 02:07:29 scott Exp $
|  Program Name  : (cm_cnt_set.c  )                                 |
|  Program Desc  : (Change status/ purge contracts.             )   |	
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : (17/03/93)       |
|---------------------------------------------------------------------|
| $Log: cm_cnt_set.c,v $
| Revision 5.3  2002/01/11 02:07:29  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_cnt_set.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_cnt_set/cm_cnt_set.c,v 5.3 2002/01/11 02:07:29 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<hot_keys.h>
#include 	<proc_sobg.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>
#include	<tabdisp.h>

#define	COMPANY		 2
#define	BRANCH		 1
#define	MANUAL		 0

#define	O_to_B		0
#define	X_to_O		1
#define	C_to_H		2

#define	PROC_CNT	(local_rec.rangeType [0] == 'C')
#define	PROC_JOB	(local_rec.rangeType [0] == 'J')
#define	PROC_ISS	(local_rec.rangeType [0] == 'I')

#include	"schema"

struct commRecord	comm_rec;
struct cmcbRecord	cmcb_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmitRecord	cmit_rec;
struct cmjtRecord	cmjt_rec;
struct cmpbRecord	cmpb_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct cmtrRecord	cmtr_rec;
struct cmtsRecord	cmts_rec;
struct cumrRecord	cumr_rec;

	char	*data  = "data",
	    	*cmhr2 = "cmhr2",
	    	*cmit2 = "cmit2",
	    	*cmjt2 = "cmjt2";

	char	branchNo [3];

	int		noInTable	= 0,
			optionType	= 0;

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char	systemDate 			[11];
	char	rangeType 			[2];
	char	rangeDesc 			[9];
	char	startContract 		[7];
	char	startContractDesc 	[71];
	char	endContract 		[7];
	char	endContractDesc 	[71];
	char	startJobType 		[5];
	char	startJobTypeDesc 	[31];
	char	endJobType 			[5];
	char	endJobTypeDesc 		[31];
	char	startIssue 			[11];
	char	startIssueDesc 		[41];
	char	endIssue 			[11];
	char	endIssueDesc 		[41];
	char	dummy 				[11];
	long	lsystemDate;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "rangeType", 	4, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Range Type       ", "Enter Range type. C(ontract), J(ob Type) or I(ssue To). ",
		 NE, NO,  JUSTLEFT, "CJI", "", local_rec.rangeType},
	{1, LIN, "rangeDesc", 	4, 35, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangeDesc},

	{1, LIN, "startContract", 	6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Contract   ", "Enter Start Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.startContract},
	{1, LIN, "startContractDesc", 	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startContractDesc},
	{1, LIN, "endContract", 	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End  Contract    ", "Enter End Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endContract},
	{1, LIN, "endContractDesc", 	7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endContractDesc},

	{1, LIN, "startJobType", 	6, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Start Job Type   ", "Enter Start Job Type . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startJobType},
	{1, LIN, "startJobTypeDesc",6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startJobTypeDesc},
	{1, LIN, "endJobType", 	7, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "End  Contract    ", "Enter End Contract Number . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endJobType},
	{1, LIN, "endJobTypeDesc",7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endJobTypeDesc},

	{1, LIN, "startIssue", 	6, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Start Issue To   ", "Enter Start Issue To Code . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startIssue},
	{1, LIN, "startIssueDesc",	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startIssueDesc},
	{1, LIN, "endIssue", 	7, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "End Issue To     ", "Enter End Issue To Code . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endIssue},
	{1, LIN, "endIssueDesc",7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endIssueDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

static	int	ToggleFunc 		(int, KEY_TAB *);
static	int	DeleteFunc 		(int, KEY_TAB *);
static	int	ShowFunc	 	(int, KEY_TAB *);
static	int	ExitFunc	 	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB list_keys [] =
{
   { " TOGGLE CONTRACT STATUS ",		'T',		ToggleFunc,
	"Toggle status for contract.",					"A" },
   { " TAG ALL CONTRACTS ",		CTRL ('A'), 	ToggleFunc,
	"Toggle status for all contracts.",				"A" },
   { " DELETE CONTRACT ",		'D', 		DeleteFunc,
	"Delete contract.", 						"D" },
   { " SHOW CONTRACT DETAILS ",		'S', 		ShowFunc,
	"Delete contract.", 						"A" },
   { NULL,			FN1, 		ExitFunc,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		ExitFunc,
	"Exit and update the database.",				"A" },
   END_KEYS
};
#else
static	KEY_TAB list_keys [] =
{
   { " [T]OGGLE STATUS",		'T',		ToggleFunc,
	"Toggle status for contract.",					"A" },
   { " [^A]LL TAG",		CTRL ('A'), 	ToggleFunc,
	"Toggle status for all contracts.",				"A" },
   { " [D]ELETE",		'D', 		DeleteFunc,
	"Delete contract.", 						"D" },
   { " [S]HOW DETAILS",		'S', 		ShowFunc,
	"Delete contract.", 						"A" },
   { NULL,			FN1, 		ExitFunc,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		ExitFunc,
	"Exit and update the database.",				"A" },
   END_KEYS
};
#endif

/*
 * Main Processing Routine.
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
void	SrchCmhr		(char *);
void	SrchCmit		(char *);
void	SrchCmjt		(char *);
void	DeleteContract	(long);
int		spec_valid		(int);
int		Process			(void);
int		Update			(void);
int		toggle_line		(int);
int		heading			(int);
int		CheckReqs		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	int		auto_con_no;
	char	*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*
	 * Option depends on program name.
	 */
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	if (!strcmp (sptr, "cm_close"))
	{
		optionType = O_to_B;
		set_keys (list_keys, "D", KEY_PASSIVE);
	}
	else
	{
		if (!strcmp (sptr, "cm_cnt_rel"))
			optionType = X_to_O;
		else
			optionType = C_to_H;
	}

	/*
	 * Check contract number level.
	 */
	sptr = chk_env ("CM_AUTO_CON");
	auto_con_no = (sptr == (char *)0) ? COMPANY : atoi (sptr);
	strcpy (branchNo, (auto_con_no == COMPANY) ? " 0" : comm_rec.est_no);

	OpenDB ();

	/*
	 * setup required parameters.
	 */
	init_scr ();
	set_tty ();
	set_masks ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	prog_exit = FALSE;
	while (prog_exit == FALSE)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		init_ok 	= TRUE;
		eoi_ok 		= FALSE;
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

		scn_write (1);
		Process ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
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

	abc_alias (cmhr2, cmhr);
	abc_alias (cmit2, cmit);
	abc_alias (cmjt2, cmjt);

	open_rec (cmcb,  cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmit,  cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmit2, cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmjt,  cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmpb,  cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cmrd,  cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (cmrh,  cmrh_list, CMRH_NO_FIELDS, "cmrh_hhhr_hash");
	open_rec (cmtr,  cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec (cmts,  cmts_list, CMTS_NO_FIELDS, "cmts_hhhr_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmit);
	abc_fclose (cmit2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cmpb);
	abc_fclose (cmrd);
	abc_fclose (cmrh);
	abc_fclose (cmtr);
	abc_fclose (cmts);
	abc_fclose (cumr);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	int	i;

	if (LCHECK ("rangeType"))
	{
		/*
		 * Disable range prompts.
		 */
		for (i = label ("startContract"); i <= label ("endIssue"); i++)
			vars [i].required = ND;

		switch (local_rec.rangeType [0])
		{
		case 'C':
			FLD ("startContract") 		= YES;
			FLD ("startContractDesc") 	= NA;
			FLD ("endContract") 		= YES;
			FLD ("endContractDesc") 	= NA;
			strcpy (local_rec.rangeDesc, ML ("Contract"));
			break;

		case 'J':
			FLD ("startJobType") 		= YES;
			FLD ("startJobTypeDesc") 	= NA;
			FLD ("endJobType") 			= YES;
			FLD ("endJobTypeDesc") 		= NA;
			strcpy (local_rec.rangeDesc, ML ("Job Type"));
			break;

		case 'I':
			FLD ("startIssue") 			= YES;
			FLD ("startIssueDesc") 		= NA;
			FLD ("endIssue") 			= YES;
			FLD ("endIssueDesc") 		= NA;
			strcpy (local_rec.rangeDesc, ML ("Issue To"));
			break;
		}
		scn_write (1);
		DSP_FLD ("rangeDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Contract Range.
	 */
	if (LCHECK ("startContract"))
	{
		if (FLD ("startContract") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startContract, "      ");
			strcpy (local_rec.startContractDesc, ML ("First Contract"));
			DSP_FLD ("startContract");
			DSP_FLD ("startContractDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num (local_rec.startContract);
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.startContract);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check against end contract no.
		 */
		if (prog_status != ENTRY && 
		    strcmp (local_rec.startContract, local_rec.endContract) > 0)
		{
			print_mess (ML (mlStdMess017));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckReqs ())
		{
			print_mess (ML (mlCmMess023));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Lookup first description line.
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			strcpy (cmcd_rec.text, ML ("Description Not Found."));
		
		sprintf (local_rec.startContractDesc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("startContractDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endContract"))
	{
		if (FLD ("endContract") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endContract, "~~~~~~");
			strcpy (local_rec.endContractDesc, ML ("Last Contract"));
			DSP_FLD ("endContract");
			DSP_FLD ("endContractDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num (local_rec.endContract);
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.endContract);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Check against start contract no.
		 */
		if (strcmp (local_rec.startContract, local_rec.endContract) > 0)
		{
			print_mess (ML (mlStdMess017));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckReqs ())
		{
			print_mess (ML (mlCmMess023));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Lookup first description line. 
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			strcpy (cmcd_rec.text, ML ("Description Not Found. "));
		
		sprintf (local_rec.endContractDesc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("endContractDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Job Type Range.
	 */
	if (LCHECK ("startJobType"))
	{
		if (FLD ("startJobType") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startJobType, "    ");
			strcpy (local_rec.startJobTypeDesc, ML ("First Job Type"));
			DSP_FLD ("startJobType");
			DSP_FLD ("startJobTypeDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		sprintf (cmjt_rec.job_type, "%-4.4s", local_rec.startJobType);
		cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess011));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startJobTypeDesc, "%-30.30s", cmjt_rec.desc);
		DSP_FLD ("startJobTypeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endJobType"))
	{
		if (FLD ("endJobType") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endJobType, "~~~~");
			strcpy (local_rec.endJobTypeDesc, ML ("Last Job Type"));
			DSP_FLD ("endJobType");
			DSP_FLD ("endJobTypeDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmjt (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		sprintf (cmjt_rec.job_type, "%-4.4s", local_rec.endJobType);
		cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess011));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endJobTypeDesc, "%-30.30s", cmjt_rec.desc);
		DSP_FLD ("endJobTypeDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Issue To Range.
	 */
	if (LCHECK ("startIssue"))
	{
		if (FLD ("startIssue") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startIssue, "          ");
			strcpy (local_rec.startIssueDesc, ML ("First Issue To Code"));
			DSP_FLD ("startIssue");
			DSP_FLD ("startIssueDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmit_rec.co_no, comm_rec.co_no);
		sprintf (cmit_rec.issto, "%-10.10s", local_rec.startIssue);
		cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess014));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startIssueDesc, "%-40.40s",cmit_rec.iss_name);
		DSP_FLD ("startIssueDesc");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endIssue"))
	{
		if (FLD ("endIssue") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endIssue, "~~~~~~~~~~");
			strcpy (local_rec.endIssueDesc, ML ("Last Issue To Code"));
			DSP_FLD ("endIssue");
			DSP_FLD ("endIssueDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmit (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmit_rec.co_no, comm_rec.co_no);
		sprintf (cmit_rec.issto, "%-10.10s", local_rec.endIssue);
		cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess014));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endIssueDesc,"%-40.40s",cmit_rec.iss_name);
		DSP_FLD ("endIssueDesc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCmhr (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#No.", "#Customer Order No.");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, key_val, strlen (key_val)))
	{
		/*
		 * Validate status for option.
		 */
		if (optionType == O_to_B &&
		    cmhr_rec.status [0] != 'O' && cmhr_rec.status [0] != 'B')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (optionType == X_to_O && cmhr_rec.status [0] != 'X')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (optionType == C_to_H &&
		    cmhr_rec.status [0] != 'C' && cmhr_rec.status [0] != 'H')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
SrchCmit (
 char *	key_val)
{
	_work_open (10,0,40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", key_val);
	cc = find_rec (cmit, &cmit_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit_rec.issto, key_val, strlen (key_val)))
	{
		cc = save_rec (cmit_rec.issto, cmit_rec.iss_name);
		if (cc)
			break;

		cc = find_rec (cmit, &cmit_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", temp_str);
	cc = find_rec (cmit, &cmit_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmit, "DBFIND");
}

void
SrchCmjt (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Type", "#Job Type Description");

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", key_val);
	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmjt_rec.job_type, key_val, strlen (key_val)))
	{
		cc = save_rec (cmjt_rec.job_type, cmjt_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);
	cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmjt, "DBFIND");
}

/*
 * Load valid contracts into tabdisp table and allow processing.    
 */
int
Process (void)
{
	/*
	 * Open table 
	 */
	tab_open ("cnt_lst", list_keys, 9, 6, 8, FALSE);
	tab_add ("cnt_lst", 
		"# %-11.11s   %-8.8s    %-10.10s   %-40.40s    %-14.14s      %-11.11s",
		"CONTRACT NO",
		"JOB TYPE",
		"ISSUE TO",
		"CUSTOMER NAME",
		"CURRENT STATUS",
		"NEW STATUS");
	noInTable = 0;

	/*
	 * Read contract records.
	 */
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	if (PROC_CNT)
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.startContract);
	else
		sprintf (cmhr_rec.cont_no, "%-6.6s", " ");
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo))
	{
		/*
		 * Check range for PROC_CNT.
		 */
		if (PROC_CNT && 
		    strcmp (cmhr_rec.cont_no, local_rec.endContract) > 0)
		{	
			break;
		}

		/*
		 * Lookup job type code for contract.
		 */
		cc = find_hash (cmjt2, &cmjt_rec, COMPARISON, "r", cmhr_rec.hhjt_hash);
		if (cc)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Valid for job type range. 
		 */
		if (PROC_JOB && 
		   (strcmp (cmjt_rec.job_type, local_rec.startJobType) < 0 ||
		     strcmp (cmjt_rec.job_type, local_rec.endJobType) > 0))
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup issue to code for contract. 
		 */
		if (cmhr_rec.hhit_hash == 0L)
			strcpy (cmit_rec.issto, "          ");
		else
		{
			cc = find_hash (cmit2, &cmit_rec, COMPARISON, "r", cmhr_rec.hhit_hash);
			if (cc)
			{
				cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
				continue;
			}
		}
		/*
		 * Valid for Issue To range.
		 */
		if (PROC_ISS && 
		   (strcmp (cmit_rec.issto, local_rec.startIssue) < 0 ||
		     strcmp (cmit_rec.issto, local_rec.endIssue) > 0))
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check that contract has valid status for the option being run. 
		 */
		if (optionType == O_to_B && 
		    cmhr_rec.status [0] != 'B' &&
		    cmhr_rec.status [0] != 'O')
		{	
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (optionType == X_to_O && 
		    cmhr_rec.status [0] != 'X')
		{	
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		if (optionType == C_to_H && 
		    cmhr_rec.status [0] != 'C' &&
		    cmhr_rec.status [0] != 'H')
		{	
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check outstanding reqs   
		 */
		if (CheckReqs ())
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup customer for contract. 
		 */
		cumr_rec.hhcu_hash	=	cmhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Add to table.
		 */
		tab_add ("cnt_lst", 
			"  %-6.6s        %-4.4s        %-10.10s   %-40.40s          %-1.1s               %-1.1s          %010ld",
			cmhr_rec.cont_no,
			cmjt_rec.job_type,
			cmit_rec.issto,
			cumr_rec.dbt_name,
			cmhr_rec.status,
			cmhr_rec.status,
			cmhr_rec.hhhr_hash);
		noInTable++;

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	if (noInTable == 0)
	{
		tab_add ("cnt_lst", "There are no valid contracts in the selected range. ");
		tab_display ("cnt_lst", TRUE);
		sleep (sleepTime);
		tab_close ("cnt_lst", TRUE);
		return (EXIT_SUCCESS);
	}
	else
	{
		tab_scan ("cnt_lst");
	}
	
	if (!restart)
		Update ();
	tab_close ("cnt_lst", TRUE);

	return (EXIT_SUCCESS);
}

int
Update (void)
{
	int	i;
	long	hhhrHash;
	char	get_buf [200];
	char	old_stat [2];
	char	new_stat [2];

	for (i = 0; i < noInTable; i++)
	{
		tab_get ("cnt_lst", get_buf, EQUAL, i);
		sprintf (old_stat, "%-1.1s", get_buf + 91);
		sprintf (new_stat, "%-1.1s", get_buf + 107);
		
		if (old_stat [0] == new_stat [0])
			continue;

		/*
		 * Find cmhr record.
		 */
		hhhrHash = atol (get_buf + 120);
		cmhr_rec.hhhr_hash	=	hhhrHash;
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "u");
		if (cc)
			continue;

		/*
		 * Delete contract.
		 */
		if (new_stat [0] == 'D')
		{
			if (optionType == X_to_O || optionType == C_to_H)
			{
				DeleteContract (hhhrHash);
				continue;
			}
		}

		/*
		 * Update contract. 
		 */
		strcpy (cmhr_rec.status, new_stat);
		cc = abc_update (cmhr2, &cmhr_rec);
		if (cc)
			file_err (cc, cmhr, "DBUPDATE");
	}

	recalc_sobg ();

	return (EXIT_SUCCESS);
}

void
DeleteContract (
	long	hhhrHash)
{
	/*
	 * Deleting a "Held" Contract.
	 */
	if (optionType == X_to_O)
	{
		/*
		 * Delete cmcd records. 
		 */
		cmcd_rec.hhhr_hash = hhhrHash;
		strcpy (cmcd_rec.stat_flag, " ");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
		while (!cc && cmcd_rec.hhhr_hash == hhhrHash)
		{
			cc = abc_delete (cmcd);
			if (cc)
				file_err (cc, cmcd, "DBDELETE");

			cmcd_rec.hhhr_hash = hhhrHash;
			strcpy (cmcd_rec.stat_flag, " ");
			cmcd_rec.line_no = 0;
			cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
		}
		/*
		 * Delete cmcb records. 
		 */
		cmcb_rec.hhhr_hash = hhhrHash;
		cmcb_rec.hhcm_hash = 0L;
		cc = find_rec (cmcb, &cmcd_rec, GTEQ, "u");
		while (!cc && cmcb_rec.hhhr_hash == hhhrHash)
		{
			cc = abc_delete (cmcb);
			if (cc)
				file_err (cc, cmcb, "DBDELETE");

			cmcb_rec.hhhr_hash = hhhrHash;
			cmcb_rec.hhcm_hash = 0L;
			cc = find_rec (cmcb, &cmcb_rec, GTEQ, "u");
		}
		/*
		 * Delete cmhr records. 
		 */
		cc = abc_delete (cmhr2);
		if (cc)
			file_err (cc, cmhr, "DBDELETE");

	}
	else
	/*
	 * Purging a "History" Contract 
	 */
	{
	    /*
	     * Delete Any Requisitions against the contract.  
	     * Uncommit any stock still commited (not issued).
	     */
	    cc = find_hash (cmrh, &cmrh_rec, GTEQ, "u", hhhrHash);
	    while (!cc && cmrh_rec.hhhr_hash == hhhrHash)
	    {
			cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
			cmrd_rec.line_no = 0;
			cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
			while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
			{
				if ((cmrd_rec.qty_order + cmrd_rec.qty_border) > 0.00)
				{
					add_hash 
					(
						comm_rec.co_no,
						comm_rec.est_no,
						"RC", 
						0,
						cmrd_rec.hhbr_hash, 
						0L,
						0L,
						(double) 0.00
					);
				}

				/*
				 * Delete detail record. 
				 */
				cc = abc_delete (cmrd);
				if (cc)
					file_err (cc, cmrd, "DBDELETE");

				cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
				cmrd_rec.line_no = 0;
				cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
			}

			/*
			 * Delete header record. 
			 */
			cc = abc_delete (cmrh);
			if (cc)
				file_err (cc, cmrh, "DBDELETE");

			cc = find_hash (cmrh, &cmrh_rec, GTEQ, "u", hhhrHash);
	    }

	    /*
	     * Delete cmcd records.
	     */
	    cmcd_rec.hhhr_hash = hhhrHash;
	    strcpy (cmcd_rec.stat_flag, "D");
	    cmcd_rec.line_no = 0;
	    cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
	    while (!cc &&
	           cmcd_rec.hhhr_hash == hhhrHash &&
	           !strcmp (cmcd_rec.stat_flag, "D"))
	    {
			/*
			 * Delete description record. 
			 */
			cc = abc_delete (cmcd);
			if (cc)
			   file_err (cc, cmcd, "DBDELETE");

			cmcd_rec.hhhr_hash = hhhrHash;
			strcpy (cmcd_rec.stat_flag, "D");
			cmcd_rec.line_no = 0;
			cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
		}

	    /*
	     * Delete cmcb records.
	     */
	    cmcb_rec.hhhr_hash = hhhrHash;
	    cmcb_rec.hhcm_hash = 0L;
	    cc = find_rec (cmcb, &cmcb_rec, GTEQ, "u");
	    while (!cc && cmcb_rec.hhhr_hash == hhhrHash)
	    {
			/*
			 * Delete budget record. 
			 */
			cc = abc_delete (cmcb);
			if (cc)
			   file_err (cc, cmcb, "DBDELETE");

			cmcb_rec.hhhr_hash = hhhrHash;
			cmcb_rec.hhcm_hash = 0L;
			cc = find_rec (cmcb, &cmcb_rec, GTEQ, "u");
	    }

	    /*
	     * Delete cmts record. 
	     */
	    cc = find_hash (cmts, &cmts_rec, GTEQ, "u", hhhrHash);
	    while (!cc && cmts_rec.hhhr_hash == hhhrHash)
	    {
			/*
			 * Delete timesheet record. 
			 */
			cc = abc_delete (cmts);
			if (cc)
			   file_err (cc, cmts, "DBDELETE");

			cc = find_hash (cmts, &cmts_rec, GTEQ, "u", hhhrHash);
	    }

	    /*
	     * Delete cmtr record. 
	     */
	    cc = find_hash (cmtr, &cmtr_rec, GTEQ, "u", hhhrHash);
	    while (!cc && cmtr_rec.hhhr_hash == hhhrHash)
	    {
			/*
			 * Delete transaction record.
			 */
			cc = abc_delete (cmtr);
			if (cc)
			   file_err (cc, cmtr, "DBDELETE");

			cc = find_hash (cmtr, &cmtr_rec, GTEQ, "u", hhhrHash);
	    }

	    /*
	     * Delete cmpb record. 
	     */
	    cc = find_hash (cmpb, &cmpb_rec, GTEQ, "u", hhhrHash);
	    while (!cc && cmpb_rec.hhhr_hash == hhhrHash)
	    {
			/*
			 * Delete progress billing record. 
			 */
			cc = abc_delete (cmpb);
			if (cc)
			   file_err (cc, cmpb, "DBDELETE");

			cc = find_hash (cmpb, &cmpb_rec, GTEQ, "u", hhhrHash);
	    }

	    /*
	     * Delete cmhr record. 
	     */
	    cc = abc_delete (cmhr2);
	    if (cc)
			file_err (cc, cmhr2, "DBDELETE");
	}
}

static int
ToggleFunc (
 int		c,
 KEY_TAB *	psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [200];

	st_line = tab_tline ("cnt_lst");

	if (c == 'T')
		toggle_line (st_line);
	else
	{
		for (i = st_line; i < noInTable; i++)
			toggle_line (i);

		tab_display ("cnt_lst", TRUE);
		tab_get ("cnt_lst", get_buf, EQUAL, st_line);
		redraw_keys ("cnt_lst");
	}


	return (c);
}

int
toggle_line (
 int	line_no)
{
	char	get_buf [200];
	char	curr_stat [2];
	char	new_stat [2];

	tab_get ("cnt_lst", get_buf, EQUAL, line_no);
	sprintf (curr_stat, "%-1.1s", get_buf + 107);
	switch (optionType)
	{
	case O_to_B:
		if (curr_stat [0] == 'B')
			strcpy (new_stat, "O");
		else
			strcpy (new_stat, "B");
		break;

	case X_to_O:
		if (curr_stat [0] == 'X')
			strcpy (new_stat, "O");
		else
			strcpy (new_stat, "X");
		break;

	case C_to_H:
		if (curr_stat [0] == 'C')
			strcpy (new_stat, "H");
		else
			strcpy (new_stat, "C");
		break;
	}

	tab_update ("cnt_lst",
		"%-107.107s%-1.1s          %010ld",
		get_buf,
		new_stat,
		atol (get_buf + 120));

	return (EXIT_SUCCESS);
}

static int
DeleteFunc (
 int		c,
 KEY_TAB *	psUnused)
{
	char	get_buf [200];
	char	curr_stat [2];
	char	new_stat [7];

	tab_get ("cnt_lst", get_buf, CURRENT, 0);
	sprintf (curr_stat, "%-1.1s", get_buf + 91);
	switch (optionType)
	{
	case O_to_B:
		putchar (BELL);
		fflush (stdout);
		return (EXIT_SUCCESS);

	case X_to_O:
		if (curr_stat [0] == 'X')
			strcpy (new_stat, "DELETE");
		else
		{
			/*You may only delete status X contracts.*/
			print_mess (ML (mlCmMess066));		
			sleep (sleepTime);
			clear_mess ();
			strcpy (new_stat, "O     ");
		}
		break;

	case C_to_H:
		if (curr_stat [0] == 'H')
			strcpy (new_stat, "DELETE");
		else
		{
			/*You may only delete status X contracts.*/
			print_mess (ML (mlCmMess066));		
			sleep (sleepTime);
			clear_mess ();
			strcpy (new_stat, "C     ");
		}
		break;
	}

	tab_update ("cnt_lst",
		"%-107.107s%-6.6s     %010ld",
		get_buf,
		new_stat,
		atol (get_buf + 120));

	return (c);
}

static int
ShowFunc (
 int		c,
 KEY_TAB *	psUnused)
{
	int	i;
	int	curr_line;
	long	hhhr_hash;
	char	get_buf [200];
	char	cont_desc [7][71];

	curr_line = tab_tline ("cnt_lst");
	tab_get ("cnt_lst", get_buf, CURRENT, 0);
	hhhr_hash = atol (get_buf + 120);
	cc = find_hash (cmhr2, &cmhr_rec, COMPARISON, "r", hhhr_hash);
	if (cc)
		return (EXIT_SUCCESS);

	/*
	 * Look up details for contract. 
	 */
	cc = find_hash (cumr, &cumr_rec, COMPARISON, "r", cmhr_rec.hhcu_hash);
	if (cc)
		return (EXIT_SUCCESS);
	for (i = 0; i < 7; i++)
	{
		cmcd_rec.hhhr_hash = hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = i;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cont_desc [i], "%-70.70s", " ");
		else
			sprintf (cont_desc [i], "%-70.70s", cmcd_rec.text);
	}

	/*
	 * Draw box for display of contract details. 
	 */
	cl_box (0, 9, 132, 10);

	/*
	 * Display details. 
	 */

	print_at (10, 1, ML (mlStdMess069), cmhr_rec.cont_no);
	print_at (11, 1, ML (mlCmMess067),
		 cumr_rec.dbt_no,
		 cumr_rec.dbt_name,
		 cmhr_rec.cus_ref);
	print_at (12, 1,  ML (mlCmMess069), DateToString (cmhr_rec.due_date));
	print_at (12, 36,  ML (mlCmMess070), 
		(cmhr_rec.quote_type [0] == 'F') ? "Fixed   " : "Variable");
	print_at (12, 70,  ML (mlCmMess071), 
		 comma_fmt (DOLLARS (cmhr_rec.quote_val), "NNN,NNN,NNN.NN"));
	print_at (13, 1, ML (mlStdMess089), cont_desc [0]);
	print_at (14, 1, "                      : %-70.70s", cont_desc [1]);
	print_at (15, 1, "                      : %-70.70s", cont_desc [2]);
	print_at (16, 1, "                      : %-70.70s", cont_desc [3]);
	print_at (17, 1, "                      : %-70.70s", cont_desc [4]);
	print_at (18, 1, "                      : %-70.70s", cont_desc [5]);
	print_at (19, 1, "                      : %-70.70s", cont_desc [6]);

	/*
	 * Wait for FN12. 
	 */
	PauseForKey (20, 50,  ML (mlCmMess115), FN16);

	erase_box (0, 9, 132, 10);
	for (i = 10; i < 14; i++)
		print_at (i, 0, "      ");
	tab_display ("cnt_lst", TRUE);
	tab_get ("cnt_lst", get_buf, EQUAL, curr_line);
	redraw_keys ("cnt_lst");

	return (c);
}

static int
ExitFunc (
 int		c,
 KEY_TAB *	psUnused)
{
	if (c == FN1)
		restart = TRUE;

	return (c);
}

/*
 * Print Heading.
 */
int
heading (
 int	scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		swide ();
		line_at (1,0,132);
		rv_pr (ML (mlCmMess072), 48,0,1);

		box (0, 3, 132, 4);
		line_at (5,1,131);

		line_cnt = 0;
		scn_write (scn);
	}
	line_at (21,0,131);

	print_at (22,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}

int
CheckReqs (void)
{
	/*
	 * Check to make sure no reqs if O_to_B
	 */
	if (optionType == O_to_B)
	{
		cc = find_hash (cmrh, &cmrh_rec, GTEQ, "r", 
						cmhr_rec.hhhr_hash);
		while (!cc && 
			cmrh_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			if (cmrh_rec.stat_flag [0] != 'C')
				return (EXIT_FAILURE);
			 
			cc = find_hash (cmrh, &cmrh_rec, NEXT, "r", 
						cmhr_rec.hhhr_hash);
		}
	}

	return (EXIT_SUCCESS);
}
