/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_srmaint.c                                     |
|  Program Desc  : ( Serviceperson Rate Maintenance               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjlr, sjsr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjsr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 21/08/87         |
|---------------------------------------------------------------------|
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (09/09/91)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Include new scrgen and general program tidy up     |
|                : Remove local_re.co_no,est_no & dp_no.              |
|                :                                                    |
|     (23/11/89) : Remove sjlr_cost fields which are not used here.   |
|                : (10/10/90) - General Update for New Scrgen. S.B.D. |
|                : (09/09/91) - General look into etc etc.            |
|     (13/09/97) : Updated for Multilingual Conversion.               |
|                :                                                    |
|                :                                                    |
| $Log: sj_srmaint.c,v $
| Revision 5.2  2001/08/09 09:17:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.7  1999/11/17 06:40:51  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/11/16 05:58:37  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.5  1999/09/29 10:13:08  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/24 05:06:42  scott
| Updated from Ansi
|
| Revision 1.3  1999/06/20 02:30:37  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_srmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_srmaint/sj_srmaint.c,v 5.2 2001/08/09 09:17:48 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

int 	new_item = FALSE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
	} comm_rec;

	/*==========================
	| Serviceperson Rate File  |
	==========================*/
	struct dbview sjsr_list[] ={
		{"sjsr_co_no"},
		{"sjsr_est_no"},
		{"sjsr_dp_no"},
		{"sjsr_code"},
		{"sjsr_name"},
		{"sjsr_lb_rt_code"},
	};

	int sjsr_no_fields = 6;

	struct {
		char	sr_co_no[3];
		char	sr_est_no[3];
		char	sr_dp_no[3];
		char	sr_code[11];
		char	sr_name[26];
		char	sr_lb_rt_code[3];
	} sjsr_rec;
  
	/*====================
	| Labour rates file  |
	====================*/
	struct dbview sjlr_list[] ={
		{"sjlr_co_no"},
		{"sjlr_est_no"},
		{"sjlr_dp_no"},
		{"sjlr_code"},
		{"sjlr_descr"},
	};

	int sjlr_no_fields = 5;

	struct {
		char	lr_co_no[3];
		char	lr_est_no[3];
		char	lr_dp_no[3];
		char	lr_code[3];
		char	lr_descr[26];
	} sjlr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char 	emp_code[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "employee_code", 4, 19, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Employee Code", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.emp_code}, 
	{1, LIN, "name", 5, 19, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Name", " ", 
		YES, NO, JUSTLEFT, "", "", sjsr_rec.sr_name}, 
	{1, LIN, "labour_code", 6, 19, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Labour rate code", " ", 
		YES, NO, JUSTLEFT, "", "", sjsr_rec.sr_lb_rt_code}, 
	{1, LIN, "sj_desc", 7, 19, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Description", " ", 
		NA, NO, JUSTLEFT, "", "", sjlr_rec.lr_descr}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int read_lr (void);
void update (void);
void save_page (char *key_val);
void show_sjlr (char *key_val);
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

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
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
	open_rec("sjlr", sjlr_list, sjlr_no_fields, "sjlr_id_no");
	open_rec("sjsr", sjsr_list, sjsr_no_fields, "sjsr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjlr");
	abc_fclose("sjsr");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	/*------------------------
	| Validate employee code |
	------------------------*/
        if (LCHECK("employee_code"))
        {
		if (last_char == SEARCH)
		{
                       save_page(temp_str);
		       return(0);
		}
		strcpy(sjsr_rec.sr_co_no,comm_rec.tco_no);
		strcpy(sjsr_rec.sr_est_no,comm_rec.tes_no);
		strcpy(sjsr_rec.sr_dp_no,comm_rec.tdp_no);
		strcpy(sjsr_rec.sr_code,local_rec.emp_code);
		cc = find_rec("sjsr",&sjsr_rec,COMPARISON,"u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
			if (read_lr())
				return(1);
			scn_display(1);
		}
                return(0);
	}
	/*----------------------
	| Validate labour code |
	----------------------*/
        if (LCHECK("labour_code"))
        {
		if (last_char == SEARCH)
		{
                       show_sjlr(temp_str);
		       return(0);
		}
		if (read_lr())
			return(1);

		DSP_FLD( "sj_desc" );
                return(0);
	}
        return(0);             
}

int
read_lr (
 void)
{
	strcpy(sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy(sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy(sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	strcpy(sjlr_rec.lr_code,sjsr_rec.sr_lb_rt_code);
	cc = find_rec("sjlr",&sjlr_rec,COMPARISON,"r");
	if (cc)
	{
		/*sprintf(err_str," Labour rate code %s is not on file",
							sjsr_rec.sr_lb_rt_code);*/
		errmess(ML(mlSjMess013));
		return(1);
	}
	return(0);
}

void
update (
 void)
{
	strcpy(sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy(sjsr_rec.sr_est_no,comm_rec.tes_no);
	strcpy(sjsr_rec.sr_dp_no,comm_rec.tdp_no);
	sprintf(sjsr_rec.sr_code,"%-10.10s",local_rec.emp_code);
	
	if (new_item)
        {
                cc = abc_add("sjsr",&sjsr_rec);
                if (cc)
                       sys_err("Error in sjsr During (DBADD)",cc,PNAME);
        }
        else
        {
                cc = abc_update("sjsr",&sjsr_rec);
                if (cc)
                       sys_err("Error in sjsr During (DBUPDATE)",cc,PNAME);
        }
}

/*====================
| Search for sr_code |
====================*/
void
save_page (
 char *key_val)
{
        work_open();
	save_rec("#Serv Code","#Service Person");
	strcpy(sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy(sjsr_rec.sr_est_no,comm_rec.tes_no);
	strcpy(sjsr_rec.sr_dp_no,comm_rec.tdp_no);
	sprintf(sjsr_rec.sr_code,"%-10.10s",key_val);

	cc = find_rec("sjsr", &sjsr_rec, GTEQ, "r");
        while (!cc && !strcmp(sjsr_rec.sr_co_no,comm_rec.tco_no) && 
		      !strcmp(sjsr_rec.sr_est_no,comm_rec.tes_no) && 
		      !strcmp(sjsr_rec.sr_dp_no,comm_rec.tdp_no) && 
		      !strncmp(sjsr_rec.sr_code,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(sjsr_rec.sr_code, sjsr_rec.sr_name);                       
		if (cc)
		        break;
		cc = find_rec("sjsr",&sjsr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;
	strcpy(sjsr_rec.sr_co_no,comm_rec.tco_no);
	strcpy(sjsr_rec.sr_est_no,comm_rec.tes_no);
	strcpy(sjsr_rec.sr_dp_no,comm_rec.tdp_no);
	sprintf(sjsr_rec.sr_code,"%-10.10s",temp_str);
	cc = find_rec("sjsr", &sjsr_rec, COMPARISON, "r");
	if (cc)
 	        sys_err("Error in sjsr During (DBFIND)", cc, PNAME);
}

/*====================
| Search for lr_code |
====================*/
void
show_sjlr (
 char *key_val)
{
        work_open();
	save_rec("#Labour","#Labour Description");
	strcpy(sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy(sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy(sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	sprintf(sjlr_rec.lr_code,"%-10.10s",key_val);

	cc = find_rec("sjlr", &sjlr_rec, GTEQ, "r");
        while (!cc && !strcmp(sjlr_rec.lr_co_no,comm_rec.tco_no) && 
		      !strcmp(sjlr_rec.lr_est_no,comm_rec.tes_no) && 
		      !strcmp(sjlr_rec.lr_dp_no,comm_rec.tdp_no) && 
		      !strncmp(sjlr_rec.lr_code,key_val,strlen(key_val)))
    	{                        
	        cc = save_rec(sjlr_rec.lr_code, sjlr_rec.lr_descr);                       
		if (cc)
		        break;
		cc = find_rec("sjlr",&sjlr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	        return;
	strcpy(sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy(sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy(sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	sprintf(sjlr_rec.lr_code,"%-10.10s",temp_str);
	cc = find_rec("sjlr", &sjlr_rec, COMPARISON, "r");
	if (cc)
 	        sys_err("Error in sjlr During (DBFIND)", cc, PNAME);
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
		rv_pr(ML(mlSjMess014),24,0,1);

		move(0,1);
		line(80);

		box(0,3,80,4);
		move(0,20);
		line(80);
		print_at(21,0,  ML(mlStdMess038) ,comm_rec.tco_no,comm_rec.tco_name);
		print_at(22,0,  ML(mlStdMess039) ,comm_rec.tes_no,clip(comm_rec.tes_name));
		print_at(22,45, ML(mlStdMess085) ,comm_rec.tdp_no ,clip(comm_rec.tdp_name));

		move(0,23);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
