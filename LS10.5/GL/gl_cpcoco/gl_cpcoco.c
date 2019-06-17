/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_cpcoco.c,v 5.4 2001/08/09 09:13:40 scott Exp $
|  Program Name  : (gl_cpcoco.c) 
|  Program Desc  : (General Ledger Copy Company to Company)
|---------------------------------------------------------------------|
|  Author        : Trev van Bremen | Date Written  : 15/11/90         |
|---------------------------------------------------------------------|
| $Log: gl_cpcoco.c,v $
| Revision 5.4  2001/08/09 09:13:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/09 01:42:09  scott
| RELEASE 5.0
|
| Revision 5.2  2001/08/06 23:27:16  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:44  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_cpcoco.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_cpcoco/gl_cpcoco.c,v 5.4 2001/08/09 09:13:40 scott Exp $";

/*
 *   Include file dependencies  
 */

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct glshRecord	glsh_rec;
struct glsdRecord	glsd_rec;

/*
 *   Constants, defines and stuff   
 */

#define		MAXLEVEL	16

	char	*data  = "data",
			*glbd2 = "glbd2",
			*glca2 = "glca2",
			*glln2 = "glln2",
			*glmr2 = "glmr2",
			*glmr3 = "glmr3",
			*glpd2 = "glpd2",
			*glsh2 = "glsh2",
			*glsd2 = "glsd2";

	GLMR_STRUCT glmr3Rec;
	GLLN_STRUCT glln2Rec;

extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	char	sourceCoNo		 [3];
	char	sourceCoName	 [41];
	char	destinCoNo		 [3];
	char	destinCoName	 [41];
	char	budget		 [2];
	char 	dummy		 [10];
} local_rec;

static struct var vars [] =
{
	{1, LIN, "sourceCoNo", 3, 2, CHARTYPE, 
		"AA", "          ",
		" ", "",  "Source Company      ", " ", 
		YES, NO, JUSTRIGHT, "1", "99", local_rec.sourceCoNo},
	{1, LIN, "sourceCoName", 3, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",  "", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.sourceCoName},
	{1, LIN, "destinCoNo", 4, 2, CHARTYPE, 
		"AA", "          ", " ", 
		"", "Destination Company  ", " ",
		YES, NO, JUSTRIGHT, "1", "99", local_rec.destinCoNo},
	{1, LIN, "destinCoName", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",  "", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.destinCoName},
	{1, LIN, "budget", 5, 2, CHARTYPE, "U", "          ",
		" ", "Y", "Copy budget Values  ", " ",
		YES, NO, JUSTLEFT, "YN", "",
		local_rec.budget},

	{0, LIN, "", 0, 0, INTTYPE, "A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ",
		local_rec.dummy}
};

/*
 *   Local function prototypes  
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	Process 		(void);
void 	CopyGlca 		(void);
void 	CopyGlbd 		(void);
void 	CopyGlsh 		(void);
void 	CopyGlsd 		(long, long);
void 	CopyGlmr 		(void);
void 	CopyGlpd 		(long, long);
void 	CopyGlln 		(void);
void 	MakeGlln2 		(void);
void 	SrchComr 		(char *);
int		heading 		(int);
int		spec_valid 		(int);

/*
 * Main Processing Routine.
 */
int
main (
 int argc,
 char *argv [])
{
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

	strcpy (local_rec.sourceCoNo, comm_rec.co_no);
	strcpy (local_rec.destinCoNo, "  ");
	strcpy (local_rec.budget, "Y");

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
		init_ok 	= FALSE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
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

		/*
		 * Process g/l records.
		 */
		Process ();
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

	abc_alias (glmr3, glmr);	/* Only for glln copy */

	OpenGlbd ();
	OpenGlca ();
	OpenGlln ();
	OpenGlmr ();
	OpenGlpd ();

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
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
	abc_fclose (comr);
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
	GL_Close ();

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

		rv_pr (ML (mlGlMess013) ,23 ,0 ,1);

		line_at (1,0,80);

		box (0, 2, 79, 3);

		line_at (20,0,80);

		sprintf (err_str, 
                ML (mlStdMess038),
                comm_rec.co_no, 
                clip (comm_rec.co_name));
        print_at (21,0, err_str);

        sprintf (err_str, 
                ML (mlStdMess039),
                comm_rec.est_no, 
                clip (comm_rec.est_name));
		print_at (22,0, err_str);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

int
spec_valid (
	int		field)
{
	int	i;

	/*
	 * Validate Destination company.
	 */
	if (LCHECK ("destinCoNo"))
	{
		if (!strcmp (local_rec.sourceCoNo, local_rec.destinCoNo))
		{
			print_mess (ML (mlGlMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (comr_rec.co_no, local_rec.destinCoNo);

        cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Company not found
			 */
			print_mess (ML (mlStdMess130)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Check for Chart of Accounts.
		 */
		strcpy (glcaRec.co_no, local_rec.destinCoNo);
		glcaRec.level_no	= 0;
		glcaRec.acc_no 		= 0L;

        cc = find_rec (glca, &glcaRec, GTEQ, "r");
		if (!cc && !strcmp (glcaRec.co_no, local_rec.destinCoNo))
		{
			print_mess (ML (mlGlMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check for company control record.
		 */
		strcpy (glmrRec.co_no, local_rec.destinCoNo);
		strcpy (glmrRec.acc_no, "0000000000000000");

        cc = find_rec (glmr, &glmrRec, COMPARISON, "w");
		if (!cc && !strcmp (glmrRec.co_no, local_rec.destinCoNo))
		{
			i = prmptmsg (ML (mlGlMess016), "YyNn", 0, 23);

			move (0, 23);
			cl_line ();
			if (i == 'N' || i == 'n')
			{
				abc_unlock (glmr);
				return (EXIT_FAILURE);
			}
		
			cc = abc_delete (glmr);
			if (cc)
                file_err (cc, glmr, "DBDELETE");
		}
		strcpy (local_rec.destinCoName, comr_rec.co_name);
		DSP_FLD ("destinCoName");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Destination company.
	 */
	if (LCHECK ("sourceCoNo"))
	{
		strcpy (comr_rec.co_no, local_rec.sourceCoNo);

		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
        cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Company not found
			 */
			print_mess (ML (mlStdMess130)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.sourceCoName, comr_rec.co_name);
		DSP_FLD ("sourceCoName");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Process (void)
{
	print_mess (ML ("Processing request"));
	sleep (sleepTime);
	CopyGlca ();
	CopyGlbd ();
	CopyGlsh ();	
	CopyGlmr ();
	CopyGlln ();
	print_mess (ML ("Copy complete"));
	sleep (sleepTime);
}

void
CopyGlca (void)
{
	strcpy (glcaRec.co_no, local_rec.sourceCoNo);
	glcaRec.level_no 	= 0;
	glcaRec.acc_no 		= 0L;

    cc = find_rec (glca, &glcaRec, GTEQ, "r");
	while (!cc && !strcmp (glcaRec.co_no, local_rec.sourceCoNo))
	{
		strcpy (glcaRec.co_no, local_rec.destinCoNo);
		cc = abc_add (glca2, &glcaRec);
		if (cc)
            file_err (cc, glca2, "DBADD");

		strcpy (glcaRec.co_no, local_rec.sourceCoNo);
		cc = find_rec (glca, &glcaRec, NEXT, "r");
	}
}

void
CopyGlbd (void)
{
	strcpy (glbdRec.co_no, local_rec.sourceCoNo);
	glbdRec.budg_no = 0;

    cc = find_rec (glbd, &glbdRec, GTEQ, "r");
	while (!cc && !strcmp (glbdRec.co_no, local_rec.sourceCoNo))
	{
		strcpy (glbdRec.co_no, local_rec.destinCoNo);
		cc = abc_add (glbd2, &glbdRec);
		if (cc)
            file_err (cc, glbd2, "DBADD");
        
		strcpy (glbdRec.co_no, local_rec.sourceCoNo);
		cc = find_rec (glbd, &glbdRec, NEXT, "r");
	}
}

void
CopyGlsh (void)
{
    long 	src_hash,
         	dst_hash;

	strcpy (glsh_rec.co_no, local_rec.sourceCoNo);
	strcpy (glsh_rec.code, "      ");

    cc = find_rec (glsh, &glsh_rec, GTEQ, "r");
	while (!cc && !strcmp (glsh_rec.co_no, local_rec.sourceCoNo))
	{
		src_hash = glsh_rec.hhsh_hash;

		strcpy (glsh_rec.co_no, local_rec.destinCoNo);
		cc = abc_add (glsh2, &glsh_rec);
		if (cc)
            file_err (cc, glsh2, "DBADD");

		cc = find_rec (glsh2, &glsh_rec, EQUAL, "r");
		if (cc)
            file_err (cc, glsh2, "DBFIND");

		dst_hash = glsh_rec.hhsh_hash;

		CopyGlsd (src_hash, dst_hash);

		strcpy (glsh_rec.co_no, local_rec.sourceCoNo);
		cc = find_rec (glsh, &glsh_rec, NEXT, "r");
	}
}

void
CopyGlsd (
	long	src_hash, 
	long	dst_hash)
{
	glsd_rec.hhsh_hash = src_hash;
	glsd_rec.prd_no 	= 0;

    cc = find_rec (glsd, &glsd_rec, GTEQ, "r");
	while (!cc && glsd_rec.hhsh_hash == src_hash)
	{
		glsd_rec.hhsh_hash = dst_hash;
		cc = abc_add (glsd2, &glsd_rec);
		if (cc)
            file_err (cc, glsd2, "DBADD");

		glsd_rec.hhsh_hash = src_hash;
		cc = find_rec (glsd, &glsd_rec, NEXT, "r");
	}
}

void
CopyGlmr (void)
{
	long src_hash,
         dst_hash;

	strcpy (glmrRec.co_no, local_rec.sourceCoNo);
	sprintf (glmrRec.acc_no, "%16.16s", " ");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc && !strcmp (glmrRec.co_no, local_rec.sourceCoNo))
	{
		src_hash = glmrRec.hhmr_hash;
		strcpy (glmrRec.co_no, local_rec.destinCoNo);

		glmrRec.mod_date	=	TodaysDate ();
		cc = abc_add (glmr2, &glmrRec);
		if (cc)
            file_err (cc, glmr2, "DBADD");

		cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
		if (cc)
            file_err (cc, glmr2, "DBFIND");

		dst_hash = glmrRec.hhmr_hash;
		if (local_rec.budget [0] == 'Y')
            CopyGlpd (src_hash, dst_hash);

		strcpy (glmrRec.co_no, local_rec.sourceCoNo);
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

void
CopyGlpd (
	long	src_hash, 
	long	dst_hash)
{
	glpdRec.hhmr_hash 	= src_hash;
	glpdRec.budg_no 	= 1;
	glpdRec.year 		= 0;
	glpdRec.prd_no 		= 0;

    cc = find_rec (glpd, &glpdRec, GTEQ, "r");
	while (!cc && glpdRec.hhmr_hash == src_hash)
	{
		glpdRec.hhmr_hash = dst_hash;

        cc = abc_add (glpd2, &glpdRec);
		if (cc)
            file_err (cc, glpd2, "DBADD");

		glpdRec.hhmr_hash = src_hash;
		cc = find_rec (glpd, &glpdRec, NEXT, "r");
	}
}

void
CopyGlln (void)
{
	strcpy (glmrRec.co_no, local_rec.sourceCoNo);
	sprintf (glmrRec.acc_no, "%16.16s", " ");

    cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc && !strcpy (glmrRec.co_no, local_rec.sourceCoNo))
	{
		gllnRec.parent_hash = glmrRec.hhmr_hash;
		gllnRec.child_hash 	= 0L;

        cc = find_rec (glln, &gllnRec, GTEQ, "r");
		while (!cc && gllnRec.parent_hash == glmrRec.hhmr_hash)
		{
			glmr3Rec.hhmr_hash = gllnRec.child_hash;

            cc = find_rec (glmr3, &glmr3Rec, GTEQ, "r");
			if (!cc && glmr3Rec.hhmr_hash == gllnRec.child_hash)
            {
                MakeGlln2 ();
            }
			cc = find_rec (glln, &gllnRec, NEXT, "r");
		}

		strcpy (glmrRec.co_no, local_rec.sourceCoNo);
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

void
MakeGlln2 (void)
{
	strcpy (glmrRec.co_no, local_rec.destinCoNo);
	cc = find_rec (glmr2, &glmrRec, EQUAL, "r");
	if (!cc)
	{
		glln2Rec.parent_hash = glmrRec.hhmr_hash;
		strcpy (glmr3Rec.co_no, local_rec.destinCoNo);
		cc = find_rec (glmr2, &glmr3Rec, EQUAL, "r");
		glln2Rec.child_hash = glmr3Rec.hhmr_hash;
	
		cc = abc_add (glln2, &glln2Rec);
		if (cc)
        	file_err (cc, glln2, "DBADD");
	}
}

void 
SrchComr (
 char *key_val)
{
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
		sprintf (err_str,"%-40.40s %-3.3s",comr_rec.co_name,comr_rec.base_curr);
		cc = save_rec (comr_rec.co_no, err_str);
		if (cc)
            break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
        return;

	sprintf (local_rec.sourceCoNo, "%-2.2s", temp_str);
	return;
}
