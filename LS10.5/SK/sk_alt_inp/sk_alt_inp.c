/*=====================================================================
|  Copyright (C) 1996 - 2000 Logistic Software Limited   .            |
|=====================================================================|
| $Id: sk_alt_inp.c,v 5.6 2002/07/24 08:39:11 scott Exp $
|  Program Desc  : (Maintain Synonym Stock Items.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr,     ,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim   | Date Written  : 03/11/88         |
|---------------------------------------------------------------------|
|  Date Modified : (11/09/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (03/03/93)      | Modified  by : Jonathan Chen     |
|  Date Modified : (27/05/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (04/09/97)      | Modified  by : Ana Marie Tario.  |
|                                                                     |
|  Comments      : (11/09/90) - Updated to add sk_alt_prn into input. |
|      (03/0/93) : PSL 8620 dbalias () -> abc_alias ()                |
|  (27/05/94     : PSL 10712.  Fix arguments to find_hash.            |
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                                                                     |
| $Log: sk_alt_inp.c,v $
| Revision 5.6  2002/07/24 08:39:11  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/06/20 07:10:53  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/09/10 03:45:58  cha
| SE-155. Corrected to put delay in error messages;
|
| Revision 5.3  2001/08/09 09:18:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:40  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:50  scott
| Update - LS10.5
|
| Revision 5.0  2001/06/19 08:15:04  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:47  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:23  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/07/10 01:52:48  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.13  2000/06/13 05:02:45  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.12  2000/02/16 01:02:20  cam
| Changes for GVision compatibility.  Fixed calls to print_mess ().
|
| Revision 1.11  1999/12/06 01:30:35  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/11 05:59:31  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.9  1999/11/03 07:31:52  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.8  1999/10/13 02:41:50  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.7  1999/10/08 05:32:13  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:19:46  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_alt_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_alt_inp/sk_alt_inp.c,v 5.6 2002/07/24 08:39:11 scott Exp $";

/*===========================================
| These next two lines for 132 tabular only |
===========================================*/
#define MAXWIDTH 150
#define MAXLINES 200

#define	SLEEP_TIME	2

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	"schema"

struct	commRecord	comm_rec;
struct	inmrRecord	inmr_rec;
struct	inmrRecord	sinmr_rec;

/*============
| File Names |
============*/
static char
	*data	= "data", 
	*inmr2	= "inmr2";

/*=========
| Globals |
==========*/
	int		printerNumber 	= 1;
	int		updateInmr 		= FALSE;
	int		newCode 		= 0;
	int		maxItems	 	= 0;
	int		alternateItem	= TRUE;
	int		alternateError	= FALSE;
	int		synonymError 	= FALSE;

	FILE	*fout;

	struct storeRec {
		long	hhbrHash;
		long	eraseHhbrHash;
	} store [MAXLINES];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	desc [41];
	char	alt_item_no [17];
	char	alt_desc [41];
	long	hhbrHash;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "item_no", 4, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number : ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "desc", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Item Desc   : ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc}, 
	{2, TAB, "alt_item_no", MAXLINES, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "   Item Number    ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.alt_item_no}, 
	{2, TAB, "alt_desc", 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                Description                ", " ", 
		NA, NO, JUSTLEFT, " ", "", local_rec.alt_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*======================
| Function Declaration |
======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int  	spec_valid 			(int);
int  	CheckForDuplicates 	(long, int *);
int  	DeleteRecord		(void);
void 	SetupArray 			(void);
void 	LoadTabularScreen 	(void);
void 	Update 				(void);
void 	IntAlternateFind 	(long);
void 	ProcessPrint 		(void);
void 	ProcessInmr 		(void);
void 	HeadingPrint		(void);
void 	ProcessCheck 		(void);
void 	ProcessSynonym 		(void);
void 	HeadingCheck		(void);
void 	CloseAudit 			(void);
int  	heading 			(int);

#include <FindInmr.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{

	if (!strncmp (argv [0], "sk_alt_prn", 10))
	{
		if (argc != 2)
		{
			print_at (0, 0, mlStdMess036, argv [0]);
			return (EXIT_FAILURE);
		}
		printerNumber = atoi (argv [1]);

		init_scr ();

		OpenDB ();

		dsp_screen (" Print Synonym Item breakdown. ", 
					comm_rec.co_no, comm_rec.co_name);

		ProcessPrint ();
		
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	if (!strncmp (argv [0], "sk_alt_chk", 10))
	{
		if (argc != 3)
		{
			print_at (0, 0, mlSkMess270, argv [0]);
			return (EXIT_FAILURE);
		}

		printerNumber = atoi (argv [1]);
		switch (argv [2] [0])
		{
			case	'U':
			case	'u':
				updateInmr = TRUE;
				break;
		
			case	'N':
			case	'n':
				updateInmr = FALSE;
				break;
		
			default	:
				printf ("U(pdate) or N(o update) only\n");
                return (EXIT_FAILURE);
		}

		OpenDB ();

		dsp_screen (" Print Synonym item Check ", 
					comm_rec.co_no, comm_rec.co_name);
	
		ProcessCheck ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	if (!strncmp (argv [0], "sk_alt_inp", 10))
		alternateItem	= TRUE;
	else
		alternateItem	= FALSE;

	SETUP_SCR (vars);

	init_scr 	();	
	set_tty 	();
	set_masks 	();		
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);

	OpenDB ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		if (newCode == 1)
		{
			SetupArray ();
			heading (2);
			entry (2);
		}

		if (restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	abc_alias (inmr2, inmr);
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhsi_hash");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		i = 0;
	int		pos = 0;
	int		highValue = (prog_status == ENTRY) ? line_cnt : lcount [2];

	char	tempItem [17];

	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
	 		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
			
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		if (inmr_rec.hhsi_hash != 0L)
		{
			abc_selfield (inmr2, "inmr_hhbr_hash");
			IntAlternateFind (inmr_rec.hhsi_hash);
			
			abc_selfield (inmr2, "inmr_hhsi_hash");
			sprintf (err_str, ML (mlSkMess533), 
					local_rec.item_no, inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.desc, inmr_rec.description);
		local_rec.hhbrHash = inmr_rec.hhbr_hash;
		DSP_FLD ("desc");

		/*---------------------------
		| Load Tabular, set newCode |
		---------------------------*/
		LoadTabularScreen ();

		if (alternateItem && alternateError	== TRUE)
		{
			print_mess (ML ("Lines belong to a Synonym item not an Alternate"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (!alternateItem && synonymError	== TRUE)
		{
			print_mess (ML ("Lines belong to an Alternate item not a Synonym"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Item No in Tabular	|
	-------------------------------*/
	if (LCHECK ("alt_item_no"))
	{
		if (SRCH_KEY)
		{
	 		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		/*-----------------------------
		| Delete file if default used |
		-----------------------------*/
		if (dflt_used)
			return (DeleteRecord ());

		cc = FindInmr (comm_rec.co_no, local_rec.alt_item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.alt_item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		
		strcpy (local_rec.alt_item_no, inmr_rec.item_no);
		strcpy (local_rec.alt_desc, inmr_rec.description);
		DSP_FLD ("alt_item_no");
		DSP_FLD ("alt_desc");

		if (inmr_rec.hhbr_hash == local_rec.hhbrHash)
		{
			print_mess (ML (mlSkMess548));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (inmr_rec.on_hand != 0.00)
		{
			print_mess (ML (mlSkMess697));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inmr_rec.hhsi_hash != 0L && 
		     inmr_rec.hhsi_hash != local_rec.hhbrHash)
		{
			abc_selfield (inmr2, "inmr_hhbr_hash");
			IntAlternateFind (inmr_rec.hhsi_hash);
			abc_selfield (inmr2, "inmr_hhsi_hash");
			sprintf (err_str, ML (mlSkMess533), local_rec.alt_item_no, inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inmr_rec.hhsi_hash == 0L)
		{
			/*-----------------------------------------------------------
			| Check if this item itself is a master of other alt_items. |
			-----------------------------------------------------------*/
			strcpy (tempItem, inmr_rec.item_no);
			inmr_rec.hhsi_hash	=	inmr_rec.hhbr_hash;
			cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
			if (!cc)
			{
				print_mess (ML (mlSkMess532));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		for (i = 0;i < highValue;i++)
		{
			if (i == line_cnt)	/* didn't change anything */
				continue;

			if (store [i].hhbrHash == inmr_rec.hhbr_hash && 
			     !CheckForDuplicates (inmr_rec.hhbr_hash, &pos))
			{
				print_mess (ML (" Duplicate Synonym / Alterate Item "));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;
		if (maxItems < line_cnt)
			maxItems = line_cnt;

		if (CheckForDuplicates (store [line_cnt].hhbrHash, &pos))
			store [pos].eraseHhbrHash = 0L;

		strcpy (local_rec.alt_desc, inmr_rec.description);
		DSP_FLD ("alt_item_no");
		DSP_FLD ("alt_desc");
	}
	return (EXIT_SUCCESS);
}

/*============================
| Check For Duplicate items. |
============================*/
int
CheckForDuplicates (
	long	hhbrHash, 
	int 	*position)
{
	int	i = 0;
	int	found = FALSE;

	for (i = 0; i <= maxItems && found == FALSE;i++)
	{
		if (store [i].eraseHhbrHash == hhbrHash)
		{
			*position = i;
			found = TRUE;
		}
	}
	return (found);
}
	
/*---------------------------------
| delete line if default was used  |
----------------------------------*/
int
DeleteRecord (
 void)
{
	int	i = 0;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (!CheckForDuplicates (store [line_cnt].hhbrHash, &i))
	{
		if (CheckForDuplicates (0L, &i))
		{
			/*---------------------------
			| Store the new hash of rec |
			| to be deleted.	    |
			---------------------------*/
			store [i].eraseHhbrHash = store [line_cnt].hhbrHash;
		}
	}

	lcount [2]--;

	this_page = line_cnt/TABLINES;

	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		store [line_cnt].hhbrHash = store [line_cnt + 1].hhbrHash;
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	sprintf (local_rec.alt_item_no, "%-16.16s", " ");
	sprintf (local_rec.alt_desc, "%-40.40s", " ");
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();

	store [line_cnt].hhbrHash = 0L;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
SetupArray (
    void)
{
	int	i = 0;

	for (i = 0; i < MAXLINES; i++)
	{
		store [i].hhbrHash = 0L;
		store [i].eraseHhbrHash = 0L;
	}
}

void
LoadTabularScreen (
 void)
{
	alternateError	=	FALSE;
	synonymError 	=	FALSE;

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	inmr_rec.hhsi_hash	=	local_rec.hhbrHash;
	cc = find_rec (inmr2, &inmr_rec, GTEQ, "r");
	while (!cc && local_rec.hhbrHash == inmr_rec.hhsi_hash)
	{
		if (alternateItem && !strcmp (inmr_rec.alternate, "                "))
			alternateError	=	TRUE;

		if (!alternateItem && strcmp (inmr_rec.alternate, "                "))
			synonymError 	=	TRUE;

		strcpy (local_rec.alt_item_no, inmr_rec.item_no);
		strcpy (local_rec.alt_desc, inmr_rec.description);
		store [lcount [2]].hhbrHash = inmr_rec.hhbr_hash;
		putval (lcount [2]++);

		if (lcount [2] >= MAXLINES)
			break;

		cc = find_rec (inmr2, &inmr_rec, NEXT, "r");
	}
	maxItems = lcount [2];
	newCode = (lcount [2] == 0);
	scn_set (1);
}

void
Update (
 void)
{
	int 	cnt = 0;
	int 	i 	= 0;
	
	/*----------------
	| Maintain inmr  |
	----------------*/	
	scn_set (2);

	abc_selfield (inmr, "inmr_hhbr_hash");

	for (cnt = 0;cnt < lcount [2];cnt++)
	{
		getval (cnt);
		
		if (store [cnt].hhbrHash == 0L)
			continue;

		inmr_rec.hhbr_hash	=	store [cnt].hhbrHash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (err_str, "inmr (%ld)", store [cnt].hhbrHash);
			file_err (cc, err_str, "DBFIND");
		}
		inmr_rec.hhsi_hash = local_rec.hhbrHash;
		strcpy (inmr_rec.alternate, (alternateItem) ? local_rec.item_no : " ");
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, "inmr", "DBUPDATE");

		abc_unlock (inmr);
	}

	/*---------------
	| Delete Record |
	--------------*/
	for (i = 0; i < maxItems; i++)
	{
		if (store [i].eraseHhbrHash == 0L)
			continue;

		inmr_rec.hhbr_hash	=	store [i].eraseHhbrHash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
		inmr_rec.hhsi_hash = 0L;
		strcpy (inmr_rec.alternate, "                ");
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			continue;

		abc_unlock (inmr);
	}
		
	abc_selfield (inmr, "inmr_id_no");
	abc_unlock (inmr);
	
}

void
IntAlternateFind (
	long	hhsiHash)
{
	if (hhsiHash == 0L)
		return;

	inmr_rec.hhbr_hash	=	hhsiHash;
	cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
	if (!cc)
		IntAlternateFind (inmr_rec.hhsi_hash);
}

/*=======================
| Process whole report. |
=======================*/
void
ProcessPrint (
 void)
{
	HeadingPrint ();

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		/*-------------------
		| master customer	|
		-------------------*/
		if (inmr_rec.hhsi_hash == 0L)
			ProcessInmr ();

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	CloseAudit ();
}

void
ProcessInmr (
 void)
{
	int	first_inmr = TRUE;

	sinmr_rec.hhsi_hash = inmr_rec.hhbr_hash;
	cc = find_rec (inmr2, &sinmr_rec, EQUAL, "r");
	while (!cc && sinmr_rec.hhsi_hash == inmr_rec.hhbr_hash)
	{
		if (first_inmr)
		{
			dsp_process (" Master Item: ", inmr_rec.item_no);

			fprintf (fout, ".LRP5\n");
			fprintf (fout, "|------------------");
			fprintf (fout, "|------------------");
			fprintf (fout, "-------------------------------------------|\n");

			fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
			fprintf (fout, "| %-40.40s ", inmr_rec.description);
			fprintf (fout, "                   |\n");

			fprintf (fout, "|------------------");
			fprintf (fout, "|------------------");
			fprintf (fout, "-------------------------------------------|\n");

			first_inmr = FALSE;
		}
		fprintf (fout, "| %-16.16s ", 
			(!strcmp (sinmr_rec.alternate, "                ")) ? "Alternate" : "Sunonym");
		fprintf (fout, "| %-16.16s ", sinmr_rec.item_no);
		fprintf (fout, "| %-40.40s |\n", sinmr_rec.description);
		fflush (fout);
		cc = find_rec (inmr2, &sinmr_rec, NEXT, "r");
	}
}

void
HeadingPrint (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL) 
	{
		file_err (errno, "pformat", "POPEN");
		return;
	}
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".9\n");
	fprintf (fout, ".L100\n");
	fprintf (fout, ".E%s\n", ML ("ALTERNATE/SYNONYM STOCK BREAKDOWN REPORT"));
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".Eas at %s\n", SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EBranch: %s\n", clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R===================");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "===================");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|                  ");
	fprintf (fout, "           ITEM DESCRIPTION                |\n");

	fflush (fout);
}

/*====================================================
| Process checking of Synonym stock item processing. |
====================================================*/
void
ProcessCheck (
 void)
{
	HeadingCheck ();

	abc_selfield (inmr, "inmr_hhsi_hash");
	abc_selfield (inmr2, "inmr_hhbr_hash");

	inmr_rec.hhbr_hash	=	1L;
	cc = find_rec (inmr, &inmr_rec, GTEQ, "u");
	while (!cc)
	{
		/*---------------------------------------
		| Find Synonym Items Without A Master	|
		----------------------------------------*/
		ProcessSynonym ();

		abc_unlock (inmr);
		cc = find_rec (inmr, &inmr_rec, GTEQ, "u");
	}
	CloseAudit ();
}

void
ProcessSynonym (
 void)
{
	sinmr_rec.hhsi_hash	=	inmr_rec.hhsi_hash;
	cc = find_rec (inmr2, &sinmr_rec, COMPARISON, "r");
	if (cc)
	{
		dsp_process ("Item Number", inmr_rec.item_no);
		fprintf (fout, "|-------------------");
		fprintf (fout, "|-------------------------------------------");
		fprintf (fout, "|--------------------------------|\n");

		fprintf (fout, "| %-16.16s  ", inmr_rec.item_no);
		fprintf (fout, "| %-40.40s  ", inmr_rec.description);
		fprintf (fout, "| Synonym link missing. %s|\n", (updateInmr) ? "Resetting" : "         " );

		fflush (fout);

		if (updateInmr)
		{
			inmr_rec.hhsi_hash = 0L;
			cc = abc_update (inmr, &inmr_rec);
			if (cc)
				file_err (cc, "inmr", "DBUPDATE");
		}
	}
}

void
HeadingCheck (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".8\n");
	fprintf (fout, ".L110\n");
	fprintf (fout, ".E%s\n", ML ("ALTERNATE/SYNONYM STOCK CHECK REPORT"));
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R===================");
	fprintf (fout, "============================================");
	fprintf (fout, "===================================\n");

	fprintf (fout, "===================");
	fprintf (fout, "============================================");
	fprintf (fout, "===================================\n");

	fprintf (fout, "|    ITEM NUMBER    |");
	fprintf (fout, "             ITEM DESCRIPTION              |");
	fprintf (fout, "          COMMENTS              |\n");

	fflush (fout);
}
/*=======================================================
|	Routine to close the audit trail output file.	|
=======================================================*/
void
CloseAudit (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

int
heading (
 int scn)
{
	if (restart) 
	{
		abc_unlock (inmr);
		return (EXIT_SUCCESS);
	}
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	if (alternateItem)
		rv_pr (ML ("Alternate item maintenance"), 26, 0, 1);
	else
		rv_pr (ML ("Synonym Stocking item maintenance"), 21, 0, 1);
	move (0, 1);
	line (80);

	move (1, input_row);
	if (scn == 1)
		box (0, 3, 80, 2);

	move (0, 21);
	line (80);
	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 0, err_str, comm_rec.co_no, clip (comm_rec.co_name));
	move (0, 23);
	line (80);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
