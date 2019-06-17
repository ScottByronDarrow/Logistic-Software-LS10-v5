/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_assign.c,v 5.6 2002/02/04 03:06:58 scott Exp $
|  Program Name  : (po_so_assign.c)
|  Program Desc  : (Purchase Order/Sales Order Assignment)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/09/90         |
|---------------------------------------------------------------------|
| $Log: so_assign.c,v $
| Revision 5.6  2002/02/04 03:06:58  scott
| Updated to fix column lineup.
|
| Revision 5.5  2002/01/23 07:37:47  scott
| S/C 693.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_assign.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_so_assign/so_assign.c,v 5.6 2002/02/04 03:06:58 scott Exp $";

#include <ml_std_mess.h>
#include <ml_po_mess.h>
#define MAXWIDTH 	200
#define MAXLINES 	500

#define	SOLN_BACKORDER	 (soln_rec.status [0] == 'B')

#define	ITLN_BACKORDER	 (itln_rec.status [0] == 'B' || \
			  			 (itln_rec.status [0] == 'T' && \
			    		  itln_rec.qty_border != 0.00))

#define	DELETED_PO		 (pohr_rec.status [0] == 'D')

#define	SEL_PO			 (selectedPo == TRUE)

#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cumrRecord	cumr_rec;
struct posoRecord	poso_rec;
struct posoRecord	poso2_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct ccmrRecord	ccmr_rec;

	/*
	 * Special fields and flags. 
	 */
	char	*data 	= "data", 
			*poso2	= "poso2";

	char	branchNumber [3];
 
	int		envVarCrCo 	= 0, 
			selectedPo 	= TRUE;

	long	currentHhcc = 0L;

	float	standardConvert	=	0.00, 
			purchaseConvert	=	0.00, 
			programConvert	=	0.00;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];


	char	previousPo 		[sizeof pohr_rec.pur_ord_no];
	char	previousSupp 	[sizeof sumr_rec.crd_no];
	char	systemDate [11];
	char 	supplierNo 		[sizeof sumr_rec.crd_no];
	long	hhso_hash;
	long	due_date;
	char	orderNumber 	[sizeof sohr_rec.order_no];
	char	trans_no [7];
	float	qty_ord;
	char	custNumber 		[sizeof cumr_rec.dbt_no];
	float	qty_alloc;
	char	custAcronym 	[sizeof cumr_rec.dbt_acronym];
	char	Pur_UOM [5];
	float	Pur_CnvFct;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplier", 	 4, 21, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier No.", " ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.supplierNo}, 
	{1, LIN, "name", 	 4, 80, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " - ", " ", 
		 NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "porder", 	 5, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "Purchase Order No.", " ", 
		 NE, NO, JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{1, LIN, "date_raised", 	 5, 80, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Date Raised.", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&pohr_rec.date_raised}, 
	{2, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		 NA, NO, JUSTLEFT, "", "", inmr_rec.item_no}, 
	{2, TAB, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "           Item  Description.           ", " ", 
		 NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{2, TAB, "hhbr", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&inmr_rec.hhbr_hash}, 
	{2, TAB, "hhpl", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpl_hash}, 
	{2, TAB, "hhsl", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&soln_rec.hhsl_hash}, 
	{2, TAB, "itff", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&itln_rec.itff_hash}, 
	{2, TAB, "due_date", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", " Due Date ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&poln_rec.due_date}, 
	{2, TAB, "PUR_CNV", 	 0, 0, FLOATTYPE, 
		"NNNNNNNN.NNNNNNN", "          ", 
		" ", " ", "", " ", 
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.Pur_CnvFct}, 
	{2, TAB, "PUR_UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", "UOM ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.Pur_UOM}, 
	{2, TAB, "qty_ord", 	 0, 0, FLOATTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "Qty Avail ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_ord}, 
	{2, TAB, "orderno", 	 0, 1, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", " ", "Order No ", " ", 
		 ND, NO, JUSTRIGHT, "", "", local_rec.orderNumber}, 
	{2, TAB, "custNo", 	 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Cust No.", " ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.custNumber}, 
	{2, TAB, "custAcronym", 	 0, 1, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", "  Acronym. ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.custAcronym}, 
	{2, TAB, "trans_no", 	 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Trans# ", " ", 
		 ND, NO, JUSTRIGHT, "", "", local_rec.trans_no}, 
	{2, TAB, "cus_stk", 	 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Cust/Stk", " ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.custNumber}, 
	{2, TAB, "wh_acr", 	 0, 1, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", "W/H Acronym", " ", 
		 ND, NO, JUSTLEFT, "", "", local_rec.custAcronym}, 
	{2, TAB, "qty_alloc", 	 0, 1, FLOATTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", " Quantity.  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_alloc}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};


#include <FindSumr.h>
/*
 * Function Declarations 
 */
float 	GetAvailable 		(long);
int 	CheckLines 			(long);
int 	GetOrder 			(long);
int 	GetTransfers 		(long);
int 	InsertLine 			(void);
int 	ValidItem 			(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CloseDB 			(void);
void 	IntFindSupercession (char *);
void 	LoadLines 			(long);
void 	OpenDB 				(void);
void 	PrintCoStuff 		(void);
void 	ReadMisc 			(void);
void 	SrchIthr 			(char *);
void 	SrchPohr 			(char *);
void 	SrchSohr 			(char *);
void 	Update 				(void);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "po_so_assign"))
	{
		FLD ("orderno")   	= YES;
		FLD ("custNo")   	= NA;
		FLD ("custAcronym") = NA;
		FLD ("trans_no")  	= ND;
		FLD ("cus_stk")   	= ND;
		FLD ("wh_acr")    	= ND;
		selectedPo 			= TRUE;
	}
	else
	{
		FLD ("orderno")   	= ND;
		FLD ("custNo")   	= ND;
		FLD ("custAcronym") = ND;
		FLD ("trans_no")  	= YES;
		FLD ("cus_stk")   	= NA;
		FLD ("wh_acr")    	= NA;
		selectedPo 			= FALSE;
	}
	
	tab_row = 7;
	tab_col = 0;

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*
	 * Open main database files. 
	 */
	OpenDB ();


	envVarCrCo = atoi (get_env ("CR_CO"));
	strcpy (branchNumber, (!envVarCrCo) ? " 0" : comm_rec.est_no);

	swide ();
	clear ();

	strcpy (local_rec.previousPo, 	"000000000000000");
	strcpy (local_rec.previousSupp, 	"000000");

	while (prog_exit == 0)
	{
		if (restart) 
			abc_unlock (poln);

		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		lcount [2] 	= 0;
		init_vars (1);	
		init_vars (2);	

		/*
		 * Enter screen 1 linear input. 
		 * Turn screen initialise on.  
		 */
		heading (1);
		scn_display (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		scn_display (2);
		edit (2);
		if (restart)
			continue;

		Update ();

		init_ok = 1;
		init_vars (2);
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

	ReadMisc ();
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrCo) ? "sumr_id_no" 
							   							   : "sumr_id_no3");

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poso, poso_list, POSO_NO_FIELDS, "poso_hhpl_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	abc_alias (poso2, poso);

	open_rec (poso2, poso_list, POSO_NO_FIELDS, "poso_hhpl_hash");
	if (SEL_PO)
	{
		open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
		open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
		open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	}
	else
	{
		open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
		open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	}
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poso);
	abc_fclose (inum);
	abc_fclose (poso2);

	if (SEL_PO)
	{
		abc_fclose (soln);
		abc_fclose (sohr);
		abc_fclose (cumr);
	}
	else
	{
		abc_fclose (itln);
		abc_fclose (ithr);
	}
	abc_fclose (ccmr);
	abc_dbclose (data);
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
		file_err (cc, "ccmr", "DBFIND");

	currentHhcc = ccmr_rec.hhcc_hash;

	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

int
spec_valid (
 int field)
{
	int		i;
	int		soLineOk = FALSE;
	long	lastHhsl = 0L;
	
	/*
	 * Validate Creditor Number. 
	 */
	if (LCHECK ("supplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNo));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Purchase Order Number.
	 */
	if (LCHECK ("porder"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		/*
		 * Check if order is on file. 
		 */
		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type, "O");
		if (find_rec (pohr, &pohr_rec, COMPARISON, "r"))
		{
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Order already on file. 
		 */
		if (DELETED_PO)
		{
			print_mess (ML (mlPoMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (pohr_rec.drop_ship [0] == 'Y')
		{
			if (SEL_PO)
				print_mess (ML (mlPoMess032));
			else
				print_mess (ML (mlPoMess033));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		LoadLines (pohr_rec.hhpo_hash);
		entry_exit = 1;
		if (lcount [2] <= 0) 
		{
			print_mess (ML (mlPoMess004));
			sleep (sleepTime);
			clear_mess ();
			restart = 1;
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("item_no"))
	{
		if (prog_status == ENTRY)
		{
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Sales order number. 
	 */
	if (LCHECK ("orderno"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.orderNumber, 	"%-8.8s", " ");
			sprintf (local_rec.custNumber, 	"%6.6s", " ");
			sprintf (local_rec.custAcronym, 	"%9.9s", " ");
			local_rec.qty_alloc = 0.00;

			DSP_FLD ("orderno");
			DSP_FLD ("custNo");
			DSP_FLD ("custAcronym");
			DSP_FLD ("qty_alloc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		strcpy (sohr_rec.order_no, zero_pad (local_rec.orderNumber, 8));
		if (find_rec (sohr, &sohr_rec, COMPARISON, "r"))
		{
			print_mess (ML (mlStdMess102));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		soLineOk = FALSE;
		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		local_rec.qty_alloc = 0.00;

		while (!cc && sohr_rec.hhso_hash == soln_rec.hhso_hash)
		{
			if (inmr_rec.hhbr_hash != soln_rec.hhbr_hash)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
			local_rec.qty_alloc += ( (soln_rec.qty_order + 
					       		     soln_rec.qty_bord) * programConvert);

			if (local_rec.qty_alloc > local_rec.qty_ord)
				local_rec.qty_alloc = local_rec.qty_ord;
		
			lastHhsl = soln_rec.hhsl_hash;
			soLineOk = TRUE;
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		if (!soLineOk)
		{
			sprintf (local_rec.orderNumber, 	"%-8.8s", " ");
			sprintf (local_rec.custNumber, 		"%-6.6s", " ");
			sprintf (local_rec.custAcronym, 	"%-9.9s", " ");
			local_rec.qty_alloc = 0.00;

			DSP_FLD ("orderno");
			DSP_FLD ("custNo");
			DSP_FLD ("custAcronym");
			DSP_FLD ("qty_alloc");

			sprintf (err_str, ML (mlPoMess034), sohr_rec.order_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
		{
			sprintf (local_rec.orderNumber, 	"%-8.8s", " ");
			sprintf (local_rec.custNumber, 		"%-6.6s", " ");
			sprintf (local_rec.custAcronym, 	"%-9.9s", " ");
			local_rec.qty_alloc = 0.00;
		}
		else
		{
			strcpy (local_rec.orderNumber, 	sohr_rec.order_no);
			strcpy (local_rec.custNumber, 	cumr_rec.dbt_no);
			strcpy (local_rec.custAcronym, 	cumr_rec.dbt_acronym);
			soln_rec.hhsl_hash = lastHhsl;
			itln_rec.itff_hash = 0L;
		}
		DSP_FLD ("orderno");
		DSP_FLD ("custNo");
		DSP_FLD ("custAcronym");
		DSP_FLD ("qty_alloc");

		return (InsertLine ());
	}
	/*
	 * Validate Transfer. 
	 */
	if (LCHECK ("trans_no"))
	{
		if (FLD ("trans_no") == ND)
			return (EXIT_SUCCESS);

		if (end_input)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.trans_no, 	"%6.6s", " ");
			sprintf (local_rec.custNumber, 	"%6.6s", " ");
			sprintf (local_rec.custAcronym, "%9.9s", " ");
			local_rec.qty_alloc = 0.00;

			DSP_FLD ("trans_no");
			DSP_FLD ("cus_stk");
			DSP_FLD ("wh_acr");
			DSP_FLD ("qty_alloc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchIthr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (ithr_rec.co_no, comm_rec.co_no);
		strcpy (ithr_rec.type, "T");
		ithr_rec.del_no = atol (local_rec.trans_no);
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (ithr_rec.co_no, comm_rec.co_no);
			strcpy (ithr_rec.type, "B");
			ithr_rec.del_no = atol (local_rec.trans_no);
			cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess103));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		soLineOk = FALSE;
		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = 0;
		cc = find_rec (itln, &itln_rec, GTEQ, "r");
		while (!cc && itln_rec.hhit_hash == ithr_rec.hhit_hash)
		{
			if (inmr_rec.hhbr_hash != itln_rec.hhbr_hash)
			{
				cc = find_rec (itln, &itln_rec, NEXT, "r");
				continue;
			}
			local_rec.qty_alloc = ( (itln_rec.qty_order + 
					       		     itln_rec.qty_border) * programConvert);

			if (local_rec.qty_alloc > local_rec.qty_ord)
				local_rec.qty_alloc = local_rec.qty_ord;
		
			soLineOk = TRUE;
			break;
		}
		if (!soLineOk)
		{
			sprintf (local_rec.trans_no, 	"%6.6s", " ");
			sprintf (local_rec.custNumber, 	"%6.6s", " ");
			sprintf (local_rec.custAcronym, 	"%9.9s", " ");
			local_rec.qty_alloc = 0.00;

			DSP_FLD ("trans_no");
			DSP_FLD ("cus_stk");
			DSP_FLD ("wh_acr");
			DSP_FLD ("qty_alloc");

			sprintf (err_str, ML (mlPoMess035), ithr_rec.del_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		ccmr_rec.hhcc_hash	=	itln_rec.r_hhcc_hash;
		if (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"))
		{
			sprintf (local_rec.trans_no, 	"%6.6s", " ");
			sprintf (local_rec.custNumber, 	"%6.6s", " ");
			sprintf (local_rec.custAcronym, 	"%9.9s", " ");
			local_rec.qty_alloc = 0.00;
		}
		else
		{
			sprintf (local_rec.trans_no, "%06ld", ithr_rec.del_no);
			if (itln_rec.stock [0] == 'S')
				strcpy (local_rec.custNumber, "STOCK.");
			else
				strcpy (local_rec.custNumber, " CUST ");
			strcpy (local_rec.custAcronym, ccmr_rec.acronym);
		}
		DSP_FLD ("trans_no");
		DSP_FLD ("cus_stk");
		DSP_FLD ("wh_acr");
		DSP_FLD ("qty_alloc");
		return (EXIT_SUCCESS); 
	}
	/*
	 * Validate Quantity Allocated to sales order. 
	 */
	if (LCHECK ("qty_alloc"))
	{
		if (FLD ("trans_no")  == ND)
		{
			if (!strcmp (local_rec.orderNumber, "        "))
			{
				print_mess (ML (mlPoMess036));
				local_rec.qty_alloc = 0.00;
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_SUCCESS);
			}
		}

		if (local_rec.qty_alloc > local_rec.qty_ord)
		{
			sprintf (err_str, ML (mlPoMess037), local_rec.qty_ord);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if ( (local_rec.qty_alloc < local_rec.qty_ord) &&
		      local_rec.qty_alloc != 0.00)
		{
			i = prmptmsg (ML (mlPoMess038), "YyNn", 1, 2);
			if (i == 'Y' || i == 'y')
				InsertLine ();
			move (0, 2);
			cl_line ();
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Insert line. 
 */
int
InsertLine (void)
{
	float	wk_qty = local_rec.qty_ord - local_rec.qty_alloc;
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (wk_qty <= 0.00)
		return (EXIT_SUCCESS);

	move (0, 2);
	cl_line ();
	print_at (2, 0, ML (mlPoMess039));

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (vars [scn_start].row < MAXLINES)
		vars [label ("item_no")].row++;

	if (lcount [2] >= vars [label ("item_no")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	local_rec.qty_ord = local_rec.qty_alloc;

	putval (line_cnt);

	for (i = line_cnt, line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;
	sprintf (local_rec.orderNumber, 	"%-8.8s", " ");
	sprintf (local_rec.trans_no, 	"%-6.6s", " ");
	sprintf (local_rec.custNumber, 	"%-6.6s", " ");
	sprintf (local_rec.custAcronym, 	"%-9.9s", " ");
	local_rec.qty_alloc = 0.00;
	local_rec.qty_ord 	= wk_qty;
	putval (line_cnt);

	init_ok = 0;
	line_cnt = i;
	getval (line_cnt);

	DSP_FLD ("orderno");
	DSP_FLD ("custNo");
	DSP_FLD ("custAcronym");

	DSP_FLD ("trans_no");
	DSP_FLD ("cus_stk");
	DSP_FLD ("wh_acr");

	DSP_FLD ("qty_alloc");
	DSP_FLD ("qty_ord");
	move (0, 2);
	cl_line ();
	scn_display (2);
	return (EXIT_SUCCESS);
}

/*
 * Routine to read all poln records whose hash matches the one on the   
 * pohr record. Stores all non screen relevant details in another       
 * structure. Also gets part number for the part hash. And g/l account  
 * number.                                                              
 */
void
LoadLines (
	long	hhpoHash)
{

	int	first_failed = FALSE;
	float	available = 0.00;

	print_at (2, 1, ML (mlStdMess035));
	
	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (2);
	lcount [2] = 0;

	abc_selfield (inmr, "inmr_hhbr_hash");
	if (SEL_PO)
	{
		abc_selfield (sohr, "sohr_hhso_hash");
		abc_selfield (soln, "soln_hhsl_hash");
	}
	else
	{
		abc_selfield (ithr, "ithr_hhit_hash");
		abc_selfield (itln, "itln_itff_hash");
	}

	poln_rec.hhpo_hash = hhpoHash;
	poln_rec.line_no = 0; 

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (!cc)
		{
			if (strcmp (inmr_rec.supercession, "                "))
			{
				abc_selfield (inmr, "inmr_id_no");
				IntFindSupercession (inmr_rec.supercession);
				abc_selfield (inmr, "inmr_hhbr_hash");
			}
		}
		if (cc) 
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		standardConvert	= (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		purchaseConvert = (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);
		programConvert	=	standardConvert / purchaseConvert;

		available 	= poln_rec.qty_ord - poln_rec.qty_rec;

		sprintf (local_rec.orderNumber, 		"%-8.8s", " ");
		sprintf (local_rec.trans_no, 		"%-6.6s", " ");
		sprintf (local_rec.custNumber, 		"%-6.6s", " ");
		sprintf (local_rec.custAcronym, 	"%-9.9s", " ");
		strcpy (local_rec.Pur_UOM, 		inum_rec.uom);
		local_rec.qty_alloc = 0.00;
		first_failed = TRUE;

		poso_rec.hhpl_hash	=	poln_rec.hhpl_hash;
		cc = find_rec (poso, &poso_rec, GTEQ, "r");
		while (!cc && poso_rec.hhpl_hash == poln_rec.hhpl_hash)
		{
			if (SEL_PO && poso_rec.hhsl_hash == 0L)
			{
				cc = find_rec (poso, &poso_rec, NEXT, "r");
				continue;
			}
			if (!SEL_PO && poso_rec.itff_hash == 0L)
			{
				cc = find_rec (poso, &poso_rec, NEXT, "r");
				continue;
			}
			first_failed = FALSE;
			if (SEL_PO)
			{
				if (GetOrder (poso_rec.hhsl_hash))
				{
					sprintf (local_rec.orderNumber, "%-8.8s", " ");
					sprintf (local_rec.custNumber, "%-6.6s", " ");
					sprintf (local_rec.custAcronym, "%-9.9s", " ");
					local_rec.qty_alloc = 0.00;
					local_rec.qty_ord = 0.00;
				}
				else
				{
					strcpy (local_rec.custNumber, cumr_rec.dbt_no);
					strcpy (local_rec.custAcronym, cumr_rec.dbt_acronym);
					strcpy (local_rec.orderNumber, sohr_rec.order_no);
					local_rec.qty_alloc = poso_rec.qty * purchaseConvert;
				}
			}
			else
			{
				if (GetTransfers (poso_rec.itff_hash))
				{
					sprintf (local_rec.orderNumber, "%-6.6s", " ");
					sprintf (local_rec.custNumber, "%-6.6s", " ");
					sprintf (local_rec.custAcronym, "%-9.9s", " ");
					local_rec.qty_alloc 	= 0.00;
					local_rec.qty_ord 		= 0.00;
				}
				else
				{
					sprintf (local_rec.trans_no, "%06ld", ithr_rec.del_no);
					if (itln_rec.stock [0] == 'S')
						strcpy (local_rec.custNumber, "STOCK.");
					else
						strcpy (local_rec.custNumber, " CUST ");
					strcpy (local_rec.custAcronym, ccmr_rec.acronym);
					local_rec.qty_alloc = poso_rec.qty * purchaseConvert;
				}
			}
			local_rec.qty_ord = available - GetAvailable (poln_rec.hhpl_hash) +
					    					poso_rec.qty;
			local_rec.qty_ord *= purchaseConvert;

			local_rec.Pur_CnvFct	=	purchaseConvert;

			putval (lcount [2]);
			/*
			 * Too many orders . 
			 */
			if (lcount [2]++ > MAXLINES) 
				break;

			cc = find_rec (poso, &poso_rec, NEXT, "r");
		}
		if (first_failed)
		{
			local_rec.qty_ord = available - GetAvailable (poln_rec.hhpl_hash);
			local_rec.qty_ord /= purchaseConvert;

			local_rec.Pur_CnvFct	=	purchaseConvert;
			putval (lcount [2]);
			/*
			 * Too many orders . 
			 */
			if (lcount [2]++ > MAXLINES) 
				break;
		}
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}

	abc_selfield (inmr, "inmr_id_no");
	if (SEL_PO)
	{
		abc_selfield (soln, "soln_id_no");
		abc_selfield (sohr, "sohr_id_no2");
	}
	else
	{
		abc_selfield (itln, "itln_id_no");
		abc_selfield (ithr, "ithr_id_no");
	}

	/*
	 * Prevents entry past end of number of entries.	
	 */
	vars [scn_start].row = lcount [2];

	scn_set (1);
	move (1, 2);
	cl_line ();
}

float 	
GetAvailable (
	long	hhplHash)
{
	float	qty_alloc = 0.00;

	poso2_rec.hhpl_hash = hhplHash;
	cc = find_rec (poso2, &poso2_rec, GTEQ, "r");
	while (!cc && poso2_rec.hhpl_hash == hhplHash)
	{
		qty_alloc += poso2_rec.qty;
		cc = find_rec (poso2, &poso2_rec, NEXT, "r");
	}
	return (qty_alloc);
}

/*
 * Get order details and other. 
 */
int
GetOrder (
	long	hhslHash)
{
	soln_rec.hhsl_hash	=	hhslHash;
	if (find_rec (soln, &soln_rec, EQUAL, "r"))
		return (EXIT_FAILURE);

	sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
	if (find_rec (sohr, &sohr_rec, EQUAL, "r"))
		return (EXIT_FAILURE);

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	if (find_rec (cumr, &cumr_rec, EQUAL, "r"))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * Get order details and other. 
 */
int
GetTransfers (
	long	itffHash)
{
	itln_rec.itff_hash	=	itffHash;
	if (find_rec (itln, &itln_rec, EQUAL, "r"))
		return (EXIT_FAILURE);

	ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
	if (find_rec (ithr, &ithr_rec, EQUAL, "r"))
		return (EXIT_FAILURE);

	ccmr_rec.hhcc_hash = itln_rec.r_hhcc_hash;
	if (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
IntFindSupercession (
 char *item_no)
{
	if (!strcmp (item_no, "                "))
		return;

	abc_selfield (inmr, "inmr_id_no");
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", item_no);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (!cc)
		IntFindSupercession (inmr_rec.supercession);
}

/*
 * Update relevent files. 
 */
void
Update (void)
{
	clear ();
	print_at (1, 0, ML (mlPoMess026));

	/*
	 * Process all purchase order lines
	 */
	print_at (1, 0, ML (mlPoMess026));

	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);

		poso_rec.hhpl_hash	=	poln_rec.hhpl_hash;
		cc = find_rec (poso, &poso_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpl_hash == poso_rec.hhpl_hash)
		{
			if (SEL_PO)
			{
				if (poso_rec.hhsl_hash > 0L)
				{
					abc_delete (poso);
					poso_rec.hhpl_hash	=	poln_rec.hhpl_hash;
					cc = find_rec (poso, &poso_rec, GTEQ, "r");
				}
				else
			    	cc = find_rec (poso, &poso_rec, NEXT, "r");
			}
			else
			{
				if (poso_rec.itff_hash > 0L)
				{
					abc_delete (poso);
					poso_rec.hhpl_hash	=	poln_rec.hhpl_hash;
					cc = find_rec (poso, &poso_rec, GTEQ, "r");
				}
				else
					cc = find_rec (poso, &poso_rec, NEXT, "r");
			}
		}	
	}
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);
			
		if (local_rec.qty_alloc <= 0.00)
			continue;

		poso_rec.hhpl_hash = poln_rec.hhpl_hash;
		poso_rec.hhsl_hash = (SEL_PO) ? soln_rec.hhsl_hash : 0L;
		poso_rec.itff_hash = (SEL_PO) ? 0L : itln_rec.itff_hash;
		poso_rec.qty = local_rec.qty_alloc / local_rec.Pur_CnvFct;
		poso_rec.qty_ord = local_rec.qty_ord / local_rec.Pur_CnvFct;
		cc = abc_add (poso, &poso_rec);
		if (cc)
			file_err (cc, poso, "DBADD");
	}

	strcpy (local_rec.previousPo, 	pohr_rec.pur_ord_no);
	strcpy (local_rec.previousSupp, 	sumr_rec.crd_no);
	scn_set (1);
}

/*
 * Search for purch order no
 */
void
SrchPohr (
	char *keyValue)
{
	work_open ();
	save_rec ("#Purchase Order ", "#Date Raised");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue)) && 
		!strcmp (pohr_rec.co_no, comm_rec.co_no) && 
		!strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] == 'O' && !DELETED_PO)
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}
/*
 * Search for order number. 
 */
void
SrchSohr (
	char	*keyValue)
{
	float	StdCnv	=	0.00, 
			OrdCnv	=	0.00, 
			CnvFct	=	0.00;

	int	first_line = TRUE;

	sprintf (err_str, "#Item : %16.16s          ", inmr_rec.item_no);

	_work_open (8, 0, 40);
	save_rec ("#Order", err_str);
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", keyValue);

	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");

	while (!cc && !strncmp (sohr_rec.order_no, keyValue, strlen (keyValue)) &&
	              !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
	              !strcmp (sohr_rec.br_no, comm_rec.est_no))
	{
		if (ValidItem ())
		{
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			StdCnv	= (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);

			inum_rec.hhum_hash	=	soln_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			OrdCnv	= (float) ( (cc) ? 1.00 : inum_rec.cnv_fct);

			CnvFct	=	StdCnv/OrdCnv;

			sprintf (err_str, " %s : %s %6.2f  / %s : %s ", 
					    (first_line) ? "Quantity"
	   						   : "        ", 

						inum_rec.uom, 
					    (soln_rec.qty_order + soln_rec.qty_bord) * CnvFct, 
					    (first_line) ? "Date"
					                   : "    ", 
					    (soln_rec.due_date) ? DateToString (soln_rec.due_date) : "  NONE ");

			first_line = FALSE;
			cc = save_rec (sohr_rec.order_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "sohr", "DBFIND");
}

/*
 * Search for Docket no 
 */
void
SrchIthr (
	char	*keyValue)
{
	_work_open (10, 6, 20);
	save_rec ("#No", "#Int Ref "); 
	strcpy (ithr_rec.co_no, comm_rec.co_no);
	strcpy (ithr_rec.type, "B");
	if (atol (keyValue) != 0L)
	{
		ithr_rec.del_no = atol (keyValue);
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%6ld [B]", ithr_rec.del_no);
				save_rec (err_str, ithr_rec.tran_ref);
			}
		}
	}
	else
	{
		ithr_rec.del_no = 0L;
		cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
		while (!cc &&
		       !strcmp (ithr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (ithr_rec.type, "B"))
		{ 
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%6ld [B]", ithr_rec.del_no);
				cc = save_rec (err_str, ithr_rec.tran_ref);
				if (cc)
					break;
			}
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
		}
	}
	strcpy (ithr_rec.co_no, comm_rec.co_no);
	strcpy (ithr_rec.type, "T");
	if (atol (keyValue) != 0L)
	{
		ithr_rec.del_no = atol (keyValue);
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%6ld [T]", ithr_rec.del_no);
				save_rec (err_str, ithr_rec.tran_ref);
			}
		}
	}
	else
	{
		ithr_rec.del_no = 0L;
		cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
		while (!cc &&
		       !strcmp (ithr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (ithr_rec.type, "T"))
		{ 
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%6ld [T]", ithr_rec.del_no);
				cc = save_rec (err_str, ithr_rec.tran_ref);
				if (cc)
					break;
			}
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
		}
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (ithr_rec.co_no, comm_rec.co_no);
	ithr_rec.del_no = atol (temp_str);
	ithr_rec.type [0] = temp_str [8];
	cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "ithr", "DBFIND");
}

int
CheckLines (
	long	hhitHash)
{
	itln_rec.hhit_hash = hhitHash;
	itln_rec.line_no = 0;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (itln_rec.i_hhcc_hash == currentHhcc && 
		     itln_rec.hhbr_hash == inmr_rec.hhbr_hash &&
		     (ITLN_BACKORDER))
			return (EXIT_FAILURE);
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

void
PrintCoStuff (void)
{
	line_at (20, 0, 132);
	print_at (21, 0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (21, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0,  ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);
}

int
heading (
 int scn)
{
	crsr_off ();
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		clear ();

		if (SEL_PO)
			rv_pr (ML (mlPoMess040), 42, 0, 1);
		else
			rv_pr (ML (mlPoMess041), 42, 0, 1);
		print_at (0, 94, ML (mlPoMess007), local_rec.previousSupp, local_rec.previousPo);
		line_at (1, 0, 132);

		switch (scn)
		{
		case	1:

			scn_set (2);
			scn_write (2);
			scn_display (2);

			box (0, 3, 130, 15);
			line_at (6, 1, 129);
			break;

		case	2:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (0, 3, 130, 15);
			line_at (6, 1, 129);
		}

		scn_set (scn);
		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		crsr_on ();
	}
    return (EXIT_SUCCESS);
}

/*
 * Check if item exists on sales order. 
 */
int
ValidItem (void)
{
	soln_rec.hhso_hash = sohr_rec.hhso_hash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && sohr_rec.hhso_hash == soln_rec.hhso_hash)
	{
		if (soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
			return (TRUE);
	
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (FALSE);
}
