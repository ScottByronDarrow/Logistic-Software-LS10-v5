/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_bk_tran.c,v 5.3 2002/07/09 08:39:23 scott Exp $
|  Program Name  : (ca_bk_tran.c)                                    |
|  Program Desc  : (Maintenance of System Bank Transactions)		  |
|---------------------------------------------------------------------|
|  Date Written  : 04/06/1996      | Author 	 : Jiggs Veloz        |
|---------------------------------------------------------------------|
| $Log: ca_bk_tran.c,v $
| Revision 5.3  2002/07/09 08:39:23  scott
| Converted to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_bk_tran.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_bk_tran/ca_bk_tran.c,v 5.3 2002/07/09 08:39:23 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_ca_mess.h>

#define		ADD_TR (local_rec.action [0] == 'A')
#define		MOD_TR (local_rec.action [0] == 'M')
#define		DEL_TR (local_rec.action [0] == 'D')

#define		INTEREST 	 (cbbt_rec.tran_type [0] == 'I')
#define		ADJUSTMENT 	 (cbbt_rec.tran_type [0] == 'A')
#define		CHARGE 		 (cbbt_rec.tran_type [0] == 'C')
#define		PETTYCASH 	 (cbbt_rec.tran_type [0] == 'P')

static char	*data	  = "data",
			*DBADD    = "DBADD",
			*DBUPDATE = "DBUPDATE";

#include	"schema"

struct commRecord	comm_rec;
struct cbbtRecord	cbbt_rec;
struct crbkRecord	crbk_rec;
struct esmrRecord	esmr_rec;
struct pocrRecord	pocr_rec;

	int		NewTran = 0;

/*=============================
| Other Variable Declarations |
=============================*/
char	systemDate [11];
		
/*===========================+
| Local & Screen Structures. |
+===========================*/
struct {
	char	action [2];
	char	tt_desc [16];
	char	dummy [11];
	char	prev_bank [6];
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "action", 4, 20, CHARTYPE, 
		"U", "          ", 
		" ", "A", " Action (A M D)", "Enter A(dd) or M(odify) or D(elete) Transaction.", 
		NE, NO, JUSTLEFT, "AMD", "", local_rec.action}, 
	{1, LIN, "bank_id", 5, 20, CHARTYPE, 
		"UUUUU", "          ", 
		" ", local_rec.prev_bank, " Bank Code", "Enter Bank Code. [SEARCH] available. <return> - last Bank", 
		YES, NO, JUSTLEFT, "", "", crbk_rec.bank_id}, 
	{1, LIN, "bank_name", 5, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.bank_name}, 
	{1, LIN, "bank_no", 6, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", " Bank No.", "Enter bank number. ", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.bank_no}, 
	{1, LIN, "br_name", 7, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Branch Name", "", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.branch_name}, 
	{1, LIN, "bk_acno", 8, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", " Account No.", "", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.bank_acct_no}, 
	{1, LIN, "acct_name", 9, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Account Name", "", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.acct_name}, 
	{1, LIN, "curcode", 10, 20, CHARTYPE, 
		"UUU", "          ", 
		" ", "", " Currency Code", "", 
		NA, NO, JUSTLEFT, "", "", crbk_rec.curr_code}, 
	{1, LIN, "curdesc", 10, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", pocr_rec.description}, 
	{1, LIN, "tran_no", 12, 20, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", " Transaction Number", "Enter Transaction No. [SEARCH] Available.", 
		NA, NO, JUSTRIGHT, "", "", (char *) &cbbt_rec.tran_no}, 
	{1, LIN, "tran_type", 13, 20, CHARTYPE, 
		"U", "          ", 
		" ", "", " Transaction Type", "Enter [A]djustment, [C]harge, [I]nterest, [P]etty cash.", 
		NI, NO, JUSTLEFT, "ACIP", "", cbbt_rec.tran_type}, 
	{1, LIN, "desc", 13, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.tt_desc}, 
	{1, LIN, "tran_date", 15, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", " Date", "Enter Transaction Date. [DD/MM/YY]", 
		YES, NO, JUSTLEFT, "", "", (char *) &cbbt_rec.tran_date}, 
	{1, LIN, "tran_desc", 16, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " Description", "Enter Description. ", 
		YES, NO, JUSTLEFT, "", "", cbbt_rec.tran_desc}, 
	{1, LIN, "tran_amt", 17, 20, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0", " Amount", "Enter Amount. ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &cbbt_rec.tran_amt}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 

};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void    OpenDB (void);
void    CloseDB (void);
void    shutdown_prog (void);
int		heading (int);

void    UpdateTran (void);
void    AddTran (void);
int     delete_tran (void);
void    SrchCrbk (char *);
void    SrchCbbt (char *);


/*=====================================================================
| Function		:	main (int argc, char* argv [])
|
| Description	:	Main Processing Routine.
|
| Parameters	:	int     argc
|                  char*   argv []
|
| Return		:	status
=====================================================================*/
int
main (
 int    argc,
 char*  argv [])
{

	strcpy (systemDate, DateToString (TodaysDate ()));

	SETUP_SCR (vars);

	/*----------------------------
	| Set up required parameters |
	----------------------------*/
	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1);
	OpenDB ();

	strcpy (local_rec.prev_bank, esmr_rec.dflt_bank);

	while (prog_exit == 0)
	{
		/*-------------------------
		|   Reset control flags   |
		-------------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);

		/*-----------------------------
		| Entry screen 1 linear input |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;
	
		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
        {
			continue;
        }

		if (!NewTran)
        {
			UpdateTran ();
        }
		else
		{
			if (ADD_TR)
            {
				AddTran ();
            }
		}

	}
	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=====================================================================
| Function		:	shutdown_prog (void)
|
| Description	:	Program exit sequence.
|
| Parameters	:	None.
|
| Return		:	None.
=====================================================================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}
/*=====================================================================
| Function		:	OpenDB (void)
|
| Description	:	Open Database Files.
|
| Parameters	:	None.
|
| Return		:	None.
=====================================================================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cbbt, cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no1");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no"); 

	strcpy (esmr_rec.co_no,	comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*=====================================================================
| Function		:	CloseDB (void)
|
| Description	:	Close Database Files.
|
| Parameters	:	None.
|
| Return		:	None.
=====================================================================*/
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_fclose (esmr);
	abc_fclose (pocr); 

	abc_dbclose (data);
}

/*=====================================================================
| Function		:	spec_valid ()
|
| Description	:	Special Validation for screen fields.
|
| Parameters	:	int field
|
| Return		:	int
=====================================================================*/
int
spec_valid (
 int    field)
{
	int	PrmptAnswer;

	if ((MOD_TR) || (DEL_TR)) 
	{
		FLD ("bank_id") 	= NE;
		FLD ("tran_no") 	= NE;
		FLD ("tran_type") 	= NI;
	}
	if (ADD_TR)
	{
		FLD ("bank_id") 	= YES;
		FLD ("tran_no") 	= NA;
		FLD ("tran_type") 	= YES;
	}	

	/*------------------------------------
	| Validate Bank ID And Allow Search. |
	------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (FLD ("bank_id") == NA)
  			return (EXIT_SUCCESS);

		if (dflt_used)
			strcpy (crbk_rec.bank_id, local_rec.prev_bank);

		if (SRCH_KEY)
		{
	 		SrchCrbk (temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (crbk_rec.co_no, comm_rec.co_no);
		strcpy (local_rec.prev_bank, crbk_rec.bank_id);
		strcpy (crbk_rec.bank_id, local_rec.prev_bank);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, "Bank Code %s is NOT on file", crbk_rec.bank_id);
			print_mess (err_str);*/
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, crbk_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		DSP_FLD ("bank_id"); 
		DSP_FLD ("bank_name");
		DSP_FLD ("bank_no");
		DSP_FLD ("br_name");
		DSP_FLD ("bk_acno");
		DSP_FLD ("acct_name");
		DSP_FLD ("curcode");
		DSP_FLD ("curdesc");
		strcpy (local_rec.prev_bank, crbk_rec.bank_id);
		entry_exit = 0; 
		return (EXIT_SUCCESS); 
	}

	/*----------------------------
	| Validate Transaction Type  |
	----------------------------*/
	if (LCHECK ("tran_type"))
	{
		if (ADJUSTMENT)
			strcpy (local_rec.tt_desc, "Adjustment");

		if (CHARGE)
			strcpy (local_rec.tt_desc, "Charge");

		if (INTEREST)
			strcpy (local_rec.tt_desc, "Interest");

		if (PETTYCASH)
			strcpy (local_rec.tt_desc, "Petty Cash");

		DSP_FLD ("tran_type"); 
		DSP_FLD ("desc");

		if (prog_status != ENTRY)
		{
			/*-----------------------------------------------
			| Force re-edit of amount if special validation |
			| fails with new transaction type. 				|
			-----------------------------------------------*/
			while (spec_valid (label ("tran_amt")))
			{
				get_entry (label ("tran_amt"));
				if (restart)
				{
					restart = FALSE;
					return (EXIT_FAILURE);
				}
			}

		}
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Transaction Date. |
	-----------------------------*/
	if (LCHECK ("tran_date"))
	{
		if (dflt_used)
		{
			cbbt_rec.tran_date = TodaysDate ();
			DSP_FLD ("tran_date");
			return (EXIT_SUCCESS);
		}
		if (cbbt_rec.tran_date > TodaysDate ())
		{
			print_mess (ML (mlCaMess016));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Transaction Number. |
	------------------------------*/
	if (LCHECK ("tran_no"))
	{
		if (ADD_TR)
		{		
			FLD ("tran_no") 	= NA;
			FLD ("tran_type") 	= YES;
			NewTran	=	TRUE;
			cbbt_rec.tran_no = esmr_rec.nx_csh_trn_no + 1;
			DSP_FLD ("tran_no");
			return (EXIT_SUCCESS);
		}

		FLD ("tran_no") 	= NE;
		FLD ("tran_type") 	= NI;

		if (MOD_TR) 
		{
			if (SRCH_KEY)
			{
				SrchCbbt (temp_str);
				return (EXIT_SUCCESS);
			}
			strcpy (cbbt_rec.co_no, comm_rec.co_no);
			strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
			NewTran = find_rec (cbbt, &cbbt_rec, COMPARISON, "u");
			if (NewTran)
			{
				print_mess (ML (mlStdMess016));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (DEL_TR)
		{
			if (SRCH_KEY)
			{
				SrchCbbt (temp_str);
				return (EXIT_SUCCESS);
			}
			strcpy (cbbt_rec.co_no, comm_rec.co_no);
			strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
			NewTran = find_rec (cbbt, &cbbt_rec, EQUAL, "u");
			if (NewTran)
			{
				print_mess (ML (mlStdMess016));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			scn_display (1);
			PrmptAnswer = prmptmsg (ML (mlCaMess017), "YyNn", 1,23);
			move (1,23);
			if (PrmptAnswer != 'Y' && PrmptAnswer != 'y')
			{	
				restart = TRUE;
				return (EXIT_SUCCESS);
			}
			else
			{
				delete_tran ();
				restart = TRUE;
			}
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Transaction Amount. |
	------------------------------*/
	if (LCHECK ("tran_amt"))
	{
		if (cbbt_rec.tran_type [0] == 'C' || cbbt_rec.tran_type [0] == 'P')
		{
			if (cbbt_rec.tran_amt < 0.00)
			{
				DSP_FLD ("tran_amt");
				return (EXIT_SUCCESS);
			}
			print_mess (ML (mlCaMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (cbbt_rec.tran_type [0] == 'I')
		{
			if (cbbt_rec.tran_amt >= 0.00)
			{		
				DSP_FLD ("tran_amt");
				return (EXIT_SUCCESS);
			}
			print_mess (ML (mlCaMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=====================================================================
| Function		:	AddTran (void)
|
| Description	:	Add bank transactions.
|
| Parameters	:	None.
|
| Return		:	None.
=====================================================================*/
void
AddTran (
 void)
{
	clear ();

	/*------------------------------------------------------------
	| Read branch record and add one to cash transaction number. |
	------------------------------------------------------------*/
	strcpy (esmr_rec.co_no,	comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, esmr, "DBFIND");

	esmr_rec.nx_csh_trn_no++;
	cc = abc_update (esmr,&esmr_rec);
	if (cc) 
		file_err (cc, esmr, DBUPDATE);

	/*---------------------------------------
	| Add System Bank Transaction to File . |
	---------------------------------------*/
	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = esmr_rec.nx_csh_trn_no;
	strcpy (cbbt_rec.reconciled, "N");
	strcpy (cbbt_rec.stat_post, "Y");

	cc = abc_add (cbbt, &cbbt_rec);
	if (cc)
		file_err (cc, cbbt, DBADD);
}

/*=====================================================================
| Function		:	UpdateTran (void)
|
| Description	:	Update bank transactions.
|
| Parameters	:	None.
|
| Return		:	None.
=====================================================================*/
void
UpdateTran (
 void)
{
	clear ();

	/*-----------------------------------
	| Update System Bank Transactions . |
	-----------------------------------*/
	cc = abc_update (cbbt, &cbbt_rec);
	if (cc)
		file_err (cc, cbbt, DBUPDATE);
	
}

/*=====================================================================
| Function		:	delete_tran (void)
|
| Description	:	Delete bank transactions
|
| Parameters	:	None.
|
| Return		:	0 - Success, else Error.
=====================================================================*/
int
delete_tran (
 void)
{
	cc = abc_delete (cbbt);
	if (cc)
		file_err (cc, cbbt, "DBDELETE"); 
	return (EXIT_SUCCESS);
}

/*=====================================================================
| Function		:	heading (int scn)
|
| Description	:	Display screen headings.
|
| Parameters	:	int scn
|
| Return		:	0 - Success, else Error.
=====================================================================*/
int
heading (
 int    scn)
{
	if (restart)
    {
		return (EXIT_FAILURE);
    }

	scn_set (scn);
	clear ();
	move (0,1);
	rv_pr (ML (mlCaMess069),23,1,1);
	move (0,2);
	line (80);
	box (0,3,80,14);
	move (1,11); line (79);
	move (1,14); line (79);
	move (0,20); line (80);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, clip (comm_rec.co_short));
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no, clip (comm_rec.est_short));
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

/*=====================================================================
| Function		:	SrchCrbk (char* key_val)
|
| Description	:	Search for Bank Id file.
|
| Parameters	:	char*   key_val
|
| Return		:	None
=====================================================================*/
void
SrchCrbk (
 char*  key_val)
{
	_work_open (5,0,40);
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s",key_val);
	save_rec ("#Bank Id", "#Bank Name");
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp (crbk_rec.co_no, comm_rec.co_no) 
	&&     !strncmp (crbk_rec.bank_id, key_val, strlen (key_val)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, crbk, "DBFIND");
}

/*=====================================================================
| Function		:	SrchCbbt (char* key_val)
|
| Description	:	Search for Tran No file.
|
| Parameters	:	char*   key_val
|
| Return		:	None
=====================================================================*/
void
SrchCbbt (
 char*  key_val)
{	
	char	no_type [14];
	char	date_desc [53];
	_work_open (15,0,40);

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = atol (key_val);
	
	save_rec ("#Tran #  Type", "#Date        Description");
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc && !strcmp (cbbt_rec.co_no, comm_rec.co_no) &&
				  !strcmp (cbbt_rec.bank_id, crbk_rec.bank_id))
	{
		sprintf (no_type, "%06ld  %-4.4s" , 
							cbbt_rec.tran_no,cbbt_rec.tran_type);
		sprintf (date_desc, "%s  %-40.40s" , 
							DateToString (cbbt_rec.tran_date), cbbt_rec.tran_desc);
		cc = save_rec (no_type, date_desc);
		if (cc)
			break;

		cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = atol (temp_str);
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, "cbbt", "DBFIND");
}

/*=====================================================================
| Function		:	pocr_search (char* key_val)
|
| Description	:	Serach routine for Purchase Order Currency File.
|
| Parameters	:	char*   key_val
|
| Return		:	None
=====================================================================*/
void
pocr_search (
 char*  key_val)
{
	_work_open (3,0,40);
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, key_val);
	cc = save_rec ("#No","#Currency Description");
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strncmp (pocr_rec.code,key_val,strlen (key_val)) && 
		      	  !strcmp (pocr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code,pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,temp_str);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, pocr, "DBFIND");
}
