/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_sk_prt.c,v 5.4 2002/07/17 09:57:08 scott Exp $
|  Program Name  : (db_sk_prt.c)
|  Program Desc  : (Print Customer specific item codes)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 13/09/93         |
|---------------------------------------------------------------------|
| $Log: db_sk_prt.c,v $
| Revision 5.4  2002/07/17 09:57:08  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/27 09:42:30  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_sk_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_sk_prt/db_sk_prt.c,v 5.4 2002/07/17 09:57:08 scott Exp $";

#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inexRecord	inex_rec;
struct cuitRecord	cuit_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;

	int		envDbCo 	= 0,
			envDbFind 	= 0;

	char	branchNo [3];

	char	*data	= "data",
			*inmr2	= "inmr2";

	FILE	*fout;

	extern	int		TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy 				[11];
	char	startCustomerNo 	[7];
	char	startCustomerName 	[41];
	char	startItemNo 		[17];
	char	startItemDesc 		[41];
	char	endCustomerNo 		[7];
	char	endCustomerName 	[41];
	char	endItemNo 			[17];
	char	endItemDesc 		[41];
	char	printerStr 			[3];
	char 	back 				[2];
	char 	backDesc 			[4];
	char	onight 				[2];
	char	onightDesc 			[4];
	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startCustomerNo",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Start Customer  ", "Enter First Customer Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCustomerNo},
	{1, LIN, "startCustomerName",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startCustomerName},
	{1, LIN, "endCustomerNo",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Customer    ", "Enter Last Customer Number",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCustomerNo},
	{1, LIN, "endCustomerName",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endCustomerName},
	{1, LIN, "startItemNo",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "                ", "Start Item      ", "Start Item Number ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItemNo},
	{1, LIN, "startItemDesc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItemNo",	 8, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", "End Item        ", "End Item Number ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItemNo},
	{1, LIN, "endItemDesc",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},
	{1, LIN, "printerNo",	 10, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer         ", "Printer Number",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background      ", "Print Report In The Background ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 11, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight", 12, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight       ", "Print Report In Overnight Batch",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 12, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	RunProgram 		(char *);
void 	ExitBack 		(void);
void 	HeadingOutput 	(void);
void 	PrintRuler 		(void);
void 	ProcessFile 	(void);
int 	spec_valid 		(int);
int 	heading 		(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	if (argc != 1 && argc != 6)
	{
		print_at (0,0,mlDbMess709,argv [0]);
        return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (argc == 6)
	{
		sprintf (local_rec.startCustomerNo, "%-6.6s", 	argv [1]);
		sprintf (local_rec.endCustomerNo, 	"%-6.6s", 	argv [2]);
		sprintf (local_rec.startItemNo, 	"%-16.16s", argv [3]);
		sprintf (local_rec.endItemNo, 		"%-16.16s", argv [4]);
		local_rec.printerNo = atoi (argv [5]);

		dsp_screen ("Processing : Printing Customer Specific Item Codes.",
					comm_rec.co_no, comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		ProcessFile ();
		pclose (fout);
		ExitBack ();
        return (EXIT_SUCCESS);
	}

	/*
	 * Setup required parameters
	 */
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/
	init_vars (1);			/*  set default values			*/

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

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

	abc_alias (inmr2, inmr);
	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
		  					       					     	: "cumr_id_no3");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cuit);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Customer Number And Allow Search.
	 */
	if (LCHECK ("startCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCustomerNo, "      ");
			strcpy (local_rec.startCustomerName, ML ("START CUSTOMER"));
			DSP_FLD ("startCustomerNo");
			DSP_FLD ("startCustomerName");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.startCustomerNo));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startCustomerName, cumr_rec.dbt_name);
		DSP_FLD ("startCustomerName");
	}

	/*
	 * Validate Customer Number And Allow Search.
	 */
	if (LCHECK ("endCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCustomerNo, "~~~~~~");
			strcpy (local_rec.endCustomerName, ML ("END CUSTOMER"));
			DSP_FLD ("endCustomerNo");
			DSP_FLD ("endCustomerName");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.endCustomerNo));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endCustomerName, cumr_rec.dbt_name);
		DSP_FLD ("endCustomerName");
	}

	/*----------------------------------------------
	| Validate Start Item Number And Allow Search. |
	----------------------------------------------*/
	if (LCHECK ("startItemNo")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.startItemNo, "                ");
			strcpy (local_rec.startItemDesc, ML ("START ITEM"));
			DSP_FLD ("startItemNo");
			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.startItemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.startItemNo, inmr_rec.item_no);
		strcpy (local_rec.startItemDesc, inmr_rec.description);
		DSP_FLD ("startItemNo");
		DSP_FLD ("startItemDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Item Number And Allow Search.
	 */
	if (LCHECK ("endItemNo")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItemNo, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.endItemDesc, ML ("END ITEM"));
			DSP_FLD ("endItemNo");
			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.endItemDesc, inmr_rec.description);

		DSP_FLD ("endItemNo");
		DSP_FLD ("endItemDesc");

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
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') ? 
								    ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, (local_rec.onight [0] == 'Y') ? 
									  ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
 char*              prog_name)
{
	char	ws_work [17];

	sprintf (local_rec.printerStr,"%d",local_rec.printerNo);
	if (strcmp (local_rec.endItemNo, local_rec.startItemNo) < 0)
	{
		strcpy (ws_work, local_rec.startItemNo);
		strcpy (local_rec.startItemNo, local_rec.endItemNo);
		strcpy (local_rec.endItemNo, ws_work);
	}
	
	if (strcmp (local_rec.endCustomerNo, local_rec.startCustomerNo) < 0)
	{
		strcpy (ws_work, local_rec.startCustomerNo);
		strcpy (local_rec.startCustomerNo, local_rec.endCustomerNo);
		strcpy (local_rec.endCustomerNo, ws_work);
	}
	
	rset_tty ();

	clear ();
	print_at (0,0,ML (mlStdMess035));
	strcpy (err_str,ML (mlDbMess229));

	CloseDB (); 
	FinishProgram ();

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.startCustomerNo,
				local_rec.endCustomerNo,
				local_rec.startItemNo,
				local_rec.endItemNo,
				local_rec.printerStr,
				err_str, (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.startCustomerNo,
				local_rec.endCustomerNo,
				local_rec.startItemNo,
				local_rec.endItemNo,
				local_rec.printerStr, (char *)0);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.startCustomerNo,
			local_rec.endCustomerNo,
			local_rec.startItemNo,
			local_rec.endItemNo,
			local_rec.printerStr, (char *)0);
	}
}

void
ExitBack (void)
{
	CloseDB (); 
	FinishProgram ();
}



/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L125\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n",
				comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  : %s - %s\n",
				comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECUSTOMER SPECIFIC ITEM CODES\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R==========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===========");
	fprintf (fout, "===============\n");

	fprintf (fout, "==========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=============");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "===========");
	fprintf (fout, "===============\n");

	fprintf (fout, "|CUSTOMERS ITEM NO ");
	fprintf (fout, "| CUSTOMERS ITEM DESCRIPTION               ");
	fprintf (fout, "| STANDARD ITEM NO ");
	fprintf (fout, "| STANDARD ITEM DESCRIPTION                |\n");
	PrintRuler ();
	fflush (fout);
}

void
PrintRuler (void)
{
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------|\n");

	fflush (fout);
}


void
ProcessFile (void)
{
	int		data_found = FALSE;
	int		dbtr_found = FALSE;
	char	dbtr_line [132];

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	strcpy (cumr_rec.dbt_no, pad_num (local_rec.startCustomerNo));
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && 
		  (strcmp (cumr_rec.co_no, comm_rec.co_no) == 0) &&
		  (!strcmp (cumr_rec.est_no, branchNo) || envDbFind) &&
		  (strcmp (cumr_rec.dbt_no, local_rec.startCustomerNo) >= 0) &&
		  (strcmp (cumr_rec.dbt_no, local_rec.endCustomerNo) <= 0))
	{
		cuit_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cuit_rec.item_no, "                ");
	
		cc = find_rec (cuit, &cuit_rec, GTEQ, "r");
	
		while (!cc && cuit_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			inmr_rec.hhbr_hash = cuit_rec.hhbr_hash;
	
			if (find_rec (inmr2, &inmr_rec, EQUAL, "r"))
			{
				cc = find_rec (cuit, &cuit_rec, NEXT, "r");
				continue;
			}
			if ((strcmp (inmr_rec.item_no, local_rec.startItemNo) >= 0) &&
				 (strcmp (inmr_rec.item_no, local_rec.endItemNo) <= 0))
			{
				if (data_found == FALSE)
					HeadingOutput ();

				if (dbtr_found == FALSE)
				{
					dsp_process ("Customer : ", cumr_rec.dbt_no);
					fprintf (fout,".LRP2\n");
					sprintf (dbtr_line, "|CUSTOMER: %6.6s  ", cumr_rec.dbt_no);
					sprintf (dbtr_line, "%s  %-40.40s ", 
									   dbtr_line, 	cumr_rec.dbt_name);
					sprintf (dbtr_line, "%s  Item Code Status %9.9s ", 
									   dbtr_line, 
									  (cumr_rec.item_codes [0] == 'Y') ?
									   "Active   " : "Inactive ");
					sprintf (dbtr_line, "%s  %-30.30s |\n", dbtr_line, " ");
					fprintf (fout, ".PD%s", dbtr_line);
					if (data_found == TRUE)
						fprintf (fout, "%s", dbtr_line);
				}
				fprintf (fout, "| %16.16s ",  	cuit_rec.item_no); 
				fprintf (fout, "| %-40.40s ",	cuit_rec.item_desc);
				fprintf (fout, "| %16.16s ",  	inmr_rec.item_no); 
				fprintf (fout, "| %-40.40s |\n",inmr_rec.description);
				data_found = TRUE;
				dbtr_found = TRUE;
			}
			cc = find_rec (cuit, &cuit_rec, NEXT, "r");
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		if (!cc && dbtr_found == TRUE)
		{
			fprintf (fout, "|-----------------------------------------");
			fprintf (fout, "-----------------------------------------");
			fprintf (fout, "-----------------------------------------|\n");
		}
		dbtr_found = FALSE;
	}
	if (data_found == TRUE)
		fprintf (fout,".EOF\n");
	
	fflush (fout);
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
	rv_pr (ML (mlDbMess217), 25, 0, 1);

	box (0, 3, 80, 9);
	line_at (1,0,80);
	line_at (6,1,79);
	line_at (9,1,79);
	line_at (20,0,80);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, clip (comm_rec.co_short));
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str,comm_rec.est_no, clip (comm_rec.est_short));
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
