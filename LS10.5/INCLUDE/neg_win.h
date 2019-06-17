/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( new_win.c      )                                 |
|  Program Desc  : ( Pricing & Discounting Negotiation Window.    )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|---------------------------------------------------------------------|
|  Date Written  : (08/11/93)      |  Author     : Campbell Mander.   |
|---------------------------------------------------------------------|
|  Date Modified : (21/03/94)      | Modified by : Dirk Heinsius.     |
|  Date Modified : (31/03/94)      | Modified by : Dirk Heinsius.     |
|  Date Modified : (24/04/96)      | Modified by : Scott B Darrow.    |
|                                                                     |
|  Comments      :                                                    |
|  (21/03/94)    : HGP  9501 - Limit discounts to the range -99.99    |
|                : to 99.99.                                          |
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (24/04/96)    : FRA - Updated to fix outer size problem.           |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include <getnum.h>

/*----------------------------------------
| Default positions for Negotiation Box. |
----------------------------------------*/
#define	NBOX_WID	125
#define	NBOX_DEP	3

#define	NMAX_FLDS	12

/*--------------------------------------------
| Structure used for pop-up discount screen. |
--------------------------------------------*/
struct
{
	char	fldPrompt[13];
	int		xPos;
	int		fldEdit;
	char	fldMask[12];
} negoScn[] = {
	{"Qty Ord", 		0,   1, "NNNN.NN"},
	{"Qty B/O", 		10,  1, "NNNN.NN"},
	{"Gross Price",		20,  1, "NNNNNN.NN"},
	{"Reg %", 			34,  1, "NNN.NN"},
	{"Disc A", 			43,  1, "NNN.NN"},
	{"Disc B", 			52,  1, "NNN.NN"},
	{"Disc C", 			61,  1, "NNN.NN"},
	{"Cum", 			70,  0, "AAA"},			/* Display Only */
	{"Net Price", 		76,  1, "NNNNNN.NN"},
	{"Extended", 		88,  0, "NNNNNNNN.NN"},	/* Display Only */
	{"Marg %",	 		102, 1, "NNNN.NN"},
	{"Margin  ", 		112, 1, "NNNNNN.NN"},
	{"", 			    0,   0, ""},
};

/*---------------------------------------------------------
| Structure used to hold variables maintained by NegPrice |
---------------------------------------------------------*/
struct	{
							/*--------------------------------------*/
	char	itemNo[17];		/* Item Number 							*/
	char	itemDesc[41];	/* Item Description 					*/
	float	qOrd;			/* Quantity Ordered 					*/
	float	qBord;			/* Quantity Backordered 				*/
	float	regPc;			/* Regulatory Percent 					*/
	float	discArray[3];	/* Discount Array (A, B and C) 			*/
	double	grossPrice;		/* Gross Price 							*/
	double	salePrice;		/* Gross Price Less Regulatory Percent 	*/
	double	netPrice;		/* Net Price 							*/
	double	margCost;		/* Cost for margins 					*/
	int		cumDisc;		/* Cumulative Discounts ? 				*/
	double	discAmt;		/* Discount Amount 						*/
	double	extVal;			/* Extended Value 						*/
	double	margVal;		/* Margin Value 						*/
	double	margPc;			/* Margin Percent 						*/
	float	outer_size;		/* Outer Size.							*/
							/*--------------------------------------*/
} negoRec;

/*----------------------
| Function Prototypes. |
----------------------*/
float	DiscNet (void);
float	DiscPc (void);
float	DiscVal (void);
static int	LclSpecValid (int);
extern	int		heading	(int);
void	NegPrice (int, int, char *, char *, int, int);
void	DispFields (int, int, int);
void	_InputField (int, int, int);
void	RecalcFigures (int);
void	DrawNegScn (int, int, char *, char *);
void	VertLine (int, int);
void	RecalcNet (void);

/*---------------------------------------------
| Allow editing of dicounts for current line. |
---------------------------------------------*/
void
NegPrice (int xPos, int yPos, char *itemNo, char *itemDesc, int cumDisc, int currScrn)
{
	int		key;
	int		currFld;
	int		tmpLineCnt;
	float	oldQOrd;
	float	oldQBord;
	float	oldRegPc;
	float	oldDisc[3];
	double	oldGross;
	double	oldSale;
	double	oldNet;

	/*--------------------
	| Initialise values. |
	--------------------*/
	sprintf(negoRec.itemNo, "%-16.16s", itemNo);
	sprintf(negoRec.itemDesc, "%-40.40s", itemDesc);
	negoRec.cumDisc   = cumDisc;
	negoRec.extVal    = (double)0.00;
	negoRec.netPrice  = (double)0.00;
	negoRec.margVal   = (double)0.00;
	negoRec.margPc    = (float)0.00;

	/*------------------
	| Save old values. |
	------------------*/
	oldQOrd    = negoRec.qOrd;
	oldQBord   = negoRec.qBord;
	oldRegPc   = negoRec.regPc;
	oldDisc[0] = negoRec.discArray[0];
	oldDisc[1] = negoRec.discArray[1];
	oldDisc[2] = negoRec.discArray[2];
	oldGross   = negoRec.grossPrice;
	oldSale    = negoRec.salePrice;
	oldNet     = negoRec.netPrice;

	/*----------------------
	| Draw box and fields. |
	----------------------*/
	DrawNegScn(xPos, yPos, itemNo, itemDesc);

	/*-----------------------------------------------
	| Allow cursor movement and selection for edit. |
	| Exit without change on FN1.                   |
	| Exit saving changes on FN16.                  |
	-----------------------------------------------*/
	crsr_off();
	currFld = 0;
	restart = FALSE;
	DispFields(xPos, yPos, currFld);
	while ((key = getkey()) != FN16)
	{
		switch (key)
		{
		case BS:
		case LEFT_KEY:
		case UP_KEY:
			currFld--;
			if (currFld < 0)
				currFld = NMAX_FLDS - 1;

			/*--------------------------------
			| Check for Display Only fields. |
			--------------------------------*/
			while (negoScn[currFld].fldEdit == 0)
			{
				currFld--;
				if (currFld < 0)
					currFld = NMAX_FLDS - 1;
			}

			break;

		case DOWN_KEY:
		case RIGHT_KEY:
		case ' ':
			currFld++;	
			if (currFld >= NMAX_FLDS)
				currFld = 0;

			/*--------------------------------
			| Check for Display Only fields. |
			--------------------------------*/
			while (negoScn[currFld].fldEdit == 0)
			{
				currFld++;
				if (currFld >= NMAX_FLDS)
					currFld = 0;
			}

			break;

		case '\r':
			_InputField(xPos, yPos, currFld);
			RecalcFigures(currFld);
			break;

		case FN1:
			/*---------------------
			| Restore old values. |
			---------------------*/
			negoRec.qOrd         = oldQOrd;
			negoRec.qBord        = oldQBord;
			negoRec.regPc        = oldRegPc;
			negoRec.discArray[0] = oldDisc[0];
			negoRec.discArray[1] = oldDisc[1];
			negoRec.discArray[2] = oldDisc[2];
			negoRec.grossPrice   = oldGross;
			negoRec.salePrice    = oldSale;
			negoRec.netPrice     = oldNet;

			restart = TRUE;
			break;

		case FN3:
			if (currScrn != 0)
			{
				tmpLineCnt = line_cnt;
				heading(currScrn);
				line_cnt = tmpLineCnt;
				lcount[ currScrn ] = (prog_status == ENTRY) ? line_cnt + 1 : lcount[ currScrn ];
				scn_display(currScrn);
			}
			DrawNegScn(xPos, yPos, itemNo, itemDesc);
			DispFields(xPos, yPos, currFld);
			break;

		default:
			putchar(BELL);
			break;
		}

		DispFields(xPos, yPos, currFld);
		if (restart)
			break;
	}
}

/*------------------------------
| Draw the negotiation window. |
------------------------------*/
void
DrawNegScn (int xPos, int yPos, char *itemNo, char *itemDesc)
{
	int		i;
	int		fldWid;
	int		headXPos;

	/*-----------
	| Draw box. |
	-----------*/
	cl_box(xPos, yPos, NBOX_WID, NBOX_DEP);

	/*------------------------------
	| Draw middle horizontal line. |
	------------------------------*/
	move(xPos + 1, yPos + 2);
	line(NBOX_WID - 1);
	move(xPos, yPos + 2);
	PGCHAR(10);
	move(xPos + NBOX_WID - 1, yPos + 2);
	PGCHAR(11);

	/*-------------------------------
	| Draw vertical dividing lines. |
	-------------------------------*/
	for (i = 1; i < NMAX_FLDS; i++)
		VertLine(xPos + negoScn[i].xPos, yPos);

	/*--------------------------------------
	| Display Item Number and Description. |
	--------------------------------------*/
	sprintf(err_str, " %-16.16s - %-40.40s", itemNo, itemDesc);
	headXPos = xPos + (NBOX_WID - strlen(err_str)) / 2;
	rv_pr(err_str, headXPos, yPos, 1);

	/*---------------
	| Draw prompts. |
	---------------*/
	for (i = 0; i < NMAX_FLDS; i++)
	{
		fldWid = strlen(negoScn[i].fldPrompt);
		print_at(yPos + 1,
				 xPos + negoScn[i].xPos + 1,
				 " %-*.*s ",
				 fldWid,
				 fldWid,
				 negoScn[i].fldPrompt);
	}
}

/*---------------------------------------
| Display fields in negotiation window. |
---------------------------------------*/
void
DispFields (int xPos, int yPos, int rvsField)
{
	print_at(yPos + 3,xPos + negoScn[0].xPos + 2,
			 		"%7.2f", negoRec.qOrd);
	print_at(yPos + 3,xPos + negoScn[1].xPos + 2,
			 		"%7.2f", negoRec.qBord);
	print_at(yPos + 3,xPos + negoScn[2].xPos + 2,
			 		"%9.2f", DOLLARS(negoRec.grossPrice));
	print_at(yPos + 3,xPos + negoScn[3].xPos + 2,
			 		"%6.2f", negoRec.regPc);
	print_at(yPos + 3,xPos + negoScn[4].xPos + 2,
			 		"%6.2f", negoRec.discArray[0]);
	print_at(yPos + 3,xPos + negoScn[5].xPos + 2,
			 		"%6.2f", negoRec.discArray[1]);
	print_at(yPos + 3,xPos + negoScn[6].xPos + 2,
			 		"%6.2f", negoRec.discArray[2]);
	print_at(yPos + 3,xPos + negoScn[7].xPos + 2,
			 		"%-3.3s", (negoRec.cumDisc == TRUE) ? "Yes" : "No ");

	/*--------------------------------------
	| Calculate margin and extended value. |
	--------------------------------------*/
	RecalcNet();
	
	print_at(yPos + 3,xPos + negoScn[8].xPos + 2,
			 		"%9.2f", DOLLARS(negoRec.netPrice));
	print_at(yPos + 3,xPos + negoScn[9].xPos + 2,
			 		"%11.2f", DOLLARS(negoRec.extVal));
	print_at(yPos + 3,xPos + negoScn[10].xPos + 2,
			 		"%7.2f", negoRec.margPc);
	print_at(yPos + 3,xPos + negoScn[11].xPos + 2,
			 		"%9.2f", DOLLARS(negoRec.margVal));

	/*--------------------------
	| Print highlighted field. |
	--------------------------*/
	switch (rvsField)
	{
	case 0:
		sprintf(err_str, "%7.2f", negoRec.qOrd);
		break;

	case 1:
		sprintf(err_str, "%7.2f", negoRec.qBord);
		break;

	case 2:
		sprintf(err_str, "%9.2f", DOLLARS(negoRec.grossPrice));
		break;

	case 3:
		sprintf(err_str, "%6.2f", negoRec.regPc);
		break;

	case 4:
	case 5:
	case 6:
		sprintf(err_str, "%6.2f", negoRec.discArray[rvsField - 4]);
		break;

	case 8:
		sprintf(err_str, "%9.2f", DOLLARS(negoRec.netPrice));
		break;

	case 10:
		sprintf(err_str, "%7.2f", negoRec.margPc);
		break;

	case 11:
		sprintf(err_str, "%9.2f", DOLLARS(negoRec.margVal));
		break;
	}
	rv_pr(err_str, xPos + negoScn[rvsField].xPos + 2, yPos + 3, 1);
}

/*---------------------
| Input chosen field. |
---------------------*/
void
_InputField (int xPos, int yPos, int fld)
{
	int		fieldOk;
	float	oldQty;
	float	oldPc;
	double	oldVal;
	float	newQty;
	float	newPc;
	double	newVal;

	crsr_on();

	fieldOk = FALSE;
	while (!fieldOk)
	{
		fieldOk = TRUE;
		switch (fld)
		{
		case 0:		/* Quantity Ordered */
			oldQty = negoRec.qOrd;
			newQty = getfloat(xPos + negoScn[fld].xPos + 2, 
							  yPos + 3,
							  negoScn[fld].fldMask);
			negoRec.qOrd = newQty;
			if (LclSpecValid(fld))
				negoRec.qOrd = oldQty;
			break;

		case 1:		/* Quantity BackOrdered */
			oldQty = negoRec.qBord;
			newQty = getfloat(xPos + negoScn[fld].xPos + 2, 
							  yPos + 3,
							  negoScn[fld].fldMask);
			negoRec.qBord = newQty;
			if (LclSpecValid(fld))
				negoRec.qBord = oldQty;
			break;

		case 2:		/* Gross Price */
			oldVal = negoRec.grossPrice;
			newVal = getmoney(xPos + negoScn[fld].xPos + 2, 
							  yPos + 3,
							  negoScn[fld].fldMask);
			negoRec.grossPrice = newVal;
			if (LclSpecValid(fld))
				negoRec.grossPrice = oldVal;
			break;

		case 3:		/* Regulatory Percent */
			oldPc = negoRec.regPc;
			newPc = getfloat(xPos + negoScn[fld].xPos + 2, 
							 yPos + 3,
							 negoScn[fld].fldMask);
			negoRec.regPc = newPc;
			if (LclSpecValid(fld))
				negoRec.regPc = oldPc;
			break;

		case 4:		/* Discount A */
		case 5:		/* Discount B */
		case 6:		/* Discount C */
			oldPc = negoRec.discArray[fld - 4];
			newPc = getfloat(xPos + negoScn[fld].xPos + 2, 
							 yPos + 3,
							 negoScn[fld].fldMask);
			negoRec.discArray[fld - 4] = newPc;
			if (LclSpecValid(fld))
				negoRec.discArray[fld - 4] = oldPc;

			break;

		case 8:		/* Net Price */
			oldVal = negoRec.netPrice;
			newVal = getmoney(xPos + negoScn[fld].xPos + 2, 
							  yPos + 3,
							  negoScn[fld].fldMask);
			negoRec.netPrice = newVal;
			if (LclSpecValid(fld))
				negoRec.netPrice = oldVal;
			break;

		case 10:		/* Margin Percent */
			oldPc = negoRec.margPc;
			newPc = getfloat(xPos + negoScn[fld].xPos + 2, 
							 yPos + 3,
							 negoScn[fld].fldMask);
			negoRec.margPc = newPc;
			if (LclSpecValid(fld))
				negoRec.margPc = oldPc;
			break;

		case 11:		/* Margin Value */
			oldVal = negoRec.margVal;
			newVal = getmoney(xPos + negoScn[fld].xPos + 2, 
							  yPos + 3,
							  negoScn[fld].fldMask);
			negoRec.margVal = newVal;
			if (LclSpecValid(fld))
				negoRec.margVal = oldVal;
			break;
		}
	}
	crsr_off();
}

/*-----------------------------------
| Local spec_valid() for validating |
| data entered into fields.         |
-----------------------------------*/
static	int	LclSpecValid (int fldNo)
{
	float	tmpDisc;

	switch (fldNo)
	{
	case 0:
		if (negoRec.qOrd < (float)0.00)
		{
			print_mess("\007 Quantity Must Be Greater Than 0.00 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 1:
		if (negoRec.qBord < (float)0.00)
		{
			print_mess("\007 Backorder Quantity Must Be Greater Than 0.00 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 2:
		if (negoRec.grossPrice < (double)0.00)
		{
			print_mess("\007 Gross Price Must Be Greater Than 0.00 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 3:
		if (negoRec.regPc < (float)0.00 || negoRec.regPc > (float)99.99)
		{
			if (negoRec.regPc < 0.00)
				print_mess("\007 Regulatory Percent may not be below 0.00 ");
			else
				print_mess("\007 Regulatory Percent may not exceed 99.99 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 4:		/* Discount A */
	case 5:		/* Discount B */
	case 6:		/* Discount C */
		if (negoRec.discArray[fldNo - 4] > (float)99.99)
		{
			print_mess("\007 Discount may not exceed 99.99 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		tmpDisc = CalcOneDisc (negoRec.cumDisc,
							   negoRec.discArray[0],
							   negoRec.discArray[1],
							   negoRec.discArray[2]);
		if (tmpDisc > 99.99)
		{
			sprintf (err_str, "Combined discount total exceeds 99.99%% \007");
			print_mess (err_str);
			sleep (2);
			clear_mess ();
			return (1);
		}
		if (tmpDisc < -99.99)
		{
			sprintf (err_str, "Combined surcharge total exceeds -99.99%% \007");
			print_mess (err_str);
			sleep (2);
			clear_mess ();
			return (1);
		}
		break;

	case 8:		/* Net Price */
		if (negoRec.netPrice < (double)0.00)
		{
			print_mess("\007 Net Price Must Be Greater Than 0.00 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		tmpDisc = DiscNet();
		if (tmpDisc < (float)-99.99 || tmpDisc > (float)99.99)
		{
			if (tmpDisc < (float)-99.99)
			{
				print_mess("\007 This Net Price would produce a discount below -99.99 ");
			}
			else
			{
				print_mess("\007 This Net Price would produce a discount above 99.99 ");
			}
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 10:	/* Margin PC */
		if (negoRec.margPc < (float)0.00 || negoRec.margPc > (float)999.99)
		{
			if (negoRec.margPc < (float)0.00)
				print_mess("\007 Margin Percent Must Be Greater Than 0.00 ");
			else
				print_mess("\007 Margin Percent may not exceed 999.99 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		tmpDisc = DiscPc();
		if (tmpDisc < (float)-99.99 || tmpDisc > (float)99.99)
		{
			if (tmpDisc < (float)-99.99)
			{
				print_mess("\007 This Margin would produce a discount below -99.99 ");
			}
			else
			{
				print_mess("\007 This Margin would produce a discount above 99.99 ");
			}
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	case 11:	/* Margin Value */
		if (negoRec.margVal < (double)0.00)
		{
			print_mess("\007 Margin Value Must Be Greater Than 0.00 ");
			sleep(1);
			clear_mess();
			return(1);
		}
		tmpDisc = DiscVal();
		if (tmpDisc < (float)-99.99 || tmpDisc > (float)99.99)
		{
			if (tmpDisc < (float)-99.99)
			{
				print_mess("\007 This Margin would produce a discount below -99.99 ");
			}
			else
			{
				print_mess("\007 This Margin would produce a discount above 99.99 ");
			}
			sleep(1);
			clear_mess();
			return(1);
		}
		break;

	}

	return(0);
}

/*-------------------------------------------------
| Recalculate all figures based on field changed. |
-------------------------------------------------*/
void
RecalcFigures (int fldNo)
{
	double	priceLessReg;

	/*-----------------------------------
	| Calculate Gross Less Regulatory % |
	-----------------------------------*/
	priceLessReg = GetCusGprice(negoRec.grossPrice, negoRec.regPc);

	switch (fldNo)
	{
	case 0:	/* Qty Ord */
	case 1:	/* Qty BO */
	case 2:	/* Gross Price */
	case 3:	/* Reg PC */
	case 4:	/* Discount A */
	case 5:	/* Discount B */
	case 6:	/* Discount C */
		RecalcNet();
		break;

	case 8:	/* Net Price */
		if (priceLessReg == (double)0.00)
			negoRec.discArray[0] = (float)0.00;
		else
			negoRec.discArray[0] = DiscNet();

		/*----------------------------
		| Zeroise discounts B and C. |
		----------------------------*/
		negoRec.discArray[1] = (float)0.00;
		negoRec.discArray[2] = (float)0.00;
		RecalcNet();
		break;

	case 10:	/* Margin Percent */
		if (priceLessReg == (double)0.00)
			negoRec.discArray[0] = (float)0.00;
		else
			negoRec.discArray[0] = DiscPc();

		/*----------------------------
		| Zeroise discounts B and C. |
		----------------------------*/
		negoRec.discArray[1] = (float)0.00;
		negoRec.discArray[2] = (float)0.00;
		RecalcNet();
		break;

	case 11:	/* Margin Value */
		if (priceLessReg == (double)0.00)
			negoRec.discArray[0] = (float)0.00;
		else
			negoRec.discArray[0] = DiscVal();

		/*----------------------------
		| Zeroise discounts B and C. |
		----------------------------*/
		negoRec.discArray[1] = (float)0.00;
		negoRec.discArray[2] = (float)0.00;
		RecalcNet();
		break;

	}

}

/*--------------------------------------------------------
| Recalculate the Net, Extended value and Margin fields. |
--------------------------------------------------------*/
void
RecalcNet (void)
{
	float	oneDisc;
	double	extCost;
	double	oc_value;

	/*----------------------
	| Calculate Net Price. |
	----------------------*/
	negoRec.salePrice = GetCusGprice(negoRec.grossPrice, negoRec.regPc);

	negoRec.netPrice = negoRec.salePrice;
	oneDisc = CalcOneDisc(negoRec.cumDisc, 
						  negoRec.discArray[0],
						  negoRec.discArray[1],
						  negoRec.discArray[2]);
	if (oneDisc == (float)0.00)
		negoRec.discAmt = (double)0.00;
	else
	{
		negoRec.discAmt = negoRec.netPrice * (double)oneDisc;
		negoRec.discAmt /= (double)100.00;
		negoRec.discAmt = no_dec(negoRec.discAmt);
	}
	negoRec.netPrice -= negoRec.discAmt;

	/*---------------------------
	| Calculate Extended Value. |
	---------------------------*/
	oc_value = out_cost (negoRec.netPrice, negoRec.outer_size);
	negoRec.extVal = (double)(negoRec.qOrd + negoRec.qBord) * oc_value;

	/*-------------------------
	| Calculate Margin Value. |
	-------------------------*/
	oc_value = out_cost (negoRec.margCost, negoRec.outer_size);
	extCost = CENTS((double)(negoRec.qOrd + negoRec.qBord) * oc_value);
	negoRec.margVal = negoRec.extVal - extCost;

	/*----------------------
	| Calculate Margin PC. |
	----------------------*/
	if (negoRec.extVal == (double)0.00)
		negoRec.margPc = (double)0.00;
	else
	{
		negoRec.margPc = negoRec.extVal - extCost;
		negoRec.margPc /= negoRec.extVal;
		negoRec.margPc *= (double)100.00;
		negoRec.margPc = twodec(negoRec.margPc);
	}
}
/*--------------------------------
| Calculate new discount percent |
| based on Net Price changing.   |
--------------------------------*/
float DiscNet (void)
{
	double	priceLessReg;
	double	tmpDisc;

	/*-----------------------------------
	| Calculate Gross Less Regulatory % |
	-----------------------------------*/
	priceLessReg = GetCusGprice(negoRec.grossPrice, negoRec.regPc);

	/*---------------------------------------
	| Recalc required discount to give net. |
	---------------------------------------*/
	tmpDisc = ((double)1.00 - (negoRec.netPrice / priceLessReg));
	tmpDisc *= (double)100.00;
	tmpDisc = twodec(tmpDisc);

	return((float)tmpDisc);
}
/*--------------------------------
| Calculate new discount percent |
| based on Margin PC changing.   |
--------------------------------*/
float DiscPc (void)
{
	double	priceLessReg;
	double	tmpQty;
	double	extCost;
	double	newExt;
	double	newNet;
	double	tmpDisc;

	/*-----------------------------------
	| Calculate Gross Less Regulatory % |
	-----------------------------------*/
	priceLessReg = GetCusGprice(negoRec.grossPrice, negoRec.regPc);

	/*------------------------------------
	| Recalc required net to give margin |
	------------------------------------*/
	tmpQty = (double)(negoRec.qOrd + negoRec.qBord);
	if (tmpQty == (double)0.00)
		tmpQty = (double)1.00;

	extCost = CENTS(tmpQty * negoRec.margCost);

	if (extCost == (double)0.00)
		newNet = negoRec.netPrice;
	else
	{
		newExt = no_dec(extCost / (1.00 - (negoRec.margPc / 100.00)));
		newNet = no_dec(newExt / tmpQty);
	}

	tmpDisc = ((double)1.00 - (newNet / priceLessReg)) * 100.00;
	tmpDisc = twodec(tmpDisc);

	return((float)tmpDisc);
}
/*--------------------------------
| Calculate new discount percent |
| based on Margin Value changing |
--------------------------------*/
float DiscVal (void)
{
	double	priceLessReg;
	double	tmpQty;
	double	extCost;
	double	newExt;
	double	newNet;
	double	tmpDisc;

	/*-----------------------------------
	| Calculate Gross Less Regulatory % |
	-----------------------------------*/
	priceLessReg = GetCusGprice(negoRec.grossPrice, negoRec.regPc);

	/*------------------------------------
	| Recalc required net to give margin |
	------------------------------------*/
	tmpQty = (double)(negoRec.qOrd + negoRec.qBord);
	if (tmpQty == (double)0.00)
		tmpQty = (double)1.00;

	extCost = CENTS(tmpQty * negoRec.margCost);

	newExt = extCost + negoRec.margVal;
	newNet = no_dec(newExt / tmpQty);

	tmpDisc = ((double)1.00 - (newNet / priceLessReg)) * 100.00;
	tmpDisc = twodec(tmpDisc);

	return((float)tmpDisc);
}

void
VertLine (int xPos, int yPos)
{
	move(xPos, yPos);
	PGCHAR(8);

	move(xPos, yPos + 1);
	PGCHAR(5);

	move(xPos, yPos + 2);
	PGCHAR(7);

	move(xPos, yPos + 3);
	PGCHAR(5);

	move(xPos, yPos + 4);
	PGCHAR(9);
}

