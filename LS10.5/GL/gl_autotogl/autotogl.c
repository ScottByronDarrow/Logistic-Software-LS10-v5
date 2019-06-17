/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: autotogl.c,v 5.3 2001/08/09 09:13:26 scott Exp $
|  Program Name  : (gl_autotogl.c)
|  Program Desc  : (Create Standing Portings Accourding to glaj)
|                  (file Created by gl_autoinp.c)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: autotogl.c,v $
| Revision 5.3  2001/08/09 09:13:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:06  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:31  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: autotogl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_autotogl/autotogl.c,v 5.3 2001/08/09 09:13:26 scott Exp $";

/*
 * Include file dependencies
 */

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_gl_mess.h>

/*
 * Constants, defines and stuff.
 */
char    *data = "data";

#include	"schema"

struct commRecord	comm_rec;
struct glajRecord	glaj_rec;

	/*
	 *   Special fields and flags   
	 */
	int		day1 		= 0,
			dayToApply 	= 0,
			printerNo 	= 1,
			gdmy [3];

	char	gldate [11];

/*
 * Local function prototypes 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	Update 			(void);
void 	ResetGlaj 		(void);
	
/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char 	*argv [])
{
	int		dmy [3];

	if (argc < 2)
	{
		print_at (0,0, mlStdMess036,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNo = atoi (argv [1]);

	OpenDB ();
	dsp_screen
	(
		"Processing And Posting Standing Jounals.",
		comm_rec.co_no,
		comm_rec.co_name
	);

	/*
	 * Process General ledger Standing Journal records.  
	 */
	strcpy (glaj_rec.co_no, comm_rec.co_no);

    cc = find_rec (glaj, &glaj_rec, GTEQ, "u");
	while (!cc && 
           !strcmp (glaj_rec.co_no, comm_rec.co_no))
	{
		dayToApply = atoi (glaj_rec.date_appl);
		DateToDMY (glaj_rec.ef_frm, &dmy [0],&dmy [1],&dmy [2]);
		day1 = dmy [0];

		if (day1 > 28)
			day1 = 28;

		if (dayToApply < day1)
			glaj_rec.ef_frm += 28;

		DateToDMY (glaj_rec.ef_to, &dmy [0],&dmy [1],&dmy [2]);
		day1 = dmy [0];
		if (day1 > 28)
			day1 = 28;

		if (dayToApply > day1)
			glaj_rec.ef_to -= 28;

		if ((glaj_rec.stat_flag [0] != '0') ||
            (comm_rec.gl_date < glaj_rec.ef_frm) ||
            (comm_rec.gl_date > glaj_rec.ef_to))
		{
			abc_unlock (glaj);
			cc = find_rec (glaj, &glaj_rec, NEXT, "u");
			continue;
		}

		/*
		 * Create Or Update glwk Record.
		 */
		if (dayToApply <= gdmy [0] || argc > 2)
			Update ();
		else
			abc_unlock (glaj);

		cc = find_rec (glaj, &glaj_rec, NEXT, "u");
	}

	if (argc > 2)
		ResetGlaj ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

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

	strcpy (gldate, DateToString (comm_rec.gl_date));
	DateToDMY (comm_rec.gl_date, &gdmy [0], &gdmy [1], &gdmy [2]);

	open_rec (glaj, glaj_list, GLAJ_NO_FIELDS, "glaj_co_no");

	OpenGlmr ();

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close Data Base Files And Database.
 */
void
CloseDB (void)
{
	abc_fclose (glaj);
	GL_CloseBatch (printerNo);
	GL_Close ();

	abc_dbclose (data);
}

/*
 * Create Work File For Standing Journals. 
 */
int
Update (void)
{
	strcpy (glwkRec.co_no, comm_rec.co_no);
	glwkRec.tran_date 	= comm_rec.gl_date;
	glwkRec.post_date 	= TodaysDate ();
	glwkRec.amount 		= glaj_rec.orig_amt;
	glwkRec.loc_amount 	= glaj_rec.loc_amt;
	glwkRec.exch_rate 	= glaj_rec.exch_rate;
	strcpy(glwkRec.currency, glaj_rec.curr_code);
	strcpy (glwkRec.user_ref, glaj_rec.user_ref);
	strcpy (glwkRec.narrative, "Automatic Journal.");
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");
	strcpy (glwkRec.stat_flag,"2");
	strcpy (glwkRec.tran_type, glaj_rec.type);
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	sprintf (glwkRec.period_no, "%02d", gdmy [1]);
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, glaj_rec.dbt_acc_no);

    cc = find_rec (glmr, &glmrRec, COMPARISON, "u");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.acc_no, glaj_rec.dbt_acc_no);
	strcpy (glwkRec.jnl_type, "1");

	GL_AddBatch ();

	dsp_process ("Account #", glaj_rec.dbt_acc_no);
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, glaj_rec.crd_acc_no);

    cc = find_rec (glmr, &glmrRec, COMPARISON, "u");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.acc_no, glaj_rec.crd_acc_no);
	strcpy (glwkRec.jnl_type, "2");

	GL_AddBatch ();

	strcpy (glaj_rec.stat_flag, "1");
	glaj_rec.date_lproc = comm_rec.gl_date;

    cc = abc_update (glaj, &glaj_rec);
	if (cc)
		file_err (cc, glaj, "DBUPDATE");

	abc_unlock (glaj);

	dsp_process ("Account #", glaj_rec.crd_acc_no);

	return (EXIT_SUCCESS);
}

/*
 * Reset Standing Journal Flags Ready For New Month. 
 */
void
ResetGlaj (void)
{
	dsp_screen ("Reseting Standing Journals.",
                comm_rec.co_no,
                comm_rec.co_name);
	/*
	 * Process General Ledger Standing Journal Records.
	 */
	strcpy (glaj_rec.co_no, comm_rec.co_no);

    cc = find_rec (glaj, &glaj_rec, GTEQ, "u");
	while (!cc && 
           !strcmp (glaj_rec.co_no, comm_rec.co_no))
	{
		strcpy (glaj_rec.stat_flag, "0");
		
        cc = abc_update (glaj, &glaj_rec);
		if (cc)
			file_err (cc, glaj, "DBUPDATE");

		dsp_process ("Account #", glaj_rec.dbt_acc_no);
		cc = find_rec (glaj, &glaj_rec, NEXT, "u");
	}
}

/* [ end of file ] */

