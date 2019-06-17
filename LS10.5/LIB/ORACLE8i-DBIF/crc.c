/*====================================================================|
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: crc.c,v 5.0 2002/05/08 01:30:07 scott Exp $
|  Program Name  : (crc.c)
|  Program Desc  : ()
|---------------------------------------------------------------------|
| $Log: crc.c,v $
| Revision 5.0  2002/05/08 01:30:07  scott
| CVS administration
|
| Revision 1.2  2002/03/11 02:31:56  scott
| Updated to perform code check and comment lineups.
|
| Revision 1.1  2002/02/05 02:39:50  kaarlo
| Initial check-in for ORACLE8i porting.
|
|
=====================================================================*/

#include <stdio.h>

#include "crc.h"

/*
 * Constants 
 */
#define CRC16_REV 0XA001

/*
 * Global variable 
 */
static unsigned short int crctbl[256];

/*
 * Local functions 
 */
static unsigned short int crcrevhware (unsigned short int data, 
                                       unsigned short int accum);
static void crcrevupdate (unsigned short int data,
						  unsigned short int *accum);
     
/*
 * initialize_CRC_table 
 */
void initialize_CRC_table (void)
{
	int i; 

	for (i = 0; i < 256; i++ )
		crctbl[i] = crcrevhware(i,0); 
}

/*
 * crcrevhware 
 */
unsigned short int crcrevhware ( 
	unsigned short int data, 
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

/*
 * compute_revCRC_16 
 */
int compute_revCRC_16 ( 
	const char *str, 
	unsigned short int *crc16_val)
{
	unsigned short int accum;
	int ch, i;

	accum = 0;
	i     = 0;
  
	while ((ch = str[i++]))
		crcrevupdate( ch, &accum );

	accum = (accum >> 8) + (accum << 8);
	*crc16_val = accum;

	return 1;
}

/*
 * crcrevupdate 
 */
void crcrevupdate (
	unsigned short int data, 
	unsigned short int *accum)
{
	static int comb_val;

	comb_val = *accum ^ data;
	*accum   = (*accum >> 8) ^ crctbl[comb_val & 0x00ff];
}

