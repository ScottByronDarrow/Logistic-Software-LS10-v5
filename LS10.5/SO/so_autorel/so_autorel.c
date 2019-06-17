/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_autorel.c,v 5.3 2002/04/30 07:56:46 scott Exp $
|  Program Name  : (so_autorel.c)
|  Program Desc  : (Automatic Release of Backorders)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 26/03/91         |
|---------------------------------------------------------------------|
| $Log: so_autorel.c,v $
| Revision 5.3  2002/04/30 07:56:46  scott
| Update for new Archive modifications;
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_autorel.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_autorel/so_autorel.c,v 5.3 2002/04/30 07:56:46 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>

#define	AUTO		(type [0] == 'A')

#define	InternalMIN(X, Y)	(( (X) < (Y)) ? (X) : (Y))
#define	COMMENT_LINE	(incc_rec.sort [0] == 'Z' && envSoCommHold)

#define	NON_STOCK	(incc_rec.sort [0] == 'N')

#define	SOLN_BACKORDER	(soln_rec.status [0] == 'B')
#define	ITLN_BACKORDER	(itln_rec.status [0] == 'B' || \
			  		    (itln_rec.qty_border != 0.00 && \
						  itln_rec.status [0] == 'T'))

#define	PHANTOM		(inmr_rec.inmr_class [0] == 'P')

struct	RCV_CC_LIST
{
	long	hhccHash;
	long	hhitHash;
	int		line_no;
	struct	RCV_CC_LIST	*_next;
};

#define	RCV_CC_NULL	((struct RCV_CC_LIST *) NULL)

struct	RCV_CC_LIST	*rcv_cc_head = RCV_CC_NULL;

/*
 * NB: To optimise memory requirements, (ie: Alignment)	
 * the sort structure is not necessarily in the order of
 * primary, secondary, tertiary etc. keys.		
 */
struct	SORT_LIST
{
	char	sort_stat;		/* (1) Primary key	*/
	char	sort_type;		/* (3) Tertiary key	*/
	char	sort_bo_flag [2];
	char	sort_class [2];
	long	sort_date;		/* (2) Secondary key	*/
	long	sort_hash;		/* (4) Final key	*/
	struct	SORT_LIST *_prev;
	struct	SORT_LIST *_next;
};

#define	SORT_NULL	((struct SORT_LIST *) NULL)

struct	SORT_LIST	*free_head = SORT_NULL;
struct	SORT_LIST	*sort_head = SORT_NULL;
struct	SORT_LIST	*sort_tail = SORT_NULL;

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct inwsRecord	inws_rec;
struct soktRecord	sokt_rec;

	char	*soln2 = "soln2", 
	    	*ithr2 = "ithr2", 
	    	*itln2 = "itln2", 
	    	*itln3 = "itln3";

    float	closingStock = 0.00;
	int		envSoFwdRel;
	int		envConOrders = 0, 
			envSoCommHold = 0;
	int		pidNumber;
	int		envQcApply = FALSE, 
			envSkQcAvl = FALSE;

	char	systemDate [11];
	long	lsystemDate = 0L;

	char	type [2];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	soln_type;
	char	startItem [17];
	char	endItem [17];
	char	itemDesc [2][41];
	float	releaseQuantity;
	float	releasePerLine;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from_item", 	 4, 27, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "ALL", "Start Item  :", "Default is ALL ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startItem}, 
	{1, LIN, "desc1", 	 5, 27, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description :", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.itemDesc [0]}, 
	{1, LIN, "to_item", 	 7, 27, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "ALL", "End   Item  :", "Default is ALL ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endItem}, 
	{1, LIN, "desc2", 	 8, 27, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description :", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.itemDesc [1]}, 
	{1, LIN, "releaseQuantity", 	10, 27, FLOATTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "99999999.00", "Max Release Qty       :", "Max quantity to release of product.", 
		YES, NO, JUSTRIGHT, "0", "999999999.99", (char *)&local_rec.releaseQuantity}, 
	{1, LIN, "releasePerLine", 	11, 27, FLOATTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "99999999.99", "Max Release Per Line  :", "Enter Maximum amount to supply per order line. ", 
		YES, NO, JUSTRIGHT, "0", "999999999.99", (char *)&local_rec.releasePerLine}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include 	<RealCommit.h>
/*
 * Function Declarations 
 */
void 	SetDefaults 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	ProcessInmr 		(void);
void 	ProcessAvailable 	(void);
void 	SortSoln 			(void);
void 	SortItln 			(void);
void 	DeleteColn 			(long);
void 	UpdateSohr 			(long);
void 	UpdateIthr 			(long);
void 	ProcessAll 			(void);
void 	FreeCCList 			(void);
void 	ItemSort 			(struct SORT_LIST *);
void 	InitSort 			(void);
void 	FreeSort 			(void);
long 	CreateIthr 			(void);
float 	ProcessPhantom 		(long);
int  	FindInws 			(void);
int  	spec_valid 			(int);
int  	heading 			(int);
struct 	RCV_CC_LIST *FindIthr (void);
static struct SORT_LIST *alloc_sort (void);

/*
 *	Structure for dynamic array, for the sorting of orders using qsort	
 */
struct SoStructure
{
	long	hhbrHash;
}	*orders;
	DArray orders_d;

int	salesOrderCount = 0;

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		i;

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	pidNumber = getpid ();

	if (argc != 3)
	{
		print_at (0, 0, mlSoMess700, argv [0]);
		return (EXIT_FAILURE);
	}

	switch (argv [1][0])
	{
	case 'A':
	case 'M':
		break;

	default:
		print_at (0, 0, mlSoMess001);
		sleep (sleepTime);
		return (EXIT_FAILURE);
		break;
	}

	switch (argv [2][0])
	{
	case 'S':
		local_rec.soln_type = 'S';
		break;

	case 'T':
		local_rec.soln_type = 's';
		break;

	default:
		print_at (0, 0, mlSoMess002);
		sleep (sleepTime);
		return (EXIT_FAILURE);
		break;
	}

	sptr = chk_env ("SO_FWD_REL");
	envSoFwdRel = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*
	 * check forward for order consolidation	
	 */
	sptr = chk_env ("CON_ORDERS");
	envConOrders = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*
	 * Check if Comment lines are held. 
	 */
	sptr = chk_env ("SO_COMM_HOLD");
	envSoCommHold = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/* QC module is active or not. */
	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/* Whether to include QC qty in available stock. */
	envSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	/*
	 * Setup required parameters 
	 */
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB ();

	sprintf (type, "%1.1s", argv [1]);
	if (AUTO)
	{
		dsp_screen ("Processing Backorders ", comm_rec.co_no, comm_rec.co_name);
		/*
		 * Allocate the initial array. 
		 */
		ArrAlloc (&orders_d, &orders, sizeof (struct SoStructure), 1000);

		salesOrderCount = 0;

		soln_rec.hhbr_hash	=	0L;
		cc = find_rec (soln2, &soln_rec, GTEQ, "r");
		while (!cc)
		{
			if (!SOLN_BACKORDER)
			{
				cc = find_rec (soln2, &soln_rec, NEXT, "r");
				continue;
			}
			/*
			 * Read sales order header to check company. 
			 */
			sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
			cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
			if (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no))
			{
				/*
				 * Check the array size before adding new element. 
				 */
				if (!ArrChkLimit (&orders_d, orders, salesOrderCount))
					sys_err ("ArrChkLimit (orders)", ENOMEM, PNAME);

				orders [salesOrderCount].hhbrHash = soln_rec.hhbr_hash;

				/*
				 * Increment array counter. 
				 */
				salesOrderCount++;
			}
			cc = find_rec (soln2, &soln_rec, NEXT, "r");
		}
		itln_rec.due_date 	= 0L;
		itln_rec.hhbr_hash 	= 0L;
		cc = find_rec (itln3, &itln_rec, GTEQ, "r");
		while (!cc)
		{
			/*
			 * If itln record is a backorder and less-than todays	
			 * date and in current warehouse then record is valid.
			 */
			if (!ITLN_BACKORDER)
			{
				cc = find_rec (itln3, &itln_rec, NEXT, "r");
				continue;
			}
			/*
			 * Read transfer header to check company. 
			 */
			ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
			cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
			if (!cc && !strcmp (ithr_rec.co_no, comm_rec.co_no))
			{
				/*
				 * Check the array size before adding new element. 
				 */
				if (!ArrChkLimit (&orders_d, orders, salesOrderCount))
					sys_err ("ArrChkLimit (orders)", ENOMEM, PNAME);

				orders [salesOrderCount].hhbrHash = itln_rec.hhbr_hash;
			}
			cc = find_rec (itln3, &itln_rec, NEXT, "r");
		}
		abc_selfield (inmr, "inmr_hhbr_hash");

		for (i = 0; i < salesOrderCount; i++)
		{
			inmr_rec.hhbr_hash = orders [i].hhbrHash;
			cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
			if (!cc)
			{
				dsp_process ("Item ", inmr_rec.item_no);

				if (inmr_rec.bo_release [0] == 'A')
					ProcessAvailable ();
			}
		}
		ArrDelete (&orders_d);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		SetDefaults ();

		/*
		 * Entry screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input 
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		ProcessInmr ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefaults (void)
{
	strcpy (local_rec.startItem, "ALL             ");
	strcpy (local_rec.endItem, "ALL             ");
	FLD ("releaseQuantity")	= NA;
	FLD ("releasePerLine")	= NA;
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
	int	field)
{
	/*
	 * Validate Category 
	 */
	if (LCHECK ("from_item"))
	{
		skip_entry = 1;
		if (dflt_used)
		{
			sprintf (local_rec.startItem, "%-16.16s", "ALL");
			sprintf (local_rec.endItem, "%-16.16s", "ALL");
			sprintf (local_rec.itemDesc [0], "%-40.40s", "ALL Items");
			sprintf (local_rec.itemDesc [1], "%-40.40s", "ALL Items");
			local_rec.releaseQuantity	= 0.0;
			local_rec.releasePerLine 	= 0.0;
			FLD ("releaseQuantity") 	= NA;
			FLD ("releasePerLine") 		= NA;
			DSP_FLD ("from_item");
			DSP_FLD ("to_item");
			DSP_FLD ("desc1");
			DSP_FLD ("desc2");
			DSP_FLD ("releaseQuantity");
			DSP_FLD ("releasePerLine");
			skip_entry = 3;
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.endItem, "ALL             ") &&
			strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.itemDesc [0], inmr_rec.description);

		SuperSynonymError ();

		DSP_FLD ("from_item");
		DSP_FLD ("desc1");

		if (!strcmp (local_rec.startItem, local_rec.endItem))
		{
			FLD ("releaseQuantity") = YES;
			FLD ("releasePerLine") 	= YES;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to_item"))
	{
		skip_entry = 1;
		if (dflt_used)
		{
			sprintf (local_rec.endItem, "%-16.16s", "ALL");
			sprintf (local_rec.itemDesc [1], "%-40.40s", "ALL Items");
			local_rec.releaseQuantity 	= 0.0;
			local_rec.releasePerLine 	= 0.0;
			FLD ("releaseQuantity") 	= NA;
			FLD ("releasePerLine") 		= NA;
			DSP_FLD ("to_item");
			DSP_FLD ("desc2");
			DSP_FLD ("releaseQuantity");
			DSP_FLD ("releasePerLine");
			skip_entry = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.startItem, "ALL             ") &&
			strcmp (local_rec.endItem, "ALL             "))
		{
			print_mess (ML (mlStdMess133));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.itemDesc [1], inmr_rec.description);

		SuperSynonymError ();

		DSP_FLD ("to_item");
		DSP_FLD ("desc2");

		if (!strcmp (local_rec.startItem, local_rec.endItem))
		{
			FLD ("releaseQuantity")	= YES;
			FLD ("releasePerLine")	= YES;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("releasePerLine"))
	{
		if (local_rec.releaseQuantity < local_rec.releasePerLine)
		{
			errmess (ML (mlSoMess003));
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}


/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	ReadMisc ();

	abc_alias (ithr2, ithr);
	abc_alias (itln2, itln);
	abc_alias (itln3, itln);
	abc_alias (soln2, soln);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (soln2,soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (ithr2,ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no2");
	open_rec (itln2,itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (itln3,itln_list, ITLN_NO_FIELDS, "itln_id_no2");
	open_rec (inws, inws_list, INWS_NO_FIELDS, "inws_id_no");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
}

/*
 * Close Database files. 
 */
void
CloseDB (void)
{
	/*
	 * De-allocate the memory that was 'malloc'ed for sorting.	
	 */
	FreeSort ();

	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_fclose (coln);
	abc_fclose (incc);
	abc_fclose (ithr);
	abc_fclose (ithr2);
	abc_fclose (itln);
	abc_fclose (itln2);
	abc_fclose (itln3);
	abc_fclose (inws);
	abc_fclose	(soic);
	SearchFindClose ();
	abc_dbclose ("data");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");

	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	abc_fclose (comm);
}

void
ProcessInmr (void)
{
	char	startItem [17], 
			endItem [17];

	dsp_screen ("Processing Backorders ", comm_rec.co_no, comm_rec.co_name);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	if (strcmp (local_rec.startItem, "ALL             "))
		strcpy (startItem, local_rec.startItem);
	else
		sprintf (startItem, "%-16.16s", " ");

	if (strcmp (local_rec.endItem, "ALL             "))
		strcpy (endItem, local_rec.endItem);
	else
		sprintf (endItem, "%-16.16s", "~");

	strcpy (inmr_rec.item_no, startItem);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
                       strcmp (inmr_rec.item_no, startItem) >= 0 &&
                       strcmp (inmr_rec.item_no, endItem) <= 0)
	{
		if (inmr_rec.bo_release [0] == 'A')
			ProcessAvailable ();

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
ProcessAvailable (void)
{
	float	realCommitted;

	if (!AUTO)
		dsp_process ("Item ", inmr_rec.item_no);

	if (PHANTOM)
		closingStock = ProcessPhantom (inmr_rec.hhbr_hash);
	else
	{
		incc_rec.hhcc_hash	= 	ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash 	= 	alt_hash 
									(
										inmr_rec.hhbr_hash, 
						  				inmr_rec.hhsi_hash
									);
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			return;
		else
		{
			/*
			 * Calculate Actual Qty Committed. 
			 */
			realCommitted = RealTimeCommitted 
							(
								incc_rec.hhbr_hash, 
							   	incc_rec.hhcc_hash
							);
			if (envSoFwdRel)
			{
				closingStock = incc_rec.closing_stock -
						 	   incc_rec.committed -
						 	   realCommitted -
						 	   incc_rec.forward;
			}
			else
			{
				closingStock = incc_rec.closing_stock -
						 	   realCommitted -
						 	   incc_rec.committed;
			}
			if (envQcApply && envSkQcAvl)
				closingStock -= incc_rec.qc_qty;

			if (incc_rec.closing_stock <= 0.00 || closingStock < 0.00)
				closingStock = 0.00;

			if (NON_STOCK || COMMENT_LINE)
				closingStock = 0.00;
		}
	}

	/*
	 * Assign maximum amount available for releasing	
	 */
	if (!AUTO && strncmp (local_rec.startItem, "ALL", 3))
		closingStock = InternalMIN (closingStock, local_rec.releaseQuantity);

	if (closingStock > 0.00 || COMMENT_LINE || NON_STOCK)
	{
		SortSoln ();
		SortItln ();
		ProcessAll ();

		/*
		 * Xfer sorted list to	the local 'free-heap'
		 */
		InitSort ();

		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
			inmr_rec.hhbr_hash, 
			0L, 
			0L, 
			(double) 0.00
		);
	}
	abc_unlock (incc);

	FreeCCList ();

	return;
}

void
SortSoln (void)
{
	struct	SORT_LIST	sort_item;

	soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*
		 * If Soln record is a backorder and less-than todays	
		 * date and in current warehouse then record is valid.
		 */
		if (!SOLN_BACKORDER ||
			soln_rec.due_date > lsystemDate ||
			soln_rec.hhcc_hash != ccmr_rec.hhcc_hash)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		sort_item.sort_stat = ' ';
		sort_item.sort_date = soln_rec.due_date;
		sort_item.sort_type = local_rec.soln_type;
		sort_item.sort_hash = soln_rec.hhsl_hash;
		strcpy (sort_item.sort_bo_flag, inmr_rec.bo_flag);
		strcpy (sort_item.sort_class, inmr_rec.inmr_class);

		ItemSort (&sort_item);

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

void
SortItln (void)
{
	struct	SORT_LIST	sort_item;

	abc_selfield (itln, "itln_id_no2");
	itln_rec.due_date = 0L;
	itln_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		/*
		 * If itln record is a backorder and less-than todays	
		 * date and in current warehouse then record is valid.
		 */
		if (!ITLN_BACKORDER ||
			itln_rec.due_date > lsystemDate ||
			itln_rec.i_hhcc_hash != ccmr_rec.hhcc_hash)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}

		sort_item.sort_stat = (itln_rec.stock [0] == 'C') ? ' ' : '1';
		sort_item.sort_date = itln_rec.due_date;
		sort_item.sort_type = 'T';
		sort_item.sort_hash = itln_rec.itff_hash;
		strcpy (sort_item.sort_bo_flag, inmr_rec.bo_flag);
		strcpy (sort_item.sort_class, inmr_rec.inmr_class);

		ItemSort (&sort_item);

		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
}

/*
 * Delete line from existing packing slip so new backorder can be released. 
 */
void
DeleteColn (
	long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (coln);
		return;
	}
	if (coln_rec.status [0] == 'P')
		abc_delete (coln);
	else
		abc_unlock (coln);
	return;
}

/*
 * Update Header to same status of lines. 
 */
void
UpdateSohr (
	long	hhsoHash)
{
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (!cc)
	{
		strcpy (sohr_rec.sohr_new, "N");
		strcpy (sohr_rec.status, "C");
		strcpy (sohr_rec.stat_flag, (envConOrders) ? "M" : "R");
		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, sohr, "DBUPDATE");
	}
	else
		abc_unlock (sohr);
}
/*
 * Update Header to same status of lines. 
 */
void
UpdateIthr (
	long	hhitHash)
{
	ithr_rec.hhit_hash	=	hhitHash;
	cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
	if (!cc)
	{
		strcpy (ithr_rec.type, "U");
		if (ithr_rec.full_supply [0] != 'Y')
			ithr_rec.del_no = 0L;
		cc = abc_update (ithr, &ithr_rec);
		if (cc)
			file_err (cc, ithr, "DBUPDATE");
	}
	else
		abc_unlock (ithr);
}

void
ProcessAll (void)
{
	int		SpecialClass = FALSE;
	int		orderMultipleRelease;
	int		updateRequired 	= FALSE;
	float	quantityOrder 	= 0.0;
	float	quantitySupply 	= 0.0;
	float	quantityRemain 	= local_rec.releasePerLine;
	float	tmpWorkQuantity = 0.0;
	float	currentStock	= 0.0;
	struct	SORT_LIST	*sort_curr;
	struct	RCV_CC_LIST	*tmp_list;

	abc_selfield (itln, "itln_itff_hash");
	abc_selfield (soln, "soln_hhsl_hash");
	sort_curr = sort_head;

	SpecialClass = FALSE;

	while (sort_curr != SORT_NULL)
	{
	    abc_unlock (itln);
	    abc_unlock (soln);

	    orderMultipleRelease = FALSE;

	    if (sort_curr->sort_type == 'T')
	    {
			itln_rec.itff_hash	=	sort_curr->sort_hash;
			cc = find_rec (itln, &itln_rec, GTEQ, "u");
			if (cc)
				file_err (cc, itln, "DBFIND");

			/*
			 * Look up inws to get order multiple 
			 */
			orderMultipleRelease = FindInws ();
			if (strcmp (itln_rec.tran_ref, "Inter W/H Trans."))
				orderMultipleRelease = FALSE;

			quantityOrder = itln_rec.qty_border;

			if (sort_curr->sort_class [0] == 'Z' && envSoCommHold) 
				SpecialClass = TRUE;
	    }
	    else
	    {
			soln_rec.hhsl_hash	=	sort_curr->sort_hash;
			cc = find_rec (soln, &soln_rec, GTEQ, "u");
			if (cc)
				file_err (cc, soln, "DBFIND");

			quantityOrder = soln_rec.qty_order + soln_rec.qty_bord;
	    }
	    updateRequired = TRUE;

	    SpecialClass = (sort_curr->sort_class [0] == 'N') ? TRUE : FALSE;

	    /*
	     * closingStock need to be held and reduced as	
	     * each valid soln/itln record is processed.
	     */
	    if (closingStock > 0.00 || SpecialClass)
	    {
			/*
			 * We used to check if the order had multiple occurences	
			 * of the same product and thereby limit the customer to
			 * a fixed quantity 'per-order'.						
			 * This is now done on a 'per-line' basis.				
			 */
			if (!AUTO && strncmp (local_rec.startItem, "ALL", 3))
			{
				quantityRemain =	InternalMIN 
									(
										closingStock, 
										local_rec.releasePerLine
									);
			}
			else
		    	quantityRemain = closingStock;

			quantitySupply	=	InternalMIN 
								(
									quantityRemain, 
									quantityOrder
								);

			if (sort_curr->sort_type != 'T')
			{
				sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
		    	cc = find_rec (sohr, &sohr_rec, EQUAL, "r");
		    	if (cc)
		    	{
			    	abc_unlock (soln);
			    	sort_curr = sort_curr->_next;
			    	continue;
		    	}
		    	if (sohr_rec.full_supply [0] == 'Y')
		    	{
					if (quantitySupply < quantityOrder)
					{
			    		abc_unlock (soln);
			    		sort_curr = sort_curr->_next;
			    		continue;
					}
		    	}
				/*
				 * Process Sales Orders 
				 */
				soln_rec.qty_order = quantitySupply;
				soln_rec.qty_bord  = quantityOrder - quantitySupply;

		    	if (sort_curr->sort_bo_flag [0] == 'F')
		    	{
					if (soln_rec.qty_bord != 0.00)
					{
			    		soln_rec.qty_bord = quantityOrder;
			    		soln_rec.qty_order = 0.00;
			    		quantitySupply = 0.00;
					}
		    	}

		    	closingStock -= quantitySupply;

		    	DeleteColn (soln_rec.hhsl_hash);

				if (soln_rec.qty_order <= 0.00)
				{
					strcpy (soln_rec.status, "B");
					strcpy (soln_rec.stat_flag, "B");
				}
		    	else
		    	{
					strcpy (soln_rec.status, "C");
					strcpy (soln_rec.stat_flag, (envConOrders) ? "M" : "R");
		    	}
	
		    	cc = abc_update (soln, &soln_rec);
		    	if (cc)
					file_err (cc, soln, "DBUPDATE");
	
		    	UpdateSohr (soln_rec.hhso_hash);
			}
			else
			{
				/*
				 * Process Transfers 
				 */
				if (quantitySupply == 0.00 && !SpecialClass)
				{
					sort_curr = sort_curr->_next;
					continue;
				}

		    	if (sort_curr->sort_bo_flag [0] == 'F')
		    	{
					if (itln_rec.qty_border != quantitySupply)
					{
						sort_curr = sort_curr->_next;
						continue;
				}
		    }

		    /*
		     * Phase I             
		     * Update the original xfer line.          
		     */
		    currentStock = closingStock;
		    closingStock -= quantitySupply;

		    /*
		     * Full supply transfer 
		     */
		    if (itln_rec.full_supply [0] == 'Y')
		    {
				/*
				 * Release only in multiples of ord_multiple. 
				 */
				if (orderMultipleRelease)
				{
					/*
					 * We must supply in multiples of 
					 * inws_ord_multiple. The qty we 
					 * supply must be >= qty ordered
					 */
					quantitySupply = rnd_mltpl 
									(
										"U", 
										inws_rec.ord_multiple, 
										quantitySupply
									);
					if (quantitySupply > currentStock)
					{
						closingStock += quantitySupply;
						sort_curr = sort_curr->_next;
						continue;
					}
				}
				if (quantitySupply < itln_rec.qty_border)
				{
					closingStock += quantitySupply;
					abc_unlock (itln);
					sort_curr = sort_curr->_next;
					continue;
				}
				itln_rec.qty_border	= 0.00;
				itln_rec.qty_order	+= quantitySupply;
				strcpy (itln_rec.status, "U");
				cc = abc_update (itln, &itln_rec);
				if (cc)
			    	file_err (cc, itln, "DBUPDATE");

				UpdateIthr (itln_rec.hhit_hash);

				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"RC", 
					0, 
					itln_rec.hhbr_hash, 
					itln_rec.r_hhcc_hash, 
					0L, 
					(double) 0.00
				);

				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"RC", 
					0, 
					itln_rec.hhbr_hash, 
					itln_rec.i_hhcc_hash, 
					0L, 
					(double) 0.00
				);
				sort_curr = sort_curr->_next;
				continue;
		    }

		    /*
		     * Full supply transfer line 
		     */
		    if (itln_rec.full_supply [0] == 'L')
		    {
				/*
				 * Release only in multiples of ord_multiple. 
				 */
				if (orderMultipleRelease)
				{
					/*
					 * We must supply in multiples of 
					 * inws_ord_multiple. The qty we  
					 * supply must be >= qty ordered. 
					 */
					quantitySupply = rnd_mltpl 
									(
										"U", 
										inws_rec.ord_multiple, 
										quantitySupply
									);
					if (quantitySupply > currentStock)
					{
						closingStock += quantitySupply;
						sort_curr = sort_curr->_next;
						continue;
					}
				}

				if (quantitySupply < itln_rec.qty_border)
				{
					closingStock += quantitySupply;
					abc_unlock (itln);
					sort_curr = sort_curr->_next;
					continue;
				}
		    }

		    /*
		     * Release only in multiples of ord_multiple. 
		     */
		    if (orderMultipleRelease)
		    {
				/*
				 * We must supply in multiples of inws_ord_multiple. 
				 * If we have sufficient stock to supply the next   
				 * multiple up then do so, otherwise supply the next
				 * multiple down.                                   
				 */
				tmpWorkQuantity = rnd_mltpl 
								(
									"U", 
									inws_rec.ord_multiple, 
									quantitySupply
							   	 );
				if (tmpWorkQuantity > currentStock)
				{
					quantitySupply = rnd_mltpl 
									(
										"D", 
										inws_rec.ord_multiple, 
										quantitySupply
									);
				}
				else
					quantitySupply = tmpWorkQuantity;
				}

				itln_rec.qty_border = 0.00;
				if (itln_rec.qty_order == 0.00 && itln_rec.qty_border == 0.00)
		    	{
		 			strcpy (itln_rec.status, "D");
		    	}

		    	cc = abc_update (itln, &itln_rec);
		    	if (cc)
					file_err (cc, itln, "DBUPDATE");
	
		    	abc_unlock (itln);

				/*
				 * Don't process 'Z' class items. 
				 */
				if (sort_curr->sort_class [0] == 'Z') 
				{
					sort_curr = sort_curr->_next;
					continue;
				}

				/*
				 * Phase II		        
				 * Create a new line for the released B/order portion. 
				 */
				tmp_list = FindIthr ();
				itln_rec.hhit_hash  = tmp_list->hhitHash;
				itln_rec.line_no    = tmp_list->line_no++;
				itln_rec.qty_order  = quantitySupply;
				itln_rec.qty_border = quantityOrder - quantitySupply;
				itln_rec.status [0] = 'U';
				cc = abc_add (itln, &itln_rec);
				if (cc)
					file_err (cc, itln, "DBADD");

				cc = find_rec (itln2, &itln_rec, EQUAL, "r");
				if (cc)
					file_err (cc, itln2, "DBFIND");

				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"RC", 
					0, 
				   	itln_rec.hhbr_hash, 
				   	itln_rec.r_hhcc_hash, 
				   	0L, 
				   	(double) 0.00
			  	);
				add_hash 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					"RC", 
					0, 
				   	itln_rec.hhbr_hash, 
				   	itln_rec.i_hhcc_hash, 
				   	0L, 
				   	(double) 0.00
			  	);
			}
	    }
	    sort_curr = sort_curr->_next;
	}

	abc_unlock (itln);
	abc_unlock (soln);

	abc_selfield (itln, "itln_id_no2");
	abc_selfield (soln, "soln_hhbr_hash");
}

/*
 * Look up inws to get order multiple 
 */
int
FindInws (void)
{
	inws_rec.hhbr_hash	=	itln_rec.hhbr_hash;
	inws_rec.hhcf_hash	=	0L;
	inws_rec.hhcc_hash	=	itln_rec.i_hhcc_hash;
	return (!find_rec (inws, &inws_rec, COMPARISON, "r"));
}

void
FreeCCList (void)
{
	struct	RCV_CC_LIST	*tmp_list, *rcv_cc_curr;

	for (rcv_cc_curr = rcv_cc_head; rcv_cc_curr != RCV_CC_NULL;)
	{
		tmp_list = rcv_cc_curr;
		rcv_cc_curr = rcv_cc_curr->_next;
		free (tmp_list);
	}
	rcv_cc_head = RCV_CC_NULL;
}

struct	RCV_CC_LIST	*
FindIthr (void)
{
	struct	RCV_CC_LIST
		*tmp_list = (struct RCV_CC_LIST *) 0, 
		*rcv_cc_curr;

	if (rcv_cc_head == RCV_CC_NULL)
	{
		rcv_cc_head = (struct RCV_CC_LIST *) malloc (sizeof (struct RCV_CC_LIST));
		if (rcv_cc_head == RCV_CC_NULL)
			file_err (errno, "FindIthr", "MALLOC");

		rcv_cc_head->hhccHash = itln_rec.r_hhcc_hash;
		rcv_cc_head->hhitHash = CreateIthr ();
		rcv_cc_head->line_no = 1;
		rcv_cc_head->_next = RCV_CC_NULL;

		return (rcv_cc_head);
	}

	for (rcv_cc_curr = rcv_cc_head; rcv_cc_curr != RCV_CC_NULL; rcv_cc_curr = rcv_cc_curr->_next)
	{
		if (rcv_cc_curr->hhccHash == itln_rec.r_hhcc_hash)
			return (rcv_cc_curr);

		tmp_list = rcv_cc_curr;
	}

	rcv_cc_curr = tmp_list;
	tmp_list = (struct RCV_CC_LIST *) malloc (sizeof (struct RCV_CC_LIST));
	if (tmp_list == RCV_CC_NULL)
		file_err (errno, "FindIthr", "MALLOC");

	rcv_cc_curr->_next = tmp_list;
	tmp_list->hhccHash = itln_rec.r_hhcc_hash;
	tmp_list->hhitHash = CreateIthr ();
	tmp_list->line_no = 1;
	tmp_list->_next = RCV_CC_NULL;

	return (tmp_list);
}

long	
CreateIthr (void)
{
	char	tmp_date [11], 
			iss_co [3], 
			rec_co [3];

	strcpy (tmp_date, DateToString (TodaysDate ()));

	ccmr2_rec.hhcc_hash = itln_rec.r_hhcc_hash;
	cc = find_rec (ccmr, &ccmr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr2_rec", "DBFIND");

	strcpy (rec_co, ccmr2_rec.co_no);

	ccmr2_rec.hhcc_hash = itln_rec.i_hhcc_hash;
	cc = find_rec (ccmr, &ccmr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr2_rec", "DBFIND");

	strcpy (iss_co, ccmr2_rec.co_no);

	if (strcmp (iss_co, rec_co))
		strcpy (ithr_rec.co_no, "  ");
	else
		strcpy (ithr_rec.co_no, iss_co);

	strcpy (ithr_rec.type, "U");
	ithr_rec.del_no = 900000L + (long) pidNumber;
	ithr_rec.iss_sdate = StringToDate (tmp_date);
	ithr_rec.iss_date = StringToDate (tmp_date);
	ithr_rec.rec_date = 0L;
	strcpy (ithr_rec.tran_ref, "Released Tran BO");
	strcpy (ithr_rec.printed, "N");
	strcpy (ithr_rec.stat_flag, "0");
	cc = abc_add (ithr, &ithr_rec);
	if (cc)
		file_err (cc, ithr, "DBADD");

	cc = find_rec (ithr2, &ithr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, ithr2, "DBFIND");

	ithr_rec.del_no = 0L;
	cc = abc_update (ithr2, &ithr_rec);
	if (cc)
		file_err (cc, ithr2, "DBUPDATE");

	return (ithr_rec.hhit_hash);
}

/*
 * To save heaps of 'malloc'ing, we maintain 2 lists of ptrs.
 * One is the actual data & the other is previously used ptrs.
 */
void
InitSort (void)
{
	/*
	 * If the sort_list is already empty, return 
	 */
	if (sort_head == SORT_NULL)
		return;

	/*
	 * If the free_list is already empty, 	
	 * just transfer the sort_list to the
	 * free_list and return.			
	 */
	if (free_head == SORT_NULL)
	{
		free_head = sort_head;
		sort_head = SORT_NULL;
		sort_tail = SORT_NULL;
		return;
	}

	/*
	 * Otherwise, place the free_list at the	end	
	 * of the sort_list and call it the free_list.
	 */
	sort_tail->_next = free_head;
	free_head->_prev = sort_tail;
	free_head = sort_head;
	sort_head = SORT_NULL;
	sort_tail = SORT_NULL;
	return;
}

/*
 * This function is called when ALL of the associated 'malloc'ed memory isnt
 * needed anymore.			|
 */
void
FreeSort (void)
{
	struct	SORT_LIST	*curr_list;

	InitSort ();
	if (sort_head == SORT_NULL)
		return;
	curr_list = sort_head->_next;
	while (curr_list != SORT_NULL)
	{
		free (sort_head);
		sort_head = curr_list;
		curr_list = curr_list->_next;
	}
	free (sort_head);
	sort_head = SORT_NULL;
	sort_tail = SORT_NULL;
	free_head = SORT_NULL;
	return;
}

static struct SORT_LIST *
alloc_sort (void)
{
	struct	SORT_LIST *tmp_sort;

	/*
	 * Firstly, try to allocate off the free_list.	
	 * If it is empty, allocate space using malloc.	
	 */
	tmp_sort = free_head;
	if (tmp_sort != SORT_NULL)
	{
		free_head = tmp_sort->_next;
		return (tmp_sort);
	}

	tmp_sort = (struct SORT_LIST *) malloc (sizeof (struct SORT_LIST));
	if (tmp_sort == SORT_NULL)
		file_err (cc, "alloc_sort", "MALLOC");

	return (tmp_sort);
}

void
ItemSort (
 struct SORT_LIST *sort_item)
{
	struct	SORT_LIST	*tmp_sort, 
				*curr_sort;

	/*
	 * Allocate a new structure and copy in the relevant data.	
	 */
	tmp_sort = alloc_sort ();
	tmp_sort->sort_stat = sort_item->sort_stat;
	tmp_sort->sort_date = sort_item->sort_date;
	tmp_sort->sort_type = sort_item->sort_type;
	tmp_sort->sort_hash = sort_item->sort_hash;
	strcpy (tmp_sort->sort_bo_flag, sort_item->sort_bo_flag);
	strcpy (tmp_sort->sort_class, sort_item->sort_class);

	/*
	 * Search for position within sort_list and insert 'record'	
	 */
	if (sort_head == SORT_NULL)
	{
		tmp_sort->_prev = SORT_NULL;
		tmp_sort->_next = SORT_NULL;
		sort_head = tmp_sort;
		sort_tail = tmp_sort;
		return;
	}

	for (curr_sort = sort_tail; curr_sort != SORT_NULL;
					curr_sort = curr_sort->_prev)
	{
	    if (curr_sort->sort_stat > tmp_sort->sort_stat)
	        continue;

	    if (curr_sort->sort_stat == tmp_sort->sort_stat)
	    {
			if (curr_sort->sort_date > tmp_sort->sort_date)
		    	continue;

			if (curr_sort->sort_date == tmp_sort->sort_date)
			{
		    	if (curr_sort->sort_type > tmp_sort->sort_type)
					continue;

		    	if (curr_sort->sort_type == tmp_sort->sort_type)
		    	{
					if (curr_sort->sort_hash > tmp_sort->sort_hash)
						continue;
		    	}
			}
	    }
	    tmp_sort->_next = curr_sort->_next;
	    curr_sort->_next = tmp_sort;
	    tmp_sort->_prev = curr_sort;
	    if (tmp_sort->_next == SORT_NULL)
			sort_tail = tmp_sort;
	    else
			tmp_sort->_next->_prev = tmp_sort;

	    return;
	}
	tmp_sort->_next = sort_head;
	tmp_sort->_prev = SORT_NULL;
	sort_head->_prev = tmp_sort;
	sort_head = tmp_sort;
	return;
}

/*
 * Specific code to handle single level Bills. 
 */
float	
ProcessPhantom (
	long	hhbrHash)
{
	float	minimumQuantity = 0.00, 
			realCommitted, 
			onHandQuantity = 0.00;

	int		firstTime = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		/*
		 * Calculate Actual Qty Committed. 
		 */
		realCommitted = 	RealTimeCommitted 
							(
								incc_rec.hhbr_hash, 
								incc_rec.hhcc_hash
							);
		if (envSoFwdRel)
		{
			onHandQuantity = incc_rec.closing_stock -
					  		 realCommitted -
					  		 incc_rec.committed -
			          		 incc_rec.forward;
		}
		else
		{
			onHandQuantity = incc_rec.closing_stock -
					  		 realCommitted -
				  	  		 incc_rec.committed;
		}
		if (envQcApply && envSkQcAvl)
			onHandQuantity -= incc_rec.qc_qty;

		if (sokt_rec.matl_qty == 0.00)
			sokt_rec.matl_qty	=	1.00;

		onHandQuantity /= sokt_rec.matl_qty;
		if (firstTime)
			minimumQuantity = onHandQuantity;

		if (minimumQuantity > onHandQuantity)
			minimumQuantity = onHandQuantity;

		firstTime = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (minimumQuantity);
}
int
heading (
	int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlSoMess004), 25, 0, 1);

		box (0, 3, 80, 8);

		line_at (1, 0, 80);
		line_at (6, 1, 79);
		line_at (9, 1, 79);
		line_at (20,0, 80);
		line_at (22,0, 80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name); 
		strcpy (err_str, ML (mlStdMess039));
		print_at (21, 45, err_str, comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
