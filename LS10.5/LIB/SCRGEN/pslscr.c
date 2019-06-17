/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pslscr.c,v 5.7 2002/10/09 02:41:49 robert Exp $
|=====================================================================|
| Program Name : (pslscr.c    )                                       |
| Program Desc : (Screen Generator Code.                       )      |
|                (                                             )      |
|---------------------------------------------------------------------|
| Authors : Scott Darrow & Roger Gibbison.                            |
| Date Written : 02/03/86                                             |
|---------------------------------------------------------------------|
|  Date Modified : (07/09/88)      | Modified by  : Roger Gibbison.   |
|  Date Modified : (27/09/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (21/03/91)      | Modified  by : Scott Darrow.     |
|  Date Modified : (31/04/91)      | Modified  by : Scott Darrow.     |
|  Date Modified : (17/07/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (17/12/91)      | Modified  by : TvB.              |
|  Date Modified : (23/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (23/01/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (30/01/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (13/02/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (26/02/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (28/02/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (11/03/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (13/03/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (13/03/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (16/03/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (07/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (10/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (11/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (15/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (15/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (29/04/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (22/06/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (30/07/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (04/09/92)      | Modified  by : TvB/SBD           |
|  Date Modified : (02/11/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (23/11/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (20/01/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (11/02/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (02/03/93)      | Modified  by : Jonathan Chen     |
|  Date Modified : (04/03/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (13/05/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (26/05/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (18/02/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (30/01/96)      | Modified  by : Shane Wolstencroft|
|  Date Modified : (10/04/96)      | Modified  by : Shane Wolstencroft|
|  Date Modified : (17/07/96)      | Modified  by : Andy Yuen         |
|  Date Modified : (03/09/99)      | Modified  by : Eumir Que Camara  |
|                                                                     |
| $Log: pslscr.c,v $
| Revision 5.7  2002/10/09 02:41:49  robert
| SC 4305 - fixed date mask problem when date format is Y4MD
|
| Revision 5.6  2001/08/28 08:46:13  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.5  2001/08/20 23:07:14  scott
| Updated from scotts machine
|
| Revision 5.4  2001/08/09 09:31:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/06 22:47:19  scott
| RELEASE 5.0
|
| Revision 5.2  2001/07/25 00:57:01  scott
| Updated for 10.5
|
| Revision 5.0  2001/06/19 07:11:15  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:54:07  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 2.4  2001/03/05 08:37:43  scott
| Updated as above message wrong
|
| Revision 2.3  2000/12/11 07:53:55  scott
| Updated to fix small warning on IBM.
|
| Revision 2.2  2000/12/04 09:55:15  scott
| Updated to ensure that hidden and no entry fields check return from spec_valid
| otherwise bad data can enter system.
|
| Revision 2.1  2000/09/11 00:46:56  scott
| Updated for warnings.
|
| Revision 2.0  2000/07/15 07:34:18  gerry
| Forced Revision No. Start to 2.0 - Rel-15072000
|
| Revision 1.13  2000/06/16 01:16:35  scott
| Checked in after 5 weeks of using new environment SCN_INPUT_REV.
|

|  Comments      : Changed to use other terminals - rv_on () problem.  |
|                : New Version using var_ptr structure to handle      |
|                : initialising problems                              |
|                : Rog Gibbison 07/09/88                              |
|                : tabular screens now use work files.                |
|                : up_arrow on entry goes to previous field.          |
|                : added required type of ND - no display.            |
|                : (27/09/90) - General Update for Movement etc.      |
|                : (18/12/90) - Added Username-Shift FN Key.          |
|                : (21/03/91) - Added spec validation for up + dn arw |
|                : (31/04/91) - Added X_EALL + Y_EALL,.               |
|                : (17/07/91) - Changed txt_edit to txt_scn_edit.     |
|                : (17/12/91) - Print mask '_' after use_window       |
|                : (23/12/91) - Added cur_field.                      |
|                : (22/01/92) - Fixed problem found with new Ctrl-W   |
|                : (23/01/92) - Turn cursor on after use_window ()     |
|                : (30/01/92) - Standardise calling of win_function ().|
|                : (13/02/92) - Implement _edit to allow editing to   |
|                :              start at a particular line number.    |
|                : (26/02/92) - Enhance _edit to allow editing to     |
|                :        allow editting to start @ a field too!!.    |
|                : (28/02/92) - Implement NEW var type (TIMETYPE).    |
|                : (11/03/92) - Fine tune NEW var type (TIMETYPE).    |
|                :           specifically, suppress ldg 0's & fix FN2 |
|                : (13/03/92) - Fix bug in NEW var type (TIMETYPE).   |
|                : (13/03/92) - Fix TIMETYPE error message when upper |
|                :              or lower limits exceeded.             |
|                : (16/03/92) - Add more code around toupper/tolower  |
|                :              in get_field ().                      |
|                : (07/04/92) - Add new functionality for commas in   |
|                :              mask on MONEYTYPE/DOUBLETYPE fields.  |
|                : (10/04/92) - Fixed problem in _getval relating to  |
|                :              the addition of comma formatted fields|
|                : (11/04/92) - Fixed problem in get_entry when FN3   |
|                :              was pressed whilst in a TIMETYPE field|
|                : (15/04/92) - Changed getval/putval routines to be  |
|                :              RAM based (ie: Remove RF_OPEN/RF_CLOSE).|
|                : (15/04/92) - Fix bug in fmt_comma () where trailng  |
|                :              null wasn't being appened to result.  |
|                : (28/04/92) - Fix bug in _getval () and _putval where|
|                :              MAXLINES was referenced instead of    |
|                :              PV_mlines.                            |
|                : (22/06/92) - Recalculate irow after win_function () |
|                :              is called.                            |
|                : (30/07/92) - Recalculate irow for REDRAW of TAB scn|
|  (04/09/92)    : Fix problem where screen not redrawing if editting |
|                : a LIN screen. (Introduced bug with CTRL-W)         |
|  (02/11/92)    : Fix problem where up-arrow whilst in entry would   |
|                : 'wrap-around' to last-field IF the 1st field was   |
|                : not enterable. Also, fixed problem when down-arrow |
|                : pressed, field NOT required, default specified in  |
|                : vars[], prog_status == ENTRY, but field not        |
|                : initialised to default from vars[]. S/C DPL 8026.  |
|  (23/11/92)    : Fixed because of problem with IBM compiler.        |
|  (20/01/93)    : irow should not be recalculated unconditionally    |
|                : within get_field (). SC 8376 PSL.                  |
|  (11/02/93)    : PSL 8526 fix bug in edit_all () when scns was less |
|                : than 10.                                           |
|  (02/03/93)    : INF 8614 Added #ifndefs to take out checks for non-|
|                : standard character checks for INTERNATIONAL_SET    |
|  (04/03/93)    : Allow for large (HUGE) _putvals so that we can do  |
|                : a putval of a WHOLE LINear screen. EGC 8303        |
|  (13/05/93)    : INF 8614 Mask out toupper and tolower if           |
|                : INTERNATIONAL_SET was defined during compilation.  |
|  (26/05/93)    : AMB 8738. If field is CHARTYPE and mask contains   |
|                : N then only digit is valid.                        |
|  (18/02/94)    : PSL 10486. Fix initilisation of temp_str in        |
|                : get_entry ().                                      |
|  (30/01/96)    : Add routines for restart checking. Add environment |
|                : variables SCN_RESTART & SCN_EDIT.                  |
|                : When moving to a field the existing data is not    |
|                : cleared & the cursor sits at the end of the field. |
|                : FN2 toggles between clearing and recalling a field.|
|		         : Allow a new date or numeric to be input before they|
|				 : are edited.                                        | 
|                : Full editing is possible within a field - cursor   |
|                : left/right, character insert/delete. This works    |
|                : for all var types except numerics.                 | 
|                : NB: No changes have been made to TIMETYPE var type.|
|                : Mods for third hot key, PSLW3.                     |
|  (10/04/96)    : Pad temp_str with spaces if nothing is returned    |
|  				 : from a search, in get_field ().                    |
|  (17/07/96)    : ISL - add fld_reposition () routine to reposition  |
|                  tabular screen fields.                             |
|  (03/09/1999)  : set_prnt_mask () - added p_strsave () code to fix  |
|                  problems assoc. with writing to constant memory.   |
|                  Used similar fix to WinNT port.                    | 
|                                                                     |
=====================================================================*/
#include	<VerNo.h>
#include	<pslscr.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/wait.h>

struct	CAL_STORE *pslscr_st_ptr    = CAL_NULL;
struct	CAL_INFO  *pslscr_st_info   = INFO_NULL;

long	pslscr_rslt_date;
long	pslscr_date_overlap;
int		pslscr_curr_day;
char	*PSLSCR_NAME = "@ (#) - (SCRGEN-1.26) 99/04/16 12:59:39";

extern	char	*PNAME;
extern	char	*PROG_VERSION;
extern	int	_win_func;

static char *	FieldContentsAsString 	(int);
static void		HighLightField 			(int, int);
static	union	_var_type	*var_ptr;
static	struct	var		*vars;

char	*ttoa ();
char	*alpha	= " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+-*/%=#$&@'`[]{}_ ()?<>;:,.\\!~\"\243",
        *numeric = "1234567890+-.",
        *intnumeric = "1234567890+-",
        *datenum = "1234567890/",
        *timenum = "1234567890:",
        *validchrs, /*  Pointer to appropiate string for input type.       */
        *mask_chrs = "ANULDS_";

char	below [32],
		above [32];

char	work_str[256];	/* Used for comma formatted numeric fields.	*/

char	SCN_NAME[16];

int	PV_str, PV_scns, PV_tlines, PV_mlines, PV_mwidth;
static	int	 (*PV_use_win) ();
static	int	envScnEdit		=	TRUE;
static	int	envScnInputRev 	= 	FALSE;

void 	fld_reposition 		(int, int *);
int		scn_set 			(int);
int		field_width 		(int);
void	init_vars 			(int);
void	scn_display 		(int);
void	blank_display 		(void);
void	line_display 		(void);
int		goto_field 			(int, int);
int		label 				(char *);
void	display_prmpt 		(int);
void	display_field 		(int);
void	entry 				(int);
void	scn_entry 			(int);
void	edit 				(int);
void	_edit 				(int, int, int);
int		_next_row 			(int, int);
int		_prev_row 			(int, int);
int		_prev_field 		(int, int, int, int);
int		_next_field 		(int, int, int, int);
void	Display_field 		(int, int, int);
void	edit_scn 			(int, int);
void	scn_write 			(int);
void	get_entry 			(int);
int		get_field 			(int);
int		invalid_entry 		(int);
int		getval 				(int);
int		_getval 			(int);
int		putval 				(int);
int		_putval 			(int);
void	set_masks 			(void);
void	_set_masks 			(char *);
void	_set_masks 			(char *);
void	set_prnt_mask 		(int, char *);
void	shell_prog 			(int);
int		DELETE 			(void);
void	no_edit 			(int);
void	edit_ok 			(int);
void	edit_fn 			(int);
void	edit_win 			(int);
void	edit_all 			(void);
int		dis_edit 			(int, int, int, int);
void	_print_head 		(int, int);
void	_print_edit 		(int, int, int);
int		valid_scn 			(int);
void	ent_edit 			(int, int);

extern	void	screen_set 	(const char *, struct var	*);
extern	int		edit_end_eall (int);
extern	int 	heading (int scn);
extern	int  	win_function (int , int, int, int);
extern	int  	win_function2 (int, int, int, int);
extern	int  	win_function3 (int, int, int, int);
extern	int  	win_function4 (int, int, int, int);
extern	int  	win_function5 (int, int, int, int);
static	char	*fmt_comma (char *, int);

int		_ck_restart (void);
int		EnvScreenOK 	= TRUE,
		TruePosition 	= FALSE,
		OUTRANGE_CHAR	= FALSE,
		stopDateSearch	= FALSE,
		X_EALL = 0,
		Y_EALL = 0;

#define		P_NOINPUT	 (FIELD.required == NI)
#define		P_NOEDIT	 (FIELD.required == NE)
#define		P_DISPLAY	 (FIELD.required == NA)
#define		P_HIDE		 (FIELD.required == ND)
#define		P_REQUIRED	 (FIELD.required == YES)
#define		P_INPUT		 (FIELD.required == NO)
#define		P_NO_KEY 	 (FIELD.required == NI || \
			 	  FIELD.required == NA || \
			 	  FIELD.required == ND )

#define		P_LEAVE(x)	 ((x) ? P_NOEDIT || P_HIDE || P_DISPLAY \
				       	      : P_NOINPUT || P_HIDE || P_DISPLAY)

#define		NORV		(FIELD.required == NA || FIELD.required == ND)

#define		P_LIN		 (scn_type == LIN)
#define		P_TAB		 (scn_type == TAB)
#define		P_TXT		 (scn_type == TXT)

#define		_KEY_LEFT(x)	 (x == LEFT_KEY || x == 8)
#define		_KEY_DOWN(x)	 (x == DOWN_KEY || x == 10)
#define		_KEY_UP(x)	 	 (x == UP_KEY || x == 11)
#define		_KEY_RIGHT(x)	 (x == RIGHT_KEY || x == 12)
#define		_KEY_FN13(x)	 (x == FN13)

#define		ASCII_RANGE		128

#define		CPOS			 (((FIELD.col - max_prompt + 1) < 2) \
                             ? 2 : FIELD.col - max_prompt + 1)
/*
 *	Start external interface
 */
void
setup_scr (
	struct var *var_tab,
	int		m_str, 
	int		m_scns, 
	int		t_lines, 
	int		m_lines, 
	int		m_width,
 	int		n,
 	int		(*m_win) ())
{
	char		pslscrWorkDate[11];
	char		*sptr;
	char		*t_str = getenv ("OUTRANGE_CHAR");
	vars 		= var_tab;
	PV_str 		= m_str;
	PV_scns 	= m_scns;
	PV_tlines 	= t_lines;
	PV_mlines 	= m_lines;
	PV_mwidth 	= m_width;
	PV_use_win 	= m_win;

	if (t_str != (char *)0 && t_str[0] == '1')
		OUTRANGE_CHAR = TRUE;
	else
		OUTRANGE_CHAR = FALSE;
	
	strcpy (below, ML ("Below Minimum"));
	strcpy (above, ML ("Above Maximum"));
	strcpy (pslscrWorkDate, DateToString (TodaysDate ()));
	pslscr_rslt_date	=	StringToDate (pslscrWorkDate);
	DateToDMY (pslscr_rslt_date, &pslscr_curr_day, NULL, NULL);

	sptr = chk_env ("RESTART"); 
	restart_ck = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*---------------------------
	| Check for field edit type |
	---------------------------*/
	sptr = chk_env ("SCN_EDIT"); 
	envScnEdit = (sptr == (char *)0) ? TRUE : atoi (sptr);
	if (EnvScreenOK == FALSE)
		envScnEdit = FALSE;

	/*---------------------------
	| Check for field edit type |
	---------------------------*/
	sptr = chk_env ("SCN_INPUT_REV"); 
	envScnInputRev = (sptr == (char *)0) ? 0 : atoi (sptr);

	SYS_LANG	=	lang_select ();

	pslscr_st_ptr	=	set_calendar
					 	(
							pslscr_st_ptr,
							pslscr_rslt_date,
							24L,
							24L
						);

	if (!var_ptr)
	{
	    var_ptr = (union _var_type *) malloc ((n+1) * sizeof (union _var_type));
	    if (!var_ptr)
			file_err (-1, "Error in setup_scr during (MALLOC)", PSLSCR_NAME);
	}
}

/*================================================================
| This Function Sets required variables to allow screen handling |
| for one screen linear or tabular.                              |
================================================================*/
int
scn_set (
	int	scn)
{
	int		field	= 0;
	int		max_col	= 0;

	crsr_off ();

	max_prompt = 0;
	nbr_fields = 0;

	/*-------------------------------------------------------
	| Start search for number of fields in current screen . |
	-------------------------------------------------------*/
	while (FIELD.scn != scn && FIELD.scn != 0)
		field++;

	/*--------------------------
	| Invalid no such screen . |
	--------------------------*/
	if (FIELD.scn == 0)	
		return (-1);

	/*-------------------------------------------
	| Set tabscn to reflect the type of screen. |
	-------------------------------------------*/
	if (FIELD.stype == LIN)
		tabscn = FALSE;
	else
		tabscn = TRUE;
	scn_type = FIELD.stype;

	/*------------------------------------- 
	| Gobal start ptr for current screen. |
	-------------------------------------*/
	scn_start = field;

 	/*---------------------------------------------
	| Calculate the max length of row and column. |
	---------------------------------------------*/
	while (FIELD.scn == scn) 
	{
		if ((int) strlen (FIELD.mask) > PV_str)
		{
			sprintf (err_str,"Error in scn %d label %s, %d > MAXSTR (%d)",scn,vars[field].label,(int) strlen (FIELD.mask),PV_str);
			file_err (-1, err_str,PNAME);
		}
		if (!P_HIDE)
		{
			if ((int) strlen (FIELD.prmpt) > max_prompt)
				max_prompt = strlen (FIELD.prmpt);

			if (FIELD.col > max_col)
				max_col = FIELD.col;
		}
		field++;
		nbr_fields++;
	}
	/*----------------------------------------------------------
	| Set current screen flag to equal screen being worked on. |
	----------------------------------------------------------*/
	cur_screen = scn;

	if ((max_prompt + 5) > max_col) 
		return (-1);
	else
		return (EXIT_SUCCESS);
}

/*=====================================
| return greater then of mask/prompt. |
=====================================*/
int
field_width (int field)
{
	if ((int) strlen (FIELD.prmpt) > (int) strlen (FIELD.mask))
		return (strlen (FIELD.prmpt));

	return (strlen (FIELD.mask));
}

/*========================================================================
| This Function clears all variables local and informix associated with  |
| a screen. Note this holds for a tabular screen as well !!!.            |
========================================================================*/
void
init_vars (int scn)
{
	int	field = 0;
	int	width;

	crsr_off ();
	if (scn != cur_screen)
		scn_set (scn);

	if (P_TXT)
	{
	    field = scn_start;
	    if (vars[scn_start].type < 1 && !init_ok)
	    {
		FIELD.type = 1 + txt_open (FIELD.row, FIELD.col, FIELD.required, FIELD.autonext, FIELD.just, FIELD.prmpt);
		if (FIELD.type < 1)
		{
		    sprintf (err_str, "Error in init_vars (%d). Window not opened", scn);
		    file_err (-1, err_str, PNAME);
		}
	    }
	}
	/*----------------------------------- 
	| Flags if initialising screen ok . |
	-----------------------------------*/
	if (!init_ok)
		return;

	if (P_TXT)
	{
		if (FIELD.type > 0)
			txt_close (FIELD.type - 1, FALSE);
		FIELD.type = 1 + txt_open (FIELD.row, FIELD.col, FIELD.required, FIELD.autonext, FIELD.just, FIELD.prmpt);
		if (FIELD.type < 1)
		{
		    sprintf (err_str, "Error in init_vars (%d). Window not opened", scn);
		    file_err (-1, err_str, PNAME);
		}
		width = FIELD.autonext;
		strcpy (VAR_PTR.cptr, string (width, " "));
		return;
	}

	/*---------------------------------------------
	| For each type of variable clear out values. |
	---------------------------------------------*/
	for (field = scn_start; field < scn_start + nbr_fields; field++)
	{
		switch (FIELD.type)
		{
		case	CHARTYPE :
			width = strlen (FIELD.mask);
			strcpy (VAR_PTR.cptr,string (width," "));
			break;

		case	INTTYPE :
			* (VAR_PTR.iptr) = 0;
			break;

		case	LONGTYPE :
		case	EDATETYPE :
		case	TIMETYPE:
			* (VAR_PTR.lptr) = 0L;
			break;

		case	FLOATTYPE :
			* (VAR_PTR.fptr) = 0.0;
			break;

		case	DOUBLETYPE :
		case	MONEYTYPE :
			* (VAR_PTR.dptr) = 0.0;
			break;

		default :
			break;
		}
	}
}

/*=================================================================
| This function writes a whole screen, regardless of wether it is |
| linear or tabular in nature.                                    |
=================================================================*/
void
scn_display (int scn)
{
	int 	base_line = 0,
		cur_line = 0;

	crsr_off ();
	/*--------------------------
	| If restart keyed return. |
	--------------------------*/
	if (restart)
		return;

	/*------------------------------------------------------
	| If the screen is <> to the screen you are displaying |
	| reset current screen equal to screen. 	       |
	------------------------------------------------------*/
	if (scn != cur_screen)
		scn_set (scn);

	if (P_TXT)
	{
		txt_display (vars[scn_start].type - 1, 1);
		return ;
	}

	/*------------------------------------
	| If linear screen display a line.   |
	| The else reflects tabular screens. |
	------------------------------------*/
	if (P_LIN)
		line_display ();
	else 
	{
		/*------------------- 
		| save current row. |
		-------------------*/
		cur_line = line_cnt;

		/*------------------------------------------
		| calc page offset for multi-page tabular. |
		------------------------------------------*/
		base_line = (line_cnt / PV_tlines) * PV_tlines;
		line_cnt = base_line;
		while (line_cnt < base_line + PV_tlines && 
			line_cnt < lcount[cur_screen] && 
			line_cnt < vars[scn_start].row) 
		{
			/*-------------------------------------- 
			| getval loads only if valid line_cnt .|
			--------------------------------------*/
			getval (line_cnt);
			line_display ();
			line_cnt++;
		}

		/*------------------------------- 
		| reset to cur line and values. |
		-------------------------------*/
		line_cnt = cur_line;
		getval (line_cnt);
	}
}

void
blank_display (void)
{
	int	field = 0;
	int	width;

	crsr_off ();
	if (P_LIN || P_TXT)
		return;

	for (field = scn_start;field < scn_start + nbr_fields;field++)
	{
		if (P_HIDE)
			continue;

		width = strlen (FIELD.mask);
		move (FIELD.col,tab_row + 2 + (line_cnt % PV_tlines));
		printf ("%*.*s",width,width," ");
	}
}

/*====================================================== 
| This Function Displays one line or screen of linear. |
======================================================*/
void
line_display (void)
{
	int	field = 0;

	for (field = scn_start; field < scn_start + nbr_fields; field++)
		display_field (field);
}

/*===============================================
| Goto specified field within current screen	|
| field is current field number			|
| field_label is new fields label		|
===============================================*/
int
goto_field (
	int	field, 
	int	gfield)
{

	skip_entry = 0;

	if (gfield < scn_start || gfield >= scn_start + nbr_fields)
		return (EXIT_SUCCESS);
	
	return (gfield - field - 1);
}

/*================================================================
| This function Returns Number of a given label -1 if not found. |
================================================================*/
int
label (char	*field_label)
{
	int	found = FALSE,
		ptr = 0;

	while (!found && vars[ptr].scn != 0)
	{
		if (strcmp (vars[ptr].label,field_label))
			ptr++;
		else
			found = TRUE;
	}
	if (found)	
		return (ptr);
	else
	{
		sprintf (err_str,"Error in locating label %s",field_label);
		file_err (-1, err_str,PNAME);
	}
	return (EXIT_SUCCESS);
}

void
display_prmpt (int	field)
{
	int	p_col;

	if (P_HIDE)
		return;

	crsr_off ();
	p_col = (TruePosition) ? FIELD.col : CPOS;
	move ((p_col < 1) ? 1 : p_col,FIELD.row);
	printf ("%s",FIELD.prmpt);
}

/*============================================================================
| This function Displays a single field for either linear or tabular screen. |
============================================================================*/
void
display_field (
	int		field)
{
	int		lcol 		= 0,
			lrow 		= 0,
			reverseFlag	= 0;
		
	if (field == -1)
		return;

	if (P_HIDE)
		return;

	crsr_off ();
	/*------------------------------------------------------
	| Calculate the row to move to for linear and tabular. |
	------------------------------------------------------*/
	lrow = (P_TAB) ? (tab_row + 2 + (line_cnt % PV_tlines)) : FIELD.row;

	if (P_LIN)
		lcol = (TruePosition) ? FIELD.col + strlen(FIELD.prmpt) : FIELD.col + 3;
	else
		lcol = FIELD.col;

	reverseFlag = (envScnInputRev && !NORV) ? TRUE : FALSE;
	
	strcpy (work_str, FieldContentsAsString (field));
	li_pr (work_str, lcol, lrow, (reverseFlag) ? 1 : 0);
	fflush (stdout);
}

/*===========================================================
| This Function reads the screen for info - as instructed . |
| Note : both tabular and linear are handled.               |
===========================================================*/
void
entry (int scn)
{
	if (restart)	
		return;

	crsr_off ();
	if (scn != cur_screen)
		scn_set (scn);

	/*-------------------------------
	| Reset line count for tabular. |
	-------------------------------*/
	line_cnt = 0;		
	cur_field = 0;

	if (P_TXT)
	{
		while (1)
		{
			lcount[scn] = line_cnt = txt_scn_edit (vars, scn_start);
			if (restart)
			{
				if (_ck_restart ())
					break;
				else   
					restart = 0;
			}
			else
				break;
		}
		return ;
	}

	/*--------------------------------
	| Reset here to ensure it works. |
	--------------------------------*/
	entry_exit = 0;
	skip_entry = 0;
	prog_status = ENTRY;
	edit_status = FALSE;

	/*-------------------------------------------------------- 
	| Will process one screen for linear / one tabular line. |
	--------------------------------------------------------*/
	do {
		scn_entry (scn);

	} while (P_TAB && line_cnt < vars[scn_start].row
			&& !entry_exit 
			&& !prog_exit 
			&& !restart
			&& line_cnt < PV_mlines);

	/*-----------------------
	| record tabular lines. |
	-----------------------*/
	if (PV_tlines != 0)
		scn_page = line_cnt / PV_tlines;
	else
		scn_page = 0;

	lcount[scn] = line_cnt;
	line_cnt = 0;
}

/*===============================================================
| This Function is concerned with the entry of one screen/line. |
===============================================================*/
void
scn_entry (int	scn)
{
	int	field = 0;
	int	invalid = 0;
	int	max_field = 0;
	int	old_field = 0;
	int	old_cnt;

	crsr_off ();

	init_vars (cur_screen);

	field = scn_start;
	row = (P_TAB) ? (tab_row + 2 + (line_cnt % PV_tlines)) : input_row;
	max_field = scn_start + nbr_fields - 1;

	while (field >= scn_start && field < scn_start + nbr_fields && 
		!entry_exit && !prog_exit && !restart) 
	{
		cur_field = field;
		/*-------------------------------------------------- 
		| Check if need to redraw screen on tabular entry. |
		--------------------------------------------------*/
		if (P_TAB && line_cnt && ! (line_cnt % PV_tlines) && 
		     field == scn_start) 
			scn_write (cur_screen);

		/*------------------------------------------------------
		| Field cannot be keyed so just do spec_val and leave. |
		------------------------------------------------------*/
		if (!P_NO_KEY)
		{
			col = (P_TAB) ? FIELD.col : input_col;
			move (col,row);
			do 
			{
				while (1)
				{
					get_entry (field);
					if (restart)
					{
						if (_ck_restart ())
							break;
						else
							restart = 0;
					}
					else
						break;
				}
				if (restart || prog_exit || entry_exit)
				{	
					invalid = -1;
					break;
				}

				/*--------------------------------
				| UP key hit while in get_entry. |
				--------------------------------*/
				if (_KEY_UP (last_char) ||
					 (_KEY_FN13 (last_char) && P_TAB))
				{
					invalid = -1;
					break;
				}
				invalid = (invalid_entry (field) || spec_valid (field));
			} while (invalid > 0);

			if (invalid != -1)
				display_field (field);

			/*-----------------------------------------------
			| If FN13_KEY Pressed while in tabular		|
			| Call edit mode for current screen.		|
			-----------------------------------------------*/
			if (_KEY_FN13 (last_char) && P_TAB)
			{
			    if (in_sub_edit)
			    {
					print_mess (ML (" Already within FN13 Sub-Edit Mode!! "));
					sleep (sleepTime);
					continue;
			    }
			    else
			    {
					in_sub_edit = TRUE;
					old_cnt = line_cnt;
					_putval (0);
					blank_display ();
					if (init_ok)
				    	lcount[scn] = line_cnt;
					scn_page = line_cnt / PV_tlines;
					ent_edit (scn, field);
					prog_status = ENTRY;
					heading (scn);
					if (init_ok)
						line_cnt = lcount[scn];
					else
						line_cnt = old_cnt;
					scn_display (scn);
					_getval (0);
					if (field != scn_start)
						line_display ();
					in_sub_edit = FALSE;
					continue;
			    }
			}
			/*-------------------------------------------
			| If UP_KEY Pressed							|
			| Go To previous Field Available for Input	|
			-------------------------------------------*/
			if (_KEY_UP (last_char))
			{
				if (field != scn_start)
					display_field (field--);

				field = _prev_field (field, field + 1,
						     max_field,FALSE);
				continue;
			}
			/*-----------------------------------------------
			| If DOWN_KEY Pressed				|
			| Go To previous Field Available for Input	|
			-----------------------------------------------*/
			if (_KEY_DOWN (last_char))
			{
				skip_entry = 0;

				old_field = field;

				if (field != max_field)
					display_field (field++);

				field = _next_field (field,field - 1,max_field,FALSE);

				if (field < old_field)
					field = old_field;

				continue;
			}
			clear_mess ();
		}
		else 
		{
			/*-----------------------------------------------
			| first field & not first line & not last line? |
			-----------------------------------------------*/
			if (P_TAB && line_cnt && ! (line_cnt % PV_tlines) && 
			     field == scn_start) 
			{
				scn_write (cur_screen);
				scn_display (cur_screen); 
			}
			invalid = spec_valid (field);
			if (invalid)
			{
				restart = TRUE;
				sleep (sleepTime);
				break;
			}
		}
		field++;
		if (skip_entry) 
		{
			field += skip_entry;
			if (field < scn_start)
				field = scn_start;

			skip_entry = 0;
		}

		/*------------------------------------------------------
		| If EOI key and within screen/line goto start of line |
		| Only if 'eoi_ok' is TRUE .                           |
		------------------------------------------------------*/
		if (end_input && eoi_ok)
		{
			field = scn_start;
			/*-----------------------------------------------
			| while in same screen & field is NA or 	|
			| field is (NI && in input)			|
			-----------------------------------------------*/
			while ((field < max_field) && (!P_REQUIRED && !P_INPUT)) 
				field++;

			if (field != max_field)
				end_input = 0;
		}
	}
	if (!entry_exit && P_TAB) 
		putval (line_cnt++);
}

void
edit (int	scn)
{
	_edit (scn, 0, 0);
}

void
_edit (
	int		scn,
	int		ed_line_no,
	int		ed_field_no)
{
	int	c;
	int	key = 0;
	int	last_row = -1;
	int	curr_row = 0;
	int	last_field = -1;
	int	old_page = 0;
	int	field = 0;
	int	max_field = 0;
	int	work_line = 0;
	int	temp_line = 0;

	line_cnt = ed_line_no;
	curr_row = ed_line_no % PV_tlines;
	cur_field = 0;		

	/*--------------------------
	| If restart keyed return. |
	--------------------------*/
	if (restart) 
	{
		edit_exit = TRUE;
		return;
	}
	crsr_off ();
	scn_page = old_page = (ed_line_no / PV_tlines);
	edit_exit = 0;
	entry_exit = 0;
	edit_exit = 0;

	prog_status = ! (ENTRY);
	if (scn != cur_screen)
		scn_set (scn);

	max_field = scn_start + nbr_fields - 1;
	if (ed_field_no >= scn_start && ed_field_no <= max_field)
		field = ed_field_no;
	else
		field = scn_start;

	if (P_TXT)
	{
		while (1)
		{
			lcount[scn] = line_cnt = txt_scn_edit (vars, scn_start);
			if (restart)
			{
				if (_ck_restart ())
					break;
				else   
					restart = 0;
			}
			else
				break;
		}
		return ;
	}

	if (P_TAB) 
	{
		while (field <= max_field && (P_NOEDIT || P_HIDE || P_DISPLAY))
			field++;

		if (field > max_field)
			field = scn_start;

		while (!restart)
		{
			cur_field = field;
			if (scn_page != old_page) 
			{
				line_cnt = curr_row % PV_tlines;
				line_cnt += (scn_page * PV_tlines);
				scn_write (cur_screen);
				scn_display (cur_screen);
				last_row = -1;
				last_field = -1;
			}

			if (last_row != curr_row || last_field != field)
			{
			    if (last_row >= 0 && last_field >= scn_start)
			    {
				work_line = last_row % PV_tlines;
				work_line += (scn_page * PV_tlines);
				getval (work_line);
				temp_line = line_cnt;
				line_cnt = work_line;
				if (line_cnt >= lcount[cur_screen])
				    Display_field (last_field, FALSE, TRUE);
				else
				    Display_field (last_field, FALSE, FALSE);
				line_cnt = temp_line;
			    }

			    work_line = curr_row % PV_tlines;
			    work_line += (scn_page * PV_tlines);

			    if (curr_row >= 0 && field >= scn_start)
			    {
				getval (work_line);
				temp_line = line_cnt;
				line_cnt = work_line;
				if (line_cnt >= lcount[cur_screen])
				    Display_field (field, TRUE, TRUE);
				else
				    Display_field (field, TRUE, FALSE);
				line_cnt = temp_line;
			    }

			    tab_other (work_line);
			}

			last_row = curr_row;
			last_field = field;
			old_page = scn_page;

			c = getkey ();

			if (c == REDRAW || c == FN_HELP || 
			     c == HELP || c == HELP1 || 
			     c == EXIT_OUT || (c > FN16 && c <= FN32))
			{
				key = c;
				c = FN17;
				last_char = REDRAW;
			}

			if (_KEY_LEFT (c) || _KEY_UP (c) || 
			     _KEY_DOWN (c) || _KEY_RIGHT (c))
			{
				key = c;
				c = LEFT_KEY;
			}

			if (c == RESTART)
			{
				if (_ck_restart ())
				{
					key = RESTART;
					c = FN16;
				}
				else
					c = key = 0;
			}

			switch (c) 
			{
			case	FN17:
				if (key != REDRAW)
					shell_prog (key);

				last_row = -1;
				last_field = -1;
				c = line_cnt;
				heading (cur_screen);
				line_cnt = c;
				scn_display (cur_screen);
				break;

			/*---------------
			| Next Page	|
			---------------*/
			case	FN14:
				if (((scn_page + 1 <
				    vars [scn_start].row / PV_tlines) ||
				    vars [scn_start].row % PV_tlines != 0) &&
				    lcount [cur_screen] / PV_tlines > scn_page)
				{
					scn_page++;
				}
				else
					scn_page = 0;
				curr_row = 0;
				break;

			/*---------------
			| Prev Page	|
			---------------*/
			case	FN15:
				if (scn_page != 0)
				    scn_page--;
				else
				{
				    scn_page = lcount[cur_screen] / PV_tlines;
				    if (lcount[cur_screen] % PV_tlines == 0 && lcount[cur_screen] == vars[scn_start].row)
					scn_page--;
				}
				curr_row = 0;
				break;

			case	LEFT_KEY:
				if (_KEY_LEFT (key))
				{
					last_field = field--;
					field = _prev_field (field, last_field,
							     max_field, TRUE);
				}
				if (_KEY_RIGHT (key))
				{
					last_field = field++;
					field = _next_field (field,last_field,
							     max_field,TRUE);
				}
				if (_KEY_DOWN (key))
				{
					last_row = curr_row++;
					curr_row = _next_row (curr_row, last_row);
				}
				if (_KEY_UP (key))
				{
					last_row = curr_row--;
					curr_row = _prev_row (curr_row,
							     last_row);
				}
				break;

			case	FN16:
				if (key == RESTART)
					restart = TRUE;
				return;

			case	'\r':
				if (P_NOEDIT || P_HIDE || P_DISPLAY)
					break;

				line_cnt = curr_row % PV_tlines;
				line_cnt += (scn_page * PV_tlines);
				edit_scn (field,line_cnt);
				last_row = -1;
				last_field = -1;
				break;

			case PSLW	:
				if (_win_func)
				{
				    win_function (
						cur_field,
						 (curr_row % PV_tlines) + (scn_page * PV_tlines),
						cur_screen,
						!ENTRY);
					old_page = -1;
				}
				else
					putchar (BELL);
				break;

			case PSLW2	:
				if (_win_func)
				{
				    win_function2 (
						cur_field,
						 (curr_row % PV_tlines) + (scn_page * PV_tlines),
						cur_screen,
						!ENTRY);
					old_page = -1;
				}
				else
					putchar (BELL);
				break;

			case PSLW3	:
				if (_win_func)
				{
				    win_function3 (
						cur_field,
						 (curr_row % PV_tlines) + (scn_page * PV_tlines),
						cur_screen,
						!ENTRY);
					old_page = -1;
				}
				else
					putchar (BELL);
				break;

			case PSLW4	:
				if (_win_func)
				{
				    win_function4 (
						cur_field,
						 (curr_row % PV_tlines) + (scn_page * PV_tlines),
						cur_screen,
						!ENTRY);
					old_page = -1;
				}
				else
					putchar (BELL);
				break;

			case PSLW5	:
				if (_win_func)
				{
				    win_function5 (
						cur_field,
						 (curr_row % PV_tlines) + (scn_page * PV_tlines),
						cur_screen,
						!ENTRY);
					old_page = -1;
				}
				else
					putchar (BELL);
				break;

			default:
				putchar (BELL);
				break;
			}
		}
	}
	else
	{
		while (field <= max_field && (P_NOEDIT || P_DISPLAY || P_HIDE))
			field++;

		if (field > max_field)
			field = scn_start;

		while (!restart)
		{
			cur_field = field;
			if (last_field != field)
			{
				if (last_field >= scn_start)
					HighLightField (last_field, FALSE);

				if (field >= scn_start)
					HighLightField (field, TRUE);
			}

			last_field = field;

			c = getkey ();

			if (c == FN_HELP || c == HELP || 
			     c == HELP1 || c == EXIT_OUT || 
			     (c > FN16 && c <= FN32))
			{
				key = c;
				c = FN17;
			}

			if (_KEY_LEFT (c) || _KEY_UP (c))
			{
				c = 11;
				key = UP_KEY;
			}

			if (_KEY_RIGHT (c) || _KEY_DOWN (c))
			{
				c = 11;
				key = DOWN_KEY;
			}

			if (c == RESTART)
			{
				if (_ck_restart ())
				{
					key = RESTART;
					c = FN16;
				}
				else
					c = key = 0;
			}

			switch (c) 
			{
			case	FN17:
				shell_prog (key);
				last_char = REDRAW;
				heading (scn);
				scn_display (scn);
				last_field = -1;
				break;

			case PSLW	:
				if (_win_func)
				{
				    win_function (cur_field, 0, cur_screen, !ENTRY);

					if (!pslw_asl)
					{
						/* Redraw */
						last_char = REDRAW;
						heading (scn);
						scn_display (scn);
					}
					last_field = -1;
				}
				else
				    putchar (BELL);
				break;

			case PSLW2	:
				if (_win_func)
				{
				    win_function2 (cur_field, 0, cur_screen, !ENTRY);

					if (!pslw_asl)
					{
						/* Redraw */
						last_char = REDRAW;
						heading (scn);
						scn_display (scn);
					}
					last_field = -1;
				}
				else
				    putchar (BELL);
				break;

			case PSLW3	:
				if (_win_func)
				{
				    win_function3 (cur_field, 0, cur_screen, !ENTRY);

					if (!pslw_asl)
					{
						/* Redraw */
						last_char = REDRAW;
						heading (scn);
						scn_display (scn);
					}
					last_field = -1;
				}
				else
				    putchar (BELL);
				break;

			case PSLW4	:
				if (_win_func)
				{
				    win_function4 (cur_field, 0, cur_screen, !ENTRY);

					if (!pslw_asl)
					{
						/* Redraw */
						last_char = REDRAW;
						heading (scn);
						scn_display (scn);
					}
					last_field = -1;
				}
				else
				    putchar (BELL);
				break;

			case PSLW5	:
				if (_win_func)
				{
				    win_function5 (cur_field, 0, cur_screen, !ENTRY);

					if (!pslw_asl)
					{
						/* Redraw */
						last_char = REDRAW;
						heading (scn);
						scn_display (scn);
					}
					last_field = -1;
				}
				else
				    putchar (BELL);
				break;

			case	REDRAW:
				last_char = REDRAW;
				heading (scn);
				scn_display (scn);
				last_field = -1;
				break;

			case	FN5:
				psl_print ();
				break;

			case	FN14:
			case	FN15:
				if (PV_use_win)
					 (*PV_use_win) (c);
				break;

			case	11:
				if (key == UP_KEY)
				{
					last_field = field--;
					field = _prev_field (field,last_field,max_field,TRUE);
				}
				else
				{
					last_field = field++;
					field = _next_field (field,last_field,max_field,TRUE);
				}
				break;

			case	FN16:
				if (key == RESTART)
					restart = TRUE;
				crsr_on ();
				return;

			case	'\r':
				edit_scn (field,field);
				last_field = -1;
				break;

			default:
				putchar (BELL);
				break;
			}
		}
	}
}

int
_next_row (
 int	_curr_row,
 int	last_row)
{
	int	curr_row = _curr_row;

	if (curr_row >= PV_tlines || curr_row >= vars[scn_start].row - (scn_page * PV_tlines))
		curr_row = 0;
	
	return (curr_row);
}

int
_prev_row (
	int	_curr_row,
	int	last_row)
{
	int	curr_row = _curr_row;

	if (curr_row < 0)
	{
		curr_row = PV_tlines - 1;

		while (curr_row > last_row && curr_row >= vars[scn_start].row - (scn_page * PV_tlines))
			curr_row--;
	}
	
	return (curr_row);
}

int
_prev_field (
	int	_field,
	int	last_field,
	int	max_field,
	int	in_edit)
{
	int	field = _field;

	while (field >= scn_start && P_LEAVE (in_edit))
	{
		if (in_edit)
			field--;
		else
			spec_valid (field--);
	}

	if (field < scn_start && in_edit)
	{
		field = max_field;
		while (field > last_field && P_LEAVE (in_edit))
		{
			if (in_edit)
				field--;
			else
				spec_valid (field--);
		}
	}
	if (field < scn_start && !in_edit)
		return (_field);
	return (field);
}

int
_next_field (
	int	_field,
	int	last_field,
	int	max_field,
	int	in_edit)
{
	int	field = _field;

	while (field <= max_field && P_LEAVE (in_edit))
	{
		if (in_edit)
			field++;
		else
		{
			spec_valid (field++);
		}
	}

	if (field > max_field)
	{
		field = scn_start;
		while (field < last_field && P_LEAVE (in_edit))
		{
			if (in_edit)
				field++;
			else
			{
				spec_valid (field++);
			}
		}
	}
	return (field);
}

void
Display_field (
 int	field,
 int	reverse,
 int	blank)
{
	int	lrow = 0;
		
	if (field == -1)
		return;

	if (P_HIDE)
		return;

	crsr_off ();
	/*------------------------------------------------------
	| Calculate the row to move to for linear and tabular. |
	------------------------------------------------------*/
	lrow = (P_TAB) ? (tab_row + 2 + (line_cnt % PV_tlines)) : FIELD.row;

	if (P_LIN)
	{
		if (TruePosition)
			move (FIELD.col + strlen (FIELD.prmpt), lrow);
		else
			move (FIELD.col + 3,lrow);
	}
	else
		move (FIELD.col,lrow);

	if (blank)
	{
		if (reverse)
			rv_on ();
		printf ("%-*.*s", (int) strlen (FIELD.mask), (int) strlen (FIELD.mask), " ");
		if (reverse)
			rv_off ();
		fflush (stdout);
		return;
	}

	if (reverse)
		rv_on ();

	switch (FIELD.type)
	{
	case	CHARTYPE :
		printf (FIELD.pmask, (FIELD.mask[0] == '_') ?	FIELD.mask :
								VAR_PTR.cptr);
		break;

	case	INTTYPE :
		printf (FIELD.pmask,* (VAR_PTR.iptr));
		break;

	case	LONGTYPE :
		printf (FIELD.pmask,* (VAR_PTR.lptr));
		break;

	case	FLOATTYPE :
		printf (FIELD.pmask, *(VAR_PTR.fptr));
		break;

	case	DOUBLETYPE :
		sprintf (work_str, FIELD.pmask, *(VAR_PTR.dptr));
		if (strchr (FIELD.mask, ',') == (char *) 0)
			printf (work_str);
		else
			printf (fmt_comma (work_str, field));
		break;

	case	MONEYTYPE :
		sprintf (work_str, FIELD.pmask,DOLLARS (*(VAR_PTR.dptr)));
		if (strchr (FIELD.mask, ',') == (char *) 0)
			printf (work_str);
		else
			printf (fmt_comma (work_str, field));
		break;

	case	EDATETYPE :
		printf (FIELD.pmask,DateToString (* (VAR_PTR.lptr)));
		break;

	case	TIMETYPE :
		printf ("%s", ttoa (* (VAR_PTR.lptr), FIELD.mask));
		break;

	default :
		break;
	}

	if (reverse)
		rv_off ();

	fflush (stdout);
}

void
edit_scn (
	int	field,
	int	col_index)
{
	int	c = 0,
		noedit,
		invalid = 0,
		edit_line = 0;

	if (P_HIDE)
		return;

	crsr_off ();
	if (P_TAB) 
	{
		edit_line = col_index;
		line_cnt = edit_line;

		/*----------------------- 
		| Add to end of fields. |
		-----------------------*/
		if (col_index >= lcount[cur_screen]) 
		{ 
			edit_line = lcount[cur_screen];
			line_cnt = edit_line;
			/*----------------------- 
			| Check for wrap around.|
			-----------------------*/
			if (edit_line/PV_tlines != scn_page) 
			{  
				scn_page = line_cnt / PV_tlines;
				scn_write (cur_screen);
				scn_display (cur_screen);
			}
			prog_status = ENTRY;
			edit_status = TRUE;
			scn_entry (cur_screen);
			edit_status = FALSE;
			prog_status = ! (ENTRY);

			if (last_char != EOI)
				lcount[cur_screen]++;
		}
		else 
		{
			noedit = P_NOEDIT;
			if (c == EOI || restart || noedit) 
			{
				if (noedit)
					putchar (BELL);
				return;
			}

			if (!P_DISPLAY)
			{
				row = tab_row + 2 + ((edit_line) % PV_tlines);
				col = FIELD.col;
				/*--------------------------------- 
				| Check for cur screen & display. |  
				---------------------------------*/
				if (lcount[cur_screen] && edit_line/PV_tlines != scn_page)
				{
					scn_page = line_cnt / PV_tlines;
					scn_write (cur_screen);
					scn_display (cur_screen);
				}

				move (col,row);

				getval (line_cnt);
				do 
				{
					while (1)
					{
						get_entry (field);
						if (restart)
						{
							if (_ck_restart ())
								break;
							else
								restart = 0;
						}
						else
							break;
					}
					if (restart)
					{
						invalid = -1;
						break;
					}
					if (end_input)
						invalid = 1;
					else
						invalid = (invalid_entry (field) || spec_valid (field));
				} while (invalid > 0);

				if (invalid != -1)
					display_field (field);
				clear_mess ();

				putval (line_cnt);
			}
			else 
				spec_valid (field);
		}
	}
	/*---------------------
	| Linear screen edit .|
	---------------------*/
	else 
	{	
		if (P_NOEDIT)
		{
			putchar (BELL);
			return;
		}

		if (!P_DISPLAY)
		{
			row = input_row;
			col = input_col;

			do 
			{
				while (1)
				{
					get_entry (field);
					if (restart)
					{
						if (_ck_restart ())
							break;
						else   
							restart = 0;
					}
					else
						break;
				}
				if (restart)
				{
					invalid = -1;
					break;
				}
				if (end_input)
					invalid = 1;
				else
					invalid = (invalid_entry (field) || spec_valid (field));
			} while (invalid > 0);

			if (invalid != -1)
				display_field (field);

			clear_mess ();
		}
		else 
			spec_valid (field);
	}
}

/*
	fld_reposition ()

	Parameter		scn 	- Screen Number
					offset	- Integer array containing offset of each field

	Description		fld_reposition only work on TAB screen. It take the screen
					and offset integer array to recalculate all the position
					of the visible TAB field. If the integer array is a NULL
					pointer, it neglect the offset.
*/
void
fld_reposition (
 int scn,
 int *offset)
{
	int field = 0,				/*  local copy allows macro use */
  	    field_offset = 0,
		i = 0;

	if (restart)				/*  case of restart flag being set  */
		return;

	if (scn != cur_screen)
		scn_set (scn);

	if (P_TXT || P_LIN)
		return;
	else
	{
		/*---------------------------------- 
		| Check prompt & field mask width. |
		-----------------------------------*/
		field_offset = 1;
		for (field=scn_start, i=0; field<scn_start + nbr_fields; field++, i++)
		{
			if (P_HIDE)
				continue;

			if (offset != (int *) NULL)
				FIELD.col = tab_col + field_offset + offset[i];
			else
				FIELD.col = tab_col + field_offset;

			field_offset = field_offset + field_width (field) + 1;
		}
	}
}
void
scn_write (
	int scn)
{
	int		q,
			temp_ptr,
			field = 0,		/*  local copy allows macro use	*/
			base_line = 0,
			field_offset = 0;
	char	prnt_mask[11],
			scn_mask[256];

	char	WorkStr[30];
	
	if (restart)		/*  case of restart flag being set	*/
		return;

	crsr_off ();

	if (scn != cur_screen)
		scn_set (scn);

	if (P_TXT)
		return;
	
	if (P_LIN) 
	      for (field = scn_start;field < scn_start + nbr_fields;field++)
			display_prmpt (field);
	else 
	{
		/*--------------------------------------
		| Set the width for each screen field. |
		--------------------------------------*/
		print_at (tab_row, tab_col, "%s%c%s", ta[16],_ta[12][5], ta[17]);

		/*---------------------------------- 
		| Check prompt & field mask width. |
		-----------------------------------*/
		field_offset = 1;
		for (field = scn_start;field < scn_start + nbr_fields;field++) 
		{
			if (P_HIDE)
				continue;

			tab_index[field - scn_start + 1] = field_offset + tab_col;
			sprintf (prnt_mask,"%%%ds",field_width (field));
			printf (prnt_mask,FIELD.prmpt);
			printf ("%s%c%s", ta[16],_ta[12][5],ta[17]);

			/*  FIELD.col contains offset within screen field */
			if (FIELD.col < tab_col + field_offset)
				FIELD.col = tab_col + field_offset + FIELD.col;
			field_offset = field_offset + field_width (field) + 1;
		}
		tab_index[field - scn_start + 1] = tab_col;
		print_at (tab_row + 1, tab_col,"%s%c%s", ta[16],_ta[12][10],ta[17]);

		for (field = scn_start;field < scn_start + nbr_fields;field++) 
		{
			if (P_HIDE)
				continue;

			printf ("%s%s%c%s",ta[16],string (field_width (field),&_ta[12] [6]),_ta[12][7],ta[17]);
		}
		printf ("\b%s%c%s", ta[16],_ta[12][11],ta[17]);

		/*--------------------------------------------------
		| start at current line map upto tabular lines or  |
		| max lines for screen.                            |
		|                                                  |
		| Generate mask once - rest of lines are same.     |
		--------------------------------------------------*/
		strcpy (scn_mask,"");
		temp_ptr = 0;
		for (field = scn_start;field < scn_start + nbr_fields;field++)
		{
			if (P_HIDE)
				continue;

			strcat (scn_mask,string (field_width (field)," "));
			sprintf (WorkStr, "%s%c%s", ta[16],_ta[12][5], ta[17]);
			strcat (scn_mask,WorkStr);
			temp_ptr = strlen (scn_mask);
			scn_mask[temp_ptr] = '\0';
		}
		base_line = (line_cnt / PV_tlines) * PV_tlines;
		for (q = base_line;q < base_line + PV_tlines;q++) 
		{
			move (tab_col,tab_row + 2 + (q % PV_tlines));
			printf ("%s%c%s%s",ta[16],_ta[12][5],ta[17],scn_mask);
		}
	}
}

/*===============================================================
| The start of the donkey routines for input                    |
| Here concerned with move stored to temp and vice versa.       |
===============================================================*/
void
get_entry (
	int	field)
{
	int	rc = 0, /* return code from get_field (0 if nothing entered) */
		reqrd = 0,	/* Flags if field required or not */
		lrow,
		ptr = 0;

	crsr_off ();

	dflt_used = 0; /* initialise for each field entered  */
	print_mess (FIELD.comment);
	lrow = (P_TAB) ? (tab_row+2+ (line_cnt % PV_tlines)) : FIELD.row;
	if (P_LIN)
	{
		if (TruePosition)
			move (FIELD.col + strlen (FIELD.prmpt),lrow);
		else
			move (FIELD.col + 3,lrow);
	}
	else
		move (FIELD.col,lrow);

	memset (temp_str, 0, LS_SIZEOF_TEMP_STR);

	/*---------------------------------------------------------------------
	| 'reqrd' is set to allow nothing to be entered only if there is a    |
	| default for the field or the field type is NOT 'NE' or 'YES'        |
	---------------------------------------------------------------------*/
	if (strlen (FIELD.deflt) == 0 && (P_REQUIRED || P_NOEDIT))
		reqrd = 1;

	switch (FIELD.type) 
	{
	case	CHARTYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = alpha;
		sprintf (temp_str,FIELD.pmask,VAR_PTR.cptr);

		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		if (!rc || (_KEY_DOWN (last_char) && strlen (temp_str) == 0))
		{ 
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		sprintf (VAR_PTR.cptr,FIELD.pmask,temp_str);

		/*-------------------------------------
		| Check for non-blank fill character. |
		-------------------------------------*/
		if (FIELD.fill[0] != ' ') 
		{
			for (ptr = 0; ptr < (int) strlen (VAR_PTR.cptr); ptr++) 
			{
				if (VAR_PTR.cptr[ptr] == ' ')
					VAR_PTR.cptr[ptr] = *FIELD.fill;
			}
		}
		break;

	case	INTTYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = intnumeric;

		sprintf (temp_str,FIELD.pmask,* (VAR_PTR.iptr));
		dec_pt = TRUE;	/*  so no dec_pt allowed in input	*/
		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc || (_KEY_DOWN (last_char) && atoi (temp_str) == 0))
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.iptr) = atoi (temp_str);
		break;

	case	LONGTYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = intnumeric;
		sprintf (temp_str,FIELD.pmask,* (VAR_PTR.lptr));
		dec_pt = TRUE;	/*  so no dec_pt allowed in input	*/
		do 
		{ 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc || (_KEY_DOWN (last_char) && atol (temp_str) == 0))
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.lptr) = atol (temp_str);;
		break;

	case	FLOATTYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = numeric;
		sprintf (temp_str,FIELD.pmask, *(VAR_PTR.fptr));
		dec_pt = FALSE;
		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);
	
		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc || (_KEY_DOWN (last_char) && atof (temp_str) == 0))
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.fptr) = (float) atof (temp_str);
		break;

	case	DOUBLETYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = numeric;
		sprintf (temp_str,FIELD.pmask, *(VAR_PTR.dptr));
		dec_pt = FALSE;
		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc || (_KEY_DOWN (last_char) && atof (temp_str) == 0))
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.dptr) = atof (temp_str);
		break;

	case	MONEYTYPE :
		if (strlen (FIELD.lowval) && !strlen (FIELD.highval))
			validchrs = FIELD.lowval;
		else
			validchrs = numeric;
		sprintf (temp_str,FIELD.pmask,DOLLARS (*(VAR_PTR.dptr)));
		dec_pt = FALSE;
		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc || (_KEY_DOWN (last_char) && atof (temp_str) == 0))
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.dptr) = CENTS (atof (temp_str));
		break;

	case	EDATETYPE :
		/*-----------------------------------------------
		| Only the date characters are acceptable here. |
		-----------------------------------------------*/
		validchrs = datenum;
		sprintf (temp_str,FIELD.pmask,DateToString (* (VAR_PTR.lptr)));
		do { 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc)
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.lptr) = StringToDate (temp_str);
		break;

	case	TIMETYPE :
		validchrs = timenum;
		sprintf (temp_str, "%s", ttoa (* (VAR_PTR.lptr), FIELD.mask));
		do 
		{ 
			rc = get_field (field);
		} while (rc == 0 && reqrd);

		/*----------------------------------
		| Set default value if applicable. |
		----------------------------------*/
		if (!rc)
		{
			strcpy (temp_str,FIELD.deflt);
			dflt_used = 1;
		}
		* (VAR_PTR.lptr) = atot (temp_str);
		break;

	default :
		break;
	}	/* end of switch on type	*/

	clear_mess ();

}	/*  end of get entry	*/


/*===========================================================
| Given the field number allow the operator to change/enter | 
| the variables value while appro.                          |
===========================================================*/
int
get_field (
	int	field)
{
	int	ml,
		x_ptr,
		t_ptr,
		lrow,
		base_line,
		ptr,
		var_lead = 0,
		var_trail = 0,
		max_inpt = 0,	/* max length of field as was keyed in */
		dec_flag = FALSE,
		recall = FALSE,	/* TRUE if recall key used in input */
		cont = TRUE,
		iptr,
		irow, icol,
		no_commas = 0,
		first_time,    	     	/* allow a new date or numeric to be 
									input before a field is edited        */
		insert_mode = FALSE,	/* toggle between insert & overwrite mode */
		winFuncCall = FALSE;    /* True when a win_function is called     */

	/*-----------------------------
	| Set first_time to FALSE, if |
	| old type of field edit.     |
	-----------------------------*/
	first_time = (envScnEdit) ? TRUE : FALSE;

	if (P_TAB)
	{
		irow = tab_row + 2 + (line_cnt % PV_tlines);
		icol = FIELD.col;
	}
	else
	{
		irow = FIELD.row;
		if (TruePosition)
			icol = FIELD.col + strlen (FIELD.prmpt);
		else
			icol = FIELD.col + 3;
	}

	/*--------------------------------------------- 
	| Remove trailing spaces from previous value. |
	---------------------------------------------*/ 
	strcpy (prv_ntry,clip (temp_str));
	ml = strlen (FIELD.mask);

	for (ptr = 0; ptr < ml; ptr++)
		if (FIELD.mask[ptr] == ',')
			no_commas++;
	move (icol, irow);
	printf ("%-*.*s", no_commas, no_commas, " ");
	icol += no_commas;

	/*------------------------
	| Ensure correct length. |
	------------------------*/
	temp_str[ml] = '\0';

	if (FIELD.type != TIMETYPE && envScnEdit)
	{
		if ((FIELD.type == FLOATTYPE && * (VAR_PTR.fptr) == (float)0.00) ||
		   (FIELD.type == INTTYPE && * (VAR_PTR.iptr) == (int)0) ||
		   (FIELD.type == LONGTYPE && * (VAR_PTR.lptr) == 0L) ||
		   (FIELD.type == EDATETYPE && * (VAR_PTR.lptr) <= 0L) ||
		   (FIELD.type == DOUBLETYPE && * (VAR_PTR.dptr) == (double)0.00) ||
		   (FIELD.type == MONEYTYPE && * (VAR_PTR.dptr) == (double)0.00))
		{
			/*----------------------------------------------
			| Don't display field if numeric field is zero |
			----------------------------------------------*/
			sprintf (temp_str,"%-*.*s",ml,ml," "); 
			temp_str[ml] = '\0';
			strncpy (prv_ntry,temp_str,ml); 
			max_inpt = 0;
			dec_pt = FALSE;         /*  so dec_pt allowed in input      */
		}
		else
		{
			/*------------------------
			| Display previous field |
			------------------------*/
			strncpy (temp_str,prv_ntry,ml); 
			temp_str[ml] = '\0';
			printf (temp_str);
			max_inpt = strlen (clip (temp_str));
			dec_pt = TRUE;          /*  so no dec_pt allowed in input	*/
		}
	}

	/*-----------------------------------------------
	| Display 'US' for length of field upon screen. |
	-----------------------------------------------*/ 
	for (ptr = 0; ptr < ml && FIELD.type != TIMETYPE; ptr++) 
	{
		/*----------------------------
		| Set flag on encountering . |
		----------------------------*/
		if (FIELD.mask[ptr] == '.')
			dec_flag = TRUE;

		/*---------------------
		| If lead digit incr .|
		---------------------*/
		if (!dec_flag && FIELD.mask[ptr] != ',')
			var_lead++;

		/*--------------------------------
		| Print 'US' etc. after temp_str |        
		--------------------------------*/
		if (ptr >= max_inpt)
		{
			/*-----------------------------------
			| Check for special case when comma |
			| format field is already displayed |
			-----------------------------------*/
			if (! (no_commas > 0 && max_inpt > 0)) 
			{
				if (strchr (mask_chrs,FIELD.mask[ptr]) != (char *) NULL)
					putchar (US);
				else
					if (FIELD.mask[ptr] != ',')
						putchar (FIELD.mask[ptr]);
			}
		}
	}

	if (FIELD.type == TIMETYPE)
	{
		sprintf (temp_str, "%s", ttoa (0L, FIELD.mask));
		move (icol, irow);
		printf ("%s%c", temp_str, '\b');
	}
	else
		for (ptr = ml; ptr > max_inpt ; ptr--)
			if (FIELD.mask[ptr] != ',')
			{
				if (! (no_commas > 0 && max_inpt > 0)) 
					putchar (BS);
			}

	fflush (stdout);
	crsr_on ();

	/*----------------------------
	| Set no of trailing digits. |
	----------------------------*/
	if (var_lead != ml)
		var_trail = ml - var_lead + 1;
	
	if (FIELD.type == TIMETYPE || !envScnEdit)
		ptr = 0;

	while (cont && (last_char = getkey ()) != '\n' && last_char != '\r' && 
		         last_char != RESTART && last_char != EOI)
	{
		if (prog_status == ENTRY && up_ok && 
	 	    (_KEY_UP (last_char) || _KEY_DOWN (last_char)))
		{
			crsr_off ();
			return (EXIT_FAILURE);
		}

		if (P_TAB && _KEY_FN13 (last_char))
		{
			crsr_off ();
			return (EXIT_FAILURE);
		}

		if ((last_char == INSLINE || last_char == DELLINE) &&
			prog_status != ENTRY && P_TAB)
		{
			crsr_off ();
			break;
		}
		
		recall = 0;	/* Reset for every entry */

		if (last_char == FN_HELP || last_char == HELP || 
		     last_char == EXIT_OUT || last_char == HELP1 ||  
		    (last_char > FN16 && last_char < FN32))
		{
			crsr_off ();
			shell_prog (last_char);
			last_char = REDRAW;
		}

		if (last_char == FN14 || last_char == FN15 || last_char == FN5)
		{
			crsr_off ();
			if (last_char == FN5)
				psl_print ();
			else
			{
				if (PV_use_win)
					 (*PV_use_win) (last_char);
			}
			if (P_TAB)
			{
				lrow = tab_row + 2 + (line_cnt % PV_tlines);
				move (icol,lrow);
			}
			else
				move (icol,FIELD.row);
			printf (string (strlen (FIELD.mask), "_"));

			if (P_TAB)
			{
				lrow = tab_row + 2 + (line_cnt % PV_tlines);
				move (icol,lrow);
			}
			else
				move (icol,FIELD.row);

			if (FIELD.type == TIMETYPE || !envScnEdit)
				temp_str[ptr] = (char) NULL;
			else
				ptr = max_inpt = strlen (clip (temp_str));
			printf (temp_str);
			crsr_on ();
			continue;
		}

		if ((last_char == FN4 || last_char == FN9 || 
		      last_char == FN10 || last_char == FN11 || 
		      last_char == FN12) && search_ok) 
		{
			if (FIELD.type == EDATETYPE && stopDateSearch == FALSE)
			{
				pslscr_rslt_date	=	cal_select 
										 (
											0,
											0,
											5,
											1,
											pslscr_curr_day,
											pslscr_st_ptr,
											pslscr_st_info,
											C_NORM
										);
				if (pslscr_rslt_date)
					strcpy (temp_str, DateToString (pslscr_rslt_date));
			}
			crsr_off ();
			search_key = last_char;
			last_char = SEARCH;
			if (FIELD.type == TIMETYPE)
				temp_str[ptr] = (char) NULL;
			else
			{
				max_inpt = strlen (clip (temp_str));
				temp_str[max_inpt] = (char) NULL;
			}
			if (FIELD.type != EDATETYPE || stopDateSearch == TRUE)
				spec_valid (field);

			if (strlen (clip (temp_str)) == 0)
				sprintf (temp_str,"%-*.*s",ml,ml," "); 

			temp_str[ml] = (char) NULL;
			max_inpt = strlen (clip (temp_str));
			ptr = max_inpt;
			if (last_char == SEARCH)
				last_char = REDRAW;
		}

		if (last_char == PSLW || last_char == PSLW2 || last_char == PSLW3 ||
		    last_char == PSLW4 || last_char == PSLW5)
		{
			if (_win_func)
			{
				switch (last_char)
				{
				case PSLW:
					win_function (cur_field, line_cnt, cur_screen, ENTRY);
					break;
				case PSLW2:
					win_function2 (cur_field, line_cnt, cur_screen, ENTRY);
					break;
				case PSLW3:
					win_function3 (cur_field, line_cnt, cur_screen, ENTRY);
					break;
				case PSLW4:
					win_function4 (cur_field, line_cnt, cur_screen, ENTRY);
					break;
				case PSLW5:
					win_function5 (cur_field, line_cnt, cur_screen, ENTRY);
					break;
				}	

				last_char = REDRAW;
				winFuncCall = TRUE;
			}
		}
		/*----------------------------------------------------
		| Redraw key pressed - redraw screen and reposition. |
		----------------------------------------------------*/
		if (last_char == REDRAW) 
		{
			crsr_off ();
			if (pslw_asl && winFuncCall)
			{
				scn_write (cur_screen);
				sr_lcnt = line_cnt;
				winFuncCall = FALSE;
			}
			else
			{
				sr_lcnt = line_cnt;
				heading (cur_screen);
				line_cnt = sr_lcnt;
			}

			/*-----------------
			| Tabular Input . |
			-----------------*/
			if (P_TAB) 
			{
				base_line = (line_cnt / PV_tlines) * PV_tlines;
				scn_page = line_cnt / PV_tlines;
				putval (line_cnt);

				if (prog_status != ENTRY || init_ok == 0)
				{
					for (line_cnt = base_line; (line_cnt/PV_tlines == scn_page) && line_cnt < lcount[cur_screen];line_cnt++) 
					{
						getval (line_cnt);
						line_display ();
					}
					line_cnt = sr_lcnt;
					getval (line_cnt);
				}
				else  
				{
					for (line_cnt=base_line ;line_cnt <= sr_lcnt;line_cnt++)
					{
						getval (line_cnt);
						line_display ();
					}
				}
				line_cnt = sr_lcnt;
				line_display ();
				lrow = tab_row + 2 + (line_cnt % PV_tlines);
				irow = tab_row + 2 + (line_cnt % PV_tlines);
				move (FIELD.col,lrow);
			}
			/*----------------
			| Linear Input . |
			----------------*/
			else  
			{
				scn_display (cur_screen);
				if (TruePosition)
					move (FIELD.col + strlen (FIELD.prmpt),FIELD.row);
				else
					move (FIELD.col+3,FIELD.row);
			}
			if (FIELD.type == TIMETYPE)
				printf ("%s\b", temp_str);
			else
			{
				printf ("%-*.*s", no_commas, no_commas, " ");
				if (!envScnEdit)
					temp_str[ptr] = (char) NULL;
				else
					max_inpt = strlen (clip (temp_str));
					
				printf (temp_str);

				for (	t_ptr = x_ptr = 0; 
						x_ptr < ml && t_ptr < ((envScnEdit) ? max_inpt : ptr);
						x_ptr++)
				{
					if (FIELD.mask[x_ptr] != ',')
						t_ptr++;
				}
				t_ptr = x_ptr;
				for (; t_ptr < ml; t_ptr++) 
				{
					if (strchr (mask_chrs,FIELD.mask[t_ptr]) != (char *) NULL)
						putchar (US);
					else
						if (FIELD.mask[t_ptr] != ',')
							putchar (FIELD.mask[t_ptr]);
				}

				move (icol + ptr, irow);
			}
			fflush (stdout);
			crsr_on ();
		}

		if (last_char == RECALL) 
		{
		    if (FIELD.type == TIMETYPE)
		    {
				strcpy (temp_str, prv_ntry); 
				move (icol, irow);
				printf ("%s%c", temp_str, '\b');
				ptr = strlen (temp_str);
				for (iptr = 0; iptr < ptr; iptr++)
				{
					if (temp_str[iptr] == ':')
					{
						dec_flag = TRUE;
						continue;
					}
					if (temp_str[iptr] > '0')
					{
						max_inpt = ptr - iptr;
						max_inpt -= (dec_flag == FALSE) ? 1 : 0;
						break;
					}
				}
				continue;
		    }
		    else
		    {
				crsr_off ();

				move (icol, irow);
				/*-----------------------------------------------
				| Display 'US' for length of field upon screen. |
				-----------------------------------------------*/ 
				for (recall = 0; recall < ml; recall++) 
				{
					if (strchr (mask_chrs,FIELD.mask[recall]) != (char *) NULL)
						putchar (US);
					else
					{
						if (FIELD.mask[recall] != ',')
							putchar (FIELD.mask[recall]);
					}
				}
				move (icol, irow);

				/*---------------------------------------
				| Toggle between recall & clear a field |
				---------------------------------------*/
				if (((int)strlen (clip (temp_str)) == 0 && 
					 (int)strlen (clip (prv_ntry)) != 0) || 
					!envScnEdit)
				{
					strncpy (temp_str,prv_ntry,ml); 
					temp_str[ml] = '\0';
					printf (temp_str);
					max_inpt = strlen (temp_str);
					ptr = max_inpt;
					dec_pt = TRUE;	/*  so no dec_pt allowed in input	*/
				}
				else
				{
					sprintf (temp_str,"%-*.*s",ml,ml," "); 
					max_inpt = ptr = 0;
					temp_str[max_inpt] = '\0';
					dec_pt = FALSE; /*  so dec_pt allowed in input      */
				}

				recall = 1;
				crsr_on ();
				continue;
			}
		}

		if (last_char > FN32)
		{
			putchar (BELL);
			continue;
		}

		crsr_on ();
		if (FIELD.type == CHARTYPE || FIELD.type == EDATETYPE)
		{
			/*  Check for non_mask chars at start	*/
			while (strchr (mask_chrs,FIELD.mask[ptr]) == (char *) NULL) 
			{ 
				putchar (FIELD.mask[ptr]);
				temp_str[ptr] = FIELD.mask[ptr];
				ptr++;
				if (envScnEdit && !insert_mode && last_char != REDRAW && 
					last_char != DELCHAR && last_char != '\b' && 
					last_char != LEFT_KEY && last_char != RIGHT_KEY)
				{
					if (max_inpt < ml && ptr > max_inpt)
						max_inpt++;
				}
			}

			switch (last_char)
			{
			case	REDRAW :
				break;

			case	INSCHAR :   /* toggle between insert & overwrite mode */
				if (!envScnEdit)
				{
					putchar (BELL);
					break;
				}
				insert_mode = (insert_mode) ? FALSE : TRUE;
				break;
				
			case	DELCHAR :   /* delete one character */
				if (!envScnEdit)
				{
					putchar (BELL);
					break;
				}

				if (max_inpt > 0 && ptr < max_inpt)
				{
					for (iptr = ptr; iptr < max_inpt; iptr++)
					{
						if (strchr (mask_chrs,FIELD.mask[iptr]) ==
								 (char *) NULL)
							continue;
						t_ptr = iptr;
						while (TRUE)
						{
							if (strchr (mask_chrs,FIELD.mask[t_ptr + 1]) ==
									 (char *) NULL)
								t_ptr++;
							else
							{
								temp_str[iptr] = temp_str[t_ptr + 1];
								break;
							}
						}
					}
					temp_str[max_inpt] = '\0';
					max_inpt--;
					crsr_off ();
					for (iptr = ptr; iptr < max_inpt; iptr++) 
					{
						if (strchr (mask_chrs,FIELD.mask[iptr]) != 
								 (char *) NULL)
						{	
							if (FIELD.mask[iptr] == '_')
								putchar (FIELD.mask[iptr]);
							else
								putchar (temp_str[iptr]);
						}
						else
							putchar (FIELD.mask[iptr]);
					}

					if (strchr (mask_chrs,FIELD.mask[max_inpt]) == 
							 (char *) NULL)
					{
						if (max_inpt)
							max_inpt--;
					}
					putchar (US);
					move (icol + ptr, irow);
					crsr_on ();
				}
				else
					putchar (BELL);
				break;

			case	LEFT_KEY : /*  check for non-mask chars	*/
				if (!envScnEdit)
				{
					putchar (BELL);
					break;
				}

				while (ptr > 0 && (strchr (mask_chrs,FIELD.mask[ptr - 1]) == (char *) NULL)) 
				{ 
					putchar ('\b');
					putchar (FIELD.mask[ptr - 1]);
					putchar ('\b');
					ptr--;
				}
				if (ptr > 0) 
				{
					putchar ('\b');
					ptr--;
					first_time = FALSE;
				}
				else
					putchar (BELL);
				break;

			case	RIGHT_KEY : /*  check for non-mask chars	*/
				if (!envScnEdit)
				{
					putchar (BELL);
					break;
				}

				while (ptr < max_inpt && (strchr (mask_chrs,FIELD.mask[ptr + 1]) == (char *) NULL)) 
				{ 
					ptr++;
					move (icol + ptr, irow);
					putchar (FIELD.mask[ptr]);
				}
				if (ptr < max_inpt) 
				{
					ptr++;
					move (icol + ptr, irow);
				}
				else
					putchar (BELL);
				break;

			case	'\b' : /*  check for non-mask chars	*/
				if (ptr == max_inpt || !envScnEdit) /* '/b' from end of string */
				{
					while (ptr > 0 && (strchr (mask_chrs,FIELD.mask[ptr - 1]) == (char *) NULL)) 
					{ 
						putchar ('\b');
						putchar (FIELD.mask[ptr - 1]);
						putchar ('\b');
						ptr--;
						if (max_inpt)
							max_inpt--;
						temp_str[ptr] = '\0';
					}
					if (ptr > 0) 
					{
						putchar ('\b');
						putchar (US);
						putchar ('\b');
						ptr--;
						if (max_inpt)
							max_inpt--;
						temp_str[ptr] = '\0';	/*  terminate */
						first_time = FALSE;
					}
					else
						putchar (BELL);
				}
				else						/* '/b' from within string */
				{
					while (ptr > 0 && (strchr (mask_chrs,FIELD.mask[ptr - 1]) ==
							 (char *) NULL)) 
					{ 
						putchar ('\b');
						ptr--;
					}
					if (ptr > 0) 
					{
						putchar ('\b');
						ptr--;
						for (iptr = ptr; iptr < max_inpt ;iptr++)
						{
							if (strchr (mask_chrs,FIELD.mask[iptr]) ==
									 (char *) NULL)
								continue;
							t_ptr = iptr;
							while (TRUE)
							{
								if (strchr (mask_chrs,FIELD.mask[t_ptr + 1]) ==
										 (char *) NULL)
									t_ptr++;
								else
								{
									temp_str[iptr] = temp_str[t_ptr + 1];
									break;
								}
							}
						}
						temp_str[max_inpt] = '\0';

						crsr_off ();
						if (max_inpt)
							max_inpt--;
						for (iptr = ptr; iptr < max_inpt; iptr++) 
						{
							if (strchr (mask_chrs,FIELD.mask[iptr]) != 
									 (char *) NULL)
							{
								if (FIELD.mask[iptr] == '_')
									putchar (FIELD.mask[iptr]);
								else
									putchar (temp_str[iptr]);
							}
							else
								putchar (FIELD.mask[iptr]);
						}
						if (strchr (mask_chrs,FIELD.mask[max_inpt]) == 
								 (char *) NULL)
						{
							if (max_inpt)
								max_inpt--;
						}

						putchar (US);

						move (icol + ptr, irow);
						crsr_on ();
					}
					else
						putchar (BELL);
				}
				break;

			default :
				if (ptr >= ml)
				{
					putchar (BELL);
					break;
				}
				if (FIELD.mask[ptr] == 'N' && last_char < ASCII_RANGE && !isdigit (last_char) && !OUTRANGE_CHAR)
				{
					putchar (BELL);
					break;
				}
				if (FIELD.mask[ptr] == 'U' && last_char < ASCII_RANGE && islower (last_char) && !OUTRANGE_CHAR)
					last_char = toupper (last_char);

				if (FIELD.mask[ptr] == 'L' && last_char < ASCII_RANGE && isupper (last_char) && !OUTRANGE_CHAR)
					last_char = tolower (last_char);

				if (OUTRANGE_CHAR || FIELD.mask[ptr] == 'S' ||
					 ((strchr (validchrs,last_char) != (char *) NULL) && 
					 (ptr < ml || (first_time && FIELD.type == EDATETYPE))))
				{
					if (first_time && FIELD.type == EDATETYPE)
					{
						if (ptr > 0)
						{
							for (first_time = 0; first_time < ml; first_time++)
							{
								putchar ('\b');
								if (strchr (mask_chrs,FIELD.mask[ptr - 1]) 
											!= (char *) NULL)
									putchar (US);
								else
									if (FIELD.mask[ptr] != ',')
										putchar (FIELD.mask[ptr - 1]);
								putchar ('\b');
								temp_str[ptr] = ' ';
								ptr --;
							}
							temp_str[ml] = '\0';
							max_inpt = ptr = 0;
						}
						first_time = FALSE;
					}

					if (insert_mode)  			    /* insert char mode */
					{
						max_inpt++;
						if (max_inpt > ml)           /* truncate string  */
							max_inpt--;
						for (iptr = max_inpt; iptr > ptr; iptr--)
						{
							if (strchr (mask_chrs,FIELD.mask[iptr]) ==
								 (char *) NULL && iptr != ml)
								continue;
							t_ptr = iptr;
							while (TRUE)
							{
								if (strchr (mask_chrs,FIELD.mask[t_ptr - 1]) ==
										 (char *) NULL)
									t_ptr--;
								else
							{
									temp_str[iptr] = temp_str[t_ptr - 1];
									break;
								}
							}
						}
						temp_str[ptr] = last_char;

						crsr_off ();
						for (iptr = ptr; iptr < max_inpt; iptr++) 
						{
							if (strchr (mask_chrs,FIELD.mask[iptr]) != 
									 (char *) NULL)
							{
								if (FIELD.mask[iptr] == '_')
									putchar (FIELD.mask[iptr]);
								else
									putchar (temp_str[iptr]);
							}
							else
								putchar (FIELD.mask[iptr]);
						}

						ptr++;
						move (icol + ptr, irow);
						crsr_on ();
						if (strchr (mask_chrs,FIELD.mask[max_inpt]) ==
								 (char *) NULL)
						{
							temp_str[max_inpt] = FIELD.mask[max_inpt];
							max_inpt++;
						}
						temp_str[max_inpt] = '\0';
					}
					else
					{                                /* overwrite char mode */
						if (FIELD.mask[ptr] == '_')
							putchar (FIELD.mask[ptr]);
						else
							putchar (last_char);
						temp_str[ptr] = last_char;
						ptr++;
						if (ptr > max_inpt)
							max_inpt++;
					}
				}
				else
					putchar (BELL);
				break;
			}	/* end of switch on last_char	*/
			/*-----------------------------------------------
			|  check for non_mask chars at current ptr pos. |
			-----------------------------------------------*/
			while (strchr (mask_chrs,FIELD.mask[ptr]) == (char *) NULL) 
			{ 
				putchar (FIELD.mask[ptr]);
				temp_str[ptr] = FIELD.mask[ptr];
				ptr++;
				if ((envScnEdit && !insert_mode && last_char != REDRAW && 
					last_char != DELCHAR && last_char != RIGHT_KEY) ||
					 (insert_mode && (last_char == '\b' || 
					last_char == LEFT_KEY)))
				{
					if (max_inpt < ml && ptr > max_inpt)
						max_inpt++;
				}
			}
			/*  end of get_alpha	*/
		}
		if (FIELD.type == TIMETYPE)
		{
			if (last_char == '.')
				last_char = ':';
			switch (last_char)
			{
			case	'\b' :
			    if (max_inpt)
				max_inpt--;
			    ptr = strlen (temp_str) - 1;
			    temp_str[ptr - 2] = temp_str[ptr - 3];
			    temp_str[ptr - 3] = ':';
			    if (temp_str[ptr - 4] == ' ')
					temp_str[ptr - 4] = '0';
			    while (ptr > 0)
			    {
					temp_str[ptr] = temp_str[ptr - 1];
					ptr--;
			    }
			    temp_str[0] = ' ';
			    move (icol, irow);
			    printf ("%s%c", temp_str, '\b');
			    break;

			default :
			    if (strchr (validchrs,last_char) != (char *) NULL && temp_str[0] == ' ')
			    {
				ptr = strlen (temp_str);
				if (last_char != ':')
				{
				    max_inpt++;
				    strcat (temp_str, "0");
				    temp_str[ptr] = (char)last_char;
				    temp_str[ptr - 3] = temp_str[ptr - 2];
				    temp_str[ptr - 2] = ':';
				    strcpy (temp_str, &temp_str[1]);
				    iptr = 0;
				    while (temp_str[iptr] == ' ')
					iptr++;
				    if (temp_str[iptr] == '0')
					temp_str[iptr] = ' ';
				    move (icol, irow);
				    printf ("%s%c", temp_str, '\b');
				}
				else
				{
				    if (temp_str[1] == ' ' ||temp_str[1] == '0')
				    {
					max_inpt += 2;
					strcat (temp_str, ":00");
					strcpy (&temp_str[ptr-3], &temp_str[ptr-2]);
					strcpy (temp_str, &temp_str[2]);
					iptr = 0;
					while (temp_str[iptr] == ' ')
					    iptr++;
					/*-------------------------------
					| The follwing line is to	|
					| suppress leading zeros.	|
					-------------------------------*/
					sprintf (temp_str, "%s", ttoa (atot (temp_str), FIELD.mask));
					move (icol, irow);
					printf ("%s%c", temp_str, '\b');
				    }
				    else
					putchar (BELL);
				}
			    }
			    else
				putchar (BELL);

			    break;
			}	/*  end of switch	*/
		}
		if (FIELD.type != CHARTYPE && FIELD.type != EDATETYPE && FIELD.type != TIMETYPE)
		{	/*  numeric input	*/
			switch (last_char)
			{
			case	REDRAW :
				break;

			case	'\b' :
				if (ptr > 0) 
				{
					if (temp_str[ptr - 1] == (char) '.')
						dec_pt = FALSE;

					putchar ('\b');
					putchar (US);
					putchar ('\b');
					temp_str[ptr] = ' ';
					ptr--;
					if (max_inpt)
						max_inpt--;
					temp_str[ptr] = '\0';/*  terminate*/
					first_time = FALSE;
				}
				else
					putchar (BELL);
				break;

			default :
				if (strchr (validchrs,last_char) != (char *) NULL && (ptr < (ml - no_commas) || first_time) && ((ptr < var_lead) || (last_char == '.' && var_trail) || dec_pt))
				{
					if ((last_char != '.' || first_time) ||
							 (!dec_pt && last_char == '.')) 
					{
		    	     	/*---------------------------------
						| Allow a new numeric to be input |
						| before the field is edited.     |
						---------------------------------*/
						if (first_time)
						{
							if (ptr > 0)
							{
								move (icol, irow);
								for (first_time = 0; 
									first_time < ml; 
									first_time++)
								{
									if (strchr (mask_chrs,FIELD.mask[first_time])
											!= (char *) NULL)
										putchar (US);
									else
										if (FIELD.mask[first_time] != ',')
											putchar (FIELD.mask[first_time]);
								}
								move (icol, irow);
								sprintf (temp_str,"%-*.*s",ml,ml," "); 
								temp_str[ml] = '\0';
								max_inpt = ptr = 0;
								dec_pt = FALSE; /* so dec_pt allowed in input */
							}
							first_time = FALSE;
						}
						if (last_char == '.')
							dec_pt = TRUE;
						putchar (last_char);
						temp_str[ptr] = (char)last_char;
						ptr++;
						max_inpt++;
					}
					else
						putchar (BELL);
				}
				else
					putchar (BELL);

				break;
			}	/*  end of switch	*/
			/*  end of get_numeric	*/
		}

		if ((FIELD.type == TIMETYPE || !envScnEdit) && ptr)		/*  terminate input if changed only	*/
			temp_str[ptr] = '\0';

		/*  check for end of field and auto next flag for field	*/
		if (ptr == ml && FIELD.autonext)
			cont = FALSE;

		fflush (stdout);
	}	/*  end of while	*/

	crsr_off ();
	restart = (last_char == RESTART);	/* initially cntrl B	*/
	prog_exit = (last_char == EOI && field == scn_start && line_cnt < 1 && cur_screen == 1);
	entry_exit = (last_char == EOI && field == scn_start && P_TAB);
	end_input = (last_char == EOI);

	if (envScnEdit && FIELD.type != TIMETYPE)
		temp_str[max_inpt] = '\0';          /* terminate */

	if (restart || prog_exit || entry_exit || end_input) 
		return (EXIT_FAILURE);
	else
		return (max_inpt);
}

/*=====================================
| First line validation is done here. |
=====================================*/
int
invalid_entry (int field)
{
	int	invalid = 0;
	int	tint = 0;
	long	tlong = 0;
	float	tfloat = 0;
	double	tdouble = 0;
	char	temptime[30];
	char	msg[80];
	char	fixDateString[MAXSTR + 1];

	crsr_off ();
	switch (FIELD.type) 
	{
	case	CHARTYPE :
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			tint = atoi (temp_str);
			if (tint < atoi (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tint > atoi (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	INTTYPE :
		tint = atoi (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tint < atoi (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tint > atoi (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	LONGTYPE :
		tlong = atol (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tlong < atol (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tlong > atol (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	FLOATTYPE :
		tfloat = (float) atof (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tfloat < (float) atof (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tfloat > (float) atof (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	DOUBLETYPE :
		tdouble = atof (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tdouble < atof (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tdouble > atof (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	MONEYTYPE :	/*  check in CENTS !!!	*/
		tdouble = atof (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tdouble < atof (FIELD.lowval)) 
			{
				sprintf (msg,"%s (%s)",below,FIELD.lowval);
				errmess (msg);
				invalid = TRUE;
			}

			if (tdouble > atof (FIELD.highval)) 
			{
				sprintf (msg,"%s (%s)",above,FIELD.highval);
				errmess (msg);
				invalid = TRUE;
			}
		}
		break;

	case	EDATETYPE :
		strcpy (fixDateString, temp_str);
		if (StringToDate (fixDateString) < 0L) 
		{
			sprintf (msg,"%s (%s)","Not a Valid Date",fixDateString);
			errmess (msg);
			invalid = TRUE;
			if (envScnEdit)
				* (VAR_PTR.lptr) = 0L;
		}
		break;

	case	TIMETYPE :
		tlong = atot (temp_str);
		if (strlen (FIELD.lowval) && strlen (FIELD.highval)) 
		{
			if (tlong < atol (FIELD.lowval)) 
			{
				sprintf (temptime, "%s", ttoa (atol (FIELD.lowval), FIELD.mask));
				sprintf (msg,"%s (%s)",below,temptime);
				errmess (msg);
				invalid = TRUE;
			}

			if (tlong > atol (FIELD.highval)) 
			{
				sprintf (temptime, "%s", ttoa (atol (FIELD.highval), FIELD.mask));
				sprintf (msg,"%s (%s)",above,temptime);
				errmess (msg);
				invalid = TRUE;
			}
		}
		if (!invalid)
		{
			sprintf (temptime, "%s", ttoa (tlong, FIELD.mask));
			if (strlen (temptime) > strlen (FIELD.mask))
			{
				errmess ("Exceeds Maximum!!");
				invalid = TRUE;
			}
		}
		break;

	default :
		/*  no action for sundry types	*/
		break;
	}
	if (invalid)
		sleep (sleepTime);
	return (invalid);
}

/*========================================================
| Routines to move variable values between local storage |
| and the screen editing varaible.                       |
========================================================*/
int
getval (
	int	line_no)
{
	if (P_TXT)
	{
		strcpy (vars[scn_start].tcptr, txt_gval (vars[scn_start].type - 1, line_no + 1));
		return (EXIT_SUCCESS);
	}

	return (_getval (line_no + 1));
}

int
_getval (
	int	line_no)
{
	char	tempdate[11];
	char	temptime[32];
	char	*sptr;
	int	eptr;
	int	field;
	int	no_commas;

	sptr = lstore[line_no + ((cur_screen - 1) * PV_mlines)];
	if (sptr == (char *) NULL)
		return (EXIT_SUCCESS);

	for (field = scn_start;field < scn_start + nbr_fields;field++) 
	{
		eptr = strlen (FIELD.mask);
		switch (FIELD.type) 
		{
		case	CHARTYPE :
			strcpy (VAR_PTR.cptr,mid (sptr,1,eptr));
			break;

		case	INTTYPE :
			* (VAR_PTR.iptr) = atoi (mid (sptr,1,eptr));
			break;

		case	LONGTYPE :
			* (VAR_PTR.lptr) = atol (mid (sptr,1,eptr));
			break;

		case	FLOATTYPE :
			for (no_commas = eptr; no_commas > 0; no_commas--)
				if (FIELD.mask[no_commas - 1] == ',')
					eptr--;
			* (VAR_PTR.fptr) = (float) atof (mid (sptr,1,eptr));
			break;

		case	DOUBLETYPE :
		case	MONEYTYPE :
			for (no_commas = eptr; no_commas > 0; no_commas--)
				if (FIELD.mask[no_commas - 1] == ',')
					eptr--;
			* (VAR_PTR.dptr) = atof (mid (sptr,1,eptr));
			break;

		case	EDATETYPE :
			strcpy (tempdate,mid (sptr,1,eptr));
			* (VAR_PTR.lptr) = StringToDate (tempdate);
			break;

		case	TIMETYPE :
			strcpy (temptime,mid (sptr,1,eptr));
			* (VAR_PTR.lptr) = atot (temptime);
			break;
		}
		sptr += eptr;
	}
	return (EXIT_SUCCESS);
}

int
putval (int	line_no)
{
	if (P_TXT)
	{
		txt_pval (vars[scn_start].type - 1, vars[scn_start].tcptr, line_no + 1);
		return (EXIT_SUCCESS);
	}

	return (_putval (line_no + 1));
}

int
_putval (int	line_no)
{
	int	field = 0;
	int	i;
	char	temptime[32];
	char	temp_val[256];
	char	temp_local[2048];

	temp_local[0] = (char) '\0';	/*  reset temp_local string	*/
	for (field = scn_start; field < scn_start + nbr_fields; field++) 
	{
		switch (FIELD.type) 
		{
		case	CHARTYPE :
			sprintf (temp_val,FIELD.pmask,VAR_PTR.cptr);
			break;

		case	INTTYPE :
			sprintf (temp_val,FIELD.pmask,* (VAR_PTR.iptr));
			break;

		case	LONGTYPE :
			sprintf (temp_val,FIELD.pmask,* (VAR_PTR.lptr));
			break;

		case	FLOATTYPE :
			sprintf (temp_val,FIELD.pmask, *(VAR_PTR.fptr));
			break;

		case	DOUBLETYPE :
			sprintf (temp_val,FIELD.pmask, *(VAR_PTR.dptr));
			break;

		case	MONEYTYPE :  
			for (i=0;FIELD.pmask[i]!='.';i++);
			i++;
			FIELD.pmask[i] = '0';
			sprintf (temp_val,FIELD.pmask, *(VAR_PTR.dptr));
			FIELD.pmask[i] = '2';
			break;

		case	EDATETYPE :
			sprintf (temp_val,FIELD.pmask,DateToString (* (VAR_PTR.lptr)));
			break;

		case	TIMETYPE :
			sprintf (temptime, "%s", ttoa (* (VAR_PTR.lptr), FIELD.mask));
			i = strlen (temptime) - strlen (FIELD.mask);
			strcpy (temp_val, &temptime[i]);
			break;
		}
		strcat (temp_local,temp_val);
	}

	if (lstore[line_no + ((cur_screen - 1) * PV_mlines)] != (char *) 0)
		free (lstore[line_no + ((cur_screen - 1) * PV_mlines)]);

	lstore[line_no + ((cur_screen - 1) * PV_mlines)] = p_strsave (temp_local);

	if (lstore[line_no + ((cur_screen - 1) * PV_mlines)] == (char *) 0)
		file_err (12, "Fatal System Error: Cannot Allocate Buffers.", PNAME);

	return (EXIT_SUCCESS);
}

/*================================================================
| Generate the mask used for structure transfers and for echoing | 
| field value to the operator.                                   |
================================================================*/
void
set_masks (void)
{
	char	file_name[31];

	sprintf (file_name, "%-*.*s.s", (int) strlen (PNAME) - 2, 
									(int) strlen (PNAME) - 2, PNAME);
	
	_set_masks (file_name);
}
/*================================================================
| Generate the mask used for structure transfers and for echoing | 
| field value to the operator.                                   |
================================================================*/
void
_set_masks (char	*scn_rfile)
{
	int	field = 0;
	int	c_scn = 0;	/* current screen number		*/

	/*-------------------------------------------
	| Check if text file exists for program and |
	| set fields as per text file.              |
	-------------------------------------------*/
	screen_set (scn_rfile, vars);

	for (c_scn = 0;c_scn < PV_scns;c_scn++)
	{
		tab_data[c_scn]._desc = (char *)0;
		tab_data[c_scn]._width = 1;
		tab_data[c_scn]._scn = 0;
		tab_data[c_scn]._row = 0;
		tab_data[c_scn]._win = 1;
	}
	c_scn = -1;

	while (FIELD.scn)
	{
		/*-------------------------------
		| Sum Only For Same Screen	|
		-------------------------------*/
		if (c_scn == - 1 || tab_data[c_scn]._scn != FIELD.scn)
			tab_data[++c_scn]._scn = FIELD.scn;
		/*-------------------------------
		| Sum Only For Tab Screen	|
		-------------------------------*/
		if (FIELD.stype == TAB)
			tab_data[c_scn]._width += strlen (FIELD.mask);

		if (tab_data[c_scn]._width > PV_mwidth)
		{
			sprintf (err_str,"(MAXWIDTH)%d < %d",PV_mwidth,tab_data[c_scn]._width);
			file_err (-1, err_str,PNAME);
		}

		switch (FIELD.type)
		{
		case	CHARTYPE :
			set_prnt_mask (field,"s");
			VAR_PTR.cptr = FIELD.tcptr;
			break;

		case	INTTYPE :
			set_prnt_mask (field,"d");
			VAR_PTR.iptr = (int *) FIELD.tcptr;
			break;

		case	LONGTYPE :
			set_prnt_mask (field,"ld");
			VAR_PTR.lptr = (long *) FIELD.tcptr;
			break;

		case	FLOATTYPE :
			set_prnt_mask (field,"f");
			VAR_PTR.fptr = (float *) FIELD.tcptr;
			break;

		case	DOUBLETYPE :
		case	MONEYTYPE :
			set_prnt_mask (field,"f");
			VAR_PTR.dptr = (double *) FIELD.tcptr;
			break;

		case	EDATETYPE :
			set_prnt_mask (field,"s");
			VAR_PTR.lptr = (long *) FIELD.tcptr;
			break;

		case	TIMETYPE :
			VAR_PTR.lptr = (long *) FIELD.tcptr;
			break;

		default :
			sprintf (err_str,"Unrecognised field type for %d\n",field);
			file_err (-1, err_str,PNAME);
			break;
		}
		field++;

		if (c_scn >= PV_scns)
		{
			sprintf (err_str," Screen No exceeds %d (MAXSCNS)",PV_scns);
			file_err (-1, err_str,PNAME);
		}
	}
}

/*===========================================================
| This routine sets the print using mask for a given field. |
===========================================================*/
void
set_prnt_mask (
	int		field,
	char	*pc)
{
	int	ptr = 0;
	int	ptr1 = 0;
	int	vlen = 0;
	int	no_commas = 0;
	char	*sptr = getenv ("DBDATE");
	char	prnt_mask[16];
	char	sp_mask[10];	/* Holds special types - left just/zero fill */

	/*--------------------------
	| User defined print mask. |
	--------------------------*/
	if (* (FIELD.pmask) == 'U') 
	{
		FIELD.pmask++;	
		return;
	}

	/*---------------------------------
	| Check Date mask within Program. |
	---------------------------------*/
	if (FIELD.type == EDATETYPE)
	{
		/*--------------------------------
		| Check for four character date. |
		--------------------------------*/
		if ((strchr (sptr, '4') != (char *) 0))
		{
			if (sptr[0] == 'Y')
				FIELD.mask = p_strsave ("DDDD/DD/DD");
			else	
				FIELD.mask = p_strsave ("DD/DD/DDDD");
		}
		else
			FIELD.mask = p_strsave ("DD/DD/DD");
	}

	vlen = strlen (FIELD.mask);
	sp_mask[0]= '\0';
	for (ptr = 0; ptr < vlen; ptr++)
		if (FIELD.mask[ptr] == ',')
			no_commas++;

	/*------------------------------------
	| Find the number of leading digits. |
	------------------------------------*/
	ptr = vlen;
	while (FIELD.mask[ptr] != '.' && ptr)
		ptr--;

	if (ptr == 0) 
	{
		if (*pc == 'f' || *pc == 'd')
			ptr1 = 0;
		else
			ptr1 = vlen;

		if (FIELD.just != JUSTRIGHT)
			strcat (sp_mask,"-");

		if (FIELD.fill[0] == '0')
			strcat (sp_mask,"0");

		if ((*pc == 'l' && * (pc + 1) == 'd') || *pc == 'd' || *pc == 's')
			sprintf (prnt_mask,"%%%s%d%s",sp_mask,vlen,pc);
		else
			sprintf (prnt_mask,"%%%s%d.%d%s",sp_mask,vlen - no_commas,ptr1,pc);
	}
	else 
	{
		if (FIELD.just != JUSTRIGHT)
			strcat (sp_mask,"-");

		if (FIELD.fill[0] == '0')
			strcat (sp_mask,"0");

		sprintf (prnt_mask,"%%%s%d.%d%s",sp_mask,vlen - no_commas,vlen - ptr - 1,pc);
	}

	if ((int) strlen (FIELD.pmask) < (int) strlen (prnt_mask)) 
	{
		sprintf (err_str,"WARNING: Field %d's mask is too large to fit in mask space\n",field);
		file_err (-1, err_str,PNAME);
	}
	else
    {   /*====================================================================
		| 03/09/1999: Decision to prevent writes to constants results in a
		| core dump. We ponit pmask to a new block of memory containing 10
		| spaces to allow the strcpy (). Same fix for Win NT.
		====================================================================*/
		FIELD.pmask = p_strsave ("          ");
		strcpy (FIELD.pmask,prnt_mask);
    }
}

void
shell_prog (int	sh_type)
{
	int	indx = 0;
	char	wk_out[35];
	char	*wk_ptr	=	(char *)0;
	char	*sptr	=	(char *)0;
	char	*curr_user = getenv ("LOGNAME");

	void 	(*old_sigvec) ();

	/*--------------------------------------------------------------
	| Only print message if shell_prog is run from inside scrgen.c |
	--------------------------------------------------------------*/
	if (sh_type == FN_HELP || sh_type == EXIT_OUT || 
	     sh_type == HELP1 || sh_type > FN16)
		rv_pr (" Please Wait....",1,0,1);

	/*------------------------------------------------------- 
	| Shell out option for program window and program help. |
	-------------------------------------------------------*/
	if (sh_type == EXIT_OUT || sh_type == HELP1)
	{
		if (sh_type == EXIT_OUT)
 			* (arg) = "run_extern";
		else
 			* (arg) = "prog_help";

		* (arg+ (1)) = PNAME;
		* (arg+ (2)) = (char *)0;
	}
	if (sh_type == HELP)
	{
		clear();
		printf ("%s\n\r", PNAME);
		printf ("%s\n\r", PROG_VERSION);
		printf ("%s\n\r", PSLSCR_NAME);
		getkey ();
		return;
	}
	/*------------------------------------------------------- 
	| Shell out option for program window and program help. |
	-------------------------------------------------------*/
	if (sh_type > FN16 && sh_type < FN32)
	{
		if (curr_user != (char *)0)
		{
			sptr = p_strsave (curr_user);
			upshift (sptr);
			sprintf (wk_out, "%s.FN%d", sptr, sh_type - (FN1 - 1));
			wk_ptr = chk_env (wk_out);
		}
		if (wk_ptr == (char *) 0)
		{
			sprintf (wk_out,"FN%d",sh_type - (FN1 - 1));
			wk_ptr = chk_env (wk_out);
		}

		if (wk_ptr == (char *) 0)
			return;

		if (* (wk_ptr + strlen (wk_ptr) - 1) == '~')
			* (wk_ptr + strlen (wk_ptr) - 1) = '\0';

		sptr = strchr (wk_ptr,'~');

		while (sptr != (char *)0)
		{
			*sptr = '\0';
			* (arg+ (indx++)) = wk_ptr;

			wk_ptr = sptr + 1;
			sptr = strchr (wk_ptr,'~');
		}

		* (arg+ (indx++)) = wk_ptr;
		* (arg+ (indx)) = (char *)0;
	}
	/*----------------------------------------- 
	| Shell out option for function key help. |
	-----------------------------------------*/
	if (sh_type == FN_HELP)
	{
 		* (arg) = "help_fn";
		* (arg+ (1)) = (char *)0;
	}

	old_sigvec	=	signal (SIGCLD, SIG_DFL);

	switch (fork())
	{
	case -1:
		signal (SIGCLD, old_sigvec);
		return;

	case 0:
		/*
		 	*	Child process
		*/
		execvp (arg [0], arg);
		signal (SIGCLD, old_sigvec);
		return;
	
	default:
		/*
		*	Parent process
		*/
			wait ((int *)0);
	}
	signal (SIGCLD, old_sigvec);

	set_tty ();

	if ((sh_type == FN_HELP || sh_type == HELP || sh_type == EXIT_OUT || 
              sh_type == HELP1 || sh_type > FN16) && _wide)
		swide ();
}

int
DELETE (void)
{
	return (EXIT_SUCCESS);
}

/*===============================================
| Set Table Entry to Disable Edit_all on screen	|
| this will normally be used where a batch	|
| total is being used.				|
===============================================*/
void
no_edit (int	scn)
{
	int	i = valid_scn (scn);

	if (i >= 0)
	{
		tab_data[i]._row = -1;
		delta_edit = 1;
	}
}

void
edit_ok (int	scn)
{
	int	i = valid_scn (scn);

	if (i >= 0)
	{
		tab_data[i]._row = 1;
		delta_edit = 1;
	}
}

void
edit_fn (int	scn)
{
	int	i = valid_scn (scn);

	if (i >= 0)
		tab_data[i]._win = FALSE;
}

void
edit_win (int	scn)
{
	int	i = valid_scn (scn);

	if (i >= 0)
		tab_data[i]._win = TRUE;
}
void
edit_all (void)
{
	int	nindx = 0;
	int	curr = -1;
	int	last = -1;
	int	opt = 0;
	int	key = -1;
	int	max_scn = 0;
	int	desc_len = 0;
	int	last_scn = -1;
	int	first_scn = -1;
	int	box_width;
	char	num_str[3];

	while (1)
	{
		if (delta_edit)
		{
			opt = 0;
			curr = -1;
			max_scn = 0;
			desc_len = 0;
			last_scn = -1;
		}

		delta_edit = 0;

		while (opt < PV_scns && tab_data[opt]._scn != 0)
		{
			if (tab_data[opt]._row >= 0)
			{
				if (curr == -1)
					curr = opt;
				if (tab_data[opt]._desc != (char *)0 && 
				     (int) strlen (tab_data[opt]._desc) > desc_len)
					desc_len = strlen (tab_data[opt]._desc);
				
				tab_data[opt]._row = max_scn + 3;
				max_scn++;
				last_scn = opt;
			}
			opt++;
		}

		first_scn = curr;

		if (max_scn == 0 || first_scn == -1 || last_scn == -1)
		{
			box (0,0,30,1);
			rv_pr (" No Screens Available To Edit ",1,1,1);
			return;
		}

		box_width = dis_edit (first_scn,max_scn,last_scn,desc_len);

		while (1)
		{
			if (last != curr)
			{
				if (last >= 0 && tab_data[last]._row > 0)
					_print_edit (last,box_width,0);

				if (curr >= 0 && tab_data[curr]._row > 0)
					_print_edit (curr,box_width,1);
			}

			_print_head (tab_data[curr]._scn,box_width);

			last = curr;
			last_char = getkey ();

			if (last_char < 2000 && isdigit (last_char))
			{
				key = last_char;
				last_char = '0';
			}

			if (last_char == FN15 || last_char == UP_KEY || last_char == LEFT_KEY)
				last_char = 11;

			if (last_char == FN13 || last_char == DOWN_KEY || last_char == FN14 || last_char == RIGHT_KEY)
				last_char = 10;

			switch (last_char)
			{
			case	'0':
				if (MAXSCNS < 10)
					nindx = 0;

				num_str[nindx++] = key;
				num_str[nindx] = '\0';
				opt = atoi (num_str);
				curr = valid_scn (opt);
				if (curr == -1 || tab_data[curr]._row < 0)
				{
					nindx = 0;
					num_str[nindx] = '\0';
					putchar (BELL);
					curr = last;
					last = -1;
				}
				break;

			case	8:
				/*-------------------------------
				| Keying the option number	|
				-------------------------------*/
				if (nindx)
				{
					num_str[--nindx] = '\0';
					opt = (nindx) ? atoi (num_str) : -1;
					curr = valid_scn (opt);
					if (curr != -1)
						break;
				}

			case	11:
				num_str[0] = '\0';
				nindx = 0;
				curr--;
				while (curr >= first_scn && tab_data[curr]._row == -1)
					curr--;

				if (curr < first_scn)
					curr = last_scn;
				break;

			case	10:
			case	' ':
			case	12:
				num_str[0] = '\0';
				nindx = 0;
				curr++;
				while (curr <= last_scn && tab_data[curr]._row == -1)
					curr++;

				if (curr > last_scn)
					curr = first_scn;
				break;

			case	'\r':
				if (tab_data[curr]._win)
				{
					heading (tab_data[curr]._scn);
					scn_display (tab_data[curr]._scn);
					edit (tab_data[curr]._scn);
				}
				else
					 (* tab_data[curr]._actn) ();
				
				if (restart)
					return;

				edit_end_eall (tab_data [curr]._scn);
				num_str[0] = '\0';
				nindx = 0;

			case	FN3:
				if (!delta_edit)
					dis_edit (first_scn,max_scn,last_scn,desc_len);
				last = -1;
				break;

			case	FN1:
				if (_ck_restart ())
					restart = TRUE;
				else
				{
					restart = FALSE;
					break;
				}

			case	FN16:
				crsr_on ();
				return;

			case PSLW	:
			case PSLW2	:
			case PSLW3	:
			case PSLW4	:
			case PSLW5	:
				if (_win_func)
				{
					switch (last_char)
					{
					case PSLW:
				    	win_function (0, 0, 0, !ENTRY);
						break;
					case PSLW2:
				    	win_function2 (0, 0, 0, !ENTRY);
						break;
					case PSLW3:
				    	win_function3 (0, 0, 0, !ENTRY);
						break;
					case PSLW4:
				    	win_function4 (0, 0, 0, !ENTRY);
						break;
					case PSLW5:
				    	win_function5 (0, 0, 0, !ENTRY);
						break;
					}
				    if (!delta_edit)
					dis_edit (first_scn,max_scn,last_scn,desc_len);
				    last = -1;
				    break;
				}

			default:
				last = -1;
				putchar (BELL);
			}

			if (delta_edit)
				break;
		}
	}
}

int
dis_edit (
	int	_first_scn,
	int	_max_scn,
	int	_last_scn,
	int	_desc_len)
{
	int	i;
	int	_box_width = (_desc_len == 0) ? 12 : _desc_len + 6;

	crsr_off ();

	if (_box_width < 12)
		_box_width = 12;

	for (i = 0;i < _max_scn + 2;i++)
	{
		move (X_EALL,i + Y_EALL);
		printf ("%-*.*s",_box_width,_box_width," ");
	}
	fflush (stdout);
	
	box (X_EALL,Y_EALL ,_box_width + 1,_max_scn + 2);

	_print_head (tab_data[_first_scn]._scn,_box_width);

	move (1 + X_EALL,2 + Y_EALL);
	line (_box_width);

	for (i = _first_scn;i <= _last_scn;i++)
		_print_edit (i,_box_width,0);

	return (_box_width);
}

void
_print_head (
	int	_indx,
	int	_box_width)
{
	int	_left = (_box_width - 10) / 2;
	int	_right = _box_width - 10 - _left;
	char	_buffer[81];

	sprintf (_buffer,"%*.*sEdit (%2d)%*.*s",
		_left,
		_left,
		" ",
		_indx,
		_right,
		_right,
		" ");

	rv_pr (_buffer,1 + X_EALL,1 + Y_EALL,1);
}

void
_print_edit (
	int	_indx,
	int	_box_width,
	int	_rv_flag)
{
	char	_buffer[81];

	if (tab_data[_indx]._row < 0)
		return;

	if (tab_data[_indx]._desc != (char *)0)
		sprintf (_buffer," %2d %-*.*s ",tab_data[_indx]._scn,
					_box_width - 6,
					_box_width - 6,
					ML (tab_data[_indx]._desc));
	else
		sprintf (_buffer," %s %2d%-*.*s",ML ("SCREEN"),tab_data[_indx]._scn,
					_box_width - 11,
					_box_width - 11,
					" ");
	rv_pr (_buffer,1 + X_EALL,tab_data[_indx]._row + Y_EALL,_rv_flag);
}

int
valid_scn (int	scn)
{
	int	i;

	if (scn <= 0)
		return (-1);

	for (i = 0;i < PV_scns;i++)
	{
		if (tab_data[i]._scn == 0)
			break;

		if (tab_data[i]._scn == scn)
			return (i);
	}
	return (-1);
}

void
ent_edit (
	int		scn,
	int		field)
{
	int	c;
	int	key = 0;
	int	last_row = -1;
	int	curr_row;
	int	last_field = -1;
	int	old_page = scn_page;
	int	max_field = 0;
	int	work_line = 0;
	int	temp_line = 0;

	/*--------------------------
	| If restart keyed return. |
	--------------------------*/
	if (restart) 
	{
		edit_exit = TRUE;
		return;
	}

	crsr_off ();
	edit_exit = 0;
	entry_exit = 0;
	edit_exit = 0;

	prog_status = ! (ENTRY);
	if (scn != cur_screen)
		scn_set (scn);

 	curr_row = line_cnt % PV_tlines;
	max_field = scn_start + nbr_fields - 1;

	while (field <= max_field && (P_NOEDIT || P_HIDE || P_DISPLAY))
		field++;

	if (field > max_field)
		field = scn_start;

	while (!restart)
	{
		cur_field = field;
		if (scn_page != old_page) 
		{
			line_cnt = curr_row % PV_tlines;
			line_cnt += (scn_page * PV_tlines);
			scn_write (cur_screen);
			scn_display (cur_screen);
			last_row = -1;
			last_field = -1;
		}

		if (last_row != curr_row || last_field != field)
		{
		    if (last_row >= 0 && last_field >= scn_start)
		    {
			work_line = last_row % PV_tlines;
			work_line += (scn_page * PV_tlines);
			getval (work_line);
			temp_line = line_cnt;
			line_cnt = work_line;
			if (line_cnt >= lcount[cur_screen])
			    Display_field (last_field, FALSE, TRUE);
			else
			    Display_field (last_field, FALSE, FALSE);
			line_cnt = temp_line;
		    }

		    work_line = curr_row % PV_tlines;
		    work_line += (scn_page * PV_tlines);

		    if (curr_row >= 0 && field >= scn_start)
		    {
			getval (work_line);
			temp_line = line_cnt;
			line_cnt = work_line;
			if (line_cnt >= lcount[cur_screen])
			    Display_field (field, TRUE, TRUE);
			else
			    Display_field (field, TRUE, FALSE);
			line_cnt = temp_line;
		    }

		    tab_other (work_line);
		}

		last_row = curr_row;
		last_field = field;
		old_page = scn_page;

		c = getkey ();

		if (c == REDRAW || c == FN_HELP || 
		     c == HELP || c == HELP1 || 
		     c == EXIT_OUT || (c > FN16 && c <= FN32))
		{
			key = c;
			c = FN17;
		}

		if (_KEY_LEFT (c) || _KEY_UP (c) || 
		     _KEY_DOWN (c) || _KEY_RIGHT (c))
		{
			key = c;
			c = LEFT_KEY;
		}

		if (c == RESTART)
		{
			if (_ck_restart ())
			{
				key = RESTART;
				c = FN16;
			}
			else
				c = key = 0;
		}

		switch (c) 
		{
		case	FN13:
			return;

		case	FN17:
			if (key != REDRAW)
				shell_prog (key);

			last_row = -1;
			last_field = -1;
			c = line_cnt;
			heading (cur_screen);
			line_cnt = c;
			scn_display (cur_screen);
			break;

		/*---------------
		| Next Page	|
		---------------*/
		case	FN14:
			if (((scn_page + 1 <
			    vars [scn_start].row / PV_tlines) ||
				    vars [scn_start].row % PV_tlines != 0) &&
			    lcount [cur_screen] / PV_tlines > scn_page)
			{
				scn_page++;
			}
			else
				scn_page = 0;
			curr_row = 0;
			break;

		/*---------------
		| Prev Page	|
		---------------*/
		case	FN15:
			if (scn_page != 0)
				scn_page--;
			else
				scn_page = lcount[cur_screen] / PV_tlines;
			curr_row = 0;
			break;

		case	LEFT_KEY:
			if (_KEY_LEFT (key))
			{
				last_field = field--;
				field = _prev_field (field, last_field,
						     max_field, TRUE);
			}
			if (_KEY_RIGHT (key))
			{
				last_field = field++;
				field = _next_field (field,last_field,
						     max_field,TRUE);
			}
			if (_KEY_DOWN (key))
			{
				last_row = curr_row++;
				curr_row = _next_row (curr_row, last_row);
			}
			if (_KEY_UP (key))
			{
				last_row = curr_row--;
				curr_row = _prev_row (curr_row, last_row);
			}
			break;

		case	FN16:
			if (key == RESTART)
				restart = TRUE;
			return;

		case	'\r':
			if (P_NOEDIT || P_HIDE || P_DISPLAY)
				break;

			line_cnt = curr_row % PV_tlines;
			line_cnt += (scn_page * PV_tlines);
			if (line_cnt >= lcount[cur_screen])
				Display_field (field, FALSE, TRUE);
			edit_scn (field,line_cnt);
			last_row = -1;
			last_field = -1;
			break;

		case PSLW:
			if (_win_func)
			{
				win_function (
					cur_field,
					 (curr_row % PV_tlines) + (scn_page * PV_tlines),
					cur_screen,
					!ENTRY);
			    old_page = -1;
			    break;
			}
			else
				putchar (BELL);
			break;

		case PSLW2	:
			if (_win_func)
			{
				win_function2 (
					cur_field,
					 (curr_row % PV_tlines) + (scn_page * PV_tlines),
					cur_screen,
					!ENTRY);
			    old_page = -1;
			    break;
			}
			else
				putchar (BELL);
			break;

		case PSLW3	:
			if (_win_func)
			{
				win_function3 (
					cur_field,
					 (curr_row % PV_tlines) + (scn_page * PV_tlines),
					cur_screen,
					!ENTRY);
			    old_page = -1;
			    break;
			}
			else
				putchar (BELL);
			break;

		case PSLW4	:
			if (_win_func)
			{
				win_function4 (
					cur_field,
					 (curr_row % PV_tlines) + (scn_page * PV_tlines),
					cur_screen,
					!ENTRY);
			    old_page = -1;
			    break;
			}
			else
				putchar (BELL);
			break;

		case PSLW5	:
			if (_win_func)
			{
				win_function5 (
					cur_field,
					 (curr_row % PV_tlines) + (scn_page * PV_tlines),
					cur_screen,
					!ENTRY);
			    old_page = -1;
			    break;
			}
			else
				putchar (BELL);
			break;

		default:
			putchar (BELL);
			break;
		}
	}
}

static	char
*fmt_comma (
	char	*str,
	int		field)
{
	char	wk_str[32];
	int	i,
		j;

	strcpy (wk_str, str);
	for (i = j = 0; i < strlen (FIELD.mask); i++)
	{
		if (FIELD.mask[i] == ',')
			if (* (str + i - 1) == ' ')
				* (str + i) = ' ';
			else
				* (str + i) = ',';
		else
			* (str + i) = wk_str[j++];
		* (str + i + 1) = 0;
	}
	return (str);
}

int
_ck_restart (void)
{
	int c;

	if (!restart_ck)
		return (EXIT_FAILURE);

	if (strlen (clip (restart_msg)) == 0)
		strcpy (restart_msg, "Restart ?");

	crsr_on ();
	putchar (BELL);
	c = prmptmsg (restart_msg, "YyNn", 0, 23);  

	crsr_off ();
	clear_mess ();

	if (c == 'Y' || c == 'y')
		return (EXIT_FAILURE);
	else
		return (EXIT_SUCCESS);
}

static void
HighLightField (
 int field,
 int yes)
{
	int x, y;

	if (TruePosition)
		x = (P_LIN) ? FIELD.col : FIELD.col;
	else
		x = (P_LIN) ? CPOS : FIELD.col;

	y = (P_TAB) ? (tab_row + 2 + (line_cnt % PV_tlines)) : FIELD.row;

	crsr_off();
	li_pr (FIELD.prmpt, x, y, yes);
	move (x, y);

	fflush(stdout);
}


static char *
FieldContentsAsString (
 int field)
{
	static char buff [200];
		
	if (field == -1)
		return NULL;

	if (P_HIDE)
		return NULL;

	switch (FIELD.type)
	{
	  case CHARTYPE:
		sprintf (buff, FIELD.pmask,(FIELD.mask[0] == '_') ? FIELD.mask 
							                              : VAR_PTR.cptr);
		break;

	  case INTTYPE:
		sprintf (buff, FIELD.pmask, *(VAR_PTR.iptr));
		break;

	  case LONGTYPE:
		sprintf (buff, FIELD.pmask, *(VAR_PTR.lptr));
		break;

	  case FLOATTYPE:
		sprintf (buff, FIELD.pmask, *(VAR_PTR.fptr));
		break;

	  case DOUBLETYPE:
		sprintf (work_str, FIELD.pmask, *(VAR_PTR.dptr));
		if (!strchr (FIELD.mask, ','))
			strcpy (buff, work_str);
		else
			strcpy (buff, fmt_comma (work_str, field));
		break;

	  case MONEYTYPE:
		sprintf (work_str, FIELD.pmask,DOLLARS(*(VAR_PTR.dptr)));
		if (!strchr (FIELD.mask, ','))
			strcpy (buff, work_str);
		else
			strcpy (buff, fmt_comma (work_str, field));
		break;

	  case EDATETYPE:
		sprintf (buff, FIELD.pmask, DateToString (*(VAR_PTR.lptr)));
		break;

	  case TIMETYPE:
		sprintf (buff, "%s", ttoa (*(VAR_PTR.lptr), FIELD.mask));
		break;

	  default:
		break;
	}

	return (buff);
}
