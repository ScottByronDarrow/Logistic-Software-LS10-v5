/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_emp_anal.c  )                                 |
|  Program Desc  : ( Contract Labour Analysis Report              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmcd, cmcm, cmem, cmeq, cmhr, cmjt, cmts    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : (05/04/93)       |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (10/09/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (10/09/97)    : Updated for Multilingual Conversion and            |
|                                                                     |
| $Log: emp_anal.c,v $
| Revision 5.3  2002/07/17 09:56:59  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:57:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:11  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:02  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:08  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:26  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.14  1999/12/06 01:32:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/17 06:39:09  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/11/08 04:35:38  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.11  1999/10/20 01:40:30  nz
| Updated for remainder of date routines
|
| Revision 1.10  1999/10/01 07:48:20  scott
| Updated for standard function calls.
|
| Revision 1.9  1999/09/29 10:10:18  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:26:15  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/17 04:40:09  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.6  1999/09/16 04:44:41  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/14 07:34:20  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: emp_anal.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_emp_anal/emp_anal.c,v 5.3 2002/07/17 09:56:59 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_cm_mess.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2
#define	PROC_CNT (local_rec.rep_type[0] == 'C')
#define	PROC_EMP (local_rec.rep_type[0] == 'E')
#define	INTERNAL (local_rec.internal[0] == 'Y')

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

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


	/*===========================================+
	 | Contract Management Costheads Master File |
	 +===========================================*/
#define	CMCM_NO_FIELDS	3

	struct dbview	cmcm_list [CMCM_NO_FIELDS] =
	{
		{"cmcm_ch_code"},
		{"cmcm_desc"},
		{"cmcm_hhcm_hash"},
	};

	struct tag_cmcmRecord
	{
		char	ch_code [5];
		char	desc [41];
		long	hhcm_hash;
	}	cmcm_rec;


	/*==========================================+
	 | Contract Management Employee Master File |
	 +==========================================*/
#define	CMEM_NO_FIELDS	4

	struct dbview	cmem_list [CMEM_NO_FIELDS] =
	{
		{"cmem_co_no"},
		{"cmem_emp_no"},
		{"cmem_emp_name"},
		{"cmem_hhem_hash"},
	};

	struct tag_cmemRecord
	{
		char	co_no [3];
		char	emp_no [11];
		char	emp_name [41];
		long	hhem_hash;
	} cmem_rec, cmem2_rec;

	/*===========================================+
	 | Contract Management Equipment Master File |
	 +===========================================*/
#define	CMEQ_NO_FIELDS	3

	struct dbview	cmeq_list [CMEQ_NO_FIELDS] =
	{
		{"cmeq_eq_name"},
		{"cmeq_hheq_hash"},
		{"cmeq_desc"},
	};

	struct tag_cmeqRecord
	{
		char	eq_name [11];
		long	hheq_hash;
		char	desc [41];
	}	cmeq_rec;

	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	7

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_hhjt_hash"},
		{"cmhr_internal"},
		{"cmhr_cus_ref"},
	};

	struct tag_cmhrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	cont_no [7];
		long	hhhr_hash;
		long	hhjt_hash;
		char	internal [2];
		char	cus_ref[21];
	} cmhr_rec;

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
	} cmjt_rec, cmjt2_rec;

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

	struct 
	{
		float	units;
		float 	ord;
		float	hlf;
		float	dbl;
		double	lab_cost;
		double	oh_cost;
		double	total;
	} total;

	char	*comm  = "comm",
	    	*cmcd  = "cmcd",
	    	*cmcm  = "cmcm",
	    	*cmem  = "cmem",
	    	*cmem2 = "cmem2",
	    	*cmeq  = "cmeq",
	    	*cmhr  = "cmhr",
	    	*cmjt  = "cmjt",
	    	*cmjt2 = "cmjt2",
	    	*cmts  = "cmts",
			*data  = "data",
			*DBFIND = "DBFIND";

	int		cm_auto_con;
	int		pipe_open = FALSE;

	FILE	*fsort,
			*fout;

	char	branchNo[3],
			*sptr;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	int	lpno;
	long	lsystemDate;
	long	st_date;
	long	end_date;
	char	rep_type[2];
	char	rep_desc[9];
	char	internal[2];
	char	int_desc[4];
	char	systemDate[11];
	char	st_cont[7];
	char	st_desc[71];
	char	end_cont[7];
	char	end_desc[71];
	char	st_jtype[5];
	char	st_jtype_desc[31];
	char	end_jtype[5];
	char	end_jtype_desc[31];
	char	st_emp[11];
	char	st_emp_desc[41];
	char	end_emp[11];
	char	end_emp_desc[41];
	char	onight[2];
	char	onight_desc[4];
	char	back[2];
	char	back_desc[4];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "rep_type", 	3, 16, CHARTYPE,
		"U", "          ",
		" ", "C", " Report Type    :", "C)ontract, E)mployee, - Default is Contract ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.rep_type},
	{1, LIN, "rep_desc", 	3, 16, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "C", " Report Type    :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rep_desc},
	{1, LIN, "st_date", 	4, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", " Start Date     :", "Enter Start Transaction Date. ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.st_date},
	{1, LIN, "end_date", 	5, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " End Date       :", "Enter End Transaction Date. Default = Today ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.end_date},
	{1, LIN, "st_cont", 	7, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Start Contract :", "Enter Start Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.st_cont},
	{1, LIN, "st_desc", 	7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "end_cont", 	8, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " End  Contract  :", "Enter End Contract Number . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.end_cont},
	{1, LIN, "end_desc", 	8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_desc},
	{1, LIN, "st_jtype", 	10, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", " Start Job Type :", "Enter Start Job Type . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.st_jtype},
	{1, LIN, "st_jtype_desc",10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_jtype_desc},
	{1, LIN, "end_jtype", 	11, 16, CHARTYPE,
		"UUUU", "          ",
		" ", " ", " End Job Type   :", "Enter End Job Type . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.end_jtype},
	{1, LIN, "end_jtype_desc",11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_jtype_desc},
	{1, LIN, "st_emp", 	13, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " Start Employee :", "Enter Start Employee Code . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.st_emp},
	{1, LIN, "st_emp_desc",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_emp_desc},
	{1, LIN, "end_emp", 	14, 16, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", " End Employee   :", "Enter End Employee Code . ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.end_emp},
	{1, LIN, "end_emp_desc",14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_emp_desc},
	{1, LIN, "internal", 	 16, 16, CHARTYPE,
		"U", "          ",
		" ", "Y", " Inc Internal   :", "Enter Whether To Include Internal Contracts ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.internal},
	{1, LIN, "int_desc",     16, 16, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.int_desc},
	{1, LIN, "lpno", 	 18, 16, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No     :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{1, LIN, "back", 	 18, 50, CHARTYPE,
		"U", "          ",
		" ", "N", " Background     :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "back_desc",     18, 50, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight", 	 18, 80, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight      :", "Enter Y(es) or N(o). ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc",  18, 80, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*------------------------
| Function Declarations. |
------------------------*/
int		main			(int argc, char * argv []);
void	OpenDB			(void);
void	CloseDB		(void);
void	shutdown_prog	(void);
void	run_prog		(char * prog_name, char * prog_desc);
int		spec_valid		(int field);
void	SrchCmhr		(char * key_val);
void	SrchCmjt		(char * key_val);
void	process			(void);
void	head_output		(void);
void	print_data		(void);
int		heading			(int scn);
void	srch_cmem		(char * key_val);
void	proc_cmts		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	if (argc != 2 && argc != 12)
	{
	/*	printf("\007Usage : %s <description>\n", argv[0]);
		printf(" OR   :    	  <lpno>\n", argv[0]);
		printf("                  <Report Type - C(ontract)>\n");
		printf("                               - E(mployee)>\n");
		printf("                  <Include Internal (Y/N)>\n");
		printf("                  <Start Date>\n");
		printf("                  <End   Date>\n");
		printf("                  <Start Contract>\n");
		printf("                  <End   Contract>\n");
		printf("                  <Start Job Type>\n");
		printf("                  <End   Job Type>\n");
		printf("                  <Start Employee>\n");
		printf("                  <End   Employee>\n");
	*/
		clear();
		print_at(0,0, ML(mlCmMess700) , argv[0]);
		print_at(1,0, ML(mlCmMess701) , argv[0]);
		print_at(2,0, ML(mlCmMess702));
		print_at(3,0, ML(mlCmMess703));
		print_at(4,0, ML(mlCmMess704));
		print_at(5,0, ML(mlCmMess705));
		print_at(6,0, ML(mlCmMess706));
		print_at(7,0, ML(mlCmMess707));
		print_at(8,0, ML(mlCmMess708));
		print_at(9,0, ML(mlCmMess709));
		print_at(10,0,ML(mlCmMess710));
		print_at(11,0,ML(mlCmMess711));
		print_at(12,0,ML(mlCmMess712));

		return (argc);
	}
	
	if (argc == 12)
	{
		local_rec.lpno = atoi(argv[1]);
		sprintf(local_rec.rep_type, "%-1.1s", argv[2]);
		sprintf(local_rec.internal, "%-1.1s", argv[3]);
		local_rec.st_date = StringToDate ( argv[4] );
		local_rec.end_date = StringToDate ( argv[5] );
		sprintf(local_rec.st_cont, "%-6.6s", argv[6] );
		sprintf(local_rec.end_cont, "%-6.6s", argv[7] );
		sprintf(local_rec.st_jtype, "%-4.4s", argv[8] );
		sprintf(local_rec.end_jtype, "%-4.4s", argv[9] );
		sprintf(local_rec.st_emp, "%-10.10s", argv[10] );
		sprintf(local_rec.end_emp, "%-10.10s", argv[11] );
	}

	/*--------------------
	| Check environment. |
	--------------------*/
	sptr = chk_env("CM_AUTO_CON");
	cm_auto_con = (sptr == (char *)0) ? COMPANY : atoi(sptr);

	OpenDB();
	strcpy(branchNo, (cm_auto_con == COMPANY) ? " 0" : comm_rec.test_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	if (argc == 12)
	{
		process();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*-----------------
	| Prepare screen. |
	-----------------*/
	SETUP_SCR(vars);

	init_scr();
	set_tty(); 
	set_masks();
	init_vars(1);

	while (!prog_exit)
	{
		/*=====================
		| Reset control flags |
		=====================*/
   		entry_exit = 0;
   		edit_exit = 0;
   		prog_exit = 0;
   		restart = 0;
		search_ok = TRUE;

		init_vars(1);	
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		run_prog (argv[0], argv[1]);
		prog_exit = 1;

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen(data);


	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	abc_alias ( cmjt2, cmjt );
	abc_alias ( cmem2, cmem );
	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmem,  cmem_list, CMEM_NO_FIELDS, "cmem_id_no");
	open_rec (cmem2, cmem_list, CMEM_NO_FIELDS, "cmem_hhem_hash");
	open_rec (cmeq,  cmeq_list, CMEQ_NO_FIELDS, "cmeq_hheq_hash");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmjt,  cmjt_list, CMJT_NO_FIELDS, "cmjt_id_no");
	open_rec (cmjt2, cmjt_list, CMJT_NO_FIELDS, "cmjt_hhjt_hash");
	open_rec (cmts,  cmts_list, CMTS_NO_FIELDS, "cmts_hhhr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmem);
	abc_fclose (cmeq);
	abc_fclose (cmhr);
	abc_fclose (cmjt);
	abc_fclose (cmts);
	abc_dbclose(data);
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

void
run_prog (
 char *	prog_name,
 char *	prog_desc)
{
	char	lpstr [10];
	char	st_date[11];
	char	end_date[11];

	if (local_rec.onight [0] == 'Y' || local_rec.back [0] == 'Y')
	{
		shutdown_prog ();
	}

	sprintf (lpstr, "%d", local_rec.lpno);

	sprintf (st_date, "%-10.10s", DateToString(local_rec.st_date));
	sprintf (end_date,"%-10.10s", DateToString(local_rec.end_date));

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					lpstr,
					local_rec.rep_type,
					local_rec.internal,
					st_date,
					end_date,
					local_rec.st_cont,
					local_rec.end_cont,
					local_rec.st_jtype,
					local_rec.end_jtype,
					local_rec.st_emp,
					local_rec.end_emp,
					prog_desc,
					(char *) 0);
		}
		else
			return;
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		if (fork() == 0)
		{
			execlp (prog_name,
					prog_name,
					lpstr,
					local_rec.rep_type,
					local_rec.internal,
					st_date,
					end_date,
					local_rec.st_cont,
					local_rec.end_cont,
					local_rec.st_jtype,
					local_rec.end_jtype,
					local_rec.st_emp,
					local_rec.end_emp,
					(char *) 0);
		}
		else
			return;
	}
	else
		process();
}

int
spec_valid (
 int	field)
{
	if ( LCHECK ("internal") )
	{
		if ( local_rec.internal[0] == 'Y' )
			sprintf ( local_rec.int_desc, "%-3.3s", "Yes" );
		else
			sprintf ( local_rec.int_desc, "%-3.3s", "No" );

		DSP_FLD ("int_desc" );
	}

	if ( LCHECK ("rep_type") )
	{
		switch (local_rec.rep_type[0]) 
		{
		case 'E' :
			sprintf ( local_rec.rep_desc, "%-8.8s", "Employee" );	
			DSP_FLD ("rep_desc");
			break;
		case 'C' :
			sprintf ( local_rec.rep_desc, "%-8.8s", "Contract" );	
			DSP_FLD ("rep_desc");
			break;
		default	:
			/*print_mess ("\007 Choice May Only Be E)mployee Or C)ontract ");*/
			print_mess (ML(mlCmMess035));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Contract Range. |
	--------------------------*/
	if (LCHECK("st_cont"))
	{
		if (FLD("st_cont") == ND)
			return(0);

		if (dflt_used)
		{
			strcpy(local_rec.st_cont, "      ");
			sprintf(local_rec.st_desc,"%-70.70s","First Contract");
			DSP_FLD ("st_cont");
			DSP_FLD ("st_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr(temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num(local_rec.st_cont);
		strcpy(cmhr_rec.co_no, comm_rec.tco_no);
		strcpy(cmhr_rec.br_no, branchNo);
		sprintf(cmhr_rec.cont_no, "%-6.6s", local_rec.st_cont);
		cc = find_rec(cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			/*print_mess("\007 Contract Not Found ");*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*--------------------------------
		| Check against end contract no. |
		--------------------------------*/
		if (prog_status != ENTRY && 
		    strcmp(local_rec.st_cont, local_rec.end_cont) > 0)
		{
			/*print_mess("\007 Start Contract Number Should Be Less Than End Contract Number ");*/
			print_mess(ML(mlStdMess017));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*--------------------------------
		| Lookup first description line. |
		--------------------------------*/
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy(cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec(cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf(cmcd_rec.text, 
				"%-70.70s", 
				"Description Not Found. ");
		}
		sprintf(local_rec.st_desc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("st_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK("end_cont"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_cont, "~~~~~~");
			sprintf(local_rec.end_desc,"%-70.70s","Last Contract");
			DSP_FLD ("end_cont");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCmhr(temp_str);
			return (EXIT_SUCCESS);
		}

		pad_num(local_rec.end_cont);
		strcpy(cmhr_rec.co_no, comm_rec.tco_no);
		strcpy(cmhr_rec.br_no, branchNo);
		sprintf(cmhr_rec.cont_no, "%-6.6s", local_rec.end_cont);
		cc = find_rec(cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			/*print_mess("\007 Contract Not Found ");*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}
		/*----------------------------------
		| Check against start contract no. |
		----------------------------------*/
		if (strcmp(local_rec.st_cont, local_rec.end_cont) > 0)
		{
			/*print_mess("\007 Start Contract Number Should Be Less Than End Contract Number ");*/
			print_mess(ML(mlStdMess017));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*--------------------------------
		| Lookup first description line. |
		--------------------------------*/
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy(cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec(cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf(cmcd_rec.text, 
				"%-70.70s", 
				"Description Not Found. ");
		}
		sprintf(local_rec.end_desc, "%-70.70s", cmcd_rec.text);
		DSP_FLD ("end_desc");

		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Job Type Range. |
	--------------------------*/
	if (LCHECK("st_jtype"))
	{
		if (FLD("st_jtype") == ND)
			return(0);

		if (dflt_used)
		{
			strcpy(local_rec.st_jtype, "    ");
			sprintf(local_rec.st_jtype_desc, "%-30.30s", "First Job Type");
			DSP_FLD("st_jtype");
			DSP_FLD("st_jtype_desc");
			return(0);
		}

	    	if (SRCH_KEY)
	    	{
			SrchCmjt(temp_str);
			return(0);
	    	}

		strcpy(cmjt_rec.co_no, comm_rec.tco_no);
		strcpy(cmjt_rec.br_no, comm_rec.test_no);
		sprintf(cmjt_rec.job_type, "%-4.4s", local_rec.st_jtype);
		cc = find_rec(cmjt, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Job Type Not Found. ");*/
			print_mess(ML(mlStdMess088));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_jtype_desc, "%-30.30s", cmjt_rec.desc);
		DSP_FLD("st_jtype_desc");

	    	return(0);
	}

	if (LCHECK("end_jtype"))
	{
		if (FLD("end_jtype") == ND)
			return(0);

		if (dflt_used)
		{
			strcpy(local_rec.end_jtype, "~~~~");
			sprintf(local_rec.end_jtype_desc, "%-30.30s", "Last Job Type");
			DSP_FLD("end_jtype");
			DSP_FLD("end_jtype_desc");
			return(0);
		}

	    	if (SRCH_KEY)
	    	{
			SrchCmjt(temp_str);
			return(0);
	    	}

		strcpy(cmjt_rec.co_no, comm_rec.tco_no);
		strcpy(cmjt_rec.br_no, comm_rec.test_no);
		sprintf(cmjt_rec.job_type, "%-4.4s", local_rec.end_jtype);
		cc = find_rec(cmjt, &cmjt_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Job Type Not Found. ");*/
			print_mess(ML(mlStdMess088));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.end_jtype_desc, "%-30.30s", cmjt_rec.desc);
		DSP_FLD("end_jtype_desc");

	    	return(0);
	}

	/*--------------------------
	| Validate Issue To Range. |
	--------------------------*/
	if (LCHECK("st_emp"))
	{
		if (FLD("st_emp") == ND)
			return(0);

		if (dflt_used)
		{
			strcpy(local_rec.st_emp, "          ");
			sprintf(local_rec.st_emp_desc, "%-40.40s", "First Employee ");
			DSP_FLD("st_emp");
			DSP_FLD("st_emp_desc");
			return(0);
		}

	    	if (SRCH_KEY)
	    	{
			srch_cmem(temp_str);
			return(0);
	    	}

		strcpy(cmem_rec.co_no, comm_rec.tco_no);
		sprintf(cmem_rec.emp_no, "%-10.10s", local_rec.st_emp);
		cc = find_rec(cmem, &cmem_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Employee Code Not Found. ");*/
			print_mess(ML(mlStdMess053));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.st_emp_desc, "%-40.40s",cmem_rec.emp_name);
		DSP_FLD("st_emp_desc");

	    	return(0);
	}

	if (LCHECK("end_emp"))
	{
		if (FLD("end_emp") == ND)
			return(0);

		if (dflt_used)
		{
			strcpy(local_rec.end_emp, "~~~~~~~~~~");
			sprintf(local_rec.end_emp_desc, "%-40.40s", "Last Employee ");
			DSP_FLD("end_emp");
			DSP_FLD("end_emp_desc");
			return(0);
		}

	    	if (SRCH_KEY)
	    	{
			srch_cmem(temp_str);
			return(0);
	    	}

		strcpy(cmem_rec.co_no, comm_rec.tco_no);
		sprintf(cmem_rec.emp_no, "%-10.10s", local_rec.end_emp);
		cc = find_rec(cmem, &cmem_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Employee Code Not Found. ");*/
			print_mess(ML(mlStdMess053));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.end_emp_desc,"%-40.40s",cmem_rec.emp_name);
		DSP_FLD("end_emp_desc");

	    	return(0);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp(local_rec.lpno))
		{
			/*print_mess ("\007 Invalid Printer Number ");*/
			print_mess (ML(mlStdMess020));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK("back"))
	{
		if (local_rec.back[0] == 'Y')
			strcpy(local_rec.back_desc, "Yes");
		else
			strcpy(local_rec.back_desc, "No ");
	
		DSP_FLD("back_desc");

		return(0);
	}

	if (LCHECK("onight"))
	{
		if (local_rec.onight[0] == 'Y')
			strcpy(local_rec.onight_desc, "Yes");
		else
			strcpy(local_rec.onight_desc, "No ");
	
		DSP_FLD("onight_desc");

		return(0);
	}


	return(0);
}

void
SrchCmhr (
 char *	key_val)
{
	work_open ();
	save_rec("#Cont. No.", "#Customer Order No.");

	strcpy(cmhr_rec.co_no, comm_rec.tco_no);
	strcpy(cmhr_rec.br_no, branchNo);
	sprintf(cmhr_rec.cont_no, "%-6.6s", key_val);
	cc = find_rec(cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmhr_rec.co_no, comm_rec.tco_no) &&
	       !strcmp(cmhr_rec.br_no, branchNo) &&
	       !strncmp(cmhr_rec.cont_no, key_val, strlen(key_val)))
	{
		cc = save_rec(cmhr_rec.cont_no, cmhr_rec.cus_ref);
		if (cc)
			break;

		cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy ( cmhr_rec.co_no, comm_rec.tco_no );
	strcpy ( cmhr_rec.br_no, branchNo );
	sprintf( cmhr_rec.cont_no, "%6.6s", temp_str );

	cc = find_rec ( cmhr, &cmhr_rec, COMPARISON, "r" );
	if (cc)
		file_err (cc, cmhr, DBFIND );
}

void
SrchCmjt (
 char *	key_val)
{
	work_open();
	save_rec("#Job Type", "#Job Type Description");

	strcpy(cmjt_rec.co_no, comm_rec.tco_no);
	sprintf(cmjt_rec.job_type, "%-4.4s", key_val);
	cc = find_rec(cmjt, &cmjt_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmjt_rec.co_no, comm_rec.tco_no) &&
	       !strncmp(cmjt_rec.job_type, key_val, strlen(key_val)))
	{
		cc = save_rec(cmjt_rec.job_type, cmjt_rec.desc);
		if (cc)
			break;

		cc = find_rec(cmjt, &cmjt_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cmjt_rec.co_no, comm_rec.tco_no);
	sprintf(cmjt_rec.job_type, "%-4.4s", temp_str);
	cc = find_rec(cmjt, &cmjt_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, cmjt, DBFIND);
}

void
process (void)
{
	int	first_time = TRUE;

	dsp_screen("Processing Contracts", comm_rec.tco_no, comm_rec.tco_name);
	
	/*------------------------
	| Read contract records. |
	------------------------*/
	strcpy(cmhr_rec.co_no, comm_rec.tco_no);
	strcpy(cmhr_rec.br_no, branchNo);
	sprintf(cmhr_rec.cont_no, "%-6.6s", local_rec.st_cont);
	cc = find_rec(cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp(cmhr_rec.co_no, comm_rec.tco_no) &&
	       !strcmp(cmhr_rec.br_no, branchNo))
	{
		/*----------------------
		| Internal OR External |
		----------------------*/
		if ( !INTERNAL && cmhr_rec.internal[0] == 'Y' )
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		/*---------------------------
		| Check range for contract. |
		---------------------------*/
		if (strcmp(cmhr_rec.cont_no,local_rec.end_cont) > 0)
			break;

		/*------------------------------------
		| Lookup job type code for contract. |
		------------------------------------*/
		cc = find_hash(cmjt2, &cmjt_rec, COMPARISON, "r", 
			       cmhr_rec.hhjt_hash);
		if (cc)
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}
		/*---------------------------
		| Valid for job type range. |
		---------------------------*/
		if (strcmp(cmjt_rec.job_type, local_rec.st_jtype) < 0 ||
		     strcmp(cmjt_rec.job_type, local_rec.end_jtype) > 0)
		{
			cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		if ( first_time )
		{
			first_time = FALSE;
			fsort = sort_open ("lab_anal");
		}

		proc_cmts ();

		cc = find_rec(cmhr, &cmhr_rec, NEXT, "r");
	}

	if ( !first_time )
	{
		fsort = sort_sort ( fsort, "lab_anal" );
		print_data ();
		sort_delete ( fsort, "lab_anal" );
	}

	if (pipe_open)
	{
		fprintf(fout, ".EOF\n");
		pclose(fout);
		pipe_open = FALSE;
	}
}

void
head_output (void)
{
	if ((fout = popen("pformat", "w")) == (FILE *)NULL)
		sys_err("Error in pformat During (POPEN)",errno, PNAME);

	sprintf(err_str, "%-10.10s<%s>", local_rec.systemDate, PNAME);

	fprintf(fout, ".START%s\n", clip (err_str));
	fprintf(fout, ".LP%d\n", local_rec.lpno);
	fprintf(fout, ".12\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, ".L153\n");

	fprintf(fout, ".ELabour Analysis By %s \n", local_rec.rep_desc );
	fprintf(fout,".ECompany : %s - %s\n",comm_rec.tco_no,comm_rec.tco_name);
	fprintf(fout, ".EAs At %s \n", SystemTime () );

	sprintf( err_str, "%10.10s", DateToString ( local_rec.st_date ));

	fprintf(fout, ".EFor The Period %s to %s\n", err_str, 
						DateToString (local_rec.end_date));
	fprintf(fout, ".B1\n");

	fprintf(fout, "========");
	fprintf(fout, "=========================================");
	fprintf(fout, "===========");
	fprintf(fout, "=========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "=================\n");

	fprintf(fout, ".R========");
	fprintf(fout, "=========================================");
	fprintf(fout, "===========");
	fprintf(fout, "=========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "========");
	fprintf(fout, "============");
	fprintf(fout, "============");
	fprintf(fout, "=================\n");

	if ( PROC_EMP )
		fprintf(fout, "|      E M P L O Y E E   N A M E         ");
	else
		fprintf(fout, "| CONT. ");

	if ( PROC_EMP )
		fprintf(fout, "| CONT. ");
	else
		fprintf(fout, "|      E M P L O Y E E   N A M E         ");

	fprintf(fout, "|   DATE   ");
	fprintf(fout, "|   PLANT  ");
	fprintf(fout, "| CSTHD ");
	fprintf(fout, "| UNITS ");
	fprintf(fout, "| 1.0   ");
	fprintf(fout, "| 1.5   ");
	fprintf(fout, "| 2.0   ");
	fprintf(fout, "|LABOUR COST");
	fprintf(fout, "|O/HEAD COST");
	fprintf(fout, "|    TOTAL    |\n");

	if ( PROC_EMP )
		fprintf(fout, "|                                        ");
	else
		fprintf(fout, "|   NO. ");

	if ( PROC_EMP )
		fprintf(fout, "|   NO. ");
	else
		fprintf(fout, "|                                        ");

	fprintf(fout, "|          ");
	fprintf(fout, "|   CODE   ");
	fprintf(fout, "| CODE  ");
	fprintf(fout, "|       ");
	fprintf(fout, "| TIME  ");
	fprintf(fout, "| TIME  ");
	fprintf(fout, "| TIME  ");
	fprintf(fout, "|           ");
	fprintf(fout, "|           ");
	fprintf(fout, "|    COST     |\n");

	if ( PROC_EMP )
	{
		fprintf(fout, "|----------------------------------------");
		fprintf(fout, "+-------");
	}
	else
	{
		fprintf(fout, "|-------");
		fprintf(fout, "+----------------------------------------");
	}

	fprintf(fout, "+----------");
	fprintf(fout, "+----------");
	fprintf(fout, "+-------");
	fprintf(fout, "+-------");
	fprintf(fout, "+-------");
	fprintf(fout, "+-------");
	fprintf(fout, "+-------");
	fprintf(fout, "+-----------");
	fprintf(fout, "+-----------");
	fprintf(fout, "+-------------|\n");

	pipe_open = TRUE;
}

void
print_data (void)
{
	if ( pipe_open == FALSE )
		head_output ();

	sptr = sort_read ( fsort );

	while ( sptr )
	{

		fprintf ( fout, "%s\n", sptr );

		total.units    +=  (float) atof( sptr + 78 );
		total.ord      +=  (float) atof( sptr + 86 );
		total.hlf      +=  (float) atof( sptr + 94 );
		total.dbl      +=  (float) atof( sptr + 102 );
		total.lab_cost +=  (float) atof( sptr + 110 );
		total.oh_cost  +=  (float) atof( sptr + 122 );

		if ( PROC_CNT )
		{
			sprintf ( err_str, "%-6.6s", sptr + 1 );
			dsp_process ( "Contract ", err_str );
		}
		else
		{
			sprintf ( err_str, "%s", sptr + 1 );
			dsp_process ( "Employee ", err_str );
		}
		sptr = sort_read ( fsort );
	}
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

		swide ();
		move (0, 1);
		line (132);
		centre_at (0, 132, ML(mlCmMess036)); 

		box (0, 2, 132, 16);
		move(1, 6);
		line(131);
		move(1, 9);
		line(131);
		move(1, 12);
		line(131);
		move(1, 15);
		line(131);
		move(1, 17);
		line(131);

		line_cnt = 0;
		scn_write (scn);
	}

	move(0, 21);
	line(131);

	strcpy(err_str, ML(mlStdMess038)); 
	print_at(22,0, err_str , comm_rec.tco_no, comm_rec.tco_name);

	return (EXIT_SUCCESS);
}

/*----------------------------------
| Search for Employee master file. |
----------------------------------*/
void
srch_cmem (
 char *	key_val)
{
	work_open();
	save_rec("#Employee", "#Employee Name");

	strcpy(cmem_rec.co_no, comm_rec.tco_no);
	sprintf(cmem_rec.emp_no, "%-10.10s", key_val);
	cc = find_rec(cmem, &cmem_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmem_rec.co_no, comm_rec.tco_no) &&
	       !strncmp(cmem_rec.emp_no, key_val, strlen(key_val)))
	{
		cc = save_rec(cmem_rec.emp_no, cmem_rec.emp_name);
		if (cc)
			break;

		cc = find_rec(cmem, &cmem_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cmem_rec.co_no, comm_rec.tco_no);
	sprintf(cmem_rec.emp_no, "%-10.10s", temp_str);
	cc = find_rec(cmem, &cmem_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, cmem, DBFIND);
}

void
proc_cmts (void)
{
	char	temp [1024];
	float	hours;

	cc = find_hash ( cmts, &cmts_rec, EQUAL, "r", cmhr_rec.hhhr_hash );
	while ( !cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash )
	{
		if ( cmts_rec.date > local_rec.end_date || 
			cmts_rec.date < local_rec.st_date )
		{
			cc = find_hash(cmts,&cmts_rec,NEXT,"r",cmhr_rec.hhhr_hash);
			continue;
		}

		cc = find_hash (cmem2,&cmem2_rec,EQUAL,"r",cmts_rec.hhem_hash);
		if ( cc )
			file_err ( cc, cmem2, DBFIND );

		if ( strcmp( cmem2_rec.emp_no, local_rec.st_emp ) < 0 ||
			strcmp( cmem2_rec.emp_no, local_rec.end_emp ) > 0 )
		{
			cc = find_hash(cmts,&cmts_rec,NEXT,"r",cmhr_rec.hhhr_hash);
			continue;
		}

		if ( cmts_rec.hheq_hash != 0L )
		{
			cc = find_hash(cmeq,&cmeq_rec,EQUAL,"r",
							cmts_rec.hheq_hash);
			if ( cc )
				file_err ( cc, cmeq, DBFIND );
		}
		else
			sprintf ( cmeq_rec.eq_name, "%-10.10s", " " );

		cc = find_hash(cmcm, &cmcm_rec, EQUAL, "r", cmts_rec.hhcm_hash);
		if ( cc )
			file_err ( cc, cmcm, DBFIND );

		hours = cmts_rec.units + cmts_rec.time_ord + 
			cmts_rec.time_hlf + cmts_rec.time_dbl;
			
		if ( PROC_CNT )
		{
			sprintf (temp, "|%-6.6s |%-40.40s|%-10.10s|%-10.10s|  %-4.4s | %5.2f | %5.2f | %5.2f | %5.2f |%10.2f |%10.2f |%12.2f |\n",
				cmhr_rec.cont_no,
				cmem2_rec.emp_name,
				DateToString(cmts_rec.date),
				cmeq_rec.eq_name,
				cmcm_rec.ch_code,
				cmts_rec.units,
				cmts_rec.time_ord,
				cmts_rec.time_hlf,
				cmts_rec.time_dbl,
				DOLLARS(cmts_rec.lab_cost * hours),
				DOLLARS(cmts_rec.oh_cost  * hours),
				DOLLARS((cmts_rec.lab_cost + cmts_rec.oh_cost))
							* hours);

			sort_save (fsort, temp);
		}

		if ( PROC_EMP )
		{
			sprintf (temp,  "|%-40.40s|%-6.6s |%-10.10s|%-10.10s|  %-4.4s | %5.2f | %5.2f | %5.2f | %5.2f |%10.2f |%10.2f |%12.2f |\n",
				cmem2_rec.emp_name,
				cmhr_rec.cont_no,
				DateToString(cmts_rec.date),
				cmeq_rec.eq_name,
				cmcm_rec.ch_code,
				cmts_rec.units,
				cmts_rec.time_ord,
				cmts_rec.time_hlf,
				cmts_rec.time_dbl,
				DOLLARS(cmts_rec.lab_cost * hours),
				DOLLARS(cmts_rec.oh_cost  * hours),
				DOLLARS((cmts_rec.lab_cost + cmts_rec.oh_cost))
							* hours);

			sort_save (fsort, temp);
		}

		cc = find_hash(cmts,&cmts_rec,NEXT,"r",cmhr_rec.hhhr_hash);
	}
}
