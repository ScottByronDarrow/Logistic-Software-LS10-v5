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
|  Author        : Aroha Merrilees   Date Written  : 18/11/93         |
|---------------------------------------------------------------------|
|  Date Modified : (25/01/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (08/11/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (04/09/1997)      Modified By   : Jiggs A Veloz  . |
|                                                                     |
|  Comments      :                                                    |
|  (25/01/94)    : DPL 9673 - Delete for current warehouse only.      |
|  (08/11/94)    : PSL 11527 - change aduit trial comment             |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|                :                                                    |
| $Log: pc_tspurge.c,v $
| Revision 5.3  2002/07/17 09:57:30  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:10  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:34  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:11  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:19  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.11  2000/03/07 12:01:29  ramon
| For GVision compatibility, I move the description field 3 chars to the right and also added a flag during sorting.
|
| Revision 1.10  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.9  1999/09/29 10:11:41  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 08:26:27  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.7  1999/09/13 07:03:20  marlene
| *** empty log message ***
|
| Revision 1.6  1999/09/09 06:12:37  marlene
| *** empty log message ***
|
| Revision 1.5  1999/06/17 07:40:50  scott
| Update for database name and Log file additions required for cvs.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_tspurge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_tspurge/pc_tspurge.c,v 5.3 2002/07/17 09:57:30 scott Exp $";

#define	X_OFF		2
#define Y_OFF		5

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_pc_mess.h>
#include <ml_std_mess.h>

#define	SLEEP_TIME	2
#define AUDIT		(local_rec.trail [0] == 'Y')

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
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
	};

	int	comm_no_fields = 10;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tco_short [16];
		char	test_no [3];
		char	test_name [41];
		char	test_short [16];
		char	tcc_no [3];
		char	tcc_name [41];
		char	tcc_short [10];
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
	} pcat_rec, pcat_rec2;

	/*==============================
	| PayRoll employee Master file |
	==============================*/
	struct dbview prmr_list [] =
	{
		{"prmr_hhmr_hash"},
		{"prmr_code"},
		{"prmr_name"}
	};

	int	prmr_no_fields = 3;

	struct tag_prmrRecord
	{
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
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_create_date"},
		{"pcwo_batch_no"},
	};

	int	pcwo_no_fields = 7;

	struct tag_pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
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

	struct TOTALS {
		long	setup;
		long	run;
		long	clean;
	};

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
	} prev_rec;

	struct {
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
	long	date;
	char	trail [2];
	char	trail_desc [4];
	int		lp_no;

	char 	dummy [11];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "date",	 4, 30, EDATETYPE,
		"DD/DD/DD", "           ",
		" ", local_rec.systemDate, "Purge Date                  : ", "Purge All TimeSheets upto and including Date - Default : Today",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.date},
	{1, LIN, "trail",	 6, 30, CHARTYPE,
		"U", "          ",
		" ", "Y", "Audit Trial Required        : ", "Enter Yes or No - Default : Yes",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.trail},
	{1, LIN, "trail_desc",	 6, 33, CHARTYPE,
		"AAA", "          ",
		"", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.trail_desc},
	{1, LIN, "lp_no",	8, 30, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No                  : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lp_no},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=============================
| function prototypes |
=====================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void ProcessRecords (void);
void ProcessReport (void);
void PrintToSortFile (void);
void PrintDetails (void);
void PrintLine (void);
void PrintCoBr (void);
void PrintWOTotal (void);
void PrintEmpTotal (void);
void PrintDateTotal (void);
void PrintBranchTotal (void);
int heading (int scn);
void InitOutput (void);
void PrintHeading (void);

			
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


	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*------------------------------------
	| Step 1 : Read a valid pcat record. |
	|          Purges all records, upto  |
	|          and including date entered|
	|          only if records have been |
	|          updated (status = 'U').   |
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

		/*----------------------------
		| Process Orders in Database.|
		----------------------------*/
		clear ();
		crsr_off ();
		fflush (stdout);
		if (AUDIT)
		{
			OpenDB ();
			InitOutput ();

			ProcessReport ();

			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		else
			ProcessRecords ();
		break;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
	
/*=========================
| Program exit sequence	. |
=========================*/
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
	open_rec (pcat,	 pcat_list, pcat_no_fields, "pcat_id_no2");
	open_rec (pcwo,	 pcwo_list, pcwo_no_fields, "pcwo_hhwo_hash");
	open_rec (prmr,	 prmr_list, prmr_no_fields, "prmr_hhmr_hash");
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
	abc_fclose (pcwo);
	if (AUDIT)
	{
		abc_fclose (prmr);
		abc_fclose (pcwc);
		abc_fclose (rgrs);
	}

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("date"))
	{
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("trail"))
	{
		switch (local_rec.trail [0])
		{
		case 'Y' :
			strcpy (local_rec.trail_desc, "Yes");
			FLD ("lp_no") = YES;
			if (prog_status != ENTRY)
			{
				/*-------------------------------------------------
				| Please ensure a printer number has been entered.|
				-------------------------------------------------*/
				print_mess ( ML(mlPcMess052) ); 
				sleep (sleepTime);
			}
			break;
		default :
			strcpy (local_rec.trail_desc, "No ");
			FLD ("lp_no") = NA;
			local_rec.lp_no = 0;
			DSP_FLD ("lp_no");
			break;
		}

		DSP_FLD ("trail_desc");

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
				/*------------------
				| Invalid Printer |
				------------------*/
				print_mess ( ML(mlStdMess020) );
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
ProcessRecords (
 void)
{
	/*------------------------------------
	| Step 1 : Read a valid pcat record. |
	|          Purges all records, upto  |
	|          and including date entered|
	|          only if records have been |
	|          updated (status = 'U').   |
	------------------------------------*/
	pcat_rec.date = 0L;
	pcat_rec.hhmr_hash = 0L;
	pcat_rec.hhwo_hash = 0L;
	pcat_rec.seq_no = 0;
	pcat_rec.hhrs_hash = 0L;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	while (!cc && pcat_rec.date <= local_rec.date)
	{
		if (pcat_rec.stat_flag [0] != 'U')
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		cc = find_hash (pcwo, &pcwo_rec, COMPARISON, "r", pcat_rec.hhwo_hash);
		if (cc)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		if (strcmp (pcwo_rec.br_no, comm_rec.test_no) != 0 &&
			strcmp (pcwo_rec.wh_no, comm_rec.tcc_no) != 0)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		pcat_rec2.date		= pcat_rec.date;
		pcat_rec2.hhmr_hash	= pcat_rec.hhmr_hash;
		pcat_rec2.hhwo_hash	= pcat_rec.hhwo_hash;
		pcat_rec2.seq_no	= pcat_rec.seq_no;
		pcat_rec2.hhrs_hash	= pcat_rec.hhrs_hash;

		/*---------------------
		| Delete pcat record. |
		---------------------*/
		cc = abc_delete (pcat);
		if (cc)
			file_err (cc, pcat, "DBDELETE");

		pcat_rec.date		= pcat_rec2.date;
		pcat_rec.hhmr_hash	= pcat_rec2.hhmr_hash;
		pcat_rec.hhwo_hash	= pcat_rec2.hhwo_hash;
		pcat_rec.seq_no		= pcat_rec2.seq_no;
		pcat_rec.hhrs_hash	= pcat_rec2.hhrs_hash;

		cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	}
}

void
ProcessReport (
 void)
{
	int	sortOpen = FALSE;
	int	bSortSave = FALSE;

	PrintHeading ();

	/*---------------------
	| initialising totals |
	---------------------*/
	br_tot.setup 	= br_tot.run 	= br_tot.clean 		= 0;
	date_tot.setup 	= date_tot.run 	= date_tot.clean 	= 0;
	emp_tot.setup 	= emp_tot.run 	= emp_tot.clean 	= 0;
	work_tot.setup 	= work_tot.run 	= work_tot.clean 	= 0;

	/*------------------------------------
	| Step 1 : Read a valid pcat record. |
	|          Purges all records, upto  |
	|          and including date entered|
	|          only if records have been |
	|          updated (status = 'U').   |
	------------------------------------*/
	pcat_rec.date = 0L;
	pcat_rec.hhmr_hash = 0L;
	pcat_rec.hhwo_hash = 0L;
	pcat_rec.seq_no = 0;
	pcat_rec.hhrs_hash = 0L;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	while (!cc && pcat_rec.date <= local_rec.date)
	{
		if (!sortOpen)
		{
			fsort = sort_open ("time");
			sortOpen = TRUE;
		}

		/*--------------------------------------
		| pcat record has already been printed |
		| or updated before.                   |
		--------------------------------------*/
		if (pcat_rec.stat_flag [0] != 'U')
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
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
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		cc = find_hash (pcwo, &pcwo_rec, COMPARISON, "r", pcat_rec.hhwo_hash);
		if (cc)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		if (strcmp (pcwo_rec.br_no, comm_rec.test_no) != 0 &&
			strcmp (pcwo_rec.wh_no, comm_rec.tcc_no) != 0)
		{
			cc = find_rec (pcat, &pcat_rec, NEXT, "u");
			continue;
		}

		PrintToSortFile ();
		bSortSave = TRUE;

		pcat_rec2.date		= pcat_rec.date;
		pcat_rec2.hhmr_hash	= pcat_rec.hhmr_hash;
		pcat_rec2.hhwo_hash	= pcat_rec.hhwo_hash;
		pcat_rec2.seq_no	= pcat_rec.seq_no;
		pcat_rec2.hhrs_hash	= pcat_rec.hhrs_hash;

		/*---------------------
		| Delete pcat record. |
		---------------------*/
		cc = abc_delete (pcat);
		if (cc)
			file_err (cc, pcat, "DBDELETE");

		pcat_rec.date		= pcat_rec2.date;
		pcat_rec.hhmr_hash	= pcat_rec2.hhmr_hash;
		pcat_rec.hhwo_hash	= pcat_rec2.hhwo_hash;
		pcat_rec.seq_no		= pcat_rec2.seq_no;
		pcat_rec.hhrs_hash	= pcat_rec2.hhrs_hash;

		cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	}

	if (bSortSave)
	{
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
			bSortSave = FALSE;
		}
	}

	/*------------------------------
	| nothing printed so tell user |
	------------------------------*/
	if (!printed)
	{
		if (AUDIT)
			fprintf (fout, ".E NO TIMESHEET'S FOUND FOR ANY EMPLOYEES \n");
	}
}

void
PrintToSortFile (
 void)
{
	char	tempSortString [200];

	sprintf (tempSortString, "%-10.10s %-40.40s %-8.8s %-7.7s %-10.10s %3d %11ld %11ld %11ld %11ld %11ld %11ld %-40.40s %-1.1s\n",
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
	sort_save (fsort, tempSortString);
}

void
PrintDetails (
 void)
{
	char	*sptr;

	firstTime = TRUE;
	sptr = sort_read (fsort);

	prev_rec.date = StringToDate (sptr);
	sprintf (prev_rec.emp_code, "%-8.8s", sptr + 52);
	sprintf (prev_rec.order_no, "%-7.7s", sptr + 61);
	while (sptr)
	{
		store_rec.date					= StringToDate (sptr);
		sprintf (store_rec.emp_name,	"%-40.40s",	sptr + 11);
		sprintf (store_rec.emp_code,	"%-8.8s",	sptr + 52);
		sprintf (store_rec.order_no,	"%-7.7s",	sptr + 61);
		sprintf (store_rec.batch_no,	"%-10.10s",	sptr + 69);
		store_rec.seq_no				= atoi (sptr + 80);
		store_rec.hhwc_hash				= atol (sptr + 84);
		store_rec.hhrs_hash				= atol (sptr + 96);
		store_rec.start_time			= atol (sptr + 108);
		store_rec.setup					= atol (sptr + 120);
		store_rec.run					= atol (sptr + 132);
		store_rec.clean					= atol (sptr + 144);
		sprintf (store_rec.comment,		"%-40.40s",	sptr + 156);
		sprintf (store_rec.status,		"%-1.1s",	sptr + 197);

		if (printed)
		{
			if (strcmp (prev_rec.order_no, store_rec.order_no) ||
				strcmp (prev_rec.emp_code, store_rec.emp_code) ||
				prev_rec.date != store_rec.date)
			{
				PrintWOTotal ();
				strcpy (prev_rec.order_no, store_rec.order_no);
				firstTime = TRUE;
			}

			if (strcmp (prev_rec.emp_code, store_rec.emp_code) ||
				prev_rec.date != store_rec.date)
			{
				PrintEmpTotal ();
				strcpy (prev_rec.emp_code, store_rec.emp_code);
				firstTime = TRUE;
			}

			if (prev_rec.date != store_rec.date)
			{
				PrintDateTotal ();
				prev_rec.date = store_rec.date;
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
		PrintWOTotal ();
		PrintEmpTotal ();
		PrintDateTotal ();
		PrintBranchTotal ();
	}
}

void
PrintLine (
 void)
{
	if (!printed)
	{
		PrintCoBr ();
		printed = TRUE;
	}

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
	fprintf (fout,
		"|%-3.3s|\n",
		(store_rec.status [0] == 'U') ? "Upd" : " ");

	/*------------------
	| calculate totals |
	------------------*/
	work_tot.setup += store_rec.setup;
	work_tot.run += store_rec.run;
	work_tot.clean += store_rec.clean;

	emp_tot.setup += store_rec.setup;
	emp_tot.run += store_rec.run;
	emp_tot.clean += store_rec.clean;

	date_tot.setup += store_rec.setup;
	date_tot.run += store_rec.run;
	date_tot.clean += store_rec.clean;

	br_tot.setup += store_rec.setup;
	br_tot.run += store_rec.run;
	br_tot.clean += store_rec.clean;
}

void
PrintCoBr (
 void)
{
	fprintf (fout, "| COMPANY : %-2.2s ", comm_rec.tco_no);
	fprintf (fout, "   BRANCH : %-2.2s      ", comm_rec.test_no);
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

void
PrintWOTotal (
 void)
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

	work_tot.setup =
		work_tot.run =
		work_tot.clean = 0;
}

void
PrintEmpTotal (
 void)
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

	emp_tot.setup =
		emp_tot.run =
		emp_tot.clean = 0;
}

void
PrintDateTotal (
 void)
{
	fprintf (fout, "|%-10.10s",		" ");
	fprintf (fout, " %-8.8s",		" ");
	fprintf (fout, " %-16.16s",		" ");
	fprintf (fout, " %-7.7s",		" ");
	fprintf (fout,
		"%-20.20s(%-8.8s)   ",
		"Date Total       ",
		DateToString (prev_rec.date));
	fprintf (fout, " %-7.7s",		" ");
	fprintf (fout, "|%s",			ttoa (date_tot.setup,	"NNNN:NN"));
	fprintf (fout, "|%s",			ttoa (date_tot.run,		"NNNN:NN"));
	fprintf (fout, "|%s",			ttoa (date_tot.clean,	"NNNN:NN"));
	fprintf (fout, "|%-40.40s",		" ");
	fprintf (fout, " %-3.3s|\n",	" ");

	date_tot.setup =
		date_tot.run =
		date_tot.clean = 0;
}

void
PrintBranchTotal (
 void)
{
	fprintf (fout, "|%-10.10s",		" ");
	fprintf (fout, " %-8.8s",		" ");
	fprintf (fout, " %-16.16s",		" ");
	fprintf (fout, " %-7.7s",		" ");
	fprintf (fout,
		"%-20.20s(%-2.2s)         ",
		"Branch Total     ",
		comm_rec.test_no);
	fprintf (fout, " %-7.7s",		" ");
	fprintf (fout, "|%s",			ttoa (br_tot.setup,	"NNNN:NN"));
	fprintf (fout, "|%s",			ttoa (br_tot.run,	"NNNN:NN"));
	fprintf (fout, "|%s",			ttoa (br_tot.clean,	"NNNN:NN"));
	fprintf (fout, "|%-40.40s",		" ");
	fprintf (fout, " %-3.3s|\n",	" ");

	br_tot.setup =
		br_tot.run =
		br_tot.clean = 0;
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
		
		/*-----------------
		| TimeSheet Purge |
		-----------------*/
		sprintf (err_str, " %s ", ML(mlPcMess089) );
		rv_pr ( err_str, 30, 0, 1);

		move (0, 1);
		line (80);

		box (0, 3, 80, 5);
		move (1, 5);
		line (79);
		move (1, 7);
		line (79);

		move (0, 20);
		line (80);

		print_at(21,0,  ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_short);
		print_at(21,29, ML(mlStdMess039),comm_rec.test_no,comm_rec.test_short);
		print_at(21,57, ML(mlStdMess099),comm_rec.tcc_no, comm_rec.tcc_short);

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
	dsp_screen (" Printing TimeSheet Purge Audit Trial ",
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
	fprintf (fout, ".START%s <%s>\n", local_rec.systemDate, PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lp_no);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L156\n");
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".E TIMESHEET PURGE AUDIT TRIAL \n");

	fprintf (fout, ".E COMPANY NAME : %s \n", clip (comm_rec.tco_name));
	fprintf (fout, ".E BRANCH NAME : %s \n", clip (comm_rec.test_name));
	fprintf (fout, ".E WAREHOUSE NAME : %s \n", clip (comm_rec.tcc_name));

	fprintf (fout, ".E PURGE DATE : %s \n", DateToString (local_rec.date));

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

