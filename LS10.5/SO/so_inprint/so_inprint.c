/*=====================================================================
|  Copyright (C) 1988 - 1991 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_inprint.c   )                                 |
|  Program Desc  : ( Background Process to Print Invoice / Credit )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  sobg,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  sobg,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow. . | Date Written  : 25/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : (12/03/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (06.10.94)      | Modified by : Jonathan Chen      |
|                                                                     |
|  Comments      : (12/03/91) - General Updated.                      |
|  (06.10.94) : PSL 11440 Extended bpro_pid from "int" to "long"      |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_inprint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_inprint/so_inprint.c,v 5.1 2001/08/09 09:21:16 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<signal.h>
#include	<alarm_time.h>

#define	LCL_VALID	(!strcmp(sobg_rec.bg_type,"IP") && \
                                 sobg_rec.bg_hash > 0L)

#define	BAD_HASH	(!strcmp(sobg_rec.bg_type,"IP") && \
                                 sobg_rec.bg_hash <= 0L)

	FILE	*pout;

	int	running = 0;
	int	lpno = 0;		/* current sobg_lpno		*/

	char	co_no[3];	/* current sobg_co_no		*/
	char	br_no[3];	/* current sobg_br_no		*/
	char	run_print[81];

static char
	*data = "data",
	*bpro = "bpro",
	*sobg = "sobg";

	struct {
		char	tco_no[3];
		char	test_no[3];
	} comm_rec;

	/*=
	|  |
	=*/
	struct dbview sobg_list[] ={
		{"sobg_co_no"},
		{"sobg_br_no"},
		{"sobg_type"},
		{"sobg_lpno"},
		{"sobg_hash"}
	};

	int sobg_no_fields = 5;

	struct {
		char	bg_co_no[3];
		char	bg_br_no[3];
		char	bg_type[3];
		int	bg_lpno;
		long	bg_hash;
	} sobg_rec;

	/*============================+
	 | System Batch Control file. |
	 +============================*/
#define	BPRO_NO_FIELDS	5

	struct dbview	bpro_list [BPRO_NO_FIELDS] =
	{
		{"bpro_co_no"},
		{"bpro_br_no"},
		{"bpro_program"},
		{"bpro_hash"},
		{"bpro_pid"}
	};

	struct tag_bproRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	program [15];
		long	hash;
		long	pid;
	}	bpro_rec;

#include	<chk_vble.h>
/*=======================
| Function Declarations |
=======================*/
void open_print (void);
void close_print (void);
void OpenDB (void);
void CloseDB (void);
void process (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int		found = 0;
	pid_t	pid = getpid ();
	char	program[15];

	OpenDB();

	sprintf (bpro_rec.program, "%-14.14s", argv [0]);
	sprintf(program,"%-14.14s",argv[0]);
	sprintf(run_print,"%-80.80s"," ");

	cc = find_rec(bpro,&bpro_rec,GTEQ,"u");

	while (!cc && !strcmp (bpro_rec.program, program))
	{
		abc_unlock(bpro);
		if (bpro_rec.pid != 0 && bpro_rec.pid == pid)
		{
			found = 1;
			break;
		}

		cc = find_rec(bpro,&bpro_rec,NEXT,"u");
	}
	abc_unlock(bpro);

	if (!found)
	{
		strcpy (bpro_rec.co_no, "  ");
		strcpy (bpro_rec.br_no, "  ");
		strcpy (bpro_rec.program, program);
		bpro_rec.hash = 0L;
		bpro_rec.pid = 0;
	}

	signal_on();

	process();

	close_print ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
open_print (
 void)
{
	int	open_pipe = 0;		/* do we need to open the pipe	*/

	/*-----------------------
	| Pipe Not Already Open	|
	-----------------------*/
	if (!running)
		open_pipe = 1;

	if (sobg_rec.bg_lpno == 0)
		sobg_rec.bg_lpno = 1;

	/*-----------------------
	| Printer No. Changed	|
	-----------------------*/
	if (!open_pipe && lpno != sobg_rec.bg_lpno)
	{
		open_pipe = 1;
		close_print();
	}

	/*-----------------------------------------------
	| first time or Company or Branch Changed	|
	-----------------------------------------------*/
	if (lpno == 0 || strcmp(co_no,sobg_rec.bg_co_no) || 
		         strcmp(br_no,sobg_rec.bg_br_no))
	{
		strcpy(err_str,_chk_vble("SO_CTR_INV","so_ctr_inv",
					sobg_rec.bg_co_no,sobg_rec.bg_br_no));

		/*-------------------------------
		| Use a different program	|
		-------------------------------*/
		if (strcmp(err_str,run_print))
		{
			if (!open_pipe)
			{
				open_pipe = 1;
				close_print();
			}
		}
	}

	strcpy(run_print,err_str);
	strcpy(co_no,sobg_rec.bg_co_no);
	strcpy(br_no,sobg_rec.bg_br_no);
	lpno = sobg_rec.bg_lpno;

	/*-------------------------------
	| Require to Open the Pipe	|
	-------------------------------*/
	if (open_pipe)
	{
		if ((pout = popen(run_print,"w")) == 0)
		{
			sprintf(err_str,"Error in %s during (POPEN)",run_print);
			sys_err(err_str,errno,PNAME);
		}
		running = 1;

		fprintf(pout,"%d\n",sobg_rec.bg_lpno);
		fprintf(pout,"S\n");
		fflush(pout);
	}

	if (running)
	{
		fprintf(pout,"%ld\n",sobg_rec.bg_hash);
		fflush(pout);
		/*---------------------------------------
		| Reset Time stuff as processing	|
		| valid sobg record			|
		---------------------------------------*/
		set_timer();
	}
}

void
close_print (
 void)
{
	if (!running)
		return;

	running = 0;
	fprintf(pout,"0\n");
	fflush(pout);

	pclose(pout);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	open_rec (bpro, bpro_list, BPRO_NO_FIELDS, "bpro_program");
	open_rec(sobg,sobg_list,sobg_no_fields,"sobg_id_no_2");
}

void
CloseDB (
 void)
{
	abc_fclose (bpro);
	abc_fclose (sobg);
	abc_dbclose (data);
}

/*===============================
| Process sobg records			|
| where sobg_type = "IP"		|
===============================*/
void
process (
 void)
{
	/*---------------------------
	| Process Until Prog Exit	|
	---------------------------*/
	while (!prog_exit)
	{
		/*---------------------------
		| Initialise to bpro record	|
		---------------------------*/
		strcpy(sobg_rec.bg_type,"IP");
		sobg_rec.bg_lpno = -1;

		cc = find_rec(sobg,&sobg_rec,GTEQ,"u");

		while (!prog_exit && !cc && !strcmp(sobg_rec.bg_type,"IP"))
		{
			if ( BAD_HASH )
			{
				abc_delete(sobg);
				break;
			}
	
			/*-------------------------------
			| Only Print Valid hhco_hashes	|
			-------------------------------*/
			if ( LCL_VALID && sobg_rec.bg_lpno >= 0 )
			{
				if ( abc_delete(sobg) )
					break;

				open_print();
			}
			else
			{
				abc_delete(sobg);
				break;
			}

			strcpy(sobg_rec.bg_type,"IP");
			sobg_rec.bg_lpno = -1;

			cc = find_rec(sobg,&sobg_rec,GTEQ,"u");
		}
		abc_unlock(sobg);
		
		/*-----------------------------------------------
		| Ran out of records to process so start timing	|
		-----------------------------------------------*/
		close_print();
		time_out();
	}
}
