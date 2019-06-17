/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( gen_num.c     )                                  |
|  Program Desc  : ( One-Off program to update the works order    )   |
|                  ( numbers with system generated numbers.       )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ccmr, pcwo, inmr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  ccmr, pcwo,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 26/01/94         |
|---------------------------------------------------------------------|
|  Modified By   : Marnie Organo     Date Modified : 15/10/97         |  
|  Date Modified : (28/09/1999)    | Modified by : edge cabalfin      |
|                :                                                    |
|                                                                     |
|  Comments      :                                                    |
|  (15/10/97)    :Updated to Multilingual Conversion                  |
|  (28/09/1999)  : ANSIfication of the code                           |
|                :      - potential problems marked with QUERY        |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gen_num.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/gen_num/gen_num.c,v 5.2 2001/08/09 09:49:46 scott Exp $";

/*==============================
|   Include file dependencies   |
 ==============================*/

#define	NO_SCRGEN
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_misc_mess.h>

#define	MOD	5
#include <dsp_screen.h>
#include <dsp_process.h>

/*==================================
|   Constants, defines and stuff    |
 ==================================*/
/*  QUERY
    these should be declared as const char*
    to minimize potential problems.
*/
char	*data	= "data",
		*comm	= "comm",
		*ccmr	= "ccmr",
		*pcwo	= "pcwo",
		*pcwo2	= "pcwo2",
		*pcwo3	= "pcwo3",
		*pcwo4	= "pcwo4",
		*inmr	= "inmr";
 
	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	const int comm_no_fields = 3;

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	tco_name [41];
	} comm_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_nx_wo_num"},
	};

	const int ccmr_no_fields = 5;

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
		long	nx_wo_num;
	} ccmr_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_hhwo_hash"},
		{"pcwo_hhbr_hash"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
	};

	const int pcwo_no_fields = 8;

	struct tag_pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	order_no [8];
		long	hhwo_hash;
		long	hhbr_hash;
		char	order_status [2];
		char	batch_no [11];
	} pcwo_rec, pcwo2_rec, pcwo3_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
	};

	const int inmr_no_fields = 3;

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
	} inmr_rec;

/*====================
|   Local variables   |
 ====================*/

	FILE	*fout;
	int		PC_GEN;
	int		STAR;
	int		lp_no;
	int 	counter;
	char	tmp_order [8];
	char	systemDate [9];

/*==============================
|   Local function prototypes   |
 ==============================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  heading (int scn);

int  ProcNum (void);
int  CheckPcwo (long int order_no);
void ReportPrint (void);
void InitOutput (void);
int  check_page (void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int   argc, 
 char *argv[])
{
	char	*sptr;

	/*-----------------------------------------------------
	| Works Order number is M(anual or S(ystem generated. |
	-----------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;

	if (!PC_GEN)
	{
		/*printf (" PC_GEN_NUM Must Be Set To 'S' \n");*/
		print_at (0,0,ML(mlMiscMess003));
		return (EXIT_FAILURE);
	}

	if (argc != 2)
	{
	/*	printf ("usage : %s LPNO\n",argv[0]);*/
		print_at (1,0,ML(mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	lp_no = atoi (argv [1]);
	sprintf (systemDate, "%-10.10s", DateToString (TodaysDate()));

	/*-----------------------------
	| Process Records in Database.|
	-----------------------------*/
	clear ();
	crsr_off ();
	fflush (stdout);
	InitOutput ();
	counter = 0;

	ProcNum ();

	fprintf (fout,".EOF\n");
	pclose (fout);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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
	abc_dbopen (data);

	abc_alias (pcwo2, pcwo);
	abc_alias (pcwo3, pcwo);
	abc_alias (pcwo4, pcwo);

	open_rec (ccmr,  ccmr_list, ccmr_no_fields, "ccmr_id_no");
	open_rec (pcwo,  pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, pcwo_no_fields, "pcwo_id_no");
	open_rec (pcwo3, pcwo_list, pcwo_no_fields, "pcwo_id_no3");
	open_rec (pcwo4, pcwo_list, pcwo_no_fields, "pcwo_hhwo_hash");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (pcwo3);
	abc_fclose (pcwo4);
	abc_fclose (inmr);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
    /* QUERY
     * why is this function empty? */
	return (EXIT_SUCCESS);
}

int
ProcNum (
 void)
{
	int		firstTime;
	char	firstOrder [8];

	strcpy (ccmr_rec.co_no, comm_rec.tco_no);
	strcpy (ccmr_rec.est_no, " ");
	strcpy (ccmr_rec.cc_no, " ");

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no, comm_rec.tco_no))
	{
		firstTime = TRUE;
		strcpy (firstOrder, " ");
		strcpy (pcwo_rec.co_no, ccmr_rec.co_no);
		strcpy (pcwo_rec.br_no, ccmr_rec.est_no);
		strcpy (pcwo_rec.wh_no, ccmr_rec.cc_no);
		strcpy (pcwo_rec.order_no, " ");

		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
		while (!cc &&
		       !strcmp (pcwo_rec.co_no, ccmr_rec.co_no) &&
		       !strcmp (pcwo_rec.br_no, ccmr_rec.est_no) &&
		       !strcmp (pcwo_rec.wh_no, ccmr_rec.cc_no))
		{
			if (!strcmp (pcwo_rec.order_no, firstOrder))
			{
				break;
			}

			cc = find_hash (pcwo4,
			                &pcwo_rec,
			                COMPARISON,
			                "u",
			                pcwo_rec.hhwo_hash);
			if (cc)
			{
				file_err (cc, pcwo, "DBFIND");
			}
				
			/*---------------------------------------------------------
			| if batch number blank copy order number to batch number |
			---------------------------------------------------------*/
			if (!strncmp (pcwo_rec.batch_no, "          ", 10))
			{
				STAR = FALSE;
				strcpy (pcwo_rec.batch_no, pcwo_rec.order_no);

				strcpy (pcwo3_rec.co_no, ccmr_rec.co_no);
				strcpy (pcwo3_rec.br_no, ccmr_rec.est_no);
				strcpy (pcwo3_rec.wh_no, ccmr_rec.cc_no);
				strcpy (pcwo3_rec.batch_no, pcwo_rec.batch_no);

				cc = find_rec (pcwo3, &pcwo3_rec, COMPARISON, "r");
				if (!cc)
				{
					strcpy (pcwo_rec.batch_no, "          ");	
					STAR = TRUE;
				}
			}
			else
			{
				STAR = TRUE;
			}

			/*--------------------
			| store order number |
			--------------------*/
			strcpy (tmp_order, pcwo_rec.order_no);

			/*---------------------------------------------
			| check if works order number already exists, |
			| if it does than skip to the next.           |
			---------------------------------------------*/
			while (CheckPcwo(++ccmr_rec.nx_wo_num) == 0);
			sprintf (pcwo_rec.order_no, "%07ld", ccmr_rec.nx_wo_num);

			if (firstTime)
			{
				strcpy (firstOrder, pcwo_rec.order_no);
				firstTime = FALSE;
			}

			cc = abc_update (pcwo4, &pcwo_rec);
			if (cc)
			{
				file_err (cc, pcwo, "DBUPDATE");
			}

			cc = find_hash (inmr,
			                &inmr_rec,
			                COMPARISON,
			                "r",
			                pcwo_rec.hhbr_hash);
			                
			if (cc)
			{
				file_err (cc, inmr, "DBFIND");
			}

			ReportPrint ();

			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}

		ccmr_rec.nx_wo_num ++;

		fprintf (fout, "| NEXT W/O NUM : %07ld", ccmr_rec.nx_wo_num);
		fprintf (fout,
		        " FOR BRANCH %-2.2s WAREHOUSE %-2.2s               ",
		        ccmr_rec.est_no,
		        ccmr_rec.cc_no);
		fprintf (fout, "                                       |\n");

		cc = abc_update (ccmr, &ccmr_rec);
		if (cc)
		{
			file_err (cc, ccmr, "DBFIND");
		}

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "| RECORD COUNT : %5d ", counter);
	fprintf (fout, "                                         ");
	fprintf (fout, "                                         |\n");

	fprintf (fout, "|           *  - Record already had an existing batch");
	fprintf (fout, " number, batch number has not be changed.            |\n");
	fprintf (fout, "|                       Or the batch number already e");
	fprintf (fout, "xists for another works order.                       |\n");

	return (EXIT_SUCCESS);
}

int
CheckPcwo (
 long order_no)
{
	strcpy (pcwo2_rec.co_no, ccmr_rec.co_no);
	strcpy (pcwo2_rec.br_no, ccmr_rec.est_no);
	strcpy (pcwo2_rec.wh_no, ccmr_rec.cc_no);
	sprintf (pcwo2_rec.order_no, "%07ld", order_no);
	cc = find_rec (pcwo2, &pcwo2_rec, COMPARISON, "r");
	return (cc);
}

void
ReportPrint (
 void)
{
	dsp_process ("W/O No :", pcwo_rec.order_no);

	fprintf (fout, "| %-2.2s ",		pcwo_rec.br_no);
	fprintf (fout, "| %-2.2s ",		pcwo_rec.wh_no);
	fprintf (fout, "| %-7.7s ",		tmp_order);
	fprintf (fout, "| %-7.7s ",		pcwo_rec.order_no);
	fprintf (fout, "| %-10.10s ",	pcwo_rec.batch_no);
	fprintf (fout, "| %-16.16s ",	inmr_rec.item_no);
	fprintf (fout, "| %-40.40s |",	inmr_rec.description);
	fprintf (fout, "%s\n",			(STAR) ? "*" : " ");

	counter ++;

	fflush (fout);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	dsp_screen (" Printing Works Order Number Generation Report ",
				comm_rec.tco_no, comm_rec.tco_name);

	/*----------------------
	| Open pipe to pformat | 
	----------------------*/
	if ((fout = popen ("pformat", "w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
	}

	/*---------------------------------
	| Initialize printer for output.  |
	---------------------------------*/
	fprintf (fout, ".START%s <%s>\n", systemDate, PNAME);
	fprintf (fout, ".LP%d\n", lp_no);
	fprintf (fout, ".9");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L115\n");

	fprintf (fout, ".E WORKS ORDER NUMBER GENERATION REPORT \n");

	fprintf (fout, ".E COMPANY NAME : %s\n",
	         clip (comm_rec.tco_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, "| BR ");
	fprintf (fout, "| WH ");
	fprintf (fout, "| CURRENT ");
	fprintf (fout, "|   NEW   ");
	fprintf (fout, "|   BATCH    ");
	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|               DESCRIPTION                |\n");

	fprintf (fout, "| NO ");
	fprintf (fout, "| NO ");
	fprintf (fout, "| W/O  NO ");
	fprintf (fout, "| W/O  NO ");
	fprintf (fout, "|     NO     ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          |\n");

	fprintf (fout, "=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");

	fprintf (fout, ".R=====");
	fprintf (fout, "=====");
	fprintf (fout, "==========");
	fprintf (fout, "==========");
	fprintf (fout, "=============");
	fprintf (fout, "===================");
	fprintf (fout, "============================================\n");
}

int
heading (
 int scn)
{
    /* QUERY
     * why is this function empty?
     */
    return (EXIT_SUCCESS);
}

int
check_page (
 void)
{
    /* QUERY
     * why is this function empty? 
     */
	return (EXIT_SUCCESS);
}

/* [ end of file ] */
