/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_balwin.c,v 5.2 2001/11/19 01:52:20 scott Exp $
|  Program Name  : (sk_db_balwin.c)                                 |
|  Program Desc  : (Display Customer Credit status)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_balwin.c,v $
| Revision 5.2  2001/11/19 01:52:20  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_balwin.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_balwin/db_balwin.c,v 5.2 2001/11/19 01:52:20 scott Exp $";

#include	<pslscr.h>
#include	<ml_db_mess.h>

	/*
	 * The Following are needed for branding Routines.
	 */
	int		currentMonth	=	0;

	double	mtdSales 		= 0.00, 
			ytdSales 		= 0.00,
			cumr_bo [6]		= {0.00,0.00,0.00,0.00,0.00,0.00};

	long	hhcuHash		= 0L;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cusaRecord	cusa_rec;

	Money	*cusa_val		=	&cusa_rec.val1;
	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

	char	workString [131];

	char	*cumr2	=	"cumr2";


/*
 * Local Functino Prototypes.
 */
void 	ProcessFile 		(long);
void 	SaveDisplayData 	(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	GetMtdYtd 			(int, long);
float 	CalculateYtd 		(void);

/*
 * Main processing routine.
 */
int
main (
 int                argc, 
 char*              argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, mlDbMess137, argv [0]);
        return (EXIT_FAILURE);
	}

	hhcuHash = atol (argv [1]);

	OpenDB ();

	init_scr 	();
	crsr_off 	();
	set_tty 	();

	ProcessFile (hhcuHash);

	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

void
ProcessFile (
	long	hhcuHash)
{
	int		i;

	Dsp_open (1, 4, 7);

	cumr_rec.hhcu_hash	=	hhcuHash;
	cc =  find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (!cc)
	{
		Dsp_saverec ("                  CUSTOMER CREDIT STATUS WINDOW DISPLAY                 ");
		Dsp_saverec (" ");
		Dsp_saverec ("");

		/*
		 * Total head office customer.
		 */
		for (i = 0; i < 6; i++)
			cumr_bo [i] = cumr_balance [i];

		/*
		 * Get MTD and YTD for head office customer.
		 */
		GetMtdYtd (TRUE,  cumr_rec.hhcu_hash);

		/*
		 * Total all child customer.
		 */
		cumr2_rec.ho_dbt_hash = cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				cumr_bo [i] += cumr2_balance [i];

			/*
			 * Get MTD and YTD for child customer.
			 */
			GetMtdYtd (FALSE,  cumr2_rec.hhcu_hash);
			
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
	    SaveDisplayData ();
	}
	Dsp_srch ();
	Dsp_close ();
}

void
SaveDisplayData (void)
{
	char	sp_credit [4];
	double	cur_bal = 0.00, 
			out_bal = 0.00;


	cur_bal  = cumr_bo [0] + cumr_bo [5];
	out_bal  = cumr_bo [1] + cumr_bo [2] + 
		   	   cumr_bo [3] + cumr_bo [4];

	strcpy (sp_credit, (cumr_rec.stop_credit [0] == 'Y') ? ML ("YES") : ML ("NO."));
	sprintf (workString, "%11.11s : %6.6s          / %40.40s", 
		ML ("Customer No"), cumr_rec.dbt_no, cumr_rec.dbt_name);
	Dsp_saverec (workString);

	sprintf (workString, "%11.11s : %10.2f      / %s %12.2f", 
		ML ("Sales MTD  "), DOLLARS (mtdSales), 
		ML ("Sales YTD"),   DOLLARS (ytdSales));
	Dsp_saverec (workString);
	
	sprintf (workString, "%11.11s : %2.2s              / %s     :%2.2s", 
		ML ("Salesman   "), cumr_rec.sman_code, 
		ML ("Area    "),    cumr_rec.area_code);
	Dsp_saverec (workString);

	sprintf (workString, "%11.11s : A%1d              / %s  :%20.20s", 
		ML ("Credit Rate"), cumr_rec.payment_flag, 
		ML ("Credit Ref "), cumr_rec.credit_ref);
	Dsp_saverec (workString);
	
	sprintf (workString, "%11.11s : %3.3s             / %s : %12.2f", 
		ML ("Stop credit"), sp_credit, 
		ML ("Credit limit"), DOLLARS (cumr_rec.credit_limit));
	Dsp_saverec (workString);
	
	sprintf (workString, "%11.11s : %15.15s / %s : %20.20s ", 
		ML ("Phone No   "), cumr_rec.phone_no, 
		ML ("Contact Name"), cumr_rec.contact_name);
	Dsp_saverec (workString);
	
	sprintf (workString, "%11.11s : %10.2f      / %s : %12.2f", 
		ML ("Current Amt"), DOLLARS (cur_bal), 
		ML ("Overdue Amt "), DOLLARS (out_bal));
	Dsp_saverec (workString);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	
	abc_alias (cumr2, cumr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cusa);
	abc_dbclose ("data");
}


void
GetMtdYtd (
	int		clear_tot, 
	long	hhcuHash)
{
	if (clear_tot)
	{
		DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, NULL);
		currentMonth--;

		/*
		 * Load Year To Date Sales from incc
		 */
		mtdSales = 0.00;
		ytdSales = 0.00;
	}

	cusa_rec.hhcu_hash = hhcuHash;
	strcpy (cusa_rec.year, "C");
	cc = find_rec (cusa, &cusa_rec, COMPARISON, "r");
	if (!cc)
	{
		mtdSales += cusa_val [currentMonth];
		ytdSales += CalculateYtd ();
	}
}

/*
 * Calculate Ytd Sales.
 */
float
CalculateYtd (void)
{
	int		i;
	float	ytd = 0.00;

	/*
	 * no fiscal set up
	 */
	if (!comm_rec.fiscal)
	{
		/*
		 * sum to current month (feb == 1)
		 */
		for (i = 0;i <= currentMonth;i++)
			ytd += (float) cusa_val [i];

		return (ytd);
	}

	/*
	 * need to sum from fiscal to dec, then jan to current month.
	 */
	if (currentMonth < comm_rec.fiscal)
	{
		for (i = comm_rec.fiscal;i < 12;i++)
			ytd += (float) cusa_val [i];

		for (i = 0;i <= currentMonth;i++)
			ytd += (float) cusa_val [i];

		return (ytd);
	}

	for (i = comm_rec.fiscal;i <= currentMonth;i++)
		ytd += (float) cusa_val [i];

	return (ytd);
}
