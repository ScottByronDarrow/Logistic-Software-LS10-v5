/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( gen_ser_no.c       )                             |
|  Program Desc  : ( Serial number generation routine.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (14/03/94)      | Author      : Campbell Mander    |
|---------------------------------------------------------------------|
|  Date Modified : (26.04.95)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|  (26.04.95)    : Rewrote to handle arbitrary length strings         |

	$Log: gen_ser_no.c,v $
	Revision 5.2  2002/07/09 02:29:56  scott
	S/C 004091 - An input of a serial number that has only 1 character integer, the system cannot allocate more than 9 serial numbers, it hangs.
	
	Revision 5.1  2001/08/06 22:40:54  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 06:59:16  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:36  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:20  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:14  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.3  1999/09/23 22:39:29  jonc
	Replaced use of deprecated <malloc.h> with <stdlib.h>
	
=====================================================================*/
#include	<std_decs.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>

#ifndef TRUE
#define	TRUE	1
#define	FALSE	0
#endif /* TRUE */

#define	MAXPARTS	20		/* Maximum parts to a serial number */

/*
 *	Local functions
 */
static int	bignumadd (char *str, int inc);

/*-------------------------------------------------------
| Generate next serial number by incrementing according |
| to the following rules :                              |
| * Serial numbers are incremented based on the mask of |
|   the first serial number passed.                     |
| * If the firstTime flag is TRUE then the mask info    |
|   will be extracted from the firstSerNo variable.     |
| * newSerNum will contain the new serial number at the |
|   completion of the routine.                          |
| * Only numeric components of firstSerNo will          |
|   increment.                                          |
| * Numeric components will increment to their maximum  |
|   value based on their original length. eg mask       |
|   passed is 12XY1.  The numeric component '1' will    |
|   increment to a maximum of '9'.                      |
| * Once a numeric component reaches its maximum value  |
|   it will be set to 0 and the next leftmost component |
|   will increment.  eg mask passed is 12XY8, the next  |
|   serial numbers generated are 12XY9, 13XY0, 13XY1 etc|
|                                                       |
| RETURNS :                                             |
|  0 - Completed successfully.                          |
|  1 - No numeric component in serial number.           |
|  2 - Maximum value for serial number reached.         |
-------------------------------------------------------*/
int
GenNextSerNo (
 char	*firstSerNo,
 int	firstTime,
 char	*newSerNum)
{
	int		i,
			carry;
	static int	numPart = 0;
	static struct
	{
		int		start,
				end;
	} serPart [MAXPARTS];

	/*----------------------------------------
	| Parse serial number for numeric parts. |
	----------------------------------------*/
	if (firstTime)
	{
		size_t	len = strlen (firstSerNo);

		/*
		 *	Work from back to front obtaining numeric sections
		 */
		numPart = 0;
		while (len)
		{
			/*
			 *	Skip non-numbers
			 */
			while (len && !isdigit (firstSerNo [len - 1]))
			{
				len--;
			}

			if (!len)
				break;

			serPart [numPart].end = len;
			/*
			 *	Work thru' the numbers
			 */
			while (len && isdigit (firstSerNo [len - 1]))
			{
				len--;
			}
			serPart [numPart++].start = len;
		}

		strcpy (newSerNum, firstSerNo);		/* initialise for increment */
	}

	/*----------------------------------------
	| No numeric component in serial number. |
	----------------------------------------*/
	if (!numPart)
		return (EXIT_FAILURE);

	carry = 1;			/* initial increment */
	for (i = 0; carry && i < numPart; i++)
	{
		/*
		 *	Use malloc for strings (I know it's an overhead, but
		 *	this makes it work for arbitrary length strings)
		 */
		size_t	len = serPart [i].end - serPart [i].start;
		char	*num = malloc (len + 1);

		/*
		 * Copy numeric section across
		 */
		memcpy (num, newSerNum + serPart [i].start, len);
		num [len] = '\0';

		carry = bignumadd (num, carry);

		/*
		 *	Copy it back
		 */
		memcpy (newSerNum + serPart [i].start, num, len);

		free (num);
	}
	/*
	 * Added this to prevent looping when numeric value input is two small 
	 * for number of serial items required.
	 */
	if (!strcmp (firstSerNo, newSerNum))
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

/*
 *	Function to handle arbitrary length numeric increments using
 *	strings (base 10 only)
 *
 *	Returns :
 *		overflow (ie amount 'carried')
 *
 */
static int
bignumadd (
 char	*number,			/* numeric string */
 int	add)				/* amount to add */
{
	int		i;
	size_t	len = strlen (number);

	if (!len)
		return (EXIT_SUCCESS);			/* nothing to do */

	/*
	 *	Get the first working digit from the end.
	 */
	for (i = len - 1; i && !isdigit (number [i]); i--);
	while (add && i >= 0 && isdigit (number [i]))
	{
		/*
		 *	The assumption is to use ASCII collation
		 */
		int	n = number [i] - '0';
		int	a = add + n;				/* addition result */
		int	r = a % 10;					/* get remainder */

		add = a / 10;					/* get carryover */
		number [i--] = '0' + r;			/* in place alteration */
	}
	return (add);						/* return carry overflow */
}
