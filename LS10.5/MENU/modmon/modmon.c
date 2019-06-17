 /*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: modmon.c,v 5.2 2001/08/09 05:13:37 scott Exp $
|  Program Name  : (modmon.c) 
|  Program Desc  : (Module Run & Reset Program)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: modmon.c,v $
| Revision 5.2  2001/08/09 05:13:37  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:31  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/02 08:46:54  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: modmon.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/modmon/modmon.c,v 5.2 2001/08/09 05:13:37 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_menu_mess.h>

#define	CO_DBT		 (envVarCoClose [0] == '1')
#define	CO_CRD		 (envVarCoClose [1] == '1')
#define	CO_INV		 (envVarCoClose [2] == '1')
#define	CO_PAY		 (envVarCoClose [3] == '1')
#define	CO_GEN		 (envVarCoClose [4] == '1')

#define	NOT_ACTIVE	0
#define	OPEN		1
#define	TO_CLOSE	2
#define	CLOSING		3
#define	CLOSED		4
#define	ERR_FND 	5
#define	BAD_ERROR	-1

/*===========================================
| Source directory of module close scripts. |
===========================================*/
#ifndef	LINUX
extern	int	errno;
#endif	/* LINUX */

	int	companyUsed;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int	i,
		j,
		k;

	char	envVarCoClose [6];

	long	dateUsed = 0L;
	
	FILE *	pout;


#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct mectRecord	mect_rec;
struct mectRecord	mect2_rec;


	long	*comr_mod_date	=	&comr_rec.dbt_date;
	long	*esmr_mod_date	=	&esmr_rec.dbt_date;

	char	*data	=	"data",
			*mect2	=	"mect2";

	struct {					/*====================================*/
		char *	longName;		/*| Long name of stream eg. payables |*/
		char *	fullName;		/*| Name of shell script to run.     |*/
		char	companyNo [3];	/*| Company number.                  |*/
		char	branchNo [4];	/*| Branch number.                   |*/
		char	startTime [20];	/*| Start time of process.           |*/
		char	endTime [20];	/*| End time of process.             |*/
		int		returnCode;		/*| Return code from process.        |*/
		char	moduleName [3];	/*| Module Name For Month End.       |*/
	} commandRec [] = {				/*====================================*/
		 {"CUSTOMERS",  "db_mend","","","","",-1,"DB"},
		 {"SUPPLIERS","cr_mend","","","","",-1,"CR"},
		 {"INVENTORY","sk_mend","","","","",-1,"SK"},
		 {"PAYROLL  ","py_mend","","","","",-1,"PR"},
		 {"G/ LEDGER","gl_mend","","","","",-1,"GL"}
	};

/*============================
| Local function prototypes  |
============================*/
void	OpenModule		 (char *);
void	UpdateStatus	 (int);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
int		RollDate		 (int);
int		AddMect			 (char *, long);
int		AllClosed		 (char *);
int		ReportRun		 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;
	
	int	status = 0;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envVarCoClose, "%-5.5s", "11111");
	else
		sprintf (envVarCoClose, "%-5.5s", sptr);

	if (argc < 2) 
	{
		print_at (0, 0, mlMenuMess707, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();

	/*-----------------------------
	| Open rest of database file. |
	-----------------------------*/
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	for (i = 1; i < argc; i++)
	{
		j = atoi (argv [i]) - 1;

		/*-------------------------
		| Closing debtors module. |
		-------------------------*/
		if (!strcmp ( commandRec [j].moduleName, "DB") )
		{
			companyUsed 	= CO_DBT;
			dateUsed 		= comm_rec.dbt_date;
		}

		/*---------------------------
		| Closing creditors module. |
		---------------------------*/
		if (!strcmp ( commandRec [j].moduleName, "CR") )
		{
			companyUsed 	= CO_CRD;
			dateUsed 		= comm_rec.crd_date;
		}

		/*-----------------------
		| Closing Stock module. |
		-----------------------*/
		if (!strcmp ( commandRec [j].moduleName, "SK") )
		{
			companyUsed 	= CO_INV;
			dateUsed 		= comm_rec.inv_date;
		}

		/*-------------------------
		| Closing Payroll module. |
		-------------------------*/
		if (!strcmp ( commandRec [j].moduleName, "PR") )
		{
			companyUsed 	= CO_PAY;
			dateUsed 		= comm_rec.payroll_date;
		}

		/*--------------------------------
		| Closing General Ledger module. |
		--------------------------------*/
		if (!strcmp (commandRec [j].moduleName, "GL"))
		{
			companyUsed 	= CO_GEN;
			dateUsed 		= comm_rec.gl_date;
		}
		
		strcpy (commandRec [j].companyNo,comm_rec.co_no);

		if (companyUsed)
			strcpy (commandRec [j].branchNo,"ALL");
		else
			sprintf (commandRec [j].branchNo,"%-3.3s", comm_rec.est_no);

		/*-----------------
		| Log start time. |
		------------------*/
		sprintf (err_str, "%s", SystemTime ());
		strcpy (commandRec [j].startTime, err_str);

		/*---------------------------
		| Run Month End Processing. |
		---------------------------*/
		if (sys_exec (commandRec [j].fullName))
		{
			commandRec [j].returnCode = errno;
			shutdown_prog ();
			return (errno);
		}

		/*--------------------------------
		| Change to first of next month. |
		--------------------------------*/
		RollDate (j);

		/*------------------
		| Log Finish Time. |
		------------------*/
		sprintf (err_str, "%s", SystemTime ());
		strcpy (commandRec [j].endTime,err_str);

		if (AllClosed (commandRec [j].moduleName))
			OpenModule (commandRec [j].moduleName);

		commandRec [j].returnCode = 0;
	}
	if (fork () == 0)
	{
		status = execlp ("co_update", "co_update", (char *)0);
		if (status)
		{
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
	}
	else
		wait (0);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=====================================================
| Re-open module as all branches have been processed. |
=====================================================*/
void
OpenModule (
 char *	moduleName)
{
	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, "  ");
	strcpy (mect_rec.module_type, moduleName);
	cc = find_rec (mect, &mect_rec, GTEQ, "u");
	while (!cc && !strcmp (mect_rec.co_no,comm_rec.co_no))
	{
		if (!strcmp (mect_rec.module_type, moduleName) )
			UpdateStatus (OPEN);
		else
			abc_unlock (mect);

		cc = find_rec (mect, &mect_rec, NEXT, "u");
	}
}

/*==========================================
| Update status and clear start/end times. |
==========================================*/
void
UpdateStatus (
 int _cur_status)
{
	mect_rec.status = _cur_status;
	strcpy (mect_rec.start_time, "00:00");
	strcpy (mect_rec.end_time, "00:00");

	cc = abc_update (mect, &mect_rec);
	if (cc)
		file_err (cc, mect, "DBUPDATE");
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
	ReportRun ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (mect2, mect);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (mect,  mect_list, MECT_NO_FIELDS, "mect_id_no");
	open_rec (mect2, mect_list, MECT_NO_FIELDS, "mect_id_no2");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (mect);
	abc_fclose (mect2);
	abc_dbclose (data);
}

/*=========================================================================
| Routine which will roll the module date over to the first of the month, |
| changes module according to modno (0= receivables,4=g/l).              |
| Returns: 0 if ok, non-zero if not.                                      |
=========================================================================*/
int
RollDate (
 int modno)
{
	if (companyUsed)
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, comr, "DBFIND");

		comr_mod_date [modno] = MonthEnd (dateUsed) + 1L;

		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		esmr_mod_date [modno] = MonthEnd (dateUsed) + 1L;
		
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");
	}
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		esmr_mod_date [modno] = MonthEnd (dateUsed) + 1L;
		
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");
	}
	return (EXIT_SUCCESS);
}

/*======================================
| Add mect record as no record exists. |
======================================*/
int
AddMect (
 char *	moduleName,
 long	_dateUsed)
{
	int	  _mth;

	DateToDMY (_dateUsed, NULL, &_mth, NULL); 
	_mth--;
	if (_mth == 0)
		_mth = 12;

	strcpy (mect_rec.co_no, comm_rec.co_no);
	strcpy (mect_rec.br_no, (companyUsed) ? "  " : comm_rec.est_no);
	sprintf (mect_rec.module_type,"%-2.2s", moduleName);
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

/*============================================================
| Check if all records for module type / company are closed. |
============================================================*/
int
AllClosed (
 char *	_type)
{
	strcpy (mect2_rec.co_no, comm_rec.co_no);
	sprintf (mect2_rec.module_type, "%-2.2s", _type);
	cc = find_rec (mect2, &mect2_rec, GTEQ, "r");
	while (!cc && !strcmp (mect2_rec.co_no, comm_rec.co_no) &&
		   !strcmp (mect2_rec.module_type, _type))
	{
		if (mect2_rec.status != CLOSED)
			return (FALSE);

		cc = find_rec (mect2, &mect2_rec, NEXT, "r");
	}
	return (TRUE);
}

/*==================================================================
| Routine which writes a report of how the update went to pformat. |
==================================================================*/
int
ReportRun (
 void)
{
	int	i,
		nbr = 0;

	if ( (pout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pout,".LP1\n");
	fprintf (pout,".L130\n");
	fprintf (pout,".SO\n");
	fprintf (pout,".OP\n");
	fprintf (pout,".8\n");
	fprintf (pout,".B4\n");
	fprintf (pout,".EMODULE CLOSE OFF\n");
	fprintf (pout,".E%s on %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pout,".B3\n");
	fprintf (pout,".R================================================");
	fprintf (pout,"===========================================\n");

	fprintf (pout,"==================================================");
	fprintf (pout,"=========================================\n");

	fprintf (pout,"|CO NO|BR NO|   MODULE NAME   |    START TIME    |");
	fprintf (pout,"   FINISH TIME    | PROCESSING ERRORS   |\n");

	fprintf (pout,"|-----|-----|-----------------|------------------|");
	fprintf (pout,"------------------|---------------------|\n");
	
	for (i=0; i<5;i++) 
	{
		if (commandRec [i].returnCode >= 0) 
		{
			nbr++;
			fprintf (pout,"| %2.2s  ",  commandRec [i].companyNo);
			fprintf (pout,"| %-3.3s ",  commandRec [i].branchNo);
			fprintf (pout,"|%-17.17s",  commandRec [i].longName);
			fprintf (pout,"|%-18.18s",  commandRec [i].startTime);
			fprintf (pout,"|%-18.18s|", commandRec [i].endTime);
			if (commandRec [i].returnCode == 0)
				fprintf (pout,"%-21.21s|\n"," NONE");
			else
				fprintf (pout,"YES, Return code: %3d|\n", commandRec [i].returnCode);
		}
	}
	if (!nbr) 
	{
		fprintf (pout,".R==========================================");
		fprintf (pout,"=====================================\n");
		fprintf (pout,".B3\n.eNO MODULES CLOSED OFF\n");
	}
	fprintf (pout,".EOF\n");
	pclose (pout);
	return (EXIT_SUCCESS);
}
