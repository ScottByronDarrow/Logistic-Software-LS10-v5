/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: DeleteControl.h,v 5.2 2002/05/02 07:09:12 scott Exp $
|	Name : DeleteControl.h
|	Desc : (Delete control pre-set structure)
|=====================================================================|
| $Log: DeleteControl.h,v $
| Revision 5.2  2002/05/02 07:09:12  scott
| Updated to add works orders delete
|
| Revision 5.1  2001/08/14 02:33:07  scott
| New header file for DeleteControl Wizard
|
=====================================================================*/
#ifndef	_DeleteControl_h
#define	_DeleteControl_h

/*
 * Delete control header file 
 */
typedef	struct
{
	char	co_no [3];
	char	code [21];
	int		key_struct;
	long	delh_hash;
	char	desc_1 [61];
	char	desc_2 [61];
	int		purge_days;
	char	reference [21];
	int		spare_fg1;
	int		spare_fg2;
	int		spare_fg3;
}	DELH_STRUCT;

#ifdef	CCMAIN
#	define	GLOB_DELETE_CONTROL
#else
#	define	GLOB_DELETE_CONTROL	extern
#endif

#ifdef	CCMAIN
/*
 * Delete control header file 
 */
struct dbview	delh_list [] =
{
	{"delh_co_no"},
	{"delh_code"},
	{"delh_key_struct"},
	{"delh_delh_hash"},
	{"delh_desc_1"},
	{"delh_desc_2"},
	{"delh_purge_days"},
	{"delh_reference"},
	{"delh_spare_fg1"},
	{"delh_spare_fg2"},
	{"delh_spare_fg3"}
};

struct	{
	char	*delCode;
	char	*userDesc1;
	char	*userDesc2;
	int		purgeDays;
	int		keyStructure;
	char	*reference;
	int		spareFlag1;
	int		spareFlag2;
	int		spareFlag3;
} deleteHeader [] = {
	{"FREIGHT-HISTORY     ",
	 "Carrier / Freight History File.                             ",
	 "History file holds values and costs for deliveries.         ",
	 60	, 1, "", -1, -1, -1},
	{"CREDIT-CONTROL-NOTE ",
	 "Customer credit control notes file.                         ",
	 "Holds credit control notes for each customer.               ",
	 365, 1, "", -1, -1, -1}, 
	{"SOP-LAST-ITEMS-SOLD ",
	 "Sales order processing customer last sale history file.     ",
	 "Holds last item, quantity, price and disc. sold to customer ",
	 30	, 1, "", -1, -1, -1},
	{"TAX-TRANSACTIONS    ", 
	 "Sales tax transaction file details history.                 ",
	 "Holds customer, invoice, date, sales and tax values.        ",
	 90,  1, "", -1, -1, -1},
	{"RECEIPT-HISTORY     ",
	 "Customer receipts history file.                             ",
	 "Holds Receipt no, amount, date, bank and branch information.",
	 10,  1, "", -1, -1, -1},
	{"STOCK-DAILY-AUDIT   ",
	 "Inventory daily transaction audit file.                     ",
	 "Holds daily stock movements for daily audit report.         ",
	 7,   1, "", -1, -1, -1},
	{"SALES-ORDER-FILE    ",
	 "Sales order file                                            ",
	 "Only allows deletion of completed sales orders on a P/Slip. ",
	 7,   1, "", -1, -1, -1},
	{"PS-INVOICE-CREDIT   ",
	 "Packing slip / Invoice / Credit note file.                  ",
	 "Only allows deletion of posted invoices and credit notes.   ",
	 90,  2, "9", 1, -1, -1}, 
	{"INVENTORY-TRANS     ",
	 "Inventory transactions file.                                ",
	 "Allows deletion of inventory transactions.                  ",
	 365, 3, "NYYNYYYYYYYYY",60, 6, -1},
	{"INVENTORY-LOCATIONS ",
	 "Inventory locations file.                                   ",
	 "Allows deletion of locations with zero quantity on hand     ",
	 120, 1, "", -1, -1, -1}, 
	{"CUSTOMER-LEDGER     ",
	 "Customer accounting ledger.                                 ",
	 "Allows deletion of paid transactions from ledger.           ",
	 30,  4, "", 1, -1, -1}, 	
	{"SUPPLIER-LEDGER     ", 
	 "Supplier accounting ledger.                                 ",
	 "Allows deletion of paid transactions from ledger.           ",
	 30,  1, "", -1, -1, -1},
	{"POST-GOODS-RECEIPTS ", 
	 "Delete completed transactions related to goods receipts.    ",
	 "Delete goods receipts, purchase order and shipments.        ",
	 30,  1, "", -1, -1, -1},
	{"PO-RECEIPT-CLOSE    ",
	 "Automatically deletes receipted purchases orders.            ",
	 "Open allows delete of PO's that are (x) percent complete.    ",
	 2,   1, "", -1, -1, -1}, 
	{"GENERAL-LEDGER-WORK ",
	 "General Ledger batch work files. (Completed postings)        ",
	 "Delete completed general ledger batches.                     ",
	 60,   1, "", -1, -1, -1}, 
	{"WORKS-ORDER-FILE    ",
	 "Works order file                                            ",
	 "Only allows deletion of completed works orders.             ",
	 7,   1, "", -1, -1, -1},
	{"", "", "",-1,-1,"",-1,-1, -1}, 
};
const	char	*delh	=	"delh";

#else

	extern	struct	dbview	delh_list [];
	extern	const	char	*delh;

#endif

#define	DELH_NO_FIELDS	11

GLOB_DELETE_CONTROL	DELH_STRUCT	delhRec;
GLOB_DELETE_CONTROL	int		FindDeleteControl		(char *, char *);

#endif

