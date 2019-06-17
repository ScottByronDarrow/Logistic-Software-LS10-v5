/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_oduemnt.c,v 5.5 2002/07/18 07:00:28 scott Exp $
|  Program Name  : ( po_oduemnt.c   )                                 |
|  Program Desc  : ( Enter/Maintain Overdue P/O Information.      )   |
|---------------------------------------------------------------------|
|  Author        : Basil Wood.     | Date Written  : 05/07/95         |
|---------------------------------------------------------------------|
| $Log: po_oduemnt.c,v $
| Revision 5.5  2002/07/18 07:00:28  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/17 09:57:39  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/07/03 04:25:32  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_oduemnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_oduemnt/po_oduemnt.c,v 5.5 2002/07/18 07:00:28 scott Exp $";

#define MAXSCNS     3
#define MAXWIDTH 	220
#define MAXLINES 	2000
#define TABLINES    13

#include 	<pslscr.h>
#include 	<getnum.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<get_lpno.h>

#define DIMOF(array) (sizeof(array) / sizeof(array[0]))

#define	PO_HEAD		1
#define	PO_LINES	2

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poliRecord	poli_rec;
struct exsiRecord	exsi_rec;
struct poslRecord	posl_rec;

	char 	*data	= "data",
			*fifteenSpaces	=	"               ";
	 
	int 	printerNumber = 0;
	char	branchNumber [3];
	long	lSystemDate;

char *screens[] =
{
	" Supplier Screen ",
	" Purchase Order Items ",
};

/*============================
| Local & Screen Structures. |
============================*/

struct
{
	char	dummy[11];				/* Dummy Screen Gen Field.      */

	char	supplierNo [sizeof sumr_rec.crd_no];
	Date	overdue_date;
	int 	ship_weeks;
	float	qty;
	float	outQty;
	float	qtyOnShip;
	Date	cont_date;
	char	comment [sizeof poli_rec.comment];
	Date	ship_date;
	char	inst_code [3];
	char	inst_text [31];
	char	systemDate [11];
}
local_rec;

static struct var vars[] =
{
	{PO_HEAD, LIN, "supplierNo", 4, 12, CHARTYPE,
		"UUUUUU", "          ",
		" ", sumr_rec.crd_no, "Supplier: ", "Enter Supplier Code",
		NE, NO, JUSTLEFT, "", "", local_rec.supplierNo},
	{PO_HEAD, LIN, "hhsu_hash", 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", " ", "", " ",
		ND, NO, JUSTLEFT, "", "", (char *) &sumr_rec.hhsu_hash},
	{PO_HEAD, LIN, "supplierName", 4, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name},
	{PO_HEAD, LIN, "overdue_date", 4, 80, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Overdue at:", "Enter ETA of most recent shipment for which shipping papers have been received",
		NE, NO, JUSTLEFT, "", "", (char *) &local_rec.overdue_date},
	{PO_HEAD, LIN, "ship_weeks", 4, 110, INTTYPE,
		"NN", "          ",
		" ", "0", "Ship Weeks:", " ",
		NO, NO, JUSTRIGHT, "0", "99", (char *) &local_rec.ship_weeks},

	{PO_LINES, TAB, "pur_ord_no", MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Purchase Order.", " ",
		NA, NO, JUSTLEFT, "", "", pohr_rec.pur_ord_no},
	{PO_LINES, TAB, "date_raised", 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "  Raised  ", " ",
		NA, NO, JUSTLEFT, "", "", (char *) &pohr_rec.date_raised},
	{PO_LINES, TAB, "due_date", 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Due Date ", " ",
		NA, NO, JUSTLEFT, "", "", (char *) &pohr_rec.due_date},
	{PO_LINES, TAB, "item_no", 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "  Item Number.  ", " ",
		NA, NO, JUSTLEFT, "", "", inmr_rec.item_no},
	{PO_LINES, TAB, "hhpl_hash", 0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", " ", "", " ",
		ND, NO, JUSTLEFT, "", "", (char *) &poln_rec.hhpl_hash},
	{PO_LINES, TAB, "qty", 0, 0, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "", "Quantity", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty},
	{PO_LINES, TAB, "cont_date", 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", " Contact. ", "Enter date of information from supplier",
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.cont_date},
	{PO_LINES, TAB, "comment", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "        Comment         ", "Enter status of item",
		NO, NO, JUSTLEFT, "", "", local_rec.comment},
	{PO_LINES, TAB, "ship_date", 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Ship  Date", "Enter advised date of shipping",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.ship_date},
	{PO_LINES, TAB, "inst_code", 0, 0, CHARTYPE,
		"NN", "          ",
		" ", " ", "Cd", "Enter Special Instruction Code. [SEARCH] available.",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.inst_code},
	{PO_LINES, TAB, "inst_text", 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Special Inst. ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.inst_text},

	{0, LIN, "", 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include 	<FindSumr.h>
/*========================
| Function Declarations. |
========================*/
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	SrchExsi 				(char *);
void 	LoadOutstandingItems 	(void);
void 	PrintDetails 			(void);
void 	Update 					(void);
void 	tab_other 				(int);
float 	GetShipQuantity 		(long);
char 	*DateStringLocal 		(long);
static 	void head_output 		(FILE *);
int 	spec_valid 				(int);
int 	heading 				(int);

char	tempStringDate [11];

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int i;

	if (argc > 1)
	{
		printerNumber = atoi (argv[1]);
		if (printerNumber <= 0 || argc > 2)
		{
			print_at (0,0,"%s <LPNO>]", argv[0]);
			return (EXIT_FAILURE);
		}
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (PO_HEAD);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	lSystemDate	= TodaysDate ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	for (i = 0; i < DIMOF (screens); i++)
		tab_data[i]._desc = screens[i];

	swide ();
	clear ();

	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
        | Reset control flags . |
        -----------------------*/
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;
		init_vars (PO_HEAD);
		init_vars (PO_LINES);
		lcount[PO_LINES] = 0;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		init_vars (PO_HEAD);
		heading (PO_HEAD);
		entry (PO_HEAD);

		if (prog_exit || restart)
			continue;

		LoadOutstandingItems ();

		heading (PO_LINES);
		scn_display (PO_LINES);
		edit (PO_LINES);

		if (restart)
			continue;

		/*------------------
		| Edit all screens.| 
		------------------*/
		edit_all ();

		if (restart)
			continue;

		Update ();
		break;
	}

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
	int envVarCrFind = atoi (get_env ("CR_FIND"));
	int envVarCrCo = atoi (get_env ("CR_CO"));

	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	strcpy (branchNumber, envVarCrCo ? comm_rec.est_no : " 0");

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS,
			  envVarCrFind ? "sumr_id_no3" : "sumr_id_no");

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (poli, poli_list, POLI_NO_FIELDS, "poli_hhpl_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (poli);
	abc_fclose (posl);
	abc_fclose (exsi);

	abc_dbclose (data);
}

/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char *key_val)
{
	char wk_code[4];

	work_open ();
	save_rec ("#Spec Inst", "#Instruction Description.");

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = 0;

	cc = find_rec (exsi, &exsi_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (exsi_rec.co_no, comm_rec.co_no) &&
		   exsi_rec.inst_code < 100)
	{
		sprintf (wk_code, "%02d", exsi_rec.inst_code);
		cc = save_rec (wk_code, exsi_rec.inst_text);
		if (cc)
			break;

		cc = find_rec (exsi, &exsi_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exsi", "DBFIND");
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return 0;
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.supplierNo));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess022));
			sleep (sleepTime);
			return 1;
		}

		DSP_FLD ("supplierName");
		return 0;
	}

	/*---------------------------
	| Validate Overview At Date
	---------------------------*/
	if (LCHECK("overdue_date"))
	{
		return (local_rec.overdue_date <= 0L);
	}

	/*---------------------------
	| Validate Last Information Date
	---------------------------*/
	if (LCHECK("cont_date"))
	{
		if (dflt_used)
			local_rec.cont_date = lSystemDate;

		DSP_FLD ("cont_date");
		return (local_rec.cont_date <= 0L || local_rec.cont_date > lSystemDate);
	}

	/*---------------------------
	| Validate Instruction Code
	---------------------------*/
	if (LCHECK("inst_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			return 0;
		}

		strcpy (local_rec.inst_text, "");

		exsi_rec.inst_code = atoi(local_rec.inst_code);		
		if (exsi_rec.inst_code)
		{
			strcpy (exsi_rec.co_no, comm_rec.co_no);
			cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
			if (cc)
			{
				print_err (ML (mlStdMess184));
				return 1;
			}
			sprintf (local_rec.inst_text, "%.15s", exsi_rec.inst_text);
		}
		DSP_FLD ("inst_text");

		return 0;
	}

	return 0;
}

/*=============================================
| Return total shipment qty less received qty |
=============================================*/
float
GetShipQuantity (
	long	hhplHash)
{
	float qty = 0.0;

	posl_rec.hhpl_hash	=	hhplHash;
	cc = find_rec (posl, &posl_rec, GTEQ, "r");
	while (!cc && posl_rec.hhpl_hash == hhplHash)
	{
		if (posl_rec.ship_qty != 0.00)
			qty += (posl_rec.ship_qty - posl_rec.rec_qty);

		cc = find_rec (posl, &posl_rec, NEXT, "r");
	}
	return qty;
}

/*=======================================================================
| Load all outstanding items on purchase orders for supplier where
| pohr_due_date is on or before local_rec.overdue_date.
| If All = N)o, only load outstanding items which do not have an ETA
| or the ETA is on or before local_rec.overdue_date.
=======================================================================*/
void
LoadOutstandingItems (
 void)
{
	int display_all;	

	display_all = prmptmsg ("Display All? - (Y)es/(N)o ","YyNn", 1, 2);
	move (1, 2); cl_line();

	/*------------------------
	| Set screen for putval. |
	------------------------*/
	init_vars (PO_LINES);
	scn_set (PO_LINES);
	lcount[PO_LINES] = 0;

	memset (&pohr_rec, 0, sizeof pohr_rec);
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.pur_ord_no, fifteenSpaces);
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (pohr_rec.br_no, comm_rec.est_no) &&
		   pohr_rec.hhsu_hash == sumr_rec.hhsu_hash &&
		   *pohr_rec.type == 'O')
	{
		if (*pohr_rec.status != 'D' &&
		    pohr_rec.due_date <= local_rec.overdue_date)
		{
			poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
			poln_rec.line_no = 0;

			cc = find_rec (poln, &poln_rec, GTEQ, "r");
			while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
			{
				/*---------------------
				| Outstanding Quantity
				---------------------*/
				local_rec.outQty = poln_rec.qty_ord - poln_rec.qty_rec;

				if (local_rec.outQty <= 0.00)
				{
					cc = find_rec (poln, &poln_rec, NEXT, "r");
					continue;
				}

				/*------------------
				| Information Line
				------------------*/
				poli_rec.hhpl_hash	=	poln_rec.hhpl_hash;
				cc = find_rec (poli, &poli_rec, EQUAL, "r");
				if (cc)
					memset (&poli_rec, 0, sizeof poli_rec);

				local_rec.cont_date = poli_rec.cont_date;
				strcpy (local_rec.comment, poli_rec.comment);
				local_rec.ship_date = poli_rec.ship_date;

				if (toupper(display_all) == 'N' &&
					poli_rec.eta_date > local_rec.overdue_date)
				{
					/*------------------------
					| ETA not expired, so skip
					------------------------*/
					cc = find_rec (poln, &poln_rec, NEXT, "r");
					continue;
				}

				/*---------------------
				| Check outstanding qty 
				| not on a shipment
				---------------------*/
				local_rec.qtyOnShip = GetShipQuantity (poln_rec.hhpl_hash);
				local_rec.qty = local_rec.outQty - local_rec.qtyOnShip;
				if (local_rec.outQty <= local_rec.qtyOnShip)
				{
					cc = find_rec (poln, &poln_rec, NEXT, "r");
					continue;
				}

				/*------------------
				| Item Number
				------------------*/
				inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (poln, &poln_rec, NEXT, "r");
					continue;
				}

				/*------------------------
				| Special Instruction Code
				------------------------*/
				strcpy (local_rec.inst_code, "");
				strcpy (local_rec.inst_text, "");

				if (poli_rec.inst_code)
				{
					sprintf (local_rec.inst_code, "%2d", poli_rec.inst_code);

					strcpy (exsi_rec.co_no, comm_rec.co_no);
					exsi_rec.inst_code = poli_rec.inst_code;		
					cc = find_rec (exsi, &exsi_rec, EQUAL, "r");
					if (!cc)
					{
						sprintf (local_rec.inst_text,"%.15s",exsi_rec.inst_text);
					}
				}

				putval (lcount[PO_LINES]++);

				if (lcount[PO_LINES] % 25 == 0)
					putchar ('R');

				fflush (stdout);

				cc = find_rec (poln, &poln_rec, NEXT, "r");

				/*-------------------
				| Too many lines
				-------------------*/
				if (!cc && lcount[PO_LINES] >= MAXLINES)
				{
					print_at (22, 0, ML("Too many lines (%d)"),MAXLINES);
					print_at (23, 0, ML("Remaining lines cannot be maintained"));
					return;
				}
			}
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}

	vars [scn_start].row = lcount [PO_LINES];

	move (1, 2);
	cl_line ();
	return;
}

char *
DateStringLocal (
 long	ldate)
{
	if (ldate > 0L)
		strcpy (tempStringDate, DateToString(ldate));
	else
		strcpy (tempStringDate, "          ");
	
	return (tempStringDate);
}

static void
head_output (
 FILE *fout)
{
	scn_set (PO_HEAD);

	fprintf (fout, ".START%s <%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EOVERDUE PURCHASE ORDER INFORMATION\n");
	fprintf (fout, ".E%s %s  %s\n",
				vars[label("supplierNo")].prmpt,
				local_rec.supplierNo,
				clip(sumr_rec.crd_name));
	fprintf (fout, ".E%s %s  %s %d\n",
				vars[label("overdue_date")].prmpt,
				DateToString (local_rec.overdue_date),
				vars[label("ship_weeks")].prmpt,
				local_rec.ship_weeks);
	fprintf (fout, ".EAs At : %-24.24s \n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "==================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "===========================");
	fprintf (fout, "=============");
	fprintf (fout, "=====");
	fprintf (fout, "===================\n");

	fprintf (fout, "| %s ", 	vars[label("pur_ord_no")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("date_raised")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("due_date")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("item_no")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("qty")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("cont_date")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("comment")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("ship_date")].prmpt);
	fprintf (fout, "| %s ", 	vars[label("inst_code")].prmpt);
	fprintf (fout, "| %15.15s ", vars[label("inst_text")].prmpt);
	fprintf (fout, "|\n");

	fprintf (fout, "|-----------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|--------------------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|\n");

	fprintf (fout, ".R==================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "===========================");
	fprintf (fout, "=============");
	fprintf (fout, "=====");
	fprintf (fout, "===================\n");
}

void
PrintDetails (
 void)
{
	FILE *fout = popen ("pformat", "w");
	if (!fout)
		sys_err ("Error in opening pformat ", errno, PNAME);

	head_output(fout);

	scn_set (PO_LINES);
	for (line_cnt = 0; line_cnt < lcount[PO_LINES]; line_cnt++)
	{
		getval (line_cnt);
		fprintf (fout, "| %-15.15s ", 	pohr_rec.pur_ord_no);
		fprintf (fout, "| %10.10s ", 	DateStringLocal(pohr_rec.date_raised));
		fprintf (fout, "| %10.10s ", 	DateStringLocal(pohr_rec.due_date));
		fprintf (fout, "| %-16s ", 		inmr_rec.item_no);
		fprintf (fout, "| %8.2f ", 		local_rec.qty);
		fprintf (fout, "| %10.10s ", 	DateStringLocal(local_rec.cont_date));
		fprintf (fout, "| %-24s ", 		local_rec.comment);
		fprintf (fout, "| %10.10s ", 	DateStringLocal(local_rec.ship_date));
		fprintf (fout, "| %-2s ", 		local_rec.inst_code);

		strcpy (exsi_rec.co_no, comm_rec.co_no);
		exsi_rec.inst_code = atoi (local_rec.inst_code);
		cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
		if (cc)
			fprintf (fout, "| %-15s |\n", local_rec.inst_text);
		else
			fprintf (fout, "| %-15.15s |\n", exsi_rec.inst_text);

	}

	fprintf(fout,".EOF\n");
	pclose(fout);
}

/*==========================
| Update Relevent Records. |
==========================*/
void
Update (
 void)
{
	int key;

	scn_set (PO_LINES);

	for (line_cnt = 0; line_cnt < lcount[PO_LINES]; line_cnt++)
	{
		getval (line_cnt);

		poli_rec.hhpl_hash	=	poln_rec.hhpl_hash;
		cc = find_rec (poli, &poli_rec, EQUAL, "r");
		
		poli_rec.hhpl_hash = poln_rec.hhpl_hash;
		poli_rec.cont_date = local_rec.cont_date;
		strcpy (poli_rec.comment, local_rec.comment);
		poli_rec.ship_date = poli_rec.eta_date = local_rec.ship_date;
		
		if (poli_rec.eta_date > 0L )
			poli_rec.eta_date += (local_rec.ship_weeks * 7);

		poli_rec.inst_code = atoi (local_rec.inst_code);

		if (cc)
		{
			/*------------------------------------------------
			| Add Information record if information supplied |
			------------------------------------------------*/
			if (poli_rec.cont_date > 0L ||
				poli_rec.ship_date > 0L || 
				poli_rec.inst_code ||
				strcmp (poli_rec.comment,"                        "))
			{
				cc = abc_add (poli, &poli_rec);
				if (cc)
					file_err (cc, "poli", "DBADD");
			}
		}
		else
		{
			cc = abc_update (poli, &poli_rec);
			if (cc)
				file_err (cc, "poli", "DBUPDATE");
		}
	}
	key = prmptmsg ( ML("Key 'P' to Print. Any other key to continue."),alpha,35,2);
	if ((key == 'P' || key == 'p') && !printerNumber)
		printerNumber = get_lpno (0);

	if (printerNumber && (key == 'P' || key == 'p'))
		PrintDetails ();

	return;
}

/*=============================================
| Display Infor for lines while in edit mode. |
=============================================*/
void
tab_other (
 int iline)
{
	if (cur_screen == PO_LINES)
	{
		FLD ("cont_date") = (iline < lcount[PO_LINES] ? YES : NA);
		FLD ("comment")   = (iline < lcount[PO_LINES] ? NO : NA);
		FLD ("ship_date") = (iline < lcount[PO_LINES] ? NO : NA);
		FLD ("inst_code") = (iline < lcount[PO_LINES] ? NO : NA);
	}
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();

	centre_at (0, 132, "%R Enter/Maintain Overdue P/O Information ");
	move (0, 1);
	line (132);

	move (0, 21);
	line (132);
	print_at (22, 0, " Co: %s %-35.35s  Br: %s %-35.35s Wh: %s %-35.35s",
			  comm_rec.co_no, comm_rec.co_name,
			  comm_rec.est_no, comm_rec.est_name,
			  comm_rec.cc_no, comm_rec.cc_name);

	switch (scn)
	{
	case PO_HEAD:
		box (0, 3, 132, 1);
		scn_write (scn);
		break;

	case PO_LINES:
		tab_row = 19 - TABLINES;
		tab_col = 0;
		line_cnt = 0;
		box (0, 3, 132, 1);
		scn_write (PO_HEAD);
		scn_display (PO_HEAD);
		scn_write (scn);
		break;

	default:
		break;
	}

    return (EXIT_SUCCESS);
}
