/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_spmaint.c                                     |
|  Program Desc  : ( Service Job Spare Parts Maintenance.         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjsp, sjhr,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjsp,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : B.C.Lim.        | Date Written  : 12/12/88         |
|---------------------------------------------------------------------|
|  Date Modified : (23/11/89)      | Modified By : Fui Choo Yap.      |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Not sure about porder field,is it suppose to come  |
|                : from sjdc or not ? field diff size though.         |
|                :                                                    |
|  (23/11/89)  	 : Change MONEYTYPE fields to DOUBLETYPE.             |
|  (10/10/90)    : General Update for New Scrgen. S.B.D.			  |
|  (13/09/97)    : Updated for Multilingual Conversion. 			  |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
| $Log: sj_spmaint.c,v $
| Revision 5.2  2001/08/09 09:17:47  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:46  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:29  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/06/13 05:02:36  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.9  2000/03/23 11:36:03  ramon
| Added sleep() after displaying the error message.
|
| Revision 1.8  1999/11/17 06:40:50  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.7  1999/11/16 05:58:36  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.6  1999/09/29 10:13:08  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/24 05:06:42  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:37  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_spmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_spmaint/sj_spmaint.c,v 5.2 2001/08/09 09:17:47 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

int 	new_sjsp = FALSE;



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
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
	} comm_rec;

	/*=================================
	| Service Job Header Record File. |
	=================================*/
	struct dbview sjhr_list[] ={
		{"sjhr_co_no"},
		{"sjhr_est_no"},
		{"sjhr_dp_no"},
		{"sjhr_order_no"},
		{"sjhr_issue_date"},
	};

	int sjhr_no_fields = 5;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_issue_date;
	} sjhr_rec;

	/*=====================================
	| Service Job Spare Parts Usage File. |
	=====================================*/
	struct dbview sjsp_list[] ={
		{"sjsp_co_no"},
		{"sjsp_est_no"},
		{"sjsp_dp_no"},
		{"sjsp_order_no"},
		{"sjsp_partno"},
		{"sjsp_part_desc"},
		{"sjsp_date"},
		{"sjsp_qty"},
		{"sjsp_uom"},
		{"sjsp_u_cost"},
		{"sjsp_u_sell"},
		{"sjsp_porder_no"}
	};

	int sjsp_no_fields = 12;

	struct {
		char	sp_co_no[3];
		char	sp_est_no[3];
		char	sp_dp_no[3];
		long	sp_order_no;
		char	sp_partno[17];
		char	sp_item_desc[41];
		long	sp_date;
		float	sp_qty;
		char	sp_uom[4];
		double	sp_u_cost;
		double	sp_u_sell;
		char	sp_porder_no[7];
	} sjsp_rec;

	/*==============================
	| Inventory Master File (inmr) |
      	==============================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_quick_code"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_outer_size"},
		{"inmr_on_hand"}
	};

	int inmr_no_fields = 17;

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		long 	mr_hhsi_hash;
		char 	mr_alpha_code[17];
		char 	mr_super_no[17];
		char 	mr_maker_no[17];
		char 	mr_alternate[17];
		char 	mr_class[2];
		char 	mr_description[41];
		char 	mr_category[12];
		char 	mr_quick_code[9];
		char 	mr_serial_item[2];
		char 	mr_costing_flag[2];
		char 	mr_sale_unit[5];
		float 	mr_outer_size;
		float 	mr_on_hand;
	} inmr_rec;

	char	systemDate[11];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	item_no[17];
	char	item_desc[41];
	char	po_no[7];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "service_no", 4, 20, LONGTYPE, 
		"NNNNNNNN", "          ", 
		" ", " ", "Service Job No ", "", 
		NE, NO, JUSTRIGHT, "", "", (char *)&sjsp_rec.sp_order_no}, 
	{1, LIN, "item_no", 6, 20, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item No ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "item_desc", 7, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{1, LIN, "date", 9, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Date ", "Default to current date ", 
		NO, NO, JUSTLEFT, "", "", (char *)&sjsp_rec.sp_date}, 
	{1, LIN, "porder", 10, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "P/O ", "", 
		YES, NO, JUSTLEFT, "", "", sjsp_rec.sp_porder_no}, 
	{1, LIN, "qty", 12, 20, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", "0", "Quantity ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjsp_rec.sp_qty}, 
	{1, LIN, "uom", 13, 20, CHARTYPE, 
		"AAA", "          ", 
		" ", " ", "Unit Of Measure ", " ", 
		NO, NO, JUSTLEFT, "", "", sjsp_rec.sp_uom}, 
	{1, LIN, "u_cost", 14, 20, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Cost Price ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjsp_rec.sp_u_cost}, 
	{1, LIN, "u_sell", 15, 20, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", " ", "Sell Price ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjsp_rec.sp_u_sell}, 

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
void update (void);
void order_search (char *key_val);
int heading (int scn);


/*============================
| Main Processing Routine.   |
============================*/
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

	strcpy (systemDate, DateToString (TodaysDate()));

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		FLD("item_desc") = YES;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		update();
		FLD("item_desc") = NA;

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
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec("sjsp", sjsp_list, sjsp_no_fields, "sjsp_id_no");
	open_rec("inmr", inmr_list, inmr_no_fields, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("inmr");
	abc_fclose("sjsp");
	SearchFindClose ();
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK("service_no"))
	{
		if (SRCH_KEY)
		{
			order_search(temp_str);
			return(0);
		}
		strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		sjhr_rec.hr_order_no = sjsp_rec.sp_order_no;
		cc = find_rec("sjhr",&sjhr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess(ML(mlStdMess122));
			sleep (sleepTime);
			return(1);
		}
		return(0);
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		clear_mess();

		cc = FindInmr (comm_rec.tco_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess(ML(mlStdMess001));
			sleep (sleepTime);
			return(1);
		}
		SuperSynonymError ();

		strcpy(local_rec.item_no, inmr_rec.mr_item_no);
		DSP_FLD("item_no");

		strcpy(local_rec.item_desc,inmr_rec.mr_description);

		DSP_FLD("item_desc");

		strcpy(sjsp_rec.sp_co_no,comm_rec.tco_no);
		strcpy(sjsp_rec.sp_est_no,comm_rec.test_no);
		strcpy(sjsp_rec.sp_dp_no,comm_rec.tdp_no);
		sjsp_rec.sp_order_no = sjhr_rec.hr_order_no;
		sprintf(sjsp_rec.sp_partno,"%-16.16s",local_rec.item_no);
		cc = find_rec("sjsp",&sjsp_rec,COMPARISON,"u");

		if (cc)
			new_sjsp = TRUE;
		else
		{
			new_sjsp = FALSE;
			skip_entry = 7;
			strcpy(local_rec.item_desc,sjsp_rec.sp_item_desc);
			DSP_FLD("date");
			DSP_FLD("qty");
			DSP_FLD("uom");
			DSP_FLD("u_cost");
			DSP_FLD("u_sell");
			DSP_FLD("porder");
		}
		return(0);
	}

	if (LCHECK("item_desc"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.item_desc,inmr_rec.mr_description);
			DSP_FLD("item_desc");
			return(0);
		}
	}

	if (LCHECK("porder"))
	{
/*
		if (last_char == SEARCH)
		{
			porder_search(temp_str);
			return(0);
		}
*/
		return(0);
	}
	return(0);
}

void
update (
 void)
{
	/*===========================
	|  update job spare parts.  |
	===========================*/
	print_at(2,27, ML(mlStdMess035));

	fflush(stdout);
		
	strcpy(sjsp_rec.sp_co_no,comm_rec.tco_no);
	strcpy(sjsp_rec.sp_est_no,comm_rec.test_no);
	strcpy(sjsp_rec.sp_dp_no,comm_rec.tdp_no);
	sprintf(sjsp_rec.sp_partno,"%-16.16s",local_rec.item_no);
	sprintf(sjsp_rec.sp_item_desc,"%-40.40s",local_rec.item_desc);
	if (new_sjsp)
	{
		cc = abc_add("sjsp",&sjsp_rec);
		if (cc)
			sys_err("Error in sjsp During (DBADD)",cc,PNAME);
	}
	else
	{
		cc = abc_update("sjsp",&sjsp_rec);
		if (cc)
			sys_err("Error in sjsp During (DBUPDATE)",cc,PNAME);
		abc_unlock("sjsp");
	}
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
order_search (
 char *key_val)
{
	char	order_str[9];

	work_open();
	save_rec("#Job No","#Issued on");
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol(key_val);
	cc = find_rec("sjhr", &sjhr_rec, GTEQ, "r");
	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && 
		      !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && 
		      !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{
		sprintf(order_str,"%8ld",sjhr_rec.hr_order_no);
		if (strlen(key_val) == 0 || 
			!strncmp(order_str,key_val,strlen(key_val)))
		{
			strcpy (err_str, DateToString(sjhr_rec.hr_issue_date));
			cc = save_rec(order_str,err_str);
			if (cc)
				break;
		}
		cc = find_rec("sjhr", &sjhr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol(temp_str);
	cc = find_rec("sjhr", &sjhr_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in sjhr During (DBFIND)", cc, PNAME);
}

/*
porder_search(key_val)
char	*key_val;
{
	work_open();
	save_rec("#P/O No","#Description");
	strcpy(sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy(sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy(sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sprintf(sjdc_rec.dc_po_no,"%-8.8s",key_val);
	cc = find_rec("sjdc", &sjdc_rec, GTEQ, "r");
	while (!cc && !strcmp(sjdc_rec.dc_co_no,comm_rec.tco_no) && !strcmp(sjdc_rec.dc_est_no,comm_rec.test_no) && !strcmp(sjdc_rec.dc_dp_no,comm_rec.tdp_no) && !strncmp(sjdc_rec.dc_po_no,key_val,strlen(key_val)))
	{
		cc = save_rec(sjdc_rec.dc_po_no,clip(sjdc_rec.dc_desc));
		if (cc)
			break;
		cc = find_rec("sjdc", &sjdc_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(sjdc_rec.dc_co_no,comm_rec.tco_no);
	strcpy(sjdc_rec.dc_est_no,comm_rec.test_no);
	strcpy(sjdc_rec.dc_dp_no,comm_rec.tdp_no);
	sprintf(sjdc_rec.dc_po_no,"%-8.8s",temp_str);
	cc = find_rec("sjdc", &sjdc_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in sjdc During (DBFIND)", cc, PNAME);
}
*/

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		rv_pr(ML(mlSjMess012) ,22,0,1);

		move(0,1);
		line(80);

		box(0,3,80,12);

		move(1,5);
		line(79);
		move(1,8);
		line(79);
		move(1,11);
		line(79);

		move(0,20);
		line(80);
		print_at(21,0, ML(mlStdMess038) ,comm_rec.tco_no,comm_rec.tco_name);
		print_at(22,0, ML(mlStdMess039) ,comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
