/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_roy_prt.c   )                                 |
|  Program Desc  : ( Print Royalty Report.                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, ryhs, ryhr, ryln,                     |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 10/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/09/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (11/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : This version incorporates 2 royalty reports.       |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (30/09/96)    : Updated to convert to version 9 pricing.           |
|  (11/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: so_roy_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_roy_prt/so_roy_prt.c,v 5.3 2002/12/01 04:48:19 scott Exp $";

#define	BY_AUTHOR	(sort_by[0] == 'A')

#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_so_mess.h>

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
		{"comm_dbt_date"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tdbt_date;
	} comm_rec;

	/*========================
	| Inventory master File. |
	========================*/
	struct dbview inmr_list[] ={
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_outer_size"},
		{"inmr_sellgrp"},
		{"inmr_disc_pc"},
		{"inmr_tax_pc"},
		{"inmr_gst_pc"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 10;

	struct {
		char	item_no[17];
		long 	hhbr_hash;
		char	description[41];
		char	category[12];
		float	outer_size;
		char	sellgrp[7];
		float 	disc_pc;
		float 	tax_pc;
		float 	gst_pc;
		char 	stat_flag[2];
	} inmr_rec;

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

	/*=============================
	| Royalty Header File Record. |
	=============================*/
	struct dbview ryln_list[] ={
		{"ryln_hhry_hash"},
		{"ryln_hhsu_hash"},
		{"ryln_roy_pc"},
		{"ryln_e_hhgl_hash"},
		{"ryln_t_rate"},
		{"ryln_t_hhgl_hash"},
		{"ryln_stat_flag"}
	};

	int ryln_no_fields = 7;

	struct {
		long	ln_hhry_hash;
		long	ln_hhsu_hash;
		float	ln_roy_pc;
		long	ln_e_hhgl_hash;
		float	ln_t_rate;
		long	ln_t_hhgl_hash;
		char	ln_stat_flag[2];
	} ryln_rec;

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

	/*========================
	| Creditors Master File. |
	========================*/
	struct dbview sumr_list[] ={
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
	};

	int sumr_no_fields = 6;

	struct {
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_crd_name[41];
		char	sm_acronym[10];
	} sumr_rec;

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

	int	lpno = 1;

	FILE	*ftmp;
	FILE	*fin;
	FILE	*fsort;

	char	sort_str[168];
	char	data_str[168];
	char	sort_by[2];

	char	prev_div[2];
	char	curr_div[2];
	char	prev_pub[10];
	char	curr_pub[10];
	char	prev_title[41];
	char	curr_title[41];
	char	curr_isbn[17];
	char	prev_isbn[17];
	char	curr_basis[2];
	char	prev_basis[2];
	char	prev_stype[4];
	char	curr_stype[4];

	long	prev_hash;
	long	curr_hash;

	float	curr_pc;
	float	prev_pc;

	int		retail;
	int		first_time;
	int		printed;
	int		line_no = 0;
	int		PriceLevel = 0;
	int		sk_dbprinum;
	int		sk_dbqtynum;
	float	gst_div     = 0.00;
	float	gst_include = 0.00;

	float	m_qty[5];
	float	y_qty[5];

	double	curr_ret;
	double	prev_ret;
	double	m_nett[5];
	double	m_roy[5];
	double	y_nett[5];
	double	y_roy[5];

	char	Curr_code[4];


/*=======================
| Function Declarations |
=======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void proc_ryhs (void);
void store_a_data (void);
void store_data (void);
void init_array (void);
void proc_sorted (void);
void set_break (char *sort_str);
int  check_break (void);
void proc_data (int print_type);
void sum_nett (char *data_line, int add);
void print_line (void);
void print_total (char *tot_type);
float f_val (char *str);
double d_val (char *str);
double out_gst (double total_amt);
int  check_page (void);
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

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (Curr_code, "%-3.3s", get_env("CURR_CODE"));

	if (argc != 4)
	{
		/*--------------------------------------
		| Usage : %s <lpno> <RETAIL> <sort_by> |
		--------------------------------------*/
		print_at(0,0, mlSoMess793,argv[0]);
		return (EXIT_FAILURE);
	}
	
	lpno = atoi(argv[1]);
	retail = atoi(argv[2]);
	sprintf(sort_by,"%-1.1s",argv[3]);

	if (BY_AUTHOR)
	{
		gst_include = (float) (atof(get_env("GST_INCLUSIVE")));
		if (gst_include != 0.00)
			gst_div = (float) ((100.00 + gst_include) / gst_include);
		else
			gst_div = 0.00;
	}

	OpenDB();


	sprintf(err_str,"Processing : Royalty Report By %s",(BY_AUTHOR) ? "Author" : "Title/Publisher");
	dsp_screen(err_str, comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	head_output();

	proc_ryhs();

	proc_sorted();

    fprintf(ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose(ftmp);

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
	fsort = sort_open("so_roy");

	/*------------------
	| Open format file |
	------------------*/
	if ((fin = pr_open("so_roy_prt.p")) == NULL)
		sys_err("Error in opening so_roy_prt.p during (FOPEN)",errno,PNAME);

	/*=======================
	| Start output to file. |
	=======================*/
	fprintf(ftmp, ".START%s<%s>\n", DateToString (comm_rec.tdbt_date), PNAME);
	fprintf(ftmp, ".LP%d\n",lpno);
	fprintf(ftmp, ".14\n");
	fprintf(ftmp, ".L158\n");
	fprintf(ftmp, ".B1\n");

	sprintf(err_str, "%s",(BY_AUTHOR) ? "BY AUTHOR" : "BY TITLE");
	fprintf(ftmp, ".EROYALTY REPORT %s\n",err_str);
	fprintf(ftmp, ".E%s \n",clip(comm_rec.tco_name));
	fprintf(ftmp, ".EBranch: %s \n",clip(comm_rec.test_name));
	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".EAS AT : %s\n",SystemTime());
	pr_format(fin,ftmp,"HEADER",0,0);
	pr_format(fin,ftmp,(BY_AUTHOR) ? "HEADER1_A" : "HEADER1",0,0);
	pr_format(fin,ftmp,(BY_AUTHOR) ? "HEADER2_A" : "HEADER2",0,0);
	pr_format(fin,ftmp,(BY_AUTHOR) ? "HEADER3_A" : "HEADER3",0,0);
	pr_format(fin,ftmp,(BY_AUTHOR) ? "LINE2" : "LINE1",0,0);
	pr_format(fin,ftmp,"RULEOFF",0,0);
	fprintf(ftmp, ".PI12\n");
	fflush(ftmp);
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

	open_rec("ryhs", ryhs_list, RYHS_NO_FIELDS, "ryhs_id_no");
	open_rec("ryhr", ryhr_list, ryhr_no_fields, "ryhr_hhry_hash");
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec("inpr", inpr_list, inpr_no_fields, "inpr_id_no");
	if (BY_AUTHOR)
	{
		open_rec("sumr", sumr_list, sumr_no_fields, "sumr_hhsu_hash");
		open_rec("ryln", ryln_list, ryln_no_fields, "ryln_hhry_hash");
	}
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("ryhs");
	abc_fclose("ryhr");
	abc_fclose("inmr");
	abc_fclose("inpr");
	if (BY_AUTHOR)
	{
		abc_fclose("sumr");
		abc_fclose("ryln");
	}
	abc_dbclose("data");
}

/*========================================================
| Read data into sort file,then process the information |
========================================================*/
void
proc_ryhs (
 void)
{
	strcpy(ryhs_rec.co_no,comm_rec.tco_no);
	ryhs_rec.hhry_hash = 0L;
	strcpy(ryhs_rec.publish, "    ");
	ryhs_rec.roy_pc = 0.00;
	strcpy(ryhs_rec.roy_basis, " ");
	
	cc = find_rec("ryhs",&ryhs_rec,GTEQ,"r");

	while (!cc && !strcmp(ryhs_rec.co_no,comm_rec.tco_no))
	{
		if (BY_AUTHOR)
			store_a_data();
		else
			store_data();

		cc = find_rec("ryhs",&ryhs_rec,NEXT,"r");
	}
}

void
store_a_data (
 void)
{
	double	retail_price = 0.00;
	double	mtd_roy = 0.00;
	double	ytd_roy = 0.00;
	float	roy_pc = 0.00;

	dsp_process("Reading Publish Code : %s",ryhs_rec.publish);
	
	cc = find_hash("ryhr",&ryhr_rec,COMPARISON,"r",ryhs_rec.hhry_hash);
	if (cc)
		return;

	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",ryhr_rec.hr_hhbr_hash);
	if (cc)
		return;

	WsFindInpr (retail - 1);

	if (gst_div != 0.00)
		retail_price = out_gst(inpr2_rec.base);
	else
		retail_price = inpr2_rec.base;

	cc = find_hash("ryln",&ryln_rec,GTEQ,"r",ryhr_rec.hr_hhry_hash);
	while (!cc && ryln_rec.ln_hhry_hash == ryhr_rec.hr_hhry_hash)
	{
		cc = find_hash("sumr",&sumr_rec,COMPARISON,"r",ryln_rec.ln_hhsu_hash);
		if (!cc)
		{
			roy_pc  = ryhs_rec.roy_pc * ryln_rec.ln_roy_pc;
			roy_pc  /= 100.00;

			ryln_rec.ln_roy_pc /= 100;

			mtd_roy = (double) ryln_rec.ln_roy_pc * ryhs_rec.mtd_roy;
			mtd_roy = no_dec(mtd_roy);

			ytd_roy =  (double) ryln_rec.ln_roy_pc * ryhs_rec.ytd_roy;
			ytd_roy = no_dec(ytd_roy);

			sprintf(data_str,"%-9.9s %-40.40s %12.2f %-3.3s %-1.1s %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %6ld\n",
			sumr_rec.sm_acronym,
			inmr_rec.description,
			DOLLARS(retail_price),
			ryhr_rec.hr_code,
			ryhs_rec.roy_basis,
			roy_pc,
			ryhs_rec.mtd_qty,
			DOLLARS(ryhs_rec.mtd_nett),
			DOLLARS(mtd_roy),
			ryhs_rec.ytd_qty,
			DOLLARS(ryhs_rec.ytd_nett),
			DOLLARS(ytd_roy),
			inmr_rec.hhbr_hash);

			sort_save(fsort,data_str);
		}
		cc = find_hash("ryln",&ryln_rec,NEXT,"r",ryhr_rec.hr_hhry_hash);
	}
}

void
store_data (
 void)
{
	dsp_process("Reading Publish Code : %s",ryhs_rec.publish);
	
	cc = find_hash("ryhr",&ryhr_rec,COMPARISON,"r",ryhs_rec.hhry_hash);
	if (cc)
		return;

	cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",ryhr_rec.hr_hhbr_hash);

	if (cc)
	{
		sprintf(inmr_rec.description,"%-40.40s"," ");
		sprintf(inmr_rec.item_no,"%-16.16s"," ");
		inmr_rec.hhbr_hash = 0L;
	}

	sprintf(data_str,"%-1.1s %-3.3s %-40.40s %-16.16s %-3.3s %-1.1s %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %12.2f %6ld\n",
		ryhs_rec.publish + 3,
		ryhs_rec.publish,
		inmr_rec.description,
		inmr_rec.item_no,
		ryhr_rec.hr_code,
		ryhs_rec.roy_basis,
		ryhs_rec.roy_pc,
		ryhs_rec.mtd_qty,
		DOLLARS(ryhs_rec.mtd_nett),
		DOLLARS(ryhs_rec.mtd_roy),
		ryhs_rec.ytd_qty,
		DOLLARS(ryhs_rec.ytd_nett),
		DOLLARS(ryhs_rec.ytd_roy),
		inmr_rec.hhbr_hash);

	sort_save(fsort,data_str);
}

void
init_array (
 void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		m_qty[j] = 0.00;
		y_qty[j] = 0.00;
		m_nett[j] = 0.00;
		y_nett[j] = 0.00;
		m_roy[j] = 0.00;
		y_roy[j] = 0.00;
	}
}

void
proc_sorted (
 void)
{
	char	*sptr;
	int	print_type;

	init_array();
	printed = FALSE;
	first_time = TRUE;

	fsort = sort_sort(fsort,"so_roy");
	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(sort_str,"%-167.167s",sptr);

		set_break(sort_str);

		if (BY_AUTHOR)
			dsp_process("Author :",curr_pub);
		else
			dsp_process("Publisher :",curr_pub);

		if (first_time)
		{
			if (BY_AUTHOR)
				prev_ret = curr_ret;
			else
			{
				strcpy(prev_div,curr_div);
				strcpy(prev_isbn,curr_isbn);
			}
			strcpy(prev_pub,curr_pub);
			strcpy(prev_title,curr_title);
			strcpy(prev_stype,curr_stype);
			strcpy(prev_basis,curr_basis);
			prev_hash = curr_hash;
			prev_pc = curr_pc;
		}

		print_type = check_break();

		proc_data(print_type);
		first_time = 0;

		if (print_type == 0)
			sum_nett(sptr + 83,1);
		else
			sum_nett(sptr + 83,0);

		if (BY_AUTHOR)
			prev_ret = curr_ret;
		else
		{
			strcpy(prev_div,curr_div);
			strcpy(prev_isbn,curr_isbn);
		}
		strcpy(prev_pub,curr_pub);
		strcpy(prev_title,curr_title);
		strcpy(prev_stype,curr_stype);
		strcpy(prev_basis,curr_basis);
		prev_hash = curr_hash;
		prev_pc = curr_pc;

		sptr = sort_read(fsort);
	}
	if (printed)
	{
		print_line();
		print_total("T");
		if (BY_AUTHOR)
			print_total("A");
		else
		{
			print_total("P");
			print_total("D");
		}
		print_total("C");
	}
	sort_delete(fsort,"so_roy");
}

void
set_break (
 char *sort_str)
{
	if (BY_AUTHOR)
	{
		sprintf(curr_pub,"%-9.9s",sort_str);
		sprintf(curr_title,"%-40.40s",sort_str + 10);
		curr_ret = f_val(sort_str + 51);
	}
	else
	{
		sprintf(curr_div,"%-1.1s",sort_str);
		sprintf(curr_pub,"%-3.3s",sort_str + 2);
		sprintf(curr_title,"%-40.40s",sort_str + 6);
		sprintf(curr_isbn,"%-16.16s",sort_str + 47);
	}
	sprintf(curr_stype,"%-3.3s",sort_str + 64);
	sprintf(curr_basis,"%-1.1s",sort_str + 68);
	curr_pc = f_val(sort_str + 70);
	curr_hash = atol(sort_str + 161);
}

int
check_break (
 void)
{
	if (!BY_AUTHOR && strcmp(curr_div,prev_div))
		return(1);

	if (strcmp(curr_pub,prev_pub))
		return(2);

	if (strcmp(curr_title,prev_title))
		return(3);

	if (!strcmp(curr_title,prev_title) && strcmp(curr_stype,prev_stype))
	{
		line_no++;
		return(4);
	}

	if (strcmp(curr_stype,prev_stype))
		return(4);
	return(0);
}

void
proc_data (
 int print_type)
{
	if (print_type == 1 || print_type == 2 || print_type == 3)
	{
		print_line();

		switch (print_type)
		{
		case	1:
			print_total("T");
			print_total("P");
			print_total("D");
			pr_format(fin,ftmp,"LINE1",0,0);
			break;

		case	2:
			print_total("T");
			if (BY_AUTHOR)
				print_total("A");
			else
				print_total("P");
			break;

		case	3:
			print_total("T");
			if (BY_AUTHOR)
				pr_format(fin,ftmp,"BL_LINE2",0,0);
			else
				pr_format(fin,ftmp,"BL_LINE1",0,0);
			break;
		}
	}

	if (print_type == 4)
		print_line();
}

void
sum_nett (
 char *data_line, 
 int add)
{
	char	*sptr = data_line;

	if (add)
	{
		m_qty[0]   	+= f_val(sptr);
		m_nett[0] 	+= d_val(sptr + 13);
		m_roy[0] 	+= d_val(sptr + 26);
		y_qty[0]   	+= f_val(sptr + 39);
		y_nett[0] 	+= d_val(sptr + 52);
		y_roy[0] 	+= d_val(sptr + 65);
	}
	else
	{
		m_qty[0]   	= f_val(sptr);
		m_nett[0] 	= d_val(sptr + 13);
		m_roy[0] 	= d_val(sptr + 26);
		y_qty[0]   	= f_val(sptr + 39);
		y_nett[0] 	= d_val(sptr + 52);
		y_roy[0] 	= d_val(sptr + 65);
	}
}

void
print_line (
 void)
{
	register	int	i = 0;
	
	if (BY_AUTHOR)
	{
		pr_format(fin,ftmp,"ROY_AUTHOR",1,prev_pub);
		pr_format(fin,ftmp,"ROY_AUTHOR",2,prev_title);
		pr_format(fin,ftmp,"ROY_AUTHOR",3,prev_ret);
		pr_format(fin,ftmp,"ROY_AUTHOR",4,prev_stype);
		pr_format(fin,ftmp,"ROY_AUTHOR",5,prev_basis);
		pr_format(fin,ftmp,"ROY_AUTHOR",6,prev_pc);
		pr_format(fin,ftmp,"ROY_AUTHOR",7,m_qty[i]);
		pr_format(fin,ftmp,"ROY_AUTHOR",8,m_nett[i]);
		pr_format(fin,ftmp,"ROY_AUTHOR",9,m_roy[i]);
		pr_format(fin,ftmp,"ROY_AUTHOR",10,y_qty[i]);
		pr_format(fin,ftmp,"ROY_AUTHOR",11,y_nett[i]);
		pr_format(fin,ftmp,"ROY_AUTHOR",12,y_roy[i]);
	}
	else
	{
		pr_format(fin,ftmp,"ROY_LINE",1,prev_div);
		pr_format(fin,ftmp,"ROY_LINE",2,prev_pub);
		pr_format(fin,ftmp,"ROY_LINE",3,prev_title);
		pr_format(fin,ftmp,"ROY_LINE",4,prev_isbn);
		pr_format(fin,ftmp,"ROY_LINE",5,prev_stype);
		pr_format(fin,ftmp,"ROY_LINE",6,prev_basis);
		pr_format(fin,ftmp,"ROY_LINE",7,prev_pc);
		pr_format(fin,ftmp,"ROY_LINE",8,m_qty[i]);
		pr_format(fin,ftmp,"ROY_LINE",9,m_nett[i]);
		pr_format(fin,ftmp,"ROY_LINE",10,m_roy[i]);
		pr_format(fin,ftmp,"ROY_LINE",11,y_qty[i]);
		pr_format(fin,ftmp,"ROY_LINE",12,y_nett[i]);
		pr_format(fin,ftmp,"ROY_LINE",13,y_roy[i]);
	}

	m_qty	[i+1] += m_qty	[i];
	m_nett	[i+1] += m_nett	[i];
	m_roy	[i+1] += m_roy	[i];
	y_qty	[i+1] += y_qty	[i];
	y_nett	[i+1] += y_nett	[i];
	y_roy	[i+1] += y_roy	[i];

	m_qty	[i] = 0.00;
	m_nett	[i] = 0.00;
	m_roy	[i] = 0.00;
	y_qty	[i] = 0.00;
	y_nett	[i] = 0.00;
	y_roy	[i] = 0.00;
}

void
print_total (
 char *tot_type)
{
	int	j = 0;
	int	draw_line = FALSE;
	int	print_ok = TRUE;

	switch (tot_type[0])
	{
	case	'T':
		j = 1;
		sprintf(err_str,"%-s","Total For Title    ");
		if (line_no == 0)
			print_ok = FALSE;
		else
			line_no = 0;
		break;

	case	'A':
		j = 2;
		sprintf(err_str,"%-s","** Total For Author   ");
		draw_line = TRUE;
		break;

	case	'P':
		j = 2;
		sprintf(err_str,"%-s","** Total For Publisher");
		break;

	case	'D':
		j = 3;
		sprintf(err_str,"%-s","*** Total For Division ");
		break;

	case	'C':
		j = (BY_AUTHOR) ? 3 : 4;
		sprintf(err_str,"%-s","**** Total For Company  ");
		break;
	}

	if (print_ok)
	{
		if (BY_AUTHOR)
		{
			pr_format(fin,ftmp,"LINE2",0,0);
			pr_format(fin,ftmp,"A_TOTAL",1,err_str);
			pr_format(fin,ftmp,"A_TOTAL",2,m_roy[j]);
			pr_format(fin,ftmp,"A_TOTAL",3,y_roy[j]);
			if (draw_line)
			{
				pr_format(fin,ftmp,"BL_LINE2",0,0);
			}
		}
		else
		{
			pr_format(fin,ftmp,"LINE1",0,0);
			pr_format(fin,ftmp,"TOTAL",1,err_str);
			pr_format(fin,ftmp,"TOTAL",2,m_qty[j]);
			pr_format(fin,ftmp,"TOTAL",3,m_nett[j]);
			pr_format(fin,ftmp,"TOTAL",4,m_roy[j]);
			pr_format(fin,ftmp,"TOTAL",5,y_qty[j]);
			pr_format(fin,ftmp,"TOTAL",6,y_nett[j]);
			pr_format(fin,ftmp,"TOTAL",7,y_roy[j]);
		}
	}

	if (tot_type[0] != 'C')
	{
		m_qty 	[j+1] += m_qty[j];
		m_nett	[j+1] += m_nett[j];
		m_roy	[j+1] += m_roy[j];
		y_qty	[j+1] += y_qty[j];
		y_nett	[j+1] += y_nett[j];
		y_roy	[j+1] += y_roy[j];
	}

	m_qty	[j] = 0.00;
	m_nett	[j] = 0.00;
	m_roy	[j] = 0.00;
	y_qty	[j] = 0.00;
	y_nett	[j] = 0.00;
	y_roy	[j] = 0.00;
}

float	
f_val (
 char *str)
{
	char	val[13];
	
	sprintf(val,"%-12.12s",str);
	return ((float) (atof(val)));
}

double	
d_val (
 char *str)
{
	char	val[13];
	
	sprintf(val,"%-12.12s",str);
	return(atof(val));
}

double	
out_gst (
 double total_amt)
{
	double	gst_amount = 0.00;
	
	if (total_amt == 0.00)
		return(0.00);

	gst_amount = no_dec(total_amt / gst_div);
	total_amt -= no_dec(gst_amount);

	return(total_amt);
}

int
check_page (
 void)
{
	return(0);
}

/*=========================
| Find pricing structure. |
=========================*/
void
WsFindInpr (
 int price_type)
{
	inpr2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inpr2_rec.price_type = price_type + 1;
	inpr2_rec.hhgu_hash = 0L;
	strcpy (inpr2_rec.curr_code, Curr_code);

	if ( !PriceLevel )
	{
		strcpy (inpr2_rec.br_no, "  ");
		strcpy (inpr2_rec.wh_no, "  ");
	}
	else 
	{
		strcpy (inpr2_rec.br_no, comm_rec.test_no);
		strcpy (inpr2_rec.wh_no, ( PriceLevel == 2 ) ? comm_rec.tcc_no : "  ");
	}
	
	strcpy (inpr2_rec.cust_type, "   ");
	strcpy (inpr2_rec.area_code, "  ");
	cc = find_rec ("inpr", &inpr2_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (inpr2_rec.br_no,comm_rec.test_no);
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
