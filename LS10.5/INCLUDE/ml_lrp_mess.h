/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_lrp_mess.h,v 5.0 2001/06/19 06:51:32 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_lrp_mess.h,v $
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
#ifndef	ML_LRP_MESS_H
#define	ML_LRP_MESS_H

char	*mlLrpMess001 = "LRP - Generate Purchase Orders",
		*mlLrpMess002 = "Regeneration of suggested order recommended.",
		*mlLrpMess003 = "Purchase order already set up for item.",
		*mlLrpMess004 = "Negative demand for item.",
		*mlLrpMess005 = "Any combination of A, B, C and D ascending",
		*mlLrpMess006 = "Item :",
		*mlLrpMess007 = "%R Weeks       Quantity ",
		*mlLrpMess008 = "Available",
		*mlLrpMess009 = "On Order        :",
		*mlLrpMess010 = "Total Cover     :",
		*mlLrpMess011 = "Cover Required  :",
		*mlLrpMess012 = "Net Requirement :",
		*mlLrpMess013 = "Review Period:  +          Weeks",
		*mlLrpMess014 = "Lead Time   :  +          Weeks",
		*mlLrpMess015 = "Safety Stock :             Weeks",
		*mlLrpMess016 = "Cover :                    Weeks",
		*mlLrpMess017 = "Weeks Demand    : ",
		*mlLrpMess018 = "Min Ord  Qty    :",
		*mlLrpMess019 = "Norm Ord Qty    :",
		*mlLrpMess020 = "Qty Recommended :",
		*mlLrpMess021 = "Qty Ordered     :",
		*mlLrpMess022 = "Last Supplier: %-s",
		*mlLrpMess023 = "Supplier Price Maintenance",
		*mlLrpMess024 = "Maintain Supplier Price",
		*mlLrpMess025 = "Warehouse Replenishment Report",
		*mlLrpMess026 = "LRP - Reorder Review Period Maintenance",
		*mlLrpMess027 = "LRP - Erratic Demand Report",
		*mlLrpMess028 = "LRP - Item Exception Maintenance",
		*mlLrpMess029 = "LRP - Item Future Demand Maintenance",
		*mlLrpMess030 = "Forecast method may not be blank.",
		*mlLrpMess031 = "Logistic Requirements Planning History Maintenance",
		*mlLrpMess032 = "No LRP_METHODS defined in Enironment.",
		*mlLrpMess033 = "Lead Time    :  +          Days",
		*mlLrpMess034 = "Order Multiple  :",
		*mlLrpMess035 = "%R Please select method(s) to graph.",
		*mlLrpMess036 = "LRP - Manual Forecast Option Update",
		*mlLrpMess037 = "This system has %d printers.",
		*mlLrpMess038 = "Filename is required.",
		*mlLrpMess039 = "Filename not found.",
		*mlLrpMess040 = "Not all branches is in the same inventory month.",
		*mlLrpMess041 = "Cannot locate file or Wrong warehouse.",
		*mlLrpMess042 = "Usage :  %s <lpno> <lower> <upper> ",
		*mlLrpMess043 = "Usage :  %s <lpno> <filename> <optional Quick> ",
		*mlLrpMess044 = "No suppliers for this item.",
		*mlLrpMess045 = "Zero order recommended for item.",
		*mlLrpMess046 = "Non-stock item.",
		*mlLrpMess047 = "Qty ordered < 0.00",
		*mlLrpMess048 = "Qty ordered < Minimum order qty",
		*mlLrpMess049 = "Qty ordered was rounded to nearest higher order multiple.",
		*mlLrpMess050 = "Supplier|   Supplier Name  |  Lead | Cur | Cnty | FOB (%s)  |  Duty  | Freight + | Into Store |  Minimum  |  Normal  |   Order ",
		*mlLrpMess051 = " Number |    Item Number   |  Time |     |      |    Cost    |Percent |Contingency|    Cost    | Order Qty |Order Qty | Multiple",
		*mlLrpMess052 = "ABC Codes (Requiring Input)  :  ",
		*mlLrpMess053 = "LRP - Input Order Quantities and Select Supplier. (Zero reorder quantities included/Suppliers in priority order.)",
		*mlLrpMess054 = "LRP - Input Order Quantities and Select Supplier. (Zero reorder quantities excluded/Suppliers in priority order.)",
		*mlLrpMess055 = "LRP - Input Order Quantities and Select Supplier. (Zero reorder quantities included/Suppliers into store cost order.)",
		*mlLrpMess056 = "LRP - Input Order Quantities and Select Supplier. (Zero reorder quantities excluded/Suppliers into store cost order.)",
		*mlLrpMess057 = "Filename : ",
		*mlLrpMess058 = "Branches are in different periods. Cannot produce company-wide forecast.",
		*mlLrpMess059 = "At least %hd future months",
		*mlLrpMess060 = "Item : ",
		*mlLrpMess061 = "LRP - Reorder Review By Company   ",
		*mlLrpMess062 = "LRP - Reorder Review By Branch    ",
		*mlLrpMess063 = "LRP - Reorder Review By Warehouse ",
		*mlLrpMess064 = " C(ompany ",
		*mlLrpMess065 = " B(ranch ",
		*mlLrpMess066 = " W(arehouse ",
		*mlLrpMess067 = " R(eview Period ",
		*mlLrpMess068 = " L(ead Time ",
		*mlLrpMess069 = " S(uppliers ",
		*mlLrpMess070 = " G(raph ",
		*mlLrpMess071 = " Stock Surplus Report ",
		*mlLrpMess072 = " D(ist Groups ",
		*mlLrpMess073 = "Min. Stock   :             Weeks",
		*mlLrpMess074 = "LRP Error - Only two items can be set to Y(es) or H(ighlighted)",
		*mlLrpMess075 = "LRP Reorder work file delete",
		*mlLrpMess076 = "LRP Input Stock transfer quantities.";
#endif	
