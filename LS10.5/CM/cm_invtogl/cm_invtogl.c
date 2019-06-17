/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_invtogl.c,v 5.2 2001/08/09 08:57:30 scott Exp $
|  Program Name  : (cm_invtogl.c)
|  Program Desc  : (Update General Ledger Work Transactions From)
|                (Invoices File (cuwk), Set tran_flag to 2)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cm_invtogl.c,v $
| Revision 5.2  2001/08/09 08:57:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:19  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_invtogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_invtogl/cm_invtogl.c,v 5.2 2001/08/09 08:57:30 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<ml_cm_mess.h>

#define		EXTERNAL	(cmhr_rec.internal [0] == 'N')
#define		FIXED    	(cmhr_rec.quote_type [0] == 'F')
#define		PROGRESS 	(cmhr_rec.progress [0] == 'Y')
#define		CLOSED   	(cmhr_rec.status [0] == 'C')

	/*================================================================
	| special fields and flags  ################################## . |
	================================================================*/
   	int		_nodata = FALSE;

   	char 	branchNo [3],
			work_type [2],
   			systemDate [11],
    		prev_inv [9];
	int		mnth;

						/*----------------------------------*/
	double	dbt_amt;    /* Holds Customer Amount. 	  		*/
	double	gst_amt;    /* Holds Customer G.S.T. 	  		*/
	long	per_date;   /* Holds Correct date for period.   */
	double	tot_cost;	/* Total of costs		  			*/
	double	tot_sale;	/* Total of sales		  			*/
	double	prev_amt;	/* Total of sales		 			*/
						/*----------------------------------*/

#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct cmhrRecord	cmhr_rec;
struct cmcbRecord	cmcb_rec;
struct cmpbRecord	cmpb_rec;
struct cmpbRecord	cmpb2_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct cuwkRecord	cuwk_rec;

	char	*data	= "data",
			*cmpb2	= "cmpb2";

	char	loc_curr [4];
	int		printerNo;

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			(void);
void	CloseDB			(void);
void	update			(void);
void	WriteGlwk		(void);
void	PostExternal	(void);
void	shutdown_prog	(void);
void	SumCmcbs		(void);
void	GetPrvInv		(void);
void	SumColns		(void);
void	ReverseWip		(void);
void	PostVariance	(void);
void	PostInternal	(void);


int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 2)
	{
		print_at (0, 0, ML (mlCmMess723), argv [0]);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv [1]);

	strcpy (systemDate, DateToString (TodaysDate ()));
	strcpy (glwkRec.name,"                              ");

	OpenDB ();


	dsp_screen ("Creating Customer Journals.", comm_rec.co_no,comm_rec.co_name);

	strcpy (branchNo, comm_rec.est_no);

	if (argc > 2)
		strcpy (branchNo, argv [2]);

	strcpy (work_type, "1");

	/*----------------------------------
	| Process receipts transactions  . |
	----------------------------------*/
	strcpy (cuwk_rec.co_no,comm_rec.co_no);
	strcpy (cuwk_rec.est, branchNo);
	sprintf (cuwk_rec.inv_no,"%8.8s"," ");
	cc = find_rec (cuwk, &cuwk_rec, GTEQ, "u");
	while (!cc && !strcmp (cuwk_rec.co_no,comm_rec.co_no) &&
		      !strcmp (cuwk_rec.est, branchNo))
	{
		if (!strcmp (cuwk_rec.stat_flag,"0") &&
		     !strcmp (cuwk_rec.type,work_type))
		{

			/*----------------------------------
			| IF CONTRACT WILL NOT FIND CMPB   |
			| first find cmhr record and check |
			| if FIXED and if INTERNAL and if  |
			| PROGRESSIVE. Will make sure that |
			| contract so that any variance    |
			| can be worked out.               |
			----------------------------------*/
			strcpy (cohr_rec.co_no, cuwk_rec.co_no);
			strcpy (cohr_rec.br_no, cuwk_rec.est);
			strcpy (cohr_rec.inv_no,cuwk_rec.inv_no);
			strcpy (cohr_rec.type, "I");
			cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
			if (cc)
			{
				abc_unlock (cuwk);
				cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
				continue;
			}
			cc = find_hash (cmpb, &cmpb_rec, EQUAL,"r", cohr_rec.hhco_hash);
			if (cc)
			{
				abc_unlock (cuwk);
				cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
				continue;
			}

			cc = find_hash (cmhr, &cmhr_rec, EQUAL, "r", cmpb_rec.hhhr_hash);
			if (cc)
			{
				abc_unlock (cuwk);
				cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
				continue;
			}
			
			if (CLOSED && FIXED)
				SumCmcbs ();
			else
				SumColns ();

			/*-------------------------------------
			| Create General Ledger Work Records. |
			-------------------------------------*/
			/*-----------------------------------
			| if EXTERNAL it will post to sales |
			| gst, debtors, cos, and reverse wip|
			| ELSE will reverse wip and post to |
			| internal from cmhr.               |
			| costs are from cmcb.              |
			-----------------------------------*/
			if (EXTERNAL)
				update ();
			_nodata = TRUE;
		}

		if (_nodata)
		{
			if (EXTERNAL)
				PostExternal ();
			else
				PostInternal ();

			ReverseWip ();


			if (CLOSED && FIXED && EXTERNAL)
				PostVariance ();

			/*----------------------
			| Try twice to delete. |
			----------------------*/
			if ((cc = abc_delete (cuwk)))
				cc = abc_delete (cuwk);

			if (cc)
			{
				/*---"INDEX ON FILE CUWK IS BAD PLEASE RUN BCHECK"---*/
				errmess (ML (mlCmMess057));
				sleep (20);
				shutdown_prog ();
				return (EXIT_FAILURE);
			}
			
		}
		cc = find_rec (cuwk, &cuwk_rec, NEXT, "u");
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Datebase Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);
	abc_fclose (comr);

	abc_alias (cmpb2, cmpb);

	open_rec (cuwk, cuwk_list, CUWK_NO_FIELDS, "cuwk_id_no");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_hhco_hash");
	open_rec (cmpb2,cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_hhhr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	OpenGlmr ();
	OpenGlwk ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=======================
| Close Datebase Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cuwk);
	abc_fclose (cmpb);
	abc_fclose (cmpb2);
	abc_fclose (cmcb);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cmhr);
	GL_CloseBatch (printerNo);
	GL_Close ();

	abc_dbclose (data);
}

/*=======================
| Transaction records . |
=======================*/
void
update (
 void)
{
	int	is_neg;
	
	/*----------------------
	| Create glwk records. |
	----------------------*/

	dsp_process ("Invoice : ", cuwk_rec.inv_no);

	/*---------------------------------------------------
	| If date is Zero then set to current debtors date. |
	---------------------------------------------------*/
	if (cuwk_rec.date_of_inv == 0L)
		cuwk_rec.date_of_inv = comm_rec.dbt_date;

	DateToDMY (cuwk_rec.date_of_inv, NULL, &mnth, NULL);

	if (strcmp (cuwk_rec.inv_no,prev_inv) ||
	     glwkRec.ci_amt != cuwk_rec.tot_fx)
	{
		gst_amt = cuwk_rec.gst;
		strcpy (prev_inv,cuwk_rec.inv_no);
	}
	else
	{
		sprintf (cuwk_rec.inv_no,"%8.8s"," ");
		strcpy (cuwk_rec.dbt_no,"      ");
	}

	dbt_amt = cuwk_rec.fx_amt;
	per_date = cuwk_rec.date_of_inv;

	sprintf (glwkRec.narrative,"Sales      %s", cuwk_rec.inv_no);

	if (FIXED)
	{
		cuwk_rec.fx_amt = tot_sale ;
		cuwk_rec.loc_amt = tot_sale ;
	}

	if (FIXED && PROGRESS && CLOSED)
	{
		/*----------------------------------------------
		| this to adjust sales to reflect actual sales |
		----------------------------------------------*/
		cuwk_rec.fx_amt = tot_sale - prev_amt + cmpb_rec.amount;
		cuwk_rec.loc_amt = tot_sale - prev_amt + cmpb_rec.amount;
	}

	if (cuwk_rec.fx_amt < 0.00)
	{
		is_neg = TRUE;
		cuwk_rec.fx_amt *= -1.00;
	}
	else
		is_neg = FALSE;

	strcpy (glwkRec.jnl_type, (is_neg) ? "1" : "2");
	WriteGlwk ();

}

/*========================================
| Write General Ledger Work File Record. |
========================================*/
void
WriteGlwk (void)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	strcpy (glmrRec.acc_no, cuwk_rec.gl_acc_no);
	if ((cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{	
		GL_GLI
		(
			comm_rec.co_no,
			branchNo,
			"  ",
			"SUSPENSE  ",
			"   ",
			" "
		);
		strcpy (cuwk_rec.gl_acc_no,glmrRec.acc_no);
	}
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	strcpy (glwkRec.tran_type, "23");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.period_no,cuwk_rec.period_no);
	strcpy (glwkRec.acc_no, cuwk_rec.gl_acc_no);
	strcpy (glwkRec.co_no,cuwk_rec.co_no);
	strcpy (glwkRec.est_no,cuwk_rec.est);

	sprintf (glwkRec.acronym,	"%-9.6s",cuwk_rec.dbt_no);
	strcpy (glwkRec.user_ref,	cuwk_rec.inv_no);
	strcpy (glwkRec.alt_desc1,  cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc2,  " ");
	strcpy (glwkRec.alt_desc3,  " ");
	strcpy (glwkRec.batch_no,   " ");

	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuwk_rec.inv_no);
	strcpy (glwkRec.stat_flag,"2");
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	glwkRec.tran_date = cuwk_rec.date_of_inv;
	glwkRec.ci_amt = cuwk_rec.tot_fx;
	glwkRec.o1_amt = cuwk_rec.disc;
	glwkRec.o4_amt = cuwk_rec.gst;
	glwkRec.post_date = StringToDate (systemDate);

	glwkRec.amount 		= cuwk_rec.fx_amt;
	glwkRec.loc_amount 	= cuwk_rec.loc_amt;
	sprintf (glwkRec.run_no, "      ");

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();
}

/*==================================================
| Post accumulated debits & credits if non-zero  . |
==================================================*/
void
PostExternal (void)
{
	strcpy (glwkRec.jnl_type, "4");

	if (gst_amt != 0.00)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			branchNo,
			"  ",
			"G.S.T CHRG",
			"   ",
			" "
		);
		strcpy (cuwk_rec.gl_acc_no, glmrRec.acc_no);
		cuwk_rec.fx_amt = gst_amt;
		cuwk_rec.loc_amt = gst_amt;
		sprintf (glwkRec.narrative, "G.S.T.     %s", cuwk_rec.inv_no);
		WriteGlwk ();
	}

	/*-------------------------------------
	| Create A Record For Customer Control. |
	-------------------------------------*/
	GL_GLI 
	(
		comm_rec.co_no,
		branchNo,
		"  ",
		"ACCT REC  ",
		"   ",
		" "
	);
	strcpy (cuwk_rec.gl_acc_no, glmrRec.acc_no);

	strcpy (glwkRec.jnl_type,"1");

	dbt_amt = dbt_amt + gst_amt;

	if (dbt_amt != 0.00)
	{
		cuwk_rec.fx_amt = dbt_amt;
		cuwk_rec.loc_amt = dbt_amt;
		sprintf (glwkRec.narrative, "Customer   %s", cuwk_rec.inv_no);
		WriteGlwk ();
	}

	/*---------------------------------------
	| Create A Record For Discount Account. |
	---------------------------------------*/
	strcpy (glwkRec.jnl_type, "3");

	if (cuwk_rec.disc != 0.00)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			branchNo,
			"  ",
			"DISC ALLOW",
			"   ",
			" "
		);
		strcpy (cuwk_rec.gl_acc_no, glmrRec.acc_no);
		cuwk_rec.fx_amt = cuwk_rec.disc;
		cuwk_rec.loc_amt = cuwk_rec.disc;
		WriteGlwk ();
	}

	/*---------------------------------------
	| Create A Record For Cost Of Sales.    |
	---------------------------------------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.cog_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.cog_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,"%-9.6s",cuwk_rec.dbt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuwk_rec.inv_no);

	sprintf (glwkRec.jnl_type,  "%-1.1s",   "1");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "23"); 
	glwkRec.amount =  tot_cost;

	sprintf (glwkRec.narrative,"COS        %s", cuwk_rec.inv_no);

	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.post_date = StringToDate (systemDate);
	glwkRec.tran_date = cuwk_rec.date_of_inv;
	strcpy (glwkRec.user_ref,	cuwk_rec.inv_no);
	strcpy (glwkRec.alt_desc1,  cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc2,  " ");
	strcpy (glwkRec.alt_desc3,  " ");
	strcpy (glwkRec.batch_no,   " ");
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	strcpy (glwkRec.period_no,cuwk_rec.period_no);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
SumCmcbs (void)
{
	tot_cost = 0.00;
	tot_sale = 0.00;
	prev_amt = 0.00;

	cc = find_hash (cmcb, &cmcb_rec, EQUAL, "r", cmhr_rec.hhhr_hash);

	while (!cc && cmcb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		tot_cost += cmcb_rec.sum_cost;
		tot_sale += cmcb_rec.sum_value;
		cc = find_hash (cmcb, &cmcb_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}

	GetPrvInv ();
}

void
GetPrvInv (
 void)
{
	/*-----------------------
	| means it is :         |
	|   PROGRESS && FIXED   |
	|   && CLOSED 	        |
	| and may have prev     |
	| amounts billed which  |
	| need to be considered |
	| when working out      |
	| the variance          |
	-----------------------*/
	cc = find_hash (cmpb2, &cmpb2_rec,EQUAL,"r",cmhr_rec.hhhr_hash);

	while (!cc && cmpb2_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		prev_amt += cmpb2_rec.amount;
		cc = find_hash (cmpb2, &cmpb2_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}
}

void
SumColns (void)
{
	tot_cost = 0.00;
	tot_sale = 0.00;
	prev_amt = 0.00;

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = 0;

	cc = find_rec (coln, &coln_rec, GTEQ, "r");

	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		if (!CLOSED && FIXED)
		/* means that it is not final billing for fixed */
			;
		else
			tot_cost += coln_rec.cost_price * coln_rec.q_order;
		tot_sale += coln_rec.gross;
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
}

void
ReverseWip (void)
{
	/*----------------------
	| Reverse WIP at Cost  |
	----------------------*/
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.wip_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.wip_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,"%-9.6s",cuwk_rec.dbt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuwk_rec.inv_no);

	/*----------------------------
	| type 2 because is reversal |
	----------------------------*/
	sprintf (glwkRec.jnl_type,  "%-1.1s",   "2");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "23"); 
	glwkRec.amount = tot_cost;
	sprintf (glwkRec.narrative,"WIP Rev.   %s", cuwk_rec.inv_no);
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.post_date = StringToDate (systemDate);
	glwkRec.tran_date = cuwk_rec.date_of_inv;
	strcpy (glwkRec.user_ref,	cuwk_rec.inv_no);
	strcpy (glwkRec.alt_desc1,  cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc2,  " ");
	strcpy (glwkRec.alt_desc3,  " ");
	strcpy (glwkRec.batch_no,   " ");
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	strcpy (glwkRec.period_no,cuwk_rec.period_no);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();
}

void
PostVariance (
 void)
{
	int is_neg = FALSE;

	glwkRec.amount = tot_sale - cmhr_rec.quote_val;

	if (glwkRec.amount < 0.00)
	{
		is_neg = TRUE;
		glwkRec.amount *= -1.00;
	}

	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.var_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,cmhr_rec.var_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,"%-9.6s",cuwk_rec.dbt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuwk_rec.inv_no);

	sprintf (glwkRec.jnl_type,  "%-1.1s", (is_neg) ? "2" : "1");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "23"); 

	sprintf (glwkRec.narrative,"Variance   %s", cuwk_rec.inv_no);

	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.post_date = StringToDate (systemDate);
	glwkRec.tran_date = cuwk_rec.date_of_inv;
	strcpy (glwkRec.user_ref,	cuwk_rec.inv_no);
	strcpy (glwkRec.alt_desc1,  cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc2,  " ");
	strcpy (glwkRec.alt_desc3,  " ");
	strcpy (glwkRec.batch_no,   " ");
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	strcpy (glwkRec.period_no,cuwk_rec.period_no);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	GL_AddBatch ();

}

void
PostInternal (
 void)
{
	/*------------------
	| lookup hhgl_hash |
	------------------*/
	strcpy (glmrRec.co_no,  comm_rec.co_no);
	strcpy (glmrRec.acc_no, cmhr_rec.int_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	strcpy (glwkRec.acc_no, cmhr_rec.int_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym,"%-9.6s",cuwk_rec.dbt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuwk_rec.inv_no);

	sprintf (glwkRec.jnl_type,  "%-1.1s",   "1");  
	sprintf (glwkRec.tran_type, "%-2.2s",   "23"); 
	glwkRec.amount = tot_cost;
	sprintf (glwkRec.narrative,"Internal   %s", cuwk_rec.inv_no);

	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.post_date = StringToDate (systemDate);
	glwkRec.tran_date = cuwk_rec.date_of_inv;
	strcpy (glwkRec.user_ref,	cuwk_rec.inv_no);
	strcpy (glwkRec.alt_desc1,  cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc2,  " ");
	strcpy (glwkRec.alt_desc3,  " ");
	strcpy (glwkRec.batch_no,   " ");
	sprintf (glwkRec.stat_flag, "%-1.1s",   "2");  
	strcpy (glwkRec.period_no,cuwk_rec.period_no);

	glwkRec.loc_amount 	= glwkRec.amount;
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);

	GL_AddBatch ();
}
