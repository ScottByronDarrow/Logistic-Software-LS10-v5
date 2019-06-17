/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cn_maint.c,v 5.7 2002/07/25 11:17:27 scott Exp $
|  Program Name  : (db_cn_maint.c)
|  Program Desc  : (Customer Contract Maintenance)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius.  | Date Written  : 20/09/93         |
|---------------------------------------------------------------------|
| $Log: db_cn_maint.c,v $
| Revision 5.7  2002/07/25 11:17:27  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.6  2002/07/24 08:38:47  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/18 06:24:12  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/16 01:04:14  scott
| Updated from service calls and general maintenance.
|
| Revision 5.3  2002/07/01 05:12:19  robert
| Rename structure storetype to storeRec (for use in SetSortArray)
|
| Revision 5.2  2002/06/25 04:02:02  scott
| Updated to add new sort routines required for GUI
|
| Revision 5.1  2001/12/07 03:30:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/11/21 01:58:20  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cn_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cn_maint/db_cn_maint.c,v 5.7 2002/07/25 11:17:27 scott Exp $";

#define		TABLINES		15
#define 	MAXSCNS			2
#define 	MAXLINES		1000
#include 	<pslscr.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#define		ScreenWidth		132
#define		SR				store [line_cnt]

	/*
	 * Special fields and flags  
	 */
	int		envCrCo 	= 0,
			envCrFind 	= 0,
			newContract = TRUE,
			envDbMcurr	= 0,
			gwsNoEdit 	= FALSE,
			firstTime 	= TRUE;

	char	branchNumber 	[3],
			envCurrCode 	[4];

	extern	int		TruePosition;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct esmrRecord	esmr_rec;
struct cnchRecord	cnch_rec;
struct cnchRecord	cnch2_rec;
struct cncdRecord	cncd_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocr_rec;
struct qthrRecord	qthr_rec;
struct qtphRecord	qtph_rec;
struct cohrRecord	cohr_rec;
struct sohrRecord	sohr_rec;
struct sumrRecord	sumr_rec;


/*============================ 
| Local & Screen Structures. |
============================*/

struct	storeRec
{
	char	itemNo [17];
	char	itemDesc [41];
	char	brNo [3];
	long	hhbrHash;
	long	hhccHash;
	long	hhsuHash;
	long	hhclHash;
	double	salePrice;
	char	currCode [4];
	double	exchRate;
	double	costPrice;
	char	supplierCurr [4];
	float	margin;
	char	discountOk [2];
	char	rebateCode [5];
} 	store [MAXLINES];


struct {
	char 	dummy [11];
	char	cnt_no [7];
	char	last_cnt_no [7];
	char	desc [41];
	char	contact [21];
	long	date_wef;
	long	date_exp;
	long	date_rev;
	long	date_ren;
	char	exch_type [2];
	char	item_no [17];
	char	item_desc [41];
	double	sal_price;
	char	sal_curr [4];
	char	br_no [3];
	char	wh_no [3];
	char	crd_no [7];
	char	crd_short [17];
	double	cst_price;
	float	margin;
	float	conv_fact;
	char	disc_ok [2];
	char	rebate_code [5];
	char	systemDate [11];
} local_rec;

	char	*data	= "data",
			*sumr2	= "sumr2";

static	struct	var	vars [] =
{
	{1, LIN, "cnt_no",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Contract Number      ", "Enter Contract Number",
		 NE, NO,  JUSTLEFT, "", "", local_rec.cnt_no},
	{1, LIN, "cnt_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description          ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "cnt_name",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "      ", "Contact Name        ", "",
		 YES, NO,  JUSTLEFT, "", "", local_rec.contact},
	{1, LIN, "cnt_date_wef",	 8, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Effective Date       ", "Enter Date Contract is Effective From",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.date_wef},
	{1, LIN, "cnt_date_exp",	9, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Expiry Date          ", "Enter Date Contract Expires",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.date_exp},
	{1, LIN, "cnt_date_rev",	 10, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Review Date          ", "Enter Date Contract Due for Review",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.date_rev},
	{1, LIN, "cnt_date_ren",	 11, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Renewal Date         ", "Enter Date Contract Due for Renewal",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.date_ren},
	{1, LIN, "exch_type",	 13, 2, CHARTYPE,
		"U", "          ",
		" ", "F", "Pricing Type         ", "Enter Pricing Type (Fixed, Variable)",
		 NE, NO,  JUSTLEFT, "FV", "", local_rec.exch_type},

	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " ITEM NUMBER    ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "item_desc",	 MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  ITEM DESCRIPTION                      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{2, TAB, "sal_curr",	MAXLINES, 0, CHARTYPE,
		"UUU", "          ",
		" ", envCurrCode, "CUR", "Enter Item Selling Currency",
		 NO, NO,  JUSTLEFT, "", "", local_rec.sal_curr},
	{2, TAB, "sal_price",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", " SALE PRICE ", "Enter Item Selling Price",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.sal_price},
	{2, TAB, "disc_ok",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "N", "DISC", "Are Discounts Allowed? (Y/N)",
		 NO, NO,  JUSTRIGHT, "YN", "", local_rec.disc_ok},
	{2, TAB, "br_no",	 0, 1, CHARTYPE,
		"AA", "          ",
		" ", "  ", "BR ", "Enter Primary Branch Number",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.br_no},
	{2, TAB, "wh_no",	 0, 1, CHARTYPE,
		"AA", "          ",
		" ", "  ", "WH ", "Enter Primary Warehouse Number",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.wh_no},
	{2, TAB, "crd_no",	 0, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "SUPPLIER", "Enter Supplier Number for Contracted Item",
		 NO, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{2, TAB, "crd_short",	 0, 0, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "0", " ACRONYM ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.acronym},
	{2, TAB, "cst_curr",	MAXLINES, 0, CHARTYPE,
		"UUU", "          ",
		" ", "", "CUR", "",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.curr_code},
	{2, TAB, "cst_price",	 0, 0, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", "0", " COST PRICE ", "Enter Agreed Item Cost Price",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.cst_price},
	{2, TAB, "margin",	 0, 0, FLOATTYPE,
		"NN.NN", "          ",
		" ", "0", "% MAR", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.margin},
	{2, TAB, "rebate_code",	 0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", "0", " REBATE CODE  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.rebate_code},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include	<FindSumr.h>

/*
 * Local Function Prototypes.
 */
float 	CalcMargin 			(int);
int 	CostsOk 			(void);
int 	DeleteLine 			(void);
int 	FindTableIndex 		(long, char *, int);
int 	heading 			(int);
int 	LoadCncd 			(void);
int 	spec_valid 			(int);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);
void 	SrchCcmr 			(char *);
void 	SrchCnch 			(char *);
void 	SrchEsmr 			(char *);
void 	SrchPocr 			(char *);
void 	tab_other 			(int);
void 	Update 				(void);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char		*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);


	init_scr 	();		/*  sets terminal from termcap	*/
	set_tty 	();
	set_masks 	();		/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);	/*  set default values		*/

	envCrCo = atoi (get_env ("CR_CO"));
	envCrFind = atoi (get_env ("CR_FIND"));

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (!envDbMcurr)
		FLD ("exch_type")   = ND;

	/*
	 * Get local currency code.
	 */
	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	OpenDB ();
	swide ();

	tab_row = 3;
	tab_col = 0;


	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	/*
	 * Beginning of input control loop.
	 */
	strcpy (local_rec.cnt_no, "000000");
	strcpy (local_rec.last_cnt_no, "000000");

	while (prog_exit == 0) 
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		prog_status	= ENTRY;
		init_vars (1);
		init_vars (2);
		lcount [2]	=	0;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		if (newContract == TRUE)
		{
			/*
			 * Enter screen 2 tabular input.
			 */
			heading (2);
			entry (2);
		}

		if (prog_exit || restart) 
			continue;

		while (TRUE)
		{
			edit_all ();
			if (restart || prog_exit || !CostsOk ())
				break;
		}
		if (restart) 
			continue;

		/*
		 * Update selection status.    
		 */
		Update ();

	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}


/*
 * Program exit sequence
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (sumr2, sumr);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no"
		  					       						    : "sumr_id_no3");
	open_rec (sumr2,sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_cont_no");
	open_rec (qtph, qtph_list, QTPH_NO_FIELDS, "qtph_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (sumr2);
	abc_fclose (sumr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (qthr);
	abc_fclose (qtph);
	abc_fclose (sohr);
	abc_fclose (cohr);
	SearchFindClose ();
	abc_dbclose (data);
}

void
tab_other (
	int		lineNo)
{
	if (gwsNoEdit == TRUE)
	{
		FLD ("item_no") 	 = NE;
		FLD ("item_desc") 	 = NA;
		FLD ("sal_curr") 	 = NA;
		FLD ("sal_price") 	 = NA;
		FLD ("disc_ok")		 = NA;
		FLD ("br_no") 		 = NA;
		FLD ("wh_no") 		 = NA;
		FLD ("crd_no") 		 = NA;
		FLD ("crd_short") 	 = NA;
		FLD ("cst_curr") 	 = NA;
		FLD ("cst_price") 	 = NA;
		if (!envDbMcurr)
		{
			FLD ("sal_curr")   = ND;
			FLD ("cst_curr")   = ND;
		}
		FLD ("rebate_code") = ND;
	}
	else
	{
		FLD ("item_no") 	 = NO;
		FLD ("item_desc") 	 = NO;
		FLD ("sal_curr") 	 = NO;
		FLD ("sal_price") 	 = NO;
		FLD ("disc_ok")		 = NO;
		FLD ("br_no") 		 = NO;
		if (strcmp (store [lineNo].brNo, "  ") == 0)
			FLD ("wh_no")   = NA;
		else
			FLD ("wh_no")   = YES;
		FLD ("wh_no") 		 = NO;
		FLD ("crd_no") 		 = NO;
		FLD ("crd_short") 	 = NA;
		FLD ("cst_curr") 	 = NA;
		if (store [lineNo].hhsuHash == 0L)
			FLD ("cst_price")   = NA;
		else
			FLD ("cst_price")   = YES;
		FLD ("rebate_code") = ND;

		if (!envDbMcurr)
		{
			FLD ("sal_curr")   = ND;
			FLD ("cst_curr")   = ND;
		}
		else
		{
			FLD ("sal_curr")   = (cnch_rec.exch_type [0] == 'F') ? YES : NA;
		}

	}
}

int
spec_valid (
 int                field)
{
	int		ws_ix;
	int		ws_table_max;

	/*
	 * Validate Contract Number And Allow Search.
	 */
	if (LCHECK ("cnt_no"))
	{
		newContract = FALSE;
		firstTime = TRUE;
		gwsNoEdit = FALSE;

		if (SRCH_KEY)
		{
	 		SrchCnch (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (cnch_rec.co_no	,comm_rec.co_no);
		strcpy (cnch_rec.cont_no	,local_rec.cnt_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			/* initialise contract header */
			FLD ("cnt_desc")   = YES;
			strcpy (local_rec.desc, " ");
			strcpy (local_rec.contact," ");
			strcpy (local_rec.exch_type, "F");
			newContract = TRUE;
			return (EXIT_SUCCESS);
		}

		gwsNoEdit = FALSE;
		if (!envDbMcurr)
		{
			cnch_rec.exch_type [0] = 'V';
			FLD ("sal_curr")   = ND;
			FLD ("cst_curr")   = ND;
		}
		else
			if (cnch_rec.exch_type [0] == 'F')
				FLD ("sal_curr")   = YES;
			else
				FLD ("sal_curr")   = NA;

		strcpy (local_rec.desc, cnch_rec.desc);
		strcpy (local_rec.contact, cnch_rec.contact);
		strcpy (local_rec.exch_type, cnch_rec.exch_type);
		local_rec.date_wef = cnch_rec.date_wef;
		local_rec.date_exp = cnch_rec.date_exp;
		local_rec.date_rev = cnch_rec.date_rev;
		local_rec.date_ren = cnch_rec.date_ren;

		LoadCncd ();
		scn_set (1);
		scn_display (1);
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("desc"))
	{
		strcpy (cnch_rec.desc, local_rec.desc);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("contact"))
	{
		strcpy (cnch_rec.contact, local_rec.contact);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cnt_date_wef"))
	{
		if (local_rec.date_wef > cnch_rec.date_exp && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess083));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_wef > cnch_rec.date_ren && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess084));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_wef > cnch_rec.date_rev && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess085));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cnch_rec.date_wef = local_rec.date_wef;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cnt_date_exp"))
	{
		if (local_rec.date_exp <= cnch_rec.date_wef)
		{
			print_mess (ML (mlDbMess086));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_exp < cnch_rec.date_ren && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess087));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_exp < cnch_rec.date_rev && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess088));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cnch_rec.date_exp = local_rec.date_exp;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cnt_date_rev"))
	{
		if (local_rec.date_rev <= cnch_rec.date_wef)
		{
			print_mess (ML (mlDbMess089));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_rev >= cnch_rec.date_exp)
		{
			print_mess (ML (mlDbMess090));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_rev > cnch_rec.date_ren && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess091));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cnch_rec.date_rev = local_rec.date_rev;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cnt_date_ren"))
	{
		if (local_rec.date_ren <= cnch_rec.date_wef)
		{
			print_mess (ML (mlDbMess092));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_ren > cnch_rec.date_exp)
		{
			print_mess (ML (mlDbMess093));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.date_ren < cnch_rec.date_rev && prog_status != ENTRY)
		{
			print_mess (ML (mlDbMess094));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cnch_rec.date_ren = local_rec.date_ren;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("exch_type"))
	{
		if (local_rec.exch_type [0] == 'F')
			FLD ("sal_curr")   = YES;
		else
			FLD ("sal_curr")   = NA;
		strcpy (cnch_rec.exch_type, local_rec.exch_type);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used) /* Delete Item */
			return (DeleteLine ());

		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Kitted Product ?
		 */
		if (inmr_rec.inmr_class [0] == 'K')
		{
			print_mess (ML (mlDbMess080));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Discontinued Product ?
		 */
		if (inmr_rec.active_status [0] == 'D')
		{
			print_mess (ML (mlDbMess081));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (local_rec.item_desc, inmr_rec.description);

		strcpy (SR.itemNo, inmr_rec.item_no);
		strcpy (SR.itemDesc, inmr_rec.description);
		SR.hhbrHash = inmr_rec.hhbr_hash;

		DSP_FLD ("item_desc");
		if (!envDbMcurr)
		{
			strcpy (local_rec.sal_curr, envCurrCode);
			strcpy (SR.currCode, envCurrCode);
		}
		else
		if (cnch_rec.exch_type [0] == 'V')
		{
			strcpy (local_rec.sal_curr, envCurrCode);
			strcpy (SR.currCode, envCurrCode);
			SR.exchRate = 1.0;
			DSP_FLD ("sal_curr");
		}

		if (prog_status == ENTRY)
			ws_table_max = line_cnt;
		else
			ws_table_max = lcount [2];

		ws_ix = FindTableIndex (inmr_rec.hhbr_hash, 
							   SR.currCode, 
							   ws_table_max);
		if (ws_ix < ws_table_max && ws_ix != line_cnt)
		{
			print_mess (ML (mlDbMess082));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		skip_entry = 1;
		tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Description.
	 */
	if (LCHECK ("item_desc"))
	{
	       return (EXIT_SUCCESS);
	}

	/*
	 * Validate Currency Code Input.
	 */
	if (LCHECK ("sal_curr"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.sal_curr, envCurrCode);
			DSP_FLD ("sal_curr");
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, local_rec.sal_curr);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess ("\007Currency Code is not on file");*/
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (prog_status == ENTRY)
			ws_table_max = line_cnt;
		else
			ws_table_max = lcount [2];

		ws_ix = FindTableIndex (SR.hhbrHash, 
							   local_rec.sal_curr, 
							   ws_table_max);
		if (ws_ix < ws_table_max && ws_ix != line_cnt)
		{
			print_mess (ML (mlDbMess082));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (SR.currCode, local_rec.sal_curr);
		SR.exchRate = pocr_rec.ex1_factor;
		DSP_FLD ("sal_curr");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate sale price.
	 */
	if (LCHECK ("sal_price"))
	{
		if (dflt_used)
			DSP_FLD ("sal_price");

		if (local_rec.sal_price <= SR.costPrice)
		{
			ws_ix = prmptmsg (ML (mlDbMess075),"YyNn",1,23);
			move (1,23);
			cl_line ();
			if (ws_ix != 'Y' && ws_ix != 'y')
				return (EXIT_FAILURE);
		}

		SR.salePrice = local_rec.sal_price;

		local_rec.margin = CalcMargin (line_cnt);
		SR.margin = local_rec.margin;
		if (prog_status != ENTRY)
			DSP_FLD ("margin");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate branch number.
	 */
	if (LCHECK ("br_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.wh_no, "  ");
			strcpy (local_rec.br_no, "  ");
			strcpy (SR.brNo, "  ");
			SR.hhccHash = 0L;
			DSP_FLD ("br_no");
			DSP_FLD ("wh_no");
			FLD ("wh_no")   = NA;
			skip_entry = 1;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no,	comm_rec.co_no);
		strcpy (esmr_rec.est_no,		local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (SR.brNo, local_rec.br_no);
		FLD ("wh_no")   = YES;
		return (EXIT_SUCCESS);
	}


	/*
	 * Validate discount allowed.
	 */
	if (LCHECK ("disc_ok"))
	{
		strcpy (SR.discountOk, local_rec.disc_ok);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate warehouse number.
	 */
	if (LCHECK ("wh_no"))
	{
		if (FLD ("wh_no") == NA)
			return (EXIT_SUCCESS);

		if (strcmp (local_rec.br_no, "  ") == 0)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		abc_selfield (ccmr,"ccmr_id_no");
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no	,comm_rec.co_no);
		strcpy (ccmr_rec.est_no	,local_rec.br_no);
		strcpy (ccmr_rec.cc_no	,local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SR.hhccHash = ccmr_rec.hhcc_hash;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Creditor Number.
	 */
	if (LCHECK ("crd_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.crd_no, "%-6.6s", " ");
			sprintf (sumr_rec.acronym, "%-9.9s", " ");
			sprintf (sumr_rec.curr_code, "%-3.3s", " ");
			sprintf (SR.supplierCurr, "%-3.3s", " ");
			local_rec.cst_price = 0;
			SR.hhsuHash = 0L;
			SR.costPrice = 0;
			local_rec.margin = CalcMargin (line_cnt);
			SR.margin = local_rec.margin;
			DSP_FLD ("crd_no");
			DSP_FLD ("crd_short");
			DSP_FLD ("cst_curr");
			DSP_FLD ("cst_price");
			DSP_FLD ("margin");
			FLD ("cst_price")   = NA;
			skip_entry = goto_field (field, label ("rebate_code"));
			return (EXIT_SUCCESS);
		} 

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.crd_no));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		SR.hhsuHash = sumr_rec.hhsu_hash;
		strcpy (SR.supplierCurr, sumr_rec.curr_code);

		DSP_FLD ("crd_short");
		DSP_FLD ("cst_curr");
		FLD ("cst_price")   = NO;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate cost price.
	 */
	if (LCHECK ("cst_price"))
	{
		if (SR.hhsuHash == 0L)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (dflt_used)
		{
			print_mess (ML (mlDbMess077));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.cst_price <= 0.0)
		{
			print_mess (ML (mlDbMess078));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		if (SR.salePrice <= local_rec.cst_price)
		{
			ws_ix = prmptmsg (ML (mlDbMess076),"YyNn",1,23);
			move (1,23);
			cl_line ();
			if (ws_ix != 'Y' && ws_ix != 'y')
				return (EXIT_FAILURE);
		}
		SR.costPrice = local_rec.cst_price;
		DSP_FLD ("cst_price");

		local_rec.margin = CalcMargin (line_cnt);
		SR.margin = local_rec.margin;
		DSP_FLD ("margin");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
CostsOk (void)
{
	int		i;
	char	ws_msg [100];

   	for (i = line_cnt, line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
   	{
    	getval (line_cnt);
		if (SR.costPrice <= 0 && SR.hhsuHash > 0L)
		{
			sprintf (ws_msg, ML (mlDbMess079), line_cnt + 1);
			print_mess (ws_msg);
			sleep (sleepTime);
			clear_mess ();
			line_cnt = i;
			getval (line_cnt);
			return (EXIT_FAILURE);
		}
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}


void
Update (void)
{
	rv_pr (ML (mlStdMess035), 20, 2, 0);
	
   	/*
   	 * Set to Tabular Screen (s) to Update Discount Details.
   	 */
   	scn_set (2);
	
	strcpy (cnch_rec.cont_no, local_rec.cnt_no);
	strcpy (cnch_rec.desc, local_rec.desc);
	strcpy (cnch_rec.contact, local_rec.contact);
	strcpy (cnch2_rec.co_no, comm_rec.co_no);
	strcpy (cnch2_rec.cont_no, local_rec.cnt_no);
	cc = find_rec (cnch, &cnch2_rec, EQUAL, "r");
	if (cc)
	{
		cc = abc_add (cnch, &cnch_rec);
		if (cc)
			file_err (cc, cnch, "DBADD");

		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, local_rec.cnt_no);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cnch, "DBFIND");
	}
	else
	{
		cc = abc_update (cnch, &cnch_rec);
		if (cc)
			file_err (cc, cnch, "DBUPDATE");
	}

	cncd_rec.hhch_hash = cnch_rec.hhch_hash;
	cncd_rec.line_no = 0;
	cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	while (!cc && cncd_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		cc = abc_delete (cncd);
	  	if (cc)
			file_err (cc, cncd, "DBDELETE");

		cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	}

   	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
   	{
    	getval (line_cnt);

		cncd_rec.hhch_hash	= cnch_rec.hhch_hash;
		cncd_rec.line_no	= line_cnt;
		cncd_rec.hhbr_hash	= SR.hhbrHash;
		cncd_rec.hhcc_hash	= SR.hhccHash;
		cncd_rec.hhsu_hash	= SR.hhsuHash;
		cncd_rec.price		= no_dec (SR.salePrice);
		strcpy (cncd_rec.curr_code, SR.currCode);
		strcpy (cncd_rec.disc_ok, SR.discountOk);
		cncd_rec.cost		= no_dec (SR.costPrice);

		cc = abc_add (cncd, &cncd_rec);
		if (cc)
			file_err (cc, cncd, "DBADD");
	}
	strcpy (local_rec.last_cnt_no, local_rec.cnt_no);
}

/*
 * Screen Heading.
 */
int
heading (
	int		scn)
{
	int	ws_ix;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		fflush (stdout);
		rv_pr (ML (mlDbMess095), (ScreenWidth -40)/2, 0, 1);

		print_at (0,ScreenWidth - 22,ML (mlDbMess096), local_rec.last_cnt_no);
		fflush (stdout);
		line_at (1,0, ScreenWidth - 1);

		if (scn == 1)
		{
			if (envDbMcurr)
			{
				box (0, 3, ScreenWidth - 1, 10);
				line_at (7,1, ScreenWidth - 2);
				line_at (12,1, ScreenWidth - 2);
			}
			else
			{
				box (0, 3, ScreenWidth - 1, 8);
				line_at (7,1, ScreenWidth - 2);
			}
		}
		scn_set (scn);
		scn_write (scn);
		if (prog_status != ENTRY)
			scn_display (scn);

		line_at (20,1, ScreenWidth - 1);

		sprintf (err_str,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0,err_str);

		if (scn == 2 && firstTime == TRUE && gwsNoEdit)
		{
			ws_ix = prmptmsg (ML (mlDbMess144),"",1,23);
			ws_ix = prmptmsg (ML (mlStdMess042),"",3,23);
			move (1,23);
			cl_line ();
			firstTime = FALSE;
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		fflush (stdout);
	}
    return (EXIT_SUCCESS);
}

/*
 * Search for Customer Contract. 
 */
void
SrchCnch (
	char	*key_val)
{
	_work_open (6,0,40);
	save_rec ("#No.","#Contract Description      ");

	strcpy (cnch_rec.co_no	,comm_rec.co_no);
	strcpy (cnch_rec.cont_no	,key_val);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strncmp (cnch_rec.cont_no, key_val,strlen (key_val)))
	{
		cc = save_rec (cnch_rec.cont_no, cnch_rec.desc);
		if (cc)
			break;
		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cnch_rec.co_no	,comm_rec.co_no);
	strcpy (cnch_rec.cont_no	,key_val);
	cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cnch, "DBFIND");
}

/*
 * Search for Currency Code.
 */
void
SrchPocr (
	char	*key_val)
{
	_work_open (3,0,40);
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, key_val);
	cc = save_rec ("#No ","#Currency Description");
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (pocr_rec.code, key_val, strlen (key_val)) && 
		   !strcmp (pocr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code,  temp_str);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, pocr, "DBFIND");
}

void
SrchEsmr (
	char	*key_val)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = save_rec ("#No ","#Branch Description");
	cc = find_rec (esmr,&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp (esmr_rec.co_no,comm_rec.co_no) &&
		      !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no,esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",temp_str);
	cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
	char	*key_val)
{
	_work_open (2,0,40);
	cc = save_rec ("#No.","#Warehouse Description");
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",key_val);
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) &&
		      !strcmp (ccmr_rec.est_no,local_rec.br_no) &&
		      !strncmp (ccmr_rec.cc_no,key_val,strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no,ccmr_rec.name);
		if (cc)
			break;
		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,local_rec.br_no);
	sprintf (ccmr_rec.cc_no,"%2.2s",temp_str);
	cc = find_rec (ccmr,&ccmr_rec,EQUAL,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

/*
 * Delete line.
 */
int
DeleteLine (void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		blank_display ();
		return (EXIT_FAILURE);
	}

	if (lcount [2] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [2]; line_cnt++)
	{
		strcpy (store [line_cnt].itemNo, store [line_cnt + 1].itemNo);
		strcpy (store [line_cnt].itemDesc, store [line_cnt + 1].itemDesc);
		store [line_cnt].hhbrHash = store [line_cnt + 1].hhbrHash;
		store [line_cnt].hhccHash = store [line_cnt + 1].hhccHash;
		store [line_cnt].hhsuHash = store [line_cnt + 1].hhsuHash;
		store [line_cnt].hhclHash = store [line_cnt + 1].hhclHash;
		store [line_cnt].salePrice = store [line_cnt + 1].salePrice;
		strcpy (store [line_cnt].currCode, store [line_cnt + 1].currCode);
		store [line_cnt].exchRate = store [line_cnt + 1].exchRate;
		store [line_cnt].costPrice = store [line_cnt + 1].costPrice;
		strcpy (store [line_cnt].discountOk, store [line_cnt + 1].discountOk);
		strcpy (store [line_cnt].rebateCode, store [line_cnt].rebateCode);

		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	while (line_cnt <= lcount [2])
	{
		sprintf (local_rec.item_no,"%-16.16s"," ");
		sprintf (local_rec.item_desc,"%-40.40s"," ");
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			blank_display ();

		line_cnt++;
	}

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Load cncd data for table 2 display 
 */
int
LoadCncd (void)
{
	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (2);

	init_vars (2);

	lcount [2] = 0;

	/*
	 * Prevents entry if not all lines loaded.
	 */
	cncd_rec.hhch_hash = cnch_rec.hhch_hash;
	cncd_rec.line_no   = 0;
	cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	while (!cc && cncd_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		/*
		 * Put Value Into Tabular Screen.
		 */
		abc_selfield (inmr, "inmr_hhbr_hash");
		inmr_rec.hhbr_hash = cncd_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cncd, &cncd_rec, NEXT, "r");
			continue;
		}

		strcpy (store [lcount [2]].itemNo, inmr_rec.item_no);
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (store [lcount [2]].itemDesc, inmr_rec.description);
		strcpy (local_rec.item_desc, inmr_rec.description);
		store [lcount [2]].hhbrHash = cncd_rec.hhbr_hash;
		store [lcount [2]].hhccHash = cncd_rec.hhcc_hash;
		store [lcount [2]].hhsuHash = cncd_rec.hhsu_hash;
		store [lcount [2]].salePrice = cncd_rec.price;
		strcpy (store [lcount [2]].currCode, cncd_rec.curr_code);
		store [lcount [2]].costPrice = cncd_rec.cost;
		strcpy (store [lcount [2]].discountOk, cncd_rec.disc_ok);
		strcpy (local_rec.disc_ok, cncd_rec.disc_ok);
		strcpy (store [lcount [2]].rebateCode, "    ");

		local_rec.sal_price = cncd_rec.price;

		abc_selfield (ccmr, "ccmr_hhcc_hash");
		if (cncd_rec.hhcc_hash == 0L)
		{
			strcpy (local_rec.br_no, "  ");
			strcpy (local_rec.wh_no, "  ");
			strcpy (store [lcount [2]].brNo, "  ");
		}
		else
		{
			ccmr_rec.hhcc_hash = cncd_rec.hhcc_hash;
			cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
			if (cc)
			{
				cc = find_rec (cncd, &cncd_rec, NEXT, "r");
				continue;
			}
	
			strcpy (local_rec.br_no, ccmr_rec.est_no);
			strcpy (local_rec.wh_no, ccmr_rec.cc_no);
			strcpy (store [lcount [2]].brNo, ccmr_rec.est_no);
			abc_selfield (ccmr, "ccmr_id_no");
		}

		if (cncd_rec.hhsu_hash == 0L)
		{
			strcpy (local_rec.crd_no, "      ");
			strcpy (sumr_rec.acronym, "         ");
			strcpy (sumr_rec.curr_code, "   ");
			strcpy (store [lcount [2]].supplierCurr, "   ");
		}
		else
		{
			sumr_rec.hhsu_hash = cncd_rec.hhsu_hash;
			cc = find_rec (sumr2,&sumr_rec,GTEQ,"r");
			if (cc)
			{
				cc = find_rec (cncd, &cncd_rec, NEXT, "r");
				continue;
			}

			strcpy (local_rec.crd_no, sumr_rec.crd_no);
			strcpy (local_rec.crd_short, sumr_rec.acronym);
			strcpy (store [lcount [2]].supplierCurr, sumr_rec.curr_code);
		}

		local_rec.cst_price = cncd_rec.cost;

		strcpy (local_rec.sal_curr, cncd_rec.curr_code);

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, local_rec.sal_curr);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cncd, &cncd_rec, NEXT, "r");
			continue;
		}
		store [lcount [2]].exchRate = pocr_rec.ex1_factor;
		local_rec.margin = CalcMargin (lcount [2]);
		store [lcount [2]].margin = local_rec.margin;

		putval (lcount [2]++);

		cc = find_rec (cncd, &cncd_rec, NEXT, "r");
	}

	/*
	 * Return to screen 1.
	 */
	scn_set (1);

	return (lcount [2]);
}


int
FindTableIndex (
	long	wsHhbrHash,
	char	*wsCurrCode,
	int		wsMaxItem)
{
	int				i;

	for (i = 0; i < wsMaxItem; i++)
	{
		if ((wsHhbrHash == store [i].hhbrHash) &&
			 (strcmp (wsCurrCode, store [i].currCode) == 0))
			return (i);
	}
	return (i);
}

float
CalcMargin (
	int		lineNo)
{
	float	ws_margin;
	double	ws_lcl_sal;
	double	ws_lcl_cst;

	ws_lcl_sal = store [lineNo].salePrice / store [lineNo].exchRate;
	if (ws_lcl_sal == 0.00)
		return ((float) ws_lcl_sal);

	if (store [lineNo].costPrice == 0.00)
	{
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = store [lineNo].hhbrHash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (!cc)
			ws_lcl_cst = CENTS (inei_rec.last_cost);
		else
			ws_lcl_cst = CENTS (1.00);
	}
	else
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, store [lineNo].supplierCurr);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");
		ws_lcl_cst = store [lineNo].costPrice / pocr_rec.ex1_factor;
	}

	ws_margin = (float) ((ws_lcl_sal - ws_lcl_cst) / ws_lcl_sal) * 100;
	return (ws_margin);
}
