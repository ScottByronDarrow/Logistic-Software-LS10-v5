/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (cr_sagebal.c  )                                   |
|  Program Desc  : (Supplier Select Aged Balance Report.             )|
|                  (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, pocr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 24/05/89         |
|---------------------------------------------------------------------|
|  Date Modified : (19/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (24/05/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (10/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (14/10/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (29/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : Program is used to selected aged balance report    |
|                : parameters.                                        |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
| (24/05/93)     : Updated for S/C DFT-8896.                          |
| (10/09/97)     : Updated to Multilingual Conversion. 				  |
| (14/10/97)     : Fixed MLDB error. 								  |
| (28/09/1999)   : Ported to ANSI standards. 						  |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_sagebal.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_sagebal/cr_sagebal.c,v 5.4 2002/07/25 11:17:27 scott Exp $";

#define	MCURR		 (mult_curr[0] == 'Y')

#define	SLEEP_TIME	2

#include <pslscr.h>
#include <dsp_screen.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include	"schema"

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	int 	printerNumber = 0,
			envCrCo 	  = 0;

	char 	printerString[3],
	    	mult_curr[2],
	    	branchNumber[3];

	char *	sptr;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
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

	struct	pocrRecord	pocr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	sel_curr[4];
	char 	proc_curr[4];
	char 	detail[2];
	char 	age_type[2];
	char 	curbal[2];
	char 	sel_cpy[2];
	char 	mode[2];
	long	due_date;
	char 	systemDate[11];
	char 	com_date[11];
	char 	onite[2];
	char 	onite_desc[5];
	char 	back[2];
	char 	back_desc[5];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "sel_cpy",	 4, 30, CHARTYPE,
		"U", "          ",
		" ", "C", "C(ompany) / B(ranch) ", "Enter C for company owned suppliers, B for branch",
		YES, NO,  JUSTLEFT, "CB", "", local_rec.sel_cpy},
	{1, LIN, "detail",	 5, 30, CHARTYPE,
		"U", "          ",
		" ", "S", "D(etail) / S(ummary) ", "Enter D to list invoice details",
		YES, NO,  JUSTLEFT, "DS", "", local_rec.detail},
	{1, LIN, "age_type",	 6, 30, CHARTYPE,
		"U", "          ",
		" ", "I", "P(ayment) / I(nvoice) Date ", "Enter P to age by P(ayment Date), I to age By I(nvoice date).",
		YES, NO,  JUSTLEFT, "PI", "", local_rec.age_type},
	{1, LIN, "curr",	 8, 30, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", "Enter Currency Code or <retn> defaults to ALL",
		YES, NO,  JUSTLEFT, "", "", local_rec.sel_curr},
	{1, LIN, "curr_desc",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "curbal",	 9, 30, CHARTYPE,
		"U", "          ",
		" ", "O", "L(ocal) / O(verseas) ", "Enter L to list balances in local currency, O for overseas",
		YES, NO,  JUSTLEFT, "LO", "", local_rec.curbal},
	{1, LIN, "back",	11, 30, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "back_desc",	11, 33, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onite",	12, 30, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onite_desc",	12, 33, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.onite_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*============================
| Local funcrtion prototypes |
============================*/
void	shutdown_prog	 (void);
int		RunProgram		 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
void	SrchPocr		 (char *);
int		heading			 (int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	_win_func	=	TRUE;
	if (argc != 2)
	{
		print_at (0,0, mlStdMess036, argv[0]);
		return (EXIT_FAILURE);
	}
	printerNumber  = atoi (argv[1]);

	sprintf (mult_curr, "%-1.1s", get_env ("CR_MCURR"));

	SETUP_SCR (vars);

	/*-----------------------------------------------
	| Setup screen if not multi-currency suppliers. |
	-----------------------------------------------*/
	if (!MCURR)
	{
		FLD ("curr") 			= ND;
		FLD ("curr_desc")	 	= ND;
		FLD ("curbal") 			= ND;
		SCN_ROW ("back")		= 8;
		SCN_ROW ("back_desc")	= 8;
		SCN_ROW ("onite")		= 9;
		SCN_ROW ("onite_desc")	= 9;
	}
	else
	{
		SCN_ROW ("back")		= 11;
		SCN_ROW ("back_desc")	= 11;
		SCN_ROW ("onite")		= 12;
		SCN_ROW ("onite_desc")	= 12;
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	
	envCrCo = atoi (get_env ("CR_CO"));
	strcpy (branchNumber, (envCrCo) ? comm_rec.test_no : " 0");

	strcpy (local_rec.com_date, DateToString (comm_rec.t_crd_date));

	while (prog_exit == 0) 
	{
		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		if (MCURR)
		{
			FLD ("curr") 			= YES;
			FLD ("curr_desc") 		= NA;
			FLD ("curbal") 			= YES;
			SCN_ROW ("back")		= 11;
			SCN_ROW ("back_desc")	= 11;
			SCN_ROW ("onite")		= 12;
			SCN_ROW ("onite_desc")	= 12;
		}
		else
		{
			FLD ("curr") 			= ND;
			FLD ("curr_desc") 		= ND;
			FLD ("curbal") 			= ND;
			SCN_ROW ("back")		= 8;
			SCN_ROW ("back_desc")	= 8;
			SCN_ROW ("onite")		= 9;
			SCN_ROW ("onite_desc")	= 9;
		}

		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

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

		/*--------------------------------
		| Execute aged balance report.   |
		--------------------------------*/
		strcpy (err_str, ML (mlCrMess190));
		if (!RunProgram ())
		{
			return (EXIT_SUCCESS);
		}
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

int
RunProgram (
 void)
{
	CloseDB (); 
	FinishProgram ();

	sprintf (printerString,"%d",printerNumber);
	if (!MCURR || !strcmp (local_rec.sel_curr, "ALL"))
		strcpy (local_rec.sel_curr, "AZZ");

	if (!MCURR)
		strcpy (local_rec.curbal, "L");

	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onite[0] == 'Y') 
	{
		if (fork () == 0)
			execlp ("ONIGHT",
					"ONIGHT",
					"cr_pagebal",
					printerString, 
					local_rec.sel_cpy,
					local_rec.curbal,
					local_rec.sel_curr,
					local_rec.detail,
					local_rec.age_type, 
					err_str, (char *)0);
					/*"Print Suppliers Ageing Schedule", (char *)0);*/
		return (EXIT_FAILURE);
	}

	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork () == 0)
			execlp ("cr_pagebal",
					"cr_pagebal",
					printerString, 
					local_rec.sel_cpy,
					local_rec.curbal,
					local_rec.sel_curr,
					local_rec.detail,
					local_rec.age_type,
					 (char *)0);
	}
	else 
	{
		execlp ("cr_pagebal",
				"cr_pagebal",
				printerString, 
				local_rec.sel_cpy,
				local_rec.curbal,
				local_rec.sel_curr,
				local_rec.detail,
				local_rec.age_type,
				 (char *)0);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("pocr", pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose ("pocr");
	abc_dbclose ("data");
}

/*============================================
| Special validation on screen entry.        |
============================================*/
int
spec_valid (
 int field)
{
	/*-------------------------------
	| Validate Currency Code.       |
	-------------------------------*/
	if (LCHECK ("curr"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (dflt_used && MCURR)
		{	
			strcpy (local_rec.sel_curr, "ALL");
			DSP_FLD ("curr");
			FLD ("curr_desc") = NA;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
            SrchPocr (temp_str);
		    return (EXIT_SUCCESS);
		}
		/*--------------------------------
		| Read Currency Record.          |
		--------------------------------*/
		strcpy (pocr_rec.co_no,comm_rec.tco_no);
		strcpy (pocr_rec.code, local_rec.sel_curr);
		cc = find_rec ("pocr", &pocr_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back_desc, (local_rec.back[0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("back_desc");
	    	return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.onite_desc, (local_rec.onite[0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("onite_desc");
	    return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*===============================
| Search for currency pocr code |
===============================*/
void
SrchPocr (
 char *	key_val)
{
    work_open ();

	save_rec ("#Code", "#Description");

	strcpy (pocr_rec.co_no,comm_rec.tco_no);
	sprintf (pocr_rec.code ,"%-3.3s",key_val);
	cc = find_rec ("pocr", &pocr_rec, GTEQ, "r");
    while (!cc && !strcmp (pocr_rec.co_no,comm_rec.tco_no) && 
	      !strncmp (pocr_rec.code,key_val,strlen (key_val)))
    {                        
	    cc = save_rec (pocr_rec.code, pocr_rec.description);                       
		if (cc)
		   break;
		cc = find_rec ("pocr",&pocr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	   return;
	strcpy (pocr_rec.co_no,comm_rec.tco_no);
	sprintf (pocr_rec.code,"%-3.3s",temp_str);
	cc = find_rec ("pocr", &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pocr", "DBFIND");
}

/*===============================================
| Screen Heading Display Routine.               |
===============================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		fflush (stdout);
		rv_pr (ML (mlCrMess122) ,20,0,1);

		fflush (stdout);
		move (0,1);
		line (80);

		if (MCURR)
		{
			box (0,3,80,9);
			move (1,7);line (78);
			move (1,10);line (78);
		}
		else
		{
			move (1,input_row);
			box (0,3,80,6);
			move (1,7);line (78);
		}

		move (1,20);
		line (78);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0, err_str, comm_rec.tco_no,comm_rec.tco_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
