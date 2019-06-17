/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_lists.c     )                                 |
|  Program Desc  : ( External file print program.                 )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sjvh, sjlr, sjsr,                           |
|  Database      : (comm)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitord   | Date Written  : 02/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (02/04/87)      | Modified  by  : Lance Whitford   |
|  Date Modified : (25/11/88)      | Modified  by  : B.C.Lim.         |
|  Date Modified : (29/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (18/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (11/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (14/10/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (02/08/99)      | Modified  by  : Mars dela CRuz.  |
|                                                                     |
|  Comments      : Tidy up program and combine the input program i.e  |
|                : sj_vhlst.i or sj_lrlst.i into sj_lists             |
|                :                                                    |
|     (29/11/89) : Remove the DOLLARS on double fields (previously    |
|                : were money fields).                                |
|                : (18/09/90) - General Update for New Scrgen. S.B.D. |
|     (14/10/97) : Fixed mldb error and added ML() to array.          | 
|                :                                                    |
|                                                                     |
| $Log: sj_lists.c,v $
| Revision 5.3  2002/07/17 09:57:50  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:38  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:24  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:58  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:34:27  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/17 06:40:48  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.11  1999/11/16 05:58:34  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.10  1999/10/20 02:07:04  nz
| Updated for final changes on date routines.
|
| Revision 1.9  1999/09/29 10:13:04  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 05:06:38  scott
| Updated from Ansi
|
| Revision 1.7  1999/06/20 02:30:33  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_lists.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_lists/sj_lists.c,v 5.3 2002/07/17 09:57:50 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>
#include    <std_decs.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dbt_date"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
		long	tdbt_date;
	} comm_rec;

	/*========================
	| Vehicles rates master  |
	=========================*/
	struct dbview sjvh_list[] ={
		{"sjvh_co_no"},
		{"sjvh_est_no"},
		{"sjvh_code"},
		{"sjvh_vehicle"},
		{"sjvh_rate"}
	};

	int sjvh_no_fields = 5;

	struct {
		char	vh_co_no[3];
		char	vh_est_no[3];
		char	vh_code[4];
		char	vh_vehicle[21];
		double	vh_rate;
	} sjvh_rec;

	/*===================
	| Labour rates file |
	===================*/
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

	/*==========================
	| Serviceperson rates file |
	==========================*/
	struct dbview sjsr_list[] ={
		{"sjsr_co_no"},
		{"sjsr_est_no"},
		{"sjsr_dp_no"},
		{"sjsr_code"},
		{"sjsr_name"},
		{"sjsr_lb_rt_code"}
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

	int		report_type = 1,
			lp_no = 1;

	char	rep_type[2];
	
	FILE	*popen(const char *, const char *),
		 	*pp;
	
/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	onight[5];
	char	lpno[2];
	int		lp_no;
	char	rep_type[3][5];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "rep_type", 6, 25, CHARTYPE, 
		"U", "          ", 
		" ", "Y(es", "Vehicle Rates ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.rep_type[0]}, 
	{1, LIN, "rep_type", 7, 25, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Labour Rates ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.rep_type[1]}, 
	{1, LIN, "rep_type", 8, 25, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Service Person Rates ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.rep_type[2]}, 
	{1, LIN, "lpno", 10, 25, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No ", " ", 
		YES, NO, JUSTRIGHT, "123456789", "", (char *)&local_rec.lp_no}, 
	{1, LIN, "back", 11, 25, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Background ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 11, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Overnight ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

	char *rep_desc[2] ;

/*=====================
| Function Prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void run_prog (char *prog_name);
void set_dflt (void);
void start_report (int prnt_no);
void prnt_sjvh (void);
void prnt_sjlr (void);
void prnt_sjsr (void);
void initML (void);
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
	if (argc != 1 && argc != 3)
	{
		print_at (0,0,mlSjMess701,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	initML ();		
	if (argc == 3)
	{
		lp_no = atoi (argv[1]);
		switch (argv[2][0])
		{
		case '1':
		case '2':
		case '3':
			report_type = atoi(argv[2]);
			strcpy (rep_type,argv[2]);
			break;

		default :
			print_at (1,0,ML(mlSjMess702));
			print_at (2,0,ML(mlSjMess703));
			print_at (3,0,ML(mlSjMess704));
			return (EXIT_FAILURE);
		}

		sprintf (err_str,"Printing %s",rep_desc[report_type - 1]);
		dsp_screen (err_str,comm_rec.tco_no,comm_rec.tco_name);

		start_report (lp_no);

		switch (rep_type[0])	
		{
		case '1':
			prnt_sjvh ();
			break;
		case '2':
			prnt_sjlr ();
			break;
		case '3':
			prnt_sjsr ();
			break;
		}
		fprintf (pp,".EOF\n");
		pclose (pp);
		shutdown_prog();
		return (EXIT_SUCCESS);
	}
	
	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		
		set_dflt ();

		heading (1);
		scn_display (1);
		edit (1);
		if (restart || prog_exit)
		{
			shutdown_prog();
	        return (EXIT_SUCCESS);
		}
		run_prog(argv[0]);
	}
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

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");
	open_rec ("sjvh", sjvh_list, sjvh_no_fields, "sjvh_id_no");
	open_rec ("sjlr", sjlr_list, sjlr_no_fields, "sjlr_id_no");
	open_rec ("sjsr", sjsr_list, sjsr_no_fields, "sjsr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("sjvh");
	abc_fclose ("sjlr");
	abc_fclose ("sjsr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int	i = 0;
	int	found_yes = 0;
	char	type[2];

	if (LCHECK ("rep_type"))
	{
		sprintf (type,"%1.1s",local_rec.rep_type[field]);

		strcpy (local_rec.rep_type[0],"N(o ");
		strcpy (local_rec.rep_type[1],"N(o ");
		strcpy (local_rec.rep_type[2],"N(o ");

		if (dflt_used)
		{
			strcpy (local_rec.rep_type[field],"Y(es");
			sprintf (rep_type,"%d",field + 1);
			for (i = 0; i < 3; i++)
				display_field (i);
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.rep_type[field],type);
		if (local_rec.rep_type[field][0] == 'Y')
			strcpy (local_rec.rep_type[field],"Y(es");
		else
			strcpy (local_rec.rep_type[field],"N(o ");

		for (i = 0; i < 3; i++)
		{
			if (strcmp (local_rec.rep_type[i],"Y(es") == 0)
			{
				sprintf (rep_type,"%d",i + 1);
				found_yes++;
			}
		}

		if (found_yes == 0)
		{
			strcpy (rep_type,"1");
			strcpy (local_rec.rep_type[0],"Y(es");
		}

		for (i = 0; i < 3; i++)
			display_field (i);
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lp_no = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lp_no))
		{
			errmess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o ");
		return (EXIT_SUCCESS);
	}
		
	return (EXIT_SUCCESS);
}

void
run_prog (
 char *prog_name)
{
	rset_tty ();

	sprintf (local_rec.lpno,"%2d",local_rec.lp_no);

	clear ();
	print_at (0,0,ML (mlStdMess035));
	CloseDB (); FinishProgram ();;
	fflush (stdout);
	
	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lpno,
				rep_type,
				rep_desc[ (atoi(rep_type) - 1) ], (char *)0);
	}

    else
	/*====================================
	| Test for forground or background . |
	====================================*/
	if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.lpno,
				rep_type, (char *)0);
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.lpno,
			rep_type, (char *)0);
	}
	prog_exit = 1;
}

void
set_dflt (
 void)
{
	strcpy (rep_type,"1");
	strcpy (local_rec.rep_type[0],"Y(es");
	strcpy (local_rec.rep_type[1],"N(o ");
	strcpy (local_rec.rep_type[2],"N(o ");
	local_rec.lp_no = 1;
	strcpy (local_rec.back,"N(o ");
	strcpy (local_rec.onight,"N(o ");
}

void
start_report (
 int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.tdbt_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	if (report_type == 2 || report_type == 3)
		fprintf (pp,".12\n");
	else
		fprintf (pp,".11\n");
	fprintf (pp,".PI12\n");
	fprintf (pp,".L134\n");
	if (report_type != 3)
		fprintf (pp,".E%s\n",rep_desc[report_type - 1]);
	else		
		fprintf (pp,".E%s\n", "SERVICE PERSON RATES MASTER FILE REPORT.");
	fprintf (pp,".ECOMPANY %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf (pp,".EBRANCH %s - %s\n",comm_rec.test_no,clip(comm_rec.test_name));
	if(report_type != 1)
		fprintf (pp,".EDEPARTMENT %s - %s\n",comm_rec.tdp_no,clip(comm_rec.tdp_name));
	fprintf (pp,".EAS AT : %-24.24s\n",SystemTime());
	fprintf (pp,".B1\n");

	switch (report_type)	
	{
	case 1:
        	fprintf (pp, ".R                          ");
        	fprintf (pp, "========");
        	fprintf (pp, "==========================");
        	fprintf (pp, "==========\n");
    
        	fprintf (pp, "                          ");
        	fprintf (pp, "|======|");
        	fprintf (pp, "=========================|");
        	fprintf (pp, "=========|\n");
  
        	fprintf (pp, "                          ");
        	fprintf (pp, "| CODE |");
        	fprintf (pp, "  DESCRIPTION            |");
        	fprintf (pp, "    RATE |\n");
		
        	fprintf (pp, "                          ");
        	fprintf (pp, "|------|");
        	fprintf (pp, "-------------------------|");
        	fprintf (pp, "---------|\n");
		break;

	case 2:
        	fprintf (pp, ".R                          ");
        	fprintf (pp, "========");
        	fprintf (pp, "==========================");
        	fprintf (pp, "============");
        	fprintf (pp, "============");
        	fprintf (pp, "============\n");
    
        	fprintf (pp, "                          ");
        	fprintf (pp, "|======|");
        	fprintf (pp, "=========================|");
        	fprintf (pp, "===========|");
        	fprintf (pp, "===========|");
        	fprintf (pp, "===========|\n");
  
        	fprintf (pp, "                          ");
        	fprintf (pp, "| CODE |");
        	fprintf (pp, "  DESCRIPTION            |");
        	fprintf (pp, "   COST/HR |");
        	fprintf (pp, "  OHEAD/HR |");
        	fprintf (pp, " PROFIT/HR |\n");
		
        	fprintf (pp, "                          ");
        	fprintf (pp, "|------|");
        	fprintf (pp, "-------------------------|");
        	fprintf (pp, "-----------|");
        	fprintf (pp, "-----------|");
        	fprintf (pp, "-----------|\n");
		break;

	case 3:
        	fprintf (pp, ".R                  ");
        	fprintf (pp, "===============");
        	fprintf (pp, "==========================");
        	fprintf (pp, "============");
        	fprintf (pp, "==========================");
        	fprintf (pp, "============\n");
    
        	fprintf (pp, "                  ");
        	fprintf (pp, "|=============|");
        	fprintf (pp, "=========================|");
        	fprintf (pp, "===========|");
        	fprintf (pp, "=========================|");
        	fprintf (pp, "===========|\n");
  
        	fprintf (pp, "                  ");
        	fprintf (pp, "|  EMPLOYEE   |");
        	fprintf (pp, "  NAME                   |");
        	fprintf (pp, " RATE CODE |");
        	fprintf (pp, "  DESCRIPTION            |");
        	fprintf (pp, "   RATE    |\n");
	
        	fprintf (pp, "                  ");
        	fprintf (pp, "|-------------|");
        	fprintf (pp, "-------------------------|");
        	fprintf (pp, "-----------|");
        	fprintf (pp, "-------------------------|");
        	fprintf (pp, "-----------|\n");
		break;
	}
	fflush (pp);
}

void
prnt_sjvh (
 void)
{
	strcpy (sjvh_rec.vh_co_no, comm_rec.tco_no);
	strcpy (sjvh_rec.vh_est_no,comm_rec.test_no); 
	sprintf (sjvh_rec.vh_code,"%-3.3s"," "); 
	cc = find_rec ("sjvh", &sjvh_rec, GTEQ, "r");

	while(!cc && !strcmp (sjvh_rec.vh_co_no, comm_rec.tco_no) && 
				 !strcmp (sjvh_rec.vh_est_no,comm_rec.test_no))
	{
		fprintf (pp, "                          ");
		fprintf (pp,"|  %-3.3s |",sjvh_rec.vh_code);
		fprintf (pp,"%-25.25s|",sjvh_rec.vh_vehicle);
		fprintf (pp,"%8.2f |\n",sjvh_rec.vh_rate);
		fflush (pp);

		dsp_process ("Vehicle : ",sjvh_rec.vh_code);
		cc = find_rec ("sjvh", &sjvh_rec, NEXT, "r");
	}
}

void
prnt_sjlr (
 void)
{
	strcpy (sjlr_rec.lr_co_no, comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.test_no); 
	strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no); 
	sprintf (sjlr_rec.lr_code,"%-2.2s"," "); 
	cc = find_rec ("sjlr", &sjlr_rec, GTEQ, "r");
	while(!cc && !strcmp (sjlr_rec.lr_co_no, comm_rec.tco_no) && 
			     !strcmp (sjlr_rec.lr_est_no,comm_rec.test_no) && 
			     !strcmp (sjlr_rec.lr_dp_no,comm_rec.tdp_no))
	{
		fprintf (pp, "                          ");
		fprintf (pp,"|  %-3.3s |",sjlr_rec.lr_code);
		fprintf (pp,"%-25.25s|",sjlr_rec.lr_descr);
		fprintf (pp,"%10.2f |",sjlr_rec.lr_cost_hr);
		fprintf (pp,"%10.2f |",sjlr_rec.lr_ovhd_hr);
		fprintf (pp,"%10.2f |\n",sjlr_rec.lr_profit_hr);
		fflush  (pp);

		dsp_process ("Labour rate code : ",sjlr_rec.lr_code);
		cc = find_rec ("sjlr", &sjlr_rec, NEXT, "r");
	}
}

void 
prnt_sjsr (
 void)
{
	strcpy (sjsr_rec.sr_co_no, comm_rec.tco_no);
	strcpy (sjsr_rec.sr_est_no,comm_rec.test_no); 
	strcpy (sjsr_rec.sr_dp_no,comm_rec.tdp_no); 
	sprintf (sjsr_rec.sr_code,"%-10.10s"," ");
	cc = find_rec ("sjsr", &sjsr_rec, GTEQ, "r");

	strcpy (sjlr_rec.lr_co_no, comm_rec.tco_no);
	strcpy (sjlr_rec.lr_est_no,comm_rec.test_no); 
	strcpy (sjlr_rec.lr_dp_no,comm_rec.tdp_no); 

	while(!cc && !strcmp (sjsr_rec.sr_co_no, comm_rec.tco_no) && 
				 !strcmp (sjsr_rec.sr_est_no,comm_rec.test_no) && 
				 !strcmp (sjsr_rec.sr_dp_no,comm_rec.tdp_no))
	{
		strcpy (sjlr_rec.lr_code,sjsr_rec.sr_lb_rt_code);
		cc = find_rec ("sjlr", &sjlr_rec, COMPARISON, "r");
		if (cc)
			sys_err ("Error in cudp During (DBFIND)",cc, PNAME);
		   
		fprintf (pp, "                  ");
		fprintf (pp,"|  %-10.10s |",sjsr_rec.sr_code);
		fprintf (pp,"%-25.25s|",sjsr_rec.sr_name);
		fprintf (pp,"    %-3.3s    |",sjsr_rec.sr_lb_rt_code);
		fprintf (pp,"%-25.25s|",sjlr_rec.lr_descr);
		fprintf (pp,"%10.2f |\n",sjlr_rec.lr_cost_hr);
		fflush (pp);

		dsp_process ("Employee : ",sjsr_rec.sr_code);
		cc = find_rec ("sjsr", &sjsr_rec, NEXT, "r");
	}
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
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSjMess001),15,0,1);
		move (0,1);
		line (80);

		print_at (4,5,ML(mlSjMess002));

		box (0,3,80,8);

		move (1,9);
		line (79);

		move (0,20);
		line (80);
		print_at (21,0,ML(mlStdMess038),comm_rec.tco_no,clip(comm_rec.tco_name));
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
initML (
 void)
{
	rep_desc[0] = strdup (ML(mlSjMess058));	
	rep_desc[1] = strdup (ML(mlSjMess059));	
	rep_desc[2] = strdup (ML(mlSjMess060));
}	
