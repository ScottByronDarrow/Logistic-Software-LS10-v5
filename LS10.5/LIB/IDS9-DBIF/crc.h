/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: crc.h,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (locks.c)
|  Program Desc  : (handles crc algo)
|---------------------------------------------------------------------|
|$Log: crc.h,v $
|Revision 1.2  2002/11/11 02:41:10  cha
|Updated for GTEQ modifications.
| 
|
=====================================================================*/

#ifndef __CRC_H__
#define __CRC_H__

int    compute_revCRC_16( const char *str, unsigned short int *crc16_val );
void   initialize_CRC_table( void );

#endif

