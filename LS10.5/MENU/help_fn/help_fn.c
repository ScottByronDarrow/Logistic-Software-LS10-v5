/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( help_fn.c      )                                 |
|  Program Desc  : ( Window function key help.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 31/12/87         |
|---------------------------------------------------------------------|
|  Date Modified : (31/12/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (05/12/90)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (30/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      : (05/12/90) - Total re-write.                       |
|   (30/08/1999) :  Converted to ANSI format.                         |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: help_fn.c,v $
| Revision 5.2  2001/08/09 05:13:29  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:24  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:41  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:03  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:14  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:47:14  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 09:41:56  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/17 07:26:58  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:00  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: help_fn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/help_fn/help_fn.c,v 5.2 2001/08/09 05:13:29 scott Exp $";

#define	X_OFF	0
#define	Y_OFF	0
#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>
#include	<get_lpno.h>

	FILE	*fin;

	char	filename[200];

/*==========================
| Funtion prototypes       |
==========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	proc_file		(char * terminal);

/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	char	*term_type = getenv("TERM");
	char	*prog_path = getenv("PROG_PATH");

	init_scr();
	crsr_off();
	set_tty();

	if ( term_type == (char *)0 )
	{
		shutdown_prog ();
		return (-1);
	}

	sprintf(filename,"%s/BIN/MENUSYS/TERM/%s.d",
			(prog_path != (char *)0) ? prog_path : "/usr/LS10.5",
			term_type);

	if ( access( filename, 00) != 0 )
	{
		sprintf(filename,"%s/BIN/MENUSYS/TERM/ansi.d",
			       (prog_path != (char *)0) ? prog_path : "/usr/LS10.5");
	}

	if (( fin = fopen( filename, "r" )) == 0 )
	{
		sprintf(err_str,ML(mlMenuMess126),term_type);
		rv_pr(err_str, 2,0,1);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	proc_file( term_type );
	fclose(fin);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	rset_tty();
	crsr_on();
}

void
proc_file (
 char *	terminal)
{
	char	data_str[201];
	char	*sptr;

	sprintf(err_str, "TERMINAL TYPE IS ( %s )", terminal );

	Dsp_prn_open( 0, 0, 15, err_str, (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0 );

	Dsp_saverec("              F U N C T I O N   K E Y   H E L P   W I N D O W.               ");
	Dsp_saverec("");
	Dsp_saverec("[PRINT]  [NEXT SCREEN]  [PREV SCREEN]  [EDIT/END]");

	sptr = fgets( data_str, 200, fin);
	while ( sptr != (char *)0 )
	{
		if ( strlen( sptr ) == 1 )
		{
			sptr = fgets( data_str, 200, fin);
			continue;
		}
		*(sptr + strlen(sptr) - 1) = '\0';

		if ( data_str[0] == '~' )
			Dsp_saverec("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		else
			Dsp_saverec( sptr );

		sptr = fgets( data_str, 200, fin);
	}
	Dsp_srch();
	Dsp_close();
}
