/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_stkup.c,v 5.6 2002/01/09 01:12:13 scott Exp $
|  Program Name  : (so_stkup.c)
|  Program Desc  : (Update Stock From Customer Invoice File)
|---------------------------------------------------------------------|
|  Date Written  : 03/11/1986      |  Author     : Scott Darrow       |
|---------------------------------------------------------------------|
| $Log: so_stkup.c,v $
| Revision 5.6  2002/01/09 01:12:13  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.5  2001/11/08 08:39:29  scott
| Updated to process as per batch control if number plates are active.
|
| Revision 5.4  2001/08/31 01:36:17  cha
| Updated to reflect Scott's changes to make
| sure that inla is handled correctly.
|
| Revision 5.3  2001/08/09 09:22:07  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:52:06  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:05  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_stkup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_stkup/so_stkup.c,v 5.6 2002/01/09 01:12:13 scott Exp $";

#include	<pslscr.h>
#include	<proc_sobg.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include	<twodec.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	"schema"
#include	<Costing.h>

#define	INVOICE		 (transactionTypeFlag [0] == 'I' || \
					  transactionTypeFlag [0] == 'P' || \
					  transactionTypeFlag [0] == 'T')

#define	FAULTY  	 (coln_rec.crd_type [0] == 'F')
#define	DOLLAR  	 (coln_rec.crd_type [0] == 'D')

#define	NON_STOCK  	 (inmr_rec.inmr_class [0] == 'N' || \
			  		  inmr_rec.inmr_class [0] == 'Z' )

#define	PHANTOM  	 (inmr_rec.inmr_class [0] == 'P')
#define	SERIAL     	 (inmr_rec.costing_flag [0] == 'S')
#define	CO_INV		 (envCoClose [2] == '1')
#define	FUTURE		 (transactionMonthEndDate > inventoryMonthEndDate)
#define	NOTAX	 	 (cohr_rec.tax_code [0] == 'A' || \
			  		  cohr_rec.tax_code [0] == 'B')

#include	<twodec.h>

/*
 * Table Names
 */
static char 	*data	= 	"data";

/*
 * Globals
 */
	char	envCurrCode 		[4],
			findStatusFlag 		[2],
			transactionTypeFlag [2],
			updateStatusFlag 	[2],
			branchNumber 		[3],
			envCoClose 			[6],
			systemDate 			[11];

	int		invoiceMonth 		= 0,  
			background 			= 0,
			envMultLoc 			= FALSE,
			envSkGrinNoPlate 	= FALSE,
			envSkBatchCont 		= FALSE,
			envSkGrades			= FALSE,
			envDbMcurr			= 0;

	long	lsystemDate 			= 0L,
			invoiceDate 			= 0L,
			transactionMonthEndDate = 0L,
			inventoryMonthEndDate 	= 0L,
			currentProcessID		= 0L;

	float	envGstInclusive 		= 0.00,
			gstDivide 				= 0.00;

	double	totalCostPrice 			= 0.00;

	/*
	 * Table structures.
	 */
	struct	commRecord	comm_rec;
	struct	comrRecord	comr_rec;
	struct	esmrRecord	esmr_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	sobgRecord	sobg_rec;
	struct	inmrRecord	inmr_rec;
	struct	ingdRecord	ingd_rec;
	struct	cumrRecord	cumr_rec;
	struct	cohrRecord	cohr_rec;
	struct	colnRecord	coln_rec;
	struct	ffdmRecord	ffdm_rec;
	struct	inccRecord	incc_rec;
	struct	inwuRecord	inwu_rec;
	struct	inmeRecord	inme_rec;
	struct	inmuRecord	inmu_rec;
	struct	intrRecord	intr_rec;
	struct	soktRecord	sokt_rec;
	struct	inlaRecord	inla_rec;
	struct	inloRecord	inlo_rec;
	struct	inafRecord	inaf_rec;
	struct	llihRecord	llih_rec;

	float	*incc_con	=	&incc_rec.c_1;
	double	*incc_val	=	&incc_rec.c_val_1;
	double	*incc_prf	=	&incc_rec.c_prf_1;

/*
 * Function Declarations
 */
void	AddInventoryTran	(double, double, float, long, long, long, int);
void	CloseDB 			(void);
void	OpenDB 				(void);
void	ProcessHeader 		(char *, char *);
void	ProcessTransaction 	(long);
void	ProcessLine 		(double,double,double,float,long,long,char *,int);
void	ProcessKitItem 		(long, float);
double	OutGst 				(double);
int		ReadInmr			(long, long);
int		CalcMonth 			(long);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char 	*argv[])
{
	char	*sptr;

	/*
	 * Get native currency.
	 */
	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*
	 * Check for multi-currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for inventory grades.
	 */
	sptr = chk_env ("SK_GRADES");
	if (sptr != (char *)0 && *sptr == 'Y')
		envSkGrades = TRUE;

	sptr = chk_env ("GST_INCLUSIVE");
	if (sptr)
		envGstInclusive = (float) (atoi (sptr));

	if (envGstInclusive != 0.00)
		gstDivide = (float) ( (100.00 + envGstInclusive) / envGstInclusive);

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *) 0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	sptr = chk_env ("SK_BATCH_CONT");
	envSkBatchCont = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("MULT_LOC");
	envMultLoc = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = (sptr == (char *) 0) ? 0 : atoi (sptr);

	if (envSkGrinNoPlate)
		envSkBatchCont = TRUE;

	if (argc != 4 && argc != 5)
	{
		print_at(0,0,"Usage : %s <findStatusFlag> <updateStatusFlag> <transactionTypeFlag> - optional <br_no>\n\r",argv [0]);
		return (EXIT_FAILURE);
	}
	sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	sprintf (findStatusFlag,"%-1.1s",argv [1]);
	sprintf (updateStatusFlag,"%-1.1s",argv [2]);
	sprintf (transactionTypeFlag,"%-1.1s",argv [3]);

	OpenDB ();

	if (argc == 5)
		sprintf (branchNumber,"%2.2s",argv [4]);
	else
		strcpy (branchNumber,comm_rec.est_no);

	/*
	 * Set to zero for first time.
	 */
	ccmr_rec.hhcc_hash = 0L;

	sprintf (err_str," Processing %s to Stock.", (INVOICE) ? "Invoices" 
							     						   : "Credits");
	dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

	ProcessHeader (comm_rec.co_no,branchNumber);

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,  ccmr_list,  CCMR_NO_FIELDS,  "ccmr_hhcc_hash");
	open_rec (cohr,  cohr_list,  COHR_NO_FIELDS,  "cohr_up_id");
	open_rec (coln,  coln_list,  COLN_NO_FIELDS,  "coln_id_no");
	open_rec (comr,  comr_list,  COMR_NO_FIELDS,  "comr_co_no");
	open_rec (cumr,  cumr_list,  CUMR_NO_FIELDS,  "cumr_hhcu_hash");
	open_rec (esmr,  esmr_list,  ESMR_NO_FIELDS,  "esmr_id_no");
	open_rec (incc,  incc_list,  INCC_NO_FIELDS,  "incc_id_no");
	open_rec (ffdm,  ffdm_list,  FFDM_NO_FIELDS,  "ffdm_id_no2");
	open_rec (inmr,  inmr_list,  INMR_NO_FIELDS,  "inmr_hhbr_hash");
	open_rec (inme,  inme_list,  INME_NO_FIELDS,  "inme_hhwh_hash");
	open_rec (inmu,  inmu_list,  INMU_NO_FIELDS,  "inmu_id_no");
	open_rec (intr,  intr_list,  INTR_NO_FIELDS,  "intr_id_no2");
	open_rec (inaf,  inaf_list,  INAF_NO_FIELDS,  "inaf_id_no");
	open_rec (inwu,  inwu_list,  INWU_NO_FIELDS,  "inwu_id_no");
	open_rec (inla,  inla_list,  INLA_NO_FIELDS,  "inla_hhcl_id");
	open_rec (inlo,  inlo_list,  INLO_NO_FIELDS,  "inlo_inlo_hash");
	if (envSkBatchCont)
		open_rec (llih,  llih_list,  LLIH_NO_FIELDS,  "llih_hhcl_hash");
	if (envSkGrades)
		open_rec (ingd, ingd_list, INGD_NO_FIELDS, "ingd_id_no");
	open_rec (sokt, sokt_list,SOKT_NO_FIELDS,"sokt_hhbr_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ffdm);
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (inme);
	abc_fclose (inmu);
	abc_fclose (intr);
	abc_fclose (inaf);
	abc_fclose (inla);
	abc_fclose (inwu);
	abc_fclose (sokt);
	if (envSkBatchCont)
		abc_fclose (llih);
	if (envSkGrades)
		abc_fclose (ingd);

	CloseCosting ();
	abc_dbclose (data);
}

/*
 * Process Invoice Header. 
 */
void
ProcessHeader (
	char	*companyNumber,
	char	*branchNumber)
{

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();

	memset (&cohr_rec, 0, sizeof (cohr_rec));
	memset (&sokt_rec, 0, sizeof (sokt_rec));
	memset (&coln_rec, 0, sizeof (coln_rec));
	memset (&cumr_rec, 0, sizeof (cumr_rec));

	while (1)
	{
		strcpy (cohr_rec.co_no, companyNumber);
		strcpy (cohr_rec.br_no, branchNumber);
		strcpy (cohr_rec.type, transactionTypeFlag);
		strcpy (cohr_rec.stat_flag, findStatusFlag);

		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (cohr);
			break;
		}
		if (CO_INV)
		{
			strcpy (comr_rec.co_no, cohr_rec.co_no);
			cc = find_rec (comr,&comr_rec,COMPARISON,"r");
			if (cc)
				file_err (cc, "comr", "DBFIND");

			inventoryMonthEndDate = MonthEnd (comr_rec.inv_date);
		}
		else
		{
			sprintf (esmr_rec.co_no,  cohr_rec.co_no);
			sprintf (esmr_rec.est_no, cohr_rec.br_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "esmr", "DBFIND");

			inventoryMonthEndDate = MonthEnd (esmr_rec.inv_date);
		}
		transactionMonthEndDate = MonthEnd (cohr_rec.date_raised); 


		dsp_process ((INVOICE) ? " Invoice : " : " Credit Note : ",cohr_rec.inv_no);
		invoiceDate = cohr_rec.date_raised;
		invoiceMonth = CalcMonth (cohr_rec.date_raised) - 1;

		ProcessTransaction (cohr_rec.hhco_hash); 

    	abc_unlock (cohr);
	}
	recalc_sobg ();
}

/*
 * Process all lines on invoice.
*/
void
ProcessTransaction (
	long	hhcoHash)
{
	/*
	 * Find Customer Master File Record
	 */
	cumr_rec.hhcu_hash 	= cohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		strcpy (cumr_rec.dbt_no, "DELETE");

   	/*
   	 * Process all order lines.
   	 */
	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
   	while (!cc && coln_rec.hhco_hash == hhcoHash)
   	{
   		if (coln_rec.stat_flag [0] == updateStatusFlag [0])
		{
			abc_unlock (coln);
			cc = find_rec (coln, &coln_rec, NEXT, "u");
			continue;
		}

		if (cohr_rec.drop_ship [0] != 'Y')
		{
			cc = ReadInmr (coln_rec.hhbr_hash, coln_rec.incc_hash);
			if (!cc && coln_rec.q_order != 0.00)
			{
				if (PHANTOM)
				{
					ProcessKitItem  
					(
						coln_rec.hhbr_hash, 
						coln_rec.q_order 
					);
				}
				else
				{
					ProcessLine 
					(
						coln_rec.gross + 
						coln_rec.erate_var,
						coln_rec.amt_disc,
						coln_rec.cost_price,
						coln_rec.q_order,
						coln_rec.incc_hash,
						coln_rec.hhum_hash,
						coln_rec.serial_no, 
						FALSE
					);
				}
			}
			else
			{
				abc_unlock (incc);
				abc_unlock (inmr);
			}
		}
    	strcpy (coln_rec.status, " ");
    	strcpy (coln_rec.stat_flag, updateStatusFlag);
		cc = abc_update (coln, &coln_rec);
		if (cc) 
			file_err (cc, "coln", "DBUPDATE");

		add_hash 
		(
			cohr_rec.co_no, 
			cohr_rec.br_no, 
			"RC", 
			0, 
			coln_rec.hhbr_hash, 
			coln_rec.incc_hash,
			currentProcessID,
			(double) 0.00
		);
		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	abc_unlock (coln);
	abc_unlock (inmr);
	abc_unlock (incc);

   	strcpy (cohr_rec.stat_flag, updateStatusFlag);
   	cc = abc_update (cohr, &cohr_rec);
   	if (cc)
		file_err (cc, "cohr", "DBUPDATE");
}

/*
 * Process all lines on invoice/credit and update relevent files.
 */
void	
ProcessLine (
	double	lineGross,
	double	lineDisc,
	double	lineCost,
	float	lineQuantity,
	long	lineHhccHash,
	long	lineHhumHash,
	char	*lineSerialNo,
	int		phantomLine)
{
	double	cost1 		= 0.00;
	double	cost2 		= 0.00;
	double	cost 		= 0.00;
	double	workValue 	= 0.00;
	double	localNett 	= 0.00;
	char	*tptr;
	
	/*
	 * Just in case set to space for invoices.
	 */
	if (INVOICE)
		strcpy (coln_rec.crd_type, " ");

	/*
	 * Extract out gst if required.
	 */
	if (gstDivide != 0.00)
	{
		lineGross = OutGst (lineGross);
		lineDisc  = OutGst (lineDisc);
	}

	/*
	 * Warehouse is not the same so get correct warehouse.
	 */
	if (ccmr_rec.hhcc_hash != lineHhccHash)
	{
		ccmr_rec.hhcc_hash	=	lineHhccHash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str,"Error in ccmr (%06ld) (DBFIND)",lineHhccHash);
			sys_err (err_str,cc,PNAME);
		}
	}
	/*
	 * Process A(verage) L(ast) S(erial) F(ifo) L(ifo) P(revious) S(T)andard
	 */
	switch (inmr_rec.costing_flag [0])
	{
	case	'A':
	case	'L':
	case	'P':
	case	'T':
		cost = 	FindIneiCosts 
				(
					inmr_rec.costing_flag,
					cohr_rec.br_no,
					inmr_rec.hhbr_hash
				);
		break;

	case	'S':
		if (!FAULTY && !DOLLAR)
		{
			cost = 	FindInsfCost 
					(
						incc_rec.hhwh_hash, 
						0L,
						lineSerialNo,
						(INVOICE) ? "C" : "S"
					);
			if (cost == -1.00)
			{
				cost = 	FindInsfCost 
						(
							incc_rec.hhwh_hash, 
							0L,
							lineSerialNo,
							(INVOICE) ? "F" : "C"
						);
			}
			if (cost == -1.00)
			{
				cost = 	FindInsfCost 
						(
							0L,
							incc_rec.hhbr_hash, 
							lineSerialNo,
							(INVOICE) ? "C" : "S"
						);
			}
			if (cost == -1.00)
			{
				cost = 	FindInsfCost 
						(
							0L,
							incc_rec.hhbr_hash, 
							lineSerialNo,
							(INVOICE) ? "F" : "C"
						);
			}
		}
		break;

	case	'F':
		if (INVOICE)
		{
			cost = FindIncfCost
			(
				incc_rec.hhwh_hash,
				incc_rec.closing_stock,
				lineQuantity, 
				TRUE,
				inmr_rec.dec_pt
			);
		}
		else
		{
			ReduceIncf	
			(
				incc_rec.hhwh_hash,
				incc_rec.closing_stock + lineQuantity, 
				TRUE
			);
			cost =	FindIncfCost
					(
						incc_rec.hhwh_hash,
						incc_rec.closing_stock + lineQuantity, 
						lineQuantity, 
						TRUE,
						inmr_rec.dec_pt
					);
		}
		break;

	case	'I':
		if (INVOICE)
		{
			cost = 	FindIncfCost
					(
						incc_rec.hhwh_hash,
				 	   	incc_rec.closing_stock,
				 	   	lineQuantity, 	
						FALSE,
						inmr_rec.dec_pt
					);
		}
		else
		{
			ReduceIncf 
			(
				incc_rec.hhwh_hash,
				incc_rec.closing_stock + lineQuantity, 
				FALSE
			);
			cost = 	FindIncfCost
					(
						incc_rec.hhwh_hash,
						incc_rec.closing_stock + lineQuantity,
						lineQuantity,
						FALSE,
						inmr_rec.dec_pt
					);
		}
		break;

	default:
		break;
	}
	/*
	 * Cost not found so use Last Cost.
	 */
	if (cost < 0.00)
	{
		cost = 	FindIneiCosts 
				(
					"L",
					cohr_rec.br_no,
					inmr_rec.hhbr_hash
				);
	}

	/*
	 * Still no cost found so use Standard Cost. 
	 */
	if (cost <= 0.00)
	{
		cost = 	FindIneiCosts 
				(
					"T",
					cohr_rec.br_no,
					inmr_rec.hhbr_hash
				);
	}

	/*
	 * Leave cost is already there.
	 */
	if (lineCost == 0.0 || SERIAL)
	{
		coln_rec.cost_price	= CENTS (cost);
		lineCost			= CENTS (cost);
	}

	/*
	 * Calculate local nett value. 
	 */
	if (envDbMcurr && cohr_rec.exch_rate != 0.00)
		localNett = no_dec ((lineGross - lineDisc) / cohr_rec.exch_rate);
	else
		localNett = (lineGross - lineDisc);

	/*
	 * Updated for new demand.
	 */
	if (!NON_STOCK)
	{
		ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
		ffdm_rec.date		=	lsystemDate;
		strcpy (ffdm_rec.type, "1");
		cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
		if (cc)
		{
			if (INVOICE)
				ffdm_rec.qty	=	lineQuantity;
			else
			{
				if (!DOLLAR)
					ffdm_rec.qty	=  0 - lineQuantity;
			}
			cc = abc_add (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBADD");
		}
		else
		{
			if (INVOICE)
				ffdm_rec.qty	+=	lineQuantity;
			else
			{
				if (!DOLLAR)
					ffdm_rec.qty	-=  lineQuantity;
			}

			cc = abc_update (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBUPDATE");
		}
		abc_unlock (ffdm);
	}
		
	if (FUTURE)
	{
		inme_rec.hhwh_hash 	= incc_rec.hhwh_hash;
		cc = find_rec (inme, &inme_rec, EQUAL, "u");  
		if (cc)
		{
			memset (&inme_rec, 0, sizeof (inme_rec));
			inme_rec.hhwh_hash 	= incc_rec.hhwh_hash;
			cc = abc_add (inme, &inme_rec);
			if (cc)
				file_err (cc, inme, "DBADD");

			inme_rec.hhwh_hash 	= incc_rec.hhwh_hash;
			cc = find_rec (inme, &inme_rec, EQUAL, "u");  
			if (cc)
				file_err (cc, inme, "DBFIND");
		}
	}
	/*
	 * Process Invoices for current or Future.
	 */
	if (!NON_STOCK && !PHANTOM)
	{
		if (INVOICE)
		{
			inmr_rec.ltd_sales     += lineQuantity;

			if (FUTURE)
			{
				inme_rec.sales				+= lineQuantity;
				inme_rec.closing_stock 		= 	inme_rec.opening_stock +
												inme_rec.pur +
												inme_rec.receipts +
												inme_rec.adj -
												inme_rec.issues -
												inme_rec.sales;
			}
			else
			{
				inmr_rec.on_hand     -= lineQuantity;
				incc_rec.sales  		+= lineQuantity;
				incc_rec.closing_stock 	= 	incc_rec.opening_stock +
											incc_rec.pur +
											incc_rec.receipts +
											incc_rec.adj -
											incc_rec.issues -
											incc_rec.sales;

				incc_rec.ytd_sales += lineQuantity;

				/*
				 * Find Warehouse unit of measure file.
				 */
				inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
				inwu_rec.hhum_hash	=	lineHhumHash,
				cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
				if (cc)
				{
					memset (&inwu_rec, 0, sizeof (inwu_rec));
					inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
					inwu_rec.hhum_hash	=	lineHhumHash;
					cc = abc_add (inwu, &inwu_rec);
					if (cc)
						file_err (cc, "inwu", "DBADD");

					inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
					inwu_rec.hhum_hash	=	lineHhumHash;
					cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
					if (cc)
						file_err (cc, "inwu", "DBFIND");
				}
				inwu_rec.sales	+= lineQuantity;
				inwu_rec.closing_stock = inwu_rec.opening_stock +
										 inwu_rec.pur +
										 inwu_rec.receipts +
										 inwu_rec.adj -
										 inwu_rec.issues -
										 inwu_rec.sales;

				cc = abc_update (inwu,&inwu_rec);
				if (cc)
					file_err (cc, "inwu", "DBUPDATE");
			}
		}
		/*
		 * Process Credit Notes for current or Future.
		 */
		else
		{
			if (!DOLLAR)
				inmr_rec.ltd_sales -= lineQuantity;

			if (FUTURE)
			{
				inme_rec.sales				-= 	lineQuantity;
				inme_rec.closing_stock 		= 	inme_rec.opening_stock +
												inme_rec.pur +
												inme_rec.receipts +
												inme_rec.adj -
												inme_rec.issues -
												inme_rec.sales;
			}
			else
			{
				if (!FAULTY && !DOLLAR)
				{
					inmr_rec.on_hand       += lineQuantity;
					incc_rec.sales  		  -= lineQuantity;
					incc_rec.closing_stock 	= 	incc_rec.opening_stock +
													incc_rec.pur +
													incc_rec.receipts +
													incc_rec.adj -
													incc_rec.issues -
													incc_rec.sales;

					incc_rec.ytd_sales -= lineQuantity;

					/*
					 * Find Warehouse unit of measure file. 
					 */
					inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
					inwu_rec.hhum_hash	=	lineHhumHash,
					cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
					if (cc)
					{
						memset (&inwu_rec, 0, sizeof (inwu_rec));
						inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
						inwu_rec.hhum_hash	=	lineHhumHash;
						cc = abc_add (inwu, &inwu_rec);
						if (cc)
							file_err (cc, "inwu", "DBADD");

						inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
						inwu_rec.hhum_hash	=	lineHhumHash;
						cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
						if (cc)
							file_err (cc, "inwu", "DBFIND");
					}
					inwu_rec.sales	-= lineQuantity;
					inwu_rec.closing_stock = inwu_rec.opening_stock +
											 inwu_rec.pur +
											 inwu_rec.receipts +
											 inwu_rec.adj -
											 inwu_rec.issues -
											 inwu_rec.sales;

					cc = abc_update (inwu,&inwu_rec);
					if (cc)
						file_err (cc, "inwu", "DBUPDATE");
				}
			}
		}
	}

	if ((envMultLoc || envSkBatchCont) && 
		!FAULTY && !DOLLAR && !NON_STOCK && !PHANTOM)
	{
		int		allocationFound	=	FALSE;
		float	allocationQty	=	0.00;
		/*
		 * Check if allocation is correct.
		 */
		inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		inla_rec.inlo_hash	=	0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "r");
		while (!cc && inla_rec.hhcl_hash == coln_rec.hhcl_hash)
		{
			allocationFound	=	TRUE;
			allocationQty	+=	(INVOICE) ? inla_rec.qty_alloc 
										  : inla_rec.qty_alloc * -1;

			cc = find_rec (inla, &inla_rec, NEXT, "r");
		}
		/*
		 * Something wrong with allocation so adjust last record.
		 */
		if (allocationFound == TRUE && allocationQty != lineQuantity)
		{
			float	diffQty	=	0.00;

			diffQty = lineQuantity - allocationQty;

			inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
			inla_rec.inlo_hash	=	0L;
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
			if (!cc && inla_rec.hhcl_hash == coln_rec.hhcl_hash)
			{
				if (INVOICE)
					inla_rec.qty_alloc += diffQty;
				else
					inla_rec.qty_alloc -= diffQty;

				cc = abc_update (inla, &inla_rec);
				if (cc)
					file_err (cc, inla, "DBUPDATE");
			}
			else
				abc_unlock (inla);
		}
		/*
		 * Check if allocation exists, if not then create.
		 */
		inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		inla_rec.inlo_hash	=	0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "r");
		if (cc || inla_rec.hhcl_hash != coln_rec.hhcl_hash)
		{
			abc_selfield (inlo, "inlo_mst_loc");

			inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inlo_rec.hhum_hash	=	coln_rec.hhum_hash;
			strcpy (inlo_rec.location, "          ");
			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
			if (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash &&
					   inlo_rec.hhum_hash == coln_rec.hhum_hash)
			{
				inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
				inla_rec.inlo_hash	=	inlo_rec.inlo_hash;
				inla_rec.pid		=	0;
				inla_rec.line_no	=	0;
				inla_rec.qty_alloc	=	(INVOICE) 	? lineQuantity 
													: lineQuantity * -1;
				cc = abc_add (inla, &inla_rec);
				if (cc)
					file_err (cc, inla, "DBADD");
			}
			else
			{
				inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
				inlo_rec.hhum_hash	=	0L;
				strcpy (inlo_rec.location, "          ");
				cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
				if (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
				{
					inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
					inla_rec.inlo_hash	=	inlo_rec.inlo_hash;
					inla_rec.pid		=	0;
					inla_rec.line_no	=	0;
					inla_rec.qty_alloc	=	(INVOICE) 	? lineQuantity 
														: lineQuantity * -1;
					cc = abc_add (inla, &inla_rec);
					if (cc)
						file_err (cc, inla, "DBADD");
				}
			}
			abc_selfield (inlo, "inlo_inlo_hash");
		}
	}

	/*
	 * Add inventory transactions unless Faulty goods supplied.
	 */
	if (!FAULTY)
	{
		AddInventoryTran
		(
			localNett,
			lineCost,
		  	lineQuantity,
		  	inmr_rec.hhbr_hash,
		  	lineHhccHash,
		  	lineHhumHash,
			phantomLine
		);
	}
	if ((envMultLoc || envSkBatchCont) && 
			!FAULTY && !DOLLAR && !NON_STOCK && !PHANTOM)
	{
		inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		inla_rec.inlo_hash	=	0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "u");
		while (!cc && inla_rec.hhcl_hash == coln_rec.hhcl_hash)
		{
			inlo_rec.inlo_hash	=	inla_rec.inlo_hash;
			cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
			if (cc)
			{
				inla_rec.qty_alloc	=	0.00;
				cc = abc_update (inla, &inla_rec);
				if (cc)
					file_err (cc, "inla", "DBDELETE");

				cc = abc_delete (inla);
				if (cc)
					file_err (cc, "inla", "DBDELETE");

				inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
				inla_rec.inlo_hash	=	0L;
				cc = find_rec (inla, &inla_rec, GTEQ, "u");
				continue;
			}

			inlo_rec.qty -= inla_rec.qty_alloc;
			inlo_rec.no_picks++;

			tptr		=	getenv ("LOGNAME");
			sprintf (inlo_rec.op_id, "%-14.14s", tptr);
			strcpy (inlo_rec.time_create, TimeHHMM ());
			inlo_rec.date_upd = TodaysDate ();
			cc = abc_update (inlo, &inlo_rec);
			if (cc)
				file_err (cc, "inlo", "DBUPDATE");

			inla_rec.qty_alloc	=	0.00;
			cc = abc_update (inla, &inla_rec);
			if (cc)
				file_err (cc, "inla", "DBDELETE");

			cc = abc_delete (inla);
			if (cc)
				file_err (cc, "inla", "DBDELETE");
			
			inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
			inla_rec.inlo_hash	=	0L;
			cc = find_rec (inla, &inla_rec, GTEQ, "u");
		}
		abc_unlock (inla);
	}

	if (INVOICE)
	{
		if (FUTURE)
		{
			inme_rec.qty	+= 	lineQuantity;
			inme_rec.value	+= 	localNett;

			cost1 = localNett;
			workValue = out_cost (lineCost, inmr_rec.outer_size);
			cost2 = (workValue * lineQuantity);
			inme_rec.profit += (cost1 - cost2);
		}
		else
		{
			/*
			 * Now update 12 months figures.
			 */
			incc_con [invoiceMonth] += lineQuantity;
			incc_val [invoiceMonth] += localNett;

			cost1 = localNett;
			workValue = out_cost (lineCost, inmr_rec.outer_size);
			cost2 = (workValue * lineQuantity);
			if (!phantomLine)
				incc_prf [invoiceMonth] += (cost1 - cost2);
		}
	}
	else
	{
		/*
		 * Now update 12 months figures.
		 */
		if (!FAULTY)
		{
			if (FUTURE)
			{
				if (!DOLLAR)
					inme_rec.qty 	-= lineQuantity;

				inme_rec.value 	-= localNett;

				cost1 = localNett;
				workValue = out_cost (lineCost, inmr_rec.outer_size);

				cost2 = (workValue * lineQuantity);
				inme_rec.profit -= (cost1 - cost2);
			}
			else
			{
				if (!DOLLAR)
					incc_con [invoiceMonth] -= lineQuantity;

				incc_val [invoiceMonth] -= localNett;

				cost1 = localNett;
				workValue = out_cost (lineCost, inmr_rec.outer_size);

				cost2 = (workValue * lineQuantity);
				if (!phantomLine)
					incc_prf [invoiceMonth] -= (cost1 - cost2);
			}
		}
	}
	if (FUTURE)
	{
		cc = abc_update (inme, &inme_rec);
		if (cc) 
			file_err (cc, inme, "DBUPDATE");
	}
	else
	{
		if (!NON_STOCK && !PHANTOM)
		{
			if (incc_rec.closing_stock <= 0.00)
			{
				if (incc_rec.os_date == 0L)
					incc_rec.os_date = comr_rec.inv_date;
			
				if (incc_rec.os_ldate == 0L)
					incc_rec.os_ldate = comr_rec.inv_date;
			}
		}
		cc = abc_update (incc, &incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");

		if (!NON_STOCK && !PHANTOM)
		{
			if (inmr_rec.on_hand <= 0.00)
				if (!strcmp (inmr_rec.ex_code, "   "))
					sprintf (inmr_rec.ex_code, "%-3.3s", "o/s");
		}
	
		cc = abc_update (inmr, &inmr_rec);
		if (cc) 
			file_err (cc, "inmr", "DBUPDATE");
		else
			abc_unlock (inmr);
	}
	if (!NON_STOCK && !PHANTOM && SERIAL)
	{
	    if (!FAULTY && !DOLLAR)
	    {
			cc	=	FindInsf 
					(
						0L,
						inmr_rec.hhbr_hash,
						lineSerialNo,
						(INVOICE) ? "C" : "S",
						"u"
					);
			if (cc)
			{
				abc_unlock (insf);
				cc	=	FindInsf 
						(
							0L,
							inmr_rec.hhbr_hash,
							lineSerialNo,
							(INVOICE) ? "F" : "C",
							"u"
						);
			}
			if (!cc)
			{
				insfRec.date_out = invoiceDate;
				strcpy (insfRec.status, (INVOICE) ? "S" : "F");
				insfRec.hhcu_hash = (INVOICE) ? cumr_rec.hhcu_hash : 0L;
				strcpy (insfRec.invoice_no, (INVOICE) ? cohr_rec.inv_no : "        ");
				cc = abc_update (insf,&insfRec);
				if (cc) 
					file_err (cc, insf, "DBUPDATE");
			}
			else
				abc_unlock (insf);
	    }
	}
}

/*
 * Find inventory master file record and warehouse stock record.
 */
int
ReadInmr (
	long	hhbrHash,
	long	hhccHash)
{
	inmr_rec.hhbr_hash = hhbrHash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, (FUTURE) ? "r" : "u");
   	if (cc)
	{
		abc_unlock (inmr);
		return (cc);
	}
	/*
	 * Check if Product has different stocking item.
	 */
	if (inmr_rec.hhsi_hash != 0L)
	{
		if (!FUTURE)
			abc_unlock (inmr);

		inmr_rec.hhbr_hash = inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, (FUTURE) ? "r" : "u");
		if (cc)
		{
			if (!FUTURE)
				abc_unlock (inmr);
			
			inmr_rec.hhbr_hash = hhbrHash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, (FUTURE) ? "r" : "u");
			if (cc)
			{
				if (!FUTURE)
					abc_unlock (inmr);
				return (cc);
			}
		}
	}

   	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
   	incc_rec.hhcc_hash = hhccHash;
   	cc = find_rec (incc, &incc_rec, COMPARISON, "u");
   	if (cc)
	{
		if (!FUTURE)
		{
			abc_unlock (inmr);
			abc_unlock (incc);
		}
	}
	return (cc);
}

/*
 * Add inventory transactions to intr and inmu files.
 */
void
AddInventoryTran (
	double	lineSale,
	double	lineCost,
	float	lineQuantity,
	long	lineHhbrHash,
	long	lineHhccHash,
	long	lineHhumHash,
	int		phantomLine)
{
	int		newRecord;
	double	saleGross 	= 0.00;
	double	costGross 	= 0.00;
	double	gradeFactor	= 0.00;
	float	AllocQty;

	/*
	 * Initilise files that will be updated.
	 */
	memset (&inmu_rec, 0, sizeof (inmu_rec));
	memset (&intr_rec, 0, sizeof (intr_rec));
	memset (&inla_rec, 0, sizeof (inla_rec));
	if (envSkBatchCont)
		memset (&llih_rec, 0, sizeof (llih_rec));

	costGross = no_dec (lineQuantity * lineCost);
	saleGross = no_dec (lineSale);

	/*
	 * Extract out gst if required.
	 */
	if (gstDivide != 0.00)
		saleGross = OutGst (saleGross);

	if (costGross == 0.00 && saleGross == 0.00 && lineQuantity == 0.00)
		return;

	/*
	 * Open Inventory Movements and transactions file.
	 */
	strcpy (inmu_rec.co_no,    cohr_rec.co_no);
	strcpy (inmu_rec.br_no,    cohr_rec.br_no);
	strcpy (inmu_rec.wh_no,    ccmr_rec.cc_no);
	strcpy (inmu_rec.inmu_class,    inmr_rec.inmr_class);
	strcpy (inmu_rec.category, inmr_rec.category);
	strcpy (inmu_rec.year,     "C");
	sprintf (inmu_rec.period,  "%02d", invoiceMonth + 1);
	
	if (envSkGrades)
	{
		gradeFactor = 1.00;
		strcpy (ingd_rec.co_no, cohr_rec.co_no);
		strcpy (ingd_rec.grade, inmr_rec.grade);
		cc = find_rec (ingd, &ingd_rec, COMPARISON, "r");
		if (!cc)
		{
			gradeFactor 	= 	(double) 100.00 - (double) ingd_rec.writedown;
			gradeFactor 	/= 	(double) 100.00;
		}
		costGross *= gradeFactor;
	}

	/*
	 * Get and update or Add Inventory Movements Transactions record. 
	 */
	newRecord = find_rec (inmu, &inmu_rec, COMPARISON, "u");
	if (newRecord)
	{
		inmu_rec.sal_dly   = 0.00;
		inmu_rec.sal_mty   = 0.00;
		inmu_rec.sal_qty   = 0.00;
		inmu_rec.icst_dly  = 0.00;
		inmu_rec.icst_mty  = 0.00;
		inmu_rec.crd_dly   = 0.00;
		inmu_rec.crd_mty   = 0.00;
		inmu_rec.crd_qty   = 0.00;
		inmu_rec.ccst_dly  = 0.00;
		inmu_rec.ccst_mty  = 0.00;
	}

	if (INVOICE)
	{
		inmu_rec.sal_dly  += saleGross;
		inmu_rec.sal_mty  += saleGross;
		inmu_rec.sal_qty  += lineQuantity;
		if (!phantomLine)
		{
			inmu_rec.icst_dly += costGross;
			inmu_rec.icst_mty += costGross;
		}
	}
	else
	{
		inmu_rec.crd_dly  += saleGross;
		inmu_rec.crd_mty  += saleGross;
		if (!DOLLAR)
		{
			inmu_rec.crd_qty  += lineQuantity;
			if (!FAULTY && !phantomLine)
			{
			
				inmu_rec.ccst_dly += costGross;
				inmu_rec.ccst_mty += costGross;
			}
		}
	}

	/*
	 * Not on file so create.
	 */
	if (newRecord)
	{
		strcpy (inmu_rec.stat_flag,"0");

		cc = abc_add (inmu,&inmu_rec);
		if (cc) 
			file_err (cc, inmu, "DBADD");

		abc_unlock (inmu);
	}
	else 
	{
		cc = abc_update (inmu, &inmu_rec);
		if (cc) 
			file_err (cc, inmu, "DBUPDATE");
	}
	if (!DOLLAR)
	{
		if (!envSkBatchCont)
		{
			/*
			 * Add inventory transaction.
			 */
			strcpy (intr_rec.co_no, cohr_rec.co_no);
			strcpy (intr_rec.br_no, cohr_rec.br_no);
			intr_rec.hhbr_hash	= lineHhbrHash;
			intr_rec.hhcc_hash	= lineHhccHash;
			intr_rec.hhum_hash	= lineHhumHash;
			intr_rec.type		= (INVOICE) ? 6 : 7;
			intr_rec.date		= invoiceDate;
			strcpy (intr_rec.batch_no, cohr_rec.batch_no);
			strcpy (intr_rec.ref1, cohr_rec.inv_no);
			strcpy (intr_rec.ref2, cumr_rec.dbt_no);
			intr_rec.qty 		= lineQuantity;
			intr_rec.cost_price = lineCost;
			if (lineQuantity != 0.00)
				intr_rec.sale_price = twodec (lineSale / lineQuantity);
			else
				intr_rec.sale_price = 0.00;
			strcpy (intr_rec.stat_flag, "0");
			cc = abc_add (intr, &intr_rec);
			if (cc) 
				file_err (cc, intr, "DBADD");

			strcpy (inaf_rec.co_no, cohr_rec.co_no);
			strcpy (inaf_rec.br_no, cohr_rec.br_no);
			strcpy (inaf_rec.wh_no, ccmr_rec.cc_no);
			inaf_rec.sys_date	=	lsystemDate;
			inaf_rec.hhbr_hash 	= 	lineHhbrHash;
			inaf_rec.hhcc_hash 	= 	lineHhccHash;
			inaf_rec.hhum_hash 	= 	lineHhumHash;
			inaf_rec.type = (INVOICE) ? 6 : 7;
			inaf_rec.date = invoiceDate;
			strcpy (inaf_rec.batch_no, cohr_rec.batch_no);
			strcpy (inaf_rec.ref1, cohr_rec.inv_no);
			strcpy (inaf_rec.ref2, cumr_rec.dbt_no);
			inaf_rec.qty = lineQuantity;
			inaf_rec.cost_price = lineCost;
			if (lineQuantity != 0.00)
				inaf_rec.sale_price = twodec (lineSale / lineQuantity);
			else
				inaf_rec.sale_price = 0.00;
			strcpy (inaf_rec.stat_flag, "0");
			cc = abc_add (inaf, &inaf_rec);
			if (cc) 
				file_err (cc, inaf, "DBADD");
		}
		else
		{
			inla_rec.hhcl_hash	=	coln_rec.hhcl_hash;
			inla_rec.inlo_hash	=	0L;
			cc = find_rec (inla, &inla_rec, GTEQ, "r");
			while (!cc && inla_rec.hhcl_hash == coln_rec.hhcl_hash)
			{
				inlo_rec.inlo_hash	=	inla_rec.inlo_hash;
				cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
				if (cc)
					strcpy (inlo_rec.lot_no, "       ");
	
				/*
				 * Add lot/location invoice history. 
				 */
				llih_rec.hhcl_hash	=	coln_rec.hhcl_hash;
				llih_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
				llih_rec.hhbr_hash	=	coln_rec.hhbr_hash;
				llih_rec.des_date	=	lsystemDate;
				llih_rec.qty		=	(INVOICE) ? inla_rec.qty_alloc
									 			  : inla_rec.qty_alloc * -1;
				strcpy (llih_rec.inv_no, 	cohr_rec.inv_no);
				strcpy (llih_rec.lot_no, 	inlo_rec.lot_no);
				strcpy (llih_rec.slot_no, 	inlo_rec.slot_no);
				strcpy (llih_rec.uom, 		inlo_rec.uom);
				cc = abc_add (llih, &llih_rec);
				if (cc)
					file_err (cc, "llih", "DBADD");

				/*
				 * Add inventory transaction.
				 */
				AllocQty = (INVOICE) ? inla_rec.qty_alloc
									 : inla_rec.qty_alloc * -1;
				strcpy (intr_rec.co_no, cohr_rec.co_no);
				strcpy (intr_rec.br_no, cohr_rec.br_no);
				intr_rec.hhbr_hash 	= lineHhbrHash;
				intr_rec.hhcc_hash 	= lineHhccHash;
				intr_rec.hhum_hash 	= lineHhumHash;
				intr_rec.type 		= (INVOICE) ? 6 : 7;
				intr_rec.qty 		= AllocQty;
				intr_rec.cost_price = lineCost;
				if (AllocQty != 0.00)
					intr_rec.sale_price 	= twodec (lineSale / AllocQty);
				else
					intr_rec.sale_price 	= 0.00;
				intr_rec.date 		= invoiceDate;
				strcpy (intr_rec.batch_no, inlo_rec.lot_no);
				strcpy (intr_rec.ref1, cohr_rec.inv_no);
				strcpy (intr_rec.ref2, cumr_rec.dbt_no);
				strcpy (intr_rec.stat_flag, "0");
				cc = abc_add (intr, &intr_rec);
				if (cc) 
					file_err (cc, intr, "DBADD");

				strcpy (inaf_rec.co_no, cohr_rec.co_no);
				strcpy (inaf_rec.br_no, cohr_rec.br_no);
				strcpy (inaf_rec.wh_no, ccmr_rec.cc_no);
				inaf_rec.sys_date	=	lsystemDate;
				inaf_rec.hhbr_hash 	= 	lineHhbrHash;
				inaf_rec.hhcc_hash 	= 	lineHhccHash;
				inaf_rec.hhum_hash 	= 	lineHhumHash;
				inaf_rec.type 		= (INVOICE) ? 6 : 7;
				inaf_rec.date 		= invoiceDate;
				strcpy (inaf_rec.batch_no, inlo_rec.lot_no);
				strcpy (inaf_rec.ref1, cohr_rec.inv_no);
				strcpy (inaf_rec.ref2, cumr_rec.dbt_no);
				inaf_rec.qty 		= AllocQty;
				inaf_rec.cost_price = lineCost;
				if (AllocQty != 0.00)
					inaf_rec.sale_price = twodec (lineSale / AllocQty);
				else
					inaf_rec.sale_price = 0.00;
				strcpy (inaf_rec.stat_flag, "0");
				cc = abc_add (inaf, &inaf_rec);
				if (cc) 
					file_err (cc, inaf, "DBADD");

				cc = find_rec (inla, &inla_rec, NEXT, "r");
			}
		}
	}
}

/*
 * Extract out GST for gst inclusive Prices.
 */
double	
OutGst (
	double	totalAmount)
{
	double	gstAmount = 0.00;

	if (totalAmount == 0)
		return (0.00);

	if (NOTAX)
		return (totalAmount);

	gstAmount = no_dec (totalAmount / gstDivide);
	
	totalAmount -= no_dec (gstAmount);

	return (totalAmount);
}

/*
 * Specific code to handle single level Bills.
 */
void
ProcessKitItem (
	long	hhbrHash,
	float	quantity)
{
	double	extendCost = 0.00;

	totalCostPrice = 0.00;

	sokt_rec.hhbr_hash = hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		if (ReadInmr (sokt_rec.mabr_hash, coln_rec.incc_hash))
		{
			cc = find_rec (sokt,&sokt_rec,NEXT,"r");
			continue;
		}

		ProcessLine
		(
			(double) 0.00, 	
			(double) 0.00, 
			(double) 0.00,
		   	sokt_rec.matl_qty * quantity,
		   	coln_rec.incc_hash,
		   	coln_rec.hhum_hash,
		   	coln_rec.serial_no,
			TRUE
		);
		
		extendCost = (double) sokt_rec.matl_qty;
		extendCost *= coln_rec.cost_price;
		extendCost = twodec (extendCost);

		totalCostPrice += extendCost;

		cc = find_rec (sokt,&sokt_rec,NEXT,"r");
	}

	cc = ReadInmr (coln_rec.hhbr_hash, coln_rec.incc_hash);
	if (cc)
		return;

	coln_rec.cost_price = totalCostPrice;

	ProcessLine 
	(
		coln_rec.gross + 
		coln_rec.erate_var,
		coln_rec.amt_disc,
		coln_rec.cost_price,
		coln_rec.q_order,
		coln_rec.incc_hash,
		coln_rec.hhum_hash,
		coln_rec.serial_no,
		FALSE
	);
	return;
}

/*
 * Calculate month from date passed to routine.
 */
int	
CalcMonth (
 long	inventoryDate)
{
	int		tmpMonth;

	DateToDMY (inventoryDate, NULL, &tmpMonth, NULL);
	return (tmpMonth);
}
