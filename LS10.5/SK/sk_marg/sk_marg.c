/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_marg.c      )                                 |
|  Program Desc  : ( Print Invoice Profit Margin Exception Report )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, coln, cohr, excf, inmr,               |
|  Database      : (sodb)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
|  Date Modified : (09/06/88)      | Modified by : Scott Darrow.      |
|  Date Modified : (08/11/88)      | Modified by : B. C. Lim.         |
|  Date Modified : (20/02/89)      | Modified by : Scott Darrow.      |
|  Date Modified : (01/06/89)      | Modified by : Fui Choo Yap.      |
|  Date Modified : (23/05/91)      | Modified by : Campbell Mander.   |
|  Date Modified : (07/06/91)      | Modified by : Campbell Mander.   |
|  Date Modified : (19/05/92)      | Modified by : Simon Dubey.       |
|  Date Modified : (18/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (17/09/97)      | Modified by : Roanna Marcelino   |
|  Date Modified : (19/03/98)      | Modified by : Ronnel L. Amanca.  |
|  Date Modified : (24/03/98)      | Modified by : Ana Marie C Tario. |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|        20/02/89: Fixed % Calculation.                               |
|      (01/06/89): Rewrite prog. using sort routines.                 |
|                :                                                    |
|   (23/05/91)   : If -1.00 is passed as margin percentage then the   |
|                : margin percentage is read from the excf file for   |
|                : the items category.                                |
|                :                                                    |
|   (07/06/91)   : If mgn_pc cannot be calculated ie. cost == 0 then  |
|                : ***** is printed instead of 100.00.                |
|                :                                                    |
|   (19/05/92)   : If the cost != 0 and the sales price == 0 then the |
|                : calc_act_marg function would be fell thru'.        |
|                : Simple insertion of ELSE. S/C DFH 6740.            |
|   (18/08/93)   : HGP 9649 Fix to compile on SVR4                    |
|   (17/09/97)   : Modified for Multilingual Conversion.              |
|   (19/03/98)   : Modified to fix bug in printing.                   |
|   (24/03/98)   : Corrected printing of item no.                     |
| $Log: sk_marg.c,v $
| Revision 5.2  2001/08/09 09:19:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:29  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:10  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/12/06 01:30:55  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/03 07:32:08  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.12  1999/10/14 01:20:55  cam
| Removed max_work
|
| Revision 1.11  1999/10/08 05:32:32  scott
| First Pass checkin by Scott.
|
| Revision 1.10  1999/06/20 05:20:15  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_marg.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_marg/sk_marg.c,v 5.2 2001/08/09 09:19:03 scott Exp $";

#define	ITEM	(group_by[0] == 'I')
#define	CUST	(group_by[0] == 'C')

#define		NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

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
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	t_inv_date;
	} comm_rec;

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
		{"cohr_date_required"},
		{"cohr_stat_flag"}
	};

	int cohr_no_fields = 10;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_dp_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhco_hash;
		long	hr_date_raised;
		long	hr_date_required;
		char	hr_stat_flag[2];
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
		{"coln_q_order"},
		{"coln_q_backorder"},
		{"coln_sale_price"},
		{"coln_cost_price"},
		{"coln_gross"},
		{"coln_amt_disc"},
		{"coln_item_desc"},
		{"coln_stat_flag"}
	};

	int coln_no_fields = 13;

	struct {
		long	ln_hhco_hash;
		int	ln_line_no;
		long	ln_hhbr_hash;
		long	ln_incc_hash;
		char	ln_serial_no[26];
		float	ln_q_order;
		float	ln_q_backorder;
		double	ln_sale_price;		/*  Money field  */
		double	ln_cost_price;		/*  Money field  */
		double	ln_gross;		/*  Money field  */
		double	ln_amt_disc;		/*  Money field  */
		char	ln_item_desc[41];
		char	ln_stat_flag[2];
	} coln_rec;

	/*===========================================
	| file inmr {Inventory Master Base Record}. |
	===========================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_outer_size"}
	};

	int inmr_no_fields = 10;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_supercession_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
		float	mr_outer_size;
	} inmr_rec;

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
		{"cumr_stat_flag"}
	};

	int cumr_no_fields = 8;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_dbt_acronym[10];
		char	cm_stat_flag[2];
	} cumr_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
		{"excf_min_marg"},
		{"excf_stat_flag"}
	};

	int excf_no_fields = 5;

	struct {
		char	cf_co_no[3];
		char	cf_categ_no[12];
		char	cf_cat_desc[41];
		float	cf_min_marg;
		char	cf_stat_flag[2];
	} excf_rec;

	struct	{
		char	number[12];
		float	mgn_pc;
	} curr_cat;

	int	lpno = 1;

	FILE	*fout;	
	FILE	*fsort;

	int	found_data = FALSE;
	int	envDbCo;
	int	envDbFind;
	int	MIN_MARG;
	int	zero_cost;

	float	mgn_pc = 0.0;
	float	act_pc = 0.0;

	double	cost = 0.00;
	double	sale = 0.00;

	double	totsale = 0.00;
	double	totcost = 0.00;
	double	totmgn = 0.00;

	double	gsale = 0.00;
	double	gcost = 0.00;
	double	gmgn = 0.00;

	long	start_date;
	long	end_date;

	char	head_desc[7]; 
	char	type_flag[2]; 
	char	lower[17];
	char	upper[17];
	char	stat_flag[2]; 
	char	all_br[2]; 
	char	group_by[2];

	char	*sptr;
	char	data_str[130];

/*=======================
| Function Declarations |
=======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void proc_cohr (void);
int  valid_cust (void);
int  valid_item (void);
void get_coln (long hhco_hash);
int  get_margin (void);
void store_data (void);
void calc_act_margin (void);
void proc_sorted (void);
void print_header (long reqd_hash);
void print_bline (void);
void print_line (void);
void print_line1 (void);
void print_totals (char *type);
float f_val (char *str);
long h_val (char *str);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 11)
	{
		print_at(0,0,mlSkMess302,argv[0]);
		return (EXIT_FAILURE);
	}
	
	envDbCo = atoi(get_env("DB_CO"));
	envDbFind  = atoi(get_env("DB_FIND"));

	lpno = atoi(argv[1]);
	sprintf(group_by,"%-1.1s",argv[2]);

	if (group_by[0] != 'I' && group_by[0] != 'C')
	{
		print_at(0,0,mlSkMess619);
		return (EXIT_FAILURE);
	}

	/*---------------------------------------------------------------------
	| For inmr, lower & upper are class-category boundaries for the report|
	| For cumr, they are acronym boundaries for the report		      |
	---------------------------------------------------------------------*/
	switch (group_by[0])
	{
	case	'I':
		sprintf(lower,"%-16.16s",argv[3]);
		sprintf(upper,"%-16.16s",argv[4]);
		break;

	case	'C':
		sprintf(lower,"%-9.9s",argv[3]);
		sprintf(upper,"%-9.9s",argv[4]);
		break;
	}

	MIN_MARG = FALSE;
	/*---------------------------------------
	| The %margin which is to be printed	|
	---------------------------------------*/
	mgn_pc = (float) atof(argv[5]);
	if (mgn_pc < 0.00)
	{
		MIN_MARG = TRUE;
		strcpy(curr_cat.number, "           ");
		curr_cat.mgn_pc = 0.00;
	}

	/*-----------------------------------------------
	| Choice to print Above or Below the %margin	|
	-----------------------------------------------*/
	sprintf(type_flag,"%-1.1s",argv[6]);

	/*-----------------------------------------------
	| The start & end dates to be considered	|
	-----------------------------------------------*/
	start_date = StringToDate(argv[7]);
	end_date   = StringToDate(argv[8]);

	/*---------------------------------------------------------------
	| Option to print all branches (Y) or current branch only (N)	|
	---------------------------------------------------------------*/
	sprintf(all_br,"%-1.1s",argv[9]);

	if (type_flag[0] != 'A' && type_flag[0] != 'B')
	{
		print_at(0,0,mlSkMess300);
		return (EXIT_FAILURE);
	}

	if (all_br[0] != 'Y' && all_br[0] != 'N')
	{
		print_at(0,0,mlSkMess301);
		return (EXIT_FAILURE);
	}

	sprintf(stat_flag,"%-1.1s",argv[10]);

	if (mgn_pc == 0.0)
		strcpy(head_desc,"(ALL) ");
	else
	{
		if (type_flag[0] == 'A')
			strcpy(head_desc,"ABOVE ");
		else
			strcpy(head_desc,"BELOW ");
	}
	/*======================
	| Open database files. |
	======================*/
	OpenDB();

	init_scr();

	dsp_screen("Processing : Sales Margin Exception Report", comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	head_output();

	proc_cohr();

    fprintf(fout,".EOF\n");

	pclose(fout);

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.t_inv_date), PNAME);
	fprintf(fout, ".SO\n");

	fprintf(fout, ".LP%d\n",lpno);
	fprintf(fout, ".12\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, ".L120\n");

	fprintf(fout, ".B1\n");
	fprintf(fout, ".E%s \n",clip(comm_rec.tco_name));
	fprintf(fout, ".EAS AT : %s\n",SystemTime());
	if (all_br[0] != 'Y')
		fprintf(fout, ".EBranch: %s \n",clip(comm_rec.tes_name));
	else
		fprintf(fout, ".B1\n");
	fprintf(fout, ".ESALES MARGIN PERCENT EXCEPTION\n");
	if (MIN_MARG)
		fprintf(fout, ".CITEMS %s    Minimum Margin\n",head_desc);
	else
		fprintf(fout, ".CITEMS %s    %4.2f pc PROFIT\n",head_desc,mgn_pc);

	switch(group_by[0])
	{
	case	'I':
		fprintf(fout, "START DATE : %-10.10s",DateToString(start_date));
		fprintf(fout, "                                           ");
		fprintf(fout, "END DATE : %-10.10s\n",DateToString(end_date));
		fprintf(fout, "========================================");
		fprintf(fout, "========================================");
		if (MIN_MARG)
			fprintf(fout, "===============\n");
		else
			fprintf(fout, "======\n");

		fprintf(fout, ".R========================================");
		fprintf(fout, "========================================");
		if (MIN_MARG)
			fprintf(fout, "===============\n");
		else
			fprintf(fout, "======\n");

		fprintf(fout, "|BR");
		fprintf(fout, "|DP");
		fprintf(fout, "|   DATE   ");
		fprintf(fout, "| INVOICE  ");
		fprintf(fout, "| CUSTOMER ");
		fprintf(fout, "|   UNITS  ");
		fprintf(fout, "|  REVENUE   ");
		fprintf(fout, "| COST/SALE  ");
		fprintf(fout, "| MGN PC |");
		if (MIN_MARG)
			fprintf(fout, " CAT PC |\n");
		else
			fprintf(fout, "\n");
		break;

	case	'C':
		fprintf(fout, "START DATE : %-10.10s",DateToString(start_date));
		fprintf(fout, "                                                   ");
		fprintf(fout, "END DATE : %-10.10s\n",DateToString(end_date));
		fprintf(fout, "========================================");
		fprintf(fout, "========================================");
		if (MIN_MARG)
			fprintf(fout, "=======================\n");
		else
			fprintf(fout, "==============\n");

		fprintf(fout, ".R========================================");
		fprintf(fout, "========================================");
		if (MIN_MARG)
			fprintf(fout, "=======================\n");
		else
			fprintf(fout, "==============\n");

		fprintf(fout, "|BR");
		fprintf(fout, "|DP");
		fprintf(fout, "|   DATE   ");
		fprintf(fout, "| INVOICE  ");
		fprintf(fout, "| ITEM NO          ");
		fprintf(fout, "|  UNITS   ");
		fprintf(fout, "|  REVENUE   ");
		fprintf(fout, "| COST/SALE  ");
		fprintf(fout, "| MGN PC |");
		if (MIN_MARG)
			fprintf(fout, " CAT PC |\n");
		else
			fprintf(fout, "\n");
		break;
	}
	print_line1();
	fflush(fout);
}

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
	open_rec("coln", coln_list, coln_no_fields, "coln_id_no");
	open_rec("cohr", cohr_list, cohr_no_fields, "cohr_id_no2");
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec("excf", excf_list, excf_no_fields, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("excf");
	abc_fclose("inmr");
	abc_fclose("coln");
	abc_fclose("cohr");
	abc_fclose("cumr");
	abc_dbclose("data");
}

void
proc_cohr (
 void)
{
	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,"  ");
	strcpy(cohr_rec.hr_type," ");
	strcpy(cohr_rec.hr_inv_no,"        ");
	cc = find_rec("cohr",&cohr_rec,GTEQ,"r");
	while (!cc && !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no))
	{
		cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",cohr_rec.hr_hhcu_hash);
		if (cc)
		{
			cc = find_rec("cohr",&cohr_rec,NEXT,"r");
			continue;
		}

		if (CUST && !valid_cust())
		{
			cc = find_rec("cohr",&cohr_rec,NEXT,"r");
			continue;
		}

		if (cohr_rec.hr_type[0] == 'P')
		{
			cc = find_rec("cohr",&cohr_rec,NEXT,"r");
			continue;
		}

		if (all_br[0] == 'N')
		{
			if (strcmp(comm_rec.tes_no,cohr_rec.hr_br_no))
			{
				cc = find_rec("cohr",&cohr_rec,NEXT,"r");
				continue;
			}
		}
	
		if (stat_flag[0] == 'A')
		{
			if (cohr_rec.hr_stat_flag[0] != 'D' && 
		    	    cohr_rec.hr_stat_flag[0] != '1' && 
	            	    cohr_rec.hr_stat_flag[0] != '9')
			{
				cc = find_rec("cohr",&cohr_rec,NEXT,"r");
				continue;
			}
		}
		else
		{
			if (cohr_rec.hr_stat_flag[0] == 'D' || 
		    	    cohr_rec.hr_stat_flag[0] == '1' || 
	    	    	    cohr_rec.hr_stat_flag[0] == '9')
			{
	
				cc = find_rec("cohr",&cohr_rec,NEXT,"r");
				continue;
			}
		}
		if (start_date <= cohr_rec.hr_date_raised && end_date >= cohr_rec.hr_date_raised)
			get_coln(cohr_rec.hr_hhco_hash);

		cc = find_rec("cohr",&cohr_rec,NEXT,"r");
	}
	if (found_data)
		proc_sorted();
}

int
valid_cust (
 void)
{
	if (strncmp (lower, cumr_rec.cm_dbt_acronym, 9) <= 0 &&
		strncmp (cumr_rec.cm_dbt_acronym, upper, 9) <= 0)
		return(1);

	return(0);
}

int
valid_item (
 void)
{
	if (strcmp(lower,inmr_rec.mr_item_no) <= 0 && strcmp(inmr_rec.mr_item_no,upper) <= 0)
		return(1);

	/*----------------------------------------------------------
	| Ignore class Z (desciption items and) N(on stock items.) |
	----------------------------------------------------------*/
	if (inmr_rec.mr_class[0] == 'Z' || inmr_rec.mr_class[0] == 'N')
		return(1);
	return(0);
}

void
get_coln (
 long hhco_hash)
{
	coln_rec.ln_hhco_hash = hhco_hash;
	coln_rec.ln_line_no = 0;
	cc = find_rec("coln",&coln_rec,GTEQ,"r");
	while (!cc && coln_rec.ln_hhco_hash == hhco_hash)
	{
		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",coln_rec.ln_hhbr_hash);
		if (cc)
		{
			cc = find_rec("coln",&coln_rec,NEXT,"r");
			continue;
		}

		if (ITEM && !valid_item())
		{
			cc = find_rec("coln",&coln_rec,NEXT,"r");
			continue;
		}

		calc_act_margin();

		if (MIN_MARG && !get_margin())
		{
			cc = find_rec("coln",&coln_rec,NEXT,"r");
			continue;
		}

		if ((!strcmp(type_flag,"B") && act_pc < mgn_pc) ||
			(!strcmp(type_flag,"A") && act_pc > mgn_pc) ||
			  mgn_pc == 0.0)
		{
			store_data();
		}
		cc = find_rec("coln",&coln_rec,NEXT,"r");
	}
}

int
get_margin (
 void)
{
	if (strcmp(curr_cat.number, inmr_rec.mr_category))
	{
		strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
		sprintf(excf_rec.cf_categ_no, "%-11.11s", inmr_rec.mr_category);
		cc = find_rec("excf", &excf_rec, COMPARISON, "r");
		if (cc)
			return(FALSE);
		else
		{
			sprintf(curr_cat.number, "%-11.11s", excf_rec.cf_categ_no);
			curr_cat.mgn_pc = excf_rec.cf_min_marg;
		}
	}

	mgn_pc = curr_cat.mgn_pc;

	return(TRUE);
}

void
store_data (
 void)
{
	if (!found_data)
	{
		fsort = sort_open("sk_marg");
		found_data = TRUE;
	}

	sprintf(data_str,"%-16.16s %-2.2s %-2.2s %-10.10s %-8.8s %-16.16s %10.2f %10.2f %10.2f %10.2f %06ld %06ld %10.2f %d\n",
	(ITEM) ? inmr_rec.mr_item_no : cumr_rec.cm_dbt_no,
	cohr_rec.hr_br_no,
	cohr_rec.hr_dp_no,
	DateToString(cohr_rec.hr_date_raised),
	cohr_rec.hr_inv_no,
	(ITEM) ? cumr_rec.cm_dbt_no : inmr_rec.mr_item_no,
	coln_rec.ln_q_order,
	DOLLARS(sale),
	DOLLARS(cost),
	act_pc,
	inmr_rec.mr_hhbr_hash,
	cumr_rec.cm_hhcu_hash,
	(MIN_MARG) ? mgn_pc : 0.00,
	zero_cost);

	sort_save(fsort,data_str);
}

void
calc_act_margin (
 void)
{
	cost = out_cost(coln_rec.ln_cost_price, inmr_rec.mr_outer_size);
	cost *= (double) coln_rec.ln_q_order;
	sale = out_cost(coln_rec.ln_sale_price, inmr_rec.mr_outer_size);
	sale *= (double) coln_rec.ln_q_order;
	sale -= coln_rec.ln_amt_disc;

	zero_cost = FALSE;
	if (cost == 0.00)
	{
		act_pc = 100.0;
		zero_cost = TRUE;
	}
	else
		if (cost == sale)
			act_pc = 0.0;
		else
			if (cost != sale && sale != 0.00)
				act_pc = (float) (((sale - cost) * 100.0) / sale);
			else 
				act_pc = -100.0;

	if (cohr_rec.hr_type[0] == 'C')
	{
		cost *= -1.00;
		sale *= -1.00;
	}
	gsale += sale;
	gcost += cost;
}

void
proc_sorted (
 void)
{
	int	first_time = TRUE;
	long	prev_hash;
	long	curr_hash;

	fsort = sort_sort(fsort,"sk_marg");

	sptr = sort_read(fsort);
	prev_hash = 0L;
	
	while (sptr != (char *)0)
	{
		if (ITEM)
			curr_hash = h_val(sptr + 104);
		else
			curr_hash = h_val(sptr + 111);

		if (prev_hash != curr_hash)
		{
			if (!first_time)
			{
				print_bline();
			}

			print_header(curr_hash);
			prev_hash = curr_hash;
			first_time = FALSE;
		}

		fprintf(fout, "|%-2.2s",sptr + 17);     /* br */
		fprintf(fout, "|%-2.2s",sptr + 20);     /* dp */ 
		fprintf(fout, "|%-10.10s",sptr + 23);   /* date_raised */ 
		fprintf(fout, "| %-8.8s ",sptr + 34);   /* inv no */
		if (ITEM)
			fprintf(fout, "|  %-6.6s  ",sptr + 43); /* 43 */
		else
			fprintf(fout, "| %-16.16s ",sptr + 43); /* 43 */
		fprintf(fout, "|%10.2f",f_val(sptr + 60)); /* 60 */
		fprintf(fout, "| %10.2f ",f_val(sptr + 71)); /* 71 */
		fprintf(fout, "| %10.2f ",f_val(sptr + 82)); /* 82 */
		if (atoi(sptr + 127) == TRUE)
			fprintf(fout, "|%8.8s|", "   *****");
		else
			fprintf(fout, "|%8.2f|",f_val(sptr + 93)); /* 93 */

		if (MIN_MARG)
			fprintf(fout, "%8.2f|\n",f_val(sptr + 118)); /* 118 */
		else
			fprintf(fout, "\n");

		totsale += f_val(sptr + 71); /* 71 */
		totcost += f_val(sptr + 82); /* 82 */

		sptr = sort_read(fsort);
	}
	print_totals("G");
	print_totals("C");
	sort_delete(fsort,"sk_marg");
}

void
print_header (
 long reqd_hash)
{
	if (ITEM)
	{
		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",reqd_hash);
		dsp_process(" Item No : ", inmr_rec.mr_item_no);

		fprintf(fout,"| Item no : %-16.16s - ",inmr_rec.mr_item_no);  
		fprintf(fout,"%-40.40s",inmr_rec.mr_description);
		if (MIN_MARG)
			fprintf(fout,"                       |\n");
		else
			fprintf(fout,"              |\n");
	}
	else
	{
		cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",reqd_hash);
		dsp_process(" Customer : ", cumr_rec.cm_dbt_acronym);

		fprintf(fout,"|Customer No: %-6.6s - ",cumr_rec.cm_dbt_no);  
		fprintf(fout,"%-40.40s",cumr_rec.cm_dbt_name);
		if (MIN_MARG)
			fprintf(fout,"                                       |\n");
		else
			fprintf(fout,"                              |\n");
	}
	fflush(fout);
}

void
print_bline (
 void)
{
	fprintf(fout, "|  ");
	fprintf(fout, "|  ");
	fprintf(fout, "|          ");
	fprintf(fout, "|          ");
	if (ITEM)
		fprintf(fout, "|          ");
	else
		fprintf(fout, "|                  ");
	fprintf(fout, "|          ");
	fprintf(fout, "|            ");
	fprintf(fout, "|            ");
	fprintf(fout, "|        |");
	if (MIN_MARG)
		fprintf(fout, "        |\n");
	else
		fprintf(fout, "\n");
}

void
print_line (
 void)
{
	fprintf(fout, "|--");
	fprintf(fout, "|--");
	fprintf(fout, "|----------");
	fprintf(fout, "|----------");
	if (ITEM)
		fprintf(fout, "|----------");
	else
		fprintf(fout, "|------------------");
	fprintf(fout, "|----------");
	fprintf(fout, "|------------");
	fprintf(fout, "|------------");
	fprintf(fout, "|--------|");
	if (MIN_MARG)
		fprintf(fout, "--------|\n");
	else
		fprintf(fout, "\n");
}

void
print_line1 (
 void)
{
	switch (group_by[0])
	{
	case	'I':
		fprintf(fout, "|------------------------------");
		fprintf(fout, "-----------------------------");
		if (MIN_MARG)
			fprintf(fout, "----------------------------------|\n");
		else
			fprintf(fout, "-------------------------|\n");
		break;

	case	'C':
		fprintf(fout, "|------------------------------");
		fprintf(fout, "-----------------------------");
		if (MIN_MARG)
			fprintf(fout, "------------------------------------------|\n");
		else
			fprintf(fout, "---------------------------------|\n");
		break;
	}
}

void
print_totals (
 char *type)
{
	print_line();

	switch (type[0])
	{
	case	'G':
		if (totsale != totcost && totcost != 0.00)
			totmgn = (totsale - totcost) * 100.00 / totsale;

		fprintf(fout, "|*****  TOTALS *****");
		fprintf(fout, "        |");

		if (ITEM)
			fprintf(fout, "          |");
		else
			fprintf(fout, "                  |");

		fprintf(fout, "          |");
		fprintf(fout, "%12.2f",totsale);
		fprintf(fout, "|%12.2f",totcost);
		fprintf(fout, "|%8.2f|",totmgn);
		if (MIN_MARG)
			fprintf(fout, "        |\n");
		else
			fprintf(fout, "\n");
		fflush(fout);
		totsale = 0.00;
		totcost = 0.00;
		break;

	case	'C':

		if (gsale != gcost && gcost != 0.00)
			gmgn = (gsale - gcost) * 100.00 / gsale;

		fprintf(fout, "|*****  GRAND TOTALS *****");
		fprintf(fout, "  |");

		if (ITEM)
			fprintf(fout, "          |");
		else
			fprintf(fout, "                  |");

		fprintf(fout, "          |");
		fprintf(fout, "%12.2f",DOLLARS(gsale));
		fprintf(fout, "|%12.2f",DOLLARS(gcost));
		fprintf(fout, "|%8.2f|",gmgn);
		if (MIN_MARG)
			fprintf(fout, "        |\n");
		else
			fprintf(fout, "\n");
		fflush(fout);
		break;
	}
}

float	
f_val (
 char *str)
{
	char	val[11];
	
	sprintf(val,"%-10.10s",str);
	return ((float) (atof(val)));
}

long	
h_val (
 char *str)
{
	char	val[7];
	
	sprintf(val,"%-6.6s",str);
	return(atol(val));
}
