/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: reccode_mnt.c,v 5.7 2002/11/19 10:37:01 robert Exp $
|  Program Name  : (gl_reccode_mnt.c)
|  Program Desc  : (General Ledger Recovery Code Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: reccode_mnt.c,v $
| Revision 5.7  2002/11/19 10:37:01  robert
| SC0013-Add/maintain recovery codes
|
| Revision 5.6  2002/03/01 02:39:19  scott
| Updated for EDIT/END default
|
| Revision 5.5  2002/02/27 06:01:17  scott
| Updated because scn_set not done
|
| Revision 5.4  2002/02/08 02:45:07  cha
| S/C 762. Updated to ensure that no duplicate records are being add
| in GLRI.
|
| Revision 5.3  2001/08/09 09:13:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:32  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:58  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: reccode_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_reccode_mnt/reccode_mnt.c,v 5.7 2002/11/19 10:37:01 robert Exp $";

#undef  MAXLINES
#define MAXLINES 	100
#define HEADER_SCN 	1
#define DETAIL_SCN 	2
#define SLEEPTIME 	3

#include <pslscr.h>
#include <minimenu.h>
#include <GlUtils.h>
#include <ml_std_mess.h>

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct glraRecord	glra_rec;
struct glrcRecord	glrc_rec;
struct glriRecord	glri_rec;

static char *data	= "data";

#define		SEL_UPDATE		0
#define		SEL_IGNORE		1
#define		SEL_DELETE		2
#define		DEFAULT			99

struct tag_localRec
{
	char	dummy [11];
	char	co_no [3];
	char	br_no [3];
	char	wh_no [3];
	char	account [MAXLEVEL + 1];
	char	accdesc [26];
} local_rec;

static	struct	var	vars []	=
{
	{HEADER_SCN, LIN, "code", 3, 16, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Recovery Code", " ",
		NE, NO, JUSTLEFT, "", "", glrc_rec.code},
	{HEADER_SCN, LIN, "desc", 4, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description", " ",
		NO, NO, JUSTLEFT, "", "", glrc_rec.desc},
	{HEADER_SCN, LIN, "value", 3, 60, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "Recovery Value", "",
		YES, NO, JUSTRIGHT, "", "", (char *) &glrc_rec.value},
	{DETAIL_SCN, TAB, "company", MAXLINES, 1, CHARTYPE,
		"NN", "          ",
		" ", " ", " CO ", " ",
		NA, NO, JUSTRIGHT, "", "", local_rec.co_no},
	{DETAIL_SCN, TAB, "branch", 0, 1, CHARTYPE,
		"NN", "          ",
		" ", " ", " BR ", " ",
		NO, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{DETAIL_SCN, TAB, "warehouse", 0, 1, CHARTYPE,
		"NN", "          ",
		" ", " ", " WH ", " ",
		NO, NO, JUSTRIGHT, "", "", local_rec.wh_no},
	{DETAIL_SCN, TAB, "account", 0, 1, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "          ",
		"0", " ", "      Account     ", "Enter General Ledger Account",
		YES, NO, JUSTLEFT, "0123456789", "", local_rec.account},
	{DETAIL_SCN, TAB, "accdesc", 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"0", " ", " Account Description       ", "",
		NA, NO, JUSTLEFT, " ", " ", local_rec.accdesc},
	{0, LIN, "", 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},

};

/*
 * Local Function Prototypes.  
 */
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	DeleteLine 		 (void);
void 	SrchGlrc 		 (char *);
void 	SrchEsmr 		 (char *);
void 	SrchCcmr 		 (char *);
void 	ReadGlri 		 (void);
void 	UpdateGlrc 		 (void);
void 	UpdateGlri 		 (void);
void 	tab_other 		 (int);
int		spec_valid 		 (int);
int		AlreadyExists 	 (void);
int 	CompanyLevelExists (void);
int 	FindAcct 		 (char *, int);
int 	UpdateMenu 		 (void);
int 	Update 			 (void);
int 	Delete 			 (void);
int 	heading 		 (int);

int  	newCode;

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char	*argv [])
{
	SETUP_SCR (vars);

	tab_row = 7;
	tab_col = 7;

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (HEADER_SCN);

	OpenDB ();


	GL_SetAccWidth (comm_rec.co_no, FALSE);

	prog_exit = FALSE;
	while (!prog_exit)
	{
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
   		restart 	= FALSE;
   		newCode 	= FALSE;
		lcount [DETAIL_SCN] = 0;
		init_vars (HEADER_SCN);
		init_vars (DETAIL_SCN);

		heading (HEADER_SCN);
		entry (HEADER_SCN);
		if (prog_exit || restart)
			continue;

		heading (DETAIL_SCN);

		if (newCode)
		{
			entry (DETAIL_SCN);
			if (prog_exit || restart)
				continue;
			heading (DETAIL_SCN);
		}

		scn_display (DETAIL_SCN);
		edit (DETAIL_SCN);

		edit_all ();

		if (restart)
			continue;

		while (!UpdateMenu ())
		{
			heading (HEADER_SCN);
			edit_all ();
		}
	}

	CloseDB ();
	FinishProgram ();

	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (glra, glra_list, GLRA_NO_FIELDS, "glra_code_id");
	open_rec (glrc, glrc_list, GLRC_NO_FIELDS, "glrc_id_no");
	open_rec (glri, glri_list, GLRI_NO_FIELDS, "glri_full_id");
	OpenGlmr ();
}

void
CloseDB (void)
{
	abc_fclose (glri);
	abc_fclose (glrc);
	abc_fclose (glra);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	GL_Close ();

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchGlrc (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (temp_str)) == 0)
		{
			print_mess (ML ("Blank recovery code not allowed."));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (glrc_rec.co_no, comm_rec.co_no);
		sprintf (glrc_rec.code, "%-5.5s", temp_str);
		cc = find_rec (glrc, &glrc_rec, EQUAL, "u");
		if (cc)
			newCode = TRUE;
		else
		{
			ReadGlri ();
			newCode = FALSE;
			entry_exit = TRUE;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("desc"))
	{
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("value"))
	{
		if (glrc_rec.value < 0.0)
		{
			print_err (ML ("Value can not be less than 0.00"));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("company"))
	{
		strcpy (local_rec.co_no, comm_rec.co_no);
		DSP_FLD ("company");
	}

	if (LCHECK ("branch"))
	{
		if (end_input)
		{
			entry_exit = TRUE;
			edit_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
		{
			if (prog_status != EDIT)
			{
				print_mess (ML (mlStdMess005));
				sleep (SLEEPTIME);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			DeleteLine ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.wh_no, "  ");
			strcpy (local_rec.br_no, "  ");
			strcpy (local_rec.wh_no, "  ");
			DSP_FLD ("branch");
			DSP_FLD ("warehouse");
			FLD ("warehouse") = NA;
			putval (line_cnt);
			if (AlreadyExists ())
				return (EXIT_FAILURE);
			return (EXIT_SUCCESS);
		}
		else
			FLD ("warehouse") = NO;

		if (prog_status == EDIT)
		{
			putval (line_cnt);
			if (AlreadyExists ())
				return (EXIT_FAILURE);
		}

		strcpy (esmr_rec.co_no,  comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		if (find_rec (esmr, &esmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess073));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("warehouse"))
	{
		if (end_input)
		{
			skip_entry = -1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
		{
			if (prog_status != EDIT)
			{
				print_mess (ML (mlStdMess005));
				sleep (SLEEPTIME);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			DeleteLine ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.wh_no, "  ");
			DSP_FLD ("warehouse");
			putval (line_cnt);
			if (AlreadyExists ())
				return (EXIT_FAILURE);
			return (EXIT_SUCCESS);
		}

		putval (line_cnt);
		if (AlreadyExists ())
			return (EXIT_FAILURE);

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.br_no);
		strcpy (ccmr_rec.cc_no,  local_rec.wh_no);
		if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess100));
			sleep (SLEEPTIME);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("account"))
	{
		if (end_input)
		{
			strcpy (local_rec.accdesc, "");
			skip_entry = -2;
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
		{
			if (prog_status != EDIT)
			{
				print_mess (ML (mlStdMess005));
				sleep (SLEEPTIME);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			DeleteLine ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			strcpy (local_rec.accdesc, "");
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		cc = FindAcct (local_rec.account, FALSE);
		if (cc)
			return (EXIT_FAILURE);

		sprintf (local_rec.accdesc, "%-25.25s", glmrRec.desc);
		DSP_FLD ("accdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("accdesc"))
	{
		DSP_FLD ("accdesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}



void
DeleteLine (void)
{
	int	i;

	for (i = line_cnt; i < lcount [2] - 1; i++)
	{
		getval (i + 1);
		putval (i);
	}
	lcount [2]--;
	getval (line_cnt);
	scn_write (2);
	scn_display (2);
}



int
AlreadyExists (void)
{
	int		i,
			numLines,
			found = FALSE;
	char	branch [3],
			warehouse [3];

	strcpy (branch, local_rec.br_no);
	strcpy (warehouse, local_rec.wh_no);

	numLines = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0; i < numLines; i++)
	{
		getval (i);
		if (!strcmp (local_rec.br_no, branch) &&
			!strcmp (local_rec.wh_no, warehouse) &&
			i != line_cnt)
		{
			found = TRUE;
			break;
		}
	}

	getval (line_cnt);

	if (found)
	{
		print_mess (ML ("An entry for that branch and warehouse already exists."));
		sleep (SLEEPTIME);
		clear_mess ();
	}

	return (found);
}



int
CompanyLevelExists (void)
{
	int i;
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);
		if (!strcmp (local_rec.br_no, "  ") && !strcmp (local_rec.wh_no, "  "))
			return (TRUE);
	}
	print_mess (ML ("No company level entry exists."));
	sleep (SLEEPTIME);
	clear_mess ();
	return (FALSE);
}

/*
 * Find account number, account description etc. 
 */
int
FindAcct (
 char               acc_no [MAXLEVEL + 1],
 int                silent)
{
	sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL, MAXLEVEL, acc_no);

	if (!strcmp (glmrRec.acc_no, "0000000000000000"))
	{
		if (!silent)
		{
			print_mess (ML ("General Ledger Account Code Must be input."));
			sleep (SLEEPTIME);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}
	strcpy (glmrRec.co_no, comm_rec.co_no);
	if ( (cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{
		if (!silent)
		{
			print_err (ML (mlStdMess024));
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
	{
		if (silent)
			return (EXIT_FAILURE);

		print_err (ML (mlStdMess025));
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

void
SrchGlrc (
 char*              keyVal)
{
	work_open ();
	save_rec ("#Recovery Code", "#Description");
	strcpy (glrc_rec.co_no, comm_rec.co_no);
	sprintf (glrc_rec.code, "%-5.5s", keyVal);
	for (cc = find_rec (glrc, &glrc_rec, GTEQ, "r");
		 !cc &&
		 !strcmp (glrc_rec.co_no, comm_rec.co_no) &&
		 !strncmp (glrc_rec.code, keyVal, strlen (keyVal));
		 cc = find_rec (glrc, &glrc_rec, NEXT, "r"))
	{
		cc = save_rec (glrc_rec.code, glrc_rec.desc);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (glrc_rec.co_no, comm_rec.co_no);
	sprintf (glrc_rec.code, "%-5.5s", temp_str);
	cc = find_rec (glrc, &glrc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, glrc, "DBFIND");
}

void
SrchEsmr (
 char*              keyVal)
{
	work_open ();
	save_rec ("#Branch", "#Description");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", keyVal);
	for (cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
		 !cc && !strcmp (esmr_rec.co_no, comm_rec.co_no);
		 cc = find_rec (esmr, &esmr_rec, NEXT, "r"))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
 char*              keyVal)
{
	work_open ();
	save_rec ("#Warehouse", "#Description");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", keyVal);
	for (cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
		 !cc &&
		 !strcmp (ccmr_rec.co_no, comm_rec.co_no)
		 && !strcmp (ccmr_rec.est_no, local_rec.br_no);
		 cc = find_rec (ccmr, &ccmr_rec, NEXT, "r"))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}



void
ReadGlri (void)
{
	scn_set (DETAIL_SCN);

	lcount [DETAIL_SCN] = 0;

	strcpy (glri_rec.co_no, comm_rec.co_no);
	sprintf (glri_rec.code, "%5.5s", glrc_rec.code);
	strcpy (glri_rec.br_no, "  ");
	strcpy (glri_rec.wh_no, "  ");
	for (cc = find_rec (glri, &glri_rec, GTEQ, "r");
         !cc &&
		  !strcmp (glri_rec.co_no, comm_rec.co_no) &&
		  !strcmp (glri_rec.code, glrc_rec.code);
		 cc = find_rec (glri, &glri_rec, NEXT, "r"))
	{
		strcpy (local_rec.co_no,  comm_rec.co_no);
		strcpy (local_rec.br_no,  glri_rec.br_no);
		strcpy (local_rec.wh_no,  glri_rec.wh_no);
		sprintf (local_rec.account, "%*.*s", MAXLEVEL,MAXLEVEL,glri_rec.acc_no);
		FindAcct (glri_rec.acc_no, TRUE);
		sprintf (local_rec.accdesc, "%-25.25s", glmrRec.desc);
		putval (lcount [DETAIL_SCN]++);
	}
	scn_set (HEADER_SCN);
}



void
UpdateGlrc (void)
{
	if (newCode)
	{
		cc = abc_add (glrc, &glrc_rec);
		if (cc)
			file_err (cc, glrc, "ADD");
	}
	else
	{
		cc = abc_update (glrc, &glrc_rec);
		if (cc)
			file_err (cc, glrc, "UPDATE");
	}
}

void
UpdateGlri (void)
{
	scn_set (2);

	/* 
	 * Delete all but company level record 
	 */
  	strcpy (glri_rec.co_no, comm_rec.co_no);
  	sprintf (glri_rec.code,  "%-5.5s", glrc_rec.code);
  	strcpy (glri_rec.br_no, " ");
  	strcpy (glri_rec.wh_no, " ");
	cc = find_rec (glri, &glri_rec, GTEQ, "u");
	while (!cc && !strcmp (glri_rec.co_no, comm_rec.co_no) &&
		  		  !strcmp (glri_rec.code, glrc_rec.code))
	{
		cc = abc_delete (glri);
		if (cc)
			file_err (cc, glri, "DELETE");

		strcpy (glri_rec.co_no, comm_rec.co_no);
		sprintf (glri_rec.code,  "%-5.5s", glrc_rec.code);
		strcpy (glri_rec.br_no, " ");
		strcpy (glri_rec.wh_no, " ");
		cc = find_rec (glri, &glri_rec, GTEQ, "u");
	}
	abc_unlock (glri);

	/* 
	 * Add/update all records 
	 */
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);
  		strcpy (glri_rec.co_no, comm_rec.co_no);
  		sprintf (glri_rec.code,  "%-5.5s", glrc_rec.code);
  		strcpy (glri_rec.br_no, local_rec.br_no);
  		strcpy (glri_rec.wh_no, local_rec.wh_no);
  		sprintf (glri_rec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL,local_rec.account);
 		cc = find_rec (glri, &glri_rec, EQUAL, "u");
		if (cc)
		{
			cc = abc_add (glri, &glri_rec);
			if (cc)
				file_err (cc, glri, "ADD");
		}
		else
		{
			cc = abc_update (glri, &glri_rec);
			if (cc)
				file_err (cc, glri, "ADD");
		}
	}
}



MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES.       ",
	  "" },
	{ " 2. IGNORE CHANGES MADE TO RECORD.    ",
	  "" },
	{ " 3. DELETE CURRENT RECORD.            ",
	  "" },
	{ ENDMENU }
};

/*
 * Update mini menu. 
 */
int
UpdateMenu (void)
{
	for (;;)
	{
	    mmenu_print ("   U P D A T E    S E L E C T I O N.  ",upd_menu,0);
	    switch (mmenu_select (upd_menu))
	    {
		case 	SEL_UPDATE :
		case 	DEFAULT :
			return (Update ());

		case 	SEL_IGNORE :
			return (TRUE);

		case 	SEL_DELETE :
			return Delete ();

	    }
	}
}

int
Update (void)
{
	if (!CompanyLevelExists ())
		return (FALSE);
	UpdateGlri ();
	UpdateGlrc ();
	return (TRUE);
}

int
Delete (void)
{
	if (newCode)
	{
		print_mess (ML (mlStdMess032));
		sleep (SLEEPTIME);
		clear_mess ();
		return (FALSE);
	}
	strcpy (glra_rec.co_no, comm_rec.co_no);
	sprintf (glra_rec.code,  "%-5.5s", glrc_rec.code);
	cc = find_rec (glra, &glra_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (glri_rec.co_no, comm_rec.co_no);
		sprintf (glri_rec.code,  "%-5.5s", glrc_rec.code);
		strcpy (glri_rec.br_no, "  ");
		strcpy (glri_rec.wh_no, "  ");
		for (cc = find_rec (glri, &glri_rec, GTEQ, "u");
			 !cc &&
			  !strcmp (glri_rec.co_no, comm_rec.co_no) &&
			  !strcmp (glri_rec.code, glrc_rec.code);
			 cc = find_rec (glri, &glri_rec, GTEQ, "u"))
		{
			cc = abc_delete (glri);
			if (cc)
				file_err (cc, glri, "DELETE");
		}
		cc = abc_delete (glrc);
		if (cc)
			file_err (cc, glrc, "DELETE");
	}
	else
	{
		print_mess (ML (mlStdMess032));
		sleep (SLEEPTIME);
		clear_mess ();
	}
	return (TRUE);
}



int
heading (
 int                screen)
{
	if (!restart)
	{

		if (screen != cur_screen)
			scn_set (screen);

		clear ();

		strcpy (err_str, ML (" General Ledger Recovery Code Maintenance "));
		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

		line_at (1, 0, 80);

		box (0, 2, 78, 2);

		switch (screen)
		{
		case HEADER_SCN :
			scn_write (HEADER_SCN);
			scn_display (HEADER_SCN);
			scn_set (DETAIL_SCN);
			scn_write (DETAIL_SCN);
			if (prog_status != ENTRY)
				scn_display (DETAIL_SCN);
			scn_set (HEADER_SCN);
			break;
		case DETAIL_SCN :
			scn_set (HEADER_SCN);
			scn_write (HEADER_SCN);
			scn_display (HEADER_SCN);
			scn_set (DETAIL_SCN);
			break;
		}

		line_at (20,0,80);

		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (21,0, "%s", err_str);

		line_at (22,0,80);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (screen);
	}
    return (EXIT_SUCCESS);
}

void
tab_other (
	int		iline)
{
	getval (iline);
	if (!strcmp (local_rec.br_no, "  "))
		FLD ("warehouse") = NA;
	else
		FLD ("warehouse") = NO;
}
