/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLReader.c)                                     |
|  Program Desc  : ( Functions for XML reader                       ) |
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
| $Id: XMLReader.c,v 5.4 2002/10/23 03:28:57 robert Exp $   
| $Log: XMLReader.c,v $
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
#include <XMLReader.h>

#include <unistd.h>

/* global variables */
int XML_LOGLEVEL = 2;

/* Private variables */
struct XMLRead_Stack
{
	struct ElementPart *element;
	struct XMLRead_Stack *next;
};

static struct ElementPart *mRootNode = NULL;
static struct ElementPart *lastParent = NULL;
static struct XMLRead_Stack *headXMLRead_Stack = NULL;
static const char *XML_SKIPCHARS = "\n\r\t";
static char xmlpath [128];
static FILE *fpLogFile;
static int LogOpen = 0;
static int CurrentLineNo = 1;

/* Private Functions */
int XML_GetNextChar (FILE *);
struct ElementPart *CreateNewElement ();
int PushToStack (struct ElementPart *);
int PopFromStack ();
int EmptyStack ();
int ProcessOpenTag (FILE *);
int ProcessEndTag (FILE *);
int ProcessText (FILE *);
int ProcessComment (FILE *);

/*-----------------------
START OF IMPLEMENTATION
------------------------*/

/*---------------------------------------------------------------
	LoadXML
	Description:
   		Builds a tree structure from the specified XML document
	Parameter: 
   		filename - the filepath of the source XML document
	Return Value:
   		Root element of the XML document
----------------------------------------------------------------*/
struct ElementPart *LoadXML (char *filename)
{
	FILE *fp;
	char c;
	
	strcpy (xmlpath, filename);
	CurrentLineNo = 1;

#ifdef WIN32
	if (_access (filename, 0) == -1)
	{
		LogXMLMessage (1, "[ERROR]: file not found [%s].", filename);
		return (NULL);
	}
#else
	if (access (filename, 0) == -1)
	{
		LogXMLMessage (1, "[ERROR]: file not found [%s].", filename);
		return (NULL);
	}
#endif

	if (mRootNode)
		FreeXMLTree ();

	fp = fopen (filename, "r");
	c = XML_GetNextChar (fp);
	while (!feof (fp))
	{		
		switch (c)
		{
			case '<':
			{
				c = XML_GetNextChar (fp);
				switch (c)
				{
					case '/':
					{
						if (ProcessEndTag (fp))
						{
							FreeXMLTree ();
							fclose (fp);
							return (NULL);
						}
						break;
					}
					case '!':
					{
						if (ProcessComment (fp))
						{
							FreeXMLTree ();
							fclose (fp);
							return (NULL);
						}
						break;
					}
					default:
					{
						ungetc (c, fp);
						
						if (ProcessOpenTag (fp))
						{
							FreeXMLTree ();
							fclose (fp);
							return (NULL);
						}
					}
				}
				break;
			}
			default:
			{
				ungetc (c, fp);
				if (ProcessText (fp))
				{
					FreeXMLTree ();
					fclose (fp);
					return (NULL);
				}
			}
		}

		c = XML_GetNextChar (fp);
	}

	// Stack should be empty after processing the XML file
	if (headXMLRead_Stack)
	{
		LogXMLMessage (1, "[ERROR]: Stack not empty after XML load.");
		FreeXMLTree ();
		return (NULL);		
	}

	EmptyStack ();

	CurrentLineNo = -1;
	fclose (fp);
	return (mRootNode);
}


/*----------------------------------------------
	FreeXMLTree
	Description:
		Free memory allocated by LoadXML
	Parameter:
		root - the root node of the XML tree
-----------------------------------------------*/
int FreeXMLTree ()
{
	FreeElement (mRootNode);
	mRootNode = NULL;
	
	EmptyStack ();

	if (LogOpen && fpLogFile)
	{
		LogOpen = 0;
		fclose (fpLogFile);
	}

	return (0);
}

/*------------------------------------------------------------
	GetFirstChildWithLabel
	Description:
		returns the first child of the specified node having 
		the specified label
	Parameters:
		parent - the parent node
		label - child label
--------------------------------------------------------------*/
struct ElementPart *GetFirstChildWithLabel (struct ElementPart *parent, char *label) 
{
	struct ElementPart *element;

	if (!parent)
		return (NULL);

	element = parent->firstChild;
	
	while (element)
	{
		if (!istrcmp (element->label_name, label))
			return (element);
		element = element->sibling;
	}

	return (element);
}

/*------------------------------------------------------------
	GetNextSiblingWithLabel
	Description:
		returns the immediate sibling of the specified node 
		having the specified label
	Parameters:
		element - the node whom we want to know its sibling
		label - label of element's sibling
-------------------------------------------------------------*/
struct ElementPart *GetNextSiblingWithLabel (struct ElementPart *element, char *label) 
{
	struct ElementPart *siblingElement;

	if (!element)
		return (NULL);

	siblingElement = element->sibling;
	
	while (siblingElement)
	{
		if (!istrcmp (siblingElement->label_name, label))
			return (siblingElement);
		siblingElement = siblingElement->sibling;
	}

	return (siblingElement);
}

/*-------------------------------------------------------------
	GetNextSibling
	Description:
		returns the immediate sibling of the specified node
	Parameters:
		element - the node whom we want to know its sibling
--------------------------------------------------------------*/
struct ElementPart *GetNextSibling (struct ElementPart *element) 
{
	if (!element)
		return (NULL);

	return (element->sibling);
}

/*-------------------------------------------------------
	GetFirstChild
	Description:
		returns the first child of the specified node
	Parameters:
		element - the parent node
--------------------------------------------------------*/
struct ElementPart *GetFirstChild (struct ElementPart *element) 
{ 
	if (!element)
		return (NULL);

	return (element->firstChild);
}

/*--------------------------------------------------------
	GetLastChild
	Description:
		returns the last child of the specified node
	Parameters:
		element - the parent node
---------------------------------------------------------*/
struct ElementPart *GetLastChild (struct ElementPart *element) 
{ 
	if (!element)
		return (NULL);

	return (element->lastChild);
}

/*---------------------------------------------------------------
	IsTText
	Description:
		returns true of if the specified node has a text value. 
		otherwise, return false.
-----------------------------------------------------------------*/
int	   IsTText (struct ElementPart *element) 
{ 
	return (element->type == TText);
}

/*------------------------------------------------------------------
	IsTElement
   Description:
		returns true of if the specified node has children element. 
		otherwise, return false.
--------------------------------------------------------------------*/
int    IsTElement (struct ElementPart *element) 
{
	return (element->type == TElement);
}

/*-------------------------------------------------------
	GetLabelName
	Description:
		Returns the Label Name of the given ElementPart.
--------------------------------------------------------*/
char 	*GetLabelName (struct ElementPart *element) 
{ 
	if (!element)		
		return (NULL);

	return (element->label_name);
}

/*---------------------------------------------------------
	GetChildLabelCount
	Description: Returns the total number of children of 
		given ElementPart and a given label name
----------------------------------------------------------*/
int		GetChildLabelCount (struct ElementPart *parent, char *label) 
{
	int childCount;

	struct ElementPart *element;

	if (!parent)
		return (0);

	childCount = 0;
	element = parent->firstChild;	
	while (element)
	{
		if (!istrcmp (element->label_name, label))
			childCount++;

		element = element->sibling;
	}

	return (childCount);
}

/*---------------------------------------------------------
	FreeElement
	Description:
		Release memory allocated to the specified element
		and all its descendants
----------------------------------------------------------*/
int FreeElement (struct ElementPart *element)
{
	struct ElementPart *delElement, *pivot;
	if (!element)
		return (0);

	// Free all descendants first
	pivot = element->firstChild;	
	while (pivot)
	{
		delElement = pivot;
		pivot = pivot->sibling;

		// Recurse
		FreeElement (delElement);
	}
	
	if (element->value)
		free (element->value);

	free (element);

	return (0);
}

/*------------------------------------------------------------
	ViewXML
	Description:
		Converts/Writes the tree structure into its XML form
		having the specified element as the root.
-------------------------------------------------------------*/
int ViewXML (struct ElementPart *element)
{
	struct ElementPart *vwElement;
	if (!element)
		return (0);

	// View all descendants first
	XML_writeEntity (element->label_name);

	vwElement = element->firstChild;	
	while (vwElement)
	{
		// Recurse
		ViewXML (vwElement);
		vwElement = vwElement->sibling;
	}

	if (element->type == TText && element->value)
		XML_writeText (element->value);

	XML_endEntity ();

	return (0);
}

/*---------------------------------------------------------
	GetLabelText
	Description:
		Returns the text content of the specified element
----------------------------------------------------------*/
char	*GetLabelText (struct ElementPart *element)
{
	static char *emptyString = "";

	if (!element)
		return (emptyString);

	if (!element->value)
		return (emptyString);

	return (element->value);
}

/*-------------------------------------------------
	GetRootNode
	Description:
		Returns the topmost parent (root element)
---------------------------------------------------*/
struct	ElementPart *GetRootNode ()
{
	return (mRootNode);
}


/*------------------------------
	START OF PRIVATE FUNCTIONS
------------------------------*/

int ProcessOpenTag (FILE *fp)
{
	char c;
	static char tagname	[101];	
	int charCount;
	int emptyTag;
	int firstNonSpace;
	struct ElementPart *newElement;
	
	firstNonSpace = 0;
	charCount = 0;
	emptyTag = 0;
	c = XML_GetNextChar (fp);
	while (!feof (fp) && charCount < 100)
	{
		if (!firstNonSpace)
		{
			if (c == ' ')
			{
				c = XML_GetNextChar (fp);
				continue;
			}

			firstNonSpace = 1;
		}

		if (c == '/')
		{
			emptyTag = 1;
			c = XML_GetNextChar (fp);
			continue;
		}

		if (c == ' ' || c == '>')
			break;

		tagname [charCount] = c;		
		charCount++;

		c = XML_GetNextChar (fp);
	}

	tagname [charCount] = '\0';
	if (strlen (tagname) == 0)
	{
		// Tagname cannot be empty
		LogXMLMessage (1, "[ERROR]: Tagname cannot be empty");
		return (1);
	}


	// skip attributes
	while (c != '>')
	{
		c = XML_GetNextChar (fp);
		if (feof (fp))
			break;

		if (c == '/')
			emptyTag = 1;
	}

	if (c != '>')
	{
		//Fatal Error
		LogXMLMessage (1, "[ERROR]: Cannot find closing tag '>'");
		return (1);
	}
	
	newElement = CreateNewElement ();
	if (!newElement)
	{
		//Cannot Allocate Memory
		LogXMLMessage (1, "[ERROR]: Cannot Allocate Memory");
		return (1);
	}
	
	// if not root element
	if (lastParent)
	{
		if (lastParent->firstChild)
		{
			lastParent->lastChild->sibling = newElement;
			lastParent->lastChild = lastParent->lastChild->sibling;
		}
		else
			lastParent->firstChild = lastParent->lastChild = newElement;
	}
	else
	{
		if (mRootNode)
		{
			//Cannot have more than one root node
			LogXMLMessage (1, "[ERROR]: Cannot have more than one root node");
			return (1);
		}
		mRootNode = newElement;
	}

	strcpy (newElement->label_name, tagname);
	if (!emptyTag)
	{
		if (PushToStack (newElement))
		{
			//Cannot Allocate Memory
			LogXMLMessage (1, "[ERROR]: Cannot Allocate Memory on PushToStack");
			return (1);
		}
	}
	return (0);
}

int ProcessEndTag (FILE *fp)
{
	char c;
	static char tagname	[101];	
	int charCount;
	
	charCount = 0;
	c = XML_GetNextChar (fp);
	while (!feof (fp) && charCount < 100)
	{
		if (c == ' ' || c == '>')
			break;

		tagname [charCount] = c;
		charCount++;
		c = XML_GetNextChar (fp);
	}
	tagname [charCount] = '\0';

	// skip attributes
	while (c != '>')
	{
		c = XML_GetNextChar (fp);
		if (feof (fp))
			break;
	}

	if (istrcmp (lastParent->label_name, tagname))
	{
		// Fatal Error
		LogXMLMessage (1, "[ERROR]: start [%s] and end [%s] tag doesn't matched.", lastParent->label_name, tagname);
		return (1);
	}

	PopFromStack ();
	return (0);
}

int ProcessText (FILE *fp)
{
	char c;
	int textCount, i;
	static char text_value [1001];
	int firstNonSpace;
	
	firstNonSpace = 0;
	textCount = 0;
	c = XML_GetNextChar (fp);

	while (!feof (fp) && textCount < 1000)
	{		
		if (c == '<')
			break;

		if (!firstNonSpace)
		{
			if (c == ' ')
			{
				c = XML_GetNextChar (fp);
				continue;
			}

			firstNonSpace = 1;
		}

		text_value [textCount] = c;
		textCount++;
		c = XML_GetNextChar (fp);
	}

	for (i=textCount;i > 0; i--)
	{
		if (text_value [i-1] != ' ')
		{
			textCount = i;
			break;
		}
	}

	text_value [textCount] = '\0';

	while (c != '<')
	{
		c = XML_GetNextChar (fp);
		if (feof (fp))
			break;
	}

	if (c != '<')
	{
		if (!lastParent)
		{
			// Warning, extra characters after end of root element
			LogXMLMessage (2, "[WARNING]: extra characters after end of root element.");
			return (0);
		}

		// Fatal Error
		LogXMLMessage (1, "[ERROR]: cannot find end of text element");
		return (1);
	}

	if (textCount > 0)
	{
		lastParent->value = strdup (XML_xltescape (text_value));
		lastParent->type = TText;
	}

	ungetc (c, fp);	

	return (0);
}

int EmptyStack ()
{
	while (1)
	{
		if (PopFromStack ())
			break;
	}

	headXMLRead_Stack = NULL;

	return (0);
}

int PushToStack (struct ElementPart *newElement)
{
	struct XMLRead_Stack *newStackElement;
	newStackElement = (struct XMLRead_Stack *) malloc (sizeof (struct XMLRead_Stack));
	if (newStackElement == NULL)
		return (1);

	lastParent = newElement;
	newStackElement->element = newElement;
	newStackElement->next = headXMLRead_Stack;
	headXMLRead_Stack = newStackElement;	

	return (0);
}

int PopFromStack ()
{
	struct XMLRead_Stack *delPtr;
	
	if (!headXMLRead_Stack)
	{
		lastParent = NULL;
		return (1);
	}

	delPtr = headXMLRead_Stack;
	headXMLRead_Stack = headXMLRead_Stack->next;
	if (headXMLRead_Stack)
		lastParent = headXMLRead_Stack->element;
	else
		lastParent = NULL;

	free (delPtr);

	return (0);
}

struct ElementPart *CreateNewElement ()
{
	struct ElementPart *newElement;
	
	newElement = (struct ElementPart *) malloc (sizeof (struct ElementPart));
	if (!newElement)
		return (NULL);

	memset (newElement, 0, sizeof (struct ElementPart));
	return (newElement);
}

int XML_GetNextChar (FILE *fp)
{
	char c;

	c = getc (fp);

	while (!feof (fp) && strchr (XML_SKIPCHARS, c))
	{
		if (c == '\n')
			CurrentLineNo++;
		c = getc (fp);
	}

	return (c);
}

int ProcessComment (FILE *fp)
{
	char c;
	int cnt;	
	char marker [2];

	// Start of comment tag should be "<!--"
	// After <!, next characters should be "--"
	c = XML_GetNextChar (fp);
	if (c != '-')
	{
		LogXMLMessage (1, "[ERROR]: comment structure not valid");
		return (1);
	}
	c = XML_GetNextChar (fp);
	if (c != '-')
	{
		LogXMLMessage (1, "[ERROR]: comment structure not valid");
		return (1);
	}

	cnt = 0;
	c = XML_GetNextChar (fp);
	while (!feof (fp) && c != '>')
	{
		marker [cnt] = c;		
		c = XML_GetNextChar (fp);
		cnt = (cnt+1) % 2;
	}

	// End of comment tag should be -->
	if (c != '>' || marker [0] != '-' || marker [1] != '-')
	{
		LogXMLMessage (1, "[ERROR]: comment structure not valid");
		return (1);
	}

	return (0);
}

void LogXMLMessage (int level, char *mask, ...)
{
	char tmp_str [256];	
	va_list	args;

	if (level > XML_LOGLEVEL)
		return;

	if (!LogOpen)
	{
		if (fpLogFile)
			fclose (fpLogFile);

		sprintf (tmp_str, "%s.log", xmlpath);
		fpLogFile = fopen (tmp_str, "w");
		LogOpen = 1;
	}
	
	va_start (args, mask);
	vsprintf (tmp_str, mask, args);
	va_end (args);

	if (fpLogFile)
		fprintf (fpLogFile, "%s [line%d]\n", tmp_str, CurrentLineNo);
}
