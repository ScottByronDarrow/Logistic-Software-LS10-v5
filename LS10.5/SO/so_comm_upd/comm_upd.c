/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (so_comm_upd.c)                                    |
|  Program Desc  : (Update Sales Commission tables.             )     |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (25/06/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: comm_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_comm_upd/comm_upd.c,v 5.3 2001/08/26 22:46:30 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<twodec.h>

#define		DOLLAR  (coln_rec.crd_type[0] == 'D')
#define		INVOICE	 (transactionTypeFlag[0] == 'I' || \
					  transactionTypeFlag[0] == 'P')

#define		NOTAX	 (cohr_rec.tcode[0] == 'A' || cohr_rec.tcode[0] == 'B')

#define		CASH	 (atoi (cuin_rec.pay_terms) == 0)

#define		BLANK_SMAN	 (!strcmp (coln_rec.sman_code, "  "))

#define		X_RATE		 (cohr_rec.exch_rate)

#define		COM_SMAN			1
#define		COM_MANAGER			2

#define		MAX_SALESMAN		200
#define		MAX_SMAN_LEVEL		30

	FILE	*fsort,
			*fout;

	/*==================================
	| file comm { System Common file } |
	==================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		long	tdbt_date;
	} comm_rec;

	/*======================================+
	 | Customer Payments Header File Record. |
	 +======================================*/
#define	CUHD_NO_FIELDS	27

	struct dbview	cuhd_list [CUHD_NO_FIELDS] =
	{
		{"cuhd_hhcu_hash"},
		{"cuhd_receipt_no"},
		{"cuhd_cheque_no"},
		{"cuhd_or_no"},
		{"cuhd_type"},
		{"cuhd_bank_id"},
		{"cuhd_bank_amt"},
		{"cuhd_bank_exch"},
		{"cuhd_bank_chg"},
		{"cuhd_rec_type"},
		{"cuhd_alt_drawer"},
		{"cuhd_due_date"},
		{"cuhd_present_date"},
		{"cuhd_hhcp_hash"},
		{"cuhd_narrative"},
		{"cuhd_date_payment"},
		{"cuhd_date_posted"},
		{"cuhd_tot_amt_paid"},
		{"cuhd_loc_amt_paid"},
		{"cuhd_disc_given"},
		{"cuhd_loc_disc_give"},
		{"cuhd_exch_variance"},
		{"cuhd_lodge_flag"},
		{"cuhd_clear_fee"},
		{"cuhd_db_bank"},
		{"cuhd_db_branch"},
		{"cuhd_stat_flag"}
	};

	struct tag_cuhdRecord
	{
		long	hhcu_hash;
		char	receipt_no [9];
		char	cheque_no [21];
		char	or_no [11];
		char	type [2];
		char	bank_id [6];
		Money	bank_amt;
		double	bank_exch;
		Money	bank_chg;
		char	rec_type [2];
		char	alt_drawer [21];
		Date	due_date;
		Date	present_date;
		long	hhcp_hash;
		char	narrative [21];
		Date	date_payment;
		Date	date_posted;
		Money	tot_amt_paid;
		Money	loc_amt_paid;
		Money	disc_given;
		Money	loc_disc_give;
		Money	exch_variance;
		char	lodge_flag [2];
		Money	clear_fee;
		char	db_bank [4];
		char	db_branch [21];
		char	stat_flag [2];
	}	cuhd_rec;

	/*=================================+
	 | Customer Payments Detail Record. |
	 +=================================*/
#define	CUDT_NO_FIELDS	8

	struct dbview	cudt_list [CUDT_NO_FIELDS] =
	{
		{"cudt_hhcp_hash"},
		{"cudt_hhci_hash"},
		{"cudt_hhdt_hash"},
		{"cudt_amt_paid_inv"},
		{"cudt_loc_paid_inv"},
		{"cudt_exch_variatio"},
		{"cudt_exch_rate"},
		{"cudt_stat_flag"}
	};

	struct tag_cudtRecord
	{
		long	hhcp_hash;
		long	hhci_hash;
		long	hhdt_hash;
		Money	amt_paid_inv;
		Money	loc_paid_inv;
		Money	exch_variatio;
		double	exch_rate;
		char	stat_flag [2];
	}	cudt_rec;

	/*===============================+
	 | Sales commission Header file. |
	 +===============================*/
#define	SACH_NO_FIELDS	11

	struct dbview	sach_list [SACH_NO_FIELDS] =
	{
		{"sach_sach_hash"},
		{"sach_hhsf_hash"},
		{"sach_hhcu_hash"},
		{"sach_hhci_hash"},
		{"sach_inv_amt"},
		{"sach_amt"},
		{"sach_com_val"},
		{"sach_com_rate"},
		{"sach_sman_pos"},
		{"sach_sale_flag"},
		{"sach_status"}
	};

	struct tag_sachRecord
	{
		long	sach_hash;
		long	hhsf_hash;
		long	hhcu_hash;
		long	hhci_hash;
		Money	inv_amt;
		Money	amt;
		Money	com_val;
		float	com_rate;
		char	sman_pos [3];
		char	sale_flag [2];
		char	status [2];
	}	sach_rec;

	/*===============================+
	 | Sales commission Detail file. |
	 +===============================*/
#define	SACL_NO_FIELDS	6

	struct dbview	sacl_list [SACL_NO_FIELDS] =
	{
		{"sacl_sach_hash"},
		{"sacl_hhcp_hash"},
		{"sacl_rec_amt"},
		{"sacl_rec_date"},
		{"sacl_com_amt"},
		{"sacl_status"}
	};

	struct tag_saclRecord
	{
		long	sach_hash;
		long	hhcp_hash;
		Money	rec_amt;
		Date	rec_date;
		Money	com_amt;
		char	status [2];
	}	sacl_rec;

	/*============================================
	| Customer Order/Invoice/Credit Header File. |
	============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_tax_code"},
		{"cohr_sale_code"},
		{"cohr_gross"},
		{"cohr_freight"},
		{"cohr_other_cost_1"},
		{"cohr_other_cost_2"},
		{"cohr_other_cost_3"},
		{"cohr_tax"},
		{"cohr_gst"},
		{"cohr_disc"},
		{"cohr_erate_var"},
		{"cohr_exch_rate"},
		{"cohr_stat_flag"},
		{"cohr_status"}
	};
	int cohr_no_fields = 22;
	struct {
		char	co_no[3];
		char	br_no[3];
		char	dp_no[3];
		char	inv_no[9];
		long	hhcu_hash;
		char	type[2];
		long	hhco_hash;
		long	date_raised;
		char	tcode[2];
		char	sale_code[3];
		Money	gross;
		Money	freight;
		Money	other_cost[3];	
		Money	tax;
		Money	gst;
		Money	disc;	
		double	erate_var;
		Money	exch_rate;
		char	stat_flag[2];
		char	status[2];
	} cohr_rec;

	/*============================================
	| Customer Order/Invoice/Credit Detail File. |
	============================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_incc_hash"},
		{"coln_crd_type"},
		{"coln_q_order"},
		{"coln_q_backorder"},
		{"coln_sale_price"},
		{"coln_cost_price"},
		{"coln_disc_pc"},
		{"coln_gross"},
		{"coln_amt_disc"},
		{"coln_erate_var"},
		{"coln_sman_code"},
		{"coln_stat_flag"}
	};

	int coln_no_fields = 15;

	struct {
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	incc_hash;
		char	crd_type[2];
		float	q_order;
		float	q_backorder;
		Money	sale_price;
		Money	cost_price;
		float	disc_pc;
		Money	gross;	
		Money	amt_disc;
		Money	erate_var;
		char	sman_code[3];
		char	stat_flag[2];
	} coln_rec;

	/*================================+
	 | External Category File Record. |
	 +================================*/
#define	EXCF_NO_FIELDS	4

	struct dbview	excf_list [EXCF_NO_FIELDS] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_hhcf_hash"},
		{"excf_stat_flag"}
	};

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		long	hhcf_hash;
		char	stat_flag [2];
	}	excf_rec;

	/*==========================================
	| file inmr {Inventory Master Base Record} |
	==========================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_class"},
		{"inmr_category"},
		{"inmr_outer_size"},
		{"inmr_sellgrp"},
		{"inmr_stat_flag"},
	};

	int inmr_no_fields = 9;

	struct {
		char 	co_no[3];
		char 	item_no[17];
		long 	hhbr_hash;
		long 	hhsi_hash;
		char 	_class[2];
		char 	category[12];
		float 	outer_size;
		char 	sellgrp[7];
		char 	stat_flag[2];
	} inmr_rec;

	/*====================================
	| file cumr {Customer Master Record} |
	====================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_acronym"},
		{"cumr_hhcu_hash"},
		{"cumr_class_type"},
		{"cumr_sman_code"},
	};

	int cumr_no_fields = 7;

	struct {
		char 	co_no[3];
		char 	est_no[3];
		char 	dbt_no[7];
		char 	dbt_acronym[10];
		long 	hhcu_hash;
		char 	class_type[4];
		char 	sman_code[3];
	} cumr_rec;

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	26

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_hhsf_hash"},
		{"exsf_logname"},
		{"exsf_salesman"},
		{"exsf_sell_type"},
		{"exsf_sell_grp"},
		{"exsf_sell_pos"},
		{"exsf_area_code"},
		{"exsf_route_no"},
		{"exsf_carr_code"},
		{"exsf_up_sman1"},
		{"exsf_up_sman2"},
		{"exsf_up_sman3"},
		{"exsf_sale_stat"},
		{"exsf_com_status"},
		{"exsf_com_type"},
		{"exsf_com_pc"},
		{"exsf_com_min"},
		{"exsf_sman_com"},
		{"exsf_lev1_com"},
		{"exsf_lev2_com"},
		{"exsf_lev3_com"},
		{"exsf_lev4_com"},
		{"exsf_stat_flag"},
		{"exsf_update"}
	};

	struct tag_exsfRecord
	{
		char	co_no [3];
		char	sman_no [3];
		long	hhsf_hash;
		char	logname [15];
		char	name [41];
		char	sell_type [3];
		char	sell_grp [3];
		char	sell_pos [3];
		char	area_code [3];
		char	route_no [9];
		char	carr_code [5];
		long	up_sman[3];
		char	sale_stat [2];
		char	com_status [2];
		char	com_type [2];
		float	com_pc;
		Money	com_min;
		float	sman_com;
		float	level[4];
		char	stat_flag [2];
		char	update [2];
	} exsf_rec, exsf2_rec;

	/*================================================
	| Customer Invoice Accounting Invoice/Credit file |
	================================================*/
	struct dbview cuin_list [] =
	{
		{"cuin_hhcu_hash"},
		{"cuin_hhci_hash"},
		{"cuin_type"},
		{"cuin_est"},
		{"cuin_inv_no"},
		{"cuin_date_of_inv"},
		{"cuin_pay_terms"},
		{"cuin_disc"},
		{"cuin_amt"},
		{"cuin_stat_flag"}
	};

	int	cuin_no_fields = 10;

	struct tag_cuinRecord
	{
		long	hhcu_hash;
		long	hhci_hash;
		char	type[2];
		char	est [3];
		char	inv_no [11];
		long	doi;
		char	pay_terms [4];
		double	disc;		/* Money Type */
		double	amt;		/* Money Type */
		char	stat_flag [2];
	} cuin_rec;

	/*========================================
	| General Ledger Journal Control Record. |
	========================================*/
	struct dbview gljc_list[] ={
		{"gljc_co_no"},
		{"gljc_journ_type"},
		{"gljc_run_no"},
		{"gljc_stat_flag"}
	};

	int gljc_no_fields = 4;

	struct {
		char	jc_co_no[3];
		char	jc_journ_type[3];
		long	jc_run_no;
		char	jc_stat_flag[2];
	} gljc_rec;

	/*=====================================+
	 | Customers Salesman Commission file. |
	 +=====================================*/
#define	CUSC_NO_FIELDS	15

	struct dbview	cusc_list [CUSC_NO_FIELDS] =
	{
		{"cusc_hhsf_hash"},
		{"cusc_alloc_type"},
		{"cusc_sellgrp"},
		{"cusc_category"},
		{"cusc_spec_no"},
		{"cusc_ud_code"},
		{"cusc_hhbr_hash"},
		{"cusc_com_type"},
		{"cusc_com_pc"},
		{"cusc_com_min"},
		{"cusc_sman_com"},
		{"cusc_lev1_com"},
		{"cusc_lev2_com"},
		{"cusc_lev3_com"},
		{"cusc_lev4_com"}
	};

	struct tag_cuscRecord
	{
		long	hhsf_hash;
		int		alloc_type;
		char	sellgrp [7];
		char	category [12];
		int		spec_no;
		char	ud_code [3];
		long	hhbr_hash;
		char	com_type [2];
		float	com_pc;
		Money	com_min;
		float	sman_com;
		float	level[4];
	}	cusc_rec;

	/*==============================================+
	 | Inventory User Defined Item allocation file. |
	 +==============================================*/
#define	IUDI_NO_FIELDS	4

	struct dbview	iudi_list [IUDI_NO_FIELDS] =
	{
		{"iudi_hhcf_hash"},
		{"iudi_hhbr_hash"},
		{"iudi_spec_no"},
		{"iudi_code"}
	};

	struct tag_iudiRecord
	{
		long	hhcf_hash;
		long	hhbr_hash;
		int		spec_no;
		char	code [3];
	}	iudi_rec;

	int		ReportPrinted = FALSE;

	char	*srt_offset[256];

	char	*data 	= 	"data",
			*comm 	= 	"comm",
			*exsf 	= 	"exsf",
			*exsf2 	= 	"exsf2",
			*excf 	= 	"excf",
			*cohr 	= 	"cohr",
			*coln 	= 	"coln",
			*cuin 	= 	"cuin",
			*cumr 	= 	"cumr",
			*sach 	= 	"sach",
			*sacl 	= 	"sacl",
			*cudt 	= 	"cudt",
			*cuhd 	= 	"cuhd",
			*gljc	= 	"gljc",
			*inmr 	= 	"inmr",
			*iudi 	= 	"iudi",
			*cusc 	= 	"cusc";

	short	tmp_dmy[3];

	float	gstInclude = 0.00;
	float	gstDivide = 0.00;

	int		printerNumber = 1;

	double	totalCommission [3];
	double	totalSalesman [3];

	char	passedBranchNumber[3];
	char	transactionTypeFlag[2];
	char	findStatusFlag[2];
	char	updateStatusFlag[2];

	int		multiCurrency;
	int		nettPriceUsed	= TRUE;
    int     Level = 0;

	double	UpdComm = 0.00;

	struct	store_rec {
		long	Salesman;
		int		Level;
	} store [MAX_SALESMAN];


/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessFile 	(void);
void 	ProcessColn 	(void);
double 	OutGst 			(double);
void 	AddSach 		(long, double, double, int, float);
void 	OpenAudit 		(void);
void 	PrintAudit 		(void);
void 	SalesmanTotal 	(void);
void 	PrintFooter 	(void);
void 	GetCommission 	(long, int, double, double, double, double, int);
void 	StoreLine 		(char *,int,char *,double, float, double, double, int);
void 	AddCommDetail 	(long, long, double);
char 	*_sort_read 	(FILE *);
void 	ResetCUSC 		(void);
void 	ProcessLevels 	(long);
void 	AddSalesman 	(long, int);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

	if (argc < 5)
	{
		print_at (0,0, "Usage : %s <findStatusFlag> <updateStatusFlag> <transactionTypeFlag)> <LPNO> <Optional branch.>\007\n\r", argv[0]);
		return (EXIT_FAILURE);
	}

	gstInclude = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (gstInclude != 0.00)
		gstDivide = (float) ((100.00 + gstInclude) / gstInclude);

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	multiCurrency = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_NETT_USED");
	nettPriceUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);


	OpenDB ();

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;

	sprintf (findStatusFlag, 	  "%1.1s", argv[1]);
	sprintf (updateStatusFlag, 	  "%1.1s", argv[2]);
	sprintf (transactionTypeFlag, "%1.1s", argv[3]);
	printerNumber = atoi (argv[4]);
	
	sprintf (err_str," Processing %s to user specific update.",
					 (INVOICE) ? "Invoices" : "Credits");

	init_scr ();
	print_mess (err_str);

	if (argc == 6)
		sprintf (passedBranchNumber, "%-2.2s", argv[5]);
	else
		sprintf (passedBranchNumber, "%-2.2s", comm_rec.tes_no);

	ProcessFile ();

	if (ReportPrinted == TRUE)
		PrintFooter ();

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
	abc_dbopen (data);

	abc_alias (exsf2, exsf);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);


	open_rec (cohr,  cohr_list, cohr_no_fields, "cohr_up_id");
	open_rec (coln,  coln_list, coln_no_fields, "coln_id_no");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (cuin,  cuin_list, cuin_no_fields, "cuin_id_no2");
	open_rec (cumr,  cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec (cusc,  cusc_list, CUSC_NO_FIELDS, "cusc_id_no2");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (exsf2, exsf_list, EXSF_NO_FIELDS, "exsf_hhsf_hash");
	open_rec (gljc,  gljc_list, gljc_no_fields, "gljc_id_no");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (iudi,  iudi_list, IUDI_NO_FIELDS, "iudi_id_no");
	open_rec (sach,  sach_list, SACH_NO_FIELDS, "sach_id_no");
	open_rec (sacl,  sacl_list, SACL_NO_FIELDS, "sacl_id_no");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{

	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cuin);
	abc_fclose (cumr);
	abc_fclose (cusc);
	abc_fclose (excf);
	abc_fclose (exsf);
	abc_fclose (exsf2);
	abc_fclose (gljc);
	abc_fclose (inmr);
	abc_fclose (iudi);
	abc_fclose (sach);
	abc_fclose (sacl);
	abc_dbclose (data);
}

/*===================================================
| Process whole coln file looking for passed status |
| flag and processing accordingly.                  |
===================================================*/
void
ProcessFile (
 void)
{
	int		ok = TRUE;
	
	fsort = sort_open ("so_comm_aud");

	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	while (ok) 
	{
		strcpy (cohr_rec.co_no,     comm_rec.tco_no);
		strcpy (cohr_rec.br_no,     passedBranchNumber);
		strcpy (cohr_rec.type,      transactionTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatusFlag);
		cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
		if (cc) 
		{
			abc_unlock ("cohr");
			ok = FALSE;
			continue;
		}
		ProcessColn ();
	}
	abc_unlock ("cohr");
	PrintAudit ();

	sort_delete (fsort,"so_comm_aud");
}

/*===============================
| Process all lines on Invoice. |
===============================*/
void
ProcessColn (
 void)
{
	double	wk_value 	= 0.00;
	double	tot_sale 	= 0.00;
	double	tot_disc 	= 0.00;
	double	tot_avge 	= 0.00;
	double	loc_gross 	= 0.00;
	double	loc_nett 	= 0.00;
	double	loc_disc 	= 0.00;
	double	InvoiceAmt 	= 0.00;
	int		i;

	coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
   	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
   	{
		if (BLANK_SMAN)
			strcpy (coln_rec.sman_code, cohr_rec.sale_code);

		/*-------------------------------
		| Find Inventory Master Record. |
		-------------------------------*/
		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", coln_rec.hhbr_hash);
		if (cc) 
		{
			strcpy (inmr_rec._class,    "D");
			strcpy (inmr_rec.category, "DELETED    ");
		}
		strcpy (excf_rec.co_no, cohr_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			excf_rec.hhcf_hash	=	0L;

		/*-----------------------------------------------------------
		| If not a sales analysis item and it is a substitute item. |
		-----------------------------------------------------------*/
		else if (!cc && inmr_rec.hhsi_hash != 0L)
		{
			cc = find_hash (inmr, &inmr_rec,EQUAL, "r", inmr_rec.hhsi_hash);
			if (cc)
				cc = find_hash (inmr, &inmr_rec,EQUAL, "r",coln_rec.hhbr_hash);
		}
		if (cc) 
		{
			strcpy (inmr_rec._class,    "D");
			strcpy (inmr_rec.category, "DELETED    ");
		}
		/*-----------------------
		| Find customer record. |
		-----------------------*/
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no, "DELETE");
			strcpy (cumr_rec.class_type, "DEL");
		}
		/*-------------------------------
		| Find customer invoice record. |
		-------------------------------*/
		cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuin_rec.inv_no, cohr_rec.inv_no);
		cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (cuin);
			cc = find_rec (coln, &coln_rec, NEXT, "u");
			continue;
		}
		InvoiceAmt 	= (nettPriceUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

		/*--------------------------------------------------
		| Ensure salesman internal memory structure clear. |
		--------------------------------------------------*/
		for (i = 0; i < MAX_SALESMAN; i++)
			memset (&store[i],0,sizeof (struct store_rec));

		/*-----------------------------------------
		| Find salesman record from invoice line. |
		-----------------------------------------*/
		strcpy (exsf_rec.co_no, cohr_rec.co_no);
		sprintf (exsf_rec.sman_no, "%-2.2s", coln_rec.sman_code); 
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------------------------------
			| Could not find so try sales from invoice header. |
			--------------------------------------------------*/
			strcpy (exsf_rec.co_no, cohr_rec.co_no);
			sprintf (exsf_rec.sman_no, "%-2.2s", cohr_rec.sale_code); 
			cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
			if (cc)
      	 		file_err (cc, exsf, "DBFIND");
		}
		/*------------------------ 
		| Not on file so create .|
		------------------------*/
		wk_value = out_cost (coln_rec.cost_price, inmr_rec.outer_size);

		tot_sale = (coln_rec.gross + coln_rec.erate_var);
		tot_avge = (wk_value * coln_rec.q_order);
		tot_disc = coln_rec.amt_disc;

		if (gstDivide != 0.00)
		{
			tot_sale = OutGst (tot_sale);
			tot_disc = OutGst (tot_disc);
		}
		if (cohr_rec.exch_rate == 0.00)
			cohr_rec.exch_rate = 1.00;

		loc_gross = no_dec (tot_sale / X_RATE);
		loc_nett  = no_dec ((tot_sale - tot_disc) / X_RATE);
		loc_disc  = no_dec ((tot_disc) / X_RATE);

		InvoiceAmt 	= (nettPriceUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

		/*------------------------------------
		| Process multi level salesman tree. |
		------------------------------------*/
		ProcessLevels (exsf_rec.hhsf_hash);

		/*---------------------------------------------------------
		| Process all salesman stored from ProcessLevels routine. |
		---------------------------------------------------------*/
		for (i = 0; i < MAX_SALESMAN; i++)
		{
			exsf2_rec.hhsf_hash	=	store [i].Salesman;
			cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
			if (!cc)
			{
				/*---------------------
				| Process Commission. |
				---------------------*/
				GetCommission 
				 (
					store [i].Salesman,
					store [i].Level,
					InvoiceAmt,
					loc_gross, 
					loc_nett , 
					tot_avge,
					 (exsf2_rec.hhsf_hash == exsf_rec.hhsf_hash) ? 
								COM_SMAN : COM_MANAGER 
				);
			}
		}

		strcpy (coln_rec.stat_flag, updateStatusFlag);
		cc = abc_update (coln, &coln_rec);
		if (cc)
			file_err (cc, coln, "DBUPDATE");

		cc = find_rec (coln, &coln_rec, NEXT, "u");
   	}
	abc_unlock (coln);

	strcpy (cohr_rec.stat_flag, updateStatusFlag);
	cc = abc_update (cohr, &cohr_rec);
	if (cc) 
      	 file_err (cc, cohr, "DBUPDATE");
}

/*===========================================
| Extract out GST for gst inclusive Prices. |
===========================================*/
double 
OutGst (
 double	 	total_amt)
{
	double	gst_amount = 0.00;

	if (total_amt == 0.00)
		return (0.00);

	if (NOTAX)
		return (total_amt);

	gst_amount = no_dec (total_amt / gstDivide);
	
	total_amt -= no_dec (gst_amount);

	return (total_amt);
}
/*=================================
| Update sales Commission record. |
=================================*/
void	
AddSach (
 long	hhsf_hash, 
 double	InvTotal,
 double	CommPay,
 int	SalesPos,
 float	CommPc)
{
	int		NewRecord;

	if (CommPay <= 0.00 || CommPc <= 0.00)
		return;

	sach_rec.hhsf_hash = hhsf_hash;
	sach_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sach_rec.hhci_hash = cuin_rec.hhci_hash;
	NewRecord = find_rec (sach, &sach_rec, COMPARISON, "u");

	sach_rec.inv_amt = (INVOICE) ? no_dec (InvTotal) : no_dec (InvTotal * -1);
	
	exsf2_rec.hhsf_hash = hhsf_hash;
	cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf2, "DBFIND");

	sach_rec.com_rate = CommPc;

	if (SalesPos == COM_SMAN)
		strcpy (sach_rec.sale_flag,"S");

	if (SalesPos == COM_MANAGER)
		strcpy (sach_rec.sale_flag,"M");

	if (NewRecord)
	{
		sach_rec.com_val	=	 (INVOICE) ? CommPay : CommPay * -1;
	}
	else
	{
		if (INVOICE)
		{
			sach_rec.com_val	+=	no_dec (CommPay);
		}
		else
		{
			sach_rec.com_val	-=	no_dec (CommPay);
		}
	}
	if (NewRecord)
	{
		sach_rec.hhci_hash	=	cuin_rec.hhci_hash;
		abc_unlock (sach);
		cc = abc_add (sach, &sach_rec);

		sach_rec.hhsf_hash = hhsf_hash;
		sach_rec.hhcu_hash = cumr_rec.hhcu_hash;
		sach_rec.hhci_hash = cuin_rec.hhci_hash;
		strcpy (sach_rec.sman_pos, exsf2_rec.sell_pos);
		strcpy (sach_rec.status, "0");
		cc = find_rec (sach, &sach_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "sach", "DBFIND");
	}
	else
	{
		sach_rec.hhci_hash = cuin_rec.hhci_hash;
		strcpy (sach_rec.sman_pos, exsf2_rec.sell_pos);
		cc = abc_update (sach, &sach_rec);
	}
	if (cc)
		file_err (cc, sach, "DBADD/DBUPDATE");

	if (CASH)
	{
		AddCommDetail
		 (	
			sach_rec.hhcu_hash, 
			sach_rec.hhci_hash,
			sach_rec.inv_amt 
		);
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
	strcpy (gljc_rec.jc_co_no, comm_rec.tco_no);

	if (INVOICE)
		strcpy (gljc_rec.jc_journ_type, " 4");
	else
		strcpy (gljc_rec.jc_journ_type, " 5");

	cc = find_rec (gljc, &gljc_rec, COMPARISON, "r"); 
	if (cc)
		file_err (cc, gljc, "DBFIND");

	gljc_rec.jc_run_no++;

	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".14\n");
	fprintf (fout, ".L158\n");
	if (INVOICE)
	{
	       fprintf (fout, ".ESALESMAN COMMISSION POSTING AUDIT.\n");
	       fprintf (fout, ".EINVOICE COMMISSION RUN NUMBER %06ld\n",gljc_rec.jc_run_no);
	}
	else
	{
	       fprintf (fout, ".ESALESMAN COMMISSION POSTING AUDIT\n");
	       fprintf (fout, ".ECREDIT COMMISSION RUN NUMBER %06ld\n",gljc_rec.jc_run_no);
	}

	fprintf (fout, ".CCommission type M (argin), N (ett), G (ross)\n");
	fprintf (fout, ".E%s\n",clip (comm_rec.tco_name));
	fprintf (fout, ".E at at %s\n", SystemTime ());
	if (multiCurrency)
		fprintf (fout, ".EALL VALUES ARE IN LOCAL CURRENCY\n");
	else
		fprintf (fout, ".B1\n");

	fprintf (fout, ".EBranch %s\n", clip (comm_rec.tes_name));

	fprintf (fout, ".R=================================================");
	fprintf (fout, "=================");
	fprintf (fout, "=============");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");

	fprintf (fout, "=================================================");
	fprintf (fout, "=================");
	fprintf (fout, "=============");
	fprintf (fout, "======");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");


	fprintf (fout, "| INVOICE  |CUSTOMER| CUSTOMERS |  NETT INVOICE  ");
	fprintf (fout, "|  ITEM  NUMBER  ");
	fprintf (fout, "| COMMISSION ");
	fprintf (fout, "|LEVEL");
	fprintf (fout, "|COMM");
	fprintf (fout, "| COMM. AMOUNT ");
	fprintf (fout, "|  VALUE COMM. ");
	fprintf (fout, "|  COMMISSION  |\n");

	fprintf (fout, "|  NUMBER  | NUMBER | ACRONYM   |    AMOUNT      ");
	fprintf (fout, "|                ");
	fprintf (fout, "| ALLOCATION ");
	fprintf (fout, "| NO. ");
	fprintf (fout, "|TYPE");
	fprintf (fout, "| / PERCENT    ");
	fprintf (fout, "|   BASED ON   ");
	fprintf (fout, "|   PAYABLE    |\n");

	fprintf (fout, "|----------|--------|-----------|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------|\n");

	ReportPrinted = TRUE;
}

/*======================================================
| Print Audit of Commission for Each Salesman/Invoice. |
======================================================*/
void
PrintAudit (
 void)
{
	char	*sptr;
	char	*tptr;
	int		FirstRecord = TRUE;
	char	new_sman[3],
			old_sman[3],
			sman_name[41];

	totalCommission [0] = 0.00;
	totalSalesman [0] = 0.00;

	fsort = sort_sort (fsort, "so_comm_aud");

	sptr = _sort_read (fsort);
	while (sptr != (char *) 0)
	{
		tptr = strdup (sptr);

		if (FirstRecord)
			OpenAudit ();
	
		sprintf (new_sman, 	"%-2.2s", 	srt_offset[0]);
		sprintf (sman_name, "%-40.40s", srt_offset[1]);

		if (FirstRecord)
		{
			fprintf (fout, ".PD| %2.2s - %40.40s  ", new_sman, sman_name);
			fprintf (fout, "                 ");
			fprintf (fout, "             ");
			fprintf (fout, "      ");
			fprintf (fout, "     ");
			fprintf (fout, "               ");
			fprintf (fout, "               ");
			fprintf (fout, "               |\n");
			strcpy (old_sman, new_sman);
		}
		else
		{
			if (strcmp (new_sman, old_sman))
			{
				SalesmanTotal ();

				strcpy (old_sman, new_sman);
				fprintf (fout, ".PD| %2.2s - %40.40s  ", new_sman, sman_name);
				fprintf (fout, "                 ");
				fprintf (fout, "             ");
				fprintf (fout, "      ");
				fprintf (fout, "     ");
				fprintf (fout, "               ");
				fprintf (fout, "               ");
				fprintf (fout, "               |\n");
				fprintf (fout, ".PA\n");
			}
		}
		fprintf (fout, "| %8.8s ", 		srt_offset[2]);
		fprintf (fout, "| %6.6s ",   	srt_offset[3]);
		fprintf (fout, "| %9.9s ",   	srt_offset[4]);
		fprintf (fout, "| %14.2f ",		atof (srt_offset[8]));
		fprintf (fout, "|%16.16s", 		srt_offset[5]);
		fprintf (fout, "|%-12.12s", 	srt_offset[7]);
		fprintf (fout, "|  %d  ", 		atoi (srt_offset[12]) + 1);
		fprintf (fout, "| %-1.1s  ",	srt_offset[6]);
		fprintf (fout, "|%14.2f", 		atof (srt_offset[9]));
		fprintf (fout, "|%14.2f",		atof (srt_offset[10]));
		fprintf (fout, "|%14.2f|\n",	atof (srt_offset[11]));

		totalCommission [0] += atof (srt_offset[11]);
		totalSalesman [0] += atof (srt_offset[11]);

		FirstRecord  = FALSE;

		sptr = _sort_read (fsort);
	}
}

/*========================
| Print Salesman Totals. |
========================*/
void
SalesmanTotal (
 void)
{
	fprintf (fout, "|  SALESPERSON TOTAL            |                ");
	fprintf (fout, "|                ");
	fprintf (fout, "|            ");
	fprintf (fout, "|     ");
	fprintf (fout, "|    ");
	fprintf (fout, "|              ");
	fprintf (fout, "|              ");
	fprintf (fout, "|%14.2f|\n", totalSalesman [0]);

	totalSalesman [0] = 0.00;
}

/*=====================
| Print Page footers. |
=====================*/
void
PrintFooter (
 void)
{
	SalesmanTotal ();

	fprintf (fout, "|==========|========|===========|================");
	fprintf (fout, "|================");
	fprintf (fout, "|============");
	fprintf (fout, "|=====");
	fprintf (fout, "|====");
	fprintf (fout, "|==============");
	fprintf (fout, "|==============");
	fprintf (fout, "|==============|\n");

	fprintf (fout, "|  GRAND TOTAL                  |                ");
	fprintf (fout, "|                ");
	fprintf (fout, "|            ");
	fprintf (fout, "|     ");
	fprintf (fout, "|    ");
	fprintf (fout, "|              ");
	fprintf (fout, "|              ");
	fprintf (fout, "|%14.2f|\n", totalCommission [0]);

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*==========================================
| Loop up commission records for salesman. |
==========================================*/
void
GetCommission (
 long	hhsf_hash, 
 int	Level, 
 double	InvTotal, 
 double	Gross, 
 double	Nett, 
 double	Cost, 
 int	SalesPos)
{
	double	CommAmt		=	0.00;
	double	CommPay		=	0.00;
	double	TotCommPay	=	0.00;
	float	CommPc		=	0.00;
	int		CommFound 	=	FALSE;
	int		UDByItem	=	FALSE;

	/*-------------------------------------
	| Look for Commission by Item Number. |
	-------------------------------------*/
	ResetCUSC ();

	cusc_rec.hhsf_hash 	=	hhsf_hash;
	cusc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
	if (!cc)
	{
		if (SalesPos == COM_MANAGER)
			CommPc = (Level < 4) ? cusc_rec.level [Level] : cusc_rec.level [3];
		else
			CommPc = cusc_rec.sman_com; 

		/*-----------------------------------------------------------------
		| Commission is by gross. i.e percentage of Gross invoice amount. |
		-----------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'G')
		{
			CommAmt = Gross;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Gross;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}

		/*---------------------------------------------------------------
		| Commission is by nett. i.e percentage of Nett invoice amount. |
		---------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'N')
		{
			CommAmt = Nett;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		
		/*---------------------------------------
		| Commission is based on a % of margin. |
		---------------------------------------*/
		if (cusc_rec.com_type[0] == 'M')
		{
			CommAmt = Nett - Cost;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett - Cost;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		/*-----------------------------------------------
		| Store commission line for printing of report. |
		-----------------------------------------------*/
		StoreLine
		 (
			"ITEM NUMBER ", 
			Level,
			cusc_rec.com_type,
			DOLLARS (InvTotal),
			CommPc,
			CommAmt,
			DOLLARS (CommPay),
			SalesPos
		);
		/*-------------------------------------------
		| Add data to sales commission header file. |
		-------------------------------------------*/
		AddSach
		 (
			hhsf_hash,
			InvTotal,
			CommPay,
			SalesPos,
			CommPc 
		);
		CommFound 	=	TRUE;
		return;
	}
	/*---------------------------------------
	| Look for Commission by Selling group. |
	---------------------------------------*/
	ResetCUSC ();

	cusc_rec.hhsf_hash 	=	hhsf_hash;
	strcpy (cusc_rec.sellgrp, inmr_rec.sellgrp);
	cc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
	if (!cc)
	{
		if (SalesPos == COM_MANAGER)
			CommPc = (Level < 4) ? cusc_rec.level [Level] : cusc_rec.level [3];
		else
			CommPc = cusc_rec.sman_com; 

		/*-----------------------------------------------------------------
		| Commission is by gross. i.e percentage of Gross invoice amount. |
		-----------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'G')
		{
			CommAmt = Gross;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Gross;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}

		/*---------------------------------------------------------------
		| Commission is by nett. i.e percentage of Nett invoice amount. |
		---------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'N')
		{
			CommAmt = Nett;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		
		/*---------------------------------------
		| Commission is based on a % of margin. |
		---------------------------------------*/
		if (cusc_rec.com_type[0] == 'M')
		{
			CommAmt = Nett - Cost;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett - Cost;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		/*-----------------------------------------------
		| Store commission line for printing of report. |
		-----------------------------------------------*/
		StoreLine
		 (
			"SELLING GRP.", 
			Level,
			cusc_rec.com_type,
			DOLLARS (InvTotal),
			CommPc,
			CommAmt,
			DOLLARS (CommPay),
			SalesPos
		);

		TotCommPay	+=	CommPay;
		CommFound 	=	TRUE;
	}
	/*----------------------------------
	| Look for Commission by Category. |
	----------------------------------*/
	ResetCUSC ();

	cusc_rec.hhsf_hash 	=	hhsf_hash;
	strcpy (cusc_rec.category, inmr_rec.category);
	cc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
	if (!cc)
	{
		if (SalesPos == COM_MANAGER)
			CommPc = (Level < 4) ? cusc_rec.level [Level] : cusc_rec.level [3];
		else
			CommPc = cusc_rec.sman_com; 

		/*-----------------------------------------------------------------
		| Commission is by gross. i.e percentage of Gross invoice amount. |
		-----------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'G')
		{
			CommAmt = Gross;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Gross;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}

		/*---------------------------------------------------------------
		| Commission is by nett. i.e percentage of Nett invoice amount. |
		---------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'N')
		{
			CommAmt = Nett;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		
		/*---------------------------------------
		| Commission is based on a % of margin. |
		---------------------------------------*/
		if (cusc_rec.com_type[0] == 'M')
		{
			CommAmt = Nett - Cost;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett - Cost;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		/*-----------------------------------------------
		| Store commission line for printing of report. |
		-----------------------------------------------*/
		StoreLine
		 (
			"CATEGORY.   ", 
			Level,
			cusc_rec.com_type,
			DOLLARS (InvTotal),
			CommPc,
			CommAmt,
			DOLLARS (CommPay),
			SalesPos
		);
					
		TotCommPay	+=	CommPay;
		CommFound 	=	TRUE;
	}

	/*----------------------------------------------------
	| Look for Commission by User defined codes by item. |
	----------------------------------------------------*/
	ResetCUSC ();

	iudi_rec.hhcf_hash	=	0L;
	iudi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	iudi_rec.spec_no	=	0;
	strcpy (iudi_rec.code, "  ");
	cc = find_rec (iudi, &iudi_rec, GTEQ, "r");
	while (!cc && iudi_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		cusc_rec.hhsf_hash 	=	hhsf_hash;
		cusc_rec.spec_no	=	iudi_rec.spec_no;
		strcpy (cusc_rec.ud_code, iudi_rec.code);
		cc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
		if (!cc)
		{
			if (SalesPos == COM_MANAGER)
				CommPc = (Level < 4) ? cusc_rec.level [Level] 
									 : cusc_rec.level [3];
			else
				CommPc = cusc_rec.sman_com; 

			/*-----------------------------------------------------------------
			| Commission is by gross. i.e percentage of Gross invoice amount. |
			-----------------------------------------------------------------*/
			if (cusc_rec.com_type[0] == 'G')
			{
				CommAmt = Gross;
				CommAmt = DOLLARS (CommAmt);
				CommPay = Gross;
				CommPay *= (double) CommPc;
				CommPay = DOLLARS (CommPay);
			}

			/*---------------------------------------------------------------
			| Commission is by nett. i.e percentage of Nett invoice amount. |
			---------------------------------------------------------------*/
			if (cusc_rec.com_type[0] == 'N')
			{
				CommAmt = Nett;
				CommAmt = DOLLARS (CommAmt);
				CommPay = Nett;
				CommPay *= (double) CommPc;
				CommPay = DOLLARS (CommPay);
			}
			
			/*---------------------------------------
			| Commission is based on a % of margin. |
			---------------------------------------*/
			if (cusc_rec.com_type[0] == 'M')
			{
				CommAmt = Nett - Cost;
				CommAmt = DOLLARS (CommAmt);
				CommPay = Nett - Cost;
				CommPay *= (double) CommPc;
				CommPay = DOLLARS (CommPay);
			}
			sprintf (err_str,"UDC %2d - %2.2s ",iudi_rec.spec_no,iudi_rec.code);
			/*-----------------------------------------------
			| Store commission line for printing of report. |
			-----------------------------------------------*/
			StoreLine
			 (
				err_str,
				Level,
				cusc_rec.com_type,
				DOLLARS (InvTotal),
				CommPc,
				CommAmt,
				DOLLARS (CommPay),
				SalesPos
			);
						
			TotCommPay	+=	CommPay;
			CommFound 	=	TRUE;
			UDByItem	=	TRUE;
		}
		cc = find_rec (iudi, &iudi_rec, NEXT, "r");
	}
	/*--------------------------------------------------------
	| Look for Commission by User defined codes by category. |
	--------------------------------------------------------*/
	if (UDByItem == FALSE)
	{
		ResetCUSC ();

		iudi_rec.hhcf_hash	=	excf_rec.hhcf_hash;
		iudi_rec.hhbr_hash	=	0L;
		iudi_rec.spec_no	=	0;
		strcpy (iudi_rec.code, "  ");
		cc = find_rec (iudi, &iudi_rec, GTEQ, "r");
		while (!cc && iudi_rec.hhcf_hash == excf_rec.hhcf_hash)
		{
			cusc_rec.hhsf_hash 	=	hhsf_hash;
			cusc_rec.spec_no	=	iudi_rec.spec_no;
			strcpy (cusc_rec.ud_code, iudi_rec.code);
			cc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
			if (!cc)
			{
				if (SalesPos == COM_MANAGER)
					CommPc = (Level < 4) ? cusc_rec.level [Level] 
									 	 : cusc_rec.level [3];
				else
					CommPc = cusc_rec.sman_com; 

				/*--------------------------------------------------------
				| Commission is by gross. i.e % of Gross invoice amount. |
				--------------------------------------------------------*/
				if (cusc_rec.com_type[0] == 'G')
				{
					CommAmt = Gross;
					CommAmt = DOLLARS (CommAmt);
					CommPay = Gross;
					CommPay *= (double) CommPc;
					CommPay = DOLLARS (CommPay);
				}

				/*------------------------------------------------------
				| Commission is by nett. i.e % of Nett invoice amount. |
				------------------------------------------------------*/
				if (cusc_rec.com_type[0] == 'N')
				{
					CommAmt = Nett;
					CommAmt = DOLLARS (CommAmt);
					CommPay = Nett;
					CommPay *= (double) CommPc;
					CommPay = DOLLARS (CommPay);
				}
				
				/*---------------------------------------
				| Commission is based on a % of margin. |
				---------------------------------------*/
				if (cusc_rec.com_type[0] == 'M')
				{
					CommAmt = Nett - Cost;
					CommAmt = DOLLARS (CommAmt);
					CommPay = Nett - Cost;
					CommPay *= (double) CommPc;
					CommPay = DOLLARS (CommPay);
				}
				sprintf (err_str,"UDC %2d - %2.2s ",iudi_rec.spec_no,iudi_rec.code);
				/*-----------------------------------------------
				| Store commission line for printing of report. |
				-----------------------------------------------*/
				StoreLine
				 (
					err_str,
					Level,
					cusc_rec.com_type,
					DOLLARS (InvTotal),
					CommPc,
					CommAmt,
					DOLLARS (CommPay),
					SalesPos
				);
							
				TotCommPay	+=	CommPay;
				UDByItem	=	TRUE;
			}
			cc = find_rec (iudi, &iudi_rec, NEXT, "r");
		}
	}
	/*--------------------------------------
	| No Commission found to use standard. |
	--------------------------------------*/
	if (!CommFound)
	{
		if (SalesPos == COM_SMAN)
		{
			CommPc = exsf_rec.sman_com; 
			strcpy (cusc_rec.com_type,exsf_rec.com_type);	
		} 
			
		if (SalesPos == COM_MANAGER)
		{
			CommPc = (Level < 4) ? exsf2_rec.level [Level] : exsf2_rec.level [3];
			strcpy (cusc_rec.com_type,exsf2_rec.com_type);	
		}
		
		/*-----------------------------------------------------------------
		| Commission is by gross. i.e percentage of Gross invoice amount. |
		-----------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'G')
		{
			CommAmt = Gross;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Gross;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}

		/*---------------------------------------------------------------
		| Commission is by nett. i.e percentage of Nett invoice amount. |
		---------------------------------------------------------------*/
		if (cusc_rec.com_type[0] == 'N')
		{
			CommAmt = Nett;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		
		/*---------------------------------------
		| Commission is based on a % of margin. |
		---------------------------------------*/
		if (cusc_rec.com_type[0] == 'M')
		{
			CommAmt = Nett - Cost;
			CommAmt = DOLLARS (CommAmt);
			CommPay = Nett - Cost;
			CommPay *= (double) CommPc;
			CommPay = DOLLARS (CommPay);
		}
		/*-----------------------------------------------
		| Store commission line for printing of report. |
		-----------------------------------------------*/
		StoreLine
		 (
			"SMAN DEFAULT", 
			Level,
			cusc_rec.com_type,
			DOLLARS (InvTotal),
			CommPc,
			CommAmt,
			DOLLARS (CommPay),
			SalesPos
		);
			
		TotCommPay	+=	CommPay;
		CommFound 	=	TRUE;
	}
	/*-------------------------------------------
	| Add data to sales commission header file. |
	-------------------------------------------*/
	AddSach
	 (
		hhsf_hash,
		InvTotal,
		TotCommPay,
		SalesPos,
		CommPc
	);
	return;
}

/*==================================================
| Store line for printing of sorted details later. |
==================================================*/
void	
StoreLine (
	char	*CommDesc,
	int		Level,
	char	*CommType,
	double	InvAmt,
	float	CommRate,
	double	CommAmt,
	double	CommPay,
	int		SalesPos)
{
	char	sman_no [3];
	char	sman_name [41];
    char    cBuffer[256];

	if (CommAmt <= 0.00 || 
		 CommPay <= 0.00 || 
		 CommRate == 0.00)
		return;
	
	if (SalesPos == COM_SMAN) 	
	{
		strcpy (sman_no,exsf_rec.sman_no);
		strcpy (sman_name,exsf_rec.name);
	}
	else
	{
		strcpy (sman_no,exsf2_rec.sman_no);
		strcpy (sman_name,exsf2_rec.name);
	}
	sprintf 
	(
		cBuffer, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%f%c%f%c%f%c%f%c%d%c%s\n",
		sman_no,								1,	/* srt_offset = 0	*/
		sman_name,								1,	/* srt_offset = 1	*/
		cuin_rec.inv_no,						1,	/* srt_offset = 2	*/
		cumr_rec.dbt_no,						1,	/* srt_offset = 3	*/
		cumr_rec.dbt_acronym,					1,	/* srt_offset = 4	*/
		inmr_rec.item_no,						1,	/* srt_offset = 5	*/
		CommType,								1,	/* srt_offset = 6	*/
		CommDesc,								1,	/* srt_offset = 7	*/
		(INVOICE) ? InvAmt 	: InvAmt * -1,		1,	/* srt_offset = 8	*/
		CommRate,								1,	/* srt_offset = 9	*/
		(INVOICE) ? CommAmt : CommAmt * -1,		1,	/* srt_offset = 10	*/
		(INVOICE) ? CommPay : CommPay * -1,		1,	/* srt_offset = 11	*/
		Level,									1,	/* srt_offset = 12	*/
		" "
	);
    sort_save (fsort, cBuffer);
}

/*=============================================================
| Add Commission record for receipts made against an invoice. |
=============================================================*/
void
AddCommDetail (
 long	hhcu_hash, 
 long	hhci_hash, 
 double	pay_amt)
{
	double	CommAmt    = 0.00;

	cudt_rec.hhci_hash = cuin_rec.hhci_hash; 
	cc = find_rec (cudt, &cudt_rec, EQUAL, "r");
	if (cc)
		return;

	cuhd_rec.hhcp_hash	=	cudt_rec.hhcp_hash;
	cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
	if (cc)
		return;

	CommAmt    =  sach_rec.com_val;
	
	sacl_rec.sach_hash	=	sach_rec.sach_hash;
	sacl_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	sacl_rec.rec_amt	=	pay_amt;
	sacl_rec.rec_date	=	cuin_rec.doi;
	sacl_rec.com_amt	=	twodec (CommAmt);
	strcpy (sacl_rec.status, "0");

	cc = find_rec (sacl, &sacl_rec, COMPARISON, "u");
	if (cc)
	{
		sacl_rec.sach_hash	=	sach_rec.sach_hash;
		sacl_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		sacl_rec.rec_amt	=	pay_amt;
		sacl_rec.rec_date	=	cuin_rec.doi;
		sacl_rec.com_amt	=	twodec (CommAmt);
		strcpy (sacl_rec.status, "0");

		abc_unlock ("sacl");
		cc = abc_add (sacl, &sacl_rec);
		if (cc)
			file_err (cc, "sacl", "DBADD");
	}
	else
	{
		sacl_rec.sach_hash	=	sach_rec.sach_hash;
		sacl_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		sacl_rec.rec_amt	=	pay_amt;
		sacl_rec.rec_date	=	cuin_rec.doi;
		sacl_rec.com_amt	=	twodec (CommAmt);
		strcpy (sacl_rec.status, "0");

		cc = abc_update (sacl, &sacl_rec);
		if (cc)
			file_err (cc, "sacl", "DBUPDATE");
	}
}

/*========================================
| Save offsets for each numerical field. |
========================================*/
/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char *
_sort_read (
 FILE *srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset[0] = sptr;

	tptr = sptr;
	while (fld_no < 13)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset[fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}

/*===============================================
| Clear out Customer commission file structure. |
===============================================*/
void
ResetCUSC (
 void)
{
	memset (&cusc_rec,0,sizeof (cusc_rec));
	strcpy (cusc_rec.sellgrp, "      ");
	strcpy (cusc_rec.category, "           ");
	strcpy (cusc_rec.ud_code, "  ");
	strcpy (cusc_rec.com_type, " ");
}

/*=================================
| Process the levels of salesman. |
=================================*/
void
ProcessLevels (
 long	hhsfHash)
{
	int		i;
	long	SavedSalesmanHash;

	if (Level > MAX_SMAN_LEVEL)
		return;
	
	exsf2_rec.hhsf_hash	=	hhsfHash;
	cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
	if (!cc && exsf2_rec.hhsf_hash	== 	hhsfHash)
	{
		if (exsf2_rec.com_status[0] == 'C')
			AddSalesman (exsf2_rec.hhsf_hash, Level);

		for (i = 0; i < 3; i++)
		{
			Level++;
			SavedSalesmanHash	=	exsf2_rec.hhsf_hash;

			if (exsf2_rec.up_sman[i] > 0L)
				ProcessLevels (exsf2_rec.up_sman[i]);

			Level--;
			exsf2_rec.hhsf_hash	=	SavedSalesmanHash;
			cc = find_rec (exsf2, &exsf2_rec, COMPARISON, "r");
		}
	}
}

/*=======================================================================
| Add Salesman to Array and ensure each salesman is captured only once. |
=======================================================================*/
void
AddSalesman (
 long	hhsfHash,
 int	LEVEL)
{
	int		i;

	for (i = 0; i < MAX_SALESMAN; i++)
	{
		if (store [i].Salesman == 0L)
		{
			store [i].Salesman 	= hhsfHash;
			store [i].Level 	= LEVEL;
			return;
		}
		if (store [i].Salesman == hhsfHash)
			return;
	}
}
