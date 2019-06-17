/*=====================================================================
|  Copyright (C) 1996 - 2002 Logistic Software                        |
|=====================================================================|
|  Program Name  : ( LSXMLInterface.h )                               |
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
| $Id: LSXMLInterface.h,v 1.3 2002/10/23 03:29:49 robert Exp $   
| $Log: LSXMLInterface.h,v $
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

#ifndef _LSXMLINTERFACE_H
#define _LSXMLINTERFACE_H

#include <pslscr.h>
#include <XMLReader.h>
#include <XMLWriter.h>

// global variables

// variable that allows input of empty string in XML document
// default: TRUE
extern int ALLOW_EMPTY_STRING;

/*-------------------------------------
LS/10 SPECIFIC INTERFACE: FUNCTION PROTOTYPES
-------------------------------------*/

// XML Reader
char	*GetTextValue_char (struct ElementPart *,  char *);
float	GetTextValue_float (struct ElementPart *,  char *);
double	GetTextValue_double (struct ElementPart *,  char *);
double	GetTextValue_money (struct ElementPart *,  char *);
long	GetTextValue_date (struct ElementPart *,  char *);
long 	GetTextValue_long (struct ElementPart *,  char *);
int 	GetTextValue_int (struct ElementPart *,  char *);

/*
	Same as GetTextValue_double, GetTextValue_float, GetTextValue_money
	except performs rounding to the nearest precision parameter.
*/
float	GetTextValue_float_p (struct ElementPart *,  char *, int);
double	GetTextValue_double_p (struct ElementPart *,  char *, int);
double	GetTextValue_money_p (struct ElementPart *,  char *, int);

// XML Writer
void WriteField (char *, void *, int);
void WriteFieldChar (char *, char *);
void WriteFieldInt (char *, int );
void WriteFieldLong (char *, long);
void WriteFieldFloat (char *, float);
void WriteFieldDouble (char *, double);
void WriteFieldMoney (char *, double);
void WriteFieldDate (char *, long);

#endif /* _LSXMLINTERFACE_H */
