/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: crc.h,v 5.0 2002/05/08 01:30:07 scott Exp $
|  Program Name  : (crc.h)
|  Program Desc  : ()
|---------------------------------------------------------------------|
| $Log: crc.h,v $
| Revision 5.0  2002/05/08 01:30:07  scott
| CVS administration
|
| Revision 1.1  2002/02/05 02:39:50  kaarlo
| Initial check-in for ORACLE8i porting.
|
|
=====================================================================*/

#ifndef __CRC_H__
#define __CRC_H__

void   initialize_CRC_table (void);
int    compute_revCRC_16 (const char *str, unsigned short int *crc16_val);

#endif