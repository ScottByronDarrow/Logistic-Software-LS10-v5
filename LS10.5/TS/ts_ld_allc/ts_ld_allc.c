/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ts_ld_allc.c,v 5.4 2002/03/01 02:07:10 scott Exp $
|  Program Name  : (tm_ld_allc.c)
|  Program Desc  : (Allocate/Deallocate leads to/from operators)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 05/12/91         |
|---------------------------------------------------------------------|
| $Log: ts_ld_allc.c,v $
| Revision 5.4  2002/03/01 02:07:10  scott
| S/C 00604 - TSMR5-Tele-Sales Lead Allocation-Assign Bulk Leads -- END fields accept inputs less than the Start fields.
|
| Revision 5.3  2001/09/20 05:10:18  robert
| Updated to fix on default value checking and follow the standard
| format for spec_valid () validation
|
| Revision 5.2  2001/08/09 09:23:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:55:59  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/28 04:33:09  scott
| Updated to remove absurd pointer array and replaced with a KISS alternative
| that works and work reliably.
|
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_ld_allc.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_ld_allc/ts_ld_allc.c,v 5.4 2002/03/01 02:07:10 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_ts_mess.h>
#include <arralloc.h>

#define	RESTRT		-1
#define	MENU_EXIT	99
#define	BULK_ASS	0
#define	BULK_UNASS	1
#define	MAN_ASS		2
#define	EXIT_RING	3

#define	OP_NUM		 (prospect [i].opNumber)

#define ALLC_ALL	 (!strncmp (local_rec.noAllocated, "ALL", 3))
#define ALL_SECTS	 (!strncmp (local_rec.startSector, "   ", 3) && \
					  !strncmp (local_rec.endSector,   "~~~", 3))
#define ALL_AREAS	 (!strncmp (local_rec.startArea,   "  ", 2) && \
					  !strncmp (local_rec.startArea,   "~~", 2))

#define	LAST_OP		 (local_rec.lastOperator [0] == 'Y')
#define	IGNORE_PH	 (local_rec.nxt_ph == 0L)
#define LETTER_SEL	 (strlen (clip (local_rec.letterCode)) != 0)

extern	int	TruePosition;
extern	int	EnvScreenOK;

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int  	envVarDbCo 			= 0,
			envVarDbFind 		= 0,
			clearOK 			= TRUE,
			envVarDbCurr		= 0,
			envVarTsPhoneDate	= 0,
			updateType			= 0;

	char	branchNumber 	[3],
			envVarCurrCode 	[4];

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct tspmRecord	tspm_rec;
struct tspmRecord	tspm2_rec;
struct tsalRecord	tsal_rec;
struct tsalRecord	tsal2_rec;
struct tslhRecord	tslh_rec;
struct tslsRecord	tsls_rec;
struct exclRecord	excl_rec;
struct exafRecord	exaf_rec;

	/*-------------------------------
	| Set up pointers to file names |
	-------------------------------*/
	char 	*cumr2   = "cumr2",
	    	*tsal2    = "tsal2",
	    	*tspm2    = "tspm2";

	char	systemDate [11];
	long	lsystemDate;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy 				[11],
			manLastOp 			[15],
			manLastOpName 		[41],
			manOperator 		[15],
			manOperatorName 	[41],
			startOperator 		[15],
			startOperatorName 	[41],
			endOperator 		[15],
			endOperatorName 	[41],
			startCust 			[7],
			startCustName 		[41],
			endCust 			[7],
			endCustName 		[41],
			startSector 		[4],
			startSectorName 	[41],
			endSector 			[4],
			endSectorName 		[41],
			startArea 			[3],
			startAreaName 		[41],
			endArea 			[3],
			endAreaName 		[41],
			lastOperator 		[2],
			lastOperatorDesc	[11],
			letterCode 			[11],
			letterDesc 			[41],
			noAllocated			[7];
	long	nxt_ph;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "startOperator", 3, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", " ", "Start Operator      ", "Enter Start Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.startOperator}, 
	{1, LIN, "startOperatorName", 3, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startOperatorName}, 
	{1, LIN, "endOperator", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", " ", "End Operator        ", "Enter End Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.endOperator}, 
	{1, LIN, "endOperatorName", 4, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endOperatorName}, 
	{1, LIN, "startCust", 6, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Lead          ", "Enter Start Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startCust}, 
	{1, LIN, "startCustName", 6, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startCustName}, 
	{1, LIN, "endCust", 7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Lead            ", "Enter End Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endCust}, 
	{1, LIN, "endCustName", 7, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endCustName}, 
	{1, LIN, "startSector", 9, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Start Sector        ", "Enter Start Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startSector}, 
	{1, LIN, "startSectorName", 9, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startSectorName}, 
	{1, LIN, "endSector", 10, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "End Sector          ", "Enter End Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endSector}, 
	{1, LIN, "endSectorName", 10, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endSectorName}, 
	{1, LIN, "startArea", 12, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Start Area          ", "Enter Start Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.startArea}, 
	{1, LIN, "startAreaName", 12, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startAreaName}, 
	{1, LIN, "endArea", 13, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "End Area            ", "Enter End Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.endArea}, 
	{1, LIN, "endAreaName", 13, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endAreaName}, 
	{1, LIN, "lett_code", 15, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ","Letter Code         ", "Enter Name Of Letter To Have Allocated.", 
		NO, NO, JUSTLEFT, "", "", local_rec.letterCode}, 
	{1, LIN, "lett_desc", 15, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.letterDesc}, 
	{1, LIN, "lastOperator", 16, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Last Operator       ", "Allocate as last operator.", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.lastOperator}, 
	{1, LIN, "lastOperatorDesc", 16, 26, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.lastOperatorDesc}, 
	{1, LIN, "nxt_ph", 17, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "",  "Next Phone Date     ", "Enter Next Phone Date. Enter 0 To Ignore This Selection Criteria", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.nxt_ph}, 
	{1, LIN, "noAllocated", 18, 2, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "",  "Number To Allocate  ", "Enter Number Of Leads To Allocate.", 
		NO, NO, JUSTLEFT, "0123456789", "", local_rec.noAllocated}, 
	{2, LIN, "man_lead", 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Lead Number         ", "Enter Lead To Assign.", 
		NE, NO, JUSTLEFT, "", "", local_rec.startCust}, 
	{2, LIN, "man_lead_name", 4, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startCustName}, 
	{2, LIN, "manLastOp", 6, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Last Operator       ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manLastOp}, 
	{2, LIN, "manLastOpName", 6, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manLastOpName}, 
	{2, LIN, "manOperator", 8, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Operator            ", "Enter Operator To Assign Lead To. (Default Deallocates Lead.)", 
		NI, NO, JUSTLEFT, "", "", local_rec.manOperator}, 
	{2, LIN, "manOperatorName", 8, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manOperatorName}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

long	oldOpHash		=	0L,
		newOpHash		=	0L,
		noToAllocate	=	0L,
		opsLeftOver		=	0L;

int		noOfOps			=	0,
		firstTime		=	0;

/*-------------------------------------------------------------------
|	Structure for dynamic array,  for the shipment lines for qsort	|
-------------------------------------------------------------------*/
struct ProspectArray
{
	long	hhcuHash;
	int		opNumber;
}	*prospect;
	DArray prospect_d;
	int	prospectCnt = 0;


struct	{
	char	id [15];
	long	hhopHash;
	long	allc;
	int		currentLine;
} tsop [256];

#include <FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int		BulkLeadUpdate		(void);
int		FindRandomOp		(void);
int		LeadDeallocation	(void);
int		LoadLeads			(void);
int		LoadOperators		(void);
int		ManualLeadUpdate	(void);
int		ProcessLeads		(void);
int		UpdateLead			(void);
int		ValidLead			(void);
int		heading				(int);
int		spec_valid			(int);
void	AssignMenu			(void);
void	CloseDB				(void);
void	OpenDB				(void);
void	SrchExaf			(char *);
void	SrchExcl			(char *);
void	SrchTmop			(char *);
void	SrchTslh			(char *);
void	shutdown_prog		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr = chk_env ("TS_PHONE_DATE");

	if (sptr)
		envVarTsPhoneDate = (*sptr == 'Y' || *sptr == 'y');
	else
		envVarTsPhoneDate = FALSE;

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	/*--------------------------
	| Set position of minimenu |
	--------------------------*/
	MENU_ROW = 2;
	MENU_COL = 0;

	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*-------------------------
	| Multi-currency debtors. |
	-------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbCurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	set_masks ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	OpenDB (); 	
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 	

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		restart = FALSE;

		heading (0);
		clearOK = TRUE;

		AssignMenu ();
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
	abc_dbopen ("data");

	abc_alias (tsal2, tsal);
	abc_alias (tspm2, tspm);
	abc_alias (cumr2, cumr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envVarDbFind) ? "cumr_id_no3"
							    : "cumr_id_no");

	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (tsal, tsal_list, TSAL_NO_FIELDS, "tsal_id_no");
	open_rec (tsal2,tsal_list, TSAL_NO_FIELDS, "tsal_hhcu_hash");
	open_rec (tslh, tslh_list, TSLH_NO_FIELDS, "tslh_id_no");
	open_rec (tsls, tsls_list, TSLS_NO_FIELDS, "tsls_id_no");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tspm, tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
	open_rec (tspm2,tspm_list, TSPM_NO_FIELDS, "tspm_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (tsal);
	abc_fclose (tsal2);
	abc_fclose (tslh);
	abc_fclose (tsls);
	abc_fclose (tspm);
	abc_fclose (tspm2);
	abc_fclose (tmop);
	abc_fclose (excl);
	abc_fclose (exaf);
	abc_dbclose ("data");
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("startOperator"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.startOperator, "              ");
			strcpy (local_rec.startOperatorName, ML ("First Operator"));
			DSP_FLD ("startOperatorName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.startOperator);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startOperatorName, tmop_rec.op_name);
		
		DSP_FLD ("startOperatorName");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("endOperator"))
	{
		if (dflt_used || !strcmp (local_rec.endOperator, "~~~~~~~~~~~~~~"))
		{
			strcpy (local_rec.endOperator, "~~~~~~~~~~~~~~");
			strcpy (local_rec.endOperatorName, ML ("Last Operator"));
			DSP_FLD ("endOperatorName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.startOperator,local_rec.endOperator) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.endOperator);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		strcpy (local_rec.endOperatorName, tmop_rec.op_name);
		DSP_FLD ("endOperatorName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCust"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCust, "      ");
			strcpy (local_rec.startCustName, ML ("First Customer"));
			DSP_FLD ("startCustName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.startCust));
		
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		tspm_rec.hhcu_hash  = cumr_rec.hhcu_hash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
		if (cc)
		{
			print_mess (ML (mlTsMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (cumr_rec.curr_code, envVarCurrCode))
		{
			print_mess (ML (mlTsMess008));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startCustName, cumr_rec.dbt_name);
		DSP_FLD ("startCustName");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endCust"))
	{
		if (dflt_used || !strcmp (local_rec.endCust, "~~~~~~"))
		{
			strcpy (local_rec.endCust, "~~~~~~");
			strcpy (local_rec.endCustName, ML ("Last Customer"));
			DSP_FLD ("endCustName");
			
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);

		strcpy (cumr_rec.dbt_no, pad_num (local_rec.endCust));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startCust,local_rec.endCust) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		tspm_rec.hhcu_hash  = cumr_rec.hhcu_hash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
		if (cc)
		{
			print_mess (ML (mlTsMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (cumr_rec.curr_code, envVarCurrCode))
		{
			print_mess (ML (mlTsMess008));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endCustName, cumr_rec.dbt_name);

		DSP_FLD ("endCustName");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startArea"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startArea, 		"  ");
			strcpy (local_rec.endArea, 			"~~");
			strcpy (local_rec.startAreaName, 	ML ("All Areas"));
			strcpy (local_rec.endAreaName, 		ML ("All Areas"));
			DSP_FLD ("startArea");
			DSP_FLD ("endArea");
			DSP_FLD ("startAreaName");
			DSP_FLD ("endAreaName");
			FLD ("endArea") = NA;
			return (EXIT_SUCCESS);
		}

		FLD ("endArea") = NO;

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code, local_rec.startArea);

		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.startArea, exaf_rec.area_code);
		strcpy (local_rec.startAreaName, exaf_rec.area);

		DSP_FLD ("startAreaName");
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endArea"))
	{
		if (FLD ("endArea") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code, local_rec.endArea);

		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.startArea,local_rec.endArea) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.endArea, exaf_rec.area_code);
		strcpy (local_rec.endAreaName, exaf_rec.area);

		DSP_FLD ("endAreaName");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startSector"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startSector, "   ");
			strcpy (local_rec.endSector,   "~~~");
			strcpy (local_rec.startSectorName, ML ("All Business Sectors"));
			strcpy (local_rec.endSectorName,   ML ("All Business Sectors"));
			DSP_FLD ("startSector");
			DSP_FLD ("endSector");
			DSP_FLD ("startSectorName");
			DSP_FLD ("endSectorName");
			FLD ("endSector") = NA;
			return (EXIT_SUCCESS);
		}
		FLD ("endSector") = NO;
		
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excl_rec.co_no,comm_rec.co_no);		
		strcpy (excl_rec.class_type,local_rec.startSector);

		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess164));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.startSectorName, excl_rec.class_desc);

		DSP_FLD ("startSectorName");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endSector"))
	{
		if (FLD ("endSector") == NA)
			return (EXIT_SUCCESS);
			
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		if (strcmp (local_rec.startSector,local_rec.endSector) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (excl_rec.co_no,comm_rec.co_no);
		strcpy (excl_rec.class_type,local_rec.endSector);
		
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess164));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.endSectorName, excl_rec.class_desc);
		DSP_FLD ("endSectorName");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lett_code"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.letterCode, "%-10.10s", " ");
			sprintf (local_rec.letterDesc, "%-40.40s", " ");
			DSP_FLD ("lett_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTslh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tslh_rec.co_no, comm_rec.co_no);
		sprintf (tslh_rec.let_code, "%-10.10s", local_rec.letterCode);
		cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess109));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*-------------------------------
		| Mailer type must be Follow up |
		-------------------------------*/
		if (tslh_rec.lett_type [0] != 'F')
		{
			print_mess (ML (mlTsMess014));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
		DSP_FLD ("lett_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lastOperator"))
	{
		if (local_rec.lastOperator [0] == 'Y')
			strcpy (local_rec.lastOperatorDesc, ML ("Yes"));
		else
			strcpy (local_rec.lastOperatorDesc, ML ("No "));

		DSP_FLD ("lastOperatorDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nxt_ph"))
	{
		if (dflt_used)
		{
			local_rec.nxt_ph = lsystemDate;
			return (EXIT_SUCCESS);
		}
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("noAllocated"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.noAllocated, "ALL");
			DSP_FLD ("noAllocated");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("man_lead"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}
	
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		sprintf (cumr_rec.dbt_no, "%-6.6s", pad_num (local_rec.startCust));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = find_hash (tspm, &tspm_rec, COMPARISON, "u", cumr_rec.hhcu_hash);
		if (cc)
		{
			print_mess (ML (mlTsMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (cumr_rec.curr_code, envVarCurrCode))
		{
			print_mess (ML (mlTsMess008));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startCustName, "%-40.40s", cumr_rec.dbt_name);
		DSP_FLD ("man_lead_name");

		if (strlen (clip (tspm_rec.lst_op_code)) != 0)
		{
			strcpy (tmop_rec.co_no, comm_rec.co_no);
			sprintf (tmop_rec.op_id, "%-14.14s", tspm_rec.lst_op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (local_rec.manLastOp, "Unknown Op.");
				strcpy (local_rec.manLastOpName, ML ("Unknown Operator"));
			}
			else
			{
				sprintf (local_rec.manLastOp, "%-14.14s", tmop_rec.op_id);

				sprintf (local_rec.manLastOpName, "%-40.40s", tmop_rec.op_name);
			}
		}
	
		oldOpHash = -1L;
		if (strlen (clip (tspm_rec.op_code)) != 0)
		{
			strcpy (tmop_rec.co_no, comm_rec.co_no);
			sprintf (tmop_rec.op_id, "%-14.14s",tspm_rec.op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (local_rec.manOperator, "Unknown Op.");
				strcpy (local_rec.manOperatorName, ML ("Unknown Operator"));
			}
			else
			{
				sprintf (local_rec.manOperator, "%-14.14s", tmop_rec.op_id);
				strcpy (local_rec.manOperatorName, tmop_rec.op_name);
				oldOpHash = tmop_rec.hhop_hash;
				newOpHash = oldOpHash;
			}
			FLD ("manOperator") = NI;
		}
		else
			FLD ("manOperator") = NO;
	
		DSP_FLD ("manLastOp");
		DSP_FLD ("manLastOpName");
		DSP_FLD ("manOperator");
		DSP_FLD ("manOperatorName");

		return (EXIT_SUCCESS);
	}		

	if (LCHECK ("manOperator"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.manOperator, "%-14.14s", " ");
			sprintf (local_rec.manOperatorName, "%-40.40s", " ");
			DSP_FLD ("manOperator");
			DSP_FLD ("manOperatorName");
			newOpHash = -1L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.manOperator);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.manOperatorName, "%-40.40s", tmop_rec.op_name);
		newOpHash = tmop_rec.hhop_hash;
		
		DSP_FLD ("manOperatorName");
		return (EXIT_SUCCESS);
	}		
	return (EXIT_SUCCESS);
}	

MENUTAB ass_menu [] =
{
	{ " 1. ASSIGN BULK LEADS  ",
	  " BULK ASSIGNMENT OF LEADS. " },
	{ " 2. UNASSIGN BULK LEADS ",
	  " UNASSIGN LEADS IN BULK." },
	{ " 3. MANUAL ASSIGNMENT  ",
	  " ASSIGN/UNASSIGN LEADS MANUALLY " },
	{ " 4. EXIT ASSIGNMENT  ",
	  " EXIT ASSIGNMENT " },
	{ ENDMENU }
};

/*===================
| Assign mini menu. |
===================*/
void
AssignMenu (void)
{
	for (;;)
	{
		mmenu_print ("    LEAD ALLOCATION.    ", ass_menu, 0);
		switch (mmenu_select (ass_menu))
		{
		case BULK_ASS :
			updateType = BULK_ASS;
			BulkLeadUpdate ();
			clearOK = TRUE;
			return;

		case BULK_UNASS :
			updateType = BULK_UNASS;
			BulkLeadUpdate ();
			clearOK = TRUE;
			return;

		case MAN_ASS :
			updateType = MAN_ASS;
			ManualLeadUpdate ();
			clearOK = TRUE;
			return;

		case RESTRT :
			restart = TRUE;
			clearOK = FALSE;
			return;
	
		case EXIT_RING :
		case MENU_EXIT :
			prog_exit = TRUE;
			return;
	
		default :
			break;
		}
	}
}

/*------------------------
| Allocate or reallocate |
| a single lead		     |
------------------------*/
int
ManualLeadUpdate (void)
{
	while (!prog_exit)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (2);

		clearOK = TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		newOpHash = -1L;
		heading (2);
		entry (2);
		if (prog_exit || restart)
			continue;

		heading (2);
		scn_display (2);
		edit (2);
		if (restart)
			continue;

		UpdateLead ();
	}

	prog_exit = FALSE;
	return (EXIT_SUCCESS);
}

/*-----------------------
| Bulk allocation or    |
| deallocation of leads |
-----------------------*/
int
BulkLeadUpdate (void)
{
	while (!prog_exit)
	{
		if (updateType == BULK_ASS)
		{
			FLD ("lastOperator") 	= NO;
			FLD ("lastOperatorDesc")= NA;
			FLD ("nxt_ph") 			= NO;
			FLD ("noAllocated") 	= NO;
		}
		else
		{
			FLD ("lastOperator") 	= ND;
			FLD ("lastOperatorDesc")= ND;
			FLD ("nxt_ph") 			= ND;
			FLD ("noAllocated") 	= ND;
		}

		FLD ("endArea") = NO;
		FLD ("endSector") = NO;

		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		clearOK = TRUE;

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

		LoadOperators ();

		firstTime = TRUE;

		/*----------------------------------------
		| Setup Array with 500 entries to start. |
		----------------------------------------*/
		ArrAlloc (&prospect_d, &prospect, sizeof (struct ProspectArray), 500);

		LoadLeads ();

		if (updateType == BULK_ASS)
		{
			if (ALLC_ALL)
			{
				noToAllocate = prospectCnt / (long) noOfOps;
				opsLeftOver = prospectCnt %  (long) noOfOps;
			}
			else
			{
				noToAllocate = atol (local_rec.noAllocated);
				opsLeftOver = 0L;
			}
	
			dsp_screen ("Allocating Leads", comm_rec.co_no, comm_rec.co_name);

			ProcessLeads ();
		}
		else
			LeadDeallocation ();

		/*---------------------------
		| Free up the array memory. |
		---------------------------*/
		ArrDelete (&prospect_d);
	}
	prog_exit = FALSE;

	return (EXIT_SUCCESS);
}

/*------------------
| Deallocate leads |
------------------*/
int
LeadDeallocation (void)
{
	int		i	=	0;
	int		tmpOpNumber;
	long	tmp_hash;
	long	leadHhcuHash;

	dsp_screen ("Deallocating Leads", comm_rec.co_no, comm_rec.co_name);

	for (i = 0; i < prospectCnt; i++)
	{
		tspm_rec.hhcu_hash	= prospect [i].hhcuHash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
		if (cc)
			continue;

		cumr_rec.hhcu_hash	= prospect [i].hhcuHash; 
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
		if (cc)
			continue;

		dsp_process ("Prospect:", cumr_rec.dbt_no);
	
		if (OP_NUM > noOfOps || OP_NUM < 0)
		{
			abc_unlock (tspm);
			continue;
		}

		if (strcmp (tsop [OP_NUM].id, tspm_rec.op_code))
		{
			abc_unlock (tspm);
			continue;
		}

		sprintf (tspm_rec.op_code, "%-14.14s", " ");

		cc = abc_update (tspm, &tspm_rec);
		if (cc)
			file_err (cc, tspm, "DBUPDATE");
			
		tmpOpNumber 	= OP_NUM;
		tmp_hash 	= tsop [tmpOpNumber].hhopHash;
		leadHhcuHash 	= prospect [i].hhcuHash; 

		tsal_rec.hhop_hash 	= tmp_hash;
		tsal_rec.line_no 	= 0;
		cc = find_rec (tsal, &tsal_rec, GTEQ, "u");
		while (!cc && tsal_rec.hhop_hash == tmp_hash)
		{
			if (tsal_rec.hhcu_hash == leadHhcuHash)
			{
				abc_delete (tsal);
				break;
			}
			else
				abc_unlock ("tsal");

			cc = find_rec (tsal, &tsal_rec, NEXT, "u");
		}
	}
	return (EXIT_SUCCESS);
}

/*===========================================================
| Count the number of operators on file within the range	|
| specified and store in an	array				            |
===========================================================*/
int
LoadOperators (void)
{
	char	tmpOperator [15];
	int     tmpLineNo;

	noOfOps = 0;

	/*----------------------------------
	| Swap start/end op if start op is |
	| greater than end op              |
	----------------------------------*/
	if (strcmp (local_rec.endOperator, local_rec.startOperator) < 0)
	{
		strcpy (tmpOperator, local_rec.startOperator);
		strcpy (local_rec.startOperator, local_rec.endOperator);
		strcpy (local_rec.endOperator, tmpOperator);
	}

	/*---------------------------------
	| Load all ops for company within |
	| specified range.                |
	---------------------------------*/
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	strcpy (tmop_rec.op_id, local_rec.startOperator);
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
	       strcmp (tmop_rec.op_id, local_rec.endOperator) <= 0)
	{
		sprintf (tsop [noOfOps].id, "%-14.14s", tmop_rec.op_id);
		tsop [noOfOps].hhopHash = tmop_rec.hhop_hash;
		tsop [noOfOps].allc = 0L;

		tmpLineNo = 0;
		tsal_rec.hhop_hash = tmop_rec.hhop_hash;
		tsal_rec.line_no = 0;
		cc = find_rec (tsal, &tsal_rec, GTEQ, "r");
		while (!cc && 
		       tsal_rec.hhop_hash == tmop_rec.hhop_hash)
		{
			tmpLineNo = tsal_rec.line_no;
			cc = find_rec (tsal, &tsal_rec, NEXT, "r");
		}
		tsop [noOfOps++].currentLine = ++tmpLineNo;

		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*=======================================================
| Count number of leads on file	within specified range	|
=======================================================*/
int
LoadLeads (void)
{
	int		i;
	char	tmpLeadNumber [7];

	/*-------------------------------------
	| If end lead comes before start lead |
	| swap the two .                      |
	-------------------------------------*/
	if (strcmp (local_rec.endCust, local_rec.startCust) < 0)
	{
		strcpy (tmpLeadNumber, local_rec.startCust);
		strcpy (local_rec.startCust, local_rec.endCust);
		strcpy (local_rec.endCust, tmpLeadNumber);
	}

	prospectCnt = 0L;

	cc = find_rec (tspm2, &tspm_rec, LAST, "r");
	while (!cc)
	{
		if (ValidLead ())
		{
			/*-------------------------------------------------
			| Check the array size before adding new element. |
			-------------------------------------------------*/
			if (!ArrChkLimit (&prospect_d, prospect, prospectCnt))
				sys_err ("ArrChkLimit (prospect)", ENOMEM, PNAME);

			/*---------------------------------------------
			| Load values into array element prospectCnt. |
			---------------------------------------------*/
			prospect [prospectCnt].hhcuHash = tspm_rec.hhcu_hash;
			prospect [prospectCnt].opNumber   = -1;

			if (LAST_OP || updateType == BULK_UNASS)
			{
				for (i = 0; i < noOfOps; i++)
				{
					if (LAST_OP)
					{
						if (!strcmp (tsop [i].id, tspm_rec.lst_op_code))
						{
							prospect [prospectCnt].opNumber   = i;
							break;
						}
					}
					else
					{
						if (!strcmp (tsop [i].id, tspm_rec.op_code))
						{
							prospect [prospectCnt].opNumber   = i;
							break;
						}
					}
				}
			}
			/*--------------------------
			| Increment array counter. |
			--------------------------*/
			prospectCnt++;
		}
		cc = find_rec (tspm2, &tspm_rec, PREVIOUS, "r");
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------
| Check that a lead matches the |
| selection criteria		    |
-------------------------------*/
int
ValidLead (void)
{
	long	phoneDate;
	int		letterOK;

	/*-----------------------
	| No cumr record exists |
	-----------------------*/
	cumr_rec.hhcu_hash = tspm_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
	if (cc || (!cc && strcmp (cumr_rec.co_no, comm_rec.co_no)))
		return (FALSE);

	if (strcmp (cumr_rec.curr_code, envVarCurrCode))
		return (FALSE);

	if (updateType == BULK_ASS)
	{
		/*---------------------------------------
		| Lead already allocated to an operator |
		---------------------------------------*/
		tsal_rec.hhcu_hash	= tspm_rec.hhcu_hash;
		cc = find_rec (tsal2,&tsal_rec, GTEQ, "r");
		if (!cc && tsal_rec.hhcu_hash == tspm_rec.hhcu_hash)
			return (FALSE);
	}

	/*-----------------------------
	| Lead outside selected range |
	-----------------------------*/
	if (strcmp (cumr_rec.dbt_no, local_rec.startCust) < 0 ||
	    strcmp (cumr_rec.dbt_no, local_rec.endCust) > 0)
		return (FALSE);

	if (!ALL_SECTS && 
	    (strcmp (cumr_rec.class_type, local_rec.startSector) < 0 ||
	    strcmp (cumr_rec.class_type, local_rec.endSector) > 0))
		return (FALSE);

	if (!ALL_AREAS && 
	    (strcmp (cumr_rec.area_code, local_rec.startArea) < 0 ||
	    strcmp (cumr_rec.area_code, local_rec.endArea) > 0))
		return (FALSE);

	/*-------------------------------------
	| Letter code selected for allocation |
	-------------------------------------*/
	if (LETTER_SEL)
	{
		/*---------------------------------------
		| Check if letter was sent to this lead |
		---------------------------------------*/
		letterOK = FALSE;
		tsls_rec.hhlh_hash = tslh_rec.hhlh_hash;
		tsls_rec.hhcu_hash = tspm_rec.hhcu_hash;
		tsls_rec.date_sent = 0L;
		cc = find_rec (tsls, &tsls_rec, GTEQ, "r");
		while (!cc &&
		       tsls_rec.hhlh_hash == tslh_rec.hhlh_hash &&
		       tsls_rec.hhcu_hash == tspm_rec.hhcu_hash)
		{
			if (tsls_rec.date_called == 0L)
			{
				letterOK = TRUE;
				break;
			}

			cc = find_rec (tsls, &tsls_rec, NEXT, "r");
		}

		if (!letterOK)
			return (FALSE);
	}

	if (!IGNORE_PH && updateType == BULK_ASS)
	{
		if (tspm_rec.n_phone_date == 0L)
		{
			if (envVarTsPhoneDate)
				return (FALSE);
			else
			{
				phoneDate = tspm_rec.lphone_date + (long) tspm_rec.phone_freq;
			}
		}
		else
			phoneDate = tspm_rec.n_phone_date;

		if (phoneDate > local_rec.nxt_ph)
			return (FALSE);
	}
	return (TRUE);
}

/*------------------------
| Process selected leads |
------------------------*/
int
ProcessLeads (void)
{
	int	tmpOpNumber;
	int	i;

	for (i = 0; i < prospectCnt; i++)
	{
		tmpOpNumber = OP_NUM;
			
		/*--------------------------------------------------
		| Assign to last operator. If LAST_OP is TRUE then |
		| last op number will be stored in opNumber          |
		--------------------------------------------------*/
		if (tmpOpNumber >= 0)
		{
			if (tsop [tmpOpNumber].allc < noToAllocate)
			{
				tsop [tmpOpNumber].allc++;
				continue;
			}

			if (opsLeftOver > 0L)
			{
				opsLeftOver--;
				tsop [tmpOpNumber].allc++;
				continue;
			}
		}

		/*---------------------------
		| Assign to random operator |
		---------------------------*/
		OP_NUM = FindRandomOp ();
	}

	for (i = 0; i < prospectCnt; i++)
	{
		if (OP_NUM < 0)
			continue;

		tspm_rec.hhcu_hash = prospect [i].hhcuHash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "u");
		if (cc)
			continue;

		cumr_rec.hhcu_hash = prospect [i].hhcuHash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
		if (cc)
			continue;

		dsp_process ("Prospect:", cumr_rec.dbt_no);
	
		sprintf (tspm_rec.op_code, "%-14.14s", tsop [OP_NUM].id);
		cc = abc_update (tspm, &tspm_rec);
		if (cc)
			file_err (cc, tspm, "DBUPDATE");

		tsal_rec.hhop_hash 	= tsop [OP_NUM].hhopHash;
		tsal_rec.line_no 	= tsop [OP_NUM].currentLine++;
		tsal_rec.hhcu_hash 	= prospect [i].hhcuHash;
		if (LETTER_SEL)
			tsal_rec.hhlh_hash = tslh_rec.hhlh_hash;
		else
			tsal_rec.hhlh_hash = 0L;
			
		cc = abc_add (tsal, &tsal_rec);
		if (cc)
			file_err (cc, tsal, "DBADD");
	}
	return (EXIT_SUCCESS);
}

/*==================================================================
| Update lead and allocation files for manual lead alloc / dealloc |
==================================================================*/
int
UpdateLead (void)
{
	long	leadHhcuHash;
	int		tmpLineNo;

	sprintf (tspm_rec.op_code, "%-14.14s", local_rec.manOperator);
	cc = abc_update (tspm, &tspm_rec);
	if (cc)
		file_err (cc, tspm, "DBUPDATE");

	if (newOpHash == oldOpHash)
		return (FALSE);

	leadHhcuHash = tspm_rec.hhcu_hash;

	/*------------------------
	| Deallocate from old op |
	------------------------*/
	if (oldOpHash > 0L)
	{
		tsal_rec.hhop_hash = oldOpHash;
		tsal_rec.line_no = 0;
		cc = find_rec (tsal, &tsal_rec, GTEQ, "u");
		while (!cc && tsal_rec.hhop_hash == oldOpHash)
		{
			if (tsal_rec.hhcu_hash == leadHhcuHash)
			{
				abc_delete (tsal);
				break;
			}
			else
				abc_unlock ("tsal");

			cc = find_rec (tsal, &tsal_rec, NEXT, "u");
		}
	}

	/*--------------------
	| Allocate to new op |
	--------------------*/
	if (newOpHash > 0L)
	{
		tmpLineNo = 0;
		tsal_rec.hhop_hash = newOpHash;
		tsal_rec.line_no = 0;
		cc = find_rec (tsal, &tsal_rec, GTEQ, "r");
		while (!cc && 
		       tsal_rec.hhop_hash == newOpHash)
		{
			tmpLineNo = tsal_rec.line_no;
			cc = find_rec (tsal, &tsal_rec, NEXT, "r");
		}

		tsal_rec.hhop_hash = newOpHash;
		tsal_rec.line_no = ++tmpLineNo;
		tsal_rec.hhcu_hash = leadHhcuHash;
		cc = abc_add (tsal, &tsal_rec);
		if (cc)
			file_err (cc, tsal, "DBADD");
	}
	return (EXIT_SUCCESS);
}

/*==============================
| Find a valid random operator |
| and return index into array  |
| that holds operator names    |
==============================*/
int
FindRandomOp (void)
{
	int		i				= 0,
			tmpOpNumber		= 0,
			allocationOK 	= FALSE;

	tmpOpNumber = rand () % noOfOps;

	for (i = tmpOpNumber; i < noOfOps; i++)
	{
		if (tsop [i].allc < noToAllocate)
		{
			tsop [i].allc++;
			allocationOK = TRUE;
			break;
		}

		if (opsLeftOver > 0L)
		{
			opsLeftOver--;
			tsop [i].allc++;
			allocationOK = TRUE;
			break;
		}
	}

	if (!allocationOK)
	{
		for (i = 0; i < tmpOpNumber; i++)
		{
			if (tsop [i].allc < noToAllocate)
			{
				tsop [i].allc++;
				allocationOK = TRUE;
				break;
			}

			if (opsLeftOver > 0L)
			{
				opsLeftOver--;
				tsop [i].allc++;
				allocationOK = TRUE;
				break;
			}
		}
	}
	if (allocationOK)
		return (i);
	else
		return (-1);
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmop (
 char*  key_val)
{
	_work_open (14,0,40);
	strcpy (tmop_rec.co_no,comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	save_rec ("#Operator I.D.","#Operator Full Name.");
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
		      !strncmp (tmop_rec.op_id, key_val,strlen (key_val)))
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

	strcpy (tmop_rec.co_no,comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", temp_str);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmop, "DBFIND");
}

/*========================================
| Search routine for Letter master file. |
========================================*/
void
SrchTslh (
 char*  key_val)
{
	_work_open (10,0,40);
	save_rec ("#Code","#Description");
	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp (tslh_rec.co_no, comm_rec.co_no) &&
	          !strncmp (tslh_rec.let_code, key_val, strlen (key_val)))
	{
		if (tslh_rec.lett_type [0] != 'F')
		{
			cc = find_rec (tslh, &tslh_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (tslh_rec.let_code, tslh_rec.let_desc);
		if (cc)
			break;

		cc = find_rec (tslh, &tslh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tslh, "DBFIND");

	sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
}

/*==========================================
| Search routine for Business Sector File. |
==========================================*/
void
SrchExcl (
 char*  key_val)
{
	_work_open (3,0,40);
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,key_val);
	save_rec ("#No.","#Business Sector Description");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (excl_rec.class_type, key_val,strlen (key_val)) && 
		!strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

/*======================================
| Search routine for Area Master File. |
======================================*/
void
SrchExaf (
 char*  key_val)
{
	_work_open (2,0,40);
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,key_val);
	save_rec ("#No.","#Area Description");
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strncmp (exaf_rec.area_code,key_val,strlen (key_val)) &&
		      !strcmp (exaf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

int
heading (
 int    scn)
{
	if (scn != cur_screen && scn != 0)
		scn_set (scn);

	if (clearOK)
		clear ();

	if (scn == 1)
	{
		if (updateType == BULK_ASS)
			box (0, 2, 80, 16);
		else
			box (0, 2, 80, 13);

		line_at (5,1,79);
		line_at (8,1,79);
		line_at (11,1,79);
		line_at (14,1,79);

		if (updateType == BULK_ASS)
			sprintf (err_str,ML (mlTsMess011));
		else
			sprintf (err_str,ML (mlTsMess012));
		rv_pr (err_str,30,0,1);
	}
	
	if (scn == 2)
	{
		box (0, 3, 80, 5);

		rv_pr (ML (mlTsMess013), 28, 0, 1);

		line_at (5,1,79);
		line_at (7,1,79);
	}

	line_at (1,0,80);
	line_at (20,0,80);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	if (scn != 0)
		scn_write (scn);

    return (EXIT_SUCCESS);
}
