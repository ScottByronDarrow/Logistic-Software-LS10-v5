/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: FindInmr.c,v 5.4 2002/01/09 00:44:39 scott Exp $
|---------------------------------------------------------------------|
| $Log: FindInmr.c,v $
| Revision 5.4  2002/01/09 00:44:39  scott
| Updated to document better.
|
|
=====================================================================*/
#include 	<ring_menu.h>
#include	<std_decs.h>

/*
 * Define local structures with names that will not conflict.
 */
static	const	char			
	*inmr	=	"_inmr_findinmr", 	/* Inventory Master file. 		*/
	*inbm	=	"_inbm_findinmr", 	/* Inventory Barcode Master.	*/
	*cuit	=	"_cuit_findinmr", 	/* Customer Item file.			*/
	*srsk	=	"_srsk_findinmr";	/* Stock Search file.			*/
							
	struct	inmrRecord	inmrRec;
	struct	inbmRecord	inbmRec;
	struct	cuitRecord	cuitRec;
	struct	srskRecord	srskRec;

									
	extern	char	err_str [];		/* External from pslscr.h		*/
	extern	int		ringClearLine;	/* External from ring_menu.h	*/
	extern	int		_wide;			/* Wide from tcap.h				*/
	extern	int		search_key;		/* External from pslscr.h		*/
	extern	int		last_char;		/* External from pslscr.h		*/

	/*
	 * Environment variables used:
	 *		SK_LOOKUP		-	Defines the search order for items.
	 *						-	I = Item number
	 *						-	B = Item Barcode
	 *						-	C = Customer item
	 *						-	Q = Item Quick Code.
	 *
	 *		SK_SER			-	Assigns search keys to search buttons.
	 *						-	I = Item
	 *						-	B = Item Barcode
	 *						-	A = Item Alpha Code
	 *						-	D = Item Description
	 *						-	M = Item Maker number (now used as brand)
	 *						-	L = Item aLternate number 
	 *						-	C = Customer item 
	 *
	 *		SK_SER_CLASS_EX	-	Defines the item classes that are excluded from
	 *						-	searches.
	 *
	 *		SK_SEARCH_POPUP	-	Defines if popup search window is allowed.
	 *
	 *		SK_SER_ACTST_EX	-	Defines active status codes to be excluded from
	 *						-	searches.
	 */

	/*
	 * Inventory Master File Base Record.
	 */
#define	INMR_NO_FIELDS	12

	static	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"}, 
		{"inmr_item_no"}, 
		{"inmr_hhbr_hash"}, 
		{"inmr_hhsi_hash"}, 
		{"inmr_alpha_code"}, 
		{"inmr_supercession"}, 
		{"inmr_maker_no"}, 
		{"inmr_alternate"}, 
		{"inmr_barcode"}, 
		{"inmr_description"}, 
		{"inmr_category"}, 
		{"inmr_quick_code"}, 
	};

	struct inmrRecord
	{
		char	co_no 			[3];
		char	item_no 		[17];
		long	hhbr_hash;
		long	hhsi_hash;
		char	alpha_code 		[17];
		char	supercession 	[17];
		char	maker_no 		[17];
		char	alternate 		[17];
		char	barcode 		[17];
		char	description 	[41];
		char	category 		[12];
		char	quick_code 		[9];
	};

	/*
	 * Inventory item barcode file.
	 */
#define	INBM_NO_FIELDS	5

	static	struct dbview	inbm_list [INBM_NO_FIELDS] =
	{
		{"inbm_co_no"}, 
		{"inbm_barcode"}, 
		{"inbm_item_no"}, 
		{"inbm_uom"}, 
		{"inbm_last_mod"}
	};

	struct inbmRecord
	{
		char	co_no 		[3];
		char	barcode 	[17];
		char	item_no 	[17];
		char	uom 		[5];
		Date	last_mod;
	};

	/*
	 * Customer Specific Inventory Item File. 
	 */
#define	CUIT_NO_FIELDS	4

	static struct dbview	cuit_list [CUIT_NO_FIELDS] =
	{
		{"cuit_hhcu_hash"}, 
		{"cuit_hhbr_hash"}, 
		{"cuit_item_no"}, 
		{"cuit_item_desc"}
	};

	struct cuitRecord
	{
		long	hhcu_hash;
		long	hhbr_hash;
		char	item_no [17];
		char	item_desc [41];
	};

	/*
	 * Search file lookup table for inventory.
	 */
#define	SRSK_NO_FIELDS	16

	static struct dbview	srsk_list [SRSK_NO_FIELDS] =
	{
		{"srsk_co_no"}, 
		{"srsk_hhbr_hash"}, 
		{"srsk_item_no"}, 
		{"srsk_class"}, 
		{"srsk_category"}, 
		{"srsk_active_status"}, 
		{"srsk_alpha_code"}, 
		{"srsk_alternate"}, 
		{"srsk_barcode"}, 
		{"srsk_maker_no"}, 
		{"srsk_description"}, 
		{"srsk_source"}, 
		{"srsk_sellgrp"}, 
		{"srsk_buygrp"}, 
		{"srsk_spare"}, 
		{"srsk_qc_reqd"}
	};

	struct srskRecord
	{
		char	co_no 			[3];
		long	hhbr_hash;
		char	item_no 		[17];
		char	srsk_class 		[2];
		char	category 		[12];
		char	active_status 	[2];
		char	alpha_code 		[17];
		char	alternate 		[17];
		char	barcode 		[17];
		char	maker_no 		[17];
		char	description 	[41];
		char	source 			[3];
		char	sellgrp 		[7];
		char	buygrp 			[7];
		char	spare 			[16];
		char	qc_reqd 		[2];
	};

#define	MAX_SUPER	500


char	*envLookupPointer;				/* Users in search lookup.			*/
char	environmentLookup		[5];	/* Search lookup order.				*/
char	supercessionPart		[17];	/* Holds supercession part.			*/
char	alternatePart 			[17];	/* Holds Synonym alternate part.	*/
char	srchMessage 			[6][61];/* Holds ML converted errors mess.	*/
char	validTypeFlags			[8];	/* Valid search values.				*/
char	excludeClassCodes 		[31];	/* Exclusion classes.				*/
char	excludeActiveStatCodes 	[31];	/* Exclusion active status codes.	*/
char	itemNo 					[17], 	/* Used for input values from ring	*/
		itemAlt 				[17], 	/*                                 	*/
		itemMaker 				[17], 	/*                                 	*/
		itemDesc 				[41], 	/*                                 	*/
		itemAlpha 				[17], 	/*                                 	*/
		itemCategory 			[12], 	/*                                 	*/
		itemSellGrp 			[7], 	/*                                 	*/
		itemBuyGrp 				[7];	/*                                 	*/
										
int		supercessionFlag	=	0;		/* Supercession flag.				*/
int		alternateFlag		=	0;		/* Synonym alternate flag.			*/
int		supercessionCounter = 	0;		/* Supercession loop counter error  */
int		alternateOK 		= 	TRUE;	/* Alternate processing flag.		*/
int		searchFilesOpen		=	FALSE;	/* Set when search fils opened.		*/
int		__searchEnvOpen		=	0;		/* Search open flag					*/
int		__searcherror		=	0;		/* Search Error flag				*/
int		andFlag				=	TRUE;	/* AND OR flag for search.			*/
int		manufacturingSrch	=	FALSE;	/* Exclude none Manufacture items	*/
int		kitPhantomSrch		=	FALSE;	/* Exclude none Kit and Phantoms.   */
int		qualityControlSrch	=	FALSE;	/* Set if only QC items required.   */
int		envSkSearchPopup	=	0;		/* Define search popup.				*/
							

/*
 * Search Function prototypes.
 */
static	int		ReadCustItems		(char *, long, char *);
static	int		ReadItemNumber 		(char *, char *);
static	int		ReadQuickCode 		(char *, char *);
static	int		ReadBarCode 		(char *, char *);
int				CheckSearch			(char *, char *, int *);
int				InmrFindError 		(char *);
void 			SearchInput 		(void);
void			SearchFindOpen 		(void);

/*
 ***************************************************************************
 * 	Function	:	FindInmr ()
 ***************************************************************************
 * 	Description	:	Main function to lookup item number keyed from screen.
 *
 *	Notes		:	Lookup performed based on environment SK_LOOKUP
 *
 *	Parameters	:	coNo		-	Company number.
 *				:	srchItemNo	-	Search item number as input from screen
 *				:	hhcuHash	-	Customer hash (cumr_hhcu_hash)
 *				:				-	Would be zero if no customer selected or
 *				:				-	not required for this input.
 *				: 	custCodesOk	-	Yes if customer specific codes flag is
 *				:				-	set on customer master file.
 */
int	
FindInmr (			
	char	*coNo, 	
	char	*srchItemNo, 
	long	hhcuHash, 	
	char	*custCodesOK)
{						
	int		ptr = 0;
	int		recordFound = 0;
	
	/*
	 * Open search files.
	 */
	SearchFindOpen	();

	/*
	 * Initilise flags.
	 */
	supercessionCounter = 0;
	supercessionFlag 	= FALSE;
	alternateFlag 		= FALSE;
	envLookupPointer 	= &environmentLookup [0];

	while (!recordFound && * (envLookupPointer + ptr))
	{
		switch (* (envLookupPointer + ptr))
		{
			/* 
			 * Search by item number.
			 */
			case	'I':	
			case	'i':
				recordFound = ReadItemNumber (coNo, srchItemNo);
			break;

			/*
			 * Search for customer specific item codes.
			 */
			case	'C':	
			case	'c':
				recordFound = ReadCustItems (srchItemNo, hhcuHash, custCodesOK);
			break;

			/*
			 * Search for item quick codes.
			 */
			case	'Q':	
			case	'q':
				recordFound = ReadQuickCode (coNo, srchItemNo);
			break;

			/*
			 * Search for item bar code.
			 */
			case	'B':	
			case	'b':
				recordFound = ReadBarCode (coNo, srchItemNo);
			break;
		}
		ptr++;
	}
	/*
	 * No record found so return an error (item not found)
	 */
	if (!recordFound)
		return (EXIT_FAILURE);

	/*
	 * Routines will handle Synonym items and alternate Synonyms 	
	 * If Synonym does not have an inmr_alternate set then it can 
	 * be considered a synonym and can be sold. 				
	 * If Synonym does have a inmr_alternate them it can be considered 
	 * as an alternate with alternate being sold not Synonym		
	 */
	if (inmrRec.hhsi_hash != 0L && alternateOK)
	{
		sprintf (alternatePart , "%-16.16s", inmrRec.item_no);
		if (FindSupercession (coNo, inmrRec.alternate, FALSE))
		{
			alternateFlag = FALSE;
			return (EXIT_FAILURE);
		}
	}
	sprintf (supercessionPart, "%-16.16s", inmrRec.item_no);
	if (FindSupercession (coNo, inmrRec.supercession, TRUE))
	{
		supercessionFlag = FALSE;
		return (EXIT_FAILURE);
	}
	strcpy (srchItemNo, strdup (inmrRec.item_no));
	return (EXIT_SUCCESS);
}


/*
 ***************************************************************************
 * 	Function	:	ReadCustItems ()
 ***************************************************************************
 * 	Description	:	Finds customer specific item codes.
 *
 *	Notes		:	Requires customer item code flag to be set      
 *
 *	Parameters	:	srchItemNo	-	Search item number as input from screen
 *				:	hhcuHash	-	Customer hash (cumr_hhcu_hash)
 *				:				-	Would be zero if no customer selected or
 *				:				-	not required for this input.
 *				: 	custCodesOk	-	Yes if customer specific codes flag is
 *				:				-	set on customer master file.
 */
static int
ReadCustItems (
	char	*srchItemNo, 
	long   hhcuHash, 
	char	*custCodesOK)
{
	int		cc	=	0;

	/*
	 * If no customer hash passed or customer does not allow specific codes.
	 */
	if (hhcuHash == 0L || custCodesOK[0] != 'Y')
		return (EXIT_SUCCESS);

	cuitRec.hhcu_hash = hhcuHash;
	strcpy (cuitRec.item_no, srchItemNo);
	cc = find_rec (cuit, &cuitRec, EQUAL, "r");
	if (!cc)
	{
		/*
		 * Select index inmr_hhbr_hash (inmr_hhbr_hash)
		 */
		abc_selfield (inmr, "inmr_hhbr_hash");
		inmrRec.hhbr_hash = cuitRec.hhbr_hash;
		if (!find_rec (inmr, &inmrRec, EQUAL, "r"))
		{
			/*
			 * Select index inmr_id_no (inmr_co_no, inmr_item_no)
			 */
			abc_selfield (inmr, "inmr_id_no");
			return (EXIT_FAILURE);
		}
	}
	/*
	 * Select index inmr_id_no (inmr_co_no, inmr_item_no)
	 */
	abc_selfield (inmr, "inmr_id_no");
	return (EXIT_SUCCESS);
}

/*
 ***************************************************************************
 * 	Function	:	ReadItemNumber ()
 ***************************************************************************
 * 	Description	:	Finds item master file.
 *
 *	Notes		:	None
 *
 *	Parameters	:	coNo		-	Company number.
 *				:	itemNo		-	Item number.
 */
static int
ReadItemNumber (
	char	*coNo, 
 	char	*itemNo)
{
	/*
	 * Select index inmr_id_no (inmr_co_no, inmr_item_no)
	 */
	abc_selfield (inmr, "inmr_id_no");

	strcpy (inmrRec.co_no, coNo);
	sprintf (inmrRec.item_no, "%-16.16s", itemNo);

	if (find_rec (inmr, &inmrRec, EQUAL, "r"))
		return (EXIT_SUCCESS);
	return (EXIT_FAILURE);
}

/*
 ***************************************************************************
 * 	Function	:	ReadQuickCode ()
 ***************************************************************************
 * 	Description	:	Finds item quick code.
 *
 *	Notes		:	None
 *
 *	Parameters	:	coNo		-	Company number.
 *				:	quickItem	-	Short item number keyed.
 */
static	int	
ReadQuickCode (
	char	*coNo, 
	char	*quickItem)
{
	char	workString	[17];
	int		workLength;

	/*
	 * Check if input length is not greater than quick code length.
	 */
	workLength	=	strlen (rtrim (quickItem, workString));
	if (workLength > 8 || workLength == 0)
		return (EXIT_SUCCESS);

	/*
	 * Select index inmr_quick_id (inmr_co_no, inmr_quick_code)
	 */
	abc_selfield (inmr, "inmr_quick_id");

	strcpy (inmrRec.co_no, coNo);
	sprintf (inmrRec.quick_code, "%-8.8s", quickItem);
	if (find_rec (inmr, &inmrRec, EQUAL, "r"))
	{
		/*
		 * Reselect index inmr_id_no (inmr_co_no, inmr_item_no)
		 */
		abc_selfield (inmr, "inmr_id_no");
		return (EXIT_SUCCESS);
	}
	/*
	 * Reselect index inmr_id_no (inmr_co_no, inmr_item_no)
	 */
	abc_selfield (inmr, "inmr_id_no");
	return (EXIT_FAILURE);
}

/*
 ***************************************************************************
 * 	Function	:	ReadBarCode ()
 ***************************************************************************
 * 	Description	:	Finds an item bar code.
 *
 *	Notes		:	UOM from inbm should be retained as default.
 *
 *	Parameters	:	coNo		-	Company number.
 *				:	barCodeItem	-	Bar code item keyed.     
 */
static	int	
ReadBarCode (
	char	*coNo, 
 	char 	*barCodeItem)
{
	char	workString	[17];

	if (strlen (rtrim (barCodeItem, workString)) == 0)
		return (EXIT_SUCCESS);

	/*
	 * Lookup barcode.
	 */
	sprintf (inbmRec.co_no,   "%-2.2s",   coNo);
	sprintf (inbmRec.barcode, "%-16.16s", barCodeItem);
	if (find_rec (inbm, &inbmRec, COMPARISON, "r"))
		return (EXIT_SUCCESS);

	/*
	 * Reselect index inmr_id_no (inmr_co_no, inmr_item_no)
	 * Find item from item held on barcode file.
	 */
	abc_selfield (inmr, "inmr_id_no");
	strcpy (inmrRec.co_no, coNo);
	sprintf (inmrRec.item_no, "%-16.16s", inbmRec.item_no);
	if (find_rec (inmr, &inmrRec, EQUAL, "r"))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

/*
 ***************************************************************************
 * 	Function	:	FindSupercession ()
 ***************************************************************************
 * 	Description	:	Check to see if item is a supercession or alternate.
 *
 *	Notes		:	UOM from inbm should be retained as default.
 *
 *	Parameters	:	coNo		-	Company number.
 *				:	sitem_no	-	Item number 
 *				:	super		-	True if supercession else alternate
 */
int
FindSupercession (
	char	*coNo, 
 	char	*sitem_no, 
 	int		super)
{
	/*
	 * If supercession item not blank and super flag is false and no alternate
	 */
	if (!strcmp (sitem_no, "                ") ||
		(!super && inmrRec.hhsi_hash == 0L))
		return (EXIT_SUCCESS);

	supercessionFlag 	= (super) ? TRUE : FALSE;
	alternateFlag 		= (super) ? FALSE: TRUE;

	/*
	 * Would indicate that we are in a never ending loop.
	 */
	if (supercessionCounter++ > MAX_SUPER)
	{
		sprintf 
		(
			err_str, 
			((super) ? srchMessage [2] : srchMessage [3]), inmrRec.item_no
		);
		return (InmrFindError (err_str));
	}
	/*
	 * Find the superceeding item
	 */
	strcpy (inmrRec.co_no, coNo);
	strcpy (inmrRec.item_no, sitem_no);
	if (!find_rec (inmr, &inmrRec, EQUAL, "r"))
	{
		return 
		(
			FindSupercession 
			(
				coNo, 
				(super) ? inmrRec.supercession : inmrRec.alternate, 
				super
			)
		);
	}
	/*
	 * Couldn't find the superceeding item	
	 */
	if (super)
		sprintf (err_str, srchMessage [4], supercessionPart);
	else
		sprintf (err_str, srchMessage [5], alternatePart);

	return (InmrFindError (err_str));
}

/*
 ***************************************************************************
 * 	Function	:	SuperSynonymError ()
 ***************************************************************************
 * 	Description	:	Error message for supercession and synonym alternates.
 *
 *	Notes		:	None
 *
 *	Parameters	:	None
 */
void
SuperSynonymError (void)
{
	if (!alternateFlag && !supercessionFlag)
		return;

	clear_mess ();
	if (alternateFlag)
	{
		sprintf 
		(
			err_str, 
			srchMessage [0], clip (alternatePart), inmrRec.item_no, BELL
		);
		alternateFlag = FALSE;
	}
	else
	{
		sprintf 
		(
			err_str, 
			srchMessage [1], clip (supercessionPart), inmrRec.item_no, BELL
		);
		supercessionFlag = FALSE;
	}
	print_mess (err_str);
	sleep (2);
}

/*
 ***************************************************************************
 * 	Function	:	InmrFindError () 
 ***************************************************************************
 * 	Description	:	Standard error warning message for find item. 
 *
 *	Notes		:	None
 *
 *	Parameters	:	message	-	Message string.
 */
int
InmrFindError (
	char	*message)
{
	int	i;

	for (i = 0; i < 5; i++)
	{
		errmess (message);
		sleep (2);
	}
	return (EXIT_FAILURE);
}

/*
 ***************************************************************************
 * 	Function	:	SearchFindClose () 
 ***************************************************************************
 * 	Description	:	Close search files opened.
 *
 *	Notes		:	None
 *
 *	Parameters	:	None
 */
void
SearchFindClose (void)
{
	if (searchFilesOpen	== TRUE)
	{
		abc_fclose	(inmr);
		abc_fclose	(inbm);
		abc_fclose	(cuit);
		abc_fclose	(srsk);
	}
}

/*
 ***************************************************************************
 * 	Function	:	InmrSearch () 
 ***************************************************************************
 * 	Description	:	Standard search function for item master file.
 *
 *	Notes		:	None
 *
 *	Parameters	:	coNo		-	Company number.
 *	          	:	searchValue	-	Search key.    
 *	          	:	hhcuHash	-	Customer hash if applicable.
 *	          	:	custCodesOk	-	Customer codes flag from customer master.
 */
void
InmrSearch (
	char	*coNo, 
	char	*searchValue, 
	long	hhcuHash, 
	char	*custCodesOk)
{
	int		noSearch	= 	0, 
			noFind		=	0;
	int		cc			=	0, 
			valid 		= 	1, 
			breakOut	=	0;

	char	selectTypeFlag [2];
	char	*sptr = (*searchValue == '*') ? (char *)0 : searchValue;

	/*
	 * Open files required by search routines.
	 */
	SearchFindOpen ();

	/*
	 * Check if one of the following search keys active.
	 * validTypeFlags comes from the environment SK_SER
	 * Values are :
	 *				I	-	Item Number
	 *				A	-	Alpha Code
	 *				B	-	Barcode Number
	 *				D	-	Description
	 *				L	-	Alternate Number
	 *				M	-	Maker Number
	 */
	switch (search_key)
	{
	case	FN4:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags);
		break;

	case	FN9:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 1);
		break;

	case	FN10:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 2);
		break;

	case	FN11:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 3);
		break;

	case	FN12:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 4);
		break;

	default:
		return;
	}

	/*
	 * If no values keyed (input blank) and search popup window allowed
	 * then allow user to input more information.
	 */
	if (!strlen (searchValue) && envSkSearchPopup)
	{
		SearchInput ();
		sprintf (selectTypeFlag, "S");
	}

	_work_open (16, 0, 56);

	switch (selectTypeFlag[0])
	{
	case	'L':
	case	'l':
		/*
		 * Select index srsk_alt_id (srck_co_no, srck_alternate)
		 */
		abc_selfield (srsk, "srsk_alt_id");
		save_rec ("#Item Number.", "#Alternate No      Description.");
		break;

	case	'A':
	case	'a':
		/*
		 * Select index srsk_alp_id (srck_co_no, srck_alpha_code)
		 */
		abc_selfield (srsk, "srsk_alp_id");
		save_rec ("#Item Number.", "#Alpha Code.       Description.");
		break;

	case	'M':
	case	'm':
		/*
		 * Select index srsk_mak_id (srck_co_no, srck_maker_no)
		 */
		abc_selfield (srsk, "srsk_mak_id");
		save_rec ("#Item Number.", "#Maker No.         Description.");
		break;

	case	'B':
	case	'b':
		/*
		 * Select index srsk_bar_id (srck_co_no, srck_barcode)
		 */
		abc_selfield (srsk, "srsk_bar_id");
		save_rec ("#Item Number.", "#BarCode No.       Description.");
		break;

	case	'D':
	case	'd':
		/*
		 * Select index srsk_des_id (srck_co_no, srck_description)
		 */
		abc_selfield (srsk, "srsk_des_id");
		save_rec ("#Item Number.", "#Description.");
		break;

	default:
		/*
		 * Select index srsk_des_id (srck_co_no, srck_description)
		 */
		abc_selfield (srsk, "srsk_itm_id");
		save_rec ("#Item Number.", "#Description.");
		break;
	}

	/*
	 * If selectTypeFlag is 'C'ustomer then rather than search the 	
	 * srsk file instead search the cuit file for debtor specific
	 * item codes for the input customer.                        
	 */
	if ((selectTypeFlag[0] == 'C' || selectTypeFlag[0] == 'c') && hhcuHash > 0L)
	{
		cuitRec.hhcu_hash = hhcuHash;
		sprintf (cuitRec.item_no, "%-16.16s", (sptr != (char *)0) ? sptr : " ");
		cc = find_rec (cuit, &cuitRec, GTEQ, "r");
		while (!cc &&
			  (hhcuHash > 0L) &&
			  (custCodesOk[0] == 'Y') &&
			  (cuitRec.hhcu_hash == hhcuHash))
		{
			valid = CheckSearch (cuitRec.item_no, searchValue, &breakOut);
			strcpy (err_str, cuitRec.item_desc);
			if (valid)
			{
				cc = save_rec (cuitRec.item_no, err_str);
				if (cc)
					break;
			}
			else
			{
				if (breakOut)
					break;
			}

			cc = find_rec (cuit, &cuitRec, NEXT, "r");
		}
		cc = disp_srch ();
		work_close ();
		if (cc)
		{
			sprintf (srskRec.item_no,   	"%16.16s", " ");
			sprintf (srskRec.alpha_code, 	"%16.16s", " ");
			sprintf (srskRec.maker_no,  	"%16.16s", " ");
			sprintf (srskRec.barcode,   	"%16.16s", " ");
			sprintf (srskRec.alternate, 	"%16.16s", " ");
			sprintf (srskRec.description, 	"%40.40s", " ");
			__searcherror	=	1;
		}
		else
		{
			cuitRec.hhcu_hash = hhcuHash;
			sprintf (cuitRec.item_no, "%-16.16s", searchValue);
			cc = find_rec (cuit, &cuitRec, EQUAL, "r");
			if (cc)
				file_err (cc, "cuit", "DBFIND");
	
			/*
			 * Select index srsk_hhbr_hash (srsk_hhbr_hash)
			 */
			abc_selfield (srsk, "srsk_hhbr_hash");

			srskRec.hhbr_hash = cuitRec.hhbr_hash;
			cc = find_rec (srsk, &srskRec, EQUAL, "r");
			if (cc)
				file_err (cc, "srsk", "DBFIND");

			__searcherror	=	0;
		}
		/*
		 * Select index cuit_id_no (cuit_hhcu_hash, cuit_hhbr_hash)
		 */
		abc_selfield (cuit, "cuit_id_no");
		return;
	}

	strcpy (srskRec.co_no, coNo);
	sprintf (srskRec.item_no,    "%-16.16s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.alpha_code, "%-16.16s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.maker_no,   "%-16.16s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.barcode,    "%-16.16s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.alternate,  "%-16.16s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.description, "%-40.40s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.category,   "%-11.11s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.sellgrp,    "%-11.11s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.buygrp,     "%-11.11s", (sptr != (char *)0) ? sptr : " ");
	sprintf (srskRec.spare,      "%-11.11s", (sptr != (char *)0) ? sptr : " ");

	cc = find_rec (srsk, &srskRec, GTEQ, "r");
	while (!cc && !strcmp (srskRec.co_no, coNo))
	{
		/* 
		 * Variable used to define if only manufactured items can be displayed.
		 */
		if (manufacturingSrch)
		{
			/*
			 * Check if item is a manufactured item.
			 */
			if (strcmp (srskRec.source, "BP") &&
				strcmp (srskRec.source, "BM") &&
				strcmp (srskRec.source, "MC") &&
				strcmp (srskRec.source, "MP"))
			{
				cc = find_rec (srsk, &srskRec, NEXT, "r");
				continue;
			}
		}
		/* 
		 * Variable used to define if only kit and phantom items are to be 
		 * displayed in the search.
		 */
		if (kitPhantomSrch)
		{
			/*
			 * Check if item is a kit or Phantom item.
			 */
			if (srskRec.srsk_class [0] != 'K' && srskRec.srsk_class [0] != 'P')
			{
				cc = find_rec (srsk, &srskRec, NEXT, "r");
				continue;
			}
		}
		/* 
		 * Variable used to define if only quality controlled items are to be 
		 * displayed in the search.
		 */
		if (qualityControlSrch)
		{
			/*
			 * Check if item is QC item.
			 */
			if (srskRec.qc_reqd [0] != 'Y')
			{
				cc = find_rec (srsk, &srskRec, NEXT, "r");
				continue;
			}
		}
		/*
		 * Exclude items that have a class defined in SK_SER_CLASS_EX.
		 */
		if (excludeClassCodes != (char *)0)
		{
			if (strchr (excludeClassCodes, srskRec.srsk_class[0]) != 0)
			{
				cc = find_rec (srsk, &srskRec, NEXT, "r");
				continue;
			}
		}
		/*
		 * Exclude items that have a class defined in SK_SER_ACTST_EX.
		 */
		if (excludeActiveStatCodes != (char *)0)
		{
			if (strchr (excludeActiveStatCodes, srskRec.active_status[0]) != 0)
			{
				cc = find_rec (srsk, &srskRec, NEXT, "r");
				continue;
			}
		}
		switch (selectTypeFlag[0])
		{
		/*
		 * Process alternate items
		 */
		case	'L':
		case	'l':
			valid = CheckSearch (srskRec.alternate, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s", 
					srskRec.alternate, 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;

		/*
		 * Process barcode items
		 */
		case	'B':
		case	'b':
			valid = CheckSearch (srskRec.barcode, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s", 
					srskRec.barcode, 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;

		/*
		 * Process alpha codes.
		 */
		case	'A':
		case	'a':
			valid = CheckSearch (srskRec.alpha_code, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s", 
					srskRec.alpha_code, 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;

		/*
		 * Process maker number.
		 */
		case	'M':
		case	'm':
			valid = CheckSearch (srskRec.maker_no, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s", 
					srskRec.maker_no, 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;

		/*
		 * Process Description
		 */
		case	'D':
		case	'd':
			valid = CheckSearch (srskRec.description, searchValue, &breakOut);
			sprintf (err_str, "%-*.*s", 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;

		/*
		 * Process Special selection (keyed item NULL and popup search window
		 * is allowed.
		 */
		case	'S':
		case	's':

			noSearch	=	0;
			noFind		=	0;

			/*
			 * Some data has been input for item number from selection
			 */
			if (strlen (itemNo))
			{
				valid = CheckSearch (srskRec.item_no, itemNo, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
					sprintf (err_str, "ITEM-> %s", srskRec.description);
			}
			/*
			 * Some data has been input for alternate number from selection
			 */
			if (strlen (itemAlt))
			{
				valid = CheckSearch (srskRec.alternate, itemAlt, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "ALT.-> (%s) %-30.30s", 
								srskRec.alternate, srskRec.description);
				}
			}
			/*
			 * Some data has been input for maker number from selection
			 */
			if (strlen (itemMaker))
			{
				valid = CheckSearch (srskRec.maker_no, itemMaker, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "MAKE-> (%s) %-30.30s", 
								srskRec.maker_no, srskRec.description);
				}
			}
			/*
			 * Some data has been input for description from selection
			 */
			if (strlen (itemDesc))
			{
				valid = CheckSearch (srskRec.description, itemDesc, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
					sprintf (err_str, "DESC-> %s", srskRec.description);
			}
			/*
			 * Some data has been input for alpha code from selection
			 */
			if (strlen (itemAlpha))
			{
				valid = CheckSearch (srskRec.alpha_code, itemAlpha, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "ALPH-> (%s) %-30.30s", 
								srskRec.alpha_code, srskRec.description);
				}
			}
			/*
			 * Some data has been input for item category from selection
			 */
			if (strlen (itemCategory))
			{
				valid = CheckSearch (srskRec.category, itemCategory, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "CATG-> (%s) %-36.36s", 
								srskRec.category, srskRec.description);
				}
			}
			/*
			 * Some data has been input for selling group from selection
			 */
			if (strlen (itemSellGrp))
			{
				valid = CheckSearch (srskRec.sellgrp, itemSellGrp, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "SELL-> (%s) %s", 
								srskRec.sellgrp, srskRec.description);
				}
			}
			/*
			 * Some data has been input for buying group from selection
			 */
			if (strlen (itemBuyGrp))
			{
				valid = CheckSearch (srskRec.buygrp, itemBuyGrp, &breakOut);
				noSearch++;
				noFind	+= valid;
				if (valid)
				{
					sprintf (err_str, "BUY.-> (%s) %s", 
								srskRec.buygrp, srskRec.description);
				}
			}
			valid	=	FALSE;
			if (andFlag == TRUE && noSearch > 0 && (noSearch == noFind))
				valid	=	TRUE;
			if (andFlag == FALSE && noFind > 0)
				valid	=	TRUE;

			break;

		default:
			valid = CheckSearch (srskRec.item_no, searchValue, &breakOut);
			sprintf (err_str, "%-*.*s", 
					(_wide) ? 40 : 36, 
					(_wide) ? 40 : 36, 
					srskRec.description);
			break;
		}

		if (valid)
		{
			cc = save_rec (srskRec.item_no, err_str);
			if (cc)
				break;
		}
		else
		{
			if (breakOut && selectTypeFlag[0] != 'S')
				break;
		}

		cc = find_rec (srsk, &srskRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&srskRec, 0, sizeof (srskRec));
		__searcherror	=	1;
		return;
	}
	/*
	 * Select index srsk_itm_id (srsk_co_no, srsk_item_no)
	 */
	abc_selfield (srsk, "srsk_itm_id");

	strcpy (srskRec.co_no, coNo);
	sprintf (srskRec.item_no, "%-16.16s", searchValue);
	cc = find_rec (srsk, &srskRec, COMPARISON, "r");
	if (cc)
		file_err (cc, "srsk", "DBFIND");

	strcpy (searchValue, strdup (srskRec.item_no));
	__searcherror	=	0;
}

/*
 * Open search files.
 * Read required environment variables.
 * Setup search messages.
 */
void
SearchFindOpen (void)
{
	char	*sptr;

	if (searchFilesOpen	== TRUE)
		return;

	searchFilesOpen = TRUE;

	abc_alias (inmr, "inmr");
	abc_alias (inbm, "inbm");
	abc_alias (cuit, "cuit");
	abc_alias (srsk, "srsk");
	
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no2");
	open_rec (inbm, inbm_list, INBM_NO_FIELDS, "inbm_id_no");
	open_rec (srsk, srsk_list, SRSK_NO_FIELDS, "srsk_itm_id");

	envLookupPointer	=	chk_env ("SK_LOOKUP");
	strcpy (environmentLookup , "IBCQ");
	if (envLookupPointer != (char *) 0 && strlen (envLookupPointer))
		sprintf (environmentLookup , "%-4.4s", envLookupPointer);

	sptr	=	chk_env ("SK_SER");
	if (sptr == (char *)0)
		strcpy (validTypeFlags, "IABLDYC");
	else
		sprintf (validTypeFlags, "%-5.5s", sptr);

	sptr	=	chk_env ("SK_SER_CLASS_EX");
	if (sptr == (char *)0)
		strcpy (excludeClassCodes, "");
	else
		strcpy (excludeClassCodes, sptr);

	sptr	=	chk_env ("SK_SEARCH_POPUP");
	envSkSearchPopup	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	sptr	=	chk_env ("SK_SER_ACTST_EX");
	if (sptr == (char *)0)
		strcpy (excludeActiveStatCodes, "");
	else
		strcpy (excludeActiveStatCodes, sptr);

	/*-------------------------------------
	| Setup standard translated messages. |
	-------------------------------------*/
	strcpy (srchMessage [0], 
				ML ("Item %s has Base Synonym Item Number %s %c"));
	strcpy (srchMessage [1], 
				ML ("Item %s has been Superceeded by Item Number %s %c"));
	strcpy (srchMessage [2], 
				ML ("Looping Supercession, Check Item Number %s"));
	strcpy (srchMessage [3], 
				ML ("Looping Alternate Synonym, Check Item Number %s"));
	strcpy (srchMessage [4], 
				ML ("Corrupt Supercession, check Item Number %s"));
	strcpy (srchMessage [5], 
				ML ("Corrupt Alternate Synonym, check Item Number %s"));
}

/*
 * Functions and variables for popup window.
 */

int		SrchItemSelect		(void);
int		SrchAlphaSelect		(void);
int		SrchAlternateSelect	(void);
int		SrchMakerSelect		(void);
int		SrchDescSelect		(void);
int		SrchCategorySelect	(void);
int		SrchSellSelect		(void);
int		SrchBuySelect		(void);
int		AndOrFunc			(void);
char	*srchStdMask	=	"UUUUUUUUUUUUUUUU";
char	*srchDesMask	=	"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU";
char	*srchGrpMask	=	"UUUUUU";
char	srchFiller[41]	=	"No value entered for selected field.    ";
char	andOrFiller [10]=   "[&& SRCH]";
void	SrchClearDesc	(void);

menu_type	curr_menu[] = 
{
	{"Item", 	srchFiller, 	SrchItemSelect, 	 "Ii", 	}, 
	{"Desc", 	srchFiller, 	SrchDescSelect, 	 "Dd", 	}, 
	{"Alph", 	srchFiller, 	SrchAlphaSelect, 	 "Aa", 	}, 
	{"Altn", 	srchFiller, 	SrchAlternateSelect, "Aa", 	}, 
	{"Make", 	srchFiller, 	SrchMakerSelect, 	 "Mm", 	}, 
	{"Catg", 	srchFiller, 	SrchCategorySelect, 	 "Cc", 	}, 
	{"S.Grp", 	srchFiller, 	SrchSellSelect, 	 "Ss", 	}, 
	{"B.Grp", 	srchFiller, 	SrchBuySelect, 		 "Bb",  }, 
	{andOrFiller, 	"", 	AndOrFunc, 			 "oOaA", }, 
	{"RESTART", 	"", 		_no_option, 			"", FN1, EXIT | SELECT}, 
	{"EDIT/END", 	"", 		_no_option, 			"", FN16, EXIT | SELECT}, 
	{"", }, 
};

/*
 * Main Processing Routine for popup window.
 */
void
SearchInput (void)
{
	int		i;

	for (i = 0; i < 8; i++)
		sprintf (DESC(i), "%-40.40s", strdup (ML(srchFiller)));

	sprintf (PRMT(8), strdup ("[&& SRCH]"));

	andFlag			=	TRUE;

	strcpy (itemNo , ""); 
	strcpy (itemAlt, "");
	strcpy (itemMaker , "");
	strcpy (itemDesc , "");
	strcpy (itemAlpha , "");
	strcpy (itemCategory , "");
	strcpy (itemSellGrp , "");
	strcpy (itemBuyGrp , "");

	ringClearLine	=	FALSE;
	cl_box (0, 0, 63, 2);
	run_menu (curr_menu, "", 1);
	ringClearLine	=	TRUE;
	last_char	=	FN3;
}

/*
 * Item number selected, clear description line, input value etc. 
 */
int		
SrchItemSelect (void)
{
	SrchClearDesc ();
	getalpha (COLM(0), 2, srchStdMask, itemNo);
	if (strlen (itemNo))
		sprintf (DESC(0), "%-40.40s", strdup (itemNo));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*
 * Item description selected, clear description line, input value etc. 
 */
int		
SrchDescSelect (void)
{
	SrchClearDesc ();
	getalpha (COLM(1), 2, srchDesMask, itemDesc);
	if (strlen (itemDesc))
		sprintf (DESC(1), "%-40.40s", strdup (itemDesc));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item alpha code selected, clear description line, input value etc. 
 */
int		
SrchAlphaSelect	(void)
{
	SrchClearDesc ();
	getalpha (COLM(2), 2, srchStdMask, itemAlpha);
	if (strlen (itemAlpha))
		sprintf (DESC(2), "%-40.40s", strdup (itemAlpha));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item alternate code selected, clear description line, input value etc. 
 */
int		
SrchAlternateSelect	(void)
{
	SrchClearDesc ();
	getalpha (COLM(3), 2, srchStdMask, itemAlt);
	if (strlen (itemAlt))
		sprintf (DESC(3), "%-40.40s", strdup (itemAlt));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item Maker Number selected, clear description line, input value etc. 
 */
int		
SrchMakerSelect	(void)
{
	SrchClearDesc ();
	getalpha (COLM(4), 2, srchStdMask, itemMaker);
	if (strlen (itemMaker))
		sprintf (DESC(4), "%-40.40s", strdup (itemMaker));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item Category selected, clear description line, input value etc. 
 */
int		
SrchCategorySelect (void)
{
	SrchClearDesc ();
	getalpha (COLM(5), 2, "UUUUUUUUUUUUUUUU", itemCategory);
	if (strlen (itemCategory))
		sprintf (DESC(5), "%-40.40s", strdup (itemCategory));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item Selling Group selected, clear description line, input value etc. 
 */
int		
SrchSellSelect (void)
{
	SrchClearDesc ();
	getalpha (COLM(6), 2, srchGrpMask, itemSellGrp);
	if (strlen (itemSellGrp))
		sprintf (DESC(6), "%-40.40s", strdup (itemSellGrp));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * Item Buying Group selected, clear description line, input value etc. 
 */
int		
SrchBuySelect (void)
{
	SrchClearDesc ();
	getalpha (COLM(7), 2, srchGrpMask, itemBuyGrp);
	if (strlen (itemBuyGrp))
		sprintf (DESC(7), "%-40.40s", strdup (itemBuyGrp));
	SrchClearDesc ();
	return (EXIT_SUCCESS);
}
/*
 * AND / OR selection made.
 */
int
AndOrFunc (void)
{
	if (andFlag	== FALSE)
	{
		sprintf (PRMT(8), strdup ("[&& SRCH]"));
		andFlag	= TRUE;
	}
	else
	{
		sprintf (PRMT(8), strdup ("[|| SRCH]"));
		andFlag	= FALSE;
	}
	rv_pr (PRMT (8), COLM (8), 1, 1);
	return (EXIT_SUCCESS);
}

/*
 * Silly clear function, I have character based almost as much as Windoze.
 */
void
SrchClearDesc (void)
{
	print_at (2, 1, "%61.61s", " ");
}

/*
 * Check for search patten.
 */
int
CheckSearch (
	char	*searchString, 
	char 	*inputValue, 
	int 	*breakOutFlag)
{
	char	*sptr;
	char	*pptr;	/* pointer to position in pattern string	*/
	char	*tptr;	/* pointer to position in pattern string	*/
	char	t_string [41];
	char	p_string [41];

	sprintf (t_string,"%-40.40s",searchString);
	sprintf (p_string,"%-.40s",inputValue);
	sptr = clip (t_string);

	while (*sptr)
	{
		*sptr = toupper(*sptr);
		sptr++;
	}

	sptr = strchr (p_string,'*');
	/*
	 * No Wild Carding
	 */
	if (sptr == (char *)0)
	{
		*breakOutFlag = strncmp (t_string, inputValue, strlen (inputValue));
		return (!*breakOutFlag);
	}

	if (p_string[0] != '*')
	{
		*sptr = '\0';
		/*
		 * Check Beginning of Target String
		 */
		*breakOutFlag = strncmp (p_string, t_string, strlen(p_string));
		if (*breakOutFlag)
			return(0);

		tptr = t_string + strlen (p_string);
		pptr = sptr + 1;
		while (*pptr == '*')
			pptr++;
	}
	else
	{
		*breakOutFlag = 0;
		tptr = t_string;
		pptr = p_string;
		while (*pptr == '*')
			pptr++;
	}

	if (!strlen (pptr))
		return(1);

	sptr = strchr (pptr,'*');
	/*
	 * Check Other Wild Cards
	 */
	while (sptr != (char *)0)
	{
		*sptr = '\0';

		tptr = strstr (tptr, pptr);
		if (tptr == (char *)0)
			return(0);
		tptr += strlen (pptr);
		pptr = sptr + 1;
		sptr = strchr (pptr,'*');
	}

	/*
	 * Look For Match At End
	 */
	if (strlen (pptr) > 0)
	{
		/*
		 * Cannot Match 
		 */
		if (strlen (pptr) > strlen (tptr))
			return(0);

		sptr = tptr + strlen (tptr) - strlen (pptr);

		return (!strcmp (sptr, pptr));
	}
	return(1);
}
