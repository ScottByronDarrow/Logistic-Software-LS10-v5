/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( dp_select.c    )                                 |
|  Program Desc  : ( Department Select.                           )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cudp,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cudp,     ,     ,     ,     ,     ,     ,         |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 14/06/88         |
|---------------------------------------------------------------------|
|  Date Modified : (14/06/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (03/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (05/10/90) - General Update for New Scrgen. S.B.D. |
|  				 : (03/09/97) - Incorporated multilingual conversion  |
|                :            - and DMY4 date.                        |
|                :                                                    |
|                                                                     |
| $Log: dp_select.c,v $
| Revision 5.2  2001/08/09 05:13:22  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:14  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:54  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:07  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/21 23:22:59  cam
| Program had been changed to use read_comm () library function, which closes
| comm when it is finished.  Changed code to open and re-read comm record with
| lock before update.
|
| Revision 1.9  1999/12/06 01:47:10  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/16 09:41:54  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.7  1999/10/04 06:52:43  scott
| *** empty log message ***
|
| Revision 1.6  1999/09/29 10:11:03  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:26:54  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 04:11:38  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 02:35:58  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dp_select.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/dp_select/dp_select.c,v 5.2 2001/08/09 05:13:22 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dp_short"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
		char	tdp_short[16];
	} comm_rec;

	/*=========================
	| Department Master File. |
	=========================*/
	struct dbview cudp_list[] ={
		{"cudp_co_no"},
		{"cudp_br_no"},
		{"cudp_dp_no"},
		{"cudp_dp_name"},
		{"cudp_dp_short"},
		{"cudp_location"},
		{"cudp_stat_flag"}
	};

	int cudp_no_fields = 7;

	struct {
		char	dp_co_no[3];
		char	dp_br_no[3];
		char	dp_dp_no[3];
		char	dp_dp_name[41];
		char	dp_dp_short[16];
		char	dp_location[41];
		char	dp_stat_flag[2];
	} cudp_rec;

struct	{
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "dp_no", 4, 15, CHARTYPE, 
		"AA", "          ", 
		" ", comm_rec.tdp_no, "Department No.", " ", 
		YES, NO, JUSTRIGHT, "", "", cudp_rec.dp_dp_no}, 
	{1, LIN, "dp_name", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", cudp_rec.dp_dp_name}, 
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
int		check_dp		(void);
int		spec_valid		(int field);
void	update			(void);
void	save_page		(char *key_val);
int		heading			(int scn);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	SETUP_SCR (vars);

	if (argc != 2)
	{
/*
		printf("Usage : %s [D|S]\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlMenuMess705),argv[0]);
		return (EXIT_SUCCESS);
	}

	init_scr();
	set_tty(); 
	set_masks();

	OpenDB();

	if (argv[1][0] == 'S' && !check_dp())
	{
		abc_unlock("comm");
		shutdown_prog();
		return (EXIT_SUCCESS);
	}

	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	search_ok = 1;
	restart = 0;

	heading(1);
	entry(1);
	if (restart || prog_exit)
	{
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	heading(1);
	scn_display(1);
	edit(1);
	if (restart)
	{
		shutdown_prog();
		return (EXIT_SUCCESS);
	}

	update();
	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*=======================
| Exit program routine. |
=======================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Data Base Files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("cudp", cudp_list, cudp_no_fields, "cudp_id_no");
}

/*========================
| Close Data Base Files. |
========================*/
void
CloseDB (void)
{
	abc_fclose("comm");
	abc_fclose("cudp");
	abc_dbclose("data");
}

int
check_dp (void)
{
	strcpy (cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy (cudp_rec.dp_br_no,comm_rec.tes_no);
	strcpy (cudp_rec.dp_dp_no,comm_rec.tdp_no);
	return (find_rec("cudp",&cudp_rec,COMPARISON,"r"));
}

int
spec_valid (
 int	field)
{
	if (LCHECK("dp_no"))
	{ 
		if (last_char == SEARCH)
		{
                       save_page(temp_str);
		       return(0);
		}

		strcpy(cudp_rec.dp_co_no,comm_rec.tco_no);
		strcpy(cudp_rec.dp_br_no,comm_rec.tes_no);
		cc = find_rec("cudp",&cudp_rec,COMPARISON,"r");
		if (cc) 
		{
			/*print_mess("Department is not on file");*/
			print_mess(ML(mlStdMess084));
			return(1);
		}
		
		display_field(label("dp_name"));
		return(0);	
	}
	return(0);
}

void
update (void)
{
	/*------------------------------
	| Open the comm table and read |
	| the current record with lock |
	------------------------------*/
	open_rec ("comm", comm_list, comm_no_fields, "comm_term");

	cc = find_rec ("comm", &comm_rec, EQUAL, "u");
	if (cc)
		file_err (cc, "comm", "DBFIND");

	strcpy(comm_rec.tdp_no,cudp_rec.dp_dp_no);
	strcpy(comm_rec.tdp_name,cudp_rec.dp_dp_name);
	strcpy(comm_rec.tdp_short,cudp_rec.dp_dp_short);

	cc = abc_update("comm",&comm_rec);
	if (cc) 
		sys_err("Error in comm During (DBUPDATE)",cc,PNAME);
}

void
save_page (
 char *	key_val)
{
        work_open();
	save_rec("#Dp","#Department Name");
	strcpy(cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy(cudp_rec.dp_br_no,comm_rec.tes_no);
	sprintf(cudp_rec.dp_dp_no,"%2.2s",key_val);
	cc = find_rec("cudp",&cudp_rec,GTEQ,"r");
        while (!cc && !strcmp(cudp_rec.dp_co_no,comm_rec.tco_no) && 
		      !strcmp(cudp_rec.dp_br_no,comm_rec.tes_no) && 
		      !strncmp(cudp_rec.dp_dp_no,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(cudp_rec.dp_dp_no,cudp_rec.dp_dp_name);
		if (cc)
		        break;
		cc = find_rec("cudp",&cudp_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;
	strcpy(cudp_rec.dp_co_no,comm_rec.tco_no);
	strcpy(cudp_rec.dp_br_no,comm_rec.tes_no);
	sprintf(cudp_rec.dp_dp_no,"%2.2s",temp_str);
	cc = find_rec("cudp",&cudp_rec,COMPARISON,"r");
	if (cc)
 	        sys_err("Error in cudp During (DBFIND)",cc,PNAME);
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();
		rv_pr(ML(mlMenuMess156),30,1,1);

		box(0,3,80,1);

		move(0,20);
		line(80);
		/*print_at(21,0,"Co : %s - %s",comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		print_at(22,0,"Br : %s - %s",comm_rec.tes_no,comm_rec.tes_name);*/
		print_at(21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at(22,0,ML(mlStdMess039),comm_rec.tes_no,comm_rec.tes_name);

		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
