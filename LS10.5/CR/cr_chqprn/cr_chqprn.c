/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (cr_chqprn.c   )                                   |
|  Program Desc  : (Print cheques.                              )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  sumr, suhd, comm,     ,     ,     ,     ,         |
|  Database      : (crdt)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Date Written  : (09/08/88)      | Author       : Roger Gibbison    |
|---------------------------------------------------------------------|
|  Date Modified : (09/08/88)      | Modified  by : Scott Darrow.     |
|  Date Modified : (09/01/89)      | Modified  by : Scott Darrow.     |
|  Date Modified : (27/04/89)      | Modified  by : Terry Keillor.    |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (06/04/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (06/04/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (20/05/97)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (16/09/1997)    | Modified  by : Jiggs A Veloz     |
|  Date Modified : (13/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : Changed to use cheque date and not system date.    |
|                : (27/04/89) Modified to use beg/end suhd hash nos   |
|                :            passed from calling programs cr_chqproc |
|                :            and cr_chqrprn.                         |
|  (14/08/92)    : S/C INF 7619. Fixes for port to HP.                |
|  (30/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (06/04/94)    : HGP 10469. Change parameters to dbltow ().         |
|  (06/04/96)    : PDL - Updated to change cheque length from 6-8.    |
|  (20/05/97)    : PDL - Updated to change cheque length from 8-13    |
|  (12/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (13/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqprn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqprn/cr_chqprn.c,v 5.2 2001/08/09 08:51:37 scott Exp $";

#define	LINES	32
#define LCL_MAX_LINES	1000

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_cr_mess.h>

#include	"schema"

#define		CHEQUE	 (printDocument[0] == 'C')
#define		REMITT	 (printDocument[0] == 'R')
#define		BOTH 	 (printDocument[0] == 'B')

	char	printDocument[2];
	char	primeUnit[16];
	char	subUnit[16];

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_cc_no"},
		{"comm_crd_date"},
		{"comm_stat_flag"}
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	tcc_no[3];
		long	tcrd_date;
		char	tstat_flag[2];
	} comm_rec;

	struct	crbkRecord	crbk_rec;
	struct	pocrRecord	pocr_rec;
	struct	sudtRecord	sudt_rec;
	struct	suhdRecord	suhd_rec;
	struct	suinRecord	suin_rec;
	struct	sumrRecord	sumr_rec;

	char	*data = "data",
			*comm = "comm";

	struct	{                       /*===================================*/
		char	invoiceDate[11];	/*| Cheque Date.                    |*/
		char	invoiceNumber[16];	/*| Invoice Number.       	    	|*/
		double	invoiceAmount;		/*| Invoice Amount.                 |*/
	} remit[LCL_MAX_LINES];        	/*===================================*/

	long	ageDays[5];

	char	*words;

	long	pidNumber = 0L;

	int		firstTime 	= TRUE;
	int		chequeFound = FALSE;
	int		reprint 	= FALSE;
	long	reprintHash;

	FILE	*fin, *fout;

#include	<pr_format3.h>

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		 (void);
void	CloseDB		 (void);
int		ProcessSuhd	 (void);
void	ReadComm	 (void);
void	PrintRemit   (void);
void	PrintLine    (int);
void	CancelCheque (void);
void	PrintCheque	 (void);
void	GetCurrUnits (void);
void	MainHeading  (int);
void	StarFill	 (char *);
int		check_page	 (void);
int		LoadPaper	 (void);
int		LineUpForm	 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		lpno;
	char *	sptr;

	if (argc < 2)
	{
		print_at (0, 0, mlCrMess002, argv[0]);
		return (EXIT_FAILURE);
	}

	dsp_screen (" Processing : Printing Remittances ",
					comm_rec.tco_no,comm_rec.tco_name);
	lpno = atoi (argv[1]);

	sptr = strrchr (argv[0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv[0];
	if (!strncmp (sptr, "cr_chq", 6))
		sprintf (printDocument, "%-1.1s", "C");

	if (!strncmp (sptr, "cr_rem", 6))
		sprintf (printDocument, "%-1.1s", "R");

	if (!strncmp (sptr, "cr_rc", 5))
		sprintf (printDocument, "%-1.1s", "B");

	if (argc == 3)
		pidNumber = atol (argv[2]);

	set_tty ();
	init_scr ();

	OpenDB ();
	ReadComm ();

	if (CHEQUE || BOTH)
		dsp_screen (" Processing : Printing Cheques ",comm_rec.tco_no,comm_rec.tco_name);
	else
		dsp_screen (" Processing : Printing Remittances ",comm_rec.tco_no,comm_rec.tco_name);

	LoadPaper ();

	MainHeading(lpno);

	/*-------------------------------------------
	| Process all cheques in range given.       |
	-------------------------------------------*/
	reprintHash = -1;
	cc = find_hash ("suhd",&suhd_rec,GTEQ,"u", (pidNumber) ? pidNumber : 0L);
	while (!cc)
	{
		if (pidNumber && suhd_rec.pid != pidNumber)
		{
			abc_unlock ("suhd");
			break;
		}

		if (ProcessSuhd ())
		{
			abc_unlock ("suhd");
			cc = find_hash ("suhd",&suhd_rec,NEXT,"u", (pidNumber) ? pidNumber : 0L);
			continue;
		}

		if (firstTime == TRUE && chequeFound == TRUE)
		{
			fprintf (fout,".EOF\n");
			fflush (stdout);
			pclose (fout);
			cc = LineUpForm ();
			if (cc)
			{
				reprintHash = suhd_rec.hhsp_hash;

				abc_unlock ("suhd");
				cc = find_hash ("suhd",&suhd_rec,GTEQ,"u", (pidNumber) ? pidNumber : 0L);
				MainHeading (lpno);
				reprint = TRUE;
				continue;
			}
			else
			{
				firstTime = FALSE;
				MainHeading (lpno);
			}
		}
		if (reprint)
			continue;

		abc_unlock ("suhd");

		cc = find_hash ("suhd",&suhd_rec,NEXT,"u", (pidNumber) ? pidNumber : 0L);
	}
	abc_unlock ("suhd");

	fprintf (fout,".EOF\n");
	fflush (stdout);
	pclose (fout);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, (pidNumber) 
											? "suhd_pid" : "suhd_hhsp_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsi_hash");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*===============================
| Close Data Base Files.       	|
===============================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suhd);
	abc_fclose (sudt);
	abc_fclose (suin);
	abc_fclose (crbk);
	abc_fclose (pocr);
	abc_dbclose (data);
}

/*===============================================================
|	Routine to process cheque.  							    |
|	Checks if there is supplier record on file for cheque.	    |
|	- if there isn't then control is returned to main.	        |
|	Returns: non-zero if not ok.				                |
===============================================================*/
int
ProcessSuhd (
 void)
{
	if (reprintHash != -1 && suhd_rec.hhsp_hash != reprintHash)
		return (EXIT_FAILURE);

	if (suhd_rec.tot_amt_paid == 0)
		return (EXIT_FAILURE);
		
	/*--------------------------------------
	| Not flagged as cheque or Remittance. |
	--------------------------------------*/
	if (!reprint && suhd_rec.stat_flag[0] != 'C' &&
	     suhd_rec.rem_prt[0] != 'R')
		return (EXIT_FAILURE);

	reprint = 0;
	reprintHash = -1;

	/*--------------------------------
	| Read supplier master record.   |
	--------------------------------*/
	cc = find_hash ("sumr",&sumr_rec,COMPARISON,"r",suhd_rec.hhsu_hash);
	if (cc)
		file_err (cc, "sumr", "DBFIND");

	dsp_process (" Supplier : ",sumr_rec.acronym);

	/*------------------------------------
	| Print combined remittance & cheque.|
	------------------------------------*/
	if (BOTH)
	{
		PrintRemit();
		PrintCheque ();
	}	

	/*--------------------------------
	| Print separate remittance.     |
	--------------------------------*/
	else if (REMITT)
		PrintRemit();

	/*--------------------------------
	| Print separate cheque.         |
	--------------------------------*/
	else if (CHEQUE)
		PrintCheque ();

	chequeFound = TRUE;

	if (CHEQUE)
		strcpy (suhd_rec.stat_flag,"0");

	if (REMITT)
		strcpy (suhd_rec.rem_prt,  "0");

	if (BOTH)
	{
		strcpy (suhd_rec.stat_flag,"0");
		strcpy (suhd_rec.rem_prt,  "0");
	}
	
	cc = abc_update ("suhd", &suhd_rec);
	if (cc)
		file_err (cc, "suhd", "DBFIND");

	return (EXIT_SUCCESS);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadComm (
 void)
{
	int	i;
	int	j;
	int	cy;			/* Current debtors month */
	int	cm;			/* Current debtors month */
	int	cd;			/* Current debtors day	*/

	static	int	days[14] = {0,31,28,31,30,31,30,31,31,30,31,30,31,31};
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*------------------------------------------------------
	| Find dates for ageing i.e. end of last three months. |
	------------------------------------------------------*/
	ageDays[0] = comm_rec.tcrd_date; ageDays[4] = 0L;

	DateToDMY (comm_rec.tcrd_date, &cd, &cm, &cy);

	/*---------------------------
	| Adjust feb for leap year. |
	---------------------------*/
	days[2] = (cy % 4) == 0 ? 29 : 28;

	/*--------------------------------------
	| If day not end of month,adjust day. |
	--------------------------------------*/
	if (cd < days[cm])
		ageDays[0] += (days[cm] - cd);

	for (i = 1;i< 4;i++) 
	{
		j = (cm - i);
		if (j < 1)
			j += 12;

		ageDays[i] = ageDays[i - 1] - days[j + 1];
	}
}

/*=======================================
| Print remittance advice before cheque	|
=======================================*/
void
PrintRemit(
 void)
{
	int		rem_cnt = 0;
	int		i, 
			countLineNo;
	double	chq_tot = 0.00;

	/*-----------------------------------
	| Initialise Remmittance Structure. |
	-----------------------------------*/
	for (countLineNo = 0; countLineNo < LCL_MAX_LINES; countLineNo++)
	{
		strcpy (remit [countLineNo].invoiceDate, 	"          ");
		strcpy (remit [countLineNo].invoiceNumber, 	"               ");
		remit [countLineNo].invoiceAmount = 0.00;
	}
	
	pr_format (fin,fout,"HMARGIN",0,0);

	countLineNo = 0;
	rem_cnt = 0;

	/*------------------------------
	| Process all cheque details . |
	------------------------------*/
	sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
	cc = find_rec ("sudt", &sudt_rec, GTEQ, "r");
	while (!cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash)
	{
		suin_rec.hhsi_hash = sudt_rec.hhsi_hash;
		cc = find_rec ("suin", &suin_rec, COMPARISON, "r");
		if (!cc)
		{
			strcpy (remit[rem_cnt].invoiceDate,DateToString (suin_rec.date_of_inv));
			strcpy (remit[rem_cnt].invoiceNumber, suin_rec.inv_no);
			remit[rem_cnt++].invoiceAmount = sudt_rec.amt_paid_inv;
			chq_tot += sudt_rec.amt_paid_inv;
		}
		cc = find_rec ("sudt", &sudt_rec, NEXT, "r");
	}
 	/*-----------------------------------
	| Process all cheque details found. |
 	-----------------------------------*/
	for (i = 0; i < rem_cnt; i++)
	{
 		/*--------------------------------
		| Cheque has gone over one page. |
 		--------------------------------*/
		if (countLineNo != 0 && countLineNo % LINES == 0)
		{
 			/*-------------------------
			| Page is full so cancel. |
 			-------------------------*/
			if (strcmp (remit[i + LINES].invoiceNumber, "               "))
			{
				if (!REMITT)
					CancelCheque ();
				i += LINES - 1;
				countLineNo = 0;
			}
			else
				break;
		}
		else
		{
			PrintLine(i);
			countLineNo++;
		}
	}
	pr_format (fin,fout,"VBLE_BLANK",1,LINES - countLineNo);
	pr_format (fin,fout,"MARGIN1",0,0);
	pr_format (fin,fout,"CHQ_NUMBER",1," ");
	pr_format (fin,fout,"CHQ_NUMBER",2,chq_tot);
	pr_format (fin,fout,"MARGIN2",0,0);
}

/*=============================
| Print Double column cheque. |
=============================*/
void
PrintLine(
 int i)
{
	char	paymentDate [11],
			paymentInvoice [16],
			paymentAmount[15];

	if (strcmp (remit[i].invoiceNumber, "               ") != 0)
	{
		strcpy (paymentDate ,remit[i].invoiceDate);
		sprintf (paymentInvoice,"%15.15s",remit[i].invoiceNumber);
		sprintf (paymentAmount,"%12.2f",DOLLARS (remit[i].invoiceAmount));
		
		pr_format (fin,fout,"REM_LINE",1,paymentDate);
		pr_format (fin,fout,"REM_LINE",2,paymentInvoice);
		pr_format (fin,fout,"REM_LINE",3,paymentAmount);
	}
	else
	{
		pr_format (fin,fout,"REM_LINE",1," ");
		pr_format (fin,fout,"REM_LINE",2," ");
		pr_format (fin,fout,"REM_LINE",3," ");
	}
	if (strcmp (remit[i + LINES].invoiceNumber, "               ") != 0)
	{
		sprintf (paymentAmount,"%12.2f",DOLLARS(remit[i+ LINES].invoiceAmount));
		strcpy (paymentDate,remit[i + LINES].invoiceDate);
		sprintf (paymentInvoice,"%15.15s",remit[i + LINES].invoiceNumber);
		
		pr_format (fin,fout,"REM_LINE",4,paymentDate);
		pr_format (fin,fout,"REM_LINE",5,paymentInvoice);
		pr_format (fin,fout,"REM_LINE",6,paymentAmount);
	}
	else
	{
		pr_format (fin,fout,"REM_LINE",4," ");
		pr_format (fin,fout,"REM_LINE",5," ");
		pr_format (fin,fout,"REM_LINE",6," ");
	}
}
/*===========================
| Routine to cancel cheque. |
===========================*/
void
CancelCheque (
 void)
{
	int	countLineNo;

	pr_format (fin,fout,"MARGIN1",0,0);
	pr_format (fin,fout,"CONTINUED",0,0);
	pr_format (fin,fout,"CONT_SKIP1",0,0);

	for (countLineNo = 0;countLineNo < 7;countLineNo++)
		pr_format (fin,fout,"CANCELLED",0,0);

	pr_format (fin,fout,"CONT_SKIP2",0,0);
	pr_format (fin,fout,"NEXT_PAGE",0,0);
	pr_format (fin,fout,"HMARGIN",0,0);
}

/*===============================================================
|	Routine to print the cheque portion of the form.			|
===============================================================*/
void
PrintCheque (
 void)
{
	char *	sptr;
	char	num_str[16];

	/*------------------------------------------ 
	| Fill leading places with '*' characters. |
	------------------------------------------ */
	sprintf (num_str,"%.2f",DOLLARS (suhd_rec.tot_amt_paid));
	StarFill (num_str);

	/*------------------------------------
	| Get prime & sub units of currency. |
	------------------------------------*/
	GetCurrUnits ();

	/*----------------------
	| Get amount as words. |
	----------------------*/
	words = dbltow (suhd_rec.tot_amt_paid, primeUnit, subUnit);

	if (strlen (words) > 74)
	{
		sptr = words + 74;

		while (*sptr != ' ' && sptr > words)
			sptr--;
		
		if (*sptr == ' ')
		{
			*sptr = '\0';
			sptr++;
		}
		else
			sptr = (char *)NULL;
	}
	else
		sptr = (char *)NULL;

	pr_format (fin,fout,"CHQ_DATE",1, DateToString (suhd_rec.date_payment));
	pr_format (fin,fout,"MARGIN3",0,0);
	pr_format (fin,fout,"WORDS",1,words);
	pr_format (fin,fout,"WORDS",1, (sptr != (char *)NULL) ? sptr : " ");
	pr_format (fin,fout,"MARGIN4",0,0);
	pr_format (fin,fout,"CHQ_TOTAL",1,num_str);
	pr_format (fin,fout,"MARGIN5",0,0);
	pr_format (fin,fout,"CUST_INFO",1,sumr_rec.crd_name);
	pr_format (fin,fout,"CUST_INFO",1,sumr_rec.adr1);
	pr_format (fin,fout,"CUST_INFO",1,sumr_rec.adr2);
	pr_format (fin,fout,"CUST_INFO",1,sumr_rec.adr3);

	pr_format (fin,fout,"NEXT_PAGE",0,0);
	fflush (fout);
}

/*------------------------------------
| Get prime & sub units of currency. |
------------------------------------*/
void
GetCurrUnits (
 void)
{
	/*---------------------------------
	| Look up bank record for cheque. |
	---------------------------------*/
	strcpy (crbk_rec.co_no, comm_rec.tco_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", suhd_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "crbk", "DBFIND");

	/*-----------------------------------
	| Look up currency record for bank. |
	-----------------------------------*/
	strcpy (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", crbk_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pocr", "DBFIND");

	sprintf (primeUnit, "%s ", clip (pocr_rec.prime_unit));
	sprintf (subUnit,   "%s ", clip (pocr_rec.sub_unit));
}

/*=======================================
|	Routine that opens pipe to standard	|
|	print and sends initial data.		|
=======================================*/
void
MainHeading (
 int lpno)
{
	if ((fin = pr_open ("cr_chqprn.p")) == NULL)
		sys_err ("Error in cr_chqprn.p During (POPEN)",errno,PNAME);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (fout,".START\n");
	fprintf (fout,".OP\n");
	fprintf (fout,".PL0\n");
	fprintf (fout,".LP%d\n",lpno);
	fprintf (fout,".2\n");
	fprintf (fout,".L110\n");
	fprintf (fout,".PI12\n");
	fflush (fout);
}

/*===================================================
|	Routine to place leading '*' characters before	|
|	a string passed to it,allows for 13 digits in	|
|	total.  To adjust no. of stars printed add or	|
|	remove stars from 'fill' - adding stars will	|
|	lengthen field - removing will shorten field.	|
===================================================*/
void
StarFill (
 char *	num_str)
{
	char *	fill = "***********  ";
	int 	num_len;

	strcpy (temp_str,num_str);
	num_len = strlen (num_str);
	if (num_len > (int)strlen (fill))	/* Avoid illegal subscript */
		num_len = strlen (fill);

	sprintf (num_str,"%s%s",fill + num_len,temp_str);
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*======================================
| Routine to check if paper loaded.    |
======================================*/
int
LoadPaper (
 void)
{
	int	c;

	if (CHEQUE || BOTH)
		c = prmptmsg (ML (mlCrMess017), "Yy", 22,3);
	else
		c = prmptmsg (ML (mlCrMess018), "Yy", 22,3);
	
	fflush (stdout);

	rv_pr (ML (mlCrMess020), 21,3, 1);

	fflush (stdout);
	return (EXIT_SUCCESS);
}

/*======================================
| Routine to reprint cheque for lineup |
======================================*/
int
LineUpForm (
 void)
{
	int	c;

	c = prmptmsg (ML (mlCrMess019), "YyNn", 22,4);

	fflush (stdout);
	if (c == 'Y' || c == 'y')
		return (EXIT_FAILURE);
	
	rv_pr (ML (mlCrMess021) ,20,4, 1);

	fflush (stdout);
	return (EXIT_SUCCESS);
}
