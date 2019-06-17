/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( error_utils.c  )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written : 24/09/90         | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (07/12/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|     (07/12/93) : Added Library version string into this file 'cos   |
|                : it's linked in with all Pinnacle applications      |

	$Log: error_utils.c,v $
	Revision 5.0  2001/06/19 06:59:16  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:36  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:20  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:13  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.7  1999/11/15 06:46:40  scott
	Updated for compile problems in AIX
	
	Revision 1.6  1999/09/30 04:32:26  jonc
	Tightened the prototype call list.
	
	Revision 1.5  1999/09/23 23:33:36  jonc
	Fixed typo and minor warning.
	
	Revision 1.4  1999/09/23 22:38:16  jonc
	Commented out attempt to close all open files. The use of _NFILE has
	been deprecated on most modern UNIX systems; besides which we don't
	particularly need to do this anyhow.
	
=====================================================================*/
#include	<std_decs.h>

#include	<VerNo.h>

extern	char	*PROG_VERSION;
extern	char	*PNAME;

/*=======================================================================
| sys_err displays system errors where text is the text passed by the   |
| program and value is the error number.                                |
=======================================================================*/
void
sys_err (
 const char *text,
 int value,
 const char *prg_name)
{
	char	err_no_str[11];
	int	status = 0;

	rset_tty();

	sprintf(err_no_str,"%d", value);
	if (fork() == 0)
	{
		execlp( "psl_error",
			"psl_error",
			 prg_name,
			 PROG_VERSION,
			 err_no_str,
			 text,
			 NULL);

		execlp ("no_option", "no_option", NULL);
	}
	else
		wait(&status);

#if 0
	/*-----------------------
	| Close EVERY file	|
	-----------------------*/
/* Since we're about to terminate with somewhat EXTREME prejudice, why BOTHER*/
/* closing a bunch of handles that are in an indeterminate state ANYWAY!*/
	for (fd = 3;fd < _NFILE;fd++)
		close(fd);
#endif

        exit(-1);

	/* Stop fussy old gcc complaining abount an unused variable */
	libver[0] = '@';
}
