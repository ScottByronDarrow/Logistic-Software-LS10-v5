/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ms_lists.c     )                                 |
|  Program Desc  : ( External file print program.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, esmr, cudp, ccmr,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 02/04/87         |
|---------------------------------------------------------------------|
|  Date Modified : (02/04/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (17/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      :                                                    |
|  (17/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (03/09/1999)  : Ported to ANSI standards                           |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: ms_lists.c,v $
| Revision 5.1  2001/08/09 05:13:39  scott
| Updated to use FinishProgram ();
|
| Revision 5.0  2001/06/19 08:08:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:55  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/09/29 10:11:12  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:04  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/06/15 02:36:52  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ms_lists.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/ms_lists/ms_lists.c,v 5.1 2001/08/09 05:13:39 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_menu_mess.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		};

	int comm_no_fields = 6;

	struct {
		int termno;
		char tco_no [3];
		char tco_name [41];
		char test_no [3];
		char test_name [41];
		long tdbt_date;
		} comm_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list [] = {
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
		{"esmr_adr1"},
		{"esmr_adr2"},
		{"esmr_adr3"},
		{"esmr_stat_flag"}
		};

	int esmr_no_fields = 8;

	struct {
		char es_co_no [3];
		char es_est_no [3];
		char es_est_name [41];
		char es_short_name [16];
		char es_adr1 [41];
		char es_adr2 [41];
		char es_adr3 [41];
		char es_stat_flag [2];
		} esmr_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list [] = {
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
		{"cudp_dp_short"},
		{"cudp_location"},
		{"cudp_stat_flag"}
		};

	int cudp_no_fields = 7;

	struct {
		char dp_co_no [3];
		char dp_br_no [3];
		char dp_no [3];
		char dp_name [41];
		char dp_short [16];
		char dp_location [41];
		char dp_stat_flag [2];
		} cudp_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list [] = {
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_master_wh"},
		{"ccmr_name"},
		{"ccmr_acronym"},
		{"ccmr_type"},
		{"ccmr_sal_ok"},
		{"ccmr_pur_ok"},
		{"ccmr_issues_ok"},
		{"ccmr_receipts"},
		{"ccmr_reports_ok"},
		{"ccmr_stat_flag"}
		};

	int ccmr_no_fields = 14;

	struct {
		char cm_co_no [3];
		char cm_est_no [3];
		char cm_no [3];
		long cm_hhcc_hash;
		char cm_master_wh [2];
		char cm_name [41];
		char cm_acronym [10];
		char cm_type [3];
		char cm_sal_ok [2];
		char cm_pur_ok [2];
		char cm_issues_ok [2];
		char cm_receipts [2];
		char cm_reports_ok [2];
		char cm_stat_flag [2];
		} ccmr_rec;


	int	rep_type = 1,
		lp_no = 1;

	int	find_type = GTEQ;

	FILE	*pp;
	
	static char *rep_desc[] = {
 		"BRANCH MASTER FILE REPORT.",
 		"DEPARTMENT MASTER FILE REPORT.",
 		"WAREHOUSE MASTER FILE REPORT."
	};

/*============================
| Local function prototypes  |
============================*/
void	OpenDB			(void);
void	CloseDB		(void);
void	start_report	(int);
void	proc_file		(void);
void	prnt_esmr		(void);
void	prnt_cudp		(void);
void	prnt_ccmr		(void);
void	end_report		(void);


int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 3)
	{
		/*------------------------------------------------------
		| Usage : %s lpno report-type (1=esmr,2=cudt,3=ccmr)\n |
		------------------------------------------------------*/
		sprintf (err_str, "%s %s",mlMenuMess163,mlMenuMess709);
	    print_at(0,0, err_str, argv [0]);
	    return (EXIT_FAILURE);
	}

	rep_type = atoi(argv[2]);
	lp_no = atoi(argv[1]);

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sprintf (err_str, "PRINTING %s", rep_desc [rep_type - 1]);
	dsp_screen (err_str, comm_rec.tco_no, comm_rec.tco_name);
	start_report (lp_no);

	proc_file ();
	
	end_report ();
	pclose (pp);
	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	open_rec("esmr", esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec("ccmr", ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec("cudp", cudp_list, cudp_no_fields, "cudp_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("esmr");
	abc_fclose ("ccmr");
	abc_fclose ("cudp");
	abc_dbclose ("data");
}

void
start_report (
 int	prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf(pp,".START%s<%s>\n", DateToString(comm_rec.tdbt_date),PNAME);
	fprintf(pp,".LP%d\n",prnt_no);
	fprintf(pp,".14\n");
	fprintf(pp,".L134\n");
	fprintf(pp,".E%s\n",rep_desc[rep_type - 1]);
	fprintf(pp,".B1\n");
	fprintf(pp,".E%s\n",comm_rec.tco_name);
	fprintf(pp,".B1\n");
	fprintf(pp,".B1\n.EAS AT : %s.B1\n", SystemTime ());

	switch (rep_type)	
	{
    	   case 1:
        	fprintf(pp, ".R======");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "================");
        	fprintf(pp, "==========================================");
        	fprintf(pp, "============================|\n");

        	fprintf(pp, "======");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "================");
        	fprintf(pp, "==========================================");
        	fprintf(pp, "============================|\n");

        	fprintf(pp, "| BR |");
        	fprintf(pp, "           BRANCH DESCRIPTION           |");
        	fprintf(pp, "    ACRONYM    |");
        	fprintf(pp, "                           ADDRESS DETAILS");
        	fprintf(pp, "                            |\n");
		
        	fprintf(pp, "|----|");
        	fprintf(pp, "----------------------------------------|");
        	fprintf(pp, "---------------|");
        	fprintf(pp, "------------------------------------------");
        	fprintf(pp, "----------------------------|\n");
	   break;

    	   case 2:
        	fprintf(pp, ".R===========");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "================");
        	fprintf(pp, "=========================================\n");
		
        	fprintf(pp, "===========");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "================");
        	fprintf(pp, "=========================================\n");
		
        	fprintf(pp, "| BR | DP |");
        	fprintf(pp, "            DEPARTMENT NAME             |");
        	fprintf(pp, "    ACRONYM    |");
        	fprintf(pp, "           DEPARTMENT LOCATION          |\n");
		
        	fprintf(pp, "|----|----|");
        	fprintf(pp, "----------------------------------------|");
        	fprintf(pp, "---------------|");
        	fprintf(pp, "----------------------------------------|\n");
	   break;

    	   case 3:
        	fprintf(pp, ".R============");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "=========");
        	fprintf(pp, "==========================================\n");
		
        	fprintf(pp, "============");
        	fprintf(pp, "=========================================");
        	fprintf(pp, "=========");
        	fprintf(pp, "==========================================\n");
		
        	fprintf(pp, "| BR | WH |");
        	fprintf(pp, "            WAREHOUSE  NAME             |");
        	fprintf(pp, " ACRONYM |");
        	fprintf(pp, "SALES | PUR. | ISS. | REC. |REPORT|MASTER|\n");
		
        	fprintf(pp, "|----|----|");
        	fprintf(pp, "----------------------------------------|");
        	fprintf(pp, "---------|");
        	fprintf(pp, "------|------|------|------|------|------|\n");
		
	   break;
	}

	fprintf (pp,".PI12\n");
}

/*===========================
| Validate and print lines. |
===========================*/
void
proc_file (
 void)
{
	find_type = GTEQ;

	strcpy(esmr_rec.es_co_no, comm_rec.tco_no);
	strcpy(esmr_rec.es_est_no, "  ");

	strcpy(cudp_rec.dp_co_no, comm_rec.tco_no);
	strcpy(cudp_rec.dp_br_no,"  ");
	strcpy(cudp_rec.dp_no,"  ");

	strcpy(ccmr_rec.cm_co_no, comm_rec.tco_no);
	strcpy(ccmr_rec.cm_est_no,"  ");
	strcpy(ccmr_rec.cm_no,"  ");
	switch (rep_type)	
	{
	    case 1:
			prnt_esmr();
			break;
	    case 2:
			prnt_cudp();
			break;
	    case 3:
			prnt_ccmr();
			break;
	}
}

void
prnt_esmr (
 void)
{
	char wk_desc [130];

	while (1)
	{
		cc = find_rec("esmr", &esmr_rec, find_type, "r");
		if (cc || strcmp(esmr_rec.es_co_no, comm_rec.tco_no) != 0)
			break;
		
		sprintf(wk_desc, "%s, %s, %s",clip(esmr_rec.es_adr1),clip(esmr_rec.es_adr2),clip(esmr_rec.es_adr3));

		fprintf(pp,"| %2.2s |",esmr_rec.es_est_no);
		fprintf(pp,"%40.40s|",esmr_rec.es_est_name);
		fprintf(pp,"%15.15s|",esmr_rec.es_short_name);
		fprintf(pp,"%-70.70s|\n",wk_desc);

		find_type = NEXT;
		dsp_process("Branch : ",esmr_rec.es_est_no);
	}
}

void
prnt_cudp (
 void)
{
	while (1)
	{
		cc = find_rec ("cudp", &cudp_rec, find_type, "r");
		if (cc || strcmp (cudp_rec.dp_co_no, comm_rec.tco_no) != 0)
			break;

		fprintf(pp,"| %2.2s |",cudp_rec.dp_br_no);
		fprintf(pp," %2.2s |",cudp_rec.dp_no);
		fprintf(pp,"%40.40s|",cudp_rec.dp_name);
		fprintf(pp,"%15.15s|",cudp_rec.dp_short);
		fprintf(pp,"%40.40s|\n",cudp_rec.dp_location);

		find_type = NEXT;
		dsp_process("Department : ",cudp_rec.dp_no);
	}
}

void
prnt_ccmr (
 void)
{
	while (1)
	{
		cc = find_rec("ccmr", &ccmr_rec, find_type, "r");
		if (cc || strcmp(ccmr_rec.cm_co_no, comm_rec.tco_no) != 0)
			break;

		fprintf(pp,"| %2.2s |",ccmr_rec.cm_est_no);
		fprintf(pp," %2.2s |",ccmr_rec.cm_no);
		fprintf(pp,"%40.40s|",ccmr_rec.cm_name);
		fprintf(pp,"%9.9s|",ccmr_rec.cm_acronym);
		if (ccmr_rec.cm_sal_ok[0] == 'Y')
			fprintf(pp," YES. |");
		else
			fprintf(pp,"  NO  |");

		if (ccmr_rec.cm_pur_ok[0] == 'Y')
			fprintf(pp," YES. |");
		else
			fprintf(pp,"  NO  |");

		if (ccmr_rec.cm_issues_ok[0] == 'Y')
			fprintf(pp," YES. |");
		else
			fprintf(pp,"  NO  |");
			
		if (ccmr_rec.cm_receipts[0] == 'Y')
			fprintf(pp," YES. |");
		else
			fprintf(pp,"  NO  |");
			
		if (ccmr_rec.cm_reports_ok[0] == 'Y')
			fprintf(pp," YES. |");
		else
			fprintf(pp,"  NO  |");
			
		if (ccmr_rec.cm_master_wh[0] == 'Y')
			fprintf(pp," YES. |\n");
		else
			fprintf(pp,"  NO  |\n");
			
		find_type = NEXT;
		dsp_process("Warehouse : ",ccmr_rec.cm_no);
	}
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
end_report (
 void)
{
	fprintf(pp,".EOF\n");
}
