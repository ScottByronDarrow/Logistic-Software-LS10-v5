/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: XML_Error.c,v 5.1 2001/08/06 22:40:54 scott Exp $
|=====================================================================|
|  Author            : Scott Darrow.  | Date Written  : 14th May 2001 |
|  Source Name       : XML_Error.c                                    |
|  Source Desc       : Logs XML errors.                               |
|                                                                     |
|  Library Routines  : XML_Error ()                                   |
|---------------------------------------------------------------------|
| $Log: XML_Error.c,v $
| Revision 5.1  2001/08/06 22:40:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:14  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/14 05:38:09  scott
| Updated to add new function XML_Errors.
|
=====================================================================*/
#include	<std_decs.h>
#include	<XML_Error.h>

	static	const	char	*xmle	=	"_xmle_error";
	static	struct	xmleRecord	xmleRec;

	/*=================+
	 | XML Error file. |
	 +=================*/
#define	XMLE_NO_FIELDS	10

	static	struct dbview	xmle_list [XMLE_NO_FIELDS] =
	{
		{"xmle_program"},
		{"xmle_date_err"},
		{"xmle_time_err"},
		{"xmle_user_name"},
		{"xmle_desc_1"},
		{"xmle_desc_2"},
		{"xmle_desc_3"},
		{"xmle_desc_4"},
		{"xmle_status"},
		{"xmle_stat_flag"}
	};

	struct xmleRecord
	{
		char	program [21];
		Date	date_err;
		char	time_err [6];
		char	user_name [15];
		char	desc_1 [61];
		char	desc_2 [61];
		char	desc_3 [61];
		char	desc_4 [61];
		char	status [2];
		char	stat_flag [2];
	};

	static void		TableSetup 		 (void),
					TableTeardown 	 (void);

	static	void 	MailError (char *, char *, char *, char *, char *, char *);
	static	int		envErrMailLog	= FALSE;
	static	char	envErrMailName [31];
	static	char	*currentUserName;

void
XML_Error (
	char	*programName,
	char	*errorDesc1,
	char	*errorDesc2,
	char	*errorDesc3,
	char	*errorDesc4)
{
	int		xmlerr;
	
	TableSetup ();

	currentUserName = getenv ("LOGNAME");

	xmleRec.date_err = TodaysDate ();
	sprintf (xmleRec.program, "%-20.20s", programName);
	sprintf (xmleRec.user_name, "%-14.14s", currentUserName);
	strcpy (xmleRec.time_err, TimeHHMM ());
	xmlerr = find_rec (xmle, &xmleRec, COMPARISON, "u");
	
	sprintf (xmleRec.desc_1, "%-60.60s", errorDesc1);
	sprintf (xmleRec.desc_2, "%-60.60s", errorDesc2);
	sprintf (xmleRec.desc_3, "%-60.60s", errorDesc3);
	sprintf (xmleRec.desc_4, "%-60.60s", errorDesc4);
	strcpy (xmleRec.status, "N");
	strcpy (xmleRec.stat_flag, "0");
		
	if (xmlerr)
		xmlerr = abc_add (xmle, &xmleRec);
	else
		xmlerr = abc_update (xmle, &xmleRec);

	if (xmlerr)
		file_err (xmlerr, xmle, "DBADD/DBUPDATE");

	if (envErrMailLog)
	{
		MailError
		(
			xmleRec.program,
			xmleRec.user_name,
			xmleRec.desc_1,
			xmleRec.desc_2,
			xmleRec.desc_3,
			xmleRec.desc_4
		);
	}
	TableTeardown ();
}

static void
TableSetup (void)
{
	char	*sptr;
	/*
	 *	Open all the necessary tables et al
	 */
	static int	done_this_before = FALSE;

	if (!done_this_before)
	{
		done_this_before = TRUE;

		abc_alias (xmle, "xmle");
	}
	open_rec (xmle, xmle_list, XMLE_NO_FIELDS, "xmle_id_no");

	sptr = chk_env ("ERR_MAIL_LOG");
	envErrMailLog = (sptr == (char *)0) ? TRUE : atoi (sptr);
	
	sptr = chk_env ("ERR_MAIL_NAME");
	strcpy (envErrMailName, (sptr == (char *)0) ? "/usr/lib/sendmail" : sptr);
}

static void
TableTeardown (void)
{
	abc_fclose (xmle);
}

static	void
MailError (
	char	*programName,
	char	*userName,
	char	*mess1,
	char	*mess2,
	char	*mess3,
	char	*mess4)
{
	char	workString [61];

	FILE * sendmail;

	sprintf (workString, "%s %s", envErrMailName, currentUserName);

	if (!(sendmail = popen (workString, "w")))
		return;

	fprintf (sendmail, "Subject       : XML Transmittion error\n");
	fprintf (sendmail, "Program       : %s\n", programName);
	fprintf (sendmail, "User Name     : %s\n", userName	? userName
													   	: "User Unknown.");
	fprintf (sendmail, "Comments      : %s\n", mess1);
	fprintf (sendmail, "              : %s\n", mess2);
	fprintf (sendmail, "              : %s\n", mess3);
	fprintf (sendmail, "              : %s\n", mess4);

	pclose (sendmail);
}

