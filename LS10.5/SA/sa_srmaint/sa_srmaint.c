/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sa_srmaint.c,v 5.3 2002/03/04 05:34:03 scott Exp $
|  Program Name  : (sa_srmaint.c)
|  Program Desc  : (Sales Analysis Sub Range Maintenance)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 07/11/87         |
|---------------------------------------------------------------------|
| $Log: sa_srmaint.c,v $
| Revision 5.3  2002/03/04 05:34:03  scott
| S/C 00792 - SAMR9-Maintain Subranges; CHAR-BASED / WINDOWS CLIENT (1) Run the program, delete one line and press F12, the program exits.  Then run the program again, there is no detail saved. WINDOWS CLIENT
| (2) When delete a line ( press <backspace>, then <enter ), the rest of the lines below do not clear.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_srmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_srmaint/sa_srmaint.c,v 5.3 2002/03/04 05:34:03 scott Exp $";

#include <ml_std_mess.h>
#include <ml_sa_mess.h>

#define	MAXWIDTH	132
#define TOTSCNS		1

#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct sasrRecord	sasr_rec;
struct excfRecord	excf_rec;

	char	*data = "data";

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy 	  [11];
	char	startCat  [12];
	char	startDesc [41];
	char	endCat 	  [12];
	char	endDesc   [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "startCat",	MAXLINES, 5, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", " Start Category. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCat},
	{1, TAB, "startDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Start Category Description       ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startDesc},
	{1, TAB, "endCat",	 0, 3, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", "   End Category  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, TAB, "endDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "        End Category Description        ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	LoadScreen 		(void);
int 	spec_valid 		(int);
void 	Update 			(void);
void 	SrchExcf 		(char *, char *);
int 	heading 		(int);

/*
 * Main Processing Routine 
 */
int
main (
 int    argc,
 char*  argv [])
{
	/*
	 * Setup required parameters. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	swide ();

	OpenDB ();

	/*
	 * Reset control flags 
	 */
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	search_ok 	= TRUE;

	LoadScreen ();

	/*
	 * Enter screen 1 linear input 
	 */
	if (lcount [1] == 0)
	{
		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
	}

	/*
	 * Edit screen 1 linear input 
	 */
	heading (1);
	scn_display (1);
	edit (1);

	if (!restart)
		Update ();

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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (sasr, sasr_list, SASR_NO_FIELDS, "sasr_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (excf);
	abc_fclose (sasr);
	abc_dbclose (data);
}

void
LoadScreen (void)
{
	scn_set (1);
	lcount [1] = 0;

	strcpy (sasr_rec.co_no,      comm_rec.co_no);
	sprintf (sasr_rec.start_cat, "%-11.11s", " ");
	sprintf (sasr_rec.end_cat,   "%-11.11s", " ");
	cc = find_rec (sasr, &sasr_rec, GTEQ, "u");
	while (!cc && !strcmp (sasr_rec.co_no, comm_rec.co_no))
	{
		strcpy (excf_rec.co_no,  comm_rec.co_no);
		strcpy (excf_rec.cat_no, sasr_rec.start_cat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (local_rec.startCat,  excf_rec.cat_no);
			strcpy (local_rec.startDesc, excf_rec.cat_desc);

			strcpy (excf_rec.co_no,    comm_rec.co_no);
			strcpy (excf_rec.cat_no, sasr_rec.end_cat);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (local_rec.endCat,  excf_rec.cat_no);
				strcpy (local_rec.endDesc, excf_rec.cat_desc);
				putval (lcount [1]++);
			}

		}
		cc = find_rec (sasr, &sasr_rec, NEXT, "u");
	}
}

int
spec_valid (
 int    field)
{
	int	this_page;
	int	i;

	if (LCHECK ("startCat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str,temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used && prog_status != ENTRY)
		{
			lcount [1]--;
			this_page = line_cnt / TABLINES;
			for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				if (this_page == line_cnt / TABLINES)
					line_display ();
			}
			sprintf (local_rec.startCat,  "%11.11s", " ");
			sprintf (local_rec.startDesc, "%40.40s", " ");
			sprintf (local_rec.endCat,    "%11.11s", " ");
			sprintf (local_rec.endDesc,   "%40.40s", " ");
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				line_display ();
			line_cnt = i;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.startCat, local_rec.endCat) > 0 && 
			strcmp (local_rec.endCat,   "           "))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no,    comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.startCat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startDesc, excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCat"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (local_rec.startCat,temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.startCat, local_rec.endCat) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no,    comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.endCat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endDesc, excf_rec.cat_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void
Update (void)
{
	abc_selfield (sasr, "sasr_id_no");

	clear ();

	print_at (0,0, ML (mlStdMess035));

	fflush (stdout);

	/*
	 * Delete the extra lines for the current company	
	 * only need to do this if we didnt add any above
	 */
	strcpy (sasr_rec.co_no,      comm_rec.co_no);
	sprintf (sasr_rec.start_cat, "%-11.11s", " ");
	sprintf (sasr_rec.end_cat,   "%-11.11s", " ");
	cc = find_rec (sasr, &sasr_rec, GTEQ, "u");
	while (!cc && !strcmp (sasr_rec.co_no, comm_rec.co_no))
	{
		abc_delete (sasr);
		strcpy (sasr_rec.co_no,      comm_rec.co_no);
		sprintf (sasr_rec.start_cat, "%-11.11s", " ");
		sprintf (sasr_rec.end_cat,   "%-11.11s", " ");
		cc = find_rec (sasr, &sasr_rec, GTEQ, "u");
	}
	abc_unlock (sasr);

	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);
		strcpy (sasr_rec.co_no,     comm_rec.co_no);
		strcpy (sasr_rec.start_cat, local_rec.startCat);
		strcpy (sasr_rec.end_cat,   local_rec.endCat);
		cc = abc_add (sasr, &sasr_rec);
		if (cc)
			file_err (cc, sasr, "DBADD");
	}
}

void
SrchExcf (
	char	*lowValue,
	char	*keyValue)
{
	_work_open (11,0,40);
	save_rec ("#Category","#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",keyValue);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (excf_rec.co_no, comm_rec.co_no) && 
		   !strncmp (excf_rec.cat_no, keyValue, strlen (keyValue)))
	{                        
		if (strncmp (excf_rec.cat_no, lowValue, strlen (lowValue)) >= 0)
		{
			cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);                       
			if (cc)
				break;
		}
		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSaMess018),47,0,1);

		line_at (1,0,132);
		line_at (20,0,132);
		line_at (22,0,132);

		print_at (21,0, ML (mlStdMess038) ,comm_rec.co_no,comm_rec.co_name);

		line_cnt = 0;
		scn_write (scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
