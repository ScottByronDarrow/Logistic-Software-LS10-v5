/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: chartinp.c,v 5.4 2001/08/21 04:32:16 scott Exp $
|  Program Name  : (gl_chartinp.c) 
|  Program Desc  : (General ledger Chart Of Accounts Input)
|---------------------------------------------------------------------|
|  Date Written  : (13/06/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: chartinp.c,v $
| Revision 5.4  2001/08/21 04:32:16  scott
| Updated to fix display prompt
|
| Revision 5.3  2001/08/09 09:13:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:11  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:40  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: chartinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_chartinp/chartinp.c,v 5.4 2001/08/21 04:32:16 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;

/*
 *   Constants, defines and stuff   
 */
#define TOTSCNS		1

	char	*data = "data";


extern int  GV_max_level;

	/*
	 * Special fields and flags  ##################################. |
	 */
   	int		newChart 	= 0,
   			edit_mode 	= 0;


	extern	int	TruePosition;
/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	long	prev_acc;
	int		curr_level,
			prev_level,
			acc_width,
			prev_width;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "level_no",	 5, 2, INTTYPE,
		"N", "          ",
		"", "1", "Level Number         ", "",
		 NE, NO,  JUSTLEFT, "", "", (char *) &local_rec.curr_level},
	{1, LIN, "acc_bit",	 6, 2, LONGTYPE,
		"NNNNNN", "          ",
		"0", "", "Account Number       ", " ",
		 NE, NO, JUSTRIGHT, "1", "999999", (char *) &glcaRec.acc_no},
	{1, LIN, "desc",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Description  ", " ",
		YES, NO,  JUSTLEFT, "", "", glcaRec.acc_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 *   Local function prototypes  
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	SrchGlca 		(char *);
int  	Update 			(void);
int  	heading 		(int);
void 	shutdown_prog 	(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char 	*argv [])
{
	static	char	level_cmnt [81];

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr 	 ();
	set_tty 	 ();	
	set_masks 	 ();
	init_vars 	 (1);

	/*
	 * Open main database files.
	 */
	OpenDB (); 

	FindGlct ();
	GL_SetMask (glctRec.format);
	sprintf (level_cmnt, "Level must be in range (1 to %d).", GV_max_level);
	vars [label ("level_no")].comment = level_cmnt;

	local_rec.prev_level = local_rec.prev_acc = 0;
	local_rec.prev_width = 1;

	search_ok = TRUE;

    /*
     * Beginning of input control loop.
     */
	while (!prog_exit)
	{
		entry_exit	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newChart 	= FALSE;
		edit_mode 	= FALSE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		/*
		 * Edit screen 1 input.
		 */
		edit_mode = 1;
		edit_exit = 0;

		heading (1);
		scn_display (1);
		edit (1);
		edit_exit = 0;

		/*
		 * Update General ledger Account Record.
		 */
		if (!restart && Update ()) 
        {
            shutdown_prog ();
            return (EXIT_SUCCESS);
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

	OpenGlca ();
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
spec_valid (
 int field)
{
	char	*acc_mask;
	int 	invalid = 0;

	if (LCHECK ("level_no")) 
	{
		glcaRec.level_no = local_rec.curr_level;

        acc_mask = GL_SetBitWidth (glcaRec.level_no,	
                                  &local_rec.prev_width, 
                                  &local_rec.acc_width);
		if (!acc_mask)
            return (EXIT_FAILURE);

		vars [label ("acc_bit")].mask = acc_mask;
		set_masks ();
		heading (1);
		scn_display (1);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Account Number Input.
	 */
	if (LCHECK ("acc_bit")) 
	{
		glcaRec.level_no = local_rec.curr_level;
		if (SRCH_KEY)
		{
			SrchGlca (temp_str);
			return (EXIT_SUCCESS);
		}

		glcaRec.acc_no = GL_FormAccBit (temp_str, local_rec.acc_width);
        newChart = 	ReadGlca 
					 (
						comm_rec.co_no,
						glcaRec.level_no, 
						temp_str,
						"u", 
						&glcaRec
					);
		if (!newChart)
		{
			entry_exit = 1;
			return (EXIT_SUCCESS);
		}
		else
		{
			/*
			 * New Account Number
			 */
			rv_pr (ML (mlGlMess048), 50,7,1);
		}

		/*
		 * If in edit mode special checks must be performed.
		 */
		if (edit_mode) 
		{ 
			/*
			 * Account not on file.
			 */
			if (newChart) 
			{
				errmess (ML (mlStdMess024));
				return (EXIT_FAILURE);
			}
			else 
                scn_display (1);
		}
		return (EXIT_SUCCESS);
	}
			
	return (invalid);
}

void	
SrchGlca (
 char *key_val)
{
	long    temp_val;
    char    form_bit [MAXLEVEL + 1];
	GLCA_STRUCT	glcaRec2;

	_work_open (10,0,40);
	save_rec ("#Acc", "#Account Description   ");

	strcpy (glcaRec2.co_no,comm_rec.co_no);
	glcaRec2.level_no = glcaRec.level_no;
	temp_val = glcaRec2.acc_no = atol (key_val);

    cc = find_rec (glca, &glcaRec2, GTEQ,"r");
	while (!cc && !strcmp (glcaRec2.co_no, comm_rec.co_no) &&
				glcaRec2.level_no == glcaRec.level_no)
	{
		sprintf (form_bit, "%0*ld", local_rec.acc_width, glcaRec2.acc_no);

        cc = save_rec (form_bit, glcaRec2.acc_desc);
		if (cc)
            break;

		cc = find_rec (glca, &glcaRec2, NEXT,"r");
	}
	disp_srch ();
	work_close ();
	cc = find_rec (glca, &glcaRec, GTEQ,"r");
}

int
Update (void)
{
	int	invalid = 0;

	clear ();

	/*
	 * Add or update database record.
	 */
	glcaRec.mod_date = TodaysDate ();
	if (newChart) 
	{
		strcpy (glcaRec.co_no,comm_rec.co_no);
		strcpy (glcaRec.stat_flag,"0");

        cc = abc_add (glca,&glcaRec);
		if (cc)
            file_err (cc, glca, "DBADD");
	}
	else 
	{
        cc = abc_update (glca,&glcaRec);
		if (cc)
            file_err (cc, glca, "DBUPDATE");
	}
	local_rec.prev_width = local_rec.acc_width;
	local_rec.prev_acc   = glcaRec.acc_no;
	local_rec.prev_level = glcaRec.level_no;
	return (invalid);
}

int
heading (
 int scn)
{
	int	s_size = 80;

	if (restart) 
	{
        abc_unlock (glca);
    	return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	/*
	 * Maintain Account Descriptions.
	 */
	sprintf (err_str, " %s ", ML (mlGlMess049));
	rv_pr (err_str, 20,0,1);

	/*
	 * Last Account: 
	 */
	print_at (0,55, ML (mlGlMess047),
					local_rec.prev_level,
					local_rec.prev_width,
					local_rec.prev_acc);
	line_at (1,0,s_size);

	if (scn == 1)
	{
		box (0,4,80,3);
	}

	line_at (20,0,s_size);
	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	line_at (22,0,s_size);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
