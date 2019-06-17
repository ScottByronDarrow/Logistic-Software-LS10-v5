/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( prmptmsg.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

extern int	_mail_ok;

/*====================================================================
| A routine which will print the message 'text' at position          |
| x,y and only return when a character from 'allowed' is entered.    |
| Assumes that tty mode is cbreak or raw with echo turned off -      |
| normal case for most displays.                                     |
====================================================================*/
int
prmptmsg(char *text, char *allowed, int x, int y)
{
	int	notok,
		tc = 0,
		c = 0;

	_mail_ok = 0;

	do
	{
		print_at(y,x,"%R %s",text);
		printf("  _\b");
		fflush(stdout);
		
		notok = strlen(allowed);
		while (notok) 
		{
			tc = getkey();
			if (strchr(allowed,(char) tc) != 0)
				notok = 0;
		}
		putchar( tc );
		while((c = getkey()) != '\r' && c != '\b');
	} while (c != '\r');

	_mail_ok = 1;

	return(tc);
}
