char	*PNAME = "$RCSfile: ON.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/ON/ON.c,v 5.1 2001/08/09 09:26:38 scott Exp $";

#include	<pslscr.h>
#include	<osdefs.h>
#include	<tcap.h>
#include	<ttyctl.h>

int
main (
 int	argc,
 char *	argv [])
{
	init_scr();
	snorm();
	crsr_on();
	gr_off();
	set_tty();
	rset_tty();
	clear();
	return (EXIT_SUCCESS);
}
