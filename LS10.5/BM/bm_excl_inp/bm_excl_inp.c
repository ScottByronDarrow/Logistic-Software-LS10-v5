/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: bm_excl_inp.c,v 5.7 2002/07/24 08:38:41 scott Exp $
|  Program Name  : (bm_excl_inp.c)
|  Program Desc  : (Maintain Class Exclusion File)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 14/08/91         |
|---------------------------------------------------------------------|
| $Log: bm_excl_inp.c,v $
| Revision 5.7  2002/07/24 08:38:41  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/18 06:09:01  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/07/08 08:09:03  scott
| ..
|
| Revision 5.4  2002/07/08 08:08:43  scott
| S/C 004065 - See S/C
|
| Revision 5.3  2002/07/08 04:58:04  scott
| S/C 004077 - Box not aligned
|
| Revision 5.2  2002/06/21 03:07:33  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_excl_inp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_excl_inp/bm_excl_inp.c,v 5.7 2002/07/24 08:38:41 scott Exp $";

#define	MAXLINES	500
#include <ml_std_mess.h>
#include <ml_bm_mess.h>
#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct pcpxRecord	pcpx_rec;

struct	storeRec {
	char	workClass [5];
	char	oldClass [5];
} store [MAXLINES];

int	noLinesInOld;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	manfClass [5];
	char	excl [5];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "class", 	 4, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Class    ", "", 
		 NE, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.manfClass}, 
	{2, TAB, "excl", 	 MAXLINES, 3, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", " Exclusions ", " Enter Excluded Production Classes ", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.excl}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 
};

int	new_code = 0;

/*
 * Local Function Prototypes
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int	 	spec_valid 		(int);
int 	DeleteLine 		(void);
void 	LoadPcpx 		(void);
void 	Update 			(void);
void 	SrchPcpx 		(char *);
int	 	heading 		(int);

int
main (
	int		argc, 
	char	*argv [])
{
	TruePosition	=	TRUE;

	/*
	 * Setup required parameters. 
	 */
	SETUP_SCR (vars);


	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	OpenDB ();

	/*
	 * Beginning of input control loop 
	 */
	while (!prog_exit)
	{
		/*
		 * Reset control flags . 
		 */
		entry_exit	= 0; 
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		search_ok	= 1;
		lcount [2]	= 0;

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
		tab_row = 7;
		tab_col = 32;
		heading (2);
		LoadPcpx ();
		scn_display (2);
		edit (2);

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

    read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (pcpx, pcpx_list, PCPX_NO_FIELDS, "pcpx_id_no");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (pcpx);
	abc_dbclose ("data");
}


int
spec_valid (
	int		field)
{
	int	i;

	/*
	 * Validate Instruction Code. 
	 */
	if (LCHECK ("class"))
	{
		if (SRCH_KEY)
		{
			SrchPcpx (temp_str);
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("excl"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (!strcmp (local_rec.excl, local_rec.manfClass))
		{
			print_mess (ML (mlBmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		for (i = 0; i < lcount [2]; i++)
		{
			if (!strcmp (store [i].workClass, local_rec.excl))
			{
				print_mess (ML (mlBmMess002));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (store [lcount [2]].workClass, local_rec.excl);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		strcpy (store [line_cnt].workClass, local_rec.excl);

		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

void
LoadPcpx (void)
{
	scn_set (2);
	init_vars (2);
	lcount [2] = 0;

	strcpy (pcpx_rec.co_no, comm_rec.co_no);
	sprintf (pcpx_rec.pcpx_class, local_rec.manfClass);
	sprintf (pcpx_rec.excl_class, "    ");
	cc = find_rec (pcpx, &pcpx_rec, GTEQ, "r");
	while (!cc && 
 	       !strcmp (pcpx_rec.co_no, comm_rec.co_no) &&
 	       !strcmp (pcpx_rec.pcpx_class, local_rec.manfClass))
	{
		strcpy (store [lcount [2]].workClass, pcpx_rec.excl_class);
		strcpy (store [lcount [2]].oldClass,  pcpx_rec.excl_class);
		strcpy (local_rec.excl, pcpx_rec.excl_class);
		putval (lcount [2]++);

		cc = find_rec (pcpx, &pcpx_rec, NEXT, "r");
	}
	noLinesInOld = lcount [2];
}

void
Update (void)
{
	int	i, j;
	int	deleteExcl;

	/*
	 * Delete Old Combinations 
	 */
	for (i = 0; i < noLinesInOld; i++)
	{
		deleteExcl = TRUE;
		for (j = 0; j < lcount [2]; j++)
		{
			if (!strcmp (store [j].workClass, store [i].oldClass))
			{
				deleteExcl = FALSE;
				break;
			}
		}

		if (deleteExcl)
		{
			/*
			 * Delete Class/Exclusion Combination If It Exists 
			 */
			strcpy (pcpx_rec.co_no, comm_rec.co_no);
			sprintf (pcpx_rec.pcpx_class, local_rec.manfClass);
			sprintf (pcpx_rec.excl_class, store [i].oldClass);
			cc = find_rec (pcpx, &pcpx_rec, COMPARISON, "u");
			if (!cc)
			{
				cc = abc_delete (pcpx);
				if (cc)
					file_err (cc, pcpx, "DBDELETE");
			}

			/*
			 * Delete Exclusion/Class Combination If It Exists	 
			 */
			strcpy (pcpx_rec.co_no, comm_rec.co_no);
			sprintf (pcpx_rec.pcpx_class, store [i].oldClass);
			sprintf (pcpx_rec.excl_class, local_rec.manfClass);
			cc = find_rec (pcpx, &pcpx_rec, COMPARISON, "u");
			if (!cc)
			{
				cc = abc_delete (pcpx);
				if (cc)
					file_err (cc, pcpx, "DBDELETE");
			}
		}
	}

	/*
	 * Add New Combinations 
	 */
	for (i = 0; i < lcount [2]; i++)
	{
		/*
		 * Add Class/Exclusion Combination if It Does NOT Exist Already.
		 */
		strcpy (pcpx_rec.co_no, comm_rec.co_no);
		sprintf (pcpx_rec.pcpx_class, local_rec.manfClass);
		sprintf (pcpx_rec.excl_class, store [i].workClass);
		cc = find_rec (pcpx, &pcpx_rec, COMPARISON, "u");
		if (cc)
		{
			cc = abc_add (pcpx, &pcpx_rec);
			if (cc)
				file_err (cc, pcpx, "DBADD");
		}
		
		/*
		 * Add Exclusion/Class Combination if It Does NOT Exist Already. 
		 */
		strcpy (pcpx_rec.co_no, comm_rec.co_no);
		sprintf (pcpx_rec.pcpx_class, store [i].workClass);
		sprintf (pcpx_rec.excl_class, local_rec.manfClass);
		cc = find_rec (pcpx, &pcpx_rec, COMPARISON, "u");
		if (cc)
		{
			cc = abc_add (pcpx, &pcpx_rec);
			if (cc)
				file_err (cc, pcpx, "DBADD");
		}
	}

}

void
SrchPcpx (
	char	*key_val)
{
	_work_open (4,0,20);
	save_rec ("#Code", "#");
	strcpy (pcpx_rec.co_no, comm_rec.co_no);
	sprintf (pcpx_rec.pcpx_class, "%-1.1s", key_val);
	cc = find_rec (pcpx, &pcpx_rec, GTEQ, "r");
	while (!cc && !strcmp (pcpx_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pcpx_rec.pcpx_class,pcpx_rec.excl_class);
		if (cc)
			break;

		cc = find_rec (pcpx, &pcpx_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pcpx_rec.co_no, comm_rec.co_no);
	sprintf (pcpx_rec.pcpx_class, "%-1.1s", key_val);
	cc = find_rec (pcpx, &pcpx_rec, GTEQ, "r");
	if (cc)
		file_err (cc, pcpx, "DBFIND");
}

int
heading (
 int                scn)
{

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		if (scn == 1)
			clear ();

		rv_pr (ML (mlBmMess010), 25, 0, 1);
		line_at (1, 0, 80);

		box (0, 3, 80, 1);
		/*
		if (scn == 2)
			box (32, 6, 14, 12);
		*/


		line_at (20, 0, 80);
		line_at (22, 0, 80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
