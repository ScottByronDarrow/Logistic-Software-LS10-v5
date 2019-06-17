/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( UNI.c          )                                 |
|  Program Desc  : ( Change Owner / Group / Permissions on files. )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (22/02/88)      | Author       : Roger Gibbison    |
|---------------------------------------------------------------------|
|  Date Modified : (22/02/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (15/04/93)      | Modified  by : Trevor van Bremen |
|                                                                     |
|  Comments      :                                                    |
|  (15/04/93)    : PSL 5784 (Joke) chmod performed BEFORE chown.      |
|                                                                     |
=====================================================================*/
char	*PNAME = "$RCSfile: UNI.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/UNI/UNI.c,v 5.1 2001/08/09 09:26:42 scott Exp $";

#include	<std_decs.h>
#include 	<stdio.h>
#include	<pwd.h>
#include	<grp.h>

void	Set_file(char *_filename);

char	*_lsl = "lsl";

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	register	int	i;

	for (i = 1;i < argc;i++)
		Set_file(argv[i]);

	return (EXIT_SUCCESS);
}
void
Set_file(char *_filename)
{
	static	int	uid;
	static	int	gid;
	static	int	got_ids;
	static	int	perm;
	int	_perm;
	char	*sptr;
	char	*_usr;
	char	*_grp;
	char	*getenv(const char *);
	struct	passwd	*getpwnam(const char *);
	struct	passwd	*usr;
	struct	group	*getgrnam(const char *);
	struct	group	*grp;
	char	_err_str[81];
	/*---------------------------------------
	| already got all the info		|
	---------------------------------------*/
	if (!got_ids)
	{
		/*---------------------------------------
		| set defaults				|
		---------------------------------------*/
		_usr = _lsl;
		_grp = _lsl;
		_perm = 512;
		/*---------------------------------------
		| read FILE_PERM environment variable	|
		---------------------------------------*/
		sptr = getenv("FILE_PERM");
		if (sptr != (char *)0)
		{
			/*---------------------------------------
			| get user name from variable		|
			---------------------------------------*/
			_usr = sptr;
			sptr = strchr(_usr,' ');
			if (sptr != (char *)0)
			{
				*sptr = '\0';
				_grp = sptr + 1;
				/*---------------------------------------
				| get group name from variable		|
				---------------------------------------*/
				sptr = strchr(_grp,' ');
				if (sptr != (char *)0)
				{
					*sptr = '\0';
					_perm = atoi(sptr + 1);
				}
			}
		}
		/*---------------------------------------
		| find user id for user			|
		---------------------------------------*/
		usr = getpwnam(_usr);
		if (usr == (struct passwd *)0)
		{
			sprintf(_err_str,"No User Name [%s] Exists",_usr);
			sys_err(_err_str,-1,PNAME);
		}
		uid = usr->pw_uid;
		/*---------------------------------------
		| find group id for group		|
		---------------------------------------*/
		grp = getgrnam(_grp);
		if (grp == (struct group *)0)
		{
			sprintf(_err_str,"No Group Name [%s] Exists",_grp);
			sys_err(_err_str,-1,PNAME);
		}
		gid = grp->gr_gid;
		/*---------------------------------------
		| convert permissions to octal		|
		---------------------------------------*/
		sprintf(_err_str,"%05d",_perm);
		for (perm = 0,sptr = _err_str;*sptr;sptr++)
		{
			perm *= 8;
			perm += (*sptr - '0');
		}
		got_ids = 1;
	}
	chmod(_filename,perm);
	chown(_filename,uid,gid);
}
