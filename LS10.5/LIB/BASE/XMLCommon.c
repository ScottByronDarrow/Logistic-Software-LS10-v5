/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLCommon.c)                                     |
|  Program Desc  : ( Functions used by XMLReader and XMLWriter      ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Updates files :                                                    |
|---------------------------------------------------------------------|
|  Date Written  : 10/22/2002     | Author      : Robert A. Mejia     |
|---------------------------------------------------------------------|
|  Date Modified : (          )    | Modified by :                    |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|  (          )  :                                                    |
|                                                                     |
| $Id: XMLCommon.c,v 5.3 2002/10/22 07:05:40 robert Exp $   
| $Log: XMLCommon.c,v $
| Revision 5.3  2002/10/22 07:05:40  robert
| Added missing '}' at end of file (occurred during convert to UNIX files.)
|
| Revision 5.2  2002/10/22 06:57:36  robert
| Initial checked-in files are Win32 files. Converted to UNIX files
|
| Revision 5.1  2002/10/22 06:19:53  robert
| Initial check-in. XML functions
|  
|                                                                     |
=====================================================================*/

#include <XMLCommon.h>
#include  <ctype.h>

int istrcmp (char *s1, char *s2)
{
	static char localS1 [MAXTEXT_LEN];
	static char localS2 [MAXTEXT_LEN];


	strcpy (localS1, s1);
	strcpy (localS2, s2);

	strtoupper (localS1);
	strtoupper (localS2);

	return (strcmp (localS1, localS2));
}

char *strtoupper (char *s)
{
	while (*s != '\0')
	{
		*s = toupper (*s);
		s++;
	}

	return (s);
}

char *XML_escape (char *str)
{
	static char buffer [MAXTEXT_LEN];
	
	strcpy (buffer, str);
	
    strcpy (buffer, XML_replaceString (buffer,"&","&amp;"));
    strcpy (buffer, XML_replaceString (buffer,"<","&lt;"));
    strcpy (buffer, XML_replaceString (buffer,">","&gt;"));
    strcpy (buffer, XML_replaceString (buffer,"\"","&quot;"));
    strcpy (buffer, XML_replaceString (buffer,"'","&apos;"));

    return (buffer);
}

char *XML_xltescape (char *str)
{
	static char buffer [MAXTEXT_LEN];
	
	strcpy (buffer, str);
	
    strcpy (buffer, XML_replaceString (buffer, "&amp;", "&"));
    strcpy (buffer, XML_replaceString (buffer, "&lt;", "<"));
    strcpy (buffer, XML_replaceString (buffer, "&gt;", ">"));
    strcpy (buffer, XML_replaceString (buffer, "&quot;", "\""));
    strcpy (buffer, XML_replaceString (buffer, "&apos;", "'"));

    return (buffer);
}

char *XML_replaceString (char *src, char *s1, char *s2)
{
	static char buffer [MAXTEXT_LEN];
	char *pos;
	size_t lenS2, lenS1, i, offset;

	if (!src)
		return (src);

	lenS1 = strlen (s1);
	if (!lenS1)
		return (src);

	lenS2 = strlen (s2);

	pos = src;
	
	strcpy (buffer, "");
	offset = 0;
	
	while (*pos != '\0')
	{
		if (!strstr (pos, s1))
		{
			strcat (buffer, pos);
			break;
		}

		if (!strncmp (pos, s1, lenS1))
		{
			for (i=0;i < lenS2;i++)
				buffer [offset++] = s2 [i];
			buffer [offset] = '\0';

			pos += lenS1;
			continue;
		}

		buffer [offset++] = *pos;
		buffer [offset] = '\0';
		
		pos++;
	}

	return (buffer);
}
