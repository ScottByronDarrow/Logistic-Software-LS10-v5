/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_rg_mess.h,v 5.0 2001/06/19 06:51:34 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_rg_mess.h,v $
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
#ifndef	ML_RG_MESS_H
#define	ML_RG_MESS_H

char	*mlRgMess001 = "Routing Copy",
		*mlRgMess002 = "Routing Print    ",
		*mlRgMess003 = "Resource Maintenance",
		*mlRgMess004 = "Invalid type. Select from DVC, DFC, IVC, or IFC.",
		*mlRgMess005 = "Display Resource Details",
		*mlRgMess006 = "Display Resource Details Restricted",
		*mlRgMess007 = "Rate Maintenance",
		*mlRgMess008 = "Resource Type not found.",
		*mlRgMess009 = "Work Centre not found.",
		*mlRgMess010 = "Concurrent processes must occur in the same work centre.  Changing first changes all.",
		*mlRgMess011 = "Resource not found.",
		*mlRgMess012 = "Routing Maintenance",
		*mlRgMess013 = "Yield calculation not found.",
		*mlRgMess014 = "Only Yes, No or Same are valid.",
		*mlRgMess015 = "%R Rate/Hr: %12.2f  Var.Ovhd: %12.2f Fixed Ovhd: %12.2f",
		*mlRgMess016 = "Out of sequence.",

		*mlRgMess700 = "OR    : %s <start_item> <end_item> <st_alternate> < end_alternate> <lpno> ",
		*mlRgMess701 = "Usage : %s <C(ode)|T(ype)> <N(ormal)|R(estricted)>",
		*mlRgMess702 = "Usage : %s <C(ode)|T(ype)>";
#endif	
