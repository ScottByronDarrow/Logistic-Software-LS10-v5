/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( db_gl_eom.c    )                                 |
|  Program Desc  : ( Update General Ledger Controls At End Of Month.  |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, glaj,     ,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  glaj,     ,     ,     ,     ,     ,     ,         |
|  Database      : (dbtr)                                             |
|---------------------------------------------------------------------|
|  Date Written  : 10/05/86        | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
|  Date Modified : (10/05/86)      | Modified by : Scott B. Darrow.   |
|  Date Modified : (16/11/88)      | Modified by : Bee Chwee Lim.     |
|  Date Modified : (15.06.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : Tidied up program, remove exit_prog after read_comm|
|                : & OpenDB and change dbfind to find_rec.           |
|       15.06.94 : Converted to pslscr                                |
|                :                                                    |
|                                                                     |
| $Log: sk_gl_eom.c,v $
| Revision 5.1  2001/08/09 09:18:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:15:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:09  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:43  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  2000/04/08 04:57:04  gerry
| Corrected readlock to writelock - gljc
|
| Revision 1.6  1999/11/03 07:31:58  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.5  1999/10/08 05:32:21  scott
| First Pass checkin by Scott.
|
| Revision 1.4  1999/06/20 05:19:58  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_gl_eom.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_gl_eom/sk_gl_eom.c,v 5.1 2001/08/09 09:18:29 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

	/*===================================
	| File comm { System Common file }. |
	===================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_gl_date"}
	};

	int comm_no_fields = 4;

	struct {
		int  termno;
		char tco_no[3];
		char tco_name[41];
		long t_gl_date;
	} comm_rec;


	/*============================================
	| File gljc {General Ledger Control Record}. |
	============================================*/
	struct dbview gljc_list[] ={
		{"gljc_co_no"},
		{"gljc_journ_type"},
		{"gljc_tot_1"},
		{"gljc_tot_2"},
		{"gljc_tot_3"},
		{"gljc_tot_4"},
		{"gljc_tot_5"},
		{"gljc_tot_6"}
	};

	int gljc_no_fields = 8;

	struct {
		char	 jc_co_no[3];
		char	 jc_journ_type[3];
		double 	 jc_tot[6];
	} gljc_rec;


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void gljc_update (void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv[])
{
	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sprintf(err_str, "Clearing G/L Controls Totals For Company %s.",comm_rec.tco_no);
	dsp_screen(err_str, comm_rec.tco_no, comm_rec.tco_name);

	gljc_update();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");
	open_rec("gljc", gljc_list, gljc_no_fields, "gljc_co_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("gljc");
	abc_dbclose("data");
}

/*===============================
| Set control totals to zero  . |
===============================*/
void
gljc_update (
 void)
{
	int	not_last = 1,
		find_type,
		j_type;

	strcpy(gljc_rec.jc_co_no,comm_rec.tco_no);
	find_type = COMPARISON;
	while (not_last && strcmp(gljc_rec.jc_co_no,comm_rec.tco_no) == 0) 		{
		cc = find_rec("gljc",&gljc_rec,find_type,"u");
		find_type = NEXT;
		if (cc) 
		{
			not_last = 0;
			continue;
		}

		j_type = atoi(gljc_rec.jc_journ_type);

		if (j_type == 10) 
		{
		    gljc_rec.jc_tot[0] = 0.0;
		    gljc_rec.jc_tot[1] = 0.0;
		    gljc_rec.jc_tot[2] = 0.0;
		    gljc_rec.jc_tot[3] = 0.0;
		    gljc_rec.jc_tot[4] = 0.0;
		    gljc_rec.jc_tot[5] = 0.0;
		
		    dsp_process("Control Total # ",gljc_rec.jc_journ_type);

		    if ((cc = abc_update("gljc",&gljc_rec)))
			sys_err("Error in gljc During (DBUPDATE)",cc, PNAME);
		}
	}
}
