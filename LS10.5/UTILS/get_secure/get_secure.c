/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( get_secure.c   )                                 |
|  Program Desc  : ( User / Company / Branch Access Maintenance.  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/07/88         |
|---------------------------------------------------------------------|
|  Date Modified : 28/07/88        | Modified  by  : Roger Gibbison.  |
|  Date Modified : 19/04/91        | Modified  by  : Campbell Mander. |
|  Date Modified : (21/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/97)      | Modified  by : Marnie  Organo    |
|                                                                     |
|  Comments      : (19/04/91) Updated dsp to Dsp.                     |
|  (21/08/92)    : Changes for Concurrent Logins. S/C PSL 7646        |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: get_secure.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/get_secure/get_secure.c,v 5.3 2002/02/22 08:36:34 cha Exp $";

#define	MAXLINES	400
#include 	<pslscr.h>
#include	<license2.h>
#include	<sys/types.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>
#include 	<ml_menu_mess.h>

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_short_name"},
	};

	int comr_no_fields = 3;

	struct {
		char	co_co_no[3];
		char	co_co_name[41];
		char	co_co_short_name[16];
	} comr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
	};

	int esmr_no_fields = 4;

	struct {
		char	es_co_no[3];
		char	es_est_no[3];
		char	es_est_name[41];
		char	es_short_name[16];
	} esmr_rec;

struct	{
	char	_name[9];
	char	_co_no[3];
	char	_br_no[3];
} sec_rec;

struct	{
	char	dummy[11];
	char	co_short[16];
	char	br_short[16];
} local_rec;

char	filename[2][100];

static	struct	var	vars[]	={	

	{1, TAB, "user_name", MAXLINES, 1, CHARTYPE, 
		"LLLLLLLL", "          ", 
		" ", " ", " User Name ", " ", 
		YES, NO, JUSTLEFT, "", "", sec_rec._name}, 
	{1, TAB, "co_no", 0, 1, CHARTYPE, 
		"AA", "          ", 
		" ", " ", " Co ", " ", 
		YES, NO, JUSTRIGHT, "", "", sec_rec._co_no}, 
	{1, TAB, "co_short", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "  Company Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.co_short}, 
	{1, TAB, "br_no", 0, 1, CHARTYPE, 
		"AA", "          ", 
		" ", " ", " Br ", " ", 
		YES, NO, JUSTRIGHT, "", "", sec_rec._br_no}, 
	{1, TAB, "br_short", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "  Branch Name  ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.br_short}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void srch_user (char *key_val);
void srch_co_no (char *key_val);
void srch_br_no (char *key_val);
void load_secure (void);
int update (void);
void process (char *user_name);
int heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr = getenv("PROG_PATH");

	SETUP_SCR( vars );

	tab_col = 13;
	if (geteuid () != (uid_t) 0)
    {
        print_at (0,0, ML(mlMenuMess109));
        return (EXIT_FAILURE);
    }

	OpenDB();

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();

	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, TRUE);

	if (argc != 1)
	{
		process(argv[1]);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

	sprintf(filename[0],"%s/BIN/SECURE",(sptr != (char *)0) ? sptr : "/usr/LS10.5");
	sprintf(filename[1],"%s/BIN/SECURE.o",(sptr != (char *)0) ? sptr : "/usr/LS10.5");

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

		load_secure();
		if (lcount[1] == 0)
		{
			heading(1);
			entry(1);
			if (prog_exit || restart)
				break;
		}

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		prog_exit = 1;
		if (update () == 1)
        {
            return (EXIT_SUCCESS);
        }
	}
	shutdown_prog();
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

void
OpenDB (void)
{
	abc_dbopen("data");
	open_rec("comr",comr_list,comr_no_fields,"comr_co_no");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
}

void
CloseDB (void)
{
	abc_fclose("comr");
	abc_fclose("esmr");
	abc_dbclose("data");
}

int
spec_valid (
 int                field)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (strcmp(FIELD.label,"user_name") == 0) 
        {
		if (last_char == SEARCH)
		{
			srch_user (temp_str);
			return(0);
		}

		if (dflt_used)
		{
			if (lcount[1] < 1)
				return(0);

			lcount[1]--;
			for (i = line_cnt;line_cnt < lcount[1];line_cnt++)
			{
				getval(line_cnt + 1);
				putval(line_cnt);
				if (line_cnt / TABLINES == this_page)
					line_display();
			}
			strcpy(sec_rec._name,"        ");
			strcpy(sec_rec._co_no,"  ");
			strcpy(local_rec.co_short,"               ");
			strcpy(sec_rec._br_no,"  ");
			strcpy(local_rec.br_short,"               ");

			putval(line_cnt);
			if (line_cnt / TABLINES == this_page)
				line_display();

			line_cnt = i;
			getval(line_cnt);
		}

		display_field(field);

                return(0);
	}

	if (strcmp(FIELD.label,"co_no") == 0) 
        {
		if (last_char == SEARCH)
		{
			srch_co_no (temp_str);
			return(0);
		}

		if (!strcmp(sec_rec._co_no,"  "))
		{
			sprintf(local_rec.co_short,"%-15.15s"," ");
			strcpy(sec_rec._br_no,"  ");
			sprintf(local_rec.br_short,"%-15.15s"," ");
			display_field(label("co_short"));
			display_field(label("br_no"));
			display_field(label("br_short"));
			skip_entry = 3;
			return(0);
		}

		strcpy(comr_rec.co_co_no,sec_rec._co_no);
		cc = find_rec("comr",&comr_rec,COMPARISON,"r");
		if (cc)
		{
			/*print_mess(" No such Company on file ");	*/
			print_mess(ML(mlStdMess130));	
			return(1);
		}
		strcpy(local_rec.co_short,comr_rec.co_co_short_name);
		display_field(label("co_short"));

		if (prog_status != ENTRY)
		{
			strcpy(sec_rec._br_no,"  ");
			sprintf(local_rec.br_short,"%-15.15s"," ");
			display_field(label("br_no"));
			display_field(label("br_short"));
		}
		return(0);
	}

	if (strcmp(FIELD.label,"br_no") == 0) 
        {
		if (last_char == SEARCH)
		{
			srch_br_no (temp_str);
			return(0);
		}

		if (!strcmp(sec_rec._br_no,"  "))
		{
			sprintf(local_rec.br_short,"%-15.15s"," ");
			display_field(label("br_no"));
			display_field(label("br_short"));
			return(0);
		}

		strcpy(esmr_rec.es_co_no,sec_rec._co_no);
		strcpy(esmr_rec.es_est_no,sec_rec._br_no);
		cc=  find_rec("esmr",&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			/*print_mess(" No such Company / Branch on file ");	*/
			print_mess(ML(mlStdMess146));	
			sleep(2);
			clear_mess();	
			return(1);
		}
		strcpy(local_rec.br_short,esmr_rec.es_short_name);
		display_field(label("br_short"));
		return(0);
	}

	return(0);
}

void
srch_user (
 char*              key_val)
{
	FILE	*fin;
	FILE	*fsort = sort_open("secure");
	char	*sptr = getenv("PROG_PATH");
	char	_filename[100];

	if (sptr == (char *) 0)
		strcpy(_filename,"/usr/LS10.5/BIN/MENUSYS/User_secure");
	else
		sprintf(_filename,"%s/BIN/MENUSYS/User_secure",sptr);

	if ((fin = fopen(_filename,"r")) == 0)
		sys_err("Error in User_secure during (FOPEN)",errno,PNAME);

	sptr = fgets(err_str,80,fin);

	while (sptr != (char *)0)
	{
		if (!strncmp(sptr,key_val,strlen(key_val)))
        {
            char    sort_temp[10];
            sprintf (sort_temp, "%-8.8s\n", sptr);
			sort_save (fsort, sort_temp);
        }
		sptr = fgets(err_str,80,fin);
	}

	fsort = sort_sort(fsort,"secure");

	sptr = sort_read(fsort);

	work_open();
	save_rec("#User Name","# ");

	while (sptr != (char *)0)
	{
		if (*(sptr + strlen(sptr) - 1) == '\n')
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
srch_co_no (
 char*              key_val)
{
	work_open();

	save_rec("#Co","#Company Name");

	sprintf(comr_rec.co_co_no,"%-2.2s",key_val);
	cc = find_rec("comr",&comr_rec,GTEQ,"r");
	while (!cc && !strncmp(comr_rec.co_co_no,key_val,strlen(key_val)))
	{
		cc = save_rec(comr_rec.co_co_no,comr_rec.co_co_short_name);
		if (cc)
			break;

		cc = find_rec("comr",&comr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf(comr_rec.co_co_no,"%-2.2s",temp_str);
	cc = find_rec("comr",&comr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in comr during (DBFIND)",cc,PNAME);
}

void
srch_br_no (
 char*              key_val)
{
	work_open();

	save_rec("#Br","#Branch Name");
	strcpy(esmr_rec.es_co_no,sec_rec._co_no);
	sprintf(esmr_rec.es_est_no,"%-2.2s",key_val);

	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");

	while (!cc && !strcmp(esmr_rec.es_co_no,sec_rec._co_no) && !strncmp(esmr_rec.es_est_no,key_val,strlen(key_val)))
	{
		cc = save_rec(esmr_rec.es_est_no,esmr_rec.es_short_name);
		if (cc)
			break;

		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(esmr_rec.es_co_no,sec_rec._co_no);
	sprintf(esmr_rec.es_est_no,"%-2.2s",temp_str);

	cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in esmr during (DBFIND)",cc,PNAME);
}

void
load_secure (void)
{
	int	fd = open_secure ();
	char	*sptr;
	FILE	*fsort = sort_open("secure");

	lcount[1] = 0;

	cc = RF_READ(fd, (char *) &sec_rec);

	while (!cc)
	{
		sprintf(err_str,"%s %s %s\n",sec_rec._name,sec_rec._co_no,sec_rec._br_no);
		sort_save(fsort,err_str);
		cc = RF_READ(fd, (char *) &sec_rec);
	}

	close_env(fd);

	fsort = sort_sort(fsort,"secure");

	sptr = sort_read(fsort);

	while (sptr != (char *)0 && lcount[1] < vars[0].row)
	{
		sprintf(sec_rec._name,"%-8.8s",sptr);
		sprintf(sec_rec._co_no,"%-2.2s",sptr + 9);
		sprintf(sec_rec._br_no,"%-2.2s",sptr + 12);

		if (!strcmp(sec_rec._co_no,"  "))
		{
			sprintf(local_rec.co_short,"%-15.15s"," ");
			sprintf(local_rec.br_short,"%-15.15s"," ");
			putval(lcount[1]++);
		}
		else
		{
			strcpy(comr_rec.co_co_no,sec_rec._co_no);
			cc=  find_rec("comr",&comr_rec,COMPARISON,"r");
			if (!cc)
			{
				strcpy(local_rec.co_short,comr_rec.co_co_short_name);
				if (!strcmp(sec_rec._br_no,"  "))
				{
					sprintf(local_rec.br_short,"%-15.15s"," ");
					putval(lcount[1]++);
				}
				else
				{
					strcpy(esmr_rec.es_co_no,sec_rec._co_no);
					strcpy(esmr_rec.es_est_no,sec_rec._br_no);
					cc=  find_rec("esmr",&esmr_rec,COMPARISON,"r");
					if (!cc)
					{
						strcpy(local_rec.br_short,esmr_rec.es_short_name);
						putval(lcount[1]++);
					}
				}
			}
		}
		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"secure");
}

int
update (void)
{
	int	c;
	int	fd;

	/*c = prmptmsg("Save Changes to Security File ? ","YyNn",0,2);*/
	c = prmptmsg(ML(mlUtilsMess001),"YyNn",0,2);

	clear();
	
	if (c == 'N' || c == 'n')
		return (EXIT_SUCCESS);

	if (access(filename[0],00) != -1)
	{
		/*print_at(0,0,"\n\rSaving Old Security File ... ");*/
		print_at(0,0,ML(mlUtilsMess002));
		fflush(stdout);

		if (fork() == 0)
        {
			execlp ("mv","mv",filename[0],filename[1],(char *)0);
            return (EXIT_FAILURE);
        }
		else
			wait((int *)0);
	}

/*	print_at(0,0,"\n\rSaving New Security File ... ");*/
	print_at(0,0,ML(mlUtilsMess002));
	fflush(stdout);

	fd = open_secure();

	for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt);
		add_secure(fd,sec_rec._name,sec_rec._co_no,sec_rec._br_no);
	}

	close_secure (fd);
    return (EXIT_SUCCESS);
}

void
process (
 char*              user_name)
{
	int	fd = open_secure();
	char	*sptr;
	char	user[9];
	FILE	*fsort = sort_open("secure");

	Dsp_open(0,0,10);
	Dsp_saverec(" User     | Co | Company Name    | Br | Branch Name     ");
	Dsp_saverec("");
	Dsp_saverec(" [FN03] [FN14] [FN15] [FN16] ");

	sprintf(user,"%-8.8s",user_name);

	cc = RF_READ(fd, (char *) &sec_rec);

	while (!cc)
	{
		if (!strcmp(sec_rec._name,user))
		{
			sprintf (err_str,"%s %s %s\n",sec_rec._name,sec_rec._co_no,sec_rec._br_no);
			sort_save (fsort, err_str);
		}
		cc = RF_READ(fd, (char *) &sec_rec);
	}

	close_env(fd);

	fsort = sort_sort (fsort,"secure");

	sptr = sort_read(fsort);

	if (sptr == (char *)0)
	{
		sprintf(err_str," %-8.8s ^E All Companies / Branches ",user);
		Dsp_saverec(err_str);
	}

	while (sptr != (char *)0)
	{
		sprintf(sec_rec._name,"%-8.8s",sptr);
		sprintf(sec_rec._co_no,"%-2.2s",sptr + 9);
		sprintf(sec_rec._br_no,"%-2.2s",sptr + 12);

		if (!strcmp(sec_rec._co_no,"  "))
		{
			sprintf(local_rec.co_short,"%-15.15s"," ");
			sprintf(local_rec.br_short,"%-15.15s"," ");
			sprintf(err_str," %-8.8s ^E %-2.2s ^E %-15.15s ^E %-2.2s ^E %-15.15s ",user," "," "," "," ");
			Dsp_saverec(err_str);
		}
		else
		{
			strcpy(comr_rec.co_co_no,sec_rec._co_no);
			cc = find_rec("comr",&comr_rec,COMPARISON,"r");
			if (!cc)
			{
				if (!strcmp(sec_rec._br_no,"  "))
				{
					sprintf(err_str," %-8.8s ^E %-2.2s ^E %-15.15s ^E %-2.2s ^E %-15.15s ",user,comr_rec.co_co_no,comr_rec.co_co_short_name," "," ");
					Dsp_saverec(err_str);
				}
				else
				{
					strcpy(esmr_rec.es_co_no,sec_rec._co_no);
					strcpy(esmr_rec.es_est_no,sec_rec._br_no);
					cc=  find_rec("esmr",&esmr_rec,COMPARISON,"r");
					if (!cc)
					{
						sprintf(err_str," %-8.8s ^E %-2.2s ^E %-15.15s ^E %-2.2s ^E %-15.15s ",user,comr_rec.co_co_no,comr_rec.co_co_short_name,esmr_rec.es_est_no,esmr_rec.es_short_name);
						Dsp_saverec(err_str);
					}
				}
			}
		}

		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"secure");

	Dsp_srch();
	Dsp_close();
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();
		rv_pr(ML(mlUtilsMess003),29,0,1);
		move(0,1);
		line(80);
		/*print_at(4,10,"Update File = (%s)",filename[0]);*/
		print_at(4,10,ML(mlUtilsMess004),filename[0]);

		move(0,20);
		line(80);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
