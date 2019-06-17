/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: SleepDelay.c,v 5.0 2001/06/19 06:59:13 cha Exp $
|---------------------------------------------------------------------|
|  Date Written  : 22nd March 2001 |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: SleepDelay.c,v $
| Revision 5.0  2001/06/19 06:59:13  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/22 01:20:43  scott
| Updated to create simple sleep functions.
|
=====================================================================*/
#include	<std_decs.h>

static	int		messageDelay	=	0,
				errorDelay		=	0;

int		ErrorDelay 		(void),
		MessageDelay 	(void),
		SleepDelay 		(void);

/*==================================================================
| Small functions to get message and error delays - User specific. |
==================================================================*/
int
MessageDelay (void)
{
	char	*sptr;
	if (!messageDelay)
	{
		sptr			=	getenv ("LSL_MSG_DELAY");
		messageDelay	=	(sptr == (char *)0) ? 2 : atoi (sptr);
	}
	return (messageDelay);
}
/*==================================================================
| Small functions to get message and error delays - User specific. |
==================================================================*/
int
ErrorDelay (void)
{
	char	*sptr;
	if (!errorDelay)
	{
		sptr		=	getenv ("LSL_ERR_DELAY");
		errorDelay 	=	(sptr == (char *)0) ? 2 : atoi (sptr);
	}
	return (errorDelay);
}

/*==================================================================
| Small functions to get message and error delays - User specific. |
==================================================================*/
int
SleepDelay (void)
{
	return (MessageDelay ());
}
