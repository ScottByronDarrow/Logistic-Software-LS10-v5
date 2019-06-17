/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLWriter.h)                                     |
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
| $Id: XMLWriter.h,v 1.2 2002/10/23 03:29:49 robert Exp $   
| $Log: XMLWriter.h,v $
| Revision 1.2  2002/10/23 03:29:49  robert
| Updated to add variable ALLOW_EMPTY_STRING to allow empty entity having
| empty text. Make XML_LOGLEVEL as int variable instead of #define.
|
| Revision 1.1  2002/10/22 06:17:58  robert
| Initial check-in. XML functions
|  
|                                                                     |
=====================================================================*/

#ifndef __XMLWRITER_H
#define __XMLWRITER_H

#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <io.h>
#endif

#include <string.h>
#include <XMLCommon.h>


#define XML_Integer		1
#define XML_Long		2
#define XML_Float		3
#define XML_Double		4
#define XML_Money		5
#define XML_Char		6
#define XML_Date		7

// global variables
union
{
   char   *ch;
   int    *i;
   long   *l;
   float  *f;
   double *d;
} xmltype;

/*-----------------------------------------
XML FUNCTION PROTOTYPES (Public Functions)
-----------------------------------------*/
void XML_setOutput (FILE *fp);
void XML_writeText (char *text);
void XML_close ();
void XML_endEntity();
void XML_writeAttribute (char *attr, char *value);
void XML_writeAttributes ();
void XML_writeEntity (char *name);

#endif // __XMLWRITER_H
