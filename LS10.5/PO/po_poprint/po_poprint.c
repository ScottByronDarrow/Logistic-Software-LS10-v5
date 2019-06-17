/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_poprint.c,v 5.2 2001/08/09 09:15:57 scott Exp $
|  Program Name  : ( po_poprint.c )                                   |
|  Program Desc  : ( po_poprint                                     ) |
|---------------------------------------------------------------------|
|  Date Written  : (MM/DD/YYYY     | Author      :                    |
|---------------------------------------------------------------------|
| $Log: po_poprint.c,v $
| Revision 5.2  2001/08/09 09:15:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:08  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.4  2001/02/21 03:49:17  scott
| Updated to make code change to print correct quantity on returns.
| Purchase orders are quantity order - receipts, returns should only
| use quantity order.
|
| Revision 3.3  2001/02/16 04:59:43  scott
| Updated to remove debug checks
|
| Revision 3.2  2001/02/16 04:58:12  scott
| Updated after testing
|
| Revision 3.1  2001/02/16 03:39:36  scott
| New 'C' version that replaces C++ version that is not database independent
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_poprint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_poprint/po_poprint.c,v 5.2 2001/08/09 09:15:57 scott Exp $";

#define bool int
#define true 	1
#define false 	0

#include	<pslscr.h>	
#include	<string.h>
#include	<FormatP.h>
#include "schema"

/*
 *	Alternate bodies
 */
static	const	char	*BodyInexDesc		= "inex-desc-body",
						*BodyInstruction	= "instruction-body",
						*po_poprint_fp		= "po_poprint.fp",
						*po_cprint_fp		= "po_cprint.fp";

/*
 *	List of program generated value-registers (aside from column values)
 */
static const char	*EvalFobFgnCst	= "eval_fob_fgn_cst",
					*EvalQuantity	= "eval_quantity",
					*EvalExtended	= "eval_extended",
					*EvalSubTotal	= "eval_subtotal";

	int		printerNumber	=	1;

	struct	pohrRecord	pohr_rec;
	struct	sumrRecord	sumr_rec;
	struct	esmrRecord	esmr_rec;
	struct	comrRecord	comr_rec;
	struct	polnRecord	poln_rec;
	struct	inmrRecord	inmr_rec;
	struct	inumRecord	inum_rec;
	struct	inisRecord	inis_rec;
	struct	ccmrRecord	ccmr_rec;
	struct 	inexRecord 	inex_rec;

	int		purchaseOrder = TRUE;

/*=================================
| Local Record for Screen Editing |
=================================*/
struct
{
	int		iDummy;				/* used as place holder */
} local_rec;

/*=============
| Table Names |
=============*/
static	char	*data = "data",
				pathName [128];

FILE *fout;

/*==================
| Global Variables |
==================*/
bool multiprint;

/*====================
| Standard Functions |
====================*/
void 	OpenDB 				(void);	
void 	CloseDB 			(void);	
void	StartProgram 		(void);	
void 	shutdown_prog 		(void);	
void 	Dump 				(long);
void 	InexDescription 	(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{

	char	mode [16];
	long	hhpoHash;
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "po_cprint"))
		purchaseOrder = FALSE;
	else
		purchaseOrder = TRUE;
	
	/*---------------------------------------------------------
	| Begin program, call initialization, abort if init fails |
	---------------------------------------------------------*/
	StartProgram ();
			
	sptr	=	OpenSpecial 
				(
					"LAYOUT", 
					(purchaseOrder) ? po_poprint_fp : po_cprint_fp
				);
	if (!sptr)
	{
		fprintf 
		(
			stderr, 
			"Layout file [%s] not found",
			(purchaseOrder) ? po_poprint_fp : po_cprint_fp
		);
		return (EXIT_FAILURE);
	}
	strcpy (pathName, sptr);
	if (scanf ("%d", &printerNumber) == EOF || scanf ("%s", mode) == EOF)
		return (EXIT_FAILURE);
	
	if (mode[0] == 'M')
		multiprint = true;

	/*
	 *  Read in stream of hhpo-hash from stdin.
	 *
	 *  For Multi-print:
	 *      0
	 *      hashes
	 *      0
	 *
	 *  For Single-print
	 *      hashes
	 *      0
	 */
	while (scanf ("%ld", &hhpoHash) != EOF)
	{
		if (hhpoHash)
			Dump (hhpoHash);
	}
	
	/*-----------------------------
	| Clean up and return SUCCESS |
	-----------------------------*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=============================================
| Open the databases and the necessary tables |
=============================================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------
	| open tables |
	-------------*/
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
}

/*===========================================================
| Close the tables that were opened, and close the database |
===========================================================*/
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (inis);
	abc_fclose (inum);
	abc_fclose (inmr);
	abc_fclose (poln);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (sumr);
	abc_fclose (pohr);

	abc_dbclose (data);
}

/*=========================
| Program initializations |
=========================*/
void
StartProgram (void)
{
	OpenDB ();
}

/*==================
| Program clean up |
==================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
Dump (
	long	hhpoHash)
{
	/*
	 *	Open a line to format-p and dump information to it
	 */

	static	int	formatPOpen	=	FALSE;

    double		pageSubtotal 		= 0.00,
				stdCnv				= 0.00, 
				purCnv				= 0.00,
				lineQuantity		= 0.00,
				qtyOrder			= 0.00,
   				fobFgnCst			= 0.00,
   				convFactor			= 0.00,
				lineFobCost			= 0.00,
         		lineExtended		= 0.00;

    int 		cc;
    float		outerSize			= 0.00;

	pohr_rec.hhpo_hash = hhpoHash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		return;

	sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	if (cc)
		return;
		
	strcpy (esmr_rec.co_no, pohr_rec.co_no);
	strcpy (esmr_rec.est_no, pohr_rec.br_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		return;

	strcpy (comr_rec.co_no, pohr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 *	Worthwhile submitting something to format-p
	 */
	if (formatPOpen == FALSE)
	{
		fout = FormatPOpenLpNo (pathName , printerNumber, NULL);
		formatPOpen = TRUE;
	}
	if (multiprint)
		FormatPReset (fout);

	/*
	 * For print out header
	 */
	FormatPSubmitTable (fout, pohr);
	FormatPSubmitTable (fout, sumr);
	FormatPSubmitTable (fout, esmr);
	FormatPSubmitTable (fout, comr);

	/*
	 *	Dump line info
	 */

	poln_rec.hhpo_hash = hhpoHash;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		purCnv = 0.00;
		stdCnv = 0.00;

		FormatPSubmitTable (fout, poln);

		inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
		if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}

		FormatPSubmitTable (fout, inmr);

		ccmr_rec.hhcc_hash = poln_rec.hhcc_hash;
		if (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"))
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		inis_rec.hhbr_hash = poln_rec.hhbr_hash;
		inis_rec.hhsu_hash = pohr_rec.hhsu_hash;
		strcpy (inis_rec.co_no, pohr_rec.co_no); 
		strcpy (inis_rec.br_no, pohr_rec.br_no);
		strcpy (inis_rec.wh_no, ccmr_rec.cc_no);
		if (find_rec (inis, &inis_rec, COMPARISON, "r"))
		{
			inum_rec.hhum_hash = inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (!cc)
			{
				FormatPSubmitTable (fout, inum);
				stdCnv = (double) inum_rec.cnv_fct;
			}
		}
		else
		{
			FormatPSubmitTable (fout, inis);
			inum_rec.hhum_hash = inis_rec.sup_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (!cc)
			{
				FormatPSubmitTable (fout, inum);
				purCnv = (double) inum_rec.cnv_fct;
			}
		}
	  	FormatPSubmitTable (fout, inis);
	  	
	  	inum_rec.hhum_hash = inmr_rec.std_uom;
	  	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	  	if (!cc)
	  	{
			stdCnv = inum_rec.cnv_fct;
			if (stdCnv == 0.00)
				stdCnv = 1.00;
	  	}

		FormatPSubmitTable (fout, inis);
		inum_rec.hhum_hash = poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (!cc)
		{
			FormatPSubmitTable (fout, inum);
			purCnv = inum_rec.cnv_fct;

			if (purCnv == 0.00)
				purCnv = 1.00;
		}

		/*
		 * Calculate Quantity & Extended and Sum
			conversion_factor(inum_rec.cnv_fct),
		 */
		outerSize 		= 	inmr_rec.outer_size;
		if (purchaseOrder == TRUE)
			qtyOrder 		= 	poln_rec.qty_ord - poln_rec.qty_rec;
		else
			qtyOrder 		= 	poln_rec.qty_ord;

		convFactor 		= 	(double) stdCnv / 
							(double) purCnv;
		lineFobCost 	= 	poln_rec.fob_fgn_cst;
		lineQuantity  	= 	qtyOrder * 
							convFactor; 

		if (outerSize == 0)
			outerSize = 1.00;
			
		fobFgnCst 		= 	lineFobCost / 
							(double) outerSize / 
							(double) convFactor; 

		FormatPSubmitMoney (fout, EvalFobFgnCst, CENTS (fobFgnCst));
		lineExtended  = (double) lineQuantity * (double) fobFgnCst;
		pageSubtotal += lineExtended;

		FormatPSubmitMoney (fout, EvalQuantity, CENTS (lineQuantity));
		FormatPSubmitMoney (fout, EvalExtended, CENTS (lineExtended));

	   /*
	 	*	Evaluated values for page trailer
	 	*/
		FormatPSubmitMoney (fout, EvalSubTotal, pageSubtotal);

		/*
		 *	Here endeth the major part
		 */
		 FormatPBatchEnd (fout);

		/*	
		 *	Print out inventory extra description lines
		 */
		InexDescription (inmr_rec.hhbr_hash);

		cc = find_rec (poln, &poln_rec, NEXT, "r");	
	}

	/*
	 *	Add special instruction onto the detail line
	 */
	FormatPBody 		(fout, BodyInstruction);
	FormatPSubmitTable 	(fout, pohr);
	FormatPBatchEnd 	(fout);
	FormatPBody 		(fout, "body");		/* revert back to std body */

	/*
	*/
	if (!multiprint)
		FormatPClose (fout);
}

/*	
 *	Print out inventory extra description lines
 */
void
InexDescription (
	long	hhbrHash)
{
	int	descLength	=	0.00;

	inex_rec.hhbr_hash = hhbrHash;
	inex_rec.line_no = 0;
	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	if (!cc)
	{
		FormatPBody (fout, BodyInexDesc);
		do
		{
			descLength	=	(int) strlen (clip (inex_rec.desc));
			if (descLength)
				FormatPBody (fout, BodyInexDesc);

			FormatPSubmitTable (fout, inex);
			FormatPBatchEnd (fout);
			cc = find_rec (inex, &inex_rec, NEXT, "r");

		}	while (!cc && inex_rec.hhbr_hash == hhbrHash);

		FormatPBody (fout, "body");
	}
}
