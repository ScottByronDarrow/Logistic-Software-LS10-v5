/* /port4/734dev/oracore3/public/sx.h */


/*
ORACLE, Copyright (c) 1982, 1983, 1986, 1990, 1995, 1996 ORACLE Corporation
ORACLE Utilities, Copyright (c) 1981, 1982, 1983, 1986, 1990, 1991, 1995, 1996 ORACLE Corp

Restricted Rights
This program is an unpublished work under the Copyright Act of the
United States and is subject to the terms and conditions stated in
your  license  agreement  with  ORACORP  including  retrictions on
use, duplication, and disclosure.

Certain uncopyrighted ideas and concepts are also contained herein.
These are trade secrets of ORACORP and cannot be  used  except  in
accordance with the written permission of ORACLE Corporation.
*/


/* $Header: sx.h@@/main/base_oracore_35_series/coreint_oracore_3.5.2.0.0 \
 *          /iylempel_oracore_3.5.2.0.0.1_prod/1 \
 * Checked in on Mon Mar 11 10:51:22 PST 1996 by iylempel \
 * Copyright (c) 1996, 1997 by Oracle Corporation. All Rights Reserved. \
 * $ */










#ifndef ORASTDDEF
# include <stddef.h>
# define ORASTDDEF
#endif

#ifndef ORALIMITS
# include <limits.h>
# define ORALIMITS
#endif

#ifndef  SX_ORACLE
#define  SX_ORACLE
#define  SX
#define  ORATYPES


#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif



#ifdef lint
# ifndef mips
#  define signed
# endif 
#endif 

#ifdef ENCORE_88K
# ifndef signed
#  define signed
# endif 
#endif 

#if defined(SYSV_386) || defined(SUN_OS)
# ifdef signed
#  undef signed
# endif 
# define signed
#endif 





#ifndef lint
typedef          int eword;                  
typedef unsigned int uword;                  
typedef   signed int sword;                  
#else
#define eword int
#define uword unsigned int
#define sword signed int
#endif 

#define  EWORDMAXVAL  ((eword) INT_MAX)
#define  EWORDMINVAL  ((eword)       0)
#define  UWORDMAXVAL  ((uword)UINT_MAX)
#define  UWORDMINVAL  ((uword)       0)
#define  SWORDMAXVAL  ((sword) INT_MAX)
#define  SWORDMINVAL  ((sword) INT_MIN)
#define  MINEWORDMAXVAL  ((eword)  32767)
#define  MAXEWORDMINVAL  ((eword)      0)
#define  MINUWORDMAXVAL  ((uword)  65535)
#define  MAXUWORDMINVAL  ((uword)      0)
#define  MINSWORDMAXVAL  ((sword)  32767)
#define  MAXSWORDMINVAL  ((sword) -32767)

 
#ifndef lint
# ifdef mips
typedef   signed char  eb1;
# else
typedef          char  eb1;                  
# endif 
typedef unsigned char  ub1;                  
typedef   signed char  sb1;                  
#else
#define eb1 char
#define ub1 unsigned char
#define sb1 signed char
#endif 

#define EB1MAXVAL ((eb1)SCHAR_MAX)
#define EB1MINVAL ((eb1)        0)
#if defined(mips)                     
# ifndef lint
#  define UB1MAXVAL (UCHAR_MAX)
# endif
#endif
#ifndef UB1MAXVAL
# ifdef SCO_UNIX
# define UB1MAXVAL (UCHAR_MAX)
# else
# define UB1MAXVAL ((ub1)UCHAR_MAX)
# endif 
#endif
#define UB1MINVAL ((ub1)        0)
#define SB1MAXVAL ((sb1)SCHAR_MAX)
#define SB1MINVAL ((sb1)SCHAR_MIN)
#define MINEB1MAXVAL ((eb1)  127)
#define MAXEB1MINVAL ((eb1)    0)
#define MINUB1MAXVAL ((ub1)  255)
#define MAXUB1MINVAL ((ub1)    0)
#define MINSB1MAXVAL ((sb1)  127)
#define MAXSB1MINVAL ((sb1) -127)

#define UB1BITS          CHAR_BIT
#define UB1MASK          ((1 << ((uword)CHAR_BIT)) - 1)


typedef  unsigned char text;


#ifndef lint
typedef          short    eb2;               
typedef unsigned short    ub2;               
typedef   signed short    sb2;               
#else
#define eb2  short
#define ub2  unsigned short
#define sb2  signed short
#endif 

#define EB2MAXVAL ((eb2) SHRT_MAX)
#define EB2MINVAL ((eb2)        0)
#define UB2MAXVAL ((ub2)USHRT_MAX)
#define UB2MINVAL ((ub2)        0)
#define SB2MAXVAL ((sb2) SHRT_MAX)
#define SB2MINVAL ((sb2) SHRT_MIN)
#define MINEB2MAXVAL ((eb2) 32767)
#define MAXEB2MINVAL ((eb2)     0)
#define MINUB2MAXVAL ((ub2) 65535)
#define MAXUB2MINVAL ((ub2)     0)
#define MINSB2MAXVAL ((sb2) 32767)
#define MAXSB2MINVAL ((sb2)-32767)

#if defined(A_OSF)

#ifndef lint
typedef          int  eb4;                  
typedef unsigned int  ub4;                  
typedef   signed int  sb4;                  
#else
#define eb4 int
#define ub4 unsigned int
#define sb4 signed int
#endif 

#define EB4MAXVAL ((eb4)  INT_MAX)
#define EB4MINVAL ((eb4)        0)
#define UB4MAXVAL ((ub4) UINT_MAX)
#define UB4MINVAL ((ub4)        0)
#define SB4MAXVAL ((sb4)  INT_MAX)
#define SB4MINVAL ((sb4)  INT_MIN)
#define MINEB4MAXVAL ((eb4) 2147483647)
#define MAXEB4MINVAL ((eb4)          0)
#define MINUB4MAXVAL ((ub4) 4294967295)
#define MAXUB4MINVAL ((ub4)          0)
#define MINSB4MAXVAL ((sb4) 2147483647)
#define MAXSB4MINVAL ((sb4)-2147483647)

#else  

 
#ifndef lint
typedef          long  eb4;                  
typedef unsigned long  ub4;                  
typedef   signed long  sb4;                  
#else
#define eb4 long
#define ub4 unsigned long
#define sb4 signed long
#endif 

#define EB4MAXVAL ((eb4) LONG_MAX)
#define EB4MINVAL ((eb4)        0)
#define UB4MAXVAL ((ub4)ULONG_MAX)
#define UB4MINVAL ((ub4)        0)
#define SB4MAXVAL ((sb4) LONG_MAX)
#define SB4MINVAL ((sb4) LONG_MIN)
#define MINEB4MAXVAL ((eb4) 2147483647)
#define MAXEB4MINVAL ((eb4)          0)
#define MINUB4MAXVAL ((ub4) 4294967295)
#define MAXUB4MINVAL ((ub4)          0)
#define MINSB4MAXVAL ((sb4) 2147483647)
#define MAXSB4MINVAL ((sb4)-2147483647)
#endif 

#ifndef lint
typedef unsigned long  ubig_ora;             
typedef   signed long  sbig_ora;             
#else
#define ubig_ora unsigned long
#define sbig_ora signed long
#endif 

#define UBIG_ORAMAXVAL ((ubig_ora)ULONG_MAX)
#define UBIG_ORAMINVAL ((ubig_ora)        0)
#define SBIG_ORAMAXVAL ((sbig_ora) LONG_MAX)
#define SBIG_ORAMINVAL ((sbig_ora) LONG_MIN)
#define MINUBIG_ORAMAXVAL ((ubig_ora) 4294967295)
#define MAXUBIG_ORAMINVAL ((ubig_ora)          0)
#define MINSBIG_ORAMAXVAL ((sbig_ora) 2147483647)
#define MAXSBIG_ORAMINVAL ((sbig_ora)-2147483647)






#ifndef SYSV_386
#define SLU8NATIVE
#define SLS8NATIVE
#endif 

#ifdef SLU8NATIVE



/* IDC Added by Shuvro during 734 build*/

/*#ifndef lint
typedef unsigned long long ub8;
#else
#define ub8 unsigned long long
#endif */

#define UB8ZERO      ((ub8)0)

#define UB8MINVAL    ((ub8)0)
#define UB8MAXVAL    ((ub8)18446744073709551615)

#define MAXUB8MINVAL ((ub8)0)
#define MINUB8MAXVAL ((ub8)18446744073709551615)

#endif 


#ifdef SLS8NATIVE




/* IDC Added by Shuvro during 734 build*/
/*#ifndef lint
typedef signed long long sb8;
#else
#define sb8 signed long long
#endif */

#define SB8ZERO      ((sb8)0)

#define SB8MINVAL    ((sb8)-9223372036854775808)
#define SB8MAXVAL    ((sb8) 9223372036854775807)

#define MAXSB8MINVAL ((sb8)-9223372036854775807)
#define MINSB8MAXVAL ((sb8) 9223372036854775807)

#endif 





#undef CONST

#ifdef _olint
# define CONST const
#else
#if defined(PMAX) && defined(__STDC__)
#   define CONST const
#else
# ifdef M88OPEN
#  define CONST const
# else 
#  if defined(SEQ_PSX) && defined(__STDC__)
#    define CONST const
#  else 
#    ifdef A_OSF
#      if defined(__STDC__)
#        define CONST const
#      else
#        define CONST
#      endif
#    else
#      define CONST
#    endif 
#  endif 
# endif 
#endif 
#endif 




#ifdef lint
# define dvoid void
#else

# ifdef UTS2
#  define dvoid char
# else
# define dvoid void
# endif 

#endif 



typedef void (*lgenfp_t)(/*_ void _*/);




#ifndef ORASYSTYPES
# include <sys/types.h>
# define ORASYSTYPES
#endif 
#define boolean int




#ifdef sparc
# define SIZE_TMAXVAL SB4MAXVAL                
#else
# define SIZE_TMAXVAL UB4MAXVAL              
#endif 

#define MINSIZE_TMAXVAL (size_t)65535


#endif 

