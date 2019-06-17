/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_ldpurge.c   )                                 |
|  Program Desc  : ( Purge Labour details from system             )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjld,     ,     ,     ,     ,               |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (22/11/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|  (11/09/97)    : Modified for Multilingual Conversion.              |
|                :                                                    |
|                :                                                    |
|  Date Modified : (20/10/1997)    | Modified  by : Scott B Darrow    |
|   Comments     :   Updated for invoices number length change.       |
|                                                                     |
| $Log: sj_ldpurge.c,v $
| Revision 5.1  2001/08/09 09:17:39  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:14:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:23  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:58  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/09/29 10:13:03  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:06:37  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/20 02:30:33  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ldpurge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ldpurge/sj_ldpurge.c,v 5.1 2001/08/09 09:17:39 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>
#include    <std_decs.h>

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*=====================
	| Labour details file  |
	======================*/
	struct dbview sjld_list[] ={
		{"sjld_co_no"},
		{"sjld_est_no"},
		{"sjld_dp_no"},
		{"sjld_order_no"},
		{"sjld_emp_code"},
		{"sjld_date"},
	};

	int sjld_no_fields = 6;

	struct {
		char	ld_co_no[3];
		char	ld_est_no[3];
		char	ld_dp_no[3];
		long	ld_order_no;
		char	ld_emp_code[11];
		long	ld_date;
	} sjld_rec;

	char    ord_number[9];	

	long	purge_date;

/*=====================
| Function Prototypes |
======================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void proc_ld (void);

/*==========================
| Main Processing Routine. |
===========================*/
int
main (
 int argc,
 char *argv[])
{
	if (argc != 2)
	{
		print_at (0,0,mlSjMess700,argv[0]);
		return (EXIT_FAILURE);
	}

	purge_date = StringToDate (argv[1]);

	if (purge_date < 0L)
	{
		print_at (0,0,mlStdMess111);
		return (EXIT_FAILURE);
	}

	dsp_screen ("Purging Labour Detail",comm_rec.tco_no,comm_rec.tco_name);

	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	proc_ld ();

	shutdown_prog();
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
	open_rec ("sjld", sjld_list, sjld_no_fields, "sjld_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjld");
	abc_dbclose ("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=================================
| Process Labour and Km  charges  |
| from sjld (labour details) file |
=================================*/
void
proc_ld (
 void)
{
	strcpy (sjld_rec.ld_co_no,comm_rec.tco_no);
	strcpy (sjld_rec.ld_est_no,comm_rec.tes_no);
	strcpy (sjld_rec.ld_dp_no,comm_rec.tdp_no);
	sjld_rec.ld_order_no = 0L;
	sjld_rec.ld_date = 0L;
	cc = find_rec ("sjld",&sjld_rec,GTEQ,"r");

	while (!cc && !strcmp (sjld_rec.ld_co_no,comm_rec.tco_no) && 
				  !strcmp (sjld_rec.ld_est_no,comm_rec.tes_no) && 
				  !strcmp (sjld_rec.ld_dp_no,comm_rec.tdp_no))
	{
		if (sjld_rec.ld_date <= purge_date)
		{
			sprintf (ord_number,"%8ld",sjld_rec.ld_order_no);
			dsp_process ("order : ",ord_number);

			cc = abc_delete ("sjld");
			if (cc)
				sys_err ("Error in sjld During (DBDELETE)",cc, PNAME);
		}
		cc = find_rec ("sjld",&sjld_rec,NEXT,"r");
	}
}
