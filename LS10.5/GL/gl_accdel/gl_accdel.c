/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_accdel.c,v 5.3 2001/08/09 09:13:18 scott Exp $
|  Program Name  : (gl_accdel.c)
|  Program Desc  : (General Ledger Account Deletion)
|---------------------------------------------------------------------|
|  Date Written  : 13/05/92        | Author       : Simon Dubey       |
|---------------------------------------------------------------------|
| $Log: gl_accdel.c,v $
| Revision 5.3  2001/08/09 09:13:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:26:54  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:19  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_accdel.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_accdel/gl_accdel.c,v 5.3 2001/08/09 09:13:18 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>
#include <hot_keys.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <GlUtils.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
extern int   GV_link_cnt, GV_cur_level, GV_max_level;
extern int	TruePosition;

char    *data	= "data";

#include	"schema"

struct commRecord	comm_rec;

static struct
{
	char	loc_acc [FORM_LEN + 1];
	char	dummy [11];
} local_rec;

struct var vars [] =
{
	{1, LIN, "acctNo",	2, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ",
		" ", " ", "Account number    ", " ",
		 NE, NO,  JUSTLEFT, "0123456789*-", "", local_rec.loc_acc},
	{1, LIN, "acctDesc",	2, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "Name   ", " ",
		NA, NO,  JUSTLEFT, "", "", glmrRec.desc},
	{1, LIN, "accCurr",	3, 2, CHARTYPE,
		"AAA", "             ",
		" ", " ", "Currency          ", " ",
		NA, NO,  JUSTLEFT, "", "", glmrRec.curr_code},

	{0, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
int  	ValidAccountNumber 	(void);
void 	DeleteFunction 		(void);
int  	NoChildren 			(void);
int  	NoFChildren 		(void);
int  	CheckBalance 		(void);
int  	DeleteAccount 		(void);
void 	DeleteLink 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv [])
{
	TruePosition = TRUE;

	SETUP_SCR (vars);

	init_scr ();		
	set_tty ();
	OpenDB ();
	vars [label ("acctNo")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
	set_masks ();	
	clear ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (!prog_exit) 
	{
		prog_exit     = FALSE;
		search_ok     = TRUE;
		init_ok       = TRUE;
		edit_exit     = FALSE;
		restart       = FALSE;
		entry_exit    = FALSE;

		init_vars (1);
		heading (1);
		entry (1);
		scn_display (1);

		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		if (restart)
        {
			continue;
        }

		DeleteFunction ();

	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
    crsr_toggle (TRUE); 
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlmr ();
	OpenGlpd ();
	OpenGlln ();

	abc_selfield (glln, "glln_id_no2");
}

void
CloseDB (
 void)
{
	GL_Close ();
	abc_dbclose (data);
}

/*==============================
| Display Headings             |
==============================*/
int
heading (
 int scn)
{
	if (restart)
		return 0;

	clear ();

	if (scn != cur_screen)
		scn_set (scn);

	centre_at (0,80,ML (mlGlMess062));
	box (1,1,79,3);

	line_at (21,0,80);
	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("acctNo")) 
    {
        return (ValidAccountNumber ());
    }
			
	return (EXIT_SUCCESS);
}

int
ValidAccountNumber (
 void)
{
	if (vars [label ("acctNo")].mask[0] == '*')
	{
		print_mess (ML (mlGlMess001));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (dflt_used)
    {
		strcpy (local_rec.loc_acc, GL_GetDfltSaccCode ());
    }

	if (SRCH_KEY)
	{
		if (strlen (temp_str) == 0)
        {
			strcpy (temp_str, GL_GetfUserCode ());
        }
		return SearchGlmr_F (comm_rec.co_no, temp_str, "***");
	}

	if (GL_FormAccNo (local_rec.loc_acc, glmrRec.acc_no, FALSE))
    {
		return (EXIT_FAILURE);
    }
	
	cc = ReadGlmr (comm_rec.co_no, &glmrRec, "u", COMPARISON);
	if (cc)
	{
		errmess (ML (mlStdMess024));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Each function should return TRUE if result is favourable for deletion.
 */
void
DeleteFunction (void)
{
	char response;

	if (NoChildren ())
	{
		if (CheckBalance ())
		{
			/*
			 * If alright to delete will prompt user to confirm.
			 */
			response = prmptmsg (mlGlMess063, "YyNn", 1, 18);
			if (response == 'Y' || response == 'y')
            {
				if (DeleteAccount ())  
                {
					DeleteLink ();
                }
            }
		}  
	}
}

int	
NoChildren (void)
{
	if (GV_cur_level == GV_max_level)
		return (TRUE);

	return NoFChildren ();
}

int	
NoFChildren (void)
{
	int	 not_found = FALSE;
    int  lvl_cnt;
    char tmp_accno [MAXLEVEL + 1];
	GLMR_STRUCT	tmp_glmr;

	tmp_accno [0] = (char) NULL;

	for (lvl_cnt = 0; lvl_cnt < GV_cur_level; lvl_cnt++)
    {
		strcat (tmp_accno, GL_GetBit (lvl_cnt + 1));
    }

	strcpy (tmp_glmr.co_no, comm_rec.co_no);
	GL_StripForm (tmp_glmr.acc_no, local_rec.loc_acc);
	cc = ReadGlmr (comm_rec.co_no, &tmp_glmr, "r", GTEQ);
	if (cc || ReadGlmr (comm_rec.co_no, &tmp_glmr, "r", NEXT) ||
		strcmp (tmp_glmr.co_no, comm_rec.co_no) ||
		strncmp (tmp_glmr.acc_no, tmp_accno, strlen (tmp_accno)))
	{
		not_found = TRUE;
	}

    cc = ReadGlmr (comm_rec.co_no, &glmrRec, "u", COMPARISON);
	if (cc)
		file_err (cc,  glmr, "DBFIND");

	if (!not_found)
	{
		errmess (ML (mlStdMess025));
		sleep (sleepTime);
	}

	return (not_found);
}

int
CheckBalance (void)
{
	glpdRec.hhmr_hash 	= glmrRec.hhmr_hash;
	glpdRec.budg_no 	= 0;
	glpdRec.year    	= 0;
	glpdRec.prd_no  	= 0;

	/*-----------------------------------------
	| Tests to see if a period has a balance  |
	-----------------------------------------*/
	if ( (!find_rec (glpd, &glpdRec, GTEQ,"r")) &&
	     (glpdRec.hhmr_hash == glmrRec.hhmr_hash))
        {
	      	errmess (ML (mlGlMess061));
			sleep (sleepTime);
	      	return (FALSE);
        }
    
    return (TRUE);
}

int
DeleteAccount (void)
{
	cc = ReadGlmr (comm_rec.co_no, &glmrRec, "u", COMPARISON);
	if (cc)
		file_err (cc,  glmr, "DBFIND");

	cc = abc_delete (glmr) ;
	if (cc)
		file_err (cc,  glmr, "DBDELETE");

	return (TRUE);
}

void
DeleteLink (void)
{
	while (1) 
	{
		gllnRec.child_hash 	= glmrRec.hhmr_hash;
		gllnRec.parent_hash = 0L;
        cc = find_rec (glln, &gllnRec, GTEQ, "u");
		if (!cc && 
            (gllnRec.child_hash == glmrRec.hhmr_hash))
		{
			cc = abc_delete (glln);
			if (cc)
				file_err (cc,  glln, "DBDELETE");
		}
		else
            return;
	}
}
