/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: dsp_indexes.c,v 5.2 2001/08/09 05:13:23 scott Exp $
|  Program Name  : ( dsp_files.c     )                                |
|  Program Desc  : ( Display files and links.                     )   |
-----------------------------------------------------------------------
| $Log: dsp_indexes.c,v $
| Revision 5.2  2001/08/09 05:13:23  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:30  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 1.2  2000/09/01 09:06:34  scott
| Updated to make small changes from testing
|
| Revision 1.1  2000/09/01 08:45:38  scott
| New program to display files and linking indexes.
| Ensure that data is loaded, unload file is held in source directory.
|
=====================================================================*/

#define	CCMAIN
char	*PNAME = "$RCSfile: dsp_indexes.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dsp_indexes/dsp_indexes.c,v 5.2 2001/08/09 05:13:23 scott Exp $";

#include 	<pslscr.h>

#include	"schema"

struct dbindexesRecord	dbindexes_rec;
struct dbindexesRecord	dbindexes2_rec;


struct	{
	char	dummy[11];
	char	fileName [16];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "fileName", 4, 15, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "File Name.", " Default : All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.fileName}, 
	{1, LIN, "fileDesc", 5, 15, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "File Desc.", " ", 
		NA, NO, JUSTLEFT, "", "", dbindexes_rec.dbi_desc}, 
	{0, LIN, "dummy", 4, 20, CHARTYPE, 
		"A", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.dummy}, 

};

/*========================
| Function prototypes    |
========================*/
int		main			 (int argc, char * argv []);
int		heading 		 (int);
void	shutdown_prog	 (void);
void	ShowFiles		 (char *);
void	ProcessFiles	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);

int
main (
 int	argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 

	OpenDB ();
	set_masks ();

	while (!prog_exit)
	{
		search_ok	= TRUE;
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		ProcessFiles ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int	field)
{
	if (!strcmp (FIELD.label,"fileName"))
	{
		if (SRCH_KEY)
		{
			ShowFiles (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (dbindexes_rec.dbi_serial, "Y");
		strcpy (dbindexes_rec.dbi_filename,local_rec.fileName);
		cc = find_rec (dbindexes, &dbindexes_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML ("Selected file not found or does not have serial fields"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("fileDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
ShowFiles (
 char *	key_val)
{
	work_open ();
	
	save_rec ("#File Name.     ","#File Description");

	/*--------------------------
	| Flush record buffer first |
	---------------------------*/
	memset (&dbindexes_rec, 0, sizeof (dbindexes_rec));

	sprintf (dbindexes_rec.dbi_filename, "%-15.15s", key_val);
	strcpy (dbindexes_rec.dbi_serial, "Y");
    cc = find_rec (dbindexes, &dbindexes_rec,  GTEQ,"r");
	while (!cc && dbindexes_rec.dbi_serial [0] == 'Y' &&
		   !strncmp (dbindexes_rec.dbi_filename, key_val, strlen (clip (key_val))))
    {
        cc = save_rec (dbindexes_rec.dbi_filename, dbindexes_rec.dbi_desc);
        if (cc)
            break;

         cc = find_rec (dbindexes, &dbindexes_rec,  NEXT, "r");
    }

    cc = disp_srch ();
    work_close ();
    if (cc)
        return;

	strcpy (dbindexes_rec.dbi_serial, "Y");
	sprintf (dbindexes_rec.dbi_filename, "%-15.15s", temp_str);
    cc = find_rec (dbindexes, &dbindexes_rec,  COMPARISON, "r");
    if (cc)
       file_err (cc, "dbindexes", "DBFIND");
}
void
ProcessFiles (void)
{
	char	displayString [200];

	strcpy (dbindexes_rec.dbi_serial, "Y");
	sprintf (dbindexes_rec.dbi_filename, local_rec.fileName);
	cc = find_rec (dbindexes, &dbindexes_rec, COMPARISON, "r");
	if (cc)
		return;

	sprintf (displayString,"File : %15.15s - %60.60s -  Serial Field (%16.16s)%8.8s",
			dbindexes_rec.dbi_filename,
			dbindexes_rec.dbi_desc,
			dbindexes_rec.dbi_field,
			" ");
			
	Dsp_open (0,0,18);
	Dsp_saverec (displayString);
	Dsp_saverec ("               | Link to file. |Link to file Description                                   | Link from field -> Link to field   ");
	Dsp_saverec ("  [REDRAW]   [NEXT SCN]   [PREV SCN]   [EDIT/END]  ");

	strcpy (dbindexes2_rec.dbi_serial, "N");
	sprintf (dbindexes2_rec.dbi_linkto_file, local_rec.fileName);
	cc = find_rec ("dbindexes2", &dbindexes2_rec, GTEQ, "r");
	while (!cc && !strcmp ( dbindexes2_rec.dbi_linkto_file, local_rec.fileName))
	{
		sprintf (displayString, "               |%15.15s|%60.60s|%16.16s -> %16.16s",
		dbindexes2_rec.dbi_filename,
		dbindexes2_rec.dbi_desc,
		dbindexes2_rec.dbi_field,
		dbindexes2_rec.dbi_linkto);

		Dsp_saverec (displayString);
		cc = find_rec ("dbindexes2", &dbindexes2_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		rv_pr (ML ("Database tables serial link display") ,45,0,1);

		move (0,1);
		line (131);

		box (0,3,132,2);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	abc_alias ("dbindexes2", dbindexes);
	open_rec (dbindexes,   dbindexes_list, DBINDEXES_NO_FIELDS, "dbi_id_no");
	open_rec ("dbindexes2",dbindexes_list, DBINDEXES_NO_FIELDS, "dbi_id_no2");
}
/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (dbindexes);
	abc_fclose ("dbindexes2");
	abc_dbclose ("data");
}
