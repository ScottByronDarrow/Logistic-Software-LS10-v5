/*=====================================================================
|  Copyright (C) 1999 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_qa_prn.c,v 5.5 2002/07/17 09:57:04 scott Exp $
|  Program Name  : (cr_qa_prt.c) 
|  Program Desc  : (Print Supplier Quanlty Assurance)	
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written : 30/10/95          |
|---------------------------------------------------------------------|
| $Log: cr_qa_prn.c,v $
| Revision 5.5  2002/07/17 09:57:04  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/12/07 06:24:18  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/07 05:44:40  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_qa_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_qa_prn/cr_qa_prn.c,v 5.5 2002/07/17 09:57:04 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>
#include 	<FindSumr.h>

#include    "schema"

struct commRecord   comm_rec;
struct sumrRecord   sumr_rec;
struct qamrRecord   qamr_rec;
struct qasdRecord   qasd_rec;

	int		envCrCo = 0,
			envCrFind = 0;

	char	branchNumber [3];
    char    *nameSpace =   "                                        ";

	FILE	*fout;

	/*
	 * Table names
	 */
	static char 	*data	= "data";

	extern	int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	startSupplierNo [7];
	char	endSupplierNo [7];
	char	startStatus [2];
	char	endStatus [2];
	char	supplierName [2][41];
	char	qaDesc [2][51];
	int		printerNo;
	char 	back [2];
	char 	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "from_supplier",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplierNo},
	{1, LIN, "name1",	 4, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierName [0]},
	{1, LIN, "to_supplier",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier     ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplierNo},
	{1, LIN, "name2",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.supplierName [1]},
	{1, LIN, "from_stat",	 7, 2, CHARTYPE,
		"N", "          ",
		" ", "1", "Start QA Status  ", " ",
		YES, NO,  JUSTLEFT, "1", "9", local_rec.startStatus},
	{1, LIN, "from_sdesc",	 7, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.qaDesc [0]},
	{1, LIN, "to_stat",	 8, 2, CHARTYPE,
		"N", "          ",
		" ", "9", "End QA Status    ", " ",
		YES, NO,  JUSTLEFT, "1", "9", local_rec.endStatus},
	{1, LIN, "to_sdesc",	 8, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.qaDesc [1]},
	{1, LIN, "printerNo",	 10, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background       ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 11, 30, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	 12, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight        ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onightDesc", 12, 30, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.oniteDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local function prototypes
 */
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	RunProgram		 (char *);
void	HeadingOutput	 (void);
void	SrchQamr		 (char *);
int		ProcessFile	 	 (void);
int		heading			 (int);
int		spec_valid		 (int);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 1 && argc != 6)
	{
		print_at (0, 0, mlCrMess121);
		return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	if (argc == 6)
	{
		abc_selfield (sumr, "sumr_qa_id");

		sprintf (local_rec.startSupplierNo, "%-6.6s", argv [1]);
		sprintf (local_rec.endSupplierNo, "%-6.6s", argv [2]);
		sprintf (local_rec.startStatus, "%-1.1s", argv [3]);
		sprintf (local_rec.endStatus, "%-1.1s", argv [4]);
		local_rec.printerNo = atoi (argv [5]);

		dsp_screen ("Printing Quality Assurance By Supplier.",
					comm_rec.co_no, comm_rec.co_name);

		if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout, ".EOF\n");
		pclose (fout);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Setup required parameters
	 */
	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
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

		strcpy (err_str, ML (mlCrMess189));
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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (qamr, qamr_list, QAMR_NO_FIELDS, "qamr_id_no");
	open_rec (qasd, qasd_list, QASD_NO_FIELDS, "qasd_id_no");

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envCrFind) 
	 	 	  ? "sumr_id_no" : "sumr_id_no3");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (qamr);
	abc_fclose (qasd);
	abc_dbclose (data);
}

int
spec_valid (int field)
{
	/*
	 * Validate Supplier
	 */
	if (LCHECK ("from_supplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.supplierName [0], ML ("Start Supplier"));
			DSP_FLD ("name1");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			strcmp (local_rec.startSupplierNo, local_rec.endSupplierNo) > 0)
		{
			print_mess (ML (mlStdMess017));
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.startSupplierNo, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [0], sumr_rec.crd_name);
		DSP_FLD ("name1");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to_supplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.supplierName [1], ML ("End Supplier"));
			DSP_FLD ("name2");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.endSupplierNo, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startSupplierNo, local_rec.endSupplierNo) > 0)
		{
			errmess (ML (mlStdMess018));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.supplierName [1], sumr_rec.crd_name);
		DSP_FLD ("name2");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Instruction Code.
	 */
	if (LCHECK ("from_stat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.qaDesc [0], ML ("Start Status"));
			DSP_FLD ("from_sdesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchQamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qamr_rec.co_no, comm_rec.co_no);
		strcpy (qamr_rec.br_no, comm_rec.est_no);
		strcpy (qamr_rec.qa_status, local_rec.startStatus);
		cc = find_rec (qamr, &qamr_rec, COMPARISON, "r");
		if (cc) 
 		{
 			print_mess (ML (mlStdMess074));
 
 			sleep (sleepTime);
 			clear_mess ();
 			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY && 
			strcmp (local_rec.startStatus, local_rec.endStatus) > 0)
		{
			print_mess (ML (mlStdMess017));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.qaDesc [0], "%-50.50s", qamr_rec.qa_desc);
		DSP_FLD ("from_sdesc");
		
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Instruction Code.
	 */
	if (LCHECK ("to_stat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.qaDesc [1], ML ("End Status"));
			DSP_FLD ("to_sdesc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchQamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (qamr_rec.co_no, comm_rec.co_no);
		strcpy (qamr_rec.br_no, comm_rec.est_no);
		strcpy (qamr_rec.qa_status, local_rec.endStatus);
		cc = find_rec (qamr, &qamr_rec, COMPARISON, "r");
		if (cc) 
 		{
 			print_mess (ML (mlStdMess074));
 
 			sleep (sleepTime);
 			clear_mess ();
 			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startStatus, local_rec.endStatus) > 0)
		{
			errmess (ML (mlStdMess018));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.qaDesc [1], "%-50.50s", qamr_rec.qa_desc);
		DSP_FLD ("to_sdesc");
		
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
				(local_rec.back [0] == 'Y') ? "Yes" : "No ");
		
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.oniteDesc, 
			    (local_rec.onite [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
RunProgram (
	char	*programName)
{
	CloseDB (); 
	FinishProgram ();

	if (local_rec.onite [0] == 'Y')
	{
		sprintf
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"",
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.startStatus,
			local_rec.endStatus,
			local_rec.printerNo,
			err_str
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.startStatus,
			local_rec.endStatus,
			local_rec.printerNo
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*
 * Start Out Put To Standard Print.
 */
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n",
			 comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  : %s - %s\n",
			 comm_rec.est_no,clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESUPPLIER QUALITY ASSURANCE REPORT.\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n",SystemTime ());
	fprintf (fout, ".CFrom Supplier %s to Supplier %s / From status code %s to status code %s.\n",
			 local_rec.startSupplierNo,
			 local_rec.endSupplierNo,
			 local_rec.startStatus,
			 local_rec.endStatus);

	fprintf (fout, ".R=========");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "==============================================================\n");

	fprintf (fout, "=========");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "==============================================================\n");

	fprintf (fout, "!SUPPLIER");
	fprintf (fout, "!      S U P P L I E R    N A M E        ");
	fprintf (fout, "! QA ");
	fprintf (fout, "! QA EXPIRY ");
	fprintf (fout, "!       SUPPLIER ADDRESS AND QUALITY ASSURANCE DETAILS.      !\n");
	fprintf (fout, "! NUMBER ");
	fprintf (fout, "!                                        ");
	fprintf (fout, "!CODE");
	fprintf (fout, "!    DATE   ");
	fprintf (fout, "!                                                            !\n");

	fprintf (fout, "!--------");
	fprintf (fout, "!----------------------------------------");
	fprintf (fout, "!----");
	fprintf (fout, "!-----------");
	fprintf (fout, "!------------------------------------------------------------!\n");
	fflush (fout);
}

int
ProcessFile (void)
{
	int		i;
	char	sumr_adr [4][41];

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.qa_status, local_rec.startStatus);
	strcpy (sumr_rec.crd_no, local_rec.startSupplierNo);

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");

	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (sumr_rec.est_no, branchNumber) && 
		      	  strcmp (sumr_rec.crd_no, local_rec.startSupplierNo) >= 0 && 
		      	  strcmp (sumr_rec.crd_no, local_rec.endSupplierNo) <= 0 &&
		      	  strcmp (sumr_rec.qa_status, local_rec.startStatus) >= 0 && 
		      	  strcmp (sumr_rec.qa_status, local_rec.endStatus) <= 0) 
	{
		dsp_process ("Supplier", sumr_rec.crd_no);

		strcpy (sumr_adr [0], sumr_rec.adr1);
		strcpy (sumr_adr [1], sumr_rec.adr2);
		strcpy (sumr_adr [2], sumr_rec.adr3);
		strcpy (sumr_adr [3], sumr_rec.adr4);

		fprintf (fout,"! %6.6s ", sumr_rec.crd_no);
		fprintf (fout,"!%40.40s", sumr_rec.crd_name);
		fprintf (fout,"! %s  ", sumr_rec.qa_status);
		fprintf (fout,"!%10.10s ", DateToString (sumr_rec.qa_expiry));
		fprintf (fout,"!%-60.60s!\n", sumr_adr [0]);
		for (i = 1; i < 4; i++)
		{
			if (strcmp (sumr_adr [i], nameSpace))
			{
				fprintf (fout, "!        ");
				fprintf (fout, "!                                        ");
				fprintf (fout, "!    ");
				fprintf (fout, "!           ");
				fprintf (fout, "!%-60.60s!\n", sumr_adr [i]);
			}
		}
		qasd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		qasd_rec.line_no   = 0;
		cc = find_rec (qasd, &qasd_rec, GTEQ, "r");

		while (!cc && qasd_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			fprintf (fout, "!        ");
			fprintf (fout, "!                                        ");
			fprintf (fout, "!    ");
			fprintf (fout, "!%10.10s ", 
					 (!qasd_rec.line_no) ? "QA DETAILS" : " ");
			fprintf (fout, "!%-60.60s!\n",	qasd_rec.desc);
			cc = find_rec (qasd, &qasd_rec, NEXT, "r");
		}
		fprintf (fout, "!--------");
		fprintf (fout, "!----------------------------------------");
		fprintf (fout, "!----");
		fprintf (fout, "!-----------");
		fprintf (fout, "!------------------------------------------------------------!\n");
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	fflush (fout);
	return (EXIT_SUCCESS);
}

/*
 * Search for Quality Assurance code.
 */
void
SrchQamr (
 char *	key_val)
{
	work_open ();
	save_rec ("#QA", "#QA Description");
	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", key_val);

	cc = find_rec (qamr, &qamr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (qamr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (qamr_rec.br_no, comm_rec.est_no) &&
		   !strncmp (qamr_rec.qa_status, key_val, strlen (key_val)))
	{
		cc = save_rec (qamr_rec.qa_status, qamr_rec.qa_desc);
		if (cc)
			break;

		cc = find_rec (qamr, &qamr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qamr_rec.co_no, comm_rec.co_no);
	strcpy (qamr_rec.br_no, comm_rec.est_no);
	sprintf (qamr_rec.qa_status, "%-1.1s", temp_str);
	cc = find_rec (qamr, &qamr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, qamr, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlCrMess120), 25, 0, 1);


	box (0, 3, 80, 9);
	line_at (1,0,80);
	line_at (6,1,79);
	line_at (9,1,79);
	line_at (20,0,80);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21, 0,  err_str, comm_rec.co_no, comm_rec.co_name);

	strcpy (err_str, ML (mlStdMess039));
	print_at (22, 0, err_str, comm_rec.est_no,comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}
