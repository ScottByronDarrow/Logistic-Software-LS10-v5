/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_incs_mnt.c,v 5.5 2002/07/24 08:39:13 scott Exp $
|  Program Name  : (incs_maint.c)
|  Program Desc  : (Competitors Substitute Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (04/06/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: sk_incs_mnt.c,v $
| Revision 5.5  2002/07/24 08:39:13  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/20 07:11:00  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/03/05 06:47:53  scott
| .
|
| Revision 5.2  2002/03/05 06:43:13  scott
| Updated as fields not cleared.
|
| Revision 5.1  2001/12/12 03:55:08  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_incs_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_incs_mnt/sk_incs_mnt.c,v 5.5 2002/07/24 08:39:13 scott Exp $";

#define		TOTSCNS		2
#define 	MAXLINES	100
#define 	MAXWIDTH	150
#include 	<pslscr.h>
#include 	<ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct incsRecord	incs_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data";

   	int  	newItem = FALSE;

	struct storeRec {
		char	subsArray [17];
	} store [MAXLINES];

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	itemNo [17];
	char	desc [41];
	char	subsCode [17];
	char	subsDesc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNo",	 3, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNo},
	{1, LIN, "item_desc",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},

	{2, TAB, "subsCode",	MAXLINES, 10, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Competitors Substitute Code.", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.subsCode},
	{2, TAB, "subsDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Competitors Substitute Description.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.subsDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	LoadSubsCodes 	(void);
int  	DeleteLine 		(void);
void 	Update 			(void);
int  	heading 		(int);

/*
 * Main Processing Routine .
 */
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);


	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	OpenDB ();

	tab_row = 7;
	tab_col = 4;

	/*
	 * Beginning of input control loop.
	 */
	while (!prog_exit)
	{
		/*
		 * Reset control flags.
		 */
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;
		lcount [2] = 0;
		init_vars (1);
		init_vars (2);

		/*
		 * Enter screen 1 linear input  
		 */
		heading (1);
		scn_display (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		if (newItem)
		{
			heading (2);
			scn_display (2);
			entry (2);
		}

		heading (2);
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (incs, incs_list, INCS_NO_FIELDS, "incs_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (incs);
	abc_fclose (inmr);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i;
        
    if (LCHECK ("itemNo"))
    {
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.itemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.itemNo, inmr_rec.item_no);
		
		sprintf (local_rec.desc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("itemNo");
		DSP_FLD ("item_desc");

		newItem = TRUE;
		if (LoadSubsCodes ())
			newItem = FALSE;

		return (EXIT_SUCCESS);
	}

    if (LCHECK ("subsCode"))
    {
		int		numLines;

		/*
		 * Delete current line.
		 */
		if (last_char == DELLINE)
			return (DeleteLine ());

		if (dflt_used)
			return (EXIT_FAILURE);

		/*
		 * Validate for uniqueness.
		 */
		numLines = (prog_status == ENTRY) ? line_cnt : lcount [2];
		for (i = 0; i < numLines; i++)
		{
			if (prog_status != ENTRY && i == line_cnt)
				continue;

			/*
			 * Current code is already in array.
			 */
			if (!strcmp (store [i].subsArray, local_rec.subsCode))
			{
				print_mess (ML ("Substitute Code Already Exists"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		sprintf (store [line_cnt].subsArray, "%-16.16s", local_rec.subsCode);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Function    : LoadSubsCodes                            
 * Description : Loads substitute codes from incs for     
 *               the current inmr record.                 
 * Parameters  : NONE                                     
 * Returns     : TRUE if substitute codes were loaded     
 *               successfully.                            
 *               FALSE if substitute codes were not       
 *               loaded.                                  
 */
int 
LoadSubsCodes (void)
{
	int		codesFound;

	scn_set (2);
	lcount [2] = 0;

	codesFound = FALSE;
	/*
	 * Check for existing competitor codes for this item.
	 */
	strcpy (incs_rec.co_no, comm_rec.co_no);
	incs_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sprintf (incs_rec.subs_code, "%-16.16s", " ");
	cc = find_rec (incs, &incs_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (incs_rec.co_no, comm_rec.co_no) &&
		   incs_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		codesFound = TRUE;
	
		sprintf (store [lcount [2]].subsArray, "%-16.16s", incs_rec.subs_code);

		sprintf (local_rec.subsCode, "%-16.16s", incs_rec.subs_code);
		sprintf (local_rec.subsDesc, "%-40.40s", incs_rec.subs_desc);

		putval (lcount [2]++);

		cc = find_rec (incs, &incs_rec, NEXT, "r");
	}

	scn_set (1);

	return (codesFound);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	/*
	 * Entry.
	 */
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	/*
	 * No lines to delete.
	 */
	if (lcount [2] <= 0)
	{
		print_mess (ML ("There are no lines to delete."));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	/*
	 * Delete lines.
	 */
	lcount [2]--;
	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		strcpy (store [i].subsArray, store [i + 1].subsArray);

		if (line_cnt / TABLINES == this_page)
			line_display ();
	}
	/*
	 * Blank last line - if required.
	 */
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*
	 * Zap buffer if deleted all.
	 */
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

/*
 * Update competitor codes.
 */
void
Update (void)
{
	int		i;

	if (!newItem)
	{
		/*
		 * Delete existing incs records for hhbr_hash.
		 */
		strcpy (incs_rec.co_no, comm_rec.co_no);
		incs_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (incs_rec.subs_code, "%-16.16s", " ");
		cc = find_rec (incs, &incs_rec, GTEQ, "u");
		while (!cc &&
			   !strcmp (incs_rec.co_no, comm_rec.co_no) &&
			   incs_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			cc = abc_delete (incs);
			if (cc)
				file_err (cc, incs, "DBDELETE");

			strcpy (incs_rec.co_no, comm_rec.co_no);
			incs_rec.hhbr_hash = inmr_rec.hhbr_hash;
			sprintf (incs_rec.subs_code, "%-16.16s", " ");
			cc = find_rec (incs, &incs_rec, GTEQ, "u");
		}
		abc_unlock (incs);
	}

	/*
	 * Add incs records from TAB screen.
	 */
	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);

		strcpy (incs_rec.co_no, comm_rec.co_no);
		incs_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (incs_rec.subs_code, "%-16.16s", local_rec.subsCode);
		sprintf (incs_rec.subs_desc, "%-40.40s", local_rec.subsDesc);
		cc = abc_add (incs, &incs_rec);
		if (cc)
			file_err (cc, incs, "DBADD");
	}
}

int
heading (
	int	scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	strcpy (err_str, ML (" Competitors Substitute Maintenance. "));
	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);
	
	line_at (1,0,80);

	switch (scn)
	{
	case 	1:
		box (0, 2, 80, 2);

		scn_set (2);
		scn_write (2);
		scn_display (2);
		break;

	case	2:
		box (0, 2, 80, 2);

		scn_set (1);
		scn_write (1);
		scn_display (1);
		break;
	}

	line_at (20,0,80);
	line_at (22,0,80);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

