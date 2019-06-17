/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_stat.i.c    )                                 |
|  Program Desc  : ( Input Program for Service Job Status enquiry )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cudp, cumr,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 28/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (23/11/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (28/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (15/10/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (04/11/1997)    | Modified  by  : Jiggs Veloz.     |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|                :                                                    |
|     (28/11/89) : Tidy up screen prompts and decrease the number of  |
|                : arguments passed to print program.                 |
|     (13/09/97) : Updated for Multilingual Conversion. 			  |
|     (15/10/97) : Fixed MLDB error.					 			  |
|  (04/11/1997)  : Changed no_lps() to valid_lp().					  |
|                :                                                    |
|                                                                     |
| $Log: sj_stat.i.c,v $
| Revision 5.4  2002/07/25 11:17:31  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2002/07/17 09:57:51  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:42  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:41  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:30  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:46  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:10:06  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  2000/04/11 12:44:39  ramon
| Added description fields for "display", background, overnight and status fields.
|
| Revision 1.10  1999/11/17 06:40:51  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.9  1999/11/16 05:58:37  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.8  1999/09/29 10:13:08  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/24 05:06:43  scott
| Updated from Ansi
|
| Revision 1.6  1999/06/20 02:30:38  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_stat.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_stat.i/sj_stat.i.c,v 5.4 2002/07/25 11:17:31 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
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
		{"comm_est_name"}
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
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

int	envDbCo = 0,
	envDbFind = 0;

char	rep_desc[61],
	branchNo[3];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	back_desc[5];
	char	onight[5];
	char	onight_desc[5];
	int	lpno;
	char	rtype[2];
	char	dbt_no[7];
	char	dbt_name[41];
	char	s_fr_date[11];
	char	s_to_date[11];
	long	fr_date;
	long	to_date;
	char	p_or_d[9];
	char	p_or_d_desc[9];
	char 	rep_type[10];
	char 	rep_type_desc[10];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "disp_print", 4, 25, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Print or Display ", " ", 
		YES, NO, JUSTLEFT, "PD", "", local_rec.p_or_d}, 
	{1, LIN, "disp_print_desc", 4, 28, CHARTYPE, 
		"AAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.p_or_d_desc}, 
	{1, LIN, "lpno", 5, 25, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number ", " ", 
		YES, NO, JUSTRIGHT, "123456789", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 6, 25, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "back_desc", 6, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc}, 
	{1, LIN, "onight", 7, 25, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "onight_desc", 7, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc},
	{1, LIN, "cust", 9, 25, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "ALL   ", "Customer No ", "Default is ALL", 
		YES, NO, JUSTLEFT, "", "", local_rec.dbt_no}, 
	{1, LIN, "cu_name", 10, 25, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.dbt_name}, 
	{1, LIN, "status", 12, 25, CHARTYPE, 
		"U", "          ", 
		" ", "O", "Select Job Status.", " Enter O(pen or C(losed or I(nvoiced Jobs. ", 
		YES, NO, JUSTLEFT, "OCI", "", local_rec.rep_type}, 
	{1, LIN, "status_desc", 12, 28, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


#include <FindCumr.h>
/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
char* IntToStr (int iValue, char* cBuffer);
int spec_valid (int field);
void set_deflt (void);
void run_prog (char *prog_name);
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

	if (argc != 3) 
	{
		print_at(0,0, mlStdMess037 ,argv[0]);
		return (EXIT_FAILURE);
	}

	sprintf(rep_desc,"%-60.60s",argv[2]);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	OpenDB();

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = TRUE;

	init_vars(1);

	set_deflt();

	heading(1);
	scn_display(1);
	edit(1);
    if (restart) 
		shutdown_prog();
	else
        run_prog(argv[1]);

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

void
OpenDB (
 void)
{
	abc_dbopen("data");
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("cumr", cumr_list, cumr_no_fields, (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
}

void
CloseDB (
 void)
{
	abc_fclose("cumr");
	abc_dbclose("data");
}

char * 
IntToStr (
 int iValue, 
 char* cBuffer)
{
    sprintf (cBuffer, "%d", iValue);
    return (cBuffer);
}

int
spec_valid (
 int field)
{
	char	valid_inp[2];
			
	if (LCHECK ("disp_print") )
	{
		if (local_rec.p_or_d[0] == 'D')
		{
			FLD ("lpno") 	= NA;
			FLD ("back") 	= NA;
			FLD ("onight") 	= NA;
			local_rec.lpno 	= 0;
			strcpy(local_rec.p_or_d_desc, "Display");
		}
		else
		{
			FLD ("lpno") 	= YES;
			FLD ("back") 	= YES;
			FLD ("onight") 	= YES;
			strcpy(local_rec.p_or_d_desc, "Print  ");
			local_rec.lpno = 1;
		}
		DSP_FLD("disp_print_desc");
		DSP_FLD("lpno");
		DSP_FLD("back");
		DSP_FLD("onight");
		return(0);
	}

	if (LCHECK ("cust") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.dbt_name,"All customers selected");
			display_field(label("cu_name"));
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no,branchNo);
		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.dbt_no));
		cc = find_rec("cumr", &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*sprintf(err_str,"Customer %s is not on file.",local_rec.dbt_no);*/
			errmess(ML(mlStdMess021));
			return(1);
		}
		strcpy(local_rec.dbt_name,cumr_rec.cm_name);
		DSP_FLD ("cu_name");
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
			errmess(ML(mlStdMess020));
			return(1);
		}
		return(0);
	}
		
	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK("back"))
	{
		sprintf(valid_inp, "%1.1s", local_rec.back);

		if (valid_inp[0] == 'N')
			strcpy(local_rec.back_desc, "No ");
		else
		{
			strcpy(local_rec.back_desc, "Yes");
			if (local_rec.onight[0] == 'Y')
			{
				strcpy(local_rec.onight, "N");
				strcpy(local_rec.onight_desc, "No ");
			}
		}

		DSP_FLD ("back_desc");
		DSP_FLD ("onight");
		DSP_FLD ("onight_desc");
		return(0);
	}

	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK("onight"))
	{
		sprintf(valid_inp, "%1.1s", local_rec.onight);

		if (valid_inp[0] == 'N')
			strcpy(local_rec.onight_desc, "No ");
		else
		{
			if (local_rec.back[0] == 'Y')
			{
				strcpy(local_rec.back, "N");
				strcpy(local_rec.back_desc, "No ");
			}
			strcpy(local_rec.onight_desc, "Yes");
		}

		DSP_FLD ("back");
		DSP_FLD ("back_desc");
		DSP_FLD ("onight_desc");
		return(0);
	}

	if (LCHECK("status"))
	{
		switch (local_rec.rep_type[0])
		{
		case	'O':
			strcpy(local_rec.rep_type_desc, "Open    ");
			break;

		case	'C':
			strcpy(local_rec.rep_type_desc, "Closed  ");
			break;

		case	'I':
			strcpy(local_rec.rep_type_desc, "Invoiced");
			break;

		default:
			return(1);
			break;
		}
		DSP_FLD ("status_desc");
		return(0);
	}

	return(0);
}

void
set_deflt (
 void)
{
	strcpy(local_rec.p_or_d, "D");
	strcpy(local_rec.p_or_d_desc, "Display");
	strcpy(local_rec.back,"N");
	strcpy(local_rec.back_desc,"No ");
	strcpy(local_rec.onight,"N");
	strcpy(local_rec.onight_desc,"No ");
	strcpy(local_rec.dbt_no,"ALL   ");
	strcpy(local_rec.dbt_name,"All customers selected");
	strcpy(local_rec.rep_type,"O");
	strcpy(local_rec.rep_type_desc,"Open    ");
	FLD("lpno") 	= NA;
	FLD("back") 	= NA;
	FLD("onight")	= NA;
}

void
run_prog (
 char *prog_name)
{
	char	printer_no[3];

	rset_tty();

	sprintf(printer_no,"%2d",local_rec.lpno);
	local_rec.p_or_d[1] = '\0';
	local_rec.rep_type[1] = '\0';

	shutdown_prog ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.p_or_d,
				local_rec.dbt_no,
				local_rec.rep_type,
				printer_no,
				rep_desc,(char *)0);
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.p_or_d,
				local_rec.dbt_no,
				local_rec.rep_type,
				printer_no,(char *)0);
	}
	else 
		execlp(prog_name,
			prog_name,
			local_rec.p_or_d,
			local_rec.dbt_no,
			local_rec.rep_type,
			printer_no,(char *)0);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();

		rv_pr(rep_desc,(80 - strlen(clip(rep_desc))) / 2,0,1);
		move(0,1);
		line(80);

		box(0,3,80,9);

		move(1,8);
		line(79);
		move(1,11);
		line(79);
		move(0,20);
		print_at(20,0, ML(mlStdMess038) ,comm_rec.tco_no,comm_rec.tco_name);
		print_at(21,0, ML(mlStdMess039) ,comm_rec.test_no,comm_rec.test_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
