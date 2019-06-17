/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_sh_inpt.c,v 5.11 2002/07/24 08:39:07 scott Exp $
|  Program Name  : (po_sh_inpt.c)
|  Program Desc  : (Overseas Shipment Detail Input)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/06/87         |
|---------------------------------------------------------------------|
| $Log: po_sh_inpt.c,v $
| Revision 5.11  2002/07/24 08:39:07  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.10  2002/07/18 07:00:28  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.9  2002/06/20 07:22:11  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/06/19 07:00:43  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.7  2001/12/03 01:28:33  scott
| Updated to allow containers on shipments.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_sh_inpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_sh_inpt/po_sh_inpt.c,v 5.11 2002/07/24 08:39:07 scott Exp $";

#define	MAXLINES	2001 
#define	MAXWIDTH	300

#define	CASE_USED	(envPoCaseUsed [0] == 'Y' || envPoCaseUsed [0] == 'y')

#include 	<pslscr.h>
#include 	<std_decs.h>
#include	<inis_update.h>
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<ml_tr_mess.h>
#include	<arralloc.h>

#define		DBOX_TOP	9
#define		DBOX_LFT	35
#define		DBOX_WID	66
#define		DBOX_DEP	3

#define	SR	store [line_cnt]

extern	int	_win_func;

#include	"schema"

typedef	int	BOOL;
struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct comrRecord	comr_rec;
struct inspRecord	insp_rec;
struct sudsRecord	suds_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;
struct pogdRecord	pogd_rec;
struct posdRecord	posd_rec;
struct posdRecord	posd2_rec;
struct polhRecord	polh_rec;
struct poslRecord	posl_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pocrRecord	pocr_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct pocfRecord	pocf_rec;
struct podtRecord	podt_rec;
struct insfRecord	insf_rec;
struct inccRecord	incc_rec;
struct ponsRecord	pons_rec;
struct ponsRecord	pons2_rec;
struct poliRecord	poli_rec;
struct poliRecord	poli2_rec;
struct skcmRecord	skcm_rec;

	char	*data  = "data", 
			*posh2 = "posh2", 
			*sumr2 = "sumr2", 
			*pohr2 = "pohr2", 
			*poln2 = "poln2", 
			*inis2 = "inis2", 
			*pons2 = "pons2", 
			*poli2 = "poli2";
		
	char	branchNumber [3], 
			envPoCaseUsed [2], 
			*serialSpace = "                         ", 
			ShipDefault [2];

	int		envCrCo 			= 0, 
			envCrFind 			= 0, 
			envPoShInpt			= 0, 
			envVarPoContainer	= 0,
			newShipment			= 0, 
			newOrder			= 0, 
			updateSupplier		= 0, 
			warehouseSelected 	= 0, 
			polnLineNo			= 0, 
			splitLines			= 0, 
			foundSupplier		= 0;

	struct	storeRec { 				/*-------------------------------*/
		double	extend;				/*| Extended value.	 			|*/
		float	cnv_fct;			/*| Conversion fct   			|*/
		double	NewPrice;			/*| Supplier price.  			|*/
		int		UpdateSupplierFlag;	/*| Inis update.	 			|*/
		char	item_desc [41];		/*| Item description.			|*/
		char	supplierPart [17];	/*| Supplier Part No.			|*/
		char	serialNo [26];		/*| Serial item.     			|*/
		char	serialItem [2];		/*| Serial item Y/N. 			|*/
		char	containerNo [16];   /*| Container Number.			|*/
		char	containerDesc [41]; /*| Container Description.		|*/
		char	uom [5];			/*| Item UOM.        			|*/
		float	onOrder;			/*| Item Quantity    			|*/
		float	shipQuantity;		/*| Ship Quantity    			|*/
		float	discArray [4];		/*| Regulatory and Discounts. 	|*/
		double	grs_fob;			/*| Item Quantity    			|*/
	} store [MAXLINES]; 			/*-------------------------------*/

	/*
	 * Structure used for pop-up discount screen.
	 */
	static	struct
	{
		char	fldPrompt [13];
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

	char	*screens [] = {
			" Shipment Details ", 
			" Purchase Order ", 
			" Shipment Lines ", 
	};

	long	systemDate;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	crd_no [7];
	long	hhplHash;
	double	extended;
	char	br_wh_no [8];
	char	br_no [3];	/* Branch number.			*/
	char	wh_no [3];	/* Warehouse number.			*/
	long	hhcc_hash;	/* Warehouse hhcc_hash.			*/
	float	cnv_fct;
	char	view_disc [2];
	char	uom [5];
	float	outer_size;
	float	QTY_ORD;
	float	QTY_REC;
	float	QTY_REM;
	float	QTY_SH;
	double	grs_fgn_cst;
	double	fob_fgn_cst;
	double	NEW_COST;
	char	systemDate [11];
	char	containerNo [16];
	char	lastContainerNo [16];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "ship_no", 	 3, 2, CHARTYPE, 
		"NNNNNNNNNNNN", "          ", 
		"0", "0", "Shipment Number  : ", " ", 
		 NE, NO, JUSTRIGHT, "1234567890", "", posh_rec.csm_no}, 
	{1, LIN, "curr_code", 	 4, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Currency Code    : ", " ", 
		 NE, NO,  JUSTLEFT, "", "", posh_rec.curr_code}, 
	{1, LIN, "curr_desc", 	 4, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Currency Desc.   : ", " ", 
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description}, 
	{1, LIN, "curr_rate", 	 5, 2, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", " ", "Exchange Rate    : ", " ", 
		YES, NO, JUSTRIGHT, "0", "9999.9999", (char *)&posh_rec.ex_rate}, 
	{1, LIN, "ship_method", 	 5, 52, CHARTYPE, 
		"U", "          ", 
		" ", ShipDefault, "Shipment Method  : ", "Shipment Method L(and) / S(ea) / A(ir)", 
		YES, NO,  JUSTLEFT, "LSA", "", posh_rec.ship_method}, 
	{1, LIN, "ship_depart", 	 6, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Ship Departure   : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.ship_depart}, 
	{1, LIN, "ship_arrive", 	 6, 52, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Shipment Arrival : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.ship_arrive}, 
	{1, LIN, "vessel", 	 8, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Shipment Name    : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.vessel}, 
	{1, LIN, "v_comm1", 	 8, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Shipment Comment : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.v_comm1}, 
	{1, LIN, "v_comm2", 	 9, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Shipment Comment : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.v_comm2}, 
	{1, LIN, "port", 	11, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Origin           : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.port}, 
	{1, LIN, "s_comm1", 	 11, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Departure Comment: ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.s_comm1}, 
	{1, LIN, "s_comm2", 	 12, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Departure Comment: ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.s_comm2}, 
	{1, LIN, "destination", 	14, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Destination      : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.destination}, 
	{1, LIN, "r_comm1", 	 14, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Arrival Comment  : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.r_comm1}, 
	{1, LIN, "s_comm2", 	 15, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Arrival Comment  : ", " ", 
		YES, NO,  JUSTLEFT, "", "", posh_rec.r_comm2}, 
	{1, LIN, "doc_rec", 	17, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Documts Received : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.doc_rec}, 
	{1, LIN, "doc_agent", 	17, 52, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Documts to Agent : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.doc_agent}, 
	{1, LIN, "neg_bol", 	18, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Negotiable B.O.L : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.neg_bol}, 
	{1, LIN, "bol_no", 	18, 52, CHARTYPE, 
		"UUUUUUUUUUUU", "          ", 
		" ", "", "Bill of Lading No: ", " ", 
		 NO, NO,  JUSTLEFT, "", "", posh_rec.bol_no}, 
	{1, LIN, "costing_date", 	19, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", "Costing Date     : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&posh_rec.costing_date}, 
	{2, LIN, "crd_no", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier Number  : ", " ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no}, 
	{2, LIN, "crd_name", 	 4, 52, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{2, LIN, "pur_ord_no", 	 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "Purchase Order # : ", " ", 
		 NE, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{2, LIN, "comment", 	 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "Entire", "Comment          : ", " ", 
		 NO, NO,  JUSTLEFT, "", "", posd_rec.comment}, 
	{2, LIN, "inv_no", 	 8, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Invoice Number   : ", " ", 
		NO, NO,  JUSTLEFT, "", "", posd_rec.inv_no}, 
	{2, LIN, "total", 	 9, 2, DOUBLETYPE, 
		"NNNNNNNNNN.NN", "          ", 
		" ", "", "Invoice Total    : ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&posd_rec.total}, 
	{3, TAB, "br_wh_no", 	MAXLINES, 0, CHARTYPE, 
		"NN/NN", "          ", 
		" ", " ", "BR/WH", "Enter Branch no / Warehouse no.", 
		 NI, NO,  JUSTLEFT, "0123456789 /", "", local_rec.br_wh_no}, 
	{3, TAB, "hhcc_hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNN", "          ", 
		" ", "", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.hhcc_hash}, 
	{3, TAB, "item_no", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", "  Item Number   ", " ", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.item_no}, 
	{3, TAB, "sup_item_no", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Supplier Part No", " ", 
		 NA, NO,  JUSTLEFT, "", "", inis2_rec.sup_part}, 
	{3, TAB, "poln_hhpo_hash", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", "", "po_has", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpo_hash}, 
	{3, TAB, "hhplHash", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", " ", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpl_hash}, 
	{3, TAB, "hhbr_hash", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", " ", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhbr_hash}, 
	{3, TAB, "qty_ord", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Qty Ord ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.QTY_ORD}, 
	{3, TAB, "orig_ord", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Qty Rcv ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.QTY_REC}, 
	{3, TAB, "onOrder", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", " Qty Rem ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.QTY_REM}, 
	{3, TAB, "ship_qty", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", " Ship Qty ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "9999999.99", (char *)&local_rec.QTY_SH}, 
	{3, TAB, "grs_fob_fgn", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNNN", "          ", 
		" ", " ", " FOB (FGN) ", " ", 
		 YES, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.grs_fgn_cst}, 
	{3, TAB, "fob_fgn", 	 0, 0, DOUBLETYPE, 
		"NNNNNN.NNNN", "          ", 
		" ", "0.00", "", " ", 
		 ND, NO, JUSTRIGHT, "0.00", "99999999.99", (char *)&local_rec.fob_fgn_cst}, 
	{3, TAB, "view_disc", 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "V", " View and Amend Discounts (Y/N) ", 
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.view_disc}, 
	{3, TAB, "case", 	 0, 0, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.case_no}, 
	{3, TAB, "case_no", 	 0, 0, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Case", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&posl_rec.case_no}, 
	{3, TAB, "container", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Container No  ", "Enter Container No. [SEARCH]", 
		ND, NO, JUSTLEFT, "", "", local_rec.containerNo}, 
	{3, TAB, "sup_price", 	 0, 0, DOUBLETYPE, 
		"NNNNNNN.NNNN", "          ", 
		" ", "0.00", "Supp. Price ", " ", 
		NA, NO, JUSTRIGHT, "0.00", "999999999.99", (char *)&local_rec.NEW_COST}, 
	{3, TAB, "UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "UOM.", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.uom}, 
	{3, TAB, "cnv_fct", 	 0, 0, FLOATTYPE, 
		"NNNNNNN.NNNNNN", "          ", 
		" ", "", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.cnv_fct}, 
	{3, TAB, "outer_size", 	 0, 0, FLOATTYPE, 
		"NNNNNNN.NNNNNN", "          ", 
		" ", "", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.outer_size}, 
	{3, TAB, "extended", 	 0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0.00", "  Extended  ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.extended}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

int		virLineCnt;
extern	int		TruePosition;

int		polnCount;
long	*polnArray;
DArray	polnDArray;

long oldHhwhHash	=	0L;

#include <FindSumr.h>
#include <SupPrice.h>
/*
 * Function Declarations
 */
static	BOOL	IsSpaces	(char *);
double 	CalculateDuty 		(void);
double 	CalculateFreight 	(void);
double 	CalculateLicence 	(void);
float 	GetShipping 		(long, long);
int		CalcVirtualLines 	(int);
int 	CheckPorderCost 	(long);
int 	CheckShipmentCost 	(long);
int 	CheckShipmentNo 	(long);
int 	GetWarehouse 		(long);
int 	LoadPoln 			(long);
int 	FindSupplier 		(void);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	win_function 		(int, int, int, int);
void 	CalculateCost 		(double, double);
void 	CalculateTotal 		(void);
void 	CloseDB 			(void);
void 	Confirm 			(void);
void	DrawDiscScn 		(void);
void	DispFields 			(int);
void	InputField 			(int);
void 	OpenDB 				(void);
void	ShufflePolnLines 	(long, int, int);
void 	SrchPocr 			(char *);
void 	SrchPohr 			(char *);
void 	SrchPosh 			(char *);
void 	SrchSkcm 			(char *);
void 	UpdatePosh 			(void);
void 	Update 				(void);
void 	UpdateHeader 		(void);
void 	UpdateDueDate 		(void);
void 	UpdateInis 			(double);
void 	UpdateInsf 			(void);
void 	UpdateOther 		(void);
void 	UpdatePoln 			(long, double, double);
void 	AddPons 			(long);
void 	AddPoli 			(long);
void	ViewDiscounts 		(void);
void	VertLine 			(int, int);
void 	shutdown_prog 		(void);
void 	tab_other 			(int);
int		MessageCnt	=	0;

#ifdef GVISION
#include <disc_win.h>
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
	char	*sptr;
	int	i;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);


	envCrCo 	= atoi (get_env ("CR_CO"));
	envCrFind 	= atoi (get_env ("CR_FIND"));

	/*
	 * Case Number Input PO_CASE_USED = 'Y' and 'N'.
	 */
	sptr = chk_env ("PO_CASE_USED");
	if (sptr == (char *)0)
		strcpy (envPoCaseUsed, "N");
	else
		sprintf (envPoCaseUsed, "%-1.1s", sptr);

	/*
	 * Container used?
	 */
	sptr = chk_env ("PO_CONTAINER");
	envVarPoContainer	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	FLD ("case_no")		= (CASE_USED) ? YES : ND;
	if (envVarPoContainer)
	{
		FLD ("container")	= YES;
		FLD ("sup_item_no")	= ND;
	}
	else
	{
		FLD ("container")	= ND;
		FLD ("sup_item_no")	= YES;
	}

	updateSupplier = chk_inis ();

	/*
	 * Shipment Default. A(ir) / L(and) / S(ea)
	 */
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

	systemDate	=	TodaysDate ();
	sprintf(local_rec.systemDate, "%-10.10s", DateToString(systemDate));
	
	/*
	 * Shipment number is M(anual or S(ystem generated).
	 */
	sptr = get_env ("PO_SH_INPT");
	envPoShInpt = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;

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
	SetSortArray (3, store, sizeof (struct storeRec));
#endif

	_win_func = TRUE;
	no_edit (1);

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = screens [i];

	OpenDB ();

	ArrAlloc (&polnDArray, &polnArray, sizeof (long), 1000);

	strcpy (branchNumber, (envCrCo == 0) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.lastContainerNo, " ");
	while (prog_exit == 0)
	{
		if (restart)
		{
			abc_unlock (posh);
			abc_unlock (pohr);
		}
		/*
		 * Reset control flags.
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_ok		= TRUE;
		init_vars (1);	
		init_vars (2);	
		init_vars (3);	
		lcount [3] = 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		polnCount	=	0;

		while (prog_exit == 0)
		{
			init_ok	=	TRUE;
			MessageCnt	=	0;

			heading (2);
			entry (2);
			if (restart)
			{
				if (!newShipment)
				{
					UpdateHeader ();
					UpdateDueDate ();
				}
				restart = FALSE;
				break;
			}

			if (newOrder)
			{

				last_char = prmptmsg (ML(mlPoMess008), "YyNn", 1, 2);
				if (last_char == 'N' || last_char == 'n')
				{
					init_ok = FALSE;
					heading (3);
					scn_display (3);
					entry (3);
					if (restart)
						break;
				}
				else
					Confirm ();
			}
			else
				scn_display (2);

			edit_all ();
			if (restart)
				break;

			UpdatePosh ();
			Update ();
		}
		if (!restart)
		{
			UpdatePosh ();
			UpdateOther ();
		}
		prog_exit = 0;
	}	/* end of input control loop	*/
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
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	abc_alias (posh2, posh);
	abc_alias (pohr2, pohr);
	abc_alias (pons2, pons);
	abc_alias (poli2, poli);
	abc_alias (sumr2, sumr);
	abc_alias (inis2, inis);
	abc_alias (poln2, poln);

	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (posh,  posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posh2, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posd,  posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" 
															 : "sumr_id_no3");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poln2, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (posl,  posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pogd,  pogd_list, POGD_NO_FIELDS, "pogd_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (pocf,  pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
  	open_rec (pons, pons_list, PONS_NO_FIELDS, "pons_id_no");
  	open_rec (pons2, pons_list, PONS_NO_FIELDS, "pons_id_no");
    open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
    open_rec (poli2, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
    open_rec (skcm, skcm_list, SKCM_NO_FIELDS, "skcm_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{

	abc_fclose (inis);
	abc_fclose (inis2);
	abc_fclose (posh);
	abc_fclose (posh2);
	abc_fclose (posd);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (pohr);
	abc_fclose (pohr2);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (posl);
	abc_fclose (pocr);
	abc_fclose (inmr);
	abc_fclose (pogd);
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (pocf);
	abc_fclose (podt);
	abc_fclose (incc);
	abc_fclose (inum);
  	abc_fclose (pons);
  	abc_fclose (pons2);
    abc_fclose (poli);
    abc_fclose (skcm);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	int		br_no, 
			wh_no;

	if (LCHECK ("br_wh_no"))
	{
		if (prog_status == ENTRY)
			getval (line_cnt);

		tab_other (line_cnt);

		br_no = atoi (local_rec.br_wh_no);
		wh_no = atoi (local_rec.br_wh_no + 3);
		
		sprintf (local_rec.br_no, "%2d", br_no);
		sprintf (local_rec.wh_no, "%2d", wh_no);

		if (dflt_used)
		{
			sprintf (local_rec.br_wh_no, "%2.2s/%2.2s", 
						comm_rec.est_no, 
						comm_rec.cc_no);
		}
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
	
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

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
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (err_str, ML ("Branch %s : %s / Warehouse %s : %s "), 
					local_rec.br_no, esmr_rec.est_name, 
					local_rec.wh_no, ccmr_rec.name);

		print_mess (err_str);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;

		incc_rec.hhbr_hash = poln_rec.hhbr_hash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		if (find_rec (incc, &incc_rec, COMPARISON, "r"))
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf 
		(
			local_rec.br_wh_no, "%2.2s/%2.2s", ccmr_rec.est_no, ccmr_rec.cc_no
		);
		DSP_FLD ("br_wh_no");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Shipment Number.
	 */
	if (LCHECK ("ship_no"))
	{
		if (SRCH_KEY)
		{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (posh_rec.csm_no, "        "))
		{
			if (!envPoShInpt)
			{
				errmess (ML (mlPoMess009));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			newShipment = TRUE;
			return (EXIT_SUCCESS);
		}

		strcpy (posh_rec.co_no, comm_rec.co_no);
		strcpy (posh_rec.csm_no, zero_pad (posh_rec.csm_no, 12));
		cc = find_rec (posh, &posh_rec, COMPARISON, "w");
		if (cc)
		{
			if (envPoShInpt)
				posh_rec.hhsh_hash = 0L;

			abc_unlock (posh);
			newShipment = TRUE;
			return (EXIT_SUCCESS);
		}
		if (posh_rec.status [0] != 'I' && posh_rec.status [0] != 'R')
		{
			abc_unlock (posh);
			print_mess (ML (mlPoMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, posh_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		entry_exit	= TRUE;
		newShipment	= FALSE;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr_code"))
	{
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, posh_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr_rate"))
	{
		if (dflt_used)
			posh_rec.ex_rate = pocr_rec.ex1_factor;

		if (posh_rec.ex_rate <= 0.00)
		{
			print_mess (ML (mlStdMess044));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("curr_rate");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ship_depart"))
	{
		if (prog_status != ENTRY && posh_rec.ship_depart > posh_rec.ship_arrive)
		{
			print_mess (ML (mlPoMess012));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ship_arrive"))
	{
		if (posh_rec.ship_arrive == 0L)
			return (EXIT_SUCCESS);
			
		if (posh_rec.ship_depart > posh_rec.ship_arrive)
		{
			print_mess (ML (mlPoMess012));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("crd_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == EOI)
		{
			restart = TRUE;
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
			return (EXIT_FAILURE);
		}

		if (strcmp (sumr_rec.curr_code, posh_rec.curr_code))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("crd_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pur_ord_no"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pohr_rec.co_no, comm_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.type, "O");
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));

		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "w");
		if (cc || pohr_rec.status [0] == 'D')
		{
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}

		if (pohr_rec.stat_flag [0] == 'Q')
		{
			print_mess (ML (mlPoMess013));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}
	
		if (pohr_rec.drop_ship [0] == 'Y')
		{
			print_mess (ML (mlPoMess014));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}
	
		if (pohr_rec.status [0] == 'U')
		{
			print_mess (ML (mlPoMess015));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}

		strcpy (posd_rec.co_no, comm_rec.co_no);
		posd_rec.hhsh_hash = posh_rec.hhsh_hash;
		posd_rec.hhpo_hash = pohr_rec.hhpo_hash;
		cc = find_rec (posd, &posd_rec, COMPARISON, "w");
		newOrder = (cc) ? TRUE : FALSE;

		if (LoadPoln (pohr_rec.hhpo_hash))
		{
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}
		vars [label ("item_no")].row = lcount [3];

		if (lcount [3] == 0)
		{
			print_mess (ML (mlPoMess004));
			sleep (sleepTime);
			abc_unlock (pohr);
			return (EXIT_FAILURE);
		}

		if (!newOrder)
			entry_exit = TRUE;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ship_qty"))
	{
		if (dflt_used)
			local_rec.QTY_SH = local_rec.QTY_REM;

		if (local_rec.QTY_SH > local_rec.QTY_REM)
		{
			sprintf (err_str, ML (mlPoMess016), local_rec.QTY_REM);
			print_mess (err_str);
			sleep (sleepTime);
		}
		
		if (prog_status != ENTRY)
		{
			local_rec.extended = 	SR.NewPrice * (double) local_rec.QTY_SH;

			SR.extend 	= local_rec.extended;
			DSP_FLD ("extended");
			CalculateTotal ();
		}
		DSP_FLD ("ship_qty");

		if (SR.serialItem [0] == 'Y')
		{
			if (local_rec.QTY_SH != 0.00 && local_rec.QTY_SH != 1.00)
			{
				print_mess (ML (mlStdMess029));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		SR.shipQuantity	=	local_rec.QTY_SH;

		if (vars [label ("case_no")].required == ND)
			return (EXIT_SUCCESS);

		if (poln_rec.case_no > 0)
		{
			posl_rec.case_no = poln_rec.case_no;
			FLD ("case_no") = NI;
		}
		else
			FLD ("case_no") = YES;
		
		DSP_FLD ("case_no");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate FOB Cost (FGN).
	 */
	if (LCHECK ("grs_fob_fgn"))
	{
		double 	oldFob	=	0.00, 
				newFob	=	0.00;

		oldFob	=	twodec (SR.grs_fob);

		if (dflt_used)
			local_rec.grs_fgn_cst = oldFob;

		SR.grs_fob = local_rec.grs_fgn_cst;

		local_rec.NEW_COST	=	CalcNet 
								(
									local_rec.grs_fgn_cst, 
									SR.discArray, 
									poln_rec.cumulative
								);
		SR.NewPrice 			= local_rec.NEW_COST;
		local_rec.fob_fgn_cst	= local_rec.NEW_COST;

		newFob	=	twodec (local_rec.grs_fgn_cst);
		if (oldFob != newFob)
		{
			if (updateSupplier == -1)
				SR.UpdateSupplierFlag = prmpt_inis (0, 23);
			else
				SR.UpdateSupplierFlag = updateSupplier;

			/*
			 * Update inis changes real-time.|
			 */
			if (SR.UpdateSupplierFlag)
				UpdateInis (local_rec.grs_fgn_cst * DPP (SR.cnv_fct));
		}

		local_rec.extended = SR.shipQuantity * SR.NewPrice;

		DSP_FLD ("sup_price");
		DSP_FLD ("extended");
		SR.extend = local_rec.extended;
		CalculateTotal ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate View Discount.
	 */
	if (LCHECK ("view_disc"))
	{
		double wrk_fob;

		if (local_rec.view_disc [0] == 'Y')
		{
			int tmpLcount;

			tmpLcount = lcount [3];

			wrk_fob = local_rec.grs_fgn_cst;

#ifdef GVISION
			discRec.grossPrice		= local_rec.grs_fgn_cst;
			discRec.discArray [0]	= SR.discArray [0];
			discRec.discArray [1]	= SR.discArray [1];
			discRec.discArray [2]	= SR.discArray [2];
			discRec.discArray [3]	= SR.discArray [3];

			ViewDiscounts (DBOX_LFT, DBOX_TOP, poln_rec.cumulative);

			local_rec.grs_fgn_cst	= discRec.grossPrice;
			SR.discArray [0]		= discRec.discArray [0];
			SR.discArray [1]		= discRec.discArray [1];
			SR.discArray [2]		= discRec.discArray [2];
			SR.discArray [3]		= discRec.discArray [3];
#else
			ViewDiscounts ();
#endif	/* GVISION */

			/*
			 * Redraw screens.
			 */
			putval (line_cnt);
			scn_write (3);

			scn_display (3);
			lcount [3] = tmpLcount;
			if (wrk_fob != local_rec.grs_fgn_cst)
			{
				move (0, 23);
				cl_line ();

				/*
				 * Prompt
				 */
				if (updateSupplier == -1)
					SR.UpdateSupplierFlag = prmpt_inis (0, 23);
				else
					SR.UpdateSupplierFlag = updateSupplier;

				/*
				 * Update inis changes real-time.
				 */
				if (SR.UpdateSupplierFlag)
					UpdateInis (local_rec.grs_fgn_cst * DPP (SR.cnv_fct));
			}
		}

		local_rec.NEW_COST	=	CalcNet 
								(
									local_rec.grs_fgn_cst, 
									SR.discArray, 
									poln_rec.cumulative
								);
		local_rec.fob_fgn_cst = local_rec.NEW_COST;
		SR.NewPrice 			= local_rec.NEW_COST;

		DSP_FLD ("sup_price");
		if (prog_status != ENTRY)
		{
			local_rec.extended = SR.shipQuantity * local_rec.NEW_COST;

			DSP_FLD ("extended");
			SR.extend = local_rec.extended;
			CalculateTotal ();

			CalculateCost (local_rec.grs_fgn_cst, local_rec.fob_fgn_cst);
			line_display ();
		}

		SR.grs_fob = local_rec.grs_fgn_cst;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sup_price"))
	{
		if (dflt_used)
		{
			local_rec.NEW_COST = local_rec.fob_fgn_cst;
			SR.NewPrice    	   = local_rec.NEW_COST;
			DSP_FLD ("sup_price");
		}


		if (local_rec.NEW_COST != local_rec.fob_fgn_cst)
		{
			if (updateSupplier == -1)
				SR.UpdateSupplierFlag = prmpt_inis (0, 23);
			else
				SR.UpdateSupplierFlag = updateSupplier;

			/*
			 * Update inis changes real-time.
			 */
			if (SR.UpdateSupplierFlag)
				UpdateInis (local_rec.fob_fgn_cst * DPP (SR.cnv_fct));
		}

		local_rec.extended = SR.shipQuantity * SR.NewPrice;

		DSP_FLD ("extended");
		SR.extend = local_rec.extended;
		CalculateTotal ();

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Container Number.
	 */
	if (LCHECK ("container"))
	{
		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.containerNo, local_rec.lastContainerNo);
		
		if (IsSpaces (local_rec.containerNo))
			return (EXIT_SUCCESS);
		
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		strcpy (skcm_rec.container, local_rec.containerNo);
		cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTrMess083));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].containerNo, 	skcm_rec.container);
		strcpy (store [line_cnt].containerDesc, skcm_rec.desc);
		strcpy (local_rec.lastContainerNo, local_rec.containerNo);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

/*
 * Confirm Complete order and allow user to Change.
 */
void
Confirm (void)
{
	scn_set (3);

	print_at (2, 1, ML (mlPoMess023));

	for (line_cnt = 0;line_cnt < lcount [3];line_cnt++)
	{
		getval (line_cnt);

		if (poln_rec.case_no > 0)
			posl_rec.case_no = poln_rec.case_no;

		local_rec.QTY_SH	=	local_rec.QTY_REM;
		SR.shipQuantity		=	local_rec.QTY_SH;
		local_rec.extended 	= 	local_rec.QTY_SH * local_rec.NEW_COST;

		SR.extend 	= local_rec.extended;
		CalculateTotal ();
		putval (line_cnt);
	}
}

void
CalculateTotal (void)
{
	register	int	i;

	posd_rec.total = 0.00;

	for (i = 0;i < lcount [3];i++)
		posd_rec.total += store [i].extend;
}

/*
 * Load Existing or new lines.
 */
int
LoadPoln (
	long	hhpoHash)
{
	float	ShipQty 	= 0.00;
	float	StdCnvFct 	= 1.00, 
			PurCnvFct 	= 1.00, 
			CnvFct		= 1.00;

	polnLineNo	=	0;

	scn_set (3);
	lcount [3] = 0;
	posd_rec.total = 0.00;

	poln_rec.hhpo_hash = hhpoHash;
	poln_rec.line_no = 0;

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		if (poln_rec.line_no > polnLineNo)
			polnLineNo	=	poln_rec.line_no;

		/*
		 * Subtract Quantity already on Shipment.
		 */
		ShipQty	=	GetShipping 
					(
						poln_rec.hhpl_hash, 
						posh_rec.hhsh_hash
					);

		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;	
		}
		
		strcpy (store [lcount [3]].serialItem, inmr_rec.serial_item);
	
		if (inmr_rec.outer_size <= 0.00)
			inmr_rec.outer_size = 1.00;
		
		local_rec.outer_size = inmr_rec.outer_size;

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		sprintf (store [lcount [3]].uom, "%-4.4s", inum_rec.uom);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;

		local_rec.cnv_fct		=	CnvFct;

		local_rec.fob_fgn_cst 	= 	twodec (poln_rec.fob_fgn_cst);
		local_rec.fob_fgn_cst 	/= 	DPP (local_rec.cnv_fct);
		local_rec.fob_fgn_cst 	/= 	local_rec.outer_size;

		local_rec.grs_fgn_cst 	= twodec (poln_rec.grs_fgn_cst);
		local_rec.grs_fgn_cst 	/= 	DPP (local_rec.cnv_fct);
		local_rec.grs_fgn_cst 	/= 	local_rec.outer_size;

		strcpy (local_rec.uom, inum_rec.uom);
		strcpy (posl_rec.co_no, comm_rec.co_no);
		posl_rec.hhsh_hash = posh_rec.hhsh_hash;
		posl_rec.hhpl_hash = poln_rec.hhpl_hash;
		cc = find_rec (posl, &posl_rec, COMPARISON, "r");
		if (cc)
		{
			posl_rec.case_no 	= poln_rec.case_no;
			local_rec.QTY_SH	= 0.00;
			local_rec.extended 	= 0.00;
			store [lcount [3]].extend = 0.00;
			local_rec.NEW_COST	= local_rec.fob_fgn_cst;
			strcpy (store [lcount [3]].containerNo, " ");
			strcpy (store [lcount [3]].containerDesc, " ");
			strcpy (local_rec.containerNo, " ");
		}
		else
		{
			local_rec.QTY_SH		= 	posl_rec.ship_qty * local_rec.cnv_fct;
			local_rec.NEW_COST		= 	local_rec.fob_fgn_cst;
			local_rec.extended 		=  	(double) local_rec.QTY_SH * 
										local_rec.NEW_COST;
			store [lcount [3]].extend = local_rec.extended;
			posd_rec.total 			+= local_rec.extended;

			strcpy (local_rec.containerNo, posl_rec.container);
			strcpy (store [lcount [3]].containerNo, posl_rec.container);
			strcpy (skcm_rec.co_no, comm_rec.co_no); 
			strcpy (skcm_rec.container, local_rec.containerNo);
			cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
			if (cc)
				strcpy (store [lcount [3]].containerDesc, " ");
			else
				strcpy (store [lcount [3]].containerDesc, skcm_rec.desc);
		}
		local_rec.QTY_ORD	=	poln_rec.qty_ord * local_rec.cnv_fct;
		local_rec.QTY_REC	=	poln_rec.qty_rec * local_rec.cnv_fct;
		local_rec.QTY_REM	=	local_rec.QTY_ORD - local_rec.QTY_REC;
		local_rec.QTY_REM	-=	(ShipQty * local_rec.cnv_fct);

		store [lcount [3]].onOrder	=	local_rec.QTY_REM;
		store [lcount [3]].shipQuantity	=	local_rec.QTY_SH;

		store [lcount [3]].NewPrice =	local_rec.NEW_COST;

		if (local_rec.QTY_REM <= 0.00 && local_rec.QTY_SH <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		virLineCnt = CalcVirtualLines (FALSE);
		if (virLineCnt >= MAXLINES - 1)
		{
			print_mess (ML (mlStdMess158));
			sleep (sleepTime);
			clear_mess ();
			scn_set (2);
			return (EXIT_FAILURE);
		}
		store [lcount [3]].UpdateSupplierFlag 	= 	FALSE;
		store [lcount [3]].cnv_fct	=	local_rec.cnv_fct;
		strcpy (store [lcount [3]].item_desc, poln_rec.item_desc);
		strcpy (store [lcount [3]].serialNo, poln_rec.serial_no);

		inis2_rec.hhsu_hash	= sumr_rec.hhsu_hash;
		inis2_rec.hhbr_hash	= poln_rec.hhbr_hash;
		strcpy (inis2_rec.co_no, "  ");
		strcpy (inis2_rec.br_no, "  ");
		strcpy (inis2_rec.wh_no, "  ");
		cc = find_rec ("inis2", &inis2_rec, GTEQ, "r");
		if (cc || inis2_rec.hhsu_hash	!= sumr_rec.hhsu_hash ||
				   inis2_rec.hhbr_hash	!= poln_rec.hhbr_hash)
	   	{
			strcpy (inis2_rec.sup_part, " ");
		}

		GetWarehouse (poln_rec.hhcc_hash);

		strcpy (store [lcount [3]].supplierPart, inis2_rec.sup_part);
		store [lcount [3]].grs_fob			=	local_rec.grs_fgn_cst;
		store [lcount [3]].discArray [0]	=	poln_rec.reg_pc;
		store [lcount [3]].discArray [1]	=	poln_rec.disc_a;
		store [lcount [3]].discArray [2]	=	poln_rec.disc_b;
		store [lcount [3]].discArray [3]	=	poln_rec.disc_c;
		strcpy (local_rec.view_disc, "N");
		putval (lcount [3]++);

		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	vars [scn_start].row = lcount [3];

	scn_set (2);

	return (EXIT_SUCCESS);
}

/*
 * Check how many are already Shipping.
 */
float	
GetShipping (
	long	hhplHash, 
	long	hhshHash)
{
	float	qty_shipping = 0.00;

	abc_selfield (posl, "posl_hhpl_hash");

	posl_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (posl, &posl_rec, GTEQ, "r");
	while (!cc && posl_rec.hhpl_hash == hhplHash)
	{
		if (posl_rec.hhsh_hash != hhshHash)
			qty_shipping += posl_rec.ship_qty;

		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}

	abc_selfield (posl, "posl_id_no");
	return (qty_shipping);
}

/*
 * Update shipment header.
 */
void
UpdateHeader (void)
{
	cc = abc_update (posh, &posh_rec);
	if (cc)
		file_err (cc, posh, "DBUPDATE");
}
/*
 * Update all relevent details.
 */
void
UpdatePosh (void)
{
	char	temp_store [13];

	clear ();

	if (newShipment)
		strcpy (posh_rec.status, "I");

	if (newShipment && envPoShInpt)
	{
		open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
        	if (cc)
               		file_err (cc, comr, "DBFIND");

		while (CheckShipmentNo (++comr_rec.ship_no) == 0);

		cc = abc_update (comr, &comr_rec);
        	if (cc)
               		file_err (cc, comr, "DBUPDATE");

		abc_fclose (comr);
		posh_rec.hhsh_hash = comr_rec.ship_no;
		sprintf (temp_store, "%ld", comr_rec.ship_no);
		sprintf (posh_rec.csm_no, "%s", zero_pad (temp_store, 12));
	}
	if (newShipment)
	{
		sprintf (err_str, ML ("Adding Shipment No. %s..."), posh_rec.csm_no);
		print_at(MessageCnt++, 0, "%s", err_str);

		strcpy (posh_rec.co_no, comm_rec.co_no);
		cc = abc_add (posh, &posh_rec);
		if (cc)
			file_err (cc, posh, "DBADD");

		strcpy (posh_rec.co_no, comm_rec.co_no);
		cc = find_rec (posh, &posh_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, posh, "DBFIND");
	}
	else
	{
		sprintf (err_str, ML ("Updating Shipment No. %s ... "), 
										posh_rec.csm_no);
		print_at(MessageCnt++, 0, "%s", err_str);
		fflush (stdout);

		cc = abc_update (posh, &posh_rec);
		if (cc)
			file_err (cc, posh, "DBUPDATE");
	}
}

void
Update (void)
{
	if (newOrder)
	{
		sprintf (err_str, ML ("Adding Purchase Order %s to Shipment ...Press any key to continue. "), pohr_rec.pur_ord_no);
		PauseForKey (MessageCnt++, 0, err_str, 0);

		strcpy (posd_rec.co_no, comm_rec.co_no);
		posd_rec.hhsh_hash = posh_rec.hhsh_hash;
		posd_rec.hhpo_hash = pohr_rec.hhpo_hash;

		cc = abc_add (posd, &posd_rec);
		if (cc)
			file_err (cc, posd, "DBADD");
	}
	else
	{
		sprintf (err_str, ML ("Updating Purchase Order %s on Shipment ... Press any key to continue."), pohr_rec.pur_ord_no);
		PauseForKey (MessageCnt++, 0, err_str, 0);

		cc = abc_update (posd, &posd_rec);
		if (cc)
			file_err (cc, posd, "DBUPDATE");
	}

	scn_set (3);

	abc_selfield (poln, "poln_hhpl_hash");

	splitLines = 0;
	for (line_cnt = 0;line_cnt < lcount [3];line_cnt++) 
	{
		getval (line_cnt);

		strcpy (posl_rec.co_no, comm_rec.co_no);
		posl_rec.hhsh_hash = posh_rec.hhsh_hash;
		posl_rec.hhpl_hash = poln_rec.hhpl_hash;

		cc = find_rec (posl, &posl_rec, COMPARISON, "u");
		if (cc)
		{
			posl_rec.ship_qty	=	local_rec.QTY_SH / local_rec.cnv_fct;

			if (posl_rec.ship_qty <= 0.00)
				continue;

			strcpy (posl_rec.co_no, comm_rec.co_no);
			posl_rec.hhsh_hash 	= posh_rec.hhsh_hash;
			posl_rec.hhpl_hash 	= poln_rec.hhpl_hash;
			posl_rec.hhpo_hash 	= poln_rec.hhpo_hash;

			posl_rec.sup_price 	= local_rec.NEW_COST;
			posl_rec.sup_price 	*= DPP (local_rec.cnv_fct);
			strcpy (posl_rec.container, local_rec.containerNo);

			cc = abc_add (posl, &posl_rec);
			if (cc)
				file_err (cc, posl, "DBADD");

			ArrChkLimit (&polnDArray, polnArray, polnCount);
			polnArray [polnCount++] = poln_rec.hhpl_hash;
		}
		else
		{
			getval (line_cnt);

			posl_rec.ship_qty	=	local_rec.QTY_SH / local_rec.cnv_fct;

			posl_rec.sup_price 	= 	local_rec.NEW_COST;
			posl_rec.sup_price 	*= 	DPP (local_rec.cnv_fct);
			strcpy (posl_rec.container, local_rec.containerNo);

			cc = abc_update (posl, &posl_rec);
			if (cc)
				file_err (cc, posl, "DBUPDATE");
			
			ArrChkLimit (&polnDArray, polnArray, polnCount);
			polnArray [polnCount++] = poln_rec.hhpl_hash;
		}
		/*
		 * Update poln for all records changed on screen poln.
		 */
		UpdatePoln 
		(
			poln_rec.hhpl_hash, 
			SR.grs_fob, 
			(posl_rec.sup_price / DPP (local_rec.cnv_fct))
		);
		/*
		 * Delete out shipment line allocation if ship qty <= 0.00
		 */
		if (posl_rec.ship_qty <= 0.00)
		{
			cc = abc_delete (posl);
			if (cc)
				file_err (cc, posl, "DBUPDATE");
		}
	}
	abc_selfield (poln, "poln_id_no");
	abc_unlock (posd);

	pohr_rec.hhsh_hash = posh_rec.hhsh_hash;
	cc = abc_update (pohr, &pohr_rec);
	if (cc)
		file_err (cc, pohr, "DBUPDATE");

	/*
	 * Advide the user of split lines.
	 */
	if (splitLines > 0)
	{
		sprintf (err_str, 
				 "Please note that %d line%s been split due to under supply. ", 
				 splitLines, 
				(splitLines > 1) ? "s have" : " has");
		rv_pr (err_str, 0, 2, 0);
	}
	newShipment = FALSE;
}
/*
 * Update purchase order lines with exchange rate on shipment header.
 */
void
UpdateOther (void)
{
	int		i;

	float	StdCnvFct 	= 1.00, 
			PurCnvFct 	= 1.00, 
			CnvFct		= 1.00;

	clear ();

	abc_selfield (poln, "poln_hhpl_hash");
	abc_selfield (sumr, "sumr_hhsu_hash");
	abc_selfield (pohr, "pohr_hhpo_hash");
	abc_selfield (posl, "posl_hhpl_hash");

	rv_pr (ML (mlPoMess021), 0, 1, 0);

	for (i = 0; i < polnCount; i++)
	{
		poln_rec.hhpl_hash	=	polnArray [i];
		if (find_rec (poln, &poln_rec, EQUAL, "u"))
		{
			abc_unlock (poln);
			continue;
		}
		posl_rec.hhpl_hash	=	polnArray [i];
		if (find_rec (posl, &posl_rec, EQUAL, "r"))
		{
			abc_unlock (poln);
			continue;
		}
		if (poln_rec.status [0] == 'R')
		{
			abc_unlock (poln);
			continue;
		}
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		if (find_rec (pohr, &pohr_rec, EQUAL, "r"))
		{
			abc_unlock (poln);
			cc = find_rec (posl, &posl_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
		{
			abc_unlock (poln);
			continue;
		}
		/*
		 * Find inventory supplier records.
		 */
		inis_rec.hhbr_hash = poln_rec.hhbr_hash;
		inis_rec.hhsu_hash = pohr_rec.hhsu_hash;
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
		if (cc)
			foundSupplier = FALSE;
		else
			foundSupplier = TRUE;
		
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec ("inmr", &inmr_rec, EQUAL, "r");
		if (cc)
			inmr_rec.outer_size	=	1.00;
		
		if (inmr_rec.outer_size <= 0.00)
			inmr_rec.outer_size	=	1.00;

		local_rec.outer_size = inmr_rec.outer_size;

		if (poln_rec.status [0] == 'R')
		{
			abc_unlock (poln);
			continue;
		}
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		if (find_rec (pohr, &pohr_rec, EQUAL, "r"))
		{
			abc_unlock (poln);
			cc = find_rec (posl, &posl_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
		{
			abc_unlock (poln);
			continue;
		}
		/*
		 * Find inventory supplier records.
		 */
		inis_rec.hhbr_hash = poln_rec.hhbr_hash;
		inis_rec.hhsu_hash = pohr_rec.hhsu_hash;
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
		if (cc)
			foundSupplier = FALSE;
		else
			foundSupplier = TRUE;
		
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;

		local_rec.cnv_fct		=	CnvFct;

		local_rec.fob_fgn_cst	=	poln_rec.fob_fgn_cst;
		local_rec.fob_fgn_cst	/=	DPP (CnvFct);
		local_rec.fob_fgn_cst	/=	local_rec.outer_size;

		local_rec.grs_fgn_cst	=	poln_rec.grs_fgn_cst;
		local_rec.grs_fgn_cst	/=	DPP (CnvFct);
		local_rec.grs_fgn_cst	/=	local_rec.outer_size;
									

		/*
		 * Calculate cost for line.
		 */
		CalculateCost (local_rec.grs_fgn_cst, local_rec.fob_fgn_cst);
	
		poln_rec.exch_rate = posh_rec.ex_rate;
		poln_rec.ship_no   = posh_rec.hhsh_hash;
		poln_rec.due_date  = posh_rec.ship_arrive;

		cc = abc_update (poln, &poln_rec);
		if (cc)
			file_err (cc, poln, "DBUPDATE");

		if (strcmp (poln_rec.serial_no, serialSpace))
			UpdateInsf ();

		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}
	abc_selfield (sumr, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");
	abc_selfield (poln, "poln_id_no");
	abc_selfield (pohr, "pohr_id_no");
	abc_selfield (posl, "posl_id_no");
}

void
UpdateDueDate (void)
{
	abc_selfield(poln, "poln_hhpl_hash");
	strcpy(posl_rec.co_no, posh_rec.co_no);
	posl_rec.hhsh_hash = posh_rec.hhsh_hash;
	posl_rec.hhpl_hash = 0L;
	cc = find_rec (posl, &posl_rec, GTEQ, "r");
	while (!cc && posl_rec.hhsh_hash == posh_rec.hhsh_hash
			&& !strcmp(posl_rec.co_no, posh_rec.co_no))
	{
		poln_rec.hhpl_hash = posl_rec.hhpl_hash;
		cc = find_rec (poln, &poln_rec, COMPARISON, "u");
		if (!cc)
		{
			poln_rec.due_date = posh_rec.ship_arrive;
			cc = abc_update (poln, &poln_rec);
			if (cc)
				file_err (cc, poln, "DBUPDATE");
		}
		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}
	abc_selfield(poln, "poln_id_no");
}
/*
 * Update Pre-receipt Serial item.
 */
void
UpdateInsf (void)
{
	inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
	if (find_rec (inmr, &inmr_rec, EQUAL, "r"))
		return;
	
	if (inmr_rec.serial_item [0] != 'Y')
		return;

	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	if (find_rec (incc, &incc_rec, COMPARISON, "r"))
		return;

	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no2");

	insf_rec.hhwh_hash = oldHhwhHash;
	sprintf (insf_rec.serial_no, "%-25.25s", poln_rec.serial_no);
	cc = find_rec (insf, &insf_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (insf);
		abc_fclose (insf);
		return;
	}
	insf_rec.exch_rate   = poln_rec.exch_rate;
	insf_rec.fob_fgn_cst = poln_rec.grs_fgn_cst;
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

	cc = abc_update (insf, &insf_rec);
	if (cc)
		file_err (cc, insf, "DBUPDATE");

	abc_fclose (insf);
	return;
}
int
CheckShipmentNo (
	long	shipmentNumber)
{
	char	TempShipment [13];

	sprintf (TempShipment, "%012ld", shipmentNumber);
	strcpy (posh2_rec.co_no, comm_rec.co_no);
	strcpy (posh2_rec.csm_no, TempShipment);
	return (find_rec (posh2, &posh2_rec, COMPARISON, "r"));
}

/*
 * Search for shipment number.
 */
void
SrchPosh (
	char    *key_val)
{
	_work_open (12,0,50);
	save_rec ("#Shipment No.", "#Vessel Name           Arrival Date");
	strcpy (posh_rec.co_no, comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", " ");
	cc = find_rec (posh, &posh_rec, GTEQ, "r");
	while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no))
	{                        
		if ((posh_rec.status [0] == 'I' || 
		      posh_rec.status [0] == 'R') && 
		      !strncmp (posh_rec.csm_no, key_val, strlen (key_val)))
		{
			sprintf 
			(
				err_str, "%20.20s  %s", 
				posh_rec.vessel, 
				DateToString (posh_rec.ship_arrive)
			);
			cc = save_rec (posh_rec.csm_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (posh, &posh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (posh_rec.co_no,  comm_rec.co_no);
	strcpy (posh_rec.csm_no, temp_str);
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, posh, "DBFIND");
}

/*
 * Search for curr_code
 */
void
SrchPocr (
	char    *key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.", "#Currency Description");
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strcmp (pocr_rec.co_no, comm_rec.co_no) && 
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
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*
 * Search for purchase order number.
 */
void
SrchPohr (
	char	*keyValue)
{
	int		NonZeroFound	=	0;

	_work_open (15,0,50);

	save_rec ("#P/O No.        ", "#Supp. |   Order Terms.     |Date Raised|                   Contact              ");

	abc_selfield (poln, "poln_id_no");

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		   		  !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
				  pohr_rec.type [0] == 'O' &&
				  pohr_rec.hhsu_hash == sumr_rec.hhsu_hash &&
		   		  !strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue)))
	{
		if (pohr_rec.drop_ship [0] == 'Y')
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}

		NonZeroFound 		= FALSE;
		poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
		poln_rec.line_no 	= 0;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
			if ((twodec (poln_rec.qty_ord - poln_rec.qty_rec)) > 0.00)
			{
				NonZeroFound = TRUE;
				break;
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		if ((pohr_rec.status [0] == 'O' || pohr_rec.status [0] == 'R') 
				&& NonZeroFound == TRUE)
		{
			sprintf 
			(
				err_str, 
				"%s|%s|%-10.10s | %s",
				sumr_rec.crd_no,
				pohr_rec.term_order, 
				DateToString (pohr_rec.date_raised),
				pohr_rec.contact
			);
			cc = save_rec (pohr_rec.pur_ord_no ,err_str);
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
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no, "%15.15s", temp_str);
	cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pohr, "DBFIND");
}

/*
 * Update Purchase order line with new cost.
 */
void
UpdatePoln (
	long	hhplHash, 
	double	grossPrice, 
	double	updatePrice)
{
	float	remainOnOrder	=	0.00;
	double	exchRate		=	1.00;

	poln_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln, &poln_rec, EQUAL, "u");
	if (cc)
		file_err (cc, poln, "DBFIND");

	incc_rec.hhbr_hash = poln_rec.hhbr_hash;
	incc_rec.hhcc_hash = poln_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (!cc)
		oldHhwhHash = incc_rec.hhwh_hash;

	if (posl_rec.ship_qty <= 0.00)
	{
		if (poln_rec.ship_no == posh_rec.hhsh_hash)
		{
			poln_rec.ship_no   = 0L;

			cc = abc_update (poln, &poln_rec);
			if (cc)
				file_err (cc, poln, "DBUPDATE");
		}
		else
			abc_unlock (poln);

		if (strcmp (poln_rec.serial_no, serialSpace))
			UpdateInsf ();
		
		return;
	}
	/*
	 * Find inventory supplier records.
	 */
	inis_rec.hhbr_hash = poln_rec.hhbr_hash;
	inis_rec.hhsu_hash = pohr_rec.hhsu_hash;
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
	if (cc)
	{
		foundSupplier = FALSE;
		abc_unlock (inis);
	}
	else
	{
		if (SR.UpdateSupplierFlag)
			UpdateInis (updatePrice * DPP (local_rec.cnv_fct));
		else
			abc_unlock (inis);
		foundSupplier = TRUE;
	}
	/*
	 * Calculate cost for line.
	 */
	CalculateCost (grossPrice, updatePrice);

	exchRate 	  		= poln_rec.exch_rate;
	poln_rec.exch_rate = posh_rec.ex_rate;
	poln_rec.ship_no   = posh_rec.hhsh_hash;
	poln_rec.hhcc_hash = local_rec.hhcc_hash;

	poln_rec.exch_rate = (double) posh_rec.ex_rate;
	poln_rec.hhcc_hash = local_rec.hhcc_hash;

	poln_rec.reg_pc 	= SR.discArray [0];
	poln_rec.disc_a 	= SR.discArray [1];
	poln_rec.disc_b 	= SR.discArray [2];
	poln_rec.disc_c 	= SR.discArray [3];

	poln_rec.fob_fgn_cst = local_rec.fob_fgn_cst;
	poln_rec.fob_fgn_cst *= DPP (local_rec.cnv_fct);
	poln_rec.fob_fgn_cst *= local_rec.outer_size;

	poln_rec.grs_fgn_cst = local_rec.grs_fgn_cst;
	poln_rec.grs_fgn_cst *= DPP (local_rec.cnv_fct);
	poln_rec.grs_fgn_cst *= local_rec.outer_size;

	cc = abc_update (poln, &poln_rec);
	if (cc)
		file_err (cc, poln, "DBUPDATE");

	/*
	 * If the shipment contains less than the quantity   
	 * ordered then split the line into:
	 * 1) The orginal line but updated with the quantity 
	 *    being shipped.                                 
	 * 2) A new line containing the balance not on this  
	 *    shipment.                                      
	 */
	remainOnOrder = 0.0;

	if (posl_rec.ship_qty < (local_rec.QTY_REM / local_rec.cnv_fct))
	{
		remainOnOrder  		= local_rec.QTY_REM / local_rec.cnv_fct;
		remainOnOrder 	   -= posl_rec.ship_qty;
		poln_rec.qty_ord	= posl_rec.ship_qty;
	}

	/*
	 * Calculate cost for line.
	 */
	CalculateCost (grossPrice, updatePrice);

	poln_rec.exch_rate	= posh_rec.ex_rate;
	poln_rec.ship_no	= posh_rec.hhsh_hash;
	poln_rec.hhcc_hash	= local_rec.hhcc_hash;

	poln_rec.reg_pc = SR.discArray [0];
	poln_rec.disc_a = SR.discArray [1];
	poln_rec.disc_b = SR.discArray [2];
	poln_rec.disc_c = SR.discArray [3];

	poln_rec.fob_fgn_cst	= 	local_rec.fob_fgn_cst;
	poln_rec.fob_fgn_cst	*= 	DPP (local_rec.cnv_fct);
	poln_rec.fob_fgn_cst	*= 	local_rec.outer_size;

	poln_rec.grs_fgn_cst	=	local_rec.grs_fgn_cst;
	poln_rec.grs_fgn_cst	*=	DPP (local_rec.cnv_fct);
	poln_rec.grs_fgn_cst	*=	local_rec.outer_size;

	cc = abc_update (poln, &poln_rec);
	if (cc)
		file_err (cc, poln, "DBUPDATE");

	if (strcmp (poln_rec.serial_no, serialSpace))
		UpdateInsf ();

	/*
	 * Split line.
	 */
	if (remainOnOrder > 0.0)
	{
		ShufflePolnLines (poln_rec.hhpo_hash, poln_rec.line_no, polnLineNo);

		/*
		 * Create new poln line for split quantity.
		 */
		poln_rec.line_no	= poln_rec.line_no + 1;
		polnLineNo++;
		poln_rec.qty_ord	= remainOnOrder;
		poln_rec.qty_rec	= 0.0;
		poln_rec.exch_rate	= exchRate;
		poln_rec.ship_no	= 0L;
		poln_rec.case_no	= 0L;

		poln_rec.fob_fgn_cst	= 	local_rec.fob_fgn_cst;
		poln_rec.fob_fgn_cst	*= 	DPP (local_rec.cnv_fct);
		poln_rec.fob_fgn_cst	*=	local_rec.outer_size;


		poln_rec.grs_fgn_cst	= 	local_rec.grs_fgn_cst;
		poln_rec.grs_fgn_cst	*= 	DPP (local_rec.cnv_fct);
		poln_rec.grs_fgn_cst	*= 	local_rec.outer_size;

		cc = abc_add (poln, &poln_rec);
		if (cc)
			file_err (cc, poln, "DBADD");

		/*
		 * To get poln_hhpl_hash of the added record.
		 */
		abc_selfield (poln, "poln_id_no");
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		abc_selfield (poln, "poln_hhpl_hash");

		AddPons(hhplHash);
		AddPoli(hhplHash);
		splitLines++;
	}
}
void
ShufflePolnLines (
	long	hhpoHash, 
	int		polnLineNo, 
	int		lastLine)
{
	int		i;
	for (i = lastLine; i > polnLineNo; i--)
	{
		poln2_rec.hhpo_hash	=	hhpoHash;
		poln2_rec.line_no	=	i;
		cc = find_rec (poln2, &poln2_rec, COMPARISON, "u");
		if (cc)
			break;

		poln2_rec.line_no	=	i + 1;

		cc = abc_update (poln2, &poln2_rec);
		if (cc)
			file_err (cc, poln2, "DBUPDATE");
	}
}

/*
 * Calculate cost of purchase order line.
 */
void
CalculateCost (
	double	grossPrice, 
	double	updatePrice)
{
	double	cif_cost = 0.00;

	int	no_recalc = FALSE;

	local_rec.grs_fgn_cst	=	grossPrice;
	local_rec.fob_fgn_cst	=	updatePrice;

	no_recalc += CheckPorderCost 	(poln_rec.hhpo_hash);
	no_recalc += CheckShipmentCost 	(posh_rec.hhsh_hash);

	if (!no_recalc)
		poln_rec.frt_ins_cst = CalculateFreight ();

	/*
	 * Calc Default Duty
	 */
	if (!no_recalc)
	{
		/*
		 * Duty is Default Duty
		 */
		poln_rec.duty = CalculateDuty ();
		poln_rec.duty = poln_rec.duty;
	}

	/*
	 * Calculate CIF FGN
	 */
	cif_cost 	= local_rec.fob_fgn_cst;
	cif_cost 	*= local_rec.outer_size;
	cif_cost 	*= DPP (local_rec.cnv_fct);

	/*
	 * Calculate CIF
	 */
	if (posh_rec.ex_rate != 0.00)
		poln_rec.fob_nor_cst = cif_cost / posh_rec.ex_rate;
	else
		poln_rec.fob_nor_cst = 0.00;

	/*
	 * FOB Nor
	 */
	poln_rec.fob_nor_cst = poln_rec.fob_nor_cst;
	poln_rec.fob_nor_cst += poln_rec.frt_ins_cst;
	poln_rec.fob_nor_cst = poln_rec.fob_nor_cst;

	/*
	 * Calculate Licence
	 */
	if (!no_recalc)
	{
		poln_rec.licence = CalculateLicence ();
		poln_rec.licence *= poln_rec.fob_nor_cst;
		poln_rec.licence = DOLLARS (poln_rec.licence);
		poln_rec.licence = poln_rec.licence;
	}

	/*
	 * Calculate Landed Cost  
	 */
	poln_rec.land_cst =  poln_rec.fob_nor_cst + 
						 poln_rec.duty + 
						 poln_rec.licence + 
						 poln_rec.lcost_load;

	poln_rec.land_cst = poln_rec.land_cst;
}

/*
 * Calculate value of freight.
 */
double	
CalculateFreight (void)
{
	double	value = 0.00;
	double	frt_conv = 0.00;

	strcpy (pocf_rec.co_no, pohr_rec.co_no);
	strcpy (pocf_rec.code, sumr_rec.ctry_code);
	if (find_rec (pocf, &pocf_rec, COMPARISON, "r"))
		return (0.00);
	
	/*
	 * Calculate Freight
	 */
	frt_conv = pocf_rec.freight_load;

	/*
	 * Freight is a Unit value.
	 */
	if (pocf_rec.load_type [0] == 'U')
		value = frt_conv;

	/*
	 * Freight is a Percentage.
	 */
	if (pocf_rec.load_type [0] == 'P')
	{
		value = local_rec.fob_fgn_cst;
		value *= local_rec.outer_size;
		value *= DPP (local_rec.cnv_fct);
		value *= frt_conv;
		value /= 100.00;
	}

	if (posh_rec.ex_rate != 0.00)
		value /= posh_rec.ex_rate;

	value = value;

	return (value);
}

/*
 * Calculate value of duty. 
 */
double	
CalculateDuty (void)
{
	double	duty;

	strcpy (podt_rec.co_no, pohr_rec.co_no);
	if (!foundSupplier)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
			return (0.00);

		strcpy (podt_rec.code, inmr_rec.duty);
	}
	else
		strcpy (podt_rec.code, inis_rec.duty);

	if (find_rec (podt, &podt_rec, COMPARISON, "r"))
		return (0.00);

	switch (podt_rec.duty_type [0])
	{
	case	'D':
		return (podt_rec.im_duty);

	case	'P':
		duty 	=	local_rec.fob_fgn_cst;
		duty	*= 	local_rec.outer_size;
		duty 	*=	DPP (local_rec.cnv_fct);
		duty 	*=	podt_rec.im_duty;
		duty 	/=	100.00;
		if (posh_rec.ex_rate != 0.00)
			duty /= posh_rec.ex_rate;

		duty = duty;
		return (duty);

	default:
		return (0.00);
	}
}

/*
 * Calculate value of Licence.
 */
double	
CalculateLicence (void)
{
	if (!foundSupplier)
	{
		open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_hhlc_hash");

		polh_rec.hhlc_hash	=	poln_rec.hhlc_hash;
		cc = find_rec (polh, &polh_rec, COMPARISON, "r");
		if (cc)
		{
			abc_fclose (polh);
			return (0.00);
		}
	}
	else
		return (0.00);

	abc_fclose (polh);

	return ((double) polh_rec.ap_lic_rate);
}

/*
 * Check is costing exists for Purchase Order.
 */
int
CheckPorderCost (
	long	hhpoHash)
{
	abc_selfield (pogd, "pogd_id_no3");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhpo_hash = hhpoHash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd, &pogd_rec, GTEQ, "r");
	if  (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) &&
		     pogd_rec.hhpo_hash == hhpoHash)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
/*
 * Check is costing exists for Shipment.
 */
int
CheckShipmentCost (
	long	hhshHash)
{
	abc_selfield (pogd, "pogd_id_no2");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhsh_hash = hhshHash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd, &pogd_rec, GTEQ, "r");
	if  (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) &&
		     pogd_rec.hhsh_hash == hhshHash)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}
/*
 * Update value of Licence.
 */
void
UpdateInis (
	double	updateValue)
{
	cc = FindSupplier ();
	if (cc)
	{
		abc_unlock (inis);
		return;
	}
	inis_rec.fob_cost	= updateValue;
	inis_rec.lcost_date = systemDate;
	cc = abc_update (inis, &inis_rec);
	if (cc)
		file_err (cc, inis, "DBUPDATE");
}

/*
 * Display Infor for lines while in edit mode.
 */
void
tab_other (
	int		iline)
{
	if (cur_screen != 3)
		return;

	print_at 
	(
		3, 
		0, 
		ML ("Line : %03d / Description : %-40.40s / Base UOM: %-4.4s / Supplier Part Number : %s"), 
		iline + 1, 
		store [iline].item_desc, 
		store [iline].uom,
		store [iline].supplierPart
	);
	if (!strcmp (store [iline].serialNo, serialSpace))
	{
		print_at 
		(
			4, 
			0, 
			ML ("%13.13sContainer   : %-15.15s / Description : %-40.40s %45.45s"),
			" ",
			store [iline].containerNo, 
			store [iline].containerDesc,
			" "
		);
	}
	else
	{
		print_at 
		(
			4, 
			0, 
			ML ("%13.13sContainer   : %-15.15s / Description : %-40.40s / Serial No : %-25.25s"),
			" ",
			store [iline].containerNo, 
			store [iline].containerDesc,
			store [iline].serialNo
		);
	}
	return;
}

int
FindSupplier (void)
{
	/*
	 * Find inventory supplier records.
	 */
	inis_rec.hhbr_hash = poln_rec.hhbr_hash;
	inis_rec.hhsu_hash = pohr_rec.hhsu_hash;
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
	return (cc);
}

int
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
			file_err (cc, ccmr, "DBFIND");
	
		sprintf 
		(
			local_rec.br_wh_no, "%2.2s/%2.2s", 
			ccmr_rec.est_no, 
			ccmr_rec.cc_no
		);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	}
	else
	{
		if (!warehouseSelected)
		{
			abc_selfield (ccmr, "ccmr_hhcc_hash");
			warehouseSelected = TRUE;
		}
		ccmr_rec.hhcc_hash	=	hhccHash;
		if (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"))
		{
			abc_selfield (ccmr, "ccmr_id_no");
			return (GetWarehouse (0L));
		}
		sprintf 
		(
			local_rec.br_wh_no, 
			"%2.2s/%2.2s", 
			ccmr_rec.est_no, 
			ccmr_rec.cc_no
		);
		local_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	}
	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		rv_pr (ML (mlPoMess022), 55, 0, 1);
		line_at (1,0,132);

		switch (scn)
		{
		case	1:
			box (0, 2, 132, 17);
			line_at (7,1,131);
			line_at (10,1,131);
			line_at (13,1,131);
			line_at (16,1,131);
			break;

		case	2:
			box (0, 3, 132, 6);
			line_at (6,1,131);
			line_at (20,0,132);
			print_at (2, 46, ML ("%R Press Window Activate-Key 1 for current Suppliers and Purchase-Orders on Shipment. "));
			break;

		case	3:
			line_at (20,0,132);
			break;

		default:
			break;
		}

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_at (23,0,132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
win_function (
	int		field, 
	int 	row, 
	int 	scn, 
	int 	status)
{
	int	got_one = FALSE;

	if (scn != 2)
		return (EXIT_SUCCESS);

	strcpy (posd2_rec.co_no, comm_rec.co_no);
	posd2_rec.hhsh_hash = posh_rec.hhsh_hash;
	posd2_rec.hhpo_hash = 0L;
	cc = find_rec (posd, &posd2_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (posd2_rec.co_no, comm_rec.co_no) &&
		posd2_rec.hhsh_hash == posh_rec.hhsh_hash
	)
	{
		if (!got_one)
		{
			Dsp_open (0, 2, 16);
			Dsp_saverec ("Supplier| Purchase Order  | Invoice Number  | Comment                                 ");
			Dsp_saverec ("");
			Dsp_saverec (" [NEXT] [PREV] [EDIT/END]");
			got_one = TRUE;
		}
		pohr2_rec.hhpo_hash	=	posd2_rec.hhpo_hash;
		cc = find_rec (pohr2, &pohr2_rec, EQUAL, "r");
		if (!cc)
		{
			sumr2_rec.hhsu_hash	=	pohr2_rec.hhsu_hash;
			cc = find_rec (sumr2, &sumr2_rec, EQUAL, "r");
			if (!cc)
			{
				sprintf 
				(
					err_str, 
					" %-6.6s ^E %-15.15s ^E %-15.15s ^E %-40.40s", 
					sumr2_rec.crd_no, 
					pohr2_rec.pur_ord_no, 
					posd2_rec.inv_no, 
					posd2_rec.comment
				);
				Dsp_saverec (err_str);
			}
		}
		cc = find_rec (posd, &posd2_rec, NEXT, "r");
	}

	if (!got_one)
		return (EXIT_SUCCESS);

	Dsp_srch ();
	Dsp_close ();
    return (EXIT_SUCCESS);
}

int
CalcVirtualLines (int entryMode)
{
	int idx;
	int actLines = 0;
	int virLines = 0;

	if (entryMode)
		actLines = line_cnt + 1;
	else
		actLines = lcount [3];
	for (idx = 0; idx < actLines; idx++)
	{
		if (store [idx].serialItem [0] == 'Y' || 
			store [idx].serialItem [0] == 'y')
			virLines += store [idx].onOrder;
		else
			virLines++;
	}
	return (virLines);
}

#ifndef GVISION
/*
 * Allow editing of dicounts for current line.
 */
void
ViewDiscounts (void)
{
	int key;
	int currFld;
	int tmpLineCnt;
	double oldFobFgn;
	float oldDisc [4];

	/*
	 * Save old values.
	 */
	oldFobFgn = local_rec.grs_fgn_cst;
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
			local_rec.grs_fgn_cst = oldFobFgn;
			SR.discArray [0] = oldDisc [0];
			SR.discArray [1] = oldDisc [1];
			SR.discArray [2] = oldDisc [2];
			SR.discArray [3] = oldDisc [3];
			restart = TRUE;
			break;

		case FN3:
			tmpLineCnt = line_cnt;
			heading (3);
			line_cnt = tmpLineCnt;
			lcount [3] = (prog_status == ENTRY) ? line_cnt + 1 : lcount [3];
			scn_display (3);
			DrawDiscScn ();
			DispFields (currFld);
			break;
		}

		DispFields (currFld);
		if (restart)
			break;
	}
	DispFields (currFld);
	restart = FALSE;
}

void
DrawDiscScn (void)
{
	int i;
	int fldWid;
	int headXPos;

	/*
	 * Draw box.
	 */
	cl_box (DBOX_LFT, DBOX_TOP, DBOX_WID, DBOX_DEP);

	/*
	 * Draw middle horizontal line.
	 */
	move (DBOX_LFT + 1, DBOX_TOP + 2);
	line (DBOX_WID - 1);
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
	 */
	sprintf (err_str, " Discounts Are %s ", 
			(poln_rec.cumulative) ? "Cumulative" : "Absolute");
	headXPos = DBOX_LFT + (DBOX_WID - strlen (err_str)) / 2;
	rv_pr (err_str, headXPos, DBOX_TOP, 1);

	/*
	 * Draw prompts.
	 */
	for (i = 0; i < 6; i++)
	{
		fldWid = strlen (discScn [i].fldPrompt);
		print_at (DBOX_TOP + 1, 
				  DBOX_LFT + discScn [i].xPos + 1, 
				  " %-*.*s ", 
				  fldWid, 
				  fldWid, 
				  discScn [i].fldPrompt);
	}
}

void
VertLine (
	int		xPos, 
	int		yPos)
{
	move (xPos, yPos);
	PGCHAR (8);

	move (xPos, yPos + 1);
	PGCHAR (5);

	move (xPos, yPos + 2);
	PGCHAR (7);

	move (xPos, yPos + 3);
	PGCHAR (5);

	move (xPos, yPos + 4);
	PGCHAR (9);
}

void
DispFields (
	int	rvsField)
{
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [0].xPos + 2, 
			  "%11.2f", local_rec.grs_fgn_cst);
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [1].xPos + 2, 
			  "%6.2f", SR.discArray [0]);
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [2].xPos + 2, 
			  "%6.2f", SR.discArray [1]);
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [3].xPos + 2, 
			  "%6.2f", SR.discArray [2]);
	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [4].xPos + 2, 
			  "%6.2f", SR.discArray [3]);

	local_rec.NEW_COST	= 	CalcNet 
							(
								local_rec.grs_fgn_cst, 
								SR.discArray, 
								poln_rec.cumulative
							);
	SR.NewPrice =	local_rec.NEW_COST;
	local_rec.fob_fgn_cst = local_rec.NEW_COST;

	print_at (DBOX_TOP + 3, DBOX_LFT + discScn [5].xPos + 2, 
			  "%11.2f", local_rec.NEW_COST);

	/*
	 * Print highlighted field.
	 */
	switch (rvsField)
	{
	case 0:
		sprintf (err_str, "%11.2f", local_rec.grs_fgn_cst);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		sprintf (err_str, "%6.2f", SR.discArray [rvsField - 1]);
		break;
	}
	rv_pr (err_str, DBOX_LFT + discScn [rvsField].xPos + 2, DBOX_TOP + 3, 1);
}

void
InputField (
	int		fld)
{
	int fieldOk;
	double tmpDbl;

	crsr_on ();

	fieldOk = FALSE;
	while (!fieldOk)
	{
		fieldOk = TRUE;
		switch (fld)
		{
		case 0:
			tmpDbl = getdouble (DBOX_LFT + discScn [fld].xPos + 2, 
								DBOX_TOP + 3, 
								discScn [fld].fldMask);
			local_rec.grs_fgn_cst = tmpDbl;

			break;

		case 1:
		case 2:
		case 3:
		case 4:
			SR.discArray [fld - 1] =
				getfloat (DBOX_LFT + discScn [fld].xPos + 2, 
						  DBOX_TOP + 3, 
						  discScn [fld].fldMask);
			if (SR.discArray [fld - 1] > 99.99)
			{
				print_mess (ML (mlStdMess120));
				sleep (sleepTime);
				clear_mess ();
				fieldOk = FALSE;
			}
			break;
		}
	}
	crsr_off ();
}
#endif	/* GVISION	*/

/*
 * Add purchase order non stock lines file if poln is splitted.
 */
void
AddPons (
	long  hhplHash)                 
{
	pons2_rec.hhpl_hash      = hhplHash;
	pons2_rec.line_no        = 0;
	cc = find_rec (pons2, &pons2_rec, GTEQ, "r");
	while (!cc && pons2_rec.hhpl_hash == hhplHash)
	{
		pons_rec.hhpl_hash      = poln_rec.hhpl_hash;
		pons_rec.line_no        = pons2_rec.line_no;
		strcpy(pons_rec.desc, pons2_rec.desc);
		cc = abc_add (pons, &pons_rec);
		if (cc)
			file_err (cc, pons, "DBADD");

		cc = find_rec (pons2, &pons2_rec, NEXT, "r");
	}
}

/*
 * Add purchase order line information file if poln is splitted.
 */
void             
AddPoli (
	long  hhplHash)                 
{
 	poli2_rec.hhpl_hash = hhplHash;

	cc = find_rec (poli2, &poli2_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (poli_rec.comment, poli2_rec.comment);
 		poli_rec.hhpl_hash 	= poln_rec.hhpl_hash;
		poli_rec.cont_date 	= poli2_rec.cont_date;
		poli_rec.ship_date 	= poli2_rec.ship_date;
		poli_rec.eta_date 	= poli2_rec.eta_date; 
		poli_rec.inst_code 	= poli2_rec.inst_code; 
		cc = abc_add (poli, &poli_rec); 
		if (cc) 
			file_err (cc, poli, "DBADD"); 
	}
}

/*
 * Search for skcm.
 */
void
SrchSkcm (
 char	*keyValue)
{
	_work_open (15,0,40);
	save_rec ("#Container", "#Description");
	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", keyValue);
	cc = find_rec (skcm, &skcm_rec, GTEQ, "r");
	while (!cc && !strcmp (skcm_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm_rec.container, keyValue, strlen (keyValue)))
	{
		cc = save_rec (skcm_rec.container, skcm_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm, &skcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", temp_str);
	cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, skcm, "DBFIND");
}

static BOOL
IsSpaces (
	char	*str)
{ 
	/*
	 * Return TRUE if str contains only white space or nulls
	 */
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
}
