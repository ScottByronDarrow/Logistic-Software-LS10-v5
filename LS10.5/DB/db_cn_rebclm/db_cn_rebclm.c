/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cn_rebclm.c,v 5.2 2002/07/17 09:57:05 scott Exp $
|  Program Name  : (db_cn_rebclm.c)
|  Program Desc  : (Print Debtos Contracts Rebate Claim)
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees | Date Written  : 10/11/93         |
|---------------------------------------------------------------------|
| $Log: db_cn_rebclm.c,v $
| Revision 5.2  2002/07/17 09:57:05  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/12/07 03:32:00  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cn_rebclm.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cn_rebclm/db_cn_rebclm.c,v 5.2 2002/07/17 09:57:05 scott Exp $";

#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<arralloc.h>

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cnreRecord	cnre_rec;
struct sumrRecord	sumr_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	char	*data	= "data";
		
	FILE	*fout;

	struct TOTALS {
		double	qtySold;
		double	rebate;
	};

	struct TOTALS supplierTotal; /* supplier totals */
	struct TOTALS contractTotal; /* contract totals */

	/*
	 *	Structure for dynamic array,  for the contRec lines for qsort
	 */
	struct ContStruct
	{
		char	contractNo 	[7];
		long	hhchHash;
		char	custNo 		[7];
		char	custName 	[41];
		char	itemNo 		[17];
		char	itemDesc	[41];
		char	invoiceNo	[9];
		char	invoieDate	[11];
		float	qtySold;
		float	standardCost;
		float	supplierCost;
	}	*contRec;
		DArray cont_details;
		int	contCnt = 0;

	char 	custNo 		[7];
	char	custName 	[21];
	char	itemNo 		[17];
	char	itemDesc 	[41];
	char	invNo 		[9];
	char	invDate 	[11];
	float	qtySold		=	0.00;
	double	stdCost		=	0.00;
	double	supCost		=	0.00;

	char	branchNumber [3];
	int		firstTime;

	extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	startSupplierNo [7];
	char	startSupplierName [41];
	char	endSupplierNo [7];
	char	endSupplierName [41];
	int		printerNo;
	char 	back [2];
	char	backDesc [4];
	char	onight [2];
	char	onightDesc [4];
	char	dummy [11];
	char	systemDate [11];
	long	today;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_sup",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier               ", "Enter Start Supplier Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplierNo},
	{1, LIN, "startSupplierName",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Start Supplier Name          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startSupplierName},
	{1, LIN, "ed_sup",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier                 ", "Enter End Supplier Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplierNo},
	{1, LIN, "endSupplierName",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "End Supplier Name            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endSupplierName},
	{1, LIN, "printerNo",	9, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number               ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	10, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background                   ", "Enter Yes or No - Default : No",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	10, 33, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight                    ", "Enter Yes or No - Default : No",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	11, 33, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include 	<FindSumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	RunProgram 			(char *);
void 	InitOutput 			(void);
void 	ProcReport 			(void);
void 	PrintTitle 			(void);
int 	ValidSupplier 		(void);
void 	PrintDetails 		(void);
void 	PrintSupTotal 		(void);
void 	PrintSupHeading 	(void);
void 	PrintConTotal 		(void);
void 	PrintConHeading 	(void);
void 	PrintCusHeading 	(void);
void 	PrintLine 			(void);
int		ContSort			(const	void *,	const void *);
int 	heading 			(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int		argc,
	char	*argv [])
{
	int		envCrCo = atoi (get_env ("CR_CO"));

	TruePosition	=	TRUE;

	if (argc != 1 && argc != 4)
	{
		print_at (0,0,mlDbMess707, argv [0]);
        return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.today = TodaysDate ();

	OpenDB ();

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	SETUP_SCR (vars);

	if (argc == 4)
	{
		strcpy (local_rec.startSupplierNo, argv [1]);
		strcpy (local_rec.endSupplierNo, argv [2]);
		local_rec.printerNo = atoi (argv [3]);

		dsp_screen ("Processing : Printing Customers Contract Rebate Claim Report",
					comm_rec.co_no, comm_rec.co_name);

		if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		/*-----------------------------------------
		| Step 1 : Read supplier record.          |
		| Step 2 : Sort all cnre records found    |
		|          for the current supplier.      |
		|          Sort records in order of       |
		|          contract, customer, and        |
		|          item.                          |
		| Step 3 : Print details to report.       |
		| Step 4 : Calculate totals and print     |
		|          totals on change of customer,  |
		|          change of contract, and change |
		|          of supplier (new page after    |
		|          supplier).                     |
		-----------------------------------------*/
		firstTime = TRUE;

		InitOutput ();
		ProcReport ();

		fprintf (fout, ".EOF\n");
		pclose (fout);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/

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

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_hhch_hash");
	open_rec (cnre, cnre_list, CNRE_NO_FIELDS, "cnre_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cnch);
	abc_fclose (cnre);
	abc_fclose (sumr);
	abc_fclose (cumr);
	abc_fclose (inmr);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("st_sup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startSupplierName, ML ("First Supplier"));
			DSP_FLD ("startSupplierName");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
			strcmp (pad_num (local_rec.startSupplierNo), 
			pad_num (local_rec.endSupplierNo)) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.startSupplierNo));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startSupplierName, sumr_rec.crd_name);
		DSP_FLD ("startSupplierName");

		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("ed_sup"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endSupplierName, ML ("End Supplier"));
			DSP_FLD ("endSupplierName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (pad_num (local_rec.startSupplierNo), 
			pad_num (local_rec.endSupplierNo)) > 0)
		{
			sprintf (err_str, 
				   ML ("Start Sup %s Must Not Be GREATER THAN End Sup %s"),
				   pad_num (local_rec.startSupplierNo), 
				   pad_num (local_rec.endSupplierNo));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.endSupplierNo));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endSupplierName, sumr_rec.crd_name);
		DSP_FLD ("endSupplierName");

		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Print Number |
	-----------------------*/
	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);

			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
				(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, 
				(local_rec.onight [0] == 'Y') ?  ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
RunProgram (
 char*              programName)
{
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		sprintf
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"", 
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.printerNo,
			ML (mlDbMess206)
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%d\"", 
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.printerNo
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*==================================
| Start Output To Standard Print . |
==================================*/
void
InitOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L135\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E CUSTOMER CONTRACT REBATE CLAIM REPORT\n");
	fprintf (fout, 
		".E COMPANY   : %s - %s\n", 
		comm_rec.co_no, 
		clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT %s\n", SystemTime ());
	fprintf (fout, 
		".E START SUPPLIER : %s END SUPPLIER : %s\n",
		pad_num (local_rec.startSupplierNo),
		pad_num (local_rec.endSupplierNo));
	fprintf (fout, ".B1\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "==============\n");

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "==============\n");
	fflush (fout);
}

void
ProcReport (void)
{
	PrintTitle ();

	/*-----------------------------------------
	| Initialis all totals structures.        |
	-----------------------------------------*/
	supplierTotal.qtySold = supplierTotal.rebate = 0.00;
	contractTotal.qtySold = contractTotal.rebate = 0.00;

	/*-----------------------------------------
	| Step 1 : Read supplier record.          |
	-----------------------------------------*/
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, pad_num (local_rec.startSupplierNo));
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && ValidSupplier ())
	{
		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&cont_details, &contRec,sizeof (struct ContStruct),50);
		contCnt = 0;

		dsp_process ("Supplier ", sumr_rec.crd_no);
		/*-----------------------------------------
		| Step 2 : Sort all cnre records found    |
		|          for the current supplier.      |
		|          Sort records in order of       |
		|          contract, customer, and        |
		|          item numbers.                  |
		| Read rebate file for the supplier.      |
		| Update status in cnre file to 'R' for   |
		| rebate.                                 |
		-----------------------------------------*/
		cnre_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (cnre, &cnre_rec, GTEQ, "u");
		while (!cc && cnre_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			if (cnre_rec.status [0] == 'R')
			{
				abc_unlock (cnre);
				cc = find_rec (cnre, &cnre_rec, NEXT, "u");
				continue;
			}

			cnch_rec.hhch_hash = cnre_rec.hhch_hash;
			cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (cnre);
				cc = find_rec (cnre, &cnre_rec, NEXT, "u");
				continue;
			}
			if (local_rec.today <= cnch_rec.date_exp)
			{
				abc_unlock (cnre);
				cc = find_rec (cnre, &cnre_rec, NEXT, "u");
				continue;
			}

			cumr_rec.hhcu_hash = cnre_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (cnre);
				cc = find_rec (cnre, &cnre_rec, NEXT, "u");
				continue;
			}

			inmr_rec.hhbr_hash = cnre_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (cnre);
				cc = find_rec (cnre, &cnre_rec, NEXT, "u");
					continue;
			}
			PrintSupHeading ();

			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&cont_details, contRec, contCnt))
				sys_err ("ArrChkLimit (contRec)", ENOMEM, PNAME);

			sprintf (contRec [contCnt].contractNo,	cnch_rec.cont_no);
			strcpy (contRec [contCnt].custNo,    	cumr_rec.dbt_no);
			strcpy (contRec [contCnt].custName,  	cumr_rec.dbt_name);
			strcpy (contRec [contCnt].itemNo,	 	inmr_rec.item_no);
			strcpy (contRec [contCnt].itemDesc,  	inmr_rec.description);
			strcpy (contRec [contCnt].invoiceNo, 	cnre_rec.inv_no);
			strcpy (contRec [contCnt].invoieDate,	DateToString (cnre_rec.inv_date));
			contRec [contCnt].hhchHash		=	cnch_rec.hhch_hash;
			contRec [contCnt].qtySold		=	cnre_rec.qty_sold;
			contRec [contCnt].standardCost	=	cnre_rec.std_cost;
			contRec [contCnt].supplierCost	=	cnre_rec.sup_cost;

			contCnt++;

			/*
			 * Update status to 'R' for rebate.
			 */
			strcpy (cnre_rec.status, "R");
			cc = abc_update (cnre, &cnre_rec);
			if (cc)
				file_err (cc, cnre, "DBUPDATE");

			cc = find_rec (cnre, &cnre_rec, NEXT, "u");
		}
		abc_unlock (cnre);

		if (contCnt)
		{
			/*
		 	* Sort the array in item description order.
		 	*/
			qsort (contRec, contCnt, sizeof (struct ContStruct), ContSort);
	
			PrintDetails ();
		}

		/*
		 *	Free up the array memory
		 */
		ArrDelete (&cont_details);

		if (contCnt)
			PrintSupTotal ();

		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
}

void
PrintTitle (void)
{
	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|      I T E M  D E S C R I P T I O N      ");
	fprintf (fout, "| INVOICE");
	fprintf (fout, "| INVOICE  ");
	fprintf (fout, "|  QUANTITY  ");
	fprintf (fout, "|  STANDARD  ");
	fprintf (fout, "|  SUPPLIER  ");
	fprintf (fout, "|   REBATE   |\n");

	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|   NO   ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|    SOLD    ");
	fprintf (fout, "|    COST    ");
	fprintf (fout, "|    COST    ");
	fprintf (fout, "|            |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------|\n");
}

int
ValidSupplier (void)
{
	if (strcmp (sumr_rec.co_no, comm_rec.co_no))
		return (FALSE);

	if (strcmp (sumr_rec.est_no, branchNumber))
		return (FALSE);

	if (strncmp (sumr_rec.crd_no, pad_num (local_rec.endSupplierNo), 6) > 0)
		return (FALSE);

	return (TRUE);
}

void
PrintDetails (void)
{
	int		i;
	long	hhch_hash = 0;

	long	prevHhchHash = 0;
	char	prevCust [7];

	strcpy (prevCust, " ");

	for (i = 0; i < contCnt; i++)
	{
		/*
		 * Print first contract and first customer.
		 */
		contRec [contCnt].hhchHash	=	cnch_rec.hhch_hash;
		prevHhchHash				=	contRec [i].hhchHash;
		cnch_rec.hhch_hash 			= 	prevHhchHash;
		cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cnch, "DBFIND");

		PrintConHeading ();
		strcpy (prevCust, 	contRec [i].custNo);
		strcpy (custNo, 	contRec [i].custNo);
		strcpy (custName, 	contRec [i].custName);

		PrintCusHeading ();

		hhch_hash	=	contRec [i].hhchHash;
		strcpy (custNo, 	contRec [i].custNo);
		strcpy (custName, 	contRec [i].custName);
		strcpy (itemNo, 	contRec [i].itemNo);
		strcpy (itemDesc,  contRec [i].itemDesc);
		strcpy (invNo, 	contRec [i].invoiceNo);
		strcpy (invDate, 	contRec [i].invoieDate);
		qtySold = contRec [i].qtySold;
		stdCost = contRec [i].standardCost;
		supCost = contRec [i].supplierCost;

		cnch_rec.hhch_hash = hhch_hash;
		if (prevHhchHash != cnch_rec.hhch_hash || strcmp (prevCust, custNo))
		{
			if (prevHhchHash != cnch_rec.hhch_hash)
			{
				PrintConTotal ();
				cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
				if (cc)
					file_err (cc, cnch, "DBFIND");
		
				prevHhchHash = cnch_rec.hhch_hash;
				PrintConHeading ();
			}
			strcpy (prevCust, custNo);
			PrintCusHeading ();
		}
		PrintLine ();
	}
	PrintConTotal ();
}

void
PrintSupTotal (void)
{
	fprintf (fout, "|------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "---------");
	fprintf (fout, "-----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------|\n");

	/*-----------------------------------------
	| Print supplier totals.                  |
	-----------------------------------------*/
	fprintf (fout, "| Supplier Total   ");
	fprintf (fout, ": %-40.40s ", " ");
	fprintf (fout, " %-8.8s", " ");
	fprintf (fout, " %-10.10s", " ");
	fprintf (fout, "| %10.2f ", supplierTotal.qtySold);
	fprintf (fout, "| %-10.10s ", " ");
	fprintf (fout, "  %-10.10s ", " ");
	fprintf (fout, "| %10.2f |\n", DOLLARS (supplierTotal.rebate));

	supplierTotal.qtySold =
		supplierTotal.rebate = 0.00;
}

void
PrintSupHeading (void)
{
	if (firstTime)
		firstTime = FALSE;
	else
		fprintf (fout, ".PA\n");

	fprintf (fout, "| Supplier No      : %-6.6s", sumr_rec.crd_no);
	fprintf (fout, 
		"%-35.35sSupplier Name       : %-40.40s%-9.9s |\n", 
		" ", 
		sumr_rec.crd_name,
		" ");
}

void
PrintConTotal (void)
{
	/*-----------------------------------------
	| Print contract totals.                  |
	-----------------------------------------*/
	fprintf (fout, "| Contract Total   ");
	fprintf (fout, ": %-40.40s ", " ");
	fprintf (fout, " %-8.8s", " ");
	fprintf (fout, " %-10.10s", " ");
	fprintf (fout, "| %10.2f ", contractTotal.qtySold);
	fprintf (fout, "| %-10.10s ", " ");
	fprintf (fout, "  %-10.10s ", " ");
	fprintf (fout, "| %10.2f |\n", DOLLARS (contractTotal.rebate));

	contractTotal.qtySold = contractTotal.rebate = 0.00;
}

void
PrintConHeading (void)
{
	char	WorkDate [4][11];

	fprintf (fout, "|------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "---------");
	fprintf (fout, "-----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------|\n");

	strcpy (WorkDate [0], DateToString (cnch_rec.date_wef));
	strcpy (WorkDate [1], DateToString (cnch_rec.date_rev));
	strcpy (WorkDate [2], DateToString (cnch_rec.date_ren));
	strcpy (WorkDate [3], DateToString (cnch_rec.date_exp));
	fprintf (fout, "| Contract No    : %6.6s  -  %40.40s %60.60s   |\n",
			cnch_rec.cont_no, cnch_rec.desc, " ");

	fprintf (fout, "| Effective Date : %10.10s  /   Review Date :   %10.10s   /  Renewal Date :   %10.10s   /   Expiry Date :   %10.10s       |\n", WorkDate [0], WorkDate [1], WorkDate [2], WorkDate [3]);
}


void
PrintCusHeading (void)
{
	fprintf (fout, "|                  ");
	fprintf (fout, "                                           ");
	fprintf (fout, "         ");
	fprintf (fout, "           ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "             |\n");

	fprintf (fout, "| Customer No      : %-6.6s", custNo);
	fprintf (fout, "%-35.35sCustomer Name       : %-40.40s%-9.9s |\n", 
		" ", 
		custName,
		" ");

	fprintf (fout, "|------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "---------");
	fprintf (fout, "-----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------|\n");
}

void
PrintLine (void)
{
	double 	rebate			= 0.0,
			totalStdCost	= 0.0,
			totalSupCost	= 0.0;
	/*
	 * Step 4 : Calculate totals and print     
	 *          totals on change of customer,  
	 *          change of contract, and change 
	 *          of supplier (new page after supplier).                     
	 */
	totalStdCost = stdCost * qtySold;
	totalSupCost = supCost * qtySold;

	rebate = totalStdCost - totalSupCost;

	fprintf (fout, "| %-16.16s ", 	itemNo);
	fprintf (fout, "| %-40.40s ", 	itemDesc);
	fprintf (fout, "|%-8.8s", 		invNo);
	fprintf (fout, "|%-10.10s", 	invDate);
	fprintf (fout, "|%11.2f ", 		qtySold);
	fprintf (fout, "|%11.2f ", 		DOLLARS (stdCost));
	fprintf (fout, "|%11.2f ", 		DOLLARS (supCost));
	fprintf (fout, "|%11.2f |\n", 	DOLLARS (rebate));

	/*
	 * Calculate totals for supplier, contract and customer. 
	 */
	supplierTotal.qtySold 	+= (double) qtySold;
	supplierTotal.rebate 	+= rebate;
	contractTotal.qtySold 	+= (double) qtySold;
	contractTotal.rebate 	+= rebate;
}

int 
ContSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct ContStruct a = * (const struct ContStruct *) a1;
	const struct ContStruct b = * (const struct ContStruct *) b1;

	result = strcmp (a.contractNo, b.contractNo);

	return (result);
}

int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		
		rv_pr (ML (mlDbMess206), 20, 0, 1);

		box (0, 3, 80, 8);
		line_at (1,0,80);
		line_at (8,1,79);
		line_at (20,0,80);
		line_at (22,0,80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no, clip (comm_rec.co_name));

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

