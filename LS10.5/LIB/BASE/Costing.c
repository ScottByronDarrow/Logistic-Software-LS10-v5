/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: Costing.c,v 5.7 2002/04/29 05:38:16 scott Exp $
|---------------------------------------------------------------------|
| $Log: Costing.c,v $
| Revision 5.7  2002/04/29 05:38:16  scott
| Fixed $Id$
|
| Revision 5.6  2002/01/09 00:56:01  scott
| Updated to change FindInsfCost () to include additional argument of hhbrHash
|
| Revision 5.5  2002/01/09 00:44:39  scott
| Updated to document better.
|
| Revision 5.4  2001/12/12 08:23:23  scott
| Issues List Reference Number = 00679
| Environment: Linux-Informix Server / HP-Oracle Server / Windows Client
| Program Description = SKTR5-Transfer Request Confirmation
| Description of the Issue: After the transfer ( done under direct issue ) from warehouse 1 to 2, warehouse 2 will receive the stock transfer; but when check the <Fifo> of the stock in warehouse 2, the transfer is not reflected.
|
| Revision 5.3  2001/12/05 03:07:45  scott
| Updated to fix incorrect return from CheckInsc.
|
| Revision 5.2  2001/08/06 22:40:51  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:25  scott
| New library for 10.5
|
=====================================================================*/
#define		COSTING_DEFINE
#include	<std_decs.h>
#include	<Costing.h>
#include <arralloc.h>

	static	const	char 	*incf2	=	"_incf2_costing";
	INCF_STRUCT	incf2Rec;

	static	int		incf_openDone	=	FALSE,
					incf2_openDone	=	FALSE,
					insf_openDone	=	FALSE,
					inei_openDone	=	FALSE,
					insc_openDone	=	FALSE,
					itff_openDone	=	FALSE;

static	char	envSerValue [2];
static	int		envSkLeaveFifo	=	3;

long	lastFifo	= 0L;

extern	float	fifoQtyShort;
extern	int		fifoError;
extern	int		IN_STAKE;
extern	int		cc;
extern	long	STAKE_COFF;
extern	char	temp_str [];

/*
 *	Structure for dynamic array.
 */
struct FifoRecord
{
	long	hhcfHash;
}	*fifoRec;
	DArray fifoDetails;
	int	fifoCnt = 0;

/*
 *************************************************************************
 * CheckInsc (S)
 *************************************************************************
 * Find stock take header record.
 *
 *	Normal values passed
 *	--------------------
 * 	hhccHash			=	ccmr_hhcc_hash
 * 	checkDate			=	Date to check to see if in stock take.
 * 	stockTakeStatus		=	incc_stat_flag
 */
int
CheckInsc (
	long	hhccHash,
	long	checkDate,
	char	*stockTakeStatus)
{
	OpenInsc ();

	inscRec.hhcc_hash = hhccHash;
	sprintf (inscRec.stake_code, "%-1.1s", stockTakeStatus);
	cc = find_rec (insc, &inscRec, COMPARISON, "r");
	if (cc)
		return (0);
	
	return ((checkDate <= inscRec.frz_date) ? 1 : 0);
}

/*
 *************************************************************************
 * CheckIncf (S)
 *************************************************************************
 *	Function returns the quantity * last cost of stock shortfall.
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 * 	qtyRequired			=	Quantity for stock value.
 * 	decimalPlaces		=	inmr_dec_pt
 */
double	
CheckIncf (					
	long	hhwhHash,
	int		fifoFlag,
	float	qtyRequired,	
	int		decimalPlaces)	
{						
	float	qtyAvailable 	= 0.00;
	double	costValue 		= 0.00;

	OpenIncf ();
	
	cc = FindIncf (hhwhHash, fifoFlag, "r");
	while (!cc && hhwhHash == incfRec.hhwh_hash)
	{
		/*
		 * Check (1) There is a stock date cutoff date (STAKE_COFF)
		 *       (2) Item is in stock take mode.       (IN_STAKE) 
		 *       (3) FIFO date is Less-than or Equal-to STAKE_COFF
		 */
		if (STAKE_COFF && IN_STAKE)
		{
			if (incfRec.fifo_date <= STAKE_COFF)
				qtyAvailable += n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		else
		{
			qtyAvailable += n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		cc = FindIncf (0L, fifoFlag, "r");
	}
	if (qtyRequired > qtyAvailable)
	{

		costValue = twodec (ineiRec.last_cost);
		qtyAvailable  = 	n_dec 
								(
										qtyRequired - 
										qtyAvailable, 
										decimalPlaces
								);

		return (twodec (costValue * n_dec (qtyAvailable, decimalPlaces)));
	}
	return (0.00);
}
/*
 *************************************************************************
 * FindIncfCost (S)
 *************************************************************************
 *	Function returns the total cost of inventory based 
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	closingStock		=	incc_closing_stock
 * 	qtyRequired			=	Quantity for stock value.
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 * 	decimalPlaces		=	inmr_dec_pt
 */
double	
FindIncfCost (
	long	hhwhHash, 			/* incc_hhwh_hash	*/
	float	closingStock, 
	float	qtyRequired, 
	int		fifoFlag,
	int		decimalPlaces)
{
	float	qtyAt 		= (float) qtyRequired;
	float	qtyValue	=	0.00;
	float	onHand 		= (float) closingStock;
	double	value 		= 0.00;
	double	extend 		= 0.00;

	if (qtyRequired == 0.00)
		return (0.00);

	if (closingStock <= 0.00)
		return (-1.00);

	OpenIncf ();

	/*
	 * Read in Lifo order and go forward until closing stock is used.
	 */
	cc = FindIncf (hhwhHash, !fifoFlag, "r");

	/*
	 * No incf records
	 */
	if (cc)
		return (0.00);

	while (onHand > 0.00 && !cc && incfRec.hhwh_hash == hhwhHash)
	{
		if (onHand <= incfRec.fifo_qty)
			break;

		onHand -= incfRec.fifo_qty;

		cc = FindIncf (0L,!fifoFlag, "r");
	}

	/*
	 * Opps ran out of FIFO record, should not occur.
	 */
	if (cc || incfRec.hhwh_hash != hhwhHash)
		cc = FindIncf (hhwhHash,fifoFlag, "r");
	else
	{
		/*
		 * Need to get current record and correct direction with new SQL
		 */
		cc = find_rec (incf, &incfRec, (fifoFlag) ? GTEQ : LTEQ, "r");
		incfRec.fifo_qty = onHand;
	}
	while (qtyAt > 0.00 && !cc && incfRec.hhwh_hash == hhwhHash)
	{
		/*
		 * fifo_qty > qty to fill
		 */
		if (incfRec.fifo_qty > qtyAt)
		{
			extend = (double) n_dec (qtyAt, decimalPlaces);
			extend *= twodec (incfRec.fifo_cost);
			value += extend;
			qtyAt = 0.00;
		}
		else
		{
			extend = (double) n_dec (incfRec.fifo_qty, decimalPlaces);
			extend *= twodec (incfRec.fifo_cost);
			value += extend;
			qtyAt -= n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		cc = FindIncf (0L,fifoFlag, "r");
	}
	qtyValue = (float) qtyRequired - qtyAt;
	extend = (double) (qtyAt > 0.00) ? (double) qtyValue : (double) qtyRequired;

	if (extend != 0.00)
		value /= extend;
	else
		value = 0.00;
	
	return (twodec (value));
}
/*
 *************************************************************************
 * FindIncfCostPO (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	closingStock		=	incc_closing_stock
 * 	qtyRequired			=	Quantity for stock value.
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 *	decimalPlaces		=	inmr_dec_pt
 */
double 
FindIncfCostPO (
	long	hhwhHash, 
	float	closingStock, 
	float	qtyRequired, 
	int		fifoFlag,
	int		decimalPlaces)
{
	float	qtyAt 		= (float) qtyRequired;
	float	qtyValue	=	0.00;
	float	onHand 		= (float) closingStock;
	double	value 		= 0.00;
	double	extend 		= 0.00;

	if (qtyRequired == 0.00)
		return (0.00);

	if (closingStock <= 0.00)
		return (-1.00);

	OpenIncf ();

	if (fifoFlag)
	{
		/*
		 * Read in Lifo order and go forward until closing stock is used.
		 */
		cc = FindIncf (hhwhHash,!fifoFlag, "r");

		/*
		 * no incf records
		 */
		if (cc)
			return (0.00);

		while (onHand > 0.00 && !cc && incfRec.hhwh_hash == hhwhHash)
		{
			if (onHand <= incfRec.fifo_qty)
				break;

			onHand -= incfRec.fifo_qty;
	
			cc = FindIncf (0L,!fifoFlag, "r");
		}

		/*
		 * Opps ran out of FIFO record, should not occur.
		 */
		if (cc || incfRec.hhwh_hash != hhwhHash)
			cc = FindIncf (hhwhHash,fifoFlag, "r");
		else
		{
			/*
			 *	need to get current record and correct direction with new SQL
			 */
			cc = find_rec (incf, &incfRec, (fifoFlag) ? GTEQ : LTEQ, "r");
			incfRec.fifo_qty = onHand;
		}
	}
	else
	{
		/*
		 * LIFO much less complicated.
		 */
		cc = FindIncf (hhwhHash,fifoFlag, "r");
		if (cc)
			return (0.00);
	}
	while (qtyAt > 0.00 && !cc && incfRec.hhwh_hash == hhwhHash)
	{
		/*
		 * fifo_qty > qty to fill
		 */
		if (incfRec.fifo_qty > qtyAt)
		{
			extend = (double) n_dec (qtyAt, decimalPlaces);
			extend *= incfRec.fifo_cost;
			value += extend;
			qtyAt = 0.00;
		}
		else
		{
			extend = (double) n_dec (incfRec.fifo_qty, decimalPlaces);
			extend *= twodec (incfRec.fifo_cost);
			value += extend;
			qtyAt -= n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		cc = FindIncf (0L,fifoFlag, "r");
	}
	qtyValue = (float) qtyRequired - qtyAt;
	extend = (double) (qtyAt > 0.00) ? (double) qtyValue : (double) qtyRequired;

	if (extend != 0.00)
		value /= extend;
	else
		value = 0.00;
	
	return (twodec (value));
}
/*
 *************************************************************************
 * FindIncfValue (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	onHand				=	incc_closing_stock
 *	average				=	Average cost (each) if true else extended cost.
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 * 	decimalPlaces		=	inmr_dec_pt
 */
double	
FindIncfValue (	
	long	hhwhHash, 
	float	onHand, 
	int		average, 
	int		fifoFlag,
	int		decimalPlaces)
{
	double	extendedValue	=	0.00;
	double	quantity		=	0.00;
	double	value			=	0.00;
	float	quantityOnHand 	= onHand;

	fifoError	= 0;
	fifoQtyShort	= 0.00;

	OpenIncf ();
	/*
	 * Find oldest lifo record first or find newest fifo record first   
	 */
	cc = FindIncf (hhwhHash,!fifoFlag, "r");
	if (cc)
	{
		fifoQtyShort	= n_dec (quantityOnHand, decimalPlaces);
		fifoError	= TRUE;
		return (0.00);
	}

	while (!cc && incfRec.hhwh_hash == hhwhHash && quantityOnHand > 0.00)
	{
		/*
		 * Check (1) There is a stock date cutoff date (STAKE_COFF)  
		 *       (2) Item is in stock take mode.       (IN_STAKE)   
		 *       (3) FIFO date is greater-than STAKE_COFF.         
		 */
		if (STAKE_COFF && IN_STAKE && incfRec.fifo_date > STAKE_COFF)
		{
			cc = FindIncf (0L, !fifoFlag, "r");
			continue;
		}
		fifoQtyShort	= n_dec (quantityOnHand, decimalPlaces);
		if (quantityOnHand > n_dec (incfRec.fifo_qty, decimalPlaces))
		{
			value = (double) n_dec (incfRec.fifo_qty, decimalPlaces);
			extendedValue += twodec (incfRec.fifo_cost) * value;
			quantity  += (double) n_dec (incfRec.fifo_qty, decimalPlaces);
			quantityOnHand -= n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		else
		{
			value = (double) n_dec (quantityOnHand, decimalPlaces);
			extendedValue += twodec (incfRec.fifo_cost) * value;
			quantity  += (double) n_dec (quantityOnHand, decimalPlaces);
			quantityOnHand -= n_dec (incfRec.fifo_qty, decimalPlaces);
		}
		lastFifo = incfRec.fifo_date;

		cc = FindIncf (0L,!fifoFlag, "r");
	}
	if (quantityOnHand > 0.00)
	{
		fifoQtyShort = n_dec (quantityOnHand, decimalPlaces);
		fifoError = TRUE;
	}
	extendedValue	=	twodec (extendedValue);

	if (average && quantity != 0.00)
		extendedValue /= n_dec (quantity, decimalPlaces);

	return (twodec (extendedValue));
}
/*
 *************************************************************************
 * FindIneiCosts (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	costingType			=	L(ast), S(T)andard, P(rev), A(verage)
 *	branchNo			=	Current branch number (comm_est_no).
 * 	hhbrHash			=	inmr_hhbr_hash
 */
double
FindIneiCosts (
	char	*costingType,
	char	*branchNo,
	long	hhbrHash)
{
	OpenInei ();
	/*
	 * Find inei record to find  cost associated with item
	 */
	memset (&ineiRec, 0, sizeof ineiRec);
	cc = FindInei (hhbrHash, branchNo, "r");
	if (cc)
		return ((double) -1.00);

	switch (costingType [0])
	{
		case 'A':
			return (twodec (ineiRec.avge_cost));
			break;

		case 'L':
			return (twodec (ineiRec.last_cost));
			break;

		case 'P':
			return (twodec (ineiRec.prev_cost));
			break;

		case 'T':
			return (twodec (ineiRec.std_cost));
			break;

		default:
			return ((double) -1.00);
	}
}
/*
 *************************************************************************
 * FindInsfCost (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	hhbrHash			=	incc_hhbr_hash
 * 	serialNo			=	insf_serial_no
 * 	serialStatus		=	F(ree) C(ommitted) S(old) T(ransit)
 */
double	
FindInsfCost (
	long	hhwhHash, 
	long	hhbrHash, 
	char	*serialNo,
	char	*serialStatus)
{
	OpenInsf ();

	if (hhwhHash != 0L)
	{
		abc_selfield (insf, "insf_id_no");
		insfRec.hhwh_hash	=	hhwhHash;
		sprintf (insfRec.status, "%-1.1s", serialStatus);
		sprintf (insfRec.serial_no,"%-25.25s", serialNo);
	}
	else if (hhbrHash != 0L)
	{
		abc_selfield (insf, "insf_hhbr_id");
		insfRec.hhbr_hash = hhbrHash;
		sprintf (insfRec.status, "%-1.1s", serialStatus);
		sprintf (insfRec.serial_no,"%-25.25s", serialNo);
	}
	cc = find_rec (insf, &insfRec, COMPARISON, "r");
	if (cc)
		return (-1.00);
	
	return (SerialValue (insfRec.est_cost, insfRec.act_cost));
}
double
SerialValue (
	double	estCost,
	double	actCost)
{
	if (envSerValue[0] == 'E' || n_dec (actCost,5) == 0.00)
		return (n_dec (estCost, 5));

	return (n_dec (actCost, 5));
}

/*
 *************************************************************************
 * FindInsfValue (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 *	average				=	Average cost (each) if true else extended cost.
 */
double	
FindInsfValue (
	long	hhwhHash, 
	int 	average)
{
	int		serialCounter	=	0;
	double	serialCost		=	0.00;

	OpenInsf ();

	cc = FindInsf (hhwhHash, 0L, (char *) NULL, "F", "r");
	while (!cc && hhwhHash == insfRec.hhwh_hash)
	{
		if (envSerValue[0] == 'E' || insfRec.act_cost == 0.00)
			serialCost += twodec (insfRec.est_cost);
		else
			serialCost += twodec (insfRec.act_cost);

		serialCounter++;

		cc = FindInsf (0L, 0L, (char *) NULL, "F", "r");
	}
	cc = FindInsf (hhwhHash, 0L, (char *) NULL, "C", "r");
	while (!cc && hhwhHash == insfRec.hhwh_hash)
	{
		if (envSerValue[0] == 'E' || insfRec.act_cost == 0.00)
			serialCost += twodec (insfRec.est_cost);
		else
			serialCost += twodec (insfRec.act_cost);

		serialCounter++;

		cc = FindInsf (0L, 0L, (char *) NULL, "C", "r");
	}
	if (average && serialCounter > 0)
		serialCost /= (double) serialCounter;

	serialCost	=	twodec (serialCost);
	return ((serialCounter) ? serialCost : 0.00);
}
/*
 *************************************************************************
 * StockValue (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 *	costingType			=	F(ifo), L(ifo),  L(ast), S(T)andard,
 *							P(rev), S(erial) A(verage)
 *	branchNo			=	Current branch number (comm_est_no).
 * 	hhbrHash			=	inmr_hhbr_hash
 * 	hhwhHash			=	incc_hhwh_hash
 * 	closingStock		=	incc_closing_stock
 * 	qtyRequired			=	Quantity for stock value.
 *	decimalPlaces		=	inmr_dec_pt
 *	average				=	Average cost (each) if true else extended cost.
 */
double	
StockValue (
	char	*costingType,
	char	*branchNo,
	long	hhbrHash,
	long	hhwhHash,
	float	closingStock,
	int		decimalPlaces,
	int		average)
{
	double	workCosting	=	0.00;
	double	holdCosting	=	0.00;

	OpenInsf ();
	OpenIncf ();
	OpenInei ();

	closingStock	=	n_dec (closingStock, decimalPlaces);

	switch (costingType[0])
	{
		/*
		 * Average costing, Last costing, Standard costing and Previous costing.
		 */
		case	'A':
		case	'L':
		case	'T':
		case	'P':
			holdCosting	=	FindIneiCosts 
							(
								costingType, 
								branchNo, 
								hhbrHash
							);
			workCosting	=	(average) 	? holdCosting 
										: holdCosting * closingStock;
			break;

		/*
		 * Serial costing.
		 */
		case	'S':
			workCosting	=	FindInsfValue
							(
								hhwhHash,
								average
							);
			break;

		/*
		 * Fifo/Lifo costing.
		 */
		case	'F':
		case	'I':
			workCosting	=	FindIncfValue 
							(
								hhwhHash, 
								closingStock, 
								average, 
								(costingType[0] == 'F') ? TRUE : FALSE,
								decimalPlaces
							);
			fifoError	=	FALSE;
			if (fifoError)
			{
				workCosting	=	FindIncfValue 
								(
									hhwhHash, 
									closingStock - fifoQtyShort, 
									average, 
									(costingType[0] == 'F') ? TRUE : FALSE,
									decimalPlaces
								);
				holdCosting	=	FindIneiCosts 
							(
								"L", 
								branchNo, 
								hhbrHash
							);

				workCosting	+=	CheckIncf	
								(
									hhwhHash,
									(costingType[0] == 'F') ? TRUE : FALSE,
									closingStock,
									decimalPlaces
								);
				fifoError	=	TRUE;
			}
			break;

		default:	
			break;
	}
	return (twodec (workCosting));
}
/*
 *************************************************************************
 * FindInsc (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhccHash			=	insc_hhcc_hash (ex. ccmr_hhcc_hash) 
 * 	stockTakeMode		=	insc_stake_code
 *	byMode 				=	True if in stock take mode
 */
void
FindInsc (
	long	hhccHash, 
	char 	*stockTakeMode,
	int		byMode)
{
	OpenInsc ();

	inscRec.hhcc_hash = hhccHash;
	sprintf (inscRec.stake_code, "%-1.1s", stockTakeMode);
	if (find_rec (insc, &inscRec, COMPARISON, "r"))
	{
		IN_STAKE	= FALSE;
		STAKE_COFF	= 0L;
		strcpy (inscRec.description, " ");
		return;
	}
	STAKE_COFF	= inscRec.frz_date;
	IN_STAKE	= (byMode) ? TRUE : FALSE;

	return;
}

/*
 *************************************************************************
 * ValidStDate (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	stockDateDate	-	Date to check against stock take date.
 */
int
ValidStDate (
	long	stockTakeDate)
{
	if (!IN_STAKE)
		return (TRUE);

	if (stockTakeDate > inscRec.frz_date)
		return (FALSE);

	return (TRUE);
}
/*
 *************************************************************************
 * CloseCosting (S)
 *************************************************************************
 *	Normal values passed (NONE)
 *	--------------------
 *	Closes all files required for costing.
 */
void
CloseCosting (void)
{
	if (incf_openDone)
		abc_fclose (incf);

	if (incf2_openDone)
		abc_fclose (incf2);

	if (insf_openDone)
		abc_fclose (insf);

	if (inei_openDone)
		abc_fclose (inei);

	if (insc_openDone)
		abc_fclose (insc);

	if (itff_openDone)
		abc_fclose (itff);
}
/*
 *************************************************************************
 * FindIncf (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 * 	updFlag				=	UpdateFlag.	"r"=Read, "u"=Update, "w"=Write
 */
int
FindIncf (
	long	hhwhHash, 
	int		fifoFlag,
	char	*updFlag)
{
	int		rCode	=	0;
	static	long	hhwhOld;

	OpenIncf ();
	
	if (hhwhHash != 0L)
	{
		incfRec.hhwh_hash	= hhwhHash;
		incfRec.fifo_date	= (fifoFlag) ? 0L : TodaysDate () + 90000L;
		incfRec.seq_no		= (fifoFlag) ? 0  : 999;
		hhwhOld = hhwhHash;
		
		rCode =	find_rec (incf, &incfRec, (fifoFlag) ? GTEQ : LTEQ, updFlag);
		if (rCode || incfRec.hhwh_hash != hhwhHash)
			return (EXIT_FAILURE);
	}
	else
	{
		rCode =	find_rec (incf, &incfRec,(fifoFlag) ? NEXT : PREVIOUS, updFlag);
		if (rCode || incfRec.hhwh_hash != hhwhOld)
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
/*
 *************************************************************************
 * FindInei (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhbrHash			=	inmr_hhbr_hash
 * 	branchNo			=	comm_est_no
 * 	updFlag				=	UpdateFlag.	"r"=Read, "u"=Update, "w"=Write
 */
 int	
 FindInei (
 	long	hhbrHash,
	char	*branchNo,
	char	*updFlag)
{
	OpenInei ();

	ineiRec.hhbr_hash	=	hhbrHash;
	sprintf (ineiRec.est_no, "%-2.2s", branchNo);
	return (find_rec (inei, &ineiRec, COMPARISON, updFlag));
}

/*
 *************************************************************************
 * AddIncf (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 *	fifoDate			=	incf_fifo_date
 *	fifoCost			=	incf_fifo_cost
 *	fifoActCost			=	incf_act_cost
 *	fifoQty				=	incf_fifo_qty
 *	*grNumber			=	incf_gr_number
 *	fobNorCst			=	incf_fob_nor_cst
 *	frtInsCst			=	incf_frt_ins_cst
 *	duty				=	incf_duty
 *	licence				=	incf_licence
 *	lcostLoad			=	incf_lcost_load
 *	landCst				=	incf_land_cst
 */
int	
AddIncf (
	long	hhwhHash,
	long	fifoDate,
	double	fifoCost,
	double	fifoActCost,
	float	fifoQty,
	char	*grNumber,
	double	fobNorCst,
	double	frtInsCst,
	double	duty,
	double	licence,
	double	lcostLoad,
	double	landCst,
	char 	*statFlag)
{
	int		seqNumber	=	0;

	OpenIncf2 ();
	
	if (fifoDate <= 0L)
		fifoDate	=	TodaysDate();

	/*
	 * The following is required as the system under SQL has to
	 * perform a GREATER call instead of NEXT. This means that 
	 * the a FIFO record with the same date is missed causing a
	 * problem. 											
	 */
	incf2Rec.hhwh_hash 	= hhwhHash;
	incf2Rec.fifo_date 	= fifoDate;
	incf2Rec.seq_no	 	= seqNumber;
	while (!find_rec (incf2, &incf2Rec, COMPARISON, "r"))
	{
		incf2Rec.hhwh_hash 	= hhwhHash;
		incf2Rec.fifo_date 	= fifoDate;
		incf2Rec.seq_no	 	= seqNumber++;
	}
	sprintf (incf2Rec.gr_number,"%-15.15s", grNumber);
	sprintf (incf2Rec.stat_flag,"%-1.1s", statFlag);
	strcpy (incf2Rec.gr_number, grNumber);
	incf2Rec.fifo_cost	=	fifoCost;
	incf2Rec.act_cost	=	fifoActCost;
	incf2Rec.fifo_qty	=	fifoQty;
	incf2Rec.fob_nor_cst	=	fobNorCst;
	incf2Rec.frt_ins_cst	=	frtInsCst;
	incf2Rec.duty		=	duty;
	incf2Rec.licence		=	licence;
	incf2Rec.lcost_load	=	lcostLoad;
	incf2Rec.land_cst	=	landCst;
	return (abc_add (incf2, &incf2Rec));
}

/*
 *************************************************************************
 * UpdateInsf (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	serialNo			=	insf_serial_no (valid serial number).
 * 	oldStatus			=	Old status of serial number.
 * 	newStatus			=	New staus of serial number.
 */
int	
UpdateInsf (
	long	hhwhHash, 
	long	hhbrHash, 
	char 	*serialNo, 
	char 	*oldStatus, 
	char 	*newStatus)
{
	OpenInsf ();
	
	cc = FindInsf (hhwhHash, hhbrHash, serialNo,oldStatus,"u");
	if (cc)
		return (cc + 1000);

	strcpy (insfRec.status,newStatus);
	return (abc_update (insf,&insfRec));
}

/*
 *************************************************************************
 * FindInsf (S)
 *************************************************************************
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	hhbrHash			=	inmr_hhbr_hash
 * 	serialNo			=	insf_serial_no (valid serial number).
 * 	serialStatus		=	F(ree) C(ommitted) S(old) T(ransit)
 * 	findType			=	"r"=Read, "u"=Update, "w"=Write
 */
int
FindInsf (
	long	hhwhHash, 
	long	hhbrHash, 
	char	*serialNo, 
	char	*serialStatus, 
	char	*findType)
{
	int	noSeral;

	OpenInsf ();

	noSeral = (serialNo == (char *)0 || strlen (serialNo) == 0);

	if (hhwhHash != 0L)
	{
		abc_selfield (insf, "insf_id_no");
		insfRec.hhwh_hash = hhwhHash;
		sprintf (insfRec.status,	"%-1.1s",serialStatus);
		sprintf (insfRec.serial_no,"%-25.25s", (!noSeral) ? serialNo : " ");
	}
	else if (hhbrHash != 0L)
	{
		abc_selfield (insf, "insf_hhbr_id");
		insfRec.hhbr_hash = hhbrHash;
		sprintf (insfRec.status,	"%-1.1s",serialStatus);
		sprintf (insfRec.serial_no,"%-25.25s", (!noSeral) ? serialNo : " ");
	}

	/*
	 *	if serialNo != NULL
	 *		find COMPARISON
	 *	else
	 *		find NEXT or GTEQ (on hhwhHash)
	 */

	if (noSeral)
		cc = find_rec (insf, &insfRec, (hhwhHash) ? GTEQ : NEXT, findType);
	else
		cc = find_rec (insf, &insfRec, COMPARISON, findType);

	/*
	 * if error in find	|
	 */
	if (cc)
		return (cc);

	/*
	 * Return FALSE if status matches status required.
	 */
	return ((insfRec.status[0] != serialStatus[0]) ? -1 : 0);
}

/*
 *************************************************************************
 * ReduceIncf (S)
 *************************************************************************
 *	Reduce incf_fifo_qty's for incf records to allow stock transfers 
 *  between warehouses to work correctly.					
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	closingStock		=	incc_closing_stock
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 */
void
ReduceIncf (
	long	hhwhHash,
	float	closingStock,
	int		fifoFlag)
{
	int	rc;
	float	qty = closingStock;
	
	if (qty < 0.00)
		qty = 0.00;

	OpenIncf ();
	
	rc = FindIncf (hhwhHash, !fifoFlag,"u");
	
	while (!rc && hhwhHash == incfRec.hhwh_hash)
	{
		/*
		 * Need to reduce ff_qty
		 */
		if (qty < incfRec.fifo_qty)
		{
			incfRec.fifo_qty = qty;
			qty = 0.00;
			rc = abc_update (incf,&incfRec);
			if (rc)
				file_err (cc, incf, "DBUPDATE");
		}
		else
			qty  -= incfRec.fifo_qty;

		abc_unlock (incf);

		rc = FindIncf (0L,!fifoFlag,"u");
	}
	abc_unlock (incf);
}
/*
 *************************************************************************
 * TransIncf (S)
 *************************************************************************
 *	Transfer incf records from one incc record to another.
 *
 *	Normal values passed
 *	--------------------
 * 	issHhwhHash			=	incc_hhwh_hash	-	Issue warehouse
 * 	recHhwhHash			=	incc_hhwh_hash	-	Receipt warehouse
 * 	closingStock		=	incc_closing_stock
 * 	qtyRequired			=	Quantity for stock value.
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 */
int	
TransIncf (
	long	issHhwhHash, 
	long	recHhwhHash, 
	float	closingStock, 
	float	qtyRequired, 
	int		fifoFlag)
{
	float	qtyAt 		= qtyRequired;
	float	stockOnHand = closingStock;
	int		rc = 0;

	/*
	 *	Check for valid parameters
	 */
	if (qtyRequired == 0.00 || closingStock <= 0.00)
	{
		fifoQtyShort = qtyAt;
		return (EXIT_FAILURE);
	}
	OpenIncf ();

	/*
	 * Both issue and receiving branch the same, no transfer required.
	 */
	if (issHhwhHash == recHhwhHash)
		return (EXIT_SUCCESS);

	/*
	 * 	Read first fifo record	
	 */
	rc = FindIncf (issHhwhHash,!fifoFlag,"r");

	/*
	 * No incf records
	 */
	if (rc || incfRec.hhwh_hash != issHhwhHash)
	{
		fifoQtyShort = qtyAt;
		return (EXIT_FAILURE);
	}
	/*
	 * Find records to transfer	
	 */
	while (stockOnHand > 0.00 && !rc && incfRec.hhwh_hash == issHhwhHash)
	{
		/*
		 *	Found last record for transfer
	 	 */
		if (stockOnHand <= incfRec.fifo_qty)
			break;

		stockOnHand -= (incfRec.fifo_qty > 0.00) ? incfRec.fifo_qty : 0.00;

		rc = FindIncf (0L, !fifoFlag,"r");
	}
	/*
	 * Lets document and understand what this next bit is doing so no one
	 * will remove it. Basically above we processed the fifo file in 		
	 * reverse order to find the position to start to get the value for the	
	 * amount of stock I want in relation to the SOH.						
	 * the line below is when we did not have enough records to we can deal	
	 * with that later. This condition does a restart using GTEQ or LTEQ 	
	 * with values RESET (See FindIncf.h)									
	 * The next condition is required because we did a LTEQ or GTEQ but		
	 * now we want to go in the oposite direction fine but LTEQ followed by	
	 * CISAM : NEXT AND PREV ALWAYS WORK IN SAME DIRECTION.	
	 */
	if (rc || incfRec.hhwh_hash != issHhwhHash)
		rc = FindIncf (issHhwhHash, fifoFlag,"r");
	else
	{
		cc = find_rec (incf, &incfRec, (fifoFlag) ? GTEQ : LTEQ, "r");
		incfRec.fifo_qty = stockOnHand;
	}
	/*
	 *	Transfer (add) appropriate records
	 */
	while (qtyAt > 0.00 && !rc && incfRec.hhwh_hash == issHhwhHash)
	{
		/*
		 * fifo_qty > qty to fill.
		 */
		if (incfRec.fifo_qty >= qtyAt)
		{
			rc 	= 	AddIncf 	
					(
						recHhwhHash,
						incfRec.fifo_date,
						incfRec.fifo_cost,
						incfRec.act_cost,
						qtyAt,
						incfRec.gr_number,
						incfRec.fob_nor_cst,
						incfRec.frt_ins_cst,
						incfRec.duty,
						incfRec.licence,
						incfRec.lcost_load,
						incfRec.land_cst,
						incfRec.stat_flag
					);
				if (rc)
					return (rc);

			qtyAt = 0.00;
			return (EXIT_SUCCESS);
		}
		else
		{
			qtyAt -= incfRec.fifo_qty;

			rc 	= 	AddIncf 	
					(
						recHhwhHash,
						incfRec.fifo_date,
						incfRec.fifo_cost,
						incfRec.act_cost,
						incfRec.fifo_qty,
						incfRec.gr_number,
						incfRec.fob_nor_cst,
						incfRec.frt_ins_cst,
						incfRec.duty,
						incfRec.licence,
						incfRec.lcost_load,
						incfRec.land_cst,
						incfRec.stat_flag
					);
			if (rc)
				return (rc);
		}
		rc = FindIncf (0L, fifoFlag,"r");
	}
	fifoQtyShort = qtyAt;
	return ((fifoQtyShort > 0.00) ? 1 : 0);
}

/*
 *************************************************************************
 * TransInsf (S)
 *************************************************************************
 *	Transfer insf records from one incc record to another.
 *
 *	Normal values passed
 *	--------------------
 * 	issHhwhHash			=	incc_hhwh_hash	-	Issue warehouse
 * 	recHhwhHash			=	incc_hhwh_hash	-	Receipt warehouse
 * 	serialNo			=	insf_serial_no (valid serial number).
 * 	serialStatus		=	F(ree) C(ommitted) S(old) T(ransit)
 */
int
TransInsf (
 long	issHhwhHash,
 long	recHhwhHash,
 char	*serialNo,
 char	*serialStatus)
{
	int	rc;

	OpenInsf ();
	
	rc = FindInsf (issHhwhHash, 0L, serialNo, serialStatus,"u");
	if (rc)
		return (rc + 1000);

	/*-------------------
	| new location		|
	-------------------*/
	insfRec.hhwh_hash = recHhwhHash;

	return (abc_update (insf ,&insfRec));
}

/*===========================
| Search for serial number. |
===========================*/
void
SearchInsf (
	long	hhwhHash,
	char	*serialStatus,
	char 	*key_val)
{
	char	serialNo [sizeof insfRec.serial_no];

	OpenInsf ();
	
	sprintf (serialNo, "%-25.25s", key_val);

	_work_open (25,0,40);
	save_rec ("#Serial Number ","#Receipted");
	cc = FindInsf (hhwhHash,0L, "",serialStatus,"r");
	while (!cc && hhwhHash == insfRec.hhwh_hash)
	{
		if (strncmp (serialNo,insfRec.serial_no,strlen (clip (serialNo))) < 0)
			break;
	
		if (!strncmp (serialNo,insfRec.serial_no,strlen (clip (serialNo))))
		{
			cc = save_rec (insfRec.serial_no,insfRec.receipted);
			if (cc)
				break;
		}
		cc = FindInsf (0L,0L, "",serialStatus,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cc = FindInsf (hhwhHash, 0L, temp_str,serialStatus,"r");
	if (cc)
		strcpy (temp_str, "");
}

/*
 *************************************************************************
 * TransItffToIncf (S)
 *************************************************************************
 *	Transfer itff records to incf file.
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash	
 * 	itffHash    		=	itln_itff_hash - Transfer fifo records.
 * 	processQuantity		=	Quantity to process.
 */
void	
TransItffToIncf (
	long	hhwhHash,
	long	itffHash,
	float	processQuantity)
{
	OpenItff ();
	
	itffRec.itff_hash	=	itffHash;
	cc = find_rec (itff, &itffRec, GTEQ, "u");
	while (!cc && itffRec.itff_hash == itffHash)
	{
		cc = AddIncf
			(
				hhwhHash,
				itffRec.fifo_date,
				itffRec.fifo_cost,
				itffRec.act_cost,
				itffRec.fifo_qty,
				itffRec.gr_number,
				itffRec.fob_nor_cst,
				itffRec.frt_ins_cst,
				itffRec.duty,
				itffRec.licence,
				itffRec.lcost_load,
				itffRec.land_cst,
				itffRec.stat_flag
			);
		if (cc)
			file_err (cc, incf, "DBADD");
			
		abc_delete (itff);

		itffRec.itff_hash	=	itffHash;
		cc = find_rec (itff, &itffRec, GTEQ, "u");
	}
	abc_unlock (itff);
}
/*
 *************************************************************************
 * TransIncfToItff (S)
 *************************************************************************
 *	Transfer incf records to itff file.
 *
 *	Normal values passed
 *	--------------------
 * 	issHhwhHash			=	incc_hhwh_hash	-	Issue warehouse
 * 	closingStock		=	incc_closing_stock
 * 	qtyRequired			=	Quantity for stock value.
 *	fileCost			=	Origional file cost of item.
 *	inputCost			=	Input cost of item
 *	inputDuty			=	Input cost of duty
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 * 	itffHash    		=	itln_itff_hash - Transfer fifo records.
 */
int	
TransIncfToItff (
	long	issHhwhHash,
	float	closingStock,
	float	qtyRequired,
	double	fileCost,
	double	inputCost,
	double	inputDuty,
	int		fifoFlag,
	long	itffHash)
{
	float	qtyAt 		= qtyRequired;
	float	stockOnHand = closingStock;
	int		rc = 0;
	int		costCharged	=	FALSE;

	if (fileCost != (inputCost + inputDuty))
		costCharged = TRUE;

	/*
	 *	Check for valid parameters
	 */
	if (qtyRequired > 0.00 && closingStock <= 0.00)
	{
		fifoQtyShort = qtyAt;
		return (EXIT_FAILURE);
	}
	OpenItff ();
	OpenIncf ();
	
	if (qtyRequired == 0.00)
		return (EXIT_SUCCESS);

	/*
	 * 	Read first fifo record	
	 */
	rc = FindIncf (issHhwhHash,!fifoFlag,"r");

	/*
	 * No incf records
	 */
	if (rc || incfRec.hhwh_hash != issHhwhHash)
	{
		fifoQtyShort = qtyAt;
		return (EXIT_FAILURE);
	}
	/*
	 * Find records to transfer	
	 */
	while (stockOnHand > 0.00 && !rc && incfRec.hhwh_hash == issHhwhHash)
	{
		/*
		 *	Found last record for transfer
	 	 */
		if (stockOnHand <= incfRec.fifo_qty)
			break;

		stockOnHand -= (incfRec.fifo_qty > 0.00) ? incfRec.fifo_qty : 0.00;

		rc = FindIncf (0L, !fifoFlag,"r");
	}
	/*
	 * Lets document and understand what this next bit is doing so no one
	 * will remove it. Basically above we processed the fifo file in 		
	 * reverse order to find the position to start to get the value for the	
	 * amount of stock I want in relation to the SOH.						
	 * the line below is when we did not have enough records to we can deal	
	 * with that later. This condition does a restart using GTEQ or LTEQ 	
	 * with values RESET (See FindIncf.h)									
	 * The next condition is required because we did a LTEQ or GTEQ but		
	 * now we want to go in the oposite direction fine but LTEQ followed by	
	 * CISAM : NEXT AND PREV ALWAYS WORK IN SAME DIRECTION.	
	 */
	if (rc || incfRec.hhwh_hash != issHhwhHash)
		rc = FindIncf (issHhwhHash, fifoFlag,"r");
	else
	{
		cc = find_rec (incf, &incfRec, (fifoFlag) ? GTEQ : LTEQ, "r");
		incfRec.fifo_qty = stockOnHand;
	}
	/*
	 * transfer (add) appropriate record
	 */
	while (qtyAt > 0.00 && !rc && incfRec.hhwh_hash == issHhwhHash)
	{
		/*
		 * fifo_qty > qty to fill
		 */
		if (incfRec.fifo_qty > qtyAt)
		{
			double	wrkCost	=	0.00;
			wrkCost = (costCharged) ? inputCost + inputDuty : incfRec.fifo_cost;

			/*
			 * Add transfer fifo information.
			 */
			itffRec.itff_hash 	= itffHash;
			itffRec.fifo_date 	= incfRec.fifo_date;
			itffRec.fifo_cost 	= (costCharged) ? inputCost + inputDuty
												: incfRec.fifo_cost;
			itffRec.act_cost 	= (costCharged) ? inputCost + inputDuty 
												: incfRec.act_cost;
			itffRec.fifo_qty  	= qtyAt;
			itffRec.fob_nor_cst	= incfRec.fob_nor_cst;
			itffRec.frt_ins_cst	= incfRec.frt_ins_cst;
			itffRec.duty		= incfRec.duty;
			itffRec.licence		= incfRec.licence;
			itffRec.lcost_load	= incfRec.lcost_load;
			itffRec.land_cst	= incfRec.land_cst;
			strcpy (itffRec.gr_number,incfRec.gr_number);
			strcpy (itffRec.stat_flag, incfRec.stat_flag);
			return (abc_add (itff,&itffRec));
		}
		else
		{
			qtyAt -= incfRec.fifo_qty;
			/*-----------------------------------
			| change hhwh_hash & add record		|
			-----------------------------------*/
			itffRec.itff_hash 	= itffHash;
			itffRec.fifo_date 	= incfRec.fifo_date;
			itffRec.fifo_cost 	= (costCharged) ? inputCost + inputDuty 
												: incfRec.fifo_cost;
			itffRec.act_cost 	= (costCharged) ? inputCost + inputDuty
												: incfRec.act_cost;
			itffRec.fifo_qty  	= incfRec.fifo_qty;
			itffRec.fob_nor_cst	= incfRec.fob_nor_cst;
			itffRec.frt_ins_cst	= incfRec.frt_ins_cst;
			itffRec.duty		= incfRec.duty;
			itffRec.licence		= incfRec.licence;
			itffRec.lcost_load	= incfRec.lcost_load;
			itffRec.land_cst	= incfRec.land_cst;
			strcpy (itffRec.gr_number,incfRec.gr_number);
			strcpy (itffRec.stat_flag, incfRec.stat_flag);
			rc = abc_add (itff,&itffRec);
			if (rc)
				return (rc);
		}
		rc = FindIncf (0L,fifoFlag,"r");
	}
	if (qtyAt <= 0.00)
		return (EXIT_SUCCESS);

	/*----------------------------------------------------------
	| Add record as not enough fifo records in issuing branch. |
	----------------------------------------------------------*/
	itffRec.itff_hash 	= itffHash;
	itffRec.fifo_date 	= TodaysDate ();
	itffRec.fifo_cost 	= inputCost + inputDuty;
	itffRec.act_cost 	= inputCost + inputDuty;
	itffRec.fifo_qty  	= qtyAt;
	itffRec.fob_nor_cst	= inputCost + inputDuty;
	itffRec.frt_ins_cst	= 0.00;
	itffRec.duty		= 0.00;
	itffRec.licence		= 0.00;
	itffRec.lcost_load	= 0.00;
	itffRec.land_cst	= inputCost + inputDuty;
	strcpy (itffRec.gr_number," ");
	strcpy (itffRec.stat_flag, "E");
	return (abc_add (itff, &itffRec));
}

/*
 *************************************************************************
 * PurgeIncf (S)
 *************************************************************************
 *	Purge Incf record(s)
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 * 	closingStock		=	incc_closing_stock
 * 	fifoFlag			=	TRUE = FIFO / FALSE = LIFO (inmr_costing_flag = F/I)
 */
void
PurgeIncf (
	long	hhwhHash, 
	int 	fifoFlag, 
	float	closingStock)
{
	float	qtyLeft 	= closingStock;
	int		fifoCounter = 0,	
			i			= 0;

	OpenIncf ();
	
	/*
	 * Allocate the initial array |
	 */
	ArrAlloc (&fifoDetails, &fifoRec, sizeof (struct FifoRecord), 50);
	fifoCnt = 0;

	abc_selfield (incf, "incf_seq_id");

	cc = FindIncf (hhwhHash, !fifoFlag,"r");
	while (qtyLeft > 0.00 && !cc && incfRec.hhwh_hash == hhwhHash)
	{
		/*-------------------------------------------------
		| Check the array size before adding new element. |
		-------------------------------------------------*/
		if (!ArrChkLimit (&fifoDetails, fifoRec, fifoCnt))
			sys_err ("ArrChkLimit (fifo)", ENOMEM, PNAME);

		if (incfRec.fifo_qty <= 0.00)
		{
			/*-----------------------------------------
			| Load values into array element custCnt. |
			-----------------------------------------*/
			fifoRec [fifoCnt].hhcfHash = incfRec.hhcf_hash;

			/*--------------------------
			| Increment array counter. |
			--------------------------*/
			fifoCnt++;
		}
		qtyLeft -= incfRec.fifo_qty;

		cc = FindIncf (0L,!fifoFlag,"r");
	}
	while (!cc && incfRec.hhwh_hash == hhwhHash)
	{
		if (fifoCounter++ < envSkLeaveFifo)
		{
			cc = FindIncf (0L,!fifoFlag,"r");
			continue;
		}
		/*-----------------------------------------
		| Load values into array element custCnt. |
		-----------------------------------------*/
		fifoRec [fifoCnt].hhcfHash = incfRec.hhcf_hash;

		/*--------------------------
		| Increment array counter. |
		--------------------------*/
		fifoCnt++;

		cc = FindIncf (0L,!fifoFlag,"r");
	}

	abc_selfield (incf, "incf_hhcf_hash");
	/*----------------------------------------------------------------
	| Step through the sorted array getting the appropriate records. |
	----------------------------------------------------------------*/
	for (i = 0; i < fifoCnt; i++)
	{
		incfRec.hhcf_hash	=	fifoRec [i].hhcfHash;
		cc = find_rec (incf, &incfRec, COMPARISON, "u");
		if (cc)
			file_err (cc, incf, "DBFIND");

		cc = abc_delete (incf);
		if (cc)
			file_err (cc, incf, "DBDELETE");
	}
	/*--------------------------
	| Free up the array memory |
	--------------------------*/
	ArrDelete (&fifoDetails);

	abc_selfield (incf, "incf_seq_id");
}

/*
 *************************************************************************
 * CheckIncfSequence (S)
 *************************************************************************
 *	Check Incf Sequence number to ensure it's unique.
 *
 *	Normal values passed
 *	--------------------
 * 	hhwhHash			=	incc_hhwh_hash
 */
int
CheckIncfSeq (
	long	hhwhHash)
{
	long	LastFifo	=	0L,
			ThisFifo	=	0L;

	int		LastSeq		=	0,
			ThisSeq		=	0;

	OpenIncf ();

	incfRec.hhwh_hash	=	hhwhHash;
	incfRec.fifo_date	=	0L;
	incfRec.seq_no		=	0;
	 
	cc = find_rec (incf, &incfRec, GTEQ, "u");
	while (!cc && incfRec.hhwh_hash == hhwhHash)
	{
		ThisFifo 	=	incfRec.fifo_date;
		ThisSeq 	=	incfRec.seq_no;
		if (ThisFifo == LastFifo && ThisSeq == LastSeq)
		{
			incfRec.seq_no++;

			cc = abc_update (incf,&incfRec);
			if (cc)
				file_err (cc, incf, "DBUPDATE");
		
			return (CheckIncfSeq (hhwhHash));
		}
		else
			abc_unlock (incf);

		LastFifo 	=	incfRec.fifo_date;
		LastSeq 	=	incfRec.seq_no;

		cc = find_rec (incf, &incfRec, NEXT, "u");
	}
	abc_unlock (incf);
	return (EXIT_SUCCESS);
}

/*
 * Open Incf file. 
 */
void
OpenIncf (void)
{
	if (incf_openDone == FALSE)
	{
		char	*sptr;

		open_rec (incf, incf_list, INCF_NO_FIELDS, "incf_seq_id"); 
		incf_openDone = TRUE;

		/*
	 	* Get FIFO purge environment.
	 	*/
		sptr = chk_env ("SK_LEAVE_FIFO");
		envSkLeaveFifo	= (sptr == (char *)0) ? 3 : atoi (sptr);
	}
}
/*
 * Open Incf file. 
 */
void
OpenIncf2 (void)
{
	static	int	alias_done_before	=	FALSE;

	if (alias_done_before == FALSE)
	{
		alias_done_before = TRUE;
		abc_alias (incf2,incf);
	}
	if (incf2_openDone == FALSE)
	{
		open_rec (incf2,incf_list, INCF_NO_FIELDS, "incf_seq_id"); 
		incf2_openDone = TRUE;
	}
}
/*
 * Open inei file. 
 */
void
OpenInei (void)
{
	if (inei_openDone == FALSE)
	{
		open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
		inei_openDone = TRUE;
	}
}
/*
 * Open Insf file. 
 */
void
OpenInsf (void)
{
	if (insf_openDone == FALSE)
	{
		char	*sptr;

		open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no");
		insf_openDone = TRUE;

		/*
	 	* Get totalValue of SER_VALUE E(stimated A(ctual)
	 	*/
		sptr	=	chk_env ("SER_VALUE");
		if (sptr == (char *)0)
			strcpy (envSerValue, "A");
		else
			sprintf (envSerValue, "%-1.1s", sptr);
	}
}
/*
 * Open Insc file. 
 */
void
OpenInsc (void)
{
	if (insc_openDone == FALSE)
	{
		open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
		insc_openDone = TRUE;
	}
}
/*
 * Open Itff file. 
 */
void
OpenItff (void)
{
	if (itff_openDone == FALSE)
	{
		open_rec (itff, itff_list, ITFF_NO_FIELDS, "itff_itff_hash");
		itff_openDone = TRUE;
	}
}
