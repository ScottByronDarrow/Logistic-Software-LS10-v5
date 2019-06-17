/*
+-----------------------------------------------------------------+
|  LOG.c - Logger of pertinent debugging information to external  |
|          log files module.                                      |
|               History:                                          |
|          Initial Development  October 1, 1996 Raymund           |
|                                                                 |
|             Copyright (c) 1996 INFORMATION MANAGERS, INC.       |
+-----------------------------------------------------------------+
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "log.h"

#define PATH_LEN   120
char *timestamp(void);
extern char *PNAME; 


/*
+---------------------------------------------------------------+
| Writes a formatted output to a logfile specified by filename  |
| in the directory specified by the global variable log_dir     |
+---------------------------------------------------------------+
*/
void LOG( const char *filename, const char *fmt, ...)
{
   va_list ap;
   char    buffer[1024],     /* 100 bytes should be enough */
           path[120],
           *timedata;
   FILE    *fp;


   if (   strlen(filename)  > PATH_LEN )
   {
      return;        
   }

   strcpy( path, filename );
   fp = fopen(path,"a");
   if (fp == NULL)
   {
     return;
   }
   va_start( ap, fmt );
   vsprintf( buffer, fmt, ap );
   fprintf( fp, "[%s] (%d) <%s>\n", timestamp(), getpid(0), PNAME );
   fprintf( fp, "%s\n", buffer ); 
   
   fclose(fp);

}

char *timestamp(void)
{
  time_t      now;
  struct tm   datetime;
  static char thetime[18];

  time(&now);
  datetime = *localtime(&now);
  strftime(thetime, 18, "%D %T", &datetime );
  return thetime;
}
