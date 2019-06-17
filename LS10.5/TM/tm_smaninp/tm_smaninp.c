/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tm_smaninp.c,v 5.4 2002/07/25 11:17:38 scott Exp $
|  Program Name  : (tm_smaninp.c)
|  Program Desc  : (Maintain Area / Class, Salesman, Instruction) 
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 27/07/91         |
|---------------------------------------------------------------------|
| $Log: tm_smaninp.c,v $
| Revision 5.4  2002/07/25 11:17:38  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2001/11/13 04:01:21  scott
| Updated to add app.schema
| Updated to clean program.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tm_smaninp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_smaninp/tm_smaninp.c,v 5.4 2002/07/25 11:17:38 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tm_mess.h>

	/*
	 *   Some Constants 
	 */
	char 	*data    = "data",
			*exsf2   = "exsf2";

	/*
	 * Special fields and flags.
	 */
   	int     newCode = 0;

#include	"schema"

struct commRecord	comm_rec;
struct exsfRecord	exsf_rec;
struct exsfRecord	exsf2_rec;

extern	int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct 
{
	char	dummy [11];
	char	previousCode [4];
	char	status [7];
	char	statusDesc [11];
	char	curr_logname [15];
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "sman_code", 4, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "",    "Salesman Number     ", "", 
		NE, NO, JUSTRIGHT, "", "", exsf_rec.salesman_no}, 
	{1, LIN, "sman_name", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",    "Salesman name       ", " ", 
		YES, NO, JUSTLEFT, "", "", exsf_rec.salesman}, 
	{1, LIN, "sman_logname", 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "",    "Salesman Logname    ", "", 
		YES, NO, JUSTLEFT, "", "", exsf_rec.logname}, 
	{1, LIN, "sman_stat", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N",   "Salesman Status     ", "", 
		NO, NO, JUSTLEFT, "SN", "", local_rec.status}, 
	{1, LIN, "sman_stat_desc", 7, 25, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.statusDesc}, 
	{1, LIN, "com_type", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "R",   "Commission type     ", "Must Be 'O' 'P' or 'R' ", 
		YES, NO, JUSTLEFT, "OPR", "", exsf_rec.com_type}, 
	{1, LIN, "com_pc", 10, 2, FLOATTYPE, 
		"NN.NNN", "          ", 
		" ", "0.0", "Commission %        ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&exsf_rec.com_pc}, 
	{1, LIN, "com_min", 11, 2, MONEYTYPE, 
		"NNNNN.NNN", "          ", 
		" ", "0.0", "Minimum commission  ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&exsf_rec.com_min}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 *   Local function prototypes  
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	Update 			(void);
void 	SrchExsf 		(char *);
int  	heading 		(int);

/*
 *   The main processing function   
 */
int
main (
 int argc, 
 char *argv [])
{
	TruePosition	=	TRUE;

	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB ();

	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags.
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
        {
			continue;
        }

		/*
		 * Edit screen 1 linear input. 
		 */
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();

	}
	shutdown_prog ();
    
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
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
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	abc_alias (exsf2, exsf);
	open_rec (exsf2, exsf_list, EXSF_NO_FIELDS, "exsf_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (exsf);
	abc_fclose (exsf2);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("sman_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
	
		newCode = 0;
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "u");
		if (cc) 
			newCode = 1;
		else    
		{
			if (exsf_rec.stat_flag [0] == 'S')
            {
				strcpy (local_rec.status, "S");
				strcpy (local_rec.statusDesc, ML ("Super "));
            }
			else
            {
				strcpy (local_rec.status, "N");
				strcpy (local_rec.statusDesc, ML ("Normal"));
            }

			sprintf (local_rec.curr_logname, "%-14.14s", exsf_rec.logname);

			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sman_logname"))
	{
		strcpy (exsf2_rec.co_no, comm_rec.co_no);
		sprintf (exsf2_rec.logname, "%-14.14s", temp_str);

		cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "u");
		if (!cc && strcmp (exsf_rec.logname, local_rec.curr_logname))
		{
			print_mess (ML (mlStdMess167));
			sleep (sleepTime);
			clear_mess ();
			abc_unlock (exsf2);
			return (EXIT_FAILURE);
		}
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sman_stat"))
	{
		if (local_rec.status [0] == 'S')
			strcpy (local_rec.statusDesc, ML ("Super "));
		else
			strcpy (local_rec.statusDesc, ML ("Normal"));

		DSP_FLD ("sman_stat_desc");
	}

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	/*=================================
	| Add or update salesman record . |
	=================================*/
	sprintf (exsf_rec.stat_flag, "%-1.1s", local_rec.status);
	if (newCode == 1)
	{
		cc = abc_add (exsf, &exsf_rec);
		if (cc) 
			file_err (cc, exsf, "DBADD");
	}
	else
	{
		cc = abc_update (exsf, &exsf_rec);
		if (cc) 
			file_err (cc, exsf, "DBUPDATE");
	}

    abc_unlock (exsf);
	strcpy (local_rec.previousCode, exsf_rec.salesman_no);
}

void
SrchExsf (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No","#Salesman's Name");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);

    cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
           !strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;

		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
        return;

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);

	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
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
		rv_pr (ML (mlTmMess039), 25, 0, 1);
		print_at (0, 55, ML (mlTmMess012), local_rec.previousCode);
		line_at (1,0,80);
		line_at (8,1,79);
		line_at (20,0,80);
		line_at (22,0,80);

		box (0,3,80,8);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}


/* [ end of file ] */
