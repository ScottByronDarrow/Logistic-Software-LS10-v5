/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_wip_mnt.c,v 5.3 2001/10/09 22:34:27 scott Exp $
|  Program Name  : (cm_wip_mnt.c) 
|  Program Desc  : (Contract Management WIP Status Maint)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 02/03/93         |
|---------------------------------------------------------------------|
| $Log: cm_wip_mnt.c,v $
| Revision 5.3  2001/10/09 22:34:27  scott
| Updated from Scotts machine.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_wip_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_wip_mnt/cm_wip_mnt.c,v 5.3 2001/10/09 22:34:27 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cmwsRecord	cmws_rec;

	char	*data   = "data";

	int	not_exists;

	extern	int	TruePosition;

struct
{
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "wip",	 4, 2, CHARTYPE,
		"UUUU", "          ",
		" ", "", "WIP Code          ", "Enter WIP Code Number For Maintenance, Full Search Available. ",
		 NE, NO,  JUSTLEFT, "", "", cmws_rec.wp_stat},
	{1, LIN, "wip_desc",	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "WIP Description   ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmws_rec.desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB			 (void);
int		spec_valid		 (int);
void	SrchCmws		 (char *);
int		heading			 (int);
void	Update			 (void);
void	shutdown_prog	 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();

	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
		init_vars (1);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cmws, cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cmws);	
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("wip"))
	{
		if (SRCH_KEY)
		{
			SrchCmws (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws_rec.co_no, comm_rec.co_no);

		not_exists = find_rec (cmws, &cmws_rec, EQUAL, "r");
		
		if (!not_exists)
		{
			scn_display (1);
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchCmws (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Code", "#WIP Code Description ");

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", key_val);
	cc = find_rec (cmws, &cmws_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmws_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmws_rec.wp_stat, key_val, strlen (key_val)))
	{
		cc = save_rec (cmws_rec.wp_stat, cmws_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmws, &cmws_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmws_rec.co_no, comm_rec.co_no);
	sprintf (cmws_rec.wp_stat, "%-4.4s", temp_str);
	cc = find_rec (cmws, &cmws_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmws, "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlCmMess147), 24, 0, 1);

	box (0, 3, 80, 2);
	line_at (21,0,80);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 1,err_str,comm_rec.co_no, comm_rec.co_name);
	line_at (1,0,80);
	
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
Update (
 void)
{
	if (not_exists)
		cc = abc_add (cmws, &cmws_rec);
	else
		cc = abc_update (cmws,&cmws_rec);

	if (cc)
		file_err (cc, cmws, (not_exists) ? "DBADD" : "DBUPDATE");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}
