/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: ol_audit.c,v 5.4 2002/01/09 01:17:39 scott Exp $
|  Program Name  : (ol_audit.c) 
|  Program Desc  : (Invoicing Audit Print)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: ol_audit.c,v $
| Revision 5.4  2002/01/09 01:17:39  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.3  2001/08/09 09:14:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:32:40  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:16  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_audit.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_audit/ol_audit.c,v 5.4 2002/01/09 01:17:39 scott Exp $";

#include	<pslscr.h>
#include	<ml_ol_mess.h>
#include	<ml_std_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<twodec.h>
#include	<Costing.h>

#define	TYPE		cohr_rec.type [0]
#define	OL_INVOICE	 (TYPE == 'O')
#define	OL_CREDIT	 (TYPE == 'R')
#define	SO_INVOICE	 (TYPE == 'I')
#define	SO_CREDIT	 (TYPE == 'C')
#define	INVOICE		 (SO_INVOICE || OL_INVOICE)
#define	CREDIT		 (SO_CREDIT || OL_CREDIT)
#define	SAVED		 (saveType [0] == 'S')
#define	CONFIRMED	 (saveType [0] == 'C')
#define	CASH		 (!strcmp (esmr_rec.sales_acc,  cumr_rec.dbt_no))
#define	VALID_INVOICE	 (((CONFIRMED && (SO_INVOICE || SO_CREDIT)) || \
				         (SAVED && (OL_INVOICE || OL_CREDIT))) && \
					        cohr_rec.date_raised >= dateToday)

	long	dateToday;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;

/*===========
 Table names
============*/
static char *data	= "data", 
			*inmr2	= "inmr2";

/*=============
 Globals (urk!)
==============*/
	double	totalLines [4], 
			grandLines [11];

	double	percentCost 	= 0.00, 
			percentSale 	= 0.00, 
			percentActual  	= 0.00;

	int		cnt, 
			firstFlag		= 0, 
			supFlag			= 0, 
			before			= 0,
			envDbNettUsed	= 0;

	char	printerNumber, 
			sup_part [17], 
			*sup_char = " *", 
			saveType [2];

	FILE	*fout, 
			*fin;

#include 	<pr_format3.h>

/*====================
 Function declarations
======================*/
void 	ProcessFile 	(char *);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadEsmr 		(void);
void 	EndReport 		(void);
int 	ProcessColn 	(long);
int 	FindCohr 		(int);
int 	FindIncc 		(void);
int 	FindInmr2 		(long);
int 	ReportHeading 	(int);
int 	HeadingPrint 	(void);
int 	InvoiceTotal 	(void);
int 	PrintLine 		(long);
int 	check_page 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	  int argc,  
	  char *argv [])
{
	char	*sptr;

	if (argc != 4 && argc != 3)
	{
		print_at (0, 0,  "Usage : %s <printerNumber> <B(efore A(fter)> <S(aved) C(onfirmed)>", argv [0]);
		return (EXIT_FAILURE);
	}
	if (argv [2] [0] == 'B')
		before = 1;
		
	if (argv [3] [0] == 'S')
		strcpy (saveType,  "S");
	else
		strcpy (saveType,  "C");
		
	/*---------------------------
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	read_comm (comm_list,  COMM_NO_FIELDS,  (char *) &comm_rec);
	ReadEsmr ();

	sprintf (temp_str,  "%s Invoice Audit Print.", 
				 (saveType [0] == 'S') ? "Saved" : "Confirmed");
	dsp_screen (temp_str, comm_rec.co_no, comm_rec.co_name);

	dateToday	=	TodaysDate ();
	ReportHeading (atoi (argv [1]));

	grandLines [0] = 0.00;
	grandLines [1] = 0.00;
	grandLines [2] = 0.00;
	grandLines [3] = 0.00;
	grandLines [4] = 0.00;
	grandLines [5] = 0.00;
	grandLines [6] = 0.00;
	grandLines [7] = 0.00;

	if (CONFIRMED)
	{
		/*-------------------
		| Process invoices. |
		-------------------*/
		ProcessFile ("I");

		/*------------------
		| Process credits. |
		------------------*/
		ProcessFile ("C");
	}
	else
	{
		/*-------------------------
		| Process saved invoices. |
		-------------------------*/
		ProcessFile ("O");

		/*------------------------
		| Process saved credits. |
		------------------------*/
		ProcessFile ("R");
	}
	EndReport ();	

	/*=========================
	| Program exit sequence	. |
	=========================*/
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*==========================
| Main processing routine. |
==========================*/
void 
ProcessFile (
	char *type)
{
	int		validCount = 0;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	sprintf (cohr_rec.type, "%-1.1s",  type);
	strcpy (cohr_rec.inv_no, "        ");

	cc = FindCohr (GTEQ);
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
		   !strcmp (cohr_rec.br_no, comm_rec.est_no) && 
		   type [0] == cohr_rec.type [0])
	{
		/*------------------------------------------------------------
		| Check if order is of a valid type (I/C) (Saved / Confirmed)|
		------------------------------------------------------------*/
		if (!VALID_INVOICE)
		{
			cc = FindCohr (NEXT);
			continue;
		}

		/*---------------------------------------
		| Check if order has correct stat flag. |
		---------------------------------------*/
		if (before == 1) 
		{
			validCount = 0;
			switch (cohr_rec.stat_flag [0])
			{
				case '1':
				case '9':
				case 'D':
					validCount = 1;
					break;
				default:
					validCount = 0;
					break;
			}
			if (validCount)
			{
				cc = FindCohr (NEXT);
				continue;
			}
		}
		else 
		{
			if ( cohr_rec.stat_flag [0] != '9' &&
			     cohr_rec.stat_flag [0] != 'D' &&
			     cohr_rec.stat_flag [0] != '1')
			{
				cc = FindCohr (NEXT);
				continue;
			}
		}

		/*-----------------------
		| Print Invoice Header. |
		-----------------------*/
		HeadingPrint ();

		/*----------------------
		| Print Invoice Lines. |
		----------------------*/
		ProcessColn (cohr_rec.hhco_hash);

		/*----------------------
		| Print Invoice Total. |
		----------------------*/
		InvoiceTotal ();
		
		cc = FindCohr (NEXT);
	}
}

/*================
| Open Database. |
================*/
void 
OpenDB (
	void)
{
	abc_dbopen (data);

	abc_alias (inmr2,  inmr);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*=================
| Close Database. |
=================*/
void
CloseDB (
	void)
{
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (incc);
	abc_fclose (inmr2);
	CloseCosting ();
	abc_dbclose (data);
}

/*=====================================
| Get branch info from database file. |
=====================================*/
void
ReadEsmr (
	void)
{
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	strcpy (esmr_rec.est_no,  comm_rec.est_no);
	cc = find_rec (esmr,  &esmr_rec,  COMPARISON,  "r");
	if (cc)
		file_err (cc,  esmr,  "DBFIND");
}

/*=====================
| Read invoice Lines. |
=====================*/
int 
ProcessColn (
	long	hhcoHash)
{
	/*--------------------------
	| Process all order lines. |
	--------------------------*/
	coln_rec.hhco_hash	= hhcoHash;
	coln_rec.line_no	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		PrintLine (coln_rec.hhbr_hash);
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int 
FindCohr (
	int		searchType)
{
	cc = find_rec (cohr, &cohr_rec, searchType, "r");
	if (!cc)
	{
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		return (find_rec (cumr, &cumr_rec, COMPARISON, "r"));
	}
	return (EXIT_FAILURE);
}

int 
FindIncc (
	void)
{
	incc_rec.hhcc_hash = coln_rec.incc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc == 999) 
		sys_err ("Error in incc During (DBFIND)", cc, PNAME);
	return (cc);
}

int 
FindInmr2 (
	long	hhbrHash)
{
	supFlag = 0; /* Sets supercession flag to 0 i.e. No Supercession */

	cc = find_hash (inmr2,  &inmr2_rec,  COMPARISON,  "r",  hhbrHash);
	if (cc == 0)
	{
  	    strcpy (inmr_rec.co_no,  inmr2_rec.co_no);
  	    strcpy (inmr_rec.item_no,  inmr2_rec.item_no);
	    cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	    supFlag = 0;
	    sprintf (sup_part, "%-16.16s", inmr_rec.item_no);
	    if (cc == 0) 
	    {
			while (strcmp (inmr_rec.supercession, "                ") != 0)
			{
		    	strcpy (inmr_rec.co_no, comm_rec.co_no);
		    	strcpy (inmr_rec.item_no, inmr_rec.supercession);
		    	supFlag = 1;
		    	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		    	if (cc != 0)
			    	break;
			}
	    }
	}
	if (cc == 0)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		return (EXIT_SUCCESS);
	}
	return (EXIT_FAILURE);
}

int 
ReportHeading (
	int		printerNumber)
{


	if (! (fout = popen ("pformat", "w"))) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if (! (fin = pr_open ("ol_audit.p"))) 
		sys_err ("Error in ol_audit.p During (FOPEN)", errno, PNAME);

	pr_format (fin, fout, "HEAD1", 1, DateToString (dateToday));
	pr_format (fin, fout, "PITCH", 0, 0);
	fprintf (fout, ".LP%d\n",  printerNumber);
	pr_format (fin, fout, "HEAD2", 0, 0);
	pr_format (fin, fout, "PAGE_LEN", 0, 0);
	pr_format (fin, fout, "HEAD3", 0, 0);
	pr_format (fin, fout, "HEAD4", 0, 0);
	fprintf (fout, ".ECompany : %s\n", comm_rec.co_name);
	fprintf (fout, ".EBranch :%s/ Warehouse :%s\n", clip (comm_rec.est_name), clip (comm_rec.cc_name));
	pr_format (fin, fout, "HEAD5", 0, 0);
	if (SAVED)
		pr_format (fin, fout, "HEAD6", 0, 0);
	else 
		pr_format (fin, fout, "HEAD7", 0, 0);

	pr_format (fin, fout, "HEAD9", 0, 0);
	pr_format (fin, fout, "HEAD10", 0, 0);
	pr_format (fin, fout, "HEAD11", 0, 0);
	pr_format (fin, fout, "HEAD12", 0, 0);
	pr_format (fin, fout, "HEAD13", 0, 0);
	pr_format (fin, fout, "HEAD14", 0, 0);
	fprintf (fout, ".PI12\n");
	return (EXIT_SUCCESS);
}

int 
HeadingPrint (
	void)
{
	char	tmp_str [128];

	dsp_process ("Customer : ", cumr_rec.dbt_no);
	if (firstFlag)
		pr_format (fin, fout, "HEAD11", 0, 0);

	firstFlag = 1;
	
	pr_format (fin, fout, "HEADER1", 1, "     ");
	pr_format (fin, fout, "HEADER1", 2, "     ");

	if (INVOICE)
		pr_format (fin, fout, "HEADER1", 3, "Invoice");
	else
		pr_format (fin, fout, "HEADER1", 3, " C/Note");

	pr_format (fin, fout, "HEADER1", 4, cohr_rec.inv_no);
	pr_format (fin, fout, "HEADER1", 5, "     ");
	pr_format (fin, fout, "HEADER1", 6, cumr_rec.dbt_no);
	pr_format (fin, fout, "HEADER1", 7,  (CASH) ? cohr_rec.dl_name :
					        cumr_rec.dbt_name);
	pr_format (fin, fout, "HEADER1", 8, DateToString (cohr_rec.date_raised));
	pr_format (fin, fout, "HEADER1", 9, cohr_rec.sale_code);
	pr_format (fin, fout, "HEADER1", 10, cohr_rec.op_id);
	pr_format (fin, fout, "HEADER2", 1, cohr_rec.cus_ord_ref);
	pr_format (fin, fout, "HEADER2", 2, cohr_rec.ord_ref);
	if (CREDIT)
	{
		sprintf (tmp_str,  "Credit Invoice : %-8.8s", 
						cohr_rec.app_inv_no);
		pr_format (fin, fout, "HEADER2", 3, tmp_str);
	}
	else
		pr_format (fin, fout, "HEADER2", 3, " ");
	return (EXIT_SUCCESS);
}

void 
EndReport (
	void)
{
	percentCost = grandLines [4];
	percentSale = DOLLARS (grandLines [6]);
	percentActual = -99.99;
	if (percentCost == percentSale)
		percentActual = 0.0;
	else
		if (percentSale != 0.00)
			percentActual = ((percentSale - percentCost) * 100.0/ percentSale);

	pr_format (fin, fout, "HEAD11", 0, 0);
	pr_format (fin, fout, "FOOTER", 1, grandLines [0]);
	pr_format (fin, fout, "FOOTER", 2, grandLines [1]);
	pr_format (fin, fout, "FOOTER", 3, grandLines [2]);
	pr_format (fin, fout, "FOOTER", 4, grandLines [3]);
	pr_format (fin, fout, "FOOTER", 5, grandLines [4]);
	pr_format (fin, fout, "FOOTER", 6, grandLines [5]);
	pr_format (fin, fout, "FOOTER", 7, grandLines [6]);
	pr_format (fin, fout, "FOOTER", 8, grandLines [7]);
	pr_format (fin, fout, "FOOTER", 9, grandLines [8]);
	pr_format (fin, fout, "FOOTER", 10, percentActual);
	pr_format (fin, fout, "HEAD11", 0, 0);
	fprintf (fout, ".EOF\n");
	pclose (fout);
	fclose (fin);
}

int 
InvoiceTotal (
	void)
{
	double	gst_freight = 0.00;
	double	b_extend = 0.00;
	double	a_extend = 0.00;

	double	freight_other = 0.00;

	freight_other = (cohr_rec.freight + 
			  		 cohr_rec.insurance + 
			  		 cohr_rec.other_cost_1 + 
			  		 cohr_rec.other_cost_2 + 
			  		 cohr_rec.other_cost_3 +
		          	 cohr_rec.sos);

	if (freight_other != 0.00)
	{
		/*-----------------------------------------------
		| Check if tax can be calculated or tax exempt. |
		-----------------------------------------------*/
		if (comm_rec.gst_rate == 0.00 || 
			cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		{
			gst_freight = 0.00;
		}
		else
		{
			/*------------------------
			| Add in Gst on freight. |
			------------------------*/
			gst_freight  = (comm_rec.gst_rate / 100);
			gst_freight *= freight_other;
		}
		pr_format (fin, fout, "LINE1", 1, "FR+IN+O");
		pr_format (fin, fout, "LINE1", 2, "FR +INSU+ OTHER");
		pr_format (fin, fout, "LINE1", 3, 1.0);
		pr_format (fin, fout, "LINE1", 4, 0.0);
		pr_format (fin, fout, "LINE1", 5, freight_other);
		pr_format (fin, fout, "LINE1", 6, freight_other);
		pr_format (fin, fout, "LINE1", 7, 0.00);
		pr_format (fin, fout, "LINE1", 8, 0.00);
		pr_format (fin, fout, "LINE1", 9, 0.00);
		pr_format (fin, fout, "LINE1", 10, freight_other);
		pr_format (fin, fout, "LINE1", 11, gst_freight);
		pr_format (fin, fout, "LINE1", 12, gst_freight + freight_other);
		pr_format (fin, fout, "LINE1", 13, 0.00);
		pr_format (fin, fout, "LINE1", 14, " ");

		if (INVOICE)
		{
			totalLines [0] += 1;
			totalLines [1] += 0;
			totalLines [2] += freight_other;
		}
		else
		{
			totalLines [0] -= 1;
			totalLines [1] -= 0;
			totalLines [2] -= freight_other;
		}
	}

	pr_format (fin, fout, "HEAD14", 0, 0);

	if (INVOICE)
		pr_format (fin, fout, "TOT_LINE1", 1, "INVOICE TOTAL ");
	else
		pr_format (fin, fout, "TOT_LINE1", 1, "CREDIT TOTAL .");

	if (envDbNettUsed)
		percentSale = DOLLARS (((cohr_rec.gross + cohr_rec.tax) - cohr_rec.disc));
	else
		percentSale = DOLLARS (cohr_rec.gross + cohr_rec.tax);

	percentCost = totalLines [3];
	percentActual = -99.99;

	if (CREDIT)
		percentCost *= -1.00;

	if (percentCost == percentSale)
		percentActual = 0.0;
	else
		if (percentCost != percentSale && percentSale != 0.00)
			percentActual = ((percentSale - percentCost) * 100.0/ percentSale);

	if (envDbNettUsed)
	{
		b_extend	=	cohr_rec.gross + cohr_rec.tax + freight_other - 
		  		   		(cohr_rec.disc + cohr_rec.ex_disc);

		a_extend	=	cohr_rec.gross + cohr_rec.tax + cohr_rec.gst + 
						freight_other - (cohr_rec.disc + cohr_rec.ex_disc);
	}
	else
	{
		b_extend	=	cohr_rec.gross + cohr_rec.tax + freight_other - 
		  		   		cohr_rec.ex_disc;

		a_extend	=	cohr_rec.gross + cohr_rec.tax + cohr_rec.gst + 
						freight_other - cohr_rec.ex_disc;
	}

	if (INVOICE)
	{
		pr_format (fin, fout, "TOT_LINE1", 2, totalLines [0]);
		pr_format (fin, fout, "TOT_LINE1", 3, totalLines [1]);
		pr_format (fin, fout, "TOT_LINE1", 4, totalLines [2]);
		pr_format (fin, fout, "TOT_LINE1", 5, 
					cohr_rec.gross + freight_other);
		pr_format (fin, fout, "TOT_LINE1", 6, totalLines [3]);
		pr_format (fin, fout, "TOT_LINE1", 7, cohr_rec.disc + cohr_rec.ex_disc);
		pr_format (fin, fout, "TOT_LINE1", 8, b_extend);
		pr_format (fin, fout, "TOT_LINE1", 9, cohr_rec.gst);
		pr_format (fin, fout, "TOT_LINE1", 10, a_extend);
		pr_format (fin, fout, "TOT_LINE1", 11, percentActual);
		grandLines [0] += totalLines [0];
		grandLines [1] += totalLines [1];
		grandLines [2] += totalLines [2];
		grandLines [3] += cohr_rec.gross;
		grandLines [4] += totalLines [3];
		grandLines [5] += cohr_rec.disc + cohr_rec.ex_disc;
		grandLines [6] += b_extend;
		grandLines [7] += cohr_rec.gst;
		grandLines [8] += a_extend;
	}
	else
	{
		pr_format (fin, fout, "TOT_LINE1", 2, totalLines [0]);
		pr_format (fin, fout, "TOT_LINE1", 3, totalLines [1]);
		pr_format (fin, fout, "TOT_LINE1", 4, totalLines [2]);
		pr_format (fin, fout, "TOT_LINE1", 5, 
				 (cohr_rec.gross + freight_other) * -1);
		pr_format (fin, fout, "TOT_LINE1", 6, totalLines [3]);
		pr_format (fin, fout, "TOT_LINE1", 7, cohr_rec.disc * -1);
		pr_format (fin, fout, "TOT_LINE1", 8, b_extend * -1);
		pr_format (fin, fout, "TOT_LINE1", 9, cohr_rec.gst * -1);
		pr_format (fin, fout, "TOT_LINE1", 10, a_extend * -1);
		pr_format (fin, fout, "TOT_LINE1", 11, percentActual);
		grandLines [0] += totalLines [0];
		grandLines [1] += totalLines [1];
		grandLines [2] += totalLines [2];
		grandLines [3] -= cohr_rec.gross;
		grandLines [4] += totalLines [3];
		grandLines [5] -= cohr_rec.disc;
		grandLines [6] -= b_extend;
		grandLines [7] -= cohr_rec.gst;
		grandLines [8] -= a_extend;
	}
	totalLines [0] = 0.00;
	totalLines [1] = 0.00;
	totalLines [2] = 0.00;
	totalLines [3] = 0.00;
	return (EXIT_SUCCESS);
}

/*======================
| Print Invoice Lines. |
======================*/
int 
PrintLine (
	long	hhbrHash)
{
	double	extendPrice	= 0.00, 
			cost		= 0.00, 
			extendCost	= 0.00, 
			extendSale	= 0.00;

	int		badCost		= 0;

	cc = FindInmr2 (hhbrHash);
	if (cc)
		return (cc);

	cc = FindIncc ();
	if (cc)
		return (cc);

	if (envDbNettUsed)
		extendPrice = ((coln_rec.gross + coln_rec.amt_tax) - coln_rec.amt_disc);
	else
		extendPrice = ((coln_rec.gross + coln_rec.amt_tax) - coln_rec.amt_disc);
	extendPrice = no_dec (extendPrice);

	if (CREDIT)
	{
		extendPrice *= -1 ;
		incc_rec.closing_stock += coln_rec.q_order;
	}

	if (coln_rec.cost_price == 0.00 || inmr_rec.costing_flag [0] == 'S')
	{
	    switch (inmr_rec.costing_flag [0])
	    {
	    case 'A':
	    case 'L':
	    case 'P':
	    case 'T':
	    	cost	=	FindIneiCosts 
						(
							inmr_rec.costing_flag, 
							cohr_rec.br_no,
							coln_rec.hhbr_hash
						);
	    	break;

	    case 'S':
			cost	=	FindInsfCost
					(
						incc_rec.hhwh_hash,
						0L,
						coln_rec.serial_no,
						"C"
					);
			if (cost == -1.00)
			{
				cost	=	FindInsfCost
						(
							incc_rec.hhwh_hash,
							0L,
							coln_rec.serial_no,
							"F"
						);
			}
			if (cost == -1.00)
				cost = 0.00;
			break;

	    case 'F':
			cost	=	FindIncfValue 
						(
							incc_rec.hhwh_hash, 
							incc_rec.closing_stock, 
							coln_rec.q_order, 
							TRUE,
							inmr_rec.dec_pt
						);
    		break;
   
	    case 'I':
	    	cost	=	FindIncfValue 
						(
							incc_rec.hhwh_hash, 
							incc_rec.closing_stock, 
							coln_rec.q_order, 
							FALSE,
							inmr_rec.dec_pt
						);
	    	break;
    
	    default:
	    	break;
	    }
	}
	else
		cost = DOLLARS (coln_rec.cost_price);

	if (cost <= 0.00)
	{
		cost	=	FindIneiCosts 
					(
						"L",
						cohr_rec.br_no,
						coln_rec.hhbr_hash
					);
		if (cost <= 0.0)
		{
			cost	=	FindIneiCosts 
						(
							"T",
							cohr_rec.br_no,
							coln_rec.hhbr_hash
						);
		}
		badCost = 1;
	}
	else
		badCost = 0;

	extendSale = out_cost (coln_rec.sale_price, inmr_rec.outer_size);
	extendCost = out_cost (cost, inmr_rec.outer_size);

	pr_format (fin, fout, "LINE1", 1, inmr_rec.item_no);
	pr_format (fin, fout, "LINE1", 2, coln_rec.item_desc);
	pr_format (fin, fout, "LINE1", 3, coln_rec.q_order);
	pr_format (fin, fout, "LINE1", 4, coln_rec.q_backorder);
	pr_format (fin, fout, "LINE1", 5, extendSale);
	pr_format (fin, fout, "LINE1", 6, coln_rec.gross);
	pr_format (fin, fout, "LINE1", 7, extendCost * coln_rec.q_order);
	pr_format (fin, fout, "LINE1", 8, coln_rec.disc_pc);
	pr_format (fin, fout, "LINE1", 9, coln_rec.amt_disc);
	if (envDbNettUsed)
	{
		pr_format (fin, fout, "LINE1", 10,  (coln_rec.gross + 
				          					 coln_rec.amt_tax) - 
				          					 coln_rec.amt_disc);
		pr_format (fin, fout, "LINE1", 11, coln_rec.amt_gst);
		pr_format (fin, fout, "LINE1", 12,  (coln_rec.gross + 
				          					 coln_rec.amt_tax + 
				          					 coln_rec.amt_gst) - 
				          					 coln_rec.amt_disc);
    }
	else
	{
		pr_format (fin, fout, "LINE1", 10,  coln_rec.gross + coln_rec.amt_tax);
		pr_format (fin, fout, "LINE1", 11, coln_rec.amt_gst);
		pr_format (fin, fout, "LINE1", 12, coln_rec.gross + 
										   coln_rec.amt_tax + 
										   coln_rec.amt_gst);
	}
	percentCost = out_cost (cost, inmr_rec.outer_size);
	percentCost	*= coln_rec.q_order;
	if (envDbNettUsed)
		percentSale = DOLLARS (coln_rec.gross - coln_rec.amt_disc);
	else
		percentSale = DOLLARS (coln_rec.gross);

	percentActual = -99.99;
	if (INVOICE)
	{
		percentCost *= -1.00;
		percentSale *= -1.00;
	}

	if (percentCost == 0.00)
	{
		percentActual = 100.0;
	}
	else
	{
		if (percentCost == percentSale)
			percentActual = 0.0;
		else
			if (percentCost != percentSale && percentSale != 0.00)
				percentActual = ((percentSale - percentCost) * 100.0/ percentSale);
	}
	pr_format (fin, fout, "LINE1", 13, percentActual);

	if (badCost)
		sprintf (err_str, "%c (NCF)", sup_char [supFlag]);
	else
		sprintf (err_str, "%c      ", sup_char [supFlag]);

	pr_format (fin, fout, "LINE1", 14, err_str);

	if (INVOICE)
	{
		totalLines [0] += coln_rec.q_order;
		totalLines [1] += coln_rec.q_backorder;
		totalLines [2] += extendSale; 
		totalLines [3] += extendCost * coln_rec.q_order;
	}
	else
	{
		totalLines [0] -= coln_rec.q_order;
		totalLines [1] -= coln_rec.q_backorder;
		totalLines [2] -= extendSale;
		totalLines [3] -= extendCost * coln_rec.q_order;
	}
	return (EXIT_SUCCESS);
}

int 
check_page (
	void)
{
	return (EXIT_SUCCESS);
}
