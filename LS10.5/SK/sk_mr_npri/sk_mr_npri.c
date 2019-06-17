/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_mr_npri.c    )                                |
|  Program Desc  : ( Inventory Master Price List (Normal Items    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (07/10/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (08/11/88)      | Modified  by  : B. C. Lim.       |
|  Date Modified : (05/07/92)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (04/11/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (23/03/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (28/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (12/09/97)      | Modified  by  : Roanna Marcelino.|
|                                                                     |
|  Comments      : Tidy up program.                                   |
|   (05/07/93)   : ABC  9239 - Updated to fix Supercessions.          |
|   (04/11/93)   : HGP  9501 - Remove inmr prices.  Add new pricing.  |
|   (23/03/94)   : HGP 10457 - Remove hard coded GST.                 |
|   (28/05/96)   : Updated to fix problems in DateToString.                |
|   (12/09/97)   : Modified for Multilingual Conversion.              |
|                :                                                    |
|                                                                     |
| $Log: sk_mr_npri.c,v $
| Revision 5.2  2002/12/01 04:48:16  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.1  2001/08/09 09:19:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:16:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:39  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:21  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:31:00  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/10/12 21:20:35  scott
| Updated by Gerry from ansi project.
|
| Revision 1.9  1999/10/08 05:32:36  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:20:19  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN 

char	*PNAME = "$RCSfile: sk_mr_npri.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mr_npri/sk_mr_npri.c,v 5.2 2002/12/01 04:48:16 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

	/*==================================
	| file comm { System Common file } |
	==================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_inv_date"}
	};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_name[41];
		char	tes_short[16];
		char	tcc_no[3];
		long	tinv_date;
	} comm_rec;

	/*==========================================
	| file inmr {Inventory Master Base Record} |
	==========================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_supercession"},
		{"inmr_alpha_code"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_on_hand"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 13;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_super_no[17];
		char	mr_acr_alpha_code[17];
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
		char	cf_cat_no[12];
		char	cf_cat_desc[41];
		char	cf_stat_flag[2];
	} excf_rec;

	char	*data = "data",
			*comm = "comm",
			*excf = "excf",
			*inmr = "inmr",
			*inpr = "inpr";

	char	inve_date[11];

	int	first_time = TRUE,
		sel_price = 0,
		lpno = 1,
		inv_mth,
		find_type = GTEQ,
	 	loop_flag = 1;

	FILE	*pp;

	char	ser_no[2],
			upper[13],
			lower[13],
			new_group[13],
			old_group[13],
			sr_group[13];
	char	Curr_code[4];
	char	gst_code[4];


/*=======================
| Function Declarations | 
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  procfile (void);
double GetRetail (void);
void head (int pr_no);
void pr_categ (int first_flag);


/*==========================
| Main Processing Routine. | 
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int	monthNum;

	if (argc < 4)
	{
		print_at(0,0,mlSkMess111,argv[0]);
		return (EXIT_FAILURE);
	}

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf(Curr_code, "%-3.3s", get_env("CURR_CODE"));
	
	/*---------------
	| Get gst code. |
	---------------*/
	sprintf(gst_code, "%-3.3s", get_env("GST_TAX_NAME"));
	
	lpno = atoi(argv[1]);
	sprintf(lower,"%-12.12s",argv[2]);
	sprintf(upper,"%-12.12s",argv[3]);

	OpenDB();

	DateToDMY (comm_rec.tinv_date, NULL, &monthNum, NULL);
	inv_mth = monthNum; 

	head(lpno);

	strcpy(inmr_rec.mr_co_no,     comm_rec.tco_no);
	sprintf(inmr_rec.mr_class,    "%-1.1s",   lower);
	sprintf(inmr_rec.mr_category, "%-11.11s", lower + 1);
	sprintf(inmr_rec.mr_item_no,  "%-16.16s", " ");

	dsp_screen("Inventory Price List Report", 
			   comm_rec.tco_no, 
			   comm_rec.tco_name);

	cc = find_rec("inmr", &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.mr_co_no, comm_rec.tco_no))
	{
		sprintf(sr_group, "%s%-11.11s",inmr_rec.mr_class, inmr_rec.mr_category);
 		if (strcmp(sr_group, lower) < 0)
		{
			cc = find_rec("inmr", &inmr_rec, NEXT, "r");
			continue;
		}
 		if (strcmp(sr_group, upper) > 0) 
			break;

		if (inmr_rec.mr_costing_flag[0] != 'S')
			cc = procfile();

		cc = find_rec("inmr", &inmr_rec, NEXT, "r");
	}

	fprintf(pp, ".EOF\n");
	pclose(pp);

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

void
OpenDB (
 void)
{
	abc_dbopen(data);

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec(excf, excf_list, excf_no_fields, "excf_id_no");
	open_rec(inmr, inmr_list, inmr_no_fields, "inmr_id_no_3");
	open_rec(inpr, inpr_list, inpr_no_fields, "inpr_id_no");
}

void
CloseDB( 
 void)
{
	abc_fclose(excf);
	abc_fclose(inmr);
	abc_fclose(inpr);
	abc_dbclose(data);
}

int 
procfile (
 void)
{
	double	price1;
	double 	GetRetail(void);

	sprintf(new_group, "%s%-11.11s", inmr_rec.mr_class, inmr_rec.mr_category);

	if (strcmp(new_group, old_group))
	{
		strcpy(old_group, new_group);
		pr_categ(first_time);
		first_time = FALSE;
	}
		
	if ( strcmp( inmr_rec.mr_super_no, "                " ) )
	{
		fprintf(pp, "|%16.16s| SAME AS %16.16s                |    |            |\n",
			inmr_rec.mr_item_no,
			inmr_rec.mr_super_no);
	}
	else
	{
		price1 = GetRetail();
		fprintf(pp, "|%16.16s| %40.40s|%4.4s| %10.2f |\n",
				inmr_rec.mr_item_no,
				inmr_rec.mr_description,
				inmr_rec.mr_sale_unit,
				DOLLARS(price1));
	}

	dsp_process("Item :", inmr_rec.mr_item_no);

	sprintf(old_group, "%s%11.11s", inmr_rec.mr_class, inmr_rec.mr_category);

	return(0);
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

void
head (
 int pr_no)
{

	if ((pp = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(pp, ".LP%d\n",pr_no);
	fprintf(pp, ".PL58\n");
	fprintf(pp, ".11\n");
	fprintf(pp, ".L80\n");
	fprintf(pp, ".B1\n");
	fprintf(pp, ".CI N V E N T O R Y   M A S T E R   R E T A I L  P R I C E   L I S T\n");
	fprintf(pp, ".C(EXCLUDING  %-3.3s)\n", gst_code);
	fprintf(pp, ".B1\n");
	fprintf(pp, ".C%s AS AT %s\n", clip(comm_rec.tco_name), SystemTime());
	fprintf(pp, ".B1\n");

	fprintf(pp, ".R=================");
	fprintf(pp, "==========================================");
	fprintf(pp, "=====");
	fprintf(pp, "==============\n");

	fprintf(pp, "=================");
	fprintf(pp, "==========================================");
	fprintf(pp, "=====");
	fprintf(pp, "==============\n");

	fprintf(pp, "|   ITEM NUMBER  ");
	fprintf(pp, "|               DESCRIPTION               ");
	fprintf(pp, "|UOM.");
	fprintf(pp, "|    PRICE.  |\n");

	fprintf(pp, "|----------------");
	fprintf(pp, "|-----------------------------------------");
	fprintf(pp, "|----");
	fprintf(pp, "|------------|\n");

	fflush(pp);
}

void
pr_categ (
 int first_flag)
{
	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no, "%-11.11s", inmr_rec.mr_category);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy(excf_rec.cf_cat_desc,ML(mlStdMess220));

	expand(err_str, excf_rec.cf_cat_desc);
	fprintf(pp, ".PD| %-74.74s |\n",err_str);

	if (!first_flag)
		fprintf(pp, ".PA\n");
}
