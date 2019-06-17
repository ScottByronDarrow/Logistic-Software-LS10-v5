/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _inws_inp.c,v 5.6 2002/07/24 08:38:54 scott Exp $
|  Program Name  : ( sk_inws_inp.c  )                                 |
|  Program Desc  : ( Add / Update branch / warehouse relationships  ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: _inws_inp.c,v $
| Revision 5.6  2002/07/24 08:38:54  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/18 06:43:37  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/18 03:20:25  scott
| Updated as input of category should have skipped item
|
| Revision 5.3  2002/06/26 05:18:50  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:29:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:34  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/24 08:37:20  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _inws_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_inws_inp/_inws_inp.c,v 5.6 2002/07/24 08:38:54 scott Exp $";

#define	TABLINES	7
#define MAXWIDTH	135
#include <pslscr.h>
#include <minimenu.h>
#include <number.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define 	SEL_UPDATE		0
#define 	SEL_IGNORE		1
#define 	SEL_DELETE		2
#define 	SEL_DEFAULT		99

#define		SR			store [line_cnt]

	int		newRecord	=	FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct esmrRecord	esmr_rec;
struct inwsRecord	inws_rec;
struct inwsRecord	inws2_rec;
struct inwdRecord	inwd_rec;
struct inwdRecord	inwd2_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;

	struct	storeRec {
		long	hhccHash;
	} store [MAXLINES];

	char	*ccmr2	= "ccmr2",
			*inws2	= "inws2",
			*inwd2	= "inwd2",
			*data	= "data";

long		currentHhccHash	=	0L;

char	ShipDefault [2];

int		EDIT_ONLY = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	update [1];
	char	prev_item [17];
	char	inv_date [11];
	char	item_no [17];
	char	inmr_desc [41];
	double	sup_cost;
	double	nor_cost;
	char	std_uom [5];
	char	std_uom_desc [31];
	char	sup_uom [5];
	char	sup_uom_desc [31];
	float	old_lead_time;
	char	old_priority [3];
	char	br_no [3],
			wh_no [3];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "cat_no",	3, 20, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category         ", "Use Search key for valid categories.",
		NO, NO,  JUSTLEFT, "", "", excf_rec.cat_no},
	{1, LIN, "cat_desc",	3, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{1, LIN, "item_no",	 4, 20,   CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Item Number.      ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",		 4, 65,   CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.inmr_desc},
	{1, LIN, "priority",	 5, 20,   CHARTYPE,
		"N", "          ",
		" ", "1", "Supplier Priority", "Enter Priority of 1-9",
		YES, NO,  JUSTLEFT, "123456789", "", inws_rec.sup_priority},
	{1, LIN, "min_order",	7, 20,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Min Order qty.    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &inws_rec.min_order},
	{1, LIN, "nor_order",	7, 56,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Normal Order qty. ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &inws_rec.norm_order},
	{1, LIN, "ord_mult",	7, 95,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", "Order Lot size    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &inws_rec.ord_multiple},
	{1, LIN, "weight",	8, 20,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Weight.           ", "Weight in Kg per Unit.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inws_rec.weight},
	{1, LIN, "pallet_size",	8, 56,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Pallet Size       ", "Pallet Size in cubic metres.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inws_rec.pallet_size},
	{1, LIN, "volume",	8, 95,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Volume.           ", "Volume in cubic metres per Unit.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inws_rec.volume},
	{1, LIN, "uplift_pc",	10, 20,  FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Uplift Percent    ", "Input default uplift percent.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inws_rec.upft_pc},
	{1, LIN, "uplift_amt",	10, 95,  MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Uplift Amount     ", "Input default uplift Amount.",
		YES, NO, JUSTRIGHT, "0", "999999.99", (char *) &inws_rec.upft_amt},
	{2, TAB, "br_no",	MAXLINES, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, "BR", "Enter Branch number.",
		 NE, NO, JUSTRIGHT, "", "", esmr_rec.est_no},
	{2, TAB, "br_name",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Br. Short Name ", "",
		 NA, NO, JUSTRIGHT, "", "", esmr_rec.short_name},
	{2, TAB, "wh_no",	 0, 0, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.cc_no, "WH", "Enter Warehouse number.",
		 NE, NO, JUSTRIGHT, "", "", ccmr_rec.cc_no},
	{2, TAB, "wh_name",	0, 1, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", " ", "Wh. Acronym", "",
		 NA, NO, JUSTRIGHT, "", "", ccmr_rec.acronym},
	{2, TAB, "distance",		0, 1,  FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Distance Km.", "Enter Distance to Warehouse in Km.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inwd_rec.km},
	{2, TAB, "dflt_lead",		0, 1,  CHARTYPE,
		"U", "          ",
		" ", ShipDefault, "D/L", "Enter default shipment method. L(and) S(ea) or A(ir)",
		YES, NO, JUSTRIGHT, "LSA", "", inwd_rec.dflt_lead},
	{2, TAB, "leadsea",		0, 3,  FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Lead Time(Sea.)", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inwd_rec.sea_time},
	{2, TAB, "leadair",		0, 3,  FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Lead Time(Air.)", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inwd_rec.air_time},
	{2, TAB, "leadland",		0, 3,  FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Lead Time(Land)", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inwd_rec.lnd_time},
	{2, TAB, "D_uplift_pc",		0, 1,  FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Uplift %", "Enter uplift percent charged.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inwd_rec.upft_pc},
	{2, TAB, "D_uplift_amt",		0, 1,  MONEYTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Uplift Amount", "Enter uplift Amount charged.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inwd_rec.upft_amt},
	{2, TAB, "demand",		0, 3,  CHARTYPE,
		"U", "          ",
		" ", "N", "Demand", "Enter Y(es) if demand is updated when stock transfered to this warehouse. <default = No>",
		YES, NO, JUSTRIGHT, "YN", "", inwd_rec.demand},
	{2, TAB, "hhccHash",		0, 0,  LONGTYPE,
		"NNNNNNNNN", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&inwd_rec.hhcc_hash},
	{2, TAB, "pri",		0, 3,  CHARTYPE,
		"U", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", inwd_rec.sup_priority},
	{0, LIN, "",		 0,  0,    INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 					(void);
void 	OpenDB 							(void);
void 	CloseDB 						(void);
int 	spec_valid 						(int);
int 	CheckDuplicateEntry 			(long, int);
void 	update_menu 					(void);
void 	SrchEsmr 						(char *);
void 	SrchExcf 						(char *);
void 	SrchCcmr 						(char *);
int 	heading 						(int);
void 	PrintCompany 					(void);
static 	int CheckPriority 				(void);
static 	void LoadWarehouseDetails 		(int);
static 	void AddUpdateWarehouseRecords 	(void);
static 	void DrawScreen 				(int);

/*=====================================================================
| Main Processing Routine.
=====================================================================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	tab_row	=	12;
	tab_col	=	0;
	SETUP_SCR (vars);


	/*------------------------------------------
	| Shipment Default. A(ir) / L(and) / S(ea) |
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

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (ccmr_rec.co_no, 	comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	currentHhccHash	=	ccmr_rec.hhcc_hash;
	
	strcpy (local_rec.inv_date, DateToString (comm_rec.inv_date));

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		init_ok		= TRUE;
		search_ok 	= TRUE;

		/*-------------------------------
		| Enter screen 1 linear input
		-------------------------------*/
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;
		heading (1);
		entry (1);

		if (!prog_exit && !restart)
		{
			/*------------------------
			| Screen 2 tabular input
			------------------------*/
			LoadWarehouseDetails (2);

			if (newRecord)
				entry (2);
			else
				edit (2);

			if (!prog_exit && !restart)
			{
				edit_all ();

				if (!prog_exit && !restart)
					AddUpdateWarehouseRecords ();
			}
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (ccmr2, ccmr);
	abc_alias (inws2, inws);
	abc_alias (inwd2, inwd);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inws,  inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd,  inwd_list, INWD_NO_FIELDS, "inwd_id_no");
	open_rec (inws2, inws_list, INWS_NO_FIELDS, "inws_id_no2");
	open_rec (inwd2, inwd_list, INWD_NO_FIELDS, "inwd_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (ccmr2);
	abc_fclose (inwd);
	abc_fclose (inws);
	abc_fclose (inwd2);
	abc_fclose (inws2);
	abc_fclose (inmr);
	abc_fclose (excf);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
	/*-------------------------------
	| Validate Item Number  input . |
	-------------------------------*/
	if (LCHECK ("item_no"))
	{
		if (F_NOKEY (label ("item_no")) || dflt_used)
		{
			inmr_rec.hhbr_hash	=	0L;
			inmr_rec.weight		=	0.00;
			strcpy (local_rec.item_no,  ML("GLOBAL SUPPLY"));
			strcpy (local_rec.inmr_desc,ML("GLOBAL SUPPLY FOR ALL PRODUCTS"));
			return (EXIT_SUCCESS);
		}
			
		/*-------------------------
		| Search for part number. |
		-------------------------*/
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML(mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("item_no");

		strcpy (local_rec.inmr_desc, inmr_rec.description);
		DSP_FLD ("desc");

		scn_write (1);
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Branch Number Input. |
	-------------------------------*/
	if (LCHECK ("br_no"))
	{
		/*---------------------
		| Search for Branches |
		---------------------*/
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------------
		| Lookup Branches master file. |
		------------------------------*/
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (esmr , &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML(mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("br_name");

		if (!strcmp (esmr_rec.est_no, comm_rec.est_no))
		{
			errmess (ML(mlSkMess459));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Warehouse Number Input. |
	----------------------------------*/
	if (LCHECK ("wh_no"))
	{
		/*-----------------------
		| Search for Suppliers. |
		-----------------------*/
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*-------------------------------
		| Lookup Warehouse master file. |
		-------------------------------*/
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, esmr_rec.est_no);
		cc = find_rec (ccmr , &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML(mlStdMess100));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SR.hhccHash		=	ccmr_rec.hhcc_hash;
		inwd_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		strcpy (inwd_rec.sup_priority, inws_rec.sup_priority);

		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			errmess (ML("LRP not available for warehouse"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
			
		if (CheckDuplicateEntry (SR.hhccHash, line_cnt))
		{
			errmess (ML(mlStdMess240));
			sleep (sleepTime);
			skip_entry = goto_field (field, label ("br_no"));
			return (EXIT_SUCCESS);
		}

		DSP_FLD ("wh_name");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Supplier Priority Input. |
	-----------------------------------*/
	if (LCHECK ("priority"))
	{
		inws_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inws_rec.hhcf_hash	=	excf_rec.hhcf_hash;
		inws_rec.hhcc_hash	=	currentHhccHash;
		cc = find_rec ("inws", &inws_rec, COMPARISON, "r");
		if (cc)
		{
			inws_rec.inws_hash	=	0L;
			newRecord	=	TRUE;
			if (CheckPriority ())
				return (EXIT_FAILURE);
		}
		else
		{
			newRecord	=	FALSE;
			entry_exit	=	TRUE;
		}
		
		/*--------------------------------------------------
		| Item has no Supplier Record so it is a new item. |
		--------------------------------------------------*/
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Minimum Order	|
	---------------------------*/
	if (LCHECK ("min_order"))
	{
		if (inws_rec.min_order < 0.00)
		{
			errmess (ML(mlSkMess322));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Normal Order		|
	---------------------------*/
	if (LCHECK ("nor_order"))
	{
		if (inws_rec.norm_order < 0.00)
		{
			errmess (ML(mlSkMess322));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Order Multiple	|
	---------------------------*/
	if (LCHECK ("ord_mult"))
	{
		if (inws_rec.ord_multiple < 0.00)
		{
			errmess (ML(mlSkMess322));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Detail uplift. |
	-------------------------*/
	if (LCHECK ("D_uplift_pc"))
	{
		if (dflt_used)
			inwd_rec.upft_pc = inws_rec.upft_pc;

		DSP_FLD ("D_uplift_pc");
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Detail uplift. |
	-------------------------*/
	if (LCHECK ("D_uplift_amt"))
	{
		if (dflt_used)
			inwd_rec.upft_amt = inws_rec.upft_amt;

		DSP_FLD ("D_uplift_amt");
		return (EXIT_SUCCESS);
	}
	/*------------------
	| Validate Weight. |
	------------------*/
	if (LCHECK ("weight"))
	{
		if (dflt_used)
			inws_rec.weight = inmr_rec.weight;

		DSP_FLD ("weight");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Category. |
	--------------------*/
	if (LCHECK ("cat_no"))
	{
		if (dflt_used)
		{
			excf_rec.hhcf_hash	=	0L;
			strcpy (excf_rec.cat_no,  ML("GLOBAL"));
			strcpy (excf_rec.cat_desc,ML("GLOBAL FOR ALL CATEGORIES.    "));
			FLD ("item_no")	=	YES;
			return (EXIT_SUCCESS);
		}
		
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML("Category is not on file."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("cat_desc");

		FLD ("item_no")	=	NA;
		inmr_rec.hhbr_hash	=	0L;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================================
| Check for prioriry already being used. |
========================================*/
static	int
CheckPriority (void)
{
	char	dispString [100];

	inws2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inws2_rec.hhcf_hash	= excf_rec.hhcf_hash;
	inws2_rec.hhcc_hash	=	0L;
	strcpy (inws2_rec.sup_priority, " ");
	cc = find_rec (inws2, &inws2_rec, GTEQ, "r");
	while (!cc)
	{
		if (inmr_rec.hhbr_hash > 0L && 
			inws2_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			cc = find_rec (inws2, &inws2_rec, NEXT, "r");
			continue;
		}
		if (excf_rec.hhcf_hash > 0L && 
			inws2_rec.hhcf_hash != excf_rec.hhcf_hash)
		{
			cc = find_rec (inws2, &inws2_rec, NEXT, "r");
			continue;
		}
		if (inws2_rec.sup_priority [0] == inws_rec.sup_priority [0])
		{
			
			ccmr2_rec.hhcc_hash	=	inws2_rec.hhcc_hash;
			cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy  (dispString, ML("Prioriry %s is already on file for Branch %s / Warehouse %s"));
				sprintf (err_str, dispString, 
									inws2_rec.sup_priority,
									ccmr2_rec.est_no,
									ccmr2_rec.cc_no);
				errmess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		cc = find_rec (inws2, &inws2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
CheckDuplicateEntry (
 long   hhccHash,
 int    line_no)
{
	int		i;
	int		no_lines = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_lines;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*---------------------------------
		---------------------------------*/
		if (store [i].hhccHash == hhccHash)
				return(1);
	}
	return(0);
}

/*==============================================
| Read detail lines from note-pad detail file. |
==============================================*/
static void
LoadWarehouseDetails (
 int    scn)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (scn);
	init_vars (scn);
	lcount [scn] = 0;

	inwd_rec.inws_hash	=	inws_rec.inws_hash;
	inwd_rec.hhcc_hash	=	0L;
	inwd_rec.hhbr_hash	=	0L;
	inwd_rec.hhcf_hash	=	0L;
	strcpy (inwd_rec.sup_priority, " ");

	cc = find_rec("inwd", &inwd_rec, GTEQ, "r");
	while (!cc && inwd_rec.inws_hash == inws_rec.inws_hash)
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec("inwd", &inwd_rec, NEXT, "r");
			continue;
		}
		ccmr_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
		cc = find_rec ("ccmr2", &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec("inwd", &inwd_rec, NEXT, "r");
			continue;
		}
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, ccmr_rec.est_no);
		cc = find_rec (esmr , &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec("inwd", &inwd_rec, NEXT, "r");
			continue;
		}
		store [lcount [scn]].hhccHash	=	ccmr_rec.hhcc_hash;
		putval (lcount [scn]++);

		cc = find_rec("inwd", &inwd_rec, NEXT, "r");
	}
	line_cnt	=	0;
	DrawScreen (scn);
	clear_mess();
	return;
}

/*==================================================
| Update or add a record to inventory branch file. |
==================================================*/
static void
AddUpdateWarehouseRecords (void)
{
	if (newRecord)
	{
		inws_rec.inws_hash	=	0L;
		inws_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inws_rec.hhcf_hash	=	excf_rec.hhcf_hash;
		inws_rec.hhcc_hash	=	currentHhccHash;

		cc = abc_add (inws, &inws_rec);
		if (cc)
			file_err (cc, inws, "DBADD");

		cc = find_rec (inws, &inws_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inws, "DBFIND");

		scn_set (2);

		for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
		{
			getval (line_cnt);

			inwd2_rec.inws_hash		=	inws_rec.inws_hash;
			inwd2_rec.hhcc_hash		=	store [line_cnt].hhccHash;
			inwd2_rec.hhbr_hash		=	inws_rec.hhbr_hash;
			inwd2_rec.hhcf_hash		=	inws_rec.hhcf_hash;
			inwd2_rec.km			=	inwd_rec.km;
			strcpy (inwd2_rec.sup_priority, inws_rec.sup_priority);
			strcpy (inwd2_rec.dflt_lead,   inwd_rec.dflt_lead);
			inwd2_rec.sea_time	=	 inwd_rec.sea_time;
			inwd2_rec.air_time	=	 inwd_rec.air_time;
			inwd2_rec.lnd_time	=	 inwd_rec.lnd_time;
			inwd2_rec.upft_pc	=	 inwd_rec.upft_pc;
			inwd2_rec.upft_amt	=	 inwd_rec.upft_amt;
			strcpy (inwd2_rec.demand, inwd_rec.demand);

			cc = abc_add (inwd2, &inwd2_rec);
			if (cc)
				file_err (cc, inwd2, "DBADD");
		}
	}
	else
		update_menu ();

	abc_unlock (inws);

	strcpy (local_rec.prev_item, inmr_rec.item_no);

	return;
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE INVENTORY WAREHOUSE SYPPLY RECORD WITH CHANGES MADE.   ", 
		  "" }, 
		{ " 2. IGNORE CHANGES JUST MADE TO INVENTORY WAREHOUSE SUPPLY RECORD.", 
		  "" }, 
		{ " 3. DELETE INVENTORY WAREHOUSE SUPPLY RECORD.                     ", 
		  "" }, 
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
update_menu (void)
{
	for (;;)
	{
	    mmenu_print ("            U P D A T E    S E L E C T I O N .            ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case SEL_DEFAULT :
		case SEL_UPDATE :

			scn_set (2);

			for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
			{
				getval (line_cnt);

				inwd2_rec.inws_hash	=	inws_rec.inws_hash;
				inwd2_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
				inwd2_rec.hhbr_hash	=	inws_rec.hhbr_hash;
				inwd2_rec.hhcf_hash	=	inws_rec.hhcf_hash;
				strcpy( inwd2_rec.sup_priority,  inwd_rec.sup_priority);
				cc = find_rec (inwd2, &inwd2_rec, COMPARISON, "r");
				if (cc)
				{
					inwd2_rec.inws_hash	=	inws_rec.inws_hash;
					inwd2_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
					inwd2_rec.hhbr_hash	=	inws_rec.hhbr_hash;
					inwd2_rec.hhcf_hash	=	inws_rec.hhcf_hash;
					inwd2_rec.km		=	 inwd_rec.km;
					strcpy (inwd2_rec.dflt_lead, inwd_rec.dflt_lead);
					strcpy (inwd2_rec.sup_priority, inwd_rec.sup_priority);
					inwd2_rec.sea_time	=	 inwd_rec.sea_time;
					inwd2_rec.air_time	=	 inwd_rec.air_time;
					inwd2_rec.lnd_time	=	 inwd_rec.lnd_time;
					inwd2_rec.upft_pc	=	 inwd_rec.upft_pc;
					inwd2_rec.upft_amt	=	 inwd_rec.upft_amt;
					strcpy (inwd2_rec.demand, inwd_rec.demand);

					cc = abc_add (inwd2, &inwd2_rec);
					if (cc)
						file_err (cc, inwd2, "DBADD");
				}
				else
				{
					inwd2_rec.hhcc_hash	=	inwd_rec.hhcc_hash;
					inwd2_rec.hhbr_hash	=	inws_rec.hhbr_hash;
					inwd2_rec.hhcf_hash	=	inws_rec.hhcf_hash;
					inwd2_rec.km		=	 inwd_rec.km;
					strcpy (inwd2_rec.dflt_lead, inwd_rec.dflt_lead);
					strcpy (inwd2_rec.sup_priority, inwd_rec.sup_priority);
					inwd2_rec.sea_time	=	 inwd_rec.sea_time;
					inwd2_rec.air_time	=	 inwd_rec.air_time;
					inwd2_rec.lnd_time	=	 inwd_rec.lnd_time;
					inwd2_rec.upft_pc	=	 inwd_rec.upft_pc;
					inwd2_rec.upft_amt	=	 inwd_rec.upft_amt;
					strcpy (inwd2_rec.demand, inwd_rec.demand);

					cc = abc_update (inwd2, &inwd2_rec);
					if (cc)
						file_err (cc, inwd2, "DBUPDATE");
				}
			}
			strcpy (inws_rec.stat_flag, "0");

			scn_set (1);
			cc = abc_update (inws, &inws_rec);
			if (cc)
				file_err (cc, inws, "DBUPDATE");
			
			return;

		case SEL_IGNORE :
			abc_unlock (inws);
			return;

		case SEL_DELETE :
			inwd2_rec.inws_hash	=	inws_rec.inws_hash;
			inwd2_rec.hhcc_hash	=	0L;
			cc = find_rec (inwd2, &inwd2_rec, GTEQ, "r");
			while (!cc && inwd2_rec.inws_hash == inws_rec.inws_hash)
			{
				cc = abc_delete (inwd2);
				if (cc)
					file_err (cc, inwd2, "DBDELETE");

				inwd2_rec.inws_hash	=	inws_rec.inws_hash;
				inwd2_rec.hhcc_hash	=	0L;
				cc = find_rec (inwd2, &inwd2_rec, GTEQ, "r");
			}
			abc_unlock (inws);
			cc = abc_delete (inws);
			if (cc)
				file_err (cc, inws, "DBDELETE");
			return;
			break;

		default :
			break;
	    }
	}
}

/*================================
| Search for Branch master file. |
================================*/
void
SrchEsmr (
 char*  key_val)
{
	int		break_out;

	work_open ();

	save_rec ("#Number", "# Branch Name.");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");

	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		if (check_search (esmr_rec.est_no, key_val, &break_out))
		{
			cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();

	work_close ();
	if (cc)
	{
		sprintf (esmr_rec.est_no, "%-2.2s" , " ");
		sprintf (esmr_rec.est_name, "%-40.40s", " ");
		return;
	}

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	save_rec ("#Cat No", "#Category Description");
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
		      !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, excf, "DBFIND");
}

/*=====================================================================
| Search for Warehouse master file.                                   |
=====================================================================*/
void
SrchCcmr (
 char*  key_val)
{
	int		break_out;

	work_open ();

	save_rec ("#Number", "# Warehouse Name.");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	strcpy (ccmr_rec.cc_no, "  ");

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, esmr_rec.est_no))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
			continue;
		}
		if (check_search (ccmr_rec.cc_no, key_val, &break_out))
		{
			cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();

	work_close ();
	if (cc)
	{
		sprintf (ccmr_rec.cc_no, "%-2.2s" , " ");
		sprintf (ccmr_rec.name, "%-40.40s", " ");
		return;
	}

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	swide ();
	clear ();

	rv_pr (ML("Inventory Warehouse Supply Maintenance"), 34, 0, 1);
	
	print_at (0,90, ML(mlSkMess096), local_rec.prev_item);
	move (0, 1);
	line (131);

		
	move(1,input_row);
	switch(scn)
	{
	case	1:
		box (0, 2, 131, 8);
		move (1, 6);  line (130);
		move (1, 9);  line (130);

		break;

	case	2:
		DrawScreen (1);
		break;
	}
	PrintCompany ();

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

void
PrintCompany (void)
{
	move (0, 21);
	line (131);
	sprintf (err_str, ML(mlStdMess276),
							comm_rec.co_no, comm_rec.co_short,
							comm_rec.est_no,comm_rec.est_short,
							comm_rec.cc_no, comm_rec.cc_name);
	rv_pr (err_str, 1, 22, 1);
}

static void
DrawScreen (
 int    scn)
{
	heading (scn);
	scn_display (scn);
}
