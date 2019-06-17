/*====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_cstrp.i.c  )                                  |
|  Program Desc  : ( Select Job to produce a costing for          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 21/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : 17/10/88            | By : B.C Lim.                |
|  Date Modified : 21/11/88            | By : Fui Choo Yap.           |
|  Date Modified : 24/11/88            | By : Bee Chwee Lim.          |
|  Date Modified : 27/11/89            | By : Fui Choo Yap.           |
|  Date Modified : (18/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : Tidy up the program.                               |
|                :                                                    |
|     (27/11/89) : Change money fields to double.                     |
|                : (18/09/90) - General Update for New Scrgen. S.B.D. |
|  (05/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: sj_cstrp.i.c,v $
| Revision 5.3  2002/07/17 09:57:48  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:04  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:12  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:39  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:40  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/17 06:40:43  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/16 05:58:29  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.7  1999/10/20 02:07:03  nz
| Updated for final changes on date routines.
|
| Revision 1.6  1999/09/29 10:12:54  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/24 05:06:28  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:25  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                        
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_cstrp.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_cstrp.i/sj_cstrp.i.c,v 5.3 2002/07/17 09:57:48 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#define MAXWIDTH	150

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/

/*  QUERY
    these should be declared as const char*
    to minimize potential problems.
*/
char *cumr = "cumr";
char *sjhr = "sjhr";
char *sjjd = "sjjd";
char *data = "data";

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
		{"sjhr_cust_type"},
		{"sjhr_cust_ord_no"},
		{"sjhr_cost_estim"},
		{"sjhr_issue_date"},
		{"sjhr_reqd_date"},
	};

	const int sjhr_no_fields = 12;

	struct 
    {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_type[2];
		char	hr_cust_ord_no[11];
		double	hr_cost_estim;
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
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_class_type"},
	};

	const int cumr_no_fields = 7;

	struct 
    {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
	} cumr_rec;

	/*===========================
	| special fields & flags    |
	===========================*/
	char  	systemDate[11];
	char  	branchNo[3];

	int		envDbCo = 0;
	int		envDbFind  = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char    c_client[7];
  	char    e_client[7];
	char    chg_client[41];
  	char    end_client[41];
	char	order_no[9];
	char	stat_desc[21];
	char    job_detail[7][71];
	char	dummy[11];
	char	rtype[9];
	int	    lpno;
	char  	lp_no[3];
} local_rec;

static struct var vars[] =
{	
	{1, LIN, "order_no", 4, 20, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "Order No ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "curr_stat", 4, 50, CHARTYPE, 
		"U", "          ", 
		" ", "", "Status ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_status}, 
	{1, LIN, "stat_desc", 4, 82, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.stat_desc}, 
	{1, LIN, "charge_to", 5, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge To ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 5, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 6, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Client ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 6, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{1, LIN, "cust_order_no", 7, 20, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "Cust Order No", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_cust_ord_no}, 
	{1, LIN, "comp_date", 8, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Completion Date", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_reqd_date}, 
	{1, LIN, "cost_est", 9, 20, DOUBLETYPE, 
		"NNNNN.NN", "          ", 
		" ", "0", "Cost Estimate", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_cost_estim}, 
	{1, LIN, "desc", 10, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[0]}, 
	{1, LIN, "desc1", 11, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[1]}, 
	{1, LIN, "desc2", 12, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[2]}, 
	{1, LIN, "desc3", 13, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[3]}, 
	{1, LIN, "desc4", 14, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[4]}, 
	{1, LIN, "desc5", 15, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[5]}, 
	{1, LIN, "desc6", 16, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[6]}, 
	{1, LIN, "print_display", 17, 20, CHARTYPE, 
		"U", "          ", 
		" ", "D(isplay", "Report Type ", "P(rint or D(isplay", 
		NO, NO, JUSTRIGHT, "DP", " ", local_rec.rtype}, 
	{1, LIN, "printer", 18, 20, INTTYPE, 
		"NN", "          ", 
		" ", "", "Printer Number", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
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
void get_job_dets (void);
int  get_chg_client (void);
int  get_end_client (void);
void get_stat_desc (void);
void proc_request (void);
int  heading (int scn);
void order_srch (char *key_val);

/*  QUERY
    cute trick this one...
*/
#include <FindCumr.h>

/*=========================
| Main Processing Routine |
==========================*/
int 
main (
 int   argc,
 char *argv[])
{
	SETUP_SCR (vars);

	strcpy (systemDate, DateToString (TodaysDate()));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

    OpenDB ();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

    swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
    /*  QUERY
        where is prog_exit initialized?
        are we to assume that prog_exit is already 
        initialized when we start?
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

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
        {
			continue;
        }

		/*--------------------------------
		| Print Service Order Job Sheet. |
		--------------------------------*/
		proc_request ();
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
    open_rec (cumr, cumr_list, 
              cumr_no_fields, 
              (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sjhr);
	abc_fclose (sjjd);
	abc_fclose (cumr);
	abc_dbclose (data);
}
int
spec_valid (
 int field)
{
	if (LCHECK ("order_no"))
	{
		if (SRCH_KEY)
		{
			order_srch (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy (sjhr_rec.hr_est_no, comm_rec.test_no);
		strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);

		cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"r");
		if (cc)
		{
			/*------------------
			| Order not found. |
			------------------*/
			errmess (ML (mlStdMess122));
			return (EXIT_FAILURE);
		}
		if ((sjhr_rec.hr_status[0] != 'O') && 
            (sjhr_rec.hr_status[0] != 'C'))
		{
			/*--------------------------------
			| Order %ld status is not C or O |
			--------------------------------*/
			sprintf (err_str, ML (mlSjMess042), sjhr_rec.hr_order_no);
			errmess (err_str);
			return (EXIT_FAILURE);
		}

		abc_selfield (cumr,"cumr_hhcu_hash");
		cc = get_chg_client ();
		if (cc)
		{
			/*---------------------
			| Customer not found. |
			---------------------*/
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		cc = get_end_client ();
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}

		abc_selfield (cumr,(envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
		get_stat_desc ();

		strcpy (local_rec.rtype,"D(isplay");
		skip_entry = 18;
		FLD ("printer")	= NA;

		/*===================
		|  read job details |
		===================*/
		get_job_dets ();
		scn_display (1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_client"))
	{
		if (vars[label ("end_client")].required == NA)
        {
			return (EXIT_SUCCESS);
        }

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no,branchNo);
		strcpy (cumr_rec.cm_dbt_no,pad_num (local_rec.e_client));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("print_display"))
	{
		if (local_rec.rtype[0] == 'D')
        {
			FLD ("printer")	= NA;
        }
		else
        {
			FLD ("printer")	= YES;
        }

		strcpy (local_rec.rtype, (local_rec.rtype[0] == 'D') ? "D(isplay" : "P(rint  ");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printer"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*------------------
			| Invalid printer. |
			------------------*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
get_job_dets (
 void)
{
	int	j = 0;

	strcpy (sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy (sjjd_rec.jd_est_no, comm_rec.test_no);
	strcpy (sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;
	cc = find_rec (sjjd,&sjjd_rec,GTEQ,"r");

	while (!cc && 
           !strcmp (sjjd_rec.jd_co_no,comm_rec.tco_no) && 
           !strcmp (sjjd_rec.jd_est_no,comm_rec.test_no) && 
           !strcmp (sjjd_rec.jd_dp_no,comm_rec.tdp_no) && 
           (sjjd_rec.jd_order_no == sjhr_rec.hr_order_no))
	{
		strcpy (local_rec.job_detail[j++],sjjd_rec.jd_detail);
		cc = find_rec (sjjd,&sjjd_rec,NEXT,"r");
	}
}

/*  QUERY
    assumed return values of 0 mean EXIT_SUCCESS and
    assumed return values of 1 mean EXIT_FAILURE.
    this is based on the program flow.
*/
int
get_chg_client (
 void)
{
	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sjhr_rec.hr_chg_client);
	if (cc)
    {
        return (cc);
    }
	    
	strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.cm_name);
	if (strcmp (cumr_rec.cm_class_type,"INT") !=  0)
	{
		strcpy (sjhr_rec.hr_cust_type,"E");
		FLD ("end_client")	= NA;
	}
	else
	{
		strcpy (sjhr_rec.hr_cust_type ,"I");
		FLD ("end_client")	= YES;
	}

	return (EXIT_SUCCESS);
}

/*  QUERY
    assumed return values of 0 mean EXIT_SUCCESS and
    assumed return values of 1 mean EXIT_FAILURE.
    this is based on the program flow.
*/
int
get_end_client (
 void)
{
	if (sjhr_rec.hr_end_client == 0L)
    {
		return (EXIT_SUCCESS);
    }

	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sjhr_rec.hr_end_client);
	if (cc)
    {
		return (cc);
    }
	
	strcpy (local_rec.e_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.end_client,cumr_rec.cm_name);

    return (EXIT_SUCCESS);
}

void
get_stat_desc (
 void)
{
	switch (sjhr_rec.hr_status[0])
	{ 
        case 'O':
			strcpy (local_rec.stat_desc,"Open order");
            break;
        case 'C':
			strcpy (local_rec.stat_desc,"Closed order");
            break;
        case 'H':
			strcpy (local_rec.stat_desc,"Order on hold");
            break;

        default:
            /*  QUERY
                added default
                this should be filled with some error handling
                    code or something.
            */
            break;
	}
}

void
proc_request (
 void)
{
	sprintf (local_rec.order_no,"%8ld",sjhr_rec.hr_order_no);
	sprintf (local_rec.lp_no,"%2d",local_rec.lpno);

	print_at (2,40, "%s\n\r", ML (mlStdMess035) );
	fflush (stdout);

	*(arg) = "sj_cstrep";
	*(arg + (1)) = local_rec.rtype;
	*(arg + (2)) = local_rec.order_no;
	*(arg + (3)) = local_rec.lp_no;
	*(arg + (4)) = (char *)0;     /* QUERY is this a null pointer? */

	shell_prog (4);
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
		clear ();

		/*-----------------------------------------------
		| S e r v i c e   J o b   C o s t   S h e e t   |
		| P r i n t / D i s p l a y 					|
		-----------------------------------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess044));
		rv_pr (err_str, 48,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
        {
			box (0,3,132,15);
        }

		move (0,20);
		line (132);
		sprintf (err_str, ML (mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		print_at (21,0, "%s", err_str); 
		print_at (22,0, ML (mlStdMess039), comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
order_srch (
 char *key_val)
{
	char order_str[9];

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
		if (((strlen (key_val) == 0) || 
            !strncmp (order_str,key_val,strlen (key_val))) && 
            (sjhr_rec.hr_status[0] == 'O'))
		{
			strcpy (err_str, DateToString (sjhr_rec.hr_issue_date));
			cc = save_rec (order_str, err_str);
			if (cc)
				break;
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

/* [ end of file ] */
