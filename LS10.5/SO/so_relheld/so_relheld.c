/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_relheld.c,v 5.9 2002/09/06 01:57:14 scott Exp $
|  Program Name  : (so_relheld.c)
|  Program Desc  : (Release Held Orders)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 12/10/88         |
|---------------------------------------------------------------------|
| $Log: so_relheld.c,v $
| Revision 5.9  2002/09/06 01:57:14  scott
| Updated for S/C 4264 - Change column heading from "A" to "Action" for GUI
|
| Revision 5.8  2002/07/24 08:39:29  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 07:18:26  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/06/20 07:16:07  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/06/20 05:49:06  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/10/23 07:16:45  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.3  2001/08/09 09:21:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:51:48  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:03  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_relheld.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_relheld/so_relheld.c,v 5.9 2002/09/06 01:57:14 scott Exp $";

#define 	MAXLINES	100
#define 	TABLINES	7
#define		CRD_OK  	0
#define		STOP_CREDIT	1
#define		OVER_LIMIT	2
#define		OVER_PERIOD	3
#define		COMMENT_LINE	(inmr_rec.inmr_class [0] == 'Z')

#include <pslscr.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>
#include <proc_sobg.h>
#include <Costing.h>

#define	LINE_OK		(soln_rec.status [0] == 'H' || \
				  	 soln_rec.status [0] == 'O')

#define	CRD_CHK		(soln_rec.status [0] == 'C')
#define	BACKORDER	(soln_rec.status [0] == 'B')

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct cumrRecord	cumr_rec;
struct cusaRecord	cusa_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cusa_value		=	&cusa_rec.val1;

	char	*data = "data";

	int		envSoCommHold		= 0,
			displayDetails 		= FALSE,
			option				= 0,
			printerNo			= 0,
			envDbCo				= 0,
			envDbFind			= 0,
			currentMonth		= 0,
			envVarRepTax 		= FALSE,
			envVarDbNettUsed 	= TRUE,
			envConOrders 		= 0;

	double	mtdSales 		= 0.00,
			ytdSales 		= 0.00,
			totalOwing 		= 0.00;

	char	branchNo [3];

	static	struct storeRec {
		long	hhsoHash;
	} store [MAXLINES];

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	customerNo [7];
	char	orderNo [9];
	long	hhsoHash;
	long	dateDue;
	double	orderValue;
	char	actionCode [2];
	char	actionDesc [21];
	char	reason [16];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo",	 4, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No. ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.customerNo},
	{1, LIN, "customerName",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Name.      ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{2, TAB, "orderNo",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Order No.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.orderNo},
	{2, TAB, "dateDue",	 0, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "  Date Due  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.dateDue},
	{2, TAB, "reason",	 0, 1, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "   Held Reason  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.reason},
	{2, TAB, "orderValue",	 0, 1, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "Order Value.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.orderValue},
#ifdef GVISION
	{2, TAB, "actionCode",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "L", "Action Code", " A(pprove credit),C(ancel order),L(eave held) ",
#else
	{2, TAB, "actionCode",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "L", " A ", " A(pprove credit),C(ancel order),L(eave held) ",
#endif
		YES, NO,  JUSTLEFT, "CAL", "", local_rec.actionCode},
	{2, TAB, "actionDesc",	 0, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Action Description ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.actionDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>

/*
 * Function Declarations
 */
double	CalculateLine 			(void);
double 	CheckCreditLine 		(void);
float 	CalcYTD 				(void);
int 	CheckBackOrder 			(void);
int  	CheckCustomer 			(double);
int		GetOrders  				(long);
int  	heading 				(int);
int		spec_valid 				(int);
void 	AddSobg 				(int, char *, long);
void 	CheckCreditCheck 		(long);
void	CloseDB 				(void);
void	DeleteOrder 			(void);
void	DisplayCreditWindow  	(void);
void 	GetMtdYtd 				(void);
void 	HoldSoln 				(long);
void	OpenDB 					(void);
void 	ProcessSoln 			(long);
void	ReleaseOrder 			(void);
void	shutdown_prog 			(void);
void	Update 					(void);

/*
 * Main Processing Routine
 */
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	if (argc > 1)
		printerNo = atoi (argv [1]);

	
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("REP_TAX");
	envVarRepTax = (sptr == (char *)0) ? 0 : atoi (sptr);
	/*
	 * Check if consolidated orders.
	 */
	sptr = chk_env ("CON_ORDERS");
	envConOrders = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Check if Comment lines are held.
	 */
	sptr = chk_env ("SO_COMM_HOLD");
	envSoCommHold = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);


	init_scr 	();
	set_tty 	();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);
	tab_row = 11;
	tab_col = 0;

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*
	 * Reset control flags
	 */
	while (prog_exit == 0)
	{
		displayDetails 	= FALSE;
		entry_exit		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		search_ok 		= TRUE;

		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		init_ok	= TRUE;
		eoi_ok	= TRUE;
		heading (1);
		entry 	(1);

		if (prog_exit || restart)
			continue;

		init_ok	= FALSE;
		eoi_ok 	= FALSE;
		heading 	(2);
		scn_display (2);
		entry 		(2);
		if (restart)
			continue;

		heading 	(2);
		scn_display (2);
		edit 		(2);
		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Program Database Open Section.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,  (!envDbFind) ? "cumr_id_no" 
							                          		 : "cumr_id_no3");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

/*
 * Program Database Close Section.
 */
void
CloseDB (void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (cumr);
	abc_fclose (cusa);
	CloseCosting ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,	comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.customerNo));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		CheckCreditCheck (cumr_rec.hhcu_hash);

		if (GetOrders (cumr_rec.hhcu_hash))
		{
			errmess (ML (mlStdMess122));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("actionCode"))
	{
		switch (local_rec.actionCode [0])
		{
		case	'C':
			strcpy (local_rec.actionDesc, ML ("Cancel        ")); 
			break; 

		case	'A':
			strcpy (local_rec.actionDesc, ML ("Approve Credit"));
			break;

		case	'L':
			strcpy (local_rec.actionDesc, ML ("Leave Held    "));
			break;
		}
		DSP_FLD ("actionDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("orderNo"))
	{
		getval (line_cnt);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Load order lines.
 */
int
GetOrders  (
	long	hhcuHash)
{
	double	orderAmount	=	0.00;
	int		foundLines	=	0,
			marginHold	=	0;

	scn_set (2);
	lcount [2] = 0;

	move (1,2);
	cl_line ();
	print_at (2,1,ML (mlStdMess035));
	fflush (stdout);
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sohr_rec.hhcu_hash = hhcuHash;
	strcpy (sohr_rec.order_no,"        ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no,comm_rec.est_no) && 
		       sohr_rec.hhcu_hash == hhcuHash)
	{
		foundLines 	= FALSE;
		orderAmount = 0.00;
		marginHold 	= FALSE;
		
		soln_rec.hhso_hash	= sohr_rec.hhso_hash;
		soln_rec.line_no	= 0;
		cc = find_rec (soln,&soln_rec,GTEQ,"r");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (LINE_OK)
			{
				orderAmount += CalculateLine ();
				foundLines = TRUE;
				if (soln_rec.status [0] == 'O')
					marginHold = TRUE;
			}
			cc = find_rec (soln, &soln_rec, NEXT,"r");
		}

		if (foundLines)
		{
			strcpy (local_rec.orderNo,sohr_rec.order_no);
			if (marginHold)
				strcpy (local_rec.reason,ML ("Margin Exceeded"));
			else
				strcpy (local_rec.reason,ML ("Credit Exceeded"));

			strcpy (local_rec.actionCode," ");
			strcpy (local_rec.actionDesc,"              ");
			local_rec.dateDue 		= sohr_rec.dt_required;
			local_rec.orderValue 	= orderAmount;
			store [lcount [2]].hhsoHash = sohr_rec.hhso_hash;
			putval (lcount [2]++);
		}
		if (lcount [2] >= MAXLINES)
			break;

		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}

	/*---------------------------------
	| set max no. of tabular entries. |
	---------------------------------*/
	vars [scn_start].row = lcount [2];

	if (lcount [2])
	{
		scn_set (1);
		return (EXIT_SUCCESS);
	}

	vars [scn_start].row = MAXLINES;
	scn_set (1);
	return (EXIT_FAILURE);
}

/*
 * Calculate line total.
 */
double	
CalculateLine (void)
{
	double	net_val = 0.00;

	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;


	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		return (0.00);
   	
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
	return (net_val);
}

/*
 * Display Credit window.
 */
void
DisplayCreditWindow  (void)
{
	char	stopCredit [4];
	double	currentBal 	= 0.00,
			outstandBal = 0.00,
			orderBal 	= 0.00;

	displayDetails 	= 	TRUE;
	currentBal  	= 	cumr_balance [0] + cumr_balance [5];
	outstandBal  	= 	cumr_balance [1] + cumr_balance [2] + 
		   				cumr_balance [3] + cumr_balance [4];

	orderBal  = cumr_rec.ord_value;

	strcpy (stopCredit, (cumr_rec.stop_credit [0] == 'Y') ? "YES" : "NO.");

	GetMtdYtd ();

	print_at (3,3,ML (mlStdMess012),cumr_rec.dbt_no, cumr_rec.dbt_name);
	print_at (4,3,ML (mlSoMess021),DOLLARS (mtdSales));
	print_at (5,3,ML (mlSoMess022),cumr_rec.crd_prd);
	print_at (6,3,ML (mlSoMess023),cumr_rec.payment_flag);
	print_at (7,3,ML (mlSoMess024),stopCredit);
	print_at (8,3,ML (mlSoMess025),cumr_rec.phone_no);
	print_at (9,3,ML (mlSoMess026),cumr_rec.contact_name);
	print_at (4,50,ML (mlSoMess027),DOLLARS (ytdSales));
	print_at (5,50,ML (mlSoMess028),DOLLARS (cumr_rec.credit_limit));
	print_at (6,50,ML (mlSoMess029),DOLLARS (currentBal + outstandBal));
	print_at (7,50,ML (mlSoMess030),DOLLARS (outstandBal));
	print_at (8,50,ML (mlSoMess031),DOLLARS (orderBal));
	print_at (9,50,ML (mlSoMess032),DOLLARS (orderBal + outstandBal + currentBal));
}

/*
 * Update Lines.
 */
void
Update (void)
{
	abc_selfield (sohr,"sohr_hhso_hash");
	scn_set (2);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);
		local_rec.hhsoHash =  store [line_cnt].hhsoHash;

		switch (local_rec.actionCode [0])
		{
			case	'C':
				DeleteOrder ();
			break;

			case	'A':
				ReleaseOrder ();
			break;

			case	'L':
				abc_unlock (sohr);
			break;

			default:
				break;
		}
	}
	abc_selfield (sohr,"sohr_id_no");
	abc_unlock (sohr);
	recalc_sobg ();
}

/*
 * Delete Orders as no longer needed.
 */
void
DeleteOrder (void)
{
	int		nonStockDelete = 0;

	soln_rec.hhso_hash	= local_rec.hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == local_rec.hhsoHash)
	{
		incc_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		cc	=	UpdateInsf 
				(
					incc_rec.hhwh_hash,
					0L,
					soln_rec.serial_no,
					"C",
					"F"
				);
		if (cc)
		{
			cc	=	UpdateInsf 
					(
						0L,
						incc_rec.hhbr_hash,
						soln_rec.serial_no,
						"C",
						"F"
					);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	soln_rec.hhso_hash	= local_rec.hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln,&soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == local_rec.hhsoHash)
	{
		if (!COMMENT_LINE && LINE_OK)
			nonStockDelete = TRUE;

		if (!COMMENT_LINE && !LINE_OK)
			nonStockDelete = FALSE;
			
		/*
		 * Delete if H(eld or Non_Stock associated with H(eld item
		 */
		if (LINE_OK || nonStockDelete)
		{
			AddSobg (0, "RC", soln_rec.hhbr_hash);
			cc = abc_delete (soln);

			cc = find_rec (soln,&soln_rec,GTEQ,"u");
		}
		else
			cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	/*
	 * Delete sohr record only if no lines remain.
	 */
	soln_rec.hhso_hash	= local_rec.hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");
	if (cc || soln_rec.hhso_hash != local_rec.hhsoHash)
	{
		sohr_rec.hhso_hash	=	local_rec.hhsoHash;
		cc = find_rec (sohr,&sohr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, sohr, "DBFIND");
		
		cc = abc_delete (sohr);
		if (cc)
			file_err (cc, sohr, "DBDELETE");
	}
}

/*
 * Check for Backorders 
 */
int
CheckBackOrder (void)
{
	int		i = 0;

	soln_rec.hhso_hash 	= local_rec.hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln,&soln_rec,GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == local_rec.hhsoHash)
	{
		if (BACKORDER)
		{
		 	i = prmptmsg (ML ("Backordered item found. Continue [y/n]?"), 
				"YyNn",1,23);
										   
		    if (i == 'Y' || i == 'y')
				return TRUE;
			else
				return FALSE;
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	return TRUE;
}
/*
 * Release Held Orders.
 */
void
ReleaseOrder (void)
{
	sohr_rec.hhso_hash	=	local_rec.hhsoHash;
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"u");
	if (cc)
		file_err (cc, sohr, "DBFIND");
	
	if (!CheckBackOrder ())
		return;

	soln_rec.hhso_hash	= local_rec.hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");
	while (!cc && soln_rec.hhso_hash == local_rec.hhsoHash)
	{
		if (LINE_OK)
		{
			if (sohr_rec.two_step [0] == 'Y')
				strcpy (soln_rec.stat_flag, "M");

			strcpy (soln_rec.status, soln_rec.stat_flag);
			cc = abc_update (soln,&soln_rec);
			if (cc)
				file_err (cc, soln, "DBUPDATE");
		}
		else
			abc_unlock (soln);

		cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	/*
	 * This will restore originial status before being placed on hold.
	 */
	if (sohr_rec.two_step [0] == 'Y')
		strcpy (sohr_rec.stat_flag, "M");

	strcpy (sohr_rec.status, sohr_rec.stat_flag);
	cc = abc_update (sohr,&sohr_rec);
	if (cc)
		file_err (cc, "sohr", "DBUPDATE");

	/*
	 * Only add sobg record is Sales order is set to one Step Release.
	 */
	if (sohr_rec.status [0] == 'R')
		AddSobg (printerNo, (printerNo) ? "PA" : "PC",sohr_rec.hhso_hash);
}

void
GetMtdYtd (void)
{
	DateToDMY (comm_rec.dbt_date, NULL, &currentMonth, NULL);
	currentMonth--;

	/*
	 * Load Year To Date Sales from incc
	 */
	mtdSales = 0.00;
	ytdSales = 0.00;

	cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cusa_rec.year,"C");
	if (!find_rec (cusa, &cusa_rec, COMPARISON,"r"))
	{
		mtdSales = cusa_value [currentMonth];
		ytdSales = CalcYTD ();
	}
}

/*
 * Calculate Ytd Sales. 
 */
float	
CalcYTD (void)
{
	int		i	= 0;
	float	ytd = 0.00;

	/*
	 * no fiscal set up.
	 */
	if (comm_rec.fiscal == 0)
	{
		/*
		 * sum to current month (feb == 1)
		 */
		for (i = 0;i <= currentMonth;i++)
			ytd += (float) (cusa_value [i]);

		return (ytd);
	}

	/*
	 * need to sum from fiscal to dec, then jan to current month.
	 */
	if (currentMonth < comm_rec.fiscal)
	{
		for (i = comm_rec.fiscal;i < 12;i++)
			ytd += (float) (cusa_value [i]);

		for (i = 0;i <= currentMonth;i++)
			ytd += (float) (cusa_value [i]);

		return (ytd);
	}
	for (i = comm_rec.fiscal;i <= currentMonth;i++)
		ytd += (float) (cusa_value [i]);

	return (ytd);
}

void
CheckCreditCheck (
	long	hhcuHash)
{
	double	orderTotal  = 0.00;
	int		crdType 	= 0;

	print_mess (ML (mlSoMess033));
	fflush (stdout);

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sohr_rec.hhcu_hash = hhcuHash;
	strcpy (sohr_rec.order_no,"        ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"u");
	while (!cc && !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no,comm_rec.est_no) && 
		       sohr_rec.hhcu_hash == hhcuHash)
	{
		/*
		 * Check value of order about to process.
		 * If going to take it over then place whole order on hold.
		 */
		orderTotal = 0.00;

		soln_rec.hhso_hash	= sohr_rec.hhso_hash;
		soln_rec.line_no 	= 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (CRD_CHK)
				orderTotal += CheckCreditLine ();

			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		crdType = CheckCustomer (orderTotal);
	
		switch (crdType)
		{
			case 	STOP_CREDIT:
			case	OVER_LIMIT:
			case	OVER_PERIOD:
					HoldSoln (sohr_rec.hhso_hash);
				break;
	
			case 	CRD_OK:
					ProcessSoln (sohr_rec.hhso_hash);
				break;
	
			default:
				break;
		}
		abc_unlock (sohr);
		cc = find_rec (sohr,&sohr_rec,NEXT,"u");
	}
	abc_unlock (sohr);
}

/*
 * Calculate value of order line. 
 */
double	
CheckCreditLine (void)
{
	double	lineValue 		= 0.00,
			lineTotal 		= 0.00,
			lineDiscount	= 0.00;
		
	float	qtySupplied 	= 0.00;

	qtySupplied = soln_rec.qty_order + soln_rec.qty_bord;

	if (qtySupplied <= 0.00)
		return (0.00);

	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	/*
	 * Update coln gross tax and disc for each line.
	 */
	lineTotal = (double) qtySupplied;
	lineTotal *= out_cost (soln_rec.sale_price,inmr_rec.outer_size);

	lineDiscount = (double) (soln_rec.dis_pc / 100.00);
	lineDiscount *= lineTotal;

	lineValue += lineTotal - lineDiscount;

	return (lineValue);
}
/*
 * Validate credit period and credit limit.
 */
int
CheckCustomer (
	double	includeAmount)
{
	int	i;

	totalOwing	=	(cumr_balance [0] + 
		        	 cumr_balance [1] + 
		        	 cumr_balance [2] + 
		        	 cumr_balance [3] + 
		        	 cumr_balance [4] + 
		        	 cumr_balance [5] + 
				 	 cumr_rec.ord_value +
					 includeAmount);

	if (cumr_rec.stop_credit [0] == 'Y')
		return (STOP_CREDIT);

	/*
	 * Check if customer is over his credit limit.
	 */
	if (cumr_rec.credit_limit <= totalOwing && cumr_rec.credit_limit != 0.00)
		return (OVER_LIMIT);

	/*
	 * Check Credit Terms
	 */
	for (i = 1;i < 5;i++)
	{
		if (cumr_balance [i] > 0.00)
			return (OVER_PERIOD);
	}
	return (CRD_OK);
}

/*
 * Order Lines and Header are to be Held.
 */
void
HoldSoln (
	long	hhsoHash)
{
	int		nonStockDelete 	= FALSE,
			heldFlag 		= FALSE,
			headerUpdate 	= FALSE;

	soln_rec.hhso_hash	= hhsoHash;
	soln_rec.line_no	= 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		if (!COMMENT_LINE && CRD_CHK)
			nonStockDelete = TRUE;

		if (!COMMENT_LINE && !CRD_CHK)
			nonStockDelete = FALSE;
		
		if (COMMENT_LINE && nonStockDelete && !CRD_CHK)
		{
			if (envSoCommHold)
			{
				abc_unlock (soln);
				abc_delete (soln);
				cc = find_rec (soln,&soln_rec,GTEQ,"u");
				continue;
			}
			strcpy (soln_rec.status, "C");
		}
		
		if (CRD_CHK)
		{
			heldFlag = TRUE;
			strcpy (soln_rec.stat_flag, (envConOrders) ? "M" : "R");
			strcpy (soln_rec.status, "H");
			cc = abc_update (soln,&soln_rec);
			if (cc)
				file_err (cc, "soln", "DBUPDATE");

			headerUpdate = TRUE;
		}
		else
			abc_unlock (soln);

		cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	if (headerUpdate)
	{
		/*
		 * Some or all lines held so set header as held.
		 */
		strcpy (sohr_rec.stat_flag, (envConOrders) ? "M" : "R");
		strcpy (sohr_rec.status,   	(envConOrders) ? "M" : "R");
		if (heldFlag == TRUE)
			strcpy (sohr_rec.status,"H");
	
		if (sohr_rec.status [0] == 'R')
			AddSobg (printerNo, (printerNo) ? "PA" : "PC",sohr_rec.hhso_hash);

		cc = abc_update (sohr,&sohr_rec);
		if (cc)
			file_err (cc, "sohr", "DBUPDATE");

	}
	abc_unlock (sohr);
}
/*
 * Order Lines and Header are to be Released.
 */
void
ProcessSoln (
	long	hhsoHash)
{
	int	nonStockDelete = FALSE;
	int	header_updated = FALSE;

	soln_rec.hhso_hash = hhsoHash;
	soln_rec.line_no = 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"u");

	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		if (!COMMENT_LINE && CRD_CHK)
			nonStockDelete = TRUE;

		if (!COMMENT_LINE && !CRD_CHK)
			nonStockDelete = FALSE;
		
		if (COMMENT_LINE && nonStockDelete && !CRD_CHK)
		{
			if (!envSoCommHold)
			{
				abc_unlock (soln);
				abc_delete (soln);
				cc = find_rec (soln,&soln_rec,GTEQ,"u");
				continue;
			}
			strcpy (soln_rec.status, "C");
		}
		if (!CRD_CHK)
		{
			abc_unlock (soln);
			cc = find_rec (soln,&soln_rec,NEXT,"u");
			continue;
		}
		strcpy (soln_rec.stat_flag, (envConOrders) ? "M" : "R");
		strcpy (soln_rec.status,   	(envConOrders) ? "M" : "R");

		cc = abc_update (soln,&soln_rec);
		if (cc)
			file_err (cc, "soln", "DBUPDATE");
		
		header_updated = TRUE;
		
		cc = find_rec (soln,&soln_rec,NEXT,"u");
	}
	abc_unlock (soln);

	if (header_updated)
	{
		strcpy (sohr_rec.status,	(envConOrders) ? "M" : "R");
		strcpy (sohr_rec.stat_flag, (envConOrders) ? "M" : "R");
		cc = abc_update (sohr,&sohr_rec);
		if (cc)
			file_err (cc, "sohr", "DBUPDATE");
	
		/*
		 * Only Release is Automatic.
		 */
		if (sohr_rec.status [0] == 'R')
			AddSobg (printerNo, (printerNo) ? "PA" : "PC",sohr_rec.hhso_hash);
	
		AddSobg (0,"RO",sohr_rec.hhcu_hash);
	}
}

/*
 * Add record to background processing file. 
 */
void
AddSobg (
	int		printerNo, 
	char	*typeFlag, 
	long	processHash)
{
	add_hash
	(
		comm_rec.co_no,
		comm_rec.est_no,
		typeFlag,
		printerNo,
		processHash,
		0L,
		0L,
		(double) 0.00
	);
}

int
heading (
	int	scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlSoMess034),30,0,1);

	line_at (1,0,80);

	switch (scn)
	{
	case	1:
		box (0,3,80,2);
		break;

	case	2:
		box (0,2,80,7);
		DisplayCreditWindow ();
		break;

	default:
		break;
	}

	line_at (20,0,80);
	print_at (21,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
