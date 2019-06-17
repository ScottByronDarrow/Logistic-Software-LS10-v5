/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_dpurge.c,v 5.0 2002/05/08 01:29:27 scott Exp $
|  Program Name  : ( db_dpurge.c    )                                 |
|  Program Desc  : ( Prints and Flags Customer Invoices/ Cheques.  )  |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: db_dpurge.c,v $
| Revision 5.0  2002/05/08 01:29:27  scott
| CVS administration
|
| Revision 1.1  2001/08/14 03:03:48  scott
| Updated for new delete wizard
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_dpurge.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_dpurge/db_dpurge.c,v 5.0 2002/05/08 01:29:27 scott Exp $";

#include	<stdarg.h>

#include	<pslscr.h>
#include	<arralloc.h>
#include	<twodec.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<dbgroup.h>
#include    <errno.h>
#include    <DeleteControl.h>

/*-----------------------------------------------------------------------
| The structures 'cheq'&'dtls' are initialised in function 'GetCheques'	|
| the number of cheques is stored in external variable 'cheq_cnt'.    	|
| the number of details is stored in external variable 'dtls_cnt'.    	|
-----------------------------------------------------------------------*/
struct Cheque
{						/*---------------------------------------*/
	Money	amount;		/*| total amount of cheque.	    		|*/
	long	hhcpHash;	/*| Link from cheque to cheque details. |*/
}	*cheq;				/*|-------------------------------------|*/
	DArray	cheq_d;		/*| dynamic info for cheq 				|*/
	int		cheq_cnt;	/*| Number of cheque details.			|*/
						/*---------------------------------------*/
struct Detail
{							/*-----------------------------------*/
	long	hhciHash;		/*| detail invoice reference.       |*/
	double	inv_amt;		/*| detail invoice amount.          |*/
	int 	cheq_offset;	/*| cheq structure pointer.         |*/
	Date	date_payment;	/*| date of payment                 |*/
}	*dtls;					/*|									|*/
	DArray	dtls_d;			/*| dynamic info for dtls 			|*/
	int		dtls_cnt; 		/*-----------------------------------*/

	FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct cudtRecord	cudt_rec;
struct cuhdRecord	cuhd_rec;
struct cuinRecord	cuin_rec;
struct cuinRecord	cuin2_rec;
struct cumrRecord	cumr_rec;

static char
		*data	= "data",
		*cudt2	= "cudt2",
		*cudt3	= "cudt3",
		*cuhd2	= "cuhd2",
		*cuin2	= "cuin2",
		*cumr2	= "cumr2";


int		envDbNettUsed 	= TRUE;
long	envPurgeMon 	= 0L;
int		envDbGrpPurge 	= 1;

#define	CHEQUE		0
#define	JNL_ONE		1
#define	INVOICE		2
#define	CREDIT		3
#define	JNL_TWO		4
#define	B_FWD		5

struct
{
	int		_type;		/* transaction type			*/
	char	*_desc;		/* long description			*/
	char	*_short;	/* short description			*/
	int		_date;		/* print date				*/
	double	_conv;		/* convertion factor			*/
} trans_type [] =
{
	{ CHEQUE,	"RECEIPT",	"CHQ",	TRUE,	1.00	},
	{ JNL_ONE,	"JOURNAL",	"JNL",	TRUE,	1.00	},
	{ INVOICE,	"INVOICE",	"INV",	TRUE,	-1.00	},
	{ CREDIT,	"CREDIT",	"CRD",	TRUE,	-1.00	},
	{ JNL_TWO,	"JOURNAL",	"JNL",	TRUE,	-1.00	},
	{ B_FWD,	"B/F BAL",	"B/F",	TRUE,	-1.00	},
	{ -1,		"** ERROR **",	"***",	FALSE,	-1.00	},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static 	FILE *OpenLog 		(char *);
static 	int BFPurge 		(Date, struct cumrRecord *, FILE *);
static 	int GetCheques 		(Date, long, FILE *);
static 	int GetTransType 	(int);
static 	int ValidCustomer 	(int, const char *, struct cumrRecord *);
static 	void BFProcInvoice 	(struct cumrRecord *, const char *, struct cuinRecord *);
static 	void CloseLog 		(FILE *, int, const char *);
static 	void CustomerHeader (struct cumrRecord *);
static 	void Invoke 		(const char *);
static 	void LclWriteLog 	(FILE *, const char *, ...);
static 	void MailSysadmin 	(const char *);
static 	void ProcessInvoices (struct cumrRecord *);
static 	void ProcessList 	(DGroupSet *, const char *, struct cumrRecord *);
static 	void ProcessPurge 	(DGroup *, int, const char *, struct cumrRecord *);
static 	void PurgeFunc 		(const char *, struct cuinRecord *);
static 	void UpdateCuhd 	(long, const char *);
static 	void UpdateCuin 	(long, const char *);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	PrintUpdate 		(DGroupItem *, double);
void 	StartProcessing		(int, int);

/*-----------------------------------------------
| Retrieve a subscript into a structure-array   |
| based upon the passed transaction type.   |
-----------------------------------------------*/
static int
GetTransType (
 int                tr_type)
{
    int i;
    /*---------------------------------------
    | find entry for type           |
    ---------------------------------------*/
    for (i = 0; trans_type [i]._type != -1; i++)
        if (trans_type [i]._type == tr_type)
            return (i);
    /*---------------------------------------
    | erroneous type. (No, not erogenous!)  |
    ---------------------------------------*/
    return (i);
}

int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	char	*paid_flag = "Z";			/* Default mark flag */
	Date	monthEndDate;
	char	envCoClose [6],
			branchNo [3];
	int		envDbCo,
			envMendLp,
			allBranches;

	FILE *	logFile;
	int		logErrCount;
	char	logFName [100];

	Invoke ("db_dpurge.sh");

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *) 0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	sptr = chk_env( "DB_NETT_USED" );
	envDbNettUsed = ( sptr == ( char *)0 ) ? TRUE : atoi( sptr );

	sptr = chk_env ("MEND_LP");
	envMendLp = (sptr == (char *)0) ? 1 : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envDbCo	= (sptr == (char *)0) ? 1 : atoi (sptr);

	sptr = chk_env ("PURGE_MON");
	envPurgeMon	= (sptr == (char *)0) ? 0L : atol (sptr);

	sptr = chk_env ("DB_GRP_PURGE");
	envDbGrpPurge = (sptr == (char *)0) ? 1 : atoi (sptr);

	/*
	 *	Allocate initial lines for cheques and details
	 */
	ArrAlloc (&cheq_d, &cheq, sizeof (struct Cheque), 1000);
	ArrAlloc (&dtls_d, &dtls, sizeof (struct Detail), 1000);

	/*
	 *	Create Error Log
	 */
	logFile = OpenLog (logFName);
	logErrCount = 0;

	OpenDB ();

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "CUSTOMER-LEDGER");
	if (!cc)
	{
		envPurgeMon		= (long) delhRec.purge_days;
		envDbGrpPurge 	= delhRec.spare_fg1;
	}

	monthEndDate = MonthEnd (comm_rec.dbt_date);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	if (envCoClose [0] == '1')
	{
		dsp_screen ("Purging Company Customers.",
					comm_rec.co_no,
					comm_rec.co_name);
		allBranches = TRUE;
	}
	else
	{
		sprintf (err_str, "Purging Customers for Branch %s - %s",
				comm_rec.est_no, clip (comm_rec.est_name));

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
		allBranches = FALSE;
	}
	/*-----------------------------
	| Initialise standard print . |
	-----------------------------*/
	StartProcessing (envMendLp, allBranches);

	if (OpenStatementGroups (comm_rec.dbt_date))
	{
		struct cumrRecord	cumr_rec;

		memset (&cumr_rec, 0, sizeof cumr_rec);
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, allBranches ? "  " : branchNo);

		for (cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			!cc && ValidCustomer (allBranches, branchNo, &cumr_rec);
			cc = find_rec (cumr, &cumr_rec, NEXT, "r"))
		{
			/*
			 *	Ignore child customers
			 */
			if (cumr_rec.ho_dbt_hash)
			{
				LclWriteLog (logFile, "Skipped child: %s\n", cumr_rec.dbt_no);
				continue;
			}

			dsp_process ("Customer : ", cumr_rec.dbt_no);

			LclWriteLog (logFile, "%s : %s\n",
				SystemTime(), cumr_rec.dbt_no);

			/*-----------------------------------------------------------
			| Load statement groups.									|
			| if it is a B/F statement or group purge turned off then	|
			| do it the old new way otherwise do it the new.			|
			-----------------------------------------------------------*/
			if (cumr_rec.stmt_type [0] == 'O' && envDbGrpPurge)
			{
				DGroupSet	GroupSet;

				if 
				(
					LoadStatementGroups 
					(
						&GroupSet,
						cumr_rec.hhcu_hash,
						0L,
						DG_PURGE,
						logFile
					)
				)
				{
					ProcessList 
					(
						&GroupSet, 
						paid_flag, 
						&cumr_rec
					);
				}
				else
					logErrCount++;

				FreeStatementGroups (&GroupSet);
			}
			else
			{
				if (!BFPurge (monthEndDate, &cumr_rec, logFile))
					logErrCount++;
			}

			LclWriteLog (logFile, " ... done\n");
		}
		CloseStatementGroups();
	}
	fprintf (fout, ".EOF\n");
	pclose (fout);
	CloseDB (); 
	CloseLog (logFile, logErrCount, logFName);

	if (logErrCount)
		MailSysadmin (logFName);

	ArrDelete (&dtls_d);
	ArrDelete (&cheq_d);

	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cuin2, cuin);
	abc_alias (cudt2, cudt);
	abc_alias (cudt3, cudt);
	abc_alias (cuhd2, cuhd);
	abc_alias (cumr2, cumr);

	open_rec (cudt,  cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cudt2, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cudt3, cudt_list, CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (cuhd2, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec (cuin2, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");

}

void
CloseDB (void)
{
	abc_fclose (cudt);
	abc_fclose (cudt2);
	abc_fclose (cudt3);
	abc_fclose (cuhd);
	abc_fclose (cuhd2);
	abc_fclose (cuin);
	abc_fclose (cuin2);
	abc_fclose (cumr);
	abc_fclose (cumr2);

	abc_dbclose (data);
}

static int
ValidCustomer (
	int		allBranches,
	const char *branchNo,
	struct cumrRecord *	cumrRec)
{
	if (!allBranches &&
		!strcmp (cumrRec -> co_no, comm_rec.co_no) &&
		!strcmp (cumrRec -> est_no, branchNo))
	{
		return (TRUE);
	}
	return (allBranches && !strcmp (cumrRec -> co_no, comm_rec.co_no));
}

static void
CustomerHeader (
	struct cumrRecord *cumrRec)
{
	char	customerName [64];
	static 	long	prevHhcuHash = 0;

	if (prevHhcuHash == cumrRec -> hhcu_hash)
		return;

	prevHhcuHash = cumrRec -> hhcu_hash;

	fprintf (fout, ".LRP3\n");

	fprintf (fout, "|----------");
	fprintf (fout, "+----------");
	fprintf (fout, "+---------");
	fprintf (fout, "+----------");
	fprintf (fout, "+------------");
	fprintf (fout, "+------------|\n");

	sprintf (customerName, "(%s)", clip (cumrRec -> dbt_name));
	fprintf (fout, "| %-6.6s %c %-42.42s                |\n",
		cumrRec -> dbt_no,
		cumrRec -> ho_dbt_hash ? '*' : ' ',
		customerName);
}

void
StartProcessing (
	int		printerNumber,
	int		allBranches)
{
	if (!(fout = popen ("pformat", "w")))
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".12\n");
	fprintf (fout,".PI12\n");
	fprintf (fout, ".L70\n");
	fprintf (fout, ".CCUSTOMER DELETED TRANSACTIONS\n");
	if (!allBranches)
	{
		fprintf (fout, ".B1\n");
		fprintf (fout, ".C%s%s\n", "FOR ", clip (comm_rec.est_short));
	}
	else
	{
		fprintf (fout, ".B1\n");
		fprintf (fout, ".B1\n");
	}
	fprintf (fout, ".B1\n");
	fprintf (fout, ".C%s AS AT%s\n", comm_rec.co_short, SystemTime());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R==================================");
	fprintf (fout, "====================================\n");

	fprintf (fout, "====================================");
	fprintf (fout, "==================================\n");

	fprintf (fout, "| DATE OF  |   DATE   |  TRANS. | ");
	fprintf (fout, " REFER.  |    TRANS.  |     GROUP  |\n");

	fprintf (fout, "|  TRANS.  |  POSTED  |   TYPE  | ");
	fprintf (fout, "         |    AMOUNT  |     TOTAL  |\n");
}

static void
ProcessList (
	DGroupSet 	*pGroupSet,
	const char 	*paid_flag,
	struct cumrRecord *cumrRec)
{
	DGroup	*pGroup;
	double	grp_value;
	int		trn_cnt;

	for (pGroup = GetFirstGroup (pGroupSet);
		 pGroup != NULL;
		 pGroup = GetNextGroup (pGroup))
	{
		grp_value = DOLLARS (GetGroupValue (pGroup));
		trn_cnt = GetGroupCount (pGroup);

		if (!twodec (grp_value))
		{
			Date	lastTran = GetLastTransactionDate (pGroup);

			if (lastTran >= comm_rec.dbt_date)
			{
				/*
				 *	Group has forward dated transaction,
				 *	we shouldn't do anything with it.
				 */
			}
			else if (lastTran + envPurgeMon >= comm_rec.dbt_date)
			{
				/*
				 *	Group has transactions falling within keepalive period
				 *	Update with transitional flag
				 */
				ProcessPurge (pGroup, trn_cnt, paid_flag, cumrRec);
			}
			else
			{
				/*
				 *	The Group's transactions all fall outside the keepalive
				 *	Update to delete
				 */
				ProcessPurge (pGroup, trn_cnt, "9", cumrRec);
			}
		}
	}
}

static void
ProcessPurge (
	DGroup 	*pGroup,
	int		trn_cnt,
	const char *stat_flag,
	struct cumrRecord *	cumrRec)
{
	DGroupItem	*pItem;
	double		run_total = 0.00;

	if (*stat_flag == '9')
	{
		CustomerHeader (cumrRec);

		fprintf (fout, ".LRP%d\n", trn_cnt + 1);
		fprintf (fout, "|----------+----------+---------+----------+------------+------------|\n");
	}

	for (pItem = GetFirstGroupItem (pGroup);
		 pItem != NULL;
		 pItem = GetNextGroupItem (pItem))
	{
		if (pItem -> source == DG_cuhd)
			UpdateCuhd (pItem -> hhcp_hash, stat_flag);
		else
			UpdateCuin (pItem -> hhci_hash, stat_flag);

		if (*stat_flag == '9')
		{
			run_total += DOLLARS (pItem -> value);
			PrintUpdate (pItem, run_total);
		}
	}
}

void
PrintUpdate (
	DGroupItem	*pItem,
	double		run_total)
{
	char	transDate [11],
			postDate [11];

	if (pItem -> source == DG_cuhd)		/* Is this a cuhd record??	*/
	{
		strcpy (transDate, DateToString (cuhd_rec.date_payment));
		strcpy (postDate, DateToString (cuhd_rec.date_posted));
		fprintf 
		(
			fout, "|%-10.10s|%-10.10s| %-7.7s | %-8.8s |%11.2f |%11.2f |\n",
			transDate,
			postDate,
			trans_type [pItem -> type]._desc,
			cuhd_rec.receipt_no,
			DOLLARS (pItem -> value),
			run_total
		);
	}
	else
	{
		strcpy (transDate, DateToString (cuin2_rec.date_of_inv));
		strcpy (postDate, DateToString (cuin2_rec.date_posted));
		fprintf 
		(
			fout, "|%-10.10s|%-10.10s| %-7.7s | %-8.8s |%11.2f |%11.2f |\n",
			transDate,
			postDate,
			trans_type [pItem -> type]._desc,
			cuin2_rec.inv_no,
			DOLLARS (pItem -> value),
			run_total
		);
	}
}

static void
UpdateCuhd (
 long			hhcpHash,
 const char *	stat_flag)
{
	cuhd_rec.hhcp_hash	=	hhcpHash;
	if ((cc = find_rec (cuhd, &cuhd_rec, EQUAL, "u")))
		file_err (cc, "cuhd", "DBFIND");

	strcpy (cuhd_rec.stat_flag, stat_flag);

	if (*stat_flag == '9')
	{
		if ((cc = abc_delete (cuhd)))
			file_err (cc, "cuhd", "abc_delete");
	}
	else
	{
		if ((cc = abc_update (cuhd, &cuhd_rec)))
			file_err (cc, "cuhd", "abc_update");
	}

	cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
	cc = find_rec (cudt2, &cudt_rec, GTEQ, "u");
	while (!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash)
	{
		strcpy (cudt_rec.stat_flag, stat_flag);

		if (*stat_flag == '9')
		{
			if ((cc = abc_delete (cudt2)))
				file_err (cc, "cudt2", "abc_delete");

			cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
			cc = find_rec (cudt2, &cudt_rec, GTEQ, "u");
		}
		else
		{
			if ((cc = abc_update (cudt2, &cudt_rec)))
				file_err (cc, "cudt2", "abc_update");
			cc = find_rec (cudt2, &cudt_rec, NEXT, "u");
		}
	}
	abc_unlock (cudt2);
}

static void
UpdateCuin (
	long	hhciHash,
	const char *	stat_flag)
{
	cuin2_rec.hhci_hash	=	hhciHash;
	cc = find_rec (cuin2, &cuin2_rec, EQUAL, "u");
	if (cc)
		file_err (cc, "cuin2", "DBFIND");

	strcpy (cuin2_rec.stat_flag, stat_flag);

	if (*stat_flag == '9')
	{
		if ((cc = abc_delete (cuin2)))
			file_err (cc, "cuin2", "abc_delete");
	}
	else
	{
		if ((cc = abc_update (cuin2, &cuin2_rec)))
			file_err (cc, "cuin2", "abc_update");
	}
}

/*===================================================
| This is for B_FWD customers and no group purging. |
===================================================*/
static int
BFPurge (
	Date	monthEndDate,
	struct cumrRecord *	cumrRec,
 FILE *						errLog)
{
	struct cumrRecord	cumr2_rec;

	cheq_cnt = 0;
	dtls_cnt = 0;

	/*
	 *	Load current cheques, as well as child customers'
	 */
	if (!GetCheques (monthEndDate, cumrRec -> hhcu_hash, errLog))
		return FALSE;

	for (cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumrRec -> hhcu_hash);
		!cc && cumr2_rec.ho_dbt_hash == cumrRec -> hhcu_hash;
		cc = find_hash (cumr2, &cumr2_rec, NEXT, "r", cumrRec -> hhcu_hash))
	{
		if (!GetCheques (monthEndDate, cumr2_rec.hhcu_hash, errLog))
			return FALSE;
	}

	/*
	 *	Work thru' current customer's, followed by children
	 */
	ProcessInvoices (cumrRec);
	for (cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumrRec -> hhcu_hash);
		!cc && cumr2_rec.ho_dbt_hash == cumrRec -> hhcu_hash;
		cc = find_hash (cumr2, &cumr2_rec, NEXT, "r", cumrRec -> hhcu_hash))
	{
		ProcessInvoices (&cumr2_rec);
	}

	return TRUE;
}

/*=====================================================
| Work thru' all cuin records for the given customer. |
=====================================================*/
static void
ProcessInvoices (
 struct cumrRecord *	cumrRec)
{
	struct cuinRecord	cuin_rec;

	memset (&cuin_rec, 0, sizeof cuin_rec);
	cuin_rec.hhcu_hash = cumrRec -> hhcu_hash;
	for (cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
		!cc && cuin_rec.hhcu_hash == cumrRec -> hhcu_hash;
		cc = find_rec (cuin, &cuin_rec, NEXT, "u"))
	{
		if ((comm_rec.dbt_date - envPurgeMon) >= cuin_rec.date_of_inv)
			BFProcInvoice (cumrRec, cuin, &cuin_rec);

		abc_unlock (cuin);
	}
	abc_unlock (cuin);
}
/*=========================================================
| Processing routine for balance B/F and non group purge. |
=========================================================*/
static void
BFProcInvoice (
	struct	cumrRecord	*cumrRec,
	const	char	*cuinTable,
	struct	cuinRecord	*cuinRec)
{
	int 	i;
	double	balance = 0.00;
	Date	purgeDate	=	0L;

	purgeDate	=	MonthStart (comm_rec.dbt_date);
	purgeDate	-=	envPurgeMon;

	balance = twodec (cuinRec -> amt - cuinRec -> disc);

	for (i = 0; i < dtls_cnt; i++)
	{
		if (dtls [i].hhciHash == cuinRec -> hhci_hash)
		{
			if (dtls [i].date_payment >= purgeDate)
				return;	/* payment is within envPurgeMon days */
			balance -= dtls [i].inv_amt;
		}
	}
	/*-------------------------
	| INVOICE CAN BE PURGED	. |
	-------------------------*/
	if (twodec (balance) == 0.00)
	{
		CustomerHeader (cumrRec);
		PurgeFunc (cuinTable, cuinRec);
	}
}

/*===================================================================
| Processing to purge invoices for balance B/F and non group purge. |
===================================================================*/
static void
PurgeFunc (
	const char	*cuinTable,
	struct 	cuinRecord	*cuinRec)
{
	char	transDate [11],
			postDate [11];

	int		i, 
			tr_type;
	Money	balance = 0.00;

	strcpy (cuinRec -> stat_flag, "9");

	if ((cc = abc_delete (cuinTable)))
		file_err (cc, "cuinTable", "abc_delete");

	/*-----------------------------------
	| convert type to statement type	|
	-----------------------------------*/
	tr_type = atoi (cuinRec -> type) + 1;
	tr_type = GetTransType (tr_type);
	balance = (envDbNettUsed) 	?	DOLLARS (cuinRec -> amt - cuinRec -> disc) 
								:	DOLLARS (cuinRec -> amt);

	strcpy (transDate, 	DateToString (cuinRec -> date_of_inv));
	strcpy (postDate, 	DateToString (cuinRec -> date_posted));

	fprintf 
	(
		fout, "|%-10.10s|%-10.10s| %-7.7s | %-8.8s |%11.2f |       N/A  |\n",
		transDate, 
		postDate,
		trans_type [tr_type]._desc,
		cuinRec -> inv_no, 
		twodec (balance * trans_type [tr_type]._conv)
	);

	for (i = 0; i < dtls_cnt; i++)
	{
		if (dtls [i].hhciHash == cuinRec -> hhci_hash)
		{
			Money	loc_balance		= 0,
					vrfy_balance 	= 0,
					reduced_amt 	= 0;
			int		all_nine;

			/*
			 * Verify existence of associated cuhd record
			 */
			if ((cc = find_hash (cuhd, &cuhd_rec, COMPARISON, "u",
					cheq [dtls [i].cheq_offset].hhcpHash)))
			{
				file_err (cc, "cuhd", "find_rec");
			}

			/*
 			 * Find & Flag cudt record with matching amount
			 */
			cudt_rec.hhci_hash = dtls [i].hhciHash;
			cudt_rec.hhcp_hash = cuhd_rec.hhcp_hash;
			for (cc = find_rec (cudt, &cudt_rec, GTEQ, "u");
				!cc && 
					cudt_rec.hhci_hash == dtls [i].hhciHash &&
					cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash;
				cc = find_rec (cudt, &cudt_rec, NEXT, "u"))
			{
				if (cudt_rec.stat_flag [0] != '9' &&
					!no_dec (cudt_rec.amt_paid_inv - dtls [i].inv_amt))
				{
					if ((cc = abc_delete (cudt)))
						file_err (cc, "cudt", "abc_delete");
					/*
					 *	Verification balance
					 */
					reduced_amt = cudt_rec.amt_paid_inv;
					break;
				}
				else
					abc_unlock (cudt);
			}
			abc_unlock (cudt);

			/*---------------------------------
			| loop through all cudt's for this
			| hhcpHash, if all are stat 9 
			| then cuhd may be flagged as a 9
			---------------------------------*/
			balance		=	0.0;
			loc_balance = 	0.0;
			all_nine 	=	TRUE;

			for (cc = find_hash (cudt2, &cudt_rec, EQUAL, "r",
						cuhd_rec.hhcp_hash);
				!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash;
				cc = find_hash (cudt2, &cudt_rec, NEXT, "r",
						cuhd_rec.hhcp_hash))
			{
				if (cudt_rec.stat_flag [0] != '9')
				{
					all_nine = FALSE;
					balance     += cudt_rec.amt_paid_inv;
					loc_balance += cudt_rec.loc_paid_inv;
				}
				else
				{
					/*
					 *	If the item has been flagged as '9' (deleted),
					 *	it really needs to be removed from the database
					 *	as well
					 */
					abc_delete (cudt2);
				}
			}

			cuhd_rec.tot_amt_paid = balance;
			cuhd_rec.loc_amt_paid = loc_balance;

			if (all_nine)
			{
				if ((cc = abc_delete (cuhd)))
					file_err (cc, "cuhd", "abc_delete");
			}
			else
			{
				if ((cc = abc_update (cuhd, &cuhd_rec)))
					file_err (cc, "cuhd", "abc_update");
			}
			strcpy (transDate,	DateToString (cuhd_rec.date_payment));
			strcpy (postDate,	DateToString (cuhd_rec.date_posted));
			fprintf 
			(
				fout, 
				"|%-10.10s|%-10.10s| %-7.7s | %-8.8s |%11.2f |       N/A  |\n",
				transDate, postDate, 
				"CHEQUE",
				cuhd_rec.receipt_no,
				DOLLARS (dtls [i].inv_amt)
			);

			/*
			 *	Verification log
			 *
			 *	Check the difference the the reduced amount against
			 *	what's in the running total for the cheque header.
			 *	If they're different, show it in the report
			 */
			vrfy_balance = cheq [dtls [i].cheq_offset].amount - reduced_amt;
			if (no_dec (fabs (vrfy_balance - balance)))
			{
				fprintf 
				(
					fout, 
					"| ******** |CORRECTED | %7s | %-8.8s |%11.2f |%11.2f |\n",
					"",
					cuhd_rec.receipt_no,
					DOLLARS (vrfy_balance),
					DOLLARS (balance)
				);
			}

			/*
			 *	Update running total
			 */
			cheq [dtls [i].cheq_offset].amount = balance;
		}
	}
}

static int
GetCheques (
 Date               month_end,
 long               hhcu_hash,
 FILE*              errLog)
{
	for (cc = find_hash (cuhd2, &cuhd_rec, GTEQ, "r", hhcu_hash);
		!cc && cuhd_rec.hhcu_hash == hhcu_hash;
		cc = find_hash (cuhd2, &cuhd_rec, NEXT, "r", hhcu_hash))
	{
		int	really_exists = FALSE;

		if (!ArrChkLimit (&cheq_d, cheq, cheq_cnt))
			sys_err ("ArrChkLimit(cheques)", ENOMEM, PNAME);

		for (cc = find_hash (cudt2, &cudt_rec, GTEQ, "r", cuhd_rec.hhcp_hash);
			!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash;
		 	cc = find_hash (cudt2, &cudt_rec, NEXT, "r", cuhd_rec.hhcp_hash))
		{
			if (cudt_rec.stat_flag [0] == '9')
			{
				/*
				 *	Ignore deleted payment records
				 */
				LclWriteLog (errLog,
					"cudt_stat_flag = '9'. "
					"Can't decide correctness for cudt_hhcp_hash=%ld",
					cudt_rec.hhcp_hash);
				return FALSE;
			}

			really_exists = TRUE;

			if (!ArrChkLimit (&dtls_d, dtls, dtls_cnt))
				sys_err ("ArrChkLimit(dtls)", ENOMEM, PNAME);

		 	dtls [dtls_cnt].hhciHash    = cudt_rec.hhci_hash;
		 	dtls [dtls_cnt].inv_amt      = twodec (cudt_rec.amt_paid_inv);
		 	dtls [dtls_cnt].cheq_offset  = cheq_cnt;
			dtls [dtls_cnt].date_payment = cuhd_rec.date_payment;
		 	++dtls_cnt;
		}

		if (really_exists)
		{
			/*
			 *	This checks to see whether we've got any bogus
			 *	cuhd (ie those without existing cudt) records hanging
			 *	around. 
			 */
			cheq [cheq_cnt].amount    = twodec (cuhd_rec.tot_amt_paid);
			cheq [cheq_cnt].hhcpHash = cuhd_rec.hhcp_hash;

			++cheq_cnt;
		}
	}

	return TRUE;
}

/*
 *
 */
static void
Invoke (
 const char *	prog)
{
	int	childState;

	switch (fork ())
	{
	case -1:
		break;

	case 0:
		execlp (prog, prog, NULL);
        break;
	default:
		wait (&childState);
	}
}

/*
 *	LogFile stuff
 */
static FILE *
OpenLog (
 char *	fname)
{
	/*
	 *	Build the file name using the date and time. It's somewhat unique.
	 */
	time_t		now = time (NULL);
	struct tm	*loc = localtime (&now);

	sprintf (fname, "%s/BIN/LOG/db_dpurge-%d%02d%02d-%02d:%02d.log",
		getenv ("PROG_PATH"),
		loc -> tm_year + 1900,
		loc -> tm_mon + 1,
		loc -> tm_mday,
		loc -> tm_hour,
		loc -> tm_min);

	return (fopen (fname, "w"));
}

static void
LclWriteLog (
 FILE *			logF,
 const char *	mask,
 ...)
{
	va_list	args;

	if (!logF)
		return;

	va_start (args, mask);
	vfprintf (logF, mask, args);
	va_end (args);

	fflush (logF);
}

static void
CloseLog (
 FILE *			logF,
 int			errCount,
 const char *	fName)
{
	if (!logF)
		return;

	fclose (logF);
	if (!errCount)
		unlink (fName);
}

/*
 *
 */
static void
MailSysadmin (
 const char *	errLog)
{
	FILE *	mailcmd = popen ("/usr/lib/sendmail -t", "w");

	if (!mailcmd)
		return;

	fprintf (mailcmd, "To: LS/10 Support\nSubject: db_dpurge error log\n\n");
	fprintf (mailcmd,
		"`db_dpurge' did not complete successfully. A copy of the\n"
		"error log can be found at %s",
		errLog);

	pclose (mailcmd);
}
