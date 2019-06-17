/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lettpro.c,v 5.3 2001/12/03 08:32:34 scott Exp $
|  Program Name  : (db_lettpro.c)
|  Program Desc  : (Overdue letter process/print)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow    | Date Written  : 17/03/87         |
|---------------------------------------------------------------------|
| $Log: db_lettpro.c,v $
| Revision 5.3  2001/12/03 08:32:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lettpro.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lettpro/db_lettpro.c,v 5.3 2001/12/03 08:32:34 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#define	rewind	Remote_rewind
#endif	/* GVISION */

	int		envDbTotalAge;
	int		envDbDaysAgeing;

	/*
	 * Set up Array to hold Months of Year used with mon in time struct.
	 */
	static char *mth [] = {
		"January", "February", "March", "April", 
		"May", "June", "July", "August", 
		"September", "October", "November", "December"
	};

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cuinRecord	cuin_rec;
struct cudtRecord	cudt_rec;
struct pocrRecord	pocrRec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

	char	*data = "data", 
			*cumr2 = "cumr2";

	FILE	*fout;
	FILE	*f_lett [5];

#define	CURR_PERIOD	0
#define	O_DUE_PERIOD	1
#define	ALL_PERIODS	2

#define	TB	0
#define	OB	1
#define	B1	2
#define	B2	3
#define	B3	4
#define	B4	5
#define	B5	6
#define	IC	7
#define	IO	8
#define	IA	9
#define	DA	10
#define	DN	11
#define	A1	12
#define	A2	13
#define	A3	14
#define	CN	15
#define	DC	16
#define	CD	17
#define	MD	18
#define	FD	19
#define	CC	20

#define	N_CMDS	21

char	*dot_cmds [] = {
	"TB", 
	"OB", 
	"B1", 
	"B2", 
	"B3", 
	"B4", 
	"B5", 
	"IC", 
	"IO", 
	"IA", 
	"DA", 
	"DN", 
	"A1", 
	"A2", 
	"A3", 
	"CN", 
	"DC", 
	"CD", 
	"MD", 
	"FD", 
	"CC", 
	""
};

#ifndef	LINUX
extern	int		errno;
#endif	/* LINUX */

	int		printerNo 		= 1, 
			letterNo 		= 0, 
			partPrinted		= 0, 
			envDbCo 		= 0, 
			firstTime 		= TRUE, 
			envDbNettUsed 	= TRUE, 
			envDbMcurr 		= FALSE;

	char	directoryName 	[51], 
			startCustomer 	[7], 
			endCustomer		[7];

	double	invBalance 		= 0.00, 
			overdueBal 		= 0.00;

	double	custBal [6];


/*
 * Local Function Prototypes.
 */
char 	*GetDate 			(long);
int 	OpenLetter 			(int);
int 	OpenOutput 			(void);
int 	ProcessCustomer 	(void);
int 	ValidCommand 		(char *);
void 	CloseDB 			(void);
void 	CloseOutput 		(void);
void 	CustInvBalance 		(void);
void 	OpenDB 				(void);
void 	Parse 				(int);
void 	PrintAmount 		(double);
void 	PrintInvoice 		(int, int);
void 	RawLetter 			(int);
void 	shutdown_prog 		(void);
void 	SubstituteCommand 	(int, int);

/*====================
| Main Program loop. |
====================*/
int
main (
 int                argc, 
 char*              argv [])
{
	char	*sptr;
#ifdef GVISION
	char	szDirName [100];
#endif	/* GVISION */

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = get_env ("DB_TOTAL_AGE");
	if (sptr == (char *)0)
		envDbTotalAge = FALSE;
	else
		envDbTotalAge = (*sptr == 'T' || *sptr == 't');

	/*
	 * Check if ageing is by days overdue or as per standard ageing.
	 */
	sptr = chk_env ("DB_DAYS_AGEING");
	envDbDaysAgeing = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envDbCo = (sptr == (char *)0) ? 0 : atoi (sptr);

	if (argc != 7)
	{
		/*
		 * Usage: %s <directoryName> <printerNo> 	 <letterNo>
		 *           <overdueBal> 	 <startCustomer> <endCustomer>
		 */
		print_at (0, 0, mlDbMess061, argv [0]);
        return (EXIT_FAILURE);
	}

	sprintf (directoryName, "%-.59s", argv [1]);
#ifdef GVISION
	/*
	 * NOTE: I added this because the  first "/" in parameter is always
	 * deleted in GVision. 
	 */
	strcpy (szDirName, "/");
	strcat (szDirName, directoryName);
	strcpy (directoryName, szDirName);
#endif	/* GVISION */
	sprintf (startCustomer, "%-.6s", argv [2]);
	sprintf (endCustomer, "%-.6s", argv [3]);

	printerNo     = (argc > 4) ? atoi (argv [4]) : 1;
	letterNo  = (argc > 5) ? atoi (argv [5]) : 0;
	overdueBal = (argc > 6) ? atof (argv [6]) : 0.00;

	overdueBal = CENTS (overdueBal);
	init_scr ();	

	OpenDB ();

	if (ProcessCustomer () == 1)
    {
        return (EXIT_FAILURE);
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

	abc_alias (cumr2, cumr);
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no"); 
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_ho_cron");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (pocr);
	abc_dbclose (data);
}

int
OpenOutput (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename, "%s/LETTERS/lett.output", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr);

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat ", errno, PNAME);

	fprintf (fout, ".START00/00/00\n");
	fprintf (fout, ".OP\n");
	fprintf (fout, ".LP%d\n", printerNo);
	fprintf (fout, ".0\n");

	if (OpenLetter (1))
    {
		shutdown_prog ();	
        return (EXIT_FAILURE);
    }

	if (OpenLetter (2))
    {
		shutdown_prog ();	
        return (EXIT_FAILURE);
    }

	if (OpenLetter (3))
    {
		shutdown_prog ();	
        return (EXIT_FAILURE);
    }

	if (OpenLetter (4))
    {
		shutdown_prog ();	
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

void
CloseOutput (void)
{
	int i;

	fprintf (fout, ".EOF\n");
	pclose (fout);

	for (i = 1; i < 5; i++)
		fclose (f_lett [i]);
}

/*
 * Process Customers.
 */
int
ProcessCustomer (void)
{
	double	o_due;
	int		i = 0;

	if (OpenOutput () == 1)
    {
        return (EXIT_FAILURE);
    }

	dsp_screen ("Now Processing Overdue Customers", 
								comm_rec.co_no, comm_rec.co_name);
	
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, comm_rec.est_no);
	strcpy (cumr_rec.dbt_no, startCustomer);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
				!strcmp (cumr_rec.est_no, comm_rec.est_no))
	{
		/*
		 * Exclude child customers for now.
		 */
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr , &cumr_rec, NEXT, "r");
			continue;
		}

		for (i = 0; i < 6; i++)
			custBal [i] = cumr_balance [i];

		cumr2_rec.ho_dbt_hash = cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				custBal [i] += cumr2_balance [i];
			
			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}
		o_due = custBal [1] + custBal [2] + custBal [3] + custBal [4];

		if (o_due > overdueBal)
		{
			if (firstTime)
				firstTime = FALSE;

			dsp_process ("Customer : ", cumr_rec.dbt_no); 

			if (custBal [4] != 0.00)
			{
				if (letterNo == 0 || letterNo == 4)
					Parse (4);
			}
			else
			{
				if (custBal [3] != 0.00)
				{
					if (letterNo == 0 || letterNo == 3)
						Parse (3);
				}
				else
				{
					if (custBal [2] != 0.00)
					{
						if (letterNo == 0 || letterNo == 2)
							Parse (2);
					}
					else
					{
						if (custBal [1] != 0.00)
						{
							if (letterNo == 0 || letterNo == 1)
								Parse (1);
						}
					}
				}
			}
		}
		if (!strcmp (cumr_rec.dbt_no, endCustomer))
 			break;
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	CloseOutput ();
    return (EXIT_SUCCESS);
}

/*
 * Open Letter File.
 */
int
OpenLetter (
	int		letterNo)
{
	char	letterName [61];

	sprintf (letterName, "%s/OD_LETT%d", clip (directoryName), letterNo);

	if ((f_lett [letterNo] = fopen (letterName, "r")) == NULL)
	{
		/*
		 * Letter File not found.
		 */
		errmess (ML (mlStdMess109));
		sleep (sleepTime);
		return (errno);
	}
	return (EXIT_SUCCESS);
}

void
RawLetter (
	int		letterNo)
{
	rewind (f_lett [letterNo]);
}

void
Parse (
	int		letterNo)
{
	int		cmd;
	char	*cptr;
	char	*dptr;
	char	*sptr;
	char	line [81];

	sptr = fgets (line, 80, f_lett [letterNo]);

	if (!firstTime)
		fprintf (fout, ".PA\n");

	while (sptr)
	{
		partPrinted = TRUE;

		/*-------------------------------
		|	remove \n from line	|
		-------------------------------*/
		* (sptr + strlen (sptr) - 1) = '\0';

		/*-------------------------------
		|	look for caret command	|
		-------------------------------*/
		cptr = strchr (sptr, '^');
		dptr = sptr;
		while (cptr)
		{
			partPrinted = FALSE;
			/*-------------------------------
			|	print line up to now	|
			-------------------------------*/
			*cptr = '\0';

			if (cptr != sptr)
			{
				partPrinted = TRUE;
				fprintf (fout, "%s", dptr);
			}

			/*-------------------------------
			|	check if valid ^cmd	|
			-------------------------------*/
			cmd = ValidCommand (cptr + 1);
			if (cmd >= TB)
			{
				SubstituteCommand (cmd, cptr - sptr);
				dptr = cptr + 3;
			}
			else
			{
				fprintf (fout, "^");
				partPrinted = TRUE;
				dptr = cptr + 1;
			}

			cptr = strchr (dptr, '^');
		}

		/*-------------------------------
		|	print rest of line	|
		-------------------------------*/

		if (partPrinted)
		{
			if (dptr)
				fprintf (fout, "%s\n", dptr);
			else
				fprintf (fout, "\n");
		}
		sptr = fgets (line, 80, f_lett [letterNo]);
	}

	RawLetter (letterNo);
}

int
ValidCommand (
 char*              wk_str)
{
	int		i;

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (wk_str, dot_cmds [i], 2))
			return (i);

	return (-1);
}

void
SubstituteCommand (
 int                cmd, 
 int                offset)
{
	char	*sptr;
	double	amt;

	switch (cmd)
	{
	/*---------------
	| Total Balance	|
	---------------*/
	case TB:
		amt = DOLLARS (custBal [0] + custBal [1] + custBal [2] + 
                       custBal [4] + custBal [5]);

		PrintAmount (amt);

		break;

	/*-------------------
	| Overdue Balance	|
	-------------------*/
	case OB:
		amt = DOLLARS (custBal [1] + custBal [2] + custBal [3] + custBal [4]);
		PrintAmount (amt);

		break;

	/*-----------------
	| Current Balance |
	-----------------*/
	case B1:
		amt = DOLLARS (custBal [0] + custBal [5]);
		PrintAmount (amt);

		break;

	/*-------------------------------
	|	1st period overdue Balance	|
	-------------------------------*/
	case B2:
	/*-------------------------------
	|	2nd period overdue Balance	|
	-------------------------------*/
	case B3:
	/*-------------------------------
	|	3rd period overdue Balance	|
	-------------------------------*/
	case B4:
	/*-------------------------------
	|	4rd period overdue Balance	|
	-------------------------------*/
	case B5:
		amt = DOLLARS (cumr_balance [cmd - B1]);
		PrintAmount (amt);

		break;

	/*-----------------------
	|	Current Invoices	|
	-----------------------*/
	case IC:
		PrintInvoice (CURR_PERIOD, offset);
		break;

	/*-----------------------------------------------
	|	Outstanding Invoices - Excluding Current	|
	-----------------------------------------------*/
	case IO:
		PrintInvoice (O_DUE_PERIOD, offset);
		break;

	/*-------------------
	|	All Invoices	|
	-------------------*/
	case IA:
		PrintInvoice (ALL_PERIODS, offset);
		break;

	/*-----------------------
	|	Customer Acronym	|
	-----------------------*/
	case DA:
		sptr = clip (cumr_rec.dbt_acronym);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-------------------
	|	Customer Number	|
	-------------------*/
	case DC:
		sptr = clip (cumr_rec.dbt_no);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-------------------
	|	Customer Name	|
	-------------------*/
	case DN:
		sptr = clip (cumr_rec.dbt_name);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-----------------------
	|	Delivery Address	|
	-----------------------*/
	case A1:
		sptr = clip (cumr_rec.ch_adr1);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	case A2:
		sptr = clip (cumr_rec.ch_adr2);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	case A3:
		sptr = clip (cumr_rec.ch_adr3);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-------------------
	|	Contact Name	|
	-------------------*/
	case CN:
		sptr = clip (cumr_rec.contact_name);
		if (*sptr)
		{
			fprintf (fout, "%s", sptr);
			partPrinted = TRUE;
		}
		else
			if (partPrinted)
				fprintf (fout, "Sir/Madam");
		break;

	/*-----------------------
	|	Current System Date |
	-----------------------*/
	case CD:

		strcpy (err_str, DateToString (TodaysDate ()));
		sptr = clip (err_str);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-----------------------
	|	Current Module Date |
	-----------------------*/
	case MD:
		sprintf (err_str, "%10.10s", DateToString (comm_rec.dbt_date));

		sptr = clip (err_str);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}
		break;

	/*-----------------------
	| Customer Module Date   |
	-----------------------*/
	case FD:
		sptr = GetDate (comm_rec.dbt_date);
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}

		break;

	/*-----------------------
	| Customer Currency Code |
	-----------------------*/
	case CC:
		if (!envDbMcurr)
			break;
		sptr = cumr_rec.curr_code;
		if (*sptr)
		{
			partPrinted = TRUE;
			fprintf (fout, "%s", sptr);
		}

		break;

	default:
		break;
	}
	fflush (fout);
}

void
PrintAmount (
	double	amt)
{
	int		neg;
	int		i;
	char	mask [21];
	char	*sptr;
	char	value [6][5];

	neg = (amt < 0.00);

	if (neg)
		amt *= -1;

	if (envDbMcurr)
	{
		strcpy (pocrRec.co_no, cumr_rec.co_no);
		strcpy (pocrRec.code, cumr_rec.curr_code);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (!cc)
			amt = amt / pocrRec.ex1_factor;
	}

	sprintf (mask, "%.2f", amt);

	sptr = (mask + strlen (mask) - 3);

	for (i = 0;sptr > mask;sptr -= 3, i++)
	{
		strcpy (value [i], sptr);
		*sptr = '\0';
	}

	if (i > 1 && strlen (mask) > 1)
		sptr = strcat (mask, ", ");

	for (i--;i > 0;i--)
	{
		sptr = strcat (mask, value [i]);
		if (i != 1)
			sptr = strcat (mask, ", ");
	}

	sptr = strcat (mask, value [0]);

	fprintf (fout, "%11.11s %s", mask, (neg) ? "CR" : "DB");

	partPrinted = TRUE;
}

void
PrintInvoice (
	int		period, 
	int		offset)
{
	int		periodIn;		/* period the invoice falls in	*/
	int		firstTime = TRUE;	/* first invoice printed	*/
	char	mask [15];
	double	amt;

	sprintf (mask, "\n%%%d.%ds", offset, offset);

	cuin_rec.ho_hash 	= cumr_rec.hhcu_hash;
	cuin_rec.date_of_inv 		= 0L;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.ho_hash == cumr_rec.hhcu_hash)
	{
		dsp_process ("Invoice : ", cuin_rec.inv_no);
		fflush (stdout);


		periodIn = 	AgePeriod 
					(
						cuin_rec.pay_terms, 
						cuin_rec.date_of_inv, 
						comm_rec.dbt_date, 
						cuin_rec.due_date, 
						envDbDaysAgeing, 
						envDbTotalAge
					);
		if (periodIn == -1)
			periodIn = 0;

		invBalance = 0.00;

		switch (period)
		{
		case CURR_PERIOD:
			if (periodIn == 0)
				CustInvBalance ();
			break;

		case O_DUE_PERIOD:
			if (periodIn > 0)
				CustInvBalance ();
			break;

		case ALL_PERIODS:
			CustInvBalance ();
			break;

		default:
			break;
		}

		amt = DOLLARS (invBalance);

		if (amt != 0.00)
		{
			if (firstTime)
				firstTime = FALSE;
			else
				fprintf (fout, mask, " ");
			fprintf (fout, "%s ", cuin_rec.inv_no);

			PrintAmount (amt);
		}

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	if (firstTime)
		fprintf (fout, "NONE ");
}

/*
 * Total invoice payments and determine balance.
 */
void
CustInvBalance (void)
{
	invBalance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;
	
	/*
	 *	find all cudt records for a cuin_in
	 */
	cudt_rec.hhci_hash = cuin_rec.hhci_hash;
	cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
	while (!cc && cudt_rec.hhci_hash == cuin_rec.hhci_hash)
	{
		invBalance -= cudt_rec.amt_paid_inv;
		cc = find_rec (cudt, &cudt_rec, NEXT, "r");
	}
}

/*
 * GetDate (long-date) returns date in 23 January 1986.
 */
char*
GetDate (
	long	currentDate)
{
	int		day, 
			mon, 
			year;

	DateToDMY (currentDate, &day, &mon, &year);

	sprintf (err_str, "%d %s %d", day, ML (mth [mon - 1]), year);
	return (err_str);
}
