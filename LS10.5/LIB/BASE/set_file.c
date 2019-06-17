/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( set_file.c     )                                 |
|  Program Desc  : (                                                ) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#include	<std_decs.h>
#include	<pwd.h>
#include	<grp.h>

char	*_lsl = "lsl";

void
set_file(char *_filename)
{
	static	uid_t	uid;
	static	gid_t	gid;
	static	int	got_ids;
	static	int	perm;
	int	_perm;
	char	*sptr;
	char	*_usr;
	char	*_grp;
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
		grp = getgrnam(clip(_grp));
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
	chown(_filename,uid,gid);
	chmod(_filename,perm);
}
