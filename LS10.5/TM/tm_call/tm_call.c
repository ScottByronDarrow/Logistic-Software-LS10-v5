/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_call.c,v 5.3 2001/11/14 02:41:45 scott Exp $
|  Program Name  : (tm_call.c)
|  Program Desc  : (Receive/Make calls)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 16/07/91         |
|---------------------------------------------------------------------|
| $Log: tm_call.c,v $
| Revision 5.3  2001/11/14 02:41:45  scott
| Updated to convert to app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_call.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_call/tm_call.c,v 5.3 2001/11/14 02:41:45 scott Exp $";

extern	int		X_EALL;
extern	int		Y_EALL;

#define	MOD	5

#define	TXT_REQD
#define	MAXSCNS		10

#include <pslscr.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

#define	RESTRT		-1
#define	PG_EXIT		99
#define	CAMPAIGN	0
#define	RANDOM		1
#define	RECEIVE		2

#define	NOTES		"N"
#define	LST_CALL	"L"
#define	USER_DEF	"U"

#define	FIRST_PRMPT		 (nextPrompt == -1)
#define	SUSPEND_CALL	 (local_rec.response [0] == 'S')
#define	END_CALL		 (local_rec.response [0] == 'E')
#define	SUPER_OP		 (tmop_rec.stat_flag [0] == 'S')

#define	SEL_PS_NULL	 ((struct PSPCT_PTR *) NULL)


	/*
	 * Special fields and flags
	 */
   	int  	envDbCo 			= 0,
			envDbFind 			= 0,
			valid_op			= 0,
			currentPrompt		= 0,
			nextPrompt			= 0,
			response			= 0,
			firstTime 			= TRUE,
			newLead				= 0,
			updateLeadCheck		= 0,
			UpdateFieldsCheck	= 0,
			exitCampaign		= 0,
			campaignCall 		= FALSE,
			randomCall 			= FALSE,
			receiveCall 		= FALSE,
			noToDelete			= 0,
			differentOperator	= 0,
			clear_ok			= 0,
			cl_5_ok				= 0,
			U_BOX 				= 12;

	char	branchNo [3],
			logName [15],
			systemDate [11];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct tmcfRecord	tmcf_rec;
struct tmxfRecord	tmxf_rec;
struct tmudRecord	tmud_rec;
struct tmopRecord	tmop_rec;
struct tmopRecord	tmop2_rec;
struct tmohRecord	tmoh_rec;
struct tmpmRecord	tmpm_rec;
struct tmahRecord	tmah_rec;
struct tmalRecord	tmal_rec;
struct tmchRecord	tmch_rec;
struct tmclRecord	tmcl_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;

	int		*tmsl_rep_goto	=	&tmsl_rec.rep1_goto;
	long	lsystemDate;

	char	*curr_user;
	char	*tmop2	=	"tmop2";

	extern	int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy [11];
	char	comment [2][66];
	char	prmpt_line [7][73];
	char	lead [9];
	char	lead_name [41];
	char	notes [61];
	char	last_call [61];
	char	ph_no [16];
	char	contact [31];
	char	alt_contact [31];
	char	response [2];
	char	rep_desc [8][26];
	char	best_ph_time [6];
	char	mail_addr [3][41];
	long	call_bk_date;
	char	call_bk_time [6];
	char	u_prmpt [12][15];
	char	user_value [12][61];
	char	re_desc [81];
	char	ok_keys [21];
	char	lead_desc [31];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campaign_no", 3, 2, INTTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number       ", "Enter Campaign no:", 
		YES, NO, JUSTLEFT, "", "", (char *)&tmcf_rec.campaign_no}, 
	{1, LIN, "camp_desc1", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Description  ", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name1}, 
	{1, LIN, "camp_desc2", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmcf_rec.c_name2}, 
	{2, LIN, "lead", 3, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "", "Lead Number     ", "Enter Lead Number To Call.", 
		NE, NO, JUSTLEFT, "", "", local_rec.lead}, 
	{2, LIN, "lead_name", 3, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.lead_desc, "", 
		NA, NO, JUSTLEFT, "", "", local_rec.lead_name}, 
	{2, LIN, "ph_no", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Phone Number    ", "Enter Phone Number.", 
		NO, NO, JUSTLEFT, "", "", local_rec.ph_no}, 
	{2, LIN, "contact", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Contact       ", " Enter Name Of Contact.", 
		NO, NO, JUSTLEFT, "", "", local_rec.contact}, 
	{2, LIN, "best_ph_time", 5, 2, CHARTYPE, 
		"AA:AA", "          ", 
		" ", "", "Best Phone Time ", "Enter Best Phone Time.", 
		NO, NO, JUSTLEFT, "", "", local_rec.best_ph_time}, 
	{2, LIN, "alt_contact", 5, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Alt. Contact  ", "Enter Name Of Alternate Contact.", 
		NO, NO, JUSTLEFT, "", "", local_rec.alt_contact}, 
	{2, LIN, "addr1", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Mail Address    ", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.mail_addr [0]}, 
	{2, LIN, "addr2", 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "------------    ", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.mail_addr [1]}, 
	{2, LIN, "addr3", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "------------    ", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.mail_addr [2]}, 
	{3, TXT, "notes", 7, 9, 0, 
		"", "          ", 
		"", "", " Notes ", "", 
		5, 60, 500, "", "", local_rec.notes}, 
	{4, TXT, "last_call_notes", 14, 9, 0, 
		"", "          ", 
		"", "", " Last Call Notes ", "", 
		5, 60, 500, "", "", local_rec.last_call}, 
	{5, LIN, "prmpt_line1", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text1}, 
	{5, LIN, "prmpt_line2", 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text2}, 
	{5, LIN, "prmpt_line3", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text3}, 
	{5, LIN, "prmpt_line4", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text4}, 
	{5, LIN, "prmpt_line5", 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text5}, 
	{5, LIN, "prmpt_line6", 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text6}, 
	{5, LIN, "prmpt_line7", 13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", tmsl_rec.text7}, 
	{5, LIN, "resp1", 15, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [0]}, 
	{5, LIN, "resp2", 15, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [1]}, 
	{5, LIN, "resp3", 16, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [2]}, 
	{5, LIN, "resp4", 16, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [3]}, 
	{5, LIN, "resp5", 16, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [4]}, 
	{5, LIN, "resp6", 17, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [5]}, 
	{5, LIN, "resp7", 17, 26, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [6]}, 
	{5, LIN, "resp8", 17, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_desc [7]}, 
	{5, LIN, "response", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "", "Enter Response  ", local_rec.re_desc, 
		YES, NO, JUSTLEFT, local_rec.ok_keys, "", local_rec.response}, 
	{5, LIN, "comment1", 19, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Comment  ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.comment [0]}, 
	{5, LIN, "comment2", 20, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " -------  ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.comment [1]}, 
	{6, LIN, "call_bk_date", 15, 18, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", " Callback Date: ", "", 
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.call_bk_date}, 
	{6, LIN, "call_bk_time", 16, 18, CHARTYPE, 
		"AA:AA", "          ", 
		" ", "", " Callback Time: ", "", 
		NO, NO, JUSTLEFT, "", "", local_rec.call_bk_time}, 
	{7, LIN, "user1", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [0], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [0]}, 
	{7, LIN, "user2", 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [1], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [1]}, 
	{7, LIN, "user3", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [2], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [2]}, 
	{7, LIN, "user4", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [3], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [3]}, 
	{7, LIN, "user5", 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [4], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [4]}, 
	{7, LIN, "user6", 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [5], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [5]}, 
	{7, LIN, "user7", 13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [6], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [6]}, 
	{7, LIN, "user8", 14, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [7], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [7]}, 
	{7, LIN, "user9", 15, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [8], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [8]}, 
	{7, LIN, "user10", 16, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [9], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [9]}, 
	{7, LIN, "user11", 17, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [10], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [10]}, 
	{7, LIN, "user12", 18, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", local_rec.u_prmpt [11], " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.user_value [11]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

long	callNumber			= 0L;

int		d_alloc [4096];

struct	PSPCT_ARY_PTR
{
	long	hhpm_hash;
	int		op_num;
};

struct	PSPCT_PTR
{
	int		last_used;
	struct	PSPCT_ARY_PTR	pspct_ary [1000];
	struct	PSPCT_PTR	*next;
};

MENUTAB call_menu [] =
{
	{ " 1. CAMPAIGN CALL      ",
	  " Make A Call For A Campaign" },
	{ " 2. RANDOM CALL        ",
	  " Make A Random Call." },
	{ " 3. RECEIVE CALL       ",
	  " Receive A Call From A Lead " },
	{ ENDMENU }
};

struct	PSPCT_PTR	*ps_head;
struct	PSPCT_PTR	*ps_curr;
struct	PSPCT_ARY_PTR	*ps_tmp;

char	*scn_desc [] = {
	"",
	"Header",
	"Notes",
	"Last Call"
};

int		AddTmpm 				 (void);
int 	CalcTime 				 (char *, char *);
int		CallBack 				 (void);
int 	CampaignCall 			 (void);
int 	FindScript 				 (void);
int 	GetCallNo 				 (void);
int 	GetCampainNo 			 (void);
int 	GetOperator 			 (void);
int 	heading 				 (int);
int 	LoadScriptLine 			 (int);
int 	ProcessCall 			 (void);
int		spec_valid 				 (int);
int 	TimeReached 			 (void);
void	ClearScreen 			 (int);
void	CloseDB 				 (void);
void	DeAllocate 				 (void);
void	EditLead  				 (void);
void	EraseEdit 				 (void);
void	FixEdge 				 (int line_no);
void	LoadTmxf 				 (char *);
void	LoadUserPrompts 		 (void);
void	OpenDB 					 (void);
void	RamdomCall 				 (int);
void	SetUpEdit 				 (int);
void	shutdown_prog 			 (void);
void	SrchTmpf 				 (char *);
void	SrchTmpm 				 (char *);
void	TmCallMenu 				 (void);
void	UpdateCallLine 			 (void);
void	UpdateLead 				 (void);
void	UpdateOperator 			 (void);
void	UpdateTime 				 (void);
void	UpdateTmxf 				 (char *);
void	UserFields 				 (void);

struct PSPCT_PTR *PsAlloc 		 (void);

/*
 * Main Processing Routine.
 */
int
main (
	int argc, char *argv [])
{
	int		i;
	char	*sptr = getenv ("LOGNAME");

	TruePosition	=	TRUE;
	if (argc > 1)
	{
		/*
		 * User has hot_keyed into receive a call.	 
		 */
		receiveCall = TRUE;
	}
	/*
	 * Set position of minimenu
	 */
	MENU_ROW = 2;
	MENU_COL = 0;

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	set_masks ();

	for (i = 0; i < 4; i++)
		tab_data [i]._desc = scn_desc [i];

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	OpenDB (); 	

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (sptr)
	{
		sprintf (logName, "%-14.14s", sptr);
		valid_op = GetOperator ();
	}
	else
	{
		errmess (ML (mlTmMess049));
		sleep (sleepTime);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	if (!valid_op)
	{
		errmess (ML (mlStdMess168));
		sleep (sleepTime);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	if (receiveCall)
		RamdomCall (FALSE);
	else
	{
		prog_exit = FALSE;
		while (!prog_exit)
		{
			campaignCall = FALSE;
			randomCall = FALSE;
			receiveCall = FALSE;
			clear_ok = TRUE;
			heading (0);
	
			TmCallMenu ();
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
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
	abc_dbopen ("data");

	abc_alias (tmop2, tmop);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (tmah, tmah_list, TMAH_NO_FIELDS, "tmah_id_no");
	open_rec (tmal, tmal_list, TMAL_NO_FIELDS, "tmal_id_no");
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_id_no");
	open_rec (tmcl, tmcl_list, TMCL_NO_FIELDS, "tmcl_id_no");
	open_rec (tmoh, tmoh_list, TMOH_NO_FIELDS, "tmoh_id_no");
	open_rec (tmop2,tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, "tmpm_id_no3");
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
	open_rec (tmud, tmud_list, TMUD_NO_FIELDS, "tmud_id_no");
	open_rec (tmxf, tmxf_list, TMXF_NO_FIELDS, "tmxf_id_no");
}	

/*
 * Close data base files .
 */
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (tmah);
	abc_fclose (tmal);
	abc_fclose (tmcf);
	abc_fclose (tmch);
	abc_fclose (tmcl);
	abc_fclose (tmoh);
	abc_fclose (tmop2);
	abc_fclose (tmop);
	abc_fclose (tmpm);
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_fclose (tmud);
	abc_fclose (tmxf);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	int		i;
	int		tmp_hour;
	int		tmp_min;
	char	tmp_time [6];
	char	*tptr;

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

	if (LCHECK ("lead"))
	{
		strcpy (local_rec.lead_desc, "");
		display_prmpt (label ("lead_name"));

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}

		newLead = FALSE;
		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		strcpy (tmpm_rec.pro_no,pad_num (local_rec.lead));
		cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "w");
		if (cc)
		{
			if (!receiveCall)
			{
				print_mess (ML (mlTmMess051));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			{
				newLead = TRUE;
				FLD ("lead_name") = NO;
				strcpy (local_rec.lead_desc, "Name                  ");
				display_prmpt (label ("lead_name"));

				if (!AddTmpm ())
				{
					print_mess (ML (mlTmMess052));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				return (EXIT_SUCCESS);
			}
		}

		differentOperator = FALSE;
		if (strcmp (tmpm_rec.op_code, tmop_rec.op_id))
		{
			if (!SUPER_OP)
			{
				print_mess (ML (mlTmMess009));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
				differentOperator = TRUE;
		}

		sprintf (local_rec.lead_name, "%-40.40s", tmpm_rec.name);
		sprintf (local_rec.ph_no, "%-15.15s", tmpm_rec.phone_no);
		sprintf (local_rec.contact, "%-30.30s", tmpm_rec.cont_name1);
		sprintf (local_rec.alt_contact, "%-30.30s", tmpm_rec.cont_name2);
		LoadTmxf (NOTES);
		LoadTmxf (LST_CALL);

		entry_exit = TRUE;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("response"))
	{
		if (END_CALL || SUSPEND_CALL || local_rec.response [0] == 'X')
		{
			if (local_rec.response [0] == 'X')
				exitCampaign = TRUE;

			nextPrompt = 999;
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		response = atoi (local_rec.response);
		if (tmsl_rep_goto [response - 1] != 0)
			nextPrompt = tmsl_rep_goto [response - 1];
		else
		{
			print_mess (ML (mlTmMess053));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("comment1"))
	{
		if (dflt_used)
			entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("call_bk_date"))
	{
		if (dflt_used)
		{
			local_rec.call_bk_date = lsystemDate;
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("call_bk_time"))
	{
		strcpy (tmp_time, TimeHHMM ());
		if (dflt_used)
		{
			strcpy (local_rec.call_bk_time, tmp_time);
			DSP_FLD ("call_bk_time");
			return (EXIT_SUCCESS);
		}

		/*
		 * Replace spaces with zeros
		 */
		sprintf (tmp_time, "%-5.5s", local_rec.call_bk_time);
		tptr = tmp_time;
		i = 0;
		while (*tptr)
		{
			if (*tptr == ' ' && i != 2)
				*tptr = '0';

			i++;
			tptr++;
		}
		sprintf (local_rec.call_bk_time, "%-5.5s", tmp_time);
		local_rec.call_bk_time [2] = ':';

		tmp_hour = atoi (local_rec.call_bk_time);
		tmp_min = atoi (local_rec.call_bk_time + 3);

		if (tmp_hour > 23 || tmp_min > 59)
		{
			print_mess (ML (mlStdMess142));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		DSP_FLD ("call_bk_time");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	

int
AddTmpm (void)
{
	sprintf (tmpm_rec.op_code,"%-14.14s", logName);
	strcpy (tmpm_rec.lst_op_code,"              ");
	strcpy (tmpm_rec.call_bk_time, "00:00");
	tmpm_rec.call_bk_date = 0L;
	tmpm_rec.call_no = 0L;
	strcpy (tmpm_rec.active_flg,"N");
	strcpy (tmpm_rec.delete_flag,"N");
	strcpy (tmpm_rec.tax_code, " ");
	strcpy (tmpm_rec.tax_no, "                ");
	strcpy (tmpm_rec.stat_flag, " ");
	strcpy (tmpm_rec.acronym, "         ");
	strcpy (tmpm_rec.area_code, "  ");
	strcpy (tmpm_rec.sman_code, "  ");
	strcpy (tmpm_rec.b_sector, "   ");
	sprintf (tmpm_rec.cont_name3, "%-30.30s", " ");
	sprintf (tmpm_rec.cont_name4, "%-30.30s", " ");
	sprintf (tmpm_rec.cont_name5, "%-30.30s", " ");
	sprintf (tmpm_rec.cont_name6, "%-30.30s", " ");
	sprintf (tmpm_rec.cont_code3, "%-3.3s", " ");
	sprintf (tmpm_rec.cont_code4, "%-3.3s", " ");
	sprintf (tmpm_rec.cont_code5, "%-3.3s", " ");
	sprintf (tmpm_rec.cont_code6, "%-3.3s", " ");
	sprintf (tmpm_rec.del1_adr, "%-40.40s", " ");
	sprintf (tmpm_rec.del2_adr, "%-40.40s", " ");
	sprintf (tmpm_rec.del3_adr, "%-40.40s", " ");
	sprintf (tmpm_rec.fax_no, "%-16.16s", " ");
	tmpm_rec.phone_freq = 0;
	sprintf (tmpm_rec.n_phone_time, "%-5.5s", " ");
	tmpm_rec.n_phone_date = 0L;
	tmpm_rec.visit_freq = 0;
	tmpm_rec.n_visit_date = 0L;
	sprintf (tmpm_rec.n_visit_time, "%-5.5s", " ");
	strcpy (tmpm_rec.mail_flag, "Y");
	strcpy (tmpm_rec.origin, "   ");
	sprintf (tmpm_rec.best_ph_time, "%-5.5s", " ");

	tmpm_rec.date_create = TodaysDate ();
	strcpy (tmpm_rec.pro_no,pad_num (local_rec.lead));
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	tmpm_rec.lphone_date = 0L;
	cc = abc_add (tmpm,&tmpm_rec);
	if (cc) 
		return (FALSE);

	strcpy (tmpm_rec.pro_no,pad_num (local_rec.lead));
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	cc = find_rec (tmpm,&tmpm_rec, COMPARISON, "u");
	if (cc) 
		return (FALSE);

	return (TRUE);
}

void
LoadTmxf (
	char *load_type)
{
	int		lcl_scn;

	/*
	 * Set screen N - for putval.
	 */
	if (!strcmp (load_type, NOTES))
		lcl_scn = 3;
	else
	{
		if (!strcmp (load_type, LST_CALL))
			lcl_scn = 4;
		else
			lcl_scn = 7;
	}

	scn_set (lcl_scn);
	lcount [lcl_scn] = 0;

	tmxf_rec.hhpm_hash 	= tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, load_type);
	tmxf_rec.line_no 	= 0;
	cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	while (!cc && 
	       tmxf_rec.hhpm_hash == tmpm_rec.hhpm_hash &&
	       !strcmp (tmxf_rec.type, load_type))
	{
		if (!strcmp (load_type, NOTES))
		{
			sprintf (local_rec.notes, "%-60.60s", tmxf_rec.desc);
		}
		else
		{
			if (!strcmp (load_type, LST_CALL))
			{
				sprintf (local_rec.last_call, "%-60.60s", tmxf_rec.desc);
			}
			else
			{
				sprintf (local_rec.user_value [tmxf_rec.line_no], "%-60.60s", 
									tmxf_rec.desc);
			}
		}

		if (strcmp (load_type, USER_DEF))
			putval (lcount [lcl_scn]++);

		cc = find_rec (tmxf, &tmxf_rec, NEXT, "r");
	}

	return;
}

void
UpdateTmxf (
	char	*loadType)
{
	int		lcl_scn;
	int		line_cnt;

	/*
	 * Set screen N - for putval.
	 */
	if (!strcmp (loadType, NOTES))
		lcl_scn = 3;
	else
	{
		if (!strcmp (loadType, LST_CALL))
			lcl_scn = 4;
		else
			lcl_scn = 7;
	}

	if (lcl_scn == 7)
		lcount [7] = U_BOX;

	scn_set (lcl_scn);

	for (line_cnt = 0;line_cnt < lcount [lcl_scn] ;line_cnt++)
	{
		if (strcmp (loadType, USER_DEF))
			getval (line_cnt);

		tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
		strcpy (tmxf_rec.type, loadType);
		tmxf_rec.line_no = line_cnt;

		cc = find_rec (tmxf,&tmxf_rec,COMPARISON,"u");
		if (cc)
		{
			if (!strcmp (loadType, NOTES)) 
				sprintf (tmxf_rec.desc, "%-60.60s", local_rec.notes);
			else
			{
				if (!strcmp (loadType, LST_CALL))
					sprintf (tmxf_rec.desc, "%-60.60s", local_rec.last_call);
				else
					sprintf (tmxf_rec.desc, "%-60.60s", local_rec.user_value [line_cnt]);
			}

			cc = abc_add (tmxf,&tmxf_rec);
			if (cc)
				file_err (cc, tmxf, "DBADD");
		}
		else
		{
			if (!strcmp (loadType, NOTES))
				sprintf (tmxf_rec.desc, "%-60.60s", local_rec.notes);
			else
			{
				if (!strcmp (loadType, LST_CALL))
				{
					sprintf (tmxf_rec.desc, "%-60.60s", local_rec.last_call);
				}
				else
				{
					sprintf (tmxf_rec.desc, "%-60.60s", local_rec.user_value [line_cnt]);
				}
			}
			cc = abc_update (tmxf,&tmxf_rec);
			if (cc)
				file_err (cc, tmxf, "DBUPDATE");
		}
	}

	tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, loadType);
	tmxf_rec.line_no = line_cnt;

	cc = find_rec (tmxf,&tmxf_rec,GTEQ, "r");
	while (!cc && tmxf_rec.hhpm_hash == tmpm_rec.hhpm_hash &&
	       		!strcmp (tmxf_rec.type, loadType))
	{
		abc_delete (tmxf);

		tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
		strcpy (tmxf_rec.type, loadType);
		tmxf_rec.line_no = line_cnt;
		
		cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	}
	return;
}

/*
 * Assign mini menu.
 */
void
TmCallMenu (void)
{
	for (;;)
	{
		campaignCall = FALSE;
	    	mmenu_print ("  TELEMARKETING CALLS  ", call_menu, 0);
	    	switch (mmenu_select (call_menu))
	    	{
		case CAMPAIGN :
			campaignCall = TRUE;
			if (CampaignCall () == EXIT_FAILURE)
				prog_exit = TRUE;
			return;

		case RANDOM :
			RamdomCall (TRUE);
			return;

		case RECEIVE :
			RamdomCall (FALSE);
			return;

		case RESTRT :
			restart = TRUE;
			return;
	
		case PG_EXIT :
			prog_exit = TRUE;
			return;
	
		default :
			break;
	    }
	}
}

/*
 * Update operator file 
 */
void
UpdateOperator (void)
{
	cc = abc_update (tmop, &tmop_rec);
	if (cc)
		file_err (cc, tmop, "DBUPDATE");

	if (!GetOperator ())
		file_err (cc, tmop, "DBFIND");

	return;
}

/*
 * Lookup operator file
 */
int
GetOperator (void)
{
	char	tmp_op [15];

	sprintf (tmp_op, "%-14.14s", logName);
	upshift (tmp_op);

	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", tmp_op);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "u");
	if (cc)
		return (FALSE);

	return (TRUE);
}

/*
 * Lookup operator file to find correct script for a 
 * lead who is allocated to a different operator.
 */
int
FindScript (void)
{
	strcpy (tmop2_rec.co_no, comm_rec.co_no);
	sprintf (tmop2_rec.op_id, "%-14.14s", tmpm_rec.op_code);
	cc = find_rec (tmop2, &tmop2_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	return (TRUE);
}

/*
 * Process Calls For A Campaign
 */
int
CampaignCall (void)
{
	noToDelete = 0;

	strcpy (local_rec.re_desc, " Enter 1 - 8 . E(nd) Call. S(uspend) Call. eX(it) Campaign Call ");
	strcpy (local_rec.ok_keys, "0123456789ESX");

	exitCampaign = FALSE;

	abc_selfield (tmpm, "tmpm_hhpm_hash");
	if (GetCampainNo () == EXIT_FAILURE)
	{
		return (EXIT_FAILURE);
	}

	strcpy (tmop_rec.login_time, TimeHHMM ());

	/*
	 * Read Lead Allocation File
	 */
	tmah_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	tmah_rec.hhop_hash = tmop_rec.hhop_hash;
	cc = find_rec (tmah, &tmah_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlTmMess014));
		sleep (sleepTime);
		return (FALSE);
	}
	
	tmal_rec.hhla_hash 	= tmah_rec.hhla_hash;
	tmal_rec.line_no 	= 0;
	cc = find_rec (tmal, &tmal_rec, GTEQ, "r");

	while (!cc && !exitCampaign && tmal_rec.hhla_hash == tmah_rec.hhla_hash)
	{
		init_vars (2);
		init_vars (3);
		init_vars (4);

		tmpm_rec.hhpm_hash = tmal_rec.hhpm_hash;
		cc = find_rec (tmpm, &tmpm_rec, EQUAL, "u");
		if (cc)
		{
			cc = find_rec (tmal, &tmal_rec, NEXT, "r");
			continue;
		}

		if (CallBack () && !TimeReached ())
		{
			abc_unlock (tmpm);
			cc = find_rec (tmal, &tmal_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.lead, "%-8.8s", tmpm_rec.pro_no);
		sprintf (local_rec.lead_name, "%-40.40s", tmpm_rec.name);
		sprintf (local_rec.ph_no, "%-15.15s", tmpm_rec.phone_no);
		sprintf (local_rec.contact, "%-30.30s", tmpm_rec.cont_name1);
		sprintf (local_rec.alt_contact, "%-30.30s", tmpm_rec.cont_name2);
		LoadTmxf (NOTES);
		LoadTmxf (LST_CALL);

		updateLeadCheck = FALSE;
		SetUpEdit (FALSE);

		clear_ok = TRUE;
		EditLead ();

		if (restart)
		{
			abc_unlock (tmpm);
			exitCampaign = TRUE;
			restart = FALSE;
			continue;
		}

		if (!ProcessCall ())
		{
			cc = find_rec (tmal, &tmal_rec, NEXT, "r");
			continue;
		}

		/*
		 * Updated lead details.
		 */
		UpdateLead ();

		/*
		 * Updated time details.
		 */
		UpdateTime ();

		cc = find_rec (tmal, &tmal_rec, NEXT, "r");
	}
	
	abc_selfield (tmpm, "tmpm_id_no");
	exitCampaign = FALSE;

	/*
	 * Delete tmal records
	 */
	DeAllocate ();

	return (EXIT_SUCCESS);
}

/*
 * Check if callback date/time has been reached or exceeded.
 */
int
TimeReached (void)
{
	char	currentTime [6];

	strcpy (currentTime, TimeHHMM ());
	if (lsystemDate >= tmpm_rec.call_bk_date && 
	     strcmp (currentTime, tmpm_rec.call_bk_time) >= 0)
		return (TRUE);

	return (FALSE);
}

/*
 * Process A Call To A Random Lead
 */
void
RamdomCall (
	int call_type)
{
	if (call_type)
		randomCall = TRUE;
	else
		receiveCall = TRUE;

	strcpy (tmop_rec.login_time, TimeHHMM ());

	strcpy (local_rec.re_desc, " Enter 1 - 8 . E(nd) Call. S(uspend) Call. ");
	strcpy (local_rec.ok_keys, "0123456789ES");

	while (!prog_exit)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (2);
		init_vars (3);
		init_vars (4);

		clear_ok = TRUE;

		updateLeadCheck = !call_type;
		SetUpEdit (!call_type);

		/*
		 * Enter screen 1 linear input.
		 */
		FLD ("lead_name") = NA;
		strcpy (local_rec.lead_desc, "");
		heading (2);
		entry (2);
		if (prog_exit || restart)
		{
			if (!newLead)
				abc_unlock (tmpm);
			continue;
		}
		FLD ("lead_name") = NA;
		strcpy (local_rec.lead_desc, "");

		EditLead ();

		if (restart)
		{
			abc_unlock (tmpm);
			continue;
		}

		if (!ProcessCall ())
			continue;

		UpdateLead ();

		if (receiveCall)
			break;
	}

	prog_exit = FALSE;

	UpdateTime ();

	return;
}

void
UpdateTime (void)
{
	char	logout_time [6];

	/*
	 * Update times on operator file
	 */
	strcpy (logout_time, TimeHHMM ());
	tmoh_rec.hhcf_hash = tmcf_rec.hhcf_hash;
	tmoh_rec.hhop_hash = tmop_rec.hhop_hash;
	cc = find_rec (tmoh, &tmoh_rec, COMPARISON, "u");
	if (cc)
	{
		tmoh_rec.campgn_time = 0L;
		tmoh_rec.campgn_time += CalcTime (tmop_rec.login_time, 
						     logout_time);
		cc = abc_add (tmoh, &tmoh_rec);
		if (cc)
			file_err (cc, tmoh, "DBUPDATE");
	}
	else
	{
		tmoh_rec.campgn_time += CalcTime (tmop_rec.login_time,logout_time);
		cc = abc_update (tmoh, &tmoh_rec);
		if (cc)
			file_err (cc, tmoh, "DBUPDATE");
	}
	strcpy (tmop_rec.login_time, "00:00");
	sprintf (tmop_rec.curr_l_no, "%-8.8s", " ");
	tmop_rec.curr_c_no = 0L;
	tmop_rec.curr_prmpt = 0;

	/*
	 * Update operator file 
	 */
	UpdateOperator ();

	return;
}

void
SetUpEdit (
	int edit_status)
{
	if (edit_status)
	{
		FLD ("addr1") 				= NO;
		FLD ("addr2") 				= NO;
		FLD ("addr3") 				= NO;
		SCN_ROW ("notes")			= 9;
		SCN_ROW ("last_call_notes") = 4;
		SCN_ROW ("last_call_notes")	= 15;
	}
	else
	{
		FLD ("addr1") 				= ND;
		FLD ("addr2") 				= ND;
		FLD ("addr3") 				= ND;
		SCN_ROW ("notes")			= 7;
		SCN_ROW ("last_call_notes") = 5;
		SCN_ROW ("last_call_notes")	= 14;
	}
}

/*
 * Edit lead details including Notes and Last Call Notes.
 */
void
EditLead (void)
{
	if (updateLeadCheck)
	{
		UpdateTmxf (NOTES);
		UpdateTmxf (LST_CALL);

		SetUpEdit (TRUE);

		init_vars (3);
		init_vars (4);

		if (!newLead)
		{
			LoadTmxf (NOTES);
			LoadTmxf (LST_CALL);
	
			clear_ok = TRUE;
	
			sprintf (local_rec.mail_addr [0], "%-40.40s",tmpm_rec.mail1_adr);
			sprintf (local_rec.mail_addr [1], "%-40.40s",tmpm_rec.mail2_adr);
			sprintf (local_rec.mail_addr [2], "%-40.40s",tmpm_rec.mail3_adr);
			sprintf (local_rec.best_ph_time,  "%-5.5s",  tmpm_rec.best_ph_time);
		}
	}

	heading (2);
	scn_display (2);
	edit (2);
	if (prog_exit || restart)
	{
		abc_unlock (tmpm);
		return;
	}

	scn_write (2);

	scn_display (3);
	scn_display (4);

	no_edit (1);
	no_edit (5);
	no_edit (6);
	no_edit (7);
	edit_ok (2);
	edit_ok (3);
	edit_ok (4);

	clear_ok = FALSE;
	edit_all ();
}

void
UserFields (void)
{
	LoadTmxf (USER_DEF);
	heading (7);
	scn_display (7);
	edit (7);

	if (restart)
	{
		UpdateFieldsCheck = FALSE;
		restart = FALSE;
	}

	return;
}

/*
 * Make a call to a lead
 */
int
ProcessCall (void)
{
	nextPrompt = -1;

	if (!GetCallNo ())
		return (FALSE);

	/*
	 * Update operator details
	 */
	sprintf (tmop_rec.curr_l_no, "%-8.8s", tmpm_rec.pro_no);
	tmop_rec.curr_c_no = callNumber;

	/*
	 * Update operator file 
	 */
	UpdateOperator ();

	cl_5_ok = TRUE;

	/*
	 * Process script
	 */
	while (nextPrompt != 999)
	{
		init_vars (5);

		if (!LoadScriptLine (nextPrompt))
		{
		       errmess (ML (mlTmMess017));
		       sleep (sleepTime);
		       nextPrompt = 999;
		       continue;
		}

		tmop_rec.curr_prmpt = tmsl_rec.prmpt_no;

		/*
		 * Update operator file 
		 */
		UpdateOperator ();

		currentPrompt = tmsl_rec.prmpt_no;
		heading (5);
		scn_display (5);
		init_ok = FALSE;
		cl_5_ok = TRUE;
		
		entry (5);

		if (nextPrompt == 999 && !SUSPEND_CALL && campaignCall)
			d_alloc [noToDelete++] = tmal_rec.line_no;

		if (restart)
		{
			nextPrompt = 999;
			restart = FALSE;
			continue;
		}

		if (SUSPEND_CALL)
		{
			init_vars (6);
			ClearScreen (6);
			heading (6);
			entry (6);

			if (restart)
			{
				restart = FALSE;
				nextPrompt = currentPrompt;
				strcpy (local_rec.response, " ");
			}
			else
			{
				print_mess (ML (mlTmMess015));
				sleep (sleepTime);
				return (TRUE);
			}

			ClearScreen (6);
			scn_write (5);
			scn_display (5);
			cl_5_ok = FALSE;
		}
		else
			UpdateCallLine ();

		init_ok = TRUE;
	}
	return (TRUE);
}

/*
 * Get Number Of Current Call
 */
int
GetCallNo (void)
{
	int		tmp_prmpt_no = 0,
			tmp_rep_no 	 = 0;

	/*
	 * Read script header. 
	 */
	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	if (randomCall || campaignCall)
		tmsh_rec.script_no = tmop_rec.out_scr_no;
	else
	{
		if (differentOperator)
			tmsh_rec.script_no = tmop2_rec.in_scr_no;
		else
			tmsh_rec.script_no = tmop_rec.in_scr_no;
	}

	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlTmMess016));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	if (!CallBack ())
	{
		/*
		 * Get next call number from comr.	      
		 */
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");
	
		callNumber = comr_rec.nx_call_no++;
		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		/*
		 * Create call record. 
		 */
		strcpy (tmch_rec.co_no, comm_rec.co_no);
		tmch_rec.call_no = callNumber;
		tmch_rec.call_date = lsystemDate;
		tmch_rec.hhpm_hash = tmpm_rec.hhpm_hash;
		tmch_rec.hhcf_hash = tmcf_rec.hhcf_hash;
		tmch_rec.hhop_hash = tmop_rec.hhop_hash;
		tmch_rec.hhsh_hash = tmsh_rec.hhsh_hash;
		if (receiveCall)
			strcpy (tmch_rec.io_flag, "I");
		else
			strcpy (tmch_rec.io_flag, "O");
	
		cc = abc_add (tmch, &tmch_rec);
		if (cc)
			file_err (cc, tmch, "DBADD");

	}
	else
		callNumber = tmpm_rec.call_no;

	tmch_rec.call_no = callNumber;

	/*
	 * Read call header. 
	 */
	strcpy (tmch_rec.co_no, comm_rec.co_no);
	cc = find_rec (tmch, &tmch_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, tmch, "DBFIND");

	tmcl_rec.line_no = 0;

	if (CallBack ())
	{
		tmcl_rec.hhcl_hash = tmch_rec.hhcl_hash;

		cc = find_rec (tmcl, &tmcl_rec, GTEQ, "r");
		if (cc || (!cc && tmcl_rec.hhcl_hash != tmch_rec.hhcl_hash))
		{
			nextPrompt = 1;
			return (TRUE);
		}

		while (!cc && tmcl_rec.hhcl_hash == tmch_rec.hhcl_hash)
		{
			tmp_prmpt_no = tmcl_rec.prmpt_no;
			tmp_rep_no = tmcl_rec.rep_no;
			cc = find_rec (tmcl, &tmcl_rec, NEXT, "r");
		}

		if (!LoadScriptLine (tmp_prmpt_no))
		{
		       errmess (ML (mlTmMess017));
		       sleep (sleepTime);
		       nextPrompt = 999;
		       return (FALSE);
		}
		nextPrompt = tmsl_rep_goto [tmp_rep_no - 1];
	}
	return (TRUE);
}

/*
 * Update call with chosen response etc
 */
void
UpdateCallLine (void)
{
	tmcl_rec.hhcl_hash 	= tmch_rec.hhcl_hash;
	tmcl_rec.line_no++;
	tmcl_rec.prmpt_no 	= tmop_rec.curr_prmpt;
	tmcl_rec.rep_no 	= response;
	sprintf (tmcl_rec.rep1_text, "%-65.65s", local_rec.comment [0]);
	sprintf (tmcl_rec.rep2_text, "%-65.65s", local_rec.comment [1]);
	cc = abc_add (tmcl, &tmcl_rec);
	if (cc)
		file_err (cc, tmcl, "DBADD");

	return;
}

/*
 * Load Required Line From Current Script
 */
int
LoadScriptLine (
	int		nextPrompt)
{
	int		i;

	tmsl_rec.hhsh_hash 	= tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no 	= nextPrompt;

	if (FIRST_PRMPT)
		cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
	else 
		cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");

	if (cc)
		return (FALSE);
	else
	{
		if (FIRST_PRMPT && tmsl_rec.hhsh_hash != tmsh_rec.hhsh_hash)
			return (FALSE);
	}


	sprintf (local_rec.rep_desc [0], "1 : %-20.20s", tmsl_rec.rep1_desc);
	sprintf (local_rec.rep_desc [1], "2 : %-20.20s", tmsl_rec.rep2_desc);
	sprintf (local_rec.rep_desc [2], "3 : %-20.20s", tmsl_rec.rep3_desc);
	sprintf (local_rec.rep_desc [3], "4 : %-20.20s", tmsl_rec.rep4_desc);
	sprintf (local_rec.rep_desc [4], "5 : %-20.20s", tmsl_rec.rep5_desc);
	sprintf (local_rec.rep_desc [5], "6 : %-20.20s", tmsl_rec.rep6_desc);
	sprintf (local_rec.rep_desc [6], "7 : %-20.20s", tmsl_rec.rep7_desc);
	sprintf (local_rec.rep_desc [7], "8 : %-20.20s", tmsl_rec.rep8_desc);

	for (i = 0; i < 8; i++)
	{
		sprintf (err_str, "resp%1d", i + 1);
		FLD (err_str) = (tmsl_rep_goto [i] == 0) ? ND : NA;
	}
	return (TRUE);
}

/*
 * Update lead details
 */
void
UpdateLead (void)
{
	int		i;

	if (newLead)
		sprintf (tmpm_rec.name, "%-40.40s", local_rec.lead_name);

	if (!SUSPEND_CALL)
	{
		crsr_on ();
		clear_mess ();
		i = prmptmsg (ML (mlTmMess018), "YyNn", 0, 23);
		if (i == 'Y' || i == 'y')
		{
			updateLeadCheck = TRUE;
			clear_ok = TRUE;
			EditLead ();
		}

		init_vars (7);
		LoadUserPrompts ();
		if (U_BOX)
		{
			crsr_on ();
			clear_mess ();
			i = prmptmsg (ML (mlTmMess019), "YyNn", 0, 23);
			if (i == 'Y' || i == 'y')
			{
				UpdateFieldsCheck = TRUE;
				UserFields ();
			}
		}
	}

	sprintf (tmpm_rec.phone_no, "%-15.15s", local_rec.ph_no);
	sprintf (tmpm_rec.cont_name1, "%-30.30s", local_rec.contact);
	sprintf (tmpm_rec.cont_name2, "%-30.30s", local_rec.alt_contact);

	if (updateLeadCheck)
	{
		sprintf (tmpm_rec.best_ph_time, "%-5.5s", local_rec.best_ph_time);
		sprintf (tmpm_rec.mail1_adr, "%-40.40s", local_rec.mail_addr [0]);
		sprintf (tmpm_rec.mail2_adr, "%-40.40s", local_rec.mail_addr [1]);
		sprintf (tmpm_rec.mail3_adr, "%-40.40s", local_rec.mail_addr [2]);
	}

	if (SUSPEND_CALL)
	{
		sprintf (tmpm_rec.call_bk_time, "%-5.5s", local_rec.call_bk_time);
		tmpm_rec.call_bk_date = local_rec.call_bk_date;
		tmpm_rec.call_no = callNumber;
	}
	else
	{
		sprintf (tmpm_rec.op_code, "%-14.14s", " ");
		upshift (logName);
		sprintf (tmpm_rec.lst_op_code, "%-14.14s", logName);
		strcpy (tmpm_rec.call_bk_time, "00:00");
		tmpm_rec.call_bk_date = 0L;
		tmpm_rec.call_no = 0L;
	}

	/*
	 * Update tmpm
	 */
	cc = abc_update (tmpm, &tmpm_rec);
	if (cc)
		file_err (cc, tmpm, "DBUPDATE");
	
	UpdateTmxf (NOTES);
	UpdateTmxf (LST_CALL);

	if (UpdateFieldsCheck)
		UpdateTmxf (USER_DEF);

	return;
}

/*
 * Delete record from allocation file
 */
void
DeAllocate (void)
{
	int		i;

	for (i = 0; i < noToDelete; i++)
	{
		tmal_rec.hhla_hash 	= tmah_rec.hhla_hash;
		tmal_rec.line_no 	= d_alloc [i];
		cc = find_rec (tmal, &tmal_rec, COMPARISON, "u");
		if (cc)
			continue;

		cc = abc_delete (tmal);
	}
	return;
}

/*
 * Calculate duration of login
 */
int
CalcTime (
	char *in_time,
	char *out_time)
{
	int		tmp_hh;
	int		tmp_mm;

	tmp_hh = atoi (out_time) - atoi (in_time);
	tmp_mm = atoi (out_time + 3) - atoi (in_time + 3);

	if (tmp_mm < 0)
	{
		tmp_hh--;
		tmp_mm += 60;
	}
	return ((tmp_hh * 60) + tmp_mm);
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
	save_rec ("#No.","#Campaign Description.");
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
		file_err (cc, tmcf, "DBFIND");
}

int
heading (
	int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	if ((scn == 1 && clear_ok) || scn == 0)
		clear ();

	if (scn == 1 || scn == 0)
	{
		if (scn == 1)
			box (0,2,80,3);

		rv_pr (ML (mlTmMess020),30,0,1);
	}
		
	if (scn == 2)
	{
		if (clear_ok)
			clear ();
		else
		{
			EraseEdit ();
			scn_display (3);
#ifndef		GVISION
			if (updateLeadCheck)
				rv_pr ("                                                                              ", 1, 9, 0); 
#endif
		}

		if (CallBack ())
			rv_pr (ML (mlTmMess021),20,0,1);
		else
		{
			if (randomCall)
				rv_pr (ML (mlTmMess050),25,0,1);
			else
				if (campaignCall)
					rv_pr (ML (mlTmMess022),25,0,1);
				else
					rv_pr (ML (mlTmMess023),25,0,1);
		}

		if (updateLeadCheck)
			box (0, 2, 80, 7);
		else
			box (0, 2, 80, 3);

		GetCampainNo ();

		sprintf (err_str,ML (mlTmMess024),
							tmcf_rec.campaign_no,
							tmcf_rec.c_name1,
							tmcf_rec.c_name2);
		rv_pr (err_str,5,2,1);
	}
	
	if (scn == 3 || scn == 4)
	{
		EraseEdit ();
		if (scn != 3)
			scn_display (3);

		return (EXIT_SUCCESS);
	}

	if (scn == 5)
	{
		if (cl_5_ok)
		{
			EraseEdit ();
			ClearScreen (5);
		}

		box (0, 6, 80, 14);

		sprintf (err_str, " %-*.*s ", (int) strlen (clip (tmsl_rec.desc)),
								   (int) strlen (clip (tmsl_rec.desc)),
								     tmsl_rec.desc);

		rv_pr (err_str, (80 - strlen (tmsl_rec.desc))/2 , 6, 1);

		FixEdge (6);

		line_at (14,0,80);
		FixEdge (14);

		line_at (18,0,80);
		FixEdge (18);
	}

	if (scn == 6)
	{
		box (1, 14, 30, 2);

		rv_pr (ML (mlTmMess025), 11, 14, 1);
	}

	if (scn == 7)
	{
		EraseEdit ();
		ClearScreen (5);
		box (0, 6, 80, U_BOX);
		FixEdge (6);
	}

	if (scn != 5 && scn != 6)
	{
		line_at (1,0,80);
		line_at (21,0,80);
	}

	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
EraseEdit (void)
{
	int		i;

#ifndef		GVISION
	for (i = Y_EALL; i < Y_EALL + 7; i++)
		rv_pr ("                 ", X_EALL, i, 0);
#endif

	scn_write (2);
	scn_display (2);
	if (updateLeadCheck)
		box (0, 2, 80, 7);
	else
		box (0, 2, 80, 3);

	sprintf (err_str, ML (mlTmMess024),
									tmcf_rec.campaign_no,
									tmcf_rec.c_name1,
									tmcf_rec.c_name2);
	rv_pr (err_str,5,2,1);

	return;
}

void
ClearScreen (
	int scn_no)
{
	int		i;

	if (scn_no == 5)
	{
		for (i = 6; i < 21; i++)
		{
			move (0,i);
			cl_line ();
		}
	}

	if (scn_no == 6)
	{
#ifndef		GVISION
		move (1,14);
		line (40);
		for (i = 15; i < 18; i++)
			rv_pr ("                             ", 1, i, 0);
#endif
	}

	return;

}

/*
 * Load User Defined Prompts
 */
void
LoadUserPrompts (void)
{
	int		i = 0;
	char	wk_fldname [7];

	strcpy (tmud_rec.co_no, comm_rec.co_no);
	tmud_rec.line_no = 0;
	cc = find_rec (tmud, &tmud_rec, GTEQ, "r");
	while (!cc && !strcmp (tmud_rec.co_no, comm_rec.co_no) && i < 12)
	{
		sprintf (wk_fldname, "user%d", i + 1);
		strcpy (local_rec.u_prmpt [i++],tmud_rec.prmpt_desc);

		FLD (wk_fldname) = NO;

		cc = find_rec (tmud, &tmud_rec, NEXT, "r");
	}
	U_BOX = i;
}

/*
 * Check if call is new or call back
 */
int
CallBack (void)
{
	if (strcmp (tmpm_rec.call_bk_time, "00:00") ||
            tmpm_rec.call_bk_date != 0L)
		return (TRUE);
	
	return (FALSE);
}

/*
 * Lookup Campaign File 
 */
int
GetCampainNo (void)
{
	int		find_status = FALSE;

	if (receiveCall && differentOperator)
		find_status = FindScript ();

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	if (find_status)
		tmcf_rec.campaign_no = tmop2_rec.campaign_no;
	else
	{
		tmcf_rec.campaign_no = tmop_rec.campaign_no;
		differentOperator = FALSE;
	}

	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlTmMess026));
		sleep (sleepTime);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Fix edge of boxes with 'T' junctions
 */
void
FixEdge (
	int		line_no)
{
	move (0,line_no);
	PGCHAR (10);
	move (79,line_no);
	PGCHAR (11);
	return;
}

/*
 * Allocate memory for prospect linked list
 */
struct	PSPCT_PTR *
PsAlloc (void)
{
	struct	PSPCT_PTR	*lcl_ptr;

	if (firstTime || ps_curr->last_used > 999)
	{
		lcl_ptr = (struct PSPCT_PTR *) malloc (sizeof (struct PSPCT_PTR));

		if (lcl_ptr == SEL_PS_NULL)
			sys_err ("Error during (MALLOC)", errno, PNAME);

		lcl_ptr->next = SEL_PS_NULL;
		if (firstTime)
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

		firstTime = FALSE;
	}

	ps_tmp = &ps_curr->pspct_ary [ps_curr->last_used++];

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
