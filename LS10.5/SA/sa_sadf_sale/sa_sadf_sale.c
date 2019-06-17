/*=====================================================================
|  Copyright (C) 1988 - 1991 Pinnacle Software Limited.               |
|=====================================================================|
|  Program Name  : ( sa_sadf_sale.c )                                 |
|  Program Desc  : ( Create sale from sadf.                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sadf, sale,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sale,     ,     ,     ,     ,     ,     ,         |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 11/12/91         |
|---------------------------------------------------------------------|
|  Date Modified : (10/07/92)      | Modified  by  : Simon Dubey.     |
|                                                                     |
|  Comments      : (10/07/92) -  To accept debtors range of parameters|
|                :               passed from sa_update.i SC DFH 6303. |
|                                                                     |
| $Id: sa_sadf_sale.c,v 5.2 2001/08/09 09:17:13 scott Exp $
| $Log: sa_sadf_sale.c,v $
| Revision 5.2  2001/08/09 09:17:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:52  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:03  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.2  2000/07/14 05:15:09  scott
| Updated for small warning error under linux
|
| Revision 1.1  2000/03/04 05:03:11  ambhet
| SC# 15059 / 2600 - Added program name in the v9.10
|
| Revision 1.1.1.1  1998/01/22 00:58:20  jonc
| Version 10 start
|
| Revision 2.1  1998/01/16 00:09:05  kirk
| PSL 14546 - Update version numbering
|
| Revision 1.1  1998/01/16 00:08:25  kirk
| PSL 14546 - Brought from version 7
|
|
=====================================================================*/
#define CCMAIN
char    *PNAME = "$RCSfile: sa_sadf_sale.c,v $",
        *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf_sale/sa_sadf_sale.c,v 5.2 2001/08/09 09:17:13 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>

	/*==================================
	| file comm { System Common file } |
	==================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_dp_no"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tcc_no[3];
		char	tdp_no[3];
		long	tdbt_date;
	} comm_rec;

	/*===============================================
	| Sales Analysis By Customer/Category/Salesman. |
	===============================================*/
	struct dbview sale_list[] ={
		{"sale_key"},
		{"sale_category"},
		{"sale_sman"},
		{"sale_area"},
		{"sale_ctype"},
		{"sale_dbt_no"},
		{"sale_year_flag"},
		{"sale_period"},
		{"sale_units"},
		{"sale_gross"},
		{"sale_cost_sale"},
		{"sale_disc"}
	};

	int sale_no_fields = 12;

	struct {
		char 	sa_key[9];
		char 	sa_category[12];
		char 	sa_sman[3];
		char 	sa_area[3];
		char 	sa_ctype[4];
		char 	sa_dbt_no[7];
		char 	sa_year_flag[2];
		char 	sa_period[3];
		double 	sa_units;
		double 	sa_gross;	/*  Money field  */
		double 	sa_cost_sale;	/*  Money field  */
		double 	sa_disc;	/*  Money field  */
	} sale_rec;

	/*==========================================
	| file inmr {Inventory Master Base Record} |
	==========================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_category"},
	};

	int inmr_no_fields = 4;

	struct {
		char 	mr_co_no[3];
		long 	mr_hhbr_hash;
		char 	mr_class[2];
		char 	mr_category[12];
	} inmr_rec;


	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_class_type"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
	};

	int	cumr_no_fields = 7;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_class_type[4];
		char	cm_area_code[3];
		char	cm_sman_code[3];
	} cumr_rec;

	/*=============================================
	| Sales Analysis Detail file By Item/Customer |
	=============================================*/
	struct dbview sadf_list[] ={
		{"sadf_co_no"},
		{"sadf_br_no"},
		{"sadf_year"},
		{"sadf_hhbr_hash"},
		{"sadf_hhcu_hash"},
		{"sadf_qty_per1"},
		{"sadf_qty_per2"},
		{"sadf_qty_per3"},
		{"sadf_qty_per4"},
		{"sadf_qty_per5"},
		{"sadf_qty_per6"},
		{"sadf_qty_per7"},
		{"sadf_qty_per8"},
		{"sadf_qty_per9"},
		{"sadf_qty_per10"},
		{"sadf_qty_per11"},
		{"sadf_qty_per12"},
		{"sadf_sal_per1"},
		{"sadf_sal_per2"},
		{"sadf_sal_per3"},
		{"sadf_sal_per4"},
		{"sadf_sal_per5"},
		{"sadf_sal_per6"},
		{"sadf_sal_per7"},
		{"sadf_sal_per8"},
		{"sadf_sal_per9"},
		{"sadf_sal_per10"},
		{"sadf_sal_per11"},
		{"sadf_sal_per12"},
		{"sadf_cst_per1"},
		{"sadf_cst_per2"},
		{"sadf_cst_per3"},
		{"sadf_cst_per4"},
		{"sadf_cst_per5"},
		{"sadf_cst_per6"},
		{"sadf_cst_per7"},
		{"sadf_cst_per8"},
		{"sadf_cst_per9"},
		{"sadf_cst_per10"},
		{"sadf_cst_per11"},
		{"sadf_cst_per12"},
		{"sadf_sman"},
		{"sadf_area"},
	};

	int	sadf_no_fields = 43;

	struct	{
		char	df_co_no[3];
		char	df_br_no[3];
		char	df_year[2];
		long	df_hhbr_hash;
		long	df_hhcu_hash;
		float	df_qty_per[12];
		double	df_sal_per[12];
		double	df_cst_per[12];
		char	df_sman[3];
		char	df_area[3];
	} sadf_rec;

	struct {
		char	st_dbt_no[7];
		char	end_dbt_no[7];
	} local_rec;

void OpenDB(void);
void CloseDB(void);
void process(void);
int  UpdSale(int);
void shutdown_prog(void);

char	*data = "data";

/*========================== 
| Main Processing Routine. |
==========================*/
int 
main (
	int	argc,
	char* argv[])
{

	if (argc < 3)
	{
		printf(" USAGE: <Start Customer> <End Customer> \007\n");
		exit(0);
	}

	strcpy( local_rec.st_dbt_no , argv[1] );
	strcpy( local_rec.end_dbt_no, argv[2] );

/*	read_comm();*/

	dsp_screen("Creating sale from sadf",comm_rec.tco_no,comm_rec.tco_name);

/*	open_db();*/
	OpenDB();

	process();

	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB(void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );	

	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec("sale", sale_list, sale_no_fields, "sale_id_no");
	open_rec("sadf", sadf_list, sadf_no_fields, "sadf_id_no");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB(void)
{
	abc_fclose("inmr");
	abc_fclose("cumr");
	abc_fclose("sale");
	abc_fclose("sadf");
	abc_dbclose(data);

}

/*=============================================
| Get common info from commom database file . |
=============================================*/
/*
read_comm()
{
	abc_dbopen(data);

	comm_rec.termno = ttyslt();

	open_rec("comm", comm_list, comm_no_fields, "comm_term");

    	while ((cc = dbfind("comm",COMPARISON,&comm_rec.termno,&length,&comm_rec)) == RECLOCK || cc == LOCKDENIED || cc == FILELOCK || cc == 36)
		sleep(1);

	if (cc) 
		sys_err("Error in comm During (DBFIND)", cc, PNAME);

	abc_fclose("comm");
}
*/
void
process (void)
{	
	int	i;

	strcpy(sadf_rec.df_co_no, "  ");
	strcpy(sadf_rec.df_br_no, "  ");
	strcpy(sadf_rec.df_year, " ");
	sadf_rec.df_hhbr_hash = 0L;
	sadf_rec.df_hhcu_hash = 0L;
	strcpy(sadf_rec.df_sman, "  ");
	strcpy(sadf_rec.df_area, "  ");
	cc = find_rec("sadf", &sadf_rec, GTEQ, "r");
	while (!cc)
	{
		cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",sadf_rec.df_hhcu_hash);
		if (cc)
		{
			cc = find_rec("sadf", &sadf_rec, NEXT, "r");
			continue;
		}

		if (strcmp (cumr_rec.cm_dbt_no , local_rec.st_dbt_no) < 0 ||
                    strcmp (cumr_rec.cm_dbt_no , local_rec.end_dbt_no) > 0)
		{
			cc = find_rec("sadf", &sadf_rec, NEXT, "r");
			continue;
		}

		cc = find_hash("inmr",&inmr_rec,COMPARISON,"r",sadf_rec.df_hhbr_hash);
		if (cc)
		{
			cc = find_rec("sadf", &sadf_rec, NEXT, "r");
			continue;
		}


		dsp_process("Customer :", cumr_rec.cm_dbt_no);

		for (i = 0; i < 12; i++)
		{
			if (sadf_rec.df_qty_per[i] != 0.00 ||
			    sadf_rec.df_sal_per[i] != 0.00 ||
			    sadf_rec.df_cst_per[i] != 0.00)
			{
				/*---------------------------
				| Add or update sale record |
				---------------------------*/
				UpdSale(i);
			}
		}

		cc = find_rec("sadf", &sadf_rec, NEXT, "r");
	}

	return;
}

/*---------------------------------
| Get and update or Add Inventory |
| Movements Transactions record.  |
---------------------------------*/
int UpdSale(
int	per)
{
	sprintf(sale_rec.sa_key, 
		"%2.2s%2.2s 1 1",
		sadf_rec.df_co_no,
		sadf_rec.df_br_no);
	strcpy(sale_rec.sa_category, inmr_rec.mr_category);
	strcpy(sale_rec.sa_sman, sadf_rec.df_sman);
	strcpy(sale_rec.sa_area, sadf_rec.df_area);
	strcpy(sale_rec.sa_ctype, cumr_rec.cm_class_type);
	strcpy(sale_rec.sa_dbt_no, cumr_rec.cm_dbt_no);
	if (!strcmp (sadf_rec.df_year, "C") ||
	    !strcmp (sadf_rec.df_year, "c"))
		strcpy(sale_rec.sa_year_flag, "C");
	else
		strcpy(sale_rec.sa_year_flag, "L");
	/*strcpy(sale_rec.sa_year_flag, sadf_rec.df_year);*/
	sprintf(sale_rec.sa_period, "%02d", per + 1);
	cc = find_rec("sale",&sale_rec, COMPARISON, "u");
	if (cc)
	{
		sale_rec.sa_units = sadf_rec.df_qty_per[per];
		sale_rec.sa_gross = CENTS(sadf_rec.df_sal_per[per]);
		sale_rec.sa_cost_sale = CENTS(sadf_rec.df_cst_per[per]);
		sale_rec.sa_disc  = 0.00;
		cc = abc_add("sale",&sale_rec);
		if (cc) 
		      sys_err("Error in sale During (DBADD)",cc,PNAME);
	}
	else 
	{
		sale_rec.sa_units += sadf_rec.df_qty_per[per];
		sale_rec.sa_gross += CENTS(sadf_rec.df_sal_per[per]);
		sale_rec.sa_cost_sale += CENTS(sadf_rec.df_cst_per[per]);
		sale_rec.sa_disc  = 0.00;
		cc = abc_update("sale",&sale_rec);
		if (cc) 
			sys_err("Error in sale During (DBUPDATE)",cc,PNAME);
	}
	abc_unlock("sale");
	return(per);
}
