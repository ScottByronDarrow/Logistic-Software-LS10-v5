/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_cus_ser.c,v 5.4 2002/03/05 05:47:14 scott Exp $
|  Program Name  : (so_cus_ser.c) 
|  Program Desc  : (Customer Service Enquiry)
|---------------------------------------------------------------------|
|  Date Written  : (28/04/89)      | Author       : Fui Choo Yap.     |
|---------------------------------------------------------------------|
| $Log: so_cus_ser.c,v $
| Revision 5.4  2002/03/05 05:47:14  scott
| ..
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_cus_ser.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cus_ser/so_cus_ser.c,v 5.4 2002/03/05 05:47:14 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <arralloc.h>

#define	X_OFF	0
#define	Y_OFF	2

#define		CREDIT		(cohr_rec.type [0] == 'C')
#define		INVOICE		(cohr_rec.type [0] == 'I')
#define		P_SLIP		(cohr_rec.type [0] == 'P')
#define		A_CREDIT	(arhr_rec.type [0] == 'C')
#define		A_INVOICE	(arhr_rec.type [0] == 'I')
#define		FORWARD		(soln_rec.status [0] == 'F')
#define		PACK_SLIP	(soln_rec.status [0] == 'P' || \
				  soln_rec.status [0] == 'S')
#define		DIS_SCN		2
#define		ALL_BACKORDER	(coln_rec.q_order == 0.00 && \
				  			  coln_rec.q_backorder != 0.00)

	int		envDbCo 		= 0, 
			envDbFind 		= 0, 
			includeArchive 	= FALSE,
			displayCredits 	= FALSE;

	char	branchNo 	[3],
			dataStr 	[300],
			lastReference [9],
			mlCusSer [10][101];

	char	*data = "data",
			*twentySpace	=	"                    ";

	long	poDueDate = -1L;

#include	"schema"

struct commRecord	comm_rec;
struct inumRecord	inum_rec;
struct cumrRecord	cumr_rec;
struct cuitRecord	cuit_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cohrRecord	cohr_rec;
struct arhrRecord	arhr_rec;
struct colnRecord	coln_rec;
struct arlnRecord	arln_rec;
struct polnRecord	poln_rec;
struct cfhsRecord	cfhs_rec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[51];
	char	type		[2];
	char	cusOrdRef	[21];
	char	invNo		[9];
	char	appInvNo	[9];
	char	itemDesc	[41];
	char	itemUom		[5];
	char	xDescCode	[4];
	char	status		[2];
	float	orderQty;
	float	borderQty;
	long	ordDate;
	long	disDate;
	long	dueDate;
	long	hhcuHash;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;


/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy 			[11],
			value 			[17],
			name 			[41],
			item_desc 		[41],
			previousValue 	[17],
			startCusRef 	[21],
			endCusRef 		[21],
			itemNo 			[17],
			option 			[2],
			optionDesc		[31],
			archiveFlag		[2],
			archiveFlagDesc	[31];
	long	hhbrHash;
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.previousValue, "Customer Number       ", " ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.value}, 
	{1, LIN, "name", 	 4, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.name}, 
	{1, LIN, "startCusRef", 	 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Customer Ref From     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startCusRef}, 
	{1, LIN, "endCusRef", 	 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Customer Ref To       ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endCusRef}, 
	{1, LIN, "itemNo", 	 7, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Item Number           ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.itemNo}, 
	{1, LIN, "desc", 	 7, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "",  "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{1, LIN, "option", 	 9, 2, CHARTYPE, 
		"U", "        ", 
		" ", "O", "Display Option        ", "Enter O(rders, Invoices) OR C(redits", 
		YES, NO, JUSTRIGHT, "OC", "", local_rec.option}, 
	{1, LIN, "optionDesc", 	9, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "        ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.optionDesc}, 
	{1, LIN, "archiveFlag",  10, 2, CHARTYPE, 
		"U", "        ", 
		" ", "Y", "Include Archive Data  ", "Enter Yes to include, No to exclude", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.archiveFlag}, 
	{1, LIN, "archiveFlagDesc",	10, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "        ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.archiveFlagDesc}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<get_lpno.h>
#include 	<RealCommit.h>
#include	<FindCumr.h>
/*
 * Function Declarations 
 */
float 	AvailableStock 		(long, long);
int  	heading 			(int);
int  	ProcessArhr 		(long);
int  	ProcessArln 		(long);
int  	ProcessCohr 		(long);
int  	ProcessColn 		(long);
int  	ProcessSohr 		(long);
int  	ProcessSoln 		(long);
int  	SortOrders 			(void);
int  	spec_valid 			(int);
int  	ValidateDesc 		(char *);
int  	ValidateItem 		(long);
int		SortFunc			(const	void *,	const void *);
long 	PurchaseOrderDate 	(long, long, float);
void 	CloseDB 			(void);
void 	DisplayData 		(void);
void 	GetCarrier 			(long, char *);
void 	InitML 				(void);
void 	OpenDB 				(void);
void 	PrintCompanyName 	(void);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc, 
 char *	argv [])
{
	int	Dsp_ok = 0;

	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	InitML ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	clear ();

	strcpy (local_rec.previousValue, "000000");

	while (prog_exit == 0)
	{
		abc_selfield (inmr, "inmr_id_no");

		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		init_vars (1);	

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Dsp_ok = SortOrders ();
		if (Dsp_ok)
			DisplayData ();
		else
		{
			if (local_rec.option [0] == 'O')
				sprintf (err_str, ML (mlSoMess296), local_rec.value);
			else
				sprintf (err_str, ML (mlSoMess298), local_rec.value);

			print_mess (err_str);
			sleep (sleepTime);
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Multi-lingual initialisation. 
 */
void
InitML (void)
{
	strcpy (mlCusSer [1], ML ("Carr"));
	strcpy (mlCusSer [2], ML ("Cons #"));
	strcpy (mlCusSer [3], ML ("No Cartons"));
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
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
						    	  : "cumr_id_no3");

	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhcu_hash");
	open_rec (arhr, arhr_list, ARHR_NO_FIELDS, "arhr_hhcu_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhcu_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (arln, arln_list, ARLN_NO_FIELDS, "arln_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (cfhs, cfhs_list, CFHS_NO_FIELDS, "cfhs_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuit);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (cohr);
	abc_fclose (arhr);
	abc_fclose (coln);
	abc_fclose (arln);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (poln);
	abc_fclose (cfhs);
	abc_fclose (soic);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Customer Number.
	 */
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.value));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.name, cumr_rec.dbt_name);
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("startCusRef"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCusRef, "                    ");
			DSP_FLD ("startCusRef");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endCusRef"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCusRef, "~~~~~~~~~~~~~~~~~~~~");
			DSP_FLD ("endCusRef");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("itemNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.itemNo, "ALL             ");
			local_rec.hhbrHash = 0L;
			sprintf (local_rec.item_desc, "%40s", " ");
			DSP_FLD ("itemNo");
			DSP_FLD ("desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch 
			(
				comm_rec.co_no, 
				temp_str, 
				cumr_rec.hhcu_hash, 
				cumr_rec.item_codes
			);
			return (EXIT_SUCCESS);
		}
		cc	=	FindInmr 
				(
					comm_rec.co_no, 
					local_rec.itemNo, 
					cumr_rec.hhcu_hash, 
					cumr_rec.item_codes
				);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.hhbrHash = inmr_rec.hhbr_hash;

		strcpy (local_rec.itemNo, inmr_rec.item_no);
		strcpy (local_rec.item_desc, inmr_rec.description);
		DSP_FLD ("itemNo");
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("option"))
	{
		if (local_rec.option [0] == 'C')
		{
			displayCredits = TRUE;
			strcpy (local_rec.optionDesc, ML ("(Credits)"));
		}
		else
		{
			displayCredits = FALSE;
			strcpy (local_rec.optionDesc, ML ("(Orders, Invoices)"));
		}
		DSP_FLD ("option");
		DSP_FLD ("optionDesc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("archiveFlag"))
	{
		if (local_rec.archiveFlag [0] == 'Y')
		{
			includeArchive = TRUE;
			strcpy (local_rec.archiveFlagDesc, ML ("(Archive Data Included)"));
		}
		else
		{
			includeArchive = FALSE;
			strcpy (local_rec.archiveFlagDesc, ML ("(Archive Data Excluded)"));
		}
		DSP_FLD ("archiveFlag");
		DSP_FLD ("archiveFlagDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}
int
SortOrders (void)
{
	int	found = FALSE;

	abc_selfield (inmr, "inmr_hhbr_hash");

	clear ();
	print_at (0, 0, ML (mlSoMess159));
	fflush (stdout);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	/*
	 * If Display Orders, Invoices, read soln file.
	 */
	if (!displayCredits)
		found += ProcessSohr (cumr_rec.hhcu_hash);

	print_at (1, 0, ML (mlSoMess160));

	/*
	 * Save Credits or Invoices from coln file	
	 */
	found += ProcessCohr (cumr_rec.hhcu_hash);

	/*
	 * Save Credits or Invoices from arhr file	
	 */
	if (includeArchive)
	{
		print_at (2, 0, ML (mlSoMess161));
		fflush (stdout);

		found += ProcessArhr (cumr_rec.hhcu_hash);
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	return (found);
}

void
DisplayData (void)
{
	int		i;
	char	ordDateString [11],
			dueDateString [11],
			disDateString [11];

	clear ();
	
	strcpy (err_str, "C U S T O M E R   S E R V I C E S   E N Q U I R Y"); 

	print_at (1, 0, ML (mlStdMess012), cumr_rec.dbt_no, cumr_rec.dbt_name);
	print_at (2, 0, ML (mlSoMess162));
	PrintCompanyName ();

	Dsp_prn_open (0, 3, 13, err_str, comm_rec.co_no, comm_rec.co_name, 
				      comm_rec.est_no, comm_rec.est_name, 
				     (char *) 0, (char *) 0);
				      
	Dsp_saverec (" Customer           |T| Doco   |Product Description |UOM.|  Qty   |  Qty   |   ETA    |Extra| Required |Stat|Related | Despatch ");
	Dsp_saverec (" Order Reference    | | No.    |                    |    |  Order |  B/O   |   Date   |Desc.|   Date   |    |Inv/Crd |   Date   ");

	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (ordDateString, "          ");
		sprintf (dueDateString, "          ");
		sprintf (disDateString, "          ");

		if (sortRec [i].ordDate > 0L)
			sprintf (ordDateString, DateToString (sortRec [i].ordDate));

		if (sortRec [i].dueDate > 0L)
			sprintf (dueDateString, DateToString (sortRec [i].dueDate));

		if (sortRec [i].disDate > 0L)
			sprintf (disDateString, DateToString (sortRec [i].disDate));

		if (sortRec [i].hhcuHash > 0L)
			GetCarrier (sortRec [i].hhcuHash, sortRec [i].invNo);

		sprintf 
		(
			dataStr, 
			"%-20.20s^E%-1.1s^E%-8.8s^E%-20.20s^E%-4.4s^E%8.2f^E%8.2f^E%-10.10s^E %-3.3s ^E%-10.10s^E %-1.1s  ^E%-8.8s^E%-10.10s", 
			sortRec [i].cusOrdRef,
			sortRec [i].type,
			sortRec [i].invNo,
			sortRec [i].itemDesc,
			sortRec [i].itemUom,
			sortRec [i].orderQty,
			sortRec [i].borderQty,
			dueDateString,
			sortRec [i].xDescCode,
			ordDateString,
			sortRec [i].status,
			sortRec [i].appInvNo,
			disDateString
		);
		Dsp_saverec (dataStr);
	}
	Dsp_srch ();
	Dsp_close ();
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
GetCarrier (
	long	hhcuHash, 
	char 	*invRef)
{
	if (!strcmp (invRef, lastReference))
		return;

	strcpy (lastReference, invRef);

	cfhs_rec.hhcu_hash = hhcuHash;
	strcpy (cfhs_rec.ref_no, invRef);
	cfhs_rec.date = 0L;

	cc = find_rec (cfhs, &cfhs_rec, GTEQ, "r");
	while (!cc && cfhs_rec.hhcu_hash == hhcuHash &&
		   !strcmp (cfhs_rec.ref_no, invRef))
	{
		sprintf 
		(
			dataStr, 
			 "                    ^E ^E^1%8.8s^E%-4.4s : %4.4s / %-6.6s : %-16.16s  %-10.10s %4d ^6   ^E          ^E    ^E        ^E^1%10.10s^6", 
			invRef, 
			mlCusSer [1], 
			cfhs_rec.carr_code, 
			mlCusSer [2], 
			cfhs_rec.cons_no, 
			mlCusSer [3], 
			cfhs_rec.no_cartons, 
			DateToString (cfhs_rec.date)
		);
 
		Dsp_saverec (dataStr);

		cc = find_rec (cfhs, &cfhs_rec, NEXT, "r");
	}
	return;
}

/*
 * Process Invoices for Given Customer Hash
 */
int
ProcessCohr (
	long	hhcuHash)
{
	int	dataFound = FALSE;

	print_mess (ML ("Processing Invoices"));

	cohr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && cohr_rec.hhcu_hash == hhcuHash)
	{
		dataFound += ProcessColn (cohr_rec.hhco_hash);
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	return (dataFound);
}

/*
 * Process archive invoices for Given Customer Hash  
 */
int
ProcessArhr (
	long	hhcuHash)
{
	int	dataFound = FALSE;

	print_mess (ML ("Processing Archive Invoices"));

	arhr_rec.hhcu_hash = hhcuHash;
	cc = find_rec (arhr, &arhr_rec, GTEQ, "r");
	while (!cc && arhr_rec.hhcu_hash == hhcuHash)
	{
		dataFound += ProcessArln (arhr_rec.hhco_hash);
		cc = find_rec (arhr, &arhr_rec, NEXT, "r");
	}
	return (dataFound);
}

/*
 * Process Orders for Given Customer Hash  
 */
int
ProcessSohr (
	long	hhcuHash)
{
	int	dataFound = FALSE;

	print_mess (ML ("Processing Archive Invoices"));
	sohr_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && sohr_rec.hhcu_hash == hhcuHash)
	{
		dataFound += ProcessSoln (sohr_rec.hhso_hash);
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	return (dataFound);
}

/*
 * Process Invoice Lines for given order hash (hhso). 
 */
int
ProcessColn (
	long	hhcoHash)
{
	int		carrFnd;
	int		found 		= FALSE;
	float	ConvFct 	= 0.00;
	float	StdCnvFct 	= 1;
	float	orderQty 		= 0.00;
	float	backOrderQty 	= 0.00;
	char	cusOrdRef	[21];

	if (!strcmp (cohr_rec.carr_code, "    "))
		carrFnd = FALSE;
	else
		carrFnd = TRUE;

	coln_rec.hhco_hash 	= hhcoHash;
	coln_rec.line_no 	= 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		if ((displayCredits && CREDIT) || 
			(!displayCredits && (INVOICE || P_SLIP)))
			;
		else
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
		if (!strcmp (coln_rec.cus_ord_ref, twentySpace))
			strcpy (cusOrdRef, cohr_rec.cus_ord_ref);
		else
			strcpy (cusOrdRef, coln_rec.cus_ord_ref);

		if (strcmp (cusOrdRef,  local_rec.startCusRef) < 0 || 
		     strcmp (cusOrdRef, local_rec.endCusRef) > 0)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
		if (ALL_BACKORDER)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}

		if (!ValidateItem (coln_rec.hhbr_hash))
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			sprintf (inmr_rec.description, coln_rec.item_desc);
		else
		{
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			StdCnvFct = inum_rec.cnv_fct;
		}

		inum_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			inum_rec.cnv_fct = 1.00;
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
		}
		ConvFct	 =	inum_rec.cnv_fct	/	StdCnvFct;
		orderQty	 =	coln_rec.q_order /	ConvFct;
		backOrderQty	 =	coln_rec.q_backorder 	/	ConvFct;

		if (!ValidateDesc (inmr_rec.ex_code))
			strcpy (inmr_rec.ex_code, "   ");
			
		found = TRUE;

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%1.1s%20.20s%8.8s%-16.16s",
			cohr_rec.type,
			cusOrdRef,
			cohr_rec.inv_no,
			inmr_rec.item_no
		);
		strcpy (sortRec [sortCnt].type,			cohr_rec.type);
		strcpy (sortRec [sortCnt].cusOrdRef,	cusOrdRef);
		strcpy (sortRec [sortCnt].invNo, 		cohr_rec.inv_no);
		strcpy (sortRec [sortCnt].appInvNo,		cohr_rec.app_inv_no);
		strcpy (sortRec [sortCnt].itemDesc,		inmr_rec.description);
		strcpy (sortRec [sortCnt].itemUom, 		inum_rec.uom);
		strcpy (sortRec [sortCnt].xDescCode, 	inmr_rec.ex_code);
		strcpy (sortRec [sortCnt].status, 		(P_SLIP) ? cohr_rec.type : " ");
		sortRec [sortCnt].orderQty	=	orderQty;
		sortRec [sortCnt].borderQty	=	(P_SLIP) ? backOrderQty : 0.00;
		sortRec [sortCnt].ordDate	=	cohr_rec.date_required;
		sortRec [sortCnt].disDate	=	cohr_rec.date_raised;
		sortRec [sortCnt].dueDate	=	0L;
		sortRec [sortCnt].hhcuHash	= 	(carrFnd) ? cohr_rec.hhcu_hash : 0L;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	return (found);
}
/*
 * Process Invoice Lines for given order hash (hhso). 
 */
int
ProcessArln (
	long	hhcoHash)
{
	float	ConvFct	 		=	0.00, 
			StdCnvFct		=	1, 
			orderQty	 	=	0.00, 
			backOrderQty	=	0.00;
	char	cusOrdRef	[21];

	int	found = FALSE;
	int	carrFnd;

	if (!strcmp (arhr_rec.carr_code, "    "))
		carrFnd = FALSE;
	else
		carrFnd = TRUE;

	arln_rec.hhco_hash = hhcoHash;
	arln_rec.line_no = 0;
	cc = find_rec (arln, &arln_rec, GTEQ, "r");
	while (!cc && arln_rec.hhco_hash == hhcoHash)
	{
		if (!strcmp (coln_rec.cus_ord_ref, twentySpace))
			strcpy (cusOrdRef, arhr_rec.cus_ord_ref);
		else
			strcpy (cusOrdRef, arln_rec.cus_ord_ref);

		if (strcmp (cusOrdRef, local_rec.startCusRef) < 0 || 
		     strcmp (cusOrdRef, local_rec.endCusRef) > 0)
		{
			cc = find_rec (arln, &arln_rec, NEXT, "r");
			continue;
		}
		if (!ValidateItem (arln_rec.hhbr_hash))
		{
			cc = find_rec (arln, &arln_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash	=	arln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			sprintf (inmr_rec.description, arln_rec.item_desc);
		else
		{
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			StdCnvFct = inum_rec.cnv_fct;
		}

		inum_rec.hhum_hash	=	arln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			inum_rec.cnv_fct = 1.00;
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
		}
		ConvFct	 =	inum_rec.cnv_fct	/	StdCnvFct;
		orderQty	 =	arln_rec.q_order /	ConvFct;
		backOrderQty	 =	arln_rec.q_backorder 	/	ConvFct;

		if (!ValidateDesc (inmr_rec.ex_code))
			strcpy (inmr_rec.ex_code, "   ");
			
		found = TRUE;

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%1.1s%20.20s%8.8s%-16.16s",
			"A",
			cusOrdRef,
			arhr_rec.inv_no,
			inmr_rec.item_no
		);
		strcpy (sortRec [sortCnt].type,			"A");
		strcpy (sortRec [sortCnt].cusOrdRef,	cusOrdRef);
		strcpy (sortRec [sortCnt].invNo, 		arhr_rec.inv_no);
		strcpy (sortRec [sortCnt].appInvNo,		arhr_rec.app_inv_no);
		strcpy (sortRec [sortCnt].itemDesc,		inmr_rec.description);
		strcpy (sortRec [sortCnt].itemUom, 		inum_rec.uom);
		strcpy (sortRec [sortCnt].xDescCode, 	inmr_rec.ex_code);
		strcpy (sortRec [sortCnt].status, 		" ");
		sortRec [sortCnt].orderQty	=	orderQty;
		sortRec [sortCnt].borderQty	=	0.00;
		sortRec [sortCnt].ordDate	=	arhr_rec.date_required;
		sortRec [sortCnt].disDate	=	arhr_rec.date_raised;
		sortRec [sortCnt].dueDate	=	0L;
		sortRec [sortCnt].hhcuHash	= 	(carrFnd) ? arhr_rec.hhcu_hash : 0L;
		/*
		 * Increment array counter.
		 */
		sortCnt++;
		
		cc = find_rec (arln, &arln_rec, NEXT, "r");
	}
	return (found);
}

/*
 * Process Orders Lines for given order hash (hhso). 
 */
int
ProcessSoln (
	long	hhsoHash)
{
	int		found 			= FALSE;
	float	avail 			= 0.00,
			ConvFct 		= 0.00,
			StdCnvFct 		= 1,
			orderQty 		= 0.00,
			backOrderQty	= 0.00;
	char	cusOrdRef	[21];

	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (PACK_SLIP)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		if ((soln_rec.qty_order + soln_rec.qty_bord) <= 0.00)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		if (!strcmp (soln_rec.cus_ord_ref, twentySpace))
			strcpy (cusOrdRef, sohr_rec.cus_ord_ref);
		else
			strcpy (cusOrdRef, soln_rec.cus_ord_ref);

		if (strcmp (cusOrdRef, local_rec.startCusRef) < 0 || 
		     strcmp (cusOrdRef, local_rec.endCusRef) > 0)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		if (!ValidateItem (soln_rec.hhbr_hash))
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (inmr_rec.description, "%24s", " ");
			StdCnvFct = 1.00;
		}
		else
		{
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			StdCnvFct = inum_rec.cnv_fct;
		}

		inum_rec.hhum_hash	=	soln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
			inum_rec.cnv_fct 	= 1.00;
		}
		ConvFct = inum_rec.cnv_fct / StdCnvFct;
		orderQty  = soln_rec.qty_order / ConvFct;
		backOrderQty = soln_rec.qty_bord / ConvFct;

		if (!ValidateDesc (inmr_rec.ex_code))
			strcpy (inmr_rec.ex_code, "   ");
			
		avail = AvailableStock (alt_hash (inmr_rec.hhbr_hash, 
									   inmr_rec.hhsi_hash), 
				     				   soln_rec.hhcc_hash);

		PurchaseOrderDate (alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash), 
			           	   soln_rec.hhcc_hash, avail);

		found = TRUE;

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%1.1s%20.20s%8.8s%-16.16s",
			local_rec.option, 
			cusOrdRef,
			sohr_rec.order_no,
			inmr_rec.item_no
		);
		strcpy (sortRec [sortCnt].type,			local_rec.option);
		strcpy (sortRec [sortCnt].cusOrdRef,	cusOrdRef);
		strcpy (sortRec [sortCnt].invNo, 		sohr_rec.order_no);
		strcpy (sortRec [sortCnt].appInvNo,		" ");
		strcpy (sortRec [sortCnt].itemDesc,		inmr_rec.description);
		strcpy (sortRec [sortCnt].itemUom, 		inum_rec.uom);
		strcpy (sortRec [sortCnt].xDescCode, 	inmr_rec.ex_code);
		strcpy (sortRec [sortCnt].status, 		soln_rec.status);
		sortRec [sortCnt].orderQty	=	orderQty;
		sortRec [sortCnt].borderQty	=	backOrderQty;
		sortRec [sortCnt].dueDate	=	soln_rec.due_date;
		sortRec [sortCnt].ordDate	=	poDueDate;
		sortRec [sortCnt].disDate	=	0L;
		sortRec [sortCnt].hhcuHash	= 	0L;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (found);
}

int
ValidateItem (
	long	hhbrHash)
{
	if (local_rec.hhbrHash == 0L || hhbrHash == local_rec.hhbrHash)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int
ValidateDesc (
	char	*itemDesc)
{
	if (!strncmp (itemDesc, "NYP", 3) || !strncmp (itemDesc, "R/P", 3) ||
	     !strncmp (itemDesc, "NOP", 3) || !strncmp (itemDesc, "O/P", 3))
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void
PrintCompanyName (void)
{
	line_at (21,0,132);
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (23, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
}


/*
 * Calculate the due date from the purchase orders
 */
long	
PurchaseOrderDate (
	long	hhbrHash, 
	long	hhccHash, 
	float	qtyAvail)
{
	float	qtyLeft = 0.00;
	float	qtyOrder = 0.00;
	
	qtyLeft = qtyAvail;
	qtyLeft *= -1.00;

	poln_rec.hhbr_hash = hhbrHash;
	poln_rec.due_date = 0L;

	cc = find_rec ("poln", &poln_rec, GTEQ, "r");
	/*
	 * Find poln records for the warehouse item
	 */
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec ("poln", &poln_rec, NEXT, "r");
			continue;
		}
		if (poln_rec.hhcc_hash != hhccHash)
		{
			cc = find_rec ("poln", &poln_rec, NEXT, "r");
			continue;
		}
		qtyOrder =  poln_rec.qty_ord - poln_rec.qty_rec;
		qtyLeft -= qtyOrder;

		/*
		 * The P/O which put the qty into -ve is the P/O to use.		
		 */
		if (qtyLeft < 0.00)
		{
			qtyLeft *= -1;
			qtyOrder = (qtyLeft > qtyOrder) ? qtyOrder : qtyLeft;
			poDueDate = poln_rec.due_date;
			break;
		}

		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Process available stock for item.
 */
float	
AvailableStock (
	long	hhbrHash, 
	long	hhccHash)
{
	float	avail = 0.00;
	float	realCommitted;

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	
	if (find_rec (incc, &incc_rec, COMPARISON, "r"))
		return (0.00);

	/*
	 * Calculate Actual Qty Committed. 
	 */
	realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
										incc_rec.hhcc_hash);
	avail = incc_rec.closing_stock - 
			incc_rec.committed -
			realCommitted;

	return (avail);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	swide ();
	clear ();

	rv_pr (ML (mlSoMess158), 53, 0, 1);

	box (0, 3, 132, 7);
	line_at (1,0,132);
	line_at (8,1,131);

	line_cnt = 0;
	scn_write (scn);

	PrintCompanyName ();
    return (EXIT_SUCCESS);
}

