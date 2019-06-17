/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_sdmaint.c                                     |
|  Program Desc  : ( Service Job Details Maintenance              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjhr, sjsd, cumr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sjsd,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 05/08/87         |
|---------------------------------------------------------------------|
|  Date Modified : 01/12/88	       | Modified By   : B.C.Lim.         |
|  Date Modified : (10/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|  (10/10/90)    : General Update for New Scrgen. S.B.D. 			  |
|  (13/09/97)    : Updated for Multilingual Conversion.				  |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
| $Log: sj_sdmaint.c,v $
| Revision 5.2  2001/08/09 09:17:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:37  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:46  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:10:04  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/11/17 06:40:50  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/16 05:58:35  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.7  1999/09/29 10:13:07  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/24 05:13:09  scott
| Updated for Ansi project
|
| Revision 1.5  1999/09/24 05:06:41  scott
| Updated from Ansi
|
| Revision 1.4  1999/06/20 02:30:36  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_sdmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_sdmaint/sj_sdmaint.c,v 5.2 2001/08/09 09:17:45 scott Exp $";

#define MAXLINES	20
#define TABLINES	10
#define DELETE_LINE
#define INSERT_LINE

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

	char	branchNumber [3];

	int	envDbCo = 0,
		envDbFind  = 0,
		new_item = 0;

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
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_issue_date"},
	};

	int sjhr_no_fields = 7;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		long	hr_chg_client;
		long	hr_end_client;
		long	hr_issue_date;
	} sjhr_rec;

	/*==========================
	| service job details file |
	===========================*/
	struct dbview sjsd_list[] ={
		{"sjsd_co_no"},
		{"sjsd_est_no"},
		{"sjsd_dp_no"},
		{"sjsd_order_no"},
		{"sjsd_line_no"},
		{"sjsd_detail"}
	};

	int sjsd_no_fields = 6;

	struct {
		char	sd_co_no[3];
		char	sd_est_no[3];
		char	sd_dp_no[3];
		long	sd_order_no;
		int	sd_line_no;
		char	sd_detail[71];
	} sjsd_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	int cumr_no_fields = 6;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
	} cumr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	c_client[7];
	char	e_client[7];
	char	chg_client[41];
	char	end_client[41];
	char	order_no[9];
	char	job_detail[71];
	double	lr_cost_est;		/*  Money field  */
	double	lr_cost_act;		/*  Money field  */
	double	lr_cost_chg;		/*  Money field  */
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "service_no", 4, 18, LONGTYPE, 
		"NNNNNNNN", "          ", 
		" ", "0", "Service Job No ", "Please enter a valid Service Job #. [FN4] to Search.", 
		NE, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "charge_to", 5, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge To ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 5, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 6, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Client ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 6, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{2, TAB, "detail", MAXLINES, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "                        J o b  D e t a i l s                            ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.job_detail}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};


#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void get_details (int scn);
int get_end_client (void);
int insert_line (void);
int delete_line (void);
void update (void);
void ord_srch (char *key_val);
int heading (int scn);


/*=========================
| Main Processing Routine |
=========================*/
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

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	OpenDB();

	strcpy(branchNumber , (!envDbCo) ? " 0" : comm_rec.test_no);

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

		/*-------------------------------
		| Enter screen 2 tabular input .|	
		-------------------------------*/
		if (new_item)
		{
			heading(2);
			entry(2);
			if (restart)
				continue;
		}

		edit_all();
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
	open_rec("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec("sjsd", sjsd_list, sjsd_no_fields, "sjsd_id_no");
	open_rec("cumr", cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no" : 
								 "cumr_id_no3");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("sjhr");
	abc_fclose("cumr");
	abc_fclose("sjsd");
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
			ord_srch(temp_str);
			return(0);
		}
		strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		cc = find_rec("sjhr",&sjhr_rec,COMPARISON,"u");
		if (cc)
		{
			errmess(ML(mlSjMess004));
			return(1);
		}
		abc_selfield("cumr","cumr_hhcu_hash");
		cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_chg_client);
		if (cc)
		{
			errmess(ML(mlStdMess021));
			return(1);
		}
		strcpy(local_rec.c_client,cumr_rec.cm_dbt_no);
		strcpy(local_rec.chg_client,cumr_rec.cm_name);
		get_end_client();
		abc_selfield("cumr",(!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
		get_details(cur_screen);
		scn_display(1);
		entry_exit = TRUE;

		return(0);
	}

	if (LCHECK("end_client"))
	{
		if (vars[label("end_client")].required == NA)
			return(0);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}
		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNumber );
		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.e_client));
		cc = find_rec("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess(ML(mlStdMess021));
			return(1);
		}
		strcpy(local_rec.end_client,cumr_rec.cm_name);
		display_field(label("end_client_name"));
		return(0);
	}
	/*-----------------------------
	| Validate Detail text lines. |
	-----------------------------*/
	if (LCHECK("detail"))
	{
		if (last_char == DELLINE)
			return(delete_line());

		if (last_char == INSLINE)
			return(insert_line());

		/*-------------------------------
		| First character is a '\'	|
		| \D	- delete current line	|
		| \I	- insert before current	|
		-------------------------------*/
		if (local_rec.job_detail[0] == '\\')
		{
			switch (local_rec.job_detail[1])
			{
			case	'D':
			case	'd':
				return(delete_line());
				break;
	
			case	'I':
			case	'i':
				return(insert_line());
				break;
	
			default:
				break;
			}
		}
	}
	return(0);
}

void
get_details (
 int scn)
{
	/*======================
	|  Loading job detail |
	======================*/
	scn_set(2);
	lcount[2] = 0;

	strcpy(sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy(sjsd_rec.sd_est_no,comm_rec.test_no);
	strcpy(sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = 0;
	cc = find_rec("sjsd",&sjsd_rec,GTEQ,"r");

	while (!cc && !strcmp(sjsd_rec.sd_co_no,comm_rec.tco_no) && 
		      !strcmp(sjsd_rec.sd_est_no,comm_rec.test_no) && 
		      !strcmp(sjsd_rec.sd_dp_no,comm_rec.tdp_no) && 
		      sjsd_rec.sd_order_no == sjhr_rec.hr_order_no)
	{
		strcpy(local_rec.job_detail,sjsd_rec.sd_detail);
		putval(lcount[2]++);
		cc = find_rec("sjsd",&sjsd_rec,NEXT,"r");
	}
	if (scn != cur_screen)
		scn_set(scn);
}

int
get_end_client (
 void)
{
	if (sjhr_rec.hr_end_client == 0L)
		return(0);

	cc = find_hash("cumr",&cumr_rec,COMPARISON,"r",sjhr_rec.hr_end_client);
	if (cc)
	{
		errmess(ML(mlStdMess021));
		return(1);
	}
	
	strcpy(local_rec.e_client,cumr_rec.cm_dbt_no);
	strcpy(local_rec.end_client,cumr_rec.cm_name);
	return(0);
}

int
insert_line (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess(ML(mlStdMess005));
		sleep(2);
		return(1);
	}

	if (lcount[2] >= vars[label("detail")].row)
	{
		print_mess(ML(mlStdMess076));
		sleep(2);
		return(1);
	}
	for (i = line_cnt,line_cnt = lcount[2];line_cnt > i;line_cnt--)
	{
		getval(line_cnt - 1);
		putval(line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display();
	}
	lcount[2]++;
	line_cnt = i;

	sprintf(local_rec.job_detail,"%-70.70s"," ");
	putval(line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry(cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	line_cnt = i;
	getval(line_cnt);
	return(0);
}

/*==============
| Delete line. |
==============*/
int
delete_line (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		print_mess(ML(mlStdMess005));
		return(1);
	}

	lcount[2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[2];line_cnt++)
	{
		getval(line_cnt + 1);
		putval(line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display();
	}

	sprintf(local_rec.job_detail,"%-70.70s"," ");
	putval(line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display();
	
	line_cnt = i;
	getval(line_cnt);
	return(0);
}

void
update (
 void)
{
	int	i = 0;

	/*=====================
	|  update job details |
	=====================*/
	scn_set(2);

	for (i = 0;i < lcount[2];i++)
	{
		getval(i);
		strcpy(sjsd_rec.sd_co_no,comm_rec.tco_no);
		strcpy(sjsd_rec.sd_est_no,comm_rec.test_no);
		strcpy(sjsd_rec.sd_dp_no,comm_rec.tdp_no);
		sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
		sjsd_rec.sd_line_no = i;
		cc = find_rec("sjsd",&sjsd_rec,COMPARISON,"u");
		if (cc == 0)
		{
			strcpy(sjsd_rec.sd_detail,local_rec.job_detail);
			cc = abc_update("sjsd",&sjsd_rec);
			if (cc)
		       		sys_err("Error in sjsd During (DBUPDATE)",cc,PNAME);
			abc_unlock("sjsd");
		}
		else
		{
			strcpy(sjsd_rec.sd_co_no,comm_rec.tco_no);
			strcpy(sjsd_rec.sd_est_no,comm_rec.test_no);
			strcpy(sjsd_rec.sd_dp_no,comm_rec.tdp_no);
			sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
			sjsd_rec.sd_line_no = i;
			strcpy(sjsd_rec.sd_detail,local_rec.job_detail);
			cc = abc_add("sjsd",&sjsd_rec);
			if (cc)
			       	sys_err("Error in sjsd During (DBADD)",cc,PNAME);
		}
	}

	strcpy(sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy(sjsd_rec.sd_est_no,comm_rec.test_no);
	strcpy(sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = lcount[2];
	cc = find_rec("sjsd",&sjsd_rec,GTEQ,"r");

	while (!cc && !strcmp(sjsd_rec.sd_co_no,comm_rec.tco_no) && 
		      !strcmp(sjsd_rec.sd_est_no,comm_rec.test_no) && 
		      !strcmp(sjsd_rec.sd_dp_no,comm_rec.tdp_no) && 
		       sjsd_rec.sd_order_no == sjhr_rec.hr_order_no)
	{
		cc = abc_delete("sjsd");
		if (cc)
			continue;

		cc = find_rec("sjsd",&sjsd_rec,NEXT,"r");
	}
	abc_unlock("sjsd");
	scn_set(1);
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
ord_srch (
 char *key_val)
{
	work_open();
	strcpy(sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy(sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol(key_val);
	save_rec("#Order","#Issued On");
	cc = find_rec("sjhr", &sjhr_rec, GTEQ, "r");

	while (!cc && !strcmp(sjhr_rec.hr_co_no,comm_rec.tco_no) && 
		      !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && 
		      !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{ 
		sprintf(local_rec.order_no,"%8ld",sjhr_rec.hr_order_no);

		if (strlen(key_val) == 0 || !strncmp(local_rec.order_no,key_val,strlen(key_val))) 
		{
			strcpy (err_str, DateToString(sjhr_rec.hr_issue_date));
			cc = save_rec(local_rec.order_no, err_str);
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
		sys_err("Error in sjhr During (DBFIND)",cc,PNAME);
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

		rv_pr(ML(mlSjMess011) ,26,0,1);

		move(0,1);
		line(80);

		if (scn == 1)
			box(0,3,80,3);

		move(0,20);
		line(80);
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
