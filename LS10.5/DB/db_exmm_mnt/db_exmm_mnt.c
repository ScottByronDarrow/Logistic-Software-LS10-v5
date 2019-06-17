/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_exmm_mnt.c,v 5.3 2002/07/25 11:17:27 scott Exp $
|  Program Name  : (db_exmm_mnt.c)
|  Program Desc  : (Merchandiser Master Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 02/08/94         |
|---------------------------------------------------------------------|
| $Log: db_exmm_mnt.c,v $
| Revision 5.3  2002/07/25 11:17:27  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/11/22 07:49:27  scott
| Updated for warning
|
| Revision 5.1  2001/11/22 07:47:26  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_exmm_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_exmm_mnt/db_exmm_mnt.c,v 5.3 2002/07/25 11:17:27 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>

#define	LSL_UPDATE	0
#define	LSL_IGNORE	1
#define	LSL_DELETE	2
#define	LSL_DEFAULT	99

#include	"schema"

struct commRecord	comm_rec;
struct exmmRecord	exmm_rec;
struct exmaRecord	exma_rec;

char	systemDate [11];
long	lsystemDate;

char	badFileName [5];

char	*data  = "data";

char	DaysWork [7][6];
char	DaysWorkDesc [7][6];

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
	{ " 3. DELETE RECORD.                     ",
	  "" },
	{ ENDMENU }
};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	/*----------------
	| Salesman   	 |
	----------------*/
	{1, LIN, "merchant_no",	 4, 19, LONGTYPE,
		"NNNNNN", "          ",
		" ", "0", "Merchandiser No.  ", "Enter Merchandiser No. [SEARCH]. <return = New Merchandiser>",
		 NE, NO, JUSTLEFT, "", "", (char *)&exmm_rec.exmm_hash},
	{1, LIN, "merchant_name",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name ", " ",
		YES, NO,  JUSTLEFT, "", "", exmm_rec.name},
	{1, LIN, "exma_code",	 7, 19, CHARTYPE,
		"UU", "          ",
		" ", "", "Agency Code.", " ",
		 NO, NO,  JUSTLEFT, "", "", exmm_rec.agency},
	{1, LIN, "exma_desc",	 7, 32, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exma_rec.desc},
	{1, LIN, "st_date",	 9, 19, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date.", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&exmm_rec.st_date},
	{1, LIN, "ed_date",	 10, 19, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date.", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&exmm_rec.en_date},
	{1, LIN, "Sun",	12, 19, CHARTYPE,
		"U", "          ",
		" ", "N", "SUNDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [0]},
	{1, LIN, "Sun_desc",	12, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [0]},
	{1, LIN, "Mon",	12, 60, CHARTYPE,
		"U", "          ",
		" ", "Y",      "MONDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [1]},
	{1, LIN, "Mon_desc",	12, 63, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [1]},
	{1, LIN, "Tue",	13, 19, CHARTYPE,
		"U", "          ",
		" ", "Y",      "TUESDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [2]},
	{1, LIN, "Tue_desc",	13, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [2]},
	{1, LIN, "Wed",	13, 60, CHARTYPE,
		"U", "          ",
		" ", "Y",      "WEDNESDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [3]},
	{1, LIN, "Wed_desc",	13, 63, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [3]},
	{1, LIN, "Thu",	14, 19, CHARTYPE,
		"U", "          ",
		" ", "Y",      "THURSDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [4]},
	{1, LIN, "Thu_desc",	14, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [4]},
	{1, LIN, "Fri",	14, 60, CHARTYPE,
		"U", "          ",
		" ", "Y",      "FRIDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [5]},
	{1, LIN, "Fri_desc",	14, 63, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [5]},
	{1, LIN, "Sat",	15, 19, CHARTYPE,
		"U", "          ",
		" ", "N",      "SATURDAY", " ",
		YES, NO,  JUSTLEFT, "YN", "", DaysWork [6]},
	{1, LIN, "Sat_desc",	15, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", DaysWorkDesc [6]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},

};

typedef int BOOL;

static BOOL	ExmmDelOk (void);

static BOOL	NewCode = FALSE;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
int 	SrchExmm 		(char *);
int 	SrchExma 		(char *);
void 	Update 			(void);
BOOL 	ExmmDelOk 		(void);
int 	heading 		(int);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = FALSE; 
		edit_exit = FALSE;
		restart = FALSE;
		search_ok = TRUE;
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();

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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (exmm, exmm_list, EXMM_NO_FIELDS, "exmm_id_no");
	open_rec (exma, exma_list, EXMA_NO_FIELDS, "exma_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (void)
{
	abc_fclose (exmm);
	abc_fclose (exma);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("merchant_no"))
	{
		if (SRCH_KEY)
		{
			SrchExmm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exmm_rec.co_no, comm_rec.co_no);
		cc = find_rec (exmm, &exmm_rec, COMPARISON, "w");
		if (cc)
		{
			exmm_rec.exmm_hash = 0L;
			us_pr (" New Merchandiser. ", 30, 4, 1);
			DSP_FLD ("merchant_no");
			NewCode = TRUE;
		}
		else
		{
			NewCode = FALSE;
			entry_exit = TRUE;
			strcpy (DaysWork [0], (exmm_rec.day [0] == '1') ? "Y" : "N");
			strcpy (DaysWork [1], (exmm_rec.day [1] == '1') ? "Y" : "N");
			strcpy (DaysWork [2], (exmm_rec.day [2] == '1') ? "Y" : "N");
			strcpy (DaysWork [3], (exmm_rec.day [3] == '1') ? "Y" : "N");
			strcpy (DaysWork [4], (exmm_rec.day [4] == '1') ? "Y" : "N");
			strcpy (DaysWork [5], (exmm_rec.day [5] == '1') ? "Y" : "N");
			strcpy (DaysWork [6], (exmm_rec.day [6] == '1') ? "Y" : "N");
			strcpy (DaysWorkDesc [0], (exmm_rec.day [0] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [1], (exmm_rec.day [1] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [2], (exmm_rec.day [2] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [3], (exmm_rec.day [3] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [4], (exmm_rec.day [4] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [5], (exmm_rec.day [5] == '1') ? "Yes" : "No ");
			strcpy (DaysWorkDesc [6], (exmm_rec.day [6] == '1') ? "Yes" : "No ");

			DSP_FLD ("Sun");
			DSP_FLD ("Mon");
			DSP_FLD ("Tue");
			DSP_FLD ("Wed");
			DSP_FLD ("Thu");
			DSP_FLD ("Fri");
			DSP_FLD ("Sat");
			DSP_FLD ("Sun_desc");
			DSP_FLD ("Mon_desc");
			DSP_FLD ("Tue_desc");
			DSP_FLD ("Wed_desc");
			DSP_FLD ("Thu_desc");
			DSP_FLD ("Fri_desc");
			DSP_FLD ("Sat_desc");

			strcpy (exma_rec.co_no,comm_rec.co_no);
			strcpy (exma_rec.code, exmm_rec.agency);
			if (find_rec (exma, &exma_rec, COMPARISON, "r"))
				strcpy (exma_rec.desc, " ");
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| Validate Merchandiser Agency File. |
	------------------------------------*/
	if (LCHECK ("exma_code"))
	{
		if (SRCH_KEY)
		{
			SrchExma (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exma_rec.co_no,comm_rec.co_no);
		strcpy (exma_rec.code, exmm_rec.agency);
		cc = find_rec (exma, &exma_rec, COMPARISON, "r");
		if (cc) 
		{
			sprintf (err_str, "Agency Code %s is not on file.", exmm_rec.agency);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("exma_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_date"))
	{
		if (dflt_used)
			exmm_rec.st_date = lsystemDate;
		
		DSP_FLD ("st_date");

		return (EXIT_SUCCESS);
	}
			
	if (LCHECK ("ed_date"))
	{
		if (dflt_used)
			exmm_rec.en_date = lsystemDate + 365;
		
		DSP_FLD ("ed_date");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("Sun"))
	{
		strcpy (DaysWorkDesc [0], (DaysWork [0][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Sun_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Mon"))
	{
		strcpy (DaysWorkDesc [1], (DaysWork [1][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Mon_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Tue"))
	{
		strcpy (DaysWorkDesc [2], (DaysWork [2][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Tue_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Wed"))
	{
		strcpy (DaysWorkDesc [3], (DaysWork [3][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Wed_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Thu"))
	{
		strcpy (DaysWorkDesc [4], (DaysWork [4][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Thu_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Fri"))
	{
		strcpy (DaysWorkDesc [5], (DaysWork [5][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Fri_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("Sat"))
	{
		strcpy (DaysWorkDesc [6], (DaysWork [6][0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("Sat_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==========================
| Search for Merchandiser. |
==========================*/
int
SrchExmm (
 char*              key_val)
{
	_work_open (6,0,40);
	save_rec ("#Code","#Description");

	strcpy (exmm_rec.co_no, comm_rec.co_no);
	exmm_rec.exmm_hash = atol (key_val);

	cc = find_rec (exmm, &exmm_rec, GTEQ, "r");
	while (!cc && !strcmp (exmm_rec.co_no, comm_rec.co_no))
	{
		sprintf (err_str, "%06ld", exmm_rec.exmm_hash);
		cc = save_rec (err_str, exmm_rec.name);
		if (cc)
			break;

		cc = find_rec (exmm, &exmm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	strcpy (exmm_rec.co_no, comm_rec.co_no);
	exmm_rec.exmm_hash = atol (temp_str);
	cc = find_rec (exmm, &exmm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exmm, "DBFIND");

	return (EXIT_SUCCESS);
}
/*======================================
| Search for Merchandiser Agency File. |
======================================*/
int
SrchExma (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Merchandiser Agency Description");
	strcpy (exma_rec.co_no,comm_rec.co_no);
	sprintf (exma_rec.code,"%-2.2s",key_val);
	cc = find_rec (exma,&exma_rec,GTEQ,"r");

	while (!cc && !strcmp (exma_rec.co_no,comm_rec.co_no) &&
		      	  !strncmp (exma_rec.code,key_val,strlen (key_val)))
	{
		cc = save_rec (exma_rec.code,exma_rec.desc);
		if (cc)
			break;

		cc = find_rec (exma,&exma_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	strcpy (exma_rec.co_no,comm_rec.co_no);
	sprintf (exma_rec.code,"%-2.2s",temp_str);
	cc = find_rec (exma,&exma_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, exma, "DBFIND");

	return (EXIT_SUCCESS);
}

/*==================
| Updated records. |
==================*/
void
Update (void)
{
	strcpy (exmm_rec.co_no, comm_rec.co_no);
	sprintf (exmm_rec.day, "%c%c%c%c%c%c%c",
				 (DaysWork [0][0] == 'Y') ? '1' : '0',
				 (DaysWork [1][0] == 'Y') ? '1' : '0',
				 (DaysWork [2][0] == 'Y') ? '1' : '0',
				 (DaysWork [3][0] == 'Y') ? '1' : '0',
				 (DaysWork [4][0] == 'Y') ? '1' : '0',
				 (DaysWork [5][0] == 'Y') ? '1' : '0',
				 (DaysWork [6][0] == 'Y') ? '1' : '0');

	if (NewCode)
	{
		cc = abc_add (exmm, &exmm_rec);
		if (cc) 
			file_err (cc, exmm, "DBADD");
	}
	else
	{
		BOOL exitLoop = FALSE;

		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case LSL_DEFAULT :
			case LSL_UPDATE :
				cc = abc_update (exmm, &exmm_rec);
				if (cc) 
					file_err (cc, exmm, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case LSL_IGNORE :
				abc_unlock (exmm);
				exitLoop = TRUE;
				break;

			case LSL_DELETE :
			{
				if (ExmmDelOk ())
				{
					clear_mess ();
					cc = abc_delete (exmm);
					if (cc)
						file_err (cc, exmm, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,
						"Matching Document Records Found in %-4.4s, Document Record Not Deleted",
						badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}

				exitLoop = TRUE;
				break;
			}

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	abc_unlock (exmm);
}

/*===========================
| Check whether it is OK to |
| delete the exmm record.   |
| Files checked are :       |
|                           |
===========================*/
static BOOL
ExmmDelOk (void)
{
	return (TRUE);
}

/*===========================
| edit () callback function |
===========================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (" Merchandiser Master Maintenance "), 23, 0, 1);

		box (0, 3, 80, 12);
		line_at (1,0,80);
		line_at (6,1,79);
		line_at (8,1,79);
		line_at (11,1,79);
		line_at (20,0,80);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		line_at (23,0,80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
