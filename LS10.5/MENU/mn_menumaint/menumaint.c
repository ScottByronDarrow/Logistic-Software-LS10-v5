/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mn_menumaint.c )                                 |
|  Program Desc  : ( Menu System Maintenance.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mnhr, mnac, mnln, mncd,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  mnhr, mnln, mncd,     ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 12/12/90         |
|---------------------------------------------------------------------|
|  Date Modified : 12/12/90        | Modified  by  : Campbell Mander  |
|  Date Modified : 24/05/91        | Modified  by  : Campbell Mander  |
|  Date Modified : 16/10/91        | Modified  by  : Campbell Mander  |
|  Date Modified : 10/04/94        | Modified  by  : Campbell Mander  |
|  Date Modified : 04/09/97        | Modified  by  : Jiggs A Veloz.   |
|                                                                     |
|  Comments      : Allows creation and editing of screen painting     |
|                : lines. Changed to use tab_disp for editing of menu |
|                : lines and screen painting lines.                   |
|  (24/05/91)    : Security codes may now be up to 8 characters long. |
|  (16/10/91)    : Fixed copy function.                               |
|  (10/04/94)    : PSL 10673 - Online conversion                      |
|  (04/09/97)    : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                                      |
|                :                                                    |
|                :                                                    |
| $Log: menumaint.c,v $
| Revision 5.2  2001/08/09 05:13:34  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:28  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:33  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:49  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:10  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:20  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/03/02 06:39:55  scott
| Updated to correct programs that update database tables without locking the record. This does not cause a problem with the character based code but causes a problem with GVision.
|
| Revision 1.12  2000/02/18 01:56:23  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.11  1999/12/06 01:47:17  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 09:41:56  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.9  1999/10/20 02:06:49  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/09/29 10:11:09  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:27:01  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 02:36:51  scott
| Update to add log + change database names + misc clean up.
|
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: menumaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mn_menumaint/menumaint.c,v 5.2 2001/08/09 05:13:34 scott Exp $";

#define MAXSTR	100
#include	<pslscr.h>
#include	<hot_keys.h>
#include	<tabdisp.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#define MNCD_SCN 2
#define MNLN_SCN 3

#define MNLN_NULL ((struct MNLN_PTR *) NULL)
#define MNCD_NULL ((struct MNCD_PTR *) NULL)

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		long	t_dbt_date;
	} comm_rec;

	/*===============================
	| Menu System Menu Header File  |
	===============================*/
	struct dbview mnhr_list[] ={
		{"mnhr_name"},
		{"mnhr_description"},
		{"mnhr_help"},
		{"mnhr_fast_access"},
		{"mnhr_hhmn_hash"},
		{"mnhr_is_sub"},
		{"mnhr_heading"},
		{"mnhr_trailer"},
		{"mnhr_fast"},
		{"mnhr_sub"},
		{"mnhr_menu_name"},
		{"mnhr_shell_out"},
	};

	int mnhr_no_fields = 12;

	struct {
		char	hr_name[15];
		char	hr_description[61];
		char	hr_help[15];
		char	hr_fast_access[7];
		long	hr_hhmn_hash;
		char	hr_is_sub[2];
		int	hr_heading;
		int	hr_trailer;
		int	hr_fast;
		int	hr_sub;
		int	hr_menu_name;
		int	hr_shell_out;
	} mnhr_rec;

	/*============================
	| Menu System Menu Line File |
	============================*/
	struct dbview mnln_list[] ={
		{"mnln_hhmn_hash"},
		{"mnln_hhln_hash"},
		{"mnln_line_no"},
		{"mnln_description"},
		{"mnln_prog_call"},
		{"mnln_quad"},
		{"mnln_line_space"}
	};

	int mnln_no_fields = 7;

	struct	MNLN_REC
	{
		long	ln_hhmn_hash;
		long	ln_hhln_hash;
		int	ln_line_no;
		char	ln_description[61];
		char	ln_prog_call[161];
		int	ln_quad;
		int	ln_line_space;
	} mnln_rec;

	/*================================
	| Menu System Security Link File |
	================================*/
	struct dbview mnsc_list[] ={
		{"mnsc_hhln_hash"},
		{"mnsc_hhac_hash"},
	};

	int	mnsc_no_fields = 2;

	struct	{
		long	sc_hhln_hash;
		long	sc_hhac_hash;
	} mnsc_rec;

	/*===================================
	| Menu System Comment Details File. |
	===================================*/
	struct dbview mncd_list[] ={
		{"mncd_hhmn_hash"},
		{"mncd_line_no"},
		{"mncd_description"},
		{"mncd_line"},
		{"mncd_column"},
		{"mncd_width"},
		{"mncd_depth"},
		{"mncd_reverse"},
		{"mncd_stat_flag"}
	};

	int mncd_no_fields = 9;

	struct MNCD_REC
	{
		long	cd_hhmn_hash;
		int	cd_line_no;
		char	cd_description[81];
		int	cd_line;
		int	cd_column;
		int	cd_width;
		int	cd_depth;
		int	cd_reverse;
		char	cd_stat_flag[2];
	} mncd_rec;

	/*======================================
	| Menu System Access Description File. |
	======================================*/
	struct dbview mnac_list[] ={
		{"mnac_hhac_hash"},
		{"mnac_code"},
		{"mnac_description"}
	};

	int mnac_no_fields = 3;

	struct {
		long	ac_hhac_hash;
		char	ac_code[9];
		char	ac_description[31];
	} mnac_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
int	LCL_new_menu;
int	super_user;
int	mn_type;
int	mn_line_no = 0;
int	exit_loop = FALSE;
int	exit_sec = FALSE;
int	move_grab = TRUE;
int	move_from = 1;
int	move_to = 1;
char	mncd_mask[131];
char	sec_master[28];

struct	MNLN_PTR
{
	struct	MNLN_REC	mnln_rec_data;
	long			line_security[50];
	int			mnsc_no;
	struct	MNLN_PTR	*next;
};

struct	MNCD_PTR
{
	struct	MNCD_REC	mncd_rec_data;
	struct	MNCD_PTR	*next;
};

struct  MNCD_PTR *mncd_curr;
struct  MNCD_PTR *mncd_head;

struct  MNLN_PTR *mnln_curr;
struct  MNLN_PTR *mnln_head;

struct	{
	char	dummy[11];
	char	menu_name[15];
	char	is_sub[4];
	char	temp_sub[4];
	char	menu_description[61];
	char	menu_help[15];
	char	menu_fast_access[7];
	char	menu_heading[7];
	char	menu_trailer[7];
	char	menu_fast[7];
	char	menu_sub[7];
	char	menu_menu_name[7];
	char	menu_shell_out[7];
	char	cd_reverse[4];	
	char	cd_stat_flag[9];	
	char	line_prog1[81];
	char	line_prog2[81];
	char	acc_security[28];
} local_rec;

static	int	quit_func		(int c, KEY_TAB *psUnused);
static	int	insert_func		(int c, KEY_TAB *psUnused);
static	int	update_func		(int c, KEY_TAB *psUnused);
static	int	delete_func		(int c, KEY_TAB *psUnused);
static	int	append_func		(int c, KEY_TAB *psUnused);
static	int	copy_func		(int c, KEY_TAB *psUnused);
static	int	move_func		(int c, KEY_TAB *psUnused);
static	int	show_func		(int c, KEY_TAB *psUnused);
static	int	exit_update_func(int c, KEY_TAB *psUnused);
static	int	choose_func		(int c, KEY_TAB *psUnused);
static	int	add_sec_func	(int c, KEY_TAB *psUnused);
static	int	del_sec_func	(int c, KEY_TAB *psUnused);
static	int	exit_sec_func	(int c, KEY_TAB *psUnused);
static	int	tag_sec			(int c, KEY_TAB *psUnused);
static	int	exit_tag_func	(int c, KEY_TAB *psUnused);

static	KEY_TAB edit_keys [] =
{
   { "[Q]UIT",		'Q', quit_func,
	"Exit without updating the database.",				"A" },
   { "[I]NSERT",	'I', insert_func,
	"Insert a line above the current cursor position",		"E" },
   { "[U]PDATE",	'U', update_func,
	"Update the line at the current cursor position.",		"E" },
   { "[D]ELETE",	'D', delete_func,
	"Delete the line at the current cursor position.",		"E" },
   { "[A]PPEND",	'A', append_func,
	"Append a line after the last line in the table.",		"A" },
   { "[C]OPY",		'C', copy_func,
	"Copy the current line and append it to the end of the table",	"E" },
   { "[M]OVE",		'M', move_func,
	"Hit M on line you wish to move. Move cursor to new position and hit M again. Line will be inserted above current cursor position.",			    		"E" },
   { "[S]ECURITY ACCESS",	'S', show_func,
	"Display user access for current menu line",			"ES" },
   { NULL,		FN16, exit_update_func,
	"Exit and update the database.",				"A" },
   END_KEYS
};

static	KEY_TAB choose_keys [] =
{
   { NULL,		'1', choose_func,				    },
   { NULL,		'2', choose_func,				    },
   { NULL,		'3', choose_func,				    },
   { NULL,		'\r', choose_func,				    },
   { NULL,		FN16, choose_func,				    },
   END_KEYS
};

static	KEY_TAB show_keys [] =
{
   { "[A]DD",		'A', add_sec_func,
	"Exit without updating the database.",				"A" },
   { "[D]ELETE",	'D', del_sec_func,
	"Insert a line above the current cursor position",		"E" },
   { NULL,		FN16, exit_sec_func,				    },
   END_KEYS
};

static	KEY_TAB security_keys [] =
{
   { "[T]AG/UNTAG",		'T', tag_sec,
	"Exit without updating the database.",				"A" },
   { "[^A]ll Tag",	CTRL('A'), tag_sec,
	"Insert a line above the current cursor position",		"A" },
   { NULL,		FN16, exit_tag_func,				    },
   END_KEYS
};

static	struct	var	vars[] =
{
	{1, LIN, "menu_name",	 4, 12, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Menu Name:", "Enter name of menu you wish to maintain",
		 NE, NO,  JUSTLEFT, "", "", local_rec.menu_name},
	{1, LIN, "is_sub",	 4, 43, CHARTYPE,
		"U", "          ",
		" ", local_rec.temp_sub, "    Sub Menu:", "Is This A Sub Menu  (Y/N)",
		 NE, NO,  JUSTLEFT, "YN", "", local_rec.is_sub},
	{1, LIN, "menu_desc",	 4, 68, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "   Description:", "Enter the on-screen description of the menu",
		 NI, NO,  JUSTLEFT, "", "", local_rec.menu_description},
	{1, LIN, "menu_help",	 5, 14, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Help Program:", " ",
		 NI, NO,  JUSTLEFT, "", "", local_rec.menu_help},
	{1, LIN, "menu_fast_access",	 6, 14, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Fast Access:", "Enter four letter fast access code for this menu",
		 NI, NO,  JUSTLEFT, "", "", local_rec.menu_fast_access},
	{1, LIN, "menu_heading",	 8, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Heading:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_heading},
	{1, LIN, "menu_trailer",	 9, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Trailer:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_trailer},
	{1, LIN, "menu_fast",	10, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Fast:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_fast},
	{1, LIN, "menu_sub",	11, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Sub:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_sub},
	{1, LIN, "menu_menu_name",	12, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Menu Name:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_menu_name},
	{1, LIN, "menu_shell_out",	13, 17, CHARTYPE,
		"U", "          ",
		" ", "", "Enable Shell Out:", "Enter  (Y/N/I)",
		 NI, NO,  JUSTLEFT, "YNI", "", local_rec.menu_shell_out},
	{2, LIN, "stat_flag",	16, 14, CHARTYPE,
		"U", "          ",
		" ", "", "Command Type:", "Enter Quadrant/ Comment/ Box/ Line",
		YES, NO,  JUSTLEFT, "QCLB", "", local_rec.cd_stat_flag},
	{2, LIN, "cd_line_desc",	17, 14, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Comment Text:", "Please Enter comment within quotes eg. \042hello\042 ",
		 NI, NO,  JUSTLEFT, "", "", mncd_rec.cd_description},
	{2, LIN, "cd_column",	18, 14, INTTYPE,
		"NN", "          ",
		" ", "", "Column:", "",
		 NI, NO, JUSTRIGHT, "0", "79", (char *) &mncd_rec.cd_column},
	{2, LIN, "cd_line",	18, 40, INTTYPE,
		"NN", "          ",
		" ", "", "Line:", "",
		 NI, NO, JUSTRIGHT, "0", "23", (char *) &mncd_rec.cd_line},
	{2, LIN, "cd_width",	18, 66, INTTYPE,
		"NN", "          ",
		" ", "", "Width:", "",
		 NI, NO, JUSTRIGHT, "0", "80", (char *) &mncd_rec.cd_width},
	{2, LIN, "cd_depth",	18, 92, INTTYPE,
		"NN", "          ",
		" ", "", "Depth:", "",
		 NI, NO, JUSTRIGHT, "0", "24", (char *) &mncd_rec.cd_depth},
	{2, LIN, "cd_reverse",	19, 14, CHARTYPE,
		"U", "          ",
		" ", "", "Reverse:", "Reverse video on  (Y/N)",
		 NI, NO,  JUSTLEFT, "YN", "", local_rec.cd_reverse},
	{3, LIN, "line_desc",	16, 14, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		YES, NO,  JUSTLEFT, "", "", mnln_rec.ln_description},
	{3, LIN, "line_prog1",	17, 14, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Program Call:", " ",
		YES, YES,  JUSTLEFT, "", "", local_rec.line_prog1},
	{3, LIN, "line_prog2",	18, 14, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "        ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.line_prog2},
	{3, LIN, "line_quad",	19, 14, INTTYPE,
		"NN", "          ",
		" ", "", "Quadrant:", "Quadrant (n) must have been defined in screen painting section ",
		YES, NO, JUSTRIGHT, "", "", (char *) &mnln_rec.ln_quad},
	{3, LIN, "line_line_space",	19, 40, INTTYPE,
		"NN", "          ",
		" ", "", "Line Space:", " ",
		YES, NO, JUSTRIGHT, "1", "24", (char *) &mnln_rec.ln_line_space},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*==========================
| Main Processing Routine. |
==========================*/
int		main				(int argc, char * argv []);
void	shutdown_prog		(void);
void	OpenDB				(void);
void	CloseDB			(void);
int		choose_func			(int c, KEY_TAB *psUnused);
void	proc_header			(void);
void	update_mnhr			(void);
void	update_mncd			(void);
void	update_mnln			(void);
int		update_security		(void);
void	proc_scr_painting	(void);
void	proc_lines			(void);
int		load_mnsc			(void);
struct MNCD_PTR * mncd_alloc(void);
struct MNLN_PTR * mnln_alloc(void);
int		mn_insert			(int line, int join_lines);
void	mncd_find_pos		(int line);
void	mnln_find_pos		(int line);
void	join_prog_call		(void);
void	split_prog_call		(void);
void	mncd_set_all_fields	(void);
void	mn_delete			(int line);
int		get_security		(void);
void	erase_btm_scn		(void);
void	clr_mncd_mem		(void);
void	clr_mnln_mem		(void);
void	mncd_tab_create		(void);
void	exp_stat_flag		(void);
void	mnln_tab_create		(void);
int		mnsc_tab_create		(void);
void	mem_alloc_err		(void);
int		spec_valid			(int field);
void	val_prog_call		(void);
int		set_req_fields		(void);
void	srch_menu			(char * key_val);
void	hr_to_loc			(char * lcl_chars_desc, int add_hr_char);
int		loc_to_hr			(char * lcl_chars_desc, int * add_hr_char);
int		heading				(int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	SETUP_SCR(vars);

	strcpy(sec_master,"abcdefghijklmnopqrstuvwxyz*");

	super_user = (argc == 1);

	if (!super_user)
	{
		FLD("line_prog1") = NA;
		FLD("line_prog2") = NA;
	}

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/
	swide();

	/*---------------------------
	| Open main database files. |  
	---------------------------*/
	OpenDB();

	/*---------------------
	| Set help key to FN6 |
	---------------------*/
	set_help(FN6,"FN6");

	strcpy(local_rec.temp_sub,"N");

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*----------------------------------
		| Reset Control Flags              |
		----------------------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);

		/*----------------------------------
		| Enter screen 1 linear input.     |
		----------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit)
			break;

		if (LCL_new_menu == 1)
			proc_header();

		if (restart)
			continue;

		tab_open("choose_option",choose_keys,0,0,4,TRUE);
		tab_add("choose_option","# Edit Which Menu Section: ");  
		tab_add("choose_option","1  Header Section");  
		tab_add("choose_option","2  Screen Painting Section ");  
		tab_add("choose_option","3  Menu Line Section ");  
		tab_scan("choose_option");

		tab_close("choose_option", TRUE);
		update_mnhr();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("mnhr", mnhr_list, mnhr_no_fields, "mnhr_id_no");
	open_rec("mncd", mncd_list, mncd_no_fields, "mncd_id_no");
	open_rec("mnln", mnln_list, mnln_no_fields, "mnln_id_no");
	open_rec("mnsc", mnsc_list, mnsc_no_fields, "mnsc_id_no");
	open_rec("mnac", mnac_list, mnac_no_fields, "mnac_code");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("mnhr");
	abc_fclose("mncd");
	abc_fclose("mnln");
	abc_fclose("mnsc");
	abc_fclose("mnac");
	abc_dbclose("data");
}

/*---------------------------------------------
| Call Header,Screen Painting or Line section |
---------------------------------------------*/
static	int
choose_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (c == '\r')
		c = (tab_tline("choose_option") + 1);
	else
		if (c != FN16)
			c = c - '0';

	tab_clear("choose_option");

	switch (c)
	{
	case 1:
		proc_header();
		break;
	case 2:
		proc_scr_painting();
		break;
	case 3:
		proc_lines();
		break;
	case FN16:
		return(FN16);
		break;
	default:
		break;
	}
	redraw_table ("choose_option");
	return (EXIT_SUCCESS);
}

/*--------------------------------
| Process the header information |
--------------------------------*/
void
proc_header (void)
{
	/*----------------------------------
	| Edit screen 1 linear input.      |
	----------------------------------*/
	heading(1);
	scn_display(1);
	edit(1);
	display_prmpt(label("menu_desc"));
}

void
update_mnhr (void)
{
	int	cd_lines = FALSE;
	int	ln_lines = FALSE;
	
	mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mnln_rec.ln_line_no = 0;
	cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
	if (!cc)
		ln_lines = TRUE;

	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 0;
	cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	if (!cc)
		cd_lines = TRUE;

	if (cd_lines || ln_lines)
	{
		cc = abc_update("mnhr",&mnhr_rec);
		if (cc)
			sys_err("Error in mnhr during (DBUPDATE)",cc,PNAME);

		abc_unlock("mnhr");
	}
	else
	{
		cc = abc_delete("mnhr");
		if (cc)
			sys_err("Error in mnhr during (DBUPDATE)",cc,PNAME);
	}
}

void
update_mncd (void)
{
	print_at(2,0, ML(mlStdMess035) ); 
	fflush(stdout);

	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 0;

	/* Delete any mnln records associated with the current header */
	cc = find_rec("mncd",&mncd_rec,GTEQ,"u");
	while (!cc && mncd_rec.cd_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		cc = abc_delete("mncd");
		if (cc)
			sys_err("Error in mncd during (DBUPDATE)",cc,PNAME);
			
		cc = find_rec("mncd",&mncd_rec,GTEQ,"u");
	}
	abc_unlock("mncd");
	
	mncd_curr = mncd_head;

	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 1;

	while (mncd_curr != MNCD_NULL)
	{
		strcpy(mncd_rec.cd_description,mncd_curr->mncd_rec_data.cd_description);
		strcpy(mncd_rec.cd_stat_flag,mncd_curr->mncd_rec_data.cd_stat_flag);
		mncd_rec.cd_line = mncd_curr->mncd_rec_data.cd_line;
		mncd_rec.cd_column = mncd_curr->mncd_rec_data.cd_column;
		mncd_rec.cd_width = mncd_curr->mncd_rec_data.cd_width;
		mncd_rec.cd_depth = mncd_curr->mncd_rec_data.cd_depth;
		mncd_rec.cd_reverse = mncd_curr->mncd_rec_data.cd_reverse;

		cc = abc_add("mncd",&mncd_rec);
		if (cc)
			sys_err("Error in mncd during (DBADD)",cc,PNAME);

		mncd_curr = mncd_curr->next;
		mncd_rec.cd_line_no++;
	}
	move(0,2);
	cl_line();
}

void
update_mnln (void)
{
	int	line_no;

	print_at(2,0, ML(mlStdMess035) ); 
	fflush(stdout);

	mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mnln_rec.ln_line_no = 0;

	/* Delete any mnln records associated with the current header */
	cc = find_rec("mnln",&mnln_rec,GTEQ,"u");
	while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		cc = abc_delete("mnln");
		if (cc)
			sys_err("Error in mnln during (DBUPDATE)",cc,PNAME);
			
		cc = find_rec("mnln",&mnln_rec,GTEQ,"u");
	}
	abc_unlock ("mnln");

	mnln_curr = mnln_head;
	
	mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mnln_rec.ln_line_no = 1;

	while (mnln_curr != MNLN_NULL)
	{
		sprintf(mnln_rec.ln_description,"%-60.60s",mnln_curr->mnln_rec_data.ln_description);
		sprintf(mnln_rec.ln_prog_call,"%-160.160s",mnln_curr->mnln_rec_data.ln_prog_call);
		mnln_rec.ln_quad = mnln_curr->mnln_rec_data.ln_quad;
		mnln_rec.ln_line_space = mnln_curr->mnln_rec_data.ln_line_space;

		cc = abc_add("mnln",&mnln_rec);
		if (cc)
			sys_err("Error in mnln during (DBADD)",cc,PNAME);

		line_no = mnln_rec.ln_line_no;

		update_security();

		mnln_curr = mnln_curr->next;
		mnln_rec.ln_line_no = ++line_no;
	}
	move(0,2);
	cl_line();
}

int
update_security (void)
{
	int	i;
	int	lcl_cc;

	lcl_cc = find_rec("mnln", &mnln_rec, COMPARISON, "r");
	if (lcl_cc)
		return(0);
	
	mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
	mnsc_rec.sc_hhac_hash = 0L;

	cc = find_rec("mnsc", &mnsc_rec, GTEQ, "u");
	while (!cc && mnsc_rec.sc_hhln_hash == mnln_rec.ln_hhln_hash)
	{
		cc = abc_delete("mnsc");
		if (cc)
			sys_err("Error in mnsc during (DBUPDATE)",cc,PNAME);
			
		cc = find_rec("mnsc", &mnsc_rec, GTEQ, "u");
	}
	abc_unlock ("mnsc");
	
	for (i = 0; i < mnln_curr->mnsc_no; i++)
	{
		mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
		mnsc_rec.sc_hhac_hash = mnln_curr->line_security[i];
		cc = abc_add("mnsc", &mnsc_rec);
		if (cc)
			sys_err("Error in mnsc during (DBADD)",cc,PNAME);
	}

	return(0);
}

/*-------------------------------------
| Process Screen Painting Information |
-------------------------------------*/
void
proc_scr_painting (void)
{
	struct	MNCD_PTR *mncd_lcl;

	mn_type = MNCD_SCN;
	mn_line_no = 0;

	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 0;

	cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	while (!cc && mncd_rec.cd_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		/*  Store record in linked list */
		/*  Allocate some memory space  */
		mncd_lcl = mncd_alloc();

		mncd_lcl->next = MNCD_NULL;

		if (mncd_head == MNCD_NULL)
			mncd_head = mncd_lcl;
		else
			mncd_curr->next = mncd_lcl;

		mncd_curr = mncd_lcl;

		memcpy ((char *) &mncd_lcl->mncd_rec_data, 
			   (char *) &mncd_rec, 
			   sizeof (struct MNCD_REC));
		mn_line_no++;


		cc = find_rec("mncd",&mncd_rec,NEXT,"r");
	}
	set_keys(edit_keys,"S",KEY_PASSIVE);

	heading(1);
	scn_display(1);

	exit_loop = FALSE;
	do
	{
		if (mncd_head == MNCD_NULL)
			set_keys(edit_keys,"E",KEY_PASSIVE);
		else
		{
			set_keys(edit_keys,"E",KEY_ACTIVE);
			set_keys(edit_keys,"S",KEY_PASSIVE);
		}

		mncd_tab_create();
			
		tab_scan("mncd");
		tab_close("mncd",TRUE);
	} while (!exit_loop);
	
	box(0,3,132,1);
}

/*--------------------------------
| Process Menu Lines Information |
--------------------------------*/
void
proc_lines (void)
{
	struct	MNLN_PTR *mnln_lcl = MNLN_NULL;

	mn_type = MNLN_SCN;
	mn_line_no = 0;

	mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mnln_rec.ln_line_no = 0;

	cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
	while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		/*  Store record in linked list */
		/*  Allocate some memory space  */
		mnln_lcl = mnln_alloc();

		mnln_lcl->next = MNLN_NULL;

		if (mnln_head == MNLN_NULL)
			mnln_head = mnln_lcl;
		else
			mnln_curr->next = mnln_lcl;

		mnln_curr = mnln_lcl;

		memcpy ((char *) &mnln_lcl->mnln_rec_data, 
			   (char *) &mnln_rec, 
			   sizeof (struct MNLN_REC));
		mnln_curr->mnsc_no = 0;
		load_mnsc();

		mn_line_no++;
		cc = find_rec("mnln",&mnln_rec,NEXT,"r");
	}
	heading(1);
	scn_display(1);

	exit_loop = FALSE;
	do
	{
		if (mnln_head == MNLN_NULL)
			set_keys(edit_keys,"E",KEY_PASSIVE);
		else
			set_keys(edit_keys,"E",KEY_ACTIVE);

		mnln_tab_create();

		tab_scan("mnln");
		tab_close("mnln",TRUE);
	} while (!exit_loop);

	box(0,3,132,1);
}

int
load_mnsc (void)
{
	int	lcl_cc;

	mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
	mnsc_rec.sc_hhac_hash = 0L;
	lcl_cc = find_rec("mnsc", &mnsc_rec, GTEQ, "r");
	while (!lcl_cc && mnsc_rec.sc_hhln_hash == mnln_rec.ln_hhln_hash)
	{
		mnln_curr->line_security[mnln_curr->mnsc_no++] = mnsc_rec.sc_hhac_hash;

		lcl_cc = find_rec("mnsc", &mnsc_rec, NEXT, "r");
	}

	return(0);
}

/*--------------------------------------
| Allocate memory for mncd linked list |
--------------------------------------*/
struct	MNCD_PTR *
mncd_alloc (void)
{
	struct	MNCD_PTR	*lcl_ptr;

	lcl_ptr = (struct MNCD_PTR *) malloc (sizeof (struct MNCD_PTR));

	if (lcl_ptr == MNCD_NULL)
		mem_alloc_err();
		
	return (lcl_ptr);
}

/*--------------------------------------
| Allocate memory for mnln linked list |
--------------------------------------*/
struct	MNLN_PTR *
mnln_alloc (void)
{
	struct	MNLN_PTR	*lcl_ptr;

	lcl_ptr = (struct MNLN_PTR *) malloc (sizeof (struct MNLN_PTR));

	if (lcl_ptr == MNLN_NULL)
		mem_alloc_err();

	return (lcl_ptr);
}

/*---------------------------------------------------
| Exit the current table and update to the database |
---------------------------------------------------*/
static	int
exit_update_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (mn_type == MNCD_SCN)
	{	
		update_mncd();
		clr_mncd_mem();
	}

	if (mn_type == MNLN_SCN)
	{
		update_mnln();
		clr_mnln_mem();
	}

	exit_loop = TRUE;
	return(FN16);
}

/*------------------------------------
| Quit without updating the database |
------------------------------------*/
static	int
quit_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (mn_type == MNCD_SCN)
		clr_mncd_mem();
	else
		clr_mnln_mem();

	exit_loop = TRUE;
	return(FN16);
}

/*-------------------------------------------------
| Get the information from the user for an insert |
-------------------------------------------------*/
static	int
insert_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int	line = 0;

	move_grab = TRUE;

	if (mn_type == MNCD_SCN)
		line = tab_tline("mncd");

	if (mn_type == MNLN_SCN)
		line = tab_tline("mnln");

	mncd_set_all_fields();

	init_vars(mn_type);
	heading(mn_type);
	entry(mn_type);
	edit(mn_type);
	erase_btm_scn();
	mn_insert(line, TRUE);

	return(FN16);
}

/*--------------------------------------
| Insert an entry into the linked list |
--------------------------------------*/
int
mn_insert (
 int	line,
 int	join_lines)
{
	struct	MNCD_PTR *mncd_lcl;
	struct	MNLN_PTR *mnln_lcl;

	if (restart)
	{
		restart = 0;
		return (EXIT_SUCCESS);
	}

	if (mn_type == MNCD_SCN)
	{
		mncd_lcl = mncd_alloc();
		if (line == 0)
		{
			mncd_lcl->next = mncd_head;
			mncd_head = mncd_lcl;
			memcpy ((char *) &mncd_lcl->mncd_rec_data, 
				   (char *) &mncd_rec, 
				   sizeof (struct MNCD_REC));
		}

		if (line >= 1)
		{
			line--;
			mncd_find_pos(line);
			mncd_lcl->next = mncd_curr->next;
			mncd_curr->next = mncd_lcl;
			memcpy ((char *) &mncd_lcl->mncd_rec_data, 
				   (char *) &mncd_rec, 
				   sizeof (struct MNCD_REC));
		}

		return(0);
	}
			
	if (mn_type == MNLN_SCN)
	{
		if (join_lines)
			join_prog_call();

		mnln_lcl = mnln_alloc();
		mnln_lcl->mnsc_no = 0;
		if (line == 0)
		{
			mnln_lcl->next = mnln_head;
			mnln_head = mnln_lcl;
			memcpy ((char *) &mnln_lcl->mnln_rec_data, 
				   (char *) &mnln_rec, 
				   sizeof (struct MNLN_REC));
		}
		else
		{
			line--;
			mnln_find_pos(line);
			mnln_lcl->next = mnln_curr->next;
			mnln_curr->next = mnln_lcl;
			memcpy ((char *) &mnln_lcl->mnln_rec_data, 
				   (char *) &mnln_rec, 
				   sizeof (struct MNLN_REC));
		}
	}

	return(0);
}

/*---------------------------------------------------------------
| Set a pointer to the correct position in the mncd linked list |
---------------------------------------------------------------*/
void
mncd_find_pos (
 int	line)
{
	int	loop;

	mncd_curr = mncd_head;

	for (loop = 0; loop < line; loop++)
		mncd_curr = mncd_curr->next;
}

/*---------------------------------------------------------------
| Set a pointer to the correct position in the mnln linked list |
---------------------------------------------------------------*/
void
mnln_find_pos (
 int	line)
{
	int	loop;

	mnln_curr = mnln_head;

	for (loop = 0; loop < line; loop++)
		mnln_curr = mnln_curr->next;
}

/*-----------------------------------------------------------
| Join the two lines of prog_call into one ready for update |
-----------------------------------------------------------*/
void
join_prog_call (void)
{
	sprintf(mnln_rec.ln_prog_call,
		"%-80.80s%-80.80s",
		local_rec.line_prog1,
		local_rec.line_prog2);
}

/*--------------------------------------------
| Split prog_call into two ready for display |
--------------------------------------------*/
void
split_prog_call (void)
{
	char	*sptr;
	int	loop = 0;

	sprintf(local_rec.line_prog1,"%-80.80s",mnln_rec.ln_prog_call);
	sptr = mnln_rec.ln_prog_call;
	
	/*---------------------------------------------------
	| Position pointer at start of second 80 characters |
	| and put these characters into prog_call2	    |
	---------------------------------------------------*/
	while (*sptr)
	{
		sptr++;
		loop++;
		if (*sptr && loop == 79)
		{
			sptr++;
			sprintf(local_rec.line_prog2,"%-80.80s",sptr);
			*sptr = '\0';
		}
	}
}

/*-----------------------------------------------------------------
| Update the current line in the table and update the linked list |
-----------------------------------------------------------------*/
static	int
update_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int	line;
		

	move_grab = TRUE;

	mncd_set_all_fields();
	heading(mn_type);

	if (mn_type == MNCD_SCN)
	{
		line = tab_tline("mncd");
		if (line == 0)
			mncd_curr = mncd_head;
		else
			mncd_find_pos(line);
		memcpy ((char *) &mncd_rec,
			   (char *) &mncd_curr->mncd_rec_data,
			   sizeof (struct MNCD_REC));
		
		exp_stat_flag();

		init_ok = 0;
		entry(mn_type);
		init_ok = 1;
	}
		
	if (mn_type == MNLN_SCN)
	{
		line = tab_tline("mnln");
		if (line == 0)
			mnln_curr = mnln_head;
		else
			mnln_find_pos(line);
		memcpy ((char *) &mnln_rec,
			   (char *) &mnln_curr->mnln_rec_data,
			   sizeof (struct MNLN_REC));
		split_prog_call();
	
	}
	scn_display(mn_type);
	edit(mn_type);

	erase_btm_scn();
	line++;

	if (restart)
	{
		restart = 0;
		return (EXIT_SUCCESS);
	}

	if (mn_type == MNCD_SCN)
	{
		memcpy ((char *) &mncd_curr->mncd_rec_data,
			   (char *) &mncd_rec,
			   sizeof (struct MNCD_REC));
	}
		
	if (mn_type == MNLN_SCN)
	{
		join_prog_call();
		memcpy ((char *) &mnln_curr->mnln_rec_data,
			   (char *) &mnln_rec,
			   sizeof (struct MNLN_REC));
	}
	return(FN16);
}

/*---------------------------------------------------------
| Set all the fields in the mncd screen ready for display |
---------------------------------------------------------*/
void
mncd_set_all_fields (void)
{
	FLD("cd_column") = NI;
	FLD("cd_line") = NI;
	FLD("cd_width") = NI;
	FLD("cd_depth") = NI;
	FLD("cd_line_desc") = NI;
	FLD("cd_reverse") = NI;
}

/*---------------------------------
| Get info from user about delete |
---------------------------------*/
static	int
delete_func (
 int		iUnused,
 KEY_TAB *	psUnused)
{
	int	c;
	int	line = 0;
	struct	MNLN_PTR *mnln_lcl;

	
	move_grab = TRUE;

	if (mn_type == MNCD_SCN)
	{
		line = tab_tline("mncd");
		if (line == 0)
		{
			mncd_curr = mncd_head;
			memcpy ((char *) &mncd_rec,
			   	   (char *) &mncd_curr->mncd_rec_data,
				   sizeof (struct MNCD_REC));
			set_req_fields();
		}
		else
		{
			line--;
			mncd_find_pos(line);
			memcpy ((char *) &mncd_rec,
				   (char *) &mncd_curr->next->mncd_rec_data,
				   sizeof (struct MNCD_REC));
			set_req_fields();
		}
	}

	if (mn_type == MNLN_SCN)
	{
		line = tab_tline("mnln");
		if (line == 0)
		{
			mnln_curr = mnln_head;
			mnln_lcl = mnln_curr;
		}
		else
		{
			line--;
			mnln_find_pos(line);
			mnln_lcl = mnln_curr->next;
		}
		memcpy ((char *) &mnln_rec,
			   (char *) &mnln_lcl->mnln_rec_data,
			   sizeof (struct MNLN_REC));
		split_prog_call();
	}

	heading(mn_type);
	scn_display(mn_type);
	/*Delete This Line ? */
	c = prmptmsg( ML(mlStdMess151), "YyNn",0,2);

	if (c == 'N' || c == 'n')
	{
		move(0,2);
		cl_line();
		erase_btm_scn();
		return(0);
	}

	if (mn_type == MNCD_SCN)
		line = tab_tline("mncd");

	if (mn_type == MNLN_SCN)
		line = tab_tline("mnln");

	mn_delete(line);
	move(0,2);
	cl_line();
	erase_btm_scn();

	return(FN16);
}

/*-----------------------------------------------------
| Delete the current table entry from the linked list |
-----------------------------------------------------*/
void
mn_delete (
 int	line)
{
	struct	MNCD_PTR *mncd_lcl;
	struct	MNLN_PTR *mnln_lcl;

	if (mn_type == MNCD_SCN)
	{
		if (line == 0)
		{
			mncd_head = mncd_head->next;
			free(mncd_curr);
		}
		else
		{
			mncd_lcl = mncd_curr->next;
			mncd_curr->next = mncd_lcl->next;
			free(mncd_lcl);
		}
	}

	if (mn_type == MNLN_SCN)
	{
		if (line == 0)
		{
			mnln_head = mnln_head->next;
			free(mnln_curr);
		}
		else
		{
			mnln_lcl = mnln_curr->next;
			mnln_curr->next = mnln_lcl->next;
			free(mnln_lcl);
		}
	}
}

/*-----------------------------------------------
| Append an entry to the end of the linked list |
-----------------------------------------------*/
static	int
append_func (
 int		c,
 KEY_TAB *	psUnused)
{
	move_grab = TRUE;

	mncd_set_all_fields();

	init_vars(mn_type);
	heading(mn_type);
	entry(mn_type);
	edit(mn_type);
	erase_btm_scn();
	mn_insert(mn_line_no, TRUE);

	return(FN16);
}

/*--------------------------------------------------------------------
| Copy the current entry and append it to the end of the linked list |
--------------------------------------------------------------------*/
static	int
copy_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int	line;
	
	move_grab = TRUE;

	if (mn_type == MNCD_SCN)
	{
		line = tab_tline("mncd");
		mncd_find_pos(line);
		memcpy ((char *) &mncd_rec,
			   (char *) &mncd_curr->mncd_rec_data,
			   sizeof (struct MNCD_REC));
		mn_insert(mn_line_no, FALSE);
	}

	if (mn_type == MNLN_SCN)
	{
		line = tab_tline("mnln");
		mnln_find_pos(line);
		memcpy ((char *) &mnln_rec,
			   (char *) &mnln_curr->mnln_rec_data,
			   sizeof (struct MNLN_REC));
		mn_insert(mn_line_no, FALSE);
	}

	return(FN16);
}

/*---------------------------------------------------------------------------
| Grab the current entry and move it to the next position where M is pushed |
---------------------------------------------------------------------------*/
static	int
move_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int	line;


	if (move_grab)
	{
		/*---------------------------------------------------
		| Please select destination of line and press [M].  |
		| Line will be inserted above the selected line     |
		---------------------------------------------------*/
		rv_pr( ML(mlMenuMess158),0,2,0);

		if (mn_type == MNCD_SCN)
		{
			move_from = tab_tline("mncd");
			if (move_from == 0)
			{
				mncd_curr = mncd_head;
				memcpy ((char *) &mncd_rec,
					   (char *) &mncd_curr->mncd_rec_data,
					   sizeof (struct MNCD_REC));
			}
			else
			{
				line = move_from - 1;
				mncd_find_pos(line);
				memcpy ((char *) &mncd_rec,
					   (char *) &mncd_curr->next->mncd_rec_data,
					   sizeof (struct MNCD_REC));
			}
		}

		if (mn_type == MNLN_SCN)
		{
			move_from = tab_tline("mnln");
			if (move_from == 0)
			{
				mnln_curr = mnln_head;
				memcpy ((char *) &mnln_rec,
					   (char *) &mnln_curr->mnln_rec_data,
					   sizeof (struct MNLN_REC));
			}
			else
			{
				line = move_from - 1;
				mnln_find_pos(line);
				memcpy ((char *) &mnln_rec,
					   (char *) &mnln_curr->next->mnln_rec_data,
					   sizeof (struct MNLN_REC));
			}
		}
		move_grab = FALSE;

		return(0);
	}

	if (!move_grab)
	{
		move(0,2);
		cl_line();

		if (mn_type == MNCD_SCN)
		{
			move_to = tab_tline("mncd");
		}

		if (mn_type == MNLN_SCN)
		{
			move_to = tab_tline("mnln");
		}

		if (move_from < move_to)
			move_to--;


		mn_delete(move_from);
		split_prog_call();
		mn_insert(move_to, FALSE);
		move_grab = TRUE;

		return(FN16);
	}
	return (EXIT_SUCCESS);
}

static	int
show_func (
 int		c,
 KEY_TAB *	psUnused)
{
	int		line;
	
	move_grab = TRUE;

	line = tab_tline("mnln");
	if (line == 0)
		mnln_curr = mnln_head;
	else
		mnln_find_pos(line);
	memcpy ((char *) &mnln_rec,
		   (char *) &mnln_curr->mnln_rec_data,
		   sizeof (struct MNLN_REC));

	exit_sec = FALSE;
	do
	{
		if (mnln_curr->mnsc_no == 0)
			set_keys(show_keys,"E",KEY_PASSIVE);
		else
			set_keys(show_keys,"E",KEY_ACTIVE);

		mnsc_tab_create();
			
		tab_scan("mnsc");
		tab_close("mnsc",TRUE);
	} while (!exit_sec);

	redraw_table("mnln");
	return (EXIT_SUCCESS);
}

static	int
add_sec_func (
 int		c,
 KEY_TAB *	psUnused)
{
	if (get_security())
		return(FN16);
	else
		redraw_table("mnsc");

	return (c);
}

int
get_security (void)
{
	int	lcl_cc;
	int	i;
	int	security_no = 0;
	int 	ok_to_add;
	int	rtn_val;
	char	tmp_code[51];

	tab_open("security",security_keys,11,87,5,FALSE);
	tab_add("security","#  %-40.40s ","Select Security Access Codes");
	sprintf(mnac_rec.ac_code, "%-8.8s", " ");
	lcl_cc = find_rec("mnac", &mnac_rec, GTEQ, "r");
	while (!lcl_cc)
	{
		ok_to_add = TRUE;
		for (i = 0; i < mnln_curr->mnsc_no; i++)
		{
			if (mnln_curr->line_security[i] == mnac_rec.ac_hhac_hash)
			{
				ok_to_add = FALSE;
				break;
			}
		}

		if (ok_to_add && strncmp(mnac_rec.ac_code, "ERROR", 5))
		{
			tab_add("security",	
				"  %-8.8s %-30.30s",	
				mnac_rec.ac_code,
				mnac_rec.ac_description);
			security_no++;
		}

		lcl_cc = find_rec("mnac", &mnac_rec, NEXT, "r");
	}

	if (security_no == 0)
	{
		tab_add("security","\007 There are no more security codes that ");
		tab_add("security","     you can add to this menu line.    ");
		tab_display("security", TRUE);
		sleep(2);
		tab_close("security", TRUE);
		return(FALSE);
	}
	else
		tab_scan("security");

	rtn_val = FALSE;

	for (i = 0; i < security_no; i++)
	{
		tab_get("security", tmp_code, EQUAL, i);
		if (tagged(tmp_code))
		{
			sprintf(mnac_rec.ac_code, "%-8.8s", &tmp_code[2]);
			lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
				sys_err("Error in mnac during (DBFIND)",cc,PNAME);

			mnln_curr->line_security[mnln_curr->mnsc_no++] = mnac_rec.ac_hhac_hash;
			rtn_val = TRUE;
		}
	}

	tab_close("security", TRUE);

	if (mnln_curr->mnsc_no == 0)
		rtn_val = TRUE;

	return(rtn_val);
}

static	int
tag_sec (
 int		c,
 KEY_TAB *	psUnused)
{
	if (c != 'T')
		tag_all("security");
	else
		tag_toggle("security");

	return(c);
}

static	int
del_sec_func (
 int		c,
 KEY_TAB *	psUnused)
{
	char	tmp_code[51];
	long	del_hash;
	int	i;
	int	lcl_cc;

	tab_get("mnsc", tmp_code, CURRENT, 0);
	
	sprintf(mnac_rec.ac_code, "%-8.8s", &tmp_code[1]);
	lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
	if (lcl_cc)
		sys_err("Error in mnac during (DBFIND)",cc,PNAME);

	del_hash = mnac_rec.ac_hhac_hash;

	/*---------------------
	| Find hash to delete |
	---------------------*/
	for (i = 0; i < mnln_curr->mnsc_no; i++)
		if (del_hash == mnln_curr->line_security[i])
			break;

	/*------------------
	| Delete from list |
	------------------*/
	for (i++; i < mnln_curr->mnsc_no; i++)
		mnln_curr->line_security[i - 1] = mnln_curr->line_security[i];
	
	mnln_curr->mnsc_no--;

	return(FN16);
}

static	int
exit_sec_func (
 int		c,
 KEY_TAB *	psUnused)
{
	exit_sec = TRUE;

	return(FN16);
}

static	int
exit_tag_func (
 int		c,
 KEY_TAB *	psUnused)
{
	return(FN16);
}

/*-------------------------------------
| Erase the bottom part of the screen |
-------------------------------------*/
void
erase_btm_scn (void)
{
	int	i;

	for(i = 15; i < 21; i++)
	{
		move(0,i);
		cl_line();
	}
	move(0,20);
	line(132);
}
	
/*------------------------------------------------
| Clear the memory used for the mncd linked list |
------------------------------------------------*/
void
clr_mncd_mem (void)
{
	struct	MNCD_PTR *mncd_lcl;

	if (mncd_head != MNCD_NULL)
	{
		mncd_curr = mncd_head;

		while (mncd_curr->next != MNCD_NULL)
		{
			mncd_lcl = mncd_curr->next;
			free(mncd_curr);
			mncd_curr = mncd_lcl;
		}
		free(mncd_curr);
		mncd_head = MNCD_NULL;
	}
}

/*------------------------------------------------
| Clear the memory used for the mnln linked list |
------------------------------------------------*/
void
clr_mnln_mem (void)
{
	struct	MNLN_PTR *mnln_lcl;

	if (mnln_head != MNLN_NULL)
	{
		mnln_curr = mnln_head;

		while (mnln_curr->next != MNLN_NULL)
		{
			mnln_lcl = mnln_curr->next;
			free(mnln_curr);
			mnln_curr = mnln_lcl;
		}
		free(mnln_curr);
		mnln_head = MNLN_NULL;
	}
}

/*-------------------------------------------------
| Create the mncd table from the mncd linked list |
-------------------------------------------------*/
void
mncd_tab_create (void)
{
	mn_line_no = 0;

	tab_open("mncd",edit_keys,5,0,6,FALSE);
	tab_add("mncd","#%-4.4s  %-8.8s  %-80.80s   %-6.6s %-4.4s %-5.5s %-5.5s %-7.7s","Line","Command","Description","Column","Line","Width","Depth","Reverse");

	if (mncd_head == MNCD_NULL)
	{
		tab_add("mncd","%-130.130s"," ");
	}
	else
	{
		mncd_curr = mncd_head;

		while (mncd_curr != MNCD_NULL)
		{
			exp_stat_flag();

			mn_line_no++;
			tab_add("mncd",
				mncd_mask,
				mn_line_no,
				local_rec.cd_stat_flag,
				mncd_curr->mncd_rec_data.cd_description,
				mncd_curr->mncd_rec_data.cd_column,
				mncd_curr->mncd_rec_data.cd_line,
				mncd_curr->mncd_rec_data.cd_width,
				mncd_curr->mncd_rec_data.cd_depth,
				local_rec.cd_reverse);

			mncd_curr = mncd_curr->next;
		}
	}
}

/*--------------------------------------------
| Expand the mncd stat_flag to its full name |
--------------------------------------------*/
void
exp_stat_flag (void)
{
	char	width_mask[6];
	char	depth_mask[6];


	strcpy(local_rec.cd_reverse,"   ");

	switch(mncd_curr->mncd_rec_data.cd_stat_flag[0])
	{
	case '0':
		strcpy(local_rec.cd_stat_flag,"Quadrant");
		strcpy(width_mask,"%2.0d");
		strcpy(depth_mask,"%2.0d");
		break;
	case '1':
		strcpy(local_rec.cd_stat_flag,"Comment ");
		if (mncd_curr->mncd_rec_data.cd_reverse == 0)
			strcpy(local_rec.cd_reverse,"No ");
		else
			strcpy(local_rec.cd_reverse,"Yes");
		strcpy(width_mask,"%2.0d");
		strcpy(depth_mask,"%2.0d");
		break;
	case '2':
		strcpy(local_rec.cd_stat_flag,"Box     ");
		strcpy(width_mask,"%2d");
		strcpy(depth_mask,"%2d");
		break;
	case '3':
		strcpy(local_rec.cd_stat_flag,"Line     ");
		strcpy(width_mask,"%2d");
		strcpy(depth_mask,"%2.0d");
		break;
	default:
		break;
	}
	strcpy(mncd_mask," %2d   %-8.8s  %-80.80s     %2d    %2d");
	sprintf(mncd_mask,"%s    %s    %s    %s",mncd_mask,width_mask,depth_mask,"%-3.3s");
}

/*-------------------------------------------------
| Create the mnln table from the mnln linked list |
-------------------------------------------------*/
void
mnln_tab_create (void)
{
	mn_line_no = 0;

	tab_open("mnln",edit_keys,5,0,6,FALSE);
	tab_add("mnln",
		"#       %-4.4s           %-60.60s            %-8.8s           %-10.10s       ",
		"Line",
		"Description",
		"Quadrant",
		"Line Space");

	if (mnln_head == MNLN_NULL)
	{
		tab_add("mnln","%-115.115s"," ");
	}
	else
	{
		mnln_curr = mnln_head;

		while (mnln_curr != MNLN_NULL)
		{
			mn_line_no++;
			tab_add("mnln","        %2d            %-60.60s               %2.0d                 %2.0d       ",
				mn_line_no,
				mnln_curr->mnln_rec_data.ln_description,
				mnln_curr->mnln_rec_data.ln_quad,
				mnln_curr->mnln_rec_data.ln_line_space);

			mnln_curr = mnln_curr->next;
		}
	}
}

int
mnsc_tab_create (void)
{
	int	i;
	int	lcl_cc;

	tab_open("mnsc",show_keys,5,87,6,FALSE);
	tab_add("mnsc",
		"# %-8.8s   %-30.30s ",
		"  Code",
		"      Description");

	if (mnln_curr->mnsc_no == 0)
	{
		tab_add("mnsc", " %-8.8s   %-30.30s ", " ", " ");
	}
	else
	{
		abc_selfield("mnac", "mnac_hhac_hash");

		for (i = 0; i < mnln_curr->mnsc_no; i++)
		{
			lcl_cc = find_hash("mnac", &mnac_rec, COMPARISON, "r", mnln_curr->line_security[i]);
			if (lcl_cc)
				sys_err("Error in mnac during (DBFIND)",cc,PNAME);
			tab_add("mnsc",
				" %-8.8s   %-30.30s ",
				mnac_rec.ac_code,
				mnac_rec.ac_description);
		}

		abc_selfield("mnac", "mnac_code");
	}
	return(0);
}

/*---------------------------------------------
| Error in allocating memory for linked lists |
---------------------------------------------*/
void
mem_alloc_err (void)
{
	sys_err("Error during (MALLOC)",errno,PNAME);
}

int
spec_valid (
 int	field)
{
	int		chck_flag = 0;
	int		quad_cnt;
	int		quote_cnt;
	int		loop;
	char	*sptr;

	/*------------------------------------------
	| Validate Menu Name And Allow Search.     |
	------------------------------------------*/
	if (LCHECK("menu_name")) 
        {
		if (SRCH_KEY)
		{	
                        srch_menu(temp_str);
			sprintf(local_rec.temp_sub,"%-1.1s",mnhr_rec.hr_is_sub);
			return(0);
		}

		sptr = clip(local_rec.menu_name);
		if (strlen(sptr) == 0)
			return(1);

		while (*sptr)
		{
			if (*sptr == ' ')
				return(1);
			sptr++;
		}
		sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);

		return(0);
	}

	if (LCHECK("is_sub")) 
        {
		if (strlen(clip(local_rec.is_sub)) == 0)
			strcpy(local_rec.is_sub,"No ");

		sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
		sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);
		cc = find_rec("mnhr",&mnhr_rec, COMPARISON,"u");
		if (cc)
		{
			LCL_new_menu = 1;

			/*----------------------------------------
			| Set menu characteristics to default -1 |
			----------------------------------------*/
			mnhr_rec.hr_heading = -1;
			mnhr_rec.hr_trailer = -1;
			mnhr_rec.hr_fast = -1;
			mnhr_rec.hr_sub = -1;
			mnhr_rec.hr_menu_name = -1;
			mnhr_rec.hr_shell_out = -1;

			cc = abc_add("mnhr",&mnhr_rec);
			if (cc)
			       sys_err("Error in mnhr during (DBADD)",cc,PNAME);

			cc = find_rec("mnhr",&mnhr_rec, COMPARISON,"u");
			if (cc)
			{
				sys_err("Error in mnhr during (DBFIND)",
					cc,PNAME);
			}
		}
		else
		{
			LCL_new_menu = 0;
			strcpy(local_rec.menu_name,mnhr_rec.hr_name);
			strcpy(local_rec.is_sub,mnhr_rec.hr_is_sub);
			strcpy(local_rec.menu_description,
			       mnhr_rec.hr_description);
			strcpy(local_rec.menu_help,mnhr_rec.hr_help);
			sprintf(local_rec.menu_fast_access,
				"%-6.6s",
				mnhr_rec.hr_fast_access);
		}

		hr_to_loc(local_rec.menu_heading, mnhr_rec.hr_heading);
		hr_to_loc(local_rec.menu_trailer, mnhr_rec.hr_trailer);
		hr_to_loc(local_rec.menu_fast, mnhr_rec.hr_fast);
		hr_to_loc(local_rec.menu_sub, mnhr_rec.hr_sub);
		hr_to_loc(local_rec.menu_menu_name, mnhr_rec.hr_menu_name);
		hr_to_loc(local_rec.menu_shell_out, mnhr_rec.hr_shell_out);

		if (local_rec.is_sub[0] == 'Y')
			strcpy(local_rec.is_sub,"Yes");
		else
			strcpy(local_rec.is_sub,"No ");
		display_field(field);

		entry_exit = 1;
                return(0);
	}

	if (LCHECK("menu_desc")) 
	{
		sprintf(mnhr_rec.hr_description,
			"%-60.60s",
			local_rec.menu_description);
		return(0);
	}

	if (LCHECK("menu_help")) 
        {
		sprintf(mnhr_rec.hr_help,"%-14.14s",local_rec.menu_help);
		return(0);
	}

	if (LCHECK("menu_fast_access")) 
	{
		sprintf(mnhr_rec.hr_fast_access,
			"%-6.6s",
			local_rec.menu_fast_access);
		return(0);
	}

	if (LCHECK("menu_heading")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_heading, 
			              &mnhr_rec.hr_heading);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("menu_trailer")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_trailer, 
				      &mnhr_rec.hr_trailer);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("menu_fast")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_fast, &mnhr_rec.hr_fast);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("menu_sub")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_sub, &mnhr_rec.hr_sub);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("menu_menu_name")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_menu_name, 
				      &mnhr_rec.hr_menu_name);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("menu_shell_out")) 
        {
		chck_flag = loc_to_hr(local_rec.menu_shell_out, 
				      &mnhr_rec.hr_shell_out);
		display_field(field);
		return(chck_flag);
	}

	if (LCHECK("line_desc")) 
        {
		if (!super_user && prog_status == ENTRY)
		{
			sprintf(local_rec.line_prog1,"%-80.80s","no_option~");
			sprintf(local_rec.line_prog2,"%-80.80s"," ");
			DSP_FLD("line_prog1");
			DSP_FLD("line_prog2");
		}
		return(0);
	}

	if (LCHECK("line_prog1")) 
        {
		val_prog_call();
		display_field(field);
		return(0);
	}

	if (LCHECK("stat_flag")) 
        {
		switch (local_rec.cd_stat_flag[0])
		{
		case 'Q':
			strcpy(mncd_rec.cd_stat_flag,"0");
			break;
		case 'C':
			strcpy(mncd_rec.cd_stat_flag,"1");
			break;
		case 'B':
			strcpy(mncd_rec.cd_stat_flag,"2");
			break;
		case 'L':
			strcpy(mncd_rec.cd_stat_flag,"3");
			break;
		default:
			return(1);
		}
		set_req_fields();
	
		erase_btm_scn();
		box(0,15,132,4);
		/*---"SCREEN PAINTING SECTION"---*/
		rv_pr( ML(mlMenuMess159),54,15,0);
		scn_write(MNCD_SCN);
		scn_display(MNCD_SCN);
		
		return(0);
	}

	if (LCHECK("cd_reverse")) 
        {
		if (local_rec.cd_reverse[0] == 'N')
		{
			mncd_rec.cd_reverse = 0;
			strcpy(local_rec.cd_reverse,"No ");
			display_field(field);
			return(0);
		}
		else
		{
			mncd_rec.cd_reverse = 1;
			strcpy(local_rec.cd_reverse,"Yes");
			display_field(field);
			return(0);
		}
	}

	if (LCHECK("cd_line_desc")) 
        {
		loop = 0;
		quote_cnt = 0;

		sptr = clip(mncd_rec.cd_description);
		while (*sptr)
		{
			if (*sptr == '"')
				quote_cnt++;
			sptr++;
			loop++;
		}
		loop--;
		
		if (quote_cnt != 2)
			return(1);

		if (mncd_rec.cd_description[0] != '"' || 
		    mncd_rec.cd_description[loop] != '"')
		{
			return(1);
		}

		return(0);
	}

	if (LCHECK("line_quad")) 
        {
		if (mnln_rec.ln_quad < 1)
			return (EXIT_FAILURE);

		quad_cnt = 0;

		mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mncd_rec.cd_line_no = 0;

		cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
      		while (!cc && mncd_rec.cd_hhmn_hash == mnhr_rec.hr_hhmn_hash)
    		{                        
			if (mncd_rec.cd_stat_flag[0] == '0')
				quad_cnt++;
		
			if (mnln_rec.ln_quad == quad_cnt)
				return(0);
	
			cc = find_rec("mncd",&mncd_rec,NEXT,"r");
		}
		return(1);
	}
	return(0);             
}

void
val_prog_call (void)
{
	int	menu;
	int	pmenu;
	char	*tptr;
	char	*sptr;
	char	directory[18];
	char	temp_call[7];
	char	temp_name[15];

	if (super_user)
		FLD( "line_prog2" ) = NO;

	join_prog_call();

	pmenu = strncmp(mnln_rec.ln_prog_call,"pmenu ",6);
	menu = strncmp(mnln_rec.ln_prog_call,"menu ",5);

	if (!menu || !pmenu)
	{
		FLD( "line_prog2" ) = NA;
		sprintf(local_rec.line_prog2,"%-80.80s"," ");
		DSP_FLD("line_prog2");

		if (!pmenu)
		{
			strcpy(directory,"MENUSYS/SUB_MENU/");
			strcpy(temp_call,"pmenu ");
		}

		if (!menu)
		{
			strcpy(directory,"MENUSYS/");
			strcpy(temp_call,"menu ");
		}

			
		sptr = mnln_rec.ln_prog_call;
		tptr = strrchr(sptr,'/');
		if (!tptr)
			tptr = strchr(sptr,' ');
		if (tptr)
		{
			tptr++;
			sprintf(temp_name,"%-14.14s",tptr);
		}
			
		sprintf(mnln_rec.ln_prog_call,"%s%s%s",temp_call,directory,temp_name);
		split_prog_call();
	}
}

/*-------------------------------------------------
| Set required mncd fields depending on stat_flag |
-------------------------------------------------*/
int
set_req_fields (void)
{
	strcpy(local_rec.cd_reverse,"   ");
	switch (mncd_rec.cd_stat_flag[0])
	{
	case '0':
		strcpy(local_rec.cd_stat_flag,"Quadrant");
		FLD("cd_column") = NI;
		FLD("cd_line") = NI;
		FLD("cd_width") = ND;
		FLD("cd_depth") = ND;
		FLD("cd_line_desc") = ND;
		FLD("cd_reverse") = ND;
		sprintf(mncd_rec.cd_description,"%-80.80s"," ");
		mncd_rec.cd_width = 0;
		mncd_rec.cd_depth = 0;
		mncd_rec.cd_reverse = 0;
		break;
	case '1':
		strcpy(local_rec.cd_stat_flag,"Comment ");
		sprintf(mncd_rec.cd_description,"\042\042%-78.78s"," ");
		FLD("cd_column") = NI;
		FLD("cd_line") = NI;
		FLD("cd_width") = ND;
		FLD("cd_depth") = ND;
		FLD("cd_line_desc") = NI;
		FLD("cd_reverse") = NI;
		mncd_rec.cd_width = 0;
		mncd_rec.cd_depth = 0;
		if (mncd_rec.cd_reverse == 0)
			strcpy(local_rec.cd_reverse,"No ");
		if (mncd_rec.cd_reverse == 1)
			strcpy(local_rec.cd_reverse,"Yes");
		break;
	case '2':
		strcpy(local_rec.cd_stat_flag,"Box     ");
		FLD("cd_column") = NI;
		FLD("cd_line") = NI;
		FLD("cd_width") = NI;
		FLD("cd_depth") = NI;
		FLD("cd_line_desc") = ND;
		FLD("cd_reverse") = ND;
		sprintf(mncd_rec.cd_description,"%-80.80s"," ");
		mncd_rec.cd_reverse = 0;
		break;
	case '3':
		strcpy(local_rec.cd_stat_flag,"Line    ");
		FLD("cd_column") = NI;
		FLD("cd_line") = NI;
		FLD("cd_width") = NI;
		FLD("cd_depth") = ND;
		FLD("cd_line_desc") = ND;
		FLD("cd_reverse") = ND;
		mncd_rec.cd_depth = 0;
		sprintf(mncd_rec.cd_description,"%-80.80s"," ");
		mncd_rec.cd_reverse = 0;
		break;
	default:
		return(1);
	}
	return (EXIT_SUCCESS);
}

/*=======================
| Search for menu_name  |
=======================*/
void
srch_menu (
 char *	key_val)
{
	char	temp_name[18];

        work_open();
	save_rec("#Menu          Sub","#Description");
	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	mnhr_rec.hr_is_sub[0] = 'N';
	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
        while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'N')
    	{                        
		sprintf(temp_name,"%-14.14s[%-1.1s]",mnhr_rec.hr_name,mnhr_rec.hr_is_sub);
	        cc = save_rec(temp_name,mnhr_rec.hr_description); 
		if (cc)
		        break;

		cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");
	}

	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	mnhr_rec.hr_is_sub[0] = 'Y';
	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
        while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'Y')
    	{                        
		sprintf(temp_name,"%-14.14s[%-1.1s]",mnhr_rec.hr_name,mnhr_rec.hr_is_sub);
	        cc = save_rec(temp_name,mnhr_rec.hr_description); 
		if (cc)
		        break;

		cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
	        return;
	sprintf(mnhr_rec.hr_name,"%-14.14s",temp_str);
	sprintf(mnhr_rec.hr_is_sub,"%-1.1s",&temp_str[15]);
	cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"u");
	if (cc)
 	        sys_err("Error in mnhr During (DBFIND)", cc, PNAME);
}

/*----------------------------------------------------
| Expand Menu Characteristics from mnhr to local_rec |
----------------------------------------------------*/
void
hr_to_loc (
 char *	lcl_chars_desc,
 int	add_hr_char)
{
	switch (add_hr_char)
	{
	case 1:
		strcpy(lcl_chars_desc,"Yes   ");
		break;

	case 0:
		strcpy(lcl_chars_desc,"No    ");
		break;

	case -1:
		strcpy(lcl_chars_desc,"Ignore");
		break;

	default:
		break;
	}
}

/*-----------------------------------------------------
| Convert Menu Characteristics from local_rec to mnhr |
-----------------------------------------------------*/
int
loc_to_hr (
 char *	lcl_chars_desc,
 int *	add_hr_char)
{
	switch (*lcl_chars_desc)
	{
	case 'Y':
		strcpy(lcl_chars_desc,"Yes   ");
		*add_hr_char = 1;
		break;

	case 'N':
		strcpy(lcl_chars_desc,"No    ");
		*add_hr_char = 0;
		break;

	case 'I':
		strcpy(lcl_chars_desc,"Ignore");
		*add_hr_char = -1;
		break;

	default:
		return(1);
	}
	return(0);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int	scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (scn == 1)	
		{
			clear();
	
			sprintf(hdng_date,"%10.10s", DateToString(TodaysDate()));

			/*---------------------------------------
			| %s Menu Maintenance ",				|
			|(super_user) ? "Super User" : "Normal" |
			---------------------------------------*/
			sprintf(err_str, (super_user) ? ML(mlMenuMess157) 
										  : ML(mlMenuMess178) );
			rv_pr(err_str,(132 - strlen(err_str)) / 2,0,1);

			rv_pr(hdng_date,121,0,0);
			move(0,1);
			line(132);
		
			move(1,7);
			line(131);
			/*---"MENU CHARACTERISTICS"---*/
			rv_pr( ML(mlMenuMess160),40,7,0);

			box(0,3,132,10); 
			move(0,20);
			line(132);
		}

		if (scn == MNCD_SCN)
		{
			box(0,15,132,4);
			/*---"SCREEN PAINTING SECTION"---*/
			rv_pr( ML(mlMenuMess159),54,15,0);
		}

		if (scn == MNLN_SCN)
		{
			box(0,15,132,4);
			/*---"MENU LINE SECTION"---*/
			rv_pr( ML(mlMenuMess162),57,15,0);
		}
			
		/*---" Company no. : %s   %s"---*/
		sprintf (err_str, ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at(21,0, "%s", err_str);
		move(0,22);
		line(132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
