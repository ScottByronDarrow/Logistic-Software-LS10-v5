/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_reqrprt.i.c )                                 |
|  Program Desc  : ( Requistion Docket Input For Reprint.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (01/03/93)       |
|---------------------------------------------------------------------|
|  Date Modified : (28/04/93)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (11/09/97)      | Modified  by : Rowena S Maandig  |
|  Date Modified : (09/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (28/04/93) - EGC 8917 Incorrect error message.     |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (11/09/97)    : Incorporate multilingual conversion.               |
|  (09/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: reqrprt.i.c,v $
| Revision 5.5  2002/07/24 08:38:42  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:12:22  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/03 04:21:42  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 08:57:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:24  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:43  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:32:40  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/17 06:39:21  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.7  1999/11/08 04:35:41  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.6  1999/09/29 10:10:29  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 04:40:14  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.4  1999/09/16 04:44:44  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/14 07:35:14  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: reqrprt.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_reqrprt.i/reqrprt.i.c,v 5.5 2002/07/24 08:38:42 scott Exp $";

#define	MAXLINES	100
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

FILE *	pout;

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*========================================
	| cmhr - Contract Management Header File |
	========================================*/
	struct dbview cmhr_list [] = {
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
	};

	int	cmhr_no_fields = 2;

	struct	{
		char	hr_cont_no[7];
		long	hr_hhhr_hash;
	} cmhr_rec;

	/*========================================
	| Contract Management Requisition Header |
	========================================*/
	struct dbview cmrh_list [] = {
		{"cmrh_co_no"},
		{"cmrh_br_no"},
		{"cmrh_req_no"},
		{"cmrh_hhrq_hash"},
		{"cmrh_hhhr_hash"},
		{"cmrh_req_date"},
		{"cmrh_rqrd_date"},
		{"cmrh_iss_date"},
		{"cmrh_op_id"},
		{"cmrh_req_by"},
		{"cmrh_time_create"},
		{"cmrh_date_create"},
		{"cmrh_full_supply"},
		{"cmrh_printed"},
		{"cmrh_del_name"},
		{"cmrh_del_adr1"},
		{"cmrh_del_adr2"},
		{"cmrh_del_adr3"},
		{"cmrh_add_int1"},
		{"cmrh_add_int2"},
		{"cmrh_add_int3"},
		{"cmrh_stat_flag"},
	};

	int	cmrh_no_fields = 22;

	struct	{
		char	rh_co_no[3];
		char	rh_br_no[3];
		long	rh_req_no;
		long	rh_hhrq_hash;
		long	rh_hhhr_hash;
		long	rh_req_date;
		long	rh_rqrd_date;
		long	rh_iss_date;
		char	rh_op_id[15];
		char	rh_req_by[21];
		long	rh_time_create;
		long	rh_date_create;
		char	rh_full_supply[2];
		char	rh_printed[2];
		char	rh_del_name[41];
		char	rh_del_adr[3][41];
		char	rh_add_int[3][41];
		char	rh_stat_flag[2];
	} cmrh_rec;

	char	*data  = "data",
			*comm  = "comm",
			*cmhr  = "cmhr",
			*cmrh  = "cmrh";

	int		lpno;
	int		auto_req_no;

	char	rq_branchNo[3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	long	st_req_no;
	long	end_req_no;
	char	st_cont[7];
	char	end_cont[7];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "st_req_no",	MAXLINES, 4, LONGTYPE,
		"NNNNNN", "          ",
		"0", " ", "Requisition No", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.st_req_no},
	{1, TAB, "st_cont",	 0, 4, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "Contract Number", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_cont},
	{1, TAB, "end_req_no",	 0, 4, LONGTYPE,
		"NNNNNN", "          ",
		"0", "", "Requisition No", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.end_req_no},
	{1, TAB, "end_cont",	 0, 4, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "Contract Number", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_cont},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	process			(void);
int		spec_valid		(int field);
void	srch_cmrh		(char *key_val);
int		delete_line		(void);
int		heading			(int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	if (argc != 2)
	{
		/*printf("Usage : %s <lpno>\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlStdMess036),argv[0]);
		return (EXIT_FAILURE);
	}

	/*-------------------------
	| Check auto generation   |
	| of requisition numbers. |
	-------------------------*/
	sptr = chk_env("CM_AUTO_REQ");
	auto_req_no = (sptr == (char *)0) ? 2 : atoi(sptr);

	SETUP_SCR (vars);

	tab_col = 8;

	lpno = atoi(argv[1]);

	OpenDB();

	strcpy(rq_branchNo, (auto_req_no == COMPANY) ? " 0" : comm_rec.test_no);

	init_scr();
	set_tty(); 
	set_masks();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		lcount[1] = 0;

		heading(1);
		entry(1);

		if (prog_exit || restart)
			break;

		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			break;

		prog_exit = 1;

		if (lcount[1] != 0)
			process ();
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
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen(data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec(cmhr,  cmhr_list, cmhr_no_fields, "cmhr_hhhr_hash");
	open_rec(cmrh,  cmrh_list, cmrh_no_fields, "cmrh_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose(cmhr);
	abc_fclose(cmrh);
	abc_dbclose(data);
}

void
process (
 void)
{
	char	req_str[7];
	char	run_prog[51];

	dsp_screen("Re-Printing Requisitions", 
		   comm_rec.tco_no,
		   comm_rec.tco_name);

	sprintf(run_prog, "cm_req_prt %2d", lpno);
	if ((pout = popen(run_prog, "w")) == 0)
		sys_err("Error in cm_req_prt during (POPEN)",errno,PNAME);

	for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt);

		sprintf(req_str, "%06ld", local_rec.st_req_no);
		dsp_process("Requisition ", req_str);

		/*-------------------------
		| Process range for line. |
		-------------------------*/
		strcpy(cmrh_rec.rh_co_no, comm_rec.tco_no);
		strcpy(cmrh_rec.rh_br_no, rq_branchNo);
		cmrh_rec.rh_req_no = local_rec.st_req_no;
		cc = find_rec(cmrh, &cmrh_rec, GTEQ, "r");
		while (!cc &&
		       !strcmp(cmrh_rec.rh_co_no, comm_rec.tco_no) &&
		       !strcmp(cmrh_rec.rh_br_no, rq_branchNo) &&
		       cmrh_rec.rh_req_no <= local_rec.end_req_no)
		{
			if (cmrh_rec.rh_stat_flag[0] != 'C')
				fprintf(pout,"%010ld\n", cmrh_rec.rh_hhrq_hash);

			cc = find_rec(cmrh, &cmrh_rec, NEXT, "r");
		}
	}

	fprintf (pout, "%010ld\n", 0L);
	pclose (pout);
}

int
spec_valid (
 int field)
{
	if (LCHECK("st_req_no")) 
	{
		if (dflt_used)
			return (delete_line());

		if (SRCH_KEY)
		{
			srch_cmrh(temp_str);
			return (EXIT_SUCCESS);
		}

		/*--------------------
		| Lookup requisition |
		--------------------*/
		strcpy(cmrh_rec.rh_co_no, comm_rec.tco_no);
		strcpy(cmrh_rec.rh_br_no, rq_branchNo);
		cmrh_rec.rh_req_no = local_rec.st_req_no;
		cc = find_rec(cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			/*sprintf(err_str,
				"\007 Requisition No %06ld not on file ",
				local_rec.st_req_no);*/
			print_mess(ML(mlCmMess015));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*---------------------------------------------------
		| Can't enter requisitions that have been confirmed |
		---------------------------------------------------*/
		if (cmrh_rec.rh_stat_flag[0] == 'C')
		{
			/*print_mess("\007 Requisition Has Been Completed. ");*/
			print_mess(ML(mlCmMess145));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*------------------
		| Lookup contract. |
		------------------*/
		cc = find_hash(cmhr, &cmhr_rec, COMPARISON, "r", cmrh_rec.rh_hhhr_hash);
		if (cc)
		{
			/*print_mess("\007 Contract Not Found. ");*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.st_cont, "%-6.6s", cmhr_rec.hr_cont_no);
		DSP_FLD("st_cont");
		return(0);
	}

	if (LCHECK("end_req_no")) 
	{
		if (SRCH_KEY)
		{
			srch_cmrh(temp_str);
			return(0);
		}

		/*--------------------
		| Lookup requisition |
		--------------------*/
		strcpy(cmrh_rec.rh_co_no, comm_rec.tco_no);
		strcpy(cmrh_rec.rh_br_no, rq_branchNo);
		cmrh_rec.rh_req_no = local_rec.end_req_no;
		cc = find_rec(cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			/*sprintf(err_str,
				"\007 Requisition No %06ld not on file ",
				local_rec.end_req_no);*/
			print_mess(ML(mlCmMess015));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*---------------------------------------------------
		| Can't enter requisitions that have been confirmed |
		---------------------------------------------------*/
		if (cmrh_rec.rh_stat_flag[0] == 'C')
		{
			/*print_mess("\007 Requisition Has Been Completed. ");*/
			print_mess(ML(mlCmMess145));
			sleep(2);
			clear_mess();
			return(1);
		}

		/*------------------
		| Lookup contract. |
		------------------*/
		cc = find_hash(cmhr, &cmhr_rec, COMPARISON, "r", cmrh_rec.rh_hhhr_hash);
		if (cc)
		{
			/*print_mess("\007 Contract Not Found. ");*/
			print_mess(ML(mlStdMess075));
			sleep(2);
			clear_mess();
			return(1);
		}

		if (local_rec.end_req_no < local_rec.st_req_no)
		{
			/*print_mess("\007 End requisition number must be greater then start requisition number. ");*/
			print_mess(ML(mlStdMess018));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.end_cont, "%-6.6s", cmhr_rec.hr_cont_no);
		DSP_FLD("end_cont");
		return(0);
	}

	return(0);
}

void
srch_cmrh (
 char *	key_val)
{
	char	req_no[7];
	char	desc[41];

	work_open();
	save_rec("#Requisition No", "#Contract | Requested By " );
	strcpy(cmrh_rec.rh_co_no, comm_rec.tco_no);
	strcpy(cmrh_rec.rh_br_no, rq_branchNo);
	cmrh_rec.rh_req_no = atol(key_val);
	cc = find_rec(cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmrh_rec.rh_br_no, rq_branchNo) &&
	       !strcmp(cmrh_rec.rh_co_no, comm_rec.tco_no))
	{
		if (cmrh_rec.rh_stat_flag[0] == 'C')
		{
			cc = find_rec(cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		cc = find_hash(cmhr, &cmhr_rec, COMPARISON, "r", cmrh_rec.rh_hhhr_hash);
		if (!cc)
		{
			sprintf(req_no, "%06ld", cmrh_rec.rh_req_no);
			sprintf(desc, 
				"%-6.6s | %-20.20s", 
				cmhr_rec.hr_cont_no,
				cmrh_rec.rh_req_by);
			cc = save_rec(req_no, desc);
			if (cc)
				break;
		}

		cc = find_rec(cmrh, &cmrh_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cmrh_rec.rh_co_no, comm_rec.tco_no);
	strcpy(cmrh_rec.rh_br_no, rq_branchNo);
	cmrh_rec.rh_req_no = atol(temp_str);
	cc = find_rec(cmrh, &cmrh_rec, GTEQ, "r");
	if (cc)
		file_err(cc, cmrh, "DBFIND");
}

int
delete_line (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		/*print_mess(" Cannot Delete Lines on Entry ");*/
		print_mess(ML(mlStdMess005));
		return(1);
	}

	lcount[1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt + 1);
		putval(line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display();
	}

	putval(line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display();
	
	line_cnt = i;
	getval(line_cnt);
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		clear();

		rv_pr(ML(mlCmMess148),(80 - strlen(ML(mlCmMess148))) / 2,0,1);

		move(0,1);
		line(80);

		rv_pr(ML(mlCmMess149),(40 - strlen(ML(mlCmMess149))) / 2,tab_row - 2,1);
		rv_pr(ML(mlCmMess150),38 + (40 - strlen(ML(mlCmMess150))) / 2,tab_row - 2,1);

		move(0, 20);
		line(80);

		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		strcpy(err_str, ML(mlStdMess039));
		print_at(22,0,err_str,comm_rec.test_no,comm_rec.test_short);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
