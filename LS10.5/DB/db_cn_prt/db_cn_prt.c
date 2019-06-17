/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cn_prt.c,v 5.4 2002/07/17 09:57:05 scott Exp $
|  Program Name  : (db_cn_prt.c)
|  Program Desc  : (Print Customer Contracts)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 30/09/93         |
|---------------------------------------------------------------------|
| $Log: db_cn_prt.c,v $
| Revision 5.4  2002/07/17 09:57:05  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/22 02:52:07  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cn_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cn_prt/db_cn_prt.c,v 5.4 2002/07/17 09:57:05 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cncdRecord	cncd_rec;
struct cnclRecord	cncl_rec;
struct ccmrRecord	ccmr_rec;
struct sumrRecord	sumr_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	int		envDbCo 	= 0,
			envDbMcurr	= 0;

	char	branchNo [3];

	char	*data	= "data";
		
	FILE	*fout;

extern	int		TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	startContractNo [7];
	char	startContractDesc [41];
	char	endContractNo [7];
	char	endContractDesc [41];
	int		printerNo;
	char 	back [2];
	char	backDesc [4];
	char	onight [2];
	char	onightDesc [4];
	char	dummy [11];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startContractNo",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Contract       ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startContractNo},
	{1, LIN, "startContractDesc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startContractDesc},
	{1, LIN, "endContractNo",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Contract         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endContractNo},
	{1, LIN, "endContractDesc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endContractDesc},
	{1, LIN, "printerNo",	  7, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	  8, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background           ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	  8, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	  9, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight            ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	  9, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes.
 */
int 	heading 		(int);
int 	spec_valid 		(int);
void 	CloseDB 		(void);
void 	HeadingCncd 	(void);
void 	HeadingCncl 	(void);
void 	HeadingOutput 	(void);
void 	OpenDB 			(void);
void 	PrintCncd 		(void);
void 	PrintCnch	 	(void);
void 	PrintCncl 		(void);
void 	PrintRulerCncd	(void);
void 	PrintRulerCncl	(void);
void 	ProcessFile 	(void);
void 	RunProgram 		(char *);
void 	shutdown_prog 	(void);
void 	SrchCnch 		(char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char		*sptr;

	TruePosition	=	TRUE;

	if (argc != 1 && argc != 4)
	{
		print_at (0,0,mlDbMess003, argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	SETUP_SCR (vars);

	OpenDB ();

	if (argc == 4)
	{
		sprintf (local_rec.startContractNo, "%-6.6s", argv [1]);
		sprintf (local_rec.endContractNo,   "%-6.6s", argv [2]);
		local_rec.printerNo = atoi (argv [3]);

		dsp_screen ("Processing : Printing Customer Contracts.",
					comm_rec.co_no,comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");

		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/

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
		init_vars (1);		

		/*
		 * Entry screen 1 linear input
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no");
	open_rec (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cnch);
	abc_fclose (cncd);
	abc_fclose (cncl);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (sumr);
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*-------------------
	| Validate Creditor |
	-------------------*/
	if (LCHECK ("startContractNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startContractDesc, ML ("Start Contract"));
			DSP_FLD ("startContractDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		if ((strcmp (local_rec.startContractNo, local_rec.endContractNo) > 0) &&
		   (strcmp (local_rec.endContractNo, "      ") != 0))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, local_rec.startContractNo);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startContractDesc, cnch_rec.desc);
		DSP_FLD ("startContractDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endContractNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endContractDesc, ML ("End Contract"));
			DSP_FLD ("endContractDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, local_rec.endContractNo);
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startContractNo, local_rec.endContractNo) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endContractDesc, cnch_rec.desc);
		DSP_FLD ("endContractDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
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

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc,
			(local_rec.onight [0] == 'Y') ? ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
	char	*programName)
{
	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		sprintf
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"",
			programName,
			local_rec.startContractNo,
			local_rec.endContractNo,
			local_rec.printerNo,
            ML ("Customer Contract Print")
		);
		SystemExec (err_str, TRUE);
	}
    else 
	{
		sprintf
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			local_rec.startContractNo,
			local_rec.endContractNo,
			local_rec.printerNo
		);
		print_at (0,0, "Ex [%s]", err_str);getchar();
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*==================================
| Start Output To Standard Print . |
==================================*/
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n",
				  comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  : %s - %s\n",
				  comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECUSTOMER CONTRACT REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n",SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "===============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======================");
	fprintf (fout, "================================================");
	fprintf (fout, "==========================================\n");
 
	fprintf (fout, ".R===============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======================");
	fprintf (fout, "================================================");
	fprintf (fout, "============================\n");
 
	fflush (fout);
}

void
PrintRulerCncl (void)
{
	strcpy (err_str, ".C|---------------------------------------------------");
	strcat (err_str, "----------------------------------------------------");
	strcat (err_str, "----------------------------------------------------|\n");
	fprintf (fout, err_str);
 
	fflush (fout);
}

void
PrintRulerCncd (void)
{
	if (envDbMcurr)
	{
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "-------------------------------------------");
		strcat (err_str, "-------------------------------------");
		strcat (err_str, "----------------------");
		strcat (err_str, "---------------------|\n");
	}
	else
	{
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "-------------------------------");
		strcat (err_str, "-------------------------------------");
		strcat (err_str, "----------------------");
		strcat (err_str, "---------------------|\n");
	}
	fprintf (fout, err_str);

	fflush (fout);
}

void
ProcessFile (void)
{
	int		first_time = TRUE;

	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, local_rec.startContractNo);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (cnch_rec.co_no, comm_rec.co_no) &&
		  (strcmp (cnch_rec.cont_no, local_rec.endContractNo) <= 0))
	{
		dsp_process ("Contract No. : ", cnch_rec.cont_no);
		if (first_time == FALSE)
		{
			fprintf (fout, ".DS1\n");
			fprintf (fout, ".B1\n");
			fprintf (fout, ".PA");
		}
		PrintCnch ();
		PrintCncd ();
		PrintCncl ();
		first_time = FALSE;

		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}
}

void
PrintCnch (void)
{
	char		ws_date_wef [11];
	char		ws_date_rev [11];
	char		ws_date_ren [11];
	char		ws_date_exp [11];

	strcpy (ws_date_wef,DateToString (cnch_rec.date_wef));
	strcpy (ws_date_rev,DateToString (cnch_rec.date_rev));
	strcpy (ws_date_ren,DateToString (cnch_rec.date_ren));
	strcpy (ws_date_exp,DateToString (cnch_rec.date_exp));

	fprintf (fout, ".B1\n");
	fprintf (fout, ".EContract No.   : %-6.6s %-40.40s\n",
				   cnch_rec.cont_no,
				   cnch_rec.desc);
	fprintf (fout, ".EContact Person : %-20.20s\n",
				   cnch_rec.contact);
	fprintf (fout, ".EEffective from : %-12.12s Expires on : %-12.12s\n",
					   ws_date_wef, ws_date_exp);
	fprintf (fout, ".EDue for review : %-12.12s Renewal    : %-12.12s\n",
					   ws_date_rev, ws_date_ren);

	if (envDbMcurr)
	{
		fprintf (fout, ".EPricing Type : %9.9s\n", 
					   cnch_rec.exch_type [0] == 'F' ? "Fixed" : "Variable");
	}
	fprintf (fout, ".B1\n");
}

void
HeadingCncd (void)
{
	if (envDbMcurr)
	{
		/*-----------------------------
		| Manually print first header |
		-----------------------------*/
		PrintRulerCncd ();
		strcpy (err_str, ".C| Item Number      ");
		strcat (err_str, "| Description                              ");
		strcat (err_str, "| Curr| Sale Price   |Disc | Br | Wh ");
		strcat (err_str, "|Supplier|  Acronym   ");
		strcat (err_str, "|Curr | Agreed Cost  |\n");
		fprintf (fout, err_str);
	
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "|------------------------------------------");
		strcat (err_str, "|-----|--------------|-----|----|----");
		strcat (err_str, "|--------|------------");
		strcat (err_str, "|-----|--------------|\n");
		fprintf (fout, err_str);
	
		/*--------------------------------
		| Set up header to be printed at |
		| the top of each page.          |
		--------------------------------*/
		fprintf (fout, ".DS2\n");
		strcpy (err_str, ".C| Item Number      ");
		strcat (err_str, "| Description                              ");
		strcat (err_str, "| Curr| Sale Price   |Disc | Br | Wh ");
		strcat (err_str, "|Supplier|  Acronym   ");
		strcat (err_str, "|Curr | Agreed Cost  |\n");
		fprintf (fout, err_str);
	
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "|------------------------------------------");
		strcat (err_str, "|-----|--------------|-----|----|----");
		strcat (err_str, "|--------|------------");
		strcat (err_str, "|-----|--------------|\n");
		fprintf (fout, err_str);
	}
	else
	{
		/*-----------------------------
		| Manually print first header |
		-----------------------------*/
		PrintRulerCncd ();
		strcpy (err_str, ".C| Item Number      ");
		strcat (err_str, "| Description                              ");
		strcat (err_str, "| Sale Price   |Disc | Br | Wh ");
		strcat (err_str, "|Supplier|  Acronym   ");
		strcat (err_str, "| Agreed Cost  |\n");
		fprintf (fout, err_str);
	
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "|------------------------------------------");
		strcat (err_str, "|--------------|-----|----|----");
		strcat (err_str, "|--------|------------");
		strcat (err_str, "|--------------|\n");
		fprintf (fout, err_str);
	
		/*--------------------------------
		| Set up header to be printed at |
		| the top of each page.          |
		--------------------------------*/
		fprintf (fout, ".DS2\n");
		strcpy (err_str, ".C| Item Number      ");
		strcat (err_str, "| Description                              ");
		strcat (err_str, "| Sale Price   |Disc | Br | Wh ");
		strcat (err_str, "|Supplier|  Acronym   ");
		strcat (err_str, "| Agreed Cost  |\n");
		fprintf (fout, err_str);
	
		strcpy (err_str, ".C|------------------");
		strcat (err_str, "|------------------------------------------");
		strcat (err_str, "|--------------|-----|----|----");
		strcat (err_str, "|--------|------------");
		strcat (err_str, "|--------------|\n");
		fprintf (fout, err_str);
	}
}

void
PrintCncd (void)
{
	int			ccbr;
	int			cccc;
	int			ccsu;
	char		ws_cost [13];
	int			first_cncd = TRUE;


	cncd_rec.hhch_hash = cnch_rec.hhch_hash;
	cncd_rec.line_no = 0;
	cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	while (!cc && cncd_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		inmr_rec.hhbr_hash = cncd_rec.hhbr_hash;
		ccbr = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (ccbr)
			file_err (ccbr, inmr, "DBFIND");

		if (first_cncd == TRUE)
		{
			HeadingCncd ();
			first_cncd = FALSE;
		}

		if (cncd_rec.hhcc_hash == 0L)
		{
			strcpy (ccmr_rec.est_no, "  ");
			strcpy (ccmr_rec.cc_no,  "  ");
		}
		else
		{
			ccmr_rec.hhcc_hash = cncd_rec.hhcc_hash;
			cccc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
			if (cccc)
				file_err (cccc, ccmr, "DBFIND");
		}

		if (cncd_rec.hhsu_hash == 0L)
		{
			strcpy (sumr_rec.crd_no, "      ");
			strcpy (sumr_rec.acronym, "        ");
			strcpy (sumr_rec.curr_code, "   ");
			strcpy (ws_cost, 		"            ");
		}
		else
		{
			sumr_rec.hhsu_hash = cncd_rec.hhsu_hash;
			ccsu = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (ccsu)
				file_err (ccsu, sumr, "DBFIND");

			sprintf (ws_cost, "%12.2f", DOLLARS (cncd_rec.cost));
		}

		if (envDbMcurr)
		{
			fprintf (fout,
	   		".C| %-16.16s | %-40.40s | %3.3s | %12.2f |   %1.1s | %2.2s | %2.2s | %-6.6s | %-10.10s | %3.3s | %12.12s |\n",
			inmr_rec.item_no,
			inmr_rec.description,
			cncd_rec.curr_code,
			DOLLARS (cncd_rec.price),
			cncd_rec.disc_ok,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			sumr_rec.crd_no,
			sumr_rec.acronym,
			sumr_rec.curr_code,
			ws_cost);
		}
		else
		{
			fprintf (fout,
	   		".C| %-16.16s | %-40.40s | %12.2f |   %1.1s | %2.2s | %2.2s | %-6.6s | %-10.10s | %12.12s |\n",
			inmr_rec.item_no,
			inmr_rec.description,
			DOLLARS (cncd_rec.price),
			cncd_rec.disc_ok,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			sumr_rec.crd_no,
			sumr_rec.acronym,
			ws_cost);
		}

		cc = find_rec (cncd, &cncd_rec, NEXT, "r");
	}
	if (first_cncd == FALSE)
		PrintRulerCncd ();
}

void
HeadingCncl (void)
{
	fprintf (fout, ".B1\n");

	/*-----------------------------
	| Manually print first header |
	-----------------------------*/
	fprintf (fout, ".LRP4\n");
	PrintRulerCncl ();

	strcpy (err_str, ".C|                                                C U");
	strcat (err_str, " S T O M E R S   A S S I G N E D   T O   C O N T R A C");
	strcat (err_str, "T                                                 |\n");
	fprintf (fout, err_str);

	strcpy (err_str, ".C|Customer| Customer Name                            ");
	strcat (err_str, "|Customer| Customer Name                            ");
	strcat (err_str, "|Customer| Customer Name                            |\n");
	fprintf (fout, err_str);

	strcpy (err_str, ".C|--------|------------------------------------------");
	strcat (err_str, "|--------|------------------------------------------");
	strcat (err_str, "|--------|------------------------------------------|\n");
	fprintf (fout, err_str);

	/*--------------------------------
	| Set up header to be printed at |
	| the top of each page.          |
	--------------------------------*/
	fprintf (fout, ".DS3\n");
	strcpy (err_str, ".C|                                                C U");
	strcat (err_str, " S T O M E R S   A S S I G N E D   T O   C O N T R A C");
	strcat (err_str, "T                                                 |\n");
	fprintf (fout, err_str);

	strcpy (err_str, ".C|Customer| Customers Name                           ");
	strcat (err_str, "|Customer| Customers Name                           ");
	strcat (err_str, "|Customer| Customers Name                           |\n");
	fprintf (fout, err_str);

	strcpy (err_str, ".C|--------|------------------------------------------");
	strcat (err_str, "|--------|------------------------------------------");
	strcat (err_str, "|--------|------------------------------------------|\n");
	fprintf (fout, err_str);
}

void
PrintCncl (void)
{
	int			cccu;
	char		ws_str [60];
	int			first_cncl = TRUE;

	cncl_rec.hhch_hash = cnch_rec.hhch_hash;
	cncl_rec.hhcu_hash = 0L;
	cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
	while (!cc && cncl_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		if (first_cncl == TRUE)
		{
			HeadingCncl ();
			first_cncl = FALSE;
		}
		cumr_rec.hhcu_hash = cncl_rec.hhcu_hash;
		cccu = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cccu)
			file_err (cccu, cumr, "DBFIND");

		strcpy (err_str, ".C");
		sprintf (ws_str, "| %-6.6s | %-40.40s ",
		                 cumr_rec.dbt_no,
		                 cumr_rec.dbt_name);
		strcat (err_str, ws_str);

		cc = find_rec (cncl, &cncl_rec, NEXT, "r");
		if (!cc && cncl_rec.hhch_hash == cnch_rec.hhch_hash)
		{
			cumr_rec.hhcu_hash = cncl_rec.hhcu_hash;
			cccu = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cccu)
				file_err (cccu, cumr, "DBFIND");

			sprintf (ws_str, "| %-6.6s | %-40.40s ",
			                 cumr_rec.dbt_no,
			                 cumr_rec.dbt_name);
			strcat (err_str, ws_str);
			cc = find_rec (cncl, &cncl_rec, NEXT, "r");
			if (!cc && cncl_rec.hhch_hash == cnch_rec.hhch_hash)
			{
				cumr_rec.hhcu_hash = cncl_rec.hhcu_hash;
				cccu = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (cccu)
					file_err (cccu, cumr, "DBFIND");
	
				sprintf (ws_str, "| %-6.6s | %-40.40s ",
				                 cumr_rec.dbt_no,
				                 cumr_rec.dbt_name);
				strcat (err_str, ws_str);
				cc = find_rec (cncl, &cncl_rec, NEXT, "r");
			}
			else
			{
				sprintf (ws_str, "| %-6.6s | %-40.40s ", " ", " ");
				strcat (err_str, ws_str);
			}
		}
		else
		{
			sprintf (ws_str, "| %-6.6s | %-40.40s ", " ", " ");
			strcat (err_str, ws_str);
			sprintf (ws_str, "| %-6.6s | %-40.40s ", " ", " ");
			strcat (err_str, ws_str);
		}
		strcat (err_str, "|\n");
		fprintf (fout, err_str);
	}
	if (first_cncl == FALSE)
		PrintRulerCncl ();
}

/*===================================
| Search for Contract Header file.  |
===================================*/
void
SrchCnch (
 char*              key_val)
{
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	_work_open (6,0,40);

	abc_selfield (cnch, "cnch_id_no");
	save_rec ("#Number","# Description.");
	
	strcpy (cnch_rec.co_no,comm_rec.co_no);
	sprintf (cnch_rec.cont_no,"%-6.6s", (sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cnch_rec.cont_no, cnch_rec.desc);
		if (cc)
			break;

		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}
	cc = disp_srch ();
	abc_selfield (cnch, "cnch_id_no");
	work_close ();
	if (cc)
	{
		sprintf (cnch_rec.cont_no, "%-6.6s"," ");
		sprintf (cnch_rec.desc, "%-40.40s"," ");
		return;
	}
	
	strcpy (cnch_rec.co_no,comm_rec.co_no);
	sprintf (cnch_rec.cont_no,"%-6.6s", temp_str);
	
	cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "cnch", "DBFIND");
}
int
heading (
 int                scn)            
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlDbMess143),25,0,1);

	line_at (1,0,80);

	box (0,3,80,6);
	line_at (6,1,79);
	line_at (20,0,80);

	print_at (21,0,ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
