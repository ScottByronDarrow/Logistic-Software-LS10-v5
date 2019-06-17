/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: batch_del.c,v 5.4 2001/08/20 23:12:47 scott Exp $
|  Program Name  : (gl_batch_del.c)                    
|  Program Desc  : (Misc Logistic file deletion Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 06/03/92         |
|---------------------------------------------------------------------|
| $Log: batch_del.c,v $
| Revision 5.4  2001/08/20 23:12:47  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 09:13:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:07  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:32  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: batch_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_batch_del/batch_del.c,v 5.4 2001/08/20 23:12:47 scott Exp $";

/*
 *   Include file dependencies  
 */
#include 	<pslscr.h>	
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<GlUtils.h>
#include 	<DeleteControl.h>

/*
 *   Constants, defines and stuff   
 */
#define		STANDING		(!strcmp (glbhRec.jnl_type, " 2"))
#define		HEADER_POSTED	(glbhRec.stat_flag [0] == 'P' && !STANDING)
#define		DETAIL_POSTED	(glblRec.stat_flag [0] == 'P')

#include	"schema"

struct commRecord	comm_rec;


char	*data  = "data";

/*
 * Local variables  
 */
char    systemDate [11];
long    lSystemDate;
long	deleteDays	=	0L;

/*
 * Local function prototypes  
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	DeleteGlbh 		(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char 	*argv [])
{
    OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate()));
	lSystemDate = TodaysDate ();

	init_scr ();
	dsp_screen ("Purging General Ledger batch files.",
                 comm_rec.co_no,comm_rec.co_name);

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "GENERAL-LEDGER-WORK");
	if (!cc)
	{
		deleteDays	=	(long) delhRec.purge_days;
	}
	DeleteGlbh ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	OpenGlbh ();
	OpenGlbl ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
    abc_dbclose (data);
}

void
DeleteGlbh (void)
{
	int		allLinesPosted	=	TRUE;

	deleteDays	=	(long) delhRec.purge_days;

	dsp_process ("DELETE","BATCH FILES");

	strcpy	(glbhRec.co_no, comm_rec.co_no);
	strcpy	(glbhRec.br_no, "  ");
	sprintf	(glbhRec.batch_no, "%10.10s", " ");
	cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no))
	{
		/*
		 * If header not posted and a least one month old. 
		 */
		if (!HEADER_POSTED || ((glbhRec.glbh_date + deleteDays) > lSystemDate))
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}
		/*
		 * Check if all lines have been posted.
		 */
		allLinesPosted		=	TRUE;
		glblRec.hhbh_hash	=	glbhRec.hhbh_hash;
		glblRec.line_no		=	0;

        cc = find_rec (glbl, &glblRec, GTEQ, "r");
		while (!cc && (glblRec.hhbh_hash == glbhRec.hhbh_hash))
		{
			if (!DETAIL_POSTED)
                allLinesPosted	=	FALSE;

			cc = find_rec (glbl, &glblRec, NEXT, "r");
		}

		if (!allLinesPosted)
		{
			cc = find_rec (glbh, &glbhRec, NEXT, "r");
			continue;
		}
		glblRec.hhbh_hash	=	glbhRec.hhbh_hash;
		glblRec.line_no		=	0;
        cc = find_rec (glbl, &glblRec, GTEQ, "u");
		while (!cc && (glblRec.hhbh_hash == glbhRec.hhbh_hash))
		{
			cc = abc_delete (glbl);
			if (cc)
				file_err (cc, glbl, "DBDELETE");

			glblRec.hhbh_hash	=	glbhRec.hhbh_hash;
			glblRec.line_no		=	0;
        	cc = find_rec (glbl, &glblRec, GTEQ, "u");
		}
		abc_unlock (glbl);

		cc = find_rec (glbh, &glbhRec, CURRENT, "u");
		if (!cc)
		{
			cc = abc_delete (glbh);
			if (cc)
				file_err (cc, glbh, "DBDELETE");
		}
		strcpy	(glbhRec.co_no, comm_rec.co_no);
		strcpy	(glbhRec.br_no, "  ");
		sprintf	(glbhRec.batch_no, "%10.10s", " ");
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
	}
}
