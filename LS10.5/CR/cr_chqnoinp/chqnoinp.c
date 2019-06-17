/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: chqnoinp.c,v 5.5 2002/07/24 08:38:43 scott Exp $
|  Program Name  : (cr_chqnoinp.c) 
|  Program Desc  : (Suppliers Cheque Number Input Program)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 27/04/89         |
|---------------------------------------------------------------------|
| $Log: chqnoinp.c,v $
| Revision 5.5  2002/07/24 08:38:43  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/21 04:10:23  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 08:51:37  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:14  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: chqnoinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqnoinp/chqnoinp.c,v 5.5 2002/07/24 08:38:43 scott Exp $";

#define MAXWIDTH	150
#define MAXLINES	1000

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include	"schema"

	char	*fifteenSpaces	=	"               ";

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	long 	pidNumber;

	struct	commRecord	comm_rec;
	struct	sumrRecord	sumr_rec;
	struct	suhdRecord	suhd_rec;

	struct	storeRec {
		char	cheques [sizeof suhd_rec.cheq_no];
	} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	est [3];
	char 	crd_no [7];
	char 	crd_name [41];
	double	amount;
	char	chq_no [sizeof suhd_rec.cheq_no];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "chq_no", MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		"0", "0", "Cheque Number", " ", 
		YES, NO, JUSTRIGHT, "0123456789", "", local_rec.chq_no}, 
	{1, TAB, "hhsp_hash", 0, 1, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", "0", "", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&suhd_rec.hhsp_hash}, 
	{1, TAB, "crd_no", 0, 1, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, TAB, "crd_name", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "        C r e d i t o r     N a m e     ", " ", 
		NA, NO, JUSTRIGHT, "", "", local_rec.crd_name}, 
	{1, TAB, "amount", 0, 0, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "", " Cheque Amt ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.amount}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
int		spec_valid		 (int);
int		Shuffle			 (void);
int		CheckDup		 (void);
int		CheckDupCheque	 (char *, int);
void	Update			 (void);
int		GetRange		 (void);
int		heading			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	if (argc < 2)
	{
		print_at (0,0,mlStdMess046, argv [0]);
		return (EXIT_FAILURE);
	}

	pidNumber = atol (argv [1]);

	SETUP_SCR (vars);


	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr	 ();
	set_tty		 ();
	set_masks	 ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars	 (1);
	
	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();

	for (i = 0; i < MAXLINES; i++)
		strcpy (store [i].cheques, fifteenSpaces);

	if (GetRange ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	while (prog_exit == 0)
	{
		abc_unlock (sumr);
		abc_unlock (suhd);

		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		lcount [1] = 0;
	
		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart) 
			continue;

		while (CheckDup ())
		{
			errmess (ML (mlCrMess123));
			sleep (sleepTime);	
			heading (1);
			scn_display (1);
			edit (1);

			if (restart) 
				continue;

		}
		/*------------------------------
		| Renumber cheques.            |
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

/*======================
| Open database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_pid");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suhd);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*-------------------------
	| Validate Cheque Number. |
	-------------------------*/
	if (LCHECK ("chq_no"))
	{
		Shuffle ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===================================================
| Shuffle lines and renumber from current position. |
===================================================*/
int
Shuffle (
 void)
{
	int		i = line_cnt;
	int		this_page = line_cnt / TABLINES;
	long	chequeNumber = 0L;
	int		firstTime = TRUE;

	print_at (2, 0, ML (mlStdMess041));
	fflush (stdout);

	putval (line_cnt);
	vars [label ("chq_no")].row = lcount [1];
	for (line_cnt = i;line_cnt < vars [label ("chq_no")].row;line_cnt++)
	{
		if (firstTime)
			chequeNumber = atol (local_rec.chq_no);

		firstTime = FALSE;
		getval (line_cnt);
		sprintf (local_rec.chq_no, "%015ld",  chequeNumber++);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	line_cnt = i;
	getval (line_cnt);
	print_at (2,0,"                        ");
	fflush (stdout);
	return (EXIT_SUCCESS);
}

/*=======================================================
| Check Whether Cheque number Has Already Been Used.    |
=======================================================*/
int
CheckDup (
 void)
{
	int	i;

	for (i = 0; i < lcount [1]; i++)
	{
		getval (i);
		strcpy (store [i].cheques, local_rec.chq_no);
	}
	for (i = 0; i < lcount [1]; i++)
	{
		getval (i);
		if (CheckDupCheque (local_rec.chq_no, i))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
	
/*=======================================================
| Check Whether Cheque number Has Already Been Used.    |
| Return 1 if duplicate									|
=======================================================*/
int
CheckDupCheque (
 char *	chequeNo,
 int	line_no)
{
	int	i;

	for (i = 0;i < lcount [1];i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*-------------------------------------------------------
		| Only compare serial numbers for the same item number	|
		-------------------------------------------------------*/
		if (!strcmp (store [i].cheques,chequeNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
/*=====================================
| Update cheque nos on suhd file.     |
=====================================*/
void
Update (
 void)
{
	int	i;

	abc_selfield (suhd, "suhd_hhsp_hash");

	for (i = 0;i < lcount [1] ;i++)
	{
		getval (i);
	
		cc = find_rec (suhd,&suhd_rec,EQUAL,"u");
		if (cc)
		{
			abc_unlock (suhd);
			continue;
		}
		strcpy (suhd_rec.cheq_no,local_rec.chq_no);
		cc = abc_update (suhd,&suhd_rec);
		if (cc)
			file_err (cc, suhd, "DBUPDATE");
	}
	abc_selfield (suhd, "suhd_pid");
}

int
GetRange (
 void)
{
	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (1);
	lcount [1] = 0;
	vars [label ("chq_no")].row = MAXLINES;

	abc_selfield (suhd, "suhd_pid");
	cc = find_hash (suhd, &suhd_rec, GTEQ, "r", pidNumber);
	while (!cc && suhd_rec.pid == pidNumber)
	{
		if (find_hash (sumr,&sumr_rec,EQUAL,"r",suhd_rec.hhsu_hash))
		{
			cc = find_hash (suhd, &suhd_rec, NEXT, "r", pidNumber);
			continue;
		}
			
		strcpy (store [lcount [1]].cheques, suhd_rec.cheq_no);
		strcpy (local_rec.crd_no, sumr_rec.crd_no);
		strcpy (local_rec.crd_name, sumr_rec.crd_name);
		local_rec.amount = suhd_rec.tot_amt_paid;
		strcpy (local_rec.chq_no, suhd_rec.cheq_no);

		putval (lcount [1]++);
		if (lcount [1] > MAXLINES) 
			break;

		cc = find_hash (suhd, &suhd_rec, NEXT, "r", pidNumber);
	}
	vars [label ("chq_no")].row = lcount [1];

	return ( (lcount [1]) ? 0 : 1);
}
		
/*-----------------
| Screen Heading. |
-----------------*/
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	fflush (stdout);
	rv_pr (ML (mlCrMess036),25,0,1);

	fflush (stdout);
	move (0, 1);
	line (80);

	move (1,20);
	line (80);
	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	
	return (EXIT_SUCCESS);
}
