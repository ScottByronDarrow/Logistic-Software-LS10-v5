/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_alloc.c,v 5.9 2002/07/25 11:17:28 scott Exp $
|  Program Name  : (gl_alloc.c) 
|  Program Desc  : (GL Journal Allocation File)
|---------------------------------------------------------------------|
|  Author        : Simon Spratt.   | Date Written  : 14/11/95         |
|---------------------------------------------------------------------|
| $Log: gl_alloc.c,v $
| Revision 5.9  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.8  2002/07/24 08:38:52  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/08 10:07:22  robert
| Fixed deletion of lines on LS10-GUI
|
| Revision 5.6  2002/07/08 05:57:16  scott
| S/C 004070. See S/C
|
| Revision 5.5  2002/06/26 05:08:30  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/02/04 02:40:09  scott
| Updated to ensure clear_mess used.
|
| Revision 5.3  2001/08/09 09:13:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:00  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:22  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_alloc.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_alloc/gl_alloc.c,v 5.9 2002/07/25 11:17:28 scott Exp $";

/*
 * Include file dependencies 
 */
#define MAXWIDTH 	150
#define MAXLINES 	112
#define TABLINES 	12

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 * Constants, defines and stuff.
 */
	char	*data	=	"data";

	int  	newGlzh = 	0,
			newGlzl = 	0;	

#include	"schema"

struct commRecord	comm_rec;
struct glzhRecord	glzh_rec;
struct glzlRecord	glzl_rec;

struct storeRec
{
    char	accountNo [FORM_LEN + 1];
} store [MAXLINES];

char *scn_desc [] = 
{
    "GL Allocation Header Information.",
    "GL Allocation Detail Lines."
};

/*
 * Local variables
*/
char 	lcl_acc_type [4] = "**P";
int		runningPercent = 0;
int		inEditAll;

/*
 * Local & Screen Structures.
*/
struct 
{
	char	dummy [11];
	int		perc;
	char	accountNo [FORM_LEN + 1];
	long	hhmrHash;
} local_rec;

char 	accountTitle [32];

static	struct	var vars []	=	
{
	{1, LIN, "allocateCode", 3, 19, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Allocation Code.", "[SEARCH] for valid codes.", 
		NE, NO, JUSTLEFT, "", "", glzh_rec.code}, 
	{1, LIN, "description", 4, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		YES, NO, JUSTLEFT, "", "", glzh_rec.description}, 
	{1, LIN, "currency", 5, 19, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Currency", " ", 
		NE, NO, JUSTLEFT, "", "", glzh_rec.currency}, 
	{2, TAB, "glacct", MAXLINES, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		"0", "0", accountTitle, " ", 
		YES, NO, JUSTLEFT, "1234567890-", "", local_rec.accountNo}, 
	{2, TAB, "glDesc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		" ", " ", "Description              ", " ", 
		NA, NO, JUSTLEFT, "", "", glmrRec.desc}, 
	{2, TAB, "percentage", 0, 3, INTTYPE, 
		"NNN", "           ", 
		"", "0", "Percentage", " ", 
		YES, NO, JUSTRIGHT, "1234567890*", "", (char *) &local_rec.perc}, 
	{2, TAB, "hhmrHash", 0, 0, INTTYPE, 
		"NNNNNNNNNNNNN", "           ", 
		"0", "0", "", " ", 
		ND, NO, JUSTRIGHT, "1234567890*", "", (char *) &local_rec.hhmrHash}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Local variables
 */
void 	shutdown_prog 		(void);
void 	CloseDB 			(void);
void 	DeleteGlzl 			(char *);
void 	LoadGlzl 			(void);
void 	OpenDB 				(void);
void 	ShowPercent 		(void);
void 	SrchGlzh 			(char *);
void 	Update 				(void);
int  	spec_valid 			(int);
int  	heading 			(int);
int  	CalculatePercent 	(int);
int  	CheckDupAccount 	(char *);
int  	DeleteLine 			(void);
int  	ExistsAccountNo 	(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int   argc,
 char *argv [])
{
	int	i;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();

	OpenDB ();

	vars [label ("glacct")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
	set_masks ();		/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	sprintf (accountTitle, "Account%*.*s", 
			 (int) strlen (vars [label ("glacct")].mask) - 7,
			 (int) strlen (vars [label ("glacct")].mask) - 7,
	  		" ");		

	for (i = 0; i < 2; i++)
        tab_data [i]._desc = scn_desc [i];

	tab_row = 7;
	tab_col = 10;
	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		inEditAll 		= FALSE;
		runningPercent 	= FALSE;
		entry_exit 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		search_ok 		= TRUE;
		init_ok 		= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;
		
		scn_write (1);
		scn_display (1);

		/*
		 * Enter screen 2 Tabular input.
		 */
		if (prog_exit || restart)
			continue;

		if (newGlzl)
		{
			entry_exit = FALSE;
			heading (2);
			entry (2);
		}
		scn_write (2);
		scn_display (2);

		if (prog_exit || restart)
			continue;

		do 
        {
            inEditAll = TRUE;
			edit_all ();

			if ((runningPercent != 100) && 
                !restart && 
                !prog_exit)
			{
				print_mess (ML (mlGlMess015));
				sleep (sleepTime);
				clear_mess ();
			}

		} while ((runningPercent != 100) && !restart && !prog_exit);

		if (restart)
			continue;
		
		/*
		 * Update records.
		 */
		Update ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS,  (char *) &comm_rec);

	OpenGlmr ();
	open_rec (glzh, glzh_list, GLZH_NO_FIELDS, "glzh_id_no");
	open_rec (glzl, glzl_list, GLZL_NO_FIELDS, "glzl_id_no");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (glzh);
	abc_fclose (glzl);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Customer Number And Allow Search.
	 */
	if (LCHECK ("allocateCode"))
	{
		if (SRCH_KEY)
		{
	 		SrchGlzh (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (glzh_rec.co_no, comm_rec.co_no);

        cc = find_rec (glzh, &glzh_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (glzh);
			newGlzh = TRUE;
			newGlzl = TRUE;
			return (EXIT_SUCCESS);
		}
		newGlzh = FALSE;
		newGlzl = FALSE;
		LoadGlzl ();
		entry_exit = TRUE;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("currency"))
	{
		if (SRCH_KEY)
		{
	 		SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}
		cc = FindPocr (comm_rec.co_no, glzh_rec.currency, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate General Ledger Account Number.
	 */
	if (LCHECK ("glacct")) 
	{
		if (vars [label ("glacct")].mask [0] ==  '*')
		{
			print_mess (ML (mlGlMess001));
			restart = TRUE;
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (prog_status == ENTRY)
				return (EXIT_FAILURE);
            
			DeleteLine ();
			runningPercent = CalculatePercent (TRUE);
			ShowPercent ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (!strlen (temp_str))
				strcpy (temp_str, GL_GetfUserCode ());
            
			return (SearchGlmr_CF (comm_rec.co_no, temp_str, lcl_acc_type, glzh_rec.currency));
		}

		strcpy (glmrRec.acc_no,"0000000000000000");
		if (GL_FormAccNo (local_rec.accountNo, glmrRec.acc_no, 0))
			return (EXIT_FAILURE);

		strcpy (glmrRec.co_no, comm_rec.co_no);
        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{	
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [2][0] != 'P')
       	{
			print_err (ML (mlStdMess025));
			return (EXIT_FAILURE);
		}

		if (lcl_acc_type [0] == '*')
			lcl_acc_type [0] = glmrRec.glmr_class [0][0];
		else
		{
			if (lcl_acc_type [0] != glmrRec.glmr_class [0][0])
			{
				print_err (ML (mlGlMess003));
				return (EXIT_FAILURE);
			}
		}
		/*
		 * Check for duplicate account entered.
		 */
		if (CheckDupAccount (local_rec.accountNo))
		{
			print_mess (ML (mlGlMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strncmp (glmrRec.curr_code, glzh_rec.currency, 3))
		{
			print_mess (ML (mlGlMess005));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].accountNo,local_rec.accountNo);
		local_rec.hhmrHash = glmrRec.hhmr_hash;
		DSP_FLD ("glDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("percentage"))
	{
		putval (line_cnt);
		runningPercent = CalculatePercent (FALSE);
		if (dflt_used)
		{
			local_rec.perc = 100 - runningPercent;
			entry_exit = TRUE;
			DSP_FLD ("percentage");
			putval (line_cnt);
		}
		if (local_rec.perc == 0)
		{
			print_mess (ML (mlGlMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.perc > 100)
		{
			print_mess (ML (mlGlMess007));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.perc + runningPercent > 100)
		{
			print_mess (ML (mlGlMess008));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if ((local_rec.perc + runningPercent == 100) && !inEditAll)
		{
			if (prog_status == ENTRY)
			{
				entry_exit = TRUE;
				putval (line_cnt++);
			}
		}
		runningPercent += local_rec.perc;
		ShowPercent ();
	}
	return (EXIT_SUCCESS);
}	

void
LoadGlzl (void)
{
	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (2);
	init_vars (2);
	lcount [2] = 0;

	glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
	strcpy (glzl_rec.gl_acc_no, "");

    cc = find_rec (glzl, &glzl_rec, GTEQ, "r");
	while (!cc && (glzh_rec.hhbh_hash == glzl_rec.hhbh_hash))
	{
		fflush (stdout);
		strcpy (local_rec.accountNo, glzl_rec.gl_acc_no);
		GL_FormAccNo (local_rec.accountNo, glzl_rec.gl_acc_no, 0);
		
		/*
		 * read glmr to get account decsription.
		 */
		strcpy (glmrRec.co_no,comm_rec.co_no);
		strcpy (glmrRec.acc_no,glzl_rec.gl_acc_no);

        cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess024));
			sleep (sleepTime);
		}
		local_rec.perc = glzl_rec.percentage;
		runningPercent += glzl_rec.percentage;
		strcpy (store [lcount [2]].accountNo, local_rec.accountNo);
		putval (lcount [2]++);
		cc = find_rec (glzl, &glzl_rec, NEXT, "r");
	}

	if (lcount [2] == 0)
		newGlzl = TRUE;
    
	scn_set (1);
}

/*
 * Search for Allocation Codes.
 */
void
SrchGlzh (
 char *key_val)
{
	_work_open (4,0,40);
	save_rec ("#Code","# Description ");

	strcpy (glzh_rec.co_no, comm_rec.co_no);
	sprintf (glzh_rec.code,"%-4.4s", key_val);

    cc = find_rec (glzh, &glzh_rec, GTEQ, "r");
	while (!cc && !strcmp (glzh_rec.co_no, comm_rec.co_no) &&
           		  !strncmp (glzh_rec.code, key_val,strlen (key_val)))
	{
		cc = save_rec (glzh_rec.code,glzh_rec.description);
		if (cc)
			break;

		cc = find_rec (glzh, &glzh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (glzh_rec.co_no, comm_rec.co_no);
	sprintf (glzh_rec.code,"%-4.4s", temp_str);

    cc = find_rec (glzh, &glzh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, glzh, "DBFIND");
}


/*
 * Update glzh/glzl records.
 */
void
Update (void)
{
	char previousAccNo [FORM_LEN + 1];

    scn_set (1);
	clear ();
	print_at (0,0, ML (mlGlMess009));
	if (newGlzh)
	{
		cc = abc_add (glzh, &glzh_rec);
		if (cc)
			file_err (cc, glzh, "DBADD");

		strcpy (glzh_rec.co_no, comm_rec.co_no);

        cc = find_rec (glzh, &glzh_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, glzh, "DBADD");
	}
	else
	{
		cc = abc_update (glzh, &glzh_rec);
		if (cc)
			file_err (cc, glzh, "DBUPDATE");

		glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
		strcpy (glzl_rec.gl_acc_no, "");
        cc = find_rec (glzl, &glzl_rec, GTEQ, "u");
		while (!cc && (glzh_rec.hhbh_hash == glzl_rec.hhbh_hash))
		{
			strcpy (previousAccNo, glzl_rec.gl_acc_no);
			if (!ExistsAccountNo (glzl_rec.gl_acc_no))
			{
				DeleteGlzl (glzl_rec.gl_acc_no);
			}
			glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
			strcpy (glzl_rec.gl_acc_no, previousAccNo);
			cc = find_rec (glzl, &glzl_rec, GREATER, "r");
		}
	}
    scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);
		glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
		GL_StripForm (glzl_rec.gl_acc_no, local_rec.accountNo);
        cc = find_rec (glzl, &glzl_rec, COMPARISON, "r");
		if (cc)
		{
			glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
			GL_StripForm (glzl_rec.gl_acc_no, local_rec.accountNo);
			glzl_rec.percentage = local_rec.perc;
			glzl_rec.hhmr_hash = local_rec.hhmrHash;

            cc = abc_add (glzl, &glzl_rec);
			if (cc)
				file_err (cc, glzl, "DBADD");
		}
		else
		{
			GL_StripForm (glzl_rec.gl_acc_no, local_rec.accountNo);
			glzl_rec.percentage = local_rec.perc;
			glzl_rec.hhmr_hash = local_rec.hhmrHash;

            cc = abc_update (glzl, &glzl_rec);
			if (cc)
				file_err (cc, glzl, "DBUPDATE");
		}
	}
}


int
CalculatePercent (
	int		full)
{
	int loop;
	int upper;
	int sum = 0;

    if (prog_status == ENTRY)
		upper = line_cnt + 1;
	else
		upper = lcount [2];

	for (loop = 0; loop < upper; loop++)
	{
		if (!full && (loop == line_cnt))
			continue;

		getval (loop);
		sum += local_rec.perc;
	}	
	getval (line_cnt);
	return (sum);
}

/*
 * Check for duplicate GL account code.
 */
int
CheckDupAccount (
	char *accountNo)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_items;i++)
	{
		/*
		 * Ignore Current Line
		 */
		if (i == line_cnt)
			continue;

		if (!strcmp (store [i].accountNo, accountNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Show the percentage as entered.
 */
void
ShowPercent (void)
{
	sprintf (err_str, ML (mlGlMess011) ,runningPercent);
	rv_pr (err_str,42,5,1);
}

/*
 * Delete line
 */
int
DeleteLine (void)
{
    int i;
    int	this_page;

    if (prog_status == ENTRY)
    {
        print_mess (ML (mlStdMess005));
        sleep (sleepTime);
        clear_mess ();
        return (EXIT_FAILURE);
    }

    lcount [2]--;

    this_page = line_cnt / TABLINES;
    for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
    {
        getval (line_cnt + 1);
        putval (line_cnt);
        strcpy (store [i].accountNo, store [i + 1].accountNo);
        if (this_page == (line_cnt / TABLINES))
            line_display ();
    }

#ifndef GVISION
    init_vars (2);
    putval (line_cnt);
    if (this_page == (line_cnt / TABLINES))
        blank_display ();
#endif
	
    line_cnt = i;
    getval (line_cnt);

    if (lcount [2] == 0)
        strcpy (lcl_acc_type, "**P");

    return (EXIT_SUCCESS);
}

void
DeleteGlzl (
	char *accountNo)
{
    glzl_rec.hhbh_hash = glzh_rec.hhbh_hash;
    GL_StripForm (glzl_rec.gl_acc_no, accountNo);
    cc = find_rec (glzl, &glzl_rec, COMPARISON, "u");
	if (cc)
		return;

	cc = abc_delete (glzl);
	if (cc)
		file_err (cc, glzl, "DBDELETE");
}

int
ExistsAccountNo (
	char *accountNo)
{
	char formAccountNo [FORM_LEN + 1];

    strcpy (formAccountNo, accountNo);
	GL_FormAccNo (formAccountNo, accountNo, 0);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
        getval (line_cnt);
        if (!strcmp (formAccountNo, local_rec.accountNo))
			return (TRUE);
	}
	return (FALSE);
}
int
heading (
 int scn)
{
	if (restart) 
	{
		abc_unlock (glzh);
		return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlGlMess010),18,0,1);


	box (0,2,80,3);
	line_at (1,0,79);
	line_at (21,0,80);

	scn_set ((scn == 1) ? 2 : 1);
	scn_write ((scn == 1) ? 2 : 1);
	scn_display ((scn == 1) ? 2 : 1);

	strcpy (err_str, ML (mlStdMess038));	
	print_at (22,0, err_str, comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	if (scn == 2)
		ShowPercent ();
	
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
