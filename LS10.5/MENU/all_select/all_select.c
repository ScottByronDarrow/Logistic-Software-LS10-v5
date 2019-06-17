/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: all_select.c,v 5.3 2001/08/09 05:13:14 scott Exp $
|  Program Name  : (all_select.c  )                                   |
|  Program Desc  : (Company / Branch Select.                    )     |
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: all_select.c,v $
| Revision 5.3  2001/08/09 05:13:14  scott
| Updated to use FinishProgram ();
|
| Revision 5.2  2001/08/06 23:32:08  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:13  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: all_select.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/all_select/all_select.c,v 5.3 2001/08/09 05:13:14 scott Exp $";

#define	BY_COMPANY	(byWhat[0] == 'C')
#define	BY_BRANCH	(byWhat[0] == 'B')
#define	BY_WHOUSE	(byWhat[0] == 'W')
#define	BY_SELECT	(byWhat[0] == 'S')
#define	MASTER		(!strcmp (mstBranch, esmr_rec.est_no))

#define	CO_DBT		(coClose[0] == '1')
#define	CO_CRD		(coClose[1] == '1')
#define	CO_INV		(coClose[2] == '1')
#define	CO_GEN		(coClose[4] == '1')
#define CO_PAY		(coClose[3] == '1')

#define BR_DBT		!CO_DBT
#define BR_CRD		!CO_CRD
#define BR_INV		!CO_INV
#define BR_GEN		!CO_GEN
#define BR_PAY		!CO_PAY

#define DBT_DATE	local_rec.all_date[0]
#define CRD_DATE	local_rec.all_date[1]
#define INV_DATE	local_rec.all_date[2]
#define GEN_DATE	local_rec.all_date[3]
#define PAY_DATE	local_rec.all_date[4]

#define SOME_COMP	(CO_DBT || CO_CRD || CO_INV || CO_GEN)
#define SOME_BRANCH	(BR_DBT || BR_CRD || BR_INV || BR_GEN)

#include 	<pslscr.h>
#include	<license2.h>
#include	<ml_menu_mess.h>
#include	<ml_std_mess.h>
#include	<dberr.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

	int		CHG_OK 	= FALSE;
	int		IN_MODE = 0;
	char	coClose[6];
	char	mstBranch[3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct comrRecord	comr_spare;
struct esmrRecord	esmr_rec;
struct esmrRecord	esmr_spare;
struct ccmrRecord	ccmr_rec;

	char	*data = "data";

	long	tloc = -1L;
	struct	tm	*ts;

	int		GetComr 			(void);
	int		ReadMisc 			(void);
	int		GetEsmr 			(void);
	int		CheckInput 			(void);
	int		spec_valid 			(int);
	int		heading 			(int);
	int		ChangeCompanyDates 	(void);
	int		ChangeBranchDates 	(void);
	int		OpenComm 			(void);
	int		DateChange 			(long);
	void	SrchComr 			(char *);
	void	SrchEsmr 			(char *);
	void	SrchCcmr 			(char *);
	void	OpenDB 				(void);
	void	CloseDB 			(void);
	void	DisplayCompanyDates (void);
	void	DisplayBranchDates 	(void);
	void	SetCompanyDates 	(void);
	void	SetBranchDates 		(void);
	void	ClearCompany 		(void);
	void	ClearBranch 		(void);
	void	ClearWarehouse 		(void);
	void	ChangeDates 		(void);
	long	GetNewDate 			(long);
	void	ClearDate 			(char *);
	void	ClearBranchDates 	(void);
	void	UnlockComr 			(void);
	void	UnlockEsmr 			(void);
	void	Update 				(void);
	void	ReadComm 			(void);
	void	Usage 				(char *);
	void	DrawOther 			(void);
	void	SetCommData 		(void);
	void	WriteComm 			(int);

	char	byWhat[2];
	char	currUser[11];
	char	*DspUser (int);

	int		changeOk = 0;
	int		addComm;

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	okay[2];
	char	dummy[11];
	char	update[1];
	char	dflt_co[3];
	char	dflt_br[3];
	char	dflt_wh[3];
	int	control;
	long	all_date[5];
} local_rec;

static	struct	var	vars[]	=	
{
	{1, LIN, "companyNumber", 7, 2, CHARTYPE, 
		"AA", "          ", 
		" ", local_rec.dflt_co, "Company Number      ", "<return> for Current Company. ", 
		NA, NO, JUSTRIGHT, "1", "99", comr_rec.co_no}, 
	{1, LIN, "companyName", 7, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", comr_rec.co_name}, 
	{1, LIN, "branchNumber", 8, 2, CHARTYPE, 
		"AA", "          ", 
		" ", local_rec.dflt_br, "Branch Number       ", "<return> for Current Branch. ", 
		NA, NO, JUSTRIGHT, "1", "99", esmr_rec.est_no}, 
	{1, LIN, "branchName", 8, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", esmr_rec.est_name}, 
	{1, LIN, "warehouseNumber", 9, 2, CHARTYPE, 
		"NN", "          ", 
		" ", local_rec.dflt_wh, "Warehouse Number    ", "<Return> For Current Warehouse", 
		NA, NO, JUSTRIGHT, "1", "99", ccmr_rec.cc_no}, 
	{1, LIN, "warehouseName", 9, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "          ", " ", ccmr_rec.name}, 
	{1, LIN, "customerDate", 11, 2, EDATETYPE, 
		"DD/DD/DDDD", "          ", 
		" ", "", "Customers Date      ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &DBT_DATE}, 
	{1, LIN, "supplierDate", 13, 2, EDATETYPE, 
		"DD/DD/DDDD", "          ", 
		" ", "", "Suppliers Date      ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &CRD_DATE}, 
	{1, LIN, "inventoryDate", 15, 2, EDATETYPE, 
		"DD/DD/DDDD", "          ", 
		" ", "", "Inventory Date      ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &INV_DATE}, 
	{1, LIN, "generalLedgerDate", 17, 2, EDATETYPE, 
		"DD/DD/DDDD", "          ", 
		" ", "", "G/Ledger Date       ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &GEN_DATE}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

int	lockComr = FALSE,
	lockEsmr = FALSE;

extern	int		TruePosition;

#include <write_comm.h>

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv[])
{
	char	*sptr;

	if (argc < 2)
		Usage (argv[0]);

	/*
	 *	Check to see whether all_select has been invoked.
	 */
	if (getenv ("ALL_SELECT"))
		return (EXIT_FAILURE);
	putenv ("ALL_SELECT=");

	sprintf (byWhat, "%-1.1s", argv[1]);
	if (!BY_COMPANY && !BY_BRANCH && !BY_WHOUSE && !BY_SELECT)
		Usage (argv[0]);

	if (argc == 3)
		changeOk = TRUE;

	if (!(sptr = chk_env ("CO_CLOSE")))
		sprintf (coClose, "%-5.5s", "11111");
	else
		sprintf(coClose, "%-5.5s", sptr);

	if (!(sptr = chk_env ("CO_MST_BR")))
		strcpy (mstBranch, " 1");
	else
		sprintf (mstBranch, "%-2.2s", sptr);

	TruePosition	=	TRUE;
	SETUP_SCR (vars);
	init_scr ();
	set_tty (); 
	set_masks ();

	OpenDB ();

	ReadComm ();

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec ("comr", &comr_rec, COMPARISON, "r");
	if (cc)
	{
		CloseDB (); 
		FinishProgram ();;
		return (EXIT_FAILURE);
	}

	if (strcmp (comr_rec.master_br, "  "))
		sprintf (mstBranch, comr_rec.master_br);

	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, FALSE);

	if (comm_rec.term == 1 || changeOk)
		CHG_OK 	= TRUE;

	IN_MODE = (CHG_OK) ? NI : NA;

	FLD ("companyNumber") = YES;
	FLD ("branchNumber") = YES;
	FLD ("warehouseNumber") = YES;

	if (BY_BRANCH || BY_WHOUSE || BY_SELECT)
	{
		FLD ("companyNumber") = NA;
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = GetComr ();
		if (cc) 
			return (EXIT_FAILURE);

		if (BY_WHOUSE || BY_SELECT)
		{
			FLD ("branchNumber") = NA;
			strcpy (esmr_rec.co_no, comm_rec.co_no);
			strcpy (esmr_rec.est_no, comm_rec.est_no);
			cc = GetEsmr ();
			if (cc)
				return (EXIT_FAILURE);

			if (BY_SELECT)
			{
				if (strcmp (comm_rec.cc_no, "  "))
				{
					CloseDB (); 
					FinishProgram ();
					return (EXIT_SUCCESS);
				}
			}
		}
	}

	if (!strcmp (comm_rec.co_no, "  "))
		strcpy (local_rec.dflt_co, " 1");
	else 
		strcpy (local_rec.dflt_co, comm_rec.co_no);

	if (!strcmp (comm_rec.co_no, "  "))
		strcpy (local_rec.dflt_br, " 1");
	else 
		strcpy (local_rec.dflt_br, comm_rec.co_no);

	if (!strcmp (comm_rec.cc_no, "  "))
		strcpy (local_rec.dflt_wh, " 1");
	else 
		strcpy (local_rec.dflt_wh, comm_rec.cc_no);

	prog_exit = FALSE;
	while (!prog_exit) 
	{
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;
		init_ok    = FALSE;

		heading (1);
		if (BY_BRANCH || BY_WHOUSE || BY_SELECT)
		{
			DisplayCompanyDates ();
			if (BY_WHOUSE || BY_SELECT)
				DisplayBranchDates ();
		}
		entry (1);
		if (restart || prog_exit)
		{
			UnlockComr ();
			UnlockEsmr ();
			continue;
		}

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			UnlockComr ();
			UnlockEsmr ();
			continue;
		}

		if (CheckInput ())
		{
			prog_exit = FALSE;
			UnlockComr ();
			UnlockEsmr ();
			continue;
		}

		if (!restart) 
		{
			Update ();
			break;
		}
		else
		{
			UnlockComr ();
			UnlockEsmr ();
		}
	}

	UnlockComr ();
	UnlockEsmr ();
	CloseDB (); 
	FinishProgram ();
	return (prog_exit ? EXIT_FAILURE : EXIT_SUCCESS);
}

/*=======================
| Open Data Base Files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (comm, comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*========================
| Close Data Base Files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (data);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_dbclose (comm);
}

/*=============================================
| Get common info from common database file . |
=============================================*/
int
ReadMisc (void)	
{
	local_rec.control = 0;

	if ((comm_rec.term = ttyslt ()) < 0)
		return (comm_rec.term);

	cc = find_rec ("comm", &comm_rec, COMPARISON, "u");
	if (cc) 
	{
		abc_unlock ("comm");
		addComm = TRUE;
		comm_rec.term = ttyslt ();
		strcpy (comm_rec.co_no, " 1");
		strcpy (comm_rec.co_no, " 1");
		strcpy (comm_rec.cc_no, " 1");
	}
	else
		addComm = FALSE;

	return (EXIT_SUCCESS);
}
	
int
CheckInput (void)
{
	char	which [12];

	if (!strcmp (comr_rec.co_no, "  "))
		strcpy (which,"Company");
	else if (!strcmp (esmr_rec.est_no, "  "))
		strcpy (which,"Branch");
	else if (!strcmp (ccmr_rec.cc_no, "  "))
		strcpy (which,"Warehouse");
	else
		return (EXIT_SUCCESS);

	sprintf (err_str, "%s was not selected.", which);
	print_mess (ML (err_str));
	sleep (sleepTime);
	return (EXIT_FAILURE);
}

int
spec_valid (
 int	field)
{
	/*--------------------------
	| Validate Company Number. |
	--------------------------*/
	if (LCHECK ("companyNumber")) 
	{ 
		UnlockComr ();
		UnlockEsmr ();

		if (FLD ("companyNumber") == NA)
		{
			DSP_FLD ("companyNumber");
			DSP_FLD ("companyName");
			return (EXIT_SUCCESS);
		}
		
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}

		ClearBranch ();
		ClearWarehouse ();

		if (!chk_secure (comr_rec.co_no, "  ") && !errno)
		{
			/*--------------------------------------------
			| User is not permitted access to Company %s |
			--------------------------------------------*/
			print_mess ( ML(mlStdMess139) );
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("companyNumber");
		cc = GetComr ();
		if (cc)
			return (EXIT_FAILURE);
		DisplayCompanyDates ();
		
		DSP_FLD ("companyNumber");
		DSP_FLD ("companyName");
		DSP_FLD ("branchNumber");
		DSP_FLD ("branchName");
		DSP_FLD ("warehouseNumber");
		DSP_FLD ("warehouseName");

		return (EXIT_SUCCESS);	
	}

	/*-------------------------
	| Validate Branch Number. |
	-------------------------*/
	if (LCHECK ("branchNumber")) 
	{ 
		UnlockEsmr ();

		if (BY_BRANCH && last_char == EOI)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (last_char == EOI && prog_status == ENTRY)
			end_input = FALSE;

		if (FLD ("branchNumber") == NA)
		{
			DSP_FLD ("branchNumber");
			DSP_FLD ("branchName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		ClearWarehouse ();

		if (!chk_secure (comr_rec.co_no, esmr_rec.est_no))
		{
			print_mess ( ML(mlStdMess139) );
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("branchNumber");
		strcpy (esmr_rec.co_no, comr_rec.co_no);
		cc = GetEsmr ();
		if (cc)
			return (EXIT_FAILURE);
		DisplayBranchDates ();
		
		FLD ("customerDate") 		= (CO_DBT && !MASTER) ? NA : IN_MODE;
		FLD ("supplierDate") 		= (CO_CRD && !MASTER) ? NA : IN_MODE;
		FLD ("inventoryDate") 		= (CO_INV && !MASTER) ? NA : IN_MODE;
		FLD ("generalLedgerDate")  	= (CO_GEN && !MASTER) ? NA : IN_MODE;

		DSP_FLD ("companyNumber");
		DSP_FLD ("companyName");
		DSP_FLD ("branchNumber");
		DSP_FLD ("branchName");
		DSP_FLD ("warehouseNumber");
		DSP_FLD ("warehouseName");

		return (EXIT_SUCCESS);	
	}

	/*----------------------------
	| Validate Warehouse Number. |
	----------------------------*/
	if (LCHECK ("warehouseNumber")) 
	{ 
		if (BY_WHOUSE && last_char == EOI)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (last_char == EOI && prog_status == ENTRY)
			end_input = FALSE;

		if (FLD ("warehouseNumber") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comr_rec.co_no);
		strcpy (ccmr_rec.est_no, esmr_rec.est_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess ( ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("companyNumber");
		DSP_FLD ("companyName");
		DSP_FLD ("branchNumber");
		DSP_FLD ("branchName");
		DSP_FLD ("warehouseNumber");
		DSP_FLD ("warehouseName");

		entry_exit = TRUE;

		return (EXIT_SUCCESS);	
	}

	/*------------------------
	| Validate Customer Date. |
	------------------------*/
	if (LCHECK ("customerDate")) 
	{ 
		if (dflt_used) 
		{
			DBT_DATE = (CO_DBT) ? GetNewDate (comr_rec.dbt_date)
							    : GetNewDate (esmr_rec.dbt_date);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate creditors Date. |
	--------------------------*/
	if (LCHECK ("supplierDate")) 
	{ 
		if (dflt_used) 
		{
			CRD_DATE = (CO_CRD) ? GetNewDate (comr_rec.crd_date)
								: GetNewDate (esmr_rec.crd_date);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Inentory Date. |
	-------------------------*/
	if (LCHECK ("inventoryDate")) 
	{ 
		if (dflt_used) 
		{
			INV_DATE = (CO_INV) ? GetNewDate (comr_rec.inv_date)
								: GetNewDate (esmr_rec.inv_date);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate General Ledger Date. |
	-------------------------------*/
	if (LCHECK ("generalLedgerDate")) 
	{ 
		if (dflt_used) 
		{
			GEN_DATE = (CO_GEN) ? GetNewDate (comr_rec.gl_date)
								: GetNewDate (esmr_rec.gl_date);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	clear ();

	SetCommData ();

	if (addComm)
	{
		cc = abc_add (comm, &comm_rec);
		if (cc) 
			file_err (cc, comm, "DBADD");
	}
	else
	{
		cc = abc_update (comm, &comm_rec);
		if (cc) 
			file_err (cc, comm, "DBUPDATE");
	}

	if (lockComr)
	{
		cc = abc_update ("comr", &comr_rec);
		if (cc) 
			file_err (cc, comr, "DBUPDATE");
		lockComr = FALSE;
	}
	if (lockEsmr)
	{
		cc = abc_update ("esmr", &esmr_rec);
		if (cc) 
			file_err (cc, esmr, "DBUPDATE");
		lockEsmr = FALSE;
 	}
 
	if (comm_rec.term == 1)
	{
		comm_rec.term = 0;
		cc = find_rec ("comm", &comm_rec, COMPARISON, "u");
		SetCommData ();
		if (cc)
			cc = abc_add (comm, &comm_rec);
		else
			cc = abc_update (comm, &comm_rec);
		if (cc)
			file_err (cc, comm, "DBADD / DBUPDATE");
	}

	WriteComm (ttyslt ());

	if (comm_rec.term == 1)
		WriteComm (0);

	return;
}

void
SetCommData (void)
{
	comr_rec.dbt_date 		= DBT_DATE;
	comr_rec.crd_date 		= CRD_DATE;
	comr_rec.inv_date 		= INV_DATE;
	comr_rec.gl_date  		= GEN_DATE;
	comr_rec.payroll_date 	= PAY_DATE;

	esmr_rec.dbt_date 		= DBT_DATE;
	esmr_rec.crd_date 		= CRD_DATE;
	esmr_rec.inv_date 		= INV_DATE;
	esmr_rec.gl_date  		= GEN_DATE;
	esmr_rec.pay_date 		= PAY_DATE;

	strcpy (comm_rec.co_no,    comr_rec.co_no);
	strcpy (comm_rec.co_name,  comr_rec.co_name);
	strcpy (comm_rec.co_short, comr_rec.co_short_name);

	if (strcmp (comr_rec.env_name, "                                                            "))
		strcpy (comm_rec.env_name, comr_rec.env_name);
	else
		sprintf (comm_rec.env_name, "%s/BIN/LOGISTIC", getenv ("PROG_PATH"));

	strcpy (comm_rec.est_no,	esmr_rec.est_no);
	strcpy (comm_rec.est_name, 	esmr_rec.est_name);
	strcpy (comm_rec.est_short, esmr_rec.short_name);
	strcpy (comm_rec.cc_no, 	ccmr_rec.cc_no);
	strcpy (comm_rec.cc_name,  	ccmr_rec.name);
	strcpy (comm_rec.cc_short, 	ccmr_rec.acronym);

	comm_rec.dbt_date 		= (CO_DBT) ? comr_rec.dbt_date : esmr_rec.dbt_date;
	comm_rec.crd_date 		= (CO_CRD) ? comr_rec.crd_date : esmr_rec.crd_date;
	comm_rec.inv_date 		= (CO_INV) ? comr_rec.inv_date : esmr_rec.inv_date;
	comm_rec.payroll_date 	= (CO_PAY) ? 0L : 0L;
	comm_rec.gl_date 		= (CO_GEN) ? comr_rec.gl_date : esmr_rec.gl_date;

	comm_rec.closed_period = comr_rec.closed_period;
	comm_rec.fiscal        = comr_rec.fiscal;
	comm_rec.gst_rate      = comr_rec.gst_rate;

	strcpy (comm_rec.price1_desc, comr_rec.price1_desc);
	strcpy (comm_rec.price2_desc, comr_rec.price2_desc);
	strcpy (comm_rec.price3_desc, comr_rec.price3_desc);
	strcpy (comm_rec.price4_desc, comr_rec.price4_desc);
	strcpy (comm_rec.price5_desc, comr_rec.price5_desc);
	strcpy (comm_rec.price6_desc, comr_rec.price6_desc);
	strcpy (comm_rec.price7_desc, comr_rec.price7_desc);
	strcpy (comm_rec.price8_desc, comr_rec.price8_desc);
	strcpy (comm_rec.price9_desc, comr_rec.price9_desc);
	comm_rec.pay_terms = comr_rec.pay_terms;
	strcpy (comm_rec.stat_flag, "0");
}

void
SrchComr (
 char    *key_val)
{
	_work_open (2,0,40);
	save_rec ("#Co", "#Company");

	sprintf (comr_rec.co_no, "%2.2s", key_val);
	cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc && !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
	{                        
		/*-----------------------------------------------
		| chk_secure() returns TRUE if access permitted	|
		| if user has access to some branches of the co	|
		| then errno is set.				|
		-----------------------------------------------*/
		if (chk_secure (comr_rec.co_no, "  ") || errno)
		{
			cc = save_rec (comr_rec.co_no, comr_rec.co_name);
			if (cc)
				break;
		}

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_no, "%2.2s", temp_str);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");
}

void
SrchEsmr (
 char    *key_val)
{
	_work_open (2,0,40);
	save_rec ("#Br", "#Branch");

	strcpy (esmr_rec.co_no, comr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (esmr_rec.co_no, comr_rec.co_no) && 
		   !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{                        
		if (chk_secure (comr_rec.co_no, esmr_rec.est_no))
		{
			cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
			if (cc)
				break;
		}

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON," r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
 char    *key_val)
{
	_work_open (2,0,40);
	save_rec ("#Wh", "#Warehouse");

	strcpy (ccmr_rec.co_no, comr_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 		
		   !strcmp (ccmr_rec.co_no, comr_rec.co_no) && 
		   !strcmp (ccmr_rec.est_no, esmr_rec.est_no) && 
		   !strncmp (ccmr_rec.cc_no, key_val, strlen (key_val)))
	{                        
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comr_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

int
heading (
 int	scn)
{
	int		i;
	char	wk_str[200];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		/*-----------------------------------
		| Get pointer to time & structure . |
		-----------------------------------*/
		tloc = time (&tloc);
		ts = localtime (&tloc);
	
		crsr_off ();
		box (0, 0, 79, 17);
		line_at (6, 1,78);
		line_at (10,1,78);
		line_at (12,1,78);
		line_at (14,1,78);
		line_at (16,1,78);
		DrawOther ();
		rv_pr (ML (" Company / Branch / Warehouse Select "), 22, 1, 1);

		if (CHG_OK)
			rv_pr (ML (" System administration Option. "), 25, 2, 1);

		sprintf (wk_str, "%s ( %s )", SystemTime (), DspUser (ts->tm_hour));

		i = strlen (clip (wk_str));
		box (((80 - i) / 2) - 2, 3, i + 4, 1);
		print_at (4, ((80 - i) / 2), wk_str);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
DrawOther (void)
{
	char	szDisplayText [512];

	/*-----------------------------------------------------------------
	| (CO_DBT) ? "COMPANY CONTROLLED DATE" : "BRANCH CONTROLLED DATE."|
	-----------------------------------------------------------------*/
	strcpy (szDisplayText, (CO_DBT) ? ML(mlMenuMess181) : ML(mlMenuMess180));
	print_at (11, 40, szDisplayText);

	/*-----------------------------------------------------------------
	| (CO_CRD) ? "COMPANY CONTROLLED DATE" : "BRANCH CONTROLLED DATE.|
	-----------------------------------------------------------------*/
	strcpy (szDisplayText, (CO_CRD) ? ML(mlMenuMess181) : ML(mlMenuMess180));
	print_at (13, 40, szDisplayText);

	/*-----------------------------------------------------------------
	| (CO_INV) ? "COMPANY CONTROLLED DATE" : "BRANCH CONTROLLED DATE.|
	-----------------------------------------------------------------*/
	strcpy (szDisplayText, (CO_INV)	? ML(mlMenuMess181) : ML(mlMenuMess180));
	print_at (15, 40, szDisplayText); 

	/*-----------------------------------------------------------------
	| (CO_GEN) ? "COMPANY CONTROLLED DATE" : "BRANCH CONTROLLED DATE.|
	-----------------------------------------------------------------*/
	strcpy (szDisplayText, (CO_GEN) ? ML(mlMenuMess181) : ML(mlMenuMess180));
	print_at (17, 40, szDisplayText); 
}

/*=======================================================================
|									|
=======================================================================*/
long	
GetNewDate (
 long	currentDate)
{
	long monthEnd = MonthEnd (currentDate);
	long sDate = TodaysDate ();

	if (monthEnd < sDate)
		return (monthEnd);

	if (currentDate < sDate)
		return (sDate);
	
	return (currentDate);
}

int
DateChange (
 long	date)
{
	return (GetNewDate (date) != date);
}

char	
*DspUser (
 int	hour)
{
	char	wellcomeString [41];

	sprintf (currUser, "%-10.10s", getenv ("LOGNAME"));

	if (hour < 12)
		strcpy (wellcomeString, ML ("Good Morning"));
	if (hour > 11 && hour < 17) 
		strcpy (wellcomeString, ML ("Good Afternoon"));
	if (hour > 16)
		strcpy (wellcomeString, ML ("Good Evening"));
	if (hour > 20)
		strcpy (wellcomeString, ML ("Good Night"));

	sprintf (err_str, "%s %c%s.", wellcomeString,
				 toupper (currUser[0]),
				 clip (currUser + 1));

	return (err_str);
}

/* gets comr record, locked if needed, returns result of find_rec */
int
GetComr (void)
{
	cc = find_rec (comr, &comr_rec, COMPARISON, (SOME_COMP) ? "w" : "r");
	if (cc) 
	{
		if (cc == -1)
			print_mess (ML ("Lock aborted by user"));
		else
			print_mess (ML ("Company is not on file"));
		sleep (sleepTime);
		clear_mess ();
		return (cc);
	}
	lockComr = ChangeCompanyDates ();
	if (!lockComr)
		abc_unlock (comr);
	SetCompanyDates ();
	return (EXIT_SUCCESS);
}

/* gets esmr record, locked if needed, returns result of find_rec */
int
GetEsmr (void)
{
	cc = find_rec (esmr, &esmr_rec, COMPARISON, (SOME_BRANCH) ? "w" : "r");
	if (cc) 
	{
		if (cc == -1)
			print_mess (ML ("Lock aborted by user"));
		else
			print_mess (ML ("Branch is not on file"));
		sleep (sleepTime);
		clear_mess ();
		return (cc);
	}
	lockEsmr = ChangeBranchDates ();
	if (!lockEsmr)
		abc_unlock (esmr);
	SetBranchDates ();
	return (EXIT_SUCCESS);
}

void
DisplayCompanyDates (void)
{
	if (CO_DBT) DSP_FLD ("customerDate");
	if (CO_CRD) DSP_FLD ("supplierDate");
	if (CO_INV) DSP_FLD ("inventoryDate");
	if (CO_GEN) DSP_FLD ("generalLedgerDate");
}

void
DisplayBranchDates (void)
{
	if (BR_DBT) DSP_FLD ("customerDate");
	if (BR_CRD) DSP_FLD ("supplierDate");
	if (BR_INV) DSP_FLD ("inventoryDate");
	if (BR_GEN) DSP_FLD ("generalLedgerDate");
}

/* return true if any of the company dates are going to change */
int
ChangeCompanyDates (void)
{
	if (SOME_COMP && CHG_OK) return TRUE;
	if (CO_DBT) if (DateChange (comr_rec.dbt_date)) return TRUE;
	if (CO_CRD) if (DateChange (comr_rec.crd_date)) return TRUE;
	if (CO_INV) if (DateChange (comr_rec.inv_date)) return TRUE;
	if (CO_GEN) if (DateChange (comr_rec.gl_date)) return TRUE;
	return FALSE;
}

/* return true if any of the branch dates are going to change */
int
ChangeBranchDates (void)
{
	if (SOME_BRANCH && CHG_OK) return TRUE;
	if (BR_DBT) if (DateChange (esmr_rec.dbt_date)) return TRUE;
	if (BR_CRD) if (DateChange (esmr_rec.crd_date)) return TRUE;
	if (BR_INV) if (DateChange (esmr_rec.inv_date)) return TRUE;
	if (BR_GEN) if (DateChange (esmr_rec.gl_date)) return TRUE;
	return FALSE;
}

/* set initial company dates */
void
SetCompanyDates (void)
{
	if (CO_DBT) DBT_DATE = GetNewDate (comr_rec.dbt_date);
	if (CO_CRD) CRD_DATE = GetNewDate (comr_rec.crd_date);
	if (CO_INV) INV_DATE = GetNewDate (comr_rec.inv_date);
	if (CO_GEN) GEN_DATE = GetNewDate (comr_rec.gl_date);
	if (CO_PAY) PAY_DATE = GetNewDate (comr_rec.payroll_date);
}

/* set initial branch dates */
void
SetBranchDates (void)
{
	if (BR_DBT) DBT_DATE = GetNewDate (esmr_rec.dbt_date);
	if (BR_CRD) CRD_DATE = GetNewDate (esmr_rec.crd_date);
	if (BR_INV) INV_DATE = GetNewDate (esmr_rec.inv_date);
	if (BR_GEN) GEN_DATE = GetNewDate (esmr_rec.gl_date);
	if (BR_PAY) PAY_DATE = GetNewDate (esmr_rec.pay_date);
}

void
ClearBranch (void)
{
	strcpy (esmr_rec.est_no, "  ");
	sprintf (esmr_rec.est_name, "%40.40s", " ");
	DSP_FLD ("branchNumber");
	DSP_FLD ("companyName");
	ClearBranchDates ();
	if (lockEsmr)
	{
		abc_unlock (esmr);
		lockEsmr = FALSE;
	}
}

void
ClearWarehouse (void)
{
	strcpy (ccmr_rec.cc_no, "  ");
	sprintf (ccmr_rec.name, "%40.40s", " ");
	DSP_FLD ("warehouseNumber");
	DSP_FLD ("warehouseName");
}

void
ClearDate (
 char *date)
{
	print_at (vars[label (date)].row, vars[label (date)].col + 20, "          ");
}

void
ClearBranchDates (void)
{
	if (BR_DBT) ClearDate ("customerDate");
	if (BR_CRD) ClearDate ("supplierDate");
	if (BR_INV) ClearDate ("inventoryDate");
	if (BR_GEN) ClearDate ("generalLedgerDate");
}
	
void
UnlockComr (void)
{
	if (lockComr)
	{
		abc_unlock (comr);
		lockComr = FALSE;
	}
}

void
UnlockEsmr (void)
{
	if (lockEsmr)
	{
		abc_unlock (esmr);
		lockEsmr = FALSE;
	}
}

void
ReadComm (void)
{
	int	err,
		newerr = 0;
	static char	*commErrors[] =
	{
		"UNKNOWN",
		"NO MAP",
		"BAD DEV",
		"NO ENTRY",
		"MAX TERMS"
	};

	err = ReadMisc ();
	if (err < 0)
	{
		err = -err;
		if (newerr > 4) newerr = 0;
		sprintf (err_str, "tty/comm error:  ttyslt () returned %d (%s).\n",
				  err, commErrors [newerr]);
		exit (EXIT_FAILURE);
	}
}

void
Usage (
 char	*programName)
{
	print_at (0,0, "Usage : %s [C|B|W|S] - optional <change>\007\n\r", programName);
	exit (EXIT_FAILURE);
}
