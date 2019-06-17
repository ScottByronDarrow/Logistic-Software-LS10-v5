/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sal_log.c,v 5.3 2001/10/23 08:18:08 scott Exp $
|  Program Name  : (so_sal_log.c)
|  Program Desc  : (Print Sales Order Log)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
| $Log: so_sal_log.c,v $
| Revision 5.3  2001/10/23 08:18:08  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sal_log.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sal_log/so_sal_log.c,v 5.3 2001/10/23 08:18:08 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	SR              store [line_cnt];   

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		printerNo = 1,	/* Line printer number			*/
			envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE,
			envVarDbMcurr		= FALSE;

	long	startDate,
			endDate;

	double	sale_qty = 0.0,
			sale_amt = 0.0;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

	long	hhso_hash;
	int		SalesLog = TRUE;

	char	*head [5] = {
			"================================================================================================",
			"|   DATE   |     ITEM       |  UOM  |   QUAN.  |        AMOUNT        | CUSTOMER |  S.O.  |STAT|",
			"|   DUE    |    NUMBER      |       |          |      OF SHIPMNT      | ACRONYM  | NUMBER |    |",
			"|----------|----------------|-------|----------|----------------------|----------|--------|----|",
			"|   DATE   |     ITEM       |  UOM  |   QUAN.  |       LCL AMT        | CUSTOMER |  S.O.  |STAT|",
		};

FILE	*pout;

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessFile 	(void);
int  	ProcessSoln 	(long);
void 	heading 		(void);
void 	EndReport 		(void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
    init_scr ();

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strncmp (sptr, "so_sal_log", 10))
		SalesLog = TRUE;
	else
		SalesLog = FALSE;

	if (argc < 4 && SalesLog)
	{
		print_at (0,0,mlSoMess769,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNo 		= atoi (argv [1]);
	if (SalesLog)
	{
		startDate 	= StringToDate (argv [2]);
		endDate 	= StringToDate (argv [3]);
		hhso_hash = 0L;
	}
	else
		hhso_hash = atol (argv [2]);

	OpenDB ();


	dsp_screen (" Processing Sales Log.",comm_rec.co_no,comm_rec.co_name);
	heading ();

	ProcessFile ();

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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr,inmr_list,INMR_NO_FIELDS,"inmr_hhbr_hash");
	open_rec (cumr,cumr_list,CUMR_NO_FIELDS,"cumr_hhcu_hash");
	open_rec (sohr,sohr_list,SOHR_NO_FIELDS, (SalesLog) 
						? "sohr_id_no" : "sohr_hhso_hash");
	open_rec (soln,soln_list,SOLN_NO_FIELDS,"soln_id_no");
		
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

	abc_dbclose ("data");
}

/*=====================================================================
| Process whole sohr file for company. printing any records which are |
| later than startDate and less than or equal to endDate.           |
| Returns: 0 if ok,non-zero if not ok.                               |
=====================================================================*/
void
ProcessFile (void)
{
	if (SalesLog)
	{
		strcpy (sohr_rec.co_no,comm_rec.co_no);
		cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
		while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no))
		{
			cc = ProcessSoln (sohr_rec.hhso_hash);
			if (!cc)
				dsp_process ("Item : ",inmr_rec.item_no);

			cc = find_rec (sohr,&sohr_rec,NEXT,"r");
		}
	}
	else
	{
		sohr_rec.hhso_hash = hhso_hash;
		cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
		if (cc)
			return;

		cc = ProcessSoln (sohr_rec.hhso_hash);
		if (cc)
			dsp_process ("Sales Order : ",sohr_rec.order_no);
	}
}

int
ProcessSoln (
 long hhso_hash)
{
	double	wk_qty = 0.00;
	double	balance = 0.00;
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;
	char	cnv_qty [11];
	char	cnv_bal [23];

	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sohr_rec.hhcu_hash);
	if (cc)
	{
		strcpy (cumr_rec.dbt_no,"000000");
		strcpy (cumr_rec.dbt_acronym,"UNKNOWN");
	}

	soln_rec.hhso_hash = hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");

	inmr_rec.hhbr_hash = soln_rec.hhbr_hash;

	while (!cc && soln_rec.hhso_hash == hhso_hash)
	{
		if ((soln_rec.due_date >= startDate && soln_rec.due_date <= endDate) || !SalesLog)
		{
			cc = find_hash (inmr,&inmr_rec,COMPARISON,"r",soln_rec.hhbr_hash);
			if (cc) 
				strcpy (inmr_rec.item_no,"Unknown part no.");

            if (sohr_rec.exch_rate == 0.00)
            	sohr_rec.exch_rate = 1.00;

			wk_qty = soln_rec.qty_order + soln_rec.qty_bord;
			if (soln_rec.bonus_flag [0] != 'Y')
			{
				l_total	=	(double) wk_qty;
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
			
				balance = balance / sohr_rec.exch_rate;
			}

			sprintf (cnv_qty, "%10.10s", comma_fmt ((double) wk_qty,
										"NNN,NNN.NN"));
			sprintf (cnv_bal, "%22.22s", comma_fmt (DOLLARS (balance),
										"NNN,NNN,NNN,NNN,NNN.NN"));

			fprintf (pout,"|%10.10s|",DateToString (soln_rec.due_date));
			fprintf (pout,"%-16.16s|",inmr_rec.item_no);
			fprintf (pout,"  %-4.4s |",inmr_rec.sale_unit);
			fprintf (pout,"%10.10s|", cnv_qty);
			fprintf (pout,"%22.22s|", cnv_bal);
			fprintf (pout," %-9.9s|",cumr_rec.dbt_acronym);
			fprintf (pout,"%-8.8s|",sohr_rec.order_no);
			fprintf (pout," %s  |\n",soln_rec.status);

			sale_qty += wk_qty;
			sale_amt += balance; 
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
    return (EXIT_SUCCESS);
}

void
heading (
 void)
{
	char	StartDate [11],
			EndDate [11];

	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	strcpy (StartDate, DateToString (startDate));
	strcpy (EndDate,   DateToString (endDate));

	fprintf (pout,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pout,".LP%d\n",printerNo);
	fprintf (pout,".%d\n", (SalesLog) ? 11 : 10);
	fprintf (pout,".L97\n");
	fprintf (pout,".EORDERS LOG\n");
	fprintf (pout,".E%s \n",clip (comm_rec.co_name));
	fprintf (pout,".EAS AT %s\n", SystemTime ());
	if (SalesLog)
		fprintf (pout,".EFROM: %s to %s\n",StartDate,EndDate);
	fprintf (pout,".B1\n");
	fprintf (pout,".R%s\n",head [0]);
	fprintf (pout,"%s\n",head [0]);
	if (envVarDbMcurr)
		fprintf (pout,"%s\n",head [4]);
	else
		fprintf (pout,"%s\n",head [1]);
	fprintf (pout,"%s\n",head [2]);
	fprintf (pout,"%s\n",head [3]);
}

void
EndReport (
 void)
{

	char	tot_qty [11];
	char	tot_bal [23];

	sprintf (tot_qty, "%10.10s", comma_fmt ((double) sale_qty,
								"NNN,NNN.NN"));
	sprintf (tot_bal, "%22.22s", comma_fmt (DOLLARS (sale_amt),
								"NNN,NNN,NNN,NNN,NNN.NN"));

	fprintf (pout,"|          |%-16.16s|       |%10.10s|%22.22s|          |        |    |\n.EOF\n","TOTAL ORDERS",tot_qty,tot_bal);
	pclose (pout);
}
