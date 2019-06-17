/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_stkdisp.c,v 5.4 2002/02/01 06:05:27 robert Exp $
|  Program Name  : (po_stkdisp.c  )                                   |
|  Program Desc  : (Suppliers Stock Backlog Display.            )     |
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
|  Date Modified : (25/03/88)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (05/10/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (20/02/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|  Date Modified : (19/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (22/02/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (09/09/97)      | Modified  by : Marnie I Organo.  |
|                                                                     |
|  Comments      : Change program to use the display utility & add    |
|                : new search & screen generator.                     |
|    (20/02/89)  : Fixed due date on screen due to printf.            |
|    (17/04/89)  : Changed MONEYTYPEs to DOUBLETYPEs.                 |
|    (19/09/90)  : General Update for New Scrgen. S.B.D.              |
|    (22/02/93)  : Updated for MOD PSL-8494.                          |
|    (01/09/95)  : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|    (09/09/97)  : Modified for Multilingual Conversion.              |
|                                                                     |
| $Log: po_stkdisp.c,v $
| Revision 5.4  2002/02/01 06:05:27  robert
| SC 00747 - added delay on error message
|
| Revision 5.3  2001/10/30 02:14:50  cha
| Fix     Issue #00645 - Dispatch Goods Returns
| Changes done by Scott.
|
| Revision 5.2  2001/08/09 09:16:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:21  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:54  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:40  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/06/13 05:02:21  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.12  1999/11/11 06:43:21  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.11  1999/11/05 05:17:20  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.10  1999/10/14 03:04:29  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/09/29 10:12:18  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:38:15  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:42  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_stkdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_stkdisp/po_stkdisp.c,v 5.4 2002/02/01 06:05:27 robert Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#ifdef PSIZE
#undef PSIZE
#endif

#define	 	PSIZE	13
#define		X_OFF		lpXoff
#define		Y_OFF		lpYoff
#define		SAME_CRD	 (!strcmp (previousSupplier, sumr_rec.crd_no))

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;

	char	*data	= 	"data",
			*ser_space = 	"                         ";
	
	int		lpXoff,
			lpYoff;

	char	branchNumber [3];

	float	supplierQty		= 0.0,
			totalQty		= 0.0;

	double	supplierShip	= 0.0,
			totalShip		= 0.0;

	long	lastpohash,
			lastsuhash;

	struct	{
		char	*_code;
	 	char	*_desc;
	} lineStatus [] =  {
		{"U", "U(napproved"},
		{"D", "D(eleted   "},
		{"O", "O(pen      "},
		{"C", "C(onfirmed "},
		{"c", "c(osted    "},
		{"R", "R(eceipted "},
		{"r", "r(ecpt Over"},
		{"T", "T(ransmited"},
		{"I", "I(n Transit"},
		{"H", "H(eld Line "},
		{"X", "X-Cancelled"},
		{"",""},
	};
	char	stat_desc [31],
			head_str [200];

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct {
	char	item [17];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 4, 13, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item No.", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.item},
	{1, LIN, "desc",	 5, 13, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	CloseDB 		(void);
void 	DisplayHeading 	(void);
void 	OpenDB 			(void);
void 	PrintTotal 		(int);
void 	ScreenHeading 	(void);
void 	shutdown_prog 	(void);
int 	ProcessPoln 	(void);
int 	spec_valid 		(int);
int 	heading 		(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	clear ();
	set_tty ();
	set_masks ();

	OpenDB ();

	while (prog_exit == 0) 
	{
		totalQty 	= 0.00;
		totalShip 	= 0.00;

		/*---------------------
		| Reset Control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		search_ok 	= TRUE;

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

		DisplayHeading ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
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

	open_rec (pohr ,pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln ,poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (inmr ,inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sumr ,sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inmr);
	abc_fclose (inum);
	SearchFindClose ();
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item, inmr_rec.item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
DisplayHeading (
 void)
{
	ScreenHeading ();
	lpXoff = 0;
	lpYoff = 2;
	Dsp_prn_open (0, 2,PSIZE, head_str, comm_rec.co_no, comm_rec.co_name,
					   comm_rec.est_no, comm_rec.est_name,
					   (char *) 0, (char *) 0);
	Dsp_saverec (" SUPP. |  SUPPLIER NAME          | BR|    DUE   |UOM.|   QTY  |   QTY  |  QTY   | PURCHASE ORDER|   EST AMOUNT  | PURCHASE ORDER ");
	Dsp_saverec (" NUMBER|                         | NO|    DATE  |    | ORDER  |RECEIVED| REMAIN |     NUMBER    | PURCHASE ORDER|      STATUS    ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [INPUT / END] ");

	ProcessPoln ();
	PrintTotal (FALSE);
	Dsp_srch ();
	Dsp_close ();
}

/*==========================
| Print heading on screen. |
==========================*/
void
ScreenHeading (
 void)
{
	clear ();
	sprintf (head_str, ML (mlPoMess052),
			clip (inmr_rec.item_no),inmr_rec.description);

	print_at (0, 2, head_str);

	move (0,1);
	line (132);
	move (0,21);
	line (132);
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no,
										clip(comm_rec.co_name));
	print_at (22, 50, ML (mlStdMess039),
			clip (comm_rec.est_no),clip (comm_rec.est_name));
	move (0,23);
	line (132);
}

/*================================================================
| Reads all orders on file that have the selected part on order  |
| Returns: 0 if ok,-1 if aborted .                               |
================================================================*/
int
ProcessPoln (
 void)
{
	int		i;

	int		first = TRUE,
			printed = FALSE;

	char	dueDate [11];

	char	wk_qty [3] [9];
	char	wk_sup [15];

	char	serial [26],
			previousSupplier [7],
			envLine [200];

	double	amt_ship 	= 0.0,
			qty_ord 	= 0.0,
			qty_rec 	= 0.0,
			qty_rem 	= 0.0;

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;

	lastpohash = lastsuhash = 0L;

	supplierQty 	= 0.00;
	supplierShip 	= 0.00;

	strcpy (previousSupplier,"      ");

	poln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec ("poln", &poln_rec, GTEQ, "r");
	while (!cc && inmr_rec.hhbr_hash == poln_rec.hhbr_hash)
	{
		if (poln_rec.qty_ord - poln_rec.qty_rec <= 0.00)
		{
			cc = find_rec ("poln", &poln_rec, NEXT, "r");
			continue;
		}
		if (lastpohash != poln_rec.hhpo_hash) 
		{
			lastpohash = poln_rec.hhpo_hash;
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec ("pohr", &pohr_rec, COMPARISON, "r");
			if (cc || pohr_rec.status [0] == 'D')
			{
				cc = find_rec ("poln", &poln_rec, NEXT, "r");
				continue;
			}
		}
		if (lastsuhash != pohr_rec.hhsu_hash) 
		{
			lastsuhash = pohr_rec.hhsu_hash;
			sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
			cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
			if (cc)
			{
				cc = find_rec ("poln", &poln_rec, NEXT, "r");
				continue;
			}

			if (!first)
				PrintTotal (TRUE);

			first = FALSE;
		}
		qty_ord = poln_rec.qty_ord;
		qty_rec = poln_rec.qty_rec;
		qty_rem = poln_rec.qty_ord - poln_rec.qty_rec;

		amt_ship = twodec (poln_rec.land_cst);
		amt_ship = out_cost (amt_ship, inmr_rec.outer_size);
		amt_ship *= qty_rem;

		if (inmr_rec.serial_item [0] == 'Y') 
			sprintf (serial, "%-25.25s", poln_rec.serial_no);
		else
			strcpy (serial, ser_space);

		strcpy (dueDate,DateToString (poln_rec.due_date));

		strcpy (stat_desc, "??????????????????????????????");

		for (i = 0; strlen (lineStatus [i]._code); i++)
		{
			if (poln_rec.pur_status [0] == lineStatus [i]._code [0])
			{
				strcpy (stat_desc,lineStatus [i]._desc);
				break;
			}
		}
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;
		
		if (qty_ord != 0.00)
			sprintf (wk_qty [0], "%8.2f", qty_ord * CnvFct);
		else
			strcpy (wk_qty [0], "        ");

		if (qty_rec != 0.00)
			sprintf (wk_qty [1], "%8.2f", qty_rec * CnvFct);
		else
			strcpy (wk_qty [1], "        ");

		if (qty_rem != 0.00)
			sprintf (wk_qty [2], "%8.2f", qty_rem * CnvFct);
		else
			strcpy (wk_qty [2], "        ");

		if (SAME_CRD)
			strcpy (wk_sup, "      ");
		else
			sprintf (wk_sup, "^1%s^6", sumr_rec.crd_no);

		sprintf (envLine,"%s ^E%-25.25s^E %-2.2s^E%-10.10s^E%4.4s^E%8.8s^E%8.8s^E%8.8s^E%-15.15s^E%14.14s ^E%-11.11s",
				wk_sup, 
				 (SAME_CRD) ? " " : sumr_rec.crd_name,
				pohr_rec.br_no,
				dueDate,
				inum_rec.uom,
				wk_qty [0], wk_qty [1], wk_qty [2],
				pohr_rec.pur_ord_no,
				comma_fmt (amt_ship, "NNN,NNN,NNN.NN"),
				stat_desc);
		
		Dsp_saverec (envLine);

		if (strcmp (serial, ser_space))
		{
			sprintf (envLine, "       | Serial Number : %25.25s   |        |        |        |               |               |                ", serial);
			Dsp_saverec (envLine);
		}

		printed = TRUE;
		strcpy (previousSupplier,sumr_rec.crd_no);

		supplierQty 	+= (float) (qty_rem	* CnvFct);
		totalQty 		+= (float) (qty_rem	* CnvFct);
		supplierShip 	+= amt_ship;
		totalShip 		+= amt_ship;

		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
	if (printed)
		PrintTotal (TRUE);

	return (EXIT_SUCCESS);
}

void
PrintTotal (
 int supp)
{
	char	envLine [200];

	sprintf 
	(
		envLine, 
		" %-s          ^E   ^E          ^E    ^E        ^E        ^E%8.2f^E               ^E%14.14s ^E                ",

		(supp) ? "** SUPPLIER TOTAL **  " : "^1** ITEM TOTAL **^6      ",
		(supp) ? supplierQty : totalQty,
		(supp) ? comma_fmt (supplierShip,"NNN,NNN,NNN.NN") 
			   : comma_fmt (totalShip,"NNN,NNN,NNN.NN")
   );

	Dsp_saverec (envLine);

	if (!supp)
		Dsp_saverec ("^^GGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGJGGGJGGGGGGGGGGJGGGGJGGGGGGGGJGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGG");
	else
		Dsp_saverec ("^^GGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGHGGGHGGGGGGGGGGHGGGGHGGGGGGGGHGGGGGGGGHGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGG");
	supplierQty = 0.00;
	supplierShip = 0.00;
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
		rv_pr (ML (mlPoMess053),55,0,1);
		move (0,1);
		line (132);

		box (0,3,132,2);

		move (0,21);
		line (132);
		print_at (22,0,ML (mlStdMess038),
			      clip (comm_rec.co_no),clip (comm_rec.co_name));
		print_at (22,45,ML (mlStdMess039),
			      clip (comm_rec.est_no),clip (comm_rec.est_name));
		move (0,23);
		line (132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
