/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_ld_allc.c,v 5.6 2002/07/25 11:17:38 scott Exp $
|  Program Name  : (tm_ld_allc.c)
|  Program Desc  : (Allocate/Deallocate leads to/from operators)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 02/07/91         |
|---------------------------------------------------------------------|
| $Log: tm_ld_allc.c,v $
| Revision 5.6  2002/07/25 11:17:38  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2002/03/01 03:10:55  scott
| S/C 00783 - TMMR3-2 - Allocate Leads; CHAR-BASED / WINDOWS CLIENT (1) END fields accept inputs less than the Start fields. WINDOWS CLIENT (2) When focus is at either Start Area or End Area, the current value truncates the first character.   Ex: Start Area = BC, becomes C only; End Area = AK, becomes K only.
|
| Revision 5.4  2001/11/15 02:35:07  robert
| Updated to rename operator variable to arrOperator since operator keyword
| is conflict Visual C++
|
| Revision 5.3  2001/11/12 05:01:46  scott
| Updated to add app.schema
| Updated to clean code
| Updated to line up prompts.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_ld_allc.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_ld_allc/tm_ld_allc.c,v 5.6 2002/07/25 11:17:38 scott Exp $";

#include <ml_tm_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#define	RESTRT		-1
#define	BULK_ASS	0
#define	BULK_UNASS	1
#define	MAN_ASS		2

#define	OP_NUM		(lcl_ptr->pspct_ary [i].op_num)

#define ALLC_ALL	(!strncmp (local_rec.numberAllocated, 	"ALL", 3))
#define ALL_SECTS	(!strncmp (local_rec.startSector, 		"~~~", 3))
#define ALL_AREAS	(!strncmp (local_rec.startArea, 		"~~", 2))
#define	LAST_OP		(local_rec.lastOperator [0] == 'Y')
#define	IGNORE_PH	(local_rec.nextPhone == 0L)

#define	LSL_PS_NULL	((struct PSPCT_PTR *) NULL)

	/*
	 * Special fields and flags.
	 */
   	int  	envDbCo 	= 0,
			envDbFind 	= 0,
			clear_ok	= FALSE,
			updateType	= 0;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct tmcfRecord	tmcf_rec;
struct tmopRecord	tmop_rec;
struct tmopRecord	tmop2_rec;
struct tmpmRecord	tmpm_rec;
struct tmpmRecord	tmpm2_rec;
struct tmahRecord	tmah_rec;
struct tmahRecord	tmah2_rec;
struct tmalRecord	tmal_rec;
struct exclRecord	excl_rec;
struct exafRecord	exaf_rec;

	char	*tmah2 	= 	"tmah2",
	    	*tmal2 	= 	"tmal2",
	    	*tmop2 	= 	"tmop2",
	    	*tmpm2 	= 	"tmpm2";

	long	lsystemDate;

	char	*curr_user;

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy 				[11];
	char	endArea 			[4];
	char	endAreaName 		[41];
	char	endLead 			[9];
	char	endLeadName 		[41];
	char	endOp 				[15];
	char	endOpName 			[41];
	char	endSector 			[4];
	char	endSectorName 		[41];
	char	lastOperator 		[2];
	char	lastOperatorDesc 	[4];
	char	manLastOp 			[15];
	char	manLastOpName 		[41];
	char	manOp 				[15];
	char	manOpName 			[41];
	char	numberAllocated 	[7];
	char	startArea 			[4];
	char	startAreaName 		[41];
	char	startLead 			[9];
	char	startLeadName 		[41];
	char	startOp 			[15];
	char	startOpName 		[41];
	char	startSector 		[4];
	char	startSectorName 	[41];
	long	nextPhone;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campaign_no", 3, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number      ", "Enter Campaign no:", 
		YES, NO, JUSTLEFT, "", "", (char *)&tmcf_rec.campaign_no}, 
	{1, LIN, "camp_desc1", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Description ", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name1}, 
	{1, LIN, "camp_desc2", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name2}, 
	{2, LIN, "startOp", 4, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Start Operator  ", "Enter Start Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.startOp}, 
	{2, LIN, "startOpName", 4, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startOpName}, 
	{2, LIN, "endOp", 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "End Operator    ", "Enter End Operator.", 
		YES, NO, JUSTLEFT, "", "", local_rec.endOp}, 
	{2, LIN, "endOpName", 5, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endOpName}, 
	{2, LIN, "startLead", 7, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Start Lead      ", "Enter Start Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startLead}, 
	{2, LIN, "startLeadName", 7, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startLeadName}, 
	{2, LIN, "endLead", 8, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Lead        ", "Enter End Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endLead}, 
	{2, LIN, "endLeadName", 8, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endLeadName}, 
	{2, LIN, "startSector", 10, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Start Sector    ", "Enter Start Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startSector}, 
	{2, LIN, "startSectorName", 10, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startSectorName}, 
	{2, LIN, "endSector", 11, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "End Sector      ", "Enter End Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endSector}, 
	{2, LIN, "endSectorName", 11, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endSectorName}, 
	{2, LIN, "startArea", 13, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Start Area      ", "Enter Start Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.startArea}, 
	{2, LIN, "startAreaName", 13, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startAreaName}, 
	{2, LIN, "endArea", 14, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "End Area        ", "Enter End Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.endArea}, 
	{2, LIN, "endAreaName", 14, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endAreaName}, 
	{2, LIN, "lastOperator", 16, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y","Last Operator   ", "Allocate as last operator.", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.lastOperator}, 
	{2, LIN, "lastOperatorDesc", 16, 30, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.lastOperatorDesc}, 
	{2, LIN, "nextPhone", 17, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Next Phone Date ", "Enter Next Phone Date. Enter 0 To Ignore This Selection Criteria", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.nextPhone}, 
	{2, LIN, "numberAllocated", 18, 2, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "No. To Allocate ", "Enter Number Of Leads To Allocate.", 
		NO, NO, JUSTLEFT, "0123456789", "", local_rec.numberAllocated}, 
	{3, LIN, "man_lead", 4, 15, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Lead Number      ", "Enter Lead To Assign.", 
		NE, NO, JUSTLEFT, "", "", local_rec.startLead}, 
	{3, LIN, "man_lead_name", 4, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startLeadName}, 
	{3, LIN, "manLastOp", 6, 15, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Last Operator   ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manLastOp}, 
	{3, LIN, "manLastOpName", 6, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manLastOpName}, 
	{3, LIN, "manOp", 8, 15, CHARTYPE, 
		"UUUUUUUUUUUUUU", "          ", 
		" ", "", "Operator        ", "Enter Operator To Assign Lead To. (Default Deallocates Lead.)", 
		NI, NO, JUSTLEFT, "", "", local_rec.manOp}, 
	{3, LIN, "manOpName", 8, 38, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.manOpName}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};
long	no_of_lds;
long	oldHhopHash;
long	newHhopHash;
int		no_to_alloc;
int		left_over;
int		no_of_ops;
int		curr_op;
int		first_time;

struct	{
	char	id [15];
	long	hhopHash;
	long	hhlaHash;
	int		allc;
	int		currentLine;
} arrOperator [256];

struct	PSPCT_ARY_PTR
{
	long		hhpmHash;
	int			op_num;
};

struct	PSPCT_PTR
{
	int	last_used;
	struct	PSPCT_ARY_PTR	pspct_ary [1000];
	struct	PSPCT_PTR	*next;
};

struct	PSPCT_PTR	*ps_head;
struct	PSPCT_PTR	*ps_curr;
struct	PSPCT_ARY_PTR	*ps_tmp;

MENUTAB ass_menu [] =
	{
		{ " 1. ASSIGN BULK LEADS   ",
		  " BULK ASSIGNMENT OF LEADS. " },
		{ " 2. UNASSIGN BULK LEADS ",
		  " UNASSIGN LEADS IN BULK." },
		{ " 3. MANUAL ASSIGNMENT   ",
		  " ASSIGN/UNASSIGN LEADS MANUALLY " },
		{ ENDMENU }
	};


int 	CheckLead 			(long);
int 	FoundRandomOperator (void);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	ValidLead 			(void);
struct 	PSPCT_PTR *ps_alloc (void);
void 	AddTmah 			(void);
void 	AssignMenu 			(void);
void 	BulkUpdate 			(void);
void 	CheckTmah 			(void);
void 	CloseDB 			(void);
void 	DeAllocate 			(void);
void 	LoadLeads 			(void);
void 	LoadOperators 		(void);
void 	ManualUpdate 		(void);
void 	OpenDB 				(void);
void 	ProcessLead 		(void);
void 	SrchExaf 			(char *);
void 	SrchExcl 			(char *);
void 	SrchTmop 			(char *);
void	SrchTmpm 			(char *);
void 	SrchTmpf 			(char *);
void 	UpdateLead 			(void);
void 	UpdateOps 			(void);

/*
 * Main Processing Routine.
 */
int
main (
	int argc, 
	char *argv [])
{
	/*
	 * Set position of minimenu
	 */
	MENU_ROW = 2;
	MENU_COL = 0;

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	set_masks ();

	lsystemDate = TodaysDate ();

	OpenDB (); 	

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*
	 * Main control loop.
	 */
	while (prog_exit == 0)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		clear_ok = TRUE;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		AssignMenu ();
		if (prog_exit || restart)
			continue;
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files .
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	abc_alias (tmop2, tmop);
	abc_alias (tmpm2, tmpm);
	abc_alias (tmal2, tmal);
	abc_alias (tmah2, tmah);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (tmcf,  tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmah,  tmah_list, TMAH_NO_FIELDS, "tmah_id_no");
	open_rec (tmah2, tmah_list, TMAH_NO_FIELDS, "tmah_hhla_hash");
	open_rec (tmal,  tmal_list, TMAL_NO_FIELDS, "tmal_id_no");
	open_rec (tmal2, tmal_list, TMAL_NO_FIELDS, "tmal_hhpm_hash");
	open_rec (tmop,  tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmop2, tmop_list, TMOP_NO_FIELDS, "tmop_hhop_hash");
	open_rec (tmpm,  tmpm_list, TMPM_NO_FIELDS, "tmpm_id_no3");
	open_rec (tmpm2, tmpm_list, TMPM_NO_FIELDS, "tmpm_hhpm_hash");
	open_rec (excl,  excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
}	

/*
 * Close data base files .
 */
void
CloseDB (void)
{
	abc_fclose (tmcf);
	abc_fclose (tmah);
	abc_fclose (tmah2);
	abc_fclose (tmal);
	abc_fclose (tmal2);
	abc_fclose (tmpm);
	abc_fclose (tmpm2);
	abc_fclose (tmop);
	abc_fclose (tmop2);
	abc_fclose (excl);
	abc_fclose (exaf);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Campaign Number
	 */
	if (LCHECK ("campaign_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
			scn_display (1);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startOp"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.startOp);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.startOpName, "%-40.40s", tmop_rec.op_name);
		
		DSP_FLD ("startOpName");
		return (EXIT_SUCCESS);
	}
					
	if (LCHECK ("endOp"))
	{
		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no,comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.endOp);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		if (strcmp (local_rec.startOp,local_rec.endOp) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.endOpName, "%-40.40s", tmop_rec.op_name);
		DSP_FLD ("endOpName");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("startLead") || LCHECK ("endLead"))
	{
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		if (LCHECK ("startLead"))
			strcpy (tmpm_rec.pro_no,pad_num (local_rec.startLead));
		else
			strcpy (tmpm_rec.pro_no,pad_num (local_rec.endLead));
		cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess051));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("startLead"))
			sprintf (local_rec.startLeadName, "%-40.40s", tmpm_rec.name);
		else
			sprintf (local_rec.endLeadName, "%-40.40s", tmpm_rec.name);

		if (LCHECK ("endLead"))
		{
			if (strcmp (local_rec.startLead,local_rec.endLead) > 0)
			{
				errmess (ML (mlStdMess006));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		DSP_FLD ("startLeadName");
		DSP_FLD ("endLeadName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startArea") || LCHECK ("endArea"))
	{
		if (LCHECK ("endArea") && FLD ("endArea") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used && LCHECK ("startArea"))
		{
			strcpy (local_rec.startArea, "~~");
			strcpy (local_rec.endArea,   "~~");
			strcpy (local_rec.startAreaName, ML ("All Areas"));
			strcpy (local_rec.endAreaName,   ML ("All Areas"));
			DSP_FLD ("startArea");
			DSP_FLD ("endArea");
			DSP_FLD ("startAreaName");
			DSP_FLD ("endAreaName");
			FLD ("endArea") = NA;
			return (EXIT_SUCCESS);
		}

		FLD ("endArea") = NO;
		if (!strcmp (local_rec.endArea, "~~"))
		{
			strcpy (local_rec.endArea, "  ");
			DSP_FLD ("endArea");
		}
		
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		if (LCHECK ("startArea"))
			sprintf (exaf_rec.area_code, "%2.2s", local_rec.startArea);
		else
			sprintf (exaf_rec.area_code, "%2.2s", local_rec.endArea);

		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (LCHECK ("startArea"))
		{
			sprintf (local_rec.startArea, "%2.2s ", exaf_rec.area_code);
			sprintf (local_rec.startAreaName, "%-40.40s", exaf_rec.area);
		}
		else
		{
			sprintf (local_rec.endArea, "%2.2s ", exaf_rec.area_code);
			sprintf (local_rec.endAreaName, "%-40.40s", exaf_rec.area);
		}

		if (LCHECK ("endArea"))
		{
			if (strcmp (local_rec.startArea,local_rec.endArea) > 0)
			{
				errmess (ML (mlStdMess006));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		DSP_FLD ("startAreaName");
		DSP_FLD ("endAreaName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startSector") || LCHECK ("endSector"))
	{
		if (LCHECK ("endSector") && FLD ("endSector") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used && LCHECK ("startSector"))
		{
			strcpy (local_rec.startSector, "~~~");
			strcpy (local_rec.endSector, "~~~");
			strcpy (local_rec.startSectorName, "All Business Sectors");
			strcpy (local_rec.endSectorName, "All Business Sectors");
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
		if (LCHECK ("startSector"))
			strcpy (excl_rec.class_type,local_rec.startSector);
		else
			strcpy (excl_rec.class_type,local_rec.endSector);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTmMess062));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (LCHECK ("startSector"))
			strcpy (local_rec.startSectorName, excl_rec.class_desc);
		else
			strcpy (local_rec.endSectorName, excl_rec.class_desc);

		if (LCHECK ("endSector"))
		{
			if (strcmp (local_rec.startSector,local_rec.endSector) > 0)
			{
				errmess (ML (mlStdMess006));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		DSP_FLD ("startSectorName");
		DSP_FLD ("endSectorName");
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

	if (LCHECK ("nextPhone"))
	{
		if (dflt_used)
		{
			local_rec.nextPhone = lsystemDate;
			return (EXIT_SUCCESS);
		}
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("numberAllocated"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.numberAllocated, "ALL   ");
			DSP_FLD ("numberAllocated");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("man_lead"))
	{
		if (last_char == FN16)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}
	
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		strcpy (tmpm_rec.pro_no,pad_num (local_rec.startLead));
		cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "w");
		if (cc)
		{
			print_mess (ML ("Lead Not Found On File "));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startLeadName, "%-40.40s", tmpm_rec.name);
		DSP_FLD ("man_lead_name");

		if (strlen (clip (tmpm_rec.lst_op_code)) != 0)
		{
			strcpy (tmop_rec.co_no, comm_rec.co_no);
			sprintf (tmop_rec.op_id, "%-14.14s", tmpm_rec.lst_op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (local_rec.manLastOp, 	ML ("Unknown"));
				strcpy (local_rec.manLastOpName,ML ("Unknown Operator"));
			}
			else
			{
				sprintf (local_rec.manLastOp, "%-14.14s", tmop_rec.op_id);
				sprintf (local_rec.manLastOpName,"%-40.40s",tmop_rec.op_name);
			}
		}
	
		oldHhopHash = -1L;
		if (strlen (clip (tmpm_rec.op_code)) != 0)
		{
			strcpy (tmop_rec.co_no, comm_rec.co_no);
			sprintf (tmop_rec.op_id, "%-14.14s",tmpm_rec.op_code);
			cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (local_rec.manOp, "%-14.14s", " ");
				sprintf (local_rec.manOpName, "%-40.40s", " ");
			}
			else
			{
				sprintf (local_rec.manOp, "%-14.14s", tmop_rec.op_id);
				sprintf (local_rec.manOpName, "%-40.40s", tmop_rec.op_name);
				oldHhopHash = tmop_rec.hhop_hash;
				newHhopHash = oldHhopHash;
			}
			FLD ("manOp") = NI;
		}
		else
			FLD ("manOp") = NO;
	
		DSP_FLD ("manLastOp");
		DSP_FLD ("manLastOpName");
		DSP_FLD ("manOp");
		DSP_FLD ("manOpName");

		return (EXIT_SUCCESS);
	}		

	if (LCHECK ("manOp"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.manOp, "%-14.14s", " ");
			sprintf (local_rec.manOpName, "%-40.40s", " ");
			DSP_FLD ("manOp");
			DSP_FLD ("manOpName");
			newHhopHash = -1L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.manOp);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.manOpName, "%-40.40s", tmop_rec.op_name);
		newHhopHash = tmop_rec.hhop_hash;
		
		DSP_FLD ("manOpName");
		return (EXIT_SUCCESS);
	}		
	return (EXIT_SUCCESS);
}	

/*
 * Assign mini menu.
 */
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
			BulkUpdate ();
			return;

		case BULK_UNASS :
			updateType = BULK_UNASS;
			BulkUpdate ();
			return;

		case MAN_ASS :
			updateType = MAN_ASS;
			ManualUpdate ();
			return;
			break;

		case RESTRT :
			restart = TRUE;
			return;
	
		default :
			break;
		}
	}
}

/*
 * Allocate or reallocate a single lead.
 */
void
ManualUpdate (void)
{
	while (!prog_exit && !restart)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (3);

		clear_ok = TRUE;

		/*
		 * Enter screen 1 linear input .
		 */
		newHhopHash = -1L;
		heading (3);
		entry (3);
		if (prog_exit || restart)
			continue;

		heading (3);
		scn_display (3);
		edit (3);
		if (restart)
			continue;

		UpdateLead ();
		CheckTmah ();
	}
	return;
}

/*
 * Bulk allocation or deallocation of leads
 */
void
BulkUpdate (void)
{

	while (!prog_exit && !restart)
	{
		if (updateType == BULK_ASS)
		{
			FLD ("lastOperator") = NO;
			FLD ("nextPhone") = NO;
			FLD ("numberAllocated") = NO;
			FLD ("lastOperatorDesc") = NI;
		}
		else
		{
			FLD ("lastOperator") = ND;
			FLD ("nextPhone") = ND;
			FLD ("numberAllocated") = ND;
			FLD ("lastOperatorDesc") = ND;
		}

		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (2);

		clear_ok = TRUE;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (2);
		entry (2);
		if (prog_exit || restart)
			continue;

		heading (2);
		scn_display (2);
		edit (2);
		if (prog_exit || restart)
			continue;

		LoadOperators ();

		first_time = TRUE;
		LoadLeads ();

		if (updateType == BULK_ASS)
		{
			if (ALLC_ALL)
			{
				no_to_alloc = no_of_lds / no_of_ops;
				left_over = no_of_lds % no_of_ops;
			}
			else
			{
				no_to_alloc = atol (local_rec.numberAllocated);
				left_over = 0;
			}
	
			dsp_screen ("Allocating Leads", 
				comm_rec.co_no, 
				comm_rec.co_name);

			ProcessLead ();
		}
		else
			DeAllocate ();

		CheckTmah ();
		UpdateOps ();
	}

	return;
}

/*
 * Deallocate leads
 */
void
DeAllocate (void)
{
	struct	PSPCT_PTR	*lcl_ptr;
	int	tmpOpNum;
	int	i;
	long	tmpHash;
	long	leadHash;

	lcl_ptr = ps_head;
	while (lcl_ptr != LSL_PS_NULL)
	{
		for (i = 0; i < lcl_ptr->last_used; i++)
		{
			tmpm2_rec.hhpm_hash	= lcl_ptr->pspct_ary [i].hhpmHash;
			cc = find_rec (tmpm2, &tmpm2_rec, COMPARISON, "u");
			if (cc)
				continue;

			sprintf (err_str, ML ("Processing Prospect : %s"),tmpm2_rec.pro_no);
			print_mess (err_str);
	
			if (OP_NUM > no_of_ops || OP_NUM < 0)
			{
				abc_unlock (tmpm2);
				continue;
			}

			if (strcmp (arrOperator [OP_NUM].id, tmpm2_rec.op_code))
			{
				abc_unlock (tmpm2);
				continue;
			}

			sprintf (tmpm2_rec.op_code, "%-14.14s", " ");

			cc = abc_update (tmpm2, &tmpm2_rec);
			if (cc)
				file_err (cc, "tmpm2", "DBUPDATE");
			
			tmpOpNum = OP_NUM;
			tmpHash = arrOperator [tmpOpNum].hhlaHash;
			leadHash = lcl_ptr->pspct_ary [i].hhpmHash;

			tmal_rec.hhla_hash = tmpHash;
			tmal_rec.line_no = 0;
			cc = find_rec (tmal, &tmal_rec, GTEQ, "u");
			while (!cc && 
			       tmal_rec.hhla_hash == tmpHash &&
			       tmal_rec.hhpm_hash != leadHash)
			{
				abc_unlock (tmal);
				cc = find_rec (tmal, &tmal_rec, NEXT, "u");
			}

			if (tmal_rec.hhpm_hash == leadHash)
				abc_delete (tmal);
		}

		lcl_ptr = lcl_ptr->next;
	}

	return;
}
/*
 * Count the number of operators on file within the range
 * specified and store in an array	
 */
void
LoadOperators (void)
{
	char	tmp_op [15];
	int	tmp_line;

	no_of_ops = 0;

	/*
	 * Swap start/end op if start op is greater than end op             
	 */
	if (strcmp (local_rec.endOp, local_rec.startOp) < 0)
	{
		sprintf (tmp_op, "%-14.14s", local_rec.startOp);
		sprintf (local_rec.startOp, "%-14.14s", local_rec.endOp);
		sprintf (local_rec.endOp, "%-14.14s", tmp_op);
	}

	/*
	 * Load all ops for company within sepcified range.
	 */
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-6.6s", local_rec.startOp);
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
	       strcmp (tmop_rec.op_id, local_rec.endOp) <= 0)
	{
		tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		tmah_rec.hhop_hash = tmop_rec.hhop_hash;
		cc = find_rec (tmah, &tmah_rec, COMPARISON, "r");
		if (cc)
		{
			if (updateType == BULK_ASS)
				AddTmah ();
			else
			{
				cc = find_rec (tmop, &tmop_rec,NEXT,"r");
				continue;
			}
		}

		sprintf (arrOperator [no_of_ops].id, "%-14.14s", tmop_rec.op_id);
		arrOperator [no_of_ops].hhlaHash = tmah_rec.hhla_hash;
		arrOperator [no_of_ops].hhopHash = tmop_rec.hhop_hash;
		arrOperator [no_of_ops].allc = 0;

		tmp_line = 0;
		tmal_rec.hhla_hash = tmah_rec.hhla_hash;
		tmal_rec.line_no = 0;
		cc = find_rec (tmal, &tmal_rec, GTEQ, "r");
		while (!cc && 
		       tmal_rec.hhla_hash == tmah_rec.hhla_hash)
		{
			tmp_line = tmal_rec.line_no;
			cc = find_rec (tmal, &tmal_rec, NEXT, "r");
		}

		arrOperator [no_of_ops++].currentLine = ++tmp_line;

		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}

	return;
}

/*
 * Add tmah_record for operator
 */
void
AddTmah (void)
{
	strcpy (tmah_rec.co_no, comm_rec.co_no);
	tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	tmah_rec.hhop_hash = tmop_rec.hhop_hash;
	cc = abc_add (tmah, &tmah_rec);
	if (cc)
		file_err (cc, tmah, "DBADD");

	tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	tmah_rec.hhop_hash = tmop_rec.hhop_hash;
	cc = find_rec (tmah, &tmah_rec, COMPARISON, "r");
	
	return;
}

/*
 * Delete tmah records that have no tmal records
 */
void
CheckTmah (void)
{
	tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	tmah_rec.hhop_hash = 0L;
	cc = find_rec (tmah, &tmah_rec, GTEQ, "u");
	while (!cc && tmah_rec.hhcf_hash == tmcf_rec.hhcf_hash)
	{
		tmal_rec.hhla_hash = tmah_rec.hhla_hash;
		tmal_rec.line_no = 0;
		cc = find_rec (tmal, &tmal_rec, GTEQ, "r");
		if (cc || tmal_rec.hhla_hash != tmah_rec.hhla_hash)
		{
			abc_delete (tmah);
			tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
			tmah_rec.hhop_hash = 0L;
			cc = find_rec (tmah, &tmah_rec, GTEQ, "u");
			continue;
		}

		cc = find_rec (tmah, &tmah_rec, NEXT, "u");
	}

	abc_unlock (tmah);

	return;
}

/*
 * Update campaign number on tmop
 */
void
UpdateOps (void)
{
	int	i;

	if (updateType != BULK_ASS)
		return;

	for (i = 0; i < no_of_ops; i++)
	{
		tmop2_rec.hhop_hash	=	arrOperator [i].hhopHash;
		cc = find_rec (tmop2, &tmop2_rec, COMPARISON, "u");
		if (!cc && tmop2_rec.campaign_no != tmcf_rec.campaign_no)
		{
			tmop2_rec.campaign_no = tmcf_rec.campaign_no;
			cc = abc_update (tmop2, &tmop2_rec);
			if (cc)
				continue;
		}
		else
			abc_unlock ("tmop2");
	}
	
	return;
}

/*
 * Count number of leads on file within specified range
 */
void
LoadLeads (void)
{
	int	i;
	char	tmp_lead [7];

	/*
	 * If end lead comes before start lead swap the two.
	 */
	if (strcmp (local_rec.endLead, local_rec.startLead) < 0)
	{
		sprintf (tmp_lead, "%-6.6s", local_rec.startLead);
		sprintf (local_rec.startLead, "%-6.6s", local_rec.endLead);
		sprintf (local_rec.endLead, "%-6.6s", tmp_lead);
	}

	no_of_lds = 0L;

	abc_selfield (tmpm, "tmpm_id_no5");

	cc = find_rec (tmpm, &tmpm_rec, LAST, "r");
	if (cc)
		file_err (cc, tmpm, "DBFIND");

	/*
	 * Find lead which is the upper limit we wish to select.
	 */
	while (!cc && strcmp (tmpm_rec.co_no, comm_rec.co_no))
		cc = find_rec (tmpm, &tmpm_rec, PREVIOUS, "r");

	while (!cc && !strcmp (tmpm_rec.co_no, comm_rec.co_no))
	{
		if (ValidLead ())
		{
			ps_alloc ();

			ps_tmp->hhpmHash = tmpm_rec.hhpm_hash;
			ps_tmp->op_num = -1;

			/*
			 * If assigning by last operator or bulk 
			 * deallocating then set operator hash to
			 * current operator                      
			 */
			if (LAST_OP || updateType == BULK_UNASS)
			{
				for (i = 0; i < no_of_ops; i++)
				{
					if (LAST_OP)
					{
						if (!strcmp (arrOperator [i].id, tmpm_rec.lst_op_code))
						{
							ps_tmp->op_num = i;
							break;
						}
					}
					else
					{
						if (!strcmp (arrOperator [i].id, tmpm_rec.op_code))
						{
							ps_tmp->op_num = i;
							break;
						}
					}
				}
			}

			no_of_lds++;
		}

		cc = find_rec (tmpm, &tmpm_rec, PREVIOUS, "r");
	}

	abc_selfield (tmpm, "tmpm_id_no3");

	return;
}

/*
 * Check that a lead matches the selection criteria.
 */
int
ValidLead (void)
{
	long	phone_date;

	if (strcmp (tmpm_rec.pro_no, local_rec.startLead) < 0)
		return (FALSE);

	if (strcmp (tmpm_rec.pro_no, local_rec.endLead) > 0)
		return (FALSE);

	if (!ALL_SECTS && 
	   (strcmp (tmpm_rec.b_sector, local_rec.startSector) < 0 ||
	    strcmp (tmpm_rec.b_sector, local_rec.endSector) > 0))
		return (FALSE);

	if (!ALL_AREAS && 
	   (strcmp (tmpm_rec.area_code, local_rec.startArea) < 0 ||
	    strcmp (tmpm_rec.area_code, local_rec.endArea) > 0))
		return (FALSE);

	if (!IGNORE_PH)
	{
		if (tmpm_rec.n_phone_date == 0L)
			phone_date = tmpm_rec.lphone_date + (long) tmpm_rec.phone_freq;
		else
			phone_date = tmpm_rec.n_phone_date;

		if (phone_date > local_rec.nextPhone)
			return (FALSE);
	
	}
	return (TRUE);
}

/*
 * Process selected leads
 */
void
ProcessLead (void)
{
	struct	PSPCT_PTR	*lcl_ptr;
	int	tmpOpNum;
	int	i;

	/*
	 * Assign leads to operators WITHIN the linked list
	 */
	lcl_ptr = ps_head;
	while (lcl_ptr != LSL_PS_NULL)
	{
		for (i = 0; i < lcl_ptr->last_used; i++)
		{
			/*
			 * Check that lead isn't already assigned
			 * to an operator for this campaign      
			 */
			if (!CheckLead (lcl_ptr->pspct_ary [i].hhpmHash))
				continue;

			tmpOpNum = OP_NUM;
			
			/*
			 * Assign to last operator. If LAST_OP is TRUE then
			 * last op number will be stored in op_num        
			 */
			if (tmpOpNum >= 0)
			{
		    		if (arrOperator [tmpOpNum].allc < no_to_alloc)
				{
					arrOperator [tmpOpNum].allc++;
					continue;
				}

				if (left_over > 0)
				{
					left_over--;
					arrOperator [tmpOpNum].allc++;
					continue;
				}
			}

			/*
			 * Assign to random operator 
			 */
			OP_NUM = FoundRandomOperator ();
		}

		lcl_ptr = lcl_ptr->next;
	}

	/*
	 * Process linked list and create allocation records.
	 */
	lcl_ptr = ps_head;
	while (lcl_ptr != LSL_PS_NULL)
	{
		for (i = 0; i < lcl_ptr->last_used; i++)
		{
			if (OP_NUM < 0)
				continue;

			tmpm2_rec.hhpm_hash	= lcl_ptr->pspct_ary [i].hhpmHash;
			cc = find_rec (tmpm2, &tmpm2_rec, COMPARISON, "u");
			if (cc)
				continue;

			sprintf (err_str, ML ("Processing Prospect : %s"),tmpm2_rec.pro_no);
			print_mess (err_str);
	
			sprintf (tmpm2_rec.op_code, "%-14.14s", arrOperator [OP_NUM].id);

			cc = abc_update (tmpm2, &tmpm2_rec);
			if (cc)
				file_err (cc, "tmpm2", "DBUPDATE");

			tmal_rec.hhla_hash 	= arrOperator [OP_NUM].hhlaHash;
			tmal_rec.line_no 	= arrOperator [OP_NUM].currentLine++;
			tmal_rec.hhpm_hash 	= lcl_ptr->pspct_ary [i].hhpmHash;
			
			cc = abc_add (tmal, &tmal_rec);
			if (cc)
				file_err (cc, tmal, "DBADD");
		}
		lcl_ptr = lcl_ptr->next;
	}

	return;
}

/*
 * Check that operator has not already been assigned the lead with this campaign
 */
int
CheckLead (
	long	hhpmHash)
{
	tmal_rec.hhpm_hash	=	hhpmHash;
	cc = find_rec (tmal2, &tmal_rec, GTEQ, "r");
	while (!cc && tmal_rec.hhpm_hash == hhpmHash)
	{
		tmah2_rec.hhla_hash	=	tmal_rec.hhla_hash;
		cc = find_rec (tmah2, &tmah2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (tmal2, &tmal_rec, NEXT, "r");
			continue;
		}
		if (tmah2_rec.hhcf_hash == tmcf_rec.hhcf_hash)
			return (FALSE);

		cc = find_rec (tmal2, &tmal_rec, NEXT, "r");
	}

	return (TRUE);
}

/*
 * Update lead and allocation files for manual lead alloc / dealloc 
 */
void
UpdateLead (void)
{
	long	leadHash;
	int		tmpLineNo = 0;

	sprintf (tmpm_rec.op_code, "%-14.14s", local_rec.manOp);
	cc = abc_update (tmpm, &tmpm_rec);
	if (cc)
		file_err (cc, tmpm, "DBUPDATE");

	abc_unlock (tmpm);

	if (newHhopHash == oldHhopHash)
		return;

	leadHash = tmpm_rec.hhpm_hash;

	/*
	 * Deallocate from old op
	 */
	if (oldHhopHash > 0L)
	{
		tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		tmah_rec.hhop_hash = oldHhopHash;
		cc = find_rec (tmah, &tmah_rec, COMPARISON, "u");
		if (!cc)
		{
			tmal_rec.hhla_hash = tmah_rec.hhla_hash;
			tmal_rec.line_no = 0;
			cc = find_rec (tmal, &tmal_rec, GTEQ, "u");
			while (!cc && 
		       	tmal_rec.hhla_hash == tmah_rec.hhla_hash &&
		       	tmal_rec.hhpm_hash != leadHash)
			{
				abc_unlock (tmal);
				cc = find_rec (tmal, &tmal_rec, NEXT, "u");
			}
	
			if (tmal_rec.hhpm_hash == leadHash)
				abc_delete (tmal);
		}
	}

	/*
	 * Allocate to new op
	 */
	if (newHhopHash > 0L)
	{
		tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		tmah_rec.hhop_hash = newHhopHash;
		cc = find_rec (tmah, &tmah_rec, COMPARISON, "u");
		if (cc)
		{
			strcpy (tmah_rec.co_no, comm_rec.co_no);
			cc = abc_add (tmah, &tmah_rec);
			if (cc)
				file_err (cc, tmah, "DBADD");

			tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
			tmah_rec.hhop_hash = newHhopHash;
			cc = find_rec (tmah, &tmah_rec, COMPARISON, "u");
			if (!cc)
			{
				tmal_rec.hhla_hash 	= tmah_rec.hhla_hash;
				tmal_rec.line_no 	= 0;
				tmal_rec.hhpm_hash 	= leadHash;
				cc = abc_add (tmal, &tmal_rec);
				if (cc)
					file_err (cc, tmal, "DBADD");
			}
		}
		else	
		{
			tmal_rec.hhla_hash = tmah_rec.hhla_hash;
			tmal_rec.line_no = 0;
			cc = find_rec (tmal, &tmal_rec, GTEQ, "r");
			while (!cc && 
			       tmal_rec.hhla_hash == tmah_rec.hhla_hash)
			{
				tmpLineNo = tmal_rec.line_no;
				cc = find_rec (tmal, &tmal_rec, NEXT, "r");
			}

			tmal_rec.hhla_hash 	= tmah_rec.hhla_hash;
			tmal_rec.line_no 	= ++tmpLineNo;
			tmal_rec.hhpm_hash 	= leadHash;
			cc = abc_add (tmal, &tmal_rec);
			if (cc)
				file_err (cc, tmal, "DBADD");
		}
	}

	return;
}

/*
 * Find a valid random operator and return index into 
 * array  that holds operator names   
 */
int
FoundRandomOperator (void)
{
	int	i;
	int	tmpOpNum;
	int	ALLOC_OK = FALSE;

	tmpOpNum = rand () % no_of_ops;

	for (i = tmpOpNum; i < no_of_ops; i++)
	{
		if (arrOperator [i].allc < no_to_alloc)
		{
			arrOperator [i].allc++;
			ALLOC_OK = TRUE;
			break;
		}

		if (left_over > 0)
		{
			left_over--;
			arrOperator [i].allc++;
			ALLOC_OK = TRUE;
			break;
		}
	}

	if (!ALLOC_OK)
	{
		for (i = 0; i < tmpOpNum; i++)
		{
			if (arrOperator [i].allc < no_to_alloc)
			{
				arrOperator [i].allc++;
				ALLOC_OK = TRUE;
				break;
			}

			if (left_over > 0)
			{
				left_over--;
				arrOperator [i].allc++;
				ALLOC_OK = TRUE;
				break;
			}
		}
	}

	if (ALLOC_OK)
		return (i);
	else
		return (-1);
}

/*
 * Search routine for Campaign Header File.
 */
void
SrchTmpf (
	char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.","#Campaign No Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%04d", tmcf_rec.campaign_no);
		cc = save_rec (err_str, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no,comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "tmcf", "DBFIND");
}

/*
 * Search routine for Script Header File.
 */
void
SrchTmop (
	char *key_val)
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

/*
 * Search routine for Business Sector File.
 */
void
SrchExcl (
	char *key_val)
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
		file_err (cc, "excl", "DBFIND");
}

/*
 * Search routine for Area Master File.
 */
void
SrchExaf (
	char	*key_val)
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
	int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	if (scn == 1 && clear_ok)
		clear ();

	if (scn == 1)
	{
		box (0,2,80,3);
		rv_pr (ML (mlTmMess041),30,0,1);
	}
		
	if (scn == 2)
	{
		clear ();
		if (updateType == BULK_ASS)
			box (0, 2, 80, 16);
		else
			box (0, 2, 80, 12);

		line_at (6,1,79);
		line_at (9,1,79);
		line_at (12,1,79);

		if (updateType == BULK_ASS)
			line_at (15,1,79);

		rv_pr ((updateType == BULK_ASS) ? ML (mlTmMess042) : ML (mlTmMess043),30,0,1);

		sprintf (err_str, 
			" Campaign: %4d %-40.40s%-13.13s ", 
			tmcf_rec.campaign_no,
			tmcf_rec.c_name1,
			tmcf_rec.c_name2);
		rv_pr (err_str,5,2,1);
	}
	
	if (scn == 3)
	{
		clear ();
		box (0, 2, 80, 6);

		rv_pr (ML (mlTmMess044), 28, 0, 1);

		line_at (5,1,79);
		line_at (7,1,79);

		sprintf (err_str, ML (mlTmMess024), tmcf_rec.campaign_no,
			tmcf_rec.c_name1,
			tmcf_rec.c_name2);
		rv_pr (err_str,5,2,1);
	}

	line_at (1,0,80);
	line_at (20,0,80);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);


	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

/*
 * Allocate memory for prospect linked list
 */
struct	PSPCT_PTR *ps_alloc (void)
{
	struct	PSPCT_PTR	*lcl_ptr;

	if (first_time || ps_curr->last_used > 999)
	{
		lcl_ptr = (struct PSPCT_PTR *) malloc (sizeof (struct PSPCT_PTR));

		if (lcl_ptr == LSL_PS_NULL)
			file_err (errno, "MALLOC", "NULL");

		lcl_ptr->next = LSL_PS_NULL;
		if (first_time)
		{
			lcl_ptr->last_used = 0;
			ps_head = lcl_ptr;
			ps_curr = lcl_ptr;
		}

		if (ps_curr->last_used >= 999)
		{
			lcl_ptr->last_used = 0;
			ps_head->next = lcl_ptr;
			ps_curr = lcl_ptr;
		}

		first_time = FALSE;
	}

	ps_tmp = &ps_curr->pspct_ary [ ps_curr->last_used++ ];

	return (EXIT_SUCCESS);
}

#include	<wild_search.h>

/*
 * Search for Prospect master file. 
 */
void
SrchTmpm (char *key_val)
{
	int		valid = 1;
	int		break_out;
	char	type_flag[2];
	char	*stype = chk_env ("DB_SER");
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	switch (search_key)
	{
	case	FN9:
		strcpy (type_flag,"N");
		break;

	case	FN10:
		strcpy (type_flag,"A");
		break;

	case	FN12:
		strcpy (type_flag,"D");
		break;

	default:
		sprintf (type_flag,"%-1.1s", (stype != (char *)0) ? stype : "N");
		break;
	}

	if (type_flag[0] == 'D' || type_flag[0] == 'd')
		sptr = (char *)0;

	work_open ();

	if (type_flag[0] == 'A' || type_flag[0] == 'a')
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no4" : "tmpm_id_no2");
		save_rec ("#Number","# Acronym   Prospect Name.");
	}
	else
	{
		abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
		save_rec ("#Number","# Prospect Name.");
	}
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.acronym,"%-9.9s", (sptr != (char *)0) ? sptr : " ");
	sprintf (tmpm_rec.pro_no,"%-6.6s", (sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	while (!cc && !strcmp (tmpm_rec.co_no,comm_rec.co_no))
	{
		/*
		 * If Debtors Branch Owned && Correct Branch. 
		 */
		if (!envDbFind && strcmp (tmpm_rec.br_no,branchNo))
			break;

		switch (type_flag[0])
		{
		case	'A':
		case	'a':
			valid = check_search (tmpm_rec.acronym,key_val,&break_out);
			break;

		case	'D':
		case	'd':
			valid = check_search (tmpm_rec.name,key_val,&break_out);
			break_out = 0;
			break;

		default:
			valid = check_search (tmpm_rec.pro_no,key_val,&break_out);
			break;
		}

		if (valid)
		{
			sprintf (err_str," (%s) %-40.40s", tmpm_rec.acronym, tmpm_rec.name);
			cc = save_rec (tmpm_rec.pro_no,err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (tmpm,&tmpm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	abc_selfield (tmpm, (envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");
	if (cc)
	{
		sprintf (tmpm_rec.acronym,"%-9.9s"," ");
		return;
	}
	
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	sprintf (tmpm_rec.pro_no,"%-6.6s",temp_str);
	sprintf (tmpm_rec.acronym,"%-9.9s",temp_str);
	cc = find_rec (tmpm,&tmpm_rec,GTEQ,"r");
	if (cc)
		file_err (cc, tmpm, "DBFIND");
}
