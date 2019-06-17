/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_ship_disp.c,v 5.0 2002/05/08 01:24:38 scott Exp $
|  Program Name  : (po_shipschd.c)
|  Program Desc  : (Purchase Order Shipping Schedule)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: po_ship_disp.c,v $
| Revision 5.0  2002/05/08 01:24:38  scott
| CVS administration
|
| Revision 1.1  2001/12/03 00:32:06  scott
| New program to display shipments.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_ship_disp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_ship_disp/po_ship_disp.c,v 5.0 2002/05/08 01:24:38 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <arralloc.h>

	/*
	 * Special fields and flags.
	 */
	int		sortByContainer	=	TRUE;

	char	shipMethodDesc	[11], 
			shipDepartDate	[11], 
			shipArriveDate	[11];

#include	"schema"

struct commRecord	comm_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct poslRecord	posl_rec;
struct inumRecord	inum_rec;
struct pocrRecord	pocr_rec;
struct skcmRecord	skcm_rec;

	char	dispStr [200];

/*
 *	Structure for dynamic array,  for the shipRec lines for qsort
 */
struct ShipmentStruct
{
	char	sort			[47];
	char	supplierNo 		[sizeof sumr_rec.crd_no];
	char	suppliername 	[sizeof sumr_rec.crd_name];
	char	pOrderNo	 	[sizeof pohr_rec.pur_ord_no];
	char	invoiceNo	 	[sizeof posd_rec.inv_no];
	char	comment	 		[sizeof posd_rec.comment];
	char	container 		[sizeof posl_rec.container];
	char	itemNo 			[sizeof inmr_rec.item_no];
	char	itemDesc 		[sizeof inmr_rec.description];
	char	uom 			[sizeof inum_rec.uom];
	int		lineNo;
	float	quantity;
}	*shipRec;
	DArray shipment_details;
	int	shipCnt = 0;

extern	int		EnvScreenOK;

/*
 * Function Declarations
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int		ShipSort 			(const void *, const void *);
void 	HeadingOutput 		(void);
void 	ProcessFile 		(void);
int 	FindSupplier		(long);
int 	ProcessDetails 		(char *);

/*
 * Main Processing Routine.
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	init_scr ();
	set_tty ();

	clear ();
	swide ();

	OpenDB ();

	if (argc == 2)
	{
		rv_pr (ML ("Shipment Display"), 50, 0, 1);
		print_at (22, 0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
		ProcessDetails (argv [1]);
	}
	else
	{
		HeadingOutput ();

		ProcessFile ();

		Dsp_srch_fn (ProcessDetails);
		Dsp_close ();
	}
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (skcm, skcm_list, SKCM_NO_FIELDS, "skcm_id_no");
}

/*
 * Close data base files .
 */
void
CloseDB (void)
{
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (posl);
	abc_fclose (inum);
	abc_fclose (pocr);
	abc_fclose (skcm);
	abc_dbclose ("data");
}

/*
 * Start Out Put To Standard Print.
 */
void
HeadingOutput (void)
{
	rv_pr (ML ("Shipment Display"), 50, 0, 1);
	print_at (22, 0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);

	Dsp_open (0, 1, 15);

	Dsp_saverec ("   Shipment   | Departure  |  Arrival   | Ship |     Vessel         |   Shipment Origin  |Shipment Destination|   B.O.L.   |Curr.");
	Dsp_saverec ("    Number    |   Date     |    Date    |  By  |                    |                    |                    |   Number   |Code ");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END] ");
}

/*
 * Main processing routine to get shipment header.
 */
void
ProcessFile (void)
{
	char	saveDesc [11];

	strcpy (posh_rec.co_no, comm_rec.co_no);
	strcpy (posh_rec.csm_no, " ");
	cc = find_rec (posh, &posh_rec, GTEQ, "r");
	while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no))
	{
		strcpy (shipDepartDate, " ");
		strcpy (shipArriveDate, " ");

		if (posh_rec.ship_depart)
			strcpy (shipDepartDate, DateToString (posh_rec.ship_depart));

		if (posh_rec.ship_arrive)
			strcpy (shipArriveDate, DateToString (posh_rec.ship_arrive));

		strcpy (shipMethodDesc, " ");
		if (posh_rec.ship_method [0] == 'A')
			strcpy (shipMethodDesc, ML ("AIR "));
		else if (posh_rec.ship_method [0] == 'S')
			strcpy (shipMethodDesc, ML ("SEA "));
		else if (posh_rec.ship_method [0] == 'L')
			strcpy (shipMethodDesc, ML ("LAND"));
		else if (posh_rec.ship_method [0] == 'R')
			strcpy (shipMethodDesc, ML ("RAIL"));

		sprintf 
		(	
			dispStr, 
			" %-12.12s ^E %-10.10s ^E %-10.10s ^E %-4.4s ^E%-20.20s^E%-20.20s^E%-20.20s^E%-12.12s^E%4.4s", 
			posh_rec.csm_no, 
			shipDepartDate, 
			shipArriveDate, 
			shipMethodDesc, 
			posh_rec.vessel, 
			posh_rec.port, 
			posh_rec.destination, 
			posh_rec.bol_no, 
			posh_rec.curr_code
		);
		sprintf (saveDesc, "%010ld", posh_rec.hhsh_hash);

		Dsp_save_fn (dispStr, saveDesc);

		cc = find_rec (posh, &posh_rec, NEXT, "r");
	}
	Dsp_saverec ("^^GGGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGJGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGG");
}

/*
 * Read the supplier record
 */
int
FindSupplier (
	long	hhsuHash)
{
	sumr_rec.hhsu_hash = hhsuHash;
	return (find_rec (sumr, &sumr_rec, COMPARISON, "r"));
}


/*
 * Process the shipment detail record
 */
int
ProcessDetails (
	char	*saveDesc)
{
	int		firstTime	= TRUE, 
			someDetails = 0,
			i			= 0;

	long	hhshHash	= 0L;
	char	previousContainer	[sizeof posl_rec.container];
	
	float	outstandingQty 	= 0.0,
			StdCnvFct 		= 1.00, 
			PurCnvFct 		= 1.00, 
			CnvFct			= 1.00;

	hhshHash = atol (saveDesc);

	abc_selfield (posh, "posh_hhsh_hash");

	posh_rec.hhsh_hash	=	hhshHash;
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, posh, "DBFIND");

	abc_selfield (posh, "posh_id_no2");

	Dsp_open (0, 1, 4);

	sprintf
	(
		err_str, 
		" Shipment Number      : %12.12s            Departure Date     : %10.10s      Arrival Date   : %10.10s                 ", 
		posh_rec.csm_no, 
		shipDepartDate, 
		shipArriveDate
	);
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec ("");

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code,  posh_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		strcpy (pocr_rec.description, " ");

	sprintf
	(
		err_str, 
		" Shipment Method      : %5.5s                   Shipment Currency  : %4.4s - %40.40s            ", 
		shipMethodDesc, 
		posh_rec.curr_code, 
		pocr_rec.description
	);
	Dsp_saverec (err_str);

	sprintf
	(
		err_str, 
		" Shipment Name        : %20.20s    Shipment Comments  : %60.60s", 
		posh_rec.vessel, 
		posh_rec.v_comm1
	);
	Dsp_saverec (err_str);
	
	sprintf 
	(
		err_str, 
		" Shipment Origin      : %20.20s    Departure Comments : %60.60s", 
		posh_rec.port, 
		posh_rec.s_comm1
	);
	Dsp_saverec (err_str);
	
	sprintf 
	(
		err_str, 
		" Shipment Destination : %20.20s    Destination Desc   : %60.60s", 
		posh_rec.destination, 
		posh_rec.r_comm1
	);
	Dsp_saverec (err_str);
	Dsp_srch ();

	Dsp_open (0, 8, 9);
	sprintf (err_str, "%57.57sShipment Details%56.56s", " ", " ");
	Dsp_saverec (err_str);
	Dsp_saverec ("");
	Dsp_saverec ("[NEXT] [PREV] [EDIT/END]");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&shipment_details,&shipRec,sizeof (struct ShipmentStruct),10);
	shipCnt = 0;

	/*
	 * Read Shipment Line allocation file.
	 */
	strcpy (posl_rec.co_no, comm_rec.co_no);
	posl_rec.hhsh_hash	=	posh_rec.hhsh_hash;
	posl_rec.hhpl_hash	=	0L;
	cc = find_rec (posl, &posl_rec, GTEQ, "r");
	while (!cc && !strcmp (posl_rec.co_no, comm_rec.co_no) &&
			posl_rec.hhsh_hash == posh_rec.hhsh_hash)
	{
		outstandingQty = posl_rec.ship_qty;

		/*
	  	 * Ignore if no quantity outstanding.
	 	 */
		if (outstandingQty <= 0.00)
		{
			cc = find_rec (posl, &posl_rec, NEXT, "r");
			continue;
		}

		/*
		 * Find purchase order line.
		 */
		poln_rec.hhpl_hash	=	posl_rec.hhpl_hash;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpl_hash == posl_rec.hhpl_hash)
		{
			/*
			 * Find purchase order header.
			 */
			pohr_rec.hhpo_hash = poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}
			/*
			 * Find Supplier.
			 */
			cc = FindSupplier (pohr_rec.hhsu_hash);
			if (cc)
				strcpy (sumr_rec.acronym, "         ");

			/*
			 * Find inventory master file.
			 */
			inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}
			/*
			 * Find item UOM and purchase UOM.
			 */
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

			inum_rec.hhum_hash	=	poln_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
			CnvFct	=	StdCnvFct / PurCnvFct;

			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&shipment_details, shipRec, shipCnt))
				sys_err ("ArrChkLimit (shipRec)", ENOMEM, PNAME);

			/*
			 * Save details in sort structure.
			 */
			sprintf 
			(
				shipRec [shipCnt].sort, "%-15.15s%-15.15s%-16.16s", 
				posl_rec.container, 
				pohr_rec.pur_ord_no,
				inmr_rec.item_no
			);
			strcpy (shipRec [shipCnt].supplierNo, 	sumr_rec.crd_no);
			strcpy (shipRec [shipCnt].suppliername, sumr_rec.crd_name);
			strcpy (shipRec [shipCnt].pOrderNo, 	pohr_rec.pur_ord_no);
			strcpy (shipRec [shipCnt].container, 	posl_rec.container);
			strcpy (shipRec [shipCnt].itemNo, 		inmr_rec.item_no);
			strcpy (shipRec [shipCnt].itemDesc, 	inmr_rec.description);
			strcpy (shipRec [shipCnt].uom, 			inum_rec.uom);
			shipRec [shipCnt].quantity	=	outstandingQty * CnvFct;
			shipCnt++;

			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (shipRec, shipCnt, sizeof (struct ShipmentStruct), ShipSort);

	/*
	 * Process details to screen.         
	 */
	for (i = 0; i < shipCnt; i++)
	{
		someDetails = TRUE;

		if (firstTime)
			strcpy (previousContainer, "");
		
		if (strcmp (previousContainer, shipRec [i].container))
		{
			if (!firstTime)
				Dsp_saverec ("^^GGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGG");
			strcpy (skcm_rec.co_no, comm_rec.co_no);
			strcpy (skcm_rec.container, shipRec [i].container);
			cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
			if (cc)
				strcpy (skcm_rec.desc, " ");

			sprintf 
			(
				err_str,
				"^1Container Number : %-15.15s - %40.40s^6", 
				skcm_rec.container,
				skcm_rec.desc
			);
			Dsp_saverec (err_str);
			strcpy (previousContainer, shipRec [i].container);
			Dsp_saverec ("^^GGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGIGGGGGGGGGGGG");
			Dsp_saverec ("Supplier^ESupplier Name                   ^EPurchase Order ^EItem Number      ^EItem Description                   ^EUom.^EShipment Qty");
			Dsp_saverec ("^^GGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGHGGGGGGGGGGGG");
		}
		sprintf
		(
			err_str,
			" %6.6s ^E%-32.32s^E%15.15s^E %16.16s^E%-35.35s^E%4.4s^E%12.2f",
			shipRec [i].supplierNo,
			shipRec [i].suppliername,
			shipRec [i].pOrderNo,
			shipRec [i].itemNo, 
			shipRec [i].itemDesc,
			shipRec [i].uom, 	
			shipRec [i].quantity
		);
		firstTime	=	FALSE;
		Dsp_saverec (err_str);
	}
	if (someDetails)
		Dsp_saverec ("^^GGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGG");
	Dsp_srch ();
	Dsp_close ();
	Dsp_close ();

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&shipment_details);
    return (EXIT_SUCCESS);
}

/*
 * Sort by Container/Purchase order OR by Purchase order/Container
 */
int 
ShipSort (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct ShipmentStruct a = * (const struct ShipmentStruct *) a1;
	const struct ShipmentStruct b = * (const struct ShipmentStruct *) b1;

	result = strcmp (a.sort, b.sort);

	return (result);
}
