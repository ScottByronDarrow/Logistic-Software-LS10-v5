/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: msupcost.c,v 5.6 2002/07/31 08:35:01 robert Exp $
|  Program Name  : ( sk_msupcost.c  )                                 |
|  Program Desc  : ( Supplier / Item Qty Break Maintenance        )   |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 16/09/93         |
|---------------------------------------------------------------------|
| $Log: msupcost.c,v $
| Revision 5.6  2002/07/31 08:35:01  robert
| S/C 4157 - Fixed display issue on LS10-GUI
|
| Revision 5.5  2002/07/24 08:39:15  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/20 07:11:05  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/09/11 01:49:32  cha
| SE-194. Updated to put delays in error messages.
|
| Revision 5.2  2001/08/09 09:19:19  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:24  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/05/06 05:02:43  scott
| Updated to fix array error on quantity break 1
|
| Revision 4.1  2001/04/28 04:10:15  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated for changes to insp schema to change money fields to double to allow
| for four digit price.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: msupcost.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_msupcost/msupcost.c,v 5.6 2002/07/31 08:35:01 robert Exp $";

#define	MAXLINES	500
#define	MAXWIDTH	200

#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <getnum.h>

#include	"schema"

struct commRecord	comm_rec;
struct inspRecord	insp_rec;
struct sumrRecord	sumr_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;

		double	*insp_price		=	&insp_rec.price1;
		float	*insp_qty_brk	=	&insp_rec.qty_brk1;

	char	*data   = "data";

	int		envCrFind;
	int		envCrCo;
	char	branchNumber [3];

	/*---------------------------------------------------
	| hhbrs already in tab screen saves checking record |
	---------------------------------------------------*/
	struct	storeRec {
		long	hhbrHash;
	} store [MAXLINES];

	extern	int	_win_func;
	float	localQty [5];
	int		lastLine;

struct
{
	int		oldItem;
	char	supplierDesc [41];
	char	item [17];
	char	itemDesc [41];
	double	price [5];
	float	qty [5];
	double	base;
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "supplierNo",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier           :", "Select Supplier, Full Search Available",
		 NE, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "supplierDesc",	4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierDesc},

	{2, TAB, "itemNumber",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ",  "Item      ", "Enter Item, Full Search Available",
		YES, NO, JUSTLEFT, "", "", local_rec.item},
	{2, TAB, "itemDesc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "D  E  S  C  R  I  P  T  I  O  N    ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc},
	{2, TAB, "base",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		"", "",  "Base Cost", "",
		 NA, NO,  JUSTRIGHT, "", "", (char *) &local_rec.base},
	{2, TAB, "pr1",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", " ",  "Cost 1  ", "Enter Cost, Use Ctrl W to Edit Quantity Breaks ",
		 YES, NO,  JUSTRIGHT, "0", "999999.9999", (char *) &local_rec.price [0]},
	{2, TAB, "pr2",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", " ",  "Cost 2  ", "Enter Cost, Use Ctrl W to Edit Quantity Breaks ",
		 YES, NO,  JUSTRIGHT, "0", "999999.9999", (char *) &local_rec.price [1]},
	{2, TAB, "pr3",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", " ",  "Cost 3  ", "Enter Cost, Use Ctrl W to Edit Quantity Breaks ",
		 YES, NO,  JUSTRIGHT, "0", "999999.9999", (char *) &local_rec.price [2]},
	{2, TAB, "pr4",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", " ",  "Cost 4  ", "Enter Cost, Use Ctrl W to Edit Quantity Breaks ",
		 YES, NO,  JUSTRIGHT, "0", "999999.9999", (char *) &local_rec.price [3]},
	{2, TAB, "pr5",	 0, 0, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", " ",  "Cost 5  ", "Enter Cost, Use Ctrl W to Edit Quantity Breaks ",
		 YES, NO,  JUSTRIGHT, "0", "999999.9999", (char *) &local_rec.price [4]},
	{2, TAB, "qty1",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		"", "",  "", "",
		 ND, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qty [0]},
	{2, TAB, "qty2",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ",  "", "",
		 ND, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qty [1]},
	{2, TAB, "qty3",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ",  "", "",
		 ND, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qty [2]},
	{2, TAB, "qty4",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ",  "", "",
		 ND, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qty [3]},
	{2, TAB, "qty5",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ",  "", "",
		 ND, NO,  JUSTRIGHT, "0", "99999.99", (char *) &local_rec.qty [4]},
	{2, TAB, "oldItem",	 0, 0, INTTYPE,
		"N", "          ",
		" ", " ",  "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.oldItem},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
int		FindInis 		 (void);
int		GetQtyBreaks 	 (int);
int		LoadLines 		 (void);
int		LocalSpecValid	 (int, int);
int		heading 		 (int);
int		spec_valid 		 (int);
int		win_function 	 (int, int, int, int);
void	CloseDB 		 (void);
void	DrawBox 		 (void);
void	OpenDB 			 (void);
void	Update 			 (void);
void	shutdown_prog 	 (void);
void	tab_other 		 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{

	int	lcl_cnt;

	envCrFind 	= atoi (get_env ("CR_FIND"));
	envCrCo 	= atoi (get_env ("CR_CO"));
	SETUP_SCR (vars);


	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");
	init_scr ();
	set_tty (); 
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	prog_exit 	= FALSE;
	tab_row = 9;

	while (!prog_exit)
	{
		FLD ("itemNumber") = YES;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
		_win_func   = FALSE;
		lastLine	= 9999;
	
		/*
		 * set all hhbrHash to 0L
		 */
		for (lcl_cnt = 0; lcl_cnt < MAXLINES; lcl_cnt++)
			store [lcl_cnt].hhbrHash = 0L;

		init_vars (1);
		init_vars (2);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		heading (2);

		_win_func = TRUE;

		if (LoadLines ())
		{	
			getval (0);
			tab_other (0);
			scn_display (2);
			edit (2);
		}
		else
			entry (2);

		if (restart)
			continue;

		no_edit (1);
		edit_all ();

		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (envCrFind) 	? "sumr_id_no3"
														: "sumr_id_no");
	open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
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
	abc_fclose (inis);	
	abc_fclose (insp);	
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	lcl_cnt;

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("itemNumber"))
	{
		if (FLD ("itemNumber") == ND)
			return (EXIT_SUCCESS);

		/*------------------------------------------
		| setup so that this "item" may be edited. |
		------------------------------------------*/
		local_rec.oldItem = FALSE;

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
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*------------------------------------------------------------------
		| check lines already in screen to make sure not entered for item. |
		------------------------------------------------------------------*/
		for (lcl_cnt = 0; store [lcl_cnt].hhbrHash != 0L; lcl_cnt++)
		{
			if ((store [lcl_cnt].hhbrHash == inmr_rec.hhbr_hash) &&
				lcl_cnt != line_cnt) 
			{
				print_mess (ML (mlStdMess204));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;

		strcpy (local_rec.itemDesc ,inmr_rec.description);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		if (FindInis ())
		{
			print_mess (ML (mlSkMess539));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.base = inis_rec.fob_cost;

		/*--------------------------
		| if no insp record for this
		| item then prompt for qty
		| breaks
		----------------------------*/
		insp_rec.hhsu_hash = sumr_rec.hhsu_hash;
		insp_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (insp, &insp_rec, EQUAL, "r");
		if (cc && prog_status == ENTRY)
		{
			print_mess (ML (mlSkMess540));
			cc = GetQtyBreaks (TRUE);
			if (cc)
			{
				DrawBox ();
				return (cc);
			}
		}

		DSP_FLD ("base");
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("supplierNo"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		pad_num (sumr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		strcpy (local_rec.supplierDesc, sumr_rec.crd_name);
		DSP_FLD ("supplierDesc");
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("pr", 2))
	{
		int		num = atoi (FIELD.label + 2) - 1;
		
		if (local_rec.qty [num] == 0.00 && local_rec.price [num])
		{
			print_mess (ML (mlSkMess538));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{

	swide ();
	clear ();
	if (scn != 1)
	{
		scn_set (1);
		scn_write (1);
		scn_display (1);
		scn_set (2);
	}
	else
		scn_display (1);

	if (scn != cur_screen)
		scn_set (scn);

	strcpy (err_str,ML (mlSkMess490));
	rv_pr (err_str, (132 - (strlen (clip (err_str)))) / 2, 0, 1);

	box (0, 3, 80, 1);

	line_at (21,0, 132);

	strcpy (err_str,ML (mlStdMess038));
	print_at (22, 1, err_str,comm_rec.co_no, comm_rec.co_name);
	line_at (1,0, 132);
	
	if (scn == 2)
		DrawBox ();

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

int
LoadLines (
 void)
{
	int	lcl_cnt;

	/*-----------------------------------------------
	| load all records from insp for keyed supplier	|
	-------------------------------------------------*/
	lcount [2] = 0;
	scn_set (2);

	abc_selfield (inmr, "inmr_hhbr_hash");

	insp_rec.hhsu_hash = sumr_rec.hhsu_hash;
	insp_rec.hhbr_hash = 0L;
	cc = find_rec (insp, &insp_rec, GTEQ, "r");
	while (!cc && insp_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		inmr_rec.hhbr_hash	=	insp_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (insp, &insp_rec, NEXT, "r");
			continue;
		}
		/*----------------------------------------
		| look up the inis to obtain base price. |
		----------------------------------------*/
		if (FindInis ())
		{
			cc = find_rec (insp, &insp_rec, NEXT, "r");
			continue;
		}

		local_rec.base = inis_rec.fob_cost;
		/*-----------------------------------
		| store info into local variables	|
		-----------------------------------*/
		strcpy (local_rec.item, inmr_rec.item_no);
		strcpy (local_rec.itemDesc, inmr_rec.description);
		for (lcl_cnt = 0; lcl_cnt < 5; lcl_cnt++)
		{
			local_rec.price [lcl_cnt] 	= insp_price 	[lcl_cnt];
			local_rec.qty [lcl_cnt]		= insp_qty_brk 	[lcl_cnt];
		}

		/*---------------------------------------------------------
		| set up flag so that edit of this "item" is not possible |
		-----------------------------------------------------------*/
		local_rec.oldItem = TRUE;

		store [lcount[2]].hhbrHash = inmr_rec.hhbr_hash;
		putval (lcount [2]++);

		cc = find_rec (insp, &insp_rec, NEXT, "r");
	}
	abc_selfield (inmr, "inmr_id_no");
	return (lcount [2]);
}

int
FindInis (void)
{
	/*---------------------------------------
	| try to find priority one inis record. |
	---------------------------------------*/
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
	}
	return (cc);
}

void
tab_other (
 int line_no)
{

	if (cur_screen != 2)
		return;

	FLD ("itemNumber") = YES;

	if (prog_status == ENTRY)
		return;

	if (line_no < lcount [2])
	{
		if (local_rec.oldItem == 1)
			FLD ("itemNumber") = NA;
		_win_func = TRUE;
	}
	else
	{
		local_rec.qty [0] = local_rec.qty [1] = local_rec.qty [2] =	
		local_rec.qty [3] = local_rec.qty [4] = 0.00;
		_win_func = FALSE;
	}
	if (prog_status != ENTRY && lastLine != line_no)
	{
		lastLine = line_no;
		DrawBox ();
	}
	return;
}

void
Update (
 void)
{
	int	lcl_cnt;
	int	count2;
	/*------------------------------------------------------------------
	| extract all lines saved, in record already update otherwise add. |
	------------------------------------------------------------------*/

	for (lcl_cnt = 0; lcl_cnt < lcount [2]; lcl_cnt++)
	{
		/*-----------------------------------------------------------
		| each lines hhbr_hash has been stored in the hhbrHash array	|
		-----------------------------------------------------------*/
		insp_rec.hhbr_hash = store [lcl_cnt].hhbrHash;
		insp_rec.hhsu_hash = sumr_rec.hhsu_hash;

		cc = find_rec (insp, &insp_rec, EQUAL, "u");
		getval (lcl_cnt);
		if (cc)
		{
			int	allZero = TRUE;
			for (count2 = 0; count2 < 5; count2++)
			{
				if (local_rec.qty [count2] != 0.00)
					allZero = FALSE;
				insp_qty_brk [count2] 	= local_rec.qty 	[count2];
				insp_price 	 [count2]	= local_rec.price 	[count2];
			}
			
			/*---------------------------------------------------
			| if all qty breaks are zero then do not add record	|
			---------------------------------------------------*/
			if (!allZero)
			{
				cc = abc_add (insp, &insp_rec);
				if (cc)
					file_err (cc, insp, "DBADD");
			}
		}
		else
		{
			/*---------------------------------------------------------------
			| update record which was found with prices which were entered	|
			---------------------------------------------------------------*/
			int	allZero = TRUE;
			for (count2 = 0; count2 < 5; count2++)
			{
				if (local_rec.qty [count2] != 0.00)
					allZero = FALSE;
				insp_qty_brk [count2] 	= local_rec.qty   [count2];
				insp_price   [count2]	= local_rec.price [count2];
			}
			
			/*-----------------------------------------------
			| if all qty breaks are zero then delete record	|
			-----------------------------------------------*/
			if (allZero)
				cc = abc_delete (insp);
			else
				cc = abc_update (insp, &insp_rec);
			if (cc)
				file_err (cc, insp, (allZero) ? "DBDELETE" : "DBUPDATE");
		}
	}

}

void
DrawBox (
 void)
{
	cl_box (90, 2, 30, 5);
	strcpy (err_str, ML (mlSkMess491));
	print_at (3, 92, err_str);
	strcpy (err_str, ML (mlSkMess492));
	print_at (4, 92, err_str);
	strcpy (err_str, ML (mlSkMess493));
	print_at (5, 92, err_str);
	strcpy (err_str, ML (mlSkMess494));
	print_at (6, 92, err_str);
	strcpy (err_str, ML (mlSkMess495));
	print_at (7, 92, err_str);

	/*-----------------------------------------------------------------
	| display previous qtys if you change this then change spec_valid |
	-----------------------------------------------------------------*/
	print_at (3, 106, "%9.2f", local_rec.qty [0]);
	print_at (4, 106, "%9.2f", local_rec.qty [1]);
	print_at (5, 106, "%9.2f", local_rec.qty [2]);
	print_at (6, 106, "%9.2f", local_rec.qty [3]);
	print_at (7, 106, "%9.2f", local_rec.qty [4]);
}
int
GetQtyBreaks (
 int _new)
{
	int		count;

	for (count = 0; count < 5; count++)
		localQty [count] = local_rec.qty [count];

	crsr_on ();
	if (!_new)
		print_mess (ML (mlSkMess305));

	DrawBox ();
	for (count = 0; count < 5; count++)
	{
		while (TRUE)
		{
			local_rec.qty [count] = getfloat (105, count + 3, "NNNNNNN.NN");
			if (!LocalSpecValid (count, _new) &&
			    (last_char == ENDINPUT || 
				last_char == RESTART || 
				last_char == DOWN_KEY))
				break;

			if (last_char == UP_KEY)
			{
				if (count)
					count--;
				continue;
			}
		}

		if (local_rec.qty [count] == 0.00)
			break;

		if (last_char == RESTART)
			break;
	}
	crsr_off ();

	if (!_new)
		clear_mess ();

	if (last_char == RESTART)
	{
		for (count = 0; count < 5; count++)
			local_rec.qty [count] = localQty [count];
		DrawBox ();
		return (TRUE);
	}
	return (FALSE);
}

int
win_function (
 int fld, 
 int lin, 
 int scn, 
 int mode)
{
	if (scn != 2)
		return (EXIT_FAILURE);

	if (mode == ENTRY && fld == label ("itemNumber"))
		return (EXIT_FAILURE);

	cc = GetQtyBreaks (FALSE);
	putval (lin);
	return (EXIT_SUCCESS);
}

int
LocalSpecValid (
 int lcl_cnt, 
 int _new)
{
	/*-------------------------------------------------------
	| validate that qty break is more than previous entered |
	-------------------------------------------------------*/
	if (last_char == UP_KEY)
		return (EXIT_SUCCESS);

	if (last_char == RESTART || last_char == FN16)
		return (EXIT_SUCCESS);

	if (dflt_used || last_char == DOWN_KEY)
	{
		/*---------------------------------------------------------------
		| if last field was set to 0.00 then if dflt used set to 0.00	|
		---------------------------------------------------------------*/
		if (lcl_cnt)
		{
			if (local_rec.qty [lcl_cnt - 1] == 0.00)
				local_rec.qty [lcl_cnt] = 0.00;
		}
		if (_new)
			local_rec.qty [lcl_cnt] = 0.00;
		else
			local_rec.qty [lcl_cnt] = localQty [lcl_cnt];
	}

	/*------------------------------------------------------------------------
	| if zero is entered then this means no more qty breaks will be entered. |
	------------------------------------------------------------------------*/
	if (local_rec.qty [lcl_cnt] == 0.00)
	{
		int	count;
		for (count = lcl_cnt; count < 5; count++)
			local_rec.price [count] = local_rec.qty [count] = 0.00;

		print_at (3, 106, "%9.2f", local_rec.qty [0]);
		print_at (4, 106, "%9.2f", local_rec.qty [1]);
		print_at (5, 106, "%9.2f", local_rec.qty [2]);
		print_at (6, 106, "%9.2f", local_rec.qty [3]);
		print_at (7, 106, "%9.2f", local_rec.qty [4]);
	}

	if (lcl_cnt)
	{
		if ( local_rec.qty [lcl_cnt - 1] == 0.00 && local_rec.qty [lcl_cnt] != 0.00)
		{
			print_mess (ML (mlSkMess122));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	if (lcl_cnt)
	{
		if (local_rec.qty [lcl_cnt] != 0.00 && 
			local_rec.qty [lcl_cnt] <= local_rec.qty [lcl_cnt - 1])
		{
			sprintf (err_str, ML (mlSkMess123),local_rec.qty [lcl_cnt],local_rec.qty [lcl_cnt - 1]);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}

	/*---------------------------------------------------
	| this is from getfloat so DSP_FLD will not work	|
	| if you change this then change DrawBox			|
	---------------------------------------------------*/
	switch (lcl_cnt) {
	case	0	:
		print_at (3, 106, "%9.2f", local_rec.qty [0]);
		break;
	case	1	:
		print_at (4, 106, "%9.2f", local_rec.qty [1]);
		break;
	case	2	:
		print_at (5, 106, "%9.2f", local_rec.qty [2]);
		break;
	case	3	:
		print_at (6, 106, "%9.2f", local_rec.qty [3]);
		break;
	case	4	:
		print_at (7, 106, "%9.2f", local_rec.qty [4]);
		break;
	}
	return (EXIT_SUCCESS);
}

