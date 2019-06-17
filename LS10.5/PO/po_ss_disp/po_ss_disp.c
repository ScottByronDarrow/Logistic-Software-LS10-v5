/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_ss_disp.c,v 5.9 2002/07/18 07:00:28 scott Exp $
|  Program Name  : (po_ss_disp.c)
|  Program Desc  : (Supplier Status Display)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.  Date Written  : (12/02/93)       |
|---------------------------------------------------------------------|
| $Log: po_ss_disp.c,v $
| Revision 5.9  2002/07/18 07:00:28  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.8  2002/07/16 02:42:29  scott
| Updated from service calls and general maintenance.
|
| Revision 5.7  2002/06/25 03:16:44  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.6  2002/03/11 04:06:17  scott
| Updated to remove disk based sort routines.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_ss_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_ss_disp/po_ss_disp.c,v 5.9 2002/07/18 07:00:28 scott Exp $";

#define	 TXT_REQD
#include <pslscr.h>
#include <twodec.h>
#include <ring_menu.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <arralloc.h>

#define	X_OFF	lpXoff
#define	Y_OFF	lpYoff

#ifdef PSIZE
#undef PSIZE
#endif

#define	PSIZE		13		/* Max lines in a page on screen	 */
#define	TOT_OFFSET	4
#define	CAL (amt, pc)	 (amt * DOLLARS (pc))

#define	INVOICE		 (suin_rec.type [0] == '1')
#define	MCURR		 (envVarCrMcurr [0] == 'Y')

#define	SAME_PO		 (strcmp (previousOrder, pohr_rec.pur_ord_no))

static	char	*sup_pri [] = 
				{ 
					"Supplier not ranked. ",
					"Number One Supplier. ", 
					"Number Two Supplier. ", 
					"Number Three Supplier", 
					"Number Four Supplier ", 
					"Number Five Supplier." 
				};

	char	*forty_space = "                                        " ;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct exsiRecord	exsi_rec;
struct poclRecord	pocl_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct ponsRecord	pons_rec;
struct sudtRecord	sudt_rec;
struct suhdRecord	suhd_rec;
struct suinRecord	suin_rec;
struct suinRecord	suin2_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct suntRecord	sunt_rec;
struct sudsRecord	suds_rec;
struct inspRecord	insp_rec;
struct ingpRecord	ingp_rec;
struct inisRecord	inis_rec;
struct qasdRecord	qasd_rec;

float	*insp_qty_brk	=	&insp_rec.qty_brk1;
double	*insp_price		=	&insp_rec.price1;
float	*suds_qty_brk	=	&suds_rec.qty_brk1;
float	*suds_disca_pc  =	&suds_rec.disca_pc1;
float	*suds_discb_pc  =	&suds_rec.discb_pc1;
float	*suds_discc_pc  =	&suds_rec.discc_pc1;
Money	*sumr_bo_per	=	&sumr_rec.bo_curr;
int		*sumr_sic		=	&sumr_rec.sic1;

	char	*data  = "data",
			*suin2 = "suin2",
			*sumr2 = "sumr2";

	int		windowCnt 		= 0,
			mainWindowOpen 	= FALSE,
			masterFileOpen 	= FALSE,
			totalOpen 		= FALSE,
			lpXoff			= 0,
			lpYoff			= 0,
			invoiceLineNum	= 0,
			disp 			= TRUE,
			displayOK		= 0,
			envVarCrCo 		= 0,
			envVarCrFind 	= 0,
			envVarDbCo 		= 0,
			envVarDbFind 	= 0,
			envVarSoDiscRev = FALSE,
			clearOK 		= TRUE,
			printOK 		= FALSE;

	long	currentHhsuHash	= 0L;

	float	orderQuantity	= 0.00,
			totalQuantity	= 0.00;

	double 	a_dbt 		= 0.00,
			u_dbt 		= 0.00,
			t_inv 		= 0.00,
			t_crd 		= 0.00,
			t_jnl 		= 0.00,
			t_chq 		= 0.00,
			orderAmount = 0.00,
			totalAmount = 0.00,
			balance 	= 0.00;

	char	statusDesc 		[31],
			displayString 	[300],
			branchNumber 	[3],
			db_branchNumber [3],
			envVarCrMcurr 	[2],
			previousOrder 	[sizeof pohr_rec.pur_ord_no],
			headingString 	[200],
			workAmounts 	[3][16];

	char	*ser_space = "                         ";
	char	*month_nm [12] = 
			{
				"January",
				"February",
				"March",
				"April",
				"May",
				"June",
				"July",
				"August",
				"September",
				"October",
				"November",
				"December"
			};

	struct	{
		char	*_code;
	 	char	*_desc;
	} lineStatus [] =  {
		{"U", "U(napproved"},
		{"D", "D(eleted   "},
		{"O", "O(pen      "},
		{"C", "C(onfirmed "},
		{"c", "c(osted.   "},
		{"R", "R(eceipted "},
		{"r", "r(ecpt Over"},
		{"T", "T(ransmited"},
		{"I", "I(n Transit"},
		{"H", "H(eld Line "},
		{"X", "X Cancelled"},
		{"",""},
	};

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[73];
	char	itemNo		[sizeof inmr_rec.item_no];
	char	supItemNo	[sizeof inis_rec.sup_part];
	char	itemDesc	[sizeof inmr_rec.description];
	char	supPriority	[sizeof inis_rec.sup_priority];
	long	lcostDate;
	long	fobCost;
	long	hhsuHash; 
	long	hhbrHash;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

/*
 * Callback Function Declarations 
 */
int 	AllDisplay 			(void);
int 	MasterDisplay 		(void);
int 	DisplayTotal 		(void);
int 	DisplayInvoice 		(void);
int 	DisplayCheque 		(void);
int 	ChequeExchHistory 	(void);
int 	OrderDisplay 		(void);
int 	NoteMaint 			(void);
int 	SupplierPricing 	(void);
int 	SupplierDiscounts 	(void);
int 	PoApproval 			(void);
int 	QA_display 			(void);
int 	ClearReDraw 		(void);
int 	HeadPrint 			(void);
int 	NextDislay 			(void);
int 	PrevDisplay 		(void);

#ifndef GVISION
menu_type	_main_menu [] = {
{ "<Supplier Status.>","Display Supplier Status Screen. [ S ]",AllDisplay,"Ss",},
{ "<Master File Details.>","Display Supplier Master File. [ M ]",MasterDisplay,"Mm", },
{ "<Total Screen.>","Display Supplier totals. [ T ]",DisplayTotal,"Tt", 	     },
{ "<Invoices.>","Display Supplier Invoices. [ I ]",DisplayInvoice,"Ii",        },
{ "<Cheques.>","Display Supplier Cheques. [ C ]", DisplayCheque, "Cc",	     },
{ "<Cheque Exch. Details.>" ,"Display Cheque Exchange rate details. [ C ]",ChequeExchHistory,"Cc",  },
{ "<Purchase Orders.>","Display Purchase Orders. [ P ]", OrderDisplay,"Pp",    },
{ "<Note Pad.>","Maintain Supplier Note Pad. [ N ]", NoteMaint, "Nn",	     },
{ "<Supplier Pricing.>","Display Supplier Pricing. [ S ]", SupplierPricing, "Ss",     },
{ "<Supplier Discounting.>","Display Supplier Discounting. [ D ]",SupplierDiscounts,"Dd",},
{ "<P/O A&A info.>","Display P/O Author / Approval details. [ P ]", PoApproval,"Pp",    },
{ "<QA info.>","Display QA Infrmation. [ Q ]", QA_display,"Qq",    },
{ "[REDRAW]", "Redraw Display", ClearReDraw, "", FN3,			     },
{ "[PRINT]", "Print Screen", HeadPrint, "", FN5,			     },
{ "[NEXT SCN]", "Display Next Supplier", NextDislay, "", FN14,	     },
{ "[PREV SCN]", "Display Prev Supplier", PrevDisplay, "", FN15,	     },
{ "[EDIT / END]", "Exit Display", _no_option, "", FN16, EXIT | SELECT	     },
{ "",									     },
};
#else
menu_group _main_group [] = {
	{1, "Main Screens"},
	{2, "Financial Screens"},
	{3, "Purchase Information"},
	{4, "Pricing/Discounting"},
	{5, "Miscellaneous"},
	{0, ""}					/* terminator item */
};
menu_type	_main_menu [] = {
{1, "<Supplier Status>", "Supplier Status",	AllDisplay,	},
{1, "<Master File Details.>", "Master File Details", MasterDisplay, },
{2, "<Invoices.>", "Invoices", DisplayInvoice, },
{2, "<Cheques.>", "Cheques", DisplayCheque, },
{2, "<Cheque Exch. Details.>", "Cheque Exchange Rate Details" , ChequeExchHistory, },
{2, "<Total Screen.>", "Total Screen", DisplayTotal, },
{3, "<Purchase Orders.>","Purchase Orders", OrderDisplay, },
{3, "<P/O A&A info.>","P/O Author / Approval Details", PoApproval, },
{3, "<QA info.>", "Quality Assurance information", QA_display, },
{4, "<Supplier Pricing.>", "Supplier Pricing", SupplierPricing, },
{4, "<Supplier Discounting.>", "Supplier Discounting", SupplierDiscounts, },
{5, "<Note Pad.>", "Note Pad", NoteMaint, },
{5, "[REDRAW]", 					"", ClearReDraw, FN3,},
{5, "[PRINT]", 						"", HeadPrint, FN5,},
{5, "[NEXT SCN]", 					"", NextDislay, FN14,},
{5, "[PREV SCN]", 					"", PrevDisplay, FN15,},
{0, "",} 
};
#endif
                            
/*
 * The structures 'cheq'&'dtls' are initialised in function 'GetCheque'
 * the number of cheques is stored in external variable 'chequeCnt'.  
 * the number of details is stored in external variable 'detailCnt'. 
 */
static	struct	Cheque {	/*-------------------------------*/
	char	no [16];		/*| cheque reciept number.	    |*/
	char	type [2];		/*| Cheque Type.     	    	|*/
	char	app [4];		/*| Transaction Type.     	    |*/
	char	bank_id [6];	/*| Bank ID.              	    |*/
	double	bank_amt;		/*| Bank Receipt Amount.		|*/
	double	bank_exch;		/*| Bank Exchange Rate.			|*/
	double	bank_chg;		/*| Bank Charge Amount.			|*/
	char	datec [11];		/*| date of payment.      	    |*/
	char	datep [11];		/*| date posted.                |*/
	double	amount;			/*| total amount of cheque.	    |*/
	double	cdisc;			/*| discount given.		    	|*/
	double	loc_amt;		/*| total local amount of cheque|*/
	double	loc_dis;		/*| total local discount.       |*/
	double	ex_var;			/*| Exchange variation.  	    |*/
} *cheq;         	        /*-------------------------------*/

static DArray cheq_d;
static int	chequeCnt;

static struct Detail { 		/*-------------------------------*/
	long	hhsiHash;		/*| detail invoice reference.   |*/
	int		fwd_payment;	/*| detail forward payments.    |*/
	double	inv_amt;		/*| detail invoice amount.      |*/
	double	inv_exch_rate;	/*| invoice exchange rate.	    |*/
	double	inv_loc_amt;	/*| invoice local amount.	    |*/
	double	inv_ex_var;		/*| invoice exchange variation.	|*/
	int		cheq_hash;		/*| cheq structure pointer.     |*/
} *dtls;					/*-------------------------------*/

static DArray dtls_d;
static int	detailCnt;

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char 	supplierNumber [7];
	char 	previousSupplier [7];
	char	text [61];
	long	lsystemDate;
	char	supplierDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber",	 2, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Supplier Number :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.supplierNumber},

	{2, TXT, "note_pad",	12, 0,CHARTYPE,
		"","          ",
		" ","", " Maintain Note Pad ","",
		6, 60, 16, "", "", local_rec.text},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, YES, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<get_lpno.h>
#include 	<FindSumr.h>
#include 	<chk_ring_sec.h>

/*
 * Function Declarations 
 */
double 	CalcPo 				(long);
float 	ScreenDisc 			(float);
int 	CalcCheque 			(void);
int 	ChangeData 			(int);
int 	FindSuin 			(long);
int 	GetCheque 			(void);
int 	PrintLine 			(long);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	AlternateMess 		(char *);
void 	CloseDB 			(void);
void 	Dsp_Heading 		(void);
void 	GetExchCheques 		(int, int *, char *);
void 	OpenDB 				(void);
void 	PrintCoStuff 		(void);
void 	PrintTotal 			(int);
void 	ProcessInvoice 		(void);
void 	ReDraw 				(void);
void 	Seperator 			(void);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));
	envVarDbCo 		= atoi (get_env ("DB_CO"));
	envVarDbFind 	= atoi (get_env ("DB_FIND"));

	sptr = chk_env ("SO_DISC_REV");
	envVarSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check for multi-currency. 
	 */
	sprintf (envVarCrMcurr, "%-1.1s", get_env ("CR_MCURR"));

	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	/*
	 *    Allocate intital cheque and detail buffers
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 1000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	input_row 	= 2,
	error_line 	= 20;

	init_scr 	();
	set_tty 	();
	swide 		();
	set_masks 	();	
	init_vars 	(1);

	tab_row = 5; 
	tab_col = 5;

	/*
	 * Open main database files. 
	 */
	OpenDB ();

	strcpy (local_rec.supplierDate, DateToString (comm_rec.crd_date));

	strcpy (branchNumber,   (envVarCrCo) ? comm_rec.est_no : " 0");
	strcpy (db_branchNumber,(envVarDbCo) ? comm_rec.est_no : " 0");

	_chk_ring_sec (_main_menu, "po_ss_disp");

	sprintf (local_rec.previousSupplier,"%-6.6s"," ");

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		displayOK 	= FALSE;
		search_ok 	= TRUE;
		clearOK 	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		displayOK 	= TRUE;
		clearOK 	= FALSE;
		heading (1);
#ifndef GVISION
		run_menu (_main_menu,"",20);
#else
		run_menu (_main_group, _main_menu);
#endif
		crsr_on ();
		strcpy (local_rec.previousSupplier,sumr_rec.crd_no);

	}
	ArrDelete (&cheq_d);
	ArrDelete (&dtls_d);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files . 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (suin2, suin);
	abc_alias (sumr2, sumr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");

	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhsu_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (suin2,suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (pocl, pocl_list, POCL_NO_FIELDS, "pocl_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	open_rec (sunt, sunt_list, SUNT_NO_FIELDS, "sunt_id_no");
	open_rec (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhsu_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pons, pons_list, PONS_NO_FIELDS, "pons_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	if (masterFileOpen)
		Dsp_close ();

	if (totalOpen)
		Dsp_close ();

	if (mainWindowOpen)
		Dsp_close ();

	abc_fclose (comm);
	abc_fclose (exsi);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (pocf);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (pocl);
	abc_fclose (suin);
	abc_fclose (suin2);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (sunt);
	abc_fclose (suds);
	abc_fclose (insp);
	abc_fclose (ingp);
	abc_fclose (inis);
	abc_fclose (inum);
	abc_fclose (pons);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate supplier number. 
	 */
	if (LCHECK ("supplierNumber"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNumber));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		currentHhsuHash = sumr_rec.hhsu_hash;
		GetCheque ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * display next supplier
 */
int
NextDislay (void)
{
	ChangeData (FN14);
	ReDraw ();
    return (EXIT_SUCCESS);
}
/*
 * Order Display. 			
 */
int
OrderDisplay (void)
{
	totalQuantity 	= 0.00;
	totalAmount 	= 0.00;
	lpXoff 	= 0;
	lpYoff 	= 3;
	Dsp_prn_open 
	(
		0, 
		3, 
		PSIZE, 
		headingString, 
		comm_rec.co_no, 
		comm_rec.co_name,
		comm_rec.est_no, 
		comm_rec.est_name,
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec ("PURCHASE ORDER |BR|   DATE   |UOM.|   QTY  |  QTY   |   EST VALUE   |      ITEM      |     ITEM DESCRIPTION      | PURCHASE ORDER ");
	Dsp_saverec ("   NUMBER      |NO|   DUE    |    |  ORDER | REMAIN |  P/O  LINE.   |     NUMBER     |                           |     STATUS     ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");

	sprintf (previousOrder, "%15.15s", " ");

	/*
	 * Display details for all purchase order for a supplier. 
	 */
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == pohr_rec.hhsu_hash) 
	{
		/*
		 * Purchase order is deleted. 
		 */
		if (pohr_rec.status [0] == 'D') 
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}

		cc = PrintLine (pohr_rec.hhpo_hash);

		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	PrintTotal (FALSE);
	Dsp_srch ();
	Dsp_close ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

/*
 * Print purchase order lines. 
 */
int
PrintLine (
 long hhpo_hash)
{
	char	envLine [200],
			date1 [11],
			workQuantity [2] [8],
			serial [25],
			purchaseOrderString [21];

	int		i,
			printed = FALSE;

	double	balance = 0.0,
			qty_ord = 0.0,
			qty_rem = 0.0;

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	orderQuantity = 0.00;
	orderAmount = 0.00;

	poln_rec.hhpo_hash 	= hhpo_hash;
	poln_rec.line_no 	= 0;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpo_hash) 
	{
		if (pohr_rec.type [0] == 'C')
		{
			if (poln_rec.pur_status [0] == 'O')
			{
				qty_rem = (double) poln_rec.qty_rec;
				qty_ord = (double) poln_rec.qty_rec;
			}
			else
			{
				qty_rem = (double) poln_rec.qty_ord - poln_rec.qty_rec;
				qty_ord = (double) poln_rec.qty_ord;
			}
			qty_rem = qty_rem * -1;
			qty_ord = qty_ord * -1;
		}
		else
		{
			qty_rem = (double) poln_rec.qty_ord - poln_rec.qty_rec;
			qty_ord = (double) poln_rec.qty_ord;
		}

		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		if (find_rec (inmr,&inmr_rec,EQUAL,"r"))
		{
			strcpy (inmr_rec.item_no," ");
			strcpy (inmr_rec.description," ");
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;

		balance = twodec (poln_rec.land_cst);
		balance = out_cost (balance, inmr_rec.outer_size);
		balance *= qty_rem;

		orderQuantity += (float) ((qty_rem * CnvFct));
		totalQuantity += (float) ((qty_rem * CnvFct));
		orderAmount += balance;
		totalAmount += balance;

		sprintf (date1, "%-10.10s", DateToString (poln_rec.due_date));

		if (inmr_rec.serial_item [0] == 'Y') 
			sprintf (serial, "%-25.25s", poln_rec.serial_no);
		else
			strcpy (serial, ser_space);

		strcpy (statusDesc, "??????????????????????????");

		for (i = 0; strlen (lineStatus [i]._code); i++)
		{
			if (poln_rec.pur_status [0] == lineStatus [i]._code [0])
			{
				strcpy (statusDesc,ML (lineStatus [i]._desc));
				break;
			}
		}
		if (qty_rem != 0.00)
			sprintf (workQuantity [0], "%7.2f", qty_rem * CnvFct);
		else
			strcpy (workQuantity [0], "       ");

		if (qty_ord != 0.00)
			sprintf (workQuantity [1], "%7.2f", qty_ord * CnvFct);
		else
			strcpy (workQuantity [1], "       ");
		
		if (SAME_PO)
			sprintf (purchaseOrderString, "^1%s^6", pohr_rec.pur_ord_no);
		else
			sprintf (purchaseOrderString, "%15.15s", " ");

		sprintf 
		(
			envLine,
			"%s^E%2.2s^E%10.10s^E%4.4s^E%7.7s ^E%7.7s ^E%14.14s ^E%-16.16s^E%-27.27s^E %s",
			purchaseOrderString,
			pohr_rec.br_no,
			date1,
			inum_rec.uom,
			workQuantity [1], workQuantity [0],
			comma_fmt (balance, "NNN,NNN,NNN.NN"),
			inmr_rec.item_no,
			inmr_rec.description,
			statusDesc
		);

		Dsp_saverec (envLine);

		if (strcmp (serial, ser_space))
		{
			sprintf (envLine,"               ^E  ^E          ^E    ^E        ^E        ^E               ^E Serial Number : %25.25s  ^E                ", serial);
			Dsp_saverec (envLine);
		}
		printed = TRUE;

		pons_rec.hhpl_hash	=	poln_rec.hhpl_hash;
		pons_rec.line_no	=	0;
		cc = find_rec (pons, &pons_rec, GTEQ, "r");
		while (!cc && pons_rec.hhpl_hash == poln_rec.hhpl_hash)
		{
			sprintf (envLine, "               ^E  ^E          ^E    ^E        ^E        ^E               ^E %40.40s   ^E                ",pons_rec.desc);

			Dsp_saverec (envLine);
			cc = find_rec (pons, &pons_rec, NEXT, "r");
		}
		strcpy (previousOrder,pohr_rec.pur_ord_no);

		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	if (printed)
		PrintTotal (TRUE);

	return (EXIT_SUCCESS);
}

void
PrintTotal (
 int end_tot)
{
	char	envLine [200];
  	sprintf (envLine, "%s^E  ^E          ^E    ^E        ^E%7.1f ^E%14.14s ^6^E                ^E                           ^E",
		 (end_tot) ? "  ORDER TOTAL  " : "^1  GRAND TOTAL  ",
		 (end_tot) ? orderQuantity : totalQuantity,
		 (end_tot) ? comma_fmt (orderAmount, "NNN,NNN,NNN.NN") 
		 	 : comma_fmt (totalAmount, "NNN,NNN,NNN.NN"));

	Dsp_saverec (envLine);

	if (end_tot)
     		Dsp_saverec ("^^GGGGGGGGGGGGGGGEGGEGGGGGGGGGGEGGGGEGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGG");
	else
     		Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGJGGGGGGGGGGJGGGGJGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGG");
	orderQuantity = 0.00;
	orderAmount = 0.00;
}

int
PoApproval (void)
{
	int	i;

	lpXoff = 0;
	lpYoff = 3;
	Dsp_prn_open (9, 3, PSIZE - 2, 
		     headingString, 
		     comm_rec.co_no, comm_rec.co_name,
		     comm_rec.est_no, comm_rec.est_name,
		    (char *) 0, (char *) 0);

	Dsp_saverec ("PURCHASE ORDER | BR |   CREATED BY   |    DATE    |   TIME  |  APPROVED  BY  |    VALUE OF    | PURCHASE ORDER STATUS ");
	Dsp_saverec ("    NUMBER     | NO |                |   CREATED  | CREATED |      CODE      | PURCHASE ORDER |                       ");
	Dsp_saverec (" [REDRAW]   [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [INPUT / END] ");

	/*
	 * Display details for all purchase order for a supplier. 
	 */
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == pohr_rec.hhsu_hash) 
	{
		/*
		 * Purchase order is deleted. 
		 */
		if (pohr_rec.status [0] == 'D' || pohr_rec.type [0] == 'C') 
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}

		strcpy (statusDesc, "??????????????????????????");

		for (i = 0; strlen (lineStatus [i]._code); i++)
		{
			if (pohr_rec.status [0] == lineStatus [i]._code [0])
			{
				strcpy (statusDesc, ML (lineStatus [i]._desc));
				break;
			}
		}
		sprintf 
		(
			displayString, 
			"%15.15s^E %2.2s ^E %14.14s ^E %10.10s ^E  %5.5s  ^E %15.15s^E%15.15s ^E %-19.19s",
			pohr_rec.pur_ord_no,
			pohr_rec.br_no,
			pohr_rec.op_id,
			DateToString (pohr_rec.date_create),
			pohr_rec.time_create,
			pohr_rec.app_code,
			comma_fmt (CalcPo (pohr_rec.hhpo_hash),"NNNN,NNN,NNN.NN"),
			statusDesc
		);
		Dsp_saverec (displayString);

		if (strcmp (pohr_rec.req_usr, forty_space) || 
			strcmp (pohr_rec.reason,  forty_space))
		{
			sprintf 
			(
				displayString,
				"  ^1Person Request^6 : %46.46s  /  ^1Reason^6 : %40.40s ",
				pohr_rec.req_usr, 
				pohr_rec.reason
			);
			Dsp_saverec (displayString);
		}
		pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
		cc = find_rec ("pohr", &pohr_rec, NEXT, "r");
	}
	Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGG");
	Dsp_srch ();
	Dsp_close ();

	ReDraw ();
    return (EXIT_SUCCESS);
}

/*
 * Calculate total value of lines allocated to purchase order. 
 */
double	
CalcPo (
 long hhpo_hash)
{
	double	line_tot = 0.00,
		total_po = 0.00;

	poln_rec.hhpo_hash = hhpo_hash;
	poln_rec.line_no = 0;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpo_hash)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			inmr_rec.outer_size = 1.00;

		line_tot = twodec (poln_rec.land_cst);
		line_tot = out_cost (line_tot, inmr_rec.outer_size);
		line_tot *= poln_rec.qty_ord - poln_rec.qty_rec;
		total_po += line_tot;
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	return (total_po);
}

int
ClearReDraw (void)
{
	swide ();
	clear ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
ReDraw (void)
{
	clearOK = FALSE;
	heading (1);
}

/*
 * display previous item	
 */
int
PrevDisplay (void)
{
	ChangeData (FN15);
	ReDraw ();
    return (EXIT_SUCCESS);
}
/*
 * Finds Next or Previous Record
 * key = NEXT_FND - Next Record	
 * key = PREV_FND - Prev Record
 */
int
ChangeData (
	int		key)
{
	char	lastSupplier [7];

	strcpy (lastSupplier,sumr_rec.crd_no);

	cc = find_rec (sumr, &sumr_rec, (key == FN14) ? GTEQ : LTEQ, "r");
	if (!cc)
		cc = find_rec (sumr, &sumr_rec, (key == FN14) ? NEXT : PREVIOUS, "r");
		
	if (cc || 
	    strcmp (sumr_rec.co_no, comm_rec.co_no) ||
	    strcmp (sumr_rec.est_no, branchNumber))
	{
		AlternateMess (ML (mlPoMess136));
		strcpy (sumr_rec.co_no,  comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, lastSupplier);
		cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	}

	currentHhsuHash = sumr_rec.hhsu_hash;
	strcpy (local_rec.supplierNumber, sumr_rec.crd_no);
	strcpy (local_rec.previousSupplier, sumr_rec.crd_no);
	GetCheque ();
	return (EXIT_SUCCESS);
}

int
AllDisplay (void)
{
	int	j;
	int	cr_ext_day = 0;
	float	percent [6];
	double	balance = 0.00;
	char	wk_str [12];

	displayOK = 1;

	sprintf (headingString, 
		"Customer %s (%s)", 
		sumr_rec.crd_no,
		sumr_rec.crd_name);

	for (j = 0; j < 6; j++)
		percent [j] = 0.00;

	balance = sumr_bo_per [0] +
			  sumr_bo_per [1] +
			  sumr_bo_per [2] +
			  sumr_bo_per [3];

	if (balance != 0)
	{
		for (j = 0; j < 4; j++)
			percent [j] = (float) ((sumr_bo_per [j] / balance) * 100.00);
	}

	if (masterFileOpen)
	{
		Dsp_close ();
		masterFileOpen = FALSE;
	}

	if (totalOpen)
	{
		Dsp_close ();
		totalOpen = FALSE;
	}

	if (mainWindowOpen)
		Dsp_close ();

	mainWindowOpen = TRUE;

	lpXoff = 0;
	lpYoff = 1;
	Dsp_nc_prn_open (0, 1, 15, 
			 headingString,
			 comm_rec.co_no, comm_rec.co_name, 
			 comm_rec.est_no, comm_rec.est_name, 
			 (char *) 0, (char *) 0);

	windowCnt = 1;
	sprintf (displayString,
		". Supplier Number : %6.6s (%9.9s) %42.42s Name : %40.40s  ",
		sumr_rec.crd_no,
		sumr_rec.acronym,
		" ",
		sumr_rec.crd_name);

	Dsp_saverec (displayString);
	Dsp_saverec ("");
	Dsp_saverec ("");

	sprintf (displayString, " Address       : %-40.40s ", sumr_rec.adr1);
	Dsp_saverec (displayString);
	sprintf (displayString, "               : %-40.40s ", sumr_rec.adr2);
	Dsp_saverec (displayString);
	sprintf (displayString, "               : %s, %-40.40s ", 
				clip (sumr_rec.adr3), sumr_rec.adr4);
	Dsp_saverec (displayString);

	Seperator ();

	/*
	 * Lookup and Display Currency Code. 
	 */
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", sumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", "Unknown Currency");

	sprintf (displayString, 
		" Currency     : %-3.3s    (%-40.40s)",
		sumr_rec.curr_code,
		pocr_rec.description);
	Dsp_saverec (displayString);

	/*
	 * Lookup and Display Country Code. 
	 */
	strcpy (pocf_rec.co_no, comm_rec.co_no);
	sprintf (pocf_rec.code, "%-3.3s", sumr_rec.ctry_code);
	cc = find_rec (pocf, &pocf_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocf_rec.description, "%-40.40s", "Unknown Country");

	sprintf (displayString, 
		" Country      : %-3.3s    (%-40.40s)",
		sumr_rec.ctry_code,
		pocf_rec.description);
	Dsp_saverec (displayString);

	Seperator ();
	sprintf (displayString, 
		" Phone #       : %-15.15s (%20.20s) (%20.20s)  Fax Number                 : %15.15s",
		sumr_rec.cont_no,
		sumr_rec.cont_name, 
		sumr_rec.cont2_name,
		sumr_rec.fax_no);
	Dsp_saverec (displayString);

	strcpy (workAmounts [0], 
		      comma_fmt (DOLLARS (sumr_rec.mtd_exp),"NNN,NNN,NNN.NN"));
	strcpy (workAmounts [1], 
		      comma_fmt (DOLLARS (sumr_rec.ytd_exp),"NNN,NNN,NNN.NN"));

	sprintf (displayString, 
		" Mtd Purch.    :%14.14s   (%20.20s)%25.25sYtd Purchases              : %14.14s ",
		workAmounts [0], 
		sumr_rec.cont3_name,
		" ", 
		workAmounts [1]);
	Dsp_saverec (displayString);

	Seperator ();

	/*
	 * Calculate balance and forward cheque amount. 
	 */
	CalcCheque ();

	strcpy (workAmounts [0], comma_fmt (DOLLARS (balance), "NN,NNN,NNN.NN"));

	sprintf (displayString, " Balance       :%13.13s  %7.2f%% %40.40s Payment Terms              :   %11.11s",
		workAmounts [0], 
		100.00,
		" ",
		sumr_rec.pay_terms);
	Dsp_saverec (displayString);

	cr_ext_day = atoi (sumr_rec.pay_terms) - 30;
	if (cr_ext_day <= 0)
		strcpy (wk_str, "       NONE");
	else
		sprintf (wk_str, "%3d  Days  ", cr_ext_day);

	sprintf (displayString, 
		" Current       :%13.13s  %7.2f%% %40.40s Credit Extension           :   %11.11s",
		comma_fmt (DOLLARS (sumr_bo_per [0]), "NN,NNN,NNN.NN"),
		percent [0], " ", wk_str);
	Dsp_saverec (displayString);

	sprintf (displayString, 
		" Overdue 1     :%13.13s  %7.2f%% %40.40s Payments made by           :        %6.6s",
		comma_fmt (DOLLARS (sumr_bo_per [1]), "NN,NNN,NNN.NN"),
		percent [1], " ", 
		 (sumr_rec.pay_method [0] == 'D') ? " Draft" : "Cheque");
	Dsp_saverec (displayString);

	sprintf (displayString, 
		" Overdue 2     :%13.13s  %7.2f%% %40.40s Payments to Supplier held  :           %3.3s",
		comma_fmt (DOLLARS (sumr_bo_per [2]), "NN,NNN,NNN.NN"),
		percent [2], " ",
		 (sumr_rec.hold_payment [0] == 'Y') ? "Yes" : " No");


	Dsp_saverec (displayString);

	sprintf (displayString, 
		" Overdue 3     :%13.13s  %7.2f%%",
		comma_fmt (DOLLARS (sumr_bo_per [3]), "NN,NNN,NNN.NN"),
		percent [3]);

	Dsp_saverec (displayString);

	Dsp_srch ();

    return (EXIT_SUCCESS);
}

int
DisplayInvoice (void)
{
	a_dbt = 0.0;
	u_dbt = 0.0;
	t_crd = 0.0;
	t_jnl = 0.0;
	t_chq = 0.0;
	invoiceLineNum = 0;

	printOK = TRUE;

	/*
	 * Store Heading Details.            
	 */
	if (disp)
	{
		lpXoff = 0;
		lpYoff = 3;
		Dsp_prn_open (0, 3, PSIZE, 
		     headingString, 
		     comm_rec.co_no, comm_rec.co_name,
		     comm_rec.est_no, comm_rec.est_name,
		    (char *) 0, (char *) 0);

		Dsp_saverec ("     INVOICE      |A| INVOICE  |   INVOICE   |   BALANCE   |     PAY     |  EXCHANGE  |   BALANCE  |     CHEQUE    |    CHEQUE    ");
		Dsp_saverec ("     NUMBER       |P| DUE DATE |   AMOUNT    |     DUE     |    AMOUNT   |    RATE    |    LOCAL   |     NUMBER    |    AMOUNT    ");
		Dsp_saverec (" [REDRAW]   [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [INPUT / END] ");
	}

	/*
	 * Store Line Details.
	 */
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.est, "  ");
	suin_rec.date_of_inv = 0L;
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
	{
		ProcessInvoice ();
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}

	/*
	 * Display Screen.                   
	 */
	if (disp)
	{
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGJGJGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGG");
		Dsp_srch ();
		Dsp_close ();
	}
	printOK = FALSE;

	if (disp)
		AllDisplay ();

    return (EXIT_SUCCESS);
}

void
ProcessInvoice (void)
{
	int 	i,
			none;

	char	date1 [11], 
			date2 [11],
			inv_hdr1 [200],
			inv_hdr2 [200],
			envLine [200];

	double	balance = 0.0,
	      	loc_bal = 0.0;

	balance = suin_rec.amt - suin_rec.amt_paid;
	
	if (suin_rec.type [0] == '1')
	{
		if (suin_rec.approved [0] == 'N')
			u_dbt += suin_rec.amt;
		else
			a_dbt += suin_rec.amt;
	}

	if (suin_rec.type [0] == '2')
		t_crd -=  suin_rec.amt;
		
	if (suin_rec.type [0] == '3')
		t_jnl += suin_rec.amt;

	if (!disp)
		return;

	sprintf (date1, "%-10.10s",DateToString (suin_rec.date_of_inv));
	sprintf (date2, "%-10.10s",DateToString (suin_rec.pay_date));

	/*
	 * Setup Invoice Header Detail.      
	 */
	if (suin_rec.exch_rate == 0.00)
		suin_rec.exch_rate = 1.0000;

	loc_bal = balance / suin_rec.exch_rate;

	sprintf (inv_hdr1,
			"%s/%s^E%s^E%s^E%13.2f^E%13.2f^E%13.2f^E%12.6f^E%12.2f^E", 
			suin_rec.est,
			suin_rec.inv_no, 
			suin_rec.approved,
			date2, 
			DOLLARS (suin_rec.amt),
			DOLLARS (balance),
			DOLLARS (suin_rec.pay_amt),
			suin_rec.exch_rate,
			DOLLARS (loc_bal));

	sprintf (inv_hdr2,
			"%s^E%s^E%s^E%s^E%s^E%s^E%s^E%s^E", 
			"                  ",
			" ",
			"          ", 
			"             ",
			"             ",
			"             ", 
			"            ",
			"            ");

	/*
	 * Process Cheque Details.           
	 */
	none = 0;
	for (i = 0; i < detailCnt; i++)
	{
		if (dtls [i].hhsiHash == suin_rec.hhsi_hash)
		{
			if (invoiceLineNum >= PSIZE)
				invoiceLineNum = 0;

			/*
			 * Print Invoice Line Details.    
			 */
			if (none == 0 || invoiceLineNum == 0)
			{
				sprintf (envLine,"%s%s^E%13.2f",
						inv_hdr1,
						cheq [dtls [i].cheq_hash].no,
						DOLLARS (dtls [i].inv_amt));
			}
			else
			{
				sprintf (envLine,"%s%s^E%13.2f",
						inv_hdr2,
						cheq [dtls [i].cheq_hash].no,
						DOLLARS (dtls [i].inv_amt));
			}
			Dsp_saverec (envLine);
			invoiceLineNum++;
			none = 1;
		}
	}
	if (none == 0)
	{
		sprintf (envLine,"%s               ^E             ", inv_hdr1);
		Dsp_saverec (envLine);
		invoiceLineNum++;
	}
}

int
DisplayCheque (void)
{
	int	j, 
		i;
	int	invoiceLineNum = 0;
	int	new_chq = 0;

	double	net = 0.0;
	char	oldCheque [16];
	char	chq_hdr1 [110];
	char	tem_line [130];
	char	envLine [140];

	t_chq = 0.00;

	printOK = TRUE;
	GetCheque ();

	/*
	 * Store Heading Details.            
	 */
	if (disp)
	{
		lpXoff = 0;
		lpYoff = 3;
		Dsp_prn_open (0, 3, PSIZE, 
		     headingString, 
		     comm_rec.co_no, comm_rec.co_name,
		     comm_rec.est_no, comm_rec.est_name,
		    (char *) 0, (char *) 0);
		Dsp_saverec ("    CHEQUE     | DATE OF  |DISCOUNT|     NET    |   CHEQUE   |     INVOICE      |    INVOICE   |   EXCH  |   INVOICE  |    EXCH   ");
		Dsp_saverec ("    NUMBER     |  CHEQUE  | AMOUNT |   CHEQUE   |   LOCAL    |      NUMBER      |    CREDIT    |   RATE  |    LOCAL   |  VARIANCE ");
		Dsp_saverec (" [REDRAW]   [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [INPUT / END] ");
	}

	/*
	 * Store Line Details.               
	 */
	invoiceLineNum = 0;
	strcpy (oldCheque, "               ");

	for (i = 0; i < detailCnt; i++)
	{
		if (invoiceLineNum >= PSIZE)
			invoiceLineNum = 0;

		j = dtls [i].cheq_hash;

		/*
		 * Print New Cheque Header Details.  
		 */
		if (strcmp (oldCheque, cheq [j].no) != 0)
		{
			new_chq = 1;
			strcpy (oldCheque, cheq [j].no);
			net = cheq [j].amount;
			t_chq += (cheq [j].amount + cheq [j].cdisc);

			if (!disp)	
				continue;

			sprintf (chq_hdr1,"%s^E%s^E%8.2f^E%12.2f^E%12.2f^E", 
					cheq [j].no, 
					cheq [j].datec,
					DOLLARS (cheq [j].cdisc), 
					DOLLARS (net), 
					DOLLARS (cheq [j].loc_amt)); 

				strcpy (tem_line,chq_hdr1);
		}
		/*
		 * Continue with same cheque.        
		 */
		else
		{
			if (!disp)	
				continue;

			if (invoiceLineNum)
			{
				sprintf (tem_line, "               ^E          ^E        ^E            ^E            ^E");
			}

			/*
			 * Reprint Cheque Header Details On New Page    
			 */
			else
			{
				strcpy (tem_line, chq_hdr1);
			}
		}
		if (FindSuin (dtls [i].hhsiHash))
		{
			strcpy (suin_rec.est, "  ");
			strcpy (suin_rec.inv_no, "               ");
		}
		/*
		 * Print Invoice Details Against Cheque.        
		 */
		sprintf (envLine, "%s%s/%s^E%14.2f^E%9.4f^E%12.2f^E%11.2f", 
		    		tem_line,
					suin_rec.est,
					suin_rec.inv_no,
		    		DOLLARS (dtls [i].inv_amt),
					dtls [i].inv_exch_rate, 
					DOLLARS (dtls [i].inv_loc_amt), 
					DOLLARS (dtls [i].inv_ex_var));
		if (disp)
		{
			Dsp_saverec (envLine);
			invoiceLineNum++;
		}
	}

	if (disp)
	{
		Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGG");
	}

	/*
	 * Display Screen.                   
	 */
	if (disp)
	{
		Dsp_srch ();
		Dsp_close ();
	}
	printOK = FALSE;

	if (disp)
		AllDisplay ();

	return (EXIT_SUCCESS);
}

int
FindSuin (
	long	hhsiHash)
{
	suin_rec.hhsi_hash	=	hhsiHash;
	return (find_rec (suin2, &suin_rec, COMPARISON, "r"));
}

int
NoteMaint (void)
{
	int	txt_cnt;

	scn_set (2);
	lcount [2] = 0;
	init_vars (2);

	sunt_rec.hhsu_hash = currentHhsuHash;
	sunt_rec.line_no = 0;
	cc = find_rec (sunt, &sunt_rec, GTEQ, "r");
	while (!cc && sunt_rec.hhsu_hash == currentHhsuHash)
	{
		sprintf (local_rec.text, "%-60.60s", sunt_rec.text);
		putval (lcount [2]++);

		cc = find_rec (sunt, &sunt_rec, NEXT, "r");
	}

	scn_display (2);

	edit (2);
	if (restart)
	{
		restart = FALSE;
		ClearReDraw ();
		return (FALSE);
	}

	for (txt_cnt = 0;txt_cnt < lcount [2] ;txt_cnt++)
	{
		getval (txt_cnt);

		sunt_rec.hhsu_hash = currentHhsuHash;
		sunt_rec.line_no = txt_cnt;
		cc = find_rec (sunt, &sunt_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (sunt_rec.text, "%-60.60s", local_rec.text);

			cc = abc_add (sunt, &sunt_rec);
			if (cc)
				file_err (cc, "sunt", "DBFIND");
		}
		else
		{
			sprintf (sunt_rec.text, "%-60.60s", local_rec.text);

			cc = abc_update (sunt, &sunt_rec);
			if (cc)
				file_err (cc, "sunt", "DBUPDATE");
		}
	}

	/*
	 * Delete old lines 
	 */
	sunt_rec.hhsu_hash = currentHhsuHash;
	sunt_rec.line_no = txt_cnt;
	cc = find_rec (sunt, &sunt_rec, GTEQ, "u");
	while (!cc && sunt_rec.hhsu_hash == currentHhsuHash)
	{
		abc_delete (sunt);

		sunt_rec.hhsu_hash = currentHhsuHash;
		sunt_rec.line_no = txt_cnt;

		cc = find_rec (sunt, &sunt_rec, GTEQ, "u");
	}
	abc_unlock (sunt);

	restart = FALSE;
	ClearReDraw ();

	return (TRUE);
}

/*
 * Read cheque routine for supplier.        
 */
int
GetCheque (void)
{
	chequeCnt = 0;
	detailCnt = 0;

	if (!ArrChkLimit (&cheq_d, cheq, chequeCnt))
		sys_err ("ArrChkLimit (cheq)", ENOMEM, PNAME);

	suhd_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		strcpy (cheq [chequeCnt].no, 		suhd_rec.cheq_no);
		strcpy (cheq [chequeCnt].bank_id, 	suhd_rec.bank_id);
		strcpy (cheq [chequeCnt].datec, DateToString (suhd_rec.date_payment));
		strcpy (cheq [chequeCnt].datep, DateToString (suhd_rec.date_post));
		cheq [chequeCnt].bank_amt	= suhd_rec.bank_amt;
		cheq [chequeCnt].bank_exch	= suhd_rec.bank_exch;
		cheq [chequeCnt].bank_chg	= suhd_rec.bank_chg;
		cheq [chequeCnt].amount 	= suhd_rec.tot_amt_paid;
		cheq [chequeCnt].cdisc 		= suhd_rec.disc_taken;
		cheq [chequeCnt].loc_amt 	= suhd_rec.loc_amt_paid;
		cheq [chequeCnt].loc_dis 	= suhd_rec.loc_disc_take;
		cheq [chequeCnt].ex_var 	= suhd_rec.exch_variance;

		sudt_rec.hhsp_hash	=	suhd_rec.hhsp_hash;
		cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
		/*
		 * Else create detail records for normal cheque. 
		 */
		while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
		{
			if (!ArrChkLimit (&dtls_d, dtls, detailCnt))
				sys_err ("ArrChkLimit (dtls)", ENOMEM, PNAME);

			dtls [detailCnt].hhsiHash 		= sudt_rec.hhsi_hash;
			dtls [detailCnt].inv_amt 		= sudt_rec.amt_paid_inv;
			dtls [detailCnt].inv_loc_amt 	= sudt_rec.loc_paid_inv;
			dtls [detailCnt].inv_ex_var 	= sudt_rec.exch_variatio;
			dtls [detailCnt].inv_exch_rate 	= sudt_rec.exch_rate;
			dtls [detailCnt].cheq_hash 		= chequeCnt;
			++detailCnt;
			cc = find_rec (sudt, &sudt_rec, NEXT, "r");
		}
		++chequeCnt;
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Master File. 
 */
int
MasterDisplay (void)
{
	int	i;

	if (masterFileOpen)
		Dsp_close ();

	if (totalOpen)
	{
		Dsp_close ();
		totalOpen = FALSE;
	}

	masterFileOpen = TRUE;

	lpXoff = 1;
	lpYoff = 3;
	Dsp_nc_prn_open (0, 1, 15, 
			headingString, 
			comm_rec.co_no, comm_rec.co_name, 
			comm_rec.est_no, comm_rec.est_name, 
			 (char *) 0, (char *) 0);

	windowCnt = 2;

	sprintf (displayString,
		" Supplier Number : %6.6s (%9.9s) %42.42s Name : %40.40s  ",
		sumr_rec.crd_no,
		sumr_rec.acronym,
		" ",
		sumr_rec.crd_name);

	Dsp_saverec (displayString);
	Dsp_saverec ("");
	Dsp_saverec ("");

	/*
	 * Lookup debtor number. 
	 */
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, db_branchNumber);
	sprintf (cumr_rec.dbt_no, "%-6.6s", sumr_rec.debtor_no);
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		sprintf (cumr_rec.dbt_name, "%-40.40s", " ");

	sprintf (displayString, 
		" Customer         : %-6.6s (%-40.40s) ",
		sumr_rec.debtor_no,
		cumr_rec.dbt_name);
	Dsp_saverec (displayString);

	/*
	 * Display Account Type. 
	 */
	sprintf (displayString, 
		" Account type     : %-12.12s %33.33s    Discount                  : %7.2f %%",
		 (sumr_rec.acc_type [0] == 'O') ? "Open Item" : "Balance B/F",
		" ",
		sumr_rec.disc);
	Dsp_saverec (displayString);

	Seperator ();

	/*
	 * Display special instructions 
	 */
	for (i = 0; i < 3; i++)
	{
		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = sumr_sic [i];
		cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
		if (cc)
			sprintf (exsi_rec.inst_text, "%-60.60s", " ");

		sprintf (displayString, 
			" Special Instruction %1d : %-60.60s",
			i + 1,
			exsi_rec.inst_text);
		Dsp_saverec (displayString);
	}

	Seperator ();

	/*
	 * Display Bank Details. 
	 */
	sprintf (displayString, 
		" Bank             : %20.20s %25.25s    Branch Code                : %20.20s",
		sumr_rec.bank, 
		" ",
		sumr_rec.bank_branch);
	Dsp_saverec (displayString);
	sprintf (displayString, 
		" Bank Code        : %15.15s %30.30s    Account Number             : %15.15s",
		sumr_rec.bank_code, 
		" ",
		sumr_rec.bank_acct_no);
	Dsp_saverec (displayString);
	sprintf (displayString, " Account Opened   : %-10.10s", DateToString (sumr_rec.date_opened));
	Dsp_saverec (displayString);

	Seperator ();
	strcpy (pocl_rec.co_no, comm_rec.co_no);
	strcpy (pocl_rec.type, sumr_rec.type_code);
	cc = find_rec ("pocl", &pocl_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocl_rec.desc, "%-40.40s", " ");

	sprintf (displayString, 
		" Supplier Type    : %12.12s (%40.40s)",
		sumr_rec.type_code, 
		pocl_rec.desc);
	Dsp_saverec (displayString);

	sprintf (displayString, 
		" Supplier Priority: (%s)",
		sup_pri [sumr_rec.sup_pri]);
	Dsp_saverec (displayString);

	Dsp_srch ();
	return (EXIT_SUCCESS);
}

void
Seperator (void)
{
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
}

void
AlternateMess (
 char *str)
{
	move (0,error_line);
	cl_line ();
	rv_pr (str,1,error_line,1);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (clearOK)
			FLD ("supplierNumber") = YES;
		else
			FLD ("supplierNumber") = ND;

		scn_set (scn);

		if (clearOK)
		{
			swide ();
			clear ();
		}

		if (scn == 2)
		{
			AllDisplay ();
			box (5,4,121,12);
		}

		if (scn == 1)
		{
			if (masterFileOpen)
			{
				Dsp_close ();
				masterFileOpen = FALSE;
			}

			if (totalOpen)
			{
				Dsp_close ();
				totalOpen = FALSE;
			}

			if (mainWindowOpen)
			{
				Dsp_close ();
				mainWindowOpen = FALSE;
			}

			move (0,0);
			crsr_off ();
			rv_pr (ML (mlPoMess042),35,0,1);

			print_at (0,108,ML (mlPoMess043),local_rec.previousSupplier);

			box (0,1,132,1);

			PrintCoStuff ();
			line_cnt = 0;
		}
		scn_write (scn);
	}

	if (displayOK && scn != 2)
	{
		AllDisplay ();
		clearOK = FALSE;
	}
    return (EXIT_SUCCESS);
}

void
PrintCoStuff (void)
{
	move (0, 22);
	line (132);
	print_at (23,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (23,52,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_short);
}

void
Dsp_Heading (void)
{
	int	main_check = mainWindowOpen;

	mainWindowOpen = FALSE;

	ReDraw ();

	if (main_check)
		Dsp_close ();

	mainWindowOpen = main_check;
}

int
HeadPrint (void)
{
	if (windowCnt == 2)
	{
		lpXoff = 1;
		lpYoff = 3;
	}
	else
	{
		lpXoff = 0;
		lpYoff = 1;
	}
	Dsp_print ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

int
DisplayTotal (void)
{
	int	i;
	double	total, total2 = 0;
	char	wk_tot [6] [14];


	crsr_off ();

	disp = FALSE;
	DisplayInvoice ();
	DisplayCheque ();

	if (totalOpen)
		Dsp_close ();

	if (masterFileOpen)
	{
		Dsp_close ();
		masterFileOpen = FALSE;
	}

	totalOpen = TRUE;

	sprintf (headingString, 
			 "Supplier %s (%s)", 
			 sumr_rec.crd_no,
			 sumr_rec.crd_name);

	Dsp_nc_prn_open (1, 3, 13, headingString, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                                     S U P P L I E R S   T O T A L S   D I S P L A Y                                          ");
	Dsp_saverec ("");
	Dsp_saverec ("");


	sprintf (displayString, 
			 "%40.40s%-28.28s %16.16s",
			 " ",
			 ML (mlPoMess044),
			 comma_fmt (DOLLARS (a_dbt), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, 
			 "%40.40s%-28.28s %16.16s",
			 " ",
			 ML (mlPoMess045),
			 comma_fmt (DOLLARS (u_dbt), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, 
			 "%40.40s%-28.28s %16.16s",
			 " ",
			 ML (mlPoMess046),
			 comma_fmt (DOLLARS (t_crd), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, 
			 "%40.40s%-28.28s %16.16s",
			 " ",
			 ML (mlPoMess047),
			 comma_fmt (DOLLARS (t_jnl), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, 
			 "%40.40s%-28.28s %16.16s",
			 " ",
			 ML (mlPoMess048),
			 comma_fmt (DOLLARS (t_chq), "N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, "%74.74s^^GGGGGGGGGGGG^^"," ");
	Dsp_saverec (displayString);

	total = (a_dbt + u_dbt + t_jnl) - (t_crd + t_chq);
	for (i = 0; i < 4; i++)
		total2 += sumr_bo_per [i];

	sprintf (displayString, 
			 "%40.40s%-10.10s                   %16.16s",
			 " ", 
			 ML (mlPoMess049),
			 comma_fmt (DOLLARS (total),"N,NNN,NNN,NNN.NN"));
	Dsp_saverec (displayString);

	sprintf (displayString, "%74.74s^^GGGGGGGGGGGG^^"," ");
	Dsp_saverec (displayString);


	sprintf (displayString,"%25.25s^^AGGGGGGGGGGGGGIGGGGGGGGGGGGGGIGGGGGGGGGGGGGGIGGGGGGGGGGGGGGIGGGGGGGGGGGGGGB^^"," ");
	Dsp_saverec (displayString);

	sprintf 
	(
		displayString, 
		 "%25.25s^E%-13.13s^E    31 - 60   ^E    61 - 90   ^E      90+     ^E%14.14s^E",
		 " ",
		 ML ("CURRENT"),
		 ML ("TOTAL AMOUNT.")
	 );
	Dsp_saverec (displayString);


	sprintf (displayString,"%25.25s^^KGGGGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGGL^^"," ");
	Dsp_saverec (displayString);

	strcpy (wk_tot [0], comma_fmt (DOLLARS (sumr_bo_per [0]), "NNNN,NNN.NN"));
	strcpy (wk_tot [1], comma_fmt (DOLLARS (sumr_bo_per [1]), "NNNN,NNN.NN"));
	strcpy (wk_tot [2], comma_fmt (DOLLARS (sumr_bo_per [2]), "NNNN,NNN.NN"));
	strcpy (wk_tot [3], comma_fmt (DOLLARS (sumr_bo_per [3]), "NNNN,NNN.NN"));
	strcpy (wk_tot [4], comma_fmt (DOLLARS (total2), "NNNN,NNN.NN"));

	sprintf (displayString,"%25.25s^E %11.11s ^E  %11.11s ^E  %11.11s ^E  %11.11s ^E  %11.11s ^E",
			" ",
			wk_tot [0], wk_tot [1], wk_tot [2],
			wk_tot [3], wk_tot [4]);

	Dsp_saverec (displayString);

	sprintf (displayString,"%25.25s^^CGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGD^^"," ");
	Dsp_saverec (displayString);
	Dsp_srch ();

	disp = TRUE;

	return (EXIT_SUCCESS);
}

/*
 * Display Supplier pricing. 
 */
int
SupplierPricing (void)
{
	int		i,
			j;
	int		qty_fnd = FALSE;

	lpXoff = 0;
	lpYoff = 3;
	Dsp_prn_open (1, 3, PSIZE, 
		     headingString, 
		     comm_rec.co_no, comm_rec.co_name,
		     comm_rec.est_no, comm_rec.est_name,
		    (char *) 0, (char *) 0);

	Dsp_saverec ("    ITEM NUMBER   |  SUPPLIER  PART  |             ITEM DESCRIPTION           |PRI|  LAST COST |  QUANTITY  | SUPPLIER PRICE ");

	Dsp_saverec ("                  |      NUMBER      |                                        |   |    DATE    |   BREAK    |   FOB (FGN)    ");

	Dsp_saverec (" [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [INPUT / END] ");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	inis_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	while (!cc && inis_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		inmr_rec.hhbr_hash	=	inis_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%-16.16s%-16.16s%-40.40s",
			inmr_rec.item_no, 
			inis_rec.sup_part,
			inmr_rec.description
		);
		
		strcpy (sortRec [sortCnt].itemNo,		inmr_rec.item_no);
		strcpy (sortRec [sortCnt].supItemNo,	inis_rec.sup_part);
		strcpy (sortRec [sortCnt].itemDesc,  	inmr_rec.description);
		strcpy (sortRec [sortCnt].supPriority,  inis_rec.sup_priority);
		sortRec [sortCnt].lcostDate = inis_rec.lcost_date;
		sortRec [sortCnt].fobCost 	= inis_rec.fob_cost;

		sortRec [sortCnt].hhsuHash 	= inis_rec.hhsu_hash;
		sortRec [sortCnt].hhbrHash 	= inis_rec.hhbr_hash;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		cc = find_rec (inis, &inis_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf 
		(
			displayString, 
			" %16.16s ^E %16.16s ^E%40.40s^E %1.1s ^E %10.10s ^E     Base   ^E%15.15s ",
			sortRec [i].itemNo,	
			sortRec [i].supItemNo,
			sortRec [i].itemDesc, 
			sortRec [i].supPriority, 
			DateToString (sortRec [i].lcostDate),
			comma_fmt (sortRec [i].fobCost,"NNNNNN,NNN.NNNN")
		);

		Dsp_saverec (displayString);
		
	 	inis_rec.hhsu_hash = sortRec [i].hhsuHash;
		inis_rec.hhbr_hash = sortRec [i].hhbrHash;
		
		qty_fnd = FALSE;

		insp_rec.hhsu_hash = inis_rec.hhsu_hash;
		insp_rec.hhbr_hash = inis_rec.hhbr_hash;
		cc = find_rec (insp, &insp_rec, GTEQ, "r");
		while (!cc && inis_rec.hhsu_hash == insp_rec.hhsu_hash &&
					   inis_rec.hhbr_hash == insp_rec.hhbr_hash)
		{
			for (j = 0; j < 5; j++)
			{
				if (insp_qty_brk [j] == 0.0)
					continue;

				sprintf (displayString, "                  ^E                  ^E%40.40s^E   ^E ---------> ^E%11.2f ^E%15.15s ",
						" ",
					insp_qty_brk [j], 
					comma_fmt (insp_price [j],"NNNNNN,NNN.NNNN"));

				Dsp_saverec (displayString);
				qty_fnd = TRUE;
			}
			cc = find_rec (insp, &insp_rec, NEXT, "r");
	    }
		if (qty_fnd)
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGEGGGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGGGGGGG");
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	Dsp_srch ();
	Dsp_close ();

	ReDraw ();
    return (EXIT_SUCCESS);
}

/*
 * Display Supplier Discounting. 
 */
int
SupplierDiscounts (void)
{
	int		i,
			rec_fnd = 0;

	lpXoff = 0;
	lpYoff = 3;
	Dsp_prn_open (3, 3, PSIZE, headingString, comm_rec.co_no, comm_rec.co_name,
		     							    comm_rec.est_no,comm_rec.est_name,
		     								 (char *) 0, (char *) 0);

	Dsp_saverec (" BUYER  |         BUYER GROUP DESCRIPTION.         | ANTICIPATED | REGULATORY | QTY BREAK | DISC  A  | DISC  B  | DISC  C  ");
	Dsp_saverec (" GROUP  |                                          |   DISC  %   |     %      |           |    %     |    %     |    %     ");
	Dsp_saverec (" [PRINT]   [NEXT SCREEN]   [PREV SCREEN]   [INPUT / END] ");

	suds_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suds_rec.buy_group, "      ");
	cc = find_rec (suds, &suds_rec, GTEQ, "r");
	while (!cc && suds_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.code,  suds_rec.buy_group);
		strcpy (ingp_rec.type,  "B");
		cc = find_rec ("ingp", &ingp_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (suds, &suds_rec, NEXT, "r");
			continue;
		}
		rec_fnd++;

		sprintf (displayString, " %6.6s ^E %40.40s ^E%11.2f%% ^E%10.2f%% ^E%10.2f ^E%8.2f%% ^E%8.2f%% ^E%8.2f%% ",
				suds_rec.buy_group,
				ingp_rec.desc,
				ScreenDisc (suds_rec.anticipated),
				ScreenDisc (suds_rec.reg_pc),
				suds_qty_brk [0],
				ScreenDisc (suds_disca_pc [0]),
				ScreenDisc (suds_discb_pc [0]),
				ScreenDisc (suds_discc_pc [0]));

		Dsp_saverec (displayString);
			
		for (i = 1; i < 6; i++)
		{
			sprintf (displayString, "        ^E %40.40s ^E             ^E            ^E%10.2f ^E%8.2f%% ^E%8.2f%% ^E%8.2f%% ",
				" ",
				suds_qty_brk [i],
				ScreenDisc (suds_disca_pc [i]),
				ScreenDisc (suds_discb_pc [i]),
				ScreenDisc (suds_discc_pc [i]));

			if (suds_qty_brk [i] != 0.00)
				Dsp_saverec (displayString);
		}
		Dsp_saverec ("^^GGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGG");
		cc = find_rec (suds, &suds_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();

	ReDraw ();
    return (EXIT_SUCCESS);
}

int
CalcCheque (void)
{
	int		j = 0;
	int		i = 0;
	char	oldCheque [16];
	double	net = 0.0;

	t_chq = 0.00;

	strcpy (oldCheque,"               ");

	for (i = 0;i < detailCnt;i++)
	{
		j = dtls [i].cheq_hash;

		if (cheq [j].amount == 0)
			continue;

		if (strcmp (oldCheque, cheq [j].no) != 0)
		{
			strcpy (oldCheque,cheq [j].no);
			net = cheq [j].amount - cheq [j].cdisc;

			if (!strcmp (cheq [j].app,"APP") &&
			     cheq [j].type [0] == '2')
			{
				t_inv -= cheq [j].amount;
				continue;
			}
			if (cheq [j].type [0] == '1')
				t_chq += cheq [j].amount;
			else
				t_jnl -= cheq [j].amount;
		}
	}
	return (EXIT_SUCCESS);
}


/*
 * Quality Assurance Text input. 
 */
int
QA_display (void)
{
	int		i,
			last_line	=	0;

	int		tx_window,
			tx_lines;

	char	displayString [61];

	tx_window = txt_open (8, 10, 10, 60, 300,
			"  Q U A L I T Y   A S S U R A N C E   S C O P E   T E X T.  ");

	open_rec (qasd,  qasd_list, QASD_NO_FIELDS, "qasd_id_no");

	qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	qasd_rec.line_no   = 0;
	cc = find_rec (qasd, &qasd_rec, GTEQ, "r");

	while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sprintf (displayString, "%-60.60s", qasd_rec.desc);

		txt_pval (tx_window, displayString, 0);

		cc = find_rec (qasd, &qasd_rec, NEXT, "r");
	}

	tx_lines = txt_edit (tx_window);
	if (!tx_lines)
	{
		txt_close (tx_window, FALSE);
		qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		qasd_rec.line_no   = 0;
		cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
		while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			abc_delete (qasd);
			cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
		}
		ReDraw ();
		return (EXIT_SUCCESS);
	}
	for (i = 1; i <= tx_lines; i++)
	{
		last_line = i;

		qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		qasd_rec.line_no   = i -1;
		cc = find_rec (qasd, &qasd_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (qasd_rec.desc, "%-60.60s", txt_gval (tx_window, i));

			cc = abc_add (qasd, &qasd_rec);
			if (cc)
				file_err (cc, "qasd", "DBADD");
		}
		else
		{
			sprintf (qasd_rec.desc, "%-60.60s", txt_gval (tx_window, i));

			cc = abc_update (qasd, &qasd_rec);
			if (cc)
				file_err (cc, "qasd", "DBUPDATE");
		}
	}
	txt_close (tx_window, FALSE);

	qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	qasd_rec.line_no = last_line;
	cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
	while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		abc_delete (qasd);
		cc = find_rec (qasd, &qasd_rec, GTEQ, "r");
	}
	abc_fclose (qasd);
	ReDraw ();
	return (EXIT_SUCCESS);
}

/*
 * Display cheque History Information. 
 */
int
ChequeExchHistory (void)
{
	int		i = 0;
	int		invoiceLineNum = 0;
	char	oldCheque [16];

	lpXoff = 4;
	lpYoff = 3;

	Dsp_prn_open (0, 3, PSIZE, headingString, 
							comm_rec.co_no, comm_rec.co_name, 
							comm_rec.est_no, comm_rec.est_name, 
							 (char *) 0, (char *) 0);

	Dsp_saverec ("    CHEQUE     | DATE  OF |BANK | DISCOUNT| BANK RECEIPT  |   BANK    |  LOCAL BANK   |RECEIPT AMOUNT |   EXCH    |RECEIPT AMOUNT ");

	sprintf (err_str,"    NUMBER     |  CHEQUE  |CODE |  AMOUNT |    AMOUNT     | EXCHANGE  |  RECEIPT AMT  |  AMOUNT (%3.3s) |   RATE    |    LOCAL      ",sumr_rec.curr_code);
	Dsp_saverec (err_str);
	Dsp_saverec (" [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END]");

	if (!MCURR)
	{
		Dsp_saverec (" ");
		Dsp_saverec ("Option is only available for multi currency module.");
		Dsp_saverec (" ");
		Dsp_srch ();
		Dsp_close ();
		ReDraw ();
		return (EXIT_SUCCESS);
	}
	strcpy (oldCheque,"      ");

	for (i = 0;i < detailCnt;i++)
		GetExchCheques (i, &invoiceLineNum, oldCheque);

	Dsp_srch ();
	Dsp_close ();
	ReDraw ();
	return (EXIT_SUCCESS);
}

/*
 * Get cheques. 
 */
void
GetExchCheques (
	int 	i, 
	int 	*invoiceLineNum, 
	char 	*oldCheque)
{
	char	fm_str [6] [15];
	int		j;
	char	envLine [300];
	double	net = 0.0;
	double	ChequeExch;

	if (*invoiceLineNum >= PSIZE)
		*invoiceLineNum = *invoiceLineNum % PSIZE;

	j = dtls [i].cheq_hash;

	if (cheq [j].amount == 0)
		return;

	if (strcmp (oldCheque, cheq [j].no) != 0)
	{
		strcpy (oldCheque,cheq [j].no);
		net = cheq [j].amount + cheq [j].cdisc;

		strcpy (fm_str [0], 
				comma_fmt (DOLLARS (cheq [j].bank_amt), "NNN,NNN,NNN.NN"));
		if (cheq [j].bank_exch)
			strcpy (fm_str [1], 
				comma_fmt (DOLLARS (cheq [j].bank_amt / cheq [j].bank_exch),
													"NNN,NNN,NNN.NN"));
		else
			strcpy (fm_str [1],"           N/A");

		strcpy (fm_str [2], comma_fmt (DOLLARS (net), "NNN,NNN,NNN.NN"));
		strcpy (fm_str [3], comma_fmt (DOLLARS (cheq [j].loc_amt),
													"NNN,NNN,NNN.NN"));
		strcpy (fm_str [4], comma_fmt (DOLLARS (cheq [j].cdisc),"NN,NNN.NN"));
		
		if (net)
		{
			ChequeExch = cheq [j].loc_amt / net;
			strcpy (fm_str [5], comma_fmt (ChequeExch,"NNNNN.NNNNN"));
		}
		else
			strcpy (fm_str [5],"          N/A");
	
		sprintf (envLine,"%s^E%s^E%s^E%-9.9s ^E%-14.14s^E%11.5f^E%-14.14s ^E%-14.14s ^E%-11.11s^E%-14.14s ",
				cheq [j].no,
				cheq [j].datec,
				cheq [j].bank_id,
				fm_str [4], 
				fm_str [0], 
				cheq [j].bank_exch,
				fm_str [1], 
				fm_str [2], 
				fm_str [5],
				fm_str [3]);
		Dsp_saverec (envLine);
	}
	*invoiceLineNum += 1;
}
/*
 * Reverse Screen Discount. 
 */
float	
ScreenDisc (
 float	DiscountPercent)
{
	if (envVarSoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
