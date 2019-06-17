/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: remtprn.i.c,v 5.6 2002/07/17 09:57:04 scott Exp $
|  Program Name  : (cr_remtprn.i.c) 
|  Program Desc  : (Suppliers remitances print date selection. ) 
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 14/05/90         |
|---------------------------------------------------------------------|
| $Log: remtprn.i.c,v $
| Revision 5.6  2002/07/17 09:57:04  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2001/12/10 00:38:44  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.4  2001/12/07 07:21:54  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: remtprn.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_remtprn.i/remtprn.i.c,v 5.6 2002/07/17 09:57:04 scott Exp $";

#include <pslscr.h>		
#include <get_lpno.h>		
#include <ml_cr_mess.h>		
#include <ml_std_mess.h>		

#define	BY_SUP		 (local_rec.sortByValue [0] == 'S')
#define	REPRINT		 (local_rec.reprintValue [0] == 'Y')

#include    "schema"

struct commRecord   comm_rec;

   	char 	programName [40],
			programDesc [100];

	char	systemDate [11];

	extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sortByDesc [10];
	char	sortByValue [2];	
	char	reprintDesc[5];
	char	reprintValue [2];	
	char	startCheque [7];
	char	endCheque [7];
	long	startDate;
	int		printerNumber;
} local_rec;
	
static	struct	var	vars []	={	
	{1, LIN, "printerNumber", 4, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1","Printer number    ", " ", 
		YES, NO, JUSTRIGHT, "123456789", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "sortByValue", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S","Sort By           ", "Sort By S(upplier / C(heque No.", 
		YES, NO, JUSTLEFT, "SC", "", local_rec.sortByValue}, 
	{1, LIN, "sortByDesc", 6, 30, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.sortByDesc},
	{1, LIN, "reprintValue", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Reprint Y/N.      ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.reprintValue},
	{1, LIN, "reprintDesc", 7, 30, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.reprintDesc}, 
	{1, LIN, "startCheque", 8, 2, CHARTYPE, 
		"NNNNNN", "          ", 
		"0", "", "Start Cheque No   ", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.startCheque}, 
	{1, LIN, "endCheque", 9, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		"0", "",  "End Cheque No.    ", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.endCheque}, 
	{1, LIN, "st_date", 10, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Start Date        ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*
 * Local function prototypes
 */
void	shutdown_prog	 (void);
int		spec_valid		 (int);
void	SetupDefaults	 (int);
void	CloseDB		 	 (void);
void	OpenDB		 	 (void);
int		heading			 (int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	TruePosition	=	TRUE;
	if (argc != 3)
	{
		print_at (0, 0, ML (mlStdMess037), argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (programName, argv [1]);
	strcpy (programDesc, argv [2]);

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/
	
	OpenDB ();

   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
   	search_ok 	= TRUE;

	init_vars (1);

	strcpy (systemDate, DateToString (TodaysDate ()));
	local_rec.startDate = TodaysDate ();
	strcpy (local_rec.reprintDesc, ML ("No "));
	strcpy (local_rec.reprintValue, "N");
	local_rec.printerNumber = 1;
	strcpy (local_rec.sortByDesc, ML ("Supplier "));
	strcpy (local_rec.sortByValue, "S");
	strcpy (local_rec.startCheque, "      ");
	strcpy (local_rec.endCheque, "      ");

	SetupDefaults (1);

	heading (1);
	scn_display (1);
	edit (1);
	if (restart || prog_exit) 
	{
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	
	strcpy (local_rec.sortByDesc, "");
	strcpy (local_rec.reprintDesc,"");

	rset_tty ();
	CloseDB (); 
	FinishProgram ();

	sprintf 
	(
		err_str,
		"\"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
		programName,
		local_rec.printerNumber,
		local_rec.sortByValue,
		local_rec.reprintValue,
		local_rec.startCheque,
		local_rec.endCheque,
		DateToString (local_rec.startDate)
	);
	SystemExec (err_str, FALSE);
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

int
spec_valid (
 int field)
{
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			errmess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sortByValue"))
	{
		strcpy (local_rec.sortByDesc, 
			(BY_SUP) ? ML ("Supplier ") : ML ("Cheque No"));

		SetupDefaults (1);
		DSP_FLD ("sortByDesc");
		DSP_FLD ("startCheque");
		DSP_FLD ("endCheque");
		DSP_FLD ("st_date");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reprintValue"))
	{
		strcpy (local_rec.reprintDesc, (REPRINT) ? ML ("Yes") : ML ("No "));
		DSP_FLD ("reprintDesc");

		if (REPRINT)
			SetupDefaults (1);
		else
			SetupDefaults (0);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_date"))
	{
		if (local_rec.startDate > TodaysDate ())
		{
			errmess (ML (mlStdMess086));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SetupDefaults (
	int		enable)
{
	if (BY_SUP)
	{
		FLD ("startCheque") 	= NA;
		FLD ("endCheque") 		= NA;
		FLD ("st_date") 		= (enable) ? YES : NA;
		strcpy (local_rec.startCheque, "      ");
		strcpy (local_rec.endCheque, "      ");
	}
	else
	{
		FLD ("startCheque") 	= (enable) ? YES : NA;
		FLD ("endCheque") 		= (enable) ? YES : NA;
		FLD ("st_date") 		= NA;
		local_rec.startDate 	= 0L;
	}
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
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

		sprintf (err_str, " %s ", ML (mlCrMess140));
		rv_pr (err_str, 26, 0, 1);
		line_at (1,0,80);

		if (scn == 1)
			box (0,3,80,7);
		
		line_at (5,1,79);
		line_at (20,0,80);
		line_at (22,0,80);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
