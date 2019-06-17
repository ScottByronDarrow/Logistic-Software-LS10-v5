/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( AgePeriod.c      )                               |
|  Program Desc  : ( Calculated ageing period.                      ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 05/04/96          |
|---------------------------------------------------------------------|
|  Date Modified : (30/10/1998)    | Modified  by : SD + CM           |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
|  Comments                                                           |
|  (30/10/1998)  :  Updated for ageing related to current and forward.|
-----------------------------------------------------------------------
	$Log: age_per.c,v $
	Revision 5.0  2001/06/19 06:59:14  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:52:35  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:34:18  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:17:12  gerry
	Forced revision no. to 2.0 - Rel-15072000
	
	Revision 1.11  1999/11/26 01:01:19  jonc
	AgePeriod() extended to include true_age parameter.
	
	Revision 1.10  1999/11/25 03:47:05  jonc
	Updated CalcDueDate to use std date routines, tightened prototypes.
	
=====================================================================*/
#include	<std_decs.h>
#include	<math.h>

static int envDbZeroAge		= FALSE,
		   dbZeroAgeRead	= FALSE;

/*==========================
| Calculate ageing period. |
==========================*/
int	
AgePeriod (
 char * PayTerms,
 Date InvoiceDate,
 Date CurrentDate,
 Date DueDate,
 int AgeDays,
 int true_age)
{
	int		mth_term = 0,
			period = 0;

	char	*sptr;

	if (dbZeroAgeRead == FALSE)
	{
		/*------------------------------------
		| Check special zero due date.       |
		------------------------------------*/
		sptr = chk_env ("DB_ZERO_AGEING");
		envDbZeroAge	= (sptr == (char *)0) ? 0 : atoi (sptr);

		dbZeroAgeRead	=	TRUE;
	}
	if (envDbZeroAge && AgeDays)
		DueDate	=	InvoiceDate;

	/*-----------------------------------------------------------
	| Ageing is by Aged days as definded in SMFI specification. |
	-----------------------------------------------------------*/
	if (AgeDays)
	{
		long	CheckDate;
		long	DaysOverdue = DueDate - CurrentDate;

		if ( DaysOverdue >= 0 )
		{
			CheckDate = DueDate - AgeDays;
			
			return( (CurrentDate > CheckDate ) ? 0 : -1 );
		}

		period = (-DaysOverdue / AgeDays) + 1;
		return period > 4 ? 4 : period;
	}
	if (envDbZeroAge)
		CurrentDate += (DueDate - InvoiceDate);

	if (!strcmp (PayTerms, "   "))
		strcpy (PayTerms,"20A");

	if (PayTerms [2] >= 'A')
	{
		/*
		 *	Days + month
		 */
		int	cd, cm, cy,
			id, im, iy;

		if (!true_age)
		{
			/*
			 *	Set current date to last day of month
			 */
			CurrentDate = MonthEnd (CurrentDate);
		}

		DateToDMY (CurrentDate, &cd, &cm, &cy);
		DateToDMY (InvoiceDate, NULL, &im, &iy);

		id = atoi (PayTerms);
		im += PayTerms [2] - 'A' + 1;
		if (im > 12)
		{
			im -= 12;
			iy++;
		}

		/*
		 *	Work out number of months difference
		 */
		mth_term = (cy - iy) * 12;
		mth_term += cm - im;
		mth_term += (cd > id) ? 1 : 0;
		mth_term = (mth_term < -1 ) ? -1 : mth_term;

		return ((mth_term < 5) ? mth_term : 4);
	}
	mth_term = atoi (PayTerms);
	if (mth_term <= 0)
		mth_term = 1;

	if (CurrentDate >= InvoiceDate)
	{
		int	days_diff = CurrentDate - InvoiceDate;
		int	period = days_diff < mth_term ? 0 : days_diff / mth_term;

		return period > 4 ? 4 : period;
	}

	return -1;		/* forward dated */
}
/*===============================================
| Determine Payment Date From Invoice Date.     |
===============================================*/
Date
CalcDueDate (
 const char * PayTerms,
 Date InvoiceDate)
{
	Date	dueDate;
	int		days = 0;			/* Payment days as an integer			*/

	static int env_checked = FALSE,
			   DbMendAge = 0;	/* Month offset for payment          	*/

	if (!env_checked)
	{
		/*
		 * Check if ageing is to be performed using month end ageing.
		 */
		char * sptr = chk_env ("DB_MEND_AGE");
		DbMendAge = sptr ? atoi (sptr) : 0;

		env_checked = TRUE;
	}

	days = atoi (PayTerms);

	if (PayTerms [2] >= 'A')
	{
		int d, m, y;
		int months_more = 0;

		/*
		 *	Work out grace-months for format nnA to nnF input
		 *	and add them to the invoice-date for prelim due-date
		 */
		months_more = PayTerms [2] - 'A' + 1;
		dueDate = AddMonths (InvoiceDate, months_more);

		/*
		 *	Set the due date to payment-term's due-date if possible,
		 *	otherwise set it to end of month
		 */
		DateToDMY (dueDate, &d, &m, &y);
		if (days > DaysInMonth (dueDate))
			dueDate = MonthEnd (dueDate);
		else
			dueDate = DMYToDate (days, m, y);

		if (DbMendAge)
		{
			/*
			 * If current day > cut-off day then increase month by one
			 */
			if (d > days)
				dueDate = AddMonths (dueDate, 1);

			/*
			 * Set payment date to last day of month
			 * (please put reason here if you know why)
			 */
			dueDate = MonthEnd (dueDate);
		}

	} else
		dueDate = InvoiceDate + days;

	return dueDate;
}
