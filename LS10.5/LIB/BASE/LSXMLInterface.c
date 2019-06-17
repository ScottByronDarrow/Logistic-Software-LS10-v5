/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( LSXMLInterface.c )                               |
|  Program Desc  : ( Contains LS/10 specific xml interface functions) |
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
| $Id: LSXMLInterface.c,v 5.4 2002/10/23 03:28:57 robert Exp $   
| $Log: LSXMLInterface.c,v $
| Revision 5.4  2002/10/23 03:28:57  robert
| Updated to add variable ALLOW_EMPTY_STRING to allow empty entity having
| empty text. Make XML_LOGLEVEL as int variable instead of #define.
|
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

#include <LSXMLInterface.h>

// global variables
int ALLOW_EMPTY_STRING = 1;

/* GetTextValue functions
	Description:
		Extract the value indicated by the specified label
	Parameters:
		parent - the parent where the label will be searched.
		label - the label name of the node where we want to 
				extract the text value.
	Return value:
		the text value of the label converted into specific types
*/
char 	*GetTextValue_char (struct ElementPart *parent,  char *label) 
{
	struct ElementPart *element;
	static char *emptyString = "";

	if (!parent)
		return (emptyString);

	element = parent->firstChild;
	
	while (element)
	{
		if (!istrcmp (element->label_name, label))
			return (GetLabelText (element));

		element = element->sibling;
	}

	LogXMLMessage (2, "[WARNING]. text element <%s> doesn't exist under parent <%s>", 
		label,
		parent->label_name);
	return (emptyString);
}

float 	GetTextValue_float (struct ElementPart *parent,  char *label) 
{
	return ((float) atof (GetTextValue_char (parent, label)));
}

double 	GetTextValue_double (struct ElementPart *parent,  char *label) 
{ 
	return (atof (GetTextValue_char (parent, label)));
}

double 	GetTextValue_money (struct ElementPart *parent,  char *label) 
{ 
	double dValue;

	dValue = atof (GetTextValue_char (parent, label)) * 100;
	
#ifdef WIN32
	return (dValue);
#else
	return (no_dec (dValue));
#endif
}

long 	GetTextValue_date (struct ElementPart *parent,  char *label) 
{ 
#ifdef WIN32
	return (0L);
#else
	return (StringToDate (GetTextValue_char (parent, label)));
#endif
}

long 	GetTextValue_long (struct ElementPart *parent,  char *label) 
{ 
	return (atol (GetTextValue_char (parent, label)));
}

int 	GetTextValue_int (struct ElementPart *parent,  char *label) 
{ 
	return (atoi (GetTextValue_char (parent, label)));
}

float	GetTextValue_float_p (struct ElementPart *parent,  char *label, int precision)
{
	float fValue;

	fValue = GetTextValue_float (parent, label);

#ifdef WIN32
	return (fValue);
#else
	return ((float) n_dec (fValue, precision));
#endif
}

double	GetTextValue_double_p (struct ElementPart *parent,  char *label, int precision)
{
	double dValue;

	dValue = GetTextValue_double (parent, label);

#ifdef WIN32
	return (dValue);
#else
	return (n_dec (dValue, precision));
#endif
}

double	GetTextValue_money_p (struct ElementPart *parent,  char *label, int precision)
{
	double dValue;

	dValue = GetTextValue_money (parent, label);

#ifdef WIN32
	return (dValue);
#else
	return (n_dec (dValue, precision));
#endif
}

/* WriteField functions
	Description:
		Writes text entity. (performs XML_writeEntity, XML_writeText, XML_endEntity in order)
	Parameters:
		element - element tag name
		value - value of text
		type - data type of parameter "value"
				#define XML_Integer		1
				#define XML_Long		2
				#define XML_Float		3
				#define XML_Double		4
				#define XML_Money		5
				#define XML_Char		6
				#define XML_Date		7
*/
void 
WriteField (
	char *element, 
	void *value, 
	int type)
{
	char xmlValue [MAXTEXT_LEN];
	
	switch (type)
	{
		case XML_Integer:
		{
			xmltype.i = (int *) value;
			sprintf (xmlValue, "%d", *xmltype.i);
			break;
		}
		case XML_Long:
		{
			xmltype.l = (long *) value;
			sprintf (xmlValue, "%ld", *xmltype.l);
			break;
		}
		case XML_Float:
		{
			xmltype.f = (float *) value;
			sprintf (xmlValue, "%f", *xmltype.f);
			break;
		}
		case XML_Double:
		{
			xmltype.d = (double *) value;
			sprintf (xmlValue, "%g", *xmltype.d);
			break;
		}
		case XML_Money:
		{
			xmltype.d = (double *) value;
			sprintf (xmlValue, "%g", *xmltype.d/100);
			break;
		}
		case XML_Char:
		{
			xmltype.ch = (char *) value;
			sprintf (xmlValue, "%s", xmltype.ch);
			break;
		}
		case XML_Date:
		{
			xmltype.l = (long *) value;
#ifdef WIN32
			sprintf (xmlValue, "%ld", *xmltype.l);
#else
			sprintf (xmlValue, "%s", DateToString (*xmltype.l));
#endif
			break;
		}
	}

	if (ALLOW_EMPTY_STRING || strlen (xmlValue) > 0)
	{
		XML_writeEntity (element);
		XML_writeText (xmlValue);
		XML_endEntity ();
	}
}

void WriteFieldChar (char *element, char *value)
{
	WriteField (element, value, XML_Char);
}
void WriteFieldInt (char *element, int value)
{
	WriteField (element, &value, XML_Integer);
}
void WriteFieldLong (char *element, long value)
{
	WriteField (element, &value, XML_Long);
}
void WriteFieldFloat (char *element, float value)
{
	WriteField (element, &value, XML_Float);
}
void WriteFieldDouble (char *element, double value)
{
	WriteField (element, &value, XML_Double);
}
void WriteFieldMoney (char *element, double value)
{
	WriteField (element, &value, XML_Money);
}
void WriteFieldDate (char *element, long value)
{
	WriteField (element, &value, XML_Date);
}
