/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ol_savdel.c    )                                 |
|  Program Desc  : ( Online Invoice - Saved invoice delete program.)  |
|                  (                                               )  |
|---------------------------------------------------------------------|
|  Access files  :  comm, cohr, coln,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr, coln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 12/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (11/06/91)      | Modified  by  :                  |
|  Date Modified : (10/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (24/09/97)      | Modified  by  : Ana Marie Tario  |
|  Date Modified : (28/10/1997)    | Modified by : Campbell Mander.   |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
|  Comments      : Based on so_auddel.c                               |
|                : (11/06/91) - Updated to clean up.                  |
|                : (10/04/94) - PSL 10673 - Online conversion         |
|                : (24/09/97) - Multilingual conversion and DMY4 date.|
|  (28/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: ol_savdel.c,v $
| Revision 5.1  2001/08/09 09:14:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:09:53  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:02  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:26  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.8  1999/09/29 10:11:28  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/20 05:51:24  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/09/10 02:10:48  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.5  1999/06/15 09:39:18  scott
| Updated for log file.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_savdel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_savdel/ol_savdel.c,v 5.1 2001/08/09 09:14:21 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	SAVED	(cohr_rec.hr_type[0] == 'O' || cohr_rec.hr_type[0] == 'R')

	/*===================================
	| File comm { System Common file }. |
	===================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_dbt_date"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		long	tdbt_date;
	} comm_rec;

	/*==============================================
	| File cohr { Customer Order/Invoice Header }. |
	==============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_stat_flag"}
	};

	int cohr_no_fields = 8;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhco_hash;
		long	hr_date_raised;
		char	hr_stat_flag[2];
	} cohr_rec;

	/*===================================
	| File coln {Customer Order Lines}. |
	===================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_stat_flag"}
	};

	int coln_no_fields = 3;

	struct {
		long	ln_hhco_hash;
		int	ln_line_no;
		char	ln_stat_flag[2];
	} coln_rec;

	int	first_flag = 0;

#include	<std_decs.h>
void OpenDB (void);
void CloseDB (void);
void proccoln (long shash);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
	int argc, 
	char *argv [])
{
/*
	if (argc != 1)
	{
		printf(" Usage : %s\007\n\r",argv[0]);
		return (EXIT_FAILURE);
	}*/

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB();

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	dsp_screen("Invoice Audit Delete.",comm_rec.tco_no,comm_rec.tco_name);

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,"  ");
	strcpy(cohr_rec.hr_type," ");
	strcpy(cohr_rec.hr_stat_flag," ");

	cc = find_rec("cohr",&cohr_rec,GTEQ,"r");

	while (!cc && !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no))
	{
		if (SAVED)
		{
			dsp_process("Trans : ",cohr_rec.hr_inv_no);
			proccoln(cohr_rec.hr_hhco_hash);
			cc = abc_delete("cohr");
			if (cc)
			{
				cc = find_rec("cohr", &cohr_rec, NEXT, "r");
				continue;
			}
			cc = find_rec("cohr",&cohr_rec,GTEQ,"r");
		}
		else
		 	cc = find_rec("cohr", &cohr_rec, NEXT, "r");
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
	void)
{
	abc_dbopen("data");

	open_rec("cohr",cohr_list,cohr_no_fields,"cohr_up_id");
	open_rec("coln",coln_list,coln_no_fields,"coln_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose("cohr");
	abc_fclose("coln");
	abc_dbclose("data");
}

void
proccoln (
	long shash)
{
	/*---------------------------
	| Process all order lines . |
	---------------------------*/
	coln_rec.ln_hhco_hash = shash;
	coln_rec.ln_line_no = 0;
	cc = find_rec("coln",&coln_rec,GTEQ,"r");

	while (!cc && coln_rec.ln_hhco_hash == shash) 
	{
		cc = abc_delete("coln");
		if (cc)
		{
			cc = find_rec("coln",&coln_rec,NEXT,"r");
			continue;
		}

		cc = find_rec("coln",&coln_rec,GTEQ,"r");

	}	/* End of while ... loop */
}

