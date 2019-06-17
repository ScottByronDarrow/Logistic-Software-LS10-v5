/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_remtprn.c,v 5.6 2002/06/25 03:17:05 scott Exp $
|  Program Name  : (cr_remtprn.c) 
|  Program Desc  : (Print Remittance Advices.)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written : 10/05/89          |
|---------------------------------------------------------------------|
| $Log: cr_remtprn.c,v $
| Revision 5.6  2002/06/25 03:17:05  scott
| Updated to ensure cheque number length is 15 characters as per schema.
|
| Revision 5.5  2001/12/10 00:43:18  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.4  2001/12/07 07:03:30  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_remtprn/cr_remtprn.c,v 5.6 2002/06/25 03:17:05 scott Exp $";
char	*PNAME = "$RCSfile: cr_remtprn.c,v $";

#include	<pslscr.h>
#include 	<pr_format3.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>
#include 	<arralloc.h>

#define LINES 24	/* Number of lines in page body */

#define	SINGLE	0
#define	MULTI	1

#define	LINE_UP				 (StringToDate (chequeDate) < 0L)
#define	LINE_UP_CHAR(x)		 (LINE_UP ? line_up : x)

#define	BY_SUP		 (sortedBy [0] == 'S')
#define	BY_CHQ		 (sortedBy [0] == 'C')
#define	REPRINT		 (reprint [0] == 'Y')

#include    "schema"

struct commRecord   comm_rec;
struct comrRecord   comr_rec;
struct pocrRecord   pocr_rec;
struct strmRecord   strm_rec;
struct sumrRecord   sumr_rec;
struct suinRecord   suin_rec;
struct suhdRecord   suhd_rec;
struct sudtRecord   sudt_rec;

	char	*line_up = "LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_LINEUP_";

	int		chequeCount			= 0,
			printerNumber		= 1,
			noMessage 			= TRUE,	
			pageNumber 			= 0,
			lineNumber			= 0,
			firstTime 			= TRUE,
			chequeFound			= TRUE,
			printed 			= FALSE,
			envCrCo 			= 0,
			envCrFind 			= 0,
			lineupCopy 			= 0,
			reprintForLineup 	= FALSE;

	char	branchNumber [3],
			sortedBy [2],
			reprint [2],
			chequeNumber [2][16],
			chequeDate [11],
			systemDate [11];

	long	ageDate [5]	 =	{0,0,0,0,0};/*	current & last 3 month dates */
	 
	FILE	*fin;
	FILE	*fout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [16];
	char	chequeNo [16];
	long	hhspHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);
/*
 * Local function prototypes 
 */
void	ReadSumr		 (void);
void	PromptForLineup	 (long);
int		ValidateCheque	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
void	OpenOutput		 (void);
void	ReadComm		 (void);
void	PrintTotal		 (double);
int		heading			 (void);
void	EndPageRoutine	 (void);
int		check_page		 (void);
int		ProcessRoutine	 (void);
void	PrintCheque		 (int);
void	PrintLine		 (long, int, char *, double);
void	PrintRemLine	 (char *, char *, char *, double);
void	PrintFill		 (void);
int		LoadPaper		 (void);
int		CheckIfLinedUp	 (void);
int		ReadCheques		 (void);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char 	*argv [])
{
	char 	*sptr;

	if (argc != 7)
	{
		print_at (0, 0, mlCrMess702, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);
	sprintf (sortedBy, "%-1.1s", argv [2]);
	sprintf (reprint, "%-1.1s", argv [3]);
	sprintf (chequeNumber [0], "%-15.15s", argv [4]);
	sprintf (chequeNumber [1], "%-15.15s", argv [5]);
	sprintf (chequeDate, "%-10.10s", argv [6]);

	sptr = chk_env ("CR_CO");
	if (sptr == (char *)0)
		envCrCo = 0;
	else
		envCrCo = atoi (sptr);

	sptr = chk_env ("CR_FIND");
	if (sptr == (char *)0)
		envCrFind = 0;
	else
		envCrFind = atoi (sptr);

	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	set_tty ();

	init_scr ();

	strcpy (systemDate, DateToString (TodaysDate ()));

	OpenDB ();
	ReadComm ();

	dsp_screen (" Processing : Printing Remittances ", 
				comm_rec.co_no, comm_rec.co_name);

	/*
	 * Open format file	
	 */
	if ((fin = pr_open ("cr_remtprn.p")) == NULL)
		sys_err ("Error in cr_remtprn.p During (FOPEN)", errno, PNAME);
	LoadPaper ();
	OpenOutput ();

	if (BY_CHQ)
		ReadCheques ();
	else
		ReadSumr ();

	if (LINE_UP)
	{
		PrintFill ();
		PrintTotal (0.00);
	}

	fprintf (fout, ".EOF\n");
	fflush (stdout);
	pclose (fout);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
ReadSumr (void)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, "      ");
	strcpy (sumr_rec.acronym, "         ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
			      !strcmp (sumr_rec.est_no, branchNumber))
	{
		
		/*
		 * Get Cheques in  date order.
		 */
		suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suhd_rec.cheq_no, "             ");

		cc = find_rec (suhd, &suhd_rec, GTEQ, "u");
		while (!cc && suhd_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			if (!lineupCopy && !ValidateCheque ())
			{
				abc_unlock (suhd);
				cc = find_rec (suhd, &suhd_rec, NEXT, "u");
				continue;
			}
		
			if (ProcessRoutine ())
			{
				abc_unlock (suhd);
				cc = find_rec (suhd, &suhd_rec, NEXT, "u");
				continue;
			}

			PromptForLineup (suhd_rec.hhsp_hash);

			cc = find_rec (suhd, &suhd_rec, NEXT, "u");
		}
		abc_unlock (suhd);
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
}

void
PromptForLineup (
	long	hhspHash)
{
	while (firstTime == TRUE && chequeFound == TRUE)
	{
		fprintf (fout, ".EOF\n");
		fflush (stdout);
		pclose (fout);
		cc = CheckIfLinedUp ();
		if (cc)
		{
			abc_unlock (suhd);
			suhd_rec.hhsp_hash = hhspHash;

			if (BY_SUP)
				cc = find_rec ("suhd2", &suhd_rec, COMPARISON, "r");
			else
				cc = find_rec (suhd, &suhd_rec, COMPARISON, "r");

			OpenOutput ();

			reprintForLineup = TRUE;
			ProcessRoutine ();
			reprintForLineup = FALSE;
		}
		else
		{
			firstTime = FALSE;
			OpenOutput ();
		}
	}
}

int
ValidateCheque (void)
{
	/*
	 * Transaction is not on valid date.       
	 */
	if (BY_SUP && REPRINT && suhd_rec.date_post != StringToDate (chequeDate))
		return (EXIT_SUCCESS);

	/*
	 * Not flagged as cheque or Remittance. 
	 */
	if (!REPRINT && suhd_rec.rem_prt [0] != 'R')
		return (EXIT_SUCCESS);

	/*
	 * Transaction is not in valid cheq range. 
	 */
	if (BY_CHQ && REPRINT && (strcmp (suhd_rec.cheq_no, chequeNumber [0]) < 0 ||
	    strcmp (suhd_rec.cheq_no, chequeNumber [1]) > 0))
		return (EXIT_SUCCESS);

	/*
	 * Transaction is a journal type.      
	 */
	if (suhd_rec.pay_type [0] == 'J')
		return (EXIT_SUCCESS);

	if (suhd_rec.tot_amt_paid == 0.00)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	if (BY_SUP)
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, 
				  (envCrFind == 0) ? "sumr_id_no" : "sumr_id_no3");
	else
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS,"sumr_hhsu_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	abc_alias ("suhd2", suhd);
	open_rec ("suhd2", suhd_list, SUHD_NO_FIELDS, "suhd_hhsp_hash");
}

void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_fclose (sudt);
	abc_fclose (pocr);
	abc_fclose (comr);
	abc_fclose ("suhd2");
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenOutput (void)
{
	/*
	 * Open pipe to standard print . 
	 */
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*
 	 * Start output to standard print. 
	 */
	fprintf (fout, ".START00/00/00\n");
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".OP\n");
	fprintf (fout, ".PL0\n");
	fprintf (fout, ".2\n");
	fprintf (fout, ".L90\n");
	fprintf (fout, ".PI12\n");
	fflush (fout);
}

/*
 * Get common info from commom database file. 
 */
void
ReadComm (void)
{
	int	i;
	int	j;
	int	cm;			/* Current debtors month */
	int	cd;			/* Current debtors day	*/
	static	int	days [14] = {0,31,28,31,30,31,30,31,31,30,31,30,31,31};
	int	dmy [3];

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*
	 * Find dates for ageing i.e. end of last three months. 
	 */
	ageDate [0] = comm_rec.crd_date;
	ageDate [4] = 0L;

	DateToDMY (comm_rec.crd_date, &dmy [0], &dmy [1], &dmy [2]);

	/*
	 * Adjust feb for leap year. 
	 */
	days [2] = (dmy [2] % 4) == 0 ? 29 : 28;

	/*
	 * Get current month from module date. 
	 */
	cm = dmy [1];
	cd = dmy [0];

	/*
	 * If day not end of month, adjust day. 
	 */
	if (cd  < days [cm])
		ageDate [0] += (days [cm] - cd); 

	for (i = 1;i< 4;i++) 
	{
		j = (cm - i);
		if (j < 1)
			j += 12;
		ageDate [i] = ageDate [i - 1] - days [j + 1];
	}

	/*
	 * See if statement message present 
	 */
	if (noMessage == FALSE) 
	{
		open_rec (strm, strm_list, STRM_NO_FIELDS, "strm_id_no");
		strcpy (strm_rec.co_no, comm_rec.co_no);
		strm_rec.mesg_type [0] = 'S';
	
		cc = find_rec (strm, &strm_rec, COMPARISON, "r");
		if (cc)
			noMessage = TRUE;
		else
			noMessage = FALSE;
	
		abc_fclose (strm);
	}
}

/*
 * Routine to print bottom of statement. 
 */
void
PrintTotal (
 double cheq_amt)
{
	pr_format (fin, fout, "VBLE_BLANK", 1, LINES - lineNumber);
	pr_format (fin, fout, "MARGIN4", 0, 0);
	pr_format (fin, fout, "CHEQ_LINE", 1, LINE_UP_CHAR (suhd_rec.cheq_no));
	pr_format (fin, fout, "CHEQ_LINE", 2, (LINE_UP) ? 0.00 : cheq_amt);
	pr_format (fin, fout, "NEXT_PAGE", 0, 0);
	pr_format (fin, fout, "HMARGIN", 0, 0);
}

/*
 * Routine to print new headings for each statment page. 
 */
int
heading (void)
{
	lineNumber = 0;
	pageNumber++;

	pr_format (fin, fout, "COMR_NAME", 1, LINE_UP_CHAR (comr_rec.co_name));
	pr_format (fin, fout, "COMR_ADD", 1, LINE_UP_CHAR (comr_rec.co_adr1));
	pr_format (fin, fout, "COMR_ADD", 1, LINE_UP_CHAR (comr_rec.co_adr2));
	pr_format (fin, fout, "COMR_ADD", 1, LINE_UP_CHAR (comr_rec.co_adr3));

	pr_format (fin, fout, "MARGIN1", 0, 0);

	pr_format (fin, fout, "SUPR_NAME", 1, LINE_UP_CHAR (sumr_rec.crd_name));
	pr_format (fin, fout, "SUPR_NAME", 2, LINE_UP ? 0 : pageNumber);
	pr_format (fin, fout, "SUPR_ADD1", 1, LINE_UP_CHAR (sumr_rec.adr1));
	pr_format (fin, fout, "SUPR_ADD2", 1, LINE_UP_CHAR (sumr_rec.adr2));
	pr_format (fin, fout, "SUPR_ADD2", 2, LINE_UP_CHAR (sumr_rec.crd_no));
	pr_format (fin, fout, "SUPR_ADD3", 1, LINE_UP_CHAR (sumr_rec.adr3));
	pr_format (fin, fout, "REMT_DATE", 1, LINE_UP_CHAR (sumr_rec.adr4));
	pr_format (fin, fout, "REMT_DATE", 2, LINE_UP_CHAR (systemDate));

	pr_format (fin, fout, "MARGIN2", 0, 0);

	pr_format (fin, fout, "CURR_CODE", 1, LINE_UP_CHAR (pocr_rec.description));

	pr_format (fin, fout, "HEAD_LINE", 0, 0);

	pr_format (fin, fout, "MARGIN3", 0, 0);
	return (EXIT_SUCCESS);
}

/*
 * Routine to end continuing page (more pages to follow). 
 */
void
EndPageRoutine (void)
{
	pr_format (fin, fout, "MARGIN4", 0, 0);		/* balance due line	*/
	pr_format (fin, fout, "CONTINUED", 0, 0);	/* "Continued"		*/
	pr_format (fin, fout, "NEXT_PAGE", 0, 0);	/* bottom of form	*/
	pr_format (fin, fout, "HMARGIN", 0, 0);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

int
ProcessRoutine (void)
{
	int	first_go = 0;

	/*
	 * Read supplier master record.   
	 */
	if (BY_CHQ)
	{
		sumr_rec.hhsu_hash = suhd_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
			return (cc);
	}

	strcpy (comr_rec.co_no, sumr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, sumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", " ");

	pageNumber = 0;
	lineNumber = 0;

	first_go = 1;
		
	if (BY_SUP)
		dsp_process ("Cheque No: ", suhd_rec.cheq_no);
	else
		dsp_process ("Supplier No: ", sumr_rec.crd_no);

	PrintCheque (first_go);

	/*
	 * Want to keep the program running so don't sys_err on error.
	 */
	if (!reprintForLineup)
	{
		strcpy (suhd_rec.rem_prt, "0");
		cc = abc_update (suhd, &suhd_rec);
		if (cc)
			sys_err ("Error in suhd During (DBUPDATE)", cc, PNAME);
	}
	abc_unlock (suhd);

	lineupCopy = 0;
	return (EXIT_SUCCESS);
}

void
PrintCheque (
 int first_go)
{
	double	balance  = 0.00;
	double	wk_bal   = 0.00;

	chequeCount = 0;

	sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
	cc = find_rec (sudt, &sudt_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
	{
		chequeCount++;
		suin_rec.hhsi_hash = sudt_rec.hhsi_hash;
		cc = find_rec (suin, &suin_rec, COMPARISON, "r");
		if (!cc)
		{
			balance = sudt_rec.amt_paid_inv;
			wk_bal = DOLLARS (balance);
			if (wk_bal != 0) 
			{
				if (first_go)
				{
					heading ();
					printed = TRUE;
					first_go = FALSE;
				}
				PrintLine (suin_rec.date_of_inv, atoi (suin_rec.type) + 1,
						  suin_rec.inv_no, wk_bal);
			}
		}
		cc = find_rec (sudt, &sudt_rec, NEXT, "r");
	}
	if (chequeCount != 0)
	{
		balance = suhd_rec.tot_amt_paid - suhd_rec.disc_taken;
		PrintTotal (balance);
	}
}

void
PrintLine (
	long	date1,
	int		type,
	char 	*ref1,
	double	amt)
{
	char	strdate [9];

	if (++lineNumber > LINES) 
	{
		EndPageRoutine ();
		heading ();
		lineNumber = 1;
	}

	strcpy (strdate, DateToString (date1));

	switch (type)
	{
	/*
	 * Cheque	
	 */
	case 1:
		PrintRemLine (strdate, "CHEQUE", ref1, amt * -1.00);
		break;

	/*
	 * Invoice	
	 */
	case 2:
		PrintRemLine (strdate, "INVOICE", ref1, amt);
		break;

	/*
	 * Credit - held as -ve	
	 */
	case 3:
		PrintRemLine (strdate, "CREDIT", ref1, amt);
		break;

	case 4:
		PrintRemLine (strdate, "JOURNAL", ref1, amt);
		break;

	case 5:
		strcpy (strdate, "        ");
		PrintRemLine (strdate, ref1, "B/F FWD", amt);
		break;

	default:
		strcpy (strdate, "*ERROR*");
		PrintRemLine (strdate, "* ERROR *", "* ERROR *", 0.00);
		break;
	}
}

void
PrintRemLine (
	char	*strdate,
	char	*lineType,
	char	*ref1,
	double	amt)
{
	char	dbtcrdt [11];

	sprintf (dbtcrdt, "%10.2f", amt);
	pr_format (fin, fout, "REMT_LINE", 1, strdate);
	pr_format (fin, fout, "REMT_LINE", 2, lineType);
	pr_format (fin, fout, "REMT_LINE", 3, ref1);
	pr_format (fin, fout, "REMT_LINE", 4, dbtcrdt);
}

void
PrintFill (void)
{
	heading ();
	do
	{
		pr_format (fin, fout, "REMT_LINE", 1, LINE_UP_CHAR (" "));
		pr_format (fin, fout, "REMT_LINE", 2, LINE_UP_CHAR (" "));
		pr_format (fin, fout, "REMT_LINE", 3, LINE_UP_CHAR (" "));
		pr_format (fin, fout, "REMT_LINE", 4, LINE_UP_CHAR (" "));
	} while (++lineNumber < LINES);
}

/*
 * Routine to check if paper loaded.    
 */
int
LoadPaper (void)
{
	int	c;

	c = prmptmsg (ML (mlCrMess018), "YyNn", 22, 3);
	fflush (stdout);
	rv_pr (ML (mlCrMess020), 21, 3, 1);
	fflush (stdout);
	return (EXIT_SUCCESS);
}

/*
 * Routine to reprint cheque for lineup 
 */
int
CheckIfLinedUp (void)
{
	int	c;

	c = prmptmsg (ML (mlCrMess019), "YyNn", 22, 4);
	fflush (stdout);
	if (c == 'Y' || c == 'y')
		return (EXIT_FAILURE);
	
	rv_pr (ML (mlCrMess021), 20, 4, 1);
	fflush (stdout);
	return (EXIT_SUCCESS);
}

int
ReadCheques (void)
{
	int		saved = FALSE;
	int		i;
	/*
	 * Get invoices in date order. 
	 */
	suhd_rec.hhsu_hash = 0L;
	strcpy (suhd_rec.cheq_no, "               ");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails,&sortRec, sizeof (struct SortStruct),10);
	sortCnt = 0;

	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc)
	{
		sumr_rec.hhsu_hash = suhd_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (suhd, &suhd_rec, NEXT, "r");
			continue;
		}
		if (!strcmp (sumr_rec.co_no, comm_rec.co_no) && 
			 !strcmp (sumr_rec.est_no, branchNumber))
		{
			/*
			 * Cheque no in valid range.          
			 */
			if (!ValidateCheque ())
			{
				cc = find_rec (suhd, &suhd_rec, NEXT, "r");
		    	continue;
			}
			saved = TRUE;

			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
				sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element sortCnt.
			 */
			strcpy (sortRec [sortCnt].sortCode, suhd_rec.cheq_no);
			strcpy (sortRec [sortCnt].chequeNo, suhd_rec.cheq_no);
			sortRec [sortCnt].hhspHash	=	suhd_rec.hhsp_hash;
			/*
			 * Increment array counter.
			 */
			sortCnt++;
		}
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
	/*
	 * No cheques found.    	
	 */
	if (!saved)
	{
		/*
	 	 *	Free up the array memory
		 */
		ArrDelete (&sortDetails);
		return (EXIT_FAILURE);
	}

	abc_selfield (suhd, "suhd_hhsp_hash");

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		suhd_rec.hhsp_hash = sortRec [i].hhspHash;
		cc = find_rec (suhd, &suhd_rec, COMPARISON, "u");
		if (!cc)
		{
			if (ProcessRoutine ())
			{
				abc_unlock (suhd);
				continue;
			}

			PromptForLineup (sortRec [i].hhspHash);

			abc_unlock (suhd);
		}
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	return (EXIT_SUCCESS);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
