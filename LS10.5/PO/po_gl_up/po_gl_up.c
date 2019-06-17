/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_gl_up.c,v 5.4 2001/10/09 22:57:21 scott Exp $
|  Program Name  : (po_gl_up.c (                                  |
|  Program Desc  : (Purchase Order to General Ledger Update.  (   |
|                 (                                         (   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: po_gl_up.c,v $
| Revision 5.4  2001/10/09 22:57:21  scott
| Updated for purchase returns
|
| Revision 5.3  2001/10/08 07:14:49  cha
| Updated to change glwk_sys_ref to the correct length.
|
| Revision 5.2  2001/08/09 09:15:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:52  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_gl_up.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_gl_up/po_gl_up.c,v 5.4 2001/10/09 22:57:21 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct powkRecord	powk_rec;
struct powkRecord	powk2_rec;

	char	*powk2	=	"powk2";

	char	localCurrency [4];

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB  		(void);
void 	CloseDB  		(void);
void 	ProcessFile  	(void);
void 	ProcessDebit  	(void);
void 	ProcessCredit  	(void);
void 	WriteGlwk  		(double, char *, long, char *, char *, int);
void 	DeletePowk  	(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	set_tty ();
	init_scr ();

	OpenDB ();

	print_mess (ML ("Posting Purchase Orders to General ledger."));

	ProcessFile ();
	DeletePowk ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	abc_alias (powk2,powk);
	open_rec (powk2, powk_list, POWK_NO_FIELDS, "powk_id_no");
	open_rec (powk,  powk_list, POWK_NO_FIELDS, "powk_id_no");

	OpenGlmr ();
	OpenGlwk ();
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (powk);
	abc_fclose (powk2);
	GL_Close ();
	abc_dbclose ("data");
}

/*============================================================
| Process powk file Using company/Branch/sort.               |
============================================================*/
void
ProcessFile (
 void)
{
	char	thisSort [sizeof powk2_rec.sort];

	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	strcpy (powk2_rec.co_no,comm_rec.co_no);
	strcpy (powk2_rec.br_no,comm_rec.est_no);
	strcpy (powk2_rec.sort,"               ");

	cc = find_rec (powk2,&powk2_rec,GTEQ,"r");
	while (!cc && !strcmp (powk2_rec.co_no,comm_rec.co_no) && 
			      !strcmp (powk2_rec.br_no,comm_rec.est_no))
	{
		strcpy (thisSort, powk2_rec.sort);
		dsp_process ("PO# ",powk2_rec.sort);
		abc_selfield (powk,"powk_id_no_1");
		ProcessDebit ();

		abc_selfield (powk,"powk_id_no_2");
		ProcessCredit ();

		while (!cc && !strcmp (powk2_rec.sort, thisSort))
			cc = find_rec (powk2,&powk2_rec, NEXT,"r");
	}
}

/*=========================
| Process debit Accounts. |
=========================*/
void
ProcessDebit (
 void)
{
	char	lastAccount [MAXLEVEL + 1];
	char	lastNarrative [sizeof glwkRec.narrative];
	char	userReference [sizeof glwkRec.user_ref];
	long	lastHash = 0L;
	double	amt_tran = 0.00;

	strcpy (powk_rec.co_no,powk2_rec.co_no);
	strcpy (powk_rec.br_no,comm_rec.est_no);
	strcpy (powk_rec.sort,powk2_rec.sort);
	sprintf (powk_rec.dbt_acc,"%*.*s", MAXLEVEL,MAXLEVEL," ");
	cc = find_rec (powk,&powk_rec,GTEQ,"r");

	strcpy (lastAccount,  powk_rec.dbt_acc);
	strcpy (lastNarrative,powk_rec.narrative);
	strcpy (userReference,powk_rec.user_ref);
	lastHash = powk_rec.dbt_hash;
	while (!cc && !strcmp (powk_rec.sort,powk2_rec.sort) && 
			     !strcmp (powk_rec.co_no,comm_rec.co_no) && 
			     !strcmp (powk_rec.br_no,comm_rec.est_no))
	{
		dsp_process ("Debit ",powk_rec.dbt_acc);
		if (strcmp (lastAccount,powk_rec.dbt_acc))
		{
			if (amt_tran >= 0.00)
			{
				WriteGlwk 
				(
					amt_tran,
					lastAccount, 
					lastHash,
					lastNarrative,
					userReference,
					TRUE
				);
			}
			else
			{
				amt_tran *= -1;
				WriteGlwk 
				(
					amt_tran,
					lastAccount,
					lastHash,	
					lastNarrative,
					userReference,
					FALSE
				);
			}
			amt_tran = 0.00;
			strcpy (lastAccount, powk_rec.dbt_acc);
		}
		strcpy (lastNarrative,powk_rec.narrative);
		strcpy (userReference,powk_rec.user_ref);
		lastHash = powk_rec.dbt_hash;
		amt_tran += powk_rec.amount;
		cc = find_rec (powk,&powk_rec,NEXT,"r");
	}
	if (amt_tran >= 0.00)
	{
		WriteGlwk 
		(
			amt_tran,
			lastAccount,
			lastHash,
			lastNarrative,
			userReference,
			TRUE
		);
	}
	else
	{
		amt_tran *= -1;
		WriteGlwk 
		(
			amt_tran,
			lastAccount,
			lastHash,
			lastNarrative,
			userReference,
			FALSE
		);
	}
}
/*==========================
| Process credit Accounts. |
==========================*/
void
ProcessCredit (
 void)
{
	char	lastAccount 	[sizeof glwkRec.acc_no];
	char	lastNarrative 	[sizeof glwkRec.narrative];
	char	userReference 	[sizeof glwkRec.user_ref];
	long	lastHash = 0L;
	double	amt_tran = 0.00;

	strcpy (powk_rec.co_no,comm_rec.co_no);
	strcpy (powk_rec.br_no,comm_rec.est_no);
	strcpy (powk_rec.sort,powk2_rec.sort);
	sprintf (powk_rec.crd_acc,"%*.*s", MAXLEVEL,MAXLEVEL," ");
	cc = find_rec (powk,&powk_rec,GTEQ,"r");

	strcpy (lastAccount,	powk_rec.crd_acc);
	strcpy (lastNarrative,	powk_rec.narrative);
	strcpy (userReference,	powk_rec.user_ref);
	while (!cc && !strcmp (powk_rec.sort,powk2_rec.sort) && 
				  !strcmp (powk_rec.co_no,comm_rec.co_no) && 
				  !strcmp (powk_rec.br_no,comm_rec.est_no))
	{
		dsp_process ("Credit ",powk_rec.crd_acc);
		if (strcmp (lastAccount,powk_rec.crd_acc))
		{
			if (amt_tran >= 0.00)
			{
				WriteGlwk 
				(
					amt_tran,
					lastAccount, 
					lastHash,
					lastNarrative, 
					userReference,
					FALSE
				);
			}
			else
			{
				amt_tran *= -1;
				WriteGlwk 
				(
					amt_tran,
					lastAccount, 
					lastHash,
					lastNarrative, 
					userReference,
					TRUE
				);
			}

			amt_tran = 0.00;
			strcpy (lastAccount, powk_rec.crd_acc);
		}
		strcpy (lastNarrative,powk_rec.narrative);
		strcpy (userReference,powk_rec.user_ref);
		lastHash = powk_rec.crd_hash;
		amt_tran += powk_rec.amount;
		cc = find_rec (powk,&powk_rec,NEXT,"r");
	}
	if (amt_tran >= 0.00)
	{
		WriteGlwk 
		(
			amt_tran,
			lastAccount,
			lastHash,
			lastNarrative,
			userReference,
			FALSE
		);
	}
	else
	{
		amt_tran *= -1;
		WriteGlwk 
		(
			amt_tran,
			lastAccount,
			lastHash,
			lastNarrative,
			userReference,
			TRUE
		);
	}
}

/*===============================
| Write General ledger Records. |
===============================*/
void
WriteGlwk (
	double	wk_amt,
	char	*acc,
	long	hash,
	char	*narr,
	char	*userReference,
	int		debit)
{
	int		monthPeriod;

	sprintf (glwkRec.acc_no,"%-*.*s",MAXLEVEL,MAXLEVEL, acc);
	strcpy (glwkRec.co_no,comm_rec.co_no);
	glwkRec.hhgl_hash = hash;
	strcpy (glwkRec.tran_type,"11");
	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	glwkRec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);
	glwkRec.post_date = TodaysDate ();

	sprintf (glwkRec.narrative,"%-20.20s",narr);
	sprintf (glwkRec.alt_desc1,"%20.20s"," ");
	sprintf (glwkRec.alt_desc2,"%20.20s"," ");
	sprintf (glwkRec.alt_desc3,"%20.20s"," ");
	sprintf (glwkRec.batch_no, "%10.10s"," ");
	sprintf (glwkRec.user_ref, "%15.15s",userReference);
	glwkRec.amount 		= CENTS (wk_amt);
	glwkRec.loc_amount 	= CENTS (wk_amt);
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, localCurrency);

	strcpy (glwkRec.jnl_type, (debit) ? "1" : "2");

	strcpy (glwkRec.stat_flag,"2");

	cc = abc_add (glwk,&glwkRec);
	if (cc) 
		file_err (cc, glwk, "DBADD");
}

/*================================
| Delete Work file transactions. |
================================*/
void
DeletePowk (
 void)
{
	strcpy (powk2_rec.co_no,comm_rec.co_no);
	strcpy (powk2_rec.br_no,comm_rec.est_no);
	strcpy (powk2_rec.sort,"               ");
	cc = find_rec (powk2,&powk2_rec,GTEQ,"u");
	while (!cc && !strcmp (powk2_rec.co_no,comm_rec.co_no) && 
				  !strcmp (powk2_rec.br_no,comm_rec.est_no))
	{
		dsp_process ("Delete ",powk2_rec.sort);
		cc = abc_delete (powk2);
		if (cc) 
			file_err (cc, powk, "DBDELETE");

		cc = find_rec (powk2,&powk2_rec,GTEQ,"u");
	}
	abc_unlock (powk2);
}
