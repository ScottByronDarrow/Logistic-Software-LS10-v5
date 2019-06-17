/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (so_16wkprt.c  )                                 |
|  Program Desc  : (Suppliers 16 week projection.               )   |
|                  (                                            )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sohr, soln, inmr, cumr,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sohr, soln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (24/10/88)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (15/11/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (24/02/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (11/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (19/01/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (17/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : Modified to allow Forward and backorders.          |
|                : Tidy up program.                                   |
|                : (24/02/89) - Added Sales Tax and All order Option. |
|                : (11/09/90) - General Update for New Scrgen. S.B.D. |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (19/01/93)    : PSL 6828 changes for multi-curr sales order.       |
|                : plus tidy up.                                      |
|  (17/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_16wkprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_16wkprt/so_16wkprt.c,v 5.4 2001/10/23 07:16:27 scott Exp $";

#define	 NO_SCRGEN	
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_so_mess.h>

#define	FORWARD_ORDERS	(reportType [0] == 'F')
#define	BACKORDERS		(reportType [0] == 'B')
#define	MANUAL_ORDER	(reportType [0] == 'M')
#define	HELD_ORDERS		(reportType [0] == 'H' || \
						 reportType [0] == 'O' || \
					 	 reportType [0] == 'C')
#define	ALL_ORDERS		(reportType [0] == ' ')

#define	FWD_OK			 (soln_rec.status [0] == 'F')
#define	BACK_OK			 (soln_rec.status [0] == 'B')
#define	MAN_OK			 (soln_rec.status [0] == 'M')
#define	HELD_OK			 (soln_rec.status [0] == 'H' || \
						  soln_rec.status [0] == 'O' || \
						  soln_rec.status [0] == 'C')

#define	NOHEADLINES	4

#define	EXCH_RATE	 ((envDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00) 

FILE	*pout;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

	char	*data	= "data";

	int		envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE,
			envVarCnNettUsed 	= TRUE,
			envDbMcurr		=	0;

	int		printertNumber	=	1;

	long	startDate	=	0L,
			lastcumr	=	0L,
			lastHash	=	0L,
			weekStart	=	0L,
			weekEnd		=	0L;

	double	grandTotalQty 	= 0.0,
			grandTotalAmt 	= 0.0,
			weekTotalQty	= 0.0,
			weekTotalAmt	= 0.0;

	char	reportType [2];

	char	*head [NOHEADLINES] = {
"===============================================================================",
"|   DATE   |     ITEM       |   QTY.  | LCL AMNT | CUSTOMER |  S.O.  | STATUS |",
"|   DUE    |    NUMBER      |         |OF SHIPMNT| ACRONYM  | NUMBER |        |",
"|----------+----------------+---------+----------+----------+--------+--------|"
	};

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	procfile 		(void);
int 	ValidateOrder 	(void);
void 	ProcessSoln 	(void);
int 	heading 		(void);
void 	EndReport 		(void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	orderDescription [30];
	char	*sptr = chk_env ("DB_MCURR");

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *)0) ? envVarDbNettUsed : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc < 4) 
	{
		/*-----------------------------------------------------
		| Usage : %s <lpno> <startDate> < [B,F,H,O,M] or  [ ]> |
		-----------------------------------------------------*/
		print_at (0,0, mlSoMess732,argv [0]);
		return (EXIT_FAILURE);
	}

	if (argv [3][0] != 'B' && argv [3][0] != 'F' && argv [3][0] != 'H' &&
	    argv [3][0] != 'O' && argv [3][0] != 'M' && argv [3][0] != ' ')
	{
		print_at (0,0, mlSoMess732,argv [0]);
		return (EXIT_FAILURE);
	}

	printertNumber = atoi (argv [1]);
	startDate = StringToDate (argv [2]);
	sprintf (reportType, "%-1.1s", argv [3]);

	OpenDB ();


	if (FORWARD_ORDERS)
		strcpy (orderDescription,"(Forward Orders)");

	if (BACKORDERS)
		strcpy (orderDescription, "(Backorders)");

	if (MANUAL_ORDER)
		strcpy (orderDescription, "(New/Unconsolidated Orders)");

	if (HELD_ORDERS)
		strcpy (orderDescription,"(Held Orders)");

	if (ALL_ORDERS)
		strcpy (orderDescription, "(ALL Orders)");

	sprintf (err_str,"Processing 16 week projection. %s", orderDescription);

	dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

    if (heading () != 0) {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	procfile ();

	EndReport ();
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_dbclose (data);
}

/*==========================================================================
| Process whole soln file for each 1 week period for all 16 weeks.         |
| Printing any records which are fall into the week start and end dates.   |
| Returns: 0 if ok,non-zero if not ok.                                    |
==========================================================================*/
int
procfile (
 void)
{
	int	i;
	
	lastcumr = 0L;

	/*-----------
	| 16 weeks. |
	-----------*/
	for (i = 1; i < 17; i++) 
	{	
		weekStart = startDate + (7 * (i - 1));
		weekEnd = weekStart + 6;
		weekTotalQty = 0.00;
		weekTotalAmt = 0.0;
		fprintf (pout, "| WEEK %2d  |", i);
		fprintf (pout, "ENDING %-10.10s", DateToString (weekEnd));
		fprintf (pout, "         |");
		fprintf (pout, "          |");
		fprintf (pout, "          |");
		fprintf (pout, "        |");
		fprintf (pout, "        |\n");

		lastHash = 0L;
		/*----------------------- 
		| Read whole soln file. |
		-----------------------*/
		soln_rec.hhso_hash = 0L;
		soln_rec.line_no = 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc)
		{
			if (!ValidateOrder ())
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
			if (soln_rec.hhso_hash != lastHash) 
			{
				cc = find_hash (sohr, &sohr_rec, COMPARISON, "r", soln_rec.hhso_hash);
				if (cc) 
				{
					cc = find_rec (soln,&soln_rec, NEXT,"r");
					continue;
				}

				lastHash = soln_rec.hhso_hash;
			}

			if (strcmp (sohr_rec.co_no,comm_rec.co_no))
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			if (soln_rec.due_date < weekStart || soln_rec.due_date > weekEnd)
			{
				cc = find_rec (soln ,&soln_rec,NEXT,"r");
				continue;
			}

			ProcessSoln ();

			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}

		fprintf (pout, "|          |");
		fprintf (pout, "TOTAL THIS WEEK |");
		fprintf (pout, "%9.2f|", weekTotalQty);
		fprintf (pout, "%10.2f|", DOLLARS (weekTotalAmt));
		fprintf (pout, "          |");
		fprintf (pout, "        |");
		fprintf (pout, "        |\n");
		fprintf (pout, "%s\n", head [3]);
		grandTotalQty += weekTotalQty;
		grandTotalAmt += weekTotalAmt;
	}	/* End of for ... loop	*/
	return (EXIT_SUCCESS);
}

/*=======================
| Validate order types. |
=======================*/
int
ValidateOrder (
 void)
{
	if (ALL_ORDERS)
		return (TRUE);

	if (FORWARD_ORDERS && FWD_OK)
		return (TRUE);

	if (BACKORDERS && BACK_OK)
		return (TRUE);

	if (MANUAL_ORDER && MAN_OK)
		return (TRUE);

	if (HELD_ORDERS && HELD_OK)
		return (TRUE);

	return (FALSE);
}

/*======================
| Process Order lines. |
======================*/
void
ProcessSoln (
 void)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;

	double	quantity = 0.00,
			balance	 = 0.00;

	/*------------------------
	| Change of customer no. |
	------------------------*/
	if (lastcumr != sohr_rec.hhcu_hash) 
	{
		if (find_hash (cumr, &cumr_rec,EQUAL, "r",sohr_rec.hhcu_hash))
		{
			strcpy (cumr_rec.dbt_no, "000000");
			strcpy (cumr_rec.dbt_acronym, "UNKNOWN");
		}
		else 
			lastcumr = sohr_rec.hhcu_hash;
	}

	/*------------------
	| Get part number. |
	------------------*/
	cc = find_hash (inmr, &inmr_rec, EQUAL, "r", soln_rec.hhbr_hash);
	if (cc)
		strcpy (inmr_rec.item_no, "Unknown part no.");

	dsp_process ("Item : ",inmr_rec.item_no);

	quantity = (double) (soln_rec.qty_order + soln_rec.qty_bord);

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	 =	quantity;
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
			balance	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			balance	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		balance = no_dec (balance / EXCH_RATE);
	}
	fprintf (pout, "|%-10.10s|",      DateToString (soln_rec.due_date));
	fprintf (pout, "%-16.16s|",       inmr_rec.item_no);
	fprintf (pout, "%9.2f|",          quantity);
	fprintf (pout, "%10.2f|",         DOLLARS (balance));
	fprintf (pout, " %-9.9s|",        cumr_rec.dbt_acronym);
	fprintf (pout, "%-8.8s|",         sohr_rec.order_no);
	fprintf (pout, "   %1.1s    |\n", soln_rec.status);

	weekTotalQty += quantity;
	weekTotalAmt += balance;
}

/*===================================================================
| Routine to open the pipe to standard print and send initial data. |
===================================================================*/
int
heading (
 void)
{
	int	i;

	if ((pout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n",DateToString (comm_rec.dbt_date),PNAME);
	fprintf (pout, ".LP%d\n",printertNumber);
	fprintf (pout, ".11\n");
	fprintf (pout, ".L120\n");
	fprintf (pout, ".B1\n");

	if (FORWARD_ORDERS)
		fprintf (pout,".ECUSTOMER S/O 16 WEEK PROJECTION (FORWARD ORDERS)\n");
	if (BACKORDERS)
		fprintf (pout,".ECUSTOMER S/O 16 WEEK PROJECTION (BACKORDER)\n");

	if (MANUAL_ORDER)
		fprintf (pout,".ECUSTOMER S/O 16 WEEK PROJECTION (NEW/UNCONSOLIDATED ORDERS)\n");
	if (HELD_ORDERS)
		fprintf (pout,".ECUSTOMER S/O 16 WEEK PROJECTION (HELD ORDERS)\n");
	if (ALL_ORDERS)
		fprintf (pout,".ECUSTOMER S/O 16 WEEK PROJECTION (ALL ORDERS)\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".E%s AS AT %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".R%s\n",head [0]);
	for (i = 0; i < NOHEADLINES;i++) 
		fprintf (pout,"%s\n",head [i]);
	
	return (EXIT_SUCCESS);
}

/*===================================================
| Routine to end report. Prints bottom line totals. |
===================================================*/
void
EndReport (
 void)
{
	fprintf (pout, "|          |");
	fprintf (pout, "%-16.16s|", "TOTAL ORDERS");
	fprintf (pout, "%9.2f|", grandTotalQty);
	fprintf (pout, "%10.2f|", DOLLARS (grandTotalAmt));
	fprintf (pout, "          |");
	fprintf (pout, "        |");
	fprintf (pout, "        |\n");
	fprintf (pout, ".EOF\n");
	pclose (pout);
}
