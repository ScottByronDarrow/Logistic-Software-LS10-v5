/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_yend.c,v 5.3 2001/08/09 09:14:04 scott Exp $
|  Program Name  : (gl_yend.c)
|  Program Desc  : (Create Year End Postings To Clear G/L Accounts.)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author        : Scott Darrow.    |
|---------------------------------------------------------------------|
| $Log: gl_yend.c,v $
| Revision 5.3  2001/08/09 09:14:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:43  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:08  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_yend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_yend/gl_yend.c,v 5.3 2001/08/09 09:14:04 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_gl_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

	char	*data	= "data";

	extern	int	GV_fiscal;

	/*
	 * Special fields and flags ##################################.
	 */
	int		data_posted = FALSE;

	char	glPLAccount [MAXLEVEL + 1];
	char	glSuspense [MAXLEVEL + 1];

	double 	totalFgnPost = 0.00;
	double 	totalLocPost = 0.00;

	int		printerNumber;

/*
 * Local Function Prototypes.
 */
int 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int 	CheckComr 		 (void);
void 	Update 			 (double, double);
int 	CheckDates 		 (void);
int 	ValidateDate 	 (long, char *);
void 	shutdown_prog 	 (int);
static 	int 	MoneyZero (double);

/*
 * Main Processing Loop.
 */
int
main (
	int		argc,
	char	*argv [])
{
	int		fyear;
	double	fgnYearEndValue	= 0.00,
			locYearEndValue	= 0.00,
			checkFgnTotal 	= 0.0,
			checkLocTotal 	= 0.0;

	if (argc != 2)
	{
		print_at (0,0, mlStdMess036, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);

	if (OpenDB () == 1)
    {
        return (EXIT_FAILURE);
    }

	sprintf (err_str, "G/L Year End Close For Period %02d", GV_fiscal);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	DateToFinDMY 
	 (
		comr_rec.yend_date,
		comm_rec.fiscal, 
		NULL, 
		NULL,
		&fyear
	);

	/*
	 * Process FLP and FAP accounts.
	 */
	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s",
						 MAXLEVEL,MAXLEVEL, "0000000000000000");
	strcpy (glmrRec.glmr_class [0], "F");
	cc = find_rec (glmr, &glmrRec, GTEQ, "u");
	while
	 (
	    !cc &&
	    glmrRec.glmr_class [0][0] == 'F' &&
	    !strcmp (glmrRec.co_no, comm_rec.co_no)
	)
	{
	    if (glmrRec.glmr_class [2][0] == 'P' && (
		     glmrRec.glmr_class [1][0] == 'A' ||
		     glmrRec.glmr_class [1][0] == 'L'))
	    {
			dsp_process ("Account : ", glmrRec.acc_no);
			fgnYearEndValue	=	GL_FgnTotGlpd 
								 (
									glmrRec.hhmr_hash, 
									0, 
									fyear, 
									1, 
									99
								);
			fgnYearEndValue = 	no_dec (fgnYearEndValue);
			locYearEndValue = 	GL_LocTotGlpd 
								 (
									glmrRec.hhmr_hash, 
									0, 
									fyear, 
									1, 
									99
								);
			locYearEndValue	= 	no_dec (locYearEndValue);
			totalLocPost 	+= 	no_dec (locYearEndValue);
			totalFgnPost 	+= 	no_dec (fgnYearEndValue);
			Update (fgnYearEndValue,locYearEndValue);
	    }
	    abc_unlock (glmr);

	    cc = find_rec (glmr, &glmrRec, NEXT, "u");
	}

	/*
	 * Process FEP and FIP accounts. 
	 */
	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s",
						 MAXLEVEL,MAXLEVEL, "0000000000000000");
	strcpy (glmrRec.glmr_class [0], "F");

	cc = find_rec (glmr, &glmrRec, GTEQ, "r");
	while
	 (
	    !cc &&
	    glmrRec.glmr_class [0][0] == 'F' &&
	    !strcmp (glmrRec.co_no,comm_rec.co_no)
	)
	{
	    if (glmrRec.glmr_class [2][0] == 'P' && (
		     glmrRec.glmr_class [1][0] == 'E' ||
		     glmrRec.glmr_class [1][0] == 'I'))
	    {
			cc = FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");

			dsp_process ("Check Account : ", glmrRec.acc_no);
			fgnYearEndValue = GL_FgnTotGlpd (glmrRec.hhmr_hash, 0, fyear,1,99);
			checkFgnTotal += no_dec (fgnYearEndValue);
			locYearEndValue = GL_LocTotGlpd (glmrRec.hhmr_hash, 0, fyear,1,99);
			checkLocTotal += no_dec (locYearEndValue);
	    }
	    cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}

	if (!MoneyZero (no_dec (totalLocPost) + no_dec (checkLocTotal)))
	{
		double	balanceLoc	=	0.00,
				balanceFgn	=	0.00;

		clear ();
		print_at (0,0, "The General Ledger is out of balance.");
		print_at (1,0, "The year end close will continue.");
		print_at (3,0, "The balances are P & L = %.2f OTHER = %.2f for Local.", 
							DOLLARS (totalLocPost), DOLLARS (checkLocTotal));
		print_at (4,0, "The balances are P & L = %.2f  OTHER = %.2f for Fgn.", 
							DOLLARS (totalFgnPost), DOLLARS (checkFgnTotal));
		print_at (5,0, "Inbalance posted to company Suspense Account.");
        PauseForKey (7, 0, "Press return to continue.", 0);

		strcpy (glmrRec.co_no, comm_rec.co_no);
		strcpy (glmrRec.glmr_class [0], "F");
		sprintf (glmrRec.acc_no, "%-*.*s",MAXLEVEL,MAXLEVEL,glSuspense);
		if ( (cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
			file_err (cc, glmr, "DBFIND");

		cc = FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");

		clear ();

		balanceFgn	=	checkFgnTotal + totalFgnPost;
		balanceLoc	=	checkLocTotal + totalLocPost;
		Update (balanceFgn * -1,balanceLoc * -1);
	}

	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.glmr_class [0], "F");
	sprintf (glmrRec.acc_no, "%-*.*s",MAXLEVEL,MAXLEVEL,glPLAccount);
	if ( (cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
		file_err (cc, glmr, "DBFIND");
	
	cc = FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");

	Update (checkFgnTotal,checkLocTotal);

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, comr, "DBFIND");

	/*
	 * Increment Year-End to next one
	 */
	comr_rec.yend_date = AddYears (comr_rec.yend_date, 1);

	cc = abc_update (comr, &comr_rec);
	if (cc)
		file_err (cc, comr, "DBUPDATE");

	if (data_posted == TRUE)
    {
		shutdown_prog (0);
        return (EXIT_SUCCESS);
    }
	else
    {
		shutdown_prog (-1);
        return (EXIT_FAILURE);
    }
}

/*
 * Open data base files.
 */
int
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no , comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (CheckDates () == 1)
    {
        return (EXIT_FAILURE);
    }

	OpenGlmr (); abc_selfield (glmr, "glmr_hhmr_hash");
	OpenGlpd ();

	if (CheckComr () == 1)
    {
        return (EXIT_FAILURE);
    }

	abc_selfield (glmr, "glmr_id_no2");

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
    return (EXIT_SUCCESS);
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_CloseBatch (printerNumber);
	GL_Close ();
	abc_fclose (comr);
	abc_dbclose (data);
}

int
CheckComr (void)
{
	glmrRec.hhmr_hash	=	comr_rec.pl_app_acc;
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc ||
		strcmp (comm_rec.co_no, glmrRec.co_no) ||
		strcmp (glmrRec.glmr_class [0], "F") ||
		strcmp (glmrRec.glmr_class [2], "P"))
	{
		errmess (ML (mlGlMess022));
		shutdown_prog (1);
        return (EXIT_FAILURE);
	}

	sprintf (glPLAccount, "%-*.*s", MAXLEVEL,MAXLEVEL, glmrRec.acc_no);

	glmrRec.hhmr_hash	=	comr_rec.fin_susp;
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc ||
		strcmp (comm_rec.co_no, glmrRec.co_no) ||
		strcmp (glmrRec.glmr_class [0], "F") ||
		strcmp (glmrRec.glmr_class [2], "P"))
	{
		errmess (ML ("Financial Suspense"));
		shutdown_prog (1);
        return (EXIT_FAILURE);
	}
	sprintf (glSuspense, "%-*.*s", MAXLEVEL,MAXLEVEL, glmrRec.acc_no);
    return (EXIT_SUCCESS);
}

/*
 * Create work file for clearing transaction.
 */
void
Update (
	double	fgnYearEndValue,
	double	locYearEndValue)
{
	if (MoneyZero (fgnYearEndValue) && MoneyZero (locYearEndValue))
		return;

	glwkRec.tran_date 	= get_ybeg (comm_rec.gl_date);
	glwkRec.post_date 	= TodaysDate ();
	glwkRec.amount 		= fgnYearEndValue;
	glwkRec.loc_amount	= locYearEndValue;

	if (MoneyZero (fgnYearEndValue))
		glwkRec.exch_rate 	= 1.00;
	else
		glwkRec.exch_rate 	= locYearEndValue / fgnYearEndValue;

	/*
	 * Set the transaction rate to match the local and forgein amount
	 */
	strcpy (glwkRec.currency, comr_rec.base_curr);
	glwkRec.hhgl_hash 	= glmrRec.hhmr_hash;
	strcpy (glwkRec.acronym, "         ");
	sprintf (glwkRec.name, "%-30.30s", ML ("G/L YEAR END POSTING"));
	sprintf (glwkRec.narrative, "%-20.20s", ML ("G/L YEAR END POST"));
	sprintf (glwkRec.alt_desc1, "%20.20s", " ");
	sprintf (glwkRec.alt_desc2, "%20.20s", " ");
	sprintf (glwkRec.alt_desc3, "%20.20s", " ");
	sprintf (glwkRec.batch_no, "%10.10s", " ");
	strcpy (glwkRec.user_ref, "YR/END");
	strcpy (glwkRec.stat_flag, "2");
	strcpy (glwkRec.tran_type, " 1");
	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.acc_no, glmrRec.acc_no);
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	sprintf (glwkRec.period_no, "%02d", GV_fiscal + 1);

	if (locYearEndValue < 0)
	{
		locYearEndValue *= -1;
		fgnYearEndValue *= -1;
		glwkRec.amount 		= fgnYearEndValue;
		glwkRec.loc_amount 	= locYearEndValue;
		strcpy (glwkRec.jnl_type, "2");
	}
	else
		strcpy (glwkRec.jnl_type, "1");

	if (locYearEndValue)
	{
		data_posted = TRUE;
		GL_AddBatch ();
	}
}

int
CheckDates (void)
{
	if (comr_rec.yend_date >= comr_rec.gl_date)
	{
		clear ();
		print_at (0, 0, ML (mlGlMess023));
		sleep (30);
		shutdown_prog (-1);
        return (EXIT_FAILURE);
	}
	if (ValidateDate (comr_rec.dbt_date, "Customers") == 1)
        return (EXIT_FAILURE);
    
    if (ValidateDate (comr_rec.crd_date, "Suppliers") == 1)
        return (EXIT_FAILURE);
    
	if (ValidateDate (comr_rec.inv_date, "Inventory") == 1)
        return (EXIT_FAILURE);
    
    return (EXIT_SUCCESS);
}

/*
 * Check other modules are ok.
 */
int
ValidateDate (
 long               m_date,
 char*              m_name)
{
	if (!m_date || m_date < comr_rec.gl_date)
	{
		clear ();
		rv_pr (ML (mlGlMess014), 0, 0, 1);
		sleep (30);
		shutdown_prog (-1);
        return (EXIT_FAILURE);
	}
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 int                rcode)
{
	CloseDB (); 
	FinishProgram ();
}


/*
 *	Minor support functions
 */
static int
MoneyZero (
 double             m)
{
	return (fabs (m) < 0.0001);
}
