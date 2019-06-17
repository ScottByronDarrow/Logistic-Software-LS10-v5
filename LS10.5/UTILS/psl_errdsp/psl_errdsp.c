/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( psl_error.c & no_option.c )                      |
|  Program Desc  : ( Logs All System Errors in programs.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  errs,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/09/90         |
|---------------------------------------------------------------------|
|  Date Modified : (12/11/92)      | Modified  by  : Scott Darrow.    |
|                                                                     |
|  Comments      : Added psl_envdsp option.                           |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_errdsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_errdsp/psl_errdsp.c,v 5.1 2001/08/09 09:27:25 scott Exp $";

#define		X_OFF	0
#define		Y_OFF	0

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<psl_errtxt.h>
#include	<get_lpno.h>

extern	int		sys_nerr;
#ifndef	LINUX
extern	char	*sys_errlist[];
#endif	// LINUX

	int	lpno = 1,
		psl_error = FALSE,
		psl_env = FALSE;

	char	disp_str[255];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void disp_error (void);
void disp_env (void);
void prnt_env (char *env_name);
void check_len (char *dsp_str);
void disp_line (char *dsp_str);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr;

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
        sptr = argv[0];
	else
        sptr++;

	if (!strcmp (sptr, "psl_errdsp"))
        psl_error = TRUE;

	if (!strcmp (sptr, "psl_envdsp"))
        psl_env = TRUE;

    set_tty();
    init_scr();

    clear();
    swide();
    crsr_off();

	if ( psl_error )
		disp_error ();

	if ( psl_env )
		disp_env ();

	snorm ();
	rset_tty ();
    return (EXIT_SUCCESS);

}

/*=================
| Display errors. |
=================*/
void
disp_error (void)
{
	int	i;

	Dsp_prn_open(0, 0, 18, "LOGISTIC ERROR REPORTING.",
				(char *)0, (char *)0,
				(char *)0, (char *)0,
				(char *)0, (char *)0 );

	Dsp_saverec(" Error No |                                         E r r o r     D e s c r i p t i o n .                                         ");
	Dsp_saverec("");
	Dsp_saverec("[REDRAW]  [PRINT]  [NEXT SCREEN]  [PREV SCREEN]  [EDIT/END]");
				
	for ( i = 1; i < sys_nerr; i++)
	{
		sprintf(disp_str, "  %6d  ^E %-119.119s", i, sys_errlist[ i ]);
		Dsp_saverec( disp_str );
	}

	for (i = 0;error_codes[ i ].code != 0; i++)
	{
		sprintf(disp_str, "  %6d  ^E %-119.119s", error_codes[ i ].code,
					error_codes[ i ].msg1);
		Dsp_saverec( disp_str );

		if ( strlen( error_codes[ i ].msg2 ) > 1 )
		{
			sprintf(disp_str, "          ^E %-119.119s",
							error_codes[ i ].msg2);
			Dsp_saverec( disp_str );
		}

		if ( strlen( error_codes[ i ].msg3 ) > 1 )
		{
			sprintf(disp_str, "          ^E %-119.119s",
							error_codes[ i ].msg3);
			Dsp_saverec( disp_str );
		}
		Dsp_saverec("^^GGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	}
	
	Dsp_srch();
	Dsp_close();
}

/*=======================
| Display environments. |
=======================*/
void
disp_env (void)
{
	Dsp_prn_open(0, 0, 18, "LOGISTIC UNIX ENVIRONMENTS .",
				(char *)0, (char *)0,
				(char *)0, (char *)0,
				(char *)0, (char *)0 );

	Dsp_saverec(" Unix Environment |                                                       D e s c r i p t i o n .                                ");
	Dsp_saverec("");
	Dsp_saverec("[REDRAW]  [PRINT]  [NEXT SCREEN]  [PREV SCREEN]  [EDIT/END]");

	prnt_env( "PSL_ENV_NAME" );
	prnt_env( "PATH" );
	prnt_env( "DATA_PATH" );
	prnt_env( "DBPATH" );
	prnt_env( "DBDATE" );
	prnt_env( "TERM_SLOT" );
	prnt_env( "COPYRIGHT" );
	prnt_env( "MAIL_USED" );
	prnt_env( "FILE_PERM" );
	prnt_env( "LPDIR" );
	prnt_env( "EXINIT" );
	prnt_env( "WP_USED" );
	prnt_env( "SPREAD_SHEET" );
	prnt_env( "LOGNAME" );
	
	Dsp_srch();
	Dsp_close();
}

void
prnt_env (
 char*              env_name)
{
	char	*sptr;

	sptr = getenv ( env_name );
	if ( !sptr )
		return;

	sprintf(disp_str, "^1%-18.18s^6^E %-108.108s", env_name, sptr);
	Dsp_saverec( disp_str );
	disp_line( sptr );
}

/*===================
| Check for length. |
===================*/
void
check_len (
 char*              dsp_str)
{
	if ( strlen( dsp_str ) < 108 )
		return;

	sprintf(disp_str, "                  ^E %-108.108s", dsp_str + 108 );
	Dsp_saverec( disp_str );

	if ( strlen( dsp_str ) < 216 )
		return;

	sprintf(disp_str, "                  ^E %-108.108s", dsp_str + 216 );
	Dsp_saverec( disp_str );

	if ( strlen( dsp_str ) < 324 )
		return;

	sprintf(disp_str, "                  ^E %-108.108s", dsp_str + 324 );
	Dsp_saverec( disp_str );

	if ( strlen( dsp_str ) < 432 )
		return;

	sprintf(disp_str, "                  ^E %-108.108s", dsp_str + 432 );
	Dsp_saverec( disp_str );

	return;
}

/*==========================================
| Display line break and check for length. |
==========================================*/
void
disp_line (
 char*              dsp_str)
{
	check_len( dsp_str );

	Dsp_saverec("^^GGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
}
