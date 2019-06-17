/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_salesm.i.c )                                  |
|  Program Desc  : ( Sales Analysis By Salesman/Category/Item        )|
|                : ( Input Program.                                  )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, cumr, sapc, esmr, excl, excf, exsf    |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 07/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : 02/03/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 04/09/97        | Modified  by  : Ana Marie Tario. |
|  Date Modified : 15/10/97        | Modified  by  : Ana Marie Tario. |
|  Date Modified : 04/11/1997      | Modified  by  : Jiggs Veloz.     |
|                                                                     |
|  Comments      : Use standard sa_salesm.i.c as base program.        |
|                : (04/09/97) - Incorporated multilingual conversion  |
|                :              and DMY4 date.                        |
|                : (15/10/97) - Inserted additional ML().             |
|                : (04/11/1997) Changed no_lps() to valid_lp().       |
| $Log: smancat.i.c,v $
| Revision 5.3  2002/07/17 09:57:48  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:53  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:07  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:35  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/12/06 01:35:33  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 04:55:35  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.9  1999/09/29 10:12:52  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:37  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 02:01:53  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:23  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "sa_smancat.i.c",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_smancat.i/smancat.i.c,v 5.3 2002/07/17 09:57:48 scott Exp $";

#include	<ml_sa_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#include	<get_lpno.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tinv_date;
	} comm_rec;

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

	/*==========================================
	| Establishment/Branch Master File Record. |
	==========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	int esmr_no_fields = 3;

	struct {
		char	esmr_co_no[3];
		char	esmr_est_no[3];
		char	esmr_est_name[41];
	} esmr_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int exsf_no_fields = 3;

	struct {
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
	} exsf_rec;

char	branchNo[3];
char	title[51];
char	br_comment[26];

int		envDbCo = 0;
int		envDbFind = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	br_no[4];
	char	br_name[41];
	char	s_sman[3];
	char	e_sman[3];
	char	s_name[41];
	char	e_name[41];
	char	det_summ[2];
	char	rep_type[9];
	char	cost_mgn[5];
	char	back[5];
	char	onight[5];
	int	lpno;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "br_no",	 4, 21, CHARTYPE,
		"UU", "          ",
		" ", "All", "Branch No ", br_comment,
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_name",	 4, 42, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_name},
	{1, LIN, "det_summ",	 5, 21, CHARTYPE,
		"U", "          ",
		" ", "S", "Detailed / Summary ", " D(etailed or S(ummary ",
		YES, NO,  JUSTLEFT, "DS", "", local_rec.det_summ},
	{1, LIN, "rep_type",	 5, 42, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "Summary", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rep_type},
	{1, LIN, "s_sman",	 7, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Salesman ", " Default is All ",
		YES, NO, JUSTRIGHT, "", "", local_rec.s_sman},
	{1, LIN, "s_name",	 8, 21, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_name},
	{1, LIN, "e_sman",	 9, 21, CHARTYPE,
		"UU", "          ",
		" ", "~~", "End Salesman   ", " Default is All ",
		YES, NO, JUSTRIGHT, "", "", local_rec.e_sman},
	{1, LIN, "e_name",	10, 21, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Name ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_name},
	{1, LIN, "cost_mgn",	11, 21, CHARTYPE,
		"UUUU", "          ",
		" ", "Y(es", "Print Cost-Margin", " Y(es) | N(o) ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.cost_mgn},
	{1, LIN, "lpno",	13, 21, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer ", " ",
		YES, NO,  JUSTLEFT, "", "", (char*) &local_rec.lpno},
	{1, LIN, "back",	14, 21, CHARTYPE,
		"UUUU", "          ",
		" ", "N(o", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	14, 60, CHARTYPE,
		"UUUU", "          ",
		" ", "N(o", "Overnight ", " ",
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Functions Prototypes.
=====================================================================*/
int run_prog (char* prog_name);
void load_dflt (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void br_srch (char* key_val);
void sman_srch (char* key_val);
int heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	SETUP_SCR (vars);

	if (argc != 2)
	{
		print_at(0,0,mlSaMess742, argv[0]);
        return (EXIT_FAILURE);
	}

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

	strcpy(branchNo,(envDbCo) ? comm_rec.test_no : " 0");
	
	swide();
	clear();

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
		init_ok = 1;
		search_ok = 1;
		init_vars(1);
		crsr_on();

		load_dflt();

		/*------------------------------
	        | Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
        {
			shutdown_prog();
            return (EXIT_SUCCESS);
        }

		if (run_prog (argv[1]) == 1)
		{
			return (EXIT_SUCCESS);
		}
        prog_exit = 1;
	}	/* end of input control loop	*/
	shutdown_prog();
    return (EXIT_SUCCESS);
}

int
run_prog (
 char*  prog_name)
{
	char	start_field[3];
	char	end_field[3];
	char	printer_no[3];

	sprintf(printer_no,"%2d",local_rec.lpno);

	sprintf(start_field,"%-2.2s",local_rec.s_sman);
	sprintf(end_field,"%-2.2s",local_rec.e_sman);

	shutdown_prog ();
	
	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork() == 0)
        {
			execlp ("ONIGHT",
				    "ONIGHT",
                    prog_name,
                    local_rec.det_summ,
                    start_field,
                    end_field,
                    printer_no,
                    local_rec.br_no,
                    local_rec.cost_mgn,
                    "sa_smancat",
                    (char *)0);
        }
	}
	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
        {
			execlp(prog_name,
				prog_name,
				local_rec.det_summ,
				start_field,
				end_field,
				printer_no,
				local_rec.br_no,
				local_rec.cost_mgn,(char *)0);
        }
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.det_summ,
			start_field,
			end_field,
			printer_no,
			local_rec.br_no,
			local_rec.cost_mgn,(char *)0);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*=========================
| Load in default values. |
=========================*/
void
load_dflt (void)
{
	strcpy(br_comment,"Return For All Branches");
	strcpy(local_rec.br_no,"All");
	sprintf(local_rec.br_name,"%-40.40s","All Branches ");
	strcpy(local_rec.det_summ,"S");
	strcpy(local_rec.rep_type,"Summary");

	sprintf(title,"%-50.50s"," Salesman Sales By Category Report ");
	strcpy(local_rec.s_sman,"  ");
	strcpy(local_rec.e_sman,"~~");
	sprintf(local_rec.s_name,"%-40.40s","Start Salesman");
	sprintf(local_rec.e_name,"%-40.40s","End   Salesman");
	strcpy(local_rec.cost_mgn,"Y(es");

	local_rec.lpno = 1;
	strcpy(local_rec.back,"N(o");
	strcpy(local_rec.onight,"N(o");
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen("data");

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec("exsf",exsf_list,exsf_no_fields,"exsf_id_no");
	open_rec("cumr",cumr_list,cumr_no_fields, (!envDbFind) ? "cumr_id_no"
														 : "cumr_id_no3");
	open_rec("esmr",esmr_list,esmr_no_fields,"esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("exsf");
	abc_fclose("cumr");
	abc_fclose("esmr");
	abc_dbclose("data");
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("br_no"))
	{
		if (dflt_used)
		{
			strcpy(br_comment,"Return For All Branches");
			strcpy(local_rec.br_no,"All");
			strcpy(local_rec.br_name,"All Branches");
			DSP_FLD("br_name");
			return(0);
		}

		if (last_char == SEARCH)
		{
			   br_srch(temp_str);
		       return(0);
		}

		strcpy(esmr_rec.esmr_co_no,comm_rec.tco_no);
		sprintf(esmr_rec.esmr_est_no,"%-2.2s",local_rec.br_no);
		cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");	
		if (cc)
		{
			/*sprintf(err_str," Branch %s Is Not on File ",local_rec.br_no);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess073));
			sleep(2);
			return(1);
		}

		if (envDbCo == 0)
			strcpy(branchNo," 0");
		else
		{
			strcpy(branchNo,local_rec.br_no);
			strcpy(comm_rec.test_no,local_rec.br_no);
		}

		sprintf(local_rec.br_no,"%-2.2s ",esmr_rec.esmr_est_no);
		DSP_FLD("br_no");
		strcpy(local_rec.br_name,esmr_rec.esmr_est_name);
		DSP_FLD("br_name");
		return(0);
	}

	if (LCHECK ("det_summ"))
	{
		if (local_rec.det_summ[0] == 'S')
			sprintf(title,"%-50.50s"," Salesman Sales By Customer Report ");
		else
			sprintf(title,"%-50.50s"," Salesman Sales By Customer By Item Report ");

		strcpy(local_rec.rep_type,(local_rec.det_summ[0] == 'D') ? "Detail" : "Summary");
		display_field(field+1);
		move(20,0);
		cl_line();
		/*rv_pr(" Print Sales By Salesman By Item ",49,0,1);*/
		rv_pr(ML(mlSaMess031),49,0,1);
		return(0);
	}

	if (LCHECK ("s_sman"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.s_sman,"%-2.2s","  ");
			sprintf(local_rec.e_sman,"%-2.2s","~~");
			sprintf(local_rec.s_name,"%-40.40s","Start Salesman");
			sprintf(local_rec.e_name,"%-40.40s","End   Salesman");

			DSP_FLD("s_sman");
			DSP_FLD("e_sman");
			DSP_FLD("s_name");
			DSP_FLD("e_name");
			return(0);
		}
		FLD("e_sman") = YES;

		if (prog_status != ENTRY && strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
			/*sprintf(err_str,"End Salesman %s may not be less than %s",local_rec.e_sman,local_rec.s_sman);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess018));
			sleep(2);
			return(1);
		}

		if (last_char == SEARCH)
		{
			sman_srch(temp_str);
			return(0);
		}
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.s_sman);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			/*sprintf(err_str,"Salesman %-2.2s Is Not On File",local_rec.s_sman);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess135));
			sleep(2);
			return(1);
		}
		sprintf(local_rec.s_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.s_name,exsf_rec.sf_salesman);

		DSP_FLD("s_sman");
		DSP_FLD("s_name");
		return(0);
	}

	if (LCHECK ("e_sman"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.e_sman,"%-2.2s","~~");
			sprintf(local_rec.e_name,"%-40.40s","End   Salesman");
			display_field(label("e_sman"));
			display_field(label("e_name"));
			return(0);
		}

		if (last_char == SEARCH)
		{
			sman_srch(temp_str);
			return(0);
		}

		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.e_sman);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			/*sprintf(err_str,"Salesman %-2.2s Is Not On File",local_rec.e_sman);
			print_mess(err_str);*/
			print_mess(ML(mlStdMess135));
			sleep(2);
			return(1);
		}

		if (strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
/*
			sprintf(err_str,"End Salesman %s may not be less than %s",local_rec.e_sman,local_rec.s_sman);
			errmess(err_str);*/
			errmess(ML(mlStdMess018));
			sleep(2);
			return(1);
		}
		sprintf(local_rec.e_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.e_name,exsf_rec.sf_salesman);
		DSP_FLD("e_sman");
		DSP_FLD("e_name");
		return(0);
	}

	if (LCHECK ("lpno"))
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			return(1);
		}

		return(0);
	}

	if (LCHECK ("cost_mgn"))
	{
		strcpy(local_rec.cost_mgn,(local_rec.cost_mgn[0] == 'Y') ? "Y(es" : "N(o");
		display_field(field);
		return(0);
	}

	if (LCHECK ("back"))
	{
		strcpy(local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o");
		display_field(field);
		return(0);
	}

	if (LCHECK ("onight"))
	{
		strcpy(local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o");
		display_field(field);
		return(0);
	}
	return(0);
}

void
br_srch (
 char*  key_val)
{
        work_open();
	save_rec("#Br No.    ","#Br Name           ");
	strcpy(esmr_rec.esmr_co_no,comm_rec.tco_no);
	sprintf(esmr_rec.esmr_est_no,"%-2.2s",key_val);
	cc = find_rec("esmr",&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp(esmr_rec.esmr_co_no,comm_rec.tco_no) && 
			      !strncmp(esmr_rec.esmr_est_no,key_val,strlen(key_val)))
	{
		cc = save_rec(esmr_rec.esmr_est_no,esmr_rec.esmr_est_name);
		if (cc)
			break;
		cc = find_rec("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(esmr_rec.esmr_co_no,comm_rec.tco_no);
	sprintf(esmr_rec.esmr_est_no,"%-2.2s",temp_str);
	cc = find_rec("esmr",&esmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in esmr During (DBFIND)",cc,PNAME);
}

void
sman_srch (
 char*  key_val)
{
	work_open();
	save_rec("#Sm","#Salesman Name");
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp(exsf_rec.sf_co_no,comm_rec.tco_no) && 
			      !strncmp(exsf_rec.sf_salesman_no,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_salesman_no,exsf_rec.sf_salesman);
		if (cc)
			break;
		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		swide();
		clear();

		rv_pr( ML(mlSaMess031),49,0,1);
		
		move(0,1);
		line(132);

		move(1,6);
		line(131);
		box(0,3,132,11);
		move(1,12);
		line(131);

		move(0,20);
		line(132);
/*
		print_at(21,0," Company no. : %s - %s",comm_rec.tco_no,comm_rec.tco_name);*/
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0, err_str,comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(132);
		/* Reset this variable for new screen NOT page */
		line_cnt = 0; 
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
