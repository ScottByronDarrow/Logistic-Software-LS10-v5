/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_so_rel.c,v 5.5 2001/11/07 06:07:38 scott Exp $
|  Program Name  : (po_so_rel.c)           
|  Program Desc  : (Release sales orders from stock receipt.  ) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/09/87         |
|---------------------------------------------------------------------|
| $Log: po_so_rel.c,v $
| Revision 5.5  2001/11/07 06:07:38  scott
| Updated as popen not called if no sales orders but transfers.
|
| Revision 5.4  2001/11/07 05:23:50  scott
| Updated to fix ???? on lines with extra description.
|
| Revision 5.3  2001/11/06 08:18:02  scott
| Updated for lineup
|
| Revision 5.2  2001/08/09 09:16:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:14  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/02/15 09:40:57  scott
| Updated to add special running mode using PID for XML execution
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_so_rel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_so_rel/po_so_rel.c,v 5.5 2001/11/07 06:07:38 scott Exp $";

#include	<pslscr.h>
#include	<ml_po_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#define		MAX_HASH	1000
#define		BACKORDER		 	(soln_rec.status [0] == 'B')
#define		TR_BACKORDER	 	(itln_rec.status [0] == 'B')
#define		InternalMIN(X,Y)	 (X < Y) ? X : Y;

	/*---------------------------
	| Special fields and flags. |
	---------------------------*/
	FILE	*fout;

	int		printerNumber 		= 1,
			envVarConOrders 	= 0,
			printerPipeOpen 	= FALSE,
			processPID			= 0;

	char	systemDate [11],
			dropShipment [2],
			poghFindStatus [2],
			cohrSetStatus [2];

	long	lsystemDate;

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct intrRecord	intr_rec;
struct ccmrRecord	ccmr_rec;
struct poghRecord	pogh_rec;
struct poglRecord	pogl_rec;
struct posoRecord	poso_rec;
struct cumrRecord	cumr_rec;
struct sumrRecord	sumr_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;

	char	*inmr2 			= "inmr2",
			*coln2 			= "coln2",
			*data 			= "data",
			*fifteenSpaces	= "               ";


#include	<proc_sobg.h>

/*=======================
| Function Declarations |
=======================*/
int 	IntFindInmr 		(long);
void 	AddIntr 			(int);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	DeleteColn 			(long);
void 	IntFindSuper 		(char *);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintInex 			(void);
void 	Process 			(long, long, long, long, float);
void 	ProcessInvoices 	(long, long, float);
void 	ProcessPogh 		(void);
void 	ProcessPogl 		(long);
void 	ProcessSalesOrder 	(long, float);
void 	ProcessTransfers 	(long, float);
void 	UpdateIthr 			(long);
void 	UpdateSohr 			(long);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 5)	
	{

		print_at (0,0, "Usage : %s <LPNO> <purchase status> <posting flag> <direct delivery [Y)es/N)o]> [Optional PID]", argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------------------------------
	| check forward for order consolidation	|
	---------------------------------------*/
	sptr = chk_env ("CON_ORDERS");
	envVarConOrders = (sptr == (char *)0) ? 0 : atoi (sptr);

	printerNumber = atoi (argv [1]);
	sprintf (poghFindStatus, 	"%-1.1s", argv [2]);
	sprintf (cohrSetStatus, 	"%-1.1s", argv [3]);
	sprintf (dropShipment, 		"%-1.1s", argv [4]);

	if (argc == 6)
		processPID	=	atoi (argv [5]);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	init_scr ();

	OpenDB ();

	if (!processPID)
	{
		dsp_screen (" Processing Automatic Sales Order Release.",
				comm_rec.co_no, comm_rec.co_name);
	}

	ProcessPogh ();

	shutdown_prog ();	
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	if (printerPipeOpen)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
}


/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_itff_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (pogh, pogh_list, POGH_NO_FIELDS, (processPID) ? "pogh_pid_id"
															: "pogh_up_id");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (poso, poso_list, POSO_NO_FIELDS, "poso_hhpl_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");

	abc_alias (inmr2, inmr);
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");

	abc_alias (coln2, coln);
	open_rec (coln2, coln_list, COLN_NO_FIELDS, "coln_hhcl_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (coln);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (ccmr);
	abc_fclose (poso);
	abc_fclose (cumr);
	abc_fclose (inmr2);
	abc_fclose (intr);
	abc_fclose (inum);
	abc_dbclose (data);
}

void
ProcessPidFunc (
 void)
{
	strcpy (pogh_rec.co_no, comm_rec.co_no);
	pogh_rec.pid	=	processPID;
	cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
	       pogh_rec.pid != processPID)
	{
		/*-------------------
		| Goods Received	|
		-------------------*/
		if ((pogh_rec.pur_status [0] == poghFindStatus [0]) &&
			 ((pogh_rec.drop_ship [0] == 'Y' && dropShipment [0] == 'Y') ||
			 (pogh_rec.drop_ship [0] != 'Y' && dropShipment [0] != 'Y')))
			ProcessPogl (pogh_rec.hhgr_hash);
		
		cc = find_rec (pogh, &pogh_rec, NEXT, "r");
	}
	recalc_sobg ();
}

void
ProcessPogh (
 void)
{
	strcpy (pogh_rec.co_no, comm_rec.co_no);
	strcpy (pogh_rec.br_no, comm_rec.est_no);
	pogh_rec.hhsu_hash = 0L;
	pogh_rec.hhsh_hash = 0L;
	strcpy (pogh_rec.gr_no, fifteenSpaces);
	cc = find_rec (pogh, &pogh_rec, GTEQ, "r");

	while (!cc && 
		   !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
	       !strcmp (pogh_rec.br_no, comm_rec.est_no))
	{
		/*-------------------
		| Goods Received	|
		-------------------*/
		if ((pogh_rec.pur_status [0] == poghFindStatus [0]) &&
			 ((pogh_rec.drop_ship [0] == 'Y' && dropShipment [0] == 'Y') ||
			 (pogh_rec.drop_ship [0] != 'Y' && dropShipment [0] != 'Y')))
			ProcessPogl (pogh_rec.hhgr_hash);
		
		cc = find_rec (pogh, &pogh_rec, NEXT, "r");
	}
	recalc_sobg ();
}

/*===============================
| Process Goods receipts lines. |
===============================*/
void
ProcessPogl (
	long	hhglHash)
{
	abc_selfield (pogl,"pogl_id_no");
	pogl_rec.hhgr_hash = hhglHash;
	pogl_rec.line_no = 0;

	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	while (!cc && pogl_rec.hhgr_hash == hhglHash)
	{
	 	if (pogl_rec.pur_status [0] != poghFindStatus [0])
	   	{
			cc = find_rec (pogl, &pogl_rec, NEXT, "r");
			continue;
		}
		poso_rec.hhpl_hash	=	pogl_rec.hhpl_hash;
		cc = find_rec (poso, &poso_rec, GTEQ, "u");
		while (!cc && poso_rec.hhpl_hash == pogl_rec.hhpl_hash)
		{
			Process 
			(
				poso_rec.hhsl_hash, 
				poso_rec.itff_hash, 
				poso_rec.hhcl_hash, 
				poso_rec.hhpl_hash, 
				poso_rec.qty
			);
			abc_delete (poso);    

			poso_rec.hhpl_hash	=	pogl_rec.hhpl_hash;
			cc = find_rec (poso, &poso_rec, GTEQ, "u");
		}
		abc_unlock (poso);
		cc = find_rec (pogl, &pogl_rec, NEXT, "r");
	}
	abc_unlock (pogl);
}

void
IntFindSuper (
 char *item_no)
{
	if (!strcmp (item_no, "                "))
		return;

	strcpy  (inmr2_rec.co_no,   comm_rec.co_no);
	sprintf (inmr2_rec.item_no, "%-16.16s", item_no);
	cc = find_rec (inmr2, &inmr2_rec, EQUAL, "r");
	if (!cc)
		IntFindSuper (inmr2_rec.supercession);
}

int
IntFindInmr (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		return (cc);

	if (inmr_rec.hhsi_hash != 0L)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			return (cc);
	}

	if (strcmp (inmr_rec.supercession, "                "))
	{
		IntFindSuper (inmr_rec.supercession);
		inmr_rec.hhbr_hash 	= inmr2_rec.hhbr_hash;	
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	}
	return (cc);
}


/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".E%s / %s\n", clip (comm_rec.est_name),comm_rec.cc_name);
	if (dropShipment [0] == 'Y')
		fprintf (fout, ".EDIRECT DELIVERY CUSTOMERS INVOICE RELEASE AUDIT.\n");
	else
		fprintf (fout, ".EPURCHASE ORDER / SALES ORDER RELEASE AUDIT.\n");
	fprintf (fout, ".Eas at %-24.24s\n", SystemTime ());

	fprintf (fout, ".R================");
	fprintf (fout, "================");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "===========");
	fprintf (fout, "============");
	fprintf (fout, "===========================\n");

	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "===========================\n");

	fprintf (fout, "|    PO NO.     ");
	fprintf (fout, "|    GR NO.     ");
	fprintf (fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|             ITEM DESCRIPTION           ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|UOM.");
	fprintf (fout, "| QUANTITY  ");
 	fprintf (fout, "| CUST |CUST ACRO");
	if (dropShipment [0] == 'Y')
		fprintf (fout, "| INV NO |\n");
	else
		fprintf (fout, "| S/O NO |\n");
 
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|------|---------|--------|\n");

	fflush (fout);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}

void
Process (
	long	hhslHash,
	long	itffHash,
	long	hhclHash,
	long	hhplHash,
	float	qtyReceipted)
{
	cc = IntFindInmr (pogl_rec.hhbr_hash);

	if (!processPID)
		dsp_process ("Item ", inmr_rec.item_no);

	/*-------------------------------------
	| No quantity so why process anything |
	-------------------------------------*/
	if (qtyReceipted <= 0.00)
		return;
	
	/*---------------------------
	| Sales order line exists.  |
	---------------------------*/
	if (hhslHash > 0L)
		ProcessSalesOrder (hhslHash, qtyReceipted);

	/*------------------------
	| Transfer line exists.  |
	------------------------*/
	if (itffHash > 0L)
		ProcessTransfers (itffHash, qtyReceipted);
	
	/*-----------------------
	| Invoice line exists.  |
	-----------------------*/
	if (hhclHash > 0L)
		ProcessInvoices (hhclHash, hhplHash, qtyReceipted);
	
	return;
}

void
ProcessInvoices (
	long	hhclHash,
	long	hhplHash,
	float	qtyReceipted)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	
	coln2_rec.hhcl_hash	=	hhclHash;
	if (find_rec (coln2, &coln2_rec, EQUAL, "u"))
		return;

	cohr_rec.hhco_hash	=	coln2_rec.hhco_hash;
	if (find_rec (cohr, &cohr_rec, EQUAL, "r"))
		return;

	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	if (find_rec (cumr, &cumr_rec, EQUAL, "r"))
		return;

	poln_rec.hhpl_hash	=	hhplHash;
	if (find_rec (poln, &poln_rec, EQUAL, "r"))
		return;

	pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
	if (find_rec (pohr, &pohr_rec, EQUAL, "r"))
		return;

	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
		return;

	if (!processPID)
		dsp_process (" Item No. : ", inmr_rec.item_no);

	if (!printerPipeOpen)
	{
		OpenAudit ();
		printerPipeOpen = TRUE;
	}

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;
	
	fprintf (fout, "|%15.15s", pogl_rec.po_number);
	fprintf (fout, "|%15.15s", pogh_rec.gr_no);
	fprintf (fout, "|%16.16s", inmr_rec.item_no);
	fprintf (fout, "|%-40.40s", pogl_rec.item_desc);
	fprintf (fout, "|%10.10s", DateToString (pogl_rec.rec_date));
	fprintf (fout, "|%4.4s",  inum_rec.uom);
	fprintf (fout, "|%11.2f", pogl_rec.qty_rec);
	fprintf (fout, "|%-6.6s", cumr_rec.dbt_no);
	fprintf (fout, "|%-9.9s", cumr_rec.dbt_acronym);
	fprintf (fout, "|%-8.8s|\n", cohr_rec.inv_no);
	PrintInex ();
	fflush (fout);

	AddIntr (0);
	AddIntr (1);

	strcpy (coln2_rec.stat_flag, cohrSetStatus);

	cc = abc_update (coln2, &coln2_rec);
	if (cc)
		file_err (cc, "coln", "DBUPDATE");

	cohr_rec.hhco_hash	=	coln2_rec.hhco_hash;
	cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
	if (!cc)	
	{
		strcpy (cohr_rec.stat_flag, cohrSetStatus);
		cc = abc_update (cohr, &cohr_rec);
		if (cc)
			file_err (cc, "cohr", "DBUPDATE");
	}
	else
		abc_unlock (cohr);
}

/*==========================================
| Add inventory transactions to intr file. |
==========================================*/
void
AddIntr (
	int		isInvoice)
{
	double	sale_price;

	if (coln2_rec.q_order == 0)
		sale_price = (coln2_rec.gross - coln2_rec.erate_var);
	else
		sale_price = (coln2_rec.gross - coln2_rec.erate_var)/ coln2_rec.q_order;

	/*----------------------------
	| Add inventory transaction. |
	----------------------------*/
	strcpy (intr_rec.co_no, cohr_rec.co_no);
	strcpy (intr_rec.br_no, cohr_rec.br_no);
	intr_rec.hhbr_hash = coln2_rec.hhbr_hash;
	intr_rec.hhcc_hash = coln2_rec.incc_hash;
	intr_rec.hhum_hash = coln2_rec.hhum_hash;
	if (isInvoice)
	{
		intr_rec.type = 13;
		strcpy (intr_rec.ref1, cohr_rec.inv_no);
		strcpy (intr_rec.ref2, cumr_rec.dbt_no);
	}
	else
	{
		intr_rec.type = 12;
		strcpy (intr_rec.ref1, pohr_rec.pur_ord_no);
		strcpy (intr_rec.ref2, sumr_rec.crd_no);
	}
	if (comm_rec.inv_date > lsystemDate)
		intr_rec.date = comm_rec.inv_date;
	else
		intr_rec.date = lsystemDate;

	strcpy (intr_rec.batch_no, cohr_rec.batch_no);
	intr_rec.qty = coln2_rec.q_order;
	intr_rec.cost_price = coln2_rec.cost_price;
	intr_rec.sale_price = twodec (sale_price);
	strcpy (intr_rec.stat_flag, "0");
	cc = abc_add (intr, &intr_rec);
	if (cc) 
		file_err (cc, "intr", "DBADD");
}

void
ProcessSalesOrder (
	long	hhslHash,
	float	qtyReceipted)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	float	qty_order 	= 0.0,
			qty_supp 	= 0.0,
			qty_left 	= 0.0,
			cstock 		= 0.00;

	soln_rec.hhsl_hash	=	hhslHash;
	if (find_rec (soln, &soln_rec, EQUAL, "u"))
		return;

	if (!BACKORDER)
		return;
	
	sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
	if (find_rec (sohr, &sohr_rec, EQUAL, "r"))
		return;

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	if (find_rec (cumr, &cumr_rec, EQUAL, "r"))
		return;

	if (!processPID)
		dsp_process (" Item No. : ", inmr_rec.item_no);

	if (!printerPipeOpen)
	{
		OpenAudit ();
		printerPipeOpen = TRUE;
	}

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;
	
	fprintf (fout, "|%15.15s", 	pogl_rec.po_number);
	fprintf (fout, "|%15.15s", 	pogh_rec.gr_no);
	fprintf (fout, "|%16.16s", 	inmr_rec.item_no);
	fprintf (fout, "|%-40.40s", pogl_rec.item_desc);
	fprintf (fout, "|%10.10s", 	DateToString (pogl_rec.rec_date));
	fprintf (fout, "|%4.4s", 	inum_rec.uom);
	fprintf (fout, "|%11.2f", 	pogl_rec.qty_rec * CnvFct);
	fprintf (fout, "|%-6.6s", cumr_rec.dbt_no);
	fprintf (fout, "|%-9.9s", cumr_rec.dbt_acronym);
	fprintf (fout, "|%-8.8s|\n", sohr_rec.order_no);
	PrintInex ();
	fflush (fout);

	qty_order = soln_rec.qty_order + soln_rec.qty_bord;
	qty_supp  = soln_rec.qty_order + soln_rec.qty_bord;
		
	cstock = InternalMIN (qtyReceipted, pogl_rec.qty_rec);
		
	/*-----------------------------------------------------------
	| This next test is in case backorder release is run twice. |
	-----------------------------------------------------------*/
	qty_left = cstock;
	qty_supp = qty_order;

	if (cstock <= qty_order)
		qty_supp = InternalMIN (qty_left, cstock);

	if (qty_order < qty_supp)
		qty_supp = InternalMIN (qty_left, qty_order);

	soln_rec.qty_order = qty_supp;
	soln_rec.qty_bord  = qty_order - qty_supp;

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0,
		soln_rec.hhbr_hash, 
		soln_rec.hhcc_hash, 
		0L, 
		0.00
	);

	if (soln_rec.qty_order <= 0.00)
		return;

	cstock -= qty_supp;

	DeleteColn (soln_rec.hhsl_hash);

	strcpy (soln_rec.status, "C");
	strcpy (soln_rec.stat_flag, (envVarConOrders) ? "M" : "R");

	cc = abc_update (soln, &soln_rec);
	if (cc)
		file_err (cc, "soln", "DBUPDATE");

	UpdateSohr (soln_rec.hhso_hash);

	return;
}

/*==========================================================================
| Delete line from existing packing slip so _new backorder can be released. |
==========================================================================*/
void
DeleteColn (
	long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (coln);
		return;
	}
	abc_unlock (coln);	/* Unlock record. */
	abc_delete (coln);	/* Delete record. */
	return;
}

/*========================================
| Update Header to same status of lines. |
========================================*/
void
UpdateSohr (
	long	hhsoHash)
{
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, EQUAL, "u");
	if (!cc)	
	{
		strcpy (sohr_rec.sohr_new, "N");
		strcpy (sohr_rec.status, "C");
		strcpy (sohr_rec.stat_flag, (envVarConOrders) ? "M" : "R");
		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, "sohr", "DBUPDATE");
	}
	else
		abc_unlock (sohr);
}

void
ProcessTransfers (
 long	itffHash,
 float	qtyReceipted)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	float	qty_order 	= 0.0,
			qty_supp 	= 0.0,
			qty_left 	= 0.0,
			cstock 		= 0.00;
		
	itln_rec.itff_hash	=	itffHash;
	if (find_rec (itln, &itln_rec, EQUAL, "u"))
		return;

	if (!TR_BACKORDER)
		return;
	
	ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
	if (find_rec (ithr, &ithr_rec, EQUAL, "r"))
		return;

	ccmr_rec.hhcc_hash	=	itln_rec.r_hhcc_hash;
	if (find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		return;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	if (!printerPipeOpen)
	{
		OpenAudit ();
		printerPipeOpen = TRUE;
	}
	if (!processPID)
		dsp_process (" Item No. : ", inmr_rec.item_no);

	fprintf (fout, "|%15.15s", 	pogl_rec.po_number);
	fprintf (fout, "|%15.15s", 	pogh_rec.gr_no);
	fprintf (fout, "|%16.16s", 	inmr_rec.item_no);
	fprintf (fout, "|%-40.40s",	pogl_rec.item_desc);
	fprintf (fout, "|%10.10s",	DateToString (pogl_rec.rec_date));
	fprintf (fout, "|%4.4s", 	inum_rec.uom);
	fprintf (fout, "|%11.2f", 	pogl_rec.qty_rec);
	fprintf (fout, "|%-6.6s",  	(itln_rec.stock [0] == 'S') ? "STOCK."
		 					 								: " CUST ");
	fprintf (fout, "|%-9.9s", 	ccmr_rec.acronym);
	fprintf (fout, "| %06ld |\n",ithr_rec.del_no);
	fflush (fout);

	qty_order = itln_rec.qty_order + itln_rec.qty_border;
	qty_supp  = itln_rec.qty_order + itln_rec.qty_border;
		
	cstock = qtyReceipted;
		
	/*-----------------------------------------------------------
	| This next test is in case backorder release is run twice. |
	-----------------------------------------------------------*/
	qty_left = cstock;
	qty_supp = qty_order;

	if (cstock <= qty_order)
		qty_supp = InternalMIN (qty_left, cstock);

	if (qty_order < qty_supp)
		qty_supp = InternalMIN (qty_left, qty_order);

	itln_rec.qty_order  = qty_supp;
	itln_rec.qty_border = qty_order - qty_supp;

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0,
		itln_rec.hhbr_hash, 
		itln_rec.i_hhcc_hash, 
		0L, 
		0.00
	);

	if (itln_rec.r_hhbr_hash != 0L)
	{
		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0,
			itln_rec.r_hhbr_hash, 
			0L, 
			0L, 
			0.00
		);
	}
	else
	{
		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0,
			itln_rec.hhbr_hash, 
			itln_rec.r_hhcc_hash, 
			0L, 
			0.00
		);
	}

	if (itln_rec.qty_order <= 0.00)
		return;

	cstock -= qty_supp;

	strcpy (itln_rec.status, "U");

	cc = abc_update (itln, &itln_rec);
	if (cc)
		file_err (cc, itln, "DBUPDATE");

	UpdateIthr (itln_rec.hhit_hash);

	return;
}

/*========================================
| Update Header to same status of lines. |
========================================*/
void
UpdateIthr (
	long	hhitHash)
{
	ithr_rec.hhit_hash	=	hhitHash;
	cc = find_rec (ithr, &ithr_rec, EQUAL, "u");
	if (!cc)	
	{
		ithr_rec.del_no = 0L;
		strcpy (ithr_rec.type, "U");
		strcpy (ithr_rec.printed , "N");
		cc = abc_update (ithr, &ithr_rec);
		if (cc)
			file_err (cc, ithr, "DBUPDATE");
	}
	else
		abc_unlock (ithr);
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout, "|%15.15s", 	" ");
		fprintf (fout, "|%15.15s", 	" ");
		fprintf (fout, "|%16.16s", 	" ");
		fprintf (fout, "|%-40.40s", inex_rec.desc);
		fprintf (fout, "|%10.10s",	" ");
		fprintf (fout, "|%4.4s", 	" ");
		fprintf (fout, "|%11.11s", 	" ");
		fprintf (fout, "|%6.6s",   " ");
		fprintf (fout, "|%9.9s", 	" ");
		fprintf (fout, "|%8.8s|\n", " ");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
