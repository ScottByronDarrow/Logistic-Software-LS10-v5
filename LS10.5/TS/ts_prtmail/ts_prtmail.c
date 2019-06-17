/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ts_prtmail.c,v 5.3 2002/07/17 09:58:17 scott Exp $
|  Program Name  : (ts_prtmail.c  )                                 |
|  Program Desc  : (Print Mailers And Update Letters Sent File. )   |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 06/12/91         |
|---------------------------------------------------------------------|
| $Log: ts_prtmail.c,v $
| Revision 5.3  2002/07/17 09:58:17  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:23:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:56:00  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/03/27 17:17:35  robert
| updated as fix_edge expects return value
|
| Revision 4.2  2001/03/27 05:14:11  scott
| Updated to add sleep delay - did not work with LS10-GUI
|
| Revision 4.1  2001/03/27 05:13:22  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_prtmail.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_prtmail/ts_prtmail.c,v 5.3 2002/07/17 09:58:17 scott Exp $";

#define	MOD	5

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_ts_mess.h>
#include <ml_std_mess.h>

#define ALL_SECTS	 (!strncmp (local_rec.startSector, "   ", 3) && \
					  !strncmp (local_rec.endSector,   "~~~", 3))
#define ALL_AREAS	 (!strncmp (local_rec.startArea, "  ", 2) && \
					  !strncmp (local_rec.endArea,   "~~", 2))
#define	FOLLOW_UP	 (tslh_rec.lett_type [0] == 'F')
#define	NO_MAILER	 (tspm_rec.mail_flag [0] == 'N')

FILE	*fout;

static	char	*mth [] = {
	"January", "February", "March"    , "April"  , "May"     , "June",
	"July"   , "August"  , "September", "October", "November", "December"
};

extern	int	TruePosition;
extern	int	EnvScreenOK;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	envVarDbCo = 0,
			envVarDbFind = 0;
	
	int		clearOk = TRUE;

	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct tspmRecord	tspm_rec;
struct exsfRecord	exsf_rec;
struct exafRecord	exaf_rec;
struct tslbRecord	tslb_rec;
struct tslhRecord	tslh_rec;
struct tslhRecord	tslh2_rec;
struct tslnRecord	tsln_rec;
struct tslsRecord	tsls_rec;
struct exclRecord	excl_rec;

	/*-------------------------------
	| Set up pointers to file names |
	-------------------------------*/
	char	*data    = "data",
	    	*cumr2   = "cumr2",
	    	*tslh2   = "tslh2";

	char	systemDate [11];
	long	lsystemDate;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	startCustomer [7];
	char	startCustomerName [41];
	char	endCustomer [7];
	char	endCustomerName [41];
	char	startSector [4];
	char	startSectorName [41];
	char	endSector [4];
	char	endSectorName [41];
	char	startArea [3];
	char	startAreaName [41];
	char	endArea [3];
	char	endAreaName [41];
	char	letterCode [11];
	char	letterDesc [41];
	long	startDate;
	long	endDate;
	int		printerNumber;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "letterCode", 4, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Letter Code    ", "Enter Name Of Letter To Have Allocated.", 
		NO, NO, JUSTLEFT, "", "", local_rec.letterCode}, 
	{1, LIN, "letterDesc", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.letterDesc}, 
	{1, LIN, "printerNumber", 6, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No     ", "", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{1, LIN, "startCustomer", 8, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Lead     ", "Enter Start Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startCustomer}, 
	{1, LIN, "startCustomerName", 8, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startCustomerName}, 
	{1, LIN, "endCustomer", 9, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Lead       ", "Enter End Lead.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endCustomer}, 
	{1, LIN, "endCustomerName", 9, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endCustomerName}, 
	{1, LIN, "startSector", 11, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", " ",  "Start Sector   ", "Enter Start Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.startSector}, 
	{1, LIN, "startSectorName", 11, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startSectorName}, 
	{1, LIN, "endSector", 12, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "",  "End Sector     ", "Enter End Business Sector.", 
		NO, NO, JUSTLEFT, "", "", local_rec.endSector}, 
	{1, LIN, "endSectorName", 12, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endSectorName}, 
	{1, LIN, "startArea", 14, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ",  "Start Area     ", "Enter Start Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.startArea}, 
	{1, LIN, "startAreaName", 14, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.startAreaName}, 
	{1, LIN, "endArea", 15, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ",  "End Area       ", "Enter End Area.", 
		NO, NO, JUSTRIGHT, "", "", local_rec.endArea}, 
	{1, LIN, "endAreaName", 15, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.endAreaName}, 
	{1, LIN, "startDate", 17, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "",  "Start Date     ", "Enter Start Of Next Phone Date Range. Default To Ignore Date Range", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate}, 
	{1, LIN, "endDate", 18, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "",  "End Date       ", "Enter End Of Next Phone Date Range.", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

char	filename [100],
		tsNumber [15],
		userLogName [15];

int		envVarTsLMargin	= 0,
		partPrinted		= 0,
		mailerNo 		= 0,
		backgroundPrint	= FALSE,
		firstTime		= 0;

long	hhcuHash	=	0L,
		hhlhHash	=	0L;

#include <ts_commands.h>
#include <FindCumr.h>

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
char 	*GetDate (long cur_date);
int 	AddLable (void);
int 	CreateTsls (void);
int 	Process (void);
int 	ValidCommand (char *wk_str);
int 	ValidCustomer (void);
int 	fix_edge (int line_no);
int 	heading (int scn);
int 	spec_valid (int field);
void 	CloseDB (void);
void 	OpenDB (void);
void 	ParseFunc (char *wrk_prt);
void 	PrintMailer (void);
void 	ProcessCustomer (void);
void 	ReadMisc (void);
void 	SrchExaf (char *key_val);
void 	SrchExcl (char *key_val);
void 	SrchTslh (char *key_val);
void 	SubstituteCommand (int cmd, int offset);
void 	shutdown_prog (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int		cc1;
	char	*sptr;

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	sptr = getenv ("LOGNAME");
	if (sptr)
	{
		sprintf (userLogName, "%-14.14s", sptr);
		upshift (userLogName);
	}
	else
		sprintf (userLogName, "%-14.14s", "DEFAULT");

	backgroundPrint = FALSE;
	if (argc == 4)
	{
		hhcuHash = atol (argv [1]);
		hhlhHash = atol (argv [2]);
		local_rec.printerNumber = atoi (argv [3]);
		backgroundPrint = TRUE;
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*--------------------
	| Check Left Margin. |
	--------------------*/
	sptr = chk_env ("TS_LMARGIN");
	if (sptr == (char *)0)
		envVarTsLMargin = 10;
	else
		envVarTsLMargin = atoi (sptr);

	envVarDbCo 	 = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	if (!backgroundPrint)
	{
		SETUP_SCR (vars);

		init_scr ();
		set_tty ();

		set_masks ();
	}

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	OpenDB ();

	ReadMisc (); 	

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	firstTime = TRUE;
	if (backgroundPrint)
	{
		cumr_rec.hhcu_hash	=	hhcuHash;
		cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
		tslh_rec.hhlh_hash	=	hhlhHash;
		cc1 = find_rec (tslh2, &tslh_rec, COMPARISON, "r");
		if (!cc && !cc1)
			ProcessCustomer ();
	}
	else
	{
		/*--------------------
		| Main control loop. |
		--------------------*/
		while (!prog_exit)
		{
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			init_vars (1);
	
			clearOk = TRUE;
	
			/*-------------------------------
			| Enter screen 1 linear input . |
			-------------------------------*/
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;
	
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
	
			Process ();
			prog_exit = TRUE;
		}
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	if (!firstTime)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}

	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumr2, cumr);
	abc_alias (tslh2, tslh);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envVarDbFind) ? "cumr_id_no3"
							    : "cumr_id_no");

	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (tslb,  tslb_list, TSLB_NO_FIELDS, "tslb_id_no3");
	open_rec (tslh,  tslh_list, TSLH_NO_FIELDS, "tslh_id_no");
	open_rec (tslh2, tslh_list, TSLH_NO_FIELDS, "tslh_hhlh_hash");
	open_rec (tsln,  tsln_list, TSLN_NO_FIELDS, "tsln_id_no");
	open_rec (tsls,  tsls_list, TSLS_NO_FIELDS, "tsls_id_no");
	open_rec (tspm,  tspm_list, TSPM_NO_FIELDS, "tspm_hhcu_hash");
	open_rec (excl,  excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exaf,  exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (exsf,  exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (tslb);
	abc_fclose (tslh);
	abc_fclose (tslh2);
	abc_fclose (tsln);
	abc_fclose (tsls);
	abc_fclose (tspm);
	abc_fclose (excl);
	abc_fclose (exaf);
	abc_fclose (exsf);
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);
}

/*--------------------------
| Process customers within |
| selected range           |
--------------------------*/
int
Process (void)
{
	abc_selfield (cumr, "cumr_id_no3");

	dsp_screen ("Processing Mailers", comm_rec.co_no, comm_rec.co_name);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	sprintf (cumr_rec.dbt_no, "%-6.6s", local_rec.startCustomer);

	if (!strncmp (local_rec.endCustomer, "~~~~~~", 6))
		memset ((char *)local_rec.endCustomer,0xff,sizeof (local_rec.endCustomer));
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
	       strcmp (cumr_rec.dbt_no, local_rec.endCustomer) <= 0)
	{
		dsp_process ("Customer :", cumr_rec.dbt_no);

		hhcuHash = cumr_rec.hhcu_hash;
		ProcessCustomer ();

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

void
ProcessCustomer (void)
{
	int		tmp_pid, tmp_day, tmp_mth;

	tspm_rec.hhcu_hash	=	hhcuHash;
	cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
	if (cc)
		return;

	if (!backgroundPrint)
	{
		if (!ValidCustomer ())
			return;
	}

	DateToDMY (local_rec.lsystemDate, &tmp_day, &tmp_mth, NULL);
	tmp_pid = getpid ();
	sprintf (tsNumber, "t%05d.%02d%02d%03d",tmp_pid,tmp_day,tmp_mth, mailerNo);

	if (FOLLOW_UP)
	{
		if (NO_MAILER)
			return;

		if (!CreateTsls ())
			return;
	}

	/*---------------------
	| Lookup exsf record. |
	---------------------*/
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, cumr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman, "%-40.40s", " ");
	
	/*---------------------
	| Lookup exaf record. |
	---------------------*/
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	strcpy (exaf_rec.area_code, cumr_rec.area_code);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exaf_rec.area, "%-40.40s", " ");
	
	/*---------------------
	| Lookup excl record. |
	---------------------*/
	strcpy (excl_rec.co_no, comm_rec.co_no);
	strcpy (excl_rec.class_type, cumr_rec.class_type);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		sprintf (excl_rec.class_desc, "%-40.40s", " ");
	
	mailerNo++;
	PrintMailer ();
}

/*-----------------------------------------------
| Check that customer is within specified range |
-----------------------------------------------*/
int
ValidCustomer (void)
{
	if (!ALL_SECTS && 
	    (strcmp (cumr_rec.class_type, local_rec.startSector) < 0 ||
	    strcmp (cumr_rec.class_type, local_rec.endSector) > 0))
		return (FALSE);

	if (!ALL_AREAS && 
	    (strcmp (cumr_rec.area_code, local_rec.startArea) < 0 ||
	    strcmp (cumr_rec.area_code, local_rec.endArea) > 0))
		return (FALSE);

	if (local_rec.startDate != 0L &&
	    (tspm_rec.n_phone_date < local_rec.startDate ||
	     tspm_rec.n_phone_date > local_rec.endDate))
		return (FALSE);

	return (TRUE);
}

/*---------------------------
| Create letter sent record |
---------------------------*/
int
CreateTsls (void)
{
	strcpy (tsls_rec.co_no, comm_rec.co_no);
	tsls_rec.hhlh_hash = hhlhHash;
	tsls_rec.hhcu_hash = hhcuHash;
	sprintf (tsls_rec.file_name, "%-14.14s", tsNumber);
	tsls_rec.date_sent 	 = local_rec.lsystemDate;
	tsls_rec.date_called = 0L;
	strcpy (tsls_rec.time_called, "00:00");
	strcpy (tsls_rec.stat_flag, "0");

	cc = find_rec (tsls, &tsls_rec, COMPARISON, "r");
	if (!cc)
		return (FALSE);
	
	cc = abc_add (tsls, &tsls_rec);
	if (cc)
		file_err (cc, tsls, "DBADD");

	return (TRUE);
}

/*==================================
| Print letters for leads selected |
==================================*/
void
PrintMailer (void)
{
	char	parse_str [201];

	if (firstTime)
	{
		if ((fout = popen ("pformat", "w")) == 0) 
			file_err (errno, "pformat", "popen ()");
		
		firstTime = FALSE;

		fprintf (fout, ".START00/00/00\n");
		fprintf (fout, ".LP%d\n", local_rec.printerNumber);
		fprintf (fout, ".OP\n");
		fprintf (fout, ".SO\n");

		fprintf (fout, ".2\n");
		fprintf (fout, ".PI10\n");
		fprintf (fout, ".L132\n");
	}

	tsln_rec.hhlh_hash = hhlhHash;
	tsln_rec.line_no = 0;

	cc = find_rec (tsln, &tsln_rec, GTEQ, "r");
	while (!cc && tsln_rec.hhlh_hash == hhlhHash)
	{
		sprintf (parse_str,
			"%*.*s%s", 
			envVarTsLMargin, envVarTsLMargin, " ",
			clip (tsln_rec.desc));
						  
		ParseFunc (parse_str);
		cc = find_rec (tsln, &tsln_rec, NEXT, "r");
	}

	fprintf (fout, ".PA\n");
	fflush (fout);

	/*--------------------------
	| Add record to label file |
	--------------------------*/
	AddLable ();
}

/*--------------------------
| Add record to label file |
--------------------------*/
int
AddLable (void)
{
	strcpy (tslb_rec.co_no, cumr_rec.co_no);
	sprintf (tslb_rec.tslb_operator, "%-14.14s", userLogName);
	tslb_rec.hhcu_hash = cumr_rec.hhcu_hash;
	tslb_rec.hhlh_hash = hhlhHash;
	tslb_rec.date_sent = TodaysDate ();
	tslb_rec.time_sent = atot (TimeHHMM ());

	strcpy (tslb_rec.label_prt, "N");
	strcpy (tslb_rec.stat_flag, "0");
	cc = abc_add (tslb, &tslb_rec);
	if (cc)
		file_err (cc, tslb, "DBADD");

	return (EXIT_SUCCESS);
}

void
ParseFunc (
 char*  wrk_prt)
{
	int		cmd;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = strdup (wrk_prt);
	
	partPrinted = TRUE;

	/*-------------------------------
	|	look for caret command	|
	-------------------------------*/
	cptr = strchr (wk_prt,'.');
	dptr = wk_prt;
	while (cptr)
	{
		partPrinted = FALSE;
		/*-------------------------------
		|	print line up to now	|
		-------------------------------*/
		*cptr = (char)NULL;

		if (cptr != wk_prt)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",dptr);
		}

		/*-------------------------------
		|	check if valid .command	|
		-------------------------------*/
		cmd = ValidCommand (cptr + 1);
		if (cmd >= CUR_DAT)
		{
			SubstituteCommand (cmd,cptr - wk_prt);
			dptr = cptr + 8;
		}
		else
		{
			fprintf (fout,".");
			partPrinted = TRUE;
			dptr = cptr + 1;
		}

		cptr = strchr (dptr,'.');
	}

	/*-------------------------------
	|	print rest of line	|
	-------------------------------*/
	if (partPrinted)
	{
		if (dptr)
			fprintf (fout,"%s\n",dptr);
		else
			fprintf (fout,"\n");
	}
	free (wk_prt);
}

/*=====================
| Validate .commands. |
=====================*/
int
ValidCommand (
 char*  wk_str)
{
	int		i;

	/*----------------------------------------
	| Dot command is last character on line. |
	----------------------------------------*/
	if (!strlen ( wk_str))
		return (-1);

	for (i = 0;i < N_CMDS;i++)
		if (!strncmp (wk_str, dot_cmds [i].command, 7))
			return (i);

	return (-1);
}

/*==============================================
| Substitute valid .commands with actual data. |
==============================================*/
void
SubstituteCommand (
 int    cmd,
 int    offset)
{
	char	*pr_sptr;
	char	tmp_amt [21];

	switch (cmd)
	{
	/*-------------------------------
	| System Date, format dd/mm/yy. |
	-------------------------------*/
	case	CUR_DAT:
		partPrinted = TRUE;
		fprintf (fout,"%-10.10s",local_rec.systemDate);
		break;

	/*-------------------
	| Full system date. |
	-------------------*/
	case FUL_DAT:
		pr_sptr = GetDate (local_rec.lsystemDate);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*---------------
	| Company Name. |
	---------------*/
	case	CO_NAME:
		pr_sptr = clip (comr_rec.co_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 1. |
	--------------------*/
	case	CO_ADR1:
		pr_sptr = clip (comr_rec.co_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 2. |
	--------------------*/
	case	CO_ADR2:
		pr_sptr = clip (comr_rec.co_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------
	| Company Address 3. |
	--------------------*/
	case	CO_ADR3:
		pr_sptr = clip (comr_rec.co_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*------------------
	| Prospect Number. |
	------------------*/
	case	DB_NUMB:
		pr_sptr = clip (cumr_rec.dbt_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-6.6s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Acronym. |
	-------------------*/
	case	DB_ACRO:
		pr_sptr = clip (cumr_rec.dbt_acronym);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-9.9s",pr_sptr);
		}
		break;

	/*----------------
	| Prospect Name. |
	----------------*/
	case	DB_NAME:
		pr_sptr = clip (cumr_rec.dbt_name);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR1:
		pr_sptr = clip (cumr_rec.ch_adr1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR2:
		pr_sptr = clip (cumr_rec.ch_adr2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR3:
		pr_sptr = clip (cumr_rec.ch_adr3);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Name 1. |
	--------------------------*/
	case	DB_CNT1:
		pr_sptr = clip (tspm_rec.cont_name1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-30.30s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Code 1. |
	--------------------------*/
	case	CNT_CD1:
		pr_sptr = clip (tspm_rec.cont_code1);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-3.3s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Name 2. |
	--------------------------*/
	case	DB_CNT2:
		pr_sptr = clip (tspm_rec.cont_name2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-30.30s",pr_sptr);
		}
		break;

	/*--------------------------
	| Prospect Contact Code 2. |
	--------------------------*/
	case	CNT_CD2:
		pr_sptr = clip (tspm_rec.cont_code2);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-3.3s",pr_sptr);
		}
		break;

	/*--------------
	| Salesman No. |
	--------------*/
	case	SM_NUMB:
		pr_sptr = clip (exsf_rec.salesman_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%2.2s",pr_sptr);
		}
		break;

	/*----------------
	| Salesman Name. |
	----------------*/
	case	SM_NAME:
		pr_sptr = clip (exsf_rec.salesman);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*----------
	| Area No. |
	----------*/
	case	AR_NUMB:
		pr_sptr = clip (exaf_rec.area_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%2.2s",pr_sptr);
		}
		break;

	/*------------
	| Area Name. |
	------------*/
	case	AR_NAME:
		pr_sptr = clip (exaf_rec.area);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*----------------
	| Contract Type. |
	----------------*/
	case	CNT_TYP:
		pr_sptr = clip (cumr_rec.cont_type);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-------------
	| Price Type. |
	-------------*/
	case	PRI_TYP:
		pr_sptr = clip (cumr_rec.price_type);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*------------
	| Bank Code. |
	------------*/
	case	BNK_CDE:
		pr_sptr = clip (cumr_rec.bank_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*--------------
	| Bank Branch. |
	--------------*/
	case	BNK_BRN:
		pr_sptr = clip (cumr_rec.branch_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*----------------
	| Discount Code. |
	----------------*/
	case	DIS_CDE:
		pr_sptr = clip (cumr_rec.disc_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-----------
	| Tax Code. |
	-----------*/
	case	TAX_CDE:
		pr_sptr = clip (cumr_rec.tax_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-------------
	| Tax Number. |
	-------------*/
	case	TAX_NUM:
		pr_sptr = clip (cumr_rec.tax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-----------------------
	| Date Of Last Invoice. |
	-----------------------*/
	case	DT_LINV:
		partPrinted = TRUE;
		fprintf (fout, DateToString (cumr_rec.date_lastinv));
		break;

	/*-----------------------
	| Date Of Last Payment. |
	-----------------------*/
	case	DT_LPAY:
		partPrinted = TRUE;
		fprintf (fout, DateToString (cumr_rec.date_lastpay));
		break;

	/*-------------------------
	| Amount Of Last Payment. |
	-------------------------*/
	case	AMT_LPY:
		sprintf (tmp_amt,"%8.2f",cumr_rec.amt_lastpay);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*-----------------------
	| Month To Date Sales . |
	-----------------------*/
	case	MTD_SAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.mtd_sales);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*----------------------
	| Year To Date Sales . |
	----------------------*/
	case	YTD_SAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.ytd_sales);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*---------------------------
	| Value Of Current Orders . |
	---------------------------*/
	case	ORD_VAL:
		sprintf (tmp_amt,"%8.2f",cumr_rec.ord_value);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*------------
	| Post Code. |
	------------*/
	case	DB_POST:
		pr_sptr = clip (cumr_rec.post_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-10.10s",pr_sptr);
		}
		break;

	/*------------------
	| Business Sector. |
	------------------*/
	case	DB_BSEC:
		pr_sptr = clip (excl_rec.class_desc);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*---------------
	| Phone Number. |
	---------------*/
	case	PHN_NUM:
		pr_sptr = clip (cumr_rec.phone_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-15.15s",pr_sptr);
		}
		break;

	/*-------------
	| Fax Number. |
	-------------*/
	case	FAX_NUM:
		pr_sptr = clip (cumr_rec.fax_no);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-15.15s",pr_sptr);
		}
		break;

	/*------------------
	| Phone Frequency. |
	------------------*/
	case	PH_FREQ:
		sprintf (tmp_amt,"%2d",tspm_rec.phone_freq);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*------------------
	| Next Phone Date. |
	------------------*/
	case	NX_PHDT:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tspm_rec.n_phone_date));
		break;

	/*------------------
	| Next Phone Time. |
	------------------*/
	case	NX_PHTM:
		pr_sptr = clip (tspm_rec.n_phone_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*------------------
	| Visit Frequency. |
	------------------*/
	case	VS_FREQ:
		sprintf (tmp_amt,"%2d",tspm_rec.visit_freq);
		pr_sptr = clip (tmp_amt);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%s",pr_sptr);
		}
		break;

	/*------------------
	| Next Visit Date. |
	------------------*/
	case	NX_VSDT:
		partPrinted = TRUE;
		fprintf (fout,DateToString (tspm_rec.n_visit_date));
		break;

	/*------------------
	| Next Visit Time. |
	------------------*/
	case	NX_VSTM:
		pr_sptr = clip (tspm_rec.n_visit_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	/*-------------------
	| Current Operator. |
	-------------------*/
	case	OPERATR:
		pr_sptr = clip (tspm_rec.op_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-40.40s",pr_sptr);
		}
		break;

	/*----------------
	| Last Operator. |
	----------------*/
	case	LST_OPR:
		pr_sptr = clip (tspm_rec.lst_op_code);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-14.14s",pr_sptr);
		}
		break;

	/*------------------
	| Last Phone Date. |
	------------------*/
	case	LST_PHN:
		partPrinted = TRUE;
		fprintf (fout, DateToString (tspm_rec.lphone_date));
		break;

	/*------------------
	| Best Phone Time. |
	------------------*/
	case	BST_PHN:
		pr_sptr = clip (tspm_rec.best_ph_time);
		if (*pr_sptr)
		{
			partPrinted = TRUE;
			fprintf (fout,"%-5.5s",pr_sptr);
		}
		break;

	default:
		break;
	}

	fflush (fout);
}

/*====================================================
| GetDate (long-date) returns date in 23 January 1986 . |
====================================================*/
char*
GetDate (
 long   cur_date)
{
	int		day,
			mon,
			year;

	DateToDMY (cur_date, &day, &mon, &year);
	sprintf (err_str, "%d %s %04d", day, mth [mon -1], year);
	return (err_str);
}

int
spec_valid (
 int     field)
{
	if (LCHECK ("letterCode"))
	{
		if (SRCH_KEY)
		{
			SrchTslh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (tslh_rec.co_no, comm_rec.co_no);
		sprintf (tslh_rec.let_code, "%-10.10s", local_rec.letterCode);
		cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
		if (cc)
		{
			/*-------------------------------
			| Letter Does Not Exist On File |
			-------------------------------*/
			print_mess (ML (mlStdMess109));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		if (tslh_rec.lett_type [0] == 'L')
		{
			/*-----------------------------------
			| This Is A Label Definition Letter |
			-----------------------------------*/
			print_mess (ML (mlTsMess096));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		hhlhHash = tslh_rec.hhlh_hash;
		sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
		DSP_FLD ("letterDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			/*-----------------
			| Invalid Printer |
			-----------------*/
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCustomer") || LCHECK ("endCustomer"))
	{
		if (dflt_used)
		{
			if (LCHECK ("startCustomer"))
			{
				strcpy (local_rec.startCustomer, "      ");
				sprintf (local_rec.startCustomerName, "%-40.40s", "First Customer");
				DSP_FLD ("startCustomerName");
			}
			else
			{
				strcpy (local_rec.endCustomer, "~~~~~~");
				sprintf (local_rec.endCustomerName, "%-40.40s", "Last Customer");
				DSP_FLD ("endCustomerName");
			}

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		if (LCHECK ("startCustomer"))
			strcpy (cumr_rec.dbt_no, pad_num (local_rec.startCustomer));
		else
			strcpy (cumr_rec.dbt_no, pad_num (local_rec.endCustomer));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*----------------------------
			| Customer Not Found On File |
			----------------------------*/
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		tspm_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = find_rec (tspm, &tspm_rec, COMPARISON, "r");
		if (cc)
		{
			/*-----------------------------------
			| Customer Not Active For Telesales |
			-----------------------------------*/
			print_mess (ML (mlTsMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("startCustomer"))
			sprintf (local_rec.startCustomerName, "%-40.40s", cumr_rec.dbt_name);
		else
			sprintf (local_rec.endCustomerName, "%-40.40s", cumr_rec.dbt_name);

		DSP_FLD ("startCustomerName");
		DSP_FLD ("endCustomerName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startArea") || LCHECK ("endArea"))
	{
		if (LCHECK ("endArea") && FLD ("endArea") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used && LCHECK ("startArea"))
		{
			strcpy (local_rec.startArea, "  ");
			strcpy (local_rec.endArea,   "~~");
			strcpy (local_rec.startAreaName, ML ("All Areas"));
			strcpy (local_rec.endAreaName, 	 ML ("All Areas"));
			DSP_FLD ("startArea");
			DSP_FLD ("endArea");
			DSP_FLD ("startAreaName");
			DSP_FLD ("endAreaName");
			FLD ("endArea") = NA;
			return (EXIT_SUCCESS);
		}

		FLD ("endArea") = NO;
		
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exaf_rec.co_no,comm_rec.co_no);
		if (LCHECK ("startArea"))
			sprintf (exaf_rec.area_code, "%2.2s", local_rec.startArea);
		else
			sprintf (exaf_rec.area_code, "%2.2s", local_rec.endArea);

		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			/*------------------
 			| Area not on file.|
			------------------*/
			errmess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (LCHECK ("startArea"))
		{
			sprintf (local_rec.startArea, "%2.2s ", exaf_rec.area_code);
			sprintf (local_rec.startAreaName, "%-40.40s", exaf_rec.area);
		}
		else
		{
			sprintf (local_rec.endArea, "%2.2s ", exaf_rec.area_code);
			sprintf (local_rec.endAreaName, "%-40.40s", exaf_rec.area);
		}

		DSP_FLD ("startAreaName");
		DSP_FLD ("endAreaName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startSector") || LCHECK ("endSector"))
	{
		if (LCHECK ("endSector") && FLD ("endSector") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used && LCHECK ("startSector"))
		{
			strcpy (local_rec.startSector, "   ");
			strcpy (local_rec.endSector,   "~~~");
			strcpy (local_rec.startSectorName, 	ML ("All Business Sectors"));
			strcpy (local_rec.endSectorName, 	ML ("All Business Sectors"));
			DSP_FLD ("startSector");
			DSP_FLD ("endSector");
			DSP_FLD ("startSectorName");
			DSP_FLD ("endSectorName");
			FLD ("endSector") = NA;
			return (EXIT_SUCCESS);
		}
		FLD ("endSector") = NO;
		
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excl_rec.co_no,comm_rec.co_no);
		if (LCHECK ("startSector"))
			strcpy (excl_rec.class_type,local_rec.startSector);
		else
			strcpy (excl_rec.class_type,local_rec.endSector);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			/*--------------------------------
			| Business Sector is not on file |
			--------------------------------*/
			errmess (ML (mlStdMess164));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (LCHECK ("startSector"))
			strcpy (local_rec.startSectorName, excl_rec.class_desc);
		else
			strcpy (local_rec.endSectorName, excl_rec.class_desc);

		DSP_FLD ("startSectorName");
		DSP_FLD ("endSectorName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startDate"))
	{
		if (dflt_used)
		{
			local_rec.startDate = 0L;
			local_rec.endDate 	= 0L;
			DSP_FLD ("endDate");
			FLD ("endDate") = NA;
			return (EXIT_SUCCESS);
		}

		FLD ("endDate") = NO;
		if (local_rec.startDate == 0L)
		{
			/*--------------
			| Invalid Date |
			--------------*/
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (local_rec.endDate == 0L)
			{
				local_rec.endDate = local_rec.startDate;
				DSP_FLD ("endDate");
			}
			else
			{
				if (local_rec.endDate < local_rec.startDate)
				{
					/*--------------------------------
					| End date must be greater than  |
					| or equal to the start date 	 |
					--------------------------------*/
					print_mess (ML (mlStdMess026));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate"))
	{
		if (local_rec.endDate < local_rec.startDate)
		{
			/*----------------------------------------------------------
			| End date must be greater than or equal to the start date |
			-----------------------------------------------------------*/
			print_mess (ML (mlStdMess026));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*========================================
| Search routine for Letter master file. |
========================================*/
void
SrchTslh (
 char*  key_val)
{
	_work_open (10,0,40);
	save_rec ("#Letter No ","#Description");
	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp (tslh_rec.co_no, comm_rec.co_no) &&
	          !strncmp (tslh_rec.let_code, key_val, strlen (key_val)))
	{
		if (tslh_rec.lett_type [0] == 'L')
		{
			cc = find_rec (tslh, &tslh_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (tslh_rec.let_code, tslh_rec.let_desc);
		if (cc)
			break;

		cc = find_rec (tslh, &tslh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tslh_rec.co_no, comm_rec.co_no);
	sprintf (tslh_rec.let_code, "%-10.10s", key_val);
	cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tslh, "DBFIND");

	sprintf (local_rec.letterDesc, "%-40.40s", tslh_rec.let_desc);
}

/*==========================================
| Search routine for Business Sector File. |
==========================================*/
void
SrchExcl (
 char*  key_val)
{
	_work_open (3,0,40);
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,key_val);
	save_rec ("#No.","#Business Sector Description");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (excl_rec.class_type, key_val,strlen (key_val)) && 
		!strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "excl", "DBFIND");
}

/*======================================
| Search routine for Area Master File. |
======================================*/
void
SrchExaf (
 char*  key_val)
{
	_work_open (2,0,40);
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,key_val);
	save_rec ("#No","#Area Description");
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strncmp (exaf_rec.area_code,key_val,strlen (key_val)) &&
		      !strcmp (exaf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

int
heading (
 int    scn)
{
	if (scn != cur_screen && scn != 0)
		scn_set (scn);

	if (clearOk)
		clear ();

	if (scn == 1)
	{
		/*-------------------------
		| Print Telesales Mailers |
		-------------------------*/
		rv_pr (ML (mlTsMess097), 30, 0, 1);

		box (0, 3, 80, 15);

		line_at (5,1,79);
		line_at (7,1,79);
		line_at (10,1,79);
		line_at (13,1,79);
		line_at (16,1,79);
	}
	line_at (1,0,80);
	line_at (20,0,80);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	if (scn != 0)
		scn_write (scn);
    return (EXIT_SUCCESS);
}

/*--------------------------------------
| Fix edge of boxes with 'T' junctions |
--------------------------------------*/
int
fix_edge (
 int    line_no)
{
/*
	move (0,line_no);
	PGCHAR (10);
	move (79,line_no);
	PGCHAR (11);
	return (EXIT_SUCCESS);
*/
	return (EXIT_SUCCESS);
}

