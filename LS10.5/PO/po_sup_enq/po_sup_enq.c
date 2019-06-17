/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_sup_enq.c,v 5.4 2002/07/24 08:39:08 scott Exp $
|  Program Name  : (po_sup_enq.c)                                     |
|  Program Desc  : (Purchase Order Enquiry by Supplier.)              |	
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 26/08/93         |
|---------------------------------------------------------------------|
| $Log: po_sup_enq.c,v $
| Revision 5.4  2002/07/24 08:39:08  scott
| Updated to ensure SetSortArray () is after set_masks
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_sup_enq.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_sup_enq/po_sup_enq.c,v 5.4 2002/07/24 08:39:08 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>
#include 	<arralloc.h>

#define		INTPSIZE		10
#define		ScreenWidth		132
#define		SORT_FILENAME	"supenq"
#define		STOCK_ORDER		 (suph_rec.drop_ship [0] != 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocr_rec;
struct sumrRecord	sumr_rec;
struct suphRecord	suph_rec;

	extern	int	TruePosition;
	int		envVarCrCo 		= 0,
			envVarCrFind 	= 0;

	char	branchNumber [3],
			*fifteenSpaces	=	"               ",
			*data	= "data";

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [2];
	char	sortStr	 [256];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

	long	lsystemDate;
		
	FILE	*fout,
			*fsort;

/*
 * Local & Screen Structures. 
 */
struct {
	char	selectionType [2];
	char	selectionDesc [10];
	char	startSuppNo [7];
	char	startSuppName [41];
	char	endSuppNo [7];
	char	endSuppName [41];
	char	startClass [2];
	char 	startCategoryNo [12];
	char	startCategoryDesc [41];
	char	endClass [2];
	char	endCategoryNo [12];
	char	endCategoryDesc [41];
	char	displayType [2];
	char	displayDesc [31];
	char	dummy [11];
	int		startPriorityNo;
	int		endPriorityNo;
} local_rec;

/*============================
| Branch Structure.          |
============================*/
struct {
	char	br_no [3];
	char	br_short_name [16];
	double	stk_mtd_total;
	double	stk_last_12_mths;
	double	stk_prev_12_mths;
	double	tot_mtd_total;
	double	tot_last_12_mths;
	double	tot_prev_12_mths;
} branch_array [100];

static	struct	var	vars [] =
{
	{1, LIN, "selectionType",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Supplier Selection     ", "N)umber, C)ategory, P)riority",
		YES, NO,  JUSTLEFT, "NCP", "", local_rec.selectionType},
	{1, LIN, "selectionDesc",	 4, 28, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.selectionDesc},
	{1, LIN, "startSupplier",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier         ", "Enter first Supplier Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSuppNo},
	{1, LIN, "startSupplierDesc",	 6, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startSuppName},
	{1, LIN, "endSuppNo",	 7, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier           ", "Enter last Supplier Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSuppNo},
	{1, LIN, "endSupplierDesc",	 7, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endSuppName},
    {1, LIN, "startClass", 6, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class            ", "Input Start Class A-Z.", 
		ND, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.startClass},
	{1, LIN, "startCategoryNo", 6, 30, CHARTYPE,
		"UUUUUUUUUUU", "          ", 
		" ", " ", "Start Category         ", "Input Start Inventory Category.", 
		ND, NO, JUSTLEFT, "", "", local_rec.startCategoryNo}, 
	{1, LIN, "startCategoryDesc", 6, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.startCategoryDesc}, 
	{1, LIN, "endClass", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Z", "End Class              ", "Input End Class A-Z.", 
		ND, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.endClass}, 
	{1, LIN, "endCategoryNo", 7, 30, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", " ", "End Category           ", "Input End Inventory Category.", 
		ND, NO, JUSTLEFT, "", "", local_rec.endCategoryNo}, 
	{1, LIN, "endCategoryDesc", 7, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.endCategoryDesc}, 
	{1, LIN, "startPriorityNo",	 6, 2, INTTYPE,
		"N", "          ",
		" ", "1", "Start Priority         ", "Priority 1 to 9",
		ND, NO,  JUSTRIGHT, "1", "9", (char *)&local_rec.startPriorityNo},
	{1, LIN, "endPriorityNo",	 7, 2, INTTYPE,
		"N", "          ",
		" ", "9", "End Priority           ", "Priority 1 to 9",
		ND, NO,  JUSTRIGHT, "1", "9", (char *)&local_rec.endPriorityNo},
	{1, LIN, "displayType",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Display Selection      ", "D)elivery Performance, P)urchase Volumes",
		YES, NO,  JUSTLEFT, "DP", "", local_rec.displayType},
	{1, LIN, "displayDesc",	 9, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.displayDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
int 	heading 			(int);
void 	SupplierDisplay 	(void);
void 	DisplayHeader 		(void);
void 	DisplayLine 		(long, long);
int 	DisplayDetail 		(char *);
void 	PerformanceDetails 	(char *);
void 	VolumeDetails 		(char *);
int 	IsCategoryOk 		(long);
int 	FindBranchIndex 	(char *, int);
int		SortFunc			(const	void *,	const void *);
void 	SrchExcf 			(char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int 	argc,
	char	*argv [])
{
	TruePosition	=	TRUE;
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo ) ? comm_rec.est_no : " 0" );

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();     	/*  get into raw mode			*/
	set_masks ();		/*  setup print using masks		*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/
		FLD ("selectionType") 		= YES;
		FLD ("startSupplier") 		= YES;
		FLD ("endSuppNo")	 		= YES;
		FLD ("startSupplierDesc") 	= NA;
		FLD ("endSupplierDesc") 	= NA;
		FLD ("startClass") 			= ND;
		FLD ("endClass") 			= ND;
		FLD ("startCategoryNo") 	= ND;
		FLD ("endCategoryNo") 		= ND;
		FLD ("startCategoryDesc") 	= ND;
		FLD ("endCategoryDesc") 	= ND;
		FLD ("startPriorityNo") 	= ND;
		FLD ("endPriorityNo") 		= ND;

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		SupplierDisplay ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_id_no4");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
							    						       : "sumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (esmr);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (pocr);
	abc_fclose (sumr);
	abc_fclose (suph);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-------------------------
	| Validate Selection Type |
	-------------------------*/
	if (LCHECK ("selectionType") )
	{
		switch (local_rec.selectionType [0])
		{
		case 'N' : 
			strcpy (local_rec.selectionDesc, "Number");
			FLD ("startSupplier") 		= YES;
			FLD ("endSuppNo")	 		= YES;
			FLD ("startSupplierDesc") 	= NA;
			FLD ("endSupplierDesc") 	= NA;
			FLD ("startClass") 			= ND;
			FLD ("endClass") 			= ND;
			FLD ("startCategoryNo") 	= ND;
			FLD ("endCategoryNo") 		= ND;
			FLD ("startCategoryDesc") 	= ND;
			FLD ("endCategoryDesc") 	= ND;
			FLD ("startPriorityNo") 	= ND;
			FLD ("endPriorityNo") 		= ND;
			break;

		case 'C' : 
			strcpy (local_rec.selectionDesc, "Category");
			strcpy (local_rec.startSuppNo, "      ");
			strcpy (local_rec.endSuppNo, "~~~~~~");
		  	FLD ("startSupplier") 		= ND;
		    FLD ("endSuppNo") 			= ND;
		    FLD ("startSupplierDesc") 	= ND;
		    FLD ("endSupplierDesc") 	= ND;
			FLD ("startClass") 			= YES;
			FLD ("endClass") 			= YES;
			FLD ("startCategoryNo") 	= YES;
			FLD ("endCategoryNo") 		= YES;
			FLD ("startCategoryDesc") 	= NA;
			FLD ("endCategoryDesc") 	= NA;
			FLD ("startPriorityNo") 	= ND;
			FLD ("endPriorityNo") 		= ND;
			break;

		case 'P' : 
			strcpy (local_rec.selectionDesc, "Priority");
			strcpy (local_rec.startSuppNo, "      ");
			strcpy (local_rec.endSuppNo, "~~~~~~");
		  	FLD ("startSupplier") 		= ND;
		    FLD ("endSuppNo") 			= ND;
		    FLD ("startSupplierDesc") 	= ND;
		    FLD ("endSupplierDesc") 	= ND;
			FLD ("startClass") 			= ND;
			FLD ("endClass") 			= ND;
			FLD ("startCategoryNo") 	= ND;
			FLD ("endCategoryNo") 		= ND;
			FLD ("startCategoryDesc") 	= ND;
			FLD ("endCategoryDesc") 	= ND;
			FLD ("startPriorityNo") 	= YES;
			FLD ("endPriorityNo") 		= YES;
			break;
		default  : 
			return (EXIT_FAILURE);
		} /* End Switch */

		scn_write (1);
		DSP_FLD ("selectionDesc" );
		FLD ("selectionType") = NA;
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("startSupplier") )
	{
		if (FLD ("startSupplier") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.startSuppName, "%-40.40s", "Start Supplier");
			DSP_FLD ("startSupplierDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if ((strcmp (local_rec.startSuppNo, local_rec.endSuppNo) > 0) &&
		    (strcmp (local_rec.endSuppNo, "      ") != 0))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.startSuppNo));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startSuppName, sumr_rec.crd_name);
		DSP_FLD ("startSupplierDesc" );
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endSuppNo") )
	{
		if (FLD ("endSuppNo") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.endSuppName, "%-40.40s", ML ("End Supplier"));
			DSP_FLD ("endSupplierDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.endSuppNo));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startSuppNo, local_rec.endSuppNo) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endSuppName, sumr_rec.crd_name);
		DSP_FLD ("endSupplierDesc");
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("startCategoryNo") )
	{
		if (FLD ("startCategoryNo") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.startCategoryNo);

		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			if (cc) 
			{
				errmess (ML (mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.startCategoryNo, "%-11.11s", "            ");
			sprintf (excf_rec.cat_desc, "%-40.40s", ML("BEGINNING OF RANGE"));
		}

		if (prog_status != ENTRY && strcmp (local_rec.startCategoryNo, local_rec.endCategoryNo) > 0 )
		{
			sprintf (err_str, 
					ML (mlStdMess017),	
					local_rec.startCategoryNo,
					local_rec.endCategoryNo);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.startCategoryDesc,"%-40.40s",excf_rec.cat_desc);
		DSP_FLD ("startCategoryDesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("endCategoryNo") )
	{
		if (FLD ("endCategoryNo") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.endCategoryNo);
		
		if (!dflt_used)
		{
			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			if (cc) 
			{
				errmess (ML (mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.endCategoryNo, "%-11.11s", "~~~~~~~~~~~");
			sprintf (excf_rec.cat_desc, "%-40.40s", "END OF RANGE");
		}



		if (strcmp (local_rec.startCategoryNo, local_rec.endCategoryNo) > 0 )
		{
			sprintf (err_str, 
					ML (mlStdMess017), 	
					local_rec.startCategoryNo,
					local_rec.endCategoryNo);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}


		strcpy (local_rec.endCategoryDesc, excf_rec.cat_desc);
		DSP_FLD ("endCategoryDesc");
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("startClass"))
	{
		if (prog_status == EDIT && local_rec.startClass [0] > local_rec.endClass [0])
		{
			errmess (ML ("Start class greater than End class"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endClass"))
	{
		if (local_rec.endClass [0] < local_rec.startClass [0])
		{
			errmess (ML ("End class less than Start class"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("startPriorityNo"))
	{
		if (prog_status == EDIT && local_rec.startPriorityNo > local_rec.endPriorityNo)
		{
			errmess (ML ("Start priority greater than End priority"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endPriorityNo"))
	{
		if (local_rec.endPriorityNo < local_rec.startPriorityNo)
		{
			errmess (ML ("End priority less than Start priority"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Display Type |
	-----------------------*/
	if (LCHECK ("displayType") )
	{
		switch (local_rec.displayType [0])
		{
			case 'D' : strcpy (local_rec.displayDesc, "Delivery Performance");
					   break;
			case 'P' : strcpy (local_rec.displayDesc, "Purchase Volumes    ");
					   break;
			default  : return (EXIT_FAILURE);
		} /* End Switch */

		DSP_FLD ("displayDesc" );
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (restart ) 
		return (EXIT_SUCCESS);

	swide ();
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlPoMess054), (ScreenWidth - 44)/2,0,1);
	move (0,1);
	line (ScreenWidth);

	box (0,3,ScreenWidth,6);
	move (1,5);
	line (ScreenWidth - 1);

	move (1,8);
	line (ScreenWidth - 1);

	move (0,20);
	line (ScreenWidth);
	print_at (21,0,ML (mlStdMess038),
			    comm_rec.co_no,clip (comm_rec.co_name));
	print_at (21,50,ML (mlStdMess039),
			    comm_rec.est_no,clip (comm_rec.est_name));
	move (0,22);
	line (ScreenWidth);
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*===========================================
| Supplier Display.     	   				|
|											|
| Displays either Performance or Purchase 	|
| Volume data calculated for each supplier 	|
===========================================*/
void
SupplierDisplay (
 void)
{
	int		data_found = FALSE;
	char	ws_br_no [3];
	char	ws_po_no [16];
	char	ws_grn_no [16];
	char	ws_ship_no [13];
	long	ws_date_diffs = 0;
	long	ws_no_dels = 0;
	long	ws_rec_date = 0;
	long	ws_ord_date = 0;
	long	ws_due_date = 0;
	double	ws_value;
	int		cur_dmy [3];
	int		tmp_dmy [3];
	int		ws_tog = 1;


	if (local_rec.displayType [0] == 'D')	/* Delivery Performance */
		Dsp_open (13, 3, INTPSIZE); 

	else /* Purchase Volumes */
		Dsp_open (0, 3, INTPSIZE); 

	DisplayHeader ();

	DateToDMY (lsystemDate, &cur_dmy [0],&cur_dmy [1],&cur_dmy [2]);
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, local_rec.startSuppNo);

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	/*-----------------------------------
	| For each supplier on the database	|
	-----------------------------------*/
	while (!cc && 
		   !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
		   (!strcmp (sumr_rec.est_no, branchNumber) || envVarCrFind) &&
		    strcmp (sumr_rec.crd_no, local_rec.startSuppNo) >= 0 && 
		    strcmp (sumr_rec.crd_no, local_rec.endSuppNo) <= 0) 
	{
		if ((local_rec.selectionType [0] == 'P') &&
			 ((sumr_rec.sup_pri < local_rec.startPriorityNo) ||
			 (sumr_rec.sup_pri > local_rec.endPriorityNo)))
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  sumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}


		ws_date_diffs = 0;
		ws_no_dels = 0;
		branch_array [0].stk_mtd_total    = 0;
		branch_array [0].tot_mtd_total    = 0;
		branch_array [0].stk_last_12_mths = 0;
		branch_array [0].tot_last_12_mths = 0;
		branch_array [0].stk_prev_12_mths = 0;
		branch_array [0].tot_prev_12_mths = 0;

		suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
		suph_rec.ship_no = 0;
		strcpy (suph_rec.br_no, "  ");
		strcpy (suph_rec.csm_no, "            ");
		strcpy (suph_rec.po_no, fifteenSpaces);
		strcpy (suph_rec.grn_no,fifteenSpaces);
		suph_rec.rec_date = 0;
		suph_rec.ord_date = 0;
		suph_rec.due_date = 0;
		cc = find_rec (suph, &suph_rec, GTEQ, "r");

		/*-------------------------------------------------------
		| For each suph record relating to the current supplier	|
		| collate data for each purchase order. 				|
		-------------------------------------------------------*/
		while (!cc &&
			    suph_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			if (IsCategoryOk (suph_rec.hhbr_hash) == FALSE)
			{
				cc = find_rec (suph, &suph_rec, NEXT, "r");
				continue;
			}

			data_found    = FALSE;
			strcpy (ws_ship_no, suph_rec.csm_no);
			strcpy (ws_br_no, 	suph_rec.br_no);
			strcpy (ws_po_no, 	suph_rec.po_no);
			strcpy (ws_grn_no, 	suph_rec.grn_no);
			ws_rec_date = suph_rec.rec_date;
			ws_ord_date = suph_rec.ord_date;
			ws_due_date = suph_rec.due_date;

			/*----------------------------------------------
			| Collate line items within a delivery      	|
			-----------------------------------------------*/
			while (!cc &&
				   (!strcmp (ws_ship_no, suph_rec.csm_no)) 	&&
				   (!strcmp (ws_br_no, suph_rec.br_no)) 	&&
				   (!strcmp (ws_po_no, suph_rec.po_no)) 	&&
				   (!strcmp (ws_grn_no, suph_rec.grn_no)) 	&&
			       (suph_rec.hhsu_hash == sumr_rec.hhsu_hash) &&
				   (ws_ord_date == suph_rec.ord_date) &&
				   (ws_due_date == suph_rec.due_date) &&
				   (ws_rec_date == suph_rec.rec_date))
			{
				if (local_rec.displayType [0] == 'D') /* Delivery Performance */
				{
					if ((data_found == FALSE) &&
					    (suph_rec.due_date != 0L) &&
						 (strcmp (suph_rec.po_no, fifteenSpaces) != 0))
					{
						ws_date_diffs += (suph_rec.rec_date - suph_rec.due_date);
						ws_no_dels++;
						data_found = TRUE;
					}
				}
				else /* Purchase Volumes */
				{
					if ((suph_rec.due_date != 0L) &&
						 (strcmp (suph_rec.po_no, fifteenSpaces) != 0))
					{
						DateToDMY
						 (
							suph_rec.rec_date, 
							&tmp_dmy [0],
							&tmp_dmy [1],
							&tmp_dmy [2]
						);
						ws_value = (suph_rec.net_cost * suph_rec.rec_qty)
													  / pocr_rec.ex1_factor;
	
						if ((cur_dmy [1] == tmp_dmy [1]) && 
							 (cur_dmy [2] == tmp_dmy [2]))
						{
							if (STOCK_ORDER)
								branch_array [0].stk_mtd_total += ws_value;
							branch_array [0].tot_mtd_total += ws_value;
						}
			
						if (suph_rec.rec_date > (lsystemDate - 365))
						{
							if (STOCK_ORDER)
								branch_array [0].stk_last_12_mths += ws_value;
							branch_array [0].tot_last_12_mths += ws_value;
							ws_no_dels++;
						}
						else
						if ((suph_rec.rec_date < (lsystemDate - 365)) &&
							 (suph_rec.rec_date > (lsystemDate - 730)))
						{
							if (STOCK_ORDER)
								branch_array [0].stk_prev_12_mths += ws_value;
							branch_array [0].tot_prev_12_mths += ws_value;
							ws_no_dels++;
						}
					}
				}
				cc = find_rec (suph, &suph_rec, NEXT, "r");
			}
		}
		if (ws_no_dels > 0)
		{
			DisplayLine (ws_date_diffs, ws_no_dels);
			/* Display Loading message */
			/*rv_pr (" LOADING... ", (ScreenWidth -12)/2, 11, ws_tog);*/
#ifdef GVISION
			Dsp_message (ML (mlPoMess055), ws_tog);
#else
			rv_pr (ML (mlPoMess055), (ScreenWidth -12)/2, 11, ws_tog);
#endif /*GVISION*/
			ws_tog = (ws_tog ^ 1);
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	Dsp_srch_fn (DisplayDetail);
	Dsp_close ();
}

void
DisplayHeader (
 void)
{
	if (local_rec.displayType [0] == 'D')	/* Delivery Performance */
	{
   		Dsp_saverec ("  SUPPLIER  |                                                           |  NUMBER OF     |    AVERAGE    ");
	    Dsp_saverec ("  NUMBER    |   SUPPLIER NAME                                           |  DELIVERIES    |   DEVIATION   ");
		Dsp_saverec (" [REDRAW] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");

	}
	else /* Purchase Volumes */
	{
	    Dsp_saverec ("    SUPPLIER  |   SUPPLIER  |                STOCK PURCHASES                    |                   TOTAL PURCHASES               ");
	    Dsp_saverec ("    NUMBER    |   ACRONYM   |      MTD      | LAST 12 MONTHS  |  PREV 12 MONTHS |       MTD     | LAST 12 MONTHS | PREV 12 MONTHS ");
		Dsp_saverec (" [REDRAW] [NEXT SCREEN] [PREV SCREEN] [INPUT / END] ");
	}
}

void
DisplayLine (
 long ws_date_diffs, 
 long ws_no_dels)
{
	double	ws_deviation	=	0.00;

	if (local_rec.displayType [0] == 'D')	/* Delivery Performance */
	{
		ws_deviation = (double) ws_date_diffs / ws_no_dels;
		sprintf (err_str, "   %6.6s   ^E %-40.40s                  ^E %10ld     ^E             %9.2f",
				sumr_rec.crd_no,
				sumr_rec.crd_name,
				ws_no_dels,
				ws_deviation);
		Dsp_save_fn (err_str, sumr_rec.crd_no);
	}
	else /* Purchase Volumes */
	{
		sprintf (err_str,
			"     %6.6s   ^E  %10.10s ^E  %12.2f ^E  %12.2f   ^E  %12.2f   ^E  %12.2f ^E  %12.2f   ^E  %12.2f   ",
				sumr_rec.crd_no,
				sumr_rec.acronym,
				branch_array [0].stk_mtd_total,
				branch_array [0].stk_last_12_mths,
				branch_array [0].stk_prev_12_mths,
				branch_array [0].tot_mtd_total,
				branch_array [0].tot_last_12_mths,
				branch_array [0].tot_prev_12_mths);
		Dsp_save_fn (err_str, sumr_rec.crd_no);
	}
}

int
DisplayDetail (
 char *ws_supplier)
{
	if (local_rec.displayType [0] == 'D')	/* Delivery Performance */
		PerformanceDetails (ws_supplier);
	else /* Purchase Volumes */
		VolumeDetails (ws_supplier);
    return (EXIT_SUCCESS);
}

/*=======================================
| Display supplier performance detail   |
=======================================*/
void
PerformanceDetails (
 char *ws_supplier)
{
	int		ws_col;
	double	ws_deviation;
	int		data_found = FALSE;
	char	ws_ship_no [13];
	char	ws_br_no [3];
	char	ws_po_no [16];
	char	ws_grn_no [16];
	long	ws_ord_date;
	long	ws_rec_date;
	long	ws_due_date;
	char	ws_date1 [11];
	char	ws_date2 [11];
	char	ws_date3 [11];
	char	ws_last_line [ScreenWidth];


	ws_supplier [6] = '\0';
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, ws_supplier);

	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		return;

	sprintf (ws_last_line, 
			 ML (mlStdMess063),		
			 sumr_rec.crd_no, 
			 sumr_rec.crd_name);
	ws_col = (int) ((ScreenWidth - strlen (ws_last_line)) / 2);
	rv_pr (ws_last_line, ws_col, 2, 1);

	Dsp_open (13, 3, INTPSIZE); 

    Dsp_saverec ("BR|  SHIPMENT  |PURCHASE ORDER | GOODS RECEIPT |    DATE    |     DATE     |     DATE      |      %      ");
    Dsp_saverec ("NO|   NUMBER   |    NUMBER     |     NUMBER    |   RAISED   |   PROMISED   |   DELIVERED   |  DEVIATION  ");
	Dsp_saverec (" [REDRAW] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");


	suph_rec.ship_no = 0;
	strcpy (suph_rec.br_no, "  ");
	strcpy (suph_rec.po_no, fifteenSpaces);
	strcpy (suph_rec.grn_no,fifteenSpaces);
	suph_rec.ord_date = 0;
	suph_rec.rec_date = 0;
	suph_rec.due_date = 0;
	suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (suph, &suph_rec, GTEQ, "r");

	while (!cc &&
		    suph_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		data_found    = FALSE;
		strcpy (ws_ship_no,	suph_rec.csm_no);
		strcpy (ws_br_no, suph_rec.br_no);
		strcpy (ws_po_no, suph_rec.po_no);
		strcpy (ws_grn_no, suph_rec.grn_no);
		ws_ord_date = suph_rec.ord_date;
		ws_rec_date = suph_rec.rec_date;
		ws_due_date = suph_rec.due_date;

		/*----------------------------------------------
		| Collate line items within a goods receipt 	|
		-----------------------------------------------*/

		while (!cc &&
			   (!strcmp (ws_ship_no, suph_rec.csm_no)) 		&&
			   (!strcmp (ws_br_no, suph_rec.br_no)) 		&&
			   (!strcmp (ws_po_no, suph_rec.po_no)) 		&&
			   (!strcmp (ws_grn_no, suph_rec.grn_no)) 		&&
			   (suph_rec.hhsu_hash == sumr_rec.hhsu_hash) 	&&
			   (ws_ord_date == suph_rec.ord_date) &&
			   (ws_rec_date == suph_rec.rec_date) &&
			   (ws_due_date == suph_rec.due_date))
		{
			if ((suph_rec.due_date != 0L) &&
				 (strcmp (suph_rec.po_no, fifteenSpaces) != 0) &&
				 (IsCategoryOk (suph_rec.hhbr_hash) == TRUE))
			{
				if (data_found == FALSE)
				{
					data_found = TRUE;
					strcpy (ws_date1, DateToString (suph_rec.ord_date));
					strcpy (ws_date2, DateToString (suph_rec.due_date));
					strcpy (ws_date3, DateToString (suph_rec.rec_date));
					if (suph_rec.rec_date == suph_rec.due_date)
						ws_deviation = 0;
					else
					if (suph_rec.ord_date == suph_rec.due_date)
					{
						ws_deviation = (suph_rec.rec_date - suph_rec.due_date);
					 	ws_deviation = ws_deviation *  100;
					}
					else
					{
						ws_deviation = (suph_rec.rec_date - suph_rec.due_date);
						ws_deviation = ws_deviation 
									 /  (suph_rec.due_date - suph_rec.ord_date);
					 	ws_deviation = ws_deviation *  100;
					}
		
					sprintf (err_str,
						  "%2.2s^E%12.12s^E%15.15s^E%15.15s^E %10.10s ^E  %10.10s  ^E  %10.10s   ^E %6.2f",
							suph_rec.br_no,
							suph_rec.csm_no,
							suph_rec.po_no,
							suph_rec.grn_no,
							ws_date1,
							ws_date2,
							ws_date3,
							ws_deviation);
					Dsp_saverec (err_str);
				}
			}
			cc = find_rec (suph, &suph_rec, NEXT, "r");
			ws_deviation = 0;
		}
	}
	Dsp_srch ();
	Dsp_close ();

	sprintf (ws_last_line, "            %6.6s %40.40s ", "      ", "                                        ");
	ws_col = (int) ((ScreenWidth - strlen (ws_last_line)) / 2);
	rv_pr (ws_last_line, ws_col, 2, 0);
}

/*=======================================
| Display supplier volumes detail
=======================================*/
void
VolumeDetails (
 char *ws_supplier)
{
	int		workIndex = -1;
	int		ws_br_max = 0;
	int		ws_col;
	double	ws_value;
	int		tmp_dmy [3];
	int		i;
	int		cur_dmy [3];
	char	ws_last_line [ScreenWidth];


	ws_supplier [6] = '\0';
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, ws_supplier);

	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
		return;

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code,  sumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
	if (cc)
		return;

	sprintf (ws_last_line, ML (mlStdMess063), sumr_rec.crd_no, sumr_rec.crd_name);
	ws_col = (int) ((ScreenWidth - strlen (ws_last_line)) / 2);
	rv_pr (ws_last_line, ws_col, 2, 1);

	Dsp_open (0, 3, INTPSIZE); 
	Dsp_saverec ("   BRANCH   |  BRANCH            |             STOCK PURCHASES                    |                TOTAL PURCHASES                ");
	Dsp_saverec ("   NUMBER   | SHORT NAME         |   MTD      | LAST 12 MONTHS  |  PREV 12 MONTHS |    MTD     | LAST 12 MONTHS | PREV 12 MONTHS  ");
	Dsp_saverec (" [REDRAW] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");


	DateToDMY
	 (
		lsystemDate,
		&cur_dmy [0],
		&cur_dmy [1],
		&cur_dmy [2]
	);

	suph_rec.ship_no = 0;
	strcpy (suph_rec.po_no,  fifteenSpaces);
	strcpy (suph_rec.grn_no, fifteenSpaces);
	suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
	cc = find_rec (suph, &suph_rec, GTEQ, "r");

	while (!cc &&
		    suph_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if ((suph_rec.due_date != 0L) &&
			 (strcmp (suph_rec.po_no, fifteenSpaces) != 0) &&
			 (IsCategoryOk (suph_rec.hhbr_hash) == TRUE))
		{
			workIndex = FindBranchIndex (suph_rec.br_no, ws_br_max);
			if (workIndex == ws_br_max)
			{
				strcpy (esmr_rec.co_no, comm_rec.co_no);
				strcpy (esmr_rec.est_no, suph_rec.br_no);
				cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (suph, &suph_rec, NEXT, "r");
					continue;
				}
	
				strcpy (branch_array [workIndex].br_no, suph_rec.br_no);
				strcpy (branch_array [workIndex].br_short_name, esmr_rec.short_name);
				branch_array [workIndex].stk_mtd_total    = 0;
				branch_array [workIndex].tot_mtd_total    = 0;
				branch_array [workIndex].stk_last_12_mths = 0;
				branch_array [workIndex].tot_last_12_mths = 0;
				branch_array [workIndex].stk_prev_12_mths = 0;
				branch_array [workIndex].tot_prev_12_mths = 0;
				ws_br_max ++;
			}	
	
			ws_value = (suph_rec.net_cost * suph_rec.rec_qty) / pocr_rec.ex1_factor;

			DateToDMY
			 (
				suph_rec.rec_date,
				&tmp_dmy [0],
				&tmp_dmy [1],
				&tmp_dmy [2]
			);
	
			if ((cur_dmy [1] == tmp_dmy [1]) && (cur_dmy [2] == tmp_dmy [2]))
			{
				if (STOCK_ORDER)
					branch_array [workIndex].stk_mtd_total += ws_value;
				branch_array [workIndex].tot_mtd_total += ws_value;
			}
		
			if (suph_rec.rec_date > (lsystemDate - 365))
			{
				if (STOCK_ORDER)
					branch_array [workIndex].stk_last_12_mths += ws_value;
				branch_array [workIndex].tot_last_12_mths += ws_value;
			}
			else
			if ((suph_rec.rec_date <= (lsystemDate - 365)) &&
				 (suph_rec.rec_date >  (lsystemDate - 730)))
			{
				if (STOCK_ORDER)
					branch_array [workIndex].stk_prev_12_mths += ws_value;
				branch_array [workIndex].tot_prev_12_mths += ws_value;
			}
		}
		cc = find_rec (suph, &suph_rec, NEXT, "r");
	}
	
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	for (workIndex = 0; workIndex < ws_br_max; workIndex++)
	{
		sprintf (err_str,
				"     %2.2s     ^E %15.15s    ^E %10.2f ^E  %12.2f   ^E  %12.2f   ^E %10.2f ^E  %12.2f   ^E  %12.2f   \n",
				branch_array [workIndex].br_no,
				branch_array [workIndex].br_short_name,
				branch_array [workIndex].stk_mtd_total,
				branch_array [workIndex].stk_last_12_mths,
				branch_array [workIndex].stk_prev_12_mths,
				branch_array [workIndex].tot_mtd_total,
				branch_array [workIndex].tot_last_12_mths,
				branch_array [workIndex].tot_prev_12_mths);
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		strcpy (sortRec [sortCnt].sortCode, branch_array [workIndex].br_no);
		strcpy (sortRec [sortCnt].sortStr, err_str);
		/*
		 * Increment array counter.
		 */
		sortCnt++;
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
		Dsp_saverec (sortRec [i].sortStr);
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	Dsp_srch ();
	Dsp_close ();
	sprintf (ws_last_line, "            %6.6s %40.40s ", "      ", "                                        ");
	ws_col = (int) ((ScreenWidth - strlen (ws_last_line)) / 2);
	rv_pr (ws_last_line, ws_col, 2, 0);
}

int
IsCategoryOk (
 long ws_hhbr_hash)
{
	if (local_rec.selectionType [0] == 'C')
	{
		inmr_rec.hhbr_hash = ws_hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			return (FALSE);

		if ( (strcmp (inmr_rec.inmr_class, local_rec.startClass) < 0) ||
 			 (strcmp (inmr_rec.inmr_class, local_rec.endClass) > 0) ||
			 (strcmp (inmr_rec.category,   local_rec.startCategoryNo) < 0) ||
			 (strcmp (inmr_rec.category,   local_rec.endCategoryNo) > 0))
			return (FALSE);
		else
			return (TRUE);
	}
	else
		return (TRUE);
}

int
FindBranchIndex (
 char *ws_no, 
 int ws_max)
{
	int		workIndex = 0;

	while (workIndex < ws_max)
	{
		if (strcmp (branch_array [workIndex].br_no, ws_no) == 0)
			return (workIndex);
		workIndex++;
	}
	return (workIndex);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	_work_open (11,0,40);
	save_rec ("#Category No", "#Category Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) && 
		   !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "excf", "DB_FIND");
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
