/*====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_hrclose.c                                     |
|  Program Desc  : ( Close / Reopen Service Job.                  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, cumr,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjhr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 01/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : 17/11/88        | Modified By : B.C.Lim.           |
|  Date Modified : 23/11/89        | Modified By : Fui Choo Yap.      |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|  Date Modified : (06/09/1999)    | Modified  by  : edge cabalfin    |
|                                                                     |
|  Comments      : Use new screen generator & tidied up program.      |
|     (23/11/89) : Change MONEYTYPE fields to DOUBLETYPE.             |
|                : (10/10/90) - General Update for New Scrgen. S.B.D. |
|     (16/09/97) : Updated to incorporate multilingual conversion.    |
|                :                                                    |
|   (06/09/1999) : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                                                                     |
| $Log: sj_hrclose.c,v $
| Revision 5.2  2001/08/09 09:17:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:25  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:13  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:40  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:45  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/17 06:40:46  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/16 05:58:30  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.7  1999/10/20 02:07:04  nz
| Updated for final changes on date routines.
|
| Revision 1.6  1999/09/29 10:12:56  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/24 05:06:31  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:27  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_hrclose.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_hrclose/sj_hrclose.c,v 5.2 2001/08/09 09:17:24 scott Exp $";

/*===============================
|   Include file dependencies   |
===============================*/
#define MAXWIDTH	150

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

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


/*=====================
|   Local variables   |
=====================*/
	int		envDbCo = 0,
			envDbFind  = 0;

	char	branchNo[3],
			systemDate[11];

	long	new_hhcu_hash = 0L;

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
	char	new_stat[8];
	char	stat_desc[21];
	char	job_detail[7][71];
} local_rec;

static struct var vars[] =
{	
	{1, LIN, "service_no", 3, 20, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "Service Job No ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "curr_stat", 3, 50, CHARTYPE, 
		"U", "          ", 
		" ", "", "Status ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_status}, 
	{1, LIN, "stat_desc", 3, 82, CHARTYPE, 
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
	{1, LIN, "cust_order_no", 8, 20, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "Cust Order No", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_cust_ord_no}, 
	{1, LIN, "comp_date", 8, 50, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Completion Date", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_reqd_date}, 
	{1, LIN, "cost_est", 8, 82, DOUBLETYPE, 
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
	{1, LIN, "new_stat", 18, 20, CHARTYPE, 
		"U", "          ", 
		" ", "O(pen) ", "New Status ", "Enter O(pen) or C(lose) ", 
		YES, NO, JUSTLEFT, "OC", "", local_rec.new_stat}, 
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
int  get_chg_client (void);
int  get_end_client (void);
int  get_cust (long hhcu_hash);
void get_stat_desc (void);
int  update (void);
void save_page1 (char *key_val);
int  heading (int scn);

/*  QUERY
    cute trick this one...
    this MUST be here for this to work properly.
*/
#include <FindCumr.h>

/*----------------------------
| Main Processing Routine.   |
----------------------------*/
int
main (
 int agrc,
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

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);
	strcpy (systemDate, DateToString (TodaysDate()));

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
    /*  QUERY
        who initializes prog_exit?
        are we to assume that prog_exit is initialized properly?
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
	print_at (0,0,ML (mlStdMess035));
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

	read_comm (comm_list, comm_no_fields,  (char *) &comm_rec);

    open_rec (sjhr, sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec (sjjd, sjjd_list, sjjd_no_fields, "sjjd_id_no");
	open_rec (cumr, cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
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
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	i = 0;

	if (LCHECK ("service_no"))
	{
		if (SRCH_KEY)
		{
			save_page1 (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"u");
		if (cc)
		{
			errmess (ML (mlSjMess004));
			return (EXIT_FAILURE);
		}

		if  ((sjhr_rec.hr_status[0] != 'O') && 
		     (sjhr_rec.hr_status[0] != 'C'))
		{
			errmess (ML (mlSjMess025));
			return (EXIT_FAILURE);
		}

		get_chg_client ();
		get_end_client ();
		get_stat_desc ();

		/*===================
		|  read job details |
		===================*/
		strcpy (sjjd_rec.jd_co_no,comm_rec.tco_no);
		strcpy (sjjd_rec.jd_est_no,comm_rec.test_no);
		strcpy (sjjd_rec.jd_dp_no,comm_rec.tdp_no);
		sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
		sjjd_rec.jd_line_no = 0;
		cc = find_rec (sjjd,&sjjd_rec,GTEQ,"u");

		while (!cc && 
               !strcmp (sjjd_rec.jd_co_no,comm_rec.tco_no) && 
               !strcmp (sjjd_rec.jd_est_no,comm_rec.test_no) && 
               !strcmp (sjjd_rec.jd_dp_no,comm_rec.tdp_no) && 
               (sjjd_rec.jd_order_no == sjhr_rec.hr_order_no))
		{
			/*=======================
			| get description lines |
			========================*/
			strcpy (local_rec.job_detail[i++],sjjd_rec.jd_detail);
			cc = find_rec (sjjd,&sjjd_rec,NEXT,"u");
		}
		scn_display (1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_client"))
	{
		if (sjhr_rec.hr_end_client == 0L)
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
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		new_hhcu_hash = cumr_rec.cm_hhcu_hash;
		strcpy (local_rec.end_client,cumr_rec.cm_name);
		display_field (label ("end_client_name"));
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_stat"))
	{
		strcpy (local_rec.new_stat, 
                (local_rec.new_stat[0] == 'O') ? "O(pen) " : "C(lose)");

		DSP_FLD ("new_stat");
	}
	return (EXIT_SUCCESS);
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
	abc_selfield (cumr,"cumr_hhcu_hash");
	cc = get_cust (sjhr_rec.hr_chg_client);
	if (cc)
    {
		return (cc);
    }
	
	strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.cm_name);
	if (strcmp (cumr_rec.cm_class_type,"INT") !=  0 )
	{
		strcpy (sjhr_rec.hr_cust_type,"E");
		FLD ("end_client")	= NA;
	}
	else
	{
		strcpy (sjhr_rec.hr_cust_type ,"I");
		FLD ("end_client")	= YES;
	}
	abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
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

	abc_selfield (cumr,"cumr_hhcu_hash");
	cc = get_cust (sjhr_rec.hr_end_client);
	if (cc)
    {
		return (cc);
    }
	
	new_hhcu_hash = cumr_rec.cm_hhcu_hash;
	strcpy (local_rec.e_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.end_client,cumr_rec.cm_name);
	abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
	return (EXIT_SUCCESS);
}

/*  QUERY
    assumed return values of 0 mean EXIT_SUCCESS and
    assumed return values of 1 mean EXIT_FAILURE.
    this is based on the program flow.
*/
int
get_cust (
 long hhcu_hash)
{
	cc = find_hash (cumr, &cumr_rec, COMPARISON, "r",hhcu_hash);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
get_stat_desc (
 void)
{
	switch (sjhr_rec.hr_status[0])
	{
		case 'O':
			strcpy (local_rec.stat_desc,"Open Service Job");
			break;
		case 'C':
			strcpy (local_rec.stat_desc,"Closed Service Job");
			break;
		case 'H':
			strcpy (local_rec.stat_desc,"Service Job on hold");
			break;
        default:
            /*  QUERY
                added default.
                this is a good place to put error checking 
                    code or something...
            */
            break;
	}
}

int
update (
 void)
{
	/*=====================
	|   update job header |
	=====================*/

	sjhr_rec.hr_end_client = new_hhcu_hash;
	sprintf (sjhr_rec.hr_status,"%1.1s",local_rec.new_stat);
	cc = abc_update (sjhr,&sjhr_rec);
	if (cc)
    {
        sys_err ("Error in sjhr During (DBUPDATE)",cc,PNAME);
    }

	abc_unlock (sjhr);
	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
save_page1 (
 char *key_val)
{
	long	atol (const char *);
	char	order_str[9];
	char	date_str[11];

	work_open ();
	save_rec ("#Srv Job No","#Issued on");
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
		sprintf (date_str,"%-10.10s",DateToString (sjhr_rec.hr_issue_date));

		if ((strlen (key_val) == 0 || 
            !strncmp (order_str,key_val,strlen (key_val))) && 
            ((sjhr_rec.hr_status[0] == 'O') || 
            (sjhr_rec.hr_status[0] == 'C')))
		{
			cc = save_rec (order_str,date_str);
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
		rv_pr (ML (mlSjMess026),60,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
		{
			box (0,2,132,16);
			move (1,4);line (131);
			move (1,7);line (131);
			move (1,9);line (131);
			move (1,17);line (131);
		}

		move (0,20);
		line (132);
		print_at (21,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

/* [ end of file ] */
