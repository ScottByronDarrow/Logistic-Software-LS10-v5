/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mn_store.c    )                                  |
|  Program Desc  : ( Menu Creation Program                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  mnhr,mnln ,mncd ,mnac ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 29/04/88         |
|---------------------------------------------------------------------|
|  Date Modified : (29/04/88)      | Modified  by  : Roger Gibbison.  |
|                : (27/11/90)                      : Campbell Mander. |
|                : (27/05/91)                      : Campbell Mander. |
|                : (10/04/94)                      : Roel Michels   . |
|                : (05/09/97)                      : Roanna Marcelino.|
|                : (03/09/1999)                    : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (27/11/90)     Now interprets and creates screen   |
|                :  painting section and quadrant and line spacing    |
|                :  details after security access code in .mdf files  |
|  (27/05/91)    : Security now up to 8 characters long. New link to  |
|                : mn_store called mn_convert and mn_conv_all which   |
|                : convert from old format .mdf files to database.    |
|  (10/04/94)    : PSL 10673 - Online conversion                      |
|  (05/09/97)    : Modified for Multilingual Conversion.              |
|  (03/09/1999)  : Ported to ANSI standards.                          |
|                                                                     |
| $Log: mn_store.c,v $
| Revision 5.2  2001/08/09 05:13:36  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/04/26 09:45:19  cha
| *** empty log message ***
|
| Revision 4.1  2001/04/26 09:44:16  cha
| Updated to solve the issue of blank menu description being created.
|
| Revision 4.0  2001/03/09 02:29:51  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:12  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:22  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/02/18 01:56:24  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.11  1999/12/06 01:47:19  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 09:41:57  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.9  1999/09/29 10:11:09  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:02  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/07/19 10:59:23  scott
| Updated for abc_delete
|
| Revision 1.5  1999/06/15 02:36:52  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: mn_store.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mn_store/mn_store.c,v 5.2 2001/08/09 05:13:36 scott Exp $";

#include <pslscr.h>		
#include <dsp_screen.h>		
#include <dsp_process2.h>		
#include <ml_std_mess.h>		
#include <ml_menu_mess.h>		

FILE *	mdf;
FILE *	mdfs;


char	directory [100];
char	filename [100];

	/*================================
	| Menu System Menu Header File   |
	=================================*/
	struct dbview mnhr_list [] = {
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
		char	hr_name [15];
		char	hr_description [61];
		char	hr_help [15];
		char	hr_fast_access [7];
		long	hr_hhmn_hash;
		char	hr_is_sub [2];
		int	hr_heading;
		int	hr_trailer;
		int	hr_fast;
		int	hr_sub;
		int	hr_menu_name;
		int	hr_shell_out;
	} mnhr_rec;
	
	/*============================
	|Menu System Menu Line File  |
	============================*/
	struct dbview mnln_list [] = {
		{"mnln_hhmn_hash"},
		{"mnln_hhln_hash"},
		{"mnln_line_no"},
		{"mnln_description"},
		{"mnln_prog_call"},
		{"mnln_quad"},
		{"mnln_line_space"}
	};

	int mnln_no_fields = 7;

	struct {
		long	ln_hhmn_hash;
		long	ln_hhln_hash;
		int	ln_line_no;
		char	ln_description [61];
		char	ln_prog_call [161];
		int	ln_quad;
		int	ln_line_space;
	} mnln_rec;

	/*================================
	| Menu System Security Link File |
	================================*/
	struct dbview mnsc_list [] = {
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
	struct dbview mncd_list [] = {
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

	struct {
		long	cd_hhmn_hash;
		int	cd_line_no;
		char	cd_description [81];
		int	cd_line;
		int	cd_column;
		int	cd_width;
		int	cd_depth;
		int	cd_reverse;
		char	cd_stat_flag [2];
	} mncd_rec, mncd_rec2;
	/*=====================================
	| Menu System Access Description File |
	=====================================*/
	struct dbview mnac_list [] = {
		{"mnac_hhac_hash"},
		{"mnac_code"},
		{"mnac_description"},
	};

	int	mnac_no_fields = 3;

	struct	{
		long	ac_hhac_hash;
		char	ac_code [9];
		char	ac_description [31];
	} mnac_rec;


/*============================ 
| Local & Screen Structures. |
============================*/
int	createall = 0;
int	updated = 0;
int	store;
int	convert;

struct {
	char    acc_security [28];	
	char    sec_master [28];	
	char	dummy [11];
	char	menu_name [15];
	char	is_sub [2];
	char	temp_sub [2];
} local_rec;

static	struct	var	vars	[]	=
{
	{1, LIN, "mn_name", 4, 15, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "Menu Name :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.menu_name}, 
	{1, LIN, "is_sub", 5, 15, CHARTYPE, 
		"U", "          ", 
		" ", local_rec.temp_sub, "Sub Menu :", " ", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.is_sub}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*============================
| Local function prototypes  |
============================*/
void	shutdown_prog			(void);
void	OpenDB					(void);
void	CloseDB					(void);
int		spec_valid				(int);
void	srch_mdf				(char *);
void	get_srch				(char *);
void	srch_mnhr				(char *);
void	init_database			(void);
void	process					(void);
void	create_all				(void);
void	store_all				(void);
void	store_mdf				(void);
int		proc_new_sec			(char *);
int		proc_old_sec			(char *);
void	proc_quadrant			(char *);
void	update_mnhr				(void);
void	proc_screen_painting	(void);
int		quadrant_param			(char *, int);
int		comment_param			(char *, int);
int		box_param				(char *, int);
int		line_param				(char *, int);
int		update_mncd				(int);
void	creat_mdf				(void);
int		heading					(int);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr = getenv ("PROG_PATH");

	if (argc != 2)
		sprintf (directory, "%s/BIN/MENUSYS", (sptr) ? sptr : "/usr/LS10.5");
	else
		strcpy (directory, argv[1]);

	store = (!strcmp (argv [0], "mn_store"));
	convert = (!strcmp (argv [0], "mn_convert") || !strcmp (argv [0], "mn_conv_all"));

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/

	OpenDB ();

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	strcpy (local_rec.temp_sub, "N");
	strcpy (local_rec.sec_master, "0123456789abcdefghijklmnopqrstuvwxyz*");

	/*-------------------------------
	| Store All mdf's in directory	|
	-------------------------------*/
	if (!strcmp(argv[0],"mn_storeall") || !strcmp(argv[0],"mn_conv_all"))
	{
		init_database ();
		strcpy(local_rec.is_sub,"N");
		store_all ();
		strcpy(local_rec.is_sub,"Y");
		sprintf(directory,"%s/SUB_MENU",directory);
		store_all ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	if (!strcmp(argv[0],"mn_createall"))
	{
		createall = 1;
		sprintf(local_rec.is_sub, "%-1.1s", "N");
		create_all ();
		sprintf(local_rec.is_sub, "%-1.1s", "Y");
		sprintf(directory,"%s/SUB_MENU",directory);
		create_all ();
		shutdown_prog();
		return (EXIT_SUCCESS);
	}

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

		heading (1);
		entry(1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display(1);
		edit(1);
		if (!restart) 
			process();
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

void
OpenDB (
 void)
{
	abc_dbopen("data");
	open_rec("mnln",mnln_list,mnln_no_fields,"mnln_id_no");
	open_rec("mnsc",mnsc_list,mnsc_no_fields,"mnsc_id_no");
	open_rec("mnhr",mnhr_list,mnhr_no_fields,"mnhr_id_no");
	open_rec("mnac",mnac_list,mnac_no_fields,"mnac_code");
	open_rec("mncd",mncd_list,mncd_no_fields,"mncd_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose("mnhr");
	abc_fclose("mnln");
	abc_fclose("mnsc");
	abc_fclose("mnac");
	abc_fclose("mncd");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("mn_name")) 
	{
		if (SRCH_KEY)
		{
			if (store || convert)
				srch_mdf (temp_str);
			else
			{
				srch_mnhr (temp_str);
				sprintf (local_rec.temp_sub, "%-1.1s", mnhr_rec.hr_is_sub);
			}
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("is_sub")) 
	{
		if (local_rec.is_sub[0] == 'Y')
			sprintf(filename,"%s/SUB_MENU/%s.mdf",directory,clip(local_rec.menu_name));
		else
			sprintf(filename,"%s/%s.mdf",directory,clip(local_rec.menu_name));
		sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);

		if (store || convert)
		{

			if (access(filename,00) == 0)
				return (EXIT_SUCCESS);

			sprintf(err_str,ML(mlMenuMess116),local_rec.menu_name);
			print_mess(err_str);
			return (EXIT_FAILURE);
		}

		sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
		sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);
		cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlMenuMess078));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
srch_mdf (
 char *	key_val)
{

	work_open ();
	save_rec ("#Menu Name     Sub","#");

	sprintf (filename,"ls %s/%s*.mdf 2> /dev/null",directory,key_val);
	strcpy (local_rec.temp_sub,"[N]");
	get_srch (key_val);
	
	sprintf (filename,"ls %s/SUB_MENU/%s*.mdf 2> /dev/null",directory,key_val);
	strcpy (local_rec.temp_sub,"[Y]");
	get_srch (key_val);

	cc = disp_srch ();
	sprintf (local_rec.temp_sub,"%-1.1s",&temp_str[15]);
	work_close ();
}

void
get_srch (
 char *	key_val)
{
	char	*fgets(char *, int, FILE *);
	char	*sptr;
	char	*tptr;

	if ((mdfs = popen(filename,"r")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}
		
	sptr = fgets(err_str,80,mdfs);

	while (sptr)
	{
		tptr = sptr + strlen(sptr);

		while (tptr > sptr && *tptr != '.')
			tptr--;

		if (*tptr == '.')
			*tptr = '\0';

		while (tptr > sptr && *tptr != '/')
			tptr--;

		if (*tptr == '/')
			tptr++;

		sprintf(filename,"%-14.14s%s",tptr,local_rec.temp_sub);

		cc = save_rec(filename," ");
		if (cc)
			break;

		sptr = fgets(err_str,80,mdfs);
	}
	pclose (mdfs);
}

void
srch_mnhr (
 char *	key_val)
{
	char	temp_desc[51];
	char	temp_sub[4];
	char	temp_name[18];

	work_open();
	save_rec("#Menu Name     Sub","#Menu Description");
	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	strcpy(mnhr_rec.hr_is_sub,"N");
	strcpy(temp_sub,"[N]");

	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");

	while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'N')
	{
		sprintf(temp_desc,"%-50.50s",mnhr_rec.hr_description);
		sprintf(temp_name,"%s%s",mnhr_rec.hr_name,temp_sub);
		cc = save_rec(temp_name,temp_desc);
		if (cc)
			break;

		cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");
	}

	sprintf(mnhr_rec.hr_name,"%-14.14s",key_val);
	strcpy(mnhr_rec.hr_is_sub,"Y");
	strcpy(temp_sub,"[Y]");

	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");

	while (!cc && !strncmp(mnhr_rec.hr_name,key_val,strlen(key_val)) && mnhr_rec.hr_is_sub[0] == 'Y')
	{
		sprintf(temp_desc,"%-50.50s",mnhr_rec.hr_description);
		sprintf(temp_name,"%s%s",mnhr_rec.hr_name,temp_sub);
		cc = save_rec(temp_name,temp_desc);
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

	cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in mnhr during (DBFIND)",cc,PNAME);
}

/*------------------------------------------
| Purge database ready for new information |
| NB Only used for storeall                |
------------------------------------------*/
void
init_database (
 void)
{

	clear();
	move(0,0);
	print_at(0,0,ML(mlStdMess035));
	fflush(stdout);

	/*-------------------------
	| Remove all mnhr records |
	-------------------------*/
	sprintf(mnhr_rec.hr_name,"%-14.14s"," ");
	sprintf(mnhr_rec.hr_is_sub,"%-1.1s"," ");
	cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
	while (!cc)
	{
		cc = abc_delete("mnhr");
		if (cc)
			sys_err("Error in mnhr during (DBUPDATE)",cc,PNAME);

		cc = find_rec("mnhr",&mnhr_rec,GTEQ,"r");
	}

	/*-------------------------
	| Remove all mnln records |
	-------------------------*/
	mnln_rec.ln_hhmn_hash = 0L;
	mnln_rec.ln_line_no = 0;
	cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
	while (!cc)
	{
		cc = abc_delete("mnln");
		if (cc)
			sys_err("Error in mnln during (DBUPDATE)",cc,PNAME);

		cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
	}

	/*-------------------------
	| Remove all mncd records |
	-------------------------*/
	mncd_rec.cd_hhmn_hash = 0L;
	mncd_rec.cd_line_no = 0;
	cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	while (!cc)
	{
		cc = abc_delete("mncd");
		if (cc)
			sys_err("Error in mncd during (DBUPDATE)",cc,PNAME);

		cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	}
}

void
process (
 void)
{
	char *	sptr;

	sptr = clip(local_rec.menu_name);

	if ((mdf = fopen(filename,(store || convert) ? "r" : "w")) == 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}


	if (store || convert)
	{
		dsp_screen(""," 1","Logistic Menu Maintenance");
		store_mdf ();
	}
	else
		creat_mdf ();

	fclose(mdf);
}

void
create_all (
 void)
{
	sprintf (mnhr_rec.hr_name, "%-14.14s", " ");
	sprintf (mnhr_rec.hr_is_sub, "%-1.1s", local_rec.is_sub);
	cc = find_rec ("mnhr", &mnhr_rec, GTEQ, "r");
	if (cc || strcmp (mnhr_rec.hr_is_sub, local_rec.is_sub))
		return;

	dsp_screen(""," 1","Logistic Menu Maintenance");

	while (!cc && !(strncmp(mnhr_rec.hr_is_sub,local_rec.is_sub,1)))
	{
		sprintf(local_rec.menu_name,"%-14.14s",mnhr_rec.hr_name);
		sprintf(filename,"%s/%s.mdf",directory,clip(local_rec.menu_name));

		if ((mdf = fopen(filename,"w")) == 0)
		{
			sprintf(err_str,"Error in %s during (FOPEN)",filename);
			sys_err(err_str,errno,PNAME);
		}

		creat_mdf ();	
		fclose(mdf);
		cc = find_rec("mnhr",&mnhr_rec,NEXT,"r");
	}
}

void
store_all (
 void)
{
	char *	sptr;
	char *	tptr;

	dsp_screen (""," 1","Logistic Menu Maintenance");

	sprintf (filename,"ls %s/*.mdf 2> /dev/null",directory);

	if ((mdfs = popen (filename,"r")) == 0)
	{
		sprintf (err_str,"Error in %s during (POPEN)",filename);
		sys_err (err_str,errno,PNAME);
	}
		
	sptr = fgets (err_str,80,mdfs);

	while (sptr)
	{
		tptr = sptr + strlen(sptr);

		while (tptr > sptr && *tptr != '.')
			tptr--;

		if (*tptr == '.')
			*tptr = '\0';

		while (tptr > sptr && *tptr != '/')
			tptr--;

		if (*tptr == '/')
			tptr++;

		strcpy(local_rec.menu_name,tptr);

		sprintf(filename,"%s/%s.mdf",directory,local_rec.menu_name);

		if ((mdf = fopen(filename,"r")) == 0)
		{
			sprintf(err_str,"Error in %s during (FOPEN)",filename);
			sys_err(err_str,errno,PNAME);
		}

		store_mdf ();

		fclose(mdf);

		sptr = fgets(err_str,80,mdfs);
	}
	pclose(mdfs);
}

/*---------------------------------------
| If status is TRUE then program is to	|
| convert from old security format to	|
| new security format					|
---------------------------------------*/
void
store_mdf (
 void)
{
	int	indx;
	int	sect;
	int	line_no = 0;
	char	*sptr;
	char	*tptr;
	char	*fgets(char *, int, FILE *);
	char	data[161];

	updated = 0;

	rv_pr("                            ",30,10,0);
	/*Reading %s.mdf */
	rv_pr(ML(mlStdMess035),30,10,0);

	sptr = fgets(data,161,mdf);
	if (!sptr)
		updated = 1;

	while (sptr != (char *) 0)
	{

		if (sptr[0] == '(' && sptr[1] == '(')
		{
			proc_screen_painting ();
			break;
		}

		tptr = sptr + strlen(sptr);
		if (strlen(sptr))
			tptr--;

		if (*tptr == '\n')
			*tptr = '\0';

		switch (line_no)
		{
		/*-----------------------
		| Menu Description	|
		-----------------------*/
		case	0:
			sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
			sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);
			cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"u");

			if (cc)
			{
				sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
				sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);
				sprintf(mnhr_rec.hr_description,"%-60.60s"," ");
				sprintf(mnhr_rec.hr_help,"%-14.14s"," ");
				sprintf(mnhr_rec.hr_fast_access,"%-6.6s"," ");
				mnhr_rec.hr_heading = -1;
				mnhr_rec.hr_trailer = -1;
				mnhr_rec.hr_fast = -1;
				mnhr_rec.hr_sub = -1;
				mnhr_rec.hr_menu_name = -1;
				mnhr_rec.hr_shell_out = -1;

				cc = abc_add("mnhr",&mnhr_rec);
				if (cc)
					sys_err("Error in mnhr during (DBADD)",cc,PNAME);
				sprintf(mnhr_rec.hr_name,"%-14.14s",local_rec.menu_name);
				sprintf(mnhr_rec.hr_is_sub,"%-1.1s",local_rec.is_sub);
				cc = find_rec("mnhr",&mnhr_rec,COMPARISON,"u");
				if (cc)
					sys_err("Error in mnhr during (DBFIND)",cc,PNAME);
			}

			/*----------------------------------------------------
			|Delete existing lines associated with current header|
			|record		             		             |
			----------------------------------------------------*/
			mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
			mnln_rec.ln_line_no = 1;
			cc = find_rec("mnln",&mnln_rec,GTEQ,"r");

			while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
			{
				cc = abc_delete("mnln");
				if (cc)
					sys_err("Error in mnln during (DBUPDATE)",cc,PNAME);

				cc = find_rec("mnln",&mnln_rec,GTEQ,"r");
			}

			/*---------------------------------------
			| Look Along Line for Fast Access	|
			---------------------------------------*/
			while (*sptr && *sptr != '(')
				sptr++;

			if (*sptr)
			{
				tptr = sptr;
				while (*tptr && *tptr != ')')
					tptr++;

				*sptr = '\0';
				*tptr = '\0';
		
				if (sptr != tptr)
					sptr++;
			}
				
			/*-----------------------------------------------
			| Description is the first part of the line	|
			-----------------------------------------------*/
			sprintf(mnhr_rec.hr_description,"%-60.60s",data);

			/*-----------------------
			| Then fast_access	|
			-----------------------*/
			sprintf(mnhr_rec.hr_fast_access,"%-6.6s",sptr);
			break;

		/*---------------
		| Help File	|
		---------------*/
		case	1:
			sprintf(mnhr_rec.hr_help,"%-14.14s",sptr);
			break;

		/*---------------
		| Menu Lines	|
		---------------*/
		default:
			sect = line_no % 2;
			indx = line_no / 2;

			if (sect == 0)
			{
				sprintf(err_str,"%2d",indx);
				dsp_process(" Line : ",err_str);

				mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
				mnln_rec.ln_line_no = indx;

				cc = find_rec("mnln",&mnln_rec,COMPARISON,"r");
				if (cc)
				{
					mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
					mnln_rec.ln_line_no = indx;
					mnln_rec.ln_quad = 1;
					mnln_rec.ln_line_space = 1;
					cc = abc_add("mnln",&mnln_rec);
					if (cc)
						sys_err("Error in mnhr during (DBADD)",cc,PNAME);
					cc = find_rec("mnln",&mnln_rec,COMPARISON,"r");
					if (cc)
						sys_err("Error in mnhr during (DBFIND)",cc,PNAME);
				}

				/*---------------------------------------
				| Look Along Line for Security Start	|
				---------------------------------------*/
				while (*sptr && *sptr != '<')
					sptr++;

				if (!*sptr)
					sprintf(mnln_rec.ln_description,"%-60.60s",data);
				else
				{
					if (convert)
						proc_old_sec (sptr);
					else
						proc_new_sec (sptr);

					sprintf(mnln_rec.ln_description,"%-60.60s",data);
				}

			}
			else
				sprintf(mnln_rec.ln_prog_call,"%-160.160s",data);
			cc = abc_update("mnln",&mnln_rec);
			if (cc)
				sys_err("Error in mnln during (DBUPDATE)",cc,PNAME);
			abc_unlock("mnln");

			break;
		}
		line_no++;

		sptr = fgets(data,161,mdf);
	}

	if (!updated)
		update_mnhr ();

	indx = line_no / 2;
}

int
proc_new_sec (
 char *	sptr)
{
	char	*tptr;
	char	*uptr = (char *) 0;
	char	temp_char;
	int	lcl_cc;
	int	first_time = TRUE;
	int	last_code = FALSE;

	tptr = sptr;

	/*----------------------------------
	| Look Along Line for Security End |
	----------------------------------*/
	while (*tptr && *tptr != '>')
		tptr++;

	if (*tptr)
		uptr = tptr;

	while (*sptr)
	{
		if (first_time)
		{
			*sptr = '\0';
			first_time = FALSE;
		}

		if (last_code)
			break;

		tptr = sptr + 1;
		while (*tptr && *tptr != '>' && *tptr != '|')
			tptr++;

		if (*tptr)
		{
			if (*tptr == '>')
				last_code = TRUE;

			temp_char = *tptr;
			*tptr = '\0';
			sprintf(mnac_rec.ac_code, "%-8.8s", (*(sptr + 1)) ? sptr + 1 : "ERROR");

			/*----------------------------------
			| Check mnac file; add if required |
			----------------------------------*/
			lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
			{
				sprintf(mnac_rec.ac_description, "%-30.30s", " ");
				lcl_cc = abc_add("mnac", &mnac_rec);
				if (lcl_cc)
					sys_err("Error in mnac during (DBADD)",lcl_cc,PNAME);
			}

			/*-----------------
			| Add mnsc record |
			-----------------*/
			sprintf (mnac_rec.ac_code, "%-8.8s", (*(sptr + 1)) ? sptr + 1 : "ERROR");
			lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
			       sys_err("Error in mnac during (DBFIND)",lcl_cc,PNAME);

			mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
			mnsc_rec.sc_hhac_hash = mnac_rec.ac_hhac_hash;
			lcl_cc = abc_add("mnsc", &mnsc_rec);
			if (lcl_cc)
			       sys_err("Error in mnsc during (DBADD)",lcl_cc,PNAME);
	
			*tptr = temp_char;
			sptr = tptr;
		}
	}

	if (*uptr)
	{
		sptr = uptr + 1;	
		proc_quadrant (sptr);
		return (EXIT_SUCCESS);
	}

	return (EXIT_FAILURE);
}

int
proc_old_sec (
 char *	sptr)
{
	char *	tptr;
	char	temp_char;
	int		lcl_cc;

	tptr = sptr + 1;

	/*----------------------------------
	| Look Along Line for Security End |
	----------------------------------*/
	while (*tptr && *tptr != '>')
	{
		if (strchr(local_rec.sec_master, *tptr))
			tptr++;
		else
			break;
	}

	*sptr = '\0';
	temp_char = *tptr;
	*tptr = '\0';

	if (sptr != tptr)
	{
		sptr++;
		while (*sptr)
		{
			/*----------------------------------
			| Check mnac file; add if required |
			----------------------------------*/
			sprintf (mnac_rec.ac_code, "%c%-7.7s", *sptr, " ");
			lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
			{
				sprintf(mnac_rec.ac_description, "%-30.30s", " ");
				lcl_cc = abc_add("mnac", &mnac_rec);
				if (lcl_cc)
					sys_err("Error in mnac during (DBADD)",lcl_cc,PNAME);
			}

			/*-----------------
			| Add mnsc record |
			-----------------*/
			sprintf (mnac_rec.ac_code, "%c%-7.7s", *sptr, " ");
			lcl_cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
			if (lcl_cc)
			       sys_err("Error in mnac during (DBFIND)",lcl_cc,PNAME);

			mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
			mnsc_rec.sc_hhac_hash = mnac_rec.ac_hhac_hash;
			lcl_cc = abc_add("mnsc", &mnsc_rec);
			if (lcl_cc)
			       sys_err("Error in mnsc during (DBADD)",lcl_cc,PNAME);

			sptr++;
		}
	}

	/*-----------------------------
	| Set sptr ready for quad srch|
	-----------------------------*/
	*tptr = temp_char;
	if (*tptr)
	{
		sptr = tptr + 1;	
		proc_quadrant (sptr);
		return (EXIT_SUCCESS);
	}

	return (EXIT_FAILURE);
}

void
proc_quadrant (
 char *	sptr)
{
	int	quad_val;
	int	line_space_val;

	while (*sptr && *sptr != '(')
		sptr++;

	if (*sptr && *(sptr + 1))
	{
		sptr++;
		quad_val = atoi (sptr);
		if (quad_val > 0)
			mnln_rec.ln_quad = quad_val;

		/*------------------------------
		| Look along line for quad end |
		------------------------------*/
		while (*sptr && *sptr != ',')
			sptr++;

		if (*sptr && *(sptr + 1))
		{
			sptr++;
			line_space_val = atoi (sptr);
			if (line_space_val > 0)
				mnln_rec.ln_line_space = line_space_val;
		}
	}
}

void
update_mnhr (
 void)
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

/*-----------------------------------
| Check access code and update mnac |
-----------------------------------
chck_security(sptr)
char	*sptr;
{

	downshift(mnln_rec.ln_security);  
	sptr = clip(mnln_rec.ln_security);
	while (*sptr)
	{
		if (*sptr == '*' && local_rec.acc_security[26] == 'N')
		{
			sprintf(mnac_rec.ac_code,"%-1.1s","*");
			cc = abc_add("mnac",&mnac_rec);
			if (cc)
				sys_err("Error in mnac during (DBADD)",cc,PNAME);
			local_rec.acc_security[26] = 'Y';
		}
		else
		{
			if (*sptr >= 'a' && *sptr <= 'z')
				if (local_rec.acc_security[*sptr - 'a'] == 'N')
				{
					sprintf(mnac_rec.ac_code,"%-1.1s",sptr);
					cc = abc_add("mnac",&mnac_rec);
					if (cc)
						sys_err("Error in mnac during (DBADD)",cc,PNAME);
					local_rec.acc_security[*sptr - 'a'] = 'Y';
				}
		}
		sptr++;
	}
}
*/

/*------------------------------------------------------
| Process the screen painting section of the .mdf file |
------------------------------------------------------*/
void
proc_screen_painting (
 void)
{
	char	*sptr;
	char	*tptr;
	int	indx;
	char	data[161];

	indx = 1;
	mnhr_rec.hr_heading = -1;
	mnhr_rec.hr_trailer = -1;
	mnhr_rec.hr_fast = -1;
	mnhr_rec.hr_sub = -1;
	mnhr_rec.hr_menu_name = -1;
	mnhr_rec.hr_shell_out = -1;

	/*-------------------------------------------------------
	| Delete all comment detail records associated with the | 
	| current header record					|
	-------------------------------------------------------*/
	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 1;
	cc = find_rec("mncd",&mncd_rec,GTEQ,"r");

	while (!cc && mncd_rec.cd_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		cc = abc_delete("mncd");
		if (cc)
			sys_err("Error in mnln during (DBUPDATE)",cc,PNAME);

		cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	}


	sptr = fgets(data,161,mdf);

	while (sptr != (char *) 0)
	{
		if (sptr[0] == ')' && sptr[1] == ')')
			break;

		/*--------------------------------------
		| Replace trailing \n with trailing \0 |
		--------------------------------------*/
		tptr = sptr + strlen(sptr);
		if (strlen(sptr))
			tptr--;

		if (*tptr == '\n')
			*tptr = '\0';

		sprintf(mncd_rec.cd_description,"%-80.80s"," ");
		mncd_rec.cd_column = 0;
		mncd_rec.cd_line = 0;
		mncd_rec.cd_width = 0;
		mncd_rec.cd_depth = 0;
		mncd_rec.cd_reverse = 0;

		if (!strncmp(sptr,"quadrant",8))
			indx = quadrant_param (sptr, indx);
		if (!strncmp(sptr,"box",3))
			indx = box_param (sptr,indx);
		if (!strncmp(sptr,"comment",7))
			indx = comment_param (sptr,indx);
		if (!strncmp(sptr,"line",4))
			indx = line_param (sptr,indx);

		if (!strncmp(sptr,"HEADING_ON",10))
			mnhr_rec.hr_heading = 1;
		if (!strncmp(sptr,"HEADING_OFF",11))
			mnhr_rec.hr_heading = 0;
		if (!strncmp(sptr,"TRAILER_ON",10))
			mnhr_rec.hr_trailer = 1;
		if (!strncmp(sptr,"TRAILER_OFF",11))
			mnhr_rec.hr_trailer = 0;
		if (!strncmp(sptr,"SUB_ON",6))
			mnhr_rec.hr_sub = 1;
		if (!strncmp(sptr,"SUB_OFF",7))
			mnhr_rec.hr_sub = 0;
		if (!strncmp(sptr,"FAST_ON",7))
			mnhr_rec.hr_fast = 1;
		if (!strncmp(sptr,"FAST_OFF",8))
			mnhr_rec.hr_fast = 0;
		if (!strncmp(sptr,"MENU_NAME_ON",12))
			mnhr_rec.hr_menu_name = 1;
		if (!strncmp(sptr,"MENU_NAME_OFF",13))
			mnhr_rec.hr_menu_name = 0;
		if (!strncmp(sptr,"SHELL_ON",8))
			mnhr_rec.hr_shell_out = 1;
		if (!strncmp(sptr,"SHELL_OFF",9))
			mnhr_rec.hr_shell_out = 0;

		sptr = fgets(data,161,mdf);
	}
	updated = 1;
	update_mnhr();

	return;
}

/*--------------------------------------------
| Get parameters from the quadrant statement |
--------------------------------------------*/
int
quadrant_param (
 char *	sptr,
 int	indx)
{
	int	temp_val;

	strcpy(mncd_rec.cd_stat_flag,"0");

	sptr = sptr + 8;
	while (*sptr && *sptr != '(')
		sptr++;

	if (*sptr && *(sptr + 1))
	{
		sptr++;
		temp_val = atoi (sptr);
		if (temp_val >= 0)
			mncd_rec.cd_column = temp_val;

		while (*sptr && *sptr != ',')
			sptr++;

		if (*sptr && *(sptr + 1))
		{
			sptr++;
			temp_val = atoi (sptr);
			if (temp_val >= 0)
				mncd_rec.cd_line = temp_val;

		}
	}
	indx = update_mncd (indx);
	return(indx);
}

/*-------------------------------------------
| Get parameters from the comment statement |
-------------------------------------------*/
int
comment_param (
 char *	sptr,
 int	indx)
{

	int	temp_val;
	int	loop;
	char	*tptr;
	char	*startptr;
	char	*lclptr;
	char	temp_ptr;
	char	temp_desc[81];
	
	mncd_rec.cd_stat_flag[0] = '1';
	sprintf(mncd_rec.cd_description,"\042\042%-78.78s"," ");

	sptr = sptr + 7;
	while (*sptr && *sptr != '(')
		sptr++;

	if (*sptr && *(sptr + 1))
	{
		startptr = sptr;

		/*--------------------------------
		| Look for start of comment text |
		--------------------------------*/
		while (*sptr && *sptr != '"')
			sptr++;

		if (*sptr == '"')
		{
			tptr = sptr + 1;
			lclptr = tptr;

			/*------------------------------
			| Look for end of comment text |
			------------------------------*/
			while (*tptr && *tptr != '"')
				tptr++;
			
			if (*tptr != '"')
			{
				tptr = lclptr;
				while (*tptr && *tptr != ',')
					tptr++;
			}
		}
		else
		{
			sptr = startptr;
			tptr = sptr;
			while (*tptr && *tptr != ',')
				tptr++;
		}
		

		if (*tptr && *(tptr + 1))
		{
			temp_ptr = *tptr;
			*tptr = '\0';

			if (sptr != tptr)
			{
				sptr++;	

				loop = 0;
				while (*sptr)
				{
					if (*sptr != '"')
					{
						temp_desc[loop] = *sptr;
						loop++;
					}
					sptr++;
				}
				temp_desc[loop] = '\042';
				temp_desc[loop + 1] = '\0';
	
				sprintf(mncd_rec.cd_description,"\042%-79.79s",temp_desc);
			}
			*tptr = temp_ptr;
		}

		while (*tptr && *tptr != ',')
			tptr++;
		sptr = tptr;
			
		if (*sptr && *(sptr + 1))
		{
			sptr++;
			temp_val = atoi (sptr);
			if (temp_val >= 0)
				mncd_rec.cd_column = temp_val;


			while (*sptr && *sptr != ',')
				sptr++;

			if (*sptr && *(sptr + 1))
			{
				sptr++;
				temp_val = atoi (sptr);
				if (temp_val >= 0)
					mncd_rec.cd_line = temp_val;
	
				while (*sptr && *sptr != ',')
					sptr++;

				if (*sptr && *(sptr + 1))
				{
					sptr++;
					temp_val = atoi (sptr);
					if (temp_val != 0)
						mncd_rec.cd_reverse = 1;

				}
			}
		}
	}
	indx = 	update_mncd (indx);
	return(indx);
}

/*---------------------------------------
| Get parameters from the box statement |
---------------------------------------*/
int
box_param (
 char *	sptr,
 int	indx)
{
	int	temp_val;

	mncd_rec.cd_stat_flag[0] ='2';

	sptr = sptr + 3;
	while (*sptr && *sptr != '(')
		sptr++;

	if (*sptr && *(sptr + 1))
	{
		sptr++;
		temp_val = atoi (sptr);
		if (temp_val >= 0)
			mncd_rec.cd_column = temp_val;

		while (*sptr && *sptr != ',')
			sptr++;

		if (*sptr && *(sptr + 1))
		{
			sptr++;
			temp_val = atoi (sptr);
			if (temp_val >= 0)
				mncd_rec.cd_line = temp_val;

			while (*sptr && *sptr != ',')
				sptr++;

			if (*sptr && *(sptr + 1))
			{
				sptr++;
				temp_val = atoi (sptr);
				if (temp_val >= 0)
					mncd_rec.cd_width = temp_val;

				while (*sptr && *sptr != ',')
					sptr++;

				if (*sptr && *(sptr + 1))
				{
					sptr++;
					temp_val = atoi (sptr);
					if (temp_val >= 0)
						mncd_rec.cd_depth = temp_val;

				}
			}
		}
	}
	indx = update_mncd (indx);
	return(indx);
}

/*----------------------------------------
| Get parameters from the line statement |
----------------------------------------*/
int
line_param (
 char *	sptr,
 int	indx)
{
	int	temp_val;


	mncd_rec.cd_stat_flag[0] ='3';

	sptr = sptr + 4;
	while (*sptr && *sptr != '(')
		sptr++;

	if (*sptr && *(sptr + 1))
	{
		sptr++;
		temp_val = atoi (sptr);
		if (temp_val >= 0)
			mncd_rec.cd_column = temp_val;

		while (*sptr && *sptr != ',')
			sptr++;

		if (*sptr && *(sptr + 1))
		{
			sptr++;
			temp_val = atoi (sptr);
			if (temp_val >= 0)
				mncd_rec.cd_line = temp_val;

			while (*sptr && *sptr != ',')
				sptr++;

			if (*sptr && *(sptr + 1))
			{
				sptr++;
				temp_val = atoi (sptr);
				if (temp_val >= 0)
					mncd_rec.cd_width = temp_val;

			}
		}
	}
	indx = update_mncd (indx);
	return(indx);
}

/*----------------------
| Update the mncd file |
----------------------*/
int
update_mncd (
 int indx)
{
	mncd_rec2.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec2.cd_line_no = indx;

	cc = find_rec("mncd",&mncd_rec2,COMPARISON,"r");
	if (cc)
	{
		mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mncd_rec.cd_line_no = indx;
		cc = abc_add("mncd",&mncd_rec);
		if (cc)
			sys_err("Error in mncd during (DBADD)",cc,PNAME);
		mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mncd_rec.cd_line_no = indx;
	}
	else
	{
		mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
		mncd_rec.cd_line_no = indx;
		cc = abc_update("mncd",&mncd_rec);
		if (cc)
			sys_err("Error in mncd during (DBUPDATE)",cc,PNAME);
	}
	indx++;
	return(indx);
}

/*------------------------------------------
| Create a .mdf file from database records |
------------------------------------------*/
void
creat_mdf (
 void)
{
	int		i;
	int		lcl_cc;
	int		first_time;
	char	tempstr[255];

	if (!createall)
	{
		/*sprintf(err_str," Create %s ?  ",filename);*/
		i = prmptmsg(ML(mlMenuMess193),"YyNn",0,2);

		if (i == 'N' || i == 'n')
			return;
		dsp_screen(""," 1","Logistic Menu Maintenance");
	}

	abc_selfield("mnac", "mnac_hhac_hash");

	rv_pr("                            ",30,10,0);
	/*sprintf(err_str," Writing %s.mdf ",clip(local_rec.menu_name));*/
	rv_pr(ML(mlStdMess035),30,10,0);
	sprintf(tempstr,"%s",mnhr_rec.hr_description);
	fprintf(mdf,"%s ",clip(tempstr));
	sprintf(tempstr,"%s",mnhr_rec.hr_fast_access);
	fprintf(mdf,"(%s)\n",clip(tempstr));
	sprintf(tempstr,"%s",mnhr_rec.hr_help);
	fprintf(mdf,"%s\n",clip(tempstr));

	mnln_rec.ln_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mnln_rec.ln_line_no = 0;
	cc = find_rec("mnln",&mnln_rec,GTEQ,"r");

	while (!cc && mnln_rec.ln_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		sprintf(err_str,"%2d",mnln_rec.ln_line_no);
		dsp_process(" Line : ",err_str);

		fprintf(mdf,"%s ",clip(mnln_rec.ln_description));

		fprintf(mdf,"<");
		mnsc_rec.sc_hhln_hash = mnln_rec.ln_hhln_hash;
		mnsc_rec.sc_hhac_hash = 0L;
		first_time = TRUE;
		cc = find_rec("mnsc", &mnsc_rec, GTEQ, "r");
		while (!cc && mnsc_rec.sc_hhln_hash == mnln_rec.ln_hhln_hash)
		{
			if (!first_time)
				fprintf(mdf,"|");
		
			first_time = FALSE;
		
			/*-----------------------------
			| Get security code from mnac |
			-----------------------------*/
			lcl_cc = find_hash("mnac", &mnac_rec, COMPARISON, "r", mnsc_rec.sc_hhac_hash);
			if (lcl_cc)
				sys_err("Error in mnac during (DBFIND)",lcl_cc,PNAME);
			fprintf(mdf,"%.8s", clip(mnac_rec.ac_code));

			cc = find_rec("mnsc", &mnsc_rec, NEXT, "r");
		}
	
		fprintf(mdf,"> ");

		fprintf(mdf,"(%d,",mnln_rec.ln_quad);
		fprintf(mdf,"%d)\n",mnln_rec.ln_line_space);
		fprintf(mdf,"%s\n",clip(mnln_rec.ln_prog_call));
		
		cc = find_rec("mnln",&mnln_rec,NEXT,"r");
	}

	fprintf(mdf,"((\n");

	mncd_rec.cd_hhmn_hash = mnhr_rec.hr_hhmn_hash;
	mncd_rec.cd_line_no = 0;
	cc = find_rec("mncd",&mncd_rec,GTEQ,"r");
	while (!cc && mncd_rec.cd_hhmn_hash == mnhr_rec.hr_hhmn_hash)
	{
		sprintf(err_str,"%2d",mncd_rec.cd_line_no);
		dsp_process(" Line : ",err_str);

		switch (mncd_rec.cd_stat_flag[0])
		{
		case 	'0':
			fprintf(mdf,"quadrant");
			fprintf(mdf,"(%d,",mncd_rec.cd_column);
			fprintf(mdf,"%d)\n",mncd_rec.cd_line);
			break;
		case 	'1':
			fprintf(mdf,"comment");
			fprintf(mdf,"(%s,",clip(mncd_rec.cd_description));
			fprintf(mdf,"%d,",mncd_rec.cd_column);
			fprintf(mdf,"%d,",mncd_rec.cd_line);
			fprintf(mdf,"%d)\n",mncd_rec.cd_reverse);
			break;
		case 	'2':
			fprintf(mdf,"box");
			fprintf(mdf,"(%d,",mncd_rec.cd_column);
			fprintf(mdf,"%d,",mncd_rec.cd_line);
			fprintf(mdf,"%d,",mncd_rec.cd_width);
			fprintf(mdf,"%d)\n",mncd_rec.cd_depth);
			break;
		case 	'3':
			fprintf(mdf,"line");
			fprintf(mdf,"(%d,",mncd_rec.cd_column);
			fprintf(mdf,"%d,",mncd_rec.cd_line);
			fprintf(mdf,"%d)\n",mncd_rec.cd_width);
			break;
		default:
				
			break;
		}
		
		cc = find_rec("mncd",&mncd_rec,NEXT,"r");
	}
	if (mnhr_rec.hr_heading == 1)
		fprintf(mdf,"HEADING_ON\n");
	else
		if (mnhr_rec.hr_heading == 0)
			fprintf(mdf,"HEADING_OFF\n");

	if (mnhr_rec.hr_trailer == 1)
		fprintf(mdf,"TRAILER_ON\n");
	else
		if (mnhr_rec.hr_trailer == 0)
			fprintf(mdf,"TRAILER_OFF\n");

	if (mnhr_rec.hr_fast == 1)
		fprintf(mdf,"FAST_ON\n");
	else
		if (mnhr_rec.hr_fast == 0)
			fprintf(mdf,"FAST_OFF\n");

	if (mnhr_rec.hr_sub == 1)
		fprintf(mdf,"SUB_ON\n");
	else
		if (mnhr_rec.hr_sub == 0)
			fprintf(mdf,"SUB_OFF\n");

	if (mnhr_rec.hr_menu_name == 1)
		fprintf(mdf,"MENU_NAME_ON\n");
	else
		if (mnhr_rec.hr_menu_name == 0)
			fprintf(mdf,"MENU_NAME_OFF\n");

	if (mnhr_rec.hr_shell_out == 1)
		fprintf(mdf,"SHELL_ON\n");
	else
		if (mnhr_rec.hr_shell_out == 0)
			fprintf(mdf,"SHELL_OFF\n");

	fprintf(mdf,"))\n");

	abc_selfield("mnac", "mnac_code");
}


/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();

		move (0, 1);
		line (80);

		if (store || convert)
			/*Menu Store. */
			rv_pr(ML(mlMenuMess113),34,0,1);
		else
			/* Menu Creation. */
			rv_pr(ML(mlMenuMess114),32,0,1);

		box(0,3,80,2);

		move(0,20);
		line(80);
		/*Directory : %s*/
		print_at(21,0,ML(mlStdMess156),directory);
		move(0,22);
		line(80);
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
