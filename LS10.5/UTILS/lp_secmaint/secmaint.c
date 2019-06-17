/*=====================================================================
|  Copyright (C) 1988 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( lp_secmaint.c )                                  |
|  Program Desc  : ( Logistic Printer Security Maintenance        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : prntype, LP_SECURE,    ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files : LP_SECURE  ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Trev van Bremen | Date Written  : 14/11/90         |
|---------------------------------------------------------------------|
|  Date Modified : (12/09/97)      | Modified  by  : Ana Marie Tario  |
|                                                                     |
|  Comments      : (12/09/97) : Incorporated multilingual conversion  |
|                :              and DMY4 date.						  |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: secmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/lp_secmaint/secmaint.c,v 5.2 2001/08/09 09:27:02 scott Exp $";

#include	<ml_utils_mess.h>
#include	<pslscr.h>
#include	<hot_keys.h>
#include	<getnum.h>
#include    <tabdisp.h>

int Tag_all (int c, KEY_TAB *psUnused);
int untag_all (int c, KEY_TAB *psUnused);
int tag_func (int c, KEY_TAB *psUnused);
int untag_func (int c, KEY_TAB *psUnused);
int queue_func (int c, KEY_TAB *psUnused);
int class_func (int c, KEY_TAB *psUnused);

KEY_TAB	keys[] =
{
	{ "[^A]ccept All",	CTRL('A'), Tag_all,	"Tag ALL printers"	},
	{ "[^U]ntag",	CTRL('U'), untag_all,	"Untag ALL printers"	},
	{ "[T]ag",	'T', tag_func,		"Tag current printer"	},
	{ "[U]ntag",	'U', untag_func,	"Untag current printer"	},
	{ "[Q]ueue",	'Q', queue_func,	"Tag printers by Queue"	},
	{ "[C]lass",	'C', class_func,	"Tag printers by Class"	},
	END_KEYS
};

struct	PRNTYPE
{
	int	tag_mode;
	char	type[16];
	char	queue[16];
	char	desc[61];
	struct	PRNTYPE	*lpno_next;
	struct	PRNTYPE	*type_next;
	struct	PRNTYPE	*queue_next;
};
#define	PRN_NULL	((struct PRNTYPE *) NULL)

#define	UNTAGGED	0x00
#define	GLB_TAG		0x01
#define	USR_TAG		0x02
#define	WRITTEN		0x04

FILE	*lp_secure;
char	*prog_path;

struct	PRNTYPE	*head_lpno = PRN_NULL;
struct	PRNTYPE	*head_type = PRN_NULL;
struct	PRNTYPE	*head_queue = PRN_NULL;
struct	PRNTYPE	*curr_lpno = PRN_NULL;
struct	PRNTYPE	*curr_type = PRN_NULL;
struct	PRNTYPE	*curr_queue = PRN_NULL;
struct	PRNTYPE	*prev_type = PRN_NULL;
struct	PRNTYPE	*prev_queue = PRN_NULL;

char	username[16];
char	*printers = "printers";

#include	<wild_search.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int process (void);
int heading (int scn);
void read_prntype (void);
void insert_prntype (struct PRNTYPE *new_prntype);
void clear_tags (void);
void load_tags (char *lcl_user);
void _load_tags (char *lcl_user, char *uptr, char *tptr, char *qptr);
void srch_users (char *key_val);
void modify_tags (void);
void set_tag (int line);
void reset_tag (int line);
void update_tags (void);
void _queue_upd (void);
void _type_upd (void);
void _lpno_upd (void);
void add_line (char *type, char *queue);

int
main (
 int                argc,
 char*              argv[])
{
	char	filename[129];

	prog_path = getenv ("PROG_PATH");

	umask (0);
	sprintf (filename, "%s/BIN/MENUSYS/LP_SECURE", (prog_path) ? prog_path : "/usr/LS10.5");
	if ((lp_secure = fopen (filename, "r")) == NULL)
	{
		sprintf (err_str, "Error in %s During (FOPEN)", filename);
		sys_err (err_str, errno, PNAME);
	}

	init_scr ();
	swide ();
	set_tty ();

	read_prntype ();

	while (!process ());

	fclose (lp_secure);

	clear ();
	snorm ();
	rset_tty ();

    return (EXIT_SUCCESS);
}

int
process (void)
{
	heading (1);

	clear_tags ();
	/*print_at (2, 0, "User : ");*/
	print_at (2, 0, ML(mlUtilsMess068));
	getalpha (6, 2, "AAAAAAAAAAAAAA", username);
	while (last_char == SEARCH)
	{
		srch_users (username);
		getalpha (6, 2, "AAAAAAAAAAAAAA", username);
	}
	if (dflt_used && last_char == EOI)
		return (TRUE);
	load_tags (username);
	modify_tags ();
	return (FALSE);
}

int
heading (
 int                scn)
{
	clear ();

	rv_pr (ML(mlUtilsMess071), 56, 0, 1);
	move (0, 1);
	line (131);
    return (EXIT_SUCCESS);
}

void
read_prntype (void)
{
	char	line[129];
	char	*sptr,
		*tptr;
	FILE	*prntype;
	struct	PRNTYPE	tmp_prntype;

	sprintf (line, "%s/BIN/MENUSYS/prntype", (prog_path) ? prog_path : "/usr/LS10.5");
	if ((prntype = fopen (line, "r")) == NULL)
	{
		sprintf (err_str, "Error in %s During (FOPEN)", line);
		sys_err (err_str, errno, PNAME);
	}

	tptr = sptr = fgets (line, 128, prntype);
	while (sptr && !feof (prntype))
	{
		*(sptr + strlen (sptr) - 1) = (char) NULL;
		sptr = strchr (tptr, '\t');
		*(sptr) = (char) NULL;
		sptr++;
		strncpy (tmp_prntype.type, tptr, 15);
		tptr = sptr;
		sptr = strchr (tptr, '\t');
		*(sptr) = (char) NULL;
		sptr++;
		strncpy (tmp_prntype.queue, tptr, 15);
		strncpy (tmp_prntype.desc, sptr, 60);
		insert_prntype (&tmp_prntype);
		tptr = sptr = fgets (line, 128, prntype);
	}

	fclose (prntype);
}

/*==========================================
| Insert prntype line into memory array.   |
| NB: Sort the array 3 ways via link-list. |
==========================================*/
void
insert_prntype (
 struct PRNTYPE*    new_prntype)
{
	struct	PRNTYPE	*lcl_prntype;

	lcl_prntype = (struct PRNTYPE *) malloc (sizeof (struct PRNTYPE));
	if (lcl_prntype == PRN_NULL)
		sys_err ("Error in prntype During (MALLOC)", errno, PNAME);

	lcl_prntype->tag_mode = UNTAGGED;
	strcpy (lcl_prntype->type,	new_prntype->type);
	strcpy (lcl_prntype->queue,	new_prntype->queue);
	strcpy (lcl_prntype->desc,	new_prntype->desc);
	lcl_prntype->lpno_next = PRN_NULL;
	lcl_prntype->type_next = PRN_NULL;
	lcl_prntype->queue_next = PRN_NULL;

	/*---------------------------------------
	| If first time, set up as head for ALL |
	---------------------------------------*/
	if (head_lpno == PRN_NULL)
	{
		head_lpno = lcl_prntype;
		head_type = lcl_prntype;
		head_queue = lcl_prntype;
		return;
	}

	/*---------------------------------
	| Insert at tail end of lpno list |
	---------------------------------*/
	for (curr_lpno = head_lpno; curr_lpno->lpno_next != PRN_NULL; curr_lpno = curr_lpno->lpno_next);
	curr_lpno->lpno_next = lcl_prntype;

	/*------------------------------
	| Insert sorted into type list |
	------------------------------*/
	prev_type = PRN_NULL;
	for (curr_type = head_type; ; curr_type = curr_type->type_next)
	{
		if (strcmp (lcl_prntype->type, curr_type->type) >= 0)
		{
			if (curr_type->type_next == PRN_NULL)
			{
				curr_type->type_next = lcl_prntype;
				break;
			}
			prev_type = curr_type;
		}
		else
		{
			lcl_prntype->type_next = curr_type;
			if (prev_type == PRN_NULL)
				head_type = lcl_prntype;
			else
				prev_type->type_next = lcl_prntype;
			break;
		}
	}

	/*-------------------------------
	| Insert sorted into queue list |
	-------------------------------*/
	prev_queue = PRN_NULL;
	for (curr_queue = head_queue; ; curr_queue = curr_queue->queue_next)
	{
		if (strcmp (lcl_prntype->queue, curr_queue->queue) >= 0)
		{
			if (curr_queue->queue_next == PRN_NULL)
			{
				curr_queue->queue_next = lcl_prntype;
				break;
			}
			prev_queue = curr_queue;
		}
		else
		{
			lcl_prntype->queue_next = curr_queue;
			if (prev_queue == PRN_NULL)
				head_queue = lcl_prntype;
			else
				prev_queue->queue_next = lcl_prntype;
			break;
		}
	}

	return;
}

/*===========================================
| Clear current tags prior to changing user |
===========================================*/
void
clear_tags (void)
{
	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next)
		curr_lpno->tag_mode = UNTAGGED;
}

/*===========================
| Load System and User tags |
| for the specified user.   |
===========================*/
void
load_tags (
 char*              lcl_user)
{
	char	*uptr,				/* User name	*/
		*tptr,				/* Printer type	*/
		*qptr,				/* Queue name	*/
		record[129];

	clip (lcl_user);

	fseek (lp_secure, 0L, 0);
	while ((uptr = fgets (record, 128, lp_secure)) && !feof (lp_secure))
	{
		*(uptr + strlen (uptr) - 1) = (char) NULL;
		tptr = strchr (uptr, '\t');
		if (!tptr)
			continue;
		*tptr = (char) NULL;
		tptr++;
		qptr = strchr (tptr, '\t');
		if (!qptr)
			continue;
		*qptr = (char) NULL;
		qptr++;
		if (*uptr != '*' && strcmp (lcl_user, uptr))
			continue;
		_load_tags (lcl_user, uptr, tptr, qptr);
	}
}

/*========================
| Set an individual tag. |
========================*/
void
_load_tags (
 char*              lcl_user,
 char*              uptr,
 char*              tptr,
 char*              qptr)
{
	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next)
	{
		if (*tptr != '*' && strcmp (curr_lpno->type, tptr))
			continue;
		if (*qptr != '*' && strcmp (curr_lpno->queue, qptr))
			continue;
		if (*lcl_user == '*')
		{
			curr_lpno->tag_mode = GLB_TAG | USR_TAG;
			continue;
		}
		curr_lpno->tag_mode |= (*uptr == '*') ? GLB_TAG : USR_TAG;
	}
}

void
srch_users (
 char*              key_val)
{
	FILE	*us_secure;
	char	filename[129];
	char	*sptr;

	sprintf (filename, "%s/BIN/MENUSYS/User_Secure", (prog_path) ? prog_path : "/usr/LS10.5");
	if ((us_secure = fopen (filename, "r")) == NULL)
	{
		sprintf (err_str, "Error in %s During (FOPEN)", filename);
		sys_err (err_str, errno, PNAME);
	}

	work_open ();
	save_rec ("#Username", "#pp");

	sptr = fgets (filename, 128, us_secure);
	while (sptr && !feof (us_secure))
	{
		sptr = strchr (sptr, '<');
		if (sptr)
			*sptr = (char) NULL;
		sprintf (err_str, "%-8.8s", filename);
		cc = save_rec (err_str, " ");
		if (cc)
			break;
		sptr = fgets (filename, 128, us_secure);
	}

	cc = disp_srch();
	work_close();
	if (cc)
		strcpy (key_val, "");
	else
		sprintf (key_val, "%-8.8s", temp_str);
}

/*==========================
| Modify tags as specified |
==========================*/
void
modify_tags (void)
{
	tab_open (printers, keys, 3, 0, 15, FALSE);

	tab_add (printers, "# Sys Usr  Description                                                    Queue             Class           ");

	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next)
	{
		tab_add (printers, "  %-1.1s   %-1.1s   %-60.60s   %-15.15s   %-15.15s ",
			(curr_lpno->tag_mode & GLB_TAG) ? "*" : " ",
			(curr_lpno->tag_mode & USR_TAG) ? "+" : " ",
			curr_lpno->desc,
			curr_lpno->queue,
			curr_lpno->type);
	}

	tab_scan (printers);

	tab_close (printers, TRUE);

	update_tags ();
}

int
Tag_all (
 int                c,
 KEY_TAB*           psUnused)
{
	int	cur_line = 0;

	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next, cur_line++)
		set_tag (cur_line);

	tab_display (printers, TRUE);
	redraw_keys (printers);

	return (c);
}

int
untag_all (
 int                c,
 KEY_TAB*           psUnused)
{
	int	cur_line = 0;

	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next, cur_line++)
		reset_tag (cur_line);

	tab_display (printers, TRUE);
	redraw_keys (printers);

	return (c);
}

int
tag_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	cur_line,
		wrk_line;

	cur_line = tab_tline (printers);
	for (wrk_line = 0, curr_lpno = head_lpno; (wrk_line != cur_line) && (curr_lpno != PRN_NULL); curr_lpno = curr_lpno->lpno_next, wrk_line++);

	if (curr_lpno != PRN_NULL)
		set_tag (wrk_line);

	return (c);
}

int
untag_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	cur_line,
		wrk_line;

	cur_line = tab_tline (printers);
	for (wrk_line = 0, curr_lpno = head_lpno; (wrk_line != cur_line) && (curr_lpno != PRN_NULL); curr_lpno = curr_lpno->lpno_next, wrk_line++);

	if (curr_lpno != PRN_NULL)
		reset_tag (wrk_line);

	return (c);
}

int
queue_func (
 int                c,
 KEY_TAB*           psUnused)
{
	char	queue_name[16];
	int	cur_line = 0;

	/*print_at (23, 0, "Queue: ");*/
	print_at (23, 0, ML(mlUtilsMess069));
	getalpha (10, 23, "AAAAAAAAAAAAAAA", queue_name);
	if (last_char == RESTART)
	{
		print_at (23, 0, "                       ");
		return (c);
	}

	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next, cur_line++)
		if (!strcmp (curr_lpno->queue, queue_name))
			set_tag (cur_line);

	tab_display (printers, TRUE);
	redraw_keys (printers);

	return (c);
}

int
class_func (
 int                c,
 KEY_TAB*           psUnused)
{
	char	type_name[16];
	int	cur_line = 0;

	/*print_at (23, 0, "Class: ");*/
	print_at (23, 0, ML(mlUtilsMess070));
	getalpha (10, 23, "AAAAAAAAAAAAAAA", type_name);
	if (last_char == RESTART)
	{
		print_at (23, 0, "                       ");
		return (c);
	}

	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next, cur_line++)
		if (!strcmp (curr_lpno->type, type_name))
			set_tag (cur_line);

	tab_display (printers, TRUE);
	redraw_keys (printers);

	return (c);
}

void
set_tag (
 int                line)
{
	char	tmp_str[132];

	tab_get (printers, tmp_str, EQUAL, line);
	curr_lpno->tag_mode |= USR_TAG;
	if (username[0] == '*')
		curr_lpno->tag_mode |= GLB_TAG;
	tab_update (printers, "  %-1.1s   %-1.1s   %-60.60s   %-15.15s   %-15.15s ",
		(curr_lpno->tag_mode & GLB_TAG) ? "*" : " ",
		(curr_lpno->tag_mode & USR_TAG) ? "+" : " ",
		curr_lpno->desc,
		curr_lpno->queue,
		curr_lpno->type);
}

void
reset_tag (
 int                line)
{
	char	tmp_str[132];

	tab_get (printers, tmp_str, EQUAL, line);
	curr_lpno->tag_mode |= USR_TAG;
	curr_lpno->tag_mode -= USR_TAG;
	if (username[0] == '*')
	{
		curr_lpno->tag_mode |= GLB_TAG;
		curr_lpno->tag_mode -= GLB_TAG;
	}
	tab_update (printers, "  %-1.1s   %-1.1s   %-60.60s   %-15.15s   %-15.15s ",
		(curr_lpno->tag_mode & GLB_TAG) ? "*" : " ",
		(curr_lpno->tag_mode & USR_TAG) ? "+" : " ",
		curr_lpno->desc,
		curr_lpno->queue,
		curr_lpno->type);
}

void
update_tags (void)
{
	char	*uptr,				/* User name	*/
		*tptr,				/* Printer type	*/		
		record[129],
		filename[129];
	FILE	*new_secure;

	sprintf (filename, "%s/BIN/MENUSYS/NEW_SECURE", (prog_path) ? prog_path : "/usr/LS10.5");
	if ((new_secure = fopen (filename, "w")) == NULL)
	{
		sprintf (err_str, "Error in %s During (FOPEN)", filename);
		sys_err (err_str, errno, PNAME);
	}

	fseek (lp_secure, 0L, 0);
	while ((uptr = fgets (record, 128, lp_secure)) && !feof (lp_secure))
	{
		tptr = strchr (uptr, '\t');
		if (!tptr)
			fputs (uptr, new_secure);
		else
			if (strncmp (username, uptr, tptr - uptr))
				fputs (uptr, new_secure);
	}

	fclose (lp_secure);
	fclose (new_secure);

	sprintf (record, "%s/BIN/MENUSYS/LP_SECURE", (prog_path) ? prog_path : "/usr/LS10.5");
	unlink (record);
	link (filename, record);
	unlink (filename);

	lp_secure = fopen (record, "a");

	_queue_upd ();
	_type_upd ();
	_lpno_upd ();

	fclose (lp_secure);

	lp_secure = fopen (record, "r");
}

void
_queue_upd (void)
{
	char	wrk_queue[16];
	int	all_tagged = TRUE;

	if (head_queue == PRN_NULL)
		return;
	strcpy (wrk_queue, head_queue->queue);
	for (prev_queue = curr_queue = head_queue; curr_queue != PRN_NULL; curr_queue = curr_queue->queue_next)
	{
		if (strcmp (wrk_queue, curr_queue->queue) || curr_queue->queue_next == PRN_NULL)
		{
			if (all_tagged)
			{
				add_line ("*", wrk_queue);
				for (; !(strcmp (prev_queue->queue, wrk_queue)); prev_queue = prev_queue->queue_next)
					prev_queue->tag_mode |= WRITTEN;
			}
			prev_queue = curr_queue;
			all_tagged = TRUE;
			strcpy (wrk_queue, curr_queue->queue);
		}
		if (!(curr_queue->tag_mode & USR_TAG))
			all_tagged = FALSE;
	}
}

void
_type_upd(void)
{
	char	wrk_type[16];
	int	all_tagged = TRUE;
	int	all_written = TRUE;

	if (head_type == PRN_NULL)
		return;
	strcpy (wrk_type, head_type->type);
	for (prev_type = curr_type = head_type; curr_type != PRN_NULL; curr_type = curr_type->type_next)
	{
		if (strcmp (wrk_type, curr_type->type) || curr_type->type_next == PRN_NULL)
		{
			if (all_tagged)
			{
				if (!all_written)
				{
					add_line (wrk_type, "*");
					for (; !(strcmp (prev_type->type, wrk_type)); prev_type = prev_type->type_next)
						prev_type->tag_mode |= WRITTEN;
				}
			}
			prev_type = curr_type;
			all_tagged = TRUE;
			all_written = TRUE;
			strcpy (wrk_type, curr_type->type);
		}
		if (!(curr_type->tag_mode & USR_TAG))
			all_tagged = FALSE;
		if (!(curr_type->tag_mode & WRITTEN))
			all_written = FALSE;
	}
}

void
_lpno_upd(void)
{
	for (curr_lpno = head_lpno; curr_lpno != PRN_NULL; curr_lpno = curr_lpno->lpno_next)
	{
		if ((curr_lpno->tag_mode & USR_TAG) && !(curr_lpno->tag_mode & WRITTEN))
			add_line (curr_lpno->type, curr_lpno->queue);
	}
}

void
add_line (
 char*              type,
 char*              queue)
{
	fprintf (lp_secure, "%s\t%s\t%s\n",
		username,
		type,
		queue);
}

