/*====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_cstrep.c                                      |
|  Program Desc  : ( Print/Display Job Cost Sheet For Selected Job)   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr, sjld, sjsp, sjld, sjdc,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 21/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (21/11/88)      | Modified by   : Fui Choo Yap.    |
|  Date Modified : (24/11/88)      | Modified by   : Bee Chwee Lim.   |
|  Date Modified : (27/11/89)      | Modified by   : Fui Choo Yap.    |
|  Date Modified : (18/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (16/10/1997)    | Modified  by  : Leah Manibog.    |
|  Date Modified : (02/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Program Tidy up                                    |
|                : Combine the printing & display programs into one   |
|                : program i.e sj_cstrep & sj_cstdsp                  |
|     (27/11/89) : Change money fields to double.                     |
|                : (18/09/90) - General Update for New Scrgen. S.B.D. |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|  (16/10/1997)  : Changed length of invoice no. from 6 to 8 chars.   |
|  (02/09/1999)  : ANISfication of the code;                          |
|                :     - potential problems are marked with QUERY     |
|                                                                     |
| $Log: sj_cstrep.c,v $
| Revision 5.2  2001/08/09 09:17:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:11  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:39  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/12/06 01:34:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/16 05:58:28  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.12  1999/10/20 02:07:03  nz
| Updated for final changes on date routines.
|
| Revision 1.11  1999/10/14 00:53:08  cam
| Remove cc
|
| Revision 1.10  1999/09/29 10:12:54  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/24 05:06:28  scott
| Updated from Ansi
|
| Revision 1.8  1999/06/20 02:30:25  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_cstrep/sj_cstrep.c,v 5.2 2001/08/09 09:17:17 scott Exp $";
char	*PNAME = "$RCSfile: sj_cstrep.c,v $";

/*===============================
|   Include file dependencies   |
===============================*/
#include <pslscr.h>

#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/
#define		P_SIZE	    18
#define		PRINTER		(rep_type[0] == 'P')
#define		RESET_LN	(ln_num >= P_SIZE) ? (ln_num = ln_num % P_SIZE) : ln_num++

/*  NOTES
    it would be better if these were declared const char* 
    to minimize potential errors.
*/
char	*ULINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char	*SP1 = "  Material / Costs  ^E                                     ^E          ^E         ^E       ^E   Extend    ^E    Extend    ";
char	*SP2 = "        Code        ^E             Description             ^E   Date   ^E   Qty   ^E  Uom  ^E    Cost     ^E    Charge    ";
char	*SP3 = "^^GGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGEGGGGGGGEGGGGGGGGGGGGGEGGGGGGGGGGGGGG";

char	*LD1 = "    Labour     ^E          ^E  1.0 t  ^E  1.5 t  ^E  2.0 t  ^E   Cost   ^E  O'head  ^E  Profit  ^E   Extend   ^E   Extend    ";
char	*LD2 = "  Serv Person  ^E   Date   ^E   Hrs   ^E   Hrs   ^E   Hrs   ^E   Rate   ^E   Rate   ^E   Rate   ^E    Cost    ^E   Charge    ";
/*
char	*LD3 = "^G^G^G^G^G^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^G^G^E^G^G^G^G^G^G^G^G^G^G^G^G^G";
*/
char	*LD3 = "^^GGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGEGGGGGGGGGEGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGGGG";

char	*KM1 = "       Travel       ^E          ^E              ^E                            ^E           ^E    Extend     ^E   Extend   ";
char	*KM2 = "   Service Person   ^E   Date   ^E   Mileage    ^E                            ^E   Rate    ^E     Cost      ^E   Charge   "; 
char	*KM3 = "^^GGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGG";


char	*DC1 = "                ^E          Outside Purchases          ^E          ^E          ^E Invoice  ^E   Extend    ^E    Extend   ";
char	*DC2 = "   P/order No   ^E             Description             ^E   Date   ^E Supplier ^E    No    ^E    Cost     ^E    Charge    ";
char	*DC3 = "^^GGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGEGGGGGGGGGGGGGG";

char	*comm	= "comm",
		*cumr	= "cumr",
		*sumr	= "sumr",
		*sjhr	= "sjhr",
		*sjjd	= "sjjd",
		*sjsd	= "sjsd",
		*sjld	= "sjld",
		*sjdc	= "sjdc",
		*sjsp	= "sjsp",
        *data   = "data";

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = 
    {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dbt_date"},
	};

	const int comm_no_fields = 7;

	struct 
    {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
		long	tdbt_date;
	} comm_rec;

	/*========================
	| Creditors Master File. |
	========================*/
	struct dbview sumr_list[] =
    {
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
	};

	const int sumr_no_fields = 5;

	struct 
    {
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_crd_name[41];
	} sumr_rec;

	/*=================================
	| Service Job Header Record File. |
	=================================*/
	struct dbview sjhr_list[] =
    {
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_status"},
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_cust_ord_no"},
		{"sjhr_contact"},
		{"sjhr_issue_date"},
		{"sjhr_reqd_date"},
	};

	const int sjhr_no_fields = 11;

	struct 
    {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_ord_no[11];
		char	hr_contact[21];
		long	hr_issue_date;
		long	hr_reqd_date;
	} sjhr_rec;

	/*==========================
	| Service Job detail file  |
	==========================*/
	struct dbview sjjd_list[] =
    {
		{"sjjd_co_no"},
		{"sjjd_est_no"},
		{"sjjd_dp_no"},
		{"sjjd_order_no"},
		{"sjjd_line_no"},
		{"sjjd_detail"}
	};

	const int sjjd_no_fields = 6;

	struct 
    {
		char	jd_co_no[3];
		char	jd_est_no[3];
		char	jd_dp_no[3];
		long	jd_order_no;
		int		jd_line_no;
		char	jd_detail[71];
	} sjjd_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] =
    {
		{"cumr_co_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_class_type"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
	};

	const int cumr_no_fields = 8;

	struct 
    {
		char	cm_co_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
		char	cm_class_type[4];
		char	cm_ch_adr[3][41];
	} cumr_rec;

	/*======================
	|  Service detail file |
	======================*/
	struct dbview sjsd_list[] =
    {
		{"sjsd_co_no"},
		{"sjsd_est_no"},
		{"sjsd_dp_no"},
		{"sjsd_order_no"},
		{"sjsd_line_no"},
		{"sjsd_detail"}
	};

	const int sjsd_no_fields = 6;

	struct 
    {
		char	sd_co_no[3];
		char	sd_est_no[3];
		char	sd_dp_no[3];
		long	sd_order_no;
		int		sd_line_no;
		char	sd_detail[71];
	} sjsd_rec;

	/*====================
	| Spare parts usage  |
	====================*/
	struct dbview sjsp_list[] =
    {
		{"sjsp_co_no"},
		{"sjsp_est_no"},
		{"sjsp_dp_no"},
		{"sjsp_order_no"},
		{"sjsp_partno"},
		{"sjsp_part_desc"},
		{"sjsp_date"},
		{"sjsp_qty"},
		{"sjsp_uom"},
		{"sjsp_u_cost"},
		{"sjsp_u_sell"},
		{"sjsp_porder_no"}
	};

	const int sjsp_no_fields = 12;

	struct 
    {
		char	sp_co_no[3];
		char	sp_est_no[3];
		char	sp_dp_no[3];
		long	sp_order_no;
		char	sp_partno[17];
		char	sp_part_desc[41];
		long	sp_date;
		float	sp_qty;
		char	sp_uom[4];
		double	sp_u_cost;
		double	sp_u_sell;
		char	sp_porder_no[7];
	} sjsp_rec;

	/*=====================
	| Labour details file |
	=====================*/
	struct dbview sjld_list[] =
    {
		{"sjld_co_no"},
		{"sjld_est_no"},
		{"sjld_dp_no"},
		{"sjld_order_no"},
		{"sjld_emp_code"},
		{"sjld_date"},
		{"sjld_km"},
		{"sjld_km_rate"},
		{"sjld_time"},
		{"sjld_time_half"},
		{"sjld_time_double"},
		{"sjld_tm_rate"},
		{"sjld_oh_rate"},
		{"sjld_pr_rate"}
	};

	const int sjld_no_fields = 14;

	struct 
    {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
		float	ld_km;
		double	ld_km_rate;
		float	ld_time;
		float	ld_time_half;
		float	ld_time_double;
		double	ld_tm_rate;
		double	ld_oh_rate;
		double	ld_pr_rate;
	} sjld_rec;

	/*===================
	| Outside purchases |
	===================*/
	struct dbview sjdc_list[] =
    {
		{"sjdc_co_no"},
		{"sjdc_est_no"},
		{"sjdc_dp_no"},
		{"sjdc_order_no"},
		{"sjdc_po_no"},
		{"sjdc_desc"},
		{"sjdc_hhsu_hash"},
		{"sjdc_invoice_no"},
		{"sjdc_date"},
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"}
	};

	const int sjdc_no_fields = 12;

	struct 
    {
		char	dc_co_no[3];
		char	dc_est_no[3];
		char	dc_dp_no[3];
		long	dc_order_no;
		char	dc_po_no[9];
		char	dc_desc[31];
		long	dc_hhsu_hash;
		char	dc_invoice_no[9];
		long	dc_date;
		double	dc_est_cost;
		double	dc_act_cost;
		double	dc_chg_cost;
	} sjdc_rec;

/*=====================
|   Local variables   |
=====================*/


	char	stat_prt[25];
	char	branchNo[3];
	char	rep_type[2];
	char	env_line[250];

	int	envDbCo = 0;
	int	envDbFind  = 0;
	int	lp_no = 1;
	int	ln_num = 1;

	double	job_cst,
			job_chg;

	FILE	*fin, 
			*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char    c_client[7];
	char    chg_client[41];
  	char    end_client[41];
	char	order_no[7];
	char	dummy[11];
} local_rec;

/*===============================
|   Local function prototypes   |
===============================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void head_output (void);
int  proc_sjhr (void);  /* print job header 		    */
int  proc_sjjd (void);  /* print job details		    */
int  proc_sjsd (void);  /* print service details        */
int  proc_sjsp (void);  /* print spare parts/charges    */
int  proc_labr (void);  /* print labour details         */
int  proc_lakm (void);  /* print mileage details        */
int  proc_sjdc (void);  /* print outside purchs details */
void get_stat_desc (void);
int  check_page (void);
int  spec_valid (int field);
int  heading (int scn);


/*=========================
| Main Processing Routine |
==========================*/
int 
main (
 int argc, 
 char *argv[])
{
	if (argc != 4)
	{
		print_at (0,0, mlSjMess709, argv[0]);
        return (EXIT_FAILURE);
	}

    /*-------------------------------------
    |  initialize some variables, flags   |
    -------------------------------------*/
    prog_exit  = FALSE;

	switch (argv[1][0])
	{
    	case 'P':
    	case 'p':
	    	strcpy (rep_type,"P");
		    break;

    	case 'D':
	    case 'd':
		    strcpy (rep_type,"D");
    		break;

	    default :
		    print_at (0,0, ML (mlSjMess710));
            return (EXIT_SUCCESS);
	}

	sjhr_rec.hr_order_no = atol (argv[2]);
	lp_no = atoi (argv[3]);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

    OpenDB ();
	/*====================
	|  get branchNolishment |
	====================*/

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.tes_no);

    init_scr ();
	set_tty ();
	swide ();

    /* print job header 		    */
	if (proc_sjhr () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }
    
    /* print job details		    */
    if (proc_sjjd () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

    /* print service details        */
	if (proc_sjsd () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

    /* print spare parts/charges    */
    if (proc_sjsp () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

    /* print labour details         */
	if (proc_labr () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

    /* print mileage details        */
	if (proc_lakm () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

    /* print outside purchs details */
	if (proc_sjdc () == EXIT_FAILURE)
    {
        shutdown_prog ();
        return (EXIT_FAILURE);
    }

	if (PRINTER)
	{
		/*=================
		| Print job totals |
		==================*/
		pr_format (fin,fout,"TOT1",1,job_cst);
		pr_format (fin,fout,"TOT1",2,job_chg);
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"EOF",0,0);
	}
	else
	{
		sprintf (env_line,
                 "  Service Job Totals                                                                   ^E  %9.2f  ^E  %9.2f   ",
                 job_cst,
                 job_chg);
        Dsp_saverec (env_line);
		Dsp_saverec (ULINE);
		Dsp_srch ();
		Dsp_close ();
	}

	shutdown_prog ();
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
    abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

    open_rec (sjhr, sjhr_list, sjhr_no_fields, "sjhr_id_no");
    open_rec (sjjd, sjjd_list, sjjd_no_fields, "sjjd_id_no");
    open_rec (cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
    open_rec (sjsd, sjsd_list, sjsd_no_fields, "sjsd_id_no");
    open_rec (sjsp, sjsp_list, sjsp_no_fields, "sjsp_id_no");
    open_rec (sjld, sjld_list, sjld_no_fields, "sjld_id_no");
    open_rec (sjdc, sjdc_list, sjdc_no_fields, "sjdc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (
 void)
{
	abc_fclose (sjhr);
	abc_fclose (cumr);
	abc_fclose (sjjd);
	abc_fclose (sjsd);
	abc_fclose (sjld);
	abc_fclose (sjdc);
	abc_fclose (sjsp);
	abc_dbclose (data);
}

void
head_output (
 void)
{
	if (PRINTER)
	{
		dsp_screen ("Processing : Printing Job Cost Sheet", 
                    comm_rec.tco_no, 
                    comm_rec.tco_name);

		if ((fin = pr_open ("sj_cstrep.p")) == NULL)
        {
			sys_err ("Error in opening sj_cstrep.p during (FOPEN)",
                     errno, 
                     PNAME);
        }
		/*----------------------
		| Open pipe to pformat | 
		----------------------*/
		if ((fout = popen ("pformat","w")) == NULL)
        {
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);
        }

		/*---------------------------------
		| Start output to standard print. |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
		fprintf (fout,".LP%d\n",lp_no);
		fprintf (fout,".OP\n");
		fprintf (fout,".9\n");
		fprintf (fout,".PI12\n");
		fprintf (fout,".L120\n");
		fprintf (fout,
                 ".ECOMPANY %s - %s\n",
                 comm_rec.tco_no, 
                 clip (comm_rec.tco_name));
		fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());
		pr_format (fin,fout,"HED1",0,0);

		pr_format (fin,fout,"SKIP",1,1);

		pr_format (fin,fout,"HED2",1,sjhr_rec.hr_order_no);
		pr_format (fin,fout,"HED2",2,sjhr_rec.hr_cust_ord_no);
	}
	else
	{
		Dsp_open (0,0,P_SIZE);
		Dsp_saverec("                            S E R V I C E   O R D E R   J O B   C O S T   D E T A I L S                             ");

		sprintf (env_line,
                 "Service Order No. %6ld                  Customer Order No. %-10.10s                                             ",
                 sjhr_rec.hr_order_no,
                 sjhr_rec.hr_cust_ord_no);
        Dsp_saverec (env_line);
		Dsp_saverec ("    [Redraw]  [Next Screen]  [Previous Screen]  [End/Input]    ");
	}
}

int
proc_sjhr (
 void)
{
	int 	j  = 0;
	char	chg_adr[3][41];
	
	sprintf (err_str,"%06ld",sjhr_rec.hr_order_no);
	if (PRINTER)
    {
		dsp_process ("Order No :",clip (err_str));
    }

	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.tes_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);

	cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"r");
	if (cc)
	{
		/*------------------
		| Order Not found. |
		------------------*/
		errmess (ML (mlStdMess122));
        /*  QUERY
            exit_prog () renamed and moved outside 
            this function...
        */
        return (EXIT_FAILURE);
	}

	head_output ();

	cc = find_hash (cumr,
                    &cumr_rec,
                    COMPARISON,
                    "r",
                    sjhr_rec.hr_chg_client);
	if (!cc)
	{
		strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
		strcpy (local_rec.chg_client,cumr_rec.cm_dbt_name);
	}
	else
	{
		strcpy (local_rec.c_client,"      ");
		strcpy (local_rec.chg_client,"**** Customer Unknown ****");
	}

	for (j = 0;j < 3; j++)
    {
		strcpy (chg_adr[j], cumr_rec.cm_ch_adr[j]);
    }

	if (sjhr_rec.hr_end_client > 0L)
	{
		cc = find_hash (cumr,
                        &cumr_rec,
                        COMPARISON,
                        "r",
                        sjhr_rec.hr_end_client);
		if (!cc)
        {
			strcpy (local_rec.end_client, cumr_rec.cm_dbt_name);
        }
		else
        {
			strcpy (local_rec.end_client,"**** Customer Unknown ****");
        }
	}
	else
    {
		strcpy (local_rec.end_client,"**** Customer Unknown ****");
    }

	get_stat_desc ();

	if (PRINTER)
	{ 
		pr_format (fin, fout, "HED3", 1, local_rec.c_client);
		pr_format (fin, fout, "HED3", 2, cumr_rec.cm_dbt_no);
		pr_format (fin, fout, "HED3", 3, DateToString (sjhr_rec.hr_issue_date));

		pr_format (fin, fout, "HED4", 1, local_rec.chg_client);
		pr_format (fin, fout, "HED4", 2, local_rec.end_client);
		pr_format (fin, fout, "HED4", 3, DateToString (sjhr_rec.hr_issue_date));

		for (j = 0; j < 3; j++)
		{
			pr_format (fin, fout, "HED5", 1, chg_adr[j]);
			pr_format (fin, fout, "HED5", 2, cumr_rec.cm_ch_adr[j]);
		}
		pr_format (fin, fout, "HED6", 1, sjhr_rec.hr_contact);
		pr_format (fin, fout, "HED6", 2, sjhr_rec.hr_status);
		pr_format (fin, fout, "HED6", 3, stat_prt);
	}
	else
	{
		sprintf (env_line,
                 "    Charge Customer  %-6.6s                   For Customer  %-6.6s                      Order Date    : %-10.10s     ", 
			     local_rec.c_client,
                 cumr_rec.cm_dbt_no,
                 DateToString (sjhr_rec.hr_issue_date));
        Dsp_saverec (env_line);
		RESET_LN;

		sprintf (env_line,
                 "    %-40.40s  %-40.40s  Required Date : %-10.10s  ", 
                 local_rec.chg_client,
                 local_rec.end_client,
                 DateToString (sjhr_rec.hr_reqd_date));
		Dsp_saverec (env_line);
		RESET_LN;

		for (j = 0; j < 3; j++)
		{
			sprintf (env_line,
                     "    %-40.40s  %-40.40s                              ", 
                     chg_adr[j],
                     cumr_rec.cm_ch_adr[j]);
            Dsp_saverec (env_line);
			RESET_LN;
		}

        sprintf (env_line,
                 "    Contact %-20.20s              Status : %1.1s - %-24.24s                                 ",   
                 sjhr_rec.hr_contact,
                 sjhr_rec.hr_status,
                 stat_prt);
        Dsp_saverec (env_line);
        RESET_LN;
	}

    return (EXIT_SUCCESS);
}

int
proc_sjjd ( 
 void)
{
	/*===================
	|  read job details |
	===================*/
	if (PRINTER)
	{
		pr_format (fin,fout,"SKIP",1,1);
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"JD1",0,0);
	}
	else
	{
		sprintf (env_line,"%-116.116s"," ");
		Dsp_saverec (env_line);
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("    Job Description : ");
		RESET_LN;
	}
	
	strcpy (sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy (sjjd_rec.jd_est_no,comm_rec.tes_no);
	strcpy (sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;

    cc = find_rec (sjjd,&sjjd_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (sjjd_rec.jd_co_no,comm_rec.tco_no) && 
           !strcmp (sjjd_rec.jd_est_no,comm_rec.tes_no) && 
           !strcmp (sjjd_rec.jd_dp_no,comm_rec.tdp_no) && 
           (sjjd_rec.jd_order_no == sjhr_rec.hr_order_no))
	{
		if (PRINTER)
        {
			pr_format (fin,fout,"JD2",1,sjjd_rec.jd_detail);
        }
		else
		{
			sprintf (env_line,
                     "                        %-70.70s",
                     sjjd_rec.jd_detail);
			Dsp_saverec (env_line);
			RESET_LN;
		}

		cc = find_rec (sjjd, &sjjd_rec, NEXT, "r");
	}
	if (PRINTER)
	{
		pr_format (fin, fout, "ULINE",0 ,0);
		pr_format (fin, fout, "SKIP",1 ,1);
	}
	else
	{
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
	}

    return (EXIT_SUCCESS);
}

int
proc_sjsd (
 void)
{
	/*=======================
	|  read service details |
	=======================*/
	if (PRINTER)
    {
		pr_format (fin, fout, "SD1", 0, 0);
    }
	else
	{
		Dsp_saverec ("    Service Details : ");
		RESET_LN;
	}
	
	strcpy (sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy (sjsd_rec.sd_est_no,comm_rec.tes_no);
	strcpy (sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = 0;
	
    cc = find_rec (sjsd, &sjsd_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (sjsd_rec.sd_co_no,comm_rec.tco_no) && 
           !strcmp (sjsd_rec.sd_est_no,comm_rec.tes_no) && 
           !strcmp (sjsd_rec.sd_dp_no,comm_rec.tdp_no) && 
           (sjsd_rec.sd_order_no == sjhr_rec.hr_order_no))
	{
		if (PRINTER)
        {
			pr_format (fin,fout,"JD2",1,sjsd_rec.sd_detail);
        }
		else
		{
			sprintf (env_line, "    %-70.70s", sjsd_rec.sd_detail);
			Dsp_saverec (env_line);
			RESET_LN;
		}

		cc = find_rec (sjsd, &sjsd_rec, NEXT, "r");
	}

	if (PRINTER)
	{
		pr_format (fin, fout, "ULINE", 0, 0);
		pr_format (fin, fout, "SKIP", 1, 1);
	}
	else
	{
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
	}

    return (EXIT_SUCCESS);
}

int
proc_sjsp (
 void)
{
	int	printed = FALSE;
	double	lin_cst = 0.0,
            lin_chg = 0.0,
            tot_cst = 0.0,
            tot_chg = 0.0;

	/*===========================
	|  read Spare parts details |
	===========================*/
	if (PRINTER)
	{
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"SP1",0,0);
		pr_format (fin,fout,"SP2",0,0);
		pr_format (fin,fout,"SP3",0,0);
	}
	else
	{
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec (SP1);
		RESET_LN;
		Dsp_saverec (SP2);
		RESET_LN;
		Dsp_saverec (SP3);
		RESET_LN;
	}
	
	strcpy (sjsp_rec.sp_co_no,comm_rec.tco_no);
	strcpy (sjsp_rec.sp_est_no,comm_rec.tes_no);
	strcpy (sjsp_rec.sp_dp_no,comm_rec.tdp_no);
	sjsp_rec.sp_order_no = sjhr_rec.hr_order_no;
	sprintf (sjsp_rec.sp_partno,"%16.16s"," ");

    cc = find_rec (sjsp,&sjsp_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (sjsp_rec.sp_co_no,comm_rec.tco_no) && 
           !strcmp (sjsp_rec.sp_est_no,comm_rec.tes_no) && 
           !strcmp (sjsp_rec.sp_dp_no,comm_rec.tdp_no) && 
           (sjsp_rec.sp_order_no == sjhr_rec.hr_order_no))
	{
		printed = TRUE;

		lin_cst = (double) sjsp_rec.sp_qty * sjsp_rec.sp_u_cost;
		lin_chg = (double) sjsp_rec.sp_qty * sjsp_rec.sp_u_sell;
		tot_cst += lin_cst;
		tot_chg += lin_chg;

		if (PRINTER)
		{
			pr_format (fin,fout,"SP4",1,sjsp_rec.sp_partno);
			pr_format (fin,fout,"SP4",2,sjsp_rec.sp_part_desc);
			pr_format (fin,fout,"SP4",3,DateToString (sjsp_rec.sp_date));
			pr_format (fin,fout,"SP4",4,sjsp_rec.sp_qty);
			pr_format (fin,fout,"SP4",5,sjsp_rec.sp_uom);
			pr_format (fin,fout,"SP4",6,lin_cst);
			pr_format (fin,fout,"SP4",7,lin_chg);
		}
		else
		{
			sprintf (env_line,
                     "  %-16.16s  ^E %-35.35s ^E%-10.10s^E %8.2f^E  %-3.3s  ^E  %9.2f  ^E  %9.2f   ",
                     sjsp_rec.sp_partno,
                     sjsp_rec.sp_part_desc,
                     DateToString (sjsp_rec.sp_date),
                     sjsp_rec.sp_qty,
                     sjsp_rec.sp_uom,
                     lin_cst,
                     lin_chg);
			Dsp_saverec (env_line);
			RESET_LN;
		}

		cc = find_rec (sjsp, &sjsp_rec, NEXT, "r");
	}
	if (PRINTER)
	{
		if (printed)
        {
			pr_format (fin, fout, "ULINE", 0, 0);
        }
		pr_format (fin,fout,"SP5",1,tot_cst);
		pr_format (fin,fout,"SP5",2,tot_chg);
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"SKIP",1,1);
		pr_format (fin,fout,"ULINE",0,0);
	}
	else
	{
		if (printed)
		{
			Dsp_saverec (ULINE);
			RESET_LN;
		}
		sprintf (env_line,"  Totals            ^E                                     ^E          ^E         ^E       ^E  %9.2f  ^E  %9.2f   ",
                 tot_cst,
                 tot_chg);
        Dsp_saverec (env_line);
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
	}

	job_cst += tot_cst;
	job_chg += tot_chg;

    return (EXIT_SUCCESS);
}

int
proc_labr (
  void)
{
	int	printed = FALSE;

	double	lin_cst = 0.0,
            lin_chg = 0.0,
            lin_hrs = 0.0,
            tot_1t  = 0.0,
            tot_15t = 0.0,
            tot_2t  = 0.0,
            tot_cst = 0.0,
            tot_chg = 0.0;

	/*======================
	|  read Labour details |
	======================*/
	if (PRINTER)
	{
		pr_format (fin,fout,"LD1",0,0);
		pr_format (fin,fout,"LD2",0,0);
		pr_format (fin,fout,"LD3",0,0);
	}
	else
	{
		Dsp_saverec (LD1);
		RESET_LN;
		Dsp_saverec (LD2);
		RESET_LN;
		Dsp_saverec (LD3);
		RESET_LN;
	}
	
	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.tes_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = sjhr_rec.hr_order_no;
	sjld_rec.ld_date = 0;

	cc = find_rec (sjld,&sjld_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) && 
           !strcmp (sjld_rec.ld_est_no,comm_rec.tes_no) && 
           !strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no) && 
           (sjld_rec.ld_order_no == sjhr_rec.hr_order_no))
	{
		printed = TRUE;

		lin_hrs = sjld_rec.ld_time
                    + (sjld_rec.ld_time_half * 1.5)
                    + (sjld_rec.ld_time_double * 2);

		lin_cst = lin_hrs * sjld_rec.ld_tm_rate;

		lin_chg = lin_cst		
                    + (lin_hrs * sjld_rec.ld_oh_rate)
                    + (lin_hrs * sjld_rec.ld_pr_rate);

		tot_1t  += sjld_rec.ld_time;
		tot_15t += sjld_rec.ld_time_half ;
		tot_2t  += sjld_rec.ld_time_double;
		tot_cst += lin_cst;
		tot_chg += lin_chg;

		if (PRINTER)
		{
			pr_format (fin,fout,"LD4",1,sjld_rec.ld_emp_code);
			pr_format (fin,fout,"LD4",2,DateToString (sjld_rec.ld_date));
			pr_format (fin,fout,"LD4",3,sjld_rec.ld_time);
			pr_format (fin,fout,"LD4",4,sjld_rec.ld_time_half);
			pr_format (fin,fout,"LD4",5,sjld_rec.ld_time_double);
			pr_format (fin,fout,"LD4",6,sjld_rec.ld_tm_rate);
			pr_format (fin,fout,"LD4",7,sjld_rec.ld_oh_rate);
			pr_format (fin,fout,"LD4",8,sjld_rec.ld_pr_rate);
			pr_format (fin,fout,"LD4",9,lin_cst);
			pr_format (fin,fout,"LD4",10,lin_chg);
		}
		else
		{
			sprintf (env_line,
                    "  %-11.11s  ^E%-10.10s^E  %6.2f ^E  %6.2f ^E  %6.2f ^E  %7.2f ^E  %7.2f ^E  %7.2f ^E  %9.2f ^E  %9.2f  ",
                    sjld_rec.ld_emp_code,
                    DateToString (sjld_rec.ld_date),
                    sjld_rec.ld_time,
                    sjld_rec.ld_time_half,
                    sjld_rec.ld_time_double,
                    sjld_rec.ld_tm_rate,
                    sjld_rec.ld_oh_rate,
                    sjld_rec.ld_pr_rate,
                    lin_cst,
                    lin_chg);
            Dsp_saverec (env_line);
            RESET_LN;
		}

		cc = find_rec (sjld, &sjld_rec, NEXT, "r");
	}
	
	if (PRINTER)
	{
		if (printed)
        {
			pr_format (fin,fout,"ULINE",0,0);
        }
		pr_format (fin,fout,"LD5",1,tot_1t);
		pr_format (fin,fout,"LD5",2,tot_15t);
		pr_format (fin,fout,"LD5",3,tot_2t);
		pr_format (fin,fout,"LD5",4,tot_cst);
		pr_format (fin,fout,"LD5",5,tot_chg);
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"SKIP",1,1);
		pr_format (fin,fout,"ULINE",0,0);
	}
	else
	{
		if (printed)
		{
			Dsp_saverec (ULINE);
			RESET_LN;
		}
		sprintf(env_line,
                "  Totals       ^E          ^E  %6.2f ^E  %6.2f ^E  %6.2f ^E          ^E          ^E          ^E  %9.2f ^E  %9.2f  ",
                tot_1t,
                tot_15t,
                tot_2t,
                tot_cst,
                tot_chg);

        Dsp_saverec (env_line);
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
	}

	job_cst += tot_cst;
	job_chg += tot_chg;

    return (EXIT_SUCCESS);
}

int
proc_lakm (
 void)
{
	int	    printed = FALSE;
	float	tot_km  = 0.00;

	double	lin_cst = 0.0,
            lin_chg = 0.0,
            tot_cst = 0.0,
            tot_chg = 0.0;

	/*======================
	|  read Labour details |
	======================*/
	if (PRINTER)
	{
		pr_format (fin,fout,"KM1",0,0);
		pr_format (fin,fout,"KM2",0,0);
		pr_format (fin,fout,"KM3",0,0);
	}
	else
	{
		Dsp_saverec (KM1);
		RESET_LN;
		Dsp_saverec (KM2);
		RESET_LN;
		Dsp_saverec (KM3);
		RESET_LN;
	}
	
	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.tes_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = sjhr_rec.hr_order_no;
	sjld_rec.ld_date = 0;

	cc = find_rec (sjld,&sjld_rec,GTEQ,"r");

	while (!cc && 
           !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) && 
           !strcmp (sjld_rec.ld_est_no,comm_rec.tes_no) && 
           !strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no) && 
           (sjld_rec.ld_order_no == sjhr_rec.hr_order_no))
	{
		printed = TRUE;
		if (sjld_rec.ld_km == 0)
		{
			cc = find_rec (sjld,&sjld_rec,NEXT,"r");
			continue;
		}

		lin_cst = sjld_rec.ld_km * sjld_rec.ld_km_rate;
		lin_chg = sjld_rec.ld_km * sjld_rec.ld_km_rate;

		tot_km  += sjld_rec.ld_km;
		tot_cst += lin_cst;
		tot_chg += lin_chg;

		if (PRINTER)
		{
			pr_format (fin,fout,"KM4",1,sjld_rec.ld_emp_code);
			pr_format (fin,fout,"KM4",2,DateToString (sjld_rec.ld_date));
			pr_format (fin,fout,"KM4",3,sjld_rec.ld_km);
			pr_format (fin,fout,"KM4",4,sjld_rec.ld_km_rate);
			pr_format (fin,fout,"KM4",5,lin_cst);
			pr_format (fin,fout,"KM4",6,lin_chg);
		}
		else
		{
			sprintf (env_line,
                     "  %-16.16s  ^E%-10.10s^E %9.2f kms^E                            ^E  %7.2f  ^E   %9.2f   ^E %9.2f  ",
                     sjld_rec.ld_emp_code,
                     DateToString (sjld_rec.ld_date),
                     sjld_rec.ld_km,
                     sjld_rec.ld_km_rate,
                     lin_cst,
                     lin_chg);
            Dsp_saverec (env_line);
			RESET_LN;
		}

		cc = find_rec (sjld,&sjld_rec,NEXT,"r");
	}
	
	if (PRINTER)
	{
		if (printed)
        {
			pr_format (fin,fout,"ULINE",0,0);
        }
		pr_format (fin,fout,"KM5",1,tot_km);
		pr_format (fin,fout,"KM5",2,tot_cst);
		pr_format (fin,fout,"KM5",3,tot_chg);

		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"SKIP",1,1);
		pr_format (fin,fout,"ULINE",0,0);
	}
	else
	{
		if (printed)
		{
			Dsp_saverec (ULINE);
			RESET_LN;
		}
		sprintf(env_line,
                "  Totals            ^E          ^E %9.2f kms^E                            ^E           ^E   %9.2f   ^E %9.2f  ",
                tot_km,
                tot_cst,
                tot_chg);
        Dsp_saverec (env_line);
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
	}
	job_cst += tot_cst;
	job_chg += tot_chg;

    return (EXIT_SUCCESS);
}

int
proc_sjdc (
 void)
{
	int	printed = FALSE;

	double 	tot_cst = 0,    
            tot_chg = 0;

	/*======================
	|  read outside purchs |
	======================*/
	if (PRINTER)
	{
		pr_format (fin,fout,"DC1",0,0);
		pr_format (fin,fout,"DC2",0,0);
		pr_format (fin,fout,"DC3",0,0);
	}
	else
	{
		Dsp_saverec (DC1);
		RESET_LN;
		Dsp_saverec (DC2);
		RESET_LN;
		Dsp_saverec (DC3);
		RESET_LN;
	}
	
	open_rec (sumr, sumr_list, sumr_no_fields, "sumr_hhsu_hash");

	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.tes_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf (sjdc_rec.dc_po_no,"%8.8s"," ");

	cc = find_rec (sjdc,&sjdc_rec,GTEQ,"r");
	while (!cc && 
           !strcmp (sjdc_rec.dc_co_no,comm_rec.tco_no) && 
           !strcmp (sjdc_rec.dc_est_no,comm_rec.tes_no) && 
           !strcmp (sjdc_rec.dc_dp_no,comm_rec.tdp_no) && 
           (sjdc_rec.dc_order_no == sjhr_rec.hr_order_no))
	{
		printed = TRUE;
		cc = find_hash (sumr,
                        &sumr_rec,
                        COMPARISON,
                        "r",
                        sjdc_rec.dc_hhsu_hash);
		if (cc)
        {
			strcpy (sumr_rec.sm_crd_no,"      ");
        }

		if (sjdc_rec.dc_act_cost == 0)
        {
			sjdc_rec.dc_act_cost = sjdc_rec.dc_est_cost;
        }

		tot_cst += sjdc_rec.dc_act_cost;
		tot_chg += sjdc_rec.dc_chg_cost;

		if (PRINTER)
		{
			pr_format (fin,fout,"DC4",1,sjdc_rec.dc_po_no);
			pr_format (fin,fout,"DC4",2,sjdc_rec.dc_desc);
			pr_format (fin,fout,"DC4",3,DateToString (sjdc_rec.dc_date));
			pr_format (fin,fout,"DC4",4,sumr_rec.sm_crd_no);
			pr_format (fin,fout,"DC4",5,sjdc_rec.dc_invoice_no);
			pr_format (fin,fout,"DC4",6,sjdc_rec.dc_act_cost);
			pr_format (fin,fout,"DC4",7,sjdc_rec.dc_chg_cost);
		}
		else
		{
			sprintf(env_line,
                    "  %-12.12s  ^E  %-33.33s  ^E%-10.10s^E  %-6.6s  ^E  %-8.8s  ^E  %9.2f  ^E   %9.2f  ",
                    sjdc_rec.dc_po_no,
                    sjdc_rec.dc_desc,
                    DateToString (sjdc_rec.dc_date),
                    sumr_rec.sm_crd_no,
                    sjdc_rec.dc_invoice_no,
                    sjdc_rec.dc_act_cost,
                    sjdc_rec.dc_chg_cost);
            Dsp_saverec (env_line);
			RESET_LN;
		}

		cc = find_rec (sjdc,&sjdc_rec,NEXT,"r");
	}

	abc_fclose (sumr);
	
	if (PRINTER)
	{
		if (printed)
        {
			pr_format (fin,fout,"ULINE",0,0);
        }
		pr_format (fin,fout,"DC5",1,tot_cst);
		pr_format (fin,fout,"DC5",2,tot_chg);
		pr_format (fin,fout,"ULINE",0,0);
		pr_format (fin,fout,"SKIP",1,1);
		pr_format (fin,fout,"ULINE",0,0);
	}
	else
	{
		if (printed)
		{
			Dsp_saverec (ULINE);
			RESET_LN;
		}
		sprintf (env_line, 
                "  Totals        ^E                                     ^E          ^E          ^E          ^E  %9.2f  ^E   %9.2f  ",
                tot_cst,
                tot_chg);
		Dsp_saverec (env_line);
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
		Dsp_saverec ("  ");
		RESET_LN;
		Dsp_saverec (ULINE);
		RESET_LN;
	}
	job_cst += tot_cst;
	job_chg += tot_chg;

    return (EXIT_SUCCESS);
}

void
get_stat_desc (
 void)
{
	switch (sjhr_rec.hr_status[0])
	{
        case 'O':
		    strcpy (stat_prt,"Open order");
		    break;

        case 'C':
		    strcpy (stat_prt,"Closed order");
		    break;

        case 'H':
            strcpy (stat_prt,"Order on hold");
            break;
            
        case 'I':
            strcpy (stat_prt,"Order invoiced");
            break;
        
        default :
            strcpy (stat_prt,"Unknown Order Status");
            break;
	}
}

/*  QUERY
    is this correct? 
    these functions are empty! they do nothing!
*/
int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
    return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
    return (EXIT_SUCCESS);
}


/* [ end of file ] */
