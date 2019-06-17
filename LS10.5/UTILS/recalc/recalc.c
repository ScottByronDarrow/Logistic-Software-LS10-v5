/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( recalc.c      )                                  |
|  Program Desc  : ( Re-calculate conversion factor in the inmr   )   |
|                  ( record.                                      )   |
|---------------------------------------------------------------------|
|  Access files  :  inmr, inum,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  inmr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 30/11/93         |
|---------------------------------------------------------------------|
|  Date Modified : 23/12/93        | Modified By : Aroha Merrilees.   |
|  Date Modified : (12/09/1997)    | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      :                                                    |
|  (23/12/93)    : DPL 10109 - if the uom does not exist, print blank |
|                : details with a star, for manual calculations.      |
|  (12/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: recalc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/recalc/recalc.c,v 5.2 2001/08/09 09:27:36 scott Exp $";

#define	NO_SCRGEN
#include <pslscr.h>
#define	MOD	5
#include <dsp_screen.h>
#include <dsp_process.h>
#include <number.h>
#include <ml_utils_mess.h>

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

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
	};

	int	inmr_no_fields = 7;

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		long	std_uom;
		long	alt_uom;
		float	uom_cfactor;
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
char	systemDate [11];
float	cnv_fct;
int		star;
int		updateRecords;

FILE	*fout;

char	*data	= "data",
		*inmr	= "inmr",
		*inum	= "inum";
	
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ProcRecords (void);
void CalcConv (void);
void PrintDetails (void);
void InitOutput (void);
void PrintHeading (void);
int  check_page (void);

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
		/*---------------------------------------
		| usage : %s LPNO <update>				|
		|      update - Y(es update inmr file	|
		|             - N(o update on file		|
		---------------------------------------*/
		print_at (0,0, ML(mlUtilsMess710), argv [0]);
		print_at (0,0, ML(mlUtilsMess711) );
		print_at (0,0, ML(mlUtilsMess712) );
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
			/*------------------------------------
			|      update - Y(es update inmr file|
			|             - N(o update on file	 |
			------------------------------------*/
			print_at (0,0, ML(mlUtilsMess711) );
			print_at (0,0, ML(mlUtilsMess712) );
			return (EXIT_FAILURE);
	}
	strcpy (systemDate, DateToString (TodaysDate()));

	clear ();
	crsr_off ();
	fflush (stdout);
	InitOutput ();

	ProcRecords ();

	fprintf (fout, "|%-155.155s|\n", " ");
	fprintf (fout, "| * - Manual calculation of the conversion factor is required. %-93.93s|\n", " ");

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

	open_rec (inmr,		inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum,		inum_list, inum_no_fields, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inum);

	abc_dbclose (data);
}

/*
spec_valid (field)
 int	field;
{
	return (EXIT_SUCCESS);
}
*/

/*=================================
| Process all valid pcwo records. |
=================================*/
void
ProcRecords (
 void)
{
	PrintHeading ();

	inmr_rec.hhbr_hash = 0L;
	cc = find_hash (inmr, &inmr_rec, GTEQ, "u", inmr_rec.hhbr_hash);
	while (!cc)
	{
		star = FALSE;
		CalcConv ();

		/* only print details if conversion factor changes */
		if (cnv_fct != inmr_rec.uom_cfactor)
		{
			PrintDetails ();

			if (updateRecords)
			{
				inmr_rec.uom_cfactor = cnv_fct;
				cc = abc_update (inmr, &inmr_rec);
				if (cc)
					file_err (cc, inmr, "DBUPDATE");
			}
		}
		else
			abc_unlock (inmr);

		cc = find_hash (inmr, &inmr_rec, NEXT, "u", inmr_rec.hhbr_hash);		
	}
	abc_unlock (inmr);
}

void
CalcConv (
 void)
{
	number	std_cnv_fct;
	number	alt_cnv_fct;
	number	result;

	cc = find_hash (inum, &inum_rec [0], EQUAL, "r", inmr_rec.std_uom);
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

	cc = find_hash (inum, &inum_rec [1], EQUAL, "r", inmr_rec.alt_uom);
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
	NumFlt (&alt_cnv_fct, inum_rec [1].cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| std uom cnv_fct / alt uom cnv_fct = conversion factor    |
	----------------------------------------------------------*/
	NumDiv (&std_cnv_fct, &alt_cnv_fct, &result);

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	cnv_fct = NumToFlt (&result);

	return;
}

void
PrintDetails (
 void)
{
	dsp_process ("Item Number :", inmr_rec.item_no);

	fprintf (fout, "|%-2.2s",			inmr_rec.co_no);
	fprintf (fout, "|%-16.16s",			inmr_rec.item_no);
	fprintf (fout, "|%-33.33s",			inmr_rec.description);
	fprintf (fout, "|%-15.15s",			inum_rec [0].uom_group);
	fprintf (fout, "|%-4.4s",			inum_rec [0].uom);
	fprintf (fout, "|%14.6f",			inum_rec [0].cnv_fct);
	fprintf (fout, "|%-15.15s",			inum_rec [1].uom_group);
	fprintf (fout, "|%-4.4s",			inum_rec [1].uom);
	fprintf (fout, "|%14.6f",			inum_rec [1].cnv_fct);
	fprintf (fout, "|%14.6f",			inmr_rec.uom_cfactor);
	fprintf (fout, "|%14.6f|%-1.1s\n",	cnv_fct, star ? "*" : " ");
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
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
	fprintf (fout, ".START%s <%s>\n", systemDate, PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	fprintf (fout, ".E ITEM CONVERSION FACTOR RE-CALCULATION REPORT\n");
	fprintf (fout, ".E FOR INVENTORY MASTER (inmr)\n");
	fprintf (fout, ".E COMPANY : %s\n", clip (comm_rec.tco_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, "===");
	fprintf (fout, "=================");
	fprintf (fout, "==================================");
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
	fprintf (fout, "|        ITEM DESCRIPTION         ");
	fprintf (fout, "|            STA");
	fprintf (fout, "NDARD");
	fprintf (fout, "               ");
	fprintf (fout, "|           ALTE");
	fprintf (fout, "RNATE");
	fprintf (fout, "               ");
	fprintf (fout, "|     OLD      ");
	fprintf (fout, "|     NEW      |\n");

	fprintf (fout, "|NO");
	fprintf (fout, "|                ");
	fprintf (fout, "|                                 ");
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
	fprintf (fout, "|---------------------------------");
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
	fprintf (fout, "==================================");
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
check_page (
 void)
{
	return (EXIT_SUCCESS);
}
