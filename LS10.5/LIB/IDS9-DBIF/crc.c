/*====================================================================|
|  Copyright (C) 2002 Logistic Software Development, Inc.             |
|=====================================================================|
| $Id: crc.c,v 1.2 2002/11/11 02:41:10 cha Exp $
|  Program Name  : (crc.c)
|  Program Desc  : (handles crc algo)
|---------------------------------------------------------------------|
|$Log: crc.c,v $
|Revision 1.2  2002/11/11 02:41:10  cha
|Updated for GTEQ modifications.
| 
|
=====================================================================*/

/*Avoid duplicate definition for some 
functions and data structures from 
Informix and Standard C*/
#ifndef _H_LOCALEDEF
#define _H_LOCALEDEF
#endif  

#include <stdio.h>

#include "crc.h"

/*----------+
| Constants |        
+----------*/
#define CRC16_REV 0XA001

/*----------------+
| Global variable |
+----------------*/
static unsigned short int crctbl[256];

/*----------------+
| Local functions |
+----------------*/
static void crcrevupdate( unsigned short int data,
                          unsigned short int *accum );
static unsigned short int crcrevhware( unsigned short int data, 
                                       unsigned short int accum );

void crcrevupdate( unsigned short int data, unsigned short int *accum )
{
  static int comb_val;

  comb_val = *accum ^ data;
  *accum   = (*accum >> 8) ^ crctbl[comb_val & 0x00ff];
}

unsigned short int crcrevhware( unsigned short int data, 
                                unsigned short int accum )
{
  static int i;

  data <<= 1;
  for ( i = 8; i > 0; i-- )
  {
    data >>= 1;
    if ((data ^ accum) & 0x0001)
      accum = (accum >> 1) ^ CRC16_REV;
    else
      accum >>= 1;
  }
  return accum;
}

void initialize_CRC_table( void )
{
  int i; 

  for (i = 0; i < 256; i++ )
    crctbl[i] = crcrevhware(i,0); 

}

int compute_revCRC_16( const char *str, unsigned short int *crc16_val )
{
  unsigned short int accum;
  int                ch, i;

  accum = 0;
  i     = 0;
  while ( (ch = str[i++]) )
    crcrevupdate( ch, &accum );
  accum = (accum >> 8) + (accum << 8);
  *crc16_val = accum;

  return 1;
}
