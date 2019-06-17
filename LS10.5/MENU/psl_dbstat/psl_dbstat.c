/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_dbstat.c   )                                 |
|  Program Desc  : ( Logistic database Table Statistic report.    )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Access files  :  systables, syscolumns, sysindexes,                |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (01/11/93)      | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
|  Date Modified : (03/09/97)      | Modified  by : Ana Marie Tario   |
|  Date Modified : (03/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments    : (03/09/97)   : Incorporated multilingual conversion  |
|              :              : and DMY4 date.                        |
|              : (03/09/1999) : Ported to ANSI standards.             |
|              :                                                      |
|              :                                                      |
| $Log: psl_dbstat.c,v $
| Revision 5.2  2001/08/09 05:13:41  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:33  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:15  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:00:25  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/02/18 01:56:27  scott
| Updated to fix small warnings found when compiled under Linux
|
| Revision 1.12  1999/12/06 01:47:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/25 10:24:00  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.10  1999/09/29 10:11:13  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 07:27:05  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.8  1999/09/16 04:11:41  scott
| Updated from Ansi Project
|
| Revision 1.7  1999/06/15 02:36:53  scott
| Update to add log + change database names + misc clean up.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: psl_dbstat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_dbstat/psl_dbstat.c,v 5.2 2001/08/09 05:13:41 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<ml_menu_mess.h>
#include	<ml_std_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef	LINUX
#include	<sys/ustat.h>
#else
#include	<ustat.h>
#endif	/* LINUX */

	/*=======================================
	| systables - System Tables File	|
	=======================================*/
	struct dbview tab_list [] =
	{
		{"tabname"},
		{"owner"},
		{"dirpath"},
		{"tabid"},
		{"rowsize"},
		{"ncols"},
		{"nindexes"},
		{"nrows"},
		{"created"},
		{"version"},
		{"tabtype"},
		{"audpath"}
	};

	int		tab_no_fields = 12;

	struct
	{
		char	tabname [19];
		char	owner [9];
		char	dirpath [65];
		long	tabid;
		int		rowsize;
		int		ncols;
		int		nindexes;
		long	nrows;
		long	created;
		long	version;
		char	tabtype [2];
		char	audpath [65];
	} tab2_rec, tab_rec;

	/*=======================================
	| syscolumns - System Columns File	|
	=======================================*/
	struct dbview col_list [] =
	{
		{"colname"},
		{"tabid"},
		{"colno"},
		{"coltype"},
		{"collength"}
	};

	int		col_no_fields = 5;

	struct
	{
		char	colname [19];
		long	tabid;
		int		colno;
		int		coltype;
		int		collength;
	} col_rec;

#define	DAT	0			/* xxx.dat	*/
#define	IDX	1			/* xxx.idx	*/
#define	DSK	2			/* Check df	*/

FILE *	fout;
		
long	size_actual,
		size_theory,
		size_each,
		size_bytes;

float	tot_megs [3];

int		lpno = 1;

extern	char *	_dbpath;

extern	int		optind;		/* For getopt() routine	*/
extern	char *	optarg;		/* For getopt() routine	*/

/*============================
| Local function prototypes  |
============================*/
void	OpenDB		(void);
void	rep_heading	(void);
void	CloseDB	(void);
void	process		(char *);
int		check		(char *);
long	get_actual	(int);


int
main (
 int	argc,
 char *	argv [])
{
	if ( argc < 2 )
	{
		print_at(0,0,mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}

	tot_megs[ 0 ] = 0.00;
	tot_megs[ 1 ] = 0.00;
	tot_megs[ 2 ] = 0.00;

	lpno = atoi( argv[1] );

	dsp_screen( "Logistic Database Statistics reporting. ", "  ", "Software Engineering" );

	OpenDB ();

	rep_heading ();

	sprintf (tab2_rec.tabname, "%-18.18s", " ");
	sprintf (tab2_rec.owner, "%-8.8s", " ");
	cc = find_rec ("systab2", &tab2_rec, GTEQ, "r");
	if (cc)
		sys_err ("Error in systab2 During (DBFIND)", cc, PNAME);

	while (!cc)
	{
		/*--------------------------------
		| Don't process SQL system files |
		--------------------------------*/
		if (tab2_rec.tabid >= 100L)
			process (clip (tab2_rec.tabname));
		cc = find_rec ("systab2", &tab2_rec, NEXT, "r");
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("systables", tab_list, tab_no_fields, "tabname");
	open_rec ("syscolumns", col_list, col_no_fields, "column");
	abc_alias ("systab2", "systables");
	open_rec ("systab2", tab_list, tab_no_fields, "tabname");
}

void
rep_heading (
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L130\n");
	fprintf (fout, ".ELOGISTIC DATABASE TABLE STATISTICS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n", SystemTime());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".R================================");
	fprintf (fout, "===================");
	fprintf (fout, "===================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "================================");
	fprintf (fout, "===================");
	fprintf (fout, "===================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "|  FILE  |  TABLE INFORMATION   ");
	fprintf (fout, "|   OLD DATA SIZE   ");
	fprintf (fout, "|   NEW DATA SIZE   ");
	fprintf (fout, "|      POSSIBLE SAVING        |\n");

	fprintf (fout, "|  NAME  | NO RECS | BYTES EACH ");
	fprintf (fout, "|   BYTES   | MEGS  ");
	fprintf (fout, "|   BYTES   | MEGS  ");
	fprintf (fout, "|   BYTES   | MEGS  | PERCENT |\n");

	fprintf (fout, "|--------|---------|------------");
	fprintf (fout, "|-----------|-------");
	fprintf (fout, "|-----------|-------");
	fprintf (fout, "|-----------|-------|---------|\n");
}

void
CloseDB (
 void)
{
	fprintf (fout, "|========|=========|============");
	fprintf (fout, "|===========|=======");
	fprintf (fout, "|===========|=======");
	fprintf (fout, "|===========|=======|=========|\n");

	fprintf (fout, "|        |         |            ");
	fprintf (fout, "|           |%6.2f ", tot_megs[0] );
	fprintf (fout, "|           |%6.2f ", tot_megs[1] );
	fprintf (fout, "|           |%6.2f |         |\n", tot_megs[2] );
	fprintf (fout, ".EOF\n");
	pclose (fout);
	
	abc_fclose ("systab2");
	abc_fclose ("systables");
	abc_fclose ("syscolumns");
	abc_dbclose ("data");
}

void
process (
 char *	table)
{
	float	per_save;

	if (check (table))
	{
		if ( size_each == 0L )
			return;

		if ( size_actual > 0L )
			per_save = (float)((((float) size_actual - (float) size_theory ) / (float) size_actual) * 100.00);
		else
			per_save = 0.00;

		fprintf (fout, "|  %-4.4s  |%8ld | %10ld |%10ld |%6.2f |%10ld |%6.2f |%10ld |%6.2f | %6.2f%% |\n",
				table,
				size_each,
				size_bytes,
				size_actual,
				(float) size_actual / 1024000,
				size_theory,
				(float) size_theory / 1024000,
				size_actual - size_theory,
				(float) (size_actual - size_theory) / 1024000,
				per_save );

		tot_megs[ 0 ] += (float) size_actual / 1024000,
		tot_megs[ 1 ] += (float) size_theory / 1024000,
		tot_megs[ 2 ] += (float) (size_actual - size_theory) / 1024000,

		dsp_process( "File ", table );
	}
}

int
check (
 char *	table)
{
	long size_total;

	/*------------------------------------------------------
	| Read in the systables column for the selected table. |
	| If it doesn't exist, return with 0.                  |
	------------------------------------------------------*/
	sprintf (tab_rec.tabname, "%-18.18s", table);
	sprintf (tab_rec.owner, "%-8.8s", " ");
	cc = find_rec ("systables", &tab_rec, GTEQ, "r");
	if (cc || strcmp (table, clip (tab_rec.tabname)))
		return ( FALSE );

	size_actual = get_actual (DAT);
	size_theory = (tab_rec.nrows * (tab_rec.rowsize + 1));
	size_each 	= tab_rec.nrows;
	size_bytes  = tab_rec.rowsize + 1;

	/*----------------------------------------------------
	| Make sure that there is enough space for 2 copies. |
	----------------------------------------------------*/
	size_total = size_actual + get_actual (IDX);
	size_total = (size_total + 511L) / 512L;
	return ( TRUE );
}

long
get_actual (
 int ext)
{
	char	basename [128];
	char	filename [128];
	int		isfd;
	struct	dictinfo	info;
	struct	stat stbuf;
	struct	ustat ustbuf;

	if (tab_rec.dirpath[0] != '/')
		sprintf (basename, "%s/%s", _dbpath, clip (tab_rec.dirpath));
	else
		sprintf (basename, "%s", clip (tab_rec.dirpath));

	sprintf (filename, "%s.%s", basename, (ext == DAT) ? "dat" : "idx");

	if (stat (filename, &stbuf))
	{
		sprintf (err_str, "Error in %s During (STAT)", filename);
		sys_err (err_str, errno, PNAME);
	}

	if (ext == DAT)
	{
		isfd = isopen (basename, ISINPUT + ISMANULOCK);
		if (isfd >= 0)
		{
			if (isindexinfo (isfd, (struct keydesc *) &info, 0) == 0)
				tab_rec.nrows = info.di_nrecords;
			isclose (isfd);
		}
	}

	if (ext == DSK)
	{
		if (ustat (stbuf.st_dev, &ustbuf))
		{
			sprintf (err_str, "Error in %s During (USTAT)", filename);
			sys_err (err_str, errno, PNAME);
		}
		return ((long) ustbuf.f_tfree);
	}

	return ((long) stbuf.st_size);
}
