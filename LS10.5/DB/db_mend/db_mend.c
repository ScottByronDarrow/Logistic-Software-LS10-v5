/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_mend.c,v 5.3 2001/11/27 08:40:39 scott Exp $
|  Program Name  : (db_mend.c)
|  Program Desc  : (Customer End Of Month Processing Program) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/05/91         |
|---------------------------------------------------------------------|
| $Log: db_mend.c,v $
| Revision 5.3  2001/11/27 08:40:39  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_mend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_mend/db_mend.c,v 5.3 2001/11/27 08:40:39 scott Exp $";

extern	int	errno;

#include	<pslscr.h>
#include	<time.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND		5
#define	BAD_ERROR	-1

#define	CO_DBT		 (envCoClose [0] == '1')

	struct	tm *ts;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;

	char	*mect2	= "mect2";

	int		envDbCo = 0,
			envMendLp = 1;

	char	envCoClose [6];

/*
 * Local Function Prototypes.
 */
void 	UpdateStatus 		(int);
void 	shutdown_prog 		(void);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	UpdateStatementDate (void);
int 	ErrorLog 			(char *, char *, int, int);
int 	AddMect 			(void);
int 	AllClosed 			(char *);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	int		monthPeriod;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	envMendLp = atoi (get_env ("MEND_LP"));

	init_scr ();

	/*
	 * Read month End Database File.
	 */
	OpenDB ();

	UpdateStatementDate ();

	if (CO_DBT)
	{
		dsp_screen ("Running Customer Company Month End Close.",
				comm_rec.co_no, comm_rec.co_name);
	}
	else
	{
		sprintf (err_str, "Running Customer Close for %s - %s",
				comm_rec.est_no, clip (comm_rec.est_name));

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_DBT) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "DB");
	if (find_rec (mect, &mect_rec, COMPARISON, "u"))
	{
		AddMect ();
		strcpy (mect_rec.co_no, comm_rec.co_no);
		strcpy (mect_rec.br_no, (CO_DBT) ? "  " : comm_rec.est_no);
		strcpy (mect_rec.module_type, "DB");
		cc = find_rec (mect, &mect_rec, COMPARISON, "u");
	}

	/*
	 * Get current month.
	 */
	DateToDMY (comm_rec.dbt_date, NULL, &monthPeriod, NULL);
	if ((monthPeriod - 1) != (mect_rec.closed_mth) % 12)
    {
		if (ErrorLog ("db_mend", "MONTH END CLOSE ALREADY RUN.", -1, TRUE) == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	UpdateStatus (CLOSING);

	/*
	 * get pointer to time & structure.
	 */
	strcpy (mect_rec.start_time, TimeHHMM ());

	/*
	 * Month end close for user defined reports.
	 */
	dsp_screen ("Now Executing User Defined Reports.",
			comm_rec.co_no, comm_rec.co_name);

	if (strcmp (mect_rec.txt_file, "                              "))
	{
		if (sys_exec (mect_rec.txt_file))
        {
            if (ErrorLog (mect_rec.txt_file,
                       "User defined reports.",
                       errno,
                       FALSE) == 1)
            {
                return (EXIT_FAILURE);
            }
        }
	}
	/*
	 * Month end close for Sales analysis.
	 */
	dsp_screen ("Now Executing End Of Month S/A Close",
			comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("sa_close"))
	{
		if (1 == ErrorLog ("sa_close",
                            "End Of Month S/A Close.",
                            errno,
                            TRUE))
        {
            return (EXIT_FAILURE);
        }
		UpdateStatus (ERR_FND);
	}

	/*
	 * Month end close for customers.
	 */
	dsp_screen ("Now Executing End Of Month Customer Close.",
			comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("db_lclose"))
	{
		if (1 == ErrorLog ("db_lclose",
                            "End Of Month Customer Close.",
                            errno,
                            TRUE))
        {
            return (EXIT_FAILURE);
        }
		UpdateStatus (ERR_FND);
	}

	/*
	 * Month end customers purge.
	 */
	dsp_screen ("Now Executing End Of Month Customer Purge.",
			comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("db_dpurge"))
	{
		if (1 == ErrorLog ("db_dpurge",
                            "End Of Month Customer Purge.",
                            errno,
                            TRUE))
        {
            return (EXIT_FAILURE);
        }
		UpdateStatus (ERR_FND);
	}

	/*
	 * Check if all other branches closed.
	 */
	if (AllClosed ("DB"))
	{
		/*
		 * Closing Customer/GL total clearing.
		 */
		dsp_screen ("Clearing out customers / GL control totals.",
					comm_rec.co_no, comm_rec.co_name);

		if (sys_exec ("db_gl_eom"))
		{
			if (1 == ErrorLog ("db_gl_eom",
                                "Clearing customers/ GL control totals.",
                                errno,
                                TRUE))
            {
                return (EXIT_FAILURE);
            }
			UpdateStatus (ERR_FND);
		}
	}
	/*
	 * get pointer to time & structure.
	 */
	strcpy (mect_rec.end_time, TimeHHMM ());

	mect_rec.status = CLOSED;

	mect_rec.closed_mth = monthPeriod;
	cc = abc_update (mect, &mect_rec);
	if (cc)
		file_err (cc, mect, "DBUPDATE");

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Update status for mect_status.
 */
void
UpdateStatus (
 int                _cur_status)
{
	mect_rec.status = _cur_status;

	cc = abc_update (mect, &mect_rec);
	if (cc)
		file_err (cc, mect, "DBUPDATE");
}

/*
 * Exit program with correct status.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
    if (CO_DBT)
    {
	    abc_fclose (comr);
    }
    else
    {
	    abc_fclose (esmr);
    }
	abc_fclose (mect);
	abc_fclose (mect2);
	abc_dbclose ("data");
}

/*
 * Open data base files
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	abc_alias (mect2, mect);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (CO_DBT)
		open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	else
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (mect, mect_list, MECT_NO_FIELDS, "mect_id_no");
	open_rec (mect2, mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

void
UpdateStatementDate (void)
{
	if (CO_DBT)
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");
		comr_rec.stmt_date = comm_rec.dbt_date;
		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");
	}
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");
		esmr_rec.stmt_date = comm_rec.dbt_date;
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");
	}
}

/*
 * Log errors found.
 */
int
ErrorLog (
	char	*progname,
	char	*progdesc,
	int     err_num,
	int     aborted)
{
	FILE	*pp;

	if ((pp = popen ("pformat", "w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp, ".LP%d\n", envMendLp);
	fprintf (pp, ".%d\n", (CO_DBT) ? 10 : 11);
	fprintf (pp, ".L132\n");
	fprintf (pp, ".B1\n");
	fprintf (pp, ".ECUSTOMER MONTH CLOSE ERROR REPORT \n");
	fprintf (pp, ".B1\n");
	fprintf (pp, ".E Company %s - %s\n", comm_rec.co_no,
			clip (comm_rec.co_name));
	if (!CO_DBT)
	{
		fprintf (pp, ".E Branch %s - %s\n", comm_rec.est_no,
						clip (comm_rec.est_name));
	}
	fprintf (pp, ".B1\n");
	fprintf (pp, ".EAS AT : %s\n", SystemTime ());
	fprintf (pp, ".B1\n");

	fprintf (pp, ".R===========================================");
	fprintf (pp, "============================================");
	fprintf (pp, "============================================\n");

	fprintf (pp, ".B2\n");

	fprintf (pp, "Customer month end close has the following errors\n\r");
	fprintf (pp, "The error description is   : %s\n", progdesc);
	fprintf (pp, "The program running was    : %s\n", progname);
	fprintf (pp, "The error code returned is : %d\n", err_num);
	if (aborted)
		fprintf (pp, "NOTE : The month close for customers WAS TERMINATED.\n");
	else
		fprintf (pp, "NOTE : The month close for customers WILL CONTINUE.\n");

	fprintf (pp, ".EOF\n");
	pclose (pp);

	if (aborted)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
    else
    {
        return (EXIT_SUCCESS);
    }
}

/*
 * Add mect record as no record exists.
 */
int
AddMect (void)
{
	int		monthPeriod;

	DateToDMY (comm_rec.dbt_date, NULL, &monthPeriod, NULL);

	monthPeriod--;
	if (monthPeriod == 0)
		monthPeriod = 12;

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_DBT) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "DB");
	mect_rec.status = 0;
	strcpy (mect_rec.start_time, 	"00:00");
	strcpy (mect_rec.end_time, 	"00:00");
	mect_rec.closed_mth = monthPeriod;
	strcpy (mect_rec.txt_file, "                              ");
	strcpy (mect_rec.prog_stat, "0");
	cc = abc_add (mect, &mect_rec);
	if (cc)
		file_err (cc, mect, "DBADD");

	return (cc);
}

/*
 * Check if all records for module type / company are closed.
 */
int
AllClosed (
	char	*closeType)
{
	int	br_open = 0,
		br_closing = 0;

	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", closeType);
	cc = find_rec (mect2, &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		!strcmp (mect2_rec.module_type, closeType))
	{
		if (mect2_rec.status == OPEN)
			br_open++;

		if (mect2_rec.status == CLOSING)
			br_closing++;

		cc = find_rec (mect2, &mect2_rec, NEXT, "r");
	}
	if (!br_open && br_closing < 2)
		return (TRUE);

	return (FALSE);
}
