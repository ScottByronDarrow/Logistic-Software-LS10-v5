/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_check.c,v 5.1 2002/07/18 05:20:35 scott Exp $
|  Program Name  : (so_check.c)  
|  Program Desc  : (Check Committed, On Order, backorder etc.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/10/1988       |
|---------------------------------------------------------------------|
| $Log: so_check.c,v $
| Revision 5.1  2002/07/18 05:20:35  scott
| Needs review for Phantoms.
|
| Revision 5.0  2002/05/07 10:22:59  scott
| Updated to bring version number to 5.0
|
| Revision 1.6  2002/04/30 07:56:47  scott
| Update for new Archive modifications;
|
| Revision 1.5  2001/09/11 23:46:01  scott
| Updated from Scott machine - 12th Sep 2001
|
| Revision 1.4  2001/09/11 22:41:36  scott
| Updated from scotts machine
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_check.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_check/so_check.c,v 5.1 2002/07/18 05:20:35 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<number.h>
#include	<arralloc.h>
#include 	<tabdisp.h>
#include 	<hot_keys.h>

#define		VALID_ITLN 	 (itln_rec.status [0] == 'T' || \
						  itln_rec.status [0] == 'B' || \
						  itln_rec.status [0] == 'M' || \
						  itln_rec.status [0] == 'U')

#define		NON_STOCK	 (inmr_rec.inmr_class [0] == 'N' || \
						  inmr_rec.inmr_class [0] == 'Z')

#define		PHANTOM		 (inmr_rec.inmr_class [0] == 'P')

#include	"schema"

struct commRecord	comm_rec;
struct cmrdRecord	cmrd_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inmrRecord	inmr3_rec;
struct inccRecord	incc_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct soicRecord	soic_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct pcwoRecord	pcwo_rec;
struct pcmsRecord	pcms_rec;
struct inumRecord	inum_rec;
struct cohrRecord	cohr_rec;
struct sohrRecord	sohr_rec;
struct itlnRecord	itln_rec;
struct soktRecord	sokt_rec;
struct soktRecord	sokt2_rec;
struct ccmrRecord	ccmr_rec;
struct inmeRecord	inme_rec;
struct qchrRecord	qchr_rec;
struct qclnRecord	qcln_rec;

	char	*data		=	"data",
			*ItemTab	=	"ItemTab",
			*incc2		=	"incc2",
			*inmr2		=	"inmr2",
			*inmr3		=	"inmr3",
			*pcwo2		=	"pcwo2",
			*sokt2		=	"sokt2";

	float	cnv_fct	=	1.00;

	/*
	 * Structure for dynamic array for actual and calculates values.
	 */
	struct	ItemCalc {
		long	hhccHash;
		float	wo_actual;	/* Works order		*/
		float	wo_calc;
		float	oo_actual;	/* On order			*/
		float	oo_calc;
		float	co_actual;	/* Committed order 	*/
		float	co_calc;
		float	bo_actual;	/* Back order		*/
		float	bo_calc;
		float	fo_actual;	/* forward order	*/
		float	fo_calc;
		float	qc_actual;	/* Quality Control	*/
		float	qc_calc;
	} *warehouses;

	DArray	warehouse_d;
	int		warehouseCount	=	0;
	int		noInTab			=	0;

	int		envVarCmInstalled	=	FALSE,
			envVarMaInstalled	= 	TRUE,
			envVarQcApply		= 	TRUE;

	char	bufferData 			[256];


static	int	RestartFunc	 	(int, KEY_TAB *);
static	int	ExitFunc	 	(int, KEY_TAB *);

	static KEY_TAB listKeys [] =
	{
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};

/*
 * Function Declarations
 */
float 	CalcMendStk 		(long);
int  	CheckColn 			(long);
void 	AddQuantity 		(long, char *, float);
void 	CalcConv 			(void);
void 	CalculateIncc  		(long);
void 	CloseDB 			(void);
void 	DelSoic 			(long, long);
void    InitDisplay			(void);
void 	OpenDB 				(void);
void 	ProcessCmrd 		(long);
void 	ProcessInmr 		(long, int);
void 	ProcessItln 		(long);
void 	ProcessPcms 		(long);
void 	ProcessPcwo2 		(long);
void 	ProcessPcwo 		(long);
void 	ProcessPoln 		(long);
void 	ProcessQchr 		(long);
void 	ProcessSokt 		(long);
void 	ProcessSoln 		(long);
void    shutdown_prog 		(void);
void	UpdateIncc			(long);
void	UpdateInmr			(long);

void PauseFunc (void);
/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		i,
			errorFound 	= TRUE,
			noErrors	= TRUE;

	/*
	 * Check if contract management installed, saves on processing.
	 */
	sptr = chk_env ("CM_INSTALLED");
	envVarCmInstalled = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if Manufacturing installed, saves on processing.
	 */
	sptr = chk_env ("MA_INSTALLED");
	envVarMaInstalled = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if Quality Control used, saves on processing.
	 */
	sptr = chk_env ("QC_APPLY");
	envVarQcApply = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
  	 * Clear screen etc. 
 	 */
	InitDisplay ();

	/*
  	 * Open Database files.
 	 */
	OpenDB ();

	/*
  	 * Process while inventory master file.
 	 */
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, "                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		/*
  	  	 * Ignore synonym items.
 	 	 */
		if (inmr_rec.hhsi_hash != 0L)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		/*
  	  	 * Ignore non stock items.
 	 	 */
		if (NON_STOCK)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		if (noInTab)
			redraw_line (ItemTab, FALSE);

		/*
  	  	 * Add entry to tab screen.
 	 	 */
		tab_add 
		(
			ItemTab, 
			"%s| %16.16s | %40.40s |%63.63s  %010ld", 
			" ",
			inmr_rec.item_no,
			inmr_rec.description,
			" ",
			inmr_rec.hhbr_hash
		);
		tab_get (ItemTab, bufferData, EQUAL, noInTab);
		ProcessInmr (inmr_rec.hhbr_hash, TRUE);
		redraw_line (ItemTab, TRUE);
		noInTab++;
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	/*
  	 * Second section fixed reported errors.
 	 */
	move (0,23);cl_line();
	clear_mess ();
	rv_pr (ML ("Phase one complete. Review changes and press EDIT/END when ready to review any inconsistencies. "),0,23,1);

	tab_scan (ItemTab);
	move (0,23);cl_line();
	clear_mess ();
	rv_pr (ML ("Phase two, correct items with inconsistencies. "),0,23,1);
	abc_selfield (inmr, "inmr_hhbr_hash");
	errorFound = TRUE;
	while (errorFound)
	{
		errorFound = FALSE;
		for (i = 0; i < noInTab ; i++)
		{
			redraw_line (ItemTab, FALSE);
			tab_get (ItemTab, bufferData, EQUAL, i);
			if (tagged (bufferData))
			{	
				errorFound 	= TRUE;
				noErrors	= FALSE;
				inmr_rec.hhbr_hash	=	atol (bufferData + 129);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				if (!cc)
				{
					ProcessInmr (inmr_rec.hhbr_hash, FALSE);
					redraw_line (ItemTab, TRUE);
					ProcessInmr (inmr_rec.hhbr_hash, TRUE);
				}
				break;
			}
		}
	}
	rv_pr (ML ("Phase two, check and correct any inconsistencies. "),0,23,1);
	clear_mess ();
	for (i = 0; i < 5; i++)
	{
		move (0,23);cl_line();
		if (noErrors)
			rv_pr (ML ("Update complete, no inconsistencies found."),0,23,(i % 2) ? 1 : 0);
		else
			rv_pr (ML ("Update complete, all inconsistencies corrected."),0,23,(i % 2) ? 1 : 0);
		sleep (1);
	}
	tab_scan (ItemTab);
	tab_close (ItemTab, TRUE);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 *   Standard program exit sequence     
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
	abc_dbopen (data);

	abc_alias (sokt2, sokt);
	abc_alias (inmr2, inmr);
	abc_alias (inmr3, inmr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	if (envVarCmInstalled)
		open_rec (cmrd,  cmrd_list, CMRD_NO_FIELDS, "cmrd_hhbr_hash");

	if (envVarMaInstalled)
	{
		abc_alias (pcwo2, pcwo);
		open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_mabr_hash");
		open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhbr_hash");
		open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	}
	if (envVarQcApply)
	{
		open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no3");
		open_rec (qcln,  qcln_list, QCLN_NO_FIELDS, "qcln_id_no");
	}
		
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhcu_hash");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inme,  inme_list, INME_NO_FIELDS, "inme_hhwh_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr3, inmr_list, INMR_NO_FIELDS, "inmr_hhsi_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_hhcu_hash");
	open_rec (soic,  soic_list, SOIC_NO_FIELDS, "soic_id_no2");
	open_rec (sokt,  sokt_list, SOKT_NO_FIELDS, "sokt_mabr_hash");
	open_rec (sokt2, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inmr3);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (soln);
	abc_fclose (soic);
	if (envVarCmInstalled)
		abc_fclose (cmrd);

	if (envVarQcApply)
	{
		abc_fclose (qchr);
		abc_fclose (qcln);
	}
	if (envVarMaInstalled)
	{
		abc_fclose (pcwo);
		abc_fclose (pcwo2);
		abc_fclose (pcms);
	}
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (cohr);
	abc_fclose (sohr);
	abc_fclose (itln);
	abc_fclose (sokt);
	abc_fclose (sokt2);
	
	abc_fclose (inme);
	abc_fclose (coln);

	abc_dbclose (data);
}
/*
 * Process all the stuff hanging off inmr.
 */
void
ProcessInmr (
	long	hhbrHash,
	int		checkOnly)
{
	int		i;
	int		errorWO	=	0,
			errorOO = 	0,
			errorCO	=	0,
			errorBO	=	0,
			errorFO	=	0,
			errorQC	=	0;
	
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&warehouse_d, &warehouses, sizeof (struct ItemCalc), 10);
	warehouseCount = 0;
		
	/*
	 * What now I have to process a Phantom item, 
	 * what is a phantom, see BOM system.		 
	 */
	 if (PHANTOM)
		 return;
 /*
	 if (PHANTOM)
	 {
		 sokt2_rec.hhbr_hash	=	hhbrHash;
		 cc = find_rec (sokt2, &sokt2_rec, GTEQ, "r");
		 while (!cc && sokt2_rec.hhbr_hash == hhbrHash)
		 {
			 ProcessInmr (sokt2_rec.mabr_hash, checkOnly);
			 cc = find_rec (sokt2, &sokt2_rec, NEXT, "r");
		 }
		 inmr2_rec.hhbr_hash	=	hhbrHash;
		 cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		 if (cc)
			 return;
	}
*/
	/*
 	 * Check and calculate what the system thinks is on incc.
 	 */
	CalculateIncc (hhbrHash);

	/*
 	 * Not a phantom item to only need to process stuff once.
 	 */
	if (!PHANTOM)
	{
		ProcessPoln 	(hhbrHash);	/* Process purchase orders		*/
		ProcessItln 	(hhbrHash); /* Process transfers.			*/
		ProcessSoln 	(hhbrHash); /* Process sales orders.		*/
		ProcessSokt 	(hhbrHash);	/* Process kits					*/
		if (envVarMaInstalled)		/* Process Manufacturing stuff	*/
		{
			ProcessPcwo 	(hhbrHash);
			ProcessPcwo2 	(hhbrHash);
			ProcessPcms 	(hhbrHash);
		}
		if (envVarQcApply)
			ProcessQchr 	(hhbrHash); /* Process QC stuff			*/
		if (envVarCmInstalled)
			ProcessCmrd 	(hhbrHash); /* Process Contract man 	*/

		/*
		 * Process Synonyms.
		 */
		inmr3_rec.hhsi_hash = hhbrHash;
		cc = find_rec (inmr3, &inmr3_rec, GTEQ, "r");
		while (!cc && inmr3_rec.hhsi_hash == inmr_rec.hhbr_hash)
		{
			/*
			 * Zero incc Record (s) for Synonym item.
			 */
			ProcessPoln 	(inmr3_rec.hhbr_hash);
			ProcessItln 	(inmr3_rec.hhbr_hash);
			ProcessSoln 	(inmr3_rec.hhbr_hash);
			if (envVarMaInstalled)
			{
				ProcessPcwo 	(inmr3_rec.hhbr_hash);
				ProcessPcwo2 	(inmr3_rec.hhbr_hash);
				ProcessPcms 	(inmr3_rec.hhbr_hash);
			}
			if (envVarQcApply)
				ProcessQchr 	(inmr3_rec.hhbr_hash);
			if (envVarCmInstalled)
					ProcessCmrd 	(inmr3_rec.hhbr_hash);

			cc = find_rec (inmr3, &inmr3_rec, NEXT, "r");
		}
	}

	/*
	 * Check if values on file match with calculated values.
	 */
	for (i = 0; i < warehouseCount; i++)
	{
		if (warehouses [i].wo_actual != warehouses [i].wo_calc)
			errorWO++;
		if (warehouses [i].oo_actual != warehouses [i].oo_calc)
			errorOO++;
		if (warehouses [i].co_actual != warehouses [i].co_calc)
			errorCO++;
		if (warehouses [i].bo_actual != warehouses [i].bo_calc)
			errorBO++;
		if (warehouses [i].fo_actual != warehouses [i].fo_calc)
			errorFO++;
		if (warehouses [i].qc_actual != warehouses [i].qc_calc)
			errorQC++;
	}
	/*
	 * All AOK.
	 */
	if (!errorWO && !errorFO && !errorBO && !errorOO && !errorCO && !errorQC)
		strcpy (err_str, ML ("No problems found with item."));
	/*
	 * Not all AOK.
	 */
	else
		strcpy (err_str, "Errors : ");

	/*
	 * Add problem to end report string.
	 */
	if (errorWO)
		strcat (err_str, ML ("works order /"));
	if (errorFO)
		strcat (err_str, ML ("forward order"));
	if (errorBO)
		strcat (err_str, ML ("back order /"));
	if (errorOO)
		strcat (err_str, ML ("on order /"));
	if (errorCO)
		strcat (err_str, ML ("committed /"));
	if (errorQC)
		strcat (err_str, ML ("QC /"));

	/*
	 * Update line with message.
	 */
	tab_update 
	(
		ItemTab, 
		"%-64.64s%-63.63s  %010ld", 
		bufferData,
		err_str,
		hhbrHash
	);
	/*
	 * Tag line as containing an error.
	 */
	if (errorWO || errorFO || errorBO || errorOO || errorCO || errorQC)
		tag_set (ItemTab);
	else
		tag_unset (ItemTab);
		
	/*
	 * Used in final update process to correct problems.
	 */
	if (!checkOnly)
	{
		UpdateIncc	(inmr_rec.hhbr_hash);
		UpdateInmr	(inmr_rec.hhbr_hash);
	}
	/*
	 * Delete arrary
	 */
	ArrDelete (&warehouse_d);
}

/*
 * Process Customer Orders and update B (ackorders) or Others.
 */
void
ProcessSoln (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Process all customer  lines.
	 */
	soln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && hhbrHash == soln_rec.hhbr_hash) 
	{
		qty =  soln_rec.qty_order + soln_rec.qty_bord;
		if (soln_rec.status [0] == 'B' || soln_rec.status [0] == 'F')
			AddQuantity (soln_rec.hhcc_hash, soln_rec.status, qty);
		else
		{
			if (soln_rec.qty_bord > 0.00)
				AddQuantity (soln_rec.hhcc_hash, "B", soln_rec.qty_bord);

			if (CheckColn (soln_rec.hhsl_hash))
				AddQuantity (soln_rec.hhcc_hash, "M", soln_rec.qty_order);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

/*
 * Process QC records. 
 */
void
ProcessQchr (
	long	hhbrHash)
{
	/*
	 * Process all customer  lines.
	 */
	strcpy (qchr_rec.co_no, comm_rec.co_no);
	qchr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc && hhbrHash == qchr_rec.hhbr_hash) 
	{
		strcpy (ccmr_rec.co_no, qchr_rec.co_no);
		strcpy (ccmr_rec.est_no, qchr_rec.br_no);
		strcpy (ccmr_rec.cc_no, qchr_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}
		qcln_rec.hhqc_hash	=	qchr_rec.hhqc_hash;
		cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
		while (!cc && qcln_rec.hhqc_hash ==	qchr_rec.hhqc_hash)
		{
			AddQuantity 
			(
				ccmr_rec.hhcc_hash, 
				"Q",
				qcln_rec.rel_qty + qcln_rec.rej_qty
			);
			cc = find_rec (qcln, &qcln_rec, NEXT, "r");
		}
		cc = find_rec (qchr, &qchr_rec, NEXT, "r");
	}
}
/*
 * Routine ensures packing slips already posted are not included in committed.
 */
int
CheckColn (
	long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, COMPARISON, "r");
	if (cc)
		return (TRUE);

	if (coln_rec.status [0] == 'P')
		return (TRUE);

	return (FALSE);
}
			
/*
 * Process Purchase order records and update On order stock.
 */
void
ProcessPoln (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Process all customer lines.
	 */
	poln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && hhbrHash == poln_rec.hhbr_hash) 
	{
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (!cc && pohr_rec.drop_ship [0] != 'Y')
		{
			qty = poln_rec.qty_ord - poln_rec.qty_rec;
			if (qty <= 0.00)
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}
			AddQuantity (poln_rec.hhcc_hash, "O", qty);
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
}

/*
 * Process works orders and updates the on order
 * quantities, for the final product.          
 */
void
ProcessPcwo (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Works Orders.
	 */
	pcwo_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == hhbrHash)
	{
		/*
		 * Ignore closed, deleted works orders.
		 */
		if (pcwo_rec.order_status [0] == 'Z' ||
		    pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		qty = pcwo_rec.prod_qty - 
		      (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
		if (qty <= 0.00)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		AddQuantity (pcwo_rec.hhcc_hash, "O", qty);

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
}

/*
 * Process the Anticipated Manufacture Quantities for all Works Orders.
 */
void
ProcessPcwo2 (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Works Orders.
	 */
	pcwo_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == hhbrHash)
	{
		/*
		 * Ignore closed, deleted works orders.
		 */
		if (pcwo_rec.order_status [0] == 'Z' ||
		    pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		strcpy (ccmr_rec.co_no, pcwo_rec.co_no);
		strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
		strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}

		qty = pcwo_rec.prod_qty - 
		      (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
		if (qty <= 0.00)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
		AddQuantity (ccmr_rec.hhcc_hash, "A", qty);

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}
}

/*
 * Process BOM and updates the committed quantities,
 * for works orders material items.                
 */
void
ProcessPcms (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Update committed quantities for each component - pcms.
	 */
	pcms_rec.mabr_hash	=	hhbrHash;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.mabr_hash == hhbrHash)
	{
		/*
		 * check works order, if deleted, closed or get next pcms record
		 */
		pcwo_rec.hhwo_hash	=	pcms_rec.hhwo_hash;
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		if (cc ||
			pcwo_rec.order_status [0] == 'Z' ||
		    pcwo_rec.order_status [0] == 'D')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		inmr2_rec.hhbr_hash	=	pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		CalcConv ();

		qty = pcms_rec.matl_qty;		/* qty of source required */
		pcms_rec.matl_wst_pc += 100;	/* calc waste % */
		pcms_rec.matl_wst_pc /= 100;
		qty *= pcms_rec.matl_wst_pc;	/* add waste % */
		qty -= pcms_rec.qty_issued;		/* less actual qty issued */

		if (qty <= 0.00)
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}
		AddQuantity 
		(
			pcwo_rec.hhcc_hash, 
			"C", 
			n_dec (qty / cnv_fct, inmr_rec.dec_pt)
		);
		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
}

void
CalcConv (void)
{
	float	std_cfact;
	float	alt_cfact;
	char	std_group [21];
	char	alt_group [21];

	number	std_cnv_fct;
	number	alt_cnv_fct;
	number	pcms_cnv_fct;
	number	result;
	number	uom_cfactor;

	/*
	 * Get the UOM conversion factor
	 */
	inum_rec.hhum_hash	=	inmr2_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	alt_cfact = inum_rec.cnv_fct;

	inum_rec.hhum_hash	=	inmr2_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	std_cfact = inum_rec.cnv_fct;

	inum_rec.hhum_hash	=	pcms_rec.uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	/*
	 * Converts a float to arbitrary precision number defined as number.
	 */
	NumFlt (&std_cnv_fct, std_cfact);
	NumFlt (&alt_cnv_fct, alt_cfact);
	NumFlt (&pcms_cnv_fct, inum_rec.cnv_fct);

	/*
	 * a function that divides one number by another and places  
	 * the result in another number defined variable             
	 * Conversion factor = std uom cnv_fct / iss uom cnv_fct     
	 *      OR                                                   
	 * Conversion factor = (std uom cnv_fct / iss uom cnv_fct)   
	 *                     * item's conversion factor            
	 * Same calculations as in pc_recprt.                        
	 */
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &pcms_cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &pcms_cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*
	 * converts a arbitrary precision number to a float.
	 */
	cnv_fct = NumToFlt (&result);

	return;
}

/*
 * Process Customer Orders and update B (ackorders) or Others.
 */
void
ProcessCmrd (
	long	hhbrHash)
{
	float	qty = 0.00;

	/*
	 * Process all customer  lines.
	 */
	cmrd_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhbr_hash == hhbrHash) 
	{
		if (cmrd_rec.stat_flag [0] == 'C')
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		qty = cmrd_rec.qty_order + cmrd_rec.qty_border;
		if (qty <= 0.00)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}
		AddQuantity (cmrd_rec.hhcc_hash, cmrd_rec.stat_flag, qty);

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
	}
}
/*
 * Process Stock transfers.
 */
void
ProcessItln (
	long	hhbrHash)
{
	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == hhbrHash)
	{
		if (!VALID_ITLN)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		switch (itln_rec.status [0])
		{
		case 'T':
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O" , 
				itln_rec.qty_order + itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"B" , 
				itln_rec.qty_border
			);
			break;

		case 'B':
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"B" , 
				itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O" , 
				itln_rec.qty_border + itln_rec.qty_order
			);
			break;

		case 'U':
		case 'M':
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"C", 
				itln_rec.qty_order + itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O", 
				itln_rec.qty_order + itln_rec.qty_border
			);
			break;
	
		default :
			break;

		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	abc_selfield (itln, "itln_r_hhbr_hash");

	itln_rec.r_hhbr_hash = hhbrHash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.r_hhbr_hash == hhbrHash)
	{
		if (!VALID_ITLN)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		switch (itln_rec.status [0])
		{
		case 'T':
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O", 
				itln_rec.qty_order + itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"B" , 
				itln_rec.qty_border
			);
			break;

		case 'B':
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"B", 
				itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O" , 
				itln_rec.qty_border + itln_rec.qty_order
			);
			break;

		case 'U':
		case 'M':
			AddQuantity 
			(
				itln_rec.i_hhcc_hash, 
				"C" , 
				itln_rec.qty_order + itln_rec.qty_border
			);
			AddQuantity 
			(
				itln_rec.r_hhcc_hash, 
				"O" , 
				itln_rec.qty_order + itln_rec.qty_border
			);
			break;
	
		default :
			break;
		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return;
}
/*
 * Process Customer Orders and update B (ackorders) or Others. 
 */
void
ProcessSokt (
	long	hhbrHash)
{
	float	qty = 0.00;

	sokt_rec.mabr_hash = hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.mabr_hash == hhbrHash)
	{
		/*
		 * Process all customer  lines. 
		 */
		soln_rec.hhbr_hash	=	sokt_rec.hhbr_hash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && sokt_rec.hhbr_hash == soln_rec.hhbr_hash) 
		{
			qty =  soln_rec.qty_order + soln_rec.qty_bord;
			qty *= sokt_rec.matl_qty;

			if (qty <= 0.00)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
			AddQuantity 
			(
				soln_rec.hhcc_hash, 
				(soln_rec.status [0] == 'O') ? "M" : soln_rec.status, 
				qty
			);
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	return;
}
/*
 * Calculate values from all incc records for current item.
 */
void
CalculateIncc (
	long	hhbrHash)
{
	float	mendQty	=	0.0;

	incc_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhbr_hash == hhbrHash)
	{
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&warehouse_d, warehouses, warehouseCount))
			sys_err ("ArrChkLimit (warehouses)", ENOMEM, PNAME);
		
		warehouses [warehouseCount].hhccHash	=	incc_rec.hhcc_hash;
		warehouses [warehouseCount].wo_actual	=	incc_rec.wo_qty_anti;
		warehouses [warehouseCount].oo_actual	=	incc_rec.on_order;
		warehouses [warehouseCount].co_actual	=	incc_rec.committed;
		warehouses [warehouseCount].bo_actual	=	incc_rec.backorder;
		warehouses [warehouseCount].qc_actual	=	incc_rec.qc_qty;
		warehouses [warehouseCount++].fo_actual	=	incc_rec.forward;
		mendQty	=	CalcMendStk (incc_rec.hhwh_hash);
		if (mendQty != 0.0)
		{
			AddQuantity 
			(
				incc_rec.hhcc_hash, 
				"C", 
				mendQty
			);
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
}
/*
 * Update inventory warehouse file with calculated values.
 */
void
UpdateIncc (
	long	hhbrHash)
{
	int		i;

	abc_selfield (incc, "incc_id_no");

	for (i = 0; i < warehouseCount; i++)
	{
		incc_rec.hhbr_hash	=	hhbrHash;
		incc_rec.hhcc_hash	=	warehouses [i].hhccHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (incc);
			continue;
		}
		if 
		(
			warehouses [i].wo_calc	== 	incc_rec.wo_qty_anti &&
			warehouses [i].oo_calc	==	incc_rec.on_order &&
			warehouses [i].co_calc	==	incc_rec.committed &&
			warehouses [i].bo_calc	==	incc_rec.backorder &&
			warehouses [i].fo_calc	==	incc_rec.forward &&
			warehouses [i].qc_calc	==	incc_rec.qc_qty
		)
		{
			abc_unlock (incc);
			continue;
		}
		incc_rec.wo_qty_anti	=	warehouses [i].wo_calc;
		incc_rec.on_order		=	warehouses [i].oo_calc;
		incc_rec.committed		=	warehouses [i].co_calc;
		incc_rec.backorder		=	warehouses [i].bo_calc;
		incc_rec.forward		=	warehouses [i].fo_calc;
		incc_rec.qc_qty			=	warehouses [i].qc_calc;
		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBUPDATE");

		/*
		 * 'Delete' soic records for specified PID.
		 */
		DelSoic 
		(
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash
		);
	}
	abc_selfield (incc, "incc_hhbr_hash");
}
/*
 * Update inventory master file with calculated values.
 */
void
UpdateInmr (
	long	hhbrHash)
{
	int		i;

	float	totalWO	=	0.00,
			totalOO	=	0.00,
			totalCO	=	0.00,
			totalBO	=	0.00,
			totalFO	=	0.00,
			totalQC	=	0.00;

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (inmr);
		return;
	}
	for (i = 0; i < warehouseCount; i++)
	{
		totalWO +=	warehouses [i].wo_calc;
		totalOO +=	warehouses [i].oo_calc;
		totalCO +=	warehouses [i].co_calc;
		totalBO +=	warehouses [i].bo_calc;
		totalFO +=	warehouses [i].fo_calc;
		totalQC +=	warehouses [i].qc_calc;
	}
	if 
	(
		totalWO == 	inmr_rec.wo_qty_anti &&
		totalOO ==	inmr_rec.on_order &&
		totalCO ==	inmr_rec.committed &&
		totalBO ==	inmr_rec.backorder &&
		totalFO ==	inmr_rec.forward &&
		totalQC ==	inmr_rec.qc_qty
	)
	{
		abc_unlock (inmr);
		return;
	}
	inmr_rec.wo_qty_anti	=	totalWO;
	inmr_rec.on_order		=	totalOO;
	inmr_rec.committed		=	totalCO;
	inmr_rec.backorder		=	totalBO;
	inmr_rec.forward		=	totalFO;
	inmr_rec.qc_qty			=	totalQC;
	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, inmr, "DBUPDATE");

	return;
}
/*
 * Add quantity to structure.
 */
void
AddQuantity (
	long	hhccHash,
	char	*type,
	float	qty)
{
	int		i;
	for (i = 0; i < warehouseCount; i++)
	{
		if (warehouses [i].hhccHash ==	hhccHash)
		{
			switch (type [0])
			{
				case 'A':
					warehouses [i].wo_calc += qty;
					break;
		
				case 'B':
					warehouses [i].bo_calc += qty;
					break;
		
				case 'F':
				case 'G':
					warehouses [i].fo_calc += qty;
				break;
		
				case 'O':
					warehouses [i].oo_calc += qty;
				break;

				case 'Q':
					warehouses [i].qc_calc += qty;
				break;

				default:
					warehouses [i].co_calc += qty;
			}
		}
	}
}

/*
 * Restart key processing function.
 */
static	int
RestartFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	restart = TRUE;
	return key;
}

/*
 * Exit key processing function.
 */
static	int
ExitFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	return key;
}

/*
 *   Initialise screen output.
 */
void
InitDisplay (void)
{
	init_scr ();
	set_tty ();
	clear ();
	swide ();
	crsr_off ();
	noInTab	=	0;
	tab_open (ItemTab, listKeys, 2, 0, 16, FALSE);
	tab_add (ItemTab, "#%-s", "S|   Item Number    |           Item Description               | Status Description                                              ");
	rv_pr (ML ("Stock committed, backorder, on order, forward order and QA integrity check"),30,1,1);
	move (0,23);cl_line();
	rv_pr (ML ("Phase one, verify all items and flag those with inconsistencies"),0,23,1);
	
}

/*
 *   Calculate month end held over balances.
 */
float
CalcMendStk (
	long	hhwhHash)
{
	float	AddStock 	= 	0.00;

	inme_rec.hhwh_hash = hhwhHash;
	cc = find_rec (inme, &inme_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	AddStock = 	inme_rec.sales;
	return (AddStock);
}
/*
 * Flag all soic records associated with the specified PID as status 'D'.
 */
void
DelSoic (
	long	hhbrHash,
	long	hhccHash)
{
	strcpy (soic_rec.status, "A");
	soic_rec.hhbr_hash  = hhbrHash;
	soic_rec.hhcc_hash  = hhccHash;
	cc = find_rec (soic, &soic_rec, GTEQ, "u");
	while (!cc && 	soic_rec.status [0] == 'A' &&
					soic_rec.hhbr_hash == hhbrHash &&
					soic_rec.hhcc_hash == hhccHash)
	{
		strcpy (soic_rec.status, "D");
		soic_rec.qty	=	0.00;
		cc = abc_update (soic, &soic_rec);
		if (cc)
			file_err (cc, soic, "DBUPDATE");

		strcpy (soic_rec.status, "A");
		soic_rec.hhbr_hash  = hhbrHash;
		soic_rec.hhcc_hash  = hhccHash;
		cc = find_rec (soic, &soic_rec, GTEQ, "u");
	}
	abc_unlock (soic);
}
