/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_whmaint.c,v 5.6 2002/04/11 03:46:23 scott Exp $
|  Program Name  : (sk_whmaint.c)
|  Program Desc  : (Add or Update Wharehouse Stock Items)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sk_whmaint.c,v $
| Revision 5.6  2002/04/11 03:46:23  scott
| Updated to add comments to audit files.
|
| Revision 5.5  2001/12/10 07:19:08  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_whmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_whmaint/sk_whmaint.c,v 5.6 2002/04/11 03:46:23 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <DBAudit.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct lomrRecord	lomr_rec;
struct ineiRecord	inei_rec;
struct inccRecord	incc_rec;
struct qcmrRecord	qcmr_rec;

	char	*data	= "data";

	char	*scn_desc [] = {
					" Item Maintenance Screen ",
					" Manufacturing Screen "
					};

/*
 * Special fields and flags.
 */
int		new_item = 0;
int		envMultLoc = FALSE;
int		envQcApply = FALSE;
int		manItem = FALSE;
char	tempSort [29];

extern	int		TruePosition;

/*=============================
| Local & Screen Structures . |
=============================*/
struct
{
	char	dummy [11];
	char	previousItem [17];
	char	itemNo [17];
	char	allowRepl [2];
	char	allowReplDesc [11];
	char	dfltQty [15];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNo",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "Item Number          ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.itemNo},
	{1, LIN, "acronym",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "", "Item Acronym         ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.alpha_code},
	{1, LIN, "desc",	 	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description          ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "stockunit",	 7, 2, CHARTYPE,
		"UUUU", "          ",
		" ", inmr_rec.sale_unit, "Stock Unit           ", " ",
		 NO, NO,  JUSTLEFT, "", "", incc_rec.stocking_unit},
	{1, LIN, "location",	 7, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Stock Location       ", "Enter the default stocking location.",
		 NO, NO,  JUSTLEFT, "", "", incc_rec.location},
	{1, LIN, "option",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Forecast Option      ", "A(utomatic M(anual P(redetermined ",
		YES, NO,  JUSTLEFT, "AMP", "", incc_rec.ff_option},
	{1, LIN, "method",	10, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Forecast Method      ", " ",
		YES, NO,  JUSTLEFT, "", "", incc_rec.ff_method},
	{1, LIN, "allowRepl",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "E", "Allow Replenishment  ", "Enter I(nclude item in replenishment report) E(xclude from report.) ",
		YES, NO,  JUSTLEFT, "IE", "", local_rec.allowRepl},
	{1, LIN, "allowReplDesc",	11, 26, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.allowReplDesc},
	{1, LIN, "wks_demand",	12, 2, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Weeks Demand         ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&incc_rec.wks_demand},
	{1, LIN, "s_stock",	13, 2, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "0.00", "Safety Stock         ", " Weeks ",
		YES, NO, JUSTRIGHT, "", "", (char *)&incc_rec.safety_stock},
	{1, LIN, "qcTime",	9, 40, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "QC Check Time (Wk)   ", "Quarantine Check Time In Weeks",
		YES, NO,  JUSTRIGHT, "0", "999.99", (char *) &incc_rec.qc_time},
	{1, LIN, "qcCentre",	10, 40, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "QC Centre            ", "Quarantine Centre",
		YES, NO,  JUSTLEFT, "", "", incc_rec.qc_centre},
	{1, LIN, "qcLocation",	11, 40, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "QC Stock Location    ", "Quarantine Default Stock Location",
		YES, NO,  JUSTLEFT, "", "", incc_rec.qc_location},
	{1, LIN, "lead_time",	12, 40, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Lead Time    (Wk)    ", "Accumulative Lead Time",
		ND, NO,  JUSTRIGHT, "0", "999.99", (char *) &incc_rec.lead_time},
	{1, LIN, "acc_mlt", 	12, 40, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", " ", "Acc MFG L/T  (Wk)    ", "Accumulative Manufacturing Lead Time in Weeks", 
		ND, NO, JUSTRIGHT, "0", "999.99", (char *) &incc_rec.acc_mlt}, 
	{1, LIN, "mlead_time",	13, 40, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ", "Mfg Lead Time (Wk)   ", "Lead Time To Manufacture Item",
		ND, NO,  JUSTRIGHT, "0", "999.99", (char *) &incc_rec.lead_time},
	{1, LIN, "abc_code",	15, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "ABC Code             ", "",
		 NO, NO,  JUSTLEFT, "ABCD", "", incc_rec.abc_code},
	{1, LIN, "abc_update",	15, 40, CHARTYPE,
		"U", "          ",
		" ", "Y", "ABC Update           ", "Automatic Update of ABC Code (Y/N)",
		 NO, NO,  JUSTLEFT, "YN", "", incc_rec.abc_update},

	{2, LIN, "dflt_bom", 	3, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", "Default BOM No       ", "Default BOM number.", 
		NO, NO, JUSTRIGHT, "0", "32767", (char *) &incc_rec.dflt_bom}, 
	{2, LIN, "dflt_rtg", 	4, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", "0", "Default RTG No       ", "Default Routing number.", 
		NO, NO, JUSTRIGHT, "0", "32767", (char *) &incc_rec.dflt_rtg}, 
	{2, LIN, "eoq", 	5, 2, FLOATTYPE, 
		local_rec.dfltQty, "          ", 
		" ", " ", "Economic Ord Qty     ", "Economic Order Quantity.", 
		NO, NO, JUSTRIGHT, "", "", (char *) &incc_rec.eoq}, 
	{2, LIN, "mlast_bom",	7, 2, INTTYPE,
		"NNNNN", "          ",
		" ", " ", "Last BOM Used        ", "Last BOM Used for Lead Time Calculation",
		NA, NO,  JUSTRIGHT, "", "", (char *) &incc_rec.last_bom},
	{2, LIN, "mlast_rtg",	8, 2, INTTYPE,
		"NNNNN", "          ",
		" ", " ", "Last RTG Used        ", "Last RTG Used for Lead Time Calculation",
		NA, NO,  JUSTRIGHT, "", "", (char *) &incc_rec.last_rtg},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadCcmr 			(void);
void 	Update 				(void);
void 	SrchQcmr 			(char *);
void 	IntSrchLomr 		(char *);
int  	spec_valid 			(int);
int  	ValidQuantity 		(double, int);
int  	heading 			(int);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	int		i;
	char	*sptr;

	TruePosition	=	TRUE;

	/*--------------------------
	| Set up the Quantity Mask |
	--------------------------*/
	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dfltQty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dfltQty, sptr);

	SETUP_SCR (vars);

	envMultLoc = atoi (get_env ("MULT_LOC"));
	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	FLD ("qcTime")		= envQcApply ? YES : ND;
	FLD ("qcCentre")	= envQcApply ? YES : ND;
	FLD ("qcLocation")	= envQcApply ? YES : ND;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	for (i = 0; i < 2; i ++)
		tab_data [i]._desc = scn_desc [i];

	OpenDB ();
	ReadCcmr ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------
		| Reset control flags. |
		----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		new_item 	= FALSE;
		init_vars (1);
		search_ok 	= TRUE;
		abc_unlock (incc);
		manItem = FALSE;

		FLD ("lead_time")	= ND;
		FLD ("acc_mlt")		= ND;
		FLD ("mlead_time")	= ND;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
			
		edit_ok (1);

		if (!entry_exit && manItem)
		{
			heading (2);
			entry (2);
			if (restart)
				continue;
			edit_ok (2);
		}
		else
		{
			scn_display (1);
			if (manItem)
				edit_ok (2);
			else
				no_edit (2);
		}
		
		edit_all ();
		if (restart)
			continue;


		if (restart == 0)
			Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (lomr, lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	open_rec (qcmr, qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("StockWarehouseMaster.txt");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_fclose (lomr);
	abc_fclose (qcmr);

	SearchFindClose ();
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

void
ReadCcmr (void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
	int		field)
{
	/*-----------------------------
	| Validate Item Number Input. |
	-----------------------------*/
	if (LCHECK ("itemNo"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.itemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		
		strcpy (local_rec.itemNo, inmr_rec.item_no);
		DSP_FLD ("itemNo");
		DSP_FLD ("acronym");
		DSP_FLD ("desc");

		if (envQcApply)
		{
			FLD ("qcTime")		= YES;
			FLD ("qcCentre")	= YES;
			FLD ("qcLocation")	= YES;
			if (inmr_rec.qc_reqd [0] == 'N')
			{
				FLD ("qcTime")		= NA;
				FLD ("qcCentre")	= NA;
				FLD ("qcLocation")	= NA;
			}
		}

		/*
		 * Check if item a manufactured item,
		 * if so can display/edit screen 5.  
		 */
		if (!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP"))
		{
			FLD ("lead_time")	= ND;
			FLD ("acc_mlt")		= NO;
			FLD ("mlead_time")	= NO;
			manItem = TRUE;
		}
		else
		{
			FLD ("lead_time")	= NO;
			FLD ("acc_mlt")		= ND;
			FLD ("mlead_time")	= ND;
			manItem = FALSE;
		}
		scn_set (1);
		scn_write (1);
		scn_display (1);

		strcpy (inei_rec.est_no,comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "w");
		if (cc)
		{
			abc_unlock (incc);
			new_item = 1;
		}
		else
		{
			if (incc_rec.ff_option [0] == 'M')
				FLD ("wks_demand") = YES;
			else
				FLD ("wks_demand") = NA;

			if (incc_rec.allow_repl [0] == 'I')
			{
				strcpy (local_rec.allowRepl, "I");
				strcpy (local_rec.allowReplDesc, ML ("(Include)"));
			}
			else
			{
				strcpy (local_rec.allowRepl, "E");
				strcpy (local_rec.allowReplDesc, ML ("(Exclude)"));
			}
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&incc_rec, sizeof (incc_rec));

			entry_exit = 1;
		}

		if (envQcApply)
		{
			if (inmr_rec.qc_reqd [0] == 'Y')
			{
				strcpy (err_str, ML (mlSkMess466));
			}
			else
			{
				/*strcpy (err_str, " QC Checking Required : No  ");*/
				strcpy (err_str, ML (mlSkMess467));
			}
			rv_pr (err_str, (80 - strlen (err_str)) / 2, 16, 1);
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate QC Centre. |
	---------------------*/
	if (LCHECK ("qcCentre"))
	{
		if (!envQcApply)
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		strcpy (qcmr_rec.centre, incc_rec.qc_centre);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Location. |
	--------------------*/
	if (LCHECK ("location") || LCHECK ("qcLocation"))
	{
		if (!envMultLoc)
			return (EXIT_SUCCESS);

		if (LCHECK ("qcLocation") &&
			 (inmr_rec.qc_reqd [0] == 'N' ||
			!envQcApply))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			IntSrchLomr (temp_str);
			return (EXIT_SUCCESS);
		}
		lomr_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		if (LCHECK ("location"))
			sprintf (lomr_rec.location, incc_rec.location);
		else
			sprintf (lomr_rec.location, incc_rec.qc_location);
		cc = find_rec (lomr, &lomr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess209));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| Validate Forecast Method. |
	---------------------------*/
	if (LCHECK ("option"))
	{
		if (incc_rec.ff_option [0] == 'M')
			FLD ("wks_demand") = YES;
		else
			FLD ("wks_demand") = NA;
	}
	/*------------------------------
	| Validate Allow Replenishment |
	------------------------------*/
	if (LCHECK ("allowRepl"))
	{
		if (local_rec.allowRepl [0] == 'I')
			strcpy (local_rec.allowReplDesc, ML ("(Include)"));
		else
			strcpy (local_rec.allowReplDesc, ML ("(Exclude)"));
		DSP_FLD ("allowReplDesc");
	}

	if (LCHECK ("dflt_bom"))
	{
		if (dflt_used)
		{
			incc_rec.dflt_bom = inei_rec.dflt_bom;
			DSP_FLD ("dflt_bom");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_rtg"))
	{
		if (dflt_used)
		{
			incc_rec.dflt_rtg = inei_rec.dflt_rtg;
			DSP_FLD ("dflt_rtg");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eoq"))
	{
		if (dflt_used)
		{
			incc_rec.eoq = inei_rec.eoq;
			DSP_FLD ("eoq");
		}
		incc_rec.eoq = (float) (n_dec (incc_rec.eoq, inmr_rec.dec_pt));
		if (!ValidQuantity (incc_rec.eoq, inmr_rec.dec_pt))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Checks if the quantity entered by the user
 * valid quantity that can be saved to a    
 * float variable without any problems of  
 * losing figures after the decimal point. 
 * eg. if _dec_pt is 2 then the greatest   
 * quantity the user can enter is 99999.99 
 */
int
ValidQuantity (
	double 	_qty, 
	int 	_dec_pt)
{
	/*
	 * Quantities to be compared with with the user has entered.  
	 */
	double	compare [7];
	
	compare [0] = 9999999.00;
	compare [1] = 999999.90;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, ML (mlSkMess238), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

void
Update (void)
{
	clear ();

	/*
	 * Update cost centre stock item record .
	 */
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	
	sprintf (tempSort,"%s%11.11s%16.16s",inmr_rec.inmr_class,
				inmr_rec.category,inmr_rec.item_no);

	strcpy (incc_rec.sort,tempSort);
	sprintf (incc_rec.allow_repl, "%-1.1s", local_rec.allowRepl);

	if (new_item == 1)
	{
		incc_rec.first_stocked = TodaysDate ();
		strcpy (incc_rec.stat_flag,"0");
		cc = abc_add (incc,&incc_rec);
		if (cc) 
			file_err (cc, incc, "DBADD");
	}
	else
	{
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %s (%s)", ML ("Item"), inmr_rec.item_no, inmr_rec.description);
		 AuditFileAdd (err_str, &incc_rec, incc_list, INCC_NO_FIELDS);
		cc = abc_update (incc,&incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");
	}
	strcpy (local_rec.previousItem,inmr_rec.item_no);
}

/*
 * Search for QC Centre master file.
 */
void
SrchQcmr (
	char *keyValue)
{
	_work_open (4,0,40);
	save_rec ("#No", "#QC Centre Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", keyValue);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, keyValue, strlen (keyValue)))
	{
		if (save_rec (qcmr_rec.centre, qcmr_rec.description))
			break;

		cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		file_err (cc, qcmr, "DBFIND");
}

/*
 * Search for Category master file.
 */
void
IntSrchLomr (
	char	*keyValue)
{
	_work_open (10,0,40);
	lomr_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (lomr_rec.location,"%-10.10s",keyValue);
	save_rec ("#Location","#Location Description");
	cc = find_rec (lomr,&lomr_rec,GTEQ,"r");
	while (!cc && !strncmp (lomr_rec.location,keyValue,strlen (keyValue)) &&
		       lomr_rec.hhcc_hash == ccmr_rec.hhcc_hash) 
	{
		cc = save_rec (lomr_rec.location,lomr_rec.desc);
		if (cc)
			break;

		cc = find_rec (lomr,&lomr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	lomr_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (lomr_rec.location,"%-10.10s",temp_str);
	cc = find_rec (lomr,&lomr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, lomr, "DBFIND");
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
		rv_pr (ML (mlSkMess468),25,0,1);
		print_at (0,53,ML (mlSkMess096),local_rec.previousItem);
		move (0,1); line (80);

		if (scn == 1)
		{
			box (0, 2, 80, 13);
			line_at (6,1,79);
			line_at (8,1,79);
			line_at (14,1,79);
			if (envQcApply && inmr_rec.qc_reqd [0] != ' ')
			{
				if (inmr_rec.qc_reqd [0] == 'Y')
					strcpy (err_str, ML (mlSkMess466));
				else
					strcpy (err_str, ML (mlSkMess467));
				
				rv_pr (err_str, (80 - strlen (err_str)) / 2, 16, 1);
			}
		}
		else
		{
			box (0, 2, 80, 6);
			line_at (6,1,79);
		}

		line_at (18,0,80);
		print_at (19, 0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (20, 0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		print_at (21, 0, ML (mlStdMess099), comm_rec.cc_no,comm_rec.cc_name);
		
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


