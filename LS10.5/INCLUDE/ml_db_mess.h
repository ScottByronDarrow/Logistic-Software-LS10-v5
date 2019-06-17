/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_db_mess.h,v 5.1 2001/07/25 01:01:56 scott Exp $
-----------------------------------------------------------------------
| $Log: ml_db_mess.h,v $
| Revision 5.1  2001/07/25 01:01:56  scott
| Updated for 10.5
|
| Revision 5.0  2001/06/19 06:51:32  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/12/12 04:50:02  scott
| Updated to place strings back into include file so plook and other utils work
|
======================================================================*/
#ifndef	ML_DB_MESS_H
#define	ML_DB_MESS_H

char	*mlDbMess001 = "Unable to find journal control record.",
		*mlDbMess002 = "Usage: %s <lpno> <pid>",
		*mlDbMess003 = "Usage: %s <Start Contract> <End Contract> <lpno>",
		*mlDbMess004 = "Customer Contract Deletion",
		*mlDbMess005 = "Alternate Item not found.",
		*mlDbMess006 = "Usage: %s printer-no [Optional C(ompany Report)]",
		*mlDbMess007 = "Contract currency does not match customer's",
		*mlDbMess008 = "Contract bank does not match input bank.",
		*mlDbMess009 = "Forward exchange contract is not activated.",
		*mlDbMess010 = "Contract no longer current - expired %s",
		*mlDbMess011 = "Contract not yet current - effective %s",
		*mlDbMess012 = "Reselecting invoices for payment.",
		*mlDbMess013 = "Selected invoices may have forward exchange contracts overridden.",
		*mlDbMess014 = "Cheques can only be processed at head office for Customer %s.",
		*mlDbMess015 = "Receipt already exists.",
		*mlDbMess016 = "Existing receipt %s has date less than current month.",
		*mlDbMess017 = "Receipt %s was not lodged with bank %s.",
		*mlDbMess018 = "The lodgement for receipt %s has been cleared.",
		*mlDbMess019 = "Receipt date must be greater than current module month.",
		*mlDbMess020 = "Receipt date cannot be greater than one month forward.",
		*mlDbMess021 = "Letter of credit is not for bank.",
		*mlDbMess022 = "Total amount of the letter of credit (%s) has been exceeded.",
		*mlDbMess023 = "Invoice/Credit not found. Add[Y/N]?",
		*mlDbMess024 = "Input I(nvoice) C(redit)?",
		*mlDbMess025 = "This invoice is pre-assigned to forward exchange contract %s, override?",
		*mlDbMess026 = "Invoice exceeds Customers cutoff date.",
		*mlDbMess027 = "Do you wish to include invoices with existing forward exchange contract assignments?",
		*mlDbMess028 = "Override existing forward exchange contract assignments [Y/N]?",
		*mlDbMess029 = "Receipt on file for gross %.2f.",
		*mlDbMess030 = "Receipt amount %.2f exceeds amount available on Forward Exchange contract %.2f.",
		*mlDbMess031 = "%8.2f will remain unallocated.  Is this correct [Y/N]?",
		*mlDbMess032 = "Total Receipt Amount (%s) %8.2f must equal allocation amount %8.2f.",
		*mlDbMess033 = "Local Receipt Amount %8.2f must equal allocation amount %8.2f.",
		*mlDbMess034 = "Do you wish to round Local Receipt Amount by %4.2f to %8.2f [Y/N]?",
		*mlDbMess035 = "Invoice ratio incorrect, Invoice = %d.",
		*mlDbMess036 = "Program will only allow 5000 invoices to be paid at once. Continuing.",
		*mlDbMess037 = "Deposit not found for this receipt.",
		*mlDbMess038 = "Deposit has been fully allocated.",
		*mlDbMess039 = "Any unallocated cash will be left against the initial deposit.",
		*mlDbMess040 = "Receipt is fully allocated.",
		*mlDbMess041 = "Program will only allow 500 invoices to be paid at once. Continuing.",
		*mlDbMess042 = "Batch Total = %-8.2f",
		*mlDbMess043 = "Processing Cheque No %s",
		*mlDbMess044 = "Last Receipts: %s",
		*mlDbMess045 = "Local amount cannot be 0.00 as base amount is greater than 0.00.",
		*mlDbMess046 = "Total Amount Unallocated (%s) %8.2f",
		*mlDbMess047 = "Total Amount Unallocated %8.2f",
		*mlDbMess048 = "Total Receipt Amount (%s) %8.2f",
		*mlDbMess049 = "Total Receipt Amount %8.2f",
		*mlDbMess050 = "Customer Receipts",
		*mlDbMess051 = "Customer Receipts (Journal Only)",
		*mlDbMess052 = "Customer Transaction Processing Selection",
		*mlDbMess053 = "Invoice not found for Customer.",
		*mlDbMess054 = "To Date cannot precede From Date.",
		*mlDbMess055 = "Currency %s does not match invoice currency %s.",
		*mlDbMess056 = "Invoice/Credit Note Exchange Rate Amendment.",
		*mlDbMess057 = "Summary Ageing Report Selection",
		*mlDbMess058 = "Extended Ageing Report Selection",
		*mlDbMess059 = "Usage: %s <S(ummary) D(etailed) E(xtended)>",
		*mlDbMess060 = "Detailed Ageing Report Selection",
		*mlDbMess061 = "Usage: %s <dir_name> <lpno> <letter_no> <odue_amt> <start_DB> <end_DB> ",
		*mlDbMess062 = "Paragraph  Input",
		*mlDbMess063 = "Use [SEARCH] to display dot commands.",
		*mlDbMess064 = "Royalty Class Maintenance",
		*mlDbMess065 = "Index on file CUWK is bad, please run BCHECK.",
		*mlDbMess066 = "Usage %s <I/C> <lpno> <branch>",
		*mlDbMess067 = "Customer must have the same currency code as the head office Customer.",
		*mlDbMess068 = "Customer must have the same G/L control account as the head office Customer.",
		*mlDbMess069 = "Customer must have no transactions to become a child account.",
		*mlDbMess070 = "Child %s-%s cannot be modified, transactions on file.",
		*mlDbMess071 = "Customer same as parent Customer.",
		*mlDbMess072 = "Last Code %s",
		*mlDbMess073 = "Bills Due Selection",
		*mlDbMess074 = "Forward Cheques Selection",
		*mlDbMess075 = "Selling Price less than Cost Price. Continue [Y/N]?",
		*mlDbMess076 = "Cost Price greater than Selling Price [Y/N]?",
		*mlDbMess077 = "An agreed cost price must be entered.",
		*mlDbMess078 = "The agreed cost price must be greater than zero.",
		*mlDbMess079 = "An agreed cost price must be entered for line %3d.",
		*mlDbMess080 = "Kits cannot be entered as contract items.",
		*mlDbMess081 = "Item has been discontinued.",
		*mlDbMess082 = "Item is already on contract for this currency.",
		*mlDbMess083 = "Effective date cannot be greater than expiry date.",
		*mlDbMess084 = "Effective date cannot be greater than renewal date.",
		*mlDbMess085 = "Effective date cannot be greater than review date.",
		*mlDbMess086 = "Expiry date must be greater than effective date.",
		*mlDbMess087 = "Expiry date must be greater than renewal date.",
		*mlDbMess088 = "Expiry date must be greater than review date.",
		*mlDbMess089 = "Review date must be greater than effective date.",
		*mlDbMess090 = "Review date must be less than expiry date.",
		*mlDbMess091 = "Review date must be less than renewal date.",
		*mlDbMess092 = "Renewal date must be greater than effective date.",
		*mlDbMess093 = "Renewal date must be less than expiry date.",
		*mlDbMess094 = "Renewal date must be greater than review date.",
		*mlDbMess095 = "Customer Contract Administration",
		*mlDbMess096 = "Last Contract: %s",
		*mlDbMess097 = "This is a forward receipt. Are you sure [Y/N]?",
		*mlDbMess098 = "Do you want to cancel further forward receipt messages [Y/N]?",
		*mlDbMess099 = "Receipt date may not be greater than current system date.",
		*mlDbMess100 = "S u n d r y  R e c e i p t s",
		*mlDbMess101 = "Update Invoice Due Date",
		*mlDbMess102 = "This program will reset all the invoice due dates",
		*mlDbMess103 = "based on the date of invoice and the payment terms.",
		*mlDbMess104 = "Before running this program, you must have converted",
		*mlDbMess105 = "your data to a SEL Version 9 format.",
		*mlDbMess106 = "This system has only %d printers.",
		*mlDbMess107 = "Overdue Letter Processing Selection",
		*mlDbMess108 = "Letter of Credit %s has been flagged for deletion.",
		*mlDbMess109 = "Letter of Credit Maintenance",
		*mlDbMess110 = "Letter of Credit Display/Print",
		*mlDbMess111 = "Customer Master File Deletion",
		*mlDbMess112 = "Customer Master File",
		*mlDbMess113 = "Deleted Customers",
		*mlDbMess114 = "Customers 24-Month Value Sales Analysis",
		*mlDbMess115 = "Selected Statement Print",
		*mlDbMess116 = "Usage: %s <dir_name>",
		*mlDbMess117 = "Usage: %s <lp_no> <N(umeric order>, A(cronym order)> -optional <selected>",
		*mlDbMess118 = "       <selected>-Print Selected Statements <Y/N> ",
		*mlDbMess119 = "Customer Delivery Instruction Maintenance",
		*mlDbMess120 = "Unassign Customers from Contracts",
		*mlDbMess121 = "Unassign Contract from Customer",
		*mlDbMess122 = "Assign Customers to Contracts",
		*mlDbMess123 = "Assign Contract to Customer",
		*mlDbMess124 = "Cannot be untagged.  Already allocated.",
		*mlDbMess125 = "No invoice/credit or journal with number %s found.",
		*mlDbMess126 = "Customer Credit Control Maintenance",
		*mlDbMess127 = "Customer Discount Rate Maintenance",
		*mlDbMess128 = "Letter Display/Print",
		*mlDbMess129 = "Customer Listings /Selection",
		*mlDbMess130 = "Now Creating New Paragraph Record.",
		*mlDbMess131 = "Now Updating New Paragraph Record.",
		*mlDbMess132 = "Now Deleting New Paragraph Record.",
		*mlDbMess133 = "Assemble Customer Letters",
		*mlDbMess134 = "Customer Overdue Invoice Report",
		*mlDbMess135 = "Statement Messages",
		*mlDbMess136 = "Remittance Messages",
		*mlDbMess137 = "Usage: %s <hhcu_hash>",
		*mlDbMess138 = "Usage: %s <N(ormal), D(eletion)>",
		*mlDbMess139 = "Letter of Credit has expired.  Do you wish to allocate [Y/N]?",
		*mlDbMess140 = "Allocate Unallocated Receipts",
		*mlDbMess141 = "No receipts in current lodgement.",
		*mlDbMess142 = "Print Bank Lodgements",
		*mlDbMess143 = "Customer Contract Report",
		*mlDbMess144 = "Contract is already in use.  No changes may be made to this contract.",
		*mlDbMess145 = "           <rep_type>  (1-8)",
		*mlDbMess146 = "           <run_type>  (1-3)",
		*mlDbMess147 = "           <bal_type>  (1-3)",
		*mlDbMess148 = "           <sort_type> (1-2)",
		*mlDbMess149 = "           <sman_cust> (1-2)",
		*mlDbMess150 = "Usage:  %s <lpno> <pid> print 2nd Rep [Y/N]",
		*mlDbMess151 = "           <true_age>  (M or T)",
		*mlDbMess152 = "           <Overdue Value 1>",
		*mlDbMess153 = "           <Overdue Value 2>",
		*mlDbMess154 = "           <Overdue Value 3>",
		*mlDbMess155 = "           <Overdue Value 4>",
		*mlDbMess156 = "           <Local/Overseas Currency>",
		*mlDbMess157 = "           <Sort by Currency>  (M or T)",
		*mlDbMess158 = "           <Start Currency> <End Currency>",
		*mlDbMess159 = "           <Age Days> <optional department> ",
		*mlDbMess160 = "Usage : %s <Letter code> <start cust> <end cust> <lpno>",
		*mlDbMess161 = "Usage : %s <lpno> <sort type> <overdue amt>",
		*mlDbMess162 = "           <start date> <end date>",
		*mlDbMess163 = "Usage : %s <lpno> <pid> <I|C> <optional branch>",
		*mlDbMess164 = "Usage : %s <jnl_type> <lpno> <pid> <I|C>",
		*mlDbMess165 = "Usage : %s <S|R>",
		*mlDbMess166 = "Last Receipts: %s/%s",
		*mlDbMess167 = "Name:   %s %s",
		*mlDbMess168 = "Running Customers Close for %s-%s",
		*mlDbMess169 = "Head Office Customer No: %s (%s)",
		*mlDbMess170 = "Customer Head Office Control Maintenance",
		*mlDbMess171 = "Customer Interest File Maintenance",
		*mlDbMess172 = "Customer %s has already been entered for deletion.",
		*mlDbMess173 = "Customer has a balance owing of %-9.2f.",
		*mlDbMess174 = "This Customer is a Head Office Customer.",
		*mlDbMess175 = "This Customer has invoices on file.",
		*mlDbMess176 = "This Customer has cheque payments on file.",
		*mlDbMess177 = "This Customer has orders on file,",
		*mlDbMess178 = "This Customer has active unposted invoices on file.",
		*mlDbMess179 = "This Customer is currently being maintained.",
		*mlDbMess180 = "Customer item code %s is already assigned to %s.",
		*mlDbMess181 = "Customer item code cannot be blank.",
		*mlDbMess182 = "Search facility is unavailable in this field.",
		*mlDbMess183 = "Item code %s is already used for a standard item. Use anyway [Y/N]?",
		*mlDbMess184 = "Customer Report Selection",
		*mlDbMess185 = "",
		*mlDbMess186 = "           <Sort by Currency, Y|N> ",
		*mlDbMess187 = "Customer already uses item code %s.",
		*mlDbMess188 = "Last Customer : %s",
		*mlDbMess189 = "Customer Specific Item Code Maintenance",
		*mlDbMess190 = "Total of %-8.2f must equal Nett Inv/Crd Total of %-8.2f.",
		*mlDbMess191 = "Proof Total %-8.2f unequal to Invoice Total %-8.2f.",
		*mlDbMess192 = "Invoice is already on file for gross %-8.2f.",
		*mlDbMess193 = "Credit is already on file for gross %-8.2f.",
		*mlDbMess194 = "Journal is already on file for gross %-8.2f.",
		*mlDbMess195 = "Account: %s",
		*mlDbMess196 = "Balance to allocate: %.2f",
		*mlDbMess197 = "Last Inv/Cn: %s",
		*mlDbMess198 = "Nett Amount Outstanding: ",
		*mlDbMess199 = "Customers Invoice Input",
		*mlDbMess200 = "Customers Credit Note Input",
		*mlDbMess201 = "Warning Customer Inv/Crd set-up option",
		*mlDbMess202 = "Receipt is not a cheque.",
		*mlDbMess203 = "Receipt has been cancelled.",
		*mlDbMess204 = "Are you sure you want to cancel cheque %s [Y/N] ?",
		*mlDbMess205 = "Customer Cancelled Cheques Entry",
		*mlDbMess206 = "Customers Expired Contract Rebate Claim Report",
		*mlDbMess207 = "Journals can only be processed at head office for Customer %s",
		*mlDbMess208 = "Updating Journals ....",
		*mlDbMess209 = "Journal Total = %.2f",
		*mlDbMess210 = "Customers Journal Input",
		*mlDbMess211 = "Note : System credit rating is A%d.",
		*mlDbMess212 = "Discount Percent %5.2f",
		*mlDbMess213 = "Bank code could not be added.",
		*mlDbMess214 = "Br=0 (Company Owned Customer)/",
		*mlDbMess215 = "Updating Customers payment terms...",
		*mlDbMess216 = "Customers Payment Terms Adjustment",
		*mlDbMess217 = "Print Customer Specific Item Codes",
		*mlDbMess218 = "Journal %s already exists.",
		*mlDbMess219 = "Date exceeds system date %s.",
		*mlDbMess220 = "Base Currency Value",
		*mlDbMess221 = "Local Currency Value",
		*mlDbMess222 = "Journal total must be 0.00.",
		*mlDbMess223 = "Invalid price type.",
		*mlDbMess224 = "Branch cannot be 0 as Customers are branch owned.",
		*mlDbMess225 = "Transactions for customer not found.",
		*mlDbMess226 = "Invoice proof total not input or incorrect.",
		*mlDbMess227 = "Date before 1st day in current Customer's month.",
		*mlDbMess228 = "Date before last day in current Customer's month.",
		*mlDbMess229 = "Print Purchase Orders By Supplier",
		*mlDbMess230 = "Customer Listings",
		*mlDbMess231 = "Print Customers Overdue Report",
		*mlDbMess232 = "Print Customers Contract Rebate Claim Report",
		*mlDbMess233 = "Overdue Letters ",
		*mlDbMess234 = "Print Delivery Performance By Supplier",
		*mlDbMess235 = "Local Currency code of %s is not on file for bank code %s",
		*mlDbMess236 = "Receipt has been Reconciled.",
		*mlDbMess237 = "Receipt has been Presented.",
		*mlDbMess238 = "Receipt %s belongs to a prior period.",
		*mlDbMess239 = "Receipt Does not exist.",
		*mlDbMess240 = "Cannot dishonour a receipt of Cash.",
		*mlDbMess241 = "Customer %s may not be processed by this branch.",
		*mlDbMess242 = "Cannot dishonour a Dishonoured Receipt.",
		*mlDbMess243 = "Customers currencies must match for split payments.",
		*mlDbMess244 = "Invoice %s is already allocated to this receipt.",
		*mlDbMess245 = "NOTE : There are multiple payments for this receipt number. ",
		*mlDbMess246 = "Process Dishonoured Receipts.",
		*mlDbMess247 = "Bank Lodgement record could not be found, cannot process.",
		*mlDbMess700 = "Usage : %s <lpno> [Optional C(ompany Report)]",
		*mlDbMess701 = "Usage : %s <prog_desc> <rep_type> ",
		*mlDbMess702 = "   OR   %s <lpno> <rep_type> <sales_type> <C(urr|L(ast Yr>",
		*mlDbMess703 = "        <select> <Num_custs> <T(op|B(ottom>",
		*mlDbMess704 = "        <rep_type, N(ormal, A(rea, C(ustType>",
		*mlDbMess705 = "        <sales_type, M(TD, Y(TD> ",
		*mlDbMess706 = "Usage : %s <type = I or C> <pid> ",
		*mlDbMess707 = "Usage : %s <start supplier> <end supplier> <lpno> ",
		*mlDbMess708 = "Usage : %s <lpno> <rep_type> <run_type> <sort_type> <salesman> ",
		*mlDbMess709 = "Usage : %s <start Customer> <end Customer> <start item> <end item> <lpno> ";
#endif	
