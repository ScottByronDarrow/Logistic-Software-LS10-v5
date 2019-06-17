/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_ar_disp.c,v 5.0 2002/05/07 10:18:34 scott Exp $
|  Program Name  : (po_ar_disp.c)
|  Program Desc  : (Archive - Supplier Backlog Display)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 1st May 2002     |
|---------------------------------------------------------------------|
| $Log: po_ar_disp.c,v $
| Revision 5.0  2002/05/07 10:18:34  scott
| Updated to bring version number to 5.0
|
| Revision 1.1  2002/05/02 01:31:00  scott
| New program for archiving
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_ar_disp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_ar_disp/po_ar_disp.c,v 5.0 2002/05/07 10:18:34 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#define	SAME_PO	 (strcmp (previousPorder, arpohr_rec.pur_ord_no))

#include	"schema"

struct commRecord	comm_rec;
struct arpohrRecord	arpohr_rec;
struct arpolnRecord	arpoln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;

	char 	*data			= 	"data", 
			*fifteenSpaces 	= 	"               ", 
			*ser_space 		= 	"                         ";

	extern	int		lp_x_off, 
					lp_y_off;

	char	headingString [200];

	int		envVarCrCo 		= 0, 
			envVarCrFind	= 0;

	char	branchNumber [3];
	char	previousPorder [sizeof arpohr_rec.pur_ord_no];

	float	orderQty = 0.00,
			totalQty = 0.00;

	double	orderAmt = 0.00,
			totalAmt = 0.00;

/*
 * Local & Screen Structure  
 */
struct {
	char	purchasesOrder [sizeof arpohr_rec.pur_ord_no];
	char	dummy [11];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier No         ", " ", 
		YES, NO, JUSTLEFT, "", "", sumr_rec.crd_no}, 
	{1, LIN, "name", 	 4, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Supplier Name       ", " ", 
		 NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "purchasesOrder", 	 5, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "ALL", "Purchase Order No   ", "Please enter purchase order or <return> for ALL.", 
		YES, NO, JUSTLEFT, "", "", local_rec.purchasesOrder}, 
	{0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<get_lpno.h>
#include	<FindSumr.h>
/*
 * Function Declarations 
 */
void 	CloseDB 		(void);
void 	DisplayHeading 	(void);
void 	OpenDB 			(void);
void 	Process 		(void);
void 	PrintTotal 		(int);
void 	SrchPohr 		(char *);
void 	shutdown_prog 	(void);
void 	PrintLine 		(long);
int 	heading 		(int);
int 	spec_valid 		(int);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr 	();
	clear 		();
	set_tty 	();
	set_masks 	();

	sptr = chk_env ("CR_CO");
	envVarCrCo = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("CR_FIND");
	envVarCrFind = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		totalQty = 0.00, 
		totalAmt = 0.00;

		/*
		 * Reset Control Flags 
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Entry Screen Input  
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
	
		/*
		 * Edit Screen Input  
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		Process ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inum, 	inum_list,		INUM_NO_FIELDS, 	"inum_hhum_hash");
	open_rec (arpohr, 	arpohr_list, 	ARPOHR_NO_FIELDS, 	"arpohr_hhsu_hash");
	open_rec (arpoln, 	arpoln_list, 	ARPOLN_NO_FIELDS, 	"arpoln_id_no");
	open_rec (inmr, 	inmr_list, 		INMR_NO_FIELDS, 	"inmr_hhbr_hash");
	open_rec (sumr, 	sumr_list, 		SUMR_NO_FIELDS, (!envVarCrFind) 
											? "sumr_id_no" : "sumr_id_no3");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (inum);
	abc_fclose (sumr);
	abc_fclose (arpohr);
	abc_fclose (arpoln);
	abc_fclose (inmr);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	if (LCHECK ("supplierNumber"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (sumr_rec.crd_no,6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("supplierNumber");
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("purchasesOrder"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.purchasesOrder, 	"%-15.15s", "ALL");
			sprintf (arpohr_rec.pur_ord_no, 	"%-15.15s", "ALL");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		abc_selfield (arpohr, "arpohr_id_no");

		strcpy (arpohr_rec.co_no, comm_rec.co_no);
		strcpy (arpohr_rec.br_no, comm_rec.est_no);
		arpohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (arpohr_rec.pur_ord_no, zero_pad (local_rec.purchasesOrder, 15));
		cc = find_rec (arpohr, &arpohr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		abc_selfield (arpohr, "arpohr_hhsu_hash");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Process (void)
{
	DisplayHeading ();
	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open 
	(
		0, 
		2, 
		13, 
		headingString, 
		comm_rec.co_no, 
		comm_rec.co_name, 
		comm_rec.est_no, 
		comm_rec.est_name, 
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec ("PURCHASE ORDER |BR| RECEIPT  |UOM.| QUANTITY | QUANTITY |   EST VALUE   |      ITEM      |          ITEM DESCRIPTION.            ");
	Dsp_saverec ("    NUMBER     |NO|   DATE   |    |   ORDER  | SUPPLIED |  P/O  LINE.   |     NUMBER     |                                       ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");

	strcpy (previousPorder, fifteenSpaces);

	/*
	 * Display details for a single purchase order. 
	 */
	if (strcmp (local_rec.purchasesOrder, "ALL            "))
	{
		PrintLine (arpohr_rec.hhpo_hash);
		PrintTotal (FALSE);
		Dsp_srch ();
		Dsp_close ();
		return;
	}
	
	/*
	 * Display details for all purchase order for a supplier. 
	 */
	arpohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec (arpohr, &arpohr_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == arpohr_rec.hhsu_hash) 
	{
		PrintLine (arpohr_rec.hhpo_hash);
		cc = find_rec (arpohr, &arpohr_rec, NEXT, "r");
	}
	PrintTotal (FALSE);
	Dsp_srch ();
	Dsp_close ();
}

/*
 * Heading routine. 
 */
void
DisplayHeading (void)
{
	clear ();
	crsr_off ();
	sprintf 
	(
		headingString, 
		" P/O : %s  / Supplier %s %s / Currency %s / Country = %s", 
		arpohr_rec.pur_ord_no, 
		sumr_rec.crd_no, 
		sumr_rec.crd_name, 
		sumr_rec.curr_code, 
		sumr_rec.ctry_code
	);
	
	print_at 
	(
		0, 
		3, 
		ML (mlPoMess064), 
		arpohr_rec.pur_ord_no, 
		sumr_rec.crd_no, 
		sumr_rec.crd_name
	);

	print_at 
	(
		0, 
		57, 
		ML (mlPoMess133), 
		sumr_rec.curr_code, 
		sumr_rec.ctry_code
	);

	line_at (1,3,130);
	line_at (21,0,132);
	
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (23, 0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
}

/*
 * Print purchase order lines. 
 */
void
PrintLine (
	long	hhpoHash)
{
	char	dspString [200], 
			receiptDate [11], 
			workQty [2] [12], 
			serial [25], 
			poString [25];

	int		printed = FALSE;

	double	balance 	= 0.0, 
			qtyOrdered 	= 0.0, 
			qtySupplied = 0.0;

	float	StdCnvFct 	= 1.00, 
			PurCnvFct 	= 1.00, 
			CnvFct		= 1.00;

	orderQty = 0.00;
	orderAmt = 0.00;

	arpoln_rec.hhpo_hash	=	hhpoHash;
	arpoln_rec.line_no		=	0;
	cc = find_rec (arpoln, &arpoln_rec, GTEQ, "r");
	while (!cc && arpoln_rec.hhpo_hash == hhpoHash) 
	{
		qtySupplied = (double) arpoln_rec.qty_ord;
		qtyOrdered 	= (double) arpoln_rec.qty_ord - 
								(arpoln_rec.qty_ord - arpoln_rec.qty_rec);

		inmr_rec.hhbr_hash	=	arpoln_rec.hhbr_hash;
		if (find_rec (inmr, &inmr_rec, EQUAL, "r"))
		{
			strcpy (inmr_rec.item_no, 	 " ");
			strcpy (inmr_rec.description, " ");
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	arpoln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;
		
		balance = twodec (arpoln_rec.land_cst);
		balance = out_cost (balance, inmr_rec.outer_size);
		balance *= qtySupplied;

		orderQty += (float) (qtySupplied	* CnvFct);
		totalQty += (float) (qtySupplied * CnvFct);
		orderAmt += balance;
		totalAmt += balance;

		strcpy (receiptDate, DateToString (arpoln_rec.due_date));

		if (inmr_rec.serial_item [0] == 'Y') 
			sprintf (serial, "%-25.25s", arpoln_rec.serial_no);
		else
			strcpy (serial, ser_space);

		if (qtySupplied != 0.00)
			sprintf (workQty [0], "%9.2f ", qtySupplied * CnvFct);
		else
			strcpy (workQty [0], "          ");

		if (qtyOrdered != 0.00)
			sprintf (workQty [1], "%9.2f ", qtyOrdered * CnvFct);
		else
			strcpy (workQty [1], "          ");
		
		if (SAME_PO)
			sprintf (poString, "^1%s^6", arpohr_rec.pur_ord_no);
		else
			sprintf (poString, "%s", fifteenSpaces);

		sprintf 
		(
			dspString, "%s^E%2.2s^E%10.10s^E%4.4s^E%10.10s^E%10.10s^E%14.14s ^E%-16.16s^E%-40.40s",
			poString, 
			arpohr_rec.br_no, 
			receiptDate, 
			inum_rec.uom, 
			workQty [1], workQty [0], 
			comma_fmt (balance, "NNN,NNN,NNN.NN"), 
			inmr_rec.item_no, 
			inmr_rec.description
		);
		Dsp_saverec (dspString);

		if (strcmp (serial, ser_space))
		{
			sprintf (dspString, "               ^E  ^E          ^E    ^E          ^E          ^E               ^E  Serial Number ^E %25.25s", 
					serial);

			Dsp_saverec (dspString);
		}
		printed = TRUE;

		strcpy (previousPorder, arpohr_rec.pur_ord_no);

		cc = find_rec (arpoln, &arpoln_rec, NEXT, "r");
	}
	if (printed)
		PrintTotal (TRUE);

	return;
}

void
PrintTotal (
 int end_tot)
{
	char	dspString [200];
  	sprintf (dspString, " %s         ^E    ^E          ^E%9.2f ^E%14.14s ^6^E                ^E",
		 (end_tot) ? "*** ORDER TOTAL ***" : "^1*** GRAND TOTAL ***", 
		 (end_tot) ? orderQty : totalQty, 
		 (end_tot) ? comma_fmt (orderAmt, "NNN,NNN,NNN.NN") 
		 	 : comma_fmt (totalAmt, "NNN,NNN,NNN.NN"));

	Dsp_saverec (dspString);

	if (end_tot)
		Dsp_saverec ("^^GGGGGGGGGGGGGGGEGGEGGGGGGGGGGEGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	else
		Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGJGGGGGGGGGGJGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	orderQty = 0.00;
	orderAmt = 0.00;
}


void
SrchPohr (
	char	*keyValue)
{
	_work_open (15, 0, 10);
	abc_selfield (arpohr, "arpohr_id_no");
	strcpy (arpohr_rec.co_no, comm_rec.co_no);
	strcpy (arpohr_rec.br_no, comm_rec.est_no);
	arpohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (arpohr_rec.pur_ord_no, "%-15.15s", keyValue);
	save_rec ("#Purchase Order ", "#Date      "); 
	cc = find_rec (arpohr, &arpohr_rec, GTEQ, "r");
	while (!cc && !strcmp (arpohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (arpohr_rec.br_no, comm_rec.est_no) && 
		      !strncmp (arpohr_rec.pur_ord_no, keyValue, strlen (keyValue)))
	{
		if (arpohr_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			strcpy (err_str, DateToString (arpohr_rec.date_raised));
			cc = save_rec (arpohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (arpohr, &arpohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (arpohr_rec.co_no, comm_rec.co_no);
	strcpy (arpohr_rec.br_no, comm_rec.est_no);
	arpohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (arpohr_rec.pur_ord_no, "%-15.15s", temp_str);
	cc = find_rec (arpohr, &arpohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, arpohr, "DBFIND");

	abc_selfield (arpohr, "arpohr_hhsu_hash");
}

int
heading (
	int	scn)
{
	swide ();
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	strcpy (err_str, ML ("Archive Purchase Order Display."));
	rv_pr (err_str, (130 - strlen (err_str)) / 2, 0, 1);

	box (0, 3, 130, 2);
	line_at (1, 0, 132);
	line_at (21, 0, 132);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (23, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
