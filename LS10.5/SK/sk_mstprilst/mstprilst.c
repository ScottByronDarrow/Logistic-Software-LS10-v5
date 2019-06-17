/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_mstprilst.c )                                 |
|  Program Desc  : ( Stock Master Price List.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, inmr, incc, inei, esmr,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 25/01/87         |
|---------------------------------------------------------------------|
|  Date Modified : (25/01/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (17/04/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/05/92)      | Modified  by  : Simon Dubey .    |
|  Date Modified : (24/05/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/11/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/09/97)      | Modified  by  : Roanna Marcelino |
|                :                                                    |
|  Comments      : (17/04/91) - Updated to fix .LP in START           | 
|  (12/05/92)    : CSP/6856   - Printed 15 char of inmr_item_no not 16|
|  (12/05/92)    :              Plus other nec changes for alignment  |
|  (24/05/93)    : DEC-8596. Ruler not defines large enough and the   |
|                :           effect is a core dump on SCO.            |
|  (05/11/93)    : HGP 9501. Remove inmr_price 1 - 5.  Now look up    |
|                : prices on inpr at company level.                   |
|  (11/09/97)    : Modified for Multilingual Conversion.              | 
|                :                                                    |
|                                                                     |
| $Log: mstprilst.c,v $
| Revision 5.3  2002/12/01 04:48:16  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.2  2001/08/09 09:19:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:16  scott
| Update - LS10.5
|
| Revision 4.0  2001/03/09 02:38:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:42  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:31:01  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/10/13 02:42:04  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.6  1999/10/08 05:32:37  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:20:21  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mstprilst.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mstprilst/mstprilst.c,v 5.3 2002/12/01 04:48:16 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_inv_date"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"}
	};

	int comm_no_fields = 11;
	
	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char test_no[3];
		char tcc_no[3];
		long tinv_date;
		char tprice[5][16];
	} comm_rec;

	/*=================================
	| Cost Centre Master File (ccmr). |
	=================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_acronym"},
		{"ccmr_reports_ok"}
	};

	int ccmr_no_fields = 6;

	struct {
		char cm_co_no[3];
		char cm_est_no[3];
		char cm_cc_no[3];
		long cm_hhcc_hash;
		char cm_acronym[10];
		char cm_reports_ok[2];
	} ccmr_rec;

	/*============================================
	| Inventory Establishment/Branch Stock File. |
	============================================*/
	struct dbview inei_list[] ={
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_last_cost"},
	};

	int inei_no_fields = 3;

	struct {
		long	ei_hhbr_hash;
		char	ei_est_no[3];
		double	ei_last_cost;
	} inei_rec;

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
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_on_hand"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 14;

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
		char	mr_serial_item[2];
		char	mr_costing_flag[2];
		char	mr_sale_unit[5];
		float	mr_on_hand;
		char	mr_stat_flag[2];
	} inmr_rec;

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
		{"inpr_price_by"},
		{"inpr_qty_brk1"},
		{"inpr_qty_brk2"},
		{"inpr_qty_brk3"},
		{"inpr_qty_brk4"},
		{"inpr_qty_brk5"},
		{"inpr_qty_brk6"},
		{"inpr_qty_brk7"},
		{"inpr_qty_brk8"},
		{"inpr_qty_brk9"},
		{"inpr_base"},
		{"inpr_price1"},
		{"inpr_price2"},
		{"inpr_price3"},
		{"inpr_price4"},
		{"inpr_price5"},
		{"inpr_price6"},
		{"inpr_price7"},
		{"inpr_price8"},
		{"inpr_price9"}
	};

	int	inpr_no_fields = 28;

	struct tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	br_no[3];
		char	wh_no[3];
		char	curr_code[4];
		char	area_code[3];
		char	cust_type[4];
		long	hhgu_hash;
		char	price_by[2];
		double	qty_brk[9];
		double	base;		/* money */
		double	price[9];	/* money */
	} inpr_rec;

	/*==========================================
	| Inventory Cost Centre Stock (file incc). |
	==========================================*/
	struct dbview incc_list[] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_sort"},
		{"incc_closing_stock"},
		{"incc_stat_flag"}
	};

	int incc_no_fields = 6;

	struct {
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		long	cc_hhwh_hash;
		char	cc_sort[29];
		float	cc_closing_stock;
		char	cc_stat_flag[2];
	} incc_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
	};

	int esmr_no_fields = 4;

	struct {
		char em_co_no[3];
		char em_est_no[3];
		char em_est_name[41];
		char em_short_name[16];
	} esmr_rec;

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
		char	cf_categ_description[41];
		char	cf_stat_flag[2];
	} excf_rec;

	char	*data = "data",
			*comm = "comm",
			*ccmr = "ccmr",
			*esmr = "esmr",
			*excf = "excf",
			*incc = "incc",
			*inei = "inei",
			*inmr = "inmr",
			*inpr = "inpr";

	FILE	*ftmp;

	char	upper[13],
			lower[13];

	int	lpno = 1,
		by_branch,
		mst_price = 0,
		no_branches,
		mult_copys = 0;

	struct {
		char	br_no[3];	/* est_no's as parameter's	*/
		char	br_short[11];	/* est_short from esmr		*/
		long	br_hhcc[99];	/* hhcc for cc's of branch	*/
		int		br_index;	/* index to last hhcc_hash	*/
	} branch[5];

	int	price[5];

	char	*overlay = "=====================================================================================================================================================================";

	char	ruler[300];
	char	Curr_code[4];


/*=======================
| Function Declarations |
=======================*/
void head_output (char *rep_name);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void load_struct (void);
void process_data (void);
double GetPrice (int pType);
int  new_group (char *old_group);
int  last_group (void);
int  valid_group (void);
void print_branch (void);
int  in_branch (void);
void print_categ (int first_time);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	inv_date[11];

	if (argc < 8)
	{
		print_at(0,0,mlSkMess109, argv[0]);
		return (EXIT_FAILURE);
	}

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf(Curr_code, "%-3.3s", get_env("CURR_CODE"));
	
	OpenDB();


	/*-----------------------------
	|  parameters:                |
	|  1:	'B' by branch         |
	|       'W' by warehouse      |
	|  2:	program description   |
	|  3-7:	branches/warehouses   |
	|  8:	printer number        |
	|  9:	low value  (optional) |
	|  10:	high value (optional) |
	|  11:	number of copies      |
	-----------------------------*/
	by_branch = argv[1][0] == 'B';
	sprintf(branch[0].br_no,"%-2.2s",argv[3]);
	sprintf(branch[1].br_no,"%-2.2s",argv[4]);
	sprintf(branch[2].br_no,"%-2.2s",argv[5]);
	sprintf(branch[3].br_no,"%-2.2s",argv[6]);
	sprintf(branch[4].br_no,"%-2.2s",argv[7]);

	lpno = atoi(argv[8]);

	if (argc > 10) 
	{
		sprintf(lower,"%-12.12s",argv[9]);
		sprintf(upper,"%-12.12s",argv[10]);
	}
	else 
	{
		strcpy(lower,"            ");
		strcpy(upper,"~~~~~~~~~~~~");
	}

	if (argc == 12)
	{
		if (strcmp(argv[11]," ") == 0)
			mult_copys = 0;
		else
			mult_copys = atoi(argv[11]);
	}

	if (strncmp(argv[0],"sk_mst",6) != 0)
		mst_price = 0;
	else
		mst_price = 1;

	strcpy(inv_date,DateToString(comm_rec.tinv_date));

	init_scr();

	sprintf(err_str,"Now Processing %s ",argv[2]);
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);

	if ((ftmp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)",errno,PNAME);

	load_struct();

	head_output(argv[2]);

	process_data();

	fprintf(ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose(ftmp);

	shutdown_prog();
    return (EXIT_SUCCESS);
}

void
head_output (
 char *rep_name)
{
	int	i = 128 + no_branches * 10;
	int	cnt;
	char	rmask[18];
	
	if (!mst_price)
		i -= 10;
	
	sprintf(rmask,".R%%-%d.%ds\n",i + 7,i + 7);
	sprintf(ruler,rmask,overlay);

	for (cnt = 0;cnt < 5;cnt++)
		if (price[cnt] == 0)
			i -= 10;


	fprintf(ftmp,".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(ftmp,".LP%d\n",lpno);
	fprintf(ftmp,".PI12\n");
	fprintf(ftmp,".SO\n");

	if (mult_copys != 0) 
		fprintf(ftmp,".NC%d\n",mult_copys);
	
	fprintf(ftmp,".%d\n",(by_branch) ? 11 : 12);
	fprintf(ftmp,".L%d\n",i + 7);

	fprintf(ftmp,".B1\n");
	fprintf(ftmp,".E%s \n",clip(comm_rec.tco_name));

	if (!by_branch)
	{
		strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
		strcpy(esmr_rec.em_est_no,comm_rec.test_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
		if (cc)
			sys_err("Error in esmr During (DBFIND)",cc,PNAME);

		fprintf(ftmp,".EBranch: %s \n",clip(esmr_rec.em_est_name));
	}
	fprintf(ftmp,".E%s\n",clip(rep_name));
	fprintf(ftmp,".B1\n.E AS AT : %s\n",SystemTime());

	fprintf(ftmp,"%-s",ruler);

	fprintf(ftmp,"%-s",ruler + 2);

	fprintf(ftmp,"|  ITEM NUMBER   ");
	fprintf(ftmp,"|                P A R T                 ");
	fprintf(ftmp,"|UNIT");

	if (mst_price)
		fprintf(ftmp,"|   LAST  ");

	for (i = 0;i < 5;i++)
		if (price[i] == 1)
			fprintf(ftmp,"|%-9.9s",comm_rec.tprice[i]);
	fprintf(ftmp,"| ON HAND  |");
	for (i = 0;i < no_branches;i++)
		fprintf(ftmp,"%-9.9s|",branch[i].br_short);
	fprintf(ftmp,"\n");

	fprintf(ftmp,"|                ");
	fprintf(ftmp,"|              DESCRIPTION               ");
	fprintf(ftmp,"|    ");

	if (mst_price)
		fprintf(ftmp,"|   COST  ");
	for (i = 0;i < 5;i++)
		if (price[i] == 1)
			fprintf(ftmp,"|%-9.9s",comm_rec.tprice[i] + 9);
	fprintf(ftmp,"| COMPANY  |");
	for (i = 0;i < no_branches;i++)
		fprintf(ftmp,"         |");
	fprintf(ftmp,"\n");

	fprintf(ftmp,"%s",ruler + 2);
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
	abc_dbopen(data);
	ReadMisc();

	open_rec(ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec(esmr, esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec(excf, excf_list, excf_no_fields, "excf_id_no");
	open_rec(incc, incc_list, incc_no_fields, "incc_id_no");
	open_rec(inei, inei_list, inei_no_fields, "inei_id_no");
	open_rec(inmr, inmr_list, inmr_no_fields, "inmr_id_no_3");
	open_rec(inpr, inpr_list, inpr_no_fields, "inpr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(ccmr);
	abc_fclose(esmr);
	abc_fclose(excf);
	abc_fclose(incc);
	abc_fclose(inei);
	abc_fclose(inmr);
	abc_fclose(inpr);
	abc_dbclose(data);
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	int	i;

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	for (i = 0 ; i < 5; i++)
	{
		if (!strcmp(comm_rec.tprice[i],"               "))
			price[i] = 0;
		else
			price[i] = 1;
	}
}

void
load_struct (
 void)
{
	int	i;

	for (i = 0,no_branches = 5;i < 5;i++)
	{
		branch[i].br_index = 0;

		if (!strcmp(branch[i].br_no,"  "))
		{
			no_branches = i;
			sprintf(branch[i].br_short,"%-10.10s"," ");
			branch[i].br_hhcc[branch[i].br_index] = 0L;
			break;
		}
		else
		{
			if (by_branch)
			{
				strcpy(esmr_rec.em_co_no,comm_rec.tco_no);
				strcpy(esmr_rec.em_est_no,branch[i].br_no);
				cc = find_rec(esmr,&esmr_rec,COMPARISON,"r");
				if (!cc)
					sprintf(branch[i].br_short,"%-10.10s",
							esmr_rec.em_short_name);
			}
			else
			{
				strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
				strcpy(ccmr_rec.cm_est_no,comm_rec.test_no);
				strcpy(ccmr_rec.cm_cc_no,branch[i].br_no);
				cc = find_rec(ccmr,&ccmr_rec,COMPARISON,"r");

				if (!cc)
				{
					branch[i].br_index = 1;
					branch[i].br_hhcc[0] = ccmr_rec.cm_hhcc_hash;
					sprintf(branch[i].br_short,"%-10.10s",ccmr_rec.cm_acronym);
				}
				continue;
			}
		}

		/*---------------------------------------
		|	find all cc's for branch	|
		---------------------------------------*/
		strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(ccmr_rec.cm_est_no,branch[i].br_no);
		strcpy(ccmr_rec.cm_cc_no,"  ");
		cc = find_rec(ccmr,&ccmr_rec,GTEQ,"r");
		while (cc == 0 && !strcmp(ccmr_rec.cm_co_no,comm_rec.tco_no) && !strcmp(ccmr_rec.cm_est_no,branch[i].br_no))
		{
			if (ccmr_rec.cm_reports_ok[0] == 'Y')
				branch[i].br_hhcc[branch[i].br_index++] = ccmr_rec.cm_hhcc_hash;
			cc = find_rec(ccmr,&ccmr_rec,NEXT,"r");
		}
	}
}

void
process_data (
 void)
{
	int		i;
	int		first_time = TRUE;
	char	old_group[13];
	double	itemPrice;

	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_class,"%1.1s",lower);
	sprintf(inmr_rec.mr_category,"%-11.11s",lower + 1);
	sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");

	strcpy(old_group,lower);

	cc = find_rec(inmr,&inmr_rec,GTEQ,"r");

	/*-----------------------
	| go thru inmr by group |
	-----------------------*/
	while (!cc && !last_group())
	{
		if (!valid_group() || !in_branch())
		{
			cc = find_rec(inmr,&inmr_rec,NEXT,"r");
			continue;
		}

		/*----------------------------------
		| if group changes then page throw |
		----------------------------------*/
		if (first_time)
		{
			print_categ(first_time);
			sprintf(old_group,"%1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);
			first_time = FALSE;
		}
		else
		{
			if (valid_group() && new_group(old_group))
			{
				sprintf(old_group,"%1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);
				print_categ(first_time);
			}
		}

		
		dsp_process(" Item: ",inmr_rec.mr_item_no);

		inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy(inei_rec.ei_est_no,comm_rec.test_no);
		cc = find_rec(inei,&inei_rec,COMPARISON,"r");

		fprintf(ftmp,"|%-16.16s",inmr_rec.mr_item_no);

		fprintf(ftmp,"|%-40.40s",inmr_rec.mr_description);
		fprintf(ftmp,"|%-4.4s",inmr_rec.mr_sale_unit);
		if (mst_price)
			fprintf(ftmp,"|%9.2f",inei_rec.ei_last_cost);

		for (i = 0;i < 5;i++)
		{
		    if (price[i] == 1)
			{
				itemPrice = GetPrice(i + 1);
				fprintf(ftmp,"|%9.2f",DOLLARS(itemPrice));
			}
		}

		fprintf(ftmp,"|%9.2f ",inmr_rec.mr_on_hand);

		/*---------------------
		| print branch totals |
		---------------------*/
		print_branch();

		fprintf(ftmp,"|\n");

		cc = find_rec(inmr,&inmr_rec,NEXT,"r");
	}
}

/*------------------------------------
| Look up price at company level. If |
| not found then price = 0.00.       |
------------------------------------*/
double	
GetPrice (
 int pType)
{
	inpr_rec.hhgu_hash  = 0L;
	inpr_rec.hhbr_hash  = inmr_rec.mr_hhbr_hash;
	inpr_rec.price_type = pType;
	strcpy(inpr_rec.br_no, "  ");
	strcpy(inpr_rec.wh_no, "  ");
	sprintf(inpr_rec.curr_code, "%-3.3s", Curr_code);
	sprintf(inpr_rec.area_code, "%-2.2s", "  ");
	sprintf(inpr_rec.cust_type, "%-3.3s", "   ");
	cc = find_rec(inpr, &inpr_rec, COMPARISON, "r");
	if (!cc)
		return(inpr_rec.base);
		
	/*------------------
	| Price not found. |
	------------------*/
	return((double)0.00);
}

/*=======================================
|	check if processing new group	|
=======================================*/
int
new_group (
 char *old_group)
{
	return(old_group[0] != inmr_rec.mr_class[0] || strcmp(old_group + 1,inmr_rec.mr_category));
}

/*=======================================
|	check if processed last group	|
=======================================*/
int
last_group (
 void)
{
	return(inmr_rec.mr_class[0] > upper[0]);
}

/*=======================================
|	check if item in valid group	|
=======================================*/
int
valid_group (
 void)
{
	int	class_ok = (inmr_rec.mr_class[0] <= upper[0] && inmr_rec.mr_class[0] >= lower[0]);
	int	categ_ok = (strcmp(inmr_rec.mr_category,upper + 1) <= 0 && strcmp(inmr_rec.mr_category,lower + 1) >= 0);

	return(class_ok && categ_ok);
}

/*=======================================
|	print on_hand at branch level	|
=======================================*/
void
print_branch (
 void)
{
	int	i;
	int	cc_cnt;		/* loop control for hhcc_hash in branch[] */
	float	on_hand = 0.00;

	for (i = 0;i < no_branches;i++)
	{
		if (!strcmp(branch[i].br_no,"  "))
			fprintf(ftmp,"|         ");
		else
		{
			/*-------------------------------
			|	sum on_hand for branch	|
			-------------------------------*/
			for (cc_cnt = 0,on_hand = 0.00;cc_cnt < branch[i].br_index;cc_cnt++)
			{
				incc_rec.cc_hhcc_hash = branch[i].br_hhcc[cc_cnt];
				incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
				cc = find_rec(incc,&incc_rec,COMPARISON,"r");
				
				if (!cc)
					on_hand += incc_rec.cc_closing_stock;
			}
			fprintf(ftmp,"|%9.2f",on_hand);
		}
	}
}

int
in_branch (
 void)
{
	int	i;
	int	cc_cnt;

	for (i = 0;i < no_branches;i++)
	{
		for (cc_cnt = 0;cc_cnt < branch[i].br_index;cc_cnt++)
		{
			incc_rec.cc_hhcc_hash = branch[i].br_hhcc[cc_cnt];
			incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
			cc = find_rec(incc,&incc_rec,COMPARISON,"r");
			if (!cc)
				return(TRUE);
		}
	}
	return(FALSE);
}

void		
print_categ (
 int first_time)
{
	int	len = 131 + no_branches * 10;
	int	cnt;
	char	mask[20];

	for (cnt = 0;cnt < 5;cnt++)
		if (price[cnt] == 0)
			len -= 10;

	if (!mst_price)
		len -= 10;

	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	strcpy(excf_rec.cf_categ_no,inmr_rec.mr_category);
	cc = find_rec(excf,&excf_rec,COMPARISON,"r");
	if (cc)
	      strcpy(excf_rec.cf_categ_description,"No Category description found.");

	expand(err_str,excf_rec.cf_categ_description);

	sprintf(mask,".PD| %%-%d.%ds |\n",len,len);
	fprintf(ftmp,mask,err_str);
	if ( !first_time )
		fprintf(ftmp,".PA\n");
}
