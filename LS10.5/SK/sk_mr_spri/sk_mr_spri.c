/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_mr_spri.c   )                                 |
|  Program Desc  : ( Stock Master Price List for Serial Items.    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, inmr, innh, innd,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 25/01/87         |
|---------------------------------------------------------------------|
|  Date Modified : (25/01/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (08/11/88)      | Modified  by  : B. C. Lim.       |
|  Date Modified : (05/11/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (24/03/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (01/10/97)      | Modified  by  : Marnie Organo.   |
|                                                                     |
|  Comments      : Tidy up program, remove shutdown_prog() after read_comm|
|                : & OpenDB.                                         |
|                :                                                    |
|  (05/11/93)    : HGP  9501 - Remove inmr_price1 - 5.  Now look up   |
|                : retail price on inpr at company level.             |
|                :                                                    |
|  (24/03/94)    : HGP 10457 - Remove hard coded GST.                 |
|                                                                     |
| $Log: sk_mr_spri.c,v $
| Revision 5.2  2002/12/01 04:48:16  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.1  2001/08/09 09:19:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:16:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:00  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:40  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:22  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:31:00  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.9  1999/10/13 02:42:03  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/12 21:20:35  scott
| Updated by Gerry from ansi project.
|
| Revision 1.7  1999/10/08 05:32:36  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:20:19  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_mr_spri.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mr_spri/sk_mr_spri.c,v 5.2 2002/12/01 04:48:16 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define	MAX_BR	20

FILE	*pp;

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
		char tconame[41];
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

	/*===========================================
	| file inmr {Inventory Master Base Record}. |
	===========================================*/
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
		{"inmr_sale_unit"},
		{"inmr_on_hand"},
		{"inmr_on_order"},
		{"inmr_committed"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 17;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_acr_alpha_code[17];
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
		float	mr_on_order;
		float	mr_committed;
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
		{"incc_ytd_sales"},
		{"incc_stat_flag"}
	};

	int incc_no_fields = 7;

	struct {
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		long	cc_hhwh_hash;
		char	cc_sort[29];
		float	cc_closing_stock;
		float	cc_ytd_sales;
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

	/*===============================
	| Inventory Serial Number file. |
	===============================*/
	struct dbview insf_list[] ={
		{"insf_hhsf_hash"},
		{"insf_hhwh_hash"},
		{"insf_status"},
		{"insf_serial_no"},
		{"insf_stat_flag"}
	};

	int insf_no_fields = 5;

	struct {
		long	sf_hhsf_hash;
		long	sf_hhwh_hash;
		char	sf_status[2];
		char	sf_serial_no[26];
		char	sf_stat_flag[2];
	} insf_rec;

	/*============================
	| Inventory Note Pad Record. |
	============================*/
	struct dbview innh_list[] ={
		{"innh_co_no"},
		{"innh_hhbr_hash"},
		{"innh_hhwh_hash"},
		{"innh_hhnh_hash"},
		{"innh_serial_no"},
		{"innh_stat_flag"}
	};

	int innh_no_fields = 6;

	struct {
		char	nh_co_no[3];
		long	nh_hhbr_hash;
		long	nh_hhwh_hash;
		long	nh_hhnh_hash;
		char	nh_serial_no[26];
		char	nh_stat_flag[2];
	} innh_rec;

	/*======================================
	| Inventory Note Pad Record (Details). |
	======================================*/
	struct dbview innd_list[] ={
		{"innd_hhnh_hash"},
		{"innd_line_no"},
		{"innd_comments"}
	};

	int innd_no_fields = 3;

	struct {
		long	nd_hhnh_hash;
		int	nd_line_no;
		char	nd_comments[61];
	} innd_rec;

	char	*data = "data",
			*comm = "comm",
			*ccmr = "ccmr",
			*esmr = "esmr",
			*excf = "excf",
			*incc = "incc",
			*inmr = "inmr",
			*inpr = "inpr",
			*innd = "innd",
			*innh = "innh",
			*insf = "insf";

	int		first_time = TRUE;
	int		found_ser = 0;
	int		lpno = 1;
	int		no_branches;


	char	upper[13];
	char	lower[13];
	char	Curr_code[4];
	char	gst_code[4];

	struct {
		char	br_no[3];	 /* est_no's as parameter's	*/
		long	br_hhcc[99];	 /* hhcc for cc's of branch	*/
		char	cc_no[99][3];	 /* cc nos for branch.     	*/
		int	br_index;	 /* index to last hhcc_hash	*/
	} branch[MAX_BR];

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
void read_records (void);
double GetRetail (void);
int  proc_serial (char *cc_name);
void print_categ (void);
void get_notes (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	inv_date[11];

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf(Curr_code, "%-3.3s", get_env("CURR_CODE"));
	
	/*---------------
	| Get gst code. |
	---------------*/
	sprintf(gst_code, "%-3.3s", get_env("GST_TAX_NAME"));
	
	OpenDB();

	ReadMisc();

	lpno = atoi(argv[1]);

	if (argc > 3)
	{
		sprintf(lower, "%-12.12s", argv[2]);
		sprintf(upper, "%-12.12s", argv[3]);
	}
	else 
	{
		strcpy(lower, "            ");
		strcpy(upper, "~~~~~~~~~~~~");
	}

	sprintf(inv_date, "%-10.10s",	DateToString(comm_rec.tinv_date));

	init_scr();

	sprintf(err_str,"Now Processing Master Price List for Serial Items.");
	dsp_screen(err_str, comm_rec.tco_no, comm_rec.tconame);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((pp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	head_output(argv[2]);

	process_data();

	fprintf(pp,".PI10\n");
    fprintf(pp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose(pp);

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 char *rep_name)
{
	fprintf(pp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(pp, ".LP%d\n",lpno);
	fprintf(pp, ".PI12\n");
	fprintf(pp, ".PL58\n");
	fprintf(pp, ".13\n");
	fprintf(pp, ".PI12\n");
	fprintf(pp, ".L110\n");

	fprintf(pp, ".B1\n");
	fprintf(pp, ".C%s\n",clip(comm_rec.tconame));

	fprintf(pp, ".CSTOCK STATUS + PRICE LIST (CONFIDENTIAL) (Excluding %-3.3s)\n", gst_code);
	fprintf(pp, ".B1\n");
	fprintf(pp, ".C AS AT : %s\n",SystemTime());

	fprintf(pp, ".R=================");
	fprintf(pp, "=========================================");
	fprintf(pp, "===================");
	fprintf(pp, "=========");
	fprintf(pp, "======");
	fprintf(pp, "======");
	fprintf(pp, "============\n");

	fprintf(pp, "=================");
	fprintf(pp, "=========================================");
	fprintf(pp, "===================");
	fprintf(pp, "=========");
	fprintf(pp, "======");
	fprintf(pp, "======");
	fprintf(pp, "============\n");

	fprintf(pp, "|  ITEM  NUMBER  ");
	fprintf(pp, "|              DESCRIPTION               ");
	fprintf(pp, "|  SERIAL NUMBER.  ");
	fprintf(pp, "| BRANCH ");
	fprintf(pp, "|COMM ");
	fprintf(pp, "| ON  ");
	fprintf(pp, "|  RETAIL  |\n");

	fprintf(pp, "|                ");
	fprintf(pp, "|                                        ");
	fprintf(pp, "|                  ");
	fprintf(pp, "|        ");
	fprintf(pp, "|STOCK");
	fprintf(pp, "|ORDER");
	fprintf(pp, "|  PRICE.  |\n");

	fprintf(pp, "|----------------");
	fprintf(pp, "|----------------------------------------");
	fprintf(pp, "|------------------");
	fprintf(pp, "|--------");
	fprintf(pp, "|-----");
	fprintf(pp, "|-----");
	fprintf(pp, "|----------|\n");
	fflush(pp);
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

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec(excf, excf_list, excf_no_fields, "excf_id_no");
	open_rec(incc, incc_list, incc_no_fields, "incc_id_no");
	open_rec(inmr, inmr_list, inmr_no_fields, "inmr_id_no_3");
	open_rec(innd, innd_list, innd_no_fields, "innd_id_no");
	open_rec(innh, innh_list, innh_no_fields, "innh_id_no2");
	open_rec(inpr, inpr_list, inpr_no_fields, "inpr_id_no");
	open_rec(insf, insf_list, insf_no_fields, "insf_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(ccmr);
	abc_fclose(excf);
	abc_fclose(incc);
	abc_fclose(inmr);
	abc_fclose(inpr);
	abc_fclose(innd);
	abc_fclose(innh);
	abc_fclose(insf);
	abc_dbclose(data);
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	open_rec(esmr, esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec(ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");

	load_struct();

	abc_fclose(esmr);
}

void
load_struct (
 void)
{
	int	i = 0;

	no_branches = 0;

	strcpy(esmr_rec.em_co_no, comm_rec.tco_no);
	strcpy(esmr_rec.em_est_no, "  ");

	cc = find_rec("esmr", &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp(esmr_rec.em_co_no, comm_rec.tco_no))
	{
	    strcpy(branch[i].br_no, esmr_rec.em_est_no);
	    /*-----------------------------------
	    |	find all cc's for branch	|
	    -----------------------------------*/
	    strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
	    strcpy(ccmr_rec.cm_est_no,branch[i].br_no);
	    strcpy(ccmr_rec.cm_cc_no,"  ");
	    cc = find_rec("ccmr",&ccmr_rec,GTEQ,"r");
	    while (cc == 0 && !strcmp(ccmr_rec.cm_co_no,comm_rec.tco_no) 
                           && !strcmp(ccmr_rec.cm_est_no,branch[i].br_no))
	    {
		if (ccmr_rec.cm_reports_ok[0] == 'Y')
		{
			branch[i].br_hhcc[branch[i].br_index] = ccmr_rec.cm_hhcc_hash;
			strcpy(branch[i].cc_no[branch[i].br_index++],ccmr_rec.cm_cc_no);
		}
		cc = find_rec("ccmr",&ccmr_rec,NEXT,"r");
	    }
	    cc = find_rec("esmr", &esmr_rec, NEXT, "r");
	    i++;
	}
	no_branches = i;
	
	abc_fclose("esmr");
}

void
process_data (
 void)
{
	char	old_group[13];
	char	new_group[13];

	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_class,"%-1.1s",lower);
	sprintf(inmr_rec.mr_category,"%-11.11s",lower + 1);
	sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");

	sprintf(old_group,"%-12.12s", lower);

	cc = find_rec("inmr",&inmr_rec,GTEQ,"r");

	/*-------------------------------
	|	go thru inmr by group	|
	-------------------------------*/

	while (!cc && !strcmp(inmr_rec.mr_co_no, comm_rec.tco_no))
	{
	
	     	sprintf(new_group, "%s%s",inmr_rec.mr_class,inmr_rec.mr_category);

	     	if (strcmp(new_group,upper) > 0)
			break;

	    	if (inmr_rec.mr_serial_item[0] != 'Y')
	    	{
	    		cc = find_rec("inmr",&inmr_rec,NEXT,"r");
			continue;
	    	}

		if (strcmp(old_group, new_group))
		{	
			print_categ();
			sprintf(old_group,"%s%s",inmr_rec.mr_class,inmr_rec.mr_category);
		}

	    	dsp_process(" Item: ", inmr_rec.mr_item_no);

		fprintf(pp, "|%-16.16s",inmr_rec.mr_item_no);
		fprintf(pp, "|%-40.40s",inmr_rec.mr_description);

		/*-------------------------------
		|	print branch totals	|
		-------------------------------*/
		read_records();
		fprintf(pp, "|----------------");
		fprintf(pp, "|----------------------------------------");
		fprintf(pp, "|------------------");
		fprintf(pp, "|--------");
		fprintf(pp, "|-----");
		fprintf(pp, "|-----");
		fprintf(pp, "|----------|\n");
	    	cc = find_rec("inmr",&inmr_rec,NEXT,"r");
	}
}

/*===============================
| print on_hand at branch level |
===============================*/
void
read_records (
 void)
{
	int		i;
	int		cc_cnt;
	int		rec_found = 0;
	double	price1;
	
	found_ser = 0;
	
	for (i = 0; i < no_branches; i++)
	{
	    for (cc_cnt = 0;cc_cnt < branch[i].br_index;cc_cnt++)
	    {
			incc_rec.cc_hhcc_hash = branch[i].br_hhcc[cc_cnt];
			incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
			cc = find_rec("incc",&incc_rec,COMPARISON,"r");
			if (!cc)
			{
	    		strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
	    		strcpy(ccmr_rec.cm_est_no,branch[i].br_no);
	    		strcpy(ccmr_rec.cm_cc_no,branch[i].cc_no[cc_cnt]);
	    		cc = find_rec("ccmr",&ccmr_rec,COMPARISON,"r");
				rec_found += proc_serial(ccmr_rec.cm_acronym);
			}
	    }
	}
	if (!rec_found)
	{
		price1 = GetRetail();

		fprintf(pp, "| NO STOCK ON HAND ");
		fprintf(pp, "|        ");
		fprintf(pp, "|     ");
		fprintf(pp, "|%5.2f", inmr_rec.mr_on_order);
		fprintf(pp, "|%10.2f|\n",DOLLARS(price1));
	}
}
/*------------------------------------
| Look up price at company level. If |
| not found then price = 0.00.       |
------------------------------------*/
double	
GetRetail (
 void)
{
	inpr_rec.hhgu_hash  = 0L;
	inpr_rec.hhbr_hash  = inmr_rec.mr_hhbr_hash;
	inpr_rec.price_type = 1;
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

int
proc_serial (
 char *cc_name)
{
	double	price1;

	insf_rec.sf_hhwh_hash = incc_rec.cc_hhwh_hash;
	strcpy(insf_rec.sf_serial_no, "                         ");
	cc = find_rec("insf",&insf_rec, GTEQ, "r");
	while (!cc && insf_rec.sf_hhwh_hash == incc_rec.cc_hhwh_hash)
	{
	    switch (insf_rec.sf_status[0])
	    {
	        case 'F':
		    if (found_ser != 0)
		    {
		    	fprintf(pp, "|%16.16s"," ");
		    	fprintf(pp, "|%40.40s"," ");
		    }
		    fprintf(pp, "|%-18.18s",insf_rec.sf_serial_no); 
		    fprintf(pp, "|%-8.8s",cc_name);
		    fprintf(pp, "| NO. ");
		    found_ser++;
		    break;

	        case 'C':
		    if (found_ser != 0)
		    {
		    	fprintf(pp, "|%16.16s"," ");
		    	fprintf(pp, "|%40.40s"," ");
		    }
		    fprintf(pp, "|%-18.18s",insf_rec.sf_serial_no); 
		    fprintf(pp, "|%-8.8s",cc_name);
		    fprintf(pp, "| YES ");
		    found_ser++;
		    break;

	        default :
    	    	    break;
	    }
	    if (insf_rec.sf_status[0] == 'F' || insf_rec.sf_status[0] == 'C')
	    {
			price1 = GetRetail();

	    	if (found_ser == 1)
	    	{
		    	fprintf(pp, "|%5.2f", inmr_rec.mr_on_order);
		    	fprintf(pp, "|%10.2f|\n",DOLLARS(price1));
	    	}
	    	else if (found_ser > 1)
	    	{
		    	fprintf(pp, "|     ");
		    	fprintf(pp, "|          |\n");
	    	}
		get_notes();
	    }
	    cc = find_rec("insf",&insf_rec, NEXT, "r");
	}
	return(found_ser);
}

void	
print_categ (
 void)
{
	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no,"%11.11s",inmr_rec.mr_category);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
	      sprintf(excf_rec.cf_categ_description, "%-40.40s",
					"No Category description found.");

	expand(err_str,excf_rec.cf_categ_description);

	fprintf(pp, ".PD|%s%28.28s|\n",err_str, " ");

	if (first_time)
	{
		fprintf(pp, "|================");
		fprintf(pp, "|========================================");
		fprintf(pp, "|==================");
		fprintf(pp, "|========");
		fprintf(pp, "|=====");
		fprintf(pp, "|=====");
		fprintf(pp, "|==========|\n");
		fprintf(pp, "|%s%28.28s|\n",err_str, " ");
	}
	else
		fprintf(pp, ".PA\n");
	
	first_time = FALSE;
}

void
get_notes (
 void)
{
	int	det_cc = 0;

	innh_rec.nh_hhbr_hash = inmr_rec.mr_hhbr_hash;
	innh_rec.nh_hhwh_hash = incc_rec.cc_hhwh_hash;
	strcpy(innh_rec.nh_serial_no, insf_rec.sf_serial_no);
	if (!find_rec("innh", &innh_rec, COMPARISON, "r"))
	{
	     innd_rec.nd_line_no = 0;
	     innd_rec.nd_hhnh_hash = innh_rec.nh_hhnh_hash;
	     det_cc = find_rec("innd", &innd_rec, GTEQ, "r");
	     while (!det_cc && innd_rec.nd_hhnh_hash == innh_rec.nh_hhnh_hash)
	     {
		fprintf(pp, "|%16.16s", " ");
		fprintf(pp, "|    %-60.60s    ", innd_rec.nd_comments);
		fprintf(pp, "|     ");
		fprintf(pp, "|     ");
		fprintf(pp, "|          |\n");

		det_cc = find_rec("innd", &innd_rec, NEXT, "r");
	     }
	     fprintf(pp, "|                ");
	     fprintf(pp, "|                                        ");
	     fprintf(pp, "                   ");
	     fprintf(pp, "         ");
	     fprintf(pp, "|     ");
	     fprintf(pp, "|     ");
	     fprintf(pp, "|          |\n");
	}
}
