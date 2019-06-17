/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: GlIntegrityReport.h,v 5.2 2001/08/23 11:48:50 scott Exp $
|	Name : GlIntegrityReport.h
|	Desc : (General ledger integrity report structure.
|=====================================================================|
| $Log: GlIntegrityReport.h,v $
| Revision 5.2  2001/08/23 11:48:50  scott
| Updated from scotts machine
|
| Revision 5.1  2001/08/20 23:15:22  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
/*
 * Control report header file 
 */
struct	{
	char	*moduleType;
	char	*ifDesc;
	char	*ifCode;
	char	*userCode;
} GL_integrity [] = {
	{"AR", "Accounts receivable.    ", "ACCT REC  ", "          "},
	{"AR", "Inventory item levy.    ", "ITEM LEVY ", "          "},
	{"AR", "Suspence.               ", "SUSPENSE  ", "          "},
	{"AR", "Sales tax               ", "SALES TAX ", "          "},
	{"AR", "Freight charged.        ", "FREIGHT   ", "          "},
	{"AR", "Other invoice charges #1", "OTHER CST1", "          "},
	{"AR", "Other invoice charges #2", "OTHER CST2", "          "},
	{"AR", "Other invoice charges #3", "OTHER CST3", "          "},
	{"AR", "Insurance charged.      ", "INSURANCE ", "          "},
	{"AR", "Small order surcharge.  ", "SOS       ", "          "},
	{"AR", "Restocking fee.         ", "RESTCK FEE", "          "},
	{"AR", "GST Charged.            ", "G.S.T CHRG", "          "},
	{"AR", "Direct delivery Sales.  ", "DD CST SAL", "INVOICE   "},
	{"AR", "Exchange rate variance. ", "EXCH VAR. ", "          "},
	{"AR", "Discount on invoices.   ", "DISCOUNT  ", "          "},
	{"AR", "Invoices                ", "INVOICE   ", "          "},
	{"AR", "Credit notes.           ", "CREDIT    ", "INVOICE   "},
	{"AP", "Accounts payable        ", "ACCT PAY  ", "          "},
	{"AP", "GST Paid.               ", "G.S.T PAID", "          "},
	{"AP", "Discounts allowed.      ", "DISC ALLOW", "          "},
	{"", "", "", ""}
};

