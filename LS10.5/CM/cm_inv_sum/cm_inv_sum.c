/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_inv_sum.c,v 5.4 2002/07/17 09:57:01 scott Exp $
|  Program Name  : (cm_inv_sum.c)
|  Program Desc  : (Contract Invoices Summary Report)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : (13/04/93)       |
|---------------------------------------------------------------------|
| $Log: cm_inv_sum.c,v $
| Revision 5.4  2002/07/17 09:57:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/01/23 02:06:11  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_inv_sum.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_inv_sum/cm_inv_sum.c,v 5.4 2002/07/17 09:57:01 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>
#include <arralloc.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#include	"schema"

struct commRecord	comm_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmjtRecord	cmjt_rec;
struct cmpbRecord	cmpb_rec;
struct cohrRecord	cohr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;

	char	*data  = "data",
	    	*cmhr2 = "cmhr2",
	    	*cmjt2 = "cmjt2";

	int		envCmAutoCon	= 0;

	FILE	*fout;

	char	branchNo [3];
		
	extern	int		TruePosition;

/*
 *	Structure for dynamic array.
 */
struct ContStruct
{
	char	sortCode [16];	/* Invoice No, Contract No, Invoice Type 	*/
	char	invoiceNo		[sizeof cohr_rec.inv_no]; 
	char	contractNo		[sizeof cmhr_rec.cont_no];
	char	invoiceType		[sizeof cohr_rec.type];
	long	hhhrHash; 
	long	hhcuHash; 
	long	hhcuHoHash; 
	long	dateRaised; 
	double	quoteValue;
	double	invoiceValue;
}	*contRec;
	DArray cont_details;

int		contCnt		=	0;
int		ContSort			(const	void *,	const void *);

/*
 * Local & Screen Structures. 
 */
struct {
	int		printerNo;
	long	lsystemDate;
	long	startDate;
	long	endDate;
	char	systemDate 		[11];
	char	startContNo 	[7];
	char	startContDesc 	[71];
	char	endContNo 		[7];
	char	endContDesc 	[71];
	char	startJobType 	[5];
	char	startJobDesc 	[31];
	char	endJobType 		[5];
	char	endJobDesc 		[31];
	char	onight 			[2];
	char	onightDesc 		[4];
	char	back 			[2];
	char	backDesc 		[4];
	char	dummy 			[11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startDate", 	4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date       ", "Enter Start Transaction Date. ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.startDate},
	{1, LIN, "endDate", 	5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "End Date         ", "Enter End Transaction Date. Default = Today ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.endDate},
	{1, LIN, "startContNo", 	7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Contract   ", "Enter Start Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.startContNo},
	{1, LIN, "startContDesc", 	7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startContDesc},
	{1, LIN, "endContNo", 	8, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End  Contract    ", "Enter End Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endContNo},
	{1, LIN, "endContDesc", 	8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endContDesc},
	{1, LIN, "startJobType", 	10, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Start Job Type   ", "Enter Start Job Type . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.startJobType},
	{1, LIN, "startJobDesc",10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startJobDesc},
	{1, LIN, "endJobType", 	11, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "End Job Type     ", "Enter End Job Type . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endJobType},
	{1, LIN, "endJobDesc",11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endJobDesc},

	{1, LIN, "printerNo", 	 13, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo},
	{1, LIN, "back", 	 14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",     14, 35, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight", 	 15, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",  15, 35, CHARTYPE,
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
void	Process			(void);
void	ProcessSorted	(void);
void	HeadingOutput	(void);
int		spec_valid		(int);
int		heading			(int);


/*
 * Main Processing Routine . 
 */
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	TruePosition	=	TRUE;

	if (argc != 2 && argc != 8)
	{
		print_at (0,0, ML (mlCmMess700), argv [0]);
		print_at (1,0, ML (mlCmMess720), argv [0]);
		print_at (2,0, ML (mlCmMess721));
		print_at (3,0, ML (mlCmMess722), argv [0]);

		return (argc);
	}

	/*
	 * Check environment. 
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	/*
	 * Read common terminal record. 
	 */
	OpenDB ();

	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();
	
	if (argc == 8)
	{
		local_rec.printerNo = atoi (argv [1]);
		local_rec.startDate = StringToDate (argv [2]);
		local_rec.endDate 	= StringToDate (argv [3]);
		sprintf (local_rec.startContNo,	"%-6.6s", argv [4]);
		sprintf (local_rec.endContNo,	"%-6.6s", argv [5]);
		sprintf (local_rec.startJobType,"%-4.4s", argv [6]);
		sprintf (local_rec.endJobType, 	"%-4.4s", argv [7]);

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
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		restart 	= FALSE;
		search_ok 	= TRUE;

		init_vars 	(1);	
		heading 	(1);
		entry 		(1);
		if (prog_exit || restart)
			continue;

		heading 	(1);
		scn_display (1);
		edit 		(1);
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

	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmjt,  cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmpb,  cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*
 * Close data base files . 
 */
void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cmpb);
	abc_fclose (cohr);
	abc_fclose (cumr);
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
	char	startDate	[11],
			endDate 	[11];

	sprintf (startDate, "%s", DateToString (local_rec.startDate));
	sprintf (endDate, 	"%s", DateToString (local_rec.endDate));

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT %s \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			programName,
			local_rec.printerNo,
			startDate,
			endDate,
			local_rec.startContNo,
			local_rec.endContNo,
			local_rec.startJobType,
			local_rec.endJobType,
			programDesc
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			programName,
			local_rec.printerNo,
			startDate,
			endDate,
			local_rec.startContNo,
			local_rec.endContNo,
			local_rec.startJobType,
			local_rec.endJobType
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

int
spec_valid (
	int		field)
{
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
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Check against start contract no. 
		 */
		if (strcmp (local_rec.startContNo, local_rec.endContNo) > 0)
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
			print_mess (ML (mlCmMess011));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startJobDesc, "%-30.30s", cmjt_rec.desc);
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

		sprintf (local_rec.endJobDesc, "%-30.30s", cmjt_rec.desc);
		DSP_FLD ("endJobDesc");

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
Process (void)
{
	dsp_screen ("Processing Contracts", comm_rec.co_no, comm_rec.co_name);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&cont_details, &contRec, sizeof (struct ContStruct), 10);
	contCnt = 0;

	cmpb_rec.hhhr_hash  = 0L;
	cc = find_rec (cmpb, &cmpb_rec, GTEQ, "r");
	while (!cc)
	{
		/*
		 * Read contract record. 
		 */
		cmhr_rec.hhhr_hash = cmpb_rec.hhhr_hash;
		cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check contract company. 
		 */
		if (strcmp (cmhr_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check contract range. 
		 */
		if (strcmp (cmhr_rec.cont_no, local_rec.startContNo) < 0 ||
		     strcmp (cmhr_rec.cont_no, local_rec.endContNo) > 0)
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check job type range. 
		 */
		cmjt_rec.hhjt_hash = cmhr_rec.hhjt_hash;
		cc = find_rec (cmjt2, &cmjt_rec, COMPARISON, "r");
		if (cc ||
		   (!cc && (strcmp (cmjt_rec.job_type,local_rec.startJobType) < 0 ||
		             strcmp (cmjt_rec.job_type,local_rec.endJobType) >0)))
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check date range. 
		 */
		cohr_rec.hhco_hash = cmpb_rec.hhco_hash;
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
		if (cc ||
		   (!cc && (cohr_rec.date_raised < local_rec.startDate ||
		            cohr_rec.date_raised > local_rec.endDate)))
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}

		/*
		 * Find customer record. 
		 */
		cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Contract : ", cmhr_rec.cont_no);

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&cont_details, contRec, contCnt))
			sys_err ("ArrChkLimit (contRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element contCnt.
		 */
		sprintf (contRec [contCnt].sortCode, "%-8.8s%-6.6s%1.1s",
						cohr_rec.inv_no, cmhr_rec.cont_no, cohr_rec.type);

		strcpy (contRec [contCnt].invoiceNo, cohr_rec.inv_no);
		strcpy (contRec [contCnt].contractNo, cmhr_rec.cont_no);
		strcpy (contRec [contCnt].invoiceType, cohr_rec.type);
		contRec [contCnt].hhhrHash		= cmhr_rec.hhhr_hash;
		contRec [contCnt].hhcuHash		= cumr_rec.hhcu_hash;
		contRec [contCnt].hhcuHoHash	= cumr_rec.ho_dbt_hash;
		contRec [contCnt].dateRaised	= cohr_rec.date_raised;
		contRec [contCnt].quoteValue	= cmhr_rec.quote_val;
		contRec [contCnt].invoiceValue	= cmpb_rec.amount;
		/*
		 * Increment array counter.
		 */
		contCnt++;
		cc = find_rec (cmpb, &cmpb_rec, NEXT, "r");
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (contRec, contCnt, sizeof (struct ContStruct), ContSort);

	/*
	 * Process sort file. 
	 */
	ProcessSorted ();
	
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&cont_details);

	return;
}

void
ProcessSorted (void)
{
	long	hhhrHash;
	long	hhcuHash;
	long	hhcuHoHash;
	double	quoteValue;
	double	invoiceValue;
	double	marginPc;
	char	marginString [8];
	char	invTypeStr [5];
	char	dateString [11];
	int		processCnt	=	0;

	HeadingOutput ();

	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (processCnt = 0; processCnt < contCnt; processCnt++)
	{
		hhhrHash		=	contRec [processCnt].hhhrHash;
		hhcuHash		=	contRec [processCnt].hhcuHash;
		hhcuHoHash		=	contRec [processCnt].hhcuHoHash;
		quoteValue		=	contRec [processCnt].quoteValue;
		invoiceValue	=	contRec [processCnt].invoiceValue;
	
		if (invoiceValue == 0.00)
			strcpy (marginString, "*******");
		else
		{
			marginPc = (invoiceValue - quoteValue) / invoiceValue;
			marginPc *= 100.00;

			sprintf (marginString, "%7.2f", marginPc);
		}
		if (contRec [processCnt].invoiceType[0] == 'I')
			strcpy (invTypeStr, "INV");
		else
			strcpy (invTypeStr, "CRD");

		strcpy (dateString, DateToString (contRec [processCnt].dateRaised));
		/*
		 * Find contract description. 
		 */
		cmcd_rec.hhhr_hash = hhhrHash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cmcd_rec.text, "%-70.70s", " ");

		/*
		 * Find customer name. 
		 */
		cumr_rec.hhcu_hash = hhcuHash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no,  " ");
			strcpy (cumr_rec.dbt_name," ");
		}

		/*
		 * Find charge customer name. 
		 */
		cumr2_rec.hhcu_hash = hhcuHoHash;
		cc = find_rec (cumr, &cumr2_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr2_rec.dbt_no,  " ");
			strcpy (cumr2_rec.dbt_name," ");
		}

		fprintf (fout, ".LRP3\n");

		fprintf (fout, "|%-8.8s",		contRec [processCnt].invoiceNo);
		fprintf (fout, "| %-3.3s ", 	invTypeStr);
		fprintf (fout, "| %-6.6s ", 	contRec [processCnt].contractNo);
		fprintf (fout, "| %-70.70s ", 	cmcd_rec.text);
		fprintf (fout, "|%s",     		dateString);
		fprintf (fout, "|%12.2f",     	DOLLARS (quoteValue));
		fprintf (fout, "|%12.2f",     	DOLLARS (invoiceValue));
		fprintf (fout, "| %7.7s |\n", 	marginString);

		fprintf (fout, "|    Customer : %-6.6s %-40.40s ",
							cumr_rec.dbt_no, cumr_rec.dbt_name);
		fprintf (fout, "    Charge To : %-6.6s %-40.40s               |\n",
							cumr2_rec.dbt_no, cumr2_rec.dbt_name);

		fprintf (fout, "|%140.140s |\n", " ");
	}

	fprintf (fout, ".EOF\n");
	fclose (fout);
}

void
HeadingOutput (void)
{
	char	startDate	[11],
			endDate		[11];

	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During (POPEN)",errno, PNAME);

	sprintf (err_str, "%s<%s>", local_rec.systemDate, PNAME);
	fprintf (fout, ".START%s\n", clip (err_str));
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".EINVOICE SUMMARY REPORT\n");
	fprintf (fout, ".ECOMPANY : %s - %s\n", comm_rec.co_no, comm_rec.co_name);
	fprintf (fout, ".EAs At %s \n", SystemTime ());

	strcpy (startDate, DateToString (local_rec.startDate));
	strcpy (endDate,   DateToString (local_rec.endDate));
	fprintf (fout, ".EFor The Period %s to %s\n", startDate, endDate);
	fprintf (fout, ".B1\n");

	fprintf (fout, "=========");
	fprintf (fout, "======");
	fprintf (fout, "=========");
	fprintf (fout, "=========================================================================");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===========\n");

	fprintf (fout, "|DOCUMENT");
	fprintf (fout, "|     ");
	fprintf (fout, "|CONTRACT");
	fprintf (fout, "|                                                                        ");
	fprintf (fout, "| DOCUMENT ");
	fprintf (fout, "|   QUOTED   ");
	fprintf (fout, "|  INVOICE   ");
	fprintf (fout, "| MARGIN  |\n");

	fprintf (fout, "| NUMBER ");
	fprintf (fout, "|TYPE ");
	fprintf (fout, "| NUMBER ");
	fprintf (fout, "|                    CONTRACT DESCRIPTION                                ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   VALUE    ");
	fprintf (fout, "|   VALUE    ");
	fprintf (fout, "| PERCENT |\n");

	fprintf (fout, "|--------");
	fprintf (fout, "|-----");
	fprintf (fout, "|--------");
	fprintf (fout, "|------------------------------------------------------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|---------|\n");

	fprintf (fout, ".R=========");
	fprintf (fout, "======");
	fprintf (fout, "=========");
	fprintf (fout, "=========================================================================");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===========\n");

	return;
}

int 
ContSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct ContStruct a = * (const struct ContStruct *) a1;
	const struct ContStruct b = * (const struct ContStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                         
 */
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		swide ();
		centre_at (0, 132, ML (mlCmMess053));
		box (0, 3, 132, 12);

		line_at (1, 0, 132);
		line_at (6, 1, 131);
		line_at (9, 1, 131);
		line_at (12, 1, 131);

		line_cnt = 0;
		scn_write (scn);
	}
	line_at (21, 0, 131);
	
	print_at (22,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}
