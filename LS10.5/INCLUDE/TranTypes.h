#ifndef	_tranTypes_h
#define	_tranTypes_h
/*
 *
 *******************************************************************************
 *	$Log: TranTypes.h,v $
 *	Revision 5.0  2001/06/19 06:51:25  cha
 *	LS10-5.0 New Release as of 19 JUNE 2001
 *	
 *	Revision 4.1  2001/04/19 12:54:20  scott
 *	New Files.
 *	
 *	
 */
	struct
	{
		int		typeCode;
		char	*typeShort;
		char	*typeMeduim;
		char	*typeLong;

	} tranTypes [] = {

static char *tr_type [] =
{
	1, 	"STK BAL", "Stock  Balance ", "Stock Balance                      "
	2, 	"STK REC", "Stock  Receipt ", "Stock Receipt                      "
	3, 	"STK ISS", "Stock  Issue   ", "Stock Issue                        "
	4, 	"STK ADJ", "Stk. Adjustment", "Stock Adjustment                   "
	5, 	"STK PUR", "Stock Purchase ", "Stock Purchase                     "
	6, 	"INVOICE", "Invoice        ", "Sales Invoice                      "
	7, 	"CREDIT ", "Credit Note    ", "Sales Credit Note                  "
	8, 	"PRD ISS", "Production Iss.", "Production Issue                   "
	9, 	"STK TRN", "Stock Transfer ", "Stock Transfer                     "
	10, "PRD REC", "Production Rec.", "Production Receipt                 "
	11, "STK W/O"  "Stk. Works Ord.", "Stock Works Order                  "
	12, "DD PUR."  "DD - Purchase  ", "Direct Delivery - Purchase         "
	13, "DD SALE"  "DD - Sales     ", "Direct Delivery - Sales            "
	14, "SALE RG"  "Sales Reg.     ", "Sale Registration                  "
	15, "WC - VP"  "W.Claim Ven Pro", "Warranty Claim - Vendor Pro-Forma  "
	16, "WC - VA"  "W.Claim Ven Acc", "Warranty Claim - Vendor Accepted   "
	17, "WC - VR"  "W.Claim Ven Rej", "Warranty Claim - Vendor Rejected   "
	18, "WC - SA"  "W.Claim Sup Acc", "Warranty Claim - Supplier Accepted "
	19, "WC - SR"  "W.Claim Sup Rej", "Warranty Claim - Supplier Rejected "
	20, "CR->DEA"  "Crd. to Dealer.", "Credit To dealer                   "
	21, "IN->SUP"  "Inv. to Supp.  ", "Invoice to Supplier.               "
	0,"",""}
	};

#endif	/* _tranTypes_h */
