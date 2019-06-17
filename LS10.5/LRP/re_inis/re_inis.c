/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( re_inis       )                                  |
|  Program Desc  : ( Re-calculate conversion factor in the inis   )   |
|                  ( record.                                      )   |
|---------------------------------------------------------------------|
|  Access files  :  inis, inmr, inum, sumr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inis,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 23/12/93         |
|---------------------------------------------------------------------|
|  Modified Date : (11/02/94)        Modified  By  : Aroha Merrilees. |
|  Modified Date : (12/09/97)        Modified  By  : Ana Marie Tario. |
|                                                                     |
|  Comments      :                                                    |
|  (11/02/94)    : PSL 10366 - Show supplier number on report.        |
|  (12/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                                                                     |
| $Log: re_inis.c,v $
| Revision 5.2  2001/08/09 09:30:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:28:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:50  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:01  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:45  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:54  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:34:24  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/09/29 10:10:54  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:26:48  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 09:20:50  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 07:27:07  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: re_inis.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/re_inis/re_inis.c,v 5.2 2001/08/09 09:30:11 scott Exp $";

#define	NO_SCRGEN
#define	MOD	5
#include <ml_std_mess.h>
#include <ml_utils_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <number.h>

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

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview sumr_list [] =
	{
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
	};

	int	sumr_no_fields = 2;

	struct tag_sumrRecord
	{
		char	crd_no [7];
		long	hhsu_hash;
	} sumr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_std_uom"},
	};

	int	inmr_no_fields = 4;

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		long	std_uom;
	} inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list [] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_cnv_fct"}
	};

	int	inum_no_fields = 4;

	struct tag_inumRecord
	{
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		float	cnv_fct;
	} inum_rec [2];

/*=================
| Local variables |
=================*/
int		lpno;
float	cnv_fct;
int		star;
int		updateRecords;

FILE	*fout;

char	*data	= "data",
		*inis	= "inis",
		*sumr	= "sumr",
		*inmr	= "inmr",
		*inum	= "inum";

#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcRecords (void);
void CalcConv (void);
void PrintDetails (void);
void InitOutput (void);
void PrintHeading (void);
int check_page (void);
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int        argc,
 char*      argv[])
{
	if (argc != 3)
	{
		print_at(0,0,mlUtilsMess710, argv[0]);
		print_at(1,0,mlUtilsMess711);
		print_at(2,0,mlUtilsMess712);
        return (EXIT_FAILURE);
	}
		
	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	lpno = atoi (argv [1]);
	switch (argv [2] [0])
	{
	case	'Y' :
			updateRecords = TRUE;
			break;
	case	'N' :
			updateRecords = FALSE;
			break;
	default :
			/*print_at (0,0,"      update - Y(es update inmr file\n");
			print_at (1,0,"             - N(o update on file\n");*/
			print_at (0,0,ML(mlUtilsMess711));
			print_at (1,0,ML(mlUtilsMess712));
            return (EXIT_FAILURE);
	}
	clear ();
	crsr_off ();
	fflush (stdout);
	InitOutput ();

	ProcRecords ();

	fprintf (fout, "|%-134.134s|\n", " ");
	fprintf (fout, "| * - Manual calculation of the conversion factor is required. %-72.72s|\n", " ");

	fprintf (fout,".EOF\n");
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

	open_rec (inis,		inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	open_rec (sumr,		sumr_list, sumr_no_fields, "sumr_hhsu_hash");
	open_rec (inmr,		inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum,		inum_list, inum_no_fields, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inis);
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (inum);

	abc_dbclose (data);
}

/*=================================
| Process all valid pcwo records. |
=================================*/
void
ProcRecords (void)
{
	PrintHeading ();

	inis_rec.hhbr_hash = 0L;
	cc = find_rec (inis, &inis_rec, GTEQ, "u");
	while (!cc)
	{
		star = FALSE;
		inmr_rec.hhbr_hash = inis_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			abc_unlock (inis);
			cc = find_rec (inis, &inis_rec, NEXT, "u");
			continue;
		}

		CalcConv ();

		/* only print details if conversion factor changes */
		if (cnv_fct != inis_rec.pur_conv)
		{
			sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, sumr, "DBFIND");

			PrintDetails ();

			if (updateRecords)
			{
				inis_rec.pur_conv = cnv_fct;
				cc = abc_update (inis, &inis_rec);
				if (cc)
					file_err (cc, inis, "DBUPDATE");
			}
		}
		else
			abc_unlock (inis);

		cc = find_rec (inis, &inis_rec, NEXT, "u");
	}
	abc_unlock (inis);
}

void
CalcConv (void)
{
	number	std_cnv_fct;
	number	sup_cnv_fct;
	number	result;

	inum_rec[0].hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec [0], EQUAL, "r");
	if (cc)
	{
		star = TRUE;
		cnv_fct = 0.00;
		strcpy (inum_rec [0].uom_group, " ");
		strcpy (inum_rec [0].uom, " ");
		inum_rec [0].cnv_fct = 0.00;
		strcpy (inum_rec [1].uom_group, " ");
		strcpy (inum_rec [1].uom, " ");
		inum_rec [1].cnv_fct = 0.00;
		return;
	}

	inum_rec[1].hhum_hash	=	inis_rec.sup_uom;
	cc = find_rec (inum, &inum_rec [1], EQUAL, "r");
	if (cc)
	{
		star = TRUE;
		cnv_fct = 0.00;
		strcpy (inum_rec [1].uom_group, " ");
		strcpy (inum_rec [1].uom, " ");
		inum_rec [1].cnv_fct = 0.00;
		return;
	}

	if (strcmp (inum_rec [0].uom_group, inum_rec [1].uom_group))
	{
		star = TRUE;
		cnv_fct = 0.00;
		return;
	}

	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec [0].cnv_fct);
	NumFlt (&sup_cnv_fct, inum_rec [1].cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| std uom cnv_fct / pur uom cnv_fct = conversion factor    |
	----------------------------------------------------------*/
	NumDiv (&std_cnv_fct, &sup_cnv_fct, &result);

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	cnv_fct = NumToFlt (&result);

	return;
}

void
PrintDetails (void)
{
	dsp_process ("Item", inmr_rec.item_no);

	fprintf (fout, "|%-2.2s",			inis_rec.co_no);
	fprintf (fout, "|%-16.16s",			inmr_rec.item_no);
	fprintf (fout, "|   %-6.6s   ",		sumr_rec.crd_no);
	fprintf (fout, "|%-15.15s",			inum_rec [0].uom_group);
	fprintf (fout, "|%-4.4s",			inum_rec [0].uom);
	fprintf (fout, "|%14.6f",			inum_rec [0].cnv_fct);
	fprintf (fout, "|%-15.15s",			inum_rec [1].uom_group);
	fprintf (fout, "|%-4.4s",			inum_rec [1].uom);
	fprintf (fout, "|%14.6f",			inum_rec [1].cnv_fct);
	fprintf (fout, "|%14.6f",			inis_rec.pur_conv);
	fprintf (fout, "|%14.6f|%-1.1s\n",	cnv_fct, star ? "*" : " ");
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (void)
{
	dsp_screen ("Printing Re-Calculation Report",
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
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L138\n");
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (void)
{
	fprintf (fout, ".E ITEM CONVERSION FACTOR RE-CALCULATION REPORT\n");
	fprintf (fout, ".E FOR INVENTORY SUPPLIER (inis)\n");
	fprintf (fout, ".E COMPANY : %s\n", clip (comm_rec.tco_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, "===");
	fprintf (fout, "=================");
	fprintf (fout, "=============");
	fprintf (fout, "================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");

	fprintf (fout, "|CO");
	fprintf (fout, "|  ITEM  NUMBER  ");
	fprintf (fout, "|SUPPLIER NO.");
	fprintf (fout, "|            STA");
	fprintf (fout, "NDARD");
	fprintf (fout, "               ");
	fprintf (fout, "|            SUP");
	fprintf (fout, "PLIER");
	fprintf (fout, "               ");
	fprintf (fout, "|     OLD      ");
	fprintf (fout, "|     NEW      |\n");

	fprintf (fout, "|NO");
	fprintf (fout, "|                ");
	fprintf (fout, "|            ");
	fprintf (fout, "|     GROUP     ");
	fprintf (fout, "|UOM ");
	fprintf (fout, "|  CONV. FACT  ");
	fprintf (fout, "|     GROUP     ");
	fprintf (fout, "|UOM ");
	fprintf (fout, "|  CONV. FACT  ");
	fprintf (fout, "|  CONV. FACT  ");
	fprintf (fout, "|  CONV. FACT  |\n");

	fprintf (fout, "|--");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------|\n");

	fprintf (fout, ".R===");
	fprintf (fout, "=================");
	fprintf (fout, "=============");
	fprintf (fout, "================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "================");
	fprintf (fout, "=====");
	fprintf (fout, "===============");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");
}

/*=====================================================
| check if a new page is needed on screen or printer. |
=====================================================*/
int
check_page (void)
{
	return (EXIT_SUCCESS);
}
