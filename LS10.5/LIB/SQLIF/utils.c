/*******************************************************************************

	Misc support utilities

*******************************************************************************/
#ifndef	lint
static char	*rscid = "$Header: /usr/LS10/REPOSITORY/LS10.5/LIB/SQLIF/utils.c,v 5.1 2001/08/09 09:31:24 scott Exp $";
#endif

#include	<malloc.h>
#include	<math.h>
#include	<sqltypes.h>
#include	<stdio.h>
#include	<string.h>
#include	"utils.h"
#include	"isamdbio.h"

char	*PadOut (
 char	*str,
 int	len)
{
	int	i = strlen (str);

	if (i < len)
	{
		while (i < len)
			str [i++] = ' ';
		str [i] = '\0';
	}
	return (str);
}

void	BuildParam (
 struct sqlvar_struct	*dst,
 struct sqlvar_struct	*src)
{
	dst -> sqltype	= src -> sqltype;
	dst -> sqllen	= src -> sqllen;
	dst -> sqldata	=
		dst -> sqltype == CCHARTYPE ?
		PadOut (src -> sqldata, src -> sqllen - 1) :
		src -> sqldata;
	dst -> sqlname	= src -> sqlname;
}

/*	Conversion from SQL type to C types,
	as used by Pinnacle apps
*/
XlatSQLC (
 int	sqlType)
{
	switch (sqlType)
	{
	case SQLCHAR	:
		return (CCHARTYPE);

	case SQLSMINT	:
		return (CINTTYPE);

	case SQLFLOAT	:
		return (CDOUBLETYPE);

	case SQLSMFLOAT	:
		return (CFLOATTYPE);

	case SQLDECIMAL	:
		return (CDECIMALTYPE);

	case SQLMONEY	:
		return (CDOUBLETYPE);

	case SQLINT	:
	case SQLSERIAL	:
		return (CLONGTYPE);

	case SQLDATE	:
		return (CDATETYPE);

	case SQLDTIME	:
		return (CDTIMETYPE);

	case SQLINTERVAL	:
		return (CINVTYPE);
	}
	return (-1);
}

/*******************************************************************************
 Data fudges and restores
*******************************************************************************/
/**
 Data fudges for stuff going into db
**/
void	DataFudgeIn (
 TableNode		*t,
 struct sqlda	*d)
{
	int	i, j;

	for (i = 0; i < d -> sqld; i++)
	{
		/*
		 *	For each element in sqlda structure, we have to determine
		 *	what the column type is in the database by searching the TableNode.
		 *	Simple offsets can't be used 'cos they may be out of step.
		 *	[eg : serial fields in UpdateRec ()]
		 */
		for (j = 0; j < t -> fldCount; j++)
			if (!strcmp (d -> sqlvar [i].sqlname, t -> fields [j].vwname))
				break;

		if (j >= t -> fldCount)
		{
			fprintf (stderr, "** Internal Error : DataFudgeIn ()\n");
			exit (1);
		}

		switch (t -> fields [j].vwtype & SQLTYPE)
		{
		case SQLMONEY	:
			*(double *) d -> sqlvar [i].sqldata /= 100.0;
			break;

		case SQLDATE	:
			if (!*(long *) d -> sqlvar [i].sqldata)
			{
				d -> sqlvar [i].sqltype = SQLNULL;
				*d -> sqlvar [i].sqlind = -1;
			}
			break;
		}
	}
}

/*
 Data fudges for stuff coming out of db :
	Handle NULL values
		default to 0, whatever.

	Handle misc fudges
		money fields from are set to (doubles * 100)
*/
void	DataFudgeOut (
 TableNode		*t,
 struct sqlda	*d)
{
	int	i;

	/* Handle NULLs
	*/
	for (i = 0; i < d -> sqld; i++)
	{
		int	colType = t -> fields [i].vwtype & SQLTYPE;

		if (*d -> sqlvar [i].sqlind == -1)
			switch (colType)
			{
			case SQLCHAR	:
				PadOut (d -> sqlvar [i].sqldata, d -> sqlvar [i].sqllen - 1);
				break;

			case SQLSMINT	:
				*(int *) d -> sqlvar [i].sqldata = 0;
				break;

			case SQLFLOAT	:
			case SQLMONEY	:
				*(double *) d -> sqlvar [i].sqldata = 0;
				break;

			case SQLSMFLOAT	:
				*(float *) d -> sqlvar [i].sqldata = 0;
				break;

			case SQLINT	:
			case SQLSERIAL	:
			case SQLDATE	:
				*(long *) d -> sqlvar [i].sqldata = 0;
				break;

			}

		/*
		 Handle misc fudges
		*/
		if (colType == SQLMONEY)
			*(double *) d -> sqlvar [i].sqldata =
				floor (*(double *) d -> sqlvar [i].sqldata * 100);
	}
}
