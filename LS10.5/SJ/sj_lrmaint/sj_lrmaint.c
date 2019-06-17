/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_lrmaint.c                                     |
|  Program Desc  : ( Labour Rate Maintenance                      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjlr,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjlr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (labr)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 04/08/87         |
|---------------------------------------------------------------------|
|  Date Modified : 27/08/87        | Modified By   : Lance Whitford.  |
|  Date Modified : 07/12/88        | Modified By   : Bee Chwee Lim.   |
|  Date Modified : 23/11/89          Modified By   : Fui Choo Yap.    |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz.  |
|                                                                     |
|  Comments      : sjlr now holds extras previously held in sjer      |
|                : Tidy up program to use new screen generator.       |
|                :                                                    |
|     (23/11/89) : Change rates from Moneytype to Doubletype.         |
|                : (10/10/90) - General Update for New Scrgen. S.B.D. |
|     (11/09/97) : Updated for Multilingual Conversion.               |
|                :                                                    |
|                                                                     |
| $Log: sj_lrmaint.c,v $
| Revision 5.2  2001/08/09 09:17:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:39  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:31  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:30  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:59  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.8  1999/11/17 06:40:49  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.7  1999/11/16 05:58:34  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.6  1999/09/29 10:13:04  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/24 05:06:38  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:33  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_lrmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_lrmaint/sj_lrmaint.c,v 5.2 2001/08/09 09:17:41 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>
#include    <std_decs.h>

int 	new_item = 0;

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
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*====================
	| Labour rates file  |
	====================*/
	struct dbview sjlr_list[] ={
		{"sjlr_co_no"},
		{"sjlr_est_no"},
		{"sjlr_dp_no"},
		{"sjlr_code"},
		{"sjlr_descr"},
		{"sjlr_cost_hr"},
		{"sjlr_ovhd_hr"},
		{"sjlr_profit_hr"},
		{"sjlr_uom"}
	};

	int sjlr_no_fields = 9;

	struct {
		char	lr_co_no[3];
		char	lr_est_no[3];
		char	lr_dp_no[3];
		char	lr_code[3];
		char	lr_descr[26];
		double	lr_cost_hr;
		double	lr_ovhd_hr;
		double	lr_profit_hr;
		char	lr_uom[4];
	} sjlr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "code", 4, 22, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Code ", " ", 
		YES, NO, JUSTLEFT, "", "", sjlr_rec.lr_code}, 
	{1, LIN, "desc", 5, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Description ", " ", 
		YES, NO, JUSTLEFT, "", "", sjlr_rec.lr_descr}, 
	{1, LIN, "hr_rate", 6, 22, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", "Cost Per Hour ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjlr_rec.lr_cost_hr}, 
	{1, LIN, "hr_rate", 7, 22, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", "Overhead Per Hour ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjlr_rec.lr_ovhd_hr}, 
	{1, LIN, "hr_rate", 8, 22, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "", "Profit Per Hour ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjlr_rec.lr_profit_hr}, 
	{1, LIN, "unit", 9, 22, CHARTYPE, 
		"AAA", "          ", 
		" ", " ", "Unit Of Measure ", " ", 
		YES, NO, JUSTRIGHT, "", "", sjlr_rec.lr_uom}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*====================
| Function Prototype |
=====================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void update (void);
void save_page (char *key_val);
int heading (int scn);
int spec_valid (int field);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char *argv[])
{
	SETUP_SCR (vars);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

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
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		update ();
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
	open_rec ("sjlr", sjlr_list, sjlr_no_fields, "sjlr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjlr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*---------------
	| Validate code |
	---------------*/
	if (LCHECK("code"))
	{
		if (SRCH_KEY)
		{
		   save_page (temp_str);
		   return (EXIT_SUCCESS);
		}
		strcpy (sjlr_rec.lr_co_no,comm_rec.tco_no);
		strcpy (sjlr_rec.lr_est_no,comm_rec.tes_no);
		strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no);
		cc = find_rec ("sjlr",&sjlr_rec,COMPARISON,"u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
			scn_display (1);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

void
update (
 void)
{
	strcpy (sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	if (new_item)
	{
		cc = abc_add ("sjlr",&sjlr_rec);
		if (cc)
		   sys_err ("Error in sjlr During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update ("sjlr",&sjlr_rec);
		if (cc)
		   sys_err ("Error in sjlr During (DBUPDATE)",cc,PNAME);
	}
}

/*====================
| Search for lr_code |
====================*/
void
save_page (
 char *key_val)
{
    work_open ();
	strcpy (sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	sprintf (sjlr_rec.lr_code,"%-10.10s",key_val);
	save_rec ("#Code","#Labour Code Description");                       

	cc = find_rec ("sjlr", &sjlr_rec, GTEQ, "r");
        while (!cc && !strcmp (sjlr_rec.lr_co_no,comm_rec.tco_no) && 
		      !strcmp (sjlr_rec.lr_est_no,comm_rec.tes_no) && 
		      !strcmp (sjlr_rec.lr_dp_no,comm_rec.tdp_no) && 
		      !strncmp (sjlr_rec.lr_code,key_val,strlen(key_val)))
    	{
		cc = save_rec (sjlr_rec.lr_code, sjlr_rec.lr_descr);                       
		if (cc)
		        break;
		cc = find_rec ("sjlr",&sjlr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (sjlr_rec.lr_co_no,comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.tes_no);
	strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no);
	sprintf (sjlr_rec.lr_code,"%-10.10s",temp_str);
	cc = find_rec ("sjlr", &sjlr_rec, COMPARISON, "r");
	if (cc)
 	        sys_err ("Error in sjlr During (DBFIND)", cc, PNAME);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		
		rv_pr (ML(mlSjMess003),28,0,1);
		move (0,1);
		line (80);

		box (0,3,80,6);
		move (0,20);
		line (80);
		print_at (21,0,ML(mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at (22,0,ML(mlStdMess039),comm_rec.tes_no,comm_rec.tes_name);
		move (0,23);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
