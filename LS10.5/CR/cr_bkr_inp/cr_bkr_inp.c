/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_bkr_inp.c,v 5.4 2002/07/25 11:17:26 scott Exp $
|  Program Name  : (cr_bkr_inp.c)
|  Program Desc  : (Suppliers Paid cheque flagging for bank rec.)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 13/04/89         |
|---------------------------------------------------------------------|
| $Log: cr_bkr_inp.c,v $
| Revision 5.4  2002/07/25 11:17:26  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 08:51:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:10  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_bkr_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_bkr_inp/cr_bkr_inp.c,v 5.4 2002/07/25 11:17:26 scott Exp $";

#define MAXLINES	5000
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

   	int	envDbCo = 0;

	char 	branchNo [3];
	char	*fifteenSpaces	=	"               ";

#include	"schema"

struct commRecord	comm_rec;
struct suhpRecord	suhp_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	chq_no [sizeof suhp_rec.cheq_no];
	long	chq_date;
	char	payee [41];
	char	present [6];
	long	hhsq_hash;
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "chq_no", MAXLINES, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Cheque No.", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chq_no}, 
	{1, TAB, "hhsq_hash", 0, 1, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", " ", " ", 
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.hhsq_hash}, 
	{1, TAB, "chq_no", 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "   Date   ", " ", 
		NA, NO, JUSTLEFT, "", "", (char *)&local_rec.chq_date}, 
	{1, TAB, "payee", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "        ", 
		" ", " ", "         P A Y E E      N A M E         ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.payee}, 
	{1, TAB, "presented", 0, 2, CHARTYPE, 
		"UUUUU", "        ", 
		" ", "Y(es)", "Presented", "Enter Y/N ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.present}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <twodec.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
void	LoadSuhp		 (void);
int		Update			 (void);
int		heading			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	strcpy (branchNo, (!envDbCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		lcount [1] = 0;

		LoadSuhp ();
		
		if (lcount [1] == 0)
		{
			clear ();
			/*------------------------------
			| No Unpresented Cheques exist.|
			------------------------------*/
			rv_pr (ML (mlCrMess008), 0,0,1);
			sleep (sleepTime);
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart) 
			continue;

		/*------------------------------
		| Update selection status.     |
		------------------------------*/
		Update ();
	
		prog_exit = 1;

	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (suhp, suhp_list, SUHP_NO_FIELDS, "suhp_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (suhp);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("presented"))
	{
		strcpy (local_rec.present, (local_rec.present [0] == 'Y') ? "Y(es)" : "N(o) ");
		DSP_FLD ("presented");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------------------
| Load Invoice / Credit Detail Into Tabular.  |
---------------------------------------------*/		
void
LoadSuhp (
 void)
{
	init_vars (1);
	scn_set (1);
	lcount [1] = 0;

	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, fifteenSpaces);
	cc = find_rec (suhp, &suhp_rec, GTEQ, "r");	
	while (!cc && !strcmp (suhp_rec.co_no, comm_rec.co_no))
	{
		local_rec.hhsq_hash = suhp_rec.hhsq_hash;
		if (suhp_rec.presented [0] == 'Y')
		{
			cc = find_rec (suhp, &suhp_rec, NEXT, "r");	
			continue;
		}

		strcpy (local_rec.present, "N(o) ");
		strcpy (local_rec.payee, suhp_rec.payee_name);
		local_rec.chq_date = suhp_rec.date_payment;
		strcpy (local_rec.chq_no, suhp_rec.cheq_no);
		putval (lcount [1]++);

		cc = find_rec (suhp, &suhp_rec, NEXT, "r");	
	}
	vars [label ("chq_no")].row = lcount [1];
	scn_set (1);
}

/*=================
| Update Files.   |
=================*/
int
Update (
 void)
{
	int	wk_line;
	clear ();

	/*-----------------------------------------------------
	| Updating Cheque history with presented cheques.\n\r |
	-----------------------------------------------------*/
	print_at (0,0, ML (mlCrMess007));

	abc_selfield (suhp, "suhp_hhsq_hash");
	/*-----------------------------------------------------
	| Add revised selection status for suin records.      |
	-----------------------------------------------------*/
	scn_set (1);
	for (wk_line = 0;wk_line < lcount [1];wk_line++)
	{
		getval (wk_line);

		suhp_rec.hhsq_hash	=	local_rec.hhsq_hash;
		cc = find_rec (suhp,&suhp_rec,EQUAL,"u");
		if (cc)
			file_err (cc,suhp, "DBFIND");

		sprintf (suhp_rec.presented,"%-1.1s", local_rec.present);

		cc = abc_update (suhp,&suhp_rec);
		if (cc)
			file_err (cc,suhp, "DBUPDATE");
	}
	abc_selfield (suhp,"suhp_id_no");
	return (EXIT_SUCCESS);
}

/*-----------------
| Screen Heading. |
-----------------*/		
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		/*-------------------------------------
		| Suppliers Presented Cheque flagging.|
		-------------------------------------*/
		rv_pr (ML (mlCrMess009), 22,0,1);
		fflush (stdout);
		move (0, 1);
		line (79);

		move (1,20);
		line (79);
		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (21,0, "%s", err_str);
		move (1,22);
		line (79);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
