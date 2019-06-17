/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( shift.c        )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
| Date Written  : 10/05/86         |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (26/08/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|     (26/08/93) : Made upshift () & downshift () return argument     |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<ctype.h>

char	*upshift (char *string)
{
	char	*sptr = string;

	while (*sptr)
	{
		*sptr = toupper (*sptr);
		sptr++;
	}
	return (string);
}

char	*downshift (char *string)
{
	char	*sptr = string;

	while (*sptr)
	{
		*sptr = tolower(*sptr);
		sptr++;
	}
	return (string);
}
