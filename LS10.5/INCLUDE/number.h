/*=====================================================================
|  Copyright (C) 2000 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: number.h,v 5.2 2001/11/26 00:45:01 scott Exp $
|=====================================================================|
|  Program Name  : (number.h)
|  Program Desc  : (This describes a semi-arbitrary precision number 
|					implementation as well as the operations that can 
|					be performed using the implemtation)
|=====================================================================|
| $Log: number.h,v $
| Revision 5.2  2001/11/26 00:45:01  scott
| Changes program name.
|
| Revision 5.1  2001/11/16 02:26:43  cha
| Updated to free ourselves from Informix's 32 bit number implementation.
| We have implemented  the equivalent of Informix's decimal functions.
|
|
=====================================================================*/

#ifndef	NUMBER_H
#define	NUMBER_H

/*
 * We're using Informix's 32-digit implementation for the time being
 */
#ifdef ORA734
	#include	<decimalLS.h>
	typedef	_dec_t	number;		/* recast type to ours */
#else
	#include	<decimal.h>
	typedef	dec_t	number;		/* recast type to ours */
#endif /*ORA734*/
#endif	/*NUMBER_H*/
