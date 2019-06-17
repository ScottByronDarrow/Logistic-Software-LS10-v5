/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mn_mnuprt.c   )                                  |
|  Program Desc  : ( Fast Access Printing Program                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  mnhr, mnln,     ,     ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 14/12/90         |
|---------------------------------------------------------------------|
|  Date Modified : 29/01/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 07/06/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 27/11/92        | Modified  by  : Simon Dubey.     |
|  Date Modified : 12/09/97        | Modified  by  : Rowena S Maandig |
|  Date Modified : 31/08/1999      | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      : Stopped program recursing through the parent menu  |
|                : after parent had already been processed.           |
|                :                                                    |
|  (07/06/91)    : Menu security codes can now be up to 8 characters  |
|                : in length so they are now not displayed in this    |
|                : report. New routine Chk_security added to deal     |
|                : with new security codes.                           |
|  (27/11/92)    : IED 8168 sprintf'ing %31.31s to temp_menu[31]      |
|  (12/09/97)    : To incorporate multilingual conversion.            |
|  (31/08/1999)  : Converted to ANSI playform.                        |
|                :                                                    |
|                                                                     |
| $Log: mn_mnuprt.c,v $
| Revision 5.3  2002/07/17 09:57:24  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 05:13:35  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:50  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:11  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:21  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:47:18  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 09:41:57  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/09/30 07:23:41  alvin
| Fixed writeable-string problem.
|
| Revision 1.7  1999/09/29 10:11:09  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:02  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 02:36:52  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: mn_mnuprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mn_mnuprt/mn_mnuprt.c,v 5.3 2002/07/17 09:57:24 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_std_mess.h>		
#include	<ml_menu_mess.h>		


FILE	*fin;
FILE	*fsort;
char	*sptr;
char	usr_fname[100];

int	by_user;

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

	/*================================
	| Menu System Menu Header File   |
	=================================*/
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
	};

	int mnhr_no_fields = 11;

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
		{"mnln_line_space"},
	};

	int	mnln_no_fields = 7;

	struct	{
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

	/*=====================================
	| Menu System Access Description File |
	=====================================*/
	struct dbview mnac_list[] ={
		{"mnac_hhac_hash"},
		{"mnac_code"},
		{"mnac_description"},
	};

	int	mnac_no_fields = 3;

	struct	{
		long	ac_hhac_hash;
		char	ac_code[9];
		char	ac_description[31];
	} mnac_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
int	prt_desc;
int	proc_all;
int	proc_to_end;
int	found = FALSE;
char	sub_hdng[120];
char	mn_disp_line[120];
char	blnk_line[120];
char	undr_line[120];
char	hdng[120];
char	menu_to_find[15];
char	menu_parent[15];
char	sub_parent[2];

struct {
	char    start_user[9];	
	char    end_user[9];	
	char    temp_user[9];	
	char	security[2049];
	char	menu_name[15];
	char	temp_sub[2];
	char	is_sub[2];
	char	menu_recurse[2];
	char	output_to[8];
	char	dummy[11];
	int	lpno;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "start_user", 4, 15, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "Start User: ", "Enter name of first user to print", 
		NO, NO, JUSTLEFT, "", "", local_rec.start_user}, 
	{1, LIN, "end_user", 4, 55, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "      End User: ", "Enter name of last user to print", 
		NO, NO, JUSTLEFT, "", "", local_rec.end_user}, 
	{1, LIN, "out_scr_ptr", 5, 14, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Output to : ", "Enter (S/P) to output to screen or printer", 
		NO, NO, JUSTLEFT, "SP", "", local_rec.output_to}, 
	{1, LIN, "ptr_number", 5, 61, INTTYPE, 
		"NN", "          ", 
		" ", "", "Output to printer number : ", "Enter number of printer to use", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno}, 
	{2, LIN, "menu_name", 4, 10, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Menu Name: ", "Enter name of menu to print", 
		YES, NO, JUSTLEFT, "", "", local_rec.menu_name}, 
	{2, LIN, "is_sub", 4, 55, CHARTYPE, 
		"U", "          ", 
		" ", local_rec.temp_sub, "      Sub Menu: ", "Is this menu a sub menu (Y/N)", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.is_sub}, 
	{2, LIN, "menu_recurse", 5, 18, CHARTYPE, 
		"U", "          ", 
		" ", "", "Include sub-menus: ", "Enter (Y/N) to include or exclude called menus", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.menu_recurse}, 
	{2, LIN, "out_scr_ptr", 6, 14, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Output to : ", "Enter (S/P) to output to screen or printer", 
		NO, NO, JUSTLEFT, "SP", "", local_rec.output_to}, 
	{2, LIN, "ptr_number", 6, 61, INTTYPE, 
		"NN", "          ", 
		" ", "", "Output to printer number : ", "Enter number of printer to use", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=========================== 
| Function prototypes.      |
===========================*/
int		main				(int argc, char * argv []);
void	shutdown_prog		(void);
void	OpenDB				(void);
void	CloseDB			(void);
int		spec_valid			(int field);
void	srch_user			(char * key_val);
void	srch_mnhr			(char * key_val);
void	menu_acc			(void);
int		find_menu			(char * mnu_name, char * sub_mnu);
void	menu_prt			(char * menu_name, char * is_sub);
void	proc_user			(void);
void	user_acc			(void);
void	rd_srt_user_secure	(void);
void	sep_security		(void);
int		compare_security	(void);
void	erase_scrn			(void);
int		heading				(int scn);
int		Chk_security		(char * _secure, char * _security);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	sptr  = getenv("PROG_PATH");

	sprintf(usr_fname,"%s/BIN/MENUSYS/User_secure", (sptr) ? sptr : "/usr/LS10.5");
	strcpy(local_rec.temp_sub,"N");


	by_user = (!strcmp(argv[0],"mn_usrprt"));
	if (!by_user)
	{
		vars[0].scn = 2;	
		vars[1].scn = 2;	
		vars[2].scn = 2;	
		vars[3].scn = 2;	
		vars[4].scn = 1;	
		vars[5].scn = 1;	
		vars[6].scn = 1;	
		vars[7].scn = 1;	
		vars[8].scn = 1;
		vars[3].label = "nullfield";
	}
	else
		vars[8].label = "nullfield";


	SETUP_SCR(vars);
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();                      /*  get into raw mode		*/

	OpenDB();

	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = 0;
		restart = 0;
		init_vars(1);	

		vars[label("ptr_number")].required = YES;

		heading(1);
		entry(1);

		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (!restart) 
		{
			if (by_user)
				user_acc();
			else
				menu_acc();
		}
	}
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

void
OpenDB (void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("mnhr",mnhr_list,mnhr_no_fields,"mnhr_id_no");
	open_rec("mnln",mnln_list,mnln_no_fields,"mnln_id_no");
	open_rec("mnsc",mnsc_list,mnsc_no_fields,"mnsc_id_no");
	open_rec("mnac",mnac_list,mnac_no_fields,"mnac_hhac_hash");
}

void
CloseDB (void)
{
	abc_fclose("mnhr");
	abc_fclose("mnln");
	abc_fclose("mnsc");
	abc_fclose("mnac");
	abc_dbclose("data");
}

int
spec_valid (
 int	field)
{
	if (LCHECK("start_user")) 
	{
		if (last_char == SEARCH)
		{
			srch_user(temp_str);
			return(0);
		}
	
		if (dflt_used)
		{
			vars[label("end_user")].required = NA;
			strcpy(local_rec.start_user,"FIRST   ");
			strcpy(local_rec.end_user,"LAST    ");
			proc_all = TRUE;
			return(0);
		}
		else
		{
			proc_all = FALSE;
			vars[label("end_user")].required = NO;
		}

		clip(local_rec.start_user);
		return(0);
	}

	if (LCHECK("end_user")) 
	{
		if (last_char == SEARCH)
		{
			srch_user(temp_str);
			return(0);
		}

		if (dflt_used)
		{
			strcpy(local_rec.end_user,"LAST    ");
			display_field(label("end_user"));
			proc_to_end = TRUE;
			return(0);
		}
		else
			proc_to_end = FALSE;
			

		clip(local_rec.end_user);

		if (strcmp(local_rec.end_user,local_rec.start_user) < 0)
		{
			strcpy(local_rec.temp_user,clip(local_rec.start_user));
			strcpy(local_rec.start_user,clip(local_rec.end_user));
			strcpy(local_rec.end_user,clip(local_rec.temp_user));
			display_field(label("start_user"));
		}
		return(0);
	}

	if (LCHECK("menu_name")) 
	{
		if (last_char == SEARCH)
		{
			srch_mnhr(temp_str);
			sprintf(local_rec.temp_sub,"%-1.1s",mnhr_rec.hr_is_sub);
			return(0);
		}
		return(0);
	}

	if (LCHECK("is_sub")) 
	{
		sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);

		sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
		cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"r");
		if (cc)
		{
			/* No Such Menu Name */
			print_mess(ML(mlMenuMess078));
			return(1);
		}
		return(0);
	}

	if (LCHECK("out_scr_ptr")) 
	{
		if (local_rec.output_to[0] == 'P')
		{
			strcpy (local_rec.output_to, "Printer");
			vars[label("ptr_number")].required = YES;
			display_prmpt(label("ptr_number"));
		}
		else
		{
			strcpy (local_rec.output_to, "Screen ");
			vars[label("ptr_number")].required = ND;
			rv_pr("                                       ",28,5,0);
		}
		display_field(field);
		return(0);
	}

	if (LCHECK("ptr_number")) 
	{
		if (last_char == SEARCH)
			get_lpno (0);
	
		return(0);
	}
	return(0);
}

void
srch_user (
 char *	key_val)
{
	char	*tptr;
	char	tmpbuf [30];

	/*-----------------------------------
	| Read User_secure file and sort it | 
	-----------------------------------*/
 	fsort = sort_open("secure");

	if ((fin = fopen(usr_fname,"r")) == 0)
		sys_err("Error in User_secure during (FOPEN)",errno,PNAME);

	sptr = fgets(err_str,80,fin);

	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		tptr = strchr (sptr, '<');
		if (tptr)
			*tptr = 0;
		clip (sptr);
		if (!strncmp(sptr,key_val,strlen(key_val))) 
		{
			sprintf (tmpbuf, "%-10.10s\n", sptr);
			sort_save (fsort, tmpbuf);
		}
		sptr = fgets(err_str,80,fin);
	}
	fclose(fin);

	fsort = sort_sort(fsort,"secure");

	sptr = sort_read(fsort);

	work_open();
	save_rec("#User Name","# ");

	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';
		cc = save_rec(sptr," ");
		if (cc)
			break;
		sptr = sort_read(fsort);
	}
	cc = disp_srch();
	work_close();
	sort_delete(fsort,"secure");
}

void
srch_mnhr (
 char *	key_val)
{
	char	temp_name[18];
	char	temp_menu[31];

        work_open();
	save_rec("#Menu          Sub","#Description");

	/*-----------------------------------------
	| Search for menus that are not sub menus |
	-----------------------------------------*/
	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	sprintf (mnhr_rec.hr_is_sub, "%-1.1s", "N");
	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
        while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'N')
    	{                        
		sprintf(temp_name,"%-14.14s[%-1.1s]",mnhr_rec.hr_name,mnhr_rec.hr_is_sub);
		sprintf(temp_menu,"%-30.30s",mnhr_rec.hr_description);
	        cc = save_rec(temp_name,temp_menu); 
		if (cc)
		        break;

		cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");
	}

	/*-------------------------------------
	| Search for menus that are sub menus |
	-------------------------------------*/
	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	sprintf (mnhr_rec.hr_is_sub, "%-1.1s", "Y");
	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
        while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'Y')
    	{                        
		sprintf(temp_name,"%-14.14s[%-1.1s]",mnhr_rec.hr_name,mnhr_rec.hr_is_sub);
		sprintf(temp_menu,"%-30.30s",mnhr_rec.hr_description);
	        cc = save_rec(temp_name,temp_menu); 
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

/*------------------------
| Process by menu access |
------------------------*/
void
menu_acc (void)
{
	if (local_rec.menu_recurse[0] == 'Y')
	{
		sprintf(menu_to_find, "%-14.14s", local_rec.menu_name);
		find_menu("main", "N");
	}

	sprintf(blnk_line,"%-62.62s^F %-8.8s "," "," ");
	strcpy(undr_line,"^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGG^^");

	erase_scrn();

	/* "Print Menu :   %-14.14s   %-9.9s called menus", local_rec.menu_name,
		(local_rec.menu_recurse[0] == 'Y') ? "Including" : "Excluding"
	*/

	if (local_rec.menu_recurse[0] == 'Y')
		sprintf (hdng, ML(mlMenuMess206), local_rec.menu_name);
	else
		sprintf (hdng, ML(mlMenuMess207), local_rec.menu_name);

	rv_pr(hdng,20,2,0);

	/*-----------------------------------
	| Open Dsp Window ready for display | 
	-----------------------------------*/
	if (local_rec.output_to[0] == 'S')
		Dsp_prn_open(0,3,15,hdng,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0);
	else
		Dsp_nd_prn_open(0,3,15,hdng,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0);

	Dsp_saverec(" Menu Entry Description                                         Username   ");
	Dsp_saverec("");
	Dsp_saverec("  [FN03]  [FN05]  [FN14]  [FN15]  [FN16]  ");

	init_mtab();

	if (sub_parent[0] == 'Y')
		sub_menu(menu_parent);
	if (sub_parent[0] == 'N')
		new_menu(menu_parent);

	menu_prt(local_rec.menu_name,local_rec.is_sub);
	
	if (local_rec.output_to[0] == 'S')
		Dsp_srch();
	else
		Dsp_print();

	Dsp_close();
}

/*------------------------------------
| Find menu that the user wishes to  |
| start with. Call new_menu for each |
| menu on the way until required     |
| menu is found but not for required |
| menu.				     |
------------------------------------*/
int
find_menu (
 char *	mnu_name,
 char *	sub_mnu)
{
	int	pmenu;
	int	menu;
	int	lcl_line_no;
	int	do_menu;
	long	lcl_hhmn_hash;
	char	next_menu_name[15];
	char	next_sub[2];
	char	*tptr;

	
	sprintf(mnhr_rec.hr_name, "%-14.14s", mnu_name);
	sprintf(mnhr_rec.hr_is_sub, "%-1.1s", sub_mnu);
	cc = find_rec("mnhr", &mnhr_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	if (!strncmp (mnu_name, menu_to_find, strlen(mnu_name)))
	{
		found = TRUE;
		return (TRUE);
	}

	do_menu = 0;
	if (sub_mnu[0] == 'Y')
	{
		sub_menu(mnu_name);
		do_menu = 1;
	}
	if (sub_mnu[0] == 'N')
	{
		new_menu(mnu_name);
		do_menu = 1;
	}

	if (do_menu)
	{
		lcl_hhmn_hash = mnhr_rec.hr_hhmn_hash;

		mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mnln_rec.ln_line_no = 0;
		cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
		while (!cc && mnln_rec.ln_hhmn_hash == lcl_hhmn_hash && !found)
		{
			/*-------------------------------------
			| Extract menu name from current mnln | 
			-------------------------------------*/
			pmenu = !strncmp(mnln_rec.ln_prog_call,"pmenu ",6);
			menu = !strncmp(mnln_rec.ln_prog_call,"menu ",5);

			if (menu || pmenu)
			{
				if (menu)
					strcpy(next_sub,"N");
				else
					strcpy(next_sub,"Y");
	
				sptr = mnln_rec.ln_prog_call;
				tptr = strrchr(sptr,'/');
				if (!tptr)
					tptr = strchr(sptr,' ');
				sptr = strrchr(tptr,'.');
				if (sptr)
					*sptr = '\0';
				tptr++;
				sprintf(next_menu_name,"%-14.14s",tptr);
				lcl_line_no = mnln_rec.ln_line_no;
				if (find_menu(next_menu_name,next_sub))
				{
					sprintf(menu_parent, "%-14.14s", mnu_name);
					sprintf(sub_parent, "%-1.1s", sub_mnu);
				}

				mnln_rec.ln_hhmn_hash = lcl_hhmn_hash;
				mnln_rec.ln_line_no = lcl_line_no;
				cc = find_rec("mnln",&mnln_rec,COMPARISON,"r");
				if (cc)
					sys_err("Error in mnln during (DBFIND)",cc,PNAME);
			}
			cc = find_rec("mnln",&mnln_rec,NEXT,"r");
		}
	}
	return(FALSE);
}

/*-----------------------------------------------------------------
| Process a menu (This routine is called recursively if required) |
-----------------------------------------------------------------*/
void
menu_prt (
 char *	menu_name,
 char *	is_sub)
{
	int	pmenu;
	int	menu;
	int	do_menu;
	int	lcl_line_no;
	long	lcl_hhmn_hash;
	char	next_menu_name[15];
	char	next_sub[2];
	char	*tptr;

	do_menu = 0;
	if (is_sub[0] == 'Y')
	{
		sub_menu(menu_name);
		do_menu = 1;
	}
	if (is_sub[0] == 'N')
	{
		new_menu(menu_name);
		do_menu = 1;
	}

	if (do_menu)
	{
		sprintf(mnhr_rec.hr_name,"%-14.14s",menu_name);
		sprintf(mnhr_rec.hr_is_sub,"%-1.1s",is_sub);
		cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"r");
		if (cc)
		{
			sprintf(sub_hdng,
				"^1 MENU :  %-14.14s does not exist in database^6%-12.12s^F %-8.8s",
				menu_name,
				" ",
				" ");
			Dsp_saverec(sub_hdng);
			Dsp_saverec(undr_line);
			return;
		}
		sprintf(sub_hdng,
			"^1 MENU :  %-14.14s    (%-6.6s) ^6%-26.26s^F %-8.8s ",
			menu_name,
			mnhr_rec.hr_fast_access,
			" ",
			" ");

		Dsp_saverec(sub_hdng);
			
		/*-------------------------------------------
		| Process all menu lines for current header |
		-------------------------------------------*/
		mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mnln_rec.ln_line_no = 0;
		cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
		while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
		{
			rd_srt_user_secure();
			prt_desc = TRUE;

			sptr = sort_read(fsort);
			while (sptr)
			{
				proc_user();

				sptr = sort_read(fsort);
			}
			Dsp_saverec(blnk_line);
			cc = find_rec("mnln",&mnln_rec,NEXT,"r");
		}
		Dsp_saverec(undr_line);
	
		/*---------------------------------
		| Process recursively if required |
		---------------------------------*/
		lcl_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		if (local_rec.menu_recurse[0] == 'Y')
		{
			mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
			mnln_rec.ln_line_no = 0;
			cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
			while (!cc && mnln_rec.ln_hhmn_hash == lcl_hhmn_hash)
			{
				/*-------------------------------------
				| Extract menu name from current mnln | 
				-------------------------------------*/
				pmenu = !strncmp(mnln_rec.ln_prog_call,"pmenu ",6);
				menu = !strncmp(mnln_rec.ln_prog_call,"menu ",5);

				if (menu || pmenu)
				{
					if (menu)
						strcpy(next_sub,"N");
					else
						strcpy(next_sub,"Y");
	
					sptr = mnln_rec.ln_prog_call;
					tptr = strrchr(sptr,'/');
					if (!tptr)
						tptr = strchr(sptr,' ');
					if (tptr)
					{
						sptr = strrchr(tptr,'.');
						if (sptr)
							*sptr = '\0';
						tptr++;
						sprintf(next_menu_name,"%-14.14s",tptr);
						lcl_line_no = mnln_rec.ln_line_no;
						menu_prt(next_menu_name,next_sub);
						mnln_rec.ln_hhmn_hash = lcl_hhmn_hash;
						mnln_rec.ln_line_no = lcl_line_no;
						cc = find_rec("mnln",&mnln_rec,COMPARISON,"r");
						if (cc)
							sys_err("Error in mnln during (DBFIND)",cc,PNAME);
					}
				}
				cc = find_rec("mnln",&mnln_rec,NEXT,"r");
			}
		}
	}
}


/*--------------------------------
| Process a user for menu access |
--------------------------------*/
void
proc_user (void)
{
	sep_security();
	if (compare_security ())
	{
		if (!prt_desc)
			sprintf(mnln_rec.ln_description,"%-60.60s"," ");
		sprintf(mn_disp_line," %-60.60s ^F %-8.8s ",mnln_rec.ln_description,local_rec.temp_user);
		Dsp_saverec(mn_disp_line);
		prt_desc = FALSE;
	}
}

/*------------------------
| Process by user access |
------------------------*/
void
user_acc (void)
{
	sprintf(blnk_line,"%-62.62s"," ");
	strcpy(undr_line,"^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");
	/*----------------------------------
	| Display message at top of screen | 
	----------------------------------*/
	erase_scrn();

	/*%s   %-8.8s         %s    %-8.8s","Print from :",local_rec.start_user,"to:",local_rec.end_user*/
	sprintf(hdng,ML(mlMenuMess082),local_rec.start_user,local_rec.end_user);
	rv_pr(hdng,20,2,0);

	/*-----------------------------------
	| Open Dsp Window ready for display | 
	-----------------------------------*/
	if (local_rec.output_to[0] == 'S')
		Dsp_prn_open(10,3,15,hdng,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0);
	else
		Dsp_nd_prn_open(10,3,15,hdng,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0,(char *) 0);

	Dsp_saverec(" Menu Entry Description                                       ");
	Dsp_saverec("");
	Dsp_saverec("  [FN03]  [FN05]  [FN14]  [FN15]  [FN16]  ");

	rd_srt_user_secure();

	sptr = sort_read(fsort);
	clip (local_rec.start_user);
	clip (local_rec.end_user);

	/*---------------------------------
	| Get first user and their access | 
	---------------------------------*/
	if (proc_all || proc_to_end )
		strcpy(local_rec.end_user,"~~~~~~~~");

	if (!proc_all )
		while (sptr && strcmp(sptr,local_rec.start_user) < 0)
			sptr = sort_read(fsort);

	/*----------------------------
	| Process all required users |
	----------------------------*/
	do
	{
		sep_security();

		/*---------------------------------
		| Display Current User and access |
		---------------------------------*/
		sprintf(sub_hdng,"^1%s  %-8.8s^6%-38.38s","CURRENT USER :",local_rec.temp_user," ");
		Dsp_saverec(sub_hdng);
		
		/*-----------------------
		| Read all mnhr records | 
		-----------------------*/
		strcpy(mnhr_rec.hr_is_sub,"N");
		sprintf(mnhr_rec.hr_name,"%-14.14s"," ");
		cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
		while (!cc)
		{
			sprintf(mn_disp_line," MENU : %-14.14s                                        ",mnhr_rec.hr_name);

			Dsp_saverec(mn_disp_line);
			Dsp_saverec(blnk_line);

			/*-----------------------------------
			| Read all associated mnln records  |
			-----------------------------------*/
			mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
			mnln_rec.ln_line_no = 0;
			cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
			while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
			{
				if (compare_security ())
				{
					sprintf(mn_disp_line," %-60.60s ",mnln_rec.ln_description);
					Dsp_saverec(mn_disp_line);
				}

				cc = find_rec("mnln",&mnln_rec,NEXT,"r");
			}
			cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");

			if (!cc)
				Dsp_saverec(undr_line);
		}

		if (strcmp(local_rec.start_user,local_rec.end_user))
			sptr = sort_read(fsort);

		if (sptr && strcmp(local_rec.end_user, clip (local_rec.temp_user)) > 0)
		{
			Dsp_saverec(undr_line);
			Dsp_saverec(blnk_line);
		}

	} while (sptr && strcmp(local_rec.end_user, clip (local_rec.temp_user)) > 0);
	Dsp_saverec(undr_line);

	sort_delete(fsort,"secure");

	if (local_rec.output_to[0] == 'S')
		Dsp_srch();
	else
		Dsp_print();

	Dsp_close();
}

/*-----------------------------------
| Read User_secure file and sort it | 
-----------------------------------*/
void
rd_srt_user_secure (void)
{
	char	tmpbuf [2048 + 1];

 	fsort = sort_open("secure");

	if ((fin = fopen(usr_fname,"r")) == 0)
		sys_err("Error in User_secure during (FOPEN)",errno,PNAME);

	sptr = fgets(err_str,2048,fin);

	while (sptr != (char *)0)
	{
		*(sptr + (strlen (sptr) - 1)) = 0;
		sprintf (tmpbuf, "%-.2048s\n", sptr);
		sort_save (fsort, tmpbuf);
		sptr = fgets (err_str, 2048, fin);
	}
	fclose(fin);

	fsort = sort_sort(fsort,"secure");
}

/*---------------------------------
| Separate user name and security | 
---------------------------------*/
void
sep_security (void)
{
	char	*tptr;

	tptr = sptr;
	while (*tptr && *tptr != '<')
		tptr++;

	if (*tptr)
	{
		*tptr = '\0';
		sprintf(local_rec.temp_user,"%-8.8s",sptr);

		tptr++;
		sptr = tptr;
		while (*tptr && *tptr != '>')
			tptr++;
		*tptr = '\0';
		if (sptr != tptr)
			sprintf(local_rec.security,"%-.2048s",sptr);
	}
	else
	{
		sprintf(local_rec.temp_user,"%-8.8s",sptr);
		sprintf(local_rec.security,"%-.2048s"," ");
	}
}

/*---------------------------------------------------------------
| Compare security in menu entry line with User_secure security |
---------------------------------------------------------------*/
int
compare_security (void)
{
	mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
	mnsc_rec.sc_hhac_hash = 0L;
	cc = find_rec("mnsc", &mnsc_rec, GTEQ, "r");
	while (!cc && mnsc_rec.sc_hhln_hash == mnln_rec.ln_hhln_hash)
	{
		cc = find_hash("mnac", &mnac_rec, COMPARISON, "r", mnsc_rec.sc_hhac_hash);
		if (cc)
		{
			cc = find_rec("mnsc", &mnsc_rec, NEXT, "r");
			continue;
		}
		
		if (Chk_security(clip(mnac_rec.ac_code), local_rec.security))
			return(TRUE);

		cc = find_rec("mnsc", &mnsc_rec, NEXT, "r");
	}

	return(FALSE);
}

void
erase_scrn (void)
{
	int	i;

	for (i = 3; i < 8; i++)
	{
		move(0,i);
		cl_line();
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
	
		strcpy(hdng_date,DateToString(TodaysDate()));
		rv_pr(hdng_date,71,0,0);

		move(0,1);
		line(80);

		if (by_user)
		{
			/* Menu Access By User. */
			rv_pr(ML(mlMenuMess079),25,0,1);
			box(0,3,80,2);
		}
		else
		{
			/*Menu Access By Menu. */
			rv_pr(ML(mlMenuMess080),25,0,1);
			box(0,3,80,3);
		}


		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

/*=======================================
| Check if user has access to menu line	|
| returns TRUE iff access permitted	|
=======================================*/
int
Chk_security (
 char *	_secure,		/* security on menu		*/
 char *_security)		/* security on user		*/
{
	char	*rptr;
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
	if ((rptr = strchr(_security,'*')))
		return(TRUE);
	
	/*-------------------------------
	| Access to all on menu option	|
	-------------------------------*/
	if ((rptr = strchr(_secure,'*')))
		return(TRUE);
	
	/*-----------------------------------------------
	| Check Security for each security group	|
	| that user belongs to.				|
	-----------------------------------------------*/	
	rptr = strdup(_security);
	while (*rptr)
	{
		/*----------------
		| Find separator |
		----------------*/
		tptr = rptr;
		while (*tptr && *tptr != '|')
			tptr++;

		usr_char = *tptr;

		*tptr = '\0';
		strcpy(tmp_usr_sec, rptr);

		if (usr_char)
			rptr = tptr + 1;
		else
			*rptr = '\0';

		uptr = strdup(_secure);
		while (*uptr)
		{
			/*----------------
			| Find separator |
			----------------*/
			vptr = uptr;
			while (*vptr && *vptr != '|')
				vptr++;

			mnu_char = *vptr;

			*vptr = '\0';
			strcpy(tmp_mnu_sec, uptr);

			if (mnu_char)
				uptr = vptr + 1;
			else
				*uptr = '\0';

			if (!strcmp(tmp_usr_sec, tmp_mnu_sec))
				return(1);
		}
	}
	return(0);
}
