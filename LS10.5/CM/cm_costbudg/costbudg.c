/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_costbudg.c    )                               |
|  Program Desc  : ( Contract Management Costhead / Budget Setup  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmcm, cmcb, inum, cmhr, cmcd,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cmcb, cmjd,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (24/02/93)      | Author       : Simon Dubey.      |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|                                                                     |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|                :                                                    |
|                                                                     |
| $Log: costbudg.c,v $
| Revision 5.3  2002/07/18 06:12:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 08:57:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:22  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:32:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/17 06:39:07  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/08 04:35:37  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.7  1999/10/01 07:48:19  scott
| Updated for standard function calls.
|
| Revision 1.6  1999/09/29 10:10:14  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 04:40:08  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.4  1999/09/16 04:44:41  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/14 07:34:10  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: costbudg.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_costbudg/costbudg.c,v 5.3 2002/07/18 06:12:21 scott Exp $";

#include	<pslscr.h>	
#include	<ml_cm_mess.h>	
#include	<ml_std_mess.h>	

#define	LCL_TAB_ROW 3
#define	LCL_TAB_COL 14
#define TAB_SIZE 106
#define TAB_1   29
#define TAB_2   45
#define TAB_3   59
#define TAB_4   81
#define TAB_5   102

#define	COMPANY	2
#define	BRANCH	1
#define	USER	0

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};
	
	int	comm_no_fields = 5;
	
	struct
	{
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*===========================================+
	 | Contract Management Costheads Master File |
	 +===========================================*/
#define	CMCM_NO_FIELDS	5

	struct dbview	cmcm_list [CMCM_NO_FIELDS] =
	{
		{"cmcm_co_no"},
		{"cmcm_ch_code"},
		{"cmcm_desc"},
		{"cmcm_hhcm_hash"},
		{"cmcm_hhum_hash"},
	};

	struct tag_cmcmRecord
	{
		char	co_no [3];
		char	ch_code [5];
		char	desc [41];
		long	hhcm_hash;
		long	hhum_hash;
	} cmcm_rec, cmcm2_rec;

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

	/*===============================================+
	 | Contract Management Contract Description File |
	 +===============================================*/
#define	CMCD_NO_FIELDS	4

	struct dbview	cmcd_list [CMCD_NO_FIELDS] =
	{
		{"cmcd_hhhr_hash"},
		{"cmcd_line_no"},
		{"cmcd_text"},
		{"cmcd_stat_flag"}
	};

	struct tag_cmcdRecord
	{
		long	hhhr_hash;
		int		line_no;
		char	text [71];
		char	stat_flag [2];
	}	cmcd_rec;

	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	8

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
		{"cmhr_st_date"},
		{"cmhr_due_date"},
		{"cmhr_status"},
		{"cmhr_cus_ref"},
	};

	struct tag_cmhrRecord
	{
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_cont_no[7];
		long	hr_hhhr_hash;
		long	hr_st_date;
		long	hr_due_date;
		char	hr_status[2];
		char	hr_cus_ref[21];
	} cmhr_rec;


	/*===============================================+
	 | Contract Management Costheads/Contract Budget |
	 +===============================================*/
#define	CMCB_NO_FIELDS	7

	struct dbview	cmcb_list [CMCB_NO_FIELDS] =
	{
		{"cmcb_hhhr_hash"},
		{"cmcb_hhcm_hash"},
		{"cmcb_budg_type"},
		{"cmcb_budg_cost"},
		{"cmcb_budg_qty"},
		{"cmcb_budg_value"},
		{"cmcb_dtl_lvl"}
	};

	struct tag_cmcbRecord
	{
		long	hhhr_hash;
		long	hhcm_hash;
		char	budg_type [2];
		double	budg_cost;		/* money */
		float	budg_qty;
		double	budg_value;		/* money */
		char	dtl_lvl [2];
	}	cmcb_rec;

	char	*data	= "data",
			*comm	= "comm",
			*cmcm	= "cmcm",
			*cmcm2	= "cmcm2",
			*cmcb	= "cmcb",
			*cmhr	= "cmhr",
			*cmcd	= "cmcd",
			*inum	= "inum";

	int		first_time = TRUE;
	int		by_what;

	char	cbranchNo[3];

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct
{
	char	dummy[11];
	char	last_cont[7];
	char	last_cost[5];
	char	budg_type[9];
	long	hhhr_hash;
	long	hhcm_hash;
	double	cost;
	float	qty;
	double	value;
} local_rec;

static	struct	var	vars[] =
{
	{1, TAB, "cont_no",	 15, 3, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Contract No. ", "Enter Contract Number, Default = Last Contract Number Entered, Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.hr_cont_no},
	{1, TAB, "costhd",	 	0, 5, CHARTYPE,
		"UUUU", "          ",
		" ", " ",  " Costhead Code ", "Enter Costhead Code, Default = Last Costhead Entered, FN4 = Costheads Already Setup, FN9 = All Costheads",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.ch_code},
	{1, TAB, "budg_type",	0, 2, CHARTYPE,
		"UAAAAAAA", "          ",
		" ", "F", " Budget Type ", "F)ixed V)ariable, Default = Fixed",
		 YES, NO,  JUSTLEFT, "FV", "", local_rec.budg_type},
	{1, TAB, "budg_cost",	0, 4, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Budgetted Unit Cost ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &local_rec.cost},
	{1, TAB, "budg_qty",	0, 4, FLOATTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Budgetted Quantity ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &local_rec.qty},
	{1, TAB, "budg_val",	0, 3, MONEYTYPE,
		"NNNNNNNN.NN", "          ",
		" ", " ", " Budgetted Value ", " ",
		 YES, NO,  JUSTRIGHT, "0.00", "99999999.99", (char *) &local_rec.value},
	{1, TAB, "hhhr_hash",	0, 3, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhhr_hash},
	{1, TAB, "hhcm_hash",	0, 3, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.hhcm_hash},
	{1, TAB, "cont_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcd_rec.text},
	{1, TAB, "csthd_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", "",
		 ND, NO,  JUSTLEFT, "", "", cmcm_rec.desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*--------------------------
| Main Processing Routine. |
--------------------------*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int field);
void	update			(void);
void	SrchCmcm		(char * key_val);
int		heading			(int scn);
void	SrchCmcb		(char * key_val);
void	SrchCmhr		(char * key_val);
void	Get_desc		(void);
int		check_pair		(int count);
void	add_new_cmcbs	(void);
void	tab_other		(int line_no);

/*--------------------------
| Main Processing Routine. |
--------------------------*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr = chk_env("CM_AUTO_CON");

	if ( sptr )
		by_what = atoi( sptr );
	else
		by_what = COMPANY;

	clear();

	tab_row = LCL_TAB_ROW;
	tab_col = LCL_TAB_COL;
	SETUP_SCR (vars);

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB(); 

	strcpy(cbranchNo, (by_what != COMPANY) ? comm_rec.test_no : " 0");

	swide ();

	while ( !prog_exit )
	{
		lcount[1]	= 0;
   		entry_exit 	= FALSE;
   		edit_exit 	= FALSE;
   		prog_exit 	= FALSE;
   		search_ok 	= TRUE;
   		restart 	= FALSE;
		init_ok		= TRUE;

		init_vars(1);
	
		heading(1);
		scn_display(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		FLD ("budg_cost") = YES;
		FLD ("budg_val") = YES;

		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		update();
	}
	shutdown_prog();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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
	abc_dbopen(data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	abc_alias ( cmcm2, cmcm );

	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (inum,  inum_list, inum_no_fields, "inum_hhum_hash");
	open_rec (cmcb,  cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");

	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmhr,	cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (inum);
	abc_fclose (cmcb);
	abc_fclose (cmhr);
	abc_fclose (cmcd);

	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	int i;

	if ( prog_status == ENTRY && local_rec.budg_type[0] == 'V' )
	{
		FLD ("budg_cost") 	= YES;
		FLD ("budg_qty") 	= YES;
		FLD ("budg_val") 	= NA;
	}
	else
	{
		FLD ("budg_cost") 	= NA;
		FLD ("budg_qty") 	= YES;
		FLD ("budg_val") 	= YES;
	}
		
	if ( LCHECK ("budg_type" ) )
	{
		if ( local_rec.budg_type[0] == 'F' )
			sprintf ( local_rec.budg_type, "%-8.8s", "Fixed" );
		else
			sprintf ( local_rec.budg_type, "%-8.8s", "Variable" );

		DSP_FLD ("budg_type");
		return (EXIT_SUCCESS);
	}

	if ( LCHECK ("costhd" ) )
	{
		if ( dflt_used )
		{
			if ( first_time )
				return (EXIT_FAILURE);
			
			local_rec.hhcm_hash = cmcm_rec.hhcm_hash;
			strcpy ( cmcm_rec.ch_code, local_rec.last_cost );
			cc = find_rec ( cmcm, &cmcm_rec, EQUAL, "r" );

			if ( cc )
				file_err (cc, cmcm, "DBFIND" );

			putval ( line_cnt );
			i = check_pair ( line_cnt );
			getval ( line_cnt );

			if ( i )
			{
				/*sprintf ( err_str, "\007 A Budget For %s/%s Has Already Been Entered On Line %d ", cmhr_rec.hr_cont_no, cmcm_rec.ch_code, i);
				print_mess (err_str);*/
				print_mess(ML(mlCmMess156));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

/*
			print_at ( 17, 18, "Contract No : %6.6s    %s", 
							cmhr_rec.hr_cont_no,
							cmcd_rec.text);*/
			print_at(17,18,ML(mlStdMess069), cmhr_rec.hr_cont_no,
							cmcd_rec.text);
			return (EXIT_SUCCESS);
		}

		if ( SRCH_KEY )
		{
			if ( search_key == FN9 )
			{
				SrchCmcm ( temp_str );
				return (EXIT_SUCCESS);
			}
			else
			{
				SrchCmcb ( temp_str );
				return (EXIT_SUCCESS);
			}
		}

		strcpy ( cmcm_rec.co_no, comm_rec.tco_no );
		cc = find_rec ( cmcm, &cmcm_rec, EQUAL, "r" );

		while ( cc )
		{
			/*i= prmptmsg ( "\007Costhead Not On File - Do You Wish To Add [Y/N] ? ", "YNyn", 18, 18 );*/
			i= prmptmsg ( ML(mlCmMess016), "YNyn", 18, 18 );

			if ( i == 'n' || i == 'N' )
			{
				print_at (18,18, "%-100.100s", " " );
				return (EXIT_FAILURE);
			}

			i = line_cnt;
			putval ( line_cnt );

			snorm ();
			*(arg) = "cm_costhd";
			*(arg+(1)) = (char *)0;
			shell_prog(2);
			swide ();
			heading (1);

			line_cnt = i;
			if ( prog_status == ENTRY )
				lcount[1] = line_cnt;

			scn_display (1);
			getval ( line_cnt );
			line_display ();

			cc = find_rec ( cmcm, &cmcm_rec, EQUAL, "r" );
		}

		local_rec.hhcm_hash = cmcm_rec.hhcm_hash;

		putval ( line_cnt );
		i = check_pair ( line_cnt );
		getval ( line_cnt );

		if ( i )
		{
			/*sprintf ( err_str, "\007 A Budget For %s/%s Has Already Been Entered On Line %d ", cmhr_rec.hr_cont_no, cmcm_rec.ch_code, i);
			print_mess (err_str);*/
			print_mess(ML(mlCmMess156));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy ( local_rec.last_cost, cmcm_rec.ch_code );
/*
		print_at ( 18, 18, "Costhead No : %4.4s      %s", 
						cmcm_rec.ch_code,
						cmcm_rec.desc);*/
		print_at(18,18,ML(mlStdMess070), cmcm_rec.ch_code,
						cmcm_rec.desc);
/*
		print_at ( 17, 18, "Contract No : %6.6s    %s", 
						cmhr_rec.hr_cont_no,
						cmcd_rec.text);*/
		print_at(17,18,ML(mlStdMess069), cmhr_rec.hr_cont_no,
						cmcd_rec.text);

		first_time = FALSE;
		return (EXIT_SUCCESS);
	}

	if ( LCHECK ("cont_no" ) )
	{
		if ( dflt_used )
		{
			if ( first_time )
				return (EXIT_FAILURE);
			
			local_rec.hhhr_hash = cmhr_rec.hr_hhhr_hash;
			strcpy ( cmhr_rec.hr_cont_no, local_rec.last_cont );
			Get_desc ();
			/*
			print_at ( 17, 18, "Contract No : %6.6s    %s", 
							cmhr_rec.hr_cont_no,
							cmcd_rec.text);*/
			print_at(17, 18, ML(mlStdMess069), cmhr_rec.hr_cont_no,
							cmcd_rec.text);

			return (EXIT_SUCCESS);
		}
		else
			pad_num ( cmhr_rec.hr_cont_no );

		if ( SRCH_KEY )
		{
			SrchCmhr ( temp_str );
			return (EXIT_SUCCESS);
		}

		strcpy ( cmhr_rec.hr_co_no, comm_rec.tco_no );
		strcpy ( cmhr_rec.hr_br_no, cbranchNo );
		cc = find_rec ( cmhr, &cmhr_rec, EQUAL, "r" );

		if ( cc )
		{
/*
			print_mess ( "Contract Number Not On File \007" );*/
			print_mess (ML(mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.hhhr_hash = cmhr_rec.hr_hhhr_hash;
		strcpy ( local_rec.last_cont, cmhr_rec.hr_cont_no );

		Get_desc ();
/*
		print_at ( 17, 18, "Contract No : %6.6s    %s", 
						cmhr_rec.hr_cont_no,
						cmcd_rec.text);*/
		print_at(17,18,ML(mlStdMess069), cmhr_rec.hr_cont_no,
						cmcd_rec.text);
		return (EXIT_SUCCESS);
	}
	return(0);
}

/*---------------
| Update files. |
---------------*/
void
update (void)
{
	int i;
	int exists;

	for ( i = 0; i < lcount[1]; i++ )
	{
		getval (i);
		cmcb_rec.hhhr_hash = local_rec.hhhr_hash;
		cmcb_rec.hhcm_hash = local_rec.hhcm_hash;

		exists = find_rec ( cmcb, &cmcb_rec, EQUAL, "u" );

		cmcb_rec.budg_cost = local_rec.cost;
		cmcb_rec.budg_qty = local_rec.qty;
		cmcb_rec.budg_value = local_rec.value;

		if ( local_rec.budg_type[0] == 'F' )
			cmcb_rec.budg_type[0] = 'F';
		else
			cmcb_rec.budg_type[0] = 'V';

		if ( exists )
		{
			strcpy ( cmcb_rec.dtl_lvl, "A" );
			cc = abc_add ( cmcb, &cmcb_rec );
		}
		else
			cc = abc_update ( cmcb, &cmcb_rec );

		if ( cc )
			file_err ( cc, cmcb, (!exists) ? "DBUPDATE" : "DBADD" );
	}
}

void
SrchCmcm (
 char *	key_val)
{
	work_open();
	save_rec("#Costhead", "#Costhead Description");

	strcpy(cmcm_rec.co_no, comm_rec.tco_no);
	sprintf(cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec(cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmcm_rec.co_no, comm_rec.tco_no) &&
	       !strncmp(cmcm_rec.ch_code, key_val, strlen(key_val)))
	{
		cc = save_rec(cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec(cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cmcm_rec.co_no, comm_rec.tco_no);
	sprintf(cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec(cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, cmcm, "DBFIND");
}

int
heading (
 int	scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);

	clear ();

	rv_pr ( ML(mlCmMess155), 46, 0, 1 );

	move ( 0, 21 );
	line ( 132 );

	strcpy(err_str, ML(mlStdMess038));
	print_at ( 22, 1,err_str, comm_rec.tco_no, comm_rec.tco_name );
	/*print_at ( 22, 1, "Co : %s - %s", comm_rec.tco_no, comm_rec.tco_name );*/
	move ( 0,1 );
	line ( 132 );

	/*-----
	| top |
	-----*/
	/* cnr */
	move ( LCL_TAB_COL, LCL_TAB_ROW - 1 );
	PGCHAR ( 0 );
	/* line */
	move ( LCL_TAB_COL + 1, LCL_TAB_ROW -1 );
	line ( TAB_SIZE );
	/* cnr */
	move ( LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW - 1 );
	PGCHAR ( 1 );
	/* little bits */
	move ( TAB_1, LCL_TAB_ROW - 1 );
	PGCHAR ( 8 );
	move ( TAB_2, LCL_TAB_ROW - 1 );
	PGCHAR ( 8 );
	move ( TAB_3, LCL_TAB_ROW - 1 );
	PGCHAR ( 8 );
	move ( TAB_4, LCL_TAB_ROW - 1 );
	PGCHAR ( 8 );
	move ( TAB_5, LCL_TAB_ROW - 1 );
	PGCHAR ( 8 );

	/*--------
	| bottom |
	--------*/
	/* cnr */
	move ( LCL_TAB_COL, LCL_TAB_ROW + 12 );
	PGCHAR ( 2 );
	/* cnr */
	move ( LCL_TAB_COL + TAB_SIZE, LCL_TAB_ROW + 12 );
	PGCHAR ( 3 );
	/* line */
	move ( LCL_TAB_COL + 1, LCL_TAB_ROW + 12 );
	line ( TAB_SIZE );
	/* little bits */
	move ( TAB_1, LCL_TAB_ROW + 12 );
	PGCHAR ( 9 );
	move ( TAB_2, LCL_TAB_ROW + 12 );
	PGCHAR ( 9 );
	move ( TAB_3, LCL_TAB_ROW + 12 );
	PGCHAR ( 9 );
	move ( TAB_4, LCL_TAB_ROW + 12 );
	PGCHAR ( 9 );
	move ( TAB_5, LCL_TAB_ROW + 12 );
	PGCHAR ( 9 );
	
	line_cnt = 0;
	scn_write(scn);
	return (EXIT_SUCCESS);
}

void
SrchCmcb (
 char *	key_val)
{
	int cc1;

	work_open();
	save_rec("#Costhead", "#Costhead Description");

	cmcb_rec.hhhr_hash = local_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;

	cc1 = find_rec ( cmcb, &cmcb_rec, GTEQ, "r" );
	if ( cmcb_rec.hhhr_hash != local_rec.hhhr_hash )
		cc1 = TRUE;

	if ( !cc1 )
		cc=find_hash(cmcm2,&cmcm2_rec,EQUAL,"r",cmcb_rec.hhcm_hash);

	while (!cc && !cc1 &&
	       !strcmp(cmcm2_rec.co_no, comm_rec.tco_no))
	{
		cc = save_rec(cmcm2_rec.ch_code, cmcm2_rec.desc);
		if (cc)
			break;

		cc1 = find_rec ( cmcb, &cmcb_rec, NEXT, "r" );
		if ( cmcb_rec.hhhr_hash != local_rec.hhhr_hash )
			cc1 = TRUE;

		if ( !cc1 )
			cc=find_hash(cmcm2,&cmcm2_rec,EQUAL,"r",
						cmcb_rec.hhcm_hash);
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cmcm_rec.co_no, comm_rec.tco_no);
	sprintf(cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec(cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, cmcm, "DBFIND");
}

void
SrchCmhr (
 char *	key_val)
{
	work_open ();
	save_rec ( "#Cont. No.", "#Customer Order No.");
	strcpy (cmhr_rec.hr_co_no, comm_rec.tco_no );
	strcpy (cmhr_rec.hr_br_no, cbranchNo );
	sprintf(cmhr_rec.hr_cont_no, "%-6.6s", key_val);

	cc = find_rec(cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp(cmhr_rec.hr_co_no, comm_rec.tco_no) &&
	       !strncmp(cmhr_rec.hr_cont_no, key_val,strlen(key_val)))
	{
		if ( strcmp ( cmhr_rec.hr_br_no, cbranchNo ))
			break;

		if ( cmhr_rec.hr_status[0] != 'O' && 
		     cmhr_rec.hr_status[0] != 'X' )
		{
			cc = find_rec ( cmhr, &cmhr_rec, NEXT, "r" );
			continue;
		}

		save_rec ( cmhr_rec.hr_cont_no, cmhr_rec.hr_cus_ref );
		cc = find_rec ( cmhr, &cmhr_rec, NEXT, "r" );
	}

	cc = disp_srch ();
	work_close ();
	if ( cc )
		return;

	strcpy ( cmhr_rec.hr_co_no, comm_rec.tco_no );
	strcpy ( cmhr_rec.hr_br_no, cbranchNo );
	sprintf( cmhr_rec.hr_cont_no, "%6.6s", temp_str );

	cc = find_rec ( cmhr, &cmhr_rec, COMPARISON, "r" );
	if (cc)
		file_err (cc, cmhr, "DBFIND" );
}

void
Get_desc (void)
{
	cmcd_rec.hhhr_hash = cmhr_rec.hr_hhhr_hash;
	strcpy(cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec ( cmcd, &cmcd_rec, EQUAL, "r" );
	if ( cc )
		file_err (cc, cmcd, "DBFIND" );
}

int
check_pair (
 int	count)
{
	int i;

	for ( i = 0; i < count; i++ )
	{
		getval (i);
		if ( local_rec.hhhr_hash == cmhr_rec.hr_hhhr_hash &&
		     local_rec.hhcm_hash == cmcm_rec.hhcm_hash )
			return (i+1);
	}
	return (EXIT_SUCCESS);

}

void
add_new_cmcbs (void)
{
	cmcb_rec.hhhr_hash = cmhr_rec.hr_hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	strcpy ( cmcb_rec.dtl_lvl, "A" );

	cc = abc_add ( cmcb, &cmcb_rec );
	if ( cc )
		file_err ( cc, cmcb, "DBADD" );
}

void
tab_other (
 int	line_no)
{
	if (prog_status == ENTRY)
		return;

	if (local_rec.budg_type[0] == 'V')
	{
		FLD ("budg_cost") = YES;
		FLD ("budg_qty") = YES;
		FLD ("budg_val") = NA;
	}
	else
	{
		FLD ("budg_cost") = NA;
		FLD ("budg_qty") = YES;
		FLD ("budg_val") = YES;
	}

/*
	print_at (17, 18, "Contract No : %6.6s    %s", 
						cmhr_rec.hr_cont_no,
		 				cmcd_rec.text);*/
	print_at(17,18,ML(mlStdMess069), cmhr_rec.hr_cont_no,
		 				cmcd_rec.text);

/*
	print_at (18, 18, "Costhead No : %4.4s      %s", 
						cmcm_rec.ch_code,
		  				cmcm_rec.desc);*/
	print_at(18,18,ML(mlStdMess070), cmcm_rec.ch_code,
		  				cmcm_rec.desc);

}

