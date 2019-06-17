/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( unload_env.c   )                                 |
|  Program Desc  : ( Unload Environment File to stdout            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 07/11/88         |
|---------------------------------------------------------------------|
|  Date Modified : 07/11/88        | Modified  by  : Roger Gibbison.  |
|  Date Modified : (21/08/92)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      :                                                    |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: unload_env.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/unload_env/unload_env.c,v 5.2 2001/08/09 09:27:48 scott Exp $";

#include	<pslscr.h>
#include	<time.h>

#include	<osdefs.h>
#include	<tcap.h>
#include	<ttyctl.h>
#include	<license2.h>
#include	<pinn_env.h>
#include	<stdlib.h>

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	int	cc;
	int	env_fd = open_env();
	PinnEnv	env_rec;

	init_scr();
	set_tty();

	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, FALSE);

	cc = RF_READ(env_fd, (char *) &env_rec);

	while (!cc)
	{
		printf("%1s",clip(env_rec.env_name));
		printf("\t%1s",clip(env_rec.env_value));
		printf("\t%1s\n",clip(env_rec.env_desc));
		cc = RF_READ(env_fd, (char *) &env_rec);
	}

	close_env(env_fd);

	rset_tty();
	return (EXIT_SUCCESS);
}
