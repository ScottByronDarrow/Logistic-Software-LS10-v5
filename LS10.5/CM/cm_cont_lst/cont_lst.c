/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_cont_lst.c  )                                 |
|  Program Desc  : ( Contracts Master Listing.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmhr, cmcd, cmcd,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Anneliese Allen | Date Written  : 14/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (10/06/93)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (30/11/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (10/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      :                                                    |
|    (10/06/93)  : Updated to fix incorrect printer checks.           |
|                : S/C EGC-9060                                       |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|  (10/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
| $Log: cont_lst.c,v $
| Revision 5.3  2002/07/17 09:56:59  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:24  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:01  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:21  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.18  1999/12/06 01:32:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.17  1999/11/29 20:25:19  cam
| Changes for GVision compatibility
|
| Revision 1.16  1999/11/17 06:39:07  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.15  1999/11/08 04:35:37  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.14  1999/10/20 01:40:30  nz
| Updated for remainder of date routines
|
| Revision 1.13  1999/10/12 05:17:46  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.12  1999/10/01 07:48:19  scott
| Updated for standard function calls.
|
| Revision 1.11  1999/09/29 10:10:14  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 04:40:08  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.9  1999/09/16 04:44:41  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/14 07:34:07  scott
| Updated to add log in heading + updated for new gcc compiler.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: cont_lst.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_cont_lst/cont_lst.c,v 5.3 2002/07/17 09:56:59 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>

#define	MANUAL		0
#define	BRANCH		1
#define	COMPANY		2

#define	SEL_PSIZE	40

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	co_no[3];
		char	co_name[41];
		char	est_no[3];
		char	est_name[41];
		long	crd_date;
		int		fiscal;
	} comm_rec;

	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	21

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_mast_hhhr"},
		{"cmhr_hhcu_hash"},
		{"cmhr_hhit_hash"},
		{"cmhr_cus_ref"},
		{"cmhr_contact"},
		{"cmhr_wip_date"},
		{"cmhr_due_date"},
		{"cmhr_end_date"},
		{"cmhr_hhjt_hash"},
		{"cmhr_wip_glacc"},
		{"cmhr_lab_glacc"},
		{"cmhr_o_h_glacc"},
		{"cmhr_sal_glacc"},
		{"cmhr_cog_glacc"},
		{"cmhr_wip_status"},
		{"cmhr_status"},
		{"cmhr_premise"},
	};

	struct tag_cmhrRecord
	{
		char	co_no[3];
		char	br_no[3];
		char	cont_no[7];
		long	hhhr_hash;
		long	mast_hhhr;
		long	hhcu_hash;
		long	hhit_hash;
		char	cus_ref[21];
		char	contact[41];
		long	wip_date;
		long	due_date;
		long	end_date;
		long	hhjt_hash;
		char	wip_glacc[17];
		char	lab_glacc[17];
		char	o_h_glacc[17];
		char	sal_glacc[17];
		char	cog_glacc[17];
		char	wip_status[5];
		char	status[2];
		char	premise[21];
	} cmhr_rec, cmhr2_rec;


	/*==========================================+
	 | Contract Management Job Type Master File |
	 +==========================================*/
#define	CMJT_NO_FIELDS	5

	struct dbview	cmjt_list [CMJT_NO_FIELDS] =
	{
		{"cmjt_co_no"},
		{"cmjt_br_no"},
		{"cmjt_job_type"},
		{"cmjt_hhjt_hash"},
		{"cmjt_desc"},
	};

	struct tag_cmjtRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	job_type [5];
		long	hhjt_hash;
		char	desc [31];
	}	cmjt_rec;


	/*===============================================+
	 | Contract Management Contract Description File |
	 +===============================================*/
#define	CMCD_NO_FIELDS	4

	struct dbview	cmcd_list [CMCD_NO_FIELDS] =
	{
		{"cmcd_hhhr_hash"},
		{"cmcd_line_no"},
		{"cmcd_text"},
		{"cmcd_stat_flag"}
	};

	struct tag_cmcdRecord
	{
		long	hhhr_hash;
		int		line_no;
		char	text [71];
		char	stat_flag [2];
	}	cmcd_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
	};

	int	cumr_no_fields = 6;

	struct	{
		char	co_no[3];
		char	est_no[3];
		char	department[3];
		char	dbt_no[7];
		long	hhcu_hash;
		char	dbt_name[41];
	} cumr_rec;

	/*=======================================+
	 | Contract Management Transaction File. |
	 +=======================================*/
#define	CMTR_NO_FIELDS	9

	struct dbview	cmtr_list [CMTR_NO_FIELDS] =
	{
		{"cmtr_hhhr_hash"},
		{"cmtr_hhcm_hash"},
		{"cmtr_hhbr_hash"},
		{"cmtr_qty"},
		{"cmtr_cost_price"},
		{"cmtr_sale_price"},
		{"cmtr_disc_pc"},
		{"cmtr_date"},
		{"cmtr_stat_flag"}
	};

	struct tag_cmtrRecord
	{
		long	hhhr_hash;
		long	hhcm_hash;
		long	hhbr_hash;
		float	qty;
		double	cost_price;		/* money */
		double	sale_price;		/* money */
		float	disc_pc;
		long	date;
	}	cmtr_rec;


	/*===========================================+
	 | Contract Management Time Sheet Trans File |
	 +===========================================*/
#define	CMTS_NO_FIELDS	11

	struct dbview	cmts_list [CMTS_NO_FIELDS] =
	{
		{"cmts_date"},
		{"cmts_hhhr_hash"},
		{"cmts_hhcm_hash"},
		{"cmts_time_ord"},
		{"cmts_time_hlf"},
		{"cmts_time_dbl"},
		{"cmts_units"},
		{"cmts_lab_cost"},
		{"cmts_oh_cost"},
		{"cmts_sale"},
		{"cmts_stat_flag"}
	};

	struct tag_cmtsRecord
	{
		long	date;
		long	hhhr_hash;
		long	hhcm_hash;
		float	time_ord;
		float	time_hlf;
		float	time_dbl;
		float	units;
		double	lab_cost;		/* money */
		double	oh_cost;		/* money */
		double	sale;		/* money */
		char	stat_flag [2];
	}	cmts_rec;

	/*===============================================+
	 | Contract Management Costheads/Contract Budget |
	 +===============================================*/
#define	CMCB_NO_FIELDS	5

	struct dbview	cmcb_list [CMCB_NO_FIELDS] =
	{
		{"cmcb_hhhr_hash"},
		{"cmcb_hhcm_hash"},
		{"cmcb_budg_cost"},
		{"cmcb_budg_qty"},
		{"cmcb_budg_value"},
	};

	struct tag_cmcbRecord
	{
		long	hhhr_hash;
		long	hhcm_hash;
		double	budg_cost;		/* money */
		float	budg_qty;
		double	budg_value;		/* money */
	}	cmcb_rec;

	/*===========================================+
	 | Contract Management Progress Billing File |
	 +===========================================*/
#define	CMPB_NO_FIELDS	5

	struct dbview	cmpb_list [CMPB_NO_FIELDS] =
	{
		{"cmpb_hhhr_hash"},
		{"cmpb_date"},
		{"cmpb_inv_date"},
		{"cmpb_amount"},
		{"cmpb_amt_rem"},
	};

	struct tag_cmpbRecord
	{
		long	hhhr_hash;
		long	date;
		long	inv_date;
		double	amount;		/* money */
		double	amt_rem;		/* money */
	}	cmpb_rec;


	int		lpno = 1,
			line_no = 0;

	FILE	*fsort,
			*pout;

	char	*sptr,
			sorted_by[12];
	char	prog_desc[100];
	
	char	*comm = "comm",
			*cmhr = "cmhr",
			*cmhr2 = "cmhr2",
			*cmcd = "cmcd",
			*cmjt = "cmjt",
			*cumr = "cumr",
			*cmtr = "cmtr",
			*cmts = "cmts",
			*cmcb = "cmcb",
			*cmpb = "cmpb",
			*data = "data";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[8];
	char	onight[8];
	int		lpno;	
	char	sort[2];
	char	sort_desc[12];
	char	st_job[5];
	char	st_job_desc[41];
	char	end_job[5];
	char	end_job_desc[41];
	char	st_con[7];
	char	st_con_desc1[71];
	char	st_con_desc2[71];
	char	end_con[7];
	char	end_con_desc1[71];
	char	end_con_desc2[71];
	char	status[2];
	char	status_desc[8];
	long	lsystemDate;
	char	systemDate[11];
	long	st_mth;
	long	st_yr;
	long	cmpb_date;
	double	mtd_sales;
	double	td_sales;
	double	mtd_cost;
	double	td_cost;
	double	mtd_qty;
	double	td_qty;
	double	budg_sales;
	double	budg_cost;
	double	sales_amt;
	double	cost_amt;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "sort",	 3, 31, CHARTYPE,
		"U", "          ",
		" ", "C", " Sort By Contracts/Finish Date :", "Enter C(ontract) or F(inish). ",
		YES, NO,  JUSTLEFT, "CcFf", "", local_rec.sort},
	{1, LIN, "sort_desc",	3, 34, CHARTYPE,
		"UAAAAAAAAAA", "          ",
		" ", "Contracts  ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.sort_desc},
	{1, LIN, "st_job",	5, 25, CHARTYPE,
		"UUUU", "          ",
		" ","    ", " Start Job Type       :", "Enter the starting job type",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_job},
	{1, LIN, "st_job_desc",	5, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_job_desc},
	{1, LIN, "end_job",	6, 25, CHARTYPE,
		"UUUU", "          ",
		" ", "~~~~", " End Job Type         :", "Enter the ending job type",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_job},
	{1, LIN, "end_job_desc",	6, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_job_desc},
	{1, LIN, "st_con",	8, 25, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", " Start Contract No.   :", "Enter Starting Contract Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_con},
	{1, LIN, "st_con_desc1",	9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Description 1  :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_con_desc1},
	{1, LIN, "st_con_desc2",	10, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "       Description 2  :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_con_desc2},
	{1, LIN, "end_con",	11, 25, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", " End Contract No.     :", "Enter End Contract Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_con},
	{1, LIN, "end_con_desc1",12, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Description 1  :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_con_desc1},
	{1, LIN, "end_con_desc2", 13, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "       Description 2  :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_con_desc2},
	{1, LIN, "status",	 15, 25, CHARTYPE,
		"U", "          ",
		" ", "A", " Contract Status      :", "Enter Contract Status. Default is A(ll).",
		YES, NO, JUSTRIGHT, "OCHA", "", local_rec.status},
	{1, LIN, "status_desc",	 15, 28, CHARTYPE,
		"UAAAAAA", "          ",
		" ", "All    ", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.status_desc},
	{1, LIN, "lpno",	 	17, 25, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No           :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 	18, 25, CHARTYPE,
		"U", "          ",
		" ", "N", " Background Y/N       :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 19, 25, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight Y/N        :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=========================== 
| Function declarations.     |
===========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
int		spec_valid		(int field);
void	CloseDB		(void);
void	OpenDB			(void);
void	run_prog		(void);
void	proc_file		(void);
void	calc_tots		(long hhhr_hash);
void	start_report	(int prnt_no);
void	proc_line		(void);
void	print_line		(char * tptr);
void	sub_heading		(void);
void	chk_page		(void);
void	end_report		(void);
void	rpt_heading		(void);
int		heading			(int scn);
int		SrchCmjt		(char * key_val);
int		SrchCmhr		(char * key_val);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	if (argc != 2 && argc != 8)
	{
/*
		printf("\007Usage %s <description>\n\r", argv[0]);
		printf(" OR   %s <start job type>\n\r", argv[0]);
		printf("         <end job type>\n\r");
		printf("         <start contract number>\n\r");
		printf("         <end contract number>\n\r");
		printf("         <status> O)pen, C)lose, H)istory, A)ll\n\r");
		printf("         <sort_by> - C)ontracts or F)inish Dates\n\r");
		printf("         <lpno>\n\r");*/
		print_at(0,0,ML(mlCmMess700), argv[0]);
		print_at(1,0,ML(mlCmMess714), argv[0]);

		print_at(2,0,"%20.20s%s"," ",ML(mlCmMess715));
		print_at(3,0,"%20.20s%s"," ",ML(mlCmMess707));
		print_at(4,0,"%20.20s%s"," ",ML(mlCmMess708));
		print_at(5,0,"%20.20s%s"," ",ML(mlCmMess743));
		print_at(6,0,"%20.20s%s"," ",ML(mlCmMess744));
		print_at(7,0,"%20.20s%s"," ",ML(mlCmMess727));
		return (EXIT_SUCCESS);
	}

	OpenDB();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();
	
	if (argc == 8)
	{
		sprintf (local_rec.st_job,  "%4.4s", argv[1]);
		sprintf (local_rec.end_job, "%4.4s", argv[2]);

		sprintf (local_rec.st_con,  "%6.6s", argv[3]);
		sprintf (local_rec.end_con, "%6.6s", argv[4]);

		sprintf (local_rec.status,  "%1.1s", argv[5]);
		sprintf (local_rec.sort, "%1.1s", argv[6]);
		lpno = atoi(argv[7]);

		proc_file();
		shutdown_prog ();
	}

	/*------------------------------
	| Capture program description. |
	------------------------------*/
	sprintf(prog_desc, "%s", argv[1]);

	/*-----------------
	| Prepare screen. |
	-----------------*/
	SETUP_SCR(vars);

	init_scr();
	set_tty(); 
	set_masks();
	init_vars(1);

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
   		entry_exit = 0;
   		edit_exit = 0;
   		prog_exit = 0;
   		restart = 0;
		line_no = 0;
	
		search_ok = TRUE;
		init_vars(1);	

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display (1);
		edit (1);
		
		if (restart)
			continue;
		run_prog();
		prog_exit = 1;
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*============================
| Program shutdown sequence. |
============================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int	field)
{
	char	valid_inp[2];

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if ( !valid_lp( local_rec.lpno ) )
		{
			/*print_mess("\007 Invalid Printer Number ");*/
			print_mess(ML(mlStdMess020));
			sleep( 2 );
			return(1);
		}
		return(0);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK("back") )
	{
		sprintf(valid_inp, "%1.1s", local_rec.back);

		strcpy(local_rec.back, (valid_inp[0] == 'N') ? "N (NO) "
							     : "Y (YES)");
		return(0);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if ( LCHECK("onight") )
	{
		sprintf(valid_inp, "%1.1s", local_rec.onight);

		strcpy(local_rec.onight, (valid_inp[0] == 'N') ? "N (NO) "
							       : "Y (YES)");
		return(0);
	}

	/*--------------------
	| Validate Sort Type |
	--------------------*/
	if ( LCHECK("sort") )
	{
		if (dflt_used)
			strcpy (local_rec.sort, "C");

		if (!strcmp (local_rec.sort, "F"))
			strcpy (local_rec.sort_desc,"Finish Date");
		if (!strcmp (local_rec.sort, "C"))
			strcpy (local_rec.sort_desc,"Contract   ");
		DSP_FLD("sort_desc");
		return(0);
	}

	/*--------------------------
	| Validate Start Job Type. |
	--------------------------*/
	if ( LCHECK("st_job") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_job, "    ");
			sprintf(local_rec.st_job_desc, "%22.22s%18.18s",
						"From beginning of file",
						" ");
			DSP_FLD("st_job_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchCmjt(temp_str);	
			return(0);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		strcpy (cmjt_rec.job_type, local_rec.st_job);
		cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf(err_str, 
				"Job type %s is not on file",
				 local_rec.st_job);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess088));
			sleep(2);
			clear_mess();
			return(1);
		}

		if (prog_status != ENTRY &&
		    strcmp(local_rec.st_job, local_rec.end_job) > 0)
		{
			/*sprintf (err_str, "Start job type %s must be less than end job type %s\007",
							 local_rec.st_job,
							 local_rec.end_job);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess017));
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.st_job_desc, cmjt_rec.desc);
		DSP_FLD("st_job_desc");
		return(0);
	}

	/*------------------------
	| Validate End Job Type. |
	------------------------*/
	if ( LCHECK("end_job") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_job, "~~~~");
			sprintf(local_rec.end_job_desc, "%14.14s%26.26s",
						"To end of file",
						" ");
			DSP_FLD("end_job_desc");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchCmjt(temp_str);	
			return(0);
		}

		strcpy (cmjt_rec.co_no, comm_rec.co_no);
		strcpy (cmjt_rec.br_no, comm_rec.est_no);
		strcpy (cmjt_rec.job_type, local_rec.end_job);
		cc = find_rec (cmjt, &cmjt_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf(err_str,
				"Job type %s is not on file\007",
				local_rec.end_job);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess088));
			sleep(2);
			clear_mess();
			return(1);
		}

		if (strcmp(local_rec.st_job, local_rec.end_job) > 0)
		{
/*
			sprintf (err_str, "End job type %s must be less than start job type %s\007",
							 local_rec.end_job,
							 local_rec.st_job);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess018));
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.end_job_desc, cmjt_rec.desc);
		DSP_FLD("end_job_desc");
		return(0);
	}

	/*---------------------------------
	| Validate Start Contract Number. |
	---------------------------------*/
	if ( LCHECK("st_con") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_con, "      ");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchCmhr(temp_str);	
			return(0);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.cont_no, pad_num(local_rec.st_con));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			sprintf (err_str, "Contract Number %s is not on file",
						 pad_num(local_rec.st_con));
			print_mess(err_str);*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}

		if (prog_status != ENTRY &&
		    strcmp(local_rec.st_con, local_rec.end_con) > 0)
		{
/*
			sprintf (err_str, "Start contract number %s must be less than end contract number %s\007",
						 pad_num(local_rec.st_con),
							 local_rec.end_con);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess017));
			sleep(2);
			clear_mess();
			return(1);
		}
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, " ");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		if (cc)
		{
			/*sprintf (err_str, "There is no extra description for this contract number\007");
			print_mess(err_str);*/
			print_mess(ML(mlCmMess020));
			sleep(2);
			clear_mess();
			return(0);
		}
		while (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			if (cmcd_rec.line_no == 0)
				strcpy (local_rec.st_con_desc1, cmcd_rec.text);
			if (cmcd_rec.line_no == 1)
				strcpy (local_rec.st_con_desc2, cmcd_rec.text);

			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}
		DSP_FLD("st_con_desc1");
		DSP_FLD("st_con_desc2");
		return(0);
	}

	/*----------------------------
	| Validate Start End Number. |
	----------------------------*/
	if ( LCHECK("end_con") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_con, "~~~~~~");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchCmhr(temp_str);	
			return(0);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.cont_no, pad_num(local_rec.end_con));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, "Contract Number %s is not on file",
						 pad_num(local_rec.end_con));
			print_mess(err_str);*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}

		if (strcmp(local_rec.st_con, local_rec.end_con) > 0)
		{
/*
			sprintf (err_str, "End contract number %s must be less than start contract number %s\007",
						 pad_num(local_rec.end_con),
							 local_rec.st_con);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess013));
			sleep(2);
			clear_mess();
			return(1);
		}
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, " ");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		if (cc)
		{
/*
			sprintf (err_str, "There is no extra description for this contract number\007");
			print_mess(err_str);*/
			print_mess(ML(mlCmMess020));
			sleep(2);
			clear_mess();
			return(0);
		}
		while (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			if (cmcd_rec.line_no == 0)
				strcpy(local_rec.end_con_desc1,cmcd_rec.text);
			if (cmcd_rec.line_no == 1)
				strcpy(local_rec.end_con_desc2,cmcd_rec.text);

			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}
		DSP_FLD("end_con_desc1");
		DSP_FLD("end_con_desc2");
		return(0);
	}

	/*--------------------------
	| Validate Contract Status |
	--------------------------*/
	if ( LCHECK("status") )
	{
		if (dflt_used)
			strcpy (local_rec.status, "A");

		if (!strcmp (local_rec.status, "O"))
			strcpy (local_rec.status_desc,"Open   ");
		if (!strcmp (local_rec.status, "C"))
			strcpy (local_rec.status_desc,"Close  ");
		if (!strcmp (local_rec.status, "H"))
			strcpy (local_rec.status_desc,"History");
		if (!strcmp (local_rec.status, "A"))
			strcpy (local_rec.status_desc,"All    ");
		DSP_FLD("status_desc");
		return(0);
	}
	return(0);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(cmhr);
	abc_fclose(cmhr2);
	abc_fclose(cmcd);
	abc_fclose(cmjt);
	abc_fclose(cumr);
	abc_dbclose(data);
}

void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	abc_alias (cmhr2, cmhr);

	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_id_no2");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_id_no3");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
}

void
run_prog (void)
{
	char	lp_str[3];

	clear();
	print_at(0,0,ML(mlStdMess035));
	/*print_at(0,0,"Busy, ....");*/
	fflush(stdout);
	sprintf(lp_str, "%2d", lpno);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y')
	{ 
		rset_tty();
		CloseDB (); 
		FinishProgram ();

		if (fork() == 0)
		{
			execlp("ONIGHT",
					"ONIGHT",
					"cm_cont_lst",
					local_rec.st_job,
					local_rec.end_job,
					local_rec.st_con,
					local_rec.end_con,
					local_rec.status,
					local_rec.sort,
					lp_str, 
					prog_desc, (char *) 0);
		}
		else
			return;
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back[0] == 'Y') 
	{
		rset_tty();
		CloseDB (); 
		FinishProgram ();

		if (fork() != 0)
			return;
		else
		{
			execlp("cm_cont_lst",
					"cm_cont_lst",
					local_rec.st_job,
					local_rec.end_job,
					local_rec.st_con,
					local_rec.end_con,
					local_rec.status,
					local_rec.sort,
					lp_str, (char *) 0);
		}
	}
	else 
		proc_file();			
}

/*-----------------------------
| The guts of the processing. |
-----------------------------*/
void
proc_file (void)
{

	if (!strcmp (local_rec.sort, "F") || !strcmp (local_rec.sort, "f"))
	{
		strcpy (sorted_by, "Finish Date");
		fsort = sort_open("master");
	}
	else
		strcpy (sorted_by, "Contracts  ");

	dsp_screen("Printing Contract Master Listings.",
				comm_rec.co_no, comm_rec.co_name);
	
	start_report(lpno);

	abc_selfield(cmjt, "cmjt_hhjt_hash");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.cont_no, local_rec.st_con);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no))
	{
		if ( strncmp(cmhr_rec.cont_no, local_rec.end_con, 6) > 0) 
			break;

		cc = find_hash (cmjt, &cmjt_rec, EQUAL, "r", cmhr_rec.hhjt_hash);
		if (cc)
			file_err (cc, cmjt, "DBFIND");
		
		if ( strncmp(cmjt_rec.job_type, local_rec.st_job, 4) < 0 || 
			 strncmp(cmjt_rec.job_type, local_rec.end_job, 4) > 0) 
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		if (local_rec.status[0] == 'O' && cmhr_rec.status[0] != 'O')
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		if (local_rec.status[0] == 'C' && cmhr_rec.status[0] != 'C')
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		if (local_rec.status[0] == 'H' && cmhr_rec.status[0] != 'H')
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		calc_tots(cmhr_rec.hhhr_hash);

		if (!strcmp(local_rec.sort, "F"))
		{

			if (cmhr_rec.mast_hhhr != 0)
			{
				cc = find_hash (cmhr2, &cmhr2_rec, EQUAL, "r", cmhr_rec.mast_hhhr);
				if (cc)
					file_err (cc, cmhr2, "DBFIND");
			}
			else
				strcpy (cmhr2_rec.cont_no, "      ");

			cc = find_hash (cumr, &cumr_rec, EQUAL, "r", cmhr_rec.hhcu_hash);
			if (cc)
				file_err (cc, cumr, "DBFIND");

			fprintf(fsort, "%10ld%6.6s%4.4s%6.6s%6.6s%20.20s%40.40s%-*.*s%-*.*s%-*.*s%-*.*s%-*.*s%s%4.4s%9.2f%9.2f%9.2f%9.2f%9.2f%9.2f%9.2f%9.2f\n",
 					cmhr_rec.due_date,
					cmhr_rec.cont_no,
					cmjt_rec.job_type,
					cmhr2_rec.cont_no,
					cumr_rec.dbt_no,
					cmhr_rec.cus_ref,
					cmhr_rec.contact,
					16,16,cmhr_rec.wip_glacc,
					16,16,cmhr_rec.lab_glacc,
					16,16,cmhr_rec.o_h_glacc,
					16,16,cmhr_rec.sal_glacc,
					16,16,cmhr_rec.cog_glacc,
					DateToString(cmhr_rec.wip_date),
					cmhr_rec.wip_status,
					local_rec.cost_amt,
					local_rec.mtd_cost,
					local_rec.td_cost,
					local_rec.budg_cost,
					local_rec.sales_amt,
					local_rec.mtd_sales,
					local_rec.td_sales,
					local_rec.budg_sales);
		}
	
		if (!strcmp (local_rec.sort, "C"))
			proc_line();
	
		dsp_process("Contract Number : ", cmhr_rec.cont_no);
		cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
	}
	if (!strcmp (local_rec.sort, "F"))
	{
		fsort = sort_sort (fsort, "master");
		sptr = sort_read (fsort);
		while (sptr)
		{
			print_line(sptr);
			sptr = sort_read (fsort);
		}
		sort_delete (fsort, "master");
	}
		
	end_report();
	pclose(pout);
	abc_selfield(cmjt, "cmjt_id_no");
}

void
calc_tots (
 long	hhhr_hash)
{
	double 	sale_price = 0.00;
	float	qty = 0.00;

	local_rec.mtd_sales  = 0.00;
	local_rec.td_sales   = 0.00;
	local_rec.mtd_cost   = 0.00;
	local_rec.td_cost    = 0.00;
	local_rec.budg_sales = 0.00;
	local_rec.budg_cost   = 0.00;
	local_rec.sales_amt  = 0.00;
	local_rec.cost_amt   = 0.00;
	local_rec.cmpb_date  = 0L;

	local_rec.st_mth = MonthStart(local_rec.lsystemDate);
	local_rec.st_yr  = get_ybeg(local_rec.lsystemDate);

	cc = find_hash (cmpb, &cmpb_rec, GTEQ, "r", hhhr_hash);
	while (!cc && cmpb_rec.hhhr_hash == hhhr_hash)
	{
		if (cmpb_rec.date > local_rec.cmpb_date)
			local_rec.cmpb_date = cmpb_rec.date;
		cc = find_hash (cmpb, &cmpb_rec, NEXT, "r", hhhr_hash);
	}

	cmtr_rec.hhhr_hash = hhhr_hash;
	cmtr_rec.hhcm_hash = 0L;
	cmtr_rec.date = local_rec.st_yr;
	cc = find_rec (cmtr, &cmtr_rec, GTEQ, "r");
	while (!cc && cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		sale_price = cmtr_rec.sale_price * (100 - cmtr_rec.disc_pc) / 100;
		if (cmtr_rec.date >= local_rec.st_mth)
		{
			local_rec.mtd_sales  += sale_price * cmtr_rec.qty;
			local_rec.mtd_cost+= cmtr_rec.cost_price * cmtr_rec.qty;
		}
		
		if (cmtr_rec.date > local_rec.cmpb_date)
		{
			local_rec.sales_amt += sale_price * cmtr_rec.qty;
			local_rec.cost_amt+= cmtr_rec.cost_price *cmtr_rec.qty;
		}

		local_rec.td_sales += sale_price * cmtr_rec.qty;
		local_rec.td_cost  += cmtr_rec.cost_price * cmtr_rec.qty;
		cc = find_rec (cmtr, &cmtr_rec, NEXT, "r");
	}

	cmts_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmts_rec.hhcm_hash = 0L;
	cmts_rec.date = local_rec.st_yr;
	cc = find_rec (cmts, &cmts_rec, GTEQ, "r");
	while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		qty = cmts_rec.time_ord + (cmts_rec.time_hlf * (float) 1.5) + 
				(cmts_rec.time_dbl * (float) 2) + cmts_rec.units;
		if (cmts_rec.date >= local_rec.st_mth)
		{
			local_rec.mtd_sales += cmts_rec.sale * qty;
			local_rec.mtd_cost  += (cmts_rec.oh_cost + cmts_rec.lab_cost) * qty;
		}

		if (cmts_rec.date > local_rec.cmpb_date)
		{
			local_rec.sales_amt += cmts_rec.sale * qty;
			local_rec.cost_amt  += (cmts_rec.oh_cost + cmts_rec.lab_cost) * qty;
		}

		local_rec.td_sales += cmts_rec.sale * qty;
		local_rec.td_cost +=  (cmts_rec.oh_cost + cmts_rec.lab_cost) * qty;
		cc = find_rec (cmts, &cmts_rec, NEXT, "r");
	}

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;
	cc = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	while (!cc && cmcb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		local_rec.budg_sales += cmcb_rec.budg_value;
		local_rec.budg_cost  += cmcb_rec.budg_cost;
		cc = find_rec (cmcb, &cmcb_rec, NEXT, "r");
	}
}

void
start_report (
 int	prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	sprintf(err_str,"%s <%s>", DateToString(comm_rec.crd_date),PNAME);
	fprintf(pout,".START%s\n",clip(err_str));
	fprintf(pout,".LP%d\n",prnt_no);

	fprintf(pout,".16\n");
	rpt_heading();
	fprintf(pout,".PI12\n");
}

/*===========================
| Validate and print lines. |
===========================*/
void
proc_line (void)
{
	if (cmhr_rec.mast_hhhr != 0)
	{
		cc = find_hash (cmhr2, &cmhr2_rec, EQUAL, "r", cmhr_rec.mast_hhhr);
		if (cc)
			file_err (cc, cmhr2, "DBFIND");
	}
	else
		strcpy (cmhr2_rec.cont_no, "      ");

	cc = find_hash (cumr, &cumr_rec, EQUAL, "r", cmhr_rec.hhcu_hash);
	if (cc)
		file_err (cc, cumr, "DBFIND");

	chk_page();

	if (line_no != 0)
	{
		fprintf(pout, "|--------");
		fprintf(pout, "-------");
		fprintf(pout, "---------");
		fprintf(pout, "---------");
		fprintf(pout, "---------------------");
		fprintf(pout, "-------------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "-----------");
		fprintf(pout, "-----|\n");
	}

	/*---------------------------
	|  Full Master file listing | 
	---------------------------*/
	fprintf (pout, "| %-6.6s ",	cmhr_rec.cont_no);
	fprintf (pout, "| %-4.4s ",	cmjt_rec.job_type);
	fprintf (pout, "| %-6.6s ",	cmhr2_rec.cont_no);
	fprintf (pout, "| %-6.6s ",	cumr_rec.dbt_no);
	fprintf (pout, "|%-20.20s",	cmhr_rec.cus_ref);
	fprintf (pout, "|%-24.24s",	cmhr_rec.contact);
	fprintf (pout, "|%-16.169s",	cmhr_rec.wip_glacc);
	fprintf (pout, "|%-16.16s",	cmhr_rec.lab_glacc);
	fprintf (pout, "|%-16.16s",	cmhr_rec.o_h_glacc);
	fprintf (pout, "|%-16.16s",	cmhr_rec.sal_glacc);
	fprintf (pout, "|%-16.16s",	cmhr_rec.cog_glacc);
	fprintf (pout, "|%10.10s",	DateToString(cmhr_rec.wip_date));
	fprintf (pout, "|%-4.4s|\n",	cmhr_rec.wip_status);
	
	sub_heading();

	fprintf (pout, "|%30.30sSALES%13.13s", " ", " ");
	fprintf (pout, " %14.2f ",	DOLLARS(local_rec.sales_amt));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %14.2f ",	DOLLARS(local_rec.mtd_sales));
	fprintf (pout, "%15.15s", " ");
	fprintf (pout, " %14.2f ",	DOLLARS(local_rec.td_sales));
	fprintf (pout, "%13.13s", " ");
	fprintf (pout, " %14.2f",	DOLLARS(local_rec.budg_sales));
	fprintf (pout, "%30.30s|\n", " ");
	fprintf (pout, "|%30.30sCOSTS%13.13s", " ", " ");
	fprintf (pout, " %14.2f ",	DOLLARS(local_rec.cost_amt));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %14.2f ",	DOLLARS(local_rec.mtd_cost));
	fprintf (pout, "%15.15s", " ");
	fprintf (pout, " %14.2f",	DOLLARS(local_rec.td_cost));
	fprintf (pout, "%14.14s", " ");
	fprintf (pout, " %14.2f",	DOLLARS(local_rec.budg_cost));
	fprintf (pout, "%30.30s|\n", " ");

	line_no +=6;
}

void
print_line (
 char *	tptr)
{
	chk_page();

	if (line_no != 0)
	{
		fprintf(pout, "|--------");
		fprintf(pout, "-------");
		fprintf(pout, "---------");
		fprintf(pout, "---------");
		fprintf(pout, "---------------------");
		fprintf(pout, "-------------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "------------------");
		fprintf(pout, "-----------");
		fprintf(pout, "-----|\n");

		line_no ++;
	}

	fprintf (pout, "| %-6.6s ",	tptr + 10);
	fprintf (pout, "| %-4.4s ",	tptr + 16);
	fprintf (pout, "| %-6.6s ",	tptr + 20);
	fprintf (pout, "| %-6.6s ",	tptr + 26);
	fprintf (pout, "|%-20.20s",	tptr + 32);
	fprintf (pout, "|%-24.24s",	tptr + 52);
	fprintf (pout, "|%-16.16s",	tptr + 92);
	fprintf (pout, "|%-16.16s",	tptr +  92 + 16);
	fprintf (pout, "|%-16.16s",	tptr + 101 + 16);
	fprintf (pout, "|%-16.16s",	tptr + 110 + 16);
	fprintf (pout, "|%-16.16s",	tptr + 119 + 16);
	fprintf (pout, "|%-10.10s", tptr + 128 + 16);
	fprintf (pout, "|%-4.4s|\n",tptr + 136 + 16);
	
	sub_heading();

	fprintf (pout, "|%30.30sSALES%13.13s", " ", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (140 + 16))));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (149 + 16))));
	fprintf (pout, "%15.15s", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (158 + 16))));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %9.2f",	DOLLARS(atof(tptr + (167 + 16))));
	fprintf (pout, "%24.24s|\n", " ");
	fprintf (pout, "|%30.30sCOSTS%13.13s", " ", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (173 + 16))));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (185 + 16))));
	fprintf (pout, "%15.15s", " ");
	fprintf (pout, " %9.2f ",	DOLLARS(atof(tptr + (194 + 16))));
	fprintf (pout, "%12.12s", " ");
	fprintf (pout, " %9.2f",	DOLLARS(atof(tptr + (203 + 16))));
	fprintf (pout, "%24.24s|\n", " ");

	line_no +=6;
}

void
sub_heading (void)
{
	fprintf (pout, "|%155.155s|\n", " ");
	fprintf (pout, "|%40.40s", " ");
	fprintf (pout, "SINCE LAST INVOICE");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "MONTH TO DATE");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "CONTRACT TO DATE");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "BUDGETED VALUE");
	fprintf (pout, "%30.30s|\n", " ");

	fprintf (pout, "|%40.40s", " ");
	fprintf (pout, "------------------");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "-------------");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "----------------");
	fprintf (pout, "%16.16s", " ");
	fprintf (pout, "--------------");
	fprintf (pout, "%30.30s|\n", " ");
}

void
chk_page(void)
{
	if ((line_no + 6) > SEL_PSIZE)
	{
		fprintf (pout, ".PA\n");
		line_no = 0;
	}
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (void)
{
	fprintf(pout,".EOF\n");
}

/*============================
| Heading for report.        |
=============================*/
void
rpt_heading (void)
{
	fprintf(pout,".L158\n");
	fprintf(pout,".E%s\n","CONTRACT MASTER FILE LISTING");
	fprintf(pout,".B1\n.EAS AT : %-24.24s\n", SystemTime());
	fprintf(pout,".B1\n");
	fprintf(pout,".E%s\n", clip(comm_rec.co_name));
	fprintf(pout,".B1\n");
	fprintf(pout,".CSorted by %s   From Job %s to %s\n", sorted_by,
				local_rec.st_job, local_rec.end_job);
	fprintf(pout,".CContract Status %s   From Contract %s to %s\n",
				local_rec.status_desc,
				local_rec.st_con, local_rec.end_con);
	fprintf(pout,".CAll Values Exclusive of Tax\n");
	
	fprintf(pout, ".R=======================================");
	fprintf(pout, "===================================================");
	fprintf(pout, "===================================================");
	fprintf(pout, "==============================================\n");

	fprintf(pout, "=======================================");
	fprintf(pout, "===================================================");
	fprintf(pout, "===================================================");
	fprintf(pout, "==============================================\n");

	fprintf(pout, "|CONTRACT");
	fprintf(pout, "|  JOB ");
	fprintf(pout, "| MASTER ");
	fprintf(pout, "| CUST.  ");
	fprintf(pout, "|      CUSTOMER      ");
	fprintf(pout, "|        CONTACT         ");
	fprintf(pout, "|       WIP      ");
	fprintf(pout, "|       LAB      ");
	fprintf(pout, "|       O/H      ");
	fprintf(pout, "|       SAL      ");
	fprintf(pout, "|       COG      ");
	fprintf(pout, "|   WIP    ");
	fprintf(pout, "|WIP |\n");

	fprintf(pout, "| NUMBER ");
	fprintf(pout, "| TYPE ");
	fprintf(pout, "| ACCT # ");
	fprintf(pout, "| NUMBER ");
	fprintf(pout, "|      REFERENCE     ");
	fprintf(pout, "|          NAME          ");
	fprintf(pout, "|     ACCOUNT     ");
	fprintf(pout, "|     ACCOUNT     ");
	fprintf(pout, "|     ACCOUNT     ");
	fprintf(pout, "|     ACCOUNT     ");
	fprintf(pout, "|     ACCOUNT     ");
	fprintf(pout, "|   DATE   ");
	fprintf(pout, "|STAT|\n");

	fprintf(pout, "|--------");
	fprintf(pout, "|------");
	fprintf(pout, "|--------");
	fprintf(pout, "|--------");
	fprintf(pout, "|--------------------");
	fprintf(pout, "|------------------------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|-----------------");
	fprintf(pout, "|----------");
	fprintf(pout, "|----|\n");

	fflush(pout);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		swide();

		rv_pr(ML(mlCmMess154), (132 - strlen(ML(mlCmMess154))) / 2, 0, 1);

		box (0, 2, 130, 17);
		move(1,4);
		line(129);

		move(1,7);
		line(129);

		move(1,14);
		line(129);

		move(1,16);
		line(129);

		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.co_no, clip(comm_rec.co_name));
		strcpy(err_str,ML(mlStdMess039));
        print_at(21,40,err_str,comm_rec.est_no, clip(comm_rec.est_name));

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;

		scn_write(scn);
/*
		print_at(21,0," Co no : %s - %s  Br no : %s - %s ",comm_rec.co_no, clip(comm_rec.co_name), comm_rec.est_no, clip(comm_rec.est_name));*/
	}
	return (EXIT_SUCCESS);
}

int
SrchCmjt (
 char *	key_val)
{
	work_open();
	save_rec("#Type","#Description");

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	strcpy (cmjt_rec.job_type, "    ");
	strncpy (cmjt_rec.job_type, key_val, strlen(key_val));
	cc = find_rec (cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp(cmjt_rec.co_no, comm_rec.co_no) &&
	       !strcmp(cmjt_rec.br_no, comm_rec.est_no) &&
	       !strncmp(cmjt_rec.job_type, key_val, strlen(key_val)))
	{
		cc = save_rec(cmjt_rec.job_type, cmjt_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmjt, &cmjt_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return(1);

	strcpy (cmjt_rec.co_no, comm_rec.co_no);
	strcpy (cmjt_rec.br_no, comm_rec.est_no);
	sprintf (cmjt_rec.job_type, "%-4.4s", temp_str);
	cc = find_rec (cmjt, &cmjt_rec, COMPARISON, "r" );
	if (cc)
	 	file_err( cc, cmjt, "DBFIND" );
	return(0);
}

int
SrchCmhr (
 char *	key_val)
{
	work_open();
	save_rec("#Contract No","#Customer Ref");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.cont_no, "      ");
	strncpy (cmhr_rec.cont_no, key_val, strlen(key_val));
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp(cmhr_rec.co_no, comm_rec.co_no) &&
	       !strncmp(cmhr_rec.cont_no, key_val, strlen(key_val)))
	{
		cc = save_rec(cmhr_rec.cont_no, cmhr_rec.cus_ref);
		if (cc)
			break;

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return(1);
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	sprintf (cmhr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r" );
	if (cc)
	 	file_err( cc, cmhr, "DBFIND" );
	return(0);
}
