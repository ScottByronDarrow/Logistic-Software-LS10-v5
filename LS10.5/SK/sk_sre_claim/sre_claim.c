/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sre_claim.c,v 5.4 2002/07/17 09:58:00 scott Exp $
|  Program Name  : (sk_sre_claim.c)                                   |
|  Program Desc  : (Supplier Rebates Claim Report.              )     |
|                  (Links To sk_sclaimrprt                      )     |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 04/11/93         |
|---------------------------------------------------------------------|
| $Log: sre_claim.c,v $
| Revision 5.4  2002/07/17 09:58:00  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/19 02:55:32  robert
| Updated to fixed overlapping of descriptions
|
| Revision 5.2  2001/08/09 09:20:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:34  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:21:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:30  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:11:58  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/06/15 02:39:05  scott
| Updated to change FindSumr () local routine to IntFindSumr to ensure no
| conflict will exist with new routine FindSumr () that is about to be
| introduced.
|
| Revision 1.16  2000/02/18 07:10:20  ronnel
| 18/02/2000 SC2017 Modified to fix the printing.
|
| Revision 1.15  1999/12/06 01:31:19  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.14  1999/11/11 06:00:07  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.13  1999/11/03 07:32:34  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.12  1999/10/20 01:38:57  nz
| Updated for remainder of old routines.
|
| Revision 1.11  1999/10/13 02:42:15  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.10  1999/10/12 21:20:42  scott
| Updated by Gerry from ansi project.
|
| Revision 1.9  1999/10/08 05:32:54  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:20:44  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
---------------------------------------------------------------------*/
char	*PNAME = "$RCSfile: sre_claim.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sre_claim/sre_claim.c,v 5.4 2002/07/17 09:58:00 scott Exp $";
#define CCMAIN

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inrbRecord	inrb_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct suphRecord	suph_rec;
struct sumrRecord	sumr_rec;
struct suraRecord	sura_rec;


	double	*inrb_reb_qty	=	&inrb_rec.reb_qty1;
	double	*inrb_reb_val	=	&inrb_rec.reb_val1;

	char	*data = "data";

	char	branchNumber [3];
	int		envVarCrCo = FALSE;
	int		envVarCrFind  = FALSE;
	long	fromDate;
	long	toDate;
	int		FirstTime;
	int		REPRINT = FALSE;

	FILE	*fsort, *fout;

struct	{
	char	dummy [11];
	char	ssupp [7];
	char	ssuppdesc [41];
	char	esupp [7];
	char	esuppdesc [41];
	char	back [2];
	char	backdesc [4];
	char	onite [2];
	char	onitedesc [4];
	int		printerNumber;
	long	date;
	char	lpstr [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "ssupp",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier       :", "Enter Start Supplier, Default = Start Of File, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ssupp},
	{1, LIN, "ssuppdesc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ssuppdesc},
	{1, LIN, "esupp",	 5, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End Supplier         :", "Enter End Supplier, Default = End Of File, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.esupp},
	{1, LIN, "esuppdesc",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.esuppdesc},
	{1, LIN, "printerNumber",	 7, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number      :  ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "back",	8, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Background          :  ", "Enter Y[es] or N[o]",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backdesc",	8, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backdesc},
	{1, LIN, "onite",	9, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight           :  ", "Enter Y[es] or N[o]",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onitedesc",9, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.onitedesc},
	{2, LIN, "supp",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier             :", "Enter Supplier For Reprint, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ssupp},
	{2, LIN, "suppdesc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ssuppdesc},
	{2, LIN, "date", 6, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Date To Reprint     :  ", "Enter Date To Reprint As At ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.date},
	{2, LIN, "printerNumber",	 7, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number      :  ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  heading (int scn);
int  spec_valid (int field);
void Process (void);
int  IntFindSumr (char *rec, int Errors);
void RunProg (char *name, char *description);
void HeadOut (void);
void OutPutInrb (void);
void DefineSection (void);
int  LoopInrb (float *qty, float *wgt, double *tot, double *reb);
void PrntTots (float *qty, float *wgt, double *tot, double *reb);
void LoadSuph (void);
void PutIntoSort (void);
void PrintLines (float *supQty, float *supWgt, double *supTot, double *TotReb);
int  NotActive (void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	if (strncmp (argv [0], "sk_sre_claim", 12))
		REPRINT = TRUE;

	if (!REPRINT && argc != 2 && argc != 4)
	{
		print_at (0,0,mlSkMess210, argv [0]);
		print_at (1,0,mlSkMess111, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();

	if (argc != 4)
	{
		SETUP_SCR (vars);
		set_masks ();
		if (!REPRINT)
			init_vars (1);
		else
			init_vars (2);
	}

	OpenDB ();

	envVarCrCo = atoi (get_env ("CR_CO"));
	envVarCrFind  = atoi (get_env ("CR_FIND"));

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	if (!REPRINT && argc == 4)
	{
		local_rec.printerNumber = atoi (argv [1]);

		sprintf (local_rec.ssupp, "%6.6s", argv [2]);
		sprintf (local_rec.esupp, "%6.6s", argv [3]);

		Process ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;

		if (!REPRINT)
		{
			init_vars (1);
			heading (1);
			entry (1);
		}
		else
		{
			init_vars (2);
			heading (2);
			entry (2);
		}

		if (restart || prog_exit)
			continue;

		if (!REPRINT)
		{
			heading (1);
			scn_display (1);
			edit (1);
		}
		else
		{
			heading (2);
			scn_display (2);
			edit (2);
		}

		if (restart)
			continue;

		RunProg (argv [0], argv [1]);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, 
			  (envVarCrFind) ? "sumr_id_no3" : "sumr_id_no");
	open_rec (inrb, inrb_list, INRB_NO_FIELDS, "inrb_id_no1");
	open_rec (sura, sura_list, SURA_NO_FIELDS, "sura_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_buy");
	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_hhbr_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (inrb);
	abc_fclose (sura);
	abc_fclose (inmr);
	abc_fclose (suph);
	abc_fclose (pocr);
	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	if (REPRINT)
		rv_pr (ML (mlSkMess368), (40 - (strlen (ML (mlSkMess368)) / 2)), 0, 1);
	else
		rv_pr (ML (mlSkMess367), (40 - (strlen (ML (mlSkMess367)) / 2)), 0, 1);

	move (0, 1);
	line (80);

	if (!REPRINT)
	{
		move (0, 6);
		line (80);
		box (0, 3, 80, 6);		
	}
	else
	{
		move (0, 5);
		line (80);
		box (0, 3, 80, 4);		
	}

	move (0, 21);
	line (80);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 0,err_str, comm_rec.co_no, comm_rec.co_name);


	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("date"))
	{
		if (local_rec.date > TodaysDate ())
		{
			print_mess (ML (mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| supplier Validate Creditor Number. 
	---------------------------*/
	if (LCHECK ("ssupp"))
	{
		if (REPRINT)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.ssupp, "%6.6s", " ");
			sprintf (local_rec.ssuppdesc, "%-40.40s", "Start Of File ");
			DSP_FLD ("ssuppdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.ssupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.ssupp, sumr_rec.crd_no);
		strcpy (local_rec.ssuppdesc, sumr_rec.crd_name);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		DSP_FLD ("ssuppdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esupp"))
	{
		if (REPRINT)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.esupp, "%6.6s", "~~~~~~");
			sprintf (local_rec.esuppdesc, "%-40.40s", "End Of File ");
			DSP_FLD ("esuppdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.esupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.esuppdesc, sumr_rec.crd_name);
		strcpy (local_rec.esupp, sumr_rec.crd_no);

		if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("esuppdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("supp"))
	{
		if (!REPRINT)
			return (EXIT_SUCCESS);

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.ssupp, TRUE))
			return (EXIT_FAILURE);

		/*----------------------
		| reprint so want both
		| start and end to be same
		--------------------------*/
		strcpy (local_rec.ssupp, sumr_rec.crd_no);
		strcpy (local_rec.ssuppdesc, sumr_rec.crd_name);
		strcpy (local_rec.esupp, sumr_rec.crd_no);
		strcpy (local_rec.esuppdesc, sumr_rec.crd_name);

		DSP_FLD ("suppdesc");
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
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.backdesc, "Yes");
		else
			strcpy (local_rec.backdesc, "No ");

		DSP_FLD ("backdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'Y')
			strcpy (local_rec.onitedesc, "Yes");
		else
			strcpy (local_rec.onitedesc, "No ");

		DSP_FLD ("onitedesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=========================
| read through entered range
| of suppliers, for each supplier 
| reset the suppliers total,
| and read thru all sura belonging
| to that supplier, for each sura
| find the corresponding inrb record
| and all inmrs that have belong to the
| buying group of the the sura
=========================================*/
void  
Process (
 void)
{
	float	suppQty = 0.00;
	float 	suppWgt = 0.00;
	double 	suppTot = 0.00;
	double 	suppReb = 0.00;

	int		fstTime = TRUE;

	dsp_screen ("Rebate Claims", comm_rec.co_no, comm_rec.co_name);
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	sprintf (sumr_rec.crd_no, "%-6.6s", local_rec.ssupp);

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc &&
			!strcmp (sumr_rec.co_no, comm_rec.co_no) &&
			 (strcmp (sumr_rec.crd_no, local_rec.esupp) < 1)
		 )
		  {
				dsp_process ("Supplier ", sumr_rec.crd_no);

				suppQty = 0.00;
				suppWgt = 0.00;
				suppTot = 0.00;
				suppReb = 0.00;

				if (fstTime)
				{
					HeadOut ();
					fstTime = FALSE;
				}

				if (LoopInrb (&suppQty, &suppWgt, &suppTot, &suppReb))
					PrntTots (&suppQty, &suppWgt, &suppTot, &suppReb);

				cc = find_rec (sumr, &sumr_rec, NEXT, "r");
		  }

		  if (!fstTime)
		  {
			fprintf (fout, ".EOF\n");
			pclose (fout);
		  }
}

int
IntFindSumr (
 char *rec, 
 int Errors)
{
		sprintf (sumr_rec.crd_no, "%6.6s", rec);
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		pad_num (sumr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc && Errors) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
		}
		return (cc); 
}

void
RunProg (
 char *name, 
 char *description)
{
	/*---------------------
	| if reprint should drop
	| through to Process ()
	-----------------------*/

	sprintf (local_rec.lpstr, "%d", local_rec.printerNumber);
	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onite [0] == 'Y') 
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				name,
				local_rec.lpstr,
				local_rec.ssupp,
				local_rec.esupp,
				description, (char *)0);
        prog_exit = 1;
		return;
	}
	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
			execlp (name,
				name,
				local_rec.lpstr,
				local_rec.ssupp,
				local_rec.esupp,
				 (char *)0);
        prog_exit = 1;
		return;
	}

	Process ();
}

void
HeadOut (
 void)
{
	FirstTime = TRUE;

	if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L135\n");

	fprintf (fout,".ECompany %s - %s\n",comm_rec.co_no,clip (comm_rec.co_name));

	sprintf (err_str, 
			 ".ESupplier Rebate Claim Report %s", 
			 (REPRINT) ? "- Reprint" : " ");
	fprintf (fout, "%s\n", clip (err_str));

	if (!REPRINT)
		fprintf (fout, ".E AS AT %s", SystemTime ());
	else
		fprintf (fout, ".E For Date %s\n", DateToString (local_rec.date));

	if (!REPRINT)
		fprintf (fout, 
			 ".E From Suppliers '%s' to '%s'\n", 
			 local_rec.ssupp,
			 local_rec.esupp);
	else
		fprintf (fout, ".E For Supplier '%s'\n", local_rec.ssupp);

	fprintf (fout, ".B1\n");


	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=============");
	fprintf (fout, "==================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===============\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=============");
	fprintf (fout, "==================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "===============\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|             DESCRIPTION                  ");
	fprintf (fout, "| DATE RECPT.");
	fprintf (fout, "|      GRN NO     ");
	fprintf (fout, "|  QUANTITY  ");
	fprintf (fout, "|   WEIGHT   ");
	fprintf (fout, "| NET FGN COST|\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|-------------|\n");
}

void
OutPutInrb (
 void)
{
	char	sdate [11];
	char	fdate [11];

	strcpy (fdate, DateToString (fromDate));
	strcpy (sdate, DateToString (inrb_rec.start_date));

	fprintf (fout, ".LRP7\n");
	fprintf (fout, "| Rebate Code : %6.6s (%40.40s)", 
								 inrb_rec.reb_code, inrb_rec.description);
	fprintf (fout, "          ");
	fprintf (fout, "                  ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "              |\n");

	fprintf (fout, "| Rebates life is from %10.10s to %10.10s (%02d Monthly Cycle) %66.66s|\n",
			 sdate,
			 DateToString (inrb_rec.end_date),
			 inrb_rec.cycle,
			 " ");

	fprintf (fout, "| This cycle is From %10.10s to %10.10s %87.87s|\n",
			 fdate,
			 DateToString (toDate - 1),
			 " ");

	fprintf (fout, "|%132.132s|\n", " ");
	fprintf (fout, "|------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "--------------|\n");
}

void
DefineSection (
 void)
{
	char	sdate [11];
	char	fdate [11];

	strcpy (sdate, DateToString (inrb_rec.start_date));
	strcpy (fdate, DateToString (fromDate));

	fprintf (fout, ".DS6\n");
	fprintf (fout, "| Supplier : %6.6s (%40.40s)", sumr_rec.crd_no,sumr_rec.crd_name);
	fprintf (fout, "             ");
	fprintf (fout, "                  ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "              |\n");

	fprintf (fout, "| Rebate Code : %6.6s (%40.40s)", 
								 inrb_rec.reb_code, inrb_rec.description);
	fprintf (fout, "          ");
	fprintf (fout, "                  ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "              |\n");

	fprintf (fout, "| Rebates life is from %10.10s to %10.10s (%02d Monthly Cycle) %66.66s|\n",
			 sdate,
			 DateToString (inrb_rec.end_date),
			 inrb_rec.cycle,
			 " ");

	fprintf (fout, "| This cycle is From %10.10s to %10.10s %87.87s|\n",
			 fdate,
			 DateToString (toDate - 1),
			 " ");

	fprintf (fout, "|%132.132s|\n", " ");
	fprintf (fout, "|------------------");
	fprintf (fout, "-------------------------------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "------------------");
	fprintf (fout, "-------------");
	fprintf (fout, "-------------");
	fprintf (fout, "--------------|\n");
}

int
LoopInrb (
 float *qty, 
 float *wgt, 
 double *tot, 
 double *reb)
{
	/*------------------
	| loop thru all rebates
	-----------------------*/
	int		loop = 0;
	float	lcl_qty = *qty;
	float	lcl_wgt = *wgt;
	double	lcl_tot = *tot;
	double	lcl_reb = *reb;

	strcpy (inrb_rec.reb_flag, "S");
	inrb_rec.link_hash = sumr_rec.hhsu_hash;
	strcpy (inrb_rec.reb_code, "     ");
	cc = find_rec (inrb, &inrb_rec, GTEQ, REPRINT ? "r" : "u");
	while (!cc &&
			inrb_rec.reb_flag [0] == 'S' &&
			inrb_rec.link_hash == sumr_rec.hhsu_hash
		)
		 {
			/*------------------
			| check to see if rebate
			| active yet/still
			-------------------------*/
			if (NotActive ())
			{
				abc_unlock (inrb);
				cc = find_rec (inrb, &inrb_rec, NEXT, REPRINT ? "r" : "u");
				continue;
			}

			DefineSection ();
			if (!loop && !FirstTime)
			{
				fprintf (fout, ".PA\n");
			}
			else
			{
				if (!FirstTime)
					OutPutInrb ();
			}

			FirstTime = FALSE;
			loop++;
			fsort = sort_open ("supre");

			/*----------------
			| for each inrb rec
			| that we find, find
			| all sura records
			----------------------*/
			sura_rec.hhsu_hash = sumr_rec.hhsu_hash;
			sprintf (sura_rec.rebate, "%5.5s", inrb_rec.reb_code);
			sprintf (sura_rec.buygrp, "%6.6s", " ");
			cc = find_rec (sura, &sura_rec, GTEQ, "r");
			while (!cc && 
					sura_rec.hhsu_hash == sumr_rec.hhsu_hash && 
					!strcmp (sura_rec.rebate, inrb_rec.reb_code)
				 )
			{
				LoadSuph ();
				cc = find_rec (sura, &sura_rec, NEXT, "r");
			}

			fsort = sort_sort (fsort, "supre");
			PrintLines (&lcl_qty, &lcl_wgt, &lcl_tot, &lcl_reb);

			*qty    = lcl_qty;
			*wgt    = lcl_wgt;
			*tot    = lcl_tot;
			*reb 	= lcl_reb;

			sort_delete (fsort, "supre");

			if (!REPRINT)
			{
				inrb_rec.la_cycle_rep++;
				cc = abc_update (inrb, &inrb_rec);
				if (cc)
					file_err (cc, "inrb", "DBUPDATE");
			}
			cc = find_rec (inrb, &inrb_rec, NEXT, REPRINT ? "r" : "u");
		 }
		 abc_unlock (inrb);
		 return (loop);
}

void
PrntTots (
 float *qty, 
 float *wgt, 
 double *tot, 
 double *reb)
{
	fprintf (fout, "| Supplier Totals  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|            ");
	fprintf (fout, "|                 ");
	fprintf (fout, "|%11.2f ",		*qty);
	fprintf (fout, "|%11.2f ",		*wgt);
	fprintf (fout, "|%12.2f |\n",	*tot);

	fprintf (fout, "| Total Rebate for ");
	fprintf (fout, "Supplier                                   ");
	fprintf (fout, "             ");
	fprintf (fout, "                  ");
	fprintf (fout, "             ");
	fprintf (fout, "             ");
	fprintf (fout, "|%12.2f |\n",	*reb);
	fprintf (fout, "|%132.132s|\n", " ");
}

void
LoadSuph (
 void)
{
	/*-----------------------
	| find all inmr belonging
	| to sura buying group
	-----------------------*/
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.buygrp, sura_rec.buygrp);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc &&
			!strcmp (inmr_rec.co_no, comm_rec.co_no) &&
			!strcmp (inmr_rec.buygrp, sura_rec.buygrp)
		 )
	  {
			/*-----------------
			| load suph records
			| into sort file
			------------------*/
			suph_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (suph, &suph_rec, GTEQ, "r");
			while (!cc && suph_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				if (suph_rec.hhsu_hash != sumr_rec.hhsu_hash)
				{
					cc = find_rec (suph, &suph_rec, NEXT, "r");
					continue;
				}
				if (suph_rec.rec_date > fromDate &&
					suph_rec.rec_date < toDate)
				{
						PutIntoSort ();
				}

				cc = find_rec (suph, &suph_rec, NEXT, "r");
			}

			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	  }
	  return;
}

void
PutIntoSort (
 void)
{
	/*--------------------
	| see doco for offsets
	---------------------*/
	fprintf (fsort, 
			 "%-16.16s%-40.40s %10.10s %10.2f %-15.15s %11.2f %10.2f\n",
			 inmr_rec.item_no,	
			 inmr_rec.description,					/* 16 */
			 DateToString (suph_rec.rec_date),		/* 57 */
			 suph_rec.rec_qty,						/* 68 */
			 suph_rec.grn_no,						/* 79 */
			 suph_rec.net_cost,						/* 95 */
			 inmr_rec.weight * suph_rec.rec_qty		/* 107 */
			);

}

void
PrintLines (
	float 	*supQty, 
	float 	*supWgt, 
	double 	*supTot, 
	double 	*TotReb)
{
	char	*sptr;
	float	suphQty = 0.00;
	float	suphWgt = 0.00;
	double	suphTot = 0.00;
	double	rebTot = 0.00;
	int		count = 0;

	sptr = sort_read (fsort);

	while (sptr)
	{
		suphQty += atof (sptr + 68);
		suphWgt += atof (sptr + 107);
		suphTot += atof (sptr + 95);
		count++;

		fprintf (fout, 
				"| %16.16s | %40.40s| %10.10s | %16.16s | %10.10s | %10.10s | %11.11s |\n",
				sptr,
				sptr + 16,
				sptr + 57,
				sptr + 79,
				sptr + 68,
				sptr + 107,
				sptr + 95
			  );
		sptr = sort_read (fsort);
	}

	if (count)
	{
		fprintf (fout, "|------------------");
		fprintf (fout, "-------------------------------------------");
		fprintf (fout, "-------------");
		fprintf (fout, "------------------");
		fprintf (fout, "-------------");
		fprintf (fout, "-------------");
		fprintf (fout, "--------------|\n");
	}
	fprintf (fout, "| Totals           ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|            ");
	fprintf (fout, "|                 ");
	fprintf (fout, "|%11.2f ", 	  suphQty);
	fprintf (fout, "|%11.2f ", 	  suphWgt);
	fprintf (fout, "|%12.2f |\n", suphTot);
	
	/*---------------
	| work out rebate
	----------------*/
	fprintf (fout, "| Rebate Total     ");
	if (suphTot == (double) 0.00)
	{
		rebTot = 0.00;
		fprintf (fout, "  - Not Available                          ");
		fprintf (fout, "             ");
		fprintf (fout, "                  ");
		fprintf (fout, "             ");
		fprintf (fout, "             ");
		fprintf (fout, "              |\n");
	}
	else
	{
		/*----------------------
		| work out break point |
		----------------------*/
		for (count = 5; count >= 0; count--)
		{
			/*-----------------------
			| on the basis of value |
			-----------------------*/
			if (inrb_rec.basis [0] == 'V' &&
				suphTot >= inrb_reb_qty [count])
			{
				if (inrb_reb_qty [count] == 0.00)
					continue;

				/*----------------------------------------
				| if by value then return rebTot = value |
				-----------------------------------------*/
				if (inrb_rec.reb_type [0] == 'V')
					rebTot = inrb_reb_val [count];

				if (inrb_rec.reb_type [0] == 'P')
					rebTot = suphTot * (inrb_reb_val [count] / 100.00);
				break;
			}

			/*-----------------------
			| on the basis of Units |
			-----------------------*/
			if (inrb_rec.basis [0] == 'U' &&
				suphQty >= inrb_reb_qty [count])
			{
				if (inrb_reb_qty [count] == 0.00)
					continue;

				/*-----------------------
				| if by Units can Not be
				| rebate by percentage
				------------------------*/
				if (inrb_rec.reb_type [0] == 'V')
					rebTot = inrb_reb_val [count];
				break;
			}

			/*------------------------
			| on the basis of Weight |
			------------------------*/
			if (inrb_rec.basis [0] == 'W' &&
				suphWgt >= inrb_reb_qty [count])
			{
				if (inrb_reb_qty [count] == 0.00)
					continue;

				/*----------------------------------------------
				| if by Weight can Not be rebate by percentage |
				----------------------------------------------*/
				if (inrb_rec.reb_type [0] == 'V')
					rebTot = inrb_reb_val [count];
				break;
			}
		}
		switch (inrb_rec.basis [0])
		{
		case 'U' :
			sprintf (err_str, "- Based On %-8.8s ", "Quantity");
			break;
		case 'V' :
			sprintf (err_str, "- Based On %-8.8s ", "Value");
			break;
		case 'W' :
			sprintf (err_str, "- Based On %-8.8s ", "Weight");
			break;
		}
		fprintf (fout, "| %20.20s                     ", err_str);
		fprintf (fout, "|            ");
		fprintf (fout, "|                 ");
		fprintf (fout, "|            ");
		fprintf (fout, "|            ");
		fprintf (fout, "|%12.2f |\n", rebTot);
	}
	fprintf (fout, "|==================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=============");
	fprintf (fout, "==================");
	fprintf (fout, "=============");
	fprintf (fout, "=============");
	fprintf (fout, "==============|\n");

	*supQty += suphQty;
	*supWgt += suphWgt;
	*supTot += suphTot;
	*TotReb += rebTot;
}

int
NotActive (
 void)
{
	static long	today = -1L;
	static int mdy [3];
	int	newmdy [3];
	long	vardate;

	if (today == -1L)
	{
		today = TodaysDate ();
	}

	if (!REPRINT)
	{
		DateToDMY (inrb_rec.start_date, &mdy [0], &mdy [1], &mdy [2]);
	}
	else
	{
		vardate = inrb_rec.start_date;
		if (vardate >= local_rec.date)
			return (TRUE);

		/*----------------------------
		| obatin start and end dates |
		----------------------------*/
		while (vardate <= local_rec.date)
		{
			/*-------------------------------------------------
			| Read Until Greater Than Then go back one cycle  |
			| For end date and 2 cycles For start date        |
			-------------------------------------------------*/
			DateToDMY (vardate, &mdy [0], &mdy [1], &mdy [2]);
			mdy [1] += inrb_rec.cycle;
			if (mdy [1] > 12)
			{
				mdy [1] = mdy [1] % 12;
				mdy [2]++;   /* increment the year */
			}
			vardate = DMYToDate (mdy [0], mdy [1], mdy [2]);
		}

		/*----------------------------------------------------
		| we have now found the cycle which is greater       |
		| than the date required so now subtract one cycle   |
		| for toDate and a further one for the fromDate      |
		----------------------------------------------------*/
		DateToDMY (vardate, &mdy [0], &mdy [1], &mdy [2]);
		mdy [1] -= inrb_rec.cycle;
		if (mdy [1] < 1)
		{
			mdy [1] = mdy [1] + 12;
			mdy [2]--;   /* decrement the year */
		}
		toDate = DMYToDate (mdy [0], mdy [1], mdy [2]);

		DateToDMY (toDate, &mdy [0], &mdy [1], &mdy [2]);
		mdy [1] -= inrb_rec.cycle;
		if (mdy [1] < 1)
		{
			mdy [1] = mdy [1] + 12;
			mdy [2]--;   /* decrement the year */
		}
		fromDate = DMYToDate (mdy [0], mdy [1], mdy [2]);
		if (fromDate < inrb_rec.start_date ||
			fromDate > inrb_rec.end_date)
			return (TRUE);

		return (FALSE);
	}

	/*-------------------------------------------------------------
	| *****   SHOULD NEVER GET THIS FAR IF IT IS REPRINT    *******
	-------------------------------------------------------------*/

	newmdy [0] = mdy [0];
	newmdy [1] = mdy [1];
	newmdy [2] = mdy [2];

	/*----------------------------------------------------------------
	| Add number of cycles from inrb to get fromDate if not reprint. |
	----------------------------------------------------------------*/
	if (!REPRINT)
		newmdy [1] += inrb_rec.cycle * inrb_rec.la_cycle_rep;

	if (newmdy [1] > 12)
	{
		newmdy [1] = newmdy [1] % 12;
		newmdy [2]++;   /* increment the year */
	}

	/*---------------------------
	| convert back to long date |
	---------------------------*/
	if (!REPRINT)
		fromDate = DMYToDate (newmdy [0], newmdy [1], newmdy [2]);

	/*---------------------------------------------------------------
	| compare date to start processing from with start and end date |
	---------------------------------------------------------------*/
	if (today < fromDate || fromDate > inrb_rec.end_date)
			return (TRUE);
	
	/*-----------------------------
	| Add one cycle to get toDate |
	-----------------------------*/
	newmdy [1] += inrb_rec.cycle;

	if (newmdy [1] > 12)
	{
		newmdy [1] = newmdy [1] % 12;
		newmdy [2]++;   /* increment the year */
	}

	/*---------------------------
	| convert back to long date |
	---------------------------*/
	toDate = DMYToDate (newmdy [0], newmdy [1], newmdy [2]);

	/*--------------------------------------
	| today has to be greater than toDate. |
	--------------------------------------*/
	if (today <= toDate)
		return (TRUE);

	return (FALSE);
}

