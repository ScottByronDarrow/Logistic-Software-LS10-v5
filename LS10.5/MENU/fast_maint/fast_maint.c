/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( fast_maint.c    )                                |
|  Program Desc  : ( Maintain Fast Access Files for Menu System   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  MENUSYS/User_secure, MENUSYS/.mdf                 |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  MENUSYS/.fa                                       |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (31/08/88)      | Author       : Roger Gibbison    |
|---------------------------------------------------------------------|
|  Date Modified : (31/08/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (27/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (31/05/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (09/09/97)      | Modified  by : Rowena S Maandig  |
|  Date Modified : (09/09/97)      | Modified  by : Rowena S Maandig  |
|  Date Modified : (30/08/1999)    | Modified  by : Alvin Misalucha   |
|                                                                     |
|  Comments      : (17/05/91) Security codes may now be up to 8 chars |
|                : long. Changed from using _chk_security to use      |
|                : Chk_security which checks for security codes using |
|                : the new format ie <debtor|first|last>.             |
|  (31/05/93)    : PSL 5784. Not fclosing when it should...           |
|  (09/09/97)    : Incorporate multilingual conversion.               |
|  (30/08/1999)  : Converted to ANSI format.                          |
|                                                                     |
| $Log: fast_maint.c,v $
| Revision 5.2  2001/08/09 05:13:29  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:24  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:39  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:01  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/09/07 06:22:39  scott
| Updated as dsp_screen called and called and called.........
|
| Revision 2.1  2000/08/11 08:27:36  scott
| Updated dsp_process as record being added to mldb.
|
| Revision 2.0  2000/07/15 09:00:13  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  1999/12/06 01:47:13  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/25 10:23:59  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.12  1999/11/16 09:41:55  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.11  1999/10/04 04:12:19  jonc
| Updated to use getrlimit() for system limits on open files.
|
| Revision 1.10  1999/09/29 02:03:14  scott
| Updated from Ansi testing
|
| Revision 1.9  1999/09/17 07:26:58  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.8  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/15 02:35:59  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fast_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/fast_maint/fast_maint.c,v 5.2 2001/08/09 05:13:29 scott Exp $";

#include	<sys/time.h>
#include	<sys/resource.h>

#define		NO_SCRGEN
#define		MOD	1
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

char	filename[200];
char	buffer[300];

FILE	*fuser;

/*=======================
| Function prototypes   |
=======================*/
void	process			(char * user);
void	fast_access		(char * user_name, char * security);
void	proc_menu		(int fd, char * mname, char * user_name,
						 char * security, int level, int maxfile);
void	dsp_mess		(char * message, char * mname);
int		Chk_security	(char * _secure, char * _security);

#include	<fast.h>

int
main (
 int	argc,
 char * argv [])
{
	int	i;

	dsp_screen ("Maintain Menu Fast Access", " ","LS/10");

	/*---------------------------------------
	| Process for all parameters - if given	|
	---------------------------------------*/
	if (argc != 1)
		for (i = 1;i < argc;i++)
			process(argv[i]);
	else
		process((char *)0);

	clear();
	crsr_on();
	return (EXIT_SUCCESS);
}

void
process (
 char *	user)
{
	char	user_line[100];
	char	user_name[10];
	char	user_test[10];
	char	*security;
	char	*sptr = getenv("PROG_PATH");
	char	*tptr;

	sprintf(user_test,"%-.8s",(user != (char *)0) ? user : " ");

	sprintf(filename,"%s/BIN/MENUSYS/User_secure",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

	/*-----------------------------------------------
	| Layout of User_secure:			|
	| rog				<abc>		|
	| ^           ^                ^                |
	| + user name + tabs or spaces + groups         |
	-----------------------------------------------*/
	if ((fuser = fopen(filename,"r")) == 0)
	{
		sprintf(buffer,"Error in %s during (FOPEN)",filename);
		sys_err(buffer,errno,PNAME);
	}

	sptr = fgets(user_line,100,fuser);

	while (sptr != (char *)0)
	{
		/*---------------------------------------
		| Get Security groups for username	|
		---------------------------------------*/
		security = (char *)0;
		tptr = strchr (sptr,'<');
		if (tptr != (char *)0)
		{
			*tptr = '\0';
			sptr = tptr + 1;

			tptr = strchr (sptr,'>');
			if (tptr != (char *)0)
			{
				*tptr = '\0';
				security = sptr;
			}
		}

		sprintf(user_test,"%-.8s",(user != (char *)0) ? user : " ");
		sprintf(user_name,"%-.8s",clip(user_line));

		/*-------------------------------
		| Process Only Specified Users	|
		-------------------------------*/
		if (user != (char *)0 && strcmp(user_test,user_name))
		{
			sptr = fgets(user_line,100,fuser);
			continue;
		}

		/*-------------------------------
		| Process Fast Access for user	|
		-------------------------------*/
		fast_access(user_line,security);

		sptr = fgets(user_line,100,fuser);
	}
}

/*===============================
| Process Fast Access for User	|
===============================*/
void
fast_access (
 char *	user_name,
 char *	security)
{
	int	fd;
	struct rlimit	limits;

	/*---------------
	| No Security	|
	---------------*/
	if (!security)
		return;

	security = clip(security);

	/*---------------
	| No Security	|
	---------------*/
	if (!strlen(security))
		return;

	/*
	 *	Determine file-limits
	 */
	if (getrlimit (RLIMIT_NOFILE, &limits) != 0)
		return;				/* Resource query failed? */


	init_mtab ();

	/*---------------------------------------
	| Open Fast Access file for writing	|
	---------------------------------------*/
	fd = open_fast(user_name,"w");

	dsp_process ("Menu : ",user_name);

	/*---------------------------------------
	| Process Fast Access From Primary Menu	|
	---------------------------------------*/
	proc_menu (fd,
		"MENUSYS/main.mdf",
		user_name,
		security,
		5,
		limits.rlim_cur);

	close_fast(fd);
}

/*===============================================
| Calculate the Number of Options Available to	|
| The User on the menu given.			|
===============================================*/
void
proc_menu (
 int fd,
 char *	mname,
 char *	user_name,
 char *	security,
 int level,
 int max_nfile)
{
	int	no_opts = 0;
	int	valid;
	char	secure[2049];
	char	menu_name[30];
	char	fast_key[7];
	char	*sptr = getenv("PROG_PATH");
	char	*tptr;
	FILE	*fmenu;

	if (level > max_nfile)
	{
		sprintf(err_str,"%d",FOPEN_MAX - 5);
		dsp_mess(mlMenuMess221,err_str);
		return;
	}

	sprintf(filename,"%s/BIN/%s",(sptr != (char *)0) ? sptr : "/usr/LS10.5",mname);

	/*-----------------------
	| Open Menu Data File	|
	-----------------------*/
	if ((fmenu = fopen(filename,"r")) == 0)
	{
		sprintf(buffer,"Error in %s during (FOPEN)",filename);
		sys_err(buffer,errno,PNAME);
	}
	new_menu (mname);

	/*-----------------------
	| Menu Heading Line	|
	-----------------------*/
	sptr = fgets(buffer,400,fmenu);
	if (sptr == (char *)0)
	{
		dsp_mess(mlMenuMess222,mname);
		fclose(fmenu);
		return;
	}

	/*---------------
	| Get Fast Key	|
	---------------*/
	tptr = strchr (sptr,'(');
	if (tptr)
	{
		sptr = tptr + 1;
		if ((tptr = strchr (sptr,')')))
		{
			*tptr = '\0';
			sprintf(fast_key,"%-*.*s", FASTLEN, FASTLEN, sptr);
		}
		else
			sprintf(fast_key,"%-*.*s", FASTLEN, FASTLEN, " ");
	}
	else
		sprintf(fast_key,"%-*.*s", FASTLEN, FASTLEN, " ");

	/*-------------------------------------------------------
	| Check if Fast Access Key Has Already Been Used	|
	-------------------------------------------------------*/
	cc = chk_fast(user_name,fast_key);
	if (cc)
	{
		dsp_mess(mlMenuMess223,mname);
		fclose(fmenu);
		return;
	}
	/*-----------------------
	| Get the Help File	|
	-----------------------*/
	sptr = fgets(buffer,400,fmenu);
	if (sptr == (char *)0)
	{
		/*Corrupt Menu Data File %s */
		dsp_mess(mlMenuMess222,mname);
		fclose(fmenu);
		return;
	}

	/*-----------------------
	| First Option Line	|
	-----------------------*/
	sptr = fgets(buffer,400,fmenu);

	/*---------------------------------------------------------------
	| Process Menu Data File Until End of File or Special Formats	|
	---------------------------------------------------------------*/
	while (sptr != (char *)0 && strncmp(sptr,"((",2))
	{
		/*-------------------------------
		| Check Security on Option	|
		-------------------------------*/
		secure[0] = '\0';
		tptr = strchr (sptr,'<');
		if (tptr != (char *)0)
		{
			sptr = tptr + 1;
			tptr = strchr (sptr,'>');
			if (tptr != (char *)0)
			{
				*tptr = '\0';
				sprintf(secure,"%-.2048s",sptr);
			}
		}

		/*---------------------------------------
		| Check if user has access to option	|
		---------------------------------------*/
		valid = Chk_security(security,secure);
		if (valid)
			no_opts++;

		/*-----------------------
		| Read Program Line	|
		-----------------------*/
		sptr = fgets(buffer,400,fmenu);
		if (sptr == (char *)0)
		{
			dsp_mess(mlMenuMess222,mname);
			fclose(fmenu);
			return;
		}

		/*---------------------------------------
		| If Option is Valid &&			|
		| Not the first option on the menu &&	|
		| Option is another menu.		|
		---------------------------------------*/
		if (valid && (no_opts > 1 || !strcmp(mname,"MENUSYS/main.mdf")) && !strncmp(sptr,"menu ",5))
		{
			*(sptr + strlen(sptr) - 1) = '\0';
			sptr += 5;
			while (sptr && *sptr == ' ')
				sptr++;

			sprintf(menu_name,"%-.22s",sptr);

			/*-----------------------
			| Recurse to Sub Menu	|
			-----------------------*/
			proc_menu (fd,
				menu_name,
				user_name,
				security,
				level + 1,
				max_nfile);
		}

		/*-----------------------
		| Read Next Option Line	|
		-----------------------*/
		sptr = fgets(buffer,400,fmenu);
	}

	/*---------------------------------------------------------------
	| If User has Options from Menu add entry to Fast Access File	|
	---------------------------------------------------------------*/
	if (no_opts)
		add_fast(fd,fast_key,mname + 8,no_opts);

	fclose(fmenu);
}

/*=======================
| Display Error message	|
=======================*/
void
dsp_mess (
 char *	message,
 char *	mname)
{
	register	int	i;

	box(2,15,74,2);
	crsr_on();
	print_at(16,5,message,mname);
	PauseForKey (17, 5, mlStdMess042, 0);

	for (i = 15;i < 19;i++)
	{
		print_at(i,2,"%74.74s"," ");
	}
	crsr_off();
	fflush(stdout);
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff access permitted	|
=======================================*/
int
Chk_security (
 char *	_secure,		/* security on menu		*/
 char * _security)		/* security on user		*/
{
	char	*sptr;
	char	*tptr;
	char	*uptr;
	char	*vptr;
	char	tmp_mnu_sec[9];
	char	tmp_usr_sec[9];
	char	usr_char;
	char	mnu_char;

	/*---------------------------------------
	| Super User Access on users security	|
	---------------------------------------*/
	if ((sptr = strchr(_security,'*')))
		return(1);
	
	/*-------------------------------
	| Access to all on menu option	|
	-------------------------------*/
	if ((sptr = strchr(_secure,'*')))
		return(1);
	
	/*-----------------------------------------------
	| Check Security for each security group	|
	| that user belongs to.				|
	-----------------------------------------------*/	
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
		strcpy(tmp_usr_sec, sptr);

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
			strcpy(tmp_mnu_sec, uptr);

			if (mnu_char)
				uptr = vptr + 1;
			else
				*uptr = '\0';

			if (!strcmp(tmp_usr_sec, tmp_mnu_sec))
				return(1);
		}
	}
	return(0);
}
