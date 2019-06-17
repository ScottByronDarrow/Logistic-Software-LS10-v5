/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_wupdate.c,v 5.6 2002/07/17 09:57:21 scott Exp $
|  Program Name  : (gl_wupdate.c) 
|  Program Desc  : (Updates General Ledger Journal Transactions)
|               (From G/L Transaction File (glwk))
|---------------------------------------------------------------------|
|  Date Written  : (20/07/89)      | Author       : Huon Butterworth  |
|---------------------------------------------------------------------|
| $Log: gl_wupdate.c,v $
| Revision 5.6  2002/07/17 09:57:21  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2001/08/20 23:12:50  scott
| Updated for development related to bullet proofing
|
| Revision 5.4  2001/08/09 09:14:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/08 07:30:24  scott
| Updated from testing
|
| Revision 5.2  2001/08/06 23:27:41  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:07  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_wupdate.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_wupdate/gl_wupdate.c,v 5.6 2002/07/17 09:57:21 scott Exp $";

#include <pslscr.h>
#include <ml_gl_mess.h>
#include <ml_std_mess.h>
#include <GlUtils.h>
#include <GlAudit.h>
#define	 X_OFF		22
#define	 Y_OFF		5
#include <get_lpno.h>
#include <pDate.h>
#include <FinancialDates.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

GLWK_STRUCT	workRec;

	Money	*gljc_tots	=	&gljcRec.tot_1;

	int		endOfMonthUpdate 	= FALSE,
			printerNumber 		= 1,
			GV_pid				= 0,
			glwk_no				= 0,
			nextLine			= 0,
			noData				= TRUE;

	long	currentPage			= 0;

	char	*data	= "data",
			mlWUpdate [10][101],
			*userRefBlank		=	"               ",
			*sixteenSpace		=	"                ",
			previousUserRef	[sizeof glwkRec.user_ref];

	double	batchAmount 		= 	0.0,
			totalAmount [6]		=	{0.0,0.0,0.0,0.0,0.0,0.0};

	static	void	UpdateGlna 	(long);

struct
{
	 char   fin_susp [FORM_LEN + 1];
	 char   nfn_susp [FORM_LEN + 1];
} local_rec;

/*
 *****************************************************************************
 *	Structure	:	PRD_STRUCT, *PRD_PTR
 *****************************************************************************
 *
 *	Description	:	Structure to hold consolidated period values.
 *
 *	Notes		:	A tree of these structures is held off each
 *					consolidated account structure.
 *			
 *	Elements	:	prd_date    - Date
 *				:	prd_year    - Financial year (YY).
 *				:	prd_no      - Financial period. (MM).
 *				:	prd_balance - Consolidated balance.
 *				:	prd_loc_balance - Consolidate local balance.
 *				:	prd_curr	- Currency
 *				:	prd_rate	- Exchange rate
 *				:	prd_left    - Pointer to greater periods.
 *				:	prd_right   - Pointer to lesser periods.
 *				:	prd_class   - Glmr_class [0][0] ('F' | 'P' | 'C')
 *				:	prd_stat    - D (eleted) B (alance) A (dded)
 */
typedef struct prd_node
	{
		long	prd_date;
		int		prd_year;
		int		prd_no;
		double	prd_balance;
		double	prd_loc_balance;
		char	prd_curr [4];
		double	prd_rate;
		struct	prd_node *prd_left, *prd_right;
		char	prd_class;
		char	prd_stat;
	} PRD_STRUCT, *PRD_PTR;

#define	PRD_DATE	prd_ptr->prd_date
#define	PRD_YEAR	prd_ptr->prd_year
#define	PRD_NO		prd_ptr->prd_no
#define	PRD_BALANCE	prd_ptr->prd_balance
#define	PRD_LOC_BAL	prd_ptr->prd_loc_balance
#define PRD_CURR	prd_ptr->prd_curr
#define	PRD_RATE	prd_ptr->prd_rate
#define	PRD_LEFT	prd_ptr->prd_left
#define	PRD_RIGHT	prd_ptr->prd_right
#define	PRD_CLASS	prd_ptr->prd_class
#define	PRD_STAT	prd_ptr->prd_stat

/*
 *****************************************************************************
 *	Structure	:	ACC_STRUCT, *ACC_PTR
 *****************************************************************************
 *
 *	Description	:	Structure to hold consolidated accounts.
 *
 *	Notes		:	A tree of period structures is held off each
 *					consolidated account structure.
 *			
 *	Elements	:	acctNo	  	- Pointer to account no.
 *				:	acctPeriods - Pointer to period tree.
 *				:	acctLeft  	- Pointer to greater accounts.
 *				:	acctRight 	- Pointer to lesser accounts.
 *				:	acctStatus  - D(eleted) B(alance) A(dded)
 */

typedef struct acctNode
        {
            char	*acctNo;
            PRD_PTR	acctPeriods;
            struct	acctNode *acctLeft, *acctRight;
            char	acctStatus;
        } ACC_STRUCT, *ACC_PTR;

static	ACC_PTR		PV_first;
static	PRD_PTR		PV_bal_head;

#define	ACC_NO		acc_ptr->acctNo
#define	ACC_PERIODS	acc_ptr->acctPeriods
#define	ACC_LEFT	acc_ptr->acctLeft
#define	ACC_RIGHT	acc_ptr->acctRight
#define	ACC_STAT	acc_ptr->acctStatus

static	char	PV_co_no [3],
				*PV_fin_susp = (char *) 0,
				*PV_nfn_susp = (char *) 0;

char	oldAcronym [10];

/*
 * Local Function Prototypes.
 */
char 	*AccountSave 		(char *);
int 	ConsEnd 			(char *, int);
int 	ConsPost 			(char *, long, double, double, char *, double);
int 	PeriodCompare 		(PRD_PTR, int, int, char);
int 	PostGLData 			(void);
int 	WorkFileRead 		(void);
int 	WriteGltr 			(void);
int 	OpenWorkFile 		(void);
static 	ACC_PTR AccountAllocate 	(void);
static 	ACC_PTR AccountWrite (ACC_PTR, char *, int, int, double, double, char *, double, long, char, char);
static 	PRD_PTR PeriodAlloc (void);
static 	PRD_PTR PeriodWrite (PRD_PTR, long, int, int, double, double, char *, double, char, char);
static 	int MoneyZero 		(double);
void 	AddGlwk 			(char *, PRD_PTR);
void	AddGltc 			(long);
void 	CloseDB 			(void);
void 	ConsAudit 			(void);
void 	ConsStart 			(char *, char *, char *, char *, int, int);
void 	GljcUpdate 			(void);
void 	GltrModify 			(void);
void 	GltrUpdate 			(void);
void 	InitML 				(void);
void 	LoadPaper 			(void);
void 	OpenDB 				(void);
void 	PeriodCheck 		(PRD_PTR);
void 	PostAccountFunc 	(FILE *, ACC_PTR acc_ptr);
void 	PostPeriod 			(FILE *, char *, PRD_PTR);
void 	PrintAccount 		(FILE *, ACC_PTR);
void 	PrintAudit 			(char *);
void 	PrintPeriod 		(FILE *, char *, PRD_PTR);
void 	ReadGljc 			(char *);
void 	WorkFileUpdate 		(void);
void 	WriteGLLog 			(char *, char *);
void 	CloseWorkFile 		(void);
void 	GlwkUpdate 			(void);
void 	shutdown_prog 		(void);

/*
 * Main processing routine.
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	int		rc;

	length = 0;

	init_scr ();
	set_tty ();

	if (argc < 3)
	{
		print_at (0,0,mlGlMess703, argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	if (!strcmp (sptr, "gl_eom_update"))
		endOfMonthUpdate = TRUE;

	strcpy (gljcRec.journ_type, argv [1]);
	GV_pid = atoi (argv [2]);

	if (argc > 3)
		printerNumber = atoi (argv [3]);

	if (!printerNumber)
		LoadPaper ();
	
	/*
	 * Open Database files.
	 */
	OpenDB ();

	/*
	 * Read journal control records.
	 */
	ReadGljc (argv [1]);

	InitML ();

	rc = TRUE;

	WriteGLLog (gljcRec.journ_type, "2");

	if (PostGLData ())
	{
		WriteGLLog (gljcRec.journ_type, "1");
        if (WriteGltr () == 1)
        {
            return (EXIT_SUCCESS);
        }
		WriteGLLog (gljcRec.journ_type, "0");
		rc = FALSE;

		/*
		 * Update journal control recs.
		 */
		if (!endOfMonthUpdate)
			GljcUpdate ();
	}

	if (noData)
	{
		WriteGLLog (gljcRec.journ_type, "0");
		rc = FALSE;
	}

	shutdown_prog ();
	CloseDB (); FinishProgram ();;
	if (rc == FALSE)
        return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
InitML (void)
{
	strcpy (mlWUpdate [1], ML ("User"));
	strcpy (mlWUpdate [2], ML ("Terminal"));
	strcpy (mlWUpdate [3], ML ("Time"));
	strcpy (mlWUpdate [4], ML ("Consolidation"));
	strcpy (mlWUpdate [5], ML (mlGlMess175));
	strcpy (mlWUpdate [6], ML (mlGlMess176));
}

void
shutdown_prog (void)
{
	if (!noData)
		ConsAudit ();
	crsr_on ();
	rset_tty ();
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
 
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	OpenGllg ();
	OpenGljc ();
	OpenGlmr ();	abc_selfield (glmr, "glmr_hhmr_hash");
	OpenGltr ();
	OpenGltc ();
	OpenGlna ();

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");
	abc_fclose (comr);

	glmrRec.hhmr_hash	=	comr_rec.fin_susp;
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
	{
		sprintf (err_str, "DBFIND FIN-SUSP [%s][%ld]", comm_rec.co_no, comr_rec.fin_susp);
		file_err (cc, glmr, err_str);
	}
	strcpy (local_rec.fin_susp, glmrRec.acc_no);

	glmrRec.hhmr_hash	=	comr_rec.nfn_susp;
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND NFN-SUSP");
	strcpy (local_rec.nfn_susp, glmrRec.acc_no);

	abc_selfield (glmr, "glmr_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose (data);
}

int
OpenWorkFile (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/gl_work%05d", (sptr) ? sptr : "/usr/LS10.5", GV_pid);

	if ((cc = RF_OPEN (filename, sizeof (workRec), "u", &glwk_no)))
	{
		if (cc == 2) /* File doesn't exist.	*/
        {
			shutdown_prog ();
        	return (EXIT_FAILURE);
        }
		else
			file_err (cc, glwk, "WKOPEN");
	}
	return (EXIT_SUCCESS);
}

void
CloseWorkFile (void)
{
	if ((cc = RF_CLOSE (glwk_no)))
		file_err (cc, glwk, "WKCLOSE");
}

int
WorkFileRead (void)
{
	if ((cc = RF_READ (glwk_no, (char *) &workRec)))
		return (cc);
	else
		memcpy ((char *) &glwkRec, (char *) &workRec, sizeof (GLWK_STRUCT));

	return (EXIT_SUCCESS);
}

void
WorkFileUpdate (void)
{
	strcpy (glwkRec.stat_flag, "9");
	memcpy ((char *) &workRec, (char *) &glwkRec, sizeof (GLWK_STRUCT));

	if ((cc = RF_UPDATE (glwk_no, (char *) &workRec)))
		file_err (cc, glwk, "WKUPDATE");
}

/*
 * Get journal control records for jnl types 1, 2 & 3 
 */
void
ReadGljc (
	char	*journalNo)
{
	int	i;

	cc = FindGljc (comm_rec.co_no, journalNo, "u");
	if (cc)
		file_err (cc, gljc, "DBFIND");
	
	for (i = 0; i < 6; i++)
		totalAmount [i] = gljc_tots [i];

	currentPage = (long) gljcRec.nxt_pge_no;
	nextLine = 1;
}

/*
 * Routine to check if paper loaded.   
 */
void
LoadPaper (void)
{
	clear ();

	box (12, 0, 56, 3);
	print_at (1,14, mlWUpdate [5]);
	print_at (3,14, mlWUpdate [6]);
	fflush (stdout);
	printerNumber = get_lpno (0);
}

void
WriteGLLog (
 char*              jnl_type,
 char*              stat)
{
	char	tmp_str [128];

	strcpy (gllgRec.co_no, comm_rec.co_no);
	strcpy (gllgRec.est_no, comm_rec.est_no);
	gllgRec.pid = GV_pid;
	strcpy (gllgRec.jnl_type, jnl_type);
	cc = find_rec (gllg, &gllgRec, EQUAL, "u");

	sprintf 
	(
		tmp_str, 
		 "%-4.4s %-8.8s - %-8.8s %2d - %-4.4s %-5.5s",
		 mlWUpdate [1],
		 getlogin (),
		 mlWUpdate [2],
		 comm_rec.term,
		 mlWUpdate [3],
		 TimeHHMM ()
	);

	sprintf (gllgRec.desc, "%-40.40s", tmp_str);

	gllgRec.mod_date = TodaysDate ();
	strcpy (gllgRec.stat_flag, stat);

	if (cc)
	{
		if ((cc = abc_add (gllg, &gllgRec)))
			file_err (cc, gllg, "DBADD");
	}
	else
		if ((cc = abc_update (gllg, &gllgRec)))
			file_err (cc, gllg, "DBUPDATE");

	if (*stat == '0')
		abc_unlock (gllg);
}

int
PostGLData (void)
{
	noData = TRUE;

	ConsStart 
	(
		comm_rec.co_no, 
		comm_rec.co_name, 
		local_rec.fin_susp, 
		local_rec.nfn_susp,
		0, 
		printerNumber
	);
	GL_SetAccWidth (comm_rec.co_no, TRUE);

   	strcpy (previousUserRef, userRefBlank);

	print_mess (ML ("Updating General Ledger Master File."));
	/*
	 * Process General Ledger Transactions.
	 */
	if (OpenWorkFile ())
		return (EXIT_FAILURE);
    
	cc = WorkFileRead ();
	while (!cc)
	{
		if (glwkRec.stat_flag [0] == '9')
		{
			cc = WorkFileRead ();
			continue;
		}
		if (MoneyZero (glwkRec.loc_amount) && MoneyZero (glwkRec.amount))
		{
			cc = WorkFileRead ();
			continue;
		}
		if (strncmp (glwkRec.acc_no, sixteenSpace,MAXLEVEL))
		{
			cc = atoi (glwkRec.jnl_type);
			if (! (cc % 2))
			{
				batchAmount 		+= glwkRec.amount;
				glwkRec.amount 		= glwkRec.amount *-1;
				glwkRec.loc_amount 	= glwkRec.loc_amount *-1;
			}

			/*
			 * Update General ledger Journal Records.
			 */
            if 
			(
				ConsPost 
				(
					glwkRec.acc_no, 
					glwkRec.tran_date, 
					glwkRec.amount,
					glwkRec.loc_amount,
					glwkRec.currency,
					glwkRec.exch_rate
				)
			)
			{
				strcpy (glwkRec.stat_flag, "D");
				glwkRec.amount 		= 0.00;
				glwkRec.loc_amount	= 0.00;
				memcpy (&workRec, &glwkRec, sizeof (GLWK_STRUCT));
				if ((cc = RF_UPDATE (glwk_no, (char *) &workRec)))
					file_err (cc, glwk, "WKUPDATE");
			}
		}
		noData = FALSE;

		cc = WorkFileRead ();
	}
	if (ConsEnd (gljcRec.journ_type, GV_pid))
	{
		CloseWorkFile ();
		return (noData ? EXIT_SUCCESS : EXIT_FAILURE);
	}
	CloseWorkFile ();
	return (EXIT_SUCCESS);
}

int
WriteGltr (void)
{
   	strcpy (previousUserRef, userRefBlank);

	/*
	 * Write Transactions.
	 */
	if (OpenWorkFile ())
        return (EXIT_FAILURE);
    
	cc = WorkFileRead ();

	while (!cc)
	{
		if (glwkRec.stat_flag [0] == '9')
		{
			cc = WorkFileRead ();
			continue;
		}
		sprintf (glwkRec.sys_ref, "%010ld",currentPage);
		if (strncmp (glwkRec.acc_no, sixteenSpace,MAXLEVEL))
		{
			cc = atoi (glwkRec.jnl_type);
			if (! (cc % 2))
			{
				batchAmount 		+= glwkRec.amount;
				glwkRec.amount 		= glwkRec.amount 		*-1;
				glwkRec.loc_amount 	= glwkRec.loc_amount	*-1;
			}

			GltrUpdate (); 
		}
		GlwkUpdate (); 
		cc = WorkFileRead ();
	}

	CloseWorkFile ();
	return (EXIT_SUCCESS);
}

/*
 * Update General Ledger transaction.
 */
void
GlwkUpdate (void)
{
	WorkFileUpdate ();

	nextLine++;
	if (nextLine >= 46)
	{
		nextLine = 1;
		currentPage++;
	}

	/*
	 * Increment page and line ref nos. & add to journal control totals.
	 */
	if (strncmp (glwkRec.acc_no, sixteenSpace,MAXLEVEL) &&
		strcmp (oldAcronym, glwkRec.acronym) && 
		strcmp (glwkRec.acronym, "         "))
	{
		nextLine++;	/* one for descriptive line	*/
		if (nextLine >= 46)
		{
			nextLine = 1;
			currentPage++;
		}
		nextLine++;	/* and one more for blank line	*/
		if (nextLine >= 46)
		{
			nextLine = 1;
			currentPage++;
		}

		strcpy (oldAcronym, glwkRec.acronym);
	}
}

void
GltrUpdate (void)
{
	int	i;

	GltrModify ();

	i = atoi (glwkRec.jnl_type);
	if (!(i % 2))
	{
		glwkRec.amount 		= glwkRec.amount 	 * -1;
		glwkRec.loc_amount 	= glwkRec.loc_amount * -1;
	}
	i = i - 1;
	totalAmount [i] += glwkRec.amount;
}

void
GltrModify (void)
{
	char	buffer [30];
	long	lastGltrHash	=	0L;

	GLTR_STRUCT	gltrRec2;

	/*
	 * The following is for Consolidated postings.
	 */
	gltrRec.hhmr_hash 	= glwkRec.hhgl_hash;
	gltrRec.tran_date 	= glwkRec.tran_date;
	gltrRec.post_date	= glwkRec.post_date;
	gltrRec.amount 		= glwkRec.loc_amount;
	gltrRec.exch_rate 	= glwkRec.exch_rate;
	gltrRec.amt_origin 	= glwkRec.amount;
	strcpy (gltrRec.tran_type, glwkRec.tran_type);
	strcpy (gltrRec.sys_ref, glwkRec.sys_ref);
	strcpy (gltrRec.batch_no, glwkRec.batch_no);
	strcpy (gltrRec.narrative, glwkRec.narrative);
	strcpy (gltrRec.currency, glwkRec.currency);

	if (strcmp (glwkRec.user_ref, userRefBlank))
	{
		strcpy (gltrRec.user_ref, glwkRec.user_ref);
		strcpy (previousUserRef,  glwkRec.user_ref);
	}
	else
		strcpy (gltrRec.user_ref, previousUserRef);

	strcpy (gltrRec.stat_flag, (endOfMonthUpdate) ? "1" : "0");

	if (gljcRec.stat_flag [0] == 'C')
	{
		gltrRec2.hhmr_hash 	= glwkRec.hhgl_hash;
		gltrRec2.tran_date 	= glwkRec.tran_date;
		gltrRec2.post_date 	= glwkRec.post_date;
		strcpy (gltrRec2.tran_type, gltrRec.tran_type);
		strcpy (gltrRec2.user_ref, glwkRec.run_no);

		if ((cc = find_rec (gltr, &gltrRec2, COMPARISON, "u")))
		{
			abc_unlock (gltr);
			sprintf 
			(
				buffer, 
				"CP %08ld-%08ld", 
				atol (glwkRec.sys_ref),
				atol (glwkRec.sys_ref)
			);

			sprintf (gltrRec2.narrative, "%-20.20s", buffer);
			strcpy (gltrRec2.sys_ref,  "REF NARR. ");
			strcpy (gltrRec2.user_ref,  glwkRec.run_no);
			strcpy (gltrRec2.batch_no, " N/A ");
			strcpy (gltrRec2.currency, 	glwkRec.currency);
			strcpy (gltrRec2.stat_flag, 	gltrRec.stat_flag);
			gltrRec2.tran_date 	= gltrRec.tran_date;
			gltrRec2.post_date 	= gltrRec.post_date;
			gltrRec2.amount 	= glwkRec.loc_amount;
			gltrRec2.amt_origin = glwkRec.amount;
			gltrRec2.exch_rate 	= glwkRec.exch_rate;

			if ((cc = abc_add (gltr, &gltrRec2)))
				file_err (cc, gltr, "DBADD");

			cc = find_rec (gltr, &gltrRec2, COMPARISON, "r");
			if (!cc)
				AddGltc (gltrRec2.gltr_hash);
		}
		else
		{
			sprintf 
			(
				buffer,
				"CP %08ld-%08ld", 
				atol (gltrRec2.narrative + 3),
				atol (glwkRec.sys_ref)
			);
			sprintf (gltrRec2.narrative, "%-20.20s", buffer);

			strcpy (gltrRec2.sys_ref,  "REF NARR. ");
			strcpy (gltrRec2.user_ref, glwkRec.run_no);
			strcpy (gltrRec2.currency, 	glwkRec.currency);
			strcpy (gltrRec2.stat_flag, 	gltrRec.stat_flag);
			gltrRec2.tran_date 	= gltrRec.tran_date;
			gltrRec2.post_date 	= gltrRec.post_date;
			gltrRec2.exch_rate 	= glwkRec.exch_rate;
			gltrRec2.amount 	+= glwkRec.loc_amount;
			gltrRec2.amt_origin += glwkRec.amount;

			cc = abc_update (gltr, &gltrRec2);
			if (cc)
				file_err (cc, gltr, "DBUPDATE");

			AddGltc (gltrRec2.gltr_hash);
		}
	}
	else
    {
		cc = abc_add (gltr, &gltrRec);
		if (cc)
			file_err (cc, gltr, "DBADD");

		lastGltrHash	=	0L;
		/*
		 * Code is required as fields within index are not enough 
		 * to allow unique record to be returned.
		 */
		cc = find_rec (gltr, &gltrRec, GTEQ, "r");
		while (!cc && gltrRec.hhmr_hash == glwkRec.hhgl_hash &&
					  !strcmp (gltrRec.tran_type, glwkRec.tran_type) &&
					  !strcmp (gltrRec.user_ref, glwkRec.user_ref) &&
					  gltrRec.tran_date == glwkRec.tran_date)
	  	{
			/*
			 * Record required
			 */
			if (!strcmp (gltrRec.sys_ref, glwkRec.sys_ref))
			{
				if (gltrRec.gltr_hash > lastGltrHash)
					lastGltrHash	=	gltrRec.gltr_hash;
			}
			cc = find_rec (gltr, &gltrRec, NEXT, "r");
		}
		if (lastGltrHash)
			UpdateGlna (lastGltrHash);
    }
}

void
AddGltc (
	long	gltrHash)
{
	/* 
	 * As transactions are consolidated hold transaction that would
	 * have been created in gltc file.
	 */
	gltcRec.gltr_hash 	= gltrHash;
	gltcRec.tran_date 	= gltrRec.tran_date;
	gltcRec.post_date	= gltrRec.post_date;
	gltcRec.amount 		= gltrRec.amount;
	gltcRec.exch_rate 	= gltrRec.exch_rate;
	gltcRec.amt_origin 	= gltrRec.amt_origin;
	strcpy (gltcRec.tran_type, 	gltrRec.tran_type);
	strcpy (gltcRec.batch_no, 	gltrRec.batch_no);
	strcpy (gltcRec.sys_ref, 	gltrRec.sys_ref);
	strcpy (gltcRec.narrative,	gltrRec.narrative);
	strcpy (gltcRec.currency, 	gltrRec.currency);
	strcpy (gltcRec.user_ref, 	gltrRec.user_ref);
	strcpy (gltcRec.stat_flag,  gltrRec.stat_flag);

	cc = abc_add (gltc, &gltcRec);
	if (cc)
		file_err (cc, gltc, "DBADD");
}

/*
 * Updated additional description lines. 
 */
static	void
UpdateGlna (
	long	gltrHash)
{
	int		lineNo	=	0;

	char	altDesc [sizeof	glnaRec.narrative];

	sprintf (altDesc, "%-20.20s", glwkRec.alt_desc1);
	if (strlen (clip (altDesc)))
	{
		glnaRec.gltr_hash	=	gltrHash;
		glnaRec.line_no		=	lineNo++;
		sprintf (glnaRec.narrative, glwkRec.alt_desc1);

		cc = abc_add (glna, &glnaRec);
		if (cc)
			file_err (cc, glna, "DBADD");
	}
	sprintf (altDesc, "%-20.20s", glwkRec.alt_desc2);
	if (strlen (clip (altDesc)))
	{
		glnaRec.gltr_hash	=	gltrHash;
		glnaRec.line_no		=	lineNo++;
		sprintf (glnaRec.narrative, glwkRec.alt_desc2);
		cc = abc_add (glna, &glnaRec);
		if (cc)
			file_err (cc, glna, "DBADD");
	}
	sprintf (altDesc, "%-20.20s", glwkRec.alt_desc3);
	if (strlen (clip (altDesc)))
	{
		glnaRec.gltr_hash	=	gltrHash;
		glnaRec.line_no		=	lineNo++;
		sprintf (glnaRec.narrative, glwkRec.alt_desc3);
		cc = abc_add (glna, &glnaRec);
		if (cc)
			file_err (cc, glna, "DBADD");
	}
}

void
ConsAudit (void)
{
	char	fname [128];

	sprintf (fname, "%s", GL_AudName (gljcRec.journ_type, GV_pid));
	PrintAudit (fname);
}

void
PrintAudit (
	char	*fname)
{
	FILE	*f_id,
			*p_id;
	char	*aud_ptr,
			*GL_AudGet (FILE *);

	if (! (f_id = fopen (fname, "r")))
		file_err (errno, fname, "FOPEN");

	if (! (p_id = popen ("pformat", "w")))
		file_err (errno, "pformat", "POPEN");

	GL_AudHeader (p_id, FALSE, gljcRec.jnl_desc, glwkRec.run_no);

	while ((aud_ptr = GL_AudGet (f_id)))
	{
		if (fprintf (p_id, aud_ptr) < 0)
			file_err (errno, fname, "FPRINTF");
	}

	fclose (f_id);

	if (GL_AudTrailer (p_id))
		unlink (fname);

	pclose (p_id);
}

/*
 * Update journal control records for jnl types 1, 2 & 3 
 */
void
GljcUpdate (void)
{
	int 	i;

	strcpy (gljcRec.co_no, comm_rec.co_no);
	if (nextLine == 1)
		currentPage--;

	gljcRec.nxt_pge_no = (int) currentPage + 1;

	for (i = 0; i < 6; i++)
		gljc_tots [i] = totalAmount [i] ;

	if ((cc = abc_update (gljc, &gljcRec)))
		file_err (cc, gljc, "DBUPDATE");
}

/*
 *****************************************************************************
 *	Function	:	ConsStart ()
 *****************************************************************************
 *	Description	:	Start consolidated posting session.
 *
 *	Notes		:	Cons_start opens the neccessary files for a
 *					consolidated posting run and initilises the
 *					variables that will be needed.
 *			
 *	Parameters	:	co_no	 - Company number.
 *					coName  - Company name.
 *					budgetNo  - Budget number.
 *					printerNo- Printer number.
 *					fin_susp - Financial Suspense Posting acct.
 *					nfn_susp - Non-Financial Suspense Posting acct.
 *
 *	Globals		:	PV_first - Pointer to first account structure
 *					   is set to (ACC_PTR) NULL.
 */
void
ConsStart (
	char	*coNo,
	char	*coName,
	char	*fin_susp,
	char	*nfn_susp,
	int		budgetNo,
	int		printerNo)
{
	strcpy (PV_co_no, coNo);

	PV_fin_susp = AccountSave (fin_susp);
	PV_nfn_susp = AccountSave (nfn_susp);

	PV_first = (ACC_PTR) NULL;
	PV_bal_head = (PRD_PTR) NULL;

	GL_PostSetup (coNo, budgetNo);
	GL_PostStamp ();
	GL_AudSetup (coName, printerNo);
}

/*
 *****************************************************************************
 *	Function	:	ConsPost ()
 *****************************************************************************
 *
 *	Description	:	Maintain tree of consolidated postings.
 *
 *	Notes		:	The account tree is held in ascending account
 *					orrder.
 *			
 *	Parameters	:	acctNo   - Pointer to account number.
 *					c_date   - Calendar date of period to post into.
 *					amount   - Amount to post.
 *
 *	Globals		:	PV_first - Pointer to first account structure
 *					   is updated to keep accounts in order.
 */
int
ConsPost (
	char	*acctNo,
	Date	c_date,
	double	amount,
	double	loc_amount,
	char	*currency,
	double	exch_rate)
{
	static	char	last_class = 'F';
	int		errCode	=	0,
			fyear	=	0,
			fmonth	=	0;

	GLMR_STRUCT	p_glmr;

	DateToFinDMY (c_date, comm_rec.fiscal, NULL, &fmonth, &fyear);

	strcpy (p_glmr.co_no, PV_co_no);
	strcpy (p_glmr.acc_no, acctNo);
	errCode = find_rec (glmr, &p_glmr, EQUAL, "r");
	if (errCode)
	{
		fprintf (stdout, "Account [%s][%d] not found\n",acctNo,errCode);
		fprintf (stdout, "**** WARNING SUSPENCE WILL BE USED ****\n");
		sleep (10);
		last_class = p_glmr.glmr_class [0][0];
	} else if (p_glmr.glmr_class [2][0] != 'P')
	{
		fprintf (stdout, "Account [%s][%d] not posting\n",acctNo,errCode);
		fprintf (stdout, "**** WARNING SUSPENCE WILL BE USED ****\n");
		sleep (10);
		errCode = 1;
		last_class = p_glmr.glmr_class [0][0];
	}
	if (errCode)
	{
		fprintf (stdout, "Account [%s] not a posting account\n",acctNo);
		fprintf (stdout, "**** WARNING SUSPENCE WILL BE USED ****\n");
		sleep (10);
	
		p_glmr.glmr_class [0][0] = last_class;
		PV_first = AccountWrite
		(
			PV_first,
			(last_class == 'F') ? PV_fin_susp : PV_nfn_susp,
			fyear,
			fmonth,
			amount,
			loc_amount,
			currency,
			exch_rate,
			c_date,
			p_glmr.glmr_class [0][0],
			'A'
		);
		PV_bal_head = PeriodWrite
		(
			PV_bal_head,
			c_date,
			fyear,
			fmonth,
			amount,
			loc_amount,
			currency,
			exch_rate,
			p_glmr.glmr_class [0][0],
			' '
		);
		amount = 0.00;
	}
	else
		last_class = p_glmr.glmr_class [0][0];

	PV_first = AccountWrite
	(
		PV_first,
		acctNo,
		fyear,
		fmonth,
		amount,
		loc_amount,
		currency,
		exch_rate,
		c_date,
		p_glmr.glmr_class [0][0],
		(errCode) ? 'D' : ' '
	);
	PV_bal_head = PeriodWrite
	(
		PV_bal_head,
		c_date,
		fyear,
		fmonth,
		amount,
		loc_amount,
		currency,
		exch_rate,
		p_glmr.glmr_class [0][0],
		' '
	);
	last_class = p_glmr.glmr_class [0][0];
	return (errCode);
}

/*
 *****************************************************************************
 *	Function	:	ConsEnd ()
 *****************************************************************************
 *
 *	Description	:	End consolidated posting session.
 *
 *	Notes		:	Routine to close files used in posting, print 
 *					the audit trail and carry out the file updates.
 *			
 *	Parameters	:	jtype	 - Journal type for audit trail.
 *					pid	 - Process id for audit trail.
 *
 *	Globals		:	PV_first - Pointer to first account structure.
 */
int
ConsEnd (
	char	*jtype,
	int      pid)
{
	FILE	*f_id;

	PeriodCheck (PV_bal_head);

	f_id = GL_AudOpen ("w+", jtype, pid);
	PrintAccount (f_id, PV_first);

	fseek (f_id, 0L, 0);
	PostAccountFunc (f_id, PV_first);

	GL_AudClose (f_id);
	GL_PostTerminate ();

   return (EXIT_FAILURE);
}

/*
 *****************************************************************************
 *	Function	:	PeriodCheck ()
 *****************************************************************************
 *	Description	:	Check/Repair consolidated period tree.
 *
 *	Notes		:	This function recursively checks and repairs the
 *				consolidated period tree.
 *
 *	Parameters	:	prd_ptr	- Pointer to period structure.
 */
void
PeriodCheck (
	PRD_PTR	prd_ptr)
{
	if (prd_ptr)
	{
		PeriodCheck (PRD_LEFT);
		if (!MoneyZero (PRD_LOC_BAL))
		{
			PV_first = AccountWrite
			(
				PV_first,
				(PRD_CLASS == 'F') ? PV_fin_susp : PV_nfn_susp,
				PRD_YEAR,
				PRD_NO,
				(0.00 - PRD_BALANCE),
				(0.00 - PRD_LOC_BAL),
				PRD_CURR,
				PRD_RATE,
				PRD_DATE,
				PRD_CLASS,
				'B'
			);
		}
		PeriodCheck (PRD_RIGHT);
	}
}

/*
 *****************************************************************************
 *	Function	:	PrintAccount ()
 *****************************************************************************
 *
 *	Description	:	Print consolidated account tree.
 *
 *	Notes		:	This function recursively prints the
 *				consolidated account tree.
 *			
 *	Parameters	:	f_id	- Pointer to FILE id of output
 *					  file / pipe.
 *				acc_ptr	- Pointer to account structure.
 */
void
PrintAccount (
	FILE	*f_id,
	ACC_PTR	acc_ptr)
{
	if (acc_ptr)
	{
		PrintAccount (f_id, ACC_LEFT);
		PrintPeriod (f_id, ACC_NO, ACC_PERIODS);
		PrintAccount (f_id, ACC_RIGHT);
	}
}

/*
 *****************************************************************************
 *	Function	:	PrintPeriod ()
 *****************************************************************************
 *
 *	Description	:	Print consolidated period tree.
 *
 *	Notes		:	This function recursively prints the
 *					consolidated period tree.
 *			
 *	Parameters	:	f_id	- Pointer to FILE id of output file / pipe.
 *					acctNo 	- Pointer to account number.
 *					prd_ptr	- Pointer to period structure.
 */
void
PrintPeriod (
 FILE*              f_id,
 char*              acctNo,
 PRD_PTR            prd_ptr)
{
	if (prd_ptr)
	{
		PrintPeriod (f_id, acctNo, PRD_LEFT);
		GL_AudSave 
		(
			f_id, 
			acctNo, 
			PRD_YEAR, 
			PRD_NO, 
			PRD_BALANCE, 
			PRD_LOC_BAL,
			PRD_CURR,
			PRD_RATE,
			"U"
		);
		PrintPeriod (f_id, acctNo, PRD_RIGHT);
	}
}

/*
 *****************************************************************************
 *	Function	:	PostAccountFunc ()
 *****************************************************************************
 *
 *	Description	:	Post contents of consolidated account tree.
 *
 *	Notes		:	This function recursively posts the
 *					consolidated account tree.
 *			
 *	Parameters	:	f_id	- Pointer to FILE id of output file / pipe.
 *					acc_ptr	- Pointer to account structure.
 */
void
PostAccountFunc (
	FILE	*f_id,
	ACC_PTR	acc_ptr)
{
	if (acc_ptr)
	{
		PostAccountFunc (f_id, ACC_LEFT);
		PostPeriod 		(f_id, ACC_NO, ACC_PERIODS);
		PostAccountFunc (f_id, ACC_RIGHT);
	}
}

/*
 *****************************************************************************
 * Function	:	PostPeriod ()
 *****************************************************************************
 *
 *	Description	:	Post contents of consolidated period tree.
 *
 *	Notes		:	This function recursively posts the
 *					consolidated period tree.
 *			
 *	Parameters	:	f_id	- Pointer to FILE id of output file / pipe.
 *					acctNo	- Pointer to account number.
 *					prd_ptr	- Pointer to period structure.
 */
void
PostPeriod (
	FILE	*f_id,
	char	*acctNo,
	PRD_PTR	prd_ptr)
{
	double fgnAmount	=	0.00;


	if (prd_ptr)
	{
		PostPeriod (f_id, acctNo, PRD_LEFT);
		if (PRD_STAT != 'D')
		{
			strcpy (glmrRec.co_no, comm_rec.co_no);
			strcpy (glmrRec.acc_no, acctNo);
			cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
			if (!strncmp (glmrRec.curr_code, PRD_CURR, 3))
			{
				fgnAmount = PRD_BALANCE;
			}
			else
			{
				FindPocr (comm_rec.co_no, glmrRec.curr_code, "r");
				fgnAmount = CurrencyFgnAmt (PRD_LOC_BAL);
			}
			_PostAccount 
			(
				acctNo, 
				PRD_YEAR, 
				PRD_NO, 
				glmrRec.curr_code,
				fgnAmount, 
				PRD_LOC_BAL
			);
		}
		if (PRD_STAT == 'A' || PRD_STAT == 'B')
			AddGlwk (acctNo, prd_ptr);
		switch (PRD_STAT)
		{
		case	'A':
			GL_AudSave 	
			(
				f_id, 
				acctNo, 
				PRD_YEAR, 
				PRD_NO, 
				PRD_BALANCE, 
				PRD_LOC_BAL, 
				PRD_CURR,
				PRD_RATE,
				"A"
			);
			break;

		case	'B':
			GL_AudSave 	
			(
				f_id, 
				acctNo, 
				PRD_YEAR, 
				PRD_NO, 
				PRD_BALANCE, 
				PRD_LOC_BAL, 
				PRD_CURR,
				PRD_RATE,
				"B"
			);
			break;

		case	'D':
			GL_AudSave 	
			(
				f_id, 
				acctNo, 
				PRD_YEAR, 
				PRD_NO, 
				PRD_BALANCE, 
				PRD_LOC_BAL, 
				PRD_CURR,
				PRD_RATE,
				"D"
			);
			break;
		default:
			GL_AudSave 	
			(
				f_id, 
				acctNo, 
				PRD_YEAR, 
				PRD_NO, 
				PRD_BALANCE, 
				PRD_LOC_BAL, 
				PRD_CURR,
				PRD_RATE,
				"P"
			);
			break;
		}
		PostPeriod (f_id, acctNo, PRD_RIGHT);
	}
}

/*
 *****************************************************************************
 *	Function	:	AccountWrite ()
 *****************************************************************************
 *	Description	:	Write account postings into consolidated list.
 *
 *	Notes		:	This routine recursively reads the tree untill
 *					the correct account is found or an empty slot
 *					is found. The amount is then written into the
 *					correct period.
 *			
 *	Parameters	:	acc_ptr	- Pointer to current slot in tree.
 *					acctNo	- Account number to be written into.
 *					year	- Year to be written into.
 *					month	- Month to be written into.
 *					amount	- Amount to be written.
 *					date	- Long Date.
 *					class	- Glmr_class [0][0] ('F' | 'P' | 'C')
 *					Status	- ' ' = Standard
 *							- 'A' = Account not found
 *							- 'B' = Balance adjustment
 *							- 'D' = Deleted account
 *
 *	Returns		:	acc_ptr	- Pointer to tree slot.
 */
static	ACC_PTR	
AccountWrite (
	ACC_PTR		acc_ptr,
	char*       acctNo,
	int         year,
	int         month,
	double      amount,
	double      loc_amount,
	char*       currency,
	double      exch_rate,
	long        c_date,
	char        _class,
	char        stat)
{
	int	cmp_res;

	if (!acc_ptr)
	{
	    acc_ptr = AccountAllocate ();
	    ACC_NO = AccountSave (acctNo);
	    ACC_PERIODS	=	PeriodWrite 
						(
							ACC_PERIODS,
							c_date,
							year,
							month,
							amount,
							loc_amount,
							currency,
							exch_rate,
							_class,
							stat
						);
	    ACC_STAT = stat;
	}
	else
	{
	    cmp_res = strcmp (ACC_NO, acctNo);
	    if (cmp_res == 0)
	    {
		    if (ACC_STAT < stat)
		        cmp_res = -1;
		    else
    		    cmp_res = (ACC_STAT == stat) ? 0 : 1;
	    }
	    if (cmp_res == 0)
        {
		    ACC_PERIODS	=	PeriodWrite 
							(
								ACC_PERIODS,
								c_date,
								year,
								month,
								amount,
								loc_amount,
								currency,
								exch_rate,
								_class,
								stat
							);
		}
	    else
        {
		    if (cmp_res > 0)
            {
		        ACC_LEFT	=	AccountWrite 
								(
									ACC_LEFT,
									acctNo,
									year,
									month,
									amount,
									loc_amount,
									currency,
									exch_rate,
									c_date,
									_class,
									stat
								);
            }
		    else if (cmp_res < 0)
            {
			    ACC_RIGHT	=	AccountWrite 
								(
									ACC_RIGHT,
									acctNo,
									year,
									month,
									amount,
									loc_amount,
									currency,
									exch_rate,
									c_date,
									_class,
									stat
								);
            }
        }
	}

	return (acc_ptr);
}

/*
 *****************************************************************************
 *	Function	:	PeriodWrite ()
 *****************************************************************************
 *	Description	:	Write period postings into consolidated list.
 *
 *	Notes		:	This routine recursively reads the tree untill
 *					the correct period is found or an empty slot
 *					is found. The amount is then written into the
 *					correct period.
 *			
 *	Parameters	:	prd_ptr	- Pointer to current slot in tree.
 *					year	- Year to be written into.
 *					month	- Month to be written into.
 *					amount	- Amount to be written.
 *					class	- Glmr_class [0][0] ('F' | 'P' | 'C')
 *
 *	Returns		:	prd_ptr	- Pointer to tree slot.
 */
static	PRD_PTR	
PeriodWrite (
	PRD_PTR	prd_ptr,
	long    c_date,
	int     year,
	int     month,
	double  amount,
	double  loc_amount,
	char*   currency,
	double  exch_rate,
	char    _class,
	char    stat)
{
	int	cmp_res;

	if (!prd_ptr)
	{
		prd_ptr		= PeriodAlloc ();
		PRD_DATE	= c_date;
		PRD_YEAR	= year;
		PRD_NO		= month;
		PRD_BALANCE	= amount;
   		PRD_LOC_BAL = loc_amount;
		strcpy (PRD_CURR, currency);
		PRD_RATE	= exch_rate;
		PRD_CLASS	= _class;
		PRD_STAT	= stat;
	}
	else
    {
		if (! (cmp_res = PeriodCompare (prd_ptr, year, month, _class)))
		{
			PRD_BALANCE += amount;
			PRD_LOC_BAL += loc_amount;
		}
		else if (cmp_res > 0)
        {
		    PRD_LEFT	=	PeriodWrite 
							(
								PRD_LEFT,
								c_date,
								year,
								month,
								amount,
								loc_amount,
								currency, 
								exch_rate,
								_class,
								stat
							);
        }
		else if (cmp_res < 0)
        {
		    PRD_RIGHT	=	PeriodWrite 
							(
								PRD_RIGHT,
								c_date,
								year,
								month,
								amount,
								loc_amount,
								currency,
								exch_rate,
								_class,
								stat
							);
        }        
    }

	return (prd_ptr);
}

/*
 *****************************************************************************
 *	Function	:	PeriodCompare ()
 *****************************************************************************
 *	Description	:	Routine to compare two periods in tree.
 *
 *	Notes		:	This routine is used to find the correct slot
 *					in the period tree to update / insert into.
 *			
 *	Parameters	:	prd_ptr	- Period to be compared.
 *					year	- Year of posting.
 *					month	- Month of posting.
 *					_class	- Glmr_class [0][0] ('F' | 'P' | 'C')
 *
 *	Returns		:	1 	- If period > insert value.
 *					-1 	- If period < insert value.
 *					0 	- If period == insert value.
 */
int
PeriodCompare (
	PRD_PTR	prd_ptr,
	int     year,
	int     month,
	char    _class)
{
	return	(year < PRD_YEAR)	? 1		:
			(year > PRD_YEAR)	? -1	:
			(month < PRD_NO)	? 1		:
			(month > PRD_NO)	? -1	:
			(_class < PRD_CLASS)	? 1		:
			(_class > PRD_CLASS)	? -1	: 0;
}

/*
 *****************************************************************************
 *	Function	:	AccountAllocate ()
 *****************************************************************************
 *	Description	:	Allocate slot in tree for new account.
 *
 *	Notes		:	Acc_alloc is used to allocate memory for new
 *					accounts in consolidated posting tree.
 *			
 *	Returns		:	acc_ptr	- Pointer to new slot.
 */
static ACC_PTR	
AccountAllocate (void)
{
	ACC_PTR	acc_ptr;

	if (! (acc_ptr = (ACC_PTR) malloc (sizeof (ACC_STRUCT))))
		file_err (-1, "AccountAllocate", "MALLOC");

	ACC_PERIODS = (PRD_PTR) NULL;
	ACC_LEFT = ACC_RIGHT = (ACC_PTR) NULL;
	return (acc_ptr);
}

/*
 *****************************************************************************
 *	Function	:	PeriodAlloc ()
 *****************************************************************************
 *	Description	:	Allocate slot in tree for new period.
 *
 *	Notes		:	Prd_alloc is used to allocate memory for new
 *				periods in consolidated posting tree.
 *			
 *	Returns		:	prd_ptr	- Pointer to new slot.
 */
static PRD_PTR	
PeriodAlloc (void)
{
	PRD_PTR	prd_ptr;

	if (! (prd_ptr = (PRD_PTR) malloc (sizeof (PRD_STRUCT))))
		file_err (-1, "PeriodAlloc", "MALLOC");

	PRD_LEFT = PRD_RIGHT = (PRD_PTR) NULL;
	return (prd_ptr);
}

/*
 *****************************************************************************
 *	Function	:	AccountSave ()
 *****************************************************************************
 *	Description	:	Allocate memory for account number.
 *
 *	Notes		:	Acc_save is used to allocate memory for new
 *					account numbers in consolidated posting tree.
 *			
 *	Returns		:	tmp_str	- Pointer to new account no.
 */
char *
AccountSave (
	char	*acctNo)
{
	char	*tmp_str;

	if (! (tmp_str = strdup (acctNo)))
		file_err (-1, "AccountSave", "STRDUP");

	return (tmp_str);
}

/*
 *****************************************************************************
 *	Function	:	AddGlwk ()
 *****************************************************************************
 *	Description	:	Create a new work record for the imbalance.
 *
 *	Parameters	:	acctNo	- Pointer to account number.
 *					prd_ptr	- Pointer to period structure.
 */
void
AddGlwk (
	char	*acctNo,
	PRD_PTR	prd_ptr)
{
	int		month;

	PRD_BALANCE	=	no_dec (PRD_BALANCE);
	PRD_LOC_BAL	=	no_dec (PRD_LOC_BAL);

	if (MoneyZero (PRD_BALANCE) && MoneyZero (PRD_LOC_BAL))
		return;

	strcpy (glmrRec.co_no, PV_co_no);
	strcpy (glmrRec.acc_no, acctNo);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");

	strcpy (workRec.acc_no, acctNo);
	strcpy (workRec.co_no, PV_co_no);
	sprintf (workRec.acronym, 	"%9.9s",  " ");
	sprintf (workRec.name, 		"%30.30s"," ");
	sprintf (workRec.chq_inv_no,"%15.15s"," ");
	sprintf (workRec.alt_desc1,"%20.20s"," ");
	sprintf (workRec.alt_desc2,"%20.20s"," ");
	sprintf (workRec.alt_desc3,"%20.20s"," ");
	workRec.ci_amt = 0.00;
	workRec.o1_amt = 0.00;
	workRec.o2_amt = 0.00;
	workRec.o3_amt = 0.00;
	workRec.o4_amt = 0.00;
	workRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (workRec.tran_type, gljcRec.journ_type);
	strcpy (workRec.sys_ref, "          ");
	workRec.tran_date = PRD_DATE;
	DateToDMY (PRD_DATE, NULL, &month, NULL);
	sprintf (workRec.period_no, "%2d", month);
	workRec.post_date = TodaysDate ();
	strcpy (workRec.narrative, "Susp. acct. posting ");
	strcpy (workRec.user_ref, 
			(PRD_STAT == 'A') ? "Account Deleted" : "Adjustment");
	if (PRD_LOC_BAL < 0)
	{
		workRec.amount 		= -PRD_BALANCE;
		workRec.loc_amount 	= -PRD_LOC_BAL;
		strcpy (workRec.currency, PRD_CURR);
		workRec.exch_rate = PRD_RATE;
		strcpy (workRec.jnl_type, "2");
	}
	else
	{
		workRec.amount 		= PRD_BALANCE;
		workRec.loc_amount 	= PRD_LOC_BAL;
		strcpy (workRec.currency, PRD_CURR);
		workRec.exch_rate = PRD_RATE;
		strcpy (workRec.jnl_type, "1");
	}
	strcpy (workRec.stat_flag, "0");
	strcpy (workRec.run_no, glwkRec.run_no);

	cc = RF_ADD (glwk_no, (char *) &workRec);
	if (cc)
		file_err (cc, "glwk_no", "WKADD");
	
}

static int
MoneyZero (
	double	m)
{
	return (fabs (m) < 0.0001);
}
