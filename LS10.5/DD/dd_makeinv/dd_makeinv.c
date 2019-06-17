/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dd_makeinv.c,v 5.4 2001/11/06 03:04:27 scott Exp $
|  Program Name  : (dd_makeinv.c)
|  Program Desc  : (Creates Invoices for despatched DD shipments)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 29/06/94         |
|---------------------------------------------------------------------|
| $Log: dd_makeinv.c,v $
| Revision 5.4  2001/11/06 03:04:27  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dd_makeinv.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_makeinv/dd_makeinv.c,v 5.4 2001/11/06 03:04:27 scott Exp $";

#include <fcntl.h>
#include <sys/stat.h>
#include <pslscr.h>	
#include <twodec.h>	/*  Charater type macros			*/
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_dd_mess.h>

#define	PENDINGFLAG		"P"
#define	ACTIVEFLAG		"A"
#define	CONFIRMFLAG		"C"
#define	DELETEFLAG		"X"
#define	DESPATCHFLAG	"D"
#define	INVOICEFLAG		"I"
#define		BY_BRANCH	1
#define		BY_DEPART	2
#define		CASH		 (cumr_rec.cash_flag [0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct ddhrRecord	ddhr_rec;
struct ddhrRecord	ddhr2_rec;
struct ddlnRecord	ddln_rec;
struct ddshRecord	ddsh_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct inmrRecord	inmr_rec;
struct posoRecord	poso_rec;
struct fehrRecord	fehr_rec;
struct felnRecord	feln_rec;
struct cumrRecord	cumr_rec;
struct cudpRecord	cudp_rec;

FILE		*fout;

int			printerNo;
int			notax;
int			FE_INSTALL;

double		l_total;
double		l_dis;
double		l_tax;
double		l_gst;

char		*currentUser;
char		createFlag [2];

char		*data 	= "data",
			*ddsh2 	= "ddsh2",
			*cohr2 	= "cohr2";

int		SO_NUMBERS =	BY_BRANCH;

/*=====================================================================
| Local Funtion Prototypes
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	GetInvNo 		(char *);
void 	AddCohr 		(void);
void 	AddColn 		(int);
void 	AddPoso 		(void);
void 	AddFeln 		(void);
void 	ProcessDdhr 	(void);
void 	CalcExtend 		(void);
int 	CheckCohr 		(char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc, 
 char*  argv [])
{
	char	*sptr;
		
	if (argc < 2)
	{
		/*-------------------------------------------
		| Usage : %s <createFlag> - optional <printerNo> |
		-------------------------------------------*/
		print_at (0,0,ML (mlDdMess700),argv [0]);
        return (argc);		
	}

	sprintf (createFlag,"%-1.1s",argv [1]);
	
	printerNo = (argc == 3) ? atoi (argv [2]) : 0;

	currentUser = getenv ("LOGNAME");

	/*---------------------------
	| Forward Exchange Enabled? |
	---------------------------*/
	sptr = chk_env ("FE_INSTALL");
	FE_INSTALL = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SO_NUMBERS");
	SO_NUMBERS = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	sprintf (err_str,"Invoicing Despatched Direct Delivery Shipments"); 
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	strcpy (ddhr_rec.co_no,	comm_rec.co_no);
	strcpy (ddhr_rec.br_no, comm_rec.est_no);
	strcpy (ddhr_rec.order_no, "        ");
	cc = find_rec (ddhr, &ddhr_rec, GTEQ, "u");

	while (!cc && 
		   !strcmp (ddhr_rec.co_no,    comm_rec.co_no) && 
		   !strcmp (ddhr_rec.br_no,    comm_rec.est_no))
	{
		if (ddhr_rec.progressive [0] == 'N' &&
		    strcmp (ddhr_rec.stat_flag, DESPATCHFLAG))
		{
			abc_unlock (ddhr);
			cc = find_rec (ddhr, &ddhr_rec, NEXT, "u");
			continue;
		}
		abc_unlock (ddhr);

		ProcessDdhr ();

		cc = find_rec (ddhr, &ddhr_rec, NEXT, "u");
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (ddsh2, ddsh);
	abc_alias (cohr2, cohr);

	open_rec (cohr2, cohr_list,COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_id_no2");
	open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_hhds_hash");
	open_rec (ddsh, ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (poso, poso_list, POSO_NO_FIELDS, "poso_id_no3");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	open_rec (ddsh2,ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no3");

	if (FE_INSTALL)
	{
		open_rec (fehr, fehr_list, FEHR_NO_FIELDS, "fehr_id_no");
		open_rec (feln, feln_list, FELN_NO_FIELDS, "feln_id_no");
	}
}



void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inmr);
	abc_fclose (poso);
	abc_fclose (cudp);
	abc_fclose (esmr);
	abc_fclose (cohr2);

	if (FE_INSTALL)
	{
		abc_fclose (fehr);
		abc_fclose (feln);
	}

	abc_dbclose (data);
}

void
GetInvNo (
 char*  invNum)
{
	char	tmp_prefix 	 [3]; 
	char	tmp_inv_no [9];
	char	tmp_mask [12];
	int		len = 8;
	long	inv_no;

	/*------------------------------------------------------
	| Is invoice number to come from department of branch. |
	------------------------------------------------------*/
	if (SO_NUMBERS == BY_DEPART)
	{
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, cohr_rec.dp_no);
		cc = find_rec (cudp, &cudp_rec, COMPARISON, "u");
		if (cc)
		file_err (cc, cudp, "DBFIND");

		inv_no	=	 (CASH) ? cudp_rec.nx_csh_no : cudp_rec.nx_chg_no;
		inv_no++;
	}
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");
	
		inv_no	=	 (CASH) ? esmr_rec.nx_csh_inv : esmr_rec.nx_inv_no;
		inv_no++;
	}

	if (SO_NUMBERS == BY_BRANCH)
	{
		if (CASH)
			strcpy (tmp_prefix, esmr_rec.csh_pref);
		else
			strcpy (tmp_prefix, esmr_rec.chg_pref);
	}
	else
	{
		if (CASH)
			strcpy (tmp_prefix, cudp_rec.csh_pref);
		else
			strcpy (tmp_prefix, cudp_rec.chg_pref);
	}

	clip (tmp_prefix);
	len = strlen (tmp_prefix);

	sprintf (tmp_mask, "%%s%%0%dld", 8 - len);
	sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no);

	/*-----------------------------------------------
	| Check if Invoice / Credit Note No Already	|
	| Allocated. If it has been then skip		|
	-----------------------------------------------*/
	while (CheckCohr (tmp_inv_no) == 0)
		sprintf (tmp_inv_no, tmp_mask, tmp_prefix, inv_no++);


	if (SO_NUMBERS == BY_DEPART)
	{
		if (CASH)
			cudp_rec.nx_csh_no	=	inv_no;
		else
			cudp_rec.nx_chg_no	=	inv_no;

		cc = abc_update (cudp, &cudp_rec);
		if (cc)
			file_err (cc, cudp, "DBUPDATE");
	}
	else
	{
		if (CASH)
			esmr_rec.nx_csh_inv	=	inv_no;
		else
			esmr_rec.nx_inv_no	=	inv_no;

		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");
	}
	sprintf (invNum, "%-8.8s", tmp_inv_no);
}

void
AddCohr (void)
{
	long		l_systemDate;

	l_systemDate = TodaysDate ();
	
	strcpy (cohr_rec.co_no, ddhr_rec.co_no);
	strcpy (cohr_rec.br_no, ddhr_rec.br_no);
	strcpy (cohr_rec.dp_no, ddhr_rec.dp_no);
	cohr_rec.hhcu_hash = ddhr_rec.hhcu_hash;

	/*---------------------------------------------------------------------
	| BFS : Check if debtor is cash or not (that is class type is XXX) |
	---------------------------------------------------------------------*/
	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	if ((cc = find_rec (cumr, &cumr_rec, EQUAL, "r")))
		file_err (cc, cumr, "DBFIND");

	if (atol (ddhr_rec.inv_no + 1) == 0L)
		GetInvNo (cohr_rec.inv_no);
	else
		sprintf (cohr_rec.inv_no, "%-8.8s", ddhr_rec.inv_no);

	strcpy (cohr_rec.type, "I");
	strcpy (cohr_rec.cont_no, ddhr_rec.cont_no);
	strcpy (cohr_rec.drop_ship, "Y");
	if (ddhr_rec.progressive [0] == 'N')
		cohr_rec.hhds_hash = 0L;
	else
		cohr_rec.hhds_hash = ddsh_rec.hhds_hash;
	strcpy (cohr_rec.cus_ord_ref, ddhr_rec.cus_ord_ref);
	strcpy (cohr_rec.cons_no, "                ");
	strcpy (cohr_rec.carr_code,"    ");
	strcpy (cohr_rec.carr_area, ddhr_rec.carr_area);
	cohr_rec.no_cartons 	= ddhr_rec.no_ctn;
	cohr_rec.wgt_per_ctn	=	ddhr_rec.wgt_per_ctn;
	cohr_rec.no_kgs 		= 0.00;
	sprintf (cohr_rec.op_id, "%-14.14s", currentUser);
	strcpy (cohr_rec.frei_req, "N");
	if (l_systemDate > comm_rec.inv_date)
		cohr_rec.date_raised = comm_rec.inv_date;
	else
		cohr_rec.date_raised = l_systemDate;
	cohr_rec.date_required = ddhr_rec.dt_required;
	strcpy (cohr_rec.tax_code, ddhr_rec.tax_code);
	strcpy (cohr_rec.tax_no, ddhr_rec.tax_no);
	strcpy (cohr_rec.area_code, ddhr_rec.area_code);
	strcpy (cohr_rec.sale_code, ddhr_rec.sman_code);
	cohr_rec.date_create = l_systemDate;
	strcpy (cohr_rec.time_create, TimeHHMM ());
	cohr_rec.insurance 	= 	ddhr_rec.insurance;
	cohr_rec.freight 	= 	ddhr_rec.freight;
	cohr_rec.gross 		= 0.00;
	cohr_rec.tax 		= 0.00;
	cohr_rec.gst 		= 0.00;
	cohr_rec.disc 		= 0.00;
	cohr_rec.deposit 	= 0.00;
	cohr_rec.ex_disc 	= 0.00;
	strcpy (cohr_rec.fix_exch, ddhr_rec.fix_exch);
	strcpy (cohr_rec.batch_no, "     ");
	strcpy (cohr_rec.dl_name, ddhr_rec.del_name);
	cohr_rec.other_cost_1 = ddhr_rec.other_cost_1;
	cohr_rec.other_cost_2 = ddhr_rec.other_cost_2;
	cohr_rec.other_cost_3 = ddhr_rec.other_cost_3;
	strcpy (cohr_rec.dl_add1, ddhr_rec.del_add1);
	strcpy (cohr_rec.dl_add2, ddhr_rec.del_add2);
	strcpy (cohr_rec.dl_add3, ddhr_rec.del_add3);
	strcpy (cohr_rec.din_1, ddhr_rec.din_1);
	strcpy (cohr_rec.din_2, ddhr_rec.din_2);
	strcpy (cohr_rec.din_3, ddhr_rec.din_3);
	strcpy (cohr_rec.pay_terms, ddhr_rec.pay_term);
	strcpy (cohr_rec.sell_terms, ddhr_rec.sell_terms);
	sprintf (cohr_rec.ins_det, "%-30.30s", " ");
	strcpy (cohr_rec.pri_type, ddhr_rec.pri_type);
	strcpy (cohr_rec.ord_type, "E");
	strcpy (cohr_rec.prt_price, "Y");
	strcpy (cohr_rec.inv_print, "N");
	strcpy (cohr_rec.stat_flag, createFlag);
	cohr_rec.exch_rate = ddhr_rec.exch_rate;

	cc = abc_add (cohr, &cohr_rec);
	if (cc) 
		file_err (cc, cohr, "DBADD");

	cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, cohr, "DBFIND");

	if (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
		notax = 1;
	else
		notax = 0;
}


void
AddColn (
 int wsLine)
{
	coln_rec.hhco_hash 		= cohr_rec.hhco_hash;
	coln_rec.line_no 		= wsLine;
	coln_rec.hhbr_hash 		= ddln_rec.hhbr_hash;
	coln_rec.hhsl_hash 		= 0L;
	coln_rec.hhdl_hash 		= ddln_rec.hhdl_hash;
	coln_rec.cont_status 	= ddln_rec.cont_status;
	coln_rec.q_order 		= ddln_rec.q_order;
	coln_rec.q_backorder 	= 0L;
	coln_rec.sale_price 	= ddln_rec.sale_price;
	coln_rec.cost_price 	= ddln_rec.cost_price;
	coln_rec.disc_pc 		= ddln_rec.disc_pc;
	coln_rec.tax_pc 		= ddln_rec.tax_pc;
	coln_rec.gst_pc 		= ddln_rec.gst_pc;
	coln_rec.gross 			= ddln_rec.gross;
	coln_rec.on_cost 		= ddln_rec.on_cost;
	coln_rec.amt_disc 		= ddln_rec.amt_disc;
	coln_rec.amt_tax 		= ddln_rec.amt_tax;
	coln_rec.amt_gst 		= ddln_rec.amt_gst;
	coln_rec.hhah_hash 		= 0L;
	coln_rec.due_date 		= ddln_rec.due_date;
	coln_rec.incc_hash 		= ccmr_rec.hhcc_hash;

	sprintf (coln_rec.serial_no, 	"%25.25s", " ");
	strcpy (coln_rec.pack_size, 	ddln_rec.pack_size);
	strcpy (coln_rec.sman_code, 	ddln_rec.sman_code);
	strcpy (coln_rec.cus_ord_ref, 	ddhr_rec.cus_ord_ref);
	strcpy (coln_rec.item_desc, 	ddln_rec.item_desc);
	strcpy (coln_rec.bonus_flag, 	ddln_rec.bonus_flag);
	strcpy (coln_rec.hide_flag, 	" ");
	strcpy (coln_rec.status, 		"I");
	strcpy (coln_rec.stat_flag, 	createFlag);

	CalcExtend ();
	cohr_rec.gross += l_total;
	cohr_rec.disc  += l_dis;
	cohr_rec.tax   += l_tax;
	cohr_rec.gst   += l_gst;

	coln_rec.hhum_hash 		= inmr_rec.std_uom;
	cc = abc_add (coln, &coln_rec);
	if (cc) 
		file_err (cc, coln, "DBADD");

	cc = find_rec (coln, &coln_rec, EQUAL, "r");
	if (cc)
		file_err (cc, coln, "DBFIND");
}



void
AddPoso (void)
{
	poso_rec.hhcl_hash = coln_rec.hhcl_hash;
	poso_rec.hhpl_hash = ddln_rec.hhpl_hash;
	cc = find_rec (poso, &poso_rec, EQUAL, "u");
	poso_rec.qty = poso_rec.qty_ord = ddln_rec.q_order;
	if (cc)
	{
		cc = abc_add (poso, &poso_rec);
 		if (cc)
			file_err (cc, poso, "DBADD");
	}
	else
	{
		cc = abc_update (poso, &poso_rec);
		if (cc)
			file_err (cc, poso, "DBUPDATE");
	}
}

void
AddFeln (void)
{
	if (!strcmp (ddhr_rec.fwd_exch, "      "))
		return;

	strcpy (fehr_rec.co_no,   comm_rec.co_no);
	strcpy (fehr_rec.cont_no, ddhr_rec.fwd_exch);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, fehr, "DBFIND");
			
	/*----------------------------------
	| Reduce feln record for D-D Order |
	| by invoiced amount and delete if |
	| zero.                            |
	----------------------------------*/
	strcpy (feln_rec.index_by, "D");
	feln_rec.index_hash = ddhr_rec.hhdd_hash;
	cc = find_rec (feln, &feln_rec, EQUAL,"u");
	if (cc)
		file_err (cc, feln, "DBFIND");
	feln_rec.value -= cohr_rec.gross;
	if (feln_rec.value <= 0.00)
	{
		cc = abc_delete (feln);
		if (cc) 
			file_err (cc, feln, "DBDELETE");
	}
	else
	{
		cc = abc_update (feln, &feln_rec);
		if (cc) 
			file_err (cc, feln, "DBUPDATE");
	}

	/*--------------------------------
	| Add New feln record for invoice |
	---------------------------------*/
	strcpy (feln_rec.index_by, "I");
	feln_rec.hhfe_hash  = fehr_rec.hhfe_hash;
	feln_rec.index_hash = cohr_rec.hhco_hash;
	feln_rec.value = cohr_rec.gross;
	cc = abc_add (feln, &feln_rec);
	if (cc) 
		file_err (cc, feln, "DBADD");
}

void
ProcessDdhr (void)
{
	int		wsLine = 0;
	int		allInvoiced = TRUE;

	dsp_process ("Order No : ", clip (ddhr_rec.order_no));

	if (ddhr_rec.progressive [0] == 'N')
	{
		abc_selfield (ddln, "ddln_id_no");
		AddCohr ();
		wsLine = 0;
		ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		cc = find_rec (ddln, &ddln_rec, GTEQ, "u");
		while (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash)
		{
			AddColn (wsLine);
			AddPoso ();
			wsLine++;
			strcpy (ddln_rec.stat_flag, "I");
			cc = abc_update (ddln, &ddln_rec);
			if (cc) 
				file_err (cc, ddln, "DBUPDATE");

			cc = find_rec (ddln, &ddln_rec, NEXT, "u");
		}
		abc_unlock (ddln);

		cc = abc_update (cohr, &cohr_rec);
		if (cc)
			file_err (cc, cohr, "DBUPDATE");
		if (FE_INSTALL)
			AddFeln ();
	}
	else
	{
		abc_selfield (ddln, "ddln_hhds_hash");
		ddsh_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		ddsh_rec.hhds_hash = 0L;
		cc = find_rec (ddsh, &ddsh_rec, GTEQ, "u");
		while (!cc && ddsh_rec.hhdd_hash == ddhr_rec.hhdd_hash)
		{
			if (!strcmp (ddsh_rec.stat_flag, DESPATCHFLAG))
			{
				cc = find_rec (ddsh2, &ddsh_rec, EQUAL, "u");
				if (cc)
					file_err (cc, ddsh2, "DBFIND");
				AddCohr ();
				wsLine = 0;
				ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
				cc = find_rec (ddln, &ddln_rec, GTEQ, "u");
				while (!cc && ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
				{
					AddColn (wsLine);
					AddPoso ();
					wsLine++;
					strcpy (ddln_rec.stat_flag, "I");
					cc = abc_update (ddln, &ddln_rec);
					if (cc) 
						file_err (cc, ddln, "DBUPDATE");
	
					cc = find_rec (ddln, &ddln_rec, NEXT, "u");
				}
				abc_unlock (ddln);
				cc = abc_update (cohr, &cohr_rec);
				if (cc)
					file_err (cc, cohr, "DBUPDATE");

				if (FE_INSTALL)
					AddFeln ();

				strcpy (ddsh_rec.stat_flag, "I");
				cc = abc_update (ddsh2, &ddsh_rec);
				if (cc)
					file_err (cc, ddsh2, "DBUPDATE");
			}
			else
			{
				abc_unlock (ddsh);
				allInvoiced = FALSE;
			}
			cc = find_rec (ddsh, &ddsh_rec, NEXT, "u");
		}
		abc_unlock (ddsh);
	}

	if (allInvoiced)
	{
		strcpy (ddhr_rec.stat_flag, "I");
		cc = abc_update (ddhr, &ddhr_rec);
		if (cc)
			file_err (cc, ddhr, "DBUPDATE");
	}
}

void
CalcExtend (void)
{
	/*
	 * Update coln gross tax for each line.
	 */
	inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc) 
		file_err (cc, inmr, "DBFIND");

	l_total = (double) coln_rec.q_order;
	l_total *= out_cost (coln_rec.sale_price, inmr_rec.outer_size);
	l_total = no_dec (l_total);

	l_dis = (double) coln_rec.disc_pc;
	l_dis *= l_total;
	l_dis = DOLLARS (l_dis);
	l_dis = no_dec (l_dis);

	if (notax)
		l_tax = 0.00;
	else
	{
		l_tax = (double) (coln_rec.tax_pc);
		l_tax = DOLLARS (l_tax);
		l_tax *= (l_total - l_dis);
		l_tax = no_dec (l_tax);
	}
	if (notax)
		l_gst = 0.00;
	else
	{
		l_gst = (double) (coln_rec.gst_pc);
		l_gst = DOLLARS (l_gst);
		l_gst *= (l_total - l_dis + l_tax);
	}
}


int
CheckCohr (
 char	*inv_no)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	strcpy (cohr2_rec.type,  "I");
	sprintf (cohr2_rec.inv_no, "%-8.8s", inv_no);
	return (find_rec (cohr2, &cohr2_rec, EQUAL, "r"));
}
