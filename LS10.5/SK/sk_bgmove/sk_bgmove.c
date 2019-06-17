/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_bgmove.c,v 5.2 2001/08/09 09:18:10 scott Exp $
|  Program Name  : ( sk_bgmove.c  )                                   |
|  Program Desc  : ( Stock background Movement update program       ) |
|---------------------------------------------------------------------|
|  Date Written  : (29/11/1997)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: sk_bgmove.c,v $
| Revision 5.2  2001/08/09 09:18:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:42  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/06 10:07:40  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
| Revision 3.0  2000/10/10 12:19:49  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/10 09:30:49  scott
| Updated to add operator, time and date stamp to audit file inaf.
|
| Revision 2.0  2000/07/15 09:10:25  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  2000/07/10 01:52:49  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.8  2000/01/28 06:36:23  scott
| General cleanup while looking for possible core dump on IED. Nothing found.
|
| Revision 1.7  1999/10/12 21:20:29  scott
| Updated by Gerry from ansi project.
|
| Revision 1.6  1999/10/08 05:32:14  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/09/30 23:44:54  scott
| Updated to use intr_id_no2 instead of intr_id_no
|
| Revision 1.4  1999/06/20 05:19:47  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_bgmove.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_bgmove/sk_bgmove.c,v 5.2 2001/08/09 09:18:10 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<twodec.h>
#include	<signal.h>
#include	<alarm_time.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	char	storeMonth [3];
 
#include	"schema"

struct BG_bproRecord	bpro_rec;
struct BG_inmuRecord	inmu_rec;
struct BG_intrRecord	intr_rec;
struct BG_inafRecord	inaf_rec;
struct BG_moveRecord	move_rec;

	char	program [15];


/*=======================
| Function Declarations |
=======================*/ 
void	TranProcess 	(void);
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void	CloseDB 		(void);
void	ProcessFile 	(void);

/*==========================
| Main Processing Routine. |
==========================*/ 
int
main (
 int argc, 
 char * argv [])
{

	int		found = 0;
	pid_t	pid = getpid ();
	char	*sptr;

	OpenDB ();

	sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	sprintf (bpro_rec.program, 		"%-14.14s", argv [0]);
	sprintf (program, 				"%-14.14s", argv [0]);

	cc = find_rec (BG_bpro, &bpro_rec, GTEQ, "u");

	while (!cc && !strcmp (bpro_rec.program, program))
	{
		abc_unlock (BG_bpro);
		if (bpro_rec.pid != 0 && bpro_rec.pid == pid)
		{
			found = 1;
			break;
		}

		cc = find_rec (BG_bpro, &bpro_rec, NEXT, "u");
	}
	abc_unlock (BG_bpro);

	if (!found)
	{
		strcpy (bpro_rec.co_no, "  ");
		strcpy (bpro_rec.br_no, "  ");
		strcpy (bpro_rec.program, program);
		bpro_rec.hash = 0L;
		bpro_rec.pid = 0;
	}
	abc_fclose (BG_bpro);

	TranProcess ();

	shutdown_prog ();
	
	return (EXIT_SUCCESS);
}

/*===============
| Process file. | 
===============*/
void
TranProcess (
 void)
{
	/*------------------------------
	| Set Signal Catching Routine. |
	-------------------------------*/
	signal_on ();

	/*--------------------------
	| Process Until Prog Exit. |
	--------------------------*/
	while (!prog_exit)
	{
		/*----------------------------
		| Initialise to bpro record. |
		----------------------------*/
		move_rec.move_hash	=	0L;
		cc = find_rec (BG_move, &move_rec, GTEQ, "u");
		while (!prog_exit && !cc)
		{
			ProcessFile ();

			/*---------------------------
			| Delete Current record.	|
			---------------------------*/
			cc = abc_delete (BG_move);
			if (cc)
				break;

			move_rec.move_hash	=	0L;
			cc = find_rec (BG_move, &move_rec, GTEQ, "u");
		}
		abc_unlock (BG_move);

		/*-----------------------------------------------
		| Ran out of records to Process so start timing |
		-----------------------------------------------*/
		time_out ();
	}
}

/* what's this doing here? it's not called at all! */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec (BG_bpro, BG_bpro_list, BPRO_NO_FIELDS, "bpro_program");
	open_rec (BG_move, BG_move_list, MOVE_NO_FIELDS, "move_move_hash");
	open_rec (BG_inmu, BG_inmu_list, INMU_NO_FIELDS, "inmu_id_no");
	open_rec (BG_intr, BG_intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (BG_inaf, BG_inaf_list, INAF_NO_FIELDS, "inaf_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (BG_bpro);
	abc_fclose (BG_move);
	abc_fclose (BG_inmu);
	abc_fclose (BG_inaf);
	abc_fclose (BG_intr);
	abc_dbclose ("data");
}

/*======================================================
| Process whole work file and update inmu accordingly. |
======================================================*/
void
ProcessFile (
 void)
{
	int		new_record = 0;
	int		monthNum;

	double	wk_val1 = 0.00,
			wk_val2 = 0.00;

	wk_val1 = twodec (move_rec.qty * move_rec.cost_price);
	wk_val2 = twodec (move_rec.qty * move_rec.sale_price);

	if (wk_val1 == 0.00 && wk_val2 == 0.00 && move_rec.qty == 0.0)
		return;

	DateToDMY (move_rec.date_tran, NULL, &monthNum, NULL);
	sprintf (storeMonth, "%02d", monthNum);

	strcpy (inmu_rec.co_no, move_rec.co_no);
	strcpy (inmu_rec.br_no, move_rec.br_no);
	strcpy (inmu_rec.wh_no, move_rec.wh_no);
	strcpy (inmu_rec.inmu_class, move_rec.move_class);
	strcpy (inmu_rec.category, move_rec.category);
	strcpy (inmu_rec.year, "C");
	strcpy (inmu_rec.period, storeMonth);
		
	/*--------------------------------------------
	| Get and update or Add Inventory Movements  |
	| Transactions record.                       |
	--------------------------------------------*/
	new_record = find_rec (BG_inmu, &inmu_rec, COMPARISON, "u");
	if (new_record)
	{
		inmu_rec.trin_dly  = 0.00;	
		inmu_rec.trin_mty  = 0.00;
		inmu_rec.trin_qty  = 0.00;
		inmu_rec.trout_dly = 0.00;
		inmu_rec.trout_mty = 0.00;
		inmu_rec.trout_qty = 0.00;
		inmu_rec.pur_dly   = 0.00;
		inmu_rec.pur_mty   = 0.00;
		inmu_rec.pur_qty   = 0.00;
		inmu_rec.sal_dly   = 0.00;
		inmu_rec.sal_mty   = 0.00;
		inmu_rec.sal_qty   = 0.00;
		inmu_rec.icst_dly  = 0.00;
		inmu_rec.icst_mty  = 0.00;
		inmu_rec.crd_dly   = 0.00;
		inmu_rec.crd_mty   = 0.00;
		inmu_rec.crd_qty   = 0.00;
		inmu_rec.ccst_dly  = 0.00;
		inmu_rec.ccst_mty  = 0.00;
	}

	switch (move_rec.type_tran)
	{
		/*-----------------------------------
		| Add Supplier Backlog Transaction. |
		-----------------------------------*/
			case 1:
			inmu_rec.pur_dly += wk_val1; 
			inmu_rec.pur_mty += wk_val1;
			inmu_rec.pur_qty += move_rec.qty;
			break;

		/*--------------------------------
		| Add Stock receipt Transaction. |
		--------------------------------*/
			case 2:
			inmu_rec.trin_dly += wk_val1;
			inmu_rec.trin_mty += wk_val1;
			inmu_rec.trin_qty += move_rec.qty;
			break;

		/*------------------------------
		| Add Stock issue Transaction. |
		------------------------------*/
			case 3:
			inmu_rec.trout_dly += wk_val1;
			inmu_rec.trout_mty += wk_val1;
			inmu_rec.trout_qty += move_rec.qty;
			break;

		/*--------------------------------
		| Add Stock balance Transaction. |
		--------------------------------*/
			case 4:

			break;

		/*---------------------------------
		| Add Stock purchase Transaction. |
		---------------------------------*/
			case 5:
			inmu_rec.pur_dly  += wk_val1;
			inmu_rec.pur_mty  += wk_val1;
			inmu_rec.pur_qty  += move_rec.qty;
			break;
	 
		/*--------------------------
		| Add Invoice Transaction. |
		--------------------------*/
			case 6:
			inmu_rec.sal_dly  += wk_val2;
			inmu_rec.sal_mty  += wk_val2;
			inmu_rec.sal_qty  += move_rec.qty;
			inmu_rec.icst_dly += wk_val1;
			inmu_rec.icst_mty += wk_val1;
			break;

		/*-------------------------
		| Add credit Transaction. |
		-------------------------*/
			case 7:
			inmu_rec.crd_dly  += wk_val2;
			inmu_rec.crd_mty  += wk_val2;
			inmu_rec.crd_qty  += move_rec.qty;
			inmu_rec.ccst_dly += wk_val1;
			inmu_rec.ccst_mty += wk_val1;
			break;
	}

	/*------------------------
	| Not on file so create. |
	------------------------*/
	if (new_record)
	{
		strcpy (inmu_rec.stat_flag,"0");

		cc = abc_add (BG_inmu,&inmu_rec);
		if (cc) 
			file_err (cc, BG_inmu, "DBADD");

		abc_unlock (BG_inmu);
	}
	else 
	{
		cc = abc_update (BG_inmu,&inmu_rec);
		if (cc) 
			file_err (cc, BG_inmu, "DBUPDATE");
	}
	/*----------------------------
	| Add inventory transaction. |
	----------------------------*/
	strcpy (intr_rec.co_no, 	move_rec.co_no);
	strcpy (intr_rec.br_no, 	move_rec.br_no);
	intr_rec.hhbr_hash 		= 	move_rec.hhbr_hash;
	intr_rec.hhcc_hash 		= 	move_rec.hhcc_hash;
	intr_rec.hhum_hash 		= 	move_rec.hhum_hash;
	intr_rec.type 			= 	move_rec.type_tran;
	intr_rec.date 			= 	move_rec.date_tran;
	strcpy (intr_rec.batch_no, 	move_rec.batch_no);
	strcpy (intr_rec.ref1, 		move_rec.ref1);
	strcpy (intr_rec.ref2, 		move_rec.ref2);
	intr_rec.qty 			= 	move_rec.qty;
	intr_rec.cost_price 	= 	move_rec.cost_price;
	intr_rec.sale_price 	= 	move_rec.sale_price;
	strcpy (intr_rec.stat_flag, "0");
	cc = abc_add (BG_intr,&intr_rec);
	if (cc) 
	   file_err (cc, BG_intr, "DBADD");

	/*------------------------------
	| Add Audit trail transaction. |
	------------------------------*/
	strcpy (inaf_rec.co_no, 	move_rec.co_no);
	strcpy (inaf_rec.br_no, 	move_rec.br_no);
	strcpy (inaf_rec.wh_no, 	move_rec.wh_no);
	inaf_rec.sys_date 		= 	TodaysDate ();
	inaf_rec.hhbr_hash 		= 	move_rec.hhbr_hash;
	inaf_rec.hhcc_hash 		= 	move_rec.hhcc_hash;
	inaf_rec.hhum_hash 		= 	move_rec.hhum_hash;
	inaf_rec.type			= 	move_rec.type_tran;
	inaf_rec.date 			= 	move_rec.date_tran;
	strcpy (inaf_rec.batch_no,	move_rec.batch_no);
	strcpy (inaf_rec.ref1, 		move_rec.ref1);
	strcpy (inaf_rec.ref2, 		move_rec.ref2);
	inaf_rec.qty 			= 	move_rec.qty;
	inaf_rec.cost_price 	= 	move_rec.cost_price;
	inaf_rec.sale_price 	= 	move_rec.sale_price;
	inaf_rec.date_create	=	move_rec.date_create;
	strcpy (inaf_rec.op_id, 	move_rec.op_id);
	strcpy (inaf_rec.time_create, 	move_rec.time_create);
	strcpy (inaf_rec.stat_flag, "0");
	cc = abc_add (BG_inaf,&inaf_rec);
	if (cc) 
		file_err (cc, BG_inaf, "DBADD");

	return;
}
