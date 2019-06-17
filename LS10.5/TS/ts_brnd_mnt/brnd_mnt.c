/*=====================================================================
|  Copyright (C) 1988 - 1992 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_brnd_mnt.c   )                                |
|  Program Desc  : ( Maintain Telesales Brands.                   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, tsbc,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  comm, tsbc,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 10/02/92         |
|---------------------------------------------------------------------|
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (04/09/97) - Incorporated multilingual conversion  |
|                :            - and DMY4 date.                        |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: brnd_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_brnd_mnt/brnd_mnt.c,v 5.3 2002/07/25 11:17:39 scott Exp $";

#include <pslscr.h>
#include <ml_ts_mess.h>
#include <ml_std_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int	new_code = 0;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_dbt_date"},
		{"comm_crd_date"},
		{"comm_inv_date"},
		{"comm_payroll_date"},
		{"comm_gl_date"}
		};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		long	t_dbt_date;
		long	t_crd_date;
		long	t_inv_date;
		long	t_pay_date;
		long	t_gl_date;
	} comm_rec;

	/*========================================
	| Tele-Sales Brand Code Description File |
	========================================*/
	struct dbview tsbc_list[] ={
		{"tsbc_co_no"},
		{"tsbc_brand"},
		{"tsbc_brand_desc"},
		{"tsbc_stat_flag"},
	};

	int	tsbc_no_fields = 4;

	struct	{
		char	bc_co_no[3];
		char	bc_brand[17];
		char	bc_brand_desc[41];
		char	bc_stat_flag[2];
	} tsbc_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	prev_code[17];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "brand",	 4, 20, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Brand Code :", "",
		 NE, NO, JUSTLEFT, "", "", tsbc_rec.bc_brand},
	{1, LIN, "desc",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Brand Description.", " ",
		YES, NO,  JUSTLEFT, "", "", tsbc_rec.bc_brand_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void    shutdown_prog (void);
void    OpenDB (void);
void    CloseDB (void);
int     spec_valid (int field);
void    update (void);
void    srch_brnd (char* key_val);
int     heading (int scn);

int
main (
 int    argc,
 char*  argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR(vars);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
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

		if (!restart)
			update();
	}
	shutdown_prog();
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
	abc_dbopen("data");

	open_rec("tsbc", tsbc_list, tsbc_no_fields, "tsbc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("tsbc");
	abc_dbclose("data");
}

int
spec_valid (
 int    field)
{
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK("brand"))
	{
		if (SRCH_KEY)
		{
			srch_brnd(temp_str);
			return(0);
		}

		strcpy(tsbc_rec.bc_co_no,comm_rec.tco_no);
		cc = find_rec("tsbc", &tsbc_rec, COMPARISON, "u");
		if (cc) 
			new_code = 1;
		else    
		{
			new_code = 0;
			entry_exit = 1;
		}

		return(0);
	}

	return(0);
}

void
update (void)
{
	/*=============================
	| Add or update area record . |
	=============================*/
	strcpy(tsbc_rec.bc_co_no,comm_rec.tco_no);
	strcpy(tsbc_rec.bc_stat_flag,"0");
	if (new_code)
	{
		cc = abc_add("tsbc",&tsbc_rec);
		if (cc) 
			sys_err("Error in tsbc During (DBADD)", cc, PNAME);
	}
	else
	{
		cc = abc_update("tsbc",&tsbc_rec);
		if (cc) 
			sys_err("Error in tsbc During (DBUPDATE)", cc, PNAME);
	}
        abc_unlock("tsbc");
	strcpy(local_rec.prev_code,tsbc_rec.bc_brand);
}

void
srch_brnd (
 char*  key_val)
{
	work_open();
	save_rec("#Brand Code","#Brand Description");
	strcpy(tsbc_rec.bc_co_no,comm_rec.tco_no);
	sprintf(tsbc_rec.bc_brand, "%-16.16s", key_val);
	cc = find_rec("tsbc",&tsbc_rec,GTEQ,"r");

	while (!cc && 
	       !strcmp(tsbc_rec.bc_co_no,comm_rec.tco_no) &&
	       !strncmp(tsbc_rec.bc_brand, key_val, strlen(key_val)))
	{
		cc = save_rec(tsbc_rec.bc_brand, tsbc_rec.bc_brand_desc);
		if (cc)
			break;

		cc = find_rec("tsbc",&tsbc_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(tsbc_rec.bc_co_no,comm_rec.tco_no);
	sprintf(tsbc_rec.bc_brand, "%-16.16s", temp_str);
	cc = find_rec("tsbc",&tsbc_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in tsbc during (DBFIND)",cc,PNAME);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		rv_pr(ML(mlTsMess054), 25,0,1);
		print_at(0,50,ML(mlTsMess055),local_rec.prev_code);
		move(0,1);
		line(80);

		box(0,3,80,3);

		move(0,20);
		line(80);
		/*print_at(21,0," Company : %s %s ",comm_rec.tco_no,comm_rec.tco_name);*/
		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(80);
	
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
