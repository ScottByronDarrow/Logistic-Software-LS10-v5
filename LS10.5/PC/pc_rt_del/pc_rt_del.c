/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_rt_del.c,v 5.3 2002/11/28 04:41:59 scott Exp $
|  Program Name  : (pc_rt_del.c)
|  Program Desc  : (Delete Works orders with status of "D")
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 2nd May 2002     |
|---------------------------------------------------------------------|
| $Log: pc_rt_del.c,v $
| Revision 5.3  2002/11/28 04:41:59  scott
| Undo comments and last changes
|
| Revision 5.2  2002/11/28 04:09:47  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.1  2002/06/05 03:53:18  scott
| Update to include 'Z' status in delete.
|
| Revision 5.0  2002/05/08 01:24:19  scott
| CVS administration
|
| Revision 1.2  2002/05/07 09:07:35  scott
| Updated for archiving
|
| Revision 1.1  2002/05/02 09:15:18  scott
| First release of new program.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_rt_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_rt_del/pc_rt_del.c,v 5.3 2002/11/28 04:41:59 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<DeleteControl.h>
#include 	<Archive.h>

#include	"schema"

struct commRecord	comm_rec;

struct	pcatRecord 	pcat_rec;
struct	pcbpRecord 	pcbp_rec;
struct	pchsRecord 	pchs_rec;
struct	pclnRecord 	pcln_rec;
struct	pcltRecord 	pclt_rec;
struct	pcmsRecord 	pcms_rec;
struct	pcoiRecord 	pcoi_rec;
struct	pcrqRecord 	pcrq_rec;
struct	pcsfRecord 	pcsf_rec;
struct	pcwlRecord 	pcwl_rec;
struct	pcwoRecord 	pcwo_rec;
struct	pstqRecord 	pstq_rec;
struct	solnRecord 	soln_rec;

int		envPcRtDelDays 	= 0;

/*
 * Function Declarations 
 */
void 	shutdown_prog 			(void);
void 	CloseDB 				(void);
void 	DeleteWorksOrderHeader 	(long);
void 	OpenDB 					(void);
void 	ProcessOrders 			(void);
void	ProcessPcat 			(long);
void	ProcessPcbp 			(long);
void	ProcessPchs 			(long);
void	ProcessPcln 			(long);
void	ProcessPclt 			(long);
void	ProcessPcms 			(long);
void	ProcessPcoi 			(long);
void	ProcessPcrq 			(long);
void	ProcessPcsf 			(long);
void	ProcessPcwl 			(long);
void	ProcessPstq 			(long);
int		OrderLineGone 			(long);

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
	 * Check if sales orders are deleted real time. 
	 */
	sptr = chk_env ("PC_RT_DEL_DAYS");
	envPcRtDelDays = (sptr == (char *)0) ? 90 : atoi (sptr);

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "WORKS-ORDER-FILE");
	if (!cc)
		envPcRtDelDays		= delhRec.purge_days;
	
	dsp_screen ("Deleting completed Works Orders.", 
					comm_rec.co_no, comm_rec.co_name);
	ProcessOrders ();

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

	open_rec (pcat, pcat_list, PCAT_NO_FIELDS, "pcat_hhwo_hash");
	open_rec (pcbp, pcbp_list, PCBP_NO_FIELDS, "pcbp_id_no");
	open_rec (pchs, pchs_list, PCHS_NO_FIELDS, "pchs_hhwo_hash");
	open_rec (pcln, pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pclt, pclt_list, PCLT_NO_FIELDS, "pclt_hhwo_hash");
	open_rec (pcms, pcms_list, PCMS_NO_FIELDS, "pcms_hhwo_hash");
	open_rec (pcoi, pcoi_list, PCOI_NO_FIELDS, "pcoi_id_no");
	open_rec (pcrq, pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcsf, pcsf_list, PCSF_NO_FIELDS, "pcsf_id_no");
	open_rec (pcwl, pcwl_list, PCWL_NO_FIELDS, "pcwl_id_no");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (pstq, pstq_list, PSTQ_NO_FIELDS, "pstq_hhwo_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (pcat);
	abc_fclose (pcbp);
	abc_fclose (pchs);
	abc_fclose (pcln);
	abc_fclose (pclt);
	abc_fclose (pcms);
	abc_fclose (pcoi);
	abc_fclose (pcrq);
	abc_fclose (pcsf);
	abc_fclose (pcwl);
	abc_fclose (pcwo);
	abc_fclose (pstq);
	abc_fclose (soln);
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

	pcwo_rec.hhwo_hash	=	0L;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc)
	{
		DelDate	=	pcwo_rec.reqd_date + (long) envPcRtDelDays;
		if (DelDate > comm_rec.inv_date)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		if (pcwo_rec.order_status [0] != 'D' &&
		    pcwo_rec.order_status [0] != 'Z')
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		if (!OrderLineGone (pcwo_rec.hhsl_hash))
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Works Order. ", pcwo_rec.order_no);

		ProcessPcat (pcwo_rec.hhwo_hash);
		ProcessPcbp (pcwo_rec.hhwo_hash);
		ProcessPchs (pcwo_rec.hhwo_hash);
		ProcessPcln (pcwo_rec.hhwo_hash);
		ProcessPclt (pcwo_rec.hhwo_hash);
		ProcessPcms (pcwo_rec.hhwo_hash);
		ProcessPcoi (pcwo_rec.hhwo_hash);
		ProcessPcrq (pcwo_rec.hhwo_hash);
		ProcessPcsf (pcwo_rec.hhwo_hash);
		ProcessPcwl (pcwo_rec.hhwo_hash);
		ProcessPstq (pcwo_rec.hhwo_hash);

		cc = ArchivePcwo (pcwo_rec.hhwo_hash);
		if (cc)
			file_err (cc, pcwo, "ARCHIVE");

		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (!cc)
		{
			cc = abc_delete (pcwo);
			if (cc)
				file_err (cc, pcwo, "DBDELETE");
		}
		else
		{
			abc_unlock (pcwo);
		}

		pcwo_rec.hhwo_hash	=	0L;
		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	}
}
/*
 * Check if sales order line has been removed.
 */
int
OrderLineGone (
	long	hhslHash)
{
	if (hhslHash == 0L)
		return (1);

	soln_rec.hhsl_hash	=	hhslHash;
	return (find_rec (soln, &soln_rec, EQUAL, "r"));
}

/*
 * Process - 
 */
void
ProcessPcat (
	long	hhwoHash)
{
	pcat_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	while (!cc && pcat_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcat);
		if (cc) 
			file_err (cc, pcat, "DBDELETE");

		pcat_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pcat, &pcat_rec, GTEQ, "u");
	}
	abc_unlock (pcat);
	return;
}
void
ProcessPcbp (
	long	hhwoHash)
{
	pcbp_rec.hhwo_hash	=	hhwoHash;
	pcbp_rec.seq_no		=	0;
	pcbp_rec.hhbr_hash	=	0L;
	cc = find_rec (pcbp, &pcbp_rec, GTEQ, "u");
	while (!cc && pcbp_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcbp);
		if (cc) 
			file_err (cc, pcbp, "DBDELETE");

		pcbp_rec.hhwo_hash	=	hhwoHash;
		pcbp_rec.seq_no		=	0;
		pcbp_rec.hhbr_hash	=	0L;
		cc = find_rec (pcbp, &pcbp_rec, GTEQ, "u");
	}
	abc_unlock (pcbp);
	return;
}
void
ProcessPchs (
	long	hhwoHash)
{
	pchs_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pchs, &pchs_rec, GTEQ, "u");
	while (!cc && pchs_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pchs);
		if (cc) 
			file_err (cc, pchs, "DBDELETE");

		pchs_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pchs, &pchs_rec, GTEQ, "u");
	}
	abc_unlock (pchs);
	return;
}
void
ProcessPcln (
	long	hhwoHash)
{
	pcln_rec.hhwo_hash	=	hhwoHash;
	pcln_rec.seq_no		=	0;
	pcln_rec.line_no	=	0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	while (!cc && pcln_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcln);
		if (cc) 
			file_err (cc, pcln, "DBDELETE");

		pcln_rec.hhwo_hash	=	hhwoHash;
		pcln_rec.seq_no		=	0;
		pcln_rec.line_no	=	0;
		cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	}
	abc_unlock (pcln);
	return;
}
void
ProcessPclt (
	long	hhwoHash)
{
	pclt_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pclt, &pclt_rec, GTEQ, "u");
	while (!cc && pclt_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pclt);
		if (cc) 
			file_err (cc, pclt, "DBDELETE");

		pclt_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pclt, &pclt_rec, GTEQ, "u");
	}
	abc_unlock (pclt);
	return;
}
void
ProcessPcms (
	long	hhwoHash)
{
	pcms_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcms);
		if (cc) 
			file_err (cc, pcms, "DBDELETE");

		pcms_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	}
	abc_unlock (pcms);
	return;
}
void
ProcessPcoi (
	long	hhwoHash)
{
	pcoi_rec.hhwo_hash	=	hhwoHash;
	pcoi_rec.line_no	=	0;
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "u");
	while (!cc && pcoi_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcoi);
		if (cc) 
			file_err (cc, pcoi, "DBDELETE");

		pcoi_rec.hhwo_hash	=	hhwoHash;
		pcoi_rec.line_no	=	0;
		cc = find_rec (pcoi, &pcoi_rec, GTEQ, "u");
	}
	abc_unlock (pcoi);
	return;
}
void
ProcessPcrq (
	long	hhwoHash)
{
	pcrq_rec.hhwo_hash	=	hhwoHash;
	pcrq_rec.seq_no		=	0;
	pcrq_rec.line_no	=	0;
	cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
	while (!cc && pcrq_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcrq);
		if (cc) 
			file_err (cc, pcrq, "DBDELETE");

		pcrq_rec.hhwo_hash	=	hhwoHash;
		pcrq_rec.seq_no		=	0;
		pcrq_rec.line_no	=	0;
		cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
	}
	abc_unlock (pcrq);
	return;
}
void
ProcessPcsf (
	long	hhwoHash)
{
	pcsf_rec.hhwo_hash	=	hhwoHash;
	pcsf_rec.uniq_id	=	0;
	pcsf_rec.line_no	=	0;
	cc = find_rec (pcsf, &pcsf_rec, GTEQ, "u");
	while (!cc && pcsf_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcsf);
		if (cc) 
			file_err (cc, pcsf, "DBDELETE");

		pcsf_rec.hhwo_hash	=	hhwoHash;
		pcsf_rec.uniq_id	=	0;
		pcsf_rec.line_no	=	0;
		cc = find_rec (pcsf, &pcsf_rec, GTEQ, "u");
	}
	abc_unlock (pcsf);
	return;
}
void
ProcessPcwl (
	long	hhwoHash)
{
	pcwl_rec.hhwo_hash	=	hhwoHash;
	pcwl_rec.hhcu_hash	=	0L;
	cc = find_rec (pcwl, &pcwl_rec, GTEQ, "u");
	while (!cc && pcwl_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pcwl);
		if (cc) 
			file_err (cc, pcwl, "DBDELETE");

		pcwl_rec.hhwo_hash	=	hhwoHash;
		pcwl_rec.hhcu_hash	=	0L;
		cc = find_rec (pcwl, &pcwl_rec, GTEQ, "u");
	}
	abc_unlock (pcwl);
	return;
}
void
ProcessPstq (
	long	hhwoHash)
{
	pstq_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	while (!cc && pstq_rec.hhwo_hash == hhwoHash)
	{
		cc = abc_delete (pstq);
		if (cc) 
			file_err (cc, pstq, "DBDELETE");

		pstq_rec.hhwo_hash	=	hhwoHash;
		cc = find_rec (pstq, &pstq_rec, GTEQ, "u");
	}
	abc_unlock (pstq);
	return;
}
