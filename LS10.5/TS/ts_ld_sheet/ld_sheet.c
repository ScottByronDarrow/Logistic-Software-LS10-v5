/*=====================================================================
|  Copyright (C) 1988 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_ld_sheet.c )                                  |
|  Program Desc  : ( Telesales Lead Sheet Report.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (tele)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 16/06/92         |
|---------------------------------------------------------------------|
|  Date Modified : 12/09/97        | Modified  by  : Marnie Organo    |
|                                                                     |
|  Comments      :                                                    |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: ld_sheet.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_ld_sheet/ld_sheet.c,v 5.4 2002/07/17 09:58:17 scott Exp $";

#include	<pslscr.h>		
#include	<ml_std_mess.h>
#include	<ml_ts_mess.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>

FILE	*fout;
FILE	*fsort;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		long	t_dbt_date;
	} comm_rec;

	/*========================
	| External Salesman File |
	========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int	exsf_no_fields = 3;

	struct	{
		char	sf_co_no[3];
		char	sf_sman[3];
		char	sf_sman_name[41];
	} exsf_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_sman_code"},
	};

	int	cumr_no_fields = 5;

	struct	{
		char	cm_co_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_sman[3];
	} cumr_rec;

	/*=================================
	| Tele-Sales Prospect Master file |
	=================================*/
	struct dbview tspm_list[] ={
		{"tspm_hhcu_hash"},
		{"tspm_n_visit_date"},
		{"tspm_n_visit_time"},
		{"tspm_op_code"},
	};

	int	tspm_no_fields = 4;

	struct	{
		long	pm_hhcu_hash;
		long	pm_n_vst_date;
		char	pm_n_vst_time[6];
		char	pm_op_code[15];
	} tspm_rec;

	/*=================================
	| Tele-Sales External detail file |
	=================================*/
	struct dbview tsxd_list[] ={
		{"tsxd_hhcu_hash"},
		{"tsxd_type"},
		{"tsxd_line_no"},
		{"tsxd_desc"},
		{"tsxd_stat_flag"},
	};

	int	tsxd_no_fields = 5;

	struct	{
		long	xd_hhcu_hash;
		char	xd_type[2];
		int	xd_line_no;
		char	xd_desc[61];
		char	xd_stat_flag[2];
	} tsxd_rec;
	
	char	*data  = "data",
	    	*comm  = "comm",
	    	*cumr  = "cumr",
	    	*cumr2 = "cumr2",
	    	*exsf  = "exsf",
	    	*tspm  = "tspm",
	    	*tsxd  = "tsxd";

	int	pipe_open;
	char	usr_fname[100];
	char	rep_type[2];
	char	dbt_dtls[100];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	st_rep[15];
	char	st_rep_name[41];
	char	end_rep[15];
	char	end_rep_name[41];
	long	prt_date;
	long	cur_ldate;
	char	back[4];
	char	onight[4];
	char	lp_str[3];
	int	lpno;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_rep",	 4, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " Start Rep      :", "",
		NO, NO, JUSTRIGHT, "", "", local_rec.st_rep},
	{1, LIN, "st_rep_name",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_rep_name},
	{1, LIN, "end_rep",	 5, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " End Rep        :", "",
		NO, NO, JUSTRIGHT, "", "", local_rec.end_rep},
	{1, LIN, "end_rep_name",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_rep_name},
	{1, LIN, "prt_date",	 7, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", " Next Visit Date:", "",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.prt_date},

	{1,LIN,"lpno",	9,16,INTTYPE,
		"NN","          ",
		" ","1", " Printer No     :","",
		NO,NO, JUSTLEFT,"","",(char *)&local_rec.lpno},
	{1,LIN,"back",	10,16,CHARTYPE,
		"U","          ",
		" ","", " Background     :","",
		NO,NO, JUSTLEFT,"","",local_rec.back},
	{1,LIN,"onight",	11,16,CHARTYPE,
		"U","          ",
		" ","", " Overnight      :","",
		NO,NO, JUSTLEFT,"","",local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void SrchExsf (char *key_val);
void run_prog (char *prog_name, char *prog_desc);
int process (void);
void proc_sort (void);
int head_output (void);
void rep_head (char *rep, char *rep_name);
int prt_notes (char *note_type);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	if (argc != 2 && argc != 5)
	{
/*
		printf("\007Usage: %s <description>\n", argv[0]);
		printf(" OR  :    <lpno>\n");
		print_at(2,0,"          <st_rep>\n");
		print_at(3,0,"          <end_rep>\n");
		print_at(4,0,"          <visit_date>\n");
*/
		print_at(0,0,ML(mlTsMess709), argv[0]);
		print_at(1,0,ML(mlTsMess710));
		print_at(2,0,ML(mlTsMess706));
		print_at(3,0,ML(mlTsMess707));
		print_at(4,0,ML(mlTsMess711));
		return (EXIT_FAILURE);
	}

	sprintf(rep_type, "%-1.1s", argv[1]);

	local_rec.cur_ldate = TodaysDate ();

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 5)
	{
		local_rec.lpno = atoi(argv[1]);
		sprintf(local_rec.st_rep, "%2.2s", argv[2]);
		sprintf(local_rec.end_rep, "%2.2s", argv[3]);
		local_rec.prt_date = StringToDate(argv[4]);

		pipe_open = FALSE;
		process();

		if (pipe_open)
		{
			fprintf(fout, ".EOF\n");
			pclose(fout);
		}
	
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	SETUP_SCR(vars);

	init_scr();			/*  sets terminal from termcap	*/
	set_tty();                      /*  get into raw mode		*/
	set_masks();			/*  setup print using masks	*/

	init_vars(1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok = 1;
		entry_exit = 1;
		prog_exit = 0;
		restart = 0;
		init_vars(1);	

		heading(1);
		entry(1);

		if (prog_exit || restart)
			continue;

		heading(1);
		scn_display(1);
		edit(1);

		run_prog(argv[0], argv[1]);
	}

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen(data);
	
	abc_alias(cumr2, cumr);

	open_rec(cumr,  cumr_list, cumr_no_fields, "cumr_id_no3");
	open_rec(cumr2, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec(exsf,  exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec(tspm,  tspm_list, tspm_no_fields, "tspm_hhcu_hash");
	open_rec(tsxd,  tsxd_list, tsxd_no_fields, "tsxd_id_no");
}

void
CloseDB (void)
{
	abc_fclose(cumr);
	abc_fclose(cumr2);
	abc_fclose(exsf);
	abc_fclose(tspm);
	abc_fclose(tsxd);
	abc_dbclose(data);
}

int
spec_valid (
 int    field)
{
	if (LCHECK("st_rep"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_rep, "  ");
			sprintf(local_rec.st_rep_name, "%-40.40s", "First Rep");
			DSP_FLD("st_rep_name");

			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExsf(temp_str);
			return(0);
		}

		strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
		sprintf(exsf_rec.sf_sman, "%2.2s", local_rec.st_rep);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Salesman Not Found On File ");*/
			print_mess(ML(mlStdMess135));
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.st_rep_name,"%-40.40s",exsf_rec.sf_sman_name);
		
		DSP_FLD("st_rep_name");
		return(0);
	}

	if (LCHECK("end_rep"))
	{
		if (dflt_used || !strcmp (local_rec.end_rep, "~~"))
		{
			strcpy(local_rec.end_rep, "~~");
			sprintf(local_rec.end_rep_name, "%-40.40s", "Last Rep");
			DSP_FLD("end_rep_name");

			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExsf(temp_str);
			return(0);
		}

		strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
		sprintf(exsf_rec.sf_sman, "%2.2s", local_rec.end_rep);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Salesman Not Found On File ");*/
			print_mess(ML(mlStdMess135));
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.end_rep_name,"%-40.40s",exsf_rec.sf_sman_name);
		
		DSP_FLD("end_rep_name");
		return(0);
	}

	if (LCHECK("prt_date"))
	{
		if (dflt_used)
			local_rec.prt_date = local_rec.cur_ldate;
		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			/*print_mess("\007Invalid Printer");*/
			print_mess(ML(mlStdMess020));
			sleep(2);
			clear_mess();
			return(1);
		}
	}

	if (LCHECK("back"))
	{
		if (local_rec.back[0] == 'Y')
			strcpy(local_rec.back, "Yes");
		else
			strcpy(local_rec.back, "No ");
	
		DSP_FLD("back");
		return(0);
	}

	if (LCHECK("onight"))
	{
		if (local_rec.onight[0] == 'Y')
			strcpy(local_rec.onight, "Yes");
		else
			strcpy(local_rec.onight, "No ");
	
		DSP_FLD("onight");
		return(0);
	}
	
	return(0);
}

/*========================================
| Search routine for Script Header File. |
========================================*/
void
SrchExsf (
 char*  key_val)
{
	work_open();
	strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
	sprintf(exsf_rec.sf_sman, "%2.2s", key_val);
	save_rec("#Salesman.","#Salesman Full Name.");
	cc = find_rec("exsf", &exsf_rec, GTEQ, "r");
	while (!cc && !strcmp(exsf_rec.sf_co_no, comm_rec.tco_no) &&
		      !strncmp(exsf_rec.sf_sman,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_sman, exsf_rec.sf_sman_name);
		if (cc)
			break;

		cc = find_rec("exsf", &exsf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
	sprintf(exsf_rec.sf_sman, "%2.2s", temp_str);
	cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err( cc, "exsf", "DBFIND" );
}

void
run_prog (
 char*  prog_name,
 char*  prog_desc)
{
	char	vst_date[11];

	sprintf(local_rec.lp_str, "%d", local_rec.lpno);
	strcpy (vst_date,DateToString(local_rec.prt_date));
	
	shutdown_prog ();

	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				local_rec.st_rep,
				local_rec.end_rep, 
				vst_date, 
				prog_desc, (char *)0);
		else
			return;
	}
    else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.st_rep,
				local_rec.end_rep, 
				vst_date, (char *)0);
		else
			return;
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.st_rep,
			local_rec.end_rep, 
			vst_date, (char *)0);
	}
}

int
process (void)
{
	char	sort_str[256];

 	fsort = sort_open("nxt_vst");

	dsp_screen("Reading Customers", comm_rec.tco_no, comm_rec.tco_name);

	sprintf(local_rec.st_rep, "%2.2s", local_rec.st_rep);
	sprintf(local_rec.end_rep, "%2.2s", local_rec.end_rep);

	strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", " ");
	cc = find_rec("cumr", &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp(cumr_rec.cm_co_no, comm_rec.tco_no))
	{
		/*---------------------
		| Validate rep range. |
		---------------------*/
		if (strcmp(cumr_rec.cm_sman, local_rec.st_rep) < 0 ||
		    strcmp(cumr_rec.cm_sman, local_rec.end_rep) > 0)
		{
			cc = find_rec("cumr", &cumr_rec, NEXT, "r");
			continue;
		}

		/*-----------------------------------------------
		| Validate that debtor is active for telesales. |
		-----------------------------------------------*/
		cc = find_hash(tspm, &tspm_rec, COMPARISON, "r", 
			       cumr_rec.cm_hhcu_hash);
		if (cc)
		{
			cc = find_rec("cumr", &cumr_rec, NEXT, "r");
			continue;
		}

		/*--------------------------
		| Validate next visit date |
		--------------------------*/
		if (tspm_rec.pm_n_vst_date > local_rec.prt_date)
		{
			cc = find_rec("cumr", &cumr_rec, NEXT, "r");
			continue;
		}

		dsp_process("Customer", cumr_rec.cm_dbt_no);

		strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
		sprintf(exsf_rec.sf_sman, "%2.2s", cumr_rec.cm_sman);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");	
		if (cc)
			sprintf(exsf_rec.sf_sman_name, "%-40.40s", "Unknown Salesman");

		sprintf(sort_str, 
			"%2.2s%-6.6s %8ld %8ld %-40.40s \n", 
			cumr_rec.cm_sman,
			cumr_rec.cm_dbt_no,
			tspm_rec.pm_n_vst_date,
			cumr_rec.cm_hhcu_hash,
			exsf_rec.sf_sman_name);

		sort_save(fsort, sort_str);

		cc = find_rec("cumr", &cumr_rec, NEXT, "r");
	}

	proc_sort();
	return (EXIT_SUCCESS);
}

void
proc_sort (void)
{
	int	first_time = TRUE;
	long	hhcu_hash;
	char	*sptr;
	char	curr_rep[3];
	char	prev_rep[3];

	fsort = sort_sort(fsort,"nxt_vst");

	strcpy(prev_rep, "");
	
	sptr = sort_read(fsort);
	while (sptr)
	{
		/*--------------------
		| Lookup tspm record |
		--------------------*/
		hhcu_hash = atol(sptr + 18);
		cc = find_hash(tspm, &tspm_rec, COMPARISON, "r", hhcu_hash);
		if (cc)
		{
			sptr = sort_read(fsort);
			continue;
		}

		/*--------------------
		| Lookup cumr record |
		--------------------*/
		cc = find_hash(cumr2, &cumr_rec, COMPARISON, "r", hhcu_hash);
		if (cc)
		{
			sptr = sort_read(fsort);
			continue;
		}

		dsp_process("Customer", cumr_rec.cm_dbt_no);

		if (first_time)
			head_output();

		sprintf(curr_rep, "%2.2s", sptr);
		if (strcmp(curr_rep, prev_rep))
		{
			rep_head(curr_rep, sptr + 27);
			if (!first_time)
				fprintf(fout, ".PA\n");

			strcpy(prev_rep, curr_rep);
		}
		else
		{
			fprintf(fout, "!--------");
			fprintf(fout, "!------------------------------------------");
			fprintf(fout, "!----------------");
			fprintf(fout, "!----------!-------");
			fprintf(fout, "!--------------------------------------------------------------------!\n");
		}

		sprintf(dbt_dtls, "! %-6.6s ! %-40.40s ! %-14.14s !%-10.10s! %-5.5s !",
			cumr_rec.cm_dbt_no,
			cumr_rec.cm_name,
			tspm_rec.pm_op_code,
			DateToString(tspm_rec.pm_n_vst_date),
			tspm_rec.pm_n_vst_time);

		prt_notes("V");

		first_time = FALSE;

		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"nxt_vst");
}

int
head_output (void)
{
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	pipe_open = TRUE;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".5\n");
	fprintf(fout,".L158\n");

	fprintf(fout,".ETELESALES REP LEAD SHEET\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".ECOMPANY : %s - %s\n",comm_rec.tco_no,comm_rec.tco_name);
	fprintf(fout,".B1\n");

	fprintf(fout, ".R=========");
	fprintf(fout, "===========================================");
	fprintf(fout, "=================");
	fprintf(fout, "===================");
	fprintf(fout, "======================================================================\n");

	fflush(fout);

	return(0);
}

void
rep_head (
 char*  rep,
 char*  rep_name)
{
	fprintf(fout, ".DS5\n");

	fprintf(fout, ".E SALESMAN: %2.2s  %-40.40s\n", rep, rep_name);

	fprintf(fout, "=========");
	fprintf(fout, "===========================================");
	fprintf(fout, "=================");
	fprintf(fout, "===================");
	fprintf(fout, "======================================================================\n");

	fprintf(fout, "!CUSTOMER");
	fprintf(fout, "!                                          ");
	fprintf(fout, "!                ");
	fprintf(fout, "!    NEXT VISIT    ");
	fprintf(fout, "!                                                                    !\n");

	fprintf(fout, "! NUMBER ");
	fprintf(fout, "!             CUSTOMER NAME                ");
	fprintf(fout, "!    OPERATOR    ");
	fprintf(fout, "!   DATE   ! TIME  ");
	fprintf(fout, "!                              NOTES                                 !\n");

	fprintf(fout, "!--------");
	fprintf(fout, "!------------------------------------------");
	fprintf(fout, "!----------------");
	fprintf(fout, "!----------!-------");
	fprintf(fout, "!--------------------------------------------------------------------!\n");

	fflush(fout);
}

/*--------------------------
| Print notes for customer |
--------------------------*/
int
prt_notes (
 char*  note_type)
{
	int	data_found;

	data_found = FALSE;
	tsxd_rec.xd_hhcu_hash = cumr_rec.cm_hhcu_hash;
	sprintf(tsxd_rec.xd_type, "%-1.1s", note_type);
	tsxd_rec.xd_line_no = 0;
	cc = find_rec(tsxd, &tsxd_rec, GTEQ, "r");
	while (!cc &&
	       tsxd_rec.xd_hhcu_hash == cumr_rec.cm_hhcu_hash &&
	       !strcmp(tsxd_rec.xd_type, note_type))
	{
		fprintf (fout, "%-89.89s", dbt_dtls);
		fprintf (fout, "    %-60.60s    !\n", tsxd_rec.xd_desc);
		sprintf (dbt_dtls, 
			"! %-6.6s ! %-40.40s ! %-14.14s ! %-8.8s ! %-5.5s !",
			" ", " ", " ", " ", " ");

		data_found = TRUE;
		cc = find_rec(tsxd, &tsxd_rec, NEXT, "r");
	}

	if (!data_found)
	{
		fprintf (fout, "%-89.89s", dbt_dtls);
		fprintf (fout, "    %-60.60s    !\n", " ");
	}

	return (EXIT_SUCCESS);
}


/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	char	hdng_date[11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
	
		strcpy(hdng_date,DateToString(TodaysDate()));
		rv_pr(hdng_date,71,0,0);

		move(0,1);
		line(80);

		rv_pr(ML(mlTsMess007),20,0,1);

		box(0, 3, 80, 8);
		move(1,6);
		line(79);
		move(1,8);
		line(79);

		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
