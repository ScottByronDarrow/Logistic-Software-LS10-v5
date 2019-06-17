/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_req_stat.c,v 5.2 2002/07/17 09:57:01 scott Exp $
|  Program Name  : (cm_req_stat.c) 
|  Program Desc  : (Contract Due Report)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (05/04/93)       |
|---------------------------------------------------------------------|
| $Log: cm_req_stat.c,v $
| Revision 5.2  2002/07/17 09:57:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2002/01/21 04:10:58  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_req_stat.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_req_stat/cm_req_stat.c,v 5.2 2002/07/17 09:57:01 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	PROC_CNT	(local_rec.rangeType [0] == 'C')
#define	PROC_JOB	(local_rec.rangeType [0] == 'J')
#define	PROC_REQ	(local_rec.rangeType [0] == 'R')
#define	PROC_ITM	(local_rec.rangeType [0] == 'I')

#include	"schema"

struct commRecord	comm_rec;
struct cmcdRecord	cmcd_rec;
struct cmcmRecord	cmcm_rec;
struct cmemRecord	cmem_rec;
struct cmeqRecord	cmeq_rec;
struct cmhrRecord	cmhr_rec;
struct cmjtRecord	cmjt_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	char	*data  = "data",
			*cmhr2 = "cmhr2",
			*cmjt2 = "cmjt2",
			*cmrh2 = "cmrh2",
			*inmr2 = "inmr2";

	int		envCmAutoCon	= FALSE,
			envCmAutoReq	= FALSE,
			pipeOpen 		= FALSE;

	FILE	*fout;

	char	branchNo 	[3],
			ReqBranchNo [3];

	extern	int		TruePosition;
/*
 * Local & Screen Structures. 
 */
struct {
	char	back 			[2];
	char	backDesc 		[4];
	char	dummy 			[11];
	char	endContDesc 	[71];
	char	endContract 	[7];
	char	endItemDesc 	[41];
	char	endItemNo 		[17];
	char	endJobDesc 		[31];
	char	endJobType 		[5];
	char	endReqDesc 		[41];
	char	onight 			[2];
	char	onightDesc 		[4];
	char	rangeDesc 		[12];
	char	rangeType 		[2];
	char	startContDesc 	[71];
	char	startContract 	[7];
	char	startItemDesc 	[41];
	char	startItemNo 	[17];
	char	startJobDesc 	[31];
	char	startJobType 	[5];
	char	startReqDesc 	[41];
	char	statusCode 		[2];
	char	statusDesc 		[21];
	char	systemDate 		[11];
	int		printerNo;
	long	dateRequired;
	long	endReqNo;
	long	lsystemDate;
	long	startReqNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "rangeType", 	4, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Range Type        ", "Enter Range type. C(ontract), J(ob Type) or R(equisition) or I(tem). ",
		 NE, NO,  JUSTLEFT, "CJRI", "", local_rec.rangeType},
	{1, LIN, "rangeDesc", 	4, 35, CHARTYPE,
		"AAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangeDesc},

	{1, LIN, "startContract", 	6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Contract    ", "Enter Start Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.startContract},
	{1, LIN, "startContDesc", 	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startContDesc},
	{1, LIN, "endContract", 	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End  Contract     ", "Enter End Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endContract},
	{1, LIN, "endContDesc", 	7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endContDesc},

	{1, LIN, "startJobType", 	6, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Start Job Type    ", "Enter Start Job Type . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startJobType},
	{1, LIN, "startJobDesc",6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startJobDesc},
	{1, LIN, "endJobType", 	7, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "End Job Type      ", "Enter End Job Type . ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endJobType},
	{1, LIN, "endJobDesc",7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endJobDesc},

	{1, LIN, "startReqNo", 	6, 2, LONGTYPE,
		"NNNNNN", "          ",
		"0", " ", "Start Requisition ", "Enter Start Requisition . ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.startReqNo},
	{1, LIN, "startReqDesc",	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startReqDesc},
	{1, LIN, "endReqNo", 	7, 2, LONGTYPE,
		"NNNNNN", "          ",
		"0", " ", "End Requisition   ", "Enter End Requisition . ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.endReqNo},
	{1, LIN, "endReqDesc",	6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endReqDesc},

	{1, LIN, "startItemNo", 	6, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item        ", "Enter Start Item.      ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startItemNo},
	{1, LIN, "startItemDesc",6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItemNo", 	7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End Item          ", "Enter End Item.      ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endItemNo},
	{1, LIN, "endItemDesc",7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},

	{1, LIN, "dateRequired", 	 9, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Date Required     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.dateRequired},
	{1, LIN, "stat", 	 10, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Status            ", "Enter Status To Report. A(ll) or B(ackordered) or F(orward) or R(eady). ",
		YES, NO, JUSTLEFT, "ABFR", "", local_rec.statusCode},
	{1, LIN, "statusDesc",  10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.statusDesc},

	{1, LIN, "printerNo", 	 12, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No        ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo},
	{1, LIN, "back", 	 13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background        ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",  13, 35, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight", 	 14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight         ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",  14, 35, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local function prototypes 
 */
void	OpenDB			(void);
void	CloseDB			(void);
void	shutdown_prog	(void);
void	RunProgram		(char *, char *);
void	SrchCmhr		(char *);
void	SrchCmjt		(char *);
void	SrchCmrh		(char *);
void	Process			(void);
void	PrintDetails	(void);
void	HeadingOutput	(void);
int		heading			(int);

/*
 * Main Processing Routine . 
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	if (argc != 2 && argc != 7)
	{
		print_at (0,0, ML (mlCmMess700), argv [0]);
		print_at (1,0, ML (mlCmMess724), argv [0]); 
		print_at (2,0, ML (mlCmMess725)); 
		print_at (3,0, ML (mlCmMess726)); 

		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;
	
	/*
	 * Check environment. 
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	sptr = chk_env ("CM_AUTO_REQ");
	envCmAutoReq = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	OpenDB ();

	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);
	strcpy (ReqBranchNo, (envCmAutoReq == COMPANY) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	if (argc == 7)
	{
		local_rec.printerNo = atoi (argv [1]);
		sprintf (local_rec.statusCode,    "%-1.1s", argv [2]);
		local_rec.dateRequired = StringToDate (argv [3]);
		sprintf (local_rec.rangeType, "%-1.1s", argv [4]);
	
		switch (local_rec.rangeType [0])
		{
		case 'C':
			sprintf (local_rec.startContract,"%-6.6s", argv [5]);
			sprintf (local_rec.endContract,	 "%-6.6s", argv [6]);
			break;

		case 'J':
			sprintf (local_rec.startJobType,"%-4.4s", argv [5]);
			sprintf (local_rec.endJobType,	"%-4.4s", argv [6]);
			break;

		case 'R':
			local_rec.startReqNo	= atol (argv [5]);
			local_rec.endReqNo 		= atol (argv [6]);
			break;

		case 'I':
			sprintf (local_rec.startItemNo,	"%-16.16s", argv [5]);
			sprintf (local_rec.endItemNo, 	"%-16.16s", argv [6]);
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

	init_scr 	();
	set_tty 	(); 
	set_masks 	();
	init_vars 	(1);

	while (!prog_exit)
	{
		/*
		 * Reset control flags 
		 */
   		entry_exit	= FALSE;
   		edit_exit	= FALSE;
   		prog_exit	= FALSE;
   		restart		= FALSE;
		search_ok	= TRUE;

		init_vars (1);	
		FLD ("rangeType") = NE;
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

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cmhr2, cmhr);
	abc_alias (cmjt2, cmjt);
	abc_alias (cmrh2, cmrh);
	abc_alias (inmr2, inmr);

	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmjt,  cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmrd,  cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (cmrh,  cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");
	open_rec (cmrh2, cmrh_list, CMRH_NO_FIELDS, "cmrh_hhrq_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cmrd);
	abc_fclose (cmrh);
	abc_fclose (cmrh2);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	SearchFindClose ();
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
	char	lpstr [10];
	char	lower [17];
	char	upper [17];
	char	dateString [11];

	if (local_rec.onight [0] == 'Y' || local_rec.back [0] == 'Y')
	{
		CloseDB (); 
		FinishProgram ();
	}

	sprintf (lpstr, "%d", local_rec.printerNo);

	sprintf (dateString, "%-10.10s", DateToString (local_rec.dateRequired));

	switch (local_rec.rangeType [0])
	{
	case 'C':
		sprintf (lower, "%-6.6s", local_rec.startContract);
		sprintf (upper, "%-6.6s", local_rec.endContract);
		break;

	case 'J':
		sprintf (lower, "%-4.4s", local_rec.startJobType);
		sprintf (upper, "%-4.4s", local_rec.endJobType);
		break;

	case 'R':
		sprintf (lower, "%06ld", local_rec.startReqNo);
		sprintf (upper, "%06ld", local_rec.endReqNo);
		break;

	case 'I':
		sprintf (lower, "%-16.16s", local_rec.startItemNo);
		sprintf (upper, "%-16.16s", local_rec.endItemNo);
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
			local_rec.statusCode,
			DateToString (local_rec.dateRequired),
			local_rec.rangeType,
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
			local_rec.statusCode,
			DateToString (local_rec.dateRequired),
			local_rec.rangeType,
			lower,
			upper
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);

	}
}

int
spec_valid (
	int		field)
{
	int	i;

	if (LCHECK ("rangeType"))
	{
		/*
		 * Disable range prompts. 
		 */
		for (i = label ("startContract"); i <= label ("endItemNo"); i++)
			vars [i].required = ND;

		switch (local_rec.rangeType [0])
		{
		case 'C':
			FLD ("startContract") 	= YES;
			FLD ("startContDesc") 	= NA;
			FLD ("endContract") 	= YES;
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

		case 'R':
			FLD ("startReqNo") 		= YES;
			FLD ("startReqDesc") 	= NA;
			FLD ("endReqNo") 		= YES;
			FLD ("endReqDesc") 		= NA;
			strcpy (local_rec.rangeDesc, ML ("Requisition"));
			break;

		case 'I':
			FLD ("startItemNo") 	= YES;
			FLD ("startItemDesc") 	= NA;
			FLD ("endItemNo") 		= YES;
			FLD ("endItemDesc") 	= NA;
			strcpy (local_rec.rangeDesc, ML ("Item"));
			break;
		}
	
		scn_write (1);
		DSP_FLD ("rangeDesc");
		FLD ("rangeType") 	= NA;

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
			strcpy (local_rec.startContDesc, ML ("First Contract"));
			DSP_FLD ("startContract");
			DSP_FLD ("startContDesc");
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
			print_mess (ML (mlCmMess006));
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
		strcpy (local_rec.startContDesc, cmcd_rec.text);
		DSP_FLD ("startContDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endContract"))
	{
		if (FLD ("endContract") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endContract, "~~~~~~");
			strcpy (local_rec.endContDesc, ML ("Last Contract"));
			DSP_FLD ("endContract");
			DSP_FLD ("endContDesc");
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
			print_mess (ML (mlCmMess006));
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
			print_mess (ML (mlStdMess088));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endJobDesc, cmjt_rec.desc);
		DSP_FLD ("endJobDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Requisition Range. 
	 */
	if (LCHECK ("startReqNo")) 
	{
		if (FLD ("startReqNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.startReqNo = 0L;
			DSP_FLD ("startReqNo");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Lookup requisition 
		 */
		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, ReqBranchNo);
		cmrh_rec.req_no = local_rec.startReqNo;
		cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endReqNo")) 
	{
		if (FLD ("endReqNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.endReqNo = 999999L;
			DSP_FLD ("endReqNo");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Lookup requisition 
		 */
		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, ReqBranchNo);
		cmrh_rec.req_no = local_rec.endReqNo;
		cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Range. 
	 */
	if (LCHECK ("startItemNo"))
	{
		if (FLD ("startItemNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startItemNo, "                ");
			strcpy (local_rec.startItemDesc, ML ("First Item"));

			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.startItemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.startItemDesc, inmr_rec.description);
		DSP_FLD ("startItemDesc");
		
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endItemNo"))
	{
		if (FLD ("endItemNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endItemNo, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.endItemDesc, ML ("Last Item"));

			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.endItemDesc, inmr_rec.description);
		DSP_FLD ("endItemDesc");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dateRequired"))
	{
		if (dflt_used)
			local_rec.dateRequired = local_rec.lsystemDate;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("stat"))
	{
		switch (local_rec.statusCode [0])
		{
		case 'R':
			strcpy (local_rec.statusDesc, ML ("Ready"));
			break;

		case 'F':
			strcpy (local_rec.statusDesc, ML ("Forward"));
			break;

		case 'B':
			strcpy (local_rec.statusDesc, ML ("Backordered"));
			break;

		case 'A':
		default :
			strcpy (local_rec.statusDesc, ML ("All"));
			break;
		}
	
		DSP_FLD ("statusDesc");

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
	_work_open (6,0,40);
	save_rec ("#No", "#Customer Order No.");

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
SrchCmjt (
char	*keyValue)
{
	_work_open (4,0,40);
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
SrchCmrh (
	char	*keyValue)
{
	char	req_no [7];
	char	desc [41];

	_work_open (6,0,40);
	save_rec ("#Req No", "#Contract | Requested By ");
	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, ReqBranchNo);
	cmrh_rec.req_no = atol (keyValue);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmrh_rec.br_no, ReqBranchNo) &&
	       !strcmp (cmrh_rec.co_no, comm_rec.co_no))
	{
		if (cmrh_rec.stat_flag [0] == 'C')
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (req_no, "%06ld", cmrh_rec.req_no);
			sprintf (desc, "%-6.6s | %-20.20s", 
							cmhr_rec.cont_no, cmrh_rec.req_by);
			cc = save_rec (req_no, desc);
			if (cc)
				break;
		}

		cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, ReqBranchNo);
	cmrh_rec.req_no = atol (temp_str);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	if (cc)
		file_err (cc, cmrh, "DBFIND");
}

void
Process (void)
{
	char	req_str [7];

	dsp_screen ("Processing Requisitions", 
		   comm_rec.co_no, 
		   comm_rec.co_name);
	
	/*
	 * Read Requisition records.
	 */
	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, ReqBranchNo);
	if (PROC_REQ)
		cmrh_rec.req_no = local_rec.startReqNo;
	else
		cmrh_rec.req_no = 0L;
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cmrh_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmrh_rec.br_no, ReqBranchNo))
	{
		/*
		 * Check Date. 
		 */
		if (cmrh_rec.rqrd_date > local_rec.dateRequired)
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check range for PROC_REQ. 
		 */
		if (PROC_REQ && cmrh_rec.req_no > local_rec.endReqNo)
			break;

		/*
		 * Lookup contract for requisition. 
		 */
		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}
		/*
		 * Valid for contract range. 
		 */
		if (PROC_CNT && 
		   (strcmp (cmhr_rec.cont_no, local_rec.startContract) < 0 ||
		     strcmp (cmhr_rec.cont_no, local_rec.endContract) > 0))
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup job type code for contract. 
		 */
		cmjt_rec.hhjt_hash	=	cmhr_rec.hhjt_hash;
		cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}
		/*
		 * Valid for job type range. 
		 */
		if (PROC_JOB && 
		   (strcmp (cmjt_rec.job_type, local_rec.startJobType) < 0 ||
		     strcmp (cmjt_rec.job_type, local_rec.endJobType) > 0))
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup first line of contract description. 
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cmcd_rec.text, "%-70.70s", " NOT FOUND ");
		
		sprintf (req_str, "%06ld", cmrh_rec.req_no);
		dsp_process ("Requisition : ", req_str);

		/*
		 * Print Requisition Details. 
		 */
		PrintDetails ();

		cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
	}

	if (pipeOpen)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
		pipeOpen = FALSE;
	}
}

void
PrintDetails (void)
{
	int		firstLine	=	0;
	float	totalQty	= 	0.00;

	/*
	 * Read Requisition Detail Records. 
	 */
	firstLine = TRUE;
	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
		/*
		 * Ignore completed lines. 
		 */
		if (cmrd_rec.stat_flag [0] == 'C')
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		/*
		 * Ignore completed lines. 
		 */
		if (local_rec.statusCode [0] != 'A' &&
		    cmrd_rec.stat_flag [0] != local_rec.statusCode [0])
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup item for line. 
		 */
		inmr_rec.hhbr_hash = cmrd_rec.hhbr_hash;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}
		if (PROC_JOB && 
		   (strcmp (inmr_rec.item_no, local_rec.startItemNo) < 0 ||
		     strcmp (inmr_rec.item_no, local_rec.endItemNo) > 0))
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Lookup costhead code for line. 
		 */
		cmcm_rec.hhcm_hash = cmrd_rec.hhcm_hash;
		cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		totalQty = cmrd_rec.qty_order + cmrd_rec.qty_border + cmrd_rec.qty_iss;

		if (!pipeOpen)
			HeadingOutput ();

		fprintf (fout, ".LRP2\n");
		if (firstLine)
		{
			fprintf (fout, "|%06ld",    cmrh_rec.req_no);
			fprintf (fout, "|%-10.10s", DateToString (cmrh_rec.rqrd_date));
			fprintf (fout, "|%-6.6s",   cmhr_rec.cont_no);
			fprintf (fout, "|%-40.40s", cmcd_rec.text);
		}
		else
		{
			fprintf (fout, "|%-6.6s",   " ");
			fprintf (fout, "|%-10.10s", " ");
			fprintf (fout, "|%-6.6s",   " ");
			fprintf (fout, "|%-40.40s", " ");
		}
		fprintf (fout, "|%-4.4s",   cmcm_rec.ch_code);
		fprintf (fout, "|%-16.16s", inmr_rec.item_no);
		fprintf (fout, "|%-38.38s", inmr_rec.description);
		fprintf (fout, "|%9.2f",    totalQty);
		fprintf (fout, "|%9.2f",   cmrd_rec.qty_order + cmrd_rec.qty_border);
		fprintf (fout, "|%9.2f",    cmrd_rec.qty_border);
		fprintf (fout, "|%-1.1s|\n",cmrd_rec.stat_flag);

		firstLine = FALSE;

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
	}

	/*
	 * Blank line. 
	 */
	if (!firstLine)
	{
		fprintf (fout, "|%-6.6s",      " ");
		fprintf (fout, "|%-10.10s",      " ");
		fprintf (fout, "|%-6.6s",      " ");
		fprintf (fout, "|%-40.40s",    " ");
		fprintf (fout, "|%-4.4s",      " ");
		fprintf (fout, "|%-16.16s",    " ");
		fprintf (fout, "|%-38.38s",    " ");
		fprintf (fout, "|%-9.9s",      " ");
		fprintf (fout, "|%-9.9s",      " ");
		fprintf (fout, "|%-9.9s",      " ");
		fprintf (fout, "|%-1.1s|\n",   " ");
	}
}

void
HeadingOutput (void)
{
	char	statusDesc [51];

	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During (POPEN)",errno, PNAME);

	sprintf (err_str, "%s<%s>", local_rec.systemDate, PNAME);
	fprintf (fout, ".START%s\n", clip (err_str));
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	switch (local_rec.statusCode [0])
	{
	case 'A':
		sprintf (statusDesc, ML ("OF ANY STATUS"));
		break;
	case 'F':
		sprintf (statusDesc, ML ("FORWARD"));
		break;
	case 'B':
		sprintf (statusDesc, ML ("BACKORDERED"));
		break;
	case 'R':
		sprintf (statusDesc, ML ("READY FOR DESPATCH"));
		break;
	}

	fprintf (fout, ".E %s (%s) \n",
					ML ("OUTSTANDING CONTRACT REQUISITIONS"), statusDesc);
	fprintf (fout, ".E Company : %2.2s - %s \n", 
					comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E AS AT : %-24.24s \n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "=================");
	fprintf (fout, "=======================================");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "==\n");

	fprintf (fout, "| REQ. ");
	fprintf (fout, "| REQUIR'D ");
	fprintf (fout, "|CONT. ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "|COST");
	fprintf (fout, "|     ITEM       ");
	fprintf (fout, "|                                      ");
	fprintf (fout, "|QUANTITY ");
	fprintf (fout, "|QUANTITY ");
	fprintf (fout, "|QUANTITY ");
	fprintf (fout, "|S|\n");

	fprintf (fout, "|NUMBER");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|NUMBER");
	fprintf (fout, "|         CONTRACT DESCRIPTION           ");
	fprintf (fout, "|HEAD");
	fprintf (fout, "|    NUMBER      ");
	fprintf (fout, "|           ITEM DESCRIPTION           ");
	fprintf (fout, "| ORDERED ");
	fprintf (fout, "|OUTSTAND.");
	fprintf (fout, "|B/ORDERED");
	fprintf (fout, "|T|\n");

	fprintf (fout, "|------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|----");
	fprintf (fout, "|----------------");
	fprintf (fout, "|--------------------------------------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------");
	fprintf (fout, "|---------");
	fprintf (fout, "|-|\n");

	fprintf (fout, ".R=======");
	fprintf (fout, "===========");
	fprintf (fout, "=======");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "=================");
	fprintf (fout, "=======================================");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "==\n");

	pipeOpen = TRUE;
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	swide ();
	
	rv_pr (ML (mlCmMess114), ((132 - strlen (ML (mlCmMess114))) / 2), 0, 1);

	box (0, 3, 132, 11);
	line_at (1, 0, 132);
	line_at (5, 1, 131);
	line_at (8, 1, 131);
	line_at (11,1, 131);

	line_cnt = 0;
	scn_write (scn);

	line_at (21,0, 131);

	print_at (22,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}

