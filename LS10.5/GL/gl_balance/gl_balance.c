/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_balance.c,v 5.0 2002/05/08 01:20:25 scott Exp $
|  Program Name  : (gl_balance.c) 
|  Program Desc  : (balance checking against ledgers.)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow      | Date Written : 20th Aug 2001 |
|---------------------------------------------------------------------|
| $Log: gl_balance.c,v $
| Revision 5.0  2002/05/08 01:20:25  scott
| CVS administration
|
| Revision 1.12  2001/09/28 02:38:53  robert
| Updated to adjust "WARNING" message display position to avoid overlap
| with other windows in LS10-GUI
|
| Revision 1.11  2001/09/11 23:14:02  scott
| Updated from Scott machine - 12th Sep 2001
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_balance.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_balance/gl_balance.c,v 5.0 2002/05/08 01:20:25 scott Exp $";
/*
 *   Program specific defines.  
 */
#define		CF(x)	comma_fmt (DOLLARS (x), "NNN,NNN,NNN,NNN.NN")
#define		CFD(x)	comma_fmt (x, "NNN,NNN,NNN,NNN.NN")

#define		LINE_INTERFACE	0
#define		LINE_ACCOUNT	1
#define		LINE_PART_ONE	3
#define		LINE_AR_LEDGER	5
#define		LINE_PART_TWO	7

#define		LINE_LEFT		0
#define		LINE_MIDDLE		1
#define		LINE_RIGHT		2
#define		LINE_RULER		6

#define		BALANCE_AR		1
#define		BALANCE_AP		2
#define		BALANCE_SK		3

#define	IF_NULL		(struct IF_LIST *) NULL

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <tabdisp.h>
#include <hot_keys.h>
#include <GlUtils.h>
#include <Costing.h>
#include <twodec.h>
#include <GlIntegrityReport.h>

#include	"schema"

struct	commRecord	comm_rec;
struct	comrRecord	comr_rec;
struct	glisRecord	glis_rec;
struct	cumrRecord	cumr_rec;
struct	cuhdRecord	cuhd_rec;
struct	cuinRecord	cuin_rec;
struct	sumrRecord	sumr_rec;
struct	suhdRecord	suhd_rec;
struct	suinRecord	suin_rec;
struct	excfRecord	excf_rec;
struct	inccRecord	incc_rec;
struct	inmrRecord	inmr_rec;
struct	sttfRecord	sttf_rec;

/*
 *   Constants, defines and stuff   
 */
	char	*glmr2 			= "glmr2",
        	*data 			= "data",
			*elevenSpaces	=	"           ";

	long	startDate 	= 0L,
			endDate 	= 0L;

	double	ledgerBalance 	= 	0.00,
			totalDebits		=	0.00,
			totalCredits	=	0.00;

	int		envDbNettUsed 		= 	TRUE,
			envDbMcurr 	  		= 	TRUE,
			envCrMcurr 	  		= 	TRUE,
			envSkValNegative	=	FALSE,
			firstComment 		= 	TRUE,
			firstBatch 			= 	TRUE,
			programType 		=	0;

	char	dispStr 			[256],
			leftString	 		[43],
			middleString		[43],
			rightString			[43],
			prdDebitCharStr		[21],
			prdCreditCharStr	[21],
			prdTotalCharStr		[21];


static	int	RestartFunc	 	(int, KEY_TAB *);
static	int	ExitFunc	 	(int, KEY_TAB *);

	static KEY_TAB listKeys [] =
	{
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};


/*
 * Link list stuff for storage of accounts.
 */
struct IF_LIST
{
	char	gl_interface	[11];
	char	accountNo 		[17];
	char	categoryNo 		[12];
	long	hhmrHash;
	struct	IF_LIST *_next;
};

struct	IF_LIST		*IF_head 	= IF_NULL;
struct	IF_LIST		*IF_tmpList;
struct	IF_LIST 	*IF_listAlloc (void);

/*
 *   Local function prototypes  
 */
double 	ProcessCategoryValue 	(char *, char *);
double	ProcessItemValue 		(void);
int     CheckCoControl 			(void);
int     MoneyZero 				(double);
void 	AccountProcessing 		(void);
void    CheckBalances 			(char *, char *, char *, long);
void	CheckCreated 			(void);
void    CloseDB 				(void);
void	DisplayProgress 		(int, int, char *);
void    IF_ClearLists 			(void);
void    IF_InsertAdditions 		(char *, char *, char *, long);
void    InitDisplay				(void);
void    OpenDB 					(void);
void    ProcessAP 				(void);
void    ProcessAR 				(void);
void 	ProcessBatch 			(void);
void 	ProcessCustomer			(void);
void	ProcessInventory		(void);
void    ProcessSK 				(void);
void	ProcessSupplier			(void);
void 	ReportNotPosted 		(void);
void    shutdown_prog 			(void);

/*
 *   Main Processing Routine    
 */
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "gl_ar_bal"))
		programType = BALANCE_AR;

	if (!strcmp (sptr, "gl_ap_bal"))
		programType = BALANCE_AP;

	if (!strcmp (sptr, "gl_sk_bal"))
		programType = BALANCE_SK;

	/*
  	 *   Get needed environment variables.
 	 */
	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("CR_MCURR");
	envCrMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_VAL_NEGATIVE");
	envSkValNegative = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	init_scr ();		/*  sets terminal from termcap	  */
	set_tty ();

	CheckCreated ();

	/*
  	 * Clear screen etc. 
 	 */
	InitDisplay ();

	/*
  	 * Main processing function.
 	 */
	if (programType == BALANCE_AR)
		ProcessAR ();

	if (programType == BALANCE_AP)
		ProcessAP ();

	if (programType == BALANCE_SK)
		ProcessSK ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 *   Standard program exit sequence     
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);			/* Get base currency */
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_alias (glmr2, glmr);

	OpenGlih ();
	OpenGlid ();
	OpenGlca ();
	OpenGlct ();
	OpenGlln ();
	OpenGlmr ();
	OpenGlpd ();
	OpenGlbh (); abc_selfield (glbh, "glbh_id_no3");
	open_rec (glmr2,glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (glis, glis_list, GLIS_NO_FIELDS, "glis_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_co_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_co_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_co_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_cat");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	GL_Close ();
	CloseCosting ();
	abc_fclose (glmr2);
	abc_fclose (glis);
	abc_fclose (cumr);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (sttf);
	abc_dbclose (data);
}

/*
 *   Initialise screen output.
 */
void
InitDisplay (void)
{
	clear ();
	swide ();
	crsr_off ();
	strcpy (leftString, " ");
	strcpy (middleString, " ");
	strcpy (rightString	, " ");
}
/*
 * Main processing routine for accounts receivable.
 */
void
ProcessAR (void)
{
	int		useAlternate	=	FALSE,
			interfaceCount	=	0,
			accountCount	=	0,
			customerCount	=	0;

	/*
 	 * Display progress stuff.
 	 */
	DisplayProgress 
	(
		LINE_INTERFACE,
		LINE_LEFT, 
		ML ("Processing General Ledger Interface :")
	);
	DisplayProgress 
	(	
		LINE_ACCOUNT,
		LINE_LEFT, 
		ML ("Processing General Ledger Account   :")
	);

	/*
 	 * Clear link list.
 	 */
	IF_ClearLists ();

	/*
 	 * Process control table for Accounts receivable.
 	 */
	strcpy (glis_rec.co_no, comm_rec.co_no);
	strcpy (glis_rec.code,  "AR");
	strcpy (glis_rec.find_inf,  "          ");
	cc = find_rec (glis, &glis_rec, GTEQ, "r");
	while (!cc && !strcmp (glis_rec.co_no, comm_rec.co_no) &&
				  !strncmp (glis_rec.code,  "AR", 2))
	{
		if (strcmp (glis_rec.use_inf, "          "))
			useAlternate	=	TRUE;
		else
			useAlternate	=	FALSE;

		/*
 	  	 * Read general ledger interface header. 
 	 	 */
		strcpy (glihRec.co_no, comm_rec.co_no);
		strcpy (glihRec.int_code, glis_rec.find_inf);
		sprintf (glihRec.class_type, "%3.3s", 	" ");
		sprintf (glihRec.cat_no, 	 "%11.11s", " ");
		cc = find_rec (glih, &glihRec, GTEQ, "r");
		while (!cc && !strcmp (glihRec.co_no, comm_rec.co_no) &&
					  !strcmp (glihRec.int_code, glis_rec.find_inf))
		{
			/*
 	  	 	 * Look for alternate/same as interface. 
 	 	 	 */
			if (useAlternate)
			{
				sprintf 
				(
					err_str, 
					" [%s] - same as [%s]", 
					glihRec.int_code,
					glis_rec.use_inf
				);
			}
			else
			{
				sprintf (err_str, " [%s] - %s", glihRec.int_code,glis_rec.desc);
			}
			DisplayProgress (LINE_INTERFACE, LINE_MIDDLE, err_str);
			sprintf 
			(
				err_str, 
				ML (" Interfaces records processed      : %02d"), 
				interfaceCount++
			);
			DisplayProgress (LINE_INTERFACE, LINE_RIGHT, err_str);

			/*
 	  	 	 * Get accounts from interface detail file.
 	 	 	 */
			glidRec.hhih_hash	=	glihRec.hhih_hash;
			strcpy (glidRec.br_no,  "  ");
			strcpy (glidRec.wh_no,  "  ");
			cc = find_rec (glid, &glidRec, GTEQ, "r");
			while (!cc && glidRec.hhih_hash	== glihRec.hhih_hash)
			{
				/*
 	  	 	 	 * Get general ledger account.
 	 	 	 	 */
				strcpy (glmrRec.co_no, comm_rec.co_no);
				strcpy (glmrRec.acc_no, glidRec.acct_no);
				cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (glid, &glidRec, NEXT, "r");
					continue;
				}

				sprintf (err_str, " [%s]", glmrRec.acc_no);
				DisplayProgress (LINE_ACCOUNT, LINE_MIDDLE, err_str);

				sprintf 
				(
					err_str, 
					ML (" General Ledger Accounts processed : %02d"), 
					accountCount++
				);
				DisplayProgress (LINE_ACCOUNT, LINE_RIGHT, err_str);

				/*
 	  	 	 	 * Insert additions to link list
 	 	 	 	 */
				IF_InsertAdditions 
				(
					(useAlternate) ? glis_rec.use_inf : glis_rec.find_inf,
					glmrRec.acc_no,
					elevenSpaces,
					glmrRec.hhmr_hash
				);
				cc = find_rec (glid, &glidRec, NEXT, "r");
			}
			cc = find_rec (glih, &glihRec, NEXT, "r");
		}
		cc = find_rec (glis, &glis_rec, NEXT, "r");
	}
	sprintf (err_str, " [ACCT REC  ] - %s", ML ("Customer specific"));
	DisplayProgress (LINE_INTERFACE, LINE_MIDDLE, err_str);
	sprintf 
	(
		err_str, 
		ML (" Interfaces records processed      : %02d"), 
		interfaceCount++
	);
	DisplayProgress (LINE_INTERFACE, LINE_RIGHT, err_str);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * Get general ledger account.
		 */
		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, cumr_rec.gl_ctrl_acct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		sprintf (err_str, " [%s]", glmrRec.acc_no);
		DisplayProgress (LINE_ACCOUNT, LINE_MIDDLE, err_str);

		sprintf 
		(
			err_str, 
			ML (" General Ledger Accounts processed : %02d"), 
			accountCount++
		);
		DisplayProgress (LINE_ACCOUNT, LINE_RIGHT, err_str);
		/*
		 * Insert additions to link list
		 */
		IF_InsertAdditions 
		(
			"ACCT REC  ",
			glmrRec.acc_no,
			elevenSpaces,
			glmrRec.hhmr_hash
		);
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	/* 
	 * Process all accounts stored into link list.
	 */
	AccountProcessing ();
	ProcessBatch ();

	/* 
	 * Now it's time to process customer ledger balance.
	 */
	DisplayProgress (LINE_PART_ONE - 1, LINE_RULER, "");
	DisplayProgress 
	(
		LINE_PART_ONE,
		LINE_LEFT, 
		ML ("Phase one complete                  :")
	);
	sprintf (prdDebitCharStr,	"%18.18s", CF (totalDebits));
	sprintf (prdCreditCharStr,	"%18.18s", CF (totalCredits));
	sprintf 
	(
		err_str, 
		ML (" Total Balance: %s"), 
		prdDebitCharStr
	);
	DisplayProgress (LINE_PART_ONE,LINE_MIDDLE, err_str);
	/*
	sprintf 
	(
		err_str, 
		ML (" Total Credits : %s"), 
		prdCreditCharStr
	);
	DisplayProgress (LINE_PART_ONE,LINE_RIGHT, err_str);
	*/
	DisplayProgress ( LINE_PART_ONE + 1, LINE_RULER, "");

	ledgerBalance = 0.00;

	DisplayProgress 
	(
		LINE_AR_LEDGER,
		LINE_LEFT, 
		ML ("Processing Account Receivables      :")
	);
	/* 
	 * Work routine to get customer ledger balance.
	 */
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		sprintf (err_str, " [%s-%s]", cumr_rec.dbt_no, cumr_rec.dbt_acronym);
		DisplayProgress (LINE_AR_LEDGER, LINE_MIDDLE, err_str);
		ProcessCustomer ();
		sprintf 
		(
			err_str, 
			ML (" Customer Accounts processed       : %02d"), 
			customerCount++
		);
		DisplayProgress (LINE_AR_LEDGER, LINE_RIGHT, err_str);
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	DisplayProgress (LINE_PART_TWO - 1, LINE_RULER, "");
	DisplayProgress 
	(
		LINE_PART_TWO,
		LINE_LEFT, 
		ML ("Phase two complete                  :")
	);
	sprintf (prdTotalCharStr,	"%18.18s", CF (ledgerBalance));
	sprintf 
	(
		err_str, 
		ML (" Total ledger : %s"), 
		prdTotalCharStr
	);
	DisplayProgress (LINE_PART_TWO,LINE_MIDDLE, err_str);
	tab_scan ("DisplayProgress");
	tab_close ("DisplayProgress", TRUE);
	if (firstBatch == FALSE)
		tab_close ("NotPosted", TRUE);
}
/*
 * Main processing routine for accounts Payable.
 */
void
ProcessAP (void)
{
	int		useAlternate	=	FALSE;
	int		interfaceCount	=	0,
			accountCount	=	0,
			supplierCount	=	0;

	/*
 	 * Display progress stuff.
 	 */
	DisplayProgress 
	(
		LINE_INTERFACE,
		LINE_LEFT, 
		ML ("Processing General Ledger Interface :")
	);
	DisplayProgress 
	(	
		LINE_ACCOUNT,
		LINE_LEFT, 
		ML ("Processing General Ledger Account   :")
	);

	/*
 	 * Clear link list.
 	 */
	IF_ClearLists ();

	/*
 	 * Process control table for Accounts payable.
 	 */
	strcpy (glis_rec.co_no, comm_rec.co_no);
	strcpy (glis_rec.code,  "AP");
	strcpy (glis_rec.find_inf,  "          ");
	cc = find_rec (glis, &glis_rec, GTEQ, "r");
	while (!cc && !strcmp (glis_rec.co_no, comm_rec.co_no) &&
				  !strncmp (glis_rec.code,  "AP", 2))
	{
		if (strcmp (glis_rec.use_inf, "          "))
			useAlternate	=	TRUE;
		else
			useAlternate	=	FALSE;

		/*
 	  	 * Read general ledger interface header. 
 	 	 */
		strcpy (glihRec.co_no, comm_rec.co_no);
		strcpy (glihRec.int_code, glis_rec.find_inf);
		sprintf (glihRec.class_type, "%3.3s", 	" ");
		sprintf (glihRec.cat_no, 	 "%11.11s", " ");
		cc = find_rec (glih, &glihRec, GTEQ, "r");
		while (!cc && !strcmp (glihRec.co_no, comm_rec.co_no) &&
					  !strcmp (glihRec.int_code, glis_rec.find_inf))
		{
			/*
 	  	 	 * Look for alternate/same as interface. 
 	 	 	 */
			if (useAlternate)
			{
				sprintf 
				(
					err_str, 
					" [%s] - same as [%s]", 
					glihRec.int_code,
					glis_rec.use_inf
				);
			}
			else
			{
				sprintf (err_str, " [%s] - %s", glihRec.int_code,glis_rec.desc);
			}
			DisplayProgress (LINE_INTERFACE, LINE_MIDDLE, err_str);
			sprintf 
			(
				err_str, 
				ML (" Interfaces records processed      : %02d"), 
				interfaceCount++
			);
			DisplayProgress (LINE_INTERFACE, LINE_RIGHT, err_str);

			/*
 	  	 	 * Get accounts from interface detail file.
 	 	 	 */
			glidRec.hhih_hash	=	glihRec.hhih_hash;
			strcpy (glidRec.br_no,  "  ");
			strcpy (glidRec.wh_no,  "  ");
			cc = find_rec (glid, &glidRec, GTEQ, "r");
			while (!cc && glidRec.hhih_hash	== glihRec.hhih_hash)
			{
				/*
 	  	 	 	 * Get general ledger account.
 	 	 	 	 */
				strcpy (glmrRec.co_no, comm_rec.co_no);
				strcpy (glmrRec.acc_no, glidRec.acct_no);
				cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (glid, &glidRec, NEXT, "r");
					continue;
				}

				sprintf (err_str, " [%s]", glmrRec.acc_no);
				DisplayProgress (LINE_ACCOUNT, LINE_MIDDLE, err_str);

				sprintf 
				(
					err_str, 
					ML (" General Ledger Accounts processed : %02d"), 
					accountCount++
				);
				DisplayProgress (LINE_ACCOUNT, LINE_RIGHT, err_str);

				/*
 	  	 	 	 * Insert additions to link list
 	 	 	 	 */
				IF_InsertAdditions 
				(
					(useAlternate) ? glis_rec.use_inf : glis_rec.find_inf,
					glmrRec.acc_no,
					elevenSpaces,
					glmrRec.hhmr_hash
				);
				cc = find_rec (glid, &glidRec, NEXT, "r");
			}
			cc = find_rec (glih, &glihRec, NEXT, "r");
		}
		cc = find_rec (glis, &glis_rec, NEXT, "r");
	}
	sprintf (err_str, " [ACCT PAY  ] - %s", ML ("Supplier specific"));
	DisplayProgress (LINE_INTERFACE, LINE_MIDDLE, err_str);
	sprintf 
	(
		err_str, 
		ML (" Interfaces records processed      : %02d"), 
		interfaceCount++
	);
	DisplayProgress (LINE_INTERFACE, LINE_RIGHT, err_str);

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.gl_ctrl_acct, "                ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		/*
		 * Get general ledger account.
		 */
		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.acc_no, sumr_rec.gl_ctrl_acct);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		sprintf (err_str, " [%s]", glmrRec.acc_no);
		DisplayProgress (LINE_ACCOUNT, LINE_MIDDLE, err_str);

		sprintf 
		(
			err_str, 
			ML (" General Ledger Accounts processed : %02d"), 
			accountCount++
		);
		DisplayProgress (LINE_ACCOUNT, LINE_RIGHT, err_str);
		/*
		 * Insert additions to link list
		 */
		IF_InsertAdditions 
		(
			"ACCT PAY  ",
			glmrRec.acc_no,
			elevenSpaces,
			glmrRec.hhmr_hash
		);
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	/* 
	 * Process all accounts stored into link list.
	 */
	ProcessBatch ();
	AccountProcessing ();

	/* 
	 * Now it's time to process supplier ledger balance.
	 */
	DisplayProgress (LINE_PART_ONE - 1, LINE_RULER, "");
	DisplayProgress 
	(
		LINE_PART_ONE,
		LINE_LEFT, 
		ML ("Phase one complete                  :")
	);
	sprintf (prdDebitCharStr,	"%18.18s", CF (totalDebits));
	sprintf (prdCreditCharStr,	"%18.18s", CF (totalCredits));
	sprintf 
	(
		err_str, 
		ML (" Total Balance: %s"), 
		prdCreditCharStr
	);
	DisplayProgress (LINE_PART_ONE,LINE_MIDDLE, err_str);
	DisplayProgress ( LINE_PART_ONE + 1, LINE_RULER, "");

	ledgerBalance = 0.00;

	DisplayProgress 
	(
		LINE_AR_LEDGER,
		LINE_LEFT, 
		ML ("Processing Account Payable          :")
	);
	/* 
	 * Work routine to get supplier ledger balance.
	 */
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no))
	{
		sprintf (err_str, " [%s-%s]", sumr_rec.crd_no, sumr_rec.acronym);
		DisplayProgress (LINE_AR_LEDGER, LINE_MIDDLE, err_str);
		ProcessSupplier ();
		sprintf 
		(
			err_str, 
			ML (" Supplier Accounts processed       : %02d"), 
			supplierCount++
		);
		DisplayProgress (LINE_AR_LEDGER, LINE_RIGHT, err_str);
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	DisplayProgress (LINE_PART_TWO - 1, LINE_RULER, "");
	DisplayProgress 
	(
		LINE_PART_TWO,
		LINE_LEFT, 
		ML ("Phase two complete                  :")
	);
	sprintf (prdTotalCharStr,	"%18.18s", CF (ledgerBalance));
	sprintf 
	(
		err_str, 
		ML (" Total ledger : %s"), 
		prdTotalCharStr
	);
	DisplayProgress (LINE_PART_TWO,LINE_MIDDLE, err_str);
	tab_scan ("DisplayProgress");
	tab_close ("DisplayProgress", TRUE);
	if (firstBatch == FALSE)
		tab_close ("NotPosted", TRUE);
}
/*
 * Main processing routine for inventory.
 */
void
ProcessSK (void)
{
	int		useAlternate	=	FALSE,
			categoryCounter	=	0,
			nonByCategory	=	TRUE;

	double	totalCategory	=	0.00,
			grandDebitGl	=	0.00,
			grandCreditGl	=	0.00,
			grandCategory	=	0.00;

	abc_selfield (glih, "glih_id_no2");

	tab_open ("DisplayProgress", listKeys, 0, 0, 19, FALSE);
	tab_add ("DisplayProgress", "#%-s", "  Category   |          Category Description          | G/L Debit Value.  | G/L Credit Value. | Inventory Value.  ");
	/*
 	 * Clear link list.
 	 */
	IF_ClearLists ();

	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no, elevenSpaces);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		/*
 	 	* Process control table for Inventory.
 	 	*/
		strcpy (glis_rec.co_no, comm_rec.co_no);
		strcpy (glis_rec.code,  "SK");
		strcpy (glis_rec.find_inf,  "          ");
		cc = find_rec (glis, &glis_rec, GTEQ, "r");
		while (!cc && !strcmp (glis_rec.co_no, comm_rec.co_no) &&
				  	  !strncmp (glis_rec.code,  "SK", 2))
		{
			if (strcmp (glis_rec.use_inf, "          "))
				useAlternate	=	TRUE;
			else
				useAlternate	=	FALSE;
		
			/*
 	  	 	 * Read general ledger interface header. 
 	 	 	 */
			strcpy (glihRec.co_no, comm_rec.co_no);
			strcpy (glihRec.int_code, glis_rec.find_inf);
			sprintf (glihRec.cat_no,  excf_rec.cat_no);
			cc = find_rec (glih, &glihRec, GTEQ, "r");
			while (!cc && !strcmp (glihRec.co_no, comm_rec.co_no) &&
					  	  !strcmp (glihRec.int_code, glis_rec.find_inf) &&
					  	  !strcmp (glihRec.cat_no, excf_rec.cat_no))
			{
				nonByCategory	=	FALSE;
				/*
 	  	 	 	* Get accounts from interface detail file.
 	 	 	 	*/
				glidRec.hhih_hash	=	glihRec.hhih_hash;
				strcpy (glidRec.br_no,  "  ");
				strcpy (glidRec.wh_no,  "  ");
				cc = find_rec (glid, &glidRec, GTEQ, "r");
				while (!cc && glidRec.hhih_hash	== glihRec.hhih_hash)
				{
					/*
					 * Get general ledger account.
					 */
					strcpy (glmrRec.co_no, comm_rec.co_no);
					strcpy (glmrRec.acc_no, glidRec.acct_no);
					cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
					if (cc)
					{
						cc = find_rec (glid, &glidRec, NEXT, "r");
						continue;
					}
					/*
					 * Insert additions to link list
					 */
					IF_InsertAdditions 
					(
						(useAlternate) ? glis_rec.use_inf : glis_rec.find_inf,
						glmrRec.acc_no,
						excf_rec.cat_no,
						glmrRec.hhmr_hash
					);
					cc = find_rec (glid, &glidRec, NEXT, "r");
				}
				cc = find_rec (glih, &glihRec, NEXT, "r");
			}
			cc = find_rec (glis, &glis_rec, NEXT, "r");
		}
		totalCategory	=	ProcessCategoryValue 
							(
								comm_rec.co_no, 
								excf_rec.cat_no
							);
		AccountProcessing ();

		sprintf (prdCreditCharStr,	"%18.18s", CF (totalCredits));
		sprintf (prdDebitCharStr,	"%18.18s", CF (totalDebits));
		sprintf (prdTotalCharStr,	"%18.18s", CFD (totalCategory));

		grandDebitGl	+=	totalDebits;
		grandCreditGl	+=	totalCredits;
		grandCategory	+=	totalCategory;
		sprintf 
		(
			err_str, 
			" %-11.11s |%-40.40s|%18.18s |%18.18s |%18.18s",
			excf_rec.cat_no, 
			excf_rec.cat_desc, 
			prdDebitCharStr,  
			prdCreditCharStr, 
			prdTotalCharStr
		);
		tab_add ("DisplayProgress", "%-s", err_str);
		categoryCounter++;

		IF_ClearLists ();
		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	tab_add ("DisplayProgress", "%-s", "-------------|----------------------------------------|-------------------|-------------------|-------------------");
	sprintf (prdCreditCharStr,	"%18.18s", CF (grandCreditGl));
	sprintf (prdDebitCharStr,	"%18.18s", CF (grandDebitGl));
	sprintf (prdTotalCharStr,	"%18.18s", CFD (grandCategory));
	sprintf 
	(
		err_str, 
		" %-11.11s |%40.40s|%18.18s |%18.18s |%18.18s",
		ML ("GRAND TOTAL"),
		" ",
		prdDebitCharStr,  
		prdCreditCharStr, 
		prdTotalCharStr
	);
	tab_add ("DisplayProgress", "%-s", err_str);
	tab_add ("DisplayProgress", "%-s", "-------------|----------------------------------------|-------------------|-------------------|-------------------");
	if (nonByCategory == TRUE)
	{
		tab_add ("DisplayProgress", "%-s", ML ("WARNING : This report is unlikely to produce valid results"));
		tab_add ("DisplayProgress", "%-s", ML ("        : Interface does not appear to be defined at a category level"));
	}
	tab_scan ("DisplayProgress");
	tab_close ("DisplayProgress", TRUE);
	if (firstBatch == FALSE)
		tab_close ("NotPosted", TRUE);
}

/*
 * Clear all link list entries.
 */
void
IF_ClearLists (void)
{
	struct	IF_LIST	*next_ptr;

	for (IF_tmpList = IF_head; IF_tmpList != IF_NULL; IF_tmpList = next_ptr)
	{
		next_ptr = IF_tmpList->_next;
		free (IF_tmpList);
	}
	IF_head = IF_NULL;
}
/*
 * Routine to insert or accumulate interface values 
 */
void
IF_InsertAdditions (
	char	*gl_interface,
	char	*accountNo,
	char	*categoryNo,
	long	hhmrHash)
{
	struct	IF_LIST	*IF_listAlloc (void);
	struct	IF_LIST	*head_ptr;
	struct	IF_LIST	*prev_ptr = IF_NULL;
	struct	IF_LIST	*temp_ptr = IF_NULL;

	head_ptr = IF_head;

	if (head_ptr == IF_NULL)
	{
		temp_ptr 			= IF_listAlloc ();
		sprintf (temp_ptr->gl_interface,	"%-10.10s", gl_interface);
		sprintf (temp_ptr->accountNo, 		"%-16.16s", accountNo);
		sprintf (temp_ptr->categoryNo, 		"%-11.11s", categoryNo);
		temp_ptr->hhmrHash	=	hhmrHash;
		temp_ptr->_next 	= 	IF_NULL;
        IF_head = temp_ptr;
		return;
	}

	for (IF_tmpList = head_ptr; IF_tmpList != IF_NULL; prev_ptr = IF_tmpList, IF_tmpList = IF_tmpList->_next)
	{
		if (!strcmp (IF_tmpList->gl_interface, gl_interface) &&
			!strcmp (IF_tmpList->accountNo, accountNo) &&
			!strcmp (IF_tmpList->categoryNo, categoryNo) &&
			IF_tmpList->hhmrHash	==	hhmrHash)
		{
			return;
		}
	}

	temp_ptr 			= IF_listAlloc ();
	sprintf (temp_ptr->gl_interface,	"%-10.10s", gl_interface);
	sprintf (temp_ptr->accountNo, 		"%-16.16s", accountNo);
	sprintf (temp_ptr->categoryNo, 		"%-11.11s", categoryNo);
	temp_ptr->hhmrHash	=	hhmrHash;
	if (prev_ptr == IF_NULL)
	{
		temp_ptr->_next = head_ptr;
		IF_head = temp_ptr;
	}
	else
	{
		prev_ptr->_next = temp_ptr;
		temp_ptr->_next = IF_tmpList;
	}
	return;
}
/*
 * Allocate memory for link list.
 */
struct	IF_LIST*
IF_listAlloc (void)
{
	struct	IF_LIST	*alloc_ptr;

	alloc_ptr = (struct IF_LIST *) malloc (sizeof (struct IF_LIST));
	if (alloc_ptr == IF_NULL)
		file_err (errno, "IF_listAlloc", "malloc");
    
	return (alloc_ptr);
}

/*
 *	Minor support functions
 */
int
MoneyZero (
	double	m)
{
	return (fabs (m) < 0.0001);
}

/*
 * Process accounts that have been stored into the link list.
 */
void
AccountProcessing (void)
{
	struct	IF_LIST	*IF_curr = IF_head;

	abc_selfield (glmr, "glmr_hhmr_hash");

	while (1)
	{
		if (IF_curr == IF_NULL)
        	break;

		glmrRec.hhmr_hash	=	IF_curr->hhmrHash;
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (!cc)
		{
			CheckBalances 
			(
				IF_curr->gl_interface,
				IF_curr->accountNo,
				IF_curr->categoryNo,
				IF_curr->hhmrHash
			);
		}
		IF_curr = IF_curr->_next;
	}
	return;
}

/*
 * Routines get balances for each account.
 */
void
CheckBalances (
	char	*gl_interface,
	char	*accountNo,
	char	*categoryNo,
	long	hhmrHash)
{
	int		glYear			=	0;
	int 	i;

	/*
 	 * General ledger account validation. 
 	 */
	glmrRec.hhmr_hash	=	hhmrHash;
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		return;

	/*
 	 * Process current general ledger year.
 	 */
	glYear	=	fisc_year (comm_rec.gl_date);
	for (i = 1; i < 13; i++)
	{
		glpdRec.hhmr_hash 	= glmrRec.hhmr_hash;
		glpdRec.budg_no 	= 0;
		glpdRec.year 		= glYear;
		glpdRec.prd_no 		= i;

    	cc = find_rec (glpd, &glpdRec, GTEQ, "r");
		while (!cc && 
           	glpdRec.budg_no == 0 && 
           	glpdRec.hhmr_hash == glmrRec.hhmr_hash &&
           	glpdRec.year 	== glYear &&
           	glpdRec.prd_no 	== i)
    	{
			strcpy (prdDebitCharStr, " ");
			strcpy (prdCreditCharStr, " ");
			if (glpdRec.balance >= 0.00)
			{
				totalDebits	+=	glpdRec.balance;
				 sprintf (prdDebitCharStr,"%18.18s", CF (glpdRec.balance));
			}
		 	else
		 	{
				totalCredits +=	(glpdRec.balance * -1);
			 	sprintf (prdCreditCharStr,"%18.18s", CF (glpdRec.balance * -1));
			}
			cc = find_rec (glpd, &glpdRec, NEXT, "r");
		}   /* end of while loop */
	}
}

/*
 * Main routine that places information onto the screen.
 */
void
DisplayProgress (
	int		lineNo,
	int		position,
	char	*comments)
{
	char	bufferData [256];
	int		i;

	/*
 	 * Open tab screen for first time. 
 	 */
	if (firstComment)
	{
		tab_open ("DisplayProgress", listKeys, 0, 0, 8, TRUE);
		if (programType == BALANCE_AR)
		{
			sprintf 
			(
				dispStr,
				"%35.35s%-95.95s",
				" ",
				ML("General ledger integrity reporting - Accounts receivable.")
			);
		}
		if (programType == BALANCE_AP)
		{
			sprintf 
			(
				dispStr,
				"%35.35s%-95.95s",
				" ",
				ML("General ledger integrity reporting - Accounts payable.")
			);
		}
		tab_add ("DisplayProgress", "#%130.130s", dispStr);
		for (i = 0; i < 30; i++)
			tab_add ("DisplayProgress", "%130.130s", " ");

		tab_display ("DisplayProgress", TRUE);
		firstComment = FALSE;
	}
	tab_get ("DisplayProgress", bufferData, EQUAL, lineNo);
	/*
 	 * Process data displayed on left, middle or right of tab screen.
 	 */
	switch	(position)
	{
		case	LINE_LEFT:
			sprintf (leftString, "%-42.42s", comments);
			tab_update 
			(
				"DisplayProgress", 
				"%-42.42s%-84.84s",
				leftString,
				bufferData + 42
			);
			redraw_line ("DisplayProgress", TRUE);
			break;

		case	LINE_MIDDLE:
			sprintf (middleString, "%-42.42s", comments);
			tab_update 
			(
				"DisplayProgress", 
				"%-42.42s%-42.42s%-42.42s",
				bufferData,
				middleString,
				bufferData + 84
			);
			break;

		case	LINE_RIGHT:
			sprintf (rightString, "%-42.42s", comments);
			tab_update 
			(
				"DisplayProgress", 
				"%-84.84s%-42.42s",
				bufferData,
				rightString
			);
			break;

		case	LINE_RULER:
			tab_update 
			(
				"DisplayProgress", 
				"----------------------------------------------------------------------------------------------------------------------------------"
			);
			break;
	}
	redraw_line ("DisplayProgress", TRUE);
}

/*
 * Restart key processing function.
 */
static	int
RestartFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	restart = TRUE;
	return key;
}

/*
 * Exit key processing function.
 */
static	int
ExitFunc (
	int 		key, 
	KEY_TAB 	*psUnused)
{
	return key;
}

/*
 * Process Customers transactions to obtain total balance.
 */
void
ProcessCustomer (void)
{
	double	amt = 0.00;

	cuin_rec.date_of_inv 	= 0L;
	cuin_rec.hhcu_hash 		= cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");

	/*
	 * Process Invoices/ Credits/ Journals.
	 */
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuin_rec.hhcu_hash) 
	{
		amt = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
						  	  : cuin_rec.amt;

		if (cuin_rec.exch_rate != 0.00)
			amt = no_dec (amt / cuin_rec.exch_rate);

		ledgerBalance += amt;
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}

	/*
	 * Process Cheques/ Journals.
	 */
	cuhd_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cuhd_rec.index_date	=	0L;
	strcpy (cuhd_rec.receipt_no, "        ");
		
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuhd_rec.hhcu_hash) 
	{
		if (envDbMcurr)
			amt = no_dec (cuhd_rec.loc_amt_paid - cuhd_rec.exch_variance);
		else
			amt = no_dec (cuhd_rec.loc_amt_paid);

		ledgerBalance -= amt;

		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}
/*
 * Process Supplier transactions to obtain total balance.
 */
void
ProcessSupplier (void)
{
	double	amt = 0.00;

	suin_rec.hhsu_hash 		= sumr_rec.hhsu_hash;
	suin_rec.date_of_inv 	= 0L;
	strcpy (suin_rec.est, "  ");

	/*
	 * Process Invoices/ Credits/ Journals.
	 */
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash) 
	{
		if (suin_rec.approved [0] == 'N')
		{
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		if (envCrMcurr)
		{
			if (suin_rec.exch_rate > 0.00)
				amt = no_dec (suin_rec.amt / suin_rec.exch_rate);
			else
				amt = no_dec (suin_rec.amt);
		}
		else
			amt = no_dec (suin_rec.amt);

		ledgerBalance += amt;
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}

	/*
	 * Process Cheques/ Journals.
	 */
	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suhd_rec.cheq_no, "             ");
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == suhd_rec.hhsu_hash) 
	{
		amt = no_dec (suhd_rec.loc_amt_paid);
			  
		ledgerBalance -= amt;
		ledgerBalance -= suhd_rec.exch_variance;
		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
}

void
ProcessBatch (void)
{
	if (programType	==	BALANCE_AR)
	{
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 4");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 4"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 5");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 5"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 6");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 6"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
	}
	if (programType	==	BALANCE_AP)
	{
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 7");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 7"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 8");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 8"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
		strcpy (glbhRec.co_no, comm_rec.co_no);
		strcpy (glbhRec.jnl_type, " 9");
		strcpy (glbhRec.batch_no, "          "); 
		cc = find_rec (glbh, &glbhRec, GTEQ, "r");
		while (!cc && !strcmp (glbhRec.co_no, comm_rec.co_no) &&
					  !strcmp (glbhRec.jnl_type, " 9"))
	  	{
			if (glbhRec.stat_flag [0] != 'P')
				ReportNotPosted ();

			cc = find_rec (glbh, &glbhRec, NEXT, "r");
		}
	}
}

/*
 * Report on non posted batches.
 */
void
ReportNotPosted (void)
{
	int		i;
	static	int	notPostedCounter	=	0;
	char	bufferData [100];

	if (firstBatch == TRUE)
	{
		rv_pr ( ML ("WARNING : THESE BATCHES HAVE NOT BEEN POSTED"),10,13,1);
		tab_open ("NotPosted", listKeys, 14, 0, 5, TRUE);
		tab_add 
		(
			"NotPosted", 
			"#%-60.60s", 
			ML (" BATCH NO | USER NAME |   DATE   | TIME |MONTH")
		);
		for (i = 0; i < 30; i++)
			tab_add ("NotPosted", "%60.60s", " ");

		tab_display ("NotPosted", TRUE);
		firstBatch = FALSE;
	}
	sprintf 
	(
		dispStr, 
		"%10.10s|%-11.11s|%10.10s|%6.6s|  %02d", 
		glbhRec.batch_no,
		glbhRec.user,
		DateToString (glbhRec.glbh_date),
		glbhRec.glbh_time,
		glbhRec.mth
	);
	tab_get ("NotPosted", bufferData, EQUAL, notPostedCounter++);
	tab_update ("NotPosted", "%-60.60s", dispStr);
	redraw_line ("NotPosted", TRUE);
}

/*
 * If data not in database default values should be added from include.
 */
void	
CheckCreated (void)
{
	int		i;

	strcpy (glis_rec.co_no, comm_rec.co_no);
	strcpy (glis_rec.code,  "  ");
	strcpy (glis_rec.find_inf,  "          ");
	cc = find_rec (glis, &glis_rec, GTEQ, "r");
	if (!cc && !strcmp (glis_rec.co_no, comm_rec.co_no))
		return;

	for (i = 0;strlen (GL_integrity [i].moduleType);i++)
	{
		strcpy (glis_rec.co_no, comm_rec.co_no);
		sprintf (glis_rec.code, 	"%-10.10s", GL_integrity [i].moduleType);
		sprintf (glis_rec.find_inf, "%-10.10s", GL_integrity [i].ifCode);
		cc = find_rec (glis, &glis_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (glis_rec.desc, 	"%-40.40s", GL_integrity [i].ifDesc);
			sprintf (glis_rec.use_inf, 	"%-10.10s", GL_integrity [i].userCode);
			cc = abc_add (glis, &glis_rec);
			if (cc)
				file_err (cc, glis, "DBADD");
		}
		else
		{
			sprintf (glis_rec.desc, 	"%-40.40s", GL_integrity [i].ifDesc);
			sprintf (glis_rec.use_inf, 	"%-10.10s", GL_integrity [i].userCode);
			cc = abc_update (glis, &glis_rec);
			if (cc)
				file_err (cc, glis, "DBADD");
		}
	}
}

/*
 * Get value of inventory for category. 
 */
double
ProcessCategoryValue (
	char	*coNo,
	char	*categoryNo)
{
	double	categoryValue	=	0.00;

	/*
	 * Read item master file for category.
	 */
	sprintf (inmr_rec.co_no, 	"%-2.2s",	coNo);
	sprintf (inmr_rec.category, "%-11.11s",	categoryNo);
	cc = find_rec (inmr, &inmr_rec, GTEQ,"r");
	while (!cc && !strncmp (inmr_rec.co_no, coNo, 2) &&
				  !strncmp (inmr_rec.category, categoryNo, 11))
	{
		if (strcmp (inmr_rec.supercession,"                "))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, GTEQ, "r");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			categoryValue	+=	ProcessItemValue ();
			cc = find_rec (incc, &incc_rec, NEXT, "r");
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	return (categoryValue);
}

/*
 * Main donkey routine to get value for item.
 */
double	
ProcessItemValue (void)
{
	double	extend;
	double	value;
	float	inccQuantity;

	inccQuantity = incc_rec.closing_stock;
	
	if (inccQuantity <= 0.00)
	{
		if (envSkValNegative)
			strcpy (inmr_rec.costing_flag, "L");
		else
			return (0.00);
	}
	value	=	StockValue
				(
					inmr_rec.costing_flag,
					comm_rec.est_no,
					inmr_rec.hhbr_hash,
					incc_rec.hhwh_hash,
					inccQuantity,
					inmr_rec.dec_pt,
					TRUE
				);

	value	=	twodec (value);
	extend	=	value;
	extend	*=	(double) n_dec (inccQuantity, inmr_rec.dec_pt);
	extend	=	out_cost (extend, inmr_rec.outer_size);
	return (extend);
}
