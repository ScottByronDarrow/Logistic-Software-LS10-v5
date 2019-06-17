/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Author            : Scott Darrow.   | Date Written  : 21/01/91     |
|  Source Name       : str_token.c                                    |
|  Source Desc       : String tokenising function.                    |
|                                                                     |
|  Library Routines  : str_token()                                    |
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
#include <stdio.h>

/*
.function
	Function	:	str_token ()

	Description	:	String tokenising function.
				
	Notes		:	Str_token is functionally similar to the
				strtok(S) call, the only difference being that
				str_token treats quote delimited tokens as being
				one piece.
				
	Parameters	:	s1	-	Pointer to string to tokenise.
				s2	-	Pointer to token delimiter(s).

	Returns		:	ret_ptr	-	Pointer to current token.
				NULL	-	If no more tokens.
.end
*/
char	*str_token (char *s1, char *s2)
{
	char *ret_ptr, *strchr (const char *, int), *brk_chrs;
	static char *b_ptr, qtes [] = "\'\"";

	if (s1)
		b_ptr = s1;

	while (*b_ptr && strchr (s2, *b_ptr))
		b_ptr++;
		
	if (*b_ptr && strchr (qtes, *b_ptr))
	{
		brk_chrs = qtes;
		b_ptr++;
	}
	else
		brk_chrs = s2;
	
	ret_ptr = b_ptr;

	while (*b_ptr)
	{
		if (strchr (brk_chrs, *b_ptr))
		{
			*b_ptr++ = (char) NULL;
			return ((*ret_ptr) ? ret_ptr : NULL);
		}

		b_ptr++;
	}

	return ((*ret_ptr) ? ret_ptr : NULL);
}
