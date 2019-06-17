/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_move.c      )                                 |
|  Program Desc  : ( Print Stock movements.                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, incc,  excf, ccmr,                    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow    | Date Written  : 14/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (14/07/87)      | Modified  by  : Scott Darrow     |
|  Date Modified : (08/11/88)      | Modified  by  : B. C. Lim.       |
|  Date Modified : (04/02/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (27/04/92)      | Modified  by  : Mike Davy.       |
|  Date Modified : (27/09/96)      | Modified  by  : Elizabeth D. Paid|
|                                                                     |
|  Comments      : Tidy up program,remove shutdown_prog() after read_comm |
|                :                                                    |
|                : (04/02/91) Report now has effectively 9 options. It|
|                : can be run by Company, Branch or Warehouse and can |
|                : produce a Detailed, Summary or Intermediate report.|
|                :                                                    |
|  (27/04/92)	 : Modified to supress printing of details when an    |
|                : incc record did not exist ie. totals were equal to |
|                : zero. SC 6694 DPL.	            			      |
|                :                                                    |
|  (27/09/96)	 : Added the Base UOM in the Detailed Report of       |
|                : Company, Branch and Warehouse.                     |
|                                                                     |
| $Log: sk_move.c,v $
| Revision 5.1  2001/08/09 09:19:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:16:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:55  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:34  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:19  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:30:59  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/10/13 02:42:03  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/08 05:32:35  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:18  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_move.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_move/sk_move.c,v 5.1 2001/08/09 09:19:14 scott Exp $";

#define MAX_CCMR 	100

#define		NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>

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
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
	} comm_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_acronym"},
	};

	int ccmr_no_fields = 5;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_cc_no[3];
		long	cm_hhcc_hash;
		char	cm_acronym[10];
	} ccmr_rec;

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
	};

	int inmr_no_fields = 12;

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
		char	mr_cost_flag[2];
		char	mr_sale_unit[5];
	} inmr_rec;

	/*====================================
	| Inventory Cost centre Base Record. |
	====================================*/
	struct dbview incc_list[] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_hhwh_hash"},
		{"incc_sort"},
		{"incc_opening_stock"},
		{"incc_receipts"},
		{"incc_pur"},
		{"incc_issues"},
		{"incc_adj"},
		{"incc_sales"},
		{"incc_closing_stock"},
		{"incc_stat_flag"}
	};

	int incc_no_fields = 12;

	struct {
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		long	cc_hhwh_hash;
		char	cc_sort[29];
		float	cc_opening_stock;
		float	cc_receipts;
		float	cc_pur;
		float	cc_issues;
		float	cc_adj;
		float	cc_sales;
		float	cc_closing_stock;
		char	cc_stat_flag[2];
	} incc_rec;

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
		char	cf_cat_desc[41];
		char	cf_stat_flag[2];
	} excf_rec;

	struct	{
		long	hhcc_hash;
		char	br_no[3];
		char	wh_no[3];
		char	wh_name[10];
		float	item_total[7];
		float	gr_total[7];
		float	al_total[7];
	} whouse[MAX_CCMR];

int	lpno = 1,
	no_in_tab = 0,
	by_company,
	by_branch,
	by_warehouse,
	DTL = FALSE,
	_INT = FALSE,
	SUM = FALSE;

FILE	*ftmp;

float	total_line[7];

char	lower[13], 
		upper[13],
		head_str[36],
		star[2],
		temp_str1[17],
		temp_str2[35],
		temp_str3[5],
		old_class[2],
		old_categ[12]; 

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void com_dtl_rpt (void);
void com_sum_rpt (void);
void bch_dtl_rpt (void);
void bch_sum_rpt (void);
void whs_dtl_rpt (void);
void whs_sum_rpt (void);
void com_int_rpt (void);
void bch_int_rpt (void);
void whs_int_rpt (void);
void load_hhcc (void);
void head_output (void);
void process_data (void);
void print_totals (void);
void print_tot (int all_finished);
void prt_cat_hd (int first_flag);
void prt_gr_tot (int all_finished);
void cal_gr_tot (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int	i;

	lpno = atoi(argv[1]);
	sprintf(lower,"%-12.12s",argv[2]);
	sprintf(upper,"%-12.12s",argv[3]);

	OpenDB();


	init_scr();

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	by_company = FALSE;
	by_branch = FALSE;
	by_warehouse = FALSE;

	/*-----------------------------------
	| Call routines depending on report |
	| required.			    |
	-----------------------------------*/
	i = atoi(argv[4]);
	switch (i)
	{
	case 1:
		com_dtl_rpt();
		break;
	case 2:
		bch_dtl_rpt();
		break;
	case 3:
		whs_dtl_rpt();
		break;
	case 4:
		com_sum_rpt();
		break;
	case 5:
		bch_sum_rpt();
		break;
	case 6:
		whs_sum_rpt();
		break;
	case 7:
		com_int_rpt();
		break;
	case 8:
		bch_int_rpt();
		break;
	case 9:
		whs_int_rpt();
		break;
	default:
		break;
	}

	print_totals();


	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose(ftmp);

	shutdown_prog();
    return (EXIT_SUCCESS);
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

	ReadMisc();

    open_rec("inmr", inmr_list, inmr_no_fields, "inmr_id_no_3");
	open_rec("incc", incc_list, incc_no_fields, "incc_id_no");
	open_rec("excf", excf_list, excf_no_fields, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("inmr");
	abc_fclose("incc");
	abc_fclose("excf");
	abc_dbclose("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("ccmr", ccmr_list, ccmr_no_fields, "ccmr_id_no");

	strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.cm_est_no,comm_rec.test_no);
	strcpy(ccmr_rec.cm_cc_no,comm_rec.tcc_no);
	cc = find_rec("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
}

/*----------------------------
| Detailed report by company |
----------------------------*/
void
com_dtl_rpt (
 void)
{
	by_company = TRUE;
	DTL = TRUE;
	load_hhcc();
	strcpy(head_str, "Detailed report by COMPANY. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*---------------------------
| Summary report by company |
---------------------------*/
void
com_sum_rpt (
 void)
{
	by_company = TRUE;
	SUM = TRUE;
	load_hhcc();
	strcpy(head_str, "Summary report by COMPANY. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*---------------------------
| Detailed report by branch |
---------------------------*/
void
bch_dtl_rpt (
 void)
{
	by_branch = TRUE;
	DTL = TRUE;
	load_hhcc();
	strcpy(head_str, "Detailed report by BRANCH. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*--------------------------
| Summary report by branch |
--------------------------*/
void
bch_sum_rpt (
 void)
{
	by_branch = TRUE;
	SUM = TRUE;
	load_hhcc();
	strcpy(head_str, "Summary report by BRANCH. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*------------------------------
| Detailed report by warehouse |
------------------------------*/
void
whs_dtl_rpt (
 void)
{
	by_warehouse = TRUE;
	DTL = TRUE;
	load_hhcc();
	strcpy(head_str, "Detailed report by WAREHOUSE. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*-----------------------------
| Summary report by warehouse |
-----------------------------*/
void
whs_sum_rpt (
 void)
{
	by_warehouse = TRUE;
	SUM = TRUE;
	load_hhcc();
	strcpy(head_str, "Summary report by WAREHOUSE. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*--------------------------------
| Intermediate report by company |
--------------------------------*/
void
com_int_rpt (
 void)
{
	by_company = TRUE;
	_INT = TRUE;
	load_hhcc();
	strcpy(head_str, "Intermediate Summary by COMPANY. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*-------------------------------
| Intermediate report by branch |
-------------------------------*/
void
bch_int_rpt (
 void)
{
	by_branch = TRUE;
	_INT = TRUE;
	load_hhcc();
	strcpy(head_str, "Intermediate Summary by BRANCH. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*----------------------------------
| Intermediate report by warehouse |
----------------------------------*/
void
whs_int_rpt (
 void)
{
	by_warehouse = TRUE;
	_INT =TRUE;
	load_hhcc();
	strcpy(head_str, "Intermediate Summary by WAREHOUSE. ");
	head_output();
	sprintf(err_str, "Processing : Stock Movements Report. %-35.35s",head_str);
	dsp_screen(clip(err_str), comm_rec.tco_no, comm_rec.tco_name);
	process_data();
}

/*===============================================
| Load hhcc_hash table with valid warehouses	|
===============================================*/
void
load_hhcc (
 void)
{
	int	indx;
	int	loop;

	/*--------------------------------------
	| Initialise table holding hhcc hashes |
	--------------------------------------*/
	for (indx = 0;indx < MAX_CCMR;indx++)
	{
		whouse[indx].hhcc_hash = 0L;
		sprintf(whouse[indx].br_no,"%-2.2s"," ");
		sprintf(whouse[indx].wh_no,"%-2.2s"," ");
		sprintf(whouse[indx].wh_name,"%-10.10s"," ");
		for (loop = 0; loop < 7; loop++)
		{
			whouse[indx].item_total[loop] = 0.00;
			whouse[indx].gr_total[loop] = 0.00;
			whouse[indx].al_total[loop] = 0.00;
		}
	}

	indx = 0;

	/*------------------------
	| Load valid hhcc hashes |
	------------------------*/
	strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.cm_est_no,(by_company) ? "  " : comm_rec.test_no);
	strcpy(ccmr_rec.cm_cc_no, (by_warehouse) ? comm_rec.tcc_no : "  ");
	cc = find_rec("ccmr",&ccmr_rec,GTEQ,"r");
	while (!cc && indx < MAX_CCMR && 
		   !strcmp(ccmr_rec.cm_co_no,comm_rec.tco_no))
	{
		if (by_branch)
		{
			if (strcmp(ccmr_rec.cm_est_no,comm_rec.test_no))
			{
				cc = find_rec("ccmr",&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		if (by_warehouse)
		{
			if (strcmp(ccmr_rec.cm_est_no,comm_rec.test_no) ||
			    strcmp(ccmr_rec.cm_cc_no,comm_rec.tcc_no))
			{
				cc = find_rec("ccmr",&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		whouse[indx].hhcc_hash = ccmr_rec.cm_hhcc_hash;
		sprintf(whouse[indx].br_no,"%-2.2s",ccmr_rec.cm_est_no);
		sprintf(whouse[indx].wh_no,"%-2.2s",ccmr_rec.cm_cc_no);
		sprintf(whouse[indx++].wh_name,"%-10.10s",ccmr_rec.cm_acronym);
		cc = find_rec("ccmr",&ccmr_rec,NEXT,"r");
	}
	no_in_tab = indx;
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	fprintf(ftmp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(ftmp, ".PI12\n");

	fprintf(ftmp, ".LP%d\n",lpno);
	fprintf(ftmp, ".%d\n", (by_company) ? 11 : (by_branch) ? 12 : 13);
	fprintf(ftmp, ".L155\n");

	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".ESTOCK MOVEMENTS REPORT  %s\n", clip(head_str));
	fprintf(ftmp, ".E%s \n",clip(comm_rec.tco_name));
	if (by_branch || by_warehouse)
		fprintf(ftmp, ".EBranch: %s \n",clip(comm_rec.test_name));
	if (by_warehouse)
		fprintf(ftmp, ".EWarehouse: %s \n",clip(comm_rec.tcc_name));
	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".E AS AT : %s\n",SystemTime());

	fprintf(ftmp, ".R===================");
	fprintf(ftmp, "===========================================");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "==============\n");

	fprintf(ftmp, "===================");
	fprintf(ftmp, "===========================================");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "=============");
	fprintf(ftmp, "==============\n");

	if (DTL)
	{
		fprintf(ftmp, "|   ITEM NUMBER    ");
		fprintf(ftmp, "|         ITEM  DESCRIPTION          ");
		fprintf(ftmp, "| UOM ");
	}
	else
	{
		fprintf(ftmp, "| CLASS / CATEGORY ");
		fprintf(ftmp, "|  C A T E G O R Y  D E S C R I P T I O N  ");
	}

	fprintf(ftmp, "|   OPENING  ");
	fprintf(ftmp, "| PURCHASES. ");
	fprintf(ftmp, "|  RECEIPTS  ");
	fprintf(ftmp, "|   ISSUES.  ");
	fprintf(ftmp, "| ADJUSTMENTS");
	fprintf(ftmp, "|   SALES.   ");
	fprintf(ftmp, "| END  STOCK |\n");

	if (DTL)
	{
		fprintf(ftmp, "|                  ");
		fprintf(ftmp, "|                                    ");
		fprintf(ftmp, "|     ");
	}
	else
	{
		fprintf(ftmp, "|    (G R O U P)   ");
		fprintf(ftmp, "|                                          ");
	}
	fprintf(ftmp, "|    STOCK   ");
	fprintf(ftmp, "|            ");
	fprintf(ftmp, "|            ");
	fprintf(ftmp, "|            ");
	fprintf(ftmp, "|            ");
	fprintf(ftmp, "|            ");
	fprintf(ftmp, "|            |\n");

	if (DTL)
	{
		fprintf(ftmp, "|------------------");
		fprintf(ftmp, "|------------------------------------");
		fprintf(ftmp, "|-----");
    }
    else
    {
		fprintf(ftmp, "|------------------");
		fprintf(ftmp, "|------------------------------------------");
    }

	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------");
	fprintf(ftmp, "|------------|\n");
}

/*-----------------
| Process records |
-----------------*/
void
process_data (
 void)
{
	char	old_group[13];
	char	new_group[13];
	int	first_time = TRUE;
	int	all_finished;
	int	i;
	int	j;

	fflush(ftmp);

	/*-----------------------
	|    read first inmr	|
	-----------------------*/
	strcpy(inmr_rec.mr_co_no, comm_rec.tco_no);
	sprintf(inmr_rec.mr_class, "%-1.1s", lower);
	sprintf(inmr_rec.mr_category, "%-11.11s", lower + 1);
	sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");
	cc = find_rec("inmr", &inmr_rec, GTEQ,"r");

	sprintf(old_group,"%-12.12s",lower);
	sprintf(new_group, "%-1.1s%-11.11s", inmr_rec.mr_class, inmr_rec.mr_category);
	while(!cc && strncmp(new_group,upper,12) <= 0 
		  && !(strcmp(inmr_rec.mr_co_no, comm_rec.tco_no)))
	{
		
		if (first_time || strncmp(new_group, old_group, 12))
		{
			sprintf(old_group, "%-12.12s", new_group);

			if (!SUM)
			{
				prt_cat_hd(first_time);
				first_time = FALSE;
			}
		}

		dsp_process(" Item: ", inmr_rec.mr_item_no);
	
		for (i = 0; i < no_in_tab; i++)
		{
			incc_rec.cc_hhbr_hash = inmr_rec.mr_hhbr_hash;
			incc_rec.cc_hhcc_hash = whouse[i].hhcc_hash;
			sprintf(incc_rec.cc_sort,"%-28.28s",lower);
			cc = find_rec("incc",&incc_rec,COMPARISON,"r");
			if (cc)
				continue;


			/*------------------------
			| Store warehouse totals |
			------------------------*/
			whouse[i].item_total[0] = incc_rec.cc_opening_stock;
			whouse[i].item_total[1] = incc_rec.cc_pur;
			whouse[i].item_total[2] = incc_rec.cc_receipts;
			whouse[i].item_total[3] = incc_rec.cc_issues;
			whouse[i].item_total[4] = incc_rec.cc_adj;
			whouse[i].item_total[5] = incc_rec.cc_sales;
			whouse[i].item_total[6] = incc_rec.cc_closing_stock;
				
		}

		/*-----------------------
		| Initialise item total |
		-----------------------*/
		for (i = 0; i < 7; i++)
			total_line[i] = 0.00;

		/*---------------------------------------
		| Calculate item, group and grand total |
		---------------------------------------*/
		for (i = 0; i < no_in_tab; i++)
			for (j = 0; j < 7; j++)
			{
				total_line[j] += whouse[i].item_total[j];
				whouse[i].gr_total[j] += whouse[i].item_total[j];
				whouse[i].al_total[j] += whouse[i].item_total[j];
				whouse[i].item_total[j] = 0.00;
			}
		if (DTL)
		{
			sprintf(temp_str1, "%-16.16s",inmr_rec.mr_item_no);
			sprintf(temp_str2, "%-34.34s",inmr_rec.mr_description);
			sprintf(temp_str3, "%-4.4s",  inmr_rec.mr_sale_unit);

			print_tot(0);
		}
	
		/*---------------------------------
		| Store last class/category for a |
		| summary report.		  |
		---------------------------------*/
		sprintf(old_class, "%-1.1s", inmr_rec.mr_class);
		sprintf(old_categ, "%-11.11s", inmr_rec.mr_category);

		cc = find_rec("inmr", &inmr_rec, NEXT,"r");
		sprintf(new_group, "%-1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);
		if (cc || strncmp(new_group, old_group, 12))
		{
			all_finished = FALSE;
			if (cc || strncmp(new_group,upper,12) > 0
		  	       || (strcmp(inmr_rec.mr_co_no, comm_rec.tco_no)))
				all_finished = TRUE;

			prt_gr_tot(all_finished);
		}
	}
}

/*-------------------------------
| Print totals at end of report |
-------------------------------*/
void
print_totals (
 void)
{
	int	i;
	int	j;

	/*-----------------------
	| Initialise total_line |
	-----------------------*/
	for (j = 0; j < 7; j++)
		total_line[j] = 0.00;

	if (DTL || SUM)
	{
		/*------------------------
		| Calculate grand totals |
		------------------------*/
		for (i = 0; i < no_in_tab; i++)
			for (j = 0; j < 7; j++)
				total_line[j] += whouse[i].al_total[j];
	    if (DTL)
        {
			sprintf(temp_str1, "%-16.16s", " ");
			sprintf(temp_str2, "%-34.34s", "G R A N D   T O T A L ");
			sprintf(temp_str3, "%-4.4s", " ");
        }
        else
        {
			sprintf(temp_str1, "%-16.16s", " ");
			sprintf(temp_str2, "%-40.40s", "G R A N D   T O T A L ");
        }

		print_tot(FALSE);
	}
	
	if (_INT)
	{
		if (!by_warehouse)
		{
			/*------------------------
			| Calculate grand totals |
			------------------------*/
			for (i = 0; i < no_in_tab; i++)
			{
				for (j = 0; j < 7; j++)
				{
					total_line[j] = whouse[i].al_total[j];
				}
				
				if (by_company)
				{
					sprintf(temp_str1, "Br : %-2.2s Wh : %-2.2s",whouse[i].br_no, whouse[i].wh_no);
					sprintf(temp_str2, "Warehouse Name: %-9.9s",whouse[i].wh_name);
				}
	
				if (by_branch)
				{
					sprintf(temp_str1, "Warehouse No: %-2.2s",whouse[i].wh_no);
					sprintf(temp_str2, "Warehouse Name: %-9.9s",whouse[i].wh_name);
				}
	
				print_tot(FALSE);
			}
		}
		/*-----------------------
		| Initialise total_line |
		-----------------------*/
		for (j = 0; j < 7; j++)
			total_line[j] = 0.00;
	
		/*-------------------
		| Print Grand Total |
		-------------------*/
		for (i = 0; i < no_in_tab; i++)
			for (j = 0; j < 7; j++)
				total_line[j] += whouse[i].al_total[j];
		sprintf(temp_str1, "%-16.16s", " ");
		sprintf(temp_str2, "%-40.40s", "G R A N D   T O T A L ");
		print_tot(FALSE);
	}
        fprintf(ftmp,".EOF\n");
}

/*---------------------------------
| Print one detail line of report |
---------------------------------*/
void
print_tot (
 int all_finished)
{
	if (all_finished)
	{
		if (_INT)
			fprintf(ftmp, ".LRP%d\n", no_in_tab + 2);
		else
			fprintf(ftmp, ".LRP5\n");
	}
	else
	{
		if (	total_line[0] == 0.00 &&
			total_line[1] == 0.00 &&
			total_line[2] == 0.00 &&
			total_line[3] == 0.00 &&
			total_line[4] == 0.00 &&
			total_line[5] == 0.00 &&
			total_line[6] == 0.00)

			return;

		if (!_INT)
			fprintf(ftmp, ".LRP3\n");
	}

	if (total_line[5] == 0.00)
		strcpy(star, " ");
	else
		strcpy(star, "*");
    if (DTL)
    {
		fprintf(ftmp, "| %-16.16s ",temp_str1);
		fprintf(ftmp, "| %-34.34s ",temp_str2);
		fprintf(ftmp, "| %-4.4s",  temp_str3);
    }
    else
    {
		fprintf(ftmp, "| %-16.16s ",temp_str1);
		fprintf(ftmp, "| %-40.40s ",temp_str2);
    }

	fprintf(ftmp, "|%12.2f",total_line[0]);
	fprintf(ftmp, "|%12.2f",total_line[1]);
	fprintf(ftmp, "|%12.2f",total_line[2]);
	fprintf(ftmp, "|%12.2f",total_line[3]);
	fprintf(ftmp, "|%12.2f",total_line[4]);
	fprintf(ftmp, "|%12.2f",total_line[5]);
	fprintf(ftmp, "|%12.2f|%s\n",total_line[6], star); 

}

/*------------------------
| Print Category heading |
------------------------*/
void
prt_cat_hd (
 int first_flag)
{

	strcpy (excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf (excf_rec.cf_categ_no, "%-11.11s", inmr_rec.mr_category);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
		strcpy(excf_rec.cf_cat_desc, "No Description Found For Group");

	sprintf(err_str, "%-40.40s", excf_rec.cf_cat_desc);

	if (_INT)
	{
		fprintf(ftmp, ".LRP%d\n", no_in_tab + 2);
		fprintf(ftmp, "| %-1.1s / %-11.11s  ", inmr_rec.mr_class, inmr_rec.mr_category);
		fprintf(ftmp, "| %-40.40s ", err_str);
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            |\n");
	}

	if (DTL)
	{
		fprintf(ftmp, ".PD| %-1.1s / %-11.11s  ", inmr_rec.mr_class, inmr_rec.mr_category);
		fprintf(ftmp, "| %-34.34s ", err_str);
		fprintf(ftmp, "|     ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            ");
		fprintf(ftmp, "|            |\n");
	}

	if (!first_flag && DTL)
		fprintf(ftmp, ".PA\n");
}

/*-----------------------
| Print category totals |
-----------------------*/
void
prt_gr_tot (
 int all_finished)
{
	int	i;
	int	j;
	int	lcl_cc;


	if (_INT)
	{
		if (!by_warehouse)
		{
			for (i = 0; i < no_in_tab; i++)
			{
				for (j = 0; j < 7; j++)
					total_line[j] = whouse[i].gr_total[j];

				if (by_company)
				{
					sprintf(temp_str1, "Br : %-2.2s Wh : %-2.2s",whouse[i].br_no, whouse[i].wh_no);
					sprintf(temp_str2, "Warehouse Name: %-9.9s",whouse[i].wh_name);
	
				}

				if (by_branch)
				{
					sprintf(temp_str1, "Warehouse No: %-2.2s",whouse[i].wh_no);
					sprintf(temp_str2, "Warehouse Name: %-9.9s",whouse[i].wh_name);
				}
	
				print_tot(all_finished);
			}
		}

		cal_gr_tot();
		
		sprintf(temp_str1, "%-16.16s", " ");
		sprintf(temp_str2, "%-40.40s", "         CLASS / CATEGORY  TOTAL        ");
		print_tot(all_finished);
	}

	/*-------------------------------
	| Calculate group total for a	|
	| DTL or SUM report		|
	-------------------------------*/
	cal_gr_tot();

	if (DTL)
	{
		sprintf(temp_str1, "%-16.16s", " ");
		sprintf(temp_str2, "%-34.34s", "      CLASS / CATEGORY  TOTAL     ");
		sprintf(temp_str3, "%-4.4s", " ");
		print_tot(all_finished);
	}

	if (SUM)
	{
		strcpy (excf_rec.cf_co_no, comm_rec.tco_no);
		sprintf (excf_rec.cf_categ_no, "%-11.11s", old_categ);
		lcl_cc = find_rec("excf", &excf_rec, COMPARISON, "r");
		if (lcl_cc)
			strcpy(excf_rec.cf_cat_desc, "No Description Found For Group");

		sprintf(temp_str1, "%-1.1s / %-11.11s", old_class, old_categ);
		sprintf(temp_str2, "%-40.40s", excf_rec.cf_cat_desc);
		print_tot(all_finished);
	}

	if (all_finished)
	{
        if (DTL)
        {
			fprintf(ftmp, "|==================");
			fprintf(ftmp, "|====================================");
			fprintf(ftmp, "|=====");
        }
        else
        {
			fprintf(ftmp, "|==================");
			fprintf(ftmp, "|==========================================");
        }
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============");
		fprintf(ftmp, "|============|\n");
	}	
	else
	{
		if (_INT)
		{
			fprintf(ftmp, "|                  ");
			fprintf(ftmp, "|                                          ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            ");
			fprintf(ftmp, "|            |\n");
		}
	}
}

/*--------------------------------------
| Calculate group totals for new group |
--------------------------------------*/
void
cal_gr_tot (
 void)
{
	int	i;
	int	j;

	for (j = 0; j < 7; j++)
		total_line[j] = 0.00;

	for (i = 0; i < no_in_tab; i++)
		for (j = 0; j < 7; j++)
		{
			total_line[j] += whouse[i].gr_total[j];
			whouse[i].gr_total[j] = 0.00;
		}
}
