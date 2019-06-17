/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_mr_delinp.c,v 5.3 2002/07/24 08:38:45 scott Exp $
|  Program Name  : (cr_mr_delinp.c) 
|  Program Desc  : (Suppliers Delete Input.) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: cr_mr_delinp.c,v $
| Revision 5.3  2002/07/24 08:38:45  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.2  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.1  2002/07/16 01:01:14  scott
| Updated from service calls and general maintenance.
|
| Revision 5.5  2001/12/06 08:01:19  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.4  2001/12/06 07:11:40  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/06 06:10:53  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_mr_delinp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_mr_delinp/cr_mr_delinp.c,v 5.3 2002/07/24 08:38:45 scott Exp $";

#define		TABLINES	14
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#define		ERR_BALANCE		1
#define		ERR_INVOICES		2
#define		ERR_CHEQUES		3	
#define		ERR_PORDERS		4
#define		ERR_INV_SUPP		5

#include        "schema"

struct commRecord       comm_rec;
struct sumrRecord       sumr_rec;
struct suinRecord       suin_rec;
struct suhdRecord       suhd_rec;
struct inisRecord       inis_rec;
struct pohrRecord       pohr_rec;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int	editMode = 0,
		envCrFind = 0,
		envCrCo = 0,
		pidNumber,
		workFileNumber;

	char	branchNumber [3];

	/*================================
	| Work file record for deletion. |
	================================*/
	struct {
		long	hhsuHash;	/* Hash of record to delete */
	} workRec;

	struct	storeRec {
		char	supplierNo	[sizeof	sumr_rec.crd_no];
	} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	supplierNo [sizeof sumr_rec.crd_no];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "supplierNo", 9, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " Supplier. ", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.supplierNo}, 
	{1, TAB, "supplierName", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "             Supplier Name              ", "", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, TAB, "hhsuHash", 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", "", 
		ND, NO, JUSTLEFT, "", "", (char *)&sumr_rec.hhsu_hash}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include <FindSumr.h>
/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog			(void);
void	OpenDB		 			(void);
void	CloseDB	 	 			(void);
int		CheckBalance			(void);
int		CheckInvoice			(void);
int		CheckCheques			(void);
int		CheckSupplier			(void);
int		CheckPorders			(void);
int		Update					(void);
int		heading					(int);
int		CheckDuplicateSupplier 	(char *, int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	if (argc < 2)
	{
		print_at (0,0, mlStdMess046,argv [0]);
		return (EXIT_FAILURE);
	}

	pidNumber = atoi (argv [1]);

	OpenDB ();

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	SETUP_SCR (vars);

	init_scr ();	
	set_tty	();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars (1);
	restart	= 0;
	tab_row	=	3;
	tab_col	=	13;

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		editMode	= FALSE;
		search_ok	= TRUE;
		init_vars (1);		/*  set default values		*/
		lcount [1]	= 0;

		for (i = 0; i < MAXLINES; i++)
			strcpy (store [i].supplierNo, "      ");

		heading (1);
		entry (1);
		if (prog_exit)
			continue;

		heading (1);
		scn_display (1);
		editMode = 1;
		edit (1);

		if (!restart)
			Update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence	
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/crdl%05d",
				(sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);

	cc = RF_OPEN (filename, sizeof (workRec), "w", &workFileNumber);
	if (cc)
		file_err (cc, "workRec", "WKOPEN");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envCrFind) ? "sumr_id_no" : "sumr_id_no3");

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsu_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhsu_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhsu_hash");
}	

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_unlock (sumr);
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_fclose (pohr);
	abc_fclose (inis);
	abc_dbclose ("data");

	cc = RF_CLOSE (workFileNumber);
	if (cc)
		file_err (cc, "workRec", "WKCLOSE");
}

int
spec_valid (
 int	field)
{
	/*
	 * Validate Supplier Number Input And All its Conditions.
	 */
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNo));
		strcpy (sumr_rec.est_no, branchNumber);

		/*
		 * Find Supplier.
		 */
		cc = find_rec (sumr, &sumr_rec, COMPARISON,"w");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*
		 * Check current supplier balance.
		 */
		cc = CheckBalance ();
		if (cc)
		{
			errmess (ML (mlCrMess064));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*
		 * Check current supplier invoices.
		 */
		cc = CheckInvoice ();
		if (cc)
		{
			errmess (ML (mlCrMess065));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*
		 * Check current supplier cheques.
		 */
		cc = CheckCheques ();
		if (cc)
		{
			errmess (ML (mlCrMess186));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*
		 * Check current supplier purchase orders.
		 */
		cc = CheckPorders ();
		if (cc)
		{
			errmess (ML (mlCrMess185));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*
		 * Check for inventory supplier records.
		 */
		cc = CheckSupplier ();
		if (cc)
		{
			errmess (ML (mlCrMess187));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (CheckDuplicateSupplier (local_rec.supplierNo, line_cnt))
		{
			errmess (ML ("Duplicate Supplier Input"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].supplierNo, local_rec.supplierNo);

		DSP_FLD ("supplierName");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check current supplier balance.
 */
int
CheckBalance (void)
{
	double	balance;

	/*
	 * Get Total Balance On Suppliers Ledger Card.
	 */
	balance = sumr_rec.bo_curr + sumr_rec.bo_per1 + 
			  sumr_rec.bo_per2 + sumr_rec.bo_per3;

	/*
	 * Total Balance Is Greater Than Zero.
	 */
	if (balance != 0.0) 
		return (ERR_BALANCE);

	return (EXIT_SUCCESS);
}

/*
 * Check current supplier balance.
 */
int
CheckInvoice (void)
{
	/*
	 * Check Is Supplier Has Invoices On File. 
	 */
	suin_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	if (!find_rec (suin,&suin_rec,EQUAL,"r"))
		return (ERR_INVOICES);

	return (EXIT_SUCCESS);
}

/*
 * Check current supplier cheques.
 */
int
CheckCheques (void)
{
	/*
	 * Check is supplier has cheques on file. 
	 */
	 suhd_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	if (!find_rec (suhd, &suhd_rec, EQUAL, "r"))
		return (ERR_CHEQUES);

	return (EXIT_SUCCESS);
}

/*
 * Check for inventory supplier records.
 */
int
CheckSupplier (void)
{
	/*
	 * Check is supplier has cheques on file. 
	 */
	inis_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	if (!find_rec (inis, &inis_rec, EQUAL, "r"))
		return (ERR_INV_SUPP);

	return (EXIT_SUCCESS);
}

/*
 * Check current supplier purchase orders.
 */
int
CheckPorders (void)
{
	/*
	 * Check is supplier has cheques on file. 
	 */
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	if (!find_rec (pohr, &pohr_rec, EQUAL, "r"))
		return (ERR_PORDERS);

	return (EXIT_SUCCESS);
}

int
Update (void)
{
	clear ();

	abc_selfield ("sumr", "sumr_hhsu_hash");
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++) 
	{
		getval (line_cnt);

		cc = find_rec (sumr, &sumr_rec, COMPARISON, "w");
		if (cc) 
				file_err (cc, sumr, "DBFIND");
			
		strcpy (sumr_rec.stat_flag,"9");
		cc = abc_update (sumr,&sumr_rec);
		if (cc) 
			file_err (cc, sumr, "DBUPDATE");

		workRec.hhsuHash = sumr_rec.hhsu_hash;
		cc = RF_ADD (workFileNumber, (char *) &workRec);
		if (cc) 
			file_err (cc, "workRec", "WKADD");
	}	/* end of 'for' loop	*/
	abc_selfield ("sumr", "sumr_id_no");
	prog_exit = 1;
	return (EXIT_SUCCESS);
}

/*
 * Check whether Supplier has been input before.
 * Return 1 if duplicate					       
 */
int
CheckDuplicateSupplier (
	char	*supplierNo,
	int		lineNumber)
{
	int		i;
	int		noSuppliers = (prog_status == ENTRY) ? line_cnt : lcount [1];

	for (i = 0;i < noSuppliers;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == lineNumber)
			continue;

		/*
		 * cannot be duplicate if not input.
		 */
		if (!strcmp (store [i].supplierNo, "      "))
			continue;

		if (!strcmp (store [i].supplierNo, supplierNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCrMess030),23,0,1);

		line_at (1,0,80);
		line_at (20,0,80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no,comm_rec.co_short);

		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no,comm_rec.est_short);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
