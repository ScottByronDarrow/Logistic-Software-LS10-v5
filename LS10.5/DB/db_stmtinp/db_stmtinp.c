/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_stmtinp.c,v 5.3 2001/12/06 09:55:03 scott Exp $
|  Program Name  : (db_stmtinp.c) 
|  Program Desc  : (Add / Maintain Statement Messages)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 28/01/87.        |
|---------------------------------------------------------------------|
| $Log: db_stmtinp.c,v $
| Revision 5.3  2001/12/06 09:55:03  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_stmtinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_stmtinp/db_stmtinp.c,v 5.3 2001/12/06 09:55:03 scott Exp $";

#define TOTSCNS		1
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

#undef	MAXWIDTH
#define	MAXWIDTH	150

	/*
	 * Special fields and flags
	 */
static char 	*data   = "data";

	int		statement,
			newRecord,
			envDbCo = 0; 	

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct strmRecord	strm_rec;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char 	mesg [3][81];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "est", 5, 16, CHARTYPE, 
		"AA", "          ", 
		" ", " 0", "Branch No.", " ", 
		YES, NO, JUSTRIGHT, "", "", esmr_rec.est_no}, 
	{1, LIN, "mesg1", 7, 16, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Message Line 1", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.mesg [0]}, 
	{1, LIN, "mesg2", 8, 16, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Message Line 2", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.mesg [1]}, 
	{1, LIN, "mesg3", 9, 16, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Message Line 3", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.mesg [2]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*
 * Local Function Prototype
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	update 			(void);
void 	SrchEsmr 		(char *);
int 	heading 		(int);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	if (argc < 2)
	{
		print_at (0,0, mlDbMess165, argv [0]);
		return (EXIT_FAILURE);
	}
		
	if (*argv [1] != 'S' && *argv [1] != 'R')
	{
		print_at (0,0, mlDbMess165, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);
	statement = argv [1][0] == 'S';

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	/*
	 * Open main database files.
	 */
	OpenDB ();

	strcpy (esmr_rec.est_no,"  ");
	strcpy (esmr_rec.short_name,"               ");

	swide ();

	while (prog_exit == 0)
	{
		abc_unlock (strm);
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		newRecord = 0;
		init_vars (1);

		/*-------------------------------
		| Enter Screen 1 Linear Input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit)
			break;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		update ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (strm, strm_list, STRM_NO_FIELDS, "strm_id_no");
}

/*
 * Close Data Base Files .
 */
void
CloseDB (void)
{
	abc_fclose (esmr);
	abc_fclose (strm);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int	invalid = 0;

	/*-------------------------------
	| Validate Branch Number Input. |
	-------------------------------*/
	if (LCHECK ("est"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (esmr_rec.est_no," 0"))
		{
			strcpy (esmr_rec.est_no,comm_rec.est_no);
			strcpy (esmr_rec.short_name,comm_rec.est_short);
			envDbCo = 1;
		}
		else
		{
			envDbCo = 0;
			strcpy (esmr_rec.co_no,comm_rec.co_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess073));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

		print_at (22,0, ML (mlStdMess039),esmr_rec.est_no,esmr_rec.est_name);
		strcpy (strm_rec.co_no,comm_rec.co_no);
		strcpy (strm_rec.est_no,esmr_rec.est_no);
		strcpy (strm_rec.mesg_type, (statement) ? "S" : "R");

		cc = find_rec (strm, &strm_rec, COMPARISON, "u");
		if (!cc)
		{
			newRecord = 0;
			strcpy (local_rec.mesg [0],strm_rec.mesg_tx1);
			strcpy (local_rec.mesg [1],strm_rec.mesg_tx2);
			strcpy (local_rec.mesg [2],strm_rec.mesg_tx3);
			entry_exit = 1;
		}
		else
			newRecord = 1;

		return (EXIT_SUCCESS);
	}

	return (invalid);
}

void
update (void)
{
    strcpy (strm_rec.co_no,comm_rec.co_no);
    strcpy (strm_rec.est_no,esmr_rec.est_no);
    strcpy (strm_rec.mesg_type, (statement) ? "S" : "R");
    strcpy (strm_rec.mesg_tx1,local_rec.mesg [0]);
    strcpy (strm_rec.mesg_tx2,local_rec.mesg [1]);
    strcpy (strm_rec.mesg_tx3,local_rec.mesg [2]);
    strcpy (strm_rec.stat_flag,"0");

    if (newRecord)
    {
	    cc = abc_add (strm, &strm_rec);
	    if (cc)
		    sys_err ("Error in strm During (DBADD)", cc, PNAME);
    }
    else
    {
	    cc = abc_update (strm, &strm_rec);
	    if (cc)
		    sys_err ("Error in strm During (DBUPDATE)", cc, PNAME);
    }
}

/*===============================================
| Search routine for Establishment Master File. |
===============================================*/
void
SrchEsmr (
	char	*key_val)
{
	_work_open (2,0,40);

	save_rec ("#No.", "#Branch Name");

	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strncmp (esmr_rec.est_no, key_val,strlen (key_val)) && 
				  !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,temp_str);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
}

int
heading (
	int		scn)
{
	int	s_size = 132;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		fflush (stdout);

		if (statement)
			print_at (0,55, "%R %s ", ML (mlDbMess135));
		else
			print_at (0,55,  "%R %s ",ML (mlDbMess136));

		printf (ta [14]); /* rv_off (); */
        fflush (stdout);
		line_at (1,0,s_size);

		if (scn == 1)
		{
			box (0,4,131,5);
			line_at (6, 1, s_size - 2);
		}
		line_at (20,0,s_size);
		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039),esmr_rec.est_no,esmr_rec.est_name);

		line_at (23,0,s_size);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
