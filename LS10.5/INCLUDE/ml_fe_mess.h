/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_fe_mess.h,v 5.0 2001/06/19 06:51:32 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_fe_mess.h,v $
| Revision 5.0  2001/06/19 06:51:32  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:03  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_FE_MESS_H
#define	ML_FE_MESS_H

char	*mlFeMess001 = "Create Forward Exchange Contract",
		*mlFeMess002 = "Close Forward Exchange Contract",
		*mlFeMess003 = "Forward Exchange Contract Enquiry",
		*mlFeMess004 = "Forward Exchange Contract Allocation",
		*mlFeMess005 = "Contract %s is on file for different currency.",
		*mlFeMess006 = "Forward Exchange Contract is not active.",
		*mlFeMess007 = "Contract no longer current, expired %s.",
		*mlFeMess008 = "Contract not yet current, effective %s.",
		*mlFeMess009 = "Invoice Nos. cannot be changed.",
		*mlFeMess010 = "Invoice %s is on file for different currency.",
		*mlFeMess011 = "Invoice %s has already been paid or partly paid.",
		*mlFeMess012 = "Invoice %s has zero balance.",
		*mlFeMess013 = "Invoice amount is greater than amount remaining on contract.",
		*mlFeMess014 = "Invoice is already assigned to this contract.",
		*mlFeMess015 = "Invoice is already assigned to this contract. Override [Y/N]?",
		*mlFeMess016 = "Invoice has a fixed exchange rate and cannot be assigned.",
		*mlFeMess017 = "Usage : %s <lpno> <pid>",
		*mlFeMess018 = "Invoice %s is already assigned to contract %s, override [Y/N]?",
		*mlFeMess019 = "Processing Account Summary : %s",
		*mlFeMess020 = "Currency Exposure Report",
		*mlFeMess021 = "Usage : %s <M(aintenance | C(losure | E(nquiry> ",
		*mlFeMess022 = "Effective date > Expiry date",
		*mlFeMess023 = "Effective date < Current date",
		*mlFeMess024 = "Expiry date <= Effective date",
		*mlFeMess025 = "Forward exchange contracts not yet available for purchasing.",
		*mlFeMess026 = "Buy-sell type not found.",
		*mlFeMess027 = "Contract amount is less than the amount already allocated.",
		*mlFeMess028 = "Invalid active status",
		*mlFeMess029 = "Print Currency Exposure Report";
#endif	
