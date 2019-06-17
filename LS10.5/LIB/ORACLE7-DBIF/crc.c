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
