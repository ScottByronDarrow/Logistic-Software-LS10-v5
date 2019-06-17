/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( comma_fmt.c                    )                 |
|  Program Desc  : ( Format numeric values to a mask.               ) |
|---------------------------------------------------------------------|
|  Date Written  : 07/04/92        | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (15/12/92)      | Modified  by : Campbell Mander   |
|                                                                     |
|  Comments      : (15/12/92) - Force trailing nulls into result str  |
|                : to fix problem caused by calling comma_fmt with    |
|                : successively smaller masks.                        |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*======================================================================
| This function returns a pointer to a static string which has been    |
| formatted with commas ',' based on the passed mask.                  |
======================================================================*/
char	*comma_fmt(double value, char *mask)
{
	static	char	result[64];
	char	tmp_str[64],
		*sptr,
		*strrchr (const char *, int);
	int	i,
		dec_dgts = 0,
		no_commas = 0,
		ptr,
		len;

	for (ptr = 0; mask[ptr] != 0; ptr++)
		if (mask[ptr] == ',')
			no_commas++;
	len = strlen (mask);
	sptr = strrchr (mask, '.');
	if (sptr != (char *) 0)
		dec_dgts = (len - (sptr - mask)) - 1;
	sprintf (tmp_str, "%*.*f", len - no_commas, dec_dgts, value);
	for (i = ptr = 0; ptr < len; ptr++)
	{
		if (mask[ptr] == ',')
		{
			if (result[ptr - 1] < '0' || result[ptr - 1] > '9')
				result[ptr] = ' ';
			else
				result[ptr] = ',';
		}
		else
			result[ptr] = tmp_str[i++];
		result[ptr + 1] = '\0';
	}
	return (result);
}
