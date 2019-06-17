/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_tr_mess.h,v 5.0 2001/06/19 06:51:46 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_tr_mess.h,v $
| Revision 5.0  2001/06/19 06:51:46  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/15 00:22:35  scott
| Updated to include new messages for sk_skcm_cp
|
| Revision 4.0  2001/03/09 00:59:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2000/12/19 00:09:41  scott
| Added new messages to transport system for containers.
|
| Revision 3.2  2000/12/12 07:03:43  scott
| Updated to add new fields for containers.
|
| Revision 3.1  2000/12/12 04:50:05  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_TR_MESS_H
#define	ML_TR_MESS_H

char	*mlTrMess001 = "Vehicle unit not found.",
		*mlTrMess002 = "No Deliveries for this date could be found.",
		*mlTrMess003 = "Trip number not found.",
		*mlTrMess004 = "Delivery Manifest",
		*mlTrMess005 = "Vehicle code not found.",
		*mlTrMess006 = "Print Vehicle Records",
		*mlTrMess007 = "No available trips for %s",
		*mlTrMess008 = "Delivery Confirmation",
		*mlTrMess009 = "Vehicle           : %s",
		*mlTrMess010 = "P/Slip   : %s",
		*mlTrMess011 = "Expected Del Date : %s",
		*mlTrMess012 = "Trip Name         : %s",
		*mlTrMess013 = "Address  : %s",
		*mlTrMess014 = "Sum of qty. returned and qty. delivered must not exceeds qty. ordered.",
		*mlTrMess015 = "Updating warehouse ....",
		*mlTrMess016 = "Return form number and actual delivery date must be entered.",
		*mlTrMess017 = "Actual delivery date must be entered.",
		*mlTrMess018 = "Invalid parameter for packing slip type flag.",
		*mlTrMess019 = "Create a new trip for vehicle [Y/N]?",
		*mlTrMess020 = "You cannot move selected Documents to its own trip.  Please enter a new trip.",
		*mlTrMess021 = "Trip already exist.  Create a new trip for vehicle [Y/N]?",
		*mlTrMess022 = "Status is already delivered.",
		*mlTrMess023 = "Status is already delivered.  Addition, subtraction, or movement of Selected Document are not allowed.",
		*mlTrMess024 = "The total weight of the selected Documents have exceeded the capacity of the vehicle. Continue [Y/N]?",
		*mlTrMess025 = "Move Document",
		*mlTrMess026 = "Allocate Document to Vehicle.",
		*mlTrMess027 = "No record found in the specified range. Press <return> to exit.",
		*mlTrMess028 = "No such vehicle number for that delivery date.",
		*mlTrMess029 = "No such area code for that vehicle number.",
		*mlTrMess030 = "Delivery Sheet",
		*mlTrMess031 = "Vehicle List",
		*mlTrMess032 = "Quantity delivered must not be greater than quantity ordered.",
		*mlTrMess033 = "Hour cannot exceed 24.",
		*mlTrMess034 = "Delivery date must not be greater than date today.",
		*mlTrMess035 = "Document Number not found.",
		*mlTrMess036 = "    [RESTART]   [END]     ",
		*mlTrMess037 = "No actual loaded quantity found.",
		*mlTrMess038 = "%R Transport & Sales Liq - Input Delivery/Returns",
		*mlTrMess039 = "Vehicle Maintenance",
		*mlTrMess040 = "Maximum weight is less than minimum weight.",
		*mlTrMess041 = "Maximum volume less than minimum volume.",
		*mlTrMess042 = "Rate per month must not be less than 0.",
		*mlTrMess043 = "Rate per day must not be less than 0.",
		*mlTrMess044 = "Rate per trip must not be less than 0.",
		*mlTrMess045 = "Account must be of a financial posting class.",
		*mlTrMess046 = "Rental Rates",
		*mlTrMess047 = "Transport Vehicle Master File Maintenance",
		*mlTrMess048 = "Rate per volume must not be less than 0.",
		*mlTrMess049 = "Rate per weight must not be less than 0.",
		*mlTrMess050 = "Transport Load Order.",
		*mlTrMess051 = "Print Vehicle Listings",
		*mlTrMess052 = "Print Vehicle Records",
		*mlTrMess053 = "Print Vehicle List Report",
		*mlTrMess054 = "The total volume of the Selected Documents have exceeded the capacity of the vehicle. Continue [Y/N]?",
		*mlTrMess055 = "The Max Height of vehicle has been exceeded by one or more items. Continue [Y/N]?",
		*mlTrMess056 = "The Max Width of vehicle has been exceeded by one or more items. Continue [Y/N]?",
		*mlTrMess057 = "Trip already exist, Update/View trip ?",
		*mlTrMess058 = "Time input conflicts with previous End time.",
		*mlTrMess059 = "Delivery Zone in not on file.",
		*mlTrMess060 = "Vehicle is unavailable. Reason - %s",
		*mlTrMess061 = "Time slot is not on file.",
		*mlTrMess062 = "Transport Schedule Maintenance.",
		*mlTrMess063 = "Schedule Delivery Now [Y/N] :",
		*mlTrMess064 = "Document %s cannot be Scheduled as delivery flag is set to No Delivery.",
		*mlTrMess065 = "Sorry cannot schedule order %s as it is already a packing slip.",
		*mlTrMess066 = "Sorry no trip schedules have been defined for this zone.",
		*mlTrMess067 = "No such delivery Zone for that vehicle number.",
		*mlTrMess068 = "Carrier Code %s is not on file.",
		*mlTrMess069 = "Carrier Code / Zone Maintenance.",
		*mlTrMess070 = "Delivery Zone already exists for carrier code.",
		*mlTrMess071 = "From : %s - %s  / Carrier %s - %s.",
		*mlTrMess072 = "Minimum Height greater than Maximum Height.",
		*mlTrMess073 = "Maximum Height less than minimum Height.",
		*mlTrMess074 = "Minimum Width greater than Maximum Width.",
		*mlTrMess075 = "Maximum Width less than minimum Width.",
		*mlTrMess076 = "****** NOTE : Delivery Date updated ******",
		*mlTrMess077 = "Minimum Depth greater than Maximum Depth.",
		*mlTrMess078 = "Maximum Depth less than minimum Depth.",
		*mlTrMess079 = "Container billing rates",
		*mlTrMess080 = "Container Status not on file.",
		*mlTrMess081 = "Container Master file Maintenance",
		*mlTrMess082 = "Status code exists in Container Master file, cannot delete",
		*mlTrMess083 = "Container not on file.",
		*mlTrMess084 = "Allocate Document to Container.",
		*mlTrMess085 = "Copy Container Maintenance",
		*mlTrMess086 = "Container already on file.",

		*mlTrMess700 = "Usage : %s <hhve_hash> <hhtr_hash> <hhco_hash> <hhcu_hash>",
		*mlTrMess701 = "Usage : %s <find flag [P|T]>",
		*mlTrMess702 = "        for packing slip type flag.",
		*mlTrMess703 = "Usage : %s <start Supplier> <end Supplier> <lpno>";
#endif	
