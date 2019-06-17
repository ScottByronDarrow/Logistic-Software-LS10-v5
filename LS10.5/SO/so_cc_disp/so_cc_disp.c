/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_cc_disp.c,v 5.17 2002/11/28 04:09:49 scott Exp $
|  Program Name  : (so_cc_disp.c )                                    |
|  Program Desc  : (Credit Control Display.                    )      |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.     Date Written  : 01/07/92         |
|---------------------------------------------------------------------|
| $Log: so_cc_disp.c,v $
| Revision 5.17  2002/11/28 04:09:49  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.16  2002/07/24 08:39:21  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.15  2002/07/10 08:25:24  robert
| S/C 4113 - fixed display issue
|
| Revision 5.14  2002/06/25 08:44:50  scott
| Added missing ArrDelete
|
| Revision 5.13  2002/06/20 07:15:50  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.12  2002/06/20 05:48:48  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_cc_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cc_disp/so_cc_disp.c,v 5.17 2002/11/28 04:09:49 scott Exp $";

#define	 TXT_REQD
#define	MAXLINES		4000
#include <pslscr.h>		/*  Gen. C Screen Handler Header          */
#include <arralloc.h>
#include <twodec.h>
#include <ring_menu.h>
#include <graph.h>
#include <errno.h>

#include <ml_std_mess.h>
#include <ml_ts_mess.h>

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

#define	InternalPageSize	12	/* Max lines in a page on screen	 */

#define	COMPLAINT	'C'
#define	NOTES		'N'
#define	LST_CALL	'L'
#define	NEXT_VISIT	'V'

#define	CF(v,m)			comma_fmt(DOLLARS (v), m)
#define	INV_DISP	(processType [0] == 'I')
#define	TRAN_DISP	(processType [0] == 'T')
#define	INVOICE		(cuin_rec.type [0] == '1')
#define	CREDIT		(cuin_rec.type [0] == '2')

#define	SORT_T		(local_rec.transactionSort [0] == 'T')
#define	SORT_C		(local_rec.transactionSort [0] == 'C')

#define	HAS_HO		(cumr_rec.ho_dbt_hash != 0L)

char		*HEADER    = "                |                                        | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*ITEM_HEAD = "  ITEM NUMBER   |         ITEM     DESCRIPTION           |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*SEPARATION = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char		*UNDERLINE = "===============================================================================================================================";

FILE	*fsort;



#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct sumrRecord	sumr_rec;
struct tspmRecord	tspm_rec;
struct tmpfRecord	tmpf_rec;
struct tsxdRecord	tsxd_rec;
struct cusaRecord	cusa_rec;
struct cudiRecord	cudi_rec;
struct cuhdRecord	cuhd_rec;
struct cuinRecord	cuin_rec;
struct cudtRecord	cudt_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct exafRecord	exaf_rec;
struct exsfRecord	exsf_rec;
struct exclRecord	excl_rec;
struct exdfRecord	exdf_rec;
struct dbryRecord	dbry_rec;
struct cuccRecord	cucc_rec;
struct incpRecord	incp_rec;
struct cuphRecord	cuph_rec;
struct sadfRecord	sadf_rec;
struct cohrRecord	cohr_rec;
struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct colnRecord	coln_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocr_rec;
struct tshsRecord	tshs_rec;
struct exmsRecord	exms_rec;
struct exmdRecord	exmd_rec;
struct exmhRecord	exmh_rec;
struct incsRecord	incs_rec;
struct exmmRecord	exmm_rec;
struct sacdRecord	sacd_rec;
struct sacaRecord	saca_rec;
struct exmaRecord	exma_rec;

	Money	*cumr_bo_per	=	&cumr_rec.bo_current;
	Money	*cumr2_bo_per	=	&cumr2_rec.bo_current;
	Money	*cusa_val		=	&cusa_rec.val1;
	Money	*incp_price		=	&incp_rec.price1;
	float	*sadf_qty_per	=	&sadf_rec.qty_per1;
	double	*sadf_sal_per	=	&sadf_rec.sal_per1;
	double	*sadf_cst_per	=	&sadf_rec.cst_per1;

	char	*data  = "data",
			*cumr2 = "cumr2",
			*inmr2 = "inmr2";

	struct storeRec {
		long	recordHash;
	} store [MAXLINES];

    char    *month_nm [12];

char	*std_foot = " [PRINT] [NEXT] [PREVIOUS] [END/INPUT]";
extern		int		lp_x_off,
					lp_y_off;

	int		envVarRepTax 		= FALSE,
			envVarDbMcurr 		= FALSE,
			envVarDbTotalAge	= FALSE,
			envVarDbDaysAgeing	= FALSE,
			envVarDbCo 			= 0,
			envVarDbFind 		= 0,
			envVarDbNettUsed 	= TRUE,
			envVarCnNettUsed 	= TRUE,
			envVarTsInstalled	= FALSE,
			window_cnt 			= 0,
			main_open 			= FALSE,
			masterfile_open 	= FALSE,
			total_open 			= FALSE,
			firstTime 			= TRUE,
			orderDisplay		= FALSE,
			displayOK			= 0,
			linePrinted 		= FALSE,
			clearOK 			= TRUE,
			localValue			= FALSE,
			invoiceDueDate 		= TRUE,
			trueAgeing			= FALSE,
			currentMonth		= 0,
			fiscal				= 0,
			saveIndex			= 0,
			mdy [3]				= {0,0,0};

	double	mtd_sales 		= 	0.00,
			ytd_sales 		= 	0.00,
			last_12mth 		= 	0.00,
			fgnTotInvoice	= 	0.00,
			fgnTotCredit	= 	0.00,
			fgnTotJournal	= 	0.00,
			fgnTotCheque	= 	0.00,
			fgnTotFwdChq	= 	0.00,
			locTotInvoice	= 	0.00,
			locTotCredit	= 	0.00,
			locTotJournal	= 	0.00,
			locTotCheque	= 	0.00,
			locTotFwdChq	= 	0.00,
			cy_sales [12]	=	{0,0,0,0,0,0,0,0,0,0,0,0},
			ly_sales [12]	=	{0,0,0,0,0,0,0,0,0,0,0,0},
			m_sales [2]		=	{0,0}, 
			y_sales [2]		=	{0,0},
			m_csale [2]		=	{0,0}, 
			y_csale [2]		=	{0,0},
			mth_total [4]	=	{0,0,0,0},
			cus_total [4]	=	{0,0,0,0},
			grd_total [4]	=	{0,0,0,0},
			ord_amt 		= 	0.00,
			tot_amt 		= 	0.00,
			wk_bal 			= 	0.00,
			fgnTotal [6] 	= 	{0, 0, 0, 0, 0, 0},
			locTotal [6] 	= 	{0, 0, 0, 0, 0, 0};

	char	pay_date [11],
			disp_str [300],
			branchNo [3],
			date_ord [11],
			date_due [11],
			fgnFmt [4] [15],
			locFmt [4] [15],
			mlCcDisp [121] [101],
			prod_str [200],
			currentItem [17],
			previousItem [17],
			itemDesc [41],
			processType [2],
			head_str [200],
			save_key [21],
			envVarCurrCode [4];

	float	ord_qty,
			tot_qty,
			m_qty [2], 
			y_qty [2];


	long	monthEndDate 	= 	0L,
			curr_hhcu	=	0L;


/*
 * Callback Functions. 
 */
static 	int IsZero 				(double);
float 	CalculateYtd 			(void);
int 	DisplayInvoice 			(int, long, char *);
int 	Dsp_heading 			(void);
int 	FindCuin 				(long);
int 	LoadDetails 			(void);
int 	MoneyZero 				(double);
int 	PayPer 					(char *, long, long);
int 	heading 				(int);
int  	BackOrderDisplay 		(void);
int  	CallNotes 				(char *, char *);
int  	ChequeDisp 				(void);
int  	ChequeExchHistory 		(void);
int  	ChequeHistory 			(void);
int  	ClearRedraw 			(void);
int  	CollectionNoteDisplay 	(void);
int  	ColnCCN 				(int);
int  	CustomerGroupDisplay 	(void);
int  	DdGetHeader 			(void);
int  	DdGetLines 				(int);
int  	DdInvoiceDisplay 		(void);
int  	DeliveryAddressDisplay 	(void);
int  	DirectDeliveryDisplay 	(void);
int  	FindColn 				(void);
int  	FindPackingSlip 		(long);
int  	FindPohr 				(long);
int  	FindSoln 				(int);
int  	GetExchCheques 			(int, int *, char *);
int  	GetInvoices 			(int, int *, char *, char *, double *, char *);
int  	GraphPayments 			(void);
int  	GraphThisLast 			(void);
int  	HeaderPrint 			(void);
int  	HistoryDisplay 			(void);
int  	InvoiceDisplay 			(void);
int  	InvoiceEnquiry 			(char *);
int  	InvoiceSetup 			(void);
int  	ItemGraph 				(char *);
int  	LastPrice 				(void);
int  	LedgerTotalDisplay 		(void);
int  	MainDisplayScreen 		(void);
int  	MarketInfo 				(void);
int  	MasterDisplay 			(void);
int  	MerchInfo 				(void);
int  	NextDisplay 			(void);
int  	NotesAdd 				(void);
int  	NotesDisplay 			(void);
int  	OrderDisplay 			(void);
int  	PreviousDisplay 		(void);
int  	ProcessSolnLines 		(int,int);
int  	RepCalls 				(void);
int  	SalesAnalysisDisplay 	(void);
int  	SpecialPrice 			(void);
int  	TransactionDisplay 		(void);
int  	TsComplaints 			(void);
int  	TsInfo 					(void);
int  	TsLastCall 				(void);
int  	TsNextVisit 			(void);
int  	TsNotes 				(void);
int  	spec_valid 				(int);
int		CustIdSort		 		(const	void *,	const void *);
int 	LocalToFgn 				(void);
int 	InvoiceDateDue			(void);
void	UpdateNotes 			(void);
void 	AlternateMess 			(char *);
void 	CalcBalance 			(int);
void 	CalcDdBalance 			(void);
void 	CalcInvoiceBalance 		(void);
void 	CalculateInvoice 		(int, long);
void 	CalculateMtd 			(void);
void 	ChangedData 			(int);
void 	ChequeHistDetails 		(long);
void 	CloseDB 				(void);
void 	DeleteMonthNames 		(void);
void 	DisplayGraphKeys 		(void);
void 	DspPayment 				(long);
void 	GetChequeData 			(int, long);
void 	GetCheques 				(int, int *, char *);
void 	GetCustomerStuff 		(int, long);
void 	GetDetails 				(void);
void 	InitML 					(void);
void 	InitMonthNames 			(void);
void 	MarketingHeader 		(void);
void 	NoteDisplay 			(long);
void 	OpenDB 					(void);
void 	OrderProcess 			(int);
void 	PrintGrandTotals 		(int);
void 	PrintTotal 				(char *);
void 	ProcessInvoices 		(void);
void 	ProcessSales 			(void);
void 	Redraw 					(void);
void 	RunSpecialDisplay 		(char *, char *, char *, long, char *, int);
void 	SetMonthName 			(int, char*);
void 	TotalCustomer 			(char *);
void 	TotalGrand 				(void);
void 	TotalMonth 				(int);
void 	shutdown_prog 			(void);

#ifndef GVISION
menu_type	_main_menu [] = {
{ "<Credit Control Details.>","Display Credit Control Screen. [C]",MainDisplayScreen,"Cc", },
{ "<Master File Details.>","Display Customers Master File. [M]",MasterDisplay,"Mm", },
{ "<Transactions.>","Display Customer Invoice Ledger.", TransactionDisplay, "Tt",  },
{ "<Invoice Trans.>","Display Customers Invoices. [T]",InvoiceDisplay,"Tt",  },
{ "<Groups.>","Display Customer Grouping. [G]",CustomerGroupDisplay,"Gg",  },
{ "<Cheques.>" ,"Display Customers Full Ledger (Cheques). [C]",ChequeDisp,"Cc",  },
{ "<Cheque Details.>" ,"Display Cheque Exchange rate details. [C]",ChequeExchHistory,"Cc",  },
{ "<Cheque History.>","Cheque History. [C]", ChequeHistory, "Cc",	     },
{ "<Invoice D-D.>","Display Direct Delivery Invoices. [I]",DdInvoiceDisplay,"IiDd",  },
{ "<B/Orders.>","Display Customer Backorders. [B]", BackOrderDisplay,"Bb",     },
{ "<Orders.>","Display Customer Orders. [O]", OrderDisplay,"Oo",            },
{ "<D-D Orders.>","Display Direct Delivery Orders. [D]", DirectDeliveryDisplay,"OoDd",            },
{ "<CCN's>","Customer Collection Notes. [C]", CollectionNoteDisplay,"Cc",            },
{ "<Note Pad.>","Display Customer Note Pad. [N]", NotesDisplay, "Nn",	      },
{ "<Add to Note Pad.>","Add/Maintain Customer Note Pad. [A]", NotesAdd,"Aa",},
{ "<Delivery Address (s).>","Delivery Addresses. [D]", DeliveryAddressDisplay, "Dd",  },
{ "<Specific Customer Pricing.>","Display Customers Specific Prices. [S] or [P]",SpecialPrice,"SsPp", },
{ "<Last Customer Pricing.>","Display Last Customers Prices. [L] or [P]",LastPrice,"LlPp", },
{ "<Graph-This/Last year>","Graphical Display of This years versus last years sales value. [G]",GraphThisLast,"Gg", },
{ "<Graph-Payments/Invoice.>","Graphical Display of Payments/Invoice. [G] or [P]",GraphPayments,"GgPp", }, 
{ "<Sales History>","Display Sales History. [H]",HistoryDisplay,"Hh", },
{ "<Sales Analysis>","Display Sales Analysis. [A]",SalesAnalysisDisplay,"Aa", },
{ "<Market Info.>","Display Market Information. [M]", MarketInfo, "Mm",  },
{ "<Merchandiser Info.>","Display Merchandiser Information. [M]", MerchInfo, "Mm",  },
{ "<Rep Calls.>","Display Rep Call information. [R]", RepCalls, "Rr",  },
{ "<Telesales Info>","Telesales Contact Information",TsInfo,"Tt",},
{ "<Telesales Complaints>","Maintain Telesales Complaints",TsComplaints,"Tt",},
{ "<Telesales Notes>","Maintain Telesales Notes", TsNotes, "Tt",  },
{ "<Telesales Last Call Notes>","Maintain Telesales Last Call Notes",TsLastCall,"Tt",},
{ "<Telesales Next Call Notes>","Maintain Telesales Next Call Notes",TsNextVisit,"Tt",},
{ "<Total Screen.>","Display Customer totals. [T]",LedgerTotalDisplay,"Tt", 	},
{ "<Switch Local/Foreign values.>","Switch between display in local or foreign currency [S]",LocalToFgn,"Ss",}, 	
{ "<Switch Inv/Due Date Ageing>","Switch between Invoice/Due Date Ageing [S]",InvoiceDateDue,"Ss", },
{ " [REDRAW]", 	"Redraw Display", 		ClearRedraw, 	  "", FN3,		     },
{ " [PRINT]", 	"Print Screen", 		HeaderPrint,   "", FN5,		     },
{ " [NEXT]", 	"Display Next Item", 	NextDisplay, "", FN14,		     },
{ " [PREVIOUS]", "Display Previous Item", 	PreviousDisplay, "", FN15,		     },
{ " [END/INPUT]","Exit Display", 		_no_option,   "", FN16, EXIT | SELECT},
{ "",									     },
};
#else
menu_group _main_group [] = {
	{1, "Display"},
	{2, "Orders"},
	{3, "Notes"},
	{4, "Pricing"},
	{5, "Graphs"},
	{6, "Telesales"},
	{0, ""}
};
 
menu_type	_main_menu [] = {
{1, "<Credit Control Details.>","Credit control notes",MainDisplayScreen, },
{1, "<Master File Details.>","Master file details",MasterDisplay, },
{1, "<Transactions.>","Transaction display", TransactionDisplay, },
{1, "<Invoice Trans.>","Invoice transaction Display",InvoiceDisplay, },
{1, "<Groups.>","Invoice transactions by group",CustomerGroupDisplay, },
{1, "<Cheques.>" ,"Cheque transaction display",ChequeDisp, },
{1, "<Cheque Details.>" ,"Cheque detail display",ChequeExchHistory, },
{2, "<Cheque History.>","Cheque history display", ChequeHistory, },
{2, "<Invoice D-D.>","Invoice display - direct deliveries",DdInvoiceDisplay, },
{2, "<B/Orders.>","Backorder display", BackOrderDisplay, },
{2, "<Orders.>","Orders display", OrderDisplay, },
{2, "<D-D Orders.>","Direct delivery order display", DirectDeliveryDisplay, },
{3, "<CCN's>","Customer collection note display", CollectionNoteDisplay, },
{3, "<Note Pad.>","Note pad display", NotesDisplay, },
{3, "<Add to Note Pad.>","Note pad maintenance", NotesAdd,},
{3, "<Delivery Address (s).>","Delivery address(s)", DeliveryAddressDisplay,  },
{4, "<Specific Customer Pricing.>","Customer specific pricing",SpecialPrice, },
{4, "<Last Customer Pricing.>","Last customer pricing",LastPrice, },
{5, "<Graph-This/Last year>","Graph this / last year sales",GraphThisLast, },
{5, "<Graph-Payments/Invoice.>","Graph payments / invoices",GraphPayments, }, 
{5, "<Sales History>","Customer sales history",HistoryDisplay, },
{5, "<Sales Analysis>","Customer sales analysis",SalesAnalysisDisplay, },
{5, "<Market Info.>","Market information", MarketInfo, },
{5, "<Merchandiser Info.>","Merchandiser information", MerchInfo, },
{5, "<Rep Calls.>","Sales representative Calls", RepCalls, },
{6, "<Telesales Info>","Telesales information.",TsInfo, },
{6, "<Telesales Complaints>","Telesales Complaints.",TsComplaints, },
{6, "<Telesales Notes>","Telesales Notes.", TsNotes, },
{6, "<Telesales Last Call Notes>","Telesales last call notes.",TsLastCall,},
{6, "<Telesales Next Call Notes>","Telesales next call notes.",TsNextVisit,},
{0, "<Total Screen.>","Customer total screen",LedgerTotalDisplay, 	},
{0, "<Switch Local/Foreign values.>","Switch between local and foreign values", LocalToFgn, },
{0, "<Switch Inv/Due Date Ageing>","Switch between Invoice/Due Date ageing.", InvoiceDateDue, },
{0, " [REDRAW]", 	"Redraw Display", 		ClearRedraw, FN3,		     },
{0, " [PRINT]", 	"Print Screen", 		HeaderPrint, FN5,		     },
{0, " [NEXT]", 	"Next Item", 	NextDisplay, FN14,		     },
{0, " [PREVIOUS]", "Previous Item", 	PreviousDisplay, FN15,		     },
{0, "",									     },
};
#endif

/*
 * The structures 'cheq'&'dtls' are initialised in function 'GetChequeData' 
 * the number of cheques is stored in external variable 'cheq_cnt'.    
 * the number of details is stored in external variable 'dtls_cnt'.   
 */
static	struct	Cheque 
{ 							/*------------------------------*/
	char	no [9];			/*| reciept number.	    		|*/
	char	cheque [21];	/*| cheque number.	    		|*/
	char	OR [11];		/*| OR number.	    			|*/
	char	alt_drawer [21];/*| Alternate Drawer.	    	|*/
	char	type [2];		/*| Cheque Type.     	    	|*/
	char	app [4];		/*| Transaction Type.     	    |*/
	char	bank_id [6];	/*| Bank ID.              	    |*/
	double	bank_amt;		/*| Bank Receipt Amount.		|*/
	double	bank_exch;		/*| Bank Exchange Rate.			|*/
	double	bank_chg;		/*| Bank Charge Amount.			|*/
	char	datec [11];		/*| date of payment.      	    |*/
	char	datep [11];		/*| date posted.                |*/
	char	dated [11];		/*| Due Date posted.            |*/
	int		status;			/*| Cheque Status.              |*/
	double	amount;			/*| total amount of cheque.	    |*/
	double	cdisc;			/*| discount given.		    	|*/
	double	loc_amt;		/*| total local amount of cheque|*/
	double	loc_dis;		/*| total local discount.       |*/
	double	ex_var;			/*| Exchange variation.  	    |*/
} *cheq;         	        /*|                             |*/	
                            /*-------------------------------*/
static DArray cheq_d;
static int	cheq_cnt;

static struct Detail
{ 							/*-------------------------------*/
	long	hhci_hash;		/*| detail invoice reference.   |*/
	int		fwd_payment;	/*| detail forward payments.    |*/
	char	type [2];		/*| Transaction Type.     	    |*/
	char	app [4];		/*| Transaction Type.     	    |*/
	double	inv_fgn_amt;	/*| detail invoice amount.      |*/
	double	inv_loc_amt;	/*| detail local invoice amount.|*/
	double	inv_ex_var;		/*| exchange local variance.    |*/
	double	inv_exch_rate;	/*| detail exchange rate.       |*/
	int		cheq_hash;		/*| cheq structure pointer.     |*/
} *dtls;					/*-------------------------------*/

static DArray dtls_d;
static int	dtls_cnt;

	static	char	*tran_type []={
		"IN","CR","JL","CH","JL","?"
	};

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char 	dbt_no [7];
	char 	prev_dbt [7];
	char	dbt_date [11];
	char	text [61];
	long	start_date;
	long	end_date;
	long	lsystemDate;
	char	transactionSort [2];
	char	CallPrompt [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "dbt_no",	 2, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer Number  :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.dbt_no},

	{2, TAB, "comment",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                              C  O  M  M  E  N  T                               ", " ",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.comment},
	{2, TAB, "contact",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "  Contact Person.   ", " ",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.con_person},
	{2, TAB, "cont_date",	 0, 0, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.dbt_date, "Cont Date", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cucc_rec.cont_date},
	{2, TAB, "hold_ref",	 0, 0, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "  Ref.  ", "Input Invoice/Credit or Journal Reference.",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.hold_ref},
	{3, TXT, "complaints",12,1,CHARTYPE,
		"","          ",
		" ","", local_rec.CallPrompt,"",
		5, 60, 16, "", "", local_rec.text},
	{4, LIN, "st_date",	 12, 62, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Start Date     :", "Enter Date for Starting Transaction - Default is start ",
		ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.start_date},
	{4, LIN, "end_date",	 13, 62, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End   Date     :", "Enter Date for Ending Transaction - Default is Today ",
		ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.end_date},
	{4, LIN, "transactionSort",	 14, 62, CHARTYPE,
		"U", "          ",
		" ", "T", "Transaction Sort :", "Sort by C (ustomer or T (ransactions) <Default = T> for Head office Customer.",
		YES, NO,  JUSTLEFT, "TC", "", local_rec.transactionSort},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, YES, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<get_lpno.h>
#include <FindCumr.h>
#include <chk_ring_sec.h>

/*-------------------------------------------------------------------
|	Structure for dynamic array,  for the Customer lines for qsort	|
-------------------------------------------------------------------*/
struct CustRecord
{
	char	custNumber [7];
	long	hhcuHash;
	char	custName [41];
}	*cust;
	DArray cust_d;
	int	custCnt = 0;

/*
 * Main Processing Routine
 */
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind  = atoi (get_env ("DB_FIND"));
	envVarRepTax  = atoi (get_env ("REP_TAX"));

	/*----------------------
	| Get native currency. |
	----------------------*/
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	/*----------------------------------------
	| Check for Multi currency debtors flag. |
	----------------------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envVarDbTotalAge = FALSE;
	else
		envVarDbTotalAge = (*sptr == 'T' || *sptr == 't');

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *)0) ? envVarDbNettUsed : atoi (sptr);

	/*---------------------------------------------------------------
	| Check if ageing is by days overdue or as per standard ageing. |
	---------------------------------------------------------------*/
	sptr = chk_env ("DB_DAYS_AGEING");
	envVarDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("TS_INSTALLED");
	envVarTsInstalled = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 *    Allocate intital cheque and detail buffers
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 1000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	SETUP_SCR (vars);

	InitML ();

	input_row = 2,
	error_line = 20;

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	swide ();
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/

	tab_row = 5; 
	tab_col = 5;

	/*---------------------------
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNo, (envVarDbCo) ? comm_rec.est_no : " 0");

	monthEndDate = MonthEnd (comm_rec.dbt_date);
	strcpy (local_rec.dbt_date, DateToString (comm_rec.dbt_date));

	fiscal = comm_rec.fiscal;

	local_rec.lsystemDate = TodaysDate ();
	DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, NULL);

	_chk_ring_sec (_main_menu, "so_cc_disp");

	if (UserSecurity == (char *)0)
	{
		print_mess (ML ("User Not Setup"));
		sleep (sleepTime);
		return (EXIT_FAILURE);

	}

	if (!envVarTsInstalled)
		_main_menu [17].flag = DISABLED;

	sprintf (local_rec.prev_dbt,"%-6.6s"," ");

	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		displayOK = 0;
		search_ok = 1;
		clearOK = TRUE;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		displayOK = TRUE;
		clearOK = FALSE;
		heading (1);
#ifndef GVISION
		run_menu (_main_menu,"",20);
#else
        run_menu (_main_group, _main_menu);
#endif
		crsr_on ();
		strcpy (local_rec.prev_dbt,cumr_rec.dbt_no);

	}
	ArrDelete (&cheq_d);
	ArrDelete (&dtls_d);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Multi-lingual initialisation. 
 */
void
InitML (void)
{
	int		i;

    InitMonthNames ();
	for (i = 0; i < 12; i++)
        SetMonthName (i, ML (month_nm [i])); 

	strcpy (mlCcDisp [1], ML ("Customer Number"));
	strcpy (mlCcDisp [2], ML ("Name"));
	strcpy (mlCcDisp [3], ML ("Postal"));
	strcpy (mlCcDisp [4], ML ("Delivery"));
	strcpy (mlCcDisp [5], ML ("Currency"));
	strcpy (mlCcDisp [6], ML ("Country"));
	strcpy (mlCcDisp [7], ML ("Phone #"));
	strcpy (mlCcDisp [8], ML ("Contact #1"));
	strcpy (mlCcDisp [9], ML ("Fax Number"));
	strcpy (mlCcDisp [10], ML ("Telex #"));
	strcpy (mlCcDisp [11], ML ("Contact #2"));
	strcpy (mlCcDisp [12], ML ("Postal Code"));
	strcpy (mlCcDisp [13], ML ("Mtd Sales"));
	strcpy (mlCcDisp [14], ML ("Contact #3"));
	strcpy (mlCcDisp [15], ML ("Sales Last 12 Months"));
	strcpy (mlCcDisp [16], ML ("Ytd Sales"));
	strcpy (mlCcDisp [17], ML ("Date account opened"));
	strcpy (mlCcDisp [18], ML ("Last Sale Date"));
	strcpy (mlCcDisp [19], ML ("Balance"));
	strcpy (mlCcDisp [20], ML ("Forward Cheques"));
	strcpy (mlCcDisp [21], ML ("Credit Limit / Terms"));
	strcpy (mlCcDisp [22], ML ("Forward"));
	strcpy (mlCcDisp [23], ML ("Credit Terms"));
	strcpy (mlCcDisp [24], ML ("NONE"));
	strcpy (mlCcDisp [25], ML ("Days"));
	strcpy (mlCcDisp [26], ML ("Current"));
	strcpy (mlCcDisp [27], ML ("Credit Extension"));
	strcpy (mlCcDisp [28], ML ("Overdue 1"));
	strcpy (mlCcDisp [29], ML ("No of days on stop credit"));
	strcpy (mlCcDisp [30], ML ("Overdue 2"));
	strcpy (mlCcDisp [31], ML ("Total no of days on stop"));
	strcpy (mlCcDisp [32], ML ("Overdue 3"));
	strcpy (mlCcDisp [33], ML ("Last Receipt Date"));
	strcpy (mlCcDisp [34], ML ("Overdue 4+"));
	strcpy (mlCcDisp [35], ML ("Amount of Last Receipt"));
	strcpy (mlCcDisp [36], ML ("C U S T O M E R  N U M B E R"));
	strcpy (mlCcDisp [37], ML ("TOTAL FOR"));
	strcpy (mlCcDisp [38], ML ("GRAND TOTAL"));
	strcpy (mlCcDisp [39], ML ("GRN NUMBER"));
	strcpy (mlCcDisp [40], ML ("Other Address No"));
	strcpy (mlCcDisp [41], ML ("Option is only available for multi currency module"));
	strcpy (mlCcDisp [42], ML ("Receipt"));
	strcpy (mlCcDisp [43], ML ("OR No"));
	strcpy (mlCcDisp [44], ML ("Cheque Number"));
	strcpy (mlCcDisp [45], ML ("Alt. Drawer Details"));
	strcpy (mlCcDisp [46], ML ("Cheque / Invoice Ratio Incorrect, Cheque = %d, Invoice = %d"));
	strcpy (mlCcDisp [47], ML ("Backorders Allowed"));
	strcpy (mlCcDisp [48], ML ("Consolidate Backorders"));
	strcpy (mlCcDisp [49], ML ("YES"));
	strcpy (mlCcDisp [50], ML ("NO"));
	strcpy (mlCcDisp [51], ML ("No days allowed on B/O"));
	strcpy (mlCcDisp [52], ML ("Account Type"));
	strcpy (mlCcDisp [53], ML ("Open Item"));
	strcpy (mlCcDisp [54], ML ("Balance B/F"));
	strcpy (mlCcDisp [55], ML ("Purchase Order Required"));
	strcpy (mlCcDisp [56], ML ("Charge to Head Office"));
	strcpy (mlCcDisp [57], ML ("Statement Required"));
	strcpy (mlCcDisp [58], ML ("Statement Type"));
	strcpy (mlCcDisp [59], ML ("Interest Charges"));
	strcpy (mlCcDisp [60], ML ("Small Order Surcharge "));
	strcpy (mlCcDisp [61], ML ("Bank Code"));
	strcpy (mlCcDisp [62], ML ("Branch Code"));
	strcpy (mlCcDisp [63], ML ("A (TAX EXEMPT)"));
	strcpy (mlCcDisp [64], ML ("B (TAX YOUR CARE)"));
	strcpy (mlCcDisp [65], ML ("C (TAXABLE)"));
	strcpy (mlCcDisp [66], ML ("D (TAXABLE ON TAX AMOUNT)"));
	strcpy (mlCcDisp [67], ML ("Discount Code / Percent"));
	strcpy (mlCcDisp [68], ML ("Tax Code"));
	strcpy (mlCcDisp [69], ML ("Supplier Number"));
	strcpy (mlCcDisp [70], ML ("H/O  Customer"));
	strcpy (mlCcDisp [71], ML ("Not Found"));
	strcpy (mlCcDisp [72], ML ("Area Code"));
	strcpy (mlCcDisp [73], ML ("Salesman Code"));
	strcpy (mlCcDisp [74], ML ("Customer Type"));
	strcpy (mlCcDisp [75], ML ("Royalty Type"));
	strcpy (mlCcDisp [76], ML ("(Customer Specials)"));
	strcpy (mlCcDisp [77], ML ("Customer Type"));
	strcpy (mlCcDisp [78], ML ("END OF CUSTOMER TYPE SPECIALS"));
	strcpy (mlCcDisp [79], ML ("Customers Credit Control Display"));
	strcpy (mlCcDisp [80], ML ("Last Customer"));
	strcpy (mlCcDisp [81], ML ("NOTE CUSTOMER HAS CREDIT CONTROL NOTES"));
	strcpy (mlCcDisp [82], ML ("Customer"));
	strcpy (mlCcDisp [83], ML ("INVOICES (DEBIT)"));
	strcpy (mlCcDisp [84], ML ("CREDITS (CREDIT)"));
	strcpy (mlCcDisp [85], ML ("JOURNALS"));
	strcpy (mlCcDisp [86], ML ("BALANCE"));
	strcpy (mlCcDisp [87], ML ("FORWARD CHEQUES (CREDIT)"));
	strcpy (mlCcDisp [88], ML ("CURRENT BALANCE"));
	strcpy (mlCcDisp [89], ML ("CURRENT"));
	strcpy (mlCcDisp [90], ML ("DAYS"));
	strcpy (mlCcDisp [91], ML ("FORWARD"));
	strcpy (mlCcDisp [92], ML ("TOTAL"));
	strcpy (mlCcDisp [93], ML ("OVERDUE 1"));
	strcpy (mlCcDisp [94], ML ("OVERDUE 2"));
	strcpy (mlCcDisp [95], ML ("OVERDUE 3"));
	strcpy (mlCcDisp [96], ML ("OVERDUE 4+"));
	strcpy (mlCcDisp [97], ML ("Total for Customer"));
	strcpy (mlCcDisp [98], ML ("CUSTOMER TOTAL"));
	strcpy (mlCcDisp [99], ML ("ORDER TOTAL"));
	strcpy (mlCcDisp [100], ML ("CHEQUES (CREDIT)"));
	strcpy (mlCcDisp [101], ML ("Job Title"));
	strcpy (mlCcDisp [102], ML ("Date Created"));
	strcpy (mlCcDisp [103], ML ("Date Last Phoned"));
	strcpy (mlCcDisp [104], ML ("Best Phone Time"));
	strcpy (mlCcDisp [105], ML ("Phone Frequency"));
	strcpy (mlCcDisp [106], ML ("Next Phone Date"));
	strcpy (mlCcDisp [107], ML ("Next Phone Time"));
	strcpy (mlCcDisp [108], ML ("Next Visit Date"));
	strcpy (mlCcDisp [109], ML ("Next Visit Time"));
	strcpy (mlCcDisp [110], ML ("Email Address"));
}

/*
 * Program exit sequence	
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
    DeleteMonthNames ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (inmr2, inmr);

	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS,
							(!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cusa,  cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	open_rec (cucc,  cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (excl,  excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exdf,  exdf_list, EXDF_NO_FIELDS, "exdf_id_no");
	open_rec (dbry,  dbry_list, DBRY_NO_FIELDS, "dbry_id_no");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");

	if (envVarTsInstalled)
	{
		open_rec (tspm, tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
		open_rec (tmpf, tmpf_list, TMPF_NO_FIELDS, "tmpf_id_no");
		open_rec (tsxd, tsxd_list, TSXD_NO_FIELDS, "tsxd_id_no");
	}
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	if (masterfile_open)
		Dsp_close ();

	if (total_open)
		Dsp_close ();

	if (main_open)
		Dsp_close ();

	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (cudt);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (cucc);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_fclose (excl);
	abc_fclose (exdf);
	abc_fclose (dbry);
	abc_fclose (comm);
	abc_fclose (cohr);
	abc_fclose (coln);
	if (envVarTsInstalled)
	{
		abc_fclose (tspm);
		abc_fclose (tsxd);
		abc_fclose (tmpf);
	}
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int		i,
		this_page;

	/*
	 * Validate Customer number. 
	 */
	if (LCHECK ("dbt_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.dbt_no, 6));

		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf (err_str,"%s", ML (mlStdMess021));
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		curr_hhcu = cumr_rec.hhcu_hash;

		GetChequeData (TRUE, cumr_rec.hhcu_hash);
		if (cumr_rec.ho_dbt_hash > 0L)
			GetChequeData (FALSE, cumr_rec.ho_dbt_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			GetChequeData (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
	
		GetCustomerStuff (TRUE, cumr_rec.hhcu_hash);

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			GetCustomerStuff (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Start Date. 
	 */
	if (LCHECK ("st_date"))
	{
		if (dflt_used)
		{
			local_rec.start_date = 0L;
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			local_rec.start_date > local_rec.end_date)
		{
			print_mess (ML (mlStdMess057));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate End Date. |
	--------------------*/
	if (LCHECK ("end_date"))
	{
		if (dflt_used)
		{
			local_rec.end_date = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (local_rec.start_date > local_rec.end_date)
		{
			print_mess (ML (mlStdMess058));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("comment"))
	{
		if (dflt_used && prog_status != ENTRY)
		{
			lcount [2]--;
			this_page = line_cnt / TABLINES;
			for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				if (this_page == line_cnt / TABLINES)
					line_display ();
			}
			sprintf (cucc_rec.comment,"%80.80s"," ");
			sprintf (cucc_rec.con_person,"%20.20s"," ");
			cucc_rec.cont_date = 0L;
			sprintf (cucc_rec.hold_ref,"%8.8s"," ");
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				line_display ();
			line_cnt = i;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------------------
	| Validate Invoice Hold Referance. |
	----------------------------------*/
	if (LCHECK ("hold_ref"))
	{
	    if (strcmp (cucc_rec.hold_ref, "        ") != 0)
	    {
			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
			strcpy (cuin_rec.inv_no,cucc_rec.hold_ref);
			cc = find_rec (cuin, &cuin_rec, COMPARISON ,"r");
			if (cc)
			{
				strcpy (cuin_rec.inv_no,zero_pad (cucc_rec.hold_ref,8));
				cc = find_rec (cuin, &cuin_rec, COMPARISON ,"r");
			}
			if (cc)
			{
				errmess (ML (mlStdMess115));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	    }
	    return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Display Next Item			            
 */
int
NextDisplay (void)
{
	ChangedData (FN14);
	Redraw ();
    return (EXIT_SUCCESS);
}
/*
 * Order Display. 			            
 */
int
OrderDisplay (void)
{
	orderDisplay = TRUE;
	OrderProcess (FALSE);
	Redraw ();
    return (EXIT_SUCCESS);
}
/*
 * Backorder Display. 			        
 */
int
BackOrderDisplay (void)
{
	OrderProcess (TRUE);
	Redraw ();
    return (EXIT_SUCCESS);
}

int
ClearRedraw (void)
{
	swide ();
	clear ();
	Redraw ();
    return (EXIT_SUCCESS);
}

void
Redraw (void)
{
	clearOK = FALSE;
	heading (1);
}

/*
 * display prevoius item			        
 */
int
PreviousDisplay (void)
{
	ChangedData (FN15);
	Redraw ();
    return (EXIT_SUCCESS);
}

/*
 * Finds Next or Previous Record	
 * key = NEXT_FND - Next Record	
 * key = PREV_FND - Prev Record
 */
void
ChangedData (
	int	key)
{
	char	last_dbt [7];

	strcpy (last_dbt,cumr_rec.dbt_no);

	cc = find_rec (cumr,&cumr_rec, (key == FN14) ? GTEQ : LTEQ,"r");
	if (!cc)
		cc = find_rec (cumr,&cumr_rec, (key == FN14) ? NEXT : PREVIOUS,"r");
	if (cc || strcmp (cumr_rec.co_no,comm_rec.co_no) ||
		  strcmp (cumr_rec.est_no,branchNo))
	{
		AlternateMess (ML ("Beginning or End of File"));
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,last_dbt);
		cc = find_rec (cumr,&cumr_rec,GTEQ,"r");
	}

	strcpy (local_rec.dbt_no,cumr_rec.dbt_no);
	strcpy (local_rec.prev_dbt,cumr_rec.dbt_no);

	GetChequeData (TRUE, cumr_rec.hhcu_hash);
	if (cumr_rec.ho_dbt_hash > 0L)
		GetChequeData (FALSE, cumr_rec.ho_dbt_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		GetChequeData (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}

	GetCustomerStuff (TRUE, cumr_rec.hhcu_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		GetCustomerStuff (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
}

int
MainDisplayScreen (void)
{
	char	dl_adr [161],
			ch_adr [161];

	long	total_stop = 0L;
	int		i;
	int		j;
	int		cr_ext_day = 0;
	int		day_stop = 0;

	float	locPercent [6]	=	{0,0,0,0,0,0},
			fgnPercent [6]	=	{0,0,0,0,0,0};

	double	fgnBalance 		= 0.00,
			locBalance 		= 0.00;

	char	wk_str [12];
	char	wk_date [2] [11];
	char	useCurr	[4];

	displayOK = 1;

	sprintf 
	(
		head_str, 
		"Customer %s (%s)",
		cumr_rec.dbt_no,
		cumr_rec.dbt_name
	);

	for (j = 0; j < 6; j++)
	{
		locPercent [j] = 0.00;
		fgnPercent [j] = 0.00;
	}

	GetCustomerStuff (TRUE, cumr_rec.hhcu_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		GetCustomerStuff (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}

	CalculateInvoice (TRUE, cumr_rec.hhcu_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		CalculateInvoice (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}

	for (i = 0; i < 6; i++)
	{
		fgnBalance 		+= fgnTotal [i];
		locBalance 		+= locTotal [i];
	}

	if (!MoneyZero (fgnBalance))
	{
		for (j = 0; j < 6; j++)
		{
			fgnPercent [j] = (float) ((fgnTotal [j] / fgnBalance) * 100.00);
			locPercent [j] = (float) ((locTotal [j] / locBalance) * 100.00);
		}
	}

	if (masterfile_open)
	{
		Dsp_close ();
		masterfile_open = FALSE;
	}

	if (total_open)
	{
		Dsp_close ();
		total_open = FALSE;
	}

	if (main_open)
		Dsp_close ();

	main_open = TRUE;

	lp_x_off = 0;
	lp_y_off = 1;
	Dsp_nc_prn_open (0, 1, 15, 
					 head_str,
					 comm_rec.co_no, comm_rec.co_name, 
					 comm_rec.est_no, comm_rec.est_name, 
					(char *) 0, (char *) 0);

	window_cnt = 1;
	sprintf (disp_str,
			 ". %-16.16s: %6.6s (%9.9s) %43.43s %-5.5s: %40.40s ",
			 mlCcDisp [1],
			 cumr_rec.dbt_no,
			 cumr_rec.dbt_acronym,
			 " ",
			 mlCcDisp [2],
			 cumr_rec.dbt_name);

	Dsp_saverec (disp_str);
	Dsp_saverec ("");
	Dsp_saverec ("");

	sprintf (dl_adr, 
			 "%s,%s,%s,%s", 
			 clip (cumr_rec.dl_adr1),
			 clip (cumr_rec.dl_adr2),
			 clip (cumr_rec.dl_adr3),
			 clip (cumr_rec.dl_adr4));

	sprintf (ch_adr, 
			 "%s,%s,%s,%s", 
			 clip (cumr_rec.ch_adr1),
			 clip (cumr_rec.ch_adr2),
			 clip (cumr_rec.ch_adr3),
			 clip (cumr_rec.ch_adr4));

	sprintf (disp_str, " %-12.12s: %-104.104s", mlCcDisp [3], ch_adr);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %-12.12s: %-104.104s", mlCcDisp [4], dl_adr);
	Dsp_saverec (disp_str);

	if (envVarDbMcurr)
	{
		sprintf (disp_str, 
				 " %-12.12s: %s (%-40.40s)             %-12.12s               : %s (%-20.20s) ",
				 mlCcDisp [5],
				 cumr_rec.curr_code, 
				 pocr_rec.description,
				 mlCcDisp [6],
		 		 cumr_rec.ctry_code, 
				 pocf_rec.description);

		Dsp_saverec (disp_str);
	}
	else
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	sprintf (disp_str, 
			 " %-12.12s:  %15.15s (%-12.12s: %20.20s)%5.5s %-12.12s               :         %15.15s",
			 mlCcDisp [7],
			 cumr_rec.phone_no,
			 mlCcDisp [8],
			 cumr_rec.contact_name,
			 " ",
			 mlCcDisp [9],
			 cumr_rec.fax_no);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-12.12s:  %15.15s (%-12.12s: %20.20s)%5.5s %-12.12s               :             %10.10s ",
			 mlCcDisp [10],
			 cumr_rec.telex,
			 mlCcDisp [11],
			 cumr_rec.contact2_name,
			 " ",
			 mlCcDisp [12],
			 cumr_rec.post_code);
	Dsp_saverec (disp_str);

	strcpy (fgnFmt [0], CF (mtd_sales, "NNN,NNN,NNN.NN"));
	strcpy (fgnFmt [1], CF (last_12mth,"NNN,NNN,NNN.NN"));

	sprintf (disp_str, 
			 " %-12.12s:%14.14s    (%-12.12s: %20.20s)%5.5s %-25.25s  :         %14.14s ",
			 mlCcDisp [13],
			 fgnFmt [0], 
			 mlCcDisp [14],
			 cumr_rec.contact3_name,
			 " ",
			 mlCcDisp [15],
			 fgnFmt [1]);

	Dsp_saverec (disp_str);

	sprintf (wk_date [0], "%-10.10s", DateToString (cumr_rec.date_open));
	sprintf (wk_date [1], "%-10.10s", DateToString (cumr_rec.date_lastinv));

	sprintf (disp_str, 
			 " %-12.12s:%14.14s    (%-20.20s : %10.10s)%7.7s%-25.25s  :             %s ",
			 mlCcDisp [16],
			 CF (ytd_sales, "NNN,NNN,NNN.NN"),
			 mlCcDisp [17],
			 wk_date [0],
			 " ",
			 mlCcDisp [18],
			 wk_date [1]);

	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	if (localValue == TRUE)
		sprintf (useCurr, "%-3.3s", envVarCurrCode);
	else
		sprintf (useCurr, "%-3.3s", cumr_rec.curr_code);

	strcpy (fgnFmt [0] , CF (fgnBalance, 			"NNN,NNN,NNN.NN"));
	strcpy (fgnFmt [1] , CF (fgnTotFwdChq,  		"NNN,NNN,NNN.NN"));
	strcpy (fgnFmt [2] , CF (cumr_rec.credit_limit,	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locBalance, 			"NNN,NNN,NNN.NN"));
	strcpy (locFmt [1] , CF (locTotFwdChq,  		"NNN,NNN,NNN.NN"));
	sprintf (disp_str, 
			 " %-12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-25.25s  :         %14.14s ",
			 mlCcDisp [19],
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 100.00, 
			 (invoiceDueDate) ? " " : ML (" Note : Ageing = Invoice Date  "),
			 mlCcDisp [20],
			 (localValue) ? locFmt [1] : fgnFmt [1]);
	Dsp_saverec (disp_str);

	strcpy (fgnFmt [0] , CF (fgnTotal [5],	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [5],	"NNN,NNN,NNN.NN"));

	sprintf (disp_str, 
			 " %-12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-25.25s  :         %14.14s/%s",
			 mlCcDisp [22],
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [5] : fgnPercent [5], 
			 " ",
			 mlCcDisp [21],
			 fgnFmt [2],
			 cumr_rec.crd_prd);
	Dsp_saverec (disp_str);

	cr_ext_day = atoi (cumr_rec.crd_prd) - 30;
	if (cr_ext_day <= 0)
		sprintf (wk_str, "%-11.11s", mlCcDisp [24]);
	else
		sprintf (wk_str, "%3d %-7.7s", cr_ext_day, mlCcDisp [25]);

	strcpy (fgnFmt [0] , CF (fgnTotal [0],	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [0],	"NNN,NNN,NNN.NN"));

	sprintf (disp_str, 
			 " %-12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-20.20s       :           %11.11s ",
			 mlCcDisp [26],
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [0] : fgnPercent [0], 
			 " ", 
			 mlCcDisp [27],
			 wk_str);
	Dsp_saverec (disp_str);

	day_stop = (int) (comm_rec.dbt_date - cumr_rec.date_stop);
	if (cumr_rec.date_stop <= 0L || day_stop <= 0)
		sprintf (wk_str, "%-11.11s", mlCcDisp [24]);
	else
		sprintf (wk_str,"%3d %-7.7s",day_stop, mlCcDisp [25]);

	if (envVarDbDaysAgeing)
    	sprintf (err_str," 1-%2d %-6.6s", envVarDbDaysAgeing, mlCcDisp [25]);
	else
    	sprintf (err_str, "%-12.12s", mlCcDisp [28]);

	strcpy (fgnFmt [0] , CF (fgnTotal [1],	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [1],	"NNN,NNN,NNN.NN"));
	sprintf (disp_str, 
			 " %12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-25.25s  :           %11.11s ",
			 err_str,
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [1] : fgnPercent [1], 
			 " ",
			 mlCcDisp [29],
			 wk_str);
	Dsp_saverec (disp_str);


	total_stop = (cumr_rec.date_stop != 0L) ? (comm_rec.dbt_date -
		      cumr_rec.date_stop) : 0L;

	total_stop += cumr_rec.total_days_sc;

	if (total_stop <= 0L)
		sprintf (wk_str,"%-11.11s", mlCcDisp [24]);
	else
		sprintf (wk_str,"%3d %-7.7s", (int) total_stop, mlCcDisp [25]);

	if (envVarDbDaysAgeing)
	{
    	sprintf (err_str,
				 "%2d-%2d %-6.6s", 
				 envVarDbDaysAgeing + 1, 
				 envVarDbDaysAgeing * 2, 
				 mlCcDisp [25]);
	}
	else
    	sprintf (err_str, "%-12.12s", mlCcDisp [30]);

	strcpy (fgnFmt [0] , CF (fgnTotal [2],	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [2],	"NNN,NNN,NNN.NN"));
	sprintf (disp_str, 
			 " %12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-25.25s  :           %11.11s ",
			 err_str,
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [2] : fgnPercent [2], 
			 " ",
			 mlCcDisp [31],
			 wk_str);
	Dsp_saverec (disp_str);

	if (envVarDbDaysAgeing)
	{
    	sprintf (err_str,
				 "%2d-%2d %-6.6s",
				(envVarDbDaysAgeing * 2) + 1,
				 envVarDbDaysAgeing * 3,
				 mlCcDisp [25]);
	}
	else
    	sprintf (err_str, "%-12.12s", mlCcDisp [32]);

	strcpy (fgnFmt [0] , CF (fgnTotal [3],	"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [3],	"NNN,NNN,NNN.NN"));
	sprintf (disp_str, 
			 " %12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-20.20s       :            %10.10s ",
			 err_str,
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [3] : fgnPercent [3], 
			 " ",
			 mlCcDisp [33],
			 DateToString (cumr_rec.date_lastpay));

	Dsp_saverec (disp_str);

	strcpy (fgnFmt [0] , CF (fgnTotal [4], "NNN,NNN,NNN.NN"));
	strcpy (fgnFmt [1] , CF (cumr_rec.amt_lastpay,"NNN,NNN,NNN.NN"));
	strcpy (locFmt [0] , CF (locTotal [4], "NNN,NNN,NNN.NN"));

	if (envVarDbDaysAgeing)
	{
    	sprintf (err_str,
				 "Over %2d%-5.5s",
				 envVarDbDaysAgeing * 3,
				 mlCcDisp [25]);
	}
	else
    	sprintf (err_str, "%-12.12s", mlCcDisp [34]);

	sprintf (disp_str, 
			 " %12.12s:%14.14s (%3.3s) %6.2f%% %30.30s %-25.25s  :         %14.14s ",
			 err_str,
			 (localValue) ? locFmt [0] : fgnFmt [0], 
			 useCurr,
			 (localValue) ? locPercent [4] : fgnPercent [4], 
			 " ", 
			 mlCcDisp [35],
			 fgnFmt [1]);

	Dsp_saverec (disp_str);
	Dsp_srch ();
    return (EXIT_SUCCESS);
}

void
GetCustomerStuff (
	int 	clearTotals, 
	long 	hhcuHash)
{
	int		i;

	DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, NULL);
	currentMonth--;

	/*---------------------------------------
	| Load Year To Date Sales from incc	|
	---------------------------------------*/
	if (clearTotals)
	{
		mtd_sales 	= 0.00;
		ytd_sales 	= 0.00;
		last_12mth  = 0.00;

		for (i = 0; i < 12; i++)
		{
			ly_sales [i] = 0.00;
			cy_sales [i] = 0.00;
		}
	}

	cusa_rec.hhcu_hash = hhcuHash;
	strcpy (cusa_rec.year,"C");
	cc = find_rec (cusa,&cusa_rec,COMPARISON,"r");
	if (!cc)
	{
		for (i = 0; i < 12; i++)
		{
			last_12mth    += cusa_val [i];
			cy_sales [i] += cusa_val [i];
		}

		mtd_sales += cusa_val [currentMonth];
		ytd_sales += CalculateYtd ();
	}
	
	cusa_rec.hhcu_hash = hhcuHash;
	strcpy (cusa_rec.year,"L");
	cc = find_rec (cusa,&cusa_rec,COMPARISON,"r");
	if (!cc)
	{
		for (i = 0; i < 12; i++)
			ly_sales [i] += cusa_val [i];
	}
	if (!clearTotals)
		return;

	open_rec (pocr,pocr_list,POCR_NO_FIELDS,"pocr_id_no");
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, cumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%40.40s"," ");

	abc_fclose (pocr);
	
	open_rec (pocf,pocf_list,POCF_NO_FIELDS,"pocf_id_no");

	strcpy (pocf_rec.co_no, comm_rec.co_no);
	strcpy (pocf_rec.code, cumr_rec.ctry_code);
	cc = find_rec (pocf, &pocf_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocf_rec.description, "%20.20s"," ");

	abc_fclose (pocf);
}

int
InvoiceSetup (void)
{
	int		i;

	FLD ("st_date")		=	YES;
	FLD ("end_date")	=	YES;
	entry_exit 	=	0;
	prog_exit	=	0;
	restart		=	0;
	displayOK	=	1;
	search_ok	=	1;

	FLD ("transactionSort")	=	(HAS_HO) ? ND : YES;

	init_vars (4);
	scn_set (4);

	heading (4);
	scn_display (4);
	entry (4);

	if (restart)
	{
		restart 	=	FALSE;
		ClearRedraw ();
		return (EXIT_FAILURE);
	}

	heading (4);
	scn_display (4);
	edit (4);
	if (restart)
	{
		restart 	=	FALSE;
		ClearRedraw ();
		return (EXIT_FAILURE);
	}
	if (HAS_HO) 
		strcpy (local_rec.transactionSort, "C");

	for (i = 0; i < 4; i++)
	{
		mth_total [i] = 0.00;
		cus_total [i] = 0.00;
		grd_total [i] = 0.00;
	}

	saveIndex = 0;

	lp_x_off = 4;
	lp_y_off = 3;
	if (TRAN_DISP)
		Dsp_prn_open ((envVarDbMcurr) ? 1 : 14, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);
	else
		Dsp_prn_open ((envVarDbMcurr) ? 20 : 30, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
		{
			Dsp_saverec ("  INVOICE  |BR| DATE  OF | PAY |   DATE   |    AMOUNT    |   EXCH   |F|    BALANCE   | CHEQUE |     AMOUNT   |    BALANCE   ");
			Dsp_saverec ("  NUMBER   |NO| INVOICE  |TERMS|    DUE   |   (DEBIT)    |   RATE   |X|     LOCAL    | NUMBER |    (CREDIT)  |    DUE       ");
		}
		else
		{
			Dsp_saverec ("  INVOICE  |BR| DATE  OF | PAY |   DATE   |    AMOUNT    | CHEQUE |     AMOUNT   |    BALANCE   ");
			Dsp_saverec ("  NUMBER   |NO| INVOICE  |TERMS|    DUE   |   (DEBIT)    | NUMBER |    (CREDIT)  |    DUE       ");
		}
	}
	else
	{
		if (envVarDbMcurr)
		{
			Dsp_saverec ("  INVOICE  |BR| DATE  OF | PAY |   DATE   |    AMOUNT    |   EXCH   |F|    BALANCE   ");
			Dsp_saverec ("  NUMBER   |NO| INVOICE  |TERMS|    DUE   |   (DEBIT)    |   RATE   |X|     LOCAL    ");
		}
		else
		{
			Dsp_saverec ("  INVOICE  |BR| DATE  OF | PAY |   DATE   |    AMOUNT    ");
			Dsp_saverec ("  NUMBER   |NO| INVOICE  |TERMS|    DUE   |   (DEBIT)    ");
		}
	}
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");
	return (EXIT_SUCCESS);
}

int	
DisplayInvoice (
 int		clearTotals, 
 long	hhcu_hash, 
 char	*drop_ship)
{
	int		old_mth	= 0,
			new_mth;

	firstTime 	= 	TRUE;

	strcpy (cuin_rec.est,"  ");
	cuin_rec.hhcu_hash 	= (SORT_C) ? hhcu_hash 	: 0L;
	cuin_rec.ho_hash 	= (SORT_C) ? 0L 		: hhcu_hash;
	cuin_rec.date_of_inv 		= local_rec.start_date;
	strcpy (cuin_rec.inv_no," "); 
	cc = find_rec (cuin,&cuin_rec,GTEQ,"r");
	while (!cc && 
		((SORT_C && hhcu_hash == cuin_rec.hhcu_hash) ||
		(SORT_T && hhcu_hash == cuin_rec.ho_hash)) &&
		  cuin_rec.date_of_inv <= local_rec.end_date)
	{
		if ((drop_ship [0] == 'A') ||
			(cuin_rec.drop_ship [0] == 'Y' && drop_ship [0] == 'Y') ||
		    (cuin_rec.drop_ship [0] != 'Y' && drop_ship [0] == 'N'))
		{
			DateToDMY (cuin_rec.date_of_inv, NULL, &new_mth, NULL);
	
			if (firstTime)
			{
				if (SORT_C)
				{
			   		sprintf (disp_str,
						"%-25.25s   : %s (%-40.40s)",
						mlCcDisp [36],
						(clearTotals) ? cumr_rec.dbt_no : cumr2_rec.dbt_no,
						(clearTotals) ? cumr_rec.dbt_name   : cumr2_rec.dbt_name);
					Dsp_saverec (disp_str);
				}
								
				old_mth = new_mth;
			}
			firstTime = FALSE;
	
			if (old_mth != new_mth)
			{
				TotalMonth (old_mth);
				old_mth = new_mth;
			}
			ProcessInvoices ();
		}
		cc = find_rec (cuin,&cuin_rec,NEXT,"r");
	}
	if (!firstTime)
	{
		TotalMonth (old_mth);
		if (SORT_C)
			TotalCustomer ((clearTotals) ? cumr_rec.dbt_no : cumr2_rec.dbt_no);
	}
	return (EXIT_SUCCESS);
}

void
TotalCustomer (
 char *cust_no)
{
	strcpy (fgnFmt [0], CF (cus_total [0],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [1], CF (cus_total [1],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [2], CF (cus_total [2],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [3], CF (cus_total [3],"NN,NNN,NNN.NN"));

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       ^1%-10.10s%-9.9s ^E%13.13s ^E          ^E ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [37],
					 cust_no,
					 fgnFmt [0], 
					 fgnFmt [3], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       ^1%-10.10s%-9.9s ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [37],
					 cust_no,
					 fgnFmt [0], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
	}
	else
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       ^1%-10.10s%-9.9s ^E%13.13s ^E          ^E ^E%13.13s ",
					 mlCcDisp [37],
					 cust_no,
					 fgnFmt [0], 
					 fgnFmt [3]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       ^1%-10.10s%-9.9s ^E%13.13s ",
					 mlCcDisp [37],
					 cust_no,
					 fgnFmt [0]);
		}
	}

	Dsp_saverec (disp_str);

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGEGEGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");
	}
	else
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGEGEGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGG");
	}

	cus_total [0] = 0.00;
	cus_total [1] = 0.00;
	cus_total [2] = 0.00;
	cus_total [3] = 0.00;
}

void
TotalMonth (
 int month)
{
	int		i;

	strcpy (fgnFmt [0], CF (mth_total [0],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [1], CF (mth_total [1],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [2], CF (mth_total [2],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [3], CF (mth_total [3],"NN,NNN,NNN.NN"));

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-10.10s%-9.9s ^E%13.13s ^E          ^E ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [37],
					 month_nm [month - 1],
					 fgnFmt [0], 
					 fgnFmt [3], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-10.10s%-9.9s ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [37],
					 month_nm [month - 1],
					 fgnFmt [0], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
	}
	else
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-10.10s%-9.9s ^E%13.13s ^E          ^E ^E%13.13s ",
					 mlCcDisp [37],
					 month_nm [month - 1],
					 fgnFmt [0], 
					 fgnFmt [3]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-10.10s%-9.9s ^E%13.13s ",
					 mlCcDisp [37],
					 month_nm [month - 1],
					 fgnFmt [0]);
		}
	}

	Dsp_saverec (disp_str);

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGEGEGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");
	}
	else
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGEGEGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGG");
	}

	for (i = 0; i < 4; i++)
	{
		grd_total [i] += mth_total [i];
		cus_total [i] += mth_total [i];
		mth_total [i] = 0.00;
	}
}

void
TotalGrand (
 void)
{
	strcpy (fgnFmt [0], CF (grd_total [0],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [1], CF (grd_total [1],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [2], CF (grd_total [2],"NN,NNN,NNN.NN"));
	strcpy (fgnFmt [3], CF (grd_total [3],"NN,NNN,NNN.NN"));

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-15.15s     ^E%13.13s ^E          ^E ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [38],
					 fgnFmt [0], 
					 fgnFmt [3], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-15.15s     ^E%13.13s ^E        ^E%13.13s ^E%13.13s ",
					 mlCcDisp [38],
					 fgnFmt [0], 
					 fgnFmt [1], 
					 fgnFmt [2]);
		}
	}
	else
	{
		if (envVarDbMcurr)
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-15.15s     ^E%13.13s ^E          ^E ^E%13.13s ",
					 mlCcDisp [38],
					 fgnFmt [0], 
					 fgnFmt [3]);
		}
		else
		{
			sprintf (disp_str,
					 "           ^E  ^E       %-15.15s     ^E%13.13s ",
					 mlCcDisp [38],
					 fgnFmt [0]);
		}
	}

	Dsp_saverec (disp_str);

	if (TRAN_DISP)
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGJGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGJGJGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGJGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGG");
	}
	else
	{
		if (envVarDbMcurr)
			Dsp_saverec ("^^GGGGGGGGGGGJGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGJGJGGGGGGGGGGGGGG");
		else
			Dsp_saverec ("^^GGGGGGGGGGGJGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGG");
	}

	grd_total [0] = 0.00;
	grd_total [1] = 0.00;
	grd_total [2] = 0.00;
	grd_total [3] = 0.00;
}

void
ProcessInvoices (
 void)
{
	int 	i;
	int		new_pg = 0;
	int		none = 0;
	char	date1 [12];
	char	date2 [12];
	char	tem_line [200];
	char	env_line [200];
	double	balance = 0.00,
			loc_bal = 0.00;
	

	if (CREDIT)
		balance = (envVarCnNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			      : cuin_rec.amt;
	else
		balance = (envVarDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			      : cuin_rec.amt;


	sprintf (date1,    "%-10.10s", DateToString (cuin_rec.date_of_inv));
	sprintf (date2,    "%-10.10s", DateToString (cuin_rec.date_posted));
	sprintf (pay_date, "%-10.10s", DateToString (cuin_rec.due_date));

	if (cuin_rec.exch_rate == 0.00)
		cuin_rec.exch_rate = 1.0000;

  	balance	= no_dec (balance);
	loc_bal = no_dec (balance / cuin_rec.exch_rate);

	if (envVarDbMcurr)
	{
		strcpy (fgnFmt [0], CF (balance,"NN,NNN,NNN.NN"));
		strcpy (fgnFmt [1], CF (loc_bal,"NN,NNN,NNN.NN"));
		sprintf (tem_line,
				 "%s %s^E%s^E%s^E %s ^E%s^E%13.13s ^E%9.4f ^E%s^E^%13.13s ^E",
				 tran_type [atoi (cuin_rec.type) - 1],
				 cuin_rec.inv_no,
				 cuin_rec.est,
				 date1,
				 cuin_rec.pay_terms,
				 pay_date,
				 fgnFmt [0],
				 cuin_rec.exch_rate,
				 cuin_rec.er_fixed,
				 fgnFmt [1]);
	}
	else
	{
		sprintf (tem_line,
				 "%s %s^E%s^E%s^E %s ^E%s^E%13.13s ^E",
				 tran_type [atoi (cuin_rec.type) - 1],
				 cuin_rec.inv_no,
				 cuin_rec.est,
				 date1,
				 cuin_rec.pay_terms,
				 pay_date,
				 CF (balance, "NN,NNN,NNN.NN"));
	}

	mth_total [0] += no_dec (balance);
	mth_total [3] += no_dec (loc_bal);

	if (INV_DISP)
	{
		sprintf (save_key, 
				 "%04d%2.2s%1.1s%-8.8s%06ld",
			   	 saveIndex++,
			   	 cuin_rec.est,
			   	 cuin_rec.type,
			   	 cuin_rec.inv_no,
			   	 cuin_rec.hhcu_hash);

		if (INVOICE || CREDIT)
			Dsp_save_fn (tem_line, save_key);
		else
			Dsp_saverec (tem_line);

		if (CREDIT && strcmp (cuin_rec.grn_no, "                    "))
		{
			sprintf (tem_line, 
					 "           ^E ^2%-10.10s %s^6", 
					 mlCcDisp [39],
					 cuin_rec.grn_no);
			Dsp_saverec (tem_line);
		}
		return;
	}

	for (i = 0;i < dtls_cnt;i++)
	{
		if (GetInvoices (i,&new_pg,date1,date2,&balance,tem_line))
			none = 1;
	}

	mth_total [2] += no_dec (balance);

	if (none == 0)
	{
		sprintf (env_line,
				 "%s%-8.8s^E              ^E%13.13s ",
				 tem_line,
				 mlCcDisp [24],
				 CF (balance, "NN,NNN,NNN.NN"));

		sprintf (save_key, 
				 "%04d%2.2s%1.1s%-8.8s%06ld",
				 saveIndex++,
				 cuin_rec.est,
				 cuin_rec.type,
				 cuin_rec.inv_no,
				 cuin_rec.hhcu_hash);
		if (INVOICE || CREDIT)
			Dsp_save_fn (env_line, save_key);
		else
			Dsp_saverec (env_line);
	}
	else
	{
		sprintf (env_line,
				 "%s%13.13s ",
				 tem_line,
				 CF (balance, "NN,NNN,NNN.NN"));

		sprintf (save_key, 
				 "%04d%2.2s%1.1s%-8.8s%06ld",
				 saveIndex++,
				 cuin_rec.est,
				 cuin_rec.type,
				 cuin_rec.inv_no,
				 cuin_rec.hhcu_hash);
		if (INVOICE || CREDIT)
			Dsp_save_fn (env_line, save_key);
		else
			Dsp_saverec (env_line);
	}
	if (CREDIT && strcmp (cuin_rec.grn_no, "                    "))
	{
		sprintf (tem_line, 
				 "           ^E ^2%-10.10s %s^6", 
				 mlCcDisp [39],
				 cuin_rec.grn_no);
		Dsp_saverec (tem_line);
	}
}

int
GetInvoices (
	int 	i, 
	int 	*new_pg, 
	char 	*date1, 
	char 	*date2, 
	double 	*balance, 
	char 	*tem_line)
{
	char	env_line [200];
	double	wrk_bal = 0.00,
			loc_bal = 0.00;

	if (dtls [i].hhci_hash == cuin_rec.hhci_hash)
	{
	    if (*new_pg == 0)
	    {
			sprintf (pay_date, "%-10.10s", DateToString (cuin_rec.due_date));

			if (cuin_rec.exch_rate == 0.00)
				cuin_rec.exch_rate = 1.0000;

			wrk_bal = *balance;

			strcpy (fgnFmt [0], CF (wrk_bal,"NN,NNN,NNN.NN"));
			strcpy (fgnFmt [1], CF (dtls [i].inv_fgn_amt,"NN,NNN,NNN.NN"));
			loc_bal = wrk_bal / cuin_rec.exch_rate;

			strcpy (fgnFmt [2],CF (loc_bal,"NN,NNN,NNN.NN"));

			if (envVarDbMcurr)
			{
				sprintf (tem_line,
						 "%s %s^E%s^E%s^E %s ^E%s^E%13.13s ^E%9.4f ^E%s^E%13.13s ^E%s^E%13.13s%s^E",
						 tran_type [atoi (cuin_rec.type) - 1],
						 cuin_rec.inv_no,
						 cuin_rec.est,
						 date1,
						 cuin_rec.pay_terms,
						 pay_date,
						 fgnFmt [0],
						 cuin_rec.exch_rate,
						 cuin_rec.er_fixed,
						 fgnFmt [2],
						 cheq [dtls [i].cheq_hash].no,
						 fgnFmt [1],
						(dtls [i].fwd_payment) ? "*" :  " ");
			}
			else
			{
				sprintf (tem_line,
						 "%s %s^E%s^E%s^E %s ^E%s^E%13.13s ^E%s^E%13.13s%s^E",
						 tran_type [atoi (cuin_rec.type) - 1],
						 cuin_rec.inv_no,
						 cuin_rec.est,
						 date1,
						 cuin_rec.pay_terms,
						 pay_date,
						 fgnFmt [0],
						 cheq [dtls [i].cheq_hash].no,
						 fgnFmt [1],
						(dtls [i].fwd_payment) ? "*" :  " ");
			}
			*new_pg += 1;
	    }
	    else
	    {
			if (*new_pg == 1)
			{
				sprintf (env_line,"%s             ",tem_line);
				sprintf (save_key, 
						 "%04d%2.2s%1.1s%-8.8s%06ld",
						 saveIndex++,
						 cuin_rec.est,
						 cuin_rec.type,
						 cuin_rec.inv_no,
						 cuin_rec.hhcu_hash);
				if (INVOICE || CREDIT)
					Dsp_save_fn (env_line, save_key);
				else
					Dsp_saverec (env_line);
			}
			*new_pg += 1;
			if (*new_pg > 2)
			{
				sprintf (save_key, 
						 "%04d%2.2s%1.1s%-8.8s%06ld",
						 saveIndex++,
						 cuin_rec.est,
						 cuin_rec.type,
						 cuin_rec.inv_no,
						 cuin_rec.hhcu_hash);

				if (INVOICE || CREDIT)
					Dsp_save_fn (env_line, save_key);
				else
					Dsp_saverec (env_line);
			}

			strcpy (fgnFmt [0],CF (dtls [i].inv_fgn_amt,"NN,NNN,NNN.NN"));

			if (envVarDbMcurr)
			{
				sprintf (tem_line, 
						 "           ^E  ^E          ^E     ^E          ^E              ^E          ^E ^E              ^E%s^E%13.13s%s^E",
						 cheq [dtls [i].cheq_hash].no,
						 fgnFmt [0],
						(dtls [i].fwd_payment) ? "*" :  " ");
			}
			else
			{
				sprintf (tem_line, 
						 "           ^E  ^E          ^E     ^E          ^E              ^E%s^E%13.13s%s^E",
						 cheq [dtls [i].cheq_hash].no,
						 fgnFmt [0],
						(dtls [i].fwd_payment) ? "*" :  " ");
			}

			sprintf (env_line,"%s             ",tem_line);
	    }
	    mth_total [1] += dtls [i].inv_fgn_amt;
	    *balance -= dtls [i].inv_fgn_amt;
	    return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
TransactionDisplay (void)
{
	int		i;
	strcpy (processType, "T");

	if (InvoiceSetup ())
		return (EXIT_SUCCESS);

	if (SORT_T)
	{
		abc_selfield ("cuin", "cuin_ho_cron3");
		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "A");
	}
	else
	{
		abc_selfield ("cuin", "cuin_cron3");

		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "A");

		/*----------------------------
		| Allocate the initial array |
		----------------------------*/
		ArrAlloc (&cust_d, &cust, sizeof (struct CustRecord), 100);
		custCnt = 0;

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			/*-------------------------------------------------
			| Check the array size before adding new element. |
			-------------------------------------------------*/
			if (!ArrChkLimit (&cust_d, cust, custCnt))
				sys_err ("ArrChkLimit (cust)", ENOMEM, PNAME);

			/*-----------------------------------------
			| Load values into array element custCnt. |
			-----------------------------------------*/
			sprintf (cust [custCnt].custNumber, "%-6.6s", cumr2_rec.dbt_no);
			sprintf (cust [custCnt].custName, "%-40.40s", cumr2_rec.dbt_name);
			cust [custCnt].hhcuHash = cumr2_rec.hhcu_hash;

			/*--------------------------
			| Increment array counter. |
			--------------------------*/
			custCnt++;

			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		/*-------------------------------------------
		| Sort the array in item description order. |
		-------------------------------------------*/
		qsort (cust, custCnt, sizeof (struct CustRecord), CustIdSort);

		/*----------------------------------------------------------------
		| Step through the sorted array getting the appropriate records. |
		----------------------------------------------------------------*/
		for (i = 0; i < custCnt; i++)
		{
			strcpy (cumr2_rec.dbt_no, 	cust [i].custNumber);
			strcpy (cumr2_rec.dbt_name, 		cust [i].custName);
			DisplayInvoice (FALSE, cust [i].hhcuHash, "A");
		}

		/*--------------------------
		| Free up the array memory |
		--------------------------*/
		ArrDelete (&cust_d);
	}

	TotalGrand ();
	Dsp_srch_fn (InvoiceEnquiry);
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

int
DdInvoiceDisplay (
 void)
{
	strcpy (processType, "I");

	if (InvoiceSetup ())
		return (EXIT_SUCCESS);

	if (SORT_T)
	{
		abc_selfield ("cuin", "cuin_ho_cron");
		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "Y");
	}
	else
	{
		abc_selfield ("cuin", "cuin_cron");

		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "Y");

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			DisplayInvoice (FALSE, cumr2_rec.hhcu_hash, "Y");
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
	}
	TotalGrand ();
	Dsp_srch_fn (InvoiceEnquiry);
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

int
InvoiceDisplay (
 void)
{
	int		i;

	/*----------------------------
	| Allocate the initial array |
	----------------------------*/
	ArrAlloc (&cust_d, &cust, sizeof (struct CustRecord), 100);
	custCnt = 0;

	strcpy (processType, "I");

	if (InvoiceSetup ())
		return (EXIT_SUCCESS);

	if (SORT_T)
	{
		abc_selfield ("cuin", "cuin_ho_cron3");
		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "A");
	}
	else
	{
		abc_selfield ("cuin", "cuin_cron3");

		DisplayInvoice (TRUE, cumr_rec.hhcu_hash, "A");

		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			/*-------------------------------------------------
			| Check the array size before adding new element. |
			-------------------------------------------------*/
			if (!ArrChkLimit (&cust_d, cust, custCnt))
				sys_err ("ArrChkLimit (cust)", ENOMEM, PNAME);

			/*-----------------------------------------
			| Load values into array element custCnt. |
			-----------------------------------------*/
			sprintf (cust [custCnt].custNumber, "%-6.6s", cumr2_rec.dbt_no);
			sprintf (cust [custCnt].custName, "%-40.40s", cumr2_rec.dbt_name);
			cust [custCnt].hhcuHash = cumr2_rec.hhcu_hash;

			/*--------------------------
			| Increment array counter. |
			--------------------------*/
			custCnt++;
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		/*-------------------------------------------
		| Sort the array in item description order. |
		-------------------------------------------*/
		qsort (cust, custCnt, sizeof (struct CustRecord), CustIdSort);

		/*----------------------------------------------------------------
		| Step through the sorted array getting the appropriate records. |
		----------------------------------------------------------------*/
		for (i = 0; i < custCnt; i++)
		{
			strcpy (cumr2_rec.dbt_no, 	cust [i].custNumber);
			strcpy (cumr2_rec.dbt_name, 		cust [i].custName);
			DisplayInvoice (FALSE, cust [i].hhcuHash, "A");
		}

		/*--------------------------
		| Free up the array memory |
		--------------------------*/
		ArrDelete (&cust_d);
	}
	TotalGrand ();
	Dsp_srch_fn (InvoiceEnquiry);
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

int
InvoiceEnquiry (
 char *find_key)
{
	char	_br_no [3];
	char	tmp_type [2];
	char	_type [2];
	char	_inv_no [9];
	long	_hhcu_hash;
	int		ProgramSelect;

	sprintf (_br_no,   "%2.2s",  find_key + 4);
	sprintf (tmp_type, "%1.1s",  find_key + 6);
	sprintf (_inv_no,  "%-8.8s", find_key + 7);
	_hhcu_hash = atol (find_key + 15);

	switch (tmp_type [0])
	{
	case '1':
	case '3':
		strcpy (_type, "I");
		ProgramSelect = 1;
		break;

	case '2':
		strcpy (_type, "C");
		ProgramSelect = 2;
		break;

	case '4':
		strcpy (_type, "N");
		ProgramSelect = 4;
		break;

	default:
		strcpy (_type, "O");
		ProgramSelect = 3;
		break;
	}

	RunSpecialDisplay (comm_rec.co_no, 
	       	       _br_no, 
	              _type, 
	              _hhcu_hash,
	              _inv_no,
				  ProgramSelect);

	return (EXIT_SUCCESS);
}

/*
 * Delivery Address. 
 */
int
DeliveryAddressDisplay (void)
{
	open_rec (cudi,cudi_list,CUDI_NO_FIELDS,"cudi_id_no");

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, 14, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                   DELIVERY ADDRESS DISPLAY                    ");
	Dsp_saverec ("");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END]");
	strcpy (disp_str,"      Master Address : ");
	Dsp_saverec (disp_str);
	sprintf (disp_str,"        %-40.40s ",cumr_rec.dl_adr1);
	Dsp_saverec (disp_str);
	sprintf (disp_str,"        %-40.40s ",cumr_rec.dl_adr2);
	Dsp_saverec (disp_str);
	sprintf (disp_str,"        %-40.40s ",cumr_rec.dl_adr3);
	Dsp_saverec (disp_str);
	sprintf (disp_str,"        %-40.40s ",cumr_rec.dl_adr4);
	Dsp_saverec (disp_str);

	cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cudi_rec.del_no = 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ,"r");

	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		sprintf (disp_str,"%-63.63s"," ");
		Dsp_saverec (disp_str);

		sprintf (disp_str,
				 "      %-17.17s: %05d",
				 mlCcDisp [40],
				 cudi_rec.del_no);
		Dsp_saverec (disp_str);

		sprintf (disp_str,"        %-40.40s",cudi_rec.adr1);
		Dsp_saverec (disp_str);
		sprintf (disp_str,"        %-40.40s",cudi_rec.adr2);
		Dsp_saverec (disp_str);
		sprintf (disp_str,"        %-40.40s",cudi_rec.adr3);
		Dsp_saverec (disp_str);
		sprintf (disp_str,"        %-40.40s",cudi_rec.adr4);
		Dsp_saverec (disp_str);
		cc = find_rec (cudi, &cudi_rec, NEXT,"r");
	}

	abc_fclose (cudi);
	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

/*
 * Display cheque History Information. 
 */
int
ChequeExchHistory (void)
{
	int		i = 0;
	int		ln_num = 0;
	char	old_cheq [9];

	lp_x_off = 4;
	lp_y_off = 3;

	Dsp_prn_open (1, 3, InternalPageSize, head_str, 
							comm_rec.co_no, comm_rec.co_name, 
							comm_rec.est_no, comm_rec.est_name, 
							(char *) 0, (char *) 0);

	Dsp_saverec (" DATE  OF |   DATE   |BANK | DISCOUNT | BANK RECEIPT  |    BANK      |  LOCAL BANK   | RECEIPT AMOUNT| EXCH  RATE| RECEIPT AMOUNT");

	sprintf (err_str,"  CHEQUE  |  POSTED  |CODE |  AMOUNT  |    AMOUNT     |EXCHANGE  RATE|  RECEIPT AMT  |  AMOUNT (%3.3s) |           |     LOCAL     ",cumr_rec.curr_code);
	Dsp_saverec (err_str);
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");

	if (!envVarDbMcurr)
	{
		Dsp_saverec (" ");
		Dsp_saverec (mlCcDisp [41]);
		Dsp_saverec (" ");
		Dsp_srch ();
		Dsp_close ();
		Redraw ();
		return (EXIT_SUCCESS);
	}
	strcpy (old_cheq,"        ");

	for (i = 0;i < dtls_cnt;i++)
		GetExchCheques (i, &ln_num, old_cheq);

	Dsp_srch ();
	Dsp_close ();
	Redraw ();
	return (EXIT_SUCCESS);
}

/*==============
| Get cheques. |
==============*/
int
GetExchCheques (
	int 	i, 
	int 	*ln_num, 
	char 	*old_cheq)
{
	char	fm_str [5] [15];
	int		j;
	char	env_line [300];
	double	net = 0.0;
	double	ChequeExch;

	if (*ln_num >= InternalPageSize)
		*ln_num = *ln_num % InternalPageSize;

	j = dtls [i].cheq_hash;

	if (cheq [j].amount == 0)
		return (EXIT_SUCCESS);

	if (strcmp (old_cheq, cheq [j].no) != 0)
	{
		strcpy (old_cheq,cheq [j].no);
		net = cheq [j].amount + cheq [j].cdisc;

		strcpy (fm_str [0], 
				CF (cheq [j].bank_amt, "NNN,NNN,NNN.NN"));

		if (cheq [j].bank_exch == 0.00)
			cheq [j].bank_exch = 1.00;

		strcpy (fm_str [1], 
			CF (cheq [j].bank_amt / cheq [j].bank_exch, "NNN,NNN,NNN.NN"));

		strcpy (fm_str [2], CF (net, "NNN,NNN,NNN.NN"));
		strcpy (fm_str [3], CF (cheq [j].loc_amt, "NNN,NNN,NNN.NN"));
		strcpy (fm_str [4], CF (cheq [j].cdisc,"NN,NNN.NN"));

		if (net == 0.00)
			ChequeExch = cheq [j].loc_amt / net;
		else
			ChequeExch = 1.00;

		sprintf (env_line,
				" ^1%-7.7s:%8.8s^6 /%-6.6s: %10.10s  /  %-14.14s: %20.20s  /  %-20.20s: %20.20s  ",
				mlCcDisp [42],
				cheq [j].no,
				mlCcDisp [43],
				cheq [j].OR,
				mlCcDisp [44],
				cheq [j].cheque,
				mlCcDisp [45],
				cheq [j].alt_drawer);
		Dsp_saverec (env_line);
	
		sprintf (env_line,
				 "%s^E%s^E%s^E%-9.9s ^E%-14.14s ^E%13.8f ^E%-14.14s ^E%-14.14s ^E%10.4f ^E%-14.14s ",
				 cheq [j].datec,
				 cheq [j].datep,
				 cheq [j].bank_id,
				 fm_str [4], 
				 fm_str [0], 
				 cheq [j].bank_exch,
				 fm_str [1], 
				 fm_str [2], 
				 ChequeExch,
				 fm_str [3]);
		Dsp_saverec (env_line);
		Dsp_saverec ("^^GGGGGGGGGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGGGGGGGGG");
	}
	*ln_num += 1;
    return (EXIT_SUCCESS);
}

/*
 * Display cheque History Information. 
 */
int
ChequeHistory (void)
{
	open_rec (cuph,cuph_list,CUPH_NO_FIELDS,"cuph_hhcu_hash");

	lp_x_off = 1;
	lp_y_off = 3;

	Dsp_nc_prn_open (1, 3, 4, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);
	
	Dsp_saverec (" CHQ NO | CHEQ  AMOUNT  |  CHQ DATE  |BANK.|       BRANCH.        ");
	Dsp_saverec ("");
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END] ");

	ChequeHistDetails (cumr_rec.hhcu_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		ChequeHistDetails (cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	abc_fclose (cuph);
	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}
/*
 * Display cheque History Information. 
 */
void
ChequeHistDetails (
	long	hhcuHash)
{
	cuph_rec.hhcu_hash = hhcuHash + 1L;
	cc = find_rec (cuph, &cuph_rec, LT,"r");
	while (!cc && cuph_rec.hhcu_hash == hhcuHash)
	{
		sprintf (disp_str,
				 "%s^E%14.14s ^E %10.10s ^E %3.3s ^E %20.20s ",
				 cuph_rec.cheq_no,
				 CF (cuph_rec.amt_cheq,"NNN,NNN,NNN.NN"),
				 DateToString (cuph_rec.date_cheq),
				 cuph_rec.bank_code,
				 cuph_rec.branch_code);
		Dsp_saverec (disp_str);
		cc = find_rec (cuph, &cuph_rec, PREVIOUS, "r");
	}
}

/*
 * Display note pad information. 
 */
int
NotesDisplay (void)
{
	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, 14, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("    Contact Name    |   Date   |                                   Comments.                                    ");
	Dsp_saverec ("");
	Dsp_saverec (" [NEXT]  [PREV]  [EDIT/END]");

	NoteDisplay (cumr_rec.hhcu_hash);
	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		NoteDisplay (cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

/*
 * Display note pad information. 
 */
void
NoteDisplay (
	long hhcuHash)
{
	cucc_rec.hhcu_hash = hhcuHash;
	cucc_rec.record_no = 0;
	cc = find_rec (cucc, &cucc_rec, GTEQ,"r");
	while (!cc && cucc_rec.hhcu_hash == hhcuHash)
	{
		sprintf (disp_str, "%-20.20s^E%-10.10s^E%-80.80s",
				cucc_rec.con_person,
				DateToString (cucc_rec.cont_date),
				cucc_rec.comment);

		Dsp_saverec (disp_str);
		cc = find_rec (cucc, &cucc_rec, NEXT,"r");
	}
}

int
NotesAdd (
 void)
{
	init_vars (2);

#ifndef GVISION
	box (5,4,123,12);
#endif

	clearOK = FALSE;

	scn_set (2);
	GetDetails ();
	scn_write (2);
	scn_display (2);
	edit (2);
	
	UpdateNotes ();
	scn_set (1);
	clearOK = TRUE;
	Redraw ();
    return (EXIT_SUCCESS);
}

void
GetChequeData (
 int		clearTotals,
 long		hhcuHash)
{
	if (clearTotals)
	{
		cheq_cnt = 0;
		dtls_cnt = 0;
	}
	cuhd_rec.hhcu_hash	=	hhcuHash;
	strcpy (cuhd_rec.receipt_no, "        ");
	cuhd_rec.index_date	=	0L;

	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
	{
		if (!LoadDetails ())
			return;

		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
	return;
}

int
LoadDetails (
 void)
{
	if (!ArrChkLimit (&cheq_d, cheq, cheq_cnt))
		sys_err ("ArrChkLimit (cheq)", ENOMEM, PNAME);

	strcpy (cheq [cheq_cnt].no,				cuhd_rec.receipt_no);
	sprintf (cheq [cheq_cnt].app,"%-3.3s",	cuhd_rec.narrative);
	strcpy (cheq [cheq_cnt].bank_id, 		cuhd_rec.bank_id);
	strcpy (cheq [cheq_cnt].type, 			cuhd_rec.type);
	strcpy (cheq [cheq_cnt].cheque, 		cuhd_rec.cheque_no);
	strcpy (cheq [cheq_cnt].OR, 			cuhd_rec.or_no);
	strcpy (cheq [cheq_cnt].alt_drawer, 	cuhd_rec.alt_drawer);
	strcpy (cheq [cheq_cnt].datec, DateToString (cuhd_rec.date_payment));
	strcpy (cheq [cheq_cnt].datep, DateToString (cuhd_rec.date_posted));
	strcpy (cheq [cheq_cnt].dated, DateToString (cuhd_rec.due_date));
	if (cuhd_rec.due_date == 0L)
		strcpy (cheq [cheq_cnt].dated,  "          ");

	cheq [cheq_cnt].bank_amt	=	cuhd_rec.bank_amt;
	cheq [cheq_cnt].bank_exch	=	cuhd_rec.bank_exch;
	cheq [cheq_cnt].bank_chg	=	cuhd_rec.bank_chg;
	cheq [cheq_cnt].amount 		= 	cuhd_rec.tot_amt_paid;
	cheq [cheq_cnt].loc_amt 	= 	cuhd_rec.loc_amt_paid;
	cheq [cheq_cnt].cdisc 		= 	cuhd_rec.disc_given;
	cheq [cheq_cnt].loc_dis 	= 	cuhd_rec.loc_disc_give;
	cheq [cheq_cnt].ex_var 		= 	cuhd_rec.exch_variance;

	cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	cc = find_rec (cudt,&cudt_rec,GTEQ,"r");
	while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	{
		if (!ArrChkLimit (&dtls_d, dtls, dtls_cnt))
			sys_err ("ArrChkLimit (dtls)", ENOMEM, PNAME);

		dtls [dtls_cnt].fwd_payment = FALSE;

		if (cuhd_rec.date_payment > monthEndDate)
			dtls [dtls_cnt].fwd_payment = TRUE;

		if (cuhd_rec.due_date > monthEndDate)
			dtls [dtls_cnt].fwd_payment = TRUE;

		dtls [dtls_cnt].hhci_hash 		= cudt_rec.hhci_hash;
		dtls [dtls_cnt].inv_fgn_amt 	= cudt_rec.amt_paid_inv;
		dtls [dtls_cnt].inv_loc_amt		= cudt_rec.loc_paid_inv;
		dtls [dtls_cnt].inv_ex_var 		= cudt_rec.exch_variatio;
		dtls [dtls_cnt].inv_exch_rate 	= cudt_rec.exch_rate;
		dtls [dtls_cnt].cheq_hash 		= cheq_cnt;
		strcpy (dtls [dtls_cnt].type, cuhd_rec.type);
		sprintf (dtls [dtls_cnt].app,"%-3.3s",  cuhd_rec.narrative);
		++dtls_cnt;
		cc = find_rec (cudt,&cudt_rec,NEXT,"r");
	}
	++cheq_cnt;
	return (EXIT_FAILURE);
}

/*
 * Display Customer Direct Deliveries. 
 */
int
DirectDeliveryDisplay (void)
{

	orderDisplay = FALSE;

	open_rec (pohr,pohr_list,POHR_NO_FIELDS,"pohr_hhpo_hash");
	open_rec (poln,poln_list,POLN_NO_FIELDS,"poln_hhpl_hash");
	open_rec (ddhr,ddhr_list,DDHR_NO_FIELDS,"ddhr_hhcu_hash");
	open_rec (ddln,ddln_list,DDLN_NO_FIELDS,"ddln_id_no");

	linePrinted = FALSE;
	tot_qty = 0.00;
	tot_amt = 0.00;

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_prn_open (0, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec (" ORDER   |BR|  ORDER   |   DUE    |  ORDER   |      ITEM       |       ITEM DESCRIPTION         |S|   AMOUNT    |PURCHASE ORDER ");
	Dsp_saverec (" NUMBER  |NO|  DATE.   |   DATE.  |   QTY.   |     NUMBER      |                                | |             |     NUMBER    ");

	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");

	ddhr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");
	while (!cc && ddhr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		linePrinted = FALSE;

		DdGetHeader ();

		if (linePrinted)
		{
			sprintf 
			(
				disp_str, 
				 "%-22.22s%-12.12s^E%10.2f^E                 ^E                                ^E ^E%12.12s ^E",
					 " ",
				 mlCcDisp [99],
				 ord_qty,
				 CF (ord_amt, "N,NNN,NNN.NN")
			 );
			Dsp_saverec (disp_str);

			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGEGGGGGGGGGGGGGEGGGGGGGGGGGGGGG");
		}
		cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
	}

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		ddhr_rec.hhcu_hash = cumr2_rec.hhcu_hash;
		cc = find_rec (ddhr, &ddhr_rec, GTEQ, "r");
		while (!cc && ddhr_rec.hhcu_hash == cumr2_rec.hhcu_hash)
		{
			linePrinted = FALSE;
			DdGetHeader ();
			if (linePrinted)
			{
				sprintf 
				(
					disp_str, 
					 "%-22.22s%-12.12s^E%10.2f^E                 ^E                                ^E ^E%12.12s ^E",
						 " ",
					 mlCcDisp [99],
					 ord_qty,
					 CF (ord_amt, "N,NNN,NNN.NN")
				 );
				Dsp_saverec (disp_str);

				Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGEGGGGGGGGGGGGGEGGGGGGGGGGGGGGG");
			}
			cc = find_rec (ddhr, &ddhr_rec, NEXT, "r");
		}
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	sprintf 
	(
		disp_str, 
		 "%-19.19s%-15.15s^E%10.2f^E                 ^E                                ^E ^E%12.12s ^E",
			 " ",
		 mlCcDisp [98],
		 tot_qty,
		 CF (tot_amt, "N,NNN,NNN.NN")
	 );
	Dsp_saverec (disp_str);

	Dsp_srch ();

	Dsp_close ();
	abc_fclose (pohr);
	abc_fclose (poln);
	Redraw ();
    return (EXIT_SUCCESS);
}

int
DdGetHeader (void)
{
	int		first_line = TRUE;

	ord_qty = 0.00;
	ord_amt = 0.00;

	ddln_rec.hhdd_hash 	= ddhr_rec.hhdd_hash;
	ddln_rec.line_no 	= 0;
	cc = find_rec (ddln,&ddln_rec,GTEQ,"r");
	while (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash)
	{
		inmr_rec.hhbr_hash	=	ddln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (!cc && (inmr_rec.inmr_class [0] == 'Z' || ddln_rec.q_order > 0.00))
		{
			if (DdGetLines (first_line))
				return (EXIT_FAILURE);

			first_line 	= FALSE;
			linePrinted = TRUE;
		}
		cc = find_rec (ddln, &ddln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
DdGetLines (
	int 	first_line)
{
	char	bal_value [13];

	if (FindPohr (ddln_rec.hhpl_hash))
		return (EXIT_FAILURE);

	CalcDdBalance ();

	ord_qty += ddln_rec.q_order;

	if (ddln_rec.bonus_flag [0] == 'Y')
		sprintf (bal_value,"%-12.12s","BONUS ITEM");
	else
	{
		ord_amt += wk_bal;
		sprintf (bal_value,"%12.12s", CF (wk_bal,"N,NNN,NNN.NN"));
	}
	sprintf 
	(
		disp_str,
		 " %-8.8s^E%-2.2s^E%-10.10s^E%-10.10s^E%10.2f^E %-16.16s^E%-32.32s^E%-1.1s^E%-12.12s ^E%s",
		 ddhr_rec.order_no,
		 ddhr_rec.br_no,
		 date_ord,
		 date_due,
		 ddln_rec.q_order,
		 inmr_rec.item_no,
		 ddln_rec.item_desc,
		 pohr_rec.status,
		 bal_value,
		 pohr_rec.pur_ord_no
	);
	cc = Dsp_saverec (disp_str);
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Display Customer Collection Notes. 
 */
int
CollectionNoteDisplay (void)
{
	abc_selfield (cohr, "cohr_hhcu_hash");
	abc_selfield (coln, "coln_id_no");

	linePrinted = FALSE;
	tot_qty = 0.00;
	tot_amt = 0.00;

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_prn_open (0, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("  CCN    |BR|   CCN    |   DUE    |   CCN    |UOM.|      ITEM      |  I T E M     D E S C R I P T I O N | |   AMOUNT    ");
	Dsp_saverec (" NUMBER  |NO|  DATE.   |   DATE.  |   QTY.   |    |     NUMBER     |                                    | |             ");

	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");

	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && cohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if (cohr_rec.type [0] != 'N')
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}	
		if (FindColn ())
			break;

		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	cumr2_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		cohr_rec.hhcu_hash = cumr2_rec.hhcu_hash;
		cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
		while (!cc && cohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			if (cohr_rec.type [0] != 'N')
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}	
			if (FindColn ())
				break;

			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
		}
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	if (linePrinted)
		PrintGrandTotals (TRUE);

	Dsp_srch_fn (InvoiceEnquiry);

	Dsp_close ();
	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (coln, "coln_hhsl_hash");
	Redraw ();
    return (EXIT_SUCCESS);
}

/*
 * Display Customer Orders. 
 */
void
OrderProcess (
 int back_ord)
{
	open_rec (sohr,sohr_list,SOHR_NO_FIELDS,"sohr_hhcu_hash");
	open_rec (soln,soln_list,SOLN_NO_FIELDS,"soln_id_no");

	linePrinted = FALSE;
	tot_qty = 0.00;
	tot_amt = 0.00;

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_prn_open (0, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec (" ORDER   |BR|  ORDER   |   DUE    |  ORDER   |UOM.|      ITEM      |  I T E M     D E S C R I P T I O N |S|   AMOUNT    | P/SLIP ");
	Dsp_saverec (" NUMBER  |NO|  DATE.   |   DATE.  |   QTY.   |    |     NUMBER     |                                    | |             |        ");

	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");

	sohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if (FindSoln (back_ord))
			break;

		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		sohr_rec.hhcu_hash	=	cumr2_rec.hhcu_hash;
		cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
		while (!cc && sohr_rec.hhcu_hash == cumr2_rec.hhcu_hash)
		{
			if (FindSoln (back_ord))
				break;

			cc = find_rec (sohr, &sohr_rec, NEXT,"r");
		}
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}
	if (linePrinted)
		PrintGrandTotals (TRUE);

	Dsp_srch_fn (InvoiceEnquiry);

	Dsp_close ();
	abc_fclose (sohr);
	abc_fclose (soln);
}

void
CalcInvoiceBalance (void)
{
	sprintf (date_ord, "%-10.10s",DateToString (cohr_rec.date_raised));
	sprintf (date_due, "%-10.10s",DateToString (coln_rec.due_date));

	if (coln_rec.bonus_flag [0] != 'Y')
	{
		if (envVarDbNettUsed)
			wk_bal	=	coln_rec.gross - coln_rec.amt_disc;
		else
			wk_bal	=	coln_rec.gross;

		if (envVarRepTax)
			wk_bal	+=	coln_rec.amt_gst + coln_rec.amt_tax;

		wk_bal	+=	coln_rec.item_levy;
	}
	tot_amt += wk_bal;
	tot_qty += coln_rec.q_order;
}

void
CalcDdBalance (void)
{
	sprintf (date_ord, "%-10.10s",DateToString (ddhr_rec.dt_raised));
	sprintf (date_due, "%-10.10s",DateToString (ddln_rec.due_date));

	if (ddln_rec.bonus_flag [0] != 'Y')
	{
		if (envVarDbNettUsed)
			wk_bal	=	ddln_rec.gross - ddln_rec.amt_disc;
		else
			wk_bal	=	ddln_rec.gross;

		if (envVarRepTax)
			wk_bal	+=	ddln_rec.amt_gst + ddln_rec.amt_tax;

	}
	tot_amt += wk_bal;
	tot_qty += ddln_rec.q_order;
}

void
CalcBalance (
	int		bord)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;

	sprintf (date_ord, "%-10.10s", DateToString (sohr_rec.dt_raised));
	sprintf (date_due, "%-10.10s", DateToString (soln_rec.due_date));

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		if (bord && soln_rec.status [0] != 'B')
			l_total	=	(double) soln_rec.qty_bord;
		else
			l_total	=	(double) soln_rec.qty_order + soln_rec.qty_bord;

		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);
			
		if (envVarDbNettUsed)
			wk_bal	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			wk_bal	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		tot_amt += wk_bal;
	}
	if (bord && soln_rec.status [0] != 'B')
		tot_qty += soln_rec.qty_bord;
	else
		tot_qty += soln_rec.qty_order + soln_rec.qty_bord;
}

void
PrintGrandTotals (
 int key_total)
{
	
	if (orderDisplay)
	{
		if (key_total)
		{
			/*-------------------
			| Customer Totals	|
			-------------------*/
			sprintf (disp_str, 
					 "%-19.19s%-14.14s ^E%10.2f^E    ^E                ^E                                    ^E ^E%12.12s ^E        ^E",
					 " ",
					 mlCcDisp [98],
					 tot_qty,
					 CF (tot_amt, "N,NNN,NNN.NN"));
			Dsp_saverec (disp_str);
		}
		else
		{
			/*---------------
			| Order Totals	|
			---------------*/
			sprintf (disp_str, 
					 "%-22.22s%-12.12s^E%10.2f^E    ^E                ^E                                    ^E ^E%12.12s ^E        ^E",
					 " ",
					 mlCcDisp [99],
					 ord_qty,
					 CF (ord_amt, "N,NNN,NNN.NN"));
			Dsp_saverec (disp_str);


			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGG^EGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGEGGGGGGGGGGGGGEGGGGGGGG");
		}
	}
	else
	{
		if (key_total)
		{
			/*-------------------
			| Customer Totals	|
			-------------------*/
			sprintf (disp_str, 
					 "%-19.19s%-14.14s ^E%10.2f^E    ^E                ^E                                    ^E ^E%12.12s ^E        ^E",
					 " ",
					 mlCcDisp [98],
					 tot_qty,
					 CF (tot_amt, "N,NNN,NNN.NN"));
			Dsp_saverec (disp_str);
		}
		else
		{
			/*---------------
			| Order Totals	|
			---------------*/
			sprintf 
			(
				disp_str, 
				 "%-22.22s%-12.12s^E%10.2f^E    ^E                ^E                                    ^E ^E%12.12s ^E        ^E",
					 " ",
				 mlCcDisp [99],
				 ord_qty,
				 CF (ord_amt, "N,NNN,NNN.NN")
			 );
			Dsp_saverec (disp_str);

			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGEGGGGGGGGGGGGGEGGGGGGGG");
		}
	}
}

int
FindColn (
 void)
{
	int		first_line = 1;

	ord_qty = 0.00;
	ord_amt = 0.00;

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = 0;
	cc = find_rec (coln,&coln_rec,GTEQ,"r");

	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");

		if (!cc && (inmr_rec.inmr_class [0] == 'Z' || coln_rec.q_order > 0.00))
		{
			if (ColnCCN (first_line))
				return (EXIT_FAILURE);

			first_line = 0;
			linePrinted = TRUE;
		}
		cc = find_rec (coln,&coln_rec,NEXT,"r");
	}
	if (!first_line)
		PrintGrandTotals (FALSE);

	return (EXIT_SUCCESS);
}

int
FindPohr (
	long	hhplHash)
{
	/*-----------------------------------------
	| Find poln record for current hhcl_hash. |
	-----------------------------------------*/
	poln_rec.hhpl_hash	=	hhplHash;
 	cc = find_rec (poln, &poln_rec, EQUAL, "r");
	if (cc)
		return (EXIT_FAILURE);

	pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
 	return (find_rec (pohr, &pohr_rec, EQUAL, "r"));
}

int
ColnCCN (
 int first_line)
{
	char	bal_value [13];
	float	StdCnvFct 	= 1.00,
			CnvFct		= 1.00,
			DspQty		= 1.00;

	CalcInvoiceBalance ();

	ord_qty += coln_rec.q_order;

	if (coln_rec.bonus_flag [0] == 'Y')
		sprintf (bal_value,"%-12.12s","BONUS ITEM");
	else
	{
		ord_amt += wk_bal;
		sprintf (bal_value,"%12.12s",CF (wk_bal,"N,NNN,NNN.NN"));
	}
	sprintf (save_key, 
			 "%04d%2.2s%1.1s%-8.8s%06ld",
			 saveIndex++,
			 cohr_rec.br_no, 
			 "4",
			 cohr_rec.inv_no,
			 cohr_rec.hhcu_hash);

	inum_rec.hhum_hash 	= 	inmr_rec.std_uom;  
	cc = find_rec ("inum", &inum_rec, EQUAL, "r"); 
	if (cc)
		file_err (cc, "inum", "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;
	if (inum_rec.hhum_hash != coln_rec.hhum_hash)
	{
		inum_rec.hhum_hash = coln_rec.hhum_hash;  
		cc = find_rec ("inum", &inum_rec, EQUAL, "r"); 
	}
	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}
	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
	DspQty	= 	coln_rec.q_order / CnvFct;
	
	if (first_line)
	{
		sprintf (disp_str,
				 " %-8.8s^E%-2.2s^E%-10.10s^E%-10.10s^E%10.2f^E%4.4s^E%-16.16s^E%-36.36s^E ^E%-12.12s ",
				 cohr_rec.inv_no,
				 cohr_rec.br_no,
				 date_ord,
				 date_due,
				 DspQty,
				 inum_rec.uom,
				 inmr_rec.item_no,
				 coln_rec.item_desc,
				 bal_value);
	}
	else
	{
		sprintf (disp_str,
				 "                       ^E%-10.10s^E%10.2f^E%4.4s^E%-16.16s^E%-36.36s^E ^E%-12.12s ",
				 date_due,
				 DspQty,
				 inum_rec.uom,
				 inmr_rec.item_no,
				 coln_rec.item_desc,
				 bal_value);
	}

	if (first_line)
		cc = Dsp_save_fn (disp_str, save_key);
	else
		cc = Dsp_saverec (disp_str);
	if (cc)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int
FindSoln (
 int bord)
{
	int		first_line = 1;

	ord_qty = 0.00;
	ord_amt = 0.00;

	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");

	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		if (bord == TRUE  && soln_rec.status [0] != 'B' && soln_rec.qty_bord <= 0.00)
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}

		if (bord == TRUE  && soln_rec.status [0] == 'B' && (soln_rec.qty_bord + soln_rec.qty_order) <= 0.00)
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}

		if (bord == FALSE && soln_rec.status [0] == 'B')
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}

		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (!cc && (inmr_rec.inmr_class [0] == 'Z' || 
		(soln_rec.qty_order + soln_rec.qty_bord) > 0.00))
		{
			if (ProcessSolnLines (first_line, bord))
				return (EXIT_FAILURE);

			first_line = 0;
			linePrinted = TRUE;
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	if (!first_line)
		PrintGrandTotals (FALSE);
	return (EXIT_SUCCESS);
}

int
ProcessSolnLines (
 int first_line,
 int bord)
{
	char	pslip_no [9];
	char	bal_value [13];
	char	star [2];

	float	StdCnvFct 	= 1.00,
			CnvFct		= 1.00,
			DspQty		= 1.00,
			BDspQty		= 1.00;

	CalcBalance (bord);

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");
	
	StdCnvFct = inum_rec.cnv_fct;

	if (inum_rec.hhum_hash != soln_rec.hhum_hash)
	{
		inum_rec.hhum_hash = soln_rec.hhum_hash;  
		cc = find_rec ("inum", &inum_rec, EQUAL, "r"); 
	}
	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}
	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
	if (bord && soln_rec.status [0] != 'B')
	{
		BDspQty	=	soln_rec.qty_bord / CnvFct;
		ord_qty	+= 	BDspQty;
	}
	else
	{
		DspQty	=	(soln_rec.qty_order + soln_rec.qty_bord) / CnvFct;
		ord_qty	+= 	DspQty;
	}


	if (soln_rec.bonus_flag [0] == 'Y')
		sprintf (bal_value,"%-12.12s","BONUS ITEM");
	else
	{
		ord_amt += wk_bal;
		sprintf (bal_value,"%12.12s",
				CF (wk_bal, "N,NNN,NNN.NN"));
	}

	if (sohr_rec.full_supply [0] == 'Y')
		strcpy (star, "*");
	else
		strcpy (star, " ");

	if (soln_rec.status [0] == 'P' || soln_rec.status [0] == 'B')
	{
		cc = FindPackingSlip (soln_rec.hhsl_hash);
		sprintf (pslip_no,
				 "%-8.8s",
				(cc) ? mlCcDisp [24] : cohr_rec.inv_no);
	}
	else
		sprintf (pslip_no, "%-8.8s", mlCcDisp [24]);

	sprintf (save_key, 
			 "%04d%2.2s%1.1s%-8.8s%06ld",
			 saveIndex++,
			 sohr_rec.br_no, 
			 soln_rec.status,
			 sohr_rec.order_no,
			 sohr_rec.hhcu_hash);

	if (first_line)
	{
		sprintf (disp_str,
				 "%s%-8.8s^E%-2.2s^E%-10.10s^E%-10.10s^E%10.2f^E%4.4s^E%-16.16s^E%-36.36s^E%-1.1s^E%-12.12s ^E%s",
				 star,
				 sohr_rec.order_no,
				 sohr_rec.br_no,
				 date_ord,
				 date_due,
				(bord && soln_rec.status [0] != 'B')? BDspQty : DspQty,
				 inum_rec.uom,
				 inmr_rec.item_no,
				 soln_rec.item_desc,
				(bord && soln_rec.status [0] != 'B')? "B" : soln_rec.status,
				 bal_value,
				 pslip_no);
	}
	else
	{
		sprintf (disp_str,
				  "                       ^E%-10.10s^E%10.2f^E%4.4s^E%-16.16s^E%-36.36s^E%-1.1s^E%-12.12s ^E%s",
				 date_due,
				(bord && soln_rec.status [0] != 'B')? BDspQty : DspQty,
				 inum_rec.uom,
				 inmr_rec.item_no,
				 soln_rec.item_desc,
				(bord && soln_rec.status [0] != 'B')? "B" : soln_rec.status,
				 bal_value,
				 pslip_no);
	}

	if (first_line)
		cc = Dsp_save_fn (disp_str, save_key);
	else
		cc = Dsp_saverec (disp_str);
	if (cc)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int
FindPackingSlip (
	long	hhslHash)
{
	/*-----------------------------------------
	| Find coln record for current hhsl_hash. |
	-----------------------------------------*/
	coln_rec.hhsl_hash	=	hhslHash;
 	cc = find_rec (coln, &coln_rec,COMPARISON,"r");
	if (cc)
		return (cc);

	/*-------------------------------------------------------------
	| Cohr record is already found so don't bother to find again. |
	-------------------------------------------------------------*/
	if (coln_rec.hhco_hash == cohr_rec.hhco_hash)
		return (EXIT_SUCCESS);

	cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
 	return (find_rec (cohr,&cohr_rec,COMPARISON,"r"));
}

/*
 * Master File. 
 */
int
MasterDisplay (void)
{
	char	tax_desc [28];

	if (masterfile_open)
		Dsp_close ();

	if (total_open)
	{
		Dsp_close ();
		total_open = FALSE;
	}

	masterfile_open = TRUE;

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	window_cnt = 2;

	Dsp_saverec ("                                           M A S T E R    F I L E   D E T A I L S                                             ");

	Dsp_saverec ("");
	Dsp_saverec ("");

	sprintf (disp_str, 
			 " %-20.20s     : %12.12s %35.35s  %-25.25s  : %12.12s",
			 mlCcDisp [47],
			(cumr_rec.bo_flag [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50],
			" ",
			 mlCcDisp [48],
			(cumr_rec.bo_cons [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50]);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-25.25s:          %3d %35.35s  %-15.15s            : %12.12s",
			 mlCcDisp [51],
			 cumr_rec.bo_days,
			 " ",
			 mlCcDisp [52],
			(cumr_rec.acc_type [0] == 'O') ? mlCcDisp [53] : mlCcDisp [54]);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-25.25s: %12.12s %35.35s  %-25.25s  : %12.12s",
			 mlCcDisp [55],
			(cumr_rec.po_flag [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50],
			" ",
			 mlCcDisp [56],
			(cumr_rec.ho_dbt_hash > 0L) ? mlCcDisp [49] : mlCcDisp [50]);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-20.20s     : %12.12s %35.35s  %-15.15s            : %12.12s",
			 mlCcDisp [57],
			(cumr_rec.stmnt_flg [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50],
			 " ",
			 mlCcDisp [58],
			(cumr_rec.stmt_type [0] == 'O') ? mlCcDisp [53] : mlCcDisp [54]);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-20.20s     : %12.12s %35.35s  %-25.25s  : %12.12s",
			 mlCcDisp [59],
			(cumr_rec.int_flag [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50],
			 " ",
			 mlCcDisp [60],
			(cumr_rec.sur_flag [0] == 'Y') ? mlCcDisp [49] : mlCcDisp [50]);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-20.20s     : %12.12s %35.35s  %-25.25s  : %20.20s",
			 mlCcDisp [61],
			 cumr_rec.bank_code, 
			 " ",
			 mlCcDisp [62],
			 cumr_rec.branch_code);
	Dsp_saverec (disp_str);

	strcpy (tax_desc,"???????????????????????????");

	if (cumr_rec.tax_code [0] == 'A')
		sprintf (tax_desc, "%-27.27s", mlCcDisp [63]);

	if (cumr_rec.tax_code [0] == 'B')
		sprintf (tax_desc, "%-27.27s", mlCcDisp [64]);

	if (cumr_rec.tax_code [0] == 'C')
		sprintf (tax_desc, "%-27.27s", mlCcDisp [65]);

	if (cumr_rec.tax_code [0] == 'D')
		sprintf (tax_desc, "%-27.27s", mlCcDisp [66]);

	strcpy (exdf_rec.co_no,comm_rec.co_no);
	strcpy (exdf_rec.code,cumr_rec.disc_code);
	cc = find_rec (exdf,&exdf_rec,COMPARISON,"r");
	if (cc)
		exdf_rec.disc_pc = 0.0;

	sprintf (disp_str, 
			 " %-25.25s:    %1.1s %6.2f%% %35.35s  %-15.15s%27.27s",
			 mlCcDisp [67],
			 cumr_rec.disc_code,
			 exdf_rec.disc_pc,
			 " ",
			 mlCcDisp [68],
			 tax_desc);

	Dsp_saverec (disp_str);

	if (cumr_rec.hhsu_hash != 0L)
	{
		open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
		sumr_rec.hhsu_hash = cumr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (disp_str, 
					 " %-20.20s     :    %6.6s (%s)",
					 mlCcDisp [69],
					 sumr_rec.crd_no,
					 sumr_rec.crd_name);
			Dsp_saverec (disp_str);
		}
	}

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	abc_selfield (cumr2, "cumr_hhcu_hash");

	if (cumr_rec.ho_dbt_hash != 0L)
	{
		cumr2_rec.hhcu_hash	=	cumr_rec.ho_dbt_hash;
		cc = find_rec (cumr2, &cumr2_rec, EQUAL, "r");
		sprintf 
		(
			disp_str, 
			 " %-20.20s     : %6.6s (%-40.40s)",
			 mlCcDisp [70],
			(cc) ? " " : cumr2_rec.dbt_no,
			(cc) ? mlCcDisp [71] : cumr2_rec.dbt_name
		 );
		Dsp_saverec (disp_str);
	}

	abc_selfield (cumr2, "cumr_ho_dbt_hash");

	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,cumr_rec.sman_code);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exsf_rec.salesman,"%-40.40s"," ");

	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,cumr_rec.class_type);
	cc = find_rec (excl,&excl_rec,COMPARISON,"r");
	if (cc)
		sprintf (excl_rec.class_desc,"%-40.40s"," ");

	strcpy (dbry_rec.co_no,comm_rec.co_no);
	strcpy (dbry_rec.cr_type,cumr_rec.roy_type);
	cc = find_rec (dbry,&dbry_rec,COMPARISON,"r");
	if (cc)
		sprintf (dbry_rec.desc,"%-40.40s"," ");

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,cumr_rec.area_code);
	cc = find_rec (exaf,&exaf_rec,COMPARISON,"r");
	if (cc)
		sprintf (exaf_rec.area,"%-40.40s"," ");

	sprintf (disp_str, 
			 " %-20.20s     : %6.6s (%40.40s)",
			 mlCcDisp [72],
			 cumr_rec.area_code, 
			 exaf_rec.area);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-25.25s: %6.6s (%40.40s)",
			 mlCcDisp [73],
			 cumr_rec.sman_code, 
			 exsf_rec.salesman);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-25.25s: %6.6s (%40.40s)",
			 mlCcDisp [74],
			 cumr_rec.class_type, 
			 excl_rec.class_desc);
	Dsp_saverec (disp_str);

	sprintf (disp_str, 
			 " %-25.25s: %6.6s (%40.40s)",
			 mlCcDisp [75],
			 cumr_rec.roy_type, 
			 dbry_rec.desc);
	Dsp_saverec (disp_str);

	Dsp_srch ();
	return (EXIT_SUCCESS);
}

/*
 * Last Customer Pricing. 
 */
int
LastPrice (void)
{
	open_rec (tshs, tshs_list, TSHS_NO_FIELDS, "tshs_id_no2");

	lp_x_off = 0;
	lp_y_off = 3;
	Dsp_nc_prn_open (0, 3, 13, 
					 head_str, 
					 comm_rec.co_no, 
					 comm_rec.co_name, 
					 comm_rec.est_no, 
					 comm_rec.est_name, 
					(char *) 0, 
					(char *) 0);

	sprintf (err_str,"    SALE    |      ITEM      |            ITEM DESCRIPTION            |   SALE     |     SALE     | DISCOUNT   |    PROFIT    ");
	Dsp_saverec (err_str);
	sprintf (err_str,"    DATE    |     NUMBER     |                                        |  QUANTITY  |    PRICE.    |   AMOUNT   |    AMOUNT    ");
	Dsp_saverec (err_str);
				
	Dsp_saverec (std_foot);

	tshs_rec.hhcu_hash	=	cumr_rec.hhcu_hash + 1;
	tshs_rec.hhbr_hash	=	0L;
	tshs_rec.date		=	0L;

	cc = find_rec (tshs, &tshs_rec, GTEQ, "r");
	if (cc)
		cc = find_rec (tshs, &tshs_rec, LAST, "r");
	else
		cc = find_rec (tshs, &tshs_rec, LT, "r");
	
	while (!cc && tshs_rec.hhcu_hash	==	cumr_rec.hhcu_hash)
	{
		inmr_rec.hhbr_hash	=	tshs_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (tshs, &tshs_rec, PREVIOUS, "r");
			continue;
		}
		sprintf (err_str," %10.10s ^E%-16.16s^E%-40.40s^E%11.2f ^E%13.2f ^E%11.2f ^E%13.2f ",
			DateToString (tshs_rec.date),
			inmr_rec.item_no,
			inmr_rec.description,
			tshs_rec.qty,
			DOLLARS (tshs_rec.sale_price),
			DOLLARS (tshs_rec.disc),
			DOLLARS ((tshs_rec.sale_price - tshs_rec.disc) - tshs_rec.cost_price));

		Dsp_saverec (err_str);

		cc = find_rec (tshs, &tshs_rec, PREVIOUS, "r");
	}
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG ^^ ^1END OF LAST CUSTOMER PRICES^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (tshs);

	Redraw ();
    return (EXIT_SUCCESS);
}

/*
 * Special Pricing. 
 */
int
SpecialPrice (void)
{
	int		i;
	int		prt_comm = TRUE;
	char	date_from [11];
	char	date_to [11];
	char	use_curr [4];
	char	incpKey [7];
	char	priceDesc [16];

	open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_id_no");

	i	=	atoi (cumr_rec.price_type) - 1;

	switch (cumr_rec.price_type [0])
	{
		case	'1':	
			strcpy (priceDesc, comm_rec.price1_desc);
			break;
		case	'2':	
			strcpy (priceDesc, comm_rec.price2_desc);
			break;
		case	'3':	
			strcpy (priceDesc, comm_rec.price3_desc);
			break;
		case	'4':	
			strcpy (priceDesc, comm_rec.price4_desc);
			break;
		case	'5':	
			strcpy (priceDesc, comm_rec.price5_desc);
			break;
		case	'6':	
			strcpy (priceDesc, comm_rec.price6_desc);
			break;
		case	'7':	
			strcpy (priceDesc, comm_rec.price7_desc);
			break;
		case	'8':	
			strcpy (priceDesc, comm_rec.price8_desc);
			break;
		case	'9':	
			strcpy (priceDesc, comm_rec.price9_desc);
			break;
	}

	lp_x_off = 0;
	lp_y_off = 3;
	Dsp_nc_prn_open (0, 3, 13, 
					 head_str, 
					 comm_rec.co_no, 
					 comm_rec.co_name, 
					 comm_rec.est_no, 
					 comm_rec.est_name, 
					(char *) 0, 
					(char *) 0);

	sprintf (err_str,"   FROM   |    TO    |Co Br Wh|AREA|  ITEM NUMBER.  |           DESCRIPTION             |%-14.14s|        COMMENTS          ",
				priceDesc);
	Dsp_saverec (err_str);

	Dsp_saverec ("   DATE   |   DATE   |        |CODE|                |                                   |PRICE.        |                          ");

	Dsp_saverec (std_foot);

	if (envVarDbMcurr)
		sprintf (use_curr, "%-3.3s", cumr_rec.curr_code);
	else
		sprintf (use_curr, "%-3.3s", envVarCurrCode);

	/* Set up key for Company level */
	sprintf (incpKey, "%2.2s    ", comm_rec.co_no);

	strcpy (incp_rec.key, incpKey);
	strcpy (incp_rec.curr_code, use_curr);
	incp_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (incp_rec.cus_type, "   ");
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = 0L;
	strcpy (incp_rec.status, "A");
	incp_rec.date_from = 0L;

	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc && !strncmp (incp_rec.key, incpKey, 2))
	{
	    if (strcmp (incp_rec.curr_code, use_curr) ||
	        incp_rec.hhcu_hash != cumr_rec.hhcu_hash ||
	        strcmp (incp_rec.cus_type, "   ") ||
	        incp_rec.status [0] != 'A')
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash	=	incp_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}
		if (prt_comm)
			Dsp_saverec (mlCcDisp [76]);

		prt_comm = FALSE;


		if (((strncmp (&incp_rec.key [2], comm_rec.est_no, 2) == 0)  ||
		     (strncmp (&incp_rec.key [2], "  ",             2) == 0)) &&
		    ((strncmp (&incp_rec.key [4], comm_rec.cc_no,  2) == 0)  ||
		     (strncmp (&incp_rec.key [4], "  ",             2) == 0)))
		{
			sprintf (date_from, "%-10.10s", DateToString (incp_rec.date_from));
			if (incp_rec.date_to == 0L)
				strcpy (date_to, "          ");
			else
				sprintf (date_to, "%-10.10s", DateToString (incp_rec.date_to));
	
			sprintf (err_str, "%10.10s^E%10.10s^E%2.2s %2.2s %2.2s^E %2.2s ^E%16.16s^E%-35.35s^E%13.13s ^E%-26.26s",
					date_from,
					date_to,
					incp_rec.key,
					&incp_rec.key [2],
					&incp_rec.key [4],
					incp_rec.area_code,
					inmr_rec.item_no,
					inmr_rec.description,
					CF (incp_price [i], "NN,NNN,NNN.NN"),
					incp_rec.comment);

			Dsp_saverec (err_str);
		}
		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
	if (!prt_comm)
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^ ^1END OF CUSTOMER SPECIALS^6 ^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	prt_comm = TRUE;

	/* Set up key for Company level */
	sprintf (incpKey, "%2.2s    ", comm_rec.co_no);

	strcpy (incp_rec.key, incpKey);
	incp_rec.hhcu_hash = 0L;
	strcpy (incp_rec.curr_code, use_curr);
	strcpy (incp_rec.cus_type, cumr_rec.class_type);
	strcpy (incp_rec.area_code, "  ");
	incp_rec.hhbr_hash = 0L;
	strcpy (incp_rec.status, "A");
	incp_rec.date_from = 0L;
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc && !strncmp (incp_rec.key, incpKey, 2))
	{
	    if (strcmp (incp_rec.curr_code, use_curr) ||
	        incp_rec.hhcu_hash != 0L ||
	        strcmp (incp_rec.cus_type, cumr_rec.class_type) ||
	        strcmp (incp_rec.area_code, "  ") ||
	        incp_rec.status [0] != 'A')
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash	=	incp_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}
		sprintf (err_str, 
				 "(%-14.14s%s)", 
				 mlCcDisp [77],
				 cumr_rec.class_type);
		if (prt_comm)
			Dsp_saverec (err_str);

		prt_comm = FALSE;

		if (((strncmp (&incp_rec.key [2], comm_rec.est_no, 2) == 0)  ||
		     (strncmp (&incp_rec.key [2], "  ",             2) == 0)) &&
		    ((strncmp (&incp_rec.key [4], comm_rec.cc_no,  2) == 0)  ||
		     (strncmp (&incp_rec.key [4], "  ",             2) == 0)))
		{
			sprintf (date_from, "%-10.10s", DateToString (incp_rec.date_from));
			if (incp_rec.date_to == 0L)
				strcpy (date_to, "          ");
			else
				sprintf (date_to, "%-10.10s",  DateToString (incp_rec.date_to));

			sprintf (err_str,"%10.10s^E%10.10s^E%2.2s %2.2s %2.2s^E%16.16s^E%-40.40s^E%13.13s ^E%-26.26s",
					date_from,
					date_to,
					incp_rec.key,
					&incp_rec.key [2],
					&incp_rec.key [4],
					inmr_rec.item_no,
					inmr_rec.description,
					CF (incp_price [i], "NN,NNN,NNN.NN"),
					incp_rec.comment);
	
			Dsp_saverec (err_str);
		}
		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
	if (!prt_comm)
	{
		sprintf (err_str,
				 "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^%-30.30s^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",
				 mlCcDisp [78]);
		Dsp_saverec (err_str);
	}

	Dsp_srch ();
	Dsp_close ();
	abc_fclose (incp);

	Redraw ();
    return (EXIT_SUCCESS);
}

void
AlternateMess (
 char *str)
{
	move (0,error_line);
	cl_line ();
	rv_pr (str,1,error_line,1);
#ifdef GVISION
	/*---------------------------------------------
	| Added to remove the text written by rv_pr () |
	---------------------------------------------*/
	move (1, error_line);
	cl_line ();
#endif	/* GVISION */
}

/*
 * Calculate Ytd Sales. 
 */
float	
CalculateYtd (void)
{
	int		i;
	float	ytd = 0.00;

	/*
	 * No fiscal set up	
	 */
	if (comm_rec.fiscal == 0)
	{
		/*-----------------------------------
		| sum to current month (feb == 1)	|
		-----------------------------------*/
		for (i = 0;i <= currentMonth;i++)
			ytd += (float) (cusa_val [i]);

		return (ytd);
	}

	/*-----------------------------------
	| need to sum from fiscal to dec,	|
	| then jan to current month.		|
	-----------------------------------*/
	if (currentMonth < comm_rec.fiscal)
	{
		for (i = comm_rec.fiscal;i < 12;i++)
			ytd += (float) (cusa_val [i]);

		for (i = 0;i <= currentMonth;i++)
			ytd += (float) (cusa_val [i]);

		return (ytd);
	}

	for (i = comm_rec.fiscal;i <= currentMonth;i++)
		ytd += (float) (cusa_val [i]);

	return (ytd);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (clearOK)
			FLD ("dbt_no") = YES;
		else
			FLD ("dbt_no") = ND;

		scn_set (scn);

		if (clearOK)
		{
			swide ();
			clear ();
		}


		if (scn == 2)
		{
			MainDisplayScreen ();
			box (5,4,122,12);
		}

		if (scn == 1)
		{
			if (masterfile_open)
			{
				Dsp_close ();
				masterfile_open = FALSE;
			}

			if (total_open)
			{
				Dsp_close ();
				total_open = FALSE;
			}

			if (main_open)
			{
				Dsp_close ();
				main_open = FALSE;
			}

			rv_pr (mlCcDisp [79],35,0,1);

			move (108,0);
			printf ("%-13.13s: %s",mlCcDisp [80], local_rec.prev_dbt);

			if (!displayOK)
				box (0,1,130,1);

			move (0,22);
			line (130);
			cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cucc_rec.record_no = 0;
			cc = find_rec (cucc, &cucc_rec, GTEQ, "r");
			if (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
			{
				sprintf (err_str, "*** %-38.38s ***", mlCcDisp [81]);
				rv_pr (err_str, 36,22,1);
			}

			print_at (23, 0,
					  ML (mlStdMess038),
					  comm_rec.co_no,
					  comm_rec.co_short);

			print_at (23, 55,
					  ML (mlStdMess039),
					  comm_rec.est_no,
					  comm_rec.est_short);

			line_cnt = 0;
		}
		if (scn == 4)
		{
			print_at (11,44,"%31.31s", " ");
			print_at (12,44,"%31.31s", " ");
			print_at (13,44,"%31.31s", " ");
			print_at (14,44,"%31.31s", " ");
			if (!HAS_HO)
				print_at (15,44,"%31.31s", " ");
			box (44,11,32, (HAS_HO) ? 2 : 3);
		}

		scn_write (scn);
	}

	if (displayOK && scn != 2 && scn != 4)
	{
		MainDisplayScreen ();
		clearOK = FALSE;
	}
    return (EXIT_SUCCESS);
}

int
Dsp_heading (
 void)
{
	int		main_check 		 = main_open;
	int		masterfile_check = masterfile_open;
	int		total_check		 = total_open;

	main_open 		= FALSE;
	masterfile_open = FALSE;
	total_open 		= FALSE;

	Redraw ();

	if (main_check)
		Dsp_close ();

	main_open		= main_check;
	masterfile_open = masterfile_check;
	total_open 		= total_check;

	return (EXIT_SUCCESS);
}

int
HeaderPrint (
 void)
{
	if (window_cnt == 2)
	{
		lp_x_off = 1;
		lp_y_off = 3;
	}
	else
	{
		lp_x_off = 0;
		lp_y_off = 1;
	}
	Dsp_print ();
	Redraw ();
    return (EXIT_SUCCESS);
}

int
LedgerTotalDisplay (void)
{
	int		i;
	double	atotal  = 0.00;
	double	tot_total  = 0.00;
	double	fgnCalcTot = 0.00;
	double	locCalcTot = 0.00;

	char	wk_tot [8] [14];
	char	useCurr [4];

	if (localValue == TRUE)
		sprintf (useCurr, "%-3.3s", envVarCurrCode);
	else
		sprintf (useCurr, "%-3.3s", cumr_rec.curr_code);

	if (total_open)
		Dsp_close ();

	if (masterfile_open)
	{
		Dsp_close ();
		masterfile_open = FALSE;
	}
	
	total_open = TRUE;

	sprintf (head_str, 
			 "Customer%s (%s)", 
			 cumr_rec.dbt_no,
			 cumr_rec.dbt_name);

	Dsp_nc_prn_open (1, 3, 13, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                                     C U S T O M E R S   T O T A L S   D I S P L A Y                                          ");
	Dsp_saverec ("");
	Dsp_saverec ("");

	CalculateInvoice (TRUE, cumr_rec.hhcu_hash);

	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		CalculateInvoice (FALSE, cumr2_rec.hhcu_hash);
		cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
	}

	sprintf 
	(
		disp_str, 
		"%40.40s%-20.20s      %16.16s (%3.3s)",
		" ",
		mlCcDisp [83],
		(localValue) ? CF (locTotInvoice, "N,NNN,NNN,NNN.NN")
					 : CF (fgnTotInvoice, "N,NNN,NNN,NNN.NN"),
		useCurr
	);
	Dsp_saverec (disp_str);

	sprintf 
	(
		disp_str, 
		"%40.40s%-20.20s      %16.16s (%3.3s)",
		" ",
		mlCcDisp [84],
		(localValue) ? CF (locTotCredit, "N,NNN,NNN,NNN.NN")
					 : CF (fgnTotCredit, "N,NNN,NNN,NNN.NN"), 
		useCurr
	);
	Dsp_saverec (disp_str);

	sprintf 
	(
		disp_str, 
		"%40.40s%-10.10s                %16.16s (%3.3s)",
		" ",
		mlCcDisp [85],
		(localValue) ? CF (locTotJournal, "N,NNN,NNN,NNN.NN")
					 : CF (fgnTotJournal, "N,NNN,NNN,NNN.NN"),
		useCurr
	);
	Dsp_saverec (disp_str);

	sprintf 
	(
		disp_str, 
		"%40.40s%-20.20s      %16.16s (%3.3s)",
		" ", 
		mlCcDisp [100],
		(localValue) ? CF (fgnTotCheque,"N,NNN,NNN,NNN.NN")
					 : CF (fgnTotCheque,"N,NNN,NNN,NNN.NN"),
		useCurr
	);
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%71.71s^^GGGGGGGGGGGG^^"," ");
	Dsp_saverec (disp_str);

	if (localValue)
	{
		tot_total	= 	(locTotInvoice + locTotJournal) - 
						(locTotCredit + locTotCheque);

		atotal 		= 	(locTotInvoice + locTotJournal) - 
						((locTotCredit + locTotCheque) - locTotFwdChq);
	}
	else
	{
		tot_total	= 	(fgnTotInvoice + fgnTotJournal) - 
						(fgnTotCredit + fgnTotCheque);

		atotal 		= 	(fgnTotInvoice + fgnTotJournal) - 
						((fgnTotCredit + fgnTotCheque) - fgnTotFwdChq);
	}
	for (i = 0;i < 6;i++)
	{
		fgnCalcTot += fgnTotal [i];
		locCalcTot += locTotal [i];
	}

	sprintf 
	(
		disp_str, 
		"%40.40s%-10.10s                %16.16s (%3.3s)",
		" ", 
		mlCcDisp [86],
		CF (tot_total,"N,NNN,NNN,NNN.NN"),
		useCurr
	);
	Dsp_saverec (disp_str);

	if (fgnTotFwdChq != 0.00)
	{
		sprintf 
		(
			disp_str, 
			"%40.40s%-25.25s %16.16s (%3.3s)",
			" ",
			mlCcDisp [87],
			(localValue) ? CF (locTotFwdChq,"N,NNN,NNN,NNN.NN")
						 : CF (fgnTotFwdChq,"N,NNN,NNN,NNN.NN"),
			useCurr
		);
		Dsp_saverec (disp_str);

		sprintf 
		(
			disp_str, 
			"%40.40s%-20.20s      %16.16s (%3.3s)",
			" ",
			mlCcDisp [88],
			CF (atotal,"N,NNN,NNN,NNN.NN"),
			useCurr
		);
		Dsp_saverec (disp_str);
	}
	else
	{
		sprintf (disp_str, "%71.71s^^GGGGGGGGGGGG^^"," ");
		Dsp_saverec (disp_str);
	}

	sprintf (disp_str,"%5.5s^^AGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGB^^"," ");
	Dsp_saverec (disp_str);


	if (envVarDbDaysAgeing)
	{
		sprintf (disp_str,
				 "%5.5s^E    %-10.10s ^E   1-%2d %-6.6s ^E   %2d-%2d %-5.5s ^E   %2d-%2d %-5.5s ^E  OVER %2d %-4.4s ^E    %-10.10s ^E    %-10.10s  ^E",
				 " ",
				 mlCcDisp [89],
				 envVarDbDaysAgeing,
				 mlCcDisp [90],
				 envVarDbDaysAgeing + 1, 
				 envVarDbDaysAgeing * 2,
				 mlCcDisp [90],
				(envVarDbDaysAgeing * 2) + 1, 
				 envVarDbDaysAgeing * 3,
				 mlCcDisp [90],
				 envVarDbDaysAgeing * 3,
				 mlCcDisp [90],
				 mlCcDisp [91],
				 mlCcDisp [92]);
	}
	else
	{
		sprintf (disp_str,
				 "%5.5s^E    %-10.10s ^E   %-10.10s  ^E   %-10.10s  ^E   %-10.10s  ^E   %-10.10s  ^E    %-10.10s ^E     %-10.10s ^E",
				 " ",
				 mlCcDisp [89],
				 mlCcDisp [93],
				 mlCcDisp [94],
				 mlCcDisp [95],
				 mlCcDisp [96],
				 mlCcDisp [91],
				 mlCcDisp [92]);
	
	}

	Dsp_saverec (disp_str);

	sprintf (disp_str,"%5.5s^^KGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGL^^"," ");
	Dsp_saverec (disp_str);

	if (localValue)
	{
		strcpy (wk_tot [0], CF (locTotal [0], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [1], CF (locTotal [1], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [2], CF (locTotal [2], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [3], CF (locTotal [3], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [4], CF (locTotal [4], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [5], CF (locTotal [5], "NNNN,NNN,NNN.NN"));
		strcpy (wk_tot [6], CF (locCalcTot,     "NNN,NNN,NNN.NN"));
	}
	else
	{
		strcpy (wk_tot [0], CF (fgnTotal [0], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [1], CF (fgnTotal [1], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [2], CF (fgnTotal [2], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [3], CF (fgnTotal [3], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [4], CF (fgnTotal [4], "NNN,NNN,NNN.NN"));
		strcpy (wk_tot [5], CF (fgnTotal [5], "NNNN,NNN,NNN.NN"));
		strcpy (wk_tot [6], CF (fgnCalcTot,     "NNN,NNN,NNN.NN"));
	}

	sprintf (disp_str,"%5.5s^E%14.14s ^E%14.14s ^E%14.14s ^E%14.14s ^E%14.14s ^E%14.14s ^E%15.15s ^E",
			" ",
			wk_tot [0], wk_tot [1], wk_tot [2],
			wk_tot [3], wk_tot [4], wk_tot [5],
			wk_tot [6]);

	Dsp_saverec (disp_str);

	sprintf (disp_str,"%5.5s^^CGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGD^^"," ");
	Dsp_saverec (disp_str);
	Dsp_srch ();
    return (EXIT_SUCCESS);
}

void
CalculateInvoice (
	int		clearTotals, 
	long 	hhcuHash)
{
	int 	i;
	double	fgnBalance 	=	0.00,
			locBalance	=	0.00;

	if (clearTotals)
	{
		fgnTotInvoice 	= 0.00;
		fgnTotCredit 	= 0.00;
		fgnTotJournal 	= 0.00;
		fgnTotCheque 	= 0.00;
		fgnTotFwdChq 	= 0.00;
		locTotInvoice 	= 0.00;
		locTotCredit 	= 0.00;
		locTotJournal 	= 0.00;
		locTotCheque 	= 0.00;
		locTotFwdChq 	= 0.00;

		for (i = 0; i < 6; i++)
		{
			fgnTotal [i] = 0.00 ;
			locTotal [i] = 0.00 ;
		}
	}

	abc_selfield (cuin, "cuin_cron");
	cuin_rec.hhcu_hash 		= hhcuHash;
	cuin_rec.date_of_inv 	= local_rec.start_date;
	strcpy (cuin_rec.est, "  ");
	cc = find_rec (cuin,&cuin_rec,GTEQ,"r");
	while (!cc && cuin_rec.hhcu_hash == hhcuHash) 
	{
	    if (CREDIT)
			fgnBalance = (envVarCnNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			              		  		 	: cuin_rec.amt;
	    else
			fgnBalance = (envVarDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			              		  		 	: cuin_rec.amt;

		if (IsZero (cuin_rec.exch_rate))
			cuin_rec.exch_rate = 1.00;

		locBalance	=	fgnBalance / cuin_rec.exch_rate;

		if (cuin_rec.type [0] == '1')
		{
			fgnTotInvoice 	+= fgnBalance;
			locTotInvoice 	+= locBalance;
		}

		if (cuin_rec.type [0] == '2')
		{
			fgnTotCredit 	+= fgnBalance * -1;
			locTotInvoice 	+= locBalance * -1;
		}

		if (cuin_rec.type [0] == '3')
		{
			fgnTotJournal 	+= fgnBalance;
			locTotJournal 	+= locBalance;
		}

		for (i = 0;i < dtls_cnt;i++)
		{
			if (dtls [i].hhci_hash == cuin_rec.hhci_hash)
			{
				if (!dtls [i].fwd_payment)
				{
					fgnBalance 	-=	dtls [i].inv_fgn_amt;
					locBalance 	-=	(dtls [i].inv_loc_amt -
								  	 dtls [i].inv_ex_var);
				}

				if (!strcmp (dtls [i].app,"APP") && dtls [i].type [0] == '2')
				{
					fgnTotInvoice 	-= dtls [i].inv_fgn_amt;
					locTotInvoice 	-= (dtls [i].inv_loc_amt -
								  	 	dtls [i].inv_ex_var);
				}
				else
				{
					if (dtls [i].fwd_payment)
					{
						fgnTotFwdChq 	+= dtls [i].inv_fgn_amt;
						locTotFwdChq	+= (dtls [i].inv_loc_amt -
								  	 	 	dtls [i].inv_ex_var);
					}

					if (dtls [i].type [0] == '1')
					{
						fgnTotCheque 	+= dtls [i].inv_fgn_amt;
						locTotCheque 	+= (dtls [i].inv_loc_amt - 
								  	 	 	dtls [i].inv_ex_var);
					}
					else
					{
						fgnTotJournal 	-= dtls [i].inv_fgn_amt;
						locTotJournal 	-= (dtls [i].inv_loc_amt -
								  	 	 	dtls [i].inv_ex_var);
					}
				}
			}
		}

		i	=	 AgePeriod 
				(
					cuin_rec.pay_terms,
					cuin_rec.date_of_inv,
					comm_rec.dbt_date,
					(invoiceDueDate) ? cuin_rec.due_date : cuin_rec.date_of_inv,
					envVarDbDaysAgeing,
					envVarDbTotalAge
				);
		if (i == -1)
		{
			fgnTotal [5] += fgnBalance;
			locTotal [5] += locBalance;
		}
		else
		{
			fgnTotal [i] += fgnBalance;
			locTotal [i] += locBalance;
		}
		cc = find_rec (cuin,&cuin_rec,NEXT,"r");
	}
}

int
GraphThisLast (void)
{
	struct	GR_WINDOW gr_win;
	struct	GR_NAMES gr_nam;
	char	*gr_tits [12];
	char	gpx_ch_indx [4];
	char	mth_nms [12] [10];
	double	gr_val [2 * 12];
	int		i;
	int		j;
	int		cur_mth;

	/*-------------------
	| Display Graph Key |
	-------------------*/
	DisplayGraphKeys ();

	DateToDMY (comm_rec.dbt_date, NULL, &cur_mth, NULL);

	lp_x_off = 1;
	lp_y_off = 3;
	gr_win.x_posn = 1;
	gr_win.y_posn = 3;
	gr_win.x_size = 118;
	gr_win.y_size = 12;

	for (i = 0; i < 12; i++)
	{
		j = (i + cur_mth) % 12;

		gr_val [ (1 * 12) + i] = DOLLARS (cy_sales [j]);
		gr_val [ (0 * 12) + i] = DOLLARS (ly_sales [j]);
		sprintf (mth_nms [i], "  %-3.3s  ", month_nm [j]);
		gr_tits [i] = mth_nms [i];
	}

	gr_nam.pr_head = head_str;
	gr_nam.heading = "          S A L E S   H I S T O R Y   D I S P L A Y          ";
	gr_nam.legends = gr_tits;
	strcpy (gpx_ch_indx, "12");
	gr_nam.gpx_ch_indx = gpx_ch_indx;

	GR_graph (&gr_win, GR_TYPE_DBAR, 12, &gr_nam, gr_val, (struct GR_LIMITS *) NULL);

	Redraw ();
    return (EXIT_SUCCESS);
}

/*-------------------
| Display Graph Key |
-------------------*/
void
DisplayGraphKeys (void)
{
#ifndef GVISION
	Dsp_open (120, 3, 13);
	Dsp_saverec ("  KEY. ");
	Dsp_saverec ("");
	Dsp_saverec ("");
	Dsp_saverec (" L   T ");
	Dsp_saverec (" a   h ");
	Dsp_saverec (" s   i ");
	Dsp_saverec (" t   s ");
	Dsp_saverec ("       ");
	Dsp_saverec (" Y   Y ");
	Dsp_saverec (" e   e ");
	Dsp_saverec (" a   a ");
	Dsp_saverec (" r   r ");
	Dsp_saverec ("      ");
	Dsp_saverec ("^M^M  ^N^N");
	Dsp_srch ();
	Dsp_close ();
#else	/* GVISION */
	char *	gr_collabels [2];
	gr_collabels [0] = "Last Year";
	gr_collabels [1] = "This Year";
	GR_SetColumnLabels (2, gr_collabels);
#endif	/* GVISION */
}

/*
 * Display Payments Graph  
 */
int
GraphPayments (void)
{
	long	PayDate = 0L;

	abc_selfield (cuhd, "cuhd_hhcp_hash");
	abc_selfield (cudt, "cudt_hhci_hash");

	lp_x_off = 4;
	lp_y_off = 3;
	Dsp_prn_open (1, 3, InternalPageSize, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);
    Dsp_saverec (" INVOICE | INVOICE  |PERIOD                                                                                                 ");
	Dsp_saverec (" NUMBER  |   DATE   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |  OVER ");
	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END]");

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
    cuin_rec.date_of_inv = 0L;
	strcpy (cuin_rec.est, "  ");
    cc = find_rec (cuin,&cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.hhcu_hash == cumr_rec.hhcu_hash)
    {
 		if (cuin_rec.type [0] != '1')
		{
    		cc = find_rec (cuin,&cuin_rec, NEXT, "r");
			continue;
		}

		PayDate = 0L;
		cudt_rec.hhci_hash = cuin_rec.hhci_hash;
    	cc = find_rec (cudt,&cudt_rec, COMPARISON, "r");
		if (!cc)
		{
			cuhd_rec.hhcp_hash = cudt_rec.hhcp_hash;
			cc = find_rec (cuhd,&cuhd_rec, COMPARISON, "r");
			if (!cc)
				PayDate = cuhd_rec.date_payment;
		}
		DspPayment (PayDate); 
    	cc = find_rec (cuin,&cuin_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	Redraw ();

	abc_selfield (cuhd, "cuhd_id_no");
	abc_selfield (cudt, "cudt_hhcp_hash");
    return (EXIT_SUCCESS);
}

void
DspPayment (
	long	PaymentDate)
{
    char    Astring [300];
    char    NewString [300];
    int     i;
    int     j; 

	i = PayPer
		(
			cuin_rec.pay_terms, 
			cuin_rec.date_of_inv, 
			(PaymentDate) ? PaymentDate : comm_rec.dbt_date 
		);

	if (i > 13)
		i = 13;
    
	if (PaymentDate != 0)
	{
		if (cuin_rec.due_date < PaymentDate)
		{         
			for (j = 0;j < i;j++)
			{
				if (j == 0)
               		strcpy (Astring,"^^GGGGGGGG");
				else
               		strcat (Astring,"GGGGGGGG");
			}
			sprintf (NewString, "%s^^",Astring);
			sprintf (disp_str, " %8.8s^E%10.10s^E%s",cuin_rec.inv_no, DateToString (cuin_rec.date_of_inv), NewString);
		}
		else
		{
			sprintf (disp_str, " %8.8s^E%10.10s^E%s",cuin_rec.inv_no, DateToString (cuin_rec.date_of_inv), "Paid on time");
		}
    }
	else
       sprintf (disp_str, " %8.8s^E%10.10s^E%s",cuin_rec.inv_no, DateToString (cuin_rec.date_of_inv), "(Not Paid)");

	Dsp_saverec (disp_str);
}

int	
PayPer (
	char	*paymentTerms,
	long	invoiceDate,
	long	currentDate)
{
	int		mth_term = 0,
			period = 0,
			cd,
			cm,
			cy;
	int		scal_dmy [3],
			scur_dmy [3];

	static	int	days [12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	/*-------------------------------------------
	| Adjust current date to last day of month. |
	-------------------------------------------*/
	DateToDMY (currentDate, &scur_dmy [0],&scur_dmy [1],&scur_dmy [2]);
	DateToDMY (invoiceDate, &scal_dmy [0],&scal_dmy [1],&scal_dmy [2]);

	if (!envVarDbTotalAge)
	{
		days [1] = ((scur_dmy [2] % 4) == 0) ? 29 : 28;

		/*------------------------------------
		| get current month from module date |
		------------------------------------*/
		scur_dmy [0] = days [scur_dmy [1] - 1];
	}

	cm = scal_dmy [1];

	if (!strcmp (paymentTerms, "   "))
		paymentTerms = "20A";

	if (* (paymentTerms + 2) >= 'A')
	{
		mth_term = * (paymentTerms + 2) - 'A' + 1;
		cd = atoi (paymentTerms);
		cm += mth_term;
		cy = scal_dmy [2];
		if (cm > 12)
		{
			cm -= 12;
			cy++;
		}
		mth_term = (scur_dmy [2] - cy) * 12;
		mth_term += scur_dmy [1] - cm;
		mth_term += (scur_dmy [0] > cd) ? 1 : 0;
		mth_term = (mth_term < -1) ? 0 : mth_term;
		return (mth_term);
	}
	else 
    {   
	    mth_term = atoi (paymentTerms);
		if (mth_term <= 0)
			mth_term = 1;

		currentDate -= invoiceDate;
		period = ((int) currentDate < mth_term)
					? 0 : (int) currentDate / mth_term;

		period = (currentDate < 0L) ? 0 : period;

		return (period);
     } 
}
/*
 * Execute invoice display passing relevent arguments. 
 */
void
RunSpecialDisplay (
 char *_co_no, 
 char *_br_no, 
 char *_type, 
 long _hhcu_hash, 
 char *_inv_no, 
 int ord_disp)
{
	char	*sptr;
	char	*tptr = (char *) 0;
	int		indx = 0;
	char	run_string [100];

	sprintf (run_string, "%-2.2s %-2.2s %-1.1s %010ld %-8.8s",
		_co_no, _br_no, _type, _hhcu_hash, _inv_no);

#ifndef GVISION
	box (40,21,40,1);
	rv_pr (ML (mlStdMess035), 41,22,1);
#endif	/* GVISION */

	/*---------------------------------
	| Shell off and get item details. |
	---------------------------------*/
	if (ord_disp == 1)
	{
		arg [0] = "so_sinvdisp" ;
		tptr = get_env ("SO_INVDISP");
	}
	if (ord_disp == 2)
	{
		arg [0] = "so_scrddisp" ;
		tptr = get_env ("SO_CRDDISP");
	}
	if (ord_disp == 3)
	{
		arg [0] = "so_sdisplay" ;
		tptr = get_env ("SO_ORDDISP");
	}
	if (ord_disp == 4)
	{
		arg [0] = "so_ccn_sdsp" ;
		tptr = get_env ("SO_CCNDISP");
	}
	sptr = strchr (tptr,'~');
	while (sptr != (char *)0)
	{
		*sptr = '\0';
		arg [++indx] = tptr;

		tptr = sptr + 1;
		sptr = strchr (tptr,'~');
	}
	arg [++indx] = run_string;
	arg [++indx] = (char *)0;

	shell_prog (2);
	swide ();
	Dsp_heading ();
}

/*========================
| Display Sales history. |
========================*/
int
HistoryDisplay (void)
{
	register	int		i;
	int		j;
	int		cur_mth;
	int		cur_year;
	char	wk_tot [2] [17];

	sprintf (head_str, 
			 "Customer %s (%s)",
			 cumr_rec.dbt_no,
			 cumr_rec.dbt_name);

	DateToDMY (comm_rec.dbt_date, NULL, &cur_mth, &cur_year);

	lp_x_off = 0;
	lp_y_off = 4;
	Dsp_prn_open (0, 3, 12, head_str, comm_rec.co_no, comm_rec.co_name,
					  comm_rec.est_no, comm_rec.est_name,
					  (char *) 0, (char *) 0);
	Dsp_saverec ("    Month        |  Sales Value    |    Month        |  Sales Value    ");
	Dsp_saverec ("");

	Dsp_saverec (" [PRINT] [NEXT] [PREV] [END] ");
	for (i = 0;i < 12; i++)
	{
		j = (i + cur_mth) % 12;
		strcpy (wk_tot [0],CF (cy_sales [j], "N,NNN,NNN,NNN.NN"));
		strcpy (wk_tot [1],CF (ly_sales [j], "N,NNN,NNN,NNN.NN"));
		sprintf (disp_str," %-10.10s %04d ^E%16.16s ^E %-10.10s %04d ^E%16.16s ",
			month_nm [j],
			((j + 1) > cur_mth) ? (cur_year-1): cur_year,
			wk_tot [0],
			month_nm [j],
			((j + 1) > cur_mth) ? (cur_year-2) : (cur_year - 1),
			wk_tot [1]);

		Dsp_saverec (disp_str);
	}
	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

/*=========================
| Display Sales Analysis. |
=========================*/
int
SalesAnalysisDisplay (void)
{
		char	data_str [200];

		open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no3");

		fsort = sort_open ("s_analy");

		sadf_rec.hhcu_hash = cumr_rec.hhcu_hash;
		sadf_rec.hhbr_hash = 0L;
		cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
		while (!cc && sadf_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			inmr_rec.hhbr_hash	=	sadf_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
				continue;
			}

			if (sadf_rec.year [0] != 'C')
			{
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
				continue;
			}

			/*-------------------------------
			| Calculate mtd and ytd figures |
			-------------------------------*/
			CalculateMtd ();

			/*-----------------------------------------------
			| Store if sales & cost of sales <> 0.00	|
			-----------------------------------------------*/
			if (m_qty [0]   != 0 || m_sales [0] != 0 || m_csale [0] != 0 ||
				 y_qty [0]   != 0 || y_sales [0] != 0 || y_csale [0] != 0)
			{
				sprintf (data_str,
				"%-16.16s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %-40.40s\n",
				inmr_rec.item_no,
				m_qty [0],
				m_sales [0],
				m_csale [0],
				y_qty [0],
				y_sales [0],
				y_csale [0],
				inmr_rec.description);
			sort_save (fsort, data_str);
		}

		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}

	/*-------------------
	| Initialize Output |
	-------------------*/
	Dsp_open (1, 3, 11);

	Dsp_saverec (HEADER);
	Dsp_saverec (ITEM_HEAD);
	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END]");
	
	ProcessSales ();

	Dsp_srch_fn (ItemGraph);
	Dsp_close ();

	Redraw ();
	abc_fclose (sadf);

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Calculate mtd and ytd figures |
-------------------------------*/
void
CalculateMtd (void)
{
	int		i;

	m_qty [0] 	= sadf_qty_per [currentMonth - 1];
	m_sales [0] = sadf_sal_per [currentMonth - 1];
	m_csale [0] = sadf_cst_per [currentMonth - 1];
	
	y_qty [0] = 0.00;
	y_sales [0] = 0.00;
	y_csale [0] = 0.00;

	if (currentMonth <= fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			y_qty [0] 	+= sadf_qty_per [i];
			y_sales [0] += sadf_sal_per [i];
			y_csale [0] += sadf_cst_per [i];
		}

		for (i = 0; i < currentMonth; i++)
		{
			y_qty [0] 	+= sadf_qty_per [i];
			y_sales [0] += sadf_sal_per [i];
			y_csale [0] += sadf_cst_per [i];
		}
	}
	else
	{
		for (i = fiscal; i < currentMonth; i++)
		{
			y_qty [0] 	+= sadf_qty_per [i];
			y_sales [0] += sadf_sal_per [i];
			y_csale [0] += sadf_cst_per [i];
		}
	}
}

/*-------------------
| Process sort file |
-------------------*/
void
ProcessSales (
 void)
{
	char	*sptr;
	int		printed;
	int		i;

	for (i = 0; i < 2; i++)
	{
		m_qty [i]   = 0.00;
		m_sales [i] = 0.00;
		m_csale [i] = 0.00;
		y_qty [i]   = 0.00;
		y_sales [i] = 0.00;
		y_csale [i] = 0.00;
	}

	printed = FALSE;
	firstTime = TRUE;

	fsort = sort_sort (fsort,"s_analy");
	sptr = sort_read (fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;
		sprintf (currentItem, "%-16.16s", sptr);

		if (firstTime || strcmp (previousItem, currentItem))
		{
			if (!firstTime)
				PrintTotal ("I");
			strcpy (previousItem, currentItem);
			sprintf (itemDesc, "%-40.40s", sptr + 83);
		}

		m_qty [0]   += (float) (atof (sptr + 17));
		m_sales [0] += atof (sptr + 28);
		m_csale [0] += atof (sptr + 39);
		y_qty [0]   += (float) (atof (sptr + 50));
		y_sales [0] += atof (sptr + 61);
		y_csale [0] += atof (sptr + 72);

		firstTime = FALSE;
		sptr = sort_read (fsort);
	}

	if (printed)
	{
		PrintTotal ("I");
		PrintTotal ("C");
	}

	sort_delete (fsort, "s_analy");
}

/*-----------------------------
| Print Sales Analysis Totals |
-----------------------------*/
void
PrintTotal (
 char *tot_type)
{
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	int		j = 0;
	int		m_mar_exceed = FALSE;
	int		y_mar_exceed = FALSE;
	char	margin_exceed [8];
	char	mnth_margin [8];
	char	year_margin [8];
	char	dsp_str [200];

	switch (tot_type [0])
	{
	case	'C':
		j = 1;
		strcpy (err_str, mlCcDisp [97]);

		break;

	case	'I':
		j = 0;
		sprintf (err_str,
			"%-16.16s^E %-40.40s",
			previousItem, 
			itemDesc);

		m_qty  [1] += m_qty [0];
		m_sales [1] += m_sales [0];
		m_csale [1] += m_csale [0];
		y_qty  [1] += y_qty [0];
		y_sales [1] += y_sales [0];
		y_csale [1] += y_csale [0];

		break;
	}

	if (m_sales [j] != 0.00)
	{
		m_margin = (float) ((m_sales [j] - m_csale [j]) / m_sales [j] * 100.00);
		/*---------------------------------
		| Check if margin is out of range |
		---------------------------------*/
		if (m_margin >= 100000)
		{
			strcpy (margin_exceed, "+******");
			m_mar_exceed = TRUE;
		}

		if (m_margin <= -10000)
		{
			strcpy (margin_exceed, "-******");
			m_mar_exceed = TRUE;
		}
	}

	if (y_sales [j] != 0.00)
	{
		y_margin = (float) ((y_sales [j] - y_csale [j]) / y_sales [j] * 100.00);
		/*---------------------------------
		| Check if margin is out of range |
		---------------------------------*/
		if (y_margin >= 100000)
		{
			strcpy (margin_exceed, "+******");
			m_mar_exceed = TRUE;
		}

		if (y_margin <= -10000)
		{
			strcpy (margin_exceed, "-******");
			m_mar_exceed = TRUE;
		}
	}

	if (m_mar_exceed)
		sprintf (mnth_margin, "%-7.7s", margin_exceed);
	else
		sprintf (mnth_margin, "%7.1f", m_margin);

	if (y_mar_exceed)
		sprintf (year_margin, "%-7.7s", margin_exceed);
	else
		sprintf (year_margin, "%7.1f", y_margin);

	sprintf (dsp_str,
		"%-57.57s%s^E%8.0f^E%8.0f^E%8.0f^E%-7.7s^E%8.0f^E%8.0f^E%8.0f^E%-7.7s",
		err_str,
		(j == 0) ? " " : "",
		m_qty [j],
		m_sales [j],
		m_csale [j],
		mnth_margin,
		y_qty [j],
		y_sales [j],
		y_csale [j],
		year_margin);
	
	if (j == 0)
		Dsp_save_fn (dsp_str, previousItem);
	else
		Dsp_saverec (dsp_str);

	m_qty  [0] = 0.00;
	m_sales [0] = 0.00;
	m_csale [0] = 0.00;
	y_qty  [0] = 0.00;
	y_sales [0] = 0.00;
	y_csale [0] = 0.00;
}

void
GetDetails (
 void)
{
	int		i = 0;

	for (i = 0; i < MAXLINES; i++)
	  	store [i].recordHash = 0;

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;
	/*-----------------------------------------
	| Prevents entry if not all lines loaded. |
	-----------------------------------------*/
	cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cucc_rec.record_no = 0;
	cc = find_rec (cucc, &cucc_rec, GTEQ, "r");
	while (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		/*--------------------------------
		| Put Value Into Tabular Screen. |
		--------------------------------*/
		putval (lcount [2]);

		store [lcount [2]].recordHash = cucc_rec.record_no;
 
		lcount [2]++;

		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lcount [2] > MAXLINES) 
			break;
		
		cc = find_rec (cucc, &cucc_rec, NEXT, "r");
	}
}

void
UpdateNotes (
 void)
{
	int		line_cnt;
	
    	/*------------------------------------------------------
    	| Set to Tabular Screen (s) to Update Discount Details. |
    	------------------------------------------------------*/
    	scn_set (2);
    	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
    	{
	    	/*----------------------------------------------------
	    	| Get Current Line From Tabular Screen and Store it. |
	    	----------------------------------------------------*/
	    	cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	    	cucc_rec.record_no = store [line_cnt].recordHash;
	    	cc = find_rec (cucc, &cucc_rec, COMPARISON, "u");
	
	    	getval (line_cnt);

		if (cc)
			cc = abc_add (cucc, &cucc_rec);
		else
			cc = abc_update (cucc, &cucc_rec);
		if (cc)
		        file_err (cc, "cucc", "DBADD");
	}

	for (line_cnt = lcount [2];line_cnt < MAXLINES;line_cnt++) 
	{
	    cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	    cucc_rec.record_no = store [line_cnt].recordHash;
	    cc = find_rec (cucc, &cucc_rec, COMPARISON, "u");
	    if (!cc)
	    {
	     	    cc = abc_delete (cucc);
	    	    if (cc)
		            file_err (cc, "cucc", "DBDELETE");
	    }
	    else
	    	break;
	}
	abc_unlock (cucc);
}

/*--------------------------------
| Telesales General Information. |
--------------------------------*/
int
TsInfo (void)
{
	if (!envVarTsInstalled)
		return (EXIT_SUCCESS);

	tspm_rec.hhcu_hash	=	curr_hhcu;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlTsMess002));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}
	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, 11, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                                     T E L E - S A L E S  I N F O R M A T I O N                                                 ");

	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (tmpf_rec.co_no, comm_rec.co_no);
	strcpy (tmpf_rec.pos_code, tspm_rec.cont_code1);
	cc = find_rec ("tmpf", &tmpf_rec, COMPARISON, "r");
	if (cc)
		strcpy (tmpf_rec.pos_desc, " ");

	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %-30.30s", 
			mlCcDisp [8], tspm_rec.cont_name1, 
			mlCcDisp [101], tmpf_rec.pos_desc);
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%-20.20s : %60.60s",mlCcDisp [110],tspm_rec.email1);
	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	strcpy (tmpf_rec.co_no, comm_rec.co_no);
	strcpy (tmpf_rec.pos_code, tspm_rec.cont_code2);
	cc = find_rec ("tmpf", &tmpf_rec, COMPARISON, "r");
	if (cc)
		strcpy (tmpf_rec.pos_desc, " ");

	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %-30.30s", 
			mlCcDisp [11], tspm_rec.cont_name2, 
			mlCcDisp [101], tmpf_rec.pos_desc);
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%-20.20s : %60.60s",mlCcDisp [110],tspm_rec.email2);
	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %-30.30s", 
			mlCcDisp [102], DateToString (tspm_rec.date_create),
			mlCcDisp [103], DateToString (tspm_rec.lphone_date));
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %03d", 
			mlCcDisp [104], tspm_rec.best_ph_time,
			mlCcDisp [105], tspm_rec.phone_freq);
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %-30.30s", 
			mlCcDisp [106], DateToString (tspm_rec.n_phone_date),
			mlCcDisp [107], tspm_rec.n_phone_time);
	Dsp_saverec (disp_str);

	sprintf (disp_str, "%-20.20s : %30.30s     ^E %-20.20s : %-30.30s", 
			mlCcDisp [108], DateToString (tspm_rec.n_visit_date),
			mlCcDisp [109], tspm_rec.n_visit_time);
	Dsp_saverec (disp_str);

	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}
int
TsNotes (
 void)
{
	return (CallNotes ("N", "Maintain Telesales Call Notes "));
}

int
TsLastCall (
 void)
{
	return (CallNotes ("L", "Maintain Telesales Last Notes "));
}

int
TsComplaints (
 void)
{
	return (CallNotes ("C", "Maintain Telesales Complaints "));
}

int
TsNextVisit (
 void)
{
	return (CallNotes ("V", "Maintain Telesales Next Visit "));
}

/*-------------------------------
| Maintain Telesales Complaints |
-------------------------------*/
int
CallNotes (
 char	*CallType,
 char	*TextDesc)
{
	int		txt_cnt;

	if (!envVarTsInstalled)
		return (EXIT_SUCCESS);

	tspm_rec.hhcu_hash	=	curr_hhcu;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlTsMess002));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_SUCCESS);
	}

	strcpy (local_rec.CallPrompt, TextDesc);
	scn_set (3);
	lcount [3] = 0;
	init_vars (3);

	tsxd_rec.hhcu_hash = curr_hhcu;
	strcpy (tsxd_rec.type, CallType);
	tsxd_rec.line_no = 0;
	cc = find_rec (tsxd, &tsxd_rec, GTEQ, "r");
	while (!cc &&
	       tsxd_rec.hhcu_hash == curr_hhcu &&
	       tsxd_rec.type [0] == CallType [0])
	{
		sprintf (local_rec.text, "%-60.60s", tsxd_rec.desc);
		putval (lcount [3]++);

		cc = find_rec (tsxd, &tsxd_rec, NEXT, "r");
	}

	blank_at (12, 2, 70);
	scn_display (3);

	edit (3);
	if (restart)
	{
		restart = FALSE;
		ClearRedraw ();
		return (FALSE);
	}

	for (txt_cnt = 0;txt_cnt < lcount [3] ;txt_cnt++)
	{
		getval (txt_cnt);

		tsxd_rec.hhcu_hash = curr_hhcu;
		strcpy (tsxd_rec.type, CallType);
		tsxd_rec.line_no = txt_cnt;
		cc = find_rec (tsxd,&tsxd_rec,COMPARISON,"u");
		if (cc)
		{
			sprintf (tsxd_rec.desc, "%-60.60s", local_rec.text);

			cc = abc_add (tsxd,&tsxd_rec);
			if (cc)
			   file_err (cc, "tsxd", "DBADD");
		}
		else
		{
			sprintf (tsxd_rec.desc, "%-60.60s", local_rec.text);

			cc = abc_update (tsxd,&tsxd_rec);
			if (cc)
				file_err (cc, "tsxd", "DBUPDATE");
		}
	}

	/*------------------
	| Delete old lines |
	------------------*/
	tsxd_rec.hhcu_hash = curr_hhcu;
	strcpy (tsxd_rec.type, CallType);
	tsxd_rec.line_no = txt_cnt;
	cc = find_rec (tsxd,&tsxd_rec,GTEQ, "r");
	while (!cc &&
	       tsxd_rec.hhcu_hash == curr_hhcu &&
	       tsxd_rec.type [0] == CallType [0])
	{
		abc_delete (tsxd);

		tsxd_rec.hhcu_hash = curr_hhcu;
		strcpy (tsxd_rec.type, CallType);
		tsxd_rec.line_no = txt_cnt;

		cc = find_rec (tsxd, &tsxd_rec, GTEQ, "r");
	}
	restart = FALSE;
	ClearRedraw ();
	return (TRUE);
}

/*------------------------------------
| Graph last 24 months sales history |
| current debtor and chosen item     |
------------------------------------*/
int
ItemGraph (
 char *item_no)
{
	struct	GR_WINDOW sa_win;
	struct	GR_NAMES sa_nam;
	char	*sa_tits [12];
	char	gpx_ch_indx [4];
	char	mth_nms [12] [10];
	double	sa_val [2 * 12];
	int		yr_idx;
	int		i, j;

	/*-------------------------
	| Initialise Graph Values |
	-------------------------*/
	for (i = 0; i < 12; i++)
	{
		sa_val [ (0 * 12) + i] = 0.00;
		sa_val [ (1 * 12) + i] = 0.00;
	}

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", item_no);
	cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_SUCCESS);

	/*------------------------
	| Calculate Graph Values |
	------------------------*/
	sadf_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
	while (!cc && 
	       sadf_rec.hhcu_hash == cumr_rec.hhcu_hash &&
	       sadf_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		yr_idx = (sadf_rec.year [0] == 'C') ? 1 : 0;

		for (i = 0; i < 12; i++)
		{
			j = (i + currentMonth) % 12;
			sa_val [ (yr_idx * 12) + i] += sadf_qty_per [j];
		}

		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}

	/*-------------------
	| Display Graph Key |
	-------------------*/
	DisplayGraphKeys ();

	lp_x_off = 1;
	lp_y_off = 3;
	sa_win.x_posn = 1;
	sa_win.y_posn = 3;
	sa_win.x_size = 118;
	sa_win.y_size = 12;

	for (i = 0; i < 12; i++)
	{
		j = (i + currentMonth) % 12;

		sprintf (mth_nms [i], "  %-3.3s  ", month_nm [j]);
		sa_tits [i] = mth_nms [i];
	}

	sa_nam.pr_head = head_str;
	sprintf (prod_str, 
		"  SALES ANALYSIS DISPLAY FOR ITEM:  %-16.16s  %-40.40s  ", 
		inmr_rec.item_no,
		inmr_rec.description);
	sa_nam.heading = prod_str;
	sa_nam.legends = sa_tits;
	strcpy (gpx_ch_indx, "12");
	sa_nam.gpx_ch_indx = gpx_ch_indx;

	GR_graph (&sa_win, GR_TYPE_DBAR, 12, &sa_nam, sa_val, (struct GR_LIMITS *) NULL);
    return (EXIT_SUCCESS);
}

/*=================
| Cheque display. |
=================*/
int
ChequeDisp (
 void)
{
	int		i = 0;
	int		ln_num = 0;
	char	old_cheq [9];

	lp_x_off = 4;
	lp_y_off = 3;

	Dsp_prn_open (1, 3, InternalPageSize, head_str, 
							comm_rec.co_no, comm_rec.co_name, 
							comm_rec.est_no, comm_rec.est_name, 
							(char *) 0, (char *) 0);

	if (envVarDbMcurr)
	{
		Dsp_saverec ("  CHEQUE   | DATE  OF |    DUE   | DISCOUNT |    NET     |  CHEQUE    |INVOICE |    INVOICE  |  EXCH  |   INVOICE   |    EXCH    ");
		Dsp_saverec ("  NUMBER   |  CHEQUE  |   DATE   |  AMOUNT  |  CHEQUE    |  LOCAL     | NUMBER |    CREDIT   |  RATE  |    LOCAL    |  VARIANCE  ");
	}
	else
	{
		Dsp_saverec ("  CHEQUE   | DATE OF  |   DUE    | DISCOUNT |    NET     |INVOICE |    INVOICE   ");
		Dsp_saverec ("  NUMBER   |  CHEQUE  |   DATE   |  AMOUNT  |  CHEQUE    | NUMBER |    CREDIT    ");
	}
	Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]");

	strcpy (old_cheq,"        ");

	for (i = 0;i < dtls_cnt;i++)
		GetCheques (i, &ln_num, old_cheq);

	Dsp_srch ();
	Dsp_close ();
	Redraw ();
	return (EXIT_SUCCESS);
}

/*==============
| Get cheques. |
==============*/
void
GetCheques (
 int i, 
 int *ln_num, 
 char *old_cheq)
{
	char	fm_str [4] [15];
	int		j;
	char	t_line [300];
	char	env_line [300];
	double	net = 0.0;

	if (*ln_num >= InternalPageSize)
		*ln_num = *ln_num % InternalPageSize;

	j = dtls [i].cheq_hash;

	if (cheq [j].amount == 0)
		return;

	if (strcmp (old_cheq, cheq [j].no) != 0)
	{
		strcpy (old_cheq,cheq [j].no);
		net = cheq [j].amount + cheq [j].cdisc;

		if (!strcmp (cheq [j].app,"APP") && cheq [j].type [0] == '2')
			return;
		
		strcpy (fm_str [0], CF (cheq [j].cdisc,"NN,NNN.NN"));
		strcpy (fm_str [1], CF (net,"N,NNN,NNN.NN"));
		strcpy (fm_str [2], CF (cheq [j].loc_amt,"N,NNN,NNN.NN"));
	
		if (envVarDbMcurr)
		{
			sprintf (t_line,"%s %s^E%s^E%s^E%9.9s ^E%12.12s^E%12.12s^E",
				tran_type [atoi (cheq [j].type) + 2],
				cheq [j].no,
				cheq [j].datec,
				cheq [j].dated,
				fm_str [0], fm_str [1], fm_str [2]);
		}
		else
		{
			sprintf (t_line,"%s %s^E%s^E%s^E%9.9s ^E%12.12s^E",
				tran_type [atoi (cheq [j].type) + 2],
				cheq [j].no,
				cheq [j].datec,
				cheq [j].dated,
				fm_str [0], fm_str [1]);
		}
	}
	else
	{
		if (!envVarDbMcurr)
		{
			if (*ln_num != 0)
				sprintf (t_line,"           ^E          ^E          ^E          ^E            ^E");
			else
			{
				strcpy (fm_str [0], CF (cheq [j].cdisc,"NN,NNN.NN"));
				strcpy (fm_str [1], CF (net,"N,NNN,NNN.NN"));

				sprintf (t_line,"%s %s^E%s^E%s^E%9.9s ^E%12.12s^E",
					tran_type [atoi (cheq [j].type) + 2],
					cheq [j].no,
					cheq [j].datec,
					cheq [j].dated,
					fm_str [0], fm_str [1]);
			}
		}
		else
		{
			if (*ln_num != 0)
				sprintf (t_line,"           ^E          ^E          ^E          ^E            ^E            ^E");
			else
			{
				strcpy (fm_str [0], CF (cheq [j].cdisc,"NN,NNN.NN"));
				strcpy (fm_str [1], CF (net,"N,NNN,NNN.NN"));
				strcpy (fm_str [2], CF (cheq [j].loc_amt,"N,NNN,NNN.NN"));

				sprintf (t_line,"%s %s^E%s^E%s^E%9.9s ^E%12.12s^E%12.12s^E",
					tran_type [atoi (cheq [j].type) + 2],
					cheq [j].no,
					cheq [j].datec,
					cheq [j].dated,
					fm_str [0], fm_str [1], fm_str [2]);
			}
		}
	}
	if (FindCuin (dtls [i].hhci_hash))
	{
		strcpy (cuin_rec.est,"  ");
		strcpy (cuin_rec.inv_no,"      ");
	}

	if (envVarDbMcurr)
	{
		strcpy (fm_str [0], CF (dtls [i].inv_fgn_amt,	"N,NNN,NNN.NN"));
		strcpy (fm_str [1], CF (dtls [i].inv_loc_amt,	"N,NNN,NNN.NN"));
		strcpy (fm_str [2], CF (dtls [i].inv_ex_var, 	"N,NNN,NNN.NN"));

		sprintf (env_line,"%s%8.8s^E%12.12s ^E%8.4f^E%12.12s ^E%12.12s",
		       t_line,
		       cuin_rec.inv_no,
		       fm_str [0], 
		       dtls [i].inv_exch_rate,
		       fm_str [1], 
		       fm_str [2]);
	}
	else
	{
		sprintf (env_line,"%s%8.8s^E%12.12s ",
			t_line, cuin_rec.inv_no, CF (dtls [i].inv_fgn_amt, "N,NNN,NNN.NN"));
	}
	Dsp_saverec (env_line);
	*ln_num += 1;
}

/*=============================
| Find cuin from detail hash. |
=============================*/
int	
FindCuin (
 long hhci_hash)
{
	int		cuin_err;

	abc_selfield (cuin, "cuin_hhci_hash");

	cuin_rec.hhci_hash = hhci_hash;
	cuin_err = find_rec (cuin, &cuin_rec, COMPARISON, "r");

	abc_selfield (cuin, "cuin_cron");

	return (cuin_err);
}


/*=============================
| Display Market Information. |
==============================*/
int
MarketInfo (
 void)
{
	int		mk_counter = 0,
			i;

	int		FirstTime	=	TRUE;

	open_rec (exms,  exms_list, EXMS_NO_FIELDS, "exms_id_no");
	open_rec (exmd,  exmd_list, EXMD_NO_FIELDS, "exmd_id_no");
	open_rec (exmh,  exmh_list, EXMH_NO_FIELDS, "exmh_id_no");
	open_rec (incs,  incs_list, INCS_NO_FIELDS, "incs_incs_hash");

	abc_selfield (exsf, "exsf_hhsf_hash");
	
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	strcpy (exsf_rec.salesman_no,cumr_rec.sman_code);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, InternalPageSize, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                                           M A R K E T    I N F O R M A T I O N                                                 ");

	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	exmh_rec.hhcu_hash = cumr_rec.hhcu_hash;
	exmh_rec.hhsf_hash = 0L;
	cc = find_rec (exmh, &exmh_rec, GTEQ, "r");
	while (!cc && exmh_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if (!FirstTime)
		{
			for (i = mk_counter; i < 7; i++)
					Dsp_saverec (" ");
			mk_counter = 0;
		}
		
		FirstTime	=	FALSE;

		exsf_rec.hhsf_hash = exmh_rec.hhsf_hash;
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (exmh, &exmh_rec, NEXT, "r");
			continue;
		}
		MarketingHeader ();
		mk_counter++;

		exmd_rec.exmh_hash = exmh_rec.exmh_hash;
		exmd_rec.hhbr_hash = 0L;
		cc = find_rec (exmd, &exmd_rec, GTEQ, "r");
		while (!cc && exmd_rec.exmh_hash == exmh_rec.exmh_hash)
		{
			incs_rec.incs_hash = exmd_rec.insc_hash;
			cc = find_rec (incs, &incs_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (exmd, &exmd_rec, NEXT, "r");
				continue;
			}
			inmr_rec.hhbr_hash = exmd_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (exmd, &exmd_rec, NEXT, "r");
				continue;
			}
			sprintf (disp_str, "^E%16.16s^E%16.16s^E%40.40s^E %s ^E%12.2f ^E %8.2f ^E%20.20s ^E",
					inmr_rec.item_no,
					incs_rec.subs_code,
					incs_rec.subs_desc,
					exmd_rec.supp_stat,
					DOLLARS (exmd_rec.unit_price),
					exmd_rec.disc,
					exmd_rec.comments);
	
			Dsp_saverec (disp_str);

			if (++mk_counter > 7)
			{
				Dsp_saverec ("^^CGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGD^^");
				MarketingHeader ();
				mk_counter = 0;
			}
			cc = find_rec (exmd, &exmd_rec, NEXT, "r");
		}
		Dsp_saverec ("^^CGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGD^^");
		cc = find_rec (exmh, &exmh_rec, NEXT, "r");
	}
	abc_fclose (exms);
	abc_fclose (exmd);
	abc_fclose (exmh);
	abc_fclose (incs);

	Dsp_srch ();
	Dsp_close ();
	abc_selfield (exsf, "exsf_id_no");
	Redraw ();
    return (EXIT_SUCCESS);
}

void
MarketingHeader (
 void)
{
	sprintf (disp_str, "Salesman Number %6.6s - %40.40s                                             Date %10.10s",
				exsf_rec.salesman_no, 
				exsf_rec.salesman, 
				DateToString (exmh_rec.date));
	Dsp_saverec (disp_str);

	sprintf (disp_str, "Remarks         %-60.60s", exmh_rec.remarks);
	Dsp_saverec (disp_str);

	Dsp_saverec ("^^AGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGIGGGGGGGGGGGGGIGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGB^^");
	Dsp_saverec ("^E   Item Number  ^ECompetitor Code ^E   Competitor Substitute Description.   ^EStat^E    Price    ^EDiscount %^E      Comments       ^E");
	Dsp_saverec ("^^KGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGEGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGL^^");
}

/*===================================
| Display Merchandiser Information. |
====================================*/
int
MerchInfo (
 void)
{
	open_rec (exmm,  exmm_list, EXMM_NO_FIELDS, "exmm_id_no");
	open_rec (exma,  exma_list, EXMA_NO_FIELDS, "exma_id_no");

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, InternalPageSize, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                                         M E R C H A N D I S E R  I N F O R M A T I O N                                         ");

	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	strcpy (exmm_rec.co_no,comm_rec.co_no);
	exmm_rec.exmm_hash = cumr_rec.merchant;
	cc = find_rec (exmm, &exmm_rec, COMPARISON,"r");
	if (!cc)
	{
		sprintf (disp_str," Merchandiser Number         %6ld",exmm_rec.exmm_hash);
		Dsp_saverec (disp_str);

		sprintf (disp_str, " Merchandiser Name           %s",exmm_rec.name);
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		Dsp_saverec (disp_str);

		strcpy (exma_rec.co_no, comm_rec.co_no);
		strcpy (exma_rec.code,  exmm_rec.agency);
		if (find_rec (exma, &exma_rec, COMPARISON, "r"))
			strcpy (exma_rec.code, " ");
		
		sprintf (disp_str, " Agency Code                 %s - %s",
				exmm_rec.agency, exma_rec.code);
		Dsp_saverec (disp_str);

		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

		sprintf (disp_str," Merchandiser Start Date   %10.10s",
							DateToString (exmm_rec.st_date));
		Dsp_saverec (disp_str);

		sprintf (disp_str," Merchandiser End Date     %10.10s",
							DateToString (exmm_rec.en_date));
		Dsp_saverec (disp_str);

		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		Dsp_saverec ("^1                                          D A Y S   W O R K E D  B Y   M E R C H A N D I S E R                             ");

		sprintf (disp_str, " SUNDAY         %s %50.50s   MONDAY         %s ",
				(exmm_rec.day [0] == '1') ? "YES" : "NO ",
				" ",
				(exmm_rec.day [1] == '1') ? "YES" : "NO ");
		Dsp_saverec (disp_str);

		sprintf (disp_str, " TUESDAY        %s  %50.50s  WEDNESDAY      %s ",
				(exmm_rec.day [2] == '1') ? "YES" : "NO ",
				" ",
				(exmm_rec.day [3] == '1') ? "YES" : "NO ");
		Dsp_saverec (disp_str);

		sprintf (disp_str, " THURSDAY       %s  %50.50s  FRIDAY         %s",
				(exmm_rec.day [4] == '1') ? "YES" : "NO ",
				" ",
				(exmm_rec.day [5] == '1') ? "YES" : "NO ");
		Dsp_saverec (disp_str);

		sprintf (disp_str, " SATURDAY       %s",
				(exmm_rec.day [6] == '1') ? "YES" : "NO ");

		Dsp_saverec (disp_str);
	}
	abc_fclose (exmm);
	abc_fclose (exma);

	Dsp_srch ();
	Dsp_close ();
	Redraw ();
    return (EXIT_SUCCESS);
}

/*===============================
| Display Rep Call Information. |
================================*/
int
RepCalls (
 void)
{
	open_rec (sacd,  sacd_list, SACD_NO_FIELDS, "sacd_id_no2");
	open_rec (saca,  saca_list, SACA_NO_FIELDS, "saca_id_no");

	lp_x_off = 1;
	lp_y_off = 3;
	Dsp_nc_prn_open (1, 3, 12, head_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, (char *) 0, (char *) 0);

	Dsp_saverec ("                               S A L E S P E R S O N   C U S T O M E R   C A L L   D E T A I L S                                ");
	Dsp_saverec ("TIME  IN|TIME OUT|          SALES ACTIVITY INFORMATION           |             CALL REMARKS                                     ");
	Dsp_saverec (std_foot);
	
	sacd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sacd_rec.seq_no = 0;
	cc = find_rec ("sacd", &sacd_rec, GTEQ, "r");
	while (!cc && sacd_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		char call_in [6];
		char call_out [6];

		strcpy (call_in,  ttoa (sacd_rec.call_in,  "HH:MM"));
		strcpy (call_out, ttoa (sacd_rec.call_out, "HH:MM"));

		strcpy (saca_rec.co_no, comm_rec.co_no);
		strcpy (saca_rec.code , sacd_rec.act_code);
		if (find_rec (saca, &saca_rec, COMPARISON, "r"))
			strcpy (saca_rec.desc , "  ");
	
		sprintf (disp_str, "  %s ^E  %s ^E %s - %40.40s ^E %40.40s ",
				call_in,
				call_out,
				sacd_rec.act_code,
				saca_rec.desc,
				sacd_rec.remarks);

		Dsp_saverec (disp_str);
	
		cc = find_rec ("sacd", &sacd_rec, NEXT, "r");
	}

	abc_fclose (sacd);
	abc_fclose (saca);

	Dsp_srch ();
	Dsp_close ();
	Redraw ();
	return (EXIT_SUCCESS);
}

int
CustomerGroupDisplay (
 void)
{
	/*
	 *  Invoke external program with arg
	 */
	int     idx = 0;
	char    hhcu_hash [80];

#ifndef GVISION
	box (40, 21, 40, 1);
	rv_pr (" Please Wait. Loading Display ...... ", 41, 22, 1);
#endif	/* GVISION */

	sprintf (hhcu_hash, "%ld", cumr_rec.hhcu_hash);

	arg [idx++] = "db_group_dsp";
	arg [idx++] = hhcu_hash;
	arg [idx++] = NULL;

	shell_prog (2);
	swide ();
	Dsp_heading ();

	return (EXIT_SUCCESS);
}

/*
 *	Minor support functions
 */
int
MoneyZero (
 double	m)
{
	return (fabs (m) < 0.0001);
}

void 
SetMonthName (
 int iPos, 
 char* cNewName)
{
    if ((iPos < 0) || (iPos >= 12)) 
        return;
    if (month_nm [iPos])
    {
        free (month_nm [iPos]);
        month_nm [iPos] = NULL;
    }

    month_nm [iPos] = (char*) malloc (strlen (cNewName) + 1);
    strcpy (month_nm [iPos], cNewName);
}

void 
InitMonthNames (
 void)
{
    int i;
    for (i = 0; i < 12; i++)
        month_nm [i] = NULL;

    SetMonthName (0, "JANUARY");
	SetMonthName (1, "FEBRUARY");
	SetMonthName (2, "MARCH");
	SetMonthName (3, "APRIL");
	SetMonthName (4, "MAY");
	SetMonthName (5, "JUNE");
	SetMonthName (6, "JULY");
	SetMonthName (7, "AUGUST");
	SetMonthName (8, "SEPTEMBER");
	SetMonthName (9, "OCTOBER");
	SetMonthName (10, "NOVEMBER");
	SetMonthName (11, "DECEMBER");
}

void 
DeleteMonthNames (
 void)
{
    int i;
    for (i = 0; i < 12; i++)
        if (month_nm [i])
        {
            free (month_nm [i]);
            month_nm [i] = NULL;
        }

}
int 
CustIdSort (
 const void *a1, 
 const void *b1)
{
	int		result;
	const struct CustRecord a = * (const struct CustRecord *) a1;
	const struct CustRecord b = * (const struct CustRecord *) b1;

	result = strcmp (a.custNumber, b.custNumber);

	return (result);
}

int 	
LocalToFgn (void)
{
	if (localValue == FALSE)
		localValue = TRUE;
	else
		localValue = FALSE;

	MainDisplayScreen ();
	return (0);
}
int 	
InvoiceDateDue (void)
{
	if (invoiceDueDate == FALSE)
		invoiceDueDate = TRUE;
	else
		invoiceDueDate = FALSE;
	MainDisplayScreen ();
	return (0);
}


/*
 *	Minor support functions
 */
static int
IsZero (
 double	m)
{
	return (fabs (m) < 0.0001);
}
