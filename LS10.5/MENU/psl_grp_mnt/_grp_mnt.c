/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_grp_mnt.c  )                                 |
|  Program Desc  : ( Maintains Mail Group File.                     ) |
|                : (                                                ) |
|---------------------------------------------------------------------|
|  Access Files  : N/A                                                |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Updates Files : N/A                                                |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 09/11/92         |
|---------------------------------------------------------------------|
|  Date Modified : (15/12/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (04/09/97)      | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (15/12/92)    : Fix bug in delete_func when compiling SCO.         |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :      print_at.                                     |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
|                                                                     |
| $Log: _grp_mnt.c,v $
| Revision 5.2  2001/08/09 05:13:43  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:35  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:48  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:01  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:27  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  2000/02/18 01:56:28  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.7  1999/12/06 01:47:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/11/16 09:42:00  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.5  1999/09/17 07:27:07  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:36:54  scott
| Update to add log + change database names + misc clean up.
|
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: _grp_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_grp_mnt/_grp_mnt.c,v 5.2 2001/08/09 05:13:43 scott Exp $";

#define NO_SCRGEN
#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <tabdisp.h>

#define	G_CREATE	0
#define	G_MODIFY	1
#define	G_SHOW		2

FILE *	usr_in;
FILE *	grp_in;
FILE *	grp_out;

struct	GRP_PTR
{
	char	grp_name [15];
	struct	USR_PTR	*usr_head;
	struct	GRP_PTR	*next;
};
#define	GRP_NULL	((struct GRP_PTR *) NULL)

struct	USR_PTR
{
	char	usr_name[15];
	struct	USR_PTR	*next;
};
#define	USR_NULL	((struct USR_PTR *) NULL)

struct  USR_PTR *usr_head;
struct  USR_PTR *usr_curr;
struct  USR_PTR *usr_free = USR_NULL;

struct  GRP_PTR *grp_head = GRP_NULL;
struct  GRP_PTR *grp_curr;

int		no_in_tab = 0;
int		no_of_groups = 0;
int		upd_group;
int		exit_loop;
char	directory [100];
char	usr_fname [100];
char	grp_fname [100];
char	get_buf [200];
char	group_line [151];

static int	create_func	(int, KEY_TAB *);
static int	modify_func	(int, KEY_TAB *);
static int	delete_func	(int, KEY_TAB *);
static int	show_func	(int, KEY_TAB *);
static int	abort_func	(int, KEY_TAB *);
static int	exit_func	(int, KEY_TAB *);

static	KEY_TAB group_keys [] =
{
   { "[C]REATE GROUP",	'C', create_func,
	"Create A Group And Add Users.",				"C" },
   { "[M]ODIFY GROUP",	'M', modify_func,
	"Modify Users In A Group.",					"M" },
   { NULL,		'\r', modify_func,
	"Modify Users In A Group.",					"M" },
   { "[D]ELETE GROUP",	'D', delete_func,
	"Delete A Group.",						"D" },
   { "[S]HOW GROUP",	'S', show_func,
	"Show All Users In A Group.",					"D" },
   { NULL,		FN1, abort_func,
	"Exit Without Saving Changes.",					"C" },
   { NULL,		FN16, exit_func,
	"Exit And Save Changes.",					"C" },
   END_KEYS
};

static int	user_tag_func	(int, KEY_TAB *);

static	KEY_TAB mod_keys [] =
{
   { "[T]AG USER",	'T', user_tag_func,
	"Tag / Untag User.",						"T" },
   { NULL,        	'\r', user_tag_func,
	"Tag / Untag User.",						"T" },
   END_KEYS
};

/*============================
| Local function prototypes  |
============================*/
void				shutdown_prog	(void);
int					load_users		(void);
int					load_groups		(void);
int					proc_group		(char *, char *);
int					add_to_grp		(char *, char *);
struct USR_PTR *	usr_alloc		(void);
int					free_usr_node	(struct USR_PTR *);
struct GRP_PTR *	grp_alloc		(void);
int					load_table		(void);
int					build_str		(struct GRP_PTR *);
int					chk_user		(char *);
int					update			(void);
int					valid_new_grp	(char *);
struct GRP_PTR *	delete_group	(char *);
int					fix_del_grp		(char *, struct GRP_PTR *);
int					del_null_grps	(void);
struct GRP_PTR *	mod_group		(char *, int);
int					heading			(int);


int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr = getenv ("PROG_PATH");

	sprintf(directory, "%s/BIN/MENUSYS", (sptr != (char *)0) ? sptr : "/usr/LS10.5");
	sprintf(usr_fname, "%s/Mail_secure", directory);
	sprintf(grp_fname, "%s/Mail_group", directory);

	if (getuid() != 0)
	{
		/*-----------------------------------------------
		| You must be a superuser to run this program\n |
		-----------------------------------------------*/
		print_at (0, 0, ML(mlStdMess140));
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();

	set_help (FN6, "FN6");

	/*-------------------------------
	| Set up screen ready for table |
	-------------------------------*/
	heading (0);

	/*--------------------------
	| Load user file into list |
	--------------------------*/
	load_users ();

	/*---------------------------
	| Load group file into list |
	---------------------------*/
	load_groups ();

	exit_loop = FALSE;
	do
	{
		if (grp_head == GRP_NULL)
		{
			create_func ('C', (KEY_TAB *) 0);
			tab_close("group", TRUE);
			continue;
		}
		else
			load_table ();

		tab_scan("group");

		if (!exit_loop)
			tab_close("group", TRUE);

	} while (!exit_loop);

	if (upd_group)
		update ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	FinishProgram ();
}

/*-------------------------------
| Read users from Mail_secure	|
| and store in a linked list	|
-------------------------------*/
int
load_users (
 void)
{
	struct	USR_PTR	*usr_lcl;
	char *	sptr;
	char *	tptr;

	/*------------------------------------
	| Read Mail_secure file and store it | 
	------------------------------------*/
	if ((usr_in = fopen(usr_fname, "r")) == 0)
		sys_err("Error in Mail_secure during (FOPEN)",errno,PNAME);

	/*----------------------------
	| Store users in linked list |
	----------------------------*/
	usr_head = USR_NULL;
	usr_curr = usr_head;

	sptr = fgets(err_str, 80, usr_in);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		/*---------------
		| Comment Line. |
		---------------*/
		if (*sptr == '#')
		{
			sptr = fgets(err_str, 80, usr_in);
			continue;
		}

		tptr = strchr (sptr, '\t');
		if (tptr)
			*tptr = 0;

		clip (sptr);

		/*--------------------------------------
		| Allocate memory and set next to null |
		--------------------------------------*/
		usr_lcl = usr_alloc();
		usr_lcl->next = USR_NULL;

		/*---------------------------
		| Set head to start of list |
		---------------------------*/
		if (usr_head == USR_NULL)
			usr_head = usr_lcl;
		else
			usr_curr->next = usr_lcl;

		/*----------------------------
		| store user name in current |
		----------------------------*/
		usr_curr = usr_lcl;
		sprintf(usr_curr->usr_name, "%-14.14s", sptr);
		clip(usr_curr->usr_name);

		sptr = fgets(err_str, 80, usr_in);
	}
	fclose(usr_in);

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Read groups from Mail_groups	|
| and store in a linked list	|
-------------------------------*/
int
load_groups (
 void)
{
	char	*sptr;
	char	*tptr;

	/*-----------------------------------
	| Read Mail_group file and store it | 
	-----------------------------------*/
	if ((grp_in = fopen(grp_fname, "r")) == 0)
		return(FALSE);

	/*-----------------------------
	| Store groups in linked list |
	-----------------------------*/
	grp_head = GRP_NULL;
	grp_curr = grp_head;

	sptr = fgets(err_str, 100, grp_in);
	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		/*---------------
		| Comment Line. |
		---------------*/
		if (*sptr == '#')
		{
			sptr = fgets(err_str, 100, grp_in);
			continue;
		}

		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		/*-------------------------------
		| No users in group definition. |
		-------------------------------*/
		if (*tptr == '\0')
		{
			sptr = fgets(err_str, 100, grp_in);
			continue;
		}

		/*----------------
		| Process group. |
		----------------*/
		*tptr = '\0';
		proc_group (sptr, tptr + 1);

		sptr = fgets(err_str, 100, grp_in);
	}

	fclose (grp_in);

	return (EXIT_SUCCESS);
}

/*------------------------
| Process current group. |
------------------------*/
int
proc_group (
 char *	grp_name,
 char *	grp_list)
{
	int	user_found;
	char	*sptr;
	char	*tptr;
	char	*xptr;
	char	user_name[15];
	struct	GRP_PTR	*grp_lcl;
	struct	USR_PTR	*usr_lcl;

	clip(grp_name);

	/*--------------------------------
	| Make sure name of group is not |
	| the name of an individual user |
	--------------------------------*/
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		if (!strcmp(grp_name, usr_lcl->usr_name))
			return(0);

		usr_lcl = usr_lcl->next;
	}

	/*------------------------
	| Get each user or group |
	| name from the list.    |
	------------------------*/
	sptr = grp_list;
	while (sptr)
	{
	    /*---------------------------
	    | Search for next separator |
	    ---------------------------*/
	    tptr = sptr;
	    while (*tptr && *tptr != '|')
		tptr++;

	    xptr = '\0';
	    if (*tptr)
	    {
		*tptr = '\0';
		xptr = tptr + 1;
	    }

	    /*-------------------------
	    | Check for user or group |
	    -------------------------*/	
	    sprintf(user_name, "%-14.14s", sptr);
	    clip(user_name);
	    user_found = FALSE;
	    usr_lcl = usr_head;
	    while (usr_lcl != USR_NULL)
	    {
		if (!strcmp(usr_lcl->usr_name, user_name))
		{
		    user_found = TRUE;
		    break;
		}

		usr_lcl = usr_lcl->next;
	    }

	    /*----------------------------------
	    | If user is a valid user or group |
	    | then add into current group.     |
	    ----------------------------------*/
	    if (!user_found)
	    {
		/*--------------------------------------------
		| See if user name is actually a group name. |
		--------------------------------------------*/
		grp_lcl = grp_head;
		while (grp_lcl != GRP_NULL)
		{
		    /*--------------------------------------------
		    | Found user_name which is actually a group. |
		    --------------------------------------------*/
		    if (!strcmp(grp_lcl->grp_name, user_name))
		    {
			user_found = TRUE;
			break;
		    }

		    grp_lcl = grp_lcl->next;
		}
	    }

	    if (user_found)
		add_to_grp (grp_name, user_name);

	    sptr = xptr;
	}

	return (EXIT_SUCCESS);
}

/*------------------------------------------
| Add user to group in alphabetical order. |
------------------------------------------*/
int
add_to_grp (
 char *	group,
 char *	user_name)
{
	int	fnd_group;
	int	dup_name;
	int	pos_fnd;
	struct	GRP_PTR *grp_lcl;
	struct	USR_PTR *usr_lcl;
	struct	USR_PTR *usr_tmp;
	struct	USR_PTR *usr_prv;

	/*-----------------------------
	| Can't add a group to itself |
	-----------------------------*/ 
	if (!strcmp(group, user_name))
		return (EXIT_SUCCESS);

	/*-------------------------------
	| Find group node if it exists. |
	-------------------------------*/
	fnd_group = FALSE;
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, group))
		{
			fnd_group = TRUE;
			break;
		}

		grp_lcl = grp_lcl->next;
	}

	/*--------------------------
	| Create a new group node. |
	--------------------------*/
	if (!fnd_group)
	{
		grp_lcl = grp_alloc();
		sprintf(grp_lcl->grp_name, "%-14.14s", group);
		clip(grp_lcl->grp_name);

		if (grp_head == GRP_NULL)
			grp_head = grp_lcl;
		else
			grp_curr->next = grp_lcl;
		
		grp_curr = grp_lcl;
	}

	/*----------------------------------------------
	| Find place for user in current group's list. |
	----------------------------------------------*/
	if (grp_lcl->usr_head == USR_NULL)
	{
		/*----------------------
		| First entry in list. |
		----------------------*/
		usr_lcl = usr_alloc();
		sprintf(usr_lcl->usr_name, "%-14.14s", user_name);
		clip(usr_lcl->usr_name);

		grp_lcl->usr_head = usr_lcl;
	}
	else
	{
		/*------------------------------
		| First alphabetical position. |
		------------------------------*/
		pos_fnd = FALSE;
		dup_name = FALSE;
		usr_prv = grp_lcl->usr_head;
		usr_tmp = grp_lcl->usr_head;
		while (usr_tmp != USR_NULL)
		{
			/*------------------------------
			| User is already in the list. |
			------------------------------*/
			if (!strcmp(usr_tmp->usr_name, user_name))
			{
				dup_name = TRUE;
				break;
			}

			if (strcmp(user_name, usr_tmp->usr_name) < 0)
			{
				usr_lcl = usr_alloc();
				sprintf(usr_lcl->usr_name,"%-14.14s",user_name);
				clip(usr_lcl->usr_name);
			
				if (usr_prv == usr_tmp)
				{
					/*--------------
					| Head insert. |
					--------------*/
					usr_lcl->next = grp_lcl->usr_head;
					grp_lcl->usr_head = usr_lcl;
				}
				else
				{
					/*----------------
					| Middle insert. |
					----------------*/
					usr_lcl->next = usr_tmp;
					usr_prv->next = usr_lcl;
				}

				pos_fnd = TRUE;
				break;
			}

			usr_prv = usr_tmp;
			usr_tmp = usr_tmp->next;
		}

		/*---------------------------------------
		| Append to end of user list for group. |
		---------------------------------------*/
		if (!pos_fnd && !dup_name)
		{
			usr_lcl = usr_alloc();
			sprintf(usr_lcl->usr_name, "%-14.14s", user_name);
			clip(usr_lcl->usr_name);
	
			usr_prv->next = usr_lcl;
		}
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct USR_PTR *
usr_alloc (
 void)
{
	struct	USR_PTR	*lcl_ptr;

	/*--------------------------------------
	| Get node from free list if possible. |
	--------------------------------------*/
	lcl_ptr = usr_free;
	if (lcl_ptr != USR_NULL)
	{
		usr_free = lcl_ptr->next;
		lcl_ptr->next = USR_NULL;
		return(lcl_ptr);
	}

	lcl_ptr = (struct USR_PTR *) malloc (sizeof (struct USR_PTR));
	if (lcl_ptr == USR_NULL)
	{
        	sys_err("Error allocating memory for usr list During (MALLOC)",
			errno,
			PNAME);
	}
		
	sprintf(lcl_ptr->usr_name, "%-14.14s", " ");
	lcl_ptr->next = USR_NULL;

	return (lcl_ptr);
}

/*------------------------------
| Place node on the free list. |
------------------------------*/
int
free_usr_node (
 struct USR_PTR *	lst_ptr)
{
	/*--------------------------------
	| Transfer A Node Onto Free List |
	--------------------------------*/
	lst_ptr->next = usr_free;
	usr_free = lst_ptr;

	return (EXIT_SUCCESS);
}

/*-------------------------------------
| Allocate memory for usr linked list |
-------------------------------------*/
struct	GRP_PTR *
grp_alloc (
 void)
{
	struct	GRP_PTR	*lcl_ptr;

	lcl_ptr = (struct GRP_PTR *) malloc (sizeof (struct GRP_PTR));

	if (lcl_ptr == GRP_NULL)
	{
        	sys_err("Error allocating memory for grp list During (MALLOC)",
			errno,
			PNAME);
	}

	sprintf(lcl_ptr->grp_name, "%-14.14s", " ");
	lcl_ptr->usr_head = USR_NULL;
	lcl_ptr->next = GRP_NULL;
		
	return (lcl_ptr);
}

/*-----------------------------
| Load group list into table. |
-----------------------------*/
int
load_table (
 void)
{
	struct	GRP_PTR *grp_lcl;

	no_of_groups = 0;
	tab_open("group", group_keys, 2, 0, 15, FALSE);
	tab_add("group", 
		"# %-16.16s  %-100.100s  %-6.6s  ", 
		"   GROUP", 
		"  USERS IN GROUP",  
		"MORE >");

	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		/*-------------------------------
		| Build line to store in table. |
		-------------------------------*/
		build_str (grp_lcl);

		tab_add("group", group_line);
		no_of_groups++;
	
		grp_lcl = grp_lcl->next;
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Build line to store in table. |
-------------------------------*/
int
build_str (
 struct GRP_PTR *	grp_ptr)
{
	int		more_in_group;
	char	tmp_name [15];
	char	group_name [15];
	char	tmp_users [151];
	char	user_list [101];
	char *	sptr;

	sprintf(group_name, "%-14.14s", grp_ptr->grp_name);

	more_in_group = FALSE;
	strcpy(tmp_users, "");
	usr_curr = grp_ptr->usr_head;
	while (usr_curr != USR_NULL)
	{
		strcat(tmp_users, usr_curr->usr_name);
		strcat(tmp_users, " ");

		usr_curr = usr_curr->next;

		if (strlen(tmp_users) >= 100)
		{
			if (usr_curr != USR_NULL)
				more_in_group = TRUE;
			break;
		}
	}

	/*----------------------------
	| Check last user in string. |
	----------------------------*/
	sprintf(user_list, "%-100.100s", tmp_users);
	clip(user_list);
	sptr = strrchr(user_list, ' ');
	if (sptr)
	{
		sprintf(tmp_name, "%-14.14s", sptr + 1);
		clip(tmp_name);
		if (!chk_user(tmp_name))
		{
			*sptr = '\0';
			more_in_group = TRUE;
		}
	}

	sprintf(group_line,
		"  %-14.14s  %-100.100s    %-1.1s",
		group_name,
		user_list,
		(more_in_group) ? "*" : " ");

	return (EXIT_SUCCESS);
}

/*-------------------------------
| Check if user is a valid user |
| from Mail_secure		|
-------------------------------*/
int
chk_user (
 char *	key_val)
{
	struct	USR_PTR	*usr_lcl;
	struct	GRP_PTR	*grp_lcl;

	/*------------------
	| Check user list. |
	------------------*/
	usr_lcl = usr_head;
	while (usr_lcl != USR_NULL)
	{
		if (!strcmp(clip(usr_lcl->usr_name), key_val))
			return(TRUE);

		usr_lcl = usr_lcl->next;
	}

	/*-------------------
	| Check group list. |
	-------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(clip(grp_lcl->grp_name), key_val))
			return(TRUE);

		grp_lcl = grp_lcl->next;
	}

	putchar (BELL);
	return (FALSE);
}

/*-------------------------
| Update Mail_group file. |
-------------------------*/
int
update (
 void)
{
	int	c;
	struct	GRP_PTR	*grp_lcl;
	struct	USR_PTR	*usr_lcl;

	/*---"\007 Update %s ? "---*/
	sprintf(err_str,  ML(mlMenuMess164), grp_fname);
	c = prmptmsg(err_str, "YyNn", 40, 22);
	move(0, 22);
	cl_line ();
	if (c == 'N' || c == 'n')
	{
		tab_close ("group", TRUE);
		return (FALSE);
	}

	/*------------------------
	| Open file for writing. |
	------------------------*/
	if ((grp_out = fopen(grp_fname, "w")) == 0)
		sys_err("Error in Mail_group during (FOPEN)", errno, PNAME);

	/*---------------------------
	| Output file heading info. |
	---------------------------*/
	fprintf(grp_out, "# This file associates mail users with group names.\n");
	fprintf(grp_out, "# Group names may be a maximum of 14 characters in length\n");
	fprintf(grp_out, "# A user may belong to more than one group.\n");
	fprintf(grp_out, "# A group may belong to another group, however the child group must be defined\n");
	fprintf(grp_out, "#   in the file BEFORE it is included in another group.\n");
	fprintf(grp_out, "#\n");
	fprintf(grp_out, "#Group  Users\n");
	fprintf(grp_out, "#\n");

	/*----------------
	| Output groups. |
	----------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		fprintf(grp_out, "%s\t", grp_lcl->grp_name);

		usr_lcl = grp_lcl->usr_head;
		while (usr_lcl != USR_NULL)
		{
			fprintf(grp_out, "%s", usr_lcl->usr_name);
			usr_lcl = usr_lcl->next;

			if (usr_lcl != USR_NULL)
				fprintf(grp_out, "|");
		}

		fprintf(grp_out, "\n");

		grp_lcl = grp_lcl->next;
	}

	tab_close("group", TRUE);

	return (TRUE);
}

/*---------------------
| Create A New Group. |
---------------------*/
static int
create_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int		i;
	int		grp_ok;
	char	tmp_name[15];
	char	new_group[15];
	struct	GRP_PTR	*grp_lcl, *grp_tmp;

	if (grp_head == GRP_NULL)
	{
		tab_open("group", group_keys, 2, 0, 15, FALSE);
		tab_add("group", 
			"# %-16.16s  %-100.100s  %-6.6s  ", 
			"   GROUP", 
			"  USERS IN GROUP",  
			"MORE >");
	}

	/*---------------------------
	| Get valid new group name. |
	---------------------------*/
	do
	{
		/*---Enter Name Of Group To Create : ---*/
		print_at(22, 10, ML(mlMenuMess167) );
		crsr_on();
		getalpha(45, 22, "AAAAAAAAAAAAAA", new_group);
		crsr_off();
		move(0, 22);
		cl_line();

		grp_ok = valid_new_grp(new_group);
	} while (!grp_ok);
	
	if (grp_ok == 2)
	{
		if (grp_head == GRP_NULL)
		{
			exit_loop = TRUE;
			return(FN16);
		}

		return (c);
	}

	/*------------------------
	| Create new group node. |
	------------------------*/
	grp_lcl = grp_alloc();
	sprintf(grp_lcl->grp_name, "%-14.14s", new_group);
	clip(grp_lcl->grp_name);

	if (grp_head == GRP_NULL)
		grp_head = grp_lcl;
	else
		grp_curr->next = grp_lcl;
	
	grp_curr = grp_lcl;

	/*---------------------------------------
	| Allow tagging of users for new group. |
	---------------------------------------*/
	grp_lcl = mod_group (new_group, G_CREATE);
	
	/*--------------------------------
	| Create list from tagged users. |
	--------------------------------*/
	for (i = 0; i < no_in_tab; i++)
	{
		tab_get("mod_grp", get_buf, EQUAL, i);
		if (!tagged(get_buf))
			continue;

		sprintf(tmp_name, "%-14.14s", get_buf + 2);
		clip(tmp_name);

		/*--------------------
		| Add user to group. |
		--------------------*/
		add_to_grp (new_group, tmp_name);
	}
	tab_close("mod_grp", TRUE);

	/*------------------
	| No users tagged. |
	------------------*/
	if (grp_lcl->usr_head == USR_NULL)
	{
		/*----------------------------------------------------
		| You must select at least one user for a new group. |
		----------------------------------------------------*/
		print_mess( ML(mlMenuMess166) ); 
		sleep(2);
		clear_mess();
		grp_tmp = delete_group (new_group);

		return (FN16);
	}

	return (FN16);
}

/*-------------------------------------------
| Check that name is valid for a new group. |
-------------------------------------------*/
int
valid_new_grp (
 char *	grp_name)
{
	struct	GRP_PTR	*grp_lcl;

	if (last_char == FN1)
		return (2);

	/*-----------------------------
	| Check for zero length name. |
	-----------------------------*/
	if (strlen(clip(grp_name)) == 0)
	{
		/*--------------------
		| Invalid Group Name |
		--------------------*/
		print_mess( ML(mlStdMess149) );
		sleep (sleepTime);
		clear_mess();
		return (FALSE);
	}

	/*--------------------------------
	| Check for existing group name. |
	--------------------------------*/
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, grp_name))
		{
			/*----------------------
			| Group Already Exists |
			----------------------*/
			print_mess( ML(mlStdMess150) );
			sleep(2);
			clear_mess();
			return (FALSE);
		}

		grp_lcl = grp_lcl->next;
	}

	return (TRUE);
}

/*-----------------
| Modify A Group. |
-----------------*/
static int
modify_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int		i;
	char	grp_name[15];
	char	tmp_name[15];
	struct  GRP_PTR *grp_ptr, *nxt_grp;
	struct  USR_PTR *usr_lcl, *usr_tmp;

	tab_get("group", get_buf, CURRENT, 0);
	sprintf(grp_name, "%-14.14s", get_buf + 2);
	clip(grp_name);

	/*-----------------------------
	| Display all users in group. |
	-----------------------------*/
	grp_ptr = mod_group (grp_name, G_MODIFY);

	/*--------------------------------
	| Clear all users from old list. |
	--------------------------------*/
	usr_lcl = grp_ptr->usr_head;
	while (usr_lcl != USR_NULL)
	{
		usr_tmp = usr_lcl->next;

		free_usr_node (usr_lcl);

		usr_lcl = usr_tmp;
	}
	grp_ptr->usr_head = USR_NULL;

	/*----------------------------------
	| Recreate list from tagged users. |
	----------------------------------*/
	for (i = 0; i < no_in_tab; i++)
	{
		tab_get("mod_grp", get_buf, EQUAL, i);
		if (!tagged(get_buf))
			continue;

		sprintf(tmp_name, "%-14.14s", get_buf + 2);
		clip(tmp_name);

		/*--------------------
		| Add user to group. |
		--------------------*/
		add_to_grp (grp_name, tmp_name);
	}
	tab_close("mod_grp", TRUE);

	/*-------------------------------------
	| Delete group if there are no users. |
	-------------------------------------*/
	if (grp_ptr->usr_head == USR_NULL)
	{
		nxt_grp = delete_group (grp_name);

		/*----------------------------------------
		| Remove all ocurrences of deleted group |
		| from all subsequent groups in the list |
		----------------------------------------*/
		fix_del_grp(grp_name, nxt_grp);
		del_null_grps ();

		return (FN16);
	}

	/*-------------------------------------
	| Update group line in "group" table. |
	-------------------------------------*/
	build_str(grp_ptr);
	tab_update("group", group_line);

	/*------------------------
	| Redraw 'parent' table. |
	------------------------*/
	tab_display("group", TRUE);
	redraw_keys("group");

	return (c);
}

/*-----------------
| Delete A Group. |
-----------------*/
static int
delete_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	tmp_group[15];
	struct  GRP_PTR *nxt_grp;

	tab_get("group", get_buf, CURRENT, 0);
	sprintf(tmp_group, "%-14.14s", get_buf + 2);
	clip(tmp_group);

	/*---"\007 Are you sure you want to delete group %s ? "---*/
	sprintf(err_str, ML(mlMenuMess165), tmp_group);
	c = prmptmsg(err_str, "YyNn", 40, 22);
	move(0, 22);
	cl_line();
	if (c == 'N' || c == 'n')
		return (c);

	/*---------------------
	| Perform the delete. |
	---------------------*/
	nxt_grp = delete_group (tmp_group);

	/*----------------------------------------
	| Remove all ocurrences of deleted group |
	| from all subsequent groups in the list |
	----------------------------------------*/
	fix_del_grp(tmp_group, nxt_grp);
	del_null_grps ();

	return (FN16);
}

/*---------------------
| Perform the delete. |
---------------------*/
struct GRP_PTR *
delete_group (
 char *	grp_name)
{
	struct USR_PTR *usr_lcl, *usr_tmp;
	struct GRP_PTR *grp_lcl, *grp_tmp, *grp_del = (struct GRP_PTR *) 0;

	grp_lcl = grp_head;
	grp_tmp = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		/*------------------------
		| Found group to delete. |
		------------------------*/
		if (!strcmp(grp_lcl->grp_name, grp_name))
		{
			/*-------------------------------
			| Store pointer to next node in |
			| list for fix_del_grp().       |
			-------------------------------*/
			grp_del = grp_lcl->next;

			/*---------------------------------
			| Group being deleted is the tail |
			| of the list so set grp_curr.    |
			---------------------------------*/
			if (grp_del == GRP_NULL)
			{
				if (grp_tmp == grp_head)
					grp_curr = GRP_NULL;
				else
					grp_curr = grp_tmp;
			}

			/*------------------------
			| Free the list of users |
			| within the group.      |
			------------------------*/
			usr_lcl = grp_lcl->usr_head;
			while (usr_lcl != USR_NULL)
			{
				usr_tmp = usr_lcl->next;
				free_usr_node (usr_lcl);
				usr_lcl = usr_tmp;
			}

			/*--------------------
			| Delete group node. |
			--------------------*/
			if (grp_lcl == grp_tmp)
			{
				/*--------------
				| Head delete. |
				--------------*/
				grp_head = grp_lcl->next;
			}
			else
			{
				/*------------------------
				| Middle or tail delete. |
				------------------------*/
				grp_tmp->next = grp_lcl->next;
			}
			free(grp_lcl);
			break;
		}

		grp_tmp = grp_lcl;
		grp_lcl = grp_lcl->next;
	}

	return (grp_del);
}

/*----------------------------------------
| Remove all ocurrences of deleted group |
| from all subsequent groups in the list |
----------------------------------------*/
int
fix_del_grp (
 char *				grp_name,
 struct GRP_PTR *	grp_del)
{
	char	tmp_name[15];
	struct	USR_PTR *usr_lcl, *usr_tmp;
	struct	GRP_PTR *grp_lcl, *grp_tmp, *nxt_grp;

	/*------------------------
	| End of list so return. |
	------------------------*/
	if (grp_del == GRP_NULL)
		return (EXIT_SUCCESS);

	/*--------------------------------
	| Remove user (group) name from  |
	| current AND subsequent groups. |
	--------------------------------*/
	grp_lcl = grp_del;
	while (grp_lcl != GRP_NULL)
	{
		grp_tmp = grp_lcl->next;

		/*----------------------------------------------
		| Remove user (group) name from current group. |
		----------------------------------------------*/
		usr_lcl = grp_lcl->usr_head;
		usr_tmp = usr_lcl;
		while (usr_lcl != USR_NULL)
		{
			if (!strcmp(usr_lcl->usr_name, grp_name))
			{	
				if (usr_lcl == usr_tmp)
				{
					grp_lcl->usr_head = usr_lcl->next;
					free_usr_node (usr_lcl);
				}
				else
				{	
					usr_tmp->next = usr_lcl->next;
					free_usr_node (usr_lcl);
				}
				break;
			}
	
			usr_tmp = usr_lcl;
			usr_lcl = usr_lcl->next;
		}

		/*---------------------------------------------------------
		| If there are no users left in the group then delete it. |
		---------------------------------------------------------*/
		if (grp_lcl->usr_head == USR_NULL)
		{
			sprintf(tmp_name, "%-14.14s", grp_lcl->grp_name);
			clip(tmp_name);
			nxt_grp = grp_lcl->next;
	
			fix_del_grp (tmp_name, nxt_grp);
		}
		
		grp_lcl = grp_tmp;
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------------------
| Delete those groups that have no users. |
-----------------------------------------*/
int
del_null_grps (
 void)
{
	struct	GRP_PTR	*lcl_ptr;

	lcl_ptr = grp_head;
	while (lcl_ptr != GRP_NULL)
	{
		if (lcl_ptr->usr_head == USR_NULL)
			lcl_ptr = delete_group(lcl_ptr->grp_name);
		else
			lcl_ptr = lcl_ptr->next;
	}

	return (EXIT_SUCCESS);
}

/*----------------------------
| Show All Users In A Group. |
----------------------------*/
static int
show_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	grp_name [15];
	struct  GRP_PTR *grp_ptr;

	tab_get("group", get_buf, CURRENT, 0);
	sprintf(grp_name, "%-14.14s", get_buf + 2);
	clip(grp_name);

	/*-----------------------------
	| Display all users in group. |
	-----------------------------*/
	grp_ptr = mod_group (grp_name, G_SHOW);
	tab_close("mod_grp", TRUE);

	/*------------------------
	| Redraw 'parent' table. |
	------------------------*/
	tab_display("group", TRUE);
	redraw_keys("group");

	return (c);
}

/*----------------
| Exit and Save. |
----------------*/
static int
exit_func (
 int		c,
 KEY_TAB *	psUnused)
{
	upd_group = TRUE;
	exit_loop = TRUE;
	
	return (FN16);
}

/*----------------------
| Exit But DON'T Save. |
----------------------*/
static int
abort_func (
 int		c,
 KEY_TAB *	psUnused)
{
	upd_group = FALSE;
	exit_loop = TRUE;
	
	return (FN16);
}

/*------------------------------------------------
| Display all users in group.                    |
| If disp_mode is not U_SHOW then allow tagging. |
------------------------------------------------*/
struct GRP_PTR *
mod_group (
 char *	grp_name,
 int	disp_mode)
{
	int		grp_fnd;
	int		in_grp;
	char	tmp_str [25];
	struct	USR_PTR	*lcl_ptr, *chk_ptr;
	struct	GRP_PTR	*grp_lcl, *grp_tmp;

	/*-------------
	| Find group. |
	-------------*/
	grp_fnd = FALSE;
	grp_lcl = grp_head;
	while (grp_lcl != GRP_NULL)
	{
		if (!strcmp(grp_lcl->grp_name, grp_name))
		{
			grp_fnd = TRUE;
			break;
		}

		grp_lcl = grp_lcl->next;
	}
	if (!grp_fnd)
		return (GRP_NULL);
	
	/*---------------
	| Set hot keys. |
	---------------*/
	if (disp_mode == G_SHOW)
		set_keys(mod_keys, "T", KEY_PASSIVE);
	else
		set_keys(mod_keys, "T", KEY_ACTIVE);

	/*-------------
	| Open table. |
	-------------*/
	tab_open("mod_grp", mod_keys, 4, 100, 10, FALSE);
	tab_add("mod_grp", "#  GROUP : %-14.14s    ", grp_name);

	/*------------------------
	| Load users into table. |
	------------------------*/
	no_in_tab = 0;
	lcl_ptr = (disp_mode == G_SHOW) ? grp_lcl->usr_head : usr_head;
	while (lcl_ptr != USR_NULL)
	{
	    in_grp = FALSE;
	    if (disp_mode == G_CREATE || disp_mode == G_MODIFY)
	    {
		/*--------------------------
		| Check if user is already |
		| in the current group.    |
		--------------------------*/
		chk_ptr = grp_lcl->usr_head;
		while (chk_ptr != USR_NULL)
		{
		    if (!strcmp(lcl_ptr->usr_name, chk_ptr->usr_name))
		    {
			in_grp = TRUE;
			break;
		    }

		    chk_ptr = chk_ptr->next;
		}
	    }
	    else
		in_grp = TRUE;

	    sprintf(tmp_str,
		    "%-1.1s %-14.14s  ",
		    (disp_mode == G_MODIFY && in_grp) ? "*" : " ",
		    lcl_ptr->usr_name);

	    tab_add("mod_grp", tmp_str);
	    no_in_tab++;

	    lcl_ptr = lcl_ptr->next;
	}

	/*-----------------------------
	| Load group names into list. |
	-----------------------------*/
	if (disp_mode != G_SHOW)
	{
	    grp_tmp = grp_head;
	    while (grp_tmp != GRP_NULL)
	    {	
		/*--------------------------------
		| Groups up to but not including |
		| the current group are OK.      |
		--------------------------------*/
		if (!strcmp(grp_tmp->grp_name, grp_name))
		    break;

		/*--------------------------
		| Check if user is already |
		| in the current group.    |
		--------------------------*/
		in_grp = FALSE;
		chk_ptr = grp_lcl->usr_head;
		while (chk_ptr != USR_NULL)
		{
		    if (!strcmp(grp_tmp->grp_name, chk_ptr->usr_name))
		    {
			in_grp = TRUE;
			break;
		    }
		
		    chk_ptr = chk_ptr->next;
		}
	    	sprintf(tmp_str,
	    		"%-1.1s %-14.14s (GROUP)",
	    		(disp_mode == G_MODIFY && in_grp) ? "*" : " ",
	    		grp_tmp->grp_name);
		
		tab_add("mod_grp", tmp_str);
		no_in_tab++;

		grp_tmp = grp_tmp->next;
		}
	}

	tab_scan ("mod_grp");

	return (grp_lcl);
}

/*----------------------
| Tag users for group. |
----------------------*/
static int
user_tag_func (
 int		c,
 KEY_TAB *	psUnused)
{
	tag_toggle ("mod_grp");

	return (c);
}

int
heading (
 int scn)
{
	swide();
	clear();
	if (scn == 0)
	{
		move(0, 1);
		line(132);

		rv_pr( ML(mlMenuMess168), 52, 0, 1);
		
		print_at(0, 121, "%-10.10s", DateToString (TodaysDate()));
	}
	return (EXIT_SUCCESS);
}
