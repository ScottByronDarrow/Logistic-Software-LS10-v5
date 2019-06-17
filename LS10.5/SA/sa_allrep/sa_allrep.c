/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sa_allrep.c,v 5.3 2002/02/26 06:42:03 scott Exp $
|  Program Name  : (sk_all_rep.c) 
|  Program Desc  : (Sales Analysis Report)
|                 (Prints Several different Reports)
|                 (1) Sales Analysis by Salesman)
|                 (2) Sales Analysis by Category)
|                 (3) Sales Analysis by Customer by Category)
|                 (4) Sales Analysis by Area Code)
|                 (5) Sales Analysis by Customer Type)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/04/87         |
|---------------------------------------------------------------------|
| $Log: sa_allrep.c,v $
| Revision 5.3  2002/02/26 06:42:03  scott
| Updated to convert to app.schema + clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_allrep.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_allrep/sa_allrep.c,v 5.3 2002/02/26 06:42:03 scott Exp $";

#include <ml_sa_mess.h>	
#include <ml_std_mess.h>	
#include <pslscr.h>	
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct exafRecord	exaf_rec;
struct excfRecord	excf_rec;
struct exclRecord	excl_rec;
struct saleRecord	sale_rec;
struct exsfRecord	exsf_rec;
struct cudpRecord	cudp_rec;
struct sasrRecord	sasr_rec;

	struct {
		char	*heading;
		char	*report;
		char	*desc;
	} head_list [5] = {
		{"SALESMAN", "SALESPERSON  ", "UNITS         "}, 
		{"CATEGORY", "CATEGORY   ", "CAT NUMBER ..... UNITS"}, 
		{"CUSTOMER BY CATEGORY", " ", " "}, 
		{"AREA CODE", "AREA CODES  ", "UNITS         "}, 
		{"CUSTOMER TYPE", "CUSTOMER TYPES", "CODES ..... UNITS   "}, 
	};

#define	SALES	0
#define	CATG	1
#define	C_CATG	2
#define	A_CODE	3
#define	C_TYPE	4

#define	BY_CO	0
#define	BY_BR	1
#define	BY_DP	2
#define	BY_WH	3

	int		envSaExp		=	FALSE, 
			envSaFreight	=	FALSE, 
			cnt				=	0, 
			firstRec 		= TRUE, 
			first_line 		= TRUE, 
	 		lowerLength		= 0, 
			printerNo 		= 1, 
			reportType		= 0, 
			analysis		= 0, 
			subrange		= 0;

	FILE	*fout;

	char	categories [2][28], 
			dp_wh [3], 
			lower [12], 
			upper [12], 
			mask [10], 
			low_value [12], 
			cur_month [3], 
			fiscal [3], 
			*expand (char *, char *); 

struct {
	char	code [12];
	char	description [41];
} local_rec;

double	sale_data [8];
double	sale_unit [4];

double	data_tot [8];
double	unit_tot [4];

double	data_grand [8];
double	unit_grand [4];

#include	<SaCategory.h>
/*
 * Local Function Prototypes.
 */
double 	Percent 			(double, double);
int 	DeltaCode 			(int);
int 	InFiscal 			(void);
int 	InRange 			(void);
int 	ProcessData 		(void);
int 	ReadDetail 			(int);
int 	ReadSale 			(int);
int 	ValidRecord 		(void);
void 	CheckACode 			(int);
void 	CheckCategory 		(int);
void 	CheckCCategory 		(void);
void 	CheckCCtype 		(int);
void 	CheckSales 			(int);
void 	CloseDB 			(void);
void 	HeadingOutput		(void);
void 	InitCustomer 		(void);
void 	InitSale 			(void);
void 	InitTotal 			(void);
void 	OpenDB 				(void);
void 	PrintSale 			(int);
void 	PrintSaleLine 		(int, int);
void 	PrintSubRange 		(int);
void 	PrintTotal 			(char *, char *, double *, double *);
void 	ReadMisc 			(void);
void 	SumSales 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc, 
 char*  argv [])
{
	char	*sptr;
	int	cal_fiscal;
	int		currentMonth;


	if (argc != 8)
	{
		print_at (0, 0, mlSaMess700, argv [0]);
        return (EXIT_FAILURE);
	}
	
	envSaExp = atoi (get_env ("SA_EXP"));

	envSaFreight = TRUE;
	if (chk_env ("SA_FREIGHT"))
		envSaFreight = atoi (get_env ("SA_FREIGHT"));

	if (argv [4][0] < '0' || argv [4][0] > '4')
	{
		print_at (1, 0, mlSaMess701);
		print_at (2, 0, mlSaMess702);
		print_at (3, 0, mlSaMess703);
		print_at (4, 0, mlSaMess704);
		print_at (5, 0, mlSaMess705);
		print_at (6, 0, mlSaMess706);
		print_at (7, 0, mlSaMess707);
		print_at (8, 0, mlSaMess708);
		print_at (9, 0, mlSaMess709);
        return (EXIT_FAILURE);
	}
	
	if (argv [5][0] < '0' || argv [5][0] > '3')
	{
		print_at (0, 0, "Analysis must be [0..3]\007\n");
        return (EXIT_FAILURE);
	}
	
	printerNo = atoi (argv [1]);
	sprintf (lower, "%-.11s", clip (argv [2]));
	reportType = atoi (argv [4]);
	analysis = atoi (argv [5]);
	sprintf (dp_wh, "%-2.2s", argv [6]);
	subrange = (argv [7][0] == 'Y');

	switch (reportType)
	{
	case SALES:
	case A_CODE:
		sprintf (upper, "%-2.2s", clip (argv [3]));
		lowerLength = 2;
		break;

	case CATG:
		sprintf (upper, "%-11.11s", clip (argv [3]));
		lowerLength = strlen (lower);
		if (lowerLength == 0 || envSaExp)
			lowerLength = 11;
		break;

	case C_CATG:
		sprintf (upper, "%-6.6s", clip (argv [3]));
		lowerLength = 6;
		break;

	case C_TYPE:
		sprintf (upper, "%-3.3s", clip (argv [3]));
		lowerLength = 3;
		break;
	}
	sprintf (mask, "%%-%d.%ds", lowerLength, lowerLength);

	ReadMisc ();

	init_scr ();

	if (subrange && reportType == CATG)
		LoadListStruct ();

	subrange = (subrange && reportType == CATG && headPtr != (struct listRec *)0);
	DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, NULL);
	sprintf (cur_month, "%02d", currentMonth);

	sptr = chk_env ("SA_YEND");
	cal_fiscal = (sptr == (char *)0) ? comm_rec.fiscal : atoi (sptr);

	if (cal_fiscal < 1 || cal_fiscal > 12)
		cal_fiscal = comm_rec.fiscal;

	sprintf (fiscal, "%02d", cal_fiscal);

	OpenDB ();

	sprintf (err_str, "Processing : SALES ANALYSIS BY %s %s%s", 
		head_list [reportType].heading, 
		 (subrange) ? " (Subrange) " : " ", 
		 (envSaExp) ? " (Detailed)" : " (Summary)");
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	HeadingOutput ();

	cc = ProcessData ();

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	fprintf (fout, ".EOF\n");
	pclose (fout);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	int	head_lines = 0;

	switch (analysis)
	{
	case BY_CO:
		head_lines = 13;
		break;

	case BY_BR:
		head_lines = 14;
		break;

	case BY_DP:
	case BY_WH:
		head_lines = 15;
		break;
	}
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".%d\n", head_lines);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L146\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESALES ANALYSIS BY %s\n", head_list [reportType].heading);
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	
	if (analysis != BY_CO)
		fprintf (fout, ".EBranch: %s \n", clip (comm_rec.est_name));

	if (analysis == BY_DP)
		fprintf (fout, ".EDepartment: %s \n", clip (cudp_rec.dp_name));

	if (analysis == BY_WH)
		fprintf (fout, ".EWarehouse: %s \n", clip (comm_rec.cc_name));

	fprintf (fout, ".EFor Month %s\n", cur_month);
	fprintf (fout, ".E AS AT : %-26.26s\n", SystemTime ());

	fprintf (fout, ".R=============================");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "==========");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "===========\n");

	fprintf (fout, "=============================");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "==========");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "===========\n");

	fprintf (fout, "|       %14.14s       ", head_list [reportType].report);
	fprintf (fout, "|                     MONTHLY  COMPARISON                 ");
	fprintf (fout, "|                  YEAR  TO  DATE  COMPARISON             |\n");

	if (reportType == C_CATG)
		fprintf (fout, "|----------------------------");
	else
		fprintf (fout, "|             AND            ");

	fprintf (fout, "|-----------");
	fprintf (fout, "------------");
	fprintf (fout, "------------");
	fprintf (fout, "------------");
	fprintf (fout, "----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "------------");
	fprintf (fout, "------------");
	fprintf (fout, "------------");
	fprintf (fout, "----------|\n");

	if (reportType == C_CATG)
		fprintf (fout, "| CUST |CUSTOMER | CATEGORY  ");
	else
		fprintf (fout, "|   %22.22s   ", head_list [reportType].desc);

	fprintf (fout, "|THIS MONTH ");
	fprintf (fout, "|THIS MONTH ");
	fprintf (fout, "|A YEAR AGO ");
	fprintf (fout, "|A YEAR AGO ");
	fprintf (fout, "| PERCENT ");
	fprintf (fout, "| THIS YEAR ");
	fprintf (fout, "| THIS YEAR ");
	fprintf (fout, "| LAST YEAR ");
	fprintf (fout, "| LAST YEAR ");
	fprintf (fout, "| PERCENT |\n");

	if (reportType == C_CATG)
		fprintf (fout, "| CODE | ACRONYM |           ");
	else
		fprintf (fout, "|                            ");

	fprintf (fout, "|   SALES   ");
	fprintf (fout, "|  PROFITS  ");
	fprintf (fout, "|   SALES   ");
	fprintf (fout, "|  PROFITS  ");
	fprintf (fout, "|         ");
	fprintf (fout, "|   SALES   ");
	fprintf (fout, "|  PROFITS  ");
	fprintf (fout, "|   SALES   ");
	fprintf (fout, "|  PROFITS  ");
	fprintf (fout, "|         |\n");

	fflush (fout);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	switch (reportType)
	{
	case SALES:
		open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
		open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_sman");
		break;

	case CATG:
		open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
		open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_category");
		break;

	case C_CATG:
		open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
		open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
		open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_id_no_2");
		break;

	case A_CODE:
		open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
		open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_area");
		break;

	case C_TYPE:
		open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
		open_rec (sale, sale_list, SALE_NO_FIELDS, "sale_ctype");
		break;
	}
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (exaf);
	abc_fclose (excf);
	abc_fclose (excl);
	abc_fclose (sale);
	abc_fclose (exsf);
	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (analysis == BY_DP)
	{
		open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, dp_wh);

		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
			sys_err ("Error in cudp During (DBFIND)", cc, PNAME);
		abc_fclose (cudp);
	}
}

int
ProcessData (void)
{
	InitSale ();
	InitCustomer ();
	InitTotal ();

	cc = ReadSale (GTEQ);
	if (cc)
		return (cc);

	cc = ReadDetail (reportType);
	
	if (reportType == C_CATG)
		cc = ReadDetail (CATG);

	while (cc || !InRange ())
	{
		cc = ReadSale (NEXT);
		if (cc)
			return (cc);

		cc = ReadDetail (reportType);
		
		if (reportType == C_CATG)
			cc = ReadDetail (CATG);
	}

	/*-----------------------
	| valid sale records	|
	-----------------------*/
	while (!cc && InRange ())
	{
		if (!envSaFreight && !strncmp (sale_rec.category, "FR+INS+OTHE", 11))
		{
			cc = ReadSale (NEXT);
			continue;
		}

		CheckCCategory ();
		CheckSales (FALSE);
		CheckCategory (FALSE);
		CheckCCtype (FALSE);
		CheckACode (FALSE);

		if (ValidRecord ())
		{
			SumSales ();
		}

		cc = ReadSale (NEXT);
	}
	CheckCCategory ();
	CheckSales (TRUE);
	CheckCategory (TRUE);
	CheckCCtype (TRUE);
	CheckACode (TRUE);

/*
	if (reportType == C_CATG)
		PrintTotal ("CUSTOMER TOTAL", "CUSTOMER UNITS", data_tot, unit_tot);
*/

	if (subrange)
	{
		currPtr = headPtr;
		while (currPtr != (struct listRec *)0)
		{
			PrintSubRange (TRUE);
			currPtr = currPtr->nextRecord;
		}
	}

	PrintTotal ("GRAND TOTAL", "GRAND UNITS", data_grand, unit_grand);
	return (EXIT_SUCCESS);
}

void
PrintSubRange (
 int    last_total)
{
	if (last_total || strcmp (currPtr->catEnd, local_rec.code) <= 0)
	{
		/*---------------------------------------
		| Has this record been printed yet ??	|
		---------------------------------------*/
		if (currPtr->catStart [0] != 'Y')
			return;

		sprintf (categories [0], "Catg From    :%-11.11s", 
			currPtr->catStart);

		sprintf (categories [1], "Catg To      :%-11.11s", 
			currPtr->catEnd);

		PrintTotal (categories [0], 
			categories [1], 
			currPtr->sumData, 
			currPtr->sumUnit);

		strcpy (currPtr->catStart, "N");
	}
}

void
CheckCCategory (void)
{
	if (reportType == C_CATG && DeltaCode (C_CATG))
	{
		PrintSaleLine (cnt, first_line);
		if (cnt != 0)
			firstRec = FALSE;

		cc = ReadDetail (C_CATG);
		cc = ReadDetail (CATG);
		if (cnt != 0)
			PrintTotal ("CUSTOMER TOTAL", "CUSTOMER UNITS", data_tot, unit_tot);
		InitSale ();
		InitCustomer ();
	}
}

void
CheckSales (
 int    last_group)
{
	if (reportType == SALES && (DeltaCode (SALES) || last_group))
	{
		PrintSaleLine (cnt, first_line);
		if (cnt != 0)
			firstRec = FALSE;

		cc = ReadDetail (SALES);
		InitSale ();
	}
}

void
CheckCategory (
 int    last_group)
{
	if ((reportType == CATG || reportType == C_CATG) && (DeltaCode (CATG) || last_group))
	{
		PrintSaleLine (cnt, first_line);
		if (cnt != 0)
			firstRec = FALSE;

		cc = ReadDetail (CATG);
		InitSale ();
	}
}

void
CheckACode (
 int    last_group)
{
	if (reportType == A_CODE && (DeltaCode (A_CODE) || last_group))
	{
		PrintSaleLine (cnt, first_line);
		if (cnt != 0)
			firstRec = FALSE;

		cc = ReadDetail (A_CODE);
		InitSale ();
	}
}

void
CheckCCtype (
 int    last_group)
{
	if (reportType == C_TYPE && (DeltaCode (C_TYPE) || last_group))
	{
		PrintSaleLine (cnt, first_line);
		if (cnt != 0)
			firstRec = FALSE;

		cc = ReadDetail (C_TYPE);
		InitSale ();
	}
}

/*=======================================================
| Check if the code that sale is read by has changed	|
=======================================================*/
int
DeltaCode (
 int    file_id)
{
	switch (file_id)
	{
	case SALES:
		return (strcmp (sale_rec.sman, exsf_rec.salesman_no));

	case CATG:
		return (strncmp (sale_rec.category, low_value, strlen (low_value)));

	case C_CATG:
		return (strcmp (sale_rec.dbt_no, cumr_rec.dbt_no));

	case A_CODE:
		return (strcmp (sale_rec.area, exaf_rec.area_code));

	case C_TYPE:
		return (strcmp (sale_rec.ctype, excl_rec.class_type));
	}
	return (EXIT_SUCCESS);
}

/*===============================================================
| print sale line iff some valid sale records have been read	|
===============================================================*/
void
PrintSaleLine (
 int    cnt, 
 int    first_line)
{
	if (cnt != 0)
	{
		dsp_process (" : ", local_rec.description);
		PrintSale (first_line);
	}
}

/*=====================================================================
| Checks whether appropriate code	                                  |
| is in subrange input.			                                      |
=====================================================================*/
int
InRange (void)
{
	int	valid = 0;

	switch (reportType)
	{
	case SALES:
		valid = strncmp (sale_rec.sman, upper, 2);
		break;

	case CATG:
		valid = strncmp (sale_rec.category, upper, 11);
		break;

	case C_CATG:
		valid = strncmp (sale_rec.dbt_no, upper, 6);
		break;

	case A_CODE:
		valid = strncmp (sale_rec.area, upper, 2);
		break;

	case C_TYPE:
		valid = strncmp (sale_rec.ctype, upper, 3);
		break;
	}
	return ((valid <= 0));
}

/*=====================================================================
| initialise salesman/category/customer type/area totals	          |
=====================================================================*/
void
InitSale (void)
{
	int	i;

	cnt = 0;
	first_line = FALSE;
	for (i = 0;i < 8;i++)
	{
		if (i < 4)
			sale_unit [i] = 0.00;
		sale_data [i] = 0.00;
	}
}

/*===============================
| initialise customer totals	|
===============================*/
void
InitCustomer (void)
{
	int	i;

	first_line = TRUE;
	for (i = 0;i < 8;i++)
	{
		if (i < 4)
			unit_tot [i] = 0.00;
		data_tot [i] = 0.00;
	}
}

/*===============================
| initialise grand totals	|
===============================*/
void
InitTotal (void)
{
	int	i;

	for (i = 0;i < 8;i++)
	{
		if (i < 4)
			unit_grand [i] = 0.00;
		data_grand [i] = 0.00;
	}
}

/*=====================================================================
| print total lines iwth appropriate descriptions.                    |
=====================================================================*/
void
PrintTotal (
 char*      tot_desc, 
 char*      unit_desc, 
 double*    total_data, 
 double*    total_units)
{

	fprintf (fout, ".LRP4\n");

	fprintf (fout, "=============================");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "==========");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "===========\n");

	fprintf (fout, "| %-26.26s ", tot_desc);
	fprintf (fout, "|%11.2f", DOLLARS (total_data [0]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [1]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [2]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [3]));
	fprintf (fout, "|%9.2f", Percent (total_data [0], total_data [2]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [4]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [5]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [6]));
	fprintf (fout, "|%11.2f", DOLLARS (total_data [7]));
	fprintf (fout, "|%9.2f|\n", Percent (total_data [5], total_data [7]));

	fprintf (fout, "|                            ");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|---------|\n");

	fprintf (fout, "| %-26.26s ", unit_desc);
	fprintf (fout, "|%11.2f", total_units [0]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%11.2f", total_units [1]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%9.2f", Percent (total_units [0], total_units [1]));
	fprintf (fout, "|%11.2f", total_units [2]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%11.2f", total_units [3]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%9.2f|\n", Percent (total_units [2], total_units [3]));

	fflush (fout);
}

/*=======================================================
| sum all the appropriate sales records for the master	|
=======================================================*/
void
SumSales (void)
{
	double	sales = (sale_rec.gross - sale_rec.disc);
	double	profit = sales - sale_rec.cost_sale;

	/*-----------------------
	| Monthly Comparison	|
	-----------------------*/
	if (sale_rec.year_flag [0] == 'C' && !strcmp (sale_rec.period, cur_month))
	{
		sale_data [0]	+= sales;
		data_tot [0]	+= sales;
		data_grand [0]	+= sales;
		sale_unit [0]	+= sale_rec.units;
		unit_tot [0]	+= sale_rec.units;
		unit_grand [0]	+= sale_rec.units;
		sale_data [1]	+= profit;
		data_tot [1]	+= profit;
		data_grand [1]	+= profit;
		cnt++;
	}


	if (sale_rec.year_flag [0] == 'L' && !strcmp (sale_rec.period, cur_month))
	{
		sale_data [2]	+= sales;
		data_tot [2]	+= sales;
		data_grand [2]	+= sales;
		sale_unit [1]	+= sale_rec.units;
		unit_tot [1]	+= sale_rec.units;
		unit_grand [1]	+= sale_rec.units;
		sale_data [3]	+= profit;
		data_tot [3]	+= profit;
		data_grand [3]	+= profit;
		cnt++;
	}

	/*-------------------------------
	| Year to date Comparison	|
	-------------------------------*/
	if (sale_rec.year_flag [0] == 'C' && InFiscal ())
	{
		sale_data [4]	+= sales;
		data_tot [4]	+= sales;
		data_grand [4]	+= sales;
		sale_unit [2]	+= sale_rec.units;
		unit_tot [2]	+= sale_rec.units;
		unit_grand [2]	+= sale_rec.units;
		sale_data [5]	+= profit;
		data_tot [5]	+= profit;
		data_grand [5]	+= profit;
		cnt++;
	}

	if (sale_rec.year_flag [0] == 'L' && InFiscal ())
	{
		sale_data [6]	+= sales;
		data_tot [6]	+= sales;
		data_grand [6]	+= sales;
		sale_unit [3]	+= sale_rec.units;
		unit_tot [3]	+= sale_rec.units;
		unit_grand [3]	+= sale_rec.units;
		sale_data [7]	+= profit;
		data_tot [7]	+= profit;
		data_grand [7]	+= profit;
		cnt++;
	}
	if (cnt != 0 && reportType == C_CATG)
		strcpy (cumr_rec.dbt_no, sale_rec.dbt_no);
}

/*===============================
| print the sales analysis data	|
===============================*/
void
PrintSale (
 int first_line)
{
 	fprintf (fout, ".LRP4\n");

	fprintf (fout, "|============================");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|=========");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|===========");
	fprintf (fout, "|=========|\n");

	if (reportType == C_CATG)
	{
		if (first_line)
		{
			fprintf (fout, "|%-6.6s", cumr_rec.dbt_no);
			fprintf (fout, "|%-9.9s", cumr_rec.dbt_acronym);
		}
		else
		{
			fprintf (fout, "|%-6.6s", " ");
			fprintf (fout, "|%-9.9s", " ");
		}
		fprintf (fout, "|%-11.11s", local_rec.code);
	}
	else
		fprintf (fout, "| %-11.11s                ", local_rec.code);
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [0]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [1]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [2]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [3]));
	fprintf (fout, "|%9.2f", Percent (sale_data [0], sale_data [2]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [4]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [5]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [6]));
	fprintf (fout, "|%11.2f", DOLLARS (sale_data [7]));
	fprintf (fout, "|%9.2f|\n", Percent (sale_data [5], sale_data [7]));

	fprintf (fout, "|                            ");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|---------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|---------|\n");

	fprintf (fout, "| %-26.26s ", (reportType == C_CATG) ? " " : local_rec.description);
	fprintf (fout, "|%11.2f", sale_unit [0]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%11.2f", sale_unit [1]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%9.2f", Percent (sale_unit [0], sale_unit [1]));
	fprintf (fout, "|%11.2f", sale_unit [2]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%11.2f", sale_unit [3]);
	fprintf (fout, "| %9.9s ", " ");
	fprintf (fout, "|%9.2f|\n", Percent (sale_unit [2], sale_unit [3]));

	fflush (fout);

	if (!subrange)
		return;

	cc = FindListStruct (local_rec.code, FIRST);
	while (!cc)
	{
		currPtr->sumData [0] += sale_data [0];
		currPtr->sumData [1] += sale_data [1];
		currPtr->sumData [2] += sale_data [2];
		currPtr->sumData [3] += sale_data [3];
		currPtr->sumData [4] += sale_data [4];
		currPtr->sumData [5] += sale_data [5];
		currPtr->sumData [6] += sale_data [6];
		currPtr->sumData [7] += sale_data [7];

		currPtr->sumUnit [0] += sale_unit [0];
		currPtr->sumUnit [1] += sale_unit [1];
		currPtr->sumUnit [2] += sale_unit [2];
		currPtr->sumUnit [3] += sale_unit [3];

		PrintSubRange (FALSE);
		cc = FindListStruct (local_rec.code, NEXT);
	}
}

/*=======================
| calculate Percentage	|
=======================*/
double
Percent (
 double current, 
 double last)
{
	if (last == 0.00)
		return (0.00);

	return (( (current - last) / last) * 100.00);
}

/*=======================================
| TRUE iff comm_rec.fiscal == 0L	|
| or sale_period in fiscal year		|
=======================================*/
int
InFiscal (void)
{
	/*-----------------------
	| valid if fiscal = 0	|
	-----------------------*/
	if (!strcmp (fiscal, "00"))
		return (TRUE);

	if (strcmp (fiscal, cur_month) < 0)
		return ((strcmp (fiscal, sale_rec.period) < 0 && strcmp (sale_rec.period, cur_month) <= 0));
	else
		return ((strcmp (fiscal, sale_rec.period) < 0 || strcmp (sale_rec.period, cur_month) <= 0));
}

/*===============================================
| return TRUE iff the sale record is valid	|
| for the circumstance.				|
===============================================*/
int
ValidRecord (void)
{
	int	valid = 0;

	switch (analysis)
	{
	case BY_CO:
		valid = !strncmp (comm_rec.co_no, sale_rec.key, 2);
		break;
		
	case BY_BR:
		valid = (!strncmp (comm_rec.co_no, sale_rec.key, 2) &&
			!strncmp (comm_rec.est_no, sale_rec.key + 2, 2));
		break;

	case BY_DP:
		valid = (!strncmp (comm_rec.co_no, sale_rec.key, 2) &&
			!strncmp (comm_rec.est_no, sale_rec.key + 2, 2) &&
			!strncmp (cudp_rec.dp_no, sale_rec.key + 4, 2));
		break;

	case BY_WH:
		valid = (!strncmp (comm_rec.co_no, sale_rec.key, 2) &&
			!strncmp (comm_rec.est_no, sale_rec.key + 2, 2) &&
			!strncmp (comm_rec.cc_no, sale_rec.key + 6, 2));
		break;
	}
	return (valid);
}

/*=======================================
| read the appropriate detail file	|
| ie exsf, excf, cumr, exaf, excl.	|
=======================================*/
int
ReadDetail (
 int    file_id)
{
	char	p_mask [12];
	char	tmp_dbt_no [7];

	switch (file_id)
	{
	case SALES:
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, sale_rec.sman);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		strcpy (local_rec.code, exsf_rec.salesman_no);
		strcpy (local_rec.description, exsf_rec.salesman);
		break;

	case CATG:
		strcpy (excf_rec.co_no, comm_rec.co_no);
		if (reportType == C_CATG)
			strcpy (excf_rec.cat_no, sale_rec.category);
		else
		{
			sprintf (p_mask, mask, sale_rec.category);
			sprintf (excf_rec.cat_no, "%-11.11s", p_mask);
		}
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		strcpy (local_rec.code, excf_rec.cat_no);
		strcpy (local_rec.description, excf_rec.cat_desc);
		break;

	case C_CATG:
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, comm_rec.est_no);
		strcpy (tmp_dbt_no, cumr_rec.dbt_no);
		strcpy (cumr_rec.dbt_no, sale_rec.dbt_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, " 0");
			strcpy (cumr_rec.dbt_no, sale_rec.dbt_no);
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		}
		strcpy (local_rec.code, cumr_rec.dbt_no);
		strcpy (local_rec.description, cumr_rec.dbt_acronym);
		strcpy (cumr_rec.dbt_no, tmp_dbt_no);
		break;

	case A_CODE:
		strcpy (exaf_rec.co_no, comm_rec.co_no);
		strcpy (exaf_rec.area_code, sale_rec.area);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		strcpy (local_rec.code, exaf_rec.area_code);
		strcpy (local_rec.description, exaf_rec.area);
		break;

	case C_TYPE:
		strcpy (excl_rec.co_no, comm_rec.co_no);
		strcpy (excl_rec.class_type, sale_rec.ctype);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		strcpy (local_rec.code, excl_rec.class_type);
		strcpy (local_rec.description, excl_rec.class_desc);
		break;
	}

	if (file_id == CATG && reportType == C_CATG)
		strcpy (low_value, local_rec.code);
	else
		sprintf (low_value, mask, local_rec.code);
	if (cc)
		strcpy (local_rec.description, " No Description found ");
	return (cc);
}

/*===============================================
| read the sale file on the appropriate key	|
===============================================*/
int
ReadSale (
 int    stype)
{
	if (stype != NEXT)
	{
		switch (reportType)
		{
		case SALES:
			sprintf (sale_rec.sman, "%-2.2s", lower);
			break;

		case CATG:
			sprintf (sale_rec.category, "%-11.11s", lower);
			break;

		case C_CATG:
			sprintf (sale_rec.dbt_no, "%-6.6s", lower);
			sprintf (sale_rec.category, "%-11.11s", " ");
			break;

		case A_CODE:
			sprintf (sale_rec.area, "%-2.2s", lower);
			break;

		case C_TYPE:
			sprintf (sale_rec.ctype, "%-3.3s", lower);
			break;
		}
	}
	return (find_rec (sale, &sale_rec, stype, "r"));
}
