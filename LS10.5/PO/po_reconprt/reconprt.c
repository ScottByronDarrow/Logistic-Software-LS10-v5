/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: reconprt.c,v 5.2 2001/08/09 09:16:04 scott Exp $
|  Program Name  : ( po_reconprt.c )                                  |
|  Program Desc  : ( Print Clearing Reconciliation Report.	  )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, posd, pohr,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (28/02/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|  Date Modified : (16/08/89)      | Modified  by  : Roger Gibbison.  |
|                                                                     |
|  Comments      : (17/04/89) Changed MONEYTYPEs to DOUBLETYPEs.      |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: reconprt.c,v $
| Revision 5.2  2001/08/09 09:16:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:13  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:15  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:41  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:01  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:32  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:32:50  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/10/14 03:04:25  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.8  1999/09/29 10:12:12  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 04:38:10  scott
| Updated from Ansi project
|
| Revision 1.6  1999/06/17 10:06:36  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: reconprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_reconprt/reconprt.c,v 5.2 2001/08/09 09:16:04 scott Exp $";

#define	MOD	1
#define		NO_SCRGEN
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct posdRecord	posd_rec;
struct poshRecord	posh_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;

	int	printerNumber = 1;

	char	*data = "data";

	FILE	*pout;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReportHeading 	(char *);
void 	Process 		(long);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	long	hhshHash;

	if (argc != 3)
	{
		print_at (0,0,mlPoMess725,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	hhshHash = atol (argv [2]);

	OpenDB ();
	Process (hhshHash);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

void
CloseDB (
 void)
{
	abc_fclose (posd);
	abc_fclose (pohr);
	abc_fclose (posh);
	abc_fclose (sumr);
	abc_dbclose (data);
}

void
ReportHeading (
 char	*csm_no)
{
	char	*sptr = chk_env ("CURR_CODE");

	if ( (pout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat during (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pout,".LP%d\n",printerNumber);

	fprintf (pout,".11\n");
	fprintf (pout,".PI12\n");
	fprintf (pout,".L125\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s\n",clip (comm_rec.co_name));
	fprintf (pout,".EPURCHASE ORDER CLEARING RECONCILIATION\n");
	fprintf (pout,".EFOR SHIPMENT : %s\n",csm_no);
	fprintf (pout,".EAs At %-24.24s\n",SystemTime ());

	fprintf (pout,".R=========");
	fprintf (pout,"===========================================");
	fprintf (pout,"==================");
	fprintf (pout,"==================");
	fprintf (pout,"==================");
	fprintf (pout,"===================\n");

	fprintf (pout,"=========");
	fprintf (pout,"===========================================");
	fprintf (pout,"==================");
	fprintf (pout,"==================");
	fprintf (pout,"==================");
	fprintf (pout,"===================\n");

	fprintf (pout,"|SUPPLIER");
	fprintf (pout,"|        S U P P L I E R    N A M E        ");
	fprintf (pout,"|  PURCHASE ORDER ");
	fprintf (pout,"|   INVOICE NO.   ");
	fprintf (pout,"|    FOB VALUE    ");
	fprintf (pout,"|    %-3.3s VALUE    |\n",(sptr == (char *)0) ? "???" : sptr);

	fprintf (pout,"|--------");
	fprintf (pout,"|------------------------------------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------|\n");
	fflush (pout);
}

void
Process (
 long	hhshHash)
{
	double	local_val 	= 0.00;
	double	local_tot 	= 0.00;
	double	fgn_tot 	= 0.00;

	strcpy (posh_rec.co_no,comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
		return;

	dsp_screen (" Printing Clearing Reconciliation Report ",comm_rec.co_no,comm_rec.co_name);

	ReportHeading (posh_rec.csm_no);

	strcpy (posd_rec.co_no,comm_rec.co_no);
	posd_rec.hhsh_hash = hhshHash;
	posd_rec.hhpo_hash = 0L;
	cc = find_rec (posd,&posd_rec,GTEQ,"r");
	while (!cc && !strcmp (posd_rec.co_no,comm_rec.co_no)
			   && posd_rec.hhsh_hash == hhshHash)
	{
		pohr_rec.hhpo_hash	=	posd_rec.hhpo_hash;
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (!cc)
		{
			sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
			cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
			if (!cc)
			{
				local_val = posd_rec.total;
				if (posh_rec.ex_rate != 0.00)
					local_val /= posh_rec.ex_rate;

				fprintf (pout,"| %-6.6s ",		sumr_rec.crd_no);
				fprintf (pout,"| %-40.40s ",	sumr_rec.crd_name);
				fprintf (pout,"| %-15.15s ",	pohr_rec.pur_ord_no);
				fprintf (pout,"| %-15.15s ",	posd_rec.inv_no);
				fprintf (pout,"| %15.2f ",		posd_rec.total);
				fprintf (pout,"| %15.2f |\n",	local_val);
				fflush (pout);

				dsp_process (" Invoice :",posd_rec.inv_no);

				fgn_tot += posd_rec.total;
				local_tot += local_val;
			}
		}

		cc = find_rec (posd,&posd_rec,NEXT,"r");
	}

	fprintf (pout,"|--------");
	fprintf (pout,"|------------------------------------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------");
	fprintf (pout,"|-----------------|\n");

	fprintf (pout,"|        ");
	fprintf (pout,"                                           ");
	fprintf (pout,"                  ");
	fprintf (pout,"                  ");
	fprintf (pout,"| %15.2f ",fgn_tot);
	fprintf (pout,"| %15.2f |\n",local_tot);

	fprintf (pout,".EOF\n");
	pclose (pout);
}
