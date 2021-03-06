/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_menu_mess.h,v 5.1 2001/10/05 02:57:12 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_menu_mess.h,v $
| Revision 5.1  2001/10/05 02:57:12  cha
| Update to fix incorrect name."External Category Maintenance"
| should have been"Category Maintenance.
|
| Revision 5.0  2001/06/19 06:51:32  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/02 08:48:14  scott
| Updated for lineup issue.
|
| Revision 4.0  2001/03/09 00:59:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:03  scott
| Updated to place strings back into include file so plook and other utils work
|
*/
#ifndef	ML_MENU_MESS_H
#define	ML_MENU_MESS_H

char	*mlMenuMess001 = "Run Fast Access Maintenance [Y/N]?",
		*mlMenuMess002 = "Writing New User_secure...",
		*mlMenuMess003 = "Updating User_secure...",
		*mlMenuMess004 = "Saving Old /etc/passwd...",
		*mlMenuMess005 = "Writing New /etc/passwd...",
		*mlMenuMess006 = "Updating /etc/passwd...",
		*mlMenuMess007 = "SCO does not allow addition of Unix passwords.",
		*mlMenuMess008 = "Logistic user is added but Unix user must be added using SCO utilities.",
		*mlMenuMess009 = "Please ask system administrator to add user.",
		*mlMenuMess010 = "Cannot start with a digit.",
		*mlMenuMess011 = "Duplicate name",
		*mlMenuMess012 = "User Maintenance",
		*mlMenuMess013 = "No General Ledger Control record exists.  Please create.",
		*mlMenuMess014 = "Warning: Changing env file may dramatically affect your system behaviour.",
		*mlMenuMess015 = "Blank company name is not allowed.",
		*mlMenuMess016 = "Blank company short name is not allowed.",
		*mlMenuMess017 = "Account is not a financial posting account.",
		*mlMenuMess018 = "Account is not a non-financial posting account.",
		*mlMenuMess019 = "%R Company Maintenance ",
		*mlMenuMess020 = "Last Company: %2.2s",
		*mlMenuMess021 = "Fast Access Display Input",
		*mlMenuMess022 = "Matching area records found in %-4.4s. Record not deleted.",
		*mlMenuMess023 = "Validating deletion of %s. Please wait.",
		*mlMenuMess024 = "Matching customer type records found in %-4.4s. Record not deleted.",
		*mlMenuMess025 = "Matching contract type records found in %-4.4s. Record not deleted.",
		*mlMenuMess026 = "Matching salesman records found in %-4.4s. Record not deleted.",
		*mlMenuMess027 = "Area Maintenance",
		*mlMenuMess028 = "Last Area: %s",
		*mlMenuMess029 = "Customer Type File Maintenance",
		*mlMenuMess030 = "Last Customer Type: %s",
		*mlMenuMess031 = "Contract Type File Maintenance",
		*mlMenuMess032 = "Last Contract Type: %s",
		*mlMenuMess033 = "Salesman Maintenance",
		*mlMenuMess034 = "Last Salesman: %s",
		*mlMenuMess035 = "Special Instruction Maintenance",
		*mlMenuMess036 = "Last Code: %3d",
		*mlMenuMess037 = "Receipt Type Maintenance",
		*mlMenuMess038 = "Customers module status",
		*mlMenuMess039 = "Suppliers module status",
		*mlMenuMess040 = "General ledger module status",
		*mlMenuMess041 = "Inventory module status",
		*mlMenuMess042 = "Module last closed : %3.3s %04d",
		*mlMenuMess043 = "Module next close : %3.3s %04d",
		*mlMenuMess044 = "Module status     : %-10.10s",
		*mlMenuMess045 = "Module operating in: %3.3s %04d",
		*mlMenuMess046 = "Module is not active. ",
		*mlMenuMess047 = "Leave module as open. ",
		*mlMenuMess048 = "Module will be closed.",
		*mlMenuMess049 = "Module is closed.     ",
		*mlMenuMess050 = "%s module is not active and cannot be changed.",
		*mlMenuMess051 = "Module closed until remaining branches finish month end for %s",
		*mlMenuMess052 = "%s module can only be closed at master branch.",
		*mlMenuMess053 = "%s module is still open or closing at Br:%s.",
		*mlMenuMess054 = "%s module must be closed as gl is being closed.",
		*mlMenuMess055 = "%RModule close selection",
		*mlMenuMess056 = "Use arrow keys to move to next module, <return> to select available options.",
		*mlMenuMess057 = "This option is not yet available.",
		*mlMenuMess058 = "Customer Default Setup Maintenance",
		*mlMenuMess059 = "Stock Default Setup Maintenance",
		*mlMenuMess060 = "Warning!",
		*mlMenuMess061 = "Are you sure you require this option [Y/N]?",
		*mlMenuMess062 = "Inventory Master File",
		*mlMenuMess063 = "Inventory Branch Record",
		*mlMenuMess064 = "Creating Inventory Branch Records",
		*mlMenuMess065 = "Status Information",
		*mlMenuMess066 = "Branch Master File Maintenance",
		*mlMenuMess067 = "%-30.30s   Terminal : %d (%c%s)   Time %02d :%02d %2.2s",
		*mlMenuMess068 = "User has no option to %s.",
		*mlMenuMess069 = "Sub Menu running",
		*mlMenuMess070 = "Return ",
		*mlMenuMess071 = "Foreground ",
		*mlMenuMess072 = "Space",
		*mlMenuMess073 = "Overnight",
		*mlMenuMess074 = "Any other key for background",
		*mlMenuMess075 = "DEL",
		*mlMenuMess076 = "Key Pressed Process %d",
		*mlMenuMess077 = "[FN04]-Change Order of Files.",
		*mlMenuMess078 = "Menu name not found.",
		*mlMenuMess079 = "Menu Access By User",
		*mlMenuMess080 = "Menu Access By Menu",
		*mlMenuMess081 = "Usage : %s <Help_file>",
		*mlMenuMess082 = "Print from %-8.8s to %-8.8s",
		*mlMenuMess083 = "Help file not found.",
		*mlMenuMess084 = "Tax Return",
		*mlMenuMess085 = "Audit Report",
		*mlMenuMess086 = "Logistic Save File Print",
		*mlMenuMess087 = "Press",
		*mlMenuMess088 = "for foreground report, any other key for background.",
		*mlMenuMess089 = "Background",
		*mlMenuMess090 = "Usage : %s <lpno> <lower> <upper> <type> <summary>",
		*mlMenuMess091 = "Usage : %s <co_no> <br_no> <wh_no> - optional <dp_no>",
		*mlMenuMess092 = "",
		*mlMenuMess093 = "Usage : %s <program> <arguments...>",
		*mlMenuMess094 = "Current user name : %s(%s)",
		*mlMenuMess095 = "User access required : %s(%s)",
		*mlMenuMess096 = "Logistic Super User Security Check",
		*mlMenuMess097 = "Enter password :",
		*mlMenuMess098 = "Unable to read terminal %c",
		*mlMenuMess099 = "Security check passed.",
		*mlMenuMess100 = "Security level not reached.",
		*mlMenuMess101 = "Password access denied.",
		*mlMenuMess102 = "Department Master File Maintenance",
		*mlMenuMess103 = "Environment variable LOGNAME not defined.",
		*mlMenuMess104 = "User Name : (%s)  Terminal (%d)",
		*mlMenuMess105 = "End of Day Processing",
		*mlMenuMess106 = "Processing Daily Audit Trail Log",
		*mlMenuMess107 = "Console may not logout until all others have logged out.",
		*mlMenuMess108 = "Terminal logging out",
		*mlMenuMess109 = "Only super users may run this program.",
		*mlMenuMess110 = "Sub Menu Maintenance",
		*mlMenuMess111 = "Usage %s <prog_type [T/L/D]>",
		*mlMenuMess112 = "Access Description File Maintenance",
		*mlMenuMess113 = "Menu Store",
		*mlMenuMess114 = "Menu Creation",
		*mlMenuMess115 = "Cannot access requested file.",
		*mlMenuMess116 = "Cannot find %s.mdf.",
		*mlMenuMess117 = "reason ",
		*mlMenuMess118 = "Control file (pmct) record not found.",
		*mlMenuMess119 = "Warning : Status may be out of date. Last updated: %-8.8s at %-8.8s",
		*mlMenuMess120 = "Are you sure you want to purge all mails deleted before %-8.8s [Y/N]?",
		*mlMenuMess121 = "Currency Exchange Maintenance",
		*mlMenuMess122 = "Mail has been deleted.",
		*mlMenuMess123 = "Mail unread.",
		*mlMenuMess124 = "Not a group.",
		*mlMenuMess125 = "No actions for this mail.",
		*mlMenuMess126 = "Function key for terminal %s not found.",
		*mlMenuMess127 = "Database was last purged on: %-10.10s at %-8.8s.",
		*mlMenuMess128 = "Statistics last updated on: %-10.10s at %-8.8s.",
		*mlMenuMess129 = "LogisticSoftware Mail System",
		*mlMenuMess130 = "Header record unavailable",
		*mlMenuMess131 = "<No Change>",
		*mlMenuMess132 = "<Archived>",
		*mlMenuMess133 = "<Deleted>",
		*mlMenuMess134 = "<Current>",
		*mlMenuMess135 = "CC cannot be equal to TO",
		*mlMenuMess136 = "%s already has a copy of this mail.",
		*mlMenuMess137 = "Mail unavailable for edit.",
		*mlMenuMess138 = "",
		*mlMenuMess139 = "Are you sure you want to delete %s [Y/N]?",
		*mlMenuMess140 = "New filename is:",
		*mlMenuMess141 = "LogisticSoftware Editor",
		*mlMenuMess142 = "<prog_type> must be L(inear), T(abular) or D(isplay)",
		*mlMenuMess143 = "Usage : %s [C/B/W/S] - optional <change>",
		*mlMenuMess144 = "Company / Branch / Warehouse Select",
		*mlMenuMess145 = "Company Page Number Maintenance",
		*mlMenuMess146 = "Menu Session Log Display",
		*mlMenuMess147 = "Category File Maintenance",
		*mlMenuMess148 = "Are all branch master file records set up [Y/N]?",
		*mlMenuMess149 = "%RPlease answer all questions to ensure month end files are created correctly.",
		*mlMenuMess150 = "Are all dates correct for modules being operated [Y/N]?",
		*mlMenuMess151 = "Do you want branch specific reports for customers's month end [Y/N]?",
		*mlMenuMess152 = "Do you want branch specific reports for supplier's month end [Y/N]?",
		*mlMenuMess153 = "Do you want branch specific reports for inventory month end [Y/N]?",
		*mlMenuMess154 = "Do you want branch specific reports for GL month end [Y/N]?",
		*mlMenuMess155 = "Do you want to overwrite existing records [Y/N]?",
		*mlMenuMess156 = "Department Select",
		*mlMenuMess157 = "Super User Menu Maintenance",
		*mlMenuMess158 = "Please select destination of line and press [M].  Line will be inserted above the selected line.",
		*mlMenuMess159 = "Screen Painting Section",
		*mlMenuMess160 = "Menu Characteristics",
		*mlMenuMess161 = "Cannot exit menu system.",
		*mlMenuMess162 = "Menu Line Section",
		*mlMenuMess163 = "Usage : %s <lpno> report_type",
		*mlMenuMess164 = "Update %s [Y/N]?",
		*mlMenuMess165 = "Are you sure you want to delete group %s [Y/N]?",
		*mlMenuMess166 = "At least one user for a new group must be selected.",
		*mlMenuMess167 = "Enter name of group to create :",
		*mlMenuMess168 = "Maintain Mail Groups",
		*mlMenuMess169 = "Logistic Phone Diary",
		*mlMenuMess170 = "%RNew Zealand : 0",
		*mlMenuMess171 = "%RSydney : -2",
		*mlMenuMess172 = "%RLondon : -12",
		*mlMenuMess173 = "%RParis : -11",
		*mlMenuMess174 = "%RNew York : -16",
		*mlMenuMess175 = "%RHong Kong : -4",
		*mlMenuMess176 = "Pick order %d cannot be the same as pick order %d.",
		*mlMenuMess177 = "Pick location is already specified in %s location pick flags.",
		*mlMenuMess178 = "Normal Menu Maintenance",
		*mlMenuMess179 = "Warehouse Master File Maintenance",
		*mlMenuMess180 = "%RBranch Controlled Date",
		*mlMenuMess181 = "%RCompany Controlled Date",
		*mlMenuMess182 = " System Administration Option ",
		*mlMenuMess183 = "User name (%s)",
		*mlMenuMess184 = "Unable to open backup log file.",
		*mlMenuMess185 = "Tape backup control (%ld tapes in set)",
		*mlMenuMess186 = "Last Four Backups:",
		*mlMenuMess187 = "Tape:#%03ld  User:%-10.10s  Date & Time:%-30.30s",
		*mlMenuMess188 = "Total number of backups completed %ld",
		*mlMenuMess189 = "Please insert tape number %ld and press <return> when ready.",
		*mlMenuMess190 = "Machine Specification Description Maintenance",
		*mlMenuMess191 = "Machine Specification Type Maintenance",
		*mlMenuMess192 = "Date must not be less than start date.",
		*mlMenuMess193 = "Create [Y/N]?",
		*mlMenuMess194 = "Delete mail [Y/N]?",
		*mlMenuMess195 = "Are you sure you want to delete this file [Y/N]?",
		*mlMenuMess196 = "System default will be used.",
		*mlMenuMess197 = "Transaction will never be deleted.",
		*mlMenuMess198 = "Are you sure you want to continue?",
		*mlMenuMess199 = "Cannot send a blank mail.",
		*mlMenuMess200 = "LogisticSoftware Mail System - %s",
		*mlMenuMess201 = "Cannot forward mail to yourself.",
		*mlMenuMess202 = "Mail has been read, cannot delete.",
		*mlMenuMess203 = "Sending mail.",
		*mlMenuMess204 = "%s read the mail before it can be deleted.",
		*mlMenuMess205 = "Logistic Mail Administration",
		*mlMenuMess206 = "Print menu : %-14.14s Including called menus",
		*mlMenuMess207 = "Print menu : %-14.14s Excluding called menus",
		*mlMenuMess208 = "Usage : %s Directory",
		*mlMenuMess209 = "Filename to read:",
		*mlMenuMess210 = "    User  | Tty   |  Date.   |  Time    | Option ",
		*mlMenuMess211 = "Process month end close.",
		*mlMenuMess212 = "Invalid source.",
		*mlMenuMess213 = "Grade not found.",
		*mlMenuMess214 = "Price is inclusive of %-3.3s so %-3.3s percent should equal to 0.00.",
		*mlMenuMess215 = "% clf ALT UOM %s is desired, then set dec_pt > %d.",
		*mlMenuMess216 = "Press 'Y' to continue.  Any other key to abort.",
		*mlMenuMess217 = "Last Code: %s",
		*mlMenuMess218 = "Operator must be *(multiply) or /(divide).",
		*mlMenuMess219 = "Operator *(multiply) not yet supported.",
		*mlMenuMess220 = "Sub Menu program not found.",
		*mlMenuMess221 = "Recursion exceeds %s levels.",
		*mlMenuMess222 = "Corrupt Menu Data File %s.",
		*mlMenuMess223 = "Duplicate fast access key %s.",
		*mlMenuMess224 = "%R Access Denied %c",
		*mlMenuMess225 = "A backup is currently in progress.",
		*mlMenuMess226 = "COMM file is not set up.",
		*mlMenuMess227 = "*** Menu System Now Exited ***",
		*mlMenuMess228 = "Timeout...Menu system logging out.",
		*mlMenuMess229 = "%R[Fast Access : %-*.*s",
		*mlMenuMess230 = "Logistic Month End Status For",
		*mlMenuMess231 = "Customers",
		*mlMenuMess232 = "Inventory",
		*mlMenuMess233 = "General Ledger",
		*mlMenuMess234 = "Suppliers Ledger",
		*mlMenuMess235 = "Warning !",
		*mlMenuMess236 = "Are You Sure You Require This Option",
		*mlMenuMess237 = "Welcome to Logistic Scratch Pad",
		*mlMenuMess238 = "Cannot continue, please try again once reason for N(o) answer is corrected.",
		*mlMenuMess239 = "You Don't Have A LOGNAME set",
		*mlMenuMess240 = "You Are Not A Valid User",
		*mlMenuMess241 = "Access violation due to password failure",
		*mlMenuMess242 = "Sun",
		*mlMenuMess243 = "Mon",
		*mlMenuMess244 = "Tue",
		*mlMenuMess245 = "Wed",
		*mlMenuMess246 = "Thu",
		*mlMenuMess247 = "Fri",
		*mlMenuMess248 = "Sat",
		*mlMenuMess249 = "Jan",
		*mlMenuMess250 = "Feb",
		*mlMenuMess251 = "Mar",
		*mlMenuMess252 = "Apr",
		*mlMenuMess253 = "May",
		*mlMenuMess254 = "Jun",
		*mlMenuMess255 = "Jul",
		*mlMenuMess256 = "Aug",
		*mlMenuMess257 = "Sep",
		*mlMenuMess258 = "Oct",
		*mlMenuMess259 = "Nov",
		*mlMenuMess260 = "Dec",
		*mlMenuMess261 = "pm",
		*mlMenuMess262 = "am",
		*mlMenuMess263 = "Good morning %c%s.",
		*mlMenuMess264 = "Good morning %c%s, early start ?",
		*mlMenuMess265 = "Good afternoon %c%s.",
		*mlMenuMess266 = "Good evening %c%s.",
		*mlMenuMess267 = "Good Night %c%s, Time to go home.",
		*mlMenuMess268 = "RUN_EODAY",
		*mlMenuMess269 = "no_option",
		*mlMenuMess270 = "User has no option on menu",
		*mlMenuMess271 = "Logistic file MENUSYS/COMM does not exist.",


		*mlMenuMess700 = "Usage : %s <A(rea, C(lass, S(alesman, I(nstructions, C(ontract Type, R(eceipt Type> .",
		*mlMenuMess701 = "Usage : %s <month_end_type> .",
		*mlMenuMess702 = "Usage : %s <C(umr defaults, I(nmr defaults, S(umr defaults>",
		*mlMenuMess703 = "Usage : %s <filename> ",
		*mlMenuMess704 = "Usage : %s <warning string> ",
		*mlMenuMess705 = "Usage : %s [D|S] .",
		*mlMenuMess706 = "Usage : %s <lpno> <report_type, 1=sman|2=area|3=cat>",
		*mlMenuMess707 = "Usage : %s module number [...]",
		*mlMenuMess708 = "Usage : %s [-d delete] file [file...] ",
		*mlMenuMess709 = "(1=esmr,2=cudt,3=ccmr)",
		*mlMenuMess710 = "Usage : %s <directory> <lpno> ",
		*mlMenuMess711 = "Input field cannot be blank. ";
#endif	
