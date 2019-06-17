/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_locprn.c,v 5.4 2002/07/17 09:57:38 scott Exp $
|  Program Name  : (po_locprn.c)
|  Program Desc  : (Print Purchase Orders by Location)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 05/09/88         |
|---------------------------------------------------------------------|
| $Log: po_locprn.c,v $
| Revision 5.4  2002/07/17 09:57:38  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/05/09 05:58:04  scott
| Updated to add memory sort routines and remove old disk based sort.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_locprn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_locprn/po_locprn.c,v 5.4 2002/07/17 09:57:38 scott Exp $";

#define MAXWIDTH	132
#define MAXPO		20

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <arralloc.h>

	/*
	 * Special fields and flags.
	 */
	int		envVarCrCo = 0;
	int		envVarCrFind = 0;

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct insfRecord	insf_rec;

	int		shipment 		= 0,
			foundData 		= FALSE,
			purchaseCount 	= 0;

	char	branchNumber [3],
			sortBy [2],
			pOrderTable [MAXPO] [16],
			*fifteenSpaces	=	"               ";

	FILE	*fout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [42];
	char	purchaseNo	[sizeof pohr_rec.pur_ord_no];
	char	locationNo	[sizeof incc_rec.location]; 
	char	itemNo		[sizeof inmr_rec.item_no];
	char	itemDesc	[sizeof inmr_rec.description];
	char	itemUom		[sizeof inum_rec.uom];
	float	quantity;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;


/*
 * Local & Screen Structures 
 */
struct {						/*---------------------------------------*/
	char	dummy [11];			/*| Dummy Used In Screen Generator.		|*/
	char	prevPo [16];		/*| Previous Purchase Order Number.		|*/
	char	prevSupplier [7];	/*| Previous Customer Number.			|*/
	char	systemDate [11];	/*| System Date.						|*/
	char	supplierNo [7];		/*| Supplier Number. 		   	     	|*/
	char	description [41];	/*| Holds part number.                  |*/
	double	cost;				/*| Landed Cost.						|*/
	long	shipmentNumber;    	/*| Shipment number.                    |*/
	int		printerNumber;
} local_rec;

struct {                    /*-------------------------------------------*/
	long	hhpo_hash;      /*| Purchase order header hash.             |*/
	long	hhbr_hash;      /*| Inventory master file hash.             |*/
	long	hhcc_hash;      /*| Warehouse master file hash.             |*/
	long	hhwh_hash;      /*| Warehouse serial hash.                  |*/
	int	line_no;        	/*| Line number for item read in            |*/
} sr_rec [MAXLINES];
		
static	struct	var	vars []	={	

	{1, LIN, "shipmentNumber", 4, 20, CHARTYPE, 
		"AAAAAAAAAAAA", "          ", 
		" ", "0", "Shipment No.", " ", 
		NE, NO, JUSTLEFT, "", "", posh_rec.csm_no}, 
	{1, LIN, "vessel", 5, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Vessel ", " ", 
		NA, NO, JUSTLEFT, "", "", posh_rec.vessel}, 
	{1, LIN, "supplierNo", 6, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier No.", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.supplierNo}, 
	{1, LIN, "supplierName", 7, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Suppier Name", " ", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "pOrderNo", 8, 20, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "Purchase Order No.", " ", 
		NE, NO, JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{1, LIN, "printerNumber", 10, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No.", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};
  
float	StdCnvFct 	= 1.00;

#include <FindSumr.h>

/*
 * Function Declarations 
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchPosh 		 	(char *);
void 	SrchPohr 			(char *);
void 	PrintCoDetails 		(void);
void 	ProcessReport 		(void);
void 	HeadingOutput 		(void);
void 	ProcessFile 		(void);
void 	FindPoln 			(void);
void 	StoreData 			(float);
void 	ProcessSortedList 	(void);
int 	ProcessPosd 		(void);
int 	heading 			(int);
int 	spec_valid 			(int);
int		SortFunc			(const	void *,	const void *);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2)
	{
		print_at (0,0,"Usage : %s [L(oc I(tem No P(urchase Order No",argv [0]);
        return (EXIT_SUCCESS);
	}

	if (argv [1][0] != 'L' && argv [1][0] != 'I' && argv [1][0] != 'P')
	{
		print_at (0,0,"Sort Report by Location OR Item No OR Purchase Order No\007\n\r");
        return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	sprintf (sortBy,"%-1.1s",argv [1]);

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	/*
	 * setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);	

	/*
	 * open main database files. 
	 */
	OpenDB ();

	strcpy (branchNumber, (!envVarCrCo) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	strcpy (local_rec.prevPo,	"000000000000000");
	strcpy (local_rec.prevSupplier,	"000000");

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags.
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		eoi_ok 		= TRUE;
		init_vars (1);	

		FLD ("pOrderNo")	= YES;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		ProcessReport ();

	}	/* end of input control loop	*/
	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files . 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
															   : "sumr_id_no3");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*
 * Close Database Files 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inum);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*
	 * Validate creditor Number.  
	 */
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supplierNo));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("supplierName");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Shipping Number.  
	 */
	if (LCHECK ("shipmentNumber"))
	{
		abc_selfield ("pohr", "pohr_hhpo_hash");
		shipment = FALSE;

		if (dflt_used)
		{
			sprintf (posh_rec.vessel,"%-20.20s"," ");
			DSP_FLD ("vessel");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (posh_rec.co_no,comm_rec.co_no);
		cc = find_rec (posh,&posh_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.supplierNo,	"%-6.6s"," ");
		sprintf (sumr_rec.crd_name,		"%-40.40s"," ");
		sprintf (pohr_rec.pur_ord_no,	"%-15.15s"," ");

		DSP_FLD ("vessel");
		DSP_FLD ("supplierNo");
		DSP_FLD ("supplierName");
		DSP_FLD ("pOrderNo");
		skip_entry = 4;

		shipment = TRUE;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Purchase Order Number. 
	 */
	if (LCHECK ("pOrderNo"))
	{
		abc_selfield ("pohr", "pohr_id_no");

		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		/*
		 * Check if order is on file. 
		 */
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type,"O");
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (cc || pohr_rec.status [0] == 'D')
		{
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (pohr_rec.hhsh_hash != 0L)
		{
			sprintf (err_str,ML (mlPoMess140),pohr_rec.hhsh_hash);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		shipment = FALSE;
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}

/*
 * Search for shipment number. 
 */
void
SrchPosh (
	char	*key_val)
{
	work_open ();
	save_rec ("#Shipment No.","#Ship Name");
	strcpy (posh_rec.co_no,comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", " ");
	cc = find_rec (posh,&posh_rec,GTEQ,"r");
	while (!cc && !strcmp (posh_rec.co_no,comm_rec.co_no))
	{
		if (posh_rec.status [0] == 'I' || posh_rec.status [0] == 'R')
		{
			cc = save_rec (posh_rec.csm_no,posh_rec.vessel);
			if (cc)
				break;
		}
		cc = find_rec (posh,&posh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (posh_rec.co_no,comm_rec.co_no);
	strcpy (posh_rec.csm_no, temp_str);
	cc = find_rec (posh,&posh_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in posh During (DBFIND)",cc,PNAME);
}

/*
 * Search for purchase order number. 
 */
void
SrchPohr (
	char *key_val)
{
	work_open ();

	save_rec ("#Purchase Order ","#Date Raised");
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type,"O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");

	while (!cc && !strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)) && 
				  !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
				  !strcmp (pohr_rec.br_no,comm_rec.est_no) && 
				  pohr_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if (pohr_rec.status [0] == 'O' || pohr_rec.status [0] == 'R')
		{
			sprintf 
			(
				err_str,
				"Date Raised: %s  %s",
				DateToString (pohr_rec.date_raised),
				pohr_rec.term_order
			);
			cc = save_rec (pohr_rec.pur_ord_no,err_str);
			if (cc)
				break;
		}
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type,"O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}

/*
 * Print Company Details. 
 */
void
PrintCoDetails (void)
{
	line_at (19,0,80);
	print_at (20,0,ML (mlStdMess038),	comm_rec.co_no, comm_rec.co_name);
	print_at (21,0,ML (mlStdMess039),	comm_rec.est_no,comm_rec.est_name);
	print_at (22,0,ML (mlStdMess099),	comm_rec.cc_no, comm_rec.cc_name);
}


void
ProcessReport (void)
{
	dsp_screen ("Processing : Printing Purchase Receipt Report.",comm_rec.co_no,comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	ProcessFile ();
	if (foundData)
	{
		ProcessSortedList ();
		fprintf (fout,".EOF\n");
	}
	pclose (fout);
	strcpy (local_rec.prevPo, pohr_rec.pur_ord_no);
	strcpy (local_rec.prevSupplier,local_rec.supplierNo);
}

/*
 * Start Out Put To Standard Print 
 */
void
HeadingOutput (void)
{
	int	i;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);

	fprintf (fout,".15\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L120\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EPURCHASE ORDER RECEIPT REPORT\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".CAS AT %s\n",SystemTime ());
	fprintf (fout,".CFOR %s %s\n",comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".CBranch %s - %s : WH %s - %s\n",
				comm_rec.est_no,clip (comm_rec.est_name),
				comm_rec.cc_no,clip (comm_rec.cc_name));

	fprintf (fout,".B1\n");

	fprintf (fout,"Shipment No : %s - / Purchase Order No (s) ",posh_rec.csm_no);
	for (i = 0; i < purchaseCount ; i++)
	{
		fprintf (fout,"%s ",pOrderTable [i]);
		if (i == purchaseCount - 1)
			fprintf (fout,"\n");
	}

	fprintf (fout,"Supplier Code : %s - %s \n",local_rec.supplierNo,sumr_rec.crd_name);

	fprintf (fout,".R===================================");
	fprintf (fout,"==========================================");
	fprintf (fout,"==========================================\n");

	fprintf (fout,"===================================");
	fprintf (fout,"==========================================");
	fprintf (fout,"==========================================\n");

	fprintf (fout,"|  ITEM NUMBER     ");
	fprintf (fout,"|  ITEM DESCRIPTION                        ");
	fprintf (fout,"|  LOCATION    ");
	fprintf (fout,"| UOM. ");
	fprintf (fout,"| QTY OUTSTANDING ");
	fprintf (fout,"|  P.O. NUMBER  |\n");

	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|------");
	fprintf (fout,"|-----------------");
	fprintf (fout,"|---------------|\n");

	fflush (fout);
}

/*
 * If shipmt_no entered,than read posh,posd,pohr,poln	
 * If po_no entered,read pohr,poln only.     		   
 */
void
ProcessFile (void)
{
	if (shipment)
	{
		strcpy (posh_rec.co_no,comm_rec.co_no);
		cc = find_rec (posh,&posh_rec,COMPARISON,"r");
		if (!cc)
			ProcessPosd ();
	}
	else
	{
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type,"O");
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (!cc)
			FindPoln ();
	}
}

int
ProcessPosd (void)
{
	int	i;

	for (i = 0; i < purchaseCount; i++)
		strcpy (pOrderTable [i],fifteenSpaces);

	i  = 0;

	strcpy (posd_rec.co_no,comm_rec.co_no);
	posd_rec.hhsh_hash = posh_rec.hhsh_hash;
	posd_rec.hhpo_hash = 0L;
	cc = find_rec (posd,&posd_rec,GTEQ,"r");
	while (!cc && !strcmp (posd_rec.co_no,comm_rec.co_no) && 
				   posd_rec.hhsh_hash == posh_rec.hhsh_hash)
	{
		pohr_rec.hhpo_hash = posd_rec.hhpo_hash;
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (!cc)
		{
			strcpy (pOrderTable [i++],pohr_rec.pur_ord_no);
			purchaseCount = i;
			FindPoln ();
		}
		cc = find_rec (posd,&posd_rec,NEXT,"r");
	}
	return (EXIT_SUCCESS);
}

void
FindPoln (void)
{
	float	qty_out = 0.00;

	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = 0;
	cc = find_rec (poln,&poln_rec,GTEQ,"r");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		qty_out = poln_rec.qty_ord - poln_rec.qty_rec;

		incc_rec.hhbr_hash = poln_rec.hhbr_hash;
		incc_rec.hhcc_hash = poln_rec.hhcc_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");

		if (!cc && qty_out > 0.00)
			StoreData (qty_out);

		cc = find_rec (poln,&poln_rec,NEXT,"r");
	}
}

void
StoreData (
	float	qty_out)
{
	float	PurCnvFct 	= 1.00;
	float	CnvFct		= 1.00;

	inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	foundData = TRUE;

	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	switch (sortBy [0])
	{
	case	'L':
		sprintf 
		(
			sortRec [sortCnt].sortCode,
			"%-10.10s%-16.16s%-15.15s",
			incc_rec.location,
			inmr_rec.item_no,
			pohr_rec.pur_ord_no
		);
		break;

	case	'I':
		sprintf 
		(
			sortRec [sortCnt].sortCode,
			"%-16.16s%-10.10s%-15.15s",
			inmr_rec.item_no,
			incc_rec.location,
			pohr_rec.pur_ord_no
		);
		break;

	case	'P':
		sprintf 
		(
			sortRec [sortCnt].sortCode,
			"%-15.15s%-10.10s%-16.16s",
			pohr_rec.pur_ord_no,
			incc_rec.location,
			inmr_rec.item_no
		);
		break;
	}
	strcpy (sortRec [sortCnt].purchaseNo, 	pohr_rec.pur_ord_no);
	strcpy (sortRec [sortCnt].locationNo, 	incc_rec.location);
	strcpy (sortRec [sortCnt].itemNo,		inmr_rec.item_no);
	strcpy (sortRec [sortCnt].itemDesc,		inmr_rec.description);
	strcpy (sortRec [sortCnt].itemUom,		inum_rec.uom);
	sortRec [sortCnt].quantity	=		qty_out * CnvFct;

	/*
	 * Increment array counter.
	 */
	sortCnt++;
	return;
}

void
ProcessSortedList (void)
{
	int		i;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	HeadingOutput ();
	
	for (i = 0; i < sortCnt; i++)
	{
		dsp_process ("Transaction: ", sortRec [i].purchaseNo);

		fprintf (fout,"| %16.16s ",		sortRec [i].itemNo);
		fprintf (fout,"| %40.40s ",		sortRec [i].itemDesc);
		fprintf (fout,"|  %10.10s  ",	sortRec [i].locationNo);
		fprintf (fout,"| %4.4s ",		sortRec [i].itemUom);
		fprintf (fout,"|    %10.2f   ",	sortRec [i].quantity);
		fprintf (fout,"|%15.15s|\n",	sortRec [i].purchaseNo);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
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

/*
 * Print Heading. 
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlPoMess192),20,0,1);

		print_at (0,57,ML (mlPoMess043),local_rec.prevSupplier);

		box (0,3,80,7);
		line_at (1,0,80);
		line_at (9,1,79);

		PrintCoDetails ();
		
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
