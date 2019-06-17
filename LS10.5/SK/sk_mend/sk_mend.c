/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_mend.c,v 5.3 2001/12/12 01:22:26 scott Exp $
|  Program Name  : (sk_mend.c) 
|  Program Desc  : (Stock end month end processing)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/05/91         |
|---------------------------------------------------------------------|
| $Log: sk_mend.c,v $
| Revision 5.3  2001/12/12 01:22:26  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_mend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_mend/sk_mend.c,v 5.3 2001/12/12 01:22:26 scott Exp $";

extern	int	errno;

#include	<pslscr.h>
#include 	<time.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND		5
#define	BAD_ERROR	-1

#define	CO_INV		 (envCoClose [2] == '1')

#include	"schema"

struct commRecord	comm_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;

	char	*mect2	=	"mect2";

	int		envDbCo = 0,
			printerNo = 1;

	char	envCoClose [6];

/*
 * Function Declarations 
 */
void 	UpdateStatus 	(int);
void 	shutdown_prog 	(void);
void 	CloseDB 		(void);
void 	OpenDB 			(void);
void 	ErrorLog 		(char *, char *, int, int);
void 	AddMect 		(void);
int  	AllClosed 		(char *);


/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char 	*argv [])
{
	char	*sptr;
	int	mth;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	printerNo = atoi (get_env ("MEND_LP"));

	init_scr ();	

	/*
	 * Read month End Database File.
	 */
	OpenDB ();

	if (CO_INV)
	{
		dsp_screen ("Running Inventory Company Month End Close.",
				comm_rec.co_no, comm_rec.co_name);
	}
	else
	{
		sprintf (err_str, "Running Inventory Close for %s - %s",
				comm_rec.est_no, clip (comm_rec.est_name)); 

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_INV) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "SK");
	if (find_rec (mect, &mect_rec, COMPARISON, "u"))
	{
		AddMect ();
		strcpy (mect_rec.co_no, comm_rec.co_no);
		strcpy (mect_rec.br_no, (CO_INV) ? "  " : comm_rec.est_no);
		strcpy (mect_rec.module_type, "SK");
		cc = find_rec (mect, &mect_rec, COMPARISON, "u");
	}

	/*
	 * Get current month.
	 */
	DateToDMY (comm_rec.inv_date, NULL, &mth, NULL);
    if ((mth - 1) != (mect_rec.closed_mth) % 12) 
	{
		ErrorLog ("sk_mend","MONTH END CLOSE ALREADY RUN.",-1, TRUE);
        shutdown_prog ();
        return (EXIT_FAILURE);
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
			ErrorLog (mect_rec.txt_file,"User defined reports.",
							errno, FALSE);
	}

	/*
	 * Month end close for Stock close.
	 */
	dsp_screen ("Now Executing End Of Month Stock Close.",
			comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("sk_close"))
	{
		ErrorLog ("sk_close","End Of Month Stock Close.", errno, TRUE);
		UpdateStatus (ERR_FND);
        shutdown_prog ();
        return (EXIT_FAILURE);
	}

	/*
	 * Check if all other branches closed.
	 */
	if (AllClosed ("SK"))
	{
		/*
		 * Closing Customer/GL total clearing.
		 */
		dsp_screen ("Clearing out stock / GL control totals.",
					comm_rec.co_no, comm_rec.co_name);

		if (sys_exec ("sk_gl_eom"))
		{
			ErrorLog ("sk_gl_eom","Clearing Stock/ GL control totals.", errno, TRUE);
			UpdateStatus (ERR_FND);
            shutdown_prog ();
            return (EXIT_FAILURE);
		}
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
	int		currentStatus)
{
	mect_rec.status = currentStatus;

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
	abc_fclose (mect2);
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

	abc_alias (mect2, mect);
	open_rec (mect2, mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

/*
 * Log errors found.
 */
void
ErrorLog (
	char	*progname,
	char	*progdesc,
	int		err_num,
	int		aborted)
{
	FILE     *pp;

	if ((pp = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");


	fprintf (pp,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".%d\n", (CO_INV) ? 10 : 11);
	fprintf (pp,".L132\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".EINVENTORY MONTH CLOSE ERROR REPORT \n");
	fprintf (pp,".B1\n");
	fprintf (pp,".E Company %s - %s\n", comm_rec.co_no, comm_rec.co_name);
	if (!CO_INV)
	{
		fprintf (pp,".E Branch %s - %s\n", comm_rec.est_no, comm_rec.est_name);
	}
	
	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n", SystemTime ());
	fprintf (pp,".B1\n");

	fprintf (pp,".R===========================================");
	fprintf (pp,"============================================");
	fprintf (pp,"============================================\n");

	fprintf (pp,".B2\n");

	fprintf (pp, "Inventory month end close has the following errors\n\r");
	fprintf (pp, "The error description is   : %s\n",progdesc);
	fprintf (pp, "The program running was    : %s\n",progname);
	fprintf (pp, "The error code returned is : %d\n", err_num);
	if (aborted)
		fprintf (pp,"NOTE : The month close for inventory WAS TERMINATED.\n");
	else
		fprintf (pp,"NOTE : The month close for inventory WILL CONTINUE.\n");

	fprintf (pp,".EOF\n");
	pclose (pp);
}
/*
 * Add mect record as no record exists.
 */
void
AddMect (void)
{
	int		_mth; 

	DateToDMY (comm_rec.inv_date, NULL, &_mth, NULL);

	_mth--;
	if (_mth == 0)
		_mth = 12;

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_INV) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "SK");
	strcpy (mect_rec.start_time, "00:00");
	strcpy (mect_rec.end_time  , "00:00");
	strcpy (mect_rec.txt_file,"                              ");
	strcpy (mect_rec.prog_stat, "0");
	mect_rec.closed_mth = _mth;
	mect_rec.status 	= 0;
	cc = abc_add (mect, &mect_rec);
	if (cc)
		file_err (cc, mect, "DBADD");
}

/*
 * Check if all records for module type / company are closed.
 */
int
AllClosed (
	char	*moduleType)
{
	int		branchOpen = 0,
			branchClosing = 0;

	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", moduleType);
	cc = find_rec (mect2, &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		          !strcmp (mect2_rec.module_type, moduleType))
	{
		if (mect2_rec.status == OPEN)
			branchOpen++;

		if (mect2_rec.status == CLOSING)
			branchClosing++;

		cc = find_rec (mect2, &mect2_rec, NEXT, "r");
	}
	if (!branchOpen && branchClosing < 2)
		return (TRUE);
	
	return (FALSE);
}

