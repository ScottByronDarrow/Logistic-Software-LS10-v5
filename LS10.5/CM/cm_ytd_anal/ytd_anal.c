/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( cm_ytd_anal.c  )                                 |
|  Program Desc  : ( Labour Analysis YTD by Week Ending by Dept   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cmcd, cmcm, cmem, cmeq, cmhr, cmjt, cmts    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written  : (15/04/93)       |
|---------------------------------------------------------------------|
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (10/09/97)      | Modified  by : Leah Manibog.     |
|  Date Modified : (09/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      :                                                    |
|  (15/11/95)    : PDL - Updated for version 9.                       |
|  (10/09/97)    : Updated for Multilingual Conversion.               |
|  (09/09/1999)  : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
| $Log: ytd_anal.c,v $
| Revision 5.2  2001/08/09 08:57:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 22:56:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:02:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:22:03  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:46  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:32:43  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/08 04:35:46  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.7  1999/09/29 10:10:34  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 04:40:15  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.5  1999/09/16 04:44:44  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/14 07:35:25  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "cm_ytd_anal.c",
	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_ytd_anal/ytd_anal.c,v 5.2 2001/08/09 08:57:49 scott Exp $";

#define	NOSCRGEN
#include 	<pslscr.h>
#include	<malloc.h>
#include	<string.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_cm_mess.h>

/*=======================
 Globals & magic numbers
=========================*/
#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

/*#define	LINESIZE	75	*/
#define	LINESIZE	77	

enum
{
	Sunday,
	Monday,
	Tueday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
};
#define	DAYS_IN_WEEK	7
#define	DFLT_WEEK_END	Saturday

typedef	struct
	{
		long	date;
		float	units,
				ord, hlf, dbl;
		double	lab, oh;

	} SortLine;

	/*====================
	| System Common File |
	====================*/
	struct dbview	comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
	} comm_rec;

	/*=========================================+
	 | cmhr - Contract Management Header File. |
	 +=========================================*/
#define	CMHR_NO_FIELDS	4

	struct dbview	cmhr_list [CMHR_NO_FIELDS] =
	{
		{"cmhr_co_no"},
		{"cmhr_br_no"},
		{"cmhr_cont_no"},
		{"cmhr_hhhr_hash"},
	};

	struct tag_cmhrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	cont_no [7];
		long	hhhr_hash;
	}	cmhr_rec;

	/*===========================================+
	 | Contract Management Time Sheet Trans File |
	 +===========================================*/
#define	CMTS_NO_FIELDS	13

	struct dbview	cmts_list [CMTS_NO_FIELDS] =
	{
		{"cmts_hhem_hash"},
		{"cmts_date"},
		{"cmts_hhhr_hash"},
		{"cmts_hhcm_hash"},
		{"cmts_hheq_hash"},
		{"cmts_time_ord"},
		{"cmts_time_hlf"},
		{"cmts_time_dbl"},
		{"cmts_units"},
		{"cmts_lab_cost"},
		{"cmts_oh_cost"},
		{"cmts_sale"},
		{"cmts_stat_flag"}
	};

	struct tag_cmtsRecord
	{
		long	hhem_hash;
		long	date;
		long	hhhr_hash;
		long	hhcm_hash;
		long	hheq_hash;
		float	time_ord;
		float	time_hlf;
		float	time_dbl;
		float	units;
		double	lab_cost;		/* money */
		double	oh_cost;		/* money */
		double	sale;		/* money */
		char	stat_flag [2];
	}	cmts_rec;


/*===========
 Table names
============*/
static char
	*data	= "data",
	*cmhr	= "cmhr",
	*cmts	= "cmts";

/*=======
 Globals
========*/
	static int	endDay;		/* day on which week ends */
	static char	branchNo [3];

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			(void);
void	CloseDB		(void);
FILE *	InitPrintOut	(int lpno);
void	EndPrintOut		(FILE *pOut);
int		proc_cmhr		(FILE *sortFl);
int		proc_cmts		(FILE *sortFl);
void	ToNextWeekEnd	(long *date);
void	PrintValues		(FILE *pOut, SortLine *vals);
void	ScanLines		(FILE *sortFile, int lpno);


/*==============
 The main thing
================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr = chk_env ("CM_AUTO_CON");
	FILE *	sortFl;

	if (argc < 2 || argc > 3)
	{
/*		printf ("Usage : %s lpno [day]\n", argv [0]);
		puts ("  lpno - printer number");
		puts ("  day  - end day for week (0 - Sun, ..,  6 - Sat) [Default - 6]");*/
		print_at (0,0, ML(mlCmMess024), argv [0]);

		return (EXIT_FAILURE);
	}
	endDay = argc == 3 ? atoi (argv [2]) % DAYS_IN_WEEK : DFLT_WEEK_END;

	/**
		Gonna use a sort file which has the following format

		dddddddddd u.uuu o.ooo h.hhh d.ddd l o

		dddddddddd	:	0 padded julian date
		u.uuu		:	units
		o.ooo		:	ordinary time
		h.hhh		:	time and half
		d.ddd		:	double time
		l		:	labour cost
		o		:	overhead cost
	**/
	if (!(sortFl = sort_open (cmts)))
		sys_err ("!sort_open()", errno, PNAME);

	/*==============================
	 Std db Initializations
	==============================*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	dsp_screen ("Labour Analysis", comm_rec.tco_no, comm_rec.tco_name);

	/*--------------------
	| Check environment. |
	--------------------*/
	strcpy (branchNo,
		(!sptr || atoi (sptr) == COMPANY) ? " 0" : comm_rec.test_no);

	if (proc_cmhr (sortFl))		/* anything? */
	{
		sortFl = sort_sort (sortFl, cmts);
		ScanLines (sortFl, atoi (argv [1]));
	}

	CloseDB (); 
	sort_delete (sortFl, cmts);

	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmts,  cmts_list, CMTS_NO_FIELDS, "cmts_hhhr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (cmhr);
	abc_fclose (cmts);

	abc_dbclose (data);
}

/*====================================
 Printer Initialization & Termination
=====================================*/
FILE *
InitPrintOut (
 int lpno)
{
	FILE *	pOut = popen ("pformat", "w");
	char *	hdrLine = (char *)malloc (LINESIZE + 1U);

	if (!pOut)
		sys_err ("!popen (pformat)", errno, PNAME);

	memset (hdrLine, '=', LINESIZE);
	hdrLine [LINESIZE] = '\0';

	fprintf (pOut, ".START%s\n", DateToString (TodaysDate ()));
	fprintf (pOut, ".LP%d\n", lpno);
	fprintf (pOut, ".12\n");
	fprintf (pOut, ".PI12\n");
	fprintf (pOut, ".L%d\n", LINESIZE);
	fprintf (pOut, ".ELabour Analysis YTD\n");
	fprintf (pOut, ".ECo : %s - %s\n", comm_rec.tco_no, comm_rec.tco_name);
	fprintf (pOut, ".EBr : %s - %s\n", comm_rec.test_no, comm_rec.test_name);
	fprintf (pOut, ".EAs At %s", SystemTime ());
	fprintf (pOut, ".B1\n");
	fprintf (pOut, "%s\n", hdrLine);
	fprintf (pOut, ".R%s\n", hdrLine);

	fprintf (pOut, "|    Week    |            |           HOURS         |         COSTS         |\n");
	fprintf (pOut, "|   Ending   |     Qty    |    1.0t    1.5t    2.0t |     Labour        O/H |\n");
	fprintf (pOut, "+------------+------------+-------------------------+-----------------------+\n");

	free (hdrLine);

	return (pOut);
}

void
EndPrintOut (
 FILE *	pOut)
{
	fprintf (pOut, ".EOF\n");
	pclose (pOut);
}

 
/* Iterate thru' all the contracts for the company
*/
int
proc_cmhr (
 FILE *	sortFl)
{
	int	anything = FALSE;

	memset (&cmhr_rec, 0, sizeof (cmhr_rec));
	strcpy (cmhr_rec.co_no, comm_rec.tco_no);
	strcpy (cmhr_rec.br_no, branchNo);

	cc = find_rec (cmhr, (char *) &cmhr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (cmhr_rec.co_no, comm_rec.tco_no) &&
		!strcmp (cmhr_rec.br_no, branchNo))
	{
		if (proc_cmts (sortFl))
			anything = TRUE;

		cc = find_rec (cmhr, (char *) &cmhr_rec, NEXT, "r");
	}

	return (anything);
}

/* iterate thru' all labour associated with contract
*/
int
proc_cmts (
 FILE *	sortFl)
{
	int		anything = FALSE;
	char	line [256];

	cc = find_hash (cmts, (char *) &cmts_rec, EQUAL, "r", cmhr_rec.hhhr_hash);
	while (!cc && cmts_rec.hhhr_hash == cmhr_rec.hhhr_hash)
	{
		anything = TRUE;

		sprintf (line, "%010ld %.2f %.2f %.2f %.2f %.0f %.0f\n",
			cmts_rec.date,
			cmts_rec.units,
			cmts_rec.time_ord,
			cmts_rec.time_hlf,
			cmts_rec.time_dbl,
			cmts_rec.lab_cost,
			cmts_rec.oh_cost);

		sort_save (sortFl, line);

		cc = find_hash (cmts, (char *) &cmts_rec, NEXT, "r", cmhr_rec.hhhr_hash);
	}
	return (anything);
}

/**	Adds enough days to bring it to the next week ending day
**/
void
ToNextWeekEnd (
 long *	date)
{
	int	startDay = *date % DAYS_IN_WEEK;

	if (startDay <= endDay)
		*date += endDay - startDay;
	else
		*date += DAYS_IN_WEEK - (startDay - endDay);
}

void
PrintValues (
 FILE *		pOut,
 SortLine *	vals)
{
	char	tmpDateStr [11];

	strcpy (tmpDateStr, DateToString (vals -> date));
	dsp_process ("Day", tmpDateStr);
	fprintf (pOut,
			 "| %s | %10.2f | %7.2f %7.2f %7.2f | %10.2f %10.2f |\n",
			 tmpDateStr,
			 vals -> units,
			 vals -> ord,
			 vals -> hlf,
			 vals -> dbl,
			 vals -> lab / 100,
			 vals -> oh / 100);
}

void
ScanLines (
 FILE *	sortFile,
 int	lpno)
{
	FILE *		pOut = InitPrintOut (lpno);
	char *		line;
	SortLine	brkVals;	/* break value accumulators */

	memset (&brkVals, 0, sizeof (brkVals));

	/* Read first line to initialize date values
	*/
	line = sort_read (sortFile);
	sscanf (line, "%ld", &brkVals.date);
	ToNextWeekEnd (&brkVals.date);

	do	{
		SortLine	v;

		sscanf (line, "%ld %f %f %f %f %lf %lf",
			&v.date,
			&v.units,
			&v.ord,
			&v.hlf,
			&v.dbl,
			&v.lab,
			&v.oh);

		if (v.date > brkVals.date)
		{
			/* Print break values
			*/
			PrintValues (pOut, &brkVals);

			while (v.date > (brkVals.date += 7))
			{
				SortLine	zero;

				memset (&zero, 0, sizeof (zero));
				zero.date = brkVals.date;

				PrintValues (pOut, &zero);

			}

			brkVals = v;
			ToNextWeekEnd (&brkVals.date);
		} else
		{
			/* accumulate stuff
			*/
			brkVals.units += v.units;
			brkVals.ord += v.ord;
			brkVals.hlf += v.hlf;
			brkVals.dbl += v.dbl;
			brkVals.lab += (v.ord + 1.5 * v.hlf + 2 * v.dbl) * v.lab;
			brkVals.oh += (v.ord + 1.5 * v.hlf + 2 * v.dbl + v.units) * v.oh;
		}

	}	while ((line = sort_read (sortFile)));

	PrintValues (pOut, &brkVals);	/* print last of the last */

	EndPrintOut (pOut);
}
