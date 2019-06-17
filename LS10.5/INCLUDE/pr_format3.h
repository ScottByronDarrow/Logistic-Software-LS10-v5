/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( pr_format.h    )                                 |
|  Program Desc  : ( Formatted print routines.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : 10/04/87        |  Author     : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (10/04/87)      | Modified by : Roger Gibbison.    |
|  Date Modified : (23/08/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : Valid mask characters in input file are:           |
|                : ^AAAAA^         Character String - (%-4.4s)        |
|                : ^FFFF.FF^       Double/Float     - (%7.2f)         |
|                : ^DD/DD/DD^      Edate            - (%8.8s)         |
|                :    Note: This mask is tested for. ie ^DD/MM/YY^    |
|                :          is incorrect.                             |
|                :                                                    |
|                : ^III^           Integer          - (%3d)           |
|                : ^LLLLL^         Long Integer     - (%5ld)          |
|                : ^MMMMMM.MM^     Money            - (%9.2f)         |
|                :                                                    |
|                : The '^' characters are replaced by ' ' to retain   |
|                : the format of the document.                        |
|                :                                                    |
|                : label is a 10 character label at the beginning of  |
|                : each line of format.                               |
|                :                                                    |
|                : fields are numbered 1 - n                          |
|                :  a field_no of 0 prints the line as it is read.    |
|     (23/08/93) : HGP 9649 Port for SVR4 - moved code library        |
|                                                                     |
=====================================================================*/
#ifndef	PR_FORMAT3_H
#define	PR_FORMAT3_H

extern FILE	*pr_open (char *);		/* opens format file */

#if	defined (__STDC__)
extern int	pr_format (FILE *, FILE *, char *, int, ...),
			scr_format (FILE *, FILE *, char *, int, ...);
#else
extern int	scr_format (),
			pr_format ();
#endif

#endif	/*PR_FORMAT3_H*/
