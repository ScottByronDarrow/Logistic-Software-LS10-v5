/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_shipass.c,v 5.3 2002/07/03 04:25:34 scott Exp $
|  Program Name  : (po_shipass.c  )                                   |
|  Program Desc  : (Purchase Order Shipment Assignment.         )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 04/09/90         |
|---------------------------------------------------------------------|
|  Date Modified : (16/09/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (08/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (28/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      :                                                    |
|  (16/09/93)    : HGP 9503. Increase cus_ord_ref to 20 chars.        |
|  (08/09/97)    : Modified for Multilingual Conversion.              |
|  (28/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                :                                                    |
| $Log: po_shipass.c,v $
| Revision 5.3  2002/07/03 04:25:34  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:16:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:16  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:07  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:44  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:07  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:17  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:36  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/11 06:43:19  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.10  1999/11/05 05:17:18  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.9  1999/10/14 03:04:27  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.8  1999/09/29 10:12:14  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 04:38:12  scott
| Updated from Ansi project
|
| Revision 1.6  1999/06/17 10:06:38  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_shipass.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_shipass/po_shipass.c,v 5.3 2002/07/03 04:25:34 scott Exp $";

#define MAXWIDTH 	150
#define MAXLINES 	500

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <pslscr.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct pohrRecord	pohr_rec;
struct poshRecord	posh_rec;
struct polnRecord	poln_rec;
struct sohrRecord	sohr_rec;
struct cumrRecord	cumr_rec;

	/*=============================================================== 
	| Special fields and flags  ##################################. |
	===============================================================*/
	int		envVarCrCo 	= 0;
	char	branchNumber [3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	previousPo [sizeof pohr_rec.pur_ord_no];
	char	previousSupplier [7];
	char	systemDate [11];
	char 	supplierNo [7];
	long	hhsoHash;
	char	orderNumber [9];
	char	customerNumber [7];
	char	orderReference [21];
	char	customerAcronym [10];
	char	shipmentNumber [13];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplier", 	 3, 21, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier No.", " ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.supplierNo}, 
	{1, LIN, "name", 	 3, 80, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " - ", " ", 
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "porder", 	 4, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "Purchase Order No.", " ", 
		 NE, NO,  JUSTLEFT, "", "", pohr_rec.pur_ord_no}, 
	{1, LIN, "date_raised", 	 4, 80, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Date Raised.", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&pohr_rec.date_raised}, 

	{2, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number.  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.item_no}, 
	{2, TAB, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "           Item  Description.           ", " ", 
		 NA, NO,  JUSTLEFT, "", "", poln_rec.item_desc}, 
	{2, TAB, "shipNumber", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAA", "          ", 
		"0", " ", "Shipment No ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)local_rec.shipmentNumber}, 
	{2, TAB, "shipHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.ship_no}, 
	{2, TAB, "hhpl", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&poln_rec.hhpl_hash}, 
	{2, TAB, "hhso", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", " ", "", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhsoHash}, 
	{2, TAB, "dueDate", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "00/00/00", " Due Date ", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&poln_rec.due_date}, 
	{2, TAB, "orderno", 	 0, 0, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", " ", "Order No", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.orderNumber}, 
	{2, TAB, "cust_no", 	 0, 0, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Cust. ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerNumber}, 
	{2, TAB, "cus_ref", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Cust. Order Ref.  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.orderReference}, 
	{2, TAB, "cust_acro", 	 0, 0, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", " Acronym ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerAcronym}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <FindSumr.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	GetPoln 			(long);
void 	IntFindSupercession (char *);
void 	Update 				(void);
void 	SrchPohr 			(char *);
void 	SrchSohr 			(char *);
void 	PrintCoStuff 		(void);
int 	spec_valid 			(int);
int 	heading 			(int);
char 	*GetShipmentNo		(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);


	tab_row = 6;
	tab_col = 0;

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	envVarCrCo = atoi (get_env ("CR_CO"));

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNumber, (!envVarCrCo) ? " 0" : comm_rec.est_no);

	swide ();
	clear ();

	strcpy (local_rec.previousPo,  		"000000000000000");
	strcpy (local_rec.previousSupplier, "000000");

	while (prog_exit == 0)
	{
		if (restart) 
			abc_unlock (poln);

		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		lcount [2] 	= 0;
		init_vars (1);	
		init_vars (2);	

		/*------------------------------
		| Enter screen 1 linear input. |
		| Turn screen initialise on.   |
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

		scn_display (2);
		edit (2);
		if (restart)
			continue;

		Update ();

		init_ok = TRUE;
		init_vars (2);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
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

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrCo) 
											? "sumr_id_no" : "sumr_id_no3");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (posh);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sohr);
	abc_fclose (cumr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int	i;
	
	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.supplierNo, 6));
		cc = find_rec ("sumr", &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
		
	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("porder"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type, "O");
		if (find_rec (pohr, &pohr_rec, COMPARISON, "r"))
		{
			print_mess (ML (mlStdMess048));
			return (EXIT_FAILURE);
		}
		/*------------------------
		| Order already on file. |
		------------------------*/
		if (pohr_rec.status [0] == 'D')
		{
			print_mess (ML (mlPoMess001));
			return (EXIT_FAILURE);
		}
		GetPoln (pohr_rec.hhpo_hash);
		entry_exit = 1;
		if (lcount [2] <= 0) 
		{
			print_mess (ML (mlPoMess004));
			sleep (sleepTime);
			restart = 1;
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		if (prog_status == ENTRY)
		{
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("orderno"))
	{
		if (end_input)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			skip_entry = 3;
			local_rec.hhsoHash = 0L;
			sprintf (local_rec.orderNumber, 	"%8.8s", 	" ");
			sprintf (local_rec.customerNumber, 	"%6.6s", 	" ");
			sprintf (local_rec.customerAcronym, "%9.9s", 	" ");
			sprintf (local_rec.orderReference, 	"%20.20s", 	" ");
			for (i = 0;i < 4;i++)
				display_field (field + i);
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		strcpy (sohr_rec.order_no, zero_pad (local_rec.orderNumber, 8));
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess102));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (local_rec.customerNumber, 	cumr_rec.dbt_no);
			strcpy (local_rec.customerAcronym, 	cumr_rec.dbt_acronym);
		}
		else
		{
			sprintf (local_rec.customerNumber, 	"%6.6s", " ");
			sprintf (local_rec.customerAcronym, "%9.9s", " ");
		}

		strcpy (local_rec.orderNumber, sohr_rec.order_no);
		strcpy (local_rec.orderReference, sohr_rec.cus_ord_ref);
		for (i = 1;i < 4;i++)
			display_field (field + i);

		local_rec.hhsoHash = sohr_rec.hhso_hash;
		return (EXIT_SUCCESS); 
	}
	return (EXIT_SUCCESS);
}

/*=======================================================================
| Routine to read all poln records whose hash matches the one on the    |
| pohr record. Stores all non screen relevant details in another        |
| structure. Also gets part number for the part hash. And g/l account   |
| number.                                                               |
=======================================================================*/
void
GetPoln (
 long hhpo_hash)
{
	int		noSohr = 0;

	print_at (19, 1, ML (mlStdMess035));
	fflush (stdout);
	
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (sohr, "sohr_hhso_hash");

	poln_rec.hhpo_hash 	= hhpo_hash;
	poln_rec.line_no 	= 0; 

	cc = find_rec (poln, &poln_rec, GTEQ, "r");

	while (!cc && poln_rec.hhpo_hash == hhpo_hash)
	{
		noSohr = 0;
		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc) 
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (strcmp (inmr_rec.supercession, "                "))
		{
			abc_selfield (inmr, "inmr_id_no");
			IntFindSupercession (inmr_rec.supercession);
			abc_selfield (inmr, "inmr_hhbr_hash");
		}
		/*---------------------
		| Setup local record. |
		---------------------*/
		sohr_rec.hhso_hash	=	poln_rec.hhso_hash;
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (!cc)
		{
			cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (local_rec.customerNumber, cumr_rec.dbt_no);
				strcpy (local_rec.customerAcronym, cumr_rec.dbt_acronym);
			}
			else
				noSohr = 1;

			strcpy (local_rec.orderNumber, sohr_rec.order_no);
			strcpy (local_rec.orderReference, sohr_rec.cus_ord_ref);
		}
		else
			noSohr = 1;
		
		if (poln_rec.hhso_hash == 0L || noSohr)
		{
			sprintf (local_rec.orderNumber, 	"%8.8s", " ");
			sprintf (local_rec.customerNumber, 	"%6.6s", " ");
			sprintf (local_rec.customerAcronym, "%9.9s", " ");
			sprintf (local_rec.orderReference, 	"%20.20s", " ");
		}
		local_rec.hhsoHash = poln_rec.hhso_hash;

		if (poln_rec.ship_no > 0L)
			strcpy (local_rec.shipmentNumber, GetShipmentNo (poln_rec.ship_no));

		putval (lcount [2]);

		cc = find_rec (poln, &poln_rec, NEXT, "r");

		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lcount [2]++ > MAXLINES) 
			break;
	}

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (sohr, "sohr_id_no2");

	/*-----------------------------------------------
	| Prevents entry past end of number of entries.	|
	-----------------------------------------------*/
	vars [scn_start].row = lcount [2];

	scn_set (1);
	move (1, 19);
	cl_line ();
}

void
IntFindSupercession (
 char *item_no)
{
	if (!strcmp (item_no, "                "))
		return;

	abc_selfield (inmr, "inmr_id_no");
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", item_no);
	cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
	if (!cc)
		IntFindSupercession (inmr_rec.supercession);
}

void
Update (
 void)
{
	int	workLine = 0;

	scn_set (1);

	clear ();
	print_at (15, 0, ML (mlPoMess026));
	fflush (stdout);

	/*-----------------------------------
	| Process all purchase order lines	| 
	-----------------------------------*/
	print_at (15, 0, ML (mlPoMess066));
	fflush (stdout);

	scn_set (2);
	abc_selfield (poln, "poln_hhpl_hash");

	for (workLine = 0;workLine < lcount [2];workLine++) 
	{
		getval (workLine);

		cc = find_rec (poln, &poln_rec, EQUAL, "u");
		if (cc)
			continue;

		poln_rec.hhso_hash = local_rec.hhsoHash;

		cc = abc_update (poln, &poln_rec);
		if (cc) 
			file_err (cc, "poln", "DBUPDATE");
	}
	abc_selfield (poln, "poln_id_no");
	abc_unlock (poln);

	sprintf (local_rec.previousPo, "%-15.15s", pohr_rec.pur_ord_no);
	sprintf (local_rec.previousSupplier,"%-6.6s", sumr_rec.crd_no);
	scn_set (1);
}

/*================================
| Search for purch order number. |
================================*/
void
SrchPohr (
 char *keyValue)
{
	work_open ();
	save_rec ("#Purchase order ", "#Date Raised");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", keyValue);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr_rec.pur_ord_no, keyValue, strlen (keyValue)) && 
		!strcmp (pohr_rec.co_no, comm_rec.co_no) && 
		!strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] == 'O' && 
		     pohr_rec.status [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}

/*==========================
| Search for order number. |
==========================*/
void
SrchSohr (
	char *keyValue)
{
	work_open ();
	save_rec ("#Ord No", "#Cust Order Ref.");
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", keyValue);

	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");

	while (!cc && !strncmp (sohr_rec.order_no, keyValue, strlen (keyValue)) &&
	              !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
	              !strcmp (sohr_rec.br_no, comm_rec.est_no))
	{
		cc = save_rec (sohr_rec.order_no, sohr_rec.cus_ord_ref);
		if (cc)
			break;
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "sohr", "DBFIND");
}

/*=============================
| See if shipment is on file. |
=============================*/
char*
GetShipmentNo (
	long    hhshHash)
{
	posh_rec.hhsh_hash	=	hhshHash;
	cc = find_rec (posh, &posh_rec, EQUAL, "r");
	if (cc)  
		strcpy (err_str, " ");
	else
		sprintf (err_str, "%-12.12s", posh_rec.csm_no);

	return (err_str);
}           
void
PrintCoStuff (
 void)
{
	move (0, 20); line (131);
	print_at (21, 0, ML (mlStdMess038),  comm_rec.co_no,  comm_rec.co_name);
	print_at (21, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0, ML (mlStdMess099),  comm_rec.cc_no,  comm_rec.cc_short);
}

int
heading (
 int scn)
{
	crsr_off ();
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		clear ();
		move (48, 0); 
		rv_pr (ML (mlPoMess027), 48, 0, 1);
		print_at (0, 95, ML (mlPoMess007), 
					local_rec.previousSupplier, local_rec.previousPo);
		fflush (stdout);
		move (0, 1);
		line (131);

		switch (scn)
		{
		case	1:

			scn_set (2);
			scn_write (2);
			scn_display (2);

			box (0, 2, 130, 15);
			move (1, 5);
			line (129);
			break;

		case	2:
			scn_set (1);
			scn_write (1);
			scn_display (1);
			box (0, 2, 130, 15);
			move (1, 5);
			line (129);
		}

		scn_set (scn);
		move (1, input_row);
		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
		crsr_on ();
	}
    return (EXIT_SUCCESS);
}
