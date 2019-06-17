/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: OptsInfo.h,v 5.0 2001/06/19 06:51:18 cha Exp $
-----------------------------------------------------------------------
| $Log: OptsInfo.h,v $
| Revision 5.0  2001/06/19 06:51:18  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/11 04:41:58  scott
| Added info files for base database create script.
|
|
*/
#ifndef	_optsInfo_h
#define	_optsInfo_h

	struct
	{
		char	*optsProgram;
		char	*optsOptionNo;
		char	*optsKey;
	}	optsInfo []	=	{
		{"po_ss_disp", "1", "<Supplier Status.>"},
		{"po_ss_disp", "2", "<Master File Details.>"},
		{"po_ss_disp", "3", "<Total Screen.>"},
		{"po_ss_disp", "4", "<Invoices.>"},
		{"po_ss_disp", "5", "<Cheques.>"},
		{"po_ss_disp", "6", "<Cheque Exch. Details.>"},
		{"po_ss_disp", "7", "<Purchase Orders.>"},
		{"po_ss_disp", "8", "<Note Pad.>"},
		{"po_ss_disp", "9", "<Supplier Pricing.>"},
		{"po_ss_disp", "10", "<Supplier Discounting.>"},
		{"po_ss_disp", "11", "<P/O A&A info.>"},
		{"po_ss_disp", "12", "<QA info.>"},
		{"sk_alldisp", "1", "<S1>"},
		{"sk_alldisp", "2", "<S2>"},
		{"sk_alldisp", "3", "<S3>"},
		{"sk_alldisp", "4", "<S4>"},
		{"sk_alldisp", "5", "<S5>"},
		{"sk_alldisp", "6", "<S6>"},
		{"sk_alldisp", "7", "<Movements>"},
		{"sk_alldisp", "8", "<Branch Status>"},
		{"sk_alldisp", "9", "<MEND Movements>"},
		{"sk_alldisp", "10", "<UOM SOH>"},
		{"sk_alldisp", "11", "<Standard Pricing>"},
		{"sk_alldisp", "12", "<Sales Orders>"},
		{"sk_alldisp", "13", "<Back Orders>"},
		{"sk_alldisp", "14", "<Purchase Orders>"},
		{"sk_alldisp", "15", "<Shipments>"},
		{"sk_alldisp", "16", "<Purchase Orders DD>"},
		{"sk_alldisp", "17", "<Purchase History>"},
		{"sk_alldisp", "18", "<Supplier Records>"},
		{"sk_alldisp", "19", "<Supply Warehouses>"},
		{"sk_alldisp", "20", "<Global Supply Warehouses>"},
		{"sk_alldisp", "21", "<Forward Schedule>"},
		{"sk_alldisp", "22", "<Reorder Review>"},
		{"sk_alldisp", "23", "<Graph Demand>"},
		{"sk_alldisp", "24", "<Graph-This/last year>"},
		{"sk_alldisp", "25", "<Graph-24 Months>"},
		{"sk_alldisp", "26", "<Graph-Sales/Profits>"},
		{"sk_alldisp", "27", "<In-Trans Stock>"},
		{"sk_alldisp", "28", "<Transfer Requests>"},
		{"sk_alldisp", "29", "<FIFO Records>"},
		{"sk_alldisp", "30", "<Location/Lot Records>"},
		{"sk_alldisp", "31", "<Serial Records>"},
		{"sk_alldisp", "32", "<Item Note Pad>"},
		{"sk_alldisp", "33", "<Maintain Item Note Pad>"},
		{"sk_alldisp", "34", "<Display Item Descriptions>"},
		{"sk_alldisp", "35", "<Maintain Item Descriptions>"},
		{"sk_alldisp", "36", "<Display Bar Codes>"},
		{"sk_alldisp", "37", "<Item User Defined Fields>"},
		{"sk_alldisp", "38", "<User Defined Fields>"},
		{"sk_alldisp", "39", "<Outlet Inventory>"},
		{"sk_alldisp", "40", "<Royalty>"},
		{"sk_alldisp", "41", "<Trans - All>"},
		{"sk_alldisp", "42", "<Trans - P/Orders>"},
		{"sk_alldisp", "43", "<Trans - Iss/Rec>"},
		{"sk_alldisp", "44", "<Trans - Inv/Crd>"},
		{"sk_alldisp", "45", "<Trans - Stock Adj>"},
		{"sk_alldisp", "46", "<Trans - Dir-Del>"},
		{"sk_alldisp", "47", "<Month Disp>"},
		{"sk_alldisp", "48", "<Requisitions>"},
		{"sk_alldisp", "49", "<Backorder Requisitions>"},
		{"sk_alldisp", "50", "<Forward Requisitions>"},
		{"sk_alldisp", "51", "<Works Orders>"},
		{"sk_alldisp", "52", "<W/O Mfg Item>"},
		{"sk_alldisp", "53", "<W/O Where Used>"},
		{"sk_alldisp", "54", "<Pre-Released QC Records>"},
		{"sk_alldisp", "55", "<Released QC Records>"},
		{"sk_alldisp", "56", "<Sales History>"},
		{"sk_alldisp", "57", "<Company>"},
		{"sk_alldisp", "58", "<Warehouse>"},
		{"so_cc_disp", "1", "<Credit Control Details.>"},
		{"so_cc_disp", "2", "<Master File Details.>"},
		{"so_cc_disp", "3", "<Transactions.>"},
		{"so_cc_disp", "4", "<Invoice Trans.>"},
		{"so_cc_disp", "5", "<Cheques.>"},
		{"so_cc_disp", "6", "<Cheque Details.>"},
		{"so_cc_disp", "7", "<Cheque History.>"},
		{"so_cc_disp", "8", "<Invoice D-D.>"},
		{"so_cc_disp", "9", "<B/Orders.>"},
		{"so_cc_disp", "10", "<Orders.>"},
		{"so_cc_disp", "11", "<D-D Orders.>"},
		{"so_cc_disp", "12", "<CCN's>"},
		{"so_cc_disp", "13", "<Note Pad.>"},
		{"so_cc_disp", "14", "<Add to Note Pad.>"},
		{"so_cc_disp", "15", "<Delivery Address(s).>"},
		{"so_cc_disp", "16", "<Specific Customer Pricing.>"},
		{"so_cc_disp", "17", "<Last Customer Pricing.>"},
		{"so_cc_disp", "18", "<Graph-This/Last year>"},
		{"so_cc_disp", "19", "<Graph-Payments/Invoice.>"},
		{"so_cc_disp", "20", "<Sales History>"},
		{"so_cc_disp", "21", "<Sales Analysis>"},
		{"so_cc_disp", "22", "<Market Info.>"},
		{"so_cc_disp", "23", "<Merchandiser Info.>"},
		{"so_cc_disp", "24", "<Rep Calls.>"},
		{"so_cc_disp", "25", "<Telesales Info>"},
		{"so_cc_disp", "26", "<Telesales Complaints>"},
		{"so_cc_disp", "27", "<Telesales Notes>"},
		{"so_cc_disp", "28", "<Telesales Last Call Notes>"},
		{"so_cc_disp", "29", "<Telesales Next Call Notes>"},
		{"so_cc_disp", "30", "<Total Screen.>"},
		{"so_cc_disp", "31", "<Groups.>"},
		{"", "", ""}
		};
#endif	/*	_optsInfo_h */
