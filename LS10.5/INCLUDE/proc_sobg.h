#ifndef	PROC_SOBG_H
#define	PROC_SOBG_H
/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
| Program Name : ( proc_sobg.h )                                      |
| Program Desc : ( Includes code for processing sobg records.   )     |
|                (                                              )     |
|---------------------------------------------------------------------|
|  Date Written  : (13/03/91)      | Author      : Trevor van Bremen  |
|---------------------------------------------------------------------|
|  Date Modified : (xx/xx/91)      | Modified by : Scott Darrow.      |
|  Date Modified : (16/06/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (07/07/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (07/02/95)      | Modified by : Basil Wood         |
|                                                                     |
|  Comments      : (xx/xx/91) -                                       |
|  (16/06/93)    : PSL 9086. Remove malloc.h                          |
|  (07/07/93)    : PSL 8948 Moved the code to library module          |
|  (07/02/95)    : PSM-V9 11674. Add add_hash_RC()                    |
|                :                                                    |
|                                                                     |
=====================================================================*/

/*--------------------------------
| Function declarations and/or prototypes
----------------------------------*/

extern void	add_hash (char *co_no, char *br_no, char *type, 
					  int lpno,
					  long hhbr_hash, long hhcc_hash, long pid,
					  double value);
extern void	add_hash_RC (char *co_no, char *br_no, 
					  long hhbr_hash, long hhcc_hash, long pid,
					  int last_line);
extern void	recalc_sobg (void);

#endif	/*PROC_SOBG_H*/
