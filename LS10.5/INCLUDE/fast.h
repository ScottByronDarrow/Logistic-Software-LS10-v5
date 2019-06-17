/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  File Name     : ( Fast.h         )                                 |
|  File Desc     : ( Fast Access Maintenance Structure.           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (30/08/88)      | Modified  by  : Roger Gibbison.  |
|                                                                     |
|  Comments      :                                                    |
|                                                                     |
=====================================================================*/

#define _isdigit(x)     (x >= '0' && x <= '9')
#define _islower(x)     (x >= 'a' && x <= 'z')
#define _isupper(x)     (x >= 'A' && x <= 'Z')
#define to_upper(x)     ((_islower(x)) ? (x - ('a' - 'A')) : x)

#define FASTLEN         (6)

/*
 * Fast Access User
 */
typedef	struct
{
	char	_f_code [FASTLEN + 1]; /* Fast Access Base ie DBM, DBR */
	char	_m_file [15];	      /* Menu Data File ie db_master.mdf */
	int	_n_opts;	      /* No. Options Avail. to User on menu */
} FASTSTRUC;

#ifndef	FAST
extern	char	currentMenu [];
#endif
