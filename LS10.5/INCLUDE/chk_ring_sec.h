/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  :  (chk_ring.h    )                                  |
|  Program Desc  :  (Check Ring Menu Security.                   )    |
| $Id: chk_ring_sec.h,v 5.0 2001/06/19 06:51:27 cha Exp $
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 19/04/96         |
|---------------------------------------------------------------------|
| $Log: chk_ring_sec.h,v $
| Revision 5.0  2001/06/19 06:51:27  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:36  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.5  2000/07/14 00:04:25  scott
| Updated to ignore '*' records as this makes all users have complete access even if options are dis-allowed.
|
|                                                                     |
=====================================================================*/
#define		DataRecordSize	200

	char	*UserSecurity;

	static	int CheckSecurity  	(char *, char *);
	void 	_chk_ring_sec  		(menu_type *, char *);
	void	down_shift 			(char *str);
	void	GetUserAccess 		(void);

	/*============================================+
	 | System options menu  (used sk_alldisp etc. |
	 +============================================*/
#define	OPTS_NO_FIELDS	6

	struct dbview	opts_list [OPTS_NO_FIELDS] =
	{
		{"opts_access_code"},
		{"opts_prog_name"},
		{"opts_option_no"},
		{"opts_key"},
		{"opts_allowed"},
		{"opts_key_desc"}
	};

	struct tag_optsRecord
	{
		char	access_code [9];
		char	prog_name [15];
		int		option_no;
		char	key [31];
		char	allowed [2];
		char	key_desc [61];
	}	opts_rec;

/*====================================
| Main ring menu processing Routine. |
====================================*/
void
_chk_ring_sec  (
	menu_type	*curr_menu, 
	char		*prog_name)
{
	int		i;
	int		offset = 0;
	int		prompt_len;

	GetUserAccess ();

	for  (i = 0, offset = prompt_len; strlen (PRMT (i)); i++)
	{
		if  (PRMT (i)[0] == '<')
			FLAG (i) = DISABLED;
	}
	
	open_rec ("opts", opts_list, OPTS_NO_FIELDS, "opts_id_no");
	
	sprintf (opts_rec.prog_name,   "%-14.14s", prog_name);
	sprintf (opts_rec.access_code, "%-8.8s",   "        ");
	sprintf (opts_rec.key, "%30.30s", " ");
	opts_rec.option_no = 0;

	cc = find_rec ("opts", &opts_rec, GTEQ, "r");
	while  (!cc && !strncmp (opts_rec.prog_name,prog_name, strlen (prog_name)))
	{
		if  (CheckSecurity  (clip (opts_rec.access_code), UserSecurity))
		{
			for  (i = 0, offset = prompt_len; strlen (PRMT (i)); i++)
			{
				if  (!strncmp (opts_rec.key,PRMT (i),strlen (PRMT (i))))
				{
					 if  (opts_rec.allowed[0] == 'Y')
						FLAG (i) = VALID;
				}
			}
		}
		cc = find_rec ("opts", &opts_rec, NEXT, "r");
	}
	abc_fclose ("opts");
	return;
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff access permitted		|
=======================================*/
static	int
CheckSecurity
 (
	char	*_secure,		/* security on ring menu	*/
	char	*_security 		/* security on user			*/
)
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
	if  ((sptr = strchr (_security,'*')))
		return (1);

	/*----------------------------------
	| Global ring menu, Access to all. |
	----------------------------------*/
	/*
		Removed this as a little bit silly
	if  ((sptr = strchr (_secure,'*')))
		return (1);
	*/
	
	/*-----------------------------------------------
	| Check Security for each security group		|
	| that user belongs to.							|
	-----------------------------------------------*/	
	sptr = p_strsave (_security);
	while  (*sptr)
	{
		/*----------------
		| Find separator |
		----------------*/
		tptr = sptr;
		while  (*tptr && *tptr != '|')
			tptr++;

		usr_char = *tptr;

		*tptr = '\0';
		strcpy (tmp_usr_sec, sptr);

		if  (usr_char)
			sptr = tptr + 1;
		else
			*sptr = '\0';

		uptr = p_strsave (_secure);
		while  (*uptr)
		{
			/*----------------
			| Find separator |
			----------------*/
			vptr = uptr;
			while  (*vptr && *vptr != '|')
				vptr++;

			mnu_char = *vptr;

			*vptr = '\0';
			strcpy (tmp_mnu_sec, uptr);

			if  (mnu_char)
				uptr = vptr + 1;
			else
				*uptr = '\0';

			if  (!strcmp (tmp_usr_sec, tmp_mnu_sec))
				return (1);
		}
	}
	return (0);
}

void
GetUserAccess (void)
{

	char	*sptr = getenv ("PROG_PATH");
	char	*tptr;
	char	filename[101];
	char	*curr_user = getenv  ("LOGNAME");
	char	DataUserSecure [DataRecordSize];
	FILE	*UserSecureFile;

	UserSecurity =  (char *)0;

	sprintf (filename,"%s/BIN/MENUSYS/User_secure", (sptr !=  (char *)0) ? sptr : "/usr/DB");

	if  ((UserSecureFile = fopen (filename,"r")) == 0)
	{
		sprintf (err_str,"Error in %s during  (FOPEN)",filename);
		sys_err (err_str,errno,PNAME);
	}

	sptr = fgets (DataUserSecure,DataRecordSize,UserSecureFile);

	while  (sptr !=  (char *)0)
	{
		tptr = sptr;
		while  (*tptr != ' ' && *tptr != '\t')
			tptr++;

		*tptr = '\0';
		/*-----------------------------------------------
		| Find the Appropriate Entry for the User	|
		-----------------------------------------------*/
		if  (!strcmp (DataUserSecure,curr_user))
		{
			sptr = tptr + 1;
			/*---------------------------
			| Found Start of security	|
			---------------------------*/
			if  ((tptr = strchr  (sptr, '<')))
			{
				sptr = tptr + 1;
				if  ((tptr = strchr  (sptr, '>')))
				{
					*tptr = '\0';
					UserSecurity = p_strsave (sptr);
					break;
				}
			}
		}
		sptr = fgets (DataUserSecure,DataRecordSize,UserSecureFile);
	}
	fclose (UserSecureFile);

	if  (UserSecurity ==  (char *)0)
		return;

	down_shift (UserSecurity);
	return;
}

/*===============================================
| Change Case of string from upper to lower	|
===============================================*/
void
down_shift (char *str)
{
	char	*sptr = str;

	while  (*sptr)
	{
		*sptr = tolower (*sptr);
		sptr++;
	}
}
