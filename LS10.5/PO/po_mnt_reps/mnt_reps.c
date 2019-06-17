/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( po_dtrept.c    )                                 |
|  Program Desc  : ( Purchase Order Duty File Listing.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  podt,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21/09/90         |
|---------------------------------------------------------------------|
|  Date Modified : (07/09/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (30/11/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (15/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      : New program to replace : po_cfrept.c , po_crrept   |
|                : po_dtrept, po_lhrept                               |
|                :                                                    |
|  (07/09/93)    : HGP 9485 Updated for lead times.                   |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|                :                                                    |
|                                                                     |
| $Log: mnt_reps.c,v $
| Revision 5.2  2001/08/09 09:15:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:43  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:49  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:22  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/09/29 10:12:03  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/21 04:38:05  scott
| Updated from Ansi project
|
| Revision 1.5  1999/06/17 10:06:29  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mnt_reps.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_mnt_reps/mnt_reps.c,v 5.2 2001/08/09 09:15:47 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#define	BY_CURRENCY	( rep_type[0] == 'C' )
#define	BY_FREIGHT	( rep_type[0] == 'F' )
#define	BY_LICENCE	( rep_type[0] == 'L' )
#define	BY_DUTY		( rep_type[0] == 'D' )

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	lpno = 1;	

	char	rep_type[2];

	FILE	*pout;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"}
		};
	
	int comm_no_fields = 4;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
	} comm_rec;

	/*=======================
	| Currency File Record. |
	=======================*/
	struct dbview pocr_list[] ={
		{"pocr_co_no"},
		{"pocr_code"},
		{"pocr_description"},
		{"pocr_ex1_factor"},
		{"pocr_ex2_factor"},
		{"pocr_ex3_factor"},
		{"pocr_ex4_factor"},
		{"pocr_ex5_factor"},
		{"pocr_ex6_factor"},
		{"pocr_ex7_factor"},
		{"pocr_ldate_up"},
		{"pocr_stat_flag"}
	};

	int pocr_no_fields = 12;

	struct {
		char	cr_co_no[3];
		char	cr_code[4];
		char	cr_description[41];
		double	cr_exch_rate[7];
		long	cr_ldate_up;
		char	cr_stat_flag[2];
	} pocr_rec;

	/*===========================
	| Composition or Duty File. |
	===========================*/
	struct dbview podt_list[] ={
		{"podt_co_no"},
		{"podt_code"},
		{"podt_description"},
		{"podt_im_tariff"},
		{"podt_im_band"},
		{"podt_ex_tariff"},
		{"podt_ex_band"},
		{"podt_duty_type"},
		{"podt_im_duty"},
		{"podt_ex_draw"}
	};

	int podt_no_fields = 10;

	struct {
		char	dt_co_no[3];
		char	dt_code[3];
		char	dt_description[21];
		char	dt_im_tariff[11];
		char	dt_im_band[2];
		char	dt_ex_tariff[11];
		char	dt_ex_band[2];
		char	dt_duty_type[2];
		double	dt_im_duty;
		double	dt_ex_draw;
	} podt_rec;

	/*=========================
	| Country / Freight File. |
	=========================*/
	struct dbview pocf_list[] ={
		{"pocf_co_no"},
		{"pocf_code"},
		{"pocf_description"},
		{"pocf_load_type"},
		{"pocf_freight_load"},
		{"pocf_lead_time"},
		{"pocf_last_update"}
	};

	int pocf_no_fields = 7;

	struct {
		char	cf_co_no[3];
		char	cf_code[4];
		char	cf_description[21];
		char	cf_load_type[2];
		double	cf_freight_load;
		int		cf_lead_time;
		long	cf_last_update;
	} pocf_rec;

	/*=================================
	| Purchase order licence control. |
	=================================*/
	struct dbview polh_list[] ={
		{"polh_co_no"},
		{"polh_est_no"},
		{"polh_lic_cate"},
		{"polh_lic_no"},
		{"polh_hhlc_hash"},
		{"polh_lic_area"},
		{"polh_lic_val"},
		{"polh_tender_rate"},
		{"polh_ap_lic_rate"},
		{"polh_type"},
		{"polh_from_date"},
		{"polh_to_date"},
		{"polh_tot_alloc"},
		{"polh_comment_1"},
		{"polh_comment_2"},
		{"polh_comment_3"}
	};

	int polh_no_fields = 16;

	struct {
		char	lh_co_no[3];
		char	lh_est_no[3];
		char	lh_lic_cate[3];
		char	lh_lic_no[11];
		long	lh_hhlc_hash;
		char	lh_lic_area[11];
		double	lh_lic_val;
		float	lh_tender_rate;
		float	lh_ap_lic_rate;
		char	lh_type[21];
		long	lh_from_date;
		long	lh_to_date;
		double	lh_tot_alloc;
		char	lh_comment[3][51];
	} polh_rec;


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void head_polh (void);
void head_pocr (void);
void head_pocf (void);
void head_podt (void);
void read_pocf (void);
void proc_pocf (void);
void read_pocr (void);
void proc_pocr (void);
void read_podt (void);
void proc_podt (void);
void read_polh (void);
void proc_polh (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 3)
	{
		print_at(0,0,mlPoMess731,argv[0]);
        return (EXIT_FAILURE);
	}

	/*-----------------------
	| Printer Number	|
	-----------------------*/
	lpno = atoi(argv[1]);
	sprintf(rep_type,"%-1.1s", argv[2]);

	if ( !BY_CURRENCY && !BY_FREIGHT && !BY_LICENCE && !BY_DUTY )
	{
		/*printf("Usage : %s <lpno> < C(ountry) F(reight) L(icence) D(uty) >\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlPoMess731),argv[0]);
        return (EXIT_FAILURE);
	}

	init_scr();

	OpenDB();

	if ( BY_CURRENCY )
		dsp_screen("Printing Currency Control File.",
				comm_rec.tco_no,comm_rec.tco_name);

	if ( BY_FREIGHT )
		dsp_screen("Printing Country Freight Control File.",
				comm_rec.tco_no,comm_rec.tco_name);

	if ( BY_LICENCE )
		dsp_screen("Printing Licence Control File.",
				comm_rec.tco_no,comm_rec.tco_name);

	if ( BY_DUTY )
		dsp_screen("Printing Duty Control File.",
				comm_rec.tco_no,comm_rec.tco_name);

	if ((pout = popen("pformat","w")) == NULL)
		sys_err("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	if ( BY_CURRENCY )
	{
		head_pocr();
		read_pocr();
	}

	if ( BY_FREIGHT )
	{
		head_pocf();
		read_pocf();
	}

	if ( BY_LICENCE )
	{
		head_polh();
		read_polh();
	}

	if ( BY_DUTY )
	{
		head_podt();
		read_podt();
	}

	fprintf(pout,".EOF\n");
	pclose(pout);
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

	if ( BY_DUTY )
		open_rec("podt",podt_list,podt_no_fields,"podt_id_no");

	if ( BY_FREIGHT )
		open_rec("pocf",pocf_list,pocf_no_fields,"pocf_id_no");

	if ( BY_CURRENCY )
		open_rec("pocr",pocr_list,pocr_no_fields,"pocr_id_no");

	if ( BY_LICENCE )
		open_rec("polh",polh_list,polh_no_fields,"polh_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	if ( BY_DUTY )
		abc_fclose("podt");

	if ( BY_FREIGHT )
		abc_fclose("pocf");

	if ( BY_CURRENCY )
		abc_fclose("pocr");

	if ( BY_LICENCE )
		abc_fclose("polh");

	abc_dbclose("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_polh (
 void)
{

	fprintf(pout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(pout,".LP%d\n",lpno);

	fprintf(pout,".10\n");
	fprintf(pout,".PI12\n");
	fprintf(pout,".L147\n");
	fprintf(pout,".B1\n");
	fprintf(pout,".E%s LICENCE CONTROL FILE.\n",clip(comm_rec.tco_name));
	fprintf(pout,".B1\n");

	fprintf(pout,".R===========");
	fprintf(pout,"=============");
	fprintf(pout,"=============");
	fprintf(pout,"==============");
	fprintf(pout,"=========");
	fprintf(pout,"==========");
	fprintf(pout,"=======================");
	fprintf(pout,"===========");
	fprintf(pout,"===========");
	fprintf(pout,"===============\n");

	fprintf(pout,"===========");
	fprintf(pout,"=============");
	fprintf(pout,"=============");
	fprintf(pout,"==============");
	fprintf(pout,"=========");
	fprintf(pout,"==========");
	fprintf(pout,"=======================");
	fprintf(pout,"===========");
	fprintf(pout,"===========");
	fprintf(pout,"===============\n");

	fprintf(pout,"| CATEGORY ");
	fprintf(pout,"|  LICENCE   ");
	fprintf(pout,"|   AREA     ");
	fprintf(pout,"|     VALUE   ");
	fprintf(pout,"| TENDER ");
	fprintf(pout,"|  APPLIED");
	fprintf(pout,"| LICENCE              ");
	fprintf(pout,"|   FROM   ");
	fprintf(pout,"|    TO    ");
	fprintf(pout,"|   ALLOCATED |\n");

	fprintf(pout,"|          ");
	fprintf(pout,"|     NO     ");
	fprintf(pout,"|            ");
	fprintf(pout,"|             ");
	fprintf(pout,"|  RATE  ");
	fprintf(pout,"|   RATE  ");
	fprintf(pout,"| TYPE                 ");
	fprintf(pout,"|          ");
	fprintf(pout,"|          ");
	fprintf(pout,"|             |\n");

	fprintf(pout,"|----------");
	fprintf(pout,"|------------");
	fprintf(pout,"|------------");
	fprintf(pout,"|-------------");
	fprintf(pout,"|--------");
	fprintf(pout,"|---------");
	fprintf(pout,"|----------------------");
	fprintf(pout,"|----------");
	fprintf(pout,"|----------");
	fprintf(pout,"|-------------|\n");

	fflush(pout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_pocr (
 void)
{
	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(pout,".LP%d\n",lpno);

	fprintf(pout,".10\n");
	fprintf(pout,".PI12\n");
	fprintf(pout,".L147\n");
	fprintf(pout,".B1\n");
	fprintf(pout,".E%s CURRENCY EXCHANGE CONTRIL FILE\n",
				clip(comm_rec.tco_name));
	fprintf(pout,".B1\n");

	fprintf(pout,".R======");
	fprintf(pout,"==========================================");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"===============\n");

	fprintf(pout,"======");
	fprintf(pout,"==========================================");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"============");
	fprintf(pout,"===============\n");

	fprintf(pout,"| CODE");
	fprintf(pout,"| COUNTRY                                 ");
	fprintf(pout,"|  CURRENT  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"|  FORWARD  ");
	fprintf(pout,"| LAST UPDATE |\n");

	fprintf(pout,"|     ");
	fprintf(pout,"|                                         ");
	fprintf(pout,"|   RATE    ");
	fprintf(pout,"| ONE MONTH ");
	fprintf(pout,"| TWO MTHS  ");
	fprintf(pout,"|THREE MTHS ");
	fprintf(pout,"| FOUR MTHS ");
	fprintf(pout,"| FIVE MTHS ");
	fprintf(pout,"| SIX MTHS  ");
	fprintf(pout,"|             |\n");

	fprintf(pout,"|-----");
	fprintf(pout,"|-----------------------------------------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-----------");
	fprintf(pout,"|-------------|\n");

	fflush(pout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_pocf (
 void)
{
	fprintf(pout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(pout, ".LP%d\n",lpno);

	fprintf(pout, ".9\n");
	fprintf(pout, ".PI12\n");
	fprintf(pout, ".L90\n");
	fprintf(pout, ".B1\n");
	fprintf(pout, ".C%s : COUNTRY/FREIGHT CONTROL FILE\n", 
				clip(comm_rec.tco_name));
	fprintf(pout, ".B1\n");

	fprintf(pout, ".R=======");
	fprintf(pout, "=======================");
	fprintf(pout, "============");
	fprintf(pout, "==================");
	fprintf(pout, "==============");
	fprintf(pout, "================\n");

	fprintf(pout, "=======");
	fprintf(pout, "=======================");
	fprintf(pout, "============");
	fprintf(pout, "==================");
	fprintf(pout, "==============");
	fprintf(pout, "================\n");

	fprintf(pout, "| CODE ");
	fprintf(pout, "| COUNTRY              ");
	fprintf(pout, "|    TYPE   ");
	fprintf(pout, "| LOADING FACTOR  ");
	fprintf(pout, "| LEAD TIME   ");
	fprintf(pout, "| LAST UPDATED |\n");

	fprintf(pout, "|------");
	fprintf(pout, "|----------------------");
	fprintf(pout, "|-----------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|-------------");
	fprintf(pout, "|--------------|\n");

	fflush(pout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_podt (
 void)
{

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(pout,".LP%d\n",lpno);
	fprintf(pout,".10\n");
	fprintf(pout,".PI12\n");
	fprintf(pout,".L110\n");
	fprintf(pout,".B1\n");
	fprintf(pout,".E%s DUTY CONTROL FILE.\n",clip(comm_rec.tco_name));
	fprintf(pout,".B1\n");

	fprintf(pout,".R==================================================");
	fprintf(pout,"====================================================");
	fprintf(pout,"==========\n");

	fprintf(pout,"|=================================================");
	fprintf(pout,"==================================================");
	fprintf(pout,"==========|\n");

	fprintf(pout,"| CODE");
	fprintf(pout,"| DESCRIPTION         ");
	fprintf(pout,"|   IMPORT   ");
	fprintf(pout,"| IMPORT ");
	fprintf(pout,"|   EXPORT   ");
	fprintf(pout,"| EXPORT ");
	fprintf(pout,"| PERCENT ");
	fprintf(pout,"|   IMPORT   ");
	fprintf(pout,"|   EXPORT    |\n");

	fprintf(pout,"|     ");
	fprintf(pout,"|                     ");
	fprintf(pout,"|   TARIFF   ");
	fprintf(pout,"|  BAND  ");
	fprintf(pout,"|   TARIFF   ");
	fprintf(pout,"|  BAND  ");
	fprintf(pout,"| /DOLLAR ");
	fprintf(pout,"|    DUTY    ");
	fprintf(pout,"|  DRAWBACK   |\n");

	fprintf(pout,"|-----");
	fprintf(pout,"|---------------------");
	fprintf(pout,"|------------");
	fprintf(pout,"|--------");
	fprintf(pout,"|------------");
	fprintf(pout,"|--------");
	fprintf(pout,"|---------");
	fprintf(pout,"|------------");
	fprintf(pout,"|-------------|\n");

	fflush(pout);
}

/*============================
| Read Country Freight File. |
============================*/
void
read_pocf (
 void)
{
	strcpy(pocf_rec.cf_co_no,comm_rec.tco_no);
	strcpy(pocf_rec.cf_code,"   ");

	cc = find_rec("pocf",&pocf_rec,GTEQ,"r");

	while (!cc && !strcmp(pocf_rec.cf_co_no,comm_rec.tco_no))
	{
		proc_pocf();
		cc = find_rec("pocf",&pocf_rec,NEXT,"r");
	}
}

/*=====================================
| Print Country Freight File Details. |
=====================================*/
void
proc_pocf (
 void)
{
	dsp_process(" Country : ",err_str);

	sprintf(err_str,"%-20.20s",pocf_rec.cf_description);
	fprintf(pout, "|  %-3.3s ",pocf_rec.cf_code);
	fprintf(pout, "| %-20.20s ",pocf_rec.cf_description);

	if (pocf_rec.cf_load_type[0] == 'P')
		fprintf(pout, "|  PERCENT  ");

	if (pocf_rec.cf_load_type[0] == 'U')
		fprintf(pout, "|   UNIT    ");

	if (pocf_rec.cf_load_type[0] == ' ')
		fprintf(pout, "|           ");

	fprintf(pout, "|    %7.2f      ",pocf_rec.cf_freight_load);
	fprintf(pout, "|     %4d    ", pocf_rec.cf_lead_time);
	fprintf(pout, "|  %10.10s  |\n",DateToString(pocf_rec.cf_last_update));
	fflush(pout);
}

/*=====================
| Read Currency File. |
=====================*/
void
read_pocr (
 void)
{
	strcpy(pocr_rec.cr_co_no,comm_rec.tco_no);
	strcpy(pocr_rec.cr_code,"   ");

	cc = find_rec("pocr",&pocr_rec,GTEQ,"r");

	while (!cc && !strcmp(pocr_rec.cr_co_no,comm_rec.tco_no))
	{
		proc_pocr();
		cc = find_rec("pocr",&pocr_rec,NEXT,"r");
	}
}

/*==============================
| Print Currenty File Details. |
==============================*/
void
proc_pocr (
 void)
{
	sprintf(err_str,"%-40.40s",pocr_rec.cr_description);
	dsp_process(" Currency : ",err_str);

	fprintf(pout,"| %-3.3s ",pocr_rec.cr_code);
	fprintf(pout,"| %-40.40s",pocr_rec.cr_description);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[0]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[1]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[2]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[3]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[4]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[5]);
	fprintf(pout,"| %9.4f ",pocr_rec.cr_exch_rate[6]);
	fprintf(pout,"| %10.10s |\n",DateToString(pocr_rec.cr_ldate_up));
	fflush(pout);
}

/*=================
| Read Duty File. |
=================*/
void
read_podt (
 void)
{
	strcpy(podt_rec.dt_co_no,comm_rec.tco_no);
	strcpy(podt_rec.dt_code,"  ");

	cc = find_rec("podt",&podt_rec,GTEQ,"r");

	while (!cc && !strcmp(podt_rec.dt_co_no,comm_rec.tco_no))
	{
		proc_podt();
		cc = find_rec("podt",&podt_rec,NEXT,"r");
	}
}

/*==========================
| Print Duty File Details. |
==========================*/
void
proc_podt (
 void)
{
	sprintf(err_str,"%-20.20s",podt_rec.dt_description);
	dsp_process(" Duty Code : ",err_str);
	fprintf(pout,"|  %-2.2s ",podt_rec.dt_code);
	fprintf(pout,"| %-20.20s",podt_rec.dt_description);
	fprintf(pout,"| %-10.10s ",podt_rec.dt_im_tariff);
	fprintf(pout,"|    %1.1s   ",podt_rec.dt_im_band);
	fprintf(pout,"| %-10.10s ",podt_rec.dt_ex_tariff);
	fprintf(pout,"|    %1.1s   ",podt_rec.dt_ex_band);
	fprintf(pout,"|    %1.1s    ",podt_rec.dt_duty_type);
	fprintf(pout,"|  %7.2f   ",podt_rec.dt_im_duty);
	fprintf(pout,"|   %7.2f   |\n",podt_rec.dt_ex_draw);
	fflush(pout);
}

/*====================
| Read Licence File. |
====================*/
void
read_polh (
 void)
{
	strcpy(polh_rec.lh_co_no,comm_rec.tco_no);
	strcpy(polh_rec.lh_est_no,comm_rec.tes_no);
	strcpy(polh_rec.lh_lic_cate,"  ");
	strcpy(polh_rec.lh_lic_no,"          ");

	cc = find_rec("polh",&polh_rec,GTEQ,"r");

	while (!cc && !strcmp(polh_rec.lh_co_no,comm_rec.tco_no) && 
		      !strcmp(polh_rec.lh_est_no,comm_rec.tes_no))
	{
		proc_polh();
		cc = find_rec("polh",&polh_rec,NEXT,"r");
	}
}

/*==============================
| Print Licence File Details. |
==============================*/
void
proc_polh (
 void)
{
	char	blnk_line[51];

	sprintf(blnk_line,"%-50.50s"," ");
	sprintf(err_str,"%-10.10s",polh_rec.lh_lic_no);
	dsp_process(" Licence : ",err_str);

	fprintf(pout,"|    %-2.2s    ",polh_rec.lh_lic_cate);
	fprintf(pout,"| %-10.10s ",polh_rec.lh_lic_no);
	fprintf(pout,"| %-10.10s ",polh_rec.lh_lic_area);
	fprintf(pout,"| %11.2f ",polh_rec.lh_lic_val);
	fprintf(pout,"| %6.2f ",polh_rec.lh_tender_rate);
	fprintf(pout,"|  %6.2f ",polh_rec.lh_ap_lic_rate);
	fprintf(pout,"| %20.20s ",polh_rec.lh_type);
	fprintf(pout,"|%10.10s",DateToString(polh_rec.lh_from_date));
	fprintf(pout,"|%10.10s",DateToString(polh_rec.lh_to_date));
	fprintf(pout,"| %11.2f |\n",polh_rec.lh_tot_alloc);
	if (strcmp(polh_rec.lh_comment[0],blnk_line))
	{
		fprintf(pout,"|                       Comment: ");
		fprintf(pout,"%50.50s",polh_rec.lh_comment[0]);
		fprintf(pout,"                              ");
		fprintf(pout,"                |\n");
	}
	if (strcmp(polh_rec.lh_comment[1],blnk_line))
	{
		fprintf(pout,"|                                ");
		fprintf(pout,"%50.50s",polh_rec.lh_comment[1]);
		fprintf(pout,"                              ");
		fprintf(pout,"                |\n");
	}
	if (strcmp(polh_rec.lh_comment[2],blnk_line))
	{
		fprintf(pout,"|                                ");
		fprintf(pout,"%50.50s",polh_rec.lh_comment[2]);
		fprintf(pout,"                              ");
		fprintf(pout,"                |\n");
	}
	fflush(pout);
}
