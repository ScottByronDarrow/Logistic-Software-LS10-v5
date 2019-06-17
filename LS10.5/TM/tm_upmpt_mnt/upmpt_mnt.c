/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: upmpt_mnt.c,v 5.3 2001/11/14 06:07:18 scott Exp $
|  Program Name  : (tm_upmpt_mnt.c)
|  Program Desc  : (Maintain User Defined Prompts)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 26/07/91         |
|---------------------------------------------------------------------|
| $Log: upmpt_mnt.c,v $
| Revision 5.3  2001/11/14 06:07:18  scott
| Updated to convert to app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: upmpt_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_upmpt_mnt/upmpt_mnt.c,v 5.3 2001/11/14 06:07:18 scott Exp $";

/*===============================
|   Include File dependencies   |
===============================*/
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

/*=======================================
|   Some constants, defines and stuff   |
=======================================*/
#define	TXT_REQD

#define	HEADER_SCN	1
#define	TELE_SCN	2
#define	NOTES_SCN	3
#define	USER_SCN	4

#define	ADD_P		0
#define	DEL_P		1
#define	BOX_TOP		3
#define	LST_PRMPT	11

extern	int		TruePosition;

int     no_of_prmpts = 0;
char    wk_fldname [7];

/*  QUERY NOTES
    these would be better if declared const char*
    to minimize potential errors/bugs
*/
char    *data = "data";

#include	"schema"

struct commRecord	comm_rec;
struct tmudRecord	tmud_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct 
{
	char 	dummy [11];
	char	data_str [61];
	char	u_prmpt [12][15];
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "user1", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt One     ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [0]}, 
	{1, LIN, "user2", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Two     ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [1]}, 
	{1, LIN, "user3", 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Three   ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [2]}, 
	{1, LIN, "user4", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Four    ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [3]}, 
	{1, LIN, "user5", 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Five    ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [4]}, 
	{1, LIN, "user6", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Six     ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [5]}, 
	{1, LIN, "user7", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Seven   ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [6]}, 
	{1, LIN, "user8", 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Eight   ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [7]}, 
	{1, LIN, "user9", 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Nine    ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [8]}, 
	{1, LIN, "user10", 13, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Ten     ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [9]}, 
	{1, LIN, "user11", 14, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Eleven  ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [10]}, 
	{1, LIN, "user12", 15, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Prompt Twelve  ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.u_prmpt [11]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*===============================
|   Local function prototypes   |
===============================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	DisablePrompt 	(void);
int  	Update 			(void);
int  	LoadUserPrompts (void);
int  	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv [])
{
	TruePosition	=	TRUE;
	
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	init_vars (1);

	OpenDB (); 	

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (!prog_exit)	
	{
		entry_exit  = FALSE;
		edit_exit   = FALSE;
		prog_exit   = FALSE;
		restart     = FALSE;
		search_ok   = TRUE;
		abc_unlock (tmud);

		/*-----------------------------
		| Load User Defined Prompts . |
		-----------------------------*/
		LoadUserPrompts ();
		
		if (no_of_prmpts == 0)
		{
			no_of_prmpts = 12;
			init_vars (1);
			heading (1);
			entry (1);
			if (prog_exit || restart)
            {
				continue;
            }
		}

		DisablePrompt ();			
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
        {
			continue;
        }

		Update ();
		prog_exit = TRUE;
	}
	
    shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmud, tmud_list, TMUD_NO_FIELDS, "tmud_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (tmud);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int	i;
	int	prmpt_no;

	if (LNCHECK ("user", 4))
	{
		prmpt_no = atoi (&FIELD.label [4]) - 1;

		if (dflt_used && 
           (prmpt_no < no_of_prmpts))
		{
			if (prog_status != ENTRY)
			{
				if (prmpt_no + 1 < no_of_prmpts)
				{
					for (i = prmpt_no; i < no_of_prmpts - 1; i++)
					{
						sprintf (local_rec.u_prmpt [i], "%-14.14s", 
												local_rec.u_prmpt [i + 1]);
					}
				}	

				sprintf (local_rec.u_prmpt [no_of_prmpts - 1], "%-14.14s", " ");

				no_of_prmpts--;
	
				scn_write (1);
				scn_display (1);
				DisablePrompt ();			

				return (EXIT_SUCCESS);
			}
			else
			{
				no_of_prmpts = prmpt_no;
				entry_exit   = TRUE;
				return (EXIT_SUCCESS);
			}
		}

		if (strlen (clip (local_rec.u_prmpt [prmpt_no])) == 0)
		{
			print_mess (ML (mlTmMess064));
			sleep (sleepTime);	
			clear_mess ();
			return (EXIT_FAILURE);
		}

		no_of_prmpts++;
		DisablePrompt ();			
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}	
int
DisablePrompt (void)
{
	int	i;

	for (i = 0; i < 12; i++)
	{
		sprintf (wk_fldname, "user%d", i + 1);
		FLD (wk_fldname) = NO;
		if (strlen (clip (local_rec.u_prmpt [i])) == 0)
        {
			break;
        }
	}

	if (i >= 11)
    {
		return (EXIT_SUCCESS);
    }

	for (i++; i < 12; i++)
	{
		sprintf (wk_fldname, "user%d", i + 1);
		FLD (wk_fldname) = NE;
	}

	return (EXIT_SUCCESS);
}

/*----------------
| Update Record. |
----------------*/
int
Update (void)
{
	int	i;

	if (no_of_prmpts == 1 && 
        strlen (clip (local_rec.u_prmpt [0])) == 0)
    {
		no_of_prmpts = 0;
    }

	for (i = 0; i < no_of_prmpts; i++)
	{
		strcpy (tmud_rec.co_no, comm_rec.co_no);
		tmud_rec.line_no = i;

		cc = find_rec (tmud, &tmud_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (tmud_rec.prmpt_desc, "%-14.14s", local_rec.u_prmpt [i]);
			strcpy (tmud_rec.stat_flag, "0");

			cc = abc_add (tmud, &tmud_rec);
			if (cc)
				file_err (cc, tmud, "DBADD");
		}
		else
		{
			sprintf (tmud_rec.prmpt_desc, "%-14.14s", local_rec.u_prmpt [i]);
			strcpy (tmud_rec.stat_flag, "0");

			cc = abc_update (tmud, &tmud_rec);
			if (cc)
				file_err (cc, tmud, "DBUPDATE");
		}
	}

	strcpy (tmud_rec.co_no, comm_rec.co_no);
	tmud_rec.line_no = no_of_prmpts;

	cc = find_rec (tmud, &tmud_rec, GTEQ, "u");
	while (!cc && 
           !strcmp (tmud_rec.co_no, comm_rec.co_no))
	{
		cc = abc_delete (tmud);
		if (cc)
			file_err (cc, tmud, "DBUPDATE");

		strcpy (tmud_rec.co_no, comm_rec.co_no);
		tmud_rec.line_no = no_of_prmpts;
		cc = find_rec (tmud, &tmud_rec, GTEQ, "u");
	}

	return (EXIT_SUCCESS);
}

int
LoadUserPrompts (void)
{
	int	i = 0;

	strcpy (tmud_rec.co_no, comm_rec.co_no);
	tmud_rec.line_no = 0;
	cc = find_rec (tmud, &tmud_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (tmud_rec.co_no, comm_rec.co_no) && 
           i < 12)
	{
		sprintf (local_rec.u_prmpt [i++], "%-14.14s", tmud_rec.prmpt_desc);

		cc = find_rec (tmud, &tmud_rec, NEXT, "r");
	}

	no_of_prmpts = i;

	if (i <= 0)
        return (EXIT_SUCCESS);

	for (i = no_of_prmpts; i < 12; i++)
		sprintf (local_rec.u_prmpt [i], "%-14.14s", " ");

	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	box (0,3,80, 12);

	rv_pr (ML (mlTmMess072),25,0,1);
	line_at (1,0,80);
	line_at (20,0,80);

	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/* [ end of file ] */
