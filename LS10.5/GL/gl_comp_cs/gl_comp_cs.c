/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_comp_cs.c,v 5.4 2002/07/08 04:06:10 scott Exp $
|  Program Name  : (gl_comp_cs.c)                                  
|  Program Desc  : (General Ledger Consolidate Company to Company)
|---------------------------------------------------------------------|
|  Author        : Andy Yuen       | Date Written  : 19/03/96         |
|---------------------------------------------------------------------|
| $Log: gl_comp_cs.c,v $
| Revision 5.4  2002/07/08 04:06:10  scott
| S/C 004084 - F4 does not show search window at Destination Company Field but will display a warning message 'Company Not Found' instead
|
| Revision 5.3  2001/08/09 09:13:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:13  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:42  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_comp_cs.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_comp_cs/gl_comp_cs.c,v 5.4 2002/07/08 04:06:10 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <tabdisp.h>
#include <GlUtils.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glshRecord	glsh_rec;
struct glsdRecord	glsd_rec;

char	*comrTab	=	"comrTab";

/*
 *   Constants, defines and stuff   
 */
#define		WK_DEPTH	10
#define		MAXLEVEL	16

	char	*data  = "data",
			*gltr2 = "gltr2",
			*glbd2 = "glbd2",
			*glca2 = "glca2",
			*glln2 = "glln2",
			*glmr2 = "glmr2",
			*glmr3 = "glmr3",
			*glpd2 = "glpd2",
			*glsh2 = "glsh2",
			*glsd2 = "glsd2";

/*
 *   Local variables  
 */
extern		int	tab_max_page;
int			first_line = 0;

	GLMR_STRUCT	glmr3Rec;
	GLLN_STRUCT	glln2_rec;

static	int 	ConfirmFunc 	(int, KEY_TAB *);
static	int 	ConfirmAllFunc	(int, KEY_TAB *);
static	int 	RejectFunc 		(int, KEY_TAB *);
static	int 	RejectAllFunc 	(int, KEY_TAB *);
static	int 	EditFunc 		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	tab_keys  [] =
{
	{ " Tag ",		'A',		ConfirmFunc,
		"Confirm Consolidate of the current company"	},
	{ " Tag all ",	CTRL ('A'),	ConfirmAllFunc,
		"Confirm Consolidate of ALL company"				},
	{ " Cancel ",		'C',		RejectFunc,
		"Reject Consolidate of the current company"	},
	{ " Cancel all ",		CTRL ('C'),	RejectAllFunc,
		"Reject Consolidate of ALL company"				},
	{ " Edit ", 		'E',		EditFunc,
		"Edit the currency rate for consolidation"		},
	END_KEYS
};
#else
static	KEY_TAB	tab_keys  [] =
{
	{ "[T]ag",		'T',		ConfirmFunc,
		"Confirm Consolidate of the current company"	},
	{ "[A]ll Tag",	'A',	ConfirmAllFunc,
		"Confirm Consolidate of ALL company"				},
	{ "[U]ntag",		'U',		RejectFunc,
		"Reject Consolidate of the current company"	},
	{ "[C]ancel All",	'C',	RejectAllFunc,
		"Reject Consolidate of ALL company"				},
	{ "[E]dit", 		'E',		EditFunc,
		"Edit the currency rate for consolidation"		},
	END_KEYS
};
#endif

/*
 * Local & Screen Structures.
 */
struct
{
	char	dst_co		  [3];
	char	dst_name	  [41];
	char	dst_curr	  [4];
	double	dst_rate;
	char	src_co		  [3];
	char	currDesc	  [41];
	double	rate;
	char 	dummy		  [10];
} local_rec;

extern	int	TruePosition;
static struct var vars  [] =
{
	{1, LIN, "dst_co", 3, 2, CHARTYPE, 
		"AA", "          ",
		" ", "", "Destination Company          ", "Enter a company number",
		YES, NO, JUSTRIGHT, "1", "99", local_rec.dst_co},
	{1, LIN, "dst_name", 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Destination Company Name     ", "Enter the name of you Consolidated company",
		NA, NO, JUSTLEFT, "", "", comr_rec.co_name},
	{1, LIN, "dst_curr",   5, 2, CHARTYPE, 
		"UUU", "          ",
		" ", "   ", "Currency                     ", "Enter the currency of you Consolidated company",
		NA, NO, JUSTLEFT, "", "", comr_rec.base_curr},

	{2, LIN, "src_co", 3, 2, CHARTYPE, "AA", "          ",
		" ", "", "Source Company               ", " ",
		NE, NO, JUSTRIGHT, "1", "99",
		local_rec.src_co},
	{2, LIN, "curr",   4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Currency                     ", " ",
		NE, NO, JUSTLEFT, "", "",
		local_rec.currDesc},
	{2, LIN, "rate",   5, 2, DOUBLETYPE, "NNNN.NNNNNNNN", "          ",
		" ", "", "Exchange Rate                ", " ",
		YES, NO, JUSTLEFT, "", "",
		(char *)&local_rec.rate},

	{0, LIN, "", 0, 0, INTTYPE, "A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ",
		local_rec.dummy}
};

/*
 *   Local function prototypes  
 */
int  	GetNextCompany 		(void);
int  	heading 			(int);
int  	LoadSourceCp 		(void);
int  	spec_valid 			(int);
void 	CloseDB 			(void);
void 	ConsolidateCompany 	(char *, char *, double);
void 	ConsolidateGlpd 	(long, long, double);
void 	ConsolidateGltr 	(long, long, char *, double);
void 	Consolidate 		(void);
void 	CopyGlbd 			(void);
void 	CopyGlca 			(void);
void 	CopyGlln 			(void);
void 	CopyGlmr 			(void);
void 	CopyGlpd 			(long, long);
void 	CopyGlsd 			(long, long);
void 	CopyGlsh 			(void);
void 	MakeGlln2 			(void);
void 	OpenDB 				(void);
void 	Process 			(void);
void 	shutdown_prog 		(void);
void 	SrchComr 			(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int  argc,
 char *argv  [])
{
	tab_max_page = 1000;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*
	 * Setup required parameters.
	 */
	init_scr ();
	set_tty ();

	set_masks ();		
	init_vars (1);	

	OpenDB ();

	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
            continue;

		/*
		 * Edit screen 1 linear input.
		 */
		heading (1);
		edit (1);
		if (restart)
            continue;
        
		break;
	}	

	if (prog_exit == 0)
	{
		if (LoadSourceCp ())
		{
			if (!tab_scan (comrTab))
			{
				/*
				 * If user select any company, Copy chart of account
				 */
				while (GetNextCompany ())
                    Process ();

				/*
				 * After coping chart of account, Consolidate
				 */
				Consolidate ();
			}
		}
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (glbd2, glbd);
	abc_alias (glca2, glca);
	abc_alias (glln2, glln);
	abc_alias (glmr2, glmr);
	abc_alias (glpd2, glpd);
	abc_alias (glsh2, glsh);
	abc_alias (glsd2, glsd);

	abc_alias (glmr3, glmr);	/* Only for glln Copy */

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	abc_alias (gltr2, gltr);

	OpenGltr ();
	OpenGlbd ();
	OpenGlca ();
	OpenGlln ();
	OpenGlmr ();
	OpenGlpd ();

	open_rec (glsh, glsh_list, GLSH_NO_FIELDS, "glsh_id_no");
	open_rec (glsd, glsd_list, GLSD_NO_FIELDS, "glsd_id_no");

	open_rec (glbd2, glbd_list, GLBD_NO_FIELDS, "glbd_id_no");
	open_rec (glca2, glca_list, GLCA_NO_FIELDS, "glca_id_no");
	open_rec (gltr2, gltr_list, GLTR_NO_FIELDS, "gltr_id_no");
	open_rec (glln2, glln_list, GLLN_NO_FIELDS, "glln_id_no");
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (glpd2, glpd_list, GLPD_NO_FIELDS, "glpd_id_no");
	open_rec (glsh2, glsh_list, GLSH_NO_FIELDS, "glsh_id_no");
	open_rec (glsd2, glsd_list, GLSD_NO_FIELDS, "glsd_id_no");

	open_rec (glmr3, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (
 void)
{
	GL_Close ();
	abc_fclose (glsh);
	abc_fclose (glsd);
	abc_fclose (glbd2);
	abc_fclose (glca2);
	abc_fclose (glln2);
	abc_fclose (glmr2);
	abc_fclose (glpd2);
	abc_fclose (glsh2);
	abc_fclose (glsd2);

	abc_fclose (glmr3);
	abc_fclose (comr);

	abc_dbclose (data);
}

/*
 * Standard Screen Heading Routine
 */
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
            scn_set (scn);
        
		clear ();
		rv_pr (ML (mlGlMess065),23 ,0 ,1);

		line_at (1,0,80);

		box (0, 2, 79, 3);

		line_at (20,0,80);
		
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		scn_display (scn);
	}

    return (EXIT_SUCCESS);
}
int
spec_valid (
 int field)
{
	/*
	 * Validate Account Number.
	 */
	if (LCHECK ("dst_co"))
	{
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (comr_rec.co_no, local_rec.dst_co);

        cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!comr_rec.consolidate)
		{
			print_mess (ML (mlGlMess135));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("dst_name");
		DSP_FLD ("dst_curr");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dst_curr"))
	{
		if (!strcmp (comr_rec.base_curr, "   "))
		{
			print_mess (ML (mlGlMess136));
			sleep (sleepTime);
			clear_mess ();
			restart = TRUE;
			return (EXIT_SUCCESS);
		}
		if (FindPocr (comm_rec.co_no, comr_rec.base_curr, "r") < 0)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.dst_curr, pocrRec.code);
		local_rec.dst_rate = pocrRec.ex1_factor;
		if (local_rec.dst_rate == 0)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("dst_curr");
			
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	CopyGlca ();
	CopyGlbd ();
	CopyGlsh ();	/* Also copies glsd */
	CopyGlmr ();	/* Also copies glpd */
	CopyGlln ();
}

void
CopyGlca (
 void)
{
	strcpy (glcaRec.co_no, local_rec.src_co);
	glcaRec.level_no 	= 0;
	glcaRec.acc_no 		= 0L;
    cc = find_rec (glca, &glcaRec, GTEQ, "r");
	while (!cc && !strcmp (glcaRec.co_no, local_rec.src_co))
	{
		/*
		 * Only add a new glca if one doesn't already exist.
		 */
		strcpy (glcaRec.co_no, local_rec.dst_co);
		
        cc = find_rec (glca2, &glcaRec, EQUAL, "r");
		if (cc)
		{
			cc = abc_add (glca2, &glcaRec);
			if (cc)
                file_err (cc, glca2, "DBADD");
		}

		strcpy (glcaRec.co_no, local_rec.src_co);
		cc = find_rec (glca, &glcaRec, NEXT, "r");
	}
}

void
CopyGlbd (void)
{
	strcpy (glbdRec.co_no, local_rec.src_co);
	glbdRec.budg_no = 0;

    cc = find_rec (glbd, &glbdRec, GTEQ, "r");
	while (!cc && !strcmp (glbdRec.co_no, local_rec.src_co))
	{
		/*
		 * Only add a new glbd if one doesn't already exist.
		 */
		strcpy (glbdRec.co_no, local_rec.dst_co);
		cc = find_rec (glbd2, &glbdRec, EQUAL, "r");
		if (cc)
		{
			cc = abc_add (glbd2, &glbdRec);
			if (cc)
                file_err (cc, glbd2, "DBADD");
		}
		strcpy (glbdRec.co_no, local_rec.src_co);
		cc = find_rec (glbd, &glbdRec, NEXT, "r");
	}
}

void
CopyGlsh (void)
{
	long    sHhmrHash,
            dHhmrHash;

	strcpy (glsh_rec.co_no, local_rec.src_co);
	strcpy (glsh_rec.code, "      ");

    cc = find_rec (glsh, &glsh_rec, GTEQ, "r");
	while (!cc && !strcmp (glsh_rec.co_no, local_rec.src_co))
	{
		sHhmrHash = glsh_rec.hhsh_hash;

		/*
		 * Only add a new glsh if one doesn't already exist. 
		 */
		strcpy (glsh_rec.co_no, local_rec.dst_co);

        cc = find_rec (glsh2, &glsh_rec, EQUAL, "r");
		if (cc)
		{
			cc = abc_add (glsh2, &glsh_rec);
			if (cc)
                file_err (cc, glsh2, "DBADD");

			cc = find_rec ("glsh2", &glsh_rec, EQUAL, "r");
			if (cc)
                file_err (cc, glsh2, "DBFIND");
		}
		dHhmrHash = glsh_rec.hhsh_hash;

		CopyGlsd (sHhmrHash, dHhmrHash);

		strcpy (glsh_rec.co_no, local_rec.src_co);
		cc = find_rec (glsh, &glsh_rec, NEXT, "r");
	}
}

void
CopyGlsd (
	long	sHhmrHash, 
	long	dHhmrHash)
{
	glsd_rec.hhsh_hash = sHhmrHash;
	glsd_rec.prd_no    = 0;
    cc = find_rec (glsd, &glsd_rec, GTEQ, "r");
	while (!cc && (glsd_rec.hhsh_hash == sHhmrHash))
	{
		/*
		 * Only add a new glsd if one doesn't already exist.
		 */
		glsd_rec.hhsh_hash = dHhmrHash;
		cc = find_rec (glsd2, &glsd_rec, EQUAL, "r");
		if (cc)
		{
			cc = abc_add (glsd2, &glsd_rec);
			if (cc)
                file_err (cc, glsd2, "DBADD");
		}
		glsd_rec.hhsh_hash = sHhmrHash;
		cc = find_rec (glsd, &glsd_rec, NEXT, "r");
	}
}

void
CopyGlmr (
 void)
{
	long    sHhmrHash,
            dHhmrHash;

	strcpy (glmrRec.co_no, local_rec.src_co);
	strcpy (glmrRec.acc_no, "");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (! (cc || 
             strcmp (glmrRec.co_no, local_rec.src_co)))
	{
		sHhmrHash = glmrRec.hhmr_hash;

		/*
		 * Only add a new glmr if one doesn't already exist.    
		 */
		strcpy (glmrRec.co_no, local_rec.dst_co);
		cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
		if (cc)
		{
			cc = abc_add (glmr2, &glmrRec);
			if (cc)
                file_err (cc, glmr2, "DBADD");

			cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
			if (cc)
                file_err (cc, glmr2, "DBFIND");
		}

		dHhmrHash = glmrRec.hhmr_hash;
		CopyGlpd (sHhmrHash, dHhmrHash);

		strcpy (glmrRec.co_no, local_rec.src_co);
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

void
CopyGlpd (
	long	sHhmrHash, 
	long	dHhmrHash)
{
	glpdRec.hhmr_hash = sHhmrHash;
	glpdRec.budg_no = 0;
	glpdRec.year = 0;
	glpdRec.prd_no = 0;

    cc = find_rec (glpd, &glpdRec, GTEQ, "r");
	while (!cc && 
         (glpdRec.hhmr_hash == sHhmrHash))
	{
		/*
		 * Only add a new glpd if one
		 * doesn't already exist.    
		 * Otherwise, update existing
		 */
		glpdRec.hhmr_hash = dHhmrHash;
		cc = find_rec (glpd2, &glpdRec, EQUAL, "r");

		glpdRec.balance = 0.00;
		glpdRec.fx_balance = 0.00;

		if (cc)
		{
			cc = abc_add (glpd2, &glpdRec);
			if (cc)
            {
                file_err (cc, glpd2, "DBADD");
            }
		}
		else
		{
			cc = abc_update (glpd2, &glpdRec);
			if (cc)
            {
                file_err (cc, glpd2, "DBUPDATE");
            }
		}

		glpdRec.hhmr_hash = sHhmrHash;
		cc = find_rec (glpd, &glpdRec, NEXT, "r");
	}
}

void
CopyGlln (
 void)
{
	strcpy (glmrRec.co_no, local_rec.src_co);
	strcpy (glmrRec.acc_no, "");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (! (cc || 
             strcpy (glmrRec.co_no, local_rec.src_co)))
	{
		gllnRec.parent_hash = glmrRec.hhmr_hash;
		gllnRec.child_hash = 0L;

        cc = find_rec (glln, &gllnRec, GTEQ, "r");
		while (!cc && 
              (gllnRec.parent_hash == glmrRec.hhmr_hash))
		{
			glmr3Rec.hhmr_hash = gllnRec.child_hash;

            cc = find_rec (glmr3, &glmr3Rec, GTEQ, "r");
			if (!cc && 
              (glmr3Rec.hhmr_hash == gllnRec.child_hash))
            {
                MakeGlln2 ();
            }
			cc = find_rec (glln, &gllnRec, NEXT, "r");
		}

		strcpy (glmrRec.co_no, local_rec.src_co);
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

void
MakeGlln2 (
 void)
{
	strcpy (glmrRec.co_no, local_rec.dst_co);
	cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
	glln2_rec.parent_hash = glmrRec.hhmr_hash;

	strcpy (glmr3Rec.co_no, local_rec.dst_co);
	cc = find_rec (glmr2, &glmr3Rec, EQUAL, "r");
	glln2_rec.child_hash = glmr3Rec.hhmr_hash;

	/*
	 * Only add a new glmr if one doesn't already exist.  
	 */
	cc = find_rec (glln2, &glln2_rec, EQUAL, "r");
	if (cc)
	{
		cc = abc_add (glln2, &glln2_rec);
		if (cc)
        {
            file_err (cc, glln2, "DBADD");
        }
	}
}

int
LoadSourceCp (
 void)
{
	int	something = FALSE;
	heading (1);
	tab_open (comrTab, tab_keys, 6, 10, WK_DEPTH, FALSE);
	tab_add (comrTab, "#%-3.3s     %14.14s     %-8.8s     %14.14s ",
					"C/R", "Company Number", "Currency", "Exchange Rate");

	strcpy (comr_rec.co_no, "");

    cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc)
	{
		if (!strcmp (comr_rec.co_no, local_rec.dst_co) || comr_rec.consolidate)
		{
			cc = find_rec (comr, &comr_rec, NEXT, "r");
			continue;
		}

		cc = FindPocr (comr_rec.co_no, comr_rec.base_curr, "r");
		if (cc)
		{
			cc = find_rec (comr, &comr_rec, NEXT, "r");
			continue;
		}

		tab_add (comrTab, 
                  "%-3.3s     %2s                 %-8.8s     %14.8lf ",
                  "C", 							/* 0 */
                  comr_rec.co_no,
                  pocrRec.code,
                  pocrRec.ex1_factor / local_rec.dst_rate);	
        
        something = TRUE;
		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}

	return (something);
}

int
ConfirmFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	rec_buffer  [256];
	int		old_line;

	old_line = tab_tline (comrTab);

    cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer  [0] = 'A';
		tab_update (comrTab, "%s", rec_buffer);
		cc = tab_get (comrTab, rec_buffer, NEXT, 0);
		if (cc)
        {
            cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
        }
	}
	old_line = tab_tline (comrTab);
	if ( (old_line % WK_DEPTH) == 0)
    {
        load_page (comrTab, FALSE);
    }
	redraw_page (comrTab, TRUE);

    return (EXIT_SUCCESS);
}

int
ConfirmAllFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	rec_buffer  [256];
	int		old_line;

	old_line = tab_tline (comrTab);

    cc = tab_get (comrTab, rec_buffer, EQUAL, first_line);
	while (!cc)
	{
		rec_buffer  [0] = 'A';
		tab_update (comrTab, "%s", rec_buffer);
		cc = tab_get (comrTab, rec_buffer, NEXT, 0);
	}
	cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	load_page (comrTab, FALSE);
	redraw_page (comrTab, TRUE);

    return (EXIT_SUCCESS);
}

int
RejectFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	rec_buffer  [256];
	int		old_line;

	old_line = tab_tline (comrTab);

    cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	if (!cc)
	{
		rec_buffer  [0] = 'R';
		tab_update (comrTab, "%s", rec_buffer);
		cc = tab_get (comrTab, rec_buffer, NEXT, 0);
		if (cc)
			cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	}
	old_line = tab_tline (comrTab);
	if ( (old_line % WK_DEPTH) == 0)
    {
        load_page (comrTab, FALSE);
    }
	redraw_page (comrTab, TRUE);

    return (EXIT_SUCCESS);
}

int
RejectAllFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	rec_buffer  [256];
	int		old_line;

	old_line = tab_tline (comrTab);
	cc = tab_get (comrTab, rec_buffer, EQUAL, first_line);
	while (!cc)
	{
		rec_buffer  [0] = 'R';
		tab_update (comrTab, "%s", rec_buffer);
		cc = tab_get (comrTab, rec_buffer, NEXT, 0);
	}
	cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	load_page (comrTab, FALSE);
	redraw_page (comrTab, TRUE);

    return (EXIT_SUCCESS);
}

int
EditFunc (
 int iUnused, 
 KEY_TAB *psUnused)
{
	char	rec_buffer  [256];
	int		old_line;

    old_line = tab_tline (comrTab);

    cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);
	sprintf (comr_rec.co_no, "%2.2s", rec_buffer + 8);

    cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);

	cc = FindPocr (comr_rec.co_no, comr_rec.base_curr, "r");
	if (cc)
		return (EXIT_FAILURE);
	
    strcpy (local_rec.src_co, comr_rec.co_no);
	strcpy (local_rec.currDesc, pocrRec.description);
	local_rec.rate = atof (rec_buffer + 40);
	heading (2);
	edit (2);
	sprintf (&rec_buffer  [40], "%14.8f", local_rec.rate);
	tab_update (comrTab, "%s", rec_buffer);

    cc = tab_get (comrTab, rec_buffer, NEXT, 0);
	cc = tab_get (comrTab, rec_buffer, EQUAL, old_line);

    heading (1);
	redraw_table (comrTab);
	redraw_page (comrTab, TRUE);

    return (EXIT_SUCCESS);
}

int
GetNextCompany (void)
{
	static int	currLine = 0;
	static int	moreLines = TRUE;
           char rec_buffer  [256];

	if (tab_get (comrTab, rec_buffer, EQUAL, currLine))
	{
		moreLines = FALSE;
		return (FALSE);
	}

	while (moreLines) 
	{
		currLine++;

		/*
		 * Return TRUE for Accepted companies
		 */
		if (rec_buffer  [0] == 'A')
		{
			sprintf (local_rec.src_co, "%2.2s", rec_buffer + 8);
			return (TRUE);
		}

		if (tab_get (comrTab, rec_buffer, NEXT, 0))
            moreLines = FALSE;
	}
	return (FALSE);
}

/*
 * Main Consolidation processing routine.
 */
void
Consolidate (void)
{
	char	rec_buffer  [256];
	char	co_no  [3];
	char	currCode  [4];
	double	exchRate;

	if (tab_get (comrTab, rec_buffer, FIRST, 0))
        return;

	while (TRUE) 
	{
		if (rec_buffer  [0] != 'A')
		{
			if (tab_get (comrTab, rec_buffer, NEXT, 0))
                break;

			continue;
		}

		sprintf (co_no,     "%2.2s", rec_buffer + 8);
		sprintf (currCode, "%3.3s", rec_buffer + 27);
		exchRate = atof (rec_buffer + 40);
		ConsolidateCompany (co_no, currCode, exchRate);

		if (tab_get (comrTab, rec_buffer, NEXT, 0))
            break;

		continue;
	}
}

/*
 * Consolidate General Ledger master file information. 
 */
void
ConsolidateCompany (
	char 	*coNo,
	char 	*currencyCode,
	double 	exchRate)
{
	long	sHhmrHash,
			dHhmrHash;

	strcpy (glmrRec.co_no, coNo);
	strcpy (glmrRec.acc_no, "");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc && !strcmp (glmrRec.co_no, coNo))
	{
		sHhmrHash = glmrRec.hhmr_hash;
		strcpy (glmrRec.co_no, local_rec.dst_co);

		cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
		if (cc)
            file_err (cc, glmr2, "DBFIND");

		dHhmrHash = glmrRec.hhmr_hash;
		ConsolidateGlpd (sHhmrHash, dHhmrHash, exchRate);
		ConsolidateGltr (sHhmrHash, dHhmrHash, currencyCode, exchRate);

		strcpy (glmrRec.co_no, coNo);
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

/*
 * Consolidate General Ledger Period Values.
 */
void
ConsolidateGlpd (
	long	sHhmrHash,
	long	dHhmrHash,   
	double	exchRate)
{
	double	fgnBalance	=	0.00,
			locBalance	=	0.00;

	glpdRec.hhmr_hash 	= sHhmrHash;
	glpdRec.budg_no 	= 0;
	glpdRec.year 		= 0;
	glpdRec.prd_no 		= 0;
    cc = find_rec (glpd, &glpdRec, GTEQ, "r");
	while (!cc && glpdRec.hhmr_hash == sHhmrHash)
	{
		glpdRec.hhmr_hash 	= dHhmrHash;
		fgnBalance 			= glpdRec.fx_balance;
		locBalance    		= glpdRec.balance / exchRate;

        cc = find_rec (glpd2, &glpdRec, COMPARISON, "u");
		if (!cc)
		{
			glpdRec.fx_balance 	+= fgnBalance;
			glpdRec.balance 	+= locBalance;
			cc = abc_update (glpd2, &glpdRec);
			if (cc)
                file_err (cc, glpd, "DBUPDATE");
		}
		glpdRec.hhmr_hash = sHhmrHash;
		cc = find_rec (glpd, &glpdRec, NEXT, "r");
	}
}

/*
 * Consolidate General Ledger Transaction file.
 */
void
ConsolidateGltr (
	long	sHhmrHash,
	long	dHhmrHash,
	char	*currCode,
	double	exchRate)
{
	gltrRec.hhmr_hash = sHhmrHash;
	gltrRec.tran_date = 0;

    cc = find_rec (gltr, &gltrRec, GTEQ, "r");
	while (!cc && (gltrRec.hhmr_hash == sHhmrHash))
	{
		strcpy (gltrRec.currency, currCode);
		gltrRec.amt_origin 	= gltrRec.amount;
		gltrRec.exch_rate  	= exchRate;
		gltrRec.amount 		= gltrRec.amt_origin / exchRate;
		gltrRec.hhmr_hash	= dHhmrHash;
		cc = abc_add (gltr2, &gltrRec);
		if (cc)
			file_err (cc, gltr2, "DBADD");

		gltrRec.hhmr_hash = sHhmrHash;
		cc = find_rec (gltr, &gltrRec, NEXT, "r");
	}
}

/*  [ end of file ] */


void 
SrchComr (
 char *key_val)
{
	char	str_s [60];

	_work_open (2,0,40);
	save_rec ("#No", "#Company Name                           Curr");

	sprintf (comr_rec.co_no, "%-2.2s", key_val);

    cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc && !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
	{
		if (comr_rec.consolidate == FALSE)
		{
			cc = find_rec (comr, &comr_rec, NEXT, "r");
			continue;	
		}
		sprintf (str_s,"%-40.40s %-3.3s",comr_rec.co_name, comr_rec.base_curr);
		cc = save_rec (comr_rec.co_no, str_s);
		if (cc)
            break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
        return;

	sprintf (local_rec.dst_co, "%-2.2s", temp_str);
	return;
}
