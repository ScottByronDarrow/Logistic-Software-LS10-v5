/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_pt_adj.c,v 5.5 2002/07/25 11:17:27 scott Exp $
|  Program Name  : (db_pt_adj.c) 
|  Program Desc  : (Customer Payment term update)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/11/90         |
|---------------------------------------------------------------------|
| $Log: db_pt_adj.c,v $
| Revision 5.5  2002/07/25 11:17:27  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.4  2002/07/23 05:09:28  scott
| .
|
| Revision 5.3  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 09:03:46  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:22:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/28 06:57:24  scott
| Updated to look better with LS10-GUI.
| Converted to use app.schema as per standard.
| Converted to use new Makefile.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_pt_adj.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_pt_adj/db_pt_adj.c,v 5.5 2002/07/25 11:17:27 scott Exp $";

#define MAXLINES	500
#include <pslscr.h>
#include <arralloc.h>
#include <ml_db_mess.h>
#include <ml_std_mess.h>
#include <p_terms.h>

extern	int		X_EALL;
extern	int		Y_EALL;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int		envVarDbCo 			= 0,
			pid 				= 0,
			workFineNo 			= 0,
			clearOK 			= TRUE,
			envVarDbFind 		= 0,
			envVarDbNettUsed 	= TRUE;

	char 	branchNumber [3];

   	double 	balance = 0.00;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;

	char	*data = "data",
			*cumr2 = "cumr2";

	struct {
		long	hhcuHash;
	} workRec;


/*=====================================================================
| The structures 'cheq'&'dtls' are initialised in function 'GetCheque' |
| the number of details is stored in external variable 'detailCnt'.    |
=====================================================================*/ 
struct	Detail {        /*|                                 |*/
	long	hhciHash;	/*| detail invoice reference.       |*/
	double	invoiceAmt;	/*| detail invoice amount.          |*/
}	*dtls;
	DArray	dtls_d;				/* state info for dynamic reallocation */
	int		detailCnt;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	est [3];
	char 	dbt_no [7];
	long	dueDate;
	char 	payTerms [4];
	char 	previousCustomerNo [7];
	char 	systemDate [11];
	char 	com_date [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customer",	 4, 16, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer No.", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{1, LIN, "name",	 5, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},

	{2, TAB, "invoice_no",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "Invoice #", " ",
		 NA, NO,  JUSTLEFT, "", "", cuin_rec.inv_no},
	{2, TAB, "inv_date",	 0, 1, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "Invoice Date", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&cuin_rec.date_of_inv},
	{2, TAB, "payTerms",	 0, 4, CHARTYPE,
		"UUU", "          ",
		" ", "20A", "Payment Terms", "nnA to NNF (20A = 20th next mth); Alt.<nnn> = no.days",
		 NO, NO,  JUSTLEFT, "", "", local_rec.payTerms},
	{2, TAB, "dueDate",	 0, 1, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", " ", "  Due Date  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.dueDate},
	{2, TAB, "hhciHash",	 0, 2, LONGTYPE,
		"NNNNNNNN", "        ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&cuin_rec.hhci_hash},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	show_pay 			(void);
void 	LoadCuin 			(int, long);
void 	Update 				(void);
void 	GetCheque 			(int, long);
int 	spec_valid 			(int);
int 	InvoiceBalance 		(void);
int 	heading 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;

	if (argc < 2 || (pid = atoi (argv [1])) < 1)
	{
		print_at (0,0,mlStdMess046,argv [0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	Y_EALL = 6;
	X_EALL = 30;

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	/*
	 *	Allocate initial detail for 1000 items
	 */
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	OpenDB ();

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	strcpy (local_rec.com_date, DateToString (comm_rec.crd_date));

	tab_row = 7;
	tab_col = 10;

	strcpy (local_rec.previousCustomerNo, "000000");

	while (prog_exit == 0) 
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;
		clearOK = TRUE,

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		clearOK = FALSE,
		/*------------------------------
		| Enter screen 2 tabular input.|
		------------------------------*/
		edit (2);

		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		clearOK = TRUE,
		/*------------------------------
		| Update selection status.     |
		------------------------------*/
		Update ();
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	ArrDelete (&dtls_d);
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen (data);

	abc_alias (cumr2, cumr);

	sprintf (filename,"%s/WORK/db_per%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	cc = RF_OPEN (filename, sizeof (workRec), "w", &workFineNo);
	if (cc)
		file_err (cc, "db_per", "WKOPEN");

	open_rec (cumr, cumr_list,CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no" 
							    		    : "cumr_id_no3");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (cudt);
	abc_fclose (cumr2);
	abc_dbclose (data);

	cc = RF_CLOSE (workFineNo);
	if (cc) 
		file_err (cc, "db_per", "WKCLOSE");
}

int
spec_valid (
 int                field)
{
	int		temp;
	int		val_pterms;
	int		i;

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("customer"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		/*----------------------
		| Get payment details. |
		----------------------*/
		GetCheque (TRUE, cumr_rec.hhcu_hash);
		if (cumr_rec.ho_dbt_hash > 0L)
			GetCheque (FALSE, cumr_rec.ho_dbt_hash);
	
		DSP_FLD ("name");

		LoadCuin (TRUE, cumr_rec.hhcu_hash);

	 	cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			LoadCuin (FALSE, cumr2_rec.hhcu_hash);
			cc = find_rec (cumr2, &cumr2_rec,NEXT,"r");
		}
		vars [label ("invoice_no")].row = lcount [2];
		scn_set (1);

		if (lcount [2] == 0)
		{
			restart = TRUE;
			errmess (ML (mlDbMess225));
			sleep (sleepTime);
		}
		
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Credit Period . |
	--------------------------*/
	if (LCHECK ("payTerms"))
	{
		if (SRCH_KEY)
		{
			show_pay ();
			return (EXIT_SUCCESS);
		}

		val_pterms = FALSE;
		/*-------------------------------------
		| Check for format NNA to NNF input . |
		-------------------------------------*/
		if (local_rec.payTerms [2] >= 'A')
		{
			temp = atoi (local_rec.payTerms);
			if (temp < 1 || temp > 30) 
			{ 
				errmess (ML (mlStdMess189));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else   
			{	
				sprintf (local_rec.payTerms,"%02d%c",
						temp, local_rec.payTerms [2]);
			}
		}
		/*------------------------------------
		| Check for straight numeric input . |
		------------------------------------*/
		else 
		{
			temp = atoi (local_rec.payTerms);
			if (temp < 0 || temp > 999) 
			{
				errmess (ML (mlStdMess182));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		val_pterms = FALSE;

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (local_rec.payTerms,
				p_terms [i]._pcode,strlen (p_terms [i]._pcode)))
			{
				sprintf (local_rec.payTerms,"%-3.3s", p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.dueDate = CalcDueDate (local_rec.payTerms, cuin_rec.date_of_inv);
	
		DSP_FLD ("dueDate");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===========================
| Search for Payment Terms. |
===========================*/
void
show_pay (void)
{
	int		i = 0;
	_work_open (3,0,40);
	save_rec ("#Cde","#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode,p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}
/*---------------------------------------------
| Load Invoice / Credit Detail Into Tabular.  |
---------------------------------------------*/
void
LoadCuin (
	int		clearTotal,
	long   	hhcuHash)
{
	if (clearTotal)
	{
		init_vars (2);
		scn_set (2);
		lcount [2] = 0;
	}
	cuin_rec.hhcu_hash 		= hhcuHash;
	cuin_rec.date_of_inv 	= 0L;
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");	
	while (!cc && hhcuHash == cuin_rec.hhcu_hash)
	{
		/*-----------------------------------------
		| See if invoice currently has a balance. |
		-----------------------------------------*/
		if (InvoiceBalance ())
		{
    		cc = find_rec (cuin, &cuin_rec, NEXT, "r");	
			continue;
    	}
		strcpy (local_rec.payTerms, cuin_rec.pay_terms);

		local_rec.dueDate =	CalcDueDate 
							(
								cuin_rec.pay_terms, 
								cuin_rec.date_of_inv
							);
	   	putval (lcount [2]++);
	   	cc = find_rec (cuin, &cuin_rec, NEXT, "r");	
	}
}

/*================================================
| Total invoice payments and determine balance . |
================================================*/
int
InvoiceBalance (void)
{
	int 	i;

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = (envVarDbNettUsed) ? twodec (cuin_rec.amt - cuin_rec.disc)
			      : twodec (cuin_rec.amt);

	for (i = 0;i < detailCnt;i++)
		if (cuin_rec.hhci_hash == dtls [i].hhciHash)
			balance -= dtls [i].invoiceAmt;
	
    return ((balance == 0.00) ? 1 : 0);
}
/*-----------------
| Update Files.   |
-----------------*/		
void
Update (void)
{
	clear ();
	print_at (0,0,ML (mlDbMess215));
	fflush (stdout);

	scn_set (2);

	/*-----------------------------------------------------
    | Add revised selection status for cuin records.      |
	-----------------------------------------------------*/
	abc_selfield (cuin, "cuin_hhci_hash");

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		cc = find_rec (cuin,&cuin_rec,EQUAL,"u");
		if (cc)
			file_err (cc, cuin, "DBFIND");

		if (strcmp (cuin_rec.pay_terms, local_rec.payTerms))
		{
			strcpy (cuin_rec.pay_terms, local_rec.payTerms);
			cuin_rec.due_date = local_rec.dueDate;

			cc = abc_update (cuin,&cuin_rec);
			if (cc)
				file_err (cc, cuin, "DBUPDATE");
		}
		else
			abc_unlock (cuin);
	}

	workRec.hhcuHash = cumr_rec.hhcu_hash;
	cc = RF_ADD (workFineNo, (char *) &workRec);
	if (cc)
		file_err (cc, "db_per", "WKADD");

	abc_selfield (cuin, "cuin_cron");

	strcpy (local_rec.previousCustomerNo,cumr_rec.dbt_no);
}

/*==================================================================
| Routine to get cheque details and hold relevent invoice Against. |
==================================================================*/
void
GetCheque (
	 int	clearTotal,
	 long	hhcuHash)
{
	if (clearTotal)
	{
		detailCnt = 0;
		abc_selfield (cuhd, "cuhd_hhcu_hash");
	}

	cuhd_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
    while (!cc && cuhd_rec.hhcu_hash == hhcuHash)
    {
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	    cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	    while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
	    {
			if (!ArrChkLimit (&dtls_d, dtls, detailCnt))
				sys_err ("ArrChkLimit ()", ENOMEM, PNAME);

	   		dtls [detailCnt].hhciHash 	= cudt_rec.hhci_hash;
	    	dtls [detailCnt].invoiceAmt = cudt_rec.amt_paid_inv;
	    	++detailCnt;

	    	cc = find_rec (cudt, &cudt_rec, NEXT, "r");
	    }
	    cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

/*-----------------
| Screen Heading. |
-----------------*/		
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		fflush (stdout);
		rv_pr (ML (mlDbMess216),16,0,1);

		print_at (0,56,ML (mlDbMess188),local_rec.previousCustomerNo);
		fflush (stdout);
		line_at (1,0,79);

		move (1,input_row);
		switch (scn)
		{
		case  1 :
			box (0,3,79,2);
			scn_set (2);

			/* Need to set line_cnt to 0 because we changed screen */
			line_cnt = 0;
			scn_write (2);
			scn_display (2);
			break;
		
		case  2 :
			box (0,3,79,2);
			scn_set (1);

			/* Need to set line_cnt to 0 because we changed screen */
			line_cnt = 0;
			scn_write (1);
			scn_display (1);
			fflush (stdout);
			break;

		}
		line_at (20,1,79);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no, comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
