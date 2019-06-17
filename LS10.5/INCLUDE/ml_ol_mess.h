/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_ol_mess.h,v 5.0 2001/06/19 06:51:33 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_ol_mess.h,v $
| Revision 5.0  2001/06/19 06:51:33  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:04  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_OL_MESS_H
#define	ML_OL_MESS_H

char	*mlOlMess001 = "Processing %s %-16.16s",
		*mlOlMess002 = "Usage : %s <lpno> <print_flags> - optional <status_flags>",
		*mlOlMess003 = "Company  Branch  Name%sStatus   ",
		*mlOlMess004 = "Option : ",
		*mlOlMess005 = " Currency code of Customer is different to native currency.",
		*mlOlMess006 = "Proof total %8.2f not equal to invoice total %8.2f",
		*mlOlMess007 = "Item has a contract price, negotiation window not available.",
		*mlOlMess008 = "Cash Draw %s",
		*mlOlMess009 = "%s (S)ubstitute %s",
		*mlOlMess010 = "Online Invoicing",
		*mlOlMess011 = "Credit Note Input",
		*mlOlMess012 = "Manual Online Invoicing",
		*mlOlMess013 = "Manual Credit Note Input",
		*mlOlMess014 = "Invoice Display",
		*mlOlMess015 = "Credit Note Display",
		*mlOlMess016 = "SEL Security Check",
		*mlOlMess017 = "                       On-line Branch Status Display                    ",
		*mlOlMess018 = "Creating lock for %s",
		*mlOlMess019 = "Removing lock for %s",
		*mlOlMess020 = "%s locked out because of end of day process.",
		*mlOlMess021 = "Branch Security File Maintenance",
		*mlOlMess022 = "Cannot invoice serial items.",
		*mlOlMess023 = "Cannot create invoice with no line details.",
		*mlOlMess024 = "Cannot abandon invoice that has not been saved.",
		*mlOlMess025 = "%s No. %s on file for a different Customer.",
		*mlOlMess026 = "%s No. %s has been processed.",
		*mlOlMess027 = "Warning: Only %.2f available for part #%s and you are invoicing %.2f.",
		*mlOlMess028 = "Amount tendered %8.2f less than invoice total %8.2f",
		*mlOlMess029 = "Usage %s <Type_lock>",
		*mlOlMess030 = "Usage : %s <OP_ID> <batch_flag> <creat_flag> <type_flag> <text_file> - optional <lpno>",
		*mlOlMess031 = "Usage : %s <Co> <Br> <Prog> <arguments>",
		*mlOlMess032 = "Usage : %s <I)ncrease/D)ecrease>",
		*mlOlMess033 = "Input Cost Price :",
		*mlOlMess034 = "Customer Order No. %-15.15s  Delivery %-40.40s",
		*mlOlMess035 = "Order ref   %-15.15s",
		*mlOlMess036 = "Net Total :",
		*mlOlMess037 = "Freight Total :",
		*mlOlMess038 = "Total:",
		*mlOlMess039 = "TOTAL   :",
		*mlOlMess040 = "Batch Total = %-8.2f",
		*mlOlMess041 = "Last %s:%6s/%s",
		*mlOlMess042 = "Minimum margin for category was not obtained.  Press return.",
		*mlOlMess043 = "Minimum margin for category is %.2f%% and margin obtained only %.2f%%.  Press return.",
		*mlOlMess044 = "Proof total %-8.2f is not equal to credit total %-8.2f.",
		*mlOlMess045 = "Confirmed - Running",
		*mlOlMess046 = "Error in running option.",
		*mlOlMess047 = "Usage : %s <type_lock [L(ock out branch)/U(nlock branch)/C(heck for lock)]>",
		*mlOlMess048 = "Usage : %s <op_id> <batch_flag [Y/N]> <creat_flag> <type_flag [O(n-line Invoice)/R(efund (On-line Credit Note))]> <text_file> - optional <lpno>",
		*mlOlMess049 = "Usage : %s <lpno> <B(efore)/S(aved)> ",
		*mlOlMess050 = "Contract is not assigned to customer.",
		*mlOlMess051 = "Qty ordered must be greater than qty supplied.",
		*mlOlMess052 = "Qty supplied must be greater than 0.00",
		*mlOlMess053 = "Cost price for this item can not be entered.",
		*mlOlMess054 = "Sale price is lower than inventory price.",
		*mlOlMess055 = "Serial item is already committed.",
		*mlOlMess056 = "Serial item is already sold.",
		*mlOlMess057 = "Serial item is exists. Credit not available.",
		*mlOlMess058 = "Invoice belongs to another customer.",
		*mlOlMess059 = "Contract no longer current - Expired %s.",
		*mlOlMess060 = "Contract not yet current - Effective %s.",
		*mlOlMess061 = "Do you want to accept this serial item [Y/N] ?",
		*mlOlMess062 = "Enter 'Y' if credit is approved/ 'N' if not approved/ 'M' for more details on credit required [Y/N/M] ?",
		*mlOlMess063 = "Has credit been approved [Y/N] ?",
		*mlOlMess064 = "Part is not at this warehouse. Create [Y/N]?",
		*mlOlMess065 = "Password already exists.",
		*mlOlMess066 = "Qty supplied must be lesser than qty ordered.",
		*mlOlMess067 = "%s(C)ancel%s  %sDisplays (N)(A)%s  ",
		*mlOlMess068 = "%s(O)verride%s  %s(C)ancel%s  %s(R)educe%s  %sDisplays (N)(A)%s  ",
		*mlOlMess069 = "Invoice",
		*mlOlMess070 = "Credit Note",
		*mlOlMess071 = " [FN3] ",
		*mlOlMess072 = " [FN14] ",
		*mlOlMess073 = " [FN15] ",
		*mlOlMess074 = " [FN16] ",
		*mlOlMess075 = "Minimum margin for category was not obtained - Accept Yes/No ?",
		*mlOlMess076 = "Minimum margin for category is %.2f%% and margin obtained only %.2f%% - Accept Yes/No ?",
		*mlOlMess077 = "Please complete the open kit on the item screen",
		*mlOlMess078 = "%s(C)ancel%s  %s(R)educe%s  %sDisplays (N)(A)%s  ";
#endif	
