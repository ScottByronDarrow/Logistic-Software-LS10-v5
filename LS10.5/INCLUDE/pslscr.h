/*=======================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=======================================================================|
| Program Name : ( pslscr.h )                                           |
| Program Desc : ( Definitions for scrgen.c                             |
|                (                                                )     |
|-----------------------------------------------------------------------|
| Authors : Scott Darrow & Roger Gibbison.                              |
| Date Written : 10/05/86                                               |
|-----------------------------------------------------------------------|
| Date Modified : (07/09/88) Modified by : Roger Gibbison.              |
| Date Modified : (23/12/91) Modified by : Campbell Mander.             |
|               : (10.06.94) Modified by : Jonathan Chen                |
|               : (30.01.96) Modified by : Shane Wolstencroft           |
|                                                                       |
| Comments      : Fixed getkey                                          |
|               : Fixed Reverse being cleared on edit.                  |
|               : Modified for work files for tabular & new search      |
|               :                                                       |
| (23/12/91)    : Added cur_field                                       |
| (10.06.94)    : includes <std_decs.h>                                 |
| (05.05.95)    : added <ptypes.h>                                      |
| (30.01.96)    : added restart_ck & restart_msg                        |
|               : added pslw_asl                                        |
|                                                                       |
=======================================================================*/
#ifndef	PSLSCR_H
#define	PSLSCR_H

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>
#include	<math.h>

#include	<psizes.h>
#include	<ptypes.h>
#include	<std_decs.h>
#include	<defkeys.h>
#include	<dberr.h>
#include	<dbio.h>
#include	<tcap.h>
#include	<ttyctl.h>

#ifdef	CCMAIN
#	define	GLOBAL
#else
#	define	GLOBAL	extern
#endif

#ifndef	NO_SCRGEN
#include	<vars_def.h>
union _var_type {
	char	*cptr;
	int	*iptr;
	long	*lptr;
	float	*fptr;
	double	*dptr;
};

#define	SETUP_SCR(x)	setup_scr (x, MAXSTR, MAXSCNS, TABLINES, MAXLINES, \
					MAXWIDTH, \
					sizeof (x) / sizeof (struct var), \
					use_window)
#endif	/*NO_SCRGEN*/

#define	BELL	7       /* Define Bell which is standard.             */
#define EOI 	FN16    /* End Of Input Key , See defkeys.h .         */
#define ESC 	27      /* Define Esc Key.                            */
#define US	'_'     /* Define '_' As User Input Prompt Mask.      */
#define RESTART  FN1    /* Define F1 As Restart Program.              */
#define RECALL   FN2    /* Define F2 As Recall Last Value.            */
#define REDRAW   FN3    /* Define F3 As Redraw Screen.                */
#define SEARCH   FN4    /* Define F4 As Search for Match Values.      */
#define HELP1    FN6    /* Define F6 As Help for program help.        */
#define FN_HELP  FN7    /* Define F7 As Help for dunction keys.       */
#define EXIT_OUT FN8    /* Define F8 As Exit Program to another,      */
#define	NSEARCH  FN9
#define	ASEARCH  FN10
#define	OSEARCH  FN11

#define BS	 '\b'		/* Define Backspace						*/
#define ENDINPUT '\r'	/* Define End of input i.e. Return.		*/
#define TRUE	 1		/* Define True as 1.					*/
#define FALSE	 0		/* Define False as 0.					*/
#define YES	TRUE		/* Define Yes as 1.						*/
#define NO	FALSE		/* Define No as 0.						*/

#define EDIT        0       /* EDIT - If program is in edit mode .        */
#define ENTRY       1       /* ENTRY - If program is in entry mode.       */
#define DISPLAY     2       /* DISPLAY - If Program is in display Mode.   */
#define NA         -1       /* NA - Not Applicable for entry.             */
#define NE         -2       /* NE - No edit allowed on field.             */
#define NI         -3       /* NI - No input allowed on field.            */
#define ND         -4       /* ND - No display of field.                  */
#define JUSTLEFT    TRUE    /* JUSTLEFT - True if field is left justified.*/
#define JUSTRIGHT   FALSE   /* JUSTRIGHT- False if field is right   ""    */
#define TXT         2       /* TXT - Set to 2 if field is text.           */
#define TAB         1       /* TAB - Set to 1 if field is tabular.        */
#define LIN         0       /* LIN - Set to 0 if field is Linear.         */
#define FIELD	vars [field]  /* Saves on typing in scrgen.c .              */
#define VAR_PTR var_ptr [field]
/*-----------------------------------------------------------------------
| The following 7 lines are macros to save on typing and clean up code. |
-----------------------------------------------------------------------*/
#define DPP(x)  		n_dec ((double) x, 5)	/* Decimal Point Precision 	*/
#define LCHECK(x)  		(!strcmp(FIELD.label,x))
#define LNCHECK(x,y)  	(!strncmp(FIELD.label,x,y))
#define FLD(x)  		vars [label (x)].required
#define SCN_ROW(x)  	vars [label (x)].row
#define SCN_COL(x)  	vars [label (x)].col
#define DSP_FLD(x)  	display_field (label(x))
#define	SRCH_KEY		(last_char == SEARCH)
#define F_NOKEY(x)		(vars [x].required == NA || \
			  			 vars [x].required == NI || \
			  			 vars [x].required == ND)

#define F_HIDE(x)		(vars [x].required == ND)
#define F_NEED(x)		(vars [x].required == YES)

#ifndef MAXSTR              /*                                            */
#define MAXSTR    150       /* Maxlines for tabular == 10 screens.        */
#endif                      /*                                            */

#ifndef MAXSCNS             /*                                            */
#define MAXSCNS     5       /* Max number of screens Will become unused.  */
#endif                      /*                                            */

#ifndef	TABLINES            /*                                            */
#define TABLINES    10      /* Standard no lines for tabular screen.      */
#endif                      /*                                            */

#ifndef MAXLINES            /*                                            */
#define MAXLINES    50      /* Maxlines for tabular == 10 screens.        */
#endif                      /*                                            */

#ifndef MAXWIDTH            /*                                            */
#define MAXWIDTH   200      /* Max. width for 1 line tabular - make       */
#endif                      /* larger for 132 column tabular (try 150).   */

#ifndef MAX_MESG_LENGTH     /*                                            */
#define MAX_MESG_LENGTH   130 /* Max. width for 1 line tabular - make       */
#endif                      /* larger for 132 column tabular (try 150).   */

struct	tab_struc {
	char	*_desc;	/* description of screen - not required		*/
	int	_width;	/* total width of all masks for tabular screen	*/
	int	_scn;	/* number of scn ie vars [].scn			*/
	int	_row;	/* row to print at for edit_all			*/
	int	(* _actn)(void);/* function				*/
	int	_win;		/* TRUE if not using function		*/
};

/*-----------------------------------------------
| The following 2 macros are for the purpose	|
| of converting between fiscal periods and	|
| calendar months.				|
-----------------------------------------------*/
#define	fisc2mth(p,f)	(((p) + 11 + (f) % 12) + 1)
#define	mth2fisc(m,f)	((((m) + 11 - (f)) % 12) + 1)

/*
 *	Some brain-dead globals :
 */
extern char	temp_str [], err_str [];	/* dso_vars.c */
extern int	cc,							/* popular global for db return */
			dflt_used,					/* Flag associated with default value */
			prog_exit,					/* used by apps to determine state */
			restart,					/* Flag associated with RESTART key */
			last_char,					/* the last char of input */
			max_work,					/* number of lines in a search file */
			search_key,					/* set to the search key pressed */
			search_ok;					/* non-zero if ok to search */

extern int	_win_func;

#ifdef CCMAIN
GLOBAL	struct	tab_struc tab_data [MAXSCNS];

GLOBAL	int	delta_edit;
GLOBAL	int	in_sub_edit = FALSE;
GLOBAL	char	temp_mask [MAXWIDTH],  /* Both gobal string space for use.    */
				prv_ntry [MAXWIDTH],   /* Used for recall key.                */
				restart_msg [132];     /* Restart Message					 */

/* Storage for tabular getval/putval info.		*/
GLOBAL	char	*lstore [MAXLINES * MAXSCNS];

GLOBAL	char	*arg [20];

GLOBAL	int	lcount [MAXSCNS + 1], /* Used to track no. of lines per table. */
        tab_index [MAXWIDTH], /* Index for editing tabular.                 */
        tab_row = 6,         /* These 2 variables define the upper left    */
        tab_col = 0,         /* hand corner of the TABULAR screen overlay. */
        input_row = 3,       /* These 2 variables define the upper left    */
        input_col = 30,      /* hand corner of the LINEAR screen overlay.  */
        dec_pt = 0,          /* Number of trailing digits for FLOAT&DOUBLE */
        row = 0,             /* Used for addressing current screen position*/
        col = 0,             /*                                            */
        max_prompt = 0,      /* Maximum prompt length for current screen.  */
        nbr_fields = 0,      /* Max Number fields for the current screen.  */
        scn_start = 0,       /* pointer to first field of current screen.  */
        prog_status,         /* Either ENTRY or FALSE.                     */
        edit_status = 0,     /* TRUE if in edit but doing entry.           */
        cur_screen = 0,      /* The number of the current screen in use.   */
        cur_field = 0,       /* The number of the current field.           */
        scn_page = 0,        /* The Screen page currently on.              */
        line_cnt = 0,        /* Number of lines in tabular screen.         */
        sr_lcnt = 0,         /* Used to Store No of lines in tab screen.   */
        scn_type = LIN,      /* Screen type (LIN | TAB | TXT).             */
        tabscn = 0,          /* Flag set on type of screen TRUE or FALSE.  */
        msg_line = 23,       /* The 'message' line - not really used.      */
        comment_line = 23,   /* The 'comment' line - used occasionally.    */
        error_line = 23,     /* The standard error line - under input.     */
        entry_exit = 0,	     /* These next 6 variables are used as flags.  */
        edit_exit = 0,       /* Internal to the skeleton.                  */
        end_input = 0,       /* True if End of Input Key pressed.          */
        skip_entry = 0,      /* Number of entries to skip - + or - value.  */
        skip_tab = 1,        /* Allow F16 to skip over tabular screens.    */
        new_rec = 0,         /* ???? No Use to scrgen.c                    */
        length,	             /* dummy for informix calls.                  */
        eoi_ok = 1,	     /* Flags if EOI is ok in entry fields,0=not ok*/
        init_ok = 1,	     /* Flags if initialising screen ok, 0 = not ok*/
                             /* Set to zero if using recall key.           */
        sr_err = 0,          /* Used as return code for save_rec routine.  */
        start_lin = 0,	     /* Start posn on cur screen of 1st entry field*/
                             /* (skips over field types of 'NA' etc).      */
	testchar = 0,	     /* Used with getkey .                         */
	restart_ck = 0,			 /* Is restart check allowed.				   */	
	pslw_asl = FALSE,        /* no redraw after a PSLW if pslw_asl = TRUE  */
	up_ok = 1,
	sleepTime = 2,
	SYS_LANG = 0,
	 
	Dsp_prn_ok = FALSE;


extern	char	*alpha;

#	ifndef	TXT_REQD
#	ifndef	NO_SCRGEN

/*ARGSUSED*/
int	txt_open(int y, int x, int dy, int dx, int max_y, char *hdg)
{ return(0);}

/*ARGSUSED*/
int	txt_display(int window, int line)
{ return(0);}

/*ARGSUSED*/
int	txt_close(int window, int clr_flg)
{ return(0);}

/*ARGSUSED*/
int	txt_pval(int window, char *text, int line)
{ return(0);}

/*ARGSUSED*/
char	*txt_gval(int window, int line)
{ return ((char *) 0);}

/*ARGSUSED*/
int	txt_scn_edit(struct var *tmp_vars, int tmp_scn_start)
{ return(0);}

/*ARGSUSED*/
int	txt_edit(int window)
{ return(0);}
#	endif	/* NO_SCRGEN */
#	endif	/* TXT_REQD */
#else

extern	char	*txt_gval(int, int);
GLOBAL	struct	tab_struc tab_data [];

GLOBAL	int	delta_edit;
GLOBAL	int	in_sub_edit;
GLOBAL	char	temp_mask [],  /* Both gobal string space for use.    */
				prv_ntry [],   /* Used for recall key.                */
				restart_msg []; /* Default restart message */

/* Storage for tabular getval/putval info.		*/
GLOBAL	char	*lstore [];

GLOBAL	char	*arg [];

GLOBAL	int	lcount [],    /* Used to track no. of lines per table.      */
        tab_index [], 	     /* Index for editing tabular.                 */
        tab_row,             /* These 2 variables define the upper left    */
        tab_col,             /* hand corner of the TABULAR screen overlay. */
        input_row,           /* These 2 variables define the upper left    */
        input_col,           /* hand corner of the LINEAR screen overlay.  */
        dec_pt,              /* Number of trailing digits for FLOAT&DOUBLE */
        row,                 /* Used for addressing current screen position*/
        col,                 /*                                            */
        max_prompt,          /* Maximum prompt length for current screen.  */
        nbr_fields,          /* Max Number fields for the current screen.  */
        scn_start,           /* pointer to first field of current screen.  */
        prog_status,         /* Either ENTRY or FALSE.                     */
        edit_status,         /* TRUE if in edit but doing entry.           */
        cur_screen,          /* The number of the current screen in use.   */
        cur_field,           /* The number of the current field.           */
        scn_page,            /* The Screen page currently on.              */
        line_cnt,            /* Number of lines in tabular screen.         */
        sr_lcnt,             /* Used to Store No of lines in tab screen.   */
	scn_type,            /* Screen type (LIN | TAB | TXT).             */
        tabscn,              /* Flag set on type of screen TRUE or FALSE.  */
        msg_line,            /* The 'message' line - not really used.      */
        comment_line,        /* The 'comment' line - used occasionally.    */
        error_line,          /* The standard error line - under input.     */
        entry_exit,	     /* These next 6 variables are used as flags.  */
        edit_exit,           /* Internal to the skeleton.                  */
        end_input,           /* True if End of Input Key pressed.          */
        skip_entry,          /* Number of entries to skip - + or - value.  */
        skip_tab,            /* Allow F16 to skip over tabular screens.    */
        new_rec,             /* ???? No Use to scrgen.c                    */
        length,	             /* dummy for informix calls.                  */
        eoi_ok,	             /* Flags if EOI is ok in entry fields,0=not ok*/
        init_ok,	     /* Flags if initialising screen ok, 0 = not ok*/
                             /* Set to zero if using recall key.           */
        sr_err,              /* Used as return code for save_rec routine.  */
        start_lin,	     /* Start posn on cur screen of 1st entry field*/
                             /* (skips over field types of 'NA' etc).      */
	testchar,	     /* Used with getkey .                         */
	restart_ck,    			 /* Is restart check allowed.				   */	
	pslw_asl,                /* no redraw after a PSLW if pslw_asl = TRUE  */
	up_ok,
	SYS_LANG,
	sleepTime,
	Dsp_prn_ok;

#endif

#endif	/*PSLSCR_H*/
