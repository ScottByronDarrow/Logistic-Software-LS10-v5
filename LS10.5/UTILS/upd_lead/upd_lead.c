/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( upd_lead.c    )                                  |
|  Program Desc  : ( Up Lead Times Program                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, incc, ccmr, inis, inmr, sumr,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  incc,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 17/05/94         |
|---------------------------------------------------------------------|
|  Date Modified : (01/09/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (04/10/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (12/09/97)        Modified By   : Marnie Organo.   |
|                                                                     |
|  Comments      :                                                    |
|  (01/09/94)    : DPL 10237 - not printing the decimal point.        |
|  (04/10/94)    : PSL 11299 - upgrade to ver 9 - mfg cutover - no    |
|                : code changes                                       |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: upd_lead.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/upd_lead/upd_lead.c,v 5.2 2001/08/09 09:27:49 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#define		MOD			 5
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int	comm_no_fields = 3;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
	} comm_rec;

	/*======================================
	| Inventory warehouse/cost centre file |
	======================================*/
	struct dbview incc_list [] =
	{
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_lead_time"},
	};

	int	incc_no_fields = 3;

	struct tag_inccRecord
	{
		long	hhcc_hash;
		long	hhbr_hash;
		float	lead_time;
	} incc_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int	ccmr_no_fields = 3;

	struct tag_ccmrRecord
	{
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
	} ccmr_rec;

	/*=================================
	| Stock Inventory Supplier Record |
	=================================*/
	struct dbview inis_list [] =
	{
		{"inis_br_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_priority"},
		{"inis_lead_time"},
	};

	int	inis_no_fields = 5;

	struct tag_inisRecord
	{
		char	br_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_priority [2];
		float	lead_time;
	} inis_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
	};

	int	inmr_no_fields = 3;

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
	} inmr_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview sumr_list [] =
	{
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
	};

	int	sumr_no_fields = 3;

	struct tag_sumrRecord
	{
		long	hhsu_hash;
		char	crd_name [41];
		char	acronym [10];
	} sumr_rec;

	char	*data	= "data",
			*comm	= "comm",
			*incc	= "incc",
			*ccmr	= "ccmr",
			*inis	= "inis",
			*inmr	= "inmr",
			*sumr	= "sumr";
		
	FILE 	*fout;

	int		lpno;
	int		UPDATE;

	char	systemDate [11];

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcRecords (void);
void PrintHeading (void);
void PrintError (int ErrorNo);
void PrintDetails (void);
void InitOutput (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 3)
	{
		print_at(0,0,mlUtilsMess710, argv [0]);
		print_at(1,0,mlUtilsMess711);
		print_at(2,0,mlUtilsMess712);
		return (EXIT_FAILURE);
	}
	lpno = atoi (argv [1]);
	switch (argv [2] [0])
	{
	case	'Y':
		UPDATE = TRUE;
		break;
	case	'N':
		UPDATE = FALSE;
		break;
	default    :
/*
		printf ("usage : %s LPNO <Update> \n", argv [0]);
		printf ("       Update - (Y)es update incc file \n");
		printf ("              - (N)o update incc file \n");
*/
		print_at(0,0,ML(mlUtilsMess710), argv [0]);
		print_at(1,0,ML(mlUtilsMess711));
		print_at(2,0,ML(mlUtilsMess712));
		return (EXIT_FAILURE);
	}
	strcpy (systemDate, DateToString (TodaysDate()));

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	clear ();
	crsr_off ();
	fflush (stdout);

	InitOutput ();

	ProcRecords ();

	fprintf (fout, ".EOF\n");
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

	open_rec (incc, incc_list, incc_no_fields, "incc_id_no");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
	open_rec (inis, inis_list, inis_no_fields, "inis_id_no2");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, sumr_no_fields, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (sumr);

	abc_dbclose (data);
}

void
ProcRecords (
 void)
{
	PrintHeading ();

	/* search for all incc records */
	incc_rec.hhcc_hash = 0L;
	incc_rec.hhbr_hash = 0L;
	cc = find_rec (incc, &incc_rec, GTEQ, "u");
	while (!cc)
	{
		/* find warehouse master file */
		cc = find_hash (ccmr, &ccmr_rec, COMPARISON, "r", incc_rec.hhcc_hash);
		if (cc)
		{
			PrintError (1);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		/* find inventory supplier file */
		strcpy (inis_rec.br_no, ccmr_rec.est_no);
		inis_rec.hhbr_hash = incc_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "1"); 
		cc = find_rec (inis, &inis_rec, GTEQ, "r");
		if (cc ||
			strcmp (inis_rec.br_no, ccmr_rec.est_no) ||
			inis_rec.hhbr_hash != incc_rec.hhbr_hash)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		/* find inventory master file */
		cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", incc_rec.hhbr_hash);
		if (cc)
		{
			PrintError (2);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		cc = find_hash (sumr, &sumr_rec, COMPARISON, "r", inis_rec.hhsu_hash);
		if (cc)
		{
			PrintError (3);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		dsp_process ("Item Number :", inmr_rec.item_no);

		PrintDetails ();

		incc_rec.lead_time = inis_rec.lead_time;

		if (UPDATE)
		{
			cc = abc_update (incc, &incc_rec);
			if (cc)
				file_err (cc, incc, "DBUPDATE");
		}

		cc = find_rec (incc, &incc_rec, NEXT, "u");
	}
	abc_unlock (incc);

	return;
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".E UPDATE LEAD TIME OF PURCHASED ITEMS \n");
	fprintf (fout, ".E FOR INVENTORY WAREHOUSE (incc) \n");
	fprintf (fout, ".E COMPANY : %s \n", clip (comm_rec.tco_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, ".E RECORDS UPDATED : %s \n", UPDATE ? "YES" : "NO");

	fprintf (fout, ".B1\n");

	fprintf (fout, "=======");
	fprintf (fout, "=======");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "============");
	fprintf (fout, "===========================================");
	fprintf (fout, "====");
	fprintf (fout, "=============\n");

	fprintf (fout, "|BR NO ");
	fprintf (fout, "|WH NO ");
	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|          D E S C R I P T I O N           ");
	fprintf (fout, "|SUP ACRONYM");
	fprintf (fout, "|        S U P P L I E R    N A M E        ");
	fprintf (fout, "|PRI");
	fprintf (fout, "| LEAD TIME |\n");

	fprintf (fout, "|------");
	fprintf (fout, "|------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|---");
	fprintf (fout, "|-----------|\n");

	fprintf (fout, ".R=======");
	fprintf (fout, "=======");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "============");
	fprintf (fout, "===========================================");
	fprintf (fout, "====");
	fprintf (fout, "=============\n");
}

void
PrintError (
 int ErrorNo)
{
	switch (ErrorNo)
	{
	case	1:
				/*"Warehouse File Not Found [%11d]",*/
		sprintf (err_str,ML(mlStdMess100),
				incc_rec.hhcc_hash);
		break;
	case	2:
				/*"Inventory File Not Found [%11d]",*/
		sprintf (err_str,ML(mlStdMess155),
				incc_rec.hhbr_hash);
		break;
	case	3:
				/*"Supplier File Not Found [%11d] ",*/
		sprintf (err_str,ML(mlStdMess022),
				inis_rec.hhsu_hash);
		break;
	}

	fprintf (fout, "|********************************************");
	fprintf (fout, "          %-38.38s          ", err_str);
	fprintf (fout, "********************************************|\n");
}

void
PrintDetails (
 void)
{
	fprintf (fout, "|  %-2.2s  ", ccmr_rec.est_no);
	fprintf (fout, "|  %-2.2s  ", ccmr_rec.cc_no);
	fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
	fprintf (fout, "| %-40.40s ", inmr_rec.description);
	fprintf (fout, "| %-9.9s ", sumr_rec.acronym);
	fprintf (fout, "| %-40.40s ", sumr_rec.crd_name);
	fprintf (fout, "| %-1.1s ", inis_rec.sup_priority);
	fprintf (fout, "|  %6.2f   |\n", inis_rec.lead_time);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	dsp_screen ("Printing Update Lead Time Report",
				comm_rec.tco_no, comm_rec.tco_name);

	/*----------------------
	| Open pipe to pformat | 
	----------------------*/
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Initialize printer for output.  |
	---------------------------------*/
	fprintf (fout, ".START%s <%s>\n", systemDate, PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L148\n");
}



