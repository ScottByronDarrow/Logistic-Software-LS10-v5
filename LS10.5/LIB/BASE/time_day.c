/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( time_day.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|	$Log: time_day.c,v $
|	Revision 5.0  2001/06/19 06:59:40  cha
|	LS10-5.0 New Release as of 19 JUNE 2001
|	
|	Revision 4.0  2001/03/09 00:52:40  scott
|	LS10-4.0 New Release as at 10th March 2001
|	
|	Revision 3.0  2000/10/12 13:34:30  gerry
|	Revision No. 3 Start
|	<after Rel-10102000>
|	
|	Revision 2.0  2000/07/15 07:17:20  gerry
|	Forced revision no. to 2.0 - Rel-15072000
|	
|	Revision 1.5  1999/09/13 09:36:35  scott
|	Updated for Copyright
|	
|	Revision 1.4  1999/09/13 06:20:50  alvin
|	Check-in all ANSI modifications made by Trev.
|	
|	Revision 1.3  1999/07/15 22:28:05  jonc
|	Fixed stupid spelling mistake.
|	
|	Revision 1.2  1999/07/15 22:24:07  jonc
|	Fixed time_t declarations from Digital Alpha.
|	Fixed character buffer usage for ANSI compliance.
|	Fixed function declarations to use std header files instead of in-file decls.
|	
=====================================================================*/
#include	<std_decs.h>

static	char	cur_dte [20];

char *ttod(void)
{
	char	*sptr;
	time_t	systime = time (NULL);
	struct	tm	*tme;

	int		YearLength = 2;

	sptr = getenv ("DBDATE");
	if (sptr)
		upshift (sptr);

	/*--------------------------------
	| Check for four character date. |
	--------------------------------*/
	YearLength = 2;
	if ((strchr (sptr, '4') != (char *) 0))
		YearLength = 4;

	tme = localtime(&systime);
	sprintf
	(
		cur_dte, "%02d/%02d/%*d %02d:%02d:%02d",
		tme->tm_mday,
		tme->tm_mon + 1,
		YearLength,
		(YearLength == 2) ? tme->tm_year : tme->tm_year + 1900,
		tme->tm_hour,
		tme->tm_min,
		tme->tm_sec
	);

	if (sptr && (!strncmp (sptr, "YMD", 3) || 
				 !strncmp (sptr, "Y2MD", 4) ||
				 !strncmp (sptr, "Y4MD", 4)))
	{
		sprintf
		(
			cur_dte, "%*d/%02d/%02d %02d:%02d:%02d",
				YearLength,
				(YearLength == 2) ? tme->tm_year : tme->tm_year + 1900,
				tme->tm_mon + 1,
				tme->tm_mday,
				tme->tm_hour,
				tme->tm_min,
				tme->tm_sec
		);
	}

	if (sptr && !strncmp (sptr, "MDY", 3))
		sprintf
		(
			cur_dte, "%02d/%02d/%*d %02d:%02d:%02d",
			tme->tm_mon + 1,
			tme->tm_mday,
			YearLength,
			(YearLength == 2) ? tme->tm_year : tme->tm_year + 1900,
			tme->tm_hour,
			tme->tm_min,
			tme->tm_sec
		);

	return(cur_dte);
}

/*-------------------------------
| Convert a time string (HH:MM)	|
| into a long in minutes.	|
-------------------------------*/
long	atot (char *time_str)
{
	char	*sptr;
	long	tm_val;

	tm_val = atol (time_str);
	tm_val *= 60L;
	sptr = strchr (time_str, ':');
	if (sptr)
		tm_val += atol (sptr + 1);
	return (tm_val);
}

/*-------------------------------
| Convert a long in minutes	|
| into a time string (HH:MM)	|
-------------------------------*/
char	*ttoa (long int tm_val, char *mask)
{
	static	char	time_str[32];
	int	len;

	len = strlen (mask);
	sprintf (time_str, "%*ld:%02ld", len - 3, tm_val / 60L, tm_val % 60L);
	return (time_str);
}

/*--------------------------------------
| Round a TIMETYPE time to the nearest |
| multiple of the specified value.     |
--------------------------------------*/
long	rnd_time(long int raw_time, long int mult_of)
{
	long	rtrn_time;
	long	rnd_min;
	long	mid_time;

	if (raw_time % mult_of == 0L)
		return(raw_time);

	/*-----------------------------
	| Calculate rounding boundary |
	-----------------------------*/
	mid_time = (mult_of / 2L) + 1L;
	
	/*------------------------------
	| Calculate amount of rounding |
	------------------------------*/
	if (raw_time % mult_of < mid_time)
		rnd_min = ( raw_time - (mult_of * (raw_time / mult_of)) );
	else
		rnd_min = ( raw_time - (mult_of * (raw_time / mult_of + 1L)) );

	/*------------
	| Round time |
	------------*/
	rtrn_time = raw_time - rnd_min;

	return(rtrn_time);
}
