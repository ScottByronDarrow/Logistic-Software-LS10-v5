/*=====================================================================
|  Copyright (C) 2000 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: number.c,v 5.1 2001/11/16 02:23:04 cha Exp $   
|=====================================================================|
|  Program Name  : (number.c)
|  Program Desc  : (Implementation of semi-arbitrary numbers)
|=====================================================================|
|  $Log: number.c,v $
|  Revision 5.1  2001/11/16 02:23:04  cha
|  Updated to add our own implementation of Informix's 32 bit
|  numbering system. This is quite usefull if the database is other
|  that Informix.
|
|  Revision 5.0  2001/06/19 06:59:35  cha
|  LS10-5.0 New Release as of 19 JUNE 2001
|
|  Revision 4.0  2001/03/09 00:52:38  scott
|  LS10-4.0 New Release as at 10th March 2001
|
|  Revision 3.0  2000/10/12 13:34:24  gerry
|  Revision No. 3 Start
|  <after Rel-10102000>
|
|  Revision 2.0  2000/07/15 07:17:17  gerry
|  Forced revision no. to 2.0 - Rel-15072000
|
|  Revision 1.3  1999/10/08 03:41:06  jonc
|  Added log and id
|
|====================================================================*/

#include	<std_decs.h>

#ifdef ORA734
	#include	<string.h>
	#include	<decimalLS.h>
	#include 	<dec.h>
#endif


#ifdef ORA734
	#define DECCVASC 	_deccvasc
	#define DECCVINT 	_deccvint
	#define DECCVLONG 	_deccvlong
	#define DECCVFLT	_deccvflt
	#define DECCVDBL	_deccvdbl
	#define DECTOASC	_dectoasc
	#define DECTOINT	_dectoint
	#define DECTOLONG	_dectolong
	#define DECTOFLT	_dectoflt
	#define DECTODBL	_dectodbl
	#define DECADD		_decadd
	#define DECSUB		_decsub
	#define DECMUL		_decmul
	#define	DECDIV		_decdiv
	#define	DECCMP		_deccmp
	#define	DECCOPY		_deccopy
	#define DEC_T		_dec_t
#else
	#define DECCVASC 	deccvasc
	#define DECCVINT 	deccvint
	#define DECCVLONG 	deccvlong
	#define DECCVFLT	deccvflt
	#define DECCVDBL	deccvdbl
	#define DECTOASC	dectoasc
	#define DECTOINT	dectoint
	#define DECTOLONG	dectolong
	#define DECTOFLT	dectoflt
	#define DECTODBL	dectodbl
	#define DECADD		decadd
	#define DECSUB		decsub
	#define DECMUL		decmul
	#define	DECDIV		decdiv
	#define	DECCMP		deccmp
	#define	DECCOPY		deccopy
	#define DEC_T		dec_t
#endif


/* Convert to Number from : */
void	NumAsc (
 number	*n,
 char	*c)
{
	DECCVASC (c, strlen (c), (DEC_T *) n);
}

void	NumShort (
 number	*n,
 short	v)
{
	DECCVINT ((int) v, (DEC_T *) n);
}

void	NumInt (
 number	*n,
 int	v)
{
	DECCVINT (v, (DEC_T *) n);
}

void	NumLong (
 number	*n,
 long	v)
{
	DECCVLONG (v, (DEC_T *) n);
}

void	NumFlt (
 number	*n,
 float	v)
{
	DECCVFLT (v, (DEC_T *) n);
}

void	NumDbl (
 number	*n,
 double	v)
{
	DECCVDBL (v, (DEC_T *) n);
}

/*
 * Get value from Number
 */
char	*NumToAsc (
 number	*n,
 char	*c)
{
	DECTOASC ((DEC_T *) n, c, 40, -1);
	return (c);
}

short	NumToShort (
 number	*n)
{
	return ((short) NumToInt (n));
}

int		NumToInt (
 number	*n)
{
	int	v;

	DECTOINT ((DEC_T *) n, &v);
	return (v);
}

long	NumToLong (
 number	*n)
{
	long	v;

	DECTOLONG ((DEC_T *) n, &v);
	return (v);
}

float	NumToFlt (
 number	*n)
{
	float	v;

	DECTOFLT ((DEC_T *) n, &v);
	return (v);
}

double	NumToDbl (
 number	*n)
{
	double	v;

	DECTODBL ((DEC_T *) n, &v);
	return (v);
}

/* Operations with numbers */
void	NumAdd (
 number	*n1,
 number	*n2,
 number	*result)
{
	DECADD ((DEC_T *) n1, (DEC_T *) n2, (DEC_T *) result);
}

void	NumSub (
 number	*n1,
 number	*n2,
 number	*result)
{
	DECSUB ((DEC_T *) n1, (DEC_T *) n2, (DEC_T *) result);
}

void	NumMul (
 number	*n1,
 number	*n2,
 number	*result)
{
	DECMUL ((DEC_T *) n1, (DEC_T *) n2, (DEC_T *) result);
}

void	NumDiv (
 number	*n1,
 number	*n2,
 number	*result)
{
	DECDIV ((DEC_T *) n1, (DEC_T *) n2, (DEC_T *) result);
}

/*
 * Misc ops
 */
int	NumCmp (
 number	*n1,
 number	*n2)
{
	return (DECCMP ((DEC_T *) n1, (DEC_T *) n2));
}

void	NumCpy (
 number	*n1,
 number	*n2)
{
	DECCOPY ((DEC_T *) n2, (DEC_T *) n1);
}

