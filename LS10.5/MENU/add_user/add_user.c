/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( add_user.c     )                                 |
|  Program Desc  : ( Add / Update / Delete Users from System.     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (17/02/88)      | Author       : Roger Gibbison    |
|---------------------------------------------------------------------|
|  Date Modified : (17/02/88)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (27/05/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (02/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (23/05/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (21/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (26/01/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (29/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (24.12.94)      | Modified  by : Jonathan Chen     |
|  Date Modified : (12/09/97)      | Modified  by : Leah Manibog.     |
|  Date Modified : (25/08/1999)    | Modified  by : Alvin Misalucha.  |
|                                                                     |
|  Comments                                                           |
|  (27/05/91)    : Security codes may now be up to 8 chars long.      |
|                : Converted from tabular screen to tabdisp for       |
|                : easier entry and display of multiple security codes|
|                : per user.                                          |
|  (02/09/91)    : SC 5618 DPL. Changes to group/user ids.            |
|  (23/05/92)    : Updated for compatibility with RiscOs 5.0          |
|                : Also, generally tidy up.                           |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|  (26/01/93)    : Allow for 'numeric' within user name. (But not the |
|                : 1st character). 600MAC 8377.                       |
|  (29/03/94)    : INF 10647. Changes for ver9 compile on SCO.        |
|  (23.12.94)    : DFT 11221 Extended reading in of security codes    |
|  (12/09/97)    : Updated for Multilingual Conversion.        		  |
|  (25/08/1999)  : Ported to ANSI convention.                         |
|                                                                     |
| $Log: add_user.c,v $
| Revision 5.3  2001/11/08 01:10:28  scott
| Updated to fix small warning message on Linux.
|
| Revision 5.2  2001/08/09 05:13:13  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/06/19 06:27:35  cha
| Updated to make compatible to HP.
|
| Revision 4.0  2001/03/09 02:29:14  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 02:28:46  scott
| Updated to for buttons on LS10-GUI
|
| Revision 3.0  2000/10/10 12:15:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/14 01:59:05  scott
| Changes group and used from sel to lsl
|
| Revision 2.0  2000/07/15 09:00:01  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  1999/11/25 10:23:58  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.15  1999/11/16 09:41:53  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.14  1999/09/29 10:10:55  scott
| Updated to be consistant on function names.
|
| Revision 1.13  1999/09/16 04:11:33  scott
| Updated from Ansi Project
|
| Revision 1.12  1999/08/02 01:12:26  scott
| General update and clean up. Removed updating of password file.
|
| Revision 1.11  1999/07/29 01:05:56  scott
| Updated to change back.
|
| Revision 1.10  1999/07/28 06:50:19  scott
| Updated to change group sys to lsl
|
| Revision 1.9  1999/06/15 02:31:44  scott
| update to add log file + change database name etc.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: add_user.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/add_user/add_user.c,v 5.3 2001/11/08 01:10:28 scott Exp $";

#include 	<pslscr.h>
#include	<hot_keys.h>
#include 	<pwd.h>
#include 	<sys/types.h>
#include 	<grp.h>
#include	<license2.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>
#include	<tabdisp.h>

#ifndef	HAS_UID_T
typedef	int	uid_t;
typedef	int	gid_t;
#endif	/*HAS_UID_T*/

#define	SLEEP_TIME	2
#define strMove(d,s) memmove(d,s,strlen(s)+1)
#define TRIM(x) rmlead(rmtrail(x)) 

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

typedef	struct list_rec
{
	char	user_name[9];
	char	user_desc[31];
	long	user_perm[50];
	int		no_perm;
	int		added;
	struct	list_rec	*next;
} LIST;


LIST	*users;
LIST	*deleted;
LIST	*node;
LIST	*sec_ptr;

#define	TNAME		tptr->user_name
#define	TNEXTN		tptr->next->user_name
#define	TDESC		tptr->user_desc
#define	TPERM(x)	tptr->user_perm[x]
#define	TNO_PERM	tptr->no_perm
#define	TADDED		tptr->added
#define	TNEXT		tptr->next

#define	SNAME		sptr->user_name
#define	SNEXTN		sptr->next->user_name
#define	SDESC		sptr->user_desc
#define	SPERM(x)	sptr->user_perm[x]
#define	SNO_PERM	sptr->no_perm
#define	SADDED		sptr->added
#define	LCL_SNEXT		sptr->next	/* Renamed due to conflict */

#define	NNAME		node->user_name
#define	NNEXTN		node->next->user_name
#define	NDESC		node->user_desc
#define	NPERM(x)	node->user_perm[x]
#define	NNO_PERM	node->no_perm
#define	NADDED		node->added
#define	NNEXT		node->next

#define	SEC_NAME		sec_ptr->user_name
#define	SEC_DESC		sec_ptr->user_desc
#define	SEC_PERM(x)		sec_ptr->user_perm[x]
#define	SEC_NO_PERM		sec_ptr->no_perm
#define	SEC_ADDED		sec_ptr->added
#define	SEC_NEXT		sec_ptr->next

#define	U_NAME		local_rec.u_name
#define	U_DESC		local_rec.u_desc
#define	U_PERM		local_rec.u_perm

#define	P_LNUL		(LIST *)0
#define	P_CNUL		(char *)0

struct	passwd		*pass_rec;
struct	passwd		*getpwent(void);
struct	passwd		*getpwnam(const char *);

#define	PW_NAME		pass_rec->pw_name
#define	PW_PASSWD	pass_rec->pw_passwd
#define	PW_UID		pass_rec->pw_uid
#define	PW_GID		pass_rec->pw_gid
#define	PW_AGE		pass_rec->pw_age
#define	PW_GECOS	pass_rec->pw_gecos
#define	PW_DIR		pass_rec->pw_dir
#define	PW_SHELL	pass_rec->pw_shell

struct	group		*grp_rec;
struct	group		*getgrnam(const char *);

#define	GR_NAME		grp_rec->gr_name
#define	GR_PASSWD	grp_rec->gr_passwd
#define	GR_GID		grp_rec->gr_gid

uid_t	lsl_uid;
gid_t	lsl_gid;
uid_t	bin_uid;
gid_t	sys_gid;
int	exit_loop;
int	exit_sec;
int	sec_status;
int	add_user;

char	bname [100];

char	*passwd	= "/etc/passwd";

char	*home_dir	= bname;
char	*home_sh	= "/bin/sh";

FILE	*secure_out;

	/*======================================+
	 | Menu System Access Description File. |
	 +======================================*/
#define	MNAC_NO_FIELDS	3

	struct dbview	mnac_list [MNAC_NO_FIELDS] =
	{
		{"mnac_hhac_hash"},
		{"mnac_code"},
		{"mnac_description"}
	};

	struct tag_mnacRecord
	{
		long	hhac_hash;
		char	code [9];
		char	description [31];
	}	mnac_rec;

	char	*data	= "data",
			*mnac	= "mnac",
			*user	= "user";

struct
{
	char	dummy[11];
	char	u_name[255];
	char	u_desc[31];
	char	u_perm[2049];
	long	tmp_perm[50];
	int		tmp_no_perm;
} local_rec;

static	int	TAddFunction 	(int, KEY_TAB *);
static	int	TDeleteFunction (int, KEY_TAB *);
static	int	TModifyFunction (int, KEY_TAB *);
static	int	TShowFunction 	(int, KEY_TAB *);
static	int	TExitFunction 	(int, KEY_TAB *);

#ifdef	GVISION
KEY_TAB user_keys [] =
{
	{ " ADD ", 	  'A', TAddFunction, 	"Add a user to the system.", "A" },
	{ " DELETE ", 'D', TDeleteFunction, "Delete a user from the system.", "E" },
	{ " MODIFY ", 'M', TModifyFunction, "Modify a user.", "E" },
	{ " SECURITY ACCESS ", 'S', TShowFunction, "Display user access security access", "E" },
	{ NULL, FN16, TExitFunction, "Exit.", "A" },
	END_KEYS
};
#else
KEY_TAB user_keys [] =
{
	{ "[A]DD", 	  'A', TAddFunction, 	"Add a user to the system.", "A" },
	{ "[D]ELETE", 'D', TDeleteFunction, "Delete a user from the system.", "E" },
	{ "[M]ODIFY", 'M', TModifyFunction, "Modify a user.", "E" },
	{ "[S]ECURITY ACCESS", 'S', TShowFunction, "Display user access security access", "E" },
	{ NULL, FN16, TExitFunction, "Exit.", "A" },
	END_KEYS
};
#endif

static	int	TAddSecurityFunction (int c, KEY_TAB *psUnused);
static	int	TDelSecurityFunction (int c, KEY_TAB *psUnused);
static	int	TExitSecurityFunction (int c, KEY_TAB *psUnused);

#ifdef	GVISION
KEY_TAB security_keys [] =
{
	{ " ADD ", 	  'A', TAddSecurityFunction, "Add security to user.",      "A"},
	{ " DELETE ", 'D', TDelSecurityFunction, "Delete security from user.", "E"},
	{ NULL, FN16, TExitSecurityFunction, "Exit.", "A" },
	END_KEYS
};
#else
KEY_TAB security_keys [] =
{
	{ "[A]DD", 	  'A', TAddSecurityFunction, "Add security to user.",      "A"},
	{ "[D]ELETE", 'D', TDelSecurityFunction, "Delete security from user.", "E"},
	{ NULL, FN16, TExitSecurityFunction, "Exit.", "A" },
	END_KEYS
};
#endif

static	int	tag_sec_func 	(int, KEY_TAB *);
static	int	exit_tag_func 	(int, KEY_TAB *);

#ifdef	GVISION
KEY_TAB add_sec_keys [] =
{
	{ " TAG SECURITY ", 'T', tag_sec_func, "Add security to user.", "A" },
	{ NULL,	FN16, exit_tag_func, "Exit.", "A" },
	END_KEYS
};
#else
KEY_TAB add_sec_keys [] =
{
	{ "[T]AG SECURITY", 'T', tag_sec_func, "Add security to user.", "A" },
	{ NULL,	FN16, exit_tag_func, "Exit.", "A" },
	END_KEYS
};
#endif

static	struct	var	vars[] =
{
	{1, LIN, "u_name",	19, 10, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "User Name:", "Enter name of new user.",
		YES, NO,  JUSTLEFT, "abcdefghijklmnopqrstuvwxyz0123456789", "", local_rec.u_name},
	{1, LIN, "u_desc",	19, 43, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "User Description:", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.u_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*
 *	Function declarations
 */

int		main			(int argc, char * argv []);
void	OpenDB			(void);
void	CloseDB			(void);
void	LoadSecurity	(void);
int		user_tab_create (void);
void	security_access	(void);
int		GetSecurity		(void);
void	SecurityTabCreate (void);
int		SaveFile		(void);
int		spec_valid		(int field);
LIST *	ListInsert		(LIST * list_head,
						 char * user_name,
						 char * user_desc,
						 char * user_perm);
void	ProcessSecurity (char * user_perm);
LIST *	ListFind		(LIST * list_head, char * user_name);
void	ListDelete		(LIST * list_head, char * user_name);
LIST *	list_alloc		(void);
int		heading			(int scn);
char *	rmlead(char *);
char *	rmtrail(char *);
char *	trimall(char *);

/*=======================================
| Main Processing Routine.		|
=======================================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	c;
	char	*sptr = getenv ("PROG_PATH");

	SETUP_SCR (vars);

	/*---------------------------------------
	| get path for menu system		|
	---------------------------------------*/
	sprintf (bname, "%s/BIN", (sptr) ? sptr : "/usr/LS10.5");

	/*---------------------------------------
	| modifies systems files - superuser	|
	---------------------------------------*/
	if (geteuid () != (uid_t) 0)
	{
		print_at (0,0, ML(mlMenuMess109));
		return (EXIT_FAILURE);
	}
	OpenDB ();
	/*---------------------------------------
	| initialise lists			|
	---------------------------------------*/
	users	=	P_LNUL;
	deleted =	P_LNUL;
	init_scr ();
	set_tty ();
	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, TRUE);
	set_masks ();
	init_vars (1);
	/*---------------------------------------
	| load existing security file		|
	---------------------------------------*/
	LoadSecurity ();

	/*-----------------------------------
	| get group record for "lsl"		|
	-----------------------------------*/
	grp_rec = getgrnam ("lsl");
	if (grp_rec == (struct group *) 0)
		sys_err ("No Unix/Xenix Group lsl", -1, PNAME);
	lsl_gid = GR_GID;

	/*-----------------------------------
	| get password record for "lsl"		|
	-----------------------------------*/
	pass_rec = getpwnam ("lsl");
	if (pass_rec == (struct passwd *) 0)
		sys_err ("No Unix/Xenix User lsl", -1, PNAME);
	lsl_uid = PW_UID;

	exit_loop = FALSE;
	do
	{
		if (users == P_LNUL)
			set_keys (user_keys, "E", KEY_PASSIVE);
		else
			set_keys (user_keys, "E", KEY_ACTIVE);

		user_tab_create ();

		tab_scan (user);
		tab_close (user, TRUE);
		move (0, 16);
		cl_line ();
	} while (!exit_loop);

	crsr_on ();
	sprintf (err_str, ML(mlMenuMess164), "Security Files");
	c = prmptmsg (err_str, "YyNn", 0, 2);
	if (c == 'Y' || c == 'y')
	{
		int	status;

		c = prmptmsg (ML(mlMenuMess001), "YyNn", 0, 4);

		clear ();
		status = SaveFile ();
		if (status != 0)
			return (status);
		if (c == 'Y' || c == 'y')
		{
			rset_tty ();
			execlp ("fast_maint", "fast_maint", (char *) 0);
			return (EXIT_SUCCESS);
		}
	}

	rset_tty ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (mnac, mnac_list, MNAC_NO_FIELDS, "mnac_code");
}

void
CloseDB (void)
{
	abc_fclose (mnac);
	abc_dbclose (data);
}

void
LoadSecurity (void)
{
	FILE	*secure_in;
	char	line[100],
			progname [100];
	int		i;
	struct passwd	*pw;
	/*-------------------------------
	| open security file			|
	-------------------------------*/
	sprintf (progname, "%s/MENUSYS/User_secure", bname);
	scn_set (1);
	if ((secure_in = fopen (progname, "r")) == NULL)
	{
		sprintf (err_str, "Error in %s during (FOPEN)", progname);
		sys_err (err_str, errno, PNAME);
	}
	/*-------------------------------
	| load current users			|
	-------------------------------*/
	sec_status = FALSE;
	i = 0;
	while (i < MAXLINES && fgets (line, sizeof (line), secure_in))
	{
		char	uid [256];
		/*----------------------------------------
		| check if name exists in password file. |
		----------------------------------------*/
		sscanf (line, "%s", uid);
		pw = getpwnam (uid);
		if (pw)
		{
			LIST	*tptr;
			strcpy (local_rec.u_name, uid);	
			sprintf (local_rec.u_desc, "%-30.30s", pw -> pw_gecos);
			sprintf (local_rec.u_perm, "%-.2048s", strchr (line, '<') + 1);	
			downshift (local_rec.u_perm);
			if ((tptr = 	ListInsert 
						(
							users,
							local_rec.u_name,
							local_rec.u_desc,
							local_rec.u_perm)
				))
				users = tptr;
		}
	}
	fclose (secure_in);
}

/*============================================
| Create tabdisp table containing user names |
============================================*/
int
user_tab_create (void)
{
	LIST	*tptr;

	add_user = FALSE;
	sec_status = FALSE;
	heading (0);
	tab_open (user, user_keys, 2, 10, 11, FALSE);
	tab_add (user, "# %-10.10s    %-30.30s  ", "User Name", "User Description");

	tptr = users;
	while (tptr != P_LNUL)
	{
		tab_add 
		(
			user,
			"   %-8.8s    %-30.30s",
			TNAME,
			TDESC
		);
		tptr = TNEXT;
	}
	return (EXIT_SUCCESS);
}

/*---------------
| Delete a user |
---------------*/
static	int
TDeleteFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	char	tmp_user[9],
			tmp_line[100];

	tab_get (user, tmp_line, CURRENT, 0);
	sprintf (tmp_user, "%-8.8s", &tmp_line[3]);

	/*---------------------------------------
	| delete user				|
	---------------------------------------*/
	ListDelete (users, tmp_user);

	return (FN16);
}

/*------------
| Add a user |
------------*/
static	int
TAddFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	LIST	*tptr;

	heading (1);
	entry (1);

	sec_status = TRUE;
	add_user = TRUE;
	security_access ();

	tptr = 	ListInsert 
			(
				users, 
				U_NAME, 
				U_DESC, 
				" "
			);
	if (tptr != P_LNUL)
		users = tptr;

	return (FN16);
}

/*---------------------------------
| Modify user name or description |
---------------------------------*/
static	int
TModifyFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	LIST	*tptr;
	char	tmp_tab[101];
	int	i;

	tab_get (user, tmp_tab, CURRENT, 0);
	sprintf (local_rec.u_name, "%-8.8s", &tmp_tab[3]);
	sprintf (local_rec.u_desc, "%-30.30s", &tmp_tab[15]);
	tptr = users;
	while (tptr != P_LNUL && strcmp (TNAME, local_rec.u_name))
		tptr = TNEXT;

	heading (1);
	scn_display (1);
	edit (1);

	if (!restart)
	{
		sprintf (TNAME, "%-8.8s", local_rec.u_name);
		sprintf (TDESC, "%-30.30s", local_rec.u_desc);
		tab_update (user,
			"   %-8.8s    %-30.30s",
			TNAME,
			TDESC);
	}

	for (i = 18; i < 21; i++)
	{
		move (0, i);
		cl_line ();
	}
	crsr_off ();
	return (c);
}

/*------------------------------------------
| Display security access for current user |
------------------------------------------*/
static	int
TShowFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	sec_status = FALSE;
	security_access ();

	return (c);
}

/*---------------------------------------
| If sec_status is TRUE this is a new	|
| user and security for that user is	|
| empty.				|
---------------------------------------*/
void
security_access (void)
{
	char	tmp_tab[101],
			tmp_user[9];

	if (sec_status)
	{
		local_rec.tmp_no_perm = 0;
		sec_ptr = P_LNUL;
	}
	else
	{
		tab_get (user, tmp_tab, CURRENT, 0);
		sprintf (tmp_user, "%-8.8s", &tmp_tab[3]);
		sec_ptr = users;
		while (sec_ptr != P_LNUL && strcmp (SEC_NAME, tmp_user))
			sec_ptr = SEC_NEXT;
	}

	exit_sec = FALSE;
	do
	{
		if (sec_ptr == P_LNUL || (sec_ptr != P_LNUL && SEC_NO_PERM == 0))
		{
			set_keys (security_keys, "E", KEY_PASSIVE);
		}
		else
			set_keys (security_keys, "E", KEY_ACTIVE);

		SecurityTabCreate ();

		tab_scan ("security");
		tab_close ("security", TRUE);
	} while (!exit_sec);

	redraw_table (user);
}

/*-------------------------------
| Add to current users security |
-------------------------------*/
static	int
TAddSecurityFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	if (GetSecurity ())
	{
		if (add_user)
		{
			box (0, 18, 80, 1);
			scn_write (1);
			scn_display (1);
		}
		return (FN16);
	}
	else
	{
		redraw_table ("security");
		if (add_user)
		{
			box (0, 18, 80, 1);
			scn_write (1);
			scn_display (1);
		}
	}
	return (c);
}

/*---------------------------------------
| Display list of valid security codes	|
| that a user can add and allow these to|
| be tagged.				|
---------------------------------------*/
int
GetSecurity (void)
{
	int	i,
		lcl_cc,
		security_no = 0,
	 	ok_to_add,
		rtn_val;
	char	tmp_code[51];

	tab_open ("add_sec", add_sec_keys, 11, 37, 5, FALSE);
	tab_add ("add_sec", "# %-40.40s", " Select Security Access Codes");
	sprintf (mnac_rec.code, "%-8.8s", " ");
	lcl_cc = find_rec (mnac, &mnac_rec, GTEQ, "r");
	while (!lcl_cc)
	{
		ok_to_add = TRUE;
		if (sec_status)
		{
			for (i = 0; i < local_rec.tmp_no_perm; i++)
			{
				if (local_rec.tmp_perm[i] == mnac_rec.hhac_hash)
				{
					ok_to_add = FALSE;
					break;
				}
			}
		}
		else
		{
			for (i = 0; i < SEC_NO_PERM; i++)
			{
				if (SEC_PERM(i) == mnac_rec.hhac_hash)
				{
					ok_to_add = FALSE;
					break;
				}
			}
		}

		if (ok_to_add && strncmp (mnac_rec.code, "ERROR", 5))
		{
			tab_add ("add_sec",
				"  %-8.8s %-30.30s",
				mnac_rec.code,
				mnac_rec.description);
			security_no++;
		}

		lcl_cc = find_rec (mnac, &mnac_rec, NEXT, "r");
	}

	if (security_no == 0)
	{
		tab_add ("add_sec", "\007 There are no more security codes that ");
		tab_add ("add_sec", "     you can add to this user name.    ");
		tab_display ("add_sec", TRUE);
		sleep (sleepTime);
		tab_close ("add_sec", TRUE);
		return (FALSE);
	}
	else
		tab_scan ("add_sec");

	rtn_val = FALSE;

	for (i = 0; i < security_no; i++)
	{
		tab_get ("add_sec", tmp_code, EQUAL, i);
		if (tagged (tmp_code))
		{
			sprintf (mnac_rec.code, "%-8.8s", &tmp_code[2]);
			lcl_cc = find_rec (mnac, &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
				file_err (cc, mnac, "DBFIND");

			if (sec_status)
				local_rec.tmp_perm[local_rec.tmp_no_perm++] = mnac_rec.hhac_hash;
			else
				SEC_PERM(SEC_NO_PERM++) = mnac_rec.hhac_hash;
			rtn_val = TRUE;
		}
	}

	tab_close ("add_sec", TRUE);

	if (!sec_status && SEC_NO_PERM == 0)
		rtn_val = TRUE;

	if (sec_status && local_rec.tmp_no_perm == 0)
		rtn_val = TRUE;

	return (rtn_val);
}

/*------------------------------------
| Delete a security code from a user |
------------------------------------*/
static	int
TDelSecurityFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	char	tmp_code[51];
	long	del_hash;
	int	i,
		lcl_cc;

	tab_get ("security", tmp_code, CURRENT, 0);

	sprintf (mnac_rec.code, "%-8.8s", &tmp_code[1]);
	lcl_cc = find_rec (mnac, &mnac_rec, COMPARISON, "r");
	if (lcl_cc)
		file_err (cc, mnac, "DBFIND");

	del_hash = mnac_rec.hhac_hash;

	/*---------------------
	| Find hash to delete |
	---------------------*/
	for (i = 0; i < SEC_NO_PERM; i++)
		if (del_hash == SEC_PERM(i))
			break;

	/*------------------
	| Delete from list |
	------------------*/
	for (i++; i < SEC_NO_PERM; i++)
		SEC_PERM(i - 1) = SEC_PERM(i);

	SEC_NO_PERM--;

	return (FN16);
}

/*---------------------------
| Tag a security code for	|
| addition to a users		|
| list of security codes	|
---------------------------*/
static	int
tag_sec_func (
 int		c,
 KEY_TAB *	psUnused)
{
	tag_toggle ("add_sec");
	return (c);
}

static int
TExitSecurityFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	exit_sec = TRUE;
	return (FN16);
}

static int
exit_tag_func (
 int		c,
 KEY_TAB *	psUnused)
{
	return (FN16);
}

/*-----------------------
| Create list of users 	|
| security codes	    |
-----------------------*/
void
SecurityTabCreate (void)
{
	int		i,
			tmp_sec_no;

	tab_open ("security", security_keys, 2, 37, 11, FALSE);
	tab_add ("security", "# %-8.8s %-30.30s ", "Code", "Description");
	if ((sec_status && local_rec.tmp_no_perm == 0) || (!sec_status && SEC_NO_PERM == 0))
		tab_add ("security", " %-8.8s %-30.30s ", " ", " ");
	else
	{
		abc_selfield (mnac, "mnac_hhac_hash");

		if (sec_status)
			tmp_sec_no = local_rec.tmp_no_perm;
		else
			tmp_sec_no = SEC_NO_PERM;

		for (i = 0; i < tmp_sec_no; i++)
		{
			cc = find_hash (mnac, &mnac_rec, COMPARISON, "r", (sec_status) ? local_rec.tmp_perm[i] : SEC_PERM(i));
			if (cc)
				continue;

			tab_add ("security",
				" %-8.8s %-30.30s ",
				mnac_rec.code,
				mnac_rec.description);
		}

		abc_selfield (mnac, "mnac_code");
	}
}

static	int
TExitFunction (
 int		c,
 KEY_TAB *	psUnused)
{
	exit_loop = TRUE;

	return (c);
}

/*==================================
| Return 1 to exit, 0 to continue  |
==================================*/
int
SaveFile (void)
{
	char	progname[3][100];
	LIST	*tptr;
	int	i,
		first_time;

	/*---------------------------------------
	| set security file names		|
	---------------------------------------*/
	sprintf (progname[0], "%s/MENUSYS/User_secure.n", bname);
	sprintf (progname[1], "%s/MENUSYS/User_secure", bname);
	sprintf (progname[2], "%s/MENUSYS/User_secure.o", bname);

	/*---------------------------------------
	| open new security file		|
	---------------------------------------*/
	if ((secure_out = fopen (progname[0], "w")) == NULL)
	{
		sprintf (err_str, "Error in %s during (FOPEN)", progname[0]);
		sys_err (err_str, errno, PNAME);
	}
	/*---------------------------------------
	| save old security file		|
	---------------------------------------*/
	printf ("Saving Old User_secure ... ");
	fflush (stdout);
	if (fork () == 0)
	{
		execlp (
			"cp",
			"cp",
			progname[1],
			progname[2], (char *) 0);
		return (EXIT_SUCCESS);
	}
	else
		wait ((int *) 0);

	/*-------------------------------
	| write new security file		|
	-------------------------------*/
	abc_selfield (mnac, "mnac_hhac_hash");

	print_at (0,0, ML(mlMenuMess002));

	fflush (stdout);

	tptr = users;
	while (tptr != P_LNUL)
	{
		first_time = TRUE;
		fprintf (secure_out, "%-8.8s  <",TNAME);
		for (i = 0; i < TNO_PERM; i++)
		{
			cc = find_hash (mnac, &mnac_rec, COMPARISON, "r", TPERM(i));
			if (cc)
				continue;

			if (!first_time)
				fprintf (secure_out, "|");
			first_time = FALSE;

			fprintf (secure_out, "%-.8s", clip (mnac_rec.code));
		}
		fprintf (secure_out, ">\n");

		tptr = TNEXT;
	}
	fclose (secure_out);
	/*---------------------------------------
	| copy new security file to live	|
	---------------------------------------*/
	print_at (0,0, ML(mlMenuMess003));

	fflush (stdout);
	if (fork () == 0)
	{
		execlp ("mv",
			"mv",
			progname[0],
			progname[1], (char *) 0);
		return (EXIT_SUCCESS);
	}
	else
		wait ((int *) 0);
	/*---------------------------------------
	| set owner, group & permissions	|
	---------------------------------------*/
	set_file (progname[1]);

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int	field)
{
	char	*sptr,
			name[255];
	LIST	*tptr;

	/*-------------------------------
	| validate user name			|
	-------------------------------*/
	if (LCHECK ("u_name"))
	{
		/*-------------------------------
		| replace spaces with "_"		|
		-------------------------------*/
		strcpy (name, U_NAME);
		sptr = clip (name);
		while (*sptr)
		{
			*sptr = tolower (*sptr);
			if (*sptr == ' ')
				*sptr = '_';
			sptr++;
		}
		if (isdigit (name[0]))
		{
			print_mess (ML(mlMenuMess010));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		/*-------------------------------
		| check for duplicates			|
		-------------------------------*/
		sprintf (U_NAME, "%-8.8s", name);
		tptr = ListFind (users, U_NAME);
		if (tptr != P_LNUL)
		{
			print_mess (ML(mlMenuMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		display_field (field);
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| validate user description		|
	-------------------------------*/
	if (LCHECK ("u_desc"))
	{
		if (dflt_used)
			sprintf (U_DESC, "%-30.30s", U_NAME);

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

LIST	*
ListInsert(
 LIST *	list_head,
 char *	user_name,
 char *	user_desc,
 char *	user_perm)
{
	LIST	*sptr;
	int		i,
			lcl_cc;

	/*---------------------------------------
	| allocate record & initialise		|
	---------------------------------------*/
	node = list_alloc ();
	sprintf (NNAME, "%-8.8s", 	user_name);
	sprintf (NDESC, "%-30.30s", user_desc);

	NNO_PERM = 0;
	if (sec_status)
	{
		abc_selfield (mnac, "mnac_hhac_hash");
		for (i = 0; i < local_rec.tmp_no_perm; i++)
		{
			mnac_rec.hhac_hash	=	local_rec.tmp_perm [i];
			lcl_cc = find_rec (mnac, &mnac_rec, COMPARISON, "r");
			if (!lcl_cc)
				NPERM(NNO_PERM++) = mnac_rec.hhac_hash;
		}
		abc_selfield (mnac, "mnac_code");
	}
	else
		ProcessSecurity (user_perm);

	NADDED = FALSE;
	NNEXT = P_LNUL;
	/*---------------------------
	| empty list				|
	---------------------------*/
	if (list_head == P_LNUL)
		return (node);

	/*---------------------------
	| head of list				|
	---------------------------*/
	sptr = list_head;
	if (strcmp (NNAME, SNAME) < 0)
	{
		NNEXT = list_head;
		return (node);
	}
	/*-----------------------
	| duplicate				|
	-----------------------*/
	if (strcmp (NNAME, SNAME) == 0)
	{
		NNEXT = P_LNUL;
		return (P_LNUL);
	}
	/*-------------------------------
	| find position in list			|
	-------------------------------*/
	while (LCL_SNEXT != P_LNUL && strcmp (NNAME, SNEXTN) > 0)
		sptr = LCL_SNEXT;
	/*-----------------------
	| duplicate				|
	-----------------------*/
	if (LCL_SNEXT != P_LNUL && strcmp (NNAME, SNEXTN) == 0)
	{
		NNEXT = P_LNUL;
		return (list_head);
	}
	/*---------------------------
	| insert into list			|
	---------------------------*/
	NNEXT = (LCL_SNEXT == P_LNUL) ? P_LNUL : LCL_SNEXT;
	LCL_SNEXT = node;
	return (list_head);
}

void
ProcessSecurity (
 char *	user_perm)
{
	char	*sptr,
			*tptr,
			temp_char;
	int		lcl_cc;

	sptr = strdup (user_perm);

	while (*sptr)
	{
		/*----------------
		| Find separator |
		----------------*/
		tptr = sptr;
		while (*tptr && *tptr != '>' && *tptr != '|')
			tptr++;

		temp_char = *tptr;
		*tptr = '\0';
		sprintf (mnac_rec.code, "%-8.8s", sptr);

		/*-----------------
		| Check mnac file |
		-----------------*/
		lcl_cc = find_rec (mnac, &mnac_rec, COMPARISON, "r");
		if (!lcl_cc)
		{
			NPERM(NNO_PERM++) = mnac_rec.hhac_hash;
		}

		if (temp_char)
			sptr = tptr + 1;
		else
			*sptr = '\0';
	}
}

LIST	*
ListFind (
 LIST *	list_head,
 char *	user_name)
{
	LIST	*tptr = list_head;
	char	name[9];
	int	cmp;

	/*-----------------------------------
	| check if user_name exists in list	|
	-----------------------------------*/
	sprintf (name, "%-8.8s", user_name);
	while (tptr != P_LNUL)
	{
		cmp = strcmp (TNAME, name);
		/*---------------------------
		| exact match found			|
		---------------------------*/
		if (cmp == 0)
			return (tptr);

		/*---------------------------
		| not in list				|
		---------------------------*/
		if (cmp > 0)
			return (P_LNUL);
		tptr = TNEXT;
	}
	return (P_LNUL);
}

void
ListDelete (
 LIST *	list_head,
 char *	user_name)
{
	LIST	*tptr = list_head,
			*sptr = P_LNUL;
	char	name[9];
	int	cmp,
		found = FALSE;

	/*-----------------------------------
	| find user_name in current list	|
	-----------------------------------*/
	sprintf (name, "%-8.8s", user_name);
	while (tptr != P_LNUL)
	{
		cmp = strcmp (TNAME, name);
		if (cmp == 0)
		{
			found = TRUE;
			break;
		}
		if (cmp > 0)
			break;
		sptr = tptr;
		tptr = TNEXT;
	}
	/*---------------------------
	| find failed				|
	---------------------------*/
	if (!found)
		return;
	/*---------------------------
	| head of list				|
	---------------------------*/
	if (sptr == P_LNUL)
	{
		sptr = users;
		users = LCL_SNEXT;
		free ((char *) sptr);
	}
	else
	{
		LCL_SNEXT = TNEXT;
		free ((char *) tptr);
	}
	/*-------------------------------
	| add to deleted list			|
	-------------------------------*/
	tptr =	ListInsert 
			(
				deleted, 
				user_name, 
				" ", 
				" "
			);
	if (tptr != P_LNUL)
		deleted = tptr;
}

LIST *
list_alloc (void)
{
	return ((LIST *) malloc (sizeof (LIST)));
}

int
heading (
 int scn)
{
	if (scn == 0)
	{
		clear ();

		rv_pr (ML(mlMenuMess012), 31, 0, 1);

		move (0, 1);
		line (80);
		move (0, 21);
		line (80);
	}

	if (scn == 1)
	{
		scn_set (1);
		box (0, 18, 80, 1);
		scn_write (1);
	}

	return (EXIT_SUCCESS);
}



char *rmlead(char *str)
{
      char *obuf;

      if (str)
      {
            for (obuf = str; *obuf && isspace(*obuf); ++obuf)
                  ;
            if (str != obuf)
                  strMove(str, obuf);
      }
      return str;
}


char *rmtrail(char *str)
{
      int i;

      if (str && 0 != (i = strlen(str)))
      {
            while (--i >= 0)
            {
                  if (!isspace(str[i]))
                        break;
            }
            str[++i] = (char) NULL;
      }
      return str;
}


char *trimall(char *str)
 {
  return (rmlead(rmtrail(str)));
 }
