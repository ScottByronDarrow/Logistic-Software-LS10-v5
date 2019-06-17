/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ir_glup.c,v 5.3 2001/08/09 09:18:48 scott Exp $
|  Program Name  : (sk_ir_glup.c)                               
|  Program Desc  : (Stock transfer / G/Ledger Audit/Update/print.) 
|---------------------------------------------------------------------|
|  Date Written  : 29/01/91        |  Author     : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: sk_ir_glup.c,v $
| Revision 5.3  2001/08/09 09:18:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/09 01:47:17  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:10  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ir_glup.c,v $",
		*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_glup/sk_ir_glup.c,v 5.3 2001/08/09 09:18:48 scott Exp $";

#define		NO_SCRGEN
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define	ISSUE		 (itgl_rec.type [0] == 'I')

#define	IC_TRANS	 (itgl_rec.ic_trans [0] == 'Y')

#define	DUTY		 (itgl_rec.jnl_type [0] == '4')
#define	FREIGHT		 (itgl_rec.jnl_type [0] == '5')

#define	DEBIT		 (itgl_rec.jnl_type [0] == '1' || \
					  itgl_rec.jnl_type [0] == '3')

#define	CREDIT		 (itgl_rec.jnl_type [0] == '2' || \
					  itgl_rec.jnl_type [0] == '4')

#define	LCL_ERROR	TRUE
#define	LCL_VALID	FALSE
#define	MAX_ACCOUNTS	500
#define	RECEIPT		 (rec_cost [0] == 'R')

#define	BY_COMPANY	 (byWhat [0] == 'C')
#define	BY_BRANCH	 (byWhat [0] == 'B')


#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct itglRecord	itgl_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;

/*
 * Table Names
 */
static char
	*data	= "data";

/*
 * Globals
 */
	int		printerNo = 1,
			openAudit = FALSE;

	FILE	*ftmp;

	static	char	locCurrency 	[4],
					debitFriAccNo 	[sizeof glmrRec.acc_no],
					creditFriAccNo 	[sizeof glmrRec.acc_no],
					transferAccNo 	[sizeof glmrRec.acc_no],
					debitAccountNo 	[sizeof glmrRec.acc_no],
					creditAccountNo [sizeof glmrRec.acc_no],
					dutyAccNo 		[sizeof glmrRec.acc_no],
					byWhat 			[2];

	static	long	debitFriHhmrHash 	= 0L,
					creditFriHhmrHash 	= 0L,
					dutyHhmrHash 		= 0L,
					transferHhmrHash 	= 0L,
					debitHhmrHash 		= 0L,
					creditHhmrHash 		= 0L;

/*
 * Function Declarations
 */
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Process 			(void);
void 	AddGlwk 			(char *, char *, char *, long, char *);
void 	OpenError 			(void);
void 	PrintError 			(char *);
void 	CloseError 			(void);
void  	GetControlAccounts 	(void);
void  	GetFreightAccounts 	(void);
void  	GetAccounts 		(char *);


/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 3)
	{
		print_at (0,0, mlSkMess196 ,argv [0]);
		return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);
	sprintf (byWhat, "%-1.1s", argv [2]);

	if (!BY_COMPANY && !BY_BRANCH)
	{
		print_at (0,0, mlSkMess197 ,argv [0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	init_scr ();

	set_tty ();

	dsp_screen ("Processing Stock transfers to General Ledger.",
					comm_rec.co_no,comm_rec.co_name);

	Process ();

	CloseDB (); 
	FinishProgram ();
	if (openAudit)
		CloseError ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files. 
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (itgl, itgl_list, ITGL_NO_FIELDS, "itgl_id_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (locCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (locCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close data base files. 
 */
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (itgl);
	GL_CloseBatch (printerNo);
	GL_Close ();
	abc_dbclose (data);
}

/*
 * Get transfer control Accounts. 
 */
void
GetControlAccounts (void)
{
	GL_GLI 
	(
		itgl_rec.i_co_no,
	 	itgl_rec.i_br_no,
		"  ",
		(IC_TRANS) ? "TRANS-CO  " : "TRANS-BR  ",
		"   ",
		inmr_rec.category
	);
	transferHhmrHash = glmrRec.hhmr_hash;
	strcpy (transferAccNo, glmrRec.acc_no);
		
	GL_GLI 
	(
		itgl_rec.i_co_no,
		itgl_rec.i_br_no,
		"  ",
		"DUTY PAY  ",
		"   ",
		inmr_rec.category
	);
	if (!ISSUE)
	{
		dutyHhmrHash = glmrRec.hhmr_hash;
		strcpy (dutyAccNo, glmrRec.acc_no);
	}
}

/*
 * Get freight control Accounts.
 */
void
GetFreightAccounts (void)
{
	GL_GLI 
	(
		itgl_rec.i_co_no,
		itgl_rec.i_br_no,
		"  ",
		"FREIGHT   ",
		"   ",
		inmr_rec.category
	);
	creditFriHhmrHash = glmrRec.hhmr_hash;
	strcpy (creditFriAccNo, glmrRec.acc_no);

	GL_GLI 
	(
		itgl_rec.r_co_no,
		itgl_rec.r_br_no,
		"  ",
		"FREIGHT   ",
		"   ",
		inmr_rec.category
	);
	debitFriHhmrHash = glmrRec.hhmr_hash;
	strcpy (creditFriAccNo, glmrRec.acc_no);
}

/*
 * Process all itgl records for current company. 
 */
void
Process (
 void)
{
	strcpy (itgl_rec.co_no,comm_rec.co_no);
	strcpy (itgl_rec.br_no, (BY_COMPANY) ? "  " : comm_rec.est_no);
	strcpy (itgl_rec.sort,"      ");
	cc = find_rec (itgl, &itgl_rec, GTEQ, "u");

	while (!cc && !strcmp (itgl_rec.co_no,comm_rec.co_no) && 
		 (!strcmp (itgl_rec.br_no,comm_rec.est_no) || BY_COMPANY))
	{
		/*
		 * Record is not valid.
		 */
		if (itgl_rec.stat_flag [0] != '0')
		{
			abc_unlock (itgl);
			cc = find_rec (itgl, &itgl_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Transfer #",itgl_rec.sort);

		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", itgl_rec.hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");

		cc = find_hash (ccmr, &ccmr_rec, COMPARISON, "r", itgl_rec.hhcc_hash);
		if (cc)
		    	file_err (cc, ccmr, "DBFIND");

		/*-----------------------------------
		| Glat control record is not valid. |
		-----------------------------------*/
		GetControlAccounts ();

		if (FREIGHT)
		{
			GetFreightAccounts ();

			AddGlwk 
			(
				itgl_rec.i_co_no, 
				itgl_rec.i_br_no,
				debitFriAccNo, 
				debitFriHhmrHash, 
				"1"
			);
			AddGlwk 
			(
				itgl_rec.r_co_no, 
				itgl_rec.r_br_no,
				creditFriAccNo, 
				creditFriHhmrHash, 
				"2"
			);
		}
		else
		{
			/*----------------
			| GLI Interface. |
			----------------*/
			GetAccounts (inmr_rec.category);
			if (DEBIT)
			{
				AddGlwk 
				(
					itgl_rec.co_no, 
					itgl_rec.br_no,
					debitAccountNo, 
					debitHhmrHash, 
					"1"
				);
			}
			else
			{
				if (DUTY)
				{
					AddGlwk 
					(
						itgl_rec.co_no, 
						itgl_rec.br_no,
						dutyAccNo, 
						dutyHhmrHash, 
						"2"
					);
				}
				else
				{
					AddGlwk 
					(
						itgl_rec.co_no, 
						itgl_rec.br_no,
						creditAccountNo, 
						creditHhmrHash, 
						"2"
					);
				}
			}
		}
		strcpy (itgl_rec.stat_flag, "D");
		cc = abc_update (itgl, &itgl_rec);
		if (cc)
				file_err (cc, itgl, "DBUPDATE");

		abc_unlock (itgl);

		cc = find_rec (itgl, &itgl_rec, NEXT, "u");
	}

	abc_unlock (itgl);

	strcpy (itgl_rec.co_no,comm_rec.co_no);
	strcpy (itgl_rec.br_no, (BY_COMPANY) ? "  " : comm_rec.est_no);
	strcpy (itgl_rec.sort,"      ");
	cc = find_rec (itgl, &itgl_rec, GTEQ, "u");

	while (!cc && !strcmp (itgl_rec.co_no,comm_rec.co_no) && 
	(!strcmp (itgl_rec.br_no,comm_rec.est_no) || BY_COMPANY))
	{
		if (itgl_rec.stat_flag [0] == 'D')
		{
			cc = abc_delete (itgl);
			if (cc)
				file_err (cc, itgl, "DBDELETE");

			strcpy (itgl_rec.co_no,comm_rec.co_no);
			strcpy (itgl_rec.br_no,"  ");
			strcpy (itgl_rec.sort,"      ");
			cc = find_rec (itgl, &itgl_rec, GTEQ, "u");
			continue;
		}
		else
			abc_unlock ("itgl");

		cc = find_rec (itgl, &itgl_rec, NEXT, "u");
	}
	abc_unlock ("itgl");
}

/*===========================
| Process control Accounts. |
===========================*/
void
GetAccounts (
 char	*cat_no)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		"   ",
		cat_no
	);
	if (ISSUE)
	{
		strcpy (debitAccountNo,transferAccNo);
		strcpy (creditAccountNo, glmrRec.acc_no);
		debitHhmrHash 	= transferHhmrHash;
		creditHhmrHash 	= glmrRec.hhmr_hash;
	}
	else
	{
		strcpy (creditAccountNo,transferAccNo);
		strcpy (debitAccountNo, glmrRec.acc_no);
		debitHhmrHash = glmrRec.hhmr_hash;
		creditHhmrHash = transferHhmrHash;
	}
}
/*===============================================================
| Add general Ledger Transactions to Purchase order worrk file. |
===============================================================*/
void
AddGlwk (
	char	*coNo, 
	char	*brNo, 
	char	*accountNo, 
	long	hhmrHash, 
	char	*journalType)
{
	/*--------------------------------------------
	| Add transaction for account if required  . |
	--------------------------------------------*/
	sprintf	(glwkRec.co_no, "%-2.2s", coNo);
	sprintf (glwkRec.est_no,"%-2.2s", brNo);
	sprintf (glwkRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,accountNo);
	strcpy 	(glwkRec.tran_type,itgl_rec.tran_type);
	sprintf (glwkRec.sys_ref,"%5.1d", comm_rec.term);
	strcpy 	(glwkRec.period_no,itgl_rec.period_no);
	sprintf (glwkRec.name,"%30.30s", " ");
	strcpy 	(glwkRec.narrative,itgl_rec.narr);
	strcpy 	(glwkRec.alt_desc1, " ");
	strcpy 	(glwkRec.alt_desc2, " ");
	strcpy 	(glwkRec.alt_desc3, " ");
	strcpy 	(glwkRec.batch_no, " ");
	sprintf (glwkRec.acronym,"%9.9s"," ");
	strcpy 	(glwkRec.user_ref, itgl_rec.user_ref);
	sprintf (glwkRec.chq_inv_no, "%15.15s", " ");
	strcpy 	(glwkRec.stat_flag,"2");
	strcpy 	(glwkRec.currency, locCurrency);
	strcpy 	(glwkRec.run_no,"      ");
	strcpy	(glwkRec.jnl_type, journalType);
	glwkRec.hhgl_hash 	= hhmrHash;
	glwkRec.tran_date 	= itgl_rec.tran_date;
	glwkRec.ci_amt 		= 0.00;
	glwkRec.o1_amt 		= 0.00;
	glwkRec.o2_amt 		= 0.00;
	glwkRec.o3_amt 		= 0.00;
	glwkRec.o4_amt 		= 0.00;
	glwkRec.post_date 	= itgl_rec.post_date;
	glwkRec.amount 		= itgl_rec.amount;
	glwkRec.loc_amount 	= itgl_rec.amount;
	glwkRec.exch_rate 	= 1.00;
		
	GL_AddBatch ();
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
OpenError (void)
{
	/*==================================
	| Open pipe work file to Pformat.  |
 	==================================*/
	if ( (ftmp = popen ("pformat","w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);
	}

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.gl_date), PNAME);
	fprintf (ftmp,".LP%d\n",printerNo);

	/*-------------------------------
	| Print Selected Accounts Type. |
	-------------------------------*/
	fprintf (ftmp,".10\n");
	fprintf (ftmp,".L140\n");
	fprintf (ftmp,".E%s\n",comm_rec.co_name);
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".E AS AT :%s\n",SystemTime ());
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ESTOCK TRANSFERS TO G/L ERROR REPORT\n");

	fprintf (ftmp,".R==================================================================================================================================\n");
	fprintf (ftmp,"==================================================================================================================================\n");
	fprintf (ftmp,"|                                          D E S C R I P T I O N    O F    E R R O R                                             |\n");
	fprintf (ftmp,"|--------------------------------------------------------------------------------------------------------------------------------|\n");
}

void
PrintError (
 char *errorMessage)
{
	if (!openAudit)
		OpenError ();

	openAudit = TRUE;

	fprintf (ftmp,"|%-128.128s|\n", errorMessage);
}

void
CloseError (
 void)
{
	fprintf (ftmp,".EOF\n");
	pclose (ftmp);
}
