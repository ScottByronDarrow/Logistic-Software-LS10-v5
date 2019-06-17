/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: GlAudit.c,v 5.2 2001/08/06 22:40:52 scott Exp $
|  Author            : Scott Darrow.   | Date Written  : 21/01/1991   |
|  Source Name       : glaudit.c                                      |
|  Source Desc       : Audit files for new general ledger.            |
|                                                                     |
|  Library Routines  : aud_* ()                                        |
|---------------------------------------------------------------------|
| $Log: GlAudit.c,v $
| Revision 5.2  2001/08/06 22:40:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
=====================================================================*/
#include	<std_decs.h>
#include	<GlUtils.h>
#include	<SystemTime.h>
#include	<GlAudit.h>
#define	LWIDTH 121

static	int		PV_lpno = 1, 
				PV_aud_stat = TRUE;
static	char	*PV_co_name;

static	double	PV_total;

/*
 **************************************************************************
 * 	Function	:	GL_AudULine ()
 **************************************************************************
 * 	Description	:	Print ruleoff for audit trail.
 * 
 * 	Parameters	:	fileID		- Pointer to FILE id of audit trail.
 * 					lineChar	- Character to be used in ruleoff.
 */
static	void
GL_AudULine (
	FILE	*fileID, 
	char 	lineChar)
{
	char	uline [256];

	pin_bfill (uline, lineChar, LWIDTH);
	uline [LWIDTH] = (char) NULL;

	fprintf (fileID, ".C%-*.*s\n", LWIDTH, LWIDTH, uline);
}

/*
 **************************************************************************
 * 	Function	:	GL_AudSetup ()
 **************************************************************************
 * 	Description	:	Sets up values needed by the posting audit.
 * 
 * 	Notes		:	Aud_setup must be run before any audit routines
 * 					are used.
 * 					The GL_AudSetup routine is automatically called
 * 					when a consolidated posting run is commenced.
 * 			
 * 	Parameters	:	companyName	- Name of the current company.
 * 					printerNo	- Number of the printer to be used.
 * 
 * 	Globals		:	PV_co_name  - Variable to Hold company name.
 * 				:	PV_lpno     - Variable to Hold printer number.
 * 				:	PV_aud_stat - Set to true.
 */
void
GL_AudSetup (
	char 	*companyName, 
	int 	printerNo)
{
	PV_co_name 	= companyName;
	PV_lpno 	= printerNo;
	PV_aud_stat = TRUE;
}

/*
 **************************************************************************
 * 	Function	:	GL_AudOpen ()
 **************************************************************************
 * 	Description	:	Open posting audit trail file.
 * 
 * 	Notes		:	This function is used to open / create posting
 * 					audit trails.
 * 			
 * 	Parameters	:	otype		- Open flag (See fopen (S)).
 * 					journalType	- Journal type.
 * 					processID	- Process id to be used in file name.
 * 
 * 	Returns		:	t_id	- Pointer to FILE id of opened file.
 */
FILE	*GL_AudOpen (
	char 	*openFlag, 
	char 	*journalType, 
	int 	processID)
{
	FILE	*t_id;

	if (! (t_id = fopen (GL_AudName (journalType, processID), openFlag)))
		file_err (errno, "audit", "FOPEN");

	return (t_id);
}

/*
 **************************************************************************
 * 	Function	:	GL_AudClose ()
 **************************************************************************
 * 	Description	:	Close a posting audit trail.
 * 
 * 	Notes		:	This routine closes but does not delete a
 * 					posting audit trail.
 * 			
 * 	Parameters	:	fileID	- 	Pointer to the FILE id of the audit
 * 								trail to be closed.
 * 
 * 	Globals		:	errno 	- 	Integer containing error number if
 * 					  			the close fails.
 * 					PNAME	-	String containing the program name.
 */
void
GL_AudClose (FILE *fileID)
{
	if (fclose (fileID))
		file_err (errno, "audit", "FCLOSE");
}

/*
 **************************************************************************
 * 	Function	:	GL_AudSave ()
 **************************************************************************
 * 	Description	:	Saves line in posting audit trail.
 * 
 * 	Notes		:	
 * 			
 * 	Parameters	:	fileID		-	Pointer to the FILE id returned by
 * 					  				a call to GL_AudOpen ().
 * 					accountNo	- 	General Ledger account number.
 * 					year		- 	Posting year (YYYY).
 * 					period		- 	Posting period (MM).
 * 					fgnAmount	- 	Posting amount.
 * 					locAmount	- 	Posting amount.
 * 					stat		- 	Posting status - (U)nposted, (P)osted.
 * 
 * 	Globals		:	GV_post_time - 	Posting time stamp set by call to poststamp
 */

void
GL_AudSave (
	FILE 	*fileID, 
	char	*accountNo, 
	int 	year, 
	int 	period, 
	double 	fgnAmount, 
	double 	locAmount, 
	char 	*currCode, 
	double	exchRate, 
	char	*stat)
{
	AUD_STRUCT	aud_buf;
	char	tmpAccount [32];
	extern	long	GV_post_time;

	strcpy (AUD_ACCOUNT_NO, accountNo);
	GL_FormAccNo (AUD_ACCOUNT_NO, tmpAccount, 0);
	AUD_YEAR 		= year;
	AUD_PERIOD 		= period;
	AUD_FGN_AMOUNT 	= fgnAmount;
	AUD_LOC_AMOUNT 	= locAmount;
	strcpy (AUD_CURR_CODE, currCode);
	AUD_EXCH_RATE 	= exchRate;
	AUD_TIME	 	= GV_post_time;
	strcpy (AUD_STATUS, stat);

	if (!fwrite (&aud_buf, sizeof (AUD_STRUCT), 1, fileID))
		file_err (errno, "audit", "FWRITE");
	
	fflush (fileID);
	PV_total += fgnAmount;
}

/*
 **************************************************************************
 * 	Function	:	GL_AudGet ()
 **************************************************************************
 * 	Description	:	Returns formatted line from posting audit file.
 * 
 * 	Notes		:
 * 			
 * 	Parameters	:	fileID	    - 	Pointer to the FILE id returned
 * 					      			by a prior call to GL_AudOpen.
 * 
 * 	Globals		:	PV_aud_stat - 	Set to FALSE if a FAILED posting, is read.
 * 
 * 	Returns		:	tmpString	- 	String containing formatted audit line.
 */
char	*GL_AudGet (
	FILE	*fileID)
{
	char	amt_str1 [17];
	char	amt_str2 [17];

	AUD_STRUCT	aud_buf;
	static	char	tmpString [255];
	char	status_str [12];
	struct	tm	*tm_ptr;

	if (!fread (&aud_buf, sizeof (AUD_STRUCT), 1, fileID))
		return ((char *) NULL);

	tm_ptr = localtime (&AUD_TIME);
	switch	 (*AUD_STATUS)
	{
	case	'A':
		strcpy (status_str, "NO ACCOUNT");
		break;

	case	'B':
		strcpy (status_str, "IMBALANCE ");
		break;

	case	'D':
		strcpy (status_str, "NO ACCOUNT");
		break;

	case	'P':
		strcpy (status_str, "POSTED    ");
		break;

	default:
		strcpy (status_str, "FAILED    ");
		break;
	}
	sprintf (amt_str1, "%16.16s", 
			comma_fmt (DOLLARS (AUD_LOC_AMOUNT), "N,NNN,NNN,NNN.NN"));
	sprintf (amt_str2, "%16.16s", 
			comma_fmt (DOLLARS (AUD_FGN_AMOUNT), "N,NNN,NNN,NNN.NN"));
	sprintf (tmpString,
		".C%-32.32s  %04d    %02d    %3.3s   %11.8f  %16.16s  %16.16s  %02d:%02d  %10.10s\n",
					AUD_ACCOUNT_NO,
					AUD_YEAR,
					AUD_PERIOD, 
					AUD_CURR_CODE,
					AUD_EXCH_RATE,
					amt_str2,
					amt_str1,
					tm_ptr->tm_hour, tm_ptr->tm_min,
					status_str);
	
	if (*AUD_STATUS == 'U')
		PV_aud_stat = FALSE;
	else
		PV_total += AUD_LOC_AMOUNT;

	return (tmpString);
}

/*
 **************************************************************************
 * 	Function	:	GL_AudName ()
 **************************************************************************
 * 	Description	:	Create full pathname for audit file.
 * 
 * 	Notes		:	Routine used to generate the full Unix pathname
 * 					of the posting audit trail.
 * 			
 * 	Parameters	:	journalType	 - Journal type.
 * 					processID	 - Process id to be used in file name.
 * 
 * 	Returns		:	filename - String containing generated file name.
 */
char	*GL_AudName (
	char	*journalType, 
	int		processID)
{
	static	char	filename [128];
	char	*sptr;

	int	journalNo;

	journalNo = atoi (journalType);

	sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/caud%02d.%05d", 
		 (sptr == (char *) 0) ? "/usr/ver9.10" : sptr, journalNo, processID);

	return (filename);
}

/*
 **************************************************************************
 * 	Function	:	GL_AudHeader ()
 **************************************************************************
 * 	Description	:	Print header for audit trail.
 * 
 * 	Notes		:	Aud_header outputs the values needed to 
 * 					configure pformat to print the audit trail.
 * 			
 * 	Parameters	:	pipeID	   - Pointer to FILE id for output.
 * 					recovery   - TRUE if recovery audit in progress.
 * 
 * 	Globals		:	PV_total   - Audit total is set to 0.0.
 * 				:	PV_lpno    - Printer number.
 * 				:	PV_co_name - Company name.
 */
void
GL_AudHeader (
	FILE	*pipeID, 
	int		recovery, 
	char	*journalDesc, 
	char	*runNo)
{
	PV_total = 0.0;

	fprintf (pipeID, ".START%s\n", DateToString (TodaysDate ()));
	fprintf (pipeID, ".SO\n");
	fprintf (pipeID, ".PL60\n");
	fprintf (pipeID, ".LP%d\n", PV_lpno);
	fprintf (pipeID, ".10\n");
	fprintf (pipeID, ".PI12\n");
	fprintf (pipeID, ".L%d\n", LWIDTH + 28);
	fprintf (pipeID, ".ECONSOLIDATED POSTING AUDIT TRAIL%s\n",
				 			(recovery) ? " - RECOVERY" : " ");
	fprintf (pipeID, ".B1\n");
	fprintf (pipeID, ".E%s\n", clip (PV_co_name));
	fprintf (pipeID, ".Eas at : %s\n", SystemTime ());
	fprintf (pipeID, ".E%s / Run No %s\n", journalDesc, runNo);
	fprintf (pipeID, ".B1\n");

	fprintf 
	(
		pipeID,
		".C%-32.32s  %4.4s  %6.6s  %4.4s  %11.11s  %16.16s  "
		"%16.16s  %-5.5s  %-10.10s\n",
		"ACCOUNT NUMBER",
		"YEAR",
		"PERIOD",
		"CURR",
		"EX. RATE",
		"FX AMOUNT",
		"LOC AMOUNT",
		"TIME",
		"STATUS"
	);
	GL_AudULine (pipeID, '=');
}

/*
 **************************************************************************
 * 	Function	:	GL_AudTrailer ()
 **************************************************************************
 * 	Description	:	Print totals for posting audit.
 * 
 * 	Notes		:	Aud_trailer prints the rule-off and totals
 * 					for the posting audit trail.
 * 					The totals should be ZERO.
 * 			
 * 	Parameters	:	pipeID	   - Pointer to FILE id for output.
 * 
 * 	Globals		:	PV_total   - Audit total.
 */
int
GL_AudTrailer (
	FILE	*fileID)
{
	GL_AudULine (fileID, '-');
	fprintf (fileID, ".CTOTAL%-80.80s%16.16s%19.19s\n",  
 			" ",
			comma_fmt (PV_total, "N,NNN,NNN,NNN.NN"),
			" ");
	GL_AudULine (fileID, '-');
	fprintf (fileID, ".EOF\n");

	return (PV_aud_stat);
}
