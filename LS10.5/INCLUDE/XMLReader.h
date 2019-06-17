/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLReader.h)                                     |
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
| $Id: XMLReader.h,v 1.3 2002/10/23 03:29:49 robert Exp $   
| $Log: XMLReader.h,v $
| Revision 1.3  2002/10/23 03:29:49  robert
| Updated to add variable ALLOW_EMPTY_STRING to allow empty entity having
| empty text. Make XML_LOGLEVEL as int variable instead of #define.
|
| Revision 1.2  2002/10/22 07:27:46  scott
| Updated for #endif
|
| Revision 1.1  2002/10/22 06:17:58  robert
| Initial check-in. XML functions
|  
|                                                                     |
=====================================================================*/

#ifndef __XMLREADER_H
#define __XMLREADER_H

#ifdef WIN32
#include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <XMLCommon.h>

/*
	Define log level
		0 - Disable error/message logging.
		1 - All errors will be logged.
		2 - All errors and warnings will be logged. (default)
*/
extern int XML_LOGLEVEL;

/* Public variables */

/* Tree structure definition*/
enum TType { TElement, TText};
struct ElementPart
{
	char label_name [30];
	char *value;
	enum TType type;
	struct ElementPart *sibling;
	struct ElementPart *firstChild;
	struct ElementPart *lastChild;
	
	/* Maybe needed */
	//struct ElementPart *parent;
	//int childCount;
};


/*-----------------------------------------
XML FUNCTION PROTOTYPES (Public Functions)
-----------------------------------------*/
void	LogXMLMessage (int level, char *mask, ...);
int		FreeElement (struct ElementPart *);
int		ViewXML (struct ElementPart *);
struct	ElementPart *GetRootNode ();
int 	FreeXMLTree ();
struct	ElementPart *LoadXML (char *);

struct	ElementPart *GetFirstChildWithLabel (struct ElementPart *, char *);
struct	ElementPart *GetNextSiblingWithLabel (struct ElementPart *, char *);
struct	ElementPart *GetNextSibling (struct ElementPart *);
struct	ElementPart *GetFirstChild (struct ElementPart *);
struct	ElementPart *GetLastChild (struct ElementPart *);
int		IsTText (struct ElementPart *);
int		IsTElement (struct ElementPart *);
char 	*GetLabelName (struct ElementPart *);
int		GetChildLabelCount (struct ElementPart *, char *);
char	*GetLabelText (struct ElementPart *);

#endif /* __XMLREADER_H */
