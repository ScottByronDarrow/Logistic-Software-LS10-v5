/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_phone.c    )                                 |
|  Program Desc  : ( Logistic Software Phone Diary.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  ppmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  ppmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (mail)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 16/07/91         |
|---------------------------------------------------------------------|
|  Date Modified : (16/07/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (17/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (03/09/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      :                                                    |
|  (17/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: psl_phone.c,v $
| Revision 5.2  2001/08/09 05:13:48  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:38  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:54  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:11  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:31  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:47:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/16 09:42:02  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.7  1999/09/29 10:11:18  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:13  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:42  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:55  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_phone.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_phone/psl_phone.c,v 5.2 2001/08/09 05:13:48 scott Exp $";


#define	TXT_REQD
#define	TABLINES	4
#define	MAXLINES	4

#include <pslscr.h>
#include <minimenu.h>
#include <get_lpno.h>
#include <ml_menu_mess.h>

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2

extern	int	X_EALL;
extern	int	Y_EALL;
extern	int	SR_X_POS;
extern	int	SR_Y_POS;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int new_phone = 0;
	int	psl_phone = FALSE;
	int	psl_pdisp = FALSE;
	
	int	clear_ok = TRUE;

	/*=============================
	| Logistic Phone Master File. |
	=============================*/
	struct dbview ppmr_list [] = {
		{"ppmr_log_name"},
		{"ppmr_u_name"},
		{"ppmr_u_ph_1"},
		{"ppmr_u_ph_2"},
		{"ppmr_u_fax"},
		{"ppmr_u_other"},
		{"ppmr_u_adr1"},
		{"ppmr_u_adr2"},
		{"ppmr_u_adr3"},
		{"ppmr_u_adr4"},
		{"ppmr_stat_flag"}
	};

	int ppmr_no_fields = 11;

	struct {
		char	pm_log_name [15];
		char	pm_u_name [21];
		char	pm_u_ph_1 [16];
		char	pm_u_ph_2 [16];
		char	pm_u_fax [16];
		char	pm_u_other [16];
		char	pm_u_adr [4][41];
		char	pm_stat_flag [2];
	} ppmr_rec;

	char	*scn_desc [] = {
		"Phone Header Screen.",
		"Phone Address Screen."
	};

	char	*curr_user;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char	data_str[41];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "u_name", 7, 36, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Name", "", 
		NE, NO, JUSTLEFT, "", "", ppmr_rec.pm_u_name}, 
	{1, LIN, "u_phone1", 8, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Main Phone Number.", "", 
		YES, NO, JUSTLEFT, "", "", ppmr_rec.pm_u_ph_1}, 
	{1, LIN, "u_phone2", 9, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Other Phone No.", "", 
		NO, NO, JUSTLEFT, "", "", ppmr_rec.pm_u_ph_2}, 
	{1, LIN, "fax_no", 10, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Fax/Telex No.", "", 
		NO, NO, JUSTLEFT, "", "", ppmr_rec.pm_u_fax}, 
	{1, LIN, "misc_phone", 11, 36, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Misc Phone No.", "", 
		NO, NO, JUSTLEFT, "", "", ppmr_rec.pm_u_other}, 
	{2, TXT, "", 13, 19, 0, 
		"", "          ", 
		"", "", "(Address)", "", 
		4, 40, 4, "", "", local_rec.data_str}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	load_ptxt		(void);
void	update_menu		(void);
void	update			(void);
void	show_ppmr		(char *);
void	cl_bx			(int, int, int, int);
void	display_phone	(void);
int		heading			(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	i;

	char *	sptr;

	sptr = strrchr (argv[0], '/');
	if (!sptr)
		sptr = argv[0];
	else
		sptr++;

	if ( !strcmp( sptr, "psl_phone") )
		psl_phone = TRUE;

	if ( !strcmp( sptr, "psl_pdisp") )
		psl_pdisp = TRUE;

	curr_user = getenv( "LOGNAME" );

	if ( psl_pdisp )
	{
		init_scr ();
		swide ();
		set_tty ();
		OpenDB ();

		display_phone ();
		
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	Y_EALL = 6;
	X_EALL = 11;
	SR_X_POS = 16;
	SR_Y_POS = 3;
	MENU_ROW = 6;
	MENU_COL = 11;
	init_scr();
	set_tty();
	set_masks();

	for (i = 0;i < 2;i++)
		tab_data[i]._desc = scn_desc[i];

	OpenDB(); 	

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (prog_exit == 0)	
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		new_phone = 0;
		search_ok = 1;
		init_vars(1);
		init_vars(2);
		clear_ok = TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if ( new_phone )
		{
			heading (2);
			entry (2);
			if (restart)
				continue;

		}
		else
			scn_display( 2 );
	
		clear_ok = FALSE;

		edit_all();
		if ( restart )
			continue;
				
		/*-----------------------
		| Update debtor record. |
		-----------------------*/
		if ( !restart )
			update();
	}
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
	abc_dbopen ("data");
	open_rec ("ppmr", ppmr_list, ppmr_no_fields, "ppmr_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("ppmr");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	/*-------------------------
	| Validate Prompt Number. |
	-------------------------*/
	if (LCHECK("u_name"))
	{
		if (last_char == SEARCH)
		{
			show_ppmr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		sprintf( ppmr_rec.pm_log_name, "%-14.14s", curr_user);
		new_phone = find_rec("ppmr", &ppmr_rec, COMPARISON, "w");
		if ( new_phone )
		{
			abc_unlock( "ppmr" );
			strcpy( ppmr_rec.pm_log_name, "*");
			new_phone = find_rec("ppmr",&ppmr_rec,COMPARISON, "w");
		}
		if ( !new_phone )
		{
			load_ptxt();
			entry_exit = TRUE;
		}
		else
			sprintf( ppmr_rec.pm_log_name, "%-14.14s", curr_user);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*=============================
| Load text and prompt lines. |
=============================*/
void
load_ptxt (
 void)
{
	int	i;


	lcount [2] = 0;

	scn_set( 2 );

	for ( i = 0; i < 4; i++ )
	{
		strcpy( local_rec.data_str, ppmr_rec.pm_u_adr[ i ] );
		putval( lcount[ 2 ]++ );
	}
	scn_set( 1 );
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD  ",
		  "" },
		{ " 2. IGNORE CHANGES ",
		  "" },
		{ " 3. DELETE RECORD  ",
		  "" },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
update_menu (
 void)
{
	for (;;)
	{
	    mmenu_print (" UPDATE SELECTION. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case SEL_UPDATE :
		case 99 :
			cc = abc_update( "ppmr", &ppmr_rec );
			if ( cc )
				file_err( cc, "ppmr", "DBUPDATE" );
			return;

		case SEL_IGNORE :
			abc_unlock("ppmr");
			return;

		case SEL_DELETE :
			cc = abc_delete( "ppmr" );
			if ( cc )
				file_err( cc, "ppmr", "DBDELETE" );
			return;
			break;
	
		default :
			break;
	    }
	}
}

/*----------------
| Update Record. |
----------------*/
void
update (
 void)
{
	int	i;

	strcpy (ppmr_rec.pm_stat_flag, "0");

	scn_set( 2 );

	for ( i = 0; i < 4; i++ )
	{
		if ( i < lcount[ 2 ] )
		{
			getval( i );
			sprintf( ppmr_rec.pm_u_adr[ i ], "%-40.40s",
						local_rec.data_str );
		}
		else
			sprintf( ppmr_rec.pm_u_adr[ i ],"%-40.40s"," ");
	}
	
	scn_set( 1 );

	if ( new_phone ) 
	{
		cc = abc_add( "ppmr", &ppmr_rec );
		if ( cc )
			file_err( cc, "ppmr", "DBADD" );
	}
	else
		update_menu();

	return;
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
show_ppmr (
 char *	key_val)
{
	work_open();
	sprintf( ppmr_rec.pm_log_name, "%-14.14s", curr_user);
	sprintf( ppmr_rec.pm_u_name,   "%-20.20s", key_val );
	save_rec("#        Name.       ","#Phone Number.");
	cc = find_rec("ppmr", &ppmr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp(ppmr_rec.pm_log_name, curr_user,strlen(curr_user)) && 
		!strncmp(ppmr_rec.pm_u_name, key_val, strlen(key_val)))
	{
		cc = save_rec(ppmr_rec.pm_u_name, ppmr_rec.pm_u_ph_1 );
		if (cc)
			break;

		cc = find_rec("ppmr", &ppmr_rec, NEXT, "r");
	}
	strcpy( ppmr_rec.pm_log_name, "*");
	sprintf( ppmr_rec.pm_u_name,   "%-20.20s", key_val );
	cc = find_rec("ppmr", &ppmr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp(ppmr_rec.pm_u_name, key_val, strlen(key_val)) &&
		ppmr_rec.pm_log_name[0] == '*' )
	{
		cc = save_rec(ppmr_rec.pm_u_name, ppmr_rec.pm_u_ph_1 );
		if (cc)
			break;

		cc = find_rec("ppmr", &ppmr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf( ppmr_rec.pm_log_name, "%-14.14s", curr_user);
	sprintf( ppmr_rec.pm_u_name,   "%-20.20s", temp_str );
	cc = find_rec("ppmr", &ppmr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy( ppmr_rec.pm_log_name, "*");
		sprintf( ppmr_rec.pm_u_name,   "%-20.20s", temp_str );
		cc = find_rec("ppmr", &ppmr_rec, COMPARISON, "r");
	}
	if ( cc )
		file_err( cc, "ppmr", "DBFIND" );
}

/*=============================================================
| Clear screen where box to be drawn + if _box then draw box. |
=============================================================*/
void
cl_bx (
 int x,
 int y,
 int h,
 int v)
{
	int	i,
		j;

	j = v;		
	i = y;
		
	while ( j-- )
	{
		if ( h > 1)
			print_at( i, x, "%*.*s", h, h, " ");
		i++;
	}
}

void
display_phone (
 void)
{
	char	disp_str [300],
			disp_str2 [300];

	strcpy(err_str, " P I N N A C L E   P H O N E   D I A R Y ");

	Dsp_prn_open( 0, 0, 16, err_str, (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0, 
			                 (char *) 0, (char *) 0 );

	Dsp_saverec( "                        P I N N A C L E       P H O N E       D I A R Y                       ");
	Dsp_saverec( "         NAME         |    PHONE NO 1.  |   PHONE NO 2.   |       FAX NO    |   OTHER NO.     ");
	Dsp_saverec("[FN5-Print]   [FN14-Next]   [FN15-Prev]    [FN16-Input/End]");


	sprintf( ppmr_rec.pm_log_name, "%-14.14s", curr_user);
	sprintf( ppmr_rec.pm_u_name, "%20.20s", " ");
	cc = find_rec("ppmr", &ppmr_rec, GTEQ, "r");
	while ( !cc && 
		!strncmp( ppmr_rec.pm_log_name, curr_user,strlen(curr_user)))
	{
		sprintf( disp_str, " %-20.20s ^E %-15.15s ^E %-15.15s ^E %-15.15s ^E %-15.15s ",

			ppmr_rec.pm_u_name,
			ppmr_rec.pm_u_ph_1,
			ppmr_rec.pm_u_ph_2,
			ppmr_rec.pm_u_fax,
			ppmr_rec.pm_u_other );

		Dsp_saverec( disp_str );

		sprintf( disp_str2, "%s,%s,%s,%s",clip( ppmr_rec.pm_u_adr[0] ), 
						  clip( ppmr_rec.pm_u_adr[1] ),
						  clip( ppmr_rec.pm_u_adr[2] ), 
						  clip( ppmr_rec.pm_u_adr[3] ));

		if ( strlen( disp_str2 ) > 3 )
		{
			sprintf(disp_str," %20.20s ^E %-70.70s"," ",disp_str2);

			Dsp_saverec( disp_str );
		}
		Dsp_saverec( "^^GGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGG");

		cc = find_rec("ppmr", &ppmr_rec, NEXT, "r");
	}
	sprintf( ppmr_rec.pm_log_name, "%-14.14s", "*");
	sprintf( ppmr_rec.pm_u_name, "%20.20s", " ");
	cc = find_rec("ppmr", &ppmr_rec, GTEQ, "r");
	while ( !cc && ppmr_rec.pm_log_name[0] == '*' )
	{
		sprintf( disp_str, " %-20.20s ^E %-15.15s ^E %-15.15s ^E %-15.15s ^E %-15.15s ",
			ppmr_rec.pm_u_name,
			ppmr_rec.pm_u_ph_1,
			ppmr_rec.pm_u_ph_2,
			ppmr_rec.pm_u_fax,
			ppmr_rec.pm_u_other );

		Dsp_saverec( disp_str );

		sprintf( disp_str2, "%s,%s,%s,%s",clip( ppmr_rec.pm_u_adr[0] ), 
						  clip( ppmr_rec.pm_u_adr[1] ),
						  clip( ppmr_rec.pm_u_adr[2] ), 
						  clip( ppmr_rec.pm_u_adr[3] ));
		
		if ( strlen( disp_str2 ) > 3 )
		{
			sprintf(disp_str," %20.20s ^E %-70.70s"," ",disp_str2);

			Dsp_saverec( disp_str );
		}

		Dsp_saverec( "^^GGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGG");
		cc = find_rec("ppmr", &ppmr_rec, NEXT, "r");
	}
	Dsp_srch();
	Dsp_close();
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);

	if (scn == 1)
	{
		if (clear_ok)
			cl_box (9, 3, 62, 16);

		cl_bx (X_EALL, Y_EALL, 48, 6);
		cl_box (16, 6, 48, 5);
		us_pr (ML(mlMenuMess169), 30, 4, 1);
		move (10,5);
		line (61);
		scn_set (2);
		scn_write (2);
		scn_display (2);
	}
	if ( scn == 2 )
	{
		cl_bx( X_EALL, Y_EALL, 48, 6 );
		cl_box( 16,6,48,5 );
		scn_set( 1 );
		scn_write( 1 );
		scn_display( 1 );
	}
	scn_set( scn );

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
