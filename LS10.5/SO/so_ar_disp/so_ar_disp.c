/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_ar_disp.c,v 5.0 2002/05/07 10:18:45 scott Exp $
|  Program Name  : (so_ar_disp.c)
|  Program Desc  : (Archive Sales Order Stock Display)
|---------------------------------------------------------------------|
|  Date Written  : (30/04/2002)    | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: so_ar_disp.c,v $
| Revision 5.0  2002/05/07 10:18:45  scott
| Updated to bring version number to 5.0
|
| Revision 1.2  2002/05/02 06:39:31  scott
| Updated to change layout
|
| Revision 1.1  2002/05/02 03:37:43  scott
| New Program
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_ar_disp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_ar_disp/so_ar_disp.c,v 5.0 2002/05/07 10:18:45 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>


#define	C_LINE		(inmr_rec.inmr_class [0] == 'Z')
#define	BONUS_ITEM	(arsoln_rec.bonus_flag [0] == 'Y')

	int		envVarDbCo 			= FALSE, 
			linePrinted			= 0, 
			envVarRepTax 		= FALSE, 
			envVarDbFind 		= FALSE, 
			envVarDbMcurr		= FALSE, 
			envVarDbNettUsed 	= TRUE, 
			envVarCnNettUsed 	= TRUE;

		extern	int		lp_x_off,
						lp_y_off;

	char	branchNumber [3], 
			displayLine [300], 
			dateOrdered [11], 
			dateInvoiced [11], 
			dateMfg [11], 
			envVarCurrCode [4], 
			mlCusDisp [6] [41];

	char 	*data = "data";

	float	ordQty 		= 0.0, 
			invQty 		= 0.0, 
			totOrdQty	= 0.0, 
			totInvQty	= 0.0, 
			StdCnvFct 	= 0.00, 
			CnvFct		= 0.00;
	
	double	ordAmt 		= 0.0, 
			invAmt 		= 0.0, 
			totOrdAmt 	= 0.0, 
			totInvAmt 	= 0.0, 
			invBalance 	= 0.0,
			ordBalance 	= 0.0;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct arsohrRecord	arsohr_rec;
struct arsolnRecord	arsoln_rec;
struct arpcwoRecord	arpcwo_rec;
struct arhrRecord	arhr_rec;
struct arlnRecord	arln_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct soktRecord	sokt_rec;
struct pcwoRecord	pcwo_rec;

	/*
	 * Structure to hold archive or live information if it exists.
	 */
	struct {
		char	invoiceNo	 [9];
		char	worksOrderNo [8];
		char	taxCode		 [2];
		Date	invoiceDate;	
		Date	woMfgDate;	
		Date	woCreateDate;	
		float	woProdQty;
		float	woMfgQty;
		float	quantity;
		float	taxPc;
		float	gstPc;
		double	salePrice;
		double	itemLevy;
		float	discPc;
	} storeRec;
/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	value [17];
	char	orderNo [9];
	char	customerName [41];
	char	previousValue [17];
} local_rec;
extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.previousValue, "Customer No      ", " ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.value}, 
	{1, LIN, "customerName", 	 4, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Customer Name    ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.customerName}, 
	{1, LIN, "orderNo", 	 5, 2, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"0", "0", "Sales Order No   ", " Default : All Current Orders ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.orderNo}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <FindCumr.h>

/*
 * Function Declarations 
 */
int  	ProcessArsohr 		(void);
int  	ProcessArsoln 		(int);
int  	heading 			(int);
int  	spec_valid 			(int);
void	FindData			(long);
void 	CalcBalances 		(int, int);
void 	CloseDB 			(void);
void 	DisplayCustomer		(void);
void 	DisplayInfo 		(void);
void 	InitML 				(void);
void 	OpenDB 				(void);
void 	PrintTotals 		(int);
void 	SrchSohr 			(char *);
void 	shutdown_prog 		(void);
void 	CheckEnvironment	(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc, 
 char	*argv [])
{
	CheckEnvironment ();
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	InitML ();

	strcpy (local_rec.previousValue, "000000");

	while (prog_exit == 0)
	{
		linePrinted 	= 0;
		search_ok 		= TRUE;
		entry_exit 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		init_ok 		= TRUE;
		init_vars (1);	
		totOrdQty 	= 0.00;
		totInvQty 	= 0.00;
		totOrdAmt 	= 0.00;
		totInvAmt 	= 0.00;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		DisplayInfo ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Multi-lingual initialisation. 
 */
void
InitML (void)
{
	strcpy (mlCusDisp [1], ML ("TOTAL"));
	strcpy (mlCusDisp [2], ML ("CUSTOMER TOTAL"));
	strcpy (mlCusDisp [3], ML ("ORDER TOTAL"));
	strcpy (mlCusDisp [4], ML ("SERIAL NUMBER"));
	strcpy (mlCusDisp [5], ML ("BONUS ITEM  "));
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
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) 
						? "cumr_id_no" : "cumr_id_no3");

	open_rec (inmr, 	inmr_list, 		INMR_NO_FIELDS, 	"inmr_hhbr_hash");
	open_rec (arsohr, 	arsohr_list, 	ARSOHR_NO_FIELDS, 	"arsohr_hhcu_hash");
	open_rec (arsoln, 	arsoln_list, 	ARSOLN_NO_FIELDS, 	"arsoln_id_no");
	open_rec (arpcwo, 	arpcwo_list, 	ARPCWO_NO_FIELDS, 	"arpcwo_hhsl_hash");
	open_rec (ccmr, 	ccmr_list, 		CCMR_NO_FIELDS, 	"ccmr_hhcc_hash");
	open_rec (arhr, 	arhr_list, 		ARHR_NO_FIELDS, 	"arhr_hhco_hash");
	open_rec (arln,	 	arln_list, 		ARLN_NO_FIELDS, 	"arln_hhsl_hash");
	open_rec (cohr, 	cohr_list, 		COHR_NO_FIELDS, 	"cohr_hhco_hash");
	open_rec (coln, 	coln_list, 		COLN_NO_FIELDS, 	"coln_hhsl_hash");
	open_rec (inum, 	inum_list, 		INUM_NO_FIELDS, 	"inum_hhum_hash");
	open_rec (pcwo, 	pcwo_list, 		PCWO_NO_FIELDS, 	"pcwo_hhsl_hash");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (arsohr);
	abc_fclose (arsoln);
	abc_fclose (arpcwo);
	abc_fclose (arhr);
	abc_fclose (arln);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inum);
	abc_fclose (pcwo);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	order_found = FALSE;

	/*
	 * Validate Customer Number. 
	 */
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.value, 6));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.customerName, cumr_rec.dbt_name);
		DSP_FLD ("customerName");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Purchase Order Number. 
	 */
	if (LCHECK ("orderNo"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}

		order_found = FALSE;

		/*
		 * Check if order is on file. 
		 */
		arsohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (arsohr, &arsohr_rec, GTEQ, "r");
		while (!cc && arsohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			if (!strcmp (arsohr_rec.order_no, local_rec.orderNo))
			{
				order_found = TRUE;
				break;
			}
			cc = find_rec (arsohr, &arsohr_rec, NEXT, "r");
		}
		if (!order_found)
		{
			errmess (ML (mlStdMess102));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
DisplayInfo (void)
{
	DisplayCustomer ();
	strcpy (local_rec.previousValue, cumr_rec.dbt_no);
}

void
DisplayCustomer (void)
{
	heading (1);

	sprintf 
	(
		err_str, 
		"Customer %s %s / Currency %s / Country = %s", 
		cumr_rec.dbt_no, 
		cumr_rec.dbt_name, 
		cumr_rec.curr_code, 
		cumr_rec.ctry_code
	);
	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open 
	(
		0, 
		2, 
		12, 
		err_str,
		comm_rec.co_no, 
		comm_rec.co_name, 
		comm_rec.est_no, 
		comm_rec.est_name, 
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec ("  ORDER   |   ORDER  | INVOICE  | INVOICE  |  WORKS  |   WORKS  |       ITEM       |BR|WH|UOM | QUANTITY | QUANTITY |    VALUE    ");
	sprintf (err_str, "  NUMBER  |   DATE   |  NUMBER  |   DATE   |  ORDER  |ORDER DATE|      NUMBER      |NO|NO|    |  ORDER   | SUPPLIED |    (%3.3s)    ",cumr_rec.curr_code);

	Dsp_saverec (err_str);

	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");

	if (strcmp (local_rec.orderNo, "00000000"))
	{
		ProcessArsohr ();
		linePrinted = 0;
	}
	else
	{
		arsohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (arsohr, &arsohr_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == arsohr_rec.hhcu_hash)
		{
			ProcessArsohr ();
			cc = find_rec (arsohr, &arsohr_rec, NEXT, "r");
		}
	}
	if (linePrinted)
		PrintTotals (TRUE);

	Dsp_srch ();

	Dsp_close ();
}
void
CalcBalances (
	int		BOM, 
	int		in_local_curr)
{
	double	o_total	=	0.00, 
			o_disc	=	0.00, 
			o_tax	=	0.00, 
			o_gst	=	0.00,
			i_total	=	0.00, 
			i_disc	=	0.00, 
			i_tax	=	0.00, 
			i_gst	=	0.00;

	float	invLineQty	=	0.00,	
			ordLineQty	=	0.00;

	invBalance	=	0.00;
	ordBalance	=	0.00;

	strcpy (dateOrdered, DateToString (arsohr_rec.dt_raised));
	if (storeRec.invoiceDate)
		strcpy (dateInvoiced, DateToString (storeRec.invoiceDate));
	else
		strcpy (dateInvoiced, " ");

	if (storeRec.woMfgDate)
		strcpy (dateMfg, DateToString (storeRec.woMfgDate));
	else
		strcpy (dateMfg, " ");

	if (arsoln_rec.bonus_flag [0] != 'Y')
	{
		ordLineQty 	= arsoln_rec.qty_org_ord;
		invLineQty	= storeRec.quantity;
		if (BOM)
		{
			ordLineQty 	*= sokt_rec.matl_qty;
			invLineQty *= sokt_rec.matl_qty;
		}
		/*
		 * Calculate order and invoice values, may need both.
		 */
			
		o_total	=	(double) ordLineQty;
		i_total	=	(double) invLineQty;

		o_total	*=	out_cost (arsoln_rec.sale_price, inmr_rec.outer_size);
		o_total	=	no_dec (o_total);

		i_total	*=	out_cost (storeRec.salePrice, inmr_rec.outer_size);
		i_total	=	no_dec (i_total);

		o_disc	=	(double) arsoln_rec.dis_pc;
		o_disc	*=	o_total;
		o_disc	=	DOLLARS (o_disc);
		o_disc	=	no_dec (o_disc);

		i_disc	=	(double) storeRec.discPc;
		i_disc	*=	i_total;
		i_disc	=	DOLLARS (i_disc);
		i_disc	=	no_dec (i_disc);

		if (envVarRepTax)
		{
			o_tax	=	(double) arsoln_rec.tax_pc;
			if (arsohr_rec.tax_code [0] == 'D')
				o_tax *= o_total;
			else
			{
				if (envVarDbNettUsed)
					o_tax	*=	(o_total + arsoln_rec.item_levy + o_disc);
				else
					o_tax	*=	(o_total + arsoln_rec.item_levy);
			}
			o_tax	=	DOLLARS (o_tax);
			o_tax	=	(double) arsoln_rec.tax_pc;

			if (storeRec.taxCode [0] == 'D')
				i_tax *= i_total;
			else
			{
				if (envVarDbNettUsed)
					i_tax	*=	(i_total + storeRec.itemLevy + i_disc);
				else
					i_tax	*=	(i_total + storeRec.itemLevy);
			}
			i_tax	=	DOLLARS (i_tax);
		}
		o_tax	=	no_dec (o_tax);
		i_tax	=	no_dec (i_tax);

		o_gst	=	(double) arsoln_rec.gst_pc;
		if (envVarDbNettUsed)
			o_gst	*=	(o_total - o_disc) + o_tax + arsoln_rec.item_levy;
		else
			o_gst	*=	(o_total + o_tax + arsoln_rec.item_levy);

		o_gst	=	DOLLARS (o_gst);
			
		if (envVarDbNettUsed)
			ordBalance	=	o_total - o_disc + o_tax + o_gst + arsoln_rec.item_levy;
		else
			ordBalance	=	o_total + o_tax + o_gst + arsoln_rec.item_levy;

		i_gst	=	(double) storeRec.gstPc;
		if (envVarDbNettUsed)
			i_gst	*=	(i_total - i_disc) + i_tax + storeRec.itemLevy;
		else
			i_gst	*=	(i_total + i_tax + storeRec.itemLevy);

		i_gst	=	DOLLARS (i_gst);
			
		if (envVarDbNettUsed)
			invBalance	=	i_total - i_disc + i_tax + i_gst + storeRec.itemLevy;
		else
			invBalance	=	i_total + i_tax + i_gst + storeRec.itemLevy;
	
		if (in_local_curr == TRUE)
		{
			/*
			 * Convert arsoln customers balance to local_currency 
			 */
			strcpy (pocr_rec.co_no, 	comm_rec.co_no);
			strcpy (pocr_rec.code, 	cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");
	
			if (pocr_rec.ex1_factor != 0.00)
			{
				invBalance = invBalance / pocr_rec.ex1_factor;
				ordBalance = ordBalance / pocr_rec.ex1_factor;
			}
		}
		totInvAmt += invBalance;
		totOrdAmt += ordBalance;
	}
	totOrdQty	+= ordLineQty;
	totInvQty 	+= invLineQty;
	ordQty 		+= ordLineQty;
	invQty 		+= invLineQty;
}

void
PrintTotals (
	int		keyTotal)
{
	if (keyTotal)
	{
		/*
		 * Customer Totals	
		 */
		sprintf 
		(
			displayLine,
			"          ^E          ^E          ^E          ^E         ^E          ^E              ^1%14.14s^6 ^E%9.2f ^E%9.2f ^E%12.2f",
			mlCusDisp [2], 
			totOrdQty, 
			totInvQty, 
			(totInvAmt > 0.00) ? DOLLARS (totInvAmt) : DOLLARS (totOrdAmt)
		 );
		Dsp_saverec (displayLine);
	}
	else
	{
		/*
		 * Order Totals	
		 */
		sprintf 
		(
			displayLine,
			"          ^E          ^E          ^E          ^E         ^E          ^E              ^1%14.14s^6 ^E%9.2f ^E%9.2f ^E%12.2f",
			mlCusDisp [3], 
			ordQty, 
			invQty, 
			(invAmt > 0.00) ? DOLLARS (invAmt) : DOLLARS (ordAmt)
		);
		Dsp_saverec (displayLine);


		Dsp_saverec ("^^GGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGGGHGGHGGHGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGG");
	}
}

int
ProcessArsohr (void)
{
	int		firstLine = TRUE;

	if (arsohr_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (EXIT_SUCCESS);

	ordQty 		= 0.00;
	invQty 		= 0.00;
	ordAmt 		= 0.00;
	invAmt 		= 0.00;

	/*
	 * Find Order lines.
	 */
	arsoln_rec.hhso_hash 	= arsohr_rec.hhso_hash;
	arsoln_rec.line_no 		= 0;
	cc = find_rec (arsoln, &arsoln_rec, GTEQ, "r");
	while (!cc && arsoln_rec.hhso_hash == arsohr_rec.hhso_hash)
	{
		/*
		 * Find Item
		 */
		inmr_rec.hhbr_hash	=	arsoln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (ProcessArsoln (firstLine))
				return (EXIT_FAILURE);

			firstLine 	= FALSE;
			linePrinted = 1;
		}
		cc = find_rec (arsoln, &arsoln_rec, NEXT, "r");
	}
	if (!firstLine)
		PrintTotals (FALSE);
	return (EXIT_SUCCESS);
}

int
ProcessArsoln (
	int 	firstLine)
{
	char	invoiceValue [13];
	float	dspOrdQty	=	0.00,
			dspInvQty	=	0.00;

	/*
	 * Find Invoice Details, could be from archive or live file.
	 */
	FindData (arsoln_rec.hhsl_hash);

    inmr_rec.hhbr_hash = arsoln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
        strcpy (inmr_rec.item_no, "Unknown Item No.");

	inum_rec.hhum_hash = arsoln_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;

	if (inum_rec.hhum_hash != arsoln_rec.hhum_hash)
	{
		inum_rec.hhum_hash = arsoln_rec.hhum_hash;  
		cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
	}
	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}

	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
	if (arsoln_rec.hhcc_hash != ccmr_rec.hhcc_hash)
	{
		ccmr_rec.hhcc_hash	=	arsoln_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");
	}
	CalcBalances (FALSE, FALSE);

	CnvFct	= inum_rec.cnv_fct / StdCnvFct;
	dspOrdQty	= (arsoln_rec.qty_org_ord) / CnvFct;
	dspInvQty	= (storeRec.quantity) / CnvFct;

	if (BONUS_ITEM)
		strcpy (invoiceValue, mlCusDisp [5]);
	else
	{
		ordAmt 	+= ordBalance;
		invAmt 	+= invBalance;
		sprintf (invoiceValue, "%12.2f", (invBalance > 0) ? DOLLARS (invBalance) : DOLLARS (ordBalance));
	}

	sprintf 
	(
		displayLine,
		" %8.8s ^E%10.10s^E %8.8s ^E%10.10s^E %7.7s ^E%10.10s^E %-16.16s ^E%2.2s^E%2.2s^E%4.4s^E%9.2f ^E%9.2f ^E%12.12s ",
		(!firstLine) ? " " : arsohr_rec.order_no, 
		(!firstLine) ? " " : dateOrdered, 
		storeRec.invoiceNo,
		dateInvoiced,
		storeRec.worksOrderNo,
		dateMfg,
		inmr_rec.item_no, 
		ccmr_rec.est_no, 
		ccmr_rec.cc_no, 
		inum_rec.uom, 
		dspOrdQty, 
		dspInvQty, 
		invoiceValue
	);

	cc = Dsp_saverec (displayLine);
	if (cc)
		return (cc);

	if (strcmp (arsoln_rec.serial_no, "                         "))
	{
		sprintf (displayLine, 
				 "         ^E          ^E       ^E  ^E  ^E          ^E     ^E            ^E             ^E %-14.14s: %-25.25s  ^E", 
				 mlCusDisp [4], 
				 arsoln_rec.serial_no);
		cc = Dsp_saverec (displayLine);
		if (cc)
			return (cc);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for order number. 
 */
void
SrchSohr (
	char	*keyValue)
{
	_work_open (8, 0, 40);
	strcpy (arsohr_rec.co_no, comm_rec.co_no);
	strcpy (arsohr_rec.br_no, comm_rec.est_no);
	sprintf (arsohr_rec.order_no, "%-8.8s", keyValue);
	arsohr_rec.hhcu_hash = cumr_rec.hhcu_hash; 

	save_rec ("#Order No", "#Customer Order No.   ");
	cc = find_rec (arsohr, &arsohr_rec, GTEQ, "r");
	while (!cc && arsohr_rec.hhcu_hash == cumr_rec.hhcu_hash &&
				!strcmp (arsohr_rec.co_no, comm_rec.co_no))
	{
		if (!strncmp (arsohr_rec.order_no, keyValue, strlen (keyValue)))
		{
			cc = save_rec (arsohr_rec.order_no, arsohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (arsohr, &arsohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (arsohr_rec.co_no, comm_rec.co_no);
	strcpy (arsohr_rec.br_no, comm_rec.est_no);
	sprintf (arsohr_rec.order_no, "%-8.8s", temp_str);
	arsohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (arsohr, &arsohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, arsohr, "DBFIND");
}

/*
 * Find Invoice from Invoice Archive file OR Invoice Live File
 * Find Works Order from Works Order Archive file OR Works Order Live File
 */
void
FindData (
	long	hhslHash)
{
	memset (&storeRec, 0, sizeof (storeRec));

	/*
	 * Find arln record for current hhslHash. 
	 */
	arln_rec.hhsl_hash	=	hhslHash;
 	cc = find_rec (arln, &arln_rec, COMPARISON, "r");
	if (!cc)
	{
		arhr_rec.hhco_hash	=	arln_rec.hhco_hash;
 		cc = find_rec (arhr, &arhr_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (storeRec.invoiceNo, arhr_rec.inv_no);
			storeRec.invoiceDate	=	arhr_rec.date_raised;
			storeRec.quantity		=	arln_rec.q_order;
			storeRec.salePrice	=	arln_rec.sale_price;
			storeRec.discPc		=	arln_rec.disc_pc;
			storeRec.taxPc		=	arln_rec.tax_pc;
			storeRec.gstPc		=	arln_rec.gst_pc;
			storeRec.itemLevy		=	arln_rec.item_levy;
		}
	}
	else
	{
		coln_rec.hhsl_hash	=	hhslHash;
		cc = find_rec (coln, &coln_rec, COMPARISON, "r");
		if (!cc)
		{
			cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
 			cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (storeRec.invoiceNo, cohr_rec.inv_no);
				storeRec.invoiceDate	=	cohr_rec.date_raised;
				storeRec.quantity		=	coln_rec.q_order;
				storeRec.salePrice	=	coln_rec.sale_price;
				storeRec.discPc		=	coln_rec.disc_pc;
				storeRec.taxPc		=	coln_rec.tax_pc;
				storeRec.gstPc		=	coln_rec.gst_pc;
				storeRec.itemLevy		=	coln_rec.item_levy;
			}
		}
	}
	pcwo_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (pcwo, &pcwo_rec, COMPARISON, "r");
	if (cc)
	{
		arpcwo_rec.hhsl_hash	=	hhslHash;
		cc = find_rec (arpcwo, &arpcwo_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (storeRec.worksOrderNo, arpcwo_rec.order_no);
			storeRec.woCreateDate	=	arpcwo_rec.create_date;
			storeRec.woMfgDate		=	(arpcwo_rec.mfg_date) 
								?  arpcwo_rec.mfg_date : arpcwo_rec.reqd_date;
			storeRec.woProdQty		=	arpcwo_rec.prod_qty;
			storeRec.woMfgQty		=	arpcwo_rec.act_prod_q;
		}
	}
	else
	{
			strcpy (storeRec.worksOrderNo, pcwo_rec.order_no);
			storeRec.woCreateDate	=	pcwo_rec.create_date;
			storeRec.woMfgDate		=	(pcwo_rec.mfg_date) 
								?  pcwo_rec.mfg_date : pcwo_rec.reqd_date;
			storeRec.woProdQty		=	pcwo_rec.prod_qty;
			storeRec.woMfgQty		=	pcwo_rec.act_prod_qty;
	}
}

void 
CheckEnvironment (void)
{
	char	*sptr;

	/*
	 * Check for multi-currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (envVarDbMcurr)
	{
		/*
		 * Get native currency. 
		 */
		sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));
	}
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *)0) ? envVarDbNettUsed : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envVarDbCo	= (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_FIND");
	envVarDbFind	= (sptr == (char *)0) ? FALSE : atoi (sptr);
}
int
heading (
	int	scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	swide ();
	clear ();

	/*
	 * Sales Order Display by Customer. 	  
	 */
	strcpy (err_str, ML ("Archive Sales Order Display")); 

	rv_pr (err_str, (130 - strlen (err_str)) / 2, 0, 1);

	line_at (1,0,132);

	box (0, 3, 132, 2);

	line_at (21,0,132);
	sprintf (err_str, " %s - %s", cumr_rec.dbt_no, cumr_rec.dbt_name);
	us_pr (err_str, (130 - strlen (err_str)) / 2, 1, 1);

	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, "%s", err_str);

	sprintf (err_str, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 48, "%s", err_str);
	line_at (23, 0, 132);

	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}
