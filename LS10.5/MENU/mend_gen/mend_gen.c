/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: mend_gen.c,v 5.4 2002/03/08 04:45:13 scott Exp $
|  Program Name  : (mend_gen.c)
|  Program Desc  : (Generate month end processing files)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/05/91         |
|---------------------------------------------------------------------|
| $Log: mend_gen.c,v $
| Revision 5.4  2002/03/08 04:45:13  scott
| .
|
| Revision 5.3  2002/03/08 04:43:37  scott
| Updated to use app.schema and to create blank report files in SCRIPT.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mend_gen.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mend_gen/mend_gen.c,v 5.4 2002/03/08 04:45:13 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_menu_mess.h>
#include 	<ml_std_mess.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#endif	/* GVISION */

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND 	5
#define	BAD_ERROR	-1

#define	CO_DBT		(envCoClose [0] == '1')
#define	BR_DBT		(envCoClose [0] == '0')
#define	CO_CRD		(envCoClose [1] == '1')
#define	BR_CRD		(envCoClose [1] == '0')
#define	CO_INV		(envCoClose [2] == '1')
#define	BR_INV		(envCoClose [2] == '0')
#define	CO_GEN		(envCoClose [4] == '1')
#define	BR_GEN		(envCoClose [4] == '0')

	long	tloc = - 1;

	struct 	tm *ts, *localtime (const time_t *);

	int		dbBrReport 	= FALSE, 
			crBrReport 	= FALSE, 
			skBrReport 	= FALSE, 
			glBrReport 	= FALSE, 
			envDbCo		= 0, 
			printerNo	= 1, 
			reportRun 	= FALSE, 
			pos 		= 1, 
			overide 	= FALSE;

	FILE	*pout;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct mectRecord	mect_rec;


	char	envCoClose [6];

/*
 * Main Processing Routine. 
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB			(void);
void	PrintMect		(void);
void	CreateMect		(void);
void	AddMect		 	(char *, char *, char *, long, int);
int		CheckOk			(void);
int		ShowError		(void);
void	ReportMect		(char *, char *, char *, long, int, char *);
void	ReportHeading	(void);
void	ClosePrint		(void);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc, 
 char * argv [])
{
	char	*sptr;
	
	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose, "%-5.5s", "11111");
	else
		sprintf (envCoClose, "%-5.5s", sptr);

	sptr = strrchr (argv [0], '/');
	if (sptr != (char *) 0)
		sptr++;
	else 
		sptr = argv [0];

	if (!strncmp (sptr, "mend_prt", 8))
		reportRun = TRUE;
	else
		reportRun = FALSE;

	if (argc > 1)
		printerNo = atoi (argv [1]);
	else
	{
		sptr = chk_env ("MEND_LP");
		if (sptr == (char *)0)
			printerNo = 1;
		else
			printerNo = atoi (sptr);
	}

	init_scr ();	

	set_tty ();

	/*
	 * Read month End Database File. 
	 */
	OpenDB ();

	if (reportRun)
	{
		dsp_screen ("Printing month end check list.", 
					comm_rec.co_no, comm_rec.co_name);

		PrintMect ();
	}
	else
	{
		if (!CheckOk ())
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}

		dsp_screen ("Generating month end control files.", 
					comm_rec.co_no, comm_rec.co_name);

		CreateMect ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
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
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (mect, mect_list, MECT_NO_FIELDS, "mect_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (mect);
	abc_dbclose ("data");
}


void
PrintMect (void)
{
	ReportHeading ();

	strcpy (comr_rec.co_no, comm_rec.co_no);

	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (CO_DBT)
	{
		ReportMect (comm_rec.co_no, "  ", "DB", 
			comr_rec.dbt_date, dbBrReport, comm_rec.co_short);
		fprintf (pout, "|----|----|-------------------");
		fprintf (pout, "|-------------|---------|------------");
		fprintf (pout, "|-----------|-------------|--------|\n");
	}

	if (CO_CRD)
	{
		ReportMect (comm_rec.co_no, "  ", "CR", 
			comr_rec.crd_date, crBrReport, comm_rec.co_short);
		fprintf (pout, "|----|----|-------------------");
		fprintf (pout, "|-------------|---------|------------");
		fprintf (pout, "|-----------|-------------|--------|\n");
	}

	if (CO_INV)
	{
		ReportMect (comm_rec.co_no, "  ", "SK", 
			comr_rec.inv_date, skBrReport, comm_rec.co_short);
		fprintf (pout, "|----|----|-------------------");
		fprintf (pout, "|-------------|---------|------------");
		fprintf (pout, "|-----------|-------------|--------|\n");
	}

	if (CO_GEN)
	{
		ReportMect (comm_rec.co_no, "  ", "GL", 
			comr_rec.gl_date, glBrReport, comm_rec.co_short);
		fprintf (pout, "|----|----|-------------------");
		fprintf (pout, "|-------------|---------|------------");
		fprintf (pout, "|-----------|-------------|--------|\n");
	}

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		if (BR_DBT)
			ReportMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "DB", esmr_rec.dbt_date, dbBrReport, 
				  esmr_rec.short_name);
		if (BR_CRD)
			ReportMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "CR", esmr_rec.crd_date, crBrReport, 
				  esmr_rec.short_name);
		if (BR_INV)
			ReportMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "SK", esmr_rec.inv_date, skBrReport, 
				  esmr_rec.short_name);
		if (BR_GEN)
			ReportMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "GL", esmr_rec.gl_date, glBrReport, 
				  esmr_rec.short_name);

		fprintf (pout, "|----|----|-------------------");
		fprintf (pout, "|-------------|---------|------------");
		fprintf (pout, "|-----------|-------------|--------|\n");

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	ClosePrint ();
}

void
CreateMect (void)
{
	strcpy (comr_rec.co_no, comm_rec.co_no);

	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (CO_DBT)
		AddMect (comm_rec.co_no, "  ", "DB", 
					comr_rec.dbt_date, dbBrReport);

	if (CO_CRD)
		AddMect (comm_rec.co_no, "  ", "CR", 
					comr_rec.crd_date, crBrReport);

	if (CO_INV)
		AddMect (comm_rec.co_no, "  ", "SK", 
					comr_rec.inv_date, skBrReport);

	if (CO_GEN)
		AddMect (comm_rec.co_no, "  ", "GL", 
					comr_rec.gl_date, glBrReport);

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		if (BR_DBT)
			AddMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "DB", esmr_rec.dbt_date, dbBrReport);
		if (BR_CRD)
			AddMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "CR", esmr_rec.crd_date, crBrReport);
		if (BR_INV)
			AddMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "SK", esmr_rec.inv_date, skBrReport);
		if (BR_GEN)
			AddMect (esmr_rec.co_no, esmr_rec.est_no, 
				  "GL", esmr_rec.gl_date, glBrReport);

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
}

/*
 * Add mect record as no record exists. 
 */
void
AddMect (
	char 	*coNo, 
	char 	*brNo, 
	char 	*moduleType, 
	long	_date, 
	int		branchRep)
{
	FILE	*textFile;

	char	bname [100];
	char	programName [100];
	char	*sptr = getenv ("PROG_PATH");

	int		_mth;
	int		new_rec;

	/*
	 * get path for menu system	
	 */
	sprintf (bname, "%s/BIN", (sptr) ? sptr : "/usr/LS10.5");

	DateToDMY (_date, NULL, &_mth, NULL);
	_mth--;
	if (_mth == 0)
		_mth = 12;

	dsp_process ("Creating Data files", moduleType);

	sprintf (mect_rec.co_no, "%-2.2s", coNo);
	sprintf (mect_rec.br_no, "%-2.2s", brNo);
	sprintf (mect_rec.module_type, "%-2.2s", moduleType);
	new_rec = find_rec (mect, &mect_rec, COMPARISON, "u");

	mect_rec.status = OPEN;
	strcpy (mect_rec.start_time, "00:00");
	strcpy (mect_rec.end_time  , "00:00");
	mect_rec.closed_mth = _mth;
	if (branchRep)
		sprintf (mect_rec.txt_file, "%-2.2s_mend.%02d%02d", 
					moduleType, atoi (coNo), atoi (brNo));
	else
		sprintf (mect_rec.txt_file, "%-2.2s_mend.%02d", 
						moduleType, atoi (coNo));
	strcpy (mect_rec.prog_stat, "0");
	
	if (new_rec)
	{
		cc = abc_add (mect, &mect_rec);
		if (cc)
			file_err (cc, mect, "DBADD");
	}
	else
	{
		if (overide)
		{
			cc = abc_update (mect, &mect_rec);
			if (cc)
				file_err (cc, mect, "DBUPDATE");
		}
	}

	/*
	 * Create Text report file.
	 */
	sprintf (programName, "%s/SCRIPT/%s", bname, mect_rec.txt_file);
	if ((textFile = fopen (programName, "w")) == NULL)
	{
		sprintf (err_str, "Error in %s during (FOPEN)", programName);
		sys_err (err_str, errno, PNAME);
	}
	fclose (textFile);

	set_file (programName);
}

int
CheckOk (void)
{
	int	i;

	clear ();
	
	box (0, 0, 78, 22);
	print_at (pos++, 1 , ML (mlMenuMess149)); 
	move (1, pos++);
	line (76);

	i = prmptmsg (ML (mlMenuMess148), "YyNn", 2, pos);
	if (i == 'N' || i == 'n')
		return (ShowError ());

	pos+= 2;
	i = prmptmsg (ML (mlMenuMess150), "YyNn", 2, pos);

	if (i == 'N' || i == 'n')
		return (ShowError ());

	if (BR_DBT)
	{
		pos+= 2;
		i = prmptmsg (ML (mlMenuMess151), "YyNn", 2, pos);

		if (i == 'N' || i == 'n')
			dbBrReport = FALSE;
		else
			dbBrReport = TRUE;
	}
	if (BR_CRD)
	{
		pos+= 2;
		i = prmptmsg (ML (mlMenuMess152), "YyNn", 2, pos);

		if (i == 'N' || i == 'n')
			crBrReport = FALSE;
		else
			crBrReport = TRUE;
	}
	if (BR_INV)
	{
		pos+= 2;
		i = prmptmsg (ML (mlMenuMess153), "YyNn", 2, pos);
		if (i == 'N' || i == 'n')
			skBrReport = FALSE;
		else
			skBrReport = TRUE;
	}
	if (BR_GEN)
	{
		pos+= 2;
		i = prmptmsg (ML (mlMenuMess154), "YyNn", 2, pos);

		if (i == 'N' || i == 'n')
			glBrReport = FALSE;
		else
			glBrReport = TRUE;
	}
	pos+= 2;
	i = prmptmsg (ML (mlMenuMess155), "YyNn", 2, pos);

	if (i == 'N' || i == 'n')
		overide = FALSE;
	else
		overide = TRUE;

	pos+= 2;
	i = prmptmsg (ML (mlMenuMess198), "YyNn", 2, pos);

	if (i == 'N' || i == 'n')
		return (ShowError ());

	return (TRUE);
}

int
ShowError (void)
{
	int	i;

	pos+= 2;

	fflush (stdout);
	for (i = 0; i < 4; i++)
	{
		strcpy (err_str, ML (mlMenuMess238));
		rv_pr (err_str, 1, pos, (i % 2));
		sleep (sleepTime);
	}
	return (FALSE);
}

/*
 * Routine which writes a report of how the update went to pformat. 
 */
void
ReportMect (
	char	*coNo, 
	char	*brNo, 
	char	*modType, 
	long	modDate, 
	int		branchRep, 
	char	*_name)
{
	char	mod_stat [10];
	char	mod_name [16];

	sprintf (mect_rec.co_no, "%-2.2s", coNo);
	sprintf (mect_rec.br_no, "%-2.2s", brNo);
	sprintf (mect_rec.module_type, "%-2.2s", modType);
	if (find_rec (mect, &mect_rec, COMPARISON, "r"))
		return;
	
	if (!strcmp (brNo, "  "))
		fprintf (pout, "| %-2.2s | CO ", coNo);
	else
		fprintf (pout, "| %-2.2s | %-2.2s ", coNo, brNo);
	fprintf (pout, "|  %-15.15s  ", _name);

	if (!strcmp (modType, "DB"))
		strcpy (mod_name, "  CUSTOMERS  ");

	if (!strcmp (modType, "CR"))
		strcpy (mod_name, "  SUPPLIERS  ");

	if (!strcmp (modType, "SK"))
		strcpy (mod_name, "  INVENTORY. ");

	if (!strcmp (modType, "GL"))
		strcpy (mod_name, " GENERAL LED.");

	if (mect_rec.status == NOT_ACTIVE)
		strcpy (mod_stat, "INACTIVE.");

	if (mect_rec.status == OPEN)
		strcpy (mod_stat, "  OPEN   ");

	if (mect_rec.status == TO_CLOSE)
		strcpy (mod_stat, "TO CLOSE.");

	if (mect_rec.status == CLOSING)
		strcpy (mod_stat, "CLOSING. ");

	if (mect_rec.status == CLOSED)
		strcpy (mod_stat, " CLOSED. ");

	fprintf (pout, "|%13.13s|%9.9s", mod_name, mod_stat);
	fprintf (pout, "|            |           |             |        |\n");
}

void
ReportHeading (void)
{
	if ((pout = popen ("pformat", "w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	fprintf (pout, ".START\n");
	fprintf (pout, ".LP%d\n", printerNo);
	fprintf (pout, ".L158\n");
	fprintf (pout, ".8\n");
	fprintf (pout, ".EMONTH END MODULE CHECK LIST\n");
	fprintf (pout, ".E%s on %s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (pout, ".B1\n");

	fprintf (pout, ".R==============================");
	fprintf (pout, "=====================================");
	fprintf (pout, "====================================\n");

	fprintf (pout, "==============================");
	fprintf (pout, "=====================================");
	fprintf (pout, "====================================\n");

	fprintf (pout, "| CO | BR |   DESCRIPTION     ");
	fprintf (pout, "|   MODULE    |  MODULE | PROCESSING ");
	fprintf (pout, "| STATMENTS | MONTH END   | MODULE |\n");

	fprintf (pout, "| NO | NO |                   ");
	fprintf (pout, "|    NAME     |  STATUS |  COMPLETE. ");
	fprintf (pout, "|    RUN    | REPORTS RUN | CLOSED |\n");

	fprintf (pout, "|----|----|-------------------");
	fprintf (pout, "|-------------|---------|------------");
	fprintf (pout, "|-----------|-------------|--------|\n");
}

void
ClosePrint (void)
{
	fprintf (pout, ".EOF\n");
	pclose (pout);
}
