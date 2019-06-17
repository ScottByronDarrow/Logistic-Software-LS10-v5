/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (so_taxprt.c                  )                    |
|  Program Desc  : (Sales Tax Summary Report.                   )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  exst, comm,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (  )                                               |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 23/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (26/04/89)      | Modified  by : Rog Gibbison.     |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (03/02/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (11/09/97)      | Modified  by : Marnie Organo.    |
|                                                                     |
|  Comments      : Use btree structure.                               |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (03/02/93)    : PSL 6828 changes for mult-curr SO.                 |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: so_taxprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_taxprt/so_taxprt.c,v 5.2 2001/08/09 09:22:09 scott Exp $";

#include	<pslscr.h>
#include	<time.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#define	CCMAIN
#define	NO_SCRGEN

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_gst_rate"},
		{"comm_pay_terms"}
	};

	int comm_no_fields = 8;
	
	struct	{
		int	 termno;
		char	tco_no [3];
		char	tco_name [41];
		char	tes_no [3];
		char	tes_name [41];
		long	t_dbt_date;
		float	t_gst_pc;
		int	tpay_terms;
	} comm_rec;

	/*=================================+
	 | External Sales Tax Master File. |
	 +=================================*/
#define	EXST_NO_FIELDS	8

	struct dbview	exst_list [EXST_NO_FIELDS] =
	{
		{"exst_co_no"},
		{"exst_year"},
		{"exst_period"},
		{"exst_tax_code"},
		{"exst_tax_percent"},
		{"exst_sales_value"},
		{"exst_tax_value"},
		{"exst_stat_flag"}
	};

	struct tag_exstRecord
	{
		char	co_no [3];
		int		year;
		char	period [3];
		char	tax_code [2];
		float	tax_percent;
		Money	sales_value;
		Money	tax_value;
		char	stat_flag [2];
	}	exst_rec;


float	rate;
char	*tax_codes = "ABCD";
#include 	<pr_format3.h>

#define	TNUL	 (struct	_btree *)0

typedef	struct	_btree	{
	float	_rate;				/* tax rate						*/
	char	_code [4] [2];		/* tax code, "A" "B" "C" or "D"	*/
	double	_tax [4];			/* taxable value				*/
	double	_sales [4];			/* sales value					*/
	struct	_btree	*_left;		/* left child					*/
	struct	_btree	*_right;	/* right child					*/
} BTREE;

BTREE	*tree_head;				/* root of tree					*/

#include	<btree.h>

	FILE	*fout;
	FILE	*fin;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	printerNumber;
	int	envVarDbMcurr;

	double	totalTax [4];


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	Compare 		(BTREE *tptr);
int  	InitNode 		(BTREE *tptr);
void 	Process 		(void);
int  	PrintTable 		(BTREE *tptr);
void 	PrintTotal 		(void);
void 	HeaderPrint 	(void);
int  	check_page 		(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	char	*sptr = chk_env ("DB_MCURR");

	if (sptr)
		envVarDbMcurr = atoi (sptr);
	else
		envVarDbMcurr = FALSE;

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	OpenDB ();


	dsp_screen ("Processing Sales Tax Summary",comm_rec.tco_no,comm_rec.tco_name);
	for (i = 0;i < 4;i++)
		totalTax [i] = 0.00;
	
	Process ();

	shutdown_prog ();
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

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec ("exst",exst_list,EXST_NO_FIELDS,"exst_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("exst");
	abc_dbclose ("data");
}

/*=======================================
| Comparison routine			|
| Returns :				|
|	-1	rate < node rate	|
|	0	rate = node rate	|
|	1	rate > node rate	|
=======================================*/
int
Compare (
 BTREE *tptr)
{
	if (tptr->_rate == rate)
		return (EXIT_SUCCESS);

	if (tptr->_rate > rate)
		return (-1);

	return (EXIT_FAILURE);
}

int
InitNode (
 BTREE *tptr)
{
	register	int	i;

	tptr->_rate = 0.00;
	for (i = 0;i < 4;i++)
	{
		strcpy (tptr->_code [i]," ");
		tptr->_tax [i] = 0.00;
		tptr->_sales [i] = 0.00;
	}
    return (EXIT_SUCCESS);	
}

/*
print_node (tptr)
BTREE	*tptr;
{
	register	int	i;

	fprintf (stderr,"Rate : %6.2f     Taxable\tSales Value\n",tptr->_rate);
	for (i = 0;i < 4;i++)
	{
		fprintf (stderr,"\t\t%s ",tptr->_code [i]);
		fprintf (stderr,"%10.2f ",DOLLARS (tptr->_tax [i]));
		fprintf (stderr,"%10.2f\n",DOLLARS (tptr->_sales [i]));
	}
}
*/

void
Process (
 void)
{
	int	i;

	BTREE	*tptr;
	char	taxPeriod [3];
	int		taxYear;
	int		monthPeriod;

	DateToDMY (comm_rec.t_dbt_date, NULL, &monthPeriod, &taxYear);
	sprintf (taxPeriod, "%02d", monthPeriod);

	strcpy (exst_rec.co_no,comm_rec.tco_no);
	exst_rec.year	=	taxYear;
	strcpy (exst_rec.period,taxPeriod);
	strcpy (exst_rec.tax_code," ");
	exst_rec.tax_percent = 0.0;
	cc = find_rec ("exst",&exst_rec,GTEQ,"r"); 
	while (!cc && !strcmp (exst_rec.co_no,comm_rec.tco_no) && 
				  !strcmp (exst_rec.period, taxPeriod) && 
				 exst_rec.year == taxYear)
	{
		for (i = 0;i < strlen (tax_codes) && tax_codes [i] != exst_rec.tax_code [0];i++)
			;

		if (i < strlen (tax_codes))
		{
			rate = exst_rec.tax_percent;

			tptr = add_node (tree_head,Compare,InitNode);

			if (tree_head == TNUL)
				tree_head = tptr;

			if (tptr != TNUL)
			{
				if (tptr->_code [i] [0] == ' ')
				{
					tptr->_rate = exst_rec.tax_percent;
					tptr->_tax [i] = exst_rec.tax_value;
					tptr->_sales [i] = exst_rec.sales_value;
					strcpy (tptr->_code [i],exst_rec.tax_code);
				}
				else
				{
					tptr->_tax [i] += exst_rec.tax_value;
					tptr->_sales [i] += exst_rec.sales_value;
				}
			}
		}
		cc = find_rec ("exst",&exst_rec,NEXT,"r"); 
	}

	HeaderPrint ();

	print_tree (tree_head,PrintTable);

	PrintTotal ();

	fprintf (fout,".EOF\n");
	pclose (fout);
}

int
PrintTable (
 BTREE *tptr)
{
	register	int	i;
	int	valid;
	double	total_tax = 0.00;

	for (i = 0,valid = 0;!valid && i < 4;i++)
	{
		if (tptr->_tax [i] != 0.00 || tptr->_sales [i] != 0.00)
			valid = 1;
	}

	if (!valid)
		return (EXIT_SUCCESS);
		
	sprintf (err_str,"%5.2f",tptr->_rate);
	dsp_process ("Tax Rate :",err_str);

	pr_format (fin,fout,"TAX",1,tptr->_rate);

	for (i = 0;i < 4;i++)
	{
		pr_format (fin,fout,"TAX",i + 2,tptr->_tax [i]);
		totalTax [i] += tptr->_tax [i];
		total_tax += tptr->_tax [i];
	}

	pr_format (fin,fout,"TAX",6,total_tax);

	for (i = 0;i < 4;i++)
		pr_format (fin,fout,"SALES", (i + 1),tptr->_sales [i]);

	pr_format (fin,fout,"LINE1",0,0);
    return (EXIT_SUCCESS);
}

void
PrintTotal (
 void)
{
	register	int	i;
	double		total_tax = 0.00;

	for (i = 0;i < 4;i++)
	{
		pr_format (fin,fout,"TOTAL",i + 1,totalTax [i]);
		total_tax += totalTax [i];
	}

	pr_format (fin,fout,"TOTAL",5,total_tax);
}

/*=======================================================
| Routine to open output pipe to standard print.	|
=======================================================*/
void
HeaderPrint (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	if ((fin = pr_open ("so_taxprt.p")) == 0) 
		sys_err ("Error in so_taxprt.p During (FOPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.t_dbt_date), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".PI12\n");
	fprintf (fout,".12\n");
	fprintf (fout,".L113\n");
	fprintf (fout,".ECOMPANY SALES TAX REPORT\n");
	fprintf (fout,".E%s\n",clip (comm_rec.tco_name));
	fprintf (fout,".EAS AT %s\n",SystemTime ());
	if (envVarDbMcurr)
		fprintf (fout,".EAll Values Are In Local Currency \n");
	else
		fprintf (fout,".B1\n");
	pr_format (fin,fout,"RULEOFF",0,0);
	pr_format (fin,fout,"RULER",0,0);
	pr_format (fin,fout,"HEAD1",0,0);
	pr_format (fin,fout,"HEAD2",0,0);
	pr_format (fin,fout,"LINE1",0,0);
	pr_format (fin,fout,"HEAD3",0,0);
	pr_format (fin,fout,"LINE1",0,0);
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}
