/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: chartdsp.c,v 5.5 2002/07/17 09:57:12 scott Exp $
|  Program Name  : (gl_chartdsp.c)                               
|  Program Desc  : (Display and Prints G/L chart of accounts.)
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 11/07/89         |
|---------------------------------------------------------------------|
| $Log: chartdsp.c,v $
| Revision 5.5  2002/07/17 09:57:12  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/08 07:49:52  scott
| S/C 004061 - Updated for layout of report.
|
| Revision 5.3  2001/08/09 09:13:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:10  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:39  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: chartdsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_chartdsp/chartdsp.c,v 5.5 2002/07/17 09:57:12 scott Exp $";

/*
 * Include file dependencies  
 */
#include <pslscr.h>
#include <GlUtils.h>
#include <getnum.h>
#include <hot_keys.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 *   Constants, defines and stuff   
 */
#define	PSL_PRINT
#define	X_OFF	20
#define	Y_OFF	4
#define InternalPageSize 	13

char	*glln2 = "glln2",
		*glmr2 = "glmr2",
		*data  = "data";

/*
 *   Local variables 
 */
extern  int GV_cur_level, 
            GV_max_level;

int     print_ok = FALSE,
        ln_num = 0;

char    prev_acc [FORM_LEN + 1];
FILE   *fout;

static char PV_end_acc [28],
            PV_start_acc [28];

static int  line_drawn = FALSE,
            PV_print = FALSE,
            lpno = 1;

static int  link_level = 0;

#include	"schema"

struct commRecord	comm_rec;

static struct
{
    char	*class_code,    /*  Class code          */
            *class_desc;    /*  Class Description   */
} *class_ptr, class_tab [] = 
		{                  
            {"CC ",	"Company Control"},

            {"NFC",	"Non-financial Control"},
            {"NFS",	"Non-financial Summary"},
            {"NFP",	"Non-financial Posting"},

			{"FAS",	"Assets Summary"},
			{"FES",	"Expense Summary"},
			{"FIS",	"Income Summary"},
			{"FLS",	"Liability Summary"},

			{"FAP",	"Assets Posting"},
			{"FEP",	"Expense Posting"},
			{"FIP",	"Income Posting"},
			{"FLP",	"Liability Posting"},
		  	{NULL}
		};

static struct
{
    char    loc_start  	 [28],
            loc_end  	 [28],
            loc_type  	 [2];
    int     loc_level;
} local_rec;
		
static struct var vars [] =
{	
	{1, LIN, "type", 2, 14, CHARTYPE, 
		"U", "                          ", 
		" ", "F", "Account Type ", "Account type F)inancial or N)on-financial.", 
		NE, NO, JUSTLEFT, "NF", "", local_rec.loc_type}, 
	{1, LIN, "level_no", 2, 68, INTTYPE, 
		"N", "          ", 
		" ", "0", "Account Level", " ", 
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.loc_level}, 
	{1, LIN, "acc_no", 3, 14, CHARTYPE, 
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ", 
		" ", "0", "Start Account", " ", 
		NE, NO, JUSTLEFT, "0123456789-", "", local_rec.loc_start}, 
	{1, LIN, "end_acc", 3, 68, CHARTYPE, 
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ", 
		" ", "99999999999999999", "End Account", " ", 
		NE, NO, JUSTLEFT, "0123456789-", "", local_rec.loc_end}, 
	{0, TAB, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

static int PrintChartAccs 	 	(int, KEY_TAB *);
static int DisplayChartAccs 	(int, KEY_TAB *);

#ifdef GVISION
static KEY_TAB chart_keys [] =
{
    { " DISPLAY ",    'D', DisplayChartAccs,
        "Display Chart of Accounts to screen." },
	{ " PRINT ", 'P', PrintChartAccs,
      "Print Chart of Accounts to printer."		  },
      END_KEYS
};
    
#else
static KEY_TAB chart_keys [] =
{
    { " [D]ISPLAY ",    'D', DisplayChartAccs,
        "Display Chart of Accounts to screen." },
	{ " [P]RINT ", 'P', PrintChartAccs,
      "Print Chart of Accounts to printer."		  },
      END_KEYS
};
#endif

/*
 *   Local function prototypes  
 */
int  	SetStartAccount 	 (void);
int	 	SetEndAccount 		 (void);
int  	spec_valid 			 (int);
int  	heading 			 (int);
void 	DrawLineStuff 		 (void);
void 	RunChartHots 		 (void);
void 	DisplayChartHots 	 (void);
void 	DspChartHeading 	 (void);
void 	init_prnt 			 (void);
void 	ProcessFile 		 (void);
void 	PrintLinks 			 (long, long, long);
void 	PrintAccounts 		 (int);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	psl_print 			 (void);
void 	shutdown_prog 		 (void);
char 	*GetCDesc 			 (char *);


/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	static char level_cmnt [81];
	char	*sptr;

	OpenDB ();

	set_help (FN6, "FN6");
	search_ok = TRUE;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
    {
		sptr = argv [0];
    }
	else
    {
		sptr++;
    }

	if (strcmp (sptr, "gl_chartprt"))
	{
		init_scr ();		/*  sets terminal from termcap	  */
		set_tty ();
		swide ();

		SETUP_SCR (vars);
		if (argc == 2)
        {
        	lpno = atoi (argv [1]);
			if (lpno == 0)
				lpno = 1;
        }

		vars [label ("acc_no")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
		vars [label ("end_acc")].mask = vars [label ("acc_no")].mask;

		sprintf (level_cmnt,
                 "Level must be in range (1 to %d) Default = %d.",
                 GV_max_level,
                 GV_max_level);
        
        vars [label ("level_no")].comment = level_cmnt;

		set_masks ();		/*  setup print using masks	  */
		init_vars (1);		/*  set default values		  */
		clear ();

		do
		{
			prog_exit = restart = 0;
			heading (1);
			entry (1);
			if (restart)
            {
				continue;
            }
		
			if (!prog_exit)
			{
				heading (1);
				scn_display (1);

				crsr_off ();
				RunChartHots ();
			}
        } while (!prog_exit);
	}
	else
	{
		GL_SetAccWidth (comm_rec.co_no, TRUE);

		PV_print = TRUE;
		strcpy (local_rec.loc_start, argv [1]);
		strcpy (local_rec.loc_end, argv [2]);
		local_rec.loc_level = atoi (argv [3]);
		strcpy (local_rec.loc_type, argv [4]);
		lpno = atoi (argv [5]);

		SetStartAccount ();
		SetEndAccount ();
		init_prnt ();
		ProcessFile ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int 
DisplayChartAccs (
 int iUnused, 
 KEY_TAB *psUnused)
{
#ifdef GVISION
    destroy_hotbuttons ();
#endif
	DspChartHeading ();
	ProcessFile ();
	heading (1);
	DisplayChartHots ();

    return (EXIT_SUCCESS);
}


int
PrintChartAccs (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	sys_str [128];

#ifdef GVISION
	destroy_hotbuttons ();
#endif
	move (0,21);
	line (132);
	lpno = get_lpno (0);

	sprintf (sys_str, 
             "gl_chartprt \"%s\" \"%s\" \"%d\" \"%s\" \"%d\"",
             local_rec.loc_start, 
             local_rec.loc_end,
             local_rec.loc_level, 
             local_rec.loc_type, lpno);
		
	PrintReport (sys_str, "Chart of Accounts", 80);
	heading (1);
	DisplayChartHots ();
#ifdef GVISION
	set_hotkeys (chart_keys, null_func, null_func);
#endif

    return (EXIT_SUCCESS);
}

void
DspChartHeading (
 void)
{
	char	env_line [200];

	/*
	 * Heading and Page Format For Screen Output Display. 
	 */
	line_drawn = FALSE;
	Dsp_open (0,4,InternalPageSize);
	sprintf (env_line,
             "CHART OF ACCOUNTS DISPLAY - %-2.2s  %-96.96s",
             comm_rec.co_no,
             comm_rec.co_name);
	Dsp_saverec (env_line);
	Dsp_saverec ("       Account Number         |                      Narrative                        |            Class           | Currency   ");
	Dsp_saverec (" [REDRAW]  [PRINT]  [NEXT]  [PREV]  [EDIT/END]  ");
}

void
init_prnt (
 void)
{
	/*
	 * Heading and Page Format For Printer Output Display. 
	 */
	line_drawn = FALSE;
    fout = popen ("pformat", "w");
	if (!fout)
        file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n", lpno);
	fprintf (fout,".PL60\n");
	fprintf (fout,".9\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".PI10\n");
	fprintf (fout,".B1\n");

	fprintf (fout,".ECHART OF ACCOUNTS - %-2.2s  %-40.40s\n",
					comm_rec.co_no,comm_rec.co_name);
	fprintf (fout,".B1\n");

	fprintf (fout, ".R=================================");
	fprintf (fout, "============================================\n");
	fprintf (fout, "=================================\n");

	fprintf (fout, "=================================");
	fprintf (fout, "===============================================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "|         ACCOUNT NUMBER         ");
	fprintf (fout, "|                  NARRATIVE                   ");
	fprintf (fout, "|             CLASS             | CURRENCY |\n");

	fprintf (fout, "|--------------------------------");
	fprintf (fout, "|----------------------------------------------");
	fprintf (fout, "|-------------------------------|----------|\n");
}

void
ProcessFile (void)
{
	int first_time = 1,	
        len, 
        printed = 0;

	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.glmr_class [0], local_rec.loc_type);
	SetEndAccount ();
	SetStartAccount ();
	
	cc = find_rec (glmr, &glmrRec, GTEQ , "r");
	len = strlen (GL_GetBit (GV_cur_level));

	while (!cc && 
           !strcmp (glmrRec.co_no,comm_rec.co_no) &&
           (strcmp (glmrRec.acc_no, PV_start_acc) >= 0) && 
           (strcmp (glmrRec.acc_no, PV_end_acc) <= 0) &&
           (glmrRec.glmr_class [0][0] == local_rec.loc_type [0]))
	{
        if ((local_rec.loc_type [0] == 'F') ||
            (glmrRec.glmr_class [2][0] == 'C'))
		{
			if (first_time)	
                strcpy (prev_acc,glmrRec.acc_no);

			PrintAccounts (local_rec.loc_type [0] == 'N' ? TRUE : FALSE);
			PrintLinks (glmrRec.hhmr_hash, 0L, 0L);
			printed = 1;
			first_time = 0;

			if (local_rec.loc_type [0] == 'N')
				DrawLineStuff ();
		}

		cc = find_rec (glmr, &glmrRec, NEXT , "r");
	}

	if (printed && !PV_print)
    {
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGG");
    }
	else
    {
		if (!PV_print)
		{
			for (len = 0; len < 7; len++)
            {
                Dsp_saverec ("        ");
            }
			Dsp_saverec ("No Accounts Found in Entered Range for security level.");
		}
    }

	if (!PV_print)
	{
		print_ok = TRUE;
		Dsp_srch ();
		Dsp_close ();
		print_ok = FALSE;
	}
}

void
PrintLinks (
	long	hhmrHash, 
	long 	parentHhmrHash, 
	long 	childHhmrHash)
{
	int	print_stat = FALSE;

	if (local_rec.loc_type [0] != 'N')
        return;

	gllnRec.parent_hash = hhmrHash;
	gllnRec.child_hash 	= 0L;

    cc = find_rec (glln, &gllnRec, GTEQ,"r");
	while (!cc && (gllnRec.parent_hash == hhmrHash))
	{
		if (!print_stat)
		{
			if (!link_level)
				DrawLineStuff ();

			link_level++;
			print_stat = TRUE;
		}

		glmrRec.hhmr_hash	=	gllnRec.child_hash;
        cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
		if (cc)
            file_err (cc, glmr2, "FIND_HASH");

		PrintAccounts (TRUE);
		PrintLinks 
		(
			glmrRec.hhmr_hash, 
			gllnRec.parent_hash, 
			gllnRec.child_hash
		);
		cc = find_rec (glln, &gllnRec, NEXT,"r");
	}
	if (print_stat)
        link_level--;

	/*
	 * If recursing reposition file pointers after read.
	 */
	if (childHhmrHash && parentHhmrHash)
	{
		gllnRec.parent_hash	= parentHhmrHash;
		gllnRec.child_hash	= childHhmrHash;

        cc = find_rec (glln, &gllnRec, GTEQ,"r");
		if (cc)
			file_err (cc, glln, "FIND_REC");
	}
}

void
PrintAccounts (
 int l_flag)
{
	char	acc_desc [4],
			accno_arr [40],
			accdesc_arr [200],
			acccurr_arr [11],
			env_line [250],
			form_acc [FORM_LEN + 1];
	int     len;

	strcpy (form_acc, glmrRec.acc_no);
	GL_FormAccNo (form_acc, glmrRec.acc_no, 0);
	/*
	 * Only display up to specified level.
	 */
	if ((!l_flag && GV_cur_level > local_rec.loc_level) ||
        (l_flag && link_level >= local_rec.loc_level))
    {		
        return;
    }


	if (!l_flag)
	{
		len = strlen (GL_GetBit (GV_cur_level));
		strcpy (prev_acc,glmrRec.acc_no);
	}

	if (ln_num >= InternalPageSize)
    {
        ln_num = ln_num % InternalPageSize;
    }

	sprintf (acc_desc, 
             "%c%c%c",
             glmrRec.glmr_class [0][0],
             glmrRec.glmr_class [1][0],
             glmrRec.glmr_class [2][0]);
	sprintf (acccurr_arr, "   %3.3s    ", glmrRec.curr_code);

	if (!l_flag)
	{
		sprintf (accno_arr,
                 "%-*.*s%s",
                 (GV_cur_level - 1) * 2, 
                 (GV_cur_level - 1) * 2, 
                 " ", 
                 form_acc);
		sprintf (accdesc_arr, 
                 "%-*.*s%s",
                 (GV_cur_level -1) * 2, 
                 (GV_cur_level -1) * 2, 
                 " ", 
                 glmrRec.desc);
	}
	else
	{
		len = (link_level > 6) ? 6 : link_level * 2;
		sprintf (accno_arr, "%-*.*s%s",	len, len, " ", form_acc);
		sprintf (accdesc_arr, "%-*.*s%s",	len, len, " ", glmrRec.desc);
	}

	if (!PV_print)
	{
		sprintf (env_line,
                 "%-30.30s^E%-55.55s^E %3.3s - %-21.21s^E%10.10s",
                 accno_arr, 
                 accdesc_arr, 
                 acc_desc, 
                 GetCDesc (acc_desc), 
                 acccurr_arr);
		Dsp_saverec (env_line);
	}
	else
	{
		sprintf (env_line,
                 "%-32.32s| %-45.45s| %3.3s - %-24.24s|%10.10s",
                 accno_arr, 
                 accdesc_arr, 
                 acc_desc, 
                 GetCDesc (acc_desc), 
                 acccurr_arr);
		fprintf (fout, "|%s|\n", env_line);
	}

	line_drawn = FALSE;
	ln_num++;
}

/*
 * Open data base files.
 */
void 
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (glmr2, glmr);
	abc_alias (glln2, glln);
	OpenGlmr (); abc_selfield (glmr, "glmr_id_no2");
	OpenGlln ();
	
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec (glln2, glln_list, GLLN_NO_FIELDS, "glln_id_no2");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (glln2);
	abc_fclose (glmr2);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	if (PV_print)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
	else
	{
		clear ();
		snorm ();
		rset_tty ();
		crsr_on ();
	}
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	if (LCHECK ("level_no"))
	{
		if (dflt_used)
		    local_rec.loc_level = GV_max_level;
		
		DSP_FLD ("level_no");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("acc_no")) 
	{
		if (dflt_used)
		    strcpy (local_rec.loc_start, GL_GetDfltSfaccCode ());

		if (SRCH_KEY)
		{
			abc_selfield (glmr, "glmr_id_no");
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());
			
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			abc_selfield (glmr, "glmr_id_no2");
		}
		return (SetStartAccount ());
	}

	if (LCHECK ("end_acc")) 
	{
		if (dflt_used)
		{
		   if (!strcmp (glmrRec.acc_no, GL_GetDfltSaccCode ()))
				strcpy (local_rec.loc_end, GL_GetDfltEfaccCode ());
		   else
				strcpy (local_rec.loc_end, local_rec.loc_start);
		}

		if (SRCH_KEY)
		{
			abc_selfield (glmr, "glmr_id_no");
			if (strlen (temp_str) == 0)
				strcpy (temp_str, GL_GetfUserCode ());
			
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			abc_selfield (glmr, "glmr_id_no2");
		}
		return (SetEndAccount ());
	}
	return (EXIT_SUCCESS);
}

int	
SetStartAccount (void)
{
    if (GL_FormAccNo (local_rec.loc_start, glmrRec.acc_no, 0))
        return (EXIT_FAILURE);

	strcpy (PV_start_acc, glmrRec.acc_no);
	return (EXIT_SUCCESS);
}

int
SetEndAccount (void)
{
	if (GL_FormAccNo (local_rec.loc_end, PV_end_acc, 0))
        return (EXIT_FAILURE);
		
	return (EXIT_SUCCESS);
}

/*
 * Display Screen Heading 
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		clear ();

		if (scn != cur_screen)
            scn_set (scn);

		rv_pr (ML (mlGlMess012), 40,0,1);

		box (0,1, 132, 2);

		line_at (21,0,132);

		strcpy (err_str, ML (mlStdMess038));
		print_at (22,0, err_str,
						comm_rec.co_no,
						comm_rec.co_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		scn_display (scn);
	}

    return (EXIT_SUCCESS);
}


void
DrawLineStuff (void)
{
	if (!line_drawn)
	{
		if (!PV_print)
        {
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
        }
		else
		{
			fprintf (fout, "|--------------------------------");
			fprintf (fout, "|----------------------------------------------");
			fprintf (fout, "|-------------------------------|\n");
		}
		ln_num++;
		line_drawn = TRUE;
	}
}

void	
RunChartHots (void)
{
	restart = FALSE;
	DisplayChartHots ();
	if (run_hotkeys (chart_keys, null_func, null_func))
        restart = TRUE;

	return;
}

void	
DisplayChartHots (void)
{
	line_at (21,0,132);
	disp_hotkeys (21, 0, 132, chart_keys);
}

char*
GetCDesc (
 char *class_code)
{
	for (class_ptr = class_tab; class_ptr->class_code; class_ptr++)
    {
		if (!strcmp (class_ptr->class_code, class_code))
            return (class_ptr->class_desc);
    }
	return (" ?????????? ");
}

void
psl_print (void)
{
	if (!print_ok)
        return;

	line_at (21,0,132);
	lpno = get_lpno (0);
	rv_pr (ML (mlStdMess035), 22, 5, 1);

	PV_print = TRUE;
	init_prnt ();
	ProcessFile ();
	fprintf (fout, ".EOF\n");
	pclose (fout);
	PV_print = FALSE;
}
