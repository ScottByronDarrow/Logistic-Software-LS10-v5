/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_script.c,v 5.7 2002/07/19 04:41:06 scott Exp $
|  Program Name  : (tm_script.c)
|  Program Desc  : (Add / Change Tele-marketing Scripts)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 05/04/91         |
|---------------------------------------------------------------------|
| $Log: tm_script.c,v $
| Revision 5.7  2002/07/19 04:41:06  scott
| .
|
| Revision 5.6  2002/02/08 08:56:50  kaarlo
| S/C 00785. Remove clear() for LS10-GUI to avoid tm_script.exe error.
|
| Revision 5.5  2001/11/14 06:07:17  scott
| Updated to convert to app.schema
| Updated to clean code.
|
| Revision 5.4  2001/11/12 07:38:20  scott
| Updated to add app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_script.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_script/tm_script.c,v 5.7 2002/07/19 04:41:06 scott Exp $";

/*
 * Include file dependencies
 */
#define TABLINES    8
#define MAXLINES    8
#define	TXT_REQD

#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>


#define	SCN_HEADER	1
#define	SCN_NOTES	2
#define	SCN_REPLY	3

#define	LSL_UPDATE	0
#define	LSL_IGNORE	1
#define	LSL_DELETE	2

extern	int	X_EALL;
extern	int	Y_EALL;

char   	*tmsh2 	= "tmsh2",
		*tmsl2 	= "tmsl2",
		*data 	= "data";
	
    int 	new_script 	= 0,
   	    	newPrompt 	= 0,
			envDbCo 	= 0,
			wk_no		= 0,
	    	envDbFind 	= 0,
			sameScript 	= 0;
	
	char 	*currentUser;

	char 	oldScreenDesc [41],
			branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct tmshRecord	tmsh_rec;
struct tmshRecord	tmsh2_rec;
struct tmslRecord	tmsl_rec;
struct tmslRecord	tmsl2_rec;

	int		*tmsl_rep_goto	=	&tmsl_rec.rep1_goto;

	char	*scn_desc [] = 
    {
		"Script Header Screen.",
		"Prompt Objective.",
		"Prompt Replies."
	};

	extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct 
{
	char 	dummy [11];
	char	dataStr [73];
	char	promptDesc [21];
	char	goToPromptDesc [41];
	int	    gotoPromptNo;
} local_rec;

static  struct  var vars [] =
{
	{SCN_HEADER, LIN, "script_no",	 4, 2, INTTYPE,
		"NNNN", "          ",
		" ", "", "Script Number       ", "Enter Script no.",
		 NE, NO,  JUSTLEFT, "1", "9999", (char *)&tmsh_rec.script_no},
	{SCN_HEADER, LIN, "script_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Script Description  ", "Enter Script description.",
		YES, NO,  JUSTLEFT, "", "", tmsh_rec.desc},
	{SCN_HEADER, LIN, "prompt_no",	 6, 2, INTTYPE,
		"NNNN", "          ",
		" ", "0","Prompt Number       ", "Enter Prompt no.",
		 NE, NO,  JUSTLEFT, "1", "9999", (char *)&tmsl_rec.prmpt_no},
	{SCN_HEADER, LIN, "prompt_desc",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Prompt Description  ", "Enter prompt description.",
		YES, NO,  JUSTLEFT, "", "", tmsl_rec.desc},
	{SCN_NOTES, TXT, "",	9, 1, 0,
		"", "          ",
		"", "", " (Prompt text)", "",
		7, 72, 7, "", "", local_rec.dataStr},

	{SCN_REPLY, TAB, "rep_pmpt_desc",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Prompt reply Desc. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.promptDesc},
	{SCN_REPLY, TAB, "go_pmpt_no",	 0, 3, INTTYPE,
		"NNNN", "          ",
		" ", "", "Prompt No.", "Enter Prompt number. [SEARCH] Search.",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.gotoPromptNo},
	{SCN_REPLY, TAB, "go_pmpt_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "     G o t o   P r o m p t   D e s c    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.goToPromptDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	UpdateMenu 		(void);
void 	Update 			(void);
void 	SrchTmsh 		(char *);
void 	SrchTmsl 		(char *);
void 	SrchTmsl2 		(char *);
void 	LoadPtext 		(void);
int  	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int 
main (
 int argc, 
 char *argv [])
{
	int	i;

	TruePosition	=	TRUE;

	currentUser = getenv ("LOGNAME");

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	SETUP_SCR (vars);

	tab_col = 0;
	tab_row = 9;
	Y_EALL = 2;
	init_scr ();
	set_tty ();
	_set_masks ("tm_script.s");

	for (i = 0;i < 3;i++)
    {
		tab_data [i]._desc = scn_desc [i];
    }

	OpenDB (); 	

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (!prog_exit)	
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		new_script 	= FALSE;
		search_ok 	= TRUE;

		abc_unlock (tmsl);
		init_vars (SCN_HEADER);
		init_vars (SCN_REPLY);
		init_vars (SCN_NOTES);

		if (sameScript)
		{
			init_ok = TRUE;
			init_vars (SCN_NOTES);
			FLD ("script_no") = NA;
			FLD ("script_desc") = NA;
			tmsh_rec.script_no = sameScript;
			sprintf (tmsh_rec.desc, "%-40.40s", oldScreenDesc);
			tmsl_rec.prmpt_no = 0;
			sprintf (tmsl_rec.desc, "%-30.30s", " ");

			heading (SCN_HEADER);
			scn_display (SCN_HEADER);
			init_ok = FALSE;
		}
		else
		{
			FLD ("script_no") 	= YES;
			FLD ("script_desc") = YES;
			heading (SCN_HEADER);
			init_ok = TRUE;
			init_vars (SCN_NOTES);
		}

		/*
		 * Enter screen SCN_HEADER linear input.
		 */
		entry (SCN_HEADER);
		if (prog_exit || restart)
		{
			sameScript = 0;
			continue;
		}

		if (newPrompt)
		{
			/*
			 * Enter screen SCN_NOTES linear input.
			 */
			heading (SCN_NOTES);
			scn_display (SCN_NOTES);
			entry (SCN_NOTES);
			if (restart)
			{
				sameScript = 0;
				continue;
			}

			/*
			 * Enter screen SCN_REPLY linear input.
			 */
			heading (SCN_REPLY);
			scn_display (SCN_REPLY);
			entry (SCN_REPLY);
			if (restart)
			{
				sameScript = 0;
				continue;
			}
		}
				
		edit_all ();
		if (restart)
		{
			sameScript = 0;
			continue;
		}

		/*
		 * Update script record.
		 */
		Update ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (tmsh2, tmsh);
	open_rec (tmsh2, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");

	abc_alias (tmsl2, tmsl);
	open_rec (tmsl2, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");

	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmsh2);
	abc_fclose (tmsl2);
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate Prospect Number And Allow Search.
	 */
	if (LCHECK ("script_no"))
	{
		if (sameScript && (FLD ("script_no") == NA))
		{
			FLD ("script_no") = YES;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
        {
			return (EXIT_FAILURE);
        }

		FLD ("script_desc") = YES;

		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tmsh_rec.co_no, comm_rec.co_no);
		new_script = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (!new_script)
        {
			skip_entry = 1;
        }

		DSP_FLD ("script_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Prompt Number.
	 */
	if (LCHECK ("prompt_no"))
	{
		if (dflt_used)
        {
			return (EXIT_FAILURE);
        }

		if (SRCH_KEY)
		{
			SrchTmsl (temp_str);
			return (EXIT_SUCCESS);
		}
		if (new_script)
        {
			tmsl_rec.hhsh_hash = 0L;
        }
		else
        {
			tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
        }
		newPrompt = find_rec (tmsl, &tmsl_rec, COMPARISON, "w");
		if (!newPrompt)
		{
			LoadPtext ();
			entry_exit = 1;
		}

		if (new_script)
		{
			tmsl_rec.prmpt_no = 1;
			sprintf (tmsl_rec.desc, "%-30.30s", " ");
		}

		DSP_FLD ("prompt_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate line item prompt number.
	 */
	if (LCHECK ("go_pmpt_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsl2 (temp_str);
#ifdef GVISION
#else
			clear ();
#endif
			return (EXIT_SUCCESS);
		}
		if (new_script)
			tmsl2_rec.hhsh_hash = 0L;
		else
			tmsl2_rec.hhsh_hash = tmsh_rec.hhsh_hash;
        
		tmsl2_rec.prmpt_no = local_rec.gotoPromptNo;
		if (find_rec (tmsl2, &tmsl2_rec, COMPARISON, "r"))
		{
			switch (local_rec.gotoPromptNo)
			{
			case 0:
				sprintf (local_rec.goToPromptDesc, "%-40.40s", " ");
				break;

			case 999:
				sprintf (local_rec.goToPromptDesc, "%-40.40s", "End Call. ");
				break;

			default:
				sprintf (local_rec.goToPromptDesc, "%-40.40s", 
					     "NOT ON FILE. WILL NEED TO BE ADDED. ");
				break;
			}
		}
		else
        {
			sprintf (local_rec.goToPromptDesc,"%-40.40s",
					 tmsl2_rec.desc);
        }

		DSP_FLD ("go_pmpt_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	


    /*===================
    |   The mini menu   |
    ===================*/
    MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD  ",
		  " UPDATE PROMPT RECORD WITH CHANGES MADE. " },
		{ " 2. IGNORE CHANGES ",
		  " IGNORE CHANGES JUST MADE TO PROMPT RECORD." },
		{ " 3. DELETE RECORD  ",
		  " DELETE PROMPT RECORD " },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
UpdateMenu (void)
{
	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case LSL_UPDATE :
			cc = abc_update (tmsl, &tmsl_rec);
			if (cc) 
				file_err (cc, tmsl, "DBUPDATE");
			return;

		case LSL_IGNORE :
			abc_unlock (tmsl);
			return;

		case LSL_DELETE :
			cc = abc_delete (tmsl);
			if (cc)
				file_err (cc, tmsl, "DBDELETE");

			return;
	
		default :
			break;
	    }
	}
}

/*
 * Update Record.
 */
void
Update (void)
{
	int	i;

	char	workDesc	[sizeof tmsl_rec.desc];

	print_at (2, 0, ML (mlStdMess035));
	fflush (stdout);

	/*
	 * Add a new script.
	 */
	if (new_script)
	{
		cc = abc_add (tmsh, &tmsh_rec);
		if (cc)
			file_err (cc, tmsh, "DBADD");
	}
	/*
	 * Update an existing script.
	 */
	else
	{
		cc = abc_update (tmsh, &tmsh_rec);
		if (cc)
			file_err (cc, tmsh, "DBUPDATE");
	}

	sameScript = tmsh_rec.script_no;
	sprintf (oldScreenDesc, "%-40.40s", tmsh_rec.desc);

	/*
	 * Find script header and lock record.
	 */
	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, tmsh, "DBFIND");
	
	strcpy (workDesc, tmsl_rec.desc);

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	new_rec = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	scn_set (SCN_NOTES);

	for (i = 0; i < 7; i++)
	{
		getval (i);
		switch (i)
		{
			case	0:
				sprintf (tmsl_rec.text1, "%72.72s", local_rec.dataStr);
			break;

			case	1:
				sprintf (tmsl_rec.text2, "%72.72s", local_rec.dataStr);
			break;

			case	2:
				sprintf (tmsl_rec.text3, "%72.72s", local_rec.dataStr);
			break;

			case	3:
				sprintf (tmsl_rec.text4, "%72.72s", local_rec.dataStr);
			break;

			case	4:
				sprintf (tmsl_rec.text5, "%72.72s", local_rec.dataStr);
			break;

			case	5:
				sprintf (tmsl_rec.text6, "%72.72s", local_rec.dataStr);
			break;

			case	6:
				sprintf (tmsl_rec.text7, "%72.72s", local_rec.dataStr);
			break;
		}
	}

	scn_set (SCN_REPLY);
	for (i = 0; i < 8; i++)
	{
		getval (i);
		tmsl_rep_goto [i] = local_rec.gotoPromptNo;
		switch (i)
		{
			case	0:
				strcpy (tmsl_rec.rep1_desc, local_rec.promptDesc);
				break;

			case	1:
				strcpy (tmsl_rec.rep2_desc, local_rec.promptDesc);
				break;

			case	2:
				strcpy (tmsl_rec.rep3_desc, local_rec.promptDesc);
				break;

			case	3:
				strcpy (tmsl_rec.rep4_desc, local_rec.promptDesc);
				break;

			case	4:
				strcpy (tmsl_rec.rep5_desc, local_rec.promptDesc);
				break;

			case	5:
				strcpy (tmsl_rec.rep6_desc, local_rec.promptDesc);
				break;

			case	6:
				strcpy (tmsl_rec.rep7_desc, local_rec.promptDesc);
				break;

			case	7:
				strcpy (tmsl_rec.rep8_desc, local_rec.promptDesc);
				break;
		}
	}
	
	if (new_rec)
	{
		cc = abc_add (tmsl, &tmsl_rec);
		if (cc)
			file_err (cc, tmsl, "DBADD");
	}
	else
	{
		strcpy (tmsl_rec.desc, workDesc);
		UpdateMenu ();
	}

	scn_set (SCN_HEADER);

	cc = abc_update (tmsh, &tmsh_rec);
	if (cc)
		file_err (cc, tmsh, "DBUPDATE");
	return;
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsh (
 char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#No.","#Script Description.");

	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
		  (tmsh_rec.script_no >= atoi (key_val)))
	{
		sprintf (err_str, "%04d", tmsh_rec.script_no);
		cc = save_rec (err_str, tmsh_rec.desc);
		if (cc)
        {
			break;
        }
		cc = find_rec (tmsh, &tmsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
		return;
    }

	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = atoi (temp_str);
	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
    {
		file_err (cc, tmsh, "DBFIND");
    }
}

/*======================================
| Search routine for Script Line File. |
======================================*/
void
SrchTmsl (
 char *key_val)
{
	_work_open (4,0,40);

	if (new_script)
    {
		tmsl_rec.hhsh_hash = 0L;
    }
	else
    {
		tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
    }
	tmsl_rec.prmpt_no = atoi (key_val);
	save_rec ("#No.","#Prompt Description.");
	cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
	while (!cc && 
          (tmsl_rec.hhsh_hash == tmsh_rec.hhsh_hash) &&
          (tmsl_rec.prmpt_no >= atoi (key_val)))
	{
		sprintf (err_str, "%04d", tmsl_rec.prmpt_no);
		cc = save_rec (err_str, tmsl_rec.desc);
		if (cc)
        {
			break;
        }
		cc = find_rec (tmsl, &tmsl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
		return;
    }

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = atoi (temp_str);
	cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	if (cc)
    {
		file_err (cc, tmsl, "DBFIND");
    }
}

/*======================================
| Search routine for Script Line File. |
======================================*/
void
SrchTmsl2 (
 char *key_val)
{
	_work_open (4,0,40);

	tmsl2_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl2_rec.prmpt_no = atoi (key_val);
	save_rec ("#No.","#Prompt Description.");

	cc = find_rec (tmsl2, &tmsl2_rec, GTEQ, "r");
	while (!cc && 
          (tmsl2_rec.hhsh_hash == tmsh_rec.hhsh_hash) &&
		  (tmsl2_rec.prmpt_no >= atoi (key_val)))
	{
		sprintf (err_str, "%04d", tmsl2_rec.prmpt_no);
		cc = save_rec (err_str, tmsl2_rec.desc);
		if (cc)
        {
			break;
        }
		cc = find_rec (tmsl2, &tmsl2_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
		return;
    }

	tmsl2_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl2_rec.prmpt_no = atoi (temp_str);
	cc = find_rec (tmsl2, &tmsl2_rec, COMPARISON, "r");
	if (cc)
    {
		file_err (cc, tmsl2, "DBFIND");
    }
}

/*=============================
| Load text and prompt lines. |
=============================*/
void
LoadPtext (void)
{
	int	i;

	lcount [SCN_NOTES] = 0;
	lcount [SCN_REPLY] = 0;

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	cc = find_rec (tmsl, &tmsl_rec, COMPARISON, "r");
	if (!cc)
	{
		scn_set (SCN_NOTES);

		strcpy (local_rec.dataStr, tmsl_rec.text1);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text2);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text3);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text4);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text5);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text6);
		putval (lcount [SCN_NOTES]++);
		strcpy (local_rec.dataStr, tmsl_rec.text7);
		putval (lcount [SCN_NOTES]++);

		scn_set (SCN_REPLY);
		for (i = 0; i < 8; i++)
		{
			switch (i)
			{
				case	0:
					strcpy (local_rec.promptDesc, tmsl_rec.rep1_desc);
				break;
				
				case	1:
					strcpy (local_rec.promptDesc, tmsl_rec.rep2_desc);
				break;
				
				case	2:
					strcpy (local_rec.promptDesc, tmsl_rec.rep3_desc);
				break;
				
				case	3:
					strcpy (local_rec.promptDesc, tmsl_rec.rep4_desc);
				break;
				
				case	4:
					strcpy (local_rec.promptDesc, tmsl_rec.rep5_desc);
				break;
				
				case	5:
					strcpy (local_rec.promptDesc, tmsl_rec.rep6_desc);
				break;
				
				case	6:
					strcpy (local_rec.promptDesc, tmsl_rec.rep7_desc);
				break;
				
				case	7:
					strcpy (local_rec.promptDesc, tmsl_rec.rep8_desc);
				break;
			}
			local_rec.gotoPromptNo = tmsl_rep_goto [i];

			tmsl2_rec.hhsh_hash = tmsh_rec.hhsh_hash;
			tmsl2_rec.prmpt_no 	= tmsl_rep_goto [i];
			if (find_rec (tmsl2, &tmsl2_rec, COMPARISON, "r"))
			{
				switch (tmsl_rep_goto [i])
				{
				case 0:
					sprintf (local_rec.goToPromptDesc, "%-40.40s", " ");
					break;

				case 999:
					sprintf (local_rec.goToPromptDesc, "%-40.40s","End Call.");
					break;

				default:
					sprintf (local_rec.goToPromptDesc, "%-40.40s", 
						"NOT ON FILE. WILL NEED TO BE ADDED. ");
					break;
				}
			}
			else
			{
				sprintf (local_rec.goToPromptDesc, "%-40.40s", tmsl2_rec.desc);
			}

			putval (lcount [SCN_REPLY]++);
		}
	}
	scn_set (SCN_HEADER);

	return;
}
int
heading (
 int scn)
{
	if (restart)
	{
		abc_unlock ("tmpm");    

        return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	pr_box_lines (scn);

	switch (scn)
	{
		case 1:
			pr_box_lines (scn);
		break;

		case 2:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			pr_box_lines (1);
		break;

		case 3:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			pr_box_lines (1);
		break;
	}

	scn_set (scn);
	rv_pr (ML (mlTmMess036), 25, 0, 1);

	line_at (1,0,80);
	line_at (20,0,80);

	print_at (21, 0, 
              ML (mlStdMess038),
              comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/* [end of file] */
