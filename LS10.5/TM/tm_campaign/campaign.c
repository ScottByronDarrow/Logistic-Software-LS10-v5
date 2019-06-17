/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: campaign.c,v 5.9 2002/03/01 03:53:14 scott Exp $
|  Program Name  : (tm_campaign.c)
|  Program Desc  : (Add / Change Tele-marketing Campaign File)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21/06/91         |
|---------------------------------------------------------------------|
| $Log: campaign.c,v $
| Revision 5.9  2002/03/01 03:53:14  scott
| ..
|
| Revision 5.8  2002/03/01 03:44:58  scott
| .
|
| Revision 5.7  2001/12/11 03:42:17  scott
| Issues List Reference Number = 00602
| Environment: Linux-Informix Server / HP-Oracle Server / Windows Client
| Program Description = OLMR4-SUB3-Part Transfer Report
| Description of the Issue: After input of Start/End Date fields, going back to
| 						  Start Date, the field can accept an input greater
|                           than End Date, and will save the record with
| 						  incorrect date range.
|
| Revision 5.6  2001/11/09 02:25:00  scott
| Updated from testing
|
| Revision 5.5  2001/11/08 05:06:27  scott
| Updated to use app.schema
| Updated to clean code and bring inline with standard.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: campaign.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_campaign/campaign.c,v 5.9 2002/03/01 03:53:14 scott Exp $";

#define	TXT_REQD
#define	MAXLINES	5

#include <ml_std_mess.h>
#include <ml_tm_mess.h>
#include <pslscr.h>
#include <minimenu.h>

#define	LSL_UPDATE	0
#define	LSL_IGNORE	1
#define	LSL_DELETE	2

extern	int	X_EALL;
extern	int	Y_EALL;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newCampaign  = 0,
			envDbCo 	 = 0,
			wk_no		 = 0,
			envDbFind 	 = 0;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct tmcfRecord	tmcf_rec;

	char	systemDate [11];
	long	lsystemDate;

	char	*scn_desc [] = {
		"Campaign Header.",
		"Campaign Details."
	};

	extern	int	TruePosition;

	char	*curr_user;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	data_str [51];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "campaign_no",	 3, 2, INTTYPE,
		"NNNN", "          ",
		" ", "", "Campaign Number      ", "Enter Campaign no.",
		 NE, NO,  JUSTLEFT, "1", "9999", (char *)&tmcf_rec.campaign_no},
	{1, LIN, "camp_desc1",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Campaign Description ", "Enter Campaign description.",
		YES, NO,  JUSTLEFT, "", "", tmcf_rec.c_name1},
	{1, LIN, "camp_desc2",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "-------------------- ", " ",
		YES, NO,  JUSTLEFT, "", "", tmcf_rec.c_name2},
	{1, LIN, "camp_manager",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Campaign Manager     ", "Enter Campaign Manager.",
		YES, NO,  JUSTLEFT, "", "", tmcf_rec.c_manager},
	{1, LIN, "start_date",	 7, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "Start date           ", "Enter Campaign Start Date.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.start_date},
	{1, LIN, "end_date",	 7, 40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End date             ", "Enter Campaign End Date.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.end_date},
	{1, LIN, "in_calls",	 9, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "", "No of Incalls        ", "Enter Number of incalls expected.",
		YES, NO,  JUSTLEFT, "0", "32700", (char *)&tmcf_rec.bg_incalls},
	{1, LIN, "out_calls",	 9, 40, INTTYPE,
		"NNNNN", "          ", 
		" ", "", "No of Outcalls       ", "Enter Number of Outcalls expected.",
		YES, NO,  JUSTLEFT, "0", "32700", (char *)&tmcf_rec.bg_outcalls},
	{1, LIN, "in_sal",	10, 2, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "No of Incall Sales   ", "Enter Number of incall sales expected.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.bg_insales},
	{1, LIN, "out_sal",	10, 40, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "No of Outcall Sales  ", "Enter Number of outcall sales expected.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.bg_outsales},
	{1, LIN, "in_val",	11, 2, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Incall Sales Value   ", "Enter Value of incall sales expected.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.bg_invalue},
	{1, LIN, "out_val",	11, 40, DOUBLETYPE,
		"NNNNNNNN.NN", "          ",
		" ", "", "Outcall Sales Value  ", "Enter Value of outcall sales expected.",
		YES, NO,  JUSTLEFT, "", "", (char *)&tmcf_rec.bg_outvalue},
	{2, TXT, "",	13, 15, 0,
		"", "          ",
		"", "", " (Campaign Objectives)", "",
		5, 50, 5, "", "", local_rec.data_str},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD  ",
		  " UPDATE CAMPAIGN RECORD WITH CHANGES MADE. " },
		{ " 2. IGNORE CHANGES ",
		  " IGNORE CHANGES JUST MADE TO CAMPAIGN RECORD." },
		{ " 3. DELETE RECORD  ",
		  " DELETE CAMPAIGN RECORD " },
		{ ENDMENU }
	};

void 	OpenDB 		 (void);
void 	CloseDB 	 (void);
int 	spec_valid 	 (int);
void 	UpdateMenu 	 (void);
void 	Update 		 (void);
void 	SrchTmpf 	 (char *);
void 	LoadPtext 	 (void);
int 	heading 	 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	int	i;

	curr_user = getenv ("LOGNAME");

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	Y_EALL = 2;

	init_scr ();
	set_tty ();
	_set_masks ("tm_campaign.s");

	for (i = 0;i < 2;i++)
		tab_data [i]._desc = scn_desc [i];

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	OpenDB (); 	

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
		newCampaign 	= FALSE;
		search_ok 		= TRUE;
		abc_unlock (tmcf);
		init_vars (1);
		init_vars (2);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		scn_display (2);

		if (newCampaign == 1) 
		{
			entry (2);
			if (restart)
				continue;
		}
		edit_all ();
		if (restart)
			continue;

		/*-----------------------
		| Update debtor record. |
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
OpenDB (
	void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmcf);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{

	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campaign_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmpf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (tmcf_rec.co_no,comm_rec.co_no);
		newCampaign = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (!newCampaign)
		{
			LoadPtext ();
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("start_date"))
	{
		if (tmcf_rec.start_date < lsystemDate && newCampaign)
		{
			errmess (ML (mlStdMess141));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY && tmcf_rec.start_date > tmcf_rec.end_date)
		{
			print_mess (mlStdMess017);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_date"))
	{
		if (dflt_used)
		{
			tmcf_rec.end_date = MonthEnd (lsystemDate);
			return (EXIT_SUCCESS);
		}

		if (tmcf_rec.end_date <= tmcf_rec.start_date)
		{
			errmess (ML (mlStdMess019));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("out_val"))			
	{
		clear_mess ();
		return (EXIT_SUCCESS);		
	}				
	return (EXIT_SUCCESS);
}	

/*===================
| Update mini menu. |
===================*/
void
UpdateMenu (void)
{
	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION.     ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case LSL_UPDATE :
			cc = abc_update (tmcf,&tmcf_rec);
			if (cc) 
				file_err (cc, tmcf, "DBUPDATE");
			return;

		case LSL_IGNORE :
			abc_unlock (tmcf);
			return;

		case LSL_DELETE :
			cc = abc_delete (tmcf);
			if (cc)
				file_err (cc, tmcf, "DBDELETE");
			return;
			break;
	
		default :
			break;
	    }
	}
}

/*----------------
| Update Record. |
----------------*/
void
Update (void)
{
	int	i;

	fflush (stdout);

	scn_set (2);

	for (i = 0; i < MAXLINES; i++)
	{
		getval (i);
		switch (i)
		{
			case	0:
				strcpy (tmcf_rec.c_obj1,  local_rec.data_str);
			break;

			case	1:
				strcpy (tmcf_rec.c_obj2,  local_rec.data_str);
			break;

			case	2:
				strcpy (tmcf_rec.c_obj3,  local_rec.data_str);
			break;

			case	3:
				strcpy (tmcf_rec.c_obj4,  local_rec.data_str);
			break;

			case	4:
				strcpy (tmcf_rec.c_obj5,  local_rec.data_str);
			break;
		}
	}
	/*-------------------
	| Add a new script. |
	-------------------*/
	if (newCampaign)
	{
		cc = abc_add (tmcf, &tmcf_rec);
		if (cc)
			file_err (cc, tmcf, "DBADD");
	}
	else
	{
		cc = abc_update (tmcf, &tmcf_rec);
		if (cc)
			file_err (cc, tmcf, "DBUPDATE");

	}
	return;
}

/*========================================
| Search routine for Script Header File. |
========================================*/
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

/*=============================
| Load text and prompt lines. |
=============================*/
void
LoadPtext (void)
{
	lcount [2] = 0;

	scn_set (2);

	strcpy (local_rec.data_str, tmcf_rec.c_obj1); putval (lcount [2]++);
	strcpy (local_rec.data_str, tmcf_rec.c_obj2); putval (lcount [2]++);
	strcpy (local_rec.data_str, tmcf_rec.c_obj3); putval (lcount [2]++);
	strcpy (local_rec.data_str, tmcf_rec.c_obj4); putval (lcount [2]++);
	strcpy (local_rec.data_str, tmcf_rec.c_obj5); putval (lcount [2]++);
	
	scn_set (1);
	return;
}
int
heading (
	int scn)
{
	if (restart)
	{
		abc_unlock (tmcf);
		return (EXIT_FAILURE);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	
	if (scn != cur_screen)
		scn_set (scn);

	rv_pr (ML (mlTmMess040),25,0,1);
	line_at (1,0,80);

	switch (scn)
	{
	case	1:
		scn_set (2);
		scn_display (2);
		pr_box_lines (scn);
		break;

	case	2:
		scn_set (1);
		scn_write (1);
		scn_display (1);
		pr_box_lines (1);
	}

	line_at (20,0,80);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
