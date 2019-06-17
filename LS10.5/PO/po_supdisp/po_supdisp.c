/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_supdisp.c,v 5.4 2002/02/01 06:03:50 robert Exp $
|  Program Name  : (po_supdisp.c  )                                   |
|  Program Desc  : (Supplier Backlog Display.                   )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, pohr, poln, inmr,     ,     ,         |
|  Database      : (podb)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (05/09/87)      | Modified  by  : Scott B. Darrow. |
|                : (05/10/88)      | Modified  by  : Bee Chwee Lim.   |
|                : (20/02/88)      | Modified  by  : Scott Darrow.    |
|                : (17/04/89)      | Modified  by  : Huon Butterworth |
|                : (19/09/90)      | Modified  by  : Scott Darrow.    |
|                : (08/02/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (13/09/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      : Change program to use the display utility & add    |
|                : new search & screen generator.                     |
|   20/02/89     : Fixed problem with date due to printf/DateToString |
|   17/04/89     : Changed MONEYTYPEs to DOUBLETYPEs.                 |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|   17/04/89     : Changed MONEYTYPEs to DOUBLETYPEs.                 |
|  (08/02/93)    : Updated for Hoskyns S/C PSL-8484.                  |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (13/09/97)    : Updated for Multilingual Conversion.			      |
|                                                                     |
| $Log: po_supdisp.c,v $
| Revision 5.4  2002/02/01 06:03:50  robert
| SC 00747 - added delay on error message
|
| Revision 5.3  2001/10/19 03:08:41  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:16:19  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:58  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:21  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:43  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/11/11 06:43:22  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.13  1999/11/05 05:17:22  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.12  1999/10/14 03:04:37  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.11  1999/10/01 07:44:24  scott
| Updated to make function calls standard
|
| Revision 1.10  1999/09/29 10:12:22  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/21 04:38:17  scott
| Updated from Ansi project
|
| Revision 1.8  1999/06/17 10:06:44  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_supdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_supdisp/po_supdisp.c,v 5.4 2002/02/01 06:03:50 robert Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#ifdef PSIZE
#undef PSIZE
#endif

#define	PSIZE	13
#define	X_OFF	lpXoff 
#define	Y_OFF	lpYoff 

#define	SAME_PO	 (strcmp (previousPorder,pohr_rec.pur_ord_no))

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;

	char 	*data			= 	"data",
			*fifteenSpaces 	= 	"               ",
			*ser_space 		= 	"                         ";

	int		lpXoff,
			lpYoff;

	struct	{
		char	*_code;
	 	char	*_desc;
	} ln_status [] =  {
		{ "U", "U(napproved"},
		{ "D", "D(eleted   "},
		{ "O", "O(pen      "},
		{ "C", "C(onfirmed "},
		{ "c", "c(osted    "},
		{ "R", "R(eceipted "},
		{ "r", "r(ecpt Over"},
		{ "T", "T(ransmited"},
		{ "I", "I(n Transit"},
		{ "H", "H(eld Line "},
		{ "X", "X Cancelled"},
		{ "","" },
	};
	char	statusDesc [31];
	char	headingString [200];

	int		envVarCrCo 		= 0,
			envVarCrFind	= 0;

	char	branchNumber [3];
	char	previousPorder [sizeof pohr_rec.pur_ord_no];

	float	ord_qty = 0.00;
	float	tot_qty = 0.00;
	double	ord_amt = 0.00;
	double	tot_amt = 0.00;

/*---------------------------
| Local & Screen Structure  |
---------------------------*/
struct {
	char	purchasesOrder [sizeof pohr_rec.pur_ord_no];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNumber",	 4, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier No.", " ",
		YES, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "name",	 4, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "purchasesOrder",	 5, 15, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "ALL", "P/Order No.", "Please enter purchase order or <return> for ALL.",
		YES, NO,  JUSTLEFT, "", "", local_rec.purchasesOrder},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<get_lpno.h>
#include	<FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void 	CloseDB 		(void);
void 	DisplayHeading 	(void);
void 	OpenDB 			(void);
void 	Process 		(void);
void 	PrintTotal 		(int);
void 	SrchPohr 		(char *);
void 	shutdown_prog 	(void);
int 	PrintLine 		(long);
int 	heading 		(int);
int 	spec_valid 		(int);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr 	();
	clear 		();
	set_tty 	();
	set_masks 	();

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		tot_qty = 0.00,
		tot_amt = 0.00;

		/*---------------------
		| Reset Control Flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		search_ok 	= TRUE;
		init_vars (1);

		/*---------------------
		| Entry Screen Input  |
		---------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
	
		/*--------------------
		| Edit Screen Input  |
		--------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		Process ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	DisplayHeading ();
	lpXoff = 0;
	lpYoff = 2;
	Dsp_prn_open 
	(
		0, 
		2, 
		PSIZE, 
		headingString, 
		comm_rec.co_no, 
		comm_rec.co_name,
		comm_rec.est_no, 
		comm_rec.est_name,
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec ("PURCHASE ORDER |BR|   DATE   |UOM.|   QTY  |  QTY   |   EST VALUE   |      ITEM      |    ITEM DESCRIPTION.     | PURCHASE ORDER ");
	Dsp_saverec ("    NUMBER     |NO|   DUE    |    |  ORDER | REMAIN |  P/O  LINE.   |     NUMBER     |                          |  STATUS.       ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT/END] ");

	strcpy (previousPorder,fifteenSpaces);

	/*----------------------------------------------
	| Display details for a single purchase order. |
	----------------------------------------------*/
	if (strcmp (local_rec.purchasesOrder,"ALL            "))
	{
		PrintLine (pohr_rec.hhpo_hash);
		PrintTotal (FALSE);
		Dsp_srch ();
		Dsp_close ();
		return;
	}
	
	/*--------------------------------------------------------
	| Display details for all purchase order for a supplier. |
	--------------------------------------------------------*/
	pohr_rec.hhsu_hash	=	sumr_rec.hhsu_hash;
	cc = find_rec ("pohr", &pohr_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == pohr_rec.hhsu_hash) 
	{
		/*----------------------------
		| Purchase order is deleted. |
		----------------------------*/
		if (pohr_rec.status [0] == 'D') 
		{
			cc = find_rec ("pohr", &pohr_rec, NEXT, "r");
			continue;
		}
		cc = PrintLine (pohr_rec.hhpo_hash);

		cc = find_rec ("pohr", &pohr_rec, NEXT, "r");
	}
	PrintTotal (FALSE);
	Dsp_srch ();
	Dsp_close ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhsu_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) 
											? "sumr_id_no" : "sumr_id_no3");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inum);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inmr);
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*==================
| Heading routine. |
==================*/
void
DisplayHeading (
 void)
{
	clear ();
	crsr_off ();
	sprintf 
	(
		headingString, 
		"P/O # %s  / Supplier %s %s / Currency %s / Country = %s",
		pohr_rec.pur_ord_no,
		sumr_rec.crd_no,
		sumr_rec.crd_name,
		sumr_rec.curr_code,
		sumr_rec.ctry_code
	);
	
	print_at 
	(
		0, 
		3, 
		ML (mlPoMess064), 
		pohr_rec.pur_ord_no,
		sumr_rec.crd_no,
		sumr_rec.crd_name
	);

	print_at 
	(
		0, 
		55, 
		ML (mlPoMess133), 
		sumr_rec.curr_code,
		sumr_rec.ctry_code
	);

		
	move (3,1);
	line (130);
	
	move (0,21);
	line (132);
	print_at (22, 0, ML (mlStdMess038), 
				comm_rec.co_no,  clip (comm_rec.co_name));
	print_at (22,45, ML (mlStdMess039),
			    comm_rec.est_no, clip (comm_rec.est_name));
	move (0,23);
	line (132);
}

/*=============================
| Print purchase order lines. |
=============================*/
int
PrintLine (
	long	hhpoHash)
{
	char	env_line [200],
			date1 [11],
			wk_qty [2] [9],
			serial [25],
			po_str [25];

	int		i,
			printed = FALSE;

	double	balance 	= 0.0,
			qty_ord 	= 0.0,
			qty_rem 	= 0.0;

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	ord_qty = 0.00;
	ord_amt = 0.00;

	poln_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec ("poln", &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash) 
	{
		qty_rem = (double) poln_rec.qty_ord - poln_rec.qty_rec;
		qty_ord = (double) poln_rec.qty_ord;

		/*------------------------------------------------
		| Purchase order line has been receiped in full. |
		------------------------------------------------*/
		if (qty_rem <= 0.00)
		{
			cc = find_rec ("poln", &poln_rec, NEXT, "r");
			continue;
		}
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		if (find_rec ("inmr", &inmr_rec, EQUAL, "r"))
		{
			strcpy (inmr_rec.item_no,	 " ");
			strcpy (inmr_rec.description," ");
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;
		
		balance = twodec (poln_rec.land_cst);
		balance = out_cost (balance,inmr_rec.outer_size);
		balance *= qty_rem;

		ord_qty += (float) (qty_rem	* CnvFct);
		tot_qty += (float) (qty_rem * CnvFct);
		ord_amt += balance;
		tot_amt += balance;

		strcpy (date1, DateToString (poln_rec.due_date));

		if (inmr_rec.serial_item [0] == 'Y') 
			sprintf (serial, "%-25.25s", poln_rec.serial_no);
		else
			strcpy (serial, ser_space);

		strcpy (statusDesc, "??????????????????????????????");

		for (i = 0; strlen (ln_status [ i ]._code); i++)
		{
			if (poln_rec.pur_status [0] == ln_status [ i ]._code [0])
			{
				strcpy (statusDesc,ln_status [ i ]._desc);
				break;
			}
		}
		if (qty_rem != 0.00)
			sprintf (wk_qty [0], "%8.2f", qty_rem * CnvFct);
		else
			strcpy (wk_qty [0], "        ");

		if (qty_ord != 0.00)
			sprintf (wk_qty [1], "%8.2f", qty_ord * CnvFct);
		else
			strcpy (wk_qty [1], "        ");
		
		if (SAME_PO)
			sprintf (po_str, "^1%s^6", pohr_rec.pur_ord_no);
		else
			strcpy (po_str, fifteenSpaces);

		sprintf (env_line,"%s^E%2.2s^E%10.10s^E%4.4s^E%8.8s^E%8.8s^E%14.14s ^E%-16.16s^E%-26.26s^E %s",
			po_str,
			pohr_rec.br_no,
			date1,
			inum_rec.uom,
			wk_qty [1], wk_qty [0],
			comma_fmt (balance, "NNN,NNN,NNN.NN"),
			inmr_rec.item_no,
			inmr_rec.description,
			statusDesc);

		Dsp_saverec (env_line);

		if (strcmp (serial, ser_space))
		{
			sprintf (env_line,"               ^E  ^E          ^E    ^E        ^E        ^E               ^E  Serial Number : %25.25s^E ",
					serial);

			Dsp_saverec (env_line);
		}
		printed = TRUE;

		strcpy (previousPorder,pohr_rec.pur_ord_no);

		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
	if (printed)
		PrintTotal (TRUE);

	return (EXIT_SUCCESS);
}

void
PrintTotal (
 int end_tot)
{
	char	env_line [200];
  	sprintf (env_line, " %s         ^E    ^E        ^E%8.2f^E%14.14s ^6^E                ^E                          ^E",
		 (end_tot) ? "*** ORDER TOTAL ***" : "^1*** GRAND TOTAL ***",
		 (end_tot) ? ord_qty : tot_qty,
		 (end_tot) ? comma_fmt (ord_amt, "NNN,NNN,NNN.NN") 
		 	 : comma_fmt (tot_amt, "NNN,NNN,NNN.NN"));

	Dsp_saverec (env_line);

	if (end_tot)
     		Dsp_saverec ("^^GGGGGGGGGGGGGGGEGGEGGGGGGGGGGEGGGGEGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGG");
	else
     		Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGJGGGGGGGGGGJGGGGEGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGG");
	ord_qty = 0.00;
	ord_amt = 0.00;
}

int
heading (
 int scn)
{
	swide ();
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML (mlPoMess104) ,50,0,1);

		move (0,1);
		line (132);

		box (0,3,130,2);

		move (0,21);
		line (132);
		print_at (22,0, ML (mlStdMess038),
					comm_rec.co_no,
					clip (comm_rec.co_name));
		print_at (22,45,ML (mlStdMess039),
					comm_rec.est_no,
					clip (comm_rec.est_name));
		move (0,23);
		line (132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("supplierNumber"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (sumr_rec.crd_no));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("supplierNumber");
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("purchasesOrder"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.purchasesOrder,	"%-15.15s", "ALL");
			sprintf (pohr_rec.pur_ord_no,		"%-15.15s", "ALL");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		abc_selfield ("pohr","pohr_id_no");

		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.type,"O");
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.purchasesOrder,15));
		cc = find_rec ("pohr",&pohr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		abc_selfield ("pohr","pohr_hhsu_hash");

		if (pohr_rec.status [0] == 'D')
		{
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchPohr (
 char *key_val)
{
	work_open ();
	abc_selfield ("pohr", "pohr_id_no");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	strcpy (pohr_rec.type, "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no, "%-15.15s",key_val);
	save_rec ("#Purchase Order ","#Date      "); 
	cc = find_rec ("pohr",&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (pohr_rec.br_no,comm_rec.est_no) && 
		      !strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.status [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type,"O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec ("pohr",&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");

	abc_selfield ("pohr","pohr_hhsu_hash");
}

