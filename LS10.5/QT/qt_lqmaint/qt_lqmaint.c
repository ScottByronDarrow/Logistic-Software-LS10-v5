/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( qt_lqmaint.c   )                                 |
|  Program Desc  : ( Add / Maintain Lost Quote Records.           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, exlq, qthr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  exlq,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Elena Cuaresma  | Date Written  : 01/10/1995       |
|---------------------------------------------------------------------|
|  Date Modified : (28/12/1995)    | Modified  by  : Elena B. Cuaresma|
|  Date Modified : (05/09/1997)    | Modified  by  : Ana Marie Tario  |
|  Date Modified : (29/10/1997)    | Modified  by  : Elena B. Cuaresma|
|  Date Modified : (23/08/1999)    | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      : (28/12/1995) - HOSKYNS 011 Updated to rebuild the  |
|                :              structure of qthr.                    |
|  				 : (05/09/1997) - Incorporated multilingual conversion|
|                               - and DMY4 date. 					  |
|  				 : (29/10/1997) - Changed the quote number length from|
|                               6 to 8.        					      |
|                               - Code checked and tested the program.|
|  				 : (20/08/1999) - Converted to ANSI format.           |
|                                              					      |
| $Log: qt_lqmaint.c,v $
| Revision 5.2  2001/08/09 08:44:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:29  gerry
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
| Revision 1.8  1999/09/13 09:23:26  alvin
| Converted to ANSI format.
|
| Revision 1.7  1999/06/18 06:12:26  scott
| Updated to add log for cvs and remove old style read_comm()
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qt_lqmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_lqmaint/qt_lqmaint.c,v 5.2 2001/08/09 08:44:42 scott Exp $";

#include <std_decs.h>
#include <ml_qt_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <minimenu.h>

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
	struct dbview exlq_list[] ={
		{"exlq_co_no"},
		{"exlq_code"},
		{"exlq_description"}
	};

	int exlq_no_fields = 3;

	struct {
		char	lq_co_no[3];
		char	lq_code[4];
		char	lq_description[31];
	} exlq_rec;

	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	5

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_reas_code"},
		{"qthr_reas_desc"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		char	reas_code [4];
		char	reas_desc [31];
	}	qthr_rec;

	char	*data = "data",
			*comm = "comm",
			*exlq = "exlq",
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
	{1, LIN, "lq_code",	 4, 15, CHARTYPE,
		"UUU", "          ",
		" ", "", "Code        :", "Lost Quote Code ",
		YES, NO,  JUSTLEFT, "", "", exlq_rec.lq_code},
	{1, LIN, "description",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description :", " ",
		YES, NO,  JUSTLEFT, "", "", exlq_rec.lq_description},

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
int		heading 		(int);
void	OpenDB			(void);
void	CloseDB			(void);
int		update			(void);
int		ExlqDelOk		(void);
void	save_page		(char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
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
shutdown_prog  (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB(void)
{
	abc_dbopen(data);

	open_rec(exlq, exlq_list, exlq_no_fields, "exlq_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (exlq);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	if (strcmp(FIELD.label,"lq_code") == 0) 
	{
		if (last_char == SEARCH)
		{
			save_page(temp_str);
			return(0);
		}
		strcpy(exlq_rec.lq_co_no,comm_rec.tco_no);
		cc = find_rec(exlq, &exlq_rec, COMPARISON, "u");
		if (!cc)
		{
			new_item = FALSE;
			entry_exit = 1;
			DSP_FLD ("lq_code");
			DSP_FLD ("description");
		}
		else
		{
			new_item = TRUE;
			DSP_FLD ("lq_code");
			DSP_FLD ("description");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
update (void)
{
	int		exitLoop;

	if (new_item)
	{
		cc = abc_add (exlq, &exlq_rec);
		if (cc)
			file_err (cc, exlq, "DBADD");
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
				cc = abc_update (exlq, &exlq_rec);
				if (cc) 
					file_err (cc, exlq, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (exlq);
				exitLoop = TRUE;
				break;
	
			case SEL_DELETE :
				if (ExlqDelOk ())
				{
					clear_mess();
					cc = abc_delete (exlq);
					if (cc)
						file_err (cc, exlq, "DBUPDATE");
				}
				else
				{
					sprintf (err_str,ML(mlQtMess001),badFileName);
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

	return(cc);
}

/*===========================
| Check whether it is OK to |
| delete the exsf record.   |
| Files checked are :       |
| inls                      |
===========================*/
int
ExlqDelOk (void)
{
	/*-------------
	| Check inls. |
	-------------*/
	print_mess(ML(mlStdMess035));
	open_rec (qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	strcpy (qthr_rec.co_no,  comm_rec.tco_no);
	strcpy (qthr_rec.br_no, "  ");
	strcpy (qthr_rec.quote_no, "        ");
	cc = find_rec (qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp (qthr_rec.co_no, comm_rec.tco_no))
	{
		if (!strcmp (qthr_rec.reas_code, exlq_rec.lq_code))
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
	save_rec("#Lq","#Description");

	strcpy (exlq_rec.lq_co_no, comm_rec.tco_no);
	sprintf (exlq_rec.lq_code, "%-3.3s", key_val);
	cc = find_rec (exlq, &exlq_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (exlq_rec.lq_code, key_val, strlen (key_val)) && 
		   !strcmp (exlq_rec.lq_co_no, comm_rec.tco_no))
	{
		cc = save_rec (exlq_rec.lq_code, exlq_rec.lq_description);
		if (cc)
			break;

		cc = find_rec (exlq, &exlq_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exlq_rec.lq_co_no, comm_rec.tco_no);
	sprintf (exlq_rec.lq_code, "%-3.3s", temp_str);
	cc = find_rec (exlq, &exlq_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exlq, "DBFIND");
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
		rv_pr(ML(mlQtMess018),28,0,1);
		move(0,1);
		line(80);

		box(0,3,80,2);

		move(0,20);
		line(80);
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0, err_str,comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
