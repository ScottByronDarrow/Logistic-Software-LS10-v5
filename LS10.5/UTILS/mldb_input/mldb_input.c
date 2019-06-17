/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: mldb_input.c,v 5.0 2002/05/08 01:25:47 scott Exp $
|  Program Name  : (mldb_input.c)
|  Program Desc  : (Multi-Lingual Conversion Program)
|---------------------------------------------------------------------|
|  Author        : Kaarlo Suanes.  | Date Written  : 12/14/01         |
|---------------------------------------------------------------------|
| $Log: mldb_input.c,v $
| Revision 5.0  2002/05/08 01:25:47  scott
| CVS administration
|
| Revision 1.1  2002/02/12 10:03:21  kaarlo
| Initial check-in.
|
|
=====================================================================*/
#define CCMAIN
char    *PNAME = "$RCSfile: mldb_input.c,v $";
char    *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/mldb_input/mldb_input.c,v 5.0 2002/05/08 01:25:47 scott Exp $";

#include <pslscr.h>
#include <ml_gl_mess.h>
#include <tabdisp.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <minimenu.h>
#ifdef GVISION
#else
	#include <dirent.h>
#endif

#include "schema"

/*----------+
| Constants |
+----------*/

#define SCNPOSOFF		1
#define LIMIT_LINES		20000
#define ADD				1
#define CHANGE			2
#define DELETE			0

#define MLDB_ENTRY		0
#define MLDB_DOWNLOAD	1
#define MLDB_UPLOAD		2
#define MLDB_QUIT		3
#define default			99

const char *fiftyspaces  = "/home/kaarlo/kaloi/files/                                                                          ";

/*-----------------+
| Global variables |
+-----------------*/

char *mldbTab 	= "mldbTab",
	 *dtlsTab	= "dtlsTab",
	 *data		= "data";
char details [121];
int	 mldbTotRec; 
int  menuExitInd = FALSE;
char tmpLine [131];
int  deleteInd = FALSE;
int  FileCount;
char FileList [100][255];
char DirFileName [150];
int	 ProgramOption;
static int iProgKey;
char debugtxt [255];

/*-----------+
| Screen Tab |
+-----------*/

static int EditFunc		(int, KEY_TAB *);
static int DeleteFunc	(int, KEY_TAB *);
static int UpdateFunc	(int, KEY_TAB *);
static int QuitFunc		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB mldbKeys [] =
{
	{ " Edit English ", 'E', EditFunc, "Edit the current line",},
	{ " Delete ", 'D', DeleteFunc, "Delete records",},
	{ " [FN16] Save All ", FN16, UpdateFunc, "Save all changes to this account",},
	{ " Quit ", 'Q', QuitFunc, "Quit all changes to this account",}
};
#else
static 	KEY_TAB mldbKeys [] =
{
	{ " Edit [E]nglish ", 'E', EditFunc, "Edit the current line",},    
	{ " [D]elete ", 'D', DeleteFunc, "Delete records",},
	{ " [FN16] Save All", FN16, UpdateFunc, "Save all changes to this account",},
	{ " [Q]uit ", 'Q', QuitFunc, "Quit all changes to this account",}
};  
#endif

/*----------------------------------+
| Screen Generator Field Label Text |
+----------------------------------*/

struct tag_mldbRecord
{
	char	lu_prmpt [121];
	char	text [5][131];
	char	pname [21];
	int 	org_len;
	int 	hide;
}	mldb_rec;

/*---------------------------+
| Local and Screen Structure |
+---------------------------*/

struct {
	char    lu_prmpt [121];
    char    text [5][131];
	char    pname [21];
	int     org_len;
	int     hide;
	char	edit_text [131];
	int		update_ind;
}   mldb_store [LIMIT_LINES];

struct {
	char	text [131];
	int		lang_no;
	char	start_no [2],
			end_no [2];
	char	curr_start_no [2],
			curr_end_no [2];
	int		curr_lang_no;
	char	dummy [11];
	char	file_name [100];
} local_rec;

static  struct  var vars [] =
{
    {1, LIN, "lang_no", 2, 25, INTTYPE,
        "N", "          ",
        " ", " ", "Enter Language Number :", " ",
        NE, NO, JUSTLEFT, "", "", (char *)&local_rec.lang_no},
	{1, LIN, "start", 2, 58, CHARTYPE,
		"U", "          ",
		" ", " ", "Start Range (A-Z)     :", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.start_no},
    {1, LIN, "end", 2, 90, CHARTYPE,
		"U", "          ",
		" ", "~", "End Range (A-Z)       :", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.end_no},
#ifdef GVISION
	{2, LIN, "FileName", 4, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "File Name :", 
		"Enter Filename to be Downloaded/Uploaded",
		YES, YES,  JUSTLEFT, "", "", local_rec.file_name},
#else
	{2, LIN, "FileName", 4, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "File Name :", 
		"Enter Filename to be Downloaded/Uploaded [Search Available]",
		YES, YES,  JUSTLEFT, "", "", local_rec.file_name},
#endif
	{0, LIN, "",  0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "" , "", local_rec.dummy},
};

/*--------------------------+
| Local Function Prototypes |
+--------------------------*/

void OpenDB				 (void);
void shutdown_prog  	 (void);
void CloseDB			 (void);
int  spec_valid			 (int);
int  heading			 (int);
void LoadRecords		 (void);
void DisplayRecords 	 (void);
void erase_btm_scn		 (void);
#ifdef GVISION
#else
	int	 OpenFileList 		 (void);
	void SrchFiles			 (char *);
#endif
void DownloadProg		 (void);
void UploadProg			 (void);
void ValidateFileName	 (void);
char *PercentToThilde	 (char *);
char *ThildeToPercent	 (char *);
void ProgramMenu		 (void);

/*========================+
| Main Processing Routine |
+========================*/

int
main ( 
	int     argc,
 	char*	argv[])
{
	char lang_no [2];
	iProgKey = 1;

	while (menuExitInd == 0)
	{
		SETUP_SCR (vars);
		init_scr ();
		set_tty ();
		set_masks ();

		menuExitInd = FALSE;
		clear ();
		swide ();

		switch (iProgKey)
		{
		case	1 :
				OpenDB ();
				prog_exit = FALSE;
	       		while (prog_exit == 0)
				{
					clear ();
					swide ();
				
					prog_exit = FALSE;
					restart = FALSE;
					search_ok = TRUE;

            		strcpy (local_rec.curr_start_no, local_rec.start_no);
					strcpy (local_rec.curr_end_no, local_rec.end_no);
					local_rec.curr_lang_no = local_rec.lang_no;
					init_vars (1);
					heading (1);

					if (!deleteInd)
						entry (1);
					else
					{
						sprintf (lang_no, "%i", local_rec.curr_lang_no);
						print_at (2, 28, lang_no);
						print_at (2, 61, local_rec.curr_start_no);
						print_at (2, 93, local_rec.curr_end_no);
						local_rec.lang_no = local_rec.curr_lang_no;
					}

					if (restart)
						continue;

					if (prog_exit) 
					{
						ProgramMenu ();
						prog_exit = TRUE;
						continue;
					}

					if (!restart)
					{
						if (!deleteInd)
						{
							LoadRecords ();
						}
						deleteInd = FALSE;
						DisplayRecords ();
					}
				}
				shutdown_prog ();
				break;	
		case	2 :
				OpenDB ();
				FileCount = 0;
#ifdef GVISION
#else
				OpenFileList ();
#endif
				prog_exit = FALSE;
				while (prog_exit == 0)
				{
					clear ();
					swide ();
	
					prog_exit = FALSE;
					restart = FALSE;
					search_ok = TRUE;
					ProgramOption = 2;

					init_vars (2);
					heading (2);
					entry (2);
				
					if (restart)
						continue;

					if (prog_exit)
					{
						ProgramMenu ();
						prog_exit = TRUE;
						continue;
					}
					
					ValidateFileName ();
					DownloadProg ();
				}
				shutdown_prog ();
				break;
		case	3 :
				OpenDB ();
				FileCount = 0;
#ifdef GVISION
#else
				OpenFileList ();
#endif
				prog_exit = FALSE;
				while (prog_exit == 0)
				{
					clear ();
					swide ();

					prog_exit = FALSE;
					restart = FALSE;
					search_ok = TRUE;
					ProgramOption = 3;

					init_vars (2);
					heading (2);
					entry (2);
					
					if (restart)
						continue;

					if (prog_exit)
					{
						ProgramMenu ();
						prog_exit = TRUE;
						continue;
					}

					ValidateFileName ();
					UploadProg ();
				}
				shutdown_prog ();
				break;
		case	4 :
				clear ();
				snorm ();
				menuExitInd = TRUE;
		}
	}
	snorm ();
	return (EXIT_SUCCESS);
}

/*====================+
| Open Database Files |
+====================*/

void
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec (mldb, mldb_list, MLDB_NO_FIELDS, "mldb_lu_prmpt");
}

/*======================+
| Program Exit Sequence |
+======================*/

void
shutdown_prog (void)
{
	CloseDB ();
	FinishProgram ();
}

/*=====================+
| Close Database Files |
+=====================*/

void
CloseDB (void)
{
	abc_fclose (mldb);
	abc_dbclose ("data");
}

/*===========+
| Spec Valid |
+===========*/

int
spec_valid (
	int	field)
{
	int ctr;

	if (LCHECK ("lang_no"))
	{
		if ((local_rec.lang_no > 5) || (local_rec.lang_no <= 0))
		{
			print_mess ("Input Range for Language Number [1-5]");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);			
		}
	}

	if (LCHECK ("FileName"))
	{

#ifdef GVISION
#else
		if (SRCH_KEY)
		{
			SrchFiles (clip (temp_str));
			return (EXIT_SUCCESS);
		}
#endif
	
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}
	
		ctr = strlen (clip (local_rec.file_name));

		if (strlen (clip (local_rec.file_name)) == 0)
		{
			print_mess ("Filename must be input");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*================================+
| Standard Screen Heading Routine |
+================================*/

int
heading (
	int scn)
{
	int s_size = 132;

	if (!restart)
	{
		clear ();
		scn_set (scn);
		
		switch (scn)
		{
		case	1:
	            print_at (0, 47, "%R %s",
						  ML ("Multi-Lingual Conversion Entry Screen "));
			  	box (0, 1, s_size, 1);
				break;
		case	2:
				if (ProgramOption == 2)
					print_at (0, 57, "%R %s",
						 	  ML ("Download Program "));
				else
					print_at (0, 58, "%R %s",
							  ML ("Upload Program "));
				line_at (1, 1, 130);
				box (0, 3, 131, 2);
		}
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*====================================================+
| Function loads data from mldb into mldb_store array |
+====================================================*/

void 
LoadRecords (void)
{
	int ctr = 0;
	int mldbCtr = 0;

	sprintf (mldb_rec.lu_prmpt, "%-120.120s", local_rec.start_no);

	cc = find_rec (mldb, &mldb_rec, GTEQ, "r");
	if (cc)
	{
		abc_unlock (mldb);
		print_mess (ML (mlGlMess024));
		sleep (sleepTime);
		return;
	}

	//while (!cc && ctr < 25)
	while (!cc)
	{
		ctr ++;

		if (mldb_rec.hide)
		{
			cc = find_rec (mldb, &mldb_rec, NEXT, "r");
			continue;
		}

		if (toupper (mldb_rec.lu_prmpt [0]) < toupper (local_rec.start_no [0]))
		{
			cc = find_rec (mldb, &mldb_rec, NEXT, "r");
			continue;
		}

		if (toupper (mldb_rec.lu_prmpt [0]) > toupper (local_rec.end_no [0]))
		{
			cc = find_rec (mldb, &mldb_rec, NEXT, "r");
			continue;
		}

		strcpy (mldb_store [mldbCtr].lu_prmpt, mldb_rec.lu_prmpt);
		strcpy (mldb_store [mldbCtr].text [0], mldb_rec.text [0]);
		
		switch (local_rec.lang_no)
		{
		case	1 :
				strcpy (mldb_store [mldbCtr].edit_text, mldb_rec.text [0]);
				break;
		case	2 :
				strcpy (mldb_store [mldbCtr].edit_text, mldb_rec.text [1]);
				break;
		case	3 :
				strcpy (mldb_store [mldbCtr].edit_text, mldb_rec.text [2]);
				break;
		case	4 :
				strcpy (mldb_store [mldbCtr].edit_text, mldb_rec.text [3]);
				break;
		case	5 :
				strcpy (mldb_store [mldbCtr].edit_text, mldb_rec.text [4]);
				break;
		}
		
		mldb_store [mldbCtr].update_ind = -1;
		
		mldbCtr ++;
		cc = find_rec (mldb, &mldb_rec, NEXT, "r");

#ifdef GVISION
#else
		sprintf (err_str, "%06d", mldbCtr);
		dsp_process ("Reading Lines", err_str);
#endif

	}
	mldbTotRec = mldbCtr;
	abc_unlock (mldb);
}

/*===============================================================+
| Function displays data from mldb_store array into tab routines |
+===============================================================*/

void 
DisplayRecords (void)
{
	int mldbCtr = 0;

	tab_open (mldbTab, mldbKeys, 3, SCNPOSOFF, 12, FALSE);
	switch (local_rec.lang_no)
	{
	case	1 :
			tab_add (mldbTab, "# English                                                       | English                                                        ");
			break;
	case	2 :
	case	3 :
			tab_add (mldbTab, "# English                                                       | Chinese                                                        ");
			break;
	case	4 :
	case 	5 :
			tab_add (mldbTab, "# English                                                       | English                                                        ");
			break;
	}

	while (mldbCtr < mldbTotRec)
	{
		tab_add 
		 (
			mldbTab, 
			" %-60.60s  |  %-60.60s  %120s-%02d", 
			mldb_store [mldbCtr].text [0],
			mldb_store [mldbCtr].edit_text,
			mldb_store [mldbCtr].lu_prmpt,
			-1
		);

		mldbCtr ++;
	}

	print_at (2, 100, "Total No of Records : %d", mldbTotRec);

#ifdef GVISION
	heading (1);
	scn_display (1);
#endif
	if (!tab_display (mldbTab, TRUE))
	{
		tab_scan (mldbTab);
	}

	tab_close (mldbTab, TRUE);
}

/*===================================================+
| Function allows user to change narrative from mldb |
+===================================================*/

static int
EditFunc (
	int			c,
	KEY_TAB*	psUnused)
{
	char tempLine [256];
	int  tPosition;
	char narrative [131];
	int  validNarrative = FALSE;
	
    box (0, 19, 132, 3);
	rv_pr ("Edit English Text",57,19,0);

	tab_get (mldbTab, tempLine, EQUAL, tab_tline (mldbTab));
	tPosition = tab_tline (mldbTab);
	strcpy (narrative, mldb_store [tPosition].edit_text);
	print_at (20, 1, "%R %-129.129s", ML (mldb_store [tPosition].edit_text));

	while (!validNarrative)
	{
		crsr_on ();
		getalpha (SCNPOSOFF + 1, 22, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", narrative); // 128 chars
		crsr_off ();
		
		if (last_char == FN4)
		{
			redraw_table ("LL_lst");
			last_char = ' ';
		}
		if (last_char == FN2)
			strcpy (narrative, mldb_store [tPosition].edit_text);
		if (last_char == FN1)
			break;
		if (last_char == FN16)
			continue;
		validNarrative = TRUE;
	}
	
	if (last_char != FN2)
	{
	    sprintf (err_str, "%-66.66s%-60.60s%-s", 
				 tempLine, narrative, tempLine + 126);
		tab_update (mldbTab, "%s", err_str);
		strcpy (mldb_store [tPosition].edit_text, narrative);
		mldb_store [tPosition].update_ind = CHANGE;
	}
	erase_btm_scn ();
	return (c);
}

/*=================================================================+
| Function that reads information from tab screen and deletes mldb |
+=================================================================*/

static int
DeleteFunc (
	int			c,
	KEY_TAB*	psUnused)
{
	int tPosition;
	int iKey;
	int	ctr;

	tPosition = tab_tline (mldbTab);
	sprintf (err_str, "Are you sure (y/n) ?");
	crsr_on ();
	iKey = prmptmsg (err_str, "YyNn", 55, 21); 
	crsr_off ();

	if (iKey == 'Y' || iKey == 'y')
	{
		deleteInd = TRUE;

		strcpy (mldb_rec.lu_prmpt, mldb_store [tPosition].lu_prmpt);
		cc = find_rec (mldb, &mldb_rec, EQUAL, "u");
		if (!cc)
		{
			cc = abc_delete (mldb);
			if (cc)
			{
				abc_unlock (mldb);
				file_err (cc, mldb, "DBDELETE");
			}
		}

		for (ctr = tPosition; ctr < mldbTotRec; ctr++)
		{
			strcpy (mldb_store [ctr].lu_prmpt, mldb_store [ctr + 1].lu_prmpt);
			strcpy (mldb_store [ctr].text [0], mldb_store [ctr + 1].text [0]);
			strcpy (mldb_store [ctr].edit_text, mldb_store [ctr + 1].edit_text);
			mldb_store [ctr].update_ind = mldb_store [ctr + 1].update_ind;
		}

		mldbTotRec --;
		return (FN16);
	}
	else
	{
		erase_btm_scn ();
		return (c);
	}
}

/*=================================================================+
| Function that reads information from tab screen and updates mldb |
+=================================================================*/

static int
UpdateFunc (
	int			c,
	KEY_TAB*	psUnused)
{
	int ctr;

	for (ctr = 0; ctr < mldbTotRec; ctr++)
	{
		if (mldb_store [ctr].update_ind == CHANGE)
		{
			strcpy (mldb_rec.lu_prmpt, mldb_store [ctr].lu_prmpt);
			cc = find_rec (mldb, &mldb_rec, EQUAL, "u");
			if (cc)
				file_err (cc, mldb, "DBFIND");
			else
			{
				switch (local_rec.lang_no)
				{
				case	1 :
						strcpy (mldb_rec.text [0], mldb_store [ctr].edit_text);
						break;
				case	2 :
						strcpy (mldb_rec.text [1], mldb_store [ctr].edit_text);
						break;
				case	3 :
						strcpy (mldb_rec.text [2], mldb_store [ctr].edit_text);
						break;
				case	4 :
						strcpy (mldb_rec.text [3], mldb_store [ctr].edit_text);
						break;
				case	5 :
						strcpy (mldb_rec.text [4], mldb_store [ctr].edit_text);
						break;
				}
				cc = abc_update (mldb, &mldb_rec);
				if (cc)
				{
					abc_unlock (mldb);
					file_err (cc, mldb, "DBUPDATE");
				}
			}
			
		}
	}
	return (c);
}

/*======================+
| Function aborts input |
+======================*/

static int
QuitFunc (
	int			c,
	KEY_TAB*	psUnused)
{
	return (FN16);
}

/*====================+
| Erase Bottom Screen |
+====================*/

void
erase_btm_scn (void)
{
	int ctr;

	for (ctr = 19; ctr < 23; ctr++)
	{
		move (0, ctr);
		cl_line ();
	}
}

#ifdef GVISION
#else

/*================================================+
| Function to get the files from $PROG_PATH/../.. |
+================================================*/

int
OpenFileList (void)
{
	char	//*sptr = getenv ("PROG_PATH"),
					directory [255],
					filename [100];
	DIR		*dp;
	struct	dirent	*dirp;

	sprintf (directory, "%s/kaloi/files", "/home/kaarlo"); 

	//print_at (15, 15, "%s", sptr);
	//getchar ();

	memset (FileList, 0, sizeof (FileList));

	/*--------------------
	| Open the directory |
	--------------------*/

	if ((dp = opendir (directory)) == NULL)
		return (EXIT_FAILURE);

	/*--------------------
	| Store file entries |
	--------------------*/

	while ((dirp = readdir (dp)) != NULL)
	{
		strcpy (filename, dirp->d_name);
		if ((strcmp (filename, ".") == 0) || 
			 (strcmp (filename, "..") == 0))
			continue;
		FileCount ++;
		sprintf (FileList [FileCount], "%-100.100s", filename);
	}

	closedir (dp);
	return (EXIT_SUCCESS);
}

/*=================================+
| Search files in $PROG_PATH/../.. |
+=================================*/

void
SrchFiles (
	char	*keyValue)
{
	int	ctr = 0;
	char filename [40];

	work_open ();
    save_rec ("#File Name                     ", "# ");

	for (ctr = 1; ctr <= FileCount; ctr++)
	{
		sprintf (filename, "%-35.35s", FileList [ctr]);
		cc = save_rec (filename, " ");
		if (cc)
			break;
	}

	cc = disp_srch ();
	work_close ();

	if (cc)
		return;
}

#endif

/*=================+
| Download Program |
+=================*/

void
DownloadProg (void)
{
	int ctr = 0;
	int iKey;
	FILE *dlFile;
	int xcoor;

	sprintf (err_str, "Download file to %s (y/n) ?", DirFileName);
	xcoor = (132 - strlen (err_str)) / 2;
	crsr_on ();
	iKey = prmptmsg (err_str, "YyNn", xcoor, 21);
	crsr_off ();

	if (iKey == 'Y' || iKey == 'y')
	{
		sprintf (mldb_rec.lu_prmpt, "%-120.120s", " ");

		if ((dlFile = fopen (DirFileName, "w")) == 0)
		{
			sprintf (err_str, "Error in %s during (FOPEN)", DirFileName);
			sys_err (err_str, errno, PNAME);
		}

		cc = find_rec (mldb, &mldb_rec, GTEQ, "r");
		if (cc)
		{
			abc_unlock (mldb);
			print_mess (ML (mlGlMess024));
			sleep (sleepTime);
			return;
		}

		//while (!cc && ctr < 25)
		while (!cc)
		{
			ctr ++;

			fprintf (dlFile, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%d%c%d\n", 
				 	 clip (mldb_rec.lu_prmpt), 1, 
				 	 clip (mldb_rec.text [0]), 1, 
				 	 clip (mldb_rec.text [1]), 1,
				 	 clip (mldb_rec.text [2]), 1,
				 	 clip (mldb_rec.text [3]), 1,
				 	 clip (mldb_rec.text [4]), 1,
				 	 clip (mldb_rec.pname), 1,
				 	 mldb_rec.org_len, 1,
				 	 mldb_rec.hide);
			cc = find_rec (mldb, &mldb_rec, NEXT, "r");
		}

		fclose (dlFile);
	}
}

/*===============+
| Upload Program |
+===============*/

void
UploadProg (void)
{
	int	 lineLength = 800;
	char line [800];
	int  lineCtr = 0;
	int	 ctr;
	int  fieldCtr;
	int  tmpLineCtr;
	FILE *ulFile;
	int iKey;
	char delimeter = '\001';
	int	xcoor;

	sprintf (err_str, "Upload file from %s (y/n) ?", DirFileName);
	xcoor = (132 - strlen (err_str)) / 2;
	crsr_on ();
	iKey = prmptmsg (err_str, "YyNn", xcoor, 21);
	crsr_off ();

	if (iKey == 'Y' || iKey == 'y')
	{
		if ((ulFile = fopen (DirFileName, "r")) == 0)
		{
			fprintf (stdout, "Error in %s during (FOPEN)\n\r", DirFileName);
			fprintf (stdout, "%s %d\n\r", PNAME, errno);
			fflush (stdout);
			return;
		}
		memset (tmpLine, '\0', sizeof (tmpLine));

		while (fgets (line, lineLength, ulFile) != NULL &&
			   line [0] != '\n')
		{	
			lineCtr = strlen (line);
			fieldCtr = 0;
			tmpLineCtr = 0;

			for (ctr = 0; ctr < lineCtr - 1; ctr ++)
			{
				if (line [ctr] == delimeter)
				{
					switch (fieldCtr)
					{
					case	0: 
							sprintf (mldb_store [0].lu_prmpt, "%-120.120s", 
									 tmpLine);
							break;
					case	1:
							sprintf (mldb_store [0].text [0], "%-130.130s", 
									 tmpLine);
							break;
					case	2:
							sprintf (mldb_store [0].text [1], "%-130.130s", 
									 tmpLine);
							break;
					case	3:
							sprintf (mldb_store [0].text [2], "%-130.130s", 
									 tmpLine);
							break;
					case	4:
							sprintf (mldb_store [0].text [3], "%-130.130s", 
									 tmpLine);
							break;
					case	5:
							sprintf (mldb_store [0].text [4], "%-130.130s", 
									 tmpLine);
							break;
					case	6:
							sprintf (mldb_store [0].pname, "%-20.20s", 
									 tmpLine);
							break;
					case	7:
							mldb_store [0].org_len = atoi (tmpLine);
							break;
					case	8:
							mldb_store [0].hide = atoi (tmpLine);
							break;
					}
					fieldCtr ++;
					tmpLineCtr = 0;
					memset (tmpLine, '\0', sizeof (tmpLine));				
				}
				else
				{
					tmpLine [tmpLineCtr] = line [ctr];
					tmpLineCtr ++;
				}
			}

			sprintf (mldb_rec.lu_prmpt, "%-120.120s",
					 mldb_store [0].lu_prmpt);
			sprintf (mldb_rec.text [0], "%-130.130s",
					 mldb_store [0].text [0]);
			sprintf (mldb_rec.text [1], "%-130.130s",
					 mldb_store [0].text [1]);
			sprintf (mldb_rec.text [2], "%-130.130s",
					 mldb_store [0].text [2]);
			sprintf (mldb_rec.text [3], "%-130.130s",
					 mldb_store [0].text [3]);
			sprintf (mldb_rec.text [4], "%-130.130s",
					 mldb_store [0].text [4]);
			sprintf (mldb_rec.pname, "%-20.20s",
					 mldb_store [0].pname);
			mldb_rec.org_len = mldb_store [0].org_len;
			mldb_rec.hide = mldb_store [0].hide;

			cc = find_rec (mldb, &mldb_rec, EQUAL, "w");
			if (cc)
			{
				cc = abc_add (mldb, &mldb_rec);
				if (cc)
				{
					abc_unlock (mldb);
					file_err (cc, mldb, "DBADD");
				}
			}
			else
			{
				cc = abc_update (mldb, &mldb_rec);
				if (cc)
				{
					abc_unlock (mldb);
					file_err (cc, mldb, "DBUPDATE");
				}
			}
		}

		fclose (ulFile);
	}
}

/*==================+
| Validate FileName |
+==================*/

void
ValidateFileName (void)
{
	int ctr;
	
#ifdef GVISION
	if (iProgKey == 2)
		sprintf (DirFileName, "%s%s", "C:\\LS10.5-GUI\\Download\\", 
				 clip (local_rec.file_name));
	else
		sprintf (DirFileName, "%s%s", "C:\\LS10.5-GUI\\Upload\\", 
				 clip (local_rec.file_name));
#else
	sprintf (DirFileName, "%s%s", "/home/kaarlo/kaloi/files/", 
			 clip (local_rec.file_name));
#endif

	for (ctr = 0; ctr < strlen (clip (local_rec.file_name)); ctr ++)
	{
		if (local_rec.file_name [ctr] == '/')
		{
			if (local_rec.file_name [0] == '/')
				sprintf (DirFileName, "%s", clip (local_rec.file_name));
			else
				sprintf (DirFileName, "%s%s", "/", clip (local_rec.file_name));
			break;
		}
	}
}

/*================+
| PercentToThilde |
+================*/

char *
PercentToThilde (
	char * in_mesg)
{
	int 	j,
			len;

	len = strlen (in_mesg);
	for (j = 0; j < len; j++)
	{
		if (in_mesg [j] == '%')
				in_mesg [j] = '~';
	}
	return (in_mesg);
}

/*================+
| ThildeToPercent |
+================*/

char *
ThildeToPercent (
	char * in_mesg)
{
	int 	j,
			len;

	len = strlen (in_mesg);
	for (j = 0; j < len; j++)
	{
		if (in_mesg [j] == '~')
			in_mesg [j] = '%';
	}
	return (in_mesg);
}

MENUTAB prog_menu [] =
{
	{ " 1. MULTI-LINGUAL CONVERSION ENTRY SCREEN.    ", 
	  "" }, 
	{ " 2. DOWNLOAD PROGRAM.                         ", 
	  "" }, 
	{ " 3. UPLOAD PROGRAM.                           ", 
	  "" }, 
	{ " 4. QUIT.                                     ", 
	  "" }, 
	{ ENDMENU }
};

/*============+
| ProgramMenu |
+============*/

void
ProgramMenu (void)
{
	for (;;)
	{
	    mmenu_print ("                   M E N U .                  ", prog_menu, 0);
	    switch (mmenu_select (prog_menu))
	    {
			case 	MLDB_ENTRY :
					iProgKey = 1;
					return;
			case 	MLDB_DOWNLOAD :
					iProgKey = 2;
					return;
			case 	MLDB_UPLOAD :
					iProgKey = 3;
					return;
			case 	MLDB_QUIT :
					iProgKey = 4;
					return;
	    }
	}
}
