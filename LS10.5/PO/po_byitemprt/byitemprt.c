/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: byitemprt.c,v 5.5 2002/07/17 09:57:32 scott Exp $
|  Program Name  : (po_byitemprt.c)                                   |
|  Program Desc  : (Print Backlog by Item No.                   )     |	
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, pohr, poln, inmr, inex,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 14/01/88         |
|---------------------------------------------------------------------|
|  Date Modified : (14/01/88)      | Modified  by  : Fui Choo Yap.    |
|                : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|                : (14/11/88)      | Modified  by  : Fui Choo Yap.    |
|                : (09/12/88)      | Modified  by  : Bee Chwee Lim.   |
|                : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|                : (24/04/89)      | Modified  by  : Scott Darrow.    |
|                : (13/09/90)      | Modified  by  : Scott Darrow.    |
|                : (07/07/92)      | Modified  by  : Simon Dubey.     |
|                : (13/11/92)      | Modified  by  : Anneliese Allen. |
|                : (10/02/94)      | Modified  by  : Campbell Mander. |
|                : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                : (14/10/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Print PNAME at top right of report.                |
|                : Modified program to allow a range of items to be   |
|                : selected as in so_byitemprt.c                      |
|                : Change Printing of land_cost from DOLLARS.         |
|                : (24/04/89) - Added test to ignore -ve Po (s).      |
|                : (13/09/90) - General Update.                       |
|                : (07/07/92) - To include inex desc lines SC DFH 7287|
|                :              defaults for start/end item & fix     |
|                :              first time logic in PrintPoln.        |
|                : (13/11/92) - DFH___ 7606 Remove printing of inex   |
|                : lines when no poln records.                        |
|  (10/02/94)    : PSL 10413. Change no_lps () to valid_lp ().        |
|  (13/09/97)    : Updated for Multilingual Conversion.				  | 
|  (14/10/97)    : Fixed MLDB error.								  | 
|                :                                                    |
|                                                                     |
| $Log: byitemprt.c,v $
| Revision 5.5  2002/07/17 09:57:32  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/08 03:29:42  scott
| S/C 4046 Error message when inputting an erroneous or invalid value are not visible.
|
| Revision 5.3  2001/10/19 02:50:48  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:15:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:04  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/16 08:05:07  scott
| Updated to fix screen selection to work correctly with LS10-GUI
| Updated / Cleaned code and added app.schema
|
| Revision 4.1  2001/03/15 03:36:24  scott
| Updated to add sleep (2) for warning messages - LS10-GUI.
|
| Revision 4.0  2001/03/09 02:32:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:12  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:04:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/07/10 01:52:37  scott
| Updated to replace "@(" with "@(" to ensure psl_what works correctly
|
| Revision 1.14  2000/06/13 05:02:16  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.13  1999/12/06 01:32:29  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/11 06:43:12  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.11  1999/11/05 05:17:07  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.10  1999/10/14 03:04:20  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/09/29 10:11:51  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:37:54  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:14  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: byitemprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_byitemprt/byitemprt.c,v 5.5 2002/07/17 09:57:32 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inexRecord	inex_rec;
struct inumRecord	inum_rec;

	int		foundPoln 		= FALSE,
			printTotal 		= FALSE,
			firstTime 		= TRUE,
			partialDetails 	= 0,
			lineNumber 		= 0;
	extern	int		TruePosition;

	float	quantity 		= 0.0,
			totalPoQty 		= 0.0,
			totalGrandQty 	= 0.0;

	double	extend 			= 0.00,
			totalPoShip 	= 0.00,
			totalGransShip 	= 0.00;

	FILE	*fout;

	float	StdCnvFct 	= 1.00;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startItem [17];
	char	endItem [17];
	char	itemDesc [2] [41];
	int		printerNumber;
	char	printerString [3];
	char 	back [2];
	char 	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from_item",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Start Item     ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "desc1",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc [0]},
	{1, LIN, "to_item",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "End   Item     ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "desc2",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc [1]},
	{1, LIN, "printerNumber",	 7, 2, INTTYPE,
		"NN", "          ",
		" ", "1","Printer No     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background     ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 8, 22, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.backDesc},
	{1, LIN, "onite",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight      ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	 9, 22, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "YN", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	RunProgram 		(char *);
void 	HeadingOutput 	(void);
void 	PrintLine 		(void);
void 	ProcessFile 	(void);
void 	ProcessPoln 	(long);
void 	PrintPoln 		(long);
void 	PrintTotalPart 	(void);
int 	heading 		(int);
void 	PrintInex 		(int, int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	TruePosition	=	TRUE;
		
	if (argc != 1 && argc != 4)
	{
		print_at (0,0, mlPoMess723,argv [0]);
        return (EXIT_FAILURE);
	}

	OpenDB ();

	if (argc == 4)
	{
		sprintf (local_rec.startItem,"%-16.16s",argv [1]);
		sprintf (local_rec.endItem,"%-16.16s",argv [2]);
		local_rec.printerNumber = atoi (argv [3]);

		dsp_screen ("Processing : Printing Purchase Order By Item No.",
					comm_rec.co_no,comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in opening pformat DURING (POPEN)",errno,PNAME);

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*-----------------------------
		| Entry screen 1 linear input |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy (err_str, ML (mlPoMess221));
		RunProgram (argv [0]);
		prog_exit = 1;
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
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (inum);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("from_item")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.startItem,"                ");
			strcpy (local_rec.itemDesc [0], ML ("START ITEM"));
			DSP_FLD ("from_item");
			DSP_FLD ("desc1");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.itemDesc [0],inmr_rec.description);
		DSP_FLD ("from_item");
		DSP_FLD ("desc1");

		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("to_item")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItem,"~~~~~~~~~~~~~~~~");
			strcpy (local_rec.itemDesc [1], ML ("END ITEM"));
			DSP_FLD ("to_item");
			DSP_FLD ("desc2");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (strcmp (local_rec.startItem,local_rec.endItem) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endItem, inmr_rec.item_no);
		strcpy (local_rec.itemDesc [1],inmr_rec.description);

		DSP_FLD ("to_item");
		DSP_FLD ("desc2");

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
				(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.oniteDesc,
				(local_rec.onite [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
 char *prog_name)
{
	sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
	
	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.startItem,
				local_rec.endItem,
				local_rec.printerString,
				err_str, (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.startItem,
				local_rec.endItem,
				local_rec.printerString, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.startItem,
			local_rec.endItem,
			local_rec.printerString, (char *)0);
	}
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNumber);

	fprintf (fout,".15\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".ECompany : %s - %s\n",comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".EBranch  : %s - %s\n",comm_rec.est_no,clip (comm_rec.est_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EPURCHASE ORDER BY PART NUMBER\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout,".R===========================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"========================================================\n");

	fprintf (fout,"===========================================");
	fprintf (fout,"==================================================");
	fprintf (fout,"========================================================\n");

	fprintf (fout,"|       PART       ");
	fprintf (fout,"|               DESCRIPTION                ");
	fprintf (fout,"|   DATE   ");
	fprintf (fout,"|  CATEGORY   ");
	fprintf (fout,"| UOM. ");
	fprintf (fout,"| QUANTITY ");
	fprintf (fout,"|    LANDED   ");
	fprintf (fout,"|  SUPPLIER  ");
	fprintf (fout,"|      P.O.     |\n");

	fprintf (fout,"|      NUMBER      ");
	fprintf (fout,"|                                          ");
	fprintf (fout,"|   DUE    ");
	fprintf (fout,"|   NUMBER    ");
	fprintf (fout,"|      ");
	fprintf (fout,"| OUTSTDNG ");
	fprintf (fout,"|     COST    ");
	fprintf (fout,"|  ACRONYM   ");
	fprintf (fout,"|    NUMBER     |\n");

	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|---------------|\n");
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|---------------|\n");

	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",local_rec.startItem);

	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
		       strcmp (inmr_rec.item_no,local_rec.startItem) >= 0 && 
		       strcmp (inmr_rec.item_no,local_rec.endItem) <= 0)
	{
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		dsp_process ("Item No. : ",inmr_rec.item_no);
		ProcessPoln (inmr_rec.hhbr_hash);
		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}

	/*-----------------------------------------------
	| On last record,print the total backlog.	|
	-----------------------------------------------*/
	if (printTotal)
	{
		fprintf (fout,"| ***** TOTAL PURCHASE ORDER ***** ");
		fprintf (fout,"                           ");
		fprintf (fout,"|          ");
		fprintf (fout,"|             ");
		fprintf (fout,"|      ");
		fprintf (fout,"|%9.2f ",totalGrandQty);
		fprintf (fout,"|%12.2f ",totalGransShip);
		fprintf (fout,"|            ");
		fprintf (fout,"|               |\n");
	}
	fflush (fout);
}

void
ProcessPoln (
 long hhbr_hash)
{
	float	quantityLeft = 0.00;

	totalPoQty = 0.0;
	totalPoShip = 0.0;

	cc = find_hash ("poln",&poln_rec,GTEQ,"r",hhbr_hash);
	firstTime = TRUE;
	lineNumber = 0;
	while (!cc && poln_rec.hhbr_hash == hhbr_hash) 
	{
		quantityLeft = poln_rec.qty_ord - poln_rec.qty_rec;
		if (quantityLeft > 0.00)
		{
			
			cc = find_hash ("pohr",&pohr_rec,COMPARISON,"r",poln_rec.hhpo_hash);
			if (cc || pohr_rec.status [0] == 'D')
				return;

			PrintPoln (hhbr_hash);
		}	
		cc = find_rec ("poln",&poln_rec,NEXT,"r");
	}
	if (foundPoln)
		PrintInex (lineNumber, TRUE);
	foundPoln = FALSE;

	if (partialDetails > 1)
	{
		PrintTotalPart ();
		PrintLine ();
		partialDetails = 0;
	}

	if (partialDetails == 1)
	{
		PrintLine ();
		partialDetails = 0;
	}
}

void
PrintPoln (
 long hhbr_hash)
{
	float	PurCnvFct 	= 0.00,
			CnvFct		= 0.00;

	foundPoln = TRUE;
	printTotal = TRUE;
	quantity = poln_rec.qty_ord - poln_rec.qty_rec;

	extend = twodec (poln_rec.land_cst);
	extend = out_cost (extend,inmr_rec.outer_size);
	extend *= (double) quantity;

	totalPoQty += quantity;
	totalPoShip += extend;

	totalGrandQty += quantity;
	totalGransShip += extend;

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	CnvFct	=	StdCnvFct / PurCnvFct;

	cc = find_hash ("sumr",&sumr_rec,COMPARISON,"r",pohr_rec.hhsu_hash);
	if (cc)
		strcpy (sumr_rec.acronym,"Unknown");

	if (firstTime)
	{
		fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
		fprintf (fout,"| %-40.40s ",poln_rec.item_desc);
		firstTime = FALSE;
	}
	else
	{
		fprintf (fout,"|%-18.18s"," ");
		PrintInex (lineNumber,FALSE);
		lineNumber++;
	}

 	fprintf (fout,"|%-10.10s",DateToString (poln_rec.due_date));
 	fprintf (fout,"| %-11.11s ",inmr_rec.category);
	fprintf (fout,"| %4.4s ",	inum_rec.uom);
	fprintf (fout,"|%9.2f ",quantity * CnvFct);
	fprintf (fout,"|%12.2f ",extend);
	fprintf (fout,"|  %-9.9s ",sumr_rec.acronym);
	fprintf (fout,"|%15.15s|\n",pohr_rec.pur_ord_no);
	fflush (fout);

	partialDetails++;
}

/*-------------------------
| print total for a part. |
-------------------------*/
void
PrintTotalPart (
 void)
{
	fprintf (fout,".LRP2\n");
	PrintLine ();
	fprintf (fout,"|  TOTAL FOR PART    :  %-16.16s ",inmr_rec.item_no);
	fprintf (fout,"                     ");
	fprintf (fout,"|          |             ");
	fprintf (fout,"|      ");
	fprintf (fout,"|%9.2f ",totalPoQty);
	fprintf (fout,"|%12.2f ",totalPoShip);
	fprintf (fout,"|            ");
	fprintf (fout,"|               |\n");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlPoMess129),25,0,1);
			
		line_at (1,0,80);

		box (0,3,80,6);
		line_at (6,1,79);
		line_at (20,0,80);
		print_at (21,0, ML (mlStdMess038) ,comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039) ,comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintInex (
 int lineNumber, 
 int FLAG)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = lineNumber;

	if (FLAG)
	{
		cc = find_rec ("inex", &inex_rec, GTEQ, "r");
		while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			fprintf (fout,"|%-18.18s"," ");
			fprintf (fout,"| %-40.40s ", inex_rec.desc);
		 	fprintf (fout,"| %-8.8s "," ");
			fprintf (fout,"| %-11.11s "," ");
			fprintf (fout,"|      ");
			fprintf (fout,"| %8.2s "," ");
			fprintf (fout,"| %11.2s "," ");
			fprintf (fout,"|  %-9.9s "," ");
			fprintf (fout,"|%-15.15s|\n"," ");
			fflush (fout);
			cc = find_rec ("inex", &inex_rec, NEXT, "r");
		}
	}

	if (!FLAG)
	{
		cc = find_rec ("inex", &inex_rec, COMPARISON, "r");

		if (cc)
		{
			fprintf (fout,"| %-40.40s ", " ");
			return;
		}

		if (inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			fprintf (fout,"| %-40.40s ", inex_rec.desc);
		}
	}
}
