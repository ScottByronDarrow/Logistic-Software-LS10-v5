/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_ca_mess.h,v 5.1 2002/02/19 09:36:12 scott Exp $
-----------------------------------------------------------------------
| $Log: ml_ca_mess.h,v $
| Revision 5.1  2002/02/19 09:36:12  scott
| Updated to fix prompts.
|
| Revision 5.0  2001/06/19 06:51:31  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:02  scott
| Updated to place strings back into include file so plook and other utils work
|
=====================================================================*/
#ifndef	ML_CA_MESS_H
#define	ML_CA_MESS_H

char	*mlCaMess001 = "Will delete reconciled transactions within date range...Continue [Y/N]? ",
		*mlCaMess002 = "Unreconciled transactions exist within date range...Continue with delete [Y/N]?",
		*mlCaMess003 = "Alternate Item already on file.",
		*mlCaMess004 = "Item source must be MC or RM.",
		*mlCaMess005 = "Alternate Item not found.",
		*mlCaMess006 = "Product is not within any specification.",
		*mlCaMess007 = "Conversion Factor must be greater than zero.",
		*mlCaMess008 = "Item source must be MC, MP, BM, or BP.",
		*mlCaMess009 = "This statement is already reconciled.",
		*mlCaMess010 = "Amount cleared is reconcilable.",
		*mlCaMess011 = "Statement has been reconciled. You can now save it.",
		*mlCaMess012 = "Statement has been unreconciled.  You can now save it.",
		*mlCaMess013 = "The statement is not yet reconciled.",
		*mlCaMess014 = "You have to unreconcile the statement first.",
		*mlCaMess015 = "Cash Flow Enquiry",
		*mlCaMess016 = "Transaction date must not be later than today's date.",
		*mlCaMess017 = "Delete this transaction [Y/N]?",
		*mlCaMess018 = "Amount must be negative for charge and petty cash.",
		*mlCaMess019 = "Amount must be positive for interest transactions.",
		*mlCaMess020 = "Costhead Maintenance",
		*mlCaMess021 = "A non-stock item must be entered.",
		*mlCaMess022 = "",
		*mlCaMess023 = "",
		*mlCaMess024 = "",
		*mlCaMess025 = "",
		*mlCaMess026 = "",
		*mlCaMess027 = "",
		*mlCaMess028 = "",
		*mlCaMess029 = "",
		*mlCaMess030 = "",
		*mlCaMess031 = "",
		*mlCaMess032 = "",
		*mlCaMess033 = "Obsolete Bank Transactions Deletion",
		*mlCaMess034 = "",
		*mlCaMess035 = "",
		*mlCaMess036 = "",
		*mlCaMess037 = "",
		*mlCaMess038 = "",
		*mlCaMess039 = "",
		*mlCaMess040 = "",
		*mlCaMess041 = "",
		*mlCaMess042 = "",
		*mlCaMess043 = "",
		*mlCaMess044 = "",
		*mlCaMess045 = "",
		*mlCaMess046 = "",
		*mlCaMess047 = "",
		*mlCaMess048 = "",
		*mlCaMess049 = "",
		*mlCaMess050 = "Reconciliation Report",
		*mlCaMess051 = "Choice may only be E)mployee or C)ontract",
		*mlCaMess052 = "Labor Analysis Report",
		*mlCaMess053 = "Equipment not found.",
		*mlCaMess054 = "Employee Maintenance",
		*mlCaMess055 = "Cost",
		*mlCaMess056 = "Charge Out",
		*mlCaMess057 = "Employees Mater Listing",
		*mlCaMess058 = "Please correct date.",
		*mlCaMess059 = "Account No.    %s",
		*mlCaMess060 = "Opening Balance   %16.16s",
		*mlCaMess061 = "Amount Cleared",
		*mlCaMess062 = "Closing Balance   %16.16s",
		*mlCaMess063 = "Debit            Credit               Net",
		*mlCaMess064 = "Amount Uncleared",
		*mlCaMess065 = "Reconcile System Transaction",
		*mlCaMess066 = "Reconcile System Transaction for the Statement Period %s %d.",
		*mlCaMess067 = "Unreconcile System Transaction for the Statement Period %s %d",
		*mlCaMess068 = "System Bank Transaction Enquiry",
		*mlCaMess069 = "Maintain System Bank Transactions";
#endif
