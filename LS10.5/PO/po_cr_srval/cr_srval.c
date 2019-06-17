/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_srval.c,v 5.2 2001/08/09 09:15:25 scott Exp $
|  Program Name  : (po_cr_srval.c)                                |
|  Program Desc  : (Serial Item Revaluation from Creditors )   |
|            (Invoices paid.                         )   |
|---------------------------------------------------------------------|
|  Date Written  : 19/02/90        |  Author     : Fui Choo Yap.      |
|---------------------------------------------------------------------|
| $Log: cr_srval.c,v $
| Revision 5.2  2001/08/09 09:15:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:48  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_srval.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_cr_srval/cr_srval.c,v 5.2 2001/08/09 09:15:25 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>

#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct insfRecord	insf_rec;
struct sumrRecord	sumr_rec;
struct suinRecord	suin_rec;

/*==========
 Table names
===========*/
static char *data	= "data";

/*======
 Globals
========*/
	FILE	*fout;

	long	ex_hash = 0L;
	char	ex_acc [MAXLEVEL + 1];
	long	stk_hash = 0L;
	char	stk_acc [MAXLEVEL + 1];
	long	cos_hash = 0L;
	char	cos_acc [MAXLEVEL + 1];

	int		envDbCo 		= 0,
			envCrCo 		= 0,
			printerNo 		= 1,
			firstTime 		= 1;

	char	branchNo [3];

	double	gr_tot_act1 	= 0.00,
			gr_tot_act2 	= 0.00,
			gr_tot_act3 	= 0.00,
			gr_tot_var 		= 0.00,
			total_act1  	= 0.00,
			total_act2  	= 0.00,
			total_act3  	= 0.00,
			total_var  		= 0.00;

	char	loc_curr [4];


/*=======================
| Function declarations |
=======================*/
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	Process 			 (void);
void 	GetInvoices 		 (long);
void 	GeneralLedgerSold 	 (double);
void 	GlProcess 			 (double);
void 	PrintSupplierTotal 	 (void);
void 	PrintGrandTotal 	 (void);
void 	OpenAudit 			 (void);
void 	CloseAudit 			 (void);
void 	PrintLine 			 (void);
int 	ProcessAudit 		 (double, double, double);
int 	GetAccounts 		 (long, char *);
int 	FindInsf 			 (long, char *);
int 	UpdateInsf 			 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2)
	{
		/*-------------------
		| Usage : %s <lpno> |
		-------------------*/
		print_at (0,0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();

	printerNo = atoi (argv [1]);

	envDbCo = atoi (get_env ("CR_CO"));
	envCrCo = atoi (get_env ("CR_FIND"));

	/*----------------------- 
	| Open database files . |
	-----------------------*/
	OpenDB ();

	strcpy (branchNo, (envDbCo == 0) ? " 0" : comm_rec.est_no);

	dsp_screen (" Revalue Serial Items ",comm_rec.co_no,comm_rec.co_name);

	Process ();

	/*========================
	| Program exit sequence. |
	========================*/
	CloseDB (); 
	FinishProgram ();
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

	open_rec (inmr,inmr_list,INMR_NO_FIELDS,"inmr_hhbr_hash");
	open_rec (incc,incc_list,INCC_NO_FIELDS,"incc_hhwh_hash");
	open_rec (insf,insf_list,INSF_NO_FIELDS,"insf_sup_id");
	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrCo) ? "sumr_id_no" 
						          : "sumr_id_no3");

	open_rec (suin,suin_list,SUIN_NO_FIELDS,"suin_id_no2");
	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_hhcc_hash");

	OpenGlmr ();
	OpenGlwk ();
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
	abc_fclose (insf);
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (ccmr);
	GL_Close ();

	abc_dbclose (data);
}

void
Process (
 void)
{
	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.est_no,branchNo);
	strcpy (sumr_rec.crd_no,"      ");
	cc = find_rec (sumr,&sumr_rec,GTEQ,"r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sumr_rec.est_no,branchNo))
	{
		GetInvoices (sumr_rec.hhsu_hash);

		cc = find_rec (sumr,&sumr_rec,NEXT,"r");
	}
	if (!firstTime)
	{
		PrintGrandTotal ();
		CloseAudit ();
	}
}

void
GetInvoices (
 long hhsu_hash)
{
	int	found = 0;

	suin_rec.hhsu_hash = hhsu_hash;
	sprintf (suin_rec.inv_no,"%-15.15s"," ");
	cc = find_rec (suin,&suin_rec,GTEQ,"r");
	while (!cc && suin_rec.hhsu_hash == hhsu_hash)
	{
		/*-------------------------------------------------------------
		| Check is invoice Approved, Invoice been paid, type invoice. |
		-------------------------------------------------------------*/
		if (suin_rec.approved [0] == 'N' ||
			suin_rec.amt_paid == 0.00 ||
			suin_rec.type [0] != '1')
		{
			cc = find_rec (suin,&suin_rec,NEXT,"r");
			continue;
		}

		found = FindInsf (hhsu_hash,suin_rec.inv_no);

		cc = find_rec (suin,&suin_rec,NEXT,"r");
	}
	if (found)
		PrintSupplierTotal ();
}

int
FindInsf (
 long hhsu_hash, 
 char *inv_no)
{
	int	found_ser = FALSE;

	insf_rec.hhsu_hash = hhsu_hash;
	sprintf (insf_rec.crd_invoice,"%-15.15s",inv_no);
	cc = find_rec (insf,&insf_rec,GTEQ,"u");

	while (!cc && insf_rec.hhsu_hash == hhsu_hash && 
		      !strncmp (insf_rec.crd_invoice,inv_no,15))
	{
		if (insf_rec.final_costing [0] == 'N')
			found_ser = UpdateInsf ();
		
		abc_unlock (insf);
		cc = find_rec (insf,&insf_rec,NEXT,"u");
	}
	abc_unlock (insf);
	return (found_ser);
}

int
UpdateInsf (
 void)
{
	double	delta = 1.00;
	double	est_cost = 0.00;
	double	act_cost = 0.00;
	double	new_cost = 0.00;

	cc = find_hash (incc,&incc_rec,COMPARISON,"r",insf_rec.hhwh_hash);
	if (!cc)
		cc = find_hash (inmr,&inmr_rec,COMPARISON,"r",incc_rec.hhbr_hash);
	if (!cc)
	{
		sprintf (err_str,"%-9.9s/%-16.16s",sumr_rec.acronym, suin_rec.inv_no);

		dsp_process ("Supplier/Inv No:",err_str);

		if (insf_rec.exch_rate == suin_rec.exch_rate)
			return (EXIT_SUCCESS);

		if (insf_rec.exch_rate != 0.00)
			delta = insf_rec.exch_rate / suin_rec.exch_rate;
		
		est_cost = twodec (insf_rec.est_cost);
		act_cost = twodec (insf_rec.act_cost);
		new_cost = twodec (delta * insf_rec.act_cost);

		if (ProcessAudit (est_cost, act_cost, new_cost))
			return (EXIT_SUCCESS);

		if (insf_rec.status [0] != 'S')
		{
			insf_rec.exch_var = new_cost - act_cost;
			insf_rec.act_cost = new_cost;
			strcpy (insf_rec.final_costing,"Y");
	
			cc = abc_update (insf,&insf_rec);
			if (cc) 
				file_err (cc, insf, "DBFIND");
		}

	}
	abc_unlock (insf);

	return ( (cc) ? 0 : 1);
}

void
GeneralLedgerSold (
 double gl_amt)
{
	int		monthPeriod;
	int	debit = TRUE;

	if (gl_amt < 0.00)
	{
		gl_amt *= -1;
		debit = FALSE;
	}

	/*--------------------------------
	| Add transactions to glwk file. |
	--------------------------------*/
	strcpy (glwkRec.co_no,ccmr_rec.co_no);
	strcpy (glwkRec.tran_type,"10");
	glwkRec.post_date = TodaysDate ();
	glwkRec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no,"%02d", monthPeriod);

	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	strcpy (glwkRec.user_ref,"Serial Exch Adj");
	strcpy (glwkRec.stat_flag,"2");
	sprintf (glwkRec.narrative,"INV :%-15.15s", suin_rec.inv_no);
	strcpy (glwkRec.alt_desc1, suin_rec.narrative);
	strcpy (glwkRec.alt_desc2, suin_rec.cus_po_no);
	strcpy (glwkRec.alt_desc3, suin_rec.destin);
	strcpy (glwkRec.batch_no,  " ");
	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, ex_acc);
	glwkRec.hhgl_hash = ex_hash;
	glwkRec.amount 		= CENTS (gl_amt);
	glwkRec.loc_amount 	= CENTS (gl_amt);
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	strcpy (glwkRec.jnl_type, (debit) ? "1" : "2");
	cc = abc_add (glwk,&glwkRec);
	if (cc) 
		file_err (cc, "glwk", "DBADD");

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,cos_acc);
	glwkRec.hhgl_hash = cos_hash;
	strcpy (glwkRec.jnl_type, (debit) ? "2" : "1");
	cc = abc_add (glwk,&glwkRec);
	if (cc)
		file_err (cc, "glwk", "DBADD");
}

void
GlProcess (
 double gl_amt)
{
	int		monthPeriod;
	int	debit = TRUE;

	if (gl_amt < 0.00)
	{
		gl_amt *= -1;
		debit = FALSE;
	}

	/*--------------------------------
	| Add transactions to glwk file. |
	--------------------------------*/
	strcpy (glwkRec.co_no,ccmr_rec.co_no);
	strcpy (glwkRec.tran_type,"10");
	glwkRec.post_date = TodaysDate ();
	glwkRec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no,"%02d", monthPeriod);
	
	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	strcpy (glwkRec.user_ref,"Serial Exch Adj");
	strcpy (glwkRec.stat_flag,"2");
	sprintf (glwkRec.narrative,"INV :%-15.15s", suin_rec.inv_no);
	strcpy (glwkRec.alt_desc1, suin_rec.narrative);
	strcpy (glwkRec.alt_desc2, suin_rec.cus_po_no);
	strcpy (glwkRec.alt_desc3, suin_rec.destin);
	strcpy (glwkRec.batch_no,  " ");
	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, ex_acc);
	glwkRec.hhgl_hash 	= ex_hash;
	glwkRec.amount 		= CENTS (gl_amt);
	glwkRec.loc_amount 	= CENTS (gl_amt);
	glwkRec.exch_rate 	= 1.00;
	strcpy (glwkRec.currency, loc_curr);
	strcpy (glwkRec.jnl_type, (debit) ? "1" : "2");
	cc = abc_add (glwk,&glwkRec);
	if (cc) 
		file_err (cc, "glwk", "DBFIND");

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL, stk_acc);
	glwkRec.hhgl_hash = stk_hash;
	strcpy (glwkRec.jnl_type, (debit) ? "2" : "1");
	cc = abc_add (glwk,&glwkRec);
	if (cc)
		file_err (cc, "glwk", "DBFIND");
}

int
ProcessAudit (
 double est_cost, 
 double act_cost, 
 double new_cost)
{
	double	variance = 0.00;

	if (GetAccounts (incc_rec.hhcc_hash, inmr_rec.category))
		return (EXIT_FAILURE);
		
	/*--------------------------
	| Start audit pipe output. |
	--------------------------*/
	if (firstTime)
	{
		OpenAudit ();
		firstTime = 0;
	}

	variance = act_cost - new_cost;

	fprintf (fout,"!%-6.6s",sumr_rec.crd_no);
	fprintf (fout,"!%-4.4s",suin_rec.currency);
	fprintf (fout,"!%-15.15s",suin_rec.inv_no);
	fprintf (fout,"!%-16.16s",inmr_rec.item_no);
	fprintf (fout,"!%-25.25s",insf_rec.serial_no);
	fprintf (fout,"!%12.2f",est_cost);
	fprintf (fout,"!%12.2f",act_cost);
	fprintf (fout,"!%12.2f",new_cost);
	fprintf (fout,"!%12.2f",variance);
	if (insf_rec.status [0] != 'S')
	{
		if (variance > 0.00)
		{
			fprintf (fout, "!%-16.16s", ex_acc);
			fprintf (fout, "!%-16.16s!\n",stk_acc);
		}
		else
		{
			fprintf (fout, "!%-16.16s",stk_acc);
			fprintf (fout, "!%-16.16s!\n", ex_acc);
		}
	}
	else
	{
		if (variance > 0.00)
		{
			fprintf (fout, "!%-16.16s", ex_acc);
			fprintf (fout, "!%-16.16s!\n",cos_acc);
		}
		else
		{
			fprintf (fout, "!%-16.16s",cos_acc);
			fprintf (fout, "!%-16.16s!\n", ex_acc);
		}
	}

	fflush (fout);

	if (insf_rec.status [0] != 'S')
		GlProcess (variance);
	else
		GeneralLedgerSold (variance);

	total_act1 += est_cost;
	total_act2 += act_cost;
	total_act3 += new_cost;
	total_var += variance;
	return (EXIT_SUCCESS);
}

/*---------------------------
| Print total for supplier. |
---------------------------*/
void
PrintSupplierTotal (
 void)
{
	PrintLine ();

	fprintf (fout,"!      ");
	fprintf (fout,"!    ");
	fprintf (fout,"! **** TOTAL FOR");
	fprintf (fout,"! SUPPLIER ****  ");
	fprintf (fout,"!                         ");
	fprintf (fout,"!%12.2f",total_act1);
	fprintf (fout,"!%12.2f",total_act2);
	fprintf (fout,"!%12.2f",total_act3);
	fprintf (fout,"!%12.2f",total_var);
	fprintf (fout,"!                ");
	fprintf (fout,"!                !\n");

	PrintLine ();
	fflush (fout);
	
	gr_tot_act1 += total_act1;
	gr_tot_act2 += total_act2;
	gr_tot_act3 += total_act3;
	gr_tot_var += total_var;

	total_act1 = 0.00;
	total_act2 = 0.00;
	total_act3 = 0.00;
	total_var = 0.00;
}

void
PrintGrandTotal (
 void)
{
	fprintf (fout,"!      ");
	fprintf (fout,"!    ");
	fprintf (fout,"!**GRAND TOTAL**");
	fprintf (fout,"!                ");
	fprintf (fout,"!                         ");
	fprintf (fout,"!%12.2f",gr_tot_act1);
	fprintf (fout,"!%12.2f",gr_tot_act2);
	fprintf (fout,"!%12.2f",gr_tot_act3);
	fprintf (fout,"!%12.2f",gr_tot_var);
	fprintf (fout,"!                ");
	fprintf (fout,"!                !\n");

	fflush (fout);
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ( (fout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)",cc,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",printerNo);
	fprintf (fout,".12\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".E%s/ %s\n",clip (comm_rec.est_name),clip (comm_rec.cc_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EREVALUE SERIAL ITEMS AUDIT REPORT.\n");

	fprintf (fout,".Eas at %-24.24s\n",SystemTime ());

	fprintf (fout,".R=======");
	fprintf (fout,"=====");
	fprintf (fout,"================");
	fprintf (fout,"=================");
	fprintf (fout,"==========================");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,"=======");
	fprintf (fout,"=====");
	fprintf (fout,"================");
	fprintf (fout,"=================");
	fprintf (fout,"==========================");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,"!SUPP. ");
	fprintf (fout,"!CURR");
	fprintf (fout,"!INVOICE  NUMBER");
	fprintf (fout,"!  ITEM NUMBER   ");
	fprintf (fout,"!   S E R I A L   N O     ");
	fprintf (fout,"! RECEIPTED  ");
	fprintf (fout,"!PRIOR FINAL ");
	fprintf (fout,"! NEW  FINAL ");
	fprintf (fout,"!  VARIANCE  ");
	fprintf (fout,"!     DEBIT      ");
	fprintf (fout,"!     CREDIT     !\n");

	fprintf (fout,"!NUMBER");
	fprintf (fout,"!CODE");
	fprintf (fout,"!               ");
	fprintf (fout,"!                ");
	fprintf (fout,"!                         ");
	fprintf (fout,"!    COST    ");
	fprintf (fout,"!    COST    ");
	fprintf (fout,"!    COST    ");
	fprintf (fout,"!            ");
	fprintf (fout,"!    ACCOUNT     ");
	fprintf (fout,"!    ACCOUNT     !\n");

	fprintf (fout,"!------");
	fprintf (fout,"!----");
	fprintf (fout,"!---------------");
	fprintf (fout,"!----------------");
	fprintf (fout,"!-------------------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!---------------");
	fprintf (fout,"!----------------!\n");
	
	fflush (fout);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout,".EOF\n");
	pclose (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"!------");
	fprintf (fout,"!----");
	fprintf (fout,"!---------------");
	fprintf (fout,"!----------------");
	fprintf (fout,"!-------------------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!----------------");
	fprintf (fout,"!----------------!\n");
	
	fflush (fout);
}

int
GetAccounts (
	long	hhccHash, 
	char 	*catNo)
{
	ccmr_rec.hhcc_hash = hhccHash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"EXCH VAR. ",
		" ",
		catNo
	);
	ex_hash = glmrRec.hhmr_hash;
	strcpy (ex_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE D",
		" ",
		catNo
	);
	stk_hash = glmrRec.hhmr_hash;
	strcpy (stk_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"PURCHASE D",
		" ",
		catNo
	);
	cos_hash = glmrRec.hhmr_hash;
	strcpy (cos_acc, glmrRec.acc_no);
    return (EXIT_SUCCESS);
}
