/*=====================================================================
|  Copyright (C) 1988 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( prn_maint.c    )                                 |
|  Program Desc  : ( Maintains the prntype file                     ) |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 23/01/91         |
|---------------------------------------------------------------------|
|  Date Modified : (20/01/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo    |
|                                                                     |
|  Comments      : (20/01/92) - Allow search on printer types and     |
|                : create /usr/LS10.5/?????? save file directory.         |
|                : (12/09/97) - Updated for Multilingual Conversion   |
=====================================================================*/
#define MAX_PRN 100
#define CCMAIN
#include <pslscr.h>
#include <hot_keys.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ml_std_mess.h>
#include <ml_utils_mess.h>
#include <tabdisp.h>

char	*PNAME = "$RCSfile: prn_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/prn_maint/prn_maint.c,v 5.2 2001/08/09 09:27:22 scott Exp $";

FILE	*ptypf;
FILE	*pcapp;
FILE	*pcapf;

struct	stat	file_info;

struct PRN_REC {
	char	ptr_name[15];
	char	q_name[15];
	char	ptr_desc[51];
} ptype_line[MAX_PRN], tmp_line;

char	prog_path[100];
char	directory[100];
char	filename[100];
char	vars_dummy[11];
int	no_in_tab = 0;
int	exit_loop;
int	scan_cont;

static	int	quit_func(int c, KEY_TAB *psUnused);
static	int	add_func(int c, KEY_TAB *psUnused);
static	int	delete_func(int c, KEY_TAB *psUnused);
static	int	modify_func(int c, KEY_TAB *psUnused);
static	int	exit_update_func(int c, KEY_TAB *psUnused);
	
static	KEY_TAB edit_keys [] =
{
   { "[Q]UIT",		'Q', quit_func,
	"Exit without updating prntype.",				"A" },
   { "[A]DD",		'A', add_func,
	"Add a line to the end of the list.",				"A" },
   { "[M]ODIFY",	'M', modify_func,
	"Modify the line at the current cursor position.",		"E" },
   { "[D]ELETE",	'D', delete_func,
	"Delete the line at the current cursor position.",		"E" },
   { NULL,		FN16, exit_update_func,
	"Exit and update prntype.",					"A" },
   END_KEYS
};
	     
static	struct	var	vars[] =
{
	{1, LIN, "ptr_name",	20, 20, CHARTYPE,
		"AAAAAAAAAAAAAA", "         ",
		" ", "", "Printer Type:", "Please enter printer type. NB This will be the name of the printcap for the printer.",
		YES, NO,  JUSTLEFT, "", "", tmp_line.ptr_name},
	{1, LIN, "q_name",	20, 60, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "Queue Name:", "Please enter name of queue.",
		YES, NO,  JUSTLEFT, "", "", tmp_line.q_name},
	{1, LIN, "ptr_desc",	21, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Printer Description:", "Please enter description you want to be displayed for the printer.",
		 NO, NO,  JUSTLEFT, "", "", tmp_line.ptr_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", vars_dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
int spec_valid (int field);
void srch_pcap (char *key_val);
void create_table (void);
void create_dir (char *dir_name);
void Set_file (char *_filename);
void erase_btm_scn (void);
void chk_keys (void);
void load_prntype (void);
int val_pcap (void);
int heading (int scn);

int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr = getenv("PROG_PATH");

	strcpy(prog_path,(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	sprintf(directory,"%s/BIN/MENUSYS", prog_path);

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
	swide();

	set_help(FN6, "FN6");

	/*-------------------------------
	| Set up screen ready for table |
	-------------------------------*/
	heading (0);

	/*------------------------------
	| Load prntype file into array |
	------------------------------*/
	load_prntype ();

	exit_loop = FALSE;
	while (!exit_loop)
	{
		/*-------------------------
		| Create table from array |
		-------------------------*/
		create_table ();

		do
		{
			scan_cont = FALSE;
			tab_scan("ptyp");
		} while (scan_cont);

		tab_close("ptyp",TRUE);
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
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
	int	chck_flag;

	if (LCHECK("ptr_name")) 
        {
		if (SRCH_KEY)
		{
			srch_pcap (temp_str);
			heading (0);
			tab_display("ptyp", TRUE);
			return(0);
		}

		chck_flag = val_pcap ();
		return(chck_flag);
	}
	return(0);
}

void
srch_pcap (
 char*              key_val)
{
	DIR	*fd;
	struct	dirent *dirbuf;
	char	tmp_pname[100];

	work_open();
	save_rec("#Printer Type", "#");

	sprintf(filename,"%s/PRINT", directory);
	clip(filename);
	if ((fd = opendir (filename)) != (DIR *) 0)
	{
		while ((dirbuf = readdir(fd)) != (struct dirent *) 0)
		{
			if (!strncmp(dirbuf->d_name, ".", 1) ||
		    	    !strncmp(dirbuf->d_name, "..", 2))
				continue;
	
			if (strncmp(dirbuf->d_name, key_val, strlen(key_val)))
				continue;
	
			sprintf(tmp_pname, "%s/PRINT/%s", 
				directory, 
				dirbuf->d_name);
			clip(tmp_pname);
			
			/*----------------------------------
			| Check if filename is a directory |
			----------------------------------*/
			if (!stat(tmp_pname, &file_info))
			{
				if (!(file_info.st_mode & S_IFREG))
					continue;
			}
			else
				continue;

			sprintf(tmp_pname, "%-14.14s", dirbuf->d_name);
			cc = save_rec(tmp_pname, "");
			if (cc)
				break;
		}
	}

	cc = disp_srch();
	work_close();

}

/*------------------------------------------
| Create table of prntype lines from array |
------------------------------------------*/
void
create_table (void)
{
	int	i = 0;

	tab_open("ptyp",edit_keys,3,20,10,FALSE);
	tab_add("ptyp","#%-15.15s  %-15.15s  %-50.50s  ","Printer Type","Queue Name","Printer Description");

	/*-----------------------
	| Load table from array |
	-----------------------*/
	for (i = 0; i < no_in_tab; i++)
		tab_add("ptyp",
			" %-15.15s  %-15.15s  %-50.50s ",
			ptype_line[i].ptr_name,
			ptype_line[i].q_name,
			ptype_line[i].ptr_desc);

	if (no_in_tab == 0)
		tab_add("ptyp", "%-86.86s", " ");

	/*---------------------------------
	| Check if table is empty and set |
	| hotkeys accordingly		  |
	---------------------------------*/
	chk_keys ();
}

/*-----------------------
| Add a line to the end |
-----------------------*/
static	int
add_func (
 int                c,
 KEY_TAB*           psUnused)
{
	char	dummy[133];

	restart = FALSE;
	prog_exit = FALSE;
	search_ok = TRUE;

	heading (1);
	entry(1);
	
	if (!prog_exit)
	{
		scn_display(1);
		edit(1);
	}

	erase_btm_scn();

	if (!restart && !prog_exit)
	{
		memcpy ((char *) &ptype_line[no_in_tab], (char *) &tmp_line, sizeof (struct PRN_REC));   

		if (no_in_tab == 0)
		{
			tab_get("ptyp", dummy, EQUAL, 0);

			tab_update("ptyp",
				" %-15.15s  %-15.15s  %-50.50s ",
				ptype_line[no_in_tab].ptr_name,
				ptype_line[no_in_tab].q_name,
				ptype_line[no_in_tab].ptr_desc);
		}
		else
		{
			tab_add("ptyp",
				" %-15.15s  %-15.15s  %-50.50s ",
				ptype_line[no_in_tab].ptr_name,
				ptype_line[no_in_tab].q_name,
				ptype_line[no_in_tab].ptr_desc);
		}
		no_in_tab++;
	}

	/*---------------------------------
	| Check if table is empty and set |
	| hotkeys accordingly		  |
	---------------------------------*/
	chk_keys ();
	tab_display("ptyp", TRUE);
	redraw_keys("ptyp");

	if (no_in_tab == 1)
	{
		scan_cont = TRUE;
		return (FN16);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------
| Delete the current line |
-------------------------*/
static	int
delete_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	pos_line;
	int	i;

	pos_line = tab_tline("ptyp");
	memcpy ((char *) &tmp_line, (char *) &ptype_line[pos_line], sizeof (struct PRN_REC));   

	restart = 0;
	
	init_ok = FALSE;
	heading (1);
	scn_display(1);
	init_ok = TRUE;

	move(0,18);
	cl_line();
	sprintf(err_str, ML(mlUtilsMess017), filename);
	crsr_on();
	i = prmptmsg(err_str,"YyNn",48,18);

	if (i == 'N' || i == 'n')
	{
		erase_btm_scn();
		return(0);
	}

	erase_btm_scn();

	/*----------------------------
	| Ripple array up by 1 entry |
	----------------------------*/
	for (i = pos_line + 1; i < no_in_tab; i++)
		memcpy ((char *) &ptype_line[i - 1], (char *) &ptype_line[i], sizeof (struct PRN_REC));   
	
	no_in_tab--;

	return (FN16);
}

/*-----------------------
| Quit without updating |
-----------------------*/
static	int
quit_func (
 int                c,
 KEY_TAB*           psUnused)
{   
	exit_loop = TRUE;
	return(FN16);
}

/*---------------------
| Modify current line |
---------------------*/
static	int
modify_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	pos_line = 0;
	char	dummy[133];

	tab_get("ptyp", dummy, CURRENT, 0);
	pos_line = tab_tline("ptyp");

	memcpy ((char *) &tmp_line, (char *) &ptype_line[pos_line], sizeof (struct PRN_REC));   

	restart = 0;

	init_ok = FALSE;
	heading (1);
	scn_display(1);
	edit(1);
	init_ok = TRUE;

	erase_btm_scn ();

	if (!restart)
	{
		memcpy ((char *) &ptype_line[pos_line], (char *) &tmp_line, sizeof (struct PRN_REC));   

		tab_update("ptyp",
			" %-15.15s  %-15.15s  %-50.50s ",
			ptype_line[pos_line].ptr_name,
			ptype_line[pos_line].q_name,
			ptype_line[pos_line].ptr_desc);
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------
| Exit and update the termcap |
-----------------------------*/
static	int
exit_update_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	i;

	sprintf(filename, "%s/prntype", directory);
	sprintf(err_str, ML(mlUtilsMess019), filename);
	rv_pr(ML(mlUtilsMess018),32,19,1);
	crsr_on();
	i = prmptmsg(err_str,"YyNn",39,18);

	move(0,18);
	cl_line();
	move(0,19);
	cl_line();

	if (i == 'N' || i == 'n')
		return(0);

	sprintf(filename, "%s/prntype", directory);
	if ((ptypf = fopen(filename,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (FOPEN)",filename);
		sys_err(err_str,errno,PNAME);
	}

	sprintf(err_str, ML(mlUtilsMess020),filename);
	rv_pr(err_str,40,18,1);

	for (i = 0; i < no_in_tab; i++)
	{
		fprintf(ptypf,"%s\t",clip(ptype_line[i].ptr_name));
		fprintf(ptypf,"%s\t",clip(ptype_line[i].q_name));
		fprintf(ptypf,"%s\n",clip(ptype_line[i].ptr_desc));

		create_dir (ptype_line[i].q_name);
	}

	fclose(ptypf);

	exit_loop = TRUE;
	return(FN16);
}

/*------------------------------------
| Create printer save file directory |
------------------------------------*/
void
create_dir (
 char*              dir_name)
{
	char	tmp_dir[15];
	char	cmd[200];

	sprintf(tmp_dir, "%-14.14s", dir_name);
	clip(tmp_dir);
	upshift(tmp_dir);
	sprintf(filename, "%s/%s", prog_path, tmp_dir);
	clip(filename);

	/*------------------------------
	| Return if dir already exists |
	------------------------------*/
	if (!access(filename, 00))
		return;

	sprintf(cmd, "mkdir %s", filename);
	sys_exec(cmd);

	Set_file (filename);
}

/*------------------------------------------
| Set correct permissions on new directory |
------------------------------------------*/
void
Set_file (
 char*              _filename)
{
	static	int	uid;
	static	int	gid;
	static	int	perm;
	int	_perm;
	char	*sptr;
	char	*_usr;
	char	*_grp;
	struct	passwd	*usr;
	struct	group	*grp;
	char	_err_str[81];
	char	*_psl = "psl";
	/*---------------------------------------
	| set defaults				|
	---------------------------------------*/
	_usr = _psl;
	_grp = _psl;
	_perm = 512;
	/*---------------------------------------
	| read FILE_PERM environment variable	|
	---------------------------------------*/
	sptr = getenv ("FILE_PERM");
	if (sptr != (char *)0)
	{
		/*---------------------------------------
		| get user name from variable		|
		---------------------------------------*/
		_usr = sptr;
		sptr = strchr(_usr,' ');
		if (sptr != (char *)0)
		{
			*sptr = '\0';
			_grp = sptr + 1;
			/*---------------------------------------
			| get group name from variable		|
			---------------------------------------*/
			sptr = strchr(_grp,' ');
			if (sptr != (char *)0)
			{
				*sptr = '\0';
				_perm = atoi(sptr + 1);
			}
		}
	}
	/*---------------------------------------
	| find user id for user			|
	---------------------------------------*/
	usr = getpwnam(_usr);
	if (usr == (struct passwd *)0)
	{
		sprintf(_err_str,"No User Name [%s] Exists",_usr);
		sys_err(_err_str,-1,PNAME);
	}
	uid = usr->pw_uid;
	/*---------------------------------------
	| find group id for group		|
	---------------------------------------*/
	grp = getgrnam(_grp);
	if (grp == (struct group *)0)
	{
		sprintf(_err_str,"No Group Name [%s] Exists",_grp);
		sys_err(_err_str,-1,PNAME);
	}
	gid = grp->gr_gid;
	/*---------------------------------------
	| convert permissions to octal		|
	---------------------------------------*/
	sprintf(_err_str,"%05d",_perm);
	for (perm = 0,sptr = _err_str;*sptr;sptr++)
	{
		perm *= 8;
		perm += (*sptr - '0');
	}

	chown(_filename,uid,gid);
	chmod(_filename,perm);
}

void
erase_btm_scn (void)
{
	int	i;

	for (i = 18; i < 23; i++)
	{
		move (0,i);
		cl_line ();
	}
}

/*---------------------------------
| Check if table is empty and set |
| hotkeys accordingly		  |
---------------------------------*/
void
chk_keys (void)
{
	if (no_in_tab == 0)
		set_keys(edit_keys, "E", KEY_PASSIVE);
	else
		set_keys(edit_keys, "E", KEY_ACTIVE);

	tab_keyset("ptyp", edit_keys);
}

/*===============================
| Loads prntype file into array |
===============================*/
void
load_prntype (void)
{
	int	loop = 0;
	char	data[81];
	char	*sptr;
	char	*tptr;

	sprintf(filename,"%s/prntype",directory);
	if ((ptypf = fopen(filename,"r")) == 0)
	{
		sprintf (err_str, "Error in %s during (FOPEN)\n\r",filename);
		sys_err(err_str, errno, PNAME);
		return;
	}

	/*-------------------------------
	| Load prntype file into arrays | 
	| FIELDS ARE SEPARATED BY  TABS | 
	-------------------------------*/
	sptr = fgets(data,80,ptypf);

	while (sptr)
	{
		*(sptr + strlen(sptr) - 1) = '\0';

		/*------------------------------
		| Look for end of printer name |
		------------------------------*/
		tptr = sptr;
		while (*tptr && *tptr != '\t')
			tptr++;

		if (*tptr)
		{
			*tptr = '\0';

			/*-------------------
			| Grab printer name |
			-------------------*/
			strcpy(ptype_line[loop].ptr_name, sptr);

			/*----------------------------
			| Look for end of queue name |
			----------------------------*/
			sptr = ++tptr;
			while (*tptr && *tptr != '\t')
				tptr++;

			if (*tptr)
			{
				/*-----------------
				| Grab queue name |
				-----------------*/
				*tptr = '\0';
				strcpy(ptype_line[loop].q_name, sptr);

				/*--------------------------
				| Grab printer description |
				--------------------------*/
				tptr++;
				strcpy(ptype_line[loop].ptr_desc, tptr);

			}
		}
		loop++;
		sptr = fgets(data,80,ptypf);
	}
	fclose(ptypf);
	no_in_tab = loop;
}

/*=======================================
| Check that printcap exists in either  |
| $PROG_PATH/BIN/MENUSYS/PRINT or       |
| $PROG_PATH/BIN/MENUSYS/termcap.cprogs |
=======================================*/
int
val_pcap (void)
{
	char 	rd_line[81];
	char	*sptr;
	char	tmp_name[15];
	int	wk_len = 0;
	
	strcpy(tmp_name, clip(temp_str));

	sprintf(filename,"%s/PRINT/%s", directory, tmp_name);
	if ((pcapf = fopen(filename,"r")) == 0)
	{
		sprintf(filename,"%s/termcap.cprogs",directory);
		/*----------------------------------------------
		| Open /usr/LS10.5/BIN/MENUSYS/termcapcprogs file. |
		----------------------------------------------*/
		if ((pcapf = fopen(filename,"r")) == 0)
			return(1);
	}

	/*=====================
	| Find printer entry. |
	=====================*/
	wk_len = strlen(tmp_name);
	sptr = fgets(rd_line,80,pcapf);
	while (sptr != (char *)0 && strncmp(rd_line,tmp_name,wk_len))
		sptr = fgets(rd_line,80,pcapf);

	/*=========================================
	| Close file /usr/LS10.5/BIN/MENUSYS/termcap. |
	=========================================*/
	fclose(pcapf);

	/*===================
	| Invalid printcap. |
	| Return 1          |
	===================*/
	if (sptr == (char *)0)
	{
		sprintf(err_str, ML(mlUtilsMess021), tmp_name);
		rv_pr(err_str,44,18,1);
		sleep(2);
		move(0,18);
		cl_line();

		return(1);
	}

	return(0);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int                scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn == 0)
		{		
			clear();

			strcpy(hdng_date,DateToString(TodaysDate()));

			sprintf(err_str,ML(mlUtilsMess022));
			rv_pr(err_str,(132 - strlen(err_str)) / 2,0,1);
			rv_pr(hdng_date,123,0,0);

			move(0,1);
			line(132);

			disp_help(110);
			return(0);

		}

		if (scn == 1)
		{
			erase_btm_scn ();
			scn_set(scn);
			box(0,19,132,2); 
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
