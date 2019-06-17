/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_cf_dsp.c  )                                   |
|  Program Desc  : ( Print Freight Charges By Br/Wh/Carrier.      )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,               |
|  Database      : (    )                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 01/05/91         |
|---------------------------------------------------------------------|
|  Date Modified : (03/10/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (11/09/97)      | Modified  by : Marnie I Organo.  |
|  Date Modified : (24/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : (03/10/91) - Reading file incorrectly. SC 5944 PSL |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (11/09/97)    : Updated for Multilingual Conversion.               |
|  (24/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|$Log: so_cf_dsp.c,v $
|Revision 5.3  2002/07/17 09:58:06  scott
|Updated to change argument to get_lpno from (1) to (0)
|
|Revision 5.2  2001/08/09 09:20:58  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:51:03  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:19:12  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:40:31  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.0  2000/10/10 12:22:04  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 09:12:49  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.13  1999/12/21 05:54:11  ana
|(21/12/1999) SC2160 Corrected display of company.  (causing segmentation fault).
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_cf_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cf_dsp/so_cf_dsp.c,v 5.3 2002/07/17 09:58:06 scott Exp $";

#define	X_OFF	5
#define	Y_OFF	4

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#ifdef PSIZE
#undef PSIZE
#endif

#define	PSIZE	15
#define	VALID_CARR ((strcmp(cfhs_rec.hs_carr_code, local_rec.st_carr) >= 0) \
	           && (strcmp(cfhs_rec.hs_carr_code, local_rec.end_carr) <= 0))

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	t_inv_date;
	} comm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
	};

	int	cumr_no_fields = 2;

	struct	{
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
	} cumr_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
	};

	int	ccmr_no_fields = 3;

	struct	{
		char	cc_co_no[3];
		char	cc_est_no[3];
		char	cc_cc_no[3];
	} ccmr_rec;

	/*============================
	| Carrier file header record |
	============================*/
	struct dbview cfhr_list[] ={
		{"cfhr_co_no"},
		{"cfhr_br_no"},
		{"cfhr_carr_code"},
		{"cfhr_carr_desc"},
	};

	int	cfhr_no_fields = 4;

	struct	{
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_carr_code[5];
		char	hr_carr_desc[41];
	} cfhr_rec;

	/*======================
	| Carrier History file |
	======================*/
	struct dbview cfhs_list[] ={
		{"cfhs_co_no"},
		{"cfhs_br_no"},
		{"cfhs_wh_no"},
		{"cfhs_ref_no"},
		{"cfhs_date"},
		{"cfhs_hhcu_hash"},
		{"cfhs_cons_no"},
		{"cfhs_carr_code"},
		{"cfhs_area_code"},
		{"cfhs_no_cartons"},
		{"cfhs_no_kgs"},
		{"cfhs_est_frt_cst"},
		{"cfhs_act_frt_cst"},
		{"cfhs_cumr_chg"},
		{"cfhs_stat_flag"},
	};

	int	cfhs_no_fields = 15;

	struct	{
		char	hs_co_no[3];
		char	hs_br_no[3];
		char	hs_wh_no[3];
		char	hs_ref_no[9];
		long	hs_date;
		long	hs_hhcu_hash;
		char	hs_cons_no[17];
		char	hs_carr_code[5];
		char	hs_area_code[3];
		int	hs_no_cartons;
		float	hs_no_kgs;
		double	hs_est_frt_cst;
		double	hs_act_frt_cst;
		char	hs_cumr_chg[2];
		char	hs_stat_flag[2];
	} cfhs_rec;

	/*================================
	| Carrier file line item records |
	================================*/
	struct dbview cfln_list[] ={
		{"cfln_cfhh_hash"},
		{"cfln_area_code"},
		{"cfln_carr_code"},
		{"cfln_cost_kg"},
		{"cfln_stat_flag"},
	};

	int	cfln_no_fields = 5;

	struct	{
		long	ln_cfhh_hash;
		char	ln_area_code[3];
		char	ln_carr_code[5];
		double	ln_cost_kg;
		char	ln_stat_flag[2];
	} cfln_rec;

/*============================
| Local & Screen Structures. |
============================*/
float	est_tot[4];
float	act_tot[4];
int	ptr_offset[7];
int	DISPLAY_REP;
int	DETAILED;
char	curr_br[3];
char	prnt_br[3];
char	curr_wh[3];
char	prnt_wh[3];
char	curr_carr[5];
char	prnt_carr[5];
long	curr_date;
char	prnt_date[11];
char	dsp_str[200];
char	branchNo[3];
char	data_str[140];
char	env_str[300];
char	rep_type[2];
char	*UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGG^^";
char	*UNDERLINE2 = "^^GGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGG^^";
char	*RULEOFF =   "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGG^^";
char	*RULEOFF2 =   "^^GGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGG^^";
char	*BLNK_LINE = "^^GGGGGGGGIGGGGGGGGGGIGGGGGGIGGGGGGGGGGGGGGGGGGIGGGGGGIGGGGGGGGIGGGGGGGGGGGGGIGGGGGGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGG";

FILE	*fout;
FILE	*fsort;

struct {
	char	dummy[11];
	int	lpno;
	char	lp_str[3];
	char	back[4];
	char	onight[4];
	char	systemDate[11];
	long	lsystemDate;
	long	st_date;
	long	end_date;
	char	st_carr[5];
	char	st_carr_desc[31];
	char	end_carr[5];
	char	end_carr_desc[31];
	char	prntscn[10];
	char	type[12];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_date",	 4, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Start Date             :", "Default = 1st of Month.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.st_date},
	{1, LIN, "end_date",	 5, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "End Date               :", "Default = today.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_date},

	{1, LIN, "st_carr",	 7, 25, CHARTYPE,
		"UUUU", "          ",
		" ", "    ", "Start Carrier code     :", "Default = First Carrier.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.st_carr},
	{1, LIN, "st_carr_desc",	 7, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "    ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_carr_desc},
	{1, LIN, "end_carr",	 8, 25, CHARTYPE,
		"UUUU", "          ",
		" ", "~~~~", "End Carrier code       :", "Default = Last Carrier.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.end_carr},
	{1, LIN, "end_carr_desc", 8, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "    ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_carr_desc},

	{1, LIN, "r_type",	10, 25, CHARTYPE,
		"U", "          ",
		" ", "S(ummary) ", "S(ummary) / D(etailed) :", " ",
		YES, NO,  JUSTLEFT, "SD", "", local_rec.type},
	{1, LIN, "prntscn",	12, 25, CHARTYPE,
		"U", "          ",
		" ", "S(creen)", "P(rinter) or S(creen)  :", " ",
		YES, NO,  JUSTLEFT, "PS", "", local_rec.prntscn},
	{1, LIN, "lpno",	13, 25, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number         :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	14, 25, CHARTYPE,
		"U", "          ",
		" ", "N(o", "Background             :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	15, 25, CHARTYPE,
		"U", "          ",
		" ", "N(o", "Overnight              :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=======================
| Function Declarations |
=======================*/
void run_prog (char *prog_name, char *prog_desc);
void shutdown_prog (void);
int  spec_valid (int field);
void SrchCfhr (char *key_val);
int  heading (int scn);
void OpenDB (void);
void CloseDB (void);
void head_output (void);
void process_frght (void);
void proc_sort_frght (void);
char *_sort_read (FILE *srt_fil);
void init_array (void);
void set_breaks (char *tptr);
int  delta_carr (char *tptr);
int  delta_wh (char *tptr);
int  delta_br (char *tptr);
int  delta_date (char *tptr);
void print_total (char *type);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int	argc,
 char	*argv [])
{

	if (argc != 2 && argc != 8)
	{
		print_at(0,0,mlSoMess704,argv[0]);
		print_at(1,0,mlSoMess705, argv[0]);
		print_at(2,0,mlSoMess706);
		print_at(3,0,mlSoMess707);
		print_at(4,0,mlSoMess708);
		print_at(5,0,mlSoMess709); 
		print_at(6,0,mlSoMess710);
		print_at(7,0,mlSoMess711);
		return (EXIT_FAILURE);
	}

	if (argc == 8)
	{
		DISPLAY_REP = (argv[2][0] == 'S');
		DETAILED = (argv[3][0] == 'D');
	}
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	if (argc == 2 || (argc == 8 && DISPLAY_REP))
	{
		init_scr();		/*  sets terminal from termcap	*/
		set_tty();		/*  get into raw mode		*/
		set_masks();		/*  setup print using masks	*/
		init_vars(1);		/*  set default values		*/
	}

	OpenDB();

	if (argc == 8)
	{
		if (DETAILED)
			swide();

		local_rec.lpno = atoi(argv[1]);
		local_rec.st_date = StringToDate(argv[4]);
		local_rec.end_date = StringToDate(argv[5]);
		sprintf(local_rec.st_carr,"%-4.4s",argv[6]);
		sprintf(local_rec.end_carr,"%-4.4s",argv[7]);

		if (!DISPLAY_REP)
			dsp_screen("Processing : Printing Freight Report.",
				comm_rec.tco_no,
				comm_rec.tco_name);

		head_output();

		process_frght();

		if (DISPLAY_REP)
		{
			Dsp_srch();
			Dsp_close();
		}
		else
			Dsp_print ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		run_prog(argv[0], argv[1]);
		prog_exit = 1;
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
}

void
run_prog (
 char *prog_name, 
 char *prog_desc)
{
	char	tmp_st_date[11];
	char	tmp_end_date[11];

	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	strcpy (tmp_st_date, DateToString(local_rec.st_date));
	strcpy (tmp_end_date,DateToString(local_rec.end_date));
	
	if (local_rec.prntscn[0] == 'S')
	{
		clear();
		fflush(stdout);
	}


	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				local_rec.prntscn,
				local_rec.type,
				tmp_st_date,
				tmp_end_date,
				local_rec.st_carr,
				local_rec.end_carr,
				prog_desc, (char *)0);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.prntscn,
				local_rec.type,
				tmp_st_date,
				tmp_end_date,
				local_rec.st_carr,
				local_rec.end_carr,(char *)0);
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.prntscn,
			local_rec.type,
			tmp_st_date,
			tmp_end_date,
			local_rec.st_carr,
			local_rec.end_carr,(char *)0);
	}
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

int
spec_valid (
 int field)
{
	char	tmp_carr[5];

	if (LCHECK("st_date")) 
	{
		if (dflt_used)
		{
			local_rec.st_date = MonthStart (local_rec.lsystemDate);
			return(0);
		}
		return(0);
	}

	if (LCHECK("end_date")) 
	{
		if (dflt_used)
		{
			local_rec.end_date = local_rec.lsystemDate;
			return(0);
		}
		return(0);
	}

	if (LCHECK("st_carr"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.st_carr_desc,
				"%-30.30s",
				"First Carrier");
			DSP_FLD("st_carr_desc");

			return(0);
		}

		if (SRCH_KEY)
		{
			SrchCfhr(temp_str);
			return(0);
		}
	
		sprintf(tmp_carr, "%-4.4s", temp_str);
		strcpy(cfhr_rec.hr_co_no, comm_rec.tco_no);
		strcpy(cfhr_rec.hr_br_no, "  ");
		sprintf(cfhr_rec.hr_carr_code, "%-4.4s", temp_str);
		cc = find_rec("cfhr", &cfhr_rec, GTEQ, "r");
		while (!cc)
		{
			if (!strcmp(cfhr_rec.hr_carr_code, tmp_carr))
			{
				sprintf(local_rec.st_carr_desc,
					"%-30.30s",
					cfhr_rec.hr_carr_desc);
				DSP_FLD("st_carr_desc");

				return(0);
			}

			cc = find_rec("cfhr", &cfhr_rec, NEXT, "r");
		}

		rv_pr(ML(mlStdMess134), 30,23,1);
		return(1);
	}

	if (LCHECK("end_carr"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.end_carr_desc,
				"%-30.30s",
				"Last Carrier");
			DSP_FLD("end_carr_desc");
	
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchCfhr(temp_str);
			return(0);
		}
	
		sprintf(tmp_carr, "%-4.4s", temp_str);
		strcpy(cfhr_rec.hr_co_no, comm_rec.tco_no);
		strcpy(cfhr_rec.hr_br_no, "  ");
		sprintf(cfhr_rec.hr_carr_code, "%-4.4s", temp_str);
		cc = find_rec("cfhr", &cfhr_rec, GTEQ, "r");
		while (!cc)
		{
			if (!strcmp(cfhr_rec.hr_carr_code, tmp_carr))
			{
				sprintf(local_rec.end_carr_desc,
					"%-30.30s",
					cfhr_rec.hr_carr_desc);
				DSP_FLD("end_carr_desc");

				return(0);
			}

			cc = find_rec("cfhr", &cfhr_rec, NEXT, "r");
		}

		rv_pr(ML(mlStdMess134), 30,23,1);
		return(1);
	}

	if (LCHECK("r_type"))
	{
		if (local_rec.type[0] == 'D')
			strcpy(local_rec.type, "D(etailed)");
		else
			strcpy(local_rec.type, "S(ummary) ");

		display_field(field);
	}
	
	if (LCHECK("prntscn"))
	{
		if (local_rec.prntscn[0] == 'P')
		{
			strcpy(local_rec.prntscn, "P(rinter)");
			FLD("lpno") = NO;
			FLD("back") = NO;
			FLD("onight") = NO;
		}
		else
		{
			strcpy(local_rec.prntscn, "S(creen) ");
			FLD("lpno") = NA;
			FLD("back") = NA;
			FLD("onight") = NA;
		}
		
		display_field(field);
		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
			local_rec.lpno = get_lpno (0);

		return(0);
	}

	if (LCHECK("back"))
	{
		if (local_rec.back[0] == 'Y')
		{
			strcpy(local_rec.back, "Yes");
			strcpy(local_rec.onight, "No ");
			DSP_FLD("onight");
		}
		else
			strcpy(local_rec.back, "No ");
	
		display_field(field);
		return(0);
	}

	if (LCHECK("onight"))
	{
		if (local_rec.onight[0] == 'Y')
		{
			strcpy(local_rec.onight, "Yes");
			strcpy(local_rec.back, "No ");
			DSP_FLD("back");
		}
		else
			strcpy(local_rec.onight, "No ");
	
		display_field(field);
		return(0);
	}

	return(0);
}

/*====================
| Search for branch  |
====================*/
void
SrchCfhr (
 char *key_val)
{

        work_open();
	save_rec("#Carrier","#Carrier Description");

	strcpy(cfhr_rec.hr_co_no, comm_rec.tco_no);
	sprintf(cfhr_rec.hr_br_no,"%2.2s"," ");
	sprintf(cfhr_rec.hr_carr_code,"%-4.4s",key_val);
	cc = find_rec("cfhr",&cfhr_rec,GTEQ,"r");
        while (!cc && !strcmp(cfhr_rec.hr_co_no,comm_rec.tco_no))
    	{                        
	        if (!strncmp(cfhr_rec.hr_carr_code, key_val, strlen(key_val)))
		{
		        cc = save_rec(cfhr_rec.hr_carr_code, cfhr_rec.hr_carr_desc); 
			if (cc)
			        break;
		}

		cc = find_rec("cfhr",&cfhr_rec,NEXT,"r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
	        return;

	sprintf(cfhr_rec.hr_carr_code,"%-4.4s",temp_str);
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
		rv_pr(ML(mlSoMess010),25,0,1);
		move(0,1);
		line(80);

		move(0,6);
		line(80);

		move(0,9);
		line(80);

		move(0,11);
		line(80);

		box(0,3,80,12);

		move(0,20);
		line(80);
		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		strcpy(err_str, ML(mlStdMess039));
		print_at(21,13,err_str,comm_rec.test_no,comm_rec.test_name);
		move(0,22);
		line(80);
		line_cnt = 0;
		scn_write(scn);
	}

    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");
	strcpy(ccmr_rec.cc_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.cc_est_no,comm_rec.test_no);
	strcpy(ccmr_rec.cc_cc_no,comm_rec.tcc_no);
	cc = find_rec("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in ccmr During (DBFIND)",cc,PNAME);
	abc_fclose ("ccmr");
	open_rec("cfhr",cfhr_list,cfhr_no_fields,"cfhr_id_no");
	open_rec("cfhs",cfhs_list,cfhs_no_fields,"cfhs_id_no");
	open_rec("cfln",cfln_list,cfln_no_fields,"cfln_id_no");
	open_rec("cumr",cumr_list,cumr_no_fields,"cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("cfhr");
	abc_fclose("cfhs");
	abc_fclose("cfln");
	abc_fclose("cumr");
	abc_dbclose("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	char	head2[133];
	char	tmp_st_date[11];
	char	tmp_end_date[11];
	int	lcl_x_coord;

	if (DETAILED)
		lcl_x_coord = 5;
	else
		lcl_x_coord = 15;

	strcpy(tmp_st_date, DateToString(local_rec.st_date));
	strcpy(tmp_end_date,DateToString(local_rec.end_date));

	sprintf(head2, ML(mlSoMess011),
		tmp_st_date,
		tmp_end_date,
		local_rec.st_carr,
		local_rec.end_carr);

	if (DISPLAY_REP)
	{
		if (DETAILED)
		{
			rv_pr(head2,(132 - strlen(head2)) / 2,2,1);
			rv_pr (ML(mlSoMess010), 55,0,0);
		}
		else
		{
			rv_pr(head2,(80 - strlen(head2)) / 2,2,1);
			rv_pr (ML(mlSoMess010), 27,0,0);
		}

		Dsp_prn_open(lcl_x_coord, 4, PSIZE, 
			head2, comm_rec.tco_no, comm_rec.tco_name, 
			(char *)0, (char *)0, (char *)0, (char *)0);
	}
	else
		Dsp_nd_prn_open(lcl_x_coord, 4, PSIZE, 
			head2, comm_rec.tco_no, comm_rec.tco_name, 
			(char *)0, (char *)0, (char *)0, (char *)0);

	if (DETAILED)
		Dsp_saverec(" Ref.   |   Date   | Carr | Consignment  No. | Area | Cust.  | No. Cartons | No. of  kgs | Est. Charge | Act. Charge ");
	else
		Dsp_saverec("    Date       |  Est. Charge  |  Act. Charge  ");

	Dsp_saverec("");
	Dsp_saverec("[REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");
}

void
process_frght (
 void)
{
	char	data_str[256];
	char	data_str1[256];

	fsort = sort_open("freight");

	cfhs_rec.hs_hhcu_hash = 0L;
	sprintf(cfhs_rec.hs_ref_no, "%-8.8s", " ");
	cfhs_rec.hs_date = 0L;

	cc = find_rec("cfhs", &cfhs_rec, GTEQ, "r");
	while (!cc)
	{
		if (strcmp(cfhs_rec.hs_co_no, comm_rec.tco_no))
		{
			cc = find_rec("cfhs", &cfhs_rec, NEXT, "r");
			continue;
		}

		if (cfhs_rec.hs_date >= local_rec.st_date &&
		    cfhs_rec.hs_date <= local_rec.end_date && VALID_CARR)
		{
			if (!DISPLAY_REP)
				dsp_process ("Reading Ref No : ", cfhs_rec.hs_ref_no);

			if (DETAILED)
			{
				sprintf (data_str,
					"%2.2s%2.2s%4.4s%8ld%-16.16s",
					cfhs_rec.hs_br_no,
					cfhs_rec.hs_wh_no,
					cfhs_rec.hs_carr_code,
					cfhs_rec.hs_date,
					cfhs_rec.hs_cons_no);
			}
			else
			{
				sprintf(data_str,
					"%5ld%2.2s%2.2s%4.4s%-16.16s",
					cfhs_rec.hs_date,
					cfhs_rec.hs_br_no,
					cfhs_rec.hs_wh_no,
					cfhs_rec.hs_carr_code,
					cfhs_rec.hs_cons_no);
			}

			clip (data_str);
			sprintf (data_str1, 
				"%c%s%c%s%c%ld%c%d%c%.2f%c%.2f%c%.2f\n",
				1, cfhs_rec.hs_ref_no,
				1, cfhs_rec.hs_area_code,
				1, cfhs_rec.hs_hhcu_hash,
				1, cfhs_rec.hs_no_cartons,
				1, cfhs_rec.hs_no_kgs,
				1, cfhs_rec.hs_est_frt_cst,
				1, cfhs_rec.hs_act_frt_cst);
			strcat (data_str, data_str1);
			sort_save (fsort, data_str);
		}
		cc = find_rec("cfhs", &cfhs_rec, NEXT, "r");
	}
	proc_sort_frght();
}

void
proc_sort_frght (
 void)
{
	char	*sptr;
	char	tmp_cust[7];
	int	first_time;
	int	tot_printed = FALSE;

	init_array();
	first_time = TRUE;

	fsort = sort_sort(fsort,"freight");

	sptr = _sort_read(fsort);
	if (sptr)
		set_breaks(sptr);

	while (sptr != (char *)0)
	{
		if (!DETAILED)
			tot_printed = FALSE;

		/*---------------------
		| Get customer number |
		---------------------*/
		cc = find_hash("cumr", &cumr_rec, COMPARISON, "r", 
				atol(sptr + ptr_offset[2]));

		if (cc)
			strcpy (tmp_cust, "Nt Fnd");
		else
			sprintf(tmp_cust, "%-6.6s", cumr_rec.cm_dbt_no);
	
		if (DETAILED)
		{
			sprintf(dsp_str,
				"%-8.8s^E%10.10s^E %-4.4s ^E %-16.16s ^E  %-2.2s  ^E %-6.6s ^E  %5d      ^E %11.2f ^E %11.2f ^E %11.2f",
				sptr + ptr_offset[0],		/* ref */
				DateToString(atol(sptr + 8)),	/* date */
				sptr + 4,			/* carrier */
				sptr + 16,			/* cons no */
				sptr + ptr_offset[1],		/* area */
				tmp_cust,			/* cust */
				atoi(sptr + ptr_offset[3]),	/* no ctns */
				atof(sptr + ptr_offset[4]),	/* no kgs */
				atof(sptr + ptr_offset[5]),	/* est chrg */
				atof(sptr + ptr_offset[6]));	/* act chrg */
		}
		else
		{
			sprintf(dsp_str, "%10.10s^E %11.2f ^E %11.2f",
				DateToString(atol(sptr)),		/* date */
				atof(sptr + ptr_offset[5]),	/* est chrg */
				atof(sptr + ptr_offset[6]));	/* act chrg */
		}

		est_tot[0] += (float) (atof(sptr + ptr_offset[5]));
		est_tot[1] += (float) (atof(sptr + ptr_offset[5]));
		est_tot[2] += (float) (atof(sptr + ptr_offset[5]));
		est_tot[3] += (float) (atof(sptr + ptr_offset[5]));
			
		act_tot[0] += (float) (atof(sptr + ptr_offset[6]));
		act_tot[1] += (float) (atof(sptr + ptr_offset[6]));
		act_tot[2] += (float) (atof(sptr + ptr_offset[6]));
		act_tot[3] += (float) (atof(sptr + ptr_offset[6]));
	
		if (DETAILED)
			Dsp_saverec(dsp_str);

		sptr = _sort_read(fsort);

		if (sptr)
		{
			if (DETAILED)
			{
				if (delta_carr(sptr))
				{
					print_total("C");
					tot_printed = TRUE;
				}
	
				if (delta_wh(sptr))
				{
					print_total("W");
					tot_printed = TRUE;
				}
	
				if (delta_br(sptr))
				{
					print_total("B");
					tot_printed = TRUE;
				}
	
				if (tot_printed)
				{
					Dsp_saverec(BLNK_LINE);
					tot_printed = FALSE;
				}
			}
			else
			{
				if (delta_date(sptr))
				{
					print_total("S");
					tot_printed = TRUE;
				}
			}
		}
	}
	
	if (DETAILED)
	{
		print_total("C");
		print_total("W");
		print_total("B");
	}
	else
		if (!tot_printed)
			print_total("S");

	print_total("G");

	sort_delete(fsort,"freight");
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char *
_sort_read (
 FILE *srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 0;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	tptr = sptr;
	while (fld_no < 7)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;
		ptr_offset[fld_no++] = tptr - sptr;
	}

	return (sptr);
}

void
init_array (
 void)
{
	int	i;

	for (i = 0; i < 4; i++)
	{
		est_tot[i] = 0.00;
		act_tot[i] = 0.00;
	}
}

void
set_breaks (
 char *tptr)
{
	if (DETAILED)
	{
	sprintf(curr_carr, "%-4.4s", tptr + 4);
	sprintf(prnt_carr, "%-4.4s", curr_carr);

	sprintf(curr_wh, "%2.2s", tptr + 2);
	sprintf(prnt_wh, "%2.2s", curr_wh);

	sprintf(curr_br, "%2.2s", tptr);
	sprintf(prnt_br, "%2.2s", curr_br);
	}
	else
	{
		curr_date = atol(tptr);
		strcpy(prnt_date, DateToString(curr_date));
	}
}

int
delta_carr (
 char *tptr)
{
	char	tmp_carr[5];
	char	tmp_wh[5];
	char	tmp_br[5];

	sprintf(tmp_carr, "%-4.4s", tptr + 4);
	sprintf(tmp_wh, "%2.2s", tptr + 2);
	sprintf(tmp_br, "%2.2s", tptr);

	if (strcmp(curr_carr, tmp_carr) ||
	    strcmp(curr_wh, tmp_wh) ||
	    strcmp(curr_br, tmp_br))
	{
		sprintf(prnt_carr, "%-4.4s", curr_carr);
		sprintf(curr_carr, "%-4.4s", tmp_carr);
		return(TRUE);
	}

	return(FALSE);
}

int
delta_wh (
 char *tptr)
{
	char	tmp_wh[5];
	char	tmp_br[5];

	sprintf(tmp_wh, "%2.2s", tptr + 2);
	sprintf(tmp_br, "%2.2s", tptr);

	if (strcmp(curr_wh, tmp_wh) || 
	    strcmp(curr_br, tmp_br))
	{
		sprintf(prnt_wh, "%2.2s", curr_wh);
		sprintf(curr_wh, "%2.2s", tmp_wh);
		return(TRUE);
	}

	return(FALSE);
}

int
delta_br (
 char *tptr)
{
	char	tmp_br[5];

	sprintf(tmp_br, "%2.2s", tptr);

	if (strcmp(curr_br, tmp_br))
	{
		sprintf(prnt_br, "%2.2s", curr_br);
		sprintf(curr_br, "%2.2s", tmp_br);
		return(TRUE);
	}

	return(FALSE);
}

int
delta_date (
 char *tptr)
{
	long	tmp_date;

	tmp_date = atol(tptr);

	if (curr_date != tmp_date)
	{
		strcpy(prnt_date, DateToString(curr_date));
		curr_date = tmp_date;
		return(TRUE);
	}

	return(FALSE);
}

void
print_total (
 char *type)
{
	switch(type[0])
	{
	case 'C':
		sprintf(dsp_str, " Total for carrier   : %-4.4s                                                              ^E %11.2f ^E %11.2f",
			prnt_carr,
			est_tot[0],
			act_tot[0]);
		Dsp_saverec(dsp_str);

		sprintf(prnt_carr, "%-4.4s", curr_carr);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;

		break;

	case 'W':
		sprintf(dsp_str, " Total for warehouse : %2.2s                                                                ^E %11.2f ^E %11.2f",
			prnt_wh,
			est_tot[1],
			act_tot[1]);
		Dsp_saverec(dsp_str);

		sprintf(prnt_wh, "%2.2s", curr_wh);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;
		est_tot[1] = 0.00;
		act_tot[1] = 0.00;

		break;

	case 'B':
		sprintf(dsp_str, " Total for branch    : %2.2s                                                                ^E %11.2f ^E %11.2f",
			prnt_br,
			est_tot[2],
			act_tot[2]);
		Dsp_saverec(dsp_str);

		sprintf(prnt_br, "%2.2s", curr_br);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;
		est_tot[1] = 0.00;
		act_tot[1] = 0.00;
		est_tot[2] = 0.00;
		act_tot[2] = 0.00;

		break;

	case 'G':
		if (DETAILED)
		{
			Dsp_saverec(UNDERLINE);
			sprintf(dsp_str, " GRAND TOTAL :                                                                           ^E %11.2f ^E %11.2f",
				est_tot[3],
				act_tot[3]);
			Dsp_saverec(dsp_str);
			Dsp_saverec(RULEOFF);
		}
		else
		{
			Dsp_saverec(UNDERLINE2);
			sprintf(dsp_str, " GRAND TOTAL : ^E %11.2f   ^E %11.2f",
				est_tot[3],
				act_tot[3]);
			Dsp_saverec(dsp_str);
			Dsp_saverec(RULEOFF2);
		}

		break;

	case 'S':
		sprintf(dsp_str, "  %-10.10s   ^E %11.2f   ^E %11.2f",
			prnt_date,
			est_tot[0],
			act_tot[0]);
		Dsp_saverec(dsp_str);

		strcpy(prnt_date, DateToString(curr_date));

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;

		break;
		

	}
}

