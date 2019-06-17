/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: shipschd.c,v 5.5 2002/07/17 09:57:40 scott Exp $
|  Program Name  : (po_shipschd.c )                                   |
|  Program Desc  : (Purchase Order Shipping Schedule.           )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (30/07/86)      | Modified  by  : Vicki Seal.      |
|  Date Modified : (08/09/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (21/10/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|  Date Modified : (03/12/91)      | Modified  by  : Campbell Mander  |
|  Date Modified : (24/08/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (14/10/97)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (10/11/1997)    | Modified  by  : Jiggs A Veloz.   |
|                                                                     |
|  Comments      : Changed d/b to data.                               |
|                : Removed pohr_ind* fields.                          |
|                : (17/04/89) Changed MONEYTYPEs to DOUBLETYPEs.      |
|                :                                                    |
| (03/12/91)    : Combined report and input programs.                 |
|                : Added display option                               |
|                :                                                    |
| (24/08/93)    : HGP 9485 Updated for shipment method.               |
|                :                                                    |
| (14/10/97)    : Added posh_csm_no                                   |
| (10/11/1997)  : SEL - Updated for multilingual.                     |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: shipschd.c,v $
| Revision 5.5  2002/07/17 09:57:40  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/07/08 10:22:16  kaarlo
| S/C 4020. Added delay to Start Period and End Period.
|
| Revision 5.3  2002/02/26 06:02:19  scott
| S/C 00776 POST10 - Print Shipping Schedule:  OUTPUT TO and ORDER DETAIL fields greyed out even if they are supposed to be editable
|
| Revision 5.2  2001/08/09 09:16:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:23  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:47  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:18:08  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:36  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/05 05:17:18  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.11  1999/10/14 03:04:28  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.10  1999/09/29 10:12:14  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/21 04:38:12  scott
| Updated from Ansi project
|
| Revision 1.8  1999/06/17 10:06:39  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: shipschd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_shipschd/shipschd.c,v 5.5 2002/07/17 09:57:40 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#define	NOHEADLINES	5

#define	DISP		 (local_rec.displayPrint [0] == 'D')
#define	DETAIL 		 (orderDetail [0] == 'Y')

char	*UNDERLINE = "^^GGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGG";

char  *HEAD2 =      "SUPP ACR |BR|PURCHASE ORDER |      COMMENTS    |INVOICE NUMBER | LN |    ITEM NO     |  DESCRIPTION            |   QTY.   | UOM  ";
char  *UNDERLINE3 = "^^GGGGGGGGGHGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGHGGGGGG^^";
char  *UNDERLINE2 = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGG^^";

char  *HEAD3 = " SUPP ACR. | BR |PURCHASE ORDER |                COMMENTS                  | INVOICE NUMBER  ";

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	int	output_open = FALSE;

	long	startDate;
	long	endDate;

	char	orderDetail [2];

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct poslRecord	posl_rec;
struct inumRecord	inum_rec;

	char	disp_str [200];
	char	disp1_str [200];

extern	int		EnvScreenOK;
/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	printerString [3];
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	prog_desc [61];
	char	back [5];
	char	backDesc [5];
	char	onight [5];
	char	onightDesc [5];
	int		printerNumber;
	char	displayPrint [8];
	char	disp_prn_desc [8];
	char	startDate [11];
	char	endDate [11];
	long	start;
	long	end;
	char	orderDetail [5];
	char	orderDetailDesc [5];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "start_period",	 4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Start Period     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start},
	{1, LIN, "end_period",	 5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "End Period       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end},
	{1, LIN, "orderDetail",	6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Order Detail     ", "Show Order detail Y(es or N(o.",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.orderDetail},
	{1, LIN, "orderDetailDesc",	6, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.orderDetailDesc},

	{1, LIN, "displayPrint",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "D", "Output To        ", " Output To D(isplay) / P(rinter) ",
		YES, NO, JUSTLEFT, "DP", "", local_rec.displayPrint},
	{1, LIN, "disp_prn_desc",	8, 22, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.disp_prn_desc},
	{1, LIN, "printerNumber",	 9, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	 10, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 10, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.backDesc},
	{1, LIN, "onight", 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc", 11, 22, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.onightDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	RunProgram 			(char *, char *);
int 	spec_valid 			(int);
void 	HeadingOutput 		(void);
void 	ProcessFile 		(void);
void 	ProcessPosh 		(void);
int 	FindSupplier		(long);
int 	ProcessPosd 		(char *);
void 	PrintHeading 		(void);
int	 	ProcessPoln 		(long, long);
int 	heading 			(int);
void	ShipmentHeader 		(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{

	if (argc != 2 && argc != 6)
	{
		sprintf (err_str, "%s\n\r", mlPoMess700);
		print_at (0,0, err_str, argv [0]);
		sprintf (err_str, "%s\n\r", mlPoMess701);
		print_at (1,0, err_str, argv [0]);
		print_at (2,0, "%s\n", mlPoMess702);
		print_at (3,0, "%s\n", mlPoMess703);
		print_at (4,0, "%s\n", mlPoMess704);
		print_at (5,0, "%s\n", mlPoMess705);
		return (EXIT_FAILURE);
	}

	EnvScreenOK	=	TRUE;
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	init_scr ();
	set_tty ();

	if (argc == 2)
	{
		TruePosition	=	TRUE;

		SETUP_SCR (vars);

		/*---------------------------
		| Setup required parameters |
		---------------------------*/
		set_masks ();
		init_vars (1);

        OpenDB ();
	
		/*=====================
		| Reset control flags |
		=====================*/
   		entry_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		restart 	= FALSE;
   		search_ok 	= TRUE;
		init_vars (1);
	
		/*-----------------------------
		| Enter screen 1 linear input |
		-----------------------------*/
		heading (1);
		entry (1);
        if (prog_exit || restart) {
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
	
		RunProgram (argv [0], argv [1]);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*-------------------
	| Printer Number	|
	-------------------*/
	local_rec.printerNumber = atoi (argv [1]);
	startDate = StringToDate (argv [2]);

	endDate   = StringToDate (argv [3]);
	if (endDate < 0L)
	{
		print_at (6,0, ML ("Error in endDate\007\n\r"));
        return (EXIT_FAILURE);
	}

	strcpy (orderDetail,argv [4]);
	if (orderDetail [0] != 'Y' && orderDetail [0] != 'N')
	{
		print_at (6,0, ML ("Error in orderDetail - should be Yes or No"));
        return (EXIT_FAILURE);
	}

	sprintf (local_rec.displayPrint, "%-1.1s", argv [5]);

	OpenDB ();

	if (!DISP)
		dsp_screen ("Printing Shipping Schedule ", comm_rec.co_no, comm_rec.co_name);

	HeadingOutput ();

	ProcessFile ();

	if (DISP)
	{
		Dsp_srch_fn (ProcessPosd);
		Dsp_close ();
	}
	else
	{
		if (output_open)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
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

	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no2");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (posl);
	abc_fclose (inum);
	abc_dbclose ("data");
}

void
RunProgram (
	char *prog_name, 
	char *prog_desc)
{
	
	strcpy (local_rec.startDate, DateToString (local_rec.start));
	strcpy (local_rec.endDate, DateToString (local_rec.end));
	sprintf (local_rec.printerString, "%2d", local_rec.printerNumber);

	shutdown_prog ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (!DISP && local_rec.onight [0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.printerString,
					local_rec.startDate,
					local_rec.endDate,
					local_rec.orderDetail,
					local_rec.displayPrint,
					prog_desc, (char *)0);
		}
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (!DISP && local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp (prog_name,
					prog_name,
					local_rec.printerString,
					local_rec.startDate,
					local_rec.endDate,
					local_rec.orderDetail,
					local_rec.displayPrint, (char *)0);
		}
	}
	else 
	{
		execlp (prog_name,
				prog_name,
				local_rec.printerString,
				local_rec.startDate,
				local_rec.endDate,
				local_rec.orderDetail, 
				local_rec.displayPrint, (char *)0);
	}
}

int
spec_valid (
 int field)
{
	if (LCHECK ("displayPrint"))
	{
		if (local_rec.displayPrint [0] == 'D')
		{
			strcpy (local_rec.disp_prn_desc, ML ("Display"));
			strcpy (local_rec.onight, "N");
			strcpy (local_rec.back, "N");
			FLD ("printerNumber") = NA;
			FLD ("onight") = NA;
			FLD ("back") = NA;
		}
		else
		{
			strcpy (local_rec.disp_prn_desc, ML ("Printer"));
			FLD ("printerNumber") = NO;
			FLD ("onight") = NO;
			FLD ("back") = NO;
		}

		DSP_FLD ("disp_prn_desc");
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc,(local_rec.back [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc,(local_rec.onight [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("orderDetail"))
	{
		strcpy (local_rec.orderDetailDesc, (local_rec.orderDetail [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("orderDetailDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_period"))
	{
		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (local_rec.start > local_rec.end)
		{
			print_mess (ML (mlStdMess057));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_period"))
	{
		if (local_rec.start > local_rec.end)
		{
			print_mess (ML (mlStdMess058));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	if (DISP)
	{
		heading (2);
		Dsp_open (0, 3, 15);
		Dsp_saverec (" SHIPMENT NO| DEPATURE | ARRIVAL  |METH|    VESSEL     |CURR|PORT OF ORIGIN|  DESTINATION  |DOC. REC. |DOC AGENT | B.O.L NUMBER  ");
		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW] [NEXT SCN] [PREV SCN] [EDIT/END] ");
	}
	else
	{
		if ((fout = popen ("pformat","w")) == NULL)
			sys_err ("Error in opening pformat During (DBPOPEN)", errno, PNAME);

		output_open = TRUE;

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	
		fprintf (fout, ".PI16\n");
		fprintf (fout, ".8\n");
		fprintf (fout, ".L170\n");
		fprintf (fout, ".B1\n");
		fprintf (fout, ".E%s SHIPPING  SCHEDULE\n", clip (comm_rec.co_name));
		fprintf (fout, ".B1\n");
		fprintf (fout, ".EREPORT COVERS PERIOD FROM %s ",DateToString (startDate));
		fprintf (fout, "TO %s\n",DateToString (endDate));
		fprintf (fout, ".B1\n");
	
		fprintf (fout, ".R=============");
		fprintf (fout, "===========");
		fprintf (fout, "===========");
		fprintf (fout, "=====");
		fprintf (fout, "=====================");
		fprintf (fout, "======");
		fprintf (fout, "=====================");
		fprintf (fout, "=====================");
		fprintf (fout, "===========");
		fprintf (fout, "===========");
		fprintf (fout, "===========");
		fprintf (fout, "=============");
		fprintf (fout, "============\n");
	}
}

void
ProcessFile (
 void)
{
	char	saveDesc [23];

	strcpy (posh_rec.co_no,comm_rec.co_no);
	posh_rec.ship_arrive = startDate;
	posh_rec.hhsh_hash = 0L;

	cc = find_rec ("posh",&posh_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp (posh_rec.co_no,comm_rec.co_no) && 
	       posh_rec.ship_arrive >= startDate && 
	       posh_rec.ship_arrive <= endDate)
	{
		ProcessPosh ();

		if (!DISP)
		{
			sprintf (saveDesc,"%s%010ld",posh_rec.csm_no,posh_rec.hhsh_hash);
			ProcessPosd (saveDesc);
		}

		cc = find_rec ("posh",&posh_rec,NEXT,"r");
	}

	if (DISP)
		Dsp_saverec (UNDERLINE);

}

/*===========================
| Print the ship details	|
===========================*/
void
ProcessPosh (
 void)
{
	char	tmp_depart [11];
	char	tmp_arrive [11];
	char	tmp_doc_rec [11];
	char	tmp_doc_agent [11];
	char	tmp_neg_bol [11];
	char	tmp_cost_date [11];
	char	saveDesc [23];
	char	dsp_method [5];

	sprintf (tmp_depart, 	"%-10.10s", 
		 (posh_rec.ship_depart <= 0L) 		? " " : DateToString (posh_rec.ship_depart));
	sprintf (tmp_arrive, 	"%-10.10s", 
		 (posh_rec.ship_arrive <= 0L) 		? " " : DateToString (posh_rec.ship_arrive));
	sprintf (tmp_doc_rec, 	"%-10.10s", 
		 (posh_rec.doc_rec <= 0L) 		? " " : DateToString (posh_rec.doc_rec));
	sprintf (tmp_doc_agent, 	"%-10.10s", 
		 (posh_rec.doc_agent <= 0L) 		? " " : DateToString (posh_rec.doc_agent));
	sprintf (tmp_neg_bol, 	"%-10.10s", 
		 (posh_rec.neg_bol <= 0L) 		? " " : DateToString (posh_rec.neg_bol));
	sprintf (tmp_cost_date, 	"%-10.10s", 
		 (posh_rec.costing_date <= 0L) 	? " " : DateToString (posh_rec.costing_date));

	if (posh_rec.ship_method [0] == 'A')
		strcpy (dsp_method, "AIR ");
	else if (posh_rec.ship_method [0] == 'S')
		strcpy (dsp_method, "SEA ");
	else if (posh_rec.ship_method [0] == 'L')
		strcpy (dsp_method, "LAND");
	else 
		strcpy (dsp_method, "????");

	if (DISP)
	{
		sprintf (disp_str,
			"%-12.12s^E%-10.10s^E%-10.10s^E%-4.4s^E%-15.15s^E %3.3s^E%-14.14s^E%-15.15s^E%-10.10s^E%-10.10s^E%-11.11s",
			posh_rec.csm_no,
			tmp_depart,
			tmp_arrive,
			dsp_method,
			posh_rec.vessel,
			posh_rec.curr_code,
			posh_rec.port,
			posh_rec.destination,
			tmp_doc_rec,
			tmp_doc_agent,
			posh_rec.bol_no);
	
		sprintf (saveDesc, "%s%010ld", posh_rec.csm_no, posh_rec.hhsh_hash);

		Dsp_save_fn (disp_str, saveDesc);
	}
	else
	{
		sprintf (err_str,"%-12.12s",posh_rec.csm_no);
		dsp_process (" Ship : ",err_str);

		fprintf (fout, ".LRP6\n");

		ShipmentHeader ();
		fprintf (fout, "|%12.12s", 		posh_rec.csm_no);
		fprintf (fout, " %10.10s", 		tmp_depart);
		fprintf (fout, " %10.10s", 		tmp_arrive);
		fprintf (fout, " %4.4s", 		dsp_method);
		fprintf (fout, " %20.20s",		posh_rec.vessel);
		fprintf (fout, "  %3.3s ",		posh_rec.curr_code);
		fprintf (fout, " %20.20s",		posh_rec.port);
		fprintf (fout, " %20.20s",		posh_rec.destination);
		fprintf (fout, " %10.10s",		tmp_doc_rec);
		fprintf (fout, " %10.10s",		tmp_doc_agent);
		fprintf (fout, " %10.10s",		tmp_neg_bol);
		fprintf (fout, " %12.12s", 		posh_rec.bol_no);
		fprintf (fout, " %10.10s|\n",	tmp_cost_date);
		fflush (fout);
	}
}

/*---------------------------
| Read the supplier record	|
---------------------------*/
int
FindSupplier (
 long hhsu_hash)
{
	sumr_rec.hhsu_hash = hhsu_hash;
	return (find_rec ("sumr",&sumr_rec,COMPARISON,"r"));
}


/*---------------------------------------
| Process the shipment detail record	|
---------------------------------------*/
int
ProcessPosd (
 char *saveDesc)
{
	int		first 		= TRUE,
			underline	= FALSE,
			someDetails = 0;

	long	hhshHash	= 0L;
	char	csmNumber [13];

	sprintf (csmNumber, "%-12.12s", saveDesc);
	hhshHash = atol (saveDesc + 12);

	if (DISP)
	{
		Dsp_open ((DETAIL) ? 0 : 10,3,15);
		if (DETAIL)
			Dsp_saverec (HEAD2);
		else
			Dsp_saverec (HEAD3);

		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW] [NEXT SCN] [PREV SCN] [EDIT/END] ");

		if (DETAIL)
		{
			sprintf (err_str, "SHIPMENT NUMBER : ^1 %s ^6", csmNumber);
			Dsp_saverec (err_str);
		}
	}

	strcpy (posd_rec.co_no, comm_rec.co_no);
	posd_rec.hhsh_hash = hhshHash;
	posd_rec.hhpo_hash = 0L; 

	cc = find_rec ("posd", &posd_rec, GTEQ, "r");

	while (!cc && posd_rec.hhsh_hash == hhshHash)
	{
		pohr_rec.hhpo_hash = posd_rec.hhpo_hash;
		cc = find_rec ("pohr", &pohr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec ("posd",&posd_rec,NEXT,"r");
			continue;
		}
		if (DETAIL)
		{
			cc = FindSupplier (pohr_rec.hhsu_hash);
			if (cc)
				strcpy (sumr_rec.acronym,"         ");

			if (first)
				PrintHeading ();

			first 	=	FALSE;

			if (DISP)
			{
				sprintf (disp_str,
					"%-9.9s^E%2.2s^E%-15.15s^E%-18.18s^E%-15.15s",
					sumr_rec.acronym,
					pohr_rec.br_no,
					pohr_rec.pur_ord_no,
					posd_rec.comment,
					posd_rec.inv_no);
			}
			else
			{
				fprintf (fout, "|%-9.9s",	sumr_rec.acronym);
				fprintf (fout, " %-2.2s",	pohr_rec.br_no);
				fprintf (fout, " %-15.15s",	pohr_rec.pur_ord_no);
				fprintf (fout, " %-39.39s", posd_rec.comment);
				fprintf (fout, " %-15.15s ",	posd_rec.inv_no);
			}

			underline = ProcessPoln (posd_rec.hhpo_hash, hhshHash);
			if (DISP && underline)
				Dsp_saverec (UNDERLINE3);

			if (!DISP && !underline)
			{
				fprintf (fout, "   ");
				fprintf (fout, "                 ");
				fprintf (fout, "                                         ");
				fprintf (fout, "              ");
				fprintf (fout, "     |\n");
			}

			someDetails += underline;
		}
		else
		{
			if (first)
			{
				cc = FindSupplier (pohr_rec.hhsu_hash);
				if (cc)
					strcpy (sumr_rec.acronym,"         ");

				if (!DISP)
				{
					fprintf (fout, " Supp Acr.");
					fprintf (fout, " Br");
					fprintf (fout, " Purchase Order ");
					fprintf (fout, " Comments                               ");
					fprintf (fout, " Invoice Number ");
					fprintf (fout, "    ");
					fprintf (fout, "                 ");
					fprintf (fout, "                                         ");
					fprintf (fout, "              ");
					fprintf (fout, "     |\n");

					fprintf (fout, "|---------");
					fprintf (fout, " --");
					fprintf (fout, " -------------- ");
					fprintf (fout, " --------                               ");
					fprintf (fout, " -------------- ");
					fprintf (fout, "    ");
					fprintf (fout, "                 ");
					fprintf (fout, "                                         ");
					fprintf (fout, "              ");
					fprintf (fout, "     |\n");
				}
				first = FALSE;
			}

			if (DISP)
			{
				sprintf (disp_str,
					" %-9.9s ^E %2.2s ^E%-15.15s^E %-40.40s ^E %-15.15s ",
					sumr_rec.acronym,
					pohr_rec.br_no,
					pohr_rec.pur_ord_no,
					posd_rec.comment,
					posd_rec.inv_no);

				Dsp_saverec (disp_str);
			}
			else
			{
				fprintf (fout, "|%9.9s",	sumr_rec.acronym);
				fprintf (fout, " %2.2s",	pohr_rec.br_no);
				fprintf (fout, " %15.15s",	pohr_rec.pur_ord_no);
				fprintf (fout, " %-39.39s", posd_rec.comment);
				fprintf (fout, " %15.15s",	posd_rec.inv_no);
				fprintf (fout, "    ");
				fprintf (fout, "                 ");
				fprintf (fout, "                                         ");
				fprintf (fout, "              ");
				fprintf (fout, "     |\n");
				fflush (fout);
			}
		}
		cc = find_rec ("posd",&posd_rec,NEXT,"r");
	}

	if (DISP)
	{
		if (!someDetails && DETAIL)
			Dsp_saverec (ML ("No detail lines could be found for shipment"));
		Dsp_srch ();
		Dsp_close ();
	}
	else
	{
		if (!someDetails && DETAIL)
		{
			fprintf (fout, "|   ");
			fprintf (fout, "                 ");
			fprintf (fout, "                                         ");
			fprintf (fout, "              ");
			fprintf (fout, "     |\n");
		}
	}
    return (EXIT_SUCCESS);
}

void
PrintHeading (
 void)
{
	if (DISP)
		return;

	fprintf (fout, "|Supp Acr. Br Purchase Order  Comments                                Invoice Number  Ln. Item Number      Description                              Quantity      UOM.|\n");
	fprintf (fout, "|--------- -- --------------  --------                                --------------  --- -----------      -----------                              --------      ----|\n");
}

/*-------------------------------
| Print the order detail lines	|
-------------------------------*/
int
ProcessPoln (
	long 	hhpoHash, 
	long 	hhshHash)
{
	int		first,
			line_no = 1,
			printed = 0;

	char	tempString [120];

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00,
			out_stand	= 0.00;

	first = TRUE;

	poln_rec.hhpo_hash 	= hhpoHash;
	poln_rec.line_no 	= 0;
	cc = find_rec ("poln",&poln_rec,GTEQ,"r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
	   	cc = find_rec ("inmr",&inmr_rec,EQUAL,"r");
	   	if (cc)
			strcpy (inmr_rec.item_no,"Unknown Item.");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;

		posl_rec.hhpl_hash = poln_rec.hhpl_hash;
		cc = find_rec ("posl", &posl_rec, EQUAL, "r");
		if (cc || posl_rec.hhsh_hash != hhshHash)
		{
			cc = find_rec ("poln",&poln_rec,NEXT,"r");
			continue;
		}

		out_stand = posl_rec.ship_qty;
			
		if (out_stand <= 0.00)
		{
			cc = find_rec ("poln",&poln_rec,NEXT,"r");
			continue;
		}
		printed = TRUE;
		if (!first && !DISP)
			fprintf (fout, "|%-84.84s "," ");

		if (DISP)
		{
			if (first)
				sprintf (tempString, "%-67.67s", disp_str);
			else
			{
				sprintf 
				(
					tempString, 
					"%9.9s^E%2.2s^E%15.15s^E%18.18s^E%15.15s",
					" ", " ", " ", " ", " "
				);
			}
			sprintf (disp1_str,
				"%s^E%03d ^E%-16.16s^E%-25.25s^E%10.2f^E %-4.4s",
				tempString,
				line_no++,
				inmr_rec.item_no,
				poln_rec.item_desc,
				out_stand * CnvFct,
				inum_rec.uom);

			Dsp_saverec (disp1_str);
		}
		else
		{
			fprintf (fout, "%03d",line_no++);
			fprintf (fout, " %-16.16s",inmr_rec.item_no);
			fprintf (fout, " %-40.40s",poln_rec.item_desc);
			fprintf (fout, " %12.2f ",out_stand * CnvFct);
			fprintf (fout, " %-4.4s|\n",inum_rec.uom);
			fflush (fout);
		}
		first = FALSE;
		cc = find_rec ("poln",&poln_rec,NEXT,"r");
	}
	return (printed);
}
void
ShipmentHeader (void)
{
	fprintf (fout, "|------------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----");
	fprintf (fout, "---------------------");
	fprintf (fout, "------");
	fprintf (fout, "---------------------");
	fprintf (fout, "---------------------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "-------------");
	fprintf (fout, "-----------|\n");

	fprintf (fout, "|SHIPMENT    ");
	fprintf (fout, " DEPARTURE ");
	fprintf (fout, " ARRIVAL   ");
	fprintf (fout, " SHIP");
	fprintf (fout, " VESSEL              ");
	fprintf (fout, " CURR ");
	fprintf (fout, " PORT OF ORIGIN      ");
	fprintf (fout, " DESTINATION         ");
	fprintf (fout, " DOC RECVD.");
	fprintf (fout, " DOC AGENT ");
	fprintf (fout, " NEG. B.O.L");
	fprintf (fout, " B.O.L  NO.  ");
	fprintf (fout, " COST DATE |\n");

	fprintf (fout, "|--------    ");
	fprintf (fout, " --------- ");
	fprintf (fout, " -------   ");
	fprintf (fout, " ----");
	fprintf (fout, " ------              ");
	fprintf (fout, " ---- ");
	fprintf (fout, " --------------      ");
	fprintf (fout, " -----------         ");
	fprintf (fout, " ----------");
	fprintf (fout, " --------- ");
	fprintf (fout, " ----------");
	fprintf (fout, " ----------  ");
	fprintf (fout, " --------- |\n");
/*

	fprintf (fout, "|  SHIPMENT  ");
	fprintf (fout, "|DEPARTURE ");
	fprintf (fout, "| ARRIVAL  ");
	fprintf (fout, "|SHIP");
	fprintf (fout, "|        VESSEL      ");
	fprintf (fout, "|CURR ");
	fprintf (fout, "|   PORT OF ORIGIN   ");
	fprintf (fout, "|     DESTINATION    ");
	fprintf (fout, "|DOC RECVD.");
	fprintf (fout, "|DOC AGENT ");
	fprintf (fout, "|NEG. B.O.L");
	fprintf (fout, "| B.O.L  NO. ");
	fprintf (fout, "|COST DATE |\n");

	fprintf (fout, "|------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------");
	fprintf (fout, "|----------|\n");
*/
	fflush (fout);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	char	tmp_date [11];

	if (!restart) 
	{
		if (scn != cur_screen && scn != 2)
			scn_set (scn);

		if (scn == 2)
			swide ();

		clear ();
	
		if (scn == 1)
		{
			rv_pr (ML (mlPoMess029), 25, 0, 1);
			box (0,3,80,8);
			line_at (1,0,80);
			line_at (7,1,79);
			line_at (20,0,80);
			
			print_at (21,0, ML (mlStdMess038), comm_rec.co_no,clip (comm_rec.co_name));
			line_at (22,0,80);
			/*  reset this variable for new screen NOT page	*/
			line_cnt = 0;
			scn_write (scn);
		}
		else
		{
			/* Shipping Schedule Display */ 
			rv_pr (ML (mlPoMess030), 50, 0, 1);
			line_at (1,0,132);

			strcpy (tmp_date,DateToString (startDate));
			sprintf (err_str, ML (mlPoMess031), tmp_date, DateToString (endDate));
			rv_pr (err_str, 40, 2, 1);
		}
	}
    return (EXIT_SUCCESS);
}
