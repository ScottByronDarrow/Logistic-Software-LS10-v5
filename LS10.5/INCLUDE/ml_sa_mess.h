/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ml_sa_mess.h,v 5.0 2001/06/19 06:51:34 cha Exp $
-----------------------------------------------------------------------
| $Log: ml_sa_mess.h,v $
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
#ifndef	ML_SA_MESS_H
#define	ML_SA_MESS_H

char	*mlSaMess001 = "Display Sales Analysis By Category",
		*mlSaMess002 = "Display Sales Analysis By Customer/Category",
		*mlSaMess003 = "Sales Analysis By Category Report",
		*mlSaMess004 = "Sales Analysis By Customer/Category Report",
		*mlSaMess005 = "Sales Analysis By Customer By Item For The Last 12 Months",
		*mlSaMess006 = "Customer Sales By Item Report",
		*mlSaMess007 = "Customer Sales Report",
		*mlSaMess008 = "Cust Type Sales By Customer By Item Report",
		*mlSaMess009 = "Cust Type Sales By Customer Report",
		*mlSaMess010 = "Salesman's Sales By Customer By Item Report",
		*mlSaMess011 = "Salesman's Sales By Customer Report",
		*mlSaMess012 = "Category Sales By Item Report",
		*mlSaMess013 = "Category Sales Report",
		*mlSaMess014 = "Item Sales By Customer Report",
		*mlSaMess015 = "Item Sales Report",
		*mlSaMess016 = "Sales Analysis Report Selection",
		*mlSaMess017 = "Category Sales By Item By Customer Report",
		*mlSaMess018 = "Sales Analysis Subrange Maintenance",
		*mlSaMess019 = "Sales Budget Maintenance Selection",
		*mlSaMess020 = "Sales Analysis Entry",
		*mlSaMess021 = "Month must be from 1 to 12.",
		*mlSaMess022 = "You may only modify sales figures for the current fiscal year.",
		*mlSaMess023 = "Sales / Budget Maintenance By Salesman",
		*mlSaMess024 = "Sales / Budget Maintenance By Category",
		*mlSaMess025 = "Sales / Budget Maintenance By Area",
		*mlSaMess026 = "Sales / Budget Maintenance By Class Type",

		*mlSaMess027 = "Last Years Budget",
		*mlSaMess028 = "Current Years Budget",
		*mlSaMess029 = "Item Sales Category Selection By Customer Report",
		*mlSaMess030 = "Item Sales Category Selection Report",
		*mlSaMess031 = "Print Sales By Salesman By Item",
		*mlSaMess032 = "For all branches",
		*mlSaMess033 = "Sales Analysis By Category By Cust For The Last 12 Months",
		*mlSaMess034 = "Sales Analysis By Item By Cust For The Last 12 Months",
		*mlSaMess035 = "<Sales Analysis is by C(ustomer) or P(roduct) only.>",
		*mlSaMess036 = "<Output is D(isplay) or P(rinter) only.>",
		*mlSaMess037 = "<Type must be C(ustomer), T(ype) or S(alesman) only.>",
		*mlSaMess038 = "Type must be I(tem) or c(A)tegory only.>",
		*mlSaMess039 = "Class Type not found.",
		*mlSaMess040 = "All categories have been selected.",
		*mlSaMess041 = "All salesmen have been selected.",
		*mlSaMess042 = "All customer types have been selected.",
		*mlSaMess043 = "Sales Analysis",

		*mlSaMess700 = "Usage : %s <lpno> <lower> <upper> <rep. type> <analysis> [dp|wh] <dummy>",
		*mlSaMess701 = "<rep_type> 0 - by salesman",
		*mlSaMess702 = "           1 - by category",
		*mlSaMess703 = "           2 - by customer by category",
		*mlSaMess704 = "           3 - by area",
		*mlSaMess705 = "           4 - by customer class",
		*mlSaMess706 = "<analysis> 0 - by company",
		*mlSaMess707 = "           1 - by branch",
		*mlSaMess708 = "           2 - by warehouse",
		*mlSaMess709 = "           3 - by department",
		*mlSaMess710 = "Usage : %s <(C)ustomer|C(A)tegory>",
		*mlSaMess711 = "Usage : %s <program description>",
		*mlSaMess712 = "Usage : %s <lpno>",
		*mlSaMess713 = "           <Start Customer>",
		*mlSaMess714 = "           <End Customer>",
		*mlSaMess715 = "           <C(ompany)|B(ranch)>",
		*mlSaMess716 = "           <S(ummary)|D(etailed)>",
		*mlSaMess717 = "Usage : %s <report type> <lower> <upper> <lpno> <br. no> <with cost margin [Y|N]>",
		*mlSaMess718 = "Usage : %s <by_what> <Output_to> <Report_type> <type> <lower> <upper> <lpno - if Printing> <Br_no> <Print Cost-Margin>",
		*mlSaMess719 = "Usage : %s <report_name> ",
		*mlSaMess720 = "Usage : %s <prog_name, C(ust | P(roduct> ",
		*mlSaMess721 = "           <by_what, C(ust | T (cust class | S(alesman | I(tem No | A (category> ",
		*mlSaMess722 = "           <selection, I(tem No | C(ategory> ",
		*mlSaMess723 = "<Sales Analysis is by C(ustomer) or P(roduct) only> ",
		*mlSaMess724 = "<Type must be C(ustomer), T(ype), or S(alesman) only> ",
		*mlSaMess725 = "<Type must be I(tem) or C(ategory) only> ",
		*mlSaMess726 = "Usage : %s <maintenance name>",
		*mlSaMess727 = "Usage : %s <lpno> <start group> <end group> <C(ompany)|B(ranch)>",
		*mlSaMess728 = "Usage : %s <by what>",
		*mlSaMess729 = "           <report type>",
		*mlSaMess730 = "           <type>",
		*mlSaMess731 = "           <selection [I/C]>",
		*mlSaMess732 = "           <lower>",
		*mlSaMess733 = "           <upper>",
		*mlSaMess734 = "           <lpno - if Printing>",
		*mlSaMess735 = "           <Br. no.>",
		*mlSaMess736 = "           <Print Cost-Margin>",
		*mlSaMess737 = "Usage : %s <sales budget type>",
		*mlSaMess738 = "<sales budget type> 0 - For Salesman",
		*mlSaMess739 = "                    1 - For Category",
		*mlSaMess740 = "                    2 - For Area",
		*mlSaMess741 = "                    3 - For Customer Class",
		*mlSaMess742 = "Usage : %s <program name>",
		*mlSaMess743 = "Usage : %s <program description> <C(ategory)/I(tem)>",
		*mlSaMess744 = "Usage : %s <program name>",
		*mlSaMess745 = "        <by_what [C(ustomer)/P(roduct)>",
		*mlSaMess746 = "		<output_to [D(isplay)/P(rint)>",
		*mlSaMess747 = "		<type [C(ustomer)/T(Cust Class)/S(alesman)/I(tem)/A(Category)>",
		*mlSaMess748 = "Usage : %s <purge date>" ,
		*mlSaMess749 = "Usage : %s <description> <lpno> <start date> <end date>" ;
#endif	
