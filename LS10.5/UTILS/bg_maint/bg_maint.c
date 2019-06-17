/*=====================================================================
|  Copyright (C) 1989, 1990 Logistic Software Limited.                |
|=====================================================================|
|  Program Name  : ( bg_maint.c     )                                 |
|  Program Desc  : ( Maintain Bakcground Processing File          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, comr, esmr, bpro,                           |
|  Database      : (sodb)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  bpro,                                             |
|  Database      : (sodb)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 17/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : (17/10/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (19/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (12/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (05/10/1999)    | Modified  by  : Ramon A. Pacheco |
|                                                                     |
|  Comments      : (19/09/90) - General Update for New Scrgen. S.B.D. |
|                : (12/09/97) - Updated for Multilingual Conversion.  |
|                : (05/10/1999) - Ported to ANSI standards.           |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bg_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/bg_maint/bg_maint.c,v 5.3 2002/07/17 09:58:18 scott Exp $";

#include	<pslscr.h>
#include	<ml_utils_mess.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>

int	new_bpro = 1;

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		char	test_short[16];
	} comm_rec;

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
		char	mr_co_no[3];
		char	mr_co_name[41];
		char	mr_short_name[16];
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

	/*============================
	| System Batch Control file. |
	============================*/
	struct dbview bpro_list[] ={
		{"bpro_co_no"},
		{"bpro_br_no"},
		{"bpro_program"},
		{"bpro_status"},
		{"bpro_lpno"},
		{"bpro_parameters"},
		{"bpro_stat_flag"},
	};

	int bpro_no_fields = 7;

	struct {
		char	bpro_co_no[3];
		char	bpro_br_no[3];
		char	bpro_program[15];
		char	bpro_status[2];
		int	bpro_lpno;
		char	bpro_parameters[31];
		char	bpro_stat_flag[2];
	} bbpro_rec, bpro_rec;

struct	{
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "co_no", 4, 12, CHARTYPE, 
		"AA", "          ", 
		" ", " ", "Company", " ", 
		YES, NO, JUSTRIGHT, "", "", bpro_rec.bpro_co_no}, 
	{1, LIN, "co_name", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", comr_rec.mr_co_name}, 
	{1, LIN, "br_no", 5, 12, CHARTYPE, 
		"AA", "          ", 
		" ", " ", "Branch", " ", 
		YES, NO, JUSTRIGHT, "", "", bpro_rec.bpro_br_no}, 
	{1, LIN, "br_name", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", esmr_rec.es_est_name}, 
	{1, LIN, "program", 6, 12, CHARTYPE, 
		"LLLLLLLLLLLLLL", "          ", 
		" ", "", "Program", " ", 
		YES, NO, JUSTLEFT, "", "", bpro_rec.bpro_program}, 
	{1, LIN, "lpno", 8, 12, INTTYPE, 
		"NN", "          ", 
		" ", "0", "Printer", " Enter 0 if Not Required ", 
		YES, NO, JUSTRIGHT, "0", "99", (char *)&bpro_rec.bpro_lpno}, 
	{1, LIN, "parameters", 9, 12, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Parameters", " ", 
		YES, NO, JUSTLEFT, "", "", bpro_rec.bpro_parameters}, 
	{1, LIN, "status", 10, 12, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Status", "Input Status A(ctive) N(on active).", 
		YES, NO, JUSTLEFT, "AN", "", bpro_rec.bpro_stat_flag}, 
	{0, LIN, "dummy", 4, 20, CHARTYPE, 
		"A", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.dummy}, 

};

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
void	show_company	(char *key_val);
void	show_branch		(char *key_val);
void	show_program	(char *key_val);
void	update			(void);
int		heading			(int scn);


int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	init_scr();
	set_tty(); 
	set_masks();

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		search_ok = 1;
		init_vars(1);	

		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		update();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen("data");

	abc_alias("bbpro","bpro");
	open_rec("comr",comr_list,comr_no_fields,"comr_co_no");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
	open_rec("bpro",bpro_list,bpro_no_fields,"bpro_id_no");
	open_rec("bbpro",bpro_list,bpro_no_fields,"bpro_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose("comr");
	abc_fclose("esmr");
	abc_fclose("bpro");
	abc_fclose("bbpro");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (strcmp(FIELD.label,"co_no") == 0)
	{
		if (last_char == SEARCH)
		{
			show_company(temp_str);
			return(0);
		}

		if (!strcmp(bpro_rec.bpro_co_no,"  "))
		{
			sprintf(comr_rec.mr_co_name,"%-40.40s"," ");
			display_field(label("co_name"));
			return(0);
		}

		strcpy(comr_rec.mr_co_no,bpro_rec.bpro_co_no);
		cc = find_rec("comr",&comr_rec,COMPARISON,"r");
		if (cc)
		{
			/*print_mess(" Company Not On File");*/
			print_mess(ML(mlStdMess130));
			return(1);
		}

		display_field(label("co_name"));
		return(0);
	}

	if (strcmp(FIELD.label,"br_no") == 0)
	{
		if (last_char == SEARCH)
		{
			show_branch(temp_str);
			return(0);
		}

		if (!strcmp(bpro_rec.bpro_br_no,"  "))
		{
			sprintf(esmr_rec.es_est_name,"%-40.40s"," ");
			display_field(label("br_name"));
			return(0);
		}

		strcpy(esmr_rec.es_co_no,bpro_rec.bpro_co_no);
		strcpy(esmr_rec.es_est_no,bpro_rec.bpro_br_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			/*print_mess(" Branch Not On File");*/
			print_mess(ML(mlStdMess073));
			return(1);
		}

		display_field(label("br_name"));
		return(0);
	}

	if (strcmp(FIELD.label,"program") == 0)
	{
		if (last_char == SEARCH)
		{
			show_program(temp_str);
			return(0);
		}

		cc = find_rec("bpro",&bpro_rec,COMPARISON,"u");
		if (!cc)
		{
			entry_exit = 1;
			new_bpro = 0;
		}
		else
			new_bpro = 1;

		return(0);
	}

	if (strcmp(FIELD.label,"lpno") == 0)
	{
		if (last_char == SEARCH)
		{
			bpro_rec.bpro_lpno = get_lpno (0);
			return(0);
		}
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (!valid_lp(bpro_rec.bpro_lpno) && bpro_rec.bpro_lpno != 0)
		{
			print_mess(ML(mlStdMess020));
			return(1);
		}
		return(0);
	}

	if (strcmp(FIELD.label,"parameters") == 0)
	{
		return(0);
	}

	return(0);
}

void
show_company (
 char *	key_val)
{
	work_open();
	save_rec("#Co","#Company Name");
	sprintf(comr_rec.mr_co_no,"%-2.2s",key_val);

	cc = find_rec("comr",&comr_rec,GTEQ,"r");

	while (!cc && !strncmp(comr_rec.mr_co_no,key_val,strlen(key_val)))
	{
		cc = save_rec(comr_rec.mr_co_no,comr_rec.mr_co_name);
		if (cc)
			break;

		cc = find_rec("comr",&comr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf(comr_rec.mr_co_no,"%-2.2s",temp_str);

	cc = find_rec("comr",&comr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in comr during (DBFIND)",cc,PNAME);
}

void
show_branch (
 char *	key_val)
{
	work_open();
	save_rec("#Br","#Branch Name");
	strcpy(esmr_rec.es_co_no,bpro_rec.bpro_co_no);
	sprintf(esmr_rec.es_est_no,"%-2.2s",key_val);

	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");

	while (!cc && !strcmp(esmr_rec.es_co_no,bpro_rec.bpro_co_no) && !strncmp(esmr_rec.es_est_no,key_val,strlen(key_val)))
	{
		cc = save_rec(esmr_rec.es_est_no,esmr_rec.es_est_name);
		if (cc)
			break;

		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(esmr_rec.es_co_no,bpro_rec.bpro_co_no);
	sprintf(esmr_rec.es_est_no,"%-2.2s",temp_str);

	cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in esmr during (DBFIND)",cc,PNAME);
}

void
show_program (
 char *	key_val)
{
	work_open();
	save_rec("#Program","# ");
	strcpy(bbpro_rec.bpro_co_no,bpro_rec.bpro_co_no);
	strcpy(bbpro_rec.bpro_br_no,bpro_rec.bpro_br_no);
	sprintf(bbpro_rec.bpro_program,"%-14.14s",key_val);

	cc = find_rec("bbpro",&bbpro_rec,GTEQ,"r");

	while (!cc && !strcmp(bbpro_rec.bpro_co_no,bpro_rec.bpro_co_no) && !strcmp(bbpro_rec.bpro_br_no,bpro_rec.bpro_br_no) && !strncmp(bbpro_rec.bpro_program,key_val,strlen(key_val)))
	{
		cc = save_rec(bbpro_rec.bpro_program," ");
		if (cc)
			break;

		cc = find_rec("bbpro",&bbpro_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf(bpro_rec.bpro_program,"%-14.14s",temp_str);

	cc = find_rec("bpro",&bpro_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in bpro during (DBFIND)",cc,PNAME);
}

void
update (
 void)
{
	if (new_bpro)
	{
		strcpy(bpro_rec.bpro_status,"S");
		cc = abc_add("bpro",&bpro_rec);
		if (cc)
			sys_err("Error in bpro during (DBADD)",cc,PNAME);
	}
	else
	{
		/*last_char = prmptmsg("U(pdate, I(gnore Changes, D(elete ? \007","UuIiDd",1,2);*/
		last_char = prmptmsg(ML(mlUtilsMess054),"UuIiDd",1,2);

		switch (last_char)
		{
		case	'U':
		case	'u':
			cc = abc_update("bpro",&bpro_rec);
			if (cc)
				sys_err("Error in bpro during (DBUPDATE)",cc,PNAME);
			abc_unlock("bpro");
			break;

		case	'I':
		case	'i':
			abc_unlock("bpro");
			break;

		case	'D':
		case	'd':
			abc_unlock("bpro");
			cc = abc_delete("bpro");
			if (cc)
				sys_err("Error in bpro during (DBDELETE)",cc,PNAME);
			break;
		}

	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		rv_pr(ML(mlUtilsMess055),24,0,1);

		box(0,3,80,7);
		move(1,7);
		line(79);

		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
