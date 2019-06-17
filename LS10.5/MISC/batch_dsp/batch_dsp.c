/*=====================================================================
|  Copyright (C) 1986, 1987 Logistic Software Limited.                |
|=====================================================================|
|  Program Name  : ( batch_dsp.c    )                                 |
|  Program Desc  : ( Batch window display.                        )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  : comm , bach,     ,     ,     ,     ,     ,         |
|                       ,     ,      ,     ,     ,     ,              |
|  Database      : (post)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (post)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
|  Date Modified : (22/08/88)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (15.08.94)      | Modified by : Jonathan Chen      |
|  Date Modified : (15.10.97)      | Modified by : Marnie Organo      |
|  Date Modified : (28/09/1999)    | Modified by : edge cabalfin      |
|                :                                                    |
|  Comments                                                           |
|  (15.08.94) : Added #include <pslscr.h>. Removed some erroneus stuff|
|  (15.10.97) : Updated to Multilingual Conversion and added          |
|  (28/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                :                                                    |                                                                     |
|                                                                     |
=====================================================================*/
#define CCMAIN
char	*PNAME = "$RCSfile: batch_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/batch_dsp/batch_dsp.c,v 5.1 2001/08/09 09:49:44 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/
#define		NO_SCRGEN

#include	<pslscr.h>
#include	<ml_misc_mess.h>

/*==================================
|   Constants, defines and stuff    |
 ==================================*/
char    *data	= "data",
        *bach	= "bach";

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] =
    {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_est_no"},
		{"comm_cc_no"},
	};

	const int comm_no_fields = 4;

	struct 
    {
		int	termno;
		char	tco_no[3];
		char	test_no[3];
		char	tcc_no[3];
	} comm_rec;

	/*============================
	| System Batch Control file. |
	============================*/
	struct dbview bach_list[] =
    {
		{"bach_co_no"},
		{"bach_type"},
		{"bach_run_no"},
		{"bach_batch_no"},
		{"bach_date"},
		{"bach_amount"},
		{"bach_stat_flag"}
	};

	const int bach_no_fields = 7;

	struct 
    {
		char	ch_co_no[3];
		char	ch_type[3];
		long	ch_run_no;
		char	ch_batch_no[6];
		long	ch_date;
		double	ch_amount;		/*  Money field  */
		char	ch_stat_flag[2];
	} bach_rec;

	char	*jnl_type[13] = 
    {
		"??????",
		"GENERAL",
		"STANDARD",
		"ACCRUAL",
		"SALES ",
		"SALES RET",
		"RECEIPTS",
		"PAYABLES",
		"CEDITORS C/N",
		"DISBURSEMENTS",
		"INVENTORY",
		"PURCHASES",
		"SKOCK ADJ",
    };

/*==============================
|   Local function prototypes   |
 ==============================*/
void OpenDB (void);
void CloseDB (void);
void display (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int   argc,
 char *argv[])
{
	init_scr ();
	set_tty ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	display ();

	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	open_rec (bach, bach_list, bach_no_fields, "bach_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (bach);
	abc_dbclose (data);
}

/*===============================
| Display note pad information. |
===============================*/
void
display (
 void)
{
	char    disp_str[201];
	int	    jnl_no = 0;

	clear();

	/*rv_pr(" RUN TRANSACTION DISPLAY ",25,0,1);*/
	rv_pr(ML(mlMiscMess001),25,0,1);

	Dsp_open(6,2,14);

	Dsp_saverec("   TRANSACTION   |   RUN    | BATCH  |   DATE   |    AMOUNT.    ");
	Dsp_saverec("    REFERENCE    |  NUMBER  | NUMBER |  TRANS   |               ");
	Dsp_saverec("[F14-Next Screen] [F15-Previous Screen] [F16-Input/End]");

	strcpy(bach_rec.ch_co_no, comm_rec.tco_no);
	strcpy(bach_rec.ch_type, "  ");
	bach_rec.ch_run_no = 0L;
	strcpy(bach_rec.ch_batch_no, "      ");

    cc = find_rec(bach,&bach_rec, GTEQ, "r");
	while (!cc && 
           !strcmp(bach_rec.ch_co_no, comm_rec.tco_no))
	{
		jnl_no = atoi(bach_rec.ch_type);

		sprintf(disp_str,
                 "  %-13.13s  ^E  %06ld  ^E %6.6s ^E%10.10s^E%13.2f ",
                 jnl_type[jnl_no],
                 bach_rec.ch_run_no,
                 bach_rec.ch_batch_no,
                 DateToString(bach_rec.ch_date),
                 DOLLARS(bach_rec.ch_amount));

		Dsp_saverec(disp_str);

		cc = find_rec(bach,&bach_rec, NEXT, "r");
	}
	Dsp_srch();
	Dsp_close();
}

/* [ end of file ] */
