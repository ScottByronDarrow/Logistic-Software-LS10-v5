/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_wcmaint.c   )                                 |
|  Program Desc  : ( Work Centre Maintenance.                     )   |
|                  ( PC04                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pcwc,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pcwc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/06/87         |
|---------------------------------------------------------------------|
|  Date Modified : (14/07/88)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (09/10/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      : (09/10/91) - Changes to pcwc to include br_no and  |
|                : hhwc_hash.                                         |
|                : (16/09/97) - Updated to incorporate multilingual   |
|                :              conversion                            |
|                :                                                    |
|                                                                     |
| $Log: pc_wcmaint.c,v $
| Revision 5.2  2001/08/09 09:14:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:12  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:42  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:11  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:20  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.7  1999/09/29 10:11:42  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 08:26:27  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.5  1999/09/13 07:03:21  marlene
| *** empty log message ***
|
| Revision 1.4  1999/09/09 06:12:38  marlene
| *** empty log message ***
|
| Revision 1.3  1999/06/17 07:40:50  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_wcmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_wcmaint/pc_wcmaint.c,v 5.2 2001/08/09 09:14:54 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*=======================
	| Work Centre Code file |
	=======================*/
	struct dbview pcwc_list[] ={
		{"pcwc_hhwc_hash"},
		{"pcwc_co_no"},
		{"pcwc_br_no"},
		{"pcwc_work_cntr"},
		{"pcwc_name"},
	};

	int	pcwc_no_fields = 5;

	struct	{
		long	wc_hhwc_hash;
		char	wc_co_no[3];
		char	wc_br_no[3];
		char	wc_work_cntr[9];
		char	wc_name[41];
	} pcwc_rec;

   	int  	new_wkcnt = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "work_cntr",	 4, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Work Centre", " ",
		NE, NO,  JUSTLEFT, "", "", pcwc_rec.wc_work_cntr},
	{1, LIN, "wc_desc",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 YES, NO,  JUSTLEFT, "", "", pcwc_rec.wc_name},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*========================
| function prototypes|
=====================*/
int main (int argc, char *argv[]);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void update (void);
void show_pcwc (char *key_val);
int heading (int scn);
/*===========================
| Main Processing Routine . |
===========================*/
int
main(
 int    argc,
 char * argv[])
{
	SETUP_SCR( vars );

	/*---------------------------
	| Stup required parameters. |
	---------------------------*/
	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();

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

		abc_unlock("pcwc");

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		update();
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog(
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB(
 void)
{
	abc_dbopen("data");
	open_rec("pcwc",pcwc_list,pcwc_no_fields,"pcwc_id_no");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB(
 void)
{
	abc_fclose("pcwc");
	abc_dbclose("data");
}

int
spec_valid(
 int field)
{
        if (LCHECK("work_cntr"))
        {
		if (SRCH_KEY)
		{
                       show_pcwc(temp_str);
		       return(0);
		}

		strcpy(pcwc_rec.wc_co_no,comm_rec.tco_no);
		strcpy(pcwc_rec.wc_br_no,comm_rec.test_no);
                cc = find_rec("pcwc",&pcwc_rec,COMPARISON,"w");
		if (cc)
		{
			abc_unlock("pcwc");
			new_wkcnt = TRUE;
		}
		else
		{
			new_wkcnt = FALSE;
			entry_exit = TRUE;
		}
                return(0);
	}

        return(0);             
}

void
update(
 void)
{
	if (new_wkcnt)
        {
                cc = abc_add("pcwc",&pcwc_rec);
                if (cc)
                       sys_err("Error in pcwc During (DBADD)",cc,PNAME);
        }
        else
        {
                cc = abc_update("pcwc",&pcwc_rec);
                if (cc)
                       sys_err("Error in pcwc During (DBUPDATE)",cc,PNAME);
		abc_unlock("pcwc");
        }
}

/*===============================
| Search for pcwc_work_cntr	|
===============================*/
void
show_pcwc(
 char *key_val)
{
        work_open();
	save_rec("#Centre","#Centre Name");
	strcpy(pcwc_rec.wc_co_no,comm_rec.tco_no);
	strcpy(pcwc_rec.wc_br_no,comm_rec.test_no);
	sprintf(pcwc_rec.wc_work_cntr,"%-8.8s",key_val);

	cc = find_rec("pcwc",&pcwc_rec,GTEQ,"r");
        while (!cc && 
	       !strcmp(pcwc_rec.wc_co_no,comm_rec.tco_no) && 
	       !strcmp(pcwc_rec.wc_br_no,comm_rec.test_no) && 
	       !strncmp(pcwc_rec.wc_work_cntr,key_val,strlen(key_val)))
    	{
	        cc = save_rec(pcwc_rec.wc_work_cntr,pcwc_rec.wc_name);
		if (cc)
		        break;

		cc = find_rec("pcwc",&pcwc_rec,NEXT,"r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	strcpy(pcwc_rec.wc_co_no, comm_rec.tco_no);
	strcpy(pcwc_rec.wc_br_no, comm_rec.test_no);
	sprintf(pcwc_rec.wc_work_cntr, "%-8.8s",temp_str);

	cc = find_rec("pcwc",&pcwc_rec,COMPARISON,"r");	
	if (cc)
 	        sys_err("Error in pcwc During (DBFIND)",cc,PNAME);
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
		rv_pr(ML(mlPcMess085),29,0,1);
		move(0,1);
		line(80);

		box(0,3,80,2);

		move(0,20);
		line(80);
		/*print_at(21,0," Company no. : %s   %s",comm_rec.tco_no,comm_rec.tco_name);*/
		print_at(21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_FAILURE);
}
