/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_trans.c,v 5.13 2002/07/24 08:39:19 scott Exp $
|  Program Name  : (sk_trans.c )                                      |
|  Program Desc  : (Stock Transfers Input Program.           )        |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_trans.c,v $
| Revision 5.13  2002/07/24 08:39:19  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.12  2002/07/18 07:15:56  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.11  2002/06/26 05:48:51  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.10  2002/06/20 07:11:17  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.9  2002/02/26 03:15:44  scott
| S/C 00855 SKAJ4 - One-Step Stock Transfers: System does not accept Location inputted whether via SEARCH or by manual encoding.  User cannot continue with transfer.  This only happens when item was originally purchased while PO_GRIN_NOPLATE = 1.  GUI Error: If SK_BATCH_CONT or SK_GRIN_NOPLATE is set to 1, the screen just blinks in and out without informing user that function not available when the 2 variables are on - unlike the character-based.
|
| Revision 5.8  2002/01/09 01:16:58  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.7  2001/12/07 06:53:35  scott
| Updated for temp fix.
|
| Revision 5.6  2001/09/21 07:29:31  cha
| ERROR-316. Updated to prevent DBFIND error when item is transferred
| to the same location in another warehouse.
|
| Revision 5.5  2001/09/21 07:00:35  cha
| ERROR-316. Updated to get the correct receipt inlo_hash when transferring
| to another warehouse.
|
| Revision 5.4  2001/08/28 08:46:43  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/09 09:20:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:46:08  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:39  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_trans.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_trans/sk_trans.c,v 5.13 2002/07/24 08:39:19 scott Exp $";

#define	NOHDLINES	3 

#define	READ_GLWK
#define	MAXSCNS 	2
#define MAXWIDTH 	160
#define	MAXLINES	100
#define	SR	store [line_cnt]

#define LOT_CONTROL	(SR._lot_ctrl [0] == 'Y' && SK_BATCH_CONT)

#include <pslscr.h>	
#include <GlUtils.h>	
#include <twodec.h>	
#include <ml_std_mess.h>	
#include <ml_sk_mess.h>	

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inccRecord	incc_rec;
struct ffdmRecord	ffdm_rec;
struct inwdRecord	inwd_rec;
struct inwuRecord	inwu_rec;
struct esmrRecord	esmr_rec;
struct inloRecord	inlo_rec;

	char	*inum2	= "inum2",
			*inlo2	= "inlo2",
			*data	= "data";

	int		printerNumber = 1,
	   		lpsw = FALSE;
	
	int		envVarSkSerialOk 		= FALSE;
	int		envVarAllowZeroCost 	= FALSE;

	double	batch_total	= 0,
			qty_total 	= 0,
			wk_value 	= 0;

	char	*head [NOHDLINES] = {	/* Heading for report output */

		"=========================================================================================================================================================",
		"|  ITEM NUMBER   |  ITEM DESCRIPTION        |     NARRATIVE      | CREDIT ACCOUNT   | DEBIT ACCOUNT   | UOM | QUANTITY  |   @COST  |  EXTENDED | BATCH. |",
		"|----------------|--------------------------|--------------------|------------------|-----------------|-----|-----------|----------|-----------|--------|"
	};

#include	<MoveRec.h>
#include	<Costing.h>

	struct storeRec {
		long 	hhbrHash;
		long 	creditHhmrHash;
		long 	debitHhmrHash;
		long 	issueHhwhHash;
		long 	receiptHhwhHash;
		long 	receiptInloHash;
		long 	issueInloHash;
		char	issueLocation [11];
		char	receiptLocation [11];
		char	issueLot [8];
		char	receiptLot [8];
		char	_uom [5];
		float 	issueClosingStock;
		float 	receiptClosingStock;
		int		serial_item;
		int		decPt;
		char	cost_flag [2];
		double	receiptAvgeCost;
		long	hhumHash;
		float	_cnv_fct;
		double	_cost;
		float	_qty;
		char	_lot_ctrl [2];
		float	upft_pc;
		double	upft_amt;
		char	Demand [2];
	} store [MAXLINES]; 

	char	iss_br [3],
	    	iss_brname [41],
	    	iss_wh [3],
	    	iss_whname [41],
	    	rec_br [3],
	    	rec_brname [41],
	    	rec_wh [3],
	    	rec_whname [41];
	   

	long	issueHhccHash,
	        receiptHhccHash;


/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char 	sr_est [3];
	char	sr_cc [3];
	char	serial_no [26];
	char	sr_ename [41];
	char	dummy [11];
	char	prev_item [7];
	char	j_ref [9];
	char	systemDate [11];
	long	lsystemDate;
	char	inv_date [11];
	char	item_no [17];
	char	wk_item [17];
	char 	crd_ac_desc [81];
	char 	dbt_ac_desc [81];
	char   	wk_credit_ac [MAXLEVEL + 1];
	char   	wk_debit_ac [MAXLEVEL + 1];
	char   	df_credit_ac [MAXLEVEL + 1];
	char   	df_debit_ac [MAXLEVEL + 1];
	long 	j_date;
	float  	wk_qty;
	double 	wk_cost;
	char	LL [2];
	char	UOM [5];
	char	LocationFrom [11];
	char	LocationTo [11];
	char	description [36];
	char	batchString [8];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "br_no",	 5, 22, CHARTYPE,
		"AA", "          ",
		" ", "", "Receiving Branch.", " ",
		 NE, NO, JUSTRIGHT, "1", "99", rec_br},
	{1, LIN, "br_name",	 6, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name.", " ",
		 NA, NO, JUSTRIGHT, "", "", rec_brname},
	{1, LIN, "wh_no",	 7, 22, CHARTYPE,
		"AA", "          ",
		" ", "", "Receiving Warehouse.", " ",
		 NE, NO, JUSTRIGHT, "1", "99", rec_wh},
	{1, LIN, "wh_name",	 8, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name.", " ",
		 NA, NO, JUSTRIGHT, "", "", rec_whname},
	{1, LIN, "ref",	10, 22, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "Batch ref.", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.j_ref},
	{1, LIN, "jdate",	11, 22, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.inv_date, "Batch date    ", " ",
		YES, NO, JUSTRIGHT, "1", "", (char *)&local_rec.j_date},
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "    Item no.    ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "item_desc",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "      D e s c r i p t i o n        ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description},
	{2, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", " Unit of Measure ",
		 NO, NO, JUSTLEFT, "", "", local_rec.UOM},
	{2, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", " Quantity ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.wk_qty},
	{2, TAB, "locn_from",	 0, 1, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "  Loc From  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.LocationFrom},
	{2, TAB, "locn_to",	 0, 1, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "   Loc to   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.LocationTo},
	{2, TAB, "cost",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Item  Cost", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.wk_cost},
	{2, TAB, "ser_no",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "        Serial No        ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{2, TAB, "debit",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"0", local_rec.df_debit_ac,  " Debit Account  ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.wk_debit_ac},
	{2, TAB, "credit",	 0, 0, CHARTYPE,
		GlMask, "          ",
		"0", local_rec.df_credit_ac, " Credit Account ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.wk_credit_ac},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
#include	<LocHeader.h>
#include 	<MoveAdd.h>

/*=======================
| Function Declarations |
=======================*/
float 	ToLclUom 			(float);
float 	ToStdUom 			(float);
int	 	CheckClass 			(void);
int  	AddIncc 			(long, long);
int  	FindCost 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	CloseDB 			(void);
void 	ClosePrintAudit 	(void);
void 	GlDefault 			(void);
void 	OpenDB 				(void);
void 	OpenPrintAudit 		(void);
void 	PrintCompanyDetails (void);
void 	ReadMisc 			(void);
void 	SearchLocation 		(int, long, char *);
void 	SrchCcmr 			(char *);
void 	SrchEsmr 			(char *);
void 	SrchInum 			(char *);
void 	shutdown_prog 		(void);
void 	Update 				(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 2 && argc != 4)	
	{
		print_at (0,0, mlSkMess588,argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (GlMask,   "%-*.*s", MAXLEVEL, MAXLEVEL, "AAAAAAAAAAAAAAAA");
	sprintf (GlDesc,   "%-*.*s", MAXLEVEL, MAXLEVEL, ML ("Account Number"));
	sprintf (GlfDesc,   "%-*.*s",FORM_LEN, FORM_LEN, ML ("Account Number"));
	sprintf (GlFormat, "%-*.*s", MAXLEVEL, MAXLEVEL, "XXXXXXXXXXXXXXXX");

	SETUP_SCR (vars);


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/

	printerNumber = atoi (argv [1]);

	if (argc == 4)
	{
		sprintf (local_rec.df_debit_ac, "%-*.*s",MAXLEVEL,MAXLEVEL,argv [2]);
		sprintf (local_rec.df_credit_ac,"%-*.*s",MAXLEVEL,MAXLEVEL,argv [3]);
	}
	else
	{
		sprintf (local_rec.df_debit_ac, "%*.*s",MAXLEVEL,MAXLEVEL," ");
		sprintf (local_rec.df_credit_ac,"%*.*s",MAXLEVEL,MAXLEVEL," ");
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();

    sptr = chk_env ("SK_SERIAL_OK");
	envVarSkSerialOk = (sptr == (char *) 0) ? 0 : atoi (sptr);

    sptr = chk_env ("ALLOW_ZERO_COST");
	envVarAllowZeroCost = (sptr == (char *) 0) ? 0 : atoi (sptr);

	FLD ("ser_no")	=	(envVarSkSerialOk) ? YES : ND;
	FLD ("UOM")		=	(envVarSkSerialOk) ? ND  : YES;

	FLD ("locn_from")	=	ND;
	FLD ("locn_to")		=	ND;
	
	if (SK_BATCH_CONT)
	{
		clear ();
		PauseForKey (0,0, "This option not available as SK_BATCH_CONT OR SK_GRIN_NOPLATE environment are set",0);
		shutdown_prog ();	
    	return (EXIT_SUCCESS);
	}
	if (SK_BATCH_CONT || MULT_LOC)
	{
		FLD ("locn_from")	=	YES;
		FLD ("locn_to")		=	YES;
	}
	strcpy (local_rec.batchString, "  N/A  ");

	GL_SetMask (GlFormat);		/*  setup default G/L format.   */

	swide ();

	strcpy (local_rec.inv_date,DateToString (comm_rec.inv_date));

	while (prog_exit == 0) 
	{
		entry_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2]	= 0;

		sprintf (inmr_rec.item_no,"%16.16s"," ");
		sprintf (inmr_rec.description,"%40.40s"," ");
		local_rec.wk_qty	= 0;
		local_rec.wk_cost	= 0;
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (2);
		entry (2);
		if (restart)
			continue;

		/* enter edit mode */
		edit_all ();
		if (restart)
			continue;

		FLD ("qty") = YES;
		/*----------------------------- 
		| Update stock & glwk record. |
		-----------------------------*/
		Update ();

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
	ClosePrintAudit ();
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open Data Dase Files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	ReadMisc ();

	MoveOpen	=	TRUE;
	abc_alias (inum2, inum);
	abc_alias (inlo2, inlo);

	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (inmr , inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (excf , excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (incc , incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inlo2, inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");
	open_rec (ffdm,  ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (inwd,  inwd_list, INWD_NO_FIELDS, "inwd_id_no3");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_mst_id");

	OpenGlmr ();
	OpenGlwk ();
	OpenLocation (ccmr_rec.hhcc_hash);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (excf);
	abc_fclose (inwu);
	abc_fclose (ffdm);
	abc_fclose (inwd);
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (inum2);
	abc_fclose (inum);
	abc_fclose (inlo);
	abc_fclose (inlo2);
	abc_fclose ("move");
	CloseLocation ();
	GL_Close ();
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr ,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
	open_rec (esmr ,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr ,&ccmr_rec,COMPARISON,"r");
	if (cc) 
		file_err (cc, ccmr, "DBFIND");

	issueHhccHash = ccmr_rec.hhcc_hash;
	strcpy (iss_wh,ccmr_rec.cc_no);
	strcpy (iss_whname,ccmr_rec.name);
	strcpy (iss_br,comm_rec.est_no);
	strcpy (iss_brname,comm_rec.est_name);
}

/*==========================================
| Primary validation and file access here. |
==========================================*/
int
spec_valid (
 int field)
{
	int 	i;
	char	*ser_no;

	/*-----------------------------------
	| Validate receiving branch number. |
	-----------------------------------*/
	if (LCHECK ("br_no"))
	{ 
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,rec_br);
		cc = find_rec (esmr ,&esmr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (rec_br,esmr_rec.est_no);
		strcpy (rec_brname,esmr_rec.est_name);
		DSP_FLD ("br_name");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------
	| Validate receiving warehouse number. |
	--------------------------------------*/
	if (LCHECK ("wh_no"))
	{
		if (!strcmp (rec_br,iss_br) && !strcmp (rec_wh,iss_wh))
		{
			print_mess (ML (mlSkMess622));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,rec_br);
		strcpy (ccmr_rec.cc_no,rec_wh);
		cc = find_rec (ccmr ,&ccmr_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		receiptHhccHash = ccmr_rec.hhcc_hash;
		strcpy (rec_wh,ccmr_rec.cc_no);
		strcpy (rec_whname,ccmr_rec.name);
		PrintCompanyDetails ();
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
			
	/*-----------------------------
	| Validate item number input. |
	-----------------------------*/
	if (LCHECK ("item_no"))
	{ 
		if (last_char == EOI) 
		{
			prog_exit = 1; 
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		sprintf (local_rec.description,"%-35.35s", inmr_rec.description);
		DSP_FLD ("item_no");
		DSP_FLD ("item_desc");

		/*---------------------
		| Find for UOM GROUP. |
		----------------------*/
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");
		
		SR.hhumHash 	= inum_rec.hhum_hash;
		SR._cnv_fct		= inum_rec.cnv_fct;

		/*--------------------------------------------------
		| Look up to see if item is on issuing Cost Centre |
		--------------------------------------------------*/
		incc_rec.hhcc_hash = issueHhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;

		cc = find_rec (incc ,&incc_rec,COMPARISON,"r");
		if (cc) 
		{
			print_mess (ML (mlSkMess616));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SR.issueHhwhHash = incc_rec.hhwh_hash;
		SR.issueClosingStock = incc_rec.closing_stock;
		/*----------------------------------------------------
		| Look up to see if item is on receiving Cost Centre |
		----------------------------------------------------*/
		incc_rec.hhcc_hash = receiptHhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc ,&incc_rec,COMPARISON,"r");
		if (cc) 
		{
			i = prmptmsg (ML (mlSkMess583),"YyNn",1,2);
			if (i == 'n' || i == 'N') 
			{
				skip_entry = goto_field (field, label ("item_no"));
				return (EXIT_SUCCESS); 
			}
			else 
			{
				cc = AddIncc (receiptHhccHash, inmr_rec.hhbr_hash);
				if (cc) 	
					file_err (cc, incc, "DBADD");
			}
			move (1,2);cl_line ();
		}
		SR.receiptHhwhHash = incc_rec.hhwh_hash;
		SR.receiptClosingStock = incc_rec.closing_stock;

		FLD ("qty") = (inmr_rec.costing_flag [0] == 'S') ? NA : YES;
		if (!envVarSkSerialOk && inmr_rec.serial_item [0] == 'Y')
		{
			print_mess (ML ("Invalid environment setup for serial item"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (envVarSkSerialOk)
			FLD ("ser_no") = (inmr_rec.serial_item [0] == 'Y') ? YES : NA;
		
		SR.serial_item = (inmr_rec.serial_item [0] == 'Y') ? 1 : 0;
		SR.decPt = inmr_rec.dec_pt;
		strcpy (SR.cost_flag,inmr_rec.costing_flag);
		SR.hhbrHash = inmr_rec.hhbr_hash;
		if (SR.cost_flag [0] == 'S')
		{
			local_rec.wk_qty = 1.00;
			cc = FindCost ();
			
			DSP_FLD ("qty");
			DSP_FLD ("cost");
			DSP_FLD ("ser_no");
		}
		SR.serial_item 	= (inmr_rec.serial_item [0] == 'Y') ? 1 : 0;
		SR.decPt 		= inmr_rec.dec_pt;
		strcpy (SR._lot_ctrl, inmr_rec.lot_ctrl);

		strcpy (excf_rec.co_no,  inmr_rec.co_no);
		strcpy (excf_rec.cat_no, inmr_rec.category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			excf_rec.hhcf_hash	=	0L;

		/*-------------------------------------------------------------------
		| Check for warehouse supply structure to see if demand is updated. |
		-------------------------------------------------------------------*/
		inwd_rec.hhcc_hash	=	receiptHhccHash;
		inwd_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inwd_rec.hhcf_hash	=	0L;
		cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
		if (cc)
		{
			inwd_rec.hhcc_hash	=	receiptHhccHash;
			inwd_rec.hhbr_hash	=	0L;
			inwd_rec.hhcf_hash	=	excf_rec.hhcf_hash;
			cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
			if (cc)
			{
				inwd_rec.hhcc_hash	=	receiptHhccHash;
				inwd_rec.hhbr_hash	=	0L;
				inwd_rec.hhcf_hash	=	0L;
				cc = find_rec (inwd, &inwd_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			strcpy (SR.Demand, "N");
			SR.upft_pc		=	0.00;
			SR.upft_amt		=	0.00;
		}
		else
		{
			strcpy (SR.Demand, 	inwd_rec.demand);
			SR.upft_pc		=	inwd_rec.upft_pc;
			SR.upft_amt		=	inwd_rec.upft_amt;
		}
		GlDefault ();
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Unit of Measure. | 
	---------------------------*/
	if (LCHECK ("UOM"))
	{
		if (dflt_used || (F_NOKEY (field))) 
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			strcpy (SR._uom, inum_rec.uom);
			SR.hhumHash = inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (SR._uom, inum2_rec.uom);
		SR.hhumHash 	= inum2_rec.hhum_hash;
		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;
		SR._cnv_fct 	= inum2_rec.cnv_fct/inum_rec.cnv_fct;
		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate quantity input and lookup cost. |
	------------------------------------------*/
	if (LCHECK ("qty"))
	{
		if (last_char == EOI && prog_status == ENTRY) 
		{
			skip_entry = goto_field (field, label ("item_no"));
			return (EXIT_SUCCESS);
		}

		clear_mess ();

		if (SR.issueClosingStock < ToStdUom (local_rec.wk_qty))
		{
			sprintf (err_str,ML (mlSkMess125),local_rec.wk_qty, SR.issueClosingStock, clip (local_rec.item_no));
			i = prmptmsg (err_str,"YyNn",1,2);
			if (i == 'n' || i == 'N') 
			{
				skip_entry = goto_field (field, label ("item_no"));
				move (1,2);cl_line ();
				return (EXIT_SUCCESS); 
			}
		}
		cc = FindCost ();
		if (cc)
		{
			skip_entry = goto_field (field, label ("item_no"));
			return (EXIT_SUCCESS);
		}
		move (1,2);cl_line ();
		
		SR._qty	=	local_rec.wk_qty;
		DSP_FLD ("cost");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("locn_from"))
	{
		if (F_NOKEY (field)) 
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchLocation (TRUE, SR.issueHhwhHash, temp_str);
			return (EXIT_SUCCESS);
		}

		abc_selfield (inlo, "inlo_mst_loc");
		inlo_rec.hhwh_hash	=	SR.issueHhwhHash;
		inlo_rec.hhum_hash	=	SR.hhumHash;
		strcpy (inlo_rec.location, local_rec.LocationFrom);
		cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess209));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (SR.issueLocation, inlo_rec.location);
		strcpy (SR.issueLot, inlo_rec.lot_no);
		SR.issueInloHash	=	inlo_rec.inlo_hash;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("locn_to"))
	{
		char	locationType [2];

		if (F_NOKEY (field)) 
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchLocation (TRUE, SR.receiptHhwhHash, temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
			strcpy (local_rec.LocationTo, SR.issueLocation);

		abc_selfield (inlo, "inlo_mst_loc");
		inlo_rec.hhwh_hash	=	SR.receiptHhwhHash;
		inlo_rec.hhum_hash	=	SR.hhumHash;
		strcpy (inlo_rec.location, local_rec.LocationTo);
		cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
		if (cc)
		{
			i = prmptmsg (ML ("Location not on file, Add Location"),"YyNn",1,2);
			if (i == 'n' || i == 'N') 
			{
				print_mess (ML (mlStdMess209));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			cc = 	CheckLocation 
				 	(
						receiptHhccHash, 
						local_rec.LocationTo, 
						locationType
					);
			if (cc)
			{
				print_mess (ML ("Location not in Location Master."));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (err_str, DateToString (0L));
			
			PostLotLocation 
			(
				SR.receiptHhwhHash,
				receiptHhccHash,
				SR.hhumHash,
				local_rec.UOM,
				(LOT_CONTROL) ? DfltLotNo : local_rec.batchString,
				(LOT_CONTROL) ? DfltLotNo : local_rec.batchString,
				local_rec.LocationTo,
				locationType,
				err_str,
				0.00,
				SR._cnv_fct,
				TRUE,
				"A",
				0.00,
				0.00,
				0.00,
				0.00,
				0L
			);
			/*Make sure that we save the currentInloHash to the receiptInloHash*/
			abc_selfield (inlo, "inlo_inlo_hash");
			inlo_rec.inlo_hash = currentInloHash;
			cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inlo, "DBFIND");	
			 abc_selfield (inlo, "inlo_mst_loc");	
		}
			
	
		strcpy (SR.receiptLocation, inlo_rec.location);
		strcpy (SR.receiptLot, inlo_rec.lot_no);
		SR.receiptInloHash	=	inlo_rec.inlo_hash;
	   
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate cost input. |
	----------------------*/
	if (LCHECK ("cost"))
	{
		if (last_char == EOI && prog_status == ENTRY) 	
		{
			skip_entry = goto_field (field, label ("item_no"));
			return (EXIT_SUCCESS);
		}

		if (dflt_used || local_rec.wk_cost == 0.00) 
		{
			cc = FindCost ();
			if (cc)
			{
				skip_entry = goto_field (field, label ("item_no"));
				return (EXIT_SUCCESS);
			}
			DSP_FLD ("cost");
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("ser_no"))
	{
		if (SR.serial_item != 1 || !envVarSkSerialOk)
			return (EXIT_SUCCESS);

		ser_no = clip (local_rec.serial_no);

		if (SRCH_KEY)
		{
			SearchInsf (SR.issueHhwhHash, "F", temp_str);
			return (EXIT_SUCCESS);
		}
		local_rec.wk_cost	=	FindInsfCost 
								(
									SR.issueHhwhHash,
									0L,
									local_rec.serial_no,
									"F"
								);
		if (local_rec.wk_cost != -1.00)
		{
			local_rec.wk_cost = SR._cost = n_dec ((local_rec.wk_cost * SR._cnv_fct), 5);
			DSP_FLD ("cost");
		}
		else
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (insfRec.receipted [0] != 'Y')
		{
			sprintf (err_str, ML (mlSkMess343), local_rec.serial_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Debit account. |
	-------------------------*/
	if (LCHECK ("debit"))
	{
		if (F_NOKEY (field)) 
			strcpy (local_rec.wk_debit_ac, local_rec.df_debit_ac);

		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		strcpy (glmrRec.co_no,comm_rec.co_no);
		GL_FormAccNo (local_rec.wk_debit_ac, glmrRec.acc_no, 0);
		cc = find_rec (glmr , &glmrRec, COMPARISON, "r");
		if (cc) 
		{
		    errmess (ML (mlStdMess024));
		    sleep (sleepTime);		
		    return (EXIT_FAILURE);
		}
			
		if (CheckClass ())
			return (EXIT_FAILURE);

		strcpy (local_rec.dbt_ac_desc,glmrRec.desc);
		SR.debitHhmrHash = glmrRec.hhmr_hash; 

		move (1,3); cl_line ();

		print_at (3,1,ML (mlSkMess584),local_rec.dbt_ac_desc);
		fflush (stdout);
		strcpy (local_rec.df_debit_ac,local_rec.wk_debit_ac);
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Credit account. |
	--------------------------*/
	if (LCHECK ("credit"))
	{
		if (F_NOKEY (field)) 
			strcpy (local_rec.wk_credit_ac, local_rec.df_credit_ac);

		if (SRCH_KEY)
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		strcpy (glmrRec.co_no,comm_rec.co_no);

		GL_FormAccNo (local_rec.wk_credit_ac, glmrRec.acc_no, 0);
		cc = find_rec (glmr , &glmrRec, COMPARISON, "r");
		if (cc) 
		{
		    errmess (ML (mlStdMess024));
		    sleep (sleepTime);		
		    return (EXIT_FAILURE);
		}
			
		if (CheckClass ())
			return (EXIT_FAILURE);

		strcpy (local_rec.crd_ac_desc,glmrRec.desc);
		SR.creditHhmrHash = glmrRec.hhmr_hash; 

		move (1,4); cl_line ();

		print_at (4,1,ML (mlSkMess585),local_rec.crd_ac_desc);
		fflush (stdout);
		strcpy (local_rec.df_credit_ac,local_rec.wk_credit_ac);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
GlDefault (
 void)
{
	GL_GLI 
	(
		comm_rec.co_no,
		rec_br, 
		rec_wh, 
		"INVENTORY ",
		" ",
		inmr_rec.category
	);
	strcpy (local_rec.df_debit_ac, glmrRec.acc_no);

	GL_GLI 
	(
		comm_rec.co_no,
		iss_br, 
		iss_wh, 
		"INVENTORY ",
		" ",
		inmr_rec.category
	);
	strcpy (local_rec.df_credit_ac, glmrRec.acc_no);
}

int
FindCost (
 void)
{
	SR.receiptAvgeCost	=	FindIneiCosts ("A", rec_br, SR.hhbrHash);

	switch (SR.cost_flag [0])
	{
	case 'L':
	case 'A':
	case 'P':
	case 'T':
		local_rec.wk_cost = FindIneiCosts (SR.cost_flag, rec_br, SR.hhbrHash);
		break;

	case 'F':
		local_rec.wk_cost	=	FindIncfValue 
								(
									SR.issueHhwhHash,
									SR.issueClosingStock,
									ToStdUom (local_rec.wk_qty),
									TRUE,
									SR.decPt
								);
		break;

	case 'I':
		local_rec.wk_cost	=	FindIncfValue 
								(
									SR.issueHhwhHash,
									SR.issueClosingStock,
									ToStdUom (local_rec.wk_qty),
									FALSE,
									SR.decPt
								);
		break;

	case 'S':
		local_rec.wk_cost = FindInsfValue (SR.issueHhwhHash,TRUE);
		break;

	default:
		break;
	}

	if (local_rec.wk_cost < 0.00)	
		local_rec.wk_cost = FindIneiCosts ("L", rec_br, SR.hhbrHash);

	local_rec.wk_cost = SR._cost = n_dec ((local_rec.wk_cost * SR._cnv_fct), 5);

	return (EXIT_SUCCESS);
}

/*=============================================
| Update incc & add transaction to glwk file. |
=============================================*/
void
Update (
 void)
{
	long	issueHhwhHash = 0L;
	long	receiptHhwhHash = 0L;
	float	issueClosingStock = 0.00;
	float	receiptClosingStock = 0.00;
	double	value;
	double	old_qty = 0;
	double	xx_qty = 0;
	double	cal1_total = 0;
	char	wk_ref [11];
	int		monthNum;

	clear ();

	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	/*-------------------------------------------------------
	| Update inventory cost centre stock record (file incc).|
	-------------------------------------------------------*/
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		/*-------------------
		| Get Labular line. |
		-------------------*/
		getval (line_cnt);

		/*-------------------------------------------------
		| Find inmr record from item number in structure. | 
		-------------------------------------------------*/
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		strcpy (inmr_rec.item_no,local_rec.item_no);
		cc = find_rec (inmr ,&inmr_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		/*----------------------------
		| Update issuing Cost Centre |
		----------------------------*/
		incc_rec.hhcc_hash = issueHhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc ,&incc_rec,COMPARISON,"u");
		if (cc) 
			file_err (cc, incc, "DBFIND");

		issueClosingStock 	= incc_rec.closing_stock;
		issueHhwhHash 		= incc_rec.hhwh_hash;

		incc_rec.issues 		+= ToStdUom (local_rec.wk_qty);
		incc_rec.ytd_issues 	+= ToStdUom (local_rec.wk_qty);
		incc_rec.closing_stock 	= 	incc_rec.opening_stock +
									incc_rec.pur +
									incc_rec.receipts +
									incc_rec.adj -
									incc_rec.issues -
									incc_rec.sales;

		cc = abc_update (incc ,&incc_rec);
		if (cc) 		
			file_err (cc, incc, "DBUPDATE");

		if (SR.Demand [0] == 'Y')
		{
			ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
			ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
			strcpy (ffdm_rec.type, "4");
			ffdm_rec.date		=	StringToDate (local_rec.systemDate);
			cc = find_rec ("ffdm", &ffdm_rec, COMPARISON, "u");
			if (cc)
			{
				ffdm_rec.qty	=	ToStdUom (local_rec.wk_qty);
				cc = abc_add (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, ffdm, "DBADD");
			}
			else
			{
				ffdm_rec.qty	+=	ToStdUom (local_rec.wk_qty);
				cc = abc_update (ffdm, &ffdm_rec);
				if (cc)
					file_err (cc, ffdm, "DBUPDATE");
			}
		}
		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		inwu_rec.issues	+= ToStdUom (local_rec.wk_qty);
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");

		/*------------------------------
		| Update Receiving Cost Centre |
		------------------------------*/
		incc_rec.hhcc_hash = receiptHhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc ,&incc_rec,COMPARISON,"u");
		if (cc) 
			file_err (cc, incc, "DBFIND");

		old_qty 	= incc_rec.closing_stock;
		xx_qty 		= incc_rec.closing_stock;
		incc_rec.receipts     	+= ToStdUom (local_rec.wk_qty);
		incc_rec.ytd_receipts   += ToStdUom (local_rec.wk_qty);
		incc_rec.closing_stock 	= 	incc_rec.opening_stock +
									incc_rec.pur +
									incc_rec.receipts +
									incc_rec.adj -
									incc_rec.issues -
									incc_rec.sales;

		receiptClosingStock = incc_rec.closing_stock;
		receiptHhwhHash 	= incc_rec.hhwh_hash;

		cc = abc_update (incc ,&incc_rec);
		if (cc) 
			file_err (cc, incc, "DBUPDATE");

		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	SR.hhumHash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	SR.hhumHash;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		inwu_rec.receipts	+= ToStdUom (local_rec.wk_qty);
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");

		if (inmr_rec.costing_flag [0] == 'F')
			ReduceIncf (incc_rec.hhwh_hash,receiptClosingStock,TRUE);

		if (inmr_rec.costing_flag [0] == 'I')
			ReduceIncf (incc_rec.hhwh_hash,receiptClosingStock,FALSE);

		/*-----------------------------------------
		| Get inei record to update average cost. |
		-----------------------------------------*/
		cc = FindInei (inmr_rec.hhbr_hash, rec_br, "r");
		if (cc) 
		{
			abc_unlock (inei);
			print_mess (ML (mlSkMess419));
			sleep (sleepTime);
			return;
		}

		switch (inmr_rec.costing_flag [0])
		{
		case 'A' :
			local_rec.wk_cost = ineiRec.avge_cost * SR._cnv_fct;
			break;

		case 'L' :
			local_rec.wk_cost = ineiRec.last_cost * SR._cnv_fct;
			break;

		case 'F' :
			cc = TransIncf
				(	
					issueHhwhHash,
					receiptHhwhHash,
					issueClosingStock,
					ToStdUom (local_rec.wk_qty),
					TRUE
				);
			if (cc)
			{
				if (SR._cnv_fct == 0.00)
					SR._cnv_fct = 1.00;

				cc	=	AddIncf
						(
							receiptHhwhHash,
							local_rec.j_date,
							(SR._cost / SR._cnv_fct),
							(SR._cost / SR._cnv_fct),
							ToStdUom (local_rec.wk_qty),
							" ",
							(SR._cost / SR._cnv_fct),
							0.00,
							0.00,
							0.00,
							0.00,
							(SR._cost / SR._cnv_fct),
							"T"
						);
				if (cc)
					file_err (cc, "incf", "DBADD");
			}
			break;

		case 'I' :
			cc = TransIncf
				(
					issueHhwhHash,
					receiptHhwhHash,
					issueClosingStock,
					ToStdUom (local_rec.wk_qty),
					FALSE
				);
			if (cc)
				file_err (cc, "incf", "DBADD");
			break;

		default :
			break;
		}

		if (inmr_rec.serial_item [0] == 'Y')
		{
			cc = TransInsf
				(
					issueHhwhHash,
					receiptHhwhHash,
					local_rec.serial_no,
					"F"
				);
			if (cc)
				file_err (cc, insf, "DBUPDATE");
		}

		value = out_cost (SR._cost/SR._cnv_fct, inmr_rec.outer_size); 
		value = twodec (value);

		if (MULT_LOC || SK_BATCH_CONT)
		{
			inlo_rec.inlo_hash	=	SR.issueInloHash;
			cc = find_rec (inlo2, &inlo_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inlo, "DBFIND");

			strcpy (err_str, DateToString (inlo_rec.expiry_date));
			OutLotLocation
			(	
				inlo_rec.hhwh_hash,
				issueHhccHash,
				inlo_rec.hhum_hash,
				inlo_rec.uom,
				inlo_rec.lot_no,
				inlo_rec.slot_no,
				inlo_rec.location,
				inlo_rec.loc_type,
				err_str, 
				ToStdUom (local_rec.wk_qty),
				SR._cnv_fct,
				inlo_rec.loc_status,
				inlo_rec.pack_qty,
				inlo_rec.chg_wgt,
				inlo_rec.gross_wgt,
				inlo_rec.cu_metre,
				inlo_rec.sknd_hash
			);
			sprintf (wk_ref, "TR%2.2s/%2.2s/%2.2s",
											comm_rec.co_no,rec_br,rec_wh);
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			(
				comm_rec.co_no,
				iss_br,
				iss_wh,
				SR.hhbrHash,
				issueHhccHash,
				SR.hhumHash,
				local_rec.j_date,
				3,
				inlo_rec.lot_no,
				inmr_rec.inmr_class,		
				inmr_rec.category,
				wk_ref,
				local_rec.j_ref,
				ToStdUom (local_rec.wk_qty),
				0.0,
				CENTS (value)
			);


			inlo_rec.inlo_hash	=	SR.receiptInloHash;
			cc = find_rec (inlo2, &inlo_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inlo, "DBFIND");

			strcpy (err_str, DateToString (inlo_rec.expiry_date));
			
			
			
			InLotLocation
			(	
				inlo_rec.hhwh_hash,
				receiptHhccHash,
				inlo_rec.hhum_hash,
				inlo_rec.uom,
				inlo_rec.lot_no,
				inlo_rec.slot_no,
				inlo_rec.location,
				inlo_rec.loc_type,
				err_str,
				ToStdUom (local_rec.wk_qty),
				SR._cnv_fct,
				inlo_rec.loc_status,
				inlo_rec.pack_qty,
				inlo_rec.chg_wgt,
				inlo_rec.gross_wgt,
				inlo_rec.cu_metre,
				inlo_rec.sknd_hash
			);
			sprintf (wk_ref,"TR%2.2s/%2.2s/%2.2s",comm_rec.co_no,iss_br,iss_wh);
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			(
				comm_rec.co_no,
				rec_br,
				rec_wh,
				SR.hhbrHash,
				receiptHhccHash,
				SR.hhumHash,
				local_rec.j_date,
				2,
				inlo_rec.lot_no,
				inmr_rec.inmr_class,
				inmr_rec.category,
				wk_ref,
				local_rec.j_ref,  
				ToStdUom (local_rec.wk_qty),
				0.0,
				CENTS (value)
			);
		} 
		if (old_qty < 0.00) 
			xx_qty = 0.00;

		if (SR._cnv_fct == 0.00)
			SR._cnv_fct = 1.00;

		if (local_rec.wk_qty != 0.00) 
		{
			if (old_qty + local_rec.wk_qty == 0.00) 
				cal1_total = SR._cost/SR._cnv_fct;
			else 
			{
				if (old_qty + local_rec.wk_qty < 0.00) 
				{
					if (envVarAllowZeroCost)
						cal1_total = 0;
					else
						cal1_total = SR.receiptAvgeCost;
				}
				else 
					cal1_total = ((xx_qty * SR.receiptAvgeCost) + (ToStdUom (local_rec.wk_qty) * SR._cost/SR._cnv_fct)) / (xx_qty + ToStdUom (local_rec.wk_qty));
			}

			ineiRec.avge_cost 	= cal1_total ;
			ineiRec.prev_cost 	= ineiRec.last_cost;
			ineiRec.last_cost 	= SR._cost/SR._cnv_fct; 
			ineiRec.lpur_qty 	= ToStdUom (local_rec.wk_qty);
			ineiRec.date_lcost 	= comm_rec.inv_date;
			cc = abc_update (inei ,&ineiRec);
			if (cc) 
				file_err (cc, inei, "DBUPDATE");
		}
		else
			abc_unlock (inei);

		/*------------------------------------
		| Set switch is something is printed |
		------------------------------------*/
		if (lpsw == FALSE) 
		{ 
			OpenPrintAudit ();
			lpsw = TRUE;
		}

		value = out_cost (SR._cost/SR._cnv_fct, inmr_rec.outer_size); 
		value = twodec (value);

		wk_value = (float) ToStdUom (local_rec.wk_qty);
		wk_value *= value;
		wk_value = twodec (wk_value); 

		sprintf (glwkRec.narrative,"Tran to BR %s /WH %s",rec_br,rec_wh);

		fprintf (fout,"|%-16.16s",local_rec.item_no);
		fprintf (fout,"|%-26.26s",local_rec.description);
		fprintf (fout,"|%-16.16s  ",local_rec.wk_credit_ac);
		fprintf (fout,"|%-16.16s ",local_rec.wk_debit_ac);
		fprintf (fout,"|%-4.4s ",inmr_rec.sale_unit);
		fprintf (fout,"|%11.2f",ToStdUom (local_rec.wk_qty));
		fprintf (fout,"|%10.10s", local_rec.LocationFrom);
		fprintf (fout,"|%10.10s", local_rec.LocationTo);
		fprintf (fout,"|%10.2f",SR._cost/SR._cnv_fct);
		fprintf (fout,"|%11.2f",wk_value);
		fprintf (fout,"|%8.8s|\n",local_rec.j_ref);

		batch_total += wk_value;
		qty_total += ToStdUom (local_rec.wk_qty);

		/*--------------------------------
		| Add transactions to glwk file. |
		--------------------------------*/
		strcpy (glwkRec.tran_type,"10");
		glwkRec.post_date = StringToDate (local_rec.systemDate);
		glwkRec.tran_date = local_rec.j_date;

		DateToDMY (local_rec.j_date, NULL, &monthNum, NULL);

		sprintf (glwkRec.period_no,"%02d", monthNum);
		sprintf (glwkRec.sys_ref,"%5.1d",1);
		strcpy (glwkRec.user_ref,local_rec.j_ref);
		strcpy (glwkRec.co_no,comm_rec.co_no);
		strcpy (glwkRec.stat_flag,"2");
		sprintf (glwkRec.narrative,"Tran From BR%s /WH%s",iss_br,iss_wh);
		strcpy (glwkRec.alt_desc1, " ");
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no, " ");

		glwkRec.amount 		= 	CENTS (wk_value);
		glwkRec.exch_rate	=	1.00;
		glwkRec.loc_amount 	= 	glwkRec.amount;
		strcpy (glwkRec.acc_no,local_rec.wk_debit_ac);
		glwkRec.hhgl_hash = SR.debitHhmrHash;
		strcpy (glwkRec.jnl_type,"1");
		cc = abc_add (glwk ,&glwkRec);
		if (cc) 
			file_err (cc, "glwk", "DBADD");

		sprintf (glwkRec.acc_no,"%-*.*s", 
							MAXLEVEL,MAXLEVEL,local_rec.wk_credit_ac);
		glwkRec.hhgl_hash = SR.creditHhmrHash;
		strcpy (glwkRec.jnl_type,"2");
		sprintf (glwkRec.narrative,"Transfer  to  WH %s ",rec_wh);
		strcpy (glwkRec.alt_desc1, " ");
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no, " ");
		cc = abc_add (glwk ,&glwkRec);
		if (cc)
			file_err (cc, "glwk", "DBADD");
	}
	strcpy (glwkRec.narrative,"                    ");
	strcpy (local_rec.prev_item,local_rec.j_ref);
}

int
AddIncc (
	long	hhccHash,
	long	hhbrHash)
{
	char	tempSort [29];

	memset (&incc_rec, 0, sizeof (incc_rec));

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	sprintf (tempSort,"%s%11.11s%-16.16s", inmr_rec.inmr_class,
					       inmr_rec.category,
					       inmr_rec.item_no);
				
	strcpy (incc_rec.sort,tempSort);
	strcpy (incc_rec.stocking_unit,inmr_rec.sale_unit);
	incc_rec.first_stocked 	= TodaysDate ();
	strcpy (incc_rec.stat_flag,"0");
	
	cc = abc_add (incc ,&incc_rec);
	if (cc) 
		return (EXIT_FAILURE);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	return (find_rec (incc ,&incc_rec,COMPARISON,"w"));
}
void
SrchEsmr (
 char *key_val)
{
	work_open ();
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = find_rec (esmr ,&esmr_rec,GTEQ,"r");
	cc = save_rec ("#Br ","#Br Name");
	while (!cc && !strcmp (esmr_rec.co_no,comm_rec.co_no) && 
	              !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.co_no,esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr ,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",temp_str);
	cc = find_rec (esmr ,&esmr_rec,COMPARISON,"r");
	if (cc) 
		file_err (cc, "esmr", "DBFIND");
}

void
SrchCcmr (
 char *key_val)
{
	work_open ();
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,rec_br);
	sprintf (ccmr_rec.cc_no,"%2.2s",key_val);
	cc = find_rec (ccmr ,&ccmr_rec,GTEQ,"r");
	cc = save_rec ("#Wh No.","#Wh Name");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) && 
	              !strcmp (ccmr_rec.est_no,rec_br) && 
	              !strncmp (ccmr_rec.cc_no,key_val,strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no,ccmr_rec.name);
		if (cc)
			break;
		cc = find_rec (ccmr ,&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,rec_br);
	sprintf (ccmr_rec.cc_no,"%2.2s",temp_str);
	cc = find_rec (ccmr ,&ccmr_rec,COMPARISON,"r");
	if (cc) 
		file_err (cc, ccmr, "DBFIND");
}

void
PrintCompanyDetails (
 void)
{
	print_at (21,0,"(Co : %s) Iss Branch . %2.2s %40.40s | W/H : %s %s",comm_rec.co_no,iss_br,iss_brname,iss_wh,iss_whname);

	print_at (22,0,"(Co : %s) Rec Branch . %2.2s %40.40s | W/H : %s %s",comm_rec.co_no,rec_br,rec_brname,rec_wh,rec_whname);
}

/*==========================================================================
| These are functions for the audit printing.                              |
|                                                                          |
| Routine to open output pipe to standard print to provide an audit trail  |
| of events. This also sends the output straight to the spooler.           |
==========================================================================*/
void
OpenPrintAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".11\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".ESTOCK TRANSFERS\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s as at %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B1\n");
	fprintf (fout,".ETransfers From Warehouse %s : Branch %s \n",clip (iss_whname),iss_brname);

	fprintf	(fout, ".R=================");
	fprintf (fout, "===========================");
	fprintf (fout, "===================");
	fprintf (fout, "==================");
	fprintf (fout, "======");
	fprintf (fout, "============");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============");
	fprintf (fout, "==========\n");

	fprintf	(fout, "=================");
	fprintf (fout, "===========================");
	fprintf (fout, "===================");
	fprintf (fout, "==================");
	fprintf (fout, "======");
	fprintf (fout, "============");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "============");
	fprintf (fout, "==========\n");

	fprintf	(fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|  ITEM DESCRIPTION        ");
	fprintf (fout, "| CREDIT ACCOUNT   ");
	fprintf (fout, "| DEBIT ACCOUNT   ");
	fprintf (fout, "| UOM ");
	fprintf (fout, "| QUANTITY  ");
	fprintf (fout, "| LOC FROM ");
	fprintf (fout, "| LOC  TO  ");
	fprintf (fout, "|   @COST  ");
	fprintf (fout, "|  EXTENDED ");
	fprintf (fout, "| BATCH. |\n");

	fprintf	(fout, "|----------------");
	fprintf (fout, "|--------------------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|--------|\n");
	
	fprintf (fout,".PI12\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
ClosePrintAudit (
 void)
{
	if (lpsw == TRUE) 	
	{
		fprintf	(fout, "|----------------");
		fprintf (fout, "|--------------------------");
		fprintf (fout, "|------------------");
		fprintf (fout, "|-----------------");
		fprintf (fout, "|-----");
		fprintf (fout, "|-----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|--------|\n");

		fprintf	(fout, "|  *** TOTAL  ***");
		fprintf (fout, "|                          ");
		fprintf (fout, "|                  ");
		fprintf (fout, "|                 ");
		fprintf (fout, "|     ");
		fprintf (fout, "|%11.2f",	qty_total);
		fprintf (fout, "|          ");
		fprintf (fout, "|          ");
		fprintf (fout, "|          ");
		fprintf (fout, "|%11.2f",	batch_total);
		fprintf (fout, "|        |\n");
		fprintf (fout,".EOF\n");
		pclose (fout);
	}
}

int
CheckClass (void)
{
	if (glmrRec.glmr_class [2] [0] != 'P')
		return print_err (ML (mlStdMess232));
	return (EXIT_SUCCESS);
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
		rv_pr (ML (mlSkMess587),50,0,1);

		line_at (1,0,130);

		if (scn == 1)
		{
			line_at (9,1,130);
			box (0,4,130,7);
		}

		line_at (20,0,130);
		PrintCompanyDetails ();

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*==========================
| Search on UOM (inum)     |
==========================*/
void
SrchInum (
 char    *key_val)
{
	work_open ();
	save_rec ("#UOM ","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{                        
		if (strncmp (inum2_rec.uom_group, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom,inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, inum2, "DBFIND");
}

float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR._cnv_fct;

	return (cnvQty);
}

float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR._cnv_fct;

	return (cnvQty);
}

/*============================                                                  
| Search for valid Location. |                                                  
============================*/                                                  
void                                                                            
SearchLocation (                                                                           
 int     LOC,                                                                
 long    HHWH_HASH,                                                          
 char    *KeyValue)                                                                               
{                                                                               
    char    DispUom [100];                                                       
    float   AllocQty;                                                           
                                                                                
    abc_selfield (inlo, (LOC) ? "inlo_id_loc" : "inlo_id_lot");               
    _work_open (25, (LOC) ? 10 : 7,20);                                           
    inlo_rec.hhwh_hash  =   HHWH_HASH;                                          
    strcpy (inlo_rec.loc_type, " ");                                            
    sprintf (inlo_rec.location,"%-10.10s", "          ");                       
    if (LOC)                                                                    
        cc = save_rec ("#  LOCATION  - LOT NO. ","# UOM. |TYPE| QUANTITY ");   
    else                                                                        
        cc = save_rec ("# LOT NO. -  LOCATION  ","# UOM. |TYPE| QUANTITY ");   
                                                                                
    cc = find_rec (inlo, &inlo_rec, GTEQ,"r");                               
    while (!cc && (inlo_rec.hhwh_hash == HHWH_HASH || HHWH_HASH == 0L))         
    {                                                                           
        if (LOC)                                                                
        {                                                                       
            if (strncmp (inlo_rec.location,KeyValue,strlen (KeyValue)))           
            {                                                                   
                cc = find_rec (inlo, &inlo_rec, NEXT,"r");                   
                continue;                                                       
            }                                                                   
        }                                                                       
        else                                                                    
        {                                                                       
            if (strncmp (inlo_rec.lot_no,KeyValue,strlen (KeyValue)))             
            {                                                                   
                cc = find_rec (inlo, &inlo_rec, NEXT,"r");                   
                continue;                                                       
            }                                                                   
        }                                                                       
        if (inlo_rec.cnv_fct == 0.00)                                           
            inlo_rec.cnv_fct    =   1.00;                                       
                                                                                
        if (LOC)                                                                
        {                                                                       
            sprintf (DispUom, "%-10.10s %-7.7s %010ld %s",                      
                            inlo_rec.location,                                  
                            inlo_rec.lot_no,                                    
                            inlo_rec.hhum_hash,                                 
                            inlo_rec.loc_type);                                 
        }                                                                       
        else                                                                    
        {                                                                       
            sprintf (DispUom, "%-7.7s %-10.10s %010ld %s",                      
                            inlo_rec.lot_no,                                    
                            inlo_rec.location,                                  
                            inlo_rec.hhum_hash,                                 
                            inlo_rec.loc_type);                                 
        }                                                                       
                                                                                
        AllocQty    =   inlo_rec.qty - CalcAlloc (inlo_rec.inlo_hash,line_cnt); 
        sprintf (err_str," %s | %s  |%11.2f",   inlo_rec.uom,                   
                                            inlo_rec.loc_type,                  
                                            AllocQty / inlo_rec.cnv_fct);       
        cc = save_rec (DispUom,err_str);                                        
        if (cc)                                                                 
            break;                                                              
                                                                                
        cc = find_rec (inlo,&inlo_rec,NEXT,"r");                             
    }                                                                           
    cc = disp_srch ();                                                           
    work_close ();                                                               
    if (cc)                                                                     
        return;                                                                 
                                                                                
    abc_selfield (inlo, "inlo_mst_id");                                       
                                                                                
    inlo_rec.hhwh_hash  =   HHWH_HASH;                                          
    inlo_rec.hhum_hash  =   atol (temp_str + 20);                               
    if (LOC)                                                                    
    {                                                                           
        sprintf (inlo_rec.location, "%-10.10s", temp_str);                      
        sprintf (inlo_rec.lot_no,   "%-7.7s",   temp_str + 11);                 
    }                                                                           
    else                                                                        
    {                                                                           
        sprintf (inlo_rec.location,"%-10.10s",  temp_str + 8);                  
        sprintf (inlo_rec.lot_no,   "%-7.7s",   temp_str);                      
    }                                                                           
                                                                                
    sprintf (inlo_rec.loc_type, "%-1.1s", temp_str + 30);                       
                                                                                
    cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");                        
    if (cc)                                                                     
        file_err (cc, inlo, "DBFIND");                                         
}                                                                               
