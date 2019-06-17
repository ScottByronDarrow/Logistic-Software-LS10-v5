/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_revalue.c,v 5.2 2002/11/28 04:09:51 scott Exp $
|  Program Name  : (so_revalue.c  )                                 |
|  Program Desc  : (Revalue orders from Inventory if order number)   |
|                  (does not start with a '#'                   )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: so_revalue.c,v $
| Revision 5.2  2002/11/28 04:09:51  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.1  2001/08/09 09:21:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:20:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:41:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 09:26:13  scott
| Updated to allow -ve discounts to be processed.
| Problem with this was test on inmr_disc_pc should have been applied
| only when inmr_disc_pc had a value.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_revalue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_revalue/so_revalue.c,v 5.2 2002/11/28 04:09:51 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>

#define		FGN_CURR     (envVarDbMcurr && strcmp (cumr_rec.curr_code, envVarCurrCode))

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

	char	*data = "data";

	int		contactPrice 	= FALSE,
			envVarDbMcurr	= 0,
			firstTime 		= TRUE,
			printerNumber	= 1,
			envVarGst 		= 1;

	float	regPc			= 0.00,
			totQty			= 0.00;

	char	envVarGstTaxName [4],
			envVarCurrCode [4];

	FILE	*fout,
			*fin;

#include	<cus_price.h>
#include	<cus_disc.h>
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessFile 	(void);
void 	ProcessSoln 	(long);
void 	PriceProcess 	(void);
void 	DiscProcess 	(void);
void 	OpenAudit 		(void);
void 	PrintLine 		(void);
void 	EndReport 		(void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr = chk_env ("DB_MCURR");

	if (sptr)
		envVarDbMcurr = atoi (sptr);
	else
		envVarDbMcurr = FALSE;

	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		return (EXIT_FAILURE);
	}

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVarGst = 0;
	else
		envVarGst = (*sptr == 'Y' || *sptr == 'y');

	if (envVarGst)
		sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVarGstTaxName, "%-3.3s", "TAX");

	printerNumber = atoi (argv [1]);

	OpenDB ();

	OpenPrice ();
	OpenDisc ();

	set_tty ();
	dsp_screen ("Revaluing Orders From Inventory Master File.",
			comm_rec.co_no, comm_rec.co_name);

	ProcessFile ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	if (firstTime == FALSE)
		EndReport ();

	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cnch);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (sohr);
	abc_fclose (soln);

	ClosePrice ();
	CloseDisc ();

	abc_dbclose (data);
}

void
ProcessFile (
 void)
{
	strcpy (sohr_rec.co_no,    comm_rec.co_no);
	strcpy (sohr_rec.br_no,    comm_rec.est_no);
	strcpy (sohr_rec.order_no, "        ");

	cc = find_rec (sohr, &sohr_rec, GTEQ, "u");
	while (!cc && 
		   !strcmp (sohr_rec.co_no, comm_rec.co_no) && 
		   !strcmp (sohr_rec.br_no, comm_rec.est_no))
	{
       	if (sohr_rec.order_no [0] == '#') 
		{
			/*-----------------
			| Get next record |
			-----------------*/
			abc_unlock (sohr);
			cc = find_rec (sohr, &sohr_rec, NEXT, "u");
			continue;
		}
    	ProcessSoln (sohr_rec.hhso_hash);

		abc_unlock (sohr);
    	dsp_process ("Order #", sohr_rec.order_no);

		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
}

/*============================
| Process sales order lines. |
============================*/
void
ProcessSoln (
 long hhso_hash)
{
	/*-------------------------
	| Process all order lines |
	-------------------------*/
	soln_rec.hhso_hash = hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhso_hash) 
	{
		/*------------------------------
		| Skip lines with bonus items. |
		------------------------------*/
		if (soln_rec.bonus_flag [0] == 'Y')
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}

		/*-----------------------------------
		| Skip lines with a contract price. |
		-----------------------------------*/
		if (soln_rec.cont_status != 0)
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", soln_rec.hhbr_hash);
		if (cc) 
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}

		cc = find_hash (cumr, &cumr_rec, COMPARISON, "r", sohr_rec.hhcu_hash);
		if (cc) 
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}

		if (envVarDbMcurr)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code,  cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");
		}
		if (!envVarDbMcurr || !pocr_rec.ex1_factor)
			pocr_rec.ex1_factor = 1.00;

		totQty = soln_rec.qty_order + soln_rec.qty_bord;

		PriceProcess ();
		DiscProcess ();

		soln_rec.tax_pc = (cumr_rec.tax_code [0] == 'A' || 
							cumr_rec.tax_code [0] == 'B') 
							   ? 0.00 : inmr_rec.tax_pc;

		soln_rec.gst_pc = inmr_rec.gst_pc;

		cc = abc_update (soln, &soln_rec);
		if (cc) 
			file_err (cc, soln, "DBUPDATE");

		/*------------------
		| New line of code |
		------------------*/
		PrintLine ();

		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}	
}

void
PriceProcess (
 void)
{
	int		pType;
	double	gsale_price;
	double	sale_price;

	/*-------------------------
	| Lookup contract header. |
	-------------------------*/
	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, sohr_rec.cont_no);
	cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
	if (cc)
	{
		cnch_rec.hhch_hash = 0L;
		strcpy (cnch_rec.exch_type, " ");
	}

	pType = atoi (cumr_rec.price_type);
	gsale_price = GetCusPrice (comm_rec.co_no,
					  		  comm_rec.est_no,
							  comm_rec.cc_no,
							  sohr_rec.area_code,
							  cumr_rec.class_type,
							  inmr_rec.sellgrp,
							  cumr_rec.curr_code,
							  pType,
						 	  cumr_rec.class_type,
							  cnch_rec.exch_type,
							  cumr_rec.hhcu_hash,
							  soln_rec.hhcc_hash,
							  soln_rec.hhbr_hash,
						 	  inmr_rec.category,
							  cnch_rec.hhch_hash,
							  sohr_rec.dt_required,
							  totQty,
							  pocr_rec.ex1_factor,
							  FGN_CURR,
							  &regPc);

	sale_price = GetCusGprice (gsale_price, regPc);

	soln_rec.gsale_price = gsale_price;
	soln_rec.sale_price  = sale_price;
	soln_rec.reg_pc      = regPc;
	soln_rec.cont_status = _cont_status;

	contactPrice = (_CON_PRICE) ? TRUE : FALSE;
}

void
DiscProcess (
 void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (soln_rec.cont_status == 2 || contactPrice)
	{
		soln_rec.dis_pc	= 0.00;
		soln_rec.disc_a	= 0.00;
		soln_rec.disc_b	= 0.00;
		soln_rec.disc_c	= 0.00;
		soln_rec.cumulative	= 0;
		return;
	}

	pType = atoi (cumr_rec.price_type);
	cumDisc	= GetCusDisc (comm_rec.co_no,
						 comm_rec.est_no,
						 soln_rec.hhcc_hash,
						 cumr_rec.hhcu_hash,
						 cumr_rec.class_type,
						 cumr_rec.disc_code,
						 soln_rec.hhbr_hash,
						 inmr_rec.category,
						 inmr_rec.sellgrp,
						 pType,
						 soln_rec.sale_price,
						 regPc,
						 totQty,
						 discArray);
							
	soln_rec.dis_pc 	= CalcOneDisc (cumDisc,
									  discArray [0],
									  discArray [1],
									  discArray [2]);

	soln_rec.disc_a 	= discArray [0];
	soln_rec.disc_b 	= discArray [1];
	soln_rec.disc_c 	= discArray [2];
	soln_rec.cumulative = cumDisc;

	if (inmr_rec.disc_pc > soln_rec.dis_pc && inmr_rec.disc_pc != 0.0)
	{
		soln_rec.dis_pc = inmr_rec.disc_pc;
		soln_rec.disc_a	= inmr_rec.disc_pc;
		soln_rec.disc_b	= 0.00;
		soln_rec.disc_c	= 0.00;
	}
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EREVALUED ORDERS REPORT\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EAs At %s\n", SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EBranch %s\n", clip (comm_rec.est_name));

	fprintf (fout, ".R============");
	fprintf (fout, "============");
	if (envVarDbMcurr)
		fprintf (fout, "======");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "=============\n");

	fprintf (fout, "============");
	fprintf (fout, "============");
	if (envVarDbMcurr)
		fprintf (fout, "======");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "=============\n");

	fprintf (fout, "!S.O. NO    ");
	fprintf (fout, "!CUSTOMER NO");
	if (envVarDbMcurr)
		fprintf (fout, "!CURR ");
	fprintf (fout, "!ITEM NO.        ");
	fprintf (fout, "!ITEM DESCRIPTION                        ");
	fprintf (fout, "!SALES PRICE");
	fprintf (fout, "!DISCOUNT %% ");
	fprintf (fout, "!  %-3.3s  %%   !\n", envVarGstTaxName);

	fprintf (fout, "!-----------");
	fprintf (fout, "!-----------");
	if (envVarDbMcurr)
		fprintf (fout, "!-----");
	fprintf (fout, "!----------------");
	fprintf (fout, "!----------------------------------------");
	fprintf (fout, "!-----------");
	fprintf (fout, "!-----------");
	fprintf (fout, "!-----------!\n");
	fflush (fout);
}

void
PrintLine (
 void)
{
	if (firstTime == TRUE)
		OpenAudit ();

	firstTime = FALSE;

	fprintf (fout, "! %s  ",      sohr_rec.order_no);
	fprintf (fout, "!  %s   ",    cumr_rec.dbt_no);
	if (envVarDbMcurr)		
		fprintf (fout, "! %s ",   pocr_rec.code );
	fprintf (fout, "!%s",         inmr_rec.item_no);
	fprintf (fout, "!%s",         inmr_rec.description);
	fprintf (fout, "!%10.2f ",    DOLLARS (soln_rec.sale_price));
	fprintf (fout, "!%10.2f ",    soln_rec.dis_pc);
	if (envVarGst)
		fprintf (fout, "!%10.2f !\n", soln_rec.gst_pc);
	else
		fprintf (fout, "!%10.2f !\n", soln_rec.tax_pc);

	fflush (fout);
}

void
EndReport (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}
