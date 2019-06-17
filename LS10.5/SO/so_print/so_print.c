/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_print.c      )                                |
|  Program Desc  : ( Print Sales Order.                           )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/88)      | Modified  by : B.C. Lim.         |
|  Date Modified : (25/02/89)      | Modified  by : Scott Darrow      |
|  Date Modified : (03/02/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (11/09/97)      | Modified  by : Marnie Organo.    |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (16/12/1997)    | Modified by : Scott B Darrow.    |
|                                                                     |
|  Comments      : Modified version of po_purlog.c                    |
|                : Tidy up program.                                   |
|                :                                                    |
|                :(25/02/89) - Added Tax and Disc Code                |
|  (03/02/93)    : PSL 6828 changes for mult-curr SO.                 |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|  (16/12/1997)  : Updated to re-apply modifications to program re UOM|
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_print.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_print/so_print.c,v 5.1 2001/08/09 09:21:41 scott Exp $";

#define		NO_SCRGEN
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>
#include <CustomerService.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		printerNumber = 1;	/* Line printer number			*/
	FILE	*pout;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 5;

	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
		char 	tco_short[16];
		long 	t_dbt_date;
	} comm_rec;

	/*===========================================
	| File inmr {Inventory Master Base Record}. |
	===========================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_std_uom"},
		{"inmr_sale_unit"},
		{"inmr_tax_amount"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 8;

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		char 	mr_description[41];
		long	mr_std_uom;
		char 	mr_sale_unit[5];
		double 	mr_tax_amt;
		char 	mr_stat_flag[2];
	} inmr_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_ch_adr4"},
		{"cumr_tax_code"},
		{"cumr_stat_flag"},
		{"cumr_curr_code"}
	};

	int cumr_no_fields = 13;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_acronym[10];
		char	cm_ch_adr[4][41];
		char	cm_tax_code[2];
		char	cm_stat_flag[2];
		char	cm_curr_code[4];
	} cumr_rec;

	/*==========================+
	 | Sales Order Header File. |
	 +==========================*/
#define	SOHR_NO_FIELDS	22

	struct dbview	sohr_list [SOHR_NO_FIELDS] =
	{
		{"sohr_co_no"},
		{"sohr_br_no"},
		{"sohr_order_no"},
		{"sohr_hhcu_hash"},
		{"sohr_hhso_hash"},
		{"sohr_cus_ord_ref"},
		{"sohr_cons_no"},
		{"sohr_del_zone"},
		{"sohr_carr_code"},
		{"sohr_dt_raised"},
		{"sohr_exch_rate"},
		{"sohr_cont_name"},
		{"sohr_cont_phone"},
		{"sohr_del_name"},
		{"sohr_del_add1"},
		{"sohr_del_add2"},
		{"sohr_del_add3"},
		{"sohr_din_1"},
		{"sohr_din_2"},
		{"sohr_din_3"},
		{"sohr_status"},
		{"sohr_stat_flag"}
	};

	struct tag_sohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	order_no [9];
		long	hhcu_hash;
		long	hhso_hash;
		char	cus_ord_ref [21];
		char	cons_no [17];
		char	del_zone [7];
		char	carr_code [5];
		Date	dt_raised;
		double	exch_rate;
		char	cont_name [21];
		char	cont_phone [16];
		char	del_adr [4][41];
		char	din_1 [61];
		char	din_2 [61];
		char	din_3 [61];
		char	status [2];
		char	stat_flag [2];
	}	sohr_rec;

	/*================================+
	 | Sales Order Detail Lines File. |
	 +================================*/
#define	SOLN_NO_FIELDS	15

	struct dbview	soln_list [SOLN_NO_FIELDS] =
	{
		{"soln_hhso_hash"},
		{"soln_line_no"},
		{"soln_hhbr_hash"},
		{"soln_hhcc_hash"},
		{"soln_hhum_hash"},
		{"soln_qty_order"},
		{"soln_qty_bord"},
		{"soln_sale_price"},
		{"soln_dis_pc"},
		{"soln_tax_pc"},
		{"soln_gst_pc"},
		{"soln_item_desc"},
		{"soln_due_date"},
		{"soln_status"},
		{"soln_stat_flag"}
	};

	struct tag_solnRecord
	{
		long	hhso_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhcc_hash;
		long	hhum_hash;
		float	qty_order;
		float	qty_bord;
		Money	sale_price;
		float	dis_pc;
		float	tax_pc;
		float	gst_pc;
		char	item_desc [41];
		Date	due_date;
		char	status [2];
		char	stat_flag [2];
	}	soln_rec;

	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
#define	INUM_NO_FIELDS	5

	struct dbview	inum_list [INUM_NO_FIELDS] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"}
	};

	struct tag_inumRecord
	{
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		char	desc [41];
		float	cnv_fct;
	}	inum_rec;


/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcessSalesHeader (long hhsoHash);
void ProcessSalesLines (long hhsoHash);
void PrintoutHeading (void);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	long	hhsoHash	=	0L;

	if (argc < 3)
	{
		print_at(0,0,mlSoMess725,argv[0]);
		return (EXIT_FAILURE);
	}

	printerNumber 	= atoi (argv[1]);
	hhsoHash 		= atol (argv[2]);

	OpenDB();

	dsp_screen(" Sales Order Print.",comm_rec.tco_no,comm_rec.tco_name);

	ProcessSalesHeader (hhsoHash);

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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec ("inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec ("inum", inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec ("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec ("sohr", sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec ("soln", soln_list, SOLN_NO_FIELDS, "soln_id_no");
		
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose ("inmr");
	abc_fclose ("inum");
	abc_fclose ("cumr");
	abc_fclose ("sohr");
	abc_fclose ("soln");

	abc_dbclose ("data");
}

/*============================
| Process sales orderheader. |
============================*/
void
ProcessSalesHeader (
 long	hhsoHash)
{
	/*--------------------------
	| Find sales order header. |
	--------------------------*/
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec ("sohr",&sohr_rec,COMPARISON,"r");
	if (cc)
		return;

	/*------------------------------
	| Find customer master record. |
	------------------------------*/
	cumr_rec.cm_hhcu_hash = sohr_rec.hhcu_hash;
	cc = find_rec ("cumr", &cumr_rec, COMPARISON, "r");
	if (cc)
		return;

	dsp_process("Sales Order : ",sohr_rec.order_no);

	PrintoutHeading ();
	
	ProcessSalesLines ( sohr_rec.hhso_hash );

	fprintf (pout, ".EOF\n");
	pclose (pout);

	/*-------------------------------------------
	| Create a log file record for sales Order. |
	-------------------------------------------*/
	LogCustService 
	(
		0L,
		sohr_rec.hhso_hash,
		cumr_rec.cm_hhcu_hash,
		sohr_rec.cus_ord_ref,
		sohr_rec.cons_no,
		sohr_rec.carr_code,
		sohr_rec.del_zone,
		LOG_ORD_PRINT
	);
}

/*============================
| Process sales order lines. |
============================*/
void
ProcessSalesLines (
 long	hhsoHash)
{
	double	workingQty 			= 	0.00;
	float	localConvFactor		=	0.00,
			conversionFactor	=	0.00;

	soln_rec.hhso_hash	= 	hhsoHash;
	soln_rec.line_no 	=	0;
	cc = find_rec ("soln",&soln_rec,GTEQ,"r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		inmr_rec.mr_hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec ("inmr",&inmr_rec, COMPARISON,"r");
		if (cc) 
			strcpy (inmr_rec.mr_item_no,"Unknown part no.");

		inum_rec.hhum_hash	=	inmr_rec.mr_std_uom;
		cc = find_rec ("inum",&inum_rec, EQUAL, "r");
		if (cc)
			conversionFactor = 0.00;
		
		conversionFactor = inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	soln_rec.hhum_hash;
		cc = find_rec ("inum",&inum_rec,EQUAL,"r");
		if (cc)
			strcpy (inum_rec.uom,"    ");

		if ( conversionFactor == 0.00 )
			conversionFactor = 1;

		localConvFactor = inum_rec.cnv_fct / conversionFactor;
		if (localConvFactor == 0.00)
			localConvFactor = 1.00;

		workingQty = (soln_rec.qty_order + soln_rec.qty_bord) / localConvFactor;

		fprintf (pout, "|%16.16s | %40.40s |%4.4s|%11.2f |\n",
						inmr_rec.mr_item_no,
						inmr_rec.mr_description,
						inum_rec.uom,
						workingQty);

		cc = find_rec ("soln",&soln_rec,NEXT,"r");
	}
}

void
PrintoutHeading (
 void)
{
	if ((pout = popen("pformat","w")) == 0) 
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout,".START%s<%s>\n", DateToString (comm_rec.t_dbt_date), PNAME);
	fprintf (pout,".LP%d\n",printerNumber);
	fprintf (pout,".17\n");
	fprintf (pout,".L100\n");
	fprintf (pout,".ESALES ORDER ACKNOWLEDGEMENT\n");
	fprintf (pout,".E%s \n",clip(comm_rec.tco_name));
	fprintf (pout,".EAS AT %s\n", SystemTime());
	fprintf (pout,".B1\n");


	fprintf (pout, ".R================================================================================\n");
	fprintf (pout, "================================================================================\n");
	fprintf (pout, "| Order Number     : %8.8s                                                  |\n",
					sohr_rec.order_no);
	fprintf (pout, "| Customer No.     : %6.6s (%40.40s)         |\n",
					cumr_rec.cm_dbt_no, cumr_rec.cm_dbt_name);
	fprintf (pout, "| Order Reference  : %20.20s                                      |\n",
					sohr_rec.cus_ord_ref);
	fprintf (pout, "| Order Date       : %10.10s                                                |\n", 
					DateToString(sohr_rec.dt_raised));
	fprintf (pout, "| Contact Name     : %20.20s      Phone No : %15.15s      |\n",
					sohr_rec.cont_name,
					sohr_rec.cont_phone);

	fprintf (pout, "| Charge To Address: %40.40s                  |\n",
					cumr_rec.cm_ch_adr[0]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					cumr_rec.cm_ch_adr[1]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					cumr_rec.cm_ch_adr[2]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					cumr_rec.cm_ch_adr[3]);

	fprintf (pout, "| Delivery Address : %40.40s                  |\n",
					sohr_rec.del_adr[0]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					sohr_rec.del_adr[1]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					sohr_rec.del_adr[2]);
	fprintf (pout, "|                  : %40.40s                  |\n",
					sohr_rec.del_adr[3]);

	fprintf (pout, "|------------------------------------------------------------------------------|\n");
	fprintf (pout, "|  ITEM NUMBER    |              ITEM DESCRIPTION            |UOM.|  QUANTITY  |\n");
	fprintf (pout, "|-----------------|------------------------------------------|----|------------|\n");
}
