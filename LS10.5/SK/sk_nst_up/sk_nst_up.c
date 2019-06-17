/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (sk_nst_up.c   )                                   |
|  Program Desc  : (Stock Take Input.                           )     |
| $Id: sk_nst_up.c,v 5.4 2002/07/17 09:57:58 scott Exp $
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 28/03/89         |
|---------------------------------------------------------------------|
| $Log: sk_nst_up.c,v $
| Revision 5.4  2002/07/17 09:57:58  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:19:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:31  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:18  scott
| Update - LS10.5
|
| Revision 4.0  2001/03/09 02:38:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/02/06 10:07:45  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
| Revision 3.2  2000/11/22 00:53:20  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.1  2000/11/20 07:40:22  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/01 05:12:05  scott
| General maintenance - added app.schema
|
| Revision 2.0  2000/07/15 09:11:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.30  2000/01/26 06:47:52  ana
| (26/01/2000) SC15866/2403 Read STAKE VAR in the category level.
|
| Revision 1.29  1999/12/06 01:31:06  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.28  1999/11/19 05:23:30  scott
| Updated for get_mend and get_mbeg
|
| Revision 1.27  1999/11/11 05:59:59  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.26  1999/11/02 02:14:06  gerry
| (02/11/1999) Corrected updating.
|
| Revision 1.25  1999/10/12 21:20:36  scott
| Updated by Gerry from ansi project.
|
| Revision 1.24  1999/10/08 05:32:44  scott
| First Pass checkin by Scott.
|
| Revision 1.23  1999/09/30 23:44:54  scott
| Updated to use intr_id_no2 instead of intr_id_no
|
| Revision 1.22  1999/09/11 02:03:32  scott
| Updated from continued testing
|
| Revision 1.21  1999/09/01 04:26:30  scott
| Updated as needs to clear inlo ;
|
| Revision 1.20  1999/06/17 08:21:15  ronnel
| 17/06/1999 Modified to consider when zero entered in stock take input ST_UP_ZERO = 2
|
| Revision 1.19  1999/06/15 09:43:23  scott
| Updated to clear itlo_stake.
|
| Revision 1.18  1999/06/15 07:18:00  scott
| Updated to fix problem with stock take and location of zero counted.
|$Log: sk_nst_up.c,v $
|Revision 5.4  2002/07/17 09:57:58  scott
|Updated to change argument to get_lpno from (1) to (0)
|
|Revision 5.3  2001/08/09 09:19:30  scott
|Updated to add FinishProgram () function
|
|Revision 5.2  2001/08/06 23:45:31  scott
|RELEASE 5.0
|
|Revision 5.1  2001/07/25 02:19:18  scott
|Update - LS10.5
|
|Revision 4.0  2001/03/09 02:38:19  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.3  2001/02/06 10:07:45  scott
|Updated to deal with length change in the following fields
|intr_ref1 from 10 to 15 characters
|intr_ref2 from 10 to 15 characters
|inaf_ref1 from 10 to 15 characters
|inaf_ref2 from 10 to 15 characters
|
|Revision 3.2  2000/11/22 00:53:20  scott
|New features related to 3PL environment
|New features related to Number Plates
|All covered in release 3 notes
|
|Revision 3.1  2000/11/20 07:40:22  scott
|New features related to 3PL environment
|New features related to Number Plates
|All covered in release 3 notes
|
|Revision 3.0  2000/10/10 12:20:51  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.1  2000/08/01 05:12:05  scott
|General maintenance - added app.schema
|
|Revision 2.0  2000/07/15 09:11:33  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.30  2000/01/26 06:47:52  ana
| (26/01/2000) SC15866/2403 Read STAKE VAR in the category level.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "sk_nst_up";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_up/sk_nst_up.c,v 5.4 2002/07/17 09:57:58 scott Exp $";

#include 	<pslscr.h>
#include	<GlUtils.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		FIFO 	 (inmr_rec.costing_flag [0] == 'F')
#define		LIFO 	 (inmr_rec.costing_flag [0] == 'I')
#define		LCST 	 (inmr_rec.costing_flag [0] == 'L')
#define		AVGE 	 (inmr_rec.costing_flag [0] == 'A')
#define		SERIAL	 (inmr_rec.serial_item [0] == 'Y')

#define		LEAVE		0
#define		ZERO_COUNT	1
#define		ZERO_ALL	2
#define		NDEC(x)		n_dec (x, inmr_rec.dec_pt)

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct comrRecord	comr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct sttfRecord	sttf_rec;
struct sttfRecord	sttf2_rec;
struct intrRecord	intr_rec;
struct inumRecord	inum_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;

	/*=============
	| Table Names |
	=============*/
	static char 	*data	= "data", 
					*inlo2	= "inlo2", 
					*sttf2	= "sttf2";


	char	glSuspenceAcct	[MAXLEVEL + 1], 
			glDebitAcct		[MAXLEVEL + 1], 
			glCreditAcct	[MAXLEVEL + 1],
			*envVarInvalidClasses,
 			*result;

	long	glSuspenceHash	= 0L, 
			glDebitHash		= 0L, 
			glCreditHash	= 0L,
			postDate 		= 0L;

	float	oldQuantity = 0.00, 
			newQuantity = 0.00;

	double	oldValue = 0.00, 
			newValue = 0.00;

	float	totalStockTake = 0.00, 
			oldStockValue = 0.00, 
			newStockValue = 0.00, 
			totalQuantity = 0.00;

	double	totalValue = 0.00;

	int		envVarStockTakeUpdate = 0,
			envVarCostSales		  = 0,
			GlPeriod = 0;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	loc_curr [4];
	char	audit [6];
	int		printerNumber;
	char	item_desc [31];
	float	qty;
	char	location [11];
	char	dflt_qty [15];
	char	rep_qty [30];
	char	batchString [8];
	char	invalidString [8];
} local_rec;

static	struct	var	vars [] =
{
	{ 1, LIN, "stake_code", 	 4, 25, CHARTYPE, 
		"U", "          ", 
		" ", "", "Stock Selection Code.", "", 
		 NE, NO,  JUSTLEFT, "", "", inscRec.stake_code }, 
	{ 1, LIN, "stake_desc", 	 5, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Stock Selection Desc", "", 
		 NA, NO,  JUSTLEFT, "", "", inscRec.description }, 
	{ 1, LIN, "stake_date", 	 6, 25, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Start Date", "", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&inscRec.start_date },
	{ 1, LIN, "stake_time", 	 7, 25, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Start Time", "", 
		 NA, NO,  JUSTLEFT, "", "", inscRec.start_time },
	{ 1, LIN, "postDate", 	 9, 25, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Inventory Posting Date", "", 
		 YES, NO,  JUSTLEFT, "", "", (char *)&postDate },
	{ 1, LIN, "GlPeriod", 	 9, 64, INTTYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "General Ledger Period.", "", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&GlPeriod },
	{ 1, LIN, "printerNumber", 	 10, 25, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber },
	{ 0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy }
};

#include <LocHeader.h>

/*=======================
| Function Declarations |
=======================*/
float 	TotalLocations 		(long);
int  	ProcLotLocation 	(long);
int  	heading 			(int);
int  	spec_valid 			(int);
static 	int FloatZero 		(float);
static 	int MoneyZero 		(double);
void 	AddGlwk 			(float, double);
void 	AddIntr 			(float, double);
void 	ClearLocations 		(long);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
void 	DeleteInsc 			(void);
void 	GetAccounts 		(char *);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	Proc_inwu 			(long);
void 	ProcessFifo 		(int, float);
void 	ProcessOtherStuff 	(void);
void 	ReadGLFiles 		(void);
void 	SrchInsc 			(void);
void 	Update 				(void);
void 	Update_inwu 		(long, long, float);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
			
	int		after,
			before;

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);
	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	envVarStockTakeUpdate 		= atoi (get_env ("ST_UPZERO"));

	sptr = chk_env ("COST_SALES");
	if (sptr == (char *)0)
		envVarCostSales = FALSE;
	else
		envVarCostSales = (sptr [0] == 'Y' || sptr [0] == 'y') ? TRUE : FALSE; 
		
	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		envVarInvalidClasses = strdup (sptr);
	else
		envVarInvalidClasses = strdup ("ZKPN");

	upshift (envVarInvalidClasses); 

	strcpy (local_rec.batchString, "  N/A  ");
	strcpy (local_rec.invalidString, "INVALID");
	init_scr ();
	set_tty ();

	OpenDB ();

	/*-------------------------------------------------------
	| Program run with arguments so in in interactive mode. |
	-------------------------------------------------------*/
	if (argc > 1)
	{
		inscRec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		sprintf (inscRec.stake_code, "%-1.1s", argv [1]);
		cc = find_rec (insc, &inscRec, COMPARISON, "r");
		if (cc)
		{
			print_at (0,0,ML (mlSkMess047), argv [1] [0]);
			sleep (10);
			shutdown_prog ();
		}

		if (inscRec.serial_take [0] == 'Y')
		{
			print_at (0,0,ML (mlSkMess042), argv [1] [0]);
			sleep (10);
			shutdown_prog ();
		}

		Update ();
	}
	else
	{
		SETUP_SCR (vars);

		set_masks ();
		init_vars (1);

		while (prog_exit == 0)
		{
			entry_exit	= FALSE;
			prog_exit	= FALSE;
			restart		= FALSE;
			search_ok	= TRUE;
	
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;
	
			edit_all ();
			if (restart)
				continue;

			Update ();
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	clear 	 ();
	CloseDB  ();
	rset_tty ();
	crsr_on  ();
}

int
spec_valid (
 int field)
{
	int		i, monthNum;

	/*---------------------------
	| Validate Stock Take code. |
	---------------------------*/
	if (LCHECK ("stake_code"))
	{
		if (SRCH_KEY)
		{
			SrchInsc ();
			return (EXIT_SUCCESS);
		}
		inscRec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		cc = find_rec (insc, &inscRec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, ML (mlSkMess047), inscRec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (inscRec.serial_take [0] == 'Y')
		{
			sprintf (err_str, ML (mlSkMess042), inscRec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("stake_desc");
		DSP_FLD ("stake_date");
		DSP_FLD ("stake_time");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Printer Number. |
	--------------------------*/
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
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("postDate"))
	{
		if (dflt_used)
			postDate = comm_rec.inv_date;

		DSP_FLD ("postDate");

		DateToDMY (postDate, NULL, &monthNum, NULL);

		GlPeriod = mth2fisc (monthNum, comm_rec.fiscal);

		DSP_FLD ("GlPeriod");

		if (postDate < MonthStart (comm_rec.inv_date) ||
		     postDate > MonthEnd (comm_rec.inv_date))
		{
			for (i = 0; i < 4; i++)
			{
				print_mess (ML (mlSkMess107));
				sleep (sleepTime);
			}
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);

	abc_alias (sttf2, sttf);
	abc_alias (inlo2, inlo);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no3");
	open_rec (sttf2,sttf_list, STTF_NO_FIELDS, "sttf_id_no3");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (inlo2,inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (local_rec.loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);
	
	OpenGlmr ();
	OpenLocation (ccmr_rec.hhcc_hash);
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenInsc ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (intr);
	abc_fclose (inum);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (sttf);
	abc_fclose (inwu);
	abc_fclose (inlo);
	abc_fclose (inlo2);
	CloseLocation ();
	GL_CloseBatch (local_rec.printerNumber);
	GL_Close ();
	CloseCosting ();
	abc_dbclose (data);
}

void
ReadGLFiles (void)
{
	if (envVarCostSales)
	{
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"STAKE VAR.",
			" ",
			inmr_rec.category
		);
		glDebitHash = glmrRec.hhmr_hash;
		strcpy (glDebitAcct, glmrRec.acc_no);

		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"SUSPENSE  ",
			" ",
			" "
		);
		glSuspenceHash = glmrRec.hhmr_hash;
		strcpy (glSuspenceAcct, glmrRec.acc_no);
	}
}

/*=============================================
| Search Routine for Stock take Control File. |
=============================================*/
void
SrchInsc (
 void)
{
	_work_open (1,0,40);
	save_rec ("#C", "#    Stock Take Selection Description    ");
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (inscRec.stake_code, " ");
	cc = find_rec (insc, &inscRec, GTEQ, "r");
	while (!cc && inscRec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (inscRec.serial_take [0] != 'Y')
		{
			cc = save_rec (inscRec.stake_code, inscRec.description);
			if (cc)
				break;
		}

		cc = find_rec (insc, &inscRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (inscRec.stake_code, temp_str);
	cc = find_rec (insc, &inscRec, COMPARISON, "r");
	if (cc)
		file_err (cc, insc, "DBFIND");
}

/*====================
| Update inmr / incc |
====================*/
void
Update (
 void)
{
	dsp_screen ("Updating Stock Take Figures", 
					comm_rec.co_no, comm_rec.co_name);
	OpenAudit ();

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (incc_rec.sort, "                            ");
	
	cc = find_rec (incc, &incc_rec, GTEQ, "u");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash) 
	{
		if (incc_rec.stat_flag [0] != inscRec.stake_code [0])
		{
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");

		result = strstr (envVarInvalidClasses, inmr_rec.inmr_class);
		if (result)
		{
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		if (SERIAL || cc)
		{
			abc_unlock (inmr);
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}
		dsp_process ("Item : ", inmr_rec.item_no);

		ReadGLFiles ();
		/*--------------------------------------
		| Process and Update Location records. |
		--------------------------------------*/
		if (ProcLotLocation (incc_rec.hhwh_hash))
		{
			/*---------------------------------------------------------
			| Leave as if stock counted as per current closing stock. |
			---------------------------------------------------------*/
			if (envVarStockTakeUpdate == LEAVE)
				totalStockTake = incc_rec.stake;

			/*-----------------------------------------
			| If Update stock as if Zero was counted. |
			-----------------------------------------*/
			if (envVarStockTakeUpdate == ZERO_COUNT)
			{
				if (MULT_LOC || SK_BATCH_CONT)
					ClearLocations (incc_rec.hhwh_hash);

				totalStockTake = 0.00;
			}
			
			/*--------------------------------------------------
			| Set to zero regardless of current closing stock. |
			--------------------------------------------------*/
			if (envVarStockTakeUpdate == ZERO_ALL)
			{
				if (MULT_LOC || SK_BATCH_CONT)
					ClearLocations (incc_rec.hhwh_hash);

				totalStockTake = 0.00;
			}
		}
		/*--------------------------------------
		| Process and Update Location records. |
		--------------------------------------*/
		ProcessOtherStuff ();
			
		abc_unlock (incc);
		cc = find_rec (incc, &incc_rec, NEXT, "u");
	}
	abc_unlock (incc);

	CloseAudit ();
	DeleteInsc ();

}
/*======================================================================
| Process transaction file and process into existing location records. |
======================================================================*/
int
ProcLotLocation (
	long	hhwhHash)
{
	int		first_time 	= TRUE;
	int		tran_found 	= FALSE;
	float	StdCnv 		= 1.00;
	float	CnvFct 		= 1.00;

	totalStockTake = 0.00;

	/*------------------------------------------------ 
	| Process new transaction (s) to update to inlo. |
	------------------------------------------------*/
	sttf_rec.hhwh_hash = hhwhHash;
	sttf_rec.hhum_hash = 0L;
	strcpy (sttf_rec.location, "          ");
	strcpy (sttf_rec.lot_no, "       ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwhHash)
	{
		tran_found = TRUE;

		totalStockTake += NDEC (sttf_rec.qty);

		/*-----------------------------------------
		| Update stock take unit of measure file. |
		-----------------------------------------*/
		Update_inwu 
		(
			sttf_rec.hhwh_hash,
			sttf_rec.hhum_hash, 
			NDEC (sttf_rec.qty)
		);

		if (!MULT_LOC && !SK_BATCH_CONT)
		{
			abc_delete (sttf);
			sttf_rec.hhwh_hash = hhwhHash;
			sttf_rec.hhum_hash = inmr_rec.std_uom;
			strcpy (sttf_rec.location, "          ");
			strcpy (sttf_rec.lot_no	 , "       ");
			cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
			continue;
		}
		if (first_time)
		{
			/*----------------------------- 
			| Clear out old inlo records. |
			-----------------------------*/
			ClearLocations (hhwhHash);
			first_time = FALSE;
		}
		if (MULT_LOC || SK_BATCH_CONT)
		{
			char	locationType [2];

			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			StdCnv	=	 (cc) ? 1.00 : inum_rec.cnv_fct;
			if (StdCnv == 0.00)
				StdCnv = 1.00;

			inum_rec.hhum_hash	=	sttf_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "inum", "DBFIND");

			CnvFct	=	inum_rec.cnv_fct / StdCnv;

			cc = 	CheckLocation
					(
						ccmr_rec.hhcc_hash,
						sttf_rec.location,
						locationType
					);
			strcpy (err_str, DateToString (sttf_rec.lot_expiry));
			InLotLocation 
			(
				hhwhHash,
				ccmr_rec.hhcc_hash,
				inum_rec.hhum_hash,
				inum_rec.uom,
				sttf_rec.lot_no,
				sttf_rec.slot_no,
				sttf_rec.location,
				locationType,
				err_str,
				NDEC (sttf_rec.qty),
				CnvFct,
				"A",
				0.00,
				0.00,
				0.00,
				0.00,
				0L
			);

		}
		abc_delete (sttf);
		sttf_rec.hhwh_hash = hhwhHash;
		strcpy (sttf_rec.location, "          ");
		cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	}
	return (!tran_found);
}
/*==========================================================================
| Clear out existing Location records for current product before updating. |
==========================================================================*/
void
ClearLocations (
	long	hhwhHash)
{
	inlo_rec.hhwh_hash = hhwhHash;
	inlo_rec.hhum_hash = 0L;
	strcpy (inlo_rec.location, "          ");
	strcpy (inlo_rec.lot_no, "       ");
	cc = find_rec ("inlo", &inlo_rec, GTEQ, "u");
	while (!cc && inlo_rec.hhwh_hash == hhwhHash)
	{
		inlo_rec.qty	=	0.00;
		inlo_rec.stake	=	0.00;

		cc = abc_update ("inlo", &inlo_rec);
		if (cc)
			file_err (cc, "inlo", "DBUPDATE");

		cc = find_rec ("inlo", &inlo_rec, NEXT, "u");
	}
	abc_unlock ("inlo");
	return;
}

/*==================
| Total locations. | 
==================*/
float	
TotalLocations (
 long	hhwhHash)
{
	float	lo_qty 	= 0.00;

	abc_selfield ("inlo", "inlo_hhwh_hash");

	inlo_rec.hhwh_hash 	= 	hhwhHash;
	cc = find_rec ("inlo",&inlo_rec,GTEQ,"r");
	while (!cc && inlo_rec.hhwh_hash == hhwhHash)
	{
		lo_qty += inlo_rec.qty;
		cc = find_rec ("inlo", &inlo_rec, NEXT,"r");
	}
	abc_selfield ("inlo", "inlo_mst_id");
	return (lo_qty);
}
/*====================================
| Main Processing Routine for Stock. |
====================================*/
void
ProcessOtherStuff (void)
{
	float	difference	=	0.00,
			QtyError	=	0.00,
			CheckQty	=	0.00;
	double	workValue	=	0.00;

	/*---------------------------------
	| Update figures for inmr & incc. |
	---------------------------------*/
	inmr_rec.on_hand 	+= NDEC (totalStockTake - incc_rec.stake);
	incc_rec.adj  		+= NDEC (totalStockTake - incc_rec.stake);
	incc_rec.ytd_adj 	+= NDEC (totalStockTake - incc_rec.stake);
	incc_rec.closing_stock = (NDEC (incc_rec.opening_stock) + 
				 			  NDEC (incc_rec.pur) + 
				 			  NDEC (incc_rec.receipts) - 
				 			  NDEC (incc_rec.issues) - 
				 			  NDEC (incc_rec.sales) + 
				 			  NDEC (incc_rec.adj));

	oldQuantity  = NDEC (incc_rec.stake);
	newQuantity  = totalStockTake;
	difference	 = totalStockTake - incc_rec.stake;
	if (FloatZero (difference))
		difference = 0.00;

	switch (inmr_rec.costing_flag [0])
	{
	case 	'A':
	case 	'L':
	case 	'T':
	case 	'P':
		workValue	=	FindIneiCosts
						(
							inmr_rec.costing_flag,
							comm_rec.est_no,
							inmr_rec.hhbr_hash
						);

		oldValue = workValue * (double) oldQuantity;
		newValue = workValue * (double) newQuantity;
		break;

	case	'F':
		ProcessFifo 
		(
			TRUE, 
			incc_rec.stake + difference
		);

		oldValue = 	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_rec.stake,
						FALSE, 
						TRUE,
						inmr_rec.dec_pt
					);

		newValue = 	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
				    	incc_rec.stake + difference, 
				    	FALSE, 
						TRUE,
						inmr_rec.dec_pt
					);
		break;

	case	'I':
		ProcessFifo 
		 (
			FALSE, 
			incc_rec.stake + difference
		);

		oldValue = 	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_rec.stake,
						FALSE, 
						FALSE,
						inmr_rec.dec_pt
					);

		newValue = 	FindIncfValue 
					(
						incc_rec.hhwh_hash, 
				    	incc_rec.stake + difference, 
				    	FALSE, 
						FALSE,
						inmr_rec.dec_pt
					);
		break;

	default:	
		break;
	}

	/*------------------------------------------------
	| Cost of sales is interfaced to general ledger. |
	------------------------------------------------*/
	if (envVarCostSales)
		GetAccounts (inmr_rec.category);

	AddIntr 
	(
		difference, 
		newValue - oldValue
	);

	AddGlwk 
	(
		difference, 
		newValue - oldValue
	);

	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, inmr, "DBUPDATE");
	
	abc_unlock (inmr);

	incc_rec.stake = 0.0;
	strcpy (incc_rec.stat_flag, "0");

	cc = abc_update ("incc", &incc_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");
	
	CheckQty	=	TotalLocations (incc_rec.hhwh_hash);

	if (incc_rec.closing_stock != CheckQty)
	{
		QtyError	=	incc_rec.closing_stock - CheckQty;
		cc = FindLocation 
			 (
				incc_rec.hhwh_hash,
				inmr_rec.std_uom,
				NULL,
				ValidLocations,
				&inlo2_rec.inlo_hash
			);
		if (!cc)
		{
			cc = find_rec (inlo2, &inlo2_rec, EQUAL, "r");
			if (cc)
				file_err (cc, inlo2, "DBFIND");

			strcpy (err_str, DateToString (inlo_rec.expiry_date));
			InLotLocation 
			(
				inlo_rec.hhwh_hash,
				incc_rec.hhcc_hash,
				inlo_rec.hhum_hash,
				inlo_rec.uom,	
				inlo_rec.lot_no,
				inlo_rec.slot_no,
				inlo_rec.location,
				inlo_rec.loc_type,
				err_str,
				QtyError,
				1.00,
				inlo_rec.loc_status,
				inlo_rec.pack_qty,
				inlo_rec.chg_wgt,
				inlo_rec.gross_wgt,
				inlo_rec.cu_metre,
				inlo_rec.sknd_hash
			);
		}
		else
		{
			strcpy (err_str, DateToString (TodaysDate ()));
			InLotLocation 
			(
				incc_rec.hhwh_hash,
				incc_rec.hhcc_hash,
				inmr_rec.std_uom,
				inmr_rec.sale_unit,
				 (inmr_rec.lot_ctrl [0] != 'Y') ? local_rec.batchString : local_rec.invalidString,
				 (inmr_rec.lot_ctrl [0] != 'Y') ? local_rec.batchString : local_rec.invalidString,
				llctDefault,
				"L",
				err_str,
				QtyError,
				1.00,
				"A",
				0.00,
				0.00,
				0.00,
				0.00,
				0L
			);
		}
	}
	/*------------------------------------------------------
	| Process inventory warehouse stock take transactions. |
	------------------------------------------------------*/
	Proc_inwu (incc_rec.hhwh_hash);
}

/*====================================================
| Updated stock take figures for Inventory UOM file. |
====================================================*/
void
Update_inwu (
	long	hhwhHash,
	long	hhumHash,
	float	Qty)
{
	inwu_rec.hhwh_hash	=	hhwhHash;
	inwu_rec.hhum_hash	=	hhumHash;
	cc = find_rec (inwu, &inwu_rec, COMPARISON, "r");
	if (cc)
	{
		memset (&inwu_rec, 0, sizeof (inwu_rec));
		inwu_rec.hhwh_hash	=	hhwhHash;
		inwu_rec.hhum_hash	=	hhumHash;
		inwu_rec.stake_calc	=	Qty;
		cc = abc_add (inwu, &inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBADD");

		return;
	}
	inwu_rec.stake_calc	+=	Qty;

	cc = abc_update (inwu, &inwu_rec);
	if (cc)
		file_err (cc, inwu, "DBUPDATE");
}
/*====================================================
| Updated stock take figures for Inventory UOM file. |
====================================================*/
void
Proc_inwu (
 long	hhwhHash)
{
	float	Diff_inwu	=	0.00;

	inwu_rec.hhwh_hash	=	hhwhHash;
	inwu_rec.hhum_hash	=	0;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "u");
	while (!cc && inwu_rec.hhwh_hash == hhwhHash)
	{
		Diff_inwu	= inwu_rec.stake_calc - inwu_rec.stake;
		inwu_rec.adj += Diff_inwu;

		inwu_rec.closing_stock = inwu_rec.opening_stock	+
								 inwu_rec.pur			+
								 inwu_rec.receipts		-
								 inwu_rec.issues		-
								 inwu_rec.sales			+
								 inwu_rec.adj;

		inwu_rec.stake		=	0.00;
		inwu_rec.stake_calc	=	0.00;

		cc = abc_update (inwu, &inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBADD");

		cc = find_rec (inwu, &inwu_rec, NEXT, "u");
	}
	abc_unlock (inwu);
	return;
}

/*============================
| Add Required FIFO records. |
============================*/
void
ProcessFifo (
	int		fifoFlag,
	float	fifoQty)
{
	float	qty 		= 0.00;
	double	workCost	= 0.00;

	cc = FindIncf (incc_rec.hhwh_hash, fifoFlag, "r");
	while (!cc && incc_rec.hhwh_hash == incfRec.hhwh_hash)
	{
		qty += NDEC (incfRec.fifo_qty);
		cc = FindIncf (0L, fifoFlag, "r");
	}
	if (fifoQty > qty)
	{
		workCost	=	FindIneiCosts
						(
							"L",
							comm_rec.est_no,
							inmr_rec.hhbr_hash
						);

		cc	=	AddIncf
				(
					incc_rec.hhwh_hash,
					postDate,
					workCost,
					workCost,
					fifoQty - qty,
					" ",
					workCost,
					0.00,
					0.00,
					0.00,
					0.00,
					workCost,
					"E"
				);
		if (cc)
			file_err (cc, incf, "DBADD");
	}
}

/*=======================================================
|	Routine to open output pipe to standard print to	|
|	provide an audit trail of events.					|
|	This also sends the output straight to the spooler.	|
=======================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".13\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".ESTOCK TAKE UPDATE AUDIT REPORT\n");
	fprintf (fout, ".ESELECTION [%s] STARTED : %s : %s (%s)\n", 
			inscRec.stake_code, 
			DateToString (inscRec.start_date), 
			inscRec.start_time, 
			clip (inscRec.description));
	
	if (envVarStockTakeUpdate == LEAVE)
		fprintf (fout, ".ENOTE : Stock not counted within selection left as is.\n");
	if (envVarStockTakeUpdate == ZERO_COUNT)
		fprintf (fout, ".ENOTE : Stock not counted within selection Set as if Zero Counted.\n");
	if (envVarStockTakeUpdate == ZERO_ALL)
		fprintf (fout, ".ENOTE : Stock not counted within selection will be set to Zero stock on hand.\n");

	fprintf (fout, ".E%s AS AT %s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EBr %s : W/H %s \n", clip (comm_rec.est_name), 
				           clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");

	fprintf (fout, ".EINVENTORY POST DATE : %s\n", DateToString (postDate));
	fprintf (fout, ".R=================================");
	fprintf (fout, "===========================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "===========================");
	if (envVarCostSales)
		fprintf (fout, "===================================\n");
	else
		fprintf (fout, "=\n");

	fprintf (fout, "=================================");
	fprintf (fout, "===========================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "===========================");
	if (envVarCostSales)
		fprintf (fout, "===================================\n");
	else
		fprintf (fout, "=\n");

	fprintf (fout, "|   GROUP    |  ITEM  NUMBER  ");
	fprintf (fout, "|    ITEM   DESCRIPTION    ");
	fprintf (fout, "|TYPE");
	fprintf (fout, "|CLOSING STOCK ");
	fprintf (fout, "|  STOCK TAKE  ");
	fprintf (fout, "|   QTY DIFF   |VALUE VARIANCE");
	if (envVarCostSales)
		fprintf (fout, "|    DEBIT       |      CREDIT    |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "|            |                ");
	fprintf (fout, "|                          ");
	fprintf (fout, "|COST");
	fprintf (fout, "|              ");
	fprintf (fout, "|              ");
	fprintf (fout, "|              |              ");
	if (envVarCostSales)
		fprintf (fout, "|                |                |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "|------------|----------------");
	fprintf (fout, "|--------------------------");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------|--------------");
	if (envVarCostSales)
		fprintf (fout, "|----------------|----------------|\n");
	else
		fprintf (fout, "|\n");
}

/*===================================================
|	Routine to close the audit trail output file.	|
===================================================*/
void
CloseAudit (
 void)
{
	fprintf (fout, "|============|================");
	fprintf (fout, "|==========================");
	fprintf (fout, "|====");
	fprintf (fout, "|==============");
	fprintf (fout, "|==============");
	fprintf (fout, "|==============|==============");
	if (envVarCostSales)
		fprintf (fout, "|================|================|\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "| TOTAL      |                ");
	fprintf (fout, "|                          ");
	fprintf (fout, "|    ");
	sprintf (err_str, local_rec.rep_qty, oldStockValue);
	fprintf (fout, "|%14s", err_str);
	sprintf (err_str, local_rec.rep_qty, newStockValue);
	fprintf (fout, "|%14s", err_str);
	sprintf (err_str, local_rec.rep_qty, totalQuantity);
	fprintf (fout, "|%14s|%13.2f ", err_str, totalValue);
	if (envVarCostSales)
		fprintf (fout, "|                |                |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*================================
| Add transactions to glwk file. |
================================*/
void
AddGlwk (
	float	updateQuantity,
	double	updateValue)
{
	int		monthNum, reverse = TRUE;

	double	wk_value = 0.00;

	if (MoneyZero (updateValue))
		return;

 	wk_value = out_cost (updateValue, inmr_rec.outer_size);
	reverse = (wk_value < 0.00) ?  TRUE : FALSE;

	if (envVarCostSales)
	{
		strcpy (glwkRec.co_no, comm_rec.co_no);
		strcpy (glwkRec.tran_type, "14");
		glwkRec.post_date = postDate;
		glwkRec.tran_date = postDate;
		DateToDMY (postDate, NULL, &monthNum, NULL);

		sprintf (glwkRec.period_no, "%02d", monthNum);
		sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
		sprintf (glwkRec.user_ref, "%8.8s", "STAKE.");
		strcpy (glwkRec.stat_flag, "2");
		sprintf (glwkRec.narrative, "Stock Take : %s / %s", 
					comm_rec.est_no, comm_rec.cc_no);
		sprintf (glwkRec.alt_desc1, " ");
		sprintf (glwkRec.alt_desc2, " ");
		sprintf (glwkRec.alt_desc3, " ");
		sprintf (glwkRec.batch_no, " ");
		if (reverse)
			glwkRec.amount = CENTS (wk_value * -1);
		else
			glwkRec.amount = CENTS (wk_value);

		sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,glDebitAcct);
		glwkRec.hhgl_hash = glDebitHash;

		strcpy (glwkRec.jnl_type, (reverse) ? "1" : "2");
		glwkRec.loc_amount 	= glwkRec.amount;
		glwkRec.exch_rate 	= 1.00;
		strcpy (glwkRec.currency, local_rec.loc_curr);
		GL_AddBatch ();

		sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,glCreditAcct);
		glwkRec.hhgl_hash 	= glCreditHash;
		strcpy (glwkRec.jnl_type, (reverse) ? "2" : "1");
		glwkRec.loc_amount 	= glwkRec.amount;
		glwkRec.exch_rate 	= 1.00;
		strcpy (glwkRec.currency, local_rec.loc_curr);
		GL_AddBatch ();
	}
	fprintf (fout, "|%s%s", inmr_rec.inmr_class, inmr_rec.category);
   	fprintf (fout, "|%s", inmr_rec.item_no);
	fprintf (fout, "|%-26.26s", inmr_rec.description);

	if (LCST)
		fprintf (fout, "|LAST");

	if (AVGE)
		fprintf (fout, "|AVGE");

	if (FIFO)
		fprintf (fout, "|FIFO");

	if (LIFO)
		fprintf (fout, "|LIFO");
		
	sprintf (err_str, local_rec.rep_qty, NDEC (incc_rec.stake));
	fprintf (fout, "|%14s", err_str);
	sprintf (err_str, local_rec.rep_qty, totalStockTake);
	fprintf (fout, "|%14s", err_str);
	sprintf (err_str, local_rec.rep_qty, updateQuantity);
	fprintf (fout, "|%14s", err_str);
	fprintf (fout, "|%13.2f ", wk_value);
	if (envVarCostSales)
	{
		fprintf (fout, "|%-16.16s",    (reverse) ? glDebitAcct : glCreditAcct);
		fprintf (fout, "|%-16.16s|\n", (reverse) ? glCreditAcct : glDebitAcct);
	}
	else
		fprintf (fout, "|\n");

	oldStockValue += NDEC (incc_rec.stake);
	newStockValue += totalStockTake;
	totalQuantity += updateQuantity;
	totalValue += wk_value;
	return;
}

/*=======================
| Get Control Accounts. |
=======================*/
void
GetAccounts (
 char	*cat_no)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		" ",
		cat_no
	);
	strcpy (glCreditAcct, glmrRec.acc_no);
	glCreditHash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"STAKE VAR.",
		" ",
		cat_no
	);
	glDebitHash = glmrRec.hhmr_hash;
	strcpy (glDebitAcct, glmrRec.acc_no);
	return;
}

/*================================
| Creeate Inventory Transaction. |
================================*/
void
AddIntr (
	float	updateQuantity,
	double	updateValue)
{
	double	wk_value = 0.00;

	if (updateQuantity == 0.00)
		return;

 	wk_value = out_cost (updateValue, inmr_rec.outer_size);

	strcpy (intr_rec.co_no, comm_rec.co_no);
	strcpy (intr_rec.br_no, comm_rec.est_no);
	intr_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
	intr_rec.hhcc_hash 	= incc_rec.hhcc_hash;
	intr_rec.hhum_hash 	= inmr_rec.std_uom;
	intr_rec.type 		= 4;
	intr_rec.date 		= postDate;
	intr_rec.qty 		= updateQuantity;
	intr_rec.cost_price = CENTS (wk_value / (double) updateQuantity);
	intr_rec.sale_price = 0.00;
	strcpy (intr_rec.batch_no, "STAKE");
	sprintf (intr_rec.ref1, "ST SEL [%s]", inscRec.stake_code);
	strcpy (intr_rec.stat_flag	, "0");

	cc = abc_add (intr, &intr_rec);
	if (cc)
		file_err (cc, intr, "DBADD");
}
/*=================================
| Delete Stock Take Control File. |
=================================*/
void
DeleteInsc (
	void)
{
	inscRec.hhcc_hash = ccmr_rec.hhcc_hash;
	cc = find_rec (insc, &inscRec, COMPARISON, "r");
	if (cc)
		file_err (cc, insc, "DBFIND");

	abc_delete (insc);
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

	rv_pr (ML (mlSkMess043), 31, 0, 1);
	line_at (1,0,80);

	box (0, 3, 80, 7);
	line_at (8,1,79);
	line_at (19,0,80);

	print_at (20,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (22,0,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);
	line_cnt = 0;
	scn_write (scn);
	
    return (EXIT_SUCCESS);
}

/*
 *	Minor support functions
 */
static int
MoneyZero (
	double	m)
{
	return (fabs (m) < 0.0001);
}

static int
FloatZero (
	float	m)
{
	return (fabs (m) < 0.0001);
}
