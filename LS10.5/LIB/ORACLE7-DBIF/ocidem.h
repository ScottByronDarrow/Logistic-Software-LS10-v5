/*
 * $Header: /usr/LS10/REPOSITORY/LS10.5/LIB/ORACLE7-DBIF/ocidem.h,v 5.0 2001/06/19 07:10:28 cha Exp $ 
 */

/* Copyright (c) 1991, 1996 by Oracle Corporation */
/*
   NAME
     ocidem.h - OCI demo header
   MODIFIED   (MM/DD/YY)
    surman     11/08/96 -  413362: Add SS_64BIT_SERVER macro
    emendez    04/07/94 -  merge changes from branch 1.6.710.1
    emendez    02/02/94 -  Fix for bug 157576
    jnlee      01/05/93 -  include oratypes.h once, make oci_func_tab static
    rkooi2     10/26/92 -  More portability mods 
    rkooi2     10/22/92 -  Change text back to char to avoid casts 
    rkooi2     10/20/92 -  Changes to make it portable 
    sjain      03/16/92 -  Creation 
*/

/*
 *  ocidem.h
 *
 *  Declares additional functions and data structures
 *  used in the OCI C sample programs.
 */

#ifndef ORATYPES
#include <oratypes.h>
#endif /* ORATYPES */

#ifndef OCIDEM
#define OCIDEM


/*  internal/external datatype codes */
#define VARCHAR2_TYPE            1
#define NUMBER_TYPE              2
#define INT_TYPE		 3
#define FLOAT_TYPE               4
#define STRING_TYPE              5
#define ROWID_TYPE              11
#define DATE_TYPE               12

/*  ORACLE error codes used in demonstration programs */
#define VAR_NOT_IN_LIST       1007
#define NO_DATA_FOUND         1403
#define NULL_VALUE_RETURNED   1405

/*  some SQL and OCI function codes */
#define FT_INSERT                3
#define FT_SELECT                4
#define FT_UPDATE                5
#define FT_DELETE                9

#define FC_OOPEN                14

/*
** Size of HDA area:
** 512 for 64 bit arquitectures
** 256 for 32 bit arquitectures
*/

#if (defined(__osf__) && defined(__alpha)) || defined(CRAY) || defined(KSR) || \
    defined(SS_64BIT_SERVER)
# define HDA_SIZE 512
#else
# define HDA_SIZE 256
#endif

#endif      /* OCIDEM */

