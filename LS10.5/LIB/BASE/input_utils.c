/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( input_utils.c  )                                 |
|  Program Desc  : ( Input Program Utilities.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (19/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/09/96)      | Modified  by : Scott B Darrow.   |
|                                                                     |
|  Comments                                                           |
|  (19/02/92)    : Update to internationalize (ie: Look at DBDATE)    |
|  (30/09/96)    : Updated for 4 character Date.                      |
|                                                                     |
=====================================================================*/
#include	<stdio.h>
#include	<strings.h>
#include	<std_decs.h>

#define MAXTEXT 132

char _gtext[MAXTEXT];

char *mid(char *s, int n, int l)
{
	if ((n + l) > (int) strlen (s) + 1)
		l = strlen (s) - n;

	if (l < 0)
		l = 0;

	strncpy (_gtext, s + n - 1, l);
	_gtext[l] = '\0';

	return (_gtext);
}

char *string(int nbr, char *chr)
{
	int	loop = 0;

	for (loop = 0; loop < nbr; loop++)
		_gtext[loop] = *chr;

	_gtext[nbr] = '\0';

	return (_gtext);
}
