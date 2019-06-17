/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_autoDesConf.c,v 5.9 2002/06/25 09:15:37 scott Exp $
|  Program Name  : (so_autoDesConf.c)                               
|  Program Desc  : (Pipe version of Automatic Despatch Confirmation)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 24th April 2001  |
|---------------------------------------------------------------------|
| $Log: so_autoDesConf.c,v $
| Revision 5.9  2002/06/25 09:15:37  scott
| Updated to comment out fields not in standard.
|
| Revision 5.8  2002/05/31 09:21:46  cha
| Updated to correct some minor spelling errors.
|
| Revision 1.1  2002/04/05 03:46:36  cha
| Made Ascent specific for modification of SO-XML cancellation.
| Updated to check if coln_q_des=0 and cancels the packing slip.
|
| Revision 5.7  2002/04/30 07:56:44  scott
| Update for new Archive modifications;
|
| Revision 5.6  2002/04/29 07:47:12  scott
| Update for new Archive modifications;
|
| Revision 5.5  2001/08/09 09:20:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.4  2001/08/06 23:50:49  scott
| RELEASE 5.0
|
| Revision 5.3  2001/07/23 02:13:14  cha
| Updated to default corh_date_raised to comm_dbt_date
|
| Revision 5.2  2001/07/05 05:28:24  scott
| Updated to not use Informix CENTS as it rounds the value
|
| Revision 5.1  2001/07/01 02:16:37  scott
| Updated to change inal_value from money to double
|
| Revision 5.0  2001/06/19 08:18:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 1.4  2001/05/21 00:59:23  scott
| Updated for new changes made regarding item levy and multi currency
|
| Revision 1.3  2001/05/02 10:35:50  scott
| Updated to fix test on release date for item levy.
|
| Revision 1.2  2001/05/02 02:23:33  scott
| Updated to add item levy
|
| Revision 1.1  2001/04/24 06:37:32  scott
| New automatic Dispatch confirmation (PIPE/XML) version.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_autoDesConf.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_autoDesConf/so_autoDesConf.c,v 5.9 2002/06/25 09:15:37 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include	<so_autoDesConf.h>
#include	<proc_sobg.h>
#include	<Archive.h>

#define		SERIAL_ITEM	 (inmr_rec.serial_item [0] == 'Y')
#define		NON_STOCK	 (inmr_rec.inmr_class [0] == 'Z')
#define		SUR_CHARGE	 (sohr_rec.sohr_new [0] == 'Y' && \
		             (cumr_rec.sur_flag [0] == 'Y' || \
		                  cumr_rec.sur_flag [0] == 'y'))

#define 	AUTO_SK_UP	 (createFlag [0] == envVarAutoSkUp [0])

#include 	<twodec.h>

	int		noTaxCharged			= 0,
			combinedInvoicePacking 	= FALSE,
			envVarDbNettUsed 		= TRUE,
			envVarAdvertLevy 		= FALSE,
			envVarSoDoi				= FALSE,
			envVarSoRtDelete 		= FALSE,
			auditReportDone 		= FALSE;

	double	lineLevyAmt 	= 0.00,
			lineLevyPc  	= 0.00,
			lineLevy 		= 0.00,
			totalTax 		= 0.00,
			totalLine 		= 0.00,
			lineDiscount 	= 0.00,
			lineTax 		= 0.00,
			lineGst 		= 0.00;

	char	createFlag [2],
			envVarAutoSkUp [2];

	long	despatchDate = 0L;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct sobgRecord	sobg_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln2_rec;
struct excfRecord	excf_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct inalRecord	inal_rec;
struct pcwoRecord   pcwo_rec;

	char	*soln2 	= "soln2", 
			*data 	= "data";

/*
 * Function Declarations 
 */
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ReadMisc 			 (void);
void 	CheckSalesman 		 (void);
void	StartProgram 		 (void);
int  	Update 				 (void);
int  	FindCustomerDetails (void);
int 	ProcessCohr 		 (long);
int  	DeleteSohr 			 (long);
int     CheckPcwo           (long);
int		PRError 			 (int);

#include	<ItemLevy.h>

/*
 * Main Processing Routine 
 */
int
main (
 int  argc, 
 char *argv [])
{
	long	hhcoHash	=	0L;

	StartProgram ();
	
	while (scanf ("%s", createFlag) != EOF && scanf ("%ld", &hhcoHash) != EOF)
	{
		cc = ProcessCohr (hhcoHash);
		if (cc)
			return (cc);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Start up program, Open database files, read default accounts, open audit. 
 */
void
StartProgram (void)
{
	char	*sptr;

	sprintf (envVarAutoSkUp, "%-1.1s", get_env ("AUTO_SK_UP"));

	/*
     * Check and Get Order Date Type. 
     */
	sptr = chk_env ("SO_DOI");
	envVarSoDoi	=	 (sptr == (char *)0 || sptr [1] == 'S') ? TRUE : FALSE;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check if advertising Levy applies. 
	 */
	sptr = chk_env ("ADVERT_LEVY");
	envVarAdvertLevy = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Check if for Sales Order Runtime Delete. 
	 */
	sptr = chk_env ("SO_RT_DELETE");
	envVarSoRtDelete = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * open main database files. 
	 */
	OpenDB ();

	/*
	 * Check for Combined P/S Invoice. 
	 */
	sptr = chk_env ("COMB_INV_PAC");
	if (sptr == (char *)0)
		combinedInvoicePacking	= FALSE;
	else
	{
		if (sptr [0] == 'Y' || sptr [0] == 'y')
			combinedInvoicePacking	= TRUE;
		else
			combinedInvoicePacking	= FALSE;
	}
	despatchDate = TodaysDate ();
}
/*
 * Program exit sequence	
 */
void
shutdown_prog (void)
{
	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files . 
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	abc_alias (soln2, soln);

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (soln2, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (pcwo, pcwo_list, PCWO_NO_FIELDS, "pcwo_hhsl_hash");
	if (envVarAdvertLevy)
		open_rec (inal, inal_list, INAL_NO_FIELDS, "inal_id_no");
}

/*
 * Close Database Files 
 */
void
CloseDB (void)
{
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (incc);
	abc_fclose (excf);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_fclose (soln2);
	abc_fclose (pcwo);
	if (envVarAdvertLevy)
		abc_fclose (inal);

	ArchiveClose ();

	abc_dbclose (data);
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.co_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}


int
ProcessCohr (
	long	hhcoHash)
{
	cohr_rec.hhco_hash	=	hhcoHash;
	cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
	if (cc)
		return (PRError (ERR_SOD_FND_COHR));

	cc = FindCustomerDetails ();
	if (cc)
		return (cc);

	return (Update ());
}
int
FindCustomerDetails (void)
{
	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_SOD_FND_CUMR));
	
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code, cohr_rec.area_code);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_SOD_FND_EXAF));

	return (EXIT_SUCCESS);
}

/*
 * Validate various fields at line level	
 */
void
CheckSalesman (void)
{
	char	oldSalesman [3];
	strcpy (oldSalesman, coln_rec.sman_code);
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, coln_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, cohr_rec.sale_code);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (exsf_rec.co_no, comm_rec.co_no);
			strcpy (exsf_rec.salesman_no, cumr_rec.sman_code);
			cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		}
		strcpy (coln_rec.sman_code, exsf_rec.salesman_no);
	}
}

/*
 * Update all files. 
 */
int
Update (
 void)
{
	int		nonStockItemOk = FALSE;

	double	value 			= 0.00,
			wkValue 		= 0.00,
			advertLevyAmt 	= 0.00,
			advertLevyPc  	= 0.00;

	char	dbt_day [3],
			dbt_mth [3];

	int		dmy [3],
			noLines = FALSE;

	auditReportDone = FALSE;

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		noTaxCharged = 1;
	else
		noTaxCharged = 0;

	if (envVarAdvertLevy)
		cohr_rec.other_cost_3 	= 0.00;

	cohr_rec.gross 		= 0.00;
	cohr_rec.item_levy 	= 0.00;
	cohr_rec.disc 		= 0.00;
	cohr_rec.tax 		= 0.00;
	cohr_rec.gst 		= 0.00;

	coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
	coln_rec.line_no 	= 0L;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	if (cc || coln_rec.hhco_hash != cohr_rec.hhco_hash)
	{
		cohr_rec.gross 		= 0.00;
		cohr_rec.item_levy 	= 0.00;
		cohr_rec.disc  		= 0.00;
		cohr_rec.tax   		= 0.00;
		cohr_rec.gst   		= 0.00;
		noLines 			= TRUE;
		abc_unlock (coln);
	}
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			abc_unlock (coln);
			cc = find_rec (coln, &coln_rec, NEXT, "u");
			continue;
		}

		if (envVarAdvertLevy)
		{
			ItemLevy
			 (
				inmr_rec.hhbr_hash,
				comm_rec.est_no,
				cumr_rec.curr_code,
				 (envVarSoDoi) ? TodaysDate () : comm_rec.dbt_date
			);
			advertLevyAmt = DPP (inal_rec.value * 100);
			advertLevyPc  = inal_rec.percent;
		}
		else
		{
			advertLevyAmt = 0.00;
			advertLevyPc  = 0.00;
		}
		/*
		 * Find Original soln record	
		 */
		soln_rec.hhsl_hash	=	coln_rec.hhsl_hash;
		cc = find_rec (soln, &soln_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock (coln);
			cc = find_rec (coln, &coln_rec, NEXT, "u");
			continue;
		}

		sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
		cc = find_rec (sohr, &sohr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, sohr, "DBUPDATE");

		if (SERIAL_ITEM && !strcmp (coln_rec.serial_no, "                         "))
		{
			abc_unlock (coln);
			return (EXIT_FAILURE);
		}

		CheckSalesman ();

		/*
		 * Non Stock Item, But Item it belongs to has been deleted.	
		 */
		if (NON_STOCK && !nonStockItemOk)
		{
			if (!envVarSoRtDelete)
			{
				soln_rec.qty_order	=	0.00;
				soln_rec.qty_bord	=	0.00;
				strcpy (soln_rec.status,    "D");
				strcpy (soln_rec.stat_flag, "D");
				cc = abc_update (soln, &soln_rec);
				if (cc)
					file_err (cc, soln, "DBUPDATE");
			}
			else
			{
				cc = ArchiveSoln (soln_rec.hhsl_hash);
				if (cc)
					file_err (cc, soln, "ARCHIVE");

				cc = abc_delete (soln);
				if (cc)
					file_err (cc, soln, "DBDELETE");
			}

			abc_unlock (coln);

			/*
			 * Check if header needs to be deleted/updated. 
			 */
			DeleteSohr (soln_rec.hhso_hash);

			cc = find_rec (coln, &coln_rec, NEXT, "u");
			continue;
		}
		else
		{
			/*
			 * Item is non stock and parent placed on B/O. 
			 */
			if (NON_STOCK)
			{
				strcpy (soln_rec.stat_flag, "B");
				strcpy (soln_rec.status, "B");
				cc = abc_update (soln, &soln_rec);
				if (cc)
					file_err (cc, soln, "DBUPDATE");
				
				abc_unlock (soln);
			}
			nonStockItemOk = TRUE;
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, excf, "DBFIND");

		soln_rec.n_xrate = excf_rec.ex_rate;
		coln_rec.o_xrate = soln_rec.o_xrate;
		coln_rec.n_xrate = soln_rec.n_xrate;
		strcpy (coln_rec.stat_flag, createFlag);
		strcpy (coln_rec.status, "I");

		/*-------------------------------------------------------
        | Check if coln_qty_des (XML dispatch qty )             |
        | is equal to zero                                      |
        | set q_order and q_backorder to zero for the packing   |
        | slip to be cancelled. If SO has an equivalent WO      |
        | do not zero out values                                |
        -------------------------------------------------------*/

/*
        if (coln_rec.q_des == 0.00 && CheckPcwo (coln_rec.hhsl_hash))
        {
            coln_rec.q_order = 0.00;
            coln_rec.q_backorder = 0.00;
        }
*/
	

		if (noTaxCharged)
		{
			coln_rec.tax_pc = 0.00;
			coln_rec.gst_pc = 0.00;
		}

		/*
		 * Perform Invoice Calculations	
		 */
		totalLine = coln_rec.q_order * 
					out_cost (coln_rec.sale_price, inmr_rec.outer_size);

		totalLine = no_dec (totalLine);

		totalTax = 	coln_rec.q_order * 
			  		out_cost (inmr_rec.tax_amount, inmr_rec.outer_size);

		totalTax = no_dec (totalTax);

		lineDiscount = (double) (coln_rec.disc_pc);
		lineDiscount = DOLLARS (lineDiscount);
		lineDiscount *= totalLine;
		lineDiscount = no_dec (lineDiscount);

		if (envVarAdvertLevy)
		{
			lineLevyPc 	= (double) advertLevyPc;
			lineLevyPc 	= DOLLARS (lineLevyPc);
			lineLevyPc 	*= totalLine;
			lineLevyPc 	= no_dec (lineLevyPc);

			lineLevyAmt = advertLevyAmt;
			lineLevyAmt *= (double) coln_rec.q_order;
			lineLevyAmt = no_dec (lineLevyAmt);
		}
		lineLevy	=	lineLevyAmt + lineLevyPc;
		
		if (noTaxCharged)
			lineTax = 0.00;
		else
		{
			lineTax = (double) inmr_rec.tax_pc;
			if (cumr_rec.tax_code [0] == 'D')
				lineTax *= totalTax;
			else
			{
				if (envVarDbNettUsed)
					lineTax *= ( (totalLine - lineDiscount) + lineLevy);
				else
					lineTax *= totalLine + lineLevy;
			}
		
			lineTax = DOLLARS (lineTax);
			lineTax = no_dec (lineTax);
		}
		if (noTaxCharged)
			lineGst = 0.00;
		else
		{
			lineGst = (double) inmr_rec.gst_pc;
			if (envVarDbNettUsed)
				lineGst *= ( (totalLine - lineDiscount) + lineTax + lineLevy);
			else
				lineGst *= (totalLine + lineTax + lineLevy);

			lineGst = DOLLARS (lineGst);
		}

		coln_rec.gross    	= totalLine;
		coln_rec.item_levy 	= lineLevy;
		coln_rec.amt_disc 	= lineDiscount;
		coln_rec.amt_tax  	= lineTax;
		coln_rec.amt_gst  	= lineGst;

		cohr_rec.gross 		+= totalLine;
		cohr_rec.item_levy 	+= lineLevy;
		cohr_rec.disc  		+= lineDiscount;
		cohr_rec.tax   		+= lineTax;
		cohr_rec.gst   		+= lineGst;

		cc = abc_update (coln, &coln_rec);
		if (cc)
			file_err (cc, coln, "DBUPDATE");

		if (!NON_STOCK)
		{
			/*
			 * backorder qty = 0 so delete soln	
			 */
			if (coln_rec.q_backorder == 0.00)
			{
				if (!envVarSoRtDelete)
				{
					soln_rec.qty_order	=	0.00;
					soln_rec.qty_bord	=	0.00;
					strcpy (soln_rec.status,    "D");
					strcpy (soln_rec.stat_flag, "D");
					cc = abc_update (soln, &soln_rec);
					if (cc)
						file_err (cc, soln, "DBUPDATE");
				}
				else
				{
					cc = ArchiveSoln (soln_rec.hhsl_hash);
					if (cc)
						file_err (cc, soln, "ARCHIVE");

					cc = abc_delete (soln);
					if (cc)
						file_err (cc, soln, "DBDELETE");
				}
				nonStockItemOk = FALSE;
			}
			else
			{
				strcpy (soln_rec.status, "B");
				strcpy (soln_rec.stat_flag, "B");
				soln_rec.qty_order 	= coln_rec.q_backorder;
				soln_rec.qty_bord 	= 0.00;
				cc = abc_update (soln, &soln_rec);
				if (cc)
					file_err (cc, soln, "DBUPDATE");
				
				nonStockItemOk = TRUE;
			}
		}
		/*
		 * Check if header needs to be deleted/updated. 
		 */
		DeleteSohr (soln_rec.hhso_hash);

		abc_unlock (coln);
		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	wkValue = (noTaxCharged) ? 0.00 : (double) (comm_rec.gst_rate / 100.00);

	value = cohr_rec.freight + 
			cohr_rec.insurance -
			cohr_rec.ex_disc + 
			cohr_rec.other_cost_1 +
			cohr_rec.other_cost_2 + 
			cohr_rec.other_cost_3 +
			cohr_rec.sos;

	wkValue *= value;
	wkValue = no_dec (wkValue);

	cohr_rec.gst += wkValue;
	cohr_rec.gst = no_dec (cohr_rec.gst);
	
	/*
	 * Set the invoice batch no	
	 */
	DateToDMY (comm_rec.dbt_date, &dmy [0], &dmy [1], &dmy [2]);
	sprintf (dbt_day, "%02d", dmy [0]);
	sprintf (dbt_mth, "%02d", dmy [1]);

	sprintf (cohr_rec.batch_no, "A%s%s", dbt_day, dbt_mth);

	/*
	 * Packing slip/invoice combined so invoice date must 
	 * be what was on printed Doco.                       
	 */
	cohr_rec.date_raised 	= (envVarSoDoi) ? TodaysDate (): comm_rec.dbt_date;
	cohr_rec.date_required 	= despatchDate;
	strcpy (cohr_rec.type, "I");

	strcpy (cohr_rec.tax_code, sohr_rec.tax_code);
	strcpy (cohr_rec.tax_no, cumr_rec.tax_no);

	strcpy (cohr_rec.stat_flag, createFlag);
	strcpy (cohr_rec.inv_print, (combinedInvoicePacking) ? "Y" : "N");

	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, cohr, "DBUPDATE");

	add_hash
	 (
		cohr_rec.co_no,
		cohr_rec.br_no,
		"RO",
		0,
		cohr_rec.hhcu_hash,
		0L,
		0L,
		 (double) 0.0
	);
		
	if (AUTO_SK_UP && !noLines)
	{
		add_hash
		 (
			cohr_rec.co_no,
			cohr_rec.br_no,
			"SU",
			0,
			cohr_rec.hhco_hash,
			0L,
			0L,
			 (double) 0.0
		);
	}
	return (EXIT_SUCCESS);
}

/*
 * Delete or update sohr record. 
 */
int
DeleteSohr (
	long	hhsoHash)
{
	int		linesFound	=	0;

	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (sohr);
		return (EXIT_SUCCESS);
	}
	/*
	 * Check if sohr needs to be deleted. ie no soln records remaining for sohr
	 */
	if (!envVarSoRtDelete)
	{
		linesFound = 0;

		soln2_rec.hhso_hash = hhsoHash;
		soln2_rec.line_no 	= 0;
		cc = find_rec (soln2, &soln2_rec, GTEQ, "r");
		while (!cc && soln2_rec.hhso_hash == hhsoHash)
		{
			if (soln2_rec.status [0] != 'D')
				linesFound++;

			cc = find_rec (soln2, &soln2_rec, NEXT, "r");
		}
		if (!linesFound)
		{
			strcpy (sohr_rec.stat_flag,	"D");
			strcpy (sohr_rec.status, 	"D");
			strcpy (sohr_rec.sohr_new,	"N");
			cc = abc_update (sohr, &sohr_rec);
			if (cc)
				return (PRError (ERR_SOD_DEL_HEAD));

			return (EXIT_SUCCESS);
		}
	}
	else
	{
		soln2_rec.hhso_hash = hhsoHash;
		soln2_rec.line_no = 0;
		cc = find_rec (soln2,&soln2_rec,GTEQ,"r");
		if (cc || soln2_rec.hhso_hash != hhsoHash)
		{
			cc = ArchiveSohr (sohr_rec.hhso_hash);
			if (cc)
				file_err (cc, sohr, "ARCHIVE");
			
			cc = abc_delete (sohr);
			if (cc) 
				return (PRError (ERR_SOD_DEL_HEAD));

			return (EXIT_SUCCESS);
		}
	}
	/*
	 * Check if status and invoice number/packing slip number 
	 * is the same as order may be on multiple packing slips.
	 */
	if (!strcmp (sohr_rec.inv_no,cohr_rec.inv_no) && sohr_rec.status [0] == 'P')
	{
		strcpy (sohr_rec.inv_no, "        ");
		strcpy (sohr_rec.stat_flag, "B");
		strcpy (sohr_rec.status, "B");
	}
	strcpy (sohr_rec.sohr_new, "N");
	cc = abc_update (sohr, &sohr_rec);
	if (cc)
		return (PRError (ERR_SOD_UPD_HEAD));

	abc_unlock (sohr);
	return (EXIT_SUCCESS);
}

/*
 * Process Errors, did not forget ML calls, just that these messages are 
 * used for debug use and it's a bit hard when converted to non english 
 */
int
PRError (
	int	errCode)
{
	if (ERR_SOD_DEL_HEAD == errCode)
	{
		fprintf (stderr, "Failed to delete sales order header\n");
		fprintf (stderr, "cohr_hhco_hash = [%ld]\n", cohr_rec.hhco_hash);
		return (errCode);
	}
	if (ERR_SOD_UPD_HEAD == errCode)
	{
		fprintf (stderr, "Failed to update sales order header\n");
		fprintf (stderr, "cohr_hhco_hash = [%ld]\n", cohr_rec.hhco_hash);
		return (errCode);
	}
	if (ERR_SOD_FND_CUMR == errCode)
	{
		fprintf (stderr, "Failed to find customer master (cumr)\n");
		fprintf (stderr, "cohr_hhcu_hash = [%ld]\n", cohr_rec.hhcu_hash);
		return (errCode);
	}
	if (ERR_SOD_FND_EXAF == errCode)
	{
		fprintf (stderr, "Failed to find area master (exaf)\n");
		fprintf (stderr, "cohr_area_code = [%s]\n", cohr_rec.area_code);
		return (errCode);
	}
	if (ERR_SOD_FND_COHR == errCode)
	{
		fprintf (stderr, "Failed to find Order Header (cohr)\n");
		fprintf (stderr, "cohr_hhco_hash = [%ld]\n", cohr_rec.hhco_hash);
		return (errCode);
	}
	return (errCode);
}

int CheckPcwo (
    long hhslHash)
{
    pcwo_rec.hhsl_hash = hhslHash;
    cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
    return cc;
}
