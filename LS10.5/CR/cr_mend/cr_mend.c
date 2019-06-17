/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_mend.c,v 5.4 2001/12/06 07:05:13 scott Exp $
|  Program Name  : (cr_mend.c)
|  Program Desc  : (Suppliers end month end processing)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/05/91         |
|---------------------------------------------------------------------|
| $Log: cr_mend.c,v $
| Revision 5.4  2001/12/06 07:05:13  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/06 02:38:49  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_mend.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_mend/cr_mend.c,v 5.4 2001/12/06 07:05:13 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND		5
#define	BAD_ERROR	-1

#define	CO_CRD		 (envCoClose [1] == '1')

#include	"schema"

struct commRecord	comm_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;

	char	*mect2	=	"mect2";
	
	int		printerNumber = 1;
	char	envCoClose [6];

/*===========================
| Local function prototypes |
===========================*/
void	UpdateStatus	 (int);
void	shutdown_prog	 (void);
void	CloseDB			 (void);
void	OpenDB			 (void);
int		ErrorLog		 (char *, char *, int, int);
int		AddMect			 (void);
int		AllClosed		 (char *);

/*==========================
| Main Processing Routine. | 
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;
	int		mth;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	printerNumber = atoi (get_env ("MEND_LP"));

	init_scr ();

	/*
	 * Read month End Database File.
	 */
	OpenDB ();

	dsp_screen ("Suppliers Month End Close.",
				comm_rec.co_no, comm_rec.co_name);

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_CRD) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "CR");
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
	{
		AddMect ();
		strcpy (mect_rec.co_no, comm_rec.co_no);
		strcpy (mect_rec.br_no, (CO_CRD) ? "  " : comm_rec.est_no);
		strcpy (mect_rec.module_type, "CR");
		cc = find_rec (mect, &mect_rec, COMPARISON, "r");
	}

	/*
	 * Get current month.
	 */
	DateToDMY (comm_rec.crd_date, NULL, &mth, NULL);
	if ((mth - 1) != (mect_rec.closed_mth) % 12)
	{
		if (ErrorLog ("cr_mend", "MONTH END CLOSE ALREADY RUN.",-1, TRUE) == -1)
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
		{
			if (ErrorLog (mect_rec.txt_file,"User defined reports.",
							errno, FALSE) == -1)
				return (EXIT_FAILURE);
		}
	}

	/*
	 * Month end purge for Suppliers.
	 */
	dsp_screen ("Executing Suppliers month end purge.",
				comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("cr_dpurge"))
	{
		if (ErrorLog ("cr_dpurge","Suppliers month end purge.",
						errno, TRUE) == -1)
			return (EXIT_FAILURE);

		UpdateStatus (ERR_FND);
	}

	/*
	 * Month end close for suppliers.
 	 */
	dsp_screen ("Executing Suppliers month end close.",
				comm_rec.co_no, comm_rec.co_name);

	if (sys_exec ("cr_lclose"))
	{
		if (ErrorLog ("cr_lclose", "Suppliers month end close.",
						errno, TRUE) == -1)
			return (EXIT_FAILURE);

		UpdateStatus (ERR_FND);
	}

	/*
	 * Check if all other branches closed.
	 */
	if (AllClosed ("CR"))
	{
		/*
		 * Closing Customer/GL total clearing.
		 */
		dsp_screen ("Clearing out suppliers / GL control totals.",
					comm_rec.co_no, comm_rec.co_name);

		if (sys_exec ("cr_gl_eom"))
		{
			if (ErrorLog ("cr_gl_eom","Clearing suppliers / GL control totals.", errno, TRUE) == -1)
				return (EXIT_FAILURE);

			UpdateStatus (ERR_FND);
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

	abc_alias (mect2, mect);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (mect, mect_list, MECT_NO_FIELDS, "mect_id_no");
	open_rec (mect2, mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

/*
 * Log errors found.
 */
int
ErrorLog (
	char	*progname,
	char	*progdesc,
	int		errNo,
	int		aborted)
{
	FILE     *pp;

	if ((pp = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (pp,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp,".LP%d\n",printerNumber);
	fprintf (pp,".%d\n", (CO_CRD) ? 10 : 11);
	fprintf (pp,".L132\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".ESUPPLIER MONTH CLOSE ERROR REPORT \n");
	fprintf (pp,".B1\n");
	fprintf (pp,".E Company %s - %s\n", comm_rec.co_no, 
			clip (comm_rec.co_name));

	if (!CO_CRD)
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

	fprintf (pp, "Suppliers month end close has the following errors\n\r");
	fprintf (pp, "The error description is   : %s\n",progdesc);
	fprintf (pp, "The program running was    : %s\n",progname);
	fprintf (pp, "The error code returned is : %d\n", errNo);
	if (aborted)
		fprintf (pp,"NOTE : The month close for suppliers WAS TERMINATED.\n");
	else
		fprintf (pp,"NOTE : The month close for suppliers WILL CONTINUE.\n");

	fprintf (pp,".EOF\n");
	pclose (pp);

	if (aborted)
	{
		shutdown_prog ();
		return (-1);
	}

	return (EXIT_SUCCESS);
}

/*
 * Add mect record as no record exists.
 */
int
AddMect (void)
{
	int		month;

	DateToDMY (comm_rec.crd_date, NULL, &month, NULL);

	month--;
	if (month == 0)
		month = 12;

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (CO_CRD) ? "  " : comm_rec.est_no);
	strcpy (mect_rec.module_type, "CR");
	mect_rec.status = 0;
	strcpy (mect_rec.start_time, "00:00");
	strcpy (mect_rec.end_time  , "00:00");
	mect_rec.closed_mth = month;
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
	char	*type)
{
	int		branchOpen 	= 0,
			branchClose = 0;

	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", type);
	cc = find_rec (mect2, &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		       !strcmp (mect2_rec.module_type, type))
	{
		if (mect2_rec.status == OPEN)
			branchOpen++;

		if (mect2_rec.status == CLOSING)
			branchClose++;

		cc = find_rec (mect2, &mect2_rec, NEXT, "r");
	}
	if (!branchOpen && branchClose < 2)
		return (TRUE);
	
	return (FALSE);
}

