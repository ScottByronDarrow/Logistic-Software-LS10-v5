/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_mf_aud.c   )                                 |
|  Program Desc  : ( Displays/ Prints master file Audits.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  xxxx.audit                                        |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/05/91         |
|---------------------------------------------------------------------|
|  Date Modified : (06/12/91)      | Modified  by  : Campbell Mander  |
|  Date Modified : (15/06/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (06/09/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (25/03/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (03/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (06/12/91) - Update for cumr phone_no length       |
|                : SC 6297 PSL.                                       |
|                :                                                    |
|  (15/06/92)    : Added cus_gl_type. SC 7241 KIL.                    |
|                : Also fixed for bugs introduced through db_mr_inpt. |
|                :                                                    |
|  (06/09/93)    : HGP  9745 - Updated for post_code.                 |
|                :                                                    |
|  (25/03/94)    : HGP 10457 - Remove hard coded GST.                 |
|  (03/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
| $Log: psl_mf_aud.c,v $
| Revision 5.2  2001/08/09 05:13:46  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:37  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:52  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:29  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:47:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/09/17 07:27:11  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/06/15 02:36:55  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_mf_aud.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_mf_aud/psl_mf_aud.c,v 5.2 2001/08/09 05:13:46 scott Exp $";

#define		NO_SCRGEN
#define		PAGE_LEN	17
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <get_lpno.h>

	struct {
		char 	wk_co_no[3];
		char 	wk_est_no[3];
		char 	wk_department[3];
		char 	wk_dbt_no[7];
		long 	wk_hhcu_hash;
		char 	wk_name[41];
		char 	wk_acronym[10];
		char 	wk_acc_type[2];
		char 	wk_stmt_type[2];
		char 	wk_class_type[4];
		char 	wk_price_type[2];
		char 	wk_cont_type[4];
		int 	wk_payment_flag;
		char 	wk_bo_flag[2];
		char	wk_bo_cons[2];
		int		wk_bo_days;
		char	wk_po_flag[2];
		char	wk_sur_flag[2];
		char 	wk_ch_adr[3][41];
		char 	wk_dl_adr[3][41];
		char 	wk_contact_name[21];
		char 	wk_phone_no[16];
		char	wk_fax_no[16];
		char	wk_telex[11];
		char	wk_post_code[11];
		char 	wk_stop_credit[2];
		long 	wk_date_stop;
		int 	wk_total_days_sc;
		double 	wk_credit_limit;		/*  Money field  */
		char	wk_crd_prd[4];
		char	wk_crd_flag[2];
		char 	wk_credit_ref[21];
		char 	wk_bank_code[4];
		char 	wk_branch_code[21];
		char 	wk_area_code[3];
		char 	wk_sman_code[3];
		char 	wk_roy_type[4];
		char 	wk_disc_code[2];
		char 	wk_tax_code[2];
		char	wk_tax_no[16];
		char 	wk_stmnt_flg[2];
		char	wk_freight_chg[2];
		char	wk_restock_fee[2];
		char	wk_nett_pri_prt[2];
		char	wk_reprint_inv[2];
		int		wk_cus_gl_type;
		int  	wk_inst_fg[3];
		char 	wk_stat_flag[2];
		char 	wk_user_name[15];
		long 	wk_log_date;
		char 	wk_log_time[6];
	} cumr1_rec, cumr2_rec;

	struct {
		char	wk_co_no[3];
		char	wk_cat_no[12];
		long	wk_hhcf_hash;
		float	wk_ex_rate;
		char	wk_cat_desc[41];
		float	wk_max_disc;
		float	wk_min_marg;
		float	wk_gp_mkup;
		char	wk_item_alloc[2];
		int		wk_no_trans;
		int		wk_no_days;
		char	wk_stat_flag[2];
		char	wk_user_name[15];
		long	wk_log_date;
		char	wk_log_time[6];
	} excf1_rec, excf2_rec;

	struct {
		char	wk_co_no[3];
		char	wk_est_no[3];
		char	wk_crd_no[7];
		long	wk_hhsu_hash;
		char	wk_name[41];
		char	wk_acronym[10];
		char	wk_acc_type[2];
		char	wk_debtor_no[13];
		char	wk_adr[4][41];
		char	wk_cont_name[21];
		char	wk_cont_no[10];
		char	wk_curr_code[4];
		char	wk_ctry_code[4];
		char	wk_pay_terms[4];
		float	wk_disc;
		int		wk_sic[3];
		char	wk_gl_ctrl_acct[10];
		char	wk_hold_payment[2];
		char	wk_fax_no[15];
		char	wk_pay_method[2];
		char	wk_bank[21];
		char	wk_bank_branch[21];
		char	wk_bank_code[16];
		char	wk_bank_acct_no[16];
		long	wk_date_opened;
		char	wk_stat_flag[2];
		char	wk_user_name[15];
		long	wk_log_date;
		char	wk_log_time[6];
	} sumr1_rec, sumr2_rec;

	struct {
		char	wk_co_no[3];
		char	wk_co_name[41];
		char	wk_co_sname[16];
		char	wk_co_adr[3][41];
		char	wk_mod_imp[21];
		int		wk_fiscal;
		long	wk_stmt_date;
		long	wk_yend_date;
		int		wk_closed_period;
		int		wk_pay_bover;
		int		wk_pay_freq;
		float	wk_gst_rate;
		char	wk_gst_ird_no[11];
		float	wk_int_rate;
		float	wk_restock_pc;
		char	wk_pdesc[5][16];
		double	wk_contingency;
		double	wk_sur_amt;		/*  Money field  */
		double	wk_sur_cof;		/*  Money field  */
		double	wk_frt_min_amt;		/*  Money field  */
		float	wk_frt_mweight;
		int		wk_pay_terms;
		char	wk_stat_flag[2];
		char	wk_user_name[15];
		long	wk_log_date;
		char	wk_log_time[6];
	} comr1_rec, comr2_rec;

	struct {
		char	wk_co_no[3];
		char	wk_no[3];
		char	wk_name[41];
		char	wk_short[16];
		char	wk_adr[3][41];
		char	wk_area_code[3];
		char	wk_stat_flag[2];
		char	wk_user_name[15];
		long	wk_log_date;
		char	wk_log_time[6];
	} esmr1_rec, esmr2_rec;

	struct {
		char	wk_co_no[3];
		char	wk_est_no[3];
		char	wk_cc_no[3];
		long	wk_hhcc_hash;
		char	wk_mst_wh[2];
		char	wk_sman_no[3];
		char	wk_name[41];
		char	wk_acronym[10];
		char	wk_type[3];
		char	wk_sal_ok[2];
		char	wk_pur_ok[2];
		char	wk_iss_ok[2];
		char	wk_rec_ok[2];
		char	wk_rep_ok[2];
		char	wk_stat_flag[2];
		char	wk_user_name[15];
		long	wk_log_date;
		char	wk_log_time[6];
	} ccmr1_rec, ccmr2_rec;
	
	char	disp_str[300];
	char	wk1_str[43];
	char	wk2_str[43];
	char	gst_code[4];

	int		wk_no;
	int		l_count;
	int		p_count;

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
void	proc_cumr		(void);
void	proc_excf		(void);
void	proc_glmr		(void);
void	header			(void);
void	prnt_line		(char *);
void	finish_page		(void);
void	proc_sumr		(void);
void	proc_comr		(void);
void	proc_esmr		(void);
void	proc_ccmr		(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	filename [150];
	char *	sptr = getenv ("PROG_PATH");

	sprintf (gst_code, "%-3.3s", getenv ("GST_TAX_NAME"));

	if (argc < 2)
	{
		print_at(0,0,mlMenuMess703);
		return (EXIT_FAILURE);
	}

	sprintf(filename,"%s/BIN/AUDIT/%s.audit", 
				( sptr == ( char *) 0) ? "/usr/LS10.5" : sptr,
				argv[ 1 ] );

	/*----------------------------------
	| Process cost centre master file. |
	----------------------------------*/
	if ( !strcmp( argv [ 1 ], "ccmr" ) )
	{
		if ( RF_OPEN(filename,sizeof(ccmr1_rec),"r",&wk_no) )
		{
			print_at(0,0,"%s\n\r",ML(mlStdMess193), argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_ccmr ();
	}

	/*-------------------------------
	| Process customer master file. |
	-------------------------------*/
	if ( !strcmp( argv[ 1 ], "cumr" ) )
	{
		if ( RF_OPEN(filename,sizeof(cumr1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193),argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_cumr ();
	}

	/*-------------------------------
	| Process category master file. |
	-------------------------------*/
	if ( !strcmp( argv[ 1 ], "excf" ) )
	{
		if ( RF_OPEN(filename,sizeof(excf1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193), argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_excf ();
	}

	/*-------------------------------------
	| Process general ledger master file. |
	-------------------------------------*/
	if ( !strcmp( argv[ 1 ], "glmr" ) )
	{
		if ( RF_OPEN(filename,sizeof(cumr1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193), argv[1]);
			return (EXIT_FAILURE);
		}
		header();
		proc_glmr();
	}

	/*-------------------------------
	| Process Supplier master file. |
	-------------------------------*/
	if ( !strcmp( argv[ 1 ], "sumr" ) )
	{
		if ( RF_OPEN(filename,sizeof(sumr1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193),argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_sumr ();
	}

	/*-----------------------------
	| Process branch master file. |
	-----------------------------*/
	if ( !strcmp( argv[ 1 ], "esmr" ) )
	{
		if ( RF_OPEN(filename,sizeof(esmr1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193),argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_esmr ();
	}

	/*------------------------------
	| Process company master file. |
	------------------------------*/
	if ( !strcmp( argv[ 1 ], "comr" ) )
	{
		if ( RF_OPEN(filename,sizeof(comr1_rec),"r",&wk_no) )
		{
			print_at(0,0,ML(mlStdMess193),argv[1]);
			return (EXIT_FAILURE);
		}
		header ();
		proc_comr ();
	}
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
	clear();
	print_at(0,0,ML(mlStdMess035));
	fflush(stdout);
	snorm();
	rset_tty();
	RF_CLOSE( wk_no );
}

void
proc_cumr (
 void)
{
	int		i;

	p_count = 0;
	strcpy( err_str, "                                      CUSTOMER MASTER FILE AUDIT OF CHANGES MADE.                                        ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[PRINT] [NEXT] [PREV] [EDIT/END]" );

	cc = RF_READ( wk_no, (char *) &cumr1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "Customer : %s ( %s ) User : %s / Date : %s / Time : %s",
				cumr1_rec.wk_dbt_no,
				cumr1_rec.wk_name,
				cumr1_rec.wk_user_name,
				DateToString( cumr1_rec.wk_log_date ),
				cumr1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &cumr2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if (strcmp( cumr1_rec.wk_department, cumr2_rec.wk_department ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Department No",
				cumr1_rec.wk_department,
				cumr2_rec.wk_department);
			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_acronym, cumr2_rec.wk_acronym ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Acronym",
				cumr1_rec.wk_acronym,
				cumr2_rec.wk_acronym);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 3; i++)
		{
			if ( strcmp(cumr1_rec.wk_ch_adr[i],cumr2_rec.wk_ch_adr[i]))
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Charge Address",
					cumr1_rec.wk_ch_adr[i],
					cumr2_rec.wk_ch_adr[i]);

				prnt_line( disp_str );
			}
		}
		for ( i = 0; i < 3; i++)
		{
			if ( strcmp(cumr1_rec.wk_dl_adr[i],cumr2_rec.wk_dl_adr[i]) )
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Delivery Address",
					cumr1_rec.wk_dl_adr[i],
					cumr2_rec.wk_dl_adr[i]);

				prnt_line( disp_str );
			}
		}
		if ( strcmp( cumr1_rec.wk_acc_type, cumr2_rec.wk_acc_type ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"account type",
				( cumr1_rec.wk_acc_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F",
				( cumr2_rec.wk_acc_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F" );

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_stmt_type, cumr2_rec.wk_stmt_type ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Statement type",
				( cumr1_rec.wk_stmt_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F",
				( cumr2_rec.wk_stmt_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F" );

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_class_type, cumr2_rec.wk_class_type ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Customer type",
				cumr1_rec.wk_class_type, cumr2_rec.wk_class_type);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_cont_type, cumr2_rec.wk_cont_type ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Contract type",
				cumr1_rec.wk_cont_type, cumr2_rec.wk_cont_type);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_price_type, cumr2_rec.wk_price_type ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Price type",
				cumr1_rec.wk_price_type, cumr2_rec.wk_price_type);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_bo_flag, cumr2_rec.wk_bo_flag ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Back Orders.",
				( cumr1_rec.wk_bo_flag[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_bo_flag[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_bo_cons, cumr2_rec.wk_bo_cons ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Consolidate B/orders.",
				( cumr1_rec.wk_bo_cons[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_bo_cons[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( cumr1_rec.wk_bo_days != cumr2_rec.wk_bo_days )
		{
			sprintf(wk1_str, "%03d", cumr1_rec.wk_bo_days );
			sprintf(wk2_str, "%03d", cumr2_rec.wk_bo_days );

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Max Days on B/O.",
				wk1_str, wk2_str);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_po_flag, cumr2_rec.wk_po_flag ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"P/order required.",
				( cumr1_rec.wk_po_flag[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_po_flag[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_sur_flag, cumr2_rec.wk_sur_flag ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Small order Surcharge.",
				( cumr1_rec.wk_sur_flag[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_sur_flag[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_stmnt_flg, cumr2_rec.wk_stmnt_flg ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Statement required.",
				( cumr1_rec.wk_stmnt_flg[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_stmnt_flg[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp(cumr1_rec.wk_freight_chg, cumr2_rec.wk_freight_chg))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Freight Charges .",
				( cumr1_rec.wk_freight_chg[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_freight_chg[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp(cumr1_rec.wk_restock_fee, cumr2_rec.wk_restock_fee))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Re-Stocking fee .",
				( cumr1_rec.wk_restock_fee[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_restock_fee[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_nett_pri_prt, cumr2_rec.wk_nett_pri_prt ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Nett pricing used.",
				( cumr1_rec.wk_nett_pri_prt[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_nett_pri_prt[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if (strcmp(cumr1_rec.wk_reprint_inv, cumr2_rec.wk_reprint_inv))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Reprint Invoice.",
				( cumr1_rec.wk_reprint_inv[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( cumr2_rec.wk_reprint_inv[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if (cumr1_rec.wk_cus_gl_type != cumr2_rec.wk_cus_gl_type)
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %1d%-41.41s ^E %1d%-41.41s ^E",
			 	"Interface Code.",
				cumr1_rec.wk_cus_gl_type, " ",
				cumr2_rec.wk_cus_gl_type, " ");

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_tax_code, cumr2_rec.wk_tax_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Tax Code.",
				cumr1_rec.wk_tax_code, 
				cumr2_rec.wk_tax_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_tax_no, cumr2_rec.wk_tax_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Tax Number.",
				cumr1_rec.wk_tax_no, 
				cumr2_rec.wk_tax_no);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_contact_name, cumr2_rec.wk_contact_name ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Contact Name.",
				cumr1_rec.wk_contact_name, 
				cumr2_rec.wk_contact_name);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_phone_no, cumr2_rec.wk_phone_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Phone No.",
				cumr1_rec.wk_phone_no, 
				cumr2_rec.wk_phone_no);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_fax_no, cumr2_rec.wk_fax_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Fax No.",
				cumr1_rec.wk_fax_no, 
				cumr2_rec.wk_fax_no);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_telex, cumr2_rec.wk_telex ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Telex No.",
				cumr1_rec.wk_telex, 
				cumr2_rec.wk_telex);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_post_code, cumr2_rec.wk_post_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Post Code.",
				cumr1_rec.wk_post_code, 
				cumr2_rec.wk_post_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_stop_credit, cumr2_rec.wk_stop_credit ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Stock Credit.",
				cumr1_rec.wk_stop_credit, 
				cumr2_rec.wk_stop_credit);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_bank_code, cumr2_rec.wk_bank_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Bank Code.",
				cumr1_rec.wk_bank_code, 
				cumr2_rec.wk_bank_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_branch_code, cumr2_rec.wk_branch_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Branch Code.",
				cumr1_rec.wk_branch_code, 
				cumr2_rec.wk_branch_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_crd_prd, cumr2_rec.wk_crd_prd ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Credit Terms.",
				cumr1_rec.wk_crd_prd, 
				cumr2_rec.wk_crd_prd);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_crd_flag, cumr2_rec.wk_crd_flag ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Credit Flag.",
				cumr1_rec.wk_crd_flag, 
				cumr2_rec.wk_crd_flag);

			prnt_line( disp_str );
		}
		if ( cumr1_rec.wk_credit_limit != cumr2_rec.wk_credit_limit )
		{
			sprintf(wk1_str, "%.2f",
					DOLLARS(cumr1_rec.wk_credit_limit ));
			sprintf(wk2_str, "%.2f",
					DOLLARS(cumr2_rec.wk_credit_limit) );

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Credit Limit.",
				wk1_str,
				wk2_str);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_credit_ref, cumr2_rec.wk_credit_ref ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Credit Ref.",
				cumr1_rec.wk_credit_ref, 
				cumr2_rec.wk_credit_ref);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_area_code, cumr2_rec.wk_area_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Area Code.",
				cumr1_rec.wk_area_code, 
				cumr2_rec.wk_area_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_sman_code, cumr2_rec.wk_sman_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Salesman Code.",
				cumr1_rec.wk_sman_code, 
				cumr2_rec.wk_sman_code);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_roy_type, cumr2_rec.wk_roy_type ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Royalty Type.",
				cumr1_rec.wk_roy_type, 
				cumr2_rec.wk_roy_type);

			prnt_line( disp_str );
		}
		if ( strcmp( cumr1_rec.wk_disc_code, cumr2_rec.wk_disc_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Discount Code.",
				cumr1_rec.wk_disc_code, 
				cumr2_rec.wk_disc_code);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 3; i++ )
		{
			if ( cumr1_rec.wk_inst_fg[i] != cumr2_rec.wk_inst_fg[i] )
			{
				sprintf(wk1_str, "%03d", cumr1_rec.wk_inst_fg[i]);
				sprintf(wk2_str, "%03d", cumr2_rec.wk_inst_fg[i]);

				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Special Inst.", wk1_str, wk2_str);
	
				prnt_line( disp_str );
			}
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");

		finish_page();
		cc = RF_READ( wk_no, (char *) &cumr1_rec );
	}
	Dsp_srch();
	Dsp_close();
}

void
proc_excf (
 void)
{
	p_count = 0;

	strcpy( err_str, "                                      CATEGORY MASTER FILE AUDIT OF CHANGES MADE.                                        ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[FN5-Print]    [FN14-Next]    [FN15-Prev]   [FN16-Input/End]" );

	cc = RF_READ( wk_no, (char *) &excf1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "Cat : %s ( %s ) User : %s / Date : %s / Time : %s",
				excf1_rec.wk_cat_no,
				excf1_rec.wk_cat_desc,
				excf1_rec.wk_user_name,
				DateToString( excf1_rec.wk_log_date ),
				excf1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &excf2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if ( strcmp( excf1_rec.wk_cat_desc, excf2_rec.wk_cat_desc ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Category Desc.",
				excf1_rec.wk_cat_desc, 
				excf2_rec.wk_cat_desc);

			prnt_line( disp_str );
		}
		if ( strcmp( excf1_rec.wk_item_alloc, excf2_rec.wk_item_alloc ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Items All to Cat",
				(excf1_rec.wk_item_alloc[0] == 'Y')
						? "Y(es)" : "N(o)",
				(excf2_rec.wk_item_alloc[0] == 'Y')
						? "Y(es)" : "N(o)");

			prnt_line( disp_str );
		}
		if ( excf1_rec.wk_max_disc != excf2_rec.wk_max_disc )
		{
			sprintf(wk1_str, "%.2f", excf1_rec.wk_max_disc);
			sprintf(wk2_str, "%.2f", excf2_rec.wk_max_disc);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Max Discount.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( excf1_rec.wk_min_marg != excf2_rec.wk_min_marg )
		{
			sprintf(wk1_str, "%.2f", excf1_rec.wk_min_marg);
			sprintf(wk2_str, "%.2f", excf2_rec.wk_min_marg);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Min Margin %.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( excf1_rec.wk_gp_mkup != excf2_rec.wk_gp_mkup )
		{
			sprintf(wk1_str, "%.2f", excf1_rec.wk_gp_mkup);
			sprintf(wk2_str, "%.2f", excf2_rec.wk_gp_mkup);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"GP Markup %.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( excf1_rec.wk_no_trans != excf2_rec.wk_no_trans )
		{
			sprintf(wk1_str, "%d", excf1_rec.wk_no_trans);
			sprintf(wk2_str, "%d", excf2_rec.wk_no_trans);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Min no of tran held.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( excf1_rec.wk_no_days != excf2_rec.wk_no_days )
		{
			sprintf(wk1_str, "%d", excf1_rec.wk_no_days);
			sprintf(wk2_str, "%d", excf2_rec.wk_no_days);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Min no of tran held.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");
		finish_page();
		cc = RF_READ( wk_no, (char *) &excf1_rec );
	}
	Dsp_srch();
	Dsp_close();
}

void
proc_glmr (
 void)
{
	clear();
/*
	print_at(0,0,"NOT YET AVAILABLE\n\r");*/
	print_at(0,0,"%s\n\r",ML(mlStdMess194));
}

void
header (
 void)
{
	init_scr();
	set_tty();
	swide();
}

void
prnt_line (
 char *	_prnt_str)
{
	l_count++;
	if ( l_count > PAGE_LEN )
		l_count = 0;

	Dsp_saverec( _prnt_str );
}

void
finish_page (
 void)
{
	int	i;
	
	for (i = l_count; i < PAGE_LEN; i++)
		Dsp_saverec( "" );
}

void
proc_sumr (
 void)
{
	int	i;

	p_count = 0;
	strcpy( err_str, "                                      SUPPLIER MASTER FILE AUDIT OF CHANGES MADE.                                        ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[FN5-Print]    [FN14-Next]    [FN15-Prev]   [FN16-Input/End]" );

	cc = RF_READ( wk_no, (char *) &sumr1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "Supplier : %s ( %s ) User : %s / Date : %s / Time : %s",
				sumr1_rec.wk_crd_no,
				sumr1_rec.wk_name,
				sumr1_rec.wk_user_name,
				DateToString( sumr1_rec.wk_log_date ),
				sumr1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &sumr2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if ( strcmp( sumr1_rec.wk_acronym, sumr2_rec.wk_acronym ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Acronym",
				sumr1_rec.wk_acronym,
				sumr2_rec.wk_acronym);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 4; i++)
		{
			if ( strcmp(sumr1_rec.wk_adr[i],sumr2_rec.wk_adr[i]))
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Address",
					sumr1_rec.wk_adr[i],
					sumr2_rec.wk_adr[i]);

				prnt_line( disp_str );
			}
		}
		if ( strcmp( sumr1_rec.wk_acc_type, sumr2_rec.wk_acc_type ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"account type",
				( sumr1_rec.wk_acc_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F",
				( sumr2_rec.wk_acc_type[0] == 'O' ) ?
						"O(pen item" : "B(alance B/F" );

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_debtor_no, sumr2_rec.wk_debtor_no ) )
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Customer No",
				sumr1_rec.wk_debtor_no,
				sumr2_rec.wk_debtor_no);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_cont_name, sumr2_rec.wk_cont_name ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Contact Name.",
				sumr1_rec.wk_cont_name, 
				sumr2_rec.wk_cont_name);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_cont_no, sumr2_rec.wk_cont_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Phone No.",
				sumr1_rec.wk_cont_no, 
				sumr2_rec.wk_cont_no);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_fax_no, sumr2_rec.wk_fax_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Fax No.",
				sumr1_rec.wk_fax_no, 
				sumr2_rec.wk_fax_no);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_curr_code, sumr2_rec.wk_curr_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Currency Code.",
				sumr1_rec.wk_curr_code, 
				sumr2_rec.wk_curr_code);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_ctry_code, sumr2_rec.wk_ctry_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Country Code.",
				sumr1_rec.wk_ctry_code, 
				sumr2_rec.wk_ctry_code);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_pay_terms, sumr2_rec.wk_pay_terms ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Payment Terms.",
				sumr1_rec.wk_pay_terms, 
				sumr2_rec.wk_pay_terms);

			prnt_line( disp_str );
		}
		if ( sumr1_rec.wk_disc != sumr2_rec.wk_disc )
		{
			sprintf(wk1_str, "%4.2f", sumr1_rec.wk_disc );
			sprintf(wk2_str, "%4.2f", sumr2_rec.wk_disc );

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Discount %",
				wk1_str, wk2_str);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 3; i++ )
		{
			if ( sumr1_rec.wk_sic[i] != sumr2_rec.wk_sic[i] )
			{
				sprintf(wk1_str, "%03d", sumr1_rec.wk_sic[i]);
				sprintf(wk2_str, "%03d", sumr2_rec.wk_sic[i]);

				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Special Inst.", wk1_str, wk2_str);
	
				prnt_line( disp_str );
			}
		}
		if ( strcmp( sumr1_rec.wk_gl_ctrl_acct, sumr2_rec.wk_gl_ctrl_acct ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Control Account.",
				sumr1_rec.wk_gl_ctrl_acct, 
				sumr2_rec.wk_gl_ctrl_acct);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_hold_payment, sumr2_rec.wk_hold_payment ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Hold Payments.",
				sumr1_rec.wk_hold_payment, 
				sumr2_rec.wk_hold_payment);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_pay_method, sumr2_rec.wk_pay_method ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Payment Method.",
				sumr1_rec.wk_pay_method, 
				sumr2_rec.wk_pay_method);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_bank, sumr2_rec.wk_bank ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Bank.",
				sumr1_rec.wk_bank, 
				sumr2_rec.wk_bank);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_bank_branch, sumr2_rec.wk_bank_branch ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Bank Branch Code.",
				sumr1_rec.wk_bank_branch, 
				sumr2_rec.wk_bank_branch);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_bank_code, sumr2_rec.wk_bank_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Bank Code.",
				sumr1_rec.wk_bank_code, 
				sumr2_rec.wk_bank_code);

			prnt_line( disp_str );
		}
		if ( strcmp( sumr1_rec.wk_bank_acct_no, sumr2_rec.wk_bank_acct_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Bank Acct No.",
				sumr1_rec.wk_bank_acct_no, 
				sumr2_rec.wk_bank_acct_no);

			prnt_line( disp_str );
		}
		if ( sumr1_rec.wk_date_opened != sumr2_rec.wk_date_opened )
		{
			sprintf(wk1_str,"%s",DateToString(sumr1_rec.wk_date_opened));
			sprintf(wk2_str,"%s",DateToString(sumr2_rec.wk_date_opened));

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Date opened.",
				wk1_str, wk2_str);

			prnt_line( disp_str );
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");

		finish_page();
		cc = RF_READ( wk_no, (char *) &sumr1_rec );
	}
	Dsp_srch();
	Dsp_close();
}

void
proc_comr (
 void)
{
	int		i;
	p_count = 0;

	strcpy( err_str, "                                      COMPANY MASTER FILE AUDIT OF CHANGES MADE.                                        ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[FN5-Print]    [FN14-Next]    [FN15-Prev]   [FN16-Input/End]" );

	cc = RF_READ( wk_no, (char *) &comr1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "Co  : %s ( %s ) User : %s / Date : %s / Time : %s",
				comr1_rec.wk_co_no,
				comr1_rec.wk_co_name,
				comr1_rec.wk_user_name,
				DateToString( comr1_rec.wk_log_date ),
				comr1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &comr2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if ( strcmp( comr1_rec.wk_co_sname, comr2_rec.wk_co_sname ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Short name.",
				comr1_rec.wk_co_sname, 
				comr2_rec.wk_co_sname);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 3; i++)
		{
			if ( strcmp(comr1_rec.wk_co_adr[i],comr2_rec.wk_co_adr[i]))
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Address",
					comr1_rec.wk_co_adr[i],
					comr2_rec.wk_co_adr[i]);

				prnt_line( disp_str );
			}
		}
		if ( comr1_rec.wk_fiscal != comr2_rec.wk_fiscal )
		{
			sprintf(wk1_str, "%02d", comr1_rec.wk_fiscal);
			sprintf(wk2_str, "%02d", comr2_rec.wk_fiscal);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Fiscal Year end.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_yend_date != comr2_rec.wk_yend_date )
		{
			sprintf(wk1_str, "%s", DateToString(comr1_rec.wk_yend_date));
			sprintf(wk2_str, "%s", DateToString(comr2_rec.wk_yend_date));

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"GL Y/end Date.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_gst_rate != comr2_rec.wk_gst_rate )
		{
			sprintf(wk1_str, "%4.2f", comr1_rec.wk_gst_rate);
			sprintf(wk2_str, "%4.2f", comr2_rec.wk_gst_rate);

			sprintf(disp_str, "   ^E %-3.3s%-18.18s ^E %-42.42s ^E %-42.42s ^E",
			 		gst_code, " Rate.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_restock_pc != comr2_rec.wk_restock_pc )
		{
			sprintf(wk1_str, "%4.2f", comr1_rec.wk_restock_pc);
			sprintf(wk2_str, "%4.2f", comr2_rec.wk_restock_pc);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Re-stocking %.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( strcmp( comr1_rec.wk_gst_ird_no, comr2_rec.wk_gst_ird_no ))
		{
			sprintf(disp_str, "   ^E %-3.3s%-18.18s ^E %-42.42s ^E %-42.42s ^E",
				gst_code,
			 	"GST/IRD Number.",
				comr1_rec.wk_gst_ird_no, 
				comr2_rec.wk_gst_ird_no);

			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_int_rate != comr2_rec.wk_int_rate )
		{
			sprintf(wk1_str, "%4.2f", comr1_rec.wk_int_rate);
			sprintf(wk2_str, "%4.2f", comr2_rec.wk_int_rate);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Interest %.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		for ( i = 0; i < 5; i++)
		{
			if ( strcmp(comr1_rec.wk_pdesc[i],comr2_rec.wk_pdesc[i]))
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Price Desc",
					comr1_rec.wk_pdesc[i],
					comr2_rec.wk_pdesc[i]);

				prnt_line( disp_str );
			}
		}
		if ( comr1_rec.wk_sur_amt != comr2_rec.wk_sur_amt )
		{
			sprintf(wk1_str,"%8.2f",DOLLARS(comr1_rec.wk_sur_amt));
			sprintf(wk2_str,"%8.2f",DOLLARS(comr2_rec.wk_sur_amt));

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Surcharge Amt.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_sur_cof != comr2_rec.wk_sur_cof )
		{
			sprintf(wk1_str,"%8.2f",DOLLARS(comr1_rec.wk_sur_cof));
			sprintf(wk2_str,"%8.2f",DOLLARS(comr2_rec.wk_sur_cof));

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Surcharge Cutoff.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_frt_min_amt != comr2_rec.wk_frt_min_amt )
		{
			sprintf(wk1_str,"%8.2f",DOLLARS(comr1_rec.wk_frt_min_amt));
			sprintf(wk2_str,"%8.2f",DOLLARS(comr2_rec.wk_frt_min_amt));

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Min freight chg.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		if ( comr1_rec.wk_frt_mweight != comr2_rec.wk_frt_mweight )
		{
			sprintf(wk1_str,"%4.4f",comr1_rec.wk_frt_mweight);
			sprintf(wk2_str,"%4.4f",comr2_rec.wk_frt_mweight);

			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Min freight weight.", wk1_str, wk2_str);
	
			prnt_line( disp_str );
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");
		finish_page();
		cc = RF_READ( wk_no, (char *) &comr1_rec );
	}
	Dsp_srch();
	Dsp_close();
}

void
proc_esmr (
 void)
{
	int		i;
	p_count = 0;

	strcpy( err_str, "                                       BRANCH MASTER FILE AUDIT OF CHANGES MADE.                                         ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[FN5-Print]    [FN14-Next]    [FN15-Prev]   [FN16-Input/End]" );

	cc = RF_READ( wk_no, (char *) &esmr1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "Br  : %s ( %s ) User : %s / Date : %s / Time : %s",
				esmr1_rec.wk_no,
				esmr1_rec.wk_name,
				esmr1_rec.wk_user_name,
				DateToString( esmr1_rec.wk_log_date ),
				esmr1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &esmr2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if ( strcmp( esmr1_rec.wk_short, esmr2_rec.wk_short ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Branch Short Name.",
				esmr1_rec.wk_short, 
				esmr2_rec.wk_short);

			prnt_line( disp_str );
		}
		if ( strcmp( esmr1_rec.wk_area_code, esmr2_rec.wk_area_code ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Area Code.",
				esmr1_rec.wk_area_code, 
				esmr2_rec.wk_area_code);

			prnt_line( disp_str );
		}
		for ( i = 0; i < 3; i++)
		{
			if ( strcmp(esmr1_rec.wk_adr[i],esmr2_rec.wk_adr[i]))
			{
				sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 		"Address",
					esmr1_rec.wk_adr[i],
					esmr2_rec.wk_adr[i]);

				prnt_line( disp_str );
			}
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");
		finish_page();
		cc = RF_READ( wk_no, (char *) &esmr1_rec );
	}
	Dsp_srch();
	Dsp_close();
}

void
proc_ccmr (
 void)
{
	p_count = 0;

	strcpy( err_str, "                                     WAREHOUSE MASTER FILE AUDIT OF CHANGES MADE.                                        ");
	Dsp_prn_open( 0,0,18,err_str, (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0,
				      (char *) 0, (char *) 0 );
	Dsp_saverec( err_str );
	Dsp_saverec( "" );
	Dsp_saverec( "[FN5-Print]    [FN14-Next]    [FN15-Prev]   [FN16-Input/End]" );

	cc = RF_READ( wk_no, (char *) &ccmr1_rec );
	while ( !cc )
	{
		sprintf( disp_str, "^1 Page : %4d ^6", ++p_count);
		l_count = 999;
		prnt_line( disp_str );

		sprintf( disp_str, "W/H : %s ( %s ) User : %s / Date : %s / Time : %s",
				ccmr1_rec.wk_cc_no,
				ccmr1_rec.wk_name,
				ccmr1_rec.wk_user_name,
				DateToString( ccmr1_rec.wk_log_date ),
				ccmr1_rec.wk_log_time );

		prnt_line( disp_str );

		cc = RF_READ( wk_no, (char *) &ccmr2_rec );
		if ( cc )
			break;

		prnt_line("   ^^AGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGB^^");
		prnt_line("   ^E   Field Changed       ^E               Old Value.                   ^E               New Value.                   ^E");
		prnt_line("   ^^EGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGL^^");

		if ( strcmp( ccmr1_rec.wk_acronym, ccmr2_rec.wk_acronym ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Warehouse Short Name.",
				ccmr1_rec.wk_acronym, 
				ccmr2_rec.wk_acronym);

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_mst_wh, ccmr2_rec.wk_mst_wh ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Master W/H.",
				( ccmr1_rec.wk_mst_wh[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_mst_wh[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_sal_ok, ccmr2_rec.wk_sal_ok ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Sales Allowed.",
				( ccmr1_rec.wk_sal_ok[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_sal_ok[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_pur_ok, ccmr2_rec.wk_pur_ok ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Purchases Allowed.",
				( ccmr1_rec.wk_pur_ok[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_pur_ok[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_iss_ok, ccmr2_rec.wk_iss_ok ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Issues Allowed.",
				( ccmr1_rec.wk_iss_ok[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_iss_ok[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_rec_ok, ccmr2_rec.wk_rec_ok ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Receipts Allowed.",
				( ccmr1_rec.wk_rec_ok[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_rec_ok[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_rep_ok, ccmr2_rec.wk_rep_ok ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Reports Allowed.",
				( ccmr1_rec.wk_rep_ok[0] == 'Y' ) ?
						"Y(es" : "N(o",
				( ccmr2_rec.wk_rep_ok[0] == 'Y' ) ?
						"Y(es" : "N(o");

			prnt_line( disp_str );
		}
		if ( strcmp( ccmr1_rec.wk_sman_no, ccmr2_rec.wk_sman_no ))
		{
			sprintf(disp_str, "   ^E %-21.21s ^E %-42.42s ^E %-42.42s ^E",
			 	"Salesman No.",
				ccmr1_rec.wk_sman_no,
				ccmr2_rec.wk_sman_no);

			prnt_line( disp_str );
		}
		prnt_line("   ^^CGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGD^^");
		finish_page();
		cc = RF_READ( wk_no, (char *) &ccmr1_rec );
	}
	Dsp_srch();
	Dsp_close();
}
