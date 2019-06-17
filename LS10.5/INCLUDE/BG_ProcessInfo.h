/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: BG_ProcessInfo.h,v 5.0 2001/06/19 06:51:11 cha Exp $
-----------------------------------------------------------------------
| $Log: BG_ProcessInfo.h,v $
| Revision 5.0  2001/06/19 06:51:11  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/11 04:41:58  scott
| Added info files for base database create script.
|
|
*/
#ifndef	_bproInfo_h
#define	_bproInfo_h

	struct
	{
		char	*bproProgram;
		char	*bproPrinter;
		char	*bproArguments;
		char	*bproStatus;
	}	bproInfo []	=	{
		{"so_pscreat",	"0",	"P~",				"A"},
		{"so_bgcalc",	"1",	"MCFB76HG~CLEAR~",	"A"},
		{"so_bgstkup",	"0",	"X~4~",				"A"},
		{"sk_bgmove",	"0",	" ",				"A"},
		{"so_bgsales",	"1",	" ",				"N"},
		{"","","", ""},
		};
#endif	/*	_bproInfo_h */
