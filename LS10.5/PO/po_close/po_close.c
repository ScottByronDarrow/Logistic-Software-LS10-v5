/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_close.c,v 5.8 2002/05/02 01:36:57 scott Exp $
|  Program Name  : (po_close.c)
|  Program Desc  : (Purchase Order Close/Open)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
| $Log: po_close.c,v $
| Revision 5.8  2002/05/02 01:36:57  scott
| Updated to add Archive functions
|
| Revision 5.7  2002/04/30 07:57:35  scott
| Update for new Archive modifications;
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_close.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_close/po_close.c,v 5.8 2002/05/02 01:36:57 scott Exp $";

#define	MAXWIDTH	80
#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_po_mess.h>
#include 	<ml_std_mess.h>
#include 	<DeleteControl.h>
#include 	<Archive.h>

#define		DEL_STS 	(poln_rec.pur_status [0] == 'D')
#define		DEL_QTY	 	(poln_rec.qty_ord - poln_rec.qty_rec <= 0.00)
#define		DEL_PC(pp, ord, rec)   (((pp / 100) * ord) <=  rec)

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct pohrRecord	pohr2_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct posdRecord	posd_rec;
struct inmrRecord	inmr_rec;
struct poslRecord	posl_rec;
struct pogdRecord	pogd_rec;
struct poliRecord	poli_rec;
struct poglRecord	pogl_rec;

	char	*pohr2	=	"pohr2", 
			*poln2	=	"poln2", 
			branchNumber [3];

	int		automaticClose 	= FALSE, 
			envVarCrCo 		= FALSE, 
			envVarCrFind 	= FALSE, 
			printerNumber 	= 1, 
			supplierFlag 	= TRUE, 
			firstPoln 		= TRUE, 
			recordFound 	= FALSE, 
			firstFlag 		= FALSE, 
			firstTime 		= TRUE, 
			poDetails 		= FALSE;

	float	oqty 			= 0.0, 
			rqty			= 0.0, 
			tot_po_oqty 	= 0.0, 
			tot_po_rqty 	= 0.0, 
			supp_tot_oqty 	= 0.0, 
			supp_tot_rqty 	= 0.0, 
			grand_tot_oqty 	= 0.0, 
			grand_tot_rqty 	= 0.0;

	double	extend 			 = 0.00, 
			tot_po_shipmt 	 = 0.00, 
			supp_tot_shipmt  = 0.00, 
			grand_tot_shipmt = 0.00, 
			envVarPoClosePc	 = 0.00;

	FILE	*fout;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	previousPoNumber [16];
	char	previousSupplier [7];
	char 	supplierNumber [7];
	char 	status [15];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNo", 	 4, 16, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier.", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.supplierNumber}, 
	{1, LIN, "name", 	 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "porder", 	 5, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "P/Order No.", " ", 
		YES, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{1, LIN, "con_name", 	 6, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Contact name.", "<RETURN> - standard contact ", 
		 NA, NO,  JUSTLEFT, "", "", pohr_rec.contact}, 
	{1, LIN, "torder", 	 7, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Terms of Order.", "ex works/FAS/FOB/CNF/C&F/CIF.", 
		 NA, NO,  JUSTLEFT, "", "", pohr_rec.term_order}, 
	{1, LIN, "btpay", 	 9, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Bank Terms.", "", 
		 NA, NO,  JUSTLEFT, "", "", pohr_rec.bnk_term_pay}, 
	{1, LIN, "stpay", 	10, 16, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Supplier Terms.", "", 
		 NA, NO,  JUSTLEFT, "", "", pohr_rec.sup_term_pay}, 
	{1, LIN, "pay_date", 	10, 65, EDATETYPE, 
		"NN/NN/NN", "          ", 
		" ", "", " Pay By.", " ", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.pay_date}, 
	{1, LIN, "ship1", 	12, 16, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "Shipment No 1.", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&pohr_rec.ship1_no}, 
	{1, LIN, "date_raised", 	12, 65, EDATETYPE, 
		"NN/NN/NN", "          ", 
		" ", "", " Raised.", " ", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.date_raised}, 
	{1, LIN, "ship2", 	13, 16, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "Shipment No 2.", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&pohr_rec.ship2_no}, 
	{1, LIN, "date_conf", 	13, 65, EDATETYPE, 
		"NN/NN/NN", "          ", 
		" ", "", " Confirmed.", " ", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.conf_date}, 
	{1, LIN, "ship3", 	14, 16, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "Shipment No 3.", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&pohr_rec.ship3_no}, 
	{1, LIN, "date_reqd", 	14, 65, EDATETYPE, 
		"NN/NN/NN", "          ", 
		" ", "", " Required.", " ", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.due_date}, 
	{1, LIN, "status", 	16, 16, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Order Status.", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.status}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<proc_sobg.h>
#include 	<FindSumr.h>

/*
 * Function Declarations 
 */
int 	spec_valid 		(int);
int 	StatusReply 	(void);
int 	heading 		(int);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Update 			(void);
void 	DeletePosl 		(long);
void 	DeletePoli 		(long);
void 	PrintSupplier 	(void);
void 	ShowPohr 		(char *);
void 	AutoDelete 		(void);
void 	ProcessPohr 	(long);
void 	PrintPoln 		(void);
void 	DeletePoln 		(long);
void 	DetetePohr 		(long);
void 	HeadingOutput 	(void);
void 	PrintLine 		(void);
void 	TotalLine 		(int);
void 	PrintCompany 	(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "po_close"))
		automaticClose = FALSE;

	if (!strcmp (sptr, "po_aclose"))
		automaticClose = TRUE;

	if (argc > 1)
		printerNumber = atoi (argv [ 1 ]);
	

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	envVarCrCo = atoi (get_env ("CR_CO"));
	envVarCrFind = atoi (get_env ("CR_FIND"));

	/*
	 * Open main database files. 
	 */
	OpenDB ();

	/*
	 * Check for Purchase order close percent variable.
	 */
	sptr = chk_env ("PO_CLOSE_PC");
	envVarPoClosePc = (sptr == (char *)0) ? 0.00 : atof (sptr);

	strcpy (branchNumber, (!envVarCrCo) ? " 0" : comm_rec.est_no);

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "PO_RECEIPT-CLOSE");
	if (!cc)
	{
		envVarPoClosePc		= (float) delhRec.purge_days;
	}
	strcpy (local_rec.previousPoNumber, "000000000000000");
	strcpy (local_rec.previousSupplier, "000000");

	if (automaticClose)
	{
		AutoDelete ();
		fprintf (fout, ".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	while (prog_exit == 0)
	{
		eoi_ok 		= TRUE;
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 & 2 & 3  input. 
		 */
		heading (1);
		scn_display (1);
		if (StatusReply ())
			Update ();
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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (pohr2, pohr);
	abc_alias (poln2, poln);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, 
							(!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
 
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_hhpo_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, 
							(automaticClose) ? "pohr_hhsu_hash" : "pohr_id_no");
	open_rec (pohr2, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poln2, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no3");
	open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_hhpl_hash");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (posd);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posl);
	abc_fclose (poln2);
	abc_fclose (inmr);
	abc_fclose (posl);
	abc_fclose (pogd);
	abc_fclose (poli);
	abc_fclose (pogl);
	ArchiveClose ();
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*
	 * Validate Creditor Number. 
	 */
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNumber));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*
			 * Supplier not found. 
			 */
			errmess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Purchase Order Number. 
	 */
	if (LCHECK ("porder"))
	{
		if (SRCH_KEY)
		{
			ShowPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		/*
		 * Check if order is on file. 
		 */
		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));
		strcpy (pohr_rec.type, "O");
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "w");

		/*
		 * Order not on file. 
		 */
		if (cc) 
		{
			/*
			 * Order not found. 
			 */
			print_mess (ML (mlStdMess122));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.status, "%14.14s", "OPEN");
		if (pohr_rec.status [0] == 'D')
			sprintf (local_rec.status, "%14.14s", "CLOSED");

		if (pohr_rec.status [0] == 'C' || pohr_rec.status [0] == 'c')
			sprintf (local_rec.status, "%14.14s", "COSTED");

		if (pohr_rec.status [0] == 'R' || pohr_rec.status [0] == 'r')
			sprintf (local_rec.status, "%14.14s", "GOODS RECEIVED");

		if (pohr_rec.status [0] == 'U')
			sprintf (local_rec.status, "%14.14s", "UNAPPROVED P/O");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
StatusReply (
 void)
{
	int		i;

	move (1, 2);
	cl_line ();
	/*
	 * Close and Delete Purchase Order. ? 
	 */
	i = prmptmsg (ML (mlPoMess090), "YyNn", 1, 2);
	move (1, 2);
	cl_line ();
	if (i != 'Y' && i != 'y')
		return (FALSE);

	if ( pohr_rec.status [0] == 'D' ||  pohr_rec.status [0] == 'C' ||
		 pohr_rec.status [0] == 'O' ||  pohr_rec.status [0] == 'U' ||
		 pohr_rec.status [0] == 'H' ||  pohr_rec.status [0] == 'X' ||
		 pohr_rec.status [0] == 'c')
		return (TRUE);
	
	errmess (ML ("Status Purchase order must be [D][U][O][C][H]or[X] to delete"));
	sleep (sleepTime);
	return (FALSE);
}

void
Update (void)
{
	move (1, 2);
	cl_line ();

	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = 0; 

	cc = find_rec (poln, &poln_rec, GTEQ, "u");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		DeletePosl (poln_rec.hhpl_hash);
		DeletePoli (poln_rec.hhpl_hash);

		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
			poln_rec.hhbr_hash, 
			poln_rec.hhcc_hash, 
			0L, 
			0.00
		);
		cc = ArchivePoln (poln_rec.hhpl_hash);
		if (cc)
			file_err (cc, poln, "ARCHIVE");

		abc_delete (poln);

		poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
		poln_rec.line_no = 0; 
		cc = find_rec (poln, &poln_rec, GTEQ, "u");
	}
	abc_unlock (poln);
	
	posd_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
	cc = find_rec (posd, &posd_rec, GTEQ, "u");
	while (!cc && posd_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		abc_delete (posd);

		cc = find_rec (posd, &posd_rec, GTEQ, "u");
	}
	abc_unlock (posd);
	    	
	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
	pogd_rec.line_no   = 0;
	cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
	while (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) && 
				  pogd_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		abc_delete (pogd);

		strcpy (pogd_rec.co_no, comm_rec.co_no);
		pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
		pogd_rec.line_no   = 0;
		cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
	}
	abc_unlock (pogd);

	cc = ArchivePohr (pohr_rec.hhpo_hash);
	if (cc)
		file_err (cc, pohr, "ARCHIVE");

	cc = abc_delete (pohr);
	if (cc)
		file_err (cc, pohr, "DBDELETE");

	sprintf (local_rec.previousPoNumber, "%-15.15s", pohr_rec.pur_ord_no);
	sprintf (local_rec.previousSupplier, "%-6.6s", sumr_rec.crd_no);

	recalc_sobg ();
}

void
DeletePosl (
	long	hhplHash)
{
	posl_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (posl, &posl_rec, GTEQ, "u");
	while (!cc && hhplHash == posl_rec.hhpl_hash)
	{
		abc_delete (posl);
		cc = find_rec (posl, &posl_rec, GTEQ, "u");
	}
	abc_unlock (posl);
}

void
DeletePoli (
	long	hhplHash)
{
	poli_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poli, &poli_rec, GTEQ, "u");
	while (!cc && poli_rec.hhpl_hash == hhplHash)
	{
		abc_delete (poli);
		cc = find_rec (poli, &poli_rec, GTEQ, "u");
	}
	abc_unlock (poli);
}

/*
 * Print Supplier.
 */
void
PrintSupplier (void)
{
	fprintf (fout, ".LRP3\n");
	if (!firstFlag)
		PrintLine ();

	fprintf (fout, "! %6.6s (%9.9s)  ",  sumr_rec.crd_no, sumr_rec.acronym);
	fprintf (fout, "(%-40.40s)             ", sumr_rec.crd_name);
	fprintf (fout, "                                           ");
	fprintf (fout, "           ");
	fprintf (fout, "               !\n");

	TotalLine (FALSE);
	supplierFlag = FALSE;
	firstFlag = FALSE;
}

/*
 * Search and Display purchase order display. 
 */
void
ShowPohr (
	char	*keyValue)
{
	_work_open (16,0,20);
	save_rec ("#P/O Number     ", "#Contact Name");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && !strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue)) && 
		      !strcmp (pohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] == 'O')
		{
		    	cc = save_rec (pohr_rec.pur_ord_no, pohr_rec.contact);
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
 * Process Automatic deletions. 
 */
void
AutoDelete (void)
{
	dsp_screen ("Purchase order Automatic deletion", 
					comm_rec.co_no, comm_rec.co_name);

	HeadingOutput ();

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, "  ");
	strcpy (sumr_rec.crd_no, "      ");
	strcpy (sumr_rec.acronym, "         ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		ProcessPohr (sumr_rec.hhsu_hash);

		/*
		 * Print supplier totals.	
		 */
		if (recordFound)
		{
			PrintLine ();
			fprintf (fout, ".LRP3\n");
			fprintf (fout, "!  TOTAL FOR SUPPLIER                 ");
			fprintf (fout, "!%9.2f ", supp_tot_oqty);
			fprintf (fout, "!%9.2f ", supp_tot_rqty);
			fprintf (fout, "!                  ");
			fprintf (fout, "!                      ");
			fprintf (fout, "                    ");
			fprintf (fout, "!          ");
			fprintf (fout, "!%12.2f !\n", supp_tot_shipmt);
			recordFound = FALSE;
			supp_tot_oqty = 0.0;
			supp_tot_rqty = 0.0;
			supp_tot_shipmt = 0.0;
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
		supplierFlag = TRUE;
	}
	/*
	 * On last record, print the total backlog.	
	 */
	PrintLine ();
	fprintf (fout, "!  TOTAL PURCHASE ORDERS              ");
	fprintf (fout, "!%9.2f ", grand_tot_oqty);
	fprintf (fout, "!%9.2f ", grand_tot_rqty);
	fprintf (fout, "!                  ");
	fprintf (fout, "!                        ");
	fprintf (fout, "                  ");
	fprintf (fout, "!          ");
	fprintf (fout, "!%12.2f !\n", grand_tot_shipmt);
	
	fprintf (fout, ".EOF\n");
	recalc_sobg ();
}

/*
 * Process purchase order header file using supplier serial hash sumr_hhsu_hash
 */
void
ProcessPohr (
	long	hhsuHash)
{
	tot_po_oqty 	= 0.0;
	tot_po_rqty 	= 0.0;
	tot_po_shipmt 	= 0.0;
	poDetails 	= 0;

	pohr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && pohr_rec.hhsu_hash == hhsuHash)
	{
		firstTime = TRUE;
		firstPoln = TRUE;
		poDetails = 0;
		poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
		poln_rec.line_no = 0; 

		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
		    if (DEL_STS || DEL_QTY || 
				DEL_PC (envVarPoClosePc, poln_rec.qty_ord, poln_rec.qty_rec))
		    {
				DeletePoln (poln_rec.hhpl_hash);

				dsp_process ("Supplier No.", sumr_rec.crd_no);

				if (supplierFlag)
					PrintSupplier ();
		
				if (!poDetails)
				{
					fprintf (fout, "!%15.15s", pohr_rec.pur_ord_no);
					fprintf (fout, "!%10.10s", (pohr_rec.date_raised == 0L) ? "          " : DateToString (pohr_rec.date_raised));
					fprintf (fout, "!%10.10s", (pohr_rec.due_date == 0L) ? "          " : DateToString (pohr_rec.due_date));
				}
				if (firstTime == TRUE)
				{
					firstTime = FALSE;
					firstPoln = TRUE;

				}
		    	PrintPoln ();
		    }
		    cc = find_rec (poln, &poln_rec, NEXT, "r");
		}

		/*
		 * If more than one line for a p.o., print total for det lines. 
		 */
		if (poDetails > 0)
		{
			fprintf (fout, ".LRP2\n");
			TotalLine (TRUE);
			fprintf (fout, "!  TOTAL FOR P.O.          !          ");
			fprintf (fout, "!%9.2f ", tot_po_oqty);
			fprintf (fout, "!%9.2f ", tot_po_rqty);
			fprintf (fout, "!                  ");
			fprintf (fout, "!                      ");
			fprintf (fout, "                    ");
			fprintf (fout, "!          ");
			fprintf (fout, "!%12.2f !\n", tot_po_shipmt);
			tot_po_oqty = 0.00;
			tot_po_rqty = 0.00;
			tot_po_shipmt = 0.00;
			TotalLine (FALSE);
		}
		else if (poDetails)
		{
			fprintf (fout, ".LRP2\n");
			fprintf (fout, "!               !          !          ");
			fprintf (fout, "!          ");
			fprintf (fout, "!          ");
			fprintf (fout, "!                  ");
			fprintf (fout, "!                      ");
			fprintf (fout, "                    ");
			fprintf (fout, "!          ");
			fprintf (fout, "!             !\n");
		}

		DetetePohr (pohr_rec.hhpo_hash);

		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
}

/*
 * Print purchase order line. 
 */
void
PrintPoln (void)
{
	recordFound = TRUE;
	
	inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		sprintf (inmr_rec.item_no, "%-16.16s", " ");

	oqty = poln_rec.qty_ord;
	rqty = poln_rec.qty_rec;

	extend = (double) rqty;
	extend *= out_cost (poln_rec.land_cst, inmr_rec.outer_size);

	tot_po_oqty 	 += oqty;
	tot_po_rqty 	 += rqty;
	tot_po_shipmt 	 += extend;
	supp_tot_oqty 	 += oqty;
	supp_tot_rqty 	 += rqty;
	supp_tot_shipmt  += extend;
	grand_tot_oqty 	 += oqty;
	grand_tot_rqty 	 += rqty;
	grand_tot_shipmt += extend;

	if (!firstPoln)
		fprintf (fout, "!               !          !          ");

	fprintf (fout, "!%9.2f ", oqty);
	fprintf (fout, "!%9.2f ", rqty);
	fprintf (fout, "! %-16.16s ", inmr_rec.item_no);
	fprintf (fout, "! %-40.40s ", poln_rec.item_desc);
	fprintf (fout, "!%10.10s", (poln_rec.due_date == 0L) 
			     ? "          " : DateToString (poln_rec.due_date));

	fprintf (fout, "!%12.2f !\n", extend);
	firstPoln = FALSE;

	poDetails++;
}
/*
 * Delete purchase order lines and relevent information. 
 */
void
DeletePoln (
	long	hhplHash)
{
	poln2_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (poln2, &poln2_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (poln2);
		return;
	}

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0, 
		poln2_rec.hhbr_hash, 
		poln2_rec.hhcc_hash, 
		0L, 
		0.00
	);
	DeletePosl (poln2_rec.hhpl_hash);
	DeletePoli (poln2_rec.hhpl_hash);
	pogl_rec.hhpl_hash	=	poln2_rec.hhpl_hash;
	cc = find_rec (pogl, &pogl_rec, COMPARISON, "r");
	if (!cc)
		poln2_rec.due_date	=	pogl_rec.rec_date;

	cc = ArchivePoln (poln2_rec.hhpl_hash);
	if (cc)
		file_err (cc, poln2, "ARCHIVE");

	cc = abc_delete (poln2);
	if (cc)
		file_err (cc, poln2, "DBDELETE");
}

/*
 * Delete purchase order header and relevent information. 
 */
void
DetetePohr (
	long	hhpoHash)
{
	poln_rec.hhpo_hash 	= hhpoHash;
	poln_rec.line_no 	= 0;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	if (!cc && poln_rec.hhpo_hash == hhpoHash)
		return;

	pohr2_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec (pohr2, &pohr2_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pohr2);
		return;
	}
	cc = ArchivePohr (pohr2_rec.hhpo_hash);
	if (cc)
		file_err (cc, pohr2, "ARCHIVE");

	cc = abc_delete (pohr2);
	if (cc)
		file_err (cc, pohr2, "DBDELETE");

	posd_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec (posd, &posd_rec, GTEQ, "u");
	while (!cc && posd_rec.hhpo_hash == hhpoHash)
	{
	    abc_delete (posd);
		cc = find_rec (posd, &posd_rec, GTEQ, "u");
	}
	abc_unlock (posd);
	return; 
}

/*
 * Start Out Put To Standard Print 
 */
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);

	fprintf (fout, ".15\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n", 
				comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  : %s - %s\n", 
				comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EPURCHASE ORDER AUTOMATIC DELETION AUDIT.\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===========");
	fprintf (fout, "===============\n");

	fprintf (fout, "================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===========");
	fprintf (fout, "===============\n");

	fprintf (fout, "!     P.O.      ");
	fprintf (fout, "!   DATE   ");
	fprintf (fout, "!   DATE   ");
	fprintf (fout, "! QUANTITY ");
	fprintf (fout, "! QUANTITY ");
	fprintf (fout, "!       PART       ");
	fprintf (fout, "!               DESCRIPTION                ");
	fprintf (fout, "!   DATE   ");
	fprintf (fout, "!    LANDED   !\n");

	fprintf (fout, "!    NUMBER     ");
	fprintf (fout, "! ORDERED  ");
	fprintf (fout, "!   DUE    ");
	fprintf (fout, "! ORDERED  ");
	fprintf (fout, "! RECEIVED ");
	fprintf (fout, "!       NUMBER     ");
	fprintf (fout, "!                                          ");
	fprintf (fout, "!   DUE    ");
	fprintf (fout, "!     COST    !\n");
	PrintLine ();
	firstFlag = TRUE;
}

void
PrintLine (void)
{
	fprintf (fout, "!---------------");
	fprintf (fout, "!----------");
	fprintf (fout, "!----------");
	fprintf (fout, "!----------");
	fprintf (fout, "!----------");
	fprintf (fout, "!------------------");
	fprintf (fout, "!------------------------------------------");
	fprintf (fout, "!----------");
	fprintf (fout, "!-------------!\n");

}

void
TotalLine (
	int		printTotal)
{
	fprintf (fout, "!               ");
	fprintf (fout, "!          ");
	fprintf (fout, "!          ");
	fprintf (fout, "!          ");
	fprintf (fout, (printTotal) ? "!----------" : "!          ");
	fprintf (fout, "!                  ");
	fprintf (fout, "!                                          ");
	fprintf (fout, "!          ");
	fprintf (fout, (printTotal) ? "!-------------!\n" : "!             !\n");

}

int
heading (
 int scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	/*
	 * P. Order Status Maintenance. 
	 */
	rv_pr (ML (mlPoMess091), 15, 0, 1);

	print_at (0, 55, ML ("Last P/O %s"), local_rec.previousPoNumber);
	line_at (1, 0, 80);

	box (0, 3, 80, 13);
	line_at (8, 1, 79);

	us_pr (ML (mlPoMess087), 5, 8, 1);
	line_at (11, 1, 79);
	us_pr (ML (mlPoMess092), 5, 11, 1);
	line_at (15, 1, 79);

	PrintCompany ();
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
PrintCompany (void)
{
	line_at (20, 0, 132);
	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (21,50, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);
}
