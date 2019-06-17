/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : print_err.c                                    |
|  Source Desc       : Print formateed error message.                 |
|                                                                     |
|  Library Routines  : print_err()                                    |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     : 02/07/99   | Modified  by  : Trevor van Bremen |
|                                                                     |
|  Comments          :                                                |
|  (02/07/99)        : Updated to standardise on use of stdarg.h over |
|                    : the old varargs.h                              |
|                    :                                                |
| $Log: print_err.c,v $
| Revision 5.1  2001/08/06 22:40:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:37  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:17  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.3  1999/09/13 09:36:33  scott
| Updated for Copyright
|
|                                                                     |
=====================================================================*/
#include	<std_decs.h>

/*
.function
	Function	:	print_err ()

	Description	:	Print formateed error message.

	Notes		:	Print_err uses the mask (see printf) to format
				the arguments passed and display them on the 
				error line.
			
	Parameters	:	mask  - Format mask.
				ARGS  - Arguments to format and display.

	Returns		:	1
.end
*/
int
print_err (char *mask, ...)
{
	va_list	args;
	char	tmp_str [256];

	va_start (args, mask);

	vsprintf (tmp_str, mask, args);
	print_mess (tmp_str);
	putchar (7);
	fflush (stdout);
	sleep (2);

	va_end (args);

	return (EXIT_FAILURE);
}
