/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crc2togl.c,v 5.1 2001/12/07 03:34:14 scott Exp $
|  Program Name  : (db_crc2togl.c) 
|  Program Desc  : (Update G/L Work Transactions from Receipts)
|                 (file (cusr) , set tran_flag to 2)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crc2togl.c,v $
| Revision 5.1  2001/12/07 03:34:14  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crc2togl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crc2togl/db_crc2togl.c,v 5.1 2001/12/07 03:34:14 scott Exp $";

#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<twodec.h>

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int		pid,		/* Process id number for work files.     */
			cusr_no;	/* File no. of cusr work file.		 */
	int		first_receipt;

   	char 	systemDate [11],
			work_date [11],
   			previousReceipt [9];
	char	currentBank [6];

	long	lsystemDate = 0L;

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;


	/*===========================================
	| Customer Sundry Receipts Work File Record. |
	===========================================*/
	struct CUSR_REC
	{
		char	co_no [3];
		char    est [3];
		char    rpt_no [9];
		long    date_of_rpt;
		char    type [2];
		char    pay_type [2];
		char    gl_acc_no [MAXLEVEL + 1];
		char    p_no [3];
		char    name [41];
		char    acronym [10];
		char    narrative [21];
		char    bank_code [4];
		char    branch_code [21];
		Money   amt;   
		Money   disc;   
		Money   p_amt;   
		Money   tax;      
		Money   gst;       
		Money   freight;    
		char    stat_flag [2];

		char    bk_id [6];
		char    bk_curr [4];
		double  bk_exch;
		Money   bk_rec_amt;  
		Money   bk_charge;    
		double  bk_lcl_exch;

		Money	o_p_amt;		
		char	o_curr [4];
		double  o_exch;
		Money   o_disc;  
		Money   o_amount;
	} cusr2_rec, cusr_rec;

	char	*data = "data";

	int		lpno;

#include	<CashBook.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	Update 				 (void);
void 	WriteGlwk 			 (struct CUSR_REC *);
void 	PostDebit 			 (void);
int 	CheckBankDetails 	 (void);
void 	shutdown_prog 		 (void);

int
main (
 int                argc,
 char*              argv [])
{
	if (argc < 3)
	{
		print_at (0,0,"Usage: %s <pid> <lpno>", argv [0]);
        return (EXIT_FAILURE);
	}
	pid   = atoi (argv [1]);
	lpno  = atoi (argv [2]);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	OpenDB ();
 
 	init_scr ();
	print_mess (ML ("Creating Customer Sundry Receipts Journals."));

	/*=================================
	| Process receipts transactions . |
	=================================*/
	first_receipt = TRUE;
	cc = RF_READ (cusr_no, (char *) &cusr_rec);
	while (!cc)
	{
		Update ();

		/*------------------------------------------
		| Store contents of cusr_rec in cusr2_rec. |
		------------------------------------------*/
		memcpy ( (char *)&cusr2_rec, (char *)&cusr_rec, sizeof (struct CUSR_REC));

		cc = RF_READ (cusr_no, (char *) &cusr_rec);
	}

	if (!first_receipt)
	{
		PostDebit ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/db_crc%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);
	cc = RF_OPEN (filename,sizeof (cusr_rec),"r",&cusr_no);
	if (cc) 
		sys_err ("Error in db_crc During (WKOPEN)",cc, PNAME);

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");

	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
	OpenCashBook ();
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (crbk);


	cc = RF_CLOSE (cusr_no);
	if (cc) 
		sys_err ("Error in cusr During (WKCLOSE)", cc, PNAME);

	GL_CloseBatch (lpno);
	GL_Close ();
	CloseCashBook ();
	abc_dbclose (data);
}

/*===================== 
|Transaction records .| 
=====================*/
void
Update (void)
{
	/*---------------------------------------------------
	| If date is Zero then set to current debtors date. |
	---------------------------------------------------*/
	if (cusr_rec.date_of_rpt == 0L)
		cusr_rec.date_of_rpt = comm_rec.dbt_date;

	/*-----------------------
	| Create glwk records . |
	-----------------------*/
	if (strcmp (cusr_rec.rpt_no, previousReceipt))
	{
		if (!first_receipt)
			PostDebit ();

		strcpy (previousReceipt, cusr_rec.rpt_no);
	}
	else
	{
		strcpy (cusr_rec.acronym, "         ");
		sprintf (cusr_rec.name,   "%-40.40s", " ");
	}

	strcpy (glwkRec.jnl_type, "4");
	if (cusr_rec.p_amt < 0.00)
	{
		strcpy (glwkRec.jnl_type, "3");
		cusr_rec.p_amt *= -1.00;
		cusr_rec.o_p_amt *= -1.00;
	}
	WriteGlwk (&cusr_rec);

	first_receipt = FALSE;
}

void
WriteGlwk (
 struct CUSR_REC*   cash_rec)
{
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no,cash_rec->gl_acc_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	/*---------------------------------------------
	| Add transaction for account if required   . |
	---------------------------------------------*/
	strcpy (glwkRec.tran_type,   "26");
	sprintf (glwkRec.sys_ref,    "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.period_no,   cash_rec->p_no);
	strcpy (glwkRec.acc_no, cash_rec->gl_acc_no);
	strcpy (glwkRec.co_no,       cash_rec->co_no);
	strcpy (glwkRec.est_no,      cash_rec->est);
	sprintf (glwkRec.name,       "%-30.30s", cash_rec->name);
	strcpy (glwkRec.narrative,   cash_rec->narrative);
	sprintf (glwkRec.alt_desc1,   "%-20.20s", cash_rec->name);
	sprintf (glwkRec.alt_desc2,   "%-20.20s", cash_rec->name + 20);
	sprintf (glwkRec.alt_desc3,   "%-20.20s", cash_rec->branch_code);
	strcpy (glwkRec.batch_no,    " ");
	strcpy (glwkRec.acronym,     cash_rec->acronym);
	strcpy (glwkRec.user_ref,    cash_rec->rpt_no);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cash_rec->rpt_no);
	strcpy (glwkRec.stat_flag,   "2");
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	glwkRec.tran_date      = cash_rec->date_of_rpt;
	glwkRec.ci_amt    = cash_rec->amt;
	glwkRec.post_date = StringToDate (systemDate);

	if (glwkRec.post_date < 0)
		glwkRec.post_date = 0;

	glwkRec.loc_amount 	= cash_rec->p_amt;
	glwkRec.amount 		= cash_rec->o_p_amt;
	strcpy (glwkRec.currency, cash_rec->o_curr);
	glwkRec.exch_rate 	= cash_rec->o_exch;
		
	GL_AddBatch ();
}

void
PostDebit (void)
{
	strcpy (cusr2_rec.name,   "                                        ");
	strcpy (cusr2_rec.acronym,"         ");
	strcpy (cusr2_rec.co_no,  comm_rec.co_no);
	strcpy (glwkRec.jnl_type,"4");

	/*-----------------------
	| Post To Bank Account. |
	-----------------------*/
	CheckBankDetails ();
	sprintf (cusr2_rec.gl_acc_no, "%-*.*s", 
							MAXLEVEL,MAXLEVEL,crbk_rec.gl_bank_acct);
	strcpy (glwkRec.jnl_type, "1");

	cusr2_rec.p_amt 		= cusr2_rec.amt 		- cusr2_rec.disc;
	cusr2_rec.o_p_amt 	= cusr2_rec.o_amount - cusr2_rec.o_disc;

	WriteGlwk (&cusr2_rec);

	sprintf (err_str, "Sundry Receipt  %8.8s / Ref %-9.9s",
					cusr2_rec.rpt_no, cusr2_rec.acronym);
	/*---------------------------------
	| Write entry to cash book system |
	---------------------------------*/
	WriteCashBook
	(							/*--------------------------*/
		cusr2_rec.co_no,		/* Company Number			*/
		cusr2_rec.est,			/* Branch Number.			*/
		cusr2_rec.bk_id,		/* Bank Id.					*/
		cusr2_rec.date_of_rpt,	/* Transaction Date			*/
		err_str,				/* Transaction Narrative.	*/
		"R",					/* Transaction Type.		*/
		cusr2_rec.p_amt, 		/* Amount posted to bank.	*/
		"0",					/* Status flag.				*/
		lsystemDate				/* System/period date.		*/
	);							/*--------------------------*/

	/*----------------
	| Post Discount. |
	----------------*/
	GL_GLI 
	(
		cusr2_rec.co_no,		/*	Company Number 	*/
		cusr2_rec.est,			/*	Branch No.		*/
		"  ",					/*	Warehouse No.	*/
		"DISC ALLOW",			/*	Interface Code. */
		"  ",					/*	Customer Type.	*/
		" "
	);

	strcpy (cusr2_rec.gl_acc_no, glmrRec.acc_no);
	strcpy (glwkRec.jnl_type, "3");
	if (cusr2_rec.disc != 0.00 || cusr_rec.o_disc != 0.00)
	{
		cusr2_rec.p_amt 	= cusr2_rec.disc;
		cusr2_rec.o_p_amt 	= cusr2_rec.o_disc;
		WriteGlwk (&cusr2_rec);
	}

	/*--------------------
	| Post Bank Charges. |
	--------------------*/
	if (cusr2_rec.pay_type [0] == 'C')
		cusr2_rec.bk_charge += crbk_rec.clear_fee;
	if (cusr2_rec.bk_charge != 0.00)
	{
		/*---------------------
		| Debit Bank Charges. |
		---------------------*/
		strcpy (cusr2_rec.gl_acc_no, crbk_rec.gl_bank_chg);
		strcpy (glwkRec.jnl_type, "5");
		cusr2_rec.p_amt		=	no_dec
								(
									cusr2_rec.bk_charge / 	
									cusr2_rec.bk_lcl_exch
								);
		cusr2_rec.o_p_amt	=	no_dec
								(
									cusr2_rec.bk_charge
								);
		WriteGlwk (&cusr2_rec);

		/*----------------------
		| Credit Bank Account. |
		----------------------*/
		strcpy (cusr2_rec.gl_acc_no, crbk_rec.gl_bank_acct);
		strcpy (glwkRec.jnl_type, "6");
		cusr2_rec.p_amt	=	no_dec
							(
								cusr2_rec.bk_charge / 
								cusr2_rec.bk_lcl_exch
							);
		cusr2_rec.o_p_amt = no_dec (cusr2_rec.bk_charge);

		sprintf (err_str, "Sundry Receipt  %8.8s / Ref %-9.9s",
							cusr2_rec.rpt_no, cusr2_rec.acronym);
		/*---------------------------------
		| Write entry to cash book system |
		---------------------------------*/
		WriteCashBook
		(							/*--------------------------*/
			cusr2_rec.co_no,		/* Company Number			*/
			cusr2_rec.est,			/* Branch Number.			*/
			cusr2_rec.bk_id,		/* Bank Id.					*/
			cusr2_rec.date_of_rpt,	/* Transaction Date			*/
			err_str,				/* Transaction Narrative.	*/
			"R",					/* Transaction Type.		*/
			cusr2_rec.p_amt, 		/* Amount posted to bank.	*/
			"0",					/* Status flag.				*/
			lsystemDate				/* System/period date.		*/
		);							/*--------------------------*/
		WriteGlwk (&cusr2_rec);
	}
}

/*-----------------------------------------
| Get bank details if bank has changed.   |
-----------------------------------------*/
int
CheckBankDetails (void)
{
	/*----------------------
	| Bank hasn't changed. |
	----------------------*/
	if (!strcmp (currentBank, cusr2_rec.bk_id))
		return (TRUE);

	strcpy (crbk_rec.co_no, cusr2_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", cusr2_rec.bk_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	sprintf (currentBank, "%-5.5s", crbk_rec.bank_id);

	return (TRUE);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

