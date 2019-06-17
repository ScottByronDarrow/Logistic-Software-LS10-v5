/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_invprnt.i.c )                                 |
|  Program Desc  : ( Invoice / Credit Note / Packing Slip Input   )   |
|                  ( For Print & Reprint incl Reflagging          )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cohr, cumr, coln,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 18/03/93         |
|---------------------------------------------------------------------|
|  Date Modified : (28/04/93)      | Modified  by : Simon Dubey       |
|  Date Modified : (03/09/97)      | Modified  by : Jiggs A Veloz     |
|  Date Modified : (16/10/97)      | Modified  by : Ana Marie Tario.  |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                :                                                    |
|  (28/04/93)    : EGC 8912 Reprint not working correctly.            |
|  (03/09/97)    : SEL Multilingual Conversion.                       |
|  (16/10/97)    : SEL Changed invoice length from 6 to 8. 			  |
|  (03/09/1999)  : Ported to ANSI standards.            			  |
|                :                                                    |
| $Log: invprnt.i.c,v $
| Revision 5.3  2002/07/18 06:12:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.2  2001/08/09 08:57:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:21:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:34  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:32:30  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/17 06:39:14  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.11  1999/11/08 04:35:39  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.10  1999/09/29 10:10:21  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/29 01:51:25  scott
| Updated for getchar and ansi stuff
|
| Revision 1.8  1999/09/17 04:40:11  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.7  1999/09/16 04:44:43  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/14 07:34:44  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: invprnt.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_invprnt.i/invprnt.i.c,v 5.3 2002/07/18 06:12:21 scott Exp $";

#define	MOD	1
#define	MAXWIDTH	140
#define	MAXPRINT	10000
#define	MAXLINES	100
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#define	REPRINT		(print_flag[0] == 'Y')
#define	INVOICE		0
#define	CREDIT_NOTE	1
#define	TYPE		program[type_flag]

	/*------------------------------------------
	| Stores hashes for those records printed. |
	------------------------------------------*/
	long	prnt_hash[ MAXPRINT ];

struct	{
	char	*_type;
	char	*_prt;
	char	*_prmpt;
	char	*_desc;
	char	*_env;
	char	*_prg;

} program[] = {
	{"I","I","Tax Invoice No  ","Tax Invoice","CM_CTR_INV","cm_ctr_inv"},
	{"C","C","Credit Note No  ","Credit Note","CM_CTR_INV","cm_ctr_inv"},
	{"","","","","",""},
};

int	envDbCo = 0;
int	envDbFind = 0;
int	lpno;
int	type_flag;

char	branchNo[3];
char	print_flag[2];
char	run_print[81];

	char	*comm  = "comm";
	char	*cohr  = "cohr";
	char	*coln  = "coln";
	char	*cumr  = "cumr";
	char	*data  = "data";

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*=
	|  |
	=*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhso_hash"},
		{"cohr_hhco_hash"},
		{"cohr_stat_flag"},
		{"cohr_status"},
		{"cohr_ps_print"},
		{"cohr_inv_print"},
		{"cohr_printing"},
	};

	int cohr_no_fields = 12;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_inv_no[9];
		long	hr_hhcu_hash;
		char	hr_type[2];
		long	hr_hhso_hash;
		long	hr_hhco_hash;
		char	hr_stat_flag[2];
		char	hr_status[2];
		char	hr_ps_print[2];
		char	hr_inv_print[2];
		char	hr_printing[2];
	} cohr_rec ;

	/*============================================
	| Customer Order/Invoice/Credit Detail File. |
	============================================*/
	struct dbview coln_list[] ={
		{"coln_hhco_hash"},
		{"coln_line_no"},
	};

	int coln_no_fields = 2;

	struct {
		long	ln_hhco_hash;
		int	ln_line_no;
	} coln_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
	};

	int cumr_no_fields = 2;

	struct {
		long	cm_hhcu_hash;
		char	cm_dbt_name[41];
	} cumr_rec;

FILE *	pout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	inv_no [2][9];
	char	customer [2][21];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "start_inv_no",	MAXLINES, 7, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "0", "1234567890123456", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inv_no[0]},
	{1, TAB, "start_customer",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  C u s t o m e r   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customer[0]},
	{1, TAB, "end_inv_no",	 0, 5, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "0", "1234567890123456", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.inv_no[1]},
	{1, TAB, "end_customer",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  C u s t o m e r   ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.customer[1]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<chk_vble.h>
#include	<prnt_all.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	print_every		(void);
void	process			(void);
int		spec_valid		(int);
void	print_all		(void);
int		delete_line		(void);
void	srch_cohr		(char *);
int		heading			(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 4)
	{
		/*----------------------------------------------------
		| Usage : %s <lpno> <reprint_flag [Y/N]> <type [I/C]>|
		----------------------------------------------------*/
		print_at (0, 0, ML(mlCmMess740), argv [0]); 
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	lpno = atoi (argv [1]);

	switch (argv [2][0])
	{
	case	'Y':
	case	'y':
		strcpy(print_flag,"Y");
		break;

	case	'N':
	case	'n':
		strcpy(print_flag,"N");
		break;

	default:
		print_at(0,0, ML(mlCmMess740), argv[0]); 
		return (EXIT_SUCCESS);
	}

	switch (argv[3][0])
	{
	case	'I':
	case	'i':
		type_flag = INVOICE;
		break;

	case	'C':
	case	'c':
		type_flag = CREDIT_NOTE;
		break;

	default:
		print_at(0,0, ML(mlCmMess740), argv[0]); 
		return (EXIT_SUCCESS);
	}

	vars [0].prmpt = TYPE._prmpt;
	vars [2].prmpt = TYPE._prmpt;

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	strcpy(run_print,chk_vble(TYPE._env,TYPE._prg));

	envDbCo = atoi(get_env("DB_CO"));
	envDbFind = atoi(get_env("DB_FIND"));

	if (envDbCo == 0)
		strcpy(branchNo," 0");
	else
		strcpy(branchNo,comm_rec.test_no);

	init_scr();
	set_tty(); 
	set_masks();

	if (!REPRINT)
	{
		print_every ();
		prog_exit = 1;
	}

	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		lcount[1] = 0;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			break;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			break;

		prog_exit = 1;

		if (lcount[1] != 0)
			process();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	open_rec(cohr,cohr_list,cohr_no_fields,"cohr_id_no3");
	open_rec(coln,coln_list,coln_no_fields,"coln_id_no");
	open_rec(cumr,cumr_list,cumr_no_fields,"cumr_hhcu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose(cohr);
	abc_fclose(coln);
	abc_fclose(cumr);
	abc_dbclose(data);
}

/*====================================
| Print all records not yet printed. |
====================================*/
void
print_every (
 void)
{
	int		first_time = 1;
	int		i;
	int		pr_cnt = 0;

	clear ();
	print_at (0, 0, "%s", ML(mlStdMess035));
	fflush (stdout);

	if ((pout = popen(run_print,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",run_print);
		sys_err(err_str,errno,PNAME);
	}

	/*----------------------------------
	| init all hashes before printing. |
	----------------------------------*/
	for (i = 0; i < MAXPRINT; i++)
		prnt_hash[ i ] = 0L;

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,TYPE._type);
	strcpy(cohr_rec.hr_inv_print,print_flag);
	strcpy(cohr_rec.hr_ps_print,print_flag);
	sprintf(cohr_rec.hr_inv_no,"%-8.8s"," ");
	cc = find_rec(cohr,&cohr_rec,GTEQ, "u");

	while ( !cc && 
		!strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
		!strcmp(cohr_rec.hr_br_no,comm_rec.test_no) && 
		cohr_rec.hr_type[0] == TYPE._type[0] )
	{
		if ( cohr_rec.hr_stat_flag[0] != 'C' )
		{
			abc_unlock ( cohr );
			cc = find_rec(cohr,&cohr_rec,NEXT,"u");
			continue;
		}

		if ( cohr_rec.hr_inv_print[0] != print_flag[0] )
		{
			abc_unlock ( cohr );
			cc = find_rec(cohr,&cohr_rec,NEXT,"u");
			continue;
		}

		if ( cohr_rec.hr_printing[0] == 'Y' )
		{
			abc_unlock ( cohr );
			cc = find_rec(cohr,&cohr_rec,NEXT,"u");
			continue;
		}
		else
		{
			strcpy ( cohr_rec.hr_printing, "Y" );
			cc = abc_update(cohr,&cohr_rec);
			if (cc)
				file_err (cc, "cohr", "DBUPDATE" );
			
		}

		if (first_time)
		{
			sprintf(err_str,"%srinting %ss",(REPRINT) ? "Rep" : "P",TYPE._desc);
			dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
			first_time = FALSE;
			prnt_all(lpno,-1L,"M",TYPE._prt);
		}

		coln_rec.ln_hhco_hash = cohr_rec.hr_hhco_hash;
		coln_rec.ln_line_no = 0;

		cc = find_rec(coln,&coln_rec,GTEQ,"r");

		if (cc || cohr_rec.hr_hhco_hash != coln_rec.ln_hhco_hash)
		{
			prnt_hash[ pr_cnt++ ] = cohr_rec.hr_hhco_hash;
	
			prnt_all(lpno,cohr_rec.hr_hhco_hash,"M",TYPE._prt);

			abc_unlock ( cohr );
			cc = find_rec(cohr,&cohr_rec,NEXT,"u");
			continue;
		}

		/*---------------------------------------
		| Store hashes for those items printed. |
		---------------------------------------*/
		prnt_hash[ pr_cnt++ ] = cohr_rec.hr_hhco_hash;
	
		prnt_all(lpno,cohr_rec.hr_hhco_hash,"M",TYPE._prt);

		dsp_process(TYPE._desc,cohr_rec.hr_inv_no);

		cc = find_rec(cohr,&cohr_rec,NEXT,"u");
	}

	abc_unlock(cohr);

	if (!first_time)
		prnt_all(lpno,0L,"M",TYPE._prt);

	abc_selfield ( cohr, "cohr_hhco_hash" );

	pclose(pout);
	
	for (i = 0; i < MAXPRINT; i++)
	{
		/*---------------------------
		| No more records to print. |
		---------------------------*/
		if ( prnt_hash[ i ] == 0L )
			return;

		/*======================================
		| then read record again with new index |
		======================================*/

		cc = find_hash(cohr,&cohr_rec,COMPARISON,"u",prnt_hash[i]);
		if (cc)
			file_err (cc, "cohr", "DBFIND" );

		strcpy(cohr_rec.hr_inv_print,"Y");

		strcpy ( cohr_rec.hr_printing, " " );
		cc = abc_update(cohr,&cohr_rec);
		if (cc)
		{
			/*------------------------------------------------
			| Error Occurred In Update of cohr_inv_no %s 	 |
			| - Note And Press Any Key ", cohr_rec.hr_inv_no |
			------------------------------------------------*/
			PauseForKey (0, 0,  ML(mlCmMess012), 0);
			continue;
		}
	}
}

void
process (
 void)
{
	int	first_time = 1;

	clear ();
	print_at (0, 0, "%s", ML(mlStdMess035));
	fflush (stdout);

	if ((pout = popen(run_print,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",run_print);
		sys_err(err_str,errno,PNAME);
	}

	for (line_cnt = 0;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt);

		strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
		strcpy(cohr_rec.hr_type,TYPE._type);
		strcpy(cohr_rec.hr_inv_print,print_flag);
		strcpy(cohr_rec.hr_ps_print,print_flag);
		sprintf(cohr_rec.hr_inv_no,"%-8.8s",local_rec.inv_no[0]);
		cc = find_rec(cohr,&cohr_rec,GTEQ,(REPRINT) ? "r" : "u" );

		while ( !cc && 
			!strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
			!strcmp(cohr_rec.hr_br_no,comm_rec.test_no) && 
			cohr_rec.hr_type[0] == TYPE._type[0] && 
			strcmp(cohr_rec.hr_inv_no,local_rec.inv_no[1]) <= 0)
		{
			if ( cohr_rec.hr_inv_print[0] != print_flag[0] )
			{
				if ( !REPRINT )	
					abc_unlock ( cohr );
				break;
			}

			/*------------------------------------------
			| if not a contract read the NEXT one baby |
			------------------------------------------*/
			if ( cohr_rec.hr_status[0] != 'C' )
			{
				cc = find_rec(cohr,&cohr_rec,NEXT,"r");
				continue;
			}
			coln_rec.ln_hhco_hash = cohr_rec.hr_hhco_hash;
			coln_rec.ln_line_no = 0;

			cc = find_rec(coln,&coln_rec,GTEQ, "r");

			if (cc || cohr_rec.hr_hhco_hash != coln_rec.ln_hhco_hash)
			{
				if ( !REPRINT )	
					abc_unlock ( cohr );
				cc = find_rec(cohr,&cohr_rec,NEXT,
                                              (REPRINT) ? "r" : "u" );
				continue;
			}

			if (first_time)
				prnt_all(lpno,-1L,"M",TYPE._prt);

			prnt_all(lpno,cohr_rec.hr_hhco_hash,"M",TYPE._prt);

			if (first_time)
			{
				sprintf(err_str,"%srinting %ss",(REPRINT) ? "Rep" : "P",TYPE._desc);
				dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
			}

			first_time = 0;
			dsp_process(TYPE._desc,cohr_rec.hr_inv_no);
		
			cc = find_rec(cohr,&cohr_rec,NEXT,(REPRINT) ? "r":"u");
		}
	}

	if (!first_time)
		prnt_all(lpno,0L,"M",TYPE._prt);

	pclose(pout);
}

int
spec_valid (
 int field)
{
	char	low_inv [9];
	char	high_inv [9];

	if (LCHECK("start_inv_no")) 
	{
		if (dflt_used)
			return (delete_line());

		if (!strcmp (local_rec.inv_no[0],"ALL     "))
		{
			print_all ();
			return (EXIT_SUCCESS);
		}

		strcpy(local_rec.inv_no[0],pad_num(local_rec.inv_no[0]));

		strcpy(low_inv,"        ");
		strcpy(high_inv,(prog_status == ENTRY) ? "~~~~~~~~" : local_rec.inv_no[1]);
		if ( SRCH_KEY )
		{
			srch_cohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
		strcpy(cohr_rec.hr_type,TYPE._type);
		strcpy(cohr_rec.hr_inv_print,print_flag);
		strcpy(cohr_rec.hr_ps_print,print_flag);
		strcpy(cohr_rec.hr_inv_no,local_rec.inv_no[0]);
		cc = find_rec(cohr,&cohr_rec,COMPARISON,"r");
		if (cc)
		{
			/*----------------------
			| %s No %s not on file |
			----------------------*/
			print_mess( (cohr_rec.hr_type[0] == 'I') ? ML(mlStdMess115) 
													 : ML(mlStdMess116) );
			sleep(2);
			return(1);
		}

		if ( cohr_rec.hr_inv_print[0] != 'Y' )
		{
			/*-------------------------------------------
			| Invoice No. %s Has Not Been Previously	|
			| Printed ", cohr_rec.hr_inv_no 			|
			-------------------------------------------*/
			print_mess ( ML(mlCmMess013) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ( cohr_rec.hr_status[0] != 'C' )
		{
			/*----------------------------------------------
			| Invoice No. %s Does Not Belong To A Contract |
			----------------------------------------------*/
			print_mess ( ML(mlCmMess054) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp(cohr_rec.hr_inv_no,high_inv) > 0)
		{
			/*-------------------------
			| %s No %s > %s (Maximum) |
			-------------------------*/
			print_mess( ML(mlStdMess017) );
			sleep(2);
			return(1);
		}

		cc = find_hash(cumr,&cumr_rec,COMPARISON,"r",cohr_rec.hr_hhcu_hash);
		if (cc)
		{
			/*-------------------------------------------------
			| Cannot find Customer (%ld)",cohr_rec.hr_hhcu_hash |
			-------------------------------------------------*/
			print_mess( ML(mlStdMess021) );
			sleep(2);
			return(1);
		}

		sprintf(local_rec.customer[0],"%-20.20s",cumr_rec.cm_dbt_name);
		DSP_FLD("start_customer");
		return(0);
	}

	if (LCHECK("end_inv_no")) 
	{
		if (dflt_used)
			strcpy(local_rec.inv_no[1],local_rec.inv_no[0]);

		if (!strcmp(local_rec.inv_no[1],"ALL     "))
		{
			print_all();
			return(0);
		}

		strcpy(local_rec.inv_no[1],pad_num(local_rec.inv_no[1]));

		strcpy(low_inv,local_rec.inv_no[0]);
		strcpy(high_inv,"~~~~~~~~");
		if (SRCH_KEY)
		{
			srch_cohr(temp_str);
			return(0);
		}

		strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
		strcpy(cohr_rec.hr_type,TYPE._type);
		strcpy(cohr_rec.hr_inv_print,print_flag);
		strcpy(cohr_rec.hr_ps_print,print_flag);
		strcpy(cohr_rec.hr_inv_no,local_rec.inv_no[1]);
		cc = find_rec(cohr,&cohr_rec,COMPARISON,"r");
		if (cc)
		{
			/*----------------------
			| %s No %s not on file |
			----------------------*/
			print_mess( (cohr_rec.hr_type[0] == 'I') ? ML(mlStdMess115) 
													 : ML(mlStdMess116) );
			sleep(2);
			return(1);
		}

		if (strcmp(cohr_rec.hr_inv_no,low_inv) < 0)
		{
			/*-------------------------
			| %s No %s < %s (Minimum) |
			-------------------------*/
			print_mess( ML(mlStdMess018) );
			sleep(2);
			return(1);
		}

		cc = find_hash(cumr,&cumr_rec,COMPARISON,"r",cohr_rec.hr_hhcu_hash);
		if (cc)
		{
			/*-------------------------------------------------
			| Cannot find Customer (%ld)",cohr_rec.hr_hhcu_hash |
			-------------------------------------------------*/
			print_mess( ML(mlStdMess021) );
			sleep(2);
			clear_mess ();
			return(1);
		}

		sprintf(local_rec.customer[1],"%-20.20s",cumr_rec.cm_dbt_name);
		DSP_FLD("end_customer");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
print_all (
 void)
{
	sprintf(local_rec.inv_no[0],"%-8.8s","00000000");
	sprintf(local_rec.inv_no[1],"%-8.8s","~~~~~~~~");
	sprintf(local_rec.customer[0],"%-20.20s","ALL Customers");
	sprintf(local_rec.customer[1],"%-20.20s","ALL Customers");
	DSP_FLD("start_inv_no");
	DSP_FLD("end_inv_no");
	DSP_FLD("start_customer");
	DSP_FLD("end_customer");

	entry_exit = 1;
	edit_exit = 1;

	if (prog_status == ENTRY)
		putval(line_cnt++);
}

int
delete_line (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		sprintf (err_str, "%s", ML(mlStdMess005) );
		print_mess( err_str );
		return(1);
	}

	lcount[1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[1];line_cnt++)
	{
		getval(line_cnt + 1);
		putval(line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display();
	}

	strcpy(local_rec.inv_no[0],"        ");
	strcpy(local_rec.inv_no[1],"        ");
	strcpy(local_rec.customer[0],"                    ");
	strcpy(local_rec.customer[1],"                    ");

	putval(line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display();
	
	line_cnt = i;
	getval(line_cnt);
	return (EXIT_SUCCESS);
}

/*==========================
| Search for order number. |
==========================*/
void
srch_cohr (
 char *	key_val)
{
	work_open();
	sprintf(err_str,"#%s",TYPE._prmpt);
	save_rec(err_str,"#Customer");

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,TYPE._type);
	strcpy(cohr_rec.hr_inv_print,print_flag);
	strcpy(cohr_rec.hr_ps_print,print_flag);
	sprintf(cohr_rec.hr_inv_no,"%-8.8s",key_val);

	cc = find_rec(cohr,&cohr_rec,GTEQ,"r");

	while ( !cc && !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
		!strcmp(cohr_rec.hr_br_no,comm_rec.test_no))
	{
		if ( cohr_rec.hr_inv_print[0] != print_flag[0] )
				break;

		/*-----------------------------
		| use status to show that it  |
		| was historically a Contract |
		-----------------------------*/
		if ( cohr_rec.hr_status[0] != 'C' )
		{
			cc = find_rec(cohr,&cohr_rec,NEXT,"r");
			continue;
		}

		coln_rec.ln_hhco_hash = cohr_rec.hr_hhco_hash;
		coln_rec.ln_line_no = 0;

		cc = find_rec(coln,&coln_rec,GTEQ,"r");

		if ( !cc && coln_rec.ln_hhco_hash == cohr_rec.hr_hhco_hash && 
			!strncmp(cohr_rec.hr_inv_no, key_val,strlen(key_val)))
		{
			cc = find_hash(cumr,&cumr_rec,COMPARISON,"r",cohr_rec.hr_hhcu_hash);
			if (!cc)
			{
				cc = save_rec(cohr_rec.hr_inv_no,cumr_rec.cm_dbt_name);
				if (cc)
					break;
			}
		}
		cc = find_rec(cohr,&cohr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,TYPE._type);
	strcpy(cohr_rec.hr_inv_print,print_flag);
	strcpy(cohr_rec.hr_ps_print,print_flag);
	sprintf(cohr_rec.hr_inv_no,"%-8.8s",temp_str);
	cc = find_rec(cohr,&cohr_rec,COMPARISON,"r");
	if (cc)
		file_err ( cc, "cohr", "DBFIND" );
}

int
heading (
 int	scn)
{
	char	start_str [21];
	char	end_str [21];
	if (!restart)
	{
		clear();

		switch (type_flag)
		{
			case INVOICE		:
				sprintf(err_str,(REPRINT) ? ML(mlCmMess189) : ML(mlCmMess190));
				sprintf (start_str, " %s ", ML(mlCmMess193) );
				sprintf (end_str,   " %s ", ML(mlCmMess194) );
				break;

			case CREDIT_NOTE	:
				sprintf(err_str,(REPRINT) ? ML(mlCmMess191) : ML(mlCmMess192));
				sprintf (start_str, " %s ", ML(mlCmMess195) );
				sprintf (end_str,   " %s ", ML(mlCmMess196) );
				break;
			default : 	
				break;
		}
		rv_pr(err_str,(80 - strlen(err_str)) / 2,0,1);

		move(0,1);
		line(80);

		box ( 0, 3, 77, 14 );

		rv_pr(start_str,(40 - strlen(start_str)) / 2,tab_row - 2,1);
		rv_pr(end_str,38 + (40 - strlen(end_str)) / 2,tab_row - 2,1);

		print_at(21,0, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		print_at(22,0, ML(mlStdMess039), comm_rec.test_no,comm_rec.test_short);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}
