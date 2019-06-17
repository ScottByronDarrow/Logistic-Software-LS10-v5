/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( dp_maint.c     )                                 |
|  Program Desc  : ( Department Master File Maintenance.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, comr, esmr, cudp,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  esmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (acct)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/02/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/09/97)      | Modified  by  : Roanna Marcelino.|
|  Date Modified : (09/10/1997)    | Modified  by  : Jiggs A. Veloz.  |
|  Date Modified : (09/06/1998)    | Modified  by  : Jiggs A. Veloz.  |
|                                                                     |
|  Comments      : (05/10/90) - General Update for New Scrgen. S.B.D. |
| (05/09/1997) 	 : SEL - Updated for Multilingual Conversion.         |
| (09/10/1997) 	 : BFS - Updated to add 2 new fields - cudp_csh_pref  |
|                :       cudp_chg_pref for the New Series of Inv. Nos.|
| (09/06/1998) 	 : SC 124 - Fixed [Below Minimum error] in Department |
|                : no field.                                          |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: dp_maint.c,v $
| Revision 5.2  2001/08/09 05:13:21  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:12  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:09  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 03:46:52  scott
| Updated as warning sleep missing from message.
|
| Revision 3.0  2000/10/10 12:15:54  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:07  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:47:10  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/16 09:41:54  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.11  1999/09/29 10:11:03  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:26:53  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dp_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dp_maint/dp_maint.c,v 5.2 2001/08/09 05:13:21 scott Exp $";

#define TOTSCNS		1
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

#define		BY_BRANCH	1
#define		BY_DEPART	2

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int	new_dp = 0;
	int		SO_NUMBERS =	BY_BRANCH;

	/*===========================
	| Company Record Structure. |
	===========================*/
	struct dbview comr_list[] = {
		{"comr_co_no"},
		{"comr_co_name"}
	};

	int comr_no_fields = 2;

	struct {
		char	co_no[3];
		char	co_name[41];
	} comr_rec;

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_stat_flag"}
		};

	int esmr_no_fields = 4;

	struct {
		char	es_co_no[3];
		char	es_no[3];
		char	es_name[41];
		char	es_stat_flag[2];
	} esmr_rec;

	/*=========================+
	 | Department Master File. |
	 +=========================*/
#define	CUDP_NO_FIELDS	16

	struct dbview	cudp_list [CUDP_NO_FIELDS] =
	{
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
		{"cudp_dp_short"},
		{"cudp_location"},
		{"cudp_csh_pref"},
		{"cudp_chg_pref"},
		{"cudp_crd_pref"},
		{"cudp_man_pref"},
		{"cudp_nx_chg_no"},
		{"cudp_nx_csh_no"},
		{"cudp_nx_crd_no"},
		{"cudp_nx_man_no"},
		{"cudp_nx_sav_no"},
		{"cudp_stat_flag"}
	};

	struct tag_cudpRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	dp_name [41];
		char	dp_short [16];
		char	location [41];
		char	csh_pref [3];
		char	chg_pref [3];
		char	crd_pref [3];
		char	man_pref [3];
		long	nx_chg_no;
		long	nx_csh_no;
		long	nx_crd_no;
		long	nx_man_no;
		long	nx_sav_no;
		char	stat_flag [2];
	}	cudp_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char 	update[1];
	char	co_no[3];
	char	br_no[3];
	char	dp_no[3];
	} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "co_no",	 3, 21, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Company No.", " ",
		 NE, NO, JUSTRIGHT, "1", "99", local_rec.co_no},
	{1, LIN, "co_name",	 4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Company Name.", " ",
		 NA, NO,  JUSTLEFT, "", "", comr_rec.co_name},
	{1, LIN, "br_no",	 5, 21, CHARTYPE,
		"AA", "          ",
		" ", "", "Branch No.", " ",
		 NE, NO, JUSTRIGHT, "00", "99", local_rec.br_no},
	{1, LIN, "br_name",	 6, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name", " ",
		 NA, NO,  JUSTLEFT, "", "", esmr_rec.es_name},
	{1, LIN, "dp_no",	 8, 21, CHARTYPE,
		"AA", "          ",
		" ", " 1", "Department No.", " ",
		 NE, NO, JUSTRIGHT, "", "", local_rec.dp_no},
	{1, LIN, "dp_name",	9, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Department Name", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.dp_name},
	{1, LIN, "dp_short",	10, 21, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Department Acronym.", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.dp_short},
	{1, LIN, "locat",	11, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Location Description.", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.location},
	{1, LIN, "csh_pref",	13, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Cash Invoice Prefix", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.csh_pref},
	{1, LIN, "next_csh",	13, 61, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Cash Invoice No ", "Input Last Cash Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&cudp_rec.nx_csh_no},
	{1, LIN, "chg_pref",	14, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Charge Invoice Prefix", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.chg_pref},
	{1, LIN, "next_chg",	14, 61, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Charge Invoice No ", "Input Last Charge Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&cudp_rec.nx_chg_no},
	{1, LIN, "man_pref",	15, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Debit Note Prefix    ", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.man_pref},
	{1, LIN, "next_man",	15, 61, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Debit Note No     ", "Input Last Manual Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&cudp_rec.nx_man_no},
	{1, LIN, "crd_pref",	16, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Credit Note  Prefix", " ",
		YES, NO,  JUSTLEFT, "", "", cudp_rec.crd_pref},
	{1, LIN, "next_crd",	16, 61, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Credit Note No ", "Input Last Credit Note #",
		YES, NO, JUSTRIGHT, "", "", (char *)&cudp_rec.nx_crd_no},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*===========================
| Function prototypes .     |
===========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
void	show_cudp		(char * key_val);
void	show_comr		(char * key_val);
void	show_esmr		(char * key_val);
int		update			(void);
int		heading			(int scn);
		

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;
	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);	


	sptr = chk_env("SO_NUMBERS");
	SO_NUMBERS = (sptr == (char *)0) ? BY_BRANCH : atoi(sptr);

	OpenDB ();

	while (prog_exit == 0)
	{
   		entry_exit = 0;
   		edit_exit = 0;
   		prog_exit = 0;
   		restart = 0;
		search_ok = TRUE;
   		new_dp = TRUE;
		init_vars(1);

		if (SO_NUMBERS == BY_BRANCH)
		{
			FLD ("csh_pref")	=	ND;
			FLD ("chg_pref")	=	ND;
			FLD ("crd_pref")	=	ND;
			FLD ("man_pref")	=	ND;
			FLD ("next_csh")	=	ND;
			FLD ("next_chg")	=	ND;
			FLD ("next_crd")	=	ND;
			FLD ("next_man")	=	ND;
		}

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit) 
			break;
			
		/*------------------------
		| Edit screen 1 linear . |
		------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (!restart)
			update();
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

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec ("comr", comr_list, comr_no_fields, "comr_co_no" );
	open_rec ("esmr", esmr_list, esmr_no_fields, "esmr_id_no" );
	open_rec ("cudp", cudp_list, CUDP_NO_FIELDS, "cudp_id_no" );
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose ("comr");
	abc_fclose ("esmr");
	abc_fclose ("cudp");
	abc_dbclose ("data" );
}

int
spec_valid (
 int	field)
{
	/*--------------------------
	| Validate Company Number. |
	--------------------------*/
	if (LCHECK("co_no"))
	{
		if (SRCH_KEY)
		{
			show_comr(temp_str);
			return(0);
		}
		strcpy(comr_rec.co_no,local_rec.co_no);
		cc = find_rec("comr",&comr_rec,COMPARISON,"r");
		if (cc)
		{
			/*--------------------
			| Company Not found. |
			--------------------*/
			print_mess(ML(mlStdMess130));
			sleep (sleepTime);
			return(1);
		}
		DSP_FLD( "co_name" );
		strcpy(err_str,ML(mlStdMess038));
		print_at(20,0,err_str,comr_rec.co_no,comr_rec.co_name);
	}

	/*-------------------------
	| Validate Branch Number. |
	-------------------------*/
	if ( LCHECK("br_no") )
	{
		strcpy( esmr_rec.es_co_no, local_rec.co_no );
		strcpy( esmr_rec.es_no, local_rec.br_no );

		if (SRCH_KEY)
		{
			show_esmr(temp_str);
			return(0);
		}
		cc =  find_rec("esmr",&esmr_rec,COMPARISON,"w");
		if (cc)
		{
			/*-------------------
			| Branch not found. |
			-------------------*/
			print_mess(ML(mlStdMess073));
			sleep (sleepTime);
			return(1);
		}
		DSP_FLD( "br_name" );
		print_at(21,0,ML(mlStdMess039),esmr_rec.es_no,esmr_rec.es_name);
		return(0);
	}
	if (LCHECK("dp_no"))
	{ 
		strcpy(cudp_rec.co_no, local_rec.co_no);
		strcpy(cudp_rec.br_no, local_rec.br_no);
		strcpy(cudp_rec.dp_no,    local_rec.dp_no);

		if (SRCH_KEY)
		{
			show_cudp(temp_str);
			return(0);
		}

		new_dp = find_rec("cudp",&cudp_rec,COMPARISON,"w");
		if ( !new_dp )
		{
			DSP_FLD( "dp_name" );
			DSP_FLD( "dp_short" );
			DSP_FLD( "locat" );
			entry_exit = TRUE;
		}
		return(0);	
	}
	return(0);
}

void
show_cudp (
 char *	key_val)
{
        work_open();
	save_rec("#Dp","#Department Name");
	strcpy(cudp_rec.co_no, local_rec.co_no);
	strcpy(cudp_rec.br_no, local_rec.br_no);
	sprintf(cudp_rec.dp_no,"%2.2s",key_val);
	cc = find_rec("cudp",&cudp_rec,GTEQ,"r");
        while (!cc && !strcmp(cudp_rec.co_no, local_rec.co_no) && 
		      !strcmp(cudp_rec.br_no, local_rec.br_no) && 
		      !strncmp(cudp_rec.dp_no,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(cudp_rec.dp_no,cudp_rec.dp_name);
		if (cc)
		        break;
		cc = find_rec("cudp",&cudp_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(cudp_rec.co_no, local_rec.co_no);
	strcpy(cudp_rec.br_no, local_rec.br_no);
	sprintf(cudp_rec.dp_no,"%2.2s",temp_str);
	cc = find_rec("cudp",&cudp_rec,COMPARISON,"r");
	if (cc)
 	        sys_err("Error in cudp During (DBFIND)",cc,PNAME);
}

/*==================
| Search for comr. |
==================*/
void
show_comr (
 char *	key_val)
{
	work_open();
	save_rec("#Co.","#Company Name.");
	sprintf(comr_rec.co_no,"%-2.2s",key_val);
	cc = find_rec("comr",&comr_rec,GTEQ,"r");
	while (!cc && !strncmp(comr_rec.co_no,key_val,strlen(key_val)))
	{
		cc = save_rec(comr_rec.co_no,comr_rec.co_name);
		if (cc)
			break;

		cc = find_rec("comr",&comr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf(comr_rec.co_no,"%-2.2s", temp_str);
	cc = find_rec("comr",&comr_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "comr", "DBFIND" );
}

/*==================
| Search for esmr. |
==================*/
void
show_esmr (
 char *	key_val)
{
	work_open();
	save_rec("#Br.","#Branch Name.");
	strcpy( esmr_rec.es_co_no, local_rec.co_no);
	sprintf(esmr_rec.es_no,"%-2.2s",key_val);
	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp( esmr_rec.es_co_no, local_rec.co_no) &&
		      !strncmp(esmr_rec.es_no,key_val,strlen(key_val)))
	{
		cc = save_rec(esmr_rec.es_no,esmr_rec.es_name);
		if (cc)
			break;

		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy( esmr_rec.es_co_no, local_rec.co_no);
	sprintf(esmr_rec.es_no,"%-2.2s", temp_str);
	cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "esmr", "DBFIND" );
}

int
update (void)
{
	clear();

	/*--------------------------------
	| Update existing Branch Record. |
	--------------------------------*/
	if ( !new_dp )
	{
		cc = abc_update("cudp",&cudp_rec);
		if (cc)
			sys_err("Error in cudp During (DBUPDATE)", cc, PNAME);
	}
	else 
	{
		cc = abc_add("cudp",&cudp_rec);
		if (cc)
			sys_err("Error in cudp During (DBADD)", cc, PNAME);
	}
		
	abc_unlock("cudp");
	return(0);
}

int
heading (
 int	scn)
{
	int	s_size = 80;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		/*------------------------------------
		| Department Master File Maintenance |
		------------------------------------*/
		rv_pr(ML(mlMenuMess102),16,0,1);
		move(0,1);
		line(s_size);

		if (scn == 1)
			box(0,2,80,(SO_NUMBERS == BY_BRANCH) ? 9 : 14);


		move(1,7);
		line(79);
		if (SO_NUMBERS == BY_DEPART)
		{
			move(1,12);
			line(79);
		}

		move(0,19);
		line(s_size);

		strcpy(err_str, ML(mlStdMess038));
		print_at(20,0,err_str,comr_rec.co_no,comr_rec.co_name);

		strcpy(err_str,ML(mlStdMess039));
		print_at(21,0,err_str,esmr_rec.es_no,clip(esmr_rec.es_name));
		move(0,22);
		line(s_size);
		move(1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
