/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: cstopt.i.c,v 5.3 2002/07/17 09:57:35 scott Exp $
|  Program Name  : ( po_cstopt.i.c  )                                 |
|  Program Desc  : ( Input Program for Costed GR Reports.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, pogh,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Vicki Seal      | Date Written  : 14/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (14/09/87)      | Modified  by  : Vicki Seal.      |
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (14/11/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (21/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (04/09/1997)    | Modified  by  : Jiggs A Veloz.   |
|  Date Modified : (15/10/1997)    | Modified  by  : Jiggs A Veloz.   |
|                                                                     |
|  Comments      : Removed return from read_comm ().                  |
|                : Include FindSumr.h                                 |
|                : (21/09/90) - General Update for New Scrgen. S.B.D. |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (15/10/1997)  : SEL Updated to fix mldb error.                     |
|                :                                                    |
|                :                                                    |
| $Log: cstopt.i.c,v $
| Revision 5.3  2002/07/17 09:57:35  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:15:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:20  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:24  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:40  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:06  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:11  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/11 06:43:13  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.8  1999/11/05 05:17:10  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.7  1999/09/29 10:11:55  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/21 04:37:59  scott
| Updated from Ansi project
|
| Revision 1.5  1999/06/17 10:06:21  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cstopt.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_cstopt.i/cstopt.i.c,v 5.3 2002/07/17 09:57:35 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct poghRecord	pogh_rec;

	int		envVarCrFind	=	0,
			envVarCrCo		=	0;
	char	branchNumber [3],
			*fifteenSpaces	=	"               ";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	programDesc [61];
	char	back [2];
	char	onight [2];
	char	printerString [3];
	int		printerNumber;
	char	selectType [2];
	char	supplier [7];
	char	gr_no [16];
	char	hash_no [10];
} local_rec;

static	struct	var	vars []	={	
	{1, LIN, "printerNumber", 4, 15, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "back", 5, 15, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "selectType", 7, 15, CHARTYPE, 
		"U", "          ", 
		" ", " ", "Select On.", "Select A(ll),S(upplier) or G(oods Receipt) ", 
		YES, NO, JUSTLEFT, "ASG", "", local_rec.selectType}, 
	{1, LIN, "supplier", 8, 15, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Supplier.", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.supplier}, 
	{1, LIN, "supp_name", 8, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "     - ", " ", 
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "gr_no", 9, 15, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "0", "G.R.Number. ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.gr_no}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindSumr.h>
/*======================= 
| Function Declarations |
=======================*/
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void	CloseDB 		(void);
void	SrchPogh 		(char *);
int		spec_valid 		(int);
int		heading 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);

	/*---------------------------------------
	|	parameters			|
	|	1:	program name		|
	|	2:	program description    	|
	---------------------------------------*/

	if (argc != 3)
	{
		/*----------------------------------------------
		| Usage : %s <prog_name> <program_description> |
		----------------------------------------------*/
		print_at (0,0, mlStdMess037, argv [0]);
        return (EXIT_FAILURE);
	}
	sprintf (local_rec.programDesc,"%s",clip (argv [2]));

	envVarCrCo = atoi (get_env ("CR_CO"));
	envVarCrFind = atoi (get_env ("CR_FIND"));
	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo == 0) ? " 0" : comm_rec.est_no);
	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = 1;
	init_vars (1);

	/*-----------------------------
	| Enter screen 1 linear input |
	-----------------------------*/
	heading (1);
	entry (1);
    if (prog_exit) {
		shutdown_prog ();
        return (EXIT_SUCCESS);  
    }

	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }
	
	CloseDB (); 
	FinishProgram ();

	if (local_rec.selectType [0] == 'A')
		sprintf (local_rec.hash_no,"%ld",0L);
	if (local_rec.selectType [0] == 'S')
		sprintf (local_rec.hash_no,"%ld",sumr_rec.hhsu_hash);
	if (local_rec.selectType [0] == 'G')
		sprintf (local_rec.hash_no,"%ld",pogh_rec.hhgr_hash);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight [0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				argv [1],
				local_rec.printerString,
				local_rec.selectType,
				local_rec.hash_no,
				argv [2],0);
		}
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp (argv [1],
				argv [1],
				local_rec.printerNumber,
				local_rec.selectType,
				local_rec.hash_no,0);
		}
	}
	else 
	{
		execlp (argv [1],
			argv [1],
			local_rec.printerNumber,
			local_rec.selectType,
			local_rec.hash_no,0);
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

	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec ("sumr",sumr_list,SUMR_NO_FIELDS, (envVarCrFind == 0) ? "sumr_id_no" : "sumr_id_no3");
	open_rec ("pogh",pogh_list,POGH_NO_FIELDS,"pogh_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("posh");
	abc_fclose ("sumr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			/*-----------------
			| Invalid printers |
			------------------*/
			print_mess ( ML (mlStdMess020) );
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("selectType"))
	{
		if (local_rec.selectType [0] == 'A')
		{
			FLD ("supplier")	=	NA;
			FLD ("gr_no")		=	NA;
			strcpy (local_rec.supplier,"      ");
			sprintf (sumr_rec.crd_name,"%40.40s"," ");
			strcpy (local_rec.gr_no,fifteenSpaces);

			DSP_FLD ("supplier");
			DSP_FLD ("supp_name");
			DSP_FLD ("gr_no");
		}
		if (local_rec.selectType [0] == 'G')
		{
			FLD ("supplier")	=	YES;
			FLD ("gr_no")		=	YES;
		}
		if (local_rec.selectType [0] == 'S')
		{
			FLD ("supplier")	=	YES;
			FLD ("gr_no")		=	NA;
			strcpy (local_rec.gr_no, fifteenSpaces);
			DSP_FLD ("gr_no");
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if (LCHECK ("supplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.supplier));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			/*----------------------
			| Supplier not on file. |
			-----------------------*/
			print_mess ( ML (mlStdMess022) );
			return (EXIT_FAILURE);
		}
		DSP_FLD ("supp_name");
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate G.R. Number. |
	-----------------------*/
	if (LCHECK ("gr_no"))
	{
		if (SRCH_KEY)
		{
			SrchPogh (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pogh_rec.co_no,comm_rec.co_no);
		pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pogh_rec.gr_no, local_rec.gr_no);
		cc = find_rec ("pogh",&pogh_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------------
			| Goods Receipt not found. |
			--------------------------*/
			print_mess ( ML (mlStdMess049) ); 
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Search for goods receipt number. |
==================================*/
void
SrchPogh (
 char *key_val)
{
	work_open ();
	save_rec ("#Goods Receipt ","#Date Raised");
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pogh_rec.gr_no,"%-15.15s",key_val);
	cc = find_rec ("pogh",&pogh_rec,GTEQ,"r");
	while (!cc && !strncmp (pogh_rec.gr_no,key_val,strlen (key_val)) && 
				  !strcmp (pogh_rec.co_no,comm_rec.co_no) && 
				  pogh_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		sprintf (err_str," Date raised : %s ",DateToString (pogh_rec.date_raised));
		cc = save_rec (pogh_rec.gr_no,err_str);
		if (cc)
			break;
		cc = find_rec ("pogh",&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	pogh_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pogh_rec.gr_no,"%-15.15s",temp_str);
	cc = find_rec ("pogh",&pogh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	int	y = (80 - strlen (local_rec.programDesc))/2;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (local_rec.programDesc,y,0,1);
		move (0,1);
		line (80);

		box (0,3,80,6);

		move (1,6);
		line (79);

		move (0,20);
		line (80);
		move (0,21);
		sprintf (err_str, ML (mlStdMess038),
						comm_rec.co_no,clip (comm_rec.co_name));
		print_at (21,0, "%s", err_str);
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
