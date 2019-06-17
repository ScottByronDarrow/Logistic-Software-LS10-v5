/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_ctr_inv.c   )                                 |
|  Program Desc  : ( Counter Invoice Print                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cohr, coln, cmhr, cmcd, cmpb,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr, coln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 18/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (26/04/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (28/04/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (18/05/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (31/05/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (16/10/97)      | Modified  by : Leah Manibog.	  |
|                :                                                    |
|  (26/04/93)    :  EGC 8499 checking that (null) not passed as hash  |
|  (28/04/93)    :  EGC 8912 Should use cohr_status to check if CONT  |
|  (18/05/93)    :  EGC 8909 Invoice Number field Added.              |
|  (31/05/93)    :  EGC 9065 Remove ongoing contract description.     |
|                :  Editted description should appear in the special  |
|                :  instruction box and nowhere else.                 |
|  (15/11/95)    :  PDL - Updated for version 9.                      |
|  (16/10/97)    :  Changed length of invoice no. from 6 to 8 char.   |
|                :                                                    |
|                                                                     |
| $Log: cm_ctr_inv.c,v $
| Revision 5.2  2001/08/09 08:57:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:09  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:04  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:24  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.10  2000/04/25 09:57:20  ramon
| Added a checking at the start if there's no data to be processed.
|
| Revision 1.9  1999/12/06 01:32:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/09/29 10:10:15  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 04:40:08  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.6  1999/09/16 04:44:41  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/14 07:34:16  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
char	*PNAME = "$RCSfile: cm_ctr_inv.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_ctr_inv/cm_ctr_inv.c,v 5.2 2001/08/09 08:57:17 scott Exp $";

#define	CCMAIN
#define	LINE_UP		(cohr_rec.hr_hhco_hash < 0L)
#define	LINE_UP_CHAR(x)	(LINE_UP) ? line_up : x

#define	INVOICE		(cohr_rec.hr_type[0] == 'I' || LINE_UP)
#define	REPRINT		(cohr_rec.hr_inv_print[0] == 'Y')

#define	UPD		(stat_flag[0] == 'S')
#define	BLANK_LN	"                                                                      "

#define	SINGLE	0
#define	MULTI	1

#define	LINES	17
#include <pslscr.h>

char	*line_up = "LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_";

char	*hold_str = "HOLD - DO NOT SEND,HOLD - DO NOT SEND,HOLD - DO NOT SEND,HOLD - DO NOT SEND";

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_adr1"},
		{"comr_co_adr2"},
		{"comr_co_adr3"},
		{"comr_gst_ird_no"},
		{"comr_stat_flag"},
		{"comr_gst_rate"}
	};

	int comr_no_fields = 8;

	struct {
		char	cm_co_no[3];
		char	cm_co_name[41];
		char	cm_co_adr[3][41];
		char	cm_gst_ird_no[11];
		char	cm_stat_flag[2];
		float	cm_gst;
	} comr_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
	};

	int cumr_no_fields = 6;

	struct {
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_addr[3][41];
	} cumr_rec;

	/*=============================================
	| Customer P-slip/ Invoice/Credit Header File |
	=============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
		{"cohr_gross"},
		{"cohr_freight"},
		{"cohr_other_cost_1"},
		{"cohr_other_cost_2"},
		{"cohr_other_cost_3"},
		{"cohr_tax"},
		{"cohr_gst"},
		{"cohr_disc"},
		{"cohr_deposit"},
		{"cohr_ex_disc"},
		{"cohr_dl_name"},
		{"cohr_dl_add1"},
		{"cohr_dl_add2"},
		{"cohr_dl_add3"},
		{"cohr_status"},
		{"cohr_stat_flag"},
		{"cohr_inv_print"},
	};

	int	cohr_no_fields = 26;

	struct	{
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_dp_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhco_hash;
		long	hr_date_raised;
		long	hr_date_required;
		double	hr_gross;	/* money */
		double	hr_freight;	/* money */
		double	hr_other_cost[3];	/* money */
		double	hr_tax;	/* money */
		double	hr_gst;	/* money */
		double	hr_disc;	/* money */
		double	hr_deposit;	/* money */
		double	hr_ex_disc;	/* money */
		char	hr_dl_name[41];
		char	hr_dl_add1[41];
		char	hr_dl_add2[41];
		char	hr_dl_add3[41];
		char	hr_status[2];
		char	hr_stat_flag[2];
		char	hr_inv_print[2];
	} cohr_rec;

	/*============================================
	| Customer Order/Invoice/Credit Detail File. |
	============================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_serial_no"},
		{"coln_q_order"},
		{"coln_q_backorder"},
		{"coln_sale_price"},
		{"coln_disc_pc"},
		{"coln_gross"},
		{"coln_amt_disc"},
		{"coln_amt_tax"},
		{"coln_amt_gst"},
		{"coln_pack_size"},
		{"coln_item_desc"},
		{"coln_due_date"},
		{"coln_bonus_flag"},
	};

	int coln_no_fields = 16;

	struct {
		long	ln_hhco_hash;
		int		ln_line_no;
		long	ln_hhbr_hash;
		char	ln_serial_no[26];
		float	ln_q_order;
		float	ln_q_backorder;
		double	ln_sale_price;		/*  Money field  */
		float	ln_disc_pc;
		double	ln_gross;		/*  Money field  */
		double	ln_amt_disc;		/*  Money field  */
		double	ln_amt_tax;		/*  Money field  */
		double	ln_amt_gst;		/*  Money field  */
		char	ln_pack_size[6];
		char	ln_item_desc[41];
		long	ln_due_date;
		char	ln_bonus_flag[2];
	} coln_rec;

	/*===============================================
	| Contract Management Contract Description File |
	===============================================*/
	struct dbview cmcd_list[] ={
		{"cmcd_hhhr_hash"},
		{"cmcd_line_no"},
		{"cmcd_text"},
		{"cmcd_stat_flag"},
	};

	int	cmcd_no_fields = 4;

	struct	{
		long	cd_hhhr_hash;
		int		cd_line_no;
		char	cd_text[71];
		char	cd_stat_flag[2];
	} cmcd_rec;

	/*===========================================
	| Contract Management Progress Billing File |
	===========================================*/
	struct dbview cmpb_list[] ={
		{"cmpb_hhhr_hash"},
		{"cmpb_hhco_hash"},
	};

	int	cmpb_no_fields = 2;

	struct	{
		long	pb_hhhr_hash;
		long	pb_hhco_hash;
	} cmpb_rec;

	/*========================================
	| cmhr - Contract Management Header File |
	========================================*/
	struct dbview cmhr_list[] ={
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_mast_hhhr"},
		{"cmhr_hhcu_hash"},
		{"cmhr_hhit_hash"},
		{"cmhr_cus_ref"},
		{"cmhr_contact"},
		{"cmhr_adr1"},
		{"cmhr_adr2"},
		{"cmhr_adr3"},
		{"cmhr_it_date"},
		{"cmhr_wip_date"},
		{"cmhr_st_date"},
		{"cmhr_due_date"},
		{"cmhr_end_date"},
		{"cmhr_hhjt_hash"},
		{"cmhr_wip_status"},
		{"cmhr_quote_type"},
		{"cmhr_progress"},
		{"cmhr_anni_day"},
		{"cmhr_quote_val"},
		{"cmhr_est_costs"},
		{"cmhr_est_prof"},
		{"cmhr_usr_ref1"},
		{"cmhr_usr_ref2"},
		{"cmhr_usr_ref3"},
		{"cmhr_usr_ref4"},
		{"cmhr_usr_ref5"},
		{"cmhr_internal"},
		{"cmhr_lab_rate"},
		{"cmhr_oh_rate"},
		{"cmhr_status"},
		{"cmhr_premise"},
	};

	int	cmhr_no_fields = 35;

	struct	{
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_cont_no[7];
		long	hr_hhhr_hash;
		long	hr_mast_hhhr;
		long	hr_hhcu_hash;
		long	hr_hhit_hash;
		char	hr_cus_ref[21];
		char	hr_contact[41];
		char	hr_adr1[41];
		char	hr_adr2[41];
		char	hr_adr3[41];
		long	hr_it_date;
		long	hr_wip_date;
		long	hr_st_date;
		long	hr_due_date;
		long	hr_end_date;
		long	hr_hhjt_hash;
		char	hr_wip_status[5];
		char	hr_quote_type[2];
		char	hr_progress[2];
		char	hr_anni_day[3];
		double	hr_quote_val;	/* money */
		double	hr_est_costs;	/* money */
		float	hr_est_prof;
		char	hr_usr_ref1[5];
		char	hr_usr_ref2[5];
		char	hr_usr_ref3[5];
		char	hr_usr_ref4[5];
		char	hr_usr_ref5[5];
		char	hr_internal[2];
		double	hr_lab_rate;	/* money */
		double	hr_oh_rate;	/* money */
		char	hr_status[2];
		char	hr_premise[21];
	} cmhr_rec;

	char	*comr	= "comr",
			*cumr	= "cumr",
			*cohr	= "cohr",
			*coln	= "coln",
			*cmhr	= "cmhr",
			*cmcd	= "cmcd",
			*cmpb	= "cmpb",
			*data	= "data";

	int		lpno = 1,
			page_no,
			line_no;

	char	*gets(char *),
			stat_flag[2];

	FILE	*fin,
			*fout;

/*==========================
| Main Processing Routine. |
==========================*/
int		main			(int argc, char * argv []);
void	open_out		(void);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		proc_cohr		(long hhco_hash);
int		proc_coln		(long hhco_hash);
int		heading			(void);
void	pr_line			(void);
void	page_break		(void);
int		check_page		(void);
void	print_cmcds		(void);

#include	<pr_format3.h>

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		first_time = 1;
	int		pipe_open = FALSE;
	int		print_status = SINGLE;
	long	hhco_hash;
	char	*sptr;

	sptr = gets(err_str);
	if (!sptr)
		return (EXIT_FAILURE);

	lpno = atoi (sptr);

	/*-----------------------------------------------
	| Stat_flag MUST be "M" for multiple invoices	|
	-----------------------------------------------*/
	sprintf(stat_flag,"%1.1s",gets(err_str));

	OpenDB();

	if ((fin = pr_open("cm_ctr_inv.p")) == NULL)
		sys_err("Error in cm_ctr_inv.p During (FOPEN)",errno,PNAME);

	while (1)
	{
		sptr = gets(err_str);
		if ( !sptr )
			break;

		hhco_hash = atol(sptr);

		if (hhco_hash == 0L)
		{
			/*-----------------------------------------------
			| Not printing multiple invoices or already	|
			| printing multiple invoices. Break out of loop	|
			-----------------------------------------------*/
			first_time = 0;
			if (stat_flag[0] != 'M' || print_status == MULTI)
				break;
			else
			{
				print_status = MULTI;
				open_out();
				pipe_open = TRUE;
				continue;
			}
		}

		/*---------------------------------------
		| Printing Single Jobs or First Time	|
		---------------------------------------*/
		if (print_status == SINGLE || first_time)
		{
			open_out();
			pipe_open = TRUE;
		}

		proc_cohr(hhco_hash);

		/*---------------------------------------
		| Printing Single Jobs or First Time	|
		---------------------------------------*/
		if (print_status == SINGLE || first_time)
		{
			first_time = 0;

			fprintf(fout,".EOF\n");
			pclose(fout);
		}
	}
	fprintf(fout,".EOF\n");

	if ( pipe_open )
		pclose(fout);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
open_out (void)
{
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	fprintf(fout,".START00/00/00\n");
	fprintf(fout,".OP\n");
	fprintf(fout,".PL0\n");
	fprintf(fout,".LP%d\n",lpno);
	fprintf(fout,".2\n");
	fprintf(fout,".PI12\n");
	fprintf(fout,".L140\n");
	fflush(fout);
}	

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
}

void
OpenDB (void)
{
	abc_dbopen( data );
	open_rec( cumr, cumr_list,cumr_no_fields,"cumr_hhcu_hash");
	open_rec( cohr, cohr_list,cohr_no_fields,"cohr_hhco_hash");
	open_rec( cmpb, cmpb_list,cmpb_no_fields,"cmpb_hhco_hash");
	open_rec( cmhr, cmhr_list,cmhr_no_fields,"cmhr_hhhr_hash");
	open_rec( cmcd, cmcd_list,cmcd_no_fields,"cmcd_id_no");
	open_rec( coln, coln_list,coln_no_fields,"coln_id_no");
	open_rec( comr, comr_list,comr_no_fields,"comr_co_no");
}

void
CloseDB (void)
{
	abc_fclose( cumr );
	abc_fclose( cohr );
	abc_fclose( coln );
	abc_fclose( comr );
	abc_fclose( cmpb );
	abc_fclose( cmhr );
	abc_fclose( cmcd );
	abc_dbclose( data );
}

int
proc_cohr (
 long	hhco_hash)
{
	double	sub_tot = 0.00;
	double	total = 0.00;

	page_no = 1;
	line_no = 0;

	if (hhco_hash > 0L)
	{
		cc = find_hash( cohr, &cohr_rec,COMPARISON,"u",hhco_hash);
		if (cc)
			return(cc);

		if ( cohr_rec.hr_status[0] != 'C' )
			return ( TRUE );

		strcpy(comr_rec.cm_co_no,cohr_rec.hr_co_no);
		cc = find_rec( comr, &comr_rec,COMPARISON,"r");
		if (cc)
			return(cc);

		cc = find_hash(cumr,&cumr_rec,COMPARISON,"r", cohr_rec.hr_hhcu_hash);
		if (cc)
			return(cc);
	}
	else
		cohr_rec.hr_hhco_hash = -1L;

	if (proc_coln(hhco_hash))
		return (EXIT_SUCCESS);

	/*-----------------------
	| Total for Invoice	|
	-----------------------*/
	sub_tot = (LINE_UP) ? 0.00 : cohr_rec.hr_gross + cohr_rec.hr_tax - 
		cohr_rec.hr_disc - cohr_rec.hr_deposit - cohr_rec.hr_ex_disc +
		cohr_rec.hr_other_cost[0] + cohr_rec.hr_other_cost[1] +
		cohr_rec.hr_other_cost[2];

	total  = (LINE_UP) ? 0.00 : sub_tot + cohr_rec.hr_gst;

	pr_format(fin,fout,"VBLE_BLANK",1,LINES - line_no);

	pr_format(fin,fout,"MARGIN3",0,0);

	pr_format(fin,fout,"SUB_TOT",1,(LINE_UP) ? 0.00 : DOLLARS(sub_tot));

	/*----------------------------------------------
	| Print editted on-going contract description. |
	----------------------------------------------*/
	cmcd_rec.cd_hhhr_hash = cmhr_rec.hr_hhhr_hash;
	cmcd_rec.cd_stat_flag[0] = 'I';
	cmcd_rec.cd_line_no = 0;
	cc = find_rec ( cmcd, &cmcd_rec, EQUAL, "r" );
	if ( cc )
		strcpy ( cmcd_rec.cd_text, BLANK_LN );

	pr_format(fin,fout,"CMCDI",1,LINE_UP_CHAR(cmcd_rec.cd_text));
	pr_format(fin,fout,"CMCDI",2,(LINE_UP) ? 0.00 : DOLLARS(total-sub_tot));

	cmcd_rec.cd_hhhr_hash = cmhr_rec.hr_hhhr_hash;
	cmcd_rec.cd_stat_flag[0] = 'I';
	cmcd_rec.cd_line_no = 1;
	cc = find_rec ( cmcd, &cmcd_rec, EQUAL, "r" );
	if ( cc )
		strcpy ( cmcd_rec.cd_text, BLANK_LN );
	pr_format(fin,fout,"CMCD2",1,LINE_UP_CHAR(cmcd_rec.cd_text));

	cmcd_rec.cd_hhhr_hash = cmhr_rec.hr_hhhr_hash;
	cmcd_rec.cd_stat_flag[0] = 'I';
	cmcd_rec.cd_line_no = 2;
	cc = find_rec ( cmcd, &cmcd_rec, EQUAL, "r" );
	if ( cc )
		strcpy ( cmcd_rec.cd_text, BLANK_LN );

	pr_format(fin,fout,"CMCDI",1,LINE_UP_CHAR(cmcd_rec.cd_text));
	pr_format(fin,fout,"CMCDI",2,(LINE_UP) ? 0.00 : DOLLARS(total));





	pr_format(fin,fout,"NEXT_PAGE",0,0);
	fflush(fout);

	if (UPD)
	{
		strcpy(cohr_rec.hr_inv_print,"Y");
		cc = abc_update( cohr, &cohr_rec);
	}
	abc_unlock( cohr );
	return(0);
}

int
proc_coln (
 long	hhco_hash)
{
	int	printed = 0;
	int	first_time = 1;

	/*-----------------
	| get header info |
	-----------------*/

	if ( !LINE_UP )
	{
		cc = find_hash ( cmpb, &cmpb_rec, EQUAL, "r", hhco_hash );
		if ( cc )
			return (cc);

		cc = find_hash(cmhr,&cmhr_rec,EQUAL,"r",cmpb_rec.pb_hhhr_hash);
		if ( cc )
			return (cc);
	}

	heading();
	print_cmcds ();

	/*--------------------------
	| Process all order lines. |
	--------------------------*/
	coln_rec.ln_hhco_hash = hhco_hash;
	coln_rec.ln_line_no = 0;
	cc = find_rec( coln, &coln_rec,GTEQ,"r");

	while (!cc && coln_rec.ln_hhco_hash == hhco_hash)
	{

		pr_line();
		printed = 1;
		first_time = 0;

		cc = find_rec( coln, &coln_rec,NEXT,"r");
	}

	if (!printed)
	{
		do
		{
			pr_format(fin,fout,"INV_LINE",1,LINE_UP_CHAR(hold_str));
			pr_format(fin,fout,"INV_LINE",2,0.00);
			pr_format(fin,fout,"INV_LINE",3,0.00);
			pr_format(fin,fout,"INV_LINE",4,0.00);
		} while (++line_no < LINES);
	}
	return (EXIT_SUCCESS);
}

int
heading (void)
{
	char	re_prt[8];

	pr_format(fin,fout,"HMARGIN",0,0);
	
	strcpy(re_prt, "       ");
	if (REPRINT)
		strcpy(re_prt, "REPRINT");

	pr_format(fin,fout,"REPRINT",1,LINE_UP_CHAR(re_prt));
	pr_format(fin,fout,"MARGIN4",0,0);

	if (INVOICE)
		pr_format(fin,fout,"INVOICE",1,LINE_UP_CHAR("TAX INVOICE"));
	else
		pr_format(fin,fout,"INVOICE",1,LINE_UP_CHAR("CREDIT NOTE"));
	pr_format(fin,fout,"INVOICE",2,LINE_UP_CHAR(cohr_rec.hr_inv_no));

	pr_format(fin,fout,"NAME",1,LINE_UP_CHAR(cumr_rec.cm_dbt_name));
	pr_format(fin,fout,"NAME",2,LINE_UP_CHAR(cmhr_rec.hr_contact));
	pr_format(fin,fout,"NAME",3,cohr_rec.hr_date_raised);

	pr_format(fin,fout,"ADR1",1,LINE_UP_CHAR(cumr_rec.cm_addr[0]));
	pr_format(fin,fout,"ADR1",2,LINE_UP_CHAR(cmhr_rec.hr_adr1));
	pr_format(fin,fout,"ADR1",3,LINE_UP_CHAR(cumr_rec.cm_dbt_no));

	pr_format(fin,fout,"ADR2",1,LINE_UP_CHAR(cumr_rec.cm_addr[1]));
	pr_format(fin,fout,"ADR2",2,LINE_UP_CHAR(cmhr_rec.hr_adr2));
	pr_format(fin,fout,"ADR2",3,(LINE_UP) ? 1 : page_no );

	pr_format(fin,fout,"ADR3",1,LINE_UP_CHAR(cumr_rec.cm_addr[2]));
	pr_format(fin,fout,"ADR3",2,LINE_UP_CHAR(cmhr_rec.hr_adr3));

	pr_format(fin,fout,"MARGIN1",0,0);

	pr_format(fin,fout,"HDG",1,LINE_UP_CHAR(cmhr_rec.hr_cont_no));
	pr_format(fin,fout,"HDG",2,LINE_UP_CHAR(cmhr_rec.hr_cus_ref));
	pr_format(fin,fout,"HDG",3,LINE_UP_CHAR(cmhr_rec.hr_usr_ref1));
	pr_format(fin,fout,"HDG",4,LINE_UP_CHAR(cmhr_rec.hr_usr_ref2));

	pr_format(fin,fout,"MARGIN2",0,0);

	return (EXIT_SUCCESS);
}

void
pr_line (void)
{
	float	delivered;
	double	sale_price;
	double	disc;
	double	gross;

	delivered  = coln_rec.ln_q_order;
	sale_price = coln_rec.ln_sale_price;
	disc       = coln_rec.ln_amt_disc;

	gross =  coln_rec.ln_gross - disc;

	/*-------------------------------
	| Print normal invoice line	|
	-------------------------------*/
	page_break();
	pr_format(fin,fout,"INV_LINE",1,coln_rec.ln_item_desc);
	pr_format(fin,fout,"INV_LINE",2,delivered);
	pr_format(fin,fout,"INV_LINE",3,DOLLARS(sale_price));
	pr_format(fin,fout,"INV_LINE",4,DOLLARS(gross));
}

void
page_break (void)
{
	if (line_no++ >= LINES)
	{
		page_no++;
		pr_format(fin,fout,"CONT_SKIP",0,0);
		pr_format(fin,fout,"CONT",0,0);
		pr_format(fin,fout,"NEXT_PAGE",0,0);
		heading();
		line_no = 0;
	}
}

int
check_page (void)
{
	return(0);
}

void
print_cmcds (void)
{
	cmcd_rec.cd_hhhr_hash = cmhr_rec.hr_hhhr_hash;
	cmcd_rec.cd_stat_flag[0] = 'D';
	cmcd_rec.cd_line_no = 0;

	cc = find_rec ( cmcd, &cmcd_rec, GTEQ, "r" );
	while (!cc && 
	       cmcd_rec.cd_hhhr_hash == cmhr_rec.hr_hhhr_hash &&
	       cmcd_rec.cd_stat_flag[0] == 'D' &&
	       cmcd_rec.cd_line_no < 7)
	{
		if ( !strcmp ( cmcd_rec.cd_text, BLANK_LN ))
		{
			cc = find_rec ( cmcd, &cmcd_rec, NEXT, "r" );
			continue;
		}

		page_break ();
		pr_format(fin,fout,"CMCDD",1,cmcd_rec.cd_text);
		cc = find_rec(cmcd, &cmcd_rec, NEXT, "r");
	}
}

