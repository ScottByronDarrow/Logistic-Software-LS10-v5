/*=====================================================================
|  Copyright (C) 1986 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (Menu.c        )                                   |
|  Program Desc  : (Main System Menu System.                    )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  MENUSYS/{COMM,User_secure,*.mdf,$LOGNAME.fa}      |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (30/08/88)      | Modified  by  : Scott Darrow.    |
|                : (15/05/90)      |               : Trevor van Bremen|
|                : (30/08/90)      | Modified  by  : Scott Darrow.    |
|                : (20/12/90)      | Modified  by  : Trevor van Bremen|
|  Date Modified : (09/04/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (02/05/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (25/05/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (21/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (09/11/92)      | Modified  by : Simon Dubey.      |
|  Date Modified : (05/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (23/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (19/04/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (21/06/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (26/07/93)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (27/07/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (02/08/93)      | Modified by : Simon Dubey.       |
|  Date Modified : (15.06.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (10.10.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (06.11.98)      | Modified by : Marnie I. Organo   |
|  Date Modified : (31/08/1999)    | Modified by : Alvin Misalucha    |
|                                                                     |
|  Comments      : (15/05/90) Added license checking etc.             |
|                : (28/08/90) - Updated to fix Alarms. S.B.D.         |
|                : (20/12/90) - Changes as necessary for pswitch.     |
|                : (09/04/91) - Updated to add message.               |
|                : (02/05/91) - Speed up Windows.                     |
|  (25/05/92)    : Security codes are now up to 8 lower case letters  |
|                : and digits separated by pipes.                     |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|  (09/11/92)    : PSL 8060 mod_env PSL_ENV_NAME from vaiable kept in |
|                : COMM (originally from comr_env_name)               |
|  (05/02/92)    : PSL 8060 changed to use putenv NOT mod_env.        |
|  (23/02/92)    : PSL 8060 changed to use putenv more often.         |
|  (19/04/93)    : DML 8856.  Store new company's environment in      |
|                : PSL_ENV_NAME if company changes.                   |
|  (21/06/93)    : Uninitialised automatic variables cause mem faults |
|                : on NCR hardware. PSL 9086.                         |
|  (26/07/93)    : Updated to use MENU_PATH.                          |
|  (27/07/93)    : Extended MENU_PATH functionality to similar to     |
|                  PATH. Renamed to PSL_MENU_PATH                     |
|  (02/08/93)    : inlude sys/types.h as this will not compile on NCR |
|  (16.06.94)    : On shell out, use login shell type                 |
|  (10.10.94)    : PSL 11417 Changes arising from v8 cleanup          |
|  (06.11.98)    : Updated to fix Y2K bug.                            |
|  (31/08/1999)  : Converted to ANSI format.                          |
|                                                                     |
| $Log: menu.c,v $
| Revision 5.4  2001/08/09 05:13:31  scott
| Updated to use FinishProgram ();
|
| Revision 5.3  2001/08/06 23:32:25  scott
| RELEASE 5.0
|
| Revision 5.2  2001/07/25 02:18:14  scott
| Update - LS10.5
|
| Revision 5.0  2001/06/19 08:08:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/30 08:58:36  cha
| Updated to make compatible to HP.
|
| Revision 4.0  2001/03/09 02:29:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/13 07:16:05  jhon
| Added wait() for additional oracle7.3.4 process
|
| Revision 3.1  2000/11/16 07:37:37  scott
| Updated as sub-menu's did not re-display correctly.
|
| Revision 3.0  2000/10/10 12:16:07  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.4  2000/09/19 12:40:36  gerry
| Reset forceRead in InitiliseMenu as it screws up comm_recs
|
| Revision 2.3  2000/08/29 07:31:03  scott
| Updated for dbopen/close and multilingual
|
| Revision 2.2  2000/08/21 05:43:26  scott
| Updated to add dbclose
|
| Revision 2.1  2000/08/14 23:39:27  scott
| Updated to add option rf_menu for use in RF terminals.
|
| Revision 2.0  2000/07/15 09:00:18  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.30  2000/07/12 01:43:59  scott
| Updated to fix locking / tockens problem with Linux and menu system in general.
| Implemented changes suggested by Trevor and removed read_comm () as this called
| ttyslt () and removed the lock.
|
| Revision 1.29  2000/07/10 01:52:34  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.28  2000/07/05 01:32:46  scott
| Updated to re-define curr_menu to currentMenu due to conflict with tab routines.
|
| Revision 1.27  2000/02/18 01:56:22  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.26  2000/02/01 03:25:55  scott
| Updated to ensure comm record selected in right order.
|
| Revision 1.25  2000/02/01 02:05:53  scott
| Updated to ensure comm record read when all select called.
|
| Revision 1.24  2000/01/21 06:24:02  scott
| Updated to fix timing problem with login and TERM_SLOT
|
| Revision 1.23  1999/12/06 01:47:15  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.22  1999/11/16 09:41:56  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.21  1999/10/16 04:56:35  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.20  1999/09/29 02:03:17  scott
| Updated from Ansi testing
|
| Revision 1.19  1999/09/17 07:26:59  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.18  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.17  1999/06/15 02:36:51  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: menu.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/menu/menu.c,v 5.4 2001/08/09 05:13:31 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<MenuHeader.h>
#include	<account.h>
#include	<get_lpno.h>
#include	<license2.h>
#include 	<pwd.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>

FILE	*_umess;

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

struct	OPT	{
	int	_row;			/* row number				*/
	int	_col;			/* col number				*/
	char	*_name;		/* option description (name)		*/
	char	*_program;
} option [MAX_OPT + 1];

struct	{
	int	_row;		/* position of first 			*/
	int	_col;		/* option in quadrant			*/
	int	_off;		/* used to calculate row position	*/
} quads [MAX_EXTRA];

struct	{
	int	_row;
	int	_col;
	int	_width;
	int	_depth;
} Boxes [MAX_EXTRA];

struct	{
	int	_row;
	int	_col;
	int	_length;
} Lines [MAX_EXTRA];

struct	{
	int	_row;
	int	_col;
	int	_indx;
} Graph [MAX_EXTRA];

struct	{
	char	*_str;
	int	_row;
	int	_col;
	int	_rv;
} comment [MAX_EXTRA];

	/*=====================+
	 | System Common File. |
	 +=====================*/
#define	COMM_NO_FIELDS	20

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_cc_short"},
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dp_short"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_gl_date"},
		{"comm_fiscal"},
		{"comm_env_name"},
		{"comm_stat_flag"}
	};

	struct tag_commRecord
	{
		int		TERMNO;
		char	CONO [3];
		char	CONAME [41];
		char	COSHORT [16];
		char	BRNO [3];
		char	BRNAME [41];
		char	BRSHORT [16];
		char	WHNO [3];
		char	WHNAME [41];
		char	WHSHORT [10];
		char	DPNO [3];
		char	DPNAME [41];
		char	DPSHORT [16];
		Date	DBTDATE;
		Date	CRDDATE;
		Date	INVDATE;
		Date	GLDATE;
		int		FIS;
		char	ENV_NAME [61];
	}	comm_rec;

struct	co {
	char	*lbl;
	char	*val;
} com [] = {
	{"CONO"},
	{"CONAME"},
	{"COSHORT"},
	{"BRNO"},
	{"BRNAME"},
	{"BRSHORT"},
	{"WHNO"},
	{"WHNAME"},
	{"WHSHORT"},
	{"DPNO"},
	{"DPNAME"},
	{"DPSHORT"},
	{"DBTDATE"},
	{"CRDDATE"},
	{"INVDATE"},
	{"GLDATE"},
	{"GLPER"},
	{"FIS"},
	{"ENV_NAME"},
	{""}
};

	int		window 			= 0;
	int		rfMenu 			= 0;
	int		oldMaxRow 	= 0;
	int		oldMaxCol 	= 0;

	int		fastAccessOff 	= FALSE,
			trailerOff 		= FALSE,
			shellOff 		= FALSE,
			subMenuOff 		= FALSE,
			menuNameOff 	= FALSE,
			headingOff 		= FALSE;

	char	oldMenu [61];
	char	currentCompany [3];
	int		pass_trys = 0;

struct	tm	*ts;

/*----------------------------------------------------------------------
| Set up Array to hold Days of the week used with wday in time struct. |
----------------------------------------------------------------------*/
static char *day [] = {
	"Sunday",	"Monday",	"Tuesday",	"Wednesday",
	"Thursday",	"Friday",	"Saturday"
};
/*-------------------------------------------------------------------
| Set up Array to hold Months of Year used with mon in time struct. |
-------------------------------------------------------------------*/
static char *mth [] = {
	"January",		"February",	"March",	"April",
        "May", 		"June", 	"July", 	"August", 
	"September", 	"October", 	"November", "December"
};

static char *abr [] = {
	"st","nd","rd","th","th","th","th","th","th","th",
	"th","th","th","th","th","th","th","th","th","th",
	"st","nd","rd","th","th","th","th","th","th","th",
	"st"
};

/*==========================
| Function prototypes.     |
==========================*/
int		CheckSecurity	 	(char *, char *);
int		GetCheckComm	 	(void);
int		Heading			 	(void);
int		LoadMenu		 	(char *);
int		MenuHelp		 	(int, int);
int		SetCoordnance		(void);
int		ShowMenu		 	(void);
void	BusyMessage		 	(int);
void	CheckForBackup	 	(void);
void	DisplayMenu		 	(char *);
void	DisplayTrailer	 	(void);
void	DownShiftFuction	(char *);
void	DrawMask		 	(void);
void	DrawMenu	 		(void);
void	GetUserSecureFile	(void);
void	InitiliseMenu	 	(void);
void	LoadBoxes		 	(char *);
void	LoadComments	 	(char *);
void	LoadGraphics	 	(char *);
void	LoadLines		 	(char *);
void	LoadQuadrant	 	(char *);
void	LoadSpecial	 		(void);
void	MenuKill		 	(int);
void	MenuShellOut		(int);
void	MenuTimeout		 	(int);
void	PrintHeading		(void);
void	RunMenu		 		(int);
void	SubtituteFunction	(char *);
void	shutdown_prog	 	(char *);

extern	int	forceRead;
/*==========================
| Main processing routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;

	forceRead	=	TRUE;
	_mail_ok = FALSE;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	isSubMenu 	= !strncmp (sptr,"smenu", 5);
	psMenu 		= !strncmp (sptr,"ps_menu", 7);
	rfMenu 		= !strncmp (sptr,"rf_menu", 7);

	abc_dbopen ("data");

	InitiliseMenu ();

	if (argc == 2)
	{
		if (sys_exec (argv [1]))
		{
			shutdown_prog (ML ("No Company / Branch / Warehouse selected"));
		}
	#ifdef ORA734
		wait (&status);
	#endif	
	}
	GetCheckComm ();
	window = 0;
	strcpy (currentCompany, "");
	DisplayMenu ((argc == 3) ? argv [2] : (char *)0);
	abc_dbclose ("data");
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 char *	reason)
{
	rset_tty ();
	clear ();
	snorm ();
	puts (reason);
	crsr_on ();
	if (!isSubMenu && !psMenu && !rfMenu)
		check_logout (comm_rec.TERMNO);

	exit (0);
}

void
InitiliseMenu (void)
{
	int	lc_errno = 0;

	char	*sptr = chk_env ("TIMEOUT");

	if (sptr != (char *)0)
		timeout = atoi (sptr);
	else
		timeout = 0;

	currentUser = getenv ("LOGNAME");

	if (currentUser == (char *)0)
	{
		shutdown_prog (ML ("You Don't Have A LOGNAME set"));
	}

	init_scr ();
	set_tty ();
	crsr_off ();

	if (isSubMenu)
		snorm ();

	CheckForBackup ();
	GetUserSecureFile ();

	forceRead	=	TRUE;
	terminalNumber = ttyslt ();
	forceRead	=	FALSE;
	if (terminalNumber != -4)
		lc_errno = lc_check (&des_rec, &lic_rec);

	if (lc_errno >= 0)
		lc_errno = check_login (terminalNumber, lic_rec.max_usr);

	if (!isSubMenu && !psMenu && !rfMenu)
		ser_msg (lc_errno, &lic_rec, TRUE);

	signal (SIGINT,MenuKill);
	signal (SIGALRM,MenuTimeout);
	alarm (60);
}

void
DisplayMenu (
 char *	mname)
{
	int		lineNumber = 1;
	char	*sptr;

	strcpy (currentMenu, (mname != (char *)0) ? mname : start_menu);
	
	changeMenu = !window;

	while (1)
	{
		LoadMenu (currentMenu);

		while (1)
		{
			/*=========================
			| set up company specific |
			| env variables filename  |
			=========================*/
			sprintf (err_str, "PSL_ENV_NAME=%s", com [15].val);
			putenv (strdup (err_str));

			DrawMenu ();
			lineNumber = ShowMenu ();

			/*-----------------------
			| Program to Execute	|
			-----------------------*/
			if (strncmp (option [lineNumber]._program,"menu ",5) &&
			     strncmp (option [lineNumber]._program,"pmenu ",6))
			{
				RunMenu (lineNumber);
				set_tty ();
				snorm ();
				oldMaxRow = 0;
				oldMaxCol = 0;
			}
			if (!strncmp (option [lineNumber]._program,"pmenu ",6))
			{
				strcpy (oldMenu, currentMenu);

				sptr = option [lineNumber]._program + 5;
				while (*sptr && *sptr == ' ')
					sptr++;

				strcpy (currentMenu,sptr);
				window = 1;
				break;
			}

			if (!strncmp (option [lineNumber]._program,"menu ",5))
			{
				window = 0;
				sptr = option [lineNumber]._program + 4;
				while (*sptr && *sptr == ' ')
					sptr++;

				strcpy (currentMenu,sptr);
				break;
			}
			if (window)
			{
				window = 0;
				LoadMenu (oldMenu);
				DrawMenu ();
				window = 1;
				break;
			}
		}
	}
}

void
CheckForBackup (void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	filename [101];

	sprintf (filename,"%s/BIN/.backup", (sptr == (char *)0) ? "/usr/LS10.5" : sptr);
	if (access (filename,00) == 0)
		shutdown_prog ("A backup is currently in progress");
}

int
GetCheckComm (void)
{
	int		periodMonth;
	char	fiscalYear [3];
	char	Period [3];


	/*------------------------------------------------------------------
	| Don't change this as you don't want to use read_comm as it calls |
	| set_slt and removed token lock. 								   |
	------------------------------------------------------------------*/
	open_rec ("comm", comm_list, COMM_NO_FIELDS, "comm_term");
	comm_rec.TERMNO	=	terminalNumber;
	cc = find_rec ("comm", &comm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comm", "DBFIND");
	
	abc_fclose ("comm");

	sprintf (fiscalYear,"%2d",comm_rec.FIS);

	com [0].val  = clip (comm_rec.CONO);
	com [1].val  = clip (comm_rec.CONAME);
	com [2].val  = clip (comm_rec.COSHORT);
	com [3].val  = clip (comm_rec.BRNO);
	com [4].val  = clip (comm_rec.BRNAME);
	com [5].val  = clip (comm_rec.BRSHORT);
	com [6].val  = clip (comm_rec.WHNO);
	com [7].val  = clip (comm_rec.WHNAME);
	com [8].val  = clip (comm_rec.WHSHORT);
	com [9].val  = strdup (DateToString (comm_rec.DBTDATE));
	com [10].val = strdup (DateToString (comm_rec.CRDDATE));
	com [11].val = strdup (DateToString (comm_rec.INVDATE));
	com [12].val = strdup (DateToString (comm_rec.GLDATE));
	DateToDMY (comm_rec.GLDATE, NULL, &periodMonth, NULL);
	sprintf (Period, "%02d", periodMonth);
	com [13].val = strdup (Period);
	com [14].val = strdup (fiscalYear);
	com [15].val = clip (comm_rec.ENV_NAME);

	/*------------------------------------
	| Company MAY have changed. Therfore |
	| set environment for new company.   |
	------------------------------------*/
	if (strcmp (currentCompany, com [0].val))
	{
		strcpy (currentCompany, com [0].val);

		sprintf (err_str, "PSL_ENV_NAME=%s", com [15].val);
		putenv (strdup (err_str));
	}
	return (EXIT_SUCCESS);
}

void
GetUserSecureFile (void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	*tptr;
	char	filename [101];

	security = (char *)0;

	sprintf (filename,"%s/BIN/MENUSYS/User_secure", (sptr != (char *)0) ? sptr : "/usr/LS10.5");

	if ((fmenu = fopen (filename,"r")) == 0)
	{
		sprintf (err_str,"Error in %s during (FOPEN)",filename);
		sys_err (err_str,errno,PNAME);
	}

	sptr = fgets (data,DATA_SIZE,fmenu);

	while (sptr != (char *)0)
	{
		tptr = sptr;
		while (*tptr != ' ' && *tptr != '\t')
			tptr++;

		*tptr = '\0';
		/*-------------------------------------------
		| Find the Appropriate Entry for the User	|
		-------------------------------------------*/
		if (!strcmp (data,currentUser))
		{
			sptr = tptr + 1;
			tptr = strchr (sptr,'<');
			/*-------------------------------
			| Found Start of security	|
			-------------------------------*/
			if (tptr != (char *)0)
			{
				sptr = tptr + 1;
				tptr = strchr (sptr,'>');
				if (tptr != (char *)0)
				{
					*tptr = '\0';
					security = strdup (sptr);
					break;
				}
			}
		}
		sptr = fgets (data,DATA_SIZE,fmenu);
	}
	fclose (fmenu);

	if (security == (char *)0)
		shutdown_prog ("You Are Not A Valid User");

	DownShiftFuction (security);
}

void
DrawMenu (void)
{
	int	i;

	if (!window && !rfMenu)
	{
		if (!oldMaxRow)
		{
			clear ();
			box (0,0,WIDTH,22);
			
			if (!headingOff)
				PrintHeading ();
		}
		DrawMask ();

		for (i = 3; i < oldMaxRow + 1; i++) 
			print_at (i,1,"%*.*s", oldMaxCol, oldMaxCol," ");
		
		oldMaxRow = 0;
		oldMaxCol = 0;
	}

	/*---------------------------------------------------
	| Only Draw Default Box if No Special Bits Exist	|
	---------------------------------------------------*/
	if (maxBox == 0 && maxLine == 0 && maxGraphics == 0)
	{
		if (window || rfMenu)
			box (minCol, minRow, maxCol - minCol, maxRow - minRow -1);
		else
			box (minCol, minRow, maxCol - minCol, maxRow - minRow -1);
	}
	if (window || rfMenu)
	{
		print_at (minRow + 1, minCol + 1, "%*.*s",maxCol - minCol - 2,maxCol - minCol - 2," ");
		move (minCol + 1,minRow + 2);
		line (maxCol - minCol - 1);
		print_at (minRow + 4, minCol + 1, "%*.*s",maxCol - minCol - 2,maxCol - minCol - 2," ");
		oldMaxRow = maxRow;
		oldMaxCol = (maxCol - minCol) + 1;
	}

	for (i = 0;i < maxBox;i++)
		box (Boxes [i]._col,Boxes [i]._row,Boxes [i]._width,Boxes [i]._depth);

	for (i = 0;i < maxLine;i++)
	{
		move (Lines [i]._col,Lines [i]._row);
		line (Lines [i]._length);
	}

	for (i = 0;i < maxGraphics;i++)
	{
		move (Graph [i]._col,Graph [i]._row);
		PGCHAR (Graph [i]._indx);
	}

	for (i = 0;i < maxComment;i++)
	{
		us_pr (comment [i]._str, comment [i]._col,
		       comment [i]._row, comment [i]._rv);
	}

	sprintf (err_str, " %s ", option [0]._name);
	if (!menuNameOff)
		us_pr (err_str, option [0]._col - 2 ,option [0]._row,1);

	for (i = 1;i < maxOptions;i++)
	{
		if (window || rfMenu)
		{
			print_at (option [i]._row, option [i]._col - 5, " %2d. %-*.*s",
				i,maxCol - minCol - 7, maxCol - minCol - 7,option [i]._name);
		}
		else
		{
			print_at (option [i]._row,option [i]._col - 4,"%2d. %s",i,option [i]._name);
		}
	}
	if (!window && !rfMenu)
	{
		if (isSubMenu)
			rv_pr (ML (" [Sub Menu Running.]"), (WIDTH - 21) / 2 + 2,20,1);
	}
}

void
DrawMask (void)
{
	if (!headingOff)
	{
		move (1,2);
		line (WIDTH - 1);
	}

	if (!trailerOff)
		DisplayTrailer ();
}

void
DisplayTrailer (void)
{
	move (1,21);
	line (WIDTH - 1);

	sprintf (err_str, "Co #: %s %-15.15s  Br #: %s %-15.15s  Wh #: %s %-15.15s",
				com [0].val, com [2].val, com [3].val, com [5].val,
				com [6].val, com [8].val);
	so_pr (err_str,2,22,1);
}

int
ShowMenu (void)
{
	int		key = 0;
	int		fn = 0;
	int		curr = 1;
	int		last = -1;
	int		nindx = 0;
	int		findx = 0;
	int		rv_flop = FALSE;
	char	oldMenu [61];
	char	num_str [3];
	char	fast_str [FASTLEN + 4];

	rv_pr (option [curr]._name,option [curr]._col,option [curr]._row,1);
	num_str [0] = '\0';
	strcpy (fast_str,"        ");

	while (1)
	{
		if (!window && !fastAccessOff && !rfMenu)
		{
			if (!findx)
			{
				if (rv_flop)
				{
					rv_pr
					 (	
						option [curr]._name, 
						option [curr]._col,
						option [curr]._row,1
					);
					rv_flop = FALSE;
				}
				print_at (20,2,"               %-*.*s ", 
					    FASTLEN + 2, FASTLEN + 2, " ");
			}
			else
			{
				if (!rv_flop)
				{
					rv_pr (option [curr]._name,
				       	       option [curr]._col,
				       	       option [curr]._row,0);
					rv_flop = TRUE;
				}

				print_at (20,2,"%R[Fast Access : %-*.*s]", 
					    FASTLEN + 2, FASTLEN + 2, fast_str);
			}
		}
		if (last != curr)
		{
			if (valid_opt (last))
				rv_pr (option [last]._name,option [last]._col,option [last]._row,0);
			if (valid_opt (curr))
				rv_pr (option [curr]._name,option [curr]._col,option [curr]._row,1);
		}

		last = curr;
		c = getkey ();

		if (c > 0)
			timer = 0;

		if (c >= FN17 && c < FN32)
		{
			fn = c;
			c = FN17;
		}

		if (_isdigit (c))
		{
			key = c;
			c = '0';
		}

		if (_islower (c) || _isupper (c))
		{
			key = to_upper (c);
			c = 'A';
		}

		switch (c)
		{
		case	16:
			if (window || rfMenu)
			{
				putchar (BELL);
				break;
			}
			break;

		case	'A':
			if (window || fastAccessOff || rfMenu)
			{
				putchar (BELL);
				break;
			}

			fast_str [findx++] = key;
			if (findx == (FASTLEN + 2))
			{
				findx--;
				putchar (BELL);
			}
			nindx = 0;
			num_str [0] = '\0';
			
			break;

		case	'0':
			if (findx)
			{
				fast_str [findx++] = key;
				if (findx == (FASTLEN + 2))
				{
					findx--;
					putchar (BELL);
				}
				break;
			}
			else
			{
				num_str [nindx++] = key;
				num_str [nindx] = '\0';
				if (!valid_opt (atoi (num_str)))
				{
					nindx = 0;
					num_str [nindx++] = key;
					num_str [nindx] = '\0';
					if (!valid_opt (atoi (num_str)))
					{
						nindx = 0;
						num_str [0] = '\0';
						putchar (BELL);
					}
					else
						curr = atoi (num_str);
				}
				else
					curr = atoi (num_str);
			}
			break;

		case	HELP:
			if (window || rfMenu)
			{
				putchar (BELL);
				break;
			}

			MenuHelp (0,curr);
			DrawMenu ();
			last = 0;
			break;

		case	FN6:
			if (window || rfMenu)
			{
				putchar (BELL);
				break;
			}

			MenuHelp (1,curr);
			DrawMenu ();
			last = 0;
			break;

		case	FN7:
			if (window || rfMenu)
			{
				putchar (BELL);
				break;
			}

			MenuShellOut (RUN_HELP);
			DrawMenu ();
			last = 0;
			break;

		case	FN8:
			if (window || subMenuOff || rfMenu)
			{
				putchar (BELL);
				break;
			}

			MenuShellOut (EXTERN);
			DrawMenu ();
			last = 0;
			break;

		case	'!':
			if (window || shellOff || psMenu || rfMenu)
			{
				putchar (BELL);
				break;
			}

			MenuShellOut (SHELL);
			DrawMenu ();
			last = 0;
			break;

		case	FN3:
			crsr_off ();
			DrawMenu ();
			last = 0;
			break;

		case	8:
			/*-------------------------------
			| Keying the option number	|
			-------------------------------*/
			if (nindx)
			{
				num_str [nindx--] = '\0';
				if (!valid_opt (atoi (num_str)))
					putchar (BELL);
				else
					curr = atoi (num_str);
				break;
			}

			/*-------------------------------
			| Keying for Fast Access	|
			-------------------------------*/
			if (findx)
			{
				fast_str [--findx] = ' ';
				break;
			}

		case	FN15:
		case	UP_KEY:
		case	11:

		case	LEFT_KEY:
			strcpy (fast_str,"      ");
			findx = 0;
			num_str [0] = '\0';
			nindx = 0;
			if (--curr == 0)
				curr = maxOptions - 1;
			break;

		case	FN13:
		case	FN14:
		case	DOWN_KEY:
		case	10:
		case	' ':

		case	RIGHT_KEY:
		case	12:
			strcpy (fast_str,"      ");
			findx = 0;
			num_str [0] = '\0';
			nindx = 0;
			if (++curr == maxOptions)
				curr = 1;
			break;

		case	'\r':
			if (valid_opt (last))
				rv_pr (option [last]._name,option [last]._col,option [last]._row,0);

			/*---------------
			| Fast Access	|
			---------------*/
			if (findx)
			{
				strcpy (oldMenu,currentMenu);

				curr = get_fast (fast_str);
				strcpy (fast_str,"        ");
				findx = 0;
				nindx = 0;
				num_str [0] = '\0';
				/*-----------------------
				| Fast Access Failed	|
				-----------------------*/
				if (curr <= 0)
				{
					curr = last;
					last = -1;
					putchar (BELL);
					break;
				}

				/*---------------
				| Load New Menu	|
				---------------*/
				if (strcmp (currentMenu,oldMenu))
				{
					last = 0;
					LoadMenu (currentMenu);
					DrawMenu ();
				}
			}
			else
				return (curr);
			break;

		case	FN17:
			MenuShellOut (fn);
			DrawMenu ();
			last = 0;
			break;

		case	FN32:
			if (psMenu)
			{
				putchar (BELL);
				break;
			}
			shutdown_prog ("* Menu Exited *");
			break;

		case	-1:
			if (timer_ticked)
				timer_ticked = FALSE;
			else
				shutdown_prog ("*** Menu System Now Exited ***");
			break;

		default:
			putchar (BELL);
		}
	}
}

int
LoadMenu (
 char *	mname)
{
	register	int	i;
	int		dflt_used = 1;
	int		valid;
	int		maxWidth = 0;
	char	delimit [3];
	char	secure [2049];
	char	*tptr;
	char	*sptr = getenv ("PROG_PATH");
	char	*mptr = getenv ("PSL_MENU_PATH");
	char	filename [101];

	/*-----------------------------------------------
	| The uninitialised status of secure was the	|
	| cause of MANY days head scratching on the NCR	|
	-----------------------------------------------*/
	strcpy (secure, "");

	for (i = 0;i < maxOptions;i++)
	{
		free (option [i]._name);
		free (option [i]._program);
	}

	for (i = 0;i < maxComment;i++)
		free (comment [i]._str);

	for (i = 0;i < MAX_EXTRA;i++)
		quads [i]._off = 0;

	maxOptions	 = 0;
	maxQuadrants = 0;
	maxBox		 = 0;
	maxLine		 = 0;
	maxGraphics	 = 0;
	maxComment	 = 0;

	/*--------------------------------------------------------
	| Look for a valid menu-file in the following manner	 |
	| 1. colon seperated paths in MENU_PATH	 				 |
	| 2. if above fails, PROG_PATH							 |
	--------------------------------------------------------*/
	*filename = '\0';
	while (mptr && *mptr)
	{
		char	base [128];
		char	*fptr = base;
		struct stat	FileInfo;

		while (*mptr && *mptr != ':')
			*fptr++ = *mptr++;

		*fptr = '\0';
		if (*mptr)
			mptr++;

		/*---------------------------
		| Check for file existence	|
		---------------------------*/
		sprintf (filename, "%s/%s", *base ? base : "." , mname);
		if (stat (filename, &FileInfo) == 0)
			break;				/* success */
		else
			*filename = '\0';		/* invalidate */
	}
	/*---------------------------------------------------
	| Couldn't make a match, use PROG_PATH to form name |
	---------------------------------------------------*/
	if (!*filename)
		sprintf (filename, "%s/BIN/%s", sptr ? sptr : "/usr/LS10.5", mname);

	if ((fmenu = fopen (filename,"r")) == 0)
	{
		crsr_on ();
		sprintf (err_str,"Error in %s during (FOPEN)",mname);
		sys_err (err_str,errno,PNAME);
	}

	strcpy (delimit,"()");

	sptr = fgets (data,DATA_SIZE,fmenu);

	while (maxOptions < MAX_OPT && sptr != (char *)0 && strncmp (sptr,"((",2))
	{
		valid = 0;
		*(sptr + strlen (sptr) - 1) = '\0';
		tptr = strchr (sptr,delimit [0]);
		if (tptr != (char *)0)
		{
			if (maxOptions != 0)
			{
				*tptr = '\0';
				option [maxOptions]._name = strdup (ML (clip (sptr)));
			}

			sptr = tptr + 1;
			tptr = strchr (sptr,delimit [1]);
			if (tptr != (char *)0)
			{
				if (maxOptions != 0)
				{
					*tptr = '\0';
					sprintf (secure,"%-.2048s",sptr);
				}

				valid = 1;

				/*-----------------------
				| Look for Quadrant		|
				-----------------------*/
				sptr = tptr + 1;
				tptr = strchr (sptr,'(');
				if (tptr != (char *)0)
				{
					if (maxOptions == 0)
					{
						*tptr = '\0';
						option [maxOptions]._name = strdup (ML (clip (data)));
					}

					/*-------------------
					| Look for Offset.	|
					-------------------*/
					sptr = tptr + 1;
					option [maxOptions]._col = atoi (sptr);
					tptr = strchr (sptr,',');
					if (tptr != (char *)0)
					{
						sptr = tptr + 1;
						option [maxOptions]._row = atoi (sptr);
						dflt_used = 0;
					}
					else
					{
						option [maxOptions]._col = 1;
						option [maxOptions]._row = 1;
					}
				}
				else
				{
					if (maxOptions == 0)
						option [maxOptions]._name = strdup (ML (clip (data)));

					option [maxOptions]._col = 1;
					option [maxOptions]._row = 1;
				}
			}
			else
			{
				sprintf (secure,"%-.2048s",sptr);
				valid = 1;
				option [maxOptions]._col = 1;
				option [maxOptions]._row = 1;
			}

			DownShiftFuction (secure);
		}

		sprintf (delimit,"<>");

		/*-------------------------------
		| Got security for the line		|
		| now check it ...				|
		| but don't bother for title	|
		-------------------------------*/
		if (valid && maxOptions != 0)
			valid = CheckSecurity (secure,security);

		/*-------------------------------
		| Skip the next line			|
		| as user has no access to line	|
		-------------------------------*/
		if (!valid)
		{
			sptr = fgets (data,DATA_SIZE,fmenu);
			if (sptr != (char *)0)
				sptr = fgets (data,DATA_SIZE,fmenu);
			continue;
		}
	
		sptr = fgets (data,DATA_SIZE,fmenu);

		if (sptr != (char *)0)
		{
			*(sptr + strlen (sptr) - 1) = '\0';
			option [maxOptions++]._program = strdup (sptr);
			if (!changeMenu && !strncmp (sptr,"menu ",5))
			{
				maxOptions--;
				option [maxOptions]._col = 1;
				option [maxOptions]._row = 1;
			}
			sptr = fgets (data,DATA_SIZE,fmenu);
		}
	}

	if (sptr != (char *)0 && !strncmp (sptr,"((",2) && !window)
		LoadSpecial ();

	if (rfMenu)
	{
		fastAccessOff 	= TRUE;
		shellOff 		= TRUE;
		subMenuOff 		= TRUE;
		trailerOff 		= TRUE;
		headingOff 		= TRUE;
	}

	fclose (fmenu);

	if (maxOptions < 2)
	{
		sprintf (err_str, ML (" User has no Options to %s "), option [0]._name);
		for (i = 0; i < 5; i++)
		{
			errmess (err_str);
			sleep (sleepTime);
		}
		return (LoadMenu (start_menu));
	}

	if (dflt_used || window || rfMenu)
	{
		if (!window && !rfMenu && dflt_used)
		{
			for (i = 0;i < maxOptions;i++)
			{
				option [i]._col = 1;
				option [i]._row = 1;
				if ((int) strlen (option [i]._name) > maxWidth)
					maxWidth = strlen (option [i]._name);
			}
			maxWidth += 8;

			minCol = (WIDTH - maxWidth) / 2;
			minRow = MIN_ROW;
			maxCol = minCol + maxWidth;
			maxRow = MIN_ROW + maxOptions;
			
			quads [0]._col = minCol + 6;
			quads [0]._row = MIN_ROW + 1;
			quads [0]._off = 0;

			maxQuadrants = 1;
		}

		if (window || rfMenu)
		{
			for (i = 0;i < maxOptions;i++)
			{
				option [i]._col = (i == 1 && window) ? 2 : 1;
				option [i]._row = (i == 1 && window) ? 2 : 1;
			}

			if (rfMenu)
			{
				minCol = 0;
				minRow = 0;
			}
			else
			{
				minCol = MIN_COL + 1;
				minRow = MIN_ROW;
			}
			maxCol = -1;
			maxRow = -1;

			quads [0]._col = minCol + 6;
			quads [0]._row = minRow + 3;
			quads [0]._off = 0;

			maxQuadrants = 1;
		}
	}
	else
	{
		/*-------------------------------------------------------
		| Set these here so we know to set them later !!!!		|
		-------------------------------------------------------*/
		maxCol = -1;
		maxRow = -1;

		/*---------------------------
		| no quadrants specified	|
		---------------------------*/
		if (maxQuadrants == 0)
		{
			for (i = 0;i < maxOptions;i++)
			{
				option [i]._col = 1;
				option [i]._row = 1;
				if ((int) strlen (option [i]._name) > maxWidth)
					maxWidth = strlen (option [i]._name);
			}
			maxWidth += 8;

			minCol = (WIDTH - maxWidth) / 2;
			quads [0]._col = minCol + 6;
			quads [0]._row = MIN_ROW + 1;
			quads [0]._off = 0;

			maxQuadrants = 1;
		}

		minCol = MAX_COL;
		minRow = MAX_ROW;

		for (i = 0;i < maxQuadrants;i++)
		{
			if (minCol + 6 > quads [i]._col)
				minCol = quads [i]._col - 6;

			if (minRow + 1 > quads [i]._row)
				minRow = quads [i]._row - 1;
		}
	}

	SetCoordnance ();
	return (EXIT_SUCCESS);
}

/*===========================
| Special bits to the menu	|
| ie Boxes, Lines, graphics	|
===========================*/
void
LoadSpecial (void)
{
	char	*sptr;

	sptr = fgets (data,DATA_SIZE,fmenu);

	while (sptr != (char *)0 && strncmp (sptr,"))",2))
	{
		if (!strncmp (sptr,"quadrant(",9))
			LoadQuadrant (sptr);

		if (!strncmp (sptr,"box(",4))
			LoadBoxes (sptr);

		if (!strncmp (sptr,"line(",5))
			LoadLines (sptr);

		if (!strncmp (sptr,"graphic(",8))
			LoadGraphics (sptr);

		if (!strncmp (sptr,"comment(",8))
			LoadComments (sptr);

		if (!strncmp (sptr,"SHELL_ON",8))
			shellOff = FALSE;

		if (!strncmp (sptr,"FAST_ON",7))
			fastAccessOff = FALSE;

		if (!strncmp (sptr,"SUB_ON",6))
			subMenuOff = FALSE;

		if (!strncmp (sptr,"TRAILER_ON",10))
			trailerOff = FALSE;

		if (!strncmp (sptr,"HEADING_ON",10))
			headingOff = FALSE;

		if (!strncmp (sptr,"MENU_NAME_ON",12))
			menuNameOff = FALSE;

		if (!strncmp (sptr,"FAST_OFF",8))
			fastAccessOff = TRUE;

		if (!strncmp (sptr,"SHELL_OFF",8))
			shellOff = TRUE;

		if (!strncmp (sptr,"SUB_OFF",7))
			subMenuOff = TRUE;

		if (!strncmp (sptr,"TRAILER_OFF",11))
			trailerOff = TRUE;

		if (!strncmp (sptr,"HEADING_OFF",11))
			headingOff = TRUE;

		if (!strncmp (sptr,"MENU_NAME_OFF",13))
			menuNameOff = TRUE;

		sptr = fgets (data,DATA_SIZE,fmenu);
	}
}

void
LoadQuadrant (
 char *	sptr)
{
	char	*tptr;

	if (maxQuadrants == MAX_EXTRA)
		return;

	quads [maxQuadrants]._col = atoi (sptr + 9);
	tptr = strchr (sptr + 10,',');
	if (tptr != (char *)0)
	{
		quads [maxQuadrants]._row = atoi (tptr + 1);
		maxQuadrants++;
	}
}

void
LoadBoxes (
 char *	sptr)
{
	char	*tptr;

	if (maxBox == MAX_EXTRA)
		return;

	Boxes [maxBox]._col = atoi (sptr + 4);
	tptr = strchr (sptr + 4,',');
	if (tptr != (char *)0)
	{
		Boxes [maxBox]._row = atoi (tptr + 1);
		sptr = tptr + 1;

		tptr = strchr (sptr,',');
		if (tptr != (char *)0)
		{
			Boxes [maxBox]._width = atoi (tptr + 1);
			sptr = tptr + 1;

			tptr = strchr (sptr,',');
			if (tptr != (char *)0)
			{
				Boxes [maxBox]._depth = atoi (tptr + 1);
				maxBox++;
			}
		}
	}
}

void
LoadLines (
 char *	sptr)
{
	char	*tptr;

	if (maxLine == MAX_EXTRA)
		return;

	Lines [maxLine]._col = atoi (sptr + 5);
	tptr = strchr (sptr + 5,',');
	if (tptr != (char *)0)
	{
		Lines [maxLine]._row = atoi (tptr + 1);
		sptr = tptr + 1;

		tptr = strchr (sptr,',');
		if (tptr != (char *)0)
		{
			Lines [maxLine]._length = atoi (tptr + 1);
			maxLine++;
		}
	}
}

void
LoadGraphics (
 char *	sptr)
{
	char	*tptr;

	if (maxGraphics == MAX_EXTRA)
		return;

	Graph [maxGraphics]._col = atoi (sptr + 8);
	tptr = strchr (sptr + 9,',');
	if (tptr != (char *)0)
	{
		Graph [maxGraphics]._row = atoi (tptr + 1);
		sptr = tptr + 1;

		tptr = strchr (sptr,',');
		if (tptr != (char *)0)
		{
			Graph [maxGraphics]._indx = atoi (tptr + 1);

			if (Graph [maxGraphics]._indx > 15)
				Graph [maxGraphics]._indx = 0;

			maxGraphics++;
		}
	}
}

void
LoadComments (
 char *	sptr)
{
	char	*tptr;

	if (maxComment == MAX_EXTRA)
		return;

	tptr = strchr (sptr,'"');
	if (tptr != (char *)0)
	{
		sptr = tptr + 1;
		tptr = strchr (sptr,'"');

		if (tptr != (char *)0)
		{
			*tptr = '\0';
			comment [maxComment]._str = strdup (sptr);

			sptr = tptr + 1;
			tptr = strchr (sptr,',');

			if (tptr != (char *)0)
			{
				comment [maxComment]._col = atoi (tptr + 1);

				sptr = tptr + 1;
				tptr = strchr (sptr,',');

				if (tptr != (char *)0)
				{
					comment [maxComment]._row = atoi (tptr + 1);
					sptr = tptr + 1;
					tptr = strchr (sptr, ',');

					if (tptr != (char *)0)
					{
						comment [maxComment]._rv = 
							atoi (tptr + 1);

						maxComment++;
						return;
					}
				}
			}
			free (comment [maxComment]._str);
		}
	}
}

/*=======================================================
| Set the coordinates for those options	requiring them.	|
=======================================================*/
int
SetCoordnance (void)
{
	register	int	i;
	int		width;
	int		centre;
	int		offset;
	int		length;
	int		indx;

	/*-----------------------------------------------
	| Not Default && Not Window	so calc maxCol		|
	-----------------------------------------------*/
	if (maxCol == -1)
	{
		for (i = 0;i < maxOptions;i++)
		{
			indx = option [i]._col - 1;
			if (indx < 0 || indx >= maxQuadrants)
				indx = 0;

			if (quads [indx]._col + (int) strlen (option [i]._name) + 2 > maxCol)
				maxCol = quads [indx]._col + strlen (option [i]._name) + 2;
		}
	}

	/*-------------------
	| For Every Option	|
	-------------------*/
	for (i = 0;i < maxOptions;i++)
	{
		/*-------------------
		| Menu Heading Line	|
		-------------------*/
		if (i == 0)
		{
			length = strlen (option [0]._name);
			if (window || rfMenu)
			{
				width = maxCol - minCol;
				centre = maxCol - width / 2;
				offset = centre - length / 2;
			}
			else
				offset = (WIDTH - length) / 2;

			option [0]._col = offset;
			option [0]._row = (window || rfMenu) ? minRow + 1 : minRow - 1;

			if (option [0]._col < 0)
				option [0]._col = 1;

			if (option [0]._row < 0)
				option [0]._row = 1;
		}
		else
		{
			/*-----------------------------------
			| Decide Which Quadrant - deflt = 1	|
			-----------------------------------*/
			indx = option [i]._col - 1;
			if (indx < 0 || indx >= maxQuadrants)
				indx = 0;

			offset = option [i]._row;

			option [i]._col = quads [indx]._col;
			option [i]._row = quads [indx]._row + quads [indx]._off;
			quads [indx]._off += offset;

			if (quads [indx]._off + quads [indx]._row > maxRow)
				maxRow = quads [indx]._off + quads [indx]._row;
		}
	}

	if (minRow < MIN_ROW)
		return (EXIT_FAILURE);

	if (minCol < MIN_COL)
		return (EXIT_FAILURE);

	if (maxRow > MAX_ROW)
		return (EXIT_FAILURE);

	if (maxCol > MAX_COL)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
SubtituteFunction (
 char *	str)
{
	int		len;
	int		indx = 0;
	char	*vptr;
	char	*sptr;
	char	tmp [200];

	/*-------------------------------------------------------
	| Go Thru All Variables which can be substituted for	|
	| checking whether they occur in the current string		|
	-------------------------------------------------------*/
	while (strlen (com [indx].lbl))
	{
		sptr = str;
		len = strlen (com [indx].lbl);
		/*-----------------------------------
		| Check Whole string for Variable	|
		-----------------------------------*/
		while (*sptr)
		{
			/*-----------------------------------------------
			| Have a variable which requires substitution	|
			-----------------------------------------------*/

			if (strncmp (sptr,com [indx].lbl,len)==0)
			{
				vptr = com [indx].val;
				/*-----------------------------------
				| Copy Balance of string into temp	|
				-----------------------------------*/
				strcpy (tmp,sptr + len);
				/*---------------------------
				| Copy Subs. onto string	|
				---------------------------*/
				while ((*sptr++ = *vptr++));
				/*-------------------------------------------
				| Concatenate balance of string onto string |
				-------------------------------------------*/
				strcat (str,tmp);
			}
			++sptr;
		}
		indx++;
	}
}

void
RunMenu (
 int	lineNumber)
{
	char	cmd [200];
	char	printerString [3];
	int		pno = 1;
	int		len;
	int		i;
	int		offset = 0;
	int		_background = 0;
	int		_lpno = 0;
	int		_lp_chain = 0;

	void 	(*old_sigvec) ();

	old_sigvec	=	signal (SIGCLD, SIG_DFL);

	alarm (0);

	UserAccountAdd (option [lineNumber]._name);

	strcpy (cmd,option [lineNumber]._program);

	/*--------------------------------
	| Do command line substitutions. |
	--------------------------------*/
	SubtituteFunction (cmd); 
	len = strlen (cmd);

	/*-----------------------------------------
	| Chop up cmd line into separate strings. | 
 	-----------------------------------------*/
	for (i = 0;i < len;i++)
		if (cmd [i] == '~')
			cmd [i] = '\0';

	/*---------------------------------------------------------------------
 	| Check for special strings in cmd line and substitute appro values.  |
	---------------------------------------------------------------------*/
	i = 0;
	do
	{
		*(arg + i) = cmd + offset;
		offset += (strlen (arg [i]) + 1);

		if ((strcmp (arg [i],"LOGNAME")) == 0)
			arg [i] = currentUser;

		if ((strcmp (arg [i],"logout")) == 0)
		{
			if (!psMenu)
				shutdown_prog (ML ("* Menu Exit *"));
		}

		if ((strcmp (arg [i],"BACK")) == 0)
			_background = 1;
		else
			if ((strcmp (arg [i],"LPNO")) == 0)
				_lpno = i;

		if ((strcmp (arg [i],"lp_chain")) == 0)
			_lp_chain = 1;
	}

	while (strlen (arg [i++]));

	*(arg + (i - 1)) = (char *)0; /* Put NULL into last array element */

	if (!_background && !_lpno)
		BusyMessage (!_lp_chain);
		
	set_tty ();
	crsr_on ();
	if (_lpno)
	{
		pno = get_lpno (1);
		sprintf (printerString,"%d",pno);
		*(arg + _lpno) = printerString;
		BusyMessage (1);
	}

	if (_background)
	{
		clear ();
		print_at (0, (80 - strlen (option [lineNumber]._name)) / 2,option [lineNumber]._name);
		DrawMask ();
		print_at (11,19, ML ("Return for Foreground , Space for Overnight.  Any Other Key for Background."));
		if ((len = getkey ()) == '\r')
		{
			_background = 0;
			rv_pr (ML ("Wait Running Program now.."),27,14,1);
		}
		/*--------------------------------------------------------
		| Change BACK to ONIGHT, Place Program Description to be |
 		| Last Argument and put a null on the end.               |
		--------------------------------------------------------*/
		if (len == ' ')
		{
			*(arg) = "ONIGHT";
			*(arg + (i - 1)) = option [lineNumber]._name;
			*(arg + (i)) = (char *) 0;
		}
	}

	rset_tty ();

	signal (SIGALRM,SIG_IGN);

	if (_background)
	{
		switch (fork())
		{
		case -1:
			signal (SIGCLD, old_sigvec);
			return;
	
		case 0:
			/*
		 	*	Child process
		 	*/
			execvp (arg [0],arg);
			signal (SIGCLD, old_sigvec);
			execlp 
			(
				"no_option",
				"no_option",
				option [lineNumber]._program,
				PROG_VERSION, 
				"2",
				option [lineNumber]._name,
				(char *)0
			);
			return;
		}
	}
	else 
	{
		switch (fork())
		{
		case -1:
			signal (SIGCLD, old_sigvec);
			return;
	
		case 0:
			/*
		 	*	Child process
		 	*/
			signal (SIGINT,SIG_IGN);
			status	=	execvp (arg [0],arg);
			signal (SIGCLD, old_sigvec);
			execlp 
			(
				"no_option",
				"no_option",
				option [lineNumber]._program,
				PROG_VERSION, 
				"2",
				option [lineNumber]._name,
				(char *)0
			);
			return;
	
		default:
			/*
		 	*	Parent process
		 	*/
			wait ((int *)0);
		}
	}
	signal (SIGCLD, old_sigvec);
	signal (SIGALRM,MenuTimeout);

	set_tty ();
	crsr_off ();
	GetCheckComm ();
	if (currentUser != getenv ("LOGNAME"))
	{
		currentUser = getenv ("LOGNAME");
		GetUserSecureFile ();
		LoadMenu (oldMenu);
	}
}

/*===============================
| Action on trapping interupt	|
===============================*/
void
MenuKill (
 int	x)
{
	int	pid;

	signal (SIGINT,MenuKill);

	if ((pid = wait (&status)) == -1)
		putchar (BELL);
	else
	{
		clear ();
		gr_off ();
		print_at (12,2, ML ("DEL key pressed, process %d killed, returning to menu"),pid);
	}
}

void
MenuTimeout (
 int	x)
{
	signal (SIGALRM,MenuTimeout);
	alarm (60);

	timer_ticked = TRUE;

	if (timer >= timeout && timeout != 0)
		shutdown_prog (ML ("Timeout .... Menu system logging out"));

	/*-------------------------------------------------------------------
	| Dont print time if timer > 15 (min), timer set to 0 by key press. |
	-------------------------------------------------------------------*/
	if (timer++ > 15)
		return;

	if (!window && !rfMenu && timer > 5)
	{
		if (!headingOff)
			PrintHeading ();
	}
	c = -1;
}

/*======================
| Process Help options |
======================*/
int
MenuHelp (
 int	std,
 int	lineNumber)
{
	char	helpFile [20];

	if (std)
		sprintf (helpFile,"%s","menu.c");
	else
		sprintf (helpFile,"%s%d.c",option [0]._program,lineNumber);

	*(arg) = "prog_help";
	*(arg + (1)) = helpFile;
	*(arg + (2)) = (char *)0;

	MenuShellOut (OTHER);
	return (EXIT_FAILURE);
}

/*===============================
| Shell Out to another program	|
===============================*/
void
MenuShellOut (
 int	sh_type)
{
	int	indx = 0;
	int	_sh_type = sh_type;
	char	fn_out [35];
	char	*sptr;
	char	*tptr = (char *) 0;
	char	*xptr;

	BusyMessage (0);

	alarm (0);

	if (sh_type >= FN17 && sh_type < FN32)
	{
		_sh_type = FN17;
	}

	switch (_sh_type)
	{
	case	RUN_HELP:
		*(arg) = "help_fn";
		*(arg + (1)) = (char *)0;
		break;
	
	case	EXTERN:
		*(arg) = "run_extern";
		*(arg + (1)) = "menu.c";
		*(arg + (2)) = (char *)0;
		break;
	
	case	SHELL:
		clear ();
		crsr_on ();
		rset_tty ();
		fflush (stdout);

		if (CheckSecurity ("a", security))
			*(arg) = getenv ("SHELL");
		else
			*(arg) = "sorry";

		*(arg + (1)) = (char *)0;
		break;
		
	case	FN17:
		if (currentUser != (char *)0)
		{
			xptr = strdup (currentUser);
			upshift (xptr);
			sprintf (fn_out,"%s.FN%d",xptr,sh_type - (FN1 - 1));
			tptr = chk_env (fn_out);
			free (xptr);
		}
		if (tptr == (char *) 0)
		{
			sprintf (fn_out,"FN%d",sh_type - (FN1 - 1));
			tptr = chk_env (fn_out);
		}

		if (tptr == (char *) 0)
			return;

		if (*(tptr + strlen (tptr) - 1) == '~')
			*(tptr + strlen (tptr) - 1) = '\0';

		sptr = strchr (tptr, '~');

		while (sptr != (char *)0)
		{
			*sptr = '\0';
			*(arg+ (indx++)) = tptr;

			tptr = sptr + 1;
			sptr = strchr (tptr, '~');
		}

		*(arg+ (indx++)) = tptr;

		*(arg+ (indx)) = (char *)0;
		break;

	default:
		break;
	}

	signal (SIGALRM,SIG_IGN);
	/*---------------
	| Child Process	|
	---------------*/
	if (fork () == 0)
	{
		signal (SIGINT,SIG_DFL);
		execvp (arg [0],arg);
	/*	execlp ("no_option","no_option", ML ("Program Shell Out"),
				    PROG_VERSION, "2",
				    "Program Shell Out",
				    (char *)0);*/
	}
	else
	{
		signal (SIGINT,SIG_IGN);
		wait (&status);
	}
	wait (&status);	
	signal (SIGINT,MenuKill);
	signal (SIGALRM,MenuTimeout);
	alarm (60);
	
	GetCheckComm ();
	set_tty ();
	crsr_off ();
	if (currentUser != getenv ("LOGNAME"))
	{
		currentUser = getenv ("LOGNAME");
		GetUserSecureFile ();
		LoadMenu (oldMenu);
	}
}

void
BusyMessage (
 int	cl_screen)
{
	if (cl_screen)
		clear ();

	print_at (0,0, ML ("Busy ... "));
	fflush (stdout);
}

/*===============================================
| Change Case of string from upper to lower	|
===============================================*/
void
DownShiftFuction (
 char *	str)
{
	char	*sptr = str;

	while (*sptr)
	{
		*sptr = tolower (*sptr);
		sptr++;
	}
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff access permitted	|
=======================================*/
int
CheckSecurity (
 char *	_secure,		/* security on menu		*/
 char *	_security)		/* security on user		*/
{
	char	*sptr;
	char	*tptr;
	char	*uptr;
	char	*vptr;
	char	tmp_mnu_sec [9];
	char	tmp_usr_sec [9];
	char	usr_char;
	char	mnu_char;

	/*---------------------------------------
	| Super User Access on users security	|
	---------------------------------------*/
	if ((sptr = strchr (_security,'*')))
		return (EXIT_FAILURE);
	
	/*-------------------------------
	| Access to all on menu option	|
	-------------------------------*/
	if ((sptr = strchr (_secure,'*')))
		return (EXIT_FAILURE);
	
	/*-------------------------------------------
	| Check Security for each security group	|
	| that user belongs to.						|
	-------------------------------------------*/	
	sptr = strdup (_security);
	while (*sptr)
	{
		/*----------------
		| Find separator |
		----------------*/
		tptr = sptr;
		while (*tptr && *tptr != '|')
			tptr++;

		usr_char = *tptr;

		*tptr = '\0';
		strcpy (tmp_usr_sec, sptr);

		if (usr_char)
			sptr = tptr + 1;
		else
			*sptr = '\0';

		uptr = strdup (_secure);
		while (*uptr)
		{
			/*----------------
			| Find separator |
			----------------*/
			vptr = uptr;
			while (*vptr && *vptr != '|')
				vptr++;

			mnu_char = *vptr;

			*vptr = '\0';
			strcpy (tmp_mnu_sec, uptr);

			if (mnu_char)
				uptr = vptr + 1;
			else
				*uptr = '\0';

			if (!strcmp (tmp_usr_sec, tmp_mnu_sec))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

int
Heading (void)
{
	return (EXIT_SUCCESS);
}

void
PrintHeading (void)
{
	char	dateString [100];
	time_t	tloc	=	time (NULL);

	/*-----------------------------------
	| get pointer to time & structure . |
	-----------------------------------*/
	ts = localtime (&tloc);
	
	sprintf 
	(
		dateString, 
		"%-s %-s %d%-s %d", day [ts->tm_wday], 
		mth [ts->tm_mon],
		ts->tm_mday,
		abr [ts->tm_mday-1],
		ts->tm_year + 1900
	);
			
	sprintf 
	(
		err_str, 
		" %-30.30s %10.10s : %d (%c%s)   %8.8s %02d:%02d %2.2s ", 
		dateString,
		ML (" Terminal "),
		terminalNumber,		
		to_upper (currentUser [0]),
		currentUser + 1,
		"    Time",
		((ts->tm_hour) > 12 ? (ts->tm_hour -12) : (ts->tm_hour)),
		ts->tm_min,
		((ts->tm_hour) >= 12 ? "pm" : "am")
	);
	so_pr (err_str,2,1,1);
}
