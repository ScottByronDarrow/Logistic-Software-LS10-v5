/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_cdrg_inp.c & sk_cdrg_prn                      |
|  Program Desc  : ( Controlled Drugs input and Report.              )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, sadf,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/05/92         |
|---------------------------------------------------------------------|
|  Date Modified : (19/06/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (28/05/96)      | Modified  by  : Jiggs A Veloz.   |
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      :                                                    |
|   (16/06/93)   : Updated to changes after user testing S/C PSM-9190 |
|   (28/05/96)   : Updated to fix problems in DateToString. 				  |
|   (04/09/97)   : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: cdrg_inp.c,v $
| Revision 5.3  2002/07/17 09:57:52  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:18:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:14  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:52  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:29  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:30:38  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/19 05:12:45  scott
| Updated to replace get_mend and get_mbeg
|
| Revision 1.11  1999/11/11 05:59:32  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.10  1999/11/03 07:31:53  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.9  1999/10/20 01:38:56  nz
| Updated for remainder of old routines.
|
| Revision 1.8  1999/10/12 21:20:30  scott
| Updated by Gerry from ansi project.
|
| Revision 1.7  1999/10/08 05:32:15  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:19:49  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cdrg_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cdrg_inp/cdrg_inp.c,v 5.3 2002/07/17 09:57:52 scott Exp $";

#define	MOD 	5
#define	MAX_CAT	99

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include    <get_lpno.h>

#define	DRG_INPT	( !strcmp(sptr, "sk_cdrg_inp") )
#define	DRG_PRNT	( !strcmp(sptr, "sk_cdrg_prn") )

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview	esmr_list [] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_stat_flag"}
	};

	int	esmr_no_fields = 4;

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
		char	stat_flag [2];
	} esmr_rec;

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
		char	cm_class_type[4];
	} cumr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_description"},
		{"inmr_hhbr_hash"},
		{"inmr_category"},
		{"inmr_std_uom"},
		{"inmr_pack_size"},
	};

	int	inmr_no_fields = 7;

	struct	{
		char	mr_co_no[3];
		char	mr_item_no[17];
		char	mr_description[41];
		long	mr_hhbr_hash;
		char	mr_category[12];
		long	mr_std_uom;
		char	mr_pack_size[6];
	} inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list[] ={
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"},
	};

	int	inum_no_fields = 5;

	struct	{
		char	um_uom_group[21];
		long	um_hhum_hash;
		char	um_uom[5];
		char	um_desc[41];
		float	um_cnv_fct;
	} inum_rec;

	/*=============================================
	| Customer P-slip/ Invoice/Credit Header File |
	=============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_dl_add1"},
		{"cohr_dl_add2"},
		{"cohr_dl_add3"},
		{"cohr_stat_flag"},
	};

	int	cohr_no_fields = 12;

	struct	{
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_dp_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhco_hash;
		long	hr_date_raised;
		char	hr_del[3][41];
		char	hr_stat_flag[2];
	} cohr_rec;

	/*===========================================
	| Customer Order/Invoice/Credit Detail File |
	===========================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_crd_type"},
		{"coln_q_order"},
		{"coln_item_desc"},
	};

	int	coln_no_fields = 6;

	struct	{
		long	ln_hhco_hash;
		int	ln_line_no;
		long	ln_hhbr_hash;
		char	ln_crd_type[2];
		float	ln_q_order;
		char	ln_item_desc[41];
	} coln_rec;

	/*===============================
	| External Category File Record |
	===============================*/
	struct dbview	excf_list [] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cont_drugs"},
		{"excf_stat_flag"}
	};

	int	excf_no_fields = 4;

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		char	cont_drugs [2];
		char	stat_flag [2];
	} excf_rec;

	FILE	*fout,
		*fsort;

	char	data_str[300],
		prev_item[17],
		curr_item[17],
		category[ MAX_CAT ][12],
		city_name[21];

	int	data_fnd = FALSE,
		no_cat;

	int	curr_mnth;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	long	s_date;
	long	e_date;
	char	back[5];
	char	onight[5];
	int	lpno;
	char	posted[16];
} local_rec;

	char *month_nm[12]; /* = {
		"JANUARY",
		"FEBRUARY",
		"MARCH",
		"APRIL",
		"MAY",
		"JUNE",
		"JULY",
		"AUGUST",
		"SEPTEMBER",
		"OCTOBER",
		"NOVEMBER",
		"DECEMBER",
	};*/

static	struct var vars[] =
{
	{1, LIN, "s_date",	 4, 17, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "      ", "Start Date :", " Default Is 1st Of Month ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.s_date},
	{1, LIN, "e_date",	 5, 17, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "~~~~~~", "End Date   :", " Default Is Todays Date ",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.e_date},
	{1, LIN, "lpno",	7, 17, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	8, 17, CHARTYPE,
		"UUUU", "          ",
		" ", "N(o", "Background", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	8, 60, CHARTYPE,
		"UUUU", "          ",
		" ", "N(o", "Overnight", " ",
		 NO, NO,  JUSTLEFT, "YN", "", local_rec.onight},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void run_prog (char *prog_name, char *prog_desc);
int  spec_valid (int field);
int  heading (int scn);
void proc_file (void);
int  load_cat (void);
void proc_cohr (char *type);
void proc_coln (void);
int  valid_cat (void);
void get_city (void);
void proc_sort (void);
void draw_line (float itot);
void head_output (void);
char *month_name (int n);
void InitML (void);
void initMonthArray (void);
void deleteMonthArray (void);
void setMonthName (int iMonth, char* szMonthName);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

	sptr = strrchr(argv[0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv[0];

	if ( argc < 3 )
	{
		print_at(0,0,mlSkMess506);
		print_at(1,0,mlSkMess507);

		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB();
			
	InitML();

	if ( !load_cat() )
	{
		print_at(0,0,ML(mlSkMess474));
		sleep ( 3 );
		shutdown_prog();
        return (EXIT_FAILURE);  /* not sure about this one... */
	}
	if (DRG_INPT)
	{
		sprintf (local_rec.posted, "%-15.15s", argv[2]);
		clip(local_rec.posted);

		SETUP_SCR(vars);

		/*----------------------------
		| Setup required parameters. |
		----------------------------*/
		init_scr();
		set_tty();
		set_masks();
		init_vars(1);
              	 
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
	
			/*------------------------------
	        	| Edit screen 1 linear input . |	
			------------------------------*/
			heading(1);
			entry(1);
			if (restart || prog_exit)
				continue;

			/*------------------------------
	        	| Edit screen 1 linear input . |	
			------------------------------*/
			heading(1);
			scn_display(1);
			edit(1);
			if (restart)
				continue;

			run_prog("sk_cdrg_prn", argv[1]);
			/*shutdown_prog();*/
            prog_exit = 1; /* seems more natural */
		}	
		/*shutdown_prog(); this is not needed! program flow
            cascades all the way down anyway */
	}
	else
	{
		local_rec.lpno = atoi(argv[1]);
		local_rec.s_date = StringToDate(argv[2]);
		if ( local_rec.s_date == -1L )
			local_rec.s_date = MonthStart( comm_rec.tdbt_date );

		local_rec.e_date = StringToDate(argv[3]);
		if ( local_rec.e_date == -1L )
			local_rec.e_date = MonthEnd( comm_rec.tdbt_date );

		sprintf (local_rec.posted, "%-15.15s", argv[4]);
		clip(local_rec.posted);

		proc_file();
	
		/*---------------------------
		| Process data in sort file |
		---------------------------*/
		if (data_fnd)
		{
			head_output();
			proc_sort();
	
			fprintf(fout,".EOF\n");
			pclose(fout);
		}
	}

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
    deleteMonthArray ();
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
	open_rec( "coln", coln_list, coln_no_fields, "coln_id_no" );
	open_rec( "cohr", cohr_list, cohr_no_fields, "cohr_id_no5" );
	open_rec( "cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash" );
	open_rec( "inmr", inmr_list, inmr_no_fields, "inmr_hhbr_hash" );
	open_rec( "inum", inum_list, inum_no_fields, "inum_hhum_hash" );
	open_rec( "excf", excf_list, excf_no_fields, "excf_id_no" );
	open_rec( "esmr", esmr_list, esmr_no_fields, "esmr_id_no" );
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose( "coln" );
	abc_fclose( "cohr" );
	abc_fclose( "inmr" );
	abc_fclose( "inum" );
	abc_fclose( "cumr" );
	abc_fclose( "excf" );
	abc_fclose( "esmr" );
	abc_dbclose( "data" );
}

void
run_prog (
 char *prog_name, 
 char *prog_desc)
{
	char	lp_str[3];
	char	tmp_date[2][11];

	CloseDB (); 
	FinishProgram ();

	strcpy (tmp_date[0], DateToString(local_rec.s_date));
	strcpy (tmp_date[1], DateToString(local_rec.e_date));

	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onight[0] == 'Y') 
	{
		sprintf(lp_str,"%2d",local_rec.lpno);
		arg[0] = "ONIGHT";
		arg[1] = prog_name;
		arg[2] = lp_str;
		arg[3] = tmp_date[0];
		arg[4] = tmp_date[1];
		arg[5] = local_rec.posted;
		arg[6] = prog_desc;

		if (fork() == 0)
        {
			execvp(arg[0], arg);
            return;
        }
		else
            prog_exit = 1;
			/*exit(0);*/
	}

	sprintf(lp_str,"%2d",local_rec.lpno);
	arg[0] = prog_name;
	arg[1] = lp_str;
	arg[2] = tmp_date[0];
	arg[3] = tmp_date[1];
	arg[4] = local_rec.posted;

	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execvp(arg[0], arg);
		else
            prog_exit = 1;
			/*exit(0);*/
	}
	else 
	{
		execvp(arg[0], arg);
	}
}

int
spec_valid (
 int field)
{
	if (LCHECK("s_date")) 
	{
		if (dflt_used)
			local_rec.s_date = MonthStart (TodaysDate ());

		return(0);
	}

	if (LCHECK("e_date")) 
	{
		if (dflt_used)
			local_rec.e_date = local_rec.lsystemDate;

		return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		return(0);
	}

	if (LCHECK("back"))
	{
		strcpy(local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o");
		display_field(field);
		return(0);
	}

	if (LCHECK("onight"))
	{
		strcpy(local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o");
		display_field(field);
		return(0);
	}

	return(0);
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

		rv_pr(ML(mlSkMess475),(80 - strlen(ML(mlSkMess475)) + 2) / 2,0,1);
		
		move(0,1);
		line(80);

		if (scn == 1)
		{
			box(0,3,80,5);

			move(1,6);
			line(79);
		}

		move(0,20);
		line(79);
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0,err_str,comm_rec.tco_no,comm_rec.tco_name);
		move(0,22);
		line(79);
		/* Reset this variable for new screen NOT page */
		line_cnt = 0; 
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}

/*==========================
| Main processing routine. |
==========================*/
void
proc_file (
 void)
{
	dsp_screen("Controlled Drugs Report",
		comm_rec.tco_no,
		comm_rec.tco_name);
 	 
	fsort = sort_open("sk_cdrg_prn");

	strcpy( esmr_rec.co_no, comm_rec.tco_no );
	strcpy( esmr_rec.est_no, "  ");
	cc = find_rec( "esmr", &esmr_rec, GTEQ, "r" );
	while ( !cc && !strcmp( esmr_rec.co_no, comm_rec.tco_no ) )
	{
		proc_cohr( "I" );
		proc_cohr( "C" );
		cc = find_rec( "esmr", &esmr_rec, NEXT, "r" );
	}
}

/*=====================
| Load category file. |
=====================*/
int
load_cat (
 void)
{
	no_cat = 0;

	strcpy( excf_rec.co_no, comm_rec.tco_no );
	strcpy( excf_rec.cat_no, "           " );
	cc = find_rec( "excf", &excf_rec, GTEQ, "r" );
	while ( !cc && !strcmp( excf_rec.co_no, comm_rec.tco_no ) )
	{
	    if ( excf_rec.cont_drugs[0] == 'Y' && no_cat < MAX_CAT )
		sprintf( category[ no_cat++ ], "%-11.11s", excf_rec.cat_no);

	    cc = find_rec( "excf", &excf_rec, NEXT, "r" );
	}
	return( no_cat );
}

/*===================
| Process invoices. |
===================*/
void
proc_cohr (
 char *type)
{
	strcpy( cohr_rec.hr_co_no, esmr_rec.co_no );
	strcpy( cohr_rec.hr_br_no, esmr_rec.est_no );
	sprintf( cohr_rec.hr_type, "%-1.1s", type );
	cohr_rec.hr_date_raised = local_rec.s_date; 
	strcpy( cohr_rec.hr_dp_no, "  ");
	strcpy( cohr_rec.hr_inv_no, "        ");
	cc = find_rec("cohr", &cohr_rec, GTEQ, "r" );
	while ( !cc && !strcmp( cohr_rec.hr_co_no, esmr_rec.co_no ) &&
	               !strcmp( cohr_rec.hr_br_no, esmr_rec.est_no ) &&
	               cohr_rec.hr_type[0] == type[0] )
	{
		if ( cohr_rec.hr_date_raised > local_rec.e_date )
		{
			break;
		}
		if ( !strchr( local_rec.posted, cohr_rec.hr_stat_flag[0]) )
		{
			cc = find_rec("cohr", &cohr_rec, NEXT, "r" );
			continue;
		}

		cc = find_hash("cumr", &cumr_rec, COMPARISON, "r", 
			       cohr_rec.hr_hhcu_hash);
		if (cc)
		{
			cc = find_rec("cohr", &cohr_rec, NEXT, "r" );
			continue;
		}

		proc_coln();

		cc = find_rec("cohr", &cohr_rec, NEXT, "r" );
	}
}

/*========================
| Process invoice lines. |
========================*/
void
proc_coln (
 void)
{
	coln_rec.ln_hhco_hash = cohr_rec.hr_hhco_hash;
	coln_rec.ln_line_no = 0;
	cc = find_rec("coln", &coln_rec, GTEQ, "r");
	while ( !cc && coln_rec.ln_hhco_hash == cohr_rec.hr_hhco_hash )
	{
		/*---------------------------------------------------
		| If cohr is type 'C' for Credit and line is not a  |
                | return of goods then ignore.                      |
		---------------------------------------------------*/
		if ( cohr_rec.hr_type[0] == 'C' && 
		     coln_rec.ln_crd_type[0] != 'R' )
		{
			cc = find_rec("coln", &coln_rec, NEXT, "r");
			continue;
		}

		cc = find_hash("inmr", &inmr_rec, EQUAL, "r", 
			       			coln_rec.ln_hhbr_hash);
		if ( cc )
		{
			cc = find_rec("coln", &coln_rec, NEXT, "r");
			continue;
		}

		if ( !valid_cat() )
		{
			cc = find_rec("coln", &coln_rec, NEXT, "r");
			continue;
		}

		/* Should have used pack size.  
		cc = find_hash( "inum", &inum_rec, EQUAL, "r", 
							inmr_rec.mr_std_uom);
		if (cc)
			sprintf(inum_rec.um_uom, "%-4.4s", " ");
		*/

		dsp_process("Customer :",cumr_rec.cm_dbt_no);
	
		get_city();

		sprintf( data_str, "%-16.16s%08ld%-9.9s%-40.40s%-20.20s%-40.40s%-5.5s%9.2f%-8.8s\n",
				inmr_rec.mr_item_no,         /* 0   */
				cohr_rec.hr_date_raised,     /* 16  */
				cumr_rec.cm_acronym,         /* 24  */
				cumr_rec.cm_name,            /* 33  */
				city_name,                   /* 73  */
				inmr_rec.mr_description,     /* 93  */
				inmr_rec.mr_pack_size,	     /* 133 */
				(cohr_rec.hr_type[0] == 'I') /* 138 */
					? coln_rec.ln_q_order 
					: coln_rec.ln_q_order * -1,
				cohr_rec.hr_inv_no );        /* 147 */
				
		sort_save(fsort, data_str);

		data_fnd = TRUE;

		cc = find_rec("coln", &coln_rec, NEXT, "r");
	}
}

/*=====================
| Validate categories |
=====================*/
int
valid_cat (
 void)
{
	int	i;

	for (i = 0; i < no_cat; i++)
		if (!strcmp(inmr_rec.mr_category, category[ i ]))
			return(TRUE);
	
	return(FALSE);
}

void
get_city (
 void)
{
	int	i;

	sprintf(city_name, "%-20.20s", " ");
	for (i = 2; i >= 0; i--)
	{
		if ( strlen( clip( cohr_rec.hr_del[ i ] ) ) != 0)
		{
			sprintf(city_name, "%-20.20s", cohr_rec.hr_del[i]);
			break;
		}
	}
}

void
proc_sort (
 void)
{
	char	*sptr;
	char	date_supp[11];
	char	invoice_no[9];
	char	item_desc[41];
	char	month_nm[41];
	char	uom[5];
	float	quantity;
	int	first_time = TRUE;
	float	item_total = 0;

	dsp_screen("Printing Controlled Drugs Report",
		comm_rec.tco_no,
		comm_rec.tco_name);

	fsort = sort_sort(fsort,"sk_cdrg_prn");
	sptr = sort_read(fsort);

	while (sptr != (char *)0)
	{
		sprintf(curr_item, "%-16.16s", 	sptr);
		sprintf(month_nm,      "%-40.40s", 	sptr + 33);
		strcpy (date_supp, 	DateToString(atol(sptr + 16)));
		sprintf(invoice_no,"%-8.8s", 	sptr + 147);
		sprintf(city_name, "%-20.20s", 	sptr + 73);
		sprintf(item_desc, "%-40.40s", 	sptr + 93);
		quantity = 			(float) (atof(sptr + 138));
		sprintf(uom,       "%-4.4s", 	sptr + 133);
	

		dsp_process("Invoice No :",invoice_no);

		if (first_time || strcmp(curr_item, prev_item))
		{
			if (!first_time)
			{
				draw_line( item_total );
				item_total = 0;
			}

			strcpy(prev_item,curr_item);
			first_time = FALSE;
		}
		item_total += quantity;

		fprintf (fout, "|%-10.10s", date_supp );
		fprintf (fout, "|%-40.40s", month_nm );
		fprintf (fout, "|%-20.20s", city_name );
		fprintf (fout, "|%-16.16s", curr_item );
		fprintf (fout, "|%-40.40s", item_desc );
		fprintf (fout, "| %-5.5s", uom);
		fprintf (fout, "| %11.2f |\n", quantity);

		sptr = sort_read(fsort);
	}

	sort_delete(fsort,"sk_cdrg_prn");
}

void
draw_line (
 float itot)
{
	fprintf( fout, "|          ");
	fprintf( fout, "|                                        ");
	fprintf( fout, "|                    ");
	fprintf( fout, "|                ");
	fprintf( fout, "| *** ITEM NUMBER TOTAL ***              ");
	fprintf( fout, "|      ");
	fprintf( fout, "| %11.2f |\n" , itot);
	fprintf( fout, "|----------");
	fprintf( fout, "|----------------------------------------");
	fprintf( fout, "|--------------------");
	fprintf( fout, "|----------------");
	fprintf( fout, "|----------------------------------------");
	fprintf( fout, "|------");
	fprintf( fout, "|-------------|\n");
}

void
head_output (
 void)
{
	DateToDMY (local_rec.s_date, NULL, &curr_mnth, NULL);

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);

	fprintf( fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf( fout, ".LP%d\n",local_rec.lpno);
	fprintf( fout, ".PI12\n");
	fprintf( fout, ".13\n");
	fprintf( fout, ".L158\n");

	fprintf( fout, ".CDEPARTMENT OF HEALTH\n");
	fprintf( fout, ".B1\n");
	fprintf( fout, ".ERETURN OF CONTROLLED DRUGS SUPPLIED BY WHOLESALE DEALER\n");
	fprintf( fout, ".B1\n");
	fprintf( fout, ".C (FORM NO.2, MISUSE OF DRUGS REGULATIONS) \n");
	fprintf( fout, ".B1\n");

	fprintf( fout, ".C FOR THE MONTH OF %s %s BY %s\n",
					month_name(curr_mnth - 1), SystemTime(), comm_rec.tco_name);
	fprintf( fout, ".B1\n");

	fprintf( fout, ".R===========");
	fprintf( fout, "=========================================");
	fprintf( fout, "=====================");
	fprintf( fout, "=================");
	fprintf( fout, "=========================================");
	fprintf( fout, "=======");
	fprintf( fout, "===============\n");
	
	fprintf( fout, "===========");
	fprintf( fout, "=========================================");
	fprintf( fout, "=====================");
	fprintf( fout, "=================");
	fprintf( fout, "=========================================");
	fprintf( fout, "=======");
	fprintf( fout, "===============\n");
	
	fprintf( fout, "|   DATE   ");
	fprintf( fout, "|           TO WHOM SUPPLIED             ");
	fprintf( fout, "|     ADDRESS        ");
	fprintf( fout, "|  ITEM NUMBER.  ");
	fprintf( fout, "|             ITEM DESCRIPTION           ");
	fprintf( fout, "| FORM ");
	fprintf( fout, "|  QUANTITY.  |\n");
	
	fprintf( fout, "|----------");
	fprintf( fout, "|----------------------------------------");
	fprintf( fout, "|--------------------");
	fprintf( fout, "|----------------");
	fprintf( fout, "|----------------------------------------");
	fprintf( fout, "|------");
	fprintf( fout, "|-------------|\n");
}

char *
month_name (
 int n)
{
	return((n >= 0 && n <= 11) ? month_nm[n] : month_nm[12]);
}

void
InitML (
 void)
{
	int		i;

    initMonthArray ();

	for (i = 0; i < 12; i++)
        setMonthName (i, ML (month_nm [i]));
}

void 
initMonthArray (
 void)
{
    int iPos;

    for (iPos = 0; iPos < 12; iPos++)
        month_nm [iPos] = NULL;

    setMonthName ( 0, "January");
    setMonthName ( 1, "February");
    setMonthName ( 2, "March");
    setMonthName ( 3, "April");
    setMonthName ( 4, "May");
    setMonthName ( 5, "June");
    setMonthName ( 6, "July");
    setMonthName ( 7, "August");
    setMonthName ( 8, "September");
    setMonthName ( 9, "October");
    setMonthName (10, "November");
    setMonthName (11, "December");
}

void 
deleteMonthArray (
 void)
{
    int iPos;

    for (iPos = 0; iPos < 12; iPos++)
    {
        if (month_nm [iPos] != NULL)
        {
            free (month_nm [iPos]);
            month_nm [iPos] = NULL;
        }
    }
}

void 
setMonthName (
 int iMonth, 
 char* szMonthName)
{
    if ((iMonth >=0) && (iMonth < 12))
    {
        if (month_nm [iMonth] != NULL)
        {
            free (month_nm [iMonth]);
            month_nm [iMonth] = NULL;
        }
        month_nm [iMonth] = (char*) malloc (strlen (szMonthName) + 1);
        strcpy (month_nm [iMonth], szMonthName);
    }
}

