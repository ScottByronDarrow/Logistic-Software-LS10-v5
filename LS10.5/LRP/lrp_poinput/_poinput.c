/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: _poinput.c,v 5.4 2002/11/25 03:16:38 scott Exp $
|  Program Name  : (lrp_poinput.c)
|  Program Desc  : (Input Purchase Orders from Reorder Review)
|                (Report. (ie Qty's & Select Suppliers))
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: _poinput.c,v $
| Revision 5.4  2002/11/25 03:16:38  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.3  2002/09/16 05:04:16  scott
| Updated to allow days demend to be shown on screen
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _poinput.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_poinput/_poinput.c,v 5.4 2002/11/25 03:16:38 scott Exp $";

#include	<pslscr.h>
#include	<getnum.h>
#include	<hot_keys.h>
#include	<get_lpno.h>
#define		OTHER			1
#define		ABC_CODE		2
#define     NO_SUPPLIER     3
#define     REORDER     	4
#define		MAX_WH			100
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_lrp_mess.h>
#include    <tabdisp.h>

extern	int		tab_max_page;

#define	WK_DEPTH	16
#define	MAX_SUPPLY	27

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ffwkRecord	ffwk_rec;
struct inccRecord	incc_rec;
struct ineiRecord	inei_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct itlnRecord	itln_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocrRec;
struct podtRecord	podt_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inspRecord	insp_rec;
struct sudsRecord	suds_rec;

float	*ffwk_cons	=	&ffwk_rec.cons_1;

	char 	*ffwk2			= "ffwk2",
			*inis2			= "inis2",
			*inmr2			= "inmr2",
			*sumr2			= "sumr2",
			*lrpWork		= "lrpWork",
			*supplierWork	= "supplierWork";

	char	branchNumber [3];

	struct {
		double	wkValue;
		long	hhsuHash;
		long	hhisHash;
		char	priority [3];
		int		firstSupplier;
		char	brNo [3];
		char	whNo [3];
		char	supplierNo [9];
		char	itemNo [17];
		char	lead [8];
		char	curr [6];
		char	ctry [6];
		char	fobCost [13];
		char	duty [9];
		char	frt [12];
		char	intoStore [13];
		char	moq [11];
		char	noq [11];
		char	oml [11];
	} wk_rec, SR [MAX_SUPPLY + 1];

	int		LRP_ByCompany		= FALSE,
			LRP_ByBranch		= FALSE,
			LRP_ByWarehouse		= FALSE,
			LRP_LinesProcessed	= 0,
			LRP_NoRecords		= 0,
			LRP_PriOneSupplier	= 0,
			LRP_SupplierIndex	= 0,
			LRP_OrderSupplier	= 0,
			envLrpInpZero 		= 0,
			envLrpPriority 		= 0,
			envLrpShowAllSup 	= 0,
			maxLines			= 0,
			currentMonth		= 0,
			firstLine 			= 0,
			useMinStockWks 		= FALSE,
			NoSuppTabLines		=	0,
			currDay				=	0,
			currMonth			=	0;

	double	freight  = 0.00;

	long	validWh [MAX_WH + 1],
			lsystemDate,
			hhccHash;

	float	LRP_QtyRecommended,
			LRP_MinOrderQty,
			LRP_NorOrderQty,
			LRP_LeadDays,
			LRP_LeadWeeks,
			wks_tot_cover;

	char	envSupOrdRound 	 [2],
			envCurrCode 	 [4],
			envLrpShowDemand  [2],
			LRP_FileName 	 [15],
			LRP_ABCCODES 	 [5],
			LRP_ReorderFlag [2],
			LRP_SuppBuffer 	 [256],
			LRP_TempBuffer 	 [256],
			periodHead [11] [4];

static char *monthName [] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};

static char *dayName [] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", ""};

	char	*mth_nam [] =
	{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

struct
{
	float	safetyStock;
	char	abcCode [2];
} local_rec;

static	int	ScnConfirm 		(int, KEY_TAB *);
static	int	ScnConfirmAll 	(int, KEY_TAB *);
static	int	ScnQuantity 	(int, KEY_TAB *);
static	int	ScnReject  		(int, KEY_TAB *);
static	int	ScnRejectAll 	(int, KEY_TAB *);
static	int	ScnSupplier 	(int, KEY_TAB *);
static	int	ScnAlldisp 		(int, KEY_TAB *);
static	int	ScnLrpdisp 		(int, KEY_TAB *);
static	int	ScnMove	   		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	Header_keys [] =
{
	{ " Quantity ",	'Q',		ScnQuantity,
		"Enter a new quantity"						},
	{ " Supplier ",	'S',		ScnSupplier,
		"Choose a new supplier/quantity"			},
	{ " Accept " ,	'A',			ScnConfirm,
		"Accept the current Purchase-Order"			},
	{ " Accept All ",	CTRL ('A'),	ScnConfirmAll,
		"Accept ALL remaining Purchase-Orders"		},
	{ " Cancel ",	'C',		ScnReject,
		"Cancel the current Purchase-Order"			},
	{ " Cancel All ",	CTRL ('C'),	ScnRejectAll,
		"Cancel ALL remaining Purchase-Orders"		},
	{ " Display stock ",	'D',	ScnAlldisp,
		"Stock Display"								},
	{ " LRP Display ",	'L',	ScnLrpdisp,
		"LRP Reorder Display"								},
	{ " Move ",	'M',	ScnMove,
		"Move to previous position in file."		},
	END_KEYS
};
#else
static	KEY_TAB	Header_keys [] =
{
	{ " [Q]uantity",	'Q',		ScnQuantity,
		"Enter a new quantity"						},
	{ " [S]upplier",	'S',		ScnSupplier,
		"Choose a new supplier/quantity"			},
	{ " [A]ccept ",	'A',			ScnConfirm,
		"Accept the current Purchase-Order"			},
	{ " [^A]ccept All",	CTRL ('A'),	ScnConfirmAll,
		"Accept ALL remaining Purchase-Orders"		},
	{ " [C]ancel",	'C',		ScnReject,
		"Cancel the current Purchase-Order"			},
	{ " [^C]ancel All",	CTRL ('C'),	ScnRejectAll,
		"Cancel ALL remaining Purchase-Orders"		},
	{ " [D]isplay stock",	'D',	ScnAlldisp,
		"Stock Display"								},
	{ " [L]RP Display",	'L',	ScnLrpdisp,
		"LRP Reorder Display"								},
	{ " [M]ove",	'M',	ScnMove,
		"Move to previous position in file."		},
	END_KEYS
};
#endif

static	int	AddToSuppPriority 		(int, KEY_TAB *);
static	int	SubtractSuppPriority 	(int, KEY_TAB *);
static	int	SetSupplierPriority 	(int, KEY_TAB *);
static	int	SupplierSelected 		(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB	Supplier_keys [] =
{
	{ NULL,	'\r',									SupplierSelected,
		"Exit item Selected",					"A"	},
	{ NULL,	FN16,									SupplierSelected,
		"Exit item Selected",					"A"	},
	{ " Add 1 to supplier Priority ",		'+',	AddToSuppPriority,
		"Enter a new quantity"						},
	{ " Subtract 1 from supplier Priority ",'-',	SubtractSuppPriority,
		"Choose a new supplier/quantity"			},
	{ " Set supplier Priority to 1 ",		'S',	SetSupplierPriority,
		"Accept the current Purchase-Order"			},
	END_KEYS
};
#else
static	KEY_TAB	Supplier_keys [] =
{
	{ NULL,	'\r',									SupplierSelected,
		"Exit item Selected",					"A"	},
	{ NULL,	FN16,									SupplierSelected,
		"Exit item Selected",					"A"	},
	{ " [+]Add 1 to supplier Priority ",		'+',	AddToSuppPriority,
		"Enter a new quantity"						},
	{ " [-]Subtract 1 from supplier Priority",'-',	SubtractSuppPriority,
		"Choose a new supplier/quantity"			},
	{ " [S]et supplier Priority to 1 ",		'S',	SetSupplierPriority,
		"Accept the current Purchase-Order"			},
	END_KEYS
};
#endif

#include <SupPrice.h>
#include <RealCommit.h>

/*
 * Local Function Prototypes.
 */
float 	CheckValue 			(char *, float);
float 	GetLeadDate 		(int, int);
float 	TrigMax 			(float, float, float);
int 	DisplayEntry 		(int);
int 	GetAbc 			 	(void);
int 	GetFilename 		(void);
int 	GetReorder 		 	(void);
int 	LoadSupplierDetails (long, float);
int 	Process 			(char *);
int 	SrchFfwk 			(char *);
int 	SupplierSelect 		(char *);
int 	heading 			(int);
static 	float Rnd_Mltpl 	(float, char *, float, float);
void 	ClearMessage 		(void);
void 	CloseDB 			(void);
void 	DisplaySupplier 	(void);
void 	DrawBox 			(void);
void 	FreightDefault 		(void);
void 	LoadHhcc 			(void);
void 	LoadLocal 			(void);
void 	OpenDB 				(void);
void 	ProcessCalc 		(float *, float *);
void 	ProcessSelected 	(void);
void 	ProcessStack 		(void);
void 	ProgramTabScan 		(void);
void 	ReadMisc 			(void);
void 	SetPeriodHeader 	(char *);
void 	SupplierClear 		(void);
void 	SupplierStore 		(long, float, int);
void 	Update 				(void);
void 	UpdatePriorities 	(int);
void 	shutdown_prog 		(void);
void 	TagOther 			(void);

int		TabOpenSupplier	=	FALSE;
int		DoublePress	=	0;

/*
 * Main Processing Routine.
 */
int
main (
	int      argc,
	char	*argv [])
{
	char	*sptr;

	/*
	 * Set the maximum number of items	displayable. Each page 
	 * can hold 16 items.         
	 */
	tab_max_page = 1000;

	sptr = chk_env ("CURR_CODE");
	if (sptr == (char *) 0)
		sprintf (envCurrCode, "%-3.3s", "???");
	else
		sprintf (envCurrCode, "%-3.3s", sptr);

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envSupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envSupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envSupOrdRound, "D");
			break;

		default:
			sprintf (envSupOrdRound, "B");
			break;
		}
	}

	sptr = chk_env ("LRP_SHOW_DEMAND");
	if (sptr == (char *) 0)
		sprintf (envLrpShowDemand, "M");
	else
	{
		switch (*sptr)
		{
		case	'D':
		case	'd':
			sprintf (envLrpShowDemand, "D");
			break;

		case	'W':
		case	'w':
			sprintf (envLrpShowDemand, "W");
			break;

		default:
			sprintf (envLrpShowDemand, "M");
			break;
		}
	}

	sptr = chk_env ("LRP_INP_ZERO");
	if (sptr == (char *) 0)
		envLrpInpZero = 0;
	else
		envLrpInpZero = atoi (sptr);

	sptr = chk_env ("LRP_SHOW_ALL_SUP");
	if (sptr == (char *) 0)
		envLrpShowAllSup = 0;
	else
		envLrpShowAllSup = atoi (sptr);

	sptr = chk_env ("LRP_PRIORITY");
	if (sptr == (char *) 0)
		envLrpPriority = 0;
	else
		envLrpPriority = atoi (sptr);

	search_ok = TRUE;

	init_scr ();
	set_tty ();

	OpenDB ();

	SetPeriodHeader (envLrpShowDemand);
	
	sptr = chk_env ("CR_CO");
	if (sptr == (char *) 0)
		strcpy (branchNumber, " 0");
	else
		strcpy (branchNumber, (atoi (sptr)) ? comm_rec.est_no : " 0");

	DateToDMY (comm_rec.inv_date, NULL, &currentMonth, NULL);
	currentMonth--;

	swide ();

	heading (1);
	if (!GetAbc ())
	{
		shutdown_prog ();
		rset_tty ();
		return (EXIT_SUCCESS);
	}
	if (!GetReorder ())
	{
		shutdown_prog ();
		rset_tty ();
		return (EXIT_SUCCESS);
	}
	last_char = 0;

	if (!GetFilename ())
	{
		abc_unlock (ffwk);
		shutdown_prog ();
		rset_tty ();
		return (EXIT_SUCCESS);
	}

	abc_selfield (ffwk, "ffwk_id_no3");
	if (!tab_scan (lrpWork))
		Update ();

	tab_close (lrpWork, TRUE);

	abc_unlock (ffwk);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	ReadMisc ();

	abc_alias (inmr2, inmr);
	abc_alias (sumr2, sumr);
	abc_alias (ffwk2, ffwk);
	abc_alias (inis2, inis);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ffwk,  ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no");
	open_rec (ffwk2, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no3");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no3");
	open_rec (inld,  inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (pocf,  pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
	open_rec (suds,  suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec (insp,  insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (soic,  soic_list, soic_no_fields, "soic_id_no2");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ffwk);
	abc_fclose (ffwk2);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inis);
	abc_fclose (inis2);
	abc_fclose (inld);
	abc_fclose (itln);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (pocf);
	abc_fclose (pocr);
	abc_fclose (podt);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (suds);
	abc_fclose (insp);
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	hhccHash = ccmr_rec.hhcc_hash;

	abc_fclose (ccmr);

	open_rec (comr, comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr, &comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

	lsystemDate = TodaysDate ();
	DateToDMY (lsystemDate, NULL, &currMonth, NULL);
	currDay = lsystemDate % 7;  
	
}

/*
 * Process all records in ffwk with Appropriate filename & warehouse	
 */
int
Process (
	char	*recBuffer)
{
	int		first_time = TRUE;
	float	qty_ordered;
	float	ord_multiple;
	float	wk_qty;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", recBuffer + 3);
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		return (FALSE);

	sprintf (ffwk_rec.filename,"%-14.14s",LRP_FileName);
	ffwk_rec.hhcc_hash = hhccHash;
	ffwk_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (ffwk2, &ffwk_rec, GTEQ, "r");
	if
	(
		cc ||
		ffwk_rec.hhcc_hash != hhccHash ||
		ffwk_rec.hhbr_hash != inmr_rec.hhbr_hash
	)
		return (FALSE);

	LRP_MinOrderQty = 0.00;
	LRP_NorOrderQty = 0.00;

	heading (1);

	cc = DisplayEntry (first_time);
	if (cc == NO_SUPPLIER)
	{
		ffwk_rec.order_qty = 0;
		return (NO_SUPPLIER);
	}
	if (ffwk_rec.hhpo_hash == 0L && !cc)
	{
		first_time = 0;
		print_at (2,0,"         ");
		fflush (stdout);

		/*
		 * Display suppliers for item	
		 */
		DisplaySupplier ();

		LRP_MinOrderQty = atof (SR [LRP_SupplierIndex].moq);
		LRP_NorOrderQty = atof (SR [LRP_SupplierIndex].noq);

		print_at (6,27,"%8.2f",LRP_MinOrderQty);

		print_at (7,27,"%8.2f",LRP_NorOrderQty);

		crsr_on ();
		do
		{
		    /*
		     * Read Qty ordered	
		     */
		    qty_ordered = getfloat (27,9,"NNNNN.NN");

		    ClearMessage ();

		    switch (last_char)
		    {
				case	REDRAW:
					crsr_off ();
					DrawBox ();
					DisplayEntry (0);
					crsr_on ();
				break;

				case	EOI:
				case	RESTART:
					abc_unlock (ffwk);
					return (FALSE);

				default:
				break;
		    }

		    if (dflt_used)
		    {
				qty_ordered = LRP_QtyRecommended;
				print_at (9,27, "%8.2f",qty_ordered);
		    }

		    if (qty_ordered < 0.00)
		    {
				sprintf (err_str, "%5.5s%18.18s%5.5s", 
									"---> ", 
									ML (mlLrpMess047),
									" <---"); 
				rv_pr (err_str,0,2,1);
				qty_ordered = 0.00;
				last_char = 0;
		    }

		    if (qty_ordered != 0.00 && qty_ordered < LRP_MinOrderQty)
		    {
				/*
				 * ---> Qty Ordered < Minimum Order Qty <--- 
				 */
				sprintf (err_str, "%5.5s%31.31s%5.5s", 
											"---> ", ML (mlLrpMess048), " <---"); 
				rv_pr (err_str, 0,2,1);
				last_char = 0;
		    }
		} while (last_char != ENDINPUT);

		crsr_off ();
		ClearMessage ();


		print_at (2,0, ML (mlStdMess035));
		fflush (stdout);

		/*
		 * Update ffwk with new supplier and qty ordered			        
		 */
		if (NoSuppTabLines >= 0)
		{
			ffwk_rec.hhsu_hash = SR [LRP_SupplierIndex].hhsuHash;
			sprintf (ffwk_rec.crd_no, "%-6.6s", SR [LRP_SupplierIndex].supplierNo + 1);
			sprintf (sumr_rec.crd_no, "%-6.6s", SR [LRP_SupplierIndex].supplierNo + 1);

			ord_multiple = atof (SR [LRP_SupplierIndex].oml);
			wk_qty = qty_ordered;
			/*
			 * Force qty_ordered to the next multiple of ord_multiple
			 * (If ord_multiple != 0.00)	  
			 */
			if (qty_ordered)
			{
				qty_ordered = 	Rnd_Mltpl 
								(
									qty_ordered, 
									envSupOrdRound, 
							  		ord_multiple, 
									LRP_MinOrderQty
								);
			}
			ffwk_rec.order_qty = qty_ordered;
		}
		/*
		 * Clear old qty & look like something is happening 
		 */
		print_at (9,27,"_____.__");
		print_at (2,0, ML (mlStdMess035));
		fflush (stdout);
	}
	if (cc == OTHER)
	{
		/*
		 * ---"Regeneration of suggested order recommended"---
		 */
		print_mess (ML (mlLrpMess002));
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
	}
	return (TRUE);
}

void
ClearMessage (void)
{
	move (0,2);
	cl_line ();
}

/*
 * Search for Database filename	
 */
int
SrchFfwk (
 char*          key_val)
{
	char	last_name [15];

	work_open ();
	save_rec ("#Filename","#Review Period");
	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename,"%-14.14s",key_val);
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"r");
	strcpy (last_name,"              ");
	while (!cc && !strncmp (ffwk_rec.filename,key_val,strlen (key_val)) && 
				   ffwk_rec.hhcc_hash == hhccHash)
	{
		if (!strncmp (ffwk_rec.filename, "TRN", 3))
		{
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
			continue;
		}
		if (strcmp (last_name,ffwk_rec.filename))
		{
			sprintf (err_str,"%6.2f Weeks ",ffwk_rec.review_pd);
			cc = save_rec (ffwk_rec.filename,err_str);
			if (cc)
				break;
		}

		strcpy (last_name,ffwk_rec.filename);
		cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
    return (EXIT_SUCCESS);
}

/*
 * return maximum of a,b,or c	
 */
float
TrigMax (
 float      a,
 float      b,
 float      c)
{
	if (a > b)
		if (a > c)
			return (a);
		else
			return ( (c > b) ? c : b);
	else
		if (b > c)
			return (b);
		else
			return ( (c > a) ? c : a);
}

/*
 * Print an entry for a item_no / supplier	
 */
int
DisplayEntry (
 int            _draw_box)
{
	char	*sptr;
	float	minStockQty	=	0.00;
	float	minStockWks	=	0.00;
	float	safety_stock,
			cover,
			wks_avail,
			wks_on_order,
			wks_net_reqt,
			wks_cover_reqd,
			qty_avail,
			qty_on_order,
			qty_tot_cover,
			qty_cover_reqd,
			qty_net_reqt;
			


	wks_tot_cover	=	0.00;
	if (ffwk_rec.hhpo_hash != 0L)
	{
		print_mess (ML (mlLrpMess003));
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
	}

	if (ffwk_rec.wks_demand <= 0.00)
	{
		print_mess (ML (mlLrpMess004));
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
	}

	inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess001));
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
	}

	minStockQty	=	inmr_rec.min_quan;
	safety_stock = inmr_rec.safety_stock;
	strcpy (local_rec.abcCode, inmr_rec.abc_code);

	if (LRP_ByBranch)
	{
		inei_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			fflush (stdout);
			sleep (sleepTime);	
			return (OTHER);
		}

		minStockQty	=	inei_rec.min_stock;
		safety_stock = inei_rec.safety_stock;
		strcpy (local_rec.abcCode, inei_rec.abc_code);
	}

	if (LRP_ByWarehouse)
	{
		incc_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		incc_rec.hhcc_hash = hhccHash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			fflush (stdout);
			sleep (sleepTime);	
			return (OTHER);
		}

		safety_stock = incc_rec.safety_stock;
		strcpy (local_rec.abcCode, incc_rec.abc_code);
	}

	if (ffwk_rec.wks_demand != 0.0)
		minStockWks = minStockQty / ffwk_rec.wks_demand;
	else
		minStockWks = 0.0;

	useMinStockWks = FALSE;
	if (minStockWks > safety_stock)
		useMinStockWks = TRUE;

	/*
	 * Non Stock Item	
	 */
	if (inmr_rec.inmr_class [0] == 'N')
	{
		print_mess (ML (mlLrpMess046));
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
	}

	maxLines = LoadSupplierDetails
				(
					ffwk_rec.hhbr_hash, 
			 		(ffwk_rec.order_qty > 0.00) ? ffwk_rec.order_qty
												   : ffwk_rec.sugg_qty
				);
	if (!maxLines)
	{
		print_mess (ML (mlLrpMess044));
		fflush (stdout);
		sleep (sleepTime);	
		return (NO_SUPPLIER);
	}

	if (useMinStockWks)
		cover = ffwk_rec.review_pd + LRP_LeadWeeks + minStockWks;
	else
		cover = ffwk_rec.review_pd + LRP_LeadWeeks + safety_stock;

	ProcessCalc (&qty_avail, &qty_on_order);

	qty_tot_cover	= qty_avail + qty_on_order;

	qty_cover_reqd	= cover;
	qty_cover_reqd	*= ffwk_rec.wks_demand;

	qty_net_reqt	= qty_cover_reqd - qty_tot_cover;

	if (qty_net_reqt < 0.00)
		qty_net_reqt = 0.00;

	wks_avail		= qty_avail     / ffwk_rec.wks_demand;
	wks_on_order	= qty_on_order  / ffwk_rec.wks_demand;
	wks_tot_cover	= qty_tot_cover / ffwk_rec.wks_demand;
	wks_cover_reqd	= cover;
	wks_net_reqt	= qty_net_reqt  / ffwk_rec.wks_demand;

	if (qty_net_reqt == 0.00)
		LRP_QtyRecommended	= 0.00;
	else
		LRP_QtyRecommended	= TrigMax (qty_net_reqt, LRP_MinOrderQty, LRP_NorOrderQty);

	if (LRP_QtyRecommended == 0.00 && !envLrpInpZero)
	{
		/*---"Zero order recommended for item"---*/
		print_mess (ML (mlLrpMess045));
		fflush (stdout);
		sleep (sleepTime);	
	}

	/*
	 * Check if Item is to be entered	
	 */
	sptr = strchr (LRP_ABCCODES,local_rec.abcCode [0]);

	if (sptr == (char *)0)
		return (ABC_CODE);

	if (inmr_rec.reorder [0] == 'N' && LRP_ReorderFlag [0] == 'N')
		return (REORDER);

	if (_draw_box)
		DrawBox ();

	print_at (3,12,inmr_rec.item_no);
	print_at (3,30,inmr_rec.description);
	print_at (3,80,inmr_rec.sale_unit);

	print_at (5,99, "%6.2f      %10.2f",wks_avail,qty_avail);
	print_at (6,99, "%6.2f      %10.2f",wks_on_order,qty_on_order);
	print_at (7,99, "%6.2f      %10.2f",wks_tot_cover,qty_tot_cover);
	print_at (8,99, "%6.2f      %10.2f",wks_cover_reqd,qty_cover_reqd);
	print_at (9,99, "%6.2f      %10.2f",wks_net_reqt,qty_net_reqt);
	print_at (5,62, "%6.2f ",	ffwk_rec.review_pd);
	print_at (6,62, "%6.2f ",	LRP_LeadDays);

	if (useMinStockWks)
		print_at (7,62, "%6.2f ",minStockWks);
	else
		print_at (7,62, "%6.2f ",safety_stock);

	print_at (9,62, "%6.2f ",cover);
	fflush (stdout);

	print_at (5,27, "%10.2f",ffwk_rec.wks_demand);
	print_at (6,27, "%10.2f",0.00);
	print_at (7,27, "%10.2f",0.00);
	print_at (8,27, "%10.2f",LRP_QtyRecommended);
	fflush (stdout);
	return (EXIT_SUCCESS);
}

void
ProcessCalc (
 float*         qty_avail,
 float*         qty_on_order)
{
	int		indx = 0;
	float	realCommitted;

	*qty_avail = 0.00;
	*qty_on_order = 0.00;

	if (LRP_ByCompany)
	{
		realCommitted = RealTimeCommitted (inmr_rec.hhbr_hash, 0L);

	    *qty_avail	= inmr_rec.on_hand -
			  		  inmr_rec.committed - 
			  		  realCommitted - 
			  		  inmr_rec.backorder - 
			  		  inmr_rec.forward;

	    poln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	    poln_rec.due_date = 0L;
	    cc = find_rec (poln, &poln_rec, GTEQ, "r");
	    while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	    {
			if (poln_rec.qty_ord > poln_rec.qty_rec)
				cc = find_hash (pohr, &pohr_rec, EQUAL, "r", 
					poln_rec.hhpo_hash);
				if (pohr_rec.drop_ship [0] != 'Y')
					*qty_on_order += (poln_rec.qty_ord 
						- poln_rec.qty_rec);

			cc = find_rec (poln, &poln_rec, NEXT, "r");
	    }

		/*
		 * Add transfers to qty_on_order. 
		 */
		memset (&itln_rec, 0, sizeof (itln_rec));
		itln_rec.hhbr_hash = inmr_rec.hhbr_hash;
		for (cc = find_rec (itln, &itln_rec, GTEQ, "r");
			 !cc && 
			  itln_rec.hhbr_hash == inmr_rec.hhbr_hash;
			 cc = find_rec (itln, &itln_rec, NEXT, "r"))
		{
			switch (itln_rec.status [0])
			{
			case	'B':
			case	'M':
			case	'T':
			case	'U':
				*qty_on_order += itln_rec.qty_order;
				*qty_on_order += itln_rec.qty_border;
				break;

			default:
				break;
			}
		}
	    return;
	}

	if (!LRP_ByCompany)
	{
	    while (indx <= MAX_WH && validWh [indx] != 0L)
	    {
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			incc_rec.hhcc_hash = validWh [indx];
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			if (!cc)
			{
				realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
													incc_rec.hhcc_hash);

				*qty_avail		+= incc_rec.closing_stock;
				*qty_avail		-= incc_rec.committed;
				*qty_avail		-= realCommitted;
				*qty_avail		-= incc_rec.backorder;
				*qty_avail		-= incc_rec.forward;
				*qty_on_order	+= incc_rec.on_order;
			}
			indx++;
	    }
	}
}

/*
 * Draw fancy box & display prompts	
 */
void
DrawBox (void)
{
	heading (2);

	/*
	 * Item :						
	 * %R Weeks       Quantity 	
	 * Available       :	
	 * On Order        :
	 * Total Cover     :
	 * Cover Required  :
	 * Net Requirement :
	 * Review Period:  +          Weeks	
	 * Lead Time    :  +          Weeks	
	 * Safety Stock :             Weeks	
	 */
	print_at (3,4, ML (mlLrpMess006));
	print_at (3,28,"-");

	box (0, 4, 130, 5);
	print_at (4,100, ML (mlLrpMess007)); 
	print_at (5,80, ML (mlLrpMess008)); print_at (5,96, ":");
	print_at (6,80, ML (mlLrpMess009)); 
	print_at (7,80, ML (mlLrpMess010));
	print_at (8,80, ML (mlLrpMess011));
	print_at (9,80, ML (mlLrpMess012));
	print_at (5,43, ML (mlLrpMess013));
	print_at (6,43, ML (mlLrpMess033));

	if (useMinStockWks)
		print_at (7,43, ML (mlLrpMess073));
	else
		print_at (7,43, ML (mlLrpMess015));

	line_at (8,43,33);
	/*
	 * Cover :                    Weeks
	 * Weeks Demand    : 
	 * Min. Ord. Qty   :
	 * Norm. Ord. Qty  :
	 * Qty Recommended :
	 * Qty Ordered     :
	 */
	print_at (9,43, ML (mlLrpMess016));
	print_at (5,6, ML (mlLrpMess017));
	print_at (6,6, ML (mlLrpMess018));
	print_at (7,6, ML (mlLrpMess019));
	print_at (8,6, ML (mlLrpMess020));
	print_at (9,6, ML (mlLrpMess021));

}

/*
 * Display Valid Suppliers	
 */
void
DisplaySupplier (void)
{
	int		i;
	char	disp_str [200];

	DoublePress	=	0;
	/*
	 * Calculate number of lines to display		
	 * - greater of last # displayed & current
	 */

	NoSuppTabLines	=	0;

	if (TabOpenSupplier)
		tab_close (supplierWork, TRUE);
		
	tab_open (supplierWork, Supplier_keys, 11, 0, 8, FALSE);

	TabOpenSupplier	=	TRUE;

	strcpy (disp_str,"#Pi|Br|Wh|Supplier|Supplier Item No|Lead tm|Curr|Cnty|  FOB Cost  | Duty %% |Fri + Cont.|In Store Cst|   MOQ   |   NOQ   |Ord Mult.");
	tab_add 
	(
		supplierWork,
		disp_str
	);
	for (i = 0;i < maxLines;i++)
	{
		sprintf (disp_str, "%2.2s|%2.2s|%2.2s|%8.8s|%16.16s|%7.7s|%4.4s|%4.4s|%12.12s|%8.8s|%11.11s|%12.12s|%9.9s|%9.9s|%9.9s",
			SR [i].priority,
			SR [i].brNo,
			SR [i].whNo,
			SR [i].supplierNo,
			SR [i].itemNo,
			SR [i].lead,
			SR [i].curr,
			SR [i].ctry,
			SR [i].fobCost,
			SR [i].duty,
			SR [i].frt,
			SR [i].intoStore,
			SR [i].moq,
			SR [i].noq,
			SR [i].oml);

		tab_add 
		(
			supplierWork,
			disp_str
		);
		if (SR [i].supplierNo [0] == '*')
			LRP_PriOneSupplier	=	NoSuppTabLines;

		NoSuppTabLines++;
	}
	if (NoSuppTabLines > 0)
		tab_scan (supplierWork);
	else
	{

		/*
		 * No packing slips found. 
		 */
		tab_add (supplierWork, "  ************  NO VALID LINES CAN BE LOADED  ************");
		tab_display (supplierWork, TRUE);
		putchar (BELL);
		fflush (stdout);
		sleep (sleepTime);
		return;
	}
	if (!prog_exit)
		ProcessSelected ();
}

/*
 * updates details 
 */
void
ProcessSelected (void)
{
	int	i;

	/*
	 * Process all tagged lines 
	 */
	for (i = 0; i < NoSuppTabLines; i++)
	{
		tab_get (supplierWork, LRP_SuppBuffer, EQUAL, i);
	   	if (!tagged (LRP_SuppBuffer))
			continue;
		
		redraw_line (supplierWork, TRUE);
		tag_unset (supplierWork);

		/*
		 * Find cuhd record. 
		 */
		redraw_line (supplierWork, FALSE);
	}
	return;
}

int
SupplierSelect (
	char	*find_key)
{
	LRP_SupplierIndex =	atoi (find_key);
    return (EXIT_SUCCESS);
}


/*
 * Load supplier details into table	
 */
int
LoadSupplierDetails (
	long       hhbrHash,
	float      QTY)
{
	int		lowestSupplier	=	3;
	int		RecordNumber	=	0;
	int		i;

	LRP_OrderSupplier		=	0;
	
	SupplierClear ();

	inis_rec.hhbr_hash	=	hhbrHash;
	strcpy (inis_rec.sup_priority, "W ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis,&inis_rec,GTEQ,"r");
	while (!cc && inis_rec.sup_priority [0] == 'W' &&
				   inis_rec.hhbr_hash == hhbrHash	)
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
	    cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
	    {
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}
		if (!envLrpShowAllSup && 
			(strcmp (inis_rec.co_no, comm_rec.co_no) ||
			 strcmp (inis_rec.br_no, comm_rec.est_no) ||
			 strcmp (inis_rec.wh_no, comm_rec.cc_no)))
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}

		SupplierStore
		(
			hhbrHash,
			QTY,
			RecordNumber++
		);
	    cc = find_rec (inis,&inis_rec,NEXT,"r");
	}
	inis_rec.hhbr_hash	=	hhbrHash;
	strcpy (inis_rec.sup_priority, "B ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis,&inis_rec,GTEQ,"r");
	while (!cc && inis_rec.sup_priority [0] == 'B' &&
				   inis_rec.hhbr_hash == hhbrHash	)
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
	    cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
	    {
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}
		if (!envLrpShowAllSup && 
			(strcmp (inis_rec.co_no, comm_rec.co_no) ||
			strcmp (inis_rec.br_no, comm_rec.est_no)))
		{
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}

		SupplierStore
		(
			hhbrHash,
			QTY,
			RecordNumber++
		);
	    cc = find_rec (inis,&inis_rec,NEXT,"r");
	}
	inis_rec.hhbr_hash	=	hhbrHash;
	strcpy (inis_rec.sup_priority, "C ");
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis,&inis_rec,GTEQ,"r");
	while (!cc && inis_rec.sup_priority [0] == 'C' &&
				   inis_rec.hhbr_hash == hhbrHash	)
	{
		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
	    cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
	    {
			cc = find_rec (inis, &inis_rec, NEXT, "r");
			continue;
		}

		SupplierStore
		(
			hhbrHash,
			QTY,
			RecordNumber++
		);
	    cc = find_rec (inis,&inis_rec,NEXT,"r");
	}

	lowestSupplier =	9;
	for (i = 0; i < RecordNumber; i++)
	{
		if (SR [i].firstSupplier)
		{
			if (SR [i].firstSupplier < lowestSupplier)
			{
				LRP_OrderSupplier	=	i;
				lowestSupplier		=	SR [i].firstSupplier;
			}
		}
	}
	SR [LRP_OrderSupplier].supplierNo [0] = '*';
	LRP_MinOrderQty	= (float) atof (SR [LRP_OrderSupplier].moq);
	LRP_NorOrderQty	= (float) atof (SR [LRP_OrderSupplier].noq);
	LRP_LeadDays	= (float) atof (SR [LRP_OrderSupplier].lead);
	LRP_LeadWeeks	= LRP_LeadDays / 7;
	return (RecordNumber);
}

void
SupplierClear (void)
{
	int		i;

	/*
	 * Only clear those lines which were used last time	
	 */
	for (i  = 0;i < MAX_SUPPLY;i++)
	{
		if (i == 0)
		{
			wk_rec.hhsuHash 		=	sumr_rec.hhsu_hash;
			wk_rec.hhisHash 		=	0L;
			wk_rec.firstSupplier	=	0;
			sprintf (wk_rec.brNo,		"%-2.2s",	" ");
			sprintf (wk_rec.whNo,		"%-2.2s",	" ");
			sprintf (wk_rec.priority,	"%-2.2s",	" ");
			sprintf (wk_rec.supplierNo,		"%-8.8s",	" ");
			sprintf (wk_rec.itemNo,	"%-16.16s",	" ");
			sprintf (wk_rec.lead,		"%-7.7s",	" ");
			sprintf (wk_rec.curr,		"%-5.5s",	" ");
			sprintf (wk_rec.ctry,		"%-5.5s",	" ");
			sprintf (wk_rec.fobCost,	"%-12.12s",	" ");
			sprintf (wk_rec.duty,		"%-8.8s",	" ");
			sprintf (wk_rec.frt,		"%-11.11s",	" ");
			sprintf (wk_rec.intoStore,	"%-12.12s",	" ");
			sprintf (wk_rec.moq,		"%-9.9s",	" ");
			sprintf (wk_rec.noq,		"%-9.9s",	" ");
			sprintf (wk_rec.oml,		"%-9.9s",	" ");
			wk_rec.wkValue = 9999999.99;
		}
		SR [i].wkValue			=	9999999.99;
		SR [i].hhsuHash 		=	0L;
		SR [i].firstSupplier	=	0;
		sprintf (SR [i].brNo,		"%-2.2s",	" ");
		sprintf (SR [i].whNo,		"%-2.2s",	" ");
		sprintf (SR [i].priority,	"%-2.2s",	" ");
		sprintf (SR [i].supplierNo,		"%-8.8s",	" ");
		sprintf (SR [i].itemNo,	"%-16.16s",	" ");
		sprintf (SR [i].lead,		"%-7.7s",	" ");
		sprintf (SR [i].curr,		"%-5.5s",	" ");
		sprintf (SR [i].ctry,		"%-5.5s",	" ");
		sprintf (SR [i].fobCost,	"%-12.12s",	" ");
		sprintf (SR [i].duty,		"%-8.8s",	" ");
		sprintf (SR [i].frt,		"%-11.11s",	" ");
		sprintf (SR [i].intoStore,"%-12.12s",	" ");
		sprintf (SR [i].moq,		"%-9.9s",	" ");
		sprintf (SR [i].noq,		"%-9.9s",	" ");
		sprintf (SR [i].oml,		"%-9.9s",	" ");
	}
}

void
SupplierStore (
 long       hhbrHash,
 float      QTY,
 int        RECNO)
{	
	float		discArray [4];	/* Regulatory and Disc A, B, C percents */
	int			cumulative;

	double	fobCost    = 0.00;
	double	cif_cost    = 0.00;
	double	contingency = 0.00;
	double	duty        = 0.00;
	float	duty_pc     = 0.00;

	if (inis_rec.lead_time == 0.00)
		inis_rec.lead_time = GetLeadDate 
							(
								inis_rec.hhis_hash, 
								comm_rec.inv_date
							);

	fobCost	= GetSupPrice
				(
						sumr_rec.hhsu_hash,
						hhbrHash, 
						inis_rec.fob_cost, 
						QTY
				);
				
	cumulative  = GetSupDisc
				(
						sumr_rec.hhsu_hash,
						inmr_rec.buygrp,
						QTY,
						discArray 
				);

	fobCost    = CalcNet
				(
						fobCost , 
						discArray, 
						cumulative
				);

	strcpy (pocrRec.co_no,comm_rec.co_no);
	strcpy (pocrRec.code,sumr_rec.curr_code);
	cc = find_rec (pocr,&pocrRec,COMPARISON,"r");
	if (cc || pocrRec.ex1_factor == 0.00)
		pocrRec.ex1_factor = 1.00;

	fobCost /= pocrRec.ex1_factor;

	strcpy (podt_rec.co_no,comm_rec.co_no);
	strcpy (podt_rec.code,inis_rec.duty);
	cc = find_rec (podt,&podt_rec,COMPARISON,"r");
	if (!cc)
	{
		if (podt_rec.duty_type [0] == 'P')
		{
			duty_pc = podt_rec.im_duty;
			duty = DOLLARS (duty_pc) * fobCost;
		}
		else
		{
			duty = podt_rec.im_duty;
			if (duty + fobCost != 0.00)
				duty_pc = duty / (duty + fobCost);
		}
	}
	else
	{
		duty = 0.00;
		duty_pc = 0.00;
	}

	FreightDefault ();

	cif_cost = fobCost + duty + freight;

	contingency = DOLLARS (comr_rec.contingency);
	contingency *= cif_cost;

	if (!envLrpPriority)
	{
		wk_rec.hhsuHash = sumr_rec.hhsu_hash;
		wk_rec.hhisHash = inis_rec.hhis_hash;
		if ( (cif_cost + contingency) <= 0.00)
			wk_rec.wkValue = 999999.99;
		else
			wk_rec.wkValue = cif_cost + contingency;

		wk_rec.firstSupplier	=	0;

		if (inis_rec.sup_priority [1] == '1' && 
			inis_rec.sup_priority [0] == 'W' &&
			!strcmp (inis_rec.co_no, comm_rec.co_no) &&
			!strcmp (inis_rec.br_no, comm_rec.est_no) &&
			!strcmp (inis_rec.wh_no, comm_rec.cc_no))
			wk_rec.firstSupplier	=	1;

		if (inis_rec.sup_priority [1] == '1' && 
			inis_rec.sup_priority [0] == 'B' &&
			!strcmp (inis_rec.co_no, comm_rec.co_no) &&
			!strcmp (inis_rec.br_no, comm_rec.est_no))
			wk_rec.firstSupplier	=	2;

		if (inis_rec.sup_priority [1] == '1' && inis_rec.sup_priority [0] == 'C')
			wk_rec.firstSupplier	=	3;

		sprintf (wk_rec.priority,	"%-2.2s", 	inis_rec.sup_priority);
		sprintf (wk_rec.brNo,		"%-2.2s", 	inis_rec.br_no);
		sprintf (wk_rec.whNo,		"%-2.2s", 	inis_rec.wh_no);
		sprintf (wk_rec.supplierNo,		" %-6.6s ", sumr_rec.crd_no);
		sprintf (wk_rec.itemNo,	"%s",		 inis_rec.sup_part);
		sprintf (wk_rec.lead,		"%7.3f", 	inis_rec.lead_time);
		sprintf (wk_rec.curr,		" %-3.3s ", sumr_rec.curr_code);
		sprintf (wk_rec.ctry,		" %-3.3s ", sumr_rec.ctry_code);
		sprintf (wk_rec.fobCost,	"%11.4f ",  fobCost);
		sprintf (wk_rec.duty,		" %6.2f ", 	duty_pc);
		sprintf (wk_rec.frt,		" %9.2f ", 	freight + contingency);
		sprintf (wk_rec.intoStore,	"%11.4f ",  cif_cost + contingency);
		sprintf (wk_rec.moq,		"%8.2f ", 	inis_rec.min_order);
		sprintf (wk_rec.noq,		"%8.2f ", 	inis_rec.norm_order);
		sprintf (wk_rec.oml,		"%8.2f ", 	inis_rec.ord_multiple);
		ProcessStack ();
	}
	else
	{
		SR [RECNO].hhsuHash		= sumr_rec.hhsu_hash;
		SR [RECNO].hhisHash		= inis_rec.hhis_hash;

		SR [RECNO].firstSupplier	= 0;
		if (inis_rec.sup_priority [1] == '1' && 
			inis_rec.sup_priority [0] == 'W' &&
			!strcmp (inis_rec.co_no, comm_rec.co_no) &&
			!strcmp (inis_rec.br_no, comm_rec.est_no) &&
			!strcmp (inis_rec.wh_no, comm_rec.cc_no))
				SR [RECNO].firstSupplier	= 1;

		if (inis_rec.sup_priority [1] == '1' && 
			inis_rec.sup_priority [0] == 'B' &&
			!strcmp (inis_rec.co_no, comm_rec.co_no) &&
			!strcmp (inis_rec.br_no, comm_rec.est_no))
			SR [RECNO].firstSupplier	= 2;

		if (inis_rec.sup_priority [1] == '1' && inis_rec.sup_priority [0] == 'C')
			SR [RECNO].firstSupplier	= 3;

		sprintf (SR [RECNO].priority,	"%-2.2s", 	inis_rec.sup_priority);
		sprintf (SR [RECNO].brNo,		"%-2.2s", 	inis_rec.br_no);
		sprintf (SR [RECNO].whNo,		"%-2.2s", 	inis_rec.wh_no);
		sprintf (SR [RECNO].supplierNo,  	" %-6.6s ", sumr_rec.crd_no);
		sprintf (SR [RECNO].itemNo, 	"%s", 		inis_rec.sup_part);
		sprintf (SR [RECNO].lead,	  	"%7.3f", 	inis_rec.lead_time);
		sprintf (SR [RECNO].curr,	  	" %-3.3s ",	sumr_rec.curr_code);
		sprintf (SR [RECNO].ctry,	  	" %-3.3s ",	sumr_rec.ctry_code);
		sprintf (SR [RECNO].fobCost,	"%11.4f ", fobCost);
		sprintf (SR [RECNO].duty,	  	" %6.2f ", 	duty_pc);
		sprintf (SR [RECNO].frt,	  	" %9.2f ", 	freight + contingency);
		sprintf (SR [RECNO].intoStore,	"%11.4f ", cif_cost + contingency);
		sprintf (SR [RECNO].moq,	  	"%8.2f ", 	inis_rec.min_order);
		sprintf (SR [RECNO].noq,	  	"%8.2f ", 	inis_rec.norm_order);
		sprintf (SR [RECNO].oml,	  	"%8.2f ",	inis_rec.ord_multiple);
	}
}

void
FreightDefault (void)
{
	double	frt_conv = 0.00;

	freight = 0.00;

	strcpy (pocf_rec.co_no,comm_rec.co_no);
	strcpy (pocf_rec.code,sumr_rec.ctry_code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
		return;

	/*
	 * Calculate Freight	
	 */
	frt_conv = pocf_rec.freight_load;

	if (pocf_rec.load_type [0] == 'U')
		freight = frt_conv;

	if (pocf_rec.load_type [0] == 'P')
		freight = (inis_rec.fob_cost * frt_conv) / 100;

	freight /= pocrRec.ex1_factor;
	return;
}

int
GetAbc (void)
{
	while (TRUE)
	{
		int	decreasing,
			num;
		/*
		 * Any combination of A, B, C and D ascending
		 * ABC Codes (Requiring Input) : 
		 */
		print_mess (ML (mlLrpMess005));
		print_at (4,11, ML (mlLrpMess052));
		getalpha (45,4,"UUUU",temp_str);
		switch (last_char)
		{
		case	REDRAW:
			heading (1);
			break;

		case	RESTART:
			return (EXIT_SUCCESS);

		default:
			break;
		}
		num = strlen (temp_str);
		decreasing = num - 1;
		while (decreasing > 0)
			if (temp_str [decreasing] <= temp_str [decreasing - 1])
				break;
			else
				decreasing--;
		if (!num || temp_str [0] < 'A' || 
			'D' < temp_str [num - 1] || decreasing)
			continue;

		sprintf (LRP_ABCCODES,"%-4.4s",temp_str);

		if (last_char == EOI || last_char == ENDINPUT)
			break;
	}

	print_at (4,45, "%-4.4s",LRP_ABCCODES);
	clear_mess ();
	fflush (stdout);

	return (EXIT_FAILURE);
}

int
GetReorder (void)
{
	while (TRUE)
	{
		print_mess (ML ("Include items set to reorder No"));
		print_at (6,11, ML ("Reorder items set to no reorder Y(es) N(o)"));
		getalpha (55,6,"U",temp_str);
		switch (last_char)
		{
		case	REDRAW:
			heading (1);
			break;

		case	RESTART:
			return (EXIT_SUCCESS);

		default:
			break;
		}
		sprintf (LRP_ReorderFlag,"%-1.1s",temp_str);

		if (last_char == EOI || last_char == ENDINPUT)
			break;
	}

	print_at (6,55, "%-1.1s",LRP_ReorderFlag);
	clear_mess ();
	fflush (stdout);

	return (EXIT_FAILURE);
}

/*
 * Read "filename" that data was filed under on database	
 */
int
GetFilename (void)
{
	int		i,
			first_time = TRUE;

	float	LRP_LeadWeeks 	= 0.00;

	float	cvr_prd,
			last_6,
			last_12;

	int		NoData	=	TRUE;

	LRP_LinesProcessed	=	0;
	LRP_NoRecords		=	0;

	while (TRUE)
	{
		/*---" Filename : "---*/
	    print_at (8,11, ML (mlLrpMess057));
	    getalpha (45,8,"UUUUUUUUUUUUUU",temp_str);
	    switch (last_char)
	    {
			case	SEARCH:
				SrchFfwk (temp_str);
				last_char = REDRAW;

			case	REDRAW:
				/*
				 * ABC Codes (Requiring Input) : 
				 */
				heading (1);
				print_at (4,10, " %s ", ML (mlLrpMess052));
				print_at (4,45, "%-4.4s",LRP_ABCCODES);
				print_at (6,11, ML ("Reorder items set to no reorder Y(es) N(o)"));
				print_at (6,55, "%-1.1s",LRP_ReorderFlag);
				fflush (stdout);
			break;

			case	RESTART:
				return (EXIT_SUCCESS);

			default:
				break;
	    }
		if (!strncmp (LRP_FileName, "TRN", 3))
			strcpy (LRP_FileName, "Not Allowed");
		
	    sprintf (LRP_FileName,"%-14.14s",temp_str);

	    if (last_char == EOI || last_char == ENDINPUT)
	    {
			ffwk_rec.hhcc_hash = hhccHash;
			sprintf (ffwk_rec.filename,"%-14.14s",LRP_FileName);
			cc = find_rec (ffwk,&ffwk_rec,COMPARISON,"w");
			if (cc)
				print_mess (ML (mlLrpMess039));
			else
				break;
	    }
	}

	tab_open (lrpWork, Header_keys, 2, 0, WK_DEPTH, FALSE);
	tab_add 
	(
		lrpWork, 
		"#St Item Number      Description                  Cvr Last 12  Last 6    %-7.7s%-7.7s%-7.7s%-7.7s%-7.7s%-3.3s   Suppl. Sugg. Qty ",
		periodHead [5],
		periodHead [4],
		periodHead [3],
		periodHead [2],
		periodHead [1],
		periodHead [0]
	);
/*
			mth_nam [ (currentMonth + 7) % 12],
			mth_nam [ (currentMonth + 8) % 12],
			mth_nam [ (currentMonth + 9) % 12],
			mth_nam [ (currentMonth + 10) % 12],
			mth_nam [ (currentMonth + 11) % 12],
			mth_nam [ (currentMonth)]);
*/

	abc_selfield (ffwk, "ffwk_sort_id");
	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	sprintf (ffwk_rec.sort, "%-34.34s", " ");
	cc = find_rec (ffwk, &ffwk_rec, GTEQ, "r");
	while (!cc && ffwk_rec.hhcc_hash == hhccHash &&
	    		   !strcmp (ffwk_rec.filename,LRP_FileName))
	{
	    if (first_time)
	    {
			first_time = FALSE;
			switch (ffwk_rec.source [0])
			{
			case	'W':
				LRP_ByWarehouse = TRUE;
				break;

			case	'B':
				LRP_ByBranch = TRUE;
				break;

			default:
				LRP_ByCompany = TRUE;
				break;
			}
			LoadHhcc ();
	    }

	    /*
	     * If we come across an ffwk	record which refers to an
	     * inmr which has since been	deleted, IGNORE IT!!	
	     */
		inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
	    {
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
			continue;
	    }

	    LoadLocal ();

	    last_6 = last_12 = (float) 0.00;
	    for (i = 0; i < 12; i++)
	    {
			last_12 += ffwk_cons [i];
			if (i < 6)
				last_6 += ffwk_cons [ (i + currentMonth + 7) % 12];
	    }

		/*
		 * if a purchased product, show supplier, otherwise show  
		 * ManProd/ManCom                                        
		 */
		if (!strcmp (inmr_rec.source, "MP"))
		{
			/* Manufactured Product */
			strcpy (sumr_rec.crd_no, "ManPrd");
		}
		else if (!strcmp (inmr_rec.source, "MC"))
		{
			/* Manufactured Component */
			strcpy (sumr_rec.crd_no, "ManCom");
		}
		else
		{
			/* purchased */
			sumr_rec.hhsu_hash	=	ffwk_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
				strcpy (sumr_rec.crd_no, "UNKNOWN");
		}

	    inis2_rec.hhsu_hash = ffwk_rec.hhsu_hash;
	    inis2_rec.hhbr_hash = ffwk_rec.hhbr_hash;
	    strcpy (inis2_rec.co_no, "  ");
	    strcpy (inis2_rec.br_no, "  ");
	    strcpy (inis2_rec.wh_no, "  ");
	    cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
		if (cc || inis2_rec.hhsu_hash != ffwk_rec.hhsu_hash ||
	    		  inis2_rec.hhbr_hash != ffwk_rec.hhbr_hash)
		{
			inis2_rec.lead_time = 0.00;
		}
		if (inis2_rec.lead_time == (float) 0.00)
			inis2_rec.lead_time = GetLeadDate (inis2_rec.hhis_hash, comm_rec.inv_date);
		LRP_LeadDays  = inis2_rec.lead_time;
		LRP_LeadWeeks = inis2_rec.lead_time / 7;

	    cvr_prd =	LRP_LeadWeeks + local_rec.safetyStock + ffwk_rec.review_pd;

	    if (cvr_prd > 999)
			cvr_prd = 999;

	    if (strchr (LRP_ABCCODES, local_rec.abcCode [0]) &&
				(envLrpInpZero || ffwk_rec.sugg_qty > 0.00) &&
				(inmr_rec.reorder [0] == 'Y' ||
				(inmr_rec.reorder [0] != 'Y' && LRP_ReorderFlag [0] == 'Y')) &&
				ffwk_rec.hhpo_hash == 0)
		{
			tab_add (lrpWork, " %-1.1s %-16.16s %-28.28s %3.0f %7.0f %7.0f %6.0f %6.0f %6.0f %6.0f %6.0f %6.0f %-6.6s %c %9.2f   %s",
				(ffwk_rec.stat_flag [0] == 'U') ? "A" : "C",
				inmr_rec.item_no,
				inmr_rec.description,
				cvr_prd,
				last_12,
				last_6,
				ffwk_cons [ (currentMonth + 7) % 12],
				ffwk_cons [ (currentMonth + 8) % 12],
				ffwk_cons [ (currentMonth + 9) % 12],
				ffwk_cons [ (currentMonth + 10) % 12],
				ffwk_cons [ (currentMonth + 11) % 12],
				ffwk_cons [currentMonth],
				sumr_rec.crd_no,
				(ffwk_rec.alt_supp)	? '*' : ' ',
				(ffwk_rec.stat_flag [0] == 'U') ?
					ffwk_rec.order_qty : ffwk_rec.sugg_qty,
				ffwk_rec.sort);

				if (ffwk_rec.stat_flag [0] == 'U')
					LRP_LinesProcessed++;

				LRP_NoRecords++;

				NoData	=	FALSE;
		}
	    cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
	if (NoData)
		tab_add (lrpWork, "%s", ML ("Sorry but no data exists based on input selection"));

	return (EXIT_FAILURE);
}

void
LoadLocal (void)
{
	local_rec.safetyStock = 0;

	if (LRP_ByCompany)
	{
		local_rec.safetyStock = inmr_rec.safety_stock;
		strcpy (local_rec.abcCode, inmr_rec.abc_code);
	}

	if (LRP_ByBranch)
	{
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (!cc)
		{
			local_rec.safetyStock = inei_rec.safety_stock;
			strcpy (local_rec.abcCode, inei_rec.abc_code);
		}
	}

	if (LRP_ByWarehouse)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (!cc)
		{
			local_rec.safetyStock = incc_rec.safety_stock;
			strcpy (local_rec.abcCode, incc_rec.abc_code);
		}
	}
}

/*
 * Heading concerns itself with clearing the screen,painting the  
 * screen overlay in preparation for input                       
 */
int
heading (
 int        scn)
{
	clear ();
	move (0,1);
	line (132);

	if (envLrpInpZero)
	{
		if (envLrpPriority)
			rv_pr (ML (mlLrpMess053),5,0,1);
		else
			rv_pr (ML (mlLrpMess055),5,0,1);
	} 
	else
	{
		if (envLrpPriority)
			rv_pr (ML (mlLrpMess054),5,0,1);
		else
			rv_pr (ML (mlLrpMess056),5,0,1);
	}

	if (scn == 1)
	{
		move (0,22); 
		cl_line ();
		if (LRP_ByCompany)
		{
			print_at (22,0, ML (mlStdMess038), 
								comm_rec.co_no, comm_rec.co_name);
		}
		else if (LRP_ByBranch)
		{
			print_at (22,0, ML (mlStdMess038), 
								comm_rec.co_no, comm_rec.co_name);
			print_at (22,40,ML (mlStdMess039), 
								comm_rec.est_no, comm_rec.est_name);
		} 
		else if (LRP_ByWarehouse)
		{
			print_at (22,0, ML (mlStdMess038), 
								comm_rec.co_no, comm_rec.co_name);
			print_at (22,40,ML (mlStdMess039), 
								comm_rec.est_no, comm_rec.est_name);
			print_at (22,90,ML (mlStdMess099), 
								comm_rec.cc_no, comm_rec.cc_name);
		}
	}
    return (EXIT_SUCCESS);
}

void
ProcessStack (void)
{
	int		i = 0;
	int		j = 0;

	/*
	 * Go thru stack from top to bottom 
	 */
	for (i = 0; i < MAX_SUPPLY; i++)
	{
	    /*
	     * If value on stack < new value then move all values below down 
	     * put new value into slot created
	     */
	    if (SR [i].wkValue > wk_rec.wkValue)
	    {
			for (j = MAX_SUPPLY - 1;j > i;j--)
			{
				SR [j].wkValue  		=   SR [j - 1].wkValue;
				SR [j].hhsuHash 		=   SR [j - 1].hhsuHash;
				SR [j].hhisHash 		=   SR [j - 1].hhisHash;
				SR [j].firstSupplier	=	SR [j - 1].firstSupplier;
				strcpy (SR [j].priority, 	SR [j - 1].priority);
				strcpy (SR [j].brNo, 		SR [j - 1].brNo);
				strcpy (SR [j].whNo, 		SR [j - 1].whNo);
				strcpy (SR [j].supplierNo,  SR [j - 1].supplierNo);
				strcpy (SR [j].itemNo,   	SR [j - 1].itemNo);
				strcpy (SR [j].lead,      	SR [j - 1].lead);
				strcpy (SR [j].curr,    	SR [j - 1].curr);
				strcpy (SR [j].ctry,      	SR [j - 1].ctry);
				strcpy (SR [j].fobCost,  	SR [j - 1].fobCost);
				strcpy (SR [j].duty,      	SR [j - 1].duty);
				strcpy (SR [j].frt,       	SR [j - 1].frt);
				strcpy (SR [j].intoStore,	SR [j - 1].intoStore);
				strcpy (SR [j].moq,       	SR [j - 1].moq);
				strcpy (SR [j].noq,       	SR [j - 1].noq);
				strcpy (SR [j].oml,       	SR [j - 1].oml);
			}
			SR [i].wkValue			=	wk_rec.wkValue;
			SR [i].hhsuHash		=	wk_rec.hhsuHash;
			SR [i].hhisHash		=	wk_rec.hhisHash;
			SR [i].firstSupplier	=	wk_rec.firstSupplier;
			strcpy (SR [i].supplierNo, 		wk_rec.supplierNo);
			strcpy (SR [i].priority, 	wk_rec.priority);
			strcpy (SR [i].brNo, 		wk_rec.brNo);
			strcpy (SR [i].whNo, 		wk_rec.whNo);
			strcpy (SR [i].priority, 	wk_rec.priority);
			strcpy (SR [i].itemNo, 	wk_rec.itemNo);
			strcpy (SR [i].lead, 		wk_rec.lead);
			strcpy (SR [i].curr, 		wk_rec.curr);
			strcpy (SR [i].ctry, 		wk_rec.ctry);
			strcpy (SR [i].fobCost, 	wk_rec.fobCost);
			strcpy (SR [i].duty, 		wk_rec.duty);
			strcpy (SR [i].frt, 		wk_rec.frt);
			strcpy (SR [i].intoStore,	wk_rec.intoStore);
			strcpy (SR [i].moq, 		wk_rec.moq);
			strcpy (SR [i].noq, 		wk_rec.noq);
			strcpy (SR [i].oml, 		wk_rec.oml);
			break;
	    }
	}
}

/*
 * Get the number of weeks between 'date' & the	next available inld_sup_date.
 * Return 0 if none found.			          
 */
float
GetLeadDate (
 int        hhisHash,
 int        currDate)
{
	float	days;

	inld_rec.hhis_hash 	= hhisHash;
	inld_rec.ord_date 	= currDate;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	if (cc || inld_rec.hhis_hash != hhisHash)
		return ( (float) 0.00);

	days = inld_rec.sup_date - currDate;
	return (days);
}

void
LoadHhcc (void)
{
	int		indx = 0;

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, (LRP_ByCompany) ? "  " : comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, (LRP_ByWarehouse) ? comm_rec.cc_no : "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
	     (LRP_ByCompany || !strcmp (ccmr_rec.est_no, comm_rec.est_no)))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
			continue;
		}
		validWh [indx] = ccmr_rec.hhcc_hash;
		indx++;
		if (LRP_ByWarehouse)
			break;

		if (indx >= MAX_WH)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	validWh [indx] = 0L;
}

static int
ScnConfirm (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	if (!cc)
	{
		if (recBuffer [118] == '*')
		{
			sprintf (err_str, " Check for Cheaper Supplier Item %16.16s ",
					recBuffer + 3);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
		recBuffer [1] = 'A';
		tab_update (lrpWork, "%s", recBuffer);
		cc = tab_get (lrpWork, recBuffer, NEXT, 0);
		if (cc)
			cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	}
	old_line = tab_tline (lrpWork);
	if ( (old_line % WK_DEPTH) == 0)
		load_page (lrpWork, FALSE);
	redraw_page (lrpWork, TRUE);

    return (iUnused);
}

static int
ScnConfirmAll (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, firstLine);
	while (!cc)
	{
		recBuffer [1] = 'A';
		tab_update (lrpWork, "%s", recBuffer);
		cc = tab_get (lrpWork, recBuffer, NEXT, 0);
	}
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	load_page (lrpWork, FALSE);
	redraw_page (lrpWork, TRUE);

    return (iUnused);
}

static int
ScnQuantity (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	float	new_val;
	int		old_line,
			scn_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	if (!cc)
	{
		if (recBuffer [118] == '*')
		{
			sprintf (err_str, " Check for Cheaper Supplier Item %16.16s ",
					recBuffer + 3);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
	
		recBuffer [1] = 'A';
		scn_line = tab_sline (lrpWork);
		crsr_on ();
		new_val = getfloat (121, scn_line, "NNNNNN.NN");
		crsr_off ();

		new_val = CheckValue (recBuffer, new_val);

		print_at (scn_line, 121, "%9.2f", new_val);
		sprintf ( (recBuffer + 120), "%9.2f", new_val);
		recBuffer [129] = ' ';
		tab_update (lrpWork, "%s", recBuffer);
		cc = tab_get (lrpWork, recBuffer, NEXT, 0);
		if (cc)
			cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	}
	old_line = tab_tline (lrpWork);
	if ( (old_line % WK_DEPTH) == 0)
		load_page (lrpWork, FALSE);
	redraw_page (lrpWork, TRUE);
    return (EXIT_SUCCESS);
}

float
CheckValue (
 char*      recBuffer,
 float      qty_ord)
{
	float	wk_qty;

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	sprintf (sumr_rec.crd_no, "%-6.6s", recBuffer + 111);
	cc = find_rec (sumr2, &sumr_rec, EQUAL, "r");
	if (cc)
		return (qty_ord);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", recBuffer + 3);
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		return (qty_ord);

	inis2_rec.hhsu_hash = ffwk_rec.hhsu_hash;
	inis2_rec.hhbr_hash = ffwk_rec.hhbr_hash;
	strcpy (inis2_rec.co_no, "  ");
	strcpy (inis2_rec.br_no, "  ");
	strcpy (inis2_rec.wh_no, "  ");
	cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
	if (cc || inis2_rec.hhsu_hash != ffwk_rec.hhsu_hash ||
			  inis2_rec.hhbr_hash != ffwk_rec.hhbr_hash)
		return (qty_ord);

	if (qty_ord < inis2_rec.min_order && qty_ord != 0.00)
		qty_ord = inis2_rec.min_order;

	if (inis2_rec.ord_multiple != 0.00)
	{
	    wk_qty = qty_ord / inis2_rec.ord_multiple;
	    wk_qty = (float) ceil ( (double) wk_qty);
	    wk_qty *= inis2_rec.ord_multiple;
	    if (wk_qty != qty_ord)
			qty_ord = wk_qty;
	}

	return (qty_ord);
}

static int
ScnReject (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	if (!cc)
	{
		recBuffer [1] = 'C';
		tab_update (lrpWork, "%s", recBuffer);
		cc = tab_get (lrpWork, recBuffer, NEXT, 0);
		if (cc)
			cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	}
	old_line = tab_tline (lrpWork);
	if ( (old_line % WK_DEPTH) == 0)
		load_page (lrpWork, FALSE);
	redraw_page (lrpWork, TRUE);

    return (iUnused);
}

static int
ScnRejectAll (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, firstLine);
	while (!cc)
	{
		recBuffer [1] = 'C';
		tab_update (lrpWork, "%s", recBuffer);
		cc = tab_get (lrpWork, recBuffer, NEXT, 0);
	}
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	load_page (lrpWork, FALSE);
	redraw_page (lrpWork, TRUE);

    return (iUnused);
}

static int
ScnSupplier (
 int        iUnused,
 KEY_TAB*   psUnused)
{
	char	recBuffer [256];
	int		old_line;
	char	wk_sort [35];

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);

	/*
	 * Check if manufactured item. If so, skip out            
	 */
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", recBuffer + 3);
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		return (FALSE);
	if (!strcmp (inmr_rec.source, "MP") || !strcmp (inmr_rec.source, "MC"))
	{
		print_mess ("Option Not Available as Item Relates to Manufacturing");
		fflush (stdout);
		sleep (sleepTime);	
		return (OTHER);
		cc = 1;
	}

	if (!cc)
	{
		sprintf (wk_sort, "%-34.34s", recBuffer + 132);
		tab_clear (lrpWork);
		if (Process (recBuffer))
		{
			recBuffer [1] = 'A';

			tab_update (lrpWork, "%-111.111s%-6.6s %c %9.2f   %-34.34s",
				recBuffer,
				sumr_rec.crd_no,
				' ',
				ffwk_rec.order_qty,
				wk_sort);

			cc = tab_get (lrpWork, recBuffer, NEXT, 0);
			if (cc)
				cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
		}
	}
	heading (1);

	if (TabOpenSupplier)
		tab_clear (supplierWork);
	redraw_table (lrpWork);
	load_page (lrpWork, FALSE);
    
    return (iUnused);
}

/*
 * Update ffwk records 
 */
void
Update (void)
{
	char	recBuffer [256];
	int		i	=	0;
	int		j	=	14;
	int		NoBars;

	sprintf (err_str, "%101.101s", " ");
	us_pr (err_str, 14,3,0);

	NoBars	=	LRP_NoRecords / 100;
	if (!NoBars)
		NoBars = 1;

	move (0, 2);
	cl_end ();
	/*---"%R Updating "--*/
	print_at (12, 60, ML (mlStdMess035));

	abc_selfield (ffwk, "ffwk_sort_id");
	if (tab_get (lrpWork, recBuffer, FIRST, 0))
		return;

	ffwk_rec.hhcc_hash = hhccHash;
	sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	sprintf (ffwk_rec.sort, "%-34.34s", recBuffer + 132);
	cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
	while
	(
		!cc &&
		ffwk_rec.hhcc_hash == hhccHash &&
		!strcmp (ffwk_rec.filename,LRP_FileName)
	)
	{
	    /*
	     * If we come across an ffwk record which refers to an
	     * inmr which has since been deleted, IGNORE IT
	     */
		inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
	    {
			if (tab_get (lrpWork, recBuffer, NEXT, 0))
				break;

			ffwk_rec.hhcc_hash = hhccHash;
			sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
			sprintf (ffwk_rec.sort, "%-34.34s", recBuffer + 132);
			cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
			continue;
	    }
	    strcpy (local_rec.abcCode, inmr_rec.abc_code);

	    if (LRP_ByBranch)
	    {
			inei_rec.hhbr_hash = ffwk_rec.hhbr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc)
				return;

			strcpy (local_rec.abcCode, inei_rec.abc_code);
	    }

	    if (LRP_ByWarehouse)
	    {
			incc_rec.hhbr_hash = ffwk_rec.hhbr_hash;
			incc_rec.hhcc_hash = hhccHash;
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			if (cc)
				return;
			strcpy (local_rec.abcCode, incc_rec.abc_code);
	    }

	    if (strchr (LRP_ABCCODES, local_rec.abcCode [0]) &&
			(envLrpInpZero || ffwk_rec.sugg_qty > 0.00) &&
			ffwk_rec.hhpo_hash == 0)
	    {
			i++;
			if ( (i % NoBars) == 0)
				us_pr (" ", j++, 3,1);

			switch (recBuffer [1])
			{
			case	'A':
				if (strcmp (inmr_rec.source, "MP") 
					&& strcmp (inmr_rec.source, "MC"))
				{
					strcpy (sumr_rec.co_no, comm_rec.co_no);
					strcpy (sumr_rec.est_no, branchNumber);
					sprintf (sumr_rec.crd_no, "%-6.6s", recBuffer + 111);
					cc = find_rec (sumr2, &sumr_rec, EQUAL, "r");
					if (cc)
					{
						strcpy (ffwk_rec.stat_flag, " ");
						abc_update (ffwk, &ffwk_rec);
						break;
					}
					strcpy (ffwk_rec.crd_no, sumr_rec.crd_no);
					ffwk_rec.hhsu_hash = sumr_rec.hhsu_hash;
				}
				ffwk_rec.order_qty = atof (recBuffer + 119);
				strcpy (ffwk_rec.stat_flag, "U");
				abc_update (ffwk, &ffwk_rec);
		    break;

			default:
				strcpy (ffwk_rec.stat_flag, " ");
				abc_update (ffwk, &ffwk_rec);
		    break;
			}
			abc_unlock (ffwk);
	    }
	    if (tab_get (lrpWork, recBuffer, NEXT, 0))
			break;

	    ffwk_rec.hhcc_hash = hhccHash;
	    sprintf (ffwk_rec.filename, "%-14.14s", LRP_FileName);
	    sprintf (ffwk_rec.sort, "%-34.34s", recBuffer + 132);
	    cc = find_rec (ffwk, &ffwk_rec, EQUAL, "u");
	}
	abc_unlock (ffwk);
}

static float
Rnd_Mltpl (
 float	ord_qty,
 char	*rnd_type,
 float	ord_mltpl,
 float	min_qty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (ord_qty == 0.00)
		return (0.00);

	if (ord_mltpl == 0.00)
		return ( (ord_qty < min_qty) ? min_qty : ord_qty);

	ord_qty -= min_qty;
	if (ord_qty < 0.00)
		ord_qty = 0.00;

	/*
	 * Already An Exact Multiple 
	 */
	wrk_qty = (double) (ord_qty / ord_mltpl);
	if (ceil (wrk_qty) == wrk_qty)
		return (ord_qty + min_qty);

	/*
	 * Perform Rounding 
	 */
	switch (rnd_type [0])
	{
	case 'U':
		/*
		 * Round Up To Nearest Multiple 
		 */
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'D':
		/*
		 * Round Down To Nearest Multiple 
		 */
		wrk_qty = (double) (ord_qty / ord_mltpl);
		wrk_qty = floor (wrk_qty);
		ord_qty = (float) (wrk_qty * ord_mltpl);
		break;

	case 'B':
		/*
		 * Find Value If Rounded Up 
		 */
		up_qty = (double) ord_qty;
		wrk_qty = (up_qty / (double)ord_mltpl);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * ord_mltpl);

		/*
		 * Find Value If Rounded Down 
		 */
		down_qty = (double) ord_qty;
		wrk_qty = (down_qty / (double) ord_mltpl);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * ord_mltpl);

		/*
		 * Round Up/Down To Nearest Multiple 
		 */
		if ( (up_qty - (double) ord_qty) <= ( (double) ord_qty - down_qty))
			ord_qty = (float) up_qty;
		else
			ord_qty = (float) down_qty;

		break;

	default:
		break;
	}

	return (min_qty + ord_qty);
}

static int
ScnAlldisp (
	int        iUnused,
	KEY_TAB*   psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", recBuffer + 3);
		cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf (err_str, "sk_alldisp %s %010ld",
						ffwk_rec.source,inmr_rec.hhbr_hash); 
			clear ();
			snorm ();
			sys_exec (err_str);
		}
		swide ();
		cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	}
	heading (1);
	load_page (lrpWork, FALSE);
	redraw_table (lrpWork);

    return iUnused;
}

static int
ScnLrpdisp (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	char	recBuffer [256];
	int		old_line;

	old_line = tab_tline (lrpWork);
	cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", recBuffer + 3);
		cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			if (LRP_ByCompany)
				sprintf (err_str, "lrp_display %010ld 0", inmr_rec.hhbr_hash); 
			else if (LRP_ByBranch)
				sprintf (err_str, "lrp_display %010ld 1", inmr_rec.hhbr_hash); 
			else if (LRP_ByWarehouse)
				sprintf (err_str, "lrp_display %010ld 2", inmr_rec.hhbr_hash); 
			clear ();
			snorm ();
			sys_exec (err_str);
		}
		swide ();
		cc = tab_get (lrpWork, recBuffer, EQUAL, old_line);
	}
	heading (1);
	load_page (lrpWork, FALSE);
	redraw_table (lrpWork);

    return iUnused;
}

/*
 * Function adds to supplier priorities.
 */
static int
AddToSuppPriority (
 int            iUnused,
 KEY_TAB*       psUnused)
{
	int		CurrentPriority;
	int		TabLine;

	TabLine		=	tab_tline (supplierWork);

	tab_get (supplierWork, LRP_SuppBuffer, CURRENT, 0);

	abc_selfield (inis2, "inis_hhis_hash");

	inis2_rec.hhis_hash	=	SR [TabLine].hhisHash;
	cc = find_rec (inis2, &inis2_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inis2, "DBFIND");

	CurrentPriority = atoi (SR [TabLine].priority + 1);
	if (CurrentPriority < 9)
		CurrentPriority++;

	sprintf (err_str, "%-1.1s%d", inis2_rec.sup_priority, CurrentPriority);
	strcpy (inis2_rec.sup_priority, err_str);
	strcpy (SR [TabLine].priority, inis2_rec.sup_priority);

	cc = abc_update (inis2, &inis2_rec);
	if (cc)
		file_err (cc, inis2, "DBUPDATE");

	abc_selfield (inis2, "inis_id_no3");

	sprintf (LRP_TempBuffer, "%s%s", inis2_rec.sup_priority, LRP_SuppBuffer + 2);
	tab_update (supplierWork, "%s", LRP_TempBuffer);

	if (CurrentPriority == 1)
	{
		LRP_OrderSupplier	=	TabLine;
	
		LRP_MinOrderQty	= (float) atof (SR [LRP_OrderSupplier].moq);
		LRP_NorOrderQty	= (float) atof (SR [LRP_OrderSupplier].noq);
		LRP_LeadDays	= (float) atof (SR [LRP_OrderSupplier].lead);
		LRP_LeadWeeks	= LRP_LeadDays / 7;
		DisplayEntry (0);
	}

    return iUnused;
}

/*
 * Function subtracts from supplier priorities. 
 */
static int
SubtractSuppPriority (
	int     iUnused,
	KEY_TAB	*psUnused)
{
	int		CurrentPriority;
	int		TabLine;

	TabLine		=	tab_tline (supplierWork);

	tab_get (supplierWork, LRP_SuppBuffer, CURRENT, 0);

	abc_selfield (inis2, "inis_hhis_hash");

	inis2_rec.hhis_hash	=	SR [TabLine].hhisHash;
	cc = find_rec (inis2, &inis2_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inis2, "DBFIND");

	CurrentPriority = atoi (SR [TabLine].priority + 1);
	if (CurrentPriority > 1)
		CurrentPriority--;

	sprintf (err_str, "%-1.1s%d", inis2_rec.sup_priority, CurrentPriority);
	strcpy (inis2_rec.sup_priority, err_str);
	strcpy (SR [TabLine].priority, inis2_rec.sup_priority);

	cc = abc_update (inis2, &inis2_rec);
	if (cc)
		file_err (cc, inis2, "DBUPDATE");

	abc_selfield (inis2, "inis_id_no3");

	sprintf (LRP_TempBuffer, "%s%s", inis2_rec.sup_priority, LRP_SuppBuffer + 2);
	tab_update (supplierWork, "%s", LRP_TempBuffer);

	if (CurrentPriority == 1)
	{
		LRP_OrderSupplier	=	TabLine;
	
		LRP_MinOrderQty	= (float) atof (SR [LRP_OrderSupplier].moq);
		LRP_NorOrderQty	= (float) atof (SR [LRP_OrderSupplier].noq);
		LRP_LeadDays	= (float) atof (SR [LRP_OrderSupplier].lead);
		LRP_LeadWeeks	= LRP_LeadDays / 7;
		DisplayEntry (0);
	}

    return iUnused;
}

/*
 * Function Sets selected supplier to priority one. 
 */
static int
SetSupplierPriority (
	int		iUnused,
	KEY_TAB	*psUnused)
{
	int		TabLine;

	TabLine		=	tab_tline (supplierWork);

	tab_get (supplierWork, LRP_SuppBuffer, CURRENT, 0);

	/*
	 * Proirity is already one so return. 
	 */
	if (SR [TabLine].priority [1] == '1')
		return iUnused;

	abc_selfield (inis2, "inis_hhis_hash");

	inis2_rec.hhis_hash	=	SR [TabLine].hhisHash;

	cc = find_rec (inis2, &inis2_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inis2, "DBFIND");

	sprintf (err_str, "%-1.1s1", inis2_rec.sup_priority);
	strcpy (inis2_rec.sup_priority, err_str);
	strcpy (SR [TabLine].priority, inis2_rec.sup_priority);

	cc = abc_update (inis2, &inis2_rec);
	if (cc)
		file_err (cc, inis2, "DBUPDATE");

	sprintf (LRP_TempBuffer, "%s%s", inis2_rec.sup_priority, LRP_SuppBuffer+2);
	tab_update (supplierWork, LRP_TempBuffer);
	redraw_table (supplierWork);

	LRP_OrderSupplier	=	TabLine;
	
	LRP_MinOrderQty	= (float) atof (SR [LRP_OrderSupplier].moq);
	LRP_NorOrderQty	= (float) atof (SR [LRP_OrderSupplier].noq);
	LRP_LeadDays	= (float) atof (SR [LRP_OrderSupplier].lead);
	LRP_LeadWeeks	= LRP_LeadDays / 7;

	UpdatePriorities (TabLine);

	tab_get (supplierWork, LRP_SuppBuffer, EQUAL, TabLine);
	redraw_table (supplierWork);
	abc_selfield (inis2, "inis_id_no3");
		
	DisplayEntry (0);

    return iUnused;
}

/*
 * Function updated duplicate priorities.
 */
void
UpdatePriorities (
	int		notThisLine)
{
	int		PriCnt;
	int		CurrentPriority;

	for (PriCnt = 0; PriCnt < maxLines; PriCnt++)
	{
		if (PriCnt == notThisLine)
			continue;

		tab_get (supplierWork, LRP_SuppBuffer, EQUAL, PriCnt);

		if (!strncmp (LRP_SuppBuffer, SR [notThisLine].priority,2))
		{
			CurrentPriority = atoi (LRP_SuppBuffer + 1);
			if (CurrentPriority < 9)
				CurrentPriority++;

			sprintf (err_str, "%-1.1s%d", LRP_SuppBuffer, CurrentPriority);
			strcpy (SR [PriCnt].priority, err_str);

			inis2_rec.hhis_hash	=	SR [PriCnt].hhisHash;
			cc = find_rec (inis2, &inis2_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, inis2, "DBFIND");

			strcpy (inis2_rec.sup_priority, SR [PriCnt].priority);
		
			cc = abc_update (inis2, &inis2_rec);
			if (cc)
				file_err (cc, inis2, "DBUPDATE");

			tab_get (supplierWork, LRP_SuppBuffer, EQUAL, PriCnt);
			sprintf (LRP_TempBuffer, "%s%s", inis2_rec.sup_priority, LRP_SuppBuffer + 2);
			tab_update (supplierWork, LRP_TempBuffer);
            redraw_table (supplierWork);
			UpdatePriorities (PriCnt);
            return;
		}
	}
}

static int
SupplierSelected (
	int			iUnused,
	KEY_TAB		*psUnused)
{
	int		Priorities [4][10];
	int		Offset;
	int		i;

	for (i = 0; i < 9; i++)
	{
		Priorities [0][i] = FALSE;
		Priorities [1][i] = FALSE;
		Priorities [2][i] = FALSE;
	}
	for (i = 0; i < maxLines; i++)
	{
		Offset	=	atoi (SR [i].priority + 1);
		if (SR [i].priority [0] == 'C')
		{
			if (Priorities [0][Offset - 1])
			{
				print_mess (ML ("Duplicate priority exists, please fix."));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}

			Priorities [0][Offset - 1] = TRUE;
		}

		if (SR [i].priority [0] == 'B')
		{
			if (Priorities [1][Offset - 1])
			{
				print_mess (ML ("Duplicate priority exists, please fix."));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}
			Priorities [1][Offset - 1] = TRUE;
		}

		if (SR [i].priority [0] == 'W')
		{
			if (Priorities [2][Offset - 1])
			{
				print_mess (ML ("Duplicate priority exists, please fix."));
				sleep (sleepTime);
				clear_mess ();
				return (' ');
			}

			Priorities [2][Offset - 1] = TRUE;
		
		}
	}
	if (LRP_LeadWeeks >  wks_tot_cover && !DoublePress)
	{
		print_mess (ML (" ** ORDER MAY NOT BE RECEIVED BEFORE STOCK IS DEPLETED, SELECT AGAIN TO CONFIRM ** \007"));
		sleep (sleepTime);	
		clear_mess ();
		DoublePress++;
		return (' ');
	}
	
	LRP_SupplierIndex	=	tab_tline (supplierWork);
	return (FN16);
}

/*
 * OK selection function. 
 */
void
TagOther (void)
{
	LRP_LinesProcessed	=	tab_tline (lrpWork);
}

/*
 * This is required to re-select line you require after tab_scan is performed.
 */
void
ProgramTabScan (void)
{
	if (LRP_PriOneSupplier)
	{
		redraw_line (supplierWork, FALSE);
		tab_get (supplierWork, LRP_SuppBuffer, EQUAL, LRP_PriOneSupplier);
		redraw_line (supplierWork, TRUE);
	}
}

/*
 * Move to pre_selected line 
 */
int
ScnMove (
 int        iUnused,
 KEY_TAB*   psUnused)
{
	char	recBuffer [256];
	cc = tab_get (lrpWork, recBuffer, EQUAL, LRP_LinesProcessed);

    return iUnused;
}

/*
 * Set up month array based on current month 
 */
void
SetPeriodHeader (
	char	*headerType)
{
	int	i, j;

	for (i = 0; i < 11; i++)
	{
		switch (headerType [0])
		{
		case 'W':
			if (i == 0)
				strcpy (periodHead [i], "CUR");
			else
				sprintf (periodHead [i], "W%02d", i);
			break;

		case 'D':
			j = currDay - (i % 7);
			if (j < 0)
				j += 7;
			sprintf (periodHead [i], "%-3.3s", dayName [j]);
			break;

		case 'M':
		default:
			j = currMonth - i - 1;
			if (j < 0)
				j += 12;
			sprintf (periodHead [i], "%-3.3s", monthName [j]);
			break;

		}
	}
}
