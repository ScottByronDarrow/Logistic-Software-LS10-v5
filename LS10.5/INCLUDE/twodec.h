/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Header Name   : ( twodec.h                    )                    |
|  Header Desc   : ( To enable rounding of double(s) to the user  )   |
|                  ( specified precision.                         )   |
|---------------------------------------------------------------------|
|  Date Written  : (  .  .  )      | Author       : Unknown           |
|---------------------------------------------------------------------|
|  Date Modified : (09/02/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (03/09/93)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|  (09/02/93)    : FULL REWRITE (Because of the new psl_round)        |
|  (03/09/93)    : Moved code to psl_round.c                          |
|                                                                     |
=====================================================================*/
#ifndef	TWODEC_H
#define	TWODEC_H

extern double	psl_round (double, int),
				fourdec (double),
				threedec (double),
				twodec (double),
				onedec (double),
				no_dec (double),
				n_dec (double, int);

#endif	/*TWODEC_H*/
