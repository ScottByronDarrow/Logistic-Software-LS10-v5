/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_daily_sales.c)                                |
|  Program Desc  : ( Daily Orders Report                          )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: daily_sales.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_daily_sales/daily_sales.c,v 5.3 2001/10/23 07:16:39 scott Exp $";

#define	TABLINES	14

#include <pslscr.h>
#include <twodec.h>

#define	SLEEP_TIME	3

#define	S_DUMM	  0
#define	S_HEAD	  1
#define DFLT_LPNO 1
#define PRTPRICE  !strcmp (soln_rec.pri_or, "Y")
#define PRTDISC   !strcmp (soln_rec.dis_or, "Y")

#include	"schema"

struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct commRecord	comm_rec;

	/*=========================== 
	| Local & Screen Structures.|
	===========================*/
	struct {
		char	dummy[11];
		char    systemDate[11];
		Date    sdate;
		Date    edate;
		int		lpno;
	} local_rec;

	int		firstTime;
	float	totalQty,
			totalValue,
			totalGst,
			totalReport;
	double	totalReportQty;
	FILE	*fout;

	char	*data	= "data";

static	struct	var vars[] =
{
	{S_HEAD, LIN, "sdate", 2, 15, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", " ", "Start Date : ",
		"Enter A Start Date",
		NE, NO,  JUSTLEFT, "", "", (char *) &local_rec.sdate},
	{S_HEAD, LIN, "edate", 3, 15, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", " ", "End Date   : ",
		"Enter A End Date",
		NE, NO,  JUSTLEFT, "", "", (char *) &local_rec.edate},
	{S_HEAD, LIN, "lpno", 4, 15, INTTYPE,
		"NN", "          ",
		" ", " ", "Printer    : ",
		"Enter A Printer Number",
		NE, NO,  JUSTRIGHT, "", "", (char *) &local_rec.lpno},

	{S_DUMM, LIN, "",		0, 0, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", (char *) &local_rec.dummy}
};


/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	ReportPrint 	(void);
int  	CheckDefault 	(void);
void 	OpenPrint 		(void);
void 	ClosePrint 		(void);
void 	PrintLine 		(void);
void 	RunFromScript 	(void);
void 	ResetVars 		(void);
void 	PrintTotal 		(void);
void 	PrintReportTot 	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc != 2 && argc != 4)
	{
		clear ();
		print_at (0,0, "Usage %s - Run From Menu", argv[0]);
		print_at (1,0, "<StartDate> <EndDate> <LPNO>  - Overnight Script");
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (argc == 4)
	{
		local_rec.sdate = StringToDate (argv[1]);
		local_rec.edate = StringToDate (argv[2]);

		if (atoi (argv[1]) == -1)
			local_rec.sdate = TodaysDate();

		if (atoi (argv[2]) == -1)
			local_rec.edate = TodaysDate();

		local_rec.lpno  = atoi (argv[3]);

		RunFromScript ();
		return (EXIT_SUCCESS);
	}
	else
		local_rec.lpno  = atoi (argv[1]);


	SETUP_SCR (vars);
	init_scr  ();			/*  sets terminal from termcap	  */
	set_tty   ();
	set_masks ();

	OpenDB ();
	clear   ();

	tab_row = 3;
	tab_col = 0;


	snorm ();
	/*---------------------------------
	| Beginning of input control loop |
	---------------------------------*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*----------------------
		| Reset Control Flags  |
		----------------------*/
		entry_exit  = FALSE;
		edit_exit   = FALSE;
		search_ok   = TRUE;
		prog_exit   = FALSE;
		prog_status = ENTRY;
		restart     = FALSE;

		init_vars  (S_HEAD);

		heading (S_HEAD);
		entry 	(S_HEAD);

		if (restart || prog_exit)
			continue;

		prog_exit = TRUE;
		ReportPrint ();
	}

	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

void
CloseDB (
 void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_dbclose (data);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		print_at (0, 30, "%R Daily Orders Report");

		box (0, 1, 79, 3);

		fflush (stdout);

		scn_set 	(scn);
		scn_write 	(scn);
		scn_display (scn);

		line_at (21, 0, 79);
		print_at (22, 0, " Co : %-2.2s %-15.15s",
				  comm_rec.co_no,
				  comm_rec.co_short);

		/*  reset this variable for new screen NOT page	*/
		fflush (stdout);
		line_cnt = 0;
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{

	if (LCHECK ("sdate"))
	{
		if (dflt_used)
			local_rec.sdate = TodaysDate ();

		DSP_FLD ("sdate");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("edate"))
	{
		if (dflt_used)
			local_rec.edate = TodaysDate ();

		DSP_FLD ("edate");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (dflt_used)
			local_rec.lpno = DFLT_LPNO;

		DSP_FLD ("lpno");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

	/*--------------------
	| Print Sales Orders |
	--------------------*/
void
ReportPrint (
 void)
{
	OpenPrint ();

	strcpy  (sohr_rec.co_no , comm_rec.co_no);
	strcpy  (sohr_rec.br_no , comm_rec.est_no);
	sprintf (sohr_rec.inv_no, "%-8.8s", " ");
	for (cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
		!cc &&
		!strcmp  (sohr_rec.co_no , comm_rec.co_no) &&
		!strcmp  (sohr_rec.br_no , comm_rec.est_no);
		cc = find_rec (sohr, &sohr_rec, NEXT, "r"))
	{
		ResetVars ();

		if (strcmp (sohr_rec.status, "D") &&
			sohr_rec.dt_raised >= local_rec.sdate &&
			sohr_rec.dt_raised <= local_rec.edate)
		{

			soln_rec.hhso_hash = sohr_rec.hhso_hash;
			soln_rec.line_no = 0;
			for (cc = find_rec (soln, &soln_rec, GTEQ, "r");
				!cc &&
				soln_rec.hhso_hash == sohr_rec.hhso_hash;
				cc = find_rec (soln, &soln_rec, NEXT, "r"))
			{
				if (twodec (soln_rec.qty_order) != 0.00 ||
					twodec (soln_rec.qty_bord) != 0.00)
				{
					PrintLine ();
					firstTime = FALSE;
				}
			}

			PrintTotal ();
		}
	}

	PrintReportTot ();
	ClosePrint ();
}

int 
CheckDefault (
 void)
{
	if (dflt_used)
	{
		print_mess (" Default not available \007");
		sleep (sleepTime);
		clear_mess ();
		return (TRUE);
	}
	return (FALSE);
}

void
OpenPrint (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".8\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, "     %s",clip (comm_rec.co_name));
	fprintf (fout, "                    O R D E R  R E G I S T E R  -  E N D  O F   D A Y");
	fprintf (fout, "       Run  : %s\n", SystemTime ());
	fprintf (fout, "                                                                        All Outstanding Orders                        Prog : %s\n", PNAME);
	fprintf (fout, ".B1\n");
	fprintf (fout, "Operator  Order                Order     Cust                                                Customer     Qty                                    Value\n");
	fprintf (fout, "          Number               Date      Code     Customer Name                    Locn      Order No     Ordered       Price    Disc           (Excl Tax)\n");

	fprintf (fout, "========");
	fprintf (fout, "=====");
	fprintf (fout, "=============");
	fprintf (fout, "========");
	fprintf (fout, "====");
	fprintf (fout, "=========");
	fprintf (fout, "========");
	fprintf (fout, "=========");
	fprintf (fout, "======");
	fprintf (fout, "====");
	fprintf (fout, "========");
	fprintf (fout, "====");
	fprintf (fout, "===========================================");
	fprintf (fout, "=========================\n");
	
	fprintf (fout, ".R========");
	fprintf (fout, "=====");
	fprintf (fout, "=============");
	fprintf (fout, "========");
	fprintf (fout, "====");
	fprintf (fout, "=========");
	fprintf (fout, "========");
	fprintf (fout, "=========");
	fprintf (fout, "======");
	fprintf (fout, "====");
	fprintf (fout, "========");
	fprintf (fout, "====");
	fprintf (fout, "===========================================");
	fprintf (fout, "=========================\n");
		
	fflush  (fout);
}

void
ClosePrint (
 void)
{
	fprintf(fout, ".EOF\n");
	fflush  (fout);
	pclose(fout);
}

void
PrintLine (
 void)
{
	double 	lineValue	=	0.00,
			l_total		=	0.00,
			l_disc		=	0.00;

	if (firstTime)
	{
		strcpy (cumr_rec.dbt_no  , "******");
		strcpy (cumr_rec.dbt_name, " * Unknown Customer Master Record *");
		find_hash (cumr, &cumr_rec, EQUAL, "r", sohr_rec.hhcu_hash);

		fprintf (fout, "%-9.9s "    , sohr_rec.op_id);
		fprintf (fout, "%-8.8s "  ,   sohr_rec.order_no);
		fprintf (fout, "%-6.6s     ", " ");
		fprintf (fout, "%-10.10s "    , DateToString (sohr_rec.dt_raised));
		fprintf (fout, "%-6.6s   "  , cumr_rec.dbt_no);
		fprintf (fout, "%-30.30s  " , cumr_rec.dbt_name);
		fprintf (fout, "%-2.2s    " , sohr_rec.sman_code);
		fprintf (fout, "%-6.6s/%-6.6s\n", sohr_rec.cus_ord_ref,
										  soln_rec.cus_ord_ref);
	}

	strcpy (inmr_rec.item_no, "******");
	find_hash (inmr, &inmr_rec, EQUAL, "r", soln_rec.hhbr_hash);

	l_total	=	(double) soln_rec.qty_order + soln_rec.qty_bord;
	l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
	l_total	=	no_dec (l_total);

	l_disc	=	(double) soln_rec.dis_pc;
	l_disc	*=	l_total;
	l_disc	=	DOLLARS (l_disc);
	l_disc	=	no_dec (l_disc);

	lineValue	=	l_total - l_disc;
	
	fprintf (fout, "%-30.30s "  , " ");
	fprintf (fout, "%-6.6s  "   , inmr_rec.item_no);
	fprintf (fout, "%-40.40s                     ", soln_rec.item_desc);
	fprintf (fout, "%8.2f      ", soln_rec.qty_order + soln_rec.qty_bord);
	fprintf (fout, "%8.2f  ", DOLLARS (soln_rec.sale_price));
	fprintf (fout, "%8.2f      ", soln_rec.dis_pc);
	fprintf (fout, "%8.2f %-1.1s %-1.1s\n", DOLLARS (lineValue),
							PRTPRICE ? "P" : " ",
							PRTDISC ? "D" : " ");

	totalValue += lineValue; 
	totalQty   += soln_rec.qty_order;
	totalGst   += (soln_rec.gst_pc / 100) * lineValue;

	fflush  (fout);
}

void
RunFromScript  (
 void)
{
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReportPrint ();	
	CloseDB (); 
	FinishProgram ();
}

void
ResetVars (
 void)
{
	firstTime = TRUE;
	totalValue = 0.00;
	totalQty   = 0.00;
	totalGst   = 0.00;
}

void
PrintTotal (
 void)
{
	if (firstTime)
		return;

	totalValue += totalGst;
	fprintf (fout, "%-80.80s G.S.T", " ");
	fprintf (fout, "%-51.51s %8.2f\n", " ", DOLLARS (totalGst));
	fprintf (fout, ".B1\n");
	fprintf (fout, "%-76.76s * Order Total *%-8.8s", " ", " ");
	fprintf (fout, "%8.2f %-28.28s %8.2f\n", totalQty, " ",
					DOLLARS (totalValue));
	fprintf (fout, "%-138.138s ----------", " ");
	fprintf (fout, "\n");

	totalReportQty += totalQty;
	totalReport    += totalValue;
}

void
PrintReportTot (
 void)
{
	fprintf (fout, ".B1\n");
	fprintf (fout, "%-76.76s * Report Total *%-4.4s %10.2f", " ", " ",
					totalReportQty);
	fprintf (fout, "%-29.29s %8.2f\n", " ", DOLLARS (totalReport));
	fprintf (fout, "%-138.138s ----------", " ");
	fprintf (fout, "\n");
}
