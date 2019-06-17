/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_pro_inp.c,v 5.4 2001/11/12 07:07:27 scott Exp $
|  Program Name  : (tm_pro_inp.c)
|  Program Desc  : (Add / Change Tele-marketing prospects)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 05/01/87         |
|---------------------------------------------------------------------|
| $Log: tm_pro_inp.c,v $
| Revision 5.4  2001/11/12 07:07:27  scott
| Updated to convert to app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_pro_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_pro_inp/tm_pro_inp.c,v 5.4 2001/11/12 07:07:27 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <ml_tm_mess.h>
#include <ml_std_mess.h>
#define	HEADER_SCN	1
#define	TELE_SCN	2
#define	NOTES_SCN	3
#define	USER_SCN	4

#define	NOT_FND		0
#define	NRM_REP		1
#define	SUP_REP		2

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newProspect 	= FALSE,
			envDbCo 		= 0,
			envDbFind 		= 0,
			U_BOX 			= 12,
			byReportType 	= FALSE,
			reportStatus 	= NOT_FND;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct exclRecord	excl_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct tmpmRecord	tmpm_rec;
struct tmxfRecord	tmxf_rec;
struct tmofRecord	tmof_rec;
struct tmudRecord	tmud_rec;
struct tmpfRecord	tmpf_rec;

	char	*scn_desc [] = {
		"Screen One.",
		"Screen Two.",
		"Prospect notes.",
		"User Defines fields."
	};

char	*currentUser;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	prev_pro [9];
	char 	cur_pro [9];
	char 	con [6][31];
	char	data_str [61];
	char	u_prmpt [12][15];
	char	user_value [12][61];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "pro_no",	 4, 18, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Lead/Prospect No", "Enter Lead/Prospect. [SEARCH] Search.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cur_pro},
	{1, LIN, "acronym",	 4, 58, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "Acronym         ", "Enter customers acronym. Duplicates are not allowed. ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.acronym},
	{1, LIN, "name",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name            ", " ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.name},
	{1, LIN, "mail_adr1",	 7, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Mail to Address ", " ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.mail1_adr},
	{1, LIN, "mail_adr2",	 8, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "---------------:", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.mail2_adr},
	{1, LIN, "mail_adr3",	 9, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "---------------:", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.mail3_adr},
	{1, LIN, "del_adr1",	10, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Address", "Enter Same if Delivery = Mail Address",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.del1_adr},
	{1, LIN, "del_adr2",	11, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "---------------:", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.del2_adr},
	{1, LIN, "del_adr3",	12, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "---------------:", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.del3_adr},
	{1, LIN, "cont_name1",	14, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (1)", "First contact name is required. ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name1},
	{1, LIN, "cont_code1",	14, 72, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code1},
	{1, LIN, "cont_name2",	15, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (2)", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name2},
	{1, LIN, "cont_code2",	15, 72, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code2},
	{1, LIN, "cont_name3",	16, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (3)", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name3},
	{1, LIN, "cont_code3",	16, 72, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code3},
	{1, LIN, "cont_name4",	17, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (4)", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name4},
	{1, LIN, "cont_code4",	17, 72, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code4},
	{1, LIN, "cont_name5",	18, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (5)", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name5},
	{1, LIN, "cont_code5",	18, 72, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code5},
	{1, LIN, "cont_name6",	19, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Contact name (6)", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_name6},
	{1, LIN, "cont_code6",	19, 72, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.cont_code6},
	{2, LIN, "phoneno",	 4, 18, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Phone No.", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.phone_no},
	{2, LIN, "fax_no",	 4, 60, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Fax No.", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.fax_no},
	{2, LIN, "post_code",	 5, 18, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "Postal code.", " ",
		 NO, NO,  JUSTLEFT, "", "", tmpm_rec.post_code},
	{2, LIN, "mail_flag",	 5, 60, CHARTYPE,
		"U", "          ",
		" ", "Y", "Include Mail.", "Prospect is included in Mail shot. (Y/N) ",
		YES, NO,  JUSTLEFT, "YN", "", tmpm_rec.mail_flag},
	{2, LIN, "n_phone_date",	 6, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Next Phone Date", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmpm_rec.n_phone_date},
	{2, LIN, "n_phone_time",	 6, 60, CHARTYPE,
		"NN:NN", "          ",
		" ", "00:00", "Next Phone Time", " ",
		YES, NO,  JUSTLEFT, "0123456789:", "", tmpm_rec.n_phone_time},
	{2, LIN, "phone_freq",	 7, 18, INTTYPE,
		"NN", "          ",
		" ", "0", "Phone Frequency", "Enter Phone frequency in Days. ",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmpm_rec.phone_freq},
	{2, LIN, "n_phone_time",	 7, 60, CHARTYPE,
		"NN:NN", "          ",
		" ", "00:00", "Best Phone Time", " ",
		YES, NO,  JUSTLEFT, "0123456789:", "", tmpm_rec.best_ph_time},
	{2, LIN, "n_visit_date",	 8, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Next Visit Date", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmpm_rec.n_visit_date},
	{2, LIN, "n_visit_time",	 8, 60, CHARTYPE,
		"NN:NN", "          ",
		" ", "00:00", "Next Visit Time", " ",
		YES, NO,  JUSTLEFT, "0123456789:", "", tmpm_rec.n_visit_time},
	{2, LIN, "visit_freq",	 9, 18, INTTYPE,
		"NN", "          ",
		" ", "0", "Visit Frequency", "Enter Visit frequency in Days. ",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmpm_rec.phone_freq},
	{2, LIN, "sman",	11, 18, CHARTYPE,
		"UU", "          ",
		" ", "", "Salesman code  ", "Enter Valid Salesman Number. [SEARCH] Search. ",
		YES, NO, JUSTRIGHT, "", "", tmpm_rec.sman_code},
	{2, LIN, "sal_name",	12, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", exsf_rec.salesman},
	{2, LIN, "area",	13, 18, CHARTYPE,
		"UU", "          ",
		" ", "", "Area code.", "Enter valid area number. [SEARCH] Search.",
		YES, NO, JUSTRIGHT, "", "", tmpm_rec.area_code},
	{2, LIN, "area_name",	14, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{2, LIN, "sec_type",	15, 18, CHARTYPE,
		"UUU", "          ",
		" ", "", "Business Sector ", "Enter valid Business Sector. [SEARCH] Search. ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.b_sector},
	{2, LIN, "sec_name",	16, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", excl_rec.class_desc},
	{2, LIN, "or_no",	17, 18, CHARTYPE,
		"UUU", "          ",
		" ", "", "Origin.", "Enter valid Origin. [SEARCH] Search. ",
		YES, NO,  JUSTLEFT, "", "", tmpm_rec.origin},
	{2, LIN, "or_desc",	18, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description ", " ",
		 NA, NO,  JUSTLEFT, "", "", tmof_rec.o_desc},
	{3, TXT, "",	 3, 10, 0,
		"", "          ",
		"", "", " (Prospect/Lead Notes.)", "",
		 15, 60, 15, "", "", local_rec.data_str},
	{4, LIN, "user1",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [0], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [0]},
	{4, LIN, "user2",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [1], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [1]},
	{4, LIN, "user3",	 6, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [2], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [2]},
	{4, LIN, "user4",	 7, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [3], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [3]},
	{4, LIN, "user5",	 8, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [4], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [4]},
	{4, LIN, "user6",	 9, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [5], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [5]},
	{4, LIN, "user7",	10, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [6], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [6]},
	{4, LIN, "user8",	11, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [7], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [7]},
	{4, LIN, "user9",	12, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [8], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [8]},
	{4, LIN, "user10",	13, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [9], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [9]},
	{4, LIN, "user11",	14, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [10], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [10]},
	{4, LIN, "user12",	15, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", local_rec.u_prmpt [11], " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.user_value [11]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	Update 			(void);
void 	SrchExcl 		(char *);
void 	SrchTmof 		(char *);
void 	SrchTmpf 		(char *);
void 	SrchTmpm 		(char *);
void 	SrchExsf 		(char *);
void 	SrchExaf 		(char *);
void 	FindMisc 		(void);
void 	LoadNotes 		(void);
void 	LoadUserValues 	(void);
void 	LoadUserPrompts (void);
void 	CheckReport 	(void);
int 	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	int		i;

	if (argc > 1)
		byReportType = TRUE;

	currentUser = getenv ("LOGNAME");

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	
	OpenDB (); 	


	if (byReportType)
	{
		CheckReport ();
		switch	 (reportStatus)
		{
		case NOT_FND:
		case NRM_REP:
			_set_masks ("tm_pro_inp.s");
			break;

		case SUP_REP:
			_set_masks ("tm_pro_inp.s");
			break;
		}
	}
	else
		_set_masks ("tm_pro_inp.s");

	for (i = 0;i < 4;i++)
		tab_data [i]._desc = scn_desc [i];

	init_vars (1);

	LoadUserPrompts ();
	
	if (!U_BOX)
		no_edit (4);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		newProspect 	= FALSE;
		search_ok 		= TRUE;
		abc_unlock (tmpm);
		init_vars (1);
		init_vars (3);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (newProspect == TRUE) 
		{
			/*-------------------------------
			| Enter screen 2 linear input . |
			-------------------------------*/
			heading (2);
			entry (2);
			if (restart)
				continue;

			/*-------------------------------
			| Enter screen 3 linear input . |
			-------------------------------*/
			heading (3);
			entry (3);
			if (restart)
				continue;

			if (U_BOX)
			{
				/*-------------------------------
				| Enter screen 4 linear input . |
				-------------------------------*/
				heading (4);
				entry (4);
				if (restart)
					continue;
			}
		}
		else 
		{
			FindMisc ();
			scn_display (1);
		}
				
		edit_all ();
		if (restart)
			continue;

		/*-----------------------
		| Update customers record. |
		-----------------------*/
		if (!restart)
			Update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (tmpm, tmpm_list, TMPM_NO_FIELDS, 
								(envDbFind) ? "tmpm_id_no3" : "tmpm_id_no");

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (tmxf, tmxf_list, TMXF_NO_FIELDS, "tmxf_id_no");
	open_rec (tmof, tmof_list, TMOF_NO_FIELDS, "tmof_id_no");
	open_rec (tmud, tmud_list, TMUD_NO_FIELDS, "tmud_id_no");
	open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmpm);
	abc_fclose (esmr);
	abc_fclose (excl);
	abc_fclose (exsf);
	abc_fclose (exaf);
	abc_fclose (tmxf);
	abc_fclose (tmof);
	abc_fclose (tmud);
	abc_fclose (tmpf);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	int		i;
	/*------------------------------------------
	| Validate Prospect Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("pro_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmpm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		strcpy (tmpm_rec.br_no,branchNo);
		strcpy (tmpm_rec.pro_no,pad_num (local_rec.cur_pro));
		newProspect = find_rec (tmpm, &tmpm_rec, COMPARISON, "w");
		if (newProspect == FALSE)
		{
			if (byReportType && 
			   (reportStatus == NOT_FND ||
			   (reportStatus != NOT_FND &&
			    strcmp (tmpm_rec.sman_code, exsf_rec.salesman_no))))
			{
				print_mess (ML (mlTmMess009));
				sleep (sleepTime);	
				clear_mess ();
				return (EXIT_FAILURE);
			}

			entry_exit = 1;
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------------------------------
	| Check first line of Delivery address for 'same' (means charge to |
	| address and delivery address are the same)                       |
	------------------------------------------------------------------*/
        if (LCHECK ("del_adr1"))
	{
		if (!strncmp (tmpm_rec.del1_adr,"same",4) || 
		     !strncmp (tmpm_rec.del1_adr,"SAME",4)) 
		{
			strcpy (tmpm_rec.del1_adr, tmpm_rec.mail1_adr);
			strcpy (tmpm_rec.del2_adr, tmpm_rec.mail2_adr);
			strcpy (tmpm_rec.del3_adr, tmpm_rec.mail3_adr);
	
			DSP_FLD ("del_adr1");
			DSP_FLD ("del_adr2");
			DSP_FLD ("del_adr3");
			skip_entry = 2;
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate salesman code and allow search. |
	------------------------------------------*/
	if (LCHECK ("sman"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,tmpm_rec.sman_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("sal_name");
		return (EXIT_SUCCESS);
	}
	/*--------------------------------------
	| Validate area code and allow search. |
	--------------------------------------*/
	if (LCHECK ("area"))
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		strcpy (exaf_rec.area_code,tmpm_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("area_name");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate customer type and allow search. |
	------------------------------------------*/
	if (LCHECK ("sec_type"))
	{
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excl_rec.co_no,comm_rec.co_no);
		strcpy (excl_rec.class_type,tmpm_rec.b_sector);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess164));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("sec_name");
		return (EXIT_SUCCESS);
	}
	/*----------------------------------------
	| Validate Origin code and allow search. |
	----------------------------------------*/
	if (LCHECK ("or_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmof (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmof_rec.co_no,comm_rec.co_no);
		strcpy (tmof_rec.o_code,tmpm_rec.origin);
		cc = find_rec (tmof, &tmof_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess165));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("or_desc");
		return (EXIT_SUCCESS);
	}
	/*----------------------------------------
	| Validate Origin code and allow search. |
	----------------------------------------*/
	if (!strncmp (FIELD.label,"cont_code",9))
	{
		i = atoi (FIELD.label + 9);

		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}
		switch (i)
		{
			case	1:
				if (dflt_used && !strncmp (tmpm_rec.cont_name1,"     ",5))
				{
					strcpy (tmpm_rec.cont_code1, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code1);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;

			case	2:
				if (dflt_used && !strncmp (tmpm_rec.cont_name2,"     ",5))
				{
					strcpy (tmpm_rec.cont_code2, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code2);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;

			case	3:
				if (dflt_used && !strncmp (tmpm_rec.cont_name3,"     ",5))
				{
					strcpy (tmpm_rec.cont_code3, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code3);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;

			case	4:
				if (dflt_used && !strncmp (tmpm_rec.cont_name4,"     ",5))
				{
					strcpy (tmpm_rec.cont_code4, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code4);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;
		
			case	5:
				if (dflt_used && !strncmp (tmpm_rec.cont_name5,"     ",5))
				{
					strcpy (tmpm_rec.cont_code5, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code5);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;

			case	6:
				if (dflt_used && !strncmp (tmpm_rec.cont_name6,"     ",5))
				{
					strcpy (tmpm_rec.cont_code6, "   ");
					DSP_FLD (FIELD.label);
					return (EXIT_SUCCESS);
				}
				strcpy (tmpf_rec.co_no,comm_rec.co_no);
				strcpy (tmpf_rec.pos_code,tmpm_rec.cont_code6);
				cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
			break;
		}
		cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess166));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		sprintf (err_str, ML (mlTmMess038),
				tmpf_rec.pos_code, tmpf_rec.pos_desc);

		print_at (2, 0, err_str);
		DSP_FLD (FIELD.label);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	
/*----------------
| Update Record. |
----------------*/
void
Update (void)
{
	char	wk_fldname [7];
	clear ();

	if (newProspect == TRUE) 
	{
		strcpy (tmpm_rec.op_code,"              ");
		strcpy (tmpm_rec.lst_op_code,"              ");
		strcpy (tmpm_rec.call_bk_time, "00:00");
		tmpm_rec.call_bk_date = 0L;
		tmpm_rec.call_no = 0L;
		strcpy (tmpm_rec.active_flg,"N");
		strcpy (tmpm_rec.delete_flag,"N");
		strcpy (tmpm_rec.tax_code, " ");
		strcpy (tmpm_rec.tax_no, "                ");

		tmpm_rec.date_create = TodaysDate ();
		strcpy (tmpm_rec.pro_no,pad_num (local_rec.cur_pro));
		strcpy (tmpm_rec.co_no,comm_rec.co_no);
		tmpm_rec.lphone_date = 0L;
		cc = abc_add (tmpm,&tmpm_rec);
		if (cc) 
		{
			if (cc == DUPADD) 
			{
			   	errmess (ML (mlTmMess013));
				sleep (sleepTime);
				clear_mess ();
				return;
			}
			file_err (cc, tmpm, "DBADD");
		}
	}
	else 
	{
		cc = abc_update (tmpm,&tmpm_rec);
		if (cc) 
			file_err (cc, tmpm, "DBUPDATE");

        	abc_unlock (tmpm);
	}
	strcpy (tmpm_rec.co_no,comm_rec.co_no);
	strcpy (tmpm_rec.br_no,branchNo);
	strcpy (tmpm_rec.pro_no,pad_num (local_rec.cur_pro));
	cc = find_rec (tmpm, &tmpm_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, tmpm, "DBFIND");

	scn_set (NOTES_SCN);
	
	for (line_cnt = 0; line_cnt < lcount [NOTES_SCN]; line_cnt++)
	{
		getval (line_cnt);

		tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
		strcpy (tmxf_rec.type, "N");
		tmxf_rec.line_no = line_cnt;
		cc = find_rec (tmxf, &tmxf_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (tmxf_rec.desc, local_rec.data_str);
			cc = abc_add (tmxf, &tmxf_rec);
			if (cc)
				file_err (cc, tmpm, "DBADD");
		}
		else
		{
			strcpy (tmxf_rec.desc, local_rec.data_str);
			cc = abc_update (tmxf, &tmxf_rec);
			if (cc)
				file_err (cc, tmpm, "DBUPDATE");
		}
	}
	tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, "N");
	tmxf_rec.line_no = line_cnt;
	cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	while (!cc && tmxf_rec.hhpm_hash == tmpm_rec.hhpm_hash &&
		 tmxf_rec.type [0] == 'N')
	{
		abc_delete (tmxf);

		cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	}
	scn_set (USER_SCN);
	for (line_cnt = 0; line_cnt < 12; line_cnt++)
	{
		sprintf (wk_fldname, "user%d", line_cnt + 1);
		if (FLD (wk_fldname) == ND)
			break;

		tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
		strcpy (tmxf_rec.type, "U");
		tmxf_rec.line_no = line_cnt;
		cc = find_rec (tmxf, &tmxf_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (tmxf_rec.desc,local_rec.user_value [line_cnt]);
			cc = abc_add (tmxf, &tmxf_rec);
			if (cc)
				file_err (cc, tmpm, "DBADD");
		}
		else
		{
			strcpy (tmxf_rec.desc,local_rec.user_value [line_cnt]);
			cc = abc_update (tmxf, &tmxf_rec);
			if (cc)
				file_err (cc, tmpm, "DBUPDATE");
		}
	}

	strcpy (local_rec.prev_pro,local_rec.cur_pro);
	return;
}

/*==========================================
| Search routine for Category Master File. |
==========================================*/
void
SrchExcl (
	char *key_val)
{
	work_open ();
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,key_val);
	save_rec ("#No.","#Customer Type Description");
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

/*========================================
| Search routine for Origin Master File. |
========================================*/
void
SrchTmof (
	char *key_val)
{
	work_open ();
	strcpy (tmof_rec.co_no,comm_rec.co_no);
	strcpy (tmof_rec.o_code,key_val);
	save_rec ("#No.","#Customer Type Description");
	cc = find_rec (tmof, &tmof_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (tmof_rec.o_code, key_val,strlen (key_val)) && 
		!strcmp (tmof_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (tmof_rec.o_code, tmof_rec.o_desc);
		if (cc)
			break;

		cc = find_rec (tmof, &tmof_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmof_rec.co_no,comm_rec.co_no);
	strcpy (tmof_rec.o_code,temp_str);
	cc = find_rec (tmof, &tmof_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmof, "DBFIND");
}

/*========================================
| Search routine for Origin Master File. |
========================================*/
void
SrchTmpf (
	char *key_val)
{
	work_open ();
	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	strcpy (tmpf_rec.pos_code,key_val);
	save_rec ("#No.","#Customer Type Description");
	cc = find_rec (tmpf, &tmpf_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (tmpf_rec.pos_code, key_val,strlen (key_val)) && 
		!strcmp (tmpf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (tmpf_rec.pos_code, tmpf_rec.pos_desc);
		if (cc)
			break;

		cc = find_rec (tmpf, &tmpf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmpf_rec.co_no,comm_rec.co_no);
	strcpy (tmpf_rec.pos_code,temp_str);
	cc = find_rec (tmpf, &tmpf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmpf, "DBFIND");
}

/*==========================================
| Search routine for Salesman Master File. |
==========================================*/
void
SrchExsf (
	char *key_val)
{
	work_open ();
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,key_val);
	save_rec ("#No.","#Salesman Description");
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strncmp (exsf_rec.salesman_no, key_val,strlen (key_val)) && 
		      !strcmp (exsf_rec.co_no, comm_rec.co_no))
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
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*======================================
| Search routine for Area Master File. |
======================================*/
void
SrchExaf (
	char *key_val)
{
	work_open ();
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

void
FindMisc (void)
{
	int		i;

	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,tmpm_rec.b_sector);
	if (find_rec (excl, &excl_rec, COMPARISON, "r"))
		strcpy (excl_rec.class_type, "   ");

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,tmpm_rec.sman_code);
	if (find_rec (exsf, &exsf_rec, COMPARISON, "r"))
		strcpy (exsf_rec.salesman_no,"  ");

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,tmpm_rec.area_code);
	if (find_rec (exaf, &exaf_rec, COMPARISON, "r"))
		strcpy (exaf_rec.area_code,"  ");

	strcpy (tmof_rec.co_no,comm_rec.co_no);
	strcpy (tmof_rec.o_code,tmpm_rec.origin);
	if (find_rec (tmof, &tmof_rec, COMPARISON, "r"))
		strcpy (tmof_rec.o_code,"   ");

	for (i = 0; i < 6; i++)
	{
		strcpy (tmpf_rec.co_no,comm_rec.co_no);
		switch (i)
		{
			case	0:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code1);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;

			case	1:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code2);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;

			case	2:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code3);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;

			case	3:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code4);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;

			case	4:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code5);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;

			case	5:
				strcpy (tmpf_rec.pos_code, tmpm_rec.cont_code6);
				if (find_rec (tmpf, &tmpf_rec, COMPARISON, "r"))
					strcpy (tmpf_rec.pos_code,"   ");
			break;
		}
	}

	LoadNotes ();
	LoadUserValues ();

}

void
LoadNotes (void)
{
	scn_set (NOTES_SCN);

	lcount [NOTES_SCN] = 0;

	tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, "N");
	tmxf_rec.line_no = 0;
	cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	while (!cc && tmxf_rec.hhpm_hash == tmpm_rec.hhpm_hash &&
		 tmxf_rec.type [0] == 'N')
	{
		strcpy (local_rec.data_str, tmxf_rec.desc);
		putval (lcount [NOTES_SCN]++);
		cc = find_rec (tmxf, &tmxf_rec, NEXT, "r");
	}

	scn_set (HEADER_SCN);
	
	return;
}

void
LoadUserValues (void)
{
	int		i = 0;

	scn_set (USER_SCN);

	tmxf_rec.hhpm_hash = tmpm_rec.hhpm_hash;
	strcpy (tmxf_rec.type, "U");
	tmxf_rec.line_no = 0;
	cc = find_rec (tmxf, &tmxf_rec, GTEQ, "r");
	while (!cc && tmxf_rec.hhpm_hash == tmpm_rec.hhpm_hash &&
		 tmxf_rec.type [0] == 'U' && i < 12)
	{
		strcpy (local_rec.user_value [i++], tmxf_rec.desc);

		cc = find_rec (tmxf, &tmxf_rec, NEXT, "r");
	}
	scn_set (HEADER_SCN);
	
	return;
}

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

void
CheckReport (void)
{
	abc_selfield (exsf, "exsf_id_no2");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.logname, "%-14.14s", currentUser);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		reportStatus = NOT_FND;
	else
	{
		if (exsf_rec.stat_flag [0] == 'S')
			reportStatus = SUP_REP;
		else
			reportStatus = NRM_REP;
	}

	abc_selfield (exsf, "exsf_id_no");
	return;
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
int
heading (
	int scn)
{
	if (restart)
	{
		abc_unlock (tmpm);
		return (EXIT_FAILURE);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	pr_box_lines (scn);

	rv_pr (ML (mlTmMess010),25,0,1);
	print_at (0,58,ML (mlTmMess011),local_rec.prev_pro);
	move (0,1);
	line (80);

	move (1,input_row);

	if (scn == 4)
		box (0, 3, 80, U_BOX);

	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

