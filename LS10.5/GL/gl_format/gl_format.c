/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_format.c,v 5.2 2001/08/09 09:13:43 scott Exp $
|  Program Name  : (gl_format.c)
|  Program Desc  : (Format General Ledger Date For Work File routines 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: gl_format.c,v $
| Revision 5.2  2001/08/09 09:13:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:17:47  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_format.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_format/gl_format.c,v 5.2 2001/08/09 09:13:43 scott Exp $";

/*
 *   Include file dependencies  
 */
#include	<ml_gl_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include	<GlUtils.h>

#include	"schema"

struct commRecord	comm_rec;

   /*
    * Special fields and flags  ##################################
    */
   int		printerNo	=	1,
   			noData		=	TRUE;

   char		journalType [3];

/*
 *   Constants, defines and stuff   
 */
	char 	*data	=	"data";

/*
 *   Local function prototypes  
 */
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	shutdown_prog 	 (void);

/*
 * Main processing routine.
 */
int
main (
 int argc, 
 char *argv [])
{
	char	termNumber [11];
	if (argc < 3)
	{
		print_at (0,0,mlGlMess701,argv [0]);	
		return (EXIT_SUCCESS);
	}

	sprintf (journalType,"%2.2s", argv [1]);
	printerNo = atoi (argv [2]);
	OpenDB ();
	init_scr ();

	sprintf (termNumber, "%010ld", (long) comm_rec.term);
 
	print_mess (ML ("Formating General Ledger Work Data."));

	noData	=	TRUE;
	/*
	 * Process General Ledger Transactions.
	 */
	strcpy (glwkRec.tran_type, journalType);
	strcpy (glwkRec.sys_ref, termNumber);

    cc = find_rec (glwk, &glwkRec, GTEQ, "u");
   	while (!cc && !strcmp (glwkRec.tran_type,journalType) && 
           		  !strcmp (glwkRec.sys_ref,termNumber))
	{
		if (glwkRec.stat_flag [0] == '2' &&
	    	    !strcmp (journalType,glwkRec.tran_type) &&
	    	    !strcmp (glwkRec.co_no,comm_rec.co_no))
		{
			noData	=	FALSE;

			GL_AddBatch ();
	
			cc = abc_delete (glwk);
			if (cc)
                file_err (cc, glwk, "DBDELETE");

			cc = find_rec (glwk, &glwkRec, GTEQ, "u");
		}
		else
		{
			abc_unlock (glwk);
			cc = find_rec (glwk, &glwkRec,NEXT, "u");
		}
   	}

	if (cc)
        abc_unlock (glwk);

	shutdown_prog ();

	if (noData == FALSE)
        return (EXIT_SUCCESS);
	else
        return (EXIT_FAILURE);
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGlwk ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close data base files
 */
void
CloseDB (
 void)
{
	GL_CloseBatch (printerNo);
	GL_Close ();
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/* [ end of file ] */
