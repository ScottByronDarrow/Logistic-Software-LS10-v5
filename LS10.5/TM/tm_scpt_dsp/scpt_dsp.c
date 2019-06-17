/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: scpt_dsp.c,v 5.3 2001/11/14 09:01:42 scott Exp $
|  Program Name  : (tm_scpt_dsp.c)                                  |
|  Program Desc  : (Telemarketing Script Display                )   |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 29/07/91         |
|---------------------------------------------------------------------|
| $Log: scpt_dsp.c,v $
| Revision 5.3  2001/11/14 09:01:42  scott
| Updated to add app.schema
| Updated to clean code
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: scpt_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_scpt_dsp/scpt_dsp.c,v 5.3 2001/11/14 09:01:42 scott Exp $";

/*============================
| Include file dependencies  |
============================*/
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_tm_mess.h>

/*================================
| Some constant data definitions |
================================*/
#define	X_OFF	20
#define	Y_OFF	3

/*  NOTES
    these should be declared const char* 
    to minimize potential problems.
*/

char    *data = "data";

char    *BLNK_LINE = "                                                                              ";
char    *UNDR_LINE1 = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";
char    *UNDR_LINE2 = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";
char    *UNDR_LINE3 = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct tmshRecord	tmsh_rec;
struct tmslRecord	tmsl_rec;

int		*tmsl_rep_goto	=	&tmsl_rec.rep1_goto;

extern	int	TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct 
{
	char    dummy [11];
	int     scr_no;
	char    scr_desc [41];
	int     lpno;
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "scr_no", 4, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "", "Script  ", "Enter Script To Display.", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.scr_no}, 
	{1, LIN, "scr_desc", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.scr_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*=============================
|  Local function prototypes  |
==============================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	SrchTmsh 		(char *);
int  	Process 		(void);
int  	heading 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int    argc, 
 char  *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();             /*  sets terminal from termcap	*/
	set_tty ();              /*  get into raw mode           */

	OpenDB ();

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

    /*  QUERY
        who sets the initial value of brain dead global 
        prog_exit? are we to assume that this variable is 
        correct at the start of the loop?
    */
	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
        /*  NOTES
            these flags are better off incorporated into a 
            global struct instead of lying around like a 
            local variable.
            something along the lines of :

                typedef struct 
                {
            		int search_ok,
		            int entry_exit, 
                    int prog_exit, 
                    int restart,
                    ...etc...etc..
                } global_t;

                extern global_t Global;

             access then would be along the lines of:

                Global.search_ok    = 1;
                Global.prog_exit    = 0;
                ...etc...etc...

             or better yet:
                
                Global.search_ok    = TRUE;
                Global.prog_exit    = FALSE;
                ...etc...etc...
        */
		search_ok   = 1;    
		entry_exit  = 1;
		prog_exit   = 0;
		restart     = 0;
		init_vars (1);	

		heading (1);
		entry (1);

		if (prog_exit || restart)
        {
			continue;
        }

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
        {
			Process ();
        }
	}

	shutdown_prog (); 

    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}


void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (tmsh, tmsh_list, TMSH_NO_FIELDS, "tmsh_id_no");
	open_rec (tmsl, tmsl_list, TMSL_NO_FIELDS, "tmsl_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose (tmsh);
	abc_fclose (tmsl);
	abc_dbclose (data);
}

/*  QUERY STD_RETURN
    assumed the return value of 0 means EXIT_SUCCESS and
    assumed the return value of 1 means EXIT_FAILURE
*/
int 
spec_valid (
 int field)
{

	if (LCHECK ("scr_no"))
	{
		if (SRCH_KEY)
		{
			SrchTmsh (temp_str);

			return (EXIT_SUCCESS);
		}

		strcpy (tmsh_rec.co_no,comm_rec.co_no);
		tmsh_rec.script_no = local_rec.scr_no;
        
		cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------
			| Script Not Found On File |
			--------------------------*/
			print_mess (ML (mlTmMess002));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}
		sprintf (local_rec.scr_desc, "%-40.40s", tmsh_rec.desc);
		
		DSP_FLD ("scr_desc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchTmsh (
 char *key_val)
{
	char	tmp_scr_no [6];

	_work_open (4,0,40);
	strcpy (tmsh_rec.co_no,comm_rec.co_no);
	tmsh_rec.script_no = atoi (key_val);
	save_rec ("#No","#Script Description.");

	cc = find_rec (tmsh, &tmsh_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (tmsh_rec.co_no, comm_rec.co_no) &&
		   tmsh_rec.script_no >= atoi (key_val))
	{
		sprintf (tmp_scr_no, "%04d", tmsh_rec.script_no);
		cc = save_rec (tmp_scr_no, tmsh_rec.desc);
		if (cc)
		{
			break;
		}

		cc = find_rec (tmsh, &tmsh_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		return;
	}

	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = atoi (temp_str);

	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
    {
		file_err (cc, tmsh, "DBFIND");
    }
}

int 
Process (
 void)
{
	int	i;

	strcpy (tmsh_rec.co_no, comm_rec.co_no);
	tmsh_rec.script_no = local_rec.scr_no;

	cc = find_rec (tmsh, &tmsh_rec, COMPARISON, "r");
	if (cc)
	{
		/*----------------------------------
		| Can't Find Script Header Record |
		----------------------------------*/
		print_mess (ML (mlTmMess002));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE); /* QUERY STD_RETURN 
                           return FALSE???!
                           what are the functions supposed to 
                           return? TRUE/FALSE or success/failure?
                        */
	}

	/*-----------------
	| Open Dsp window |
	-----------------*/
	Dsp_prn_open (0, 3, 15,
		          "                        Telemarketing Script Display                            ", 
                 (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);

	sprintf (err_str,
             "    Script Number: %5d   Name:  %-40.40s    ",
             tmsh_rec.script_no,
             tmsh_rec.desc);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec (" [PRINT][NEXT][PREV][EDIT/END]  ");

	tmsl_rec.hhsh_hash = tmsh_rec.hhsh_hash;
	tmsl_rec.prmpt_no = 0;
	cc = find_rec (tmsl, &tmsl_rec, GTEQ, "r");
	while (!cc && 
          (tmsl_rec.hhsh_hash == tmsh_rec.hhsh_hash))
	{
		sprintf (err_str, " Prompt Number: %5d   %-30.30s",
			     tmsl_rec.prmpt_no,
			     tmsl_rec.desc);
		Dsp_saverec (err_str);

		sprintf (err_str, "          %-72.72s", tmsl_rec.text1);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text2);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text3);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text4);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text5);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text6);
		Dsp_saverec (err_str);
		sprintf (err_str, "          %-72.72s", tmsl_rec.text7);
		Dsp_saverec (err_str);

		Dsp_saverec (UNDR_LINE2);
		strcpy (err_str, " Reply     Description        Next    ^E  Reply     Description        Next ");
		Dsp_saverec (err_str);
		Dsp_saverec (UNDR_LINE3);

		for (i = 0; i < 8; i += 2)
		{
			char	dispStr1 [41];
			char	dispStr2 [41];
			switch (i)
			{
				case	0:
					 strcpy (dispStr1, tmsl_rec.rep1_desc);
					 strcpy (dispStr2, tmsl_rec.rep2_desc);
				break;
				case	2:
					 strcpy (dispStr1, tmsl_rec.rep3_desc);
					 strcpy (dispStr2, tmsl_rec.rep4_desc);
				break;
				case	4:
					 strcpy (dispStr1, tmsl_rec.rep5_desc);
					 strcpy (dispStr2, tmsl_rec.rep6_desc);
				break;
				case	6:
					 strcpy (dispStr1, tmsl_rec.rep7_desc);
					 strcpy (dispStr2, tmsl_rec.rep8_desc);
				break;
			}
			sprintf (err_str,
				     "   %1d     %-20.20s %3d     ^E    %1d    %-20.20s %3d",
				     i + 1,
				     dispStr1,
				     tmsl_rep_goto [i],
				     i + 2,
				     dispStr2,
				     tmsl_rep_goto [i + 1]);
			Dsp_saverec (err_str);
		}

		cc = find_rec (tmsl, &tmsl_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();
	
    /* QUERY STD_RETURN
       what is the standard return values? 
       TRUE/FALSE or
       EXIT_SUCCESS/EXIT_FAILURE or
       something else?
    */
    return (EXIT_SUCCESS); 
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
/* NOTES
   the standard parameters are not defined.
   we would be better off using constants/defines 
   thus:
        heading (TM_SCREEN); or something like that...
*/
/*  QUERY STD_RETURN
    assumed the return value of 0 means EXIT_SUCCESS and
    assumed the return value of 1 means EXIT_FAILURE
*/

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
	
		move (0, 1);
		line (80);

		/*-------------------------------
		| Telemarketing Script Display. |
		-------------------------------*/
		sprintf (err_str, " %s ", ML (mlTmMess003));
		rv_pr (err_str, 25, 0, 1);

		box (0, 3, 80, 1);

		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}


/* [ end of file ] */
