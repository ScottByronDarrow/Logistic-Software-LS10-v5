/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_sj_mess.h,v 5.0 2001/06/19 06:51:34 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_sj_mess.h,v $
| Revision 5.0  2001/06/19 06:51:34  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:04  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_SJ_MESS_H
#define	ML_SJ_MESS_H

char	*mlSjMess001 = "Vehicle/Labour/Service Person Rates Master File Report",
		*mlSjMess002 = "Master File Report By : ",
		*mlSjMess003 = "Labour Rates Maintenance",
		*mlSjMess004 = "Service Job not found.",
		*mlSjMess005 = "Service Job status is closed.",
		*mlSjMess006 = "Updating records ...",
		*mlSjMess007 = "Printing service credits and prebill invoices ...",
		*mlSjMess008 = "Sum of GL Allocation = %8.2f  Variance = %8.2f",
		*mlSjMess009 = "Service Prebill Invoicing",
		*mlSjMess010 = "Amount to allocate = %8.2f",
		*mlSjMess011 = "Service Job Details Maintenance",
		*mlSjMess012 = "Service Job Spare Parts Maintenance",
		*mlSjMess013 = "Labour rate code not found.",
		*mlSjMess014 = "Service Person Rates Maintenance",
		*mlSjMess015 = "Vehicle Maintenance",
		*mlSjMess016 = "Enter order number to cost:",
		*mlSjMess017 = "Select option :",
		*mlSjMess018 = " S(ervice Job %-s Display)",
		*mlSjMess019 = " P(rint Cost Sheet)",
		*mlSjMess020 = " D(isplay Cost Sheet) E(xit)",
		*mlSjMess021 = " Created invoice no %s, <return> to continue.",
		*mlSjMess022 = "Order Status : ",
		*mlSjMess023 = "Please note service job is not on hold.",
		*mlSjMess024 = "Release Held Service Job",
		*mlSjMess025 = "Service Job status is not C or O.",
		*mlSjMess026 = "Service Order Closure",
		*mlSjMess027 = "Has credit been approved [Y/N]?",
		*mlSjMess028 = "Do you want to enter job number manually [Y/N]?",
		*mlSjMess029 = "Credit limit exceeded.  Job will be placed on hold.",
		*mlSjMess030 = "Service Job Maintenance",
		*mlSjMess031 = "Created service job %ld",
		*mlSjMess032 = "Enter service job number : ",
		*mlSjMess033 = "Service job number already exists.",
		*mlSjMess034 = "Release Service Order Invoice",
		*mlSjMess035 = "Sum of GL Allocation = %8.2f   Variance = %8.2f, <RETURN> to continue",
		*mlSjMess036 = "Service Credits Entry",
		*mlSjMess037 = "Service job has not been invoiced, credit not allowed.",
		*mlSjMess038 = "Created credit note no %s, <return> to continue.",
		*mlSjMess039 = "Service Job Control Maintenance",
		*mlSjMess040 = "Service Job is held.",
		*mlSjMess041 = "Service Job Outside Purchases Log",
		*mlSjMess042 = "Order %ld not C or O.",
		*mlSjMess043 = "Department Selection",
		*mlSjMess044 = "Service Job Cost Sheet Print / Display",
		*mlSjMess045 = "Erroneous From Date.",
		*mlSjMess046 = "Erroneous To Date.",
		*mlSjMess047 = "Enter 'Y' Credit Approved / 'N' Not Approved  / 'M' More Details [Y/N/M]?",
		*mlSjMess048 = "Service Job Cost Sheet Print",
		*mlSjMess049 = "Service Invoice Summary",
		*mlSjMess050 = "Erroneous Date : %s",
		*mlSjMess051 = "Daily Job Card Entry",
		*mlSjMess052 = "Line: %3d  Customer No: %-6.6s Name: %-40.40s",
		*mlSjMess053 = "Extra Code(1): %s - %27.27s",
		*mlSjMess054 = "Extra Code(2): %s - %27.27s",
		*mlSjMess055 = "Extra Code(3): %s - %27.27s",
		*mlSjMess056 = "Extra Code(4): %s - %27.27s",
		*mlSjMess057 = "Service order is on hold.",
		*mlSjMess058 = "VEHICLE RATES MASTER FILE REPORT",
		*mlSjMess059 = "LABOUR RATES MASTER FILE REPORT",
		*mlSjMess060 = "SERVICE PERSON RATES MASTER FILE REPORT",
		*mlSjMess061 = "Purging Service Job",


		*mlSjMess700 = "Usage : %s <date>",
		*mlSjMess701 = "Usage : %s optional - <lpno> <rep_type>",
		*mlSjMess702 = "<rep_type> 1 - Vehicle Rates Master File",
		*mlSjMess703 = "           2 - Labour Rates Master File",
		*mlSjMess704 = "           3 - Service Person Rates Master File",
		*mlSjMess705 = "Usage : %s <program description> <lpno> <from date> <to date>",
		*mlSjMess706 = "Usage : %s <lpno> <invoice no.>",
		*mlSjMess707 = "Usage : %s <purge date>",
		*mlSjMess708 = "Usage : %s <output to> <Cust no | ALL> <order status, O(pen| C(losed| I(nvoiced> <lpno> ",
		*mlSjMess709 = "Usage : %s <output to> <order no> <lpno>",
		*mlSjMess710 = "Output to P(rinter) or D(isplay) only.",
		*mlSjMess711 = "Usage : %s <program name>",
		*mlSjMess712 = "Usage : %s <prog_desc> <lpno>";
#endif	
