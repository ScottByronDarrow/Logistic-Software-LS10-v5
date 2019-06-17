/*
 *	Copyright	(C)	1999	-	1999	LogisticSoftware
 *
 *	$Id: LocHeader.h,v 5.6 2001/12/06 05:29:25 scott Exp $
 *
 *	$Log: LocHeader.h,v $
 *	Revision 5.6  2001/12/06 05:29:25  scott
 *	Updated to allow purchase returns that are allocated to an original purchase order to select lot information. The lot display will only show those Location/Lot records that belong to the original receipt. This caters for both multiple receipts of the same product on the same purchase order AND Locations/Lots being split after receipt.
 *	
 *	Revision 5.5  2001/10/09 23:19:10  scott
 *	Updated from Scotts Machine
 *	
 *
 */
#ifndef	MAX_LOTS
#define	MAX_LOTS	200
#endif

#ifndef	INP_VIEW
#define	INP_VIEW 		0
#endif

#ifndef	INP_AUTO
#define	INP_AUTO		1
#endif

#ifndef	INP_MANUAL	
#define	INP_MANUAL		2
#endif

#ifndef	INP_NONE
#define	INP_NONE 		3
#endif


#ifndef LL_LOAD_INV
#define	LL_LOAD_INV		1
#endif

#ifndef LL_LOAD_CRD
#define	LL_LOAD_CRD		2
#endif

#ifndef LL_LOAD_SO
#define	LL_LOAD_SO 		3
#endif

#ifndef LL_LOAD_TRN
#define	LL_LOAD_TRN		4
#endif

#ifndef LL_LOAD_CM
#define	LL_LOAD_CM 		5
#endif

#ifndef LL_LOAD_MS
#define	LL_LOAD_MS 		6
#endif

#ifndef LL_LOAD_GL
#define	LL_LOAD_GL 		7
#endif

#ifndef	_LocHeader_h
#define	_LocHeader_h

extern	char	*GetExpiry			(int, int),	
				*GetLUOM			(int, int),	
				*GetLoc				(int, int),	
				*GetLotNo			(int, int),	
				*GetSLotNo			(int, int),	
				*GetUOM				(int, int),	
				*GetLocStat  		(int, int),
				*DefaultLocation	(long),
				DfltLotNo [8],
				LLQtyFormat [15],
				LLQtyMask [10],
				PickLocation [31],
				ReceiptLocation [31],
				StockTake [2],
				ValidLocations [31],
				llctAdesConf [2],
				llctAllLocs [2],
				llctAltUom [2],
				llctAutoAll [2],
				llctCcnInp [2],
				llctCredit [2],
				llctDefault [11],
				llctDesConf [2],
				llctExpItems [2],
				llctInput [2],
				llctInvoice [2],
				llctPcIssue [2];

extern	float	CalcAlloc			(long, int),
				GetBaseQty			(int, int),	
				GetCnvFct			(int, int),	
				GetQty				(int, int),
				GetAvailQty			(int, int),
				GetPackQty 	 		(int, int),
				GetChgWgt	 		(int, int),
				GetGrossWgt	 		(int, int),
				GetCuMetres	 		(int, int),
				qtyAllocated,
				qtyRequired,
				errorQuantity;

extern	int		CheckLocation		(long, char *, char *),
				DisplayLL			(int, int, int, int, long, long, long, char *, float, float, long, int, int, char *),
				DspLLStake 			(int,int,int,int,long,long,long,float,char *,long,char *,char *, int),
				DspLLTrans 			(int,int,int,int,long,long,long,char *,float, char *,int),
				FindLocation		(long, long, char *, char *, long *),
				FindLotNo			(long, long, char *, long *),
				IgnoreAvailChk,
				IgnoreTotal,
				InputExpiry,
				LLInputClear,	
				LLReturns,
				LL_EditDate,
				LL_EditLoc,	
				LL_EditLot,	
				LL_EditSLot,	
				LL_Valid			(int, int),
				Load_LL_Lines 		(int,int, long,long,char *,float,int),
				LoadLocation 		(int, long, long, char *, float, float),
				PoLoad 				(int, long, long, char *, float, float),
				MULT_LOC,
				SK_BATCH_CONT,
				locStatusIgnore;

extern	long	GetHHUM				(int, int),	
				GetHHWH				(int, int),	
				GetINLO				(int, int),
				GetSKND				(int, int),
				NextThreePlNo		(long),
				currentInloHash;

extern	void	AllocLotLocation 	(int,int,int,long),
				ClearLotsInfo 		(long),
				CloseLocation		(void),
				InLotLocation		(long, long, long, char *, char *, char *, char *, char *, char *, float, float, char *, float, float, float, float, long),
				LLAddList 			(long,long,long,long,char *,char *,char *,char *,char *,char *,float,char *,float,float,float, long, char *, float, float, float, float, long),
				LotClear 			(int),
				LotMove				(int, int),
				OpenLocation		(long),
				OutLotLocation		(long, long, long, char *, char *, char *, char *, char *, char *, float, float, char *, float, float, float, float,long),
				PostLotLocation 	(long, long, long, char *, char *,char *,char *,char *,char *,float,float, int, char *, float, float, float, float, long),
				PutNoPlateData 		(int, int, char *, float,float,float,float,long),
				ProcList 			(int, float),
				ReadLLCT			(long),
				SearchLOC			(int, long, char *),
				SearchLomr			(long, char *),
				SetOpID				(int),
				TransLocation 		(int, long),
				UpdateItlo 			(int,long,long),
				UpdateLotLocation	(int, int),
				AllocationRestore	(void),
				AllocationComplete	(void);

#endif	/*	_LocHeader_h	*/
