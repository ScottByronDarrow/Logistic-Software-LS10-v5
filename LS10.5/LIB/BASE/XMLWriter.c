/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLWriter.c)                                     |
|  Program Desc  : ( Functions for XML writer                       ) |
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
| $Id: XMLWriter.c,v 5.5 2002/10/23 06:54:37 robert Exp $   
| $Log: XMLWriter.c,v $
| Revision 5.5  2002/10/23 06:54:37  robert
| Updated to recover gracefully after unfinished XML
|
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
#include <XMLWriter.h>


/* Private variables */
struct Stack
{
	char value [30];
	struct Stack *next;
};

static struct Stack *headStack = NULL;
static char XML_attrs [250];
static int XML_CLOSED = 1;
static int XML_EMPTY = 1;
static int XML_LEVEL = 0;
static int XML_TEXT = 0;
static FILE *xmlOut = NULL;

/* Private Functions */
int  XML_AddToStack (char *element);
char *XML_PopFromStack (char *element);
void XML_closeOpeningTag ();

/*-----------------------
START OF IMPLEMENTATION
------------------------*/

/*--------------------------------------------
	XML_setOutput
	Description:
		Stream destination of the XML output
---------------------------------------------*/
void 
XML_setOutput (FILE *fp)
{
	xmlOut = fp;
}

/*--------------------------------------------
	XML_writeEntity
	Description:
		Writes/Add new XML tag
---------------------------------------------*/
void XML_writeEntity(char *name)
{	
	int cnt;	

    if (!xmlOut)
	xmlOut = stdout;
	
    XML_closeOpeningTag();
    XML_CLOSED = 0;

    if (XML_LEVEL > 0)
    	fprintf (xmlOut, "\n");

    for (cnt=0; cnt < XML_LEVEL; cnt++)
		fprintf (xmlOut, "\t");
		
    fprintf (xmlOut, "<");
    fprintf (xmlOut, "%s", name);        
    XML_AddToStack (name);
    XML_EMPTY = 1;
}

/*--------------------------------------------
	XML_writeEntity
	Description:
		Writes out all attributes of the
		current element tag
---------------------------------------------*/
void XML_writeAttributes ()
{
    if (strlen (XML_attrs))
    {
        fprintf (xmlOut, "%s", XML_attrs);
        strcpy (XML_attrs, "");
        XML_EMPTY = 0;
    }
}

/*------------------------------------------------------
	XML_writeAttribute
	Description:
		Write an attribute out for the current entity. 
		Any xml characters in the value are escaped.
-------------------------------------------------------*/
void XML_writeAttribute (char *attr, char *value)
{
    strcat (XML_attrs, " ");
    strcat (XML_attrs, attr);
    strcat (XML_attrs, "=\"");
    strcat (XML_attrs, XML_escape(value));
    strcat (XML_attrs, "\"");
}

/*-------------------------------
	XML_endEntity
	Description:
		End the current entity.
---------------------- --------*/
void XML_endEntity()
{
	char name [30];
	int cnt;
	
    if (headStack == NULL)
        return;
            
    if (XML_PopFromStack (name) != NULL) {
        if (XML_EMPTY)
        {
            XML_writeAttributes();
            fprintf (xmlOut, "/>");
            XML_CLOSED = 1;
        } else
        {        	
        	if (!XML_TEXT)
        	{
        		fprintf (xmlOut, "\n");
	    		for (cnt=0; cnt < XML_LEVEL; cnt++)
					fprintf (xmlOut, "\t");
			}

			XML_TEXT = 0;

            fprintf (xmlOut, "</");
            fprintf (xmlOut, "%s", name);
            fprintf (xmlOut, ">");
        }
        XML_EMPTY = 0;
        
    }
}

/*---------------------------------
	XML_close
	Description:
		Close the XML Writer.
----------------------------------*/
void XML_close()
{
	char name [30];
	
	fprintf (xmlOut, "\n");

	// clear all data from stack
	while (1)
	{
		if (XML_PopFromStack (name) == NULL)
			break;

		// print an error message
		fprintf (xmlOut, "<[%s] is unclosed>\n", name);
	}

	// reset variables
	headStack = NULL;
	XML_CLOSED = 1;
	XML_EMPTY = 1;
	XML_LEVEL = 0;
	XML_TEXT = 0;
	xmlOut = stdout;
}

/*---------------------------------------------------------
	XML_writeText
	Description:
		Output body text. Any xml characters are escaped. 
----------------------------------------------------------*/
void XML_writeText (char *text)
{
    XML_closeOpeningTag();
    XML_EMPTY = 0;
    XML_TEXT = 1;
    fprintf (xmlOut, "%s", XML_escape (text));
}

/*------------------------------
	START OF PRIVATE FUNCTIONS
-------------------------------*/
int
XML_AddToStack (char *element)
{
	struct Stack *newStackElement;
	newStackElement = (struct Stack *) malloc (sizeof (struct Stack));
	if (newStackElement == NULL)
		return (1);
		
	strcpy (newStackElement->value, element);

	newStackElement->next = headStack;
	headStack = newStackElement;
	XML_LEVEL++;
	
	return (0);
}


/*------------------------------
	START OF PRIVATE FUNCTIONS
------------------------------*/

char *XML_PopFromStack (char *element)
{
	struct Stack *delPtr;
	
	if (!headStack)
		return (NULL);

	strcpy (element, headStack->value);
	
	delPtr = headStack;
	headStack = headStack->next;
	free (delPtr);	
	XML_LEVEL--;
	
	return (element);
}

// close off the opening tag
void XML_closeOpeningTag ()
{
    if (!XML_CLOSED) {
        XML_writeAttributes ();
        XML_CLOSED = 1;
        fprintf (xmlOut, ">");
    }
}
