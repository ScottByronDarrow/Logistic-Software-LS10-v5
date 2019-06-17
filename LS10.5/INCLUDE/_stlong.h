/*=====================================================================
|  Copyright (C) 2000 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _stlong.h,v 5.2 2002/07/26 05:50:36 robert Exp $
|=====================================================================|
|  Program Name  : (_stlong.h)
|  Program Desc  : (substitute for stlong for Oracle use only)
|=====================================================================|
| $Log: _stlong.h,v $
| Revision 5.2  2002/07/26 05:50:36  robert
| Fixed number overflow on license encryption (makes password key longer)
|
| Revision 5.1  2002/02/07 01:18:09  cha
| Initial checkin for _stlong.
| For Oracle use only.
|
|
=====================================================================*/

#ifdef ORA734
#define stlong _stlong 
void
_stlong(
   long l,
   char *p )
{
   p[0] = (char ) ((l & 0xff000000) >> 24);
   p[1] = (char ) ((l & 0x00ff0000) >> 16);
   p[2] = (char ) ((l & 0x0000ff00) >> 8);
   p[3] = (char ) (l & 0x000000ff);
}

#endif
