/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: bycusprt.c,v 5.4 2002/07/17 09:58:05 scott Exp $
|  Program Name  : (so_bycusprt.c) |
|  Program Desc  : (Print Sales Orders by Customer)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 17/10/88         |
|---------------------------------------------------------------------|
| $Log: bycusprt.c,v $
| Revision 5.4  2002/07/17 09:58:05  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/10/23 07:16:36  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.2  2001/08/09 09:20:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:59  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:19:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/03/23 01:57:09  scott
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
| Revision 4.2  2001/03/23 01:54:48  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bycusprt.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bycusprt/bycusprt.c,v 5.4 2002/07/17 09:58:05 scott Exp $";

#include <pslscr.h>

#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	PACKING_SLIPS	(status [0] == 'P')
#define	FORWARD_ORDERS	(status [0] == 'F')
#define	BACKORDERS		(status [0] == 'B')
#define	MANUAL_ORDER	(status [0] == 'M')
#define	HELD_ORDERS		(status [0] == 'H')
#define	ALL_ORDERS		(status [0] == ' ')
 
#define	PS_OK			(soln_rec.status [0] == 'P' || \
				  		 soln_rec.status [0] == 'S')
#define	FWD_OK			(soln_rec.status [0] == 'F')
#define	BACK_OK			(soln_rec.status [0] == 'B')
#define	MAN_OK			(soln_rec.status [0] == 'M')
#define	HELD_OK			(soln_rec.status [0] == 'H' || \
				  		 soln_rec.status [0] == 'O' || \
				  		 soln_rec.status [0] == 'C')

#include	"schema"

struct commRecord	comm_rec;
struct inexRecord	inex_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	int		envVarDbCo			= 0,
			envVarDbFind		= 0,
			envVarRepTax 		= 0,
			envVarDbMcurr		= 0,
			customerFlag 		= TRUE,
			firstSoln 			= TRUE,
			recordFound 		= FALSE,
			firstFlag 			= 0,
			salesOrderDetails 	= 0,
			afterSoLines 		= FALSE,
			envVarDbNettUsed 	= TRUE;
	

	char	branchNumber [3],
			status [2];

	float	qty 				= 0.0,
			totalSoQty 			= 0.0,
			totalSuppQty 		= 0.0,
			totalGrandQty 		= 0.0;

	double	extend				= 0.0,
			soShipTotal 		= 0.0,
			supplierShipTotal 	= 0.0,
			grandShipTotal 		= 0.0;

	FILE	*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startCustomer [7];
	char	endCustomer [7];
	char	cust_name [2] [41];
	int	    printerNo;
	char	printerString [3];
	char 	back [2];
	char 	backDesc [21];
	char	onite [2];
	char	oniteDesc [21];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startCustomer",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCustomer},
	{1, LIN, "startCustDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cust_name [0]},
	{1, LIN, "endCustomer",	 5, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End   Customer", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCustomer},
	{1, LIN, "endCustDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cust_name [1]},
	{1, LIN, "printerNo",	 7, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 8, 18, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 8, 22, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite",	 8, 60, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	 8, 64, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
int		ValidOrder 			(void);
int		heading 			(int);
int		spec_valid 			(int);
void	CloseDB 			(void);
void	HeadingOutput 		(void);
void	OpenDB 				(void);
void	PrintCustomer 		(void);
void	PrintInex 			(void);
void	PrintLine 			(void);
void	ProcessFile 		(void);
void	ProcessSohr 		(void);
void	ProcessSoln 		(void);
void	RunProgram 			(char *);
void	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	order_desc [30];
	char	*sptr = chk_env ("DB_MCURR");
	if (sptr)
		envVarDbMcurr = atoi (sptr);
	else
		envVarDbMcurr = FALSE;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);

	sprintf (status,"%-1.1s",argv [1]);

	if (!PACKING_SLIPS 	&& !FORWARD_ORDERS 	&& !BACKORDERS && 
		!MANUAL_ORDER 	&& !HELD_ORDERS		&& !ALL_ORDERS)
		argc = 0;

	if (argc != 2 && argc != 5)
	{
		print_at (0,0,mlSoMess780,argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	envVarDbCo 	 = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	if (argc == 5)
	{
		sprintf (local_rec.startCustomer, "%-6.6s",argv [2]);
		sprintf (local_rec.endCustomer,   "%-6.6s",argv [3]);
		local_rec.printerNo = atoi (argv [4]);

		if (PACKING_SLIPS)
			strcpy (order_desc,	ML ("Packing Slips"));

		if (FORWARD_ORDERS)
			strcpy (order_desc,	ML ("Forward Orders"));

		if (BACKORDERS)
			strcpy (order_desc, ML ("Backorders"));

		if (MANUAL_ORDER)
			strcpy (order_desc, ML ("New/Unconsolidated Orders"));

		if (HELD_ORDERS)
			strcpy (order_desc,	ML ("Held Orders"));

		if (ALL_ORDERS)
			strcpy (order_desc, ML ("ALL Orders"));

		sprintf (err_str,"Processing %s By Customer", order_desc);

		dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/
	init_vars (1);			/*  set default values			*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
 char *prog_name)
{
	sprintf (local_rec.printerString,"%d",local_rec.printerNo);
	
	shutdown_prog ();

	if (!strncmp (local_rec.endCustomer, "~~~~~~", 6))
		memset ((char *)local_rec.endCustomer,0xff,sizeof (local_rec.endCustomer));
	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				status,
				local_rec.startCustomer,
				local_rec.endCustomer,
				local_rec.printerString,
				"Print Sales Order By Customers", (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				status,
				local_rec.startCustomer,
				local_rec.endCustomer,
				local_rec.printerString, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			status,
			local_rec.startCustomer,
			local_rec.endCustomer,
			local_rec.printerString, (char *)0);
	}
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

int
spec_valid (
 int field)
{
	/*-------------------
	| Validate Customer |
	-------------------*/
	if (LCHECK ("startCustomer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCustomer,"      ");
			sprintf (local_rec.cust_name [0],"%-40.40s","First Customer");
			DSP_FLD ("startCustomer");
			DSP_FLD ("startCustDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && strcmp (local_rec.startCustomer,local_rec.endCustomer) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.startCustomer));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.cust_name [0],cumr_rec.dbt_name);
		DSP_FLD ("startCustDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCustomer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCustomer,"~~~~~~");
			sprintf (local_rec.cust_name [1],"%-40.40s",ML ("Last Customer"));
			DSP_FLD ("endCustomer");
			DSP_FLD ("endCustDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.endCustomer));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startCustomer,local_rec.endCustomer) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.cust_name [1],cumr_rec.dbt_name);
		DSP_FLD ("endCustDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo") )
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc,
			 (local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No"));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc,
			 (local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No"));
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
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

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhcu_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
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
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBOPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);

	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	if (PACKING_SLIPS)
		fprintf (fout,".EPACKING SLIPS BY CUSTOMER\n");

	if (FORWARD_ORDERS)
		fprintf (fout,".EFORWARD ORDERS BY CUSTOMER\n");

	if (BACKORDERS)
		fprintf (fout,".EBACKORDER BY CUSTOMER\n");

	if (MANUAL_ORDER)
		fprintf (fout,".ENEW/UNCONSOLIDATED ORDERS BY CUSTOMER\n");

	if (HELD_ORDERS)
		fprintf (fout,".EHELD ORDERS BY CUSTOMER\n");

	if (ALL_ORDERS)
		fprintf (fout,".EALL ORDERS BY CUSTOMER\n");

	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime ());

	fprintf (fout,".R====================================================");
	fprintf (fout,"=======");
	fprintf (fout,"=====");
	fprintf (fout,"==================================================");
	fprintf (fout,"==========================================\n");

	fprintf (fout,"========================================================");
	fprintf (fout,"=======");
	fprintf (fout,"=====");
	fprintf (fout,"==============================================");
	fprintf (fout,"==========================================\n");

	fprintf (fout,"|  S.O.  ");
	fprintf (fout,"|   CUSTOMER ORDER    ");
	fprintf (fout,"|    DATE   ");
	fprintf (fout,"|     DATE    ");
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"|UOM.");
	fprintf (fout,"|       ITEM       ");
	fprintf (fout,"|               DESCRIPTION                ");
	fprintf (fout,"| OUTSTDNG    |");
	fprintf (fout,"ST.|\n");

	fprintf (fout,"|   NO.  ");
	fprintf (fout,"|      REFERENCE      ");
	fprintf (fout,"|  ORDERED  ");
	fprintf (fout,"|     DUE     ");
	fprintf (fout,"|  OUTSTDNG  ");
	fprintf (fout,"|    ");
	fprintf (fout,"|       NUMBER     ");
	fprintf (fout,"|                                          ");
	fprintf (fout,"| SALES VALUE ");
	fprintf (fout,"|   |\n");

	PrintLine ();
	firstFlag = TRUE;
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"|--------");
	fprintf (fout,"|---------------------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|----");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|-------------|");
	fprintf (fout,"---|\n");

	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no,branchNumber);
	strcpy (cumr_rec.dbt_no,local_rec.startCustomer);

	cc = find_rec (cumr,&cumr_rec,GTEQ,"r");

	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (cumr_rec.est_no,branchNumber) && 
		      strcmp (cumr_rec.dbt_no,local_rec.startCustomer) >= 0 && 
		      strcmp (cumr_rec.dbt_no,local_rec.endCustomer) <= 0) 
	{
		sohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (sohr,&sohr_rec,GTEQ,"r");

		while (!cc && sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			ProcessSohr ();
			firstSoln = TRUE;
			cc = find_rec (sohr,&sohr_rec,NEXT,"r");
		}
		/*---------------------------
		| Print customer totals.	|
		---------------------------*/
		if (recordFound)
		{
			fprintf (fout,".LRP2\n");
			if (!afterSoLines)
				PrintLine ();
			fprintf (fout,"|  TOTAL FOR CUSTOMER          ");
			fprintf (fout,"|           ");
			fprintf (fout,"|             ");
			fprintf (fout,"| %10.2f ",totalSuppQty);
			fprintf (fout,"|    ");
			fprintf (fout,"|                  ");
			fprintf (fout,"|                      ");
			fprintf (fout,"                    ");
			fprintf (fout,"| %11.2f |",DOLLARS (supplierShipTotal));
			fprintf (fout,"   |\n");

			recordFound = FALSE;
			totalSuppQty = 0.0;
			supplierShipTotal = 0.0;
		}
		cc = find_rec (cumr,&cumr_rec,NEXT,"r");
		customerFlag = TRUE;
	}

	/*------------------------------------------
	| On last record,print the total backlog.   |
	--------------------------------------------*/
	PrintLine ();
	fprintf (fout,"|  TOTAL SALES ORDER           "); 
	fprintf (fout,"|           ");
	fprintf (fout,"|             ");
	fprintf (fout,"| %10.2f ",totalGrandQty);
	fprintf (fout,"|    ");
	fprintf (fout,"|                  ");
	fprintf (fout,"|                        ");
	fprintf (fout,"                  ");
	fprintf (fout,"| %11.2f |",DOLLARS (grandShipTotal));
	fprintf (fout,"   |\n");

	fflush (fout);
}

void
PrintCustomer (
 void)
{
	fprintf (fout,".LRP3\n");
	if (!firstFlag)
		PrintLine ();

	fprintf (fout,"| %-40.40s                                  ",
                cumr_rec.dbt_name);
	fprintf (fout,"********** (%6.6s)********** ",cumr_rec.dbt_no);
	fprintf (fout,"     ");
	fprintf (fout,"                           |             |");
	fprintf (fout,"   |\n");
	customerFlag = FALSE;
	firstFlag = FALSE;
}

void
ProcessSohr (
 void)
{
	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;

	totalSoQty = 0.0;
	soShipTotal = 0.0;
	salesOrderDetails = 0;

	cc = find_rec (soln,&soln_rec,GTEQ,"r");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		if (!ValidOrder ())
		{
			cc = find_rec (soln,&soln_rec,NEXT,"r");
			continue;
		}
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (!cc && ((soln_rec.qty_order + soln_rec.qty_bord > 0.00)
			 || inmr_rec.inmr_class [0] == 'Z'))
		{
			dsp_process ("Customer# : ",cumr_rec.dbt_no);
			if (customerFlag)
				PrintCustomer ();

			if (salesOrderDetails == 0)
			{
				fprintf (fout,"|%-8.8s",sohr_rec.order_no);
				fprintf (fout,"|%-20.20s ",sohr_rec.cus_ord_ref);
				if (sohr_rec.dt_raised == 0L)
					fprintf (fout,"|           ");
				else
					fprintf (fout,"| %-10.10s",DateToString (sohr_rec.dt_raised));
				if (sohr_rec.dt_required == 0L)
					fprintf (fout,"|             ");
				else
					fprintf (fout,"|  %-10.10s ",DateToString (sohr_rec.dt_required));
			}

			ProcessSoln ();
			fflush (fout);
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	/*-----------------------------------------------------------------
	| If more than one line for a s.o.,print total for detail lines. |
	-----------------------------------------------------------------*/
	if (salesOrderDetails > 1)
	{
		fprintf (fout,".LRP2\n");
		PrintLine ();
		fprintf (fout,"|  TOTAL FOR S.O.              ");
		fprintf (fout,"|           ");
		fprintf (fout,"|             ");
		fprintf (fout,"| %10.2f ",totalSoQty);
		fprintf (fout,"|    ");
		fprintf (fout,"|                  ");
		fprintf (fout,"|                      ");
		fprintf (fout,"                    ");
		fprintf (fout,"| %11.2f |",DOLLARS (soShipTotal));
		fprintf (fout,"   |\n");
		PrintLine ();
		afterSoLines = TRUE;
	}
	else
	{
		if (salesOrderDetails == 1)
		{
			fprintf (fout,".LRP2\n");
			fprintf (fout,"|        |                     ");
			fprintf (fout,"|           ");
			fprintf (fout,"|             ");
			fprintf (fout,"|            ");
			fprintf (fout,"|    ");
			fprintf (fout,"|                  ");
			fprintf (fout,"|                      ");
			fprintf (fout,"                    ");
			fprintf (fout,"|             |");
			fprintf (fout,"   |\n");
		}
	}
}

/*=======================
| Validate order types. |
=======================*/
int
ValidOrder (
 void)
{
	if (ALL_ORDERS)
		return (TRUE);

	if (FORWARD_ORDERS && FWD_OK)
		return (TRUE);

	if (PACKING_SLIPS && PS_OK)
		return (TRUE);

	if (BACKORDERS && BACK_OK)
		return (TRUE);

	if (MANUAL_ORDER && MAN_OK)
		return (TRUE);

	if (HELD_ORDERS && HELD_OK)
		return (TRUE);

	return (FALSE);
}

void
ProcessSoln (
 void)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00,
			lcl_ex_rate	= 0.00;

	lcl_ex_rate = (envVarDbMcurr && sohr_rec.exch_rate != 0.00) ? sohr_rec.exch_rate : 1.00;

	qty = soln_rec.qty_order + soln_rec.qty_bord;

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) qty;
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
			extend	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			extend	=	l_total + l_tax + l_gst + soln_rec.item_levy;

		extend	/=	lcl_ex_rate;
	}
	recordFound = TRUE;

	totalSoQty 			+= qty;
	soShipTotal 		+= extend;
	totalSuppQty 		+= qty;
	supplierShipTotal 	+= extend;
	totalGrandQty 		+= qty;
	grandShipTotal 		+= extend;

	if (!firstSoln)
		fprintf (fout,"|        |                     |           |             ");

	fprintf (fout,"| %10.2f ",qty);
	fprintf (fout,"|%4.4s", inmr_rec.sale_unit);
	fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
	fprintf (fout,"| %-40.40s ",soln_rec.item_desc);
	fprintf (fout,"| %11.2f |",DOLLARS (extend));
	fprintf (fout," %1.1s |\n",soln_rec.status);
	firstSoln = FALSE;

	PrintInex ();

	salesOrderDetails++;
	afterSoLines = FALSE;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		if (PACKING_SLIPS)
			strcpy (err_str,ML (mlSoMess218));

		if (FORWARD_ORDERS)
			strcpy (err_str,ML (mlSoMess219));

		if (BACKORDERS)
			strcpy (err_str,ML (mlSoMess220));

		if (MANUAL_ORDER)
			strcpy (err_str,ML (mlSoMess221));

		if (HELD_ORDERS)
			strcpy (err_str,ML (mlSoMess222));

		if (ALL_ORDERS)
			strcpy (err_str,ML (mlSoMess223));

		centre_at (1, 80, "%R %s ", err_str);

		box (0,3,80,5);
		line_at (6,1,79);
		line_at (20,0,80);
		print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintInex (
 void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, COMPARISON, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout,"|        |                     |           |             ");
		fprintf (fout,"| %10.2s "," ");
		fprintf (fout,"|%4.4s"," ");
		fprintf (fout,"| %-16.16s ", " ");
		fprintf (fout,"| %-40.40s ",inex_rec.desc);
		fprintf (fout,"| %11.2s |"," ");
		fprintf (fout," %1.1s |\n"," ");
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
