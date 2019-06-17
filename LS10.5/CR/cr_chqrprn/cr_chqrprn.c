/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (cr_chqrprn.c   )                                  |
|  Program Desc  : (Suppliers Cheque Reprint Selection Program. )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, comr, sumr, suhd,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 27/04/89         |
|---------------------------------------------------------------------|
|  Date Modified : (05/04/90)      | Modified  by : Scott Darrow.     |
|  Date Modified : (11/07/91)      | Modified  by :                   |
|  Date Modified : (27/01/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (06/04/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (20/05/97)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (13/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (05/04/90) - Total Rewrite.                        |
|                : (11/07/91) - Updated to check sumr search.         |
|  (27/01/94)    : HGP 9846. Remove suhd_loc_disc_taken from dbview.  |
|  (06/04/96)    : PDL - Updated to change cheque length from 6-8.    |
|  (20/05/97)    : PDL - Updated to change cheque length from 8-13.   |
|  (03/09/97)    : SEL - Multilingual Conversion.                     |
|  (13/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqrprn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqrprn/cr_chqrprn.c,v 5.3 2002/07/25 11:17:26 scott Exp $";

#define	REMIT		 (remmitChequeFlag[0] == 'Y' || remmitChequeFlag[0] == 'y')
#define	COMB		 (remmitChequeFlag[1] == 'Y' || remmitChequeFlag[1] == 'y')

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include "schema"

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	int		envDbCo = 0,
			cr_find = 0;

	int		printerNumber = 0;
	int		printOk = 0;

	char 	branchNumber[3],
			remmitChequeFlag[3];

   	long 	pidNumber = 0L;

	int		supplierFound = FALSE;

	char	printerNumberString[3];
	char	pidString[10];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"},
		{"comm_gl_date"}
	};
	
	int comm_no_fields = 7;
	
	struct {
		int   termno;
		char  tco_no[3];
		char  tco_name[41];
		char  test_no[3];
		char  test_name[41];
		long  t_crd_date;
		long  t_gl_date;
	} comm_rec;

	struct	comrRecord	comr_rec;
	struct	sumrRecord	sumr_rec;
	struct	suhdRecord	suhd_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	crd_no[7];
	char 	crd_name[41];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "cred", 4, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Supplier Number", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, LIN, "name", 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Supplier Name", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.crd_name}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	(void);
void	ReadComm		 (void);
int		spec_valid		 (int field);
void	Update			 (void);
int		PrintAll		 (void);
int		RunProgram		 (char *name, char *run1, char *run2);
int		heading			 (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 3)
	{
		/*--------------------------
		|Usage %s <LPNO> <PID>\n\r |
		--------------------------*/
		print_at (0, 0, ML (mlCrMess002), argv[0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv[1]);
	pidNumber = atol (argv[2]);

	sprintf (printerNumberString,"%02d",printerNumber);
	sprintf (pidString,"%09ld",pidNumber);

	sprintf (remmitChequeFlag, "%-2.2s", get_env ("CR_REMIT"));

 	SETUP_SCR (vars);

	envDbCo = atoi (get_env ("CR_CO"));
	cr_find  = atoi (get_env ("CR_FIND"));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	ReadComm ();
	
	strcpy (branchNumber, (envDbCo) ? comm_rec.test_no : " 0");

	while (prog_exit == 0) 
	{
		/*----------------------------
		| Setup required parameters. |
		----------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();
		init_vars (1);
	
		abc_unlock ("sumr");
		abc_unlock ("suhd");
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		Update ();
	
		if (printOk)
		{
			if (PrintAll () == -1)
			{
				return (EXIT_FAILURE);
			}
		}
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

	open_rec ("sumr",sumr_list,SUMR_NO_FIELDS, (!cr_find) ? "sumr_id_no"
														: "sumr_id_no3");
	open_rec ("suhd", suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
}

/*========================
| CloseDB data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose ("sumr");
	abc_fclose ("suhd");
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadComm (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec ("comr", comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no,comm_rec.tco_no);
	cc = find_rec ("comr", &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose ("comr");
}

int
spec_valid (
 int field)
{
	/*---------------------------------
	| Validate Supplier Number Input. |
	---------------------------------*/
	if (LCHECK ("cred"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.tco_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.crd_no));
		cc = find_rec ("sumr", &sumr_rec, COMPARISON, "w");
		if (cc) 
		{
			/*-------------------------------------------------
			| Supplier %s is not on file.",sumr_rec.crd_no |
			-------------------------------------------------*/
			errmess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		supplierFound = FALSE;

		strcpy (local_rec.crd_name, sumr_rec.crd_name);
		DSP_FLD ("name");

		/*---------------------------------------
		| Check for valid cheque header 	|
		---------------------------------------*/
		cc = find_hash ("suhd",&suhd_rec,GTEQ,"r",sumr_rec.hhsu_hash);
		while (!cc && sumr_rec.hhsu_hash == suhd_rec.hhsu_hash)
		{
			if (suhd_rec.pid == pidNumber) 
				supplierFound = TRUE;

			cc = find_hash ("suhd",&suhd_rec,NEXT,"r",sumr_rec.hhsu_hash);
		}
		if (!supplierFound)
		{
			/*-------------------------------------
			| No cheques on file for supplier %s .|
			-------------------------------------*/
			errmess (ML (mlCrMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}
/*=============================
| Update suhd as not printed. |
=============================*/
void
Update (
 void)
{
	clear ();
	/*-------------------------------------------
	|Flagging Supplier %s cheques as unprinted. |
	-------------------------------------------*/
	print_at (0,0, ML (mlCrMess158), sumr_rec.crd_no);
	fflush (stdout);

	cc = find_hash ("suhd",&suhd_rec,GTEQ,"u",sumr_rec.hhsu_hash);
	while (!cc && sumr_rec.hhsu_hash == suhd_rec.hhsu_hash)
	{
		if (suhd_rec.pid == pidNumber) 
		{
			strcpy (suhd_rec.stat_flag, "C");

			if (REMIT)
				strcpy (suhd_rec.rem_prt, "R");

			cc = abc_update ("suhd", &suhd_rec);
			if (cc)
				file_err (cc, "suhd", "DBUPDATE");

			printOk = TRUE;
		}
		abc_unlock ("suhd");

		cc = find_hash ("suhd",&suhd_rec,NEXT,"u",sumr_rec.hhsu_hash);
	}
	abc_unlock ("suhd");
}

int
PrintAll (
 void)
{
	/*-------------------------
	| Print seperate Cheques. |
	-------------------------*/
	if (COMB)
	{
		if (RunProgram ("cr_rc_prn",printerNumberString, pidString))
			return (-1);
	}
	else
	{
		if (RunProgram ("cr_chqprn",printerNumberString, pidString))
			return (-1);
	}
	return (EXIT_SUCCESS);
}
/*===================
| Execute Programs. |
===================*/
int
RunProgram (
 char *	name,
 char *	run1,
 char *	run2)
{
 	char szBuff [100];
	strcpy (szBuff, name);
	strcat (szBuff, " ");
	strcat (szBuff, run1);
	strcat (szBuff, " ");
	strcat (szBuff, run2);
	sys_exec (szBuff);

 	return (EXIT_SUCCESS);
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

	/*-----------------------------------
	| Supplier Cheque Reprint Selection.|
	-----------------------------------*/
	rv_pr (ML (mlCrMess155),22,0,1);

	fflush (stdout);
	move (0,1);
	line (80);

	move (1,input_row);
	box (0,3,80,2);

	move (1,20);
	line (80);

	print_at (21,0, ML (mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.test_no,comm_rec.test_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
