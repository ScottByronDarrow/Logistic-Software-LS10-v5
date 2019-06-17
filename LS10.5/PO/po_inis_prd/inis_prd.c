/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( po_inis_prd.c )                                  |
|  Program Desc  : ( Print X-Ref Report By Product Group.         )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 03/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (17/03/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (06/09/93)      | Modified  by  : Scott B Darrow.  |
|                                                                     |
|  Comments      : Change Printing of inis_land_cst  DOLLARS.         |
|  (06/09/93)    : HGP 9485. Updated for lead times.                  |
|                :                                                    |
|                                                                     |
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: inis_prd.c,v $
| Revision 5.1  2001/08/09 09:15:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:11:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:44  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:18  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  2000/02/18 02:17:45  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.10  1999/12/06 01:32:36  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/09/29 10:11:57  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:38:02  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:25  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inis_prd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_inis_prd/inis_prd.c,v 5.1 2001/08/09 09:15:40 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>

#ifndef	LINUX
extern	int	errno;
#endif	/* LINUX */

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tinv_date;
	} comm_rec;

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
		{"sumr_curr_code"},
	};

	int sumr_no_fields = 7;

	struct {
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_name[41];
		char	sm_acronym[10];
		char	sm_curr_code[4];
	} sumr_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
	};

	int inmr_no_fields = 6;

	struct {
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
	} inmr_rec;

	/*==================================+
	 | Stock Inventory Supplier Record. |
	 +==================================*/
#define	INIS_NO_FIELDS	26

	struct dbview	inis_list [INIS_NO_FIELDS] =
	{
		{"inis_co_no"},
		{"inis_br_no"},
		{"inis_wh_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_part"},
		{"inis_sup_priority"},
		{"inis_hhis_hash"},
		{"inis_fob_cost"},
		{"inis_lcost_date"},
		{"inis_duty"},
		{"inis_licence"},
		{"inis_sup_uom"},
		{"inis_pur_conv"},
		{"inis_min_order"},
		{"inis_norm_order"},
		{"inis_ord_multiple"},
		{"inis_pallet_size"},
		{"inis_lead_time"},
		{"inis_sea_time"},
		{"inis_air_time"},
		{"inis_lnd_time"},
		{"inis_dflt_lead"},
		{"inis_weight"},
		{"inis_volume"},
		{"inis_stat_flag"}
	};

	struct tag_inisRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_part [17];
		char	sup_priority [3];
		long	hhis_hash;
		double	fob_cost;
		Date	lcost_date;
		char	duty [3];
		char	licence [3];
		long	sup_uom;
		float	pur_conv;
		float	min_order;
		float	norm_order;
		float	ord_multiple;
		float	pallet_size;
		float	lead_time;
		float	sea_time;
		float	air_time;
		float	lnd_time;
		char	dflt_lead [2];
		float	weight;
		float	volume;
		char	stat_flag [2];
	}	inis_rec;

	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
#define	INUM_NO_FIELDS	2

	struct dbview	inum_list [INUM_NO_FIELDS] =
	{
		{"inum_hhum_hash"},
		{"inum_uom"},
	};

	struct tag_inumRecord
	{
		long	hhum_hash;
		char	uom [5];
	}	inum_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_hhcf_hash"},
		{"excf_cat_desc"},
	};

	int excf_no_fields = 4;

	struct {
		char	cf_co_no[3];
		char	cf_cat_no[12];
		long	cf_hhcf_hash;
		char	cf_cat_desc[41];
	} excf_rec;


	int	lpno;
		
	int	first_time = 1;

	char	lower[13];
	char	upper[13];
	char	old_gp[13];
	char	new_gp[13];

	FILE	*fout;


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void head_output (void);
void process (void);
void get_inis (long hhbr_hash);
void print_group (int first_time);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc < 4)
	{
		print_at(0,0,mlPoMess722,argv[0]);
        return (EXIT_FAILURE);
	}
	
	lpno = atoi(argv[1]);
	sprintf(lower,"%-12.12s",argv[2]);
	sprintf(upper,"%-12.12s",argv[3]);

	init_scr();

	OpenDB();

	dsp_screen("Processing : Inventory Supplier X-Ref By Prod Gp.", 
					comm_rec.tco_no, comm_rec.tco_name);

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in opening pformat During (POPEN)",errno,PNAME);
	
	head_output();

	process();

	fprintf(fout, ".EOF\n");
	pclose(fout);
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_id_no_3");
	open_rec("inis", inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	open_rec("sumr", sumr_list, sumr_no_fields, "sumr_hhsu_hash");
	open_rec("excf", excf_list, excf_no_fields, "excf_id_no");
	open_rec("inum", inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("inmr");
	abc_fclose("inis");
	abc_fclose("sumr");
	abc_fclose("excf");
	abc_fclose("inum");
	abc_dbclose("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(fout, ".LP%d\n",lpno);
	fprintf(fout, ".14\n");
	fprintf(fout, ".PI16\n");
	fprintf(fout, ".L162\n");
	fprintf(fout, ".EINVENTORY SUPPLIER XREF BY PRODUCT GROUP REPORT\n");
	fprintf(fout, ".B1\n");
	fprintf(fout, ".E%s\n", clip(comm_rec.tco_name));
	fprintf(fout, ".B1\n");
	fprintf(fout, ".E%s\n", clip(comm_rec.test_name));
	fprintf(fout, ".B1\n");
	fprintf(fout, ".EAS AT %-24.24s\n",SystemTime());

	fprintf(fout, ".R=================");
	fprintf(fout, "============================");
	fprintf(fout, "=======");
	fprintf(fout, "==========");
	fprintf(fout, "=================");
	fprintf(fout, "====");
	fprintf(fout, "=====");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "=====");
	fprintf(fout, "=====");
	fprintf(fout, "=====");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "============\n");

	fprintf(fout, "=================");
	fprintf(fout, "============================");
	fprintf(fout, "=======");
	fprintf(fout, "==========");
	fprintf(fout, "=================");
	fprintf(fout, "====");
	fprintf(fout, "=====");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "=====");
	fprintf(fout, "=====");
	fprintf(fout, "=====");
	fprintf(fout, "===========");
	fprintf(fout, "===========");
	fprintf(fout, "============\n");

	fprintf(fout, "!   ITEM NUMBER  ");
	fprintf(fout, "!      ITEM DESCRIPTION     ");
	fprintf(fout, "! SUPP ");
	fprintf(fout, "! SUPPLIER");
	fprintf(fout, "!  SUPPLIER ITEM ");
	fprintf(fout, "!PRI");
	fprintf(fout, "!CURR");
	fprintf(fout, "! SUPPLIER ");
	fprintf(fout, "!LAST  COST");
	fprintf(fout, "!DUTY");
	fprintf(fout, "!LIC.");
	fprintf(fout, "!UOM.");
	fprintf(fout, "!MIN  ORDER");
	fprintf(fout, "!NORM ORDER");
	fprintf(fout, "!LEAD  TIME!\n");

	fprintf(fout, "!                ");
	fprintf(fout, "!                           ");
	fprintf(fout, "!NUMBER");
	fprintf(fout, "! ACRONYM ");
	fprintf(fout, "!     NUMBER     ");
	fprintf(fout, "!   ");
	fprintf(fout, "!    ");
	fprintf(fout, "!   PRICE  ");
	fprintf(fout, "!   DATE   ");
	fprintf(fout, "!CODE");
	fprintf(fout, "!CODE");
	fprintf(fout, "!    ");
	fprintf(fout, "!   QTY    ");
	fprintf(fout, "!   QTY    ");
	fprintf(fout, "!   DAYS   !\n");

	fprintf(fout, "!----------------");
	fprintf(fout, "!---------------------------");
	fprintf(fout, "!------");
	fprintf(fout, "!---------");
	fprintf(fout, "!----------------");
	fprintf(fout, "!---");
	fprintf(fout, "!----");
	fprintf(fout, "!----------");
	fprintf(fout, "!----------");
	fprintf(fout, "!----");
	fprintf(fout, "!----");
	fprintf(fout, "!----");
	fprintf(fout, "!----------");
	fprintf(fout, "!----------");
	fprintf(fout, "!----------!\n");

	fflush(fout);
}

void
process (
 void)
{
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_class,"%-1.1s",lower);
	sprintf(inmr_rec.mr_category,"%-11.11s",lower + 1);
	sprintf(inmr_rec.mr_item_no,"%-16.16s"," ");

	cc = find_rec("inmr",&inmr_rec,GTEQ,"r");
	sprintf(old_gp,"%-1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);

	while (!cc && !strcmp(inmr_rec.mr_co_no,comm_rec.tco_no))
	{
		sprintf(new_gp,"%-1.1s%-11.11s",inmr_rec.mr_class,inmr_rec.mr_category);
		if (strcmp(new_gp,upper) > 0)
			break;

		dsp_process("Item No : ",inmr_rec.mr_item_no);

		get_inis(inmr_rec.mr_hhbr_hash);

		cc = find_rec("inmr",&inmr_rec,NEXT,"r");
	}
}

void
get_inis (
 long hhbr_hash)
{
	cc = find_hash("inis",&inis_rec,GTEQ,"r",hhbr_hash);
	while (!cc && inis_rec.hhbr_hash == hhbr_hash)
	{
		cc = find_hash("sumr",&sumr_rec,COMPARISON,"r",inis_rec.hhsu_hash);
		if (cc)
			strcpy(sumr_rec.sm_crd_no,"      ");

		if (first_time || strcmp(new_gp,old_gp))
		{
			print_group(first_time);
			strcpy(old_gp,new_gp);
			first_time = 0;
		}

		inum_rec.hhum_hash	=	inis_rec.sup_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		if (cc)
			strcpy (inum_rec.uom, "????");

		fprintf(fout,"!%16.16s",inmr_rec.mr_item_no);
		fprintf(fout,"!%-27.27s",inmr_rec.mr_description);
		fprintf(fout,"!%6.6s",sumr_rec.sm_crd_no);
		fprintf(fout,"!%9.9s",sumr_rec.sm_acronym);
		fprintf(fout,"!%16.16s",inis_rec.sup_part);
		fprintf(fout,"!%-2.2s ",inis_rec.sup_priority);
		fprintf(fout,"!%-3.3s ",sumr_rec.sm_curr_code);
		fprintf(fout,"!%9.2f ",inis_rec.fob_cost);
		fprintf(fout,"!%10.10s",DateToString(inis_rec.lcost_date));
		fprintf(fout,"! %-2.2s ",inis_rec.duty);
		fprintf(fout,"! %-2.2s ",inis_rec.licence);
		fprintf(fout,"!%-4.4s",inum_rec.uom);
		fprintf(fout,"!%9.2f ",inis_rec.min_order);
		fprintf(fout,"!%9.2f ",inis_rec.norm_order);
		fprintf(fout,"!%9.2f !\n",inis_rec.lead_time);

		cc = find_hash("inis",&inis_rec,NEXT,"r",hhbr_hash);
	}
}

void
print_group (
 int first_time)
{
	char	gp_str[100];
	char	exp_str[200];

	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	strcpy(excf_rec.cf_cat_no,new_gp + 1);
	cc = find_rec("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		strcpy(excf_rec.cf_cat_desc,"No Description found.");
	sprintf(gp_str,"%-12.12s - %-40.40s",new_gp,excf_rec.cf_cat_desc);

	expand(exp_str,gp_str);

	fprintf(fout, ".PD!%-157.157s!\n",exp_str);
	if ( !first_time )
		fprintf(fout, ".PA\n");
}
