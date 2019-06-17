/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: load_env.c,v 5.2 2001/08/09 09:27:00 scott Exp $
-----------------------------------------------------------------------
| $Log: load_env.c,v $
| Revision 5.2  2001/08/09 09:27:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:20:07  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: load_env.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/load_env/load_env.c,v 5.2 2001/08/09 09:27:00 scott Exp $";

#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<stdio.h>

struct {
	char	envName [16];
	char	envValue [31];
	char	envDesc [71];
} envRec;

	extern	int		envMaintOption;

#define	BUF_SIZE	200
char	inputLine [BUF_SIZE];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int		Splat 	(char *);

int
main (
	int		argc,
	char	*argv [])
{
	FILE	*fin;
	char	*sptr;

	envMaintOption	=	TRUE;

	if (argc != 2)
	{
		print_at(0,0,mlUtilsMess709,argv [0]);
		return (EXIT_FAILURE);
	}

	if (!(fin = fopen (argv [1], "r")))
	{
		print_at (0,0,mlUtilsMess052,argv [1]);
		return (EXIT_FAILURE);
	}

	while ((sptr = fgets (inputLine, BUF_SIZE, fin)))
	{
		if (Splat(sptr))
			put_env(envRec.envName,envRec.envValue,envRec.envDesc);
	}
	fclose (fin);
	return (EXIT_SUCCESS);
}

int
Splat (
	char	*line)
{
	char	*sptr = line;
	char	*tptr = line;

	*(sptr + strlen(line) - 1) = '\0';

	while (*sptr && *sptr != '\t')
		sptr++;

	if (*sptr)
	{
		*sptr = '\0';
		sprintf(envRec.envName,"%-15.15s",tptr);
		sptr++;
		tptr = sptr;
	}

	if (!*sptr)
	{
		sprintf(envRec.envValue,"%-30.30s"," ");
		sprintf(envRec.envDesc,"%-70.70s"," ");
		return(0);
	}

	while (*sptr && *sptr != '\t')
		sptr++;

	if (*sptr)
	{
		*sptr = '\0';
		sprintf(envRec.envValue,"%-30.30s",tptr);
		sptr++;
		tptr = sptr;
	}

	if (!*sptr)
	{
		sprintf(envRec.envDesc,"%-70.70s"," ");
		return(0);
	}

	print_at (0,0,mlUtilsMess053,envRec.envName);

	sprintf(envRec.envDesc,"%-70.70s",tptr);

	return(1);
}
