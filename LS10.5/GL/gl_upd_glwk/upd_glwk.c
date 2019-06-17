/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: upd_glwk.c,v 5.2 2001/08/09 09:14:00 scott Exp $
|  Program Name  : (gl_upd_glwk.c) 
|  Program Desc  : (Updated glwk work file.)
|                 (replaces inf.glwk.sql)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/06/92         |
|---------------------------------------------------------------------|
| $Log: upd_glwk.c,v $
| Revision 5.2  2001/08/09 09:14:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:18:04  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: upd_glwk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_upd_glwk/upd_glwk.c,v 5.2 2001/08/09 09:14:00 scott Exp $";

#include	<pslscr.h>
#include    <GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_gl_mess.h>

#define		STK_ISS_REC		(!strcmp (glwkRec.tran_type, "10"))
#define		STK_PUR			(!strcmp (glwkRec.tran_type, "11"))
#define		STK_ADJ			(!strcmp (glwkRec.tran_type, "12"))
#define		STK_COST_SAL	(!strcmp (glwkRec.tran_type, "13"))
#define		BLANK			(!strcmp (glwkRec.tran_type, "  ") && \
                                  glwkRec.stat_flag [0] == ' ' && \
                                  !strcmp (glwkRec.sys_ref, "          "))

	char	termNoStr [11];
	long	terminalNo;

/*
 * Local Function Prototypes.
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Process 		(void);
void 	shutdown_prog 	(int);

/*
 * Main processing routine. 
 */
int
main (
 int                argc,
 char*              argv [])
{
	if (argc < 2)
	{
		print_at (0,0, mlGlMess700, argv [0]);
        return (EXIT_FAILURE);
	}

	terminalNo = atol (argv [1]);
	sprintf (termNoStr,"%010ld", terminalNo);

	OpenDB ();

	dsp_screen ("Updating General Ledger Work file."," ", " ");

	/*
	 * Process general ledger transactions.
	 */
	glwkRec.hhgl_hash = 0L;
	cc = find_rec (glwk, &glwkRec, GTEQ, "u");
	while (!cc)
	{
		if (BLANK)
		{
			abc_delete (glwk);
			glwkRec.hhgl_hash = 0L;
			cc = find_rec (glwk, &glwkRec, GTEQ, "u");
			continue;
		}
		if (glwkRec.stat_flag [0] == '2')
			Process ();
		else
			abc_unlock (glwk);

		cc = find_rec (glwk, &glwkRec, NEXT, "u");
	}
	abc_unlock (glwk);

	shutdown_prog (0);
    return (EXIT_SUCCESS);
}

/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	OpenGlwk () ; abc_selfield (glwk, "glwk_hhgl_hash");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose ("data");
}

/*
 * Process all valid glwk transactions.
 */
void
Process (void)
{
	/*
	 * Update valid glwk transactions.
	 */
	if (STK_ISS_REC || STK_PUR || STK_ADJ || STK_COST_SAL)
	{
		dsp_process ("Reference", glwkRec.sys_ref);
		strcpy (glwkRec.sys_ref, termNoStr);
		cc = abc_update (glwk, &glwkRec);
		if (cc)
			file_err (cc, glwk, "DBUPDATE");
	}
	else
		abc_unlock (glwk);
}

void
shutdown_prog (
 int                rcode)
{
	CloseDB (); 
	FinishProgram ();
}
