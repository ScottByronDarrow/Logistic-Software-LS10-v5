/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( lc_i_no.c      )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (01/11/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|     (01/11/93) : Fixed character array overflow. Also made it more  |
|                : understandable - without compromising secrecy      |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define		BASE		1962		/*	Random number */

/*	Signficant char count */
#define	SIGC_MENUSYS	7
#define	SIGC_UTILS		5
#define	SIGC_MENU		4
#define	SIGC_LICENSE	7

#define	BRAND_PERIOD	8

static int	brand_code[] = {

	'M' + BASE, 'E' + BASE, 'N' + BASE, 'U' + BASE, 'S' + BASE,
	'Y' + BASE, 'S' + BASE, '[' + BASE,

	'U' + BASE, 'T' + BASE, 'I' + BASE, 'L' + BASE, 'S' + BASE,
	'i' + BASE, '8' + BASE, 'n' + BASE,

	'M' + BASE, 'E' + BASE, 'N' + BASE, 'U' + BASE, 'G' + BASE,
	'o' + BASE, 'S' + BASE, 'h' + BASE,

	'L' + BASE, 'I' + BASE, 'C' + BASE, 'E' + BASE, 'N' + BASE,
	'S' + BASE, 'E' + BASE, 'y' + BASE, 0
};

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

extern char	*getenv (const char *);		/*stdlib*/

/*================================
| Get i-node numbers from files. |
================================*/
long	lc_i_no(void)
{
	int		i;
	char	*sptr = getenv ("PROG_PATH");
	long	i_number  = 0L;

	if (!sptr)
		sptr = "/usr/LS10.5";

	for (i = 0; i < 4; i++)
	{
		int		j, sigc;
		char	conv [BRAND_PERIOD];
		char	file_in [60];
		struct stat	buf;

		switch (i)
		{
		case 0	:	sigc = SIGC_MENUSYS;	break;
		case 1	:	sigc = SIGC_UTILS;		break;
		case 2	:	sigc = SIGC_MENU;		break;
		case 3	:	sigc = SIGC_LICENSE;	break;
		default	:	return (-1);
		}

		for (j = 0; j < sigc; j++)
			conv [j] = (char) (brand_code [i * BRAND_PERIOD + j] - BASE);
		conv [j] = '\0';

		sprintf (file_in,"%s/BIN/%s", sptr, conv);

		if (stat (file_in, &buf))
			return (-1);

		i_number += buf.st_ino;
	}
	return (i_number);
}
