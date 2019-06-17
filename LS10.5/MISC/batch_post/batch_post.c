/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( batch_post.c   )                                 |
|  Program Desc  : ( Post Invoices / Credit Notes as a Batch      )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cohr,     ,     ,     ,     ,     ,         |
|  Database      : (oedb)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cohr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (oedb)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 17/08/88      
|---------------------------------------------------------------------|
|  Date Modified : (17/08/88)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (26/09/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (15/10/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (03/11/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      : (26/09/91) - Check batch relates to current branch |
|                : (15/10/97) - Updated to Multilingual Conversion.   |
|  (03/11/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: batch_post.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/batch_post/batch_post.c,v 5.1 2001/08/09 09:49:45 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_misc_mess.h>

/*==================================
|   Constants, defines and stuff    |
 ==================================*/

#define	INVOICE		(type_flag[0] == 'I')
#define	INV_STR		((INVOICE) ? "Invoice" : "Credit Note")

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

	const int comm_no_fields = 5;

	struct 
    {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*============================================
	| Customer Order/Invoice/Credit Header File. |
	============================================*/
	struct dbview cohr_list[] =
    {
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_type"},
		{"cohr_batch_no"},
		{"cohr_stat_flag"},
	};

	const int cohr_no_fields = 6;

	struct 
    {
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

/*============================
| Local & Screen Structures. |
============================*/
struct 
{
	char	batch_no[6];
	char	dummy[11];
} local_rec;

static struct var vars[] =
{	
	{1, LIN, "batch_no", 4, 15, CHARTYPE, 
		"UUUUU", "          ", 
		" ", "", "Batch No ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.batch_no}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""} 
};

/*==============================
|   Local function prototypes   |
 ==============================*/
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void show_batch (char *key_val);
void next_batch (char *batch_no);
void process (void);
int  heading (int scn);
void shutdown_prog (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int   argc, 
 char *argv[])
{
	if (argc != 4)
	{
		print_at (0,0,ML (mlMiscMess700),argv[0]);
		return (EXIT_FAILURE);
	}

	switch (argv[1][0])
	{
	case	'I':
	case	'i':
		strcpy (type_flag,"I");
		break;

	case	'C':
	case	'c':
		strcpy (type_flag,"C");
		break;

	default:
		print_at (1,0,ML (mlMiscMess701));
		return (EXIT_FAILURE);
	}

	sprintf (find_flag,"%-1.1s",argv[2]);
	sprintf (update_flag,"%-1.1s",argv[3]);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	search_ok = 1;

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;

		heading (1);
		entry (1);
		if (prog_exit)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
        {
			continue;
        }

		rset_tty ();
		prog_exit = 1;
		process ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("cohr",cohr_list,cohr_no_fields,"cohr_batch_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("cohr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (strcmp (FIELD.label,"batch_no") == 0)
	{
		if (last_char == SEARCH)
        {
			show_batch (temp_str);
        }
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


void
show_batch (
 char *key_val)
{
	char	last_batch[6];

	work_open ();
	save_rec ("#Batch","# ");

	strcpy (last_batch,"~~~~~");

	strcpy (cohr_rec.hr_batch_no,local_rec.batch_no);
	cc = find_rec ("cohr",&cohr_rec,GTEQ,"r");

	while (!cc && !strncmp (cohr_rec.hr_batch_no,key_val,strlen (key_val)))
	{
		if (!strcmp (cohr_rec.hr_co_no, comm_rec.tco_no) && 
		    !strcmp (cohr_rec.hr_br_no, comm_rec.test_no) && 
		    cohr_rec.hr_type[0] == type_flag[0] && 
		    cohr_rec.hr_stat_flag[0] == find_flag[0])
		{
			/*---------------------------------------
			| Only Save One Copy of Each Batch No	|
			---------------------------------------*/
			if (strcmp (last_batch,cohr_rec.hr_batch_no))
			{
				cc = save_rec (cohr_rec.hr_batch_no," ");
				if (cc)
                {
					break;
                }
			}
			strcpy (last_batch,cohr_rec.hr_batch_no);
			next_batch (cohr_rec.hr_batch_no);
			cc = find_rec ("cohr",&cohr_rec,GTEQ,"r");
		}
		else
        {
			cc = find_rec ("cohr",&cohr_rec,NEXT,"r");
        }
	}
	disp_srch ();
	work_close ();
}

/*===============================
| Get The Next Batch No.	|
===============================*/
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
            {
				batch_no[j] = ' ';
            }
			return;
		}
	}
}

void
process (
 void)
{
	sprintf (err_str," Updating %s Batch %s",INV_STR,local_rec.batch_no);
	dsp_screen (err_str,comm_rec.tco_no,comm_rec.tco_name);

	strcpy (cohr_rec.hr_batch_no,local_rec.batch_no);

    cc = find_rec ("cohr",&cohr_rec,GTEQ,"u");
	while (!cc && 
	       !strcmp (cohr_rec.hr_batch_no, local_rec.batch_no))
	{
		if (!strcmp (cohr_rec.hr_co_no,comm_rec.tco_no) && 
		    !strcmp (cohr_rec.hr_br_no,comm_rec.test_no) && 
		    cohr_rec.hr_type[0] == type_flag[0] && 
		    cohr_rec.hr_stat_flag[0] == find_flag[0])
		{
			sprintf (err_str,"%s : ",INV_STR);
			dsp_process (err_str,cohr_rec.hr_inv_no);
			strcpy (cohr_rec.hr_stat_flag,update_flag);
	
            cc = abc_update ("cohr",&cohr_rec);
			if (cc)
            {
				sys_err ("Error in cohr During (DBUPDATE)",cc,PNAME);
            }
		}
		abc_unlock ("cohr");
		cc = find_rec ("cohr",&cohr_rec,NEXT,"u");
	}
	abc_unlock ("cohr");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
			scn_set (scn);
        }
		clear ();
		sprintf (err_str,ML (mlMiscMess002),INV_STR);
		rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
		move (0,1);
		line (80);

		box (0,3,80,1);

		move (0,20);
		line (80);
		print_at (21,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		print_at (22,0,ML (mlStdMess039),comm_rec.test_no,comm_rec.test_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

/* [ end of file ] */
