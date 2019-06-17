/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: groupsel.c,v 5.6 2002/07/17 09:57:53 scott Exp $
|  Program Name  : (sk_groupsel.c)
|  Program Desc  : (Input group And Class For Report Programs)
|---------------------------------------------------------------------|
|  Date Written  : (02/03/1999)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: groupsel.c,v $
| Revision 5.6  2002/07/17 09:57:53  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2001/10/17 09:06:57  cha
| Updated as getchar left in program.
| Changes made by Scott.
|
| Revision 5.4  2001/10/10 10:15:35  robert
| updated to accept only alphabet characters on start class and end class
|
| Revision 5.3  2001/09/21 02:20:43  cha
| COSMETIC-504. Updated to handle month validation correctly.
|
| Revision 5.2  2001/08/09 09:18:33  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/06/01 09:35:13  scott
| Updated to add validation on start and end class.
| Updated to add missing sleep on warning message.
|
| Revision 4.1  2001/03/23 00:37:51  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: groupsel.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_groupsel/groupsel.c,v 5.6 2002/07/17 09:57:53 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
				    
	/*================================================================
	| Special fields and flags  ##################################.  |
	================================================================*/
	char 	programName [41],
			programDescription [101],
			by_what [2],
			progtype [2];

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	back [2];
	char	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
	char	startCat [12];
	char	startClass [2];
	char	startDescription [41];
	char	endCat [12];
	char	endClass [2];
	char	endDescription [41];
	char	startGroup [13];
	char	endGroup [13];
	char	reportBy [2];
	char	reportByDesc [11];
	int		printerNo;
	char	lp_str [3];
	char	sMthYr [11];
	char	eMthYr [11];
	long	sDate;
	long	eDate;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "startClass", 3, 2, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Start Class              ", "Input Start Class A-Z.", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.startClass}, 
	{1, LIN, "startCat", 4, 2, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "Start Category           ", "Input Start Inventory Category.", 
		YES, NO, JUSTLEFT, "", "", local_rec.startCat}, 
	{1, LIN, "startCatDesc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",  "Description              ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.startDescription}, 
	{1, LIN, "endClass", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Z", "End Class                ", "Input End Class A-Z.", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.endClass}, 
	{1, LIN, "endCat", 8, 2, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "End Category             ", "Input End Inventory Category.", 
		YES, NO, JUSTLEFT, "", "", local_rec.endCat}, 
	{1, LIN, "endCatDesc", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",  "Description              ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.endDescription}, 
	{1, LIN, "reportBy", 11, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Report M(onth) / Y(ear   ", "Input Y(ear) of M(onth) <default = Year>", 
		YES, NO, JUSTLEFT, "MY", "", local_rec.reportBy}, 
	{1, LIN, "reportByDesc", 11, 30, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.reportByDesc}, 
	{1, LIN, "sMthYr",	 12, 2, CHARTYPE,
		"UU/UUUU", "          ",
		" ", "01/1900", "Start Month/Year         ", "Enter Month and Year for Start of Display- Default is current month ",
		YES, NO,  JUSTLEFT, "", "0123456789", local_rec.sMthYr},
	{1, LIN, "eMthYr",	 13, 2, CHARTYPE,
		"UU/UUUU", "          ",
		" ", " ", "End   Month/Year         ", "Enter Month and Year for End of Display - Default is current Month ",
		YES, NO,  JUSTLEFT, "", "0123456789", local_rec.eMthYr},
	{1, LIN, "printerNo", 15, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number           ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 16, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background               ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 16, 30, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.backDesc}, 
	{1, LIN, "onite", 17, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight                ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 17, 30, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.oniteDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

extern	int	TruePosition;
extern	int	EnvScreenOK;
int		mdy [3];

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
int  	spec_valid 			(int);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchExcf 			(char *);
int  	heading 			(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*---------------------------------------
	|	parameters							|
	|	1:	program name					|
	|	2:	program description				|
	|	3:	program type (optional)			|
	|               char string             |
	---------------------------------------*/

	if (argc < 3) 
	{
		print_at (0,0,"Usage %s Program name, Program Desc",argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (programName,"%-.40s",argv [1]);
	sprintf (programDescription,"%-.100s",argv [2]);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*===========================
	| Open main database files. |  
	===========================*/
	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
   	search_ok 	= TRUE;

	init_vars (1);
	heading (1);
	entry (1);
    if (prog_exit) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1; 
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }
	
	rset_tty ();

	if (!strcmp (local_rec.endCat, "~~~~~~~~~~~"))
		memset ((char *)local_rec.endCat,0xff,sizeof (local_rec.endCat));

	sprintf (local_rec.startGroup,	"%1.1s%-11.11s",
								local_rec.startClass,local_rec.startCat);
	sprintf (local_rec.endGroup,	"%1.1s%-11.11s", 
								local_rec.endClass, local_rec.endCat);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onite [0] == 'Y') 
	{
		execlp
		(
			"ONIGHT",
			"ONIGHT",
			programName,
			local_rec.lp_str,
			local_rec.startGroup,
			local_rec.endGroup,
			local_rec.reportBy,
			local_rec.sMthYr,
			local_rec.eMthYr,
			programDescription,	
			(char *)0
		);
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () != 0)
		{
			clear ();
			print_at (0,0,ML (mlStdMess035));
			fflush (stdout);
			return (EXIT_FAILURE);
		}
		else
		{
			execlp
			 (
				programName,
				programName,
				local_rec.lp_str,
				local_rec.startGroup,
				local_rec.endGroup,
				local_rec.reportBy,
				local_rec.sMthYr,
				local_rec.eMthYr,
				(char *)0
			);
		}
	}
	else 
	{
		clear ();
		print_at (0,0,ML (mlStdMess035));
		fflush (stdout);

		execlp
		(
			programName,
			programName,
			local_rec.lp_str,
			local_rec.startGroup,
			local_rec.endGroup,
			local_rec.reportBy,
			local_rec.sMthYr,
			local_rec.eMthYr,
			(char *)0
		);
	}
	shutdown_prog (); 
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	int		smdy [3],
			emdy [3],
			eMth;
	char	tmpMth [3];
			
	if (LCHECK ("startClass"))
	{
		if (prog_status != ENTRY && 
				strcmp (local_rec.startClass,local_rec.endClass) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("endClass"))
	{
		if (strcmp (local_rec.startClass,local_rec.endClass) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("startCat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.startCat);

		if (dflt_used)
		{
			sprintf (local_rec.startCat,"%-11.11s","           ");
			sprintf (excf_rec.cat_desc,"%-40.40s",ML ("BEGINNING OF RANGE"));
			sprintf (local_rec.startDescription,"%-40.40s",excf_rec.cat_desc);
			DSP_FLD ("startCatDesc");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (prog_status != ENTRY && strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.startDescription,"%-40.40s",excf_rec.cat_desc);
		DSP_FLD ("startCatDesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("endCat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.endCat);
		
		if (dflt_used)
		{
			sprintf (local_rec.endCat,"%-11.11s","~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc,"%-40.40s",ML ("END OF RANGE"));
			strcpy (local_rec.endDescription,excf_rec.cat_desc);
			DSP_FLD ("endCatDesc");
			return (EXIT_SUCCESS);
		}
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		if (strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.endDescription,excf_rec.cat_desc);
		DSP_FLD ("endCatDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sMthYr"))
	{
		DateToDMY (comm_rec.inv_date, &mdy [0], &mdy [1], &mdy [2]);

		sprintf (tmpMth, "%2.2s", local_rec.sMthYr);
		smdy [0] = 1;
		smdy [1] = atoi (tmpMth);
		smdy [2] = atoi (local_rec.sMthYr + 3);

		if (smdy [1] < 1 || smdy [1] > 12)
		{
			print_mess (ML ("Start Month Must Be Between 1 and 12"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.sDate = DMYToDate (smdy [0], smdy [1], smdy [2]);

		if (prog_status != ENTRY && local_rec.sDate > local_rec.eDate)
		{
			print_mess (ML (mlStdMess057));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eMthYr"))
	{
		if (dflt_used)
			sprintf (local_rec.eMthYr, "%02d/%04d", mdy [1], mdy [2]);

		sprintf (tmpMth, "%2.2s", local_rec.eMthYr);
		eMth = atoi (tmpMth);
		if (eMth < 1 || eMth > 12)
		{
			print_mess (ML ("End Month Must Be Between 1 and 12"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		emdy [0] = 1;
		emdy [1] = atoi (tmpMth);
		emdy [2] = atoi (local_rec.eMthYr + 3);
		
		local_rec.eDate = DMYToDate (emdy [0], emdy [1], emdy [2]);
		local_rec.eDate = MonthEnd (local_rec.eDate);
			
		if (local_rec.eDate < local_rec.sDate)
		{
			print_mess (ML (mlStdMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp  (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.lp_str,"%d",local_rec.printerNo);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("reportBy"))
	{
		if (local_rec.reportBy [0] == 'M')
			strcpy (local_rec.reportByDesc, ML ("MONTH"));
		else
			strcpy (local_rec.reportByDesc, ML ("YEAR"));

		DSP_FLD ("reportByDesc");
		return (EXIT_SUCCESS);
	}
	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'N')
			strcpy (local_rec.backDesc, ML ("NO"));
		else
			strcpy (local_rec.backDesc, ML ("YES"));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'N')
			strcpy (local_rec.oniteDesc, ML ("NO"));
		else
			strcpy (local_rec.oniteDesc, ML ("YES"));

		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
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

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (excf);
	abc_dbclose ("data");
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	save_rec ("#Category No", "#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
				  !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	sprintf (err_str,ML (mlSkMess031),programDescription);
	rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
	
	line_at (1,0,80);
	line_at (6,1,79);
	line_at (10,1,79);
	line_at (14,1,79);
	box (0,2,80,15);

	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	line_at (22,0,80);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
