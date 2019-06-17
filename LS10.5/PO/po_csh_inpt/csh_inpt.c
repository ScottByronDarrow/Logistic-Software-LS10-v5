/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: csh_inpt.c,v 5.9 2002/07/24 08:39:04 scott Exp $
|  Program Name  : (po_csh_inpt.c )                                   |
|  Program Desc  : (Purchase Order Shipment Costing.            )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 22/06/90         |
|---------------------------------------------------------------------|
| $Log: csh_inpt.c,v $
| Revision 5.9  2002/07/24 08:39:04  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/18 07:00:27  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.7  2002/06/20 07:22:08  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2002/06/19 07:00:40  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2001/11/22 01:42:16  scott
| Updated to fix busy message staying on screen
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: csh_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_csh_inpt/csh_inpt.c,v 5.9 2002/07/24 08:39:04 scott Exp $";

#define MAXWIDTH 	160
#include 	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<ml_cr_mess.h>

#ifndef GVISION
#define NDEBUG
#endif	// GVISION

#ifndef NDEBUG
#define DEBUG 
#endif
#include <assert.h>

#define	SH_HEAD		1
#define	SH_COST		2

#define	MAX_POS		3000 

#define	STORE	store [line_cnt]
#define	POGD	store2 [line_cnt]

#define	FOB	0
#define	FRT	1
#define	INS	2
#define	INT	3
#define	B_C	4
#define	DTY	5
#define	O_1	6
#define	O_2	7
#define	O_3	8
#define	O_4	9
#define	MAX_POGD	10

#define	GOODS_VAL	 (store2 [FOB].storeInvValue)
#define	FRT_INS_VAL	 (store2 [FRT].storeInvValue + store2 [INS].storeInvValue)
#define	FIN_CHG_VAL	 (store2 [INT].storeInvValue + store2 [B_C].storeInvValue)
#define	DUTY_VAL	 (store2 [DTY].storeInvValue)
#define	OTHER_VAL	 (store2 [O_1].storeInvValue + store2 [O_2].storeInvValue +\
			 		  store2 [O_3].storeInvValue + store2 [O_4].storeInvValue)

#include	"schema"

struct commRecord	comm_rec;
struct exsiRecord	exsi_rec;
struct inisRecord	inis_rec;
struct poshRecord	posh_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poslRecord	posl_rec;
struct pocrRecord	pocr_rec;
struct polhRecord	polh_rec;
struct podtRecord	podt_rec;
struct pocfRecord	pocf_rec;
struct pogdRecord	pogd_rec;
struct suinRecord	suin_rec;
struct insfRecord	insf_rec;
struct inccRecord	incc_rec;

	char	*data = "data";

	int		envCrCo		= 0;
	int		envCrFind 	= 0;
	int		maxPoLines 	= 0;

	char	branchNumber [3],
			*fifteenSpaces	=	"               ";

	long	hhpo_hash [MAX_POS];

	char	catDesc [10] [21];
	char	*invCat [] = 
			{
				"Goods (FOB)",
				"O/S Freight",
				"O/S Insurance",
				"O/S Interest",
				"O/S Bank Charges",
				"Duty",
				"Other - 1",
				"Other - 2",
				"Other - 3",
				"Other - 4",
			};

	char	envCurrCode [4];
	char	*screens [] = 
			{
				" Header Screen ",
				" Costing Details ",
				" Item Details ",
			};

	char	promptLocalValue [15];
	char	ShipDefault [2];

	int	new_pogd;

struct store2Rec{
	char	storeCurrency [4];
	double	storeExchange;
	double	storeInvValue;			/* invoice value (less gst)		*/
	double	storeItemValue;			/* extension of items & fob's	*/
	int		invFound;				/* Invoice found on suin.		*/
	long	hhsu_hash;				/* supplier hash				*/
	char	storeAllocation [2];	/* allocation D / W / V			*/
	char	storeCostEdit [2];		/* Y(es) if cost edited.		*/
} store2 [TABLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct storeRec {

	long	hhpl_hash;
	double	quantity;		/* quantity received		*/
	double	outer;			/* outer size.       		*/
	double	imp_duty;		/* duty rate from podt		*/
	char	duty_type [2];		/* duty type from podt		*/
	char	duty_code [3];		/* duty type from podt		*/
	double	duty_val;
	char	lic_cat [2];		/* licence category from polh.  */
	char	lic_no [11];		/* licence no from polh.  	*/
	double	lic_hash;		/* licence hhlc_hash from polh	*/
	double	lic_rate;		/* licence rate from polh	*/
	double	lic_val;		/* licence rate from polh	*/
	double	fob_fgn;
	double	fi_loc;
	double	cif_loc;
	double	other;
	double	land_cst;
	double	fob_cost;
	double	weight;			/* inis_weight			*/
	double	volume;			/* inis_volume			*/
	long	hhbr_hash;		/* inis_hhbr_hash		*/
	long	hhsu_hash;		/* inis_hhsu_hash		*/

} store [MAX_POS];

struct {
	char	dummy [11];
				/*======================================*/
				/* Costing Screen Local field.		*/
				/*======================================*/
	char	category [21];	/* Costing Category.			*/
	char	supplier [7];	/* Costing screen Supplier.		*/
	char	allocation [2];	/* Allocation D(ollar) W(eight) V(olume */
	char	inv_no [16];	/* Invoice Number.			*/
	char	currency [4];	/* Currency Code.			*/
	double	fgn_val;	/* Fgn Value.				*/
	double	lexch_rate;	/* Exchange Rate.			*/
	double	loc_val;	/* Local Value.				*/
				/*--------------------------------------*/

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "ship_no",	 4, 20, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		"0", "0", "Shipment No.", " ",
		 NE, NO, JUSTRIGHT, "", "", posh_rec.csm_no},
	{1, LIN, "curr_code",	 5, 20, CHARTYPE,
		"UUU", "          ",
		" ", "", "Currency.", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.curr_code},
	{1, LIN, "curr_desc",	 5, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "curr_rate",	 6, 20, DOUBLETYPE,
		"NNNNN.NNNNNNNN", "          ",
		" ", "", "Exch. Rate.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.ex_rate},
	{1, LIN, "ship_depart",	 7, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Ship Depart.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.ship_depart},
	{1, LIN, "ship_arrive",	 7, 80, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Ship Arrive.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.ship_arrive},
	{1, LIN, "ship_method",	 8, 20, CHARTYPE,
		"U", "          ",
		" ", ShipDefault, "Ship Method.", "Shipment Method L (and) / S (ea) / A (ir)",
		YES, NO,  JUSTLEFT, "LSA", "", posh_rec.ship_method},
	{1, LIN, "vessel",	 8, 80, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Vessel.", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.vessel},
	{1, LIN, "port",	10, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Port of Origin.", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.port},
	{1, LIN, "destination",	10, 80, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Destination.", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.destination},
	{1, LIN, "doc_rec",	11, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Documts Received.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.doc_rec},
	{1, LIN, "doc_agent",	12, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Docmts to Agent.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.doc_agent},
	{1, LIN, "neg_bol",	13, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Negotiable B.O.L.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.neg_bol},
	{1, LIN, "bol_no",	13, 80, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", "", "Bill of Lading No.", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.bol_no},
	{1, LIN, "costing_date",	14, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Costing Date.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&posh_rec.costing_date},
	{2, TAB, "category",	MAX_POGD, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Category      ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.category},
	{2, TAB, "supplier",	 0, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Supplier ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.supplier},
	{2, TAB, "hhsu_hash",	 0, 4, LONGTYPE,
		"NNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&sumr_rec.hhsu_hash},
	{2, TAB, "allocation",	 0, 3, CHARTYPE,
		"U", "          ",
		" ", "D", "Spread", " by : D(ollar  W(eight  V(olume ",
		YES, NO,  JUSTLEFT, "DWV", "", local_rec.allocation},
	{2, TAB, "inv_no",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "  Invoice No   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inv_no},
	{2, TAB, "currency",	 0, 4, CHARTYPE,
		"UUU", "          ",
		" ", " ", " Currency. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.currency},
	{2, TAB, "fgn_val",	 0, 1, DOUBLETYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "0", " Foreign Value ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.fgn_val},
	{2, TAB, "lexch_rate",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "0", " Exch. Rate ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lexch_rate},
	{2, TAB, "loc_val",	 0, 0, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "0", promptLocalValue, " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_val},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
double DutyCalculate 			 (int);
double WeightCalc 				 (int, int, double, double, double, double);
int		spec_valid 				 (int);
int 	CheckInvoice 			 (char *);
int 	LoadLines 				 (long);
int 	SaveMissing 			 (long, long);
int 	SrchPocf 				 (char *);
int 	heading 				 (int);
static double DollarTotal 		 (void);
static double DutyTotal   		 (void);
static double VolumeTotal 		 (void);
static double WeightTotal 		 (void);
void 	AddHash 				 (long);
void 	CalcCosting 			 (void);
void 	CalculateCost 			 (int);
void 	CloseDB 				 (void);
void 	DiscountScreen 			 (void);
void 	LoadCategoryDescription (void);
void 	LoadInvoiceScreen 		 (void);
void 	OpenDB 					 (void);
void 	OpenMissing 			 (char *);
void 	PrintCompany 			 (void);
void 	PrintCostDesc 			 (int);
void 	SetEditStuff 			 (int);
void 	ShowMissing 			 (void);
void 	SrchPocr 				 (char *);
void 	SrchPosh 				 (char *);
void 	SrchSuin 				 (char *, long);
void 	Update 					 (void);
void 	UpdateInsf 				 (void);
void 	UpdatePoln 				 (long, int);
void 	tab_other 				 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	char	*sptr;

	SETUP_SCR (vars);


	init_scr ();
	set_tty (); 
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store2, sizeof (struct store2Rec));
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	init_vars (SH_HEAD);

	envCrFind = atoi (get_env ("CR_FIND"));
	envCrCo = atoi (get_env ("CR_CO"));

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = screens [i];

	/*------------------------
	| Set up Screen Prompts. |
	------------------------*/
	sprintf (envCurrCode,"%-3.3s",get_env ("CURR_CODE"));
	sprintf (promptLocalValue,"  %-3.3s Value  ",envCurrCode);

	/*------------------------------------------
	| Shipment Default. A (ir) / L (and) / S (ea) |
	------------------------------------------*/
	sptr = chk_env ("PO_SHIP_DEFAULT");
	if (sptr == (char *) 0)
		sprintf (ShipDefault, "S");
	else
	{
		switch (*sptr)
		{
		case	'S':
		case	's':
			sprintf (ShipDefault, "S");
			break;

		case	'L':
		case	'l':
			sprintf (ShipDefault, "L");
			break;

		case	'A':
		case	'a':
			sprintf (ShipDefault, "A");

		default:
			sprintf (ShipDefault, "S");
			break;
		}
	}

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	/*-----------------------------
	| Load category descriptions. |	
	-----------------------------*/
	LoadCategoryDescription ();

	while (prog_exit == 0)
	{
		if (restart) 
			abc_unlock (posh);

		for (i = 0;i < MAX_POS;i++)
		{
			if (i < MAX_POGD)
			{
				strcpy (store2 [i].storeCurrency,"   ");
				strcpy (store2 [i].storeCostEdit, "N");
				store2 [i].storeExchange 		= 1.00;
				store2 [i].storeInvValue 	= 0.00;
				store2 [i].storeItemValue 	= 0.00;
				store2 [i].invFound 	= FALSE;
				store2 [i].hhsu_hash 	= 0L;
			}
			strcpy (store [i].duty_type, " ");
			strcpy (store [i].duty_code, "  ");
			strcpy (store [i].lic_cat, "  ");
			strcpy (store [i].lic_no , "          ");
			store [i].duty_val 	= 0.00;
			store [i].hhpl_hash = 0L;
			store [i].quantity 	= 0.00;
			store [i].outer 	= 0.00;
			store [i].imp_duty 	= 0.00;
			store [i].lic_hash 	= 0L;
			store [i].lic_rate 	= 0.00;
			store [i].lic_val 	= 0.00;
			store [i].fob_fgn 	= 0.00;
			store [i].fi_loc 	= 0.00;
			store [i].cif_loc 	= 0.00;
			store [i].other 	= 0.00;
			store [i].land_cst 	= 0.00;
			store [i].weight 	= 0.00;
			store [i].volume 	= 0.00;
		}
		eoi_ok		= TRUE;
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		init_vars (SH_HEAD);	
		init_vars (SH_COST);	
		lcount [SH_COST] =  0;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (SH_HEAD);
		entry (SH_HEAD);

		if (prog_exit || restart)
			continue;

		scn_display (SH_HEAD);

		/*------------------
		| Edit all screens.| 
		------------------*/
		edit_all ();

		if (restart)
			continue;
		Update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_id_no");
	open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posl);
	abc_fclose (polh);
	abc_fclose (podt);
	abc_fclose (inis);
	abc_fclose (pogd);
	abc_fclose (pocr);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Shipment Number. |
	---------------------------*/
	if (LCHECK ("ship_no"))
	{
		if (SRCH_KEY)
    	{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
    	}
		strcpy (posh_rec.co_no,comm_rec.co_no);
		strcpy (posh_rec.csm_no,zero_pad (posh_rec.csm_no,12));
	
    	cc = find_rec (posh,&posh_rec,COMPARISON,"w");
    	if (cc)
		{
			abc_unlock (posh);
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if ( posh_rec.status [0] != 'I' && posh_rec.status [0] != 'R')
		{
			sprintf (err_str, ML (mlPoMess010), posh_rec.status);
			abc_unlock (posh);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code,posh_rec.curr_code);
		cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (LoadLines (posh_rec.hhsh_hash))
		{
			sprintf (err_str, ML (" No Purchase order lines on Shipment %s. "),posh_rec.csm_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		LoadInvoiceScreen ();
		entry_exit = TRUE;
		store2 [FOB].storeExchange = posh_rec.ex_rate;

	    	return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| Validate Supplier (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("supplier"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		sprintf (err_str,"%-6.6s",local_rec.supplier);

		if (prog_status == ENTRY)
			getval (line_cnt);

		sprintf (local_rec.supplier,"%-6.6s",err_str);

		if (dflt_used || line_cnt == FOB)
		{
			if (!dflt_used)
			{
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
				strcpy (local_rec.supplier,"      ");
			}
			strcpy (POGD.storeCurrency,"   ");
			POGD.hhsu_hash = 0L;
			sumr_rec.hhsu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supplier));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (POGD.invFound)
		{
			open_rec (suin,suin_list,SUIN_NO_FIELDS,"suin_id_no2");
			suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
			sprintf (suin_rec.inv_no,"%-15.15s",local_rec.inv_no);
			cc = find_rec (suin,&suin_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML ("Cannot change the Supplier."));
				sleep(sleepTime);
				abc_fclose (suin);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			strcpy (POGD.storeCurrency,sumr_rec.curr_code);
			strcpy (local_rec.currency,POGD.storeCurrency);
			POGD.hhsu_hash = sumr_rec.hhsu_hash;
			DSP_FLD ("currency");

			strcpy (pocr_rec.co_no,comm_rec.co_no);
			strcpy (pocr_rec.code,local_rec.currency);
			cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
			if (cc)
			{
				/*--------------------------
				| Currency Code not found. |
				--------------------------*/
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			local_rec.lexch_rate = pocr_rec.ex1_factor;
			DSP_FLD ("lexch_rate");
			DSP_FLD ("fgn_val");

		}
		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val = local_rec.fgn_val/local_rec.lexch_rate;
		DSP_FLD ("loc_val");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Allocation (costing Screen). |
	-----------------------------------------*/
	if (LCHECK ("allocation"))
	{
		if (line_cnt == FOB)
		{
			strcpy (local_rec.allocation,"D");
			DSP_FLD ("allocation");
		}
		strcpy (POGD.storeAllocation,local_rec.allocation);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Invoice Number (Costing Screen.) |
	---------------------------------------------*/
	if (LCHECK ("inv_no"))
	{
		if (line_cnt == FOB)
		{
			print_mess (ML (mlPoMess059));
			sleep (sleepTime);
			POGD.invFound = FALSE;
			strcpy (local_rec.inv_no,fifteenSpaces);
			DSP_FLD ("inv_no");
			return (EXIT_SUCCESS);
		}
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		open_rec (suin,suin_list,SUIN_NO_FIELDS,"suin_id_no2");
		if (SRCH_KEY)
		{
			if (POGD.hhsu_hash == 0L)
			{
				print_mess (ML (mlPoMess063));
				abc_fclose (suin);
				return (EXIT_SUCCESS);
			}
			SrchSuin (temp_str, POGD.hhsu_hash);
			abc_fclose (suin);
			return (EXIT_SUCCESS);
		}
			
		if (POGD.hhsu_hash != 0L && strcmp (local_rec.inv_no,fifteenSpaces))
		{
			suin_rec.hhsu_hash = POGD.hhsu_hash;
			sprintf (suin_rec.inv_no,"%-15.15s",local_rec.inv_no);
			cc = find_rec (suin,&suin_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess115));
				abc_fclose (suin);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (suin_rec.approved [0] != 'Y')
			{
				print_mess (ML (mlCrMess052));
				abc_fclose (suin);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			POGD.invFound = TRUE;

			local_rec.loc_val = suin_rec.amt - suin_rec.gst;
			local_rec.fgn_val = DOLLARS (local_rec.loc_val);

			if (suin_rec.exch_rate != 0)
				local_rec.loc_val /= suin_rec.exch_rate;

			local_rec.loc_val = DOLLARS (local_rec.loc_val);

			POGD.storeExchange = suin_rec.exch_rate;
			local_rec.lexch_rate = suin_rec.exch_rate;
			strcpy(local_rec.currency, suin_rec.currency);

			DSP_FLD ("currency");
			DSP_FLD ("fgn_val");
			DSP_FLD ("lexch_rate");
			DSP_FLD ("loc_val");
		}
		else
			POGD.invFound = FALSE;

		if (line_cnt == FOB)
			POGD.storeInvValue = local_rec.fgn_val;
		else
			POGD.storeInvValue = local_rec.loc_val;

		SetEditStuff (line_cnt);
		abc_fclose (suin);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Validate Currency (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("currency"))
	{

		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || line_cnt == FOB)
			strcpy (local_rec.currency,POGD.storeCurrency);
	
		if (strcmp (local_rec.currency,"   "))
		{
			strcpy (pocr_rec.co_no,comm_rec.co_no);
			strcpy (pocr_rec.code,local_rec.currency);
			cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			local_rec.lexch_rate = pocr_rec.ex1_factor;
			POGD.storeExchange = pocr_rec.ex1_factor;
		}
		else
		{
			if (line_cnt != FOB)
			{
				local_rec.lexch_rate = 1.00;
				POGD.storeExchange = 1.00;
			}
		}

		DSP_FLD ("lexch_rate");

		if (POGD.invFound)
		{
			local_rec.fgn_val = local_rec.loc_val;
			local_rec.fgn_val *= local_rec.lexch_rate;
			DSP_FLD ("fgn_val");

			skip_entry = 4;
		}
		else
		{
			if (prog_status != ENTRY)
			{
				local_rec.loc_val = local_rec.fgn_val;
				if (local_rec.lexch_rate != 0.00)
					local_rec.loc_val /= local_rec.lexch_rate;

				DSP_FLD ("loc_val");
			}
		}
		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.storeInvValue = local_rec.fgn_val;
		else
			POGD.storeInvValue = local_rec.loc_val;
			
		SetEditStuff (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate FGN_VAL Costing Screen. |
	----------------------------------*/
	if (LCHECK ("fgn_val"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used || line_cnt == FOB)
		{
			CalcCosting ();
			local_rec.fgn_val = POGD.storeInvValue;
			if (!dflt_used)
			{
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
			}
		}

		/*-----------------------------------------------
		| Cannot Change Foreign Value if Invoice Exists	|
		-----------------------------------------------*/
		local_rec.loc_val = local_rec.fgn_val;
		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val /= local_rec.lexch_rate;

		DSP_FLD ("loc_val");

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.storeInvValue = local_rec.fgn_val;
		else
			POGD.storeInvValue = local_rec.loc_val;

		SetEditStuff (line_cnt);
	}

	/*---------------------------------------------
	| Validate Exchange Rate. (Costing Screen.) |
	---------------------------------------------*/
	if (LCHECK ("lexch_rate"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
			
		if (dflt_used || line_cnt == FOB)
		{
			local_rec.lexch_rate = POGD.storeExchange;
	
			if (!dflt_used)
			{
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
			}
		}
		local_rec.loc_val = local_rec.fgn_val;
		if (local_rec.lexch_rate != 0.00)
			local_rec.loc_val /= local_rec.lexch_rate;

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.storeInvValue = local_rec.fgn_val;
		else
			POGD.storeInvValue = local_rec.loc_val;

		DSP_FLD ("loc_val");

		if (dflt_used && prog_status == ENTRY)
			skip_entry++;

		POGD.storeExchange = local_rec.lexch_rate;
		SetEditStuff (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------
	| Calculate NZL_VAL (Costing Screen.) |
	---------------------------------------*/
	if (LCHECK ("loc_val"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		CalcCosting ();
		if (dflt_used || line_cnt == FOB)
		{
			local_rec.loc_val = local_rec.fgn_val;

			if (local_rec.lexch_rate != 0.00)
				local_rec.loc_val /= local_rec.lexch_rate;

			DSP_FLD ("loc_val");
			if (!dflt_used)
			{
				print_mess (ML (mlPoMess059));
				sleep (sleepTime);
			}
		}
		else
		{
			local_rec.fgn_val = local_rec.loc_val;
			local_rec.fgn_val *= local_rec.lexch_rate;

			DSP_FLD ("fgn_val");
		}

		/*-------------------------------
		| Hold Fgn value for goods	|
		-------------------------------*/
		if (line_cnt == FOB)
			POGD.storeInvValue = local_rec.fgn_val;
		else
			POGD.storeInvValue = local_rec.loc_val;

		SetEditStuff (line_cnt);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*============================
| Find Country freight file. |
============================*/
int
SrchPocf (
 char *code)
{
	open_rec (pocf,pocf_list,POCF_NO_FIELDS,"pocf_id_no");
	strcpy (pocf_rec.co_no,comm_rec.co_no);
	sprintf (pocf_rec.code,"%-3.3s", code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
	{
		print_mess (ML (mlStdMess118));
		abc_fclose (pocf);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	abc_fclose (pocf);
	return (EXIT_SUCCESS);
}
/*==================================================
| Calculate Duty on total quantity and each basis. |
==================================================*/
double	
DutyCalculate (
 int wk_line)
{
	double	value  = 0.00;

	/*-----------------------
	| Calculate Duty   	|
	-----------------------*/
	if (store [wk_line].duty_type [0] == 'D')
		value = store [wk_line].imp_duty;
	else
	{
		switch (store2 [DTY].storeAllocation [0])
        {
        case 'V':
            value = store [wk_line].volume;
			value *= (store [wk_line].imp_duty / 100);
			if (posh_rec.ex_rate != 0.00)
				value /= posh_rec.ex_rate;
            break;

        case 'W':
            value = store [wk_line].weight;
			value *= (store [wk_line].imp_duty / 100);
			if (posh_rec.ex_rate != 0.00)
				value /= posh_rec.ex_rate;
            break;

        default:
			value = store [wk_line].fob_fgn; 
			value *= (store [wk_line].imp_duty / 100);
			if (posh_rec.ex_rate != 0.00)
				value /= posh_rec.ex_rate;
			value  = value;
        }
	}
	return (value);
}

/*============================
| Calculate total line cost. |
============================*/
void
CalculateCost (
 int wk_line)
{
	double	cif_cost = 0.00;

	/*-----------------------
	| Calculate CIF FGN	|
	-----------------------*/
	cif_cost = store [wk_line].fob_fgn;

	/*-----------------------
	| Calculate CIF NZL	|
	-----------------------*/
	if (posh_rec.ex_rate != 0.00)
		store [wk_line].cif_loc = cif_cost / posh_rec.ex_rate;
	else
		store [wk_line].cif_loc = 0.00;

	store [wk_line].cif_loc = store [wk_line].cif_loc;
	store [wk_line].cif_loc += store [wk_line].fi_loc;
	store [wk_line].cif_loc = store [wk_line].cif_loc;

	/*-----------------------
	| Calculate Licence NZL	|
	-----------------------*/
	store [wk_line].lic_val  = (double) store [wk_line].lic_rate;
	store [wk_line].lic_val *= store [wk_line].cif_loc;
	store [wk_line].lic_val =  store [wk_line].lic_val;

	/*-------------------------------
	| Calculate Landed Cost NZL	|
	-------------------------------*/
	store [wk_line].land_cst = 	store [wk_line].cif_loc + 
			            		store [wk_line].duty_val + 
			            		store [wk_line].lic_val + 
			            		store [wk_line].other;

	store [wk_line].land_cst = 	store [wk_line].land_cst;
}

/*=======================================================================
| Routine to read all poln records whose hash matches the one on the    |
| pohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
int
LoadLines (
	long	hhshHash)
{
	int	lineNo = 0;

	/*------------------------
	| Set screen for putval. |
	------------------------*/
	strcpy (posl_rec.co_no, comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpl_hash = 0L;

	cc = find_rec (posl,&posl_rec,GTEQ,"r");
	while (!cc && posl_rec.hhsh_hash == hhshHash)
	{
		if (posl_rec.ship_qty <= 0.00)
		{
			cc = find_rec (posl,&posl_rec,NEXT,"r");
			continue;
		}

		poln_rec.hhpl_hash	=	posl_rec.hhpl_hash;
		cc = find_rec (poln,&poln_rec,EQUAL,"r");
		if (cc)
		{
			cc = find_rec (posl,&posl_rec,NEXT,"r");
			continue;
		}

		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec,EQUAL,"r");
		if (cc) 
		{
			cc = find_rec (posl,&posl_rec,NEXT,"r");
			continue;
		}

		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,EQUAL,"r");
		if (cc) 
		{
			cc = find_rec (posl,&posl_rec,NEXT,"r");
			continue;
		}

		store [lineNo].outer = (double) inmr_rec.outer_size;

		/*-------------------
		| Find duty record. |
		-------------------*/
		inis_rec.hhbr_hash = store [lineNo].hhbr_hash = inmr_rec.hhbr_hash;
		inis_rec.hhsu_hash = store [lineNo].hhsu_hash = pohr_rec.hhsu_hash;
		strcpy (inis_rec.co_no,comm_rec.co_no);
		strcpy (inis_rec.br_no,comm_rec.est_no);
		strcpy (inis_rec.wh_no,comm_rec.cc_no);
		cc = find_rec (inis,&inis_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (inis_rec.wh_no,"  ");
			cc = find_rec (inis,&inis_rec,COMPARISON,"r");
		}
		if (cc)
		{
			strcpy (inis_rec.br_no,"  ");
			strcpy (inis_rec.wh_no,"  ");
			cc = find_rec (inis,&inis_rec,COMPARISON,"r");
		}
		if (cc)
		{
			store [lineNo].weight = 0.00;
			store [lineNo].volume = 0.00;

			strcpy (store [lineNo].duty_code,inmr_rec.duty);
			strcpy (store [lineNo].lic_cat,inmr_rec.licence);
		}
		else
		{
			store [lineNo].weight = inis_rec.weight;
			store [lineNo].volume = inis_rec.volume;

			strcpy (store [lineNo].duty_code,inis_rec.duty);
			strcpy (store [lineNo].lic_cat,inis_rec.licence);

			strcpy (inmr_rec.duty,store [lineNo].duty_code);
			strcpy (inmr_rec.licence,store [lineNo].lic_cat);
		}
		if (store [lineNo].weight <= 0.0)
			store [lineNo].weight = inmr_rec.weight;

		if (!strcmp (store [lineNo].duty_code,"  "))
		{
			store [lineNo].imp_duty = 0.00;
			store [lineNo].duty_val = 0.00;
			strcpy (store [lineNo].duty_type," ");
		}
		else
		{
			strcpy (podt_rec.co_no,comm_rec.co_no);
			strcpy (podt_rec.code,store [lineNo].duty_code);
			cc = find_rec (podt,&podt_rec,COMPARISON,"r");
			if (cc) 
			{
				assert (FALSE);
				file_err (cc, "podt", "DBFIND");
			}
			store [lineNo].imp_duty = podt_rec.im_duty;
			strcpy (store [lineNo].duty_type,podt_rec.duty_type);
		}
		/*----------------------
		| Find licence record. |
		----------------------*/
		if (!strcmp (store [lineNo].lic_cat,"  ") ||
		      poln_rec.hhlc_hash == 0L)
		{
			strcpy (store [lineNo].lic_cat,"  ");
			strcpy (store [lineNo].lic_no,"          ");
			store [lineNo].lic_hash = 0L;
			store [lineNo].lic_rate = 0.00;
			strcpy (store [lineNo].lic_no,"          ");
			store [lineNo].lic_val = 0.00;
		}
		else
		{
			polh_rec.hhlc_hash = poln_rec.hhlc_hash;
			cc = find_rec (polh,&polh_rec,COMPARISON,"r");
			if (cc) 
			{
				assert (FALSE);
				file_err (cc, "polh", "DBFIND");
			}
			strcpy (store [lineNo].lic_cat, 	polh_rec.lic_cate);
			strcpy (store [lineNo].lic_no, 	polh_rec.lic_no);
			store [lineNo].lic_hash = 			polh_rec.hhlc_hash;
			store [lineNo].lic_rate = 			polh_rec.ap_lic_rate;
			strcpy (store [lineNo].lic_no,		polh_rec.lic_no);
			store [lineNo].lic_val = 			polh_rec.ap_lic_rate;
		}
		/*---------------------
		| Setup local record. |
		---------------------*/
		store [lineNo].hhpl_hash   = poln_rec.hhpl_hash;
		store [lineNo].quantity    = posl_rec.ship_qty;
		store [lineNo].fob_fgn     = poln_rec.fob_fgn_cst;
		store [lineNo].fi_loc      = poln_rec.frt_ins_cst;
		store [lineNo].cif_loc     = poln_rec.fob_nor_cst;
		store [lineNo].duty_val    = poln_rec.duty;
		store [lineNo].lic_val     = poln_rec.licence;
		store [lineNo].other       = poln_rec.lcost_load;
		store [lineNo].land_cst    = poln_rec.land_cst;

		/*----------------------------------------------------
		| Put this bit in here to handle change of other etc |
		----------------------------------------------------*/
		CalculateCost (lineNo);

		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lineNo++ > MAX_POS) 
			break;

		cc = find_rec (posl,&posl_rec,NEXT,"r");
	}
	maxPoLines = lineNo;

	if (lineNo == 0)
		return (EXIT_FAILURE);

	/*-------------------------
	| Normal exit - return 0. |
	-------------------------*/
	return (EXIT_SUCCESS);
}

/*==========================
| Update Relevent Records. |
==========================*/
void
Update (
 void)
{
	int 	i = 0;
	int	cost_edit = FALSE;

	clear ();

	DiscountScreen ();

	for (i = 0; i < MAX_POS; i++)
		hhpo_hash [i] = 0L;

	for (i = 0; i < MAX_POGD; i++)
		if (store2 [i].storeCostEdit [0] == 'Y')
			cost_edit = TRUE;

	rv_pr (ML (mlPoMess096),0,1,0);

	/*-----------------------------------
	| Process all purchase order lines. |
	-----------------------------------*/
	for (line_cnt = 0;line_cnt < maxPoLines;line_cnt++) 
		UpdatePoln (STORE.hhpl_hash, line_cnt);

	rv_pr (ML (mlPoMess097),0,2,0);

	abc_selfield (pogd,"pogd_id_no3");
	
	for (i = 0; i < MAX_POS; i++)
	{
		if (hhpo_hash [i] == 0L)
			break;

		strcpy (pogd_rec.co_no,comm_rec.co_no);
		pogd_rec.hhpo_hash = hhpo_hash [i];
		pogd_rec.line_no = 0;
		cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
		while (!cc && pogd_rec.hhpo_hash == hhpo_hash [i])
		{
			cc = abc_delete (pogd);
			if (cc)
			{
				assert (FALSE);
				file_err (cc, "pogd", "DBDELETE");
			}

			strcpy (pogd_rec.co_no,comm_rec.co_no);
			pogd_rec.hhpo_hash = hhpo_hash [i];
			pogd_rec.line_no = 0;
			cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
		}
	}
	abc_selfield (pogd,"pogd_id_no2");
	if (cost_edit)
	{
	    scn_set (SH_COST);

	    rv_pr (ML (mlPoMess098),0,3,0);
	    for (line_cnt = 0;line_cnt < MAX_POGD;line_cnt++)
	    {
			getval (line_cnt);

			strcpy (pogd_rec.co_no,comm_rec.co_no);
			pogd_rec.hhpo_hash = 0L;
			pogd_rec.hhgr_hash = 0L;
			pogd_rec.hhsh_hash = posh_rec.hhsh_hash;
			pogd_rec.line_no = line_cnt;
			cc = find_rec (pogd,&pogd_rec,COMPARISON,"u");
		
			strcpy (pogd_rec.category,local_rec.category);
			sprintf (pogd_rec.invoice,"%-15.15s",local_rec.inv_no);
			strcpy (pogd_rec.allocation,local_rec.allocation);
			strcpy (pogd_rec.cost_edit,POGD.storeCostEdit);
			pogd_rec.hhsu_hash = sumr_rec.hhsu_hash;
			sprintf (pogd_rec.currency,local_rec.currency);
			pogd_rec.foreign = local_rec.fgn_val;
			pogd_rec.exch_rate = local_rec.lexch_rate;
			pogd_rec.nz_value = local_rec.loc_val;

			if (!cc)
			{
				cc = abc_update (pogd,&pogd_rec);
				if (cc)
				{
					assert (FALSE);
					file_err (cc, "pogd", "DBUPDATE");
				}
			}
			else
			{
				cc = abc_add (pogd,&pogd_rec);
				if (cc)
				{
					assert (FALSE);
					file_err (cc, "pogd", "DBADD");
				}
			}
			abc_unlock (pogd);
	    }
	}
	scn_set (SH_HEAD);
}

/*========================================
| Update or Add lines to Purchase order. |
========================================*/
void
UpdatePoln (
	long	hhplHash, 
	int		i)
{
	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln,&poln_rec,COMPARISON,"u");
	if (cc)
	{
		assert (FALSE);
		file_err (cc, "poln", "DBFIND");
	}

	/*------------------------------------------------------
	| Add record for later update of p/order related info. |
	------------------------------------------------------*/
	AddHash (poln_rec.hhpo_hash);

	poln_rec.hhlc_hash   = (long) store [i].lic_hash;
	poln_rec.exch_rate   = posh_rec.ex_rate;
	poln_rec.fob_fgn_cst = store [i].fob_fgn;
	poln_rec.fob_nor_cst = store [i].cif_loc;
	poln_rec.frt_ins_cst = store [i].fi_loc;
	poln_rec.duty        = store [i].duty_val;
	poln_rec.licence     = store [i].lic_val;
	poln_rec.lcost_load  = store [i].other;
	poln_rec.land_cst    = store [i].land_cst;
	strcpy (poln_rec.stat_flag,"B");

	/*------------------------
	| Update existing order. |
	------------------------*/
	cc = abc_update (poln,&poln_rec);
	if (cc) 
	{
		assert (FALSE);
		file_err (cc, "poln", "DBUPDATE");
	}

	if (strcmp (poln_rec.serial_no,"                         "))
		UpdateInsf ();

	abc_unlock (poln);
}

/*=================================
| Update Pre-receipt Serial item. |
=================================*/
void
UpdateInsf (
 void)
{
	inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
	cc = find_rec (inmr,&inmr_rec,EQUAL,"r");
	if (cc)
		return;
	
	if (inmr_rec.serial_item [0] != 'Y')
		return;

	open_rec (incc,incc_list,INCC_NO_FIELDS,"incc_id_no");

	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	if (find_rec (incc,&incc_rec,COMPARISON,"r"))
	{
		abc_fclose (incc);
		return;
	}
	abc_fclose (incc);

	open_rec (insf,insf_list,INSF_NO_FIELDS,"insf_id_no2");

	insf_rec.hhwh_hash = incc_rec.hhwh_hash;
	sprintf (insf_rec.serial_no,"%-25.25s",poln_rec.serial_no);
	cc = find_rec (insf,&insf_rec,COMPARISON,"u");
	if (cc)
	{
		abc_unlock (insf);
		abc_fclose (insf);
		return;
	}
	insf_rec.exch_rate   = poln_rec.exch_rate;
	insf_rec.fob_fgn_cst = poln_rec.fob_fgn_cst;
	insf_rec.fob_nor_cst = poln_rec.fob_nor_cst;
	insf_rec.frt_ins_cst = poln_rec.frt_ins_cst;
	insf_rec.duty        = poln_rec.duty;
	insf_rec.licence     = poln_rec.licence;
	insf_rec.lcost_load  = poln_rec.lcost_load;

	insf_rec.land_cst    = poln_rec.fob_nor_cst +
				  			  poln_rec.lcost_load +
		          	  		  poln_rec.duty +
		          	  		  poln_rec.licence;

	insf_rec.istore_cost = poln_rec.land_cst;
	insf_rec.est_cost    = poln_rec.land_cst;

	cc = abc_update (insf,&insf_rec);
	if (cc)
	{
		assert (FALSE);
		file_err (cc, "insf", "DBUPDATE");
	}

	abc_unlock (insf);
	abc_fclose (insf);
	return;
}

/*======================
| Load Costing screen. |
======================*/
void
LoadInvoiceScreen (
 void)
{
	int	i;

	scn_set (SH_COST);
	init_vars (SH_COST);	

	abc_selfield (sumr,"sumr_hhsu_hash");

	CalcCosting ();

	for (i = 0; i < MAX_POGD; i++) 
	{
		strcpy (pogd_rec.co_no,comm_rec.co_no);
		pogd_rec.hhsh_hash = posh_rec.hhsh_hash;
		pogd_rec.line_no = i;
		cc = find_rec (pogd,&pogd_rec,COMPARISON,"r");
		if (!cc && posh_rec.hhsh_hash > 0L)
		{
			new_pogd = FALSE;
			store2 [i].hhsu_hash = pogd_rec.hhsu_hash;
			sumr_rec.hhsu_hash = pogd_rec.hhsu_hash;
			sprintf (local_rec.category,"%-20.20s",catDesc [i]);
			sprintf (local_rec.inv_no,"%-15.15s",pogd_rec.invoice);
			sprintf (local_rec.allocation,"%-1.1s",pogd_rec.allocation);
			store2 [i].invFound = (pogd_rec.hhsu_hash != 0L && strcmp (local_rec.inv_no,fifteenSpaces));

			if (pogd_rec.hhsu_hash != 0L)
			{
				sumr_rec.hhsu_hash	=	pogd_rec.hhsu_hash;
				cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
				if (cc)
				{
					assert (FALSE);
					sprintf (err_str,"Error in sumr (%06ld) During (DBFIND)",pogd_rec.hhsu_hash);
					sys_err (err_str,cc,PNAME);
				}
				sprintf (local_rec.supplier,"%-6.6s",sumr_rec.crd_no);
			}
			else
				sprintf (local_rec.supplier,"%-6.6s"," ");

			strcpy (store2 [i].storeCurrency,pogd_rec.currency);
			sprintf (local_rec.currency,"%-3.3s",pogd_rec.currency);
			local_rec.lexch_rate	= pogd_rec.exch_rate;

			local_rec.fgn_val		= pogd_rec.foreign;
			local_rec.loc_val		= pogd_rec.nz_value;

			/*-----------------------
			| Store fgn if goods	|
			-----------------------*/
			if (i == FOB)
			{
				local_rec.fgn_val = store2 [i].storeInvValue;
				local_rec.loc_val = local_rec.fgn_val;
				if (local_rec.lexch_rate != 0.00)
					local_rec.loc_val /= local_rec.lexch_rate;
			}
			else
				store2 [i].storeInvValue = local_rec.loc_val;

			strcpy (store2 [i].storeAllocation,pogd_rec.allocation);
			strcpy (store2 [i].storeCostEdit,pogd_rec.cost_edit);
		}
		else
		{
			store2 [i].storeExchange = 1.00;
			local_rec.lexch_rate = 1.00;

			if (i == FOB)
			{
				store2 [i].storeExchange		= posh_rec.ex_rate;
				local_rec.lexch_rate	= posh_rec.ex_rate;
			}

			strcpy (store2 [i].storeCurrency,"   ");
			store2 [i].hhsu_hash = pogd_rec.hhsu_hash;

			sprintf (local_rec.category,"%-20.20s",catDesc [i]);

			local_rec.fgn_val = store2 [i].storeInvValue;
			local_rec.loc_val = local_rec.fgn_val;
			if (local_rec.lexch_rate != 0.00)
				local_rec.loc_val /= local_rec.lexch_rate;

			strcpy (local_rec.allocation,"D");
			strcpy (store2 [i].storeAllocation, "D");
			strcpy (store2 [i].storeCostEdit, "N");
		}
		putval (i);
	}
	lcount [SH_COST] = MAX_POGD;
	vars [scn_start].row = lcount [SH_COST];

	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");

	scn_set (SH_HEAD);
	return;
}

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchSuin (
 char *key_val, 
 long hhsu_hash)
{
	char	disp_amt [22];
	double	inv_balance;
	
	work_open ();
	suin_rec.hhsu_hash = hhsu_hash;
	strcpy (suin_rec.inv_no,key_val);
	save_rec ("#Document","#  Base Currency ");
	cc = find_rec ("suin", &suin_rec, GTEQ, "r");
	while (!cc && !strncmp (suin_rec.inv_no, key_val,strlen (key_val)) && (suin_rec.hhsu_hash == hhsu_hash))
	{
		if (suin_rec.approved [0] == 'Y')
		{
			inv_balance = suin_rec.amt - suin_rec.gst;
			sprintf (disp_amt, "%-14.2f ",DOLLARS (inv_balance));
			cc = save_rec (suin_rec.inv_no, disp_amt);
			if (cc)
				break;
		}
		cc = find_rec ("suin", &suin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	suin_rec.hhsu_hash = hhsu_hash;
	strcpy (suin_rec.inv_no,temp_str);
	cc = find_rec ("suin", &suin_rec, COMPARISON, "r");
	if (cc)
	{
		assert (FALSE);
		file_err (cc, "suin", "DBFIND");
	}
}

/*===============================
| Search for shipment number.	|
===============================*/
void
SrchPosh (
 char *key_val)
{
    work_open ();
	save_rec ("#Shipment No","#Vessel Name - Date Ship Arrive");
	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", key_val);

	cc = find_rec (posh,&posh_rec,GTEQ,"r");
    while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no))
    {                        
		if ((posh_rec.status [0] == 'I' || 
		      posh_rec.status [0] == 'R') && 
		      !strncmp (posh_rec.csm_no, key_val, strlen (key_val)))
		{
			sprintf (err_str,"%20.20s  %s",posh_rec.vessel,DateToString (posh_rec.ship_arrive));
			cc = save_rec (posh_rec.csm_no,err_str);
			if (cc)
				break;
		}
		cc = find_rec (posh,&posh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no, temp_str);
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
	{
		assert (FALSE);
		file_err (cc, "posh", "DBFIND");
	}
}

/*=======================
| Search for currency	|
=======================*/
void
SrchPocr (
 char *key_val)
{
	work_open ();

	save_rec ("#Cur","#Currency Description");
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",key_val);
	cc = find_rec (pocr,&pocr_rec,GTEQ,"r");
	while (!cc && !strncmp (pocr_rec.code,key_val,strlen (key_val)) && !strcmp (pocr_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code,pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr,&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	if (cc)
	{
		assert (FALSE);
		file_err (cc, "pocr", "DBFIND");
	}
}


/*===============================================
| Setup for Missing Weight / Volume Display	|
===============================================*/
void
OpenMissing (
 char *desc)
{
	Dsp_open (2,9,6);

	sprintf (err_str,"   Missing %-6.6s Values   ",desc);
	Dsp_saverec (err_str);
	Dsp_saverec (" From Inventory / Supplier ");
	Dsp_saverec ("  [FN14]  [FN15]  [FN16]   ");
}

/*=======================================
| Save Supplier / Item No. for display	|
=======================================*/
int
SaveMissing (
	long	hhsuHash,
	long	hhbrHash)
{
	assert (hhsuHash != 0L);
	assert (hhbrHash != 0L);

	abc_selfield (sumr, "sumr_hhsu_hash");
	sumr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");

	if (!cc)
	{
		inmr_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (!cc)
		{
			sprintf (err_str," %-6.6s ^E %-16.16s ",	
					sumr_rec.crd_no, inmr_rec.item_no);
			return (Dsp_saverec (err_str));
		}
	}
	return (EXIT_SUCCESS);
}

/*===============
| Display Data	|
===============*/
void
ShowMissing (
 void)
{
	Dsp_srch ();
	Dsp_close ();
}


/*===============================
| Calculate Extended FOB total	|
===============================*/
static double
DollarTotal (
 void)
{
	int	i;
	double	value;
	double	total = 0.00;

	for (i = 0;i < maxPoLines;i++)
	{
		value = out_cost ((float) (store [i].fob_fgn), (float) (store [i].outer));
		value *= store [i].quantity;
		total += value;
	}
	total = total;
	return (total);
}

/*===========================================
| Calculate duty allocation per line	    |
|											|
| line1 extend_fob / exch_rate * duty% = A  |
| line2 extend_fob / exch_rate * duty% = B	|
| line3 extend_fob / exch_rate * duty% = C	|
|										--	|
|										 D	|
|											|
| line1    A/D*duty_invoice_total/line_qty	|
| line2    B/D*duty_invoice_total/line_qty	|
| line3    C/D*duty_invoice_total/line_qty	|
===========================================*/
static double
DutyTotal (
 void)
{
	int	i;
	double	total = 0.00;
	double	value = 0.00;

	for (i = 0;i < maxPoLines;i++)
	{
		value = out_cost ((float)DutyCalculate (i), (float)store [i].outer);
		store [i].duty_val = value;
		value *= store [i].quantity;
		total += value;
	}
	total = total;
	return (total);
}

/*===============================
| Calc Total Extended Weights	|
| display any items where the	|
| inis_weight is zero           |
===============================*/
static double
WeightTotal (
 void)
{
	int	i;
	int missing = 0;
	int	check;		/* check for missing weights	*/
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("W");

	for (i = 0;i < maxPoLines;i++)
	{
		if (check && store [i].weight <= 0.00)
		{
			if (!missing)
            {
                OpenMissing ("Weight");
                missing = 1;
            }
            if (SaveMissing (store [i].hhsu_hash, store [i].hhbr_hash))
                break;
		}
		value = store [i].weight;
		value *= store [i].quantity;
		total += value;
	}
	
	if (missing)
        ShowMissing ();

	return (total);
}

/*===============================
| Calc Total Extended Volumes	|
| display any items where the	|
| inis_volume is zero           |
===============================*/
static double
VolumeTotal (
 void)
{
	int	i;
 	int missing = 0;
	int	check;		/* check for missing volumes	*/
	double	value;
	double	total = 0.00;

	check = CheckInvoice ("V");

	for (i = 0;i < maxPoLines;i++)
	{
		if (check && store [i].volume <= 0.00)
		{
			if (!missing)
            {
                OpenMissing ("Volume");
                missing = 1;
            }
            if (SaveMissing (store [i].hhsu_hash, store [i].hhbr_hash))
                break;
		}
		value = store [i].volume;
		value *= store [i].quantity;
		total += value;
	}

	if (missing)
        ShowMissing ();

	return (total);
}


/*=======================================
| Check what weightings are being used	|
| Pass Weighting Type - 		  		|
|	D (ollar,V (olume,W (eight			|
| Return 1 if Weighting type is used	|
| otherwise 0.							|
=======================================*/
int
CheckInvoice (
 char *weight_type)
{
	int	i;

	for (i = FOB;i <= O_4;i++)
		if (store2 [i].storeAllocation [0] == weight_type [0])
			return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*===========================================
| Calculate Spreading of Invoices Received	|
| on a per unit basis						|
===========================================*/
double	
WeightCalc (
	int		indx, 
	int		lineNo, 
	double	fob_total, 
	double	wgt_total, 
	double	vol_total, 
	double	dty_total)
{
	double inv_value = store2 [indx].storeInvValue;
	double value = 0.0;

	if (indx == DTY)
	{
		value = store [lineNo].duty_val;

		if (dty_total != 0.00)
			value /= dty_total;
		else
			value = 0.00;

		value *= inv_value;
	}
	else
	{
		switch (store2 [indx].storeAllocation [0])
		{
		/*---------------
		| Weight        |
		---------------*/
		case 'W':
			value = store [lineNo].weight;
			if (wgt_total != 0.00)
				value /= wgt_total;
			else
				value = 0.00;
			value *= inv_value;
			break;

		/*---------------
		| Volume        |
		---------------*/
		case 'V':
			value = store [lineNo].volume;
			if (vol_total != 0.00)
				value /= vol_total;
			else
				value = 0.00;
			value *= inv_value;
			break;

		default:
			switch (indx)
			{
			case FOB:
				value = store [lineNo].fob_fgn;
				break;

			default:
				value = store [lineNo].fob_fgn;
				if (fob_total != 0.00)
					value /= fob_total;
				else
					value = 0.00;
				value *= inv_value;
				break;
			}
			break;
		}
	}
	return (value);
}


/*===============================================
| Recalculate & Display screen for every change	|
===============================================*/
void
DiscountScreen (
 void)
{
	int	lineNo;

	double	cif_cost;
	double	value [MAX_POGD];
	double	fob_total = DollarTotal ();
	double	dty_total = DutyTotal ();
	double	wgt_total = WeightTotal ();
	double	vol_total = VolumeTotal ();
	
	rv_pr (ML (mlPoMess096),0,0,0);

	if (store2 [DTY].storeInvValue > 0.0 && dty_total <= 0.0)
	{
		print_err (ML (mlPoMess094));
		print_err (ML ("Please ensure at least one product is dutiable"));
	}

	for (line_cnt = 0;line_cnt < maxPoLines; line_cnt++)
	{
		/*-----------------------------------------------
		| Calculate default spreading of invoices	|
		-----------------------------------------------*/
		for (lineNo = FOB;lineNo <= O_4;lineNo++)
		{
			if (store2 [lineNo].storeCostEdit [0] == 'Y')
			{
				value [lineNo] = WeightCalc (lineNo,
							      line_cnt,
							      fob_total,
							      wgt_total,
							      vol_total,
							      dty_total);
			}
			else
			{
				switch (lineNo)
				{
				case	FOB:
					break;

				case	FRT:
				case	INS:
					value [FRT] = STORE.fi_loc;
					value [INS] = 0.00;
					break;

				case	INT:
				case	B_C:
				case	O_1:
				case	O_2:
				case	O_3:
				case	O_4:
					value [INT] = 0.00;
					value [B_C] = 0.00;
					value [O_1] = STORE.other;
					value [O_2] = 0.00;
					value [O_3] = 0.00;
					value [O_4] = 0.00;
					break;

				case	DTY:
					value [DTY] = STORE.duty_val;
					break;

				default:
					break;
				}
			}
		}
		STORE.fi_loc = value [FRT] + value [INS];
		
		cif_cost = STORE.fob_fgn;

		if (posh_rec.ex_rate != 0.00)
			STORE.cif_loc = cif_cost / posh_rec.ex_rate;
		else
			STORE.cif_loc = 0.00;

		STORE.cif_loc = STORE.cif_loc;

		STORE.cif_loc += STORE.fi_loc;
		STORE.cif_loc = STORE.cif_loc;

		STORE.duty_val = value [DTY];

		STORE.lic_val = (double) STORE.lic_rate;
		STORE.lic_val *= STORE.cif_loc;
	
		STORE.lic_val =  STORE.lic_val;

		STORE.other = value [O_1] + value [O_2] + 
			      value [O_3] + value [O_4] +
			      value [INT] + value [B_C];

		/*-------------------------------
		| Calculate Landed Cost NZL	|
		-------------------------------*/
		STORE.land_cst = STORE.cif_loc + 
			     	 STORE.duty_val + 
			     	 STORE.lic_val + 
			     	 STORE.other;
	}
}

/*==============================================================
| Set fields to be edited once line on costing screen changed. |
==============================================================*/
void
SetEditStuff (
 int lineNo)
{
	switch (lineNo)
	{
	case	FOB:
		strcpy (store2 [FOB].storeCostEdit, "Y");
		break;

	case	FRT:
	case	INS:
		strcpy (store2 [FRT].storeCostEdit, "Y");
		strcpy (store2 [INS].storeCostEdit, "Y");
		break;

	case	INT:
	case	B_C:
	case	O_1:
	case	O_2:
	case	O_3:
	case	O_4:
		strcpy (store2 [INT].storeCostEdit, "Y");
		strcpy (store2 [B_C].storeCostEdit, "Y");
		strcpy (store2 [O_1].storeCostEdit, "Y");
		strcpy (store2 [O_2].storeCostEdit, "Y");
		strcpy (store2 [O_3].storeCostEdit, "Y");
		strcpy (store2 [O_4].storeCostEdit, "Y");
		break;
		break;

	case	DTY:
		strcpy (store2 [DTY].storeCostEdit, "Y");
		break;
	}
}

/*=============================
| Calculate default costings. |
=============================*/
void
CalcCosting (
 void)
{
	int	i;
	double	value = 0.00;

	store2 [DTY].storeInvValue = 0.00;
	store2 [FRT].storeInvValue = 0.00;
	store2 [FOB].storeInvValue = 0.00;
	store2 [O_1].storeInvValue = 0.00;

	for (i = 0;i < maxPoLines;i++)
	{
		value	=	out_cost 
					(
						(float) (store [i].duty_val), 
						(float) (store [i].outer)
					);
		value *= store [i].quantity;
		store2 [DTY].storeInvValue += value;

		value	=	out_cost 
					(
						(float) (store [i].fi_loc), 
						(float) (store [i].outer)
					);
		value *= store [i].quantity;
		store2 [FRT].storeInvValue += value;

		value	=	out_cost 
					(
						(float) (store [i].fob_fgn), 
						(float) (store [i].outer)
					);
		value *= store [i].quantity;
		store2 [FOB].storeInvValue += value;

		value	=	out_cost 
					(
						(float) (store [i].other), 
						(float) (store [i].outer)
					);
	 	value = value;
		value *= store [i].quantity;
		store2 [O_1].storeInvValue += value;
	}
}

/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
tab_other (
 int iline)
{
	if (cur_screen == SH_COST)
	{
		print_at (4,1, "%130.130s"," ");
		PrintCostDesc (iline);
	}

	return;
}

/*========================
| Print Costing Details. |
========================*/
void
PrintCostDesc (
 int lineNo)
{
	char	*comm_fob = ML (mlPoMess082),
			*comm_frt = ML (mlPoMess083),
			*comm_oth = ML (mlPoMess084),
			*comm_dty = ML (mlPoMess085);

	switch (lineNo)
	{
	case	FOB:
		rv_pr (comm_fob, (132 - strlen (comm_fob)) / 2,4,1);
		break;

	case	FRT:
	case	INS:
		rv_pr (comm_frt, (132 - strlen (comm_frt)) / 2,4,1);
		break;

	case	INT:
	case	B_C:
	case	O_1:
	case	O_2:
	case	O_3:
	case	O_4:
		rv_pr (comm_oth, (132 - strlen (comm_oth)) / 2,4,1);
		break;

	case	DTY:
		rv_pr (comm_dty, (132 - strlen (comm_dty)) / 2,4,1);
		break;

	default:
		break;
	}
	fflush (stdout);
}

/*=========================================================
| Load category descriptions if defined else use default. |
=========================================================*/
void
LoadCategoryDescription (
 void)
{
	char	*sptr;
	int	i;

	for (i = 0; i < 10; i++)
	{
		switch (i)
		{
		case 0:
			sprintf (catDesc [i], "%-20.20s", invCat [i]);
			break;

		case 1:
			sptr = chk_env ("PO_OS_1");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;

		case 2:
			sptr = chk_env ("PO_OS_2");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 3:
			sptr = chk_env ("PO_OS_3");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 4:
			sptr = chk_env ("PO_OS_4");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 5:
			sprintf (catDesc [i], "%-20.20s", invCat [i]);
			break;
		case 6:
			sptr = chk_env ("PO_OTHER1");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 7:
			sptr = chk_env ("PO_OTHER2");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 8:
			sptr = chk_env ("PO_OTHER3");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;
		case 9:
			sptr = chk_env ("PO_OTHER4");
			if (sptr == (char *)0)
				sprintf (catDesc [i],"%-20.20s",invCat [i]);
			else
				sprintf (catDesc [i],"%-20.20s",sptr);
			break;

		default:
			break;
		}
	}
}

/*===========================================
| Add hash to stored record to update last. |
===========================================*/
void
AddHash (
	long	hhpoHash)
{
	int	i;

	for (i = 0; i < MAX_POS; i++)
	{
		if (hhpo_hash [i] == hhpoHash)
			break;

		if (hhpo_hash [i] == 0L)
		{
			hhpo_hash [i] = hhpoHash;
			break;
		}
	}
}

void
PrintCompany (
 void)
{
	move (0,19);
	line (132);
	print_at (20,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (21,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	print_at (22,0, ML (mlStdMess099), comm_rec.cc_no,comm_rec.cc_name);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		rv_pr (ML (mlPoMess100),55,0,1);
		move (0,1);
		line (132);

		switch (scn)
		{
		case	SH_HEAD:
			box (0,3,132,11);
			move (1,9);
			line (131);
			break;

		case	SH_COST:
			box (0,3,132,1);
			break;

		default:
			break;
		}

		PrintCompany ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
