/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: glct_maint.c,v 5.3 2001/08/09 09:14:05 scott Exp $
|  Program Name  : (glct_maint.c)
|  Program Desc  : (Maintain General Ledger Control Record)
|---------------------------------------------------------------------|
|  Date Written  : (12/06/89)      | Author       : Huon Butterworth. |
|---------------------------------------------------------------------|
| $Log: glct_maint.c,v $
| Revision 5.3  2001/08/09 09:14:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:44  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:09  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: glct_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/glct_maint/glct_maint.c,v 5.3 2001/08/09 09:14:05 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<ml_std_mess.h>
#include	<ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;

	char	*data	= "data";

int		newCode = 0;
extern	int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	char	format [32];
	int		history;
	int		link_max;
	char	ptype [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "format",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", "XXX-XXX-XXX-XXX-XXXX", "Account code format.           ", " ",
		 NO, NO,  JUSTLEFT, "X-", "", local_rec.format},
	{1, LIN, "history",	 6, 2, INTTYPE,
		"N", "          ",
		" ", "1", "No of history years.           ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &local_rec.history},
	{1, LIN, "links",	 7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Maximum no of parent Accounts. ", " ",
		 NO, NO,  JUSTLEFT, "1", "10", (char *) &local_rec.link_max},
	 /*
	{1, LIN, "period",	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "C", "Period type.                   ", " ",
		 NO, NO,  JUSTLEFT, "C", "", local_rec.ptype},
	 */

	{0, LIN, "",	 0, 0, CHARTYPE,
		"", "",
		"", "", "", "",
		 NO, NO,  JUSTLEFT, "", "", NULL},
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int 	heading 		 (int);
int 	spec_valid 		 (int);
int 	CheckFormat 	 (void);
int 	Update 			 (void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	TruePosition	=	TRUE;
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	search_ok = 1;

	OpenDB ();
	strcpy (local_rec.format, glctRec.format);
	local_rec.history       = glctRec.history;
	local_rec.link_max      = glctRec.link_max;
	strcpy (local_rec.ptype,  glctRec.ptype);

	if (newCode)
	{
		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
	}

	heading (1);
	scn_display (1);
	edit (1);

	if (!restart)
    {
		if (Update ())
		{
			/*
			 * Couldn't update... Accounts EXIST!!
			 */
			print_mess (ML (mlGlMess056));
			sleep (10);
		}
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	OpenGlmr ();

	if (FindGlct ())
		newCode = TRUE;
	else
	{
		/*
		 * Disable add/update IF ANY G/L accounts already exist!  
		 */
		cc = find_rec (glmr, &glmrRec, FIRST, "r");
		if (!cc)
			FLD ("format")	=	NE;
	}
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose (data);
}

int
heading (
	int		scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		/*
		 * General Ledger Control File Maintenance
		 */
		centre_at (0, 80, ML (mlGlMess054));
		line_at (1,0,80);

		if (scn == 1)
		{
			line_at (5,1,80);
			box (0, 3, 80, 4);
		}

		line_at (20,0,80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("format"))
	{
		strcpy (glctRec.format, local_rec.format);
		return (CheckFormat ());
	}

	return (EXIT_SUCCESS);
}

int
CheckFormat (void)
{
	char	last_char = ' ',
		*t_ptr;
	int	x_cnt = 0;

	t_ptr = glctRec.format;

	for (t_ptr = glctRec.format; *t_ptr && *t_ptr != ' '; t_ptr++)
	{
		if (*t_ptr == 'X')
			x_cnt++;

		if (*t_ptr == '-' && last_char == '-')
		{
			last_char = *t_ptr;
			while (*t_ptr == '-' && last_char == '-')
			{
				strcpy (t_ptr, t_ptr + 1);
				* (t_ptr + FORM_LEN) = ' ';
			}
		}
		else
			last_char = *t_ptr;
	}

	if (x_cnt > MAXLEVEL)
	{
		/*
		 * Too many X's in format.
		 */
		print_mess (ML (mlGlMess057)); 
		return (EXIT_FAILURE);
	}

	if (last_char == '-')
	{
		/*
		 * Format cannot end with a '-'.
		 */
		print_mess (ML (mlGlMess058));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
Update (void)
{
	/*
	 * Add or update product group description record.
	 */
	cc = find_rec (glct, &glctRec, FIRST, "u");
	strcpy (glctRec.format, local_rec.format);
	glctRec.history       = local_rec.history;
	glctRec.link_max      = local_rec.link_max;
	/*
	strcpy (glctRec.ptype,  local_rec.ptype);
	*/
	strcpy (glctRec.ptype,  "C");
	glctRec.mod_date = TodaysDate ();
	if (newCode)
	{
		strcpy (glctRec.stat_flag, "0");
		cc = abc_add (glct, &glctRec);
		if (cc)
			file_err (cc, glct, "DBADD");
	}
	else
		cc = abc_update (glct, &glctRec);
		if (cc)
			file_err (cc, glct, "DBUPDATE");

	return (EXIT_SUCCESS);
}
