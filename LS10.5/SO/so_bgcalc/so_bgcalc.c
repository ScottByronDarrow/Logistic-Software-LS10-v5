/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_bgcalc.c,v 5.7 2002/04/30 07:56:46 scott Exp $
|  Program Name  : (so_bgcalc.c)
|  Program Desc  : (Recalculate Committed And On Order From)
|                  (Order Entry and Invoice From Backlog.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/10/1988       |
|---------------------------------------------------------------------|
| $Log: so_bgcalc.c,v $
| Revision 5.7  2002/04/30 07:56:46  scott
| Update for new Archive modifications;
|
| Revision 5.6  2002/03/20 03:01:59  scott
| Updated for comments
|
| Revision 5.5  2002/01/02 08:59:31  cha
| Updated to make sure that values are preserve when
| making a recursive call in ProcessInmr().
|
| Revision 5.4  2001/10/23 07:16:33  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.3  2001/09/11 23:19:04  scott
| Updated from Scott machine - 12th Sep 2001
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_bgcalc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bgcalc/so_bgcalc.c,v 5.7 2002/04/30 07:56:46 scott Exp $";

#include	<pslscr.h>
#include	<signal.h>
#include	<alarm_time.h>
#include	<twodec.h>
#include	<number.h>
#include	<arralloc.h>

#define		PO_CALC 	(!strcmp (sobg_rec.type, "RP"))
#define		STK_CALC 	(!strcmp (sobg_rec.type, "RC"))
#define		ORD_CALC 	(!strcmp (sobg_rec.type, "RO"))
#define		STOCK_UP 	(!strcmp (sobg_rec.type, "SU"))

#define		BAD_HASH 	(sobg_rec.hash <= 0L)

#define		VALID_ITLN 	(itln_rec.status [0] == 'T' || \
						 itln_rec.status [0] == 'B' || \
						 itln_rec.status [0] == 'M' || \
						 itln_rec.status [0] == 'U')

#define		NON_STOCK	(inmr_rec.inmr_class [0] == 'N' || \
						 inmr_rec.inmr_class [0] == 'Z')

#define		PHANTOM		(inmr_rec.inmr_class [0] == 'P')

#define		INVOICE		(cohr_rec.type [0] == 'I')
#define		PSLIP		(cohr_rec.type [0] == 'P')
#define		CREDIT		(cohr_rec.type [0] == 'C')

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct cmrdRecord	cmrd_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct cumrRecord	cumr_rec;
struct inccRecord	incc_rec;
struct inmeRecord	inme_rec;
struct inmrRecord	inmr2_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct itlnRecord	itln_rec;
struct pcmsRecord	pcms_rec;
struct pcwoRecord	pcwo_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct qchrRecord	qchr_rec;
struct qclnRecord	qcln_rec;
struct sobgRecord	sobg2_rec;
struct sobgRecord	sobg_rec;
struct sohrRecord	sohr_rec;
struct soicRecord	soic_rec;
struct soktRecord	sokt2_rec;
struct soktRecord	sokt_rec;
struct solnRecord	soln_rec;

	char	*data	=	"data",
			*incc2	=	"incc2",
			*inmr2	=	"inmr2",
			*inmr3	=	"inmr3",
			*pcwo2	=	"pcwo2",
			*sobg2	=	"sobg2",
			*sokt2	=	"sokt2";

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
	} *warehouses, *prevwarehouses;

	DArray	warehouse_d;
	int		warehouseCount		=	0;

	float	cnv_fct				=	0.00;

	char	validStatusCodes [11];
	int		envVarDbNettUsed 	= 	TRUE,
			envVarCnNettUsed	=	TRUE,
			envVarCmInstalled	=	FALSE,
			envVarMaInstalled	= 	TRUE,
			envVarQcApply		= 	FALSE;
/*
 * Function Declarations
 */
double 	CalculateLineValue 	(void);
float 	CalcMendStk 		(long);
int  	AllZero 			(void);
int  	CheckColn 			(long);
int  	CheckIncc 			(long);
void 	AddQuantity 		(long, char *, float);
void 	CalcConv 			(void);
void 	CalculateIncc  		(long);
void 	CalculateMaster		(long);
void 	CloseDB 			(void);
void 	DeleteDupSobg 		(void);
void 	DelSoic 			(long, int, long, long);
void 	MainProcessRoutine 	(void);
void 	OpenDB 				(void);
void 	ProcessCmrd 		(long);
void 	ProcessCohr 		(long);
void 	ProcessInmr 		(long);
void 	ProcessItln 		(long);
void 	ProcessPcms 		(long);
void 	ProcessPcwo2 		(long);
void 	ProcessPcwo 		(long);
void 	ProcessPoln 		(long);
void 	ProcessQchr 		(long);
void 	ProcessSokt 		(long);
void 	ProcessSoln 		(long);
void	UpdateIncc			(long);
void	UpdateInmr			(long);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char 	*argv [])
{
	char	*sptr;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CM_INSTALLED");
	envVarCmInstalled = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("MA_INSTALLED");
	envVarMaInstalled = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("QC_APPLY");
	envVarQcApply = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	/*
	 * Process for all parameters - if given
	 */
	strcpy (validStatusCodes, "MCFB76HG");

	if (argc > 1)
		sprintf (validStatusCodes, "%-10.10s", argv [1]);

	MainProcessRoutine ();
	CloseDB ();
	FinishProgram ();
	return (EXIT_SUCCESS);
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
	abc_alias (incc2, incc);
	abc_alias (sobg2, sobg);

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
		open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_hhbr_hash");
		open_rec (qcln,  qcln_list, QCLN_NO_FIELDS, "qcln_id_no");
	}

	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhcu_hash");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inme,  inme_list, INME_NO_FIELDS, "inme_hhwh_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhsi_hash");
	open_rec (inmr3, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (sobg,  sobg_list, SOBG_NO_FIELDS, "sobg_id_no_2");
	open_rec (sobg2, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
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
	abc_fclose (sobg);
	abc_fclose (sobg2);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inmr3);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (soln);
	abc_fclose (soic);
	if (envVarCmInstalled)
		abc_fclose (cmrd);

	if (envVarMaInstalled)
	{
		abc_fclose (pcwo);
		abc_fclose (pcwo2);
		abc_fclose (pcms);
	}
	if (envVarQcApply)
	{
		abc_fclose (qchr);
		abc_fclose (qcln);
	}
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (cumr);
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
 * Process sobg records where sobg_type = "RC" or sobg_type = RO
 */
void
MainProcessRoutine (void)
{
	signal_on ();

	/*
	 * Process Until Prog Exit
	 */
	while (!prog_exit)
	{
		/*
		 * Wait until all SU (Stock update) records have been processed.
		 */
		strcpy (sobg_rec.type, "SU");
		sobg_rec.lpno 	= 0;
		sobg_rec.hash 	= -1L;
		cc = find_rec (sobg, &sobg_rec, GTEQ, "r");
		if (!cc && STOCK_UP)
		{
			time_out ();
			continue;
		}

		/*
		 * Initialise to sobg record
		 */
		strcpy (sobg_rec.type, "RC");
		sobg_rec.lpno = 0;
		sobg_rec.hash = -1L;
		cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
		while (!prog_exit && !cc)
		{
			if (!ORD_CALC && !STK_CALC && !PO_CALC)
			{
				abc_unlock (sobg);
				break;
			}
			if (BAD_HASH)
			{
				abc_delete (sobg);
				break;
			}

			/*
			 * Process all stock recalculate records.
			 */
			if (STK_CALC || PO_CALC)
			{
				abc_selfield (soln, "soln_hhbr_hash");
				ProcessInmr (sobg_rec.hash);
				/*
				 * 'Delete' soic records for specified PID.
				 */
				DelSoic
				(
					sobg_rec.pid,
					sobg_rec.last_line,
					sobg_rec.hash,
					sobg_rec.hash2
				);
				/*
				 * Remove other sobg records for this item.
				 */
				abc_unlock (sobg);
				DeleteDupSobg ();
			}

			if (ORD_CALC)
			{
				abc_selfield (soln, "soln_id_no");
				ProcessCohr (sobg_rec.hash);
			}

			/*
			 * Delete sobg record as processed.
			 */
			abc_delete (sobg);
			abc_unlock (sobg);

			strcpy (sobg_rec.type, "SU");
			sobg_rec.lpno 	= 0;
			sobg_rec.hash 	= -1L;
			cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
			if (!cc && STOCK_UP)
			{
				abc_unlock (sobg);
				break;
			}

			/*
			 * Initialise to sobg record.
			 */
			strcpy (sobg_rec.type, "RC");
			sobg_rec.lpno = 0;
			sobg_rec.hash = -1L;
			cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
		}
		abc_unlock (sobg);

		/*
		 * Ran out of records to process so start timing
		 */
		time_out ();
	}
}

/*
 * Process All Packing Slips, Invoices and Credits.
 */
void
ProcessCohr (
	long	hhcuHash)
{
	double	ord_value = 0.00;

	/*
	 * Process Invoice / Packing slip file.
	 */
	cohr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && cohr_rec.hhcu_hash == hhcuHash)
	{
		if (strchr (validStatusCodes, cohr_rec.stat_flag [0]) == (char *)0)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}

		if (INVOICE || PSLIP)
		{
		    if (envVarDbNettUsed)
				ord_value += cohr_rec.gross - cohr_rec.disc;
		    else
				ord_value += cohr_rec.gross;
		}
		if (CREDIT)
		{
		    if (envVarCnNettUsed)
				ord_value -= cohr_rec.gross - cohr_rec.disc;
		    else
				ord_value -= cohr_rec.gross;
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	/*
	 * Process Orders File.
	 */
	sohr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && sohr_rec.hhcu_hash == hhcuHash)
	{
		if ((strchr (validStatusCodes, sohr_rec.status [0]) == (char *)0) ||
			sohr_rec.status [0] == 'P')
		{
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
			continue;
		}

		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			/*
			 * Cannot include credit check or held orders.
			 */
			if (soln_rec.status [0] == 'H' || soln_rec.status [0] == 'C')
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
			if (strchr (validStatusCodes, soln_rec.status [0]) == (char *)0)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
			ord_value += CalculateLineValue ();
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cumr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (cumr);
		return;
	}

	cumr_rec.ord_value = ord_value;

	cc = abc_update (cumr, &cumr_rec);
	if (cc)
		file_err (cc, cumr, "DBUPDATE");

	return;
}

/*
 * Calculate value of order line.
 */
double
CalculateLineValue (void)
{
	double	line_val	= 0.00,
			l_total		= 0.00,
			l_disc		= 0.00;

	if ((soln_rec.qty_order + soln_rec.qty_bord) <= 0.00)
		return (0.00);

	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (0.00);

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) soln_rec.qty_order + soln_rec.qty_bord;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarDbNettUsed)
			line_val	=	l_total - l_disc;
		else
			line_val	=	l_total;
	}
	return (line_val);
}
/*
 * Process Stock Master File.
 */
void
ProcessInmr (
	long	hhbrHash)
{
	/*
	 * Allocate the initial array.
	 */
	int prevWhCount = 0;

	ArrAlloc (&warehouse_d, &warehouses, sizeof (struct ItemCalc), 10);
	warehouseCount = 0;

	/*
	 * Find item for passed hhbr_hash.
	 */
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 * Item is a Synonym , Get master item.
	 */
	if (inmr_rec.hhsi_hash != 0L)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			inmr_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
	}
	/*
	 * Set hhbr_hash to current item.
	 */
	hhbrHash = inmr_rec.hhbr_hash ;

	/*
 	 * Check and calculate what the system thinks is on incc.
 	 */
	CalculateIncc (hhbrHash);

	/*
	 * What now I have to process a Phantom item,
	 * what is a phantom, see BOM system.
	 */
	if (PHANTOM)
	{
		sokt2_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (sokt2, &sokt2_rec, GTEQ, "r");
		while (!cc && sokt2_rec.hhbr_hash == hhbrHash)
		{
			prevWhCount = warehouseCount;
			prevwarehouses = warehouses;
			ProcessInmr (sokt2_rec.mabr_hash);
			warehouses = prevwarehouses;
			warehouseCount = prevWhCount;
			cc = find_rec (sokt2, &sokt2_rec, NEXT, "r");
		}
		inmr_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			return;
	}

	if (!PHANTOM)
	{
		ProcessPoln 	(hhbrHash);
		ProcessItln 	(hhbrHash);
		ProcessSoln 	(hhbrHash);
		ProcessSokt 	(hhbrHash);
		if (envVarMaInstalled)
		{
			ProcessPcwo 	(hhbrHash);
			ProcessPcwo2 	(hhbrHash);
			ProcessPcms 	(hhbrHash);
		}
		if (envVarCmInstalled)
			ProcessCmrd 	(hhbrHash);

		if (envVarQcApply)
			ProcessQchr 	(hhbrHash);

		/*
		 * Process Synonyms.
		 */
		inmr2_rec.hhsi_hash = hhbrHash;
		cc = find_rec (inmr2, &inmr2_rec, GTEQ, "r");
		while (!cc && inmr2_rec.hhsi_hash == inmr_rec.hhbr_hash)
		{
			ProcessPoln 	(inmr2_rec.hhbr_hash);
			ProcessItln 	(inmr2_rec.hhbr_hash);
			ProcessSoln 	(inmr2_rec.hhbr_hash);
			if (envVarMaInstalled)
			{
				ProcessPcwo 	(inmr2_rec.hhbr_hash);
				ProcessPcwo2 	(inmr2_rec.hhbr_hash);
				ProcessPcms 	(inmr2_rec.hhbr_hash);
			}
			if (envVarCmInstalled)
				ProcessCmrd 	(inmr2_rec.hhbr_hash);

			if (envVarQcApply)
				ProcessQchr 	(inmr2_rec.hhbr_hash);

			cc = find_rec (inmr2, &inmr2_rec, NEXT, "r");
		}
	}
	UpdateInmr (hhbrHash);
	UpdateIncc (hhbrHash);

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
		if (qty <= 0.00)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
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
		cc = find_rec (inmr3, &inmr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr3, "DBFIND");

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
 * Process QC records.
 */
void
ProcessQchr (
	long	hhbrHash)
{
	/*
	 * Process all customer  lines.
	 */
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
 * Process Customer Orders and update B(ackorders) or Others.
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
 * Flag all soic records associated with the specified PID as status 'D'.
 */
void
DelSoic (
	long	delPid,
	int 	lastLine,
	long	hhbrHash,
	long	hhccHash)
{
	soic_rec.hhbr_hash  = hhbrHash;
	soic_rec.hhcc_hash  = hhccHash;
	strcpy (soic_rec.status, "A");
	cc = find_rec (soic, &soic_rec, GTEQ, "u");
	while (!cc && 	soic_rec.status [0] == 'A' &&
					soic_rec.hhbr_hash == hhbrHash &&
					soic_rec.hhcc_hash == hhccHash)
	{
		if (soic_rec.pid == delPid)
		{
			strcpy (soic_rec.status, "D");
			soic_rec.qty = 0.00;
			/*
			 * Removed check on update as it is possible for another
			 * record to be created between finding this record and
			 * updating it. The error will be picked up when the GTEQ
			 * call is executed.
			 */
			cc = abc_update (soic, &soic_rec);
			if (cc)
			{
				abc_unlock (soic);
				cc = find_rec (soic, &soic_rec, NEXT, "u");
			}
			abc_unlock (soic);

			soic_rec.hhbr_hash  = hhbrHash;
			soic_rec.hhcc_hash  = hhccHash;
			strcpy (soic_rec.status, "A");
			soic_rec.pid  = delPid;
			soic_rec.line = 0;
			cc = find_rec (soic, &soic_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (soic);
			cc = find_rec (soic, &soic_rec, NEXT, "u");
		}
	}
	abc_unlock (soic);
}

/*
 * Check if duplicate sobg_id_no's exist
 */
void
DeleteDupSobg (void)
{
	int cc;

	sobg2_rec = sobg_rec;
	cc = find_rec (sobg2, &sobg2_rec, GTEQ, "u");
	while (!cc && !strcmp (sobg2_rec.co_no, sobg_rec.co_no) &&
			   	  !strcmp (sobg2_rec.br_no, sobg_rec.br_no) &&
			      !strcmp (sobg2_rec.type, sobg_rec.type) &&
			      sobg2_rec.lpno == sobg_rec.lpno &&
			      sobg2_rec.hash == sobg_rec.hash)
	{
		if (memcmp (&sobg2_rec, &sobg_rec, sizeof sobg_rec)
		 && sobg2_rec.hash2 == sobg_rec.hash2)
		{
			DelSoic
			(
				sobg2_rec.pid,
				sobg2_rec.last_line,
				sobg2_rec.hash,
				sobg2_rec.hash2
			);
			abc_delete (sobg2);
			abc_unlock (sobg2);
			sobg2_rec = sobg_rec;
			cc = find_rec (sobg2, &sobg2_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (sobg2);
			cc = find_rec (sobg2, &sobg2_rec, NEXT, "u");
		}
	}
	abc_unlock (sobg2);
}

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
 * Calculate values from all incc records for current item.
 */
void
CalculateIncc (
	long	hhbrHash)
{
	float	mendQty	=	0.0;

	incc_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incc2, &incc_rec, GTEQ, "r");
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
		cc = find_rec (incc2, &incc_rec, NEXT, "r");
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
	}
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

	for (i = 0; i < warehouseCount; i++)
	{
		totalWO +=	warehouses [i].wo_calc;
		totalOO +=	warehouses [i].oo_calc;
		totalCO +=	warehouses [i].co_calc;
		totalBO +=	warehouses [i].bo_calc;
		totalFO +=	warehouses [i].fo_calc;
		totalQC +=	warehouses [i].qc_calc;
	}

	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (inmr);
		return;
	}
	if
	(
		totalWO == 	inmr_rec.wo_qty_anti &&
		totalOO ==	inmr_rec.on_order 	 &&
		totalCO ==	inmr_rec.committed 	 &&
		totalBO ==	inmr_rec.backorder 	 &&
		totalFO ==	inmr_rec.forward 	 &&
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
