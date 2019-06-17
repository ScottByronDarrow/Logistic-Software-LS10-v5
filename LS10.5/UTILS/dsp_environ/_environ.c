/*=====================================================================
|  Copyright (C) 1989, 1990 Logistic Software Limited.                |
|=====================================================================|
|  Program Name  : ( dsp_environ.c  )                                 |
|  Program Desc  : ( Display environment variables.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 22/06/90         |
|---------------------------------------------------------------------|
|  Date Modified : (05/10/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      :                                                    |
|   (05/10/1999) :  Ported to ANSI standards.                         |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _environ.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/dsp_environ/_environ.c,v 5.2 2001/08/09 09:26:52 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>

struct	env_type {
	char	env_name [16];
	char	env_value [31];
	char	env_desc [71];
} env_rec;

FILE	*fsort;

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	load_env		(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	init_scr();			/*  sets terminal from termcap	*/
	swide();
	set_tty();

	load_env();

	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	clear();
	snorm();
	rset_tty();
}

void
load_env (
 void)
{
	int	env_fd = open_env();
	char *	sptr;
	char	szTemp [256];

	Dsp_open(0,0,18);
	Dsp_saverec("  Variable Name  |   Environment Variable Value   |                             Description                                ");
	Dsp_saverec("");
	Dsp_saverec(" [FN03] [FN14] [FN15] [FN16] ");

	RF_REWIND(env_fd);

	fsort = sort_open("environ");

	cc = RF_READ(env_fd, (char *) &env_rec);

	while (!cc)
	{
		sprintf (szTemp, " %-15.15s ^E %-30.30s ^E %-70.70s\n",env_rec.env_name,env_rec.env_value,env_rec.env_desc);
		sort_save (fsort, szTemp);
		cc = RF_READ(env_fd, (char *) &env_rec);
	}

	fsort = sort_sort(fsort,"environ");

	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		cc = Dsp_saverec(sptr);
		if (cc)
			break;

		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"environ");

	close_env(env_fd);

	Dsp_srch();

	Dsp_close();
}
