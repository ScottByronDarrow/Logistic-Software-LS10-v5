/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( upd_lead.c    )                                  |
|  Program Desc  : ( Up Lead Times Program                        )   |
|                  (                                                ) |
|                  (                                                ) |
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
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: upd_lead.c,v $
| Revision 5.3  2001/08/09 09:30:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:28:07  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/24 03:19:38  cha
| Sample.
|
| Revision 5.0  2001/06/19 08:07:51  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:46  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:54  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:34:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/10/20 02:06:48  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/09/29 10:10:55  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:26:48  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 09:20:51  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 07:27:08  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: upd_lead.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/upd_lead/upd_lead.c,v 5.3 2001/08/09 09:30:12 scott Exp $";
/*testing*/
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
		{"comm_est_no"},
		{"comm_cc_no"},
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	tcc_no [3];
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


	/*==================================+
	 | Stock Inventory Supplier Record. |
	 +==================================*/
#define	INIS_NO_FIELDS	26

	struct dbview	inis_list [INIS_NO_FIELDS] =
	{
		{"inis_co_no"},
		{"inis_br_no"},
		{"inis_wh_no"},
		{"inis_hhbr_hash"},
		{"inis_hhsu_hash"},
		{"inis_sup_part"},
		{"inis_sup_priority"},
		{"inis_hhis_hash"},
		{"inis_fob_cost"},
		{"inis_lcost_date"},
		{"inis_duty"},
		{"inis_licence"},
		{"inis_sup_uom"},
		{"inis_pur_conv"},
		{"inis_min_order"},
		{"inis_norm_order"},
		{"inis_ord_multiple"},
		{"inis_pallet_size"},
		{"inis_lead_time"},
		{"inis_sea_time"},
		{"inis_air_time"},
		{"inis_lnd_time"},
		{"inis_dflt_lead"},
		{"inis_weight"},
		{"inis_volume"},
		{"inis_stat_flag"}
	};

	struct tag_inisRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		long	hhbr_hash;
		long	hhsu_hash;
		char	sup_part [17];
		char	sup_priority [3];
		long	hhis_hash;
		double	fob_cost;
		Date	lcost_date;
		char	duty [3];
		char	licence [3];
		long	sup_uom;
		float	pur_conv;
		float	min_order;
		float	norm_order;
		float	ord_multiple;
		float	pallet_size;
		float	lead_time;
		float	sea_time;
		float	air_time;
		float	lnd_time;
		char	dflt_lead [2];
		float	weight;
		float	volume;
		char	stat_flag [2];
	}	inis_rec;

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
		
	FILE	*fout;

	int		lpno;
	int		UPDATE;

#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcRecords (void);
void PrintHeading (void);
void PrintError (int iErrNo);
void PrintDetails (void);
void InitOutput (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	if (argc != 3)
	{
/*
		printf ("usage : %s LPNO <Update> \n", argv [0]);
		print_at(1,0,"       Update - (Y)es update incc file \n");
		print_at (2,0,"              - (N)o update incc file \n");
*/
		print_at(0,0,ML(mlUtilsMess710), argv [0]);
		print_at(1,0,ML(mlUtilsMess711));
		print_at(2,0,ML(mlUtilsMess712));
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
	abc_dbopen (data);

	open_rec (incc, incc_list, incc_no_fields, "incc_id_no");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, sumr_no_fields, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (sumr);

	abc_dbclose (data);
}

void
ProcRecords (void)
{
	PrintHeading ();

	/* search for all incc records */
	incc_rec.hhcc_hash = 0L;
	incc_rec.hhbr_hash = 0L;
	cc = find_rec (incc, &incc_rec, GTEQ, "u");
	while (!cc)
	{
		/* find warehouse master file */
		ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			PrintError (1);
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		/*---------------------------------------
		| try to find priority one inis record. |
		---------------------------------------*/
		inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		strcpy (inis_rec.sup_priority, "W1");
		strcpy (inis_rec.co_no, comm_rec.tco_no);
		strcpy (inis_rec.br_no, comm_rec.test_no);
		strcpy (inis_rec.wh_no, comm_rec.tcc_no);
		cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
		if (cc)
		{
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
		if (cc)
		{
			inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			strcpy (inis_rec.sup_priority, "B1");
			strcpy (inis_rec.co_no, comm_rec.tco_no);
			strcpy (inis_rec.br_no, comm_rec.test_no);
			strcpy (inis_rec.wh_no, comm_rec.tcc_no);
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (inis_rec.wh_no, "  ");
				cc = find_rec (inis, &inis_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
		}
		if (cc)
		{
			inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			strcpy (inis_rec.sup_priority, "W1");
			strcpy (inis_rec.co_no, comm_rec.tco_no);
			strcpy (inis_rec.br_no, comm_rec.test_no);
			strcpy (inis_rec.wh_no, comm_rec.tcc_no);
			cc = find_rec (inis, &inis_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (inis_rec.wh_no, "  ");
				cc = find_rec (inis, &inis_rec, COMPARISON, "r");
			}
			if (cc)
			{
				strcpy (inis_rec.br_no, "  ");
				strcpy (inis_rec.wh_no, "  ");
				cc = find_rec (inis, &inis_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}
		/* find inventory master file */
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			PrintError (2);
			abc_unlock (incc);
			cc = find_rec (incc, &incc_rec, NEXT, "u");
			continue;
		}

		sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			PrintError (3);
			abc_unlock (incc);
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
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (void)
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
 int    iErrNo)
{
	switch (errno)
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
PrintDetails (void)
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
InitOutput (void)
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
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L148\n");
}



