/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_bgsales.c,v 5.2 2001/10/23 07:16:34 scott Exp $
|  Program Name  : (so_bgsales.c  )                                 |
|  Program Desc  : (Updates sosa , Sales order analysis files.  )   |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow    | Date Written  : 10/12/89         |
|---------------------------------------------------------------------|
| $Log: so_bgsales.c,v $
| Revision 5.2  2001/10/23 07:16:34  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_bgsales.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_bgsales/so_bgsales.c,v 5.2 2001/10/23 07:16:34 scott Exp $";

#include	<pslscr.h>
#include	<signal.h>
#include	<alarm_time.h>
#include	<twodec.h>

#define		ORDERS 		 (!strcmp (sobg_rec.type, "AO"))
#define		INVOICES 	 (!strcmp (sobg_rec.type, "AI"))
#define		BAD_HASH 	 (sobg_rec.hash == 0L)
#define		MAX_BR		100

#include	"schema"

struct sobgRecord	sobg_rec;
struct bproRecord	bpro_rec;
struct sosaRecord	sosa_rec;
struct sohrRecord	sohr_rec;
struct cohrRecord	cohr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct cumrRecord	cumr_rec;

static char *data = "data";

	int 	MCURR;
	int		envVarDbNettUsed 	= TRUE;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Process 		(void);
void 	ProcessOrder 	(long);
void 	ProcessInvoice 	(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		found = 0;
	pid_t	pid = getpid ();
	char	program [15];
	char	*sptr = chk_env ("DB_MCURR");

	if (sptr)
		MCURR = atoi (sptr);
	else
		MCURR = FALSE;

	
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	OpenDB ();

	sprintf (bpro_rec.program, "%-14.14s", argv [0]);
	sprintf (program, "%-14.14s", argv [0]);

	cc = find_rec (bpro, &bpro_rec, GTEQ, "u");
	while (!cc && !strcmp (bpro_rec.program, program))
	{
		if (bpro_rec.pid != 0 && bpro_rec.pid == pid)
		{
			abc_unlock (bpro);
			found = 1;
			break;
		}
		abc_unlock (bpro);
		cc = find_rec (bpro, &bpro_rec, NEXT, "u");
	}
	abc_unlock (bpro);

	if (!found)
	{
		strcpy (bpro_rec.co_no, " 1");
		strcpy (bpro_rec.br_no, "  ");
		strcpy (bpro_rec.program, program);
		bpro_rec.hash = 0L;
		bpro_rec.pid = 0;
	}

	Process ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	open_rec (sosa, sosa_list, SOSA_NO_FIELDS, "sosa_id_no");
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no_2");
	open_rec (bpro, bpro_list, BPRO_NO_FIELDS, "bpro_program");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhcl_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sosa);
	abc_fclose (sobg);
	abc_fclose (bpro);
	abc_fclose (soln);
	abc_fclose (coln);
	abc_fclose (cohr);
	abc_fclose (sohr);
	abc_fclose (cumr);
	abc_dbclose (data);
}

/*==========================================
| Process sobg records			   |
| where sobg_type = "RC" or sobg_type = RO |
==========================================*/
void
Process (
 void)
{
	signal_on ();
	/*-------------------------------
	| Process Until Prog Exit	|
	-------------------------------*/
	while (!prog_exit)
	{
		/*-------------------------------
		| Initialise to bpro record	|
		-------------------------------*/
		strcpy (sobg_rec.type, "AI");
		sobg_rec.lpno = 0;
		sobg_rec.hash = -1L;
		cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
		while (!prog_exit && !cc)
		{
			if (!ORDERS && !INVOICES)
			{
				abc_unlock (sobg);
				break;
			}
			if (BAD_HASH)
			{
				abc_delete (sobg);
				break;
			}

			if (ORDERS)
				ProcessOrder (sobg_rec.hash);

			if (INVOICES)
				ProcessInvoice (sobg_rec.hash);

			/*---------------------------------------
			| Delete sobg record as processed	|
			---------------------------------------*/
			abc_unlock (sobg);
			abc_delete (sobg);

			/*-------------------------------
			| Initialise to bpro record	|
			-------------------------------*/
			strcpy (sobg_rec.type, "AI");
			sobg_rec.lpno = 0;
			sobg_rec.hash = -1L;
			cc = find_rec (sobg, &sobg_rec, GTEQ, "u");
		}
		abc_unlock (sobg);
		/*-----------------------------------------------
		| Ran out of records to process so start timing	|
		-----------------------------------------------*/
		time_out ();
	}
}

void
ProcessOrder (
 long hhsl_hash)
{
	float	l_qty = 0.00;
	double	l_disc = 0.00;
	double	l_total = 0.00;

	cc = find_hash (soln, &soln_rec, COMPARISON, "r", hhsl_hash);
	if (cc)
		return;

	cc = find_hash (sohr, &sohr_rec, COMPARISON, "r", soln_rec.hhso_hash);
	if (cc)
		return;

	l_qty = soln_rec.qty_order + soln_rec.qty_bord;

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	(double) l_qty;
		l_total	*=	soln_rec.sale_price;
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (MCURR && sohr_rec.exch_rate != 0.00)
		{
			l_total 	/= sohr_rec.exch_rate;
			l_disc 		/= sohr_rec.exch_rate;
		}
		l_total	=	no_dec (l_total);
		l_disc	=	no_dec (l_disc);
	}
	strcpy (sosa_rec.type, "O");
	sosa_rec.hhsa_hash = hhsl_hash;
	cc = find_rec (sosa, &sosa_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (sosa_rec.co_no, sohr_rec.co_no);
		strcpy (sosa_rec.br_no, sohr_rec.br_no);
		sosa_rec.date = (soln_rec.due_date == 0L) ? 
					sohr_rec.dt_raised :
					soln_rec.due_date;

		sosa_rec.hhbr_hash = soln_rec.hhbr_hash;
		sosa_rec.hhcu_hash = sohr_rec.hhcu_hash;
		strcpy (sosa_rec.sman_no, soln_rec.sman_code);
		sosa_rec.qty   = l_qty;
		sosa_rec.value = DOLLARS (l_total);
		sosa_rec.disc  = DOLLARS (l_disc);

		cc = abc_update (sosa, &sosa_rec);
		if (cc)
			file_err (cc, sosa, "DBUPDATE");
	}
	else
	{
		strcpy (sosa_rec.co_no, sohr_rec.co_no);
		strcpy (sosa_rec.br_no, sohr_rec.br_no);
		sosa_rec.date = (soln_rec.due_date == 0L) ? 
					sohr_rec.dt_raised :
					soln_rec.due_date;
		sosa_rec.hhbr_hash = soln_rec.hhbr_hash;
		sosa_rec.hhcu_hash = sohr_rec.hhcu_hash;
		strcpy (sosa_rec.sman_no, soln_rec.sman_code);
		sosa_rec.qty   = l_qty;
		sosa_rec.value = DOLLARS (l_total);
		sosa_rec.disc  = DOLLARS (l_disc);
		sosa_rec.cost  = 0.00;
		cc = abc_add (sosa, &sosa_rec);
		if (cc)
			file_err (cc, sosa, "DBADD");
	}
}

void
ProcessInvoice (
 long hhcl_hash)
{
	double	l_total		=	0.00,
			l_disc		=	0.00;
	float	l_qty = 0.00;

	cc = find_hash (coln, &coln_rec, COMPARISON, "r", hhcl_hash);
	if (cc)
		return;

	cc = find_hash (cohr, &cohr_rec, COMPARISON, "r", coln_rec.hhco_hash);
	if (cc)
		return;

	l_qty = coln_rec.q_order + coln_rec.q_backorder;
	if (coln_rec.bonus_flag [0] != 'Y')
	{
		l_total	=	coln_rec.gross;
		l_disc	=	coln_rec.amt_disc;

		if (MCURR && sohr_rec.exch_rate != 0.00)
		{
			l_total 	/= sohr_rec.exch_rate;
			l_disc 		/= sohr_rec.exch_rate;
		}
		l_total	=	no_dec (l_total);
		l_disc	=	no_dec (l_disc);
	}
	strcpy (sosa_rec.type, "I");
	sosa_rec.hhsa_hash = hhcl_hash;
	cc = find_rec (sosa, &sosa_rec, COMPARISON, "r");
	if (!cc)
	{
		strcpy (sosa_rec.co_no, cohr_rec.co_no);
		strcpy (sosa_rec.br_no, cohr_rec.br_no);
		sosa_rec.date = (coln_rec.due_date == 0L) ? 
					cohr_rec.date_raised :
					coln_rec.due_date;

		sosa_rec.hhbr_hash = coln_rec.hhbr_hash;
		sosa_rec.hhcu_hash = cohr_rec.hhcu_hash;
		strcpy (sosa_rec.sman_no, coln_rec.sman_code);
		sosa_rec.qty   = l_qty;
		sosa_rec.value = DOLLARS (l_total);
		sosa_rec.disc  = DOLLARS (l_disc);

		cc = abc_update (sosa, &sosa_rec);
		if (cc)
			file_err (cc, sosa, "DBUPDATE");
	}
	else
	{
		strcpy (sosa_rec.co_no, cohr_rec.co_no);
		strcpy (sosa_rec.br_no, cohr_rec.br_no);
		sosa_rec.date = (coln_rec.due_date == 0L) ? 
					cohr_rec.date_raised :
					coln_rec.due_date;
		sosa_rec.hhbr_hash = coln_rec.hhbr_hash;
		sosa_rec.hhcu_hash = cohr_rec.hhcu_hash;
		strcpy (sosa_rec.sman_no, coln_rec.sman_code);
		sosa_rec.qty   = l_qty;
		sosa_rec.value = DOLLARS (l_total);
		sosa_rec.disc  = DOLLARS (l_disc);
		sosa_rec.cost  = 0.00;
		cc = abc_add (sosa, &sosa_rec);
		if (cc)
			file_err (cc, sosa, "DBADD");
	}
}
