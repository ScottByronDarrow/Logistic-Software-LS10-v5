#ifndef __CRC_H__
#define __CRC_H__

int    compute_revCRC_16( const char *str, unsigned short int *crc16_val );
void   initialize_CRC_table( void );

#endif

