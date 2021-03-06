/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_ts_mess.h,v 5.0 2001/06/19 06:51:46 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_ts_mess.h,v $
| Revision 5.0  2001/06/19 06:51:46  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/04 08:56:54  scott
| Updated for message changes
|
| Revision 4.0  2001/03/09 00:59:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:05  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_TS_MESS_H
#define	ML_TS_MESS_H

char	*mlTsMess001 = "Activate Customer For Telesales",
		*mlTsMess002 = "Customer is not active for telesales.",
		*mlTsMess003 = "Telesales Complaints Report",
		*mlTsMess004 = "Telesales Last Call Notes Report",
		*mlTsMess005 = "Telesales Call Notes Report",
		*mlTsMess006 = "Telesales Next Visit Notes Report",
		*mlTsMess007 = "Telesales Next Visit Report",
		*mlTsMess008 = "Customer must operate in local currency.",
		*mlTsMess009 = "Date must be greater than today.",
		*mlTsMess010 = "Complimentary Item Maintenance",
		*mlTsMess011 = "Leads In Bulk Assign",
		*mlTsMess012 = "Leads In Bulk Unassign",
		*mlTsMess013 = "Manual Assignment Of Leads",
		*mlTsMess014 = "This is not a follow up letter.",
		*mlTsMess015 = "Customer is not allocated to you.",
		*mlTsMess016 = "Forward order, delay release [Y/N/?",
		*mlTsMess017 = "Customer is not active for telesales.  Activate [Y/N]?",
		*mlTsMess018 = "Has credit been approved [Y/N]? ",
		*mlTsMess019 = "You do not have any leads allocated to you for today.",
		*mlTsMess020 = "Minimum margin for category was not obtained.",
		*mlTsMess021 = "Adding order detail lines ...",
		*mlTsMess022 = "No packing slip will be printed as all lines are backordered.",
		*mlTsMess023 = "Nett Total:     ",
		*mlTsMess024 = "Fri/Other Total:     ",
		*mlTsMess025 = "%-3.3s    Total:     ",
		*mlTsMess026 = "Tax   Total:     ",
		*mlTsMess027 = "TOTAL  :     ",
		*mlTsMess028 = "Last Phone Date : %s / Rep Code : %s / Order Ref : %40.40s",
		*mlTsMessb28 = "Last Phone Date : %s / Rep Code : %s",
		*mlTsMess029 = "Last 12 months : %12.2f",
		*mlTsMess030 = "Last Order Date: %10.10s",
		*mlTsMess031 = "Sales order not created as no lines are present.",
		*mlTsMess032 = "Telesales Order Entry.",
		*mlTsMess033 = "Maintain Call Notes",
		*mlTsMess034 = "Credit Control Notes",
		*mlTsMess035 = "Sales History",
		*mlTsMess036 = "Promotional Items",
		*mlTsMess037 = "Brand Promotions",
		*mlTsMess038 = "Tag Mailers",
		*mlTsMess039 = "Parcel Promotion",
		*mlTsMess040 = "Item History",
		*mlTsMess041 = "Arrange Visit",
		*mlTsMess042 = "Sales Details",
		*mlTsMess043 = "Telesales Planned Call",
		*mlTsMess044 = "Telesales Mailer Call",
		*mlTsMess045 = "Telesales Random Call",
		*mlTsMess046 = "Telesales Receive Call",
		*mlTsMess047 = "Telesales Call",
		*mlTsMess048 = "Item Negotiation",
		*mlTsMess049 = "Call Date",
		*mlTsMess050 = "Contract Details",
		*mlTsMess051 = "Not a label definition mailer.",
		*mlTsMess052 = "Invalid mailer format for printing labels.",
		*mlTsMess053 = "Telesales Label Printing",
		*mlTsMess054 = "Brand Maintenance",
		*mlTsMess055 = "Last Brand : %s",
		*mlTsMess056 = "Set Phone Frequency For Telesales Customer",
		*mlTsMess057 = "Creating letter record ",
		*mlTsMess058 = "Updating letter record ",
		*mlTsMess059 = "Deleting letter record ",
		*mlTsMess060 = "Letter Input",
		*mlTsMess061 = "Contract no longer current - expired %s.",
		*mlTsMess062 = "Contract not yet current - effective %s.",
		*mlTsMess063 = "Contract not assigned to this Customer.",
		*mlTsMess064 = "Class Z (description items) cannot have a quantity.",
		*mlTsMess065 = "Backorder quantity of %8.2f exceeds total order quantity of %8.2f.",
		*mlTsMess066 = "Item is a bonus item - price not allowed.",
		*mlTsMess067 = "Return not valid, press [RESTART] key.",
		*mlTsMess068 = "Customer does not exist. Create [Y/N]?",
		*mlTsMess069 = "Enter 'Y' if credit is approved / 'N' if not approved / 'M' for more details on credit required [Y/N/M]?",
		*mlTsMess070 = "Item no %s has been superceeded by item no %s %c.",
		*mlTsMess071 = "Item no %s has been superceeded by item no %s. Item %s not found.",
		*mlTsMess072 = "Product has complimentary products. Examine [Y/N]?",
		*mlTsMess073 = "Item has a contract price.  Negotiation window not available.",
		*mlTsMess074 = "You must order some of the original items or Fn1 to abort.",
		*mlTsMess075 = "Input Cost Price :",
		*mlTsMess076 = "Order %s is on credit hold as existing orders plus this order exceedscredit terms.",
		*mlTsMess077 = "Order %s is on credit hold as lines did not obtain minimum margin percent.",
		*mlTsMess078 = "Customer Number  %-6.6s  (%40.40s)  Head Office: %-6.6s",
		*mlTsMessb78 = "Customer Number  %-6.6s  (%40.40s)",
		*mlTsMess079 = "Phone Number     %-10.10s  Contact : %-30.30s  Alternate Contact: %-30.30s",
		*mlTsMessb79 = "Phone Number     %-10.10s  Contact : %-30.30s",
		*mlTsMess080 = "Arrow Keys - Select window [EDIT/END]-Exit",
		*mlTsMess081 = "Mailer format not found.",
		*mlTsMess082 = "Error with customer record.",
		*mlTsMess083 = "Do you want to supply order in full to customer %s [Y/N]?",
		*mlTsMess084 = "Item same as master item.",
		*mlTsMess085 = "Item already complimentary.",
		*mlTsMess086 = "No current logname.",
		*mlTsMess087 = "Sales:   Mtd: %12.2f    Ytd: %12.2f",
		*mlTsMess088 = "Last Order Val: %12.2f",
		*mlTsMess089 = "%s(O)verride%s %s(C)ancel%s %s(R)educe%s %sDisp (N)(A)%s ",
		*mlTsMess090 = "%s(F)orce b/o%s ",
		*mlTsMess091 = "%s(B)ackorder bal%s  (F)orce b/o%s ",
		*mlTsMess092 = "Sales Order (%s) created.",
		*mlTsMess093 = "Cannot read letter lines.",
		*mlTsMess094 = "Label format .LBL_HZ and .LBL_VT must be defined on line 1.",
		*mlTsMess095 = " Telesales Customer Call Cycle Report. ",
		*mlTsMess096 = "This is a Label Definition Letter.",
		*mlTsMess097 = " Print Telesales Mailers ",

		*mlTsMess700 = "Usage : %s <description> <note type>",
		*mlTsMess701 = " OR   : %s <lpno>",
		*mlTsMess702 = "           <start operator>",
		*mlTsMess703 = "           <end operator>",
		*mlTsMess704 = "           <start customer>",
		*mlTsMess705 = "           <end customer>",
		*mlTsMess706 = "           <start rep.>",
		*mlTsMess707 = "           <end rep.>",
		*mlTsMess708 = "           <note type>",
		*mlTsMess709 = "Usage : %s <description>",
		*mlTsMess710 = " OR   :    <lpno>",
		*mlTsMess711 = "           <visit date>",
		*mlTsMess712 = "Usage : %s <creat_flag> <screen_file> <department_no> optional <lpno> ",
		*mlTsMess713 = "       creat_flag - R(elease Order Automatically|M(anual Realease|L(ate Release|S(tandard Order-No release",
		*mlTsMess714 = "%s(S)ubstitute %s ",
		*mlTsMess715 = "Minimum Margin for category is %.2f%% and margin obtained only %.2f%% %c - Press Return. ",
		*mlTsMess716 = "WARNING only %.2f Available for part # %s",
		*mlTsMess717 = "LAST CALL NOTES",
		*mlTsMess718 = "NOTES",
		*mlTsMess719 = "COMPLAINTS",
		*mlTsMess720 = "Use Window-Activate Key to run Telesales ring menu.",
		*mlTsMess721 = "          <start date>",
		*mlTsMess722 = "          <end date>";
#endif	
