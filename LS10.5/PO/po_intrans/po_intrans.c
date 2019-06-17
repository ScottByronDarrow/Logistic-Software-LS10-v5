/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_intrans.c,v 5.7 2002/12/01 04:48:14 scott Exp $
|  Program Name  : (po_intrans.c)                                     |
|  Program Desc  : (Print Pro Forma Cost of In-Transits)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written : 28/02/89          |
|---------------------------------------------------------------------|
| $Log: po_intrans.c,v $
| Revision 5.7  2002/12/01 04:48:14  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.6  2002/07/17 09:57:38  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2002/07/03 07:05:38  scott
| Updated to change disk based sorts to memory based sort
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_intrans.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_intrans/po_intrans.c,v 5.7 2002/12/01 04:48:14 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <arralloc.h>

#include	"schema"

struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct incfRecord	incf_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inprRecord	inpr_rec;
struct insfRecord	insf_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct poslRecord	posl_rec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [256];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

	int		taxInclusive 	= FALSE,
			envVarGst 		= 1;

	char	*data	= "data";

	float	taxRate = 0.00,
			gstPc 	= 0.00;

	char	envVarTaxName [4];
	char	envVarCurrCode [4];

	FILE	*pout;

struct
{
	char	dummy [11];
	int		printerNo;
	char	pOrderNumber [16];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "shipmentNo",	 4, 20, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", "", "Ship No.", " ",
		 NO, NO, JUSTRIGHT, "", "", posh_rec.csm_no},
	{1, LIN, "vessel",	 4, 55, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Vessel", " ",
		 NA, NO,  JUSTLEFT, "", "", posh_rec.vessel},
	{1, LIN, "poNumber",	 5, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "Purchase Order No.", " ",
		 NO, NO, JUSTRIGHT, "", "", local_rec.pOrderNumber},
	{1, LIN, "printerNo",		 6, 20, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

float	StdCnvFct 	= 0.00;
char	*fifteenSpaces	=	"               ";

/*
 * Function Declarations 
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchPosh 			(char *);
void 	SrchPohr 			(char *);
void 	RunProgram 			(char *);
void 	ReportHeading 		(long, char *, char *);
void 	ProcessShipment 	(long);
void 	ProcessPorder 		(char *);
void 	SaveDetailLine 		(int, int);
double 	GetRetail 			(void);
double	FindLastCost 		(long, char	*);
int 	spec_valid 			(int);
int 	heading 			(int);
int		SortFunc			(const	void *,	const void *);
char 	*GetShipmentNumber 	(long);

/*
 * Main Processing Routine 
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	char	*sptr;
	long	hhshHash;

	if (argc != 1 && argc != 3 && argc != 4)
	{
		print_at (0,0, mlPoMess725, argv [0]);
		print_at (1,0, mlPoMess726);
		print_at (2,0, mlPoMess727);
        return (EXIT_FAILURE);
	}

	/*
	 * Check if GST applies 
	 */
	sptr = chk_env ("GST");
	if (sptr == (char *) 0)
		envVarGst = 0;
	else
		envVarGst = (*sptr == 'Y' || *sptr == 'y');

	/*
	 * Get gst code.
	 */
	sprintf (envVarTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));

	/*
	 * Get local currency code. 
	 */
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	SETUP_SCR (vars);

	OpenDB ();

	if (argc != 1)
	{
		abc_selfield (posh, "posh_id_no");

		sptr = chk_env ("GST_INCLUSIVE");
		if (sptr == (char *) 0)
		{
			taxInclusive = FALSE;
			taxRate = comm_rec.gst_rate;
		}
		else
		{
			taxRate = (float) atof (sptr);
			if (taxRate == 0.00)
			{
				taxInclusive = FALSE;
				taxRate = comm_rec.gst_rate;
			}
			else
				taxInclusive = TRUE;
		}

		if (taxInclusive)
			gstPc = (float) (100.00 / (100.00 + taxRate));
		else
			gstPc = taxRate;

		local_rec.printerNo = atoi (argv [1]);

		if (argc == 3)
		{
			hhshHash = atol (argv [2]);
			ProcessShipment (hhshHash);
		}
		else
			ProcessPorder (argv [3]);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	init_scr ();
	set_tty ();
	set_masks ();

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
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

		RunProgram (argv [0]);
		return (EXIT_SUCCESS);
	}	/* end of input control loop	*/

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (incf, incf_list, INCF_NO_FIELDS, "incf_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_id_no");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

void
CloseDB (void)
{
	abc_fclose (incc);
	abc_fclose (incf);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inpr);
	abc_fclose (insf);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	abc_fclose (posl);
	abc_fclose (inum);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			errmess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("shipmentNo"))
	{
		if (SRCH_KEY)
		{
			SrchPosh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used || strlen (clip (posh_rec.csm_no)) == 0)
			return (EXIT_SUCCESS);

		strcpy (posh_rec.co_no, comm_rec.co_no);
		strcpy (posh_rec.csm_no, zero_pad (posh_rec.csm_no, 12));
		cc = find_rec (posh, &posh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess050));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		skip_entry = 2;

		DSP_FLD ("vessel");

		strcpy (local_rec.pOrderNumber, fifteenSpaces);
		DSP_FLD ("poNumber");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("poNumber"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || strlen (clip (local_rec.pOrderNumber)) == 0)
		{
			errmess (ML ("Please enter either shipment no OR P/O number"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pohr_rec.co_no, comm_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.pOrderNumber, 15));
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		posh_rec.hhsh_hash = 0L;
		strcpy (posh_rec.csm_no, " ");
		strcpy (posh_rec.vessel, "                    ");

		DSP_FLD ("shipmentNo");
		DSP_FLD ("vessel");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for shipment number. 
 */
void
SrchPosh (
	char *keyValue)
{
	_work_open (12,0,40);
	save_rec ("#Shipment No.", "#Ship Name");
	strcpy (posh_rec.co_no, comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", keyValue);
	cc = find_rec (posh, &posh_rec, GTEQ, "r");
	while (!cc && !strcmp (posh_rec.co_no, comm_rec.co_no))
	{
		if (!strncmp (posh_rec.csm_no, keyValue, strlen (keyValue)))
		{
			cc = save_rec (posh_rec.csm_no, posh_rec.vessel);
			if (cc)
				break;
		}
		cc = find_rec (posh, &posh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (posh_rec.co_no, comm_rec.co_no);
	sprintf (posh_rec.csm_no, "%-12.12s", temp_str);
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "posh", "DBFIND");
}

/*
 * Search for purchase order.	
 */
void
SrchPohr (
 char *keyValue)
{
	_work_open (15,0,10);
	save_rec ("#P/Order Number ", "#Date Raised");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		!strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		!strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue))
	)
	{
		strcpy (err_str, DateToString (pohr_rec.date_raised));
		cc = save_rec (pohr_rec.pur_ord_no, err_str);
		if (cc)
			break;
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pohr, "DBFIND");
}

void
RunProgram (
 char *programName)
{
	char	hhshString [11];

	shutdown_prog ();

	sprintf (hhshString, "%ld", posh_rec.hhsh_hash);

	if (posh_rec.hhsh_hash != 0L)
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"%ld\"",
			programName,
			local_rec.printerNo,
			posh_rec.hhsh_hash
		);
		SystemExec (err_str, FALSE);
	}
	else
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"P\" \"%s\"",
			programName,
			local_rec.printerNo,
			pohr_rec.pur_ord_no
		);
		SystemExec (err_str, FALSE);
	}
}

void
ReportHeading (
	long 	hhshHash, 
	char 	*shipmentNo, 
	char 	*poNumber)
{
	if ((pout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pout, ".LP%d\n", local_rec.printerNo);

	fprintf (pout, ".11\n");
	fprintf (pout, ".PI16\n");
	fprintf (pout, ".L162\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (pout, ".EPRO-FORMA COST OF IN-TRANSITS\n");
	if (hhshHash != 0L)
		fprintf (pout, ".EFOR SHIPMENT : %-12.12s\n", GetShipmentNumber (hhshHash));
	else
		fprintf (pout, ".EFOR PURCHASE ORDER : %s\n", poNumber);
	fprintf (pout, ".EAs At %-24.24s\n", SystemTime ());

	fprintf (pout, ".R=================");
	fprintf (pout, "=========================================");
	fprintf (pout, "=======");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "========");
	if (envVarGst)
		fprintf (pout, "===========");
	fprintf (pout, "============\n");

	fprintf (pout, "=================");
	fprintf (pout, "=========================================");
	fprintf (pout, "=======");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "========");
	if (envVarGst)
		fprintf (pout, "===========");
	fprintf (pout, "============\n");

	fprintf (pout, "|   ITEM NUMBER  ");
	fprintf (pout, "|   D E S C R I P T I O N                ");
	fprintf (pout, "| UOM. ");
	fprintf (pout, "|  QTY. ON ");
	fprintf (pout, "|        C U R R E N T        ");
	fprintf (pout, "|          ");
	fprintf (pout, "|  N E W   ");
	if (envVarGst)
		fprintf (pout, "|      S U G G E S T E D      |\n");
	else
		fprintf (pout, "| S U G G E S T E D|\n");

	fprintf (pout, "|                ");
	fprintf (pout, "|                                        ");
	fprintf (pout, "|      ");
	fprintf (pout, "|  ORDER   ");
	fprintf (pout, "|SELL PRICE");
	fprintf (pout, "| LAST COST");
	fprintf (pout, "| MARGN ");
	fprintf (pout, "| FOB COST ");
	fprintf (pout, "|   COST   ");
	fprintf (pout, "| MARGN ");
	if (envVarGst)
	{
		fprintf (pout, "| %-3.3s EXCL ", envVarTaxName);
		fprintf (pout, "| %-3.3s INCL |\n", envVarTaxName);
	}
	else
		fprintf (pout, "| NEW SELL |\n");

	fprintf (pout, "|----------------");
	fprintf (pout, "|----------------------------------------");
	fprintf (pout, "|------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|-------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|-------");
	if (envVarGst)
	{
		fprintf (pout, "|----------");
		fprintf (pout, "|----------|\n");
	}
	else
		fprintf (pout, "|----------|\n");
	fflush (pout);
}

void
ProcessShipment (
	long	hhshHash)
{
	int		i,
			size = 11;
	char	*sptr = chk_env ("DIS_FIND");

	if (sptr != (char *)0)
		size = atoi (sptr);

	strcpy (posh_rec.co_no, comm_rec.co_no);
	posh_rec.hhsh_hash = hhshHash;
	cc = find_rec (posh, &posh_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	dsp_screen (" Printing Pro-Forma Cost of In-Transits ", 
								comm_rec.co_no, comm_rec.co_name);

	ReportHeading (hhshHash, posh_rec.csm_no, "");

	strcpy (posl_rec.co_no, comm_rec.co_no);
	posl_rec.hhsh_hash = hhshHash;
	posl_rec.hhpl_hash = 0L;

	cc = find_rec (posl, &posl_rec, GTEQ, "r");
	while (!cc && !strcmp (posl_rec.co_no, comm_rec.co_no) && 
						   posl_rec.hhsh_hash == hhshHash)
	{
		if (posl_rec.ship_qty <= 0.00)
		{
			cc = find_rec (posl, &posl_rec, NEXT, "r");
			continue;
		}
		poln_rec.hhpl_hash	=	posl_rec.hhpl_hash;
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (!cc)
		{
			inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (!cc)
				SaveDetailLine (0, size);
		}
		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (err_str, "%-16.16s", sortRec [i].sortCode + 1 + size);
		dsp_process (" Read : ", err_str);

		fprintf (pout, "%s\n", sortRec [i].sortCode + 1 + size);
	}

	fprintf (pout, ".EOF\n");
	pclose (pout);

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
ProcessPorder (
	char	*poNumber)
{
	int		i,
			size = 11;
	char	*sptr = chk_env ("DIS_FIND");

	if (sptr != (char *)0)
		size = atoi (sptr);

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.pur_ord_no, poNumber);
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	dsp_screen (" Printing Pro-Forma Cost of In-Transits ", comm_rec.co_no, comm_rec.co_name);

	ReportHeading (0L, "", poNumber);

	abc_selfield (poln, "poln_id_no");
	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = 0;

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (!cc)
			SaveDetailLine (1, size);
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (err_str, "%-16.16s", sortRec [i].sortCode + 1 + size);
		dsp_process (" Read : ", err_str);

		fprintf (pout, "%s\n", sortRec [i].sortCode + 1 + size);
	}
	fprintf (pout, ".EOF\n");
	pclose (pout);

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
SaveDetailLine (
	int type, 
	int size)
{
    char 	cTemp [256];
	double	margin 		= 0.00;
	double	cost 		= 0.00;
	double	last_cost	= 0.00;
	double	sell_price 	= 0.00;
	double	no_gst_sell = 0.00;
	double	new_sell 	= 0.00;
	float	PurCnvFct 	= 0.00;
	float	CnvFct		= 0.00;

	dsp_process (" Sort : ", inmr_rec.item_no);

	last_cost 	= CENTS (FindLastCost (inmr_rec.hhbr_hash, comm_rec.est_no));
	cost 		= out_cost (last_cost, inmr_rec.outer_size);

	if (taxInclusive)
	{
		/*
		 * sell_price - gst exclusive	
		 */
		sell_price = (double) gstPc;
		sell_price *= GetRetail ();
	}
	else
		sell_price = GetRetail ();

	/*
	 * calculate sell margin on gst exclusive sell price	
	 */
	if (cost == 0.00 || sell_price == 0.00)
		margin = 0.00;
	else
	{
		margin = (sell_price - cost);
		margin /= sell_price;
	}

	no_gst_sell = twodec (poln_rec.land_cst);
	no_gst_sell = out_cost (no_gst_sell, inmr_rec.outer_size);
	if (margin != 1.00)
		no_gst_sell /= (1.00 - margin);

	margin *= 100.00;
	new_sell = DOLLARS (100.00 + taxRate);
	new_sell *= no_gst_sell;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

    if (envVarGst)
	{
		sprintf 
		(
			cTemp,
			"%-*.*s |%-16.16s|%-40.40s| %s |%10.2f|%10.2f|%10.2f|%7.2f|%10.2f|%10.2f|%7.2f|%10.2f|%10.2f|",
			size, size, inmr_rec.category,
			inmr_rec.item_no,
			poln_rec.item_desc,
			inum_rec.uom,
			 (type == 0) ? posl_rec.ship_qty * CnvFct
                        : poln_rec.qty_ord * CnvFct,
			DOLLARS (sell_price),
			DOLLARS (last_cost),
			margin,
			 (type == 0) ? posl_rec.sup_price
                        : poln_rec.fob_fgn_cst,
			poln_rec.land_cst,
			margin,
			no_gst_sell,
			new_sell
		);
	}
	else
    {
		sprintf 
		(
			cTemp,
			"%-*.*s |%-16.16s|%-40.40s| %s |%10.2f|%10.2f|%10.2f|%7.2f|%10.2f|%10.2f|%7.2f|%10.2f|",
			size, size, inmr_rec.category,
			inmr_rec.item_no,
			poln_rec.item_desc,
			inum_rec.uom,
			 (type == 0) ? posl_rec.ship_qty * CnvFct
                        : poln_rec.qty_ord * CnvFct,
			DOLLARS (sell_price),
			DOLLARS (last_cost),
			margin,
			 (type == 0) ? posl_rec.sup_price
                        : poln_rec.fob_fgn_cst,
			poln_rec.land_cst,
			margin,
			no_gst_sell
		);
    }
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	/*
	 * Load values into array element sortCnt.
	 */
	strcpy (sortRec [sortCnt].sortCode, cTemp);
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

/*
 * Look up price at company level. If not found then price = 0.00.   
 */
double	
GetRetail (void)
{
	inpr_rec.hhgu_hash  = 0L;
	inpr_rec.hhbr_hash  = inmr_rec.hhbr_hash;
	inpr_rec.price_type = 1;
	strcpy (inpr_rec.br_no, "  ");
	strcpy (inpr_rec.wh_no, "  ");
	sprintf (inpr_rec.curr_code, "%-3.3s", envVarCurrCode);
	sprintf (inpr_rec.area_code, "%-2.2s", "  ");
	sprintf (inpr_rec.cust_type, "%-3.3s", "   ");
	cc = find_rec (inpr, &inpr_rec, COMPARISON, "r");
	if (!cc)
		return (inpr_rec.base);
		
	/*
	 * Price not found. 
	 */
	return ((double)0.00);
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
		rv_pr (ML (mlPoMess138), 21, 0, 1);
		line_at (1,0,80);
		line_at (20,0,80);
		line_at (22,0,80);
		box (0, 3, 80, 3);

		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

char*
GetShipmentNumber (
	long	hhshHash)
{
	abc_selfield (posh, "posh_hhsh_hash");

	posh_rec.hhsh_hash	=	hhshHash;
    cc = find_rec (posh, &posh_rec, EQUAL, "r");
    if (cc)
        strcpy (err_str, " N/A ");
    else
        sprintf (err_str, "%-12.12s", posh_rec.csm_no);

	abc_selfield (posh, "posh_id_no");

    return (err_str);
}

double	
FindLastCost (
	long	hhbrHash, 
	char	*branchNumber)
{
	inei_rec.hhbr_hash = hhbrHash;
	strcpy (inei_rec.est_no,branchNumber);
	cc = find_rec (inei, &inei_rec, COMPARISON,"r");
	return ((cc) ? 0.00 : inei_rec.last_cost);
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
