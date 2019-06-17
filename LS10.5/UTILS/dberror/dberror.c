char	*PNAME = "$RCSfile: dberror.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/dberror/dberror.c,v 5.1 2001/08/09 09:26:48 scott Exp $";

#include	<pslscr.h>
#include	<stdio.h>
#include 	<dbtext.h>	/* database error text/message stucture */

#define  VDATE	"28/02/90"	/* date of last revision of error codes */

/*===========================
| Local function prototypes |
===========================*/
int		printerr	(int ErrorNo);
void	printall	(void);


int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	if (argc <= 1)
	{
		printall();
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[1],"-v") == (char) NULL)
	{
		printf("dberror: Date of last revision of codes was :%s\n",VDATE);
		return (EXIT_FAILURE);
	}

	if (strcmp(argv[1],"-a") == (char) NULL)
	{
		printall();
		return (EXIT_FAILURE);
	}
	else
	{
		for (i = 1;i < argc;i++)
			printerr (atoi(argv[i]));
	}

	return (EXIT_SUCCESS);
}

int
printerr (
 int ErrorNo)
{
	int	p = 0;
	int	pos = 0;
	int	found = 0;

	if (ErrorNo == 0)
	{
		printf("Error code of 0 is : command executed successfully\n");
		return(0);
	}

	for (p = 0;error_codes[p].code != 0 && error_codes[p].code <= ErrorNo;p++)
	{
		if (error_codes[p].code == ErrorNo)
		{
			found = 1;
			pos = p;
			break;
		}
	}

	if (found != 0)
		printf("Error code of %d is : %s\n",error_codes[pos].code,error_codes[pos].msg);
	else
		printf("error code of %d not found.\n",ErrorNo);

	return(0);
}

void
printall (
 void)
{
	int	cnt = 0;
	int	i;

	printf("Code   :     Message     (Last updated on %s) \n",VDATE);
	printf("=========================================================================\n");

	for (i = 0;error_codes[i].code != 0;i++)
	{
		printf("| %5d  : %-60.60s |\n",error_codes[i].code,error_codes[i].msg);
		cnt++;
	}

	printf("=========================================================================\n");
	printf("\n %d error codes printed\n",cnt);
}
