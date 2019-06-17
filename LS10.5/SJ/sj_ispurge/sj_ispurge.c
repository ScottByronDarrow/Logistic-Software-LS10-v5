/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_ispurge.c   )                                 |
|  Program Desc  : ( Purge service job invoice summary file       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjis, sjig,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (18/11/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (26/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (16/10/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : Tidy up program.                                   |
|                : (26/09/97) : Incorporated multilingual conversion. |
|                : (16/10/97) : Changed length from 6 to 8.           |
|                :                                                    |
|                                                                     |
| $Log: sj_ispurge.c,v $
| Revision 5.1  2001/08/09 09:17:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:14:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:53  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/09/29 10:12:58  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 05:06:34  scott
| Updated from Ansi
|
| Revision 1.7  1999/06/20 02:30:29  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ispurge.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ispurge/sj_ispurge.c,v 5.1 2001/08/09 09:17:30 scott Exp $";

#define	NO_SCRGEN
#include <ml_sj_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

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
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*====================================
	| Service Job Invoice Summary  File. |
	====================================*/
	struct dbview sjis_list[] ={
		{"sjis_co_no"},
		{"sjis_est_no"},
		{"sjis_dp_no"},
		{"sjis_invno"},
		{"sjis_order_no"},
		{"sjis_date"},
	};

	int sjis_no_fields = 6;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
	} sjis_rec;

	/*======================
	| Service Invoice G/L  |
	======================*/
	struct dbview sjig_list[] ={
		{"sjig_co_no"},
		{"sjig_est_no"},
		{"sjig_dp_no"},
		{"sjig_invno"},
		{"sjig_order_no"},
		{"sjig_acc_no"},
		{"sjig_amount"}
	};

	int sjig_no_fields = 7;

	struct {
		char	ig_co_no[3];
		char	ig_est_no[3];
		char	ig_dp_no[3];
		char	ig_invno[9];
		long	ig_order_no;
		char	ig_acc_no[17];
		double	ig_amount;
	} sjig_rec;

	char    ord_number[9];	

	long	purge_date;

/*======================
| Function Prototypes  |
======================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void proc_file (void);
void proc_sjig (void);

/*==================
| Start of Program |
===================*/
int
main (
 int argc,
 char *argv[])
{
	if (argc != 2)
	{
		print_at (0,0,mlSjMess707,argv[0]);
		return (EXIT_FAILURE);
	}

	purge_date = StringToDate (argv[1]);

	if (purge_date < 0L)
	{
		print_at (0,0,ML (mlSjMess050),DateToString(purge_date));
	    return (EXIT_FAILURE);	
	}

	dsp_screen (" Purging Service Job Invoice Summary File ",comm_rec.tco_no,comm_rec.tco_name);

	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	proc_file ();

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
	open_rec("sjis", sjis_list, sjis_no_fields, "sjis_id_no");
	open_rec("sjig", sjig_list, sjig_no_fields, "sjig_id_no2");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose("sjis");
	abc_fclose("sjig");
	abc_dbclose("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=================
| Purge sjis file |
=================*/
void
proc_file (
 void)
{
	strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no,comm_rec.tes_no);
	strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);
	sprintf (sjis_rec.is_invno,"%-8.8s"," ");
	cc = find_rec ("sjis", &sjis_rec, GTEQ, "r");

	while(!cc && !strcmp (sjis_rec.is_co_no,comm_rec.tco_no) && 
				 !strcmp (sjis_rec.is_est_no,comm_rec.tes_no) && 
				 !strcmp (sjis_rec.is_dp_no,comm_rec.tdp_no))
	{
		if (sjis_rec.is_date <= purge_date)
		{
			sprintf (ord_number,"%8ld",sjis_rec.is_order_no);
			dsp_process ("Order No : ",ord_number);

			proc_sjig ();
			
			cc = abc_delete ("sjis");
			if (cc)
				sys_err ("Error in sjis During (DBDELETE)",cc, PNAME);
		}
		cc = find_rec ("sjis", &sjis_rec, NEXT, "r");
	}
}

void
proc_sjig (
 void)
{
	strcpy (sjig_rec.ig_co_no,comm_rec.tco_no);
	strcpy (sjig_rec.ig_est_no,comm_rec.tes_no);
	strcpy (sjig_rec.ig_dp_no,comm_rec.tdp_no);
	sprintf (sjig_rec.ig_invno,"%-8.8s",sjis_rec.is_invno);
	sjig_rec.ig_order_no = sjis_rec.is_order_no;
	sprintf (sjig_rec.ig_acc_no,"%-16.16s"," ");
	cc = find_rec ("sjig", &sjig_rec, GTEQ, "r");

	while(!cc && !strcmp (sjig_rec.ig_co_no,comm_rec.tco_no) && 
			     !strcmp (sjig_rec.ig_est_no,comm_rec.tes_no) && 
			     !strcmp (sjig_rec.ig_dp_no,comm_rec.tdp_no) && 
			     sjig_rec.ig_order_no == sjis_rec.is_order_no && 
			     !strcmp (sjig_rec.ig_invno,sjis_rec.is_invno))
	{
		sprintf (ord_number,"%ld",sjig_rec.ig_order_no);
		cc = abc_delete ("sjig");
		if (cc)
			sys_err ("Error in sjig During (DBDELETE)",cc, PNAME);

		cc = find_rec ("sjig", &sjig_rec, NEXT, "r");
	}
}
