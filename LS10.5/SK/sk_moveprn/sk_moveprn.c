/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_moveprn.c   )                                 |
|  Program Desc  : ( Print Category totals by class/warehouse/branch) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmu, excf,      ,     ,                    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow    | Date Written  : 14/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (07/03/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (03/05/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (28/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (11/09/97)      | Modified  by  : Roanna Marcelino |
|                                                                     |
|  Comments      : 07/03/91 - Fixed class totals (they were either not|
|                :  appearing or were wrong).  Report line now shows  |
|                :  correct warehouse number.                         |
|                : (03/05/91) - Updated to clean up and fix core dump.|
|                : (28/05/96) - Updated to fix problems in DateToString.   |
|                : (11/09/97) - Modified for Multilingual Conversion. |
|                :                                                    |
| $Log: sk_moveprn.c,v $
| Revision 5.1  2001/08/09 09:19:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:16:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:34  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:20  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:30:59  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/10/12 21:20:34  scott
| Updated by Gerry from ansi project.
|
| Revision 1.6  1999/10/08 05:32:35  scott
| First Pass checkin by Scott.
|
| Revision 1.5  1999/06/20 05:20:19  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_moveprn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_moveprn/sk_moveprn.c,v 5.1 2001/08/09 09:19:15 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

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
		{"comm_fiscal"},
	};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
		int	tfiscal;
	} comm_rec;

	/*===========================
	| Inventory Movements File. |
	===========================*/
	struct dbview inmu_list[] ={
		{"inmu_co_no"},
		{"inmu_br_no"},
		{"inmu_wh_no"},
		{"inmu_class"},
		{"inmu_category"},
		{"inmu_year"},
		{"inmu_period"},
		{"inmu_trin_mty"},
		{"inmu_trout_mty"},
		{"inmu_pur_mty"},
		{"inmu_sal_mty"},
		{"inmu_icst_mty"},
		{"inmu_crd_mty"},
		{"inmu_ccst_mty"},
		{"inmu_stat_flag"}
	};

	int inmu_no_fields = 15;

	struct {
		char	mu_co_no[3];
		char	mu_br_no[3];
		char	mu_wh_no[3];
		char	mu_class[2];
		char	mu_category[12];
		char	mu_year[2];
		char	mu_period[3];
		double	mu_trin_mty;		/*  Money field  */
		double	mu_trout_mty;		/*  Money field  */
		double	mu_pur_mty;		/*  Money field  */
		double	mu_sal_mty;		/*  Money field  */
		double	mu_icst_mty;		/*  Money field  */
		double	mu_crd_mty;		/*  Money field  */
		double	mu_ccst_mty;		/*  Money field  */
		char	mu_stat_flag[2];
	} inmu_rec;

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
		char	cf_description[41];
		char	cf_stat_flag[2];
	} excf_rec;

	int	printerNumber = 1;

	FILE	*ftmp;

	double	ln_tot[7],
			cl_tot[7],
			wh_tot[7],
			br_tot[7],
			gr_tot[7];

	char	curr_mnth[3], 
			lower[13],
			upper[13]; 

	double	mtd_prf = 0.00,
		ytd_prf = 0.00;

/*=======================
| Function Declarations |
=======================*/
void head_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void clear_totals (char *type);
void process_data (void);
void sum_inmu (void);
void prt_line (char *last_cat, char *br_no, char *wh_no);
void prt_wh_tot (void);
void prt_bt_tot (void);
void prt_cl_tot (void);
void prt_totals (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int		monthNum;

	if (argc != 2)
	{
		print_at(0,0,mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi(argv[1]);

	/*======================
	| Open database files. |
	======================*/
	OpenDB();

	DateToDMY (comm_rec.tinv_date, NULL, &monthNum, NULL);
	sprintf(curr_mnth,"%02d", monthNum);

	init_scr();

	dsp_screen("Processing : Stock Movements Report.", 
			comm_rec.tco_no, comm_rec.tco_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	head_output();

	process_data();

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	prt_totals();
	fprintf(ftmp, ".EOF\n");
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
	fprintf(ftmp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(ftmp, ".LP%d\n",printerNumber);
	fprintf(ftmp, ".PI12\n");
	fprintf(ftmp, ".11\n");
	fprintf(ftmp, ".L158\n");

	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".ESTOCK CATEGORY MOVEMENTS TOTALS REPORT\n");
	fprintf(ftmp, ".E%s \n",clip(comm_rec.tco_name));
	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".E AS AT : %s\n",SystemTime());

	fprintf(ftmp, ".R====================");
	fprintf(ftmp, "======================================");
	fprintf(ftmp, "============");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "==================\n");

	fprintf(ftmp, "====================");
	fprintf(ftmp, "===================================");
	fprintf(ftmp, "============");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "========================");
	fprintf(ftmp, "==================\n");

	fprintf(ftmp, "|BR|WH|    GROUP   |");
	fprintf(ftmp, "      CATEGORY DESCRIPTION.       |");
	fprintf(ftmp, " PURCHASES |");
	fprintf(ftmp, " TRANSFERS | TRANSFERS |");
	fprintf(ftmp, " MTH SALES | YTD SALES |");
	fprintf(ftmp, "    MTD    |    YTD    |");
	fprintf(ftmp, "   MTD  |  YTD   |\n");

	fprintf(ftmp, "|NO|NO|            |");
	fprintf(ftmp, "                                  |");
	fprintf(ftmp, "   VALUE.  |");
	fprintf(ftmp, " IN VALUE  | OUT VALUE |");
	fprintf(ftmp, "   VALUE   |   VALUE   |");
	fprintf(ftmp, "  PROFIT.  |  PROFIT.  |");
	fprintf(ftmp, "PRF PERC|PRF PERC|\n");

	fprintf(ftmp, "|--|--|------------|");
	fprintf(ftmp, "----------------------------------|");
	fprintf(ftmp, "-----------|");
	fprintf(ftmp, "-----------|-----------|");
	fprintf(ftmp, "-----------|-----------|");
	fprintf(ftmp, "-----------|-----------|");
	fprintf(ftmp, "--------|--------|\n");
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
	open_rec("inmu", inmu_list, inmu_no_fields, "inmu_id_no");
	open_rec("excf", excf_list, excf_no_fields, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("inmu");
	abc_fclose("excf");
	abc_dbclose("data");
}

void
clear_totals (
 char *type)
{
	int	i;

	if (type[0] == 'L')
	{
		for (i = 0; i < 7; i++)
			ln_tot[i] = 0.00;
	}

	if (type[0] == 'C')
	{
		for (i = 0; i < 7; i++)
			cl_tot[i] = 0.00;
	}
	
	if (type[0] == 'W')
	{
		for (i = 0; i < 7; i++)
			wh_tot[i] = 0.00;
	}
	if (type[0] == 'B')
	{
		for (i = 0; i < 7; i++)
			br_tot[i] = 0.00;
	}
	if (type[0] == 'A')
	{
		for (i = 0; i < 7; i++)
		{
			ln_tot[i] = 0.00;
			cl_tot[i] = 0.00;
			wh_tot[i] = 0.00;
			br_tot[i] = 0.00;
			gr_tot[i] = 0.00;
		}
	}
}

void
process_data (
 void)
{
	char	last_br[3];
	char	last_cl[2];
	char	last_wh[3];
	char	last_group[13];
	char	new_group[13];
	char	new_br[3];
	char	new_wh[3];
	char	new_cl[2];
	int	tot_prnted = FALSE;

	clear_totals( "A" );

	strcpy(inmu_rec.mu_co_no,comm_rec.tco_no);
	strcpy(inmu_rec.mu_br_no,"  ");
	strcpy(inmu_rec.mu_wh_no,"  ");
	strcpy(inmu_rec.mu_class," ");
	sprintf(inmu_rec.mu_category,"%-11.11s"," ");
	strcpy(inmu_rec.mu_year," ");
	strcpy(inmu_rec.mu_period,"  ");

	cc = find_rec("inmu",&inmu_rec,GTEQ,"r");
	strcpy(last_cl,inmu_rec.mu_class);
	strcpy(last_br,inmu_rec.mu_br_no);
	strcpy(last_wh,inmu_rec.mu_wh_no);
	sprintf(last_group,"%-1.1s%-11.11s",
				inmu_rec.mu_class,inmu_rec.mu_category);

	while (!cc && !strcmp(inmu_rec.mu_co_no,comm_rec.tco_no))
	{
		sprintf(new_group,"%-1.1s%-11.11s",
					inmu_rec.mu_class,inmu_rec.mu_category);
		strcpy(new_cl,inmu_rec.mu_class);
		strcpy(new_wh,inmu_rec.mu_wh_no);
		strcpy(new_br,inmu_rec.mu_br_no);
	
		if (strcmp(new_group, last_group) || strcmp(new_cl, last_cl) ||
		    strcmp(new_wh, last_wh) || strcmp(new_br, last_br))
		{
			prt_line(last_group, last_br, last_wh);
			strcpy(last_group,new_group);
			clear_totals("L");
		}

		if (strcmp(new_cl, last_cl) || strcmp(new_wh, last_wh) ||
	            strcmp(new_br, last_br))
		{
			prt_cl_tot();
			strcpy(last_cl,new_cl);
			clear_totals("C");
			tot_prnted = TRUE;
		}

		if (strcmp(new_wh, last_wh) || strcmp(new_br, last_br))
		{
			prt_wh_tot();
			strcpy(last_wh,new_wh);
			clear_totals("W");
		}

		if (strcmp(new_br, last_br))
		{
			prt_bt_tot();
			strcpy(last_br,new_br);
			clear_totals("B");
		}
	
		if (tot_prnted)
		{
			fprintf(ftmp, "|==|==|============|");
			fprintf(ftmp, "==================================|");
			fprintf(ftmp, "===========|");
			fprintf(ftmp, "===========|===========|");
			fprintf(ftmp, "===========|===========|");
			fprintf(ftmp, "===========|===========|");
			fprintf(ftmp, "========|========|\n");
			tot_prnted = FALSE;
		}

		sum_inmu();
		cc = find_rec("inmu",&inmu_rec,NEXT,"r");
		dsp_process("Category : ",inmu_rec.mu_category);
	}
	prt_line(last_group, last_br, last_wh);
	prt_cl_tot();
	prt_wh_tot();
	prt_bt_tot();
}

void
sum_inmu (
 void)
{
	if (!strcmp(inmu_rec.mu_period,curr_mnth))
	{
		/*------------------------------
		| Add up Month to date totals. |
		------------------------------*/
		ln_tot[0] += inmu_rec.mu_pur_mty;
		ln_tot[1] += inmu_rec.mu_trin_mty;
		ln_tot[2] += inmu_rec.mu_trout_mty;
		ln_tot[3] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
		ln_tot[5] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);

		cl_tot[0] += inmu_rec.mu_pur_mty;
		cl_tot[1] += inmu_rec.mu_trin_mty;
		cl_tot[2] += inmu_rec.mu_trout_mty;
		cl_tot[3] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
		cl_tot[5] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);

		wh_tot[0] += inmu_rec.mu_pur_mty;
		wh_tot[1] += inmu_rec.mu_trin_mty;
		wh_tot[2] += inmu_rec.mu_trout_mty;
		wh_tot[3] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
		wh_tot[5] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);

		br_tot[0] += inmu_rec.mu_pur_mty;
		br_tot[1] += inmu_rec.mu_trin_mty;
		br_tot[2] += inmu_rec.mu_trout_mty;
		br_tot[3] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
		br_tot[5] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);

		gr_tot[0] += inmu_rec.mu_pur_mty;
		gr_tot[1] += inmu_rec.mu_trin_mty;
		gr_tot[2] += inmu_rec.mu_trout_mty;
		gr_tot[3] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
		gr_tot[5] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
	}
	ln_tot[4] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
	ln_tot[6] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
	cl_tot[4] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
	cl_tot[6] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
	wh_tot[4] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
	wh_tot[6] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
	br_tot[4] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
	br_tot[6] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
	gr_tot[4] += (inmu_rec.mu_sal_mty  - inmu_rec.mu_crd_mty);
	gr_tot[6] += (inmu_rec.mu_icst_mty - inmu_rec.mu_ccst_mty);
}

void
prt_line (
 char *last_cat, 
 char *br_no, 
 char *wh_no)
{
	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_categ_no, "%-11.11s", last_cat + 1);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf(excf_rec.cf_description,"%-40.40s","Category Unknown.");

	fprintf(ftmp, "|%s",br_no);
	fprintf(ftmp, "|%s",wh_no);
	fprintf(ftmp, "|%s",last_cat);
	fprintf(ftmp, "|%-34.34s",excf_rec.cf_description);
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[0]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[1]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[2]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[3]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[4]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[3] - ln_tot[5]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(ln_tot[4] - ln_tot[6]));

	if (ln_tot[3] != 0.00)
		mtd_prf = (((ln_tot[3] - ln_tot[5]) / ln_tot[3]) * 100);
	else
		mtd_prf = 0.00;

	if (ln_tot[4] != 0.00)
		ytd_prf = (((ln_tot[4] - ln_tot[6]) / ln_tot[4]) * 100);
	else
		ytd_prf = 0.00;
	
	fprintf(ftmp, "|%8.2f",mtd_prf);
	fprintf(ftmp, "|%8.2f|\n",ytd_prf);
	
}

void
prt_wh_tot (
 void)
{
	fprintf(ftmp, "|==|==|============|");
	fprintf(ftmp, "==================================|");
	fprintf(ftmp, "===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "========|========|\n");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","            ");
	fprintf(ftmp, "|%34.34s","    **** WAREHOUSE TOTALS ***      ");
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[0]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[1]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[2]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[3]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[4]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[3] - wh_tot[5]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(wh_tot[4] - wh_tot[6]));

	if (wh_tot[3] != 0.00)
		mtd_prf = (((wh_tot[3] - wh_tot[5]) / wh_tot[3]) * 100);
	else
		mtd_prf = 0.00;

	if (wh_tot[4] != 0.00)
		ytd_prf = (((wh_tot[4] - wh_tot[6]) / wh_tot[4]) * 100);
	else
		ytd_prf = 0.00;

	fprintf(ftmp, "|%8.2f",mtd_prf);
	fprintf(ftmp, "|%8.2f|\n",ytd_prf);

}

void
prt_bt_tot (
 void)
{
	fprintf(ftmp, "|==|==|============|");
	fprintf(ftmp, "==================================|");
	fprintf(ftmp, "===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "========|========|\n");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","            ");
	fprintf(ftmp, "|%34.34s","      **** BRANCH TOTALS ***      ");
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[0]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[1]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[2]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[3]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[4]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[3] - br_tot[5]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(br_tot[4] - br_tot[6]));

	if (br_tot[3] != 0.00)
		mtd_prf = (((br_tot[3] - br_tot[5]) / br_tot[3]) * 100);
	else
		mtd_prf = 0.00;

	if (br_tot[4] != 0.00)
		ytd_prf = (((br_tot[4] - br_tot[6]) / br_tot[4]) * 100);
	else
		ytd_prf = 0.00;

	fprintf(ftmp, "|%8.2f",mtd_prf);
	fprintf(ftmp, "|%8.2f|\n",ytd_prf);

}

void
prt_cl_tot (
 void)
{
	fprintf(ftmp, "|==|==|============|");
	fprintf(ftmp, "==================================|");
	fprintf(ftmp, "===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "========|========|\n");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","            ");
	fprintf(ftmp, "|%34.34s","      **** CLASS  TOTALS ***      ");
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[0]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[1]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[2]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[3]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[4]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[3] - cl_tot[5]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(cl_tot[4] - cl_tot[6]));

	if (cl_tot[3] != 0.00)
		mtd_prf = (((cl_tot[3] - cl_tot[5]) / cl_tot[3]) * 100);
	else
		mtd_prf = 0.00;

	if (cl_tot[4] != 0.00)
		ytd_prf = (((cl_tot[4] - cl_tot[6]) / cl_tot[4]) * 100);
	else
		ytd_prf = 0.00;

	fprintf(ftmp, "|%8.2f",mtd_prf);
	fprintf(ftmp, "|%8.2f|\n",ytd_prf);

}

void
prt_totals (
 void)
{
	fprintf(ftmp, "|==|==|============|");
	fprintf(ftmp, "==================================|");
	fprintf(ftmp, "===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "===========|===========|");
	fprintf(ftmp, "========|========|\n");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","  ");
	fprintf(ftmp, "|%s","            ");
	fprintf(ftmp, "|%34.34s","      **** GRAND TOTALS ***       ");
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[0]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[1]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[2]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[3]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[4]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[3] - gr_tot[5]));
	fprintf(ftmp, "|%10.2f ",DOLLARS(gr_tot[4] - gr_tot[6]));

	if (gr_tot[3] != 0.00)
		mtd_prf = (((gr_tot[3] - gr_tot[5]) / gr_tot[3]) * 100);
	else
		mtd_prf = 0.00;

	if (gr_tot[4] != 0.00)
		ytd_prf = (((gr_tot[4] - gr_tot[6]) / gr_tot[4]) * 100);
	else
		ytd_prf = 0.00;

	fprintf(ftmp, "|%8.2f",mtd_prf);
	fprintf(ftmp, "|%8.2f|\n",ytd_prf);
}
