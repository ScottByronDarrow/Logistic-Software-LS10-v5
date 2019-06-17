/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_roy_up.c                                      |
|  Program Desc  : ( Update Sales History  File From cohr,coln    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, coln, cohr, inmr, ryhs, ryhr, rymr,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  ryhs, cohr, coln,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 08/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (14/03/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (15/03/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (18/12/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (30/09/97)      | Modified  by  : Elizabeth D. Paid|
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Fixed Bugs.                                        |
|                : Create ryhs against cumr_roy_type;ie. one product  |
|                : may be assigned to diff. royalties.                |
|    (18/12/89)  : If ryhr_basis = "D", do not calc. royalty values.  |
|    (30/09/97)  : SEL -    Multilingual Conversion, changed printf   |
|                           to print_at                               |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: so_roy_up.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_roy_up/so_roy_up.c,v 5.4 2002/12/01 04:48:19 scott Exp $";

#define	INVOICE	(type_flag[0] == 'I')
#define	NOTAX	( cohr_rec.hr_tcode[0] == 'A' || cohr_rec.hr_tcode[0] == 'B' )
#define	CCMAIN	

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>


/*================================================================
| Special fields and flags  ################################## . |
================================================================*/
	char	type_flag[2];
	char	fnd_status[2];
	char	upd_status[2];
 
	/*==================================
	| file comm { System Common file } |
	==================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tcc_no[3];
		long	tdbt_date;
	} comm_rec;

	/*============================================
	| Customer Order/Invoice/Credit Header File. |
	============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhco_hash"},
		{"cohr_tax_code"},
		{"cohr_stat_flag"}
	};

	int cohr_no_fields = 8;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhco_hash;
		char	hr_tcode[2];
		char	hr_stat_flag[2];
	} cohr_rec;

	/*============================================
	| Customer Order/Invoice/Credit Detail File. |
	============================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_q_order"},
		{"coln_sale_price"},
		{"coln_cost_price"},
		{"coln_gross"},
		{"coln_amt_disc"},
		{"coln_stat_flag"}
	};

	int coln_no_fields = 9;

	struct {
		long 	ln_hhco_hash;
		int 	ln_line_no;
		long 	ln_hhbr_hash;
		float 	ln_q_order;
		double 	ln_sale_price;		/*  Money field  */
		double 	ln_cost_price;		/*  Money field  */
		double 	ln_gross;		/*  Money field  */
		double 	ln_amt_disc;		/*  Money field  */
		char 	ln_stat_flag[2];
	} coln_rec;

	/*=======================================
	| Royalty Type Master File Base Record. |
	=======================================*/
	struct dbview rymr_list[] ={
		{"rymr_co_no"},
		{"rymr_code"},
		{"rymr_desc"},
		{"rymr_qty1"},
		{"rymr_qty2"},
		{"rymr_qty3"},
		{"rymr_qty4"},
		{"rymr_qty5"},
		{"rymr_qty6"},
		{"rymr_pc1"},
		{"rymr_pc2"},
		{"rymr_pc3"},
		{"rymr_pc4"},
		{"rymr_pc5"},
		{"rymr_pc6"},
		{"rymr_stat_flag"}
	};

	int rymr_no_fields = 16;

	struct {
		char	rm_co_no[3];
		char	rm_code[10];
		char	rm_desc[41];
		float	rm_qty[6];
		float	rm_pc[6];
		char	rm_stat_flag[2];
	} rymr_rec;

	/*=============================
	| Royalty Header File Record. |
	=============================*/
	struct dbview ryhr_list[] ={
		{"ryhr_code"},
		{"ryhr_hhbr_hash"},
		{"ryhr_hhry_hash"},
		{"ryhr_basis"},
		{"ryhr_abs_amt"},
		{"ryhr_amt_extract"},
		{"ryhr_stat_flag"}
	};

	int ryhr_no_fields = 7;

	struct {
		char	hr_code[10];
		long	hr_hhbr_hash;
		long	hr_hhry_hash;
		char	hr_basis[2];
		double	hr_abs_amt;		/*  Money field  */
		double	hr_amt_extract;		/*  Money field  */
		char	hr_stat_flag[2];
	} ryhr_rec;

	/*=======================+
	 | Royalty History File. |
	 +=======================*/
#define	RYHS_NO_FIELDS	17

	struct dbview	ryhs_list [RYHS_NO_FIELDS] =
	{
		{"ryhs_co_no"},
		{"ryhs_hhry_hash"},
		{"ryhs_publish"},
		{"ryhs_roy_pc"},
		{"ryhs_roy_basis"},
		{"ryhs_mtd_qty"},
		{"ryhs_mtd_gross"},
		{"ryhs_mtd_nett"},
		{"ryhs_mtd_roy"},
		{"ryhs_ytd_qty"},
		{"ryhs_ytd_gross"},
		{"ryhs_ytd_nett"},
		{"ryhs_ytd_roy"},
		{"ryhs_htd_qty"},
		{"ryhs_htd_gross"},
		{"ryhs_htd_nett"},
		{"ryhs_htd_roy"}
	};

	struct tag_ryhsRecord
	{
		char	co_no [3];
		long	hhry_hash;
		char	publish [5];
		float	roy_pc;
		char	roy_basis [2];
		float	mtd_qty;
		Money	mtd_gross;
		Money	mtd_nett;
		Money	mtd_roy;
		float	ytd_qty;
		Money	ytd_gross;
		Money	ytd_nett;
		Money	ytd_roy;
		float	htd_qty;
		Money	htd_gross;
		Money	htd_nett;
		Money	htd_roy;
	}	ryhs_rec;


	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_pack_size"},
		{"inmr_outer_size"},
		{"inmr_ltd_sales"},
	};

	int inmr_no_fields = 15;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_alpha_code[17];
		char	mr_supercession_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		char	mr_serial_item[2];
		char	mr_costing_flag[2];
		char	mr_pack_size[6];
		float	mr_outer_size;
		float	mr_ltd_sales;
	} inmr_rec;

	/*==============================================
	| Customer Royalty Type Master File Base Record. |
	==============================================*/
	struct dbview dbry_list[] ={
		{"dbry_co_no"},
		{"dbry_cr_type"},
		{"dbry_desc"},
		{"dbry_stat_flag"}
	};

	int dbry_no_fields = 4;

	struct {
		char	ry_co_no[3];
		char	ry_cr_type[4];
		char	ry_desc[41];
		char	ry_stat_flag[2];
	} dbry_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_curr_code"},
		{"cumr_roy_type"},
	};

	int cumr_no_fields = 9;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_acronym[10];
		char	cm_curr_code[4];
		char	cm_roy_type[4];
	} cumr_rec;

	/*======================
	| Inventory Price File |
	======================*/
	struct dbview inpr_list [] =
	{
		{"inpr_hhbr_hash"},
		{"inpr_price_type"},
		{"inpr_br_no"},
		{"inpr_wh_no"},
		{"inpr_curr_code"},
		{"inpr_area_code"},
		{"inpr_cust_type"},
		{"inpr_hhgu_hash"},
		{"inpr_base"},
	};

	int	inpr_no_fields = 9;

	struct tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	br_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	area_code [3];
		char	cust_type [4];
		long	hhgu_hash;
		double	base;	  /* money */
	} inpr_rec, inpr2_rec;

	char	wk_branch[3];
	int		retail;

	double	gross_sales	=	0.00,
			nett_sales	=	0.00,
			roy			=	0.00;

	float	qty			=	0.00,
			roy_pc		=	0.00,
			gst_div     =	0.00,
			gst_include =	0.00;

	int		PriceLevel = 0;
	int		sk_dbprinum;
	int		sk_dbqtynum;


/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
double out_gst (double total_amt);
void procfile (void);
void proccoln (void);
void proc_ryhr (long hhbr_hash);
float find_roy_pc (void);
void calc_ryhs (void);
void upd_ryhs (void);
int  find_dbry (char *cr_code);
void WsFindInpr (int price_type);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

	sptr = chk_env("SK_CUSPRI_LVL");
	PriceLevel = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_DBQTYNUM");
	sk_dbqtynum = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_DBPRINUM");
	sk_dbprinum = (sptr == (char *)0) ? 0 : atoi (sptr);

	gst_include = (float) (atof(get_env("GST_INCLUSIVE")));

	if (gst_include != 0.00)
		gst_div = (float) ((100.00 + gst_include) / gst_include);
	else
		gst_div = 0.00;

	if (argc < 5)
	{
		print_at(0,0, mlSoMess788,argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf (type_flag,"%-1.1s",argv[1]);
	sprintf (fnd_status,"%-1.1s",argv[2]);
	sprintf (upd_status,"%-1.1s",argv[3]);
	retail = atoi(argv[4]);

	OpenDB();

	init_scr ();
	sprintf (err_str," Updating %s to Royalty History.",(INVOICE) ? "Invoices" : "Credits");
	print_mess (err_str);

	if (argc == 6)
		sprintf (wk_branch,"%-2.2s",argv[5]);
	else
		sprintf (wk_branch,"%-2.2s",comm_rec.tes_no);

	procfile();

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("inpr", inpr_list, inpr_no_fields, "inpr_id_no");
	open_rec("cohr", cohr_list, cohr_no_fields, "cohr_up_id");
	open_rec("coln", coln_list, coln_no_fields, "coln_id_no");
	open_rec("ryhr", ryhr_list, ryhr_no_fields, "ryhr_id_no2");
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec("ryhs", ryhs_list, RYHS_NO_FIELDS, "ryhs_id_no");
	open_rec("rymr", rymr_list, rymr_no_fields, "rymr_id_no");
	open_rec("dbry", dbry_list, dbry_no_fields, "dbry_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose("inpr");
	abc_fclose("cohr");
	abc_fclose("coln");
	abc_fclose("inmr");
	abc_fclose("ryhs");
	abc_fclose("ryhr");
	abc_fclose("rymr");
	abc_fclose("dbry");
	abc_fclose("cumr");
	abc_dbclose("data");
}

double	
out_gst (
 double total_amt)
{
	double	gst_amount	= 0.00;

	if (total_amt == 0)
		return(0.00);

	gst_amount = no_dec(total_amt / gst_div);
	total_amt -= no_dec(gst_amount);
	return(total_amt);
}

/*===========================================================
| Process whole cohr file looking for stat_flag = find_stat |
===========================================================*/
void
procfile (
 void)
{
	int	ok = TRUE;

	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	while (ok) 
	{
		strcpy (cohr_rec.hr_co_no, comm_rec.tco_no);
		strcpy (cohr_rec.hr_br_no, wk_branch);
		strcpy (cohr_rec.hr_type, type_flag);
		strcpy (cohr_rec.hr_stat_flag, fnd_status);

		cc = find_rec("cohr", &cohr_rec, COMPARISON, "u");
		if (cc) 
		{
			abc_unlock("cohr");
			ok = FALSE;
			continue;
		}
		
		/*---------------------------------------
		| Find cumr to get cumr_roy_type	|
		---------------------------------------*/
		cumr_rec.cm_hhcu_hash	=	cohr_rec.hr_hhcu_hash;
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
		if (!cc)
		{
			/*----------------------------------------------------
			| Validate royalty class if cc then ignore proc_coln |
			----------------------------------------------------*/
			cc = find_dbry(cumr_rec.cm_roy_type);
			if (!cc)
				proccoln();
		}

		strcpy (cohr_rec.hr_stat_flag,upd_status);
		cc = abc_update("cohr",&cohr_rec);
		if (cc) 
			sys_err("Error in cohr During (DBUPDATE)",cc,PNAME);
		
		abc_unlock("cohr");
	}
	abc_unlock("cohr");
}

/*==========================
| Process whole coln file. | 
==========================*/
void
proccoln (
 void)
{
	coln_rec.ln_hhco_hash = cohr_rec.hr_hhco_hash;
	coln_rec.ln_line_no = 0;
	cc = find_rec("coln",&coln_rec,GTEQ,"u");

	while (!cc && coln_rec.ln_hhco_hash == cohr_rec.hr_hhco_hash)
	{
		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",coln_rec.ln_hhbr_hash);
		if (!cc)
			proc_ryhr(coln_rec.ln_hhbr_hash);

		strcpy (coln_rec.ln_stat_flag,upd_status);
		cc = abc_update("coln",&coln_rec);
		if (cc) 
			sys_err("Error in coln During (DBUPDATE)",cc,PNAME);

		cc = find_rec("coln",&coln_rec,NEXT,"u");
	}
	abc_unlock("coln");
}

/*=======================================
| Get all ryhr with same dbry_code	|
=======================================*/
void
proc_ryhr (
 long hhbr_hash)
{
	ryhr_rec.hr_hhbr_hash = hhbr_hash;
	sprintf (ryhr_rec.hr_code,"%-3.3s%-6.6s",cumr_rec.cm_roy_type," ");
	cc = find_rec("ryhr",&ryhr_rec,GTEQ,"r");

	while (!cc && ryhr_rec.hr_hhbr_hash == hhbr_hash)
	{
		if (!strncmp(ryhr_rec.hr_code,cumr_rec.cm_roy_type,3))
		{
			strcpy (rymr_rec.rm_co_no,comm_rec.tco_no);
			strcpy (rymr_rec.rm_code,ryhr_rec.hr_code);
			cc = find_rec("rymr", &rymr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec("ryhr",&ryhr_rec,NEXT,"r");
				continue;
			}
			if (ryhr_rec.hr_basis[0] != 'D')
			{
				calc_ryhs();
				upd_ryhs();
			}
		}
		cc = find_rec("ryhr",&ryhr_rec,NEXT,"r");
	}
}

/*=======================
| Find Royalty Percent. |
=======================*/
float	
find_roy_pc (
 void)
{
	register	int	i;

	if (inmr_rec.mr_ltd_sales <= rymr_rec.rm_qty[0])
		return(rymr_rec.rm_pc[0]);

	if (rymr_rec.rm_qty[0] == 0)
		return(0.00);

	for (i = 0; rymr_rec.rm_qty[i] != 0 && i < 5; i++)
	{
		if (i != 5 && rymr_rec.rm_qty[i + 1] == 0.00)
			return(rymr_rec.rm_pc[i]);

		if ( rymr_rec.rm_qty[i] < inmr_rec.mr_ltd_sales && 
 		     inmr_rec.mr_ltd_sales <= rymr_rec.rm_qty[i + 1])
		{
			return(rymr_rec.rm_pc[i + 1]);
		}
	}
	return(rymr_rec.rm_pc[5]);
}

/*==========================
| Calculate Royaly Amount. |
==========================*/
void
calc_ryhs (
 void)
{
	double	price = 0.00;	
	double	ext_extr = 0.00;	

	gross_sales	=	0.00;
	nett_sales	=	0.00;
	roy			=	0.00;
	qty			=	0.00;
	roy_pc		=	0.00;

	WsFindInpr (retail -1);

	/*-----------------------
	| Calculate Gross Sales	|
	-----------------------*/
	if (gst_div != 0.00 && !NOTAX)
		price = out_gst(inpr2_rec.base);
	else
		price = inpr2_rec.base;

	qty = coln_rec.ln_q_order;
	gross_sales = (price - ryhr_rec.hr_amt_extract) * (double) qty;
	gross_sales = no_dec(gross_sales);

	/*-----------------------
	| Calculate Nett  Sales	|
	-----------------------*/
	if (gst_div != 0.00 && !NOTAX)
		price = out_gst(coln_rec.ln_gross - coln_rec.ln_amt_disc);
	else
		price = coln_rec.ln_gross - coln_rec.ln_amt_disc;

	ext_extr = ryhr_rec.hr_amt_extract * (double) qty;
	ext_extr = no_dec(ext_extr);
	nett_sales = price - ext_extr;
	nett_sales = no_dec(nett_sales);

	switch (ryhr_rec.hr_basis[0])
	{
	/*--------------------------------
	| Royalty Based on Retail Price. |
	--------------------------------*/
	case	'R':
		roy = DOLLARS(gross_sales);

		roy_pc =  find_roy_pc();
		roy *= (double) roy_pc;

		break;

	/*------------------------------
	| Royalty Based on Nett Price. |
	------------------------------*/
	case	'N':
		roy = DOLLARS(nett_sales);

		roy_pc =  find_roy_pc();
		roy *= (double) roy_pc;

		break;

	/*----------------------------------
	| Royalty Based on Absolute Value. |
	----------------------------------*/
	case	'A':
		qty = coln_rec.ln_q_order;
		roy = (ryhr_rec.hr_abs_amt - ryhr_rec.hr_amt_extract) * (double) qty;
		break;

	case	'D':
	default:
		break;

	}
	roy = no_dec(roy);
}

/*==============================
| Update Royalty History File. |
==============================*/
void
upd_ryhs (
 void)
{
	strcpy (ryhs_rec.co_no,comm_rec.tco_no);
	ryhs_rec.hhry_hash = ryhr_rec.hr_hhry_hash;
	sprintf (ryhs_rec.publish,"%-3.3s%-1.1s",inmr_rec.mr_category + 1,inmr_rec.mr_category);
	ryhs_rec.roy_pc = roy_pc;
	strcpy (ryhs_rec.roy_basis,ryhr_rec.hr_basis);

	cc = find_rec("ryhs",&ryhs_rec,COMPARISON,"r");
	if (cc)
	{
		ryhs_rec.mtd_qty   = (INVOICE) ? qty   		: (qty   * -1);
		ryhs_rec.mtd_gross = (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.mtd_nett  = (INVOICE) ? nett_sales 	: (nett_sales * -1);
		ryhs_rec.mtd_roy   = (INVOICE) ? roy   		: (roy   * -1);
		ryhs_rec.ytd_qty   = (INVOICE) ? qty   		: (qty   * -1);
		ryhs_rec.ytd_gross = (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.ytd_nett  = (INVOICE) ? nett_sales 	: (nett_sales * -1);
		ryhs_rec.ytd_roy   = (INVOICE) ? roy   		: (roy   * -1);
		ryhs_rec.htd_qty   = (INVOICE) ? qty   		: (qty   * -1);
		ryhs_rec.htd_gross = (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.htd_nett  = (INVOICE) ? nett_sales 	: (nett_sales * -1);
		ryhs_rec.htd_roy   = (INVOICE) ? roy   		: (roy   * -1);
		cc = abc_add("ryhs",&ryhs_rec);
		if (cc) 
			sys_err("Error in ryhs During (DBADD)",cc,PNAME);
	}
	else
	{
		ryhs_rec.mtd_qty   += (INVOICE) ? qty   		 : (qty   * -1);
		ryhs_rec.mtd_gross += (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.mtd_nett  += (INVOICE) ? nett_sales  : (nett_sales * -1);
		ryhs_rec.mtd_roy   += (INVOICE) ? roy         : (roy   * -1);
		ryhs_rec.ytd_qty   += (INVOICE) ? qty         : (qty   * -1);
		ryhs_rec.ytd_gross += (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.ytd_nett  += (INVOICE) ? nett_sales  : (nett_sales * -1);
		ryhs_rec.ytd_roy   += (INVOICE) ? roy         : (roy   * -1);
		ryhs_rec.htd_qty   += (INVOICE) ? qty         : (qty   * -1);
		ryhs_rec.htd_gross += (INVOICE) ? gross_sales : (gross_sales * -1);
		ryhs_rec.htd_nett  += (INVOICE) ? nett_sales  : (nett_sales * -1);
		ryhs_rec.htd_roy   += (INVOICE) ? roy   	     : (roy   * -1);
		cc = abc_update("ryhs",&ryhs_rec);
		if (cc) 
			sys_err("Error in ryhs During (DBUPDATE)",cc,PNAME);
	}
}

int
find_dbry (
 char *cr_code)
{
	strcpy (dbry_rec.ry_co_no, comm_rec.tco_no);
	sprintf (dbry_rec.ry_cr_type,"%-3.3s",cr_code);
	return (cc = find_rec("dbry",&dbry_rec,COMPARISON,"r"));
}

/*=========================
| Find pricing structure. |
=========================*/
void
WsFindInpr (
 int price_type)
{
	inpr2_rec.hhbr_hash = coln_rec.ln_hhbr_hash;
	inpr2_rec.price_type = price_type + 1;
	inpr2_rec.hhgu_hash = 0L;
	strcpy (inpr2_rec.curr_code, cumr_rec.cm_curr_code);

	if ( !PriceLevel )
	{
		strcpy (inpr2_rec.br_no, "  ");
		strcpy (inpr2_rec.wh_no, "  ");
	}
	else 
	{
		strcpy (inpr2_rec.br_no, comm_rec.tes_no);
		strcpy (inpr2_rec.wh_no, ( PriceLevel == 2 ) ? comm_rec.tcc_no : "  ");
	}
	
	strcpy (inpr2_rec.cust_type, "   ");
	strcpy (inpr2_rec.area_code, "  ");
	cc = find_rec ("inpr", &inpr2_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (inpr2_rec.br_no,comm_rec.tes_no);
		strcpy (inpr2_rec.wh_no,"  ");
		cc = find_rec ("inpr", &inpr2_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (inpr2_rec.br_no,"  ");
			strcpy (inpr2_rec.wh_no,"  ");
		}
		cc = find_rec ("inpr", &inpr2_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (inpr2_rec.br_no, "  ");
			strcpy (inpr2_rec.wh_no, "  ");
			inpr2_rec.base     = 0.00;
		}
	}
}
