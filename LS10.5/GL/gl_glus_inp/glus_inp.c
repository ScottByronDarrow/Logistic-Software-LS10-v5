/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: glus_inp.c,v 5.6 2002/07/25 11:17:28 scott Exp $
|=====================================================================|
|  Program Name  : (gl_glus_inp.c) 
|  Program Desc  : (General Ledger User Security Maint)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/11/95         |
|---------------------------------------------------------------------|
| $Log: glus_inp.c,v $
| Revision 5.6  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2002/07/18 06:39:31  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2001/12/12 10:06:28  robert
| LS10.5-GUI update
|
| Revision 5.3  2001/08/09 09:13:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:23  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:50  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: glus_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_glus_inp/glus_inp.c,v 5.6 2002/07/25 11:17:28 scott Exp $";

/*
 *   Include file dependencies  
 */
#define TABLINES	15
#define MAXLINES	200

#include <pslscr.h>
#include <GlUtils.h>

#ifdef GVISION
#include <RemoteFile.h>
#define fopen	Remote_fopen
#define fgets	Remote_fgets
#define fclose	Remote_fclose
#else
#include <menu.h>
#endif

#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 *   Constants, defines and stuff   
 */
int loadUserSecurity = 0;

#include	"schema"

struct commRecord	comm_rec;

/*
 * Local & Screen Structures.
 */
struct 
{
	char	dummy [11];
	char 	user_name [15];
	char 	acc_prefix [FORM_LEN + 1];
	char 	superUser [2];
} local_rec;
	
static struct var vars [] =
{
	{1, TAB, "user_name",	MAXLINES, 2, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "   User Name    ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.user_name},
	{1, TAB, "acc_prefix",	 0, 0, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNN", "          ",
		"", " ", "General Ledger Account Prefix. ", "",
		YES, NO, JUSTLEFT, "", "", local_rec.acc_prefix},
	{1, TAB, "superUser",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "Y", "Accountant.", "Y(es) = Accountant , N(o) = Normal User.",
		YES, NO, JUSTLEFT, "YN", "", local_rec.superUser},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 *   Local function prototypes  
 */
void 	LoadGlus 		 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	Update 			 (void);
int  	spec_valid 		 (int);
int  	heading 		 (int);
void 	shutdown_prog 	 (void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc,
 char *argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	sptr = chk_env ("GL_USER_SECURE");
	loadUserSecurity = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Setup required parameters.
	 */
	init_scr ();    /*  sets terminal from termcap	*/
	set_tty ();     /*  get into raw mode		*/

	OpenDB ();


	vars [label ("acc_prefix")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);

	tab_row = 3;
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	clear ();

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars (1);
		lcount [1] = 0;

		LoadGlus ();
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
        {
            Update ();
        }

		/*
		 * UnLock whole file as numbers are maintained.
		 */
		prog_exit = TRUE;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	OpenGlus ();
}

void
CloseDB (
 void)
{
	GL_Close ();
	abc_dbclose ("data");
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

int
spec_valid (
 int field)
{
	if (LCHECK ("superUser"))
	{
		if (local_rec.superUser [0] == 'Y')
		{
			strcpy (local_rec.acc_prefix, " ");
			DSP_FLD ("acc_prefix");
		}

		DSP_FLD ("superUser");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update all files.
 */
void
Update (void)
{
	int	i;

	clear ();
	print_at (0,0, ML (mlStdMess035));

	fflush (stdout);

	scn_set (1);
	for (i = 0;i < lcount [1];i++) 
	{
		getval (i);

		strcpy (glusRec.co_no, comm_rec.co_no);
		strcpy (glusRec.user_name, local_rec.user_name);

        cc = find_rec (glus, &glusRec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (glusRec.acc_hdr_code, local_rec.acc_prefix);
			glusRec.super_user = (local_rec.superUser [0] == 'Y') ? 1 : 0;
		
            cc = abc_update (glus, &glusRec);
			if (cc)
                file_err (cc, glus, "DBUPDATE");
		}
		else
		{
			strcpy (glusRec.co_no, comm_rec.co_no);
			strcpy (glusRec.user_name, local_rec.user_name);
			strcpy (glusRec.acc_hdr_code, local_rec.acc_prefix);
			glusRec.super_user = (local_rec.superUser [0] == 'Y') ? 1 : 0;

            cc = abc_add (glus, &glusRec);
			if (cc)
                file_err (cc, glus, "DBADD");
		}
	}
}

void
LoadGlus (void)
{
	char	*sptr;
	char	*tptr;
	char	filename [101];
	int		LenForm;

	LenForm = strlen (vars [label ("acc_prefix")].mask);

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (1);

	lcount [1] = 0;

	strcpy (glusRec.co_no, comm_rec.co_no);
	sprintf	 (glusRec.user_name, "%-14.14s", "SYSTEM WIDE");

    cc = find_rec (glus, &glusRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (local_rec.user_name, "%-14.14s", glusRec.user_name);
		sprintf (local_rec.acc_prefix, "%*.*s", LenForm,LenForm," ");

		putval (lcount [1]++);
	}
	else
	{
		sprintf (local_rec.user_name, "%-14.14s", glusRec.user_name);
		sprintf (local_rec.acc_prefix, 
                 "%-*.*s", 
                 LenForm,
                 LenForm,
                 glusRec.acc_hdr_code);
		strcpy (local_rec.superUser, (glusRec.super_user) ? "Y" : "N");
		putval (lcount [1]++);
	}

#ifdef GVISION
	#define DATA_SIZE 200
	FILE *fmenu;
	char data [256];
	sptr = ServerPROG_PATH ();
#else
	sptr = getenv ("PROG_PATH");
#endif

	if (loadUserSecurity)
	{
		sprintf (filename,"%s/BIN/MENUSYS/User_secure", (sptr != (char *)0) ? sptr : "/usr/LS10.5");

		if ( (fmenu = fopen (filename,"r")) == 0)
		{
			sprintf (err_str,"Error in %s during (FOPEN)",filename);
			sys_err (err_str,errno,PNAME);
		}

		sptr = fgets (data,DATA_SIZE,fmenu);
	
		while (sptr != (char *)0)
		{
			tptr = sptr;
			while (*tptr != ' ' && *tptr != '\t')
            {
                tptr++;
            }
	
			*tptr = '\0';
	
			strcpy (glusRec.co_no, comm_rec.co_no);
			sprintf	 (glusRec.user_name, "%-14.14s", data);

            cc = find_rec (glus, &glusRec, COMPARISON, "r");
			if (cc)
			{
				sprintf (local_rec.user_name, "%-14.14s", glusRec.user_name);
				sprintf (local_rec.acc_prefix, "%*.*s", 
                         LenForm,
                         LenForm,
                         " ");

				putval (lcount [1]++);
			}
			else
			{
				sprintf (local_rec.user_name, "%-14.14s", glusRec.user_name);
				sprintf (local_rec.acc_prefix, 
                         "%-*.*s", 
                         LenForm,
                         LenForm,
                         glusRec.acc_hdr_code);

				strcpy (local_rec.superUser, (glusRec.super_user) ? "Y" : "N");
				putval (lcount [1]++);
			}

			if (lcount [1] > MAXLINES) 
            {
                break;
            }

			sptr = fgets (data,DATA_SIZE,fmenu);
		}
		fclose (fmenu);
	}

	scn_set (1);
	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
            scn_set (scn);
        }
		clear ();

		rv_pr (ML (mlGlMess019) ,16,0,1);

		line_at (1,0,80);
		line_at (20,0,80);

		sprintf (err_str, 
                 ML (mlStdMess038),
                 comm_rec.co_no,
                 comm_rec.co_name);
		print_at (21,0, err_str);

		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
