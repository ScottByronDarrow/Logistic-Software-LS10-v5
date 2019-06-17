/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mn_acdmaint.c )                                  |
|  Program Desc  : ( Menu System Access Description Maintenance.  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, mnac,     ,     ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  mnac,     ,     ,     ,     ,     ,     ,         |
|  Database      : (menu)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 22/03/88         |
|---------------------------------------------------------------------|
|  Date Modified : (23/05/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (03/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (31/08/1999)    | Modified  by  : Alvin Misalucha. |
|                                                                     |
|  Comments      :                                                    |
|  (23/05/91)    : Security Access codes are now variable length      |
|                : strings of up to 8 characters made up of lower     |
|                : case letters, (0 - 9) or '*'.                      |
|  (03/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|  (31/08/1999)  : Converted to ANSI format.                          |
|                :                                                    |
|                                                                     |
| $Log: acdmaint.c,v $
| Revision 5.2  2001/08/09 05:13:33  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:31  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:09  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:19  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/12/06 01:47:16  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/16 09:41:56  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.6  1999/09/29 10:11:08  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:27:01  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:40  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:36:51  scott
| Update to add log + change database names + misc clean up.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: acdmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/mn_acdmaint/acdmaint.c,v 5.2 2001/08/09 05:13:33 scott Exp $";

#define MAXSCNS		1
#define MAXWIDTH 	100
#include	<pslscr.h>
#include	<ml_menu_mess.h>
#include	<ml_std_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	new_code = 0,	
		envDbFind = 0,
		rc;

	char	branchNo[3];

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
		int  termno;
		char tco_no[3];
		char tco_name[41];
		char tes_no[3];
		char tes_name[41];
		long t_dbt_date;
	} comm_rec;


	/*======================================
	| Menu System Access Description File. |
	======================================*/
	struct dbview mnac_list[] ={
		{"mnac_code"},
		{"mnac_description"}
	};

	int mnac_no_fields = 2;

	struct {
		char	code[9];
		char	desc[31];
	} mnac_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	ac_code[9];
	char	dummy[11];
}local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "code", 4, 15, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "Access Code ", "", 
		NE, NO, JUSTLEFT, "0123456789abcdefghijklmnopqrstuvwxyz*", "", mnac_rec.code}, 
	{1, LIN, "desc", 5, 15, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		NO, NO, JUSTLEFT, "", "", mnac_rec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*==========================
| Function prototypes.     |
==========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
int		update			(void);
void	save_page		(char * key_val);
int		heading			(int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	OpenDB ();

	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	sprintf(local_rec.ac_code, "%-8.8s", " ");

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
		if (prog_exit)
			break;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		update();
	}	/* end of input control loop	*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program shutdown.      |
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

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("mnac", mnac_list, mnac_no_fields, "mnac_code");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("mnac");
	abc_dbclose("data");
}

int
spec_valid (
 int	field)
{
	if (LCHECK("code"))
	{
		if (SRCH_KEY)
		{
		   save_page(temp_str);
		   return(0);
		}

		if (strchr(temp_str, '*'))
		{
			sprintf(mnac_rec.code, "%-8.8s", "*");
			sprintf(temp_str, "%-8.8s", "*");
		}

		cc = find_rec("mnac",&mnac_rec,COMPARISON,"u");
		if (cc)
			new_code = TRUE;
		else
		{
			new_code = FALSE;
			entry_exit = TRUE;
			DSP_FLD("desc");
		}
		return(0);
	}

	return(FALSE);             
}

int
update (void)
{
	if (new_code)
	{
		cc = abc_add("mnac",&mnac_rec);
		if (cc)
		   sys_err("Error in mnac During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update("mnac",&mnac_rec);
		if (cc)
		   sys_err("Error in mnac During (DBUPDATE)",cc,PNAME);
	}
	abc_unlock("mnac");
	return (cc);
}

/*=======================
| Search for accs_code  |
=======================*/
void
save_page (
 char *	key_val)
{
	work_open();
	save_rec("#  Code  ","#Description                             ");
	sprintf(mnac_rec.code,"%-8.8s",key_val);
	cc = find_rec("mnac", &mnac_rec, GTEQ, "r");
	while (!cc && !strncmp(mnac_rec.code,key_val,strlen(key_val)))
	{                        
		if (!strncmp(mnac_rec.code, "ERROR", 5))
		{
			cc = find_rec("mnac",&mnac_rec,NEXT,"r");
			continue;
		}

	        cc = save_rec(mnac_rec.code,mnac_rec.desc); 
		if (cc)
		        break;
		cc = find_rec("mnac",&mnac_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	sprintf(mnac_rec.code,"%-8.8s",temp_str);
	cc = find_rec("mnac", &mnac_rec, COMPARISON, "r");
	if (cc)
 	        sys_err("Error in mnac During (DBFIND)", cc, PNAME);
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
		rv_pr(ML(mlMenuMess112),20,0,1);
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
