char	*PNAME = "$RCSfile: chk_secure.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/chk_secure/chk_secure.c,v 5.1 2001/08/09 09:26:47 scott Exp $";

#define		CCMAIN

#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<stdio.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	co_no[3];
	char	br_no[3];

	if (argc != 3)
	{
		/*printf("Usage : %s <co_no> <br_no>\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlUtilsMess708),argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf(co_no,"%2.2s",argv[1]);
	sprintf(br_no,"%2.2s",argv[2]);

	print_at(0, 0, "%s(",argv[0]);
	print_at(1,0,"%s,",co_no);
	print_at(2,0,"%s) : ",br_no);
	/*print_at(3,0,"Access %s\n",chk_secure(co_no,br_no) ? "Permitted" : "Denied");*/
	if (chk_secure(co_no, br_no)) 
		print_at(3,0,ML(mlUtilsMess037));
	else
		print_at(3,0,ML(mlUtilsMess038));

	return (EXIT_SUCCESS);
}
