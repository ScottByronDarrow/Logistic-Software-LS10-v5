/*=====================================================================
|  Copyright (C) 1986 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( psl_dbstat.c   )                                 |
|  Program Desc  : ( Database Statistics reporting.               )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (01/11/94)      | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: psl_dbstat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_dbstat/psl_dbstat.c,v 5.2 2001/08/09 09:27:25 scott Exp $";

#define		NO_SCRGEN
#include	<ml_std_mess.h>
#include	<pslscr.h>
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
	struct dbview tab_list[] =
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
		char	tabname[19];
		char	owner[9];
		char	dirpath[65];
		long	tabid;
		int		rowsize;
		int		ncols;
		int		nindexes;
		long	nrows;
		long	created;
		long	version;
		char	tabtype[2];
		char	audpath[65];
	} tab2_rec, tab_rec;

	/*=======================================
	| syscolumns - System Columns File	|
	=======================================*/
	struct dbview col_list[] =
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
		char	colname[19];
		long	tabid;
		int		colno;
		int		coltype;
		int		collength;
	} col_rec;

#define	DAT	0			/* xxx.dat	*/
#define	IDX	1			/* xxx.idx	*/
#define	DSK	2			/* Check df	*/

FILE	*fout;
		
long	size_actual,
		size_theory,
		size_each,
		size_bytes;

float	tot_megs[3];

int		lpno = 1;

extern	char	*_dbpath;

extern	int		optind;			/* For getopt() routine	*/
extern	char	*optarg;		/* For getopt() routine	*/

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void rep_heading (void);
void CloseDB (void);
void process (char *table);
int check (char *table);
long get_actual (int ext);

int
main (
 int                argc,
 char*              argv[])
{
	if ( argc < 2 )
	{
		/*printf("Usage %s <LPNO>\n", argv[0] );*/
		printf(ML(mlStdMess036), argv[0] );
		return (EXIT_FAILURE);
	}

	tot_megs[ 0 ] = 0.00;
	tot_megs[ 1 ] = 0.00;
	tot_megs[ 2 ] = 0.00;

	lpno = atoi ( argv[1] );

	dsp_screen ( "Logistic Database Statistics reporting. ", "  ", "Logistic" );

	OpenDB ();

	rep_heading();

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
OpenDB (void)
{
	abc_dbopen ("data");
	open_rec ("systables", tab_list, tab_no_fields, "tabname");
	open_rec ("syscolumns", col_list, col_no_fields, "column");
	abc_alias ("systab2", "systables");
	open_rec ("systab2", tab_list, tab_no_fields, "tabname");
}

void
rep_heading (void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString(TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", lpno);
	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L130\n");
	fprintf (fout, ".ELOGISTIC DATABASE TABLE STATISTICS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n", ttod());
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
CloseDB (void)
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
 char*              table)
{
	float	per_save;

	if ( check (table) )
	{
		if ( size_each == 0L )
			return;

		if ( size_actual > 0L )
			per_save = (float)( ( (float) size_actual - (float) size_theory ) / (float) size_actual ) * (float) 100.00;
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
 char*              table)
{
	long	size_total;

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
 int                ext)
{
	char	basename[128];
	char	filename[128];
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
