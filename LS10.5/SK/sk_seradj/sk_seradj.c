/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_seradj.c,v 5.3 2002/06/25 07:30:44 scott Exp $
|  Program Name  : (sk_seradj.c  )                                    |
|  Program Desc  : (Serial Adjustments Add / Update / Delete   )      |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : 03/02/87         |
|                                                                     |
| $Log: sk_seradj.c,v $
| Revision 5.3  2002/06/25 07:30:44  scott
| .
|
| Revision 5.2  2001/08/09 09:19:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:49  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/03/22 07:05:19  scott
| Reversed previous as Licence can be spelt License
|
| Revision 4.2  2001/03/22 06:44:45  scott
| Updated to change spelling of "nncence" to "License"
|
| Revision 4.1  2001/03/22 06:21:04  scott
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.0  2001/03/09 02:38:53  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:32  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:21:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/07/10 01:53:43  scott
| Updated to replace "@(" with "@(" to ensure psl_what works correctly
|
| Revision 1.15  2000/06/13 05:03:30  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.14  2000/04/12 04:55:24  ramon
| Added sleep () calls after displaying the error message.
|
| Revision 1.13  1999/12/06 01:31:18  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/11 06:00:06  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.11  1999/11/03 07:32:33  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.9  1999/10/13 02:42:15  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/08 05:32:53  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:43  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_seradj.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_seradj/sk_seradj.c,v 5.3 2002/06/25 07:30:44 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	ACTUAL		 (insf_rec.stat_flag [0] == 'A')

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/

	FILE	*fout;

	int		printerNumber = 1,
			NewInsf;

	char	*fifteenSpaces	=	"               ",
			*insf2			=	"insf2";

	int		displaySerial = FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct insfRecord	insf_rec;
struct insfRecord	insf2_rec;

#include	<FindSerial.h>

	long	hhsfHash;
	long	hhwhHash;
	char	status [2];
	char	serial_no [26];
	long	date_in;
	long	date_out;
	long	hhcu_hash;
	char	location [11];
	double	exch_rate;
	double	fob_fgn_cst;
	double	fob_nor_cst;
	double	frt_ins_cst;
	double	duty;
	double	licence;
	double	lcost_load;
	double	land_cst;
	double	other_cst;
	double	istore_cost;
	double	prep_cost;
	double	est_cost;
	double	act_cost;
	char	poNumber [sizeof insf_rec.po_number];
	char	grinNumber [sizeof insf_rec.gr_number];
	char	invoice_no [9];
	char	crd_invoice [16];
	char	stat_flag [2];

/*=========================== 
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy [11];
	char	previousItem [17];
	char	wk_desc [41];
	char	sr_item [17];
	char	systemDate [11];
	long	hhsfHash;
	long	hhwhHash;
	char	status [2];
	char	serial_no [26];
	char	new_ser_no [26];
	long	date_in;
	long	date_out;
	long	hhcu_hash;
	char	location [11];
	double	exch_rate;
	double	fob_fgn_cst;
	double	fob_nor_cst;
	double	frt_ins_cst;
	double	duty;
	double	licence;
	double	lcost_load;
	double	land_cst;
	double	other_cst;
	double	istore_cost;
	double	prep_cost;
	double	est_cost;
	double	act_cost;
	double	paid_cost;
	double	pd_rate;
	double	exch_var;
	char	poNumber [sizeof insf_rec.po_number];
	char	grinNumber [sizeof insf_rec.gr_number];
	char	invoice_no [9];
	char	crd_invoice [16];
	char	stat_flag [2];
	char	prmt_fi [26];
	char	stat_desc [31];
	char	costStatDesc [10];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item",	 4, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item no.", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.sr_item},
	{1, LIN, "descr",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description    ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "ser_no",	 7, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Serial No   ", " ",
		 NE, NO,  JUSTLEFT, "", "", insf_rec.serial_no},
	{1, LIN, "new_ser_no",	 8, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "New Serial No", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.new_ser_no},
	{1, LIN, "chasis",	9, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Chasis No.", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.chasis_no},
	{1, LIN, "locn",	 10, 20, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Location", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.location},
	{1, LIN, "pOrderNumber",	12, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Purchase Order", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.po_number},
	{1, LIN, "grinNumber",	13, 20, CHARTYPE,
		"UUUUUUUUNNNNNNN", "          ",
		" ", " ", "G/R Number.", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.gr_number},
	{1, LIN, "invoice",	14, 20, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "Sold on Invoice", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.invoice_no},
	{1, LIN, "crd_inv",	15, 20, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Paid on Invoice", " ",
		YES, NO,  JUSTLEFT, "", "", insf_rec.crd_invoice},
	{1, LIN, "status",	16, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Serial Item Status", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.stat_desc},
	{1, LIN, "des_stat", 17, 20, CHARTYPE, 
		"U", "          ", 
		" ", "I", "Sold Flag (I)", " ", 
		YES, NO, JUSTLEFT, "", "", insf_rec.des_flag}, 
	{1, LIN, "cost_stat", 18, 20, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", "", "Cost Stat A(ct) E(st)", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.costStatDesc}, 
	{1, LIN, "datein", 12, 66, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Date In", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&insf_rec.date_in}, 
	{1, LIN, "dateout", 13, 66, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Date Out", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&insf_rec.date_out}, 
	{2, LIN, "descr",	 3, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.wk_desc},
	{2, LIN, "fob_fgn",	 5, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "FOB (FGN)", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.fob_fgn_cst},
	{2, LIN, "exch_rate",	 6, 24, DOUBLETYPE,
		"NNN.NNN", "          ",
		" ", "", "Exchange Rate", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.exch_rate},
	{2, LIN, "freight",	 7, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Freight & Insurance.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.frt_ins_cst},
	{2, LIN, "fob_nz",	 8, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "CIF (LOC)", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&insf_rec.fob_nor_cst},
	{2, LIN, "duty",	 9, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Duty", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.duty},
	{2, LIN, "licence",	10, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Licence", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.licence},
	{2, LIN, "load_cst",	11, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Landed Cost loading.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.lcost_load},
	{2, LIN, "other",	12, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Other Costs", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.other_cst},
	{2, LIN, "into_store",	13, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Into Store Cost", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&insf_rec.istore_cost},
	{2, LIN, "prep_cost",	14, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Preparation Cost", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.prep_cost},
	{2, LIN, "est_cost",	15, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Estimated Cost", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.est_cost},
	{2, LIN, "exch_var",	17, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Exchange Variance", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.exch_var},
	{2, LIN, "act_cost",	18, 24, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Actual Cost", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&insf_rec.act_cost},
	{2, LIN, "pd_rate", 5, 60, DOUBLETYPE, 
		"NNN.NNN", "          ", 
		" ", "", "Paid  Rate", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&insf_rec.pd_rate}, 
	{2, LIN, "paid_cost", 7, 60, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", "Paid  Cost", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&insf_rec.paid_cost}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ReadMisc 			 (void);
int  	spec_valid 			 (int);
void 	Update 				 (void);
void 	UpdateInsf 			 (long);
void 	OpenAudit 			 (void);
void 	CloseAudit 			 (void);
void 	SrckInsf 			 (char *);
void 	CalculateAll 		 (int);
void 	CheckInsf 			 (void);
int  	heading 			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	field;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/

	if (argc > 1)
		printerNumber = atoi (argv [1]);

	if (!strncmp (argv [0], "sk_dser",7))
	{
		for (field = label ("new_ser_no");FIELD.scn != 0;field++)
			FIELD.required = NA;

		displaySerial = TRUE;
	}
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*----------------------- 
	| Open database files . |
	-----------------------*/
	OpenDB ();

	/*--------------------------
	| Start audit pipe output. |
	--------------------------*/
	OpenAudit ();

	sprintf (local_rec.previousItem, "%16.16s", " ");

	while (prog_exit == 0) 
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		search_ok 	= TRUE;

		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (NewInsf)
		{
			heading (2);
			entry (2);
			if (restart)
				continue;
		}
		else
			scn_display (1);

		/* enter edit mode */

		edit_all ();
		if (restart)
			continue;

		if (!displaySerial)
			Update ();

		strcpy (local_rec.previousItem,local_rec.sr_item);
	}
	shutdown_prog ();	
    return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	CloseAudit ();

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
	abc_dbopen ("data");
	ReadMisc ();

	abc_alias (insf2, insf);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (insf2,insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (insf);
	abc_fclose (insf2);
	abc_fclose (incc);
	SearchFindClose ();
	abc_dbclose ("data");
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("ccmr", ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

	abc_fclose ("ccmr");
}

int
spec_valid (
 int field)
{
	int	no_modify = FALSE;

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item"))
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
		cc = FindInmr (comm_rec.co_no, local_rec.sr_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.sr_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}

		DSP_FLD ("item");
		DSP_FLD ("descr");

		SuperSynonymError ();

		if (inmr_rec.serial_item [0] != 'Y')
		{
			sprintf (err_str, ML (mlSkMess560), inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec ("incc", &incc_rec, COMPARISON, "r");

		local_rec.hhwhHash = incc_rec.hhwh_hash;

		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("ser_no"))
	{
		if (SRCH_KEY)
		{
			SrckInsf (temp_str);
			return (EXIT_SUCCESS);
		}

		cc = FindInsf (local_rec.hhwhHash, insf_rec.serial_no, "F", "u");
		if (cc)
		{
			cc = FindInsf (local_rec.hhwhHash, 
				  	  insf_rec.serial_no, "C", "u");
		}
		if (cc)
		{
			cc = FindInsf (local_rec.hhwhHash, 
				  	  insf_rec.serial_no, "T", "u");
		}
		if (cc)
		{
			cc = FindInsf (local_rec.hhwhHash, 
				  	  insf_rec.serial_no, "S", "u");
			no_modify = TRUE;
		}
		if (cc)
		{
			/*Ser # %s not on file.",insf_rec.serial_no*/
			errmess (ML (mlStdMess201));
			return (EXIT_FAILURE);
		}
		if (no_modify)
		{
			for (field = label ("datein");FIELD.scn != 0;field++)
				FIELD.required = NA;
			
			FLD ("new_ser_no") = NA;
		}

		if (insf_rec.receipted [0] == 'N' && !displaySerial)
		{
			for (field = label ("datein");FIELD.scn != 0;field++)
				FIELD.required = NA;
			
			/*NOTE : Changes cannot be made as serial no is not receipted.*/
			print_mess (ML (mlSkMess120));
			sleep (sleepTime);
			displaySerial = TRUE;
		}
		strcpy (local_rec.serial_no,insf_rec.serial_no);
		strcpy (local_rec.location,insf_rec.location);
		entry_exit = 1;
		NewInsf = FALSE;

		CheckInsf ();
		CalculateAll (FALSE);

		/*-------------------------------
		|	initialise old values	|
		-------------------------------*/
		local_rec.hhsfHash   	= insf_rec.hhsf_hash;
		local_rec.hhwhHash   	= insf_rec.hhwh_hash;
		local_rec.date_in     	= insf_rec.date_in;
		local_rec.date_out    	= insf_rec.date_out;
		local_rec.hhcu_hash   	= insf_rec.hhcu_hash;
		local_rec.exch_rate   	= insf_rec.exch_rate;
		local_rec.fob_fgn_cst 	= insf_rec.fob_fgn_cst;
		local_rec.fob_nor_cst 	= insf_rec.fob_nor_cst;
		local_rec.frt_ins_cst 	= insf_rec.frt_ins_cst;
		local_rec.duty        	= insf_rec.duty;
		local_rec.licence     	= insf_rec.licence;
		local_rec.lcost_load  	= insf_rec.lcost_load;
		local_rec.land_cst    	= insf_rec.land_cst;
		local_rec.other_cst   	= insf_rec.other_cst;
		local_rec.istore_cost 	= insf_rec.istore_cost;
		local_rec.prep_cost   	= insf_rec.prep_cost;
		local_rec.est_cost    	= insf_rec.est_cost;
		local_rec.act_cost    	= insf_rec.act_cost;
		local_rec.exch_var    	= insf_rec.exch_var;
		local_rec.pd_rate	  	= insf_rec.pd_rate;
		local_rec.paid_cost     	= insf_rec.paid_cost;

		strcpy (local_rec.poNumber,		insf_rec.po_number);
		strcpy (local_rec.grinNumber,	insf_rec.gr_number);
		strcpy (local_rec.invoice_no,	insf_rec.invoice_no);
		strcpy (local_rec.crd_invoice,	insf_rec.crd_invoice);
		strcpy (local_rec.stat_flag,	insf_rec.stat_flag);

		if (ACTUAL)
			strcpy (local_rec.costStatDesc, "Actual   ");
		else
			strcpy (local_rec.costStatDesc, "Estimated");

		if (strcmp (insf_rec.po_number, fifteenSpaces) || no_modify || displaySerial)
			FLD ("pOrderNumber") = NA;
		else
			FLD ("pOrderNumber") = YES;

		if (strcmp (insf_rec.gr_number, fifteenSpaces) || displaySerial)
			FLD ("grinNumber") = NA;
		else
			FLD ("grinNumber") = YES;

		if (strcmp (insf_rec.invoice_no, "        ") || no_modify || displaySerial)
			FLD ("invoice") = NA;
		else
			FLD ("invoice") = YES;

		if (strcmp (insf_rec.crd_invoice, "               ") || displaySerial)
			FLD ("crd_inv") = NA;
		else
			FLD ("crd_inv") = YES;

		FLD ("est_cost") = NA;
		FLD ("act_cost") = NA;
		FLD ("paid_cost")  = NA;
		FLD ("exch_var") = (ACTUAL) ? YES : NA;
		FLD ("other")    = (ACTUAL) ? YES : NA;

		if (ACTUAL)
		{
			FLD ("est_cost")	= ND;
			FLD ("pd_rate")		= YES;
		}
		else
		{
			FLD ("act_cost")	= ND;
			FLD ("pd_rate")	= ND;
			FLD ("paid_cost")	= ND;
		}
		sprintf (local_rec.stat_desc,"%-30.30s"," ????.");
		if (insf_rec.status [0] == 'F')
			sprintf (local_rec.stat_desc,"%-30.30s"," Free.");

		if (insf_rec.status [0] == 'C')
			sprintf (local_rec.stat_desc,"%-30.30s"," Committed.");

		if (insf_rec.status [0] == 'S')
			sprintf (local_rec.stat_desc,"%-30.30s"," Sold.");

		if (insf_rec.status [0] == 'I')
			sprintf (local_rec.stat_desc,"%-30.30s"," In Transit.");

		DSP_FLD ("status");
		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| Check If New Serial No already exists |
	---------------------------------------*/
	if (LCHECK ("new_ser_no"))
	{
		insf2_rec.hhwh_hash = local_rec.hhwhHash;
		strcpy (insf2_rec.status, "F");
		sprintf (insf2_rec.serial_no,"%-25.25s",local_rec.new_ser_no);
		cc = find_rec (insf2,&insf2_rec,COMPARISON,"r");
		if (!cc)
		{
			sprintf (err_str, ML (mlStdMess097), insf2_rec.serial_no);
			errmess (err_str);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("descr"))
	{
		strcpy (local_rec.wk_desc, inmr_rec.description);
		display_field (field);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fob_fgn"))
	{
		if (last_char == EOI) 
		{
			entry_exit = 1;
			edit_exit = 1;
			return (EXIT_SUCCESS);
		}
		CalculateAll (TRUE);
	}

	if (LCHECK ("exch_rate"))
	{
		if (insf_rec.exch_rate == 0.00)
		{
			/*Exchange Rate cannot be 0.00*/
			print_mess (ML (mlStdMess044));
			return (EXIT_FAILURE);
		}
		CalculateAll (TRUE);
	}
	if (LCHECK ("freight"))
	{
		print_mess (ML (mlSkMess119));
		sleep (sleepTime);
		CalculateAll (TRUE);

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("duty"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("licence"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("load_cst"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("other"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("prep_cost"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("exch_var"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pd_rate"))
	{
		CalculateAll (TRUE);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*===============================
| update insf file              |
===============================*/
void
Update (
 void)
{
	clear ();
	/*Updating Serial # %s\n\r",insf_rec.serial_no*/
	print_at (0,0,ML (mlSkMess088),insf_rec.serial_no);fflush (stdout);

	/*-----------------------
	|	write audit	        |
	-----------------------*/
	fprintf (fout, "|%-16.16s", inmr_rec.item_no);
	fprintf (fout, "|%-21.21s", inmr_rec.description);
	fprintf (fout, "|%-25.25s", insf_rec.serial_no);
	fprintf (fout, "|%-10.10s", DateToString (local_rec.date_in));
	fprintf (fout, "|%-10.10s", DateToString (local_rec.date_out));
	fprintf (fout, "|%-15.15s", local_rec.poNumber);
	fprintf (fout, "|%-15.15s", local_rec.grinNumber);
	fprintf (fout, "|%-8.8s", local_rec.invoice_no);
	fprintf (fout, "|%-15.15s", local_rec.crd_invoice);
	fprintf (fout, "|  %-1.1s   ",insf_rec.status);
	fprintf (fout, "|%-10.10s|\n", local_rec.location);

	fprintf (fout, "|%-16.16s", " ");
	fprintf (fout, "|%-21.21s", " ");
	fprintf (fout, "|%-25.25s", insf2_rec.serial_no);
	fprintf (fout, "|%-10.10s", DateToString (insf_rec.date_in));
	fprintf (fout, "|%-10.10s", DateToString (insf_rec.date_out));
	fprintf (fout, "|%-15.15s", insf_rec.po_number);
	fprintf (fout, "|%-15.15s", insf_rec.gr_number);
	fprintf (fout, "|%-8.8s", insf_rec.invoice_no);
	fprintf (fout, "|%-15.15s", insf_rec.crd_invoice);
	fprintf (fout, "|  %-1.1s   ",insf_rec.status);
	fprintf (fout, "|%-10.10s|\n", insf_rec.location);

	fprintf (fout, "|============");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "=========");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===============");
	fprintf (fout, "=============");
	fprintf (fout, "==============|\n");

	fprintf (fout, "| FOB (FGN)  ");
	fprintf (fout, "|EXCH RATE");
	fprintf (fout, "| FRI + INS  ");
	fprintf (fout, "| CIF (LOC)  ");
	fprintf (fout, "|  DUTY  ");
	fprintf (fout, "| LICENCE ");
	fprintf (fout, "| LCOST LOAD ");
	fprintf (fout, "|OTHER COSTS ");
	fprintf (fout, "|INTO SR COST");
	fprintf (fout, "| PREP COSTS ");
	fprintf (fout, "|ESTIMATE  COST");
	fprintf (fout, "| EXCH  VAR. ");
	fprintf (fout, "| ACTUAL COST |\n");

	fprintf (fout, "|============");
	fprintf (fout, "|=========");
	fprintf (fout, "|============");
	fprintf (fout, "|============");
	fprintf (fout, "|========");
	fprintf (fout, "|=========");
	fprintf (fout, "|============");
	fprintf (fout, "|============");
	fprintf (fout, "|============");
	fprintf (fout, "|============");
	fprintf (fout, "|==============");
	fprintf (fout, "|============");
	fprintf (fout, "|=============|\n");

	fprintf (fout, "|%11.2f ", local_rec.fob_fgn_cst);
	fprintf (fout, "|%9.4f",   local_rec.exch_rate);
	fprintf (fout, "|%11.2f ", local_rec.frt_ins_cst);
	fprintf (fout, "|%11.2f ", local_rec.fob_nor_cst);
	fprintf (fout, "|%7.2f ",  local_rec.duty);
	fprintf (fout, "|%8.2f ",  local_rec.licence);
	fprintf (fout, "|%11.2f ", local_rec.lcost_load);
	fprintf (fout, "|%11.2f ", local_rec.other_cst);
	fprintf (fout, "|%11.2f ", local_rec.istore_cost);
	fprintf (fout, "|%11.2f ", local_rec.prep_cost);
	fprintf (fout, "|%13.2f ", local_rec.est_cost);
	fprintf (fout, "|%11.2f ", local_rec.exch_var);
	fprintf (fout, "|%12.2f |\n", local_rec.act_cost);

	fprintf (fout, "|%11.2f ", insf_rec.fob_fgn_cst);
	fprintf (fout, "|%9.4f",   insf_rec.exch_rate);
	fprintf (fout, "|%11.2f ", insf_rec.frt_ins_cst);
	fprintf (fout, "|%11.2f ", insf_rec.fob_nor_cst);
	fprintf (fout, "|%7.2f ",  insf_rec.duty);
	fprintf (fout, "|%8.2f ",  insf_rec.licence);
	fprintf (fout, "|%11.2f ", insf_rec.lcost_load);
	fprintf (fout, "|%11.2f ", insf_rec.other_cst);
	fprintf (fout, "|%11.2f ", insf_rec.istore_cost);
	fprintf (fout, "|%11.2f ", insf_rec.prep_cost);
	fprintf (fout, "|%13.2f ", insf_rec.est_cost);
	fprintf (fout, "|%11.2f ", insf_rec.exch_var);
	fprintf (fout, "|%12.2f |\n", insf_rec.act_cost);

	fprintf (fout, "|------------");
	fprintf (fout, "----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "---------");
	fprintf (fout, "----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "---------------");
	fprintf (fout, "-------------");
	fprintf (fout, "--------------|\n");

	cc = abc_update ("insf", &insf_rec);
	if (cc) 
		sys_err ("Error in insf During (DBADD)", cc, PNAME);

	abc_unlock ("insf");

	if (strcmp (local_rec.new_ser_no,"                         "))
		UpdateInsf (insf_rec.hhwh_hash);

}

void
UpdateInsf (
	long	hhwhHash)
{
	printf ("Changing Serial No from %s to %s",
			insf_rec.serial_no,local_rec.new_ser_no);

	abc_selfield (insf2,"insf_hhwh_hash");
	cc = find_hash (insf2, &insf2_rec, GTEQ, "u", hhwhHash);
	while (!cc && insf2_rec.hhwh_hash == hhwhHash)
	{
		if (!strcmp (insf2_rec.serial_no, insf_rec.serial_no) &&
			insf2_rec.status [0] == insf_rec.status [0]) 
		{
			sprintf (insf2_rec.serial_no, "%-25.25s", local_rec.new_ser_no);
			cc = abc_update (insf2, &insf2_rec);
			if (cc) 
				sys_err ("Error in insf2 During (DBUPDATE)", cc, PNAME);
			break;
		}
		abc_unlock (insf2);
		cc = find_hash (insf2, &insf2_rec, NEXT, "u", hhwhHash);
	}
	abc_unlock (insf2);

	abc_selfield (insf2,"insf_id_no");

	return;
}

/*=======================================================
|	Routine to open output pipe to standard print to	|
|	provide an audit trail of events.				    |
|	This also sends the output straight to the spooler.	|
=======================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".13\n");
	fprintf (fout,".PI16\n");
	fprintf (fout,".L170\n");
	fprintf (fout,".ESERIAL STOCK ADJUSTMENTS\n");
	fprintf (fout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".CNOTE : If costs are changed General ledger Journals shold be done as this option is not interfaced.\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EBRANCH %s : Warehouse %s \n",clip (comm_rec.est_name),clip (comm_rec.cc_name));

	fprintf (fout, ".R=================");
	fprintf (fout, "======================");
	fprintf (fout, "==========================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "=========");
	fprintf (fout, "================");
	fprintf (fout, "=======");
	fprintf (fout, "============\n");

	fprintf (fout, "=================");
	fprintf (fout, "======================");
	fprintf (fout, "==========================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "=========");
	fprintf (fout, "================");
	fprintf (fout, "=======");
	fprintf (fout, "============\n");

	fprintf (fout, "|   ITEM NUMBER  ");
	fprintf (fout, "|  ITEM DESCRIPTION   ");
	fprintf (fout, "|      SERIAL NUMBER      ");
	fprintf (fout, "| DATE IN  ");
	fprintf (fout, "| DATE OUT ");
	fprintf (fout, "|PURCHASE ORDER ");
	fprintf (fout, "|GOODS  RECEIPT ");
	fprintf (fout, "| INV NO ");
	fprintf (fout, "|SUPPLIERS INV #");
	fprintf (fout, "| STAT ");
	fprintf (fout, "| LOCATION |\n");

	fprintf (fout, "|----------------");
	fprintf (fout, "|---------------------");
	fprintf (fout, "|-------------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|------");
	fprintf (fout, "|----------|\n");
}

/*===================================================
|	Routine to close the audit trail output file.	|
===================================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}


/*===========================
| Search for serial number. |
===========================*/
void
SrckInsf (
	char	*keyValue)
{
	char	*ser_no = clip (keyValue);

	_work_open (25,0,40);
	save_rec ("#Serial No", "#Item Description");
	cc = FindInsf (local_rec.hhwhHash, "", "F", "r");
	while (!cc && local_rec.hhwhHash == insf_rec.hhwh_hash)
	{
	    if (strncmp (ser_no,insf_rec.serial_no,strlen (ser_no)) < 0)
		break;

	    if (!strncmp (ser_no,insf_rec.serial_no,strlen (ser_no)))
	    {
		cc = save_rec (insf_rec.serial_no, inmr_rec.description);
		if (cc)
			break;
	    }
	    cc = FindInsf (0L, "", "F", "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	ser_no = clip (keyValue);
	cc = FindInsf (local_rec.hhwhHash, ser_no, "F", "r");
}

void
CalculateAll (
 int draw)
{
	int	i;

	if (insf_rec.exch_rate == 0.00)
		insf_rec.exch_rate = 1.00;

	if (ACTUAL)
	{
		insf_rec.fob_nor_cst = insf_rec.fob_fgn_cst / 
				  				  insf_rec.exch_rate;

		insf_rec.fob_nor_cst += insf_rec.frt_ins_cst;
	}
	else
	{
		insf_rec.fob_nor_cst	=	 (insf_rec.fob_fgn_cst +
								  	insf_rec.frt_ins_cst) / 
				  				  	insf_rec.exch_rate;
	}

	insf_rec.istore_cost	=	insf_rec.fob_nor_cst +
			  	  			  	insf_rec.licence +
			  	  			  	insf_rec.duty +
			  	  			  	insf_rec.lcost_load +
			  	  			  	insf_rec.prep_cost;

	if (ACTUAL)
	{

		insf_rec.act_cost = insf_rec.istore_cost +
				       		   insf_rec.other_cst;
	
		if (insf_rec.pd_rate	< 0.0001)
			insf_rec.pd_rate =	0.00;
		else
		{
			insf_rec.paid_cost = ((insf_rec.fob_fgn_cst /
									insf_rec.pd_rate) - (
									insf_rec.fob_fgn_cst /
									insf_rec.exch_rate)) +
									insf_rec.act_cost;	
		}
	}
	else
		insf_rec.est_cost = insf_rec.istore_cost + insf_rec.other_cst;
	
	if (draw)
	{
		for (i = label ("fob_fgn");i <= label ("paid_cost");i++)
			display_field (i);
	}
}

void
CheckInsf (
 void)
{
	double	estimate 	= 0.00,
			actual   	= 0.00,
			fob_nor_cst = 0.00,
			istore_cost = 0.00;

	if (insf_rec.exch_rate == 0.00)
		insf_rec.exch_rate = 1.00;

	if (ACTUAL)
	{
		fob_nor_cst = insf_rec.fob_fgn_cst / insf_rec.exch_rate;
		fob_nor_cst += insf_rec.frt_ins_cst;
	}
	else
	{
		fob_nor_cst 	= 	 (insf_rec.fob_fgn_cst +
		 	        		  insf_rec.frt_ins_cst) / 
			        		  insf_rec.exch_rate;
	}
	istore_cost	=	insf_rec.fob_nor_cst +
			  		insf_rec.licence +
			  		insf_rec.duty +
			  		insf_rec.prep_cost;
	if (ACTUAL)
	{
		actual = istore_cost;

		if (actual != insf_rec.act_cost)
		{
			if (fob_nor_cst == 0.00)
				insf_rec.fob_fgn_cst = insf_rec.act_cost;
			else
				insf_rec.other_cst = insf_rec.act_cost - actual;
		}
	}
	else
	{
		estimate = istore_cost + insf_rec.lcost_load;

		if (estimate != insf_rec.est_cost)
		{
			if (fob_nor_cst == 0.00)
				insf_rec.fob_fgn_cst = insf_rec.est_cost;
			else
				insf_rec.other_cst = insf_rec.est_cost - estimate;
		}
	}
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	if (displaySerial)
		rv_pr (ML (mlSkMess086),17,0,1);
	else
		rv_pr (ML (mlSkMess087),20,0,1);
	
	print_at (0,50,ML (mlSkMess089),local_rec.previousItem);
	move (0,1);
	line (80);
	if (scn == 1)
	{
		box (0,3,80,15);
		line_at (6,1,79);
		line_at (11,1,79);
	}
	else
	{
		box (0,2,80,16);
		line_at (4,1,79);
	}

	print_at (21,0, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039),comm_rec.est_no,comm_rec.est_short);
	print_at (22,40,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_short);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

