/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mn_acdprint.c )                                  |
|  Program Desc  : ( Menu System Print Access Description         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mnac,     ,     ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  :  6/04/88         |
|---------------------------------------------------------------------|
|  Date Modified : (23/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (14/08/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (12/09/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      :                                                    |
|  (23/05/91)    : Security access codes are now up to 8 characters   |
|                : long. SC 5273 PSL.                                 |
|                :                                                    |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (12/09/97)    : Updated for Multilingual Conversion and			  |
|                                                                     |
| $Log: acdprint.c,v $
| Revision 5.2  2001/08/09 05:13:34  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:32  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:48  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:20  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:16  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/10/20 02:06:49  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/09/29 10:11:08  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:27:01  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 02:36:51  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: acdprint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mn_acdprint/acdprint.c,v 5.2 2001/08/09 05:13:34 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<pr_format3.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	new_code = 0,	
			envDbFind = 0;

	char	branchNo[3];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;
	
	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char tes_no[3];
		char tes_name[41];
		long t_dbt_date;
	} comm_rec;


	/*======================================
	| Menu System Access Description File. |
	======================================*/
	struct dbview mnac_list[] ={
		{"mnac_code"},
		{"mnac_description"}
	};

	int mnac_no_fields = 2;

	struct {
		char	code[9];
		char	desc[31];
	} mnac_rec;


	/*======================================
	| Local Variables                      |
	======================================*/
	int	lpno = 1;

	char	menu[11];

	long	input_date;

	FILE	*fout, *fin;

/*===========================
| Function prototypes.      |
===========================*/
int		main			(int argc, char * argv []);
int		check_page		(void);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	head_output		(void);
int		proc_mnac		(void);
int		print_mnac		(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	/*------------------------------------------------------------
	|Initialize Parameters (0= program, 1= printer)              |
	------------------------------------------------------------*/
	if (argc != 2)
	{
		print_at(0,0, mlStdMess036 ,argv[0]);
		return (EXIT_FAILURE);
	}
	lpno = atoi(argv[1]);

	init_scr();

	OpenDB();

	dsp_screen("Printing Security Access Codes", comm_rec.tco_no, comm_rec.tco_name);

	if ((fin = pr_open("mn_secu.p")) == NULL)
		sys_err("Error in opening mn_secu.p during (FOPEN)",errno,PNAME);

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	head_output();

	proc_mnac();

	pr_format(fin,fout,"END_FILE",0,0);
	pclose(fout);
	fclose(fin);
	shutdown_prog();
	return (EXIT_SUCCESS);
}

int
check_page (void)
{
	return(0);
}

/*========================
| Program exit sequence. |
========================*/
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
OpenDB (void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("mnac", mnac_list, mnac_no_fields, "mnac_code");

	abc_fclose("comm");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("mnac");
	abc_dbclose("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout, ".LP%d\n",lpno);
	fprintf(fout, ".11\n");
	fprintf(fout, ".PI12\n");
	fprintf(fout, ".L80\n");
	fprintf(fout, ".E%s\n", clip(comm_rec.tco_name));
	fprintf(fout, ".ESECURITY ACCESS CODE DETAILS\n");
	pr_format(fin,fout,"BLANK",0,0);
	pr_format(fin,fout,"LINE1",0,0);
	pr_format(fin,fout,"HEAD1",0,0);
	pr_format(fin,fout,"LINE2",0,0);
	pr_format(fin,fout,"RULER",0,0);

	fflush(fout);
}

int
proc_mnac (void)
{
	strcpy(mnac_rec.code,"  ");
	cc = find_rec("mnac",&mnac_rec,GTEQ,"r");

	while (!cc)
	{
		print_mnac();
		cc = find_rec("mnac",&mnac_rec,NEXT,"r");
	}
	return(0);
}

int
print_mnac (void)
{
	pr_format(fin,fout,"DETAIL",1,mnac_rec.code);
	pr_format(fin,fout,"DETAIL",2,mnac_rec.desc);

	fflush(fout);
	return(0);
}
