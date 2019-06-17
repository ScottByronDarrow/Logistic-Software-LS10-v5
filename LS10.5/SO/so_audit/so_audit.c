/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_audit.c,v 5.4 2002/01/09 01:12:04 scott Exp $
|  Program Name  : (so_audit.c)
|  Program Desc  : (Invoicing Audit Print) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: so_audit.c,v $
| Revision 5.4  2002/01/09 01:12:04  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.3  2001/08/09 09:20:35  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:50:46  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:48  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_audit.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_audit/so_audit.c,v 5.4 2002/01/09 01:12:04 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>
#include 	<Costing.h>
#define		ALL_PRNT	 (!strcmp (batch, "ALL  "))

#define		NOTAX		 (cohr_rec.tax_code [0] == 'A' || \
                          cohr_rec.tax_code [0] == 'B')

#define		GST		 	(gstApplied [0] == 'Y')
#define		FAULTY		(coln_rec.crd_type [0] == 'F')
#define		DOLLAR		(coln_rec.crd_type [0] == 'D')
#define		SERIAL		(inmr_rec.costing_flag [0] == 'S')
#define		INVOICE		(cohr_rec.type [0] == 'I')
#define		CREDIT		(cohr_rec.type [0] == 'C')
#define 	POS			TRUE
#define 	NEG			FALSE
#define 	HEAD		TRUE
#define 	LINE		FALSE
#define		AFTER_STAT	(cohr_rec.stat_flag [0] == '1' || \
			          	cohr_rec.stat_flag [0] == '9' || \
			          	cohr_rec.stat_flag [0] == 'D')

#define		BEFORE_STAT	(cohr_rec.stat_flag [0] == '9' ||  \
 			          	cohr_rec.stat_flag [0] == '1')

#define		FLAG_DELETE	 (cohr_rec.stat_flag [0] == '9' ||  \
						  cohr_rec.stat_flag [0] == '1')

FILE *fout, *fin;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;

	char	*data  = "data", 
			*inmr2 = "inmr2";

	int		firstFlag		= 0,
			supFlag			= 0,
			beforeFlag		= 0,
			envDbNettUsed	= 0,
			envDbMcurr 		= TRUE;

	float	gstInclude		= 0.00,
			gstDivide		= 0.00,
			gstPercent		= 0.00;

	double	totalLines [4],
			grandLines [11],
			localExchangeRate	= 0.00,
			percentCost 		= 0.00,
			percentSale 		= 0.00,
			percentActual  		= 0.00;

	char	supPart [17],
			batch [6],
			PrintBatch [15],
			*supChar = " *",
			gstApplied [2];

#include 	<pr_format3.h>

/*=======================
| Function Declarations |
=======================*/
double 	OutGst 			(double, int, int, int);
int		FindCohr 		(int);
int		FindIncc 		(void);
int		FindInmr2 		(long);
int  	PrintLine 		(long);
int  	check_page 		(void);
void	CloseDB 		(void);
void	OpenDB 			(void);
void	ProcessColn 	(long);
void	shutdown_prog	(void);
void 	EndReport 		(void);
void 	HeadingPrint 	(void);
void 	InvoiceTotal 	(void);
void 	NormalGst 		(int);
void 	ProcessBatch 	(void);
void 	ProcessNormal 	(void);
void 	ReportHeading 	(int);
void 	SplitGst 		(int);
void 	ToLocal 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sprintf (gstApplied, "%-1.1s", get_env ("GST"));

	gstInclude = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (gstInclude != 0.00)
	{
		gstDivide 	= (float) (((100.00 + gstInclude) / gstInclude));
		gstPercent  = gstInclude;
	}
	else
	{
		gstDivide 	= 0.00;
		gstPercent  = 0.00;
	}

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	if (sptr)
		envDbMcurr = atoi (sptr);

	if (argc != 3 && argc != 4)
	{
		print_at (0, 0, mlSoMess756, argv [0]);
		return (EXIT_FAILURE);
	}

	if (argc >= 3) 
	{
		beforeFlag = (argv [2] [0] == 'B') ? TRUE : FALSE;
		sprintf (batch, "%-5.5s", (argc == 4) ? argv [3] : "     ");
	}

	/*---------------------------
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	dsp_screen ("Invoicing Audit Print.", comm_rec.co_no, comm_rec.co_name);

	ReportHeading (atoi (argv [1]));

	grandLines [0] = 0.00;
	grandLines [1] = 0.00;
	grandLines [2] = 0.00;
	grandLines [3] = 0.00;
	grandLines [4] = 0.00;
	grandLines [5] = 0.00;
	grandLines [6] = 0.00;
	grandLines [7] = 0.00;

	if (!ALL_PRNT)
		ProcessBatch ();
	else
		ProcessNormal ();

	EndReport ();	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*================
| Open Database. |
================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
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

/*=====================
| Read invoice Lines. |
=====================*/
void
ProcessColn (
	long	hhcoHash)
{
	if (gstDivide != 0.00 && !NOTAX)
		SplitGst (TRUE);
	else
		NormalGst (TRUE);

	/*----------------------------------------------------
	| if envDbMcurr convert cohr2 to local currency      |
	----------------------------------------------------*/
	if (envDbMcurr && localExchangeRate)
		ToLocal (TRUE);
	
	/*--------------------------
	| Process all order lines. |
	--------------------------*/
	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		if (gstDivide != 0.00 && !NOTAX)
			SplitGst (FALSE);
		else
			NormalGst (FALSE);

		/*----------------------------------------------------
		| if envDbMcurr convert coln2 to local currency      |
		----------------------------------------------------*/
		if (envDbMcurr && localExchangeRate > 0.00)
			ToLocal (FALSE);

		PrintLine (coln_rec.hhbr_hash);
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
}

int
FindCohr (
	int	searchType)
{
	cc = find_rec (cohr, &cohr_rec, searchType, "r");
	if (cc)
		return (cc);

	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");

	return (EXIT_SUCCESS);
}

int
FindIncc (void)
{
	incc_rec.hhcc_hash = coln_rec.incc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	return (find_rec (incc, &incc_rec, COMPARISON, "r"));
}

int
FindInmr2 (
	long	hhbrHash)
{
	supFlag = 0; /* Sets supercession flag to 0 i.e. No Supercession */

	inmr2_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (inmr_rec.co_no, inmr2_rec.co_no);
		strcpy (inmr_rec.item_no, inmr2_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		supFlag = 0;
		sprintf (supPart, "%-16.16s", inmr_rec.item_no);
		if (!cc)
		{
			while (strcmp (inmr_rec.supercession, "                "))
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, inmr_rec.supercession);
				supFlag = 1;
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				if (cc)
					break;
			}
		}
	}
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		return (EXIT_SUCCESS);
	}
	return (EXIT_FAILURE);
}

void
ReportHeading (
 int printerNumber)
{
	if ((fout = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("so_audit.p")) == 0) 
		sys_err ("Error in so_audit.p During (FOPEN)", errno, PNAME);

	pr_format (fin, fout, "HEAD1", 1, DateToString (comm_rec.dbt_date));
	if (!beforeFlag)
		fprintf (fout, ".SO\n");

	pr_format (fin, fout, "PITCH", 0, 0);
	fprintf (fout, ".LP%d\n", printerNumber);
	pr_format (fin, fout, "HEAD2", 0, 0);
	pr_format (fin, fout, "PAGE_LEN", 0, 0);
	pr_format (fin, fout, "HEAD3", 0, 0);
	pr_format (fin, fout, "HEAD4", 0, 0);
	fprintf (fout, ".ECompany : %s\n", comm_rec.co_name);
	fprintf (fout, ".EBranch :%s/ Warehouse :%s\n", 
			clip (comm_rec.est_name), clip (comm_rec.cc_name));
	pr_format (fin, fout, "HEAD5", 0, 0);
	if (!strcmp (batch, "ALL  "))
	{
		if (beforeFlag)
			pr_format (fin, fout, "HEAD6", 0, 0);
		else 
			pr_format (fin, fout, "HEAD7", 0, 0);
	}
	else 
		pr_format (fin, fout, "HEAD8", 1, batch);

	pr_format (fin, fout, "HEAD9", 0, 0);
	pr_format (fin, fout, "HEAD10", 0, 0);
	pr_format (fin, fout, "HEAD11", 0, 0);
	if (envDbMcurr)
	{
		pr_format (fin, fout, "MHEAD12", 0, 0);
		pr_format (fin, fout, "MHEAD13", 0, 0);
	}
	else
	{
		pr_format (fin, fout, "HEAD12", 0, 0);
		pr_format (fin, fout, "HEAD13", 0, 0);
	}
	pr_format (fin, fout, "HEAD14", 0, 0);
	fprintf (fout, ".PI12\n");
}

void
HeadingPrint (
 void)
{
	dsp_process ("Customer : ", cumr_rec.dbt_no);
	if (firstFlag)
		pr_format (fin, fout, "HEAD11", 0, 0);

	firstFlag = 1;
	
	if (!strcmp (batch, "ALL  "))
	{
		pr_format (fin, fout, "HEADER", 1, "Batch");
		pr_format (fin, fout, "HEADER", 2, cohr_rec.batch_no);
	}
	else
	{
		pr_format (fin, fout, "HEADER", 1, "     ");
		pr_format (fin, fout, "HEADER", 2, "     ");
	}

	pr_format (fin, fout, "HEADER", 3, (INVOICE) ? "Invoice" : " C/Note");
	pr_format (fin, fout, "HEADER", 4, cohr_rec.inv_no);
	pr_format (fin, fout, "HEADER", 5, PrintBatch);
	pr_format (fin, fout, "HEADER", 6, cumr_rec.dbt_no);
	pr_format (fin, fout, "HEADER", 7, cumr_rec.dbt_name);
	pr_format (fin, fout, "HEADER", 8, DateToString (cohr_rec.date_raised));
}

void
EndReport (
	void)
{
	percentCost 	= grandLines [4];
	percentSale 	= DOLLARS (grandLines [6]);
	percentActual 	= -99.99;
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

void
InvoiceTotal (
 void)
{
	double	gst_freight		= 0.00,
			b_extend 		= 0.00,
			a_extend 		= 0.00,
			freight_other 	= 0.00;

	freight_other = (cohr2_rec.freight + 
			  		 cohr2_rec.insurance + 
			  		 cohr2_rec.other_cost_1 + 
			  		 cohr2_rec.other_cost_2 + 
			  		 cohr2_rec.other_cost_3 +
		          	 cohr2_rec.sos);

	if (freight_other != 0.00)
	{
		/*-----------------------------------------------
		| Check if tax can be calculated or tax exempt. |
		-----------------------------------------------*/
		if (comm_rec.gst_rate == 0.00 || NOTAX)
			gst_freight = 0.00;
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

	pr_format (fin, fout, "TOT_LINE1", 1, (INVOICE) ? "INVOICE TOTAL "
							 : "CREDIT TOTAL. ");

	if (envDbNettUsed)
		percentSale = DOLLARS ((cohr2_rec.gross + cohr2_rec.tax + cohr2_rec.item_levy) - cohr2_rec.disc);
	else
		percentSale = DOLLARS (cohr2_rec.gross + cohr2_rec.tax + cohr2_rec.item_levy);
	
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
		b_extend	=	cohr2_rec.gross + 
						cohr2_rec.tax + 
						cohr2_rec.item_levy + 
						freight_other - 
		  	   			(cohr2_rec.disc + cohr2_rec.ex_disc);

		a_extend 	= 	cohr2_rec.gross + 
						cohr2_rec.tax + 
						cohr2_rec.gst + 
						cohr2_rec.item_levy + 
						freight_other - 
						(cohr2_rec.disc + cohr2_rec.ex_disc);
	}
	else
	{
		b_extend	=	cohr2_rec.item_levy + 
						cohr2_rec.gross + 
						cohr2_rec.tax +
						freight_other - 
						cohr2_rec.ex_disc;

		a_extend	=	cohr2_rec.item_levy + 
						cohr2_rec.gross + 
						cohr2_rec.tax + 
						cohr2_rec.gst + 
						freight_other - 
						cohr2_rec.ex_disc;
	}

	if (INVOICE)
	{
		pr_format (fin, fout, "TOT_LINE1", 2, totalLines [0]);
		pr_format (fin, fout, "TOT_LINE1", 3, totalLines [1]);
		pr_format (fin, fout, "TOT_LINE1", 4, totalLines [2]);
		pr_format (fin, fout, "TOT_LINE1", 5, cohr2_rec.gross);
		pr_format (fin, fout, "TOT_LINE1", 6, totalLines [3]);
		pr_format (fin, fout, "TOT_LINE1", 7, cohr2_rec.disc + cohr2_rec.ex_disc);
		pr_format (fin, fout, "TOT_LINE1", 8, b_extend);
		pr_format (fin, fout, "TOT_LINE1", 9, cohr2_rec.gst);
		pr_format (fin, fout, "TOT_LINE1", 10, a_extend);
		pr_format (fin, fout, "TOT_LINE1", 11, percentActual);
		grandLines [0] += totalLines [0];
		grandLines [1] += totalLines [1];
		grandLines [2] += totalLines [2];
		grandLines [3] += cohr2_rec.gross;
		grandLines [4] += totalLines [3];
		grandLines [5] += cohr2_rec.disc + cohr2_rec.ex_disc;
		grandLines [6] += b_extend;
		grandLines [7] += cohr2_rec.gst;
		grandLines [8] += a_extend;
	}
	else
	{
		pr_format (fin, fout, "TOT_LINE1", 2, totalLines [0]);
		pr_format (fin, fout, "TOT_LINE1", 3, totalLines [1]);
		pr_format (fin, fout, "TOT_LINE1", 4, totalLines [2]);
		pr_format (fin, fout, "TOT_LINE1", 5, cohr2_rec.gross * -1);
		pr_format (fin, fout, "TOT_LINE1", 6, totalLines [3]);
		pr_format (fin, fout, "TOT_LINE1", 7, cohr2_rec.disc * -1);
		pr_format (fin, fout, "TOT_LINE1", 8, b_extend * -1);
		pr_format (fin, fout, "TOT_LINE1", 9, cohr2_rec.gst * -1);
		pr_format (fin, fout, "TOT_LINE1", 10, a_extend * -1);
		pr_format (fin, fout, "TOT_LINE1", 11, percentActual);
		grandLines [0] += totalLines [0];
		grandLines [1] += totalLines [1];
		grandLines [2] += totalLines [2];
		grandLines [3] -= cohr2_rec.gross;
		grandLines [4] += totalLines [3];
		grandLines [5] -= cohr2_rec.disc;
		grandLines [6] -= b_extend;
		grandLines [7] -= cohr2_rec.gst;
		grandLines [8] -= a_extend;
	}
	totalLines [0] = 0.00;
	totalLines [1] = 0.00;
	totalLines [2] = 0.00;
	totalLines [3] = 0.00;
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
			extendCost 	= 0.00, 
			extendSale 	= 0.00,
			itemLevy	= 0.00;

	int		badCost 	= 0;

	cc = FindInmr2 (hhbrHash);
	if (cc)
		return (cc);

	cc = FindIncc ();
	if (cc)
		return (cc);

	if (envDbNettUsed)
	{
		extendPrice = ((coln2_rec.gross + coln2_rec.amt_tax) - coln2_rec.amt_disc);
	}
	else
		extendPrice = coln2_rec.gross + coln2_rec.amt_tax;

	extendPrice = no_dec (extendPrice);

	if (CREDIT)
	{
		extendPrice *= -1 ;
		incc_rec.closing_stock += coln_rec.q_order;
	}

	if (coln_rec.cost_price == 0.00 || SERIAL)
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
			{
				cost	=	FindInsfCost
							(
								0L,
								incc_rec.hhbr_hash,
								coln_rec.serial_no,
								"C"
							);
			}
			if (cost == -1.00)
			{
				cost	=	FindInsfCost
							(
								0L,
								incc_rec.hhbr_hash,
								coln_rec.serial_no,
								"F"
							);
			}
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

	if (CREDIT && (FAULTY || DOLLAR))
		cost = 0.00;

	if (cost < 0.00)
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

	extendSale 	= out_cost (coln2_rec.sale_price, inmr_rec.outer_size);
	extendCost 	= out_cost (cost, inmr_rec.outer_size);
	itemLevy 	= out_cost (cost, inmr_rec.outer_size);

	pr_format (fin, fout, "LINE1", 1, inmr_rec.item_no);
	pr_format (fin, fout, "LINE1", 2, coln_rec.item_desc);
	pr_format (fin, fout, "LINE1", 3, coln_rec.q_order);
	pr_format (fin, fout, "LINE1", 4, coln_rec.q_backorder);
	pr_format (fin, fout, "LINE1", 5, extendSale);
	pr_format (fin, fout, "LINE1", 6, coln2_rec.gross);
	pr_format (fin, fout, "LINE1", 7, extendCost * coln_rec.q_order);
	pr_format (fin, fout, "LINE1", 8, coln_rec.disc_pc);
	pr_format (fin, fout, "LINE1", 9, coln2_rec.amt_disc);

	if (envDbNettUsed)
	{
		pr_format (fin, fout, "LINE1", 10, (coln2_rec.gross + 
											itemLevy + 
				          	  				coln2_rec.amt_tax) - 
				          	  				coln2_rec.amt_disc);
		pr_format (fin, fout, "LINE1", 11, coln2_rec.amt_gst);
		pr_format (fin, fout, "LINE1", 12, (coln2_rec.gross + 
											itemLevy + 
				          	  				coln2_rec.amt_tax + 
				          	  				coln2_rec.amt_gst) - 
				          	  				coln2_rec.amt_disc);
	}
	else
	{
		pr_format (fin, fout, "LINE1", 10, (coln2_rec.gross + 
											itemLevy + 
				          	  				coln2_rec.amt_tax));
		pr_format (fin, fout, "LINE1", 11, coln2_rec.amt_gst);
		pr_format (fin, fout, "LINE1", 12, (coln2_rec.gross + 
											itemLevy + 
				          	  				coln2_rec.amt_tax + 
				          	  				coln2_rec.amt_gst));
	}
	percentCost	=	out_cost (cost, inmr_rec.outer_size);
	percentCost *= 	coln_rec.q_order;

	if (envDbNettUsed)
		percentSale = DOLLARS (coln2_rec.gross - coln2_rec.amt_disc);
	else
		percentSale = DOLLARS (coln2_rec.gross);
	percentActual = -99.99;
	if (CREDIT)
	{
		percentCost *= -1.00;
		percentSale *= -1.00;
	}

	if (percentCost == 0)
	{
		if (percentCost == percentSale)
			percentActual = 0.0;
		else
			percentActual = 100.0;
	}
	else
	{
		if (percentCost == percentSale)
			percentActual = 0.0;
		else
			if (percentCost != percentSale && percentSale != 0)
				percentActual = ((percentSale - percentCost) * 100.0/ percentSale);
	}
	pr_format (fin, fout, "LINE1", 13, percentActual);

	if (badCost)
		sprintf (err_str, "%cNCF", supChar [supFlag]);
	else
		sprintf (err_str, "%c   ", supChar [supFlag]);

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

/*====================================================
| Take out Gst from all the header lines of invoice. |
====================================================*/
void
SplitGst (
 int header)
{
    if (header)
    {
		cohr2_rec.gst = 0.00;

		cohr2_rec.gross		= OutGst (cohr_rec.gross,     HEAD, POS, TRUE);
		cohr2_rec.freight   = OutGst (cohr_rec.freight,   HEAD, POS, TRUE);
		cohr2_rec.insurance = OutGst (cohr_rec.insurance, HEAD, POS, TRUE);
		cohr2_rec.other_cost_1 = OutGst (cohr_rec.other_cost_1, HEAD,POS,TRUE);
		cohr2_rec.other_cost_2 = OutGst (cohr_rec.other_cost_2, HEAD,POS,TRUE);
		cohr2_rec.other_cost_3 = OutGst (cohr_rec.other_cost_3, HEAD,POS,TRUE);
		cohr2_rec.disc      = OutGst (cohr_rec.disc,      HEAD, NEG, TRUE);
		cohr2_rec.ex_disc   = OutGst (cohr_rec.ex_disc,   HEAD, NEG, TRUE);
		cohr2_rec.erate_var = OutGst (cohr_rec.erate_var, HEAD, POS, TRUE);
		cohr2_rec.sos       = OutGst (cohr_rec.sos,       HEAD, POS, TRUE);
		cohr2_rec.tax       = cohr_rec.tax;
    }
    else
    {
		coln2_rec.amt_gst = 0.00;

		coln2_rec.gross      = OutGst (coln_rec.gross,     LINE, POS, TRUE);
		coln2_rec.amt_disc   = OutGst (coln_rec.amt_disc,  LINE, NEG, TRUE);
		coln2_rec.amt_tax    = OutGst (coln_rec.amt_tax,   LINE, POS, TRUE);
		coln2_rec.sale_price = OutGst (coln_rec.sale_price,LINE, POS, FALSE);
		coln2_rec.tax_pc     = coln_rec.tax_pc;
		coln2_rec.gst_pc     = gstPercent;
	}
}

/*==============
| Extract Gst. |
==============*/
double	
OutGst (
	double	totalAmount, 
	int 	header, 
	int 	pos, 
	int 	add_gst)
{
	double	gstAmount = 0.00;

	if (totalAmount == 0)
		return (0.00);

	gstAmount = no_dec (totalAmount / gstDivide);
	
	totalAmount -= no_dec (gstAmount);
	
	if (!add_gst)
		return (totalAmount);
		
	if (header)
	{
		if (pos)
			cohr2_rec.gst += no_dec (gstAmount);
		else
			cohr2_rec.gst -= no_dec (gstAmount);
	}
	else
	{
		if (pos)
			coln2_rec.amt_gst += no_dec (gstAmount);
		else
			coln2_rec.amt_gst -= no_dec (gstAmount);
	}
	return (totalAmount);
}

void
NormalGst (
	int	header)
{
	if (header)
	{
		cohr2_rec.gross     = cohr_rec.gross;
		cohr2_rec.freight   = cohr_rec.freight;
		cohr2_rec.insurance = cohr_rec.insurance;
		cohr2_rec.other_cost_1  = cohr_rec.other_cost_1;
		cohr2_rec.other_cost_2  = cohr_rec.other_cost_2;
		cohr2_rec.other_cost_2  = cohr_rec.other_cost_3;
		cohr2_rec.tax       = cohr_rec.tax;
		cohr2_rec.gst       = cohr_rec.gst;
		cohr2_rec.disc      = cohr_rec.disc;
		cohr2_rec.deposit   = cohr_rec.deposit;
		cohr2_rec.ex_disc   = cohr_rec.ex_disc;
		cohr2_rec.erate_var = cohr_rec.erate_var;
		cohr2_rec.sos       = cohr_rec.sos;
	}
	else
	{
		coln2_rec.tax_pc     = coln_rec.tax_pc;
		coln2_rec.gst_pc     = coln_rec.gst_pc;
		coln2_rec.gross      = coln_rec.gross;
		coln2_rec.amt_disc   = coln_rec.amt_disc;
		coln2_rec.amt_tax    = coln_rec.amt_tax;
		coln2_rec.amt_gst    = coln_rec.amt_gst;
		coln2_rec.sale_price = coln_rec.sale_price;
	}
}

/*========================================
| Process cohr as per normal co/br/type. |
========================================*/
void
ProcessNormal (void)
{
	abc_selfield (cohr, "cohr_id_no2");

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, "I");
	strcpy (cohr_rec.inv_no, "        ");

	cc = find_rec (cohr, &cohr_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cohr_rec.br_no, comm_rec.est_no) &&
	       INVOICE)
	{
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");

		localExchangeRate = (envDbMcurr) ? cohr_rec.exch_rate : 1.00;

		/*---------------------------------------------------------
		| Check if order has correct stat flag.                   |
		---------------------------------------------------------*/
		if (beforeFlag && AFTER_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}

		if (!beforeFlag && !BEFORE_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
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

		/*------------------------------------------
		| Valid stat flag so update to stat of 'D' |
		------------------------------------------*/
		if (FLAG_DELETE)
		{
		   	strcpy (cohr_rec.stat_flag, "D");
		   	cc = abc_update (cohr, &cohr_rec);
		   	if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}

		abc_unlock (cohr);
		cc = find_rec (cohr, &cohr_rec, NEXT, "u");
	}
	abc_unlock (cohr);

	strcpy (cohr_rec.co_no,  comm_rec.co_no);
	strcpy (cohr_rec.br_no,  comm_rec.est_no);
	strcpy (cohr_rec.type,   "C");
	strcpy (cohr_rec.inv_no, "        ");

	cc = find_rec (cohr, &cohr_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cohr_rec.br_no, comm_rec.est_no) &&
	       CREDIT)
	{
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");

		localExchangeRate = (envDbMcurr) ? cohr_rec.exch_rate : 1.00;

		/*---------------------------------------------------------
		| Check if order has correct stat flag.                   |
		---------------------------------------------------------*/
		if (beforeFlag && AFTER_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}

		if (!beforeFlag && !BEFORE_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
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

		/*------------------------------------------
		| Valid stat flag so update to stat of 'D' |
		------------------------------------------*/
		if (FLAG_DELETE)
		{
		   	strcpy (cohr_rec.stat_flag, "D");
		   	cc = abc_update (cohr, &cohr_rec);
		   	if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}

		abc_unlock (cohr);
		cc = find_rec (cohr, &cohr_rec, NEXT, "u");
	}
	abc_unlock (cohr);
}

/*===============================
| Process cohr by batch number. |
===============================*/
void
ProcessBatch (void)
{
	abc_selfield (cohr, "cohr_batch_no");

	strcpy (cohr_rec.batch_no, batch);

	cc = find_rec (cohr, &cohr_rec, GTEQ, "u");

	while (!cc && !strcmp (cohr_rec.batch_no, batch))
	{
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");

		localExchangeRate = (envDbMcurr) ? cohr_rec.exch_rate : 1.00;

		if (!INVOICE && !CREDIT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}

		if (strcmp (cohr_rec.co_no, comm_rec.co_no) ||
	             strcmp (cohr_rec.br_no, comm_rec.est_no))
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}
		       
		/*---------------------------------------------------------
		| Check if order has correct stat flag.                   |
		---------------------------------------------------------*/
		if (beforeFlag && AFTER_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
		}

		if (!beforeFlag && !AFTER_STAT)
		{
			abc_unlock (cohr);
			cc = find_rec (cohr, &cohr_rec, NEXT, "u");
			continue;
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

		/*------------------------------------------
		| Valid stat flag so update to stat of 'D' |
		------------------------------------------*/
		if (FLAG_DELETE)
		{
		   	strcpy (cohr_rec.stat_flag, "D");
		   	cc = abc_update (cohr, &cohr_rec);
		   	if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}

		abc_unlock (cohr);
		cc = find_rec (cohr, &cohr_rec, NEXT, "u");
	}
	abc_unlock (cohr);
}

void
ToLocal (
	int		header)
{
	if (header)
	{
		cohr2_rec.gross     	/= localExchangeRate;
		cohr2_rec.freight   	/= localExchangeRate;
		cohr2_rec.insurance 	/= localExchangeRate;
		cohr2_rec.other_cost_1 	/= localExchangeRate;
		cohr2_rec.other_cost_2 	/= localExchangeRate;
		cohr2_rec.other_cost_3 	/= localExchangeRate;
		cohr2_rec.disc      	/= localExchangeRate;
		cohr2_rec.ex_disc   	/= localExchangeRate;
		cohr2_rec.sos       	/= localExchangeRate;
		cohr2_rec.tax       	/= localExchangeRate;
		cohr2_rec.gst       	/= localExchangeRate;
	}
	else
	{
		coln2_rec.gross      	/= localExchangeRate;
		coln2_rec.amt_disc   	/= localExchangeRate;
		coln2_rec.amt_tax    	/= localExchangeRate;
		coln2_rec.amt_gst    	/= localExchangeRate;
		coln2_rec.sale_price 	/= localExchangeRate;
	}
}
