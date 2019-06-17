/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_grin_del.c,v 5.1 2002/05/08 00:48:23 scott Exp $
|  Program Name  : (po_grin_del.c )                                   |
|  Program Desc  : (Purchase Order pogh/pogl delete Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/09/90         |
|---------------------------------------------------------------------|
| $Log: po_grin_del.c,v $
| Revision 5.1  2002/05/08 00:48:23  scott
| Updated to bring version number to 5.0
|
| Revision 5.6  2002/05/02 01:37:00  scott
| Updated to add Archive functions
|
| Revision 5.5  2002/04/30 07:57:39  scott
| Update for new Archive modifications;
|
| Revision 5.4  2001/11/06 05:41:58  scott
| General clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_grin_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_grin_del/po_grin_del.c,v 5.1 2002/05/08 00:48:23 scott Exp $";

#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <DeleteControl.h>
#include <Archive.h>

#define	DEL_HEAD	 ((pogh2_rec.pur_status [0] == 'A' &&  \
                   	   pogh2_rec.gl_status [0] == 'D') ||  \
		   	   		   pogh2_rec.gl_status [0] == 'D')

#define	DEL_LINE	 ((pogl_rec.pur_status [0] == 'A' &&  \
                   	   pogl_rec.gl_status [0] == 'D') ||  \
		   	   		   pogl_rec.gl_status [0] == 'D')

#define	SHIP_AT_INP	 (posh_rec.status [0] == 'I')

#define	OLD_GRIN	 (lsystemDate - pogh2_rec.date_raised >= envPoPurge)

#include	"schema"

struct commRecord	comm_rec;
struct poglRecord	pogl_rec;
struct poghRecord	pogh_rec;
struct poghRecord	pogh2_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;
struct poslRecord	posl_rec;
struct posdRecord	posd_rec;
struct pogdRecord	pogd_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct poliRecord	poli_rec;

	char	*poln2	=	"poln2",
			*pogh2	=	"pogh2",
			*posh2	=	"posh2",
			*data	=	"data";

	long	envPoPurge	=	0L,
			lsystemDate	=	0L;

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessGrin		(void);
void 	DeletePosh 		(long);
void 	DeletePosl 		(long);
void 	DeletePosd 		(long);
void 	DeletePogd 		(long);
void 	ProcessPogl 	(long);
void 	DeletePoln 		(long);
void 	DeleteShipments (void);
void 	CheckPohr 		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	OpenDB ();

	lsystemDate = TodaysDate ();

	/*
	 * Check for number of dates to hold po.
	 */
	sptr = chk_env ("PO_PURGE");
	envPoPurge = (sptr == (char *)0) ? 0L : atol (sptr);
					 
	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "POST-GOODS-RECEIPTS");
	if (!cc)
	{
		envPoPurge		= (long) delhRec.purge_days;
	}
	init_scr ();

	dsp_screen ("Deteting Costed Shipments.", comm_rec.co_no, comm_rec.co_name);

	ProcessGrin ();
	DeleteShipments ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias (pogh2, pogh);
	abc_alias (posh2, posh);
	abc_alias (poln2, poln);

	open_rec (pogh, pogh_list, POGH_NO_FIELDS, "pogh_hhgr_hash");
	open_rec (pogh2,pogh_list, POGH_NO_FIELDS, "pogh_id_no2");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (posh2,posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no2");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (poln2,poln_list, POLN_NO_FIELDS, "poln_id_no");
    open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (pogh2);
	abc_fclose (posh2);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (posh);
	abc_fclose (posl);
	abc_fclose (posd);
	abc_fclose (pogd);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (pohr);
	abc_fclose (poli);
	ArchiveClose ();
	abc_dbclose (data);
}

void
ProcessGrin (void)
{
	strcpy (pogh2_rec.co_no, comm_rec.co_no);
	sprintf (pogh2_rec.gr_no, "%15.15s", " ");
	cc = find_rec (pogh2, &pogh2_rec, GTEQ, "r");

	while (!cc && !strcmp (pogh2_rec.co_no, comm_rec.co_no))
	{
		/*-------------------
		| Goods Received	|
		-------------------*/
		if (DEL_HEAD && OLD_GRIN)
		{
			dsp_process ("GR NO", pogh2_rec.gr_no);
			ProcessPogl (pogh2_rec.hhgr_hash);
		}
		cc = find_rec (pogh2, &pogh2_rec, NEXT, "r");
	}
}

void
DeletePosh (
	long	hhshHash)
{
	/*
	 * Process through shipment file deleting posl & posd first.
	 */
	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh, &posh_rec, GTEQ, "r");
	while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no) &&
			posh_rec.hhsh_hash == hhshHash)
	{
		dsp_process ("SHIP NO", posh_rec.csm_no);

		DeletePosl (posh_rec.hhsh_hash);
		DeletePosd (posh_rec.hhsh_hash);
		DeletePogd (posh_rec.hhsh_hash);
		posh2_rec.hhsh_hash	=	posh_rec.hhsh_hash;
		cc = find_rec (posh2, &posh2_rec, COMPARISON, "u");
		if (!cc)
			abc_delete (posh2);
		
		cc = find_rec (posh, &posh_rec, NEXT, "r");
	}
}

/*
 * Routine deletes shipments that have no goods receipt data.
 */
void
DeleteShipments (void)
{
	abc_selfield (pogh, "pogh_sh_id");

	/*
	 * Process through shipment file deleting posl & posd first.
	 */
	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = 0L;
	cc = find_rec (posh, &posh_rec, GTEQ, "r");
	while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no))
	{
		if (SHIP_AT_INP)
		{
			cc = find_rec (posh, &posh_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check if a goods receipt exists.
		 */
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsh_hash	=	posh_rec.hhsh_hash;
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (cc)
		{
			dsp_process ("SHIP NO", posh_rec.csm_no);

			DeletePosl (posh_rec.hhsh_hash);
			DeletePosd (posh_rec.hhsh_hash);
			DeletePogd (posh_rec.hhsh_hash);

			posh2_rec.hhsh_hash	=	posh_rec.hhsh_hash;
			cc = find_rec (posh2, &posh2_rec, COMPARISON, "u");
			if (!cc)
				abc_delete (posh2);
		}
		cc = find_rec (posh, &posh_rec, NEXT, "r");
	}
}

/*
 * Delete shipment line items.
 */
void
DeletePosl (
	long   hhshHash)
{
	strcpy (posl_rec.co_no, comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpl_hash = 0L;

	cc = find_rec (posl, &posl_rec, GTEQ, "u");
	while (!cc && !strcmp (posl_rec.co_no, comm_rec.co_no) &&
		       posl_rec.hhsh_hash == hhshHash)
	{
		cc = abc_delete (posl);
		if (cc)
		   	file_err (cc, posl, "DBDELETE");

		cc = find_rec (posl, &posl_rec, GTEQ, "u");
	}
}

/*
 * Delete shipment Line Detail file.
 */
void
DeletePosd (
	long	hhshHash)
{
	strcpy (posd_rec.co_no, comm_rec.co_no);
	posd_rec.hhsh_hash = hhshHash;
	posd_rec.hhpo_hash = 0L;

	cc = find_rec (posd, &posd_rec, GTEQ, "u");

	while (!cc && !strcmp (posd_rec.co_no, comm_rec.co_no) &&
		       posd_rec.hhsh_hash == hhshHash)
	{
		cc = abc_delete (posd);
		if (cc)
		   	file_err (cc, posd, "DBDELETE");

		cc = find_rec (posd, &posd_rec, GTEQ, "u");
	}
}
/*
 * Delete shipment costing file.
 */
void
DeletePogd (
	long	hhshHash)
{
	abc_selfield (pogd, "pogd_id_no2");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhsh_hash = hhshHash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd, &pogd_rec, GTEQ, "u");

	while (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) &&
		       pogd_rec.hhsh_hash == hhshHash)
	{
		cc = abc_delete (pogd);
		if (cc)
		   	file_err (cc, pogd, "DBDELETE");

		cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
	}
}

void
ProcessPogl (
	long	hhgrHash)
{
	abc_selfield (pogd, "pogd_id_no");

	pogl_rec.hhgr_hash 	= hhgrHash;
	pogl_rec.line_no 	= 0;
	
	cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
	while (!cc && pogl_rec.hhgr_hash == hhgrHash)
	{
	    if (DEL_LINE)
	    {
		   	cc = abc_delete (pogl);
		   	if (cc)
		   		file_err (cc, pogl, "DBDELETE");

			DeletePoln (pogl_rec.hhpl_hash);
			cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
		}
		else
			cc = find_rec (pogl, &pogl_rec, NEXT, "u");
	}
	pogl_rec.hhgr_hash 	= hhgrHash;
	pogl_rec.line_no 	= 0;

	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	if (cc || pogl_rec.hhgr_hash != hhgrHash)
	{
		pogh_rec.hhgr_hash	=	hhgrHash;
		cc = find_rec (pogh, &pogh_rec, EQUAL, "u");
		if (!cc)
		{
			cc = abc_delete (pogh);
			if (cc)
		   		file_err (cc, pogh, "DBDELETE");

			/*
			 * Delete other shipment stuff.
			 */
			DeletePosh (pogh_rec.hhsh_hash);
		}
		pogd_rec.hhgr_hash = hhgrHash;
		pogd_rec.line_no = 0;
		cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
		while (!cc && pogd_rec.hhgr_hash == hhgrHash)
		{
			cc = abc_delete (pogd);
			if (cc)
				file_err (cc, pogd, "DBDELETE");

			pogd_rec.hhgr_hash = hhgrHash;
			pogd_rec.line_no = 0;
			cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
		}
		abc_unlock (pogd);
	}
}

/*
 * Delete Purchase order lines.
 */
void
DeletePoln (
	long	hhplHash)
{
	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, COMPARISON, "u");
	if (cc)
		return;

	if (poln_rec.qty_ord - poln_rec.qty_rec <= 0.00)
	{
	    /*
		 * Find and Delete existing poli records.
		 */
		poli_rec.hhpl_hash  =   poln_rec.hhpl_hash;                 
		cc = find_rec (poli, &poli_rec, EQUAL, "u");                 
		if (!cc)
			abc_delete (poli);

		poln_rec.due_date = pogl_rec.rec_date;
		cc = ArchivePoln (poln_rec.hhpl_hash);
		if (cc)
			file_err (cc, poln, "ARCHIVE");

		cc = abc_delete (poln);
		if (cc)
			file_err (cc, poln, "DBDELETE");

		CheckPohr (poln_rec.hhpo_hash);
	}
}

/*
 * Check if Purchase order header can be deleted.
 */
void
CheckPohr (
	long	hhpoHash)
{
	poln2_rec.hhpo_hash	=	hhpoHash;
	poln2_rec.line_no	=	0;
	cc = find_rec (poln2, &poln2_rec, GTEQ, "r");
	if (cc || poln2_rec.hhpo_hash != hhpoHash)
	{
		pohr_rec.hhpo_hash	=	hhpoHash;
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "u");
		if (cc)
			return;

		cc = ArchivePohr (hhpoHash);
		if (cc)
			file_err (cc, pohr, "ARCHIVE");

		cc = abc_delete (pohr);
		if (cc)
			file_err (cc, pohr, "DBDELETE");
	}
}
