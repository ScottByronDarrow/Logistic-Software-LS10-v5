/*
+-----------------------------------------------------------------+
|  LOG.h - Header file for LOG.c which is the logger of pertinent |
|          information of the operation of the gateway.           |
|               History:                                          |
|                 Initial Development  October 1, 1996 Raymund    |
|                                                                 |
+-----------------------------------------------------------------+
*/

#ifndef __LOG_H__
#define __LOG_H__

void LOG( const char *filename, const char *fmt, ...);

#endif
