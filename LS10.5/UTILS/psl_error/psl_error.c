/*=====================================================================
|  Copyright (C) 1996 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (psl_error.c & no_option.c)                        |
|  Program Desc  : (Logs All System Errors in programs.        )      |
|                  (                                          )       |
|---------------------------------------------------------------------|
|  Access files  :  errs,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/09/90         |
|---------------------------------------------------------------------|
|  Date Modified : (12/03/92)      | Modified  by  : Campbell Mander. |
|  Date Modified : (14/09/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (01/10/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : Fixed definition of tloc;                          |
|  (14/09/94)    : PSL 11341.  Removed call to sys_err ().            |
|  (17/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
-----------------------------------------------------------------------
	$Log: psl_error.c,v $
	Revision 5.3  2002/07/17 09:58:18  scott
	Updated to change argument to get_lpno from (1) to (0)
	
	Revision 5.2  2001/08/09 09:27:26  scott
	Updated to add FinishProgram () function
	
	Revision 5.1  2001/08/06 23:58:53  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 08:23:20  robert
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 02:44:17  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.1  2001/01/22 00:08:31  scott
	Updated to add PROG_PATH and DBPATH to ERR.LOG file output
	
	Revision 3.0  2000/10/10 12:24:29  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 09:15:33  gerry
	Forced Revision No Start 2.0 Rel-15072000
	
	Revision 1.18  2000/07/10 01:53:54  scott
	Updated to replace "@ (" with "@(" to ensure psl_what works correctly
	
	Revision 1.17  2000/02/21 00:54:09  scott
	S C/ LSANZ - 15962 (AIL)
	Updated to send error via mail if unable to write to ERR.LOG even if mail is turned off.
	Updated to write error to "ERR.LOG.<user-name>" if unable to write to ERR.LOG
	Updated to send email error last as this effectes foreground check.
	
	Revision 1.16  2000/02/15 00:04:44  scott
	Removed signal code as implementation from version 10 as it is unrealiable.
	Version 9.10 will not support signal timeout for error logging until Jonathan has time to implement new signals that are better supported on all platforms.
	Related to problems found on RS6000 at DPL.
	
	Revision 1.15  2000/01/31 00:43:06  scott
	Updated to look for possible causes of why program does not work at DPL.
	Will need to add a #define for AIX to remove alarm functions if these changes do not work.
	
	Revision 1.14  1999/12/15 09:44:46  scott
	Updated to add environment ERR_MAIL_LOG to turn off mail logging if required.
	
	Revision 1.13  1999/12/06 01:48:16  scott
	Updated to change CTime to SystemTime due to conflict with VisualC++.
	
	Revision 1.12  1999/11/25 10:24:32  scott
	Updated to remove c++ comment lines and replace with standard 'C'
	
	Revision 1.11  1999/11/16 08:11:37  scott
	Update for warnings due to usage of -Wall flags.
	
	Revision 1.10  1999/10/15 23:51:22  scott
	Updated from ansi project
	
	Revision 1.9  1999/09/28 03:53:50  jonc
	Replace sys_errlist usage with strerror (as sys_errlist isn't std on AIX).
	
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_error.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_error/psl_error.c,v 5.3 2002/07/17 09:58:18 scott Exp $";

#define		_TIMEOUT	120
#define		X_OFF	22
#define		Y_OFF	5

#include	<pslscr.h>
/*
#include	<signal.h>
*/
#include	<psl_errtxt.h>
#include	<get_lpno.h>

	int		noOption 		= FALSE,
			printerNumber 	= 1,
			envErrMailLog	= FALSE,
			errorNumber		= 0;

	char	*programName,
			*programVersion, 
			*programText,
			*userName;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	CentreDsp 		(char *, int, int);
void	DecodeVersion 	(const char *);
void 	DisplayError 	(void);
void 	LogError 		(char *, char *, char *);
void 	MailError		(char *, char *, char *);
void 	PrintErr 		(char *, char *, char *);
void 	SignalAlarmFunc (int);

int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	/*
	 *	Check to see whether psl_error has been invoked recursively
	 */
	if (getenv ("SEL_ERROR_INVOKED"))
		return (EXIT_FAILURE);
	putenv ("SEL_ERROR_INVOKED=");

	if (argc != 5)
	{
		print_at (0,0, "Usage : %s progName progVersion errNo text\n", argv[0]);
		return (EXIT_FAILURE);
	}

	if (!strncmp (argv[0], "no_option",9))
		noOption = TRUE;
	else
		noOption = FALSE;

	sptr = chk_env ("ERR_MAIL_LOG");
	envErrMailLog = (sptr == (char *)0) ? TRUE : atoi (sptr);

	programName 	= argv [1];
	programVersion 	= argv[2];
	errorNumber 	= atoi (argv[3]);
	programText 	= argv [4];

	set_tty ();

	userName = getenv ("LOGNAME");

	DisplayError ();

	rset_tty ();
	return (EXIT_SUCCESS);
}

void
MailError (
	char	*mess1,
	char	*mess2,
	char	*mess3)
{
	FILE * sendmail;

	if (!(sendmail = popen ("/usr/lib/sendmail support", "w")))
		return;

	fprintf (sendmail, "Subject       : Error Report\n\n");
	fprintf (sendmail, "Program       : %s\n", programName);
	fprintf (sendmail, "User Name     : %s\n", userName ? userName
												    	 : "User Unknown.");
	fprintf (sendmail, "Version       : %s\n", programVersion);
	fprintf (sendmail, "Date          : %s\n", SystemTime ());
	if (noOption)
		fprintf (sendmail, "Menu Option : %s\n", programText);
	else
		fprintf (sendmail, "Desc        : %s (%d)\n", programText, errorNumber);

	fprintf (sendmail, "Comments      : %s\n", mess1);
	fprintf (sendmail, "              : %s\n", mess2);
	fprintf (sendmail, "              : %s\n", mess3);

	pclose (sendmail);
}

/*======================
| Log error to ERR.LOG |
======================*/
void
LogError (
	char	*mess1,
	char	*mess2,
	char	*mess3)
{
	char	workFileName [41];
	FILE 	*errFileOutput;

	if ((errFileOutput = fopen ("ERR.LOG","a")) == NULL)
	{
		/*----------------------------------------------------------
		| Set mail of error on even if not defined in environment. |
		----------------------------------------------------------*/
		envErrMailLog	=	TRUE;
		sprintf (workFileName, "ERR.LOG.%s", userName);

		if ((errFileOutput = fopen (workFileName,"a")) == NULL)
			exit (1);
	}
	fprintf (errFileOutput,
		"================================================================================\n");

	fprintf (errFileOutput, "Program  : %s\n", programName);

	fprintf (errFileOutput,
		"User Name: %s\n",
		userName ? userName : "User Unknown.");

	fprintf (errFileOutput, "Version  : %s\n", programVersion);
	fprintf (errFileOutput, "Date     : %s\n", SystemTime ());
	fprintf (errFileOutput, "PROG_PATH: %s\n", getenv ("PROG_PATH"));
	fprintf (errFileOutput, "DBPATH   : %s\n", getenv ("DBPATH"));

	if (noOption)
		fprintf (errFileOutput, "Menu Option : %s\n", programText);
	else
		fprintf (errFileOutput, "Desc     : %s (%d)\n", programText, errorNumber);

	fprintf (errFileOutput, "Comments : %s\n", mess1);
	fprintf (errFileOutput, "         : %s\n", mess2);
	fprintf (errFileOutput, "         : %s\n", mess3);

	fprintf (errFileOutput,
		"================================================================================\n");

	fclose (errFileOutput);
}

void
CentreDsp (
	char	*str,
	int		y,
	int		effect)
{
	rv_pr (str, (80 - strlen (str)) / 2, y, effect);
}
/*
void
SignalAlarmFunc (
	int		signo)
{
    return;
}
*/

/*==========================
| Display error to screen. |
==========================*/
void
DisplayError (void)
{
	int		key;
	int		p = 0;
	int		pos = 0;
	int		errorCodeFound	=	FALSE;
	int		messageLength = 0;
	char	menuWorkString[255];
	char	menuErrorMess[3][150];

	if (foreground ())
	{
		init_scr ();

     	clear ();
		crsr_off ();
		box (1,3,78,16);

		CentreDsp (ML ("LOGISTIC SOFTWARE INTERNAL ERROR LOGGING"), 4, 1);

		move (2,5);
		line (77);

		if (noOption)
		{
			sprintf (menuWorkString,"Menu Option : %s ", programText);
			CentreDsp (menuWorkString, 9, 0);
		}
		else
		{
			CentreDsp ("Service call must be logged with your supplier.",6, 0);

			sprintf (menuWorkString,"Program : %s", programName);
			CentreDsp (menuWorkString, 7, 0);

			DecodeVersion (programVersion);

			sprintf (menuWorkString,"Desc : %s (%d) ", programText, errorNumber);
			CentreDsp (menuWorkString, 10, 0);
		}

		CentreDsp ("SYSTEM  DESCRIPTION OF  ERROR.", 12, 1);

		move (14,13);
		line (54);

		if (!userName)
		{
			sprintf (menuWorkString," Error (%d) has being logged. " , errorNumber);
		}
		else
		{
			sprintf (menuWorkString,
				" %s please note that your error (%d) has been logged. " ,
				userName,
				errorNumber);
		}

     	CentreDsp (menuWorkString, 18, 0);
	}

	if (noOption)
	{
		sprintf (menuErrorMess[0],"%s","Menu option does not exist or an error occured while executing program.");
		sprintf (menuErrorMess[1],"%s","If menu option has NOT been used before then your supplier will need to");
		sprintf (menuErrorMess[2],"%s","release to you all relevant program (s) to allow this option to be used.");

	}
	else
	{
		if (errorNumber > 1 && errorNumber < 100)
		{
			sprintf (menuErrorMess[0],"%s", strerror (errorNumber));
			sprintf (menuErrorMess[1],"%s","Please reference Operating System manual for full details.");
			sprintf (menuErrorMess[2],"%s","A Possible Kernel change may be required if error has occured before.");
		}
		else
		{
			for (p = 0; error_codes[p].code != 0 && 
		    	    	error_codes[p].code <= errorNumber;p++)
			{
				if (error_codes[p].code == errorNumber)
				{
					errorCodeFound = 1;
					pos = p;
					break;
				}
			}
			if (errorCodeFound)
			{
				sprintf (menuErrorMess[0],"%s",error_codes[pos].msg1);
				sprintf (menuErrorMess[1],"%s",error_codes[pos].msg2);
				sprintf (menuErrorMess[2],"%s",error_codes[pos].msg3);
			}
			else
			{
				sprintf (menuErrorMess[0],"%s","Error code not found");
				sprintf (menuErrorMess[1],"%s","No description has been found for the current error.");
				sprintf (menuErrorMess[2],"%s","Please reference Operating System manual for possible details.");
			}
		}
	}
	if (foreground ())
	{
		messageLength  = strlen (menuErrorMess[0]);
		if (messageLength < (int) strlen (menuErrorMess[1])) 
			messageLength  = strlen (menuErrorMess[1]);

		if (messageLength < (int) strlen (menuErrorMess[2])) 
			messageLength  = strlen (menuErrorMess[2]);

     		rv_pr (menuErrorMess[0], 40 - (messageLength / 2), 14,0);
     		rv_pr (menuErrorMess[1], 40 - (messageLength / 2), 15,0);
     		rv_pr (menuErrorMess[2], 40 - (messageLength / 2), 16,0);

	}
	LogError (menuErrorMess[0], menuErrorMess[1], menuErrorMess[2]);

	if (foreground ())
	{
		strcpy (err_str, " Please press [PRINT] to print error or any other key to exit. ");
		messageLength  = strlen (err_str);

		for (p = 0; p < 4; p++)
		{
			putchar (BELL);

			rv_pr (err_str, 40 - (messageLength / 2), 20, (p % 2));
			fflush (stdout);
			sleep (sleepTime);
		}

		
		key = getkey ();
		/*
		key = 0;
		if (signal (SIGALRM, SignalAlarmFunc) != SIG_ERR)
		{
			alarm (_TIMEOUT);
			key = getkey ();
			alarm (0);
		}
		*/

		if (key == FN5)
			PrintErr (menuErrorMess[0], menuErrorMess[1], menuErrorMess[2]);
	
		clear ();
		crsr_on ();
	}
	else
		PrintErr (menuErrorMess[0], menuErrorMess[1], menuErrorMess[2]);

	if (envErrMailLog)
		MailError (menuErrorMess[0], menuErrorMess[1], menuErrorMess[2]);
}

void
DecodeVersion (
 const char*        str)
{
	/*
	 *	Print on lines 8 - 9
	 */
	int		shown_id = FALSE;
	char	*beg,
			*end;
	char	*version = strdup (str);
	char	buf [256];

	for (beg = end = version;
		!shown_id;
		beg = end = end + 1)
	{
		/*
		 *	Get a token for examination
		 */
		while (*end && !isspace (*end))
			end++;
		if (!*end)
		{
			free (version);
			return;
		}
		*end = '\0';

		/*
		 *	Stuff to ignore
		 */
		if (!*beg)
			continue;
		if (!strcmp (beg, "@(#)"))				/* SCCS header */
			continue;
		if (!strcmp (beg, "-"))					/* LogisticSoftware marker */
			continue;
		if (!strncmp (beg, "$Header:", 8))		/* Possible RCS header */
			continue;

		/*
		 *	Current word is the CVS Repository location
		 */
		sprintf (buf, "Id: %s", beg);
		CentreDsp (buf, 8, 0);

		/*
		 *	The assumption is that the version id follows next
		 */
		sprintf (buf, "Version: %s", end + 1);
		CentreDsp (buf, 9, 0);
		shown_id = TRUE;
	}
	free (version);
}

/*=========================
| Print error to printer. |
=========================*/
void
PrintErr (
 char*              mess1,
 char*              mess2,
 char*              mess3)
{
	FILE *	fout;
	
	if (foreground ())
	{
		printerNumber = get_lpno (0);

		rv_pr ("  Please wait, Printing report. ",24,6,1);
	}
	else
		printerNumber = 1;

	if ((fout = popen ("pformat","w")) == NULL)
		return;

	fprintf (fout,".START%s\n","00/00/00");
	fprintf (fout,".PI12\n");

	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".6\n");
	fprintf (fout,".L139\n");

	fprintf (fout,".ELOGISTIC SOFTWARE INTERNAL ERROR LOGGING\n");

	fprintf (fout,".EAS AT : %s\n",SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout, "=======================================");
	fprintf (fout, "=========================================\n");

	fprintf (fout, ".R=======================================");
	fprintf (fout, "=========================================\n");

	fprintf (fout, "Program  : %s\n", programName);

	fprintf (fout,"User Name: %s\n", 
			 (userName == (char *)0) ? "User Unknown." : userName);

	if (!noOption)
		fprintf (fout,"Version  : %s\n", programVersion);

	fprintf (fout,"Date     : %s\n", SystemTime ());

	if (noOption)
		fprintf (fout,"Menu Option : %s\n", programText);
	else
		fprintf (fout,"Desc     : %s (%d)\n", programText,errorNumber);

	fprintf (fout,"Comments : %s\n", mess1);
	fprintf (fout,"         : %s\n", mess2);
	fprintf (fout,"         : %s\n", mess3);

	fprintf (fout,".EOF\n");
	pclose (fout);
}
