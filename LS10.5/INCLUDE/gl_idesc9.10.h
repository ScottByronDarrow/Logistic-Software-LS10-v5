/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_idesc9.10.h,v 5.3 2001/08/20 23:15:22 scott Exp $
|	Name : gl_idesc9.10.h
|	Desc : (interface pre-set structure)
|=====================================================================|
| $Log: gl_idesc9.10.h,v $
| Revision 5.3  2001/08/20 23:15:22  scott
| Updated for development related to bullet proofing
|
=====================================================================*/
	struct	{
		char	*_ifcode;
		char	*_ifdesc;
		int		_active;
		int		_byBrNo;
		int		_byWhNo;
		int		_byClassType;
		int		_byCategory;

	} gl_idesc[] = {
	{"ACCT PAY  ", "ACCOUNTS PAYABLE                        ",-1,1,-1,-1,-1},
	{"ACCT REC  ", "ACCOUNT RECEIVABLE                      ",-1,1,-1,1,-1},
	{"BANK ACCT ", "BANK                                    ",-1,1,-1,-1,-1},
	{"CANCEL CHQ", "CANCELLED CHEQUES                       ",-1,1,-1,-1,-1},
	{"COSTSALE C", "COST OF SALES CREDIT                    ",-1,1,1,1,1},
	{"COSTSALE D", "COST OF SALES DEBIT                     ",-1,1,1,1,1},
	{"COSTSALE M", "COST OF SALE - MANUFACTURING (CREDIT)   ",-1,1,1,1,1},
	{"CREDIT    ", "SALES - CREDIT NOTES                    ",-1,1,1,1,1},
	{"DD CST CST", "DIRECT DELIVERY ON COSTS - COST OF SALE ",-1,1,1,1,1},
	{"DD CST SAL", "DIRECT DELIVERY ON COSTS - SALES        ",-1,1,0,-1,0},
	{"DD CST SAL", "DIRECT DELIVERY ON COSTS - SALES        ",-1,1,-1,-1,-1},
	{"DFT EXPEN.", "DEFAULT EXPENSE ACCOUNT                 ",-1,1,-1,-1,-1},
	{"DFT REVEN.", "DEFAULT REVENUE ACCOUNT                 ",-1,1,-1,-1,-1},
	{"DISC ALLOW", "DISCOUNTS ALLOWED                       ",-1,1,-1,-1,-1},
	{"DISCOUNT  ", "DISCOUNT ACCOUNT                        ",-1,1,-1,1,-1},
	{"DISC REC  ", "DISCOUNTS RECEIVED                      ",-1,1,-1,1,-1},
	{"DUTY PAY  ", "DUTY PAYABLE                            ",-1,1,1,1,1},
	{"EXCH VAR. ", "EXCHANGE VARIATION                      ",-1,1,-1,1,-1},
	{"FREIGHT   ", "FREIGHT                                 ",-1,1,-1,1,-1},
	{"G.S.T CHRG", "GST CHARGED                             ",-1,1,-1,1,-1},
	{"G.S.T PAID", "GST PAYABLE                             ",-1,1,-1,-1,-1},
	{"INSURANCE ", "INSURANCE                               ",-1,1,-1,1,-1},
	{"INVENTORY ", "INVENTORY                               ",-1,1,1,-1,1},
	{"INVOICE   ", "SALES - INVOICES                        ",-1,1,1,1,1},
	{"INV SUSPEN", "INVENTORY SUSPENCE                      ",-1,1,1,1,1},
	{"ITEM LEVY ", "INVENTORY ITEM LEVY                     ",-1,1,1,1,1},
	{"JOURNAL   ", "CUSTOMER JOURNALS                       ",-1,1,-1,1,-1},
	{"MAN D LABR", "MAN. VAR. - DIRECT LABOUR COSTS         ",-1,1,1,-1,1},
	{"MAN D MACH", "MAN. VAR. - DIRECT MACHINE COSTS        ",-1,1,1,-1,1},
	{"MAN D MATL", "MAN. VAR. - DIRECT MATERIAL COSTS       ",-1,1,1,-1,1},
	{"MAN D OTH ", "MAN. VAR. - DIRECT OTHER COSTS          ",-1,1,1,-1,1},
	{"MAN D QC  ", "MAN. VAR. - DIRECT QUALITY QC CHK COSTS ",-1,1,1,-1,1},
	{"MAN D SPEC", "MAN. VAR. - DIRECT SPECIAL COSTS        ",-1,1,1,-1,1},
	{"MAN F LABR", "MAN. VAR. - FIXED O/H FOR LABOUR COSTS  ",-1,1,1,-1,1},
	{"MAN F MACH", "MAN. VAR. - FIXED O/H FOR MACHINE COSTS ",-1,1,1,-1,1},
	{"MAN F OTH ", "MAN. VAR. - FIXED O/H FOR OTHER COSTS   ",-1,1,1,-1,1},
	{"MAN F QC  ", "MAN. VAR. - FIXED O/H FOR QC CHECK COSTS",-1,1,1,-1,1},
	{"MAN F SPEC", "MAN. VAR. - FIXED O/H FOR SPECIAL COSTS ",-1,1,1,-1,1},
	{"MIS SK ISS", "MISCELLANEOUS STOCK ISSUES              ",-1,1,1,-1,1},
	{"MIS SK REC", "MISCELLANEOUS STOCK RECEIPTS            ",-1,1,1,-1,1},
	{"OTHER CST1", "OTHER COST AMOUNT #1                    ",-1,1,-1,1,-1},
	{"OTHER CST2", "OTHER COST AMOUNT #2                    ",-1,1,-1,1,-1},
	{"OTHER CST3", "OTHER COST AMOUNT #3                    ",-1,1,-1,1,-1},
	{"PO ACOST V", "PURCHASE ORDER ACTUAL COST VARIANCE     ",-1,1,1,-1,1},
	{"PO CLEAR D", "PURCHASE CLEARING ACCOUNT - DIR DEL PO's",-1,1,-1,-1,1},
	{"PO CLEAR N", "PURCHASE CLEARING ACCOUNT - NORMAL  PO's",-1,1,-1,-1,1},
	{"PO CLEAR Q", "PURCHASE CLEARING ACCOUNT - QUICK   PO's",-1,1,-1,-1,1},
	{"POS_AMEXCO", "POS-AMEXCO- POS AMERICAN EXPRESS COMM.  ",0,1,-1,-1,-1},
	{"POS_AMEXCU", "POS-AMEXCU- POS AMERICAN EXPRESS CUST.  ",0,1,-1,-1,-1},
	{"POS_ASSM  ", "POS-ASSM  - POS ASSEMBLY CHARGES        ",0,1,-1,-1,-1},
	{"POS_CASHC ", "POS-CASHC - POS CASH COUPON             ",0,1,-1,-1,-1},
	{"POS_CASH  ", "POS-CASH  - POS CASH SALE ACCOUNT       ",0,1,-1,-1,-1},
	{"POS_C_C_CS", "POS-C&C CS- POS C&C COST OF SALE        ",0,1,-1,-1,-1},
	{"POS_C_C_SA", "POS-C&C SA- POS C&C SALES               ",0,1,-1,-1,-1},
	{"POS_CHEQUE", "POS-CHEQUE- POS CHEQUE PAYMENT ACCOUNT  ",0,1,-1,-1,-1},
	{"POS_CUSDIS", "POS-CUSDIS- POS CUSTOMER DISCOUNT       ",0,1,-1,-1,-1},
	{"POS_DEL   ", "POS-DEL   - POS DELIVERY CHARGES        ",0,1,-1,-1,-1},
	{"POS_DEPOFF", "POS-DEPOFF- POS DEPOSIT OFFSET          ",0,1,-1,-1,-1},
	{"POS_DEPREC", "POS-DEPREC- POS DEPOSIT RECEIPT         ",0,1,-1,-1,-1},
	{"POS_DINRCO", "POS-DINRCO- POS DINERS CLUB COMMISSION. ",0,1,-1,-1,-1},
	{"POS_DINRCU", "POS-DINRCU- POS DINERS CLUB CUSTOMER.   ",0,1,-1,-1,-1},
	{"POS_ESPCO ", "POS-ESPCO - POS ESP COMMISSION.         ",0,1,-1,-1,-1},
	{"POS_ESPCU ", "POS-ESPCU - POS ESP CUSTOMER.           ",0,1,-1,-1,-1},
	{"POS_EXCESS", "POS-EXCESS- POS EXCESS PAYMENT          ",0,1,-1,-1,-1},
	{"POS_EXDISP", "POS-EXDISP- POS EX-DISPLAY DISCOUNT     ",0,1,-1,-1,-1},
	{"POS_GIFTV ", "POS-GIFTV - POS GIFT VOUCHER RECEIPTS   ",0,1,-1,-1,-1},
	{"POS_GV_SA ", "POS-GV SA - POS GIFT VOUCHER SALES      ",0,1,-1,-1,-1},
	{"POS_IMBAL ", "POS-IMBAL - POS IMBALANCE.              ",0,1,-1,-1,-1},
	{"POS_JARD  ", "POS-JARD  - POS JARDINE DISCOUNT        ",0,1,-1,-1,-1},
	{"POS_MASTCO", "POS-MASTCO- POS MASTER CARD COMMISSION. ",0,1,-1,-1,-1},
	{"POS_MASTCU", "POS-MASTCU- POS MASTER CARD CUSTOMER.   ",0,1,-1,-1,-1},
	{"POS_REFPAY", "POS-REFPAY- POS REFUNDS PAYABLE         ",0,1,-1,-1,-1},
	{"POS_RET_CS", "POS-RET CS- POS RETURNS COST OF SALE    ",0,1,-1,-1,-1},
	{"POS_RET_SA", "POS-RET SA- POS RETURNS                 ",0,1,-1,-1,-1},
	{"POS_SO_CS ", "POS-SO CS - POS SO COST OF SALE         ",0,1,-1,-1,-1},
	{"POS_SO_SA ", "POS-SO SA - POS SO SALES                ",0,1,-1,-1,-1},
	{"POS_STAFFD", "POS-STAFFD- POS STAFF DISCOUNT          ",0,1,-1,-1,-1},
	{"POS_STOCK ", "POS-STOCK - POS STOCK                   ",0,1,-1,-1,-1},
	{"POS_TRPAYM", "POS-TRPAYM- POS TRANSFER OF PAYMENT     ",0,1,-1,-1,-1},
	{"POS_VISACO", "POS-VISACO- POS VISA CARD COMMISSION.   ",0,1,-1,-1,-1},
	{"POS_VISACU", "POS-VISACU- POS VISA CARD CUSTOMER.     ",0,1,-1,-1,-1},
	{"POVARIANCE", "PURCHASE ORDER VARIANCE                 ",-1,1,-1,-1,-1},
	{"PURCHASE C", "PURCHASE CREDIT                         ",-1,1,1,-1,1},
	{"PURCHASE D", "PURCHASE DEBIT                          ",-1,1,1,-1,1},
	{"REC D LABR", "RECOVERY - DIRECT LABOUR COSTS          ",-1,1,1,-1,1},
	{"REC D MACH", "RECOVERY - DIRECT MACHINE COSTS         ",-1,1,1,-1,1},
	{"REC D OTH ", "RECOVERY - DIRECT OTHER COSTS           ",-1,1,1,-1,1},
	{"REC D QC  ", "RECOVERY - DIRECT QUALITY QC CHECK COSTS",-1,1,1,-1,1},
	{"REC D SPEC", "RECOVERY - DIRECT SPECIAL COSTS         ",-1,1,1,-1,1},
	{"REC F LABR", "RECOVERY - FIXED O/H FOR LABOUR COSTS   ",-1,1,1,-1,1},
	{"REC F MACH", "RECOVERY - FIXED O/H FOR MACHINE COSTS  ",-1,1,1,-1,1},
	{"REC F OTH ", "RECOVERY - FIXED O/H FOR OTHER COSTS    ",-1,1,1,-1,1},
	{"REC F QC  ", "RECOVERY - FIXED O/H FOR QC CHECK COSTS ",-1,1,1,-1,1},
	{"REC F SPEC", "RECOVERY - FIXED O/H FOR SPECIAL COSTS  ",-1,1,1,-1,1},
	{"RESTCK FEE", "RE-STOCKING FEE                         ",-1,1,-1,1,-1},
	{"SALES TAX ", "SALES TAX                               ",-1,1,-1,1,-1},
	{"SOS       ", "SMALL ORDER SURCHARGE                   ",-1,1,-1,1,-1},
	{"STAKE VAR.", "STOCK TAKE VARIATION                    ",-1,1,1,-1,1},
	{"STK ADJ CR", "STOCK ADJUSTMENT CREDIT                 ",-1,1,1,-1,1},
	{"STK ADJ DB", "STOCK ADJUSTMENT DEBIT                  ",-1,1,1,-1,1},
	{"SUSPENSE  ", "SUSPENSE                                ",-1,1,-1,-1,-1},
	{"TRANS-BR  ", "INTER-BRANCH TRANSFERS                  ",-1,1,-1,-1,1},
	{"TRANS-CO  ", "INTER-COMPANY TRANSFERS                 ",-1,1,-1,-1,1},
	{"WIP D LABR", "WIP - DIRECT LABOUR COSTS               ",-1,1,1,-1,1},
	{"WIP D MACH", "WIP - DIRECT MACHINE COSTS              ",-1,1,1,-1,1},
	{"WIP D MATL", "WIP - DIRECT MATERIAL COSTS             ",-1,1,1,-1,1},
	{"WIP D OTH ", "WIP - DIRECT OTHER COSTS                ",-1,1,1,-1,1},
	{"WIP D QC  ", "WIP - DIRECT QUALITY QC CHECK COSTS     ",-1,1,1,-1,1},
	{"WIP D SPEC", "WIP - DIRECT SPECIAL COSTS              ",-1,1,1,-1,1},
	{"WIP F LABR", "WIP - FIXED OVERHEAD FOR LABOUR COSTS   ",-1,1,1,-1,1},
	{"WIP F MACH", "WIP - FIXED OVERHEAD FOR MACHINE COSTS  ",-1,1,1,-1,1},
	{"WIP F OTH ", "WIP - FIXED OVERHEAD FOR OTHER COSTS    ",-1,1,1,-1,1},
	{"WIP F QC  ", "WIP - FIXED OVERHEAD FOR QC CHECK COSTS ",-1,1,1,-1,1},
	{"WIP F SPEC", "WIP - FIXED OVERHEAD FOR SPECIAL COSTS  ",-1,1,1,-1,1},
	{"","",0,0,0,0,0}
};
