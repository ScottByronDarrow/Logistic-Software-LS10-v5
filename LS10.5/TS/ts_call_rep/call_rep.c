/*=====================================================================
|  Copyright (C) 1988 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_call_rep.c )                                  |
|  Program Desc  : ( Telesales Called Customer Cycle Report       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 15/09/92         |
|---------------------------------------------------------------------|
|  Date Modified : (15/10/1997)    | Modified  by  : Jiggs A Veloz.   |
|  Date Modified :   /  /          | Modified  by  :                  |
|                                                                     |
|  Comments      :                                                    |
|  (15/10/1997)  : SEL - Multilingual Covnersion.                     |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: call_rep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_call_rep/call_rep.c,v 5.3 2002/07/17 09:58:15 scott Exp $";

#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_ts_mess.h>
#include	<ml_std_mess.h>

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

	/*==============================
	| Tele-Marketing OPerator file |
	==============================*/
	struct dbview tmop_list[] ={
		{"tmop_co_no"},
		{"tmop_op_id"},
		{"tmop_op_name"},
	};

	int	tmop_no_fields = 3;

	struct	{
		char	op_co_no[3];
		char	op_op_id[15];
		char	op_op_name[41];
	} tmop_rec;

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
		{"tspm_n_phone_date"},
		{"tspm_op_code"},
	};

	int	tspm_no_fields = 3;

	struct	{
		long	pm_hhcu_hash;
		long	pm_n_phone_date;
		char	pm_op_code[15];
	} tspm_rec;

	char	*data  = "data",
	    	*comm  = "comm",
	    	*cumr  = "cumr",
	    	*tspm  = "tspm",
	    	*tmop  = "tmop";

	int	pipe_open;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	char	st_op[15];
	char	st_op_name[41];
	char	end_op[15];
	char	end_op_name[41];
	long	st_date;
	long	end_date;
	char	back[4];
	char	onight[4];
	char	lp_str[3];
	int	lpno;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "st_op",	 4, 16, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", " Start Operator :", "",
		NO, NO, JUSTRIGHT, "", "", local_rec.st_op},
	{1, LIN, "st_op_name",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_op_name},
	{1, LIN, "end_op",	 5, 16, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "", " End Operator   :", "",
		NO, NO, JUSTRIGHT, "", "", local_rec.end_op},
	{1, LIN, "end_op_name",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_op_name},

	{1, LIN, "st_date",	 7, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " Start Date     :", "",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.st_date},
	{1, LIN, "end_date",	 8, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, " End Date       :", "",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.end_date},

	{1,LIN,"lpno",	10,16,INTTYPE,
		"NN","          ",
		" ","1", " Printer No     :","",
		NO,NO, JUSTLEFT,"","",(char *)&local_rec.lpno},
	{1,LIN,"back",	11,16,CHARTYPE,
		"U","          ",
		" ","", " Background     :","",
		NO,NO, JUSTLEFT,"","",local_rec.back},
	{1,LIN,"onight",	12,16,CHARTYPE,
		"U","          ",
		" ","", " Overnight      :","",
		NO,NO, JUSTLEFT,"","",local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void    shutdown_prog (void);
void    OpenDB (void);
void    CloseDB (void);
int     spec_valid (int field);
void    SrchTmop (char *key_val);
void    run_prog (char *prog_name, char *prog_desc);
int     process (void);
void    proc_sort (void);
int     head_output (void);
int     heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	if (argc != 2 && argc != 6)
	{
		/*---------------------------
		| Usage: %s <description>\n	|
		| OR  :    <lpno>\n			|
		|          <st_op>\n		|
		|          <end_op>\n		|
		|          <start date>\n	|
		|          <end date>\n		|
		---------------------------*/
		print_at(0,0, mlTsMess709, argv[0]);
		print_at(0,0, mlTsMess710);
		print_at(0,0, mlTsMess702);
		print_at(0,0, mlTsMess703);
		print_at(0,0, mlTsMess721);
		print_at(0,0, "%s\n", mlTsMess722);
        return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 6)
	{
		local_rec.lpno = atoi(argv[1]);
		sprintf(local_rec.st_op, "%2.2s", argv[2]);
		sprintf(local_rec.end_op, "%2.2s", argv[3]);
		local_rec.st_date  = StringToDate(argv[4]);
		local_rec.end_date = StringToDate(argv[5]);

		pipe_open = FALSE;
		process();

		if (pipe_open)
		{
			fprintf(fout, ".EOF\n");
			pclose(fout);
		}
	
		shutdown_prog();   
        return (EXIT_FAILURE);
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

		run_prog (argv[0], argv[1]);
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
	
	open_rec(cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec(tspm, tspm_list, tspm_no_fields, "tspm_hhcu_hash");
	open_rec(tmop, tmop_list, tmop_no_fields, "tmop_id_no");
}

void
CloseDB (void)
{
	abc_fclose(cumr);
	abc_fclose(tspm);
	abc_fclose(tmop);
	abc_dbclose(data);
}

int
spec_valid(
 int    field)
{
	if (LCHECK("st_op"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_op, "              ");
			sprintf(local_rec.st_op_name, "%-40.40s", "First Operator");
			DSP_FLD("st_op_name");

			return(0);
		}

		if (SRCH_KEY)
		{
			SrchTmop(temp_str);
			return(0);
		}

		strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
		sprintf(tmop_rec.op_op_id, "%-14.14s", local_rec.st_op);
		cc = find_rec("tmop", &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*----------------------------
			| Operator Not Found On File |
			----------------------------*/
			print_mess( ML(mlStdMess168) );
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.st_op_name, "%-40.40s", tmop_rec.op_op_name);
		
		DSP_FLD("st_op_name");
		return(0);
	}

	if (LCHECK("end_op"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_op, "~~~~~~~~~~~~~~");
			sprintf(local_rec.end_op_name, "%-40.40s", "Last Operator");
			DSP_FLD("end_op_name");

			return(0);
		}

		if (SRCH_KEY)
		{
			SrchTmop(temp_str);
			return(0);
		}

		strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
		sprintf(tmop_rec.op_op_id, "%-14.14s", local_rec.end_op);
		cc = find_rec("tmop", &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*----------------------------
			| Operator Not Found On File |
			----------------------------*/
			print_mess( ML(mlStdMess168) );
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.end_op_name, "%-40.40s", tmop_rec.op_op_name);
		
		DSP_FLD("end_op_name");
		return(0);
	}

	if (LCHECK("st_date"))
	{
		if (prog_status != ENTRY && 
		    local_rec.st_date > local_rec.end_date)
		{
			/*------------------------------------------
			| End Date Must Be Greater Than Start Date |
			------------------------------------------*/
			print_mess( ML(mlStdMess026) );
			sleep(2);
			clear_mess();
			return(1);
		}

		return(0);
	}

	if (LCHECK("end_date"))
	{
		if (local_rec.st_date > local_rec.end_date)
		{
			/*------------------------------------------
			| End Date Must Be Greater Than Start Date |
			------------------------------------------*/
			print_mess( ML(mlStdMess026) );
			sleep(2);
			clear_mess();
			return(1);
		}

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
			/*-----------------
			| Invalid Printer |
			-----------------*/
			print_mess( ML(mlStdMess020) );
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
	
	return (EXIT_SUCCESS);
}

void
SrchTmop (
 char*  key_val)
{
	work_open();
	strcpy(tmop_rec.op_co_no,comm_rec.tco_no);
	sprintf(tmop_rec.op_op_id, "%-14.14s", key_val);
	save_rec("#Operator I.D.","#Operator Full Name.");
	cc = find_rec(tmop, &tmop_rec, GTEQ, "r");
	while (!cc && !strcmp(tmop_rec.op_co_no, comm_rec.tco_no) &&
		      !strncmp(tmop_rec.op_op_id, key_val, strlen(key_val)))
	{
		cc = save_rec(tmop_rec.op_op_id, tmop_rec.op_op_name);
		if (cc)
			break;

		cc = find_rec(tmop, &tmop_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
	sprintf(tmop_rec.op_op_id, "%-14.14s", temp_str);
	cc = find_rec(tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
		file_err( cc, "tmop", "DBFIND" );
}

void 
run_prog (
 char*  prog_name,
 char*  prog_desc)
{
	char	st_date [11];
	char	end_date[11];

	sprintf(local_rec.lp_str, "%d", local_rec.lpno);
	sprintf(st_date,  "%-10.10s", DateToString(local_rec.st_date));
	sprintf(end_date, "%-10.10s", DateToString(local_rec.end_date));
	
	shutdown_prog ();
	rset_tty();

	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				local_rec.st_op,
				local_rec.end_op, 
				st_date,
				end_date,
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
				local_rec.st_op,
				local_rec.end_op, 
				st_date,
				end_date, (char *)0);
		else
			return;
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.st_op,
			local_rec.end_op, 
			st_date,
			end_date, (char *)0);
	}
}

int
process (void)
{
	char	sort_str[256];

 	fsort = sort_open("call_rp");

	dsp_screen("Reading Customers", comm_rec.tco_no, comm_rec.tco_name);

	cc = find_hash(tspm, &tspm_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		/*-------------------------------------------
		| Validate Lead against selection criteria. |
		-------------------------------------------*/
		if (strcmp(tspm_rec.pm_op_code, local_rec.st_op) < 0  ||
		    strcmp(tspm_rec.pm_op_code, local_rec.end_op) > 0 ||
		    strlen(clip(tspm_rec.pm_op_code)) == 0           ||
		    tspm_rec.pm_n_phone_date < local_rec.st_date      ||
		    tspm_rec.pm_n_phone_date > local_rec.end_date)
		{
			cc = find_hash(tspm, &tspm_rec, NEXT, "r", 0L);
			continue;
		}

		/*-----------------------
		| Validate cumr record. |
		-----------------------*/
		cc = find_hash(cumr, &cumr_rec, COMPARISON, "r", 
			       tspm_rec.pm_hhcu_hash);
		if (cc)
		{
			cc = find_hash(tspm, &tspm_rec, NEXT, "r", 0L);
			continue;
		}
		if (strcmp(cumr_rec.cm_co_no, comm_rec.tco_no))
		{
			cc = find_hash(tspm, &tspm_rec, NEXT, "r", 0L);
			continue;
		}

		dsp_process("Customer", cumr_rec.cm_dbt_no);

		sprintf(sort_str, 
			"%-14.14s %010ld %-6.6s %-40.40s\n",
			tspm_rec.pm_op_code,
			tspm_rec.pm_n_phone_date,
			cumr_rec.cm_dbt_no,
			cumr_rec.cm_name);

		sort_save(fsort, sort_str);

		cc = find_hash(tspm, &tspm_rec, NEXT, "r", 0L);
	}

	proc_sort();
	return(0);
}

void
proc_sort (void)
{
	int	first_time = TRUE;
	char	*sptr;
	long	curr_date;
	long	prev_date;
	char	curr_op[15];
	char	prev_op[15];
	char	date_str[11];

	fsort = sort_sort(fsort,"call_rp");

	strcpy(prev_op, "");
	prev_date = 0L;
	
	sptr = sort_read(fsort);
	while (sptr)
	{
		dsp_process("Customer", cumr_rec.cm_dbt_no);

		if (first_time)
			head_output();

		sprintf(curr_op, "%-14.14s", sptr);
		curr_date = atol(sptr + 15);
		if (strcmp(curr_op, prev_op) ||
		    curr_date != prev_date)
		{
			if (strcmp(curr_op, prev_op))
			{
				fprintf(fout, 
					".PD! %-14.14s %-52.52s !\n", 
					curr_op,
					" ");

				if (!first_time)
					fprintf(fout, ".PA\n");

				strcpy(prev_op, curr_op);
			}

			sprintf(date_str, "%-10.10s", DateToString(curr_date));
		
			prev_date = curr_date;
		}

		fprintf(fout, 
			"!   %-10.10s    ! %-6.6s ! %-40.40s !\n",
			date_str,
			sptr + 26,	/*24,*/
			sptr + 33);	/*31);*/

		sprintf(date_str, "%-10.10s", " ");
		first_time = FALSE;

		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"call_rp");
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
	fprintf(fout,".8\n");
	fprintf(fout,".L80\n");

	fprintf(fout,".ETELESALES CUSTOMER CALL CYCLE REPORT\n");
	fprintf(fout,".B1\n");
	fprintf(fout,"COMPANY : %s - %s\n",comm_rec.tco_no,comm_rec.tco_name);
	fprintf(fout,".B1\n");

	fprintf(fout, "==================");
	fprintf(fout, "=========");
	fprintf(fout, "============================================\n");

	fprintf(fout, "! NEXT PHONE DATE ");
	fprintf(fout, "!CUSTOMER");
	fprintf(fout, "!             CUSTOMER NAME                !\n");

	fprintf(fout, "!-----------------");
	fprintf(fout, "!--------");
	fprintf(fout, "!------------------------------------------!\n");

	fprintf(fout, ".R==================");
	fprintf(fout, "=========");
	fprintf(fout, "============================================\n");

	fflush(fout);

	return (EXIT_SUCCESS);
}

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
		rv_pr(hdng_date,70,0,0);

		move(0,1);
		line(80);
		/*---------------------------------------
		| Telesales Customer Call Cycle Report. |
		---------------------------------------*/
		rv_pr( ML(mlTsMess095), 20, 0, 1);

		box(0, 3, 80, 9);

		move(1,6);
		line(79);

		move(1,9);
		line(79);

		move(1,20);
		line(80);

		/*Company: %s - %s*/ 
		print_at(21,1, ML(mlStdMess038),comm_rec.tco_no, comm_rec.tco_name);

		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
