/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: del_ccomp.c,v 5.4 2002/07/08 03:51:49 scott Exp $
|  Program Name  : (gl_con_sol.c)                             
|  Program Desc  : (General Ledger Delete Consolidate Company)
|---------------------------------------------------------------------|
|  Author        : Andy Yuen       | Date Written  : 19/03/96         |
|---------------------------------------------------------------------|
| $Log: del_ccomp.c,v $
| Revision 5.4  2002/07/08 03:51:49  scott
| S/C 4083 - Company Description is not displayed.
|
| Revision 5.3  2001/08/09 09:13:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:18  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:45  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: del_ccomp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_del_ccomp/del_ccomp.c,v 5.4 2002/07/08 03:51:49 scott Exp $";

/*
 *   Include file dependencies  
 */

#include <pslscr.h>
#include <GlUtils.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 *   Constants, defines and stuff   
 */
#define		WK_DEPTH	10
#define		MAXLEVEL	16

extern int tab_max_page;
       int first_line = 0;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glshRecord	glsh_rec;
struct glsdRecord	glsd_rec;

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

	GLMR_STRUCT glmr3Rec;
	GLLN_STRUCT glln2_rec;


/*
 * Local & Screen Structures.
 */
struct
{
	char	src_co		 [3];
	char	src_name	 [41];
	char 	dummy		 [10];
} local_rec;

extern	int	TruePosition;

static struct var vars [] =
{
	{1, LIN, "src_co", 3, 2, CHARTYPE, 
		"AA", "          ",
		" ", "", "Consolidated Company       ", " ",
		YES, NO, JUSTRIGHT, "1", "99", local_rec.src_co},
	{1, LIN, "src_name", 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Consolidated Company Name  ", " ",
		NA, NO, JUSTRIGHT, "", "", local_rec.src_name},

	{0, LIN, "", 0, 0, INTTYPE, "A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ",
		local_rec.dummy}
};

/*
 *   Local function prototypes  
 */

int  	heading 		(int);
int  	spec_valid 		(int);
void 	shutdown_prog 	(void);
void 	CloseDB 		(void);
void 	DelComr 		(void);
void 	DelGlbd 		(void);
void 	DelGlca 		(void);
void 	DelGlln 		(long);
void 	DelGlmr 		(void);
void 	DelGlpd 		(long);
void 	DelGlsh 		(void);
void 	DelGlsd 		(long);
void 	DelGltr 		(long);
void 	OpenDB 			(void);
void 	Process 		(void);
void 	SrchComr 		(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int argc,
 char *argv [])
{
	int c;

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
		scn_display (1);
		edit (1);
		if (restart)
            continue;
        
		break;
	}	

	if (prog_exit == 0)
	{
		/*
		 * Are you sure to delete this consolidated company
		 */
		c = prmptmsg (ML (mlGlMess159), "YyNn",10,10);
		if (c == 'y' || c == 'Y')
            Process ();
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

	abc_alias (glbd2, glbd);
	abc_alias (glca2, glca);
	abc_alias (glln2, glln);
	abc_alias (glmr2, glmr);
	abc_alias (glpd2, glpd);
	abc_alias (glsh2, glsh);
	abc_alias (glsd2, glsd);

	abc_alias (glmr3, glmr);	/* Only for glln copy */

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
		/*
		 * General Ledger Company Consolidation
		 */
		sprintf (err_str, " %s ", ML (mlGlMess052));
		rv_pr (err_str, 23 ,0 ,1);

		line_at (1,0,80);

		box (0, 2, 79, 2);

		line_at (20,0,80);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		line_at (22,0,80);
	
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
	if (LCHECK ("src_co"))
	{
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (comr_rec.co_no, local_rec.src_co);

        cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess130)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Not a consolidated company
		 */
		if (comr_rec.consolidate == FALSE)
		{
			print_mess (ML (mlGlMess143));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.src_name, comr_rec.co_name);
		DSP_FLD ("src_name");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Process (void)
{
	DelComr ();
	DelGlca ();
	DelGlbd ();
	DelGlsh ();	/* Also copies glsd */
	DelGlmr ();	/* Also copies glpd */
}

void
DelComr (void)
{
return;
	memset (&comr_rec, 0, sizeof (comr_rec));
	strcpy (comr_rec.co_no,   local_rec.src_co);
	strcpy (comr_rec.co_name, local_rec.src_name);
	comr_rec.consolidate = TRUE;

    cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
        file_err (cc, comr, "DBFIND");
    
	cc = abc_delete (comr);
	if (cc)
        file_err (cc, comr, "DBDELETE");
}

void 
DelGlca (void)
{
	strcpy (glcaRec.co_no, local_rec.src_co);
	glcaRec.level_no 	= 0;
	glcaRec.acc_no 		= 0L;

    cc = find_rec (glca, &glcaRec, GTEQ, "u");
	while (!cc && !strcmp (glcaRec.co_no, local_rec.src_co))
	{
		cc = abc_delete (glca);
		if (cc)
            file_err (cc, glca, "DBDELETE");
        
		strcpy (glcaRec.co_no, local_rec.src_co);
		glcaRec.level_no 	= 0;
		glcaRec.acc_no 		= 0L;
		cc = find_rec (glca, &glcaRec, GTEQ, "u");
	}
	abc_unlock (glca);
}

void 
DelGlbd (void)
{
	strcpy (glbdRec.co_no, local_rec.src_co);
	glbdRec.budg_no = 0;

    cc = find_rec (glbd, &glbdRec, GTEQ, "u");
	while (!cc && !strcmp (glbdRec.co_no, local_rec.src_co))
	{
		cc = abc_delete (glbd);
		if (cc)
        {
            file_err (cc, glbd, "DBDELETE");
        }
		strcpy (glbdRec.co_no, local_rec.src_co);
		glbdRec.budg_no = 0;
		cc = find_rec (glbd, &glbdRec, GTEQ, "u");
	}
	abc_unlock (glbd);
}

void 
DelGlsh (void)
{
	long src_hash;

	strcpy (glsh_rec.co_no, local_rec.src_co);
	strcpy (glsh_rec.code, "      ");

    cc = find_rec (glsh, &glsh_rec, GTEQ, "u");
	while (!cc && !strcmp (glsh_rec.co_no, local_rec.src_co))
	{
		src_hash = glsh_rec.hhsh_hash;

		cc = abc_delete (glsh);
		if (cc)
            file_err (cc, glsh, "DBDELETE");

		DelGlsd (src_hash);

		strcpy (glsh_rec.co_no, local_rec.src_co);
		strcpy (glsh_rec.code, "      ");
		cc = find_rec (glsh, &glsh_rec, GTEQ, "u");
	}
	abc_unlock (glsh);
}

void 
DelGlsd (
 long src_hash)
{
	glsd_rec.hhsh_hash 	= src_hash;
	glsd_rec.prd_no 	= 0;

    cc = find_rec (glsd, &glsd_rec, GTEQ, "u");
	while (!cc && glsd_rec.hhsh_hash == src_hash)
	{
		cc = abc_delete (glsd);
		if (cc)
            file_err (cc, glsh, "DBDELETE");

		glsd_rec.hhsh_hash 	= src_hash;
		glsd_rec.prd_no 	= 0;
		cc = find_rec (glsd, &glsd_rec, GTEQ, "u");
	}
	abc_unlock (glsd);
}

void 
DelGlmr (void)
{
	long src_hash;

	strcpy (glmrRec.co_no, local_rec.src_co);
	sprintf (glmrRec.acc_no, "%16.16s", " ");

    cc = find_rec (glmr, &glmrRec, GTEQ, "u");
	printf ("[%s][%s][%d]", glmrRec.co_no, glmrRec.acc_no, cc);getchar();
	while (!cc && !strcmp (glmrRec.co_no, local_rec.src_co))
	{
		src_hash = glmrRec.hhmr_hash;
		cc = abc_delete (glmr);
		if (cc)
            file_err (cc, glmr, "DBDELETE");
        
		DelGlpd (src_hash);
		DelGltr (src_hash);
		DelGlln (src_hash);
	
		strcpy (glmrRec.co_no, local_rec.src_co);
		sprintf (glmrRec.acc_no, "%16.16s", " ");
		cc = find_rec (glmr, &glmrRec, GTEQ, "u");
	}
	abc_unlock (glmr);
}

void 
DelGlpd (
	long	src_hash)
{
	glpdRec.hhmr_hash 	= src_hash;
	glpdRec.budg_no 	= 1;
	glpdRec.year 		= 0;
	glpdRec.prd_no 		= 0;

    cc = find_rec (glpd, &glpdRec, GTEQ, "u");
	while (!cc && glpdRec.hhmr_hash == src_hash)
	{
		cc = abc_delete (glpd);
		if (cc)
            file_err (cc, glmr, "DBDELETE");
        
		glpdRec.hhmr_hash 	= src_hash;
		glpdRec.budg_no 	= 1;
		glpdRec.year 		= 0;
		glpdRec.prd_no 		= 0;
		cc = find_rec (glpd, &glpdRec, GTEQ, "u");
	}
	abc_unlock (glpd);
}

void 
DelGltr (
 long src_hash)
{
	gltrRec.hhmr_hash = src_hash;
	gltrRec.tran_date = 0;

    cc = find_rec (gltr, &gltrRec, GTEQ, "u");
	while (!cc && gltrRec.hhmr_hash == src_hash)
	{
		cc = abc_delete (gltr);
		if (cc)
            file_err (cc, gltr, "DBDELETE");
        
		gltrRec.hhmr_hash = src_hash;
		gltrRec.tran_date = 0;
		cc = find_rec (gltr, &gltrRec, GTEQ, "u");
	}
	abc_unlock (gltr);
}

void 
DelGlln (
 long src_hash)
{
	gllnRec.parent_hash = src_hash;
	gllnRec.child_hash 	= 0L;

    cc = find_rec (glln, &gllnRec, GTEQ, "r");
	while (!cc && gllnRec.parent_hash == src_hash)
	{
		cc = abc_delete (glln);
		if (cc)
            file_err (cc, glmr, "DBDELETE");

		gllnRec.parent_hash = src_hash;
		gllnRec.child_hash 	= 0L;
		cc = find_rec (glln, &gllnRec, GTEQ, "r");
	}
}

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

	sprintf (local_rec.src_co, "%-2.2s", temp_str);
	return;
}

/* [ end of file ] */
