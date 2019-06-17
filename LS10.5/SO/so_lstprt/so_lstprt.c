/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_lstprt.c,v 5.2 2001/10/23 07:16:44 scott Exp $
|  Program Name  : (so_lstprt.c)
|  Program Desc  : (Late list Report)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 15/08/88         |
|---------------------------------------------------------------------|
| $Log: so_lstprt.c,v $
| Revision 5.2  2001/10/23 07:16:44  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_lstprt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lstprt/so_lstprt.c,v 5.2 2001/10/23 07:16:44 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_so_mess.h>

#define	FORWARD_ORDERS	 (status [0] == 'F')
#define	BACKORDERS		 (status [0] == 'B')
#define	MANUAL_ORDER	 (status [0] == 'M')
#define	HELD_ORDERS		 (status [0] == 'H' || status [0] == 'O')
#define	ALL_ORDERS		 (status [0] == ' ')

#define	FWD_OK			 (soln_rec.status [0] == 'F')
#define	BACK_OK			 (soln_rec.status [0] == 'B')
#define	MAN_OK			 (soln_rec.status [0] == 'M')
#define	HELD_OK			 (soln_rec.status [0] == 'H' || soln_rec.status [0] == 'O')
	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	int 	envVarDbMcurr,
			lpno 				= 1,	/* Line printer number			*/
			envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE;

	long	StartDate;

	double	late_qty = 0.0,
			late_amt = 0.0;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

	char	*head [5] = {
		"======================================================================================================================",
		"|   DATE   |     ITEM       |      DESCRIPTION                       | QUAN. |  AMOUNT  | CUSTOMER |  S.O.  | STATUS |",
		"|   DUE    |    NUMBER      |                                        |       |OF SHIPMNT| ACRONYM  | NUMBER |        |",
		"|----------|----------------|----------------------------------------|-------|----------|----------|--------|--------|",
		"|   DATE   |     ITEM       |      DESCRIPTION                       | QUAN. |LOCAL AMT | CUSTOMER |  S.O.  | STATUS |"
		};
	
	char	status [2];

FILE	*pout;

/*=======================
| Function Declarations | 
=======================*/
void shutdown_prog 	(void);
void OpenDB 		(void);
void CloseDB 		(void);
int  ProcessFile 	(void);
int  ProcessSoln 	(long);
int  ValidOrder 	(void);
int  RepHead 		(long);
void EndReport 		(void);


/*==========================
| Main Processing Routine. | 
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	order_desc [40];
	char	*sptr;

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc < 4) 
	{
		/*----------------------------------------------------
		| Usage : %s <lpno> <StartDate> < [B,F,H,O,M] or [ ]> |
		----------------------------------------------------*/
		print_at (0,0, mlSoMess732, argv [0]);
		return (EXIT_FAILURE);
	}

	lpno = atoi (argv [1]);
	StartDate = StringToDate (argv [2]);
	sprintf (status, "%-1.1s", argv [3]);

	OpenDB ();


	if (FORWARD_ORDERS)
		strcpy (order_desc," (Forward Orders)");

	if (BACKORDERS)
		strcpy (order_desc, " (Backorders)");

	if (MANUAL_ORDER)
		strcpy (order_desc, " (New/Unconsolidated Orders)");

	if (HELD_ORDERS)
		strcpy (order_desc," (Held Orders)");

	if (ALL_ORDERS)
		strcpy (order_desc, " (ALL Orders)");

	sprintf (err_str,"Processing Late List. %s", order_desc);

	dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

    /* this is dead code methinks. RepHead always returns 0 */
    if (RepHead (StartDate) != 0) {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
	
	cc = ProcessFile ();

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

	open_rec (inmr, inmr_list, INMR_NO_FIELDS,"inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,"cumr_hhcu_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS,"sohr_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS,"soln_id_no");
		
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

/*======================================================================
| Process whole sohr file for company. printing any records which are  |
| earlier than StartDate.                                             |
| Returns: 0 if ok,non-zero if not ok.                                |
======================================================================*/
int
ProcessFile (
 void)
{
	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no, "  ");
	strcpy (sohr_rec.order_no, "        ");
	sohr_rec.hhcu_hash = 0L;
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");

	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no))
	{
		cc = ProcessSoln (sohr_rec.hhso_hash);
		if (!cc) 
			dsp_process ("Item : ",inmr_rec.item_no);

		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}	
	return (EXIT_SUCCESS);
}

int
ProcessSoln (
	long	hhsoHash)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;

	double	wk_qty = 0.00,
			balance = 0.00;

	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sohr_rec.hhcu_hash);
	if (cc)
	{
		strcpy (cumr_rec.dbt_no,"000000");
		strcpy (cumr_rec.dbt_acronym,"UNKNOWN");
	}

	soln_rec.hhso_hash = hhsoHash;
	soln_rec.line_no = 0;

	cc = find_rec (soln,&soln_rec,GTEQ,"r");

	while (!cc && soln_rec.hhso_hash == hhsoHash) 
	{	
		if (!ValidOrder () || StartDate >= soln_rec.due_date)
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}
		/*------------------
		| Get part number. |
		------------------*/
		cc = find_hash (inmr,&inmr_rec,COMPARISON,"r",soln_rec.hhbr_hash);
		if (cc) 
		{
			sprintf (inmr_rec.description,"%-40.40s"," ");
			strcpy (inmr_rec.item_no,"Unknown part no.");
		}
	
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
		fprintf (pout, "|%10.10s",	DateToString (soln_rec.due_date));
		fprintf (pout, "|%-16.16s",	inmr_rec.item_no);
		fprintf (pout, "|%-40.40s",	inmr_rec.description);
		fprintf (pout, "|%7.2f",	wk_qty);
		fprintf (pout, "|%10.2f",	DOLLARS (balance));
		fprintf (pout, "| %-9.9s",	cumr_rec.dbt_acronym);
		fprintf (pout, "|%-8.8s",	sohr_rec.order_no);
		fprintf (pout, "|   %1.1s    |\n",soln_rec.status);

		late_qty += wk_qty;
		late_amt += balance;

		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	return (EXIT_SUCCESS);
}

/*=======================
| Validate order types. |
=======================*/
int
ValidOrder (void)
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
/*===================================================================
| Routine to open the pipe to standard print and send initial data. |
===================================================================*/
int
RepHead (
 long	StartDate)
{
	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN) ",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pout,".LP%d\n",lpno);
	fprintf (pout,".13\n");
	fprintf (pout,".L118\n");
	fprintf (pout,".B1\n");

	if (FORWARD_ORDERS)
		fprintf (pout,".ECUSTOMER ORDERS LATE LIST : (FORWARD ORDERS)\n");
	if (BACKORDERS)
		fprintf (pout,".ECUSTOMER ORDERS LATE LIST : (BACKORDER)\n");

	if (MANUAL_ORDER)
		fprintf (pout,".ECUSTOMER ORDERS LATE LIST : (NEW/UNCONSOLIDATED ORDERS)\n");
	if (HELD_ORDERS)
		fprintf (pout,".ECUSTOMER ORDERS LATE LIST : (HELD ORDERS)\n");

	if (ALL_ORDERS)
		fprintf (pout,".ECUSTOMER ORDERS LATE LIST : (ALL ORDERS)\n");

	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".ESTART DATE: %s\n",DateToString (StartDate));
	fprintf (pout,".B1\n");
	fprintf (pout,".R%s\n",head [0]);
	if (envVarDbMcurr)
	{
		fprintf (pout,"%s\n",head [0]);
		fprintf (pout,"%s\n",head [4]);
		fprintf (pout,"%s\n",head [2]);
		fprintf (pout,"%s\n",head [3]);
	}
	else
	{
		fprintf (pout,"%s\n",head [0]);
		fprintf (pout,"%s\n",head [1]);
		fprintf (pout,"%s\n",head [2]);
		fprintf (pout,"%s\n",head [3]);
	}
	fflush (pout);
	return (EXIT_SUCCESS);
}

/*===================================================
| Routine to end report. Prints bottom line totals. |
===================================================*/
void
EndReport (void)
{
	fprintf (pout,"|          |%-16.16s|%40.40s|%7.2f|%10.2f|          |        |        |\n.EOF\n","TOTAL LATE"," ",late_qty,DOLLARS (late_amt));
	pclose (pout);
}
