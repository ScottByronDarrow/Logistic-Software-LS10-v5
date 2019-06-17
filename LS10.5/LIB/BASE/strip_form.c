/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : strip_form.c                                   |
|  Source Desc       : Strip format characters out of an account code.|
|                                                                     |
|  Library Routines  : strip_form()                                   |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|---------------------------------------------------------------------|
|  Date Modified     :   /  /     | Modified  by  :                   |
|                                                                     |
|  Comments          :                                                |
|                    :                                                |
|                    :                                                |
|                    :                                                |
|                                                                     |
=====================================================================*/
/*
.function
	Function	:	strip_form ()

	Description	:	Strip format characters out of an account code.

	Parameters	:	s1 - Pointer to string to hold stripped code.
				s2 - Pointer to string holding unstripped code.
.end
*/
void
strip_form (char *s1, char *s2)
{
	while (*s2)
	{
		while (*s2 == '-')
			s2++;

		*s1++ = *s2++;
	}
	*s1 = '\0';
}
