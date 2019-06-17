/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_qc_mess.h,v 5.0 2001/06/19 06:51:33 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_qc_mess.h,v $
| Revision 5.0  2001/06/19 06:51:33  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:04  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_QC_MESS_H
#define	ML_QC_MESS_H

char	*mlQcMess001 = "Environment variable 'QC_APPLY' not set.",
		*mlQcMess002 = "Print QC Records",
		*mlQcMess003 = "Quality Control not required for this item.",
		*mlQcMess004 = "%cCannot sort by supplier and date.  Source type must be purchase order.",
		*mlQcMess005 = "Last Center: %4.4s",
		*mlQcMess006 = "QC Center Maintenance",
		*mlQcMess007 = "No QC records for release.",
		*mlQcMess008 = "No QC records to maintain.",
		*mlQcMess009 = "Total previous receipts plus this receipt %f is greater than original %f.",
		*mlQcMess010 = "Release plus rejected quantity %f greater than original quantity %f.",
		*mlQcMess011 = "Source Type           : %-14.14s",
		*mlQcMess012 = "Receipt Date          : %-10.10s",
		*mlQcMess013 = "Expected Release Date : %-10.10s",
		*mlQcMess014 = "First Expiry Period   : %-3.3s",
		*mlQcMess015 = "QC Stock Release",
		*mlQcMess016 = "Maintenance Of QC Released Stock",
		*mlQcMess017 = "Original Qty : %-11.11s Released Qty : %-11.11s Rejected Qty : %-11.11s",
		*mlQcMess018 = "Release quantity %f greater than original quantity %f.",
		*mlQcMess019 = "Quantity %14.6f greater than allowed quantity %f.",
		*mlQcMess020 = "Item Description    : %-30.30s",
		*mlQcMess021 = "Supplier Name       : %-30.30s",
		*mlQcMess022 = "Reason Description  : %-20.20s",
		*mlQcMess023 = "WOF Account         : %-16.16s",
		*mlQcMess024 = "QC Manual Entry",
		*mlQcMess025 = "Cannot issue %f because %f is/are available.",
		*mlQcMess026 = "QC location not found.",
		*mlQcMess027 = "Conversion factor is 0.00 for unit of measure.",

		*mlQcMess700 = "Usage : %s <lpno> <Maintenance/Release>";
#endif	
