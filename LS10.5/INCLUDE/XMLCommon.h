/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( XMLCommon.h)                                     |
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
| $Id: XMLCommon.h,v 1.2 2002/10/22 07:27:46 scott Exp $   
| $Log: XMLCommon.h,v $
| Revision 1.2  2002/10/22 07:27:46  scott
| Updated for #endif
|
| Revision 1.1  2002/10/22 06:17:58  robert
| Initial check-in. XML functions
|  
|                                                                     |
=====================================================================*/

#ifndef _XMLCOMMON_H
#define _XMLCOMMON_H

#include <stdlib.h>
#include <stdio.h>

#ifdef WIN32
#include <io.h>
#endif

#include <string.h>

#define MAXTEXT_LEN		512

char *strtoupper (char *s);
int istrcmp (char *s1, char *s2);
char *XML_replaceString (char *src, char *s1, char *s2);
char *XML_escape (char *str);
char *XML_xltescape (char *str);

#endif /*_XMLCOMMON_H*/
