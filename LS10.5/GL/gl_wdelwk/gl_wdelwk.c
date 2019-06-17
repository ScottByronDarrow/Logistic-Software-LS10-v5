/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_wdelwk.c,v 5.3 2001/08/09 09:14:01 scott Exp $
|  Program Name  : (gl_wdelwk.c)
|  Program Desc  : (Delete G/L Work Transactions)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author        : Scott Darrow.    |
|---------------------------------------------------------------------|
| $Log: gl_wdelwk.c,v $
| Revision 5.3  2001/08/09 09:14:01  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:40  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:05  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_wdelwk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_wdelwk/gl_wdelwk.c,v 5.3 2001/08/09 09:14:01 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct wfbcRecord	wfbc_rec;

	char	*data	= "data";

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
	int		pid;		/* Process id number for work files.	*/
	static	int	glwkNo	=	0;	/* File no. of glwk work file.		*/

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int	 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	shutdown_prog 		 (void);
void 	Update 				 (void);

/*==========================
| Main processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	init_scr ();

	if (argc < 2)
	{
		print_at (0,0,  mlStdMess046, argv [0]);
        return (EXIT_FAILURE);
	}
	pid = atoi (argv [1]);

	if (OpenDB ())
        return (EXIT_FAILURE);

	print_mess ("Deleting General Ledger Work File.");

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "gl_wdel_cr"))
		Update ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
OpenDB (void)
{
	char	filename [100],
			*sptr = getenv ("PROG_PATH");

	sprintf (filename, "%s/WORK/gl_work%05d", (sptr) ? sptr : "/usr/LS10.5", pid);
	cc = RF_OPEN (filename, sizeof (glwkRec), "u", &glwkNo);
	if (cc)
        return (EXIT_FAILURE);

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");

    return (EXIT_SUCCESS);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	RF_DELETE (glwkNo);

	abc_fclose (wfbc);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*================================
| Update Workfile Batch Control. |
================================*/
void
Update (void)
{
	/*-------------------------------------------
	| Delete GLWK gen led batch control record.	|
	-------------------------------------------*/
	strcpy (wfbc_rec.co_no, comm_rec.co_no);
	wfbc_rec.pid_no = pid;
	sprintf (wfbc_rec.work_file, "gl_work%05d", pid);
	cc = find_rec (wfbc, &wfbc_rec, COMPARISON, "r");
	if (!cc)
	{
		cc = abc_delete (wfbc);
		if (cc)
			file_err (cc, wfbc, "DBDELETE");
	}
}
