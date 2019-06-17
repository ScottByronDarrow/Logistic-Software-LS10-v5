/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: so_pwindow.c,v 5.2 2001/08/09 09:21:44 scott Exp $
|  Program Name  : (so_pwindow.c  )                                   |
|  Program Desc  : (Order Entry window display program.         )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, incc, pomr, poln,     ,     ,         |
|                       ,     ,     ,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
|  Date Modified : (04/01/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/06/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (21/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (10/09/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (14/01/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (14/01/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/05/94)      | Modified by : Scott B Darrow.    |
|  Date Modified : (07/11/94)      | Modified by : Aroha Merrilees.   |
|  Date Modified : (30/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : Added Available Column.                            |
|                :                                                    |
|  Date Modified : (01/06/89) - added envVarSoFwdAvl                      |
|                : (21/09/90) - General Update for New Scrgen. S.B.D. |
|                : (10/09/91) - Updated for Synonyms.                 |
|                :                                                    |
|  (14/01/94)    : DHL 10162.  Tidy code for global mods before logic |
|                : change.                                            |
|  (14/01/94)    : DHL 10162.  Clear display values before each       |
|                : iteration.                                         |
|                :                                                    |
|  (07/11/94)    : PSL 11299 - mfg cutover - avail less qc qty        |
|  (30/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_pwindow.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_pwindow/so_pwindow.c,v 5.2 2001/08/09 09:21:44 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_so_mess.h>
#include	<ml_std_mess.h>

	int	np_fn;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;

	char	*data = "data";

	long	dueDate 	= 0L;
	float	qtyOrder 	= 0.00;
	char	poNumber [sizeof pohr_rec.pur_ord_no],
			supplier [sizeof sumr_rec.crd_no];

	int		envVarSoFwdAvl 	= TRUE,
			envVarQcApply 	= FALSE,
			envVarSkQcAvl 	= FALSE;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	MainDisplay 		(long, long);
void 	heading 			(void);
static	int	FindPoDate 		(long, long, float);
static	int	FindPohr		(long);
void 	StatusDisplay 		(long);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	char	hhbrString [11];
	char	hhccString [11];
	long	hhbrHash = 0L;
	long	hhccHash = 0L;
	int		PID;

	envVarSoFwdAvl 	= (sptr = chk_env ("SO_FWD_AVL")) 	? atoi (sptr) : 0;
	envVarQcApply 	= (sptr = chk_env ("QC_APPLY")) 	? atoi (sptr) : 0;
	envVarSkQcAvl 	= (sptr = chk_env ("SK_QC_AVL")) 	? atoi (sptr) : 0;

	init_scr ();

	OpenDB ();

	sptr = gets (err_str);
	PID = atoi (sptr);

    if ((np_fn = IP_OPEN (PID)) == -1) 
	{
        CloseDB (); 
		FinishProgram ();
        return (EXIT_FAILURE);
    }

	sptr = gets (err_str);
	while (sptr != (char *)0)
	{
		sprintf (hhbrString, "%-10.10s", sptr);
		sprintf (hhccString, "%-10.10s", sptr + 10);

		hhbrHash = atol (hhbrString);
		hhccHash = atol (hhccString);

		crsr_off ();
		if (hhccHash == 0L)
			StatusDisplay (hhbrHash);
		else
		{
			heading ();
			MainDisplay (hhbrHash, hhccHash);
		}
		crsr_on ();
		fflush (stdout);

		IP_WRITE (np_fn);

		/*---------------------------
		| This one is a dummy gets. |
		---------------------------*/
		sptr = gets (err_str);
	}
	IP_CLOSE (np_fn);
    CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (comm);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (ccmr);
	abc_dbclose (data);
}

void
MainDisplay (
	long	hhbrHash, 
	long	hhccHash)
{
	float	wh_on_hand = 0.00;
	float	wh_comm    = 0.00;
	float	wh_bo      = 0.00;
	float	wh_avail   = 0.00;
	float	co_avail   = 0.00;
	char	disp_str [201];

	qtyOrder = 0.00;
	sprintf (pohr_rec.pur_ord_no, "%15.15s", " ");
	sprintf (sumr_rec.crd_no, "%6.6s",  " ");
	dueDate = 0L;

	inmr_rec.hhbr_hash	=	hhbrHash;
   	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	incc_rec.hhcc_hash = hhccHash;

	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, incc, "DBFIND");

	wh_on_hand = incc_rec.closing_stock;
	if (envVarSoFwdAvl)
		wh_comm = incc_rec.committed + incc_rec.forward;
	else
		wh_comm = incc_rec.committed;

	wh_bo = incc_rec.backorder;

	if (envVarSoFwdAvl)
	{
		wh_avail = 	incc_rec.closing_stock - 
                   	incc_rec.committed - 
		     	   	incc_rec.backorder - 
			       	incc_rec.forward;
	}
	else
	{
		wh_avail = 	incc_rec.closing_stock - 
                   	incc_rec.committed - 
		     	   	incc_rec.backorder;
	}
	if (envVarQcApply && envVarSkQcAvl)
		wh_avail -= incc_rec.qc_qty;

	if (envVarSoFwdAvl)
	{
		co_avail = 	inmr_rec.on_hand - 
		           	inmr_rec.committed - 
		           	inmr_rec.backorder - 
		           	inmr_rec.forward;
	}
	else
	{
		co_avail = 	inmr_rec.on_hand - 
		           	inmr_rec.committed - 
		           	inmr_rec.backorder;
	}
	if (envVarQcApply && envVarSkQcAvl)
		co_avail -= inmr_rec.qc_qty;

	FindPoDate 
	(
		alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash),
		hhccHash,
		wh_avail
	);

	strcpy (poNumber, (!strcmp (pohr_rec.pur_ord_no,"               ")) 
						      ? " " : pohr_rec.pur_ord_no);
	strcpy (supplier,sumr_rec.crd_no);

	sprintf 
	(
		disp_str,  
		"%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%10.2f ^E%-16.16s ^E%-15.15s^E%10.2f ^E%10.10s^E %6.6s ",
		wh_on_hand,
		wh_comm,
		wh_bo,
		wh_avail,
		co_avail,
		(!strcmp (inmr_rec.alternate,"                ")) ? "NONE" : inmr_rec.alternate,
		poNumber,
		qtyOrder,
		(dueDate <= 0L) ? "   N/A    " : DateToString (dueDate),
		supplier
	);
	Dsp_saverec (disp_str);
	Dsp_srch ();
	Dsp_close ();
}

static int
FindPohr (
	long	hhpoHash)
{
	pohr_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (pohr_rec.pur_ord_no, "%15.15s", " ");
		sprintf (sumr_rec.crd_no, 	  "%6.6s",   " ");
		dueDate	=	0L;
		return (cc);
	}

	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	{
		sprintf (pohr_rec.pur_ord_no, "%15.15s", " ");
		sprintf (sumr_rec.crd_no, "%6.6s",  " ");
		dueDate	=	0L;
	}
	return (EXIT_SUCCESS);
}

void
heading (
 void)
{
	Dsp_open (0, 17, 2);
	Dsp_saverec ("  ON HAND  | COMMITTED | BACKORDER | AVAILABLE | AVAILABLE |    ALTERNATE    |PURCHASE ORDER | REP.ORDER |    P/O   |   P/O  ");
	Dsp_saverec (" WAREHOUSE | WAREHOUSE | WAREHOUSE | WAREHOUSE |  COMPANY  |  STOCK NUMBER   |     NUMBER    | QUANTITY  |   DATE   |SUPPLIER");
	Dsp_saverec ("");
}

/*=======================================================
| Calculate the due date from the purchase orders	|
=======================================================*/
static int 
FindPoDate (
	long	hhbrHash,
	long	hhccHash,
	float	quantityAvailable)
{
	float	qtyLeft = 0.00;

	sprintf (poNumber, "%15.15s", " ");
	sprintf (supplier,  "%6.6s",  " ");

	qtyLeft = quantityAvailable;
	qtyLeft *= -1.00;

	/*------------------------------------------
	| Find poln records for the warehouse item |
	------------------------------------------*/
	poln_rec.hhbr_hash 	= hhbrHash;
	poln_rec.due_date 	= 0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (poln_rec.hhcc_hash != hhccHash)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		qtyOrder =  poln_rec.qty_ord - poln_rec.qty_rec;
		qtyLeft -= qtyOrder;

		/*------------------------------------
		| The P/O which put the qty into -ve |
		| is the P/O to use.                 |
		------------------------------------*/
		if (qtyLeft < 0.00)
		{
			qtyLeft *= -1;
			qtyOrder = (qtyLeft > qtyOrder) ? qtyOrder : qtyLeft;
			dueDate = poln_rec.due_date;
			break;
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	if (!cc && poln_rec.hhbr_hash == hhbrHash)
		return (FindPohr (poln_rec.hhpo_hash));
	else
		return (FindPohr (0L));
}

/*========================
| Display Branch Status. |
========================*/
void
StatusDisplay (
	long	hhbrHash)
{
	char	disp_str [300];
	float	wk_avail = 0.00;
	int		no_avail = TRUE;

	Dsp_open (0, 17, 3);
	Dsp_saverec ("                Warehouse                 |  On Hand   | Committed  | Backorder  | Forward O. | Available  |  On Order. ");
	Dsp_saverec ("");
	Dsp_saverec ("");

	inmr_rec.hhbr_hash	=	hhbrHash;
    cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no,  "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
						  				 inmr_rec.hhsi_hash);
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (!cc)
		{
			/*---------------------------
			| calculate available stock |
			---------------------------*/
			if (envVarSoFwdAvl)
			{
				wk_avail = 	incc_rec.closing_stock - 
					   	   	incc_rec.committed - 
					   	   	incc_rec.backorder - 
					   	   	incc_rec.forward;
			}
			else
			{
				wk_avail = 	incc_rec.closing_stock - 
					   	   	incc_rec.committed - 
					   	   	incc_rec.backorder;
			}
			if (envVarQcApply && envVarSkQcAvl)
				wk_avail -= incc_rec.qc_qty;
			if (wk_avail <= 0.00)
			{
				cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
				continue;
			}
			no_avail = FALSE;

			sprintf (disp_str,
					" %-40.40s ^E %10.0f ^E %10.0f ^E %10.0f ^E %10.0f ^E %10.0f ^E %10.0f ",
					ccmr_rec.name,
					incc_rec.closing_stock,
					incc_rec.committed,
					incc_rec.backorder,
					incc_rec.forward,
					wk_avail,
					incc_rec.on_order);
			Dsp_saverec (disp_str);
		}
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	if (no_avail)
		Dsp_saverec (ML ("No Stock available at any branch."));
	Dsp_srch ();
	Dsp_close ();
}
