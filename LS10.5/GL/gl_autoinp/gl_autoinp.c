/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_autoinp.c,v 5.5 2002/07/25 11:17:28 scott Exp $
|  Program Name  : (gl_autoinp.c)
|  Program Desc  : (General Ledger Standing Journal Input Program)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: gl_autoinp.c,v $
| Revision 5.5  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.4  2002/07/18 06:39:31  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2001/08/09 09:13:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:03  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:29  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_autoinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_autoinp/gl_autoinp.c,v 5.5 2002/07/25 11:17:28 scott Exp $";

/*
 *   Include file dependencies  
 */
#define MAXWIDTH	200
#define MAXLINES	1000

#include <pslscr.h>
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <GlUtils.h>
#include <twodec.h>

#include	"schema"

struct commRecord	comm_rec;
struct glajRecord	glaj_rec;

/*
 * Constants, defines and stuff 
 */
char    *data = "data";

/*
 *   Local variables  
 */
char	envVarCurrCode [4];

	/*
	 * Special fields and flags  ##################################
	 */
	int	newJournal = TRUE;

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	systemDate [11];
	char	save_class [2];
	char	old_dbt_acc [FORM_LEN + 1];
	char	new_dbt_acc [FORM_LEN + 1];
	char	dbt_desc [26];
	char	old_crd_acc [FORM_LEN + 1];
	char	new_crd_acc [FORM_LEN + 1];
	char	crd_desc [26];
	char	curr_desc [41];
	char	curr_code [4];
	double	exch_rate;
} local_rec;

static struct var vars [] =
{
	{1, LIN, "curr",	3, 15, CHARTYPE,
		"UUU", "          ",
		" ", envVarCurrCode, "Currency Code", "Default is local currency",
		 YES, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{1, LIN, "curr_desc",	3, 35, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},
	{2, TAB, "refer",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "User Reference.", "Enter standing journal reference.",
		 NE, NO,  JUSTLEFT, "", "", glaj_rec.user_ref},
	{2, TAB, "old_dbt",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.old_dbt_acc},
	{2, TAB, "new_dbt",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "Enter Debit Account Number. ",
		 NO, NO,  JUSTLEFT, "1234567890*", "", local_rec.new_dbt_acc},
	{2, TAB, "dbt_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dbt_desc},
	{2, TAB, "save_class",	 0, 0, CHARTYPE,
		"A", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.save_class},
	{2, TAB, "old_crd",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.old_crd_acc},
	{2, TAB, "new_crd",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "Enter Credit Account Number. ",
		YES, NO,  JUSTLEFT, "1234567890*", "", local_rec.new_crd_acc},
	{2, TAB, "crd_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.crd_desc},
	{2, TAB, "datefrom",	 0, 1, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Effect. From", "Enter Starting Date of Journal.",
		YES, NO, JUSTRIGHT, "", "", (char *)&glaj_rec.ef_frm},
	{2, TAB, "dateto",	 0, 1, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Effective To", "Enter Date When Journal Expires. ",
		YES, NO, JUSTRIGHT, "", "", (char *)&glaj_rec.ef_to},
	{2, TAB, "day",	 0, 1, CHARTYPE,
		"NN", "          ",
		"0", "01", "App ", "Range: 1 to 28",
		YES, NO, JUSTRIGHT, "01", "28", glaj_rec.date_appl},

	{2, TAB, "curr_code", 0, 0, CHARTYPE,
		"UUU", "          ",
		" ", "", "Curr", "",
		NA, NO,  JUSTLEFT, "", "", glaj_rec.curr_code},

	{2, TAB, "orig_amt",	 0, 1, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Orig. Amount ", "Enter an amount of zero to delete record on update.",
		YES, NO,  JUSTRIGHT, "0", "99999999.99", (char *)&glaj_rec.orig_amt},

	{2, TAB, "exch_rate",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", " ", "Exchange Rate ", " ",
		NA, NO,  JUSTLEFT, "", "99999999",(char *)&glaj_rec.exch_rate},

	{2, TAB, "loc_amt",	 0, 2, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", "Local Amount ", "",
		NA, NO,  JUSTRIGHT, "0", "99999999.99", (char *)&glaj_rec.loc_amt},

	{2, TAB, "lst_prc",	 0, 0, EDATETYPE,
		"DD/DD/DD", "        ",
		"0", "01/01/01", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *)&glaj_rec.date_lproc},
	{2, TAB, "stat_flag",	 0, 0, CHARTYPE,
		"A", "          ",
		"0", "0", "", "",
		 ND, NO,  JUSTLEFT, "0", "", glaj_rec.stat_flag},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 *   Local function prototypes 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	LoadGlaj 		(void);
int  	Update 			(void);
int  	CheckClass 		(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{

    SETUP_SCR (vars);

	search_ok = TRUE;

	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	OpenDB ();

	GL_SetMask (GlFormat);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	strcpy (local_rec.old_crd_acc, " ");
	strcpy (local_rec.old_dbt_acc, " ");
	strcpy (local_rec.new_crd_acc, " ");
	strcpy (local_rec.new_dbt_acc, " ");

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	swide ();

	while (!prog_exit)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		heading(1);
		entry (1);
		if (restart || prog_exit)
        {
        	shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		LoadGlaj ();

		if (newJournal)
		{
			/*
			 * Enter Screen 2 Tabular Input.
			 */
			heading (2);
			entry (2);
			if (restart)
				continue;
		}
		edit_all();

		if (restart)
			continue;

		/*
		 * Update Standing Journal Record.
		 */
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program Exit Sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Data Base Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (glaj, glaj_list, GLAJ_NO_FIELDS, "glaj_id_no");
	OpenGljc ();
	OpenPocr ();
	OpenGlmr ();
}

/*
 * Close Data Base Files
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (glaj);
	abc_dbclose (data);
}

int
heading (
	int		scn)
{
	int	s_size = 130;

	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	centre_at (0, 132, ML (mlGlMess157));
	line_at (1, 0, s_size);
	
	if (scn == 1)
		box(0,2,s_size,1);

	line_at (19, 0, s_size);
	print_at (20,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	print_at (21,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
	line_at (22, 0, s_size);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
int
spec_valid (
	int		field)
{
	int		i = 0;

	if (LCHECK ("curr"))
	{
		if (SRCH_KEY)
		{
            SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		} 
		if (dflt_used)
		{
            cc = FindPocr (comm_rec.co_no, envVarCurrCode, "r");
			if (cc)
			{
				print_mess(ML (mlStdMess040));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.curr_desc,"%-40.40s", pocrRec.description);
			local_rec.exch_rate = pocrRec.ex1_factor;
			DSP_FLD ("curr");
			DSP_FLD ("curr_desc");
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
		cc = FindPocr (comm_rec.co_no, local_rec.curr_code, "r");
		if (cc)
		{
			print_mess(ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.curr_desc,"%-40.40s", pocrRec.description);
		sprintf (local_rec.curr_code,"%-3.3s", pocrRec.code);
		local_rec.exch_rate = pocrRec.ex1_factor;
		DSP_FLD ("curr");
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Debit General Ledger Account Number.
	 */
	if (LCHECK ("new_dbt"))
	{
		if (dflt_used)
			strcpy (local_rec.new_dbt_acc, GL_GetDfltSfaccCode ());

		if (SRCH_KEY)
		{
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());

			return (SearchGlmr_F (comm_rec.co_no, temp_str, "**P"));
		}

		GL_FormAccNo (local_rec.new_dbt_acc, glaj_rec.dbt_acc_no, 0);
		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, glaj_rec.dbt_acc_no);
		if (!GL_ValidUserCode (glmrRec.acc_no))
		{
			print_err (ML (mlStdMess140));
			return(1);
		}
        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess024));
			return (EXIT_FAILURE);
		}

        if (CheckClass (glmrRec.glmr_class [0]))
        {
			return (EXIT_FAILURE);
        }

		strcpy (local_rec.save_class, glmrRec.glmr_class [0]);
		sprintf (local_rec.dbt_desc, glmrRec.desc);

		if (prog_status == ENTRY)
			print_at (3, 0, ML (mlGlMess026), local_rec.dbt_desc);
        
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Credit General Ledger Account Number.
	 */
	if (LCHECK ("new_crd"))
	{
		if (dflt_used)
			strcpy (local_rec.new_crd_acc, GL_GetDfltSfaccCode ());

		if (SRCH_KEY)
		{
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());

			return (SearchGlmr_F (comm_rec.co_no, temp_str, "**P"));
		}

		GL_FormAccNo (local_rec.new_crd_acc, glaj_rec.crd_acc_no, 0);
		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, glaj_rec.crd_acc_no);
		if (!GL_ValidUserCode (glmrRec.acc_no))
		{
			print_err (ML (mlGlMess168),glmrRec.acc_no);
			return(1);
		}

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess024));
			return (EXIT_FAILURE);
		}

		if (CheckClass (local_rec.save_class))
        {
			return (EXIT_FAILURE);
        }

		sprintf (local_rec.crd_desc, glmrRec.desc);

		if (prog_status == ENTRY)
			print_at (4, 0, ML (mlGlMess027), local_rec.crd_desc);
        
		strcpy (glaj_rec.type, " 1");
		strcpy (glaj_rec.co_no, comm_rec.co_no);

        cc = find_rec (glaj, &glaj_rec, COMPARISON, "r");
		if (!cc)
		{
			for (i = 1; i < 5; i++)
				display_field (field + i);

			skip_entry = 3;
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Day Input.
	 */
	if (LCHECK ("day"))
	{
		i = atoi (glaj_rec.date_appl);

		if (i < 1 || i > 28)
        {
			return (EXIT_FAILURE);
        }

		strcpy (glaj_rec.curr_code, local_rec.curr_code);
		DSP_FLD ("curr_code");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Date to.
	 */
	if (LCHECK ("dateto"))
	{
		if (dflt_used)
		{
			glaj_rec.ef_to = AddYears (glaj_rec.ef_frm, 1);
			DSP_FLD ("dateto");
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("orig_amt"))
	{
		glaj_rec.exch_rate = local_rec.exch_rate;
		DSP_FLD ("exch_rate");
		glaj_rec.loc_amt 	= 	no_dec(glaj_rec.orig_amt / local_rec.exch_rate);

		DSP_FLD ("loc_amt");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
LoadGlaj (void)
{
	newJournal = TRUE;

	scn_set (2);
	lcount [2] = 0;

	strcpy (glaj_rec.co_no, comm_rec.co_no);
	strcpy (glaj_rec.type, " 1");
	strcpy (glaj_rec.curr_code, "   ");
	sprintf (glaj_rec.dbt_acc_no, "%16.16s", " ");
	sprintf (glaj_rec.crd_acc_no, "%16.16s", " ");
	sprintf (glaj_rec.user_ref, "%15.15s"  , " ");
	 
    cc = find_rec (glaj, &glaj_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (glaj_rec.co_no, comm_rec.co_no) &&
           !strcmp (glaj_rec.curr_code, local_rec.curr_code))
	{
		newJournal = FALSE;
		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, glaj_rec.dbt_acc_no);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			sprintf (local_rec.dbt_desc, "%-25.25s", "Not On File");
		else
			sprintf (local_rec.dbt_desc, "%-25.25s", glmrRec.desc);
        
		strcpy (local_rec.save_class, glmrRec.glmr_class [0]);
		strcpy (local_rec.old_dbt_acc, glaj_rec.dbt_acc_no);
		GL_FormAccNo (local_rec.old_dbt_acc, glaj_rec.dbt_acc_no, 0);
		strcpy (local_rec.new_dbt_acc, local_rec.old_dbt_acc);

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, glaj_rec.crd_acc_no);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			sprintf (local_rec.crd_desc, "%-25.25s", "Not On File");
		else
			sprintf (local_rec.crd_desc, "%-25.25s", glmrRec.desc);

		strcpy (local_rec.old_crd_acc, glaj_rec.crd_acc_no);
		GL_FormAccNo (local_rec.old_crd_acc, glaj_rec.crd_acc_no, 0);
		strcpy (local_rec.new_crd_acc, local_rec.old_crd_acc);

		putval (lcount [2]++);

		cc = find_rec (glaj, &glaj_rec, NEXT, "r");
	}
	scn_set (1);
}

int
Update (void)
{
	double	LocAmount	=	0.00,
			FgnAmount	=	0.00;
	char	StatusFlag [2];
	long	DateLastProcessed;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	strcpy (glaj_rec.co_no, comm_rec.co_no);
	sprintf (glaj_rec.dbt_acc_no, "%16.16s", " ");
	sprintf (glaj_rec.crd_acc_no, "%16.16s", " ");
	strcpy (glaj_rec.type, " 1");
	strcpy (glaj_rec.curr_code, local_rec.curr_code);
	sprintf (glaj_rec.user_ref, "%15.15s"  , " ");

    cc = find_rec (glaj, &glaj_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (glaj_rec.co_no, comm_rec.co_no) &&
           !strcmp (glaj_rec.curr_code, local_rec.curr_code))
	{
		cc = abc_delete (glaj);
		if (cc)
			file_err (cc, glaj, "DBDELETE");

		strcpy (glaj_rec.co_no, comm_rec.co_no);
		sprintf (glaj_rec.dbt_acc_no, "%16.16s", " ");
		sprintf (glaj_rec.crd_acc_no, "%16.16s", " ");
		strcpy (glaj_rec.type, " 1");
		strcpy (glaj_rec.curr_code, local_rec.curr_code);
		sprintf (glaj_rec.user_ref, "%15.15s"  , " ");
		cc = find_rec (glaj, &glaj_rec, GTEQ, "r");
	}

	/*
	 * Set to tab screen.
	 */
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		if (glaj_rec.orig_amt == 0.00)   /*should be loc_amt*/
			continue;

		FgnAmount	=	glaj_rec.orig_amt; 
		LocAmount	=	glaj_rec.loc_amt;
		strcpy (StatusFlag, glaj_rec.stat_flag);
		if (glaj_rec.stat_flag [0] == ' ')	
			strcpy (StatusFlag, "0");
        
		DateLastProcessed	=	glaj_rec.date_lproc;
		if (DateLastProcessed <= 0L)
			DateLastProcessed	=	TodaysDate ();

		strcpy (glaj_rec.co_no, comm_rec.co_no);
		GL_StripForm (glaj_rec.dbt_acc_no, local_rec.new_dbt_acc);
		GL_StripForm (glaj_rec.crd_acc_no, local_rec.new_crd_acc);
		strcpy (glaj_rec.type, " 1");

        cc = find_rec ("glaj", &glaj_rec, COMPARISON, "r");
		if (cc)
		{
			glaj_rec.orig_amt	=	FgnAmount;
			glaj_rec.loc_amt	=	LocAmount;
			glaj_rec.date_lproc	=	DateLastProcessed;
			strcpy (glaj_rec.stat_flag, StatusFlag);
			cc = abc_add (glaj, &glaj_rec);
			if (cc)
            {
				file_err (cc, glaj, "DBADD");
            }
		}
		else
		{
			glaj_rec.orig_amt	=	FgnAmount;
			glaj_rec.loc_amt	=	LocAmount;
			strcpy (glaj_rec.stat_flag, StatusFlag);
			glaj_rec.date_lproc	=	DateLastProcessed;
			cc = abc_update (glaj, &glaj_rec);
			if (cc)
				file_err (cc, glaj, "DBUPDATE");
		}
	}
	return (EXIT_SUCCESS);
}

int	
CheckClass (
	char	*other)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
		print_err (ML (mlStdMess025));
		return (EXIT_FAILURE);
	}
	if (glmrRec.glmr_class [0][0] != *other)
	{
		print_err (ML (mlGlMess169));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
tab_other (
 int	line)
{
	print_at (3, 0, ML (mlGlMess026), local_rec.dbt_desc);
	print_at (4, 0, ML (mlGlMess027), local_rec.crd_desc);
}
