/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cus_disc.c,v 5.13 2002/10/07 03:37:53 robert Exp $
|  Program Name  : (sk_cus_disc.c)
|  Program Desc  : (Add / Maintain Discount Structure)
|                 (Renamed from so_dismaint.c)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 12/02/88         |
|---------------------------------------------------------------------|
| $Log: cus_disc.c,v $
| Revision 5.13  2002/10/07 03:37:53  robert
| SC 4279 - fixed box alignment
|
| Revision 5.12  2002/07/24 08:39:12  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.11  2002/07/18 07:15:52  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.10  2002/06/26 05:48:48  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.9  2002/06/20 07:10:54  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/01/30 05:52:08  robert
| SC 00730/00731 - Updated to adjust box display for LS10-GUI
|
| Revision 5.7  2001/12/13 05:48:10  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cus_disc.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cus_disc/cus_disc.c,v 5.13 2002/10/07 03:37:53 robert Exp $";

#define	MAXLINES	6
#define	TABLINES	6
#define	ScreenWidth	80

#include <pslscr.h>
#include <minimenu.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	NO_SELECT	-2
#define	SEL_UPDATE		0
#define	SEL_IGNORE		1
#define	SEL_DELETE		2

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct cuitRecord	cuit_rec;
struct esmrRecord	esmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct indsRecord	inds_rec;
struct excfRecord	excf_rec;
struct exclRecord	excl_rec;
struct ingpRecord	ingp_rec;

	double	*inds_qty_brk	=	&inds_rec.qty_brk1;
	float	*inds_disca_pc	=	&inds_rec.disca_pc1;
	float	*inds_discb_pc	=	&inds_rec.discb_pc1;
	float	*inds_discc_pc	=	&inds_rec.discc_pc1;

#define	CUSTNO_CAT		0
#define	CUSTNO_ITEM		1
#define	CUSTNO_SGRP		2
#define	CUSTYPE_CAT		3
#define	CUSTYPE_ITEM	4
#define	CUSTYPE_SGRP	5
#define	PRICE_CAT		6
#define	PRICE_ITEM		7
#define	PRICE_SGRP		8

   	int  	newEntry = 0, 
			envSkDbPriNum, 
			envSkCusDisLvl, 
			byWhat, 
			envDbFind, 
			envDbCo, 
			insFlag;

	int		envSoDiscRev =	FALSE;

	char	branchNumber [3],
			*cust_str = "UUUUUU",
			*type_str = "UUUUUU",
			*cat_str  = "UUUUUUUUUUUUUUUU",
			*prce_str = "N",
			*data	= "data";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	line1_label [17];
	char	line1_type [17];
	char	line1_mask [7];
	char	line1_prmpt [17];
	char	line1_help [80];
	char	line1_value [7];
	char	line1_desc [41];
	char	cat_label [17];
	char	cat_mask [17];
	char	cat_prmpt [18];
	char	cat_help [80];
	char	cat_value [17];
	char	cat_desc [41];
	char	cat2_label [17];
	char	cat2_mask [17];
	char	cat2_prmpt [18];
	char	cat2_help [80];
	char	cat2_value [17];
	char	cat2_desc [41];
	char	disc_by [2];
	char	disc_desc [9];
	char	cum_disc [2];
	char	cum_desc [4];
	char	br_no [3];
	char	br_name [16];
	char	wh_no [3];
	char	wh_name [10];
	long	hhcc_hash;
	double	qty_brk;
	float	disc_a;
	float	disc_b;
	float	disc_c;
	long	hhbr_hash;
	int		price_type;
	char	break_prmpt [12];
} local_rec;


struct storeRec
{
	double	_qty_brk;
	float	_disc_a;
	float	_disc_b;
	float	_disc_c;
} store [MAXLINES];


static	struct	var	vars [] =
{
	{1, LIN, local_rec.line1_label, 	 4, 13, CHARTYPE, 
		local_rec.line1_mask, "          ", 
		" ", "", local_rec.line1_prmpt, local_rec.line1_help, 
		 NE, NO,  JUSTLEFT, "", "", local_rec.line1_value}, 
	{1, LIN, "line1_desc", 	 4, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.line1_desc}, 
	{1, LIN, local_rec.cat_label, 	 5, 13, CHARTYPE, 
		local_rec.cat_mask, "          ", 
		" ", "", local_rec.cat_prmpt, local_rec.cat_help, 
		 NE, NO,  JUSTLEFT, "", "", local_rec.cat_value}, 
	{1, LIN, "cat_desc", 	 5, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.cat_desc}, 
	{1, LIN, local_rec.cat2_label, 	 6, 13, CHARTYPE, 
		local_rec.cat2_mask, "          ", 
		" ", "", local_rec.cat2_prmpt, local_rec.cat2_help, 
		 ND, NO,  JUSTLEFT, "", "", local_rec.cat2_value}, 
	{1, LIN, "cat2_desc", 	 6, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 ND, NO,  JUSTLEFT, "", "", local_rec.cat2_desc}, 
	{1, LIN, "br_no", 	7, 16, CHARTYPE, 
		"NN", "          ", 
		" ", "  ", " Branch         ", "Enter Branch Number. Default For All Branches.", 
		 NE, NO, JUSTRIGHT, "", "", local_rec.br_no}, 
	{1, LIN, "br_name", 	7, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.br_name}, 
	{1, LIN, "wh_no", 	8, 16, CHARTYPE, 
		"NN", "          ", 
		" ", "  ", " Warehouse      ", "Enter Warehouse Number. Default For All Warehouses", 
		NE, NO,  JUSTRIGHT, "", "", local_rec.wh_no}, 
	{1, LIN, "wh_name", 	8, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.wh_name}, 
	{1, LIN, "disc_by", 	7, 51, CHARTYPE, 
		"U", "          ", 
		" ", "Q", "Discount by  ", "Enter Quantity Breaks by Q(uantity) or V(alue).", 
		 YES, NO, JUSTLEFT, "QV", "", local_rec.disc_by}, 
	{1, LIN, "disc_desc", 	7, 57, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.disc_desc}, 
	{1, LIN, "cum_disc", 	8, 51, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Cumulative   ", "Y(es) Apply discounts cumulatively, N(o) Apply discounts absolutely", 
		YES, NO,  JUSTLEFT, "NY", "", local_rec.cum_disc}, 
	{1, LIN, "cum_desc", 	8, 57, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.cum_desc}, 

	{2, TAB, "qty_brk", 	MAXLINES, 1, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", local_rec.break_prmpt, " ", 
		 NO, NO,  JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.qty_brk}, 
	{2, TAB, "disc_a", 	MAXLINES, 4, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", " DISCOUNT A ", "", 
		 NO, NO, JUSTRIGHT, "-99.99", "100", (char *)&local_rec.disc_a}, 
	{2, TAB, "disc_b", 	MAXLINES, 4, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", " DISCOUNT B ", "", 
		 NO, NO, JUSTRIGHT, "-99.99", "100", (char *)&local_rec.disc_b}, 
	{2, TAB, "disc_c", 	MAXLINES, 4, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", " DISCOUNT C ", "", 
		 NO, NO, JUSTRIGHT, "-99.99", "100", (char *)&local_rec.disc_c}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 
};

int		LastCaseSelect	=	NO_SELECT;
int		PosInstalled = FALSE;


#include	<FindCumr.h>
/*
 * Function Declarations
 */
float 	CalcOneDisc 	(int, float, float, float);
float 	ScreenDisc 		(float);
int  	DeleteLine 		(void);
int  	heading 		(int);
int  	InsertLine 		(void);
int  	spec_valid 		(int);
void 	CloseDB 		(void);
void 	FindInds 		(int);
void 	OpenDB 			(void);
void 	SetIndsKey 		(void);
void 	shutdown_prog 	(void);
void 	SrchCcmr 		(char *);
void 	SrchEsmr 		(char *);
void 	SrchExcf 		(char *);
void 	SrchExcl 		(char *);
void 	SrchIngp 		(char *);
void	SrchPrice		(void);
void 	UpdateCustCat 	(void);
void 	UpdateMenu 		(void);
void 	Update 			(void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);


	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envSkDbPriNum = 5;
	else
		envSkDbPriNum = atoi (sptr);

	sptr = chk_env ("SK_CUSDIS_LVL");
	if (sptr == (char *)0)
		envSkCusDisLvl = 0;
	else
		envSkCusDisLvl = atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	envSoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (argc != 2)
	{
		print_at (0, 0, mlSkMess019);
		print_at (1, 0, mlSkMess138);
		print_at (2, 0, mlSkMess139);
		print_at (3, 0, mlSkMess140);
		print_at (4, 0, mlSkMess141);
		print_at (5, 0, mlSkMess142);
		print_at (6, 0, mlSkMess143);
		print_at (7, 0, mlSkMess144);
		print_at (8, 0, mlSkMess145);
		return (EXIT_FAILURE);
	}

	switch (argv [1][0])
	{
	case	'I':
	case	'i':
	case	'1':
		strcpy (local_rec.line1_label, "cust_no");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-6.6s", cust_str);
		strcpy (local_rec.line1_prmpt, "Customer No  ");
		strcpy (local_rec.line1_help, "Enter Customer Number, [SEARCH] Available");

		strcpy (local_rec.cat_label, "item_no");
		sprintf (local_rec.cat_mask, "%-16.16s", cat_str);
		strcpy (local_rec.cat_prmpt, "Item Number  ");
		strcpy (local_rec.cat_help, "Enter Item Number. [SEARCH] And Customer Specifics Available");
		byWhat = CUSTNO_ITEM;
		break;

	case	'C':
	case	'c':
	case	'2':
		strcpy (local_rec.line1_label, "cust_no");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-6.6s", cust_str);
		strcpy (local_rec.line1_prmpt, "Customer No  ");
		strcpy (local_rec.line1_help, "Enter Customer Number, [SEARCH] Available");

		strcpy (local_rec.cat_label, "category");
		sprintf (local_rec.cat_mask, "%-11.11s", cat_str);
		strcpy (local_rec.cat_prmpt, "Category From ");
		strcpy (local_rec.cat_help, "Enter Item Category From , [SEARCH] Available");
		strcpy (local_rec.cat2_label, "category2");
		sprintf (local_rec.cat2_mask, "%-11.11s", cat_str);
		strcpy (local_rec.cat2_prmpt, "Category to  ");
		strcpy (local_rec.cat2_help, "Enter Item Category, [SEARCH] Available");

		FLD ("category2")   = NE;
		FLD ("cat2_desc") 	= NA;
		byWhat = CUSTNO_CAT;
		break;

	case	'S':
	case	's':
	case	'3':
		strcpy (local_rec.line1_label, "cust_no");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-6.6s", cust_str);
		strcpy (local_rec.line1_prmpt, "Customer No  ");
		strcpy (local_rec.line1_help, "Enter Customer Number, [SEARCH] Available");

		strcpy (local_rec.cat_label, "sel_group");
		sprintf (local_rec.cat_mask, "%-6.6s", cat_str);
		strcpy (local_rec.cat_prmpt, "Sell Group   ");
		strcpy (local_rec.cat_help, "Enter Sell Group, [SEARCH] Available");
		byWhat = CUSTNO_SGRP;
		break;

	case	'U':
	case	'u':
	case	'4':
		strcpy (local_rec.line1_label, "cust_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-3.3s", type_str);
		strcpy (local_rec.line1_prmpt, "Cust Type    ");
		strcpy (local_rec.line1_help, "Enter Customer Type, [SEARCH] Available");

		strcpy (local_rec.cat_label, "item_no");
		sprintf (local_rec.cat_mask, "%-16.16s", cat_str);
		strcpy (local_rec.cat_prmpt, "Item Number  ");
		strcpy (local_rec.cat_help, "Enter Item Number, [SEARCH] Available");
		byWhat = CUSTYPE_ITEM;
		break;

	case	'T':
	case	't':
	case	'5':
		strcpy (local_rec.line1_label, "cust_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-3.3s", type_str);
		strcpy (local_rec.line1_prmpt, "Cust Type    ");
		strcpy (local_rec.line1_help, "Enter Customer Type, [SEARCH] Available");

		sprintf (local_rec.cat_mask, "%-11.11s", cat_str);
		strcpy (local_rec.cat_prmpt, "Category     ");
		strcpy (local_rec.cat_label, "category");
		strcpy (local_rec.cat_help, "Enter Item Category, [SEARCH] Available");
		byWhat = CUSTYPE_CAT;
		break;

	case	'V':
	case	'v':
	case	'6':
		strcpy (local_rec.line1_label, "cust_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-3.3s", type_str);
		strcpy (local_rec.line1_prmpt, "Cust Type    ");
		strcpy (local_rec.line1_help, "Enter Customer Type, [SEARCH] Available");

		strcpy (local_rec.cat_label, "sel_group");
		sprintf (local_rec.cat_mask, "%-6.6s", cat_str);
		strcpy (local_rec.cat_prmpt, "Sell Group   ");
		strcpy (local_rec.cat_help, "Enter Sell Group, [SEARCH] Available");
		byWhat = CUSTYPE_SGRP;
		break;

	case	'Y':
	case	'y':
	case	'7':
		strcpy (local_rec.line1_label, "price_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-1.1s", prce_str);
		strcpy (local_rec.line1_prmpt, "Price Type   ");
		sprintf (local_rec.line1_help, "Enter Price Type, Types 1 - %1d Available", envSkDbPriNum);

		strcpy (local_rec.cat_label, "item_no");
		sprintf (local_rec.cat_mask, "%-16.16s", cat_str);
		strcpy (local_rec.cat_prmpt, "Item Number  ");
		strcpy (local_rec.cat_help, "Enter Item Number, [SEARCH] Available");
		byWhat = PRICE_ITEM;
		break;

	case	'X':
	case	'x':
	case	'8':
		strcpy (local_rec.line1_label, "price_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-1.1s", prce_str);
		strcpy (local_rec.line1_prmpt, "Price Type   ");
		sprintf (local_rec.line1_help, "Enter Price Type, Types 1 - %1d Available", envSkDbPriNum);

		sprintf (local_rec.cat_mask, "%-11.11s", cat_str);
		strcpy (local_rec.cat_prmpt, "Category     ");
		strcpy (local_rec.cat_label, "category");
		strcpy (local_rec.cat_help, "Enter Item Category, [SEARCH] Available");
		byWhat = PRICE_CAT;
		break;

	case	'Z':
	case	'z':
	case	'9':
		strcpy (local_rec.line1_label, "price_type");
		strcpy (local_rec.line1_type, "CHARTYPE");
		sprintf (local_rec.line1_mask, "%-1.1s", prce_str);
		strcpy (local_rec.line1_prmpt, "Price Type   ");
		sprintf (local_rec.line1_help, "Enter Price Type, Types 1 - %1d Available", envSkDbPriNum);

		strcpy (local_rec.cat_label, "sel_group");
		sprintf (local_rec.cat_mask, "%-6.6s", cat_str);
		strcpy (local_rec.cat_prmpt, "Sell Group   ");
		strcpy (local_rec.cat_help, "Enter Sell Group, [SEARCH] Available");
		byWhat = PRICE_SGRP;
		break;

	default:
		return (EXIT_FAILURE);
	}
	if (envSkCusDisLvl == 0) /* Company Level Discounts */
	{
		FLD ("br_no")   = ND;
		FLD ("br_name") = ND;
		FLD ("wh_no")   = ND;
		FLD ("wh_name") = ND;
		SCN_COL ("disc_by")		= 13;
		SCN_COL ("disc_desc")  	= 17;
		SCN_COL ("cum_disc")   	= 13;
		SCN_COL ("cum_desc")   	= 17;
	}

	if (envSkCusDisLvl == 1) /* Branch Level Discounts */
	{
		FLD ("wh_no")   = ND;
		FLD ("wh_name") = ND;
	}

	/*
	 * Setup required parameters.
	 */
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


	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.est_no);

	tab_row = 11;
	tab_col = 14;

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/

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
		lcount [2] = 0;


		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		heading (2);
		/*------------------------------
		| Enter screen 2 tabular input.|
		------------------------------*/
		if (newEntry == TRUE)
			entry (2);
		else
			edit (2);

		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (prog_exit || restart) 
			continue;

		/*------------------------------
		| Update selection status.     |
		------------------------------*/
		if (byWhat == CUSTNO_CAT)
			UpdateCustCat ();
		else
			Update ();
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind) ? "cumr_id_no3" 
							                            : "cumr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");

	if (byWhat == PRICE_CAT  || 
		 byWhat == PRICE_ITEM || 
		 byWhat == PRICE_SGRP)
		open_rec (inds, inds_list, INDS_NO_FIELDS, "inds_id_no2");
	else
		open_rec (inds, inds_list, INDS_NO_FIELDS, "inds_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (ccmr);
	abc_fclose (cuit);
	abc_fclose (esmr);
	abc_fclose (excf);
	abc_fclose (excl);
	abc_fclose (incc);
	abc_fclose (ingp);
	abc_fclose (inmr);
	abc_fclose (inds);

	SearchFindClose ();
	abc_dbclose (data);
}

/*-------------------------------------------------------------
| CalcOneDisc calculates a single discount figure             |
| from the list of discounts passed based on the              |
| discount type (Cumulative or Absolute)                      |
|                                                             |
| PARAMETERS :                                                |
|  int	 cumulative - Discounts are cumulative (TRUE / FALSE) |
|  float discA		- discount A.                             |
|  float discB		- discount B.                             |
|  float discC		- discount C.                             |
|                                                             |
| RETURNS : A float containing the single discount.           |
-------------------------------------------------------------*/
float	
CalcOneDisc (
 int cumulative, 
 float discA, 
 float discB, 
 float discC)
{
	int		i;
	int		numDiscs;
	float	nDisc [10];
	float	tmpDisc;
	float	newDisc;

	nDisc [0] = discA;
	nDisc [1] = discB;
	nDisc [2] = discC;
	numDiscs = 3;

	/*------------------------------------
	| If absolute then add all together. |
	------------------------------------*/
	if (!cumulative)
	{
		newDisc = (float)0.00;
		for (i = 0; i < numDiscs; i++)
			newDisc += nDisc [i];

		return (newDisc);
	}

	/*----------------------------
	| Calculate single discount. |
	----------------------------*/
	newDisc = nDisc [0];
	for (i = 1; i < numDiscs; i++)
	{
		tmpDisc = (float)100.00 - newDisc;
		tmpDisc *= nDisc [i];
		tmpDisc = (float) (twodec (tmpDisc / (float)100.00));

		newDisc += tmpDisc;
	}

	newDisc = (float) (twodec (newDisc));

	return (newDisc);
}

int
spec_valid (
 int field)
{
	int		i;
	float	discTot;

	/*----------------------------
	| Validate Customer Number.. |
	----------------------------*/
	if (LCHECK ("cust_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------------------
		| Find Customer Master file details. |
		------------------------------------*/
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.line1_value));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.line1_desc, "%-40.40s", cumr_rec.dbt_name);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Customer Type. |
	-------------------------*/
	if (LCHECK ("cust_type"))
	{
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excl_rec.co_no, comm_rec.co_no);
		sprintf (excl_rec.class_type, "%-3.3s", local_rec.line1_value);
		cc = find_rec (excl, &excl_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess170));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.line1_desc, "%-35.35s", excl_rec.class_desc);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate Price Type. |
	----------------------*/
	if (LCHECK ("price_type"))
	{
		if (SRCH_KEY)
		{
			SrchPrice ();
			return (EXIT_SUCCESS);
		}
		i = atoi (local_rec.line1_value);
		if (i < 1 || i > envSkDbPriNum)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.price_type = i;
		switch (i)
		{
			case	1:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price1_desc);
			break;

			case	2:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price2_desc);
			break;

			case	3:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price3_desc);
			break;

			case	4:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price4_desc);
			break;

			case	5:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price5_desc);
			break;

			case	6:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price6_desc);
			break;

			case	7:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price7_desc);
			break;

			case	8:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price8_desc);
			break;

			case	9:
			sprintf (local_rec.line1_desc, "%-35.35s", comm_rec.price9_desc);
			break;
		}
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			if (byWhat == CUSTNO_CAT  || 
				 byWhat == CUSTNO_ITEM || 
				 byWhat == CUSTNO_SGRP)
			{
				InmrSearch 
				 (
					comm_rec.co_no, 
					temp_str, 
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
			}		
			else
			{
				InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			}
			return (EXIT_SUCCESS);
		}
		clear_mess ();
		
		if (byWhat == CUSTNO_CAT  || 
			 byWhat == CUSTNO_ITEM || 
			 byWhat == CUSTNO_SGRP)
		{
			cc = FindInmr 
			 	 (
					comm_rec.co_no, 
					local_rec.cat_value, 
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
		}
		else
		{
			cc = FindInmr (comm_rec.co_no, local_rec.cat_value, 0L, "N");
		}
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.cat_value);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.cat_value,  "%-16.16s",  inmr_rec.item_no);

		SuperSynonymError ();
		
		sprintf (local_rec.cat_desc, "%-40.40s", inmr_rec.description);
		local_rec.hhbr_hash = inmr_rec.hhbr_hash;
		DSP_FLD ("cat_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Inventory Category. |
	------------------------------*/
	if (LCHECK ("category"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.cat_value);
		if (!dflt_used)
		{
			cc = find_rec (excf , &excf_rec, COMPARISON, "r");
			if (cc) 
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.cat_value, "%-11.11s", "           ");
			sprintf (local_rec.cat_desc, "%-35.35s", "END OF RANGE");
		}
		if (prog_status != ENTRY && strcmp (local_rec.cat_value, local_rec.cat2_value) > 0)
		{
			/*NOTE Range<%s is now GREATER than %s>, 
					local_rec.cat_value, local_rec.cat2_value);*/
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.cat_desc, "%-40.40s", excf_rec.cat_desc);
		DSP_FLD ("cat_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Inventory Category. |
	------------------------------*/
	if (LCHECK ("category2"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", local_rec.cat2_value);
		if (!dflt_used)
		{
			cc = find_rec (excf , &excf_rec, COMPARISON, "r");
			if (cc) 
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.cat2_value, "%-11.11s", "~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc, "%-40.40s", "END OF RANGE");
		}
		if (strcmp (local_rec.cat_value, local_rec.cat2_value) > 0)
		{
			/*Invalid Range < %s is GREATER than %s>, 
						local_rec.cat_value, local_rec.cat2_value*/
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.cat2_desc, "%-40.40s", excf_rec.cat_desc);
		DSP_FLD ("cat2_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate Selling Group And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("sel_group"))
	{
		if (SRCH_KEY)
		{
	 		SrchIngp (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		sprintf (ingp_rec.code, "%-6.6s", local_rec.cat_value);
		strcpy (ingp_rec.type, "S");
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			/*Selling group %s is not on file., local_rec.cat_value*/
			sprintf (err_str, ML (mlStdMess208), local_rec.cat_value);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.cat_desc, "%-35.35s", ingp_rec.desc);
		DSP_FLD ("cat_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Branch Number. |
	-------------------------*/
	if (LCHECK ("br_no"))
	{
		if (envSkCusDisLvl == 0)
		{
			strcpy (local_rec.br_no, "  ");
			strcpy (local_rec.wh_no, "  ");
			local_rec.hhcc_hash = 0L;
			FindInds (field);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.br_name, "All Branches   ");
			DSP_FLD ("br_name");
			strcpy (local_rec.wh_no, "  ");
			if (envSkCusDisLvl == 2)
			{
				FLD ("wh_no") = NA;
				DSP_FLD ("wh_no");
				strcpy (local_rec.wh_name, "All Wrhs ");
				DSP_FLD ("wh_name");
			}
			local_rec.hhcc_hash = 0L;
			FindInds (field);
			return (EXIT_SUCCESS);
		}
		if (envSkCusDisLvl == 2)
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
			errmess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (envSkCusDisLvl == 1)
		{
			strcpy (local_rec.wh_no, "  ");
			local_rec.hhcc_hash = 0L;
			FindInds (field);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Warehouse Number. |
	----------------------------*/
	if (LCHECK ("wh_no"))
	{
		if (envSkCusDisLvl == 0 || envSkCusDisLvl == 1)
			return (EXIT_SUCCESS);
		
		if (FLD ("wh_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.wh_no, "  ");
			DSP_FLD ("wh_no");
			strcpy (local_rec.wh_name, "All Wrhs ");
			DSP_FLD ("wh_name");
			local_rec.hhcc_hash = 0L;
			FindInds (field);
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
		strcpy (local_rec.wh_name, ccmr_rec.acronym);
		DSP_FLD ("wh_name");
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		FindInds (field);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("disc_by"))
	{
		if (local_rec.disc_by [0] == 'Q')
		{
			strcpy (local_rec.disc_desc, "Quantity");
			strcpy (local_rec.break_prmpt, " QTY BREAK ");
		}
		else
		{
			strcpy (local_rec.disc_desc, "Value   ");
			strcpy (local_rec.break_prmpt, " VAL BREAK ");
		}
		DSP_FLD ("disc_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cum_disc"))
	{
		for (i = 0; i < MAXLINES; i++)
		{
			if (store [i]._qty_brk != 0.00)
			{
				discTot = CalcOneDisc ((local_rec.cum_disc [0] == 'Y'), 
									   store [i]._disc_a, 
									   store [i]._disc_b, 
									   store [i]._disc_c);
				if (discTot > 99.99)
				{
					/*Combined discount total on line %1d exceeds 99.99%%, i + 1);*/
					sprintf (err_str, ML (mlSkMess102));
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				if (discTot < -99.99)
				{
					/*Combined surcharge total on line %1d exceeds -99.99%%, i + 1);*/
					print_mess (ML (mlSkMess103));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}

		if (local_rec.cum_disc [0] == 'Y')
			strcpy (local_rec.cum_desc, "Yes");
		else
			strcpy (local_rec.cum_desc, "No ");

		DSP_FLD ("cum_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate quantity break. |
	--------------------------*/
	if (LCHECK ("qty_brk"))
	{
		if (last_char == INSLINE)
			return (InsertLine ());

		if (dflt_used || last_char == DELLINE)
			return (DeleteLine ());

		if (local_rec.qty_brk == 0.00)
		{
			if (prog_status == ENTRY)
				return (DeleteLine ());

			while (line_cnt < lcount [2])
				cc = DeleteLine ();
			return (cc);
		}

		if ((line_cnt > 0) &&
			 (local_rec.qty_brk <= store [line_cnt - 1]._qty_brk))
		{
			/*Quantity break must be greater than previous break.*/
			print_mess (ML (mlSkMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((line_cnt < lcount [2]) &&
			 (local_rec.qty_brk >= store [line_cnt + 1]._qty_brk) &&
			 (store [line_cnt + 1]._qty_brk > 0))
		{
			/*Quantity break must be less than succeeding break.*/
			print_mess (ML (mlSkMess105));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		store [line_cnt]._qty_brk = local_rec.qty_brk;
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate Discount A. |
	----------------------*/
	if (LCHECK ("disc_a"))
	{
		store [line_cnt]._disc_a = local_rec.disc_a;
		discTot = CalcOneDisc ((local_rec.cum_disc [0] == 'Y'), 
							   store [line_cnt]._disc_a, 
							   store [line_cnt]._disc_b, 
							   store [line_cnt]._disc_c);
		if (discTot > 99.99)
		{
			/*Combined discount total exceeds 99.99%% */
			print_mess (ML (mlSkMess102));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (discTot < -99.99)
		{
			/*Combined surcharge total exceeds -99.99%% \007*/
			print_mess (ML (mlSkMess103));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	/*----------------------
	| Validate Discount B. |
	----------------------*/
	if (LCHECK ("disc_b"))
	{
		store [line_cnt]._disc_b = local_rec.disc_b;
		discTot = CalcOneDisc ((local_rec.cum_disc [0] == 'Y'), 
							   store [line_cnt]._disc_a, 
							   store [line_cnt]._disc_b, 
							   store [line_cnt]._disc_c);
		if (discTot > 99.99)
		{
			/*Combined discount total exceeds 99.99%% */
			print_mess (ML (mlSkMess102));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (discTot < -99.99)
		{
			/*Combined surcharge total exceeds -99.99%% */
			print_mess (ML (mlSkMess103));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	/*----------------------
	| Validate Discount C. |
	----------------------*/
	if (LCHECK ("disc_c"))
	{
		store [line_cnt]._disc_c = local_rec.disc_c;
		discTot = CalcOneDisc ((local_rec.cum_disc [0] == 'Y'), 
							   store [line_cnt]._disc_a, 
							   store [line_cnt]._disc_b, 
							   store [line_cnt]._disc_c);
		if (discTot > 99.99)
		{
			/*Combined discount total exceeds 99.99%% */
			print_mess (ML (mlSkMess102)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (discTot < -99.99)
		{
			/*Combined surcharge total exceeds -99.99%% */
			print_mess (ML (mlSkMess103));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*================================================
| Set key for reading inds_rec based on byWhat. |
================================================*/
void
SetIndsKey (void)
{
	strcpy (inds_rec.co_no, comm_rec.co_no);
	strcpy (inds_rec.br_no, local_rec.br_no);
	inds_rec.hhcc_hash = local_rec.hhcc_hash;

	if (byWhat == PRICE_CAT  || 
		 byWhat == PRICE_ITEM || 
		 byWhat == PRICE_SGRP)
		inds_rec.price_type = local_rec.price_type;
	else
		inds_rec.price_type = 0;

	if (byWhat == CUSTNO_CAT  || 
		 byWhat == CUSTNO_ITEM || 
		 byWhat == CUSTNO_SGRP)
		inds_rec.hhcu_hash = cumr_rec.hhcu_hash;
	else
		inds_rec.hhcu_hash = 0L;

	sprintf (inds_rec.cust_type, "%-3.3s", (byWhat == CUSTYPE_CAT  || 
										   byWhat == CUSTYPE_ITEM ||
										   byWhat == CUSTYPE_SGRP) ? 
				                           local_rec.line1_value : " ");

	sprintf (inds_rec.category, "%-11.11s", (byWhat == CUSTNO_CAT  || 
											byWhat == CUSTYPE_CAT ||
			                                byWhat == PRICE_CAT) ? 
				                            local_rec.cat_value : " ");

	sprintf (inds_rec.sel_group, "%-6.6s",  (byWhat == CUSTNO_SGRP  || 
											 byWhat == CUSTYPE_SGRP ||
			                                 byWhat == PRICE_SGRP) ? 
				                             local_rec.cat_value : " ");

	inds_rec.hhbr_hash = (byWhat == CUSTNO_ITEM  || 
						  byWhat == CUSTYPE_ITEM || 
						  byWhat == PRICE_ITEM ) ? 
				          local_rec.hhbr_hash : 0L;

}

/*==========================
| Find discount structure. |
==========================*/
void
FindInds (
	int		field)
{
	int		i;

	SetIndsKey ();

	cc = find_rec (inds, &inds_rec, EQUAL, "u");
	if (cc)
	{
		newEntry = 1;
		display_field (field + 1);
	}
	else
	{
		newEntry = 0;
		entry_exit = 1;
	}

	lcount [2] = 0;
	if (!newEntry)
	{
		entry_exit = 1;

		strcpy (local_rec.disc_by, inds_rec.disc_by);
		if (local_rec.disc_by [0] == 'Q')
		{
			strcpy (local_rec.disc_desc, "Quantity");
			strcpy (local_rec.break_prmpt, " QTY BREAK ");
		}
		else
		{
			strcpy (local_rec.disc_desc, "Value   ");
			strcpy (local_rec.break_prmpt, " VAL BREAK ");
		}
		DSP_FLD ("disc_desc");

		strcpy (local_rec.cum_disc, inds_rec.cum_disc);
		if (local_rec.cum_disc [0] == 'Y')
			strcpy (local_rec.cum_desc, "Yes");
		else
			strcpy (local_rec.cum_desc, "No ");
		DSP_FLD ("cum_desc");

		/*----------------------------
		| Set screen 2 - for putval. |
		----------------------------*/
		scn_set (2);
	
		init_vars (2);
		for (i = 0; i < 6; i++)
		{
			store [i]._qty_brk 	= inds_qty_brk [i];
			store [i]._disc_a 	= ScreenDisc (inds_disca_pc [i]);
			store [i]._disc_b 	= ScreenDisc (inds_discb_pc [i]);
			store [i]._disc_c 	= ScreenDisc (inds_discc_pc [i]);
			local_rec.qty_brk 	= inds_qty_brk [i];
			local_rec.disc_a 	= ScreenDisc (inds_disca_pc [i]);
			local_rec.disc_b 	= ScreenDisc (inds_discb_pc [i]);
			local_rec.disc_c 	= ScreenDisc (inds_discc_pc [i]);
			if (store [i]._qty_brk > 0.00)
				putval (lcount [2]++);
		}
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			store [i]._qty_brk = 0.00;
			store [i]._disc_a = 0.00;
			store [i]._disc_b = 0.00;
			store [i]._disc_c = 0.00;
		}
	}
}

/*============================
| Update Discount structure. |
============================*/
void
Update (void)
{
	int		i;
	int		blankRecord;

	if (newEntry)
	{
		SetIndsKey ();

		strcpy (inds_rec.disc_by, local_rec.disc_by);
		strcpy (inds_rec.cum_disc, local_rec.cum_disc);

		blankRecord = TRUE;
		for (i = 0; i < 6; i++)
		{
			inds_qty_brk [i] = store [i]._qty_brk;
			if (inds_qty_brk [i] > 0.00)
				blankRecord = FALSE;

			inds_disca_pc [i] = ScreenDisc (store [i]._disc_a);
			inds_discb_pc [i] = ScreenDisc (store [i]._disc_b);
			inds_discc_pc [i] = ScreenDisc (store [i]._disc_c);
		}

		if (blankRecord == FALSE)
		{
			cc = abc_add (inds, &inds_rec);
			if (cc)
				file_err (cc, inds, "DBADD");
		}
	}
	else
	{
		LastCaseSelect	=	NO_SELECT;
		UpdateMenu ();
	}
}

/*============================
| Update Discount structure. |
============================*/
void
UpdateCustCat (void)
{
	int		i;
	int		blankRecord;

	LastCaseSelect	=	NO_SELECT;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", local_rec.cat_value);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strcmp (excf_rec.co_no, comm_rec.co_no) &&
				  strncmp (	excf_rec.cat_no, local_rec.cat_value, 11) >= 0 &&
				  strncmp (	excf_rec.cat_no, local_rec.cat2_value, 11) <= 0)
	{
		sprintf (local_rec.cat_value, "%-11.11s", excf_rec.cat_no);
		SetIndsKey ();
		newEntry = find_rec (inds, &inds_rec, EQUAL, "u");
		if (newEntry)
		{
			if (LastCaseSelect == SEL_UPDATE || LastCaseSelect == NO_SELECT)
			{
				SetIndsKey ();

				strcpy (inds_rec.disc_by, local_rec.disc_by);
				strcpy (inds_rec.cum_disc, local_rec.cum_disc);

				blankRecord = TRUE;
				for (i = 0; i < 6; i++)
				{
					inds_qty_brk [i] = store [i]._qty_brk;
					if (inds_qty_brk [i] > 0.00)
						blankRecord = FALSE;

					inds_disca_pc [i] = ScreenDisc (store [i]._disc_a);
					inds_discb_pc [i] = ScreenDisc (store [i]._disc_b);
					inds_discc_pc [i] = ScreenDisc (store [i]._disc_c);
				}

				if (blankRecord == FALSE)
				{
					cc = abc_add (inds, &inds_rec);
					if (cc)
					file_err (cc, inds, "DBADD");
				}
			}
		}
		else
			UpdateMenu ();
		
		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
}

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD  ", 
	  " Update Discount Structure File With Changes Made. " }, 
	{ " 2. IGNORE CHANGES ", 
	  " Ignore Changes Just Made To Discount Structure File." }, 
	{ " 3. DELETE RECORD  ", 
	  " Delete Discount Structure Record." }, 
	{ ENDMENU }
};

/*===================
| Update mini menu. |
===================*/
void
UpdateMenu (void)
{
	int		i;
	int		blankRecord;
	int		CaseSel;

	for (;;)
	{
		if (LastCaseSelect == NO_SELECT)
		{
	    	mmenu_print (" UPDATE SELECTION. ",  upd_menu,  0);
	    	CaseSel = mmenu_select (upd_menu);
			LastCaseSelect = CaseSel;
		}
		else
			CaseSel = LastCaseSelect;

	    switch (CaseSel)
	    {
		case SEL_UPDATE :
		case     99 :
			SetIndsKey ();

			strcpy (inds_rec.disc_by, local_rec.disc_by);
			strcpy (inds_rec.cum_disc, local_rec.cum_disc);

			blankRecord = TRUE;
			for (i = 0; i < 6; i++)
			{
				inds_qty_brk [i] = store [i]._qty_brk;
				if (inds_qty_brk [i] > 0.00)
					blankRecord = FALSE;
				inds_disca_pc [i] = ScreenDisc (store [i]._disc_a);
				inds_discb_pc [i] = ScreenDisc (store [i]._disc_b);
				inds_discc_pc [i] = ScreenDisc (store [i]._disc_c);
			}

			if (blankRecord == FALSE)
			{
				cc = abc_update (inds, &inds_rec);
				if (cc)
					file_err (cc, inds, "DBUPDATE");
			}
			else
			{
				abc_unlock (inds);
				cc = abc_delete (inds);
				if (cc)
					file_err (cc, inds, "DBDELETE");
			}

			return;

		case SEL_IGNORE :
		case     -1 :
			abc_unlock (inds);
			return;

		case SEL_DELETE :
			inds_rec.hhcu_hash = 0L;
			strcpy (inds_rec.category,  "REC DELETED.");
			strcpy (inds_rec.cust_type,  "   ");
			inds_rec.hhbr_hash = 0L;
			inds_rec.hhcc_hash = 0L;
			strcpy (inds_rec.disc_by, " ");
			strcpy (inds_rec.cum_disc, " ");

			for (i = 0; i < 6; i++)
			{
				inds_qty_brk [i] = 0.00;
				inds_disca_pc [i] = 0.00;
				inds_discb_pc [i] = 0.00;
				inds_discc_pc [i] = 0.00;
			}
			abc_unlock (inds);
			cc = abc_delete (inds);
			if (cc)
				file_err (cc, inds, "DBDELETE");
			return;
	
		default :
			break;
	    }
	}
}

/*
 * Search for Selling Price Types.
 */
void
SrchPrice (void)
{
	_work_open (2,0,40);
	save_rec ("# ","#Price ");

	cc = save_rec ("1",comm_rec.price1_desc);
	cc = save_rec ("2",comm_rec.price2_desc);
	cc = save_rec ("3",comm_rec.price3_desc);
	cc = save_rec ("4",comm_rec.price4_desc);
	cc = save_rec ("5",comm_rec.price5_desc);
	cc = save_rec ("6",comm_rec.price6_desc);
	cc = save_rec ("7",comm_rec.price7_desc);
	cc = save_rec ("8",comm_rec.price8_desc);
	cc = save_rec ("9",comm_rec.price9_desc);
	cc = disp_srch ();
	work_close ();
}
void
SrchEsmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Branch Description.");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", keyValue);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
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
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Warehouse Description ");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		   !strcmp	 (ccmr_rec.est_no, local_rec.br_no) &&
	       !strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
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
SrchIngp (
	char	*keyValue)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", keyValue);
	strcpy (ingp_rec.type, "S");
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, keyValue, strlen (keyValue)) &&
		   !strcmp (ingp_rec.type, "S"))
	{
		cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	strcpy (ingp_rec.type, "S");
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

void
SrchExcf (
	char	*keyValue)
{
	_work_open (11,0,40);
	save_rec ("#Category", "#Category Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", keyValue);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, keyValue, strlen (keyValue)) && 
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
		file_err (cc,  excf,  "DBFIND");
}

void
SrchExcl (
	char	*keyValue)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Customer Type Description");
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", keyValue);
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strncmp (excl_rec.class_type, keyValue, strlen (keyValue)) && 
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
		file_err (cc,  excl,  "DBFIND");
}

/*
 * Delete line.
 */
int
DeleteLine (void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY && insFlag == 0)
	{
		blank_display ();
		store [line_cnt]._qty_brk = 0.00;
		store [line_cnt]._disc_a = 0.00;
		store [line_cnt]._disc_b = 0.00;
		store [line_cnt]._disc_c = 0.00;
		return (EXIT_FAILURE);
	}

	if (lcount [2] == 0)
	{
		/* 
		 * Cannot Delete Line - No Lines to Delete 
		 */
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [2]; line_cnt++)
	{
		store [line_cnt]._qty_brk = store [line_cnt + 1]._qty_brk;
		store [line_cnt]._disc_a = store [line_cnt + 1]._disc_a;
		store [line_cnt]._disc_b = store [line_cnt + 1]._disc_b;
		store [line_cnt]._disc_c = store [line_cnt + 1]._disc_c;

		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	while (line_cnt <= lcount [2])
	{
		store [line_cnt]._qty_brk = 0.00;
		store [line_cnt]._disc_a = 0.00;
		store [line_cnt]._disc_b = 0.00;
		store [line_cnt]._disc_c = 0.00;

		if (this_page == line_cnt / TABLINES)
			blank_display ();

		line_cnt++;
	}

	line_cnt = i;
	getval (line_cnt);
	if (insFlag == 1)
		entry_exit = 1;
	return (EXIT_SUCCESS);
}

int
InsertLine (void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*
		 * Cannot Insert Lines On Entry
		 */
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("qty_brk")].row)
	{
		/*
		 * Cannot Insert Line - Table is Full
		 */
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	for (i = line_cnt, line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		store [line_cnt]._qty_brk = store [line_cnt - 1]._qty_brk;
		store [line_cnt]._disc_a = store [line_cnt - 1]._disc_a;
		store [line_cnt]._disc_b = store [line_cnt - 1]._disc_b;
		store [line_cnt]._disc_c = store [line_cnt - 1]._disc_c;

		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	store [i]._qty_brk 	= inds_qty_brk [i];
	store [i]._disc_a 	= ScreenDisc (inds_disca_pc [i]);
	store [i]._disc_b 	= ScreenDisc (inds_discb_pc [i]);
	store [i]._disc_c 	= ScreenDisc (inds_discc_pc [i]);
	local_rec.qty_brk 	= inds_qty_brk [i];
	local_rec.disc_a 	= ScreenDisc (inds_disca_pc [i]);
	local_rec.disc_b 	= ScreenDisc (inds_discb_pc [i]);
	local_rec.disc_c 	= ScreenDisc (inds_discc_pc [i]);
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	insFlag = 1;
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	insFlag = 0;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Screen Heading.
 */
int
heading (
	int	scn)
{
	int		i;

	if (restart) 
    	return (EXIT_SUCCESS);

	clear ();

	if (scn != cur_screen)
		scn_set (scn);

	fflush (stdout);
	strcpy (err_str, ML (mlSkMess020));
	i = strlen (err_str);

	rv_pr (err_str, (ScreenWidth - i) / 2, 0, 1);

	/* By Unknown Selection */
	strcpy (err_str, ML (mlSkMess021));
	switch (byWhat)
	{
	/*
	 * By Customer Number And Category
	 */
	case	CUSTNO_CAT:
		strcpy (err_str, ML (mlSkMess022));
		break;

	/*
	 * By Customer And Item Number
	 */
	case	CUSTNO_ITEM:
		strcpy (err_str, ML (mlSkMess023));
		break;

	/*
	 * By Customer And Selling Group 
	 */
	case	CUSTNO_SGRP:
		strcpy (err_str, ML (mlSkMess024));
		break;

	/*
	 * By Customer Type And Category
	 */
	case	CUSTYPE_CAT:
		strcpy (err_str, ML (mlSkMess025));
		break;

	/* 
	 * By Customer Type And Item Number 
	 */
	case	CUSTYPE_ITEM:
		strcpy (err_str, ML (mlSkMess026));
		break;

	/*
	 * By Customer Type And Selling Group 
	 */
	case	CUSTYPE_SGRP:
		strcpy (err_str, ML (mlSkMess027));
		break;

	/* 
	 * By Price Type And Category 
	 */
	case	PRICE_CAT:
		strcpy (err_str, ML (mlSkMess028));
		break;

	/* 
	 * By Price Type And Item Number 
	 */
	case	PRICE_ITEM:
		strcpy (err_str, ML (mlSkMess029));
		break;
	
	/* 
	 * By Price Type And Selling Group 
	 */
	case	PRICE_SGRP:
		strcpy (err_str, ML (mlSkMess030));
		break;
	}
	i = strlen (err_str);

	rv_pr (err_str, (ScreenWidth - i)/2, 2, 1);

	line_at (1,0, ScreenWidth - 1);

	switch (scn)
	{
	case  1 :
		if (prog_status != ENTRY)
		{
			scn_set (2);
			scn_write (2);
			scn_display (2);
#ifdef GVISION
			box (tab_col - 1, tab_row - 1, 52, TABLINES + 2);
#else
			box (tab_col, tab_row - 1, 52, TABLINES + 2);
#endif
		}
		box (0, 3, ScreenWidth - 1, 5);
		scn_set (1);
		scn_write (1);
		scn_display (1);
		break;
	
	case  2 :
		box (0, 3, ScreenWidth - 1, 5);
		scn_set (1);
		scn_write (1);
		scn_display (1);
		scn_set (2);
		scn_write (2);

#ifndef GVISION
		box (tab_col, tab_row - 1, 52, TABLINES + 2);
#endif
		scn_display (2);
		break;
	}
	line_at (20,1, ScreenWidth - 1);
		
	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
    return (EXIT_SUCCESS);
}

/*
 * Reverse Screen Discount.
 */
float	
ScreenDisc (
	float	DiscountPercent)
{
	if (envSoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

