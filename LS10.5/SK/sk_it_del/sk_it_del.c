/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_it_del.c   )                                  |
|  Program Desc  : ( ithr/itln deletion program.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  ithr,itln ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 01/07/91         |
|---------------------------------------------------------------------|
|  Date Modified : (10/10/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (12/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (11/09/97)      | Modified  by  : Roanna Marcelino |
|                                                                     |
|  Comments      : (10/10/91) - Print usage if argc incorrect. Use    |
|                : dsp_screen & dsp_process.                          |
|  (12/04/94)    : PSL 10673 - Online conversion                      |
|                :                                                    |
|                                                                     |
| $Log: sk_it_del.c,v $
| Revision 5.2  2001/08/09 09:18:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:08  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:09  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:01  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/22 07:00:10  scott
| Updated to ensure itff is deleted.
|
| Revision 1.8  1999/11/03 07:32:05  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.7  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.6  1999/10/08 05:32:28  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:20:09  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: sk_it_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_it_del/sk_it_del.c,v 5.2 2001/08/09 09:18:50 scott Exp $";

#define	MOD 10
#define	NO_SCRGEN
#include <pslscr.h>		
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include "schema"

#define	LN_DEL	       ((itln_rec.status[0]  == 'D') || \
						 (itln_rec.qty_order ==	0.0 && \
						 itln_rec.qty_border ==	0.0 && \
						 itln_rec.qty_rec	 ==	0.0 ))
#define	HR_DEL	       ((local_rec.lsystemDate - ithr_rec.rec_date) > days_old)

	struct	ithrRecord	ithr_rec;
	struct	itlnRecord	itln_rec;
	struct	itffRecord	itff_rec;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
} local_rec;

int	days_old;

/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  heading (int scn);
void ProcessIthr (void);
void DeleteItff 	(long);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 2)
	{
		print_at(0,0,mlSkMess110, argv[0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();
	
	days_old = atoi(argv[1]);

	OpenDB();

	ProcessIthr();

	shutdown_prog();   
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen("data");
	open_rec ("ithr",ithr_list,ITHR_NO_FIELDS,"ithr_hhit_hash");
	open_rec ("itln",itln_list,ITLN_NO_FIELDS,"itln_id_no");
	open_rec ("itff",itff_list,ITFF_NO_FIELDS,"itff_itff_hash");
}

void
CloseDB (
 void)
{
	abc_fclose("ithr");
	abc_fclose("itln");
	abc_fclose("itff");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
    return (EXIT_SUCCESS);        
}

int
heading (
 int scn)
{
    return (EXIT_SUCCESS);        
}

void
ProcessIthr (
 void)
{
	int	lcl_cc;
	int 	ANY_LEFT;
	char	tmp_del_no[9];

	init_scr();
	dsp_screen("Deleting ithr/itln Records ", " ", " ");

	cc = find_hash("ithr", &ithr_rec, GTEQ, "u", 0L);
	while (!cc)
	{
		sprintf(tmp_del_no, "%8ld", ithr_rec.del_no);
		dsp_process("Docket Number : ", tmp_del_no);

		ANY_LEFT = FALSE;
		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = 0;
		lcl_cc = find_rec("itln", &itln_rec, GTEQ, "r");
		while (!lcl_cc && itln_rec.hhit_hash == ithr_rec.hhit_hash)
		{
			if (LN_DEL && HR_DEL)
			{
				DeleteItff (itln_rec.itff_hash);

				abc_delete("itln");
				lcl_cc = find_rec("itln", &itln_rec, GTEQ, "r");
			}
			else
			{
				ANY_LEFT = TRUE;
				lcl_cc = find_rec("itln", &itln_rec, NEXT, "r");
			}
		}

		if (!ANY_LEFT)
		{
			abc_delete("ithr");
			cc = find_hash("ithr", &ithr_rec, GTEQ, "u", 0L);
		}
		else
		{
			abc_unlock ("ithr");
			cc = find_hash("ithr", &ithr_rec, NEXT, "u", 0L);
		}
	}

	abc_unlock ("ithr");
}

void 
DeleteItff (
	long	itffHash)
{
	itff_rec.itff_hash	=	itffHash;
	cc = find_rec ("itff", &itff_rec, GTEQ, "r");
	while (!cc && itff_rec.itff_hash == itffHash)
	{
		abc_delete ("itff");
		itff_rec.itff_hash	=	itffHash;
		cc = find_rec ("itff", &itff_rec, GTEQ, "r");
	}
}
