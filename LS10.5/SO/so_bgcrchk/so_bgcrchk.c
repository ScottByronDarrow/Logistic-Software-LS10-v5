/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_bgcrchk.c,v 5.3 2001/10/23 07:16:33 scott Exp $
|  Program Name  : (so_bgcrchk.c)
|  Program Desc  : (Background Credit Check of Packing Slips)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 26/10/88         |
|---------------------------------------------------------------------|
| $Log: so_bgcrchk.c,v $
| Revision 5.3  2001/10/23 07:16:33  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_bgcrchk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bgcrchk/so_bgcrchk.c,v 5.3 2001/10/23 07:16:33 scott Exp $";

#include 	<pslscr.h>	
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>

#define	CRD_OK  	0
#define	STOP_CREDIT	1
#define	OVER_LIMIT	2
#define	OVER_PERIOD	3

#define	COMMENT_LINE	 (inmr_rec.inmr_class [0] == 'Z')
#define	CREDIT_HOLD	 (soln_rec.status [0] == 'C')

#define	SCHED_ORD	 (sohr_rec.sch_ord [0] == 'Y')

#define	F_SUPP	 (sohr_rec.full_supply [0] == 'Y')

	int	c_ords = 0;
	int	hold_comments = 0;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data";

	int	lpno = 0;
	int		envVarDbNettUsed 	= TRUE;

	double	total_owing = 0.00;

#include	<proc_sobg.h>
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	ReadMisc 		(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Process 		(void);
void 	ProcessSohr 	(long);
double 	CalulateLine 	(void);
int  	CheckCumr 		(double);
void 	HoldSoln 		(long);
void 	ProcessSoln 	(long);
int  	GetItem 		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	sptr = chk_env ("CON_ORDERS");
	c_ords = (sptr == (char *)0) ? FALSE : atoi (sptr);

	
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*----------------------------------
	| Check if Comment lines are held. |
	----------------------------------*/
	sptr = chk_env ("SO_COMM_HOLD");
	hold_comments = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (argc > 1)
		lpno = atoi (argv [1]);
	
	OpenDB ();

	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no3");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
 }

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_dbclose (data);
}

/*===============================
===============================*/
void
Process (
 void)
{
	
	dsp_screen ("Credit Check Released Orders.",comm_rec.co_no,
						comm_rec.co_name);
	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	strcpy (sohr_rec.status, "C");
	sohr_rec.hhcu_hash = 0L;
	sohr_rec.hhso_hash = 0L;
	cc = find_rec (sohr,&sohr_rec,GTEQ,"u");
	/*-----------------------
	| Read sohr records	|
	-----------------------*/
	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no) &&
		      !strcmp (sohr_rec.br_no,comm_rec.est_no) &&
		      sohr_rec.status [0] == 'C')
	{
		ProcessSohr (sohr_rec.hhso_hash);

		strcpy (sohr_rec.co_no,comm_rec.co_no);
		strcpy (sohr_rec.br_no,comm_rec.est_no);
		strcpy (sohr_rec.status, "C");
		sohr_rec.hhcu_hash = 0L;
		sohr_rec.hhso_hash = 0L;
		cc = find_rec (sohr,&sohr_rec,GTEQ,"u");
	}
	abc_unlock (sohr);
}

void
ProcessSohr (
 long hhso_hash)
{
	double	ord_total = 0.00;
	int	crd_type = 0;

	cumr_rec.hhcu_hash	= sohr_rec.hhcu_hash;
	cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
	if (cc)
	{
		abc_unlock (sohr);
		return;
	}
	dsp_process ("Order: ",sohr_rec.order_no);

	/*----------------------------------------------------------
	| Check value of order about to process.                   |
	| If going to take it over then place whole order on hold. |
	----------------------------------------------------------*/
	ord_total = 0.00;

	soln_rec.hhso_hash = hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhso_hash)
	{
		if (CREDIT_HOLD)
			ord_total += CalulateLine ();

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	crd_type = CheckCumr (ord_total);
	
	switch (crd_type)
	{
	case 	STOP_CREDIT:
	case	OVER_LIMIT:
	case	OVER_PERIOD:
		HoldSoln (hhso_hash);
		break;

	case 	CRD_OK:
		ProcessSoln (hhso_hash);
		break;

	default:
		break;
	}
	abc_unlock (sohr);
}

/*================================
| Calculate value of order line. |
================================*/
double	
CalulateLine (
 void)
{
	double	l_total		= 0.00,
			l_disc		= 0.00,
			line_val 	= 0.00;

	if ((soln_rec.qty_order + soln_rec.qty_bord) <= 0.00)
		return (0.00);

	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
		return (0.00);
	
	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	 (double) soln_rec.qty_order + soln_rec.qty_bord;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	 (double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);
			
		if (envVarDbNettUsed)
			line_val	=	l_total - l_disc;
		else
			line_val	=	l_total;
	}
	return (line_val);
}
/*==========================================
| Validate credit period and credit limit. |
==========================================*/
int
CheckCumr (
 double inc_amt)
{
	if (cumr_rec.crd_flag [0] == 'Y')
		return (CRD_OK);

	total_owing = (	cumr_rec.bo_current + 
		        	cumr_rec.bo_per1 + 
		        	cumr_rec.bo_per2 + 
		        	cumr_rec.bo_per3 + 
		        	cumr_rec.bo_per4 + 
		        	cumr_rec.bo_fwd + 
					cumr_rec.ord_value +
					inc_amt);

	if (cumr_rec.stop_credit [0] == 'Y')
		return (STOP_CREDIT);

	/*---------------------------------------------
	| Check if customer is over his credit limit. |
	---------------------------------------------*/
	if (cumr_rec.credit_limit <= total_owing && cumr_rec.credit_limit != 0.00)
		return (OVER_LIMIT);

	/*-----------------------
	| Check Credit Terms	|
	-----------------------*/
	if (cumr_rec.od_flag)
		return (OVER_PERIOD);
	
	return (CRD_OK);
}
/*========================================
| Order Lines and Header are to be Held. |
========================================*/
void
HoldSoln (
 long hhso_hash)
{
	int	ns_del = FALSE;
	int	held = 0;

	soln_rec.hhso_hash = hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");

	while (!cc && soln_rec.hhso_hash == hhso_hash)
	{
		cc = GetItem (soln_rec.hhbr_hash);
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		if (!COMMENT_LINE && CREDIT_HOLD)
			ns_del = TRUE;

		if (!COMMENT_LINE && !CREDIT_HOLD)
			ns_del = FALSE;
		
		if (COMMENT_LINE && ns_del)
		{
			if (!hold_comments)
			{
				abc_unlock (soln);
				abc_delete (soln);
				cc = find_rec (soln,&soln_rec,GTEQ,"u");
				continue;
			}
			strcpy (soln_rec.status, "C");
		}
		
		if (CREDIT_HOLD)
		{
			held = 1;
			strcpy (soln_rec.stat_flag, (c_ords) ? "M" : "R");
			if (SCHED_ORD)
				strcpy (soln_rec.stat_flag, "M");
			strcpy (soln_rec.status, "H");
			cc = abc_update (soln, &soln_rec);
			if (cc)
				file_err (cc, "soln", "DBUPDATE");
		}
		else
			abc_unlock (soln);

		cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	/*-----------------------------------------------
	| Some or all lines held so set header as held. |
	-----------------------------------------------*/
	strcpy (sohr_rec.stat_flag, (c_ords) ? "M" : "R");
	strcpy (sohr_rec.status,  (c_ords) ? "M" : "R");
	if (SCHED_ORD)
	{
		strcpy (sohr_rec.status,    "M");
		strcpy (sohr_rec.stat_flag, "M");
	}
	if (held)
		strcpy (sohr_rec.status,"H");
	
	cc = abc_update (sohr,&sohr_rec);
	if (cc)
		file_err (cc, "sohr", "DBUPDATE");

	if (sohr_rec.status [0] == 'R')
	{
		add_hash (comm_rec.co_no, 
				  comm_rec.est_no, 
				 (lpno) ? "PA" : "PC",
				  lpno, 
				  sohr_rec.hhso_hash, 
				  0L,
				  0L,
				 (double) 0.00);
	}
}
/*============================================
| Order Lines and Header are to be Released. |
============================================*/
void
ProcessSoln (
 long hhso_hash)
{
	int	ns_del = FALSE;

	soln_rec.hhso_hash = hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");

	while (!cc && soln_rec.hhso_hash == hhso_hash)
	{
		cc = GetItem (soln_rec.hhbr_hash);
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		if (!COMMENT_LINE && CREDIT_HOLD)
			ns_del = TRUE;

		if (!COMMENT_LINE && !CREDIT_HOLD)
			ns_del = FALSE;
		
		if (COMMENT_LINE && ns_del)
		{
			if (!hold_comments)
			{
				abc_unlock (soln);
				abc_delete (soln);
				cc = find_rec (soln,&soln_rec,GTEQ,"u");
				continue;
			}
			strcpy (soln_rec.status, "C");
		}
		if (!CREDIT_HOLD)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		strcpy (soln_rec.stat_flag, (c_ords || F_SUPP) ? "M" : "R");
		strcpy (soln_rec.status,  (c_ords || F_SUPP) ? "M" : "R");
		if (SCHED_ORD)
		{
			strcpy (soln_rec.stat_flag, "M");
			strcpy (soln_rec.status,    "M");
		}

		cc = abc_update (soln,&soln_rec);
		if (cc)
			file_err (cc, "soln", "DBUPDATE");
		
		cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	strcpy (sohr_rec.status,  (c_ords || F_SUPP) ? "M" : "R");
	strcpy (sohr_rec.stat_flag, (c_ords || F_SUPP) ? "M" : "R");
	if (SCHED_ORD)
	{
		strcpy (sohr_rec.status,    "M");
		strcpy (sohr_rec.stat_flag, "M");
	}

	cc = abc_update (sohr,&sohr_rec);
	if (cc)
		file_err (cc, "sohr", "DBUPDATE");
	
	/*----------------------------
	| Only Release is Automatic. |
	----------------------------*/
	if (sohr_rec.status [0] == 'R')
	{
		add_hash (comm_rec.co_no, 
				  comm_rec.est_no, 
				 (lpno) ? "PA" : "PC",
				  lpno, 
				  sohr_rec.hhso_hash, 
				  0L,
				  0L,
				 (double) 0.00);
	}
	add_hash (comm_rec.co_no, 
			  comm_rec.est_no,
			  "RO", 
			 (int) 0,
			  sohr_rec.hhcu_hash, 
			  0L, 
			  0L, 
			 (double) 0.00);
}

int
GetItem (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash = hhbrHash;
	return (find_rec (inmr, &inmr_rec, COMPARISON, "r"));
}
