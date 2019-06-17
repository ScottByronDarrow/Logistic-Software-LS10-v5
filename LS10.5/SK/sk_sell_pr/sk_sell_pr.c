/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_sell_pr.c,v 5.6 2002/12/01 04:48:18 scott Exp $
|  Program Name  : (sk_sell_pr.c)
|  Program Desc  : (Add / Maintain Individual Pricing Records)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius.  | Date Written  : 18/10/93         |
|---------------------------------------------------------------------|
| $Log: sk_sell_pr.c,v $
| Revision 5.6  2002/12/01 04:48:18  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sell_pr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sell_pr/sk_sell_pr.c,v 5.6 2002/12/01 04:48:18 scott Exp $";

#define	MAXLINES	9
#define	TABLINES	9


#define	SCREENWIDTH	132

#include <pslscr.h>
#include <hot_keys.h>
#include <minimenu.h>
#include <twodec.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	UPDATE		0
#define	SEL_IGNORE		1
#define	SEL_DELETE		2

#define	LIVE  		0
#define	FUTURE 		1

#define	KITITEM		 (inmr_rec.inmr_class [0] == 'K')
#define	DESCITEM	 (inmr_rec.inmr_class [0] == 'Z')

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cuqbRecord	cuqb_rec;
struct esmrRecord	esmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inguRecord	ingu_rec;
struct inprRecord	inpr_rec;
struct inprRecord	inpr_old;
struct excfRecord	excf_rec;
struct exclRecord	excl_rec;
struct ingpRecord	ingp_rec;
struct pocrRecord	pocr_rec;
struct exafRecord	exaf_rec;

	/* 
	 * Set up array's required from file structures.
	 */
	double	*inpr_price		=	&inpr_rec.base;
	double	*old_price		=	&inpr_old.base;
	double	*inpr_qty_brk	=	&inpr_rec.qty_brk1;
	double	*old_qty_brk	=	&inpr_old.qty_brk1;
	float	*cuqb_qty_brk	=	&cuqb_rec.qty_brk1;

extern	int	_win_func;

	char	systemDate [11];
	long	lsystemDate;
	
	int		fstTime			= 0,
			fstPrice		= 0,
			priceMaint		= 0,
			envDbPriNum		= 0,
			envSkDbQtyNum	= 0,
			envSkCusPriLvl	= 0,
			futurePrices	= 0,
			selectPriceType	= 0,
			tab_width		= 0,
			new_entry 		= FALSE,
			envDbMcurr		= 0;

	double	lcl_qty [9];

	char	lcl_prce_by [2];
	char	Curr_code [4];

	FILE	*faud;
	FILE	*fout;

	char	*data	= "data";

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD  ", 
	  " Update Pricing Structure File With Changes Made. " }, 
	{ " 2. IGNORE CHANGES ", 
	  " Ignore Changes Just Made To Pricing Structure File." }, 
	{ " 3. DELETE RECORD  ", 
	  " Delete Pricing Structure Record." }, 
	{ ENDMENU }
};


/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	int		price_type;
	char	price_name [20];
	char	ctype_code [4];
	char	ctype_desc [41];
	char	area_code [3];
	char	area_desc [41];
	char	item_no [17];
	char	item_desc [41];
	char	curr_code [4];
	char	curr_desc [41];
	char	br_no [3];
	char	br_name [41];
	char	wh_no [3];
	char	wh_name [41];
	char	prceby_code [2];
	char	prceby_desc [9];
	double	qty_brk [9];
	char	qty_hdr [9][11];
	char	price_desc [16];
	double	base;
	double	price [9];
	long	hhbr_hash;
	long	hhgu_hash;
	int		lpno;
	char	file_code [7];
	char	file_desc [41];
	char	DummyField [2];
} local_rec;

struct
{
	char	_prceby_code [2];
	double	_qty_brk [9];
	char	_qty_hdr [9][11];
	char	_price_desc [16];
	double	_base;
	double	_price [9];
} store [9];

static	struct	var	vars [] =
{
	{1, LIN, "prce_type",	 3, 16, INTTYPE,
		"N", "          ",
		" ", "   ", " Price Type: ", "Enter Price Type.",
		 NE, NO,  JUSTLEFT, "1", "9", (char *) &local_rec.price_type},
	{1, LIN, "prce_name",	 3, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.price_name},
	{1, LIN, "ctype_code",	 3, 16, CHARTYPE,
		"UUU", "          ",
		" ", "   ", " Cust Type : ", "Enter Customer Type. Search Available, Default For All Types.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.ctype_code},
	{1, LIN, "ctype_desc",	 3, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ctype_desc},
	{1, LIN, "area_code",	 4, 16, CHARTYPE,
		"UU", "          ",
		" ", "   ", " Area Code : ", "Enter Customer Area. Search Available, Default For All Types.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.area_code},
	{1, LIN, "area_desc",	 4, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_desc},
	{1, LIN, "item_no",	 5, 16, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Item No   : ", "Enter Item Number. Full Search Available.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "item_desc",	 5, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{1, LIN, "curr_code",	 6, 16, CHARTYPE,
		"UUU", "          ",
		" ", "   ", " Currency  : ", "Enter Currency Code. Full Search Available.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{1, LIN, "curr_desc",	 6, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},
	{1, LIN, "file_code",	 5, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", " File Name : ", "Enter Future Price File Name",
		 NE, NO,  JUSTLEFT, "", "", local_rec.file_code},
	{1, LIN, "file_desc",	 5, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.file_desc},
	{1, LIN, "br_no",	7, 16, CHARTYPE,
		"NN", "          ",
		" ", "  ", " Branch    : ", "Enter Branch Number. Default For All Branches.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_name",	7, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.br_name},
	{1, LIN, "wh_no",	8, 16, CHARTYPE,
		"NN", "          ",
		" ", "  ", " Warehouse : ", "Enter Warehouse Number. Default For All Warehouses",
		NE, NO,  JUSTRIGHT, "", "", local_rec.wh_no},
	{1, LIN, "wh_name",	8, 38, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.wh_name},

	{2, TAB, "prce_desc",	MAXLINES, 1, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "  PRICE TYPE    ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.price_desc},
	{2, TAB, "base",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", " BASE PRICE ", "Base Price of Item.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.base},
	{2, TAB, "price1",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [0], "Price If Order Exceeds Quantity Break 1.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [0]},
	{2, TAB, "price2",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [1], "Price If Order Exceeds Quantity Break 2.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [1]},
	{2, TAB, "price3",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [2], "Price If Order Exceeds Quantity Break 3.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [2]},
	{2, TAB, "price4",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [3], "Price If Order Exceeds Quantity Break 4.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [3]},
	{2, TAB, "price5",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [4], "Price If Order Exceeds Quantity Break 5.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [4]},
	{2, TAB, "price6",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [5], "Price If Order Exceeds Quantity Break 6.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [5]},
	{2, TAB, "price7",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [6], "Price If Order Exceeds Quantity Break 7.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [6]},
	{2, TAB, "price8",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [7], "Price If Order Exceeds Quantity Break 8.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [7]},
	{2, TAB, "price9",	0, 0, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", local_rec.qty_hdr [8], "Price If Order Exceeds Quantity Break 9.",
		 NO, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.price [8]},
	{2, TAB, "DummyField",	0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", local_rec.DummyField},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 * Function Declarations. 
 */
const char	*GetPriceDesc 		(int);
int  	FindInpr 				(int);
int  	GetQtyBreaks 			(int, int);
int  	heading 				(int);
int  	LocalSpecValid 			(int, int);
int  	RecordChanged 			(void);
int  	SetCuqbBreaks 			(char *, int);
int  	spec_valid 				(int);
int  	win_function 			(int, int, int, int);
void 	CloseDB 				(void);
void	DispColHeadings 		(int);
void 	DispPriceDescs 			(void);
void 	DispScn2Title 			(void);
void 	DoUpdate 				(int, int);
void 	DrawBox 				(void);
void 	GetCuqbBreaks 			(int);
void 	LoadInpr 				(void);
void 	OpenAud 				(void);
void 	OpenDB 					(void);
void 	PrintAud 				(void);
void 	SetUpData 				(int);
void 	shutdown_prog 			(void);
void 	SrchCcmr 				(char *);
void 	SrchEsmr 				(char *);
void 	SrchExaf 				(char *);
void 	SrchExcf 				(char *);
void 	SrchExcl 				(char *);
void 	SrchIngu 				(char *);
void 	SrchPocr 				(char *);
void 	tab_other 				(int);
void 	update_menu 			(void);
void 	update 					(void);
void 	ZeroData 				(void);

#ifdef GVISION
void	LoadPriceDescs ();
#endif	/* GVISION */

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char **argv)
{
	int		i;
	char	*sptr;
	char	work [7];

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	if (argc < 3)
	{
		print_at (0,0,mlSkMess303, argv [0]);
		return (EXIT_FAILURE);
	}


	sptr = chk_env ("SK_DBQTYNUM");
	if (sptr == (char *)0)
		envSkDbQtyNum = 0;
	else
	{
		envSkDbQtyNum = atoi (sptr);
		if (envSkDbQtyNum > 9 || envSkDbQtyNum < 0)
			envSkDbQtyNum = 9;
	}

	for (i = envSkDbQtyNum + 1; i <= 9; i++)
	{
		sprintf (work, "price%1d", i);
		FLD (work) = ND;
	}

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envDbPriNum = 5;
	else
	{
		envDbPriNum = atoi (sptr);
		if (envDbPriNum > 9 || envDbPriNum < 1)
			envDbPriNum = 9;
	}

	sptr = chk_env ("SK_CUSPRI_LVL");
	if (sptr == (char *)0)
		envSkCusPriLvl = 0;
	else
		envSkCusPriLvl = atoi (sptr);

	/*
	 * Set tabular screen width and x, y starting co-ordinates.  
	 */

	tab_width = 31 + (envSkDbQtyNum * 11);
	tab_row = 10;
	tab_col = (SCREENWIDTH - tab_width) / 2;

	fstTime = TRUE;

	/*
	 * Set variables based on parameters received.   
	 */

	if (argv [1][0] == 'F')
	{
		FLD ("br_no")      	= NA;
		FLD ("wh_no")      	= NA;
		FLD ("ctype_code") 	= ND;
		FLD ("ctype_desc") 	= ND;
		FLD ("area_code") 	= ND;
		FLD ("area_desc") 	= ND;
		FLD ("curr_code")  	= ND;
		FLD ("curr_desc")  	= ND;
		futurePrices	   	= envDbPriNum;
		envDbPriNum        	= 1;
		priceMaint 		   	= FUTURE;
	}
	else
	{
		FLD ("prce_type")  = ND;
		FLD ("prce_name")  = ND;
		FLD ("file_code")  = ND;
		FLD ("file_desc")  = ND;
		priceMaint         = LIVE;
	}

	local_rec.lpno = atoi (argv [2]);

	/*
	 * Check for multi-currency. 
	 */
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);
	if (!envDbMcurr && priceMaint != FUTURE)
	{
		FLD ("curr_code")		= ND;
		FLD ("curr_desc")		= ND;
		SCN_ROW ("br_no")		= SCN_ROW ("br_no")		- 1;
		SCN_ROW ("br_name")     = SCN_ROW ("br_name")	- 1;
		SCN_ROW ("wh_no")       = SCN_ROW ("wh_no")		- 1;
		SCN_ROW ("wh_name")     = SCN_ROW ("wh_name")	- 1;
	}

	if (envSkCusPriLvl == 0) /* Pricing at company level */
	{
		FLD ("br_no")   = ND;
		FLD ("br_name") = ND;
		FLD ("wh_no")   = ND;
		FLD ("wh_name") = ND;
	}
	else
	if (envSkCusPriLvl == 1) /* Pricing at branch level */
	{
		FLD ("wh_no")   = ND;
		FLD ("wh_name") = ND;
	}

	/*
	 * Get native currency. 
	 */
	sprintf (Curr_code, "%-3.3s", get_env ("CURR_CODE"));


	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	swide ();

	OpenDB ();

	/*
	 * Beginning of input control loop 
	 */
	while (prog_exit == 0) 
	{
		abc_unlock (inpr);
		entry_exit 	= 0;
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		search_ok 	= 1;
		prog_status	= ENTRY;
		in_sub_edit = TRUE;
		_win_func   = FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;


		heading (2);

		/*
		 * Enter screen 2 tabular input.
		 */
		if (new_entry == TRUE)
		{
#ifdef GVISION
			init_ok = FALSE;

			LoadPriceDescs ();

			heading (2);
			scn_display (2);
			entry (2);

			init_ok = TRUE;
#else
			entry (2);
#endif	/* GVISION */

			if (prog_exit || restart) 
				continue;
			heading (2);
		}

		_win_func   = TRUE;
		edit (2);

		if (restart) 
			continue;

		/*
		 * Update selection status.     
		 */
		update ();

	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	if (fstTime == FALSE)
	{
		fprintf (faud, ".EOF\n");
		pclose (faud);
	}
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_id_no");
	open_rec (ingu, ingu_list, INGU_NO_FIELDS, "ingu_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (cuqb, cuqb_list, CUQB_NO_FIELDS, "cuqb_id_sellgrp");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (exaf);
	abc_fclose (excf);
	abc_fclose (excl);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inpr);
	abc_fclose (ingu);
	abc_fclose (pocr);
	abc_fclose (cuqb);

	SearchFindClose ();
	abc_dbclose (data);
}


void
tab_other (
	int	line_no)
{
	int		i;
	char	work [13];

	DispColHeadings (line_no);

	/* Use Window-Activate Key 1 to alter quantity breaks. */
	strcpy (err_str,ML (mlSkMess318)); 

	if (_win_func == TRUE)
		us_pr (err_str, (SCREENWIDTH - strlen (err_str))/2, tab_row - 2, 1);

	if (envSkDbQtyNum > 0)
	{
		if (store [line_no]._prceby_code [0] == 'V')
			sprintf (local_rec.prceby_desc, "%-8.8s", "Value   ");
		else
			sprintf (local_rec.prceby_desc, "%-8.8s", "Quantity");
	}

	local_rec.base = store [line_no]._base;
	for (i = 0; i < 9; i++)
	{
		local_rec.qty_brk [i] = store [line_no]._qty_brk [i];
		local_rec.price [i]   = store [line_no]._price [i];
	}

	if (line_no >= envDbPriNum)
	{
		FLD ("prce_desc") = NA;
		FLD ("base") 	  = NA;
		for (i = 1; i <= envSkDbQtyNum; i++)
		{
			sprintf (work, "price%1d", i);
			FLD (work) = NA;
		}
	}
	else
	{
		strcpy (local_rec.price_desc, store [line_no]._price_desc);
		FLD ("prce_desc") = NA;
		FLD ("base") 	  = NO;
		for (i = 1; i <= envSkDbQtyNum; i++)
		{
			sprintf (work, "price%1d", i);
			FLD (work) = NO;
		}
	}
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Price Type. 
	 */
	if (LCHECK ("prce_type"))
	{
		if (priceMaint == LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.price_type > futurePrices)
		{
			sprintf (err_str,ML (mlSkMess218), futurePrices);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.price_name, GetPriceDesc (local_rec.price_type));

		DSP_FLD ("prce_name");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Type. 
	 */
	if (LCHECK ("ctype_code"))
	{
		if (priceMaint == FUTURE)
			return (EXIT_SUCCESS);

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.ctype_code, "   ");
			sprintf (local_rec.ctype_desc,"%-40.40s",ML ("All Customer Types"));
			DSP_FLD ("ctype_desc");
			FLD ("area_code")	= NA;
			return (EXIT_SUCCESS);
		}
		FLD ("area_code")      	= YES;

		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excl_rec.co_no, comm_rec.co_no);
		sprintf (excl_rec.class_type, "%-3.3s", local_rec.ctype_code);
		cc = find_rec (excl, &excl_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.ctype_desc, "%-40.40s", excl_rec.class_desc);
		DSP_FLD ("ctype_desc");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Area Code.
	 */
	if (LCHECK ("area_code"))
	{
		if (priceMaint == FUTURE)
			return (EXIT_SUCCESS);

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.area_code, "  ");
			sprintf (local_rec.area_desc, "%-40.40s", ML ("All Area Codes."));
			DSP_FLD ("area_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no, comm_rec.co_no);
		sprintf (exaf_rec.area_code, "%-2.2s", local_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.area_desc, "%-40.40s", exaf_rec.area);
		DSP_FLD ("area_code");
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
		clear_mess ();

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

		if (KITITEM)
		{
			print_mess (ML (mlSkMess315));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (DESCITEM)
		{
			print_mess (ML (mlSkMess316));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.item_no,  "%-16.16s",  inmr_rec.item_no);

		SuperSynonymError ();
		
		sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.description);
		local_rec.hhbr_hash = inmr_rec.hhbr_hash;
		DSP_FLD ("item_no");
		DSP_FLD ("item_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Currency. 
	 */
	if (LCHECK ("curr_code"))
	{
		if (priceMaint == FUTURE)
			return (EXIT_SUCCESS);

		if (!envDbMcurr)
		{
			strcpy (local_rec.curr_code, Curr_code);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			strcpy (local_rec.curr_code, Curr_code);

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", local_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.curr_desc, "%-40.40s", pocr_rec.description);
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Branch Number. 
	 */
	if (LCHECK ("br_no"))
	{
		if (envSkCusPriLvl == 0) /* Pricing at company level */
		{
			strcpy (local_rec.wh_no, "  ");
			strcpy (local_rec.br_no, "  ");
			LoadInpr ();
			return (EXIT_SUCCESS);
		}

		if (priceMaint == FUTURE)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.br_name, "%-40.40s", "All Branches");
			DSP_FLD ("br_name");
			if (envSkCusPriLvl == 2)
			{
				FLD ("wh_no") = NA;
				strcpy (local_rec.wh_no, "  ");
				DSP_FLD ("wh_no");
				sprintf (local_rec.wh_name, "%-40.40s", "All Warehouses");
				DSP_FLD ("wh_name");
			}
			LoadInpr ();
			return (EXIT_SUCCESS);
		}
		if (envSkCusPriLvl == 2)
			FLD ("wh_no") = NE;

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%-2.2s", local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			/*Branch %s not on File, local_rec.br_no*/
			errmess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.br_name, esmr_rec.est_name);
		DSP_FLD ("br_name");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Warehouse Number. 
	 */
	if (LCHECK ("wh_no"))
	{
		if (envSkCusPriLvl == 0) /* Pricing at company level */
			return (EXIT_SUCCESS);

		if (envSkCusPriLvl == 1) /* Pricing at branch level */
		{
			strcpy (local_rec.wh_no, "  ");
			LoadInpr ();
			return (EXIT_SUCCESS);
		}

		if (priceMaint == FUTURE)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.wh_no, "  ");
			DSP_FLD ("wh_no");
			sprintf (local_rec.wh_name, "%-40.40s", "All Warehouses");
			DSP_FLD ("wh_name");
			LoadInpr ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.br_no);
		sprintf (ccmr_rec.cc_no, "%-2.2s", local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.wh_name, ccmr_rec.name);
		DSP_FLD ("wh_name");
		LoadInpr ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("file_code"))
	{
		if (priceMaint == LIVE)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngu (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_FAILURE);

		strcpy (ingu_rec.co_no, comm_rec.co_no);
		sprintf (ingu_rec.file_code, "%-6.6s", local_rec.file_code);
		ingu_rec.price_type = local_rec.price_type;
		cc = find_rec (ingu, &ingu_rec, EQUAL, "r");
		if (cc)
			return (EXIT_FAILURE);


		local_rec.hhgu_hash = ingu_rec.hhgu_hash;
		strcpy (local_rec.curr_code, ingu_rec.curr_code);
		strcpy (local_rec.br_no, ingu_rec.br_no);
		strcpy (local_rec.wh_no, ingu_rec.wh_no);

		strcpy (local_rec.file_desc, ingu_rec.file_desc);
		DSP_FLD ("file_desc");

		/*
		 * Invalid if no inprs exist for this file code 
		 */
		 cc = FindInpr (local_rec.price_type);
		 if (cc)
		 {
			print_mess (ML (mlSkMess317));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		 }
		 else
		 {
			int	i;

			lcount [2] = 0;
			new_entry = FALSE;
			{
				scn_set (2);
				strcpy (local_rec.price_desc, local_rec.price_name);
				local_rec.base = inpr_price [0];
				for (i = 0; i < envSkDbQtyNum; i++)
					local_rec.price [i] = inpr_price [i + 1];
				putval (lcount [2]++);
				scn_set (1);
			}
		}


		strcpy (local_rec.br_no, ingu_rec.br_no);
		if (strcmp (local_rec.br_no, "  ") == 0)
			sprintf (local_rec.br_name, "%-40.40s", "All Branches");
		else
		{
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			sprintf (esmr_rec.est_no, "%-2.2s", local_rec.br_no);
			cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
			if (cc)
				sprintf (local_rec.br_name, "%-40.40s", "Unknown Branch");
			else
				strcpy (local_rec.br_name, esmr_rec.est_name);
		}
		DSP_FLD ("br_no");
		DSP_FLD ("br_name");

		strcpy (local_rec.wh_no, ingu_rec.wh_no);
		if (strcmp (local_rec.wh_no, "  ") == 0)
			sprintf (local_rec.wh_name, "%-40.40s", "All Warehouses");
		else
		{
			strcpy (ccmr_rec.co_no, comm_rec.co_no);
			strcpy (ccmr_rec.est_no, local_rec.br_no);
			sprintf (ccmr_rec.cc_no, "%-2.2s", local_rec.wh_no);
			cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
			if (cc)
				sprintf (local_rec.wh_name, "%-40.40s", "Unknown Warehouse");
			else
				strcpy (local_rec.wh_name, ccmr_rec.name);
		}
		DSP_FLD ("wh_no");
		DSP_FLD ("wh_name");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Price Type. 
	 */
	if (LCHECK ("prce_desc"))
	{
		if (store [line_cnt]._qty_brk [0] == 0.00)
			win_function (label ("prce_desc"), line_cnt, 2, ENTRY);
		DispColHeadings (line_cnt);
		strcpy (local_rec.price_desc, store [line_cnt]._price_desc);
		DispPriceDescs ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate quantity break. 
	 */
	if (LCHECK ("base"))
	{
		int	    i;
		char	work [20];

		if (line_cnt < envDbPriNum)
		{
			strcpy (local_rec.price_desc, store [line_cnt]._price_desc);
			DSP_FLD ("prce_desc");
		}

		if (last_char == FN16)
		{
			while (line_cnt < envDbPriNum)
			{
				strcpy (local_rec.price_name, GetPriceDesc (line_cnt + 1));
				store [line_cnt]._base 	= 0.00;
				local_rec.base 			= 0.00;
				for (i = 0; i < 9; i++)
				{
					store [line_cnt]._price [i] 	= 0.00;
					local_rec.price [i] 			= 0.00;
				}
				putval (line_cnt++);
			}
			entry_exit = 1;
			prog_status = EDIT;
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			local_rec.base = store [line_cnt]._base;
		
			if (local_rec.base == 0.00)
			{
				if (line_cnt > 0)
					local_rec.base = store [line_cnt - 1]._base;
			}
			DSP_FLD ("base");
		}

		if (local_rec.base == 0.00)
		{
			for (i = 0; i < envSkDbQtyNum; i++)
			{
				store [line_cnt]._price [i] = 0.00;
				local_rec.price [i]        = 0.00;
				sprintf (work, "price%1d", i + 1);
				DSP_FLD (work);
			}
			if (prog_status == ENTRY)
				skip_entry = goto_field (field, label ("DummyField"));
		}

		store [line_cnt]._base = local_rec.base;
		tab_other (line_cnt);
		if (line_cnt + 1 == envDbPriNum &&
			 (envSkDbQtyNum == 0 || local_rec.base == 0.00) &&
			prog_status == ENTRY)
		{
			putval (line_cnt++);
			entry_exit = 1;
			prog_status = EDIT;
		}
		if (local_rec.qty_brk [0] <= 0)
		{
			skip_entry = goto_field (field, label ("DummyField"));
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Prices. 
	 */
	if (LNCHECK ("price", 5))
	{
		int		i;
		char	work [20];

		strcpy (work, FIELD.label);
		work [6] = '\0';
		if (FLD (work) == ND)
			return (EXIT_SUCCESS);

		i = atoi (FIELD.label + 5);

		if (dflt_used)
		{
			if (prog_status == ENTRY)
			{
				if (local_rec.qty_brk [i - 1] <= 0)
					local_rec.price [i - 1] = 0.00;
				else
					if (i <= 1)
						local_rec.price [i - 1] = store [line_cnt]._base;
					else
						local_rec.price [i - 1] = store [line_cnt]._price [i - 2];
			}
			else
				local_rec.price [i - 1] = store [line_cnt]._price [i - 1];
		}

		if (local_rec.base <= 0 && local_rec.price [i - 1] > 0.00)
		{
			sprintf (err_str, "%s %s",ML (mlSkMess313),ML (mlSkMess314));
			print_mess (err_str);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.qty_brk [i - 1] <= 0 && local_rec.price [i - 1] > 0.00)
		{
			sprintf (err_str, "%s %s",ML (mlSkMess306), ML (mlSkMess314));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		store [line_cnt]._price [i - 1] = local_rec.price [i - 1];
		tab_other (line_cnt);

		if (line_cnt + 1 == envDbPriNum && i == envSkDbQtyNum &&
			prog_status == ENTRY)
		{
			putval (line_cnt++);
			entry_exit = 1;
			prog_status = EDIT;
		}
		if (local_rec.qty_brk [i] <= 0)
			skip_entry = goto_field (field, label ("DummyField"));
			
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Load All pricing structureis For An Item. 
 */
void
LoadInpr (void)
{
	int		i;

	new_entry = TRUE;

	for (i = 1; i <= envDbPriNum; i++)
	{
		cc = FindInpr (i);
		if (cc)
		{
			GetCuqbBreaks (i);
			if (restart)
				return;
		}
	}

	lcount [2] = 0;
	if (new_entry == FALSE)
	{
		scn_set (2);
		for (i = 0; i < envDbPriNum; i++)
		{
			strcpy (local_rec.price_desc, store [i]._price_desc);
			local_rec.base      = store [i]._base;
			local_rec.price [0] = store [i]._price [0];
			local_rec.price [1] = store [i]._price [1];
			local_rec.price [2] = store [i]._price [2];
			local_rec.price [3] = store [i]._price [3];
			local_rec.price [4] = store [i]._price [4];
			local_rec.price [5] = store [i]._price [5];
			local_rec.price [6] = store [i]._price [6];
			local_rec.price [7] = store [i]._price [7];
			local_rec.price [8] = store [i]._price [8];
			putval (lcount [2]++);
		}
		scn_set (1);
	}
}

/*
 * Find pricing structure. 
 */
int
FindInpr (
	int	price_type)
{
	int		i;
	int		store_ix;

	if (priceMaint == FUTURE)
		store_ix = 0;
	else
		store_ix = price_type - 1;

	inpr_rec.hhbr_hash 	= local_rec.hhbr_hash;
	inpr_rec.price_type = price_type;
	inpr_rec.hhgu_hash 	= local_rec.hhgu_hash;
	strcpy (inpr_rec.curr_code, local_rec.curr_code);
	strcpy (inpr_rec.br_no, local_rec.br_no);
	strcpy (inpr_rec.wh_no, local_rec.wh_no);
	strcpy (inpr_rec.area_code, local_rec.area_code);
	strcpy (inpr_rec.cust_type, local_rec.ctype_code);
	cc = find_rec (inpr, &inpr_rec, EQUAL, "u");
	if (cc)
	{
		local_rec.hhgu_hash = 0L;
		strcpy (store [store_ix]._price_desc, GetPriceDesc (price_type));
		strcpy (store [store_ix]._prceby_code, "Q");
		store [store_ix]._base     = 0.00;
		for (i = 0; i < 9; i++)
			store [store_ix]._price [i] = 0.00;
	}
	else
	{
		new_entry = FALSE;
		local_rec.hhgu_hash = inpr_rec.hhgu_hash;
		strcpy (local_rec.prceby_code, inpr_rec.price_by);
		strcpy (store [store_ix]._prceby_code, inpr_rec.price_by);
		if (local_rec.prceby_code [0] == 'Q')
			sprintf (local_rec.prceby_desc, "%-8.8s", "Quantity");
		else
			sprintf (local_rec.prceby_desc, "%-8.8s", "Value   ");

		strcpy (store [store_ix]._price_desc, GetPriceDesc (price_type));
		store [store_ix]._base     = inpr_price [0];

		for (i = 0; i < 9; i++)
		{
			store [store_ix]._qty_brk [i] = inpr_qty_brk [i];
			sprintf (store [store_ix]._qty_hdr [i], "%10.2f", inpr_qty_brk [i]);
			store [store_ix]._price [i] = inpr_price [i + 1];
		}
	}
	return (cc);
}

void
ZeroData (void)
{
	inpr_qty_brk [0]	= 0.00;
	inpr_qty_brk [1]	= 0.00;
	inpr_qty_brk [2]	= 0.00;
	inpr_qty_brk [3]	= 0.00;
	inpr_qty_brk [4]	= 0.00;
	inpr_qty_brk [5]	= 0.00;
	inpr_qty_brk [6]	= 0.00;
	inpr_qty_brk [7]	= 0.00;
	inpr_qty_brk [8]	= 0.00;

	inpr_price [0]	= no_dec (0.00);
	inpr_price [1]	= no_dec (0.00);
	inpr_price [2]	= no_dec (0.00);
	inpr_price [3]	= no_dec (0.00);
	inpr_price [4]	= no_dec (0.00);
	inpr_price [5]	= no_dec (0.00);
	inpr_price [6]	= no_dec (0.00);
	inpr_price [7]	= no_dec (0.00);
	inpr_price [8]	= no_dec (0.00);
	inpr_price [9]	= no_dec (0.00);
}


void
SetUpData (
	int	price)
{
	int		store_ix;

	if (priceMaint == FUTURE)
		store_ix = 0;
	else
		store_ix = price;

	strcpy (inpr_rec.price_by, store [store_ix]._prceby_code);

	inpr_qty_brk [0]	= store [store_ix]._qty_brk [0];
	inpr_qty_brk [1]	= store [store_ix]._qty_brk [1];
	inpr_qty_brk [2]	= store [store_ix]._qty_brk [2];
	inpr_qty_brk [3]	= store [store_ix]._qty_brk [3];
	inpr_qty_brk [4]	= store [store_ix]._qty_brk [4];
	inpr_qty_brk [5]	= store [store_ix]._qty_brk [5];
	inpr_qty_brk [6]	= store [store_ix]._qty_brk [6];
	inpr_qty_brk [7]	= store [store_ix]._qty_brk [7];
	inpr_qty_brk [8]	= store [store_ix]._qty_brk [8];

	inpr_price [0]		= no_dec (store [store_ix]._base);
	inpr_price [1]		= no_dec (store [store_ix]._price [0]);
	inpr_price [2]		= no_dec (store [store_ix]._price [1]);
	inpr_price [3]		= no_dec (store [store_ix]._price [2]);
	inpr_price [4]		= no_dec (store [store_ix]._price [3]);
	inpr_price [5]		= no_dec (store [store_ix]._price [4]);
	inpr_price [6]		= no_dec (store [store_ix]._price [5]);
	inpr_price [7]		= no_dec (store [store_ix]._price [6]);
	inpr_price [8]		= no_dec (store [store_ix]._price [7]);
	inpr_price [9]		= no_dec (store [store_ix]._price [8]);
}

int
RecordChanged (void)
{
	int		i;

	if (inpr_rec.price_by [0] != inpr_old.price_by [0])
		return (EXIT_FAILURE);

	if (inpr_price [0] != old_price [0])
		return (EXIT_FAILURE);

	for (i = 0; i < envSkDbQtyNum; i++)
	{
		if (inpr_qty_brk [i]   != old_qty_brk [i] ||
		    inpr_price [i + 1] != old_price [i + 1])
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
DoUpdate (
	int price, 
	int action)
{
	int		store_ix;

	if (priceMaint == FUTURE)
	{
		store_ix = 0;
		inpr_rec.price_type = local_rec.price_type;
	}
	else
	{
		store_ix = price;
		inpr_rec.price_type = price + 1;
	}

	inpr_rec.hhbr_hash = local_rec.hhbr_hash;
	inpr_rec.hhgu_hash = local_rec.hhgu_hash;
	strcpy (inpr_rec.curr_code, local_rec.curr_code);
	strcpy (inpr_rec.br_no, local_rec.br_no);
	strcpy (inpr_rec.wh_no, local_rec.wh_no);
	strcpy (inpr_rec.cust_type, local_rec.ctype_code);
	strcpy (inpr_rec.area_code, local_rec.area_code);

	cc = find_rec (inpr, &inpr_rec, EQUAL, "u");

	if (!cc) /* Record Exists */
	{
		memcpy ((char *) &inpr_old, (char *) &inpr_rec, sizeof (inpr_rec));
		if (store [store_ix]._base == 0.00 || action == SEL_DELETE)
		{
			ZeroData ();
			abc_unlock (inpr);
			cc = abc_delete (inpr);
			if (cc)
				file_err (cc, inpr, "DBDELETE");
		}
		else
		{
			SetUpData (price);
			cc = abc_update (inpr, &inpr_rec);
			if (cc)
				file_err (cc, inpr, "DBUPDATE");
		}
	}
	else
	{
		ZeroData ();
		memcpy ((char *) &inpr_old, (char *) &inpr_rec, sizeof (inpr_rec));
		if (store [store_ix]._base != 0.00 && action == UPDATE)
		{
			SetUpData (price);
			cc = abc_add (inpr, &inpr_rec);
			if (cc)
				file_err (cc, inpr, "DBADD");
		}
	}

	if (RecordChanged ())
		PrintAud ();
}

/*
 * Update Pricing structure. 
 */
void
update (void)
{
	int		i;

	fstPrice = 1;

	if (new_entry == TRUE)
	{
		for (i = 0; i < envDbPriNum; i++)
			DoUpdate (i, UPDATE);
	}
	else
		update_menu ();
}

/*
 * Update mini menu. 
 */
void
update_menu (
 void)
{
	int		i;

	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ",  upd_menu,  0);
	    switch (mmenu_select (upd_menu))
	    {
		case UPDATE :
		case 99     :
			for (i = 0; i < envDbPriNum; i++)
				DoUpdate (i, UPDATE);
			return;

		case SEL_IGNORE :
		case -1     :
			return;

		case SEL_DELETE :
			for (i = 0; i < envDbPriNum; i++)
				DoUpdate (i, SEL_DELETE);
			return;
	
		default :
			for (i = 0; i < envDbPriNum; i++)
				DoUpdate (i, UPDATE);
			return;
	    }
	}
}

void
GetCuqbBreaks (
	int price_type)
{
	int		i;
	char	work [13];
	int		breaks_found = 0;
	char	selection1 [140];
	char	selection2 [140];
	char	selection3 [140];
	int		store_ix;

	if (priceMaint == FUTURE)
		store_ix = 0;
	else
		store_ix = price_type - 1;


	if (envSkDbQtyNum == 0)
	{
		for (i = 0; i < 9; i++)
		{
			store [store_ix]._qty_brk [i] = 0.00;
			sprintf (store [store_ix]._qty_hdr [i], "%10.2f", 0.00);
		}
		return;
	}

	/*
	 * Breaks by Selling Group. 
	 */
	cuqb_rec.price_type = price_type;
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strcpy (cuqb_rec.sellgrp, inmr_rec.sellgrp);
	cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

	if (!cc)
	{
		sprintf (selection1, " Selling Group : %-11.11s |", cuqb_rec.sellgrp);

		for (i = 0; i < envSkDbQtyNum; i++)
		{
			sprintf (work, " %8.2f |", cuqb_qty_brk [i]);
			strcat (selection1, work);
		}
		breaks_found++;
	}
	else
		strcpy (selection1, "");

	/*
	 * Breaks by Category. 
	 */
	abc_selfield (cuqb, "cuqb_id_cat");
	cuqb_rec.price_type = price_type;
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strcpy (cuqb_rec.category, inmr_rec.category);
	cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

	if (!cc)
	{
		sprintf (selection2, " Category      : %-11.11s |",
			cuqb_rec.category);

		for (i = 0; i < envSkDbQtyNum; i++)
		{
			sprintf (work, " %8.2f |", cuqb_qty_brk [i]);
			strcat (selection2, work);
		}
		breaks_found++;
	}
	else
		strcpy (selection2, "");

	/*
	 * Breaks by Buying Group. 
	 */
	abc_selfield (cuqb, "cuqb_id_buygrp");
	cuqb_rec.price_type = price_type;
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strcpy (cuqb_rec.buygrp, inmr_rec.buygrp);
	cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

	if (!cc)
	{
		sprintf (selection3, " Buying Group  : %-11.11s |", cuqb_rec.buygrp);

		for (i = 0; i < envSkDbQtyNum; i++)
		{
			sprintf (work, " %8.2f |", cuqb_qty_brk [i]);
			strcat (selection3, work);
		}
		breaks_found++;
	}
	else
		strcpy (selection3, "");

	if (breaks_found == 0)
	{
		for (i = 0; i < 9; i++)
			store [store_ix]._qty_brk [i] = 0.00;
		return;
	}
	else
	if (breaks_found > 0)
	{
		if (strcmp (selection1, "") != 0)
		{
			SetCuqbBreaks (selection1, price_type);
			return;
		}
		if (strcmp (selection2, "") != 0)
		{
			SetCuqbBreaks (selection2, price_type);
			return;
		}
		if (strcmp (selection3, "") != 0)
		{
			SetCuqbBreaks (selection3, price_type);
			return;
		}
		print_mess (ML (mlSkMess304));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	/*** Code Retained here for possible future use
	else
	{
		selectPriceType = price_type;
		strcpy (err_str, "#                             ");
		for (i = 1; i <= envSkDbQtyNum; i++)
		{
			sprintf (work, "|  BREAK %1d ", i);
			strcat (err_str, work);
		}
		cuqb_width = strlen (err_str);
		i = (int) ((SCREENWIDTH - (cuqb_width + 2)) / 2);
		tab_open ("qty_list", qty_keys, tab_row - 1, i, 3, FALSE);
		tab_add ("qty_list", err_str);
		sprintf (err_str,"Select Quantity Breaks To Use For The %s Prices", 
							clip (GetPriceDesc (selectPriceType)));
		sprintf (err_str,ML (mlSkMess683), 
							clip (GetPriceDesc (selectPriceType)));
		rv_pr (err_str, (SCREENWIDTH - strlen (err_str))/2, tab_row - 1, 1);
	
		if (strcmp (selection1, "") != 0)
			tab_add ("qty_list", selection1);
		if (strcmp (selection2, "") != 0)
			tab_add ("qty_list", selection2);
		if (strcmp (selection3, "") != 0)
			tab_add ("qty_list", selection3);
	
		tab_scan ("qty_list");

		tab_close ("qty_list", TRUE);
	}
	***/
}

int	
SetCuqbBreaks (
	char *buffer, 
	int price_type)
{
	int		i;
	int		store_ix;

	if (priceMaint == FUTURE)
		store_ix = 0;
	else
		store_ix = price_type - 1;


	switch (buffer [1])
	{
	case  'S' :
		/*
		 * Breaks by Selling Group. 
		 */
		abc_selfield (cuqb, "cuqb_id_sellgrp");
		strcpy (cuqb_rec.co_no, comm_rec.co_no);
		strcpy (cuqb_rec.sellgrp, inmr_rec.sellgrp);
		cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

		if (cc)
			file_err (cc, cuqb, "DBFIND");

		break;

	case  'C' :
		/*
		 * Breaks by Category. 
		 */
		abc_selfield (cuqb, "cuqb_id_cat");
		strcpy (cuqb_rec.co_no, comm_rec.co_no);
		strcpy (cuqb_rec.category, inmr_rec.category);
		cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

		if (cc)
			file_err (cc, cuqb, "DBFIND");

		break;

	case  'B' :
		/*
		 * Breaks by Buying Group. 
		 */
		abc_selfield (cuqb, "cuqb_id_buygrp");
		strcpy (cuqb_rec.co_no, comm_rec.co_no);
		strcpy (cuqb_rec.buygrp, inmr_rec.buygrp);
		cc = find_rec (cuqb, &cuqb_rec, EQUAL, "r");

		if (cc)
			file_err (cc, cuqb, "DBFIND");

		break;

	}
	for (i = 0; i < 9; i++)
	{
		store [store_ix]._qty_brk [i] = cuqb_qty_brk [i];
		sprintf (store [store_ix]._qty_hdr [i], "%10.2f", 
													local_rec.qty_brk [i]);
	}
    return (EXIT_SUCCESS);
}

void
SrchEsmr (
	char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Branch Name  ");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}


void
SrchCcmr (
	char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Warehouse Description ");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		   !strcmp	 (ccmr_rec.est_no, local_rec.br_no) &&
	       !strncmp (ccmr_rec.cc_no, key_val, strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
SrchPocr (
	char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No", "#Description ");

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pocr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (pocr_rec.code, key_val, strlen (key_val)))
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
	sprintf (pocr_rec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	save_rec ("#Category", "#Category Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
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

void
SrchExcl (
 char *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No", "#Customer Type Description");
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", key_val);
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (excl_rec.class_type, key_val, strlen (key_val)) && 
		   !strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

void
SrchExaf (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Area Description");
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", key_val);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (exaf_rec.area_code, key_val, strlen (key_val)) && 
		   !strcmp (exaf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}
void
SrchIngu (
 char *key_val)
{
	_work_open (6,0,40);
	save_rec ("#File", "#Description ");

	strcpy (ingu_rec.co_no, comm_rec.co_no);
	sprintf (ingu_rec.file_code, "%-6.6s", key_val);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingu_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingu_rec.file_code, key_val, strlen (key_val)))
	{
		if (ingu_rec.price_type != local_rec.price_type)
		{
			cc = find_rec (ingu, &ingu_rec, NEXT, "r");
			continue;
		}
		cc = save_rec (ingu_rec.file_code, ingu_rec.file_desc);
		if (cc)
			break;

		cc = find_rec (ingu, &ingu_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingu_rec.co_no, comm_rec.co_no);
	sprintf (ingu_rec.file_code, "%-6.6s", temp_str);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingu, "DBFIND");
}

void
DispScn2Title (void)
{
	int		i;
	int		pgchar_pos [10];
	char	work [12];

	strcpy (err_str, "                            ");
	pgchar_pos [0] = tab_col + strlen (err_str) + 2;
	for (i = 1; i <= envSkDbQtyNum; i++)
	{
		sprintf (work, "     QTY %1d ", i);
		strcat (err_str,work);
		pgchar_pos [i] = tab_col + strlen (err_str) + 2;
	}
	print_at (tab_row -1, tab_col + 1, err_str);
	for (i = 0; i < envSkDbQtyNum; i++)
	{
		move (pgchar_pos [i], tab_row - 1);
		PGCHAR (5);
	}
}

/*
 * Screen Heading. 
 */
int
heading (
 int scn)
{
	int		i;
	int		box_lines;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		fflush (stdout);
		if (priceMaint == FUTURE)
			strcpy (err_str,ML (mlSkMess310));
		else
			strcpy (err_str,ML (mlSkMess311));
		i = strlen (err_str);

		rv_pr (err_str, (SCREENWIDTH - i)/2, 0, 1);

		fflush (stdout);
		line_at (1,0, SCREENWIDTH - 1);

		box_lines = 6;
		if (!envDbMcurr && priceMaint != FUTURE)
			box_lines--;

		if (envSkCusPriLvl == 0)
			box_lines = box_lines - 2;

		if (envSkCusPriLvl == 1)
			box_lines = box_lines - 1;

		switch (scn)
		{
		case  1 :
			box (0, 2, SCREENWIDTH - 1, box_lines);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			if (prog_status != ENTRY)
			{
				scn_set (2);
#ifndef GVISION
				box (tab_col, tab_row - 2, tab_width, TABLINES + 3);
#endif	/* GVISION */

				DispScn2Title ();
				scn_write (2);
				DispColHeadings (line_cnt);
				scn_display (2);
				DispPriceDescs ();
				scn_set (1);
			}
			break;
		
		case  2 :
			box (0, 2, SCREENWIDTH - 1, box_lines);
			scn_set (1);
			scn_write (1);
			scn_display (1);
			scn_set (2);
#ifndef GVISION
			box (tab_col, tab_row - 2, tab_width, TABLINES + 3);
#endif	/* GVISION */
			DispScn2Title ();
			scn_write (2);
			DispColHeadings (line_cnt);
			scn_display (2);
			DispPriceDescs ();
			break;

		}
		line_at (21,1, SCREENWIDTH - 1);

		print_at (22,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_short);
		print_at (22,45,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_short);

		/*  reset this variable for _new screen NOT page	*/
		line_cnt = 0;
		fflush (stdout);
	}
    return (EXIT_SUCCESS);
}

/*
 * Display Price Description fields.                            
 * This is neccessary because the price descriptions are not truely part of 
 * the tabular screen. We need to be able to display these fields during 
 * screen entry hence the following procedure.
 */
void
DispPriceDescs (void)
{
#ifndef GVISION
	int		i;

	for (i = 0; i < envDbPriNum; i++)
		print_at (tab_row + i + 2, tab_col + 2,store [i]._price_desc);
#endif	/* GVISION */
}

void
DrawBox (void)
{
	int		i;

	cl_box (tab_col + 1, tab_row - 1, 35, envSkDbQtyNum + 2);
	rv_pr (ML (mlSkMess676), tab_col + 3, tab_row, 1);
	print_at (tab_row + 1, tab_col + 3, ML (mlSkMess677));
	for (i = 0; i < envSkDbQtyNum; i++)
		print_at (tab_row + i + 2, tab_col + 3,ML (mlSkMess678), i + 1);

	/*
	 * display previous qtys if you change this then change spec_valid
	 */
	print_at (tab_row + 1, tab_col + 19, "%8.8s", local_rec.prceby_desc);
	for (i = 0; i < envSkDbQtyNum; i++)
		print_at (tab_row + i + 2, tab_col + 17, "%10.2f", local_rec.qty_brk [i]);
}

int
GetQtyBreaks (
 int _new, 
 int line_no)
{
	int		count;
#ifdef GVISION
	scn_hide (2);
#endif	/* GVISION */

	strcpy (lcl_prce_by, store [line_no]._prceby_code);
	for (count = 0; count < envSkDbQtyNum; count++)
	{
		lcl_qty [count]           = store [line_no]._qty_brk [count];
		local_rec.qty_brk [count] = store [line_no]._qty_brk [count];
	}

	crsr_on ();
	if (!_new)
		print_mess (ML (mlSkMess305));
	else
		print_mess (ML (mlSkMess306));

	DrawBox ();
	for (count = 1; count <= envSkDbQtyNum; count++)
	{
		while (TRUE)
		{
			if (count == 0)
				getalpha (tab_col + 19, tab_row + count + 1, "U", local_rec.prceby_code);
			else
				local_rec.qty_brk [count - 1] = getdouble (tab_col + 17, tab_row + count + 1, "NNNNNNN.NN");
			if (!LocalSpecValid (count, _new) &&
			   (last_char == ENDINPUT || 
				 last_char == RESTART || 
				 last_char == DOWN_KEY))
				break;

			if (last_char == UP_KEY)
			{
				if (count)
					count--;
				continue;
			}
		}

		if (count && local_rec.qty_brk [count - 1] == 0.00)
			break;

		if (last_char == RESTART)
			break;
	}
	crsr_off ();

	if (!_new)
		clear_mess ();

#ifdef GVISION
	erase_box (tab_col + 1, tab_row - 1, 35, envSkDbQtyNum + 2);

	for (int i = 10; i < 21; i++)
	{
		move (0, i);
		cl_line ();
	}
	scn_display (2);
#endif	/* GVISION */

	if (last_char == RESTART)
	{
		strcpy (store [line_no]._prceby_code, lcl_prce_by);
		for (count = 0; count < envSkDbQtyNum; count++)
			local_rec.qty_brk [count] = lcl_qty [count];
		return (TRUE);
	}
	return (FALSE);
}

int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	int		i;
	int		ws_new;

	if (envSkDbQtyNum == 0)
	{
		if (prog_status != ENTRY)
		{
			print_mess (ML (mlSkMess307));
			sleep (sleepTime);
			clear_mess ();
		}
		return (EXIT_FAILURE);
	}

	if (scn != 2)
		return (EXIT_FAILURE);

	if (mode == ENTRY && fld != label ("prce_desc"))
		return (EXIT_FAILURE);

	if (mode == ENTRY)
		ws_new = TRUE;
	else
		ws_new = FALSE;

	cc = GetQtyBreaks (ws_new, lin);
	if (!cc)
	{
		strcpy (store [lin]._prceby_code, local_rec.prceby_code);

		for (i = 0; i < 9; i++)
		{
			store [lin]._qty_brk [i] = local_rec.qty_brk [i];
			store [lin]._price [i] = local_rec.price [i];
			sprintf (store [lin]._qty_hdr [i],"%10.2f",local_rec.qty_brk [i]);
		}
	}
	strcpy (local_rec.price_desc, store [lin]._price_desc);
	putval (lin);
	if (mode == ENTRY)
	{
			ws_new = lcount [2];
			lcount [2] = lin;
			scn_write (2);
			scn_display (2);
			lcount [2] = ws_new;
	}
	DispScn2Title ();
	return (EXIT_SUCCESS);
}


int 
LocalSpecValid (
 int lcl_cnt, 
 int _new)
{
	if (last_char == UP_KEY)
		return (EXIT_SUCCESS);

	if (last_char == RESTART || last_char == FN16)
		return (EXIT_SUCCESS);

	if (lcl_cnt == 0)
	{
		if (local_rec.prceby_code [0] != 'Q' && local_rec.prceby_code [0] != 'V')
		{
			print_mess (ML (mlSkMess308));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			if (local_rec.prceby_code [0] == 'V')
				strcpy (local_rec.prceby_desc,ML (mlSkMess679));
			else
				strcpy (local_rec.prceby_desc,ML (mlSkMess680));
			print_at (tab_row + 1, tab_col + 19, "%8.8s", local_rec.prceby_desc);
			return (EXIT_SUCCESS);
		}
	}

	/*
	 * validate that qty break is more than previous entered 
	 */
	if (dflt_used || last_char == DOWN_KEY)
	{
		/*
		 * if last field was set to 0.00 then if dflt used set to 0.00
		 */
		if (lcl_cnt > 1 && local_rec.qty_brk [lcl_cnt - 2] == 0.00)
			local_rec.qty_brk [lcl_cnt - 1] = 0.00;
		else
		{
			if (_new)
				local_rec.qty_brk [lcl_cnt - 1] = 0.00;
			else
				local_rec.qty_brk [lcl_cnt - 1] = lcl_qty [lcl_cnt - 1];
		}
	}

	/*
	 * if zero is entered then this means no more qty breaks will be entered
	 */
	if (local_rec.qty_brk [lcl_cnt - 1] == 0.00)
	{
		int	count;
		for (count = lcl_cnt; count <= envSkDbQtyNum; count++)
			local_rec.price [count - 1] = local_rec.qty_brk [count - 1] = 0.00;

		for (count = 0; count < envSkDbQtyNum; count++)
			print_at (tab_row + count + 2, tab_col + 17, "%10.2f", local_rec.qty_brk [count]);
	}

	if (lcl_cnt > 1 &&
		local_rec.qty_brk [lcl_cnt - 2] == 0.00 && 
		local_rec.qty_brk [lcl_cnt - 1] != 0.00)
	{
		print_mess (ML (mlSkMess122));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcl_cnt > 1 &&
		local_rec.qty_brk [lcl_cnt - 1] != 0.00 && 
		local_rec.qty_brk [lcl_cnt - 1] <= local_rec.qty_brk [lcl_cnt - 2])
	{
		sprintf (err_str, ML (mlSkMess123),
				local_rec.qty_brk [lcl_cnt - 1],
				local_rec.qty_brk [lcl_cnt - 2]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * this is from getdouble so DSP_FLD will not work
	 * if you change this then change DrawBox
	 */
	switch (lcl_cnt) {
	case	1	:
		print_at (tab_row + 2, tab_col + 17, "%10.2f", local_rec.qty_brk [0]);
		break;
	case	2	:
		print_at (tab_row + 3, tab_col + 17, "%10.2f", local_rec.qty_brk [1]);
		break;
	case	3	:
		print_at (tab_row + 4, tab_col + 17, "%10.2f", local_rec.qty_brk [2]);
		break;
	case	4	:
		print_at (tab_row + 5, tab_col + 17, "%10.2f", local_rec.qty_brk [3]);
		break;
	case	5	:
		print_at (tab_row + 6, tab_col + 17, "%10.2f", local_rec.qty_brk [4]);
		break;
	case	6	:
		print_at (tab_row + 7, tab_col + 17, "%10.2f", local_rec.qty_brk [5]);
		break;
	case	7	:
		print_at (tab_row + 8, tab_col + 17, "%10.2f", local_rec.qty_brk [6]);
		break;
	case	8	:
		print_at (tab_row + 9, tab_col + 17, "%10.2f", local_rec.qty_brk [7]);
		break;
	case	9	:
		print_at (tab_row + 10, tab_col + 17, "%10.2f", local_rec.qty_brk [8]);
		break;
	case	10	:
		print_at (tab_row + 11, tab_col + 17, "%10.2f", local_rec.qty_brk [9]);
		break;
	}
	return (EXIT_SUCCESS);
}

void
DispColHeadings (
 int line_no)
{
	int		ws_col, i;

	if (envSkDbQtyNum > 0)
	{
		if (store [line_no]._prceby_code [0] == 'V')
			/*PRICE BY VALUE   */
			print_at (tab_row - 1, tab_col + 3,ML (mlSkMess681));
		else
			/*PRICE BY QUANTITY */
			print_at (tab_row - 1, tab_col + 3, ML (mlSkMess682));
	}

	strcpy (err_str, "                            ");
	ws_col = tab_col + strlen (err_str) + 3;
	for (i = 0; i < envSkDbQtyNum; i++)
	{
#ifdef GVISION
		char	headStr [64];
		sprintf (headStr, "%10.2f", store [line_no]._qty_brk [i]);
		ChangeTabHeading (2, label ("price1") + i, headStr);
#else
		print_at (tab_row, ws_col, "%10.2f", store [line_no]._qty_brk [i]);
#endif	/* GVISION */

		ws_col = ws_col + 11;
		strcpy (local_rec.qty_hdr [i], store [line_no]._qty_hdr [i]);
	}
}

void
PrintAud (void)
{
	int		count;
	char	ws_work [20];
	char	formatWork [81];

	if (fstTime)
		OpenAud ();

	if (!fstTime)
	{
		if (fstPrice == 1)
		{
			fprintf (faud, "|-------------------------------------------");
			fprintf (faud, "--------------------------------------------");
			fprintf (faud, "--------------------------------------------");
			fprintf (faud, "-------------------------|\n");
		}
		else
			fprintf (faud, "|%156.156s|\n", " ");
	}

	fstTime = FALSE;

	if (fstPrice == 1)
	{
		fprintf (faud, "| Item : %16.16s", inmr_rec.item_no);	
		fprintf (faud, "- %40.40s", inmr_rec.description);
		fprintf (faud, "Br :  %2.2s ", inpr_rec.br_no);
		fprintf (faud, "Wh : %2.2s ", inpr_rec.wh_no);
		fprintf (faud, "Customer Type : %3.3s ", inpr_rec.cust_type);
		fprintf (faud, "Area Code: %2.2s", inpr_rec.area_code);
		fprintf (faud, "%40.40s|\n", " ");
		if (priceMaint == FUTURE)
		{
			fprintf (faud, "| Future Prices Effective");	
			fprintf (faud, " From : %10.10s  ", DateToString (ingu_rec.eff_date));
			fprintf (faud, "                    ");
			fprintf (faud, "         ");
			fprintf (faud, "        ");
			fprintf (faud, "              ");
			fprintf (faud, "%61.61s|\n", " ");
		}
		fprintf (faud, "|%156.156s|\n", " ");
		fstPrice = 0;
	}
	
	/*
	 * all this stuff should be centered so cat stuff to a string a centre it
	 */
	strcpy (err_str, "");
	if (envSkDbQtyNum > 0)
	{
	 	if (inpr_rec.price_by [0] == 'V') 
			sprintf (err_str, "|%s", ML (mlSkMess681));
		else
			sprintf (err_str, "|%s", ML (mlSkMess682));
	}
	
	else
	{
		sprintf (formatWork, ML (mlSkMess684), GetPriceDesc (inpr_rec.price_type));
		sprintf (err_str,"|%s",formatWork);
	}
	count = 157 - (strlen (err_str));
	fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

	strcpy (err_str, "");
	strcat (err_str,ML (mlSkMess685));
	strncat (err_str, "           ", 11);
	for (count = 0; count < envSkDbQtyNum; count++)	/*  Qty Brk info	*/
	{
		sprintf (ws_work, " %10.2f", old_qty_brk [count]);
		strncat (err_str, ws_work, 11);
	}
	
	count = 157 - (strlen (err_str));

	fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");
	strcpy (err_str, "");
	/*strcat (err_str, "|  New Quantity Breaks -   ");*/
	strcat (err_str,ML (mlSkMess686));
	strncat (err_str, "           ", 11);
	for (count = 0; count < envSkDbQtyNum; count++)	/*  Qty Brk info	*/
	{
		sprintf (ws_work, " %10.2f", inpr_qty_brk [count]);
		strncat (err_str, ws_work, 11);
	}
	
	count = 157 - (strlen (err_str));
	fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

	strcpy (err_str, "");
	/*strcat (err_str, "|  Old Prices          -   ");*/
	strcat (err_str,ML (mlSkMess687));
	for (count = 0; count <= envSkDbQtyNum; count++)	/*  Old Prices  	*/
	{
		sprintf (ws_work, " %10.2f", DOLLARS (old_price [count]));
		strncat (err_str, ws_work, 11);
	}

	count = 157 - (strlen (err_str));
	fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

	strcpy (err_str, "");
	/*strcat (err_str, "|  New Prices          -   ");*/
	strcat (err_str,ML (mlSkMess688));
	for (count = 0; count <= envSkDbQtyNum; count++)	/*  New Prices   */
	{
		sprintf (ws_work, " %10.2f", DOLLARS (inpr_price [count]));
		strncat (err_str, ws_work, 11);
	}

	count = 157 - (strlen (err_str));
	fprintf (faud, "%s%*.*s|\n", err_str, count, count, " ");

}

void
OpenAud (void)
{
	if ((faud = popen ("pformat", "w")) == (FILE *) NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (faud, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (faud, ".LP%d\n", local_rec.lpno);

	fprintf (faud, ".7\n");
	fprintf (faud, ".PI12\n");
	fprintf (faud, ".L158\n");

	fprintf (faud,".ECompany %s - %s\n",comm_rec.co_no,clip (comm_rec.co_name));
	if (priceMaint == FUTURE)
		fprintf (faud, ".EFUTURE SELLING PRICE ADJUSTMENT Audit\n");
	else
		fprintf (faud, ".ESELLING PRICE MAINTENANCE Audit\n");
	fprintf (faud, ".E AS AT %s\n", SystemTime ());

	fprintf (faud, ".B1\n");

	fprintf (faud, "=========================================================");
	fprintf (faud, "=========================================================");
	fprintf (faud, "============================================\n");

	fprintf (faud, ".R=======================================================");
	fprintf (faud, "=========================================================");
	fprintf (faud, "==============================================\n");
}

#ifdef GVISION
void
LoadPriceDescs ()
{
	int     saveCurScreen;
	saveCurScreen = cur_screen;
	init_ok = TRUE;
	scn_set (2);
	init_ok = FALSE;
	lcount [2] = MAXLINES;
	for (line_cnt = 0; line_cnt < envDbPriNum; line_cnt++)
	{
		strcpy (local_rec.price_desc, store [line_cnt]._price_desc);
		local_rec.base = 0.00;
		local_rec.price [0] = 0.00;
		local_rec.price [1] = 0.00;
		local_rec.price [2] = 0.00;
		local_rec.price [3] = 0.00;
		local_rec.price [4] = 0.00;
		local_rec.price [5] = 0.00;
		local_rec.price [6] = 0.00;
		local_rec.price [7] = 0.00;
		local_rec.price [8] = 0.00;
		putval (line_cnt);
	}

	scn_set (saveCurScreen);
	line_cnt = 0;
}
#endif	/* GVISION */


/*
 * Routine to get price desctiptions from comm record.
 */
const char	*
GetPriceDesc (
	int		priceNo)
{
	static	char	priceDesc [16];

	strcpy (priceDesc, " ");

	switch (priceNo)
	{
		case	1:	
			strcpy (priceDesc,	comm_rec.price1_desc);
			break;
		case	2:	
			strcpy (priceDesc, comm_rec.price2_desc);
			break;
		case	3:	
			strcpy (priceDesc, comm_rec.price3_desc);
			break;
		case	4:	
			strcpy (priceDesc, comm_rec.price4_desc);
			break;
		case	5:	
			strcpy (priceDesc, comm_rec.price5_desc);
			break;
		case	6:	
			strcpy (priceDesc, comm_rec.price6_desc);
			break;
		case	7:	
			strcpy (priceDesc, comm_rec.price7_desc);
			break;
		case	8:	
			strcpy (priceDesc, comm_rec.price8_desc);
			break;
		case	9:	
			strcpy (priceDesc, comm_rec.price9_desc);
			break;
	}
	return (priceDesc);
}
