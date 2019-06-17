/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: br_boprt.c,v 5.7 2002/07/17 09:58:03 scott Exp $
|  Program Name  : (so_br_boprt.c)
|  Program Desc  : (Print Transfer Backorders By Item)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 07/10/91         |
|---------------------------------------------------------------------|
| $Log: br_boprt.c,v $
| Revision 5.7  2002/07/17 09:58:03  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.6  2001/10/11 10:01:05  scott
| Updated to print data for all branches
|
| Revision 5.5  2001/09/26 23:15:20  scott
| Updated from Scott's machine
|
| Revision 5.4  2001/09/20 07:44:13  robert
| Update for display alignment
|
| Revision 5.3  2001/09/20 07:23:43  robert
| Updated to fix display alignments
|
| Revision 5.2  2001/08/09 09:20:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:50:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:52  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/03/26 07:46:19  scott
| Updated to limit useage of dsp_screen to report
|
| Revision 4.1  2001/03/26 07:37:59  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: br_boprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_br_boprt/br_boprt.c,v 5.7 2002/07/17 09:58:03 scott Exp $";

#include <pslscr.h>	
#include <signal.h>	
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

#define	DISP	 (reportType [0] == 'D')

#define	LOWER	 (label ("startItem"))
#define	UPPER	 (label ("endItem"))

#define	ITEM	0
#define	BRANCH	1
#define	GRAND	2

#define	BLNK		0
#define	LINE		1
#define	GRAND_TOT	3

#define	ITLN_BACKORDER 	 (itln_rec.status [0] == 'B' || \
		          		  itln_rec.qty_border > 0.00)

char	*UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

char	*SEPARATOR = "                                     ^E        ^E             ^E          ^E          ^E           ^E ^E ^E         ^E          ^E         ";

char	*ITEM_LINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGHGHGHGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGG^^";

char	*BR_LINE =  "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGHGHGHGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGG^^";

char	*CO_LINE =  "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGJGJGJGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGG^^";

char  *HEADER =  " RECEIVING WAREHOUSE                  |  DEL  |  TRANSFER   | ISSUE    | REQUIRED |  QUANTITY |C|F|  PURCH  | DUE DATE | QUANTITY ";
char  *HEADER1 = "   NUMBER / NAME                      |  NO.  |  COMMENTS   |  DATE    |  DATE    | BACKORDER |S|S|  NUMBER |          |          ";

char	*displayUserKeys = "[REDRAW] [NEXT] [PREV] [EDIT/END]";

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct polnRecord	poln2_rec;
struct posoRecord	poso_rec;
struct posoRecord	poso2_rec;
struct inmrRecord	inmr_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct inexRecord	inex_rec;

	extern	int	TruePosition;
	extern	int	EnvScreenOK;

	int		envVarDbCo,
			envVarDbFind,
			firstFlag = FALSE,
			dataFound = FALSE;

	char	branchNo [3],
			dataStr [200],
			envStr [300],
			reportType [2];

	float	tot_po [3];	/* For Purchase-Orders	*/
	float	tot_qty [4];	/* tot_qty [3] is for transfers	*/
	double	tot_ext [3];

	FILE	*fout;
	FILE	*fsort;

	char	*poln2	=	"poln2",
			*poso2	=	"poso2",
			*ccmr2	=	"ccmr2";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	startItem [17];
	char	startItemDesc [41];
	char	endItem [17];
	char	endItemDesc [41];
	char	startBrNo [3];
	char	startBrDesc [41];
	char	endBrNo [3];
	char	endBrDesc [41];
	char	date_reqd [11];
	long	inputDate;
	int		printerNumber;
	char	printerString [3];
	char 	back [2];
	char 	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
	char	reportDesc [31];
	char	cutoff_date [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startItemDesc",	 4, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItem",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End Item          ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endItemDesc", 5, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},

	{1, LIN, "startBrNo",	 7, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Branch      ", "",
		YES, NO,  JUSTRIGHT, "", "", local_rec.startBrNo},
	{1, LIN, "startBrDesc",	 7, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startBrDesc},
	{1, LIN, "endBrNo",	 8, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "End Branch        ", "",
		YES, NO,  JUSTRIGHT, "", "", local_rec.endBrNo},
	{1, LIN, "endBrDesc",	 8, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endBrDesc},

	{1, LIN, "reqd_date",	 10, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Required Date     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.inputDate},

	{1, LIN, "printerNumber",	12, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	13, 23, CHARTYPE,
		"AAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite",14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight         ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc",	14, 23, CHARTYPE,
		"AAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		poPrinted = FALSE;
char	currBranch [3];
char	prevBranch [3];
char	currItem [17];
char	prevItem [17];
char	dispStr [256];

struct	{
	long	hhplHash;
	char	po_num [sizeof (pohr_rec.pur_ord_no)];
	char	dueDate [11];
	float	poQtyRemain;
	} poRec [250];

int	no_in_tab;
int	printed = FALSE;


/*=======================
| Function Declarations |
=======================*/
char 	*GetPoNum 				(long);
float 	OutstandingPo 			(float);
int  	FindRange 				(int, char *, char *);
int  	GetIthr 				(void);
int  	GetRecWarehouse 		(void);
int  	ValidBranch 			(char *);
int  	heading 				(int);
int  	spec_valid 				(int);
void 	CloseDB 				(void);
void 	FindPurchaseOrder 		(char *);
void 	GetBranch 				(void);
void 	HeadingOutput 			(void);
void 	OpenDB 					(void);
void 	PrintBrTotal 			(int);
void 	PrintBranchHead 		(char *);
void 	PrintCoTotal 			(void);
void 	PrintInex 				(void);
void 	PrintItemHead 			(char *);
void 	PrintItemTotal 			(void);
void 	PrintLine 				(int);
void 	PrintTransfer 			(char *);
void 	ProcessInmr 			(void);
void 	ProcessSort 			(void);
void	ReadMisc 				(void);
void 	ReadTransfers 			(void);
void 	RunProgram 				(char *);
void 	SetDefaults 			(void);
void 	SrchEsmr 				(char *);
void 	shutdown_prog 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2 && argc != 8)
	{
		/*-----------------------------------
		| Usage : %s <D (isplay | P (rint> OR |
		|      : %s <D (isplay | P (rint>		|
		|           <Start Item>			|
		|           <End Item>				|
		|           <Start Branch>			|
		|           <End Branch>			|
		|           <req_date>				|
		|           <printerNumber>					|
		-----------------------------------*/
		print_at (0,0, mlSoMess733, argv [0]);
		print_at (1,0, mlSoMess734, argv [0]);
		print_at (2,0, mlSoMess735);
		print_at (3,0, mlSoMess736);
		print_at (4,0, mlSoMess737);
		print_at (5,0, mlSoMess738);
		print_at (6,0, mlSoMess739);
		print_at (7,0, mlSoMess740);
		return (EXIT_FAILURE);
	}

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	switch	 (argv [1] [0])
	{
	case	'D':
	case	'd':
		strcpy (reportType,"D");
		if (argc == 2)
		{
			FLD ("printerNumber") 	= ND;
			FLD ("back") 		 	= ND;
			FLD ("onite") 		 	= ND;
			FLD ("backDesc") 	 	= ND;
			FLD ("oniteDesc") 	 	= ND;
		}
		break;

	case	'P':
	case	'p':
		strcpy (reportType,"P");
		if (argc == 2)
		{
			FLD ("printerNumber") 	= YES;
			FLD ("back") 			= YES;
			FLD ("onite") 			= YES;
			FLD ("backDesc") 	 	= NA;
			FLD ("oniteDesc") 	 	= NA;
		}
		break;
	
	default :
		print_at (0,0, mlSoMess741);
		return (EXIT_FAILURE);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	if (argc == 2 || (argc == 8 && DISP))
	{
		init_scr ();		/*  sets terminal from termcap	*/
		set_tty ();			/*  get into raw mode		*/
		set_masks ();		/*  setup print using masks	*/
		init_vars (1);		/*  set default values		*/
		swide ();
	}

	envVarDbCo 	 = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envVarDbCo) ? comm_rec.est_no : " 0");

	if (argc == 8)
	{
		/*---------------------------------------------------
		| If child process is backgd, then set to raw mode	|
		---------------------------------------------------*/
		if (for_chk () != 0)
			signal (SIGINT,SIG_IGN);

		sprintf (local_rec.startItem, 	"%-16.16s", 	argv [2]);
		sprintf (local_rec.endItem, 	"%-16.16s", 	argv [3]);
		sprintf (local_rec.startBrNo, 	"%2.2s", 		argv [4]);
		sprintf (local_rec.endBrNo, 	"%2.2s", 		argv [5]);
		sprintf (local_rec.date_reqd,	"%-10.10s",		argv [6]);
		local_rec.printerNumber = atoi (argv [7]);
		
		sprintf (err_str,
			"Processing : %s Backorders By Item",
			 (DISP) ? "Display" : "Print");

		if (!DISP)
			dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		ProcessInmr ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------
	| Reset control flags |
	---------------------*/
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	search_ok 	= TRUE;
	init_vars (1);		/*  set default values		*/

	SetDefaults ();

	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
    /* reversing the logic to compact the code... */
	/* if (restart)
		shutdown_prog (); */

	if (!restart)
		RunProgram (argv [0]);
	
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefaults (
 void)
{
	strcpy (local_rec.startItem, "                ");
	sprintf (local_rec.startItemDesc, "%-40.40s", ML ("First Item"));
	strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
	sprintf (local_rec.endItemDesc, "%-40.40s", ML ("Last Item"));
	strcpy (local_rec.startBrNo, "  ");
	sprintf (local_rec.startBrDesc, "%-40.40s", ML ("First Branch"));
	strcpy (local_rec.endBrNo, "~~");
	sprintf (local_rec.endBrDesc, "%-40.40s", ML ("Last Branch"));
	local_rec.inputDate = StringToDate (local_rec.systemDate);
	local_rec.printerNumber = 1;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.backDesc, ML ("No "));
	strcpy (local_rec.onite, "N");
	strcpy (local_rec.oniteDesc, ML ("No "));
}

void
RunProgram (
	char *programName)
{
	sprintf (local_rec.reportDesc, ML ("Print Backorders By Item"));

	sprintf (local_rec.printerString,"%2d",local_rec.printerNumber);
	sprintf (local_rec.cutoff_date,"%s",DateToString (local_rec.inputDate));
	
	shutdown_prog ();

	if (!strncmp (local_rec.endItem, "~~~~~~~~~~~~~~~~", 16))
		memset ((char *)local_rec.endItem,0xff,sizeof (local_rec.endItem));

	if (!strncmp (local_rec.endBrNo, "~~", 2))
		memset ((char *)local_rec.endBrNo,0xff,sizeof (local_rec.endBrNo));
	
	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				"ONIGHT",
				"ONIGHT",
				programName,
				reportType,
				local_rec.startItem,
				local_rec.endItem,
				local_rec.startBrNo,
				local_rec.endBrNo,
				local_rec.cutoff_date,
				local_rec.printerString,
				local_rec.reportDesc, (char *)0
			);
		}
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				programName,
				programName,
				reportType,
				local_rec.startItem,
				local_rec.endItem,
				local_rec.startBrNo,
				local_rec.endBrNo,
				local_rec.cutoff_date,
				local_rec.printerString, (char *)0
			);
		}
	}
	else 
	{
		execlp 
		(
			programName,
			programName,
			reportType,
			local_rec.startItem,
			local_rec.endItem,
			local_rec.startBrNo,
			local_rec.endBrNo,
			local_rec.cutoff_date,
			local_rec.printerString, (char *)0
		);
	}
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

int
spec_valid (
	int		field)
{
	if (LCHECK ("startItem"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startItem,"%-16.16s"," ");
			sprintf (local_rec.startItemDesc, "%-40.40s", ML ("First Item"));
			DSP_FLD ("startItem");
			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}

		cc = FindRange (field, local_rec.startItem, local_rec.startItemDesc);
		return (cc);
	}

	if (LCHECK ("endItem")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItem,"~~~~~~~~~~~~~~~~");
			sprintf (local_rec.endItemDesc, "%-40.40s", ("Last Item"));
			DSP_FLD ("endItem");
			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}
		cc = FindRange (field,local_rec.endItem,local_rec.endItemDesc);
		return (cc);
	}

	if (LCHECK ("startBrNo"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startBrNo, "%2.2s", " "); 
			sprintf (local_rec.startBrDesc, "%-40.40s", ML ("First Branch"));
			DSP_FLD ("startBrNo");
			DSP_FLD ("startBrDesc");
			return (EXIT_SUCCESS);
		}
	
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%2.2s", local_rec.startBrNo);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------
			| Branch Not Found On File |
			--------------------------*/
			print_mess (ML (mlStdMess073)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startBrNo, local_rec.endBrNo) > 0)
		{
			/*------------------------------
			| Start Must Be less than End. |
			------------------------------*/
			print_mess (ML (mlStdMess017)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.startBrDesc, "%-40.40s", esmr_rec.est_name);
		DSP_FLD ("startBrDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endBrNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endBrNo, "~~");
			sprintf (local_rec.endBrDesc, "%-40.40s", ML ("Last Branch"));

			DSP_FLD ("endBrNo");
			DSP_FLD ("endBrDesc");
			return (EXIT_SUCCESS);
		}
	
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%2.2s", local_rec.endBrNo);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------
			| Branch Not Found. |
			-------------------*/
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startBrNo, local_rec.endBrNo) > 0)
		{
			/*-------------------------------------
			| Start Branch Must Be Less Than End. |
			-------------------------------------*/
			print_mess (ML (mlStdMess017)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.endBrDesc,"%-40.40s", esmr_rec.est_name);
		DSP_FLD ("endBrDesc");
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

/*==================
| Search for esmr. |
==================*/
void
SrchEsmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Br.","#Branch Name.");
	sprintf (esmr_rec.est_no,"%-2.2s",key_val);
	cc = find_rec (esmr,&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp (comm_rec.co_no, esmr_rec.co_no) &&
		      !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no,esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%-2.2s", temp_str);
	cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
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
		/*------------------------------------
		| Transfer Backorders Report By Item |
		------------------------------------*/
		rv_pr (ML (mlSoMess041),45,0,1);
		line_at (1,0,132);

		move (1,input_row);

		if (DISP)
		{
			box (0,3,132,7);

			line_at (6,1,131);
			line_at (9,1,131);
		}
		else
		{
			box (0,3,132,11);

			line_at (6,1,131);
			line_at (9,1,131);
			line_at (11,1,131);
		}

		line_at (20,0,132);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	ReadMisc ();

	abc_alias (poln2, poln);
	abc_alias (poso2, poso);

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (poln2,poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poso, poso_list, POSO_NO_FIELDS, "poso_id_no2");
	open_rec (poso2,poso_list, POSO_NO_FIELDS, "poso_hhpl_hash");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (esmr);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (poln);
	abc_fclose (poln2);
	abc_fclose (pohr);
	abc_fclose (poso);
	abc_fclose (poso2);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	abc_alias (ccmr2, ccmr);
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	GetBranch ();

	if (DISP)
	{
		clear ();

		/*-----------------------------------
		| TRANSFER BACKORDER REPORT BY ITEM |
		-----------------------------------*/
		rv_pr (ML (mlSoMess041),45,0,1);

		/*---------------------------------------------
		| Start Item %-16.16s       End Item %-16.16s |
		---------------------------------------------*/
		print_at (1,36, ML (mlSoMess040), 
						local_rec.startItem, (local_rec.endItem [0] == -1) ? "~~~~~~~~~~~~~~~~" : local_rec.endItem);

		Dsp_open (0, 2, 15);
		Dsp_saverec (HEADER);
		Dsp_saverec (HEADER1);

		Dsp_saverec (displayUserKeys);
	}
	else
	{
		if ((fout = popen ("pformat","w")) == NULL)
			file_err (errno, "pformat", "popen");

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout,".LP%d\n",local_rec.printerNumber);
		fprintf (fout,".13\n");
		fprintf (fout,".PI12\n");
		fprintf (fout,".L160\n");
		fprintf (fout,".B1\n");
		fprintf (fout, ".ETRANSFER BACKORDER REPORT BY ITEM\n");
		fprintf (fout, ".ECOMPANY %s : %s\n",
			comm_rec.co_no, clip (comm_rec.co_name));
	
		fprintf (fout, ".EFOR BRANCH %s  %s",
			local_rec.startBrNo, clip (local_rec.startBrDesc));
	
		fprintf (fout, "  TO %s  %s\n",
			local_rec.endBrNo, clip (local_rec.endBrDesc));
	
		fprintf (fout,".EAS AT %s\n",SystemTime ());
		fprintf (fout,".B1\n");
	
		fprintf (fout, ".CStart Item %s ", local_rec.startItem);
		fprintf (fout, "   End Item %s\n", local_rec.endItem);
	
		fprintf (fout,".B1\n");

		fprintf (fout, ".R================================");
		fprintf (fout, "==================================");
		fprintf (fout, "==================================");
		fprintf (fout, "==================================");
		fprintf (fout, "==========\n");
	}
}

void
PrintBranchHead (
 char *br_no)
{
	char	br_desc [41];

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", br_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		sprintf (br_desc, "%-40.40s", " ");
	else
		sprintf (br_desc, "%-40.40s", esmr_rec.est_name);

	if (DISP)
	{
		sprintf (dispStr, "^1 REC. BRANCH  %s : %s ^6", br_no, br_desc);
		Dsp_saverec (dispStr);
		return;
	}

	fprintf (fout, ".DS5\n");

	fprintf (fout, ".e RECEIVING BRANCH %s : %s\n", br_no, br_desc);

	fprintf (fout, "================================================");
	fprintf (fout, "=========");
	fprintf (fout, "===================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "====");
	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "! RECEIVING WAREHOUSE                           ");
	fprintf (fout, "!  DEL   ");
	fprintf (fout, "!     TRANSFER     ");
	fprintf (fout, "!  ISSUE   ");
	fprintf (fout, "! REQUIRED ");
	fprintf (fout, "!  QUANTITY  ");
	fprintf (fout, "!C|F");
	fprintf (fout, "! PURCH   ");
	fprintf (fout, "! DUE DATE ");
	fprintf (fout, "! QUANTITY !\n");

	fprintf (fout, "! NUMBER / NAME                                 ");
	fprintf (fout, "!  NO.   ");
	fprintf (fout, "!     COMMENTS     ");
	fprintf (fout, "!   DATE   ");
	fprintf (fout, "!   DATE   ");
	fprintf (fout, "! BACKORDER  ");
	fprintf (fout, "!S|S");
	fprintf (fout, "! NUMBER  ");
	fprintf (fout, "!          ");
	fprintf (fout, "!          !\n");

	PrintLine (LINE);
	fflush (fout);
}

void
GetBranch (
 void)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", local_rec.startBrNo);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		sprintf (local_rec.startBrDesc, "%-40.40s", " ");
	else
		sprintf (local_rec.startBrDesc, "%-40.40s", esmr_rec.est_name);

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", local_rec.endBrNo);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		sprintf (local_rec.endBrDesc, "%-40.40s", " ");
	else
		sprintf (local_rec.endBrDesc,"%-40.40s", esmr_rec.est_name);
}

void
PrintLine (
 int type)
{
	switch (type)
	{
	case	BLNK:
		fprintf (fout,"!                   ");
		fprintf (fout,"                            ");
		fprintf (fout,"!        ");
		fprintf (fout,"!                  ");
		fprintf (fout,"!          ");
		fprintf (fout,"!          ");
		fprintf (fout,"!            ");
		fprintf (fout,"! ! ");
		fprintf (fout,"!         ");
		fprintf (fout,"!          ");
		fprintf (fout,"!          !\n");
		break;

	case	LINE:
		fprintf (fout,"!-------------------");
		fprintf (fout,"----------------------------");
		fprintf (fout,"!--------");
		fprintf (fout,"!------------------");
		fprintf (fout,"!----------");
		fprintf (fout,"!----------");
		fprintf (fout,"!------------");
		fprintf (fout,"!-!-");
		fprintf (fout,"!---------");
		fprintf (fout,"!----------");
		fprintf (fout,"!----------!\n");
		break;

	case	GRAND_TOT:
		fprintf (fout,"!===================");
		fprintf (fout,"============================");
		fprintf (fout,"!========");
		fprintf (fout,"!==================");
		fprintf (fout,"!==========");
		fprintf (fout,"!==========");
		fprintf (fout,"!============");
		fprintf (fout,"!=!=");
		fprintf (fout,"!=========");
		fprintf (fout,"!==========");
		fprintf (fout,"!==========!\n");
		break;
	}
	
	fflush (fout);
}

void
ProcessInmr (
 void)
{

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,local_rec.startItem);

	fsort = sort_open ("br_tfer");

	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && 
	       !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
	       strcmp (inmr_rec.item_no,local_rec.startItem) >= 0 && 
	       strcmp (inmr_rec.item_no,local_rec.endItem) <= 0) 
	{

		ReadTransfers ();

		if (!DISP)
			dsp_process ("Item No. : ",inmr_rec.item_no);

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}

	HeadingOutput ();

	if (dataFound)
		ProcessSort ();

	if (DISP)
	{
		Dsp_srch ();
		Dsp_close ();
	}
	else
	{
		fprintf (fout,".EOF\n");
		pclose (fout);
	}

	sort_delete (fsort, "br_tfer");
}

void
PrintTransfer (
 char *tfer_ln)
{
	char	tmpDate [11];
	char	fullSupply [2];

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", tfer_ln);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		sprintf (local_rec.startBrDesc, "%-40.40s", " ");
	else
		sprintf (local_rec.startBrDesc, "%-40.40s", esmr_rec.est_name);

	poPrinted = FALSE;

	FindPurchaseOrder (tfer_ln);

	switch (tfer_ln [93])
	{
	case 'Y':
		strcpy (fullSupply, "O");
		break;

	case 'L':
		strcpy (fullSupply, "L");
		break;

	case 'N':
	default:
		strcpy (fullSupply, " ");
		break;
	}

	if (!poPrinted)
	{
		if (DISP)
		{
			strcpy (tmpDate, DateToString (atol (tfer_ln + 20)));

			sprintf (dispStr,
				" %2.2s / %-30.30s ^E%08ld^E%-13.13s^E%-10.10s^E%-10.10s^E %10.2f^E%-1.1s^E%-1.1s^E%-9.9s^E%-10.10s^E%-10.10s",
				tfer_ln,
				strncpy (temp_str, esmr_rec.est_name, 26),
				atol (tfer_ln + 29), 
				tfer_ln + 47, 
				tmpDate,
				DateToString (atol (tfer_ln + 38)), 
				atof (tfer_ln + 68), 
				tfer_ln + 91,
				fullSupply,
				" ", " ", " ");

			Dsp_saverec (dispStr);
		}
		else
		{
			fprintf (fout, "! %2.2s /", tfer_ln);
			fprintf (fout, " %-40.40s ", esmr_rec.est_name);
			fprintf (fout, "!%08ld",  atol (tfer_ln + 29));
			fprintf (fout, "! %-16.16s ", tfer_ln + 47);
			fprintf (fout, "!%-10.10s", DateToString (atol (tfer_ln + 20)));
			fprintf (fout, "!%-10.10s", DateToString (atol (tfer_ln + 38)));
			fprintf (fout, "! %10.2f ", atof (tfer_ln + 68));
			fprintf (fout, "!%-1.1s",   tfer_ln + 91);
			fprintf (fout, "!%-1.1s",   fullSupply);
			fprintf (fout, "!%-9.9s",   " ");
			fprintf (fout, "!%-10.10s", " ");
			fprintf (fout, "!%-10.10s!\n", " ");
		}
	}

	tot_qty [ITEM] += (float) (atof (tfer_ln + 68));
}

void
FindPurchaseOrder (
 char *tfer_ln)
{
	int	firstPo;
	long	d_date;
	long	itffHash;
	char	tmpPoNo [sizeof (pohr_rec.pur_ord_no)];
	char	tmpDelNo [7];
	char	tmpQtyBo [11];
	char	tmpDate [11];
	char	fullSupply [2];

	firstPo = TRUE;
	
	itffHash = atol (tfer_ln + 82);

	switch (tfer_ln [93])
	{
	case 'Y':
		strcpy (fullSupply, "O");
		break;

	case 'L':
		strcpy (fullSupply, "L");
		break;

	case 'N':
	default:
		strcpy (fullSupply, " ");
		break;
	}

	poso_rec.itff_hash = itffHash;
	poso_rec.hhpl_hash = 0L;
	cc = find_rec (poso, &poso_rec, GTEQ, "r");
	while (!cc && poso_rec.itff_hash == itffHash)
	{
		poln_rec.hhpl_hash	=	poso_rec.hhpl_hash;
		cc = find_rec (poln, &poln_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}

		sprintf (tmpPoNo,"%-15.15s", GetPoNum (poso_rec.hhpl_hash));
		d_date = poln_rec.due_date;

		poPrinted = TRUE;
		if (DISP)
		{
			strcpy (tmpDate, DateToString (atol (tfer_ln + 20)));
			sprintf (tmpDelNo, "%06ld", atol (tfer_ln + 29));
			sprintf (tmpQtyBo, "%10.2f", atof (tfer_ln + 68));

			sprintf (dispStr,
				" %2.2s / %-30.30s ^E%-8.8s^E%-13.13s^E%-10.10s^E%-10.10s^E%-1.1s^E%-1.1s^E %-10.10s^E%-9.9s^E%-10.10s^E%10.2f",
				 (firstPo) ? tfer_ln : " ",
				 (firstPo) ? strncpy (temp_str, esmr_rec.est_name, 26) : " ",
				 (firstPo) ? tmpDelNo : " ", 
				 (firstPo) ? (tfer_ln + 47) : " ", 
				 (firstPo) ? tmpDate : " ", 
				 (firstPo) ? DateToString (atol (tfer_ln + 38)) : " ", 
				 (firstPo) ? tmpQtyBo : " ", 
				 (firstPo) ? (tfer_ln + 91) : " ", 
				 (firstPo) ? fullSupply : " ", 
				tmpPoNo + 6,
				DateToString (d_date),
				poso_rec.qty);

			Dsp_saverec (dispStr);
		}
		else
		{
			fprintf (fout, "! %2.2s / ", 
				 (firstPo) ? tfer_ln : " ");

			fprintf (fout, "%-40.40s ", 
				 (firstPo) ? esmr_rec.est_name : " ");

			if (firstPo)
				fprintf (fout, "!%08ld", atol (tfer_ln + 29));
			else
				fprintf (fout, "! %-6.6s ", " ");

			fprintf (fout, "! %-16.16s ", 
				 (firstPo) ? (tfer_ln + 47) : " ");

			fprintf (fout, "!%-10.10s", 
				 (firstPo) ? DateToString (atol (tfer_ln + 20)) : " ");

			fprintf (fout, "!%-10.10s", 
				 (firstPo) ? DateToString (atol (tfer_ln + 38)) : " ");

			if (firstPo)
				fprintf (fout, "! %10.2f ", atof (tfer_ln + 68));
			else
				fprintf (fout, "! %-10.10s ", " ");

			fprintf (fout,"!%-1.1s", (firstPo) ? (tfer_ln + 91) : " ");
			fprintf (fout,"!%-1.1s", (firstPo) ? fullSupply : " ");

			fprintf (fout, "!%-9.9s", tmpPoNo + 6);
			fprintf (fout, "!%-10.10s", DateToString (d_date));
			fprintf (fout, "!%10.2f!\n", poso_rec.qty);
		}

		tot_po [ITEM]  += poso_rec.qty;

		firstPo = FALSE;
		cc = find_rec (poso, &poso_rec, NEXT, "r");
	}
}

void
PrintItemTotal (
 void)
{
	float	tot_outstd;

	tot_outstd = OutstandingPo (tot_po [ITEM]);

	if (DISP)
	{
		Dsp_saverec (SEPARATOR);
		sprintf (dispStr,
			" %-35.35s ^E%-8.8s^E%-13.13s^E%-10.10s^E%-10.10s^E %10.2f^E ^E ^E%-9.9s^E%-10.10s^E%10.2f",
			" TOTAL FOR ITEM",
			" ", " ", " ", " ",
			tot_qty [ITEM],
			" ", " ",
			tot_outstd);

		Dsp_saverec (dispStr);
	}
	else
	{
		PrintLine (BLNK);
	
		fprintf (fout, "! %-45.45s ", " TOTAL FOR ITEM");
		fprintf (fout, "! %-6.6s ", " ");
		fprintf (fout, "! %-16.16s ", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "! %10.2f ", tot_qty [ITEM]);
		fprintf (fout, "! ! ");
		fprintf (fout, "!%-9.9s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%10.2f!\n", tot_outstd);
	}

	tot_qty [BRANCH] += tot_qty [ITEM];
	tot_po [BRANCH]  += tot_outstd;

	tot_qty [ITEM] = 0.00;
	tot_po [ITEM] = 0.00;
}

float	
OutstandingPo (
 float c_po_qty)
{
	int	first_time = TRUE;	
	int	i, po_found;
	char	tmpPoNo [sizeof (pohr_rec.pur_ord_no)];
	char	tmp_hd [31];

	no_in_tab = 0;
	sprintf (tmp_hd, "%-30.30s", "Outstanding Purchase Orders");
	
	poln2_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (poln2, &poln2_rec, GTEQ, "r");
	while (!cc && poln2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (poln2_rec.hhcc_hash != ccmr_rec.hhcc_hash ||
		    (poln2_rec.qty_ord - poln2_rec.qty_rec) == 0.00)
		{
			cc = find_rec (poln2, &poln2_rec, NEXT, "r");
			continue;
		}

		sprintf (tmpPoNo,"%-15.15s",GetPoNum (poln2_rec.hhpl_hash));
		po_found = FALSE;
		for (i = 0; i < no_in_tab; i++)
		{
			if (!strcmp (poRec [i].po_num, tmpPoNo))
			{
				/* decrement in poRec */
				poRec [i].poQtyRemain += (poln2_rec.qty_ord - poln2_rec.qty_rec);
				po_found = TRUE;
			}
		}

		if (!po_found)
		{
			/* store in poRec */
			poRec [no_in_tab].hhplHash = poln_rec.hhpl_hash;
			sprintf (poRec [no_in_tab].po_num,"%-15.15s",tmpPoNo);
			strcpy (poRec [no_in_tab].dueDate, DateToString (poln2_rec.due_date));
			poRec [no_in_tab++].poQtyRemain = (poln_rec.qty_ord - poln_rec.qty_rec);
		}
		cc = find_rec (poln2, &poln2_rec, NEXT, "r");
	}

	for (i = 0; i < no_in_tab; i++)
	{
		poso2_rec.hhpl_hash	= poRec [i].hhplHash;
		cc = find_rec (poso2, &poso2_rec, GTEQ, "r");
		while (!cc && poso2_rec.hhpl_hash == poRec [i].hhplHash)
		{
			poRec [i].poQtyRemain -= poso2_rec.qty;

			cc = find_rec (poso2,&poso2_rec, GTEQ, "r");
		}
	}
	
	for (i = 0; i < no_in_tab; i++)
	{
		if (poRec [i].poQtyRemain <= 0.00)
			continue;

		c_po_qty += poRec [i].poQtyRemain;

		if (first_time)
		{
			if (DISP)
				Dsp_saverec ("                                     ^E        ^E             ^E          ^E          ^E           ^E ^E ^E         ^E          ^E          ");
			else
				PrintLine (BLNK);
		}
		else
			sprintf (tmp_hd, "%-30.30s", " ");

		if (DISP)
		{
			sprintf (envStr,
				"%-30.30s       ^E        ^E             ^E          ^E          ^E           ^E ^E ^E%-9.9s^E%10.10s^E%10.2f",
				tmp_hd,
				poRec [i].po_num + 6,
				poRec [i].dueDate,
				poRec [i].poQtyRemain);
			Dsp_saverec (envStr);
			first_time = FALSE;
		}
		else
		{
			fprintf (fout,"!%-30.30s                 ",tmp_hd);
			fprintf (fout,"!        ");
			fprintf (fout,"!                  ");
			fprintf (fout,"!          ");
			fprintf (fout,"!          ");
			fprintf (fout,"!            ");
			fprintf (fout,"! ! ");
			fprintf (fout,"!%-9.9s", poRec [i].po_num + 6);
			fprintf (fout,"!%10.10s", poRec [i].dueDate);
			fprintf (fout, "!%10.2f!\n", poRec [i].poQtyRemain);
			first_time = FALSE;
		}
	}

	return (c_po_qty);
}

void
PrintBrTotal (
 int last_tot)
{
	if (DISP)
	{
		Dsp_saverec (BR_LINE);
		sprintf (dispStr,
			" %-35.35s ^E%-8.8s^E%-13.13s^E%10.10s^E%10.10s^E %10.2f^E ^E ^E%-9.9s^E%-10.10s^E%10.2f",
			" TOTAL FOR BRANCH",
			" ", " ", " ", " ",
			tot_qty [BRANCH],
			" ", " ",
			tot_po [BRANCH]);

		Dsp_saverec (dispStr);
		if (!last_tot)
			Dsp_saverec (CO_LINE);
	}
	else
	{
		PrintLine (LINE);

		fprintf (fout, "! %-45.45s ", " TOTAL FOR BRANCH");
		fprintf (fout, "! %-6.6s ", " ");
		fprintf (fout, "! %-16.16s ", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "! %10.2f ", tot_qty [BRANCH]);
		fprintf (fout, "! ! ");
		fprintf (fout, "!%-9.9s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%10.2f!\n", tot_po [BRANCH]);
	}

	tot_qty [GRAND] += tot_qty [BRANCH];
	tot_po [GRAND]  += tot_po [BRANCH];

	tot_qty [BRANCH] = 0.00;
	tot_po [BRANCH] = 0.00;
}

void
PrintCoTotal (
 void)
{
	if (DISP)
	{
		Dsp_saverec (BR_LINE);
		sprintf (dispStr,
			" %-35.35s ^E%-8.8s^E%-13.13s^E%10.10s^E%10.10s^E %10.2f^E ^E ^E%-9.9s^E%-10.10s^E%10.2f",
			" TOTAL FOR COMPANY",
			" ", " ", " ", " ",
			tot_qty [GRAND],
			" ", " ",
			tot_po [GRAND]);

		Dsp_saverec (dispStr);
		Dsp_saverec (CO_LINE);
	}
	else
	{
		PrintLine (GRAND_TOT);

		fprintf (fout, "! %-45.45s ", " TOTAL FOR COMPANY");
		fprintf (fout, "! %-6.6s ", " ");
		fprintf (fout, "! %-16.16s ", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "! %10.2f ", tot_qty [GRAND]);
		fprintf (fout, "! ! ");
		fprintf (fout, "!%-9.9s", " ");
		fprintf (fout, "!%-10.10s", " ");
		fprintf (fout, "!%10.2f!\n", tot_po [GRAND]);
	}
}

void
PrintItemHead (
 char *item_no)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", item_no);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		sprintf (inmr_rec.description, "%-40.40s", "Item Not Found");

	if (!DISP)
		fprintf (fout,".LRP3\n");

	if (!firstFlag)
	{
		if (DISP)
			Dsp_saverec (ITEM_LINE);
		else
			PrintLine (LINE);
	}

	if (DISP)
	{
		sprintf (envStr,
			"%-16.16s  %-40.40s  ^E          ^E          ^E           ^E ^E ^E         ^E          ^E        ",
			item_no,
			inmr_rec.description);

		Dsp_saverec (envStr);
		PrintInex ();
		Dsp_saverec (SEPARATOR);
	}
	else
	{
		fprintf (fout, "! %-16.16s  %-40.40s%-16.16s",
			item_no, inmr_rec.description, " ");
		fprintf (fout,"!          ");
		fprintf (fout,"!          ");
		fprintf (fout,"!            ");
		fprintf (fout,"! ! ");
		fprintf (fout,"!         ");
		fprintf (fout,"!          ");
		fprintf (fout,"!          !\n");
		PrintInex ();
		PrintLine (BLNK);

		fflush (fout);
	}

	firstFlag = FALSE;
}

int
FindRange (
	int 	field, 
	char 	*fld_value, 
	char 	*fld_desc)
{
	if (SRCH_KEY)
	{
		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		return (EXIT_SUCCESS);
		return (EXIT_SUCCESS);
	}

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", fld_value);
	sprintf ((field == LOWER) ? local_rec.startItem : local_rec.endItem,
		"%-16.16s", inmr_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		/*------------------
		| Item  not found. |
		------------------*/
		print_mess (ML (mlStdMess001));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	sprintf (local_rec.startItem, "%-16.16s", local_rec.startItem);
	sprintf (local_rec.endItem, "%-16.16s", local_rec.endItem);
	if (strncmp (local_rec.startItem, local_rec.endItem, 16) > 0)
	{
		/*-----------------------------------
		| Start Item Must Be less THAN End. |
		-----------------------------------*/
		print_mess (ML (mlStdMess017));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	sprintf (fld_desc, "%-40.40s", (!cc) ? inmr_rec.description : " ");

	display_field (field);
	if (field == LOWER)
		DSP_FLD ("startItemDesc");
	else
		DSP_FLD ("endItemDesc");

	return (cc);
}

void
ReadTransfers (
 void)
{
	itln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while ((!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash))
	{
		if (ITLN_BACKORDER)
		{
		    if (GetIthr ())
		    {
				cc = find_rec (itln, &itln_rec, NEXT, "r");
				continue;
		    }

		    if (itln_rec.qty_border <= 0.00)
		    {
				cc = find_rec (itln, &itln_rec, NEXT, "r");
				continue;
		    }

		    GetRecWarehouse ();

		    if (ValidBranch (ccmr2_rec.est_no))
		    {
		    	sprintf (dataStr,
			    "%2.2s %-16.16s %08ld %08ld %08ld %-20.20s %10.2f %2.2s %08ld %-1.1s %-1.1s\n",
			    ccmr2_rec.est_no,
			    inmr_rec.item_no,
			    ithr_rec.iss_date,
			    ithr_rec.del_no,
				itln_rec.due_date,
				itln_rec.tran_ref,
				itln_rec.qty_border,
			    ccmr2_rec.cc_no,
			    itln_rec.itff_hash,
			    itln_rec.stock,
			    itln_rec.full_supply);

	 	        dataFound = TRUE;

		        sort_save (fsort, dataStr);
		    }
		}
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
}

void
ProcessSort (
 void)
{
	int	first_time = TRUE;
	char	head_disp [200];
	char	head_prt [200];
	char	*sptr;

	fsort = sort_sort (fsort, "br_tfer");

	strcpy (head_disp, HEADER);
	strcpy (head_prt, HEADER1);

	sptr = sort_read (fsort);
	while (sptr)
	{
		sprintf (currBranch, "%2.2s", sptr);
		sprintf (currItem, "%-16.16s", sptr + 3);

		if (first_time)
		{
			PrintBranchHead (currBranch);
			firstFlag = TRUE;
			PrintItemHead (currItem);

			strcpy (prevBranch, currBranch);
			strcpy (prevItem, currItem);
		}

		if (strcmp (prevItem, currItem) || strcmp (prevBranch, currBranch))
		{
			PrintItemTotal ();	
	
			if (strcmp (prevBranch, currBranch))
			{
				PrintBrTotal (FALSE);
	
				PrintBranchHead (currBranch);
				if (!DISP)
					fprintf (fout, ".PA\n");
				firstFlag = TRUE;
			}

			strcpy (prevBranch, currBranch);
			strcpy (prevItem, currItem);

			PrintItemHead (currItem);
		}

		PrintTransfer (sptr);

		first_time = FALSE;
		sptr = sort_read (fsort);
	}

	PrintItemTotal ();	
	PrintBrTotal (TRUE);
	PrintCoTotal ();
}

int
ValidBranch (
 char *br_no)
{
/*
	if (!strcmp (br_no, comm_rec.est_no))
		return (FALSE);
*/

	if (strcmp (br_no, local_rec.startBrNo) >= 0 && 
	    strcmp (br_no, local_rec.endBrNo) <= 0)
		return (TRUE);

	return (FALSE);
}

int
GetIthr (
 void)
{
	ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
	return (find_rec (ithr,&ithr_rec,COMPARISON, "r"));
}

int
GetRecWarehouse (
 void)
{

	cc = find_hash (ccmr2, &ccmr2_rec, COMPARISON, "r", 
		       itln_rec.r_hhcc_hash);

	return (cc);
}

char *
GetPoNum (
	long	hhplHash)
{
	poln_rec.hhpl_hash	=	hhplHash;
	if (find_rec (poln, &poln_rec, COMPARISON, "r"))
		return ("Not Fnd");

	pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
	if (find_rec (pohr, &pohr_rec, COMPARISON,"r"))
		return ("Not Fnd");

	return (pohr_rec.pur_ord_no);
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (DISP)
		{
			sprintf (envStr,
			"%-16.16s  %-40.40s  ^E          ^E          ^E           ^E ^E ^E         ^E          ^E        ",
			" ",
			inex_rec.desc);
			Dsp_saverec (envStr);
		}
		else
		{
			fprintf (fout,
				"! %-16.16s  %-40.40s%-16.16s",
				" ",
				inex_rec.desc,
				" ");
			fprintf (fout,"!          ");
			fprintf (fout,"!          ");
			fprintf (fout,"!            ");
			fprintf (fout,"! ! ");
			fprintf (fout,"!         ");
			fprintf (fout,"!          ");
			fprintf (fout,"!          !\n");
		}
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
