/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_anniprt.c )                                   |
|  Program Desc  : ( External Open Contracts Due on the           )   |
|                  ( Anniversary Day Display/Print                )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmhr, cmpb, cmts, cmtr, cmjt, cmit,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : 18/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (29/04/93)      | Modified  by : Jonc              |
|  Date Modified : (11/06/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Marnie I Organo.  |
|  Date Modified : (15/10/97)      | Modified  by : Marnie I Organo.  |
|  Date Modified : (02/09/1999)    | Modified  by : Alvin Misalucha.  |
|                                                                     |
|  Comments      :                                                    |
|    (29/04/93)  : EGC 8917 Make sure ONIGHT working properly         |
|    (11/06/93)  : EGC 9078 Getting the thing to work.                |
|    (15/11/95)  : PDL - Updated for version 9.                       |
|    (03/09/97)  : Modified for Multilingual Conversion               |
|    (15/10/97)  : Added ML().                                        |
|    (02/09/1999): Ported to ANSI format.                             |
|                                                                     |
| $Log: cm_anniprt.c,v $
| Revision 5.3  2002/07/17 09:56:58  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:07  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:58  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:19  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.16  1999/12/06 01:32:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.15  1999/11/29 21:41:39  cam
| Changes for GVision compatibility
|
| Revision 1.14  1999/11/17 06:39:04  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/08 04:35:36  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.12  1999/10/20 01:40:29  nz
| Updated for remainder of date routines
|
| Revision 1.11  1999/10/01 07:48:17  scott
| Updated for standard function calls.
|
| Revision 1.10  1999/09/29 10:10:12  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/29 01:51:21  scott
| Updated for getchar and ansi stuff
|
| Revision 1.8  1999/09/17 04:40:06  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.7  1999/09/16 04:44:40  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/14 07:33:58  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_anniprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_anniprt/cm_anniprt.c,v 5.3 2002/07/17 09:56:58 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include	<string.h>
#include	<memory.h>
#include	<malloc.h>
#include	<ml_std_mess.h>
#include	<ml_cm_mess.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"}
	};
	int comm_no_fields = 6;
	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		long	tcrd_date;
	} comm_rec;
		
	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	14

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_hhit_hash"},
		{"cmhr_st_date"},
		{"cmhr_due_date"},
		{"cmhr_hhjt_hash"},
		{"cmhr_wip_status"},
		{"cmhr_quote_type"},
		{"cmhr_progress"},
		{"cmhr_anni_day"},
		{"cmhr_quote_val"},
		{"cmhr_status"},
	};

	struct tag_cmhrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	cont_no [7];
		long	hhhr_hash;
		long	hhit_hash;
		long	st_date;
		long	due_date;
		long	hhjt_hash;
		char	wip_status [5];
		char	quote_type [2];
		char	progress [2];
		char	anni_day [3];
		double	quote_val;		/* money */
		char	status [2];
	}	cmhr_rec;

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

	/*==========================================+
	 | Contract Management Issue To Master File |
	 +==========================================*/
#define	CMIT_NO_FIELDS	4

	struct dbview	cmit_list [CMIT_NO_FIELDS] =
	{
		{"cmit_co_no"},
		{"cmit_issto"},
		{"cmit_hhit_hash"},
		{"cmit_iss_name"},
	};

	struct tag_cmitRecord
	{
		char	co_no [3];
		char	issto [11];
		long	hhit_hash;
		char	iss_name [41];
	}	cmit_rec;

	/*===========================================+
	 | Contract Management Progress Billing File |
	 +===========================================*/
#define	CMPB_NO_FIELDS	6

	struct dbview	cmpb_list [CMPB_NO_FIELDS] =
	{
		{"cmpb_hhhr_hash"},
		{"cmpb_date"},
		{"cmpb_inv_date"},
		{"cmpb_amount"},
		{"cmpb_amt_rem"},
		{"cmpb_hhco_hash"}
	};

	struct tag_cmpbRecord
	{
		long	hhhr_hash;
		long	date;
		long	inv_date;
		double	amount;		/* money */
		double	amt_rem;		/* money */
		long	hhco_hash;
	}	cmpb_rec;

	/*===========================================+
	 | Contract Management Time Sheet Trans File |
	 +===========================================*/
#define	CMTS_NO_FIELDS	13

	struct dbview	cmts_list [CMTS_NO_FIELDS] =
	{
		{"cmts_hhem_hash"},
		{"cmts_date"},
		{"cmts_hhhr_hash"},
		{"cmts_hhcm_hash"},
		{"cmts_hheq_hash"},
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
		long	hhem_hash;
		long	date;
		long	hhhr_hash;
		long	hhcm_hash;
		long	hheq_hash;
		float	time_ord;
		float	time_hlf;
		float	time_dbl;
		float	units;
		double	lab_cost;		/* money */
		double	oh_cost;		/* money */
		double	sale;		/* money */
		char	stat_flag [2];
	}	cmts_rec;

	/*=======================================+
	 | Contract Management Transaction File. |
	 +=======================================*/
#define	CMTR_NO_FIELDS	13

	struct dbview	cmtr_list [CMTR_NO_FIELDS] =
	{
		{"cmtr_hhhr_hash"},
		{"cmtr_hhcm_hash"},
		{"cmtr_hhbr_hash"},
		{"cmtr_qty"},
		{"cmtr_cost_price"},
		{"cmtr_sale_price"},
		{"cmtr_disc_pc"},
		{"cmtr_gst_pc"},
		{"cmtr_tax_pc"},
		{"cmtr_ser_no"},
		{"cmtr_date"},
		{"cmtr_time"},
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
		float	gst_pc;
		float	tax_pc;
		char	ser_no [26];
		long	date;
		char	time [9];
		char	stat_flag [2];
	}	cmtr_rec;


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


	/*==========================================+
	 | Contract Management Employee Master File |
	 +==========================================*/
#define	CMEM_NO_FIELDS	2

	struct dbview	cmem_list [CMEM_NO_FIELDS] =
	{
		{"cmem_emp_name"},
		{"cmem_hhem_hash"},
	};

	struct tag_cmemRecord
	{
		char	emp_name [41];
		long	hhem_hash;
	}	cmem_rec;

	/*===========================================+
	 | Contract Management Equipment Master File |
	 +===========================================*/
#define	CMEQ_NO_FIELDS	2

	struct dbview	cmeq_list [CMEQ_NO_FIELDS] =
	{
		{"cmeq_hheq_hash"},
		{"cmeq_desc"},
	};

	struct tag_cmeqRecord
	{
		long	hheq_hash;
		char	desc [41];
	}	cmeq_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview	inmr_list [] =
	{
		{"inmr_hhbr_hash"},
		{"inmr_description"},
	};

	int	inmr_no_fields = 2;

	struct tag_inmrRecord
	{
		long	hhbr_hash;
		char	description [41];
	}	inmr_rec;

/*=======================
 Globals & Magic numbers
=========================*/

	static int	bgmode = FALSE;
	static char	branchNo [3];

/**	Some of the valid status codes for cmhr_status **/
#define	STS_OPEN	'O'
#define	STS_BILLING	'B'

/**	Valid select by codes **/
#define	BY_CONTRACT	'C'
#define	BY_JOBTYPE	'J'
#define	BY_ISSUETO	'I'

/**	Valid transaction list codes **/
#define	TR_DETAILS	'D'
#define	TR_SUMMARY	'S'

/**	CM_AUTO_CON values **/
#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	LINELEN	132

	static struct
	{
		double	inv_to_date,		/* accumulator for invoices */
				cst_to_date;		/* accumulator for costs */

		long	last_inv_date;
		double	last_inv_amt;

		double	cost_amt,		/* costs since last invoice */
				sale_amt;

	}	acc_info;

/*===========
 Table names
============*/
static char *data	= "data",
			*cmcd	= "cmcd",
			*cmem	= "cmem",
			*cmeq	= "cmeq",
			*cmhr	= "cmhr",
			*cmit	= "cmit",
			*cmit2	= "cmit2",
			*cmjt	= "cmjt",
			*cmjt2	= "cmjt2",
			*cmpb	= "cmpb",
			*cmtr	= "cmtr",
			*cmts	= "cmts",
			*inmr	= "inmr";

/*============================
| Local & Screen Structures. |
============================*/
#define	CODE_WIDTH	10
#define	DESC_WIDTH	40
struct
{
	char	dummy[11];

	char	by_code [20];

	char	beg_code		[CODE_WIDTH + 1];
	char	beg_code_desc	[DESC_WIDTH + 1];
	char	end_code		[CODE_WIDTH + 1];
	char	end_code_desc	[DESC_WIDTH + 1];

	int		beg_ann,
			end_ann;
	char	beg_ann_str [3],
			end_ann_str [3];

	char	details [20];

	int		lpno;
	char	back	[8];
	char	atnight	[8];

}	local_rec;

static struct var	vars [] = 
{
	{1, LIN, "by_code", 3, 23, CHARTYPE, 
		"U", "          ", 
		" ", "J", " Select by             :", "Choose : J)obtype, C)ontract, I)ssueTo", 
		NE, NO, JUSTLEFT, "IJC", "", local_rec.by_code}, 
	{1, LIN, "beg_code", 5, 23, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "", "Enter the start code", 
		YES, NO, JUSTLEFT, "", "", local_rec.beg_code}, 
	{1, LIN, "beg_desc", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.beg_code_desc}, 
	{1, LIN, "end_code", 6, 23, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "~", "", "Enter the ending code", 
		YES, NO, JUSTLEFT, "", "", local_rec.end_code}, 
	{1, LIN, "end_desc", 6, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_code_desc}, 
	{1, LIN, "beg_ann", 8, 23, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Start Anniversary Day :", "Enter the starting Anniversary Day", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.beg_ann}, 
	{1, LIN, "end_ann", 9, 23, INTTYPE, 
		"NN", "          ", 
		" ", "31", " End Anniversary Day   :", "Enter the ending Anniversary Day", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.end_ann}, 
	{1, LIN, "details", 11, 23, CHARTYPE, 
		"U", "          ", 
		" ", "S", " Print Details/Summary :", "D)etails or S)ummary", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.details}, 
	{1, LIN, "lpno", 13, 23, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No            :", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno}, 
	{1, LIN, "back", 14, 23, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Background            :", "Enter Y)es or N)o", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "atnight", 15, 23, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight             :", "Enter Y)es or N)o ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.atnight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

/*=====================
 Function declarations
======================*/
int		main			(int argc, char * argv []);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
int		run_prog		(char * cmd);
void	proc_file		(void);
void	beg_report		(FILE * pout, int prnt_no);
void	print_line		(FILE * pout);
void	end_report		(FILE * pout);
int		heading			(int scn);
int		FindFirst		(char byCode);
int		FindNext		(char byCode);
int		WithinBounds	(char byCode);
int		PassFilter		(char byCode);
void	Search			(char * key_val);
void	SrchCmhr		(char * key_val);
void	SrchCmjt		(char * key_val);
void	SrchCmit		(char * key_val);
void	GetContractInfo (void);
void	ListDetails		(FILE * pout);
int		GetDescription	(char * key, char * dst);
int		cmhrDescription (char * key, char * dst);
int		cmjtDescription (char * key, char * dst);
int		cmitDescription (char * key, char * dst);
void	cmemDesc		(long hhem_hash, char * dst);
void	cmeqDesc		(long hheq_hash, char * dst);
void	inmrDesc		(long hhbr_hash, char * dst);
int		IsOpenContract	(char * status);
void	DetailHdg		(FILE * pout);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	char	*auto_con = chk_env ("CM_AUTO_CON");

	if (argc == 8)
	{
		/* Background mode */
		memset (&local_rec, 0, sizeof (local_rec));
		strcpy (local_rec.by_code, argv [1]);
		strcpy (local_rec.beg_code, argv [2]);
		strcpy (local_rec.end_code, argv [3]);
		sprintf (local_rec.beg_ann_str, "%2.2s", argv [4]);
		sprintf (local_rec.end_ann_str, "%2.2s", argv [5]);
		strcpy (local_rec.details, argv [6]);
		local_rec.lpno = atoi (argv [7]);

		bgmode = TRUE;

	} else if (argc != 1)
	{
/*
		printf ("Usage : %s {%c%c%c} beg_code end_code beg_anni end_anni {%c%c} lpno\n",
			argv [0],
			BY_CONTRACT, BY_JOBTYPE, BY_ISSUETO,
			TR_DETAILS, TR_SUMMARY);
		print_at (2,0,"\t%c%c%c - by C)ontract, J)obtype, I)ssue\n",
			BY_CONTRACT, BY_JOBTYPE, BY_ISSUETO);
		print_at (3,0,"\t%c%c  - D)etail, S)ummarised transactions\n",
			TR_DETAILS, TR_SUMMARY);
*/

		print_at (0,0,ML(mlCmMess729),argv[0]);
		print_at (1,0,ML(mlCmMess739));
		print_at (2,0,ML(mlCmMess730));
		return (EXIT_FAILURE);
	}

	if (!bgmode)
	{
		SETUP_SCR (vars);

		init_scr ();
		set_tty ();
		set_masks ();
	}

	/*====================================
	| Open db and read in terminal record
	======================================*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*=========
	 Check env
	===========*/
	strcpy (branchNo,
		(!auto_con || atoi (auto_con) == COMPANY) ? " 0" : comm_rec.test_no);

	if (bgmode)
	{
		GetDescription (local_rec.beg_code, local_rec.beg_code_desc);
		GetDescription (local_rec.end_code, local_rec.end_code_desc);
		proc_file ();
	}
	else
		do	{
			int	bc = label ("beg_code"),
				ec = label ("end_code");

			/*=====================
			| Reset control flags |
			=====================*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = TRUE;
			init_vars (1);

			/* Disable range prompts
			*/
			vars [bc].required = vars [ec].required = ND;
			vars [bc].prmpt = vars [ec].prmpt = "";

			/* Flush local_rec
			*/
			memset (&local_rec, 0, sizeof (local_rec));

			heading (1);
			entry (1);

			if (prog_exit || restart)
				continue;

			heading (1);
			scn_display (1);
			edit (1);

			if (restart)
				continue;

			if (!run_prog (argv [0]))
				break;

		}	while (!prog_exit);

	/*========================
	| Program exit sequence. |
	========================*/
	CloseDB (); 
	if (!bgmode)
		FinishProgram ();
		
	return (EXIT_SUCCESS);
}

/*==============================
 Open and closes of db and files
================================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cmjt2, cmjt);
	abc_alias (cmit2, cmit);

	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmem, cmem_list, CMEM_NO_FIELDS, "cmem_hhem_hash");
	open_rec (cmeq, cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmit2, cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmjt, cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmpb, cmpb_list, CMPB_NO_FIELDS, "cmpb_hhhr_hash");
	open_rec (cmts, cmts_list, CMTS_NO_FIELDS, "cmts_hhhr_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
}

void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmhr);
	abc_fclose (cmit);
	abc_fclose (cmit2);
	abc_fclose (cmjt);
	abc_fclose (cmjt2);
	abc_fclose (cmpb);
	abc_fclose (cmts);
	abc_fclose (cmtr);
	abc_fclose (inmr);
	abc_dbclose (data);
}

/** Trap lib routine
**/
int
spec_valid (
 int	field)
{
	if (LCHECK ("by_code"))
	{
		int	bc = label ("beg_code"),
			ec = label ("end_code");

		switch (*local_rec.by_code)
		{
		case BY_CONTRACT	:
			strcpy (local_rec.by_code, "Contract");
			vars [bc].prmpt = " Start Contract Code   :";
			vars [ec].prmpt = " End Contract Code     :";
			vars [bc].mask = vars [ec].mask = "UUUUUU";
			break;

		case BY_JOBTYPE	:
			strcpy (local_rec.by_code, "Job Type");
			vars [bc].prmpt = " Start JobType         :";
			vars [ec].prmpt = " End JobType           :";
			vars [bc].mask = vars [ec].mask = "UUUU";
			break;

		case BY_ISSUETO	:
			strcpy (local_rec.by_code, "Issue To");
			vars [bc].prmpt = " Start IssueTo Code    :";
			vars [ec].prmpt = " End IssueTo Code      :";
			vars [bc].mask = vars [ec].mask = "UUUUUUUUUU";
			break;
	/*	default	:
			sys_err ("Internal error", -1, PNAME);
			return (EXIT_FAILURE);*/
		}
		vars [bc].required = vars [ec].required = YES;

		*local_rec.beg_code_desc = *local_rec.end_code_desc = '\0';
		scn_write (1);

	}
	else if (LCHECK ("beg_ann"))
	{
		if (local_rec.beg_ann < 0 || local_rec.beg_ann > 31)
		{
			/*Values must be between 0 and 31*/
			print_mess (ML(mlCmMess063));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.end_ann && local_rec.beg_ann > local_rec.end_ann)
		{
			/*Start day must be less or equal to End day*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.beg_ann_str, "%2d", local_rec.beg_ann);
	}
	else if (LCHECK ("end_ann"))
	{
		if (local_rec.end_ann < 0 || local_rec.end_ann > 31)
		{
			/*Values must be between 0 and 30*/
			print_mess (ML(mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.end_ann < local_rec.beg_ann)
		{
			/*End day must be greater or equal to Start day*/
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.end_ann_str, "%2d", local_rec.end_ann);
	}
	else if (LCHECK ("details"))
	{
		strcpy (local_rec.details, *local_rec.details == 'D' ? "Details" : "Summary");
	}
	else if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if ( !valid_lp (local_rec.lpno))
		{
			/*Invalid Printer Number*/
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	else if (LCHECK ("back"))
	{
		/*---------------------------------------------
		| Validate Field Selection background option. |
		---------------------------------------------*/
		strcpy (local_rec.back,
			*local_rec.back == 'N' ? "N (No) " : "Y (Yes)");
	}

	else if (LCHECK ("atnight"))
	{
		/*--------------------------------------------
		| Validate Field Selection overnight option. |
		--------------------------------------------*/
		strcpy (local_rec.atnight,
			*local_rec.atnight == 'N' ? "N (No) " : "Y (Yes)");
	}

	else if (LCHECK ("beg_code"))
	{
		/*---------------------
		 Validate Start Code.
		--------------------------*/
		if (dflt_used)
		{
			sprintf (local_rec.beg_code , "%-10.10s", " ");
			strcpy (local_rec.beg_code_desc, "From beginning of file");
			DSP_FLD ("beg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			Search (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
		    strcmp (local_rec.beg_code, local_rec.end_code) > 0)
		{
			/*Start Code %s must be less than End Code %s*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!GetDescription (local_rec.beg_code, local_rec.beg_code_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("beg_desc");
	}
	else if (LCHECK ("end_code"))
	{
		/*----------------
		 Validate End Code
		-------------------*/
		if (dflt_used)
		{
			switch (*local_rec.by_code)
			{
			case BY_CONTRACT	:
				memset (local_rec.end_code, '~', 6);
				local_rec.end_code [6] = '\0';
				break;

			case BY_JOBTYPE	:
				memset (local_rec.end_code, '~', 4);
				local_rec.end_code [4] = '\0';
				break;

			case BY_ISSUETO	:
				memset (local_rec.end_code, '~', 10);
				local_rec.end_code [10] = '\0';
				break;
			}
			strcpy (local_rec.end_code_desc, "To end of file");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			Search (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.beg_code, local_rec.end_code) > 0)
		{
		    /*End Code %s must be less than Start Code %s*/
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!GetDescription (local_rec.end_code, local_rec.end_code_desc))
			return (EXIT_FAILURE);

		DSP_FLD ("end_desc");
	}
	return (EXIT_SUCCESS);
}

int
run_prog (
 char *	cmd)
{
	char	lpstr [10];

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.atnight [0] == 'Y')
	{
		/* Spawn new process to schedule job sometime later
		*/
		switch (fork ())
		{
		case -1	:
			sys_err ("!fork()", errno, PNAME);

		case 0	:
			/* Child overlays self to run overnight scheduler
			*/
			sprintf (lpstr, "%d", local_rec.lpno);
			if (execlp ("ONIGHT",
				"ONIGHT",
				cmd,
				local_rec.by_code,
				local_rec.beg_code,
				local_rec.end_code,
				local_rec.beg_ann_str,
				local_rec.end_ann_str,
				local_rec.details,
				lpstr,
				ML(mlCmMess064),
				NULL) < 0)
					sys_err ("!execlp()", errno, PNAME);

			return (FALSE);		/* Shutdown afterwards */

		default	:
			/* Parent returns to finish
			*/
			return (FALSE);
		}
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		/*	Spawn new process to run it in background
		*/
		switch (fork ())
		{
		case -1	:
			sys_err ("!fork()", errno, PNAME);

		case 0	:
		/*	Child overlays self to rerun
			Can't do a straight continue 'cos shadow
			processes aren't replicated
		*/
			sprintf (lpstr, "%d", local_rec.lpno);
			if (execlp (cmd,
				cmd,
				local_rec.by_code,
				local_rec.beg_code,
				local_rec.end_code,
				local_rec.beg_ann_str,
				local_rec.end_ann_str,
				local_rec.details,
				lpstr,
				NULL) < 0)
					sys_err ("!execlp()", errno, PNAME);

			return (FALSE);			/* Shutdown afterwards */

		default	:
			/* Parent goes back to finish
			*/
			return (FALSE);
		}
	}
	else
		proc_file ();

	return (TRUE);
}

/*-----------------------------
| The guts of the processing. |
-----------------------------*/
void
proc_file (void)
{
	if (!bgmode)
		dsp_screen ("Printing Contracts Due on the Anniversary Day",
			comm_rec.tco_no, comm_rec.tco_name);

	/** Set up match conditions
	clip (local_rec.beg_code);
	clip (local_rec.end_code);
	**/

	if (FindFirst (*local_rec.by_code))
	{
		FILE	*pout = popen ("pformat", "w");	/* printer output */

		if (!pout)
			sys_err ("Error in pformat During (POPEN)", 1, PNAME);

		beg_report (pout, local_rec.lpno);

		do	{

			if (!PassFilter (*local_rec.by_code))
				continue;

			/* Get Contract description
			*/
			memset (&cmcd_rec, 0, sizeof (cmcd_rec));	/* flush */
			dsp_process ("Contract No :", cmhr_rec.cont_no);
			cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
			strcpy (cmcd_rec.stat_flag, "D");
			cmcd_rec.line_no = 0;

			if (find_rec (cmcd, (char *) &cmcd_rec, EQUAL, "r"))
				strcpy (cmcd_rec.text, "**ERROR : NO DESCRIPTION**");

			GetContractInfo ();	/* Get accumulator info */

			print_line (pout);

			if (*local_rec.details == 'D')
				ListDetails (pout);

		}	while (FindNext (*local_rec.by_code));

		end_report (pout);
		pclose (pout);
	}
}

void
beg_report (
 FILE *	pout,
 int	prnt_no)
{

	char	*ruler = (char *) malloc (LINELEN + 1U);

	memset (ruler, '=', LINELEN);
	ruler [LINELEN] = '\0';

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pout, ".START%s <%s>\n", DateToString (comm_rec.tcrd_date), PNAME);
	fprintf (pout, ".LP%d\n", prnt_no);

	fprintf (pout, ".16\n");	/* next 15 lines descs heading */
	fprintf (pout, ".PI10\n");
	fprintf (pout, ".L%d\n", LINELEN);
	fprintf (pout, ".E%s\n", "ANNIVERSARY DAY REPORT");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EAS AT : %s", SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ECompany %s : %s\n", comm_rec.tco_no, clip (comm_rec.tco_name));
	fprintf (pout, ".B1\n");

	switch (local_rec.by_code[0])
	{
	case BY_CONTRACT	:
		fprintf (pout, ".E Selected By Contract Number\n");
		fprintf (pout, ".E From : '%-6.6s' - %s\n",
			local_rec.beg_code, local_rec.beg_code_desc);
		fprintf (pout, ".E To : '%-6.6s' - %s\n",
			local_rec.end_code, local_rec.end_code_desc);
		break;
	case BY_JOBTYPE	:
		fprintf (pout, ".E Selected By Job Type Code\n");
		fprintf (pout, ".E From : '%-4.4s' - %s\n",
			local_rec.beg_code, local_rec.beg_code_desc);
		fprintf (pout, ".E To : '%-4.4s' - %s\n",
			local_rec.end_code, local_rec.end_code_desc);
		break;
	case BY_ISSUETO	:
		fprintf (pout, ".E Selected By Issue To Code\n");
		fprintf (pout, ".E From : '%-10.10s' - %s\n",
			local_rec.beg_code, local_rec.beg_code_desc);
		fprintf (pout, ".E To : '%-10.10s' - %s\n",
			local_rec.end_code, local_rec.end_code_desc);
		break;
	default	:
		sys_err ("Internal error on selection code", -1, PNAME);
	}


        fprintf (pout, ".R%s\n", ruler);
        fprintf (pout, "%s\n", ruler);

        fprintf (pout, "|  C O N T R A C T                       |    Last Invoiced   |     I N V O I C E D |     U N P O S T E D |    Quote          Date |\n");
        fprintf (pout, "| No.    Description                     |   Date      Amount |     Sales     Costs |     Sales     Costs | Type   Value      Due  |\n");
        fprintf (pout, "|----------------------------------------+--------------------+---------------------+---------------------+------------------------|\n");

	free (ruler);
}

/*===========================
| Validate and print lines. |
===========================*/
void
print_line (
 FILE *	pout)
{
	char	*null = "";

       	fprintf (pout, "| %s %31.31s ", cmhr_rec.cont_no, cmcd_rec.text);

	if (acc_info.last_inv_date)
		fprintf (pout, "| %s %9.2f ",
			DateToString (acc_info.last_inv_date),
			DOLLARS(acc_info.last_inv_amt));
	else
		fprintf (pout, "| %18s ", null);

	fprintf (pout, "| %9.2f %9.2f | %9.2f %9.2f | %s  %9.2f  %s |\n",
		DOLLARS (acc_info.inv_to_date),
		DOLLARS (acc_info.cst_to_date),
		DOLLARS (acc_info.sale_amt),
		DOLLARS (acc_info.cost_amt),
		cmhr_rec.quote_type,
		DOLLARS (cmhr_rec.quote_val),
		DateToString (cmhr_rec.due_date));
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (
 FILE *	pout)
{
	fprintf (pout, ".EOF\n");
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
			scn_set (scn);
		clear ();


		rv_pr (ML(mlCmMess064), (80 - strlen (ML(mlCmMess064))) / 2, 0, 1);

		box (0, 2, 80, 13);

		move (1, 4);
		line (79);
		move (1, 7);
		line (79);
		move (1, 10);
		line (79);
		move (1, 12);
		line (79);
		move (0, 21);
		line (80);

		print_at (22, 0, ML(mlStdMess038),
			comm_rec.tco_no,
			clip (comm_rec.tco_name));
		print_at (22, 45, ML(mlStdMess039),
			comm_rec.test_no,
			clip (comm_rec.test_name));

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/**	Record extractions for printouts
**/
int
FindFirst (
 char	byCode)
{
	memset (&cmhr_rec, 0, sizeof (cmhr_rec));	/* flush out */
	strcpy (cmhr_rec.co_no, comm_rec.tco_no);
	strcpy (cmhr_rec.br_no, branchNo);

	switch (byCode)
	{
	case BY_CONTRACT	:
		strcpy (cmhr_rec.cont_no, local_rec.beg_code);
		break;

	case BY_JOBTYPE	:
	case BY_ISSUETO	:
		break;

	default	:
		sys_err ("Internal error on selection code", -1, PNAME);
		return (FALSE);
	}

	if (find_rec (cmhr, (char *) &cmhr_rec, GTEQ, "r"))
		return (FALSE);

	while (!PassFilter (byCode))
		if (!FindNext (byCode))
			return (FALSE);

	return (WithinBounds (byCode));
}

int
FindNext (
 char	byCode)
{
	if (find_rec (cmhr, (char *) &cmhr_rec, NEXT, "r"))
		return (FALSE);

#ifdef DEBUG
	if ( !strcmp ( cmhr_rec.cont_no, "NLA   "))
		PauseForKey (0,0, ML(mlCmMess116), 0);
#endif
	return (WithinBounds (byCode));
}

int
WithinBounds (
 char	byCode)
{
	switch (byCode)
	{
	case BY_CONTRACT	:
		return ((strcmp (cmhr_rec.co_no, comm_rec.tco_no) == 0) &&
			(strcmp (cmhr_rec.br_no, branchNo) == 0) &&
			(strcmp (cmhr_rec.cont_no, local_rec.end_code) <= 0));

	case BY_JOBTYPE	:
	case BY_ISSUETO	:
		return (!strcmp (cmhr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (cmhr_rec.br_no, branchNo));

	}
	sys_err ("Internal error on selection code", -1, PNAME);
	return (FALSE);
}

/**	Filtering routine
**/
int
PassFilter (
 char	byCode)
{
	if (!IsOpenContract (cmhr_rec.status) ||
			cmhr_rec.progress[0] != 'Y' ||
			strcmp (cmhr_rec.anni_day, local_rec.beg_ann_str) < 0 ||
			strcmp (cmhr_rec.anni_day, local_rec.end_ann_str) > 0)
		return (FALSE);

	switch (byCode)
	{
	case BY_CONTRACT	:
		return (TRUE);

	case BY_JOBTYPE	:
		if ((cc = find_hash (cmjt2, (char *) &cmjt_rec, EQUAL, "r", cmhr_rec.hhjt_hash)))
		{
			file_err (cc,cmjt2, "DBFIND");
			return (FALSE);
		}
		return (strcmp (cmjt_rec.job_type, local_rec.beg_code) >= 0 &&
			strcmp (cmjt_rec.job_type, local_rec.end_code) <= 0);

	case BY_ISSUETO	:
		if (cmhr_rec.hhit_hash == 0L)
			return(FALSE);
		if ((cc = find_hash (cmit2, (char *) &cmit_rec, EQUAL, "r", cmhr_rec.hhit_hash)))
		{
#ifdef DEBUG
			sprintf (temp, ML(mlCmMess091), cmhr_rec.cont_no);
			PauseForKey (0,0, temp, 0);
#endif
			file_err (cc,cmit2, "DBFIND");
			return (FALSE);
		}
		return (strcmp (cmit_rec.issto, local_rec.beg_code) >= 0 &&
			strcmp (cmit_rec.issto, local_rec.end_code) <= 0);

	}
 	sys_err ("Internal error on selection code", -1, PNAME);
	return (FALSE);
}

/**	Various search routines
**/
void
Search (
 char *	key_val)
{
	switch (*local_rec.by_code)
	{
	case BY_CONTRACT	:
		SrchCmhr (key_val);
		break;

	case BY_JOBTYPE	:
		SrchCmjt (key_val);
		break;

	case BY_ISSUETO	:
		SrchCmit (key_val);
		break;

	default	:
		sys_err ("Internal error on selection code", -1, PNAME);
	}
}

void
SrchCmhr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Contract", "#Description");

	/* Set up match codes
	*/
	memset (&cmhr_rec, 0, sizeof (cmhr_rec));	/* flush out */
	strcpy (cmhr_rec.co_no, comm_rec.tco_no);
	strcpy (cmhr_rec.br_no, branchNo);
	strcpy (cmhr_rec.cont_no, key_val);

	for (cc = find_rec (cmhr, (char *) &cmhr_rec, GTEQ, "r");
		!cc &&
			!strcmp (cmhr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (cmhr_rec.br_no, branchNo) &&
			!strncmp (cmhr_rec.cont_no, key_val, strlen (key_val));
		cc = find_rec (cmhr, (char *) &cmhr_rec, NEXT, "r"))
	{
		if (!IsOpenContract (cmhr_rec.status))
			continue;

		/* Pull out description line
		*/
		memset (&cmcd_rec, 0, sizeof (cmcd_rec));	/* flush */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;

		if (find_rec (cmcd, (char *) &cmcd_rec, EQUAL, "r"))
			strcpy (cmcd_rec.text, "**INTERNAL ERROR**");
		else
			cmcd_rec.text [31] = '\0';	/* trunc it */
		if (save_rec (cmhr_rec.cont_no, cmcd_rec.text))
			break;
	}
	disp_srch ();
	work_close ();
}

void
SrchCmjt (
 char *	key_val)
{
	work_open ();
	save_rec ("#Type", "#Description");

	/* Set up match codes
	*/
	memset (&cmjt_rec, 0, sizeof (cmjt_rec));	/* flush out */
	strcpy (cmjt_rec.co_no, comm_rec.tco_no);
	strcpy (cmjt_rec.br_no, comm_rec.test_no);
	strcpy (cmjt_rec.job_type, key_val);

	for (cc = find_rec (cmjt, (char *) &cmjt_rec, GTEQ, "r");
		!cc &&
			!strcmp (cmjt_rec.co_no, comm_rec.tco_no) &&
			!strcmp (cmjt_rec.br_no, comm_rec.test_no) &&
			!strncmp (cmjt_rec.job_type, key_val, strlen (key_val));
		cc = find_rec (cmjt, (char *) &cmjt_rec, NEXT, "r"))
	{
		if (save_rec (cmjt_rec.job_type, cmjt_rec.desc))
			break;
	}
	disp_srch ();
	work_close ();
}

void
SrchCmit (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code", "#Issued to");

	/* Set up match codes
	*/
	memset (&cmit_rec, 0, sizeof (cmit_rec));	/* flush out */
	strcpy (cmit_rec.co_no, comm_rec.tco_no);
	strcpy (cmit_rec.issto, key_val);

	for (cc = find_rec (cmit, (char *) &cmit_rec, GTEQ, "r");
		!cc &&
			!strcmp (cmit_rec.co_no, comm_rec.tco_no) &&
			!strncmp (cmit_rec.issto, key_val, strlen (key_val));
		cc = find_rec (cmit, (char *) &cmit_rec, NEXT, "r"))
	{
		if (save_rec (cmit_rec.issto, cmit_rec.iss_name))
			break;
	}
	disp_srch ();
	work_close ();
}

/**	Contract Info extraction
**/
void
GetContractInfo (void)
{
	float	qty;

	memset (&acc_info, 0, sizeof (acc_info));	/* clear accumulators */

	/* Look thru' progressive billing file (cmpb)
	*/
	cc = find_hash (cmpb, &cmpb_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmpb_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		acc_info.inv_to_date += cmpb_rec.amount;

		if ( cmpb_rec.date > acc_info.last_inv_date)
		{
			acc_info.last_inv_date = cmpb_rec.date;
			acc_info.last_inv_amt = cmpb_rec.amount;
		}

		cc = find_hash (cmpb, (char *) &cmpb_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}

	/* For contracts, Sales/Costs Amounts are the accumulated
	   Values that have been charged to the contract with blank stat_flag

	   This involves scanning
		1. cmts (time sheets) 
		2. cmtr (transaction file) 
	*/

	/*	Look thru' transaction file (cmtr)
	*/
	cc = find_hash (cmtr, &cmtr_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmtr_rec.stat_flag[0] != 'P')
		{
			/* unposted stuff 
			*/
			acc_info.cost_amt += cmtr_rec.cost_price * cmtr_rec.qty;
			acc_info.sale_amt += cmtr_rec.sale_price * 
					     cmtr_rec.qty *
					     ((100 - cmtr_rec.disc_pc) / 100);
		}
		else
		{
			/* posted stuff 
			*/
			acc_info.cst_to_date += cmtr_rec.cost_price * 
						cmtr_rec.qty;
		}
		cc = find_hash (cmtr, &cmtr_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}

	memset (&cmts_rec, 0, sizeof (cmts_rec));
	cc = find_hash (cmts, &cmts_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		qty = (float) (cmts_rec.time_ord +
		      (cmts_rec.time_hlf * 1.5) +
		      (cmts_rec.time_dbl * 2.0) +
		      cmts_rec.units);

		if ( cmts_rec.stat_flag[0] != 'P')
		{
			/* unposted stuff
			*/
			acc_info.cost_amt += (cmts_rec.oh_cost + 
					      cmts_rec.lab_cost) * qty;
			acc_info.sale_amt += cmts_rec.sale * qty;
		}
		else
		{
			/* posted stuff
			*/
			acc_info.cst_to_date += (cmts_rec.oh_cost + 
					      cmts_rec.lab_cost) * qty;
		}
		cc = find_hash (cmts, &cmts_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}
}

/**	Dumps out transaction details
	from cmtr, cmts

	Sorted by date
**/
void
ListDetails (
 FILE *	pout)
{
	char	line [100];
	char	*sortMask = "%010ld %c %010ld %.2f %.2f %.2f\n";
	char	*sortName = "anniprt";
	FILE	*sortFile = sort_open (sortName);
	char	*sortLine;
	float	qty;
	double	cst, sale;

	if (!sortFile)
	{
		sys_err ("!sort_open()", errno, PNAME);
		return;
	}

	/**	Let the sort file be composed of
		dddddddddd t hhhhhhhhhh cccccc.cc ssssss.ss

		dddddddddd	- 0 padded julian date
		t		- hash type : M)aterial, L)abour, E)quipment
		hhhhhhhhhh	- 0 padded hash value
		cccccc.cc	- cost value
		ssssss.ss	- sale value
		qqqqqq.qq	- quantity
	**/

	/*	Look thru' transaction file (cmtr)
	*/
	memset (&cmtr_rec, 0, sizeof (cmtr_rec));
	cc = find_hash (cmtr, (char *) &cmtr_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmtr_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmtr_rec.stat_flag[0] != 'P')
		{
			/* unposted stuff 
			*/
			sprintf (line, sortMask,
				cmtr_rec.date,
				'M',
				cmtr_rec.hhbr_hash,
				DOLLARS (cmtr_rec.cost_price),
				DOLLARS (cmtr_rec.sale_price * 
					((100.00 - cmtr_rec.disc_pc) / 100.00)),
				cmtr_rec.qty);
			sort_save (sortFile, line);
		}
		cc = find_hash (cmtr, &cmtr_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}

	/* Look thru' time sheets (cmts)
	*/
	memset (&cmts_rec, 0, sizeof (cmts_rec));
	cc = find_hash (cmts, &cmts_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		if (cmts_rec.stat_flag[0] != 'P')
		{
			/* unposted stuff 
			*/
			sprintf (line, sortMask,
				cmts_rec.date,
				cmts_rec.hheq_hash ? 'E' : 'L',
				cmts_rec.hheq_hash ? cmts_rec.hheq_hash : cmts_rec.hhem_hash,
				DOLLARS (cmts_rec.oh_cost + cmts_rec.lab_cost),
				DOLLARS (cmts_rec.sale),
				cmts_rec.time_ord +
				(cmts_rec.time_hlf * 1.5) +
				(cmts_rec.time_dbl * 2) +
				cmts_rec.units);
			sort_save (sortFile, line);
		}
		cc = find_hash (cmts, &cmts_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}

	/* Make the call to sort
	*/
	sortFile = sort_sort (sortFile, sortName);

	/* Read in and dump it out
	*/
	if ((sortLine = sort_read (sortFile)))
	{
		DetailHdg (pout);
		do	{
			long	julDate, hashVal;
			char	type;
			char	type_desc[5];
			char	desc [100];
			char	costVal [20],
				saleVal [20],
				qtyVal [20];

			sscanf (sortLine, "%ld %c %ld %s %s %s",
				&julDate,
				&type,
				&hashVal,
				costVal,
				saleVal,
				qtyVal);

			qty  = (float) atof (qtyVal);
			cst  = atof (costVal);
			sale = atof (saleVal);

			switch (type)
			{
			case 'E'	:
				sprintf ( type_desc, "%-4.4s", "Eqpt");
				cmeqDesc (hashVal, desc);
				break;
			case 'L'	:
				sprintf ( type_desc, "%-4.4s", "Labr");
				cmemDesc (hashVal, desc);
				break;

			case 'M'	:
				sprintf ( type_desc, "%-4.4s", "Matl");
				inmrDesc (hashVal, desc);
				break;

			default	:
				sys_err ("Internal error : sort?", -1, PNAME);
			}

			fprintf (pout, "|                     |%10.10s| %4.4s | %40.40s  |%9.2f  %9.2f |                        |\n",
				DateToString (julDate),
				type_desc,
				desc,
				sale * qty,
				cst  * qty);
		}	while ((sortLine = sort_read (sortFile)));

        fprintf (pout, "|                     =====================================================================================                        |\n");
		/* some spacing */
        	fprintf (pout, "|                                                                                                                                  |\n");
	}

	sort_delete (sortFile, sortName);
}

/**	Extraction routines for description
**/
int
GetDescription (
 char *	key,
 char *	dst)
{
	switch (*local_rec.by_code)
	{
	case BY_CONTRACT	:
		return (cmhrDescription (key, dst));
	case BY_JOBTYPE	:
		return (cmjtDescription (key, dst));
	case BY_ISSUETO	:
		return (cmitDescription (key, dst));
	}
	sys_err ("Internal error on selection code", -1, PNAME);
	return (FALSE);
}

int
cmhrDescription (
 char *	key,
 char *	dst)
{
	/* Validate existence of Contract
	*/
	memset (&cmhr_rec, 0, sizeof (cmhr_rec));	/* flush */
	strcpy (cmhr_rec.co_no, comm_rec.tco_no);
	strcpy (cmhr_rec.br_no, branchNo);
	strcpy (cmhr_rec.cont_no, pad_num (key));

	if (find_rec (cmhr, (char *) &cmhr_rec, EQUAL, "r"))
	{
		/*Contract '%-6.6s' not found*/
		print_mess (ML(mlStdMess075));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	if (!IsOpenContract (cmhr_rec.status))
	{
		/*Contract '%-6.6s' is not open*/
		print_mess (ML(mlCmMess018));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	memset (&cmcd_rec, 0, sizeof (cmcd_rec));	/* flush */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;

	if (find_rec (cmcd, (char *) &cmcd_rec, EQUAL, "r"))
	{
		/*Contract description not found*/
		print_mess (ML(mlCmMess020));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strncpy (dst, cmcd_rec.text, DESC_WIDTH);
	return (TRUE);
}

int
cmjtDescription (
 char *	key,
 char *	dst)
{
	/* Set up match
	*/
	memset (&cmjt_rec, 0, sizeof (cmjt_rec));	/* flush */
	strcpy (cmjt_rec.co_no, comm_rec.tco_no);
	strcpy (cmjt_rec.br_no, comm_rec.test_no);
	strcpy (cmjt_rec.job_type, key);

	if (find_rec (cmjt, (char *) &cmjt_rec, EQUAL, "r"))
	{
		/*Job type '%-4.4s' not found*/
		print_mess (ML(mlStdMess088));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmjt_rec.desc);
	return (TRUE);
}

int
cmitDescription (
 char *	key,
 char *	dst)
{
	/* Set up match
	*/
	memset (&cmit_rec, 0, sizeof (cmit_rec));	/* flush */
	strcpy (cmit_rec.co_no, comm_rec.tco_no);
	strcpy (cmit_rec.issto, key);

	if (find_rec (cmit, (char *) &cmit_rec, EQUAL, "r"))
	{
		/*"Issue code '%-10.10s' not found\007"*/
		print_mess (ML(mlCmMess054));
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	strcpy (dst, cmit_rec.iss_name);
	return (TRUE);
}

void
cmemDesc (
 long	hhem_hash,
 char *	dst)
{
	strcpy (dst,
		find_hash (cmem, (char *) &cmem_rec, EQUAL, "r", hhem_hash) ?
		"**ERROR : Employee name not found**" :
		cmem_rec.emp_name);
}

void
cmeqDesc (
 long	hheq_hash,
 char *	dst)
{
	strcpy (dst,
		find_hash (cmeq, (char *) &cmeq_rec, EQUAL, "r", hheq_hash) ?
		"**ERROR : Equipment desc not found**" :
		cmeq_rec.desc);
}

void
inmrDesc (
 long	hhbr_hash,
 char *	dst)
{
	strcpy (dst,
		find_hash (inmr, (char *) &inmr_rec, EQUAL, "r", hhbr_hash) ?
		"**ERROR : Material desc not found**" :
		inmr_rec.description);
}

/**	Support utilities
**/
int
IsOpenContract (
 char *	status)
{
	return (*status == STS_OPEN || *status == STS_BILLING);
}

void
DetailHdg (
 FILE *	pout)
{

	fprintf (pout, ".LRP5\n");
        fprintf (pout, "|                     =====================================================================================                        |\n");
        fprintf (pout, "|                     |                    U N P O S T E D   T R A N S A C T I O N S                      |                        |\n");
        fprintf (pout, "|                     |  Date    | Type | Description                               |    Sales     Costs  |                        |\n");
        fprintf (pout, "|                     |----------+------+-------------------------------------------+---------------------|                        |\n");
}
