/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_tsprnt.c   )                                  |
|  Program Desc  : ( Print/Display TimeSheet Transactions.        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pcat, prmr, pcwo, pcwc, rgrs,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pcat,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 09/11/93         |
|---------------------------------------------------------------------|
|  Date Modified : (23/11/93)      | Modified By : Aroha Merrilees.   |
|  Date Modified : (17/03/94)      | Modified By : Aroha Merrilees.   |
|  Date Modified : (01/09/95)      | Modified by : Scott B Darrow.    |
|  Date Modified : (03/09/97)      | Modified by : Leah Manibog.      |
|                                                                     |
|  Comments      :                                                    |
|  (23/11/93)    : DPL 9888 - select a range of employees, current    |
|                : branch only. Displays all records but only updates |
|                : stat_flag if not already 'P' or 'U'.               |
|  (17/03/94)    : DPL 10620 - was updating 'U' staus back to 'P'.    |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (03/09/97)    : Updated for Multilingual Conversion.               |
|                :                                                    |
|                                                                     |
| $Log: pc_tsprnt.c,v $
| Revision 5.4  2002/11/21 03:47:25  kaarlo
| LS01125 SC4182. Remove branch field from the search window for Start/End Employee code.
|
| Revision 5.3  2002/07/17 09:57:30  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:09  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:19  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/07 02:54:32  cam
| Changes for GVision compatibility.  Moved description fields 3 characters to
| the right.  Fixed the vars field mask for employee name fields.  Changed
| fprintf (fsort) to use sort_save ().
|
| Revision 1.12  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.11  1999/10/01 07:48:58  scott
| Updated for standard function calls.
|
| Revision 1.10  1999/09/29 10:11:41  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 08:26:26  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.8  1999/09/13 07:03:19  marlene
| *** empty log message ***
|
| Revision 1.7  1999/09/09 06:12:37  marlene
| *** empty log message ***
|
| Revision 1.6  1999/06/17 07:40:49  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_tsprnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_tsprnt/pc_tsprnt.c,v 5.4 2002/11/21 03:47:25 kaarlo Exp $";

#define	X_OFF		2
#define Y_OFF		5

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#define	SLEEP_TIME	2
#ifdef PSIZE
#undef PSIZE
#endif

#define	PSIZE	12

#define	PRINTER		(local_rec.prt_disp [0] == 'P')
#define	COMPANY		(local_rec.co_br [0] == 'C')
#define	SUB_TOTAL	(local_rec.sub [0] == 'Y')

char	*endLine = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGJGGGGGGGJGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGG"; 

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
	};

	int	comm_no_fields = 7;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tco_short [16];
		char	test_no [3];
		char	test_name [41];
		char	test_short [16];
	} comm_rec;

	/*======================================
	| Production Control Actual Times File |
	======================================*/
	struct dbview pcat_list [] =
	{
		{"pcat_date"},
		{"pcat_hhmr_hash"},
		{"pcat_hhrs_hash"},
		{"pcat_hhwo_hash"},
		{"pcat_seq_no"},
		{"pcat_hhwc_hash"},
		{"pcat_start_time"},
		{"pcat_setup"},
		{"pcat_run"},
		{"pcat_clean"},
		{"pcat_comment"},
		{"pcat_stat_flag"}
	};

	int	pcat_no_fields = 12;

	struct tag_pcatRecord
	{
		long	date;
		long	hhmr_hash;
		long	hhrs_hash;
		long	hhwo_hash;
		int		seq_no;
		long	hhwc_hash;
		long	start_time;
		long	setup;
		long	run;
		long	clean;
		char	comment [41];
		char	stat_flag [2];
	} pcat_rec;

	/*==============================
	| PayRoll employee Master file |
	==============================*/
	struct dbview prmr_list [] =
	{
		{"prmr_co_no"},
		{"prmr_br_no"},
		{"prmr_hhmr_hash"},
		{"prmr_code"},
		{"prmr_name"}
	};

	int	prmr_no_fields = 5;

	struct tag_prmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		long	hhmr_hash;
		char	code [9];
		char	name [41];
	} prmr_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_create_date"},
		{"pcwo_batch_no"},
	};

	int	pcwo_no_fields = 6;

	struct tag_pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	order_no [8];
		long	hhwo_hash;
		long	create_date;
		char	batch_no [11];
	} pcwo_rec;

	/*=======================
	| Work Centre Code file |
	=======================*/
	struct dbview pcwc_list [] =
	{
		{"pcwc_hhwc_hash"},
		{"pcwc_work_cntr"},
		{"pcwc_name"}
	};

	int	pcwc_no_fields = 3;

	struct tag_pcwcRecord
	{
		long	hhwc_hash;
		char	work_cntr [9];
		char	name [41];
	} pcwc_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list [] =
	{
		{"rgrs_hhrs_hash"},
		{"rgrs_code"},
		{"rgrs_desc"},
	};

	int	rgrs_no_fields = 3;

	struct tag_rgrsRecord
	{
		long	hhrs_hash;
		char	code [9];
		char	desc [41];
	} rgrs_rec;

	char	*data	= "data",
			*comm	= "comm",
			*pcat	= "pcat",
			*prmr	= "prmr",
			*pcwo	= "pcwo",
			*pcwc	= "pcwc",
			*rgrs	= "rgrs";
 
	FILE	*fout,
			*fsort;

	int		firstTime;
	int 	printed;
	char	temp_date [11];

	struct TOTALS {
		long	setup;
		long	run;
		long	clean;
	};

	struct TOTALS co_tot;
	struct TOTALS br_tot;
	struct TOTALS date_tot;
	struct TOTALS emp_tot;
	struct TOTALS work_tot;

	char	disp_str1 [150];
	char	disp_str2 [150];

	struct {
		char	order_no [8];
		char	emp_code [9];
		long	date;
		char	br_no [3];
	} prev_rec;

	struct {
		char	br_no [3];
		long	date;
		char	emp_name [41];
		char	emp_code [9];
		char	order_no [8];
		char	batch_no [11];
		int		seq_no;
		long	hhwc_hash;
		long	hhrs_hash;
		long	start_time;
		long	setup;
		long	run;
		long	clean;
		char	comment [41];
		char	status [2];
	} store_rec;

	struct {
		char	start_time [8];
		char	setup [8];
		char	run [8];
		char	clean [8];
	} temp_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	long	st_date;
	long	ed_date;

	char	st_emp [9];
	char	st_emp_name [41];
	char	ed_emp [9];
	char	ed_emp_name [41];

	char	co_br [2];
	char	cobr_desc [10];
	char	sub [2];
	char	sub_desc [4];

	char	prt_disp [2];
	char	prt_disp_desc [8];
	int		lp_no;

	char 	dummy [11];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_strdate",	 4, 30, EDATETYPE,
		"DD/DD/DD", "           ",
		" ", "00/00/00", "Start Date               : ", "Date Time was Allocated - Default : 00/00/00",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.st_date},
	{1, LIN, "ed_date",	 5, 30, EDATETYPE,
		"DD/DD/DD", "           ",
		" ", local_rec.systemDate, "End Date                 : ", "Date Time was Allocated - Default : Today's Date",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.ed_date},
	{1, LIN, "st_emp",	 7, 30, CHARTYPE,
		"UUUUUUUU", "           ",
		" ", " ", "Start Employee Code      : ", "Employee Name Code for this Branch - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_emp},
	{1, LIN, "st_emp_name",	 8, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ", " ", "Start Employee Name      : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_emp_name},
	{1, LIN, "ed_emp",	 9, 30, CHARTYPE,
		"UUUUUUUU", "           ",
		" ", "~~~~~~~~", "End Employee Code        : ", "Employee Name Code for this Branch - Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ed_emp},
	{1, LIN, "ed_emp_name",	 10, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ",
		" ", "~~~~~~~~", "End Employee Name        : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ed_emp_name},
	{1, LIN, "cobr",	 12, 30, CHARTYPE,
		"U", "          ",
		" ", "C", "Company or Branch        : ", "Company or Branch - Default : Company",
		YES, NO,  JUSTLEFT, "BC", "", local_rec.co_br},
	{1, LIN, "cobr_desc",	 12, 33, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cobr_desc},
	{1, LIN, "sub",	 13, 30, CHARTYPE,
		"U", "          ",
		" ", "Y", "Subtotal by Works Orders : ", "Enter Yes or No - Default : Yes",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.sub},
	{1, LIN, "sub_desc",	 13, 33, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sub_desc},
	{1, LIN, "prt_disp",	 15, 30, CHARTYPE,
		"U", "          ",
		" ", "D", "Display or Print         : ", "Display or Print - Default : Display",
		YES, NO,  JUSTLEFT, "DP", "", local_rec.prt_disp},
	{1, LIN, "prt_disp_desc",	 15, 33, CHARTYPE,
		"AAAAAAAAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.prt_disp_desc},
	{1, LIN, "lp_no",	16, 30, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No               : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lp_no},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| fucntion prototypes|
========================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void SrchPrmr (char *key_val);
void ProcReport (void);
int ValidEmployee (void);
void AddToSortFile (void);
void PrintDetails (void);
void PrintLine (void);
void PrintCoBr (void);
void PrintWOTotal (void);
void PrintEmpTotal (void);
void PrintDateTotal (void);
void PrintBranchTotal (void);
void PrintCompanyTotal (void);
int heading (int scn);
void InitOutput (void);
void InitDisplay (void);
void PrintHeading (void);
void DisplayHeading (void);
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc,
 char *argv[])
{
	SETUP_SCR (vars);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/


	/*------------------------------------
	| Step 1 : Read a valid pcat record  |
	|          using the start date.     |
	| Step 2 : Read employees (prmr), &  |
	|          works orders (pcwo), then |
	|          sort in same order.       |
	|          Update pcat stat_flag to  |
	|          'P'.                      |
	| Step 3 : Sort the above details.   |
	| Step 4 : Print details, showing    |
	|          totals for the company,   |
	|          the date, the employee,   |
	|          and maybe the works order.|
	------------------------------------*/
	while (prog_exit == 0)
	{
		printed = FALSE;

		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		crsr_on ();

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		/*============================
		| Process Orders in Database.|
		============================*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcReport ();

		if (PRINTER)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		else
		{
			Dsp_srch();
			Dsp_close();
		}
	}
	shutdown_prog ();
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
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (pcat,	 pcat_list, pcat_no_fields, "pcat_id_no2");
	open_rec (prmr,	 prmr_list, prmr_no_fields, "prmr_id_no");
	open_rec (pcwo,	 pcwo_list, pcwo_no_fields, "pcwo_hhwo_hash");
	open_rec (pcwc,	 pcwc_list, pcwc_no_fields, "pcwc_hhwc_hash");
	open_rec (rgrs,	 rgrs_list, rgrs_no_fields, "rgrs_hhrs_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pcat);
	abc_fclose (prmr);
	abc_fclose (pcwo);
	abc_fclose (pcwc);
	abc_fclose (rgrs);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("st_date") || LCHECK ("ed_date"))
	{
		if ((LCHECK ("ed_date") || prog_status != ENTRY) &&
			local_rec.st_date > local_rec.ed_date)
		{
			/*strcpy (temp_date,DateToString (local_rec.ed_date));
			sprintf (err_str,
					"Start Date %s Greater Than End Date %s",
					DateToString (local_rec.st_date),
					temp_date);*/

			print_mess (ML(mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_emp"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.st_emp_name, "%-40.40s", "First Employee");
			DSP_FLD ("st_emp_name");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPrmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (prmr_rec.co_no, comm_rec.tco_no);
		strcpy (prmr_rec.br_no, comm_rec.test_no);
		strcpy (prmr_rec.code, local_rec.st_emp);
		cc = find_rec (prmr, &prmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*sprintf (err_str, "Employee %s Not found", local_rec.st_emp);*/
			print_mess (ML(mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_emp_name, "%-40.40s", " ");
			DSP_FLD ("st_emp_name");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_emp, local_rec.ed_emp) > 0)
		{
			/*sprintf (err_str,
				"Start Employee [%-8.8s] Greater Than End Employee [%-8.8s]",
				local_rec.st_emp,
				local_rec.ed_emp);*/

			print_mess (ML(mlStdMess017));

			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_emp_name, "%-40.40s", " ");
			DSP_FLD ("st_emp_name");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.st_emp_name, prmr_rec.name);
		DSP_FLD ("st_emp_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ed_emp"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.ed_emp_name, "%-40.40s", "Last Employee");
			DSP_FLD ("ed_emp_name");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPrmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (prmr_rec.co_no, comm_rec.tco_no);
		strcpy (prmr_rec.br_no, comm_rec.test_no);
		strcpy (prmr_rec.code, local_rec.ed_emp);
		cc = find_rec (prmr, &prmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*sprintf (err_str, "Employee %s Not found", local_rec.ed_emp);*/
			print_mess (ML(mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_emp_name, "%-40.40s", " ");
			DSP_FLD ("ed_emp_name");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_emp, local_rec.ed_emp) > 0)
		{
			/*sprintf (err_str,
				"Start Employee [%-8.8s] Greater Than End Employee [%-8.8s]",
				local_rec.st_emp,
				local_rec.ed_emp);*/
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_emp_name, "%-40.40s", " ");
			DSP_FLD ("ed_emp_name");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ed_emp_name, prmr_rec.name);
		DSP_FLD ("ed_emp_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cobr"))
	{
		strcpy (local_rec.cobr_desc, "         ");
		DSP_FLD ("cobr_desc");
		switch (local_rec.co_br [0])
		{
		case	'C':
			strcpy (local_rec.cobr_desc, "Company  ");
			break;
		case	'B':
			strcpy (local_rec.cobr_desc, "Branch   ");
			break;
		}
		DSP_FLD ("cobr_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sub"))
	{
		*local_rec.sub_desc = '\0';
		DSP_FLD ("sub_desc");

		if (local_rec.sub [0] == 'Y')
			strcpy (local_rec.sub_desc, "Yes");
		else
			strcpy (local_rec.sub_desc, "No ");

		DSP_FLD ("sub_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prt_disp"))
	{
		*local_rec.prt_disp_desc = '\0';
		DSP_FLD ("prt_disp_desc");

		if (local_rec.prt_disp [0] == 'D')
		{
			strcpy (local_rec.prt_disp_desc, "Display");
			DSP_FLD ("prt_disp_desc");
			FLD ("lp_no") = NA;
			local_rec.lp_no = 0;
			DSP_FLD ("lp_no");
		}
		else
		{
			strcpy (local_rec.prt_disp_desc, "Print  ");
			DSP_FLD ("prt_disp_desc");
			FLD ("lp_no") = YES;
			if (prog_status != ENTRY)
			{
				/*print_mess ("Please ensure a printer number has been entered.");*/
				print_mess (ML(mlPcMess052));
				sleep (sleepTime);
				clear_mess ();
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lp_no"))
	{
		if (FLD ("lp_no") != NA) 
		{
			if (SRCH_KEY)
			{
				local_rec.lp_no = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lp_no))
			{
				/*print_mess ("Invalid Printer");*/
				print_mess (ML(mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=============================
| Search on Employee details. |
=============================*/
void
SrchPrmr (
 char *key_val)
{
    work_open ();
	save_rec ("#Emp Code", "#Employee Name");

	strcpy (prmr_rec.co_no, comm_rec.tco_no);
	strcpy (prmr_rec.br_no, comm_rec.test_no);
	sprintf (prmr_rec.code, "%-8.8s", key_val);
	cc = find_rec (prmr, &prmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (prmr_rec.co_no, comm_rec.tco_no) &&
		!strcmp (prmr_rec.br_no, comm_rec.test_no) &&
		!strncmp (prmr_rec.code, key_val, strlen (key_val)))
	{
		/*sprintf (err_str, "%-2.2s | %-40.40s", prmr_rec.br_no, prmr_rec.name);
		cc = save_rec (prmr_rec.code, err_str);*/
		cc = save_rec (prmr_rec.code, prmr_rec.name);
		if (cc)
			break;

		cc = find_rec (prmr, &prmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (prmr_rec.co_no, comm_rec.tco_no);
	strcpy (prmr_rec.br_no, comm_rec.test_no);
	strcpy (prmr_rec.code, temp_str);
	cc = find_rec (prmr, &prmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, prmr, "DBFIND");
}

void
ProcReport (
 void)
{
	int		sortOpen = FALSE;

	if (PRINTER)
		PrintHeading ();
	else
		DisplayHeading ();

	/*---------------------
	| initialising totals |
	---------------------*/
	co_tot.setup = 
		co_tot.run =
		co_tot.clean = 0;
	br_tot.setup = 
		br_tot.run =
		br_tot.clean = 0;
	date_tot.setup = 
		date_tot.run =
		date_tot.clean = 0;
	emp_tot.setup = 
		emp_tot.run =
		emp_tot.clean = 0;
	if (SUB_TOTAL)
	{
		work_tot.setup = 
			work_tot.run =
			work_tot.clean = 0;
	}

	abc_selfield (prmr, "prmr_hhmr_hash");
	/*------------------------------------
	| Step 1 : Read a valid pcat record  |
	|          using the start date.     |
	------------------------------------*/
	pcat_rec.date = local_rec.st_date;
	pcat_rec.hhmr_hash = 0L;
	pcat_rec.hhwo_hash = 0L;
	pcat_rec.seq_no = 0;
	pcat_rec.hhrs_hash = 0L;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "r");
	while (!cc && 
		pcat_rec.date <= local_rec.ed_date)
	{
		if (!sortOpen)
		{
			fsort = sort_open ("time");
			sortOpen = TRUE;
		}

		/*------------------------------------
		| Step 2 : Read employees (prmr), &  |
		|          works orders (pcwo), then |
		|          sort in same order.       |
		|          Update pcat stat_flag to  |
		|          'P'.                      |
		------------------------------------*/
		cc = find_hash (prmr, &prmr_rec, COMPARISON, "r", pcat_rec.hhmr_hash);
		if (cc)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}
		if (!ValidEmployee ())
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}

		cc = find_hash (pcwo, &pcwo_rec, COMPARISON, "r", pcat_rec.hhwo_hash);
		if (cc)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}

		/*-------------------------------------------------
		| If the level is BRANCH, and the branches of the |
		| works order and the common file not match, get  |
		| the next pcat record.                           |
		-------------------------------------------------*/
		if (!COMPANY && strcmp (pcwo_rec.br_no, comm_rec.test_no))
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			continue;
		}

		AddToSortFile ();

		/*------------------------------------------------
		| update stat_flag to 'P' if not already updated |
		| to 'P' or 'U'                                  |
		------------------------------------------------*/
		if (pcat_rec.stat_flag [0] != 'P' &&
			pcat_rec.stat_flag [0] != 'U')
		{
			pcat_rec.stat_flag [0] = 'P';
			cc = abc_update (pcat, &pcat_rec);
			if (cc)
				file_err (cc, pcat, "DBUPDATE");
		}

		cc = find_rec (pcat, &pcat_rec, NEXT, "r");
	}

	/*------------------------------------
	| Step 3 : Sort the above details.   |
	------------------------------------*/
	if (sortOpen)
	{
		fsort = sort_sort (fsort, "time");

		/*------------------------------------
		| Step 4 : Print details, showing    |
		|          totals for the company,   |
		|          the date, the employee,   |
		|          and maybe the works order.|
		------------------------------------*/
		PrintDetails ();

		sort_delete (fsort, "time");
		sortOpen = FALSE;
	}

	/*------------------------------
	| nothing printed so tell user |
	------------------------------*/
	if (!printed)
	{
		if (PRINTER)
			fprintf (fout, ".E NO TIMESHEET'S FOUND FOR ANY EMPLOYEES \n");
	}
	abc_selfield (prmr, "prmr_id_no");
}

int
ValidEmployee (
 void)
{
	if (strcmp (local_rec.st_emp, prmr_rec.code) > 0)
		return (FALSE);

	if (strcmp (prmr_rec.code, local_rec.ed_emp) > 0)
		return (FALSE);

	return (TRUE);
}

void
AddToSortFile (
 void)
{
	char	sortStr [512];

	sprintf (sortStr,
			"%-2.2s %-10.10s %-40.40s %-8.8s %-7.7s %-10.10s %3d %11ld %11ld %11ld %11ld %11ld %11ld %-40.40s %-1.1s\n",
			pcwo_rec.br_no,
			DateToString (pcat_rec.date),
			prmr_rec.name,
			prmr_rec.code,
			pcwo_rec.order_no,
			pcwo_rec.batch_no,
			pcat_rec.seq_no,
			pcat_rec.hhwc_hash,
			pcat_rec.hhrs_hash,
			pcat_rec.start_time,
			pcat_rec.setup,
			pcat_rec.run,
			pcat_rec.clean,
			pcat_rec.comment,
			pcat_rec.stat_flag);
	sort_save (fsort, sortStr);
}

void
PrintDetails (
 void)
{
	char	*sptr;

	firstTime = TRUE;
	sptr = sort_read (fsort);

	sprintf (store_rec.br_no,	"%-2.2s", sptr);
	strcpy (prev_rec.br_no, store_rec.br_no);
	prev_rec.date = StringToDate (sptr + 3);
	sprintf (prev_rec.emp_code, "%-8.8s", sptr + 55);
	sprintf (prev_rec.order_no, "%-7.7s", sptr + 64);
	while (sptr)
	{
		sprintf (store_rec.br_no,		"%-2.2s",	sptr);
		store_rec.date					= StringToDate (sptr + 3);
		sprintf (store_rec.emp_name,	"%-40.40s",	sptr + 14);
		sprintf (store_rec.emp_code,	"%-8.8s",	sptr + 55);
		sprintf (store_rec.order_no,	"%-7.7s",	sptr + 64);
		sprintf (store_rec.batch_no,	"%-10.10s",	sptr + 72);
		store_rec.seq_no				= atoi (sptr + 83);
		store_rec.hhwc_hash				= atol (sptr + 87);
		store_rec.hhrs_hash				= atol (sptr + 99);
		store_rec.start_time			= atol (sptr + 111);
		store_rec.setup					= atol (sptr + 123);
		store_rec.run					= atol (sptr + 135);
		store_rec.clean					= atol (sptr + 147);
		sprintf (store_rec.comment,		"%-40.40s",	sptr + 159);
		sprintf (store_rec.status,		"%-1.1s",	sptr + 200);

		if (!COMPANY &&
			strcmp (store_rec.br_no, comm_rec.test_no))
		{
			strcpy (prev_rec.order_no,	store_rec.order_no);
			strcpy (prev_rec.emp_code,	store_rec.emp_code);
			prev_rec.date = store_rec.date;
			strcpy (prev_rec.br_no,		store_rec.br_no);

			sptr = sort_read (fsort);
			continue;
		}

		if (printed)
		{
			if (strcmp (prev_rec.order_no, store_rec.order_no) ||
				strcmp (prev_rec.emp_code, store_rec.emp_code) ||
				prev_rec.date != store_rec.date ||
				strcmp (prev_rec.br_no, store_rec.br_no))
			{
				if (SUB_TOTAL)
					PrintWOTotal ();
				strcpy (prev_rec.order_no, store_rec.order_no);
				firstTime = TRUE;
			}

			if (strcmp (prev_rec.emp_code, store_rec.emp_code) ||
				prev_rec.date != store_rec.date ||
				strcmp (prev_rec.br_no, store_rec.br_no))
			{
				PrintEmpTotal ();
				strcpy (prev_rec.emp_code, store_rec.emp_code);
				firstTime = TRUE;
			}

			if (prev_rec.date != store_rec.date ||
				strcmp (prev_rec.br_no, store_rec.br_no))
			{
				PrintDateTotal ();
				prev_rec.date = store_rec.date;
				firstTime = TRUE;
			}

			if (strcmp (prev_rec.br_no, store_rec.br_no))
			{
				PrintBranchTotal ();
				PrintCoBr ();
				strcpy (prev_rec.br_no, store_rec.br_no);
				firstTime = TRUE;
			}
		}
		/*-----------------------------
		| get work centre information |
		-----------------------------*/
		cc = find_hash (pcwc, &pcwc_rec, COMPARISON, "r", store_rec.hhwc_hash);
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		/*--------------------------
		| get resource information |
		--------------------------*/
		cc = find_hash (rgrs, &rgrs_rec, COMPARISON, "r", store_rec.hhrs_hash);
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		PrintLine ();

		sptr = sort_read (fsort);
	}
	if (printed)
	{
		if (SUB_TOTAL)
			PrintWOTotal ();
		PrintEmpTotal ();
		PrintDateTotal ();
		PrintBranchTotal ();
		if (COMPANY)
			PrintCompanyTotal ();
		
		if (!PRINTER)
			Dsp_saverec (endLine);
	}
}

void
PrintLine (
 void)
{
	char	status [4];

	if (!printed)
	{
		PrintCoBr ();
		printed = TRUE;
	}

	switch (store_rec.status [0])
	{
	case 'P' :
		strcpy (status, "Prt");
		break;
	case 'U' :
		strcpy (status, "Upd");
		break;
	default :
		sprintf (status, "%-3.3s", " ");
		break;
	}

	if (PRINTER)
	{
		dsp_process ("Employee :", clip (store_rec.emp_name));

		if (firstTime)
		{
			fprintf (fout, "|%-10.10s",	DateToString (store_rec.date));
			fprintf (fout, "|%-8.8s",	store_rec.emp_code);
			fprintf (fout, "|%-16.16s",	store_rec.emp_name);
			fprintf (fout, "|%-7.7s",	store_rec.order_no);
			fprintf (fout, "|%10.10s",	store_rec.batch_no);
			firstTime = FALSE;
		}
		else
		{
			fprintf (fout, "|%-10.10s",	" ");
			fprintf (fout, "|%-8.8s",	" ");
			fprintf (fout, "|%-16.16s",	" ");
			fprintf (fout, "|%-7.7s",	" ");
			fprintf (fout, "|%10.10s",	" ");
		}
		fprintf (fout, "|%3d",			store_rec.seq_no);
		fprintf (fout, "|%-8.8s",		pcwc_rec.work_cntr);
		fprintf (fout, "|%-8.8s",		rgrs_rec.code);
		fprintf (fout, "|%s",			ttoa (store_rec.start_time,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (store_rec.setup,		"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (store_rec.run,		"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (store_rec.clean,		"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		store_rec.comment);
		fprintf (fout, "|%-3.3s|\n",	status);
	}
	else
	{
		if (firstTime)
		{
			sprintf (disp_str1,
				"%-10.10s^E%-8.8s^E%-14.14s^E%-7.7s^E%10.10s",
				DateToString (store_rec.date),
				store_rec.emp_code,
				store_rec.emp_name,
				store_rec.order_no,
				store_rec.batch_no);
			firstTime = FALSE;
		}
		else
		{
			sprintf (disp_str1,
				"%-10.10s^E%-8.8s^E%-14.14s^E%-7.7s^E%10.10s",
				" ",
				" ",
				" ",
				" ",
				" ");
		}
		sprintf (temp_rec.start_time, 
			"%s", 
			ttoa (store_rec.start_time, "NNNN:NN"));
		sprintf (temp_rec.setup,	"%s", ttoa (store_rec.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (store_rec.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (store_rec.clean, "NNNN:NN"));

		sprintf (disp_str2,
			"^E%3d^E%-8.8s^E%-8.8s^E%s^E%s^E%s^E%s^E%-16.16s^E%-3.3s",
			store_rec.seq_no,
			pcwc_rec.work_cntr,
			rgrs_rec.code,
			temp_rec.start_time,
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean,
			store_rec.comment,
			status);

		strcat (disp_str1, disp_str2);

		Dsp_saverec (disp_str1);
	}

	/*------------------
	| calculate totals |
	------------------*/
	if (SUB_TOTAL)
	{
		work_tot.setup += store_rec.setup;
		work_tot.run += store_rec.run;
		work_tot.clean += store_rec.clean;
	}

	emp_tot.setup += store_rec.setup;
	emp_tot.run += store_rec.run;
	emp_tot.clean += store_rec.clean;

	date_tot.setup += store_rec.setup;
	date_tot.run += store_rec.run;
	date_tot.clean += store_rec.clean;

	br_tot.setup += store_rec.setup;
	br_tot.run += store_rec.run;
	br_tot.clean += store_rec.clean;

	if (COMPANY)
	{
		co_tot.setup += store_rec.setup;
		co_tot.run += store_rec.run;
		co_tot.clean += store_rec.clean;
	}
}

void
PrintCoBr (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "| COMPANY : %-2.2s ", comm_rec.tco_no);
		fprintf (fout, "   BRANCH : %-2.2s      ", store_rec.br_no);
		fprintf (fout, "          ");
		fprintf (fout, "           ");
		fprintf (fout, "    ");
		fprintf (fout, "         ");
		fprintf (fout, "         ");
		fprintf (fout, "        ");
		fprintf (fout, "        ");
		fprintf (fout, "        ");
		fprintf (fout, "        ");
		fprintf (fout, "                                          ");
		fprintf (fout, "   |\n");
	}
	else
	{
		if (printed)
			Dsp_saverec (endLine);

		sprintf (disp_str1,
			"Company : %-2.2s    Branch : %-2.2s",
			comm_rec.tco_no,
			store_rec.br_no);
		Dsp_saverec (disp_str1);
	}
}

void
PrintWOTotal (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "|%-10.10s",		" ");
		fprintf (fout, " %-8.8s",		" ");
		fprintf (fout, " %-16.16s",		" ");
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout,
			"%-20.20s(%-7.7s)    ",
			"Works Order Total",
			prev_rec.order_no);
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout, "|%s",			ttoa (work_tot.setup,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (work_tot.run,		"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (work_tot.clean,	"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		" ");
		fprintf (fout, " %-3.3s|\n",	" ");
	}
	else
	{
		sprintf (temp_rec.setup,	"%s", ttoa (work_tot.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (work_tot.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (work_tot.clean, "NNNN:NN"));

		sprintf (disp_str1,
			"%-50.50s%-19.19s (%-8.8s) ^E%s^E%s^E%s^E",
			" ",
			" Works Order Total ",
			prev_rec.order_no,
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean);
		Dsp_saverec (disp_str1);
	}

	work_tot.setup =
		work_tot.run =
		work_tot.clean = 0;
}

void
PrintEmpTotal (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "|%-10.10s",		" ");
		fprintf (fout, " %-8.8s",		" ");
		fprintf (fout, " %-16.16s",		" ");
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout,
			"%-20.20s(%-8.8s)   ",
			"Employee Total   ",
			prev_rec.emp_code);
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout, "|%s",			ttoa (emp_tot.setup,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (emp_tot.run,		"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (emp_tot.clean,	"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		" ");
		fprintf (fout, " %-3.3s|\n",	" ");
	}
	else
	{
		sprintf (temp_rec.setup,	"%s", ttoa (emp_tot.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (emp_tot.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (emp_tot.clean, "NNNN:NN"));

		sprintf (disp_str1,
			"%-50.50s%-19.19s (%-8.8s) ^E%s^E%s^E%s^E",
			" ",
			" Employee Total    ",
			prev_rec.emp_code,
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean);
		Dsp_saverec (disp_str1);
	}

	emp_tot.setup =
		emp_tot.run =
		emp_tot.clean = 0;
}

void
PrintDateTotal (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "|%-10.10s",		" ");
		fprintf (fout, " %-8.8s",		" ");
		fprintf (fout, " %-16.16s",		" ");
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout,
			"%-20.20s(%-10.10s)   ",
			"Date Total       ",
			DateToString (prev_rec.date));
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout, "|%s",			ttoa (date_tot.setup,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (date_tot.run,		"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (date_tot.clean,	"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		" ");
		fprintf (fout, " %-3.3s|\n",	" ");
	}
	else
	{
		sprintf (temp_rec.setup,	"%s", ttoa (date_tot.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (date_tot.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (date_tot.clean, "NNNN:NN"));

		sprintf (disp_str1,
			"%-50.50s%-19.19s (%-10.10s) ^E%s^E%s^E%s^E",
			" ",
			" Date Total        ",
			DateToString (prev_rec.date),
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean);
		Dsp_saverec (disp_str1);
	}

	date_tot.setup =
		date_tot.run =
		date_tot.clean = 0;
}

void
PrintBranchTotal (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "|%-10.10s",		" ");
		fprintf (fout, " %-8.8s",		" ");
		fprintf (fout, " %-16.16s",		" ");
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout,
			"%-20.20s(%-2.2s)         ",
			"Branch Total     ",
			prev_rec.br_no);
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout, "|%s",			ttoa (br_tot.setup,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (br_tot.run,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (br_tot.clean,	"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		" ");
		fprintf (fout, " %-3.3s|\n",	" ");
	}
	else
	{
		sprintf (temp_rec.setup,	"%s", ttoa (br_tot.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (br_tot.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (br_tot.clean, "NNNN:NN"));

		sprintf (disp_str1,
			"%-50.50s%-19.19s (%-2.2s)       ^E%s^E%s^E%s^E",
			" ",
			" Branch Total      ",
			prev_rec.br_no,
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean);
		Dsp_saverec (disp_str1);
	}

	br_tot.setup =
		br_tot.run =
		br_tot.clean = 0;
}

void
PrintCompanyTotal (
 void)
{
	if (PRINTER)
	{
		fprintf (fout, "|%-10.10s",		" ");
		fprintf (fout, " %-8.8s",		" ");
		fprintf (fout, " %-16.16s",		" ");
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout,
			"%-20.20s(%-2.2s)         ",
			"Company Total    ",
			comm_rec.tco_no);
		fprintf (fout, " %-7.7s",		" ");
		fprintf (fout, "|%s",			ttoa (co_tot.setup,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (co_tot.run,	"NNNN:NN"));
		fprintf (fout, "|%s",			ttoa (co_tot.clean,	"NNNN:NN"));
		fprintf (fout, "|%-40.40s",		" ");
		fprintf (fout, " %-3.3s|\n",	" ");
	}
	else
	{
		sprintf (temp_rec.setup,	"%s", ttoa (co_tot.setup, "NNNN:NN"));
		sprintf (temp_rec.run,		"%s", ttoa (co_tot.run, "NNNN:NN"));
		sprintf (temp_rec.clean,	"%s", ttoa (co_tot.clean, "NNNN:NN"));

		sprintf (disp_str1,
			"%-50.50s%-19.19s (%-2.2s)       ^E%s^E%s^E%s^E",
			" ",
			" Company Total     ",
			comm_rec.tco_no,
			temp_rec.setup,
			temp_rec.run,
			temp_rec.clean);
		Dsp_saverec (disp_str1);
	}

	co_tot.setup =
		co_tot.run =
		co_tot.clean = 0;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		snorm ();
		rv_pr (ML(mlPcMess050) , 26, 0, 1);

		move (0, 1);
		line (80);

		box (0, 3, 80, 13);
		move (1, 6);
		line (79);
		move (1, 11);
		line (79);
		move (1, 14);
		line (79);

		move (0, 20);
		line (80);

		print_at (21,0, ML(mlStdMess038),
			comm_rec.tco_no,
			clip (comm_rec.tco_name));
		print_at (21,45, ML(mlStdMess039),
			comm_rec.test_no,
			clip (comm_rec.test_name));


		move (0, 22);
		line (80);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	if (PRINTER)
	{
		dsp_screen (" Printing TimeSheet Audit Trial ",
				comm_rec.tco_no, 
				comm_rec.tco_name);

		/*----------------------
		| Open pipe to pformat | 
		----------------------*/
		if ( (fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.lp_no);
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L156\n");
	}
	else
		InitDisplay ();
}

/*======================================================
| DISPLAY SCREEN.                                      |
| Display Heading at screen after clearing the screen. |
======================================================*/
void
InitDisplay (
 void)
{
	strcpy (temp_date, DateToString (local_rec.ed_date));

	clear ();
	swide ();

/*	rv_pr ("D I S P L A Y   T I M E S H E E T   A U D I T   T R I A L",
		30, 0, 1); 
*/
	rv_pr (ML(mlPcMess051), 30, 0, 1); 

 	print_at (2, 2,
		 ML(mlPcMess045),
		clip (comm_rec.tco_name));

	print_at (3, 2,
		 ML(mlStdMess112),
		DateToString (local_rec.st_date));
	print_at (3, 15,
		 ML(mlStdMess113),
		temp_date);

	print_at (4, 2,
		ML(mlPcMess046),
		local_rec.st_emp);
	print_at (4, 12,
		ML(mlPcMess047),
		local_rec.ed_emp);

	print_at (5, 2, 
		ML(mlPcMess048), 
		local_rec.cobr_desc);
	print_at (5, 12, 
		ML(mlPcMess049), 
		local_rec.sub_desc);

}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".E TIMESHEET AUDIT TRIAL \n");

	fprintf (fout, ".E COMPANY NAME : %s \n", clip (comm_rec.tco_name));
	fprintf (fout, ".E BRANCH NAME : %s \n", clip (comm_rec.test_name));

	sprintf (temp_date, "%s", DateToString (local_rec.ed_date));
	fprintf (fout, 
			".E START DATE : %s  END DATE : %s\n",
			DateToString (local_rec.st_date),
			temp_date);

	fprintf (fout,
			".E START EMPLOYEE : %s  END EMPLOYEE : %s\n",
			local_rec.st_emp,
			local_rec.ed_emp);

	fprintf (fout, 
			".E LEVEL : %s  SUB TOTALS : %s\n",
  			local_rec.cobr_desc,
			local_rec.sub_desc);

	fprintf (fout, ".B1\n");

	fprintf (fout, "===========");
	fprintf (fout, "=========");
	fprintf (fout, "=================");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "====");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "=========");
	fprintf (fout, "========================================");
	fprintf (fout, "=====\n");

	fprintf (fout, "|  ENTRY   ");
	fprintf (fout, "|EMPLOYEE");
	fprintf (fout, "|    EMPLOYEE    ");
	fprintf (fout, "| WORKS ");
	fprintf (fout, "|  BATCH   ");
	fprintf (fout, "|SEQ");
	fprintf (fout, "| WORKS  ");
	fprintf (fout, "|RESOURCE");
	fprintf (fout, "| START ");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "|               COMMENTS                 ");
	fprintf (fout, "|STS|\n");

	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   NO   ");
	fprintf (fout, "|      NAME      ");
	fprintf (fout, "|ORD. NO");
	fprintf (fout, "|    NO    ");
	fprintf (fout, "|NO ");
	fprintf (fout, "|CENT. NO");
	fprintf (fout, "|   NO   ");
	fprintf (fout, "| TIME  ");
	fprintf (fout, "|  HRS  ");
	fprintf (fout, "|  HRS  ");
	fprintf (fout, "|  HRS  ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "|   |\n");

	fprintf (fout, "|----------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|-------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|-------");
	fprintf (fout, "|-------");
	fprintf (fout, "|-------");
	fprintf (fout, "|-------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|---|\n");

	fprintf (fout, ".R===========");
	fprintf (fout, "=========");
	fprintf (fout, "=================");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "====");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "=========");
	fprintf (fout, "========================================");
	fprintf (fout, "=====\n");
}

void
DisplayHeading (
 void)
{
	Dsp_nc_prn_open (0, 6, PSIZE, err_str,
			comm_rec.tco_no, comm_rec.tco_name,
			comm_rec.test_no, comm_rec.test_name,
			(char *)0, (char *)0);

	Dsp_saverec ("  ENTRY   |EMPLOYEE|   EMPLOYEE   | WORKS |  BATCH   |SEQ| WORKS  |RESOURCE| START | SETUP |  RUN  | CLEAN |    COMMENTS     |STS");
	Dsp_saverec ("   DATE   |   NO   |     NAME     |ORD. NO|    NO    |NO |CENT. NO|   NO   | TIME  |  HRS  |  HRS  |  HRS  |                 |   ");
	Dsp_saverec (" [Redraw] [Print] [Next] [Prev] [End/Input] ");
}

