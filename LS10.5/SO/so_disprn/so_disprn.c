/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_disprn.c   )                                  |
|  Program Desc  : ( Discount Structure Printout.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, excl, excf, inds, ccmr, inmr,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 04/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : 08/11/88        | Modified  by  : B. C. Lim.       |
|  Date Modified : 24/01/89        | Modified  by  : B. C. Lim.       |
|  Date Modified : (23/11/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (22/06/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (14/08/92)      | Modified  by  : Trevor van Bremen|
|  Date Modified : (09/11/93)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (15/03/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : Change to use new screen generator.                |
|                : Remove init_scr & set_tty from program so that if  |
|                : background is run, screen doesn't hang up.         |
|                : (23/11/90) - Updated for new index on inds.        |
|                : (22/06/92) - print disc by cust by item not prntng |
|                :              -signs wrong way around in proc_inmr  |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (09/11/93)    : HGP 9501 Update for new inds table.                |
|  (15/03/94)    : HGP 9501 Fix bug in selection criteria.			  |
|  (11/09/97)    : Updated for Multilingual Conversion.               |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: so_disprn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_disprn/so_disprn.c,v 5.4 2002/07/17 09:58:07 scott Exp $";

#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<twodec.h>
#include 	<dsp_process2.h>
#include	<get_lpno.h>
#include 	<pr_format3.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_inv_date"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
	};

	int comm_no_fields = 17;

	struct {
		int	term;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
		char	tprice_desc[9][16];
	} comm_rec;

	/*=================================
	| Customer Class Type Master File. |
	=================================*/
	struct dbview excl_list[] ={
		{"excl_co_no"},
		{"excl_class_type"},
		{"excl_class_desc"},
		{"excl_stat_flag"}
	};

	int excl_no_fields = 4;

	struct {
		char	cl_co_no[3];
		char	cl_cus_type[4];
		char	cl_class_desc[41];
		char	cl_stat_flag[2];
	} excl_rec;


	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
		{"excf_stat_flag"}
	};

	int excf_no_fields = 4;

	struct {
		char	cf_co_no[3];
		char	cf_categ_no[12];
		char	cf_categ_desc[41];
		char	cf_stat_flag[2];
	} excf_rec;

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
		{"cumr_acc_type"},
		{"cumr_class_type"}
	};

	int cumr_no_fields = 8;

	struct {
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_dbt_no[7];
		long	mr_hhcu_hash;
		char	mr_dbt_name[41];
		char	mr_dbt_acronym[10];
		char	mr_acc_type[2];
		char	mr_cus_type[4];
	} cumr_rec;

	/*==================================
	| Customer Discount Subranges File |
	==================================*/
	struct dbview inds_list [] =
	{
		{"inds_co_no"},
		{"inds_br_no"},
		{"inds_hhcu_hash"},
		{"inds_price_type"},
		{"inds_category"},
		{"inds_sel_group"},
		{"inds_cust_type"},
		{"inds_hhbr_hash"},
		{"inds_hhcc_hash"},
		{"inds_disc_by"},
		{"inds_qty_brk1"},
		{"inds_qty_brk2"},
		{"inds_qty_brk3"},
		{"inds_qty_brk4"},
		{"inds_qty_brk5"},
		{"inds_qty_brk6"},
		{"inds_disca_pc1"},
		{"inds_disca_pc2"},
		{"inds_disca_pc3"},
		{"inds_disca_pc4"},
		{"inds_disca_pc5"},
		{"inds_disca_pc6"},
		{"inds_discb_pc1"},
		{"inds_discb_pc2"},
		{"inds_discb_pc3"},
		{"inds_discb_pc4"},
		{"inds_discb_pc5"},
		{"inds_discb_pc6"},
		{"inds_discc_pc1"},
		{"inds_discc_pc2"},
		{"inds_discc_pc3"},
		{"inds_discc_pc4"},
		{"inds_discc_pc5"},
		{"inds_discc_pc6"},
		{"inds_cum_disc"}
	};

	int	inds_no_fields = 35;

	struct tag_indsRecord
	{
		char	co_no [3];
		char	br_no [3];
		long	hhcu_hash;
		int	 	price_type;
		char	category [12];
		char	sel_group [7];
		char	cust_type [4];
		long	hhbr_hash;
		long	hhcc_hash;
		char	disc_by [2];
		double	qty_brk[6];
		float	disca_pc[6];
		float	discb_pc[6];
		float	discc_pc[6];
		char	cum_disc [2];
	} inds_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_type"},
	};

	int ccmr_no_fields = 7;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_cc_no[3];
		long	cm_hhcc_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_type[3];
	} ccmr_rec, ccmr2_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
	};

	int inmr_no_fields = 3;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
	} inmr_rec;

	int		rep_type = 0;
	int 	envDbFind = 0;
	int 	envDbCo = 0;
	int 	first_time = 1;
	int 	ctype_cust = 1;
	int 	lp_no = 1;
	int		sk_dbprinum;

	int		prev_ptype;
	char	prev_ctype[4];
	char	prev_cust[7];
	char	prev_category[12];
	char	prev_selgrp[7];
	char	prev_item[17];
	char	branchNo[3];
	char	br_no[3];
	char	wh_no[3];
	char 	qwerty;

	char	*comm	= "comm";
	char	*ccmr	= "ccmr";
	char	*excl	= "excl";
	char	*excf	= "excf";
	char	*cumr	= "cumr";
	char	*inds	= "inds";
	char	*inmr	= "inmr";
	char	*data	= "data";

	long	prev_hash = 0L;

	FILE	*fin;
	FILE	*pp;
	
	static char *rep_desc[] = {
		"DISCOUNTS BY CUSTOMER AND ITEM", 
		"DISCOUNTS BY CUSTOMER AND SELLING GROUP",
		"DISCOUNTS BY CUSTOMER AND CATEGORY",
		"DISCOUNTS BY CUSTOMER-TYPE AND ITEM",
		"DISCOUNTS BY CUSTOMER-TYPE AND SELLING GROUP",
		"DISCOUNTS BY CUSTOMER-TYPE AND CATEGORY",
		"DISCOUNTS BY PRICE-TYPE AND ITEM", 
		"DISCOUNTS BY PRICE-TYPE AND SELLING GROUP",
		"DISCOUNTS BY PRICE-TYPE AND CATEGORY",
	};

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	back_desc[5];
	char	rep_type[9][5];
	char	rep_type_desc[9][5];
	char	onight[5];
	char	onight_desc[5];
	int		lpno;
	char	lp_str[3];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "lpno",	 4, 36, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 5, 36, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "back_desc", 5, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",	 6, 36, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc", 6, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc},
	{1, LIN, "dis",	 8, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer     /Item. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[0]},
	{1, LIN, "dis_desc", 8, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[0]},
	{1, LIN, "dis",	 9, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer     /Selling Group. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[1]},
	{1, LIN, "dis_desc", 9, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[1]},
	{1, LIN, "dis",	10, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer     /Category. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[2]},
	{1, LIN, "dis_desc", 10, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[2]},
	{1, LIN, "dis",	11, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer Type/Item. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[3]},
	{1, LIN, "dis_desc", 11, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[3]},
	{1, LIN, "dis",	12, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer Type/Selling Group. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[4]},
	{1, LIN, "dis_desc", 12, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[4]},
	{1, LIN, "dis",	13, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Customer Type/Category. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[5]},
	{1, LIN, "dis_desc", 13, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[5]},
	{1, LIN, "dis",	14, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Price Type   /Item. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[6]},
	{1, LIN, "dis_desc", 14, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[6]},
	{1, LIN, "dis",	15, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Price Type   /Selling Group. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[7]},
	{1, LIN, "dis_desc", 15, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[7]},
	{1, LIN, "dis",	16, 36, CHARTYPE,
		"U", "          ",
		" ", "Y", "Disc By Price Type   /Category. ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rep_type[8]},
	{1, LIN, "dis_desc", 16, 39, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc[8]},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void shutdown_prog (void);
int  spec_valid (int field);
void head (int prnt_no);
void procCustItem (void);
void procCustCat (void);
void procCustSelgp (void);
void procCtypeItem (void);
void procCtypeCat (void);
void procCtypeSelgp (void);
void procPtypeItem (void);
void procPtypeCat (void);
void procPtypeSelgp (void);
void printPtypeHdr (void);
void printCtypeHdr (void);
void printItemDetail (void);
void printSelgrpDetail (void);
void printCatDetail (void);
void printCustHdr (void);
int  find_inmr (long hhbr_hash);
void setup_dlt (void);
int  check_page (void);
void run_prog (void);
int  heading (int scn);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

	envDbFind  = atoi (get_env ("DB_FIND"));
	envDbCo = atoi (get_env ("DB_CO"));

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		sk_dbprinum = 5;
	else
	{
		sk_dbprinum = atoi (sptr);
		if (sk_dbprinum > 9 || sk_dbprinum < 1)
			sk_dbprinum = 9;
	}

	OpenDB ();


	strcpy (branchNo, (envDbCo) ? comm_rec.test_no : " 0");

	if ( argc < 2 )
	{
		init_scr ();

		SETUP_SCR (vars);

		set_tty ();  
		set_masks ();
		init_vars (1);

		/*=====================
		| Reset control flags |
		=====================*/
   		entry_exit = 1;
   		prog_exit = 0;
   		restart = 0;
	
		setup_dlt ();

		/*-----------------------------
		| Edit screen 1 linear input. |
		-----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		prog_exit = 1;

		strcpy(err_str,ML(mlSoMess395));

		rset_tty ();

		if (!restart) 
			run_prog ();

		FinishProgram ();

		return (EXIT_SUCCESS);
	}

	init_scr ();
	/*----------------------------------------------------------
	| Report Types are as follows :                            |
    |                                                          |
 	| 1 = Discount By Customer      / Item.                    |
 	| 2 = Discount By Customer      / Selling Group.           |
 	| 3 = Discount By Customer      / Category.                |
 	| 4 = Discount By Customer Type / Item.                    |
 	| 5 = Discount By Customer Type / Selling Group.           |
 	| 6 = Discount By Customer Type / Category.                |
 	| 7 = Discount By Price Type    / Item.                    |
 	| 8 = Discount By Price Type    / Selling Group.           |
 	| 9 = Discount By Price Type    / Category.                |
	----------------------------------------------------------*/

	if (argc < 3)
	{
	    print_at (0,0,mlSoMess712, argv[0]);
	    shutdown_prog();
        return (EXIT_FAILURE);
	}

	lp_no    = atoi (argv[1]);
	rep_type = atoi (argv[2]);
	if (rep_type < 1 || rep_type > 9)
	{
		print_at (1,0,mlSoMess705, argv[0]);
 		print_at (2,0,mlSoMess713);
 		print_at (3,0,mlSoMess714);
 		print_at (4,0,mlSoMess715);
 		print_at (5,0,mlSoMess716);
 		print_at (6,0,mlSoMess717);
 		print_at (7,0,mlSoMess718);
 		print_at (8,0,mlSoMess719);
 		print_at (9,0,mlSoMess720);
 		print_at (10,0,mlSoMess721);
		return (EXIT_FAILURE);
	}

	switch (rep_type)
	{
	case 1:
		dsp_screen ("Printing Discount By Customer / Item.",
				     comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 2:
		dsp_screen ("Printing Discount By Customer / Selling Group.",
			comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 3:
		dsp_screen ("Printing Discount By Customer / Category.",
			comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 4:
		dsp_screen ("Printing Discount By Customer-Type / Item.",
				comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 5:
		dsp_screen ("Printing Discount By Customer-Type / Selling Group.",
				comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 6:
		dsp_screen ("Printing Discount By Customer-Type / Category.",
				comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 7:
		dsp_screen ("Printing Discount By Price-Type / Item.",
				comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 8:
		dsp_screen ("Printing Discount By Price-Type / Selling Group.",
			comm_rec.tco_no,comm_rec.tco_name);
		break;

	case 9:
		dsp_screen ("Printing Discount By Price-Type / Category.",
			comm_rec.tco_no,comm_rec.tco_name);
		break;

	}

	head (lp_no);

	switch (rep_type)
	{
	case 1:
		procCustItem ();
		break;

	case 2:
		procCustSelgp ();
		break;

	case 3:
		procCustCat ();
		break;

	case 4:
		procCtypeItem ();
		break;

	case 5:
		procCtypeSelgp ();
		break;

	case 6:
		procCtypeCat ();
		break;

	case 7:
		procPtypeItem ();
		break;

	case 8:
		procPtypeSelgp ();
		break;

	case 9:
		procPtypeCat ();
		break;

	}

	fprintf (pp, ".EOF\n");
	pclose (pp);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (cumr, cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no"
							                              : "cumr_id_no3");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
	open_rec (excl, excl_list, excl_no_fields, "excl_id_no");
	open_rec (excf, excf_list, excf_no_fields, "excf_id_no");
	open_rec (inds, inds_list, inds_no_fields, "inds_id_no");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (excl);
	abc_fclose (excf);
	abc_fclose (cumr);
	abc_fclose (inds);
	abc_fclose (inmr);

	abc_dbclose (data);
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");

	strcpy (ccmr_rec.cm_co_no,  comm_rec.tco_no);
	strcpy (ccmr_rec.cm_est_no, comm_rec.test_no);
	strcpy (ccmr_rec.cm_cc_no,  comm_rec.tcc_no);
	if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

	abc_fclose (ccmr);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	char	valid_inp[2];
	int		found_yes = 0;
	int		i;

	if (LCHECK ("lpno"))
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp(local_rec.lpno))
		{
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		strcpy (local_rec.back_desc, (local_rec.back[0] == 'Y') ? "Yes" : "No ");
		display_field (field+1);
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight_desc, (local_rec.onight[0] == 'Y') ? "Yes" : "No ");
		display_field (field+1);
		return (EXIT_SUCCESS);
	}
	
	/*-----------------------------------
	| Validate Field Selection group 1. |
	-----------------------------------*/
	if (LCHECK ("dis"))
	{
		sprintf (valid_inp, "%1.1s", local_rec.rep_type[(field - 5)/2]);

		if (valid_inp[0] == 'Y')
		{
			strcpy (local_rec.rep_type[0], "N");
			strcpy (local_rec.rep_type[1], "N");
			strcpy (local_rec.rep_type[2], "N");
			strcpy (local_rec.rep_type[3], "N");
			strcpy (local_rec.rep_type[4], "N");
			strcpy (local_rec.rep_type[5], "N");
			strcpy (local_rec.rep_type[6], "N");
			strcpy (local_rec.rep_type[7], "N");
			strcpy (local_rec.rep_type[8], "N");

			strcpy (local_rec.rep_type_desc[0], "No ");
			strcpy (local_rec.rep_type_desc[1], "No ");
			strcpy (local_rec.rep_type_desc[2], "No ");
			strcpy (local_rec.rep_type_desc[3], "No ");
			strcpy (local_rec.rep_type_desc[4], "No ");
			strcpy (local_rec.rep_type_desc[5], "No ");
			strcpy (local_rec.rep_type_desc[6], "No ");
			strcpy (local_rec.rep_type_desc[7], "No ");
			strcpy (local_rec.rep_type_desc[8], "No ");
		}

		if (valid_inp[0] == 'Y')
		{
			strcpy (local_rec.rep_type[(field - 5)/2], "Y");
			strcpy (local_rec.rep_type_desc[(field - 5)/2], "Yes");
		}
		else 
		{
			strcpy (local_rec.rep_type[(field - 5)/2], "N");
			strcpy (local_rec.rep_type_desc[(field - 5)/2], "No ");
		}

		for (i = 5; i < 14; i++)
		{
			if (strcmp (local_rec.rep_type[i - 5], "Y") == 0)
			{
				rep_type = i - 4;
				found_yes++;
			}
		}

		if (found_yes == 0)
		{
			rep_type = 1;
			strcpy (local_rec.rep_type[0], "Y");
			strcpy (local_rec.rep_type_desc[0], "Yes");
		}

		for (i = 5; i < 22; i+=2)
		{
			display_field (i);
			display_field (i+1);
		}
				
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
head (
 int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("so_disprn.p")) == NULL)
	{
		sprintf (err_str, "Error in so_disprn.p During (FOPEN)");
		sys_err (err_str, errno, PNAME);
	}

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".LP%d\n", prnt_no);
	fprintf (pp, ".14\n");

	fprintf (pp, ".L130\n");
	fprintf (pp, ".E%s\n", rep_desc[rep_type - 1]);
	pr_format (fin, pp, "BLANK", 0, 0);
	fprintf (pp, ".ECOMPANY : %s %s\n", comm_rec.tco_no,
						clip (comm_rec.tco_name));
	
	fprintf (pp, ".EBRANCH : %s %s \n",
			comm_rec.test_no, clip (comm_rec.test_name)); 

	fprintf (pp, ".EWAREHOUSE %s %s \n",
			comm_rec.tcc_no, comm_rec.tcc_name);

	pr_format (fin, pp, "BLANK", 0, 0);
	fprintf (pp, ".EAS AT : %s\n", SystemTime ());
	pr_format (fin, pp, "BLANK", 0, 0);
	pr_format (fin, pp, "UNDERLINE", 0, 0);
	pr_format (fin, pp, "RULER", 0, 0);

	if (rep_type == 1 || rep_type == 4 || rep_type == 7)
	{
		pr_format (fin, pp, "HEAD1_ITM", 0, 0);
		pr_format (fin, pp, "HEAD2_ITM", 0, 0);
		pr_format (fin, pp, "SEPLN_ITM", 0, 0);
	}
	else
	if (rep_type == 2 || rep_type == 5 || rep_type == 8)
	{
		pr_format (fin, pp, "HEAD1_SGP", 0, 0);
		pr_format (fin, pp, "HEAD2_SGP", 0, 0);
		pr_format (fin, pp, "SEPLN_SGP", 0, 0);
	}
	else
	{
		pr_format (fin, pp, "HEAD1_CAT", 0, 0);
		pr_format (fin, pp, "HEAD2_CAT", 0, 0);
		pr_format (fin, pp, "SEPLN_CAT", 0, 0);
	}
	fflush (pp);
}

void
procCustItem (
 void)
{
	first_time = 1;
		
	abc_selfield (inds, "inds_hhcu_hash");

	strcpy  (cumr_rec.mr_co_no,  comm_rec.tco_no);
	strcpy  (cumr_rec.mr_est_no, branchNo);
	sprintf (cumr_rec.mr_dbt_no, "%-6.6s", " ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (cumr_rec.mr_co_no, comm_rec.tco_no))
	{
		if ((strcmp (cumr_rec.mr_est_no, branchNo) != 0) && !envDbFind)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		inds_rec.hhcu_hash = cumr_rec.mr_hhcu_hash;
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && inds_rec.hhcu_hash == cumr_rec.mr_hhcu_hash)
		{
			if (!find_inmr (inds_rec.hhbr_hash))
			{
				if (first_time)
				{
					prev_hash = inds_rec.hhcu_hash;
					strcpy (prev_item, inmr_rec.mr_item_no);
					printCustHdr ();
				}

				if (prev_hash != inds_rec.hhcu_hash)
				{
					prev_hash = inds_rec.hhcu_hash;
					strcpy (prev_item, inmr_rec.mr_item_no);
					printCustHdr ();
				}

				strcpy (br_no, inds_rec.br_no);

				if (inds_rec.hhcc_hash == 0L)
					strcpy (wh_no, "  ");
				else
				{
					ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
					cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
					if (cc)
						file_err (cc, ccmr, "DBFIND");
					strcpy (wh_no, ccmr2_rec.cm_cc_no);
				}

				prev_hash = inds_rec.hhcu_hash;
				strcpy (prev_item, inmr_rec.mr_item_no);
				printItemDetail ();
				first_time = 0;
				dsp_process (" Cust No : ", clip (cumr_rec.mr_dbt_no));
			}
			cc = find_rec (inds, &inds_rec, NEXT, "r");
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

void
procCustCat (
 void)
{
	first_time = 1;

	abc_selfield (inds, "inds_hhcu_hash");

	strcpy  (cumr_rec.mr_co_no, comm_rec.tco_no);
	strcpy  (cumr_rec.mr_est_no, branchNo);
	sprintf (cumr_rec.mr_dbt_no, "%-6.6s", " ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.mr_co_no, comm_rec.tco_no))
	{
		if ((strcmp (cumr_rec.mr_est_no, branchNo) != 0) && !envDbFind)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		inds_rec.hhcu_hash = cumr_rec.mr_hhcu_hash;
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && inds_rec.hhcu_hash == cumr_rec.mr_hhcu_hash)
		{
			if (inds_rec.hhbr_hash > 0L ||
			   (strcmp (inds_rec.sel_group, "      ") != 0))
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (first_time)
			{
				strcpy (prev_category, inds_rec.category);
				prev_hash = inds_rec.hhcu_hash;
				printCustHdr ();
			}

			if (prev_hash != inds_rec.hhcu_hash)
			{
				strcpy (prev_category, inds_rec.category);
				prev_hash = inds_rec.hhcu_hash;
				printCustHdr ();
			}

			strcpy (br_no, inds_rec.br_no);

			if (inds_rec.hhcc_hash == 0L)
				strcpy (wh_no, "  ");
			else
			{
				ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
				cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, ccmr, "DBFIND");
				strcpy (wh_no, ccmr2_rec.cm_cc_no);
			}
			prev_hash = inds_rec.hhcu_hash;
			strcpy (prev_category, inds_rec.category);
			printCatDetail ();
			first_time = 0;
			cc = find_rec (inds, &inds_rec, NEXT, "r");
			dsp_process (" Cust No : ", clip (cumr_rec.mr_dbt_no));
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

void
procCustSelgp (
 void)
{
	first_time = 1;

	abc_selfield (inds, "inds_hhcu_hash");

	strcpy (cumr_rec.mr_co_no, comm_rec.tco_no);
	strcpy (cumr_rec.mr_est_no, branchNo);
	sprintf (cumr_rec.mr_dbt_no, "%-6.6s", " ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.mr_co_no, comm_rec.tco_no))
	{
		if ((strcmp (cumr_rec.mr_est_no, branchNo) != 0) && !envDbFind)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		inds_rec.hhcu_hash = cumr_rec.mr_hhcu_hash;
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && inds_rec.hhcu_hash == cumr_rec.mr_hhcu_hash)
		{
			if (inds_rec.hhbr_hash > 0L ||
			   (strcmp (inds_rec.category, "           ") != 0))
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}
			if (first_time)
			{
				strcpy (prev_selgrp, inds_rec.sel_group);
				prev_hash = inds_rec.hhcu_hash;
				printCustHdr ();
			}

			if (prev_hash != inds_rec.hhcu_hash)
			{
				strcpy (prev_selgrp, inds_rec.sel_group);
				prev_hash = inds_rec.hhcu_hash;
				printCustHdr ();
			}

			strcpy (br_no, inds_rec.br_no);

			if (inds_rec.hhcc_hash == 0L)
				strcpy (wh_no, "  ");
			else
			{
				ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
				cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, ccmr, "DBFIND");
				strcpy (wh_no, ccmr2_rec.cm_cc_no);
			}
			prev_hash = inds_rec.hhcu_hash;
			strcpy (prev_selgrp, inds_rec.sel_group);
			printSelgrpDetail ();
			first_time = 0;
			cc = find_rec (inds, &inds_rec, NEXT, "r");
			dsp_process (" Cust No : ", clip (cumr_rec.mr_dbt_no));
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

void
procCtypeItem (
 void)
{
	first_time = 1;

	strcpy (excl_rec.cl_co_no,comm_rec.tco_no);
	strcpy (excl_rec.cl_cus_type,"   ");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strcmp (excl_rec.cl_co_no, comm_rec.tco_no))
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		strcpy (inds_rec.br_no, "  ");
		inds_rec.hhcu_hash = 0L;
		inds_rec.hhbr_hash = 0L;
		inds_rec.hhcc_hash = 0L;
		strcpy (inds_rec.cust_type,excl_rec.cl_cus_type);
		sprintf (inds_rec.category,"%-11.11s"," ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (!find_inmr (inds_rec.hhbr_hash) && 
				!strcmp (inds_rec.cust_type, excl_rec.cl_cus_type))
			{
				if (first_time)
				{
					strcpy (prev_item,  inmr_rec.mr_item_no);
					strcpy (prev_ctype, inds_rec.cust_type);
					printCtypeHdr ();
				}

				if (strcmp (prev_ctype, inds_rec.cust_type) != 0)
				{
					strcpy (prev_item,  inmr_rec.mr_item_no);
					strcpy (prev_ctype, inds_rec.cust_type);
					printCtypeHdr ();
				}

				strcpy (br_no, inds_rec.br_no);

				if (inds_rec.hhcc_hash == 0L)
					strcpy (wh_no, "  ");
				else
				{
					ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
					cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
					if (cc)
						file_err (cc, ccmr, "DBFIND");
					strcpy (wh_no, ccmr2_rec.cm_cc_no);
				}

				strcpy (prev_ctype, inds_rec.cust_type);
				strcpy (prev_item, inmr_rec.mr_item_no);
				printItemDetail ();
				first_time = 0;
				dsp_process(" Cust Type : ",excl_rec.cl_cus_type);	
			}
			cc = find_rec (inds, &inds_rec, NEXT, "r");
		}
		cc = find_rec("excl", &excl_rec, NEXT, "r");
	}
}

void
procCtypeCat (
 void)
{
	first_time = 1;

	strcpy (excl_rec.cl_co_no,comm_rec.tco_no);
	strcpy (excl_rec.cl_cus_type,"   ");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strcmp (excl_rec.cl_co_no, comm_rec.tco_no))
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		inds_rec.hhcu_hash = 0L;
		inds_rec.hhbr_hash = 0L;
		inds_rec.hhcc_hash = 0L;
		strcpy (inds_rec.cust_type,excl_rec.cl_cus_type);
		sprintf (inds_rec.category,"%-11.11s"," ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (strcmp(inds_rec.cust_type,excl_rec.cl_cus_type))
			{ 
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}
			if ( !strcmp(inds_rec.category, "           ") )
			{ 
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}
			if (first_time)
			{
				strcpy(prev_ctype,inds_rec.cust_type);
				strcpy(prev_category,inds_rec.category);
				printCtypeHdr();
			}

			if (strcmp(prev_ctype,inds_rec.cust_type) != 0)
			{
				strcpy(prev_ctype,inds_rec.cust_type);
				strcpy(prev_category,inds_rec.category);
				printCtypeHdr();
			}
			
			printCatDetail ();
			strcpy(prev_ctype,inds_rec.cust_type);
			strcpy(prev_category,inds_rec.category);
			first_time = 0;
			cc = find_rec(inds, &inds_rec, NEXT, "r");
		}
		dsp_process(" Cust Type : ",excl_rec.cl_cus_type);	
		cc = find_rec("excl", &excl_rec, NEXT, "r");
	}
}

void
procCtypeSelgp (
 void)
{
	first_time = 1;

	strcpy (excl_rec.cl_co_no, comm_rec.tco_no);
	strcpy (excl_rec.cl_cus_type, "   ");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strcmp (excl_rec.cl_co_no, comm_rec.tco_no))
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		inds_rec.hhcu_hash = 0L;
		inds_rec.hhbr_hash = 0L;
		inds_rec.hhcc_hash = 0L;
		strcpy (inds_rec.cust_type, excl_rec.cl_cus_type);
		sprintf (inds_rec.category, "%-11.11s", " ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (strcmp(inds_rec.cust_type, excl_rec.cl_cus_type))
			{ 
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}
			if (!strcmp(inds_rec.sel_group, "      ") )
			{ 
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (first_time)
			{
				strcpy (prev_ctype, inds_rec.cust_type);
				strcpy (prev_selgrp, inds_rec.sel_group);
				printCtypeHdr ();
			}

			if (strcmp (prev_ctype, inds_rec.cust_type) != 0)
			{
				strcpy (prev_ctype, inds_rec.cust_type);
				strcpy (prev_selgrp, inds_rec.sel_group);
				printCtypeHdr ();
			}

			strcpy (br_no, inds_rec.br_no);

			if (inds_rec.hhcc_hash == 0L)
				strcpy (wh_no, "  ");
			else
			{
				ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
				cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, ccmr, "DBFIND");
				strcpy (wh_no, ccmr2_rec.cm_cc_no);
			}
			strcpy (prev_ctype, inds_rec.cust_type);
			strcpy (prev_selgrp, inds_rec.sel_group);
			printSelgrpDetail ();
			first_time = 0;
			cc = find_rec (inds, &inds_rec, NEXT, "r");
		}
	
		dsp_process (" Cust Type : ",excl_rec.cl_cus_type);	
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
}

void
procPtypeItem (
 void)
{
	int		pType;

	first_time = 1;
		
	abc_selfield (inds, "inds_id_no2");

	for (pType = 1; pType <= sk_dbprinum; pType++)
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		strcpy (inds_rec.br_no, "  ");
		inds_rec.hhcc_hash = 0L;
		inds_rec.price_type = pType;
		inds_rec.hhbr_hash = 0L;
		strcpy (inds_rec.category, "           ");
		strcpy (inds_rec.sel_group, "      ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (inds_rec.price_type != pType)
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (!find_inmr (inds_rec.hhbr_hash))
			{
				if (first_time)
				{
					prev_ptype = inds_rec.price_type;
					strcpy (prev_item, inmr_rec.mr_item_no);
					printPtypeHdr ();
				}

				if (prev_ptype != inds_rec.price_type)
				{
					prev_ptype = inds_rec.price_type;
					strcpy (prev_item, inmr_rec.mr_item_no);
					printPtypeHdr();
				}

				strcpy (br_no, inds_rec.br_no);

				if (inds_rec.hhcc_hash == 0L)
					strcpy (wh_no, "  ");
				else
				{
					ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
					cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
					if (cc)
						file_err (cc, ccmr, "DBFIND");
					strcpy (wh_no, ccmr2_rec.cm_cc_no);
				}

				printItemDetail ();
				first_time = 0;
				dsp_process(" Price Type : ", comm_rec.tprice_desc[pType - 1]);	
			}
			cc = find_rec (inds, &inds_rec, NEXT, "r");
		}
	}
}

void
procPtypeCat (
 void)
{
	int		pType;

	first_time = 1;
		
	abc_selfield (inds, "inds_id_no2");

	for (pType = 1; pType <= sk_dbprinum; pType++)
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		strcpy (inds_rec.br_no, "  ");
		inds_rec.hhcc_hash = 0L;
		inds_rec.price_type = pType;
		inds_rec.hhbr_hash = 0L;
		strcpy (inds_rec.category, "           ");
		strcpy (inds_rec.sel_group, "      ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (inds_rec.price_type != pType)
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (!strcmp (inds_rec.category, "           "))
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (first_time)
			{
				prev_ptype = inds_rec.price_type;
				strcpy (prev_category, inds_rec.category);
				printPtypeHdr();
			}

			if (prev_ptype != inds_rec.price_type)
			{
				prev_ptype = inds_rec.price_type;
				strcpy (prev_category, inds_rec.category);
				printPtypeHdr();
			}

			strcpy (br_no, inds_rec.br_no);

			if (inds_rec.hhcc_hash == 0L)
				strcpy (wh_no, "  ");
			else
			{
				ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
				cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, ccmr, "DBFIND");
				strcpy (wh_no, ccmr2_rec.cm_cc_no);
			}

			printCatDetail ();
			first_time = 0;
			
			cc = find_rec (inds, &inds_rec, NEXT, "r");
			dsp_process(" Price Type : ", comm_rec.tprice_desc[pType - 1]);	
		}
	}
}

void
procPtypeSelgp (
 void)
{
	int		pType;

	first_time = 1;
		
	abc_selfield (inds, "inds_id_no2");

	for (pType = 1; pType <= sk_dbprinum; pType++)
	{
		strcpy (inds_rec.co_no, comm_rec.tco_no);
		strcpy (inds_rec.br_no, "  ");
		inds_rec.hhcc_hash = 0L;
		inds_rec.price_type = pType;
		inds_rec.hhbr_hash = 0L;
		strcpy (inds_rec.category, "           ");
		strcpy (inds_rec.sel_group, "      ");
		cc = find_rec (inds, &inds_rec, GTEQ, "r");
		while (!cc && !strcmp (inds_rec.co_no, comm_rec.tco_no))
		{
			if (inds_rec.price_type != pType)
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (!strcmp (inds_rec.sel_group, "      "))
			{
				cc = find_rec(inds, &inds_rec, NEXT, "r");
				continue;
			}

			if (first_time)
			{
				prev_ptype = inds_rec.price_type;
				strcpy (prev_selgrp, inds_rec.sel_group);
				printPtypeHdr();
			}

			if (prev_ptype != inds_rec.price_type)
			{
				prev_ptype = inds_rec.price_type;
				strcpy (prev_selgrp, inds_rec.sel_group);
				printPtypeHdr();
			}

			strcpy (br_no, inds_rec.br_no);

			if (inds_rec.hhcc_hash == 0L)
				strcpy (wh_no, "  ");
			else
			{
				ccmr2_rec.cm_hhcc_hash = inds_rec.hhcc_hash;
				cc = find_rec(ccmr, &ccmr2_rec, EQUAL, "r");
				if (cc)
					file_err (cc, ccmr, "DBFIND");
				strcpy (wh_no, ccmr2_rec.cm_cc_no);
			}

			printSelgrpDetail ();
			first_time = 0;
			
			cc = find_rec (inds, &inds_rec, NEXT, "r");
			dsp_process(" Price Type : ", comm_rec.tprice_desc[pType - 1]);	
		}
	}
}

void
printPtypeHdr (
 void)
{
	if (!first_time)
	{
		if (rep_type == 7)
			pr_format (fin, pp, "SEPLN_ITM", 0, 0);
		else
		if (rep_type == 8)
			pr_format (fin, pp, "SEPLN_CAT", 0, 0);
		else
			pr_format (fin, pp, "SEPLN_SGP", 0, 0);
	}

	if (rep_type == 7)
	{
		pr_format (fin, pp, "PTYPE_ITM", 1, prev_ptype);
		pr_format (fin, pp, "PTYPE_ITM", 2, comm_rec.tprice_desc[prev_ptype - 1]);
		pr_format (fin, pp, "SEPBL_ITM", 0, 0);
	}
	else
	if (rep_type == 8)
	{
		pr_format (fin, pp, "PTYPE_CAT", 1, prev_ptype);
		pr_format (fin, pp, "PTYPE_CAT", 2, comm_rec.tprice_desc[prev_ptype - 1]);
		pr_format (fin, pp, "SEPBL_CAT", 0, 0);
	}
	else
	{
		pr_format (fin, pp, "PTYPE_SGP", 1, prev_ptype);
		pr_format (fin, pp, "PTYPE_SGP", 2, comm_rec.tprice_desc[prev_ptype - 1]);
		pr_format (fin, pp, "SEPBL_SGP", 0, 0);
	}
}

void
printCtypeHdr (
 void)
{
	if (!first_time)
	{
		if (rep_type == 4)
			pr_format (fin, pp, "SEPLN_ITM", 0, 0);
		else
		if (rep_type == 5)
			pr_format (fin, pp, "SEPLN_CAT", 0, 0);
		else
			pr_format (fin, pp, "SEPLN_SGP", 0, 0);
	}

	if (rep_type == 4)
	{
		pr_format (fin, pp, "CTYPE_ITM", 1, excl_rec.cl_cus_type);
		pr_format (fin, pp, "CTYPE_ITM", 2, excl_rec.cl_class_desc);
		pr_format (fin, pp, "SEPBL_ITM", 0, 0);
	}
	else
	if (rep_type == 5)
	{
		pr_format (fin, pp, "CTYPE_CAT", 1, excl_rec.cl_cus_type);
		pr_format (fin, pp, "CTYPE_CAT", 2, excl_rec.cl_class_desc);
		pr_format (fin, pp, "SEPBL_CAT", 0, 0);
	}
	else
	{
		pr_format (fin, pp, "CTYPE_SGP", 1, excl_rec.cl_cus_type);
		pr_format (fin, pp, "CTYPE_SGP", 2, excl_rec.cl_class_desc);
		pr_format (fin, pp, "SEPBL_SGP", 0, 0);
	}
}

void
printItemDetail (
 void)
{
	char	break_by[9];

	if (inds_rec.disc_by[0] == 'V')
		strcpy (break_by, "Value   ");
	else
		strcpy (break_by, "Quantity");

	pr_format (fin, pp, "DHEAD_ITM",  1, prev_item);
	pr_format (fin, pp, "DHEAD_ITM",  2, br_no);
	pr_format (fin, pp, "DHEAD_ITM",  3, wh_no);
	pr_format (fin, pp, "DHEAD_ITM",  4, break_by);
	pr_format (fin, pp, "DHEAD_ITM",  5, inds_rec.qty_brk[0]);
	pr_format (fin, pp, "DHEAD_ITM",  6, inds_rec.qty_brk[1]);
	pr_format (fin, pp, "DHEAD_ITM",  7, inds_rec.qty_brk[2]);
	pr_format (fin, pp, "DHEAD_ITM",  8, inds_rec.qty_brk[3]);
	pr_format (fin, pp, "DHEAD_ITM",  9, inds_rec.qty_brk[4]);
	pr_format (fin, pp, "DHEAD_ITM", 10, inds_rec.qty_brk[5]);

	pr_format (fin, pp, "DETAIL_ITM",  1, "A");
	pr_format (fin, pp, "DETAIL_ITM",  2, inds_rec.disca_pc[0]);
	pr_format (fin, pp, "DETAIL_ITM",  3, inds_rec.disca_pc[1]);
	pr_format (fin, pp, "DETAIL_ITM",  4, inds_rec.disca_pc[2]);
	pr_format (fin, pp, "DETAIL_ITM",  5, inds_rec.disca_pc[3]);
	pr_format (fin, pp, "DETAIL_ITM",  6, inds_rec.disca_pc[4]);
	pr_format (fin, pp, "DETAIL_ITM",  7, inds_rec.disca_pc[5]);

	pr_format (fin, pp, "DETAIL_ITM",  1, "B");
	pr_format (fin, pp, "DETAIL_ITM",  2, inds_rec.discb_pc[0]);
	pr_format (fin, pp, "DETAIL_ITM",  3, inds_rec.discb_pc[1]);
	pr_format (fin, pp, "DETAIL_ITM",  4, inds_rec.discb_pc[2]);
	pr_format (fin, pp, "DETAIL_ITM",  5, inds_rec.discb_pc[3]);
	pr_format (fin, pp, "DETAIL_ITM",  6, inds_rec.discb_pc[4]);
	pr_format (fin, pp, "DETAIL_ITM",  7, inds_rec.discb_pc[5]);

	pr_format (fin, pp, "DETAIL_ITM",  1, "C");
	pr_format (fin, pp, "DETAIL_ITM",  2, inds_rec.discc_pc[0]);
	pr_format (fin, pp, "DETAIL_ITM",  3, inds_rec.discc_pc[1]);
	pr_format (fin, pp, "DETAIL_ITM",  4, inds_rec.discc_pc[2]);
	pr_format (fin, pp, "DETAIL_ITM",  5, inds_rec.discc_pc[3]);
	pr_format (fin, pp, "DETAIL_ITM",  6, inds_rec.discc_pc[4]);
	pr_format (fin, pp, "DETAIL_ITM",  7, inds_rec.discc_pc[5]);
}

void
printSelgrpDetail (
 void)
{
	char	break_by[9];

	if (inds_rec.disc_by[0] == 'V')
		strcpy (break_by, "Value   ");
	else
		strcpy (break_by, "Quantity");

	pr_format (fin, pp, "DHEAD_SGP",  1, prev_selgrp);
	pr_format (fin, pp, "DHEAD_SGP",  2, br_no);
	pr_format (fin, pp, "DHEAD_SGP",  3, wh_no);
	pr_format (fin, pp, "DHEAD_SGP",  4, break_by);
	pr_format (fin, pp, "DHEAD_SGP",  5, inds_rec.qty_brk[0]);
	pr_format (fin, pp, "DHEAD_SGP",  6, inds_rec.qty_brk[1]);
	pr_format (fin, pp, "DHEAD_SGP",  7, inds_rec.qty_brk[2]);
	pr_format (fin, pp, "DHEAD_SGP",  8, inds_rec.qty_brk[3]);
	pr_format (fin, pp, "DHEAD_SGP",  9, inds_rec.qty_brk[4]);
	pr_format (fin, pp, "DHEAD_SGP", 10, inds_rec.qty_brk[5]);

	pr_format (fin, pp, "DETAIL_SGP",  1, "A");
	pr_format (fin, pp, "DETAIL_SGP",  2, inds_rec.disca_pc[0]);
	pr_format (fin, pp, "DETAIL_SGP",  3, inds_rec.disca_pc[1]);
	pr_format (fin, pp, "DETAIL_SGP",  4, inds_rec.disca_pc[2]);
	pr_format (fin, pp, "DETAIL_SGP",  5, inds_rec.disca_pc[3]);
	pr_format (fin, pp, "DETAIL_SGP",  6, inds_rec.disca_pc[4]);
	pr_format (fin, pp, "DETAIL_SGP",  7, inds_rec.disca_pc[5]);

	pr_format (fin, pp, "DETAIL_SGP",  1, "B");
	pr_format (fin, pp, "DETAIL_SGP",  2, inds_rec.discb_pc[0]);
	pr_format (fin, pp, "DETAIL_SGP",  3, inds_rec.discb_pc[1]);
	pr_format (fin, pp, "DETAIL_SGP",  4, inds_rec.discb_pc[2]);
	pr_format (fin, pp, "DETAIL_SGP",  5, inds_rec.discb_pc[3]);
	pr_format (fin, pp, "DETAIL_SGP",  6, inds_rec.discb_pc[4]);
	pr_format (fin, pp, "DETAIL_SGP",  7, inds_rec.discb_pc[5]);

	pr_format (fin, pp, "DETAIL_SGP",  1, "C");
	pr_format (fin, pp, "DETAIL_SGP",  2, inds_rec.discc_pc[0]);
	pr_format (fin, pp, "DETAIL_SGP",  3, inds_rec.discc_pc[1]);
	pr_format (fin, pp, "DETAIL_SGP",  4, inds_rec.discc_pc[2]);
	pr_format (fin, pp, "DETAIL_SGP",  5, inds_rec.discc_pc[3]);
	pr_format (fin, pp, "DETAIL_SGP",  6, inds_rec.discc_pc[4]);
	pr_format (fin, pp, "DETAIL_SGP",  7, inds_rec.discc_pc[5]);
}

void
printCatDetail (
 void)
{
	char	break_by[9];

	if (inds_rec.disc_by[0] == 'V')
		strcpy (break_by, "Value   ");
	else
		strcpy (break_by, "Quantity");

	pr_format (fin, pp, "DHEAD_CAT",  1, prev_category);
	pr_format (fin, pp, "DHEAD_CAT",  2, br_no);
	pr_format (fin, pp, "DHEAD_CAT",  3, wh_no);
	pr_format (fin, pp, "DHEAD_CAT",  4, break_by);
	pr_format (fin, pp, "DHEAD_CAT",  5, inds_rec.qty_brk[0]);
	pr_format (fin, pp, "DHEAD_CAT",  6, inds_rec.qty_brk[1]);
	pr_format (fin, pp, "DHEAD_CAT",  7, inds_rec.qty_brk[2]);
	pr_format (fin, pp, "DHEAD_CAT",  8, inds_rec.qty_brk[3]);
	pr_format (fin, pp, "DHEAD_CAT",  9, inds_rec.qty_brk[4]);
	pr_format (fin, pp, "DHEAD_CAT", 10, inds_rec.qty_brk[5]);

	pr_format (fin, pp, "DETAIL_CAT",  1, "A");
	pr_format (fin, pp, "DETAIL_CAT",  2, inds_rec.disca_pc[0]);
	pr_format (fin, pp, "DETAIL_CAT",  3, inds_rec.disca_pc[1]);
	pr_format (fin, pp, "DETAIL_CAT",  4, inds_rec.disca_pc[2]);
	pr_format (fin, pp, "DETAIL_CAT",  5, inds_rec.disca_pc[3]);
	pr_format (fin, pp, "DETAIL_CAT",  6, inds_rec.disca_pc[4]);
	pr_format (fin, pp, "DETAIL_CAT",  7, inds_rec.disca_pc[5]);

	pr_format (fin, pp, "DETAIL_CAT",  1, "B");
	pr_format (fin, pp, "DETAIL_CAT",  2, inds_rec.discb_pc[0]);
	pr_format (fin, pp, "DETAIL_CAT",  3, inds_rec.discb_pc[1]);
	pr_format (fin, pp, "DETAIL_CAT",  4, inds_rec.discb_pc[2]);
	pr_format (fin, pp, "DETAIL_CAT",  5, inds_rec.discb_pc[3]);
	pr_format (fin, pp, "DETAIL_CAT",  6, inds_rec.discb_pc[4]);
	pr_format (fin, pp, "DETAIL_CAT",  7, inds_rec.discb_pc[5]);

	pr_format (fin, pp, "DETAIL_CAT",  1, "C");
	pr_format (fin, pp, "DETAIL_CAT",  2, inds_rec.discc_pc[0]);
	pr_format (fin, pp, "DETAIL_CAT",  3, inds_rec.discc_pc[1]);
	pr_format (fin, pp, "DETAIL_CAT",  4, inds_rec.discc_pc[2]);
	pr_format (fin, pp, "DETAIL_CAT",  5, inds_rec.discc_pc[3]);
	pr_format (fin, pp, "DETAIL_CAT",  6, inds_rec.discc_pc[4]);
	pr_format (fin, pp, "DETAIL_CAT",  7, inds_rec.discc_pc[5]);
}

void
printCustHdr (
 void)
{
	if (!first_time)
	{
		if (rep_type == 1)
			pr_format (fin, pp, "SEPLN_ITM", 0, 0);
		else
		if (rep_type == 2)
			pr_format (fin, pp, "SEPLN_CAT", 0, 0);
		else
			pr_format (fin, pp, "SEPLN_SGP", 0, 0);
	}
	if (rep_type == 1)
	{
		pr_format (fin, pp, "CUST_ITM",  1, cumr_rec.mr_dbt_no);
		pr_format (fin, pp, "CUST_ITM",  2, cumr_rec.mr_dbt_name);
		pr_format (fin, pp, "SEPBL_ITM", 0, 0);
	}
	else
	if (rep_type == 2)
	{
		pr_format (fin, pp, "CUST_CAT",  1, cumr_rec.mr_dbt_no);
		pr_format (fin, pp, "CUST_CAT",  2, cumr_rec.mr_dbt_name);
		pr_format (fin, pp, "SEPBL_CAT", 0, 0);
	}
	else
	{
		pr_format (fin, pp, "CUST_SGP",  1, cumr_rec.mr_dbt_no);
		pr_format (fin, pp, "CUST_SGP",  2, cumr_rec.mr_dbt_name);
		pr_format (fin, pp, "SEPBL_SGP", 0, 0);
	}
}

int
find_inmr (
 long hhbr_hash)
{
	return (find_hash (inmr, &inmr_rec, EQUAL, "r", hhbr_hash));
}

void
setup_dlt (
 void)
{
	local_rec.lpno = 1;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.back_desc, "No ");
	strcpy (local_rec.onight, "N");
	strcpy (local_rec.onight_desc, "No ");
	strcpy (local_rec.rep_type[0], "Y");
	strcpy (local_rec.rep_type[1], "N");
	strcpy (local_rec.rep_type[2], "N");
	strcpy (local_rec.rep_type[3], "N");
	strcpy (local_rec.rep_type[4], "N");
	strcpy (local_rec.rep_type[5], "N");
	strcpy (local_rec.rep_type[6], "N");
	strcpy (local_rec.rep_type[7], "N");
	strcpy (local_rec.rep_type[8], "N");

	strcpy (local_rec.rep_type_desc[0], "Yes");
	strcpy (local_rec.rep_type_desc[1], "No ");
	strcpy (local_rec.rep_type_desc[2], "No ");
	strcpy (local_rec.rep_type_desc[3], "No ");
	strcpy (local_rec.rep_type_desc[4], "No ");
	strcpy (local_rec.rep_type_desc[5], "No ");
	strcpy (local_rec.rep_type_desc[6], "No ");
	strcpy (local_rec.rep_type_desc[7], "No ");
	strcpy (local_rec.rep_type_desc[8], "No ");
	rep_type = 1;
}

int
check_page (
 void)
{
	return(0);
}

void
run_prog (
 void)
{
	char	report_type[2];

	sprintf(report_type,"%1d",rep_type);

	sprintf(local_rec.lp_str,"%2d",local_rec.lpno);

	shutdown_prog ();
	
	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				"so_disprn",
				local_rec.lp_str,
				report_type,
				err_str,(char *)0);
				/*"Print Discount Structure",(char *)0);*/
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp( "so_disprn",
				"so_disprn",
				local_rec.lp_str,
				report_type,(char *)0);
	}
	else 
	{
		execlp( "so_disprn",
		        "so_disprn",
			local_rec.lp_str,
			report_type,(char *)0);
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML(mlSoMess012),20,0,1);

		move (0, 1);
		line (79);

		move (1, 7);
		line (79);

		box (0, 3, 80, 13);

		move (0, 20);
		line (79);
		print_at(21,0,ML(mlStdMess038), comm_rec.tco_no, 
					clip (comm_rec.tco_name));
		print_at(21,45,ML(mlStdMess039),  comm_rec.test_no, 
				clip (comm_rec.test_name));
		move (0, 22);
		line (79);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
