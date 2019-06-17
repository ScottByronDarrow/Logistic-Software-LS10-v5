/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: fast.c,v 5.1 2001/08/06 22:40:54 scott Exp $
|=====================================================================|
|  Program Name  : (fast.c        )                                   |
|  Program Desc  : (Fast Access Maintenance Routines.           )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  MENUSYS/?.fa                                      |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Updates Files :  MENUSYS/?.fa                                      |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Date Written  : 30/08/88        |  Author     : Roger Gibbison     |
|---------------------------------------------------------------------|
|  Date Modified : (30/08/88)      | Modified by : Roger Gibbison.    |
|  Date Modified : (21.01.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      :                                                    |
|       21.01.94 : Removed references to _cc                          |
|                :                                                    |
| $Log: fast.c,v $
| Revision 5.1  2001/08/06 22:40:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 06:59:16  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:17:13  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.6  2000/07/05 01:39:07  scott
| Updated to re-define curr_ment to currentMenu due to conflict with tab routines
|
=====================================================================*/
#define	FAST

#include	<std_decs.h>
#include	<fast.h>

char	currentMenu [60];
/*===================
| Fast Access User	|
===================*/
FASTSTRUC fast_rec;

/*=======================================
| Check if fast access is permitted		|
| Return:	menu file to load			|
| if opt_no is set to -1 then fast_key	|
| wasn't found.							|
=======================================*/
int
get_fast (char *fast_key)
{
	int	fd;
	int	fast_opt;
	char	fast_name [FASTLEN + 3];
	char	fast_base [FASTLEN + 1];
	char	*sptr = getenv ("LOGNAME");

	sprintf (fast_name,"%-*.*s", FASTLEN + 2, FASTLEN + 2, fast_key);

	fd = open_fast (sptr,"r");
	if (fd == -1)
		return (fd);

	/*-------------------------------
	| Get Fast Base - the menu	|
	-------------------------------*/
	sptr = fast_name;
	while (*sptr && *sptr != ' ' && !_isdigit (*sptr))
		sptr++;

	if (*sptr == ' ')
		*sptr++ = '\0';
	/*---------------------------
	| Get The Option Requested	|
	---------------------------*/
	fast_opt = atoi (sptr);

	if (_isdigit (*sptr))
		*sptr = '\0';

	sprintf (fast_base,"%-*.*s", FASTLEN, FASTLEN, fast_name);

	while (!RF_READ (fd, (char *) &fast_rec))
	{
		/*---------------------------------------
		| Found Match on Fast Code for User	|
		---------------------------------------*/
		if (!strcmp (fast_base,fast_rec._f_code))
		{
			close_fast (fd);
			/*-----------------------------------------------
			| Don't have access to that many options	|
			-----------------------------------------------*/
			if (fast_opt > fast_rec._n_opts)
				return (-1);
			else
			{
				sprintf (currentMenu,"MENUSYS/%s", clip (fast_rec._m_file));
				return ((fast_opt) ? fast_opt : 1);
			}
		}
	}
	close_fast (fd);
	return (-1);
}

/*===================================================
| Check if record exists for user_name / fast_key	|
| already - return 1 if exists						|
===================================================*/
int
chk_fast (char *user, char *fast_key)
{
	int	fd;
	char	fast_base [FASTLEN + 1];

	sprintf (fast_base,"%-*.*s", FASTLEN, FASTLEN, fast_key);
	fd = open_fast (user,"r");
	if (fd == -1)
		return (EXIT_SUCCESS);

	while (!RF_READ (fd, (char *) &fast_rec))
	{
		/*---------------------------------------
		| Found Match on Fast Code for User	|
		---------------------------------------*/
		if (!strcmp (fast_base,fast_rec._f_code))
		{
			close_fast (fd);
			return (EXIT_FAILURE);
		}
	}

	close_fast (fd);
	return (EXIT_SUCCESS);
}

int
open_fast (char *user_name, char *mode)
{
	int		errc, fd;
	char	*sptr = getenv ("PROG_PATH");
	char	*user = getenv ("LOGNAME");
	char	filename [100];

	if (user_name)
		user = user_name;

	sprintf (filename,"%s/BIN/MENUSYS/%s.fa", (sptr) ? sptr :
							 "/usr/LS10.5",
							 clip (user));
	/*---------------------------------------
	| If variable file doesn't exist	|
	---------------------------------------*/
	if (access (filename,00) < 0)
	{
		if ((errc = RF_OPEN (filename, sizeof (FASTSTRUC), "w", &fd)))
			sys_err ("Error in FAST_ACCESS during (WKCREAT)", errc,PNAME);

		if ((errc = RF_CLOSE (fd)))
			sys_err ("Error in FAST_ACCESS during (WKCLOSE)", errc, PNAME);
	}

	if ((errc = RF_OPEN (filename, sizeof (FASTSTRUC), mode, &fd)))
		sys_err ("Error in FAST_ACCESS during (WKOPEN)", errc, PNAME);

	return (fd);
}

int
add_fast (int fd, char *fast_code, char *menu_file, int fast_opts)
{
	sprintf (fast_rec._f_code,"%-*.*s", FASTLEN, FASTLEN, fast_code);
	sprintf (fast_rec._m_file,"%-14.14s",menu_file);

	fast_rec._n_opts = fast_opts;

	return (RF_ADD (fd, (char *)&fast_rec));
}

void
close_fast (int fd)
{
	int	errc = RF_CLOSE (fd);

	if (errc)
		sys_err ("Error in FAST_ACCESS during (WKCLOSE)", errc, PNAME);
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff accesss permitted	|
=======================================*/
int
_chk_security (char *_secure, char *_security)
    	         		/* security on menu		*/
    	           		/* security on user		*/
{
	char	*sptr;

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
	sptr = _security;
	while (*sptr)
	{
		if (strchr (_secure, *sptr))
			return (EXIT_FAILURE);
		
		sptr++;
	}
	return (EXIT_SUCCESS);
}
