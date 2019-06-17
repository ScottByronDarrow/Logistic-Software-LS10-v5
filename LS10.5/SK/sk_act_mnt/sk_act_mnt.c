/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_act_mnt.c,v 5.6 2002/07/25 11:17:32 scott Exp $
|  Program Name  : (sk_act_mnt.c)                      
|  Program Desc  : (Inventory Active Status Maintenance)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (17/08/93)       |
|---------------------------------------------------------------------|
| $Log: sk_act_mnt.c,v $
| Revision 5.6  2002/07/25 11:17:32  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2002/04/11 03:46:11  scott
| Updated to add comments to audit files.
|
| Revision 5.4  2001/10/05 03:00:25  cha
| Added code to produce audit files.
|
| Revision 5.3  2001/08/09 09:17:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:33  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:42  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_act_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_act_mnt/sk_act_mnt.c,v 5.6 2002/07/25 11:17:32 scott Exp $";

#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <pslscr.h>
#include <DBAudit.h>

#include	"schema"

struct commRecord	comm_rec;
struct inasRecord	inas_rec;

	char	*data = "data";
	extern	int		TruePosition;

   	int	new_code = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	previousCode [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "act_code",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "", "Active Status   ", " Enter Active Status code.",
		 NE, NO, JUSTLEFT, "", "", inas_rec.act_code},
	{1, LIN, "act_desc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description     ", " Enter description for Active Status code ",
		YES, NO,  JUSTLEFT, "", "", inas_rec.description},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int  	spec_valid 		 (int);
void 	Update 			 (void);
void 	SrchInas 		 (char *);
int  	heading 		 (int);


/*==========================
| Main processing routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = FALSE; 
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (!restart)
			Update ();

	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inas, inas_list, INAS_NO_FIELDS, "inas_id_no");
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("ItemActiveStatus.txt");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inas);
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK ("act_code"))
	{
		if (SRCH_KEY)
		{
			SrchInas (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inas_rec.co_no, comm_rec.co_no);
		cc = find_rec (inas, &inas_rec, COMPARISON, "w");
		if (cc) 
			new_code = 1;
		else    
		{
			new_code = 0;
			entry_exit = 1;
			/*
			 * Save old record.
			 */
			SetAuditOldRec (&inas_rec, sizeof (inas_rec));
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*==================
| Updated records. |
==================*/
void
Update (
 void)
{
	strcpy (inas_rec.co_no, comm_rec.co_no);
	if (new_code)
	{
		cc = abc_add (inas, &inas_rec);
		if (cc) 
			file_err (cc, inas, "DBADD");
	}
	else
	{
		/*
		 * Update changes audit record.
		 */

		 sprintf (err_str, "%s : %s (%s)", ML ("Active Status"), inas_rec.act_code, inas_rec.description);
		 AuditFileAdd (err_str, &inas_rec, inas_list, INAS_NO_FIELDS);
		cc = abc_update (inas, &inas_rec);
		if (cc) 
			file_err (cc, inas, "DBUPDATE");
	}

	abc_unlock (inas);
	strcpy (local_rec.previousCode, inas_rec.act_code);
}

void
SrchInas (
 char *key_val)
{
	_work_open (1,0,40);
	save_rec ("##", "#Active status code description");
	strcpy (inas_rec.co_no, comm_rec.co_no);
	sprintf (inas_rec.act_code, "%-1.1s", key_val);

	cc = find_rec (inas, &inas_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (inas_rec.co_no, comm_rec.co_no) &&
		   !strncmp (inas_rec.act_code, key_val, strlen (key_val)))
	{
		cc = save_rec (inas_rec.act_code, inas_rec.description);
		if (cc)
			break;

		cc = find_rec (inas, &inas_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (inas_rec.description, "%-40.40s", " ");
		return;
	}

	strcpy (inas_rec.co_no, comm_rec.co_no);
	sprintf (inas_rec.act_code, "%-1.1s", temp_str);
	cc = find_rec (inas, &inas_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inas, "DBFIND");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		strcpy (err_str, ML (mlSkMess319));
		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

		print_at (0,65, ML (mlSkMess084), local_rec.previousCode);

		line_at (1,0,80);

		box (0, 3, 80, 2);

		line_at (20,0,80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);

		line_at (22,0,80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


