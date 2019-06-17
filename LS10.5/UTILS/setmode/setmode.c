#define	CCMAIN
char	*PNAME = "$RCSfile: setmode.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/setmode/setmode.c,v 5.1 2001/08/09 09:27:38 scott Exp $";

#include	<stdio.h>
#include	<pslscr.h>
#include	<fcntl.h>
#include	<ml_utils_mess.h>

extern	int	errno;
FILE	*fconf;
int	fd_prn;

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	stty_cmd[1025];
	int	i;

	if (argc < 2)
	{
		/*"\007\r\nUsage: -\t%s <device> <stty mode(s)>\r\n",*/
		print_at (0,0, ML(mlUtilsMess724), argv[0]);
		return (EXIT_FAILURE);
	}

	fd_prn = open (argv[1], O_RDONLY);
	if (fd_prn < 0)
	{
		/*"Open error [%d] on device %s", errno, argv[1]);*/
		print_at (0,0, ML(mlUtilsMess108), errno, argv[1]);
		return (EXIT_FAILURE);
	}

	if (argc > 2)
	{
		strcpy (stty_cmd, "/bin/stty ");
		for (i = 2; i < argc; i++)
		{
			strcat (stty_cmd, argv[i]);
			strcat (stty_cmd, " ");
		}
		strcat (stty_cmd, "< ");
		strcat (stty_cmd, argv[1]);

		system (stty_cmd);
	}

	while (1)
		sleep ((unsigned) 32767);

	return (EXIT_SUCCESS);
}
