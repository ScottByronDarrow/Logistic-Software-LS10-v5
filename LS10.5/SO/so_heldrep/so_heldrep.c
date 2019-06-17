/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
| $Id: so_heldrep.c,v 5.3 2001/10/23 07:16:40 scott Exp $
|  Program Name  : (so_heldrep.c)           
|  Program Desc  : (Print Held Orders Report)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap    | Date Written  : 31/10/88         |
|---------------------------------------------------------------------|
| $Log: so_heldrep.c,v $
| Revision 5.3  2001/10/23 07:16:40  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_heldrep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_heldrep/so_heldrep.c,v 5.3 2001/10/23 07:16:40 scott Exp $";

#include	<pslscr.h>
#include	<ml_so_mess.h>
#include	<ml_std_mess.h>

#define	PAGELINES	65

#define	LINE_OK		 (soln_rec.status [0] == 'H' || \
			 soln_rec.status [0] == 'O' || \
			 soln_rec.status [0] == 'C')

#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<pr_format3.h>
#include 	<twodec.h>

#include	"schema"

struct commRecord	comm_rec;
struct exsfRecord	exsf_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct inmrRecord	inmr_rec;

	int		envVarRepTax 		= FALSE,
			envVarDbMcurr		= FALSE,
			envVarDbNettUsed 	= TRUE,
			lpno 				= 1,
			rep_tax				= 0,
			found_data 			= 0,
			line_no 			= 0;

	double	cust_tot     		= 0.00,
			lcl_cust_tot 		= 0.00;

	char	data_str [101];
	char	systemDate [11];

	FILE	*fin;
	FILE	*fsort;
	FILE	*fout;

	char	*sptr;


/*=======================
| Function Declarations |
=======================*/
int  	check_page 			(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	ProcessOrders 		(void);
double 	CalculateLine 		(void);
void 	StoreOrderLines 	(int, double);
void 	ProcessSortedList 	(void);
long 	LongValue 			(char *);
double 	FloatValue 			(char *);
void 	PrintOrderLine 		(char *, double, char *, double);
void 	PrintCustomerLine 	(char *, long, double);
void 	PrintCustomerTotal 	(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char *sptr = get_env ("DB_MCURR");

	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc != 2)
	{
		print_at (0,0,mlSoMess705,argv [0]);
		return (EXIT_FAILURE);
	}

	/*-----------------------
	| Printer Number	|
	-----------------------*/
	lpno = atoi (argv [1]);

	strcpy (systemDate, DateToString (TodaysDate ()));

	init_scr ();

	OpenDB ();


	dsp_screen ("Printing Held Orders Report",
					comm_rec.co_no,comm_rec.co_name);

	ProcessOrders ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
check_page (
 void)
{
	if (line_no > PAGELINES)
		line_no = 0;
	line_no++;
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (exsf);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	/*------------------
	| Open format file |
	------------------*/
	if ((fin = pr_open ("so_heldrep.p")) == NULL)
		sys_err ("Error in opening so_heldrep.p during (FOPEN)",errno,PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",lpno);

	fprintf (fout, ".8\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	pr_format (fin,fout,"HEAD1",1,comm_rec.dbt_date);
	pr_format (fin,fout,"BLANK",0,0);
	pr_format (fin,fout,"LINE1",0,0);
	pr_format (fin,fout,"RULER",0,0);
	fprintf (fout, "|%-144.144s|\n", "                    NOTE : O/S Order  includes Unposted Invoices and Credits, Current Orders, Released Backorders");
	line_no = 4;
	fflush (fout);
}

void
ProcessOrders (
 void)
{
	int	ord_found;
	int	margin_fail;
	double	ord_value = 0.00;

	fsort = sort_open ("so_held");

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,"  ");
	sohr_rec.hhcu_hash = 0L;
	sprintf (sohr_rec.order_no,"%-8.8s"," ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");

	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no))
	{
		margin_fail = FALSE;
		ord_found = FALSE;
		ord_value = 0.00;

		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln,&soln_rec,GTEQ,"r");

		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (LINE_OK)
			{
				if (!found_data)
					found_data = TRUE;
				
				if (!ord_found)
				{
				    cc = find_hash (cumr,&cumr_rec,COMPARISON,
						"r",sohr_rec.hhcu_hash);
				    if (cc)
					file_err (cc, cumr, "DBFIND");

				    ord_found = TRUE;
				}
				ord_value += CalculateLine ();
				if (soln_rec.status [0] == 'O') 
					margin_fail = TRUE;
			}
			cc = find_rec (soln,&soln_rec,NEXT,"r");
		}
		if (ord_found)
			StoreOrderLines (margin_fail, ord_value);

		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
	if (found_data)
		ProcessSortedList ();
}
/*=======================
| Calculate line total. |
=======================*/
double	
CalculateLine (
 void)
{
	double	net_val = 0.00;
	double	lcl_ex_rate;

	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;


	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		return (0.00);
   	
	lcl_ex_rate = (envVarDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00;
	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) soln_rec.qty_order + soln_rec.qty_bord;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);
			
		if (envVarDbNettUsed)
			net_val	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			net_val	=	l_total + l_tax + l_gst + soln_rec.item_levy;
	}
	net_val = no_dec (net_val / lcl_ex_rate);

	return (net_val);
}

void
StoreOrderLines (
 int marg_fail, 
 double ord_value)
{
	sprintf (data_str,
		"%-9.9s %-1.1s %-2.2s %-8.8s %10.2f %6ld %9.4f\n",
		cumr_rec.dbt_acronym,
		 (marg_fail) ? "M" : "C",
		sohr_rec.br_no,
		sohr_rec.order_no,
		DOLLARS (ord_value),
		sohr_rec.hhcu_hash,
		sohr_rec.exch_rate);

	sort_save (fsort,data_str);
}

void
ProcessSortedList (
 void)
{
	char	prev_acro [10];
	char	curr_acro [10];
	char	prev_br [3];
	char	curr_br [3];
	char	curr_type [2];
	char	curr_order [9];
	int	first_time = TRUE;
	long	hhcu_hash = 0L;
	double	order_val   = 0.00;
	double	lcl_ex_rate = (double) 0;

	fsort = sort_sort (fsort,"so_held");
	sptr = sort_read (fsort);

	if (sptr != (char *) 0)
	{
		sprintf (prev_acro,"%-9.9s"," ");
		strcpy (prev_br,"  ");
	}

	HeadingOutput ();

	while (sptr != (char *) 0)
	{
		sprintf (curr_acro ,"%-9.9s", sptr);
		sprintf (curr_br,   "%-2.2s", sptr + 12);
		sprintf (curr_order,"%-8.8s", sptr + 15);
		sprintf (curr_type ,"%-1.1s", sptr + 10);
		order_val = FloatValue (sptr + 24);
		hhcu_hash = LongValue (sptr + 35);
		lcl_ex_rate = atof (sptr + 42);

		dsp_process ("Customer Acronym : ",curr_acro);

		if (strcmp (prev_acro,curr_acro) || strcmp (prev_br,curr_br))
		{
			if (!first_time)
			{
				first_time = FALSE;
				PrintCustomerTotal ();
			}

			PrintCustomerLine (curr_br, hhcu_hash, lcl_ex_rate);
			strcpy (prev_acro,curr_acro);
			strcpy (prev_br,curr_br);
		}
		if (PAGELINES - line_no >= 65)
		{
			fprintf (fout, ".PA\n");
			PrintCustomerLine (curr_br, hhcu_hash, lcl_ex_rate);
		}
		PrintOrderLine (curr_order,
			    order_val, 
			   (curr_type [0] == 'M') ? "Margin Exceeded" : "Credit Exceeded",
			    lcl_ex_rate);
		
		first_time = FALSE;

		sptr = sort_read (fsort);
	}
	if (PAGELINES - line_no >= 65)
	{
		fprintf (fout, ".PA\n");
		PrintCustomerLine (prev_br, hhcu_hash, lcl_ex_rate);
	}
	PrintCustomerTotal ();
	sort_delete (fsort,"so_held");
	pr_format (fin,fout,"END_FILE",0,0);
	pclose (fout);
}

long	
LongValue (
 char *str)
{
	char	val [7];

	sprintf (val,"%-6.6s",str);
	return (atol (val));
}

double	
FloatValue (
 char *str)
{
	char	val [11];

	sprintf (val,"%-10.10s",str);
	return (atof (val));
}

void
PrintOrderLine (
 char *order, 
 double ord_val, 
 char *reason, 
 double ex_rate)
{
	double	lcl_ex_rate;
	double	lcl_ord_val = (double) 0;

	if (envVarDbMcurr)
	{
		pr_format (fin,fout,"ORDER_DET",1,reason);
		pr_format (fin,fout,"ORDER_DET",2,order);
		pr_format (fin,fout,"ORDER_DET",3,ord_val);

		lcl_ex_rate = (envVarDbMcurr && ex_rate != 0.00) ? ex_rate : 1.00;

		lcl_ord_val = ord_val / lcl_ex_rate;
	    	pr_format (fin,fout,"ORDER_DET",4,lcl_ord_val);
	}
	else
	{
		pr_format (fin,fout,"ORDER_DET2",1,reason);
		pr_format (fin,fout,"ORDER_DET2",2,order);
		pr_format (fin,fout,"ORDER_DET2",3,ord_val);
	}
	fflush (fout);
	cust_tot += ord_val;
	lcl_cust_tot += lcl_ord_val;
}

void
PrintCustomerLine (
 char *br, 
 long hhcu_hash, 
 double ex_rate)
{
	double	total_val = 0.00;
	double	lcl_total_val = 0.00;
	double	lcl_ex_rate;

	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",hhcu_hash);
	if (cc)
		sprintf (cumr_rec.dbt_name,"%-40.40s"," ");

	total_val = cumr_rec.bo_current + 
		    	cumr_rec.bo_per1 +
		    	cumr_rec.bo_per2 +
		    	cumr_rec.bo_per3 +
		    	cumr_rec.bo_per4 +
		    	cumr_rec.bo_fwd +
		    	cumr_rec.ord_value;
	
	/*-------------------
	| Convert to local. |
	-------------------*/
	lcl_ex_rate = (envVarDbMcurr && ex_rate != 0.00) ? ex_rate : 1.00;

	cumr2_rec.bo_current = no_dec (cumr_rec.bo_current 	/ lcl_ex_rate);
	cumr2_rec.bo_per1 	 = no_dec (cumr_rec.bo_per1 	/ lcl_ex_rate);
	cumr2_rec.bo_per2 	 = no_dec (cumr_rec.bo_per2 	/ lcl_ex_rate);
	cumr2_rec.bo_per3 	 = no_dec (cumr_rec.bo_per3 	/ lcl_ex_rate);
	cumr2_rec.bo_per4 	 = no_dec (cumr_rec.bo_per4 	/ lcl_ex_rate);
	cumr2_rec.bo_fwd 	 = no_dec (cumr_rec.bo_fwd 	 	/ lcl_ex_rate);
	cumr2_rec.ord_value  = no_dec (cumr_rec.ord_value 	/ lcl_ex_rate);

	lcl_total_val 	= cumr2_rec.bo_current + 
		    		  cumr2_rec.bo_per1 +
		    		  cumr2_rec.bo_per2 +
		    		  cumr2_rec.bo_per3 +
		    		  cumr2_rec.bo_per4 +
		    		  cumr2_rec.bo_fwd +
		    		  cumr2_rec.ord_value;

	fprintf (fout, ".LRP9\n");

	pr_format (fin,fout,"CUST_HEAD",0,0);
	pr_format (fin,fout,"CUST_UHD",0,0);

	pr_format (fin,fout,"CUST_DET",1,cumr_rec.dbt_no);
	pr_format (fin,fout,"CUST_DET",2,br);
	pr_format (fin,fout,"CUST_DET",3,cumr_rec.dbt_name);
	pr_format (fin,fout,"CUST_DET",4, (envVarDbMcurr) ? cumr_rec.curr_code : " ");
	pr_format (fin,fout,"CUST_DET",5, cumr_rec.bo_current + cumr_rec.bo_fwd);
	pr_format (fin,fout,"CUST_DET",6,cumr_rec.bo_per1);
	pr_format (fin,fout,"CUST_DET",7,cumr_rec.bo_per2);
	pr_format (fin,fout,"CUST_DET",8,cumr_rec.bo_per3 + cumr_rec.bo_per4);
	pr_format (fin,fout,"CUST_DET",9,cumr_rec.ord_value);
	pr_format (fin,fout,"CUST_DET",10,total_val);
	pr_format (fin,fout,"CUST_DET",11,cumr_rec.phone_no);

	if (envVarDbMcurr)
	{
		pr_format (fin,fout,"CUST_DET",1," ");
		pr_format (fin,fout,"CUST_DET",2," ");
		pr_format (fin,fout,"CUST_DET",3," ");
		pr_format (fin,fout,"CUST_DET",4,"Local");
		pr_format (fin,fout,"CUST_DET",5,cumr2_rec.bo_current + cumr2_rec.bo_fwd);
		pr_format (fin,fout,"CUST_DET",6,cumr2_rec.bo_per1);
		pr_format (fin,fout,"CUST_DET",7,cumr2_rec.bo_per2);
		pr_format (fin,fout,"CUST_DET",8,cumr2_rec.bo_per3 + cumr2_rec.bo_per4);
		pr_format (fin,fout,"CUST_DET",9,cumr2_rec.ord_value);
		pr_format (fin,fout,"CUST_DET",10,lcl_total_val);
		pr_format (fin,fout,"CUST_DET",11," ");
	}

	pr_format (fin,fout,"LINE3",0,0);
	pr_format (fin,fout,"ORDER_HEAD",1, (envVarDbMcurr) ? "O R D E R   V A L U E" : "ORDER VALUE");
	pr_format (fin,fout,"ORDER_HEAD",2,cumr_rec.stop_credit);
	pr_format (fin,fout,"ORDER_HEAD",3,cumr_rec.crd_prd);
	pr_format (fin,fout,"ORDER_HEAD",4,cumr_rec.credit_limit);
	pr_format (fin,fout,"ORDER_HEAD",5, (envVarDbMcurr) ? cumr_rec.curr_code : " ");
	if (envVarDbMcurr)
		pr_format (fin,fout,"ORDER_HD2", 1,cumr_rec.curr_code);
	fflush (fout);
}

void
PrintCustomerTotal (
 void)
{
	if (envVarDbMcurr)
	{
		pr_format (fin,fout,"TOT_LINE",0,0);
		pr_format (fin,fout,"CUST_TOT",1,cust_tot);
	    	pr_format (fin,fout,"CUST_TOT",2,lcl_cust_tot);
	}
	else
	{
		pr_format (fin,fout,"TOT_LINE2",0,0);
		pr_format (fin,fout,"CUST_TOT2",1,cust_tot);
	}
	pr_format (fin,fout,"LINE2",0,0);
	fflush (fout);
	cust_tot = 0.00;
	lcl_cust_tot = 0.00;
}
