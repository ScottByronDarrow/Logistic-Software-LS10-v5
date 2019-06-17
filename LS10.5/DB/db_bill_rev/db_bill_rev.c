/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_bill_rev.c,v 5.1 2001/12/07 03:29:12 scott Exp $
|  Program Name  : (db_bill_rev.c) 
|  Program Desc  : (Bank Draft Reversal)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 29/11/92         |
|---------------------------------------------------------------------|
| $Log: db_bill_rev.c,v $
| Revision 5.1  2001/12/07 03:29:12  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.4  2001/08/20 23:10:24  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 08:22:30  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:41  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:01  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_bill_rev.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_bill_rev/db_bill_rev.c,v 5.1 2001/12/07 03:29:12 scott Exp $";

#include <pslscr.h>	
#include <GlUtils.h>	
#include <hot_keys.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>
#include <tabdisp.h>

#define	CF(x)	 (comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN"))

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct bkcrRecord	bkcr_rec;
struct bldtRecord	bldt_rec;
struct blhdRecord	blhd_rec;
struct crbkRecord	crbk_rec;
struct cudtRecord	cudt_rec;
struct cuhdRecord	cuhd_rec;
struct cuinRecord	cuin_rec;
struct cumrRecord	cumr_rec;

	char	*data  = "data",
	    	*bldt2 = "bldt2",
	    	*blhd2 = "blhd2";

	int		num_in_tab;
	int		glwk_no;
	int		DB_NETT = TRUE;
	int		MCURR = FALSE;

	long	tloc = -1L;

	double	old_rec_amt;
	double	new_rec_amt;
	double	exch_var;
	double	current_exch;
	double	org_bank_exch;
	double	bank_lcl_exch;

	char	get_buf [200];
 	char 	key_name [11];

	char	loc_curr [4];
	int		BillsDue = FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	curr_desc [41];
} local_rec;

static	int	tag_func (int c, KEY_TAB *psUnused);
static	int	abort_func (int c, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB ind_keys [] = 
{
    { NULL,		FN1, abort_func,
	"",								"A" },
    { " TAG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " ALL TAG/UNTAG ",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#else
static	KEY_TAB ind_keys [] = 
{
    { NULL,		FN1, abort_func,
	"",								"A" },
    { " [T]AG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " [^A]ALL TAG/UNTAG",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    END_KEYS
};
#endif

static	struct	var	vars [] =
{
	{1, LIN, "bank_id",	 3, 12, CHARTYPE,
		"UUUUU", "          ",
		" ", "", " Bank Code :", "Enter Bank Code. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "curr_desc",	 4, 12, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Currency  :", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		printerNo;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	SrchCrbk 			 (char *);
void 	ProcBills 			 (void);
void 	ProcFwdCheques 		 (void);
void 	WriteGlwk 			 (void);
int 	spec_valid 			 (int);
int 	heading 			 (int);
int 	Update 				 (void);
int 	GetExchangeRates 	 (void);
int 	UpdateCuhd 			 (void);
int 	UpdateGL 			 (void);
	
/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
 	char	*sptr;
	
	if (argc != 2)
	{
		print_at (0,0,mlStdMess036, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);

	sptr = get_env ("DB_NETT_USED");
	DB_NETT = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	MCURR = (sptr == (char *)0) ? FALSE : atoi (sptr);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*---------------------
	| Check program name. |
	---------------------*/
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	BillsDue = FALSE;
	if (!strcmp (sptr, "db_bill_rev"))
		BillsDue = TRUE;

	SETUP_SCR (vars);

	OpenDB ();

	set_tty ();
	init_scr ();		/*  sets terminal from termcap	*/
	set_masks ();		/*  setup print using masks	*/
	init_vars (1);		/*  set default values		*/

	if (!MCURR)
		FLD ("curr_desc") = ND;

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);

		if (BillsDue)
			ProcBills ();
		else
			ProcFwdCheques ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (bldt2, bldt);
	abc_alias (blhd2, blhd);

	open_rec (bldt,  bldt_list, BLDT_NO_FIELDS, "bldt_id_no2");
	open_rec (bldt2, bldt_list, BLDT_NO_FIELDS, "bldt_hhcp_hash");
	open_rec (blhd,  blhd_list, BLHD_NO_FIELDS, "blhd_id_no");
	open_rec (blhd2, blhd_list, BLHD_NO_FIELDS, "blhd_hhbl_hash");
	open_rec (bkcr,  bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
	open_rec (crbk,  crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenGlmr ();
	OpenPocr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (bldt);
	abc_fclose (bldt2);
	abc_fclose (blhd);
	abc_fclose (blhd2);
	abc_fclose (bkcr);
	abc_fclose (crbk);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cuin);
	abc_fclose (cumr);
	GL_CloseBatch (printerNo);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*------------------------
		| Find lodgement header. |
		------------------------*/
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		cc = find_rec (blhd, &blhd_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess078));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("bk_name");

		if (MCURR)
		{
			cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess040));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
	
			sprintf (local_rec.curr_desc, "%-40.40s", pocrRec.description);
			DSP_FLD ("curr_desc");
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchCrbk (
 char*              key_val)
{
	_work_open (5,0,40);
	save_rec ("#Bank ","#Bank Name ");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec ("crbk", &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
		      !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;

		cc = find_rec ("crbk", &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec ("crbk", &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, "crbk", "DBFIND");
}

/*------------------------------------
| Allow tagging of bills to reverse. |
------------------------------------*/
void
ProcBills (void)
{
	char	rec_date [11];
	char	amt_prmpt [15];
	double	lcl_amt;

	if (MCURR)
		sprintf (amt_prmpt, " AMOUNT (%-3.3s)  ", crbk_rec.curr_code);
	else
		strcpy (amt_prmpt, "    AMOUNT    ");

	num_in_tab = 0;
	tab_open ("BillsDue", ind_keys, 6, 3, 10, FALSE);
	tab_add ("BillsDue", 
		"# %-7.7s  %-12.12s  %-20.20s %-10.10s %-14.14s   ",
		"RECEIPT", 
		"RECEIPT DATE",
		"     REFERENCE", 
		"DUE DATE", 
		amt_prmpt);

	bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
	strcpy (bldt_rec.rec_type,  "B");
	strcpy (bldt_rec.posted_gl, "N");
	bldt_rec.due_date = 0L;
	cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
	while (!cc && 
	       bldt_rec.hhbl_hash == blhd_rec.hhbl_hash &&
			!strcmp (bldt_rec.rec_type,"B") &&
			 !strcmp (bldt_rec.posted_gl,"N"))
	{
		/*---------------
		| Find receipt. |
		---------------*/
		cc = find_hash (cuhd, &cuhd_rec, COMPARISON, "r", bldt_rec.hhcp_hash);	
		if (cc)
		{
			cc = find_rec (bldt, &bldt_rec, NEXT, "r");
			continue;
		}

		sprintf (rec_date, "%-10.10s", DateToString (cuhd_rec.date_payment));
		lcl_amt = cuhd_rec.loc_amt_paid + cuhd_rec.loc_disc_give;

		cc = tab_add ("BillsDue", 
		        "  %-8.8s %-10.10s   %-20.20s %-10.10s   %14.14s         %010ld",
			cuhd_rec.receipt_no,
			rec_date,
			cuhd_rec.narrative,
			DateToString (cuhd_rec.present_date),
			CF (bldt_rec.amount),
			cuhd_rec.hhcp_hash);
		
		if (cc)
			break;

		num_in_tab++;

		cc = find_rec (bldt, &bldt_rec, NEXT, "r");
	}

	if (num_in_tab > 0)
		tab_scan ("BillsDue");
	else
	{
		sprintf (err_str, "** %s **", ML ("NO VALID LINES CAN BE LOADED"));
		tab_add ("BillsDue", err_str);
		tab_display ("BillsDue", TRUE);
		sleep (sleepTime);
		tab_close ("BillsDue", TRUE);
		return;
	}

	if (!prog_exit)
		Update ();

	tab_close ("BillsDue", TRUE);
}       

/*------------------------------------
| Allow tagging of bills to reverse. |
------------------------------------*/
void
ProcFwdCheques (void)
{
	char	rec_date [11];
	char	amt_prmpt [15];
	double	lcl_amt;

	if (MCURR)
		sprintf (amt_prmpt, " AMOUNT (%-3.3s)  ", crbk_rec.curr_code);
	else
		strcpy (amt_prmpt, "    AMOUNT    ");

	num_in_tab = 0;
	tab_open ("BillsDue", ind_keys, 6, 3, 10, FALSE);
	tab_add ("BillsDue", 
		"# %-7.7s  %-12.12s  %-20.20s %-10.10s %-14.14s   ",
		"RECEIPT", 
		"RECEIPT DATE",
		"     REFERENCE", 
		"DUE DATE", 
		amt_prmpt);

	bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
	strcpy (bldt_rec.rec_type,  "1");
	strcpy (bldt_rec.posted_gl, "N");
	bldt_rec.due_date = 0L;
	cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
	while (!cc && bldt_rec.hhbl_hash == blhd_rec.hhbl_hash)
	{
		if (bldt_rec.posted_gl [0] != 'N' ||
			 bldt_rec.rec_type [0] == 'B' ||
			 bldt_rec.rec_type [0] == 'D' ||
			 bldt_rec.rec_type [0] == 'A')
		{
			cc = find_rec (bldt, &bldt_rec, NEXT, "r");
			continue;
		}
		/*---------------
		| Find receipt. |
		---------------*/
		cc = find_hash (cuhd, &cuhd_rec, COMPARISON, "r", bldt_rec.hhcp_hash);	
		if (cc)
		{
			cc = find_rec (bldt, &bldt_rec, NEXT, "r");
			continue;
		}
		sprintf (rec_date, "%-10.10s", DateToString (cuhd_rec.date_payment));
		lcl_amt = cuhd_rec.loc_amt_paid + cuhd_rec.loc_disc_give;

		cc = tab_add ("BillsDue", 
		        "  %-8.8s %-10.10s   %-20.20s %-10.10s   %14.14s         %010ld",
			cuhd_rec.receipt_no,
			rec_date,
			cuhd_rec.narrative,
			DateToString (cuhd_rec.present_date),
			CF (bldt_rec.amount), 
			cuhd_rec.hhcp_hash);
		
		if (cc)
			break;

		num_in_tab++;

		cc = find_rec (bldt, &bldt_rec, NEXT, "r");
	}

	if (num_in_tab > 0)
		tab_scan ("BillsDue");
	else
	{
		sprintf (err_str, "** %s **", ML ("NO VALID LINES CAN BE LOADED"));
		tab_add ("BillsDue", err_str);
		tab_display ("BillsDue", TRUE);
		sleep (sleepTime);
		tab_close ("BillsDue", TRUE);
		return;
	}

	if (!prog_exit)
		Update ();

	tab_close ("BillsDue", TRUE);
}       

/*------------------------------------
| Allow user to tag lines for update |
------------------------------------*/
static int
tag_func (
 int                c,
 KEY_TAB*           psUnused)
{
	if (c == 'T')
		tag_toggle ("BillsDue");
	else
		tag_all ("BillsDue");

	return (c);
}

static int
abort_func (
 int                c,
 KEY_TAB*           psUnused)
{
	prog_exit = TRUE;

	return (c);
}

int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		if (BillsDue)
			rv_pr (ML (mlDbMess073), 30, 0, 1);
		else
			rv_pr (ML (mlDbMess074), 30, 0, 1);

		move (0,1);
		line (80);

		if (scn == 1)
			box (0, 2, 80, (MCURR) ? 2 : 1);

		move (0, 21);
		line (80);
		sprintf (err_str,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,err_str);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*-----------------
| updates details |
-----------------*/
int
Update (void)
{
	int	i;

	/*--------------------------
	| Process all tagged lines |
	--------------------------*/
	for (i = 0; i < num_in_tab; i++)
	{
		tab_get ("BillsDue", get_buf, EQUAL, i);
	   	if (!tagged (get_buf))
			continue;

		/*-------------------
		| Find cuhd record. |
		-------------------*/
		cc = find_hash (cuhd, &cuhd_rec, COMPARISON, "u", atol (get_buf + 81));
		if (cc)
			file_err (cc, cuhd, "DBFIND");

		/*-------------------
		| Find cumr record. |
		-------------------*/
		cc = find_hash (cumr, &cumr_rec, COMPARISON, "r", cuhd_rec.hhcu_hash);
		if (cc)
			file_err (cc, cumr, "DBFIND");

		/*-------------------
		| Find bldt record. |
		-------------------*/
		cc = find_hash (bldt2, &bldt_rec, COMPARISON, "u", atol (get_buf + 81));
		if (cc)
			file_err (cc, bldt, "DBFIND");

		/*-----------------------------
		| Find Current Exchange Rate. |
		-----------------------------*/
		GetExchangeRates ();

		/*---------------------------
		| Calculate posting values. |
		---------------------------*/
		old_rec_amt = cuhd_rec.loc_amt_paid;
		new_rec_amt = cuhd_rec.tot_amt_paid;
		if (current_exch != 0.00)
			new_rec_amt /= current_exch;
		exch_var = no_dec (new_rec_amt) - no_dec (old_rec_amt);

		/*--------------
		| Update bldt. |
		--------------*/
		strcpy (bldt_rec.posted_gl, "Y");

		bldt_rec.amount = cuhd_rec.tot_amt_paid / org_bank_exch;
		bldt_rec.bk_lcl_exch = bank_lcl_exch;
		cc = abc_update (bldt2, &bldt_rec);
		if (cc)
			file_err (cc, bldt2, "DBUPDATE");

		/*-----------------------------
		| Update cuhd / cudt records. |
		-----------------------------*/
		UpdateCuhd ();

		/*-------------
		| Post to GL. |
		-------------*/
		UpdateGL ();
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------
| Find Current Exchange Rate. |
-----------------------------*/
int
GetExchangeRates (void)
{
	/*-------------------------------------
	| Find Origin to Local exchange rate. |
	-------------------------------------*/
	cc = FindPocr (comm_rec.co_no, cumr_rec.curr_code, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");

	current_exch = pocrRec.ex1_factor;

	/*------------------------------------
	| Find Origin to Bank exchange rate. |
	------------------------------------*/
	cc = find_hash (blhd2, &blhd_rec, COMPARISON,"r", bldt_rec.hhbl_hash);
	if (cc)
		file_err (cc, blhd, "DBFIND");

	strcpy (bkcr_rec.co_no, comm_rec.co_no);
	sprintf (bkcr_rec.bank_id, "%-5.5s", blhd_rec.bank_id);
	sprintf (bkcr_rec.curr_code, "%-3.3s", cumr_rec.curr_code);
	cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, bkcr, "DBFIND");

	org_bank_exch = bkcr_rec.ex1_factor;

	/*-----------------------------------
	| Find Bank to Local exchange rate. |
	-----------------------------------*/
	cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");

	bank_lcl_exch = pocrRec.ex1_factor;

	if (!MCURR)
	{
		current_exch  = 1.00;
		org_bank_exch = 1.00;
		bank_lcl_exch = 1.00;
	}

	return (TRUE);
}

/*-----------------------------
| Update cuhd / cudt records. |
-----------------------------*/
int
UpdateCuhd (void)
{
	double	tot_exch_var;
	double	lcl_amt;

	tot_exch_var = 0.00;

	cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;
	cudt_rec.hhci_hash = 0L;
	cc = find_rec (cudt, &cudt_rec, GTEQ, "u");
	while (!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash)
	{
		cc = find_hash (cuin, &cuin_rec, COMPARISON, "r", cudt_rec.hhci_hash);
		if (cc)
			file_err (cc, cuin, "DBFIND");

		if (cuin_rec.amt == 0.00)
		{
			abc_unlock (cudt);
			cc = find_rec (cudt, &cudt_rec, NEXT, "u");
			continue;
		}

		cudt_rec.exch_rate = current_exch;
		cudt_rec.loc_paid_inv = no_dec (cudt_rec.amt_paid_inv / current_exch);
		lcl_amt = no_dec (cudt_rec.amt_paid_inv / cuin_rec.exch_rate);
		cudt_rec.exch_variatio = cudt_rec.loc_paid_inv - lcl_amt;

		tot_exch_var += cudt_rec.exch_variatio;

		cc = abc_update (cudt, &cudt_rec);
		if (cc)
			file_err (cc, cudt, "DBUPDATE");

		cc = find_rec (cudt, &cudt_rec, NEXT, "u");
	}
	abc_unlock (cudt);

	cuhd_rec.bank_amt = bldt_rec.amount;
	cuhd_rec.bank_exch = org_bank_exch;
	cuhd_rec.loc_amt_paid = new_rec_amt;
	cuhd_rec.loc_disc_give = cuhd_rec.disc_given / current_exch;
	cuhd_rec.exch_variance = tot_exch_var;
	
	cc = abc_update (cuhd, &cuhd_rec);
	if (cc)
		file_err (cc, cuhd, "DBFIND");

	return (EXIT_SUCCESS);
}

/*-------------
| Post to GL. |
-------------*/
int
UpdateGL (void)
{
	/*--------------------------
	| Credit Bills Receivable. |
	--------------------------*/
	sprintf (glwkRec.acc_no, "%-*.*s", 
					MAXLEVEL, MAXLEVEL, (BillsDue) ? crbk_rec.gl_bill_rec
												  : crbk_rec.gl_fwd_rec);	
	strcpy (glwkRec.jnl_type, "2");
	glwkRec.amount 		= no_dec (old_rec_amt);
	glwkRec.loc_amount 	= no_dec (old_rec_amt);
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	WriteGlwk ();

	/*---------------------
	| Debit Bank Account. |
	---------------------*/
	sprintf (glwkRec.acc_no, "%-*.*s", 
					MAXLEVEL, MAXLEVEL,crbk_rec.gl_bank_acct);
	strcpy (glwkRec.jnl_type, "1");
	glwkRec.amount 		= no_dec (new_rec_amt);
	glwkRec.loc_amount 	= no_dec (new_rec_amt);
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	WriteGlwk ();

	/*-------------------------
	| Post Exchange Variance. |
	-------------------------*/
	if (exch_var != 0.00 && MCURR)
	{
		if (exch_var > 0.00)
			strcpy (glwkRec.jnl_type, "6");
		else
		{
			strcpy (glwkRec.jnl_type, "5");
			exch_var *= -1.00;
		}
		strcpy (glwkRec.acc_no, crbk_rec.gl_exch_var);

		glwkRec.amount 		= no_dec (exch_var);
		glwkRec.loc_amount 	= no_dec (exch_var);
		glwkRec.exch_rate 	= 1.00;
		strcpy (glwkRec.currency, loc_curr);
		WriteGlwk ();
	}
	return (EXIT_SUCCESS);
}

/*--------------------------------------
| Create a glwk record for GL posting. |
--------------------------------------*/
void
WriteGlwk (void)
{
	int		monthPeriod;

	/*----------------------
	| Validate GL account. |
	----------------------*/
	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
					MAXLEVEL, MAXLEVEL,glwkRec.acc_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	/*--------------
	| Create glwk. |
	--------------*/
	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	sprintf (glwkRec.acronym, "%-9.6s", cumr_rec.dbt_no);
	sprintf (glwkRec.name, "%-30.30s", crbk_rec.curr_code);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cuhd_rec.receipt_no);

	glwkRec.ci_amt = 0.00;
	glwkRec.o1_amt = bldt_rec.amount;
	glwkRec.o2_amt = bldt_rec.bk_lcl_exch;
	glwkRec.o3_amt = 0.00;
	glwkRec.o4_amt = 0.00;
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	strcpy (glwkRec.tran_type, (BillsDue) ? "20" : "25");
	strcpy (glwkRec.sys_ref, "     ");

	glwkRec.tran_date = bldt_rec.due_date;

	DateToDMY (glwkRec.tran_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	glwkRec.post_date = local_rec.lsystemDate;
	if (glwkRec.post_date < 0L)
		glwkRec.post_date = glwkRec.tran_date;

	sprintf (glwkRec.narrative, "%-20.20s", cuhd_rec.narrative);
	sprintf (glwkRec.alt_desc1, "%-20.20s", cuhd_rec.cheque_no);
	sprintf (glwkRec.alt_desc2, "%-20.20s", cuhd_rec.alt_drawer);
	sprintf (glwkRec.alt_desc3, "%-20.20s", cuhd_rec.db_branch);
	sprintf (glwkRec.batch_no, "%-10.10s",  cuhd_rec.or_no);
	strcpy (glwkRec.user_ref, cuhd_rec.receipt_no);

	strcpy (glwkRec.stat_flag, "2");
	strcpy (glwkRec.run_no, "");

	GL_AddBatch ();
}
