/*======================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .             |
|======================================================================|
|  Program Name  : ( qt_lcmaint.c   )                                  |
|  Program Desc  : ( Add / Maintain Competitor Records.           )    |
|                  (                                              )    |
|----------------------------------------------------------------------|
|  Access files  :  comm, exlc, qthr,     ,     ,     ,     ,          |
|  Database      : (data)                                              |
|----------------------------------------------------------------------|
|  Updates Files :  exlq,     ,     ,     ,     ,     ,     ,          |
|  Database      : (data)                                              |
|----------------------------------------------------------------------|
|  Author        : Elena Cuaresma  | Date Written  : 01/10/1995        |
|----------------------------------------------------------------------|
|  Date Modified : (28/12/1995)    | Modified  by  : Elena B. Cuaresma |
|  Date Modified : (09/09/1997)    | Modified  by  : Marnie I. Organo  |
|  Date Modified : (29/10/1997)    | Modified  by  : Elena B. Cuaresma |
|  Date Modified : (20/08/1999)    | Modified  by  : Alvin Misalucha   |
|                                                                      |
|  Comments      : (28/12/1995) - Rebuild the structure of qthr_rec.   |
|                : (09/09/1997) - Modified for Multilingual Conversion.|
|                : (29/10/1997) - Changed the quote number length from |
|                :              6 to 8.                                |
|                :            - Code checked and tested the program.   |
|                : (20/08/1999) - Converted to ANSI format.            |
|                                                                      |
| $Log: qt_lcmaint.c,v $
| Revision 5.2  2001/08/09 08:44:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:08:57  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/16 03:29:19  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.10  1999/09/29 10:12:31  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/22 05:18:26  scott
| Updated from Ansi project.
|
| Revision 1.8  1999/09/13 09:22:24  alvin
| Converted to ANSI format.
|
| Revision 1.7  1999/06/18 06:12:26  scott
| Updated to add log for cvs and remove old style read_comm()
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qt_lcmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_lcmaint/qt_lcmaint.c,v 5.2 2001/08/09 08:44:42 scott Exp $";

#include <std_decs.h>
#include <pslscr.h>
#include <minimenu.h>
#include <ml_std_mess.h>
#include <ml_qt_mess.h>

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	SEL_DEFAULT	99

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	new_item = 0;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int comm_no_fields = 3;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*==========================
	| External Lost Sale File. |
	==========================*/
	struct dbview exlc_list[] ={
		{"exlc_co_no"},
		{"exlc_code"},
		{"exlc_name"}
	};

	int exlc_no_fields = 3;

	struct {
		char	lc_co_no[3];
		char	lc_code[4];
		char	lc_name[31];
	} exlc_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	5

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_comp_code"},
		{"qthr_comp_name"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		char	comp_code [4];
		char	comp_name [31];
	}	qthr_rec;


	char	*data = "data",
			*comm = "comm",
			*exlc = "exlc",
			*qthr = "qthr";

	char	badFileName[5];

MENUTAB upd_menu [] =
	{
		{ " 1. SAVE   CHANGES JUST MADE TO RECORD.",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
		  "" },
		{ " 3. DELETE RECORD.                     ",
		  "" },
		{ ENDMENU }
	};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "lc_code",	 4, 20, CHARTYPE,
		"UUU", "          ",
		" ", "", "Competitor Code :", "Competitor Code ",
		YES, NO,  JUSTLEFT, "", "", exlc_rec.lc_code},
	{1, LIN, "lc_name",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Competitor Name :", " ",
		YES, NO,  JUSTLEFT, "", "", exlc_rec.lc_name},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Main Processing Routine . |
===========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
int		update			(void);
int		ExlcDelOk		(void);
void	save_page		(char *);
int		heading			(int);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	/*---------------------------
	| Stup required parameters. |
	---------------------------*/
	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		if (update())
			prog_exit = 1;

	}	/* end of input control loop	*/
	shutdown_prog ();

	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(exlc, exlc_list, exlc_no_fields, "exlc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (exlc);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	if (strcmp(FIELD.label,"lc_code") == 0) 
	{
		if (last_char == SEARCH)
		{
			save_page(temp_str);
			return(0);
		}
		strcpy(exlc_rec.lc_co_no,comm_rec.tco_no);
		cc = find_rec(exlc, &exlc_rec, COMPARISON, "u");
		if (!cc)
		{
			new_item = FALSE;
			entry_exit = 1;
			DSP_FLD ("lc_code");
			DSP_FLD ("lc_name");
		}
		else
		{
			new_item = TRUE;
			DSP_FLD ("lc_code");
			DSP_FLD ("lc_name");
		}
		return(0);
	}
	return(0);
}

int
update (void)
{
	int		exitLoop;

	if (new_item)
	{
		cc = abc_add (exlc, &exlc_rec);
		if (cc)
			file_err (cc, exlc, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case SEL_DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (exlc, &exlc_rec);
				if (cc) 
					file_err (cc, exlc, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (exlc);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExlcDelOk ())
				{
					clear_mess();
					cc = abc_delete (exlc);
					if (cc)
						file_err (cc, exlc, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,ML(mlQtMess001), badFileName);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}

	return (cc);
}

/*===========================
| Check whether it is OK to |
| delete the exsf record.   |
| Files checked are :       |
| inls                      |
===========================*/
int
ExlcDelOk (void)
{
	/*-------------
	| Check inls. |
	-------------*/
	print_mess(ML(mlStdMess014));
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	strcpy (qthr_rec.co_no,  comm_rec.tco_no);
	strcpy (qthr_rec.br_no, "  ");
	strcpy (qthr_rec.quote_no, "        ");
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp (qthr_rec.co_no, comm_rec.tco_no))
	{
		if (!strcmp (qthr_rec.comp_code, exlc_rec.lc_code))
		{
			abc_fclose (qthr);
			strcpy (badFileName, qthr);
			return (FALSE);
		}
		cc = find_rec (qthr, &qthr_rec, NEXT, "r");
	}
	abc_fclose (qthr);

	return (TRUE);
}

void
save_page (
 char *	key_val)
{
	work_open();
	save_rec("#Cd","#Competitor's Name");

	strcpy (exlc_rec.lc_co_no, comm_rec.tco_no);
	sprintf (exlc_rec.lc_code, "%-3.3s", key_val);
	cc = find_rec (exlc, &exlc_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (exlc_rec.lc_code, key_val, strlen (key_val)) && 
		   !strcmp (exlc_rec.lc_co_no, comm_rec.tco_no))
	{
		cc = save_rec (exlc_rec.lc_code, exlc_rec.lc_name);
		if (cc)
			break;

		cc = find_rec (exlc, &exlc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exlc_rec.lc_co_no, comm_rec.tco_no);
	sprintf (exlc_rec.lc_code, "%-3.3s", temp_str);
	cc = find_rec (exlc, &exlc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exlc, "DBFIND");
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlQtMess002),22,0,1);
		move(0,1);
		line(80);

		box(0,3,80,2);

		move(0,20);
		line(80);
		print_at(21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}

	return (EXIT_SUCCESS);
}
