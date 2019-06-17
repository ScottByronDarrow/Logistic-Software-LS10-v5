/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_exmaint.c,v 5.5 2002/07/24 08:39:05 scott Exp $
|  Program Name  : (po_exmaint.c)  
|  Program Desc  : (Currency Exchange Maintenance)
|---------------------------------------------------------------------|
|  Author        : Richard Benyon  | Date Written  : 18/04/90         |
|---------------------------------------------------------------------|
| $Log: po_exmaint.c,v $
| Revision 5.5  2002/07/24 08:39:05  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 07:00:28  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/03 04:25:27  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:15:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:22  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:41  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/23 04:12:23  scott
| Updated as MAXLINES not large enough for all currencies (world wide)
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_exmaint.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_exmaint/po_exmaint.c,v 5.5 2002/07/24 08:39:05 scott Exp $";

#define	MAXLINES		1000
#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>

#define		MAIN_SCN	1

#include	"schema"

struct commRecord	comm_rec;
struct pocrRecord	pocr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct	storeRec {
	double	factors [7];
} store [MAXLINES];

struct {
	char	dummy [11];
	char	systemDate [11];
	char	code [4];
	double	factor;
	long	ldate_up;
} local_rec;

static	struct	var	vars []	={	

	{MAIN_SCN, TAB, "code", MAXLINES, 1, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Curr.", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.code}, 
	{MAIN_SCN, TAB, "description", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "          Currency Description.          ", " ", 
		NA, NO, JUSTLEFT, "", "", pocr_rec.description}, 
	{MAIN_SCN, TAB, "factor", 0, 0, DOUBLETYPE, 
		"NNNN.NNNNNNNN", "          ", 
		" ", "", "Exchange Rate", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.factor}, 
	{MAIN_SCN, TAB, "date", 0, 1, EDATETYPE, 
		"NN/NN/NN", "          ", 
		" ", local_rec.systemDate, "Last Updated", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.ldate_up}, 
	{0, TAB, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		NA, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
int 	LoadDetails 		 (void);
int 	spec_valid 			 (int);
void 	Update 				 (void);
int 	heading 			 (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char* argv [])
{
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr 	();
	set_tty 	();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (MAIN_SCN, store, sizeof (struct storeRec));
#endif
	init_vars 	(MAIN_SCN);

	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars  (MAIN_SCN);
		lcount [MAIN_SCN] = 0;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (MAIN_SCN);
		scn_set (MAIN_SCN);
		LoadDetails ();
		scn_display (MAIN_SCN);
		edit (MAIN_SCN);
		if (restart)
            break;
			/*shutdown_prog ();*/

		Update ();

		if (prog_exit)
            break;
			/*shutdown_prog ();*/
		prog_exit = 1;

	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pocr);
	abc_dbclose ("data");
}

int
LoadDetails (
 void)
{
	scn_set (MAIN_SCN);
	lcount [MAIN_SCN] = 0;
	
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code ,"%-3.3s","   ");
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no))
	{
		/*---------------------
		| Setup local record. |
		---------------------*/
		strcpy (local_rec.code,pocr_rec.code);
		local_rec.factor = pocr_rec.ex1_factor;
		store [lcount [MAIN_SCN]].factors [0] = pocr_rec.ex1_factor;
		store [lcount [MAIN_SCN]].factors [1] = pocr_rec.ex2_factor;
		store [lcount [MAIN_SCN]].factors [2] = pocr_rec.ex3_factor;
		store [lcount [MAIN_SCN]].factors [3] = pocr_rec.ex4_factor;
		store [lcount [MAIN_SCN]].factors [4] = pocr_rec.ex5_factor;
		store [lcount [MAIN_SCN]].factors [5] = pocr_rec.ex6_factor;
		store [lcount [MAIN_SCN]].factors [6] = pocr_rec.ex7_factor;
		local_rec.ldate_up = pocr_rec.ldate_up;

		putval (lcount [MAIN_SCN]++);

		cc = find_rec (pocr,&pocr_rec,NEXT,"r");
	}

	if (lcount [MAIN_SCN] == 0)
	{
		/*-------------------------------------------
		| No Exchange Details Exist - press any key |
		-------------------------------------------*/
		print_mess (ML (mlPoMess102));
		return (EXIT_FAILURE);
	}

	vars [scn_start].row = lcount [MAIN_SCN];

	/*-------------------------
	| Normal exit - return 0. |
	-------------------------*/
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("factor"))
	{
		local_rec.ldate_up = StringToDate (local_rec.systemDate);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS); 
}

void
Update (
 void)
{
	int	i;

	for (i = 0; i < lcount [MAIN_SCN]; i++)
	{
		getval (i);

		strcpy (pocr_rec.co_no,comm_rec.co_no);
		strcpy (pocr_rec.code, local_rec.code);

		cc = find_rec (pocr,&pocr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		pocr_rec.ex1_factor = local_rec.factor;
		pocr_rec.ex2_factor = store [i].factors [1];
		pocr_rec.ex3_factor = store [i].factors [2];
		pocr_rec.ex4_factor = store [i].factors [3];
		pocr_rec.ex5_factor = store [i].factors [4];
		pocr_rec.ex6_factor = store [i].factors [5];
		pocr_rec.ex7_factor = store [i].factors [6];

		pocr_rec.ldate_up = local_rec.ldate_up;

		strcpy (pocr_rec.stat_flag, "0");

		cc = abc_update (pocr,&pocr_rec);
		if (cc)
			file_err (cc, pocr, "DBUPDATE");
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		/*------------------------------
		| Currency Exchange Maintenance |
		-------------------------------*/
		clear ();
		rv_pr (ML (mlPoMess101),26,0,1);
		line_at (1,0,80);
		line_at (20,0,80);
		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (21,0, "%s", err_str);
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
