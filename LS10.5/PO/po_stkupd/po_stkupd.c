/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_stkupd.c,v 5.14 2001/12/06 05:22:37 scott Exp $
|  Program Name  : (po_stkupd.c)
|  Program Desc  : (Update Goods Receipted to Stock.)
|---------------------------------------------------------------------|
| $Log: po_stkupd.c,v $
| Revision 5.14  2001/12/06 05:22:37  scott
| Updated to allow purchase returns that are allocated to an original purchase order to select lot information. The lot display will only show those Location/Lot records that belong to the original receipt. This caters for both multiple receipts of the same product on the same purchase order AND Locations/Lots being split after receipt.
|
| Revision 5.13  2001/11/07 07:19:00  robert
| Updated to fix focus problem on LS10-GUI
|
| Revision 5.12  2001/10/24 07:26:39  francis
| Updated as wrong sknd is being updated when its PO return
| with PO matching.
|
| Revision 5.11  2001/10/22 04:14:51  scott
| Updated as wrong variable being used for allocated return to PO.
|
| Revision 5.10  2001/10/19 03:05:21  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.9  2001/10/10 03:28:40  scott
| Updated to ensure movements performed by lot if required
|
| Revision 5.8  2001/10/09 22:57:22  scott
| Updated for purchase returns
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_stkupd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_stkupd/po_stkupd.c,v 5.14 2001/12/06 05:22:37 scott Exp $";

#include	<pslscr.h>
#include	<time.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<Costing.h>
#include	<arralloc.h>
#include	"schema"

#define		FIFO	 (inmr_rec.costing_flag [0] == 'F' || \
			  		  inmr_rec.costing_flag [0] == 'I')
#define		LCOST	 (inmr_rec.costing_flag [0] == 'L')
#define		ACOST	 (inmr_rec.costing_flag [0] == 'A')

#define		DROPSHIP (pogh_rec.drop_ship [0] == 'Y')

#define		QCITEM	 (inmr_rec.qc_reqd [0] == 'Y')

#define		RETURN_LINE (pogl_rec.qty_rec < 0.00)

#define		S_DUMM	0
#define		S_HEAD	1

/*
 * LOCAL_CENTS used instead of CENTS as CENTS function performs a round(x). 
 * although when you look at the data it looks the same informix still holds
 * the required decimal placed in the background.                           
 */
#define		LOCAL_CENTS(d)	(d * 100.0)

	/*
	 * Special fields and flags.
	 */
	FILE	*fout;

	int		printerNumber		= 1,
			envQcApply			= FALSE,
			envPoVarRept		= 0,
			envSkGrinNoPlate	= 0,
			envPoGrinCosts 		= 1,
			envPoRecStdUom 		= 0,
			envAllowZeroCost 	= 0,
			envIkeaHk			= FALSE,
			envPoReturnApply 	= 0,
			firstPurchaseOrder 	= TRUE,
			processPID			= 0,
			allocCnt			= 0;

	char	systemDate [11];
	long	lsystemDate 		= 0L,
			origionalPO			= 0L;

	char 	*ccmr2 	= "ccmr2",
	    	*incc2 	= "incc2",
	    	*pogl2 	= "pogl2",
	    	*pogh2 	= "pogh2",
	    	*inmr2 	= "inmr2",
	    	*data 	= "data";

	double	totalReceipted = 0.00;
	int		auditFileOpen = FALSE;

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	ccmrRecord	ccmr2_rec;
	struct	poghRecord	pogh_rec;
	struct	poghRecord	pogh2_rec;
	struct	poglRecord	pogl_rec;
	struct	poglRecord	pogl2_rec;
	struct	pocaRecord	poca_rec;
	struct	poshRecord	posh_rec;
	struct	inexRecord	inex_rec;
	struct	inmrRecord	inmr_rec;
	struct	inmrRecord	inmr2_rec;
	struct	inccRecord	incc_rec;
	struct	inccRecord	incc2_rec;
	struct	inumRecord	inum_rec;
	struct	inwuRecord	inwu_rec;
	struct	ffdmRecord	ffdm_rec;
	struct	sknhRecord	sknh_rec;
	struct	skndRecord	sknd_rec;
	struct	inloRecord	inlo_rec;
	struct	polnRecord	poln_rec;
	struct	inlaRecord	inla_rec;

	char	*exCodes [] = {
							"NYP",
							"R/P",
							"",
					      };

	char	dropShipment [2];

    float	GetBrClosing (long),
		    CnvFct		= 1.00;

    char	UOM [5];

#include	<proc_sobg.h>
#include	<LocHeader.h>
#include	<MoveRec.h>
#include    <MoveAdd.h>

/*
 * Function Declarations
 */
float 	GetBrClosing 		(long);
int 	IntFindInmr 		(long);
int		spec_valid			(int);
int		heading				(int);
void 	AddFffm 			(float, long, long);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	IntFindSuper 		(char *);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintAudit 			(double);
void 	PrintInex 			(void);
void 	Process 			(char *);
void 	ProcessPidFunc 		(char *);
void 	ProcessAudit 		(void);
void 	ProcessNoPlate 		(void);
void 	ProcessPogl 		(long, char *);
void 	ProcessPosh 		(long);
void 	ProcessStock 		(long, long, char *);
void 	UnallocatedReturn 	(long, float);
void 	shutdown_prog 		(void);

struct 
{
	char    dummy [11];
	char	sGrn [sizeof pogh_rec.gr_no];
	char	eGrn [sizeof pogh_rec.gr_no];
	char	sDesc [6];
	char	eDesc [6];
	Date	sDate;
	Date	eDate;
} local_rec;


static  struct  var vars [] =
{
    {S_HEAD, LIN, "sGrn", 2, 25, CHARTYPE,
        "AAAAAAAAAAAAAAA", "          ",
        "", " ", "Start Goods Receipt No :", 
		" Enter Start Number - Search Available ",
		YES, NO,  JUSTLEFT, "", "", local_rec.sGrn},
    {S_HEAD, LIN, "sDesc", 2, 45, CHARTYPE,
        "AAAAA", "          ",
        "", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.sDesc},
    {S_HEAD, LIN, "eGrn", 3, 25, CHARTYPE,
        "AAAAAAAAAAAAAAA", "          ",
        "", " ", "End   Goods Receipt No :", 
		" Enter End Number - Search Available ",
		YES, NO,  JUSTLEFT, "", "", local_rec.eGrn},
    {S_HEAD, LIN, "eDesc", 3, 45, CHARTYPE,
        "AAAAA", "          ",
        "", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.eDesc},
    {S_HEAD, LIN, "sDate", 4, 25, EDATETYPE,
        "DD/DD/DD", "          ",
        "", " ", "Start Receipt Date     :", 
		" Enter Start Date - Search Available ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.sDate},
    {S_HEAD, LIN, "eDate", 5, 25, EDATETYPE,
        "DD/DD/DD", "          ",
        "", " ", "End   Receipt Date     :", 
		" Enter End Date - Search Available ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.eDate},

	{S_DUMM, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

struct grnStruct 
{
	char	sGrn [sizeof pogh_rec.gr_no];
	char	eGrn [sizeof pogh_rec.gr_no];
	Date	sDate;
	Date	eDate;
};

void 	SrchPogh  					(char *);
void 	SetupVars 					(void);
struct 	grnStruct ValidateFields 	(void);

struct	tagAllocation {	
	char	location	[sizeof inlo_rec.location];
	char	lotNo		[sizeof inlo_rec.lot_no];
	char	slotNo		[sizeof inlo_rec.slot_no];
	char	uom			[sizeof inlo_rec.uom];
	char	locType		[sizeof inlo_rec.loc_type];
	char	locStatus	[sizeof inlo_rec.loc_status];
	long	hhwhHash;
	long	hhumHash;
	long	inloHash;
	long	skndHash;
	long	hhglHash;
	float	cnvFct;
	float	qtyAlloc;
} *allocRec;

DArray	allocationDetail;

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	char	purchaseStatus [2];
	char	*sptr;

	if (argc < 4)	
	{
		print_at (0,0,"Usage : %s <printerNumber> <purchase status> <direct delivery [Y)es/N)o]> [Optional PID]", argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);
	sprintf (purchaseStatus, "%-1.1s", argv [2]);
	sprintf (dropShipment,  "%-1.1s", argv [3]);
	if (argc == 5)
		processPID	=	atoi (argv[4]);

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();
	init_scr ();

	envQcApply 		 = (sptr = chk_env ("QC_APPLY")) 	 	? atoi (sptr) : 0;
	envPoVarRept 	 = (sptr = chk_env ("PO_VAR_REPT"))   	? atoi (sptr) : 1;
	envSkGrinNoPlate = (sptr = chk_env ("SK_GRIN_NOPLATE")) ? atoi (sptr) : 1;
	envPoGrinCosts 	 = (sptr = chk_env ("PO_GRIN_COSTS"))  	? atoi (sptr) : 1;
	envPoRecStdUom 	 = (sptr = chk_env ("PO_REC_STD_UOM"))	? atoi (sptr) : 0;
	envAllowZeroCost = (sptr = chk_env ("ALLOW_ZERO_COST"))	? atoi (sptr) : 0;
	envIkeaHk		 = (sptr = chk_env ("IKEA_HK"))  		? atoi (sptr) : 0;
	envPoReturnApply = (sptr = chk_env ("PO_RETURN_APPLY")) ? atoi (sptr) : 0;

	OpenDB ();
	
	SetupVars ();
		
	/*
	 * Process Status of Receipt or Transmit. One manual one automatic.
	 */
	if (purchaseStatus [0] == 'R')
	{
		dsp_screen (" Updating Goods Receipt to Stock ",
					 		comm_rec.co_no, comm_rec.co_name);
		OpenAudit ();		
		Process (purchaseStatus);
	}
	else if (purchaseStatus [0] == 'T')
	{
		OpenAudit ();
		ProcessPidFunc (purchaseStatus);
	}
	else
	{
		SETUP_SCR (vars);
    	init_scr  ();
    	set_tty   ();
    	set_masks ();
    	init_vars (S_HEAD);

   		while (!prog_exit)
    	{
       		search_ok  = TRUE;
        	init_ok    = TRUE;
        	entry_exit = FALSE;
        	edit_exit  = FALSE;
        	restart    = FALSE;
        	prog_exit  = FALSE;
   
        	init_vars (S_HEAD);
        	heading   (S_HEAD);
        	entry     (S_HEAD);
   
        	if (restart || prog_exit)
           		continue;

			heading (S_HEAD);
			scn_display (S_HEAD);
			edit (S_HEAD);
	        if (restart)
   	        	continue;

			prog_exit = TRUE;

			if (!processPID)
			{
				dsp_screen (" Reprinting Goods Receipt Audit.",
						 		comm_rec.co_no, comm_rec.co_name);
			}

			if (!auditFileOpen)
				OpenAudit ();
			
			Process (purchaseStatus);
		}
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
	CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}
/*
 * Open data base files.
 */
void
OpenDB (void)
{
	MoveOpen	=	TRUE;
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);
	abc_alias (incc2, incc);
	abc_alias (ccmr2, ccmr);
	abc_alias (pogl2, pogl);
	abc_alias (pogh2, pogh);

	open_rec (pogh, pogh_list, POGH_NO_FIELDS, (processPID) ? "pogh_pid_id"
															: "pogh_up_id");
	open_rec (pogh2,pogh_list, POGH_NO_FIELDS, "pogh_hhgr_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (pogl2,pogl_list, POGL_NO_FIELDS, "pogl_hhpl_hash");
	open_rec (poca, poca_list, POCA_NO_FIELDS, "poca_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (ccmr2,ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2,incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
    open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
    open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
    open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
    open_rec (inla, inla_list, INLA_NO_FIELDS, "inla_hhgl_id");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
	if (envSkGrinNoPlate)
	{
		open_rec (sknh, sknh_list, SKNH_NO_FIELDS, "sknh_id_no");
		open_rec (sknd, sknd_list, SKND_NO_FIELDS, "sknd_id_no");
	}

	strcpy (ccmr2_rec.co_no, comm_rec.co_no);
	strcpy (ccmr2_rec.est_no,comm_rec.est_no);
	strcpy (ccmr2_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr2, &ccmr2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr2, "DBFIND");
	
	OpenIncf ();
	OpenInsf ();
	abc_selfield (insf, "insf_id_no2");

	OpenLocation(ccmr2_rec.hhcc_hash);
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	if (envSkGrinNoPlate)
	{
		abc_fclose (sknh);
		abc_fclose (sknd);
	}
	abc_fclose (pogh);
	abc_fclose (pogh2);
	abc_fclose (posh);
	abc_fclose (pogl);
	abc_fclose (pogl2);
	abc_fclose (poca);
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (insf);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (inmr2);
	abc_fclose (inwu);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (poln);
	abc_fclose (inla);
	abc_fclose ("move");
	CloseLocation();
	abc_dbclose (data);
}
/*
 * Process goods receipts by process ID passed.
 */
void
ProcessPidFunc (
	char	*purchaseStatus)
{
	strcpy  (pogh_rec.co_no,  comm_rec.co_no);
	pogh_rec.pid	=	(long) processPID;
	cc = find_rec (pogh, &pogh_rec, GTEQ, "u");
	while (!cc && 
		   !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
	       pogh_rec.pid == (long) processPID)
	{
		/*
		 * Stock not updated but G/L updated.
		 */
		if (pogh_rec.pur_status [0] == purchaseStatus [0] && 
			pogh_rec.gl_status [0]  != purchaseStatus [0])
		{
			if (!firstPurchaseOrder)
				fprintf (fout, ".PA\n");

			fprintf (fout, ".PD! G O O D S   R E C E I P T   N U M B E R  (%15.15s)                                ", pogh_rec.gr_no);
			if (MULT_LOC)
				fprintf (fout, "           ");

			fprintf (fout, "                            ");
			if (envPoGrinCosts)
				fprintf (fout, "                          !\n");
			else
				fprintf (fout, "!\n");
			
			ProcessPogl (pogh_rec.hhgr_hash, purchaseStatus);

			strcpy (pogh_rec.pur_status, "U");
			cc = abc_update (pogh, &pogh_rec);
			if (cc)
				file_err (cc, pogh, "DBUPDATE");

			if (pogh_rec.hhsh_hash != 0L)
				ProcessPosh (pogh_rec.hhsh_hash);

			firstPurchaseOrder = FALSE;
		}
		cc = find_rec (pogh, &pogh_rec, NEXT, "u");
	}
	abc_unlock (pogh);
	recalc_sobg ();
}

/*
 * Standard goods receipt processing routine.
 */
void
Process (
	char	*purchaseStatus)
{
	char	work [256];

	struct grnStruct grn = ValidateFields ();

	abc_selfield (pogh, "pogh_up_id");
	strcpy  (pogh_rec.co_no,  comm_rec.co_no);
	strcpy  (pogh_rec.br_no,  comm_rec.est_no);
	sprintf (pogh_rec.gr_no, "%-15.15s", grn.sGrn);
	pogh_rec.hhsu_hash = 0L;
	pogh_rec.hhsh_hash = 0L;
	cc = find_rec (pogh, &pogh_rec, GTEQ, "u");
	while (!cc && 
		   !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
	       !strcmp (pogh_rec.br_no, comm_rec.est_no) &&
	    	strcmp (pogh_rec.gr_no, grn.eGrn) <= 0)
	{
		if (pogh_rec.date_raised < grn.sDate ||
			pogh_rec.date_raised > grn.eDate)
		{
			abc_unlock (pogh);
			cc = find_rec (pogh, &pogh_rec, NEXT, "u");
			continue;
		}
		/*
		 * Goods Received
		 */
		if (pogh_rec.pur_status [0] == purchaseStatus [0] && 
			pogh_rec.gl_status [0]  != purchaseStatus [0])
		{
			if (!firstPurchaseOrder)
				fprintf (fout, ".PA\n");

			if (DROPSHIP && dropShipment [0] == 'Y')
			{
				sprintf (work, ".PD| G O O D S   R E C E I P T   N U M B E R   ");
				strcpy  (err_str, work);
				sprintf (work, "(%15.15s)", pogh_rec.gr_no);
				strcat  (err_str, work);
				sprintf (work, "   Direct Delivery Confirmation Only ");
				strcat  (err_str, work);
				if (MULT_LOC)
					sprintf (work, "- Stock NOT Updated.   %34.34s |\n", " ");
				else
					sprintf (work, "- Stock NOT Updated.   %23.23s |\n", " ");
				strcat  (err_str, work);
				fprintf (fout, err_str);
			}
			else
			if (!DROPSHIP && dropShipment [0] != 'Y')
			{
				fprintf (fout, ".PD! G O O D S   R E C E I P T   N U M B E R  (%15.15s)                                ", pogh_rec.gr_no);
				if (MULT_LOC)
					fprintf (fout, "           ");

				fprintf (fout, "                            ");
				if (envPoGrinCosts)
					fprintf (fout, "                          !\n");
				else
					fprintf (fout, "!\n");
			}
			else
			{
				abc_unlock (pogh);
				cc = find_rec (pogh, &pogh_rec, NEXT, "u");
				continue;
			}
			ProcessPogl (pogh_rec.hhgr_hash, purchaseStatus);

			/*
			 * Process Status of Receipt or Transmit. One manual one automatic.
			 */
			if (purchaseStatus [0] == 'R' || purchaseStatus [0] == 'T')
			{
				strcpy (pogh_rec.pur_status, "U");
				cc = abc_update (pogh, &pogh_rec);
				if (cc)
					file_err (cc, pogh, "DBUPDATE");

				if (pogh_rec.hhsh_hash != 0L)
					ProcessPosh (pogh_rec.hhsh_hash);
			}
			else
				abc_unlock (pogh);

			firstPurchaseOrder = FALSE;
		}
		cc = find_rec (pogh, &pogh_rec, NEXT, "u");
	}
	abc_unlock (pogh);
	recalc_sobg ();
}

/*
 * Process shipment header.
 */
void
ProcessPosh (
	long	hhshHash)
{
	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh, &posh_rec, COMPARISON, "u");
	if (!cc)
	{
		strcpy (posh_rec.status, "R");
		cc = abc_update (posh, &posh_rec);
		if (cc)
			file_err (cc, "posh", "DBUPDATE");
	}
	else
		abc_unlock (posh);
}

/*
 * Process goods receipts lines.
 */
void
ProcessPogl (
	long	hhglHash, 
	char	*purchaseStatus)
{
	double	value	= 0.00;
	double	extend	= 0.00;


	pogl_rec.hhgr_hash 	= hhglHash;
	pogl_rec.line_no 	= 0;

	cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
	while (!cc && pogl_rec.hhgr_hash == hhglHash)
	{
		allocCnt = 0;

	   	if (pogl_rec.pur_status [0] == purchaseStatus [0])
		{
		 	/*
		 	 * This first block deals with purchase returns lines that are 
		 	 * allocated to an existing purchase orders. These lines are 
			 * processed differently as they would only have one location.
		 	 */
			origionalPO	= 0L;
			if (RETURN_LINE)
			{
				poln_rec.hhpl_hash	=	pogl_rec.hhpl_hash;
				cc = find_rec (poln, &poln_rec, COMPARISON, "r");
				if (!cc && poln_rec.hhpl_orig > 0L)
					origionalPO	= poln_rec.hhpl_orig;
			}
			/*
			 * Process unallocated returns.
			 */
			if (RETURN_LINE)
			{
				ArrAlloc 
				(
					&allocationDetail, 
					&allocRec,
					sizeof (struct tagAllocation),
					10
				);
				UnallocatedReturn (pogl_rec.hhgl_hash, pogl_rec.qty_rec * -1);
			}
			cc = IntFindInmr (pogl_rec.hhbr_hash);
			if (cc)
			{
				abc_unlock (pogl);
				cc = find_rec (pogl, &pogl_rec, NEXT, "u");
				continue;
			}

			if (envPoRecStdUom)
				pogl_rec.hhum_hash	=	inmr_rec.std_uom;

			if (!DROPSHIP)
		   	{
				ProcessStock 
				(
					inmr_rec.hhbr_hash,
				  	pogl_rec.hhcc_hash,
					purchaseStatus
				);
				/*
				 * Process Status of Receipt or Transmit.
				 */
				if (purchaseStatus [0] == 'R' || purchaseStatus [0] == 'T')
				{
					/* 
					 * Process audit.
					 */
					if (envPoVarRept)
						ProcessAudit ();

					/*
					 * Process number plates.
					 */
					if (envSkGrinNoPlate)
						ProcessNoPlate ();
	
					strcpy (pogl_rec.pur_status, "U");
					cc = abc_update (pogl, &pogl_rec);
					if (cc)
						file_err (cc, pogl, "DBUPDATE");
				}
				else
					abc_unlock (pogl);
			}
			else
			{
				value	=	twodec (pogl_rec.land_cst);
				value 	= 	out_cost (value, inmr_rec.outer_size);
				extend 	= value * (double) pogl_rec.qty_rec;
				extend 	= twodec (extend);
				PrintAudit (extend);
				totalReceipted += extend;

				/*
				 * Process Status of Receipt or Transmit.
				 */
				if (purchaseStatus [0] == 'R' || purchaseStatus [0] == 'T')
				{
					strcpy (pogl_rec.pur_status, "U");
					cc = abc_update (pogl, &pogl_rec);
					if (cc)
						file_err (cc, pogl, "DBUPDATE");
				}
				else
					abc_unlock (pogl);
			}
		}
		else
			abc_unlock (pogl);

		if (RETURN_LINE)
			ArrDelete (&allocationDetail);

		cc = find_rec (pogl, &pogl_rec, NEXT, "u");
	}
	abc_unlock (pogl);
}

/*
 * Check for item supercession.
 */
void
IntFindSuper (
	char	*itemNo)
{
	if (!strcmp (itemNo, "                "))
		return;

	strcpy (inmr2_rec.co_no, comm_rec.co_no);
	sprintf (inmr2_rec.item_no, "%-16.16s", itemNo);
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (!cc)
		IntFindSuper (inmr2_rec.supercession);
}

/*
 * Find Item.
 */
int
IntFindInmr (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	if (inmr_rec.hhsi_hash != 0L)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			return (cc);
	}

	if (strcmp (inmr_rec.supercession, "                "))
	{
		abc_unlock (inmr);
		IntFindSuper (inmr_rec.supercession);
		inmr_rec.hhbr_hash = inmr2_rec.hhbr_hash;
	}
	return (find_rec (inmr, &inmr_rec, COMPARISON, "u"));
}

/*
 * Process Stock.
 */
void
ProcessStock (
	long	hhbrHash,
	long	hhccHash,
	char	*purchaseStatus)
{
	int		i				=	0,
			addSerialItem 	= 	0;

	double 	oldQty 			= 0.00,
			xxQty 			= 0.00,
			oldCost 		= 0.00,
			newCost 		= 0.00,
			extend 			= 0.00,
			value 			= 0.00,
			newActCost 		= 0.00,
			qtyRemaining	= 0.00,
			qtyReturn		= 0.00,
			newActQty  		= 0.00;

	float	Weeks 			= 0.00,
			UnservedQty 	= 0.00;

	ccmr_rec.hhcc_hash	=	hhccHash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	newCost = pogl_rec.land_cst;

	/*
	 * Process Status of Receipt or Transmit.
	 */
	if (purchaseStatus [0] == 'R' || purchaseStatus [0] == 'T')
	{
		incc_rec.hhcc_hash = hhccHash;
		incc_rec.hhbr_hash = hhbrHash;

		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, incc, "DBFIND");

		if (!RETURN_LINE)
		{
			cc = FindInei (hhbrHash, ccmr_rec.est_no, "u");
			if (cc)
			{
				memset (&ineiRec, 0, sizeof (ineiRec));

				ineiRec.hhbr_hash	= hhbrHash;
				strcpy (ineiRec.est_no, ccmr_rec.est_no);
				strcpy (ineiRec.stat_flag, "0");
				cc = abc_add (inei, &ineiRec);
				if (cc)
					file_err (cc, inei, "DBADD");

				cc = FindInei (hhbrHash, ccmr_rec.est_no, "u");
				if (cc)
					file_err (cc,inei, "DBFIND");
			}
		}
		/*
		 * -ve quantity is a credit or issue not a purchase.
		 */
		if (RETURN_LINE)
		{
			incc_rec.issues		+= (pogl_rec.qty_rec * -1);
			incc_rec.ytd_issues	+= (pogl_rec.qty_rec * -1);
		}
		else
		{
			incc_rec.pur		+= pogl_rec.qty_rec;
			incc_rec.ytd_pur 	+= pogl_rec.qty_rec;
			if (envQcApply && QCITEM)
				incc_rec.qc_qty += pogl_rec.qty_rec;
		}

		oldQty = xxQty = 	GetBrClosing (incc_rec.hhbr_hash);
		
		incc_rec.closing_stock =	incc_rec.opening_stock +
								  	incc_rec.pur +
								  	incc_rec.receipts +
								  	incc_rec.adj -
								  	incc_rec.issues -
								  	incc_rec.sales;

		/*
		 * If location on incc is blank then set to new receipts location.
		 */
		if (!strcmp (incc_rec.location, "          ") && MULT_LOC)
			strcpy (incc_rec.location, pogl_rec.location);
			
		/*
		 * If the product now has stock-on-hand and the out of stock date > 0L
		 * then add a record to ffdm providing the weeks demand for item is   
		 * less than 0.                                                      
		 */
		if (incc_rec.closing_stock > 0.00 && incc_rec.os_date != 0L)
		{
			Weeks = (float)((lsystemDate - incc_rec.os_date) / 7.00);
		
			if (incc_rec.wks_demand > 0.00)
			{
				UnservedQty = Weeks * incc_rec.wks_demand;

				AddFffm 
				(
					UnservedQty,
					incc_rec.hhbr_hash,
					incc_rec.hhcc_hash
				);
				incc_rec.os_date 	= 0L;
				incc_rec.os_ldate 	= 0L;
				if (!strcmp (inmr_rec.ex_code, "o/s"))
					sprintf (inmr_rec.ex_code, "%-3.3s", " ");
			}
		}
		cc = abc_update (incc, &incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");

		/*
		 * Find Warehouse unit of measure file.
		 */
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	pogl_rec.hhum_hash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	pogl_rec.hhum_hash;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	pogl_rec.hhum_hash;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		if (RETURN_LINE)
			inwu_rec.issues	+= (pogl_rec.qty_rec * -1);
		else
			inwu_rec.pur	+= pogl_rec.qty_rec;

		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu, &inwu_rec);
		if (cc)
			file_err(cc, inwu, "DBUPDATE");
	}
		
	value 	= twodec (pogl_rec.land_cst);
	value 	= out_cost (value, inmr_rec.outer_size);
	extend 	= value * (double) pogl_rec.qty_rec;
	extend 	= twodec (extend);
	
	if (!processPID)
		dsp_process (" Item No. : ", inmr_rec.item_no);

	PrintAudit (extend);
	totalReceipted += extend;

	/*
	 * Process Status of Receipt or Transmit and quantity > 0.00
	 */
	if ((purchaseStatus [0] != 'R' && purchaseStatus [0] != 'T')  ||
		 pogl_rec.qty_rec == 0.00) 
	{
		abc_unlock (inmr);
		return;
	}
	if (!RETURN_LINE)
	{
		strcpy (ineiRec.est_no, ccmr_rec.est_no);
		ineiRec.prev_cost = (ACOST)	? ineiRec.avge_cost
									: ineiRec.last_cost;
		ineiRec.last_cost 	= twodec (newCost);
		ineiRec.date_lcost 	= pogl_rec.rec_date;
		oldCost 			= ineiRec.avge_cost;
		newActCost 			= newCost;
		newActQty  			= pogl_rec.qty_rec;
		if (oldQty < 0.00) 
			xxQty = 0.00;

		if (oldQty + newActQty == 0.00) 
			ineiRec.avge_cost = newActCost;
		else 	
		{
			if (xxQty + newActQty <= 0.00)
			{
				if (envAllowZeroCost)
					ineiRec.avge_cost = 0.00;
				else
					ineiRec.avge_cost = oldCost;
			}
			else 
				ineiRec.avge_cost = (((xxQty * oldCost) 
									  + (newActCost * newActQty)) 
									  / (xxQty + newActQty));
		}
		ineiRec.lpur_qty = (float) (newActQty);
		ineiRec.hhis_hash = pogh_rec.hhsu_hash;

		cc = abc_update (inei, &ineiRec);
		if (cc) 
			file_err (cc, inei, "DBUPDATE");
	}
	
	/*
	 * Update Fifo/Lifo records for credits.
	 */
	if (FIFO && RETURN_LINE)
	{
		int		processSecondOption	=	TRUE;

		/*
		 * Goods receipt quantity -ve so reverse to use.
		 */
		qtyReturn = pogl_rec.qty_rec * -1;

		/*
		 * This code will find the origional FIFO/LIFO record and update 
		 * for purchase return line.
		 */
		if (origionalPO)
		{
			/*
			 * Find goods receipt line from origional p/o.
			 */
			pogl2_rec.hhpl_hash	=	origionalPO;
			cc = find_rec (pogl2, &pogl2_rec, COMPARISON, "r");
			if (!cc)
			{
				/*
				 * Find goods receipt Header from origional p/o.
				 */
				pogh2_rec.hhgr_hash	=	pogl2_rec.hhgr_hash;
				cc = find_rec (pogh2, &pogh2_rec, COMPARISON, "r");
				if (!cc)
				{
					abc_selfield (incf, "incf_id_no_2");
					incfRec.hhwh_hash	=	incc_rec.hhwh_hash;
					strcpy (incfRec.gr_number, pogh2_rec.gr_no);
					cc = find_rec (incf, &incfRec, COMPARISON, "u");
					if (!cc)
					{
						incfRec.fifo_qty -= qtyReturn;
						strcpy (incfRec.stat_flag, "Z");
						cc = abc_update (incf, &incfRec);
						if (cc)             
							file_err (cc, incf, "DBUPDATE");

						processSecondOption = FALSE;
					}
					/*
					 * Failed so second option processing applies.
					 */
					else
						abc_unlock (incf);

					abc_selfield (incf, "incf_seq_id");
				}
			}
		}
		/*
		 * Failed first option so second option applies.
		 */
		if (processSecondOption ==	TRUE)
		{
			qtyReturn = pogl_rec.qty_rec * -1;

			/*
			 * Process FIFO or LIFO depending on PO_RETURN_APPLY environment.
			 */
			cc	=	FindIncf 
					(
						incc_rec.hhwh_hash, 
						(envPoReturnApply) ? TRUE : FALSE, 
						"u"
					);
			while (!cc && incfRec.hhwh_hash == incc_rec.hhwh_hash &&  
					qtyReturn > 0.00)
			{
				qtyRemaining = incfRec.fifo_qty - qtyReturn;
				if (qtyRemaining > 0.00)
				{
					incfRec.fifo_qty = (float) qtyRemaining;
					cc = abc_update (incf, &incfRec);
					if (cc)             
						file_err (cc, incf, "DBUPDATE");

					qtyReturn	=	0.00;
				}
				else
				{
					incfRec.fifo_qty = 0;
					cc = abc_update (incf, &incfRec);
					if (cc)             
						file_err (cc, incf, "DBUPDATE");

					qtyReturn = qtyRemaining * -1;
				}
				cc	=	FindIncf (0L,(envPoReturnApply) ? TRUE : FALSE, "u");
			}
		}
		abc_unlock (incf);
	}

	if (FIFO && !RETURN_LINE)
	{
		/*
		 * Product is in stock take mode so control record needs to be checked.
		 */
		if (incc_rec.stat_flag [0] >= 'A' && incc_rec.stat_flag [0] <= 'Z')
		{
			while 
			(
				CheckInsc 
				(
					incc_rec.hhcc_hash, 
					pogl_rec.rec_date,
					incc_rec.stat_flag
				)
			) pogl_rec.rec_date 	+=	1L;
		}
		cc	=	AddIncf	
				(
					incc_rec.hhwh_hash,
					pogl_rec.rec_date,
					twodec (newCost),
					twodec (newCost),
					pogl_rec.qty_rec,
					pogh_rec.gr_no,
					pogl_rec.fob_nor_cst,
					pogl_rec.frt_ins_cst,
					pogl_rec.duty,
					pogl_rec.licence,
					pogl_rec.lcost_load,
				   (pogl_rec.fob_nor_cst + pogl_rec.lcost_load 	+ 
					pogl_rec.duty 		 + pogl_rec.licence),
					"E"
				);
		if (cc)
			file_err (cc, incf, "DBADD");
	}

	if (inmr_rec.serial_item [0] == 'Y')
	{
		insfRec.hhwh_hash = incc_rec.hhwh_hash;
		insfRec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (insfRec.serial_no, "%-25.25s",pogl_rec.serial_no);
		addSerialItem = find_rec (insf, &insfRec, COMPARISON, "u");
		
		if (!RETURN_LINE)
		{
			strcpy (insfRec.receipted, "Y");
			sprintf (insfRec.location, "%-10.10s", pogl_rec.location);
			insfRec.date_in     = pogl_rec.rec_date;
			insfRec.exch_rate   = pogh_rec.exch_rate;
			insfRec.fob_fgn_cst = pogl_rec.fob_fgn_cst;
			insfRec.fob_nor_cst = pogl_rec.fob_nor_cst;
			insfRec.frt_ins_cst = pogl_rec.frt_ins_cst;
			insfRec.duty        = pogl_rec.duty;
			insfRec.licence     = pogl_rec.licence;
			insfRec.lcost_load  = pogl_rec.lcost_load;
			insfRec.land_cst    = pogl_rec.fob_nor_cst 
									+ pogl_rec.lcost_load 
									+ pogl_rec.duty 
									+ pogl_rec.licence;
	
			insfRec.istore_cost = twodec (newCost);
			insfRec.est_cost    = twodec (newCost);
			strcpy (insfRec.stat_flag, "E");
	
			strcpy (insfRec.po_number, pogl_rec.po_number);
			strcpy (insfRec.gr_number, pogh_rec.gr_no);
		}
		if (addSerialItem)
		{
			strcpy (insfRec.status, "F");
			cc = abc_add (insf, &insfRec);
			if (cc) 
				file_err (cc, insf, "DBADD");

			abc_unlock (insf);
		}
		else
		{
			if (RETURN_LINE)
				strcpy (insfRec.status, "S");

			cc = abc_update (insf, &insfRec);
			if (cc) 
				file_err (cc, insf, "DBUPDATE");
		}
	}
	/* 
	 * Process in's and out's for locations.
	 */
	if (MULT_LOC || SK_BATCH_CONT)
	{	
		/*
		 * Process purchase returns that ARE NOT allocated to an 
		 * origional purchase order.
		 */
		if (RETURN_LINE)
		{
			for (i = 0; i < allocCnt; i++)
			{
				strcpy (err_str, DateToString (pogl_rec.exp_date));
				InLotLocation
				(
					allocRec [i].hhwhHash,
					incc_rec.hhcc_hash,
					allocRec [i].hhumHash,
					allocRec [i].uom, 
					allocRec [i].lotNo,
					allocRec [i].slotNo,
					allocRec [i].location,
					allocRec [i].locType,
					err_str, 
					allocRec [i].qtyAlloc * -1,
					allocRec [i].cnvFct,
					allocRec [i].locStatus,
					pogl_rec.pack_qty,
					pogl_rec.chg_wgt,
					pogl_rec.gross_wgt,
					pogl_rec.cu_metre,
					0L
				);
				/*
				 * Log inventory movements.
				 */
				MoveAdd 
				(
					comm_rec.co_no,
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					inmr_rec.hhbr_hash,
					incc_rec.hhcc_hash,
					allocRec [i].hhumHash,
					pogl_rec.rec_date,
					3,
					allocRec [i].lotNo,
					inmr_rec.inmr_class,
					inmr_rec.category,
					pogl_rec.po_number,
					pogh_rec.gr_no,
					allocRec [i].qtyAlloc * -1,
					0.0,
					LOCAL_CENTS (twodec (newCost))
				);
			}
		}
		/*
		 * Process purchase orders.
		 */
		else
		{
			strcpy (err_str, DateToString (pogl_rec.exp_date));
			InLotLocation
			(
				incc_rec.hhwh_hash,
				incc_rec.hhcc_hash,
				pogl_rec.hhum_hash,
				UOM,
				pogl_rec.lot_no,
				pogl_rec.slot_no,
				pogl_rec.location,
				"B",
				err_str, 
				pogl_rec.qty_rec,
				CnvFct,
				"A",
				pogl_rec.pack_qty,
				pogl_rec.chg_wgt,
				pogl_rec.gross_wgt,
				pogl_rec.cu_metre,
				0L
			);
			/*
			 * Log inventory movements.
			 */
			MoveAdd 
			(
				comm_rec.co_no,
				ccmr_rec.est_no,
				ccmr_rec.cc_no,
				inmr_rec.hhbr_hash,
				incc_rec.hhcc_hash,
				pogl_rec.hhum_hash,
				pogl_rec.rec_date,
				5,
				pogl_rec.lot_no,
				inmr_rec.inmr_class,
				inmr_rec.category,
				pogl_rec.po_number,
				pogh_rec.gr_no,
				pogl_rec.qty_rec,
				0.0,
				LOCAL_CENTS (twodec (newCost))
			);
		}
	}
	inmr_rec.on_hand += pogl_rec.qty_rec;
	if (envQcApply && QCITEM && !RETURN_LINE)
		inmr_rec.qc_qty += pogl_rec.qty_rec;

	/*
	 * Clear the inmr_ex_code if the code is one of the codes in the table AND	
	 * the receipt puts the inmr_on_hand into positive.						|
	 */
	for (i = 0; inmr_rec.on_hand > 0.00 && strlen (exCodes [i]); i++)
	{
		if (!strcmp (exCodes [i], inmr_rec.ex_code))
		{
			strcpy (inmr_rec.ex_code, "   ");
			break;
		}
	}

	cc = abc_update (inmr, &inmr_rec);
	if (cc) 
		file_err (cc, inmr, "DBUPDATE");

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0,
		inmr_rec.hhbr_hash, 
		incc_rec.hhcc_hash, 
		0L, 
		0.00
	);

	return;
}

/*
 * Routine to print line item to the audit trail.
 */
void
PrintAudit (
	double extend)
{
	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00;

	double	workValue	=	0.00;

	int		i;
	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	PurCnvFct / StdCnvFct;
	strcpy (UOM, inum_rec.uom);

	strcpy (err_str, DateToString (pogl_rec.exp_date));

	workValue	=	twodec (pogl_rec.land_cst);
	if (RETURN_LINE && MULT_LOC)
	{
		for (i = 0; i < allocCnt; i++)
		{

			if (i == 0)
			{
				fprintf (fout, "|%15.15s",		pogl_rec.po_number);
				fprintf (fout, "|%15.15s",		pogh_rec.gr_no);
				fprintf (fout, "|%2.2s", 		ccmr_rec.est_no);
				fprintf (fout, "|%2.2s - %9.9s",ccmr_rec.cc_no,
										  		ccmr_rec.acronym);
				fprintf (fout, "|%16.16s",		inmr_rec.item_no);
				fprintf (fout, "|%-24.24s",		pogl_rec.item_desc);
				if (MULT_LOC)
					fprintf (fout, "|%10.10s",	allocRec [i].location);
				fprintf (fout, "|%10.10s",	DateToString (pogl_rec.rec_date));
				fprintf (fout, "|%4.4s",		inum_rec.uom);
				fprintf (fout, "|%11.2f",		allocRec [i].qtyAlloc /
												allocRec [i].cnvFct);
				fprintf (fout, "|%12.4f", 		out_cost 
												(
													workValue, 
													inmr_rec.outer_size)
												);
				fprintf (fout, "|%12.2f|\n",	extend);
			}
			else
			{
				fprintf (fout, "|%15.15s",	" ");
				fprintf (fout, "|%15.15s",	" ");
				fprintf (fout, "|%2.2s", 	" ");
				fprintf (fout, "|%14.14s",	" ");
				fprintf (fout, "|%16.16s",	" ");
				fprintf (fout, "|%24.24s",	" ");
				if (MULT_LOC)
					fprintf (fout, "|%10.10s",	allocRec [i].location);
				fprintf (fout, "|%10.10s",	" ");
				fprintf (fout, "|%4.4s",	" ");
				fprintf (fout, "|%11.2f",	allocRec [i].qtyAlloc /
											allocRec [i].cnvFct);
				fprintf (fout, "|%12.12s",	" ");
				fprintf (fout, "|%12.12s|\n"," ");
			}
		}
	}
	else
	{
		fprintf (fout, "|%15.15s",			pogl_rec.po_number);
		fprintf (fout, "|%15.15s",			pogh_rec.gr_no);
		fprintf (fout, "|%2.2s", 			ccmr_rec.est_no);
		fprintf (fout, "|%2.2s - %9.9s",	ccmr_rec.cc_no,
									  		ccmr_rec.acronym);
		fprintf (fout, "|%16.16s",			inmr_rec.item_no);
		fprintf (fout, "|%-24.24s",			pogl_rec.item_desc);
		if (MULT_LOC)
			fprintf (fout, "|%10.10s",		pogl_rec.location);
	
		fprintf (fout, "|%10.10s",			DateToString (pogl_rec.rec_date));
		fprintf (fout, "|%4.4s",			inum_rec.uom);
		fprintf (fout, "|%11.2f",			pogl_rec.qty_rec / CnvFct);
		if (envPoGrinCosts)
		{
			fprintf (fout, "|%12.4f", out_cost (workValue, inmr_rec.outer_size));
			fprintf (fout, "|%12.2f|\n",	extend);
		}
		else
			fprintf (fout, "|\n");
	
		if (envIkeaHk == FALSE)
			PrintInex ();
	
		if (strcmp (pogl_rec.serial_no, "                         "))
		{
			fprintf (fout, "|               ");
			fprintf (fout, "|               ");
			fprintf (fout, "|  ");
			fprintf (fout, "|              ");
			fprintf (fout, "|SERIAL NUMBER %25.25s  ", pogl_rec.serial_no);
			if (MULT_LOC)
				fprintf (fout, "|          ");
			fprintf (fout, "|          ");
			fprintf (fout, "|    ");
			fprintf (fout, "|           ");
			if (envPoGrinCosts)
				fprintf (fout, "|            |            |\n");
			else
				fprintf (fout, "|\n");
		}
	}
	fflush (fout);
}

/*
 * Process Number Plate.
 */
void
ProcessNoPlate (void)
{
	int		updSknd,
			i;

	/*
	 * The next bit of code deals with purchase returns.
	 * It's more complicated then one would expect so change with care.
	 * Make sure you look at and understand function UnallocatedReturn ().
	 */
	if (RETURN_LINE)
	{
		/*
		 * Process unallocated purchase returns.
		 */
		abc_selfield (sknd, "sknd_sknd_hash");
		for (i = 0; i < allocCnt; i++)
		{
			sknd_rec.sknd_hash = allocRec [i].skndHash;
			cc = find_rec (sknd, &sknd_rec, COMPARISON, "u");
			if (!cc)
			{
				sknd_rec.qty_return		+=	allocRec [i].qtyAlloc;
				cc = abc_update (sknd, &sknd_rec);
				if (cc)
					file_err (cc, sknd, "DBUPDATE");
			}
			else
				abc_unlock (sknd);
		}
		return;
	}

	/*
	 * Check if number plate defined, if not then ignore.
	 */
	strcpy (sknh_rec.co_no, comm_rec.co_no);
	strcpy (sknh_rec.br_no, ccmr_rec.est_no);
	strcpy (sknh_rec.plate_no, pogh_rec.gr_no);
	cc = find_rec (sknh, &sknh_rec, COMPARISON, "r");
	if (cc)
		return;

	memset (&sknd_rec, 0, sizeof (sknd_rec));

	sknd_rec.sknh_hash		=	sknh_rec.sknh_hash;
	sknd_rec.line_no		=	pogl_rec.line_no;	
	updSknd = find_rec (sknd, &sknd_rec, COMPARISON, "u");
	
	sknd_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
	sknd_rec.hhum_hash	=	pogl_rec.hhum_hash;
	sknd_rec.hhcc_hash	=	pogl_rec.hhcc_hash;
	sknd_rec.hhsu_hash	=	pogh_rec.hhsu_hash;
	sknd_rec.hhve_hash	=	pogl_rec.hhve_hash;
	sknd_rec.hhpl_hash	=	pogl_rec.hhpl_hash;
	strcpy (sknd_rec.cus_ord_ref, pogl_rec.cus_ord_ref); 
	strcpy (sknd_rec.serial_no, pogl_rec.serial_no); 
	sknd_rec.qty_rec		=	pogl_rec.qty_rec;
	sknd_rec.land_cst		=	pogl_rec.land_cst;
	strcpy (sknd_rec.status,	"R");	/* Set status as receipted. */
	strcpy (sknd_rec.lstat_chg,	"0");	
	strcpy (sknd_rec.edi,		"0");	

	if (updSknd)
	{
		cc = abc_add (sknd, &sknd_rec);
		if (cc)
			file_err (cc, sknd, "DBADD");
	}
	else
	{
		cc = abc_update (sknd, &sknd_rec);
		if (cc)
			file_err (cc, sknd, "DBUPDATE");
	}

	if (MULT_LOC || SK_BATCH_CONT)
	{
		cc = find_rec (sknd, &sknd_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, sknd, "DBFIND");

		inlo_rec.hhwh_hash	=		incc_rec.hhwh_hash;
		inlo_rec.hhum_hash	=		pogl_rec.hhum_hash;
		strcpy (inlo_rec.location, 	pogl_rec.location);
		strcpy (inlo_rec.lot_no,	pogl_rec.lot_no);
		cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, inlo, "DBFIND");
		
		inlo_rec.sknd_hash	=	sknd_rec.sknd_hash;

		cc = abc_update (inlo, &inlo_rec);
		if (cc)
			file_err (cc, inlo, "DBUPDATE");
	}

}
/*
 * Routine to create audit record of the last cost price on file.
 */
void
ProcessAudit (void)
{
	strcpy (poca_rec.co_no, comm_rec.co_no);
	strcpy (poca_rec.br_no, ccmr_rec.est_no);
	strcpy (poca_rec.type, "V");
	strcpy (poca_rec.item_cat, inmr_rec.category);
	strcpy (poca_rec.item_no, inmr_rec.item_no);
	poca_rec.line_no = pogl_rec.line_no;
	strcpy (poca_rec.item_desc, inmr_rec.description);
	strcpy (poca_rec.po_no, pogl_rec.po_number);
	strcpy (poca_rec.gr_no, pogh_rec.gr_no);
	poca_rec.est_cst = pogl_rec.land_cst;
	poca_rec.act_cst = 0.00;
	strcpy (poca_rec.status, "P");
	cc = abc_add (poca, &poca_rec);
	if (cc)
		file_err (cc, "poca", "DBADD");
}

/*
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.         
 */
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	auditFileOpen = TRUE;

	if (comm_rec.inv_date > lsystemDate)
		fprintf (fout, ".START%s<%s>\n",DateToString(comm_rec.inv_date),PNAME);
	else
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);

	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n",clip(comm_rec.co_name));
	fprintf (fout, ".E%s / %s\n",clip(comm_rec.est_name),comm_rec.cc_name);
	if (dropShipment [0] == 'Y')
		fprintf (fout, ".EDIRECT DELIVERY GOODS RECEIPT CONFIRMATION\n");
	else
		fprintf (fout, ".EPURCHASE ORDER GOODS RECEIPTS\n");
	fprintf (fout, ".Eas at %-24.24s\n",SystemTime ());

	fprintf (fout, ".R================"); 
	fprintf (fout, "================");
	fprintf (fout, "==="); 
	fprintf (fout, "===============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================");
	if (MULT_LOC)
		fprintf (fout, "===========");

	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	if (envPoGrinCosts)
		fprintf (fout, "===========================\n");
	else
		fprintf (fout, "=\n");

	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "===");
	fprintf (fout, "===============");
	fprintf (fout, "=================");
	fprintf (fout, "=========================");
	if (MULT_LOC)
		fprintf (fout, "===========");

	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	if (envPoGrinCosts)
		fprintf (fout, "===========================\n");
	else
		fprintf (fout, "=\n");

	fprintf (fout, "|   PURCHASE    ");
	fprintf (fout, "|     GOODS     ");
	fprintf (fout, "|BR");
	fprintf (fout, "|  WAREHOUSE.  ");
	fprintf (fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|    ITEM DESCRIPTION    ");
	if (MULT_LOC)
		fprintf (fout, "| LOCATION ");

	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|UOM.");
	fprintf (fout, "| QUANTITY  ");
	if (envPoGrinCosts)
		fprintf (fout, "| UNIT  COST |EXTEND COST |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "|ORDER / RETURN ");
	fprintf (fout, "|RECEIPT/ RETURN");
	fprintf (fout, "|NO");
	fprintf (fout, "|    ACRONYM.  ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                        ");
	if (MULT_LOC)
		fprintf (fout, "|          ");

	fprintf (fout, "|          ");
	fprintf (fout, "|    ");
	fprintf (fout, "|           ");
	if (envPoGrinCosts)
		fprintf (fout, "|            |            |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|--");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------------------");
	if (MULT_LOC)
		fprintf (fout, "|----------");

	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	if (envPoGrinCosts)
		fprintf (fout, "|------------|------------|\n");
	else
		fprintf (fout, "|\n");
}

/*
 * Routine to close the audit trail output file.
 */
void
CloseAudit (void)
{
	if (!auditFileOpen)
		return;

	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|--");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------------------");
	if (MULT_LOC)
		fprintf (fout, "|----------");

	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	if (envPoGrinCosts)
		fprintf (fout, "|------------|------------|\n");
	else
		fprintf (fout, "|\n");


	fprintf (fout, "|               ");
	fprintf (fout, "|               ");
	fprintf (fout, "|  ");
	fprintf (fout, "|              ");
	fprintf (fout, "|                ");
	if (envPoGrinCosts)
		fprintf (fout, "| TOTAL VALUE RECEIPTED  ");
	else
		fprintf (fout, "|                        ");
	if (MULT_LOC)
		fprintf (fout, "|          ");

	fprintf (fout, "|          ");
	fprintf (fout, "|    ");
	fprintf (fout, "|           ");
	if (envPoGrinCosts)
		fprintf (fout, "|            |%12.2f|\n",totalReceipted);
	else
		fprintf (fout, "|\n");

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*
 * Print Extra description.
 */
void
PrintInex (void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout, "|%15.15s",	" ");
		fprintf (fout, "|%15.15s",	" ");
		fprintf (fout, "|%2.2s", 	" ");
		fprintf (fout, "|%2.2s   %9.9s",	" ",	" ");
		fprintf (fout, "|%16.16s",	" ");
		fprintf (fout, "|%-24.24s",	inex_rec.desc);
		if (MULT_LOC)
			fprintf (fout, "|          ");
		fprintf (fout, "|%10.10s",	" ");
		fprintf (fout, "|%4.4s",	" ");
		fprintf (fout, "|%11.2s",	" ");
		if (envPoGrinCosts)
		{
			fprintf (fout, "|%12.2s",	" ");
			fprintf (fout, "|%12.2s|\n"," ");
		}
		else
			fprintf (fout, "|\n");
		
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}

/*
 * Get branch closing stock.
 */
float	
GetBrClosing (
	long	hhbrHash)
{
	float	ClosingStock = 0.00;

	strcpy (ccmr2_rec.co_no, ccmr_rec.co_no);
	strcpy (ccmr2_rec.est_no,ccmr_rec.est_no);
	strcpy (ccmr2_rec.cc_no, "  ");
	cc = find_rec (ccmr2, &ccmr2_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr2_rec.co_no, ccmr_rec.co_no) && 
				   !strcmp (ccmr2_rec.est_no, ccmr_rec.est_no))
	{
		incc2_rec.hhcc_hash = ccmr2_rec.hhcc_hash;
		incc2_rec.hhbr_hash = hhbrHash;

		if (!find_rec (incc2 , &incc2_rec, COMPARISON, "r"))
			ClosingStock += incc2_rec.closing_stock;

		cc = find_rec (ccmr2, &ccmr2_rec, NEXT, "r");
	}
	return (ClosingStock);
}

/*
 * Add record to ffdm table 
 */
void
AddFffm (
	float	unsrv,
	long	hhbrHash,
	long	hhccHash)
{
	memset (&ffdm_rec, 0, sizeof (ffdm_rec));

    ffdm_rec.hhbr_hash = hhbrHash;
    ffdm_rec.hhcc_hash = hhccHash;
    ffdm_rec.date = (comm_rec.inv_date > lsystemDate)
		 						? comm_rec.inv_date : lsystemDate;
						
    sprintf (ffdm_rec.type, "%-1.1s", "3");
    cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
    if (!cc)
    {
        ffdm_rec.qty	+=  unsrv;

        cc = abc_update (ffdm, &ffdm_rec);
        if (cc)
            file_err (cc, "ffdm", "DBUPDATE");
    }
    else
    {
        ffdm_rec.qty	=  unsrv;
        cc = abc_add (ffdm, &ffdm_rec);
        if (cc)
            file_err (cc, ffdm, "DBADD");
    }
}

/*
 * Standard special validation.
 */
int
spec_valid (
	int    field)
{
	/*
	 * Start goods receipt number. 
	 */
	if (LCHECK ("sGrn"))
    {
		if (dflt_used)
		{
    		sprintf (local_rec.sGrn , "%15.15s", " ");
    		sprintf (local_rec.sDesc, "%-5.5s", ML ("START"));
			DSP_FLD ("sDesc");
            return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
        {
            SrchPogh (temp_str);
            return (EXIT_SUCCESS);
        }

		abc_selfield (pogh, "pogh_id_no2");
    	strcpy  (pogh_rec.co_no, comm_rec.co_no);
    	sprintf (pogh_rec.gr_no, "%-15.15s", zero_pad (local_rec.sGrn, 15));
    	cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
		abc_selfield (pogh, "pogh_up_id");
		if (cc)
		{
			print_mess (ML (mlStdMess049));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
    	sprintf (local_rec.sDesc, "%-5.5s", " ");
		DSP_FLD ("sGrn");
		DSP_FLD ("sDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * End goods receipt number. 
	 */
	if (LCHECK ("eGrn"))
    {
		if (dflt_used)
		{
    		sprintf (local_rec.eGrn , "%-15.15s", "~~~~~~~~~~~~~~~");
    		sprintf (local_rec.eDesc, "%-5.5s", ML ("END"));
			DSP_FLD ("eDesc");
            return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
        {
            SrchPogh (temp_str);
            return (EXIT_SUCCESS);
        }

		abc_selfield (pogh, "pogh_id_no2");
    	strcpy  (pogh_rec.co_no, comm_rec.co_no);
    	sprintf (pogh_rec.gr_no, "%-15.15s", zero_pad (local_rec.eGrn, 15));
    	cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
		abc_selfield (pogh, "pogh_up_id");
		if (cc)
		{
			print_mess (ML (mlStdMess049));
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

    	sprintf (local_rec.eDesc, "%-5.5s", " ");
		DSP_FLD ("eGrn");
		DSP_FLD ("eDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Start Date number. 
	 */
	if (LCHECK ("sDate"))
    {
		if (dflt_used)
		{
    		local_rec.sDate = 0L;
            return (EXIT_SUCCESS);
		}
		DSP_FLD ("sDate");
        return (EXIT_SUCCESS);
	}

	/*
	 * End Date number. 
	 */
	if (LCHECK ("eDate"))
    {
		if (dflt_used)
		{
    		local_rec.eDate = TodaysDate();
            return (EXIT_SUCCESS);
		}
		DSP_FLD ("eDate");
        return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SetupVars ()
{
   	sprintf (local_rec.sGrn , "%15.15s", " ");
   	sprintf (local_rec.eGrn , "%15.15s", "~~~~~~~~~~~~~~~");
    local_rec.sDate = 0L;
   	local_rec.eDate = TodaysDate();
}

void
SrchPogh (
	char	*key_val)
{
	char	srchDate [11];

    work_open ();
    save_rec ("#GRN Number    ", "#Date Raised");

	abc_selfield (pogh, "pogh_id_no2");
    strcpy  (pogh_rec.co_no, comm_rec.co_no);
    sprintf (pogh_rec.gr_no, "%-15.15s", key_val);
    for (cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
         !cc &&
          !strcmp (pogh_rec.co_no, comm_rec.co_no);
         cc = find_rec (pogh, &pogh_rec, NEXT, "r"))
    {
        if (strcmp (pogh_rec.br_no, comm_rec.est_no))
			continue;

        if (!strncmp (pogh_rec.gr_no, key_val, strlen (key_val)))
        {
			strcpy (srchDate, DateToString (pogh_rec.date_raised));
            cc = save_rec (pogh_rec.gr_no, srchDate);
            if (cc)
                break;
        }
	}
	cc = disp_srch ();
    work_close ();
    if (cc)
    {
		abc_selfield (pogh, "pogh_up_id");
        memset (&pogh_rec, 0, sizeof pogh_rec);
        return;
    }
    strcpy  (pogh_rec.co_no, comm_rec.co_no);
    sprintf (pogh_rec.gr_no, "%-15.15s", temp_str);
    cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
	if (cc)
        file_err (cc, pogh, "DBFIND");

	abc_selfield (pogh, "pogh_up_id");
}

int
heading (
	int    scn)
{
    if (restart)
        return (EXIT_SUCCESS);

    if (scn != cur_screen)
        scn_set (scn);

    clear ();
    snorm ();
    rv_pr (ML ("Reprint Goods Receipt Numbers "), 24, 0, 1);

    line_at (1,0,79);
    line_at (21,0,79);
	print_at (22, 0, mlStdMess038, comm_rec.co_no, comm_rec.co_name);
    box (0, 1, 79, 4);

	scn_set     (scn);
    scn_write   (scn);
    scn_display (scn);
	return (EXIT_SUCCESS);
}

/*
 * Switch fields where warranted
 */
struct grnStruct
ValidateFields (void)
{
	struct grnStruct grn;

	if (strcmp (local_rec.sGrn, local_rec.eGrn) <= 0)
	{
		sprintf (grn.sGrn, "%-15.15s", local_rec.sGrn);
		sprintf (grn.eGrn, "%-15.15s", local_rec.eGrn);
	}
	else
	{
		sprintf (grn.sGrn, "%-15.15s", local_rec.eGrn);
		sprintf (grn.eGrn, "%-15.15s", local_rec.sGrn);
	}

	if (local_rec.sDate <= local_rec.eDate)
	{
		grn.sDate = local_rec.sDate;
		grn.eDate = local_rec.eDate;
	}
	else
	{
		grn.sDate = local_rec.eDate;
		grn.eDate = local_rec.sDate;
	}

	return (grn);
}

/*
 * Process unallocated returns.
 */
void
UnallocatedReturn (
	long	hhglHash,
	float	returnQuantity)
{
	/*
	* Some explanations of this code is required so read on.
	*/
	int		allocationFound	=	FALSE;
	float	allocationQty	=	0.00;

	/*
	 * Check if allocation is correct so add current allocation records.
	 */
	inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
	inla_rec.inlo_hash	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "r");
	while (!cc && inla_rec.hhgl_hash == pogl_rec.hhgl_hash)
	{
		allocationFound	=	TRUE;
		allocationQty	+=	inla_rec.qty_alloc;

		cc = find_rec (inla, &inla_rec, NEXT, "r");
	}
	/*
	 * If this bit is executed when the allocation WAS found 
	 * but the total on the goods receipt line did not equal 
	 * the allocation. In this case we have to make an adjustment.
	 */
	if (allocationFound == TRUE && allocationQty != returnQuantity)
	{
		float	diffQty	=	0.00;

		diffQty = returnQuantity - allocationQty;

		inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
		inla_rec.inlo_hash	=	0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "u");
		if (!cc && inla_rec.hhgl_hash == pogl_rec.hhgl_hash)
		{
			inla_rec.qty_alloc += diffQty;

			cc = abc_update (inla, &inla_rec);
			if (cc)
				file_err (cc, inla, "DBUPDATE");
		}
		else
			abc_unlock (inla);
	}
	/*
	 * This next bit of code checks if the allocation exists.
	 * The allocation must now be created if not.
	 */
	inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
	inla_rec.inlo_hash	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "r");
	if (cc || inla_rec.hhgl_hash != pogl_rec.hhgl_hash)
	{
		abc_selfield (inlo, "inlo_mst_loc");

		inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inlo_rec.hhum_hash	=	pogl_rec.hhum_hash;
		strcpy (inlo_rec.location, "          ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		if (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash &&
				   inlo_rec.hhum_hash == pogl_rec.hhum_hash)
		{
			inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
			inla_rec.inlo_hash	=	inlo_rec.inlo_hash;
			inla_rec.pid		=	0;
			inla_rec.line_no	=	0;
			inla_rec.qty_alloc	=	returnQuantity;
			cc = abc_add (inla, &inla_rec);
			if (cc)
				file_err (cc, inla, "DBADD");
		}
		else
		{
			inlo_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inlo_rec.hhum_hash	=	0L;
			strcpy (inlo_rec.location, "          ");
			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
			if (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
			{
				inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
				inla_rec.inlo_hash	=	inlo_rec.inlo_hash;
				inla_rec.pid		=	0;
				inla_rec.line_no	=	0;
				inla_rec.qty_alloc	=	returnQuantity;
				cc = abc_add (inla, &inla_rec);
				if (cc)
					file_err (cc, inla, "DBADD");
			}
		}
	}
	abc_selfield (inlo, "inlo_inlo_hash");

	/*
	 * Now that we know inla records exist we need to add them to the 
	 * array that will be used later.
	 */
	inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
	inla_rec.inlo_hash	=	0L;
	cc = find_rec (inla, &inla_rec, GTEQ, "u");
	while (!cc && inla_rec.hhgl_hash == pogl_rec.hhgl_hash)
	{
		inlo_rec.inlo_hash	=	inla_rec.inlo_hash;
		cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
		if (!cc)
		{
			if (!ArrChkLimit (&allocationDetail, allocRec, allocCnt))
				sys_err ("ArrChkLimit (allocRec)", ENOMEM, PNAME);

			strcpy (allocRec [allocCnt].location, inlo_rec.location);
			strcpy (allocRec [allocCnt].lotNo, 	  inlo_rec.lot_no);
			strcpy (allocRec [allocCnt].slotNo,	  inlo_rec.slot_no);
			strcpy (allocRec [allocCnt].uom,	  inlo_rec.uom);
			strcpy (allocRec [allocCnt].locType,  inlo_rec.loc_type);
			strcpy (allocRec [allocCnt].locStatus,inlo_rec.loc_status);
			allocRec [allocCnt].hhwhHash	=	inlo_rec.hhwh_hash;
			allocRec [allocCnt].hhumHash	=	inlo_rec.hhum_hash;
			allocRec [allocCnt].inloHash	=	inlo_rec.inlo_hash;
			allocRec [allocCnt].skndHash	=	inlo_rec.sknd_hash;
			allocRec [allocCnt].hhglHash	=	inla_rec.hhgl_hash;
			allocRec [allocCnt].cnvFct		=	inlo_rec.cnv_fct;
			allocRec [allocCnt].qtyAlloc	=	inla_rec.qty_alloc;
			allocCnt++;
		}
		cc = abc_delete (inla);
		if (cc)
			file_err (cc, inla, "DBDELETE");

		inla_rec.hhgl_hash	=	pogl_rec.hhgl_hash;
		inla_rec.inlo_hash	=	0L;
		cc = find_rec (inla, &inla_rec, GTEQ, "u");
	}
}
