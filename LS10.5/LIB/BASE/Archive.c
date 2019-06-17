/*
 ******************************************************************************
 *  Copyright (C) 1999 - 2002 LogisticSoftware
 ******************************************************************************
 * $Id: Archive.c,v 5.1 2002/10/09 06:01:58 robert Exp $
 * Set of Functions to archive Sales order and works order information
 * Create all necessary records for Archive data
 ******************************************************************************
 * $Log: Archive.c,v $
 * Revision 5.1  2002/10/09 06:01:58  robert
 * Modified to save archive data into text file format to make
 * it OS independent
 *
 * Revision 5.0  2002/05/07 10:12:22  scott
 * Updated to bring version number to 5.0
 *
 * Revision 1.11  2002/05/07 07:25:35  scott
 * Updated from testing
 *
 * Revision 1.10  2002/05/07 06:38:32  scott
 * Last release before testing
 *
 * Revision 1.9  2002/05/07 02:50:23  scott
 * Updated for new Archiving system
 *
 * Revision 1.8  2002/05/06 09:01:10  scott
 * More changes from testing.
 *
 *
 */

#define	TRUE	1
#define	FALSE	0

#include	<std_decs.h>
#include	<Archive.h>

#ifdef GVISION
	#include <RemoteFile.h>
	#define fopen	Remote_fopen
	#define fclose	Remote_fclose
	#define fgets	Remote_fgets
	#define fprintf	Remote_fprintf	
#endif

static	const	char 	
		*arhr	= "_arhr_archive",
		*arln	= "_arln_archive",
		*arpcwo	= "_arpcwo_archive",
		*arpohr	= "_arpohr_archive",
		*arpoln	= "_arpoln_archive",
		*arsohr	= "_arsohr_archive",
		*arsoln	= "_arsoln_archive",
		*pcwo	= "_pcwo_archive",
		*pohr	= "_pohr_archive",
		*poln	= "_poln_archive",
		*sohr	= "_sohr_archive",
		*soln	= "_soln_archive";

static	int				
		arhr_openDone	=	FALSE,
		arln_openDone	=	FALSE,
		arpcwo_openDone	=	FALSE,
		arpohr_openDone	=	FALSE,
		arpoln_openDone	=	FALSE,
		arsohr_openDone	=	FALSE,
		arsoln_openDone	=	FALSE,
		pcwo_openDone	=	FALSE,
		pohr_openDone	=	FALSE,
		poln_openDone	=	FALSE,
		sohr_openDone	=	FALSE,
		soln_openDone	=	FALSE,
		envPoArchivePo	=	FALSE,
		envPcArchiveWo	=	FALSE,
		envSoArchiveSo	=	FALSE,
		openEnvironment	=	FALSE;

#include	<ArchiveTables.h>

static	int		errReturn	=	FALSE;
static	char	bufTemp [10000];
static	char	mask [1024];

void			ArchiveClose			(void);
static	void 	ArchiveEnvironmentOpen 	(void),
				OpenArhr 				(void),
				OpenArln 				(void),
				OpenSohr 				(void),
				OpenSoln 				(void),
				OpenPcwo 				(void),
				OpenArsohr 				(void),
				OpenArsoln 				(void),
				OpenArpcwo 				(void),
				OpenPohr 				(void),
				OpenPoln 				(void),
				OpenArpohr 				(void),
				OpenArpoln 				(void);

/*
 * Archive Sales Order Header Record (sohr)
 */
int
ArchiveSohr (
	long 	hhsoHash)
{
	/*
	 * Setup internal structure for data.
	 */
	struct sohrRecord	sohrRec;
	struct arsohrRecord	arsohrRec;

	/*
	 * Open Archive environment variables required.
	 */
	 ArchiveEnvironmentOpen ();

	if (envSoArchiveSo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Sales order Header and Duplicate archive file.
	 */
	OpenSohr 	();
	OpenArsohr 	();

	/*
	 * Read sales order header.
	 */
	sohrRec.hhso_hash	=	hhsoHash;
	errReturn = find_rec (sohr, &sohrRec, EQUAL, "r");
	if (errReturn)
		return (ArchiveErr_Failed);

	/*
	 * Copy sales order header structure to archive structure.
	 */
	memcpy 
	(
		(char *) &arsohrRec, 
		(char *) &sohrRec, 
		sizeof (struct arsohrRecord)
	);

	/*
	 * Add Archive Sales Order Header.
	 */
	errReturn = abc_add (arsohr, &arsohrRec);
	if (errReturn)
		return (ArchiveErr_Failed);

	return (ArchiveErr_Ok);
}

/*
 * Archive Sales Order line Record (soln)
 */
int
ArchiveSoln (
	long 	hhslHash)
{
	/*
	 * Setup internal structure for data.
	 */
	struct solnRecord	solnRec;
	struct arsolnRecord	arsolnRec;

	/*
	 * Open Archive environment variables required.
	 */
	ArchiveEnvironmentOpen ();
	if (envSoArchiveSo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Sales order lines and Duplicate archive file.
	 */
	OpenSoln 	();
	OpenArsoln 	();

	/*
	 * Read sales order Detail.
	 */
	solnRec.hhsl_hash	=	hhslHash;
	errReturn = find_rec (soln, &solnRec, EQUAL, "r");
	if (errReturn)
		return (ArchiveErr_Failed);


	/*
	 * Copy sales order detail structure to archive structure.
	 */
	memcpy 
	(
		(char *) &arsolnRec, 
		(char *) &solnRec, 
		sizeof (struct arsolnRecord)
	);
	/*
	 * Update to ensure these are set in case of a recovery.
	 * Actual ship quantity can be found in coln or arln.
	 */
	arsolnRec.qty_order	= 0.00;
	arsolnRec.qty_bord	= 0.00;
	strcpy (arsolnRec.status, "D");
	strcpy (arsolnRec.stat_flag, "D");

	/*
	 * Add Archive Sales Order Detail.
	 */
	errReturn = abc_add (arsoln, &arsolnRec);
	if (errReturn)
		return (ArchiveErr_Failed);

	return (ArchiveErr_Ok);
}

/*
 * Archive Purchase Order Header Record (pohr)
 */
int
ArchivePohr (
	long 	hhpoHash)
{
	/*
	 * Setup internal structure for data.
	 */
	struct pohrRecord	pohrRec;
	struct arpohrRecord	arpohrRec;

	/*
	 * Open Archive environment variables required.
	 */
	 ArchiveEnvironmentOpen ();

	if (envPoArchivePo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Sales order Header and Duplicate archive file.
	 */
	OpenPohr 	();
	OpenArpohr 	();

	/*
	 * Read Purchase Order Header.
	 */
	pohrRec.hhpo_hash	=	hhpoHash;
	errReturn = find_rec (pohr, &pohrRec, EQUAL, "r");
	if (errReturn)
		return (ArchiveErr_Failed);

	/*
	 * Copy Purchase Order Header structure to archive structure.
	 */
	memcpy 
	(
		(char *) &arpohrRec, 
		(char *) &pohrRec, 
		sizeof (struct arpohrRecord)
	);

	/*
	 * Add Archive Purchase Order Header.
	 */
	errReturn = abc_add (arpohr, &arpohrRec);
	if (errReturn)
		return (ArchiveErr_Failed);

	return (ArchiveErr_Ok);
}

/*
 * Archive Purchase Order line Record (poln)
 */
int
ArchivePoln (
	long 	hhplHash)
{
	/*
	 * Setup internal structure for data.
	 */
	struct polnRecord	polnRec;
	struct arpolnRecord	arpolnRec;

	/*
	 * Open Archive environment variables required.
	 */
	ArchiveEnvironmentOpen ();

	if (envPoArchivePo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Sales order lines and Duplicate archive file.
	 */
	OpenPoln 	();
	OpenArpoln 	();

	/*
	 * Read sales order Detail.
	 */
	polnRec.hhpl_hash	=	hhplHash;
	errReturn = find_rec (poln, &polnRec, EQUAL, "r");
	if (errReturn)
		return (ArchiveErr_Failed);

	/*
	 * Copy Purchase Order Detail structure to archive structure.
	 */
	memcpy 
	(
		(char *) &arpolnRec, 
		(char *) &polnRec, 
		sizeof (struct arpolnRecord)
	);
	/*
	 * Update to ensure these are set in case of a recovery.
	 */
	strcpy (arpolnRec.status, "D");
	strcpy (arpolnRec.stat_flag, "D");

	/*
	 * Add Archive Purchase Order Detail.
	 */
	errReturn = abc_add (arpoln, &arpolnRec);
	if (errReturn)
		return (ArchiveErr_Failed);

	return (ArchiveErr_Ok);
}

/*
 * Archive Works order. (pcwo)
 */
int
ArchivePcwo (
	long 	hhwoHash)
{
	/*
	 * Setup internal structure for data.
	 */
	struct pcwoRecord	pcwoRec;
	struct arpcwoRecord	arpcwoRec;

	/*
	 * Open Archive environment variables required.
	 */
	ArchiveEnvironmentOpen ();
	if (envPcArchiveWo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Works order header and details and archive versions.
	 */
	OpenPcwo 	();
	OpenArpcwo 	();

	/*
	 * Read Works Order Header
	 */
	pcwoRec.hhwo_hash	=	hhwoHash;
	errReturn = find_rec (pcwo, &pcwoRec, EQUAL, "r");
	if (errReturn)
		return (ArchiveErr_Failed);

	/*
	 * Copy works order header structure to archive structure.
	 */
	memcpy 
	(
		(char *) &arpcwoRec, 
		(char *) &pcwoRec, 
		sizeof (struct arpcwoRecord)
	);
	/*
	 * Add Archive works order Header.
	 */
	errReturn = abc_add (arpcwo, &arpcwoRec);
	if (errReturn)
		return (ArchiveErr_Failed);

	return (ArchiveErr_Ok);
}
/*
 * Download and Delete Archived Sales Order Records (arsohr/arsoln)
 */
int
DownloadDeleteSO (
	char	*fileName,
	long 	hhsoHash)
{
	char	arsohrFileName	[255],
			arsolnFileName	[255];

	FILE	*fpHdr, *fpDtl;
	
	/*
	 * Setup internal structure for data.
	 */
	struct arsohrRecord	arsohrRec;
	struct arsolnRecord	arsolnRec;

	sprintf (arsohrFileName, "%s.arsohr.dat", fileName);
	sprintf (arsolnFileName, "%s.arsoln.dat", fileName);

	/*
	 * Open Archive environment variables required.
	 */
	 ArchiveEnvironmentOpen ();

	if (envSoArchiveSo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Sales order Header and Detail archive files.
	 */
	OpenArsohr 	();
	OpenArsoln 	();

	/*
	 * Open archive file to write Archive Header information.
	 */
	if ((fpHdr = fopen (arsohrFileName, "a")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Open archive file to write Archive Detail information.
	 */
	 if ((fpDtl = fopen (arsolnFileName, "a")) == NULL)
	 {
	 	fclose (fpHdr);
	 	return (ArchiveOpenError);
	 }

	/*
	 * Read Archive sales order Lines.
	 */
	arsolnRec.hhso_hash	=	hhsoHash;
	arsolnRec.line_no	=	0;
	errReturn = find_rec (arsoln, &arsolnRec, GTEQ, "u");
	
	strcpy (mask, "%ld\x04%d\x04%ld\x04%ld\x04%ld\x04%ld\x04%s\x04%d\x04%f\x04%f\x04%f\x04%lf\x04%lf\x04");
	strcat (mask, "%lf\x04%lf\x04%f\x04%f\x04%f\x04%f\x04%f\x04%d\x04%f\x04%f\x04%f\x04%f\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%s\x04%ld\x04%d\x04%s\x04%s\x04%ld\x04%s\x04%s\x04\n");
	
	while (!errReturn && arsolnRec.hhso_hash == hhsoHash)
	{
		/*
	 	 * Add Archive Sales Order Line(s).
	 	 */
	 	errReturn = selfprintf (fpDtl, mask,
			arsolnRec.hhso_hash,
			arsolnRec.line_no,
			arsolnRec.hhbr_hash,
			arsolnRec.hhcc_hash,
			arsolnRec.hhum_hash,
			arsolnRec.hhsl_hash,
			arsolnRec.serial_no,
			arsolnRec.cont_status,
			arsolnRec.qty_order,
			arsolnRec.qty_bord,
			arsolnRec.qty_org_ord,
			arsolnRec.gsale_price,
			arsolnRec.sale_price,
			arsolnRec.cost_price,
			arsolnRec.item_levy,
			arsolnRec.dis_pc,
			arsolnRec.reg_pc,
			arsolnRec.disc_a,
			arsolnRec.disc_b,
			arsolnRec.disc_c,
			arsolnRec.cumulative,
			arsolnRec.tax_pc,
			arsolnRec.gst_pc,
			arsolnRec.o_xrate,
			arsolnRec.n_xrate,
			arsolnRec.pack_size,
			arsolnRec.sman_code,
			arsolnRec.cus_ord_ref,
			arsolnRec.pri_or,
			arsolnRec.dis_or,
			arsolnRec.item_desc,
			arsolnRec.due_date,
			arsolnRec.del_no,
			arsolnRec.bonus_flag,
			arsolnRec.hide_flag,
			arsolnRec.hhah_hash,
			arsolnRec.status,
			arsolnRec.stat_flag);

		if (errReturn < 0)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveAddError);
		}

		errReturn = abc_delete (arsoln);
		if (errReturn)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveDeleteError);
		}

		arsolnRec.hhso_hash	=	hhsoHash;
		arsolnRec.line_no	=	0;
		errReturn = find_rec (arsoln, &arsolnRec, GTEQ, "u");
	}
	abc_unlock (arsoln);

	/*
	 * Read archive sales order header.
	 */
	arsohrRec.hhso_hash	=	hhsoHash;
	errReturn = find_rec (arsohr, &arsohrRec, EQUAL, "u");
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveFindError);
	}

	/*
	 * Add Archive Sales Order Header.
	 */
	strcpy (mask, "%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%ld\x04%s\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%d\x04%f\x04%s\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%lf\x04%lf\x04%s\x04%lf\x04%lf\x04%lf\x04");
	strcat (mask, "%lf\x04%lf\x04%lf\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%s\x04%s\x04\n");
	
	errReturn = selfprintf (fpHdr, mask,
		arsohrRec.co_no,
		arsohrRec.br_no,
		arsohrRec.dp_no,
		arsohrRec.order_no,
		arsohrRec.cont_no,
		arsohrRec.hhcu_hash,
		arsohrRec.chg_hhcu,
		arsohrRec.hhso_hash,
		arsohrRec.inv_no,
		arsohrRec.cus_ord_ref,
		arsohrRec.chg_ord_ref,
		arsohrRec.op_id,
		arsohrRec.time_create,
		arsohrRec.date_create,
		arsohrRec.cons_no,
		arsohrRec.del_zone,
		arsohrRec.del_req,
		arsohrRec.del_date,
		arsohrRec.asm_req,
		arsohrRec.asm_date,
		arsohrRec.s_timeslot,
		arsohrRec.e_timeslot,
		arsohrRec.carr_code,
		arsohrRec.carr_area,
		arsohrRec.no_cartons,
		arsohrRec.no_kgs,
		arsohrRec.sch_ord,
		arsohrRec.ord_type,
		arsohrRec.pri_type,
		arsohrRec.frei_req,
		arsohrRec.dt_raised,
		arsohrRec.dt_required,
		arsohrRec.tax_code,
		arsohrRec.tax_no,
		arsohrRec.area_code,
		arsohrRec.sman_code,
		arsohrRec.sell_terms,
		arsohrRec.pay_term,
		arsohrRec.freight,
		arsohrRec.insurance,
		arsohrRec.ins_det,
		arsohrRec.o_cost_1,
		arsohrRec.o_cost_2,
		arsohrRec.o_cost_3,
		arsohrRec.deposit,
		arsohrRec.discount,
		arsohrRec.exch_rate,
		arsohrRec.fix_exch,
		arsohrRec.batch_no,
		arsohrRec.cont_name,
		arsohrRec.cont_phone,
		arsohrRec.del_name,
		arsohrRec.del_add1,
		arsohrRec.del_add2,
		arsohrRec.del_add3,
		arsohrRec.din_1,
		arsohrRec.din_2,
		arsohrRec.din_3,
		arsohrRec.new,
		arsohrRec.prt_price,
		arsohrRec.full_supply,
		arsohrRec.two_step,
		arsohrRec.status,
		arsohrRec.stat_flag);

	if (errReturn < 0)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveAddError);
	}

	errReturn = abc_delete (arsohr);
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveDeleteError);
	}

	fclose (fpHdr);
	fclose (fpDtl);

	return (ArchiveErr_Ok);
}
/*
 * Upload sales Orders from Disk based Archive.
 */
int
UploadSO (
	char	*fileName,
	long 	hhsoHash)
{
	char	arsohrFileName	[255],
			arsolnFileName	[255];
	
	FILE *fpHdr, *fpDtl;

	/*
	 * Setup internal structure for data.
	 */
	struct arsohrRecord	arsohrRec;
	struct arsolnRecord	arsolnRec;

	/* 
	 * Create origional system file names
	 */
	sprintf (arsohrFileName, "%s.arsohr.dat", fileName);
	sprintf (arsolnFileName, "%s.arsoln.dat", fileName);

	/*
	 * Open Sales Order Header and Detail archive files.
	 */
	OpenArsohr 	();
	OpenArsoln 	();

	/*
	 * Open disk based archive file (Archive Header).
	 */
	if ((fpHdr = fopen (arsohrFileName, "r")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive purchase order header.
	 */	
	while (!ArSohrRead (fpHdr, &arsohrRec))
	{
		if (arsohrRec.hhso_hash	== hhsoHash)
		{
			if (find_rec (arsohr, &arsohrRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arsohr, &arsohrRec);
				if (errReturn)
				{
					fclose (fpHdr);
					return (ArchiveAddError);
				}
			}
		}
	}
	fclose (fpHdr);

	/*
	 * Open disk based archive file to reading Archive Detail information.
	 */
	if ((fpDtl = fopen (arsolnFileName, "r")) == NULL)	
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive Sales Order Lines.
	 */
	while (!ArSolnRead (fpDtl, &arsolnRec))
	{
		if (arsolnRec.hhso_hash	== hhsoHash)
		{
			if (find_rec (arsoln, &arsolnRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arsoln, &arsolnRec);
				if (errReturn)
				{
					fclose (fpDtl);
					return (ArchiveAddError);
				}
			}
		}
	}

	fclose (fpDtl);

	return (ArchiveErr_Ok);
}
/*
 * Download and Delete Archived Invoice Records (arhr/arln)
 */
int
DownloadDeleteIN (
	char	*fileName,
	long 	hhcoHash)
{
	char	arhrFileName	[255],
			arlnFileName	[255];

	FILE	*fpHdr, *fpDtl;

	/*
	 * Setup internal structure for data.
	 */
	struct arhrRecord	arhrRec;
	struct arlnRecord	arlnRec;

	sprintf (arhrFileName, "%s.arhr.dat", fileName);
	sprintf (arlnFileName, "%s.arln.dat", fileName);

	/*
	 * Open Archive Invoice Header and Details archive files.
	 */
	OpenArhr ();
	OpenArln ();

	/*
	 * Open archive file to write Archive Header information.
	 */
	if ((fpHdr = fopen (arhrFileName, "a")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Open archive file to write Archive Detail information.
	 */
	 if ((fpDtl = fopen (arlnFileName, "a")) == NULL)
	 {
	 	fclose (fpHdr);
	 	return (ArchiveOpenError);
	 }

	/*
	 * Read Archive sales order Lines.
	 */
	arlnRec.hhco_hash	=	hhcoHash;
	arlnRec.line_no		=	0;
	errReturn = find_rec (arln, &arlnRec, GTEQ, "u");
	
	strcpy (mask, "%ld\x04%ld\x04%d\x04%ld\x04%ld\x04%ld\x04%ld\x04%s\x04%ld\x04%s\x04%s\x04%d\x04%f\x04");
	strcat (mask, "%f\x04%f\x04%f\x04%f\x04%lf\x04%lf\x04%lf\x04%lf\x04%f\x04%f\x04%f\x04%f\x04%f\x04%d\x04");
	strcat (mask, "%f\x04%f\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%s\x04%s\x04%s\x04%s\x04%f\x04");
	strcat (mask, "%f\x04%s\x04%ld\x04%s\x04%s\x04%s\x04%ld\x04%s\x04%s\x04%ld\x04\n");

	while (!errReturn && arlnRec.hhco_hash == hhcoHash)
	{
		/*
	 	 * Add Archive Sales Order Line(s).
	 	 */
		errReturn = selfprintf (fpDtl, mask,
			arlnRec.hhcl_hash,
			arlnRec.hhco_hash,
			arlnRec.line_no,
			arlnRec.hhbr_hash,
			arlnRec.incc_hash,
			arlnRec.hhum_hash,
			arlnRec.hhsl_hash,
			arlnRec.order_no,
			arlnRec.hhdl_hash,
			arlnRec.crd_type,
			arlnRec.serial_no,
			arlnRec.cont_status,
			arlnRec.qty_org_ord,
			arlnRec.q_order,
			arlnRec.qty_del,
			arlnRec.qty_ret,
			arlnRec.q_backorder,
			arlnRec.gsale_price,
			arlnRec.sale_price,
			arlnRec.cost_price,
			arlnRec.item_levy,
			arlnRec.disc_pc,
			arlnRec.reg_pc,
			arlnRec.disc_a,
			arlnRec.disc_b,
			arlnRec.disc_c,
			arlnRec.cumulative,
			arlnRec.tax_pc,
			arlnRec.gst_pc,
			arlnRec.gross,
			arlnRec.freight,
			arlnRec.on_cost,
			arlnRec.amt_disc,
			arlnRec.amt_tax,
			arlnRec.amt_gst,
			arlnRec.erate_var,
			arlnRec.pack_size,
			arlnRec.sman_code,
			arlnRec.cus_ord_ref,
			arlnRec.org_ord_ref,
			arlnRec.o_xrate,
			arlnRec.n_xrate,
			arlnRec.item_desc,
			arlnRec.due_date,
			arlnRec.status,
			arlnRec.bonus_flag,
			arlnRec.hide_flag,
			arlnRec.hhah_hash,
			arlnRec.price_type,
			arlnRec.stat_flag,
			arlnRec.hhwo_hash);

		if (errReturn < 0)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveAddError);
		}

		errReturn = abc_delete (arln);
		if (errReturn)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveDeleteError);
		}

		arlnRec.hhco_hash	=	hhcoHash;
		arlnRec.line_no	=	0;
		errReturn = find_rec (arln, &arlnRec, GTEQ, "u");
	}
	abc_unlock (arln);

	/*
	 * Read Archive Invoice Header.
	 */
	arhrRec.hhco_hash	=	hhcoHash;
	errReturn = find_rec (arhr, &arhrRec, EQUAL, "u");
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveFindError);
	}

	/*
	 * Add Archive Invoice Header.
	 */
	strcpy (mask, "%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04%ld\x04%s\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%s\x04%ld\x04%s\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%d\x04%lf\x04");
	strcat (mask, "%f\x04%ld\x04%ld\x04%s\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04%lf\x04");
	strcat (mask, "%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%s\x04%s\x04%d\x04%s\x04%s\x04%s\x04%ld\x04%s\x04%ld\x04%s\x04%d\x04\n");

	errReturn = selfprintf (fpHdr, mask,
		arhrRec.co_no,
		arhrRec.br_no,
		arhrRec.dp_no,
		arhrRec.inv_no,
		arhrRec.app_inv_no,
		arhrRec.hhcu_hash,
		arhrRec.chg_hhcu_hash,
		arhrRec.type,
		arhrRec.cont_no,
		arhrRec.drop_ship,
		arhrRec.hhds_hash,
		arhrRec.cus_ord_ref,
		arhrRec.chg_ord_ref,
		arhrRec.ord_ref,
		arhrRec.grn_no,
		arhrRec.cons_no,
		arhrRec.del_zone,
		arhrRec.del_req,
		arhrRec.del_date,
		arhrRec.asm_req,
		arhrRec.asm_date,
		arhrRec.asm_hash,
		arhrRec.s_timeslot,
		arhrRec.e_timeslot,
		arhrRec.carr_code,
		arhrRec.carr_area,
		arhrRec.no_cartons,
		arhrRec.wgt_per_ctn,
		arhrRec.no_kgs,
		arhrRec.hhso_hash,
		arhrRec.hhco_hash,
		arhrRec.frei_req,
		arhrRec.date_raised,
		arhrRec.date_required,
		arhrRec.tax_code,
		arhrRec.tax_no,
		arhrRec.area_code,
		arhrRec.sale_code,
		arhrRec.op_id,
		arhrRec.time_create,
		arhrRec.date_create,
		arhrRec.gross,
		arhrRec.freight,
		arhrRec.insurance,
		arhrRec.other_cost_1,
		arhrRec.other_cost_2,
		arhrRec.other_cost_3,
		arhrRec.tax,
		arhrRec.gst,
		arhrRec.disc,
		arhrRec.deposit,
		arhrRec.ex_disc,
		arhrRec.erate_var,
		arhrRec.sos,
		arhrRec.item_levy,
		arhrRec.exch_rate,
		arhrRec.fix_exch,
		arhrRec.batch_no,
		arhrRec.dl_name,
		arhrRec.dl_add1,
		arhrRec.dl_add2,
		arhrRec.dl_add3,
		arhrRec.din_1,
		arhrRec.din_2,
		arhrRec.din_3,
		arhrRec.pay_type,
		arhrRec.pay_terms,
		arhrRec.sell_terms,
		arhrRec.ins_det,
		arhrRec.pri_type,
		arhrRec.pri_break,
		arhrRec.ord_type,
		arhrRec.prt_price,
		arhrRec.status,
		arhrRec.stat_flag,
		arhrRec.ps_print,
		arhrRec.ps_print_no,
		arhrRec.inv_print,
		arhrRec.ccn_print,
		arhrRec.printing,
		arhrRec.hhtr_hash,
		arhrRec.load_flag,
		arhrRec.wrmr_hash,
		arhrRec.pos_inv_no,
		arhrRec.pos_tran_type);

	if (errReturn < 0)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveAddError);
	}

	errReturn = abc_delete (arhr);
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveDeleteError);
	}

	fclose (fpHdr);
	fclose (fpDtl);

	return (ArchiveErr_Ok);
}
/*
 * Upload Invoices/Credits from Disk based Archived.
 */
int
UploadIN (
	char	*fileName,
	long 	hhcoHash)
{
	char	arhrFileName	[255],
			arlnFileName	[255];

	FILE *fpHdr, *fpDtl;

	/*
	 * Setup internal structure for data.
	 */
	struct arhrRecord	arhrRec;
	struct arlnRecord	arlnRec;

	/* 
	 * Create origional system file names
	 */
	sprintf (arhrFileName, "%s.arhr.dat", fileName);
	sprintf (arlnFileName, "%s.arln.dat", fileName);

	/*
	 * Open Invoice Header and Detail archive files.
	 */
	OpenArhr	();
	OpenArln 	();

	/*
	 * Open disk based archive file to reading Archive Header information.
	 */
	if ((fpHdr = fopen (arhrFileName, "r")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive Invoice header.
	 */
	while (!ArhrRead (fpHdr , &arhrRec))
	{
		if (arhrRec.hhco_hash	== hhcoHash)
		{
			if (find_rec (arhr, &arhrRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arhr, &arhrRec);
				if (errReturn)
				{
					fclose (fpHdr);
					return (ArchiveAddError);
				}
			}
		}
	}
	fclose (fpHdr);

	/*
	 * Open disk based archive file to reading Archive Detail information.
	 */
	if ((fpDtl = fopen (arlnFileName, "r")) == NULL)	
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive purchase order lines.
	 */
	while (!ArlnRead (fpDtl, &arlnRec))
	{
		if (arlnRec.hhco_hash	== hhcoHash)
		{
			if (find_rec (arln, &arlnRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arln, &arlnRec);
				if (errReturn)
				{
					fclose (fpDtl);
					return (ArchiveAddError);
				}
			}
		}
	}
	fclose (fpDtl);
	return (ArchiveErr_Ok);
}
/*
 * Download and Delete Archived Purchase Order Records (arpohr/arpoln)
 */
int
DownloadDeletePO (
	char	*fileName,
	long 	hhpoHash)
{
	char	arpohrFileName	[255],
			arpolnFileName	[255];

	FILE	*fpHdr, *fpDtl;

	/*
	 * Setup internal structure for data.
	 */
	struct arpohrRecord	arpohrRec;
	struct arpolnRecord	arpolnRec;

	sprintf (arpohrFileName, "%s.arpohr.dat", fileName);
	sprintf (arpolnFileName, "%s.arpoln.dat", fileName);

	/*
	 * Open Archive environment variables required.
	 */
	 ArchiveEnvironmentOpen ();

	if (envPoArchivePo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Purchase Order Header and Detail archive files.
	 */
	OpenArpohr 	();
	OpenArpoln 	();

	/*
	 * Open archive file to write Archive Header information.
	 */
	if ((fpHdr = fopen (arpohrFileName, "a")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Open archive file to write Archive Detail information.
	 */
	if ((fpDtl = fopen (arpolnFileName, "a")) == NULL)
	{
		fclose (fpHdr);
		return (ArchiveOpenError);
	}

	/*
	 * Read Archive sales order Lines.
	 */
	arpolnRec.hhpo_hash	=	hhpoHash;
	arpolnRec.line_no	=	0;
	errReturn = find_rec (arpoln, &arpolnRec, GTEQ, "u");

	strcpy (mask, "%ld\x04%d\x04%ld\x04%ld\x04%ld\x04%ld\x04%ld\x04%ld\x04%lf\x04%s\x04%s\x04%s\x04%f\x04%f\x04");
	strcat (mask, "%f\x04%f\x04%f\x04%f\x04%f\x04%f\x04%f\x04%f\x04%d\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04%lf\x04");
	strcat (mask, "%lf\x04%lf\x04%s\x04%s\x04%ld\x04%d\x04%ld\x04%ld\x04%ld\x04%s\x04%s\x04%s\x04\n");

	while (!errReturn && arpolnRec.hhpo_hash == hhpoHash)
	{
		/*
	 	 * Add Archive Sales Order Line(s).
	 	 */
		errReturn = selfprintf (fpDtl, mask,
			arpolnRec.hhpo_hash,
			arpolnRec.line_no,
			arpolnRec.hhbr_hash,
			arpolnRec.hhum_hash,
			arpolnRec.hhcc_hash,
			arpolnRec.hhlc_hash,
			arpolnRec.hhpl_hash,
			arpolnRec.hhpl_orig,
			arpolnRec.exch_rate,
			arpolnRec.serial_no,
			arpolnRec.container,
			arpolnRec.cus_ord_ref,
			arpolnRec.qty_ord,
			arpolnRec.qty_rec,
			arpolnRec.pack_qty,
			arpolnRec.chg_wgt,
			arpolnRec.gross_wgt,
			arpolnRec.cu_metre,
			arpolnRec.reg_pc,
			arpolnRec.disc_a,
			arpolnRec.disc_b,
			arpolnRec.disc_c,
			arpolnRec.cumulative,
			arpolnRec.grs_fgn_cst,
			arpolnRec.fob_fgn_cst,
			arpolnRec.fob_nor_cst,
			arpolnRec.frt_ins_cst,
			arpolnRec.duty,
			arpolnRec.licence,
			arpolnRec.lcost_load,
			arpolnRec.land_cst,
			arpolnRec.cat_code,
			arpolnRec.item_desc,
			arpolnRec.ship_no,
			arpolnRec.case_no,
			arpolnRec.hhso_hash,
			arpolnRec.due_date,
			arpolnRec.fwd_date,
			arpolnRec.pur_status,
			arpolnRec.status,
			arpolnRec.stat_flag);

		if (errReturn < 0)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveAddError);
		}

		errReturn = abc_delete (arpoln);
		if (errReturn)
		{
			fclose (fpHdr);
			fclose (fpDtl);
			return (ArchiveDeleteError);
		}

		arpolnRec.hhpo_hash	=	hhpoHash;
		arpolnRec.line_no		=	0;
		errReturn = find_rec (arpoln, &arpolnRec, GTEQ, "u");
	}
	abc_unlock (arpoln);

	/*
	 * Read archive sales order header.
	 */
	arpohrRec.hhpo_hash	=	hhpoHash;
	errReturn = find_rec (arpohr, &arpohrRec, EQUAL, "u");
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveFindError);
	}

	strcpy (mask, "%s\x04%s\x04%s\x04%ld\x04%s\x04%ld\x04%ld\x04%ld\x04%ld\x04%ld\x04%ld\x04%s\x04%s\x04");
	strcat (mask, "%s\x04%s\x04%ld\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04%ld\x04%ld\x04");
	strcat (mask, "%s\x04%lf\x04%s\x04%s\x04%s\x04%ld\x04%lf\x04%lf\x04%s\x04%s\x04%s\x04%s\x04%s\x04\n");

	/*
	 * Add Archive Purchase Order Header.
	 */
	errReturn = selfprintf (fpHdr, mask,
		arpohrRec.co_no,
		arpohrRec.br_no,
		arpohrRec.type,
		arpohrRec.hhsu_hash,
		arpohrRec.pur_ord_no,
		arpohrRec.hhpo_hash,
		arpohrRec.hhsh_hash,
		arpohrRec.hhdd_hash,
		arpohrRec.date_raised,
		arpohrRec.due_date,
		arpohrRec.conf_date,
		arpohrRec.contact,
		arpohrRec.app_code,
		arpohrRec.op_id,
		arpohrRec.time_create,
		arpohrRec.date_create,
		arpohrRec.req_usr,
		arpohrRec.reason,
		arpohrRec.stdin1,
		arpohrRec.stdin2,
		arpohrRec.stdin3,
		arpohrRec.delin1,
		arpohrRec.delin2,
		arpohrRec.delin3,
		arpohrRec.ship1_no,
		arpohrRec.ship2_no,
		arpohrRec.ship3_no,
		arpohrRec.curr_code,
		arpohrRec.curr_rate,
		arpohrRec.term_order,
		arpohrRec.sup_trm_pay,
		arpohrRec.bnk_trm_pay,
		arpohrRec.pay_date,
		arpohrRec.fgn_total,
		arpohrRec.fgn_ostand,
		arpohrRec.ship_method,
		arpohrRec.drop_ship,
		arpohrRec.status,
		arpohrRec.stat_flag,
		arpohrRec.sup_type);

	if (errReturn < 0)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveAddError);
	}

	errReturn = abc_delete (arpohr);
	if (errReturn)
	{
		fclose (fpHdr);
		fclose (fpDtl);
		return (ArchiveDeleteError);
	}

	fclose (fpHdr);
	fclose (fpDtl);

	return (ArchiveErr_Ok);
}
/*
 * Upload Purchase orders from Disk based Archived.
 */
int
UploadPO (
	char	*fileName,
	long 	hhpoHash)
{
	char	arpohrFileName	[255],
			arpolnFileName	[255];

	FILE	*fpHdr, *fpDtl;

	/*
	 * Setup internal structure for data.
	 */
	struct arpohrRecord	arpohrRec;
	struct arpolnRecord	arpolnRec;

	/* 
	 * Create origional system file names
	 */
	sprintf (arpohrFileName, "%s.arpohr.dat", fileName);
	sprintf (arpolnFileName, "%s.arpoln.dat", fileName);

	/*
	 * Open Purchase Order Header and Detail archive files.
	 */
	OpenArpohr 	();
	OpenArpoln 	();

	/*
	 * Open disk based archive file to reading Archive Header information.
	 */
	if ((fpHdr = fopen (arpohrFileName, "r")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive purchase order header.
	 */
	while (!ArPohrRead (fpHdr, &arpohrRec))
	{
		if (arpohrRec.hhpo_hash	== hhpoHash)
		{
			if (find_rec (arpohr, &arpohrRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arpohr, &arpohrRec);
				if (errReturn)
				{
					fclose (fpHdr);
					return (ArchiveAddError);
				}
			}
		}
	}
	fclose (fpHdr);

	/*
	 * Open archive file to reading Archive Detail information.
	 */
	if ((fpDtl = fopen (arpolnFileName, "r")) == NULL)	
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive purchase order lines.
	 */
	while (!ArPolnRead (fpDtl, &arpolnRec))
	{
		if (arpolnRec.hhpo_hash	== hhpoHash)
		{
			if (find_rec (arpoln, &arpolnRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arpoln, &arpolnRec);
				if (errReturn)
				{
					fclose (fpDtl);
					return (ArchiveAddError);
				}
			}
		}
	}
	fclose (fpDtl);
	return (ArchiveErr_Ok);
}
/*
 * Download and Delete Archived Works Order Records (arpcwo)
 */
int
DownloadDeleteWO (
	char	*fileName,
	long 	hhwoHash)
{
	char	arpcwoFileName	[255];

	FILE *fp;

	/*
	 * Setup internal structure for data.
	 */
	struct arpcwoRecord	arpcwoRec;

	sprintf (arpcwoFileName, "%s.arpcwo.dat", fileName);

	/*
	 * Open Archive environment variables required.
	 */
	 ArchiveEnvironmentOpen ();

	if (envPcArchiveWo == FALSE)
		return (ArchiveErr_Ok);

	/*
	 * Open Works Order archive file.
	 */
	OpenArpcwo 	();

	/*
	 * Open archive file to write Archive Header information.
	 */
	if ((fp = fopen (arpcwoFileName, "a")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Read archive works order.
	 */
	arpcwoRec.hhwo_hash	=	hhwoHash;
	errReturn = find_rec (arpcwo, &arpcwoRec, EQUAL, "u");
	if (errReturn)
	{
		fclose (fp);
		return (ArchiveFindError);
	}

	/*
	 * Add Archive Works Order.
	 */
	strcpy (mask, "%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%s\x04%ld\x04%ld\x04%d\x04%d\x04%s\x04%s\x04");
	strcat (mask, "%ld\x04%ld\x04%ld\x04%d\x04%d\x04%ld\x04%f\x04%f\x04%f\x04%s\x04%s\x04%ld\x04%s\x04\n");
	
	errReturn = selfprintf (fp, mask,
		arpcwoRec.co_no,
		arpcwoRec.br_no,
		arpcwoRec.wh_no,
		arpcwoRec.order_no,
		arpcwoRec.req_br_no,
		arpcwoRec.req_wh_no,
		arpcwoRec.rec_br_no,
		arpcwoRec.rec_wh_no,
		arpcwoRec.hhwo_hash,
		arpcwoRec.reqd_date,
		arpcwoRec.rtg_seq,
		arpcwoRec.priority,
		arpcwoRec.op_id,
		arpcwoRec.create_time,
		arpcwoRec.create_date,
		arpcwoRec.mfg_date,
		arpcwoRec.hhbr_hash,
		arpcwoRec.bom_alt,
		arpcwoRec.rtg_alt,
		arpcwoRec.hhcc_hash,
		arpcwoRec.prod_qty,
		arpcwoRec.act_prod_q,
		arpcwoRec.act_rej_q,
		arpcwoRec.order_stat,
		arpcwoRec.batch_no,
		arpcwoRec.hhsl_hash,
		arpcwoRec.stat_flag);

	if (errReturn < 0)
	{
		fclose (fp);
		return (ArchiveAddError);
	}

	errReturn = abc_delete (arpcwo);
	if (errReturn)
	{
		fclose (fp);
		return (ArchiveDeleteError);
	}

	fclose (fp);

	return (ArchiveErr_Ok);
}
/*
 * Upload Works Orders from Disk based Archived.
 */
int
UploadWO (
	char	*fileName,
	long 	hhwoHash)
{
	char	arpcwoFileName	[255];

	FILE	*fp;

	/*
	 * Setup internal structure for data.
	 */
	struct arpcwoRecord	arpcwoRec;

	/* 
	 * Create origional system file names
	 */
	sprintf (arpcwoFileName, "%s.arpcwo.dat", fileName);

	/*
	 * Open Works order archive file.
	 */
	OpenArpcwo	();

	/*
	 * Open disk based archive file to reading Archive information.
	 */
	if ((fp = fopen (arpcwoFileName, "r")) == NULL)
		return (ArchiveOpenError);

	/*
	 * Process Disk based Archive Works orders.
	 */
	while (!ArPcwoRead (fp, &arpcwoRec))
	{
		if (arpcwoRec.hhwo_hash	== hhwoHash)
		{
			if (find_rec (arpcwo, &arpcwoRec, COMPARISON, "r"))
			{
				errReturn = abc_add (arpcwo, &arpcwoRec);
				if (errReturn)
				{
					fclose (fp);
					return (ArchiveAddError);
				}
			}
		}
	}

	fclose (fp);
	return (ArchiveErr_Ok);
}

/*
 * Close archive tables.
 */
void
ArchiveClose (void)
{  
	if (sohr_openDone)
		abc_fclose (sohr);

	if (soln_openDone)
		abc_fclose (soln);

	if (pcwo_openDone)
		abc_fclose (pcwo);

	if (arsohr_openDone)
		abc_fclose (arsohr);

	if (arsoln_openDone)
		abc_fclose (arsoln);

	if (arpcwo_openDone)
		abc_fclose (arpcwo);

	if (pohr_openDone)
		abc_fclose (pohr);

	if (poln_openDone)
		abc_fclose (poln);

	if (arpohr_openDone)
		abc_fclose (arpohr);

	if (arpoln_openDone)
		abc_fclose (arpoln);
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArhr (void)
{
	if (arhr_openDone	==	FALSE)
	{
		abc_alias (arhr, "arhr");
		open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_hhco_hash");
		arhr_openDone	=	TRUE;
	}
}
/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArln (void)
{
	if (arln_openDone	==	FALSE)
	{
		abc_alias (arln, "arln");
		open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
		arln_openDone	=	TRUE;
	}
}
/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenSohr (void)
{
	if (sohr_openDone	==	FALSE)
	{
		abc_alias (sohr, "sohr");
		open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
		sohr_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenSoln (void)
{
	if (soln_openDone	==	FALSE)
	{
		abc_alias (soln, "soln");
		open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
		soln_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenPcwo (void)
{
	if (pcwo_openDone	==	FALSE)
	{
		abc_alias (pcwo, "pcwo");
		open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
		pcwo_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArsohr (void)
{
	if (arsohr_openDone	==	FALSE)
	{
		abc_alias (arsohr, "arsohr");
		open_rec (arsohr, arsohr_list, ARSOHR_NO_FIELDS, "arsohr_hhso_hash");
		arsohr_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArsoln (void)
{
	if (arsoln_openDone	==	FALSE)
	{
		abc_alias (arsoln, "arsoln");
		open_rec (arsoln, arsoln_list, ARSOLN_NO_FIELDS, "arsoln_id_no");
		arsoln_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArpcwo (void)
{
	if (arpcwo_openDone	==	FALSE)
	{
		abc_alias (arpcwo, "arpcwo");
		open_rec (arpcwo, arpcwo_list, ARPCWO_NO_FIELDS, "arpcwo_hhwo_hash");
		arpcwo_openDone	=	TRUE;
	}
}

/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenPohr (void)
{
	if (pohr_openDone	==	FALSE)
	{
		abc_alias (pohr, "pohr");
		open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
		pohr_openDone	=	TRUE;
	}
}
/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenPoln (void)
{
	if (poln_openDone	==	FALSE)
	{
		abc_alias (poln, "poln");
		open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
		poln_openDone	=	TRUE;
	}
}
/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArpohr (void)
{
	if (arpohr_openDone	==	FALSE)
	{
		abc_alias (arpohr, "arpohr");
		open_rec (arpohr, arpohr_list, ARPOHR_NO_FIELDS, "arpohr_hhpo_hash");
		arpohr_openDone	=	TRUE;
	}
}
/*
 * Open alias for table, open table and set open variable to TRUE.
 */
static	void	
OpenArpoln (void)
{
	if (arpoln_openDone	==	FALSE)
	{
		abc_alias (arpoln, "arpoln");
		open_rec (arpoln, arpoln_list, ARPOLN_NO_FIELDS, "arpoln_id_no");
		arpoln_openDone	=	TRUE;
	}
}

static	void
ArchiveEnvironmentOpen (void)
{
	char	*sptr;

	if (openEnvironment	== FALSE)
	{
		/*
		 * Check if Archiving for sales orders is required
		 */
		sptr = chk_env ("SO_ARCHIVE_SO");
		envSoArchiveSo = (sptr == (char *)0) ? 0 : atoi (sptr);

		sptr = chk_env ("PC_ARCHIVE_WO");
		envPcArchiveWo = (sptr == (char *)0) ? 0 : atoi (sptr);

		sptr = chk_env ("PO_ARCHIVE_PO");
		envPoArchivePo = (sptr == (char *)0) ? 0 : atoi (sptr);

		openEnvironment	= TRUE;
	}
}

char *Replace (char *src, char oldc, char newc)
{
	char *pivot;

	pivot = src;

	if (src == NULL)
		return (src);

	while (*pivot != '\0')
	{
		if (*pivot == oldc)
			*pivot = newc;
		pivot++;
	}

	return (src);
}

int selfprintf (FILE *fp, const char *mask, ...)
{
	static char bufTemp [10000];
	va_list args;
	va_start (args, mask);
	vsprintf (bufTemp, mask, args);
	va_end (args);
	Replace (bufTemp, ' ', '\x05');
	Replace (bufTemp, '\x04', ' ');
	return (fprintf (fp, "%s", bufTemp));
}

int ArSohrRead (FILE *fp, void *structPtr)
{
	struct arsohrRecord *arsohrRecPtr;
	int fldCnt;
	
	arsohrRecPtr = (struct arsohrRecord *) structPtr;
	memset (arsohrRecPtr, 0, sizeof (struct arsohrRecord));
	
	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);
	
	strcpy (mask, "%s %s %s %s %s %ld %ld %ld %s %s %s %s %s %ld ");
	strcat (mask, "%s %s %s %ld %s %ld %s %s %s %s %d %f %s %s %s ");
	strcat (mask, "%s %ld %ld %s %s %s %s %s %s %lf %lf %s %lf %lf %lf ");
	strcat (mask, "%lf %lf %lf %s %s %s %s %s %s %s %s %s %s %s %s ");
	strcat (mask, "%s %s %s %s %s");
	
	fldCnt = sscanf (bufTemp, mask,
		arsohrRecPtr->co_no,
		arsohrRecPtr->br_no,
		arsohrRecPtr->dp_no,
		arsohrRecPtr->order_no,
		arsohrRecPtr->cont_no,
		&arsohrRecPtr->hhcu_hash,
		&arsohrRecPtr->chg_hhcu,
		&arsohrRecPtr->hhso_hash,
		arsohrRecPtr->inv_no,
		arsohrRecPtr->cus_ord_ref,
		arsohrRecPtr->chg_ord_ref,
		arsohrRecPtr->op_id,
		arsohrRecPtr->time_create,
		&arsohrRecPtr->date_create,
		arsohrRecPtr->cons_no,
		arsohrRecPtr->del_zone,
		arsohrRecPtr->del_req,
		&arsohrRecPtr->del_date,
		arsohrRecPtr->asm_req,
		&arsohrRecPtr->asm_date,
		arsohrRecPtr->s_timeslot,
		arsohrRecPtr->e_timeslot,
		arsohrRecPtr->carr_code,
		arsohrRecPtr->carr_area,
		&arsohrRecPtr->no_cartons,
		&arsohrRecPtr->no_kgs,
		arsohrRecPtr->sch_ord,
		arsohrRecPtr->ord_type,
		arsohrRecPtr->pri_type,
		arsohrRecPtr->frei_req,
		&arsohrRecPtr->dt_raised,
		&arsohrRecPtr->dt_required,
		arsohrRecPtr->tax_code,
		arsohrRecPtr->tax_no,
		arsohrRecPtr->area_code,
		arsohrRecPtr->sman_code,
		arsohrRecPtr->sell_terms,
		arsohrRecPtr->pay_term,
		&arsohrRecPtr->freight,
		&arsohrRecPtr->insurance,
		arsohrRecPtr->ins_det,
		&arsohrRecPtr->o_cost_1,
		&arsohrRecPtr->o_cost_2,
		&arsohrRecPtr->o_cost_3,
		&arsohrRecPtr->deposit,
		&arsohrRecPtr->discount,
		&arsohrRecPtr->exch_rate,
		arsohrRecPtr->fix_exch,
		arsohrRecPtr->batch_no,
		arsohrRecPtr->cont_name,
		arsohrRecPtr->cont_phone,
		arsohrRecPtr->del_name,
		arsohrRecPtr->del_add1,
		arsohrRecPtr->del_add2,
		arsohrRecPtr->del_add3,
		arsohrRecPtr->din_1,
		arsohrRecPtr->din_2,
		arsohrRecPtr->din_3,
		arsohrRecPtr->new,
		arsohrRecPtr->prt_price,
		arsohrRecPtr->full_supply,
		arsohrRecPtr->two_step,
		arsohrRecPtr->status,
		arsohrRecPtr->stat_flag);
		
	if (fldCnt != ARSOHR_NO_FIELDS)
		return (1);

	Replace (arsohrRecPtr->co_no, '\x05', ' '); 
	Replace (arsohrRecPtr->br_no, '\x05', ' '); 
	Replace (arsohrRecPtr->dp_no, '\x05', ' '); 
	Replace (arsohrRecPtr->order_no, '\x05', ' '); 
	Replace (arsohrRecPtr->cont_no, '\x05', ' '); 
	Replace (arsohrRecPtr->inv_no, '\x05', ' '); 
	Replace (arsohrRecPtr->cus_ord_ref, '\x05', ' '); 
	Replace (arsohrRecPtr->chg_ord_ref, '\x05', ' '); 
	Replace (arsohrRecPtr->op_id, '\x05', ' '); 
	Replace (arsohrRecPtr->time_create, '\x05', ' '); 
	Replace (arsohrRecPtr->cons_no, '\x05', ' '); 
	Replace (arsohrRecPtr->del_zone, '\x05', ' '); 
	Replace (arsohrRecPtr->del_req, '\x05', ' '); 
	Replace (arsohrRecPtr->asm_req, '\x05', ' '); 
	Replace (arsohrRecPtr->s_timeslot, '\x05', ' '); 
	Replace (arsohrRecPtr->e_timeslot, '\x05', ' '); 
	Replace (arsohrRecPtr->carr_code, '\x05', ' '); 
	Replace (arsohrRecPtr->carr_area, '\x05', ' '); 
	Replace (arsohrRecPtr->sch_ord, '\x05', ' '); 
	Replace (arsohrRecPtr->ord_type, '\x05', ' '); 
	Replace (arsohrRecPtr->pri_type, '\x05', ' '); 
	Replace (arsohrRecPtr->frei_req, '\x05', ' '); 
	Replace (arsohrRecPtr->tax_code, '\x05', ' '); 
	Replace (arsohrRecPtr->tax_no, '\x05', ' '); 
	Replace (arsohrRecPtr->area_code, '\x05', ' '); 
	Replace (arsohrRecPtr->sman_code, '\x05', ' '); 
	Replace (arsohrRecPtr->sell_terms, '\x05', ' '); 
	Replace (arsohrRecPtr->pay_term, '\x05', ' '); 
	Replace (arsohrRecPtr->ins_det, '\x05', ' '); 
	Replace (arsohrRecPtr->fix_exch, '\x05', ' '); 
	Replace (arsohrRecPtr->batch_no, '\x05', ' '); 
	Replace (arsohrRecPtr->cont_name, '\x05', ' '); 
	Replace (arsohrRecPtr->cont_phone, '\x05', ' '); 
	Replace (arsohrRecPtr->del_name, '\x05', ' '); 
	Replace (arsohrRecPtr->del_add1, '\x05', ' '); 
	Replace (arsohrRecPtr->del_add2, '\x05', ' '); 
	Replace (arsohrRecPtr->del_add3, '\x05', ' '); 
	Replace (arsohrRecPtr->din_1, '\x05', ' '); 
	Replace (arsohrRecPtr->din_2, '\x05', ' '); 
	Replace (arsohrRecPtr->din_3, '\x05', ' '); 
	Replace (arsohrRecPtr->new, '\x05', ' '); 
	Replace (arsohrRecPtr->prt_price, '\x05', ' '); 
	Replace (arsohrRecPtr->full_supply, '\x05', ' '); 
	Replace (arsohrRecPtr->two_step, '\x05', ' '); 
	Replace (arsohrRecPtr->status, '\x05', ' '); 
	Replace (arsohrRecPtr->stat_flag, '\x05', ' '); 

	return (0);
}

int ArSolnRead (FILE *fp, void *structPtr)
{
	struct arsolnRecord *arsolnRecPtr;
	int fldCnt;
	
	arsolnRecPtr = (struct arsolnRecord *) structPtr;
	memset (arsolnRecPtr, 0, sizeof (struct arsolnRecord));
	
	strcpy (mask, "%ld %d %ld %ld %ld %ld %s %d %f %f %f %lf %lf ");
	strcat (mask, "%lf %lf %f %f %f %f %f %d %f %f %f %f %s %s ");
	strcat (mask, "%s %s %s %s %ld %d %s %s %ld %s %s");

	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);
	
	fldCnt = sscanf (bufTemp, mask, 
		&arsolnRecPtr->hhso_hash,
		&arsolnRecPtr->line_no,
		&arsolnRecPtr->hhbr_hash,
		&arsolnRecPtr->hhcc_hash,
		&arsolnRecPtr->hhum_hash,
		&arsolnRecPtr->hhsl_hash,
		arsolnRecPtr->serial_no,
		&arsolnRecPtr->cont_status,
		&arsolnRecPtr->qty_order,
		&arsolnRecPtr->qty_bord,
		&arsolnRecPtr->qty_org_ord,
		&arsolnRecPtr->gsale_price,
		&arsolnRecPtr->sale_price,
		&arsolnRecPtr->cost_price,
		&arsolnRecPtr->item_levy,
		&arsolnRecPtr->dis_pc,
		&arsolnRecPtr->reg_pc,
		&arsolnRecPtr->disc_a,
		&arsolnRecPtr->disc_b,
		&arsolnRecPtr->disc_c,
		&arsolnRecPtr->cumulative,
		&arsolnRecPtr->tax_pc,
		&arsolnRecPtr->gst_pc,
		&arsolnRecPtr->o_xrate,
		&arsolnRecPtr->n_xrate,
		arsolnRecPtr->pack_size,
		arsolnRecPtr->sman_code,
		arsolnRecPtr->cus_ord_ref,
		arsolnRecPtr->pri_or,
		arsolnRecPtr->dis_or,
		arsolnRecPtr->item_desc,
		&arsolnRecPtr->due_date,
		&arsolnRecPtr->del_no,
		arsolnRecPtr->bonus_flag,
		arsolnRecPtr->hide_flag,
		&arsolnRecPtr->hhah_hash,
		arsolnRecPtr->status,
		arsolnRecPtr->stat_flag);

	if (fldCnt != ARSOLN_NO_FIELDS)
		return (1);

	Replace (arsolnRecPtr->serial_no, '\x05', ' ');
	Replace (arsolnRecPtr->pack_size, '\x05', ' ');
	Replace (arsolnRecPtr->sman_code, '\x05', ' ');
	Replace (arsolnRecPtr->cus_ord_ref, '\x05', ' ');
	Replace (arsolnRecPtr->pri_or, '\x05', ' ');
	Replace (arsolnRecPtr->dis_or, '\x05', ' ');
	Replace (arsolnRecPtr->item_desc, '\x05', ' ');
	Replace (arsolnRecPtr->bonus_flag, '\x05', ' ');
	Replace (arsolnRecPtr->hide_flag, '\x05', ' ');
	Replace (arsolnRecPtr->status, '\x05', ' ');
	Replace (arsolnRecPtr->stat_flag, '\x05', ' ');
	
	return (0);
}

int ArhrRead (FILE *fp, void *structPtr)
{
	struct arhrRecord *arhrRecPtr;
	int fldCnt;
	
	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);

	arhrRecPtr = (struct arhrRecord *) structPtr;
	memset (arhrRecPtr, 0, sizeof (struct arhrRecord));	

	strcpy (mask, "%s %s %s %s %s %ld %ld %s %s %s %ld %s %s %s ");
	strcat (mask, "%s %s %s %s %ld %s %ld %ld %s %s %s %s %d %lf ");
	strcat (mask, "%f %ld %ld %s %ld %ld %s %s %s %s %s %s %ld %lf ");
	strcat (mask, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s ");
	strcat (mask, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s ");
	strcat (mask, "%s %s %s %s %d %s %s %s %ld %s %ld %s %d");

	fldCnt = sscanf (bufTemp, mask,
		arhrRecPtr->co_no,
		arhrRecPtr->br_no,
		arhrRecPtr->dp_no,
		arhrRecPtr->inv_no,
		arhrRecPtr->app_inv_no,
		&arhrRecPtr->hhcu_hash,
		&arhrRecPtr->chg_hhcu_hash,
		arhrRecPtr->type,
		arhrRecPtr->cont_no,
		arhrRecPtr->drop_ship,
		&arhrRecPtr->hhds_hash,
		arhrRecPtr->cus_ord_ref,
		arhrRecPtr->chg_ord_ref,
		arhrRecPtr->ord_ref,
		arhrRecPtr->grn_no,
		arhrRecPtr->cons_no,
		arhrRecPtr->del_zone,
		arhrRecPtr->del_req,
		&arhrRecPtr->del_date,
		arhrRecPtr->asm_req,
		&arhrRecPtr->asm_date,
		&arhrRecPtr->asm_hash,
		arhrRecPtr->s_timeslot,
		arhrRecPtr->e_timeslot,
		arhrRecPtr->carr_code,
		arhrRecPtr->carr_area,
		&arhrRecPtr->no_cartons,
		&arhrRecPtr->wgt_per_ctn,
		&arhrRecPtr->no_kgs,
		&arhrRecPtr->hhso_hash,
		&arhrRecPtr->hhco_hash,
		arhrRecPtr->frei_req,
		&arhrRecPtr->date_raised,
		&arhrRecPtr->date_required,
		arhrRecPtr->tax_code,
		arhrRecPtr->tax_no,
		arhrRecPtr->area_code,
		arhrRecPtr->sale_code,
		arhrRecPtr->op_id,
		arhrRecPtr->time_create,
		&arhrRecPtr->date_create,
		&arhrRecPtr->gross,
		&arhrRecPtr->freight,
		&arhrRecPtr->insurance,
		&arhrRecPtr->other_cost_1,
		&arhrRecPtr->other_cost_2,
		&arhrRecPtr->other_cost_3,
		&arhrRecPtr->tax,
		&arhrRecPtr->gst,
		&arhrRecPtr->disc,
		&arhrRecPtr->deposit,
		&arhrRecPtr->ex_disc,
		&arhrRecPtr->erate_var,
		&arhrRecPtr->sos,
		&arhrRecPtr->item_levy,
		&arhrRecPtr->exch_rate,
		arhrRecPtr->fix_exch,
		arhrRecPtr->batch_no,
		arhrRecPtr->dl_name,
		arhrRecPtr->dl_add1,
		arhrRecPtr->dl_add2,
		arhrRecPtr->dl_add3,
		arhrRecPtr->din_1,
		arhrRecPtr->din_2,
		arhrRecPtr->din_3,
		arhrRecPtr->pay_type,
		arhrRecPtr->pay_terms,
		arhrRecPtr->sell_terms,
		arhrRecPtr->ins_det,
		arhrRecPtr->pri_type,
		arhrRecPtr->pri_break,
		arhrRecPtr->ord_type,
		arhrRecPtr->prt_price,
		arhrRecPtr->status,
		arhrRecPtr->stat_flag,
		arhrRecPtr->ps_print,
		&arhrRecPtr->ps_print_no,
		arhrRecPtr->inv_print,
		arhrRecPtr->ccn_print,
		arhrRecPtr->printing,
		&arhrRecPtr->hhtr_hash,
		arhrRecPtr->load_flag,
		&arhrRecPtr->wrmr_hash,
		arhrRecPtr->pos_inv_no,
		&arhrRecPtr->pos_tran_type);

	if (fldCnt != ARHR_NO_FIELDS)
		return (1);

	Replace (arhrRecPtr->co_no, '\x05', ' ');
	Replace (arhrRecPtr->br_no, '\x05', ' ');
	Replace (arhrRecPtr->dp_no, '\x05', ' ');
	Replace (arhrRecPtr->inv_no, '\x05', ' ');
	Replace (arhrRecPtr->app_inv_no, '\x05', ' ');
	Replace (arhrRecPtr->type, '\x05', ' ');
	Replace (arhrRecPtr->cont_no, '\x05', ' ');
	Replace (arhrRecPtr->drop_ship, '\x05', ' ');
	Replace (arhrRecPtr->cus_ord_ref, '\x05', ' ');
	Replace (arhrRecPtr->chg_ord_ref, '\x05', ' ');
	Replace (arhrRecPtr->ord_ref, '\x05', ' ');
	Replace (arhrRecPtr->grn_no, '\x05', ' ');
	Replace (arhrRecPtr->cons_no, '\x05', ' ');
	Replace (arhrRecPtr->del_zone, '\x05', ' ');
	Replace (arhrRecPtr->del_req, '\x05', ' ');
	Replace (arhrRecPtr->asm_req, '\x05', ' ');
	Replace (arhrRecPtr->s_timeslot, '\x05', ' ');
	Replace (arhrRecPtr->e_timeslot, '\x05', ' ');
	Replace (arhrRecPtr->carr_code, '\x05', ' ');
	Replace (arhrRecPtr->carr_area, '\x05', ' ');
	Replace (arhrRecPtr->frei_req, '\x05', ' ');
	Replace (arhrRecPtr->tax_code, '\x05', ' ');
	Replace (arhrRecPtr->tax_no, '\x05', ' ');
	Replace (arhrRecPtr->area_code, '\x05', ' ');
	Replace (arhrRecPtr->sale_code, '\x05', ' ');
	Replace (arhrRecPtr->op_id, '\x05', ' ');
	Replace (arhrRecPtr->time_create, '\x05', ' ');
	Replace (arhrRecPtr->fix_exch, '\x05', ' ');
	Replace (arhrRecPtr->batch_no, '\x05', ' ');
	Replace (arhrRecPtr->dl_name, '\x05', ' ');
	Replace (arhrRecPtr->dl_add1, '\x05', ' ');
	Replace (arhrRecPtr->dl_add2, '\x05', ' ');
	Replace (arhrRecPtr->dl_add3, '\x05', ' ');
	Replace (arhrRecPtr->din_1, '\x05', ' ');
	Replace (arhrRecPtr->din_2, '\x05', ' ');
	Replace (arhrRecPtr->din_3, '\x05', ' ');
	Replace (arhrRecPtr->pay_type, '\x05', ' ');
	Replace (arhrRecPtr->pay_terms, '\x05', ' ');
	Replace (arhrRecPtr->sell_terms, '\x05', ' ');
	Replace (arhrRecPtr->ins_det, '\x05', ' ');
	Replace (arhrRecPtr->pri_type, '\x05', ' ');
	Replace (arhrRecPtr->pri_break, '\x05', ' ');
	Replace (arhrRecPtr->ord_type, '\x05', ' ');
	Replace (arhrRecPtr->prt_price, '\x05', ' ');
	Replace (arhrRecPtr->status, '\x05', ' ');
	Replace (arhrRecPtr->stat_flag, '\x05', ' ');
	Replace (arhrRecPtr->ps_print, '\x05', ' ');
	Replace (arhrRecPtr->inv_print, '\x05', ' ');
	Replace (arhrRecPtr->ccn_print, '\x05', ' ');
	Replace (arhrRecPtr->printing, '\x05', ' ');
	Replace (arhrRecPtr->load_flag, '\x05', ' ');
	Replace (arhrRecPtr->pos_inv_no, '\x05', ' ');
	
	return (0);
}

int ArlnRead (FILE *fp, void *structPtr)
{
	struct arlnRecord *arlnRecPtr;
	int fldCnt;
	
	arlnRecPtr = (struct arlnRecord *) structPtr;
	memset (arlnRecPtr, 0, sizeof (struct arlnRecord));

	strcpy (mask, "%ld %ld %d %ld %ld %ld %ld %s %ld %s %s %d %f ");
	strcat (mask, "%f %f %f %f %lf %lf %lf %lf %f %f %f %f %f %d ");
	strcat (mask, "%f %f %lf %lf %lf %lf %lf %lf %lf %s %s %s %s %f ");
	strcat (mask, "%f %s %ld %s %s %s %ld %s %s %ld");

	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);

	fldCnt = sscanf (bufTemp, mask,
		&arlnRecPtr->hhcl_hash,
		&arlnRecPtr->hhco_hash,
		&arlnRecPtr->line_no,
		&arlnRecPtr->hhbr_hash,
		&arlnRecPtr->incc_hash,
		&arlnRecPtr->hhum_hash,
		&arlnRecPtr->hhsl_hash,
		arlnRecPtr->order_no,
		&arlnRecPtr->hhdl_hash,
		arlnRecPtr->crd_type,
		arlnRecPtr->serial_no,
		&arlnRecPtr->cont_status,
		&arlnRecPtr->qty_org_ord,
		&arlnRecPtr->q_order,
		&arlnRecPtr->qty_del,
		&arlnRecPtr->qty_ret,
		&arlnRecPtr->q_backorder,
		&arlnRecPtr->gsale_price,
		&arlnRecPtr->sale_price,
		&arlnRecPtr->cost_price,
		&arlnRecPtr->item_levy,
		&arlnRecPtr->disc_pc,
		&arlnRecPtr->reg_pc,
		&arlnRecPtr->disc_a,
		&arlnRecPtr->disc_b,
		&arlnRecPtr->disc_c,
		&arlnRecPtr->cumulative,
		&arlnRecPtr->tax_pc,
		&arlnRecPtr->gst_pc,
		&arlnRecPtr->gross,
		&arlnRecPtr->freight,
		&arlnRecPtr->on_cost,
		&arlnRecPtr->amt_disc,
		&arlnRecPtr->amt_tax,
		&arlnRecPtr->amt_gst,
		&arlnRecPtr->erate_var,
		arlnRecPtr->pack_size,
		arlnRecPtr->sman_code,
		arlnRecPtr->cus_ord_ref,
		arlnRecPtr->org_ord_ref,
		&arlnRecPtr->o_xrate,
		&arlnRecPtr->n_xrate,
		arlnRecPtr->item_desc,
		&arlnRecPtr->due_date,
		arlnRecPtr->status,
		arlnRecPtr->bonus_flag,
		arlnRecPtr->hide_flag,
		&arlnRecPtr->hhah_hash,
		arlnRecPtr->price_type,
		arlnRecPtr->stat_flag,
		&arlnRecPtr->hhwo_hash);

	if (fldCnt != ARLN_NO_FIELDS)
		return (1);

	Replace (arlnRecPtr->order_no, '\x05', ' ');
	Replace (arlnRecPtr->crd_type, '\x05', ' ');
	Replace (arlnRecPtr->serial_no, '\x05', ' ');
	Replace (arlnRecPtr->pack_size, '\x05', ' ');
	Replace (arlnRecPtr->sman_code, '\x05', ' ');
	Replace (arlnRecPtr->cus_ord_ref, '\x05', ' ');
	Replace (arlnRecPtr->org_ord_ref, '\x05', ' ');
	Replace (arlnRecPtr->item_desc, '\x05', ' ');
	Replace (arlnRecPtr->status, '\x05', ' ');
	Replace (arlnRecPtr->bonus_flag, '\x05', ' ');
	Replace (arlnRecPtr->hide_flag, '\x05', ' ');
	Replace (arlnRecPtr->price_type, '\x05', ' ');
	Replace (arlnRecPtr->stat_flag, '\x05', ' ');
	
	return (0);
}

int ArPohrRead (FILE *fp, void *structPtr)
{
	struct arpohrRecord *arpohrRecPtr;
	int fldCnt;
	
	arpohrRecPtr = (struct arpohrRecord *) structPtr;
	memset (arpohrRecPtr, 0, sizeof (struct arpohrRecord));
	
	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);

	strcpy (mask, "%s %s %s %ld %s %ld %ld %ld %ld %ld %ld %s %s ");
	strcat (mask, "%s %s %ld %s %s %s %s %s %s %s %s %ld %ld %ld ");
	strcat (mask, "%s %lf %s %s %s %ld %lf %lf %s %s %s %s %s");

	fldCnt = sscanf (bufTemp, mask,
		arpohrRecPtr->co_no,
		arpohrRecPtr->br_no,
		arpohrRecPtr->type,
		&arpohrRecPtr->hhsu_hash,
		arpohrRecPtr->pur_ord_no,
		&arpohrRecPtr->hhpo_hash,
		&arpohrRecPtr->hhsh_hash,
		&arpohrRecPtr->hhdd_hash,
		&arpohrRecPtr->date_raised,
		&arpohrRecPtr->due_date,
		&arpohrRecPtr->conf_date,
		arpohrRecPtr->contact,
		arpohrRecPtr->app_code,
		arpohrRecPtr->op_id,
		arpohrRecPtr->time_create,
		&arpohrRecPtr->date_create,
		arpohrRecPtr->req_usr,
		arpohrRecPtr->reason,
		arpohrRecPtr->stdin1,
		arpohrRecPtr->stdin2,
		arpohrRecPtr->stdin3,
		arpohrRecPtr->delin1,
		arpohrRecPtr->delin2,
		arpohrRecPtr->delin3,
		&arpohrRecPtr->ship1_no,
		&arpohrRecPtr->ship2_no,
		&arpohrRecPtr->ship3_no,
		arpohrRecPtr->curr_code,
		&arpohrRecPtr->curr_rate,
		arpohrRecPtr->term_order,
		arpohrRecPtr->sup_trm_pay,
		arpohrRecPtr->bnk_trm_pay,
		&arpohrRecPtr->pay_date,
		&arpohrRecPtr->fgn_total,
		&arpohrRecPtr->fgn_ostand,
		arpohrRecPtr->ship_method,
		arpohrRecPtr->drop_ship,
		arpohrRecPtr->status,
		arpohrRecPtr->stat_flag,
		arpohrRecPtr->sup_type);
		
	if (fldCnt != ARPOHR_NO_FIELDS)
		return (1);

	Replace (arpohrRecPtr->co_no, '\x05', ' ');
	Replace (arpohrRecPtr->br_no, '\x05', ' ');
	Replace (arpohrRecPtr->type, '\x05', ' ');
	Replace (arpohrRecPtr->pur_ord_no, '\x05', ' ');
	Replace (arpohrRecPtr->contact, '\x05', ' ');
	Replace (arpohrRecPtr->app_code, '\x05', ' ');
	Replace (arpohrRecPtr->op_id, '\x05', ' ');
	Replace (arpohrRecPtr->time_create, '\x05', ' ');
	Replace (arpohrRecPtr->req_usr, '\x05', ' ');
	Replace (arpohrRecPtr->reason, '\x05', ' ');
	Replace (arpohrRecPtr->stdin1, '\x05', ' ');
	Replace (arpohrRecPtr->stdin2, '\x05', ' ');
	Replace (arpohrRecPtr->stdin3, '\x05', ' ');
	Replace (arpohrRecPtr->delin1, '\x05', ' ');
	Replace (arpohrRecPtr->delin2, '\x05', ' ');
	Replace (arpohrRecPtr->delin3, '\x05', ' ');
	Replace (arpohrRecPtr->curr_code, '\x05', ' ');
	Replace (arpohrRecPtr->term_order, '\x05', ' ');
	Replace (arpohrRecPtr->sup_trm_pay, '\x05', ' ');
	Replace (arpohrRecPtr->bnk_trm_pay, '\x05', ' ');
	Replace (arpohrRecPtr->ship_method, '\x05', ' ');
	Replace (arpohrRecPtr->drop_ship, '\x05', ' ');
	Replace (arpohrRecPtr->status, '\x05', ' ');
	Replace (arpohrRecPtr->stat_flag, '\x05', ' ');
	Replace (arpohrRecPtr->sup_type, '\x05', ' ');

	return (0);
}

int ArPolnRead (FILE *fp, void *structPtr)
{
	struct arpolnRecord *arpolnRecPtr;
	int fldCnt;
	
	arpolnRecPtr = (struct arpolnRecord *) structPtr;
	memset (arpolnRecPtr, 0, sizeof (struct arpolnRecord));
	
	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);

	strcpy (mask, "%ld %d %ld %ld %ld %ld %ld %ld %lf %s %s %s %f %f ");
	strcat (mask, "%f %f %f %f %f %f %f %f %d %lf %lf %lf %lf %lf %lf ");
	strcat (mask, "%lf %lf %s %s %ld %d %ld %ld %ld %s %s %s");
	
	fldCnt = sscanf (bufTemp, mask,
		&arpolnRecPtr->hhpo_hash,
		&arpolnRecPtr->line_no,
		&arpolnRecPtr->hhbr_hash,
		&arpolnRecPtr->hhum_hash,
		&arpolnRecPtr->hhcc_hash,
		&arpolnRecPtr->hhlc_hash,
		&arpolnRecPtr->hhpl_hash,
		&arpolnRecPtr->hhpl_orig,
		&arpolnRecPtr->exch_rate,
		arpolnRecPtr->serial_no,
		arpolnRecPtr->container,
		arpolnRecPtr->cus_ord_ref,
		&arpolnRecPtr->qty_ord,
		&arpolnRecPtr->qty_rec,
		&arpolnRecPtr->pack_qty,
		&arpolnRecPtr->chg_wgt,
		&arpolnRecPtr->gross_wgt,
		&arpolnRecPtr->cu_metre,
		&arpolnRecPtr->reg_pc,
		&arpolnRecPtr->disc_a,
		&arpolnRecPtr->disc_b,
		&arpolnRecPtr->disc_c,
		&arpolnRecPtr->cumulative,
		&arpolnRecPtr->grs_fgn_cst,
		&arpolnRecPtr->fob_fgn_cst,
		&arpolnRecPtr->fob_nor_cst,
		&arpolnRecPtr->frt_ins_cst,
		&arpolnRecPtr->duty,
		&arpolnRecPtr->licence,
		&arpolnRecPtr->lcost_load,
		&arpolnRecPtr->land_cst,
		arpolnRecPtr->cat_code,
		arpolnRecPtr->item_desc,
		&arpolnRecPtr->ship_no,
		&arpolnRecPtr->case_no,
		&arpolnRecPtr->hhso_hash,
		&arpolnRecPtr->due_date,
		&arpolnRecPtr->fwd_date,
		arpolnRecPtr->pur_status,
		arpolnRecPtr->status,
		arpolnRecPtr->stat_flag);

	if (fldCnt != ARPOLN_NO_FIELDS)
		return (1);

	Replace (arpolnRecPtr->serial_no, '\x05', ' ');
	Replace (arpolnRecPtr->container, '\x05', ' ');
	Replace (arpolnRecPtr->cus_ord_ref, '\x05', ' ');
	Replace (arpolnRecPtr->cat_code, '\x05', ' ');
	Replace (arpolnRecPtr->item_desc, '\x05', ' ');
	Replace (arpolnRecPtr->pur_status, '\x05', ' ');
	Replace (arpolnRecPtr->status, '\x05', ' ');
	Replace (arpolnRecPtr->stat_flag, '\x05', ' ');

	return (0);
}

int ArPcwoRead (FILE *fp, void *structPtr)
{
	struct arpcwoRecord *arpcwoRecPtr;
	int fldCnt;
	
	arpcwoRecPtr = (struct arpcwoRecord *) structPtr;
	memset (arpcwoRecPtr, 0, sizeof (struct arpcwoRecord));

	if (fgets (bufTemp, 10000, fp) == NULL)
		return (1);

	strcpy (mask, "%s %s %s %s %s %s %s %s %ld %ld %d %d %s %s ");
	strcat (mask, "%ld %ld %ld %d %d %ld %f %f %f %s %s %ld %s");

	fldCnt = sscanf (bufTemp, mask,
		arpcwoRecPtr->co_no,
		arpcwoRecPtr->br_no,
		arpcwoRecPtr->wh_no,
		arpcwoRecPtr->order_no,
		arpcwoRecPtr->req_br_no,
		arpcwoRecPtr->req_wh_no,
		arpcwoRecPtr->rec_br_no,
		arpcwoRecPtr->rec_wh_no,
		&arpcwoRecPtr->hhwo_hash,
		&arpcwoRecPtr->reqd_date,
		&arpcwoRecPtr->rtg_seq,
		&arpcwoRecPtr->priority,
		arpcwoRecPtr->op_id,
		arpcwoRecPtr->create_time,
		&arpcwoRecPtr->create_date,
		&arpcwoRecPtr->mfg_date,
		&arpcwoRecPtr->hhbr_hash,
		&arpcwoRecPtr->bom_alt,
		&arpcwoRecPtr->rtg_alt,
		&arpcwoRecPtr->hhcc_hash,
		&arpcwoRecPtr->prod_qty,
		&arpcwoRecPtr->act_prod_q,
		&arpcwoRecPtr->act_rej_q,
		arpcwoRecPtr->order_stat,
		arpcwoRecPtr->batch_no,
		&arpcwoRecPtr->hhsl_hash,
		arpcwoRecPtr->stat_flag);

	if (fldCnt != ARPCWO_NO_FIELDS)
		return (1);

	Replace (arpcwoRecPtr->co_no, '\x05', ' ');
	Replace (arpcwoRecPtr->br_no, '\x05', ' ');
	Replace (arpcwoRecPtr->wh_no, '\x05', ' ');
	Replace (arpcwoRecPtr->order_no, '\x05', ' ');
	Replace (arpcwoRecPtr->req_br_no, '\x05', ' ');
	Replace (arpcwoRecPtr->req_wh_no, '\x05', ' ');
	Replace (arpcwoRecPtr->rec_br_no, '\x05', ' ');
	Replace (arpcwoRecPtr->rec_wh_no, '\x05', ' ');
	Replace (arpcwoRecPtr->op_id, '\x05', ' ');
	Replace (arpcwoRecPtr->create_time, '\x05', ' ');
	Replace (arpcwoRecPtr->order_stat, '\x05', ' ');
	Replace (arpcwoRecPtr->batch_no, '\x05', ' ');
	Replace (arpcwoRecPtr->stat_flag, '\x05', ' ');

	return (0);
}

