/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( clip.c & lclip.c & GET_PNAME.c )                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (21/07/1994)    | Modified by : Jonathan Chen      |
|                : (03/09/1999)    | Modified by : Eumir Que Camara   |
|  Comments                                                           |
|  (21/07/1994)  : Removed GET_PNAME                                  |
|  (03/09/1999)  : Added ltrim().                                     | 
|                                                                     |
=====================================================================*/
#include	<string.h>

/*=====================================================================
| This function returns a pointer to the clipped (no trailing spaces) |
| version of the string you pass to it. It must be declared in the    |
| function which calls it i.e. 'char	*clip();'                     |
=====================================================================*/
char*
clip (
 char*	string)
{
	char	*sptr = (string + strlen(string) - 1);

	for (;sptr >= string && (*sptr == ' ' || *sptr == '\t');sptr--);
	*(sptr + 1) = '\0';
	return(string);
}

/*=====================================================================
| Function	    : lclip ()                                            |
| Description	: Remove leading spaces.                              |
| Parameters	: s - String from which spaces are to be removed.     |
| Returns		: s - Pointer to stripped string.                     |
=====================================================================*/
char*
lclip (
 char* s)
{
	register	char	*t;

	t = s;
	while (*t == ' ')
		t++;

	return (strcpy (s, t));
}


/*=====================================================================
| Function     : ltrim ()                                             |
| Description  : Remove leading spaces.                               |
| Parameters   : szSrc - string to trim.                              |
|              : szBuffer - memory to hold string w/o leading spaces. |
| Returns      : Pointer to trimmed string.                           |
=====================================================================*/
char*
ltrim (
 char*  szSrc,
 char*  szBuffer)
{
    char*   szStart = szSrc;
    if (szSrc == NULL ||
		szBuffer == NULL)
	{  /* unable to work */ 
       return "";
    }
  
	/* move to first non-space character */
    while (*szStart == ' ')
    {
       szStart++;
	}
	return (strcpy (szBuffer, szSrc));
}

/*======================================================================
| Function     : rtrim ()                                              |
| Description  : Remove trailing spaces.                               |
|                This function should be used instead of clip ()       |
|                if the passed parameter is a string constant.         |
| Parameters   : szSrc - string to trim.                               |
|              : szBuffer - memory to hold string w/o trailing spaces. |
| Returns      : Pointer to trimmed string.                            |
======================================================================*/
char *
rtrim (
 char *  szSrc,
 char *  szBuffer)
{
	char    *sptr;

	if (szSrc == NULL ||
		szBuffer == NULL)
	{
		return (NULL);
	}

	strcpy (szBuffer, szSrc);
	sptr = szBuffer + strlen (szBuffer) - 1;
	for ( ; sptr >= szBuffer && (*sptr == ' ' || *sptr == '\t'); sptr--)
		;
	*(sptr + 1) = '\0';
	return (szBuffer);
}

