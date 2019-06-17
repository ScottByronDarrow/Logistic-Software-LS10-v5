/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: bud_export.c,v 5.5 2002/09/06 03:14:49 scott Exp $
|  Program Name  : (gl_bud_export.c) 
|  Program Desc  : (Export General Ledger Budgets)
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: bud_export.c,v $
| Revision 5.5  2002/09/06 03:14:49  scott
| S/C SC4296/LS01238 - Import of budgets produced an error.
|
| Revision 5.4  2002/03/08 00:56:29  scott
| Updated to remove sort routines.
|
| Revision 5.3  2001/08/09 09:13:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:09  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:36  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bud_export.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_bud_export/bud_export.c,v 5.5 2002/09/06 03:14:49 scott Exp $";

/*
 *   Include file dependencies  
 */
#define TABLINES    12
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <hot_keys.h>
#include <dirent.h>
#include <GlUtils.h>
#include <arralloc.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>

/*
 *   Constants, defines and stuff  
 */
#define	S_DUMM		0
#define	S_HEAD		1
#define	S_TAB 		2

#define	SLEEP_TIME	2
#define	LINES       20
#define	BUDNO    	12
#define	HEADLINES   5

#define	LEVEL0       !strcmp (glmrRec.acc_no, "0000000000000000")
#define	OVERWRITE    fileOp == 'o' || fileOp == 'O'
#define	NUMLINES	 (local_rec.e_per - local_rec.s_per) + 1

char    *data = "data";

enum progtype 
{ 
    SEL_EXPORT, 
    SEL_IMPORT 
};

typedef enum progtype progtype;
typedef int  Bool;

/*
 *   Local variables  
 */
progtype    progType;
	FILE	*fptr;
	char	filePath [256];
	char	ProgDir [256];
	char	str [512];
	char	branchNo [3];
	int		fileOp;
	char	accNo [MAXLEVEL + 1];
	char	*logName;
	double	budget [12];
	char 	mon_name [12][4] = {
						 "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [61];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

extern int GV_cur_level, GV_max_level;

#include	"schema"

struct commRecord	comm_rec;

    struct	
    {
		char	dummy	 [11];
		char	filename [41];
		char	dir		 [41];
		int 	type;
		int 	level;
		int 	budget;
		int 	s_per;
		int 	e_per;
		int 	year;
		char 	s_acc [FORM_LEN + 1];
		char 	e_acc [FORM_LEN + 1];
		char 	s_glmr [MAXLEVEL + 1];
		char 	e_glmr [MAXLEVEL + 1];
		char	desc     [21];
		char	systemDate [11];
		long	lsystemDate;
		long	sys_time;
		int		lpno;
		int		tabyer;
		char	tabmon [4];
		int		tabper;
		int		tabbud;
		char	tabexp [2];
	} 	local_rec;


static struct gllnStruct
{
	long	parentHash;
	long	childHash;
    double  budget [12];
} *gllnArr;

static DArray   gllnArray;
static int      gllnCnt = 0;

static	struct	var vars [] =
{
	{S_HEAD, LIN, "dir", 2, 17, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		"", " ", "Directory     : ", 
		" Enter Output Directory ",
		YES, NO,  JUSTLEFT, "", "", local_rec.dir},
	{S_HEAD, LIN, "filename", 3, 17, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		"", " ", "Filename      : ", " Enter Filename - Search Available ",
		YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{S_HEAD, LIN, "type", 4, 17, INTTYPE,
		"N", "             ",
		"", "1", "Type          : ", " 1 - Excel ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.type},
	{S_HEAD, LIN, "budget_no", 5, 17, INTTYPE,
		"NN", "             ",
		"", "1", "Budget Number : ", " Enter Budget Number ",
		YES, NO,  JUSTLEFT, "0", "99", (char *) &local_rec.budget},
	{S_HEAD, LIN, "year", 6, 17, INTTYPE,
		"NNNN", "             ",
		"", " ", "Year          : ", " Enter Year ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.year},
	{S_HEAD, LIN, "s_per", 7, 17, INTTYPE,
		"NN", "             ",
		"", "1", "Start Period  : ", " Enter Start Period 1 - 12 ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.s_per},
	{S_HEAD, LIN, "e_per", 8, 17, INTTYPE,
		"NN", "             ",
		"", "12", "End Period    : ", " Enter End Period 1 - 12 ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.e_per},
	{S_HEAD, LIN, "level", 9, 17, INTTYPE,
		"NN", "             ",
		"", " ", "Level         : ", " Enter Level",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.level},
	{S_HEAD, LIN, "s_acc", 10, 17, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "             ",
		"", "0000000000000000", "Start Account : ", 
		" Enter Start Account - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_acc},
	{S_HEAD, LIN, "e_acc", 11, 17, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "             ",
		"", "9999999999999999", "End Account   : ", 
		" Enter Start Account - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_acc},
	{S_TAB, TAB, "tabyer", 12, 1, INTTYPE,
		"NNNN", "             ",
		"", "", " Year ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.tabyer},
	{S_TAB, TAB, "tabmon", 0, 2, CHARTYPE,
		"AAA", "             ",
		"", "", " Month ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.tabmon},
	{S_TAB, TAB, "tabper", 0, 2, INTTYPE,
		"NN", "             ",
		"", "", " Period ", " ",
		NA, NO,  JUSTRIGHT, "", "", (char *) &local_rec.tabper},
	{S_TAB, TAB, "tabbud", 0, 2, INTTYPE,
		"NN", "             ",
		"", "", " Budget ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.tabbud},
	{S_TAB, TAB, "tabexp", 0, 3, CHARTYPE,
		"U", "             ",
		"", "", " Export ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.tabexp},
	{S_DUMM, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

/*
 *   Local function prototypes  
 */
Bool 	ReadLines 		 (void);
int  	FindGllnArr 	 (long, int);
int  	heading 		 (int);
int  	spec_valid 		 (int);
long 	CurrentTime 	 (void);
void 	AddBudget 		 (long);
void 	AddGlln 		 (void);
void 	clearScn 		 (void);
void 	CloseDB 		 (void);
void 	Consolidate 	 (void);
void 	LoadData 		 (void);
void 	OpenDB 			 (void);
void 	ProcessGlln 	 (void);
void 	ProcExport 		 (void);
void 	ProcImport 		 (void);
void 	SetupHead 		 (void);
void 	SetupLine 		 (void);
void 	SetupVars 		 (void);
void 	SrchFile 		 (char *);
void 	StoreGlln 		 (long, char *, double *);
void 	UpdateBudget 	 (long, double *);
void 	UpdateLine 		 (void);

/*
 *   Main Processing Function  
 */
int
main (
 int   argc,
 char *argv [])
{
	char	*sptr;
	char	TmpTime [6];

	sptr = strrchr (argv [0], '/');
	if (sptr)
    {
		sptr++;
    }
	else
    {
		sptr = argv [0];
    }

	if (!strncmp (sptr, "gl_bud_export", 12))
    {
		progType = SEL_EXPORT;
    }
	else if (!strncmp (sptr, "gl_bud_import", 12))
    {
		progType = SEL_IMPORT;
    }

	if (argc > 2)
	{
		printf ("\nUsage : %s [dir]\n", argv [0]);
		return (EXIT_FAILURE);
	}
	/*
	 * setup $prog_path/edi/$filename
	 */
	if (argc == 2)
	{
		sprintf (filePath, "%s", argv [1]);
	}
	else
	{
		sprintf (filePath, "%s/SPREAD/", getenv ("PROG_PATH"));
	}
	logName = getenv ("LOGNAME");

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();
	strcpy (TmpTime, TimeHHMM ());
	local_rec.sys_time = atot (TmpTime);

	OpenDB ();

	SETUP_SCR (vars);

	vars [label ("s_acc")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
	vars [label ("e_acc")].mask = vars [label ("s_acc")].mask;

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (S_HEAD);

	tab_row = 4;
	tab_col = 18;

	snorm ();

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;

		init_vars (S_HEAD);

		SetupVars ();

		heading (S_HEAD);
		entry (S_HEAD);

		if (restart || prog_exit)
        {
			continue;
        }

		edit (S_HEAD);

		if (restart)
        {
			continue;
        }

		if (progType == SEL_EXPORT)
		{
			LoadData ();
			heading (S_TAB);
			edit (S_TAB);

			if (restart)
            {
				continue;
            }

			edit_all ();

			if (restart)
            {
				continue;
            }

			ProcExport ();
		}
		else
		{
			ProcImport ();
			ProcessGlln ();
		}

		prog_exit = TRUE;
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	OpenGlmr ();
	OpenGlpd ();
	OpenGlln ();
	ArrAlloc (&gllnArray, &gllnArr, sizeof (struct gllnStruct), 2);
}

void
CloseDB (
 void)
{
	GL_Close ();
	if (gllnCnt > 0)
        ArrDelete (&gllnArray);

	abc_dbclose (data);
}

int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	snorm ();

	sprintf (err_str, 
             ML (" General Ledger Budgets - %s "),
             (progType == SEL_IMPORT) ?	ML ("Import") : ML ("Export"));

	rv_pr (err_str, 24, 0, 1);

	line_at (1,0,79);
	line_at (21,0,79);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	if (scn == S_HEAD)
		box (0, 1, 79, 10);		

	if (scn == S_TAB)
		box (tab_col, tab_row - 1, 43, 14);		

	scn_set     (scn);
	scn_write   (scn);
	scn_display (scn);

	line_cnt = 0;

    return (EXIT_SUCCESS);
}

void
clearScn (
 void)
{
	clear ();
	snorm ();

	sprintf (err_str, 
             ML (" General Ledger Budgets - %s "),
             (progType == SEL_IMPORT) ?	ML ("Import") : ML ("Export"));
	rv_pr (err_str, 24, 0, 1);

	line_at (1,0,79);
	line_at (21,0,79);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
}

int
spec_valid (
 int	field)
{
	DIR 	*dptr;
	struct 	dirent *dirEnt;
	Bool	fileFound;

	if (LCHECK ("dir"))
	{
		if (dflt_used)
        {
			sprintf (local_rec.dir, "%s", filePath);
        }

		if (strcmp (local_rec.dir + (strlen (local_rec.dir) - 1), "/"))
        {
			strcat (local_rec.dir, "/");
        }

		if (strlen (clip (local_rec.dir)) == 0)
		{
			strcpy (err_str, ML (mlStdMess132));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (ProgDir, "%s", local_rec.dir);

        (dptr = opendir (ProgDir));
		if (dptr == NULL)
		{
			sprintf (err_str, ML ("Error Opening Directory %s"), ProgDir);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (progType == SEL_EXPORT)
		{
            if (access (ProgDir, W_OK) < 0) 
			{
				sprintf (err_str, ML ("The User Does Not Have Write Access To %s"), ProgDir);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		DSP_FLD ("dir");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("filename"))
	{
		if (SRCH_KEY)
		{
			SrchFile (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (local_rec.filename)) == 0)
		{
			sprintf (err_str, ML ("A Valid Filename Must Be Entered"));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (ProgDir, "%s", local_rec.dir);
		if ((dptr = opendir (ProgDir)) == NULL)
		{
			print_mess (ML ("Error Opening Directory"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (progType == SEL_EXPORT)
		{
			fileFound = FALSE;

			for (dirEnt = readdir (dptr);
                 dirEnt != NULL;
                 dirEnt = readdir (dptr))
			{	
				if (!strcmp (dirEnt->d_name, local_rec.filename))
				{
					fileFound = TRUE;
					break;
				}
			}

			closedir (dptr);

			if (fileFound == TRUE)
			{
				print_mess (ML ("Warning - File Already Exists"));
				fileOp = prmptmsg (ML ("O(verwrite) or A(ppend)"),"aAoO",1,14);
				print_at (14, 1, "%-40.40s", " ");
				clear_mess ();
			}

			DSP_FLD ("filename");
			return (EXIT_SUCCESS);
		}

		fileFound = FALSE;

		for (dirEnt = readdir (dptr);
             dirEnt != NULL;
             dirEnt = readdir (dptr))
		{	
			if (!strcmp (dirEnt->d_name, local_rec.filename))
			{
				fileFound = TRUE;
				break;
			}
		}

		closedir (dptr);

		if (fileFound == FALSE)
		{
			print_mess (ML ("File Does Not Exists"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("filename");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("budget_no"))
	{
		if (local_rec.budget == 0 && progType == SEL_IMPORT)
		{
			print_mess (ML ("Actual Data Cannot Be Processed"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("budget_no");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("year"))
	{
		DSP_FLD ("year");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_acc"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			return (EXIT_SUCCESS);
		}

		cc = GL_FormAccNo (local_rec.s_acc, glmrRec.acc_no, 0);
    	if (cc)
        {
			return (EXIT_FAILURE);
        }

		strcpy (local_rec.s_glmr, glmrRec.acc_no);

		DSP_FLD ("s_acc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_acc"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			return (EXIT_SUCCESS);
		}

		cc = GL_FormAccNo (local_rec.e_acc, glmrRec.acc_no, 0);
    	if (cc)
        {
			return (EXIT_FAILURE);
        }

		strcpy (local_rec.e_glmr, glmrRec.acc_no);

		DSP_FLD ("e_acc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("level"))
	{
		if (dflt_used)
        {
			local_rec.level = GV_max_level;
        }

	
		if (local_rec.level > GV_max_level)
		{
			sprintf (err_str, ML ("Level number must be from 1 to %d"),GV_max_level);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("level");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchFile (
 char *key_val)
{
	DIR     *dptr;
	int		i;
	struct dirent *dirEnt;

	_work_open (40,0,3);
	save_rec ("#File", "#N/A");

	sprintf (ProgDir, "%s", local_rec.dir);
	if ((dptr = opendir (ProgDir)) == NULL)
	{
		work_close ();
		return;
	}

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	for (dirEnt = readdir (dptr);
         dirEnt != NULL;
         dirEnt = readdir (dptr))
	{	
		if (!strncmp (dirEnt->d_name, key_val, strlen (key_val)) &&
			strncmp (dirEnt->d_name, ".", 1)) 
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
				sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element sortCnt.
			 */
			sprintf (sortRec [sortCnt].sortCode, "%-40.40s", dirEnt->d_name);
			/*
			 * Increment array counter.
			 */
			sortCnt++;
		}
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		cc = save_rec (sortRec [i].sortCode, " ");
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	cc = disp_srch ();
	work_close ();

	closedir (dptr);
	return;
}

/*
 * Process Export 
 */
void
ProcExport (
 void)
{
	memset (str, 0, sizeof str);

	/*
     * Write EDI file
     */
    sprintf (ProgDir, "%s%s", local_rec.dir, local_rec.filename);

	if (OVERWRITE)
	{
   		if ((fptr = fopen (ProgDir, "w")) == NULL)
    	{
        	sprintf (err_str, ML ("Can't open %s\n"), ProgDir);
        	print_mess (err_str);
        	sleep (sleepTime);
        	clear_mess ();
        	return;
		}
    }
	else
	{
    	if ((fptr = fopen (ProgDir, "a")) == NULL)
    	{
        	sprintf (err_str, ML ("Can't open %s\n"), ProgDir);
        	print_mess (err_str);
        	sleep (sleepTime);
        	clear_mess ();
        	return;
    	}
	}	

	clearScn ();

	if (OVERWRITE)
	{
		SetupHead ();
		SetupLine ();
	}
	else
    {
		SetupLine ();
    }

	fclose (fptr);
}

/*
 * Process Import
 */
void
ProcImport (
 void)
{
	int		i;

	sprintf (ProgDir, "%s%s", local_rec.dir, local_rec.filename);

	if ((fptr = fopen (ProgDir, "r")) == NULL)
	{
		sprintf (err_str, ML ("Can't open %s"), ProgDir);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return;
	}

	/*
	 * check if file valid
	 */
	if (fgets (str, sizeof str, fptr) == NULL)
	{
		sprintf (err_str, ML ("There Are No Valid Lines Within %s"), ProgDir);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return;
	}

	clearScn ();

	/*
	 * reset file pointer
	 */
	fseek (fptr, 0, 0);

	/*
	 * skip header
	 */
	for (i = 0; i < HEADLINES; i++)
	{
		while (getc (fptr) != '\n')
		{
			if (feof (fptr))
			{	
				fclose (fptr);
				return;
			}
		}
	}

	while (!ReadLines ())
    {
        UpdateLine ();
    }

	Consolidate ();

	fclose (fptr);
}

void
SetupVars (
 void)
{
	fileOp = 'O';
	FLD ("s_per") = YES;	
	FLD ("e_per") = YES;	
	FLD ("year") = YES;	
	FLD ("budget_no") = YES;
}

void
LoadData (
 void)
{
	int	i, t;

	init_vars (S_TAB);
	scn_set   (S_TAB);
	lcount [S_TAB] = 0;
	line_cnt = 0;

	for (t = local_rec.s_per - 1; t < local_rec.e_per; t++)
	{
		i = (comm_rec.fiscal + t) % 12;
		local_rec.tabyer = local_rec.year;
		sprintf (local_rec.tabmon, "%-3.3s", mon_name [i]);

		local_rec.tabbud = local_rec.budget;
		local_rec.tabper = t + 1;
		strcpy (local_rec.tabexp, "Y");

		putval (line_cnt++);
		lcount [S_TAB]++;
	}

	vars [scn_start].row = t;
	line_cnt = 0;
	getval (line_cnt);

	FLD ("s_per") = NA;	
	FLD ("e_per") = NA;	
	FLD ("year") = NA;	
	FLD ("budget_no") = NA;	
}

void
SetupHead (
 void)
{
	char	tmpStr [5][256];

	memset (tmpStr, 0, sizeof tmpStr);
	scn_set (S_TAB);

	for (line_cnt = 0; line_cnt < NUMLINES; line_cnt++)
	{
		getval (line_cnt);
		sprintf (err_str, "| %4d ", local_rec.tabyer);	
		strcat (tmpStr [0], err_str);	
		sprintf (err_str, "| %-3.3s ", local_rec.tabmon);	
		strcat (tmpStr [1], err_str);	
		sprintf (err_str, "| %d ", local_rec.tabper);	
		strcat (tmpStr [2], err_str);	
		sprintf (err_str, "| %d ", local_rec.tabbud);	
		strcat (tmpStr [3], err_str);	
		sprintf (err_str, "| %s ", local_rec.tabexp);	
		strcat (tmpStr [4], err_str);	
	}

	sprintf (str, 
			 "Fin Year%s\nMonth%s\nPeriod No%s\nBudget No%s\nExport%s\n", 
			 tmpStr [0], 
			 tmpStr [1], 
			 tmpStr [2], 
			 tmpStr [3], 
			 tmpStr [4]);

	fputs (str, fptr);
}

void
SetupLine (
 void)
{
	char	tmpStr [256],
			tmpAcc [FORM_LEN + 1];

	memset (tmpStr, 0, sizeof tmpStr);
	scn_set (S_TAB);

	abc_selfield (glmr, "glmr_id_no");
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, local_rec.s_glmr);

    for (cc = find_rec (glmr, &glmrRec, GTEQ, "r");
		!cc &&	
        !strcmp (glmrRec.co_no, comm_rec.co_no);
		cc = find_rec (glmr, &glmrRec, NEXT, "r"))
	{
		if (strcmp (glmrRec.acc_no, local_rec.e_glmr) > 0)
        {
			continue;
        }

		if (!strcmp (glmrRec.glmr_class [0], "C") &&
			!strcmp (glmrRec.glmr_class [1], "C"))
        {
			continue;
        }

		strcpy (tmpAcc, glmrRec.acc_no);
	 	GL_FormAccNo (tmpAcc, glmrRec.acc_no, 0);

		if (GV_cur_level > local_rec.level)
        {
			continue;
        }

		sprintf (tmpStr, "%-*.*s ", MAXLEVEL,MAXLEVEL,glmrRec.acc_no);

		dsp_process (" Account : ", glmrRec.acc_no);

		for (line_cnt = 0; line_cnt < NUMLINES; line_cnt++)
		{
			getval (line_cnt);
			glpdRec.hhmr_hash = glmrRec.hhmr_hash;
			glpdRec.budg_no = local_rec.tabbud;
		 	glpdRec.year = local_rec.tabyer; 
			glpdRec.prd_no = local_rec.tabper;

			cc = find_rec (glpd, &glpdRec, EQUAL, "r");
			if (cc)
            {
				glpdRec.balance = 0.00;
            }

			if (!strcmp (local_rec.tabexp, "Y"))
            {
				sprintf (err_str, "|%10.2f ", DOLLARS (glpdRec.balance));	
            }
			else
            {
				sprintf (err_str, "|  ");	
            }
			strcat (tmpStr, err_str);	
		}

		sprintf (str, "%s\n", tmpStr);
		fputs (str, fptr);
	}
}

Bool
ReadLines (void)
{
	char	ch, 
			chr [2],
			tmpStr [255];
	char	*sptr;
	int		i = 0;
	Bool	firstTime = TRUE;

/*
	sptr = fgets (tmpStr, sizeof (accNo), fptr);
*/
	sptr = fgets (tmpStr, 20, fptr);
	if (sptr == (char *)0)
    {
		return (TRUE);
    }

	sprintf (accNo, "%-*.*s", MAXLEVEL, MAXLEVEL, &tmpStr [0]);
	memset (tmpStr, 0, sizeof tmpStr);
	memset (budget, 0, sizeof budget);

	dsp_process (" Account : ", accNo);

	while ((ch = getc (fptr)) != '\n')
	{
		if (feof (fptr))
			return (TRUE);

		if (ch == ',' || ch == '\r')
		{
			if (firstTime)
			{
				firstTime = FALSE;
				memset (tmpStr, 0, sizeof tmpStr);
				continue;
			}

			if (i < BUDNO)
			{
				if (tmpStr [0] != 0)
					budget [i] = atof (tmpStr);
				else
					budget [i] = 0.00;

				i++;
				memset (tmpStr, 0, sizeof tmpStr);
				continue;
			}	
		}

		if	 ((ch >= '0' && ch <= '9') ||
		     (ch == '-') ||
		     (ch == '.'))
		{
			sprintf (chr, "%c", ch);
			strcat (tmpStr, chr);	
		}
	}

	return (FALSE);
}

void
UpdateLine (void)
{

	abc_selfield (glmr, "glmr_id_no");
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, accNo);

    cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
        file_err (cc, glmr, "DBFIND");

	/*
	 * Update new budget values
	 */
	UpdateBudget (glmrRec.hhmr_hash, budget);

	/*
	 * Check if links to glln records exist 
	 * - store budget figures for later use
	 */
	StoreGlln (glmrRec.hhmr_hash, glmrRec.glmr_class [0], budget);
}

void
UpdateBudget (
 long hhmrHash,
 double budget [12])
{
	int	i;

	for (i = local_rec.s_per - 1; i < local_rec.e_per; i++)
	{
		memset (&glpdRec, 0, sizeof glpdRec);
		glpdRec.hhmr_hash 	= hhmrHash;
		glpdRec.budg_no 	= local_rec.budget;
	 	glpdRec.year 		= local_rec.year; 
		glpdRec.prd_no 		= i + 1;

        cc = find_rec (glpd, &glpdRec, EQUAL, "u");
		glpdRec.mod_time = CurrentTime ();
		if (cc)
		{
			strcpy (glpdRec.user_id, logName);
			glpdRec.balance = CENTS (budget [i]);

            cc = abc_add (glpd, &glpdRec);	
			if (cc)
        		file_err (cc, glpd, "DBADD");
		}
		else
		{
			strcpy (glpdRec.user_id, logName);
			glpdRec.balance = CENTS (budget [i]);

            cc = abc_update (glpd, &glpdRec);	
			if (cc)
        		file_err (cc, glpd, "DBUPDATE");
		}
	}
}

void
Consolidate (void)
{
	int			i;
	char		tmpAcc [FORM_LEN + 1];
	Bool		foundGlmr;
	GLMR_STRUCT glmrRec2;

	for (i = local_rec.level; i > 0; i--)
	{
		memset (budget, 0, sizeof budget);
		foundGlmr = FALSE;
		
		abc_selfield (glmr, "glmr_id_no");
		strcpy (glmrRec.co_no, comm_rec.co_no);
		sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,local_rec.s_glmr);

        for (cc = find_rec (glmr, &glmrRec, GTEQ, "r");
			!cc &&	
            !strcmp (glmrRec.co_no, comm_rec.co_no);
			cc = find_rec (glmr, &glmrRec, NEXT, "r"))
		{
			if (strcmp (glmrRec.acc_no, local_rec.e_glmr) > 0)
            {
				continue;
            }

			/*
			 * Get current GL level
			 */
			strcpy (tmpAcc, glmrRec.acc_no);
	 		GL_FormAccNo (tmpAcc, glmrRec.acc_no, 0);

			if (LEVEL0)
            {
				GV_cur_level = 0;
            }

			if (GV_cur_level == i - 1)
			{
				if (foundGlmr &&
					 (glmrRec2.glmr_class [2][0] != 'P'))
				{
					UpdateBudget (glmrRec2.hhmr_hash, budget);
					StoreGlln 
					(
						glmrRec2.hhmr_hash, 
						glmrRec2.glmr_class [0],
						budget
					);
				}

				foundGlmr = TRUE;
				glmrRec2 = glmrRec;
				memset (budget, 0, sizeof budget);
				continue;
			}

			if (GV_cur_level != i)
            {
				continue;
            }

			AddBudget (glmrRec.hhmr_hash);
		}

		if (foundGlmr)
		{
			UpdateBudget (glmrRec2.hhmr_hash, budget);
			StoreGlln 
			(
				glmrRec2.hhmr_hash, 
				glmrRec2.glmr_class [0],
				budget
			);
		}
	}

}

void
AddBudget (
	long	hhmrHash)
{
	int	i;

	for (i = local_rec.s_per - 1; i < local_rec.e_per; i++)
	{
		memset (&glpdRec, 0, sizeof glpdRec);
		glpdRec.hhmr_hash = hhmrHash;
		glpdRec.budg_no = local_rec.budget;
	 	glpdRec.year = local_rec.year; 
		glpdRec.prd_no = i + 1;

        cc = find_rec (glpd, &glpdRec, EQUAL, "u");
		if (!cc)
        {
			budget [i] += DOLLARS (glpdRec.balance);
        }
	}
}

void
StoreGlln (
	long   hhmrHash,
	char   type [2],
	double budget [12])
{
	int	i, t, idx;

	if (type [0] == 'N')
    {
		return;
    }

	abc_selfield (glln, "glln_id_no2");
	gllnRec.child_hash  = hhmrHash;
	gllnRec.parent_hash = 0L;

    for (cc = find_rec (glln, &gllnRec, GTEQ, "u");
		 !cc &&
         (gllnRec.child_hash == hhmrHash);
         cc = find_rec (glln, &gllnRec, NEXT, "u"))
	{
		/*
		 * Check if glln already stored - add budgets
		 */
		idx = FindGllnArr (gllnRec.parent_hash, 0);
		if (idx >= 0)
		{
			for (t = 0; t < BUDNO; t++)
			{
				gllnArr [idx].budget [t] += budget [t];
			}
		}
		else
		{
			/*
        	 * Increment array
        	 */
        	ArrChkLimit (&gllnArray, gllnArr, gllnCnt);
        	memset (&gllnArr [gllnCnt], 0, sizeof (struct gllnStruct));

			gllnArr [gllnCnt].parentHash = gllnRec.parent_hash;
			gllnArr [gllnCnt].childHash  = gllnRec.child_hash;

			for (i = 0; i < BUDNO; i++)
			{
				gllnArr [gllnCnt].budget [i] = budget [i];
			}

        	gllnCnt++;
		}
	}
}

/*
 * Traverse glln records
 */
void
ProcessGlln (
 void)
{
	int	i;

	if (gllnCnt == 0)
		return;

	/*
	 * Overwrite entry node budgets
	 */
	for (i = 0; i < gllnCnt; i++)
	{
		UpdateBudget (gllnArr [i].parentHash, gllnArr [i].budget);
	}

	/*
	 * Add glln budgets
	 */
	AddGlln ();	
}

void
AddGlln (void)
{
	int		i, t, idx;
	long	hhmrHash;
	double	budgetEntry [12];

	/*
	 * Set start point in existing array
	 */
	int	startCnt = gllnCnt;

	for (i = 0; i < startCnt; i++)
	{
		/*
		 * Get entry node data
		 */
		hhmrHash = gllnArr [i].parentHash;
		for (t = 0; t < BUDNO; t++)
			budgetEntry [t] = gllnArr [i].budget [t];

		abc_selfield (glln, "glln_id_no2");
		gllnRec.child_hash  = hhmrHash;
		gllnRec.parent_hash = 0L;

        for (cc = find_rec (glln, &gllnRec, GTEQ, "u");
             !cc &&
             (gllnRec.child_hash == hhmrHash);
             cc = find_rec (glln, &gllnRec, GTEQ, "u"))
		{
			/*
			 * If flagged then add budget figures
			 */
			idx = FindGllnArr (gllnRec.parent_hash, startCnt);
			if (idx >= 0)
			{
				for (t = 0; t < BUDNO; t++)
				{
					gllnArr [idx].budget [t] += budgetEntry [t];
					budget [t] = gllnArr [idx].budget [t];
				}
			}
			else
			{
				/*
				 * Flag glln, so if we process it 
				 * again we add the budget figures
				 */
				ArrChkLimit (&gllnArray, gllnArr, gllnCnt);
           		memset (&gllnArr [gllnCnt], 0, sizeof (struct gllnStruct));
           		gllnArr [gllnCnt].parentHash = gllnRec.parent_hash;
           		gllnArr [gllnCnt].childHash  = gllnRec.child_hash;

				/*
				 * New so we overwrite the budgets
				 */
				for (t = 0; t < BUDNO; t++)
				{
					gllnArr [gllnCnt].budget [t] = budgetEntry [t];
					budget [t] = gllnArr [gllnCnt].budget [t];
				}
				gllnCnt++;
			}

			/*
			 * Overwrite budget figures
			 */
			UpdateBudget (gllnRec.parent_hash, budget);

			/*
			 * Store hash so we go up the tree
			 */
			hhmrHash = gllnRec.parent_hash;

			gllnRec.child_hash  = hhmrHash;
			gllnRec.parent_hash = 0L;
		}
	}
}

/*
 * Find glln within array 
 * return   -1 not found, return >= 0 found  
 */
int 
FindGllnArr  (
	long	hhmrHash,
	int		startCnt)
{
	int	i;

	for (i = startCnt; i < gllnCnt; i++)
	{
		if 	 (gllnArr [i].parentHash == hhmrHash)
		{
			return (i);
		}
	}

	return (-1);
}

long
CurrentTime (void)
{
	time_t	currTime;

	currTime = -1L;

	currTime = time (&currTime);

	return ((long) currTime);
}

int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
