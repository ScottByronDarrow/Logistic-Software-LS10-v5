/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (so_invmhup.c                                      |
|  Program Desc  : (Update Machine History File From cohr,coln    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, coln, cohr, inmr, mhdr,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr, mhdr,     ,     ,     ,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/12/87         |
|---------------------------------------------------------------------|
|  Date Modified : (07/02/88)      | Modified  by  : Scott Darrow.n.  |
|  Date Modified : (19/06/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (28/01/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (16/09/93)      | Modified  by  : Aroha Merrilees. |
|  Date Modified : (01/10/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      :                                                    |
|     (19/06/89) : Read invoice lines (coln) using coln_id_no instead |
|                : of index on coln_hhco_hash.                        |
|                :                                                    |
|     (28/01/93) : Update for multi-currency order entry. SC 6828 PSL |
|                :                                                    |
|  (16/09/93)    : HGP 9503 - increased cus_ord_ref to 20 chars.      |
|  (01/10/97)    : Updated to incorporate multilingual conversion.    |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_invmhup.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_invmhup/so_invmhup.c,v 5.2 2001/08/26 22:46:39 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <twodec.h>

	/*==================================
	| file comm { System Common file } |
	==================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"}
	};

	int comm_no_fields = 4;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
	} comm_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_category"},
		{"inmr_serial_item"}
	};

	int inmr_no_fields = 4;

	struct {
		long	hhbr_hash;
		char	_class[2];
		char	category[12];
		char	serial_item[2];
	} inmr_rec;

	/*=====================================
	| Machine History Detail Record File. |
	=====================================*/
	struct dbview mhdr_list[] ={
		{"mhdr_co_no"},
		{"mhdr_hhcc_hash"},
		{"mhdr_hhbr_hash"},
		{"mhdr_serial_no"},
		{"mhdr_prod_gp"},
		{"mhdr_order_no"},
		{"mhdr_order_date"},
		{"mhdr_sell_date"},
		{"mhdr_hhcu_hash"},
		{"mhdr_cust_type"},
		{"mhdr_cust_area"},
		{"mhdr_rep_no"},
		{"mhdr_inv_no"},
		{"mhdr_val_nzd"},
	};

	int mhdr_no_fields = 14;

	struct {
		char	co_no[3];
		long	hhcc_hash;
		long	hhbr_hash;
		char	serial_no[26];
		char	prod_gp[13];
		char	order_no[17];
		long	order_date;
		long	sell_date;
		long	hhcu_hash;
		char	cust_type[4];
		char	cust_area[3];
		char	rep_no[3];
		char	inv_no[9];
		double	val_nzd;		/*  Money field  */
	} mhdr_rec;

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
		{"cohr_cus_ord_ref"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
		{"cohr_area_code"},
		{"cohr_sale_code"},
		{"cohr_exch_rate"},
		{"cohr_stat_flag"},
	};

	int cohr_no_fields = 14;

	struct {
		char	co_no[3];
		char	br_no[3];
		char	dp_no[3];
		char	inv_no[9];
		long	hhcu_hash;
		char	type[2];
		char	cus_ord_ref[21];
		long	hhco_hash;
		long	date_raised;
		long	date_required;
		char	area_code[3];
		char	sale_code[3];
		double	exch_rate;
		char	stat_flag[2];
	} cohr_rec;

	/*============================================
	| Customer Order/Invoice/Credit Detail File. |
	============================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_incc_hash"},
		{"coln_serial_no"},
		{"coln_cost_price"},
		{"coln_gross"},
		{"coln_due_date"},
	};

	int coln_no_fields = 8;

	struct {
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		char	serial_no[26];
		Money	cost_price;
		Money	gross;	
		long	due_date;
	} coln_rec;

	/*====================================
	| file cumr {Customer Master Record} |
	====================================*/
	struct dbview cumr_list[] ={
		{"cumr_hhcu_hash"},
		{"cumr_class_type"},
		{"cumr_curr_code"}
	};

	int cumr_no_fields = 3;

	struct {
		long	cm_hhcu_hash;
		char	cm_class_type[4];
		char	cm_curr_code[4];
	} cumr_rec;

	char	*data = "data",
			*comm = "comm",
			*cohr = "cohr",
			*coln = "coln",
			*cumr = "cumr",
			*inmr = "inmr",
			*mhdr = "mhdr";

	char	passedBranchNumber [3],
			findStatusFLag [2],
			updateStatusFlag [2],
			transactionTypeFlag [2];


/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcessHeader (void);
void ProcessLines (long hhcoHash);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	if (argc < 4)
	{
		print_at (0,0,"Usage : %s <findStatusFlag> <updateStatusFlag> <transactionTypeFlag> <Optional - branchNumber>",argv[0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	init_scr ();
	print_mess (ML("Processing Invoices to Machine History."));

	sprintf (findStatusFLag,     "%1.1s", argv[1]);
	sprintf (updateStatusFlag,   "%1.1s", argv[2]);
	sprintf (transactionTypeFlag,"%1.1s", argv[3]);

	if (argc == 5)
		sprintf (passedBranchNumber, "%2.2s", argv[4]);
	else
		sprintf (passedBranchNumber, "%2.2s", comm_rec.tes_no);

	ProcessHeader ();

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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (cohr, cohr_list, cohr_no_fields, "cohr_up_id");
	open_rec (coln, coln_list, coln_no_fields, "coln_id_no");
	open_rec (cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (mhdr, mhdr_list, mhdr_no_fields, "mhdr_serial_id");
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
	abc_fclose (mhdr);
	abc_fclose (cumr);
	abc_fclose (inmr);

	abc_dbclose (data);
}

void
ProcessHeader (
 void)
{
	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	strcpy (cohr_rec.co_no,     comm_rec.tco_no);
	strcpy (cohr_rec.br_no,     passedBranchNumber);
	strcpy (cohr_rec.type,      transactionTypeFlag);
	strcpy (cohr_rec.stat_flag, findStatusFLag);
	cc = find_rec ("cohr",&cohr_rec,COMPARISON,"u");
	while (!cc)
	{
		cumr_rec.cm_hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,"Error in cumr (%06ld) During (DBFIND)",cohr_rec.hhcu_hash);
			errmess (err_str);
		}
	
		ProcessLines (cohr_rec.hhco_hash);

		abc_unlock (cohr);

		strcpy (cohr_rec.co_no,     comm_rec.tco_no);
		strcpy (cohr_rec.br_no,     passedBranchNumber);
		strcpy (cohr_rec.type,      transactionTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatusFLag);
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
	}
}

void
ProcessLines (
 long	hhcoHash)
{
	int	new_item = 0;

	if (cohr_rec.exch_rate == 0.00)
		cohr_rec.exch_rate = 1.00;

	coln_rec.hhco_hash	= hhcoHash;
	coln_rec.line_no		= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash) 	
	{
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc || inmr_rec.serial_item[0] == 'N')
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
		strcpy (mhdr_rec.co_no, comm_rec.tco_no);
		mhdr_rec.hhcc_hash = coln_rec.hhcc_hash;
		mhdr_rec.hhbr_hash = coln_rec.hhbr_hash;
		strcpy (mhdr_rec.serial_no, coln_rec.serial_no);
		new_item = find_rec (mhdr, &mhdr_rec, COMPARISON, "r");
		
		strcpy (mhdr_rec.co_no, comm_rec.tco_no);
		mhdr_rec.hhcc_hash = coln_rec.hhcc_hash;
		mhdr_rec.hhbr_hash = coln_rec.hhbr_hash;
		strcpy (mhdr_rec.serial_no,coln_rec.serial_no);
		sprintf (mhdr_rec.prod_gp, "%1.1s%11.11s", inmr_rec._class,
												   inmr_rec.category);
		strncpy (mhdr_rec.order_no,cohr_rec.cus_ord_ref,16);
		mhdr_rec.order_date	= cohr_rec.date_raised;
		mhdr_rec.sell_date	= coln_rec.due_date;
		mhdr_rec.hhcu_hash	= cohr_rec.hhcu_hash;
		strcpy (mhdr_rec.cust_type, cumr_rec.cm_class_type);
		strcpy (mhdr_rec.cust_area, cohr_rec.area_code);
		strcpy (mhdr_rec.rep_no,    cohr_rec.sale_code);
		strcpy (mhdr_rec.inv_no,    cohr_rec.inv_no);
		mhdr_rec.val_nzd = no_dec (coln_rec.gross / cohr_rec.exch_rate);
		if (new_item)
		{
		    cc = abc_add (mhdr, &mhdr_rec);
		    if (cc)
			    file_err (cc, mhdr, "DBADD");
		}
		else
		{
		    cc = abc_update (mhdr, &mhdr_rec);
		    if (cc)
			    file_err (cc, mhdr, "DBUPDATE");
		}
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	strcpy (cohr_rec.stat_flag , updateStatusFlag);
	
	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, cohr, "DBUPDATE");
}
