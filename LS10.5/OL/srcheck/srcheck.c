/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: srcheck.c,v 5.8 2002/01/18 02:29:56 robert Exp $
|  Program Name  : (essrcheck.c   )                                 |
|  Program Desc  : (Check Branch Security Record.               )   |
|---------------------------------------------------------------------|
|  Date Written  : 16/03/89        |  Author     : Huon Butterworth.  |
|---------------------------------------------------------------------|
| $Log: srcheck.c,v $
| Revision 5.8  2002/01/18 02:29:56  robert
| Updated to use errmess instead of print_at to display error message
| in LS10-GUI
|
| Revision 5.7  2001/12/10 02:54:23  scott
| .
|
| Revision 5.6  2001/12/10 02:53:00  scott
| Updated from testing
|
| Revision 5.5  2001/12/10 02:50:59  scott
| Updated to fix warning
|
| Revision 5.4  2001/12/10 02:49:35  scott
| Updated to fix exec problems.
|
| Revision 5.3  2001/10/19 09:30:30  cha
| Fix Issue 612. Put delay in Error messages.
| Corrected execvp arguments.
|
| Revision 5.2  2001/08/09 09:14:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:57  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 06:42:35  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: srcheck.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/srcheck/srcheck.c,v 5.8 2002/01/18 02:29:56 robert Exp $";

#include 	<pslscr.h>
#include 	<ctype.h>
#include 	<ml_std_mess.h>
#include 	<ml_ol_mess.h>

#define		MAX_TRYS	3

#define	EXIT_VAL	 (exit_val) ? 0 : 1

#include	"schema"

struct essrRecord	essr_rec;
struct essrRecord	prog_rec;

	int		securityLevel	=	-1;
/*
 * Table names
 */
static char
	*data	= "data";

extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy			[11];
	char	companyNumber	[3];
	char	branchNumber 	[3];
	char	passwd 			[14];
	int		securityLevel;
	int		securityok;
	int		numberTrys;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "passwd",	 12, 20, CHARTYPE,
		"_____________", "          ",
		" ", "", "Password : ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.passwd},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
int 	spec_valid 			 (int);
int 	PasswordCheck 		 (int, char **);
int 	CheckSecurityLevel 	 (char *, char *, char *);
int	 	RunProgram 			 (char **);
void 	ShowError 			 (int, char *, char *);
int 	heading 			 (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	if (argc < 4)
	{
		clear ();	
	    print_at (0,0,mlOlMess031, argv[0]);
		return (EXIT_FAILURE);
	}
	    
	local_rec.securityok	=	1;

	sprintf (local_rec.companyNumber, "%2.2s", argv [1]);
	sprintf (local_rec.branchNumber,  "%2.2s", 	argv [2]);
	sprintf (local_rec.passwd, 		  "%-13.13s",argv [3]);

	OpenDB ();

	local_rec.securityLevel = 	CheckSecurityLevel 
								 (
									local_rec.companyNumber,
									local_rec.branchNumber,
									local_rec.passwd
								);
	if (local_rec.securityLevel == -1)
	{
		clear ();
		upshift (local_rec.passwd);
		ShowError (4, ML ("No security set up for "), local_rec.passwd);
		CloseDB (); 
		FinishProgram ();
		return (EXIT_FAILURE);
	}

	local_rec.numberTrys	=	0;
	abc_selfield (essr, "essr_pass_no");

	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		local_rec.securityok	=	FALSE;

		init_vars (1);

		heading (1);
		entry (1);
	}
	CloseDB (); 
	FinishProgram ();
	if (local_rec.securityok)
	{
		++argv;
		if (argc > 4)
			return (RunProgram (++argv));
	}
	return (EXIT_FAILURE);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
	void)
{
	abc_dbopen (data);
	open_rec (essr, essr_list, ESSR_NO_FIELDS, "essr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
	void)
{
	abc_fclose (essr);
	abc_dbclose (data);
}

int
spec_valid (
	int field)
{
        
	if (local_rec.numberTrys >= MAX_TRYS)
	{
		prog_exit	=	TRUE;
		local_rec.securityok	=	FALSE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("passwd"))
	{
		strcpy (essr_rec.co_no,		local_rec.companyNumber);
		strcpy (essr_rec.est_no,	local_rec.branchNumber);
		strcpy (essr_rec.op_passwd, local_rec.passwd);
		upshift (essr_rec.op_passwd);
		cc = find_rec (essr, &essr_rec, COMPARISON, "r");
		if (cc)
		{
			ShowError (1,ML (mlStdMess180), "       ");
			local_rec.numberTrys++;
			if (local_rec.numberTrys >= MAX_TRYS)
			{
				prog_exit	=	TRUE;
				local_rec.securityok	=	FALSE;
				return (EXIT_SUCCESS);
			}
			return (FALSE);
		}
		if (local_rec.securityLevel > essr_rec.sec_level)
		{
			ShowError (1,ML (mlStdMess140), "      ");
			local_rec.numberTrys++;
			if (local_rec.numberTrys >= MAX_TRYS)
			{
				prog_exit	=	TRUE;
				local_rec.securityok	=	FALSE;
				return (EXIT_SUCCESS);
			}
			return (FALSE);
		}
		local_rec.securityok	=	TRUE;
		prog_exit	=	TRUE;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

int
PasswordCheck (
	int securityLevel, 
	char *argv [])
{
	if (!securityLevel)
		return (TRUE);

	abc_selfield (essr, "essr_pass_no");
	sprintf (essr_rec.co_no, "%2s", argv[0]);
	sprintf (essr_rec.est_no, "%2s", argv[1]);

	
	while (local_rec.securityok) 
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		scn_display (1);
		scn_write (1);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		entry (1);

		if (prog_exit || restart) 
			continue;
	}	
	if (!local_rec.securityok)
		ShowError (1,ML (mlOlMess045), argv[2]);
	return (TRUE);
}

int
CheckSecurityLevel (
	char	*CoNo,
	char	*BrNo,
	char	*operatorID)
{
	sprintf (essr_rec.co_no, "%2.2s", 	CoNo);
	sprintf (essr_rec.est_no,"%2.2s", 	BrNo);
	sprintf (essr_rec.op_id, "%-14.14s",operatorID);
	upshift (essr_rec.op_id);
	cc = find_rec (essr, &essr_rec, COMPARISON, "r");
	if (cc)
		return (-1);
	
	return (essr_rec.sec_level);
}

int
RunProgram (
	char *argv [])
{
	char	checkString [15];
	int		status	=	0;
	/*--------------------------------------------------------------------------
	| Move program name into 1st argv slot and Operator ID. into 2nd argv slot.|
	--------------------------------------------------------------------------*/
	argv[0] = argv[1];
	argv[1] = essr_rec.op_id;

	strcpy (checkString, argv[0]);
	upshift (checkString);
	if (!strncmp (checkString, argv[1], strlen (checkString)))
		sprintf (argv[1], "%-14.14s", " ");

	CloseDB (); 
	FinishProgram ();

	switch (fork())
	{
		case -1:
		return 0;
	
		case 0:
			/*
		 	*	Child process
		 	*/
			status	=	execvp (argv [0], argv);
			return (EXIT_FAILURE);
	
		default:
			/*
		 	*	Parent process
		 	*/
			wait ((int *)0);
	}
	if (status)
		ShowError (4,ML (mlOlMess046), argv [0]);

	return (status);
}
void
ShowError (
	int loop_cnt, 
	char *mess1, 
	char *mess2)
{
#ifndef GVISION
	int	i;
	for (i = 0; i < loop_cnt ; i++)
	{
		if (i % 2)
			print_at (12, 19, " %s%s ", mess1, mess2);
		else
			print_at (12, 19, "%R %s%s ", mess1, mess2);
		sleep (sleepTime);
	}
#else
	char errmsg [100];
	sprintf (errmsg, " %s%s ", mess1, mess2);
	errmess (errmsg);
	sleep (sleepTime);
#endif
}

int
heading (
	int scn)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	crsr_off ();
	box (18,9,42,3);
	line_at (11,19,41);
	print_at (10, 25, ML (mlOlMess016));

	scn_write (scn);
	return (EXIT_SUCCESS);
}

