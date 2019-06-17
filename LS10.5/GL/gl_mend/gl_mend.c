/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_mend.c,v 5.3 2001/08/09 09:13:50 scott Exp $
|  Program Name  : (gl_mend.c)
|  Program Desc  : (General Ledger Month Processing Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/05/91         |
|---------------------------------------------------------------------|
| $Log: gl_mend.c,v $
| Revision 5.3  2001/08/09 09:13:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:26  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:52  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_mend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_mend/gl_mend.c,v 5.3 2001/08/09 09:13:50 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND 	5
#define	BAD_ERROR	-1

#define	CO_GEN		 (envVarCoClose [4] == '1')

	struct 	tm *ts;

#include	"schema"

struct commRecord	comm_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;

	int		printerNo = 1;

	long	progPid;
	char	envVarCoClose [6];

/*
 * Local Function Prototypes.
 */
void 	UpdateStatus 	(int);
void 	shutdown_prog 	(void);
void 	CloseDB 		(void);
void 	OpenDB 			(void);
int 	ErrorLog 		(char *, char *, int, int);
int 	AddMect 		(void);
int 	AllClosed 		(char *);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	int		mth;

	progPid = (long) getpid ();

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envVarCoClose,"%-5.5s","11111");
	else
		sprintf (envVarCoClose,"%-5.5s",sptr);

	printerNo = atoi (get_env ("MEND_LP"));

	init_scr ();	

	/*
	 * Read month End Database File.
	 */
	OpenDB ();

	/*
	 * Read common terminal record.
	 */

	if (CO_GEN)
	{
		dsp_screen ("Running General Ledger Month End Close.",
				comm_rec.co_no, comm_rec.co_name);
	}
	else
	{
		sprintf (err_str, "General Ledger Close for %s - %s",
				comm_rec.est_no, clip (comm_rec.est_name)); 

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_GEN) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "GL");
		
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
	{
		AddMect ();
		strcpy (mect_rec.co_no, comm_rec.co_no);
		strcpy (mect_rec.br_no, (CO_GEN) ? "  " : comm_rec.est_no);
		strcpy (mect_rec.module_type, "GL");
		cc = find_rec (mect, &mect_rec, COMPARISON, "r");
	}

	/*
	 * Get current month.
	 */
	DateToDMY (comm_rec.gl_date, NULL, &mth, NULL);

	if ((mth - 1) != (mect_rec.closed_mth) % 12)
    {
		if (ErrorLog ("gl_mend","MONTH END CLOSE ALREADY RUN.",-1,TRUE) == 1)
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
			if (ErrorLog (mect_rec.txt_file,"User defined reports.",
                           errno, FALSE) == 1)
            {
                return (EXIT_FAILURE);
            }
        }
	}
	
	/*
	 * Month end close for Sales analysis.
	 */
	dsp_screen ("Now Executing Automatic Journal Update.",
			comm_rec.co_no, comm_rec.co_name);

	sprintf (err_str, "gl_autotogl %d",  printerNo);
	if (!sys_exec (err_str))
	{
		sprintf (err_str, "gl_postbat %ld %d \" 2\"", progPid, printerNo);
		if (sys_exec (err_str))
		{
			if (ErrorLog ("gl_autotogl","Automatic journal update.",
                       errno, FALSE) == 1)
            {
                return (EXIT_FAILURE);
            }
        }
		UpdateStatus (ERR_FND);
	}

	if (sys_exec ("gl_autotogl 1 EOM"))
	{
		if (ErrorLog ("gl_autotogl","Automatic journal update.",
                       errno, FALSE) == 1)
        {
            return (EXIT_FAILURE);
        }
		UpdateStatus (ERR_FND);
	}

	/*
	 * Month end close for debtors.
	 */
	dsp_screen ("Executing General Ledger month end clode.",
				comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("gl_eom"))
	{
		if (ErrorLog ("gl_eom","General Ledger month end close.",
                       errno, TRUE) == 1)
        {
            return (EXIT_FAILURE);
        }
		UpdateStatus (ERR_FND);
	}
	/*
	 * get pointer to time & structure.
	 */
	strcpy (mect_rec.end_time, TimeHHMM ());

	mect_rec.status = CLOSED;

	mect_rec.closed_mth = mth;
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
	abc_fclose (mect);
	abc_fclose ("mect2");
	abc_dbclose ("data");
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (mect, mect_list, MECT_NO_FIELDS, "mect_id_no");

	abc_alias ("mect2", mect);
	open_rec ("mect2", mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

/*
 * Log errors found.
 */
int
ErrorLog (
 char*              progname,
 char*              progdesc,
 int                err_num,
 int                aborted)
{
	FILE     *pp;

	if ((pp = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".%d\n", (CO_GEN) ? 10 : 11);
	fprintf (pp,".L132\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".EGENERAL LEDGER MONTH CLOSE ERROR REPORT \n");
	fprintf (pp,".B1\n");
	fprintf (pp,".E Company %s - %s\n", comm_rec.co_no, 
			clip (comm_rec.co_name));
	if (!CO_GEN)
	{
		fprintf (pp,".E Branch %s - %s\n", comm_rec.est_no, 
						clip (comm_rec.est_name));
	}
	
	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n", SystemTime ());
	fprintf (pp,".B1\n");

	fprintf (pp,".R===========================================");
	fprintf (pp,"============================================");
	fprintf (pp,"============================================\n");

	fprintf (pp,".B2\n");

	fprintf (pp, "General ledger month end close has the following errors\n\r");
	fprintf (pp, "The error description is   : %s\n",progdesc);
	fprintf (pp, "The program running was    : %s\n",progname);
	fprintf (pp, "The error code returned is : %d\n", err_num);
	if (aborted)
		fprintf (pp,"NOTE : The month close for general ledger WAS TERMINATED.\n");
	else
		fprintf (pp,"NOTE : The month close for general ledger WILL CONTINUE.\n");

	fprintf (pp,".EOF\n");
	pclose (pp);

	if (aborted)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

/*
 * Add mect record as no record exists.
 */
int
AddMect (void)
{
	int		_mth;

	DateToDMY (comm_rec.gl_date, NULL, &_mth, NULL);
	_mth--;
	if (_mth == 0)
		_mth = 12;

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_GEN) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "GL");
	mect_rec.status = 0;
	strcpy (mect_rec.start_time, "00:00");
	strcpy (mect_rec.end_time  , "00:00");
	mect_rec.closed_mth = _mth;
	strcpy (mect_rec.txt_file,"                              ");
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
 char*              _type)
{
	int	br_open = 0,
		br_closing = 0;

	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", _type);
	cc = find_rec ("mect2", &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		       !strcmp (mect2_rec.module_type, _type))
	{
		if (mect2_rec.status == OPEN)
			br_open++;

		if (mect2_rec.status == CLOSING)
			br_closing++;

		cc = find_rec ("mect2", &mect2_rec, NEXT, "r");
	}
	if (!br_open && br_closing < 2)
		return (TRUE);
	
	return (FALSE);
}

