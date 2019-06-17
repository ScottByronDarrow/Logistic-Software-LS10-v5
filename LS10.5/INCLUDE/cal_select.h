/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name : ( cal_select.h )                                    |
|  Program Desc : ( Definitions for cal_select.c                )     |
|               : (                                             )     |
|---------------------------------------------------------------------|
|  Author       : Campbell Mander.                                    |
|  Date Written : (07/12/91)                                          |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by :                   |
|                :                                                    |
|  Comments      : (  /  /  ) -                                       |
|                :                                                    |
|                :                                                    |
=====================================================================*/
#ifndef	CAL_SELECT_H
#define	CAL_SELECT_H

#ifndef	MAX_MTHS
#define	MAX_MTHS	128
#endif

#define	C_NORM	0
#define	C_GRID	1
#define	C_INFO	2

#define	BK_MTH		0
#define	FD_MTH		1

int	FD_INFNTY;	/* TRUE if endless increasing calendar OK */
int	BK_INFNTY;	/* TRUE if endless decreasing calendar OK */

struct	CAL_STORE {
	int	no_days;
	int	day_first;
	long	date_first;
	struct	CAL_STORE  *prev;
	struct	CAL_STORE  *next;
};

struct	CAL_INFO {
	long	info_date;
	char	*info;
	struct	CAL_INFO  *head;
	struct	CAL_INFO  *next;
};


#define	CAL_NULL ((struct CAL_STORE *) NULL)
#define	INFO_NULL ((struct CAL_INFO *) NULL)

#endif	/* CAL_SELECT_H */
