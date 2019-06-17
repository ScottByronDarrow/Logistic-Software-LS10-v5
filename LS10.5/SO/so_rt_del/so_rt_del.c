/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: so_rt_del.c,v 5.5 2002/04/30 07:56:50 scott Exp $
|  Program Name  : (so_rt_del.c)                                      |
|  Program Desc  : (Delete orders with status of "D"             )    |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/03/92         |
|---------------------------------------------------------------------|
| $Log: so_rt_del.c,v $
| Revision 5.5  2002/04/30 07:56:50  scott
| Update for new Archive modifications;
|
| Revision 5.4  2002/04/29 07:47:15  scott
| Update for new Archive modifications;
|
| Revision 5.3  2001/08/14 02:45:15  scott
| Updated for new delete wizard
|
| Revision 5.2  2001/08/09 09:21:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:55  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/05/31 10:01:20  scott
| Updated to ensure works order is check before delete is allowed
|
| Revision 4.1  2001/04/21 03:55:29  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.0  2001/03/09 02:41:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:22:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:13:46  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/07/13 23:36:14  scott
| Updated to add app.schema as part of general maintenance.
| Updated to delete from lineNumber -1 instead of 0 as some lines could start
| at -1 due to sorting.
|
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_rt_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_rt_del/so_rt_del.c,v 5.5 2002/04/30 07:56:50 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<DeleteControl.h>
#include 	<Archive.h>

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct sohrRecord	sohr2_rec;
struct solnRecord	soln_rec;
struct trhrRecord	trhr_rec;
struct trhrRecord	trhr2_rec;
struct trlnRecord	trln_rec;
struct pcwoRecord	pcwo_rec;

char	*trhr2	=	"trhr2",
		*sohr2	=	"sohr2";

int		envSoRtDelDays 	= 0,
		envSoWoAllowed 	= 0;

/*
 * Function Declarations 
 */
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ProcessOrders 			(void);
void 	DeleteOrderHeader 		(long);
void 	ProcessTransport 		(void);
void 	DeleteTransportHeader 	(long);
int		CheckWorksOrder 		(long);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	init_scr ();
	set_tty (); 
	
	OpenDB ();
	
	/*
	 * Check if works order can be created from backorder line. 
	 */
	sptr = chk_env ("SO_WO_ALLOWED");
	envSoWoAllowed = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if sales orders are deleted real time. 
	 */
	sptr = chk_env ("SO_RT_DEL_DAYS");
	envSoRtDelDays = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "SALES-ORDER-FILE");
	if (!cc)
		envSoRtDelDays		= delhRec.purge_days;
	
envSoRtDelDays		= 0;

	dsp_screen ("Deleting completed Orders.", 
					comm_rec.co_no, comm_rec.co_name);
	ProcessOrders ();
	ProcessTransport ();

	shutdown_prog ();	
    return (EXIT_SUCCESS);
}
	
/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (sohr2, sohr);
	abc_alias (trhr2, trhr);

	open_rec (sohr2, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (trhr,  trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash");
	open_rec (trhr2, trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash");
	open_rec (trln,  trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhsl_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (sohr2);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (trhr2);
	abc_fclose (trhr);
	abc_fclose (trln);
	abc_fclose (pcwo);
	ArchiveClose ();
	abc_dbclose ("data");
}

/*
 * Process deleted invoices. 
 */
void
ProcessOrders (void)
{
	long	DelDate	=	0L;

	sohr_rec.hhso_hash	=	0L;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc)
	{
		DelDate	=	sohr_rec.dt_raised + (long) envSoRtDelDays;
		if (DelDate > comm_rec.inv_date)
		{
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
			continue;
		}
		if (envSoWoAllowed)
		{
			if (CheckWorksOrder (sohr_rec.hhso_hash))
			{
				cc = find_rec (sohr, &sohr_rec, NEXT, "r");
				continue;
			}
		}
		dsp_process ("Order. ", sohr_rec.order_no);

		/*
		 * Process all lines and delete as required. 
		 */
		soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
		soln_rec.line_no 	= -1;
		cc = find_rec (soln, &soln_rec, GTEQ, "u");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (soln_rec.status [0] == 'D')
			{
				cc = ArchiveSoln (soln_rec.hhsl_hash);
				if (cc)
					file_err (cc, soln, "ARCHIVE");

				cc = abc_delete (soln);
				if (cc)
					file_err (cc, soln, "DBDELETE");

				soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
				soln_rec.line_no 	= -1;
				cc = find_rec (soln, &soln_rec, GTEQ, "r");
			}
			else
			{
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
			}
		}
		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = -1;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
		{
			DeleteOrderHeader (sohr_rec.hhso_hash);
			sohr_rec.hhso_hash	=	0L;
			cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
		}
		else
		{
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
		}
	}
}

void
DeleteOrderHeader (
	long	hhsoHash)
{
	sohr2_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr2, &sohr2_rec , EQUAL, "u");
	if (cc)
	{
		abc_unlock (sohr2);
		return;
	}
	cc = ArchiveSohr (sohr2_rec.hhso_hash);
	if (cc)
		file_err (cc, sohr, "ARCHIVE");

	abc_delete (sohr2);
}

/*
 * Process deleted invoices. 
 */
void
ProcessTransport (void)
{
	long	DelDate	=	0L;

	trhr_rec.hhtr_hash	=	0L;
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc)
	{
		dsp_process ("Order. ", trhr_rec.trip_name);

		DelDate	=	MonthEnd (trhr_rec.del_date) + 1L;
		DelDate	=	MonthEnd (DelDate);

		/*
		 * Process all lines and delete as required. 
		 */
		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		cc = find_rec (trln, &trln_rec, GTEQ, "u");
		while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			if (DelDate < comm_rec.inv_date)
			{
				abc_delete (trln);
				trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
				cc = find_rec (trln, &trln_rec, GTEQ, "u");
			}
			else
			{
				abc_unlock (trln);
				cc = find_rec (trln, &trln_rec, NEXT, "u");
			}
		}
		abc_unlock (trln);

		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		cc = find_rec (trln, &trln_rec, GTEQ, "r");
		if (cc || trln_rec.hhtr_hash != trhr_rec.hhtr_hash)
		{
			DeleteTransportHeader (trhr_rec.hhtr_hash);
			trhr_rec.hhtr_hash	=	0L;
			cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
		}
		else
			cc = find_rec (trhr, &trhr_rec, NEXT, "r");
	}
}

void
DeleteTransportHeader (
	long	hhtrHash)
{
	trhr2_rec.hhtr_hash	=	hhtrHash;
	cc = find_rec (trhr2, &trhr2_rec , EQUAL, "u");
	if (cc)
	{
		abc_unlock (trhr2);
		return;
	}
	abc_delete (trhr2);
}
int
CheckWorksOrder (
	long	hhsoHash)
{
	soln_rec.hhso_hash	=	hhsoHash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		pcwo_rec.hhsl_hash	=	soln_rec.hhsl_hash;
		cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
		if (!cc)
		{
			if (pcwo_rec.order_status [0] != 'D' &&
				pcwo_rec.order_status [0] != 'Z')
					return (EXIT_FAILURE);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}
