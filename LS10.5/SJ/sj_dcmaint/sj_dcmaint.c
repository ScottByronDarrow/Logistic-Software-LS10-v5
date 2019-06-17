/*====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_dcmaint.c                                     |
|  Program Desc  : ( Service Job Disbursements maintenance        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, sjdc, cumr, sumr,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjdc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 05/08/87         |
|---------------------------------------------------------------------|
|  Date Modified : 17/11/88        | Modified By : B.C.Lim.           |
|  Date Modified : 23/11/89        | Modified By : Fui Choo Yap.      |
|  Date Modified : 20/12/89        | Modified By : Fui Choo Yap.      |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (06/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Use new screen generator & tidy up program.        |
|                :                                                    |
|     (23/11/89) : Change MONEYTYPE fields to DOUBLETYPE.             |
|                :                                                    |
|     (20/12/89) : fix bug - cumr_hhcu_hash was not declared but used.|
|                : (10/10/90) - General Update for New Scrgen. S.B.D. |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|  (06/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems are marked with QUERY    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_dcmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_dcmaint/sj_dcmaint.c,v 5.2 2001/08/09 09:17:19 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#define MAXWIDTH	132
#define TABLINES	10

#include <pslscr.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/

#define	OPEN_JOB	(sjhr_rec.hr_status[0] == 'O')
#define	HELD_JOB	(sjhr_rec.hr_status[0] == 'H')

/*  QUERY
    these should be declared const char*
    to minimize potential problems
*/
char *cumr = "cumr";
char *sjhr = "sjhr";
char *sjdc = "sjdc";
char *sumr = "sumr";
char *data = "data";

/*=====================
|   Local variables   |
=====================*/

	char	branchNumber[3];

	int		envDbCo = 0,
			cr_find  = 0,
			new_item = 0;

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
	};

	const int comm_no_fields = 6;

	struct 
    {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*===================
	|  Job Header File  |
	|===================*/
	struct dbview sjhr_list[] = 
    {
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_status"},
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_issue_date"},
	};

	const int sjhr_no_fields = 8;

	struct 
    {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		long	hr_issue_date;
	} sjhr_rec;

	/*================================
	| Service Job Disbursement Cost. |
	================================*/
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
		{"sjdc_est_cost"},
		{"sjdc_act_cost"},
		{"sjdc_chg_cost"},
	};

	const int sjdc_no_fields = 11;

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
		double	dc_est_cost;
		double	dc_act_cost;
		double	dc_chg_cost;
	} sjdc_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] =
    {
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
	};

	const int cumr_no_fields = 5;

	struct 
    {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
	} cumr_rec;

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
		{"sumr_acronym"},
	};

	const int sumr_no_fields = 6;

	struct 
    {
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		long	sm_hhsu_hash;
		char	sm_name[41];
		char	sm_acronym[10];
	} sumr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char	dummy[11];
	char	c_client[7];
	char	e_client[7];
	char	chg_client[41];
	char	end_client[41];
	char	lr_po_no[9];
	char	lr_desc[31];
	char	lr_supplier_no[7];
	char	lr_invoice_no[9];
	char	name[24];
	double	lr_est_cost;		/*  Money field  */
	double	lr_act_cost;		/*  Money field  */
	double	lr_chg_cost;		/*  Money field  */
	long	hhsu_hash;
} local_rec;

static struct var vars[] =
{	
	{1, LIN, "service_no", 4, 20, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "Service Job No ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "charge_to", 5, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge To ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 6, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 7, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Client ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 8, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{2, TAB, "po_no", MAXLINES, 0, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", " ", "P/O No  ", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.lr_po_no}, 
	{2, TAB, "po_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Description                  ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.lr_desc}, 
	{2, TAB, "supplier", 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " Supplier", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.lr_supplier_no}, 
	{2, TAB, "name", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Name                  ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.name}, 
	{2, TAB, "invoice", 0, 0, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "0", " Invoice ", " ", 
		NO, NO, JUSTLEFT, "", "", local_rec.lr_invoice_no}, 
	{2, TAB, "cost_est", 0, 0, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Est Cost ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.lr_est_cost}, 
	{2, TAB, "cost_act", 0, 0, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Act Cost ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.lr_act_cost}, 
	{2, TAB, "cost_chg", 0, 0, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Charge Amt ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.lr_chg_cost}, 
	{2, TAB, "hhsu_hash", 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhsu_hash}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*===============================
|   Local function prototypes   |
===============================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  find_cumr (long hhcu_hash);
int  get_disbursements (void);
int  update (void);
void order_search (char *key_val);
void po_search (char *key_val);
int  heading (int scn);

/*  QUERY
    cute trick this one...
    it MUST be here for this to work properly
*/
#include <FindSumr.h>

/*----------------------------
| Main Processing Routine.   |
----------------------------*/
int
main (
 int argc,
 char *argv[])
{
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	swide ();

	envDbCo = atoi (get_env ("CR_CO"));
	cr_find = atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.test_no);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
    /*  QUERY
        who initializes prog_exit?
        are we to assume that prog_exit is already initialized 
        properly?
    */
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
        {
            continue;
        }

		get_disbursements ();

		/*-------------------------------
		| Enter screen 2 tabular input .|	
		-------------------------------*/
		if (new_item)
		{
			init_vars (2);
			heading (2);
			scn_display (2);
			entry (2);
			if (restart)
            {
				continue;
            }
		}

		edit_all ();
		if (restart)
        {
			continue;
        }

		update ();
	}	/* end of input control loop	*/

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
	open_rec (sjdc, sjdc_list, sjdc_no_fields, "sjdc_id_no");
	open_rec (cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec (sumr, sumr_list, sumr_no_fields, (!cr_find) ? "sumr_id_no" : "sumr_id_no3");
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
	abc_fclose (sjdc);
	abc_fclose (sumr);
	abc_dbclose (data);
}
int
spec_valid (
 int field)
{
	if (LCHECK ("service_no"))
	{
		if (SRCH_KEY)
		{
			order_search (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"u");
		if (cc)
		{
			/*------------------------
			| Service Job not found. |
			------------------------*/
			errmess (ML (mlSjMess004));
			return (EXIT_FAILURE);
		}
		if (HELD_JOB)
		{
			/*---------------------
			| Service Job is held |
			---------------------*/
			errmess (ML (mlSjMess040));
			return (EXIT_FAILURE);
		}
		if (!OPEN_JOB)
		{
			/*-----------------------
			| Service Job is closed |
			-----------------------*/
			errmess (ML (mlSjMess005));
			return (EXIT_FAILURE);
		}

		if (find_cumr (sjhr_rec.hr_chg_client))
		{
			/*---------------------
			| Customer not found. |
			---------------------*/
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		
		strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
		strcpy (local_rec.chg_client,cumr_rec.cm_dbt_name);

		if (find_cumr (sjhr_rec.hr_end_client))
		{
			sprintf (local_rec.e_client,"%-6.6s"," ");
			sprintf (local_rec.end_client,"%-40.40s"," ");
		}
		else
		{
			strcpy (local_rec.e_client,cumr_rec.cm_dbt_no);
			strcpy (local_rec.end_client,cumr_rec.cm_dbt_name);
		}

		DSP_FLD ("charge_to");
		DSP_FLD ("charge_to_name");
		DSP_FLD ("end_client");
		DSP_FLD ("end_client_name");
		
		return (EXIT_SUCCESS);
	}

	if  (LCHECK ("po_no"))
	{
		if (SRCH_KEY)
		{
			po_search (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
		strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
		strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
		strcpy (sjdc_rec.dc_po_no,local_rec.lr_po_no);

		cc = find_rec (sjdc,&sjdc_rec,COMPARISON,"r");
		if (cc)
        {
			new_item = TRUE;
        }
		else
        {
			strcpy (local_rec.lr_desc,sjdc_rec.dc_desc);
        }

		DSP_FLD ("po_no");
		DSP_FLD ("po_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("supplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.sm_co_no,comm_rec.tco_no);
		strcpy (sumr_rec.sm_est_no,branchNumber);
		strcpy (sumr_rec.sm_crd_no,pad_num (local_rec.lr_supplier_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Supplier not found. |
			---------------------*/
			errmess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.name,"%-23.23s",sumr_rec.sm_name);
		local_rec.hhsu_hash = sumr_rec.sm_hhsu_hash;
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
find_cumr (
 long hhcu_hash)
{
	return (find_hash (cumr, &cumr_rec,COMPARISON,"r", hhcu_hash));
}

/*  QUERY
    assumed return values of 0 mean EXIT_SUCCESS and
    assumed return values of 1 mean EXIT_FAILURE.
    this is based on the program flow.
*/
int
get_disbursements (
 void)
{
	/*========================
	|  read job disbursements|
	========================*/
	scn_set (2);
	lcount[2] = 0;
	new_item = TRUE;

	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf (sjdc_rec.dc_po_no,"%-8.8s"," ");

	cc = find_rec (sjdc,&sjdc_rec,GTEQ,"u");
	while (!cc && 
           !strcmp (sjhr_rec.hr_co_no,comm_rec.tco_no) && 
           !strcmp (sjhr_rec.hr_est_no,comm_rec.test_no) && 
           !strcmp (sjhr_rec.hr_dp_no,comm_rec.tdp_no) && 
           (sjdc_rec.dc_order_no == sjhr_rec.hr_order_no))
	{
		/*==============================================
		| add next description line to tabular memory  |
		==============================================*/
		strcpy (local_rec.lr_po_no,sjdc_rec.dc_po_no);
		strcpy (local_rec.lr_desc,sjdc_rec.dc_desc);
		abc_selfield (sumr,"sumr_hhsu_hash");

		cc = find_hash (sumr, &sumr_rec, COMPARISON, "r",sjdc_rec.dc_hhsu_hash);
		if (cc)
		{
			/*---------------------
			| Supplier not found. |
			---------------------*/
			errmess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.lr_supplier_no,sumr_rec.sm_crd_no);
		sprintf (local_rec.name,"%-23.23s",sumr_rec.sm_name);
		strcpy (local_rec.lr_invoice_no,sjdc_rec.dc_invoice_no);
		local_rec.hhsu_hash = sjdc_rec.dc_hhsu_hash;
		local_rec.lr_est_cost = sjdc_rec.dc_est_cost;
		local_rec.lr_act_cost = sjdc_rec.dc_act_cost;
		local_rec.lr_chg_cost = sjdc_rec.dc_chg_cost;
		putval (lcount[2]++);
		new_item = FALSE;
		cc = find_rec (sjdc,&sjdc_rec,NEXT,"u");
	}
	abc_selfield (sumr, (!cr_find) ? "sumr_id_no" : "sumr_id_no3");
	scn_set (1);

    return (EXIT_SUCCESS);
}

int
update (
 void)
{
	int	i = 0;
	int	adding = 0;

	/*===========================
	|  update job disbursements |
	===========================*/
	print_at (2,40, "%s\n", ML (mlStdMess035));
	fflush (stdout);
		
	scn_set (2);
	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
	sprintf (sjdc_rec.dc_po_no,"%-8.8s"," ");
	cc = find_rec (sjdc,&sjdc_rec,GTEQ,"u");
	if (cc)
    {
		adding = 1;
    }

	for (i = 0;i < lcount[2];i++)
	{
		getval (i);
		if (adding)
		{
			strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
			strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
			strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
			sjdc_rec.dc_order_no = sjhr_rec.hr_order_no;
			strcpy (sjdc_rec.dc_po_no,local_rec.lr_po_no);
			strcpy (sjdc_rec.dc_desc,local_rec.lr_desc);
			sjdc_rec.dc_hhsu_hash = local_rec.hhsu_hash;
			strcpy (sjdc_rec.dc_invoice_no,local_rec.lr_invoice_no);
			sjdc_rec.dc_est_cost = local_rec.lr_est_cost;
			sjdc_rec.dc_act_cost = local_rec.lr_act_cost;
			sjdc_rec.dc_chg_cost = local_rec.lr_chg_cost;
	
            cc = abc_add (sjdc,&sjdc_rec);
			if (cc)
            {
                sys_err ("Error in sjdc During (DBADD)",cc,PNAME);
            }
		}
		else
		{
			strcpy (sjdc_rec.dc_po_no,local_rec.lr_po_no);
			strcpy (sjdc_rec.dc_desc,local_rec.lr_desc);
			sjdc_rec.dc_hhsu_hash = local_rec.hhsu_hash;
			strcpy (sjdc_rec.dc_invoice_no,local_rec.lr_invoice_no);
			sjdc_rec.dc_est_cost = local_rec.lr_est_cost;
			sjdc_rec.dc_act_cost = local_rec.lr_act_cost;
			sjdc_rec.dc_chg_cost = local_rec.lr_chg_cost;
			cc = abc_update (sjdc,&sjdc_rec);
			if (cc)
            {
                sys_err ("Error in sjdc During (DBUPDATE)",cc,PNAME);
            }
			abc_unlock (sjdc);

			cc = find_rec (sjdc,&sjdc_rec,NEXT,"u");
			if (cc)
            {
                adding = 1;
            }
		}
	}

	abc_unlock (sjdc);

	scn_set (1);

	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
order_search (
 char *key_val)
{
	char	order_str[9];

	work_open ();
	save_rec ("#Job No","#Issued on");
	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol (key_val);

	cc = find_rec (sjhr, &sjhr_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (sjhr_rec.hr_co_no,comm_rec.tco_no) && 
           !strcmp (sjhr_rec.hr_est_no,comm_rec.test_no) && 
           !strcmp (sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{
		sprintf (order_str,"%8ld",sjhr_rec.hr_order_no);
		if ((!strlen(key_val) || 
             !strncmp(order_str,key_val,strlen (key_val))) && 
		      OPEN_JOB)
		{
			strcpy (err_str, DateToString (sjhr_rec.hr_issue_date));
			cc = save_rec (order_str,err_str);
			if (cc)
            {
                break;
            }
		}
		cc = find_rec (sjhr, &sjhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
        return;
    }
	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol (temp_str);
	cc = find_rec (sjhr, &sjhr_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err ("Error in sjhr During (DBFIND)", cc, PNAME);
    }
}

void
po_search (
 char *key_val)
{
	work_open ();
	save_rec ("#P/O No","#Description");
	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sprintf (sjdc_rec.dc_po_no,"%-8.8s",key_val);
	cc = find_rec (sjdc, &sjdc_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (sjdc_rec.dc_co_no,comm_rec.tco_no) && 
           !strcmp (sjdc_rec.dc_est_no,comm_rec.test_no) && 
           !strcmp (sjdc_rec.dc_dp_no,comm_rec.tdp_no) && 
           !strncmp (sjdc_rec.dc_po_no,key_val,strlen (key_val)))
	{
		cc = save_rec (sjdc_rec.dc_po_no,clip (sjdc_rec.dc_desc));
		if (cc)
        {
            break;
        }
		cc = find_rec (sjdc, &sjdc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
    {
        return;
    }
	strcpy (sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy (sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy (sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sprintf (sjdc_rec.dc_po_no,"%-8.8s",temp_str);
	cc = find_rec (sjdc, &sjdc_rec, COMPARISON, "r");
	if (cc)
    {
        sys_err ("Error in sjdc During (DBFIND)", cc, PNAME);
    }
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
            scn_set (scn);
        }

		swide ();
		clear ();
		/*-----------------------------------
		| Service Job Outside Purchases Log |
		-----------------------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess041));
		rv_pr (err_str, 48,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
        {
            box (0,3,132,5);
        }

		move (0,20);
		line (132);
		sprintf (err_str, ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at (21,0, "%s", err_str);
		print_at (22,0, ML (mlStdMess039),comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

/* [ end of file ] */
