/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_credit.c,v 5.18 2002/07/24 08:39:04 scott Exp $
|  Program Name  : (po_credit.c )                                     |
|  Program Desc  : (Purchase Order Credit note entry.             )   |
|---------------------------------------------------------------------|
|  Date Written  : (20/04/1997)    | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
| $Log: po_credit.c,v $
| Revision 5.18  2002/07/24 08:39:04  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.17  2002/07/18 07:00:27  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.16  2002/07/17 09:57:35  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.15  2002/06/20 07:22:07  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.14  2002/06/19 07:00:39  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.13  2002/05/14 05:20:42  cha
| Updated to make sure that over dispatch PO returns are validated.
|
| Revision 5.12  2001/12/06 05:22:30  scott
| Updated to allow purchase returns that are allocated to an original purchase order to select lot information. The lot display will only show those Location/Lot records that belong to the original receipt. This caters for both multiple receipts of the same product on the same purchase order AND Locations/Lots being split after receipt.
|
| Revision 5.11  2001/11/05 01:37:19  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_credit.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_credit/po_credit.c,v 5.18 2002/07/24 08:39:04 scott Exp $";

#define MAXWIDTH 	300
#define MAXLINES 	1000
#define	TXT_REQD
#include 	<pslscr.h>
#include 	<getnum.h>
#include 	<twodec.h>
#include	<s_terms.h>
#include	<get_lpno.h>
#include	<inis_update.h>
#include	<ml_po_mess.h>
#include	<ml_std_mess.h>
#include	<Costing.h>

#define	PO_HEAD		1
#define	PO_LINES	2

#define	CASE_USED	 (env.PoCaseUsed [0] == 'Y' || env.PoCaseUsed [0] == 'y')
#define	INIS_REQ 	 (env.PoInisReq [0]  == 'Y' || env.PoInisReq [0]  == 'y')

#define	SR	store [line_cnt]

#define	DBOX_TOP	9
#define	DBOX_LFT	35
#define	DBOX_WID	66
#define	DBOX_DEP	3

#define	MAX_PONS	10

int		APPLIED	=	FALSE;
extern	int		_win_func;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct exsiRecord	exsi_rec;
struct inisRecord	inis_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inspRecord	insp_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct pohrRecord	pohr3_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct polnRecord	poln3_rec;
struct polnRecord	poln4_rec;
struct pocrRecord	pocr_rec;
struct polhRecord	polh_rec;
struct podtRecord	podt_rec;
struct pocfRecord	pocf_rec;
struct pogdRecord	pogd_rec;
struct sudsRecord	suds_rec;
struct suinRecord	suin_rec;
struct ponsRecord	pons_rec;

int		*comr_po_sic	=	&comr_rec.po_sic1;
int		*sumr_sic		=	&sumr_rec.sic1;

/*
 * Table names
 */
static	char	*data	= "data", 
				*pohr2	= "pohr2", 
				*pohr3	= "pohr3", 
				*poln2	= "poln2", 
				*poln3	= "poln3", 
				*inum2	= "inum2", 
				*DBFIND = "DBFIND", 
				*fifteenSpaces	=	"               ", 
				*currentUser, 
				*serialSpace = "                         ", 
				*ns_space = "                                        ";
	 
	int		newPoCredit				= FALSE, 
			deleteLine				= 0, 
			updateSupplierRecord	= 0, 
			warehouseSelected 		= FALSE, 
			running					= FALSE, 
			printerNumber 			= 0, 
			new_pogd				= 0, 
			messageCnt				= 0;


	char	branchNumber [3], 
			nd_flag [4], 
			creditNoteProgramName [15], 
			loc_prmt [15], 
			fi_prmt [15], 
			cif_prmt [15], 
			dty_prmt [15], 
			lic_prmt [15];


	long 	currentDate;
		
	FILE	*pout;

    static struct {
	    int		CrCo;
		int		CrFind;
		int		PoOverride;
		int		PoInput;
		int		PoIndentOk;
		int		PoCprint;
		int		PoMaxLines;
		int		PoNumGen;
		int		PoReturnApply;
		int		SoDiscRev;
		char	SupOrdRound [2];
		char	PoCaseUsed [2];
		char	PoInisReq [2];
		char	CurrCode [4];
    } env;

    /*
     * Structure used for pop-up discount screen.
     */
    static	struct {
	    char	fldPrompt [14];
	    int		xPos;
	    char	fldMask [12];
    } discScn [] = {
	    {"FOB (FGN)", 		0,  "NNNNNNNN.NN"}, 
	    {"Reg %", 			14, "NNN.NN"}, 
	    {"Disc A", 			23, "NNN.NN"}, 
	    {"Disc B", 			32, "NNN.NN"}, 
	    {"Disc C", 			41, "NNN.NN"}, 
	    {"NET FOB(FGN)", 	50, "NNNNNNNN.NN"}, 
	    {"", 			0,  ""}, 
    };

	char	cat_desc [10] [21];
	char	*inv_cat [] = {
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

	char	*screens [] = {
			" Header Screen ", 
			" Item Details ", 
		};



	float	ScreenDisc (float);

struct	storeRec 
{
							/*======================================*/
	char	itemClass [2], 	/* Item Class.							*/
			duty_type [2], 	/* duty type from podt					*/
			item_desc [41],	/* Item description.					*/
			costingFlag [2],/* Item costing flag.					*/
			lic_cat [2], 	/* licence category from polh.  		*/
			lic_no [11], 	/* licence no from polh.  				*/
			nsDesc [MAX_PONS + 1] [41], /*Non stock description		*/
			ser_no [26], 	/* Serial number.						*/
			std_uom [5], 	/* Standard (Stock) UOM.				*/
			sup_uom [5], 	/* Supplier UOM.						*/
			uom_group [21];	/* Standard (Stock) UOM group.			*/
	double	base_cost, 		/* Base cost from inis or inei 			*/
			cst_price, 		/* Cost price.							*/
			exch_rate, 		/* Exchange Rate. 						*/
			grs_fgn, 		/* Gross FOB before discounts 			*/
			imp_duty, 		/* duty rate from podt					*/
			land_cost, 		/* Landed cost.  						*/
			lic_hash, 		/* licence hhlc_hash from polh			*/
			lic_rate, 		/* licence rate from polh				*/
			net_fob, 		/* Net FOB after discounts 				*/
			outer, 			/* outer size.       					*/
			quantity, 		/* quantity received					*/
			val_duty, 		/* value of duty						*/
			val_fi;			/* value of freight + ins.				*/
	float	StdCnvFct, 		/* Standard (Stock) UOM.				*/
			air_time, 		/* inis_air_time						*/
			discArray [4], 	/* Regulatory and Disc A, B, C percents */
			lead_time, 		/* inis_lead_time						*/
			lnd_time, 		/* inis_lnd_time						*/
			min_order, 		/* stuff copied from inis_rec			*/
			ord_multiple, 	/* Relates to Min and order multiple.   */
			pur_conv, 		/* Supplier UOM Conversion Fctr			*/
			sea_time, 		/* inis_sea_time						*/
			volume, 		/* inis_volume							*/
			weight, 		/* inis_weight							*/
			closingStock;	/* Closing Stock						*/
	int		cumulative, 	/* Discounts are cumulative ?  			*/
			no_inis, 		/* No Inventory supplier record (inis)  */
			upd_inis;		/* Update inventory supplier record.    */
	long	hhbrHash, 		/* Item Hash from (inmr_hhbr_hash)		*/
			hhumHash, 		/* UOM Hash (inum_hhum_hash)			*/
			hhccHash,     	/* Warehouse (ccmr_hhcc_hash)			*/
			hhwhHash,     	/* Warehouse (incc_hhwh_hash)			*/
			hhplHash,     	/* Purchase order link.          		*/
			ship_no;		/* Shipment number.						*/
							/*======================================*/
} store [MAXLINES];

struct {
							/*======================================*/
	char	dummy [11];		/* Dummy Screen Gen Field.		        */
							/*======================================*/

							/*======================================*/
							/* Header Screen Local field.		    */
							/*======================================*/
	char	pur_ord_no [16];/* Local purchase order number			*/
	char	previousPo [16];/* Previous Purchase order No.			*/
	char	previousCrdNo [7];/* Previous Creditor Number.			*/
	char	systemDate [11];/* Current Date dd/mm/yy.				*/
	char 	supplierNo [7];	/* Current Supplier Number.				*/
	double	exch_rate;		/* Local Exchange Rate.					*/
	char	ship_desc [5];	/* Shipment Method Description.			*/
							/*--------------------------------------*/

							/*======================================*/
							/* Line Item Screen Local field.		*/
							/*======================================*/
	char	item_no [17];	/* Local Item Number.					*/
	char	lic_no [11];	/* Local Licence number.				*/
	char	lic_cat [3];	/* Local Licence Category.				*/
	float	ret_qty;		/* Return Quantity.						*/
	float	rec_qty;		/* Return Quantity.						*/
	double	grs_fgn;		/* Free-on-board Fgn Dollars.			*/
	char	view_disc [2];	/* View discounts ?          			*/
	double	net_fob;		/* Net after discount (Fgn dollars)     */
	double	loc_fi;			/* Over-Seas Freight + Insurance.		*/
	double	cif_loc;		/* Cost/Insurance/Freight Local.		*/
	double	duty_val;		/* Duty Value.							*/
	char	duty_code [3];	/* Duty Code.							*/
	double	lic_val;		/* Licance Value.						*/
	double	other;			/* Other Cost 1-4 + Bank and Interest.  */
	int		case_no;		/* Local Case Number.					*/
	double	land_cst;		/* Landed cost.							*/
	double	Dsp_land_cst;	/* Landed cost.							*/
	double	fob_cost;		/* Free-On-Bourd Cost.					*/
	long	due_date;		/* Due Date at line item Level.			*/
	char	br_no [3];		/* Branch number.						*/
	char	br_name [41];	/* Branch name.							*/
	char	wh_no [3];		/* Warehouse number.					*/
	char	wh_name [41];	/* Warehouse name.						*/
	char	std_uom [5];	/* Standars UOM.						*/
	char	sup_uom [5];	/* Supplier UOM.  						*/
	float	pur_conv;		/* Purchase comversion.					*/
	long	hhccHash;		/* Warehouse hhccHash.					*/
							/*--------------------------------------*/
} local_rec;

static	struct	var	vars [] =
{
	{PO_HEAD, LIN, "supplierNo", 	 3, 22, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.previousCrdNo, "Supplier No.", "Enter Supplier number. [SEARCH] ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.supplierNo}, 
	{PO_HEAD, LIN, "hhsu_hash", 	 0, 4, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *)&sumr_rec.hhsu_hash}, 
	{PO_HEAD, LIN, "name", 	 3, 64, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{PO_HEAD, LIN, "po_credit_no", 	 4, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Purchase Return No.", "Enter Purchase Return number or [SEARCH] <default = new return> ", 
		 NE, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{PO_HEAD, LIN, "date_raised", 	 4, 85, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Date Raised.", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.date_raised}, 
	{PO_HEAD, LIN, "pur_ord_no", 	 5, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Purchase Order No.", "Enter existing P/O number for Supplier. [SEARCH] Must have a quantity received into warehouse. ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.pur_ord_no}, 
	{PO_HEAD, LIN, "ship_method", 	 5, 85, CHARTYPE, 
		"U", "          ", 
		" ", sumr_rec.ship_method, "Shipment Method.", "Shipment Method L(and) / S(ea) / A(ir)", 
		NO, NO,  JUSTLEFT, "LSA", "", pohr_rec.ship_method}, 
	{PO_HEAD, LIN, "ship_desc", 	 5, 88, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.ship_desc}, 
	{PO_HEAD, LIN, "con_name", 	 6, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", sumr_rec.cont_name, "Contact name.", "<RETURN> - standard contact ", 
		 NO, NO,  JUSTLEFT, "", "", pohr_rec.contact}, 
	{PO_HEAD, LIN, "date_conf", 	 6, 85, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Date Confirmed.", "Date return sent or confirmed with Supplier ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.conf_date}, 
	{PO_HEAD, LIN, "torder", 	 7, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Terms of Return.", "ex works/FAS/FOB/CNF/C&F/CIF.", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.term_order}, 
	{PO_HEAD, LIN, "date_reqd", 	 7, 85, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00 ", "Date Due Overide.", "Note : Due date other than 00/00/0000 will overide line item due dates on lines not allocated to a shipment. ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.due_date}, 
	{PO_HEAD, LIN, "btpay", 	 9, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Bank Terms.", "", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.bnk_term_pay}, 
	{PO_HEAD, LIN, "exch_rate", 	9, 85, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", "0", "Exchange Rate.", "<RETURN> - default to current", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.exch_rate}, 
	{PO_HEAD, LIN, "stpay", 	10, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", sumr_rec.pay_terms, "Supplier Terms.", "", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.sup_term_pay}, 
	{PO_HEAD, LIN, "pay_date", 	10, 85, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Return Date.", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.pay_date}, 
	{PO_HEAD, LIN, "req", 		12, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Person Req P/C.", "Details of person requesting purchase Return. ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.req_usr}, 
	{PO_HEAD, LIN, "reason", 	12, 85, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Reason for Return.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.reason}, 
	{PO_HEAD, LIN, "sin1", 	14, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Standard Instr 1.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin1}, 
	{PO_HEAD, LIN, "sin2", 	15, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Standard Instr 2.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin2}, 
	{PO_HEAD, LIN, "sin3", 	16, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Standard Instr 3.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.stdin3}, 
	{PO_HEAD, LIN, "del1", 	18, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Return Instr 1.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin1}, 
	{PO_HEAD, LIN, "del2", 	19, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Return Instr 2.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin2}, 
	{PO_HEAD, LIN, "del3", 	20, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Return Instr 3.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.delin3}, 
	{PO_LINES, TAB, "br_no", 	MAXLINES, 0, CHARTYPE, 
		"NN", "          ", 
		" ", comm_rec.est_no, "BR", "Enter Branch number.", 
		 NE, NO, JUSTRIGHT, "", "", local_rec.br_no}, 
	{PO_LINES, TAB, "br_name", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.br_name}, 
	{PO_LINES, TAB, "wh_no", 	 0, 0, CHARTYPE, 
		"NN", "          ", 
		" ", comm_rec.cc_no, "WH", "Enter Warehouse number.", 
		 NE, NO, JUSTRIGHT, "", "", local_rec.wh_no}, 
	{PO_LINES, TAB, "wh_name", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.wh_name}, 
	{PO_LINES, TAB, "hhccHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.hhccHash}, 
	{PO_LINES, TAB, "item_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "  Item Number.  ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{PO_LINES, TAB, "hhpl_hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *) &poln_rec.hhpl_hash}, 
	{PO_LINES, TAB, "UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", "UOM.", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.sup_uom}, 
	{PO_LINES, TAB, "rec_qty", 	 0, 0, FLOATTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "1", "Order Qty ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.rec_qty}, 
	{PO_LINES, TAB, "ret_qty", 	 0, 0, FLOATTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "1", "Return Qty", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ret_qty}, 
	{PO_LINES, TAB, "lic_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", " Lic. No. ", " ", 
		 ND, NO,  JUSTLEFT, "", "", local_rec.lic_no}, 
	{PO_LINES, TAB, "fob_cst", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NNNN", "          ", 
		" ", "0", "  FOB (FGN)  ", "Enter Cost Per Item. (Return for Last Cost).", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.grs_fgn}, 
	{PO_LINES, TAB, "view_disc", 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "V", " View and Amend Discounts (Y/N) ", 
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.view_disc}, 
	{PO_LINES, TAB, "net_fob", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNNN", "          ", 
		" ", "", "NET FOB (FGN)", "", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.net_fob}, 
	{PO_LINES, TAB, "loc_fi", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NNNN", "          ", 
		" ", "0", fi_prmt, "<Return> will Calculate Freight.", 
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_fi}, 
	{PO_LINES, TAB, "cif_loc", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NNNNNN", "          ", 
		" ", "0", cif_prmt, " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.cif_loc}, 
	{PO_LINES, TAB, "duty_val", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "0", dty_prmt, "<Return> will Calculate Duty.", 
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.duty_val}, 
	{PO_LINES, TAB, "lic_val", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NNNN", "          ", 
		" ", "0", lic_prmt, " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.lic_val}, 
	{PO_LINES, TAB, "other", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "0", "Other Costs", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.other}, 
	{PO_LINES, TAB, "land_cost", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NNNNNN", "          ", 
		" ", "0", " Unit Cost. ", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.land_cst}, 
	{PO_LINES, TAB, "Dsp_land_cost", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NNNN", "          ", 
		" ", "0", " Unit Cost. ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.Dsp_land_cst}, 
	{PO_LINES, TAB, "case_no", 	 0, 1, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Case #", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.case_no}, 
	{PO_LINES, TAB, "due_date", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Due Date", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.due_date}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<proc_sobg.h>
#include	<CheckIndent.h>
#include	<SupPrice.h>
#include 	<FindSumr.h>

/*
 * Function declarations
 */
double 	DutyCalculate 		(int);
double 	FreightCalculate 	(int);
float 	ScreenDisc 			(float);
float 	ToLclUom 			(float, float);
float 	ToStdUom 			(float, float);
int 	CheckPohr 			(char *);
int 	FindInis 			(long, long);
int 	FindPocf 			(char *);
int 	FindPocr 			(char *);
int 	LoadPoln 			(long);
int 	LoadPurchaseOrder 	(long);
int 	LoadSupplier 		(int);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	win_function 		(int, int, int, int);
void 	CalculateCost 		(int);
void	CheckEnvironment	(void);
void 	ClearBox 			(int, int, int, int);
void 	CloseDB 			(void);
void	ClosePrint			(void);
void 	CreditAll 			(int);
void 	DeleteInsf 			(void);
void 	DeletePONS 			(long);
void 	DispFields 			(int);
void 	DrawDiscScn 		(void);
void 	GetWarehouse 		(long);
void 	InputField 			(int);
void 	InputPONS 			(int);
void 	LoadPONS 			(int, long);
void 	OpenDB 				(void);
void 	ReadMisc 			(void);
void 	ShowPay 			(void);
void 	SrchExsi 			(char *);
void 	SrchInum			(char *, int);
void 	SrchPohr 			(char *);
void 	SrchPohr2 			(char *);
void 	SrchPohr3 			(char *);
void 	SrchPolh 			(char *);
void 	Update 				(void);
void 	UpdateInis 			(double);
void 	InsfUpdate 			(void);
void 	UpdatePONS 			(int, long);
void 	UpdatePoln 			(int, long, int);
void 	VertLine 			(int, int);
void 	pr_other 			(void);
void 	tab_other 			(int);


#ifdef GVISION
#include <disc_win.h>
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#else
void ViewDiscounts (void);
#endif	/* GVISION */

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	int		i, 
			cnt;

	if (argc != 2)
	{
		print_at (0, 0, mlPoMess713, argv [0]);
		print_at (1, 0, mlPoMess714);
		print_at (2, 0, mlPoMess715);
		print_at (3, 0, mlPoMess717);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);


	/*
	 * Check environment variables and set values in the envVar structure.
	 */
	CheckEnvironment ();
	_win_func = TRUE;

	/*
	 * Check if costs are updated real time.
	 */
	updateSupplierRecord = chk_inis ();

	/*
	 * Set up Input for Licence/Due Date.
	 */
	sprintf (nd_flag, "%-3.3s", argv [1]);

	FLD ("lic_no")   = (nd_flag [0] == 'Y') ? YES : ND;
	FLD ("lic_val")  = (nd_flag [1] == 'Y') ? YES : ND;
	FLD ("br_no")    = (nd_flag [2] == 'Y') ? NE  : ND;
	FLD ("wh_no")    = (nd_flag [2] == 'Y') ? NE  : ND;
	FLD ("case_no")  = (CASE_USED) ? YES : ND;

	init_scr ();
	set_tty (); 
	_set_masks ("po_credit.s");
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (PO_LINES, store, sizeof (struct storeRec));
#endif
	init_vars (PO_HEAD);
	init_vars (PO_LINES);

	tab_row = 9;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	currentDate = TodaysDate ();

	/*
	 * Open main database files & init
	 */
	OpenDB ();

	ReadMisc ();

	for (i = 0;i < 2;i++)
		tab_data [i]._desc = screens [i];

	/*
	 * Set up Screen Prompts.
	 */
	sprintf (loc_prmt, "  %-3.3s Value  ", env.CurrCode);
	sprintf (fi_prmt, " F&I (%-3.3s) ", env.CurrCode);
	sprintf (cif_prmt, " CIF (%-3.3s) ", env.CurrCode);
	strcpy (dty_prmt, "Duty/ Unit");
	strcpy (lic_prmt, "Lic / Unit");

	strcpy (branchNumber, env.CrCo ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	strcpy (local_rec.previousPo, "000000000000000");
	strcpy (local_rec.previousCrdNo, "000000");

	while (prog_exit == 0)
	{
		if (restart) 
		{
			abc_unlock (pohr);
			abc_unlock (poln);
		}

		for (i = 0;i < MAXLINES;i++)
		{
			memset (store + i, 0, sizeof (struct storeRec));
			strcpy (store [i].duty_type, " ");
			strcpy (store [i].ser_no, serialSpace);
			strcpy (store [i].std_uom, "    ");
			strcpy (store [i].sup_uom, "    ");
			strcpy (store [i].uom_group, "                    ");

			for (cnt = 0; cnt < MAX_PONS; cnt++)
				sprintf (store [i].nsDesc [cnt], "%40.40s", " ");
		}
		eoi_ok 		= TRUE;
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newPoCredit	= FALSE;
		init_ok 	= TRUE;
		skip_tab 	= FALSE;
		init_vars (PO_HEAD);	
		init_vars (PO_LINES);	
		lcount [PO_LINES] = 0;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (PO_HEAD);
		entry (PO_HEAD);

		if (prog_exit || restart)
			continue;

		if (newPoCredit && APPLIED)
		{
			/*
			 * Do You Wish to Credit Whole Purchase Order ?
			 */
			i = prmptmsg (ML (mlPoMess114), "YyNn", 1, 23);
			move (1, 23);
			cl_line ();
			CreditAll ((i == 'Y' || i == 'y') ? TRUE : FALSE);
			i = 'Y';
		}
		else
		{
			i = 'N';
			vars [label ("br_no")].row = MAXLINES;
		}

		if (newPoCredit && i != 'Y' && i != 'y')
		{
			heading (2);
			if (!APPLIED)
				vars [scn_start].required = YES;
			else
				scn_display (2);
			
			if (APPLIED)
			{
				init_ok = FALSE;
				eoi_ok = FALSE;
			}
			entry (2);
			
			vars [scn_start].required = NI;
			if (restart)
				continue;
		}
		else
			scn_display (1);

		prog_status = ! (ENTRY);

		if (!newPoCredit)
			FLD ("br_no") = YES;

		edit_all ();

		if (!restart)
			Update ();
	}

	/*
	 * Program exit sequence.
	 */
	ClosePrint ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
CreditAll (
	int		CreditAll)
{
	scn_set (PO_LINES);

	for (line_cnt = 0;line_cnt < lcount [PO_LINES];line_cnt++)
	{
		getval (line_cnt);
		local_rec.ret_qty = (float) ((CreditAll) ? local_rec.rec_qty : 0.00);
		putval (line_cnt);
	}
	scn_set (PO_HEAD);
}
/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (pohr3, pohr);
	abc_alias (pohr2, pohr);
	abc_alias (poln2, poln);
	abc_alias (poln3, poln);
	abc_alias (inum2, inum);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!env.CrFind) ? "sumr_id_no" 
														     : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (pohr3, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (poln2, poln_list, POLN_NO_FIELDS, "poln_hhpl_orig");
	open_rec (poln3, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_id_no");
	open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no3");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
	open_rec (pons, pons_list, PONS_NO_FIELDS, "pons_id_no");

	OpenInsf ();
	OpenIncf ();
	OpenInei ();
	abc_selfield (insf, "insf_id_no2");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (incc);
	abc_fclose (pohr);
	abc_fclose (pohr3);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (poln3);
	abc_fclose (polh);
	abc_fclose (podt);
	abc_fclose (inis);
	abc_fclose (pogd);
	abc_fclose (pocr);
	abc_fclose (suds);
	abc_fclose (ccmr);
	abc_fclose (pons);
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

void
ClosePrint (
 void)
{
	if (!running)
		return;

	running = 0;
	fprintf (pout, "0\n");

#ifdef GVISION
	Remote_fflush (pout);
#else
	fflush (pout);
#endif

	pclose (pout);
}

void
ReadMisc (void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, DBFIND);

	abc_fclose (comr);

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, DBFIND);
}

int
spec_valid (
	int		field)
{
	int		line_no = 0;
	int		i;
	double	workFob		=	0.00, 
			workFifo	=	0.00;

	if (LCHECK ("br_no"))
	{
		if (F_NOKEY (field))
		{
			strcpy (local_rec.br_no, comm_rec.est_no);
			return (EXIT_SUCCESS);
		}
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

		if (dflt_used)
			strcpy (local_rec.br_no, comm_rec.est_no);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
	
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Branch not found.
			 */
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			abc_fclose (esmr);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.br_name, esmr_rec.est_name);
		if (nd_flag [2] == 'Y')
			print_at (5, 11, "%s %-40.40s", local_rec.br_no, local_rec.br_name);
		abc_fclose (esmr);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("wh_no"))
	{
		if (F_NOKEY (field))
		{
			strcpy (local_rec.wh_no, comm_rec.cc_no);
			local_rec.hhccHash = ccmr_rec.hhcc_hash;
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.wh_no, comm_rec.cc_no);

		if (warehouseSelected)
		{
			abc_selfield (ccmr, "ccmr_id_no");
			warehouseSelected = FALSE;
		}
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.br_no);
		strcpy (ccmr_rec.cc_no, local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Warehouse not found.
			 */
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.wh_name, ccmr_rec.name);
		if (nd_flag [2] == 'Y')
			print_at (5, 68, "%s %-40.40s", local_rec.wh_no, local_rec.wh_name);
		local_rec.hhccHash = ccmr_rec.hhcc_hash;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Creditor Number.
	 */
	if (LCHECK ("supplierNo"))
	{
		if (env.PoInput && dflt_used)
		{
			sprintf (local_rec.supplierNo, "%-6.6s", " ");
			DSP_FLD ("supplierNo");
			return (EXIT_SUCCESS);
		} 

		abc_selfield (sumr, (env.CrFind) ? "sumr_id_no3" : "sumr_id_no");

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.supplierNo, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Supplier not found.
			 */
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		/*
		 * Find currency code file.
		 */
		if (FindPocr (sumr_rec.curr_code))
			return (EXIT_FAILURE);

		/*
		 * Find freight file.
		 */
		if (FindPocf (sumr_rec.ctry_code))
			return (EXIT_FAILURE);

		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Purchase Order Number.
	 */
	if (LCHECK ("po_credit_no"))
	{
		if (SRCH_KEY)
		{
			if (strcmp (local_rec.supplierNo, "      "))
				SrchPohr (temp_str);
			else
				SrchPohr2 (temp_str);
			return (EXIT_SUCCESS);
		}

		if ((dflt_used || !strcmp (pohr_rec.pur_ord_no, fifteenSpaces)) &&
			strcmp (local_rec.supplierNo, "      "))
		{
			if (!env.PoInput)
			{
				/*
				 * PRNo must be input. 
				 */
				print_mess (ML (mlPoMess009));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
				strcpy (pohr_rec.pur_ord_no, "SYSTEM GEN.    ");
			
			DSP_FLD ("pur_ord_no");
			newPoCredit = TRUE;
			return (EXIT_SUCCESS);
		}
		/*
		 * Check if order is on file.
		 * if defaulted over crd_no then we need to use diff index 
		 */
		if (!strcmp (local_rec.supplierNo, "      "))
		{
			abc_selfield (pohr, "pohr_id_no2");
			strcpy (sumr_rec.co_no, comm_rec.co_no);
		}
		else
			abc_selfield (pohr, "pohr_id_no");

		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type, "C");
		sprintf (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));

		cc = find_rec (pohr, &pohr_rec, COMPARISON, "w");
		if (cc == -1)
		{
			restart = FALSE;
			heading (1);
			return (EXIT_FAILURE);
		}

		/*
		 * Order already on file.
		 */
		if (!cc) 
		{
			if (pohr_rec.status [0] == 'D')
			{
				/*
				 * Return %s has been Closed.
				 */
				sprintf (err_str, ML (mlPoMess003), pohr_rec.pur_ord_no);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			local_rec.exch_rate = pohr_rec.curr_rate;
			cc = LoadPoln (pohr_rec.hhpo_hash);
			if (cc) 
			{
				restart = 1;
				return (EXIT_SUCCESS);
			}
			entry_exit = 1;
			newPoCredit = FALSE;

			switch (pohr_rec.ship_method [0])
			{
				case 'L' :	strcpy (local_rec.ship_desc, ML ("Land"));
							break;
				case 'S' :	strcpy (local_rec.ship_desc, ML ("Sea "));
							break;
				case 'A' :	strcpy (local_rec.ship_desc, ML ("Air "));
							break;
				case 'R' :	strcpy (local_rec.ship_desc, ML ("Rail"));
							break;
			}
			DSP_FLD ("ship_desc");

			if (env.PoInput && !strcmp (local_rec.supplierNo, "      "))
				return (LoadSupplier (field));
			return (EXIT_SUCCESS);

		}
		if (cc && !strcmp (local_rec.supplierNo, "      "))
		{
			/*
			 * Purchase Return not found.
			 */
			print_mess (ML (mlPoMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (env.PoInput)
			strcpy (pohr_rec.pur_ord_no, "SYSTEM GEN.    ");

		DSP_FLD ("pur_ord_no");

		pohr_rec.hhpo_hash = 0L;
		abc_unlock (pohr);
		newPoCredit = TRUE;
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Purchase Order Number.
	 */
	if (LCHECK ("pur_ord_no"))
	{
		if (SRCH_KEY)
		{
			SrchPohr3 (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			i = prmptmsg (ML (mlPoMess231), "YyNn", 1, 23);
			move (1, 23);
			cl_line ();
			if (i != 'N' && i != 'n')
			{
				APPLIED	=	FALSE;
				return (EXIT_SUCCESS);
			}
			return (EXIT_FAILURE);
		}

		/*
		 * Check if order is on file.
		 */
		abc_selfield (pohr3, "pohr_id_no");

		strcpy (pohr3_rec.co_no, sumr_rec.co_no);
		strcpy (pohr3_rec.br_no, comm_rec.est_no);
		pohr3_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr3_rec.type, "O");
		sprintf (pohr3_rec.pur_ord_no, zero_pad (local_rec.pur_ord_no, 15));

		cc = find_rec (pohr3, &pohr3_rec, COMPARISON, "w");
		if (cc)
		{
			/*
			 * Purchase Return not found.
			 */
			print_mess (ML (mlPoMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Order already on file.
		 */
		local_rec.exch_rate = pohr3_rec.curr_rate;
		cc = LoadPurchaseOrder (pohr3_rec.hhpo_hash);
		if (cc) 
			return (EXIT_FAILURE);
		
		switch (pohr3_rec.ship_method [0])
		{
			case 'L' :	strcpy (local_rec.ship_desc, "Land");
						break;
			case 'S' :	strcpy (local_rec.ship_desc, "Sea ");
						break;
			case 'A' :	strcpy (local_rec.ship_desc, "Air ");
						break;
		}
		DSP_FLD ("ship_desc");

		DSP_FLD ("pur_ord_no");

		abc_unlock (pohr3);
		APPLIED	=	TRUE;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Due Date Override.
	 */
	if (LCHECK ("date_reqd"))
	{
		if (dflt_used)
			pohr_rec.due_date = 0L;
	
		if (pohr_rec.due_date != 0L)
		{
			if (prog_status != ENTRY)
			{
				/*
				 * Set due date at line level to PoOverride date.
				 */
				scn_set (PO_LINES);
				for (i = 0; i < lcount [PO_LINES]; i++)
				{
					getval (i);
					if (store [i].ship_no == 0L)
						local_rec.due_date = pohr_rec.due_date;
					putval (i);
				}
	
				scn_set (PO_HEAD);
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Due Date Override.
	 */
	if (LCHECK ("date_reqd"))
	{
		if (dflt_used)
			pohr_rec.due_date = 0L;
	
		if (pohr_rec.due_date != 0L)
		{
			if (prog_status != ENTRY)
			{
				/*
				 * Set due date at line level to override date.
				 */
				scn_set (PO_LINES);
				for (i = 0; i < lcount [PO_LINES]; i++)
				{
					getval (i);
					if (store [i].ship_no == 0L)
						local_rec.due_date = pohr_rec.due_date;
					putval (i);
				}
	
				scn_set (PO_HEAD);
			}
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Default Exchange Rate.
	 */
	if (LCHECK ("exch_rate"))
	{
		if (dflt_used)
		{
			local_rec.exch_rate = pocr_rec.ex1_factor;
			DSP_FLD ("exch_rate");
		}

		if (local_rec.exch_rate == 0.00)
		{
			/*
			 * Exchange Rate must be > 0.00
			 */
			print_mess (ML (mlStdMess044)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			scn_set (PO_LINES);
			for (line_no = 0;line_no < lcount [PO_LINES];line_no++) 
			{
				getval (line_no);
				if (store [line_no].ship_no == 0L)
					store [line_no].exch_rate = local_rec.exch_rate;
				if (store [line_no].duty_type [0] != 'D')
				{
					local_rec.duty_val = DutyCalculate (line_no);
					store [line_no].val_duty = local_rec.duty_val;
				}
				
				CalculateCost (line_no);
				putval (line_no);
			}
			scn_set (PO_HEAD);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate supplier payment terms.
	 */
	if (LCHECK ("stpay"))
	{
		if (SRCH_KEY)
		{
			ShowPay ();
			return (EXIT_SUCCESS);
		}
		for (i = 0; strlen (p_terms [i]._pcode); i++)
		{
			if (!strncmp (sumr_rec.pay_terms, p_terms [i]._pcode, 
					     strlen (p_terms [i]._pcode)))
			{
				sprintf (pohr_rec.sup_term_pay,"%-30.30s", p_terms [i]._pterm);
				break;
			}
		}
		DSP_FLD ("stpay");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Special instructions. 
	 */
	if (LCHECK ("sin1") || LCHECK ("sin2") || LCHECK ("sin3"))
	{
		i = field - label ("sin1");

		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			abc_fclose (exsi);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no, comm_rec.co_no);

		if (i == 0)
			exsi_rec.inst_code = atoi (pohr_rec.stdin1);
		else if (i == 1)
			exsi_rec.inst_code = atoi (pohr_rec.stdin2);
		else if (i == 2)
			exsi_rec.inst_code = atoi (pohr_rec.stdin3);

		if (dflt_used  && exsi_rec.inst_code == 0)
			exsi_rec.inst_code = comr_po_sic [i];
		
		/*
		 * Changed to not overwrite changed instructions
		 */
		if (newPoCredit && prog_status == ENTRY && exsi_rec.inst_code)
		{
			if (!find_rec (exsi, &exsi_rec, COMPARISON, "r"))
			{
				if (i == 0)
					sprintf (pohr_rec.stdin1, "%-60.60s", exsi_rec.inst_text);
				else if (i == 1)
					sprintf (pohr_rec.stdin2, "%-60.60s", exsi_rec.inst_text);
				else if (i == 2)
					sprintf (pohr_rec.stdin3, "%-60.60s", exsi_rec.inst_text);
			}
		}

		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Delivery Instructions
	 */
	if (LCHECK ("del1") || LCHECK ("del2") || LCHECK ("del3"))
	{
		i = field - label ("del1");

		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);

			abc_fclose (exsi);

			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no, comm_rec.co_no);
		if (i == 0)
			exsi_rec.inst_code = atoi (pohr_rec.delin1);
		else if (i == 1)
			exsi_rec.inst_code = atoi (pohr_rec.delin2);
		else if (i == 2)
			exsi_rec.inst_code = atoi (pohr_rec.delin3);

		if (dflt_used && exsi_rec.inst_code == 0)
			exsi_rec.inst_code = sumr_sic [i];

		/*
		 * Changed to not overwrite changed instructions 
		 */
		if (newPoCredit && prog_status == ENTRY)
		{
			if (!find_rec (exsi, &exsi_rec, COMPARISON, "r"))
			{
				if (i == 0)
					sprintf (pohr_rec.delin1, "%-60.60s", exsi_rec.inst_text);
				else if (i == 1)
					sprintf (pohr_rec.delin2, "%-60.60s", exsi_rec.inst_text);
				else if (i == 2)
					sprintf (pohr_rec.delin3, "%-60.60s", exsi_rec.inst_text);
			}
		}
		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
	{
		if (last_char == EOI)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

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
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			if (env.PoIndentOk)
			{
				cc =	check_indent 
						(
							comm_rec.co_no, 
							comm_rec.est_no, 
							local_rec.hhccHash, 
							local_rec.item_no
						);
				cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.co_no, comm_rec.co_no);
					strcpy (inmr_rec.item_no, local_rec.item_no);
					cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				}
				if (!cc)
				{
					pr_other ();
					line_at (4, 0, 132);
				}
			}
			if (!cc)
			{
				print_at (2, 3, ML (mlPoMess105), 	pohr_rec.pur_ord_no, 
													sumr_rec.crd_no, 
													sumr_rec.crd_name);
			}
			if (cc)
			{
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

		}

		/*
		 * Discontinued Product ?
		 */
		if (inmr_rec.active_status [0] == 'D')
		{
			/*
			 * Item has been Discontinued. 
			 */
			print_mess (ML (mlPoMess122));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhbrHash = inmr_rec.hhbr_hash;
		strcpy (SR.itemClass, inmr_rec.inmr_class); 
		if (check_class (inmr_rec.inmr_class))
		{
			InputPONS (line_cnt); 
			pr_other ();
			line_at (4, 0, 132);
		}

		SuperSynonymError ();
		
		if (!check_class (SR.itemClass))
			strcpy (SR.item_desc, inmr_rec.description);

		strcpy (SR.costingFlag, inmr_rec.costing_flag);
		DSP_FLD ("item_no");
		if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.00;

		SR.outer = (double) inmr_rec.outer_size;

		/*
		 * Find part number for warehouse record.
		 */
		incc_rec.hhcc_hash = local_rec.hhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");

		if (cc) 
		{
			/*
			 * Item not found.
			 */
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SR.hhwhHash = incc_rec.hhwh_hash;
		SR.hhccHash = incc_rec.hhcc_hash;
		SR.closingStock = incc_rec.closing_stock;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Item has no UOM
			 */
			print_mess (ML (mlPoMess123)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (SR.std_uom, inum_rec.uom);
		strcpy (SR.sup_uom, inum_rec.uom);
		strcpy (local_rec.std_uom, inum_rec.uom);
		strcpy (local_rec.sup_uom, inum_rec.uom);
		strcpy (SR.uom_group, inum_rec.uom_group);
		SR.StdCnvFct 		= inum_rec.cnv_fct;
		SR.pur_conv  		= 1.00;
		local_rec.pur_conv 	= 1.00;

		if (FindInis (inmr_rec.hhbr_hash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			strcpy (local_rec.duty_code, inmr_rec.duty);
			strcpy (local_rec.lic_cat, inmr_rec.licence);

			SR.no_inis 			= TRUE;
			SR.weight 			= 0.00;
			SR.volume 			= 0.00;
			SR.lead_time 		= 0.00;
			SR.sea_time 		= 0.00;
			SR.air_time 		= 0.00;
			SR.lnd_time 		= 0.00;
			SR.min_order 		= 0;
			SR.ord_multiple 	= 0;
			SR.base_cost 		= 0.00;
		}
		else
		{
			SR.no_inis 	= FALSE;
			SR.weight 	= inis_rec.weight;
			SR.volume 	= inis_rec.volume;
			SR.sea_time = inis_rec.sea_time;
			SR.air_time = inis_rec.air_time;
			SR.lnd_time = inis_rec.lnd_time;
			switch (pohr_rec.ship_method [0])
			{
			case 'S':
				SR.lead_time = inis_rec.sea_time;
				break;

			case 'A':
				SR.lead_time = inis_rec.air_time;
				break;

			case 'L':
				SR.lead_time = inis_rec.lnd_time;
				break;
			}
			SR.min_order 	= ToStdUom (inis_rec.min_order, SR.pur_conv);
			SR.ord_multiple = ToStdUom (inis_rec.ord_multiple, SR.pur_conv);

			strcpy (local_rec.duty_code, inis_rec.duty);
			strcpy (local_rec.lic_cat, inis_rec.licence);
			strcpy (inmr_rec.duty, inis_rec.duty);
			strcpy (inmr_rec.licence, inis_rec.licence);
			SR.base_cost = DPP (inis_rec.fob_cost);
			inum_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (SR.sup_uom, inum_rec.uom);
				strcpy (local_rec.sup_uom, inum_rec.uom);
				local_rec.pur_conv 	= 1.00;
				SR.pur_conv 		= 1.00;
			}
			else
			{
				strcpy (SR.sup_uom, inum_rec.uom);
				strcpy (local_rec.sup_uom, inum_rec.uom);
				local_rec.pur_conv = SR.StdCnvFct / inum_rec.cnv_fct;

				SR.pur_conv = local_rec.pur_conv;
			}
		}
		if (SR.weight == 0.00)
			SR.weight = inmr_rec.weight;

		DSP_FLD ("UOM");

		if (SR.no_inis && INIS_REQ && !check_class (SR.itemClass))
		{
			/*
			 * No inventory supplier record exists.
			 */
			print_mess (ML (mlStdMess155));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Find part number for branch record.
		 */
		ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (ineiRec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &ineiRec, COMPARISON, "r");
		if (cc) 
		{
			/*
			 * Item not found. 
			 */
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Find duty record.
		 */
		if (!strcmp (local_rec.duty_code, "  "))
		{
			SR.imp_duty = 0.00;
			SR.val_duty = 0.00;
			strcpy (SR.duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code, local_rec.duty_code);
			cc = find_rec (podt, &podt_rec, COMPARISON, "r");
			if (cc)
			{
				/*
				 * Duty Code not found.
				 */
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			SR.imp_duty = podt_rec.im_duty;
			strcpy (SR.duty_type, podt_rec.duty_type);
		}

		/*
		 * Find and Validate Licence.
		 */
		if (!strcmp (local_rec.lic_cat, "  "))
		{
			if (nd_flag [0] == 'Y' || nd_flag [0] == 'y')
				FLD ("lic_no") = NA;

			strcpy (SR.lic_cat, "  ");
			strcpy (SR.lic_no, "          ");
			SR.lic_hash = 0L;
			SR.lic_rate = 0.00;
			strcpy (local_rec.lic_no, "          ");
			local_rec.lic_val = 0.00;

			if (nd_flag [0] == 'Y' || nd_flag [0] == 'y')
				DSP_FLD ("lic_no");
		}
		else
		{
			if (nd_flag [0] == 'Y' || nd_flag [0] == 'y')
				FLD ("lic_no") = YES;

			strcpy (SR.lic_cat, inmr_rec.licence);
		}

		if (prog_status == ENTRY)
		{
			SR.exch_rate = local_rec.exch_rate;
			SR.ship_no = 0L;
		}

		SR.upd_inis = FALSE;
		if (SR.base_cost != 0.00)
		{
			SR.cst_price 		= SR.base_cost;
			local_rec.fob_cost	= SR.base_cost;
		}
		else
		{
			SR.cst_price = ineiRec.last_cost * SR.exch_rate;
			SR.cst_price /= SR.outer;
			SR.base_cost = SR.cst_price;
		}

		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			line_display ();
		}
		/*
		 * Set due date.
		 */
		if (pohr_rec.due_date != 0L)
			local_rec.due_date = pohr_rec.due_date;

		tab_other (line_cnt);
	}

	/*
	 * Validate Unit of Measure
	 */
	if (LCHECK ("UOM"))
	{
		if (F_NOKEY (label ("UOM")) || dflt_used)
			strcpy (local_rec.sup_uom, SR.std_uom);

		if (SRCH_KEY)
		{
			SrchInum (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR.uom_group);
		strcpy (inum2_rec.uom, local_rec.sup_uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Invalid UOM
			 */
			print_mess (ML (mlStdMess028)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			/*
			 * Invalid UOM
			 */
			print_mess (ML (mlStdMess028)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.sup_uom, inum2_rec.uom);
		SR.hhumHash = inum2_rec.hhum_hash;
		strcpy (SR.sup_uom, inum2_rec.uom);
        if (inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR.pur_conv = SR.StdCnvFct / inum2_rec.cnv_fct;
		strcpy (SR.sup_uom, inum2_rec.uom);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Quantity input.
	 */
	if (LCHECK ("ret_qty"))
	{
		if (dflt_used)
		{
			local_rec.ret_qty 	= local_rec.rec_qty;
			local_rec.fob_cost	= 0.00;
		}
		if (SR.costingFlag [0] == 'F' || SR.costingFlag [0] == 'L')
		{
			workFifo	=	FindIncfCostPO
							(
								SR.hhwhHash, 
								SR.closingStock, 
								ToStdUom (local_rec.ret_qty, SR.pur_conv), 
								(env.PoReturnApply) ? TRUE : FALSE, 
								inmr_rec.dec_pt
							);
			workFifo	=	twodec (workFifo);
			if (workFifo != -1.00)
			{
				SR.cst_price 	=	workFifo;
				SR.cst_price	*=	SR.exch_rate;
				SR.cst_price	/=	SR.outer;
			}
			SR.base_cost 		= SR.cst_price;
			local_rec.fob_cost	= SR.cst_price;
		}
		else
			local_rec.fob_cost	= SR.cst_price;
		
		SR.cst_price =	GetSupPrice 
						(
							sumr_rec.hhsu_hash, 
							SR.hhbrHash, 
							SR.base_cost, 
							ToStdUom (local_rec.ret_qty, SR.pur_conv)
						);

		SR.cumulative =	GetSupDisc 
						(
							sumr_rec.hhsu_hash, 
							inmr_rec.buygrp, 
							ToStdUom (local_rec.ret_qty, SR.pur_conv), 
							SR.discArray
						);

		if (local_rec.ret_qty > 100.00 && inmr_rec.serial_item [0] == 'Y')
		{
			/*
			 * Serial items are limited to 100 per line.
			 */
			print_mess (ML (mlPoMess124));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.quantity = ToStdUom (local_rec.ret_qty, SR.pur_conv);
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			line_display ();
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Licence Number. 
	 */
	if (LCHECK ("lic_no"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY)
			strcpy (inmr_rec.licence, SR.lic_cat);

		if (!strcmp (inmr_rec.licence, "  "))
		{
			strcpy (SR.lic_cat, "  ");
			strcpy (SR.lic_no, "          ");
			SR.lic_hash = 0L;
			SR.lic_rate = 0.00;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPolh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);
		strcpy (polh_rec.lic_cate, SR.lic_cat);
		strcpy (polh_rec.lic_no, local_rec.lic_no);
		cc = find_rec (polh, &polh_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Licence Number Not found.
			 */
			print_mess (ML (mlStdMess154));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (SR.lic_cat, polh_rec.lic_cate);
		strcpy (SR.lic_no, polh_rec.lic_no);
		SR.lic_hash = polh_rec.hhlc_hash;
		SR.lic_rate = polh_rec.ap_lic_rate;
		strcpy (local_rec.lic_no, polh_rec.lic_no);
		local_rec.lic_val = polh_rec.ap_lic_rate;
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			line_display ();
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate FOB (FGN).
	 */
	if (LCHECK ("fob_cst"))
	{
		workFob =  SR.cst_price;
		workFob /= DPP (SR.pur_conv);

		if (dflt_used || F_NOKEY (field))
			local_rec.grs_fgn = workFob;

		SR.grs_fgn = local_rec.grs_fgn;
		if (local_rec.grs_fgn == 0.00)
		{
			i = prmptmsg (ML (mlStdMess121), "YyNn", 1, 23);
			move (1, 23);
			cl_line ();
			if (i != 'Y' && i != 'y')
			{
				FLD ("fob_cst")  =   YES;
				skip_entry = goto_field (field, label ("fob_cst"));
				return (EXIT_FAILURE);
			}
		}
		if (SR.no_inis == TRUE && !check_class (SR.itemClass))
		{
			move (0, 23);
			cl_line ();
			/*
			 * NOTE : Item not found.
			 */
			rv_pr (ML (mlPoMess230), 0, 32, 1);
			sleep (sleepTime);
			SR.upd_inis = 0;
			if (prog_status == ENTRY)
				local_rec.due_date = pohr_rec.due_date;
		}
		else
		{
			move (0, 23);
			cl_line ();
		}

		if (workFob != local_rec.grs_fgn && !check_class (SR.itemClass))
		{
			move (0, 23);
			cl_line ();
			if (SR.no_inis == FALSE)
			{
				/*
				 * Prompt
				 */
				if (updateSupplierRecord == -1)
				{
					SR.upd_inis = prmpt_inis (0, 23);
				}
				else
					SR.upd_inis = updateSupplierRecord;
			}
		}
		if (prog_status != ENTRY)
		{
			if (SR.duty_type [0] != 'D')
			{
				local_rec.duty_val = DutyCalculate (line_cnt);
				SR.val_duty = local_rec.duty_val;
			}

			if (pocf_rec.load_type [0] != 'D')
			{
				local_rec.loc_fi = FreightCalculate (line_cnt);
				SR.val_fi = local_rec.loc_fi;
			}
			local_rec.net_fob =	CalcNet 
								(
									local_rec.grs_fgn, 
									SR.discArray, 
									SR.cumulative
								);
			SR.net_fob = local_rec.net_fob;
			CalculateCost (line_cnt);
			line_display ();
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate View Discount.
	 */
	if (LCHECK ("view_disc"))
	{
		if (local_rec.view_disc [0] == 'Y')
		{
			int	tmpLcount;

			tmpLcount = lcount [PO_LINES];

			workFob = SR.cst_price;
			workFob /= DPP (SR.pur_conv);

#ifdef GVISION
			discRec.grossPrice		= local_rec.grs_fgn;
			discRec.discArray [0]	= SR.discArray [0];
			discRec.discArray [1]	= SR.discArray [1];
			discRec.discArray [2]	= SR.discArray [2];
			discRec.discArray [3]	= SR.discArray [3];

			ViewDiscounts (DBOX_LFT, DBOX_TOP, SR.cumulative);

			local_rec.grs_fgn	= discRec.grossPrice;
			SR.discArray [0]	= discRec.discArray [0];
			SR.discArray [1]	= discRec.discArray [1];
			SR.discArray [2]	= discRec.discArray [2];
			SR.discArray [3]	= discRec.discArray [3];
#else
			ViewDiscounts ();
#endif	/* GVISION */

			/*
			 * Redraw screens.
			 */
			putval (line_cnt);
			scn_write (PO_LINES);

			lcount [PO_LINES] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [PO_LINES];
			scn_display (PO_LINES);
			lcount [PO_LINES] = tmpLcount;
		}
		else
		{
			local_rec.net_fob =	CalcNet 
								(
									local_rec.grs_fgn, 
									SR.discArray, 
									SR.cumulative
								);
			SR.net_fob = local_rec.net_fob;
		}
		DSP_FLD ("net_fob");
		if (prog_status != ENTRY)
		{
			CalculateCost (line_cnt);
			line_display ();
		}
	}

	/*
	 * Validate FOB (LOC).
	 */
	if (LCHECK ("loc_fi"))
	{
		if (dflt_used || F_NOKEY (field))
			local_rec.loc_fi = FreightCalculate (line_cnt);
		
		SR.val_fi = local_rec.loc_fi;
		CalculateCost (line_cnt);
		line_display ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Duty     
	 */
	if (LCHECK ("duty_val"))
	{
		if (dflt_used || F_NOKEY (field))
			local_rec.duty_val = DutyCalculate (line_cnt);
		
		SR.val_duty = local_rec.duty_val;
		CalculateCost (line_cnt);
		line_display ();

		return (EXIT_SUCCESS);
	}
	/*
	 * Validate due date at line item level.
	 */
	if (LCHECK ("due_date"))
	{
		if (F_NOKEY (field) && pohr_rec.due_date != 0L)
			return (EXIT_SUCCESS);

		if (dflt_used || F_NOKEY (field))
		{
			if (pohr_rec.due_date == 0L)
			{
				local_rec.due_date = currentDate;
				local_rec.due_date += (long) SR.lead_time;
			}
			else
				local_rec.due_date = pohr_rec.due_date;

			DSP_FLD ("due_date");
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Shipment Method.
	 */
	if (LCHECK ("ship_method"))
	{
		if (dflt_used)
			strcpy (pohr_rec.ship_method, sumr_rec.ship_method);

		switch (pohr_rec.ship_method [0])
		{
			case 'L' :	strcpy (local_rec.ship_desc, "Land");
						break;
			case 'S' :	strcpy (local_rec.ship_desc, "Sea ");
						break;
			case 'A' :	strcpy (local_rec.ship_desc, "Air ");
						break;
			default	 :  /*
						 * Invalid Shipment Method
						 */
						print_mess (ML (mlStdMess119));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
		}
		DSP_FLD ("ship_desc");

		/*
		 * Change due dates based on new shipment method.
		 */
		if (prog_status != ENTRY && pohr_rec.due_date == 0L)
		{
			scn_set (PO_LINES);
			for (i = 0; i < lcount [PO_LINES]; i++)
			{
				getval (i);

				switch (pohr_rec.ship_method [0])
				{
				case 'S':
					store [i].lead_time = store [i].sea_time;
					break;

				case 'A':
					store [i].lead_time = store [i].air_time;
					break;

				case 'L':
					store [i].lead_time = store [i].lnd_time;
					break;

				}
				if (store [i].ship_no == 0L)
				{
					local_rec.due_date = currentDate;
					local_rec.due_date += (long) store [i].lead_time;
				}
				putval (i);
			}
			scn_set (PO_HEAD);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Find INventory SUpplier file (inis);
 */
int
FindInis (
	long	hhbrHash, 
	long	hhsuHash)
{
	inis_rec.hhbr_hash = hhbrHash;
	inis_rec.hhsu_hash = hhsuHash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	return (cc);
}

/*
 * Find Currency file.
 */
int
FindPocr (
	char	*code)
{
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
	{
		/*
		 * Currency code not found.
		 */
		print_mess (ML (mlStdMess040));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

/*
 * Find Country freight file.
 */
int
FindPocf (
	char	*code)
{
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	strcpy (pocf_rec.co_no, comm_rec.co_no);
	sprintf (pocf_rec.code, "%-3.3s", code);
	cc = find_rec (pocf, &pocf_rec, COMPARISON, "r");
	if (cc)
	{
		/*
		 * Country Code not found.
		 */
		print_mess (ML (mlStdMess118));
		abc_fclose (pocf);
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	abc_fclose (pocf);
	return (EXIT_SUCCESS);
}

/*
 * Calculate Freight.
 */
double	
FreightCalculate (
 int		line_cnt)
{
	double	friValue 	= 0.00;
	double	frtConvert 	= 0.00;

	/*
	 * Calculate Freight
	 */
	frtConvert = pocf_rec.freight_load;

	/*
	 * Freight is a Unit value.
	 */
	if (pocf_rec.load_type [0] == 'U')
		friValue = frtConvert;

	/*
	 * Freight is a Percentage.
	 */
	if (pocf_rec.load_type [0] == 'P')
	{
		friValue = SR.net_fob;
		friValue *= SR.outer;
		friValue *= DPP (SR.pur_conv);
		friValue *= frtConvert;
		friValue /= 100;
	}

	if (SR.exch_rate != 0.00)
		friValue /= SR.exch_rate;

	return (friValue);
}

/*
 * Calculate Duty on total quantity and each basis.
 */
double	
DutyCalculate (
	int		line_cnt)
{
	double	dutyValue  = 0.00;

	/*
	 * Calculate Duty   
	 */
	if (SR.duty_type [0] == 'D')
		dutyValue = SR.imp_duty;
	else
	{
		dutyValue	=	SR.net_fob;
		dutyValue	*=	SR.outer;
		dutyValue	*=	DPP (SR.pur_conv);
		dutyValue	*=	SR.imp_duty / 100;
		if (SR.exch_rate != 0.00)
			dutyValue /= SR.exch_rate;
	}
	return (dutyValue);
}

/*
 * Calculate total line cost.
 */
void
CalculateCost (
	int		wk_line)
{
	double	cifCost = 0.00;

	/*
	 * Calculate FOB FGN
	 */
	if (prog_status != ENTRY)
	{
		local_rec.net_fob =	CalcNet 
							(
								local_rec.grs_fgn, 
								SR.discArray, 
								SR.cumulative
							);
		store [wk_line].net_fob	= local_rec.net_fob;
	}

	/*
	 * Calculate CIF FGN
	 */
	cifCost = 	local_rec.net_fob; 
	cifCost *= 	store [wk_line].outer;
	cifCost *= 	DPP (store [wk_line].pur_conv);

	/*
	 * Calculate CIF LOC
	 */
	if (store [wk_line].exch_rate != 0.00)
		local_rec.cif_loc = cifCost / store [wk_line].exch_rate;
	else
		local_rec.cif_loc = 0.00;

	local_rec.cif_loc = local_rec.cif_loc;
	local_rec.cif_loc += local_rec.loc_fi;
	local_rec.cif_loc = local_rec.cif_loc;

	/*
	 * Calculate Licence LOC
	 */
	local_rec.lic_val  = (double) store [wk_line].lic_rate;
	local_rec.lic_val *= local_rec.cif_loc;
	local_rec.lic_val = local_rec.lic_val;

	/*
	 * Calculate Landed Cost local
	 */
	local_rec.land_cst = local_rec.cif_loc + 
			     		 local_rec.duty_val + 
			     		 local_rec.lic_val + 
			     		 local_rec.other;

	local_rec.Dsp_land_cst 		= local_rec.land_cst;
	store [wk_line].land_cost 	= local_rec.land_cst;
}

/*
 * Routine to read all poln records whose hash matches the one on the   
 * pohr record. Stores all non screen relevant details in another       
 * structure. Also gets part number for the part hash. And g/l account  
 * number.                                                              
 */
int
LoadPoln (
	long	hhpoHash)
{
	int		i;
	/*
	 * Set screen for putval.
	 */
	scn_set (PO_LINES);
	lcount [PO_LINES] = 0;
	
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (polh, "polh_hhlc_hash");
	abc_selfield (poln, "poln_id_no");

	poln_rec.hhpo_hash 	= hhpoHash;
	poln_rec.line_no 	= 0; 

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		/*
		 * Get part number.
		 */
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (strcmp (inmr_rec.supercession, "                "))
		{
			abc_selfield (inmr, "inmr_id_no");
			FindSupercession (comm_rec.co_no, inmr_rec.supercession, TRUE);
			abc_selfield (inmr, "inmr_hhbr_hash");
		}
		store [lcount [PO_LINES]].hhbrHash = inmr_rec.hhbr_hash;
		strcpy (local_rec.item_no, inmr_rec.item_no);

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		strcpy (store [lcount [PO_LINES]].std_uom, inum_rec.uom);
		strcpy (local_rec.std_uom, inum_rec.uom);
		strcpy (store [lcount [PO_LINES]].uom_group, inum_rec.uom_group);
		store [lcount [PO_LINES]].StdCnvFct 		= inum_rec.cnv_fct;
		store [lcount [PO_LINES]].hhumHash = inum_rec.hhum_hash;

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (store [lcount [PO_LINES]].std_uom, inum_rec.uom);
			strcpy (local_rec.std_uom, inum_rec.uom);
			store [lcount [PO_LINES]].pur_conv = 1.00;
		}
		else
		{
			if (inum_rec.cnv_fct == 0.00)
				inum_rec.cnv_fct = 1.00;

			strcpy (store [lcount [PO_LINES]].sup_uom, inum_rec.uom);
			strcpy (local_rec.sup_uom, inum_rec.uom);
			store [lcount [PO_LINES]].pur_conv	=	
					   	store [lcount [PO_LINES]].StdCnvFct / inum_rec.cnv_fct;
			local_rec.pur_conv 	= 
			   			store [lcount [PO_LINES]].StdCnvFct / inum_rec.cnv_fct;

			store [lcount [PO_LINES]].hhumHash = inum_rec.hhum_hash;
		}
		GetWarehouse (poln_rec.hhcc_hash);

		add_hash 
		(
			comm_rec.co_no, 
			local_rec.br_no, 
			"RC", 
			0, 
			poln_rec.hhbr_hash, 
			poln_rec.hhcc_hash, 
			0L, 
			0.00
		);

		store [lcount [PO_LINES]].outer = (double) inmr_rec.outer_size;
		strcpy (store [lcount [PO_LINES]].itemClass, inmr_rec.inmr_class);

		if (store [lcount [PO_LINES]].outer == 0.0)
			store [lcount [PO_LINES]].outer = 1.0;

		strcpy (store [lcount [PO_LINES]].costingFlag, inmr_rec.costing_flag);
		strcpy (store [lcount [PO_LINES]].item_desc, inmr_rec.description);

		if (FindInis (inmr_rec.hhbr_hash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			store [lcount [PO_LINES]].no_inis 	= TRUE;
			store [lcount [PO_LINES]].lead_time 	= 0.00;
			store [lcount [PO_LINES]].sea_time 		= 0.00;
			store [lcount [PO_LINES]].air_time 		= 0.00;
			store [lcount [PO_LINES]].lnd_time 		= 0.00;
			store [lcount [PO_LINES]].weight 		= 0.00;
			store [lcount [PO_LINES]].volume 		= 0.00;

			strcpy (local_rec.duty_code, inmr_rec.duty);
			strcpy (local_rec.lic_cat, inmr_rec.licence);
			store [lcount [PO_LINES]].cst_price = local_rec.fob_cost = 0.00;

			store [lcount [PO_LINES]].min_order = 0;
			store [lcount [PO_LINES]].ord_multiple = 0;

			/*
			 * Find part number for branch record.
			 */
			ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (ineiRec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &ineiRec, COMPARISON, "r");
			if (cc) 
				file_err (cc, inei, DBFIND);

			store [lcount [PO_LINES]].cst_price = ineiRec.last_cost;
			store [lcount [PO_LINES]].cst_price *= pohr_rec.curr_rate;
			store [lcount [PO_LINES]].cst_price /= store [lcount [PO_LINES]].outer;
		}
		else
		{
			store [lcount [PO_LINES]].no_inis = FALSE;
			store [lcount [PO_LINES]].weight = inis_rec.weight;
			store [lcount [PO_LINES]].volume = inis_rec.volume;

			store [lcount [PO_LINES]].sea_time = inis_rec.sea_time;
			store [lcount [PO_LINES]].air_time = inis_rec.air_time;
			store [lcount [PO_LINES]].lnd_time = inis_rec.lnd_time;
			switch (pohr_rec.ship_method [0])
			{
			case 'S':
				SR.lead_time = inis_rec.sea_time;
				break;

			case 'A':
				SR.lead_time = inis_rec.air_time;
				break;

			case 'L':
				SR.lead_time = inis_rec.lnd_time;
				break;
			}
			store [lcount [PO_LINES]].min_order = 
										ToStdUom 	
										(
											inis_rec.min_order, 
											store [lcount [PO_LINES]].pur_conv
										);
			store [lcount [PO_LINES]].ord_multiple = 
										ToStdUom 
										(
											inis_rec.ord_multiple, 
											store [lcount [PO_LINES]].pur_conv
										);

			strcpy (local_rec.duty_code, inis_rec.duty);
			strcpy (local_rec.lic_cat, inis_rec.licence);

			strcpy (inmr_rec.duty, local_rec.duty_code);
			strcpy (inmr_rec.licence, local_rec.lic_cat);
			local_rec.fob_cost = DPP (inis_rec.fob_cost);
			store [lcount [PO_LINES]].cst_price = DPP (inis_rec.fob_cost);

		}
		if (!strcmp (local_rec.duty_code, "  "))
		{
			store [lcount [PO_LINES]].imp_duty = 0.00;
			store [lcount [PO_LINES]].val_duty = 0.00;
			strcpy (store [lcount [PO_LINES]].duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code, local_rec.duty_code);
			cc = find_rec (podt, &podt_rec, COMPARISON, "r");
			if (cc)
			{
				/*
				 * Duty Code not found.
				 */
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			store [lcount [PO_LINES]].imp_duty = podt_rec.im_duty;
			strcpy (store [lcount [PO_LINES]].duty_type, podt_rec.duty_type);
		}
		/*
		 * Find licence record.
		 */
		if (!strcmp (local_rec.lic_cat, "  ") ||
		      poln_rec.hhlc_hash == 0L)
		{
			strcpy (store [lcount [PO_LINES]].lic_cat, "  ");
			strcpy (store [lcount [PO_LINES]].lic_no, "          ");
			store [lcount [PO_LINES]].lic_hash = 0L;
			store [lcount [PO_LINES]].lic_rate = 0.00;
			strcpy (local_rec.lic_no, "          ");
			local_rec.lic_val = 0.00;
		}
		else
		{
			polh_rec.hhlc_hash = poln_rec.hhlc_hash;
			cc = find_rec (polh, &polh_rec, COMPARISON, "r");
			if (cc) 
				file_err (cc, polh, DBFIND);

			strcpy (store [lcount [PO_LINES]].lic_cat, polh_rec.lic_cate);
			strcpy (store [lcount [PO_LINES]].lic_no, polh_rec.lic_no);
			store [lcount [PO_LINES]].lic_hash = polh_rec.hhlc_hash;
			store [lcount [PO_LINES]].lic_rate = polh_rec.ap_lic_rate;
			strcpy (local_rec.lic_no, polh_rec.lic_no);
			local_rec.lic_val = polh_rec.ap_lic_rate;
		}
		/*
		 * Setup local record.
		 */
		local_rec.rec_qty = ToLclUom 
							(
								poln_rec.qty_ord, 
								store [lcount [PO_LINES]].pur_conv
							);
		local_rec.ret_qty = ToLclUom 
							(
								poln_rec.qty_rec, 
								store [lcount [PO_LINES]].pur_conv
							);
		local_rec.case_no 		= poln_rec.case_no;
		local_rec.cif_loc 		= poln_rec.fob_nor_cst;
		local_rec.loc_fi 		= poln_rec.frt_ins_cst;
		local_rec.duty_val 		= poln_rec.duty;
		store [lcount [PO_LINES]].val_duty = poln_rec.duty;
		store [lcount [PO_LINES]].val_fi = poln_rec.frt_ins_cst;
		local_rec.lic_val 		= poln_rec.licence;
		local_rec.other 		= poln_rec.lcost_load;
		local_rec.land_cst 		= poln_rec.land_cst;

		local_rec.Dsp_land_cst 	= poln_rec.land_cst;
		local_rec.Dsp_land_cst 	/= DPP (store [lcount [PO_LINES]].pur_conv);

		store [lcount [PO_LINES]].land_cost = local_rec.land_cst;
		local_rec.due_date = poln_rec.due_date;

		store [lcount [PO_LINES]].quantity = local_rec.ret_qty;
		store [lcount [PO_LINES]].exch_rate = (poln_rec.exch_rate == 0.00) ?
									local_rec.exch_rate : poln_rec.exch_rate;

		store [lcount [PO_LINES]].ship_no = poln_rec.ship_no;
		if (store [lcount [PO_LINES]].outer != 0.00)
		{
			local_rec.grs_fgn = poln_rec.grs_fgn_cst;
			local_rec.grs_fgn /= DPP (store [lcount [PO_LINES]].pur_conv);
			local_rec.grs_fgn /= store [lcount [PO_LINES]].outer;

			local_rec.net_fob = poln_rec.fob_fgn_cst;
			local_rec.net_fob /= DPP (store [lcount [PO_LINES]].pur_conv);
			local_rec.net_fob /= store [lcount [PO_LINES]].outer;
		}
		store [lcount [PO_LINES]].grs_fgn 		= local_rec.grs_fgn;
		store [lcount [PO_LINES]].net_fob 		= local_rec.net_fob;
		store [lcount [PO_LINES]].discArray [0] 	= poln_rec.reg_pc;
		store [lcount [PO_LINES]].discArray [1] 	= poln_rec.disc_a;
		store [lcount [PO_LINES]].discArray [2] 	= poln_rec.disc_b;
		store [lcount [PO_LINES]].discArray [3] 	= poln_rec.disc_c;

		strcpy (local_rec.view_disc, (poln_rec.cumulative) ? "Y" : "N");
		for (i = 0; i < 4; i++)
			if (store [lcount [PO_LINES]].discArray [i])
				strcpy (local_rec.view_disc, "Y");
				
		store [lcount [PO_LINES]].cumulative   	= poln_rec.cumulative;
		store [lcount [PO_LINES]].upd_inis 		= FALSE;
		strcpy (store [lcount [PO_LINES]].ser_no, poln_rec.serial_no);
		
		/*
		 * Put this bit in here to handle change of other etc
		 */
		CalculateCost (lcount [PO_LINES]);
		LoadPONS (lcount [PO_LINES], poln_rec.hhpl_hash);

		putval (lcount [PO_LINES]++);

		cc = find_rec (poln, &poln_rec, NEXT, "r");

		/*
		 * Too many orders.
		 */
		if (lcount [PO_LINES] > MAXLINES) 
			break;
	}

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (polh, "polh_id_no");
	abc_selfield (poln, "poln_hhpl_hash");
	abc_selfield (pogd, "pogd_id_no3");

	scn_set (PO_HEAD);

	/*
	 * No entries to edit.
	 */
	if (lcount [PO_LINES] == 0)
	{
		/*
		 * Purchase Return has no lines.
		 */
		print_mess (ML (mlPoMess113)); 
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Normal exit - return 0.
	 */
	return (EXIT_SUCCESS);
}

/*
 * Routine to read all poln records whose hash matches the one on the   
 * pohr record. Stores all non screen relevant details in another       
 * structure. Also gets part number for the part hash. And g/l account  
 * number.                                                              
 */
int
LoadPurchaseOrder (
	long	hhpoHash)
{
	float	returnQuantity	=	0.00;
	int		i;

	/*
	 * Set screen for putval.
	 */
	scn_set (PO_LINES);
	lcount [PO_LINES] = 0;
	
	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (polh, "polh_hhlc_hash");

	poln3_rec.hhpo_hash	= hhpoHash;
	poln3_rec.line_no 	= 0; 

	cc = find_rec (poln3, &poln3_rec, GTEQ, "r");
	while (!cc && poln3_rec.hhpo_hash == hhpoHash)
	{
		if (poln3_rec.qty_rec <= 0.00)
		{
			cc = find_rec (poln3, &poln3_rec, NEXT, "r");
			continue;
		}
		returnQuantity		=	poln3_rec.qty_rec;
		poln2_rec.hhpl_orig	=	poln3_rec.hhpl_hash;
		cc = find_rec (poln2, &poln2_rec, GTEQ, "r");
		while (!cc && poln2_rec.hhpl_orig == poln3_rec.hhpl_hash)
		{
			returnQuantity	=	 poln2_rec.qty_rec - returnQuantity;
			cc = find_rec (poln2, &poln2_rec, NEXT, "r");
		}
		if (returnQuantity <= 0.00)
		{
			cc = find_rec (poln3, &poln3_rec, NEXT, "r");
			continue;
		}
		/*
		 * Get part number.
		 */
		inmr_rec.hhbr_hash	=	poln3_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln3, &poln3_rec, NEXT, "r");
			continue;
		}
		if (strcmp (inmr_rec.supercession, "                "))
		{
			abc_selfield (inmr, "inmr_id_no");
			FindSupercession (comm_rec.co_no, inmr_rec.supercession, TRUE);
			abc_selfield (inmr, "inmr_hhbr_hash");
		}
		store [lcount [PO_LINES]].hhbrHash = inmr_rec.hhbr_hash;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poln3, &poln3_rec, NEXT, "r");
			continue;
		}
		strcpy (store [lcount [PO_LINES]].std_uom, inum_rec.uom);
		strcpy (local_rec.std_uom, inum_rec.uom);
		strcpy (store [lcount [PO_LINES]].uom_group, inum_rec.uom_group);
		store [lcount [PO_LINES]].StdCnvFct 		= inum_rec.cnv_fct;
		store [lcount [PO_LINES]].hhumHash = inum_rec.hhum_hash;

		inum_rec.hhum_hash	=	poln3_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (store [lcount [PO_LINES]].std_uom, inum_rec.uom);
			strcpy (local_rec.std_uom, inum_rec.uom);
			store [lcount [PO_LINES]].pur_conv = 1.00;
		}
		else
		{
			if (inum_rec.cnv_fct == 0.00)
				inum_rec.cnv_fct = 1.00;

			strcpy (store [lcount [PO_LINES]].sup_uom, inum_rec.uom);
			strcpy (local_rec.sup_uom, inum_rec.uom);
			store [lcount [PO_LINES]].pur_conv	=	
			   		store [lcount [PO_LINES]].StdCnvFct / inum_rec.cnv_fct;
			local_rec.pur_conv 	= 
			   		store [lcount [PO_LINES]].StdCnvFct / inum_rec.cnv_fct;

			store [lcount [PO_LINES]].hhumHash = inum_rec.hhum_hash;
		}
		strcpy (local_rec.item_no, inmr_rec.item_no);

		GetWarehouse (poln3_rec.hhcc_hash);

		add_hash 
		(
			comm_rec.co_no, 
			local_rec.br_no, 
			"RC", 
			0, 
			poln3_rec.hhbr_hash, 
			poln3_rec.hhcc_hash, 
			0L, 
			0.00
		);
		store [lcount [PO_LINES]].outer = (double) inmr_rec.outer_size;

		strcpy (store [lcount [PO_LINES]].itemClass, inmr_rec.inmr_class);

		if (store [lcount [PO_LINES]].outer == 0.0)
			store [lcount [PO_LINES]].outer = 1.0;

		strcpy (store [lcount [PO_LINES]].costingFlag, inmr_rec.costing_flag);
		strcpy (store [lcount [PO_LINES]].item_desc, inmr_rec.description);

		if (FindInis (inmr_rec.hhbr_hash, sumr_rec.hhsu_hash))
		{
			/* inis *not* found! */
			store [lcount [PO_LINES]].no_inis 	= TRUE;
			store [lcount [PO_LINES]].lead_time	= 0.00;
			store [lcount [PO_LINES]].sea_time 	= 0.00;
			store [lcount [PO_LINES]].air_time 	= 0.00;
			store [lcount [PO_LINES]].lnd_time 	= 0.00;
			store [lcount [PO_LINES]].weight 	= 0.00;
			store [lcount [PO_LINES]].volume 	= 0.00;

			strcpy (local_rec.duty_code, inmr_rec.duty);
			strcpy (local_rec.lic_cat, inmr_rec.licence);
			store [lcount [PO_LINES]].cst_price = local_rec.fob_cost = 0.00;

			store [lcount [PO_LINES]].min_order = 0;
			store [lcount [PO_LINES]].ord_multiple = 0;

			/*
			 * Find part number for branch record.
			 */
			ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (ineiRec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &ineiRec, COMPARISON, "r");
			if (cc) 
				file_err (cc, inei, DBFIND);

			store [lcount [PO_LINES]].cst_price = ineiRec.last_cost;
			store [lcount [PO_LINES]].cst_price *= pohr_rec.curr_rate;
			store [lcount [PO_LINES]].cst_price /= store [lcount [PO_LINES]].outer;
		}
		else
		{
			store [lcount [PO_LINES]].no_inis = FALSE;
			store [lcount [PO_LINES]].weight = inis_rec.weight;
			store [lcount [PO_LINES]].volume = inis_rec.volume;

			store [lcount [PO_LINES]].sea_time = inis_rec.sea_time;
			store [lcount [PO_LINES]].air_time = inis_rec.air_time;
			store [lcount [PO_LINES]].lnd_time = inis_rec.lnd_time;
			switch (pohr_rec.ship_method [0])
			{
			case 'S':
				SR.lead_time = inis_rec.sea_time;
				break;

			case 'A':
				SR.lead_time = inis_rec.air_time;
				break;

			case 'L':
				SR.lead_time = inis_rec.lnd_time;
				break;
			}
			store [lcount [PO_LINES]].min_order = 	
										ToStdUom 
										(
											inis_rec.min_order, 
											store [lcount [PO_LINES]].pur_conv
										);
			store [lcount [PO_LINES]].ord_multiple = 
										ToStdUom 
										(
											inis_rec.ord_multiple, 
							   				store [lcount [PO_LINES]].pur_conv
										);

			strcpy (local_rec.duty_code, inis_rec.duty);
			strcpy (local_rec.lic_cat, inis_rec.licence);

			strcpy (inmr_rec.duty, local_rec.duty_code);
			strcpy (inmr_rec.licence, local_rec.lic_cat);
			local_rec.fob_cost = DPP (inis_rec.fob_cost);
			store [lcount [PO_LINES]].cst_price = DPP (inis_rec.fob_cost);

		}
		if (!strcmp (local_rec.duty_code, "  "))
		{
			store [lcount [PO_LINES]].imp_duty = 0.00;
			store [lcount [PO_LINES]].val_duty = 0.00;
			strcpy (store [lcount [PO_LINES]].duty_type, " ");
			local_rec.duty_val = 0.00;
		}
		else
		{
			strcpy (podt_rec.co_no, comm_rec.co_no);
			strcpy (podt_rec.code, local_rec.duty_code);
			cc = find_rec (podt, &podt_rec, COMPARISON, "r");
			if (cc)
			{
				/*
				 * Duty Code Not found.
				 */
				print_mess (ML (mlStdMess124));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			store [lcount [PO_LINES]].imp_duty = podt_rec.im_duty;
			strcpy (store [lcount [PO_LINES]].duty_type, podt_rec.duty_type);
		}
		/*
		 * Find licence record.
		 */
		if (!strcmp (local_rec.lic_cat, "  ") ||
		      poln3_rec.hhlc_hash == 0L)
		{
			strcpy (store [lcount [PO_LINES]].lic_cat, "  ");
			strcpy (store [lcount [PO_LINES]].lic_no, "          ");
			store [lcount [PO_LINES]].lic_hash = 0L;
			store [lcount [PO_LINES]].lic_rate = 0.00;
			strcpy (local_rec.lic_no, "          ");
			local_rec.lic_val = 0.00;
		}
		else
		{
			polh_rec.hhlc_hash = poln3_rec.hhlc_hash;
			cc = find_rec (polh, &polh_rec, COMPARISON, "r");
			if (cc) 
				file_err (cc, polh, DBFIND);

			strcpy (store [lcount [PO_LINES]].lic_cat, polh_rec.lic_cate);
			strcpy (store [lcount [PO_LINES]].lic_no, polh_rec.lic_no);
			store [lcount [PO_LINES]].lic_hash = polh_rec.hhlc_hash;
			store [lcount [PO_LINES]].lic_rate = polh_rec.ap_lic_rate;
			strcpy (local_rec.lic_no, polh_rec.lic_no);
			local_rec.lic_val = polh_rec.ap_lic_rate;
		}
		/*
		 * Setup local record.
		 */
		local_rec.rec_qty 		= 	ToLclUom 
									(
										returnQuantity, 
									  	store [lcount [PO_LINES]].pur_conv
									);
		local_rec.ret_qty 		= 	ToLclUom 
									(
										returnQuantity, 
									  	store [lcount [PO_LINES]].pur_conv
									);
		local_rec.case_no 		= poln3_rec.case_no;
		local_rec.cif_loc 		= poln3_rec.fob_nor_cst;
		local_rec.loc_fi 		= poln3_rec.frt_ins_cst;
		local_rec.duty_val 		= poln3_rec.duty;
		store [lcount [PO_LINES]].val_duty 	= poln3_rec.duty;
		store [lcount [PO_LINES]].val_fi 	= poln3_rec.frt_ins_cst;
		store [lcount [PO_LINES]].hhplHash 	= poln3_rec.hhpl_hash;
		local_rec.lic_val 		= poln3_rec.licence;
		local_rec.other 		= poln3_rec.lcost_load;
		local_rec.land_cst 		= poln3_rec.land_cst;

		local_rec.Dsp_land_cst 	= poln3_rec.land_cst;
		local_rec.Dsp_land_cst 	/= DPP (store [lcount [PO_LINES]].pur_conv);

		store [lcount [PO_LINES]].land_cost = local_rec.land_cst;
		local_rec.due_date 		= poln3_rec.due_date;

		store [lcount [PO_LINES]].quantity = local_rec.ret_qty;
		store [lcount [PO_LINES]].exch_rate = (poln3_rec.exch_rate == 0.00) ?
									local_rec.exch_rate : poln3_rec.exch_rate;

		store [lcount [PO_LINES]].ship_no = poln3_rec.ship_no;
		if (store [lcount [PO_LINES]].outer != 0.00)
		{
			local_rec.grs_fgn = poln3_rec.grs_fgn_cst;
			local_rec.grs_fgn /= DPP (store [lcount [PO_LINES]].pur_conv);
			local_rec.grs_fgn /= store [lcount [PO_LINES]].outer;

			local_rec.net_fob = poln3_rec.fob_fgn_cst;
			local_rec.net_fob /= DPP (store [lcount [PO_LINES]].pur_conv);
			local_rec.net_fob /= store [lcount [PO_LINES]].outer;
		}
		store [lcount [PO_LINES]].grs_fgn 		= local_rec.grs_fgn;
		store [lcount [PO_LINES]].net_fob 		= local_rec.net_fob;
		store [lcount [PO_LINES]].discArray [0] 	= poln3_rec.reg_pc;
		store [lcount [PO_LINES]].discArray [1] 	= poln3_rec.disc_a;
		store [lcount [PO_LINES]].discArray [2] 	= poln3_rec.disc_b;
		store [lcount [PO_LINES]].discArray [3] 	= poln3_rec.disc_c;

		strcpy (local_rec.view_disc, (poln3_rec.cumulative) ? "Y" : "N");
		for (i = 0; i < 4; i++)
			if (store [lcount [PO_LINES]].discArray [i])
				strcpy (local_rec.view_disc, "Y");
				
		store [lcount [PO_LINES]].cumulative   	= poln3_rec.cumulative;
		store [lcount [PO_LINES]].upd_inis 		= FALSE;
		strcpy (store [lcount [PO_LINES]].ser_no, poln3_rec.serial_no);
		
		/*
		 * Put this bit in here to handle change of other etc
		 */
		CalculateCost (lcount [PO_LINES]);
		LoadPONS (lcount [PO_LINES], poln3_rec.hhpl_hash);

		/*
		 * Ensure poln_hhpl_hash is set to zero.
		 */
		poln_rec.hhpl_hash	=	0L;
		putval (lcount [PO_LINES]++);

		cc = find_rec (poln3, &poln3_rec, NEXT, "r");

		/*
		 * Too many orders .
		 */
		if (lcount [PO_LINES] > MAXLINES) 
			break;
	}

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (polh, "polh_id_no");
	abc_selfield (pogd, "pogd_id_no3");

	scn_set (PO_HEAD);

	/*
	 * No entries to edit.
	 */
	if (lcount [PO_LINES] == 0)
	{
		/*
		 * PO has no detail lines.
		 */
		print_mess (ML (mlPoMess004));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Normal exit - return 0.
	 */
	return (EXIT_SUCCESS);
}

/*
 * Update Relevent Records.
 */
void
Update (void)
{
	int		key			= 0,
			i 			= 0,
			po_line 	= 0,
			cost_change = FALSE,
			messageCnt	= 0,
			openPipe	= FALSE;

	float	sr_qty = 0.00;
	char	tempNumber [16];

	deleteLine = 0;

	clear ();
	scn_set (PO_HEAD);
		
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, zero_pad (local_rec.supplierNo, 6));
	cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sumr, DBFIND);

	if (newPoCredit && env.PoInput)
	{
	    if (!env.PoNumGen)
	    {
			open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

			strcpy (comr_rec.co_no, comm_rec.co_no);
			cc = find_rec (comr, &comr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, comr, DBFIND);

			open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");

			sprintf (tempNumber, "%ld", ++comr_rec.nx_pc_no);

			while (CheckPohr (tempNumber) == 0)
			{
				sprintf (tempNumber, "%ld", ++comr_rec.nx_pc_no);
			}
			abc_fclose (pohr2);

			cc = abc_update (comr, &comr_rec);
			if (cc)
				file_err (cc, comr, "DBUPDATE");

			sprintf (pohr_rec.pur_ord_no,"CR%-13.13s",zero_pad (tempNumber,13));


			abc_fclose (comr);
	    }
	    else
	    {
			open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			sprintf (esmr_rec.est_no, comm_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, esmr, DBFIND);

			/*
			 * Check if Order No Already Allocated if it has been then skip. 
			 */

			open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");

			sprintf (tempNumber, "%ld", ++esmr_rec.nx_pur_crd_no);
			while (CheckPohr (tempNumber) == 0)
				sprintf (tempNumber, "%ld", ++esmr_rec.nx_pur_crd_no);
			
			abc_fclose (pohr2);

			cc = abc_update (esmr, &esmr_rec);
			if (cc)
		    	file_err (cc, esmr, "DBUPDATE");
			
			sprintf (pohr_rec.pur_ord_no,"CR%-13.13s",zero_pad (tempNumber,13));

			abc_fclose (esmr);
	    }
	}
	/*
	 * Add new purchase order header.
	 */
	if (newPoCredit) 
	{
		sprintf (err_str, ML (mlPoMess106), pohr_rec.pur_ord_no);
		print_at (messageCnt++, 0, "%s", err_str);
		fflush (stdout);

		strcpy (pohr_rec.type, "C");
		strcpy (pohr_rec.co_no, comm_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.curr_code, sumr_rec.curr_code);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		pohr_rec.curr_rate = local_rec.exch_rate;
		strcpy (pohr_rec.status, "O");
		sprintf (pohr_rec.op_id, "%-14.14s", currentUser);
		pohr_rec.date_create = TodaysDate ();
		strcpy (pohr_rec.time_create, TimeHHMM ());

		cc = abc_add (pohr, &pohr_rec);
		if (cc) 
			file_err (cc, pohr, "DBADD");

		cc = find_rec (pohr, &pohr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, pohr, DBFIND);

		print_at (messageCnt++, 0, "%s", ML (mlPoMess107));
	}
	else
	{
		/*
		 * Updating Return Detail Lines.
		 */
		print_at (messageCnt++, 0, "%s", ML (mlPoMess108));
	}

	fflush (stdout);

	/*
	 * Process all purchase order lines.
	 */
	scn_set (PO_LINES);
	for (line_cnt = 0;line_cnt < lcount [PO_LINES];line_cnt++) 
	{
		getval (line_cnt);
	
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, DBFIND);
	
		if (inmr_rec.serial_item [0] == 'Y' && local_rec.ret_qty > 1.00)
		{
			sr_qty = local_rec.ret_qty;
			for (i = 0;i < sr_qty;i++)
			{
				local_rec.ret_qty = 1.00;
				UpdatePoln (po_line++, (i) ? 0L : poln_rec.hhpl_hash, line_cnt);
			}
		}
		else
			UpdatePoln (po_line++, poln_rec.hhpl_hash, line_cnt);

		if (SR.upd_inis)
		{
			UpdateInis (local_rec.net_fob * DPP (SR.pur_conv));
			cost_change = TRUE;
		}
	}
	/*
	 * Update existing order header.
	 */
	if (!newPoCredit) 
	{	
		/*
		 * Delete cancelled order.
		 */
		if (lcount [PO_LINES] == 0) 
		{
			print_at (messageCnt++, 0, "%s", ML (mlPoMess109)); 
			fflush (stdout);
			abc_unlock (pohr);

			/*
			 * Delete Purchase order header.
			 * if no poln recs (receipted) 
			 */
			abc_selfield (poln2, "poln_hhpo_hash");
			poln2_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
			cc = find_rec (poln2, &poln2_rec, EQUAL, "r");
			if (!cc)
				cc = abc_delete (pohr);

			abc_selfield (poln2, "poln_hhpl_hash");
		}
		/*
		 * Just update stat flag and rewrite.
		 */
		else 
		{
			/*
			 * Delete cancelled order.
			 */
			if (lcount [PO_LINES] == deleteLine)
			{
				print_at (messageCnt++, 0, "%s", ML (mlPoMess109)); 
				fflush (stdout);
			}
			else
			{
				/*
				 * Now Updating Existing Return
				 */
				print_at (messageCnt++, 0, "%s", ML (mlPoMess110));
				fflush (stdout);
				pohr_rec.curr_rate = local_rec.exch_rate;
				cc = abc_update (pohr, &pohr_rec);
				if (cc) 
					file_err (cc, pohr, "DBUPDATE");
			}
		}
	}
	strcpy (local_rec.previousPo, pohr_rec.pur_ord_no);
	strcpy (local_rec.previousCrdNo, sumr_rec.crd_no);

	recalc_sobg ();

	if (!env.PoCprint)
	{
		/*
		 * Purchase Return no (%s) %s. Press any key to continue.
		 */
		sprintf (err_str, (newPoCredit) ? ML (mlPoMess111) : ML (mlPoMess131), 
									  	pohr_rec.pur_ord_no);
		PauseForKey (messageCnt++, 0, err_str, 0);
	}
	else
	{
		/*
		 * Purchase Return no (%s) %s. Key F5 to Print any other
		 * key to continue.									
		 */
		sprintf (err_str, (newPoCredit) ? ML (mlPoMess112) : ML (mlPoMess132), 
 									  	pohr_rec.pur_ord_no);

		key = PauseForKey (messageCnt++, 0, err_str, 0);
		if ((key == 'P' || key == 'p') && !printerNumber)
			printerNumber = get_lpno (0);
	
		if (printerNumber && (key == 'P' || key == 'p'))
		{
			if (!running)
				openPipe = TRUE;

			if (openPipe == TRUE)
			{
				if ((pout = popen(creditNoteProgramName, "w")) == 0)
					file_err (errno, creditNoteProgramName, "POPEN");
				
				running = TRUE;
				fprintf(pout, "%d\n", printerNumber);
				fprintf(pout, "S\n");
#ifdef GVISION
				Remote_fflush (pout);
#else
				fflush (pout);
#endif
			}
			if (running == TRUE)
			{
				fprintf (pout, "%ld\n", pohr_rec.hhpo_hash);
#ifdef GVISION
				Remote_fflush (pout);
#else
				fflush (pout);
#endif
			}
		}
	}
	abc_unlock (pohr);
}

/*
 * Update or Add lines to Purchase order.
 */
void
UpdatePoln (
	int 	line_no, 
	long 	hhplHash, 
	int 	store_line)
{
	int 	addItem = FALSE;

	abc_selfield (poln, "poln_hhpl_hash");

	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, COMPARISON, "u");
	if (cc)
		addItem = TRUE;
	else
		addItem = FALSE;

	poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
	poln_rec.line_no 	= line_no;
	
	poln_rec.qty_ord 	= ToStdUom (local_rec.ret_qty, 
								    store [store_line].pur_conv);
	poln_rec.qty_rec 	= ToStdUom (local_rec.ret_qty, 
								    store [store_line].pur_conv);
	poln_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	poln_rec.hhcc_hash 	= local_rec.hhccHash;
	poln_rec.hhum_hash 	= store [store_line].hhumHash;
	poln_rec.hhpl_orig 	= store [store_line].hhplHash;
	poln_rec.exch_rate 	= store [store_line].exch_rate;
	poln_rec.ship_no 	= store [store_line].ship_no;
	poln_rec.hhlc_hash 	= (long) (store [store_line].lic_hash);
	poln_rec.case_no 	= local_rec.case_no;
	if (pohr_rec.due_date > 0L && store [line_no].ship_no == 0L)
		poln_rec.due_date = pohr_rec.due_date;
	else
		poln_rec.due_date = local_rec.due_date;

	poln_rec.reg_pc 	= store [store_line].discArray [0];
	poln_rec.disc_a 	= store [store_line].discArray [1];
	poln_rec.disc_b 	= store [store_line].discArray [2];
	poln_rec.disc_c 	= store [store_line].discArray [3];
	poln_rec.cumulative	= store [store_line].cumulative;

	poln_rec.grs_fgn_cst = local_rec.grs_fgn;
	poln_rec.grs_fgn_cst *= store [store_line].outer;
	poln_rec.grs_fgn_cst *= DPP (store [store_line].pur_conv);

	poln_rec.fob_fgn_cst = local_rec.net_fob;
	poln_rec.fob_fgn_cst *= store [store_line].outer;
	poln_rec.fob_fgn_cst *= DPP (store [store_line].pur_conv);
	
	poln_rec.fob_nor_cst 	= local_rec.cif_loc;
	poln_rec.frt_ins_cst 	= local_rec.loc_fi;
	poln_rec.duty 			= local_rec.duty_val;
	poln_rec.licence 		= local_rec.lic_val;
	poln_rec.lcost_load 	= local_rec.other;
	poln_rec.land_cst 		= local_rec.land_cst;
	strcpy (poln_rec.item_desc, inmr_rec.description);
	strcpy (poln_rec.cat_code, inmr_rec.category);
	strcpy (poln_rec.stat_flag, "B");

	if (addItem)
	{
		sprintf (poln_rec.serial_no, "%25.25s", " ");
		strcpy (poln_rec.pur_status, "O");
		if (local_rec.ret_qty != 0.00)
		{
			cc = abc_add (poln, &poln_rec);
			if (cc) 
				file_err (cc, poln, "DBUPDATE");

			abc_selfield (poln, "poln_id_no");
	
			poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
			poln_rec.line_no 	= line_no;
			cc = find_rec (poln, &poln_rec, COMPARISON, "r");
			if (cc) 
				file_err (cc, poln, DBFIND);
		}
		UpdatePONS (line_no, poln_rec.hhpl_hash);
	}
	else
	{
		if (poln_rec.qty_rec == 0.00)
		{
			/*
			 * Delete existing order line.
			 */
			if (inmr_rec.serial_item [0] == 'Y' && 
			     strcmp (poln_rec.serial_no, serialSpace))
			{
				DeleteInsf ();
			}

			DeletePONS (poln_rec.hhpl_hash);
			
			cc = abc_delete (poln);
			deleteLine++;
		}
		else
		{
			/*
			 * Update existing order.
			 */
			cc = abc_update (poln, &poln_rec);
			if (cc) 
				file_err (cc, poln, "DBUPDATE");

			UpdatePONS (line_no, poln_rec.hhpl_hash);
		}
		abc_unlock (poln);
	}
	if (inmr_rec.serial_item [0] == 'Y' && strcmp (poln_rec.serial_no, serialSpace))
		InsfUpdate ();

	add_hash 
	(
		comm_rec.co_no, 
		local_rec.br_no, 
		"RC", 
		0, 
		poln_rec.hhbr_hash, 
		poln_rec.hhcc_hash, 
		0L, 
		0.00
	);
}

/*
 * Search for Special instructions.
 */
void
SrchExsi (
	char	*key_val)
{
	char	wk_code [4];

	_work_open (3, 0, 60);
	save_rec ("#SI.", "#Special Instruction description.");

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);

	cc = find_rec (exsi, &exsi_rec, GTEQ, "r");
	while (!cc && !strcmp (exsi_rec.co_no, comm_rec.co_no))
	{
		sprintf (wk_code, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wk_code, err_str);
		if (cc)
			break;

		cc = find_rec (exsi, &exsi_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsi, DBFIND);
}

/*
 * Search for Payment Terms.
 */
void
ShowPay (
 void)
{
	int		i = 0;

	_work_open (3,0,60);
	save_rec ("#No.", "#Supplier Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode, p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*
 * Search for order number. 
 */
void
SrchPohr (
	char	*key_val)
{
	_work_open (15,0,12);
	save_rec ("#P/O Return No. ", "#Date Raised");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", key_val);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "C");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr_rec.pur_ord_no, key_val, strlen (key_val)) && 
		!strcmp (pohr_rec.co_no, comm_rec.co_no) && 
		!strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] == 'C' && pohr_rec.status [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&pohr_rec, 0, sizeof (pohr_rec));
		return;
	}
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "C");
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pohr, DBFIND);
}

/*
 * Search2 for order number.
 */
void
SrchPohr2 (
 char *key_val)
{
	abc_selfield (pohr, "pohr_id_no2");
	abc_selfield (sumr, "sumr_hhsu_hash");

	_work_open (15,0,60);
	sprintf 
	(
		err_str, 
		"#%-7.7s%-10.10s%-40.40s", 
		"Crd No", 
		"Acronym", 
		"Supplier Name"
	);
	save_rec ("#PO No.", err_str);
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", key_val);
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr_rec.pur_ord_no, key_val, strlen (key_val)) && 
		!strcmp (pohr_rec.co_no, comm_rec.co_no) && 
		!strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.type [0] == 'C' && pohr_rec.status [0] != 'D')
		{
			sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (pohr, &pohr_rec, NEXT, "r");
				continue;
			}

			sprintf 
			(
				err_str, 
				"%-7.7s%-10.10s%-40.40s", 
				 sumr_rec.crd_no, 
				 sumr_rec.acronym, 
				 sumr_rec.crd_name
			);
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pohr, DBFIND);

	abc_selfield (pohr, "pohr_id_no");
	abc_selfield (sumr, (env.CrFind) ? "sumr_id_no3" : "sumr_id_no");
}

/*
 * Search3 for order number.
 */
void
SrchPohr3 (
	char *key_val)
{
	abc_selfield (pohr3, "pohr_id_no");
	abc_selfield (sumr, "sumr_hhsu_hash");

	_work_open (15,0,60);
	sprintf 
	(
		err_str, 
		"#%-7.7s%-10.10s%-40.40s", 
		"Crd No", 
		"Acronym", 
		"Supplier Name"
	);
					
	save_rec ("#P/Order Number.", err_str);
	strcpy (pohr3_rec.co_no, comm_rec.co_no);
	strcpy (pohr3_rec.br_no, comm_rec.est_no);
	strcpy (pohr3_rec.type, "O");
	pohr3_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr3_rec.pur_ord_no, "%-15.15s", key_val);
	cc = find_rec (pohr3, &pohr3_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr3_rec.pur_ord_no, key_val, strlen (key_val)) && 
		!strcmp (pohr3_rec.co_no, comm_rec.co_no) && 
		!strcmp (pohr3_rec.br_no, comm_rec.est_no) &&
		pohr3_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if (pohr3_rec.type [0] == 'O' && pohr3_rec.status [0] != 'D')
		{
			sumr_rec.hhsu_hash	=	pohr3_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec (pohr3, &pohr3_rec, NEXT, "r");
				continue;
			}

			sprintf (err_str, "%-7.7s%-10.10s%-40.40s", 
						 sumr_rec.crd_no, sumr_rec.acronym, sumr_rec.crd_name);

			cc = save_rec (pohr3_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr3, &pohr3_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pohr3_rec.co_no, comm_rec.co_no);
	strcpy (pohr3_rec.br_no, comm_rec.est_no);
	strcpy (pohr3_rec.type, "O");
	pohr3_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr3_rec.pur_ord_no, "%-15.15s", temp_str);
	cc = find_rec (pohr3, &pohr3_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pohr3, DBFIND);

	abc_selfield (sumr, (env.CrFind) ? "sumr_id_no3" : "sumr_id_no");
}

/*
 * Search routine for Licence Header file.
 */
void
SrchPolh (
	char	*key_val)
{
	_work_open (10,0,40);
	save_rec ("#Lic. No.", "#Licence Type");
	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	strcpy (polh_rec.lic_cate, inmr_rec.licence);
	sprintf (polh_rec.lic_no, "%-10.10s", key_val);
	cc = find_rec (polh, &polh_rec, GTEQ, "r");
	while (!cc && !strcmp (polh_rec.co_no, comm_rec.co_no) && 
		      !strcmp (polh_rec.est_no, comm_rec.est_no) && 
		      !strcmp (polh_rec.lic_cate, inmr_rec.licence) && 
		      !strncmp (polh_rec.lic_no, key_val, strlen (key_val)))
	{
		cc = save_rec (polh_rec.lic_no, polh_rec.type);
		if (cc)
			break;
		cc = find_rec (polh, &polh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	strcpy (polh_rec.lic_cate, inmr_rec.licence);
	sprintf (polh_rec.lic_no, "%-10.10s", temp_str);
	cc = find_rec (polh, &polh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, polh, DBFIND);
}

void
pr_other (void)
{
	/*
	 * Purchase Return # %s  / Supplier %s %s
	 */
	print_at (2, 3, ML (mlPoMess105), 
		pohr_rec.pur_ord_no, 
		sumr_rec.crd_no, 
		sumr_rec.crd_name);

	/*
	 * Date %s  Currency %s %s  P/O Header Exchange Rate %8.4f
	 */
	print_at (3, 3, ML (mlPoMess073), 
		DateToString (pohr_rec.date_raised), 
		pocr_rec.code, 
		pocr_rec.description, 
		local_rec.exch_rate);

	fflush (stdout);
}

/*
 * Update Inventory Supplier Record.
 */
void
UpdateInis (
	double upd_value)
{
	/*
	 * Find inventory supplier records.
	 */
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (inis);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	}
	if (cc)
	{
		abc_unlock (inis);
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "u");
	}
	if (!cc)
	{
		inis_rec.fob_cost = upd_value;
		inis_rec.lcost_date = StringToDate (local_rec.systemDate);
		cc = abc_update (inis, &inis_rec);
		if (cc)
			file_err (cc, inis, "DBUPDATE");
	}
	else
		abc_unlock (inis);
}

int
CheckPohr (
	char	*poCreditNo)
{
	strcpy (pohr2_rec.co_no, comm_rec.co_no);
	strcpy (pohr2_rec.br_no, comm_rec.est_no);
	sprintf (pohr2_rec.pur_ord_no, "CR%-13.13s", zero_pad (poCreditNo, 13));
	return (find_rec (pohr2, &pohr2_rec, COMPARISON, "r"));
}

/*
 * Display Infor for lines while in edit mode.
 */
void
tab_other (
 int iline)
{
	if (cur_screen == PO_LINES)
	{
		if (env.PoMaxLines)
		{
			if (prog_status == ENTRY && iline >= env.PoMaxLines)
				centre_at (4, 132, ML (mlStdMess008));
			if (prog_status != ENTRY && lcount [PO_LINES] > env.PoMaxLines)
				centre_at (4, 132, ML (mlStdMess008));
		}
		if (store [iline].exch_rate == 0.00)
		{
			move (0, 5);
			cl_line ();
			move (0, 6);
			cl_line ();
			move (0, 7);
			cl_line ();
			return;
		}
		if (nd_flag [2] == 'Y')
		{
			print_at (5,  0, ML (mlStdMess039), local_rec.br_no, 
									  			local_rec.br_name);
			
			print_at (5, 55, ML (mlStdMess099), local_rec.wh_no, 
									  		   local_rec.wh_name);
		}
		print_at (6,  0, ML (mlPoMess075));
		print_at (6, 55, ML (mlPoMess076));
		print_at (6, 75, ML (mlPoMess077));
		print_at (6, 102, ML (mlPoMess078)); 
		print_at (7,  0, ML (mlPoMess079));
		print_at (7, 55, ML (mlPoMess080));

		print_at (6, 11, "%3d", iline + 1);
		if (store [iline].ship_no == 0L)
		{
			/*------
			| N/A   |
			-------*/
			print_at (6, 68, ML (mlPoMess117)); 
		}
		else
			print_at (6, 68, "%06ld", store [iline].ship_no);
		print_at (6, 91, "%-4.8f", store [iline].exch_rate);

		print_at (7, 11, "%-40.40s", 
			check_class (store [iline].itemClass) ? store [iline].nsDesc [0] 
											: store [iline].item_desc);
				
		print_at (7, 68, "%-4.4s", store [iline].std_uom);
		if (store [iline].outer > 0.00)
			print_at (6, 118, "%-7.1f", store [iline].outer);
		else
			print_at (6, 118, "%-7.1f", 1.00);

		if (strcmp (store [iline].ser_no, serialSpace))
		{
			/*---"%R Serial No : ");---*/
			print_at (7, 75, "%s %-25.25s ", ML (mlPoMess081), 
											store [iline].ser_no);
		}
		else
		{
			print_at (7, 75, "             ");
			print_at (7, 88, "%-25.25s", serialSpace);
		}

		fflush (stdout);

		if (prog_status != ENTRY)
			strcpy (local_rec.sup_uom, store [iline].sup_uom);

		if (store [iline].no_inis)
		{
			if (!F_HIDE (label ("due_date")))
				FLD ("due_date") = YES;
		}
		else
		{
			if (!F_HIDE (label ("due_date")))
				FLD ("due_date") = (pohr_rec.due_date) ? NA : NI;
		}
	}
	return;
}

/*
 * Update Pre-receipt Serial item.
 */
void
InsfUpdate (
 void)
{
	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		return;


	insfRec.hhwh_hash = incc_rec.hhwh_hash;
	sprintf (insfRec.serial_no, "%-25.25s", poln_rec.serial_no);
	cc = find_rec (insf, &insfRec, COMPARISON, "u");
	if (cc)
	{
		abc_fclose (insf);
		return;
	}
	insfRec.exch_rate   = poln_rec.exch_rate;
	insfRec.fob_fgn_cst = poln_rec.fob_fgn_cst;
	insfRec.fob_nor_cst = poln_rec.fob_nor_cst;
	insfRec.frt_ins_cst = poln_rec.frt_ins_cst;
	insfRec.duty        = poln_rec.duty;
	insfRec.licence     = poln_rec.licence;
	insfRec.lcost_load  = poln_rec.lcost_load;

	insfRec.land_cst    = poln_rec.fob_nor_cst +
				           poln_rec.lcost_load +
		          	       poln_rec.duty +
		          	       poln_rec.licence;

	insfRec.istore_cost = poln_rec.land_cst;
	insfRec.est_cost    = poln_rec.land_cst;

	cc = abc_update (insf, &insfRec);
	if (cc)
		file_err (cc, insf, "DBUPDATE");

	abc_fclose (insf);
	return;
}

/*
 * Delete Pre-receipt Serial item.
 */
void
DeleteInsf (
 void)
{
	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		return;

	insfRec.hhwh_hash = incc_rec.hhwh_hash;
	sprintf (insfRec.serial_no, "%-25.25s", poln_rec.serial_no);
	cc = find_rec (insf, &insfRec, COMPARISON, "u");
	if (cc)
		return;
	
	cc = abc_delete (insf);
	if (cc)
		file_err (cc, insf, "DBDELETE");

	return;
}

void
GetWarehouse (
	long	hhccHash)
{
	if (hhccHash == 0L)
	{
		if (warehouseSelected)
		{
			abc_selfield (ccmr, "ccmr_id_no");
			warehouseSelected = FALSE;
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, DBFIND);
	
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhccHash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, esmr, DBFIND);
		strcpy (local_rec.br_name, esmr_rec.est_name);
		abc_fclose (esmr);
	}
	else
	{
		if (!warehouseSelected)
		{
			abc_selfield (ccmr, "ccmr_hhcc_hash");
			warehouseSelected = TRUE;
		}
		
		ccmr_rec.hhcc_hash	=	hhccHash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			abc_selfield (ccmr, "ccmr_id_no");
			GetWarehouse (0L);
			return;
		}
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		local_rec.hhccHash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.wh_name, ccmr_rec.name);

		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, esmr, DBFIND);

		strcpy (local_rec.br_name, esmr_rec.est_name);
		abc_fclose (esmr);
	}
	return;
}

/*
 * Input not stock description lines for non-stock products.
 */
void	
InputPONS (
	int	line_cnt)
{
	int 	i, 
			tx_window;
	char	disp_str [200];
		
	sprintf (err_str, "ADDITIONAL DESCRIPTION FOR ITEM %s", inmr_rec.item_no);
	tx_window = txt_open (2, 1, 5, 40, 5, err_str);

	for (i = 0; i < MAX_PONS ; i++)
	{
		sprintf (disp_str, "%-40.40s", SR.nsDesc [i]);
		txt_pval (tx_window, disp_str, 0);
	}

	txt_edit (tx_window);

	for (i = 0; i < 5 ; i++)
		sprintf (SR.nsDesc [i], "%-40.40s", txt_gval (tx_window, i + 1));

	txt_close (tx_window, FALSE);
	ClearBox (0, 2, 43, 7);
}

/*
 * Clear screen where box to be drawn + if _box then draw box.
 */
void
ClearBox (
	int	x, 
	int y, 
	int h, 
	int v)
{
	int	i, 
		j;

	j = v;		
	i = y;
		
	while (j--)
	{
		if (h > 1)
			print_at (i, x, "%*.*s", h, h, " ");
		i++;
	}
}

/*
 * Update purchase order non stock lines file. 
 */
void	
UpdatePONS 	 (
 int	line_cnt, 
 long	hhpl_hash)
{
	int	i;

	for (i = 0; i < MAX_PONS; i++)
	{
		pons_rec.hhpl_hash 	= hhpl_hash;
		pons_rec.line_no 	= i;
		cc = find_rec (pons, &pons_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (pons_rec.desc, "%-40.40s", SR.nsDesc [i]);
			/*
			 * Add line only if it is not blank.
			 */
			if (strcmp (pons_rec.desc, ns_space))
			{
				cc = abc_add (pons, &pons_rec);
				if (cc)
					file_err (cc, pons, "DBADD");
			}
		}
		else
		{
			sprintf (pons_rec.desc, "%-40.40s", SR.nsDesc [i]);
			/*
			 * Update line only if it is not blank.
			 */
			if (strcmp (pons_rec.desc, ns_space))
			{
				cc = abc_update (pons, &pons_rec);
				if (cc)
					file_err (cc, pons, "DBUPDATE");
			}
			else
			{
				cc = abc_delete (pons);
				if (cc)
					file_err (cc, pons, "DBDELETE");
			}
		}
	}
}

/*
 * Load purchase order non stock lines file.
 */
void	
LoadPONS (
 int	line_cnt, 
 long	hhpl_hash)
{
	int	i;

	for (i = 0; i < MAX_PONS; i++)
		sprintf (SR.nsDesc [i], "%40.40s", " ");

	pons_rec.hhpl_hash 	= hhpl_hash;
	pons_rec.line_no 	= 0;
	cc = find_rec (pons, &pons_rec, GTEQ, "r");
	while (!cc && pons_rec.hhpl_hash == hhpl_hash)
	{
		sprintf (SR.nsDesc [pons_rec.line_no], "%40.40s", pons_rec.desc);

		cc = find_rec (pons, &pons_rec, NEXT, "r");
	}
}
/*
 * Delete purchase order non stock lines file.
 */
void	
DeletePONS (
 long	hhpl_hash) 
{
	pons_rec.hhpl_hash 	= hhpl_hash;
	pons_rec.line_no 	= 0;
	cc = find_rec (pons, &pons_rec, GTEQ, "r");
	while (!cc && pons_rec.hhpl_hash == hhpl_hash)
	{
		cc = abc_delete (pons);
		if (cc)
			file_err (cc, pons, "DBDELETE");

		pons_rec.hhpl_hash 	= hhpl_hash;
		pons_rec.line_no 	= 0;
		cc = find_rec (pons, &pons_rec, GTEQ, "r");
	}
}

/*
 * Menu called when CTRL W pressed
 */
int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn == PO_LINES)
	{
		InputPONS (lin); 
		pr_other ();
		line_at (4, 0, 132);
	}

	restart = FALSE;

	return (PSLW);
}

int 
LoadSupplier (
	int field)
{
	abc_selfield (sumr, "sumr_hhsu_hash");
	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, sumr, DBFIND);

	sprintf (local_rec.supplierNo, "%-6.6s", sumr_rec.crd_no);
	cc = spec_valid (field -1);
	abc_selfield (sumr, (env.CrFind) ? "sumr_id_no3" : "sumr_id_no");
	return (cc);
}

#ifndef GVISION
/*
 * Allow editing of dicounts for current line.
 */
void
ViewDiscounts (
 void)
{
	int		key;
	int		currFld;
	int		tmpLineCnt;
	double	oldFobFgn;
	float	oldDisc [4];

	/*
	 * Save old values.
	 */
	oldFobFgn = local_rec.grs_fgn;
	oldDisc [0] = SR.discArray [0];
	oldDisc [1] = SR.discArray [1];
	oldDisc [2] = SR.discArray [2];
	oldDisc [3] = SR.discArray [3];

	/*
	 * Draw box and fields.
	 */
	DrawDiscScn ();

	/*
	 * Allow cursor movement and selection for edit.
	 * Exit without change on FN1.                  
	 * Exit saving changes on FN16.                
	 */
	crsr_off ();
	currFld = 0;
	restart = FALSE;
	DispFields (currFld);
	while ((key = getkey ()) != FN16)
	{
		switch (key)
		{
		case BS:
		case LEFT_KEY:
		case UP_KEY:
			currFld--;
			if (currFld < 0)
				currFld = 4;
			break;

		case DOWN_KEY:
		case RIGHT_KEY:
		case ' ':
			currFld++;
			if (currFld >= 5)
				currFld = 0;
			break;

		case '\r':
			InputField (currFld);
			break;

		case FN1:
			/*
			 * Restore old values.
			 */
			local_rec.grs_fgn = oldFobFgn;
			SR.discArray [0] = oldDisc [0];
			SR.discArray [1] = oldDisc [1];
			SR.discArray [2] = oldDisc [2];
			SR.discArray [3] = oldDisc [3];
			restart = TRUE;
			break;

		case FN3:
			tmpLineCnt = line_cnt;
			heading (PO_LINES);
			line_cnt = tmpLineCnt;
			lcount [PO_LINES] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [PO_LINES];
			scn_display (PO_LINES);
			DrawDiscScn ();
			DispFields (currFld);
			break;
		}

		DispFields (currFld);
		if (restart)
			break;
	}
	restart = FALSE;
}

void
DrawDiscScn (
 void)
{
	int		i;
	int		fldWid;
	int		headXPos;

	/*
	 * Draw box.
	 */
	cl_box (DBOX_LFT, DBOX_TOP, DBOX_WID, DBOX_DEP);

	/*
	 * Draw middle horizontal line.
	 */
	line_at (4, 0, 132);
	line_at (DBOX_TOP + 2, DBOX_LFT + 1, DBOX_WID - 1);

	move (DBOX_LFT, DBOX_TOP + 2);
	PGCHAR (10);
	move (DBOX_LFT + DBOX_WID - 1, DBOX_TOP + 2);
	PGCHAR (11);

	/*
	 * Draw vertical dividing lines.
	 */
	for (i = 1; i < 6; i++)
		VertLine (DBOX_LFT + discScn [i].xPos, DBOX_TOP);

	/*
	 * Draw heading. 					
	 * Discounts are Cumulative or Absolute.
	 */
	sprintf (err_str, (SR.cumulative) ? ML (mlPoMess134) : ML (mlPoMess135)); 
	headXPos = DBOX_LFT + (DBOX_WID - strlen (err_str)) / 2;
	rv_pr (err_str, headXPos, DBOX_TOP, 1);

	/*
	 * Draw prompts.
	 */
	for (i = 0; i < 6; i++)
	{
		fldWid = strlen (discScn [i].fldPrompt);
		print_at 
		(
			DBOX_TOP + 1, 
			DBOX_LFT + discScn [i].xPos + 1, 
			" %-*.*s ", 
			fldWid, 
			fldWid, 
			discScn [i].fldPrompt
		);
	}
}

void
VertLine (
	int	xPos, 
	int	yPos)
{
	move (xPos, yPos); 		PGCHAR (8);
	move (xPos, yPos + 1); 	PGCHAR (5);
	move (xPos, yPos + 2); 	PGCHAR (7);
	move (xPos, yPos + 3); 	PGCHAR (5);
	move (xPos, yPos + 4); 	PGCHAR (9);
}

void
DispFields (
 int rvsField)
{
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [0].xPos + 2, 
			 		"%11.2f", local_rec.grs_fgn);
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [1].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.discArray [0]));
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [2].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.discArray [1]));
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [3].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.discArray [2]));
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [4].xPos + 2, 
			 		"%6.2f", ScreenDisc (SR.discArray [3]));

	local_rec.net_fob = CalcNet (local_rec.grs_fgn, SR.discArray, SR.cumulative);
	SR.net_fob = local_rec.net_fob;
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [5].xPos + 2, 
			 "%11.2f", local_rec.net_fob);

	/*
	 * Print highlighted field.
	 */
	switch (rvsField)
	{
	case 0:
		sprintf (err_str, "%11.2f", local_rec.grs_fgn);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		sprintf (err_str, "%6.2f", ScreenDisc (SR.discArray [rvsField - 1]));
		break;
	}
	rv_pr (err_str, DBOX_LFT + discScn [rvsField].xPos + 2, DBOX_TOP + 3, 1);
}

void
InputField (
	int		fld)
{
	int	fieldOk;

	crsr_on ();

	fieldOk = FALSE;
	while (!fieldOk)
	{
		fieldOk = TRUE;
		switch (fld)
		{
		case 0:
			local_rec.grs_fgn = getdouble (DBOX_LFT + discScn [fld].xPos + 2, 
									  	DBOX_TOP + 3, 
									  	discScn [fld].fldMask);

			break;
	
		case 1:
		case 2:
		case 3:
		case 4:
			SR.discArray [fld - 1] = getfloat (DBOX_LFT + discScn [fld].xPos + 2, 
										 	DBOX_TOP + 3, 
										 	discScn [fld].fldMask);
			if (SR.discArray [fld - 1] > 99.99)
			{
				/*
				 * Discount may not exceed 99.99 
				 */
				print_mess (ML (mlStdMess120));
				sleep (sleepTime);
				clear_mess ();
				fieldOk = FALSE;
			}
			SR.discArray [fld - 1] = ScreenDisc (SR.discArray [fld - 1]);
			break;
		}
	}
	crsr_off ();
}
#endif	/* GVISION */

/*
 * Reverse Screen Discount.
 */
float	
ScreenDisc (
	float	DiscountPercent)
{
	if (env.SoDiscRev)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

/*
 * Convert value passed (Qty in STD UOM) to Lcl UOM (UOM)
 * specified by local_rec.whUom                         
 */
float
ToLclUom (
	float	stdQty, 
	float	CnvFct)
{
	float	cnvQty;

	if (CnvFct == 0.00)
		return (stdQty);

	cnvQty = stdQty * CnvFct;

	return ((float) twodec (cnvQty));
}

/*
 * Convert value passed (Qty in LCL UOM) to STD UOM
 */
float 
ToStdUom (
	float	lclQty, 
	float	CnvFct)
{
	float	cnvQty;

	if (CnvFct == 0.00)
		return (lclQty);

	if (lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / CnvFct;

	return ((float) twodec (cnvQty));
}

/*
 * Search on UOM
 */
void
SrchInum (
	char 	*key_val, 
	int		line_cnt)
{
	_work_open (4,0,40);
	save_rec ("#UOM", "#Description");

	strcpy (inum2_rec.uom_group, SR.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR.uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}
		
		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, SR.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

/*
 * Check environment variables and set values in the envVar structure.
 */
void
CheckEnvironment (void)
{
	char	*sptr;

	currentUser = getenv ("LOGNAME");

	sptr = chk_env ("PO_MAX_LINES");
	env.PoMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	env.SoDiscRev = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (env.SupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (env.SupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (env.SupOrdRound, "D");
			break;

		default:
			sprintf (env.SupOrdRound, "B");
			break;
		}
	}
	/*
	 * Purchase Order number is M (anual or S (ystem generated).
	 */
	sptr = chk_env ("PO_INPUT");
	env.PoInput = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;

	/*
	 * Purchase Order number is Company or branch generated.
	 */
	sptr = chk_env ("PO_NUM_GEN");
	env.PoNumGen = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*
	 * Case Number Input env.PoCaseUsed = 'Y' and 'N'.
	 */
	sptr = chk_env ("PO_CASE_USED");
	if (sptr == (char *)0)
		strcpy (env.PoCaseUsed, "N");
	else
		sprintf (env.PoCaseUsed, "%-1.1s", sptr);

	/*
	 * Get Purchase order print program.
	 */
	sptr = chk_env ("PO_CPRINT");
	if (sptr == (char *)0)
		env.PoCprint = FALSE;
	else
	{
		env.PoCprint = TRUE;
		sprintf (creditNoteProgramName, "%-14.14s", sptr);
	}
	/*
	 * Check inis record must exist, env.PoInisReq = 'Y' and 'N'.
	 */
	sptr = chk_env ("PO_INIS_REQ");
	if (sptr == (char *)0)
		strcpy (env.PoInisReq, "N");
	else
		sprintf (env.PoInisReq, "%-1.1s", sptr);

	sptr 	= 	chk_env ("PO_INDENT_OK");
	env.PoIndentOk = (sptr == (char *) 0) ? 0 : atoi (sptr);

	sptr	=	get_env ("CR_FIND");
	env.CrFind 		= (sptr == (char *) 0) ? 0 : atoi (sptr);

	sptr		=	get_env ("CR_CO");
	env.CrCo	= (sptr == (char *) 0) ? 0 : atoi (sptr);

	sptr 		= 	chk_env ("PO_OVERRIDE");
	env.PoOverride 	  = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr 		= 	chk_env ("PO_RETURN_APPLY");
	env.PoReturnApply = (sptr == (char *)0) ? 0 : atoi (sptr);

	sprintf (env.CurrCode, "%-3.3s", get_env ("CURR_CODE"));
}
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	switch (scn)
	{
	case	PO_LINES:
		tab_row = 9;
		tab_col = 0;
		break;

	default:
		break;
	}

	if (scn != cur_screen)
		scn_set (scn);

	/*
	 * Purchase Return Maintenance.
	 */
	swide ();
	clear ();
	sprintf (err_str, " %s ", ML (mlPoMess116));
	rv_pr (err_str, 45, 0, 1);

	print_at (0, 80, ML (mlPoMess115), 	local_rec.previousCrdNo, 
										local_rec.previousPo);
	pr_box_lines (scn);

	switch (scn)
	{
	case	PO_HEAD:
		/*
		 * Return Terms.		
		 * Standard Instructions.
		 * Ship To Details.		
		 */
		us_pr (ML (mlPoMess119), 5, 8, 1);
		us_pr (ML (mlPoMess088), 5, 13, 1);
		us_pr (ML (mlPoMess092), 5, 17, 1);
		break;

	case	PO_LINES:
		line_at (21, 0, 132);
		if (env.PoMaxLines)
		{
			/*
			 * Warning, maximum lines exceeded
			 */
			if (prog_status == ENTRY && line_cnt >= env.PoMaxLines)
				centre_at (4, 132, ML (mlStdMess008));
			if (prog_status != ENTRY && lcount [PO_LINES] > env.PoMaxLines)
				centre_at (4, 132, ML (mlStdMess008));
		}
		pr_other ();

		break;

	default:
		break;
	}
	print_at (22, 0,  ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 40, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (22, 89, ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}
