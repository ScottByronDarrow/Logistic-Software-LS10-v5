/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_ictogl.c,v 5.3 2002/07/16 06:27:37 scott Exp $
|  Program Name  : (cr_ictogl.c)                                    |
|  Program Desc  : (Update General Ledger Work Transactions from)  |
|               (Suppliers Invoice / Credit Work File (file suwk)) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_ictogl.c,v $
| Revision 5.3  2002/07/16 06:27:37  scott
| Updated from service calls and general maintenance.
|
| Revision 5.2  2001/08/09 08:51:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:26  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_ictogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_ictogl/cr_ictogl.c,v 5.3 2002/07/16 06:27:37 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#define 	MAXCTRLS 100
#include 	<ml_cr_mess.h>

#include 	"schema"

#define	INVOICE		 (suwk_rec.type [0] == '1')
#define	CREDIT		 (suwk_rec.type [0] == '2')

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int		pidNumber,
			suwkNumber,
			firstRecord	= FALSE,
   			noDataFound = FALSE;

   	char 	pidString [6],
   			previousInvoice [16],
   			previousSupplier [7],
	    	envCrGlRef [2],
	    	glBatchType [2],
			branchNumbr [3],
			invoiceNumber [16],
			supplierNumber [7],
	     	controlAccount [MAXLEVEL + 1];
	
	struct	commRecord	comm_rec;
	struct	sumrRecord	sumr_rec;

	/*======================
	| Suppliers Work File. |
	======================*/
	struct {
		char	co_no [3];
		char	est [3];
		char	invoiceNumber [sizeof glwkRec.user_ref];
		char	cus_po_no [sizeof glwkRec.user_ref];
		char	type [2];
		char	supplierNumber [7];
		Date	date_of_inv;
		Date	gl_date;
		double	exch_rate;
		Money	fx_disc;
		Money	loc_disc;
		Money	fx_gst;
		Money	loc_gst;
		Money	tot_fx;
		Money	tot_loc;
		char	gl_acc_no [sizeof glwkRec.acc_no];
		Money	fx_amt;
		Money	loc_amt;
		char	narrative [sizeof glwkRec.narrative];
		char	user_ref [sizeof glwkRec.user_ref];
		char	currency [4];
		char	stat_flag [2];
	}	suwk_rec;

	int		printerNumber;


/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		FindGlmr		 (char *);
void	shutdown_prog	 (void);
void	PostTransactions (void);
void	WriteControl	 (char *, double, double);
void	WriteGst		 (double, double);
void	WriteDiscount	 (double, double);
void	WriteGlwk		 (void);


int
main (
	int		argc,
	char	*argv [])
{
	int		eof = 0;

	if (argc < 3 || (pidNumber = atoi (argv [1])) < 1)
	{
		/*--------------------------------
		| Usage : %s <pid> printerNumber |
		--------------------------------*/
		print_at (0, 0, mlCrMess015, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [2]);

	sprintf (pidString,"%05d",pidNumber);
	
	strcpy (glwkRec.name,"                              ");

	sprintf (envCrGlRef, "%-1.1s", get_env ("CR_GLREF"));

	set_tty ();

	OpenDB ();

	print_mess (ML ("Creating Payables Journals."));

	/*---------------------------------
	| Process Payables transactions . |
	---------------------------------*/
	while (eof == 0) 
	{
		cc = RF_READ (suwkNumber, (char *) &suwk_rec);
		if (cc) 
		{
			eof = 1;
			continue;
		}

		if (firstRecord == FALSE)
		{
			strcpy (glBatchType,suwk_rec.type);
			firstRecord = TRUE;
		}
		if (suwk_rec.type [0] == glBatchType [0])
			PostTransactions (); 
		else
		{
			/*------------------------------------
			| Error processing cr_suwk work file |
			------------------------------------*/
			errmess (ML (mlCrMess016));
			sleep (20);
		}
	}

	if (noDataFound == TRUE)
	{
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
	char *	sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/cr_suwk%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);

	cc = RF_OPEN (filename,sizeof (suwk_rec),"u",&suwkNumber);
	if (cc) 
		file_err (cc, "suwk", "WKOPEN");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no3");

	OpenGlmr ();
	OpenPocr ();
	
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	RF_DELETE (suwkNumber);

	abc_fclose (sumr);
	GL_CloseBatch (printerNumber);

	GL_Close ();
	abc_dbclose ("data");
}

/*============================
| Read gl account master.    |
============================*/
int
FindGlmr (
 char *	account)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P') 
	      return (2);
	
	return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=============================
| Update Transaction Records. |
=============================*/
void
PostTransactions (
 void)
{
	int		month;
	double	disc_fx_amt 	= 0.0, 
			disc_loc_amt 	= 0.00,
			gst_fx_amt 		= 0.0, 
			gst_loc_amt 	= 0.0, 
			acct_fx_amt 	= 0.00,
			acct_loc_amt 	= 0.00;

	if (CREDIT)
	{
		suwk_rec.fx_disc 	*= -1;
		suwk_rec.loc_disc 	*= -1;
		suwk_rec.fx_gst 	*= -1;
		suwk_rec.loc_gst 	*= -1;
		suwk_rec.tot_fx 	*= -1;
		suwk_rec.tot_loc 	*= -1;
		suwk_rec.fx_amt 	*= -1;
		suwk_rec.loc_amt 	*= -1;
	}
	/*-----------------------
	| Setup Period.         |
	-----------------------*/
	DateToDMY (suwk_rec.gl_date, NULL, &month, NULL);

	/*----------------------------------------
	| Process first allocn for new inv/crd . |
	----------------------------------------*/
	if (strcmp (suwk_rec.invoiceNumber,previousInvoice) || 
		strcmp (suwk_rec.supplierNumber,previousSupplier))
	{
		/*------------------------------------------------
		| Get control account for supplier.              |
		------------------------------------------------*/
		strcpy (sumr_rec.co_no,suwk_rec.co_no);
		strcpy (sumr_rec.crd_no,suwk_rec.supplierNumber);
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (!cc) 
		{
			sprintf (controlAccount,"%-*.*s", 
							MAXLEVEL,MAXLEVEL,sumr_rec.gl_ctrl_acct);
			cc = FindGlmr (controlAccount);
			if (cc)
			{
				strcpy (pocrRec.co_no,sumr_rec.co_no);
				strcpy (pocrRec.code,sumr_rec.curr_code);
				cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
				if (!cc)
				{
					strcpy (controlAccount,pocrRec.gl_ctrl_acct);
					cc = FindGlmr (controlAccount);
				}
			}
		}
		if (cc)
		{
			GL_GLI 
			 (
				comm_rec.co_no,
				suwk_rec.est,
				"  ",
				"ACCT PAY  ",
				"   ",
				" "
			);
			strcpy (controlAccount, glmrRec.acc_no);

			cc = FindGlmr (controlAccount);
			if (cc)
				file_err (cc, glmr, "DBFIND");

		}
		disc_fx_amt 	= suwk_rec.fx_disc;
		disc_loc_amt 	= suwk_rec.loc_disc;
		gst_fx_amt 		= suwk_rec.fx_gst;
		gst_loc_amt 	= suwk_rec.loc_gst;
	  	acct_fx_amt 	= suwk_rec.fx_gst - suwk_rec.fx_disc;
	  	acct_loc_amt 	= suwk_rec.loc_gst - suwk_rec.loc_disc;

		glwkRec.ci_amt 		= suwk_rec.tot_fx;
		glwkRec.o1_amt 		= suwk_rec.fx_disc;
		glwkRec.exch_rate 	= suwk_rec.exch_rate;
		glwkRec.o2_amt 		= 0;
		glwkRec.o3_amt 		= 0;
		glwkRec.o4_amt 		= suwk_rec.fx_gst;
		strcpy (previousInvoice, suwk_rec.invoiceNumber);
		strcpy (previousSupplier,suwk_rec.supplierNumber);
		strcpy (invoiceNumber,	 suwk_rec.invoiceNumber);
		strcpy (branchNumbr,	 suwk_rec.est);
		strcpy (supplierNumber,	 suwk_rec.supplierNumber);
	}
	else
	{
		strcpy (invoiceNumber,"               ");
		strcpy (branchNumbr,"  ");
		strcpy (supplierNumber,"      ");
		glwkRec.ci_amt 		= 0;
		glwkRec.exch_rate 	= 1.0;
		glwkRec.o1_amt 		= 0;
		glwkRec.o2_amt 		= 0;
		glwkRec.o3_amt 		= 0;
		glwkRec.o4_amt 		= 0;
	}

	glwkRec.amount 		= suwk_rec.fx_amt;
	glwkRec.loc_amount 	= suwk_rec.loc_amt;
	strcpy (glwkRec.currency, suwk_rec.currency);

	acct_fx_amt 	+= suwk_rec.fx_amt;
	acct_loc_amt 	+= suwk_rec.loc_amt;
 
	/*----------------------------------------
	| Process gl allocation for inv/crd .    |
	----------------------------------------*/
	strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");

	if (suwk_rec.stat_flag [0] != '9')
	{	
		strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");
		WriteGlwk ();

		strcpy (glwkRec.jnl_type, (INVOICE) ? "2" : "1");
		WriteControl (controlAccount, acct_fx_amt, acct_loc_amt);

		strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");

		WriteGst (gst_fx_amt, gst_loc_amt);

		strcpy (glwkRec.jnl_type, (INVOICE) ? "2" : "1");
		WriteDiscount (disc_fx_amt, disc_loc_amt);

		noDataFound = TRUE;

		/*----------------------------------------
		| Update suwk stat flag for record.      |
		----------------------------------------*/
		strcpy (suwk_rec.stat_flag, "9");
		cc = RF_UPDATE (suwkNumber, (char *) &suwk_rec);
		if (cc)
			file_err (cc, "suwk", "WKUPDATE");
	}
}

void
WriteControl (
 char *	acct_no,
 double	fx_amt,
 double	loc_amt)
{
	if (!fx_amt)
		return;

	glwkRec.ci_amt = 0.0;
	glwkRec.o1_amt = 0.0;
	glwkRec.o2_amt = 0.0;
	glwkRec.o3_amt = 0.0;
	glwkRec.o4_amt = 0.0;

	sprintf (suwk_rec.gl_acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,acct_no);
	glwkRec.amount 		= fx_amt;
	glwkRec.loc_amount 	= loc_amt;
	WriteGlwk ();
}

void
WriteGst (
 double	fx_amt,
 double	loc_amt)
{
	if (!fx_amt)
		return;

	GL_GLI 
	 (
		comm_rec.co_no,
		suwk_rec.est,
		"  ",
		"G.S.T PAID",
		"   ",
		" "
	);
	strcpy (suwk_rec.gl_acc_no, glmrRec.acc_no);
	glwkRec.amount 		= fx_amt;
	glwkRec.loc_amount 	= loc_amt;
	WriteGlwk ();
}

void
WriteDiscount (
	double	fx_amt,
	double	loc_amt)
{
	if (!fx_amt)
		return;

	GL_GLI 
	 (
		comm_rec.co_no,
		suwk_rec.est,
		"  ",
		"DISC ALLOW",
		"   ",
		" "
	);
	strcpy (suwk_rec.gl_acc_no, glmrRec.acc_no);
	glwkRec.amount 		= fx_amt;
	glwkRec.loc_amount 	= loc_amt;
	WriteGlwk ();
}

/*=================================
| Write GLWK Transaction Records. |
=================================*/
void
WriteGlwk (void)
{
	int		month;
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", 
								MAXLEVEL,MAXLEVEL,suwk_rec.gl_acc_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str, "DBFIND - %s", suwk_rec.gl_acc_no);
		file_err (cc, glmr, err_str);
	}
		
	/*-------------------------------------------
	| Add transaction for account if required . |
	-------------------------------------------*/
	strcpy (glwkRec.tran_type, (INVOICE) ? " 7" : " 8");
	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.est_no,branchNumbr);
	strcpy (glwkRec.chq_inv_no, invoiceNumber);
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.acc_no, suwk_rec.gl_acc_no);
	strcpy (glwkRec.narrative,suwk_rec.narrative);
	strcpy (glwkRec.alt_desc1,suwk_rec.invoiceNumber);
	strcpy (glwkRec.alt_desc2,suwk_rec.cus_po_no);
	strcpy (glwkRec.alt_desc3," ");
	strcpy (glwkRec.batch_no, " ");

	if (envCrGlRef [0] == 'A')
		strcpy (glwkRec.acronym,sumr_rec.acronym);
	else
		sprintf (glwkRec.acronym,"%-9.6s",supplierNumber);
	sprintf (glwkRec.name,"%-30.30s",sumr_rec.crd_name);
	strcpy (glwkRec.user_ref,suwk_rec.user_ref);
	strcpy (glwkRec.stat_flag,"2");
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	glwkRec.tran_date = suwk_rec.gl_date;
	glwkRec.post_date = TodaysDate ();

	DateToDMY (glwkRec.tran_date, NULL, &month, NULL);
	sprintf (glwkRec.period_no, "%02d", month);

	if (glwkRec.post_date < 0)
		glwkRec.post_date = 0;

	GL_AddBatch ();
}
