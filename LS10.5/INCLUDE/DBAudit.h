/*=====================================================================
|  Copyright (C) 2000 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: DBAudit.h,v 5.2 2002/04/11 03:51:16 scott Exp $
|=====================================================================|
|  Program Name  : (DBAudit.h)
|  Program Desc  : (Standard Audit Header File)
|=====================================================================|
| $Log: DBAudit.h,v $
| Revision 5.2  2002/04/11 03:51:16  scott
| Updated to add comments to audit files.
|
| Revision 5.1  2001/09/27 02:35:14  cha
| New header file for DBAudit.c.
|
|
|
=====================================================================*/
#ifndef _DBAudit_h
#define _DBAudit_h

int  OpenAuditFile  (const char *);
int  AuditFileAdd   (char *, void *, struct dbview  *, const int);
int  CloseAuditFile (void);
void SetAuditOldRec (void *, int);

#endif  /* _DBAudit_h */ 
