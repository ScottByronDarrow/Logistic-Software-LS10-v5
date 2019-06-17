/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_vhmaint.c   )                                 |
|  Program Desc  : ( Add / Update Vehicle Records.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjvh,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjvh,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 24/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (20/08/87)      | Modified  by  : Lance Whitford.  |
|  Date Modified : (07/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (23/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Copied off la_vhmaint.c which this program         |
|                : replaces                                           |
|                : Tidy up program to use new screen generator.       |
|     (23/11/89) : Change sjvh_rate to Doubletype.(was Moneytype)     |
|                : (10/10/90) - General Update for New Scrgen. S.B.D. |
|     (13/09/97) : Updated for Multilingual Conversion.               |
|                                                                     |
| $Log: sj_vhmaint.c,v $
| Revision 5.2  2001/08/09 09:17:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:49  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:42  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:07  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/17 06:40:51  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/11/16 05:58:37  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.5  1999/09/29 10:13:09  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/24 05:06:43  scott
| Updated from Ansi
|
| Revision 1.3  1999/06/20 02:30:38  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_vhmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_vhmaint/sj_vhmaint.c,v 5.2 2001/08/09 09:17:50 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*===================
	| Vehicle rate file |
	===================*/
	struct dbview sjvh_list[] ={
		{"sjvh_co_no"},
		{"sjvh_est_no"},
		{"sjvh_code"},
		{"sjvh_vehicle"},
		{"sjvh_rate"}
	};

	int sjvh_no_fields = 5;

	struct {
		char	vh_co_no[3];
		char	vh_est_no[3];
		char	vh_code[4];
		char	vh_vehicle[21];
		double	vh_rate;
	} sjvh_rec;

   	int  	new_item = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "v_code", 4, 17, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Vehicle Code ", " ", 
		YES, NO, JUSTLEFT, "", "", sjvh_rec.vh_code}, 
	{1, LIN, "v_desc", 5, 17, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description ", " ", 
		YES, NO, JUSTLEFT, "", "", sjvh_rec.vh_vehicle}, 
	{1, LIN, "v_rate", 6, 17, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0", "Rate ($/km) ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&sjvh_rec.vh_rate}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void update (void);
void save_page (char *key_val);
int heading (int scn);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv[])
{
	SETUP_SCR (vars);

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

		if (restart)
			continue;

		update();
	}	/* end of input control loop	*/

	shutdown_prog();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sjvh", sjvh_list, sjvh_no_fields, "sjvh_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjvh");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK("v_code"))
	{
		if (SRCH_KEY)
		{
			save_page(temp_str);
			return(0);
		}
		strcpy(sjvh_rec.vh_co_no,comm_rec.tco_no);
		strcpy(sjvh_rec.vh_est_no,comm_rec.test_no);
		cc = find_rec("sjvh", &sjvh_rec, COMPARISON, "u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
		}
		return(0);
	}
			
	return(0);
}

void
update (
 void)
{
	if (new_item)
	{
		cc = abc_add("sjvh",&sjvh_rec);
		if (cc)
			sys_err("Error in sjvh During (DBADD)", cc, PNAME);
	}
	else
	{
		cc = abc_update("sjvh",&sjvh_rec);
		if (cc)
			sys_err("Error in sjvh During (DBUPDATE)", cc, PNAME);
	}
	abc_unlock("sjvh");
}

void
save_page (
 char *key_val)
{
	work_open();
	strcpy(sjvh_rec.vh_co_no,comm_rec.tco_no);
	strcpy(sjvh_rec.vh_est_no,comm_rec.test_no);
	sprintf(sjvh_rec.vh_code,"%-3.3s",key_val);
	save_rec("#Veh","#Vehicle Description");
	cc = find_rec("sjvh", &sjvh_rec, GTEQ, "r");
	while (!cc && !strncmp(sjvh_rec.vh_code,key_val,strlen(key_val)) && 
		      !strcmp(sjvh_rec.vh_co_no,comm_rec.tco_no) && 
		      !strcmp(sjvh_rec.vh_est_no,comm_rec.test_no))
	{
		cc = save_rec(sjvh_rec.vh_code,sjvh_rec.vh_vehicle);
		if (cc)
			break;
		cc = find_rec("sjvh", &sjvh_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(sjvh_rec.vh_co_no,comm_rec.tco_no);
	strcpy(sjvh_rec.vh_est_no,comm_rec.test_no);
	sprintf(sjvh_rec.vh_code,"%-3.3s",temp_str);
	cc = find_rec("sjvh", &sjvh_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in sjvh During (DBFIND)", cc, PNAME);
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
		rv_pr(ML(mlSjMess015) ,29,0,1);

		move(0,1);
		line(80);

		box(0,3,80,3);

		move(0,20);
		line(80);
		move(0,21);
		print_at(21,0, ML(mlStdMess038) ,comm_rec.tco_no,comm_rec.tco_name);
		print_at(22,0, ML(mlStdMess039) ,comm_rec.test_no,comm_rec.test_name);
		move(0,23);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}

    return (EXIT_SUCCESS);
}
