/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( std_decs.h     )                                 |
|  Program Desc  : ( Standard Declarations for Large Model esp.   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Authors       : Roger Gibbison.                                    |
|  Date Written  : 29/12/87.                                          |
|---------------------------------------------------------------------|
|  Date Modified : (29/12/87)      | Modified by : Roger Gibbison.    |
|  Date Modified : (21/06/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (13/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (26/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (01/09/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (07.02.94)      | Modified by : Jonathan Chen      |
|                  (13.03.94)      | Modified by : Jonathan Chen      |
|                  (10.06.94)      | Modified by : Jonathan Chen      |
|                  (20.07.94)      | Modified by : Jonathan Chen      |
|                  (21.07.94)      | Modified by : Jonathan Chen      |
|                  (03.09.1999)    | Modified by : Eumir Que Camara   |
|                                                                     |
|  Comments      :                                                    |
|     (21/06/93) : Added :                                            |
|                     extern char *malloc()                           |
|                  declaration. This should be changed at some point  |
|                  in time to :                                       |
|                     #include <malloc.h>                             |
|     (13/08/93) : PSL 9513 uses <malloc.h>, and <string.h>           |
|     (26/08/93) : Added upshift(), downshift() declarations          |
|     (01/09/93) : Added exit()                                       |
|     (07.02.94) : APP 9690 Removed bad #def's for the SCO            |
|     (13.03.94) : Changes to enable compile on AIX                   |
|     (10.06.94) : Moved code out of header file                      |
|     (20.07.94) : Prototyped all functions defined within file       |
|     (21.07.94) : Removed obsolete prototype GET_PNAME               |
|     (03.09.99) : Added ltrim() prototype. 
-----------------------------------------------------------------------
	$Log: std_decs.h,v $
	Revision 5.3  2001/08/09 09:31:45  scott
	Updated to add FinishProgram () function
	
	Revision 5.2  2001/08/06 22:49:52  scott
	RELEASE 5.0
	
	Revision 5.1  2001/07/25 01:01:56  scott
	Updated for 10.5
	
	Revision 5.0  2001/06/19 06:51:49  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:59:35  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:29:03  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:15:52  gerry
	Force revision no. to 2.0 - Rel-15072000
	
	Revision 1.16  2000/06/15 02:26:59  scott
	Added include of FindSumr.h as per what was done for FindInmr.h
	
	Revision 1.15  2000/05/29 09:25:48  scott
	Updated to add new functions to move include files into library. Process will allow GVision to have more processes in the back end server.
	
	Revision 1.14  2000/02/07 04:53:58  scott
	Added sort_reopen (added by Trevor.)
	
	Revision 1.13  2000/01/11 08:45:19  scott
	Updated for new routine StockValue.
	
	Revision 1.12  2000/01/10 21:43:21  cam
	Modified for new sort API sort_reopen ().
	
	Revision 1.11  1999/12/06 01:22:23  scott
	Updated to change CTime () to SystemTime due to problems with Conflicts in VisualC++
	
	Revision 1.10  1999/11/25 03:47:04  jonc
	Updated CalcDueDate to use std date routines, tightened prototypes.
	
	Revision 1.9  1999/11/15 06:47:06  scott
	Updated for compile problems on AIX
	
	Revision 1.8  1999/10/26 05:29:55  scott
	Updated from code checking
	
	Revision 1.7  1999/10/08 03:37:36  jonc
	Removed prototypes for INFORMIX functions. Now defined in isam.h
	
=====================================================================*/
#ifndef	STD_DECS_H
#define	STD_DECS_H

#include	<assert.h>
#include	<errno.h>
#include	<string.h>
#include	<time.h>

#include	<osdefs.h>			/* OS Specific kludges */
#include	<ptypes.h>	
#include	<pDate.h>	
#include	<SystemTime.h>	
#include	<DateToString.h>	
#include	<FinancialDates.h>	
/*
#include	<StockValue.h>	
*/
#include	<FindInmr.h>	
#include	<FindSumr.h>	

/* TvB Start Extension */
#include	<stdio.h>
#include	<sys/types.h>
#include	<arralloc.h>
#include	<decimal.h>
#include	<dbio.h>
#include	<license2.h>
#include	<graph.h>
#include	<wild_card.h>
#include	<hot_keys.h>
#include	<tabdisp.h>
#include	<sys_log.h>
#include	<minimenu.h>
#include	<vars_def.h>
/*
#include	<glutils.h>
*/
#include	<stdarg.h>
#include	<tcap.h>
#include	<fcntl.h>
#include	<sys/wait.h>
#include	<sys/stat.h>
#include	<ctype.h>
#include	<dberr.h>
#include	<signal.h>
#include	<sys/ioctl.h>
#ifndef	HAS_MAJOR_T		
typedef	dev_t	major_t;
typedef	dev_t	minor_t;
#endif	
extern	char	*PNAME;		
#include	<TimeStrings.h>
#include	<ProtosBASE.h>
#include	<ProtosIF.h>
#include	<ProtosSCR.h>
#include	<ProtosINCLUDE.h>
/*
| Inclusions defining major () and minor () macros/functions :
| -	<sys/mkdev.h> is to be used whenever possible 'cos this is the preferred
|	standard. unfortunately this is not available on all platforms or
|	compile environments
| -	<sys/sysmacros.h> is the poor man's alternative, but this does not work
|	on machines which have <sys/mkdev.h>
*/
#ifdef	MAJOR_IN_SYSMKDEV
#include	<sys/mkdev.h>
#endif	/* MAJOR_IN_SYSMKDEV */
#ifdef	MAJOR_IN_SYSMACROS
#include	<sys/sysmacros.h>
#endif	/* MAJOR_IN_SYSMACROS */

/*
| Common std lib functions
*/
#ifdef	HAS_STDLIB_H

#include	<stdlib.h>

#else	/*HAS_STDLIB_H*/

extern char		*getenv (char *);
extern void		exit (int);
extern long		atol (char *);
extern double	atof (char *);

#endif	/*HAS_STDLIB_H*/

#ifdef	HAS_UNISTD_H

#include	<unistd.h>

#else	/*HAS_UNISTD_H*/

extern unsigned	sleep (unsigned);

#endif	/*HAS_UNISTD_H*/

/*
 *	Global variables: hangover from way, way back
 */
extern 	int	_cc, _wide;			/* LIB: glob_vars.c */
extern 	int		IN_STAKE;		/* LIB: stk_vars.c */
extern	int		BY_MODE;		/* LIB: stk_vars.c */
extern 	int		fifoError;		/* LIB: stk_vars.c */
extern 	long	STAKE_COFF;		/* LIB: stk_vars.c */
extern	float	fifoQtyShort;	/* LIB: stk_vars.c */

/*-------------------------------------------------
 * Dsp_utils variables
 * This should be placed in dsp_utils.h, however
 * due to the way it is structured it is more
 * advisable to declare it here.
 */
extern  int DSP_UTILS_lpno;


/*******************************************************************************
 *
 *	Provide prototypes
 *
 ******************************************************************************/
/*
 *	Logistic string functions prototypes
 */
extern 	char	*string 		(int, char *),
				*upshift 		(char *),
				*downshift 		(char *),
				*clip 			(char *),
				*lclip 			(char *),
	        	*ltrim 			(char *, char *),
	        	*rtrim 			(char *, char *),
				*expand 		(char *, char *),
				*mid 			(char *, int, int),	
				*p_strsave 		(char *),		
				*ML 			(char *),					
				*ml 			(char *),					
				*ttod 			(void),
				*ttoa 			(long, char *);

/*
 *	Logistic date functions prototypes
 */
extern	int		FullYear 		(void),
				lang_select 	(void),
				ValidItemUom 	(long, long);

extern long		TodaysDate 		(void),
				atot 			(char *);

/*
 *	Numeric routines
 */
extern char		*dbltow 	(double, char *, char *),
				*pad_num 	(char *),
				*zero_pad 	(char *, int),
				*pad_batch 	(char *);

extern float	rnd_mltpl 	(char *rnd_type, float mltpl, float qty);

/*
 *	Logistic environment functions prototypes
 */
extern int		open_env 		(void);
extern void		close_env 		(int);
extern char		*get_env 		(char *),
				*chk_env 		(char *);

extern void		put_env 		(char *, char *, char *);

/*
 *	Logistic sort functions
 */
extern FILE	*sort_open 			(char *);
extern int	sort_rewind 		(FILE *);
extern FILE	*sort_reopen 		(char *);
extern FILE	*sort_sort 			(FILE *, char *);
extern FILE	*dsort_sort 		(FILE *, char *);
extern char	*sort_read 			(FILE *);

/*
 *	G/L Application routines
 */
extern char		*comma_fmt 	(double, char *);
extern long		get_eoy 	(long, int),
				get_mend 	(long),
				get_mbeg 	(long);

/*
 *	Stock Application functions
 */
extern int		GenNextSerNo 	(char *, int, char *);	/* gen_ser_no.c */
extern long		alt_hash 		(long, long);		/* alt_hash.c */
extern double	out_cost 		(double, float);		/* out_cost.c */
extern	void	FinishProgram 	(void);

#endif	/*STD_DECS_H*/
