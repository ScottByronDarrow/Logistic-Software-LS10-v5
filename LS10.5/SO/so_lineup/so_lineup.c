/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_lineup.c    )                                 |
|  Program Desc  : ( Invoice / Credit Note / Packing Slip etc     )   |
|                  ( line up.                                     )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 21/02/89         |
|---------------------------------------------------------------------|
|  Date Modified : (21/02/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (03/08/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (08/09/1997)    | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (27/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      :                                                    |
|                : (03/08/90) - General Update for New Scrgen. S.B.D. |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (08/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|  (27/10/1997)  : SEL. 9.9.3 Update for 8 character invoice numbers. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_lineup.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_lineup/so_lineup.c,v 5.3 2002/07/17 09:58:09 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

int	indx;
char	*comm = "comm";
char	*data = "data";
char	run_print[81];

struct	{
	char	*_desc;
	char	*_prog;
	char	*_dflt;
} programs[] = {
	{"Tax Invoice / Credit Note","SO_CTR_INV","so_ctr_inv"},
	{"Packing Slip","SO_CTR_PAC","so_ctr_pac"},
	{"Customer Statement","DB_STMTPRN","db_stmtprn"},
	{"Supplier Cheque","CR_CHQPRN","cr_chqprn"},
	{"","",""},
};

#define	LCL_DESC	programs[indx]._desc
#define	LCL_PROG	programs[indx]._prog
#define	LCL_DFLT	programs[indx]._dflt

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_short"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_short[16];
	} comm_rec;

FILE	*pout;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	p_type[31];
	int	lpno;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "lpno", 4, 15, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No.", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "type", 5, 15, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Print Type", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.p_type}, 

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
int  spec_valid (int field);
void show_type (char *key_val);
void line_up (void);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv[])
{
	SETUP_SCR (vars);

	abc_dbopen(data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	init_scr();
	set_tty(); 
	set_masks();

	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;

		heading(1);
		entry(1);

		if (prog_exit || restart)
			break;

		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			break;

		line_up();
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
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	if (strcmp(FIELD.label,"lpno") == 0)
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess( ML(mlStdMess020) );
			return(1);
		}
		return(0);
	}

	if (strcmp(FIELD.label,"type") == 0)
	{
		if (last_char == SEARCH)
		{
			show_type(temp_str);
			return(0);
		}

		for (indx = 0;strlen(LCL_DESC);indx++)
		{
			sprintf(err_str,"%-30.30s",LCL_DESC);

			if (!strcmp(err_str,local_rec.p_type))
				return(0);
		}

		print_mess( ML(mlSoMess288) );
		return(1);
	}
	return(0);
}

void
show_type (
 char *key_val)
{
	work_open();
	
	save_rec("#Print Type","# ");

	for (indx = 0;strlen(LCL_DESC);indx++)
	{
		cc = save_rec(LCL_DESC," ");
		if (cc)
			break;
	}
	cc = disp_srch();
	work_close();
}

void
line_up (
 void)
{
	int	i;
	char	*sptr;

	/*-------------------------------
	| Check Company & Branch	|
	-------------------------------*/
	sprintf(run_print,"%s%s%s",LCL_PROG,comm_rec.tco_no,comm_rec.test_no);
	sptr = chk_env(run_print);
	if (sptr == (char *)0)
	{
		/*---------------
		| Check Company	|
		---------------*/
		sprintf(run_print,"%s%s",LCL_PROG,comm_rec.tco_no);
		sptr = chk_env(run_print);
		if (sptr == (char *)0)
		{
			sprintf(run_print,"%s",LCL_PROG);
			sptr = chk_env(run_print);
			strcpy(run_print,(sptr == (char *)0) ? LCL_DFLT : sptr);
		}
		else
			strcpy(run_print,sptr);
	}
	else
		strcpy(run_print,sptr);

	if ((pout = popen(run_print,"w")) == 0)
	{
		sprintf(err_str,"Error in %s during (POPEN)",run_print);
		sys_err(err_str,errno,PNAME);
	}

	fprintf(pout,"%d\n",local_rec.lpno);
	fprintf(pout,"S\n");

	do
	{
		fprintf(pout,"-1\n");
		fflush(pout);
		sleep(2);
/* "Reprint Form for Lineup ? "*/
		i = prmptmsg( ML(mlSoMess287),"YyNn",1,2);
		move(1,2);
		cl_line();
		fflush(stdout);
	} while (i == 'Y' || i == 'y');

	fprintf(pout,"0\n");
	pclose(pout);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		clear();

		/*--------------
		| Forms Lineup |
		--------------*/
		rv_pr( ML(mlSoMess286), 32,0,1);

		move(0,1);
		line(80);

		box(0,3,80,2);

		sprintf (err_str, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		print_at(21,0, "%s", err_str);

		sprintf (err_str, ML(mlStdMess039), 
							comm_rec.test_no,comm_rec.test_short);
		print_at(22,0, "%s", err_str);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
