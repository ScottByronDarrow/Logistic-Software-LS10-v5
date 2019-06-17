/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: select.i.c,v 5.3 2002/07/17 09:58:10 scott Exp $
|  Program Name  : ( so_select.i.c    )                               |
|  Program Desc  : ( Sales order reports selection input program. )   |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
| $Log: select.i.c,v $
| Revision 5.3  2002/07/17 09:58:10  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:22:06  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:52:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/15 05:14:55  scott
| Updated to fix prompts as not compatable with LS10-GUI.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: select.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_select.i/select.i.c,v 5.3 2002/07/17 09:58:10 scott Exp $";

#include 	<pslscr.h>
#include	<get_lpno.h>		
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>

   	char 	progdesc [101];
	char	status [2],
			type_flag [2];

	char	yesPrompt [11],
			noPrompt  [11];

	extern	int		TruePosition;

#include	"schema"

struct commRecord	comm_rec;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	back [2];
	char	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
	long	startDate;
	long	endDate;
	int		printerNumber;
	char	printerString [3];
} local_rec;
	
static	struct	var	vars []	={	

	{1, LIN, "startDate", 4, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Start Date        ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate}, 
	{1, LIN, "endDate", 5, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "End Date          ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate}, 
	{1, LIN, "printerNumber", 7, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number    ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "back", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background        ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 8, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight         ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "onightDesc", 9, 22, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "(", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
int  	spec_valid 		(int);
int  	heading 		(int);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	date1 [11];
	char	date2 [11];

	TruePosition	=	TRUE;
	if (argc < 5)
	{
		print_at (0,0,mlSoMess766,argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	sprintf (progdesc,"%.100s",argv [2]);
	sprintf (type_flag,"%-1.1s", argv [3]);
	sprintf (status,"%-1.1s", argv [4]);

	if (type_flag [0] == 'S')
		vars [label ("endDate")].required = ND;

	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	search_ok 	= TRUE;
	init_vars (1);

	sprintf (yesPrompt, "%-10.10s", ML ("YES"));
	sprintf (noPrompt,	"%-10.10s", ML ("NO"));
	heading (1);
	entry (1);
    if (prog_exit || restart) { 
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }
	
	strcpy (date1,DateToString (local_rec.startDate));
	strcpy (date2,DateToString (local_rec.endDate));

	/*====================================
	| Test for forground or background . |
	====================================*/
	rset_tty ();	
	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	abc_dbclose ("data");

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
		{
			if (type_flag [0] == 'S')
			{
				execlp 
				(
					"ONIGHT",
					"ONIGHT",
					argv [1],
					local_rec.printerString,
					date1,
					status,
					progdesc, (char *)0
				);
			}
			else
			{
				execlp 
				(
					"ONIGHT",
					"ONIGHT",
					argv [1],
					local_rec.printerString,
					date1,
					date2,
					status,
					progdesc, (char *)0
				);
			}
		}
	}
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
		{
			if (type_flag [0] == 'S')
			{
				execlp 
				(
					argv [1],
					argv [1],
					local_rec.printerString,
					date1,
					status, (char *) 0
				);
			}
			else
			{
				execlp 
				(
					argv [1],
					argv [1],
					local_rec.printerString,
					date1,
					date2,
					status, (char *) 0
				);
			}
		}
	}
	else
	{
		if (type_flag [0] == 'S')
		{
			execlp 
			(
				argv [1],
				argv [1],
				local_rec.printerString,
				date1,
				status, (char *) 0
			);
		}
		else
		{
			execlp 
			(
				argv [1],
				argv [1],
				local_rec.printerString,
				date1,
				date2,
				status, (char *) 0
			);
		}
	}
	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	crsr_on ();
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startDate"))
	{
		if (dflt_used)
		{
			if (F_NOKEY (label ("endDate")))
				local_rec.startDate = StringToDate (local_rec.systemDate);
			else
				local_rec.startDate = 0L;

			DSP_FLD ("startDate");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			if (F_NOKEY (label ("endDate")))
				local_rec.endDate = 0L;
			else
				local_rec.endDate = StringToDate (local_rec.systemDate);

			DSP_FLD ("endDate");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}
	
		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.printerString,"%2d",local_rec.printerNumber);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') ? yesPrompt
																: noPrompt);
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.oniteDesc, (local_rec.onite [0] == 'Y') ? yesPrompt
																  : noPrompt);
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		sprintf (err_str, ML (mlSoMess182),progdesc);
		rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);

		line_at (1,0,80);

		box (0,3,80,6);

		line_at (6,1,80);
		line_at (20,0,80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
