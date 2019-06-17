/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_cont_due.c,v 5.2 2002/07/17 09:56:58 scott Exp $
|  Program Name  : (cm_cont_due.c)
|  Program Desc  : (Contract Due Report)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (05/04/93)       |
|---------------------------------------------------------------------|
| $Log: cm_cont_due.c,v $
| Revision 5.2  2002/07/17 09:56:58  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2002/01/21 08:58:01  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_cont_due.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_cont_due/cm_cont_due.c,v 5.2 2002/07/17 09:56:58 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<get_lpno.h>
#include 	<ml_cm_mess.h>
#include 	<ml_std_mess.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	PROC_CNT	(local_rec.rangeType [0] == 'C')
#define	PROC_JOB	(local_rec.rangeType [0] == 'J')
#define	PROC_ISS	(local_rec.rangeType [0] == 'I')
#define	PROC_WIP	(local_rec.rangeType [0] == 'W')
#define	SUMMARY		(local_rec.sumDet [0] == 'S')

#include	"schema"

struct commRecord	comm_rec;
struct cmcdRecord	cmcd_rec;
struct cmcmRecord	cmcm_rec;
struct cmwsRecord	cmws_rec;
struct cmemRecord	cmem_rec;
struct cmeqRecord	cmeq_rec;
struct cmhrRecord	cmhr_rec;
struct cmitRecord	cmit_rec;
struct cmjtRecord	cmjt_rec;
struct cmpbRecord	cmpb_rec;
struct cmtrRecord	cmtr_rec;
struct cmtsRecord	cmts_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	char	*cmcm2 = "cmcm2", 
			*cmhr2 = "cmhr2",
			*cmit2 = "cmit2", 
			*cmjt2 = "cmjt2", 
			*data  = "data";

	int		envCmAutoCon	= FALSE, 
			pipeOpen 		= FALSE;

	long	lastDate;

	FILE	*fout;

	char	branchNo [3], 
			*sptr;

	struct	{
		char	lastInvDate [11];
		char	lastInvAmount [11];
		char	invToDate [11];
		char	costToDate [11];
		char	amountCost [11];
		char	amountSale [11];
		char	quoteAmount [11];
		char	finishDate [11];
	} contRec;

	extern	int		TruePosition;
/*
 * Local & Screen Structures. 
 */
struct {
	char	back 			[2];
	char	backDesc 		[4];
	char	dummy 			[11];
	char	endContDesc 	[71];
	char	endContNo 		[7];
	char	endIssue 		[11];
	char	endIssueDesc 	[41];
	char	endJobDesc 		[31];
	char	endJobType 		[5];
	char	endWip 			[5];
	char	endWipDesc 		[41];
	char	internal 		[2];
	char	internalDesc 	[4];
	char	onight 			[2];
	char	onightDesc 		[4];
	char	rangeDesc 		[11];
	char	rangeType 		[2];
	char	startContDesc 	[71];
	char	startContNo 	[7];
	char	startIssue 		[11];
	char	startIssueDesc 	[41];
	char	startJobDesc 	[31];
	char	startJobType 	[5];
	char	startWip 		[5];
	char	startWipDesc 	[41];
	char	sumDet 			[2];
	char	sumDetDesc 		[8];
	char	systemDate 		[11];
	int		printerNo;
	long	lsystemDate;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "rangeType", 	4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Range Type         ", "Enter Range type. C(ontract), J(ob Type), W(IP Status) or I(ssue To). ", 
		 NE, NO, JUSTLEFT, "WCJI", "", local_rec.rangeType}, 
	{1, LIN, "rangeDesc", 	4, 35, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.rangeDesc}, 
	{1, LIN, "startContNo", 	6, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Contract     ", "Enter Start Contract Number.", 
		 YES, NO, JUSTLEFT, "", "", local_rec.startContNo}, 
	{1, LIN, "startContDesc", 	6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.startContDesc}, 
	{1, LIN, "endContNo", 	7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End  Contract      ", "Enter End Contract Number . ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.endContNo}, 
	{1, LIN, "endContDesc", 	7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.endContDesc}, 

	{1, LIN, "startWip", 	6, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Start WIP Status   ", "Enter Start WIP Status . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startWip}, 
	{1, LIN, "startWipDesc", 6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startWipDesc}, 
	{1, LIN, "endWip", 	7, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", " End WIP Status   :", "Enter End WIP Status . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endWip}, 
	{1, LIN, "endWipDesc", 7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endWipDesc}, 
	{1, LIN, "startJobType", 	6, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Start Job Type     ", "Enter Start Job Type . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startJobType}, 
	{1, LIN, "startJobDesc", 6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startJobDesc}, 
	{1, LIN, "endJobType", 	7, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "End Job Type       ", "Enter End Job Type . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endJobType}, 
	{1, LIN, "endJobDesc", 7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endJobDesc}, 
	{1, LIN, "startIssue", 	6, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Start Issue To     ", "Enter Start Issue To Code . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startIssue}, 
	{1, LIN, "startIssueDesc", 	6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.startIssueDesc}, 
	{1, LIN, "endIssue", 	7, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "End Issue To       ", "Enter End Issue To Code . ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endIssue}, 
	{1, LIN, "endIssueDesc", 7, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.endIssueDesc}, 
	{1, LIN, "internal", 	 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Internal           ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.internal}, 
	{1, LIN, "internalDesc", 9, 35, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.internalDesc}, 
	{1, LIN, "sumDet", 	 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Summary/Detail     ", " ", 
		YES, NO, JUSTLEFT, "SD", "", local_rec.sumDet}, 
	{1, LIN, "sumDetDesc", 10, 35, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.sumDetDesc}, 
	{1, LIN, "printerNo", 	 12, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No         ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo}, 
	{1, LIN, "back", 	 13, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background         ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 13, 35, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 	 14, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight          ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "onightDesc", 14, 35, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * Function declarations.    
 */
int		heading			(int);
int		spec_valid		(int);
void	CalcTodate		(void);
void	CloseDB		 	(void);
void	DescDetails		(void);
void	DescHead		(int);
void	HeadingOutput	(void);
void	MaterialDetails	(void);
void	MaterialHead	(int);
void	OpenDB			(void);
void	PrintContract	(void);
void	PrintDetails	(void);
void	Process			(void);
void	RunProgram		(char *, char *);
void	shutdown_prog	(void);
void	SrchCmhr		(char *);
void	SrchCmit		(char *);
void	SrchCmjt		(char *);
void	SrchCmws		(char *);
void	TimeDetails		(void);
void	TimeHead		(int);

/*
 * Main Processing Routine . 
 */
int
main (
 int	argc, 
 char *	argv [])
{
	TruePosition	=	TRUE;

	if (argc != 2 && argc != 7)
	{
		print_at (0, 0, ML (mlCmMess717), argv [0]);
		print_at (1, 0, ML (mlCmMess701), argv [0]);
		print_at (2, 0, ML (mlCmMess730));
		print_at (3, 0, ML (mlCmMess731));
		print_at (4, 0, ML (mlCmMess732));
		print_at (5, 0, ML (mlCmMess733));
		print_at (6, 0, ML (mlCmMess734));
		print_at (7, 0, ML (mlCmMess735));
		print_at (8, 0, ML (mlCmMess736));
		print_at (9, 0, ML (mlCmMess737));
		return (argc);
	}
	
	/*
	 * Check environment. 
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	OpenDB ();

	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	if (argc == 7)
	{
		local_rec.printerNo = atoi (argv [1]);
		sprintf (local_rec.sumDet, "%-1.1s", argv [2]);
		sprintf (local_rec.rangeType, "%-1.1s", argv [3]);
		sprintf (local_rec.internal, "%-1.1s", argv [4]);
	
		switch (local_rec.rangeType [0])
		{
		case 'C':
			sprintf (local_rec.startContNo, "%-6.6s", argv [5]);
			sprintf (local_rec.endContNo, "%-6.6s", argv [6]);
			break;

		case 'J':
			sprintf (local_rec.startJobType, "%-4.4s", argv [5]);
			sprintf (local_rec.endJobType, "%-4.4s", argv [6]);
			break;

		case 'W':
			sprintf (local_rec.startWip, "%-4.4s", argv [5]);
			sprintf (local_rec.endWip, "%-4.4s", argv [6]);
			break;

		case 'I':
			sprintf (local_rec.startIssue, "%-10.10s", argv [5]);
			sprintf (local_rec.endIssue, "%-10.10s", argv [6]);
			break;
		}

		Process ();

		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Prepare screen. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	while (!prog_exit)
	{
		/*
		 * Reset control flags 
		 */
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		restart 	= FALSE;
		search_ok 	= TRUE;

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
		prog_exit = 1;
	}
	shutdown_prog ();   
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cmcm2, cmcm);
	abc_alias (cmhr2, cmhr);
	abc_alias (cmit2, cmit);
	abc_alias (cmjt2, cmjt);

	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmws, cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmem, cmem_list, CMEM_NO_FIELDS, "cmem_hhem_hash");
	open_rec (cmeq, cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmit2, cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_id_no");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_id_no2");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_id_no3");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close data base files  
 */
void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmws);
	abc_fclose (cmcm2);
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmit);
	abc_fclose (cmit2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cumr);
	abc_fclose (cmpb);
	abc_fclose (cmtr);
	abc_fclose (cmts);
	abc_fclose (inmr);
	abc_dbclose (data);
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

void
RunProgram (
	char	*programName, 
	char	*programDesc)
{
	char	lower [11];
	char	upper [11];

	if (local_rec.onight [0] == 'Y' || local_rec.back [0] == 'Y')
	{
		CloseDB (); 
		snorm ();
		rset_tty ();
	}

	switch (local_rec.rangeType [0])
	{
	case 'C':
		sprintf (lower, "%-6.6s", local_rec.startContNo);
		sprintf (upper, "%-6.6s", local_rec.endContNo);
		break;

	case 'W':
		sprintf (lower, "%-4.4s", local_rec.startWip);
		sprintf (upper, "%-4.4s", local_rec.endWip);
		break;
	case 'J':
		sprintf (lower, "%-4.4s", local_rec.startJobType);
		sprintf (upper, "%-4.4s", local_rec.endJobType);
		break;

	case 'I':
		sprintf (lower, "%-10.10s", local_rec.startIssue);
		sprintf (upper, "%-10.10s", local_rec.endIssue);
		break;

	}

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT \"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			programName, 
			local_rec.printerNo,
			local_rec.sumDet, 
			local_rec.rangeType, 
			local_rec.internal, 
			lower, 
			upper, 
			programDesc
		);
		SystemExec (err_str, TRUE);
	}
	else 
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			programName, 
			local_rec.printerNo,
			local_rec.sumDet, 
			local_rec.rangeType, 
			local_rec.internal, 
			lower, 
			upper
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
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
		for (i = label ("startContNo"); i <= label ("endIssueDesc"); i++)
			vars [i].required = ND;

		switch (local_rec.rangeType [0])
		{
		case 'C':
			FLD ("startContNo") 	= YES;
			FLD ("startContDesc") 	= NA;
			FLD ("endContNo") 		= YES;
			FLD ("endContDesc") 	= NA;
			strcpy (local_rec.rangeDesc, ML ("Contract"));
			break;

		case 'J':
			FLD ("startJobType") 	= YES;
			FLD ("startJobDesc") 	= NA;
			FLD ("endJobType") 		= YES;
			FLD ("endJobDesc") 		= NA;
			strcpy (local_rec.rangeDesc, ML ("Job Type"));
			break;

		case 'W':
			FLD ("startWip") 		= YES;
			FLD ("startWipDesc") 	= NA;
			FLD ("endWip") 			= YES;
			FLD ("endWipDesc") 		= NA;
			strcpy (local_rec.rangeDesc, ML ("WIP Status"));
			break;

		case 'I':
			FLD ("startIssue") 		= YES;
			FLD ("startIssueDesc") 	= NA;
			FLD ("endIssue") 		= YES;
			FLD ("endIssueDesc") 	= NA;
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
	if (LCHECK ("startContNo"))
	{
		if (FLD ("startContNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startContNo, "      ");
			strcpy (local_rec.startContDesc, ML ("First Contract"));
			DSP_FLD ("startContNo");
			DSP_FLD ("startContDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num (local_rec.startContNo);
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.startContNo);
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
		    strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
		{
			print_mess (ML (mlStdMess017));
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
			strcpy (cmcd_rec.text, ML ("Description Not Found"));
		
		strcpy (local_rec.startContDesc, cmcd_rec.text);
		DSP_FLD ("startContDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endContNo"))
	{
		if (FLD ("endContNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endContNo, "~~~~~~");
			strcpy (local_rec.endContDesc, ML ("Last Contract"));
			DSP_FLD ("endContNo");
			DSP_FLD ("endContDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num (local_rec.endContNo);
		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.endContNo);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Check against start contract no. 
		 */
		if (strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
		{
			print_mess (ML (mlStdMess018));
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
		{
			strcpy (cmcd_rec.text, ML ("Description Not Found"));
		}
		strcpy (local_rec.endContDesc, cmcd_rec.text);
		DSP_FLD ("endContDesc");

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
			strcpy (local_rec.startJobDesc, ML ("First Job Type"));
			DSP_FLD ("startJobType");
			DSP_FLD ("startJobDesc");
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
			print_mess (ML (mlStdMess088));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check against end JobType no. 
		 */
		if (prog_status != ENTRY && 
		    strcmp (local_rec.startJobType, local_rec.endJobType) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startJobDesc, cmjt_rec.desc);
		DSP_FLD ("startJobDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endJobType"))
	{
		if (FLD ("endJobType") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endJobType, "~~~~");
			strcpy (local_rec.endJobDesc, ML ("Last Job Type"));
			DSP_FLD ("endJobType");
			DSP_FLD ("endJobDesc");
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

		/*
		 * Check against start JobType no. 
		 */
		if (strcmp (local_rec.startJobType, local_rec.endJobType) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endJobDesc, cmjt_rec.desc);
		DSP_FLD ("endJobDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate WIP Status Range. 
	 */
	if (LCHECK ("startWip"))
	{
		if (FLD ("startWip") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startWip, "    ");
			strcpy (local_rec.startWipDesc, ML ("First WIP Status"));
			DSP_FLD ("startWip");
			DSP_FLD ("startWipDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmws (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws_rec.co_no, comm_rec.co_no);
		sprintf (cmws_rec.wp_stat, "%-4.4s", local_rec.startWip);
		cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check against end WIP Status. 
		 */
		if (prog_status != ENTRY && 
		    strcmp (local_rec.startWip, local_rec.endWip) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startWipDesc, cmws_rec.desc);
		DSP_FLD ("startWipDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endWip"))
	{
		if (FLD ("endWip") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endWip, "~~~~");
			strcpy (local_rec.endWipDesc, ML ("Last WIP Status"));
			DSP_FLD ("endWip");
			DSP_FLD ("endWipDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmws (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws_rec.co_no, comm_rec.co_no);
		sprintf (cmws_rec.wp_stat, "%-4.4s", local_rec.endWip);
		cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check against start WIP Status. 
		 */
		if (strcmp (local_rec.startWip, local_rec.endWip) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endWipDesc, cmws_rec.desc);
		DSP_FLD ("endWipDesc");

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

		/*
		 * Check against end issto Code . 
		 */
		if (prog_status != ENTRY && 
		    strcmp (local_rec.startIssue, local_rec.endIssue) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startIssueDesc, cmit_rec.iss_name);
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

		/*
		 * Check against start issto code.  
		 */
		if (strcmp (local_rec.startIssue, local_rec.endIssue) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endIssueDesc, cmit_rec.iss_name);
		DSP_FLD ("endIssueDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("internal"))
	{
		if (local_rec.internal [0] == 'Y')
			strcpy (local_rec.internalDesc, ML ("Yes"));
		else
			strcpy (local_rec.internalDesc, ML ("No "));
	
		DSP_FLD ("internalDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sumDet"))
	{
		if (SUMMARY)
			strcpy (local_rec.sumDetDesc, ML ("Summary"));
		else
			strcpy (local_rec.sumDetDesc, ML ("Detail "));
	
		DSP_FLD ("sumDetDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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

void
SrchCmhr (
	char	*keyValue)
{
	_work_open (6, 0, 30);
	save_rec ("#No.", "#Customer Order No.");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		if (cc)
			break;

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
SrchCmws (
	char	*keyValue)
{
	_work_open (4, 0, 40);
	save_rec ("#Code", "#WIP Code Description ");

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", keyValue);
	cc = find_rec (cmws, &cmws_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmws_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmws_rec.wp_stat, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmws_rec.wp_stat, cmws_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmws, &cmws_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%4.4s", temp_str);

	cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmws, "DBFIND");
}

void
SrchCmit (
	char	*keyValue)
{
	_work_open (10, 0, 40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit_rec.co_no, comm_rec.co_no);
	sprintf (cmit_rec.issto, "%-10.10s", keyValue);
	cc = find_rec (cmit, &cmit_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit_rec.issto, keyValue, strlen (keyValue)))
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
	char	*keyValue)
{
	_work_open (4, 0, 40);
	save_rec ("#Job", "#Job Type Description");

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", keyValue);
	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmjt_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmjt_rec.job_type, keyValue, strlen (keyValue)))
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

void
Process (void)
{
	int	firstContract;

	dsp_screen ("Processing Contracts", comm_rec.co_no, comm_rec.co_name);
	
	/*
	 * Read contract records. 
	 */
	firstContract = TRUE;
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	if (PROC_CNT)
		sprintf (cmhr_rec.cont_no, "%-6.6s", local_rec.startContNo);
	else
		sprintf (cmhr_rec.cont_no, "%-6.6s", " ");
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo))
	{
		/*
		 * Must be status OPEN. 
		 */
		if (cmhr_rec.status [0] != 'O')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Internal OR External 
		 */
		if (cmhr_rec.internal [0] != local_rec.internal [0])
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check range for PROC_CNT. 
		 */
		if (PROC_CNT && strcmp (cmhr_rec.cont_no, local_rec.endContNo) > 0)
			break;

		/*
		 * Check range for PROC_WIP. 
		 */
		if (PROC_WIP && 
		   (strcmp (cmhr_rec.wip_status, local_rec.endWip) > 0 ||
		     strcmp (cmhr_rec.wip_status, local_rec.startWip) < 0))
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup job type code for contract. 
		 */
		cmjt_rec.hhjt_hash = cmhr_rec.hhjt_hash;
		cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
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
			cmit_rec.hhit_hash = cmhr_rec.hhit_hash;
			cc = find_rec (cmit2, &cmit_rec, COMPARISON, "r");
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
		 * Lookup customer for contract. 
		 */
		cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Report Headings.
		 */
		if (!pipeOpen)
			HeadingOutput ();

		/*
		 * Contract info. 
		 */
		if (!firstContract && !SUMMARY)
		{
			fprintf (fout, ".DS0\n");
			fprintf (fout, ".PA\n");
		}
		PrintContract ();
		firstContract = FALSE;

		/*
		 * Print Summary/Detail info. 
		 */
		if (!SUMMARY)
			PrintDetails ();

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	if (pipeOpen)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
		pipeOpen = FALSE;
	}
}

void
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", local_rec.systemDate, clip (err_str));
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L165\n");

	fprintf (fout, 
		".E %s OPEN CONTRACTS READY FOR %s. (%s REPORT) \n", 
		(local_rec.internal [0] == 'Y') ? "INTERNAL" : "EXTERNAL", 
		(local_rec.internal [0] == 'Y') ? "UPDATE" : "BILLING", 
		(SUMMARY) ? "SUMMARY" : "DETAILED");

	switch (local_rec.rangeType [0])
	{
	case 'C':
		fprintf (fout, ".E For Contract '%-6.6s' to '%-6.6s'\n", 
							local_rec.startContNo, 
							local_rec.endContNo);
		break;

	case 'W':
		fprintf (fout, ".E For WIP Status '%-4.4s' to '%-4.4s'\n", 
							local_rec.startWip, 
							local_rec.endWip);
		break;

	case 'J':
		fprintf (fout, ".E For Job Type '%-4.4s' to '%-4.4s'\n", 
							local_rec.startJobType, 
							local_rec.endJobType);
		break;

	case 'I':
		fprintf (fout, ".E For Issue To Code '%-10.10s' to '%-10.10s'\n", 
							local_rec.startIssue, 
							local_rec.endIssue);
		break;

	}
	fprintf (fout, ".E Company : %2.2s - %s \n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E AS AT : %-24.24s \n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "===========================================");
	fprintf (fout, "====================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "=========");
	fprintf (fout, "=======================\n");

	fprintf (fout, "|                CONTRACT                  ");
	fprintf (fout, "|    LAST INVOICE     ");
	fprintf (fout, "|INVOICE TO");
	fprintf (fout, "| COST   TO");
	fprintf (fout, "|   COST   ");
	fprintf (fout, "|  SALES   ");
	fprintf (fout, "| QUOTE  ");
	fprintf (fout, "|  QUOTE   ");
	fprintf (fout, "|    DUE   ");
	fprintf (fout, "| WORK IN PROGRESS|\n");

	fprintf (fout, "|NUMBER|            DESCRIPTION            ");
	fprintf (fout, "|   DATE   |  AMOUNT  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|  AMOUNT  ");
	fprintf (fout, "|  AMOUNT  ");
	fprintf (fout, "|  TYPE  ");
	fprintf (fout, "|  AMOUNT  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|STATUS|   DATE   |\n");

	fprintf (fout, "|------|-----------------------------------");
	fprintf (fout, "|----------|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------|----------|\n");

	fprintf (fout, ".R===========================================");
	fprintf (fout, "====================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "=========");
	fprintf (fout, "=======================\n");


	pipeOpen = TRUE;
}

void
PrintContract (void)
{
	dsp_process ("Contract : ", cmhr_rec.cont_no);

	/*
	 * Lookup first line of description. 
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
	if (cc)
		strcpy (cmcd_rec.text, ML ("NOT FOUND"));

	/*
	 * Lookup cmpb for last invoice date and amount. 
	 */
	cmpb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmpb_rec.inv_date = 9999999L;
	cc = find_rec (cmpb, &cmpb_rec, GTEQ, "r");
	if (cc)
		cc = find_rec (cmpb, &cmpb_rec, LAST, "r");
	else
		cc = find_rec (cmpb, &cmpb_rec, PREVIOUS, "r");
	lastDate = 0L;
	strcpy (contRec.lastInvDate, "          ");
	strcpy (contRec.lastInvAmount, "            ");
	if (!cc && cmpb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		lastDate = cmpb_rec.inv_date;
		sprintf (contRec.lastInvDate, "%-10.10s", DateToString (lastDate));
		sprintf (contRec.lastInvAmount, "%10.2f", DOLLARS (cmpb_rec.amount));
	}
	
	/*
	 * Calculate 'to date' amounts and amounts since last invoice. 
	 */
	CalcTodate ();

	/*
	 * Quote value. 
	 */
	if (cmhr_rec.quote_val == 0.00)
		strcpy (contRec.quoteAmount, "          ");
	else
		sprintf (contRec.quoteAmount, "%10.2f", DOLLARS (cmhr_rec.quote_val));

	/*
	 * Finish Date. 
	 */
	if (cmhr_rec.due_date == 0L)
		strcpy (contRec.finishDate, "          ");
	else
		sprintf (contRec.finishDate, "%-10.10s", DateToString (cmhr_rec.due_date));
	fprintf (fout, "|%-6.6s", cmhr_rec.cont_no);
	fprintf (fout, "|%-35.35s", cmcd_rec.text);
	fprintf (fout, "|%10.10s" , contRec.lastInvDate);
	fprintf (fout, "|%10.10s", contRec.lastInvAmount);
	fprintf (fout, "|%10.10s", contRec.invToDate);
	fprintf (fout, "|%10.10s", contRec.costToDate);
	fprintf (fout, "|%10.10s", contRec.amountCost);
	fprintf (fout, "|%10.10s", contRec.amountSale);
	fprintf (fout, "|%-8.8s", 
		(cmhr_rec.quote_type [0] == 'F') ? "Fixed" : "Variable");
	fprintf (fout, "|%10.10s", contRec.quoteAmount);
	fprintf (fout, "|%-10.10s", contRec.finishDate);
	fprintf (fout, "| %-4.4s ", cmhr_rec.wip_status);
	fprintf (fout, "|%-10.10s|\n", DateToString (cmhr_rec.wip_date));
}

void
PrintDetails (void)
{

	/*
	 * Print ongoing description.
	 */
	fprintf (fout, "| %-150.150s |\n", " ");
	DescHead (FALSE);
	DescHead (TRUE);
	DescDetails ();
	fprintf (fout, "|                                       ");
	fprintf (fout, "-------------------------------------");
	fprintf (fout, "------------------------------------");
	fprintf (fout, "                                        |\n");
	fprintf (fout, "| %-150.150s |\n", " ");

	/*
	 * Print Employee/Plant Time. 
	 */
	fprintf (fout, "| %-150.150s |\n", " ");
	TimeHead (FALSE);
	TimeHead (TRUE);
	TimeDetails ();
	fprintf (fout, "|           ---------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------               |\n");
	fprintf (fout, "| %-150.150s |\n", " ");

	/*
	 * Print Materials. 
	 */
	MaterialHead (FALSE);
	MaterialHead (TRUE);
	MaterialDetails ();
	fprintf (fout, "|                   -------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------                       |\n");
	fprintf (fout, "| %-150.150s |\n", " ");
}

/*
 * Print header for Employee/Plant Time section. 
 */
void
TimeHead (
	int	defineSection)
{
	if (defineSection)
	{
		fprintf (fout, ".DS6\n");

		fprintf (fout, "|%-6.6s", cmhr_rec.cont_no);
		fprintf (fout, "|%-35.35s", cmcd_rec.text);
		fprintf (fout, "|%-10.10s", contRec.lastInvDate);
		fprintf (fout, "|%10.10s", contRec.lastInvAmount);
		fprintf (fout, "|%10.10s", contRec.invToDate);
		fprintf (fout, "|%10.10s", contRec.costToDate);
		fprintf (fout, "|%10.10s", contRec.amountCost);
		fprintf (fout, "|%10.10s", contRec.amountSale);
		fprintf (fout, 
			"|%-8.8s",  
			(cmhr_rec.quote_type [0] == 'F') ? "Fixed" : "Variable");
		fprintf (fout, "|%10.10s", contRec.quoteAmount);
		fprintf (fout, "|%-10.10s", contRec.finishDate);
		fprintf (fout, "| %-4.4s ", cmhr_rec.wip_status);
		fprintf (fout, "|%-10.10s\n", DateToString (cmhr_rec.wip_date));

		fprintf (fout, "| %-150.150s |\n", " ");
	}
	else
	{
		fprintf (fout, ".DS0\n");
		fprintf (fout, ".LRP12\n");
	}
	
	fprintf (fout, "|           ---------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------               |\n");

	fprintf (fout, "|           |        ");
	fprintf (fout, "| EMPLOYEE ");
	fprintf (fout, "|          ");
	fprintf (fout, "| UNITS ");
	fprintf (fout, "| 1.0   ");
	fprintf (fout, "| 1.5   ");
	fprintf (fout, "| 2.0   ");
	fprintf (fout, "|LABOUR COST");
	fprintf (fout, "|O/HEAD COST");
	fprintf (fout, "|  LABOUR   ");
	fprintf (fout, "| OVERHEAD  ");
	fprintf (fout, "|   TOTAL     |               |\n");

	fprintf (fout, "|           |  DATE  ");
	fprintf (fout, "|   CODE   ");
	fprintf (fout, "|PLANT CODE");
	fprintf (fout, "| USED  ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| HOURS ");
	fprintf (fout, "| PER UNIT  ");
	fprintf (fout, "| PER UNIT  ");
	fprintf (fout, "|   COSTS   ");
	fprintf (fout, "|   COSTS   ");
	fprintf (fout, "|   COSTS     |               |\n");

	fprintf (fout, "|           |--------");
	fprintf (fout, "+----------");
	fprintf (fout, "+----------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-----------");
	fprintf (fout, "+-------------|               |\n");
}

/*
 * Print header for Ongoing Description Section. 
 */
void
DescHead (
 int	defineSection)
{
	if (defineSection)
	{
		fprintf (fout, ".DS5\n");

		fprintf (fout, "|%-6.6s", cmhr_rec.cont_no);
		fprintf (fout, "|%-35.35s", cmcd_rec.text);
		fprintf (fout, "|%-10.10s", contRec.lastInvDate);
		fprintf (fout, "|%10.10s", contRec.lastInvAmount);
		fprintf (fout, "|%10.10s", contRec.invToDate);
		fprintf (fout, "|%10.10s", contRec.costToDate);
		fprintf (fout, "|%10.10s", contRec.amountCost);
		fprintf (fout, "|%10.10s", contRec.amountSale);
		fprintf (fout, 
			"|%-8.8s",  
			(cmhr_rec.quote_type [0] == 'F') ? "Fixed" : "Variable");
		fprintf (fout, "|%10.10s", contRec.quoteAmount);
		fprintf (fout, "|%-10.10s", contRec.finishDate);
		fprintf (fout, "| %-4.4s ", cmhr_rec.wip_status);
		fprintf (fout, "|%-10.10s\n", DateToString (cmhr_rec.wip_date));

		fprintf (fout, "| %-150.150s |\n", " ");
	}
	else
	{
		fprintf (fout, ".DS0\n");
		fprintf (fout, ".LRP12\n");
	}
	
	fprintf (fout, "|                                       ");
	fprintf (fout, "-------------------------------------");
	fprintf (fout, "------------------------------------");
	fprintf (fout, "                                        |\n");
	
	fprintf (fout, "|                                       ");
	fprintf (fout, "|                    O N G O I N G ");
	fprintf (fout, "  D E S C R I P T I O N S            |");
	fprintf (fout, "                                        |\n");

	fprintf (fout, "|                                       ");
	fprintf (fout, "-------------------------------------");
	fprintf (fout, "------------------------------------");
	fprintf (fout, "                                        |\n");
}

void
DescDetails (void)
{
	int	found = FALSE;

	cmcd_rec.line_no = 7;
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc && 
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       cmcd_rec.stat_flag [0] == 'D')
	{
		found = TRUE;
		fprintf (fout, "|                                       ");
		fprintf (fout, "| %-70.70s|", cmcd_rec.text);
		fprintf (fout, "                                        |\n");
		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}
	if (!found)
	{
		fprintf (fout, "|                                       ");
		fprintf (fout, 
			 "| %-20.20s%-30.30s%-20.20s|", 
			 " ", 
		         "*** NO ONGOING DESCRIPTION ***", 
			 " ");

		fprintf (fout, "                                        |\n");
	}

}

/*
 * Print header for Materials section. 
 */
void
MaterialHead (
 int	defineSection)
{
	if (defineSection)
	{
		fprintf (fout, ".DS6\n");

		fprintf (fout, "|%-6.6s", cmhr_rec.cont_no);
		fprintf (fout, "|%-35.35s", cmcd_rec.text);
		fprintf (fout, "|%-10.10s", contRec.lastInvDate);
		fprintf (fout, "|%10.10s", contRec.lastInvAmount);
		fprintf (fout, "|%10.10s", contRec.invToDate);
		fprintf (fout, "|%10.10s", contRec.costToDate);
		fprintf (fout, "|%10.10s", contRec.amountCost);
		fprintf (fout, "|%10.10s", contRec.amountSale);
		fprintf (fout, 
			"|%-8.8s",
			(cmhr_rec.quote_type [0] == 'F') ? "Fixed" : "Variable");
		fprintf (fout, "|%10.10s", contRec.quoteAmount);
		fprintf (fout, "|%-10.10s", contRec.finishDate);
		fprintf (fout, "| %-4.4s ", cmhr_rec.wip_status);
		fprintf (fout, "|%-10.10s\n", DateToString (cmhr_rec.wip_date));

		fprintf (fout, "| %-150.150s |\n", " ");
	}
	else
	{
		fprintf (fout, ".DS0\n");
		fprintf (fout, ".LRP12\n");
	}
	
	fprintf (fout, "|                   -------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "---------------------------------------");
	fprintf (fout, "-------------                       |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|          ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "| QUANTITY  ");
	fprintf (fout, "|   UNIT    ");
	fprintf (fout, "|   TOTAL   ");
	fprintf (fout, "|                       |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|             ITEM DESCRIPTION             ");
	fprintf (fout, "|   USED    ");
	fprintf (fout, "|   COST    ");
	fprintf (fout, "|   COST    ");
	fprintf (fout, "|                       |\n");

	fprintf (fout, "|                   ");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|                       |\n");
}

/*
 * Display Materials Used for contract. 
 */
void
MaterialDetails (void)
{
	int	firstTrans;
	int	dataFound;
	double	costHeadTotal;
	double	allTotal;

	allTotal = 0.00;

	/*
	 * Process cmcm records. 
	 */
	dataFound = FALSE;
	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", " ");
	cc = find_rec (cmcm2, &cmcm_rec, GTEQ, "r");
	while (!cc && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
		costHeadTotal = 0.00;

		/*
		 * Process cmtr records. 
		 */
		firstTrans = TRUE;
		cmtr_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cmtr_rec.hhcm_hash = cmcm_rec.hhcm_hash;
		cmtr_rec.date = 0L;
		cc = find_rec (cmtr, &cmtr_rec, GTEQ, "r");
		while (!cc && 
		       cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		       cmtr_rec.hhcm_hash == cmcm_rec.hhcm_hash)
		{
			/*
			 * Record must be AFTER last invoice date. 
			 */
			if (cmtr_rec.date <= lastDate)
			{
				cc = find_rec (cmts, &cmts_rec, NEXT, "r");
				continue;
			}

			/*
			 * Look up inventory record. 
			 */
			inmr_rec.hhbr_hash = cmtr_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
			if (cc)
			{
				cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
				continue;
			}
	
			if (firstTrans)
			{
				fprintf (fout, 
					"|%-19.19s| Costhead : %-4.4s  %-40.40s%-50.50s|                       |\n", 
					" ", 
					cmcm_rec.ch_code, 
					cmcm_rec.desc, 
					" ");
			}
			firstTrans = FALSE;
	
			dataFound = TRUE;
			fprintf (fout, ".LRP6\n");
			fprintf (fout, "|%-19.19s", " ");
			fprintf (fout, "|%-10.10s", DateToString (cmtr_rec.date));
			fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
			fprintf (fout, "| %-40.40s ", inmr_rec.description);
			fprintf (fout, "| %9.2f ", cmtr_rec.qty);
			fprintf (fout, "| %9.2f ", DOLLARS (cmtr_rec.cost_price));
			fprintf (fout, "| %9.2f ", DOLLARS (cmtr_rec.cost_price * cmtr_rec.qty));
			fprintf (fout, "|%-23.23s|\n", " ");
			
			costHeadTotal += (cmtr_rec.cost_price * cmtr_rec.qty);
	
			cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
		}

		/*
		 * Costhead Total. 
		 */
		if (costHeadTotal != 0.00)
		{
			fprintf (fout, 
				"|%-19.19s| TOTAL FOR COSTHEAD : %-4.4s  %-40.40s %-25.25s  |%10.2f |%-23.23s|\n", 
				" ", 
				cmcm_rec.ch_code, 
				cmcm_rec.desc, 
				" ", 
				DOLLARS (costHeadTotal), 
				" ");
	
			fprintf (fout, "|%-19.19s|", " ");
			fprintf (fout, " %-105.105s  ", " ");
			fprintf (fout, "|%-23.23s|\n", " ");
		
			allTotal += costHeadTotal;
		}

		cc = find_rec (cmcm2, &cmcm_rec, NEXT, "r");
	}

	/*
	 * Materials Total. 
	 */
	if (allTotal != 0.00)
	{
		fprintf (fout, 
			"|%-19.19s| GRAND TOTAL : %-80.80s |%10.2f |%-23.23s|\n", 
			" ", 
			" ", 
			DOLLARS (allTotal), 
			" ");
	}

	if (!dataFound)
	{
		fprintf (fout, 
			"|%-19.19s| %-106.106s |%-23.23s|\n", 
			" ", 
			"                               ****     NO MATERIALS FOR CONTRACT     **** ", 
			" ");
	}
}

/*
 * Display Transactions for contract. 
 */
void
TimeDetails (void)
{
	int	firstTrans;
	int	proc_equip;
	int	dataFound;
	double	tot_time;
	double	tot_lab;
	double	tot_oh;
	double	tot_cost;
	double	costHeadTotal;
	double	allTotal;
	char	units [8];
	char	time_ord [8];
	char	time_hlf [8];
	char	time_dbl [8];
	char	lab_unit_cst [11];
	char	oh_unit_cst [11];
	char	lab_cost [11];
	char	oh_cost [11];
	char	tot_cost_str [13];

	allTotal = 0.00;

	/*
	 * Process cmcm records. 
	 */
	dataFound = FALSE;
	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", " ");
	cc = find_rec (cmcm2, &cmcm_rec, GTEQ, "r");
	while (!cc && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
	    costHeadTotal = 0.00;

	    /*
	     * Process cmts records. 
	     */
	    firstTrans = TRUE;
	    cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	    cmts_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	    cmts_rec.date = 0L;
	    cc = find_rec (cmts, &cmts_rec, GTEQ, "r");
	    while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		   			  cmts_rec.hhcm_hash == cmcm_rec.hhcm_hash)
	    {
		/*
		 * Record must be AFTER last invoice date. 
		 */
		if (cmts_rec.date <= lastDate)
		{
		    cc = find_rec (cmts, &cmts_rec, NEXT, "r");
		    continue;
		}

		/*
		 * Look up employee record. 
		 */
		cmem_rec.hhem_hash = cmts_rec.hhem_hash;
		cc = find_rec (cmem, &cmem_rec, GTEQ, "r");
		if (cc)
		{
		    cc = find_rec (cmts, &cmts_rec, NEXT, "r");
		    continue;
		}
	
		/*
		 * Look up plant record if necessary. 
		 */
		proc_equip = FALSE;
		if (cmts_rec.hheq_hash == 0L)
		    sprintf (cmeq_rec.eq_name, "%-10.10s", " ");
		else
		{
			cmeq_rec.hheq_hash = cmts_rec.hheq_hash;
		    cc = find_rec (cmeq, &cmeq_rec, GTEQ, "r");
		    if (cc)
		    {
				cc = find_rec (cmts, &cmts_rec, NEXT, "r");
				continue;
		    }
		    proc_equip = TRUE;
		}
	
		/*
		 * Costhead Heading. 
		 */
		if (firstTrans)
		{
		    fprintf (fout, 
				"|%-11.11s| Costhead : %-4.4s  %-40.40s %-65.65s|               |\n", 
				" ", 
				cmcm_rec.ch_code, 
				cmcm_rec.desc, 
				" ");
		}
		firstTrans = FALSE;
	
		/*--------
		| Units. |
		--------*/
		if (cmts_rec.units == 0.00)
		    strcpy (units, "       ");
		else
		    sprintf (units, "%7.2f", cmts_rec.units);

		/*
		 * Ordinary Time. 
		 */
		if (cmts_rec.time_ord == 0.00)
		    strcpy (time_ord, "       ");
		else
		    sprintf (time_ord, "%7.2f", cmts_rec.time_ord);

		/*
		 * Time and a Half. 
		 */
		if (cmts_rec.time_hlf == 0.00)
		    strcpy (time_hlf, "       ");
		else
		    sprintf (time_hlf, "%7.2f", cmts_rec.time_hlf);

		/*
		 * Double Time. 
		 */
		if (cmts_rec.time_dbl == 0.00)
		    strcpy (time_dbl, "       ");
		else
		    sprintf (time_dbl, "%7.2f", cmts_rec.time_dbl);

		/*
		 * Labour Unit Cost. 
		 */
		if (cmts_rec.lab_cost == 0.00)
		    strcpy (lab_unit_cst, "          ");
		else
		{
		    sprintf (lab_unit_cst, "%10.2f", DOLLARS (cmts_rec.lab_cost));
		}

		/*
		 * Overhead Unit Cost. 
		 */
		if (cmts_rec.oh_cost == 0.00)
		    strcpy (oh_unit_cst, "          ");
		else
		{
		    sprintf (oh_unit_cst, "%10.2f", DOLLARS (cmts_rec.oh_cost));
		}

		/*
		 * Calculate total hours. 
		 */
		if (proc_equip)
		    tot_time = cmts_rec.units;
		else
		{
		    tot_time = cmts_rec.time_ord +
					  (cmts_rec.time_hlf * 1.5) +
					  (cmts_rec.time_dbl * 2.0);
		}

		/*
		 * Labour Cost. 
		 */
		if (proc_equip)
		{
		    tot_lab = 0.00;
		    strcpy (lab_cost, "          ");
		}
		else
		{
		    tot_lab = tot_time * cmts_rec.lab_cost;
		    if (tot_lab == 0.00)
				strcpy (lab_cost, "          ");
		    else
		    {
				sprintf (lab_cost, "%10.2f", DOLLARS (tot_lab));
		    }
		}

		/*
		 * Overhead Cost. 
		 */
		tot_oh = tot_time * cmts_rec.oh_cost;
		if (tot_oh == 0.00)
		    strcpy (oh_cost, "          ");
		else
		{
		    sprintf (oh_cost, "%10.2f", DOLLARS (tot_oh));
		}

		/*
		 * TOTAL Cost. 
		 */
		tot_cost = tot_lab + tot_oh;
		if (tot_cost == 0.00)
		    strcpy (tot_cost_str, "            ");
		else
		{
		    sprintf (tot_cost_str, 
			    "%12.2f", 
			    DOLLARS (tot_cost));
		}

		/*
		 * Display String. 
		 */
		dataFound = TRUE;
		fprintf (fout, ".LRP7\n");
		fprintf (fout, "|%-11.11s|%-10.10s", " ", DateToString (cmts_rec.date));
		fprintf (fout, "|%-10.10s", cmem_rec.emp_no);
		fprintf (fout, "|%-10.10s", cmeq_rec.eq_name);
		fprintf (fout, "|%-7.7s", units);
		fprintf (fout, "|%-7.7s", time_ord);
		fprintf (fout, "|%-7.7s", time_hlf);
		fprintf (fout, "|%-7.7s", time_dbl);
		fprintf (fout, "| %10.10s", lab_unit_cst);
		fprintf (fout, "| %10.10s", oh_unit_cst);
		fprintf (fout, "| %10.10s", lab_cost);
		fprintf (fout, "| %10.10s", oh_cost);
		fprintf (fout, "| %12.12s|               |\n", tot_cost_str);

		costHeadTotal += tot_cost;
	
		cc = find_rec (cmts, &cmts_rec, NEXT, "r");
	    }

	    /*
	     * Costhead Total. 
	     */
	    if (costHeadTotal != 0.00)
	    {
		fprintf (fout, "|           | TOTAL FOR COSTHEAD :  ");
		fprintf (fout, 
			    "%-4.4s  %-40.40s %-40.40s", 
			    cmcm_rec.ch_code, 
			    cmcm_rec.desc, 
			    " ");
		fprintf (fout, 
			    "| %12.2f|%15.15s|\n", 
			    DOLLARS (costHeadTotal), 
			    " ");

		fprintf (fout, "|           |");
		fprintf (fout, "  %-120.120s  ", " ");
		fprintf (fout, "|               |\n");
		
		allTotal += costHeadTotal;
	    }

	    cc = find_rec (cmcm2, &cmcm_rec, NEXT, "r");
	}

	/*
	 * Timesheets Total. 
	 */
	if (allTotal != 0.00)
	{
		fprintf (fout, 
			"|           | GRAND TOTAL OF EMPLOYEE / PLANT TIME  %-70.70s | %12.2f|               |\n", 
			" ", 
			DOLLARS (allTotal));
	}

	if (!dataFound)
	{
		fprintf (fout, 
			"|           | %-122.122s |               |\n", 
			"                                 ****     NO EMPLOYEE / PLANT TIME FOR CONTRACT     **** ");
	}
}

/*
 * Calculate 'to date' amounts 
 */
void
CalcTodate (void)
{
	double	invToDate	= 0.00, 
			costToDate	= 0.00, 
			amountCost	= 0.00, 
			amountSale	= 0.00, 
			lcl_cost	= 0.00, 
			lcl_sales	= 0.00, 
			lcl_disc	= 0.00, 
			lcl_units	= 0.00;

	invToDate	= 0.00;
	costToDate 	= 0.00;
	amountCost  = 0.00;
	amountSale  = 0.00;

	/*
	 * Accumulate Material Transactions 
	 */
	cmtr_rec.hhhr_hash	= cmhr_rec.hhhr_hash;
	cmtr_rec.hhcm_hash	= 0L;
	cmtr_rec.date		= 0L;
	cc = find_rec (cmtr, &cmtr_rec, GTEQ, "r");
	while (!cc && cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmtr_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Transaction cost. 
		 */
		lcl_cost = (cmtr_rec.cost_price * (double)cmtr_rec.qty);
		costToDate += lcl_cost;

		/*
		 * Transaction Sales. 
		 */
		lcl_sales = (cmtr_rec.sale_price * (double)cmtr_rec.qty);
		lcl_disc = lcl_sales * (double) (cmtr_rec.disc_pc / 100.00);
		lcl_sales -= lcl_disc;
		
		if (cmtr_rec.date > lastDate)
		{
			amountCost += lcl_cost;
			amountSale += lcl_sales;
		}
		cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
	}

	/*
	 * Accumulate Employee and Plant Transactions 
	 */
	cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmts_rec.hhcm_hash = 0L;
	cmts_rec.date = 0L;
	cc = find_rec (cmts, &cmts_rec, GTEQ, "r");
	while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmts_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmts, &cmts_rec, NEXT, "r");
			continue;
		}

		if (cmts_rec.hheq_hash == 0)
		{
			lcl_units = (cmts_rec.time_ord + 
				    	(cmts_rec.time_hlf * 1.5) + 
				    	(cmts_rec.time_dbl * 2.0));
			lcl_cost = cmts_rec.lab_cost + cmts_rec.oh_cost;
		}
		else
		{
			lcl_units = cmts_rec.units;
			lcl_cost  = cmts_rec.oh_cost;
		}

		costToDate += ((double)lcl_units * lcl_cost);

		if (cmts_rec.date > lastDate)
		{
			amountCost += ((double)lcl_units * lcl_cost);
			lcl_sales = ((double)lcl_units * cmts_rec.sale);
			amountSale += lcl_sales;
		}

		cc = find_rec (cmts, &cmts_rec, NEXT, "r");
	}

	/*
	 * Calculate amount invoiced to date. 
	 */
	cmpb_rec.hhhr_hash = cmhr_rec.hhhr_hash;	
	cc = find_rec (cmpb, &cmpb_rec, GTEQ, "r");
	while (!cc && cmpb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmpb_rec.date > local_rec.lsystemDate)
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		invToDate += cmpb_rec.amount;

		cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
	}

	/*
	 * Store values as strings in contract info structure. 
	 */
	if (invToDate == 0.00)
		strcpy (contRec.invToDate, "          ");
	else
		sprintf (contRec.invToDate, "%10.2f", DOLLARS (invToDate));

	if (costToDate == 0.00)
		strcpy (contRec.costToDate, "          ");
	else
		sprintf (contRec.costToDate, "%10.2f", DOLLARS (costToDate));

	if (amountCost == 0.00)
		strcpy (contRec.amountCost, "          ");
	else
		sprintf (contRec.amountCost, "%10.2f", DOLLARS (amountCost));

	if (amountSale == 0.00)
		strcpy (contRec.amountSale, "          ");
	else
		sprintf (contRec.amountSale, "%10.2f", DOLLARS (amountSale));
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
	int		scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		swide ();

		centre_at (0, 132, ML (mlCmMess073));

		box (0, 3, 132, 11);
		line_at (1, 0, 132);
		line_at (5, 1, 131);
		line_at (8, 1, 131);
		line_at (11, 1, 131);

		line_cnt = 0;
		scn_write (scn);
	}
	line_at (21, 0, 132);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}
