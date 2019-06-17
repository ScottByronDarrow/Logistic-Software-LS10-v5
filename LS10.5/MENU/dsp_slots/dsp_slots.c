/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: dsp_slots.c,v 5.1 2001/08/09 05:13:24 scott Exp $
-----------------------------------------------------------------------
| $Log: dsp_slots.c,v $
| Revision 5.1  2001/08/09 05:13:24  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:08:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:58  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:09  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.6  1999/12/06 01:47:11  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.5  1999/09/17 07:26:55  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dsp_slots.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dsp_slots/dsp_slots.c,v 5.1 2001/08/09 05:13:24 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<sys/types.h>
#include	<utmp.h>

int		main	(int argc, char * argv []);

int
main (
 int	argc,
 char *	argv [])
{
	int	fd;
	int	slot = 1;
	struct	utmp	ttybuf;
	int	name_size = sizeof(ttybuf.ut_name);
	int	line_size = sizeof(ttybuf.ut_line);

	if ((fd = open("/etc/utmp",0)) == -1)
		return (errno);

	init_scr();

	set_tty();

	Dsp_open(0,0,10);

	sprintf(err_str," Slot  %-*.*s   %-*.*s ",
		line_size,
		line_size,
		"Device",
		name_size,
		name_size,
		"User");
	Dsp_saverec(err_str);
	Dsp_saverec("");
	Dsp_saverec(" [FN14] [FN15] [FN16] ");
	while (read(fd,&ttybuf,sizeof(struct utmp)) > 0)
	{
		if (ttybuf.ut_line[0])
		{
			sprintf(err_str,"  %2d ^E %-*.*s ^E %-*.*s ",
				slot++,
				line_size,
				line_size,
				ttybuf.ut_line,
				name_size,
				name_size,
				ttybuf.ut_name);
			Dsp_saverec(err_str);
		}
		else
			slot++;
	}

	Dsp_srch();
	Dsp_close();
	close(fd);
	rset_tty();
	return (EXIT_SUCCESS);
}
