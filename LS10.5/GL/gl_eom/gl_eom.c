/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_eom.c,v 5.3 2001/08/09 09:13:42 scott Exp $
|=====================================================================|
|  Program Name  : (gl_eom.c)
|  Program Desc  : (General Ledger End Of Month Close)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author        : Scott Darrow.    |
|---------------------------------------------------------------------|
| $Log: gl_eom.c,v $
| Revision 5.3  2001/08/09 09:13:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:19  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:46  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_eom.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_eom/gl_eom.c,v 5.3 2001/08/09 09:13:42 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <GlUtils.h>

#include	"schema"

struct commRecord	comm_rec;

/*
 *   Constants, defines and stuff   
 */
	char	*data	= "data";

	/*
	 * Special fields and flags  ##################################.
	 */
   	int		processID,		/* Process id number for work files.     */
			glwkWorkFileNo,	/* File no. of glwk work file.		 */
			printerNumber = 1,
			PV_accruals = FALSE;

	long	postingDate		 =	0L;
	long	startProcessDate =	0L;

/*
 *   Local function prototypes  
 */
void 	ProcessAccruals 	(void);
void 	ReverseAccruals 	(char *, long);
void 	AddGeneralLedger 	(char *, long, double, double, double, char *);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	UpdateGljc 			(void);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char	*argv [])
{
	int		monthPeriod;

	if (argc > 1)
    {
        printerNumber = atoi (argv [1]);
		if (!printerNumber)
        {
            printerNumber = 1;
        }
    }

	processID = getpid (); /* QUERY */

	OpenDB (); 

	GL_SetAccWidth (comm_rec.co_no, TRUE);

	DateToDMY (comm_rec.gl_date, NULL, &monthPeriod, NULL);
	sprintf (err_str, "Closing General Ledger For Period %02d.", monthPeriod);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	ProcessAccruals ();

	UpdateGljc ();

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

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 

	OpenGljc ();
	OpenGlmr ();
	OpenGltr ();
	
	sprintf (filename, "%s/WORK/gl_work%05d", (sptr) ? sptr : "/usr/LS10.5", processID);
	cc = RF_OPEN (filename, sizeof (glwkRec), "w", &glwkWorkFileNo);
	if (cc) 
        file_err (cc, glwk, "WKOPEN");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();

	cc = RF_CLOSE (glwkWorkFileNo);
	if (cc) 
        file_err (cc, glwk, "WKCLOSE");

	if (PV_accruals)
	{
		sprintf (err_str, "gl_eom_update \" 3\" %d", processID);
		if (sys_exec (err_str))
            file_err (errno, "gl_eom_update", "sysexec");

		sprintf (err_str, "gl_wjnlprnt \" 3\" %d %d", printerNumber, processID);
		if (sys_exec (err_str))
            file_err (errno, "gl_wjnlprnt", "sysexec");
	}

	sprintf (err_str, "gl_wdelwk %d", processID);
	if (sys_exec (err_str))
        file_err (errno, "gl_wdelwk", "sysexec");
	
	abc_dbclose (data);
}

void
ProcessAccruals (
 void)
{
	postingDate			=	MonthEnd (comm_rec.gl_date) + 1L;
	startProcessDate	=	MonthStart (comm_rec.gl_date);

	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%*.*s", MAXLEVEL,MAXLEVEL," ");

	cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while (!cc && !strcmp (glmrRec.co_no, comm_rec.co_no))
	{
		if (glmrRec.glmr_class [2][0] == 'P')
		{
			ReverseAccruals (glmrRec.acc_no,
                             glmrRec.hhmr_hash);
		}
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
}

void
ReverseAccruals (
	char 	*acc_no,
	long 	hhmrHash)
{
	gltrRec.hhmr_hash = hhmrHash;
	strcpy (gltrRec.tran_type, " 3");
	gltrRec.tran_date = startProcessDate;
	sprintf (gltrRec.user_ref, "%-8.8s", " ");

    cc = find_rec (gltr, &gltrRec, GTEQ, "u");
	while (!cc && 
           (gltrRec.hhmr_hash == hhmrHash) && 
            !strcmp (gltrRec.tran_type, " 3") &&
           (gltrRec.tran_date < postingDate))
	{
		dsp_process ("Accruals : ", acc_no);

		/*
		 * If not already reversed.
		 */
		if (gltrRec.stat_flag [0] == '0')
		{
			strcpy (gltrRec.stat_flag, "1");

            cc = abc_update (gltr, &gltrRec);
			if (cc)
                file_err (cc, gltr, "DBUPDATE");

			gltrRec.amount 	 = -gltrRec.amount;
			gltrRec.amt_origin = -gltrRec.amt_origin;

			AddGeneralLedger 
			(
				acc_no, 
				hhmrHash, 
				gltrRec.amount, 
				gltrRec.amt_origin,
				gltrRec.exch_rate,
				gltrRec.currency
			);
			PV_accruals = TRUE;
		}
		else
            abc_unlock (gltr);

		cc = find_rec (gltr, &gltrRec, NEXT, "u");
	}
	abc_unlock (gltr);
}

/*
 * Set control totals to zero.
 */
void
UpdateGljc (
 void)
{
	int	j_type;

	strcpy (gljcRec.co_no, comm_rec.co_no);

	cc = find_rec (gljc, &gljcRec, GTEQ, "u");
	while (!cc && 
           !strcmp (gljcRec.co_no, comm_rec.co_no))
	{
		j_type = atoi (gljcRec.journ_type);
		if (j_type < 4) 
		{
			gljcRec.tot_1 = 0.00;
			gljcRec.tot_2 = 0.00;
			gljcRec.tot_3 = 0.00;
			gljcRec.tot_4 = 0.00;
			gljcRec.tot_5 = 0.00;
			gljcRec.tot_6 = 0.00;

            cc = abc_update (gljc, &gljcRec);
			if (cc) 
                file_err (cc, gljc, "DBUPDATE");
		}
		else
            abc_unlock (gljc);

		cc = find_rec (gljc, &gljcRec, NEXT, "u");
	}
}

/*
 * Add transactions to glwk file.
 */
void
AddGeneralLedger (
	 char   	*acc_no,
	 long   	hhmrHash,
	 double 	amount,
	 double	 	amountOrigin,
	 double 	exch_rate,
	 char   	*currency)
{
	int		monthPeriod;

	strcpy (glwkRec.est_no,		"  ");
	sprintf (glwkRec.acronym,	"%9.9s",   " ");
	sprintf (glwkRec.name,		"%30.30s", " ");
	sprintf (glwkRec.chq_inv_no,"%15.15s", " ");
	sprintf (glwkRec.alt_desc1, "%20.20s", " ");
	sprintf (glwkRec.alt_desc2, "%20.20s", " ");
	sprintf (glwkRec.alt_desc3, "%20.20s", " ");
	sprintf (glwkRec.batch_no,  "%10.10s", " ");
	sprintf (glwkRec.run_no,  	"%10.10s", " ");
	glwkRec.ci_amt = 0.00;
	glwkRec.o1_amt = 0.00;
	glwkRec.o2_amt = 0.00;
	glwkRec.o3_amt = 0.00;
	glwkRec.o4_amt = 0.00;

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, " 3");
	glwkRec.post_date	= TodaysDate ();
	glwkRec.tran_date		= postingDate;

	DateToDMY (postingDate, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	strcpy (glwkRec.user_ref, gltrRec.user_ref);
	strcpy (glwkRec.stat_flag, "1");
	strcpy (glwkRec.jnl_type, "1");
	if (amount < 0.00)
	{
		strcpy (glwkRec.jnl_type, "2");
		amount		 *= -1.00;
		amountOrigin *= -1.00;
	}
	strcpy (glwkRec.narrative, gltrRec.narrative);
	glwkRec.amount		= amountOrigin;
	glwkRec.loc_amount	= amount;
	glwkRec.exch_rate	= exch_rate;
	sprintf (glwkRec.currency, "%-3.3s", currency);

	strcpy (glwkRec.acc_no, acc_no);
	glwkRec.hhgl_hash = hhmrHash;

	cc = RF_ADD (glwkWorkFileNo, (char *) &glwkRec);
	if (cc)
        file_err (cc, glwk, "WKADD");
}

/* [ end of file ] */
