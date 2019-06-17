/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_jbsh.c                                        |
|  Program Desc  : ( Print Job cost sheet for selected Job        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 21/09/87         |
|---------------------------------------------------------------------|
| $Log: jbsh_new.c,v $
| Revision 5.2  2001/08/09 09:17:33  scott
| Updated to add FinishProgram () function
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: jbsh_new.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_jbsh_new/jbsh_new.c,v 5.2 2001/08/09 09:17:33 scott Exp $";

#define MAXWIDTH	150

#include <pslscr.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>

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
		int		termno;
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
		{"sjhr_status"},
		{"sjhr_chg_client"},
		{"sjhr_end_client"},
		{"sjhr_cust_type"},
		{"sjhr_cust_ord_no"},
		{"sjhr_cost_estim"},
		{"sjhr_contact"},
		{"sjhr_issue_date"},
		{"sjhr_reqd_date"},
	};

	int sjhr_no_fields = 13;

	struct {
		char	hr_co_no[3];
		char	hr_est_no[3];
		char	hr_dp_no[3];
		long	hr_order_no;
		char	hr_status[2];
		long	hr_chg_client;
		long	hr_end_client;
		char	hr_cust_type[2];
		char	hr_cust_ord_no[11];
		double	hr_cost_estim;		/*  Money field  */
		char	hr_contact[21];
		long	hr_issue_date;
		long	hr_reqd_date;
	} sjhr_rec;

	/*==========================
	| Service Job detail file  |
	==========================*/
	struct dbview sjjd_list[] ={
		{"sjjd_co_no"},
		{"sjjd_est_no"},
		{"sjjd_dp_no"},
		{"sjjd_order_no"},
		{"sjjd_line_no"},
		{"sjjd_detail"}
	};

	int sjjd_no_fields = 6;

	struct {
		char	jd_co_no[3];
		char	jd_est_no[3];
		char	jd_dp_no[3];
		long	jd_order_no;
		int		jd_line_no;
		char	jd_detail[71];
	} sjjd_rec;

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
		{"cumr_class_type"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
	};

	int cumr_no_fields = 10;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
		char	cm_ch_adr[3][41];
	} cumr_rec;

	/*======================
	|  Service detail file |
	======================*/
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
		int		sd_line_no;
		char	sd_detail[71];
	} sjsd_rec;

	/*===========================
	| special fields & flags    |
	===========================*/
	char	branchNo[3],
	  		systemDate[11],
			address[6][41];

	int		envDbCo = 0,
			envDbFind  = 0,
			lp_no = 1;

	FILE	*fin, 
			*fout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char    c_client[7];
  	char    e_client[7];
	char    chg_client[41];
  	char    end_client[41];
	char	stat_desc[21];
	char    job_detail[7][71];
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "order_no", 4, 20, LONGTYPE, 
		"NNNNNNNN", "          ", 
		" ", "", "Order No ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_order_no}, 
	{1, LIN, "curr_stat", 4, 50, CHARTYPE, 
		"U", "          ", 
		" ", "", "Status ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_status}, 
	{1, LIN, "stat_desc", 4, 82, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.stat_desc}, 
	{1, LIN, "charge_to", 5, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge To ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 5, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 6, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Client ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 6, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{1, LIN, "cust_order_no", 7, 20, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", " ", "Cust Order No ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.hr_cust_ord_no}, 
	{1, LIN, "comp_date", 8, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Completion Date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_reqd_date}, 
	{1, LIN, "Cost est", 9, 20, MONEYTYPE, 
		"NNNNN.NN", "          ", 
		" ", "0", "Cost Estimate ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.hr_cost_estim}, 
	{1, LIN, "desc", 10, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[0]}, 
	{1, LIN, "desc1", 11, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[1]}, 
	{1, LIN, "desc2", 12, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[2]}, 
	{1, LIN, "desc3", 13, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[3]}, 
	{1, LIN, "desc4", 14, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[4]}, 
	{1, LIN, "desc5", 15, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[5]}, 
	{1, LIN, "desc6", 16, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            -", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail[6]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include <FindCumr.h>


/*=====================
| Function Prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void load_detail (void);
void get_stat_desc (void);
void head_output (void);
void print_sjhr (void);
void proc_sjsd (void);
void order_search (char *key_val);
int heading (int scn);
int get_chg_client (void);
int get_end_client (void);
int spec_valid (int field);

/*----------------------------
| Main Processing Routine.   |
----------------------------*/
int
main (
 int argc,
 char *argv[])
{
	if (argc != 2)
	{
		print_at (0,0, mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	strcpy (systemDate, DateToString (TodaysDate()));

	lp_no = atoi (argv[1]);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*====================
	|  get branchNolishment |
	====================*/
	envDbCo = atoi (get_env("DB_CO"));
	envDbFind  = atoi (get_env("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);
	swide ();

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

		/*--------------------------------
		| Print Service Order Job Sheet. |
		--------------------------------*/
		clear ();
		if (sjhr_rec.hr_status[0] == 'H')
		{
			print_mess (ML (mlSjMess057));
			sleep (sleepTime);
			return (-1);
		}
		dsp_screen ("Printing Service Order Job Sheet ",comm_rec.tco_no,comm_rec.tco_name);
	
		if ((fin = pr_open ("sj_jbsh.p")) == NULL)
			sys_err ("Error in opening sj_jbsh.p During (FOPEN)",errno,PNAME);
	
		head_output ();
		print_sjhr ();
	}	/* end of input control loop	*/
    shutdown_prog ();
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
OpenDB (void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("sjhr", sjhr_list, sjhr_no_fields, "sjhr_id_no");
	open_rec ("sjjd", sjjd_list, sjjd_no_fields, "sjjd_id_no");
	open_rec ("cumr", cumr_list, cumr_no_fields, (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
	open_rec ("sjsd", sjsd_list, sjsd_no_fields, "sjsd_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjhr");
	abc_fclose ("cumr");
	abc_fclose ("sjjd");
	abc_fclose ("sjsd");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("order_no"))
	{
		if (SRCH_KEY)
		{
			order_search (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
		strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
		strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
		cc = find_rec ("sjhr",&sjhr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess122));
			return (EXIT_FAILURE);
		}

		if (strcmp (sjhr_rec.hr_status,"O") && strcmp (sjhr_rec.hr_status,"C"))
		{
			sprintf (err_str,ML (mlSjMess042),sjhr_rec.hr_order_no);
			errmess (err_str);
			return (EXIT_FAILURE);
		}
		abc_selfield ("cumr","cumr_hhcu_hash");
		get_chg_client ();
		get_end_client ();
		abc_selfield ("cumr",(envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
		get_stat_desc ();

		load_detail ();
	}

	if (LCHECK ("end_client"))
	{
		if (FLD ("end_client") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no,branchNo);
		strcpy (cumr_rec.cm_dbt_no,pad_num(local_rec.e_client));
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.end_client,cumr_rec.cm_name);
		DSP_FLD ("end_client_name");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
load_detail (
 void)
{
	int	i = 0;

	/*===================
	|  read job details |
	===================*/
	strcpy (sjjd_rec.jd_co_no,comm_rec.tco_no);
	strcpy (sjjd_rec.jd_est_no,comm_rec.test_no);
	strcpy (sjjd_rec.jd_dp_no,comm_rec.tdp_no);
	sjjd_rec.jd_order_no = sjhr_rec.hr_order_no;
	sjjd_rec.jd_line_no = 0;
	cc = find_rec ("sjjd",&sjjd_rec,GTEQ,"r");

	while (!cc && !strcmp (sjjd_rec.jd_co_no,comm_rec.tco_no) && 
				  !strcmp (sjjd_rec.jd_est_no,comm_rec.test_no) && 
				  !strcmp (sjjd_rec.jd_dp_no,comm_rec.tdp_no) && 
				  sjjd_rec.jd_order_no == sjhr_rec.hr_order_no)
	{
		/*=======================
		| get description lines |
		========================*/
		strcpy (local_rec.job_detail[i++],sjjd_rec.jd_detail);
		cc = find_rec ("sjjd",&sjjd_rec,NEXT,"r");
	}
}

int
get_chg_client (
 void)
{
	cc = find_hash ("cumr", &cumr_rec, COMPARISON, "r",sjhr_rec.hr_chg_client);
	if (cc)
	{
		errmess ( ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.c_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.cm_name);
	strcpy (address[0],cumr_rec.cm_ch_adr[0]);
	strcpy (address[1],cumr_rec.cm_ch_adr[1]);
	strcpy (address[2],cumr_rec.cm_ch_adr[2]);
	if (strcmp (cumr_rec.cm_class_type,"INT") !=  0 )
	{
		strcpy (sjhr_rec.hr_cust_type,"E");
		FLD ("end_client")	= NA;
	}
	else
	{
		strcpy (sjhr_rec.hr_cust_type ,"I");
		FLD ("end_client")	= YES;
	}
	return (EXIT_SUCCESS);
}

int
get_end_client (
 void)
{
	if (sjhr_rec.hr_end_client == 0L)
		return (EXIT_SUCCESS);

	cc = find_hash ("cumr", &cumr_rec, COMPARISON, "r",sjhr_rec.hr_end_client);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.e_client,cumr_rec.cm_dbt_no);
	strcpy (local_rec.end_client,cumr_rec.cm_name);
	strcpy (address[3],cumr_rec.cm_ch_adr[0]);
	strcpy (address[4],cumr_rec.cm_ch_adr[1]);
	strcpy (address[5],cumr_rec.cm_ch_adr[2]);
	return (EXIT_SUCCESS);
}

void
get_stat_desc (
 void)
{
	switch(sjhr_rec.hr_status[0])
	{
        case 'O':
			strcpy(local_rec.stat_desc,"Open order");
		break;
        case 'C':
			strcpy(local_rec.stat_desc,"Closed order");
		break;
        case 'H':
			strcpy(local_rec.stat_desc,"Order on hold");
		break;
	}
}

void
head_output (
 void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",lp_no);
	fprintf (fout,".OP\n");
	fprintf (fout,".5\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L132\n");
	fprintf (fout,".ECOMPANY : %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf (fout,".EAS AT : %-24.24s\n",SystemTime());
	fprintf (fout,"\n");
}

void
print_sjhr (
 void)
{
	int	j = 0;
	char	order_no[9];

	sprintf (order_no,"%8ld",sjhr_rec.hr_order_no);

	dsp_process ("Order No :",order_no);

	/*================
	|  Print heading |
	================*/
	fprintf (fout,".ESERVICE ORDER JOB SHEET\n");
	fprintf (fout,"\n");
	pr_format (fin,fout,"ORDER_NO",1,sjhr_rec.hr_order_no);
	pr_format (fin,fout,"ORDER_NO",2,sjhr_rec.hr_cust_ord_no);
	fprintf (fout,"\n");

	pr_format (fin,fout,"CUST_DATE",1,local_rec.c_client);
	pr_format (fin,fout,"CUST_DATE",2,local_rec.e_client);
	pr_format (fin,fout,"CUST_DATE",3,sjhr_rec.hr_issue_date);

	pr_format (fin,fout,"NAME_ADD",1,local_rec.chg_client);
	pr_format (fin,fout,"NAME_ADD",2,local_rec.end_client);
	pr_format (fin,fout,"NAME_ADD",3,sjhr_rec.hr_reqd_date);
	for(j = 0; j < 6; j++)
	{
		pr_format (fin,fout,"ADDRESS",1,address[j++]);
		pr_format (fin,fout,"ADDRESS",2,address[j++]);
	}
	fprintf (fout,"\n\n");

	pr_format (fin,fout,"CONTACT",1,sjhr_rec.hr_contact);
	pr_format (fin,fout,"CONTACT",2,sjhr_rec.hr_status);
	pr_format (fin,fout,"CONTACT",3,local_rec.stat_desc);
	fprintf (fout,"\n");
	pr_format (fin,fout,"JOB_DESC",0,0);

	/*=======================
	| Process order details |
	=======================*/
	for (j = 0; j < 7; j++)
		pr_format (fin,fout,"DETAIL",1,local_rec.job_detail[j]);

	fprintf (fout,"\n");
	pr_format (fin,fout,"LINE",0,0);

	proc_sjsd ();

	fprintf (fout,".EOF\n");
	pclose (fout);
}

void
proc_sjsd (
 void)
{
	/*=======================
	|  read service details |
	=======================*/
	pr_format (fin,fout,"SERV_DET",0,0);
	
	strcpy (sjsd_rec.sd_co_no,comm_rec.tco_no);
	strcpy (sjsd_rec.sd_est_no,comm_rec.test_no);
	strcpy (sjsd_rec.sd_dp_no,comm_rec.tdp_no);
	sjsd_rec.sd_order_no = sjhr_rec.hr_order_no;
	sjsd_rec.sd_line_no = 0;
	cc = find_rec ("sjsd",&sjsd_rec,GTEQ,"r");

	while (!cc && !strcmp (sjsd_rec.sd_co_no,comm_rec.tco_no) && !strcmp(sjsd_rec.sd_est_no,comm_rec.test_no) && !strcmp(sjsd_rec.sd_dp_no,comm_rec.tdp_no) && sjsd_rec.sd_order_no == sjhr_rec.hr_order_no)
	{
		/*=======================
		| get description lines |
		========================*/
		pr_format (fin,fout,"DETAIL",1,sjsd_rec.sd_detail);

		cc = find_rec ("sjsd",&sjsd_rec,NEXT,"r");
	}
	
	fprintf (fout,"\n");
	pr_format (fin,fout,"LINE",0,0);
	pr_format (fin,fout,"MATERIAL",0,0);
	pr_format (fin,fout,"MAT_DESC",0,0);
	pr_format (fin,fout,"SEPARATOR",0,0);
	fprintf (fout,"\n");
	pr_format (fin,fout,"PERSON",0,0);
	pr_format (fin,fout,"PER_LINE",0,0);

}

/*==================================
| Search routine for Order Number. |
===================================*/
void
order_search (
 char *key_val)
{
	char	order_no[7];

	work_open ();
	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	save_rec ("#Order","#Issued On");
	cc = find_rec ("sjhr", &sjhr_rec, GTEQ, "r");
	while (!cc && !strcmp (sjhr_rec.hr_co_no,comm_rec.tco_no) && !strcmp(sjhr_rec.hr_est_no,comm_rec.test_no) && !strcmp(sjhr_rec.hr_dp_no,comm_rec.tdp_no))
	{ 
		sprintf (order_no,"%6ld",sjhr_rec.hr_order_no);

		if ((strlen (key_val) == 0 || !strncmp (order_no,key_val,strlen (key_val))) && (sjhr_rec.hr_status[0] == 'O' || sjhr_rec.hr_status[0] == 'C'))
		{
			strcpy (err_str, DateToString(sjhr_rec.hr_issue_date));
			cc = save_rec (order_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec ("sjhr", &sjhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjhr_rec.hr_co_no,comm_rec.tco_no);
	strcpy (sjhr_rec.hr_est_no,comm_rec.test_no);
	strcpy (sjhr_rec.hr_dp_no,comm_rec.tdp_no);
	sjhr_rec.hr_order_no = atol (temp_str);
	cc = find_rec ("sjhr", &sjhr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in sjhr During (DBFIND)", cc, PNAME);
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
		rv_pr ( ML (mlSjMess048),40,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
			box (0,3,132,15);

		move (0,20);
		line (132);
/*
		print_at(21,0," Company no. : %s   %s",comm_rec.tco_no,comm_rec.tco_name);
		print_at (22,0," Branch no.  : %s   %s",comm_rec.test_no,comm_rec.test_name);*/
		strcpy (err_str,ML(mlStdMess038));
		print_at (21,0,err_str, comm_rec.tco_no,comm_rec.tco_name);
		strcpy (err_str,ML(mlStdMess039));
		print_at (22,0,err_str, comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

