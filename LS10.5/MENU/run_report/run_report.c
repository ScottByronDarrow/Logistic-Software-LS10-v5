/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: run_report.c,v 5.4 2001/11/15 07:53:38 robert Exp $
|  Program Name  : (run_report.c)
|  Program Desc  : (Run End of Month Reports)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 13/07/87         |
|---------------------------------------------------------------------|
| $Log: run_report.c,v $
| Revision 5.4  2001/11/15 07:53:38  robert
| Updated for LS10-GUI
|
| Revision 5.3  2001/11/15 03:12:20  scott
| Cleaned up and added PID to arguments.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: run_report.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/run_report/run_report.c,v 5.4 2001/11/15 07:53:38 robert Exp $";

#define	MAXLINE	200
#define	CODE	com [indx].lbl

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#include	"schema"

struct commRecord	comm_rec;

char *	args [20];

#ifdef GVISION
	extern 		bool	gbSysExec;
#endif	/* GVISION */

/*
 * Initialise structure type 'co' and set label members, value members
 * are assigned in the function get_db_com () & later passed passed to 
 * the Substit () function for variable expansion.                     
 */
struct	tco {
	char *	lbl;
	char *	val;
} com [] = {
	{"CONO", 	0}, 
	{"CONAME", 	0}, 
	{"COSHORT", 	0}, 
	{"BRNO", 	0}, 
	{"BRNAME", 	0}, 
	{"BRSHORT", 	0}, 
	{"WHNO", 	0}, 
	{"WHNAME", 	0}, 
	{"WHSHORT", 	0}, 
	{"DBTDATE", 	0}, 
	{"CRDDATE", 	0}, 
	{"INVDATE", 	0}, 
	{"GLDATE", 	0}, 
	{"GLPER", 	0}, 
	{"FIS", 		0}, 
	{"LPNO", 	0}, 
	{"PERFROM", 	0}, 
	{"PERTO", 	0}, 
	{"PID", 		0}, 
	{"", ""}
};

char	fiscalMonth 	[3], 
		printerString 	[3], 
		glPeriod 		[3], 
		periodPlus 		[3], 
		periodMinus 	[3], 
		PID				[7], 
		dates 			[4][11];
		
FILE *	fileName;

/*
 * Local function prototypes 
 */
void	shutdown_prog	 (void);
void	ReadComm		 (void);
void	ProcessFile		 (void);
void	ProcessLine		 (char *);
void	ExecuteCommand	 (void);
char *	Substitute		 (char *);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc, 
 char *	argv [])
{
	char *prog_path;
	char fullpath [256];
	
	if (argc != 2)
	{
		print_at (0, 0, ML (mlMenuMess703), argv [0]);
		return (EXIT_FAILURE);
	}

	// Search in current directory, if not found search in PROG_PATH/BIN/SCRIPT
	if ((fileName = fopen (argv [1], "r")) == NULL)
	{
		prog_path = getenv ("PROG_PATH");
		if (prog_path == NULL)
		{
			sprintf (err_str, "PROG_PATH not defined");
			sys_err (err_str, errno, PNAME);
		}

		sprintf (fullpath, "%s/BIN/SCRIPT/%s", prog_path, argv [1]);
		if ((fileName = fopen (fullpath, "r")) == NULL)
		{
			sprintf (err_str, "Error in %s During (FOPEN)", argv [1]);
			sys_err (err_str, errno, PNAME);
		}
	}

	/*
	 * Setup required parameters.
	 */
	init_scr ();

	ReadComm ();

	ProcessFile ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	fclose (fileName);
}

/*
 * Get common info from commom database file.
 */
void
ReadComm (void)
{	
	int		gl_per;
	char *	sptr = chk_env ("MEND_LP");

	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_dbclose ("data");

	strcpy (dates [0], DateToString (comm_rec.dbt_date));
	strcpy (dates [1], DateToString (comm_rec.crd_date));
	strcpy (dates [2], DateToString (comm_rec.inv_date));
	strcpy (dates [3], DateToString (comm_rec.gl_date));
	sprintf (PID, "%06d", getpid());

	sprintf (glPeriod, "%2.2s", dates [3] + 3);
	sprintf (fiscalMonth, "%d", comm_rec.fiscal);
	sprintf (printerString, "%1.1s", (sptr != (char *)0) ? sptr : "1");

	gl_per = atoi (glPeriod);
	sprintf (periodPlus, "%02d", (gl_per == 12) ? 1 : gl_per + 1);
	sprintf (periodMinus, "%02d", gl_per);

	com [0].val 	= comm_rec.co_no;
	com [1].val 	= comm_rec.co_name;
	com [2].val 	= comm_rec.co_short;
	com [3].val 	= comm_rec.est_no;
	com [4].val 	= comm_rec.est_name;
	com [5].val 	= comm_rec.est_short;
	com [6].val 	= comm_rec.cc_no;
	com [7].val 	= comm_rec.cc_name;
	com [8].val 	= comm_rec.cc_short;
	com [9].val 	= dates [0];
	com [10].val 	= dates [1];
	com [11].val 	= dates [2];
	com [12].val 	= dates [3];
	com [13].val 	= glPeriod;
	com [14].val 	= fiscalMonth;
	com [15].val 	= printerString;
	com [16].val 	= periodPlus;
	com [17].val 	= periodMinus;
	com [18].val 	= PID;
}

void
ProcessFile (void)
{
	char *	sptr;	
	char	line [MAXLINE];

	sptr = fgets (line, MAXLINE, fileName);

	/*
	 * Process ALL lines in the file
	 */
	while (sptr != (char *)0)
	{
		if (sptr != (char *)0)
			line [strlen (sptr) - 1] = (char) NULL;

		/*
		 * Split line into parameters etc & Substit
		 */
		ProcessLine (line);

		/*
		 * Execute cmd line
		 */
		ExecuteCommand ();
		
		sptr = fgets (line, MAXLINE, fileName);
	}
}

void
ProcessLine (
	char 	*line)
{
	char *	sptr = line;
	char *	tptr = line;
	int		indx = 0;
	int		quote = 0;

	while (*tptr)
	{
		if (*tptr == '~' && !quote)
		{
			*tptr = (char) NULL;

			/*
			 * Nothing to be in background
			 */
			if (!strcmp (sptr, "BACK"))
				sptr = tptr + 1;
			else
			{
				if (indx == 0)
					args [indx++] = sptr;
				else
					args [indx++] = Substitute (sptr);
				args [indx] = NULL;
				sptr = tptr + 1;
			}
		}
		if (*tptr == '"')
			quote = !quote;
		tptr++;
	}
}

void
ExecuteCommand (void)
{
	int	status;

#ifdef GVISION

	// gbSysExec, if set to TRUE waits for the child process to exit.
	gbSysExec = TRUE;
	status  =   execvp (args [0], args);	
#else
	if (!fork ())
		execvp (args [0], args);
	else
		wait (&status);
#endif
}

char *
Substitute (
 char *	string)
{
	int	indx;

	for (indx = 0; strlen (com [indx].lbl); indx++)
	{
		if (!strcmp (CODE, string))
			return (com [indx].val);
	}
	return (string);
}
