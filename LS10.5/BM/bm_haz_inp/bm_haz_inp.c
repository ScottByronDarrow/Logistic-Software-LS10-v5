/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: bm_haz_inp.c,v 5.4 2002/07/25 11:17:26 scott Exp $
|  Program Name  : (bm_haz_inp.c)
|  Program Desc  : (Maintain Hazard Class / Handling Class)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 14/08/91         |
|---------------------------------------------------------------------|
| $Log: bm_haz_inp.c,v $
| Revision 5.4  2002/07/25 11:17:26  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2002/03/05 02:49:41  scott
| S/C 00825 - BOMMT10 - Maintain Handling Class. The header title of the program should be  'Handling Class Maintenance'.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_haz_inp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_haz_inp/bm_haz_inp.c,v 5.4 2002/07/25 11:17:26 scott Exp $";

#include <pslscr.h>
#include <ml_bm_mess.h>
#include <ml_std_mess.h>

#define	HAZARD	 (inputType [0] == 'Z')
#define	HNDLNG	 (inputType [0] == 'H')

	char	inputType [2];

   	int	new_code = 0;

#include	"schema"

struct commRecord	comm_rec;
struct pchcRecord	pchc_rec;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	prev_code [5];
	char	classType [5];
	char	desc [41];
	char	c_prmpt [100];
} local_rec;

extern	int	TruePosition;
static	struct	var	vars []	={	
	{1, LIN, "class", 4, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", local_rec.c_prmpt, "", 
		NE, NO, JUSTRIGHT, "", "", local_rec.classType}, 
	{1, LIN, "desc", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};
/*
 * Local Function Prototypes
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	SrchPchc 		(char *);
int	 	spec_valid 		(int);
int  	heading 		(int);

int
main (
	int		argc, 
	char	*argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, mlBmMess016, argv [0]);
        return (EXIT_FAILURE);
	}

	/*
	 * Printer Number
	 */
	sprintf (inputType, "%-1.1s", argv [1]);

	if (!HAZARD && !HNDLNG)
	{
		print_at (0, 0, mlBmMess016, argv [0]);
        return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	/*
	 * Setup required parameters.
	 */
	SETUP_SCR (vars);

	if (HAZARD)
		strcpy (local_rec.c_prmpt, "Hazard Class    ");
	else
		strcpy (local_rec.c_prmpt, "Handling Class  ");

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*
	 * Beginning of input control loop 
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit	= 0; 
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		search_ok	= 1;

		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 linear input
		 */
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();

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
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (pchc, pchc_list, PCHC_NO_FIELDS, "pchc_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (pchc);
	abc_dbclose ("data");
}


int
spec_valid (
	int		field)
{
	/*
	 * Validate Instruction Code.
	 */
	if (LCHECK ("class"))
	{
		if (SRCH_KEY)
		{
			SrchPchc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pchc_rec.co_no, comm_rec.co_no);
		strcpy (pchc_rec.type, inputType);
		sprintf (pchc_rec.pchc_class, local_rec.classType);
		cc = find_rec (pchc, &pchc_rec, COMPARISON, "u");
		if (cc) 
			new_code = 1;
		else    
		{
			new_code = 0;
			sprintf (local_rec.desc, "%-40.40s", pchc_rec.desc);
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	/*
	 * Add or update area record
	 */
	strcpy (pchc_rec.co_no, comm_rec.co_no);
	strcpy (pchc_rec.type, inputType);
	sprintf (pchc_rec.pchc_class, "%-4.4s", local_rec.classType);
	sprintf (pchc_rec.desc, "%-40.40s", local_rec.desc);
	if (new_code)
	{
		cc = abc_add (pchc, &pchc_rec);
		if (cc) 
			file_err (cc, pchc, "DBADD");
	}
	else
	{
		cc = abc_update (pchc, &pchc_rec);
		if (cc) 
			file_err (cc, pchc, "DBUPDATE");
	}
	abc_unlock (pchc);
	strcpy (local_rec.prev_code, pchc_rec.pchc_class);
}

void
SrchPchc (
	char	*keyValue)
{
	_work_open (4,0,40);
	save_rec ("#Code", "#Description");
	strcpy (pchc_rec.co_no, comm_rec.co_no);
	strcpy (pchc_rec.type, inputType);
	sprintf (pchc_rec.pchc_class, "%-4.4s", keyValue);

	cc = find_rec (pchc, &pchc_rec, GTEQ, "r");
	while (!cc && !strcmp (pchc_rec.co_no, comm_rec.co_no) &&
	      !strcmp (pchc_rec.type, inputType) &&
	      !strncmp (pchc_rec.pchc_class, keyValue, strlen (keyValue)))
	{
		cc = save_rec (pchc_rec.pchc_class, pchc_rec.desc);
		if (cc)
			break;

		cc = find_rec (pchc, &pchc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pchc_rec.co_no, comm_rec.co_no);
	strcpy (pchc_rec.type, inputType);
	sprintf (pchc_rec.pchc_class, "%-4.4s", keyValue);
	cc = find_rec (pchc, &pchc_rec, GTEQ, "r");
	if (cc)
		file_err (cc, pchc, "DBFIND");
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

		if (HAZARD)
		{
			rv_pr (ML (mlBmMess014), 25, 0, 1);

			print_at (0, 55, ML (mlBmMess015), local_rec.prev_code);
			box (0, 3, 80, 2);
			line_at (1,0,80);

		}

		if (HNDLNG)
		{
			rv_pr (ML (mlBmMess051), 20, 0, 1);

			print_at (0, 56, ML (mlBmMess015), local_rec.prev_code);
			line_at (1,0,80);
			box (0, 3, 80, 2);
		}

		line_at (20,0,80);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		line_at (22,0,80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
