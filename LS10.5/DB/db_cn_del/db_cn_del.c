/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cn_del.c,v 5.5 2002/07/17 09:57:05 scott Exp $
|  Program Name  : (db_cn_del.c)
|  Program Desc  : (Delete Customer Contracts)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 27/09/93         |
|---------------------------------------------------------------------|
| $Log: db_cn_del.c,v $
| Revision 5.5  2002/07/17 09:57:05  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/11/22 02:52:23  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/11/22 02:14:54  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cn_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cn_del/db_cn_del.c,v 5.5 2002/07/17 09:57:05 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cnchRecord	cnch_rec;
struct cncdRecord	cncd_rec;
struct cnclRecord	cncl_rec;
struct cohrRecord	cohr_rec;
struct sohrRecord	sohr_rec;
struct qthrRecord	qthr_rec;
struct qtphRecord	qtph_rec;

	int		envDbCo = 0;

	char	branchNo [3];

	long	lsystemDate;

	char	*data	= "data";
		
	FILE	*fout;

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
	char	backDesc [11];
	char	onight [2];
	char	onightDesc [11];
	char	dummy [11];
	char	systemDate [11];
} local_rec;

extern		int		TruePosition;
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

#include	<wild_search.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	ContractInUse 		(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	CloseDB 			(void);
void 	DeleteCncd 			(void);
void 	DeleteCnch 			(void);
void 	DeleteCncl 			(void);
void 	head_output 		(void);
void 	OpenDB 				(void);
void 	PrintCnch 			(char *);
void 	PrintRuler 			(void);
void 	ProcessFile 		(void);
void 	RunProgram 			(char *);
void 	shutdown_prog 		(void);
void 	SrchCnch 			(char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int      argc,
	char	*argv [])
{
	TruePosition	=	TRUE;
	if (argc != 1 && argc != 4)
	{
		/*-----------------------------------------------------
		| Usage : %s <Start Contract> <End Contract> <printerNo>\n |
		-----------------------------------------------------*/
		print_at (0,0, mlDbMess003, argv [0]);
        return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	OpenDB ();


	if (argc == 4)
	{
		sprintf (local_rec.startContractNo, "%-6.6s", argv [1]);
		sprintf (local_rec.endContractNo, "%-6.6s", argv [2]);
		local_rec.printerNo = atoi (argv [3]);

		dsp_screen ("Processing : Deleting Expired Customer Contracts.",
					comm_rec.co_no,comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");


		head_output ();
		ProcessFile ();
		fprintf (fout,".EOF\n");

		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();				/*  sets terminal from termcap	*/
	set_tty ();              /*  get into raw mode			*/
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
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
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
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no");
	open_rec (cncl, cncl_list, CNCL_NO_FIELDS, "cncl_hhch_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_cont_no");
	open_rec (qtph, qtph_list, QTPH_NO_FIELDS, "qtph_id_no");
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
	abc_fclose (cohr);
	abc_fclose (sohr);
	abc_fclose (qthr);
	abc_fclose (qtph);
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
			sprintf (local_rec.startContractDesc, "%-40.40s", "Start Contract");
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
			/*-----------------------------------------------------------
			| Start Contract %s Must Not Be GREATER THAN End Contract %s|
			-----------------------------------------------------------*/
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
			/*----------------------------
			| Contract %s is not on file |
			----------------------------*/
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
			sprintf (local_rec.endContractDesc, "%-40.40s", "End   Contract");
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
			/*----------------------------
			| Contract %s is not on file |
			----------------------------*/
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startContractNo, local_rec.endContractNo) > 0)
		{
			/*------------------------------------------
			| End Contract %s may not be less than %s |
			------------------------------------------*/
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
			/*------------------------
			| Invalid Printer Number |
			------------------------*/
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
            ML ("Customer Contract Delete.")
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
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*==================================
| Start Output To Standard Print . |
==================================*/
void
head_output (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L132\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n", comm_rec.co_no, comm_rec.co_name);
	fprintf (fout, ".EBranch  : %s - %s\n", comm_rec.est_no,comm_rec.est_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECUSTOMERS CONTRACT DELETION AUDIT TRAIL\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "===============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======================");
	fprintf (fout, "====================");
	fprintf (fout, "============================\n");
 
	fprintf (fout, "|   CONTRACT   ");
	fprintf (fout, "|                                            ");
	fprintf (fout, "|                        ");
	fprintf (fout, "|                    ");
	fprintf (fout, "                      |\n");
 
	fprintf (fout, "|    NUMBER    ");
	fprintf (fout, "| DESCRIPTION                                ");
	fprintf (fout, "| CONTACT NAME           ");
	fprintf (fout, "| COMMENTS           ");
	fprintf (fout, "                      |\n");
 
	PrintRuler ();

	fprintf (fout, ".R===============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======================");
	fprintf (fout, "====================");
	fprintf (fout, "============================\n");
 
	fflush (fout);
}

void
PrintRuler (void)
{
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------------------------------------");
	fprintf (fout, "|------------------------");
	fprintf (fout, "|--------------------");
	fprintf (fout, "----------------------|\n");
 
	fflush (fout);
}

void
ProcessFile (void)
{
	strcpy (cnch_rec.co_no, comm_rec.co_no);
	strcpy (cnch_rec.cont_no, local_rec.startContractNo);
	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (cnch_rec.co_no, comm_rec.co_no) &&
		  (strcmp (cnch_rec.cont_no, local_rec.endContractNo) <= 0))
	{
		dsp_process ("Contract No. : ", cnch_rec.cont_no);
		if (!ContractInUse ())
		{
			DeleteCncl ();
			DeleteCncd ();
			DeleteCnch ();
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			strcpy (cnch_rec.cont_no, local_rec.startContractNo);
			cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
		}
		else
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}
}

int
ContractInUse (void)
{
	/* Check Expiry date */

	if (cnch_rec.date_exp >= lsystemDate)
	{
		PrintCnch (ML ("Contract has not yet expired."));
		return (EXIT_FAILURE);
	}

	/* Check no Sales Orders pending */

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, "  ");
	strcpy (sohr_rec.order_no, "        ");
	sohr_rec.hhcu_hash = 0L;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (sohr_rec.co_no, comm_rec.co_no))
	{
		if (strncmp (sohr_rec.cont_no, cnch_rec.cont_no, 6) == 0)
		{
			sprintf (err_str,ML ("Still in use for Sales Order %s"),sohr_rec.order_no);
			PrintCnch (err_str);
			return (EXIT_FAILURE);
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}


	/* Check no Invoices pending */

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, "  ");
	strcpy (cohr_rec.inv_no, "        ");
	strcpy (cohr_rec.type, " ");
	cohr_rec.hhcu_hash = 0L;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (cohr_rec.co_no, comm_rec.co_no))
	{
		if (strncmp (cohr_rec.cont_no, cnch_rec.cont_no, 6) == 0)
		{
			sprintf (err_str, ML ("Still in use for Invoice %s"), cohr_rec.inv_no);
			PrintCnch (err_str);
			return (EXIT_FAILURE);
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	/* Check no Quotations pending */

	strcpy (qthr_rec.cont_no, cnch_rec.cont_no);
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp (qthr_rec.cont_no, cnch_rec.cont_no))
	{
		if (!strcmp (qthr_rec.co_no, comm_rec.co_no))
		{
			sprintf (err_str, ML("Still in use for Quotation %10ld"), 
							 qthr_rec.hhqt_hash);
			PrintCnch (err_str);
			return (EXIT_FAILURE);
		}
		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}

	/* Check no Prospects pending */

	strcpy (qtph_rec.co_no, comm_rec.co_no);
	qtph_rec.hhph_hash = 0L;
	cc = find_rec (qtph, &qtph_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (qtph_rec.co_no, comm_rec.co_no))
	{
		if (strncmp (qtph_rec.cont_no, cnch_rec.cont_no, 6) == 0)
		{
			sprintf (err_str, ML ("Still in use for Prospect %10ld"), 
							 qtph_rec.hhph_hash);
			PrintCnch (err_str);
			return (EXIT_FAILURE);
		}
		cc = find_rec (qtph, &qthr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

void
DeleteCncl (void)
{
	cncl_rec.hhch_hash = cnch_rec.hhch_hash;
	cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
	while (!cc && cncl_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		cc = abc_delete (cncl);
	  	if (cc)
			file_err (cc, cncl, "DBDELETE");

		cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
	}
}

void
DeleteCncd (void)
{
	cncd_rec.hhch_hash 	= cnch_rec.hhch_hash;
	cncd_rec.line_no 	= 0;
	cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	while (!cc && cncd_rec.hhch_hash == cnch_rec.hhch_hash)
	{
		cc = abc_delete (cncd);
	  	if (cc)
			file_err (cc, cncd, "DBDELETE");

		cc = find_rec (cncd, &cncd_rec, GTEQ, "r");
	}
}

void
DeleteCnch (void)
{
	PrintCnch (ML ("DELETED"));
	cc = abc_delete (cnch);
	if (cc)
		file_err (cc, cnch, "DBDELETE");
}

void
PrintCnch (
	char	*ws_msg)
{
	fprintf (fout,
	   "|    %-6.6s    |  %-40.40s  |  %-20.20s  | %-40.40s |\n",
		cnch_rec.cont_no,
		cnch_rec.desc,
		cnch_rec.contact,
		ws_msg);
}

/*===================================
| Search for Contract Header file.  |
===================================*/
void
SrchCnch (
	char	*key_val)
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
		file_err (cc, cnch, "DBFIND");
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

	/*---------------------------
	| Customer Contract Deletion |
	---------------------------*/
	sprintf (err_str, " %s ", ML (mlDbMess004));
	rv_pr (err_str, 25,0,1);

	box (0,3,80,6);
	line_at (1,0,80);
	line_at (6,1,79);
	line_at (20,0,80);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
