/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_bm_mess.h,v 5.1 2002/03/05 02:51:15 scott Exp $
|----------------------------------------------------------------------
| $Log: ml_bm_mess.h,v $
| Revision 5.1  2002/03/05 02:51:15  scott
| S/C 00825
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
#ifndef	ML_BM_MESS_H
#define	ML_BM_MESS_H

char	*mlBmMess001 = "Cannot exclude the current class.",
		*mlBmMess002 = "Class already excluded.",
		*mlBmMess003 = "Alternate Item already on file.",
		*mlBmMess004 = "Item source must be MC or RM.",
		*mlBmMess005 = "Alternate Item not found.",
		*mlBmMess006 = "Product is not within any specification.",
		*mlBmMess007 = "Conversion Factor must be greater than zero.",
		*mlBmMess008 = "Item source must be MC, MP, BM, or BP.",
		*mlBmMess009 = "Product is part of another/old specification.",
		*mlBmMess010 = "Class Exclusion Maintenance.",
		*mlBmMess011 = "Enter the conversion factor to get to %s from %s.",
		*mlBmMess012 = "Substitute Material Usage",
		*mlBmMess013 = "Substitute existing item for new item.",
		*mlBmMess014 = "Hazard Class Maintenance",
		*mlBmMess015 = "Last Class: %s",
		*mlBmMess016 = "Usage: %s <H(andling) ha(Z)ard>",
		*mlBmMess017 = "Undefined Bill Report",
		*mlBmMess018 = "Maintain new bill [Y/N]?",
		*mlBmMess019 = "Copying specification.",
		*mlBmMess020 = "Copy Product Specification.",
		*mlBmMess021 = "Invalid quantity.",
		*mlBmMess022 = "Quantity is below minimum batch size.",
		*mlBmMess023 = "Quantity is above maximum batch size.",
		*mlBmMess024 = "Material Usage Display (Detailed)",
		*mlBmMess025 = "Material Usage Display (Summary)",
		*mlBmMess026 = "Units of Measure",
		*mlBmMess027 = "Batch Sizes",
		*mlBmMess028 = "Std. %14.6f",
		*mlBmMess029 = "Min. %14.6f",
		*mlBmMess030 = "Max. %14.6f",
		*mlBmMess031 = "Where Used Display",
		*mlBmMess032 = "Delete Product Specification",
		*mlBmMess033 = "Material Usage Report",
		*mlBmMess034 = "Where Used Report",
		*mlBmMess035 = "Can only use non-zero standard cost items.",
		*mlBmMess036 = "Material cannot be same as product.",
		*mlBmMess037 = "Material cannot be product of itself.",
		*mlBmMess038 = "A sequence number must be entered.",
		*mlBmMess039 = "Enter full line first, then maintain instruction.",
		*mlBmMess040 = "Mfg item %s standard cost is zero. Continue [Y/N]?",
		*mlBmMess041 = "Quantity %14.6f greater than allowed quantity %f.",
		*mlBmMess042 = "Product Specification Maintenance",
		*mlBmMess043 = "Units of Measure",
		*mlBmMess044 = "Batch Sizes",
		*mlBmMess045 = "This line is tagged for deletion.",
		*mlBmMess046 = "Unit of Measure Maintenance",
		*mlBmMess047 = "Add First Unit of New Group",
		*mlBmMess048 = "Add Unit to Group",
		*mlBmMess049 = "Modify Unit",
		*mlBmMess050 = "UOM %s could calc qty greater than dec_pt %d will allow. Continue [Y/N]?",
		*mlBmMess051 = "Handling Class Maintenance",
		*mlBmMess700 = "Usage : %s <I(tem range) G(roup range)>",
		*mlBmMess701 = "Usage : %s <D(etailed) S(ummary)>",
		*mlBmMess702 = "Usage : %s <Description> <I(tem Range) G(roup Range)>",
		*mlBmMess703 = "OR      %s <I(tem Range) G(roup Range)>",
		*mlBmMess704 = "                     <lpno>",
		*mlBmMess705 = "                     <lower limit>",
		*mlBmMess706 = "                     <upper limit>",
		*mlBmMess707 = "                     <D(etailed) S(ummary)>";
#endif	/* ML_BM_MESS_H */
