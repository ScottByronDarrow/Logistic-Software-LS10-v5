/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_reflag.c    )                                 |
|  Program Desc  : ( Post Invoices / Credit Notes as a Batch      )   |
|                  ( or Post Invoices / Credit Notes Individually )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cohr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 26/08/88         |
|---------------------------------------------------------------------|
|  Date Modified : (26/08/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (15/11/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (19/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (27/09/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (13/12/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (17/09/97)      | Modified  by  : Elizabeth D. Paid|
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (14/04/1999)    | Modified by : Ana Marie C Tario. |
|                                                                     |
|  Comments      : Use new screen generator.                          |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|                : (27/09/91) - General Clean up.                     |
|  (13/12/94)    : PSM 11700  - Allow entry of non-numeric batch nos. |
|  (17/09/97)    : SEL        - Multilingual Conversion, changed      |
|                :              printf to print_at                    |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|  (14/04/1999)  : Changed invoice prompt from 6 to 8 chars.          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_reflag.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_reflag/so_reflag.c,v 5.2 2001/08/09 09:21:45 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <signal.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>

#define	INVOICE		( type_flag[0] == 'I' )
#define	BATCH		( by_flag[0] == 'B' )
#define	INV_STR		( (INVOICE) ? "Invoice" : "Credit Note")

	int	del_key = FALSE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*============================================
	| Customer Order/Invoice/Credit Header File. |
	============================================*/
	struct dbview cohr_list[] ={
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_type"},
		{"cohr_batch_no"},
		{"cohr_stat_flag"},
	};

	int cohr_no_fields = 6;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_inv_no[9];
		char	hr_type[2];
		char	hr_batch_no[6];
		char	hr_stat_flag[2];
	} cohr_rec;

	char	find_flag[2];
	char	update_flag[2];
	char	type_flag[2];
	char	by_flag[2];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	_value[9];
	char	_label[15];
	char	_mask[10];
	char	_prmpt[21];
	char	dummy[11];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, local_rec._label, 4, 15, CHARTYPE, 
		local_rec._mask, "          ", 
		"0", "", local_rec._prmpt, " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec._value}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void show_batch (char *key_val);
void show_invoice (char *key_val);
void next_batch (char *batch_no);
void del_catch (int x);
void update (void);
void process (void);
int  heading (int scn);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char	*argv [])
{
	if (argc != 5)
	{
		print_at(0,0, mlSoMess785,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	switch (argv[1][0])
	{
	case	'I':
	case	'i':
		strcpy(by_flag,"I");
		strcpy(local_rec._label,"invoice_no");
		strcpy(local_rec._mask,"UUUUUUUU");
		break;

	case	'B':
	case	'b':
		strcpy(by_flag,"B");
		strcpy(local_rec._label,"batch_no");
		strcpy(local_rec._mask,"UUUUU");
		break;

	default:
		print_at(0,0, mlSoMess786);

		/*("<by_flag> must be I(ndividual or B(atch\007\n\r");*/
		return (EXIT_FAILURE);
	}

	switch (argv[2][0])
	{
	case	'I':
	case	'i':
		strcpy(type_flag,"I");
		strcpy(local_rec._prmpt,(BATCH) ? "Batch No" : "Invoice No");
		break;

	case	'C':
	case	'c':
		strcpy(type_flag,"C");
		strcpy(local_rec._prmpt,(BATCH) ? "Batch No" : "Credit Note");
		break;

	default:
		print_at(0,0, mlSoMess787);
		return (EXIT_FAILURE);
	}


	sprintf(find_flag,"%-1.1s",argv[3]);
	sprintf(update_flag,"%-1.1s",argv[4]);

	init_scr();
	set_masks();
	init_vars(1);

	OpenDB();


	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		set_tty();
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		search_ok = 1;
		restart = 0;

		heading(1);
		entry(1);

		if (prog_exit)
			/* shutdown_prog(); <-- old line */
            break; /* <-- new line */

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		rset_tty();

		if (BATCH)
		{
			process();
			prog_exit = 1;
		}
		else
			update();
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

	open_rec("cohr",cohr_list,cohr_no_fields,(BATCH) ? "cohr_batch_no" : "cohr_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("cohr");
	abc_dbclose("data");
}

int
spec_valid (
 int field)
{
	if ( LCHECK("batch_no") )
	{
		if (last_char == SEARCH)
			show_batch(temp_str);
		return(0);
	}

	if ( LCHECK("invoice_no") )
	{
		if (last_char == SEARCH)
		{
			show_invoice(temp_str);
			return(0);
		}

		strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
		strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
		strcpy(cohr_rec.hr_type,type_flag);
		strcpy(cohr_rec.hr_inv_no,local_rec._value);
		cc = find_rec("cohr",&cohr_rec,COMPARISON,"u");
		if (cc)
		{
			print_mess(ML(mlSoMess048));
			return(1);
		}

		if (find_flag[0] != cohr_rec.hr_stat_flag[0])
		{
			sprintf(err_str,ML(mlSoMess265),
				INV_STR,
				local_rec._value,
				cohr_rec.hr_stat_flag,
				find_flag);
			print_mess(err_str);
			abc_unlock("cohr");
			return(1);
		}

		return(0);
	}
	return(0);
}

void
show_batch (
 char *key_val)
{
	char	last_batch[6];

	work_open ();
	save_rec ("#Batch","# ");

	strcpy (last_batch,"~~~~~");

	sprintf (cohr_rec.hr_batch_no, "%-5.5s", key_val);
	cc = find_rec ("cohr", &cohr_rec, GTEQ, "r");

	while (!cc && !strncmp(cohr_rec.hr_batch_no,key_val,strlen(key_val)))
	{
		if ( !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
			cohr_rec.hr_type[0] == type_flag[0] && 
			cohr_rec.hr_stat_flag[0] == find_flag[0])
		{
			/*---------------------------------------
			| Only Save One Copy of Each Batch No	|
			---------------------------------------*/
			if (strcmp(last_batch,cohr_rec.hr_batch_no))
			{
				cc = save_rec(cohr_rec.hr_batch_no," ");
				if (cc)
					break;
			}
			strcpy(last_batch,cohr_rec.hr_batch_no);
			next_batch(cohr_rec.hr_batch_no);
			cc = find_rec("cohr",&cohr_rec,GTEQ,"r");
		}
		else
			cc = find_rec("cohr",&cohr_rec,NEXT,"r");
	}
	disp_srch();
	work_close();
}

void
show_invoice (
 char *key_val)
{
	work_open();
	save_rec((INVOICE) ? "#Inv No" : "#Credit","# ");

	strcpy(cohr_rec.hr_co_no,comm_rec.tco_no);
	strcpy(cohr_rec.hr_br_no,comm_rec.test_no);
	strcpy(cohr_rec.hr_type,type_flag);
	sprintf(cohr_rec.hr_inv_no,"%-8.8s",key_val);
	cc = find_rec("cohr",&cohr_rec,GTEQ,"r");

	while (!cc && !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
		      !strcmp(cohr_rec.hr_br_no,comm_rec.test_no) && 
		      cohr_rec.hr_type[0] == type_flag[0] && 
		      !strncmp(cohr_rec.hr_inv_no,key_val,strlen(key_val)))
	{
		if (cohr_rec.hr_stat_flag[0] == find_flag[0])
		{
			cc = save_rec(cohr_rec.hr_inv_no," ");
			if (cc)
				break;
		}
		cc = find_rec("cohr",&cohr_rec,NEXT,"r");
	}
	disp_srch();
	work_close();
}

/*========================
| Get The Next Batch No. |
========================*/
void
next_batch (
 char *batch_no)
{
	int	i;
	int	j;

	for (i = 4;i >= 0;i++)
	{
		if (batch_no[i] < '~')
		{
			batch_no[i]++;
			for (j = i + 1;j < 5;j++)
				batch_no[j] = ' ';
			return;
		}
	}
}

void	
del_catch (
 int x)
{
	del_key = TRUE;
}

void
update (
 void)
{
	strcpy(cohr_rec.hr_stat_flag,update_flag);
	cc = abc_update("cohr",&cohr_rec);
	if (cc)
		sys_err("Error in cohr During (DBUPDATE)",cc,PNAME);
	abc_unlock("cohr");
}

void
process (
 void)
{
	signal(SIGINT,del_catch);

	dsp_screen("Updating batch",comm_rec.tco_no,comm_rec.tco_name);

	strcpy(cohr_rec.hr_batch_no,local_rec._value);
	cc = find_rec("cohr",&cohr_rec,GTEQ,"u");

	while (!del_key && !cc && !strcmp(cohr_rec.hr_batch_no,local_rec._value))
	{
		if ( !strcmp(cohr_rec.hr_co_no,comm_rec.tco_no) && 
		     cohr_rec.hr_type[0] == type_flag[0] && 
		     cohr_rec.hr_stat_flag[0] == find_flag[0])
		{
			sprintf(err_str,"%s : ",INV_STR);
			dsp_process(err_str,cohr_rec.hr_inv_no);
			strcpy(cohr_rec.hr_stat_flag,update_flag);
			cc = abc_update("cohr",&cohr_rec);
			if (cc)
				sys_err("Error in cohr During (DBUPDATE)",cc,PNAME);
		}
		abc_unlock("cohr");
		cc = find_rec("cohr",&cohr_rec,NEXT,"u");
	}
	signal(SIGINT,SIG_DFL);
	abc_unlock("cohr");
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

		if (BATCH)
			rv_pr(ML(mlSoMess260),(80 - strlen(ML(mlSoMess260))) / 2,0,1);
		else
			rv_pr(ML(mlSoMess263),(80 - strlen(ML(mlSoMess263))) / 2,0,1);

		move(0,1);
		line(80);

		box(0,3,80,1);

		move(0,20);
		line(80);

		print_at(21,0,ML(mlStdMess038),
					comm_rec.tco_no,comm_rec.tco_name);

		print_at(22,0,ML(mlStdMess039),
					comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
