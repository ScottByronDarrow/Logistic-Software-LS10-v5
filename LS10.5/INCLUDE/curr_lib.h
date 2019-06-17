/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( curr_lib.h )                                     |
|  Program Desc  : ( Currency Library Header File )                   |
|=====================================================================|
|  Library sources for Currency.                                      |
|  This library aimed at encapsulate references to pocr table.        |
|  Programmer who use this library should use read_currency to        |
|  instance the currency record first and then use member function    |
|  to access currency information.                                    |
|  Programs use curr_lib to access pocr should not touch pocr in your |
|  programs because curr_lib will open and close pocr which will      |
|  crash your program if your program opened pocr.                    |
=====================================================================*/
#ifndef _CURR_LIB_H_
#define _CURR_LIB_H_

#ifdef CCMAIN
#	define	SCOPE extern
#else
#	define	SCOPE
#endif

SCOPE	int 	read_currency	(char *, char *);
SCOPE	char 	*curr_code	(void);
SCOPE	char 	*curr_desc	(void);
SCOPE	char 	*curr_prime_unit(void);
SCOPE	char 	*curr_sub_unit	(void);
SCOPE	double 	curr_exch_rate	(void);
SCOPE	Date 	curr_ldate_up	(void);
SCOPE	char 	*curr_ctrl_acct	(void);
SCOPE	char 	*curr_exch_var	(void);
SCOPE	char 	curr_operator	(void);
SCOPE	char 	curr_stat_flag	(void);
SCOPE	double 	curr_fx_amt	(double);
SCOPE	double 	curr_loc_amt	(double);
SCOPE	int	curr_srch_pocr	(char *, char *, char *);
SCOPE	void	curr_set_exch_rate (double);

#endif
