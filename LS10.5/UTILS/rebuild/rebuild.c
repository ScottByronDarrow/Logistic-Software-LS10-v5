/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( rebuild.c      )                                 |
|  Program Desc  : ( SQL rebuild script generator.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  systables, syscolumns, sysindexes,                |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (07/08/90)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (13/01/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (31/03/93)      | Modified  by : Trevor van Bremen |
|  Date Modified : (12/09/97)      | Modified  by : Roanna Marcelino  |
|                                                                     |
|  Comments      :                                                    |
|  (13/01/93)    : Changed to bypass any further need for ISQL.       |
|                : DFT 8024.                                          |
|  (31/03/93)    : Changed to FORCE unique id of OLD onto NEW.        |
|                : PSL 5784.                                          |
|  (12/09/97)    : Modified for Multilingual Conversion.              |
|                                                                     |
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: rebuild.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/rebuild/rebuild.c,v 5.2 2001/08/09 09:27:35 scott Exp $";

#include	<pslscr.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef	LINUX
#include	<sys/ustat.h>
#else
#include	<ustat.h>
#endif	/* LINUX */
#include	<ml_std_mess.h>
#include	<ml_utils_mess.h>

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

	int	tab_no_fields = 12;

	struct
	{
		char	tabname[19];
		char	owner[9];
		char	dirpath[65];
		long	tabid;
		int	rowsize;
		int	ncols;
		int	nindexes;
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

	int	col_no_fields = 5;

	struct
	{
		char	colname[19];
		long	tabid;
		int	colno;
		int	coltype;
		int	collength;
	} col_rec;

#define	NEEDED	1			/* Rebuild deemd necessary.	*/
#define	NOT_NEC	2			/* Rebuild unnecessary.		*/
#define	NO_DSK	3			/* Insufficient disk available	*/

#define	DAT	0			/* xxx.dat	*/
#define	IDX	1			/* xxx.idx	*/
#define	DSK	2			/* Check df	*/

FILE *fout, *f_logging;
int	all_files = FALSE,
	condtnl = FALSE,
	pr_log = FALSE,
	pr_lno = 4;
double	min_save;			/* Minimum %age saving rqd	*/
long	size_actual,
	size_theory;

extern	char	*_dbpath;

extern	int	optind;			/* For getopt() routine	*/
extern	char	*optarg;		/* For getopt() routine	*/


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
void process (char *table);
int  check (char *table);
long get_actual (int ext);
int  rebuild (char *table);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	loop;

	/*------------------------
	| Check for a/c/p switch |
	------------------------*/
	while ((cc = getopt (argc, argv, "acp")) != EOF)
	{
		switch (cc)
		{
		case	'a':
			all_files = TRUE;
			break;

		case	'c':
			condtnl = TRUE;
			break;

		case	'p':
			pr_log = TRUE;
			break;

		case	'?':
			/*Usage : %s -[acp] <file_name...>\007\n", argv[0]);
			            -a  All files in the database.\n");
			printf ("   -c  Conditional rebuild.\n");
			printf ("   -p  Print log\n");*/

			print_at (0,0,ML(mlUtilsMess725), argv[0]);
			print_at (0,0,ML(mlUtilsMess726));
			print_at (0,0,ML(mlUtilsMess727));
			print_at (0,0,ML(mlUtilsMess728));
			return (EXIT_FAILURE);
		}
	}
	if (optind > argc - 1 && all_files == FALSE)
	{
		/*rintf ("Usage : %s -[acp] <file_name...>\007\n", argv[0]);
		printf ("   -a  All files in the database.\n");
		printf ("   -c  Conditional rebuild.\n");
		printf ("   -p  Print log\n");*/

		print_at (0,0,ML(mlUtilsMess725), argv[0]);
		print_at (0,0,ML(mlUtilsMess726));
		print_at (0,0,ML(mlUtilsMess727));
		print_at (0,0,ML(mlUtilsMess728));
		return (EXIT_FAILURE);
	}

	init_scr ();
	clear ();

	OpenDB ();

	if (all_files)
	{
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
	}
	else
	{
		for (loop = optind; loop < argc; loop++)
			process (argv[loop]);
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
	if (all_files)
	{
		abc_alias ("systab2", "systables");
		open_rec ("systab2", tab_list, tab_no_fields, "tabname");
	}
	if (condtnl)
	{
		min_save = atof (get_env ("MIN_SAVE"));
		min_save += 100.00;
		min_save /= 100.00;
	}
	if (pr_log)
	{
		if ((f_logging = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);
		fprintf (f_logging, ".START %-8.8s\n", ttod ());
		fprintf (f_logging, ".LP1\n");
		fprintf (f_logging, ".PI12\n");
		fprintf (f_logging, ".3\n");
		fprintf (f_logging, "================================================================================\n");
		fprintf (f_logging, "| FILE | OLD DATA SIZE | NEW DATA SIZE | REBUILT | SPACE SAVED |     ERRORS    |\n");
		fprintf (f_logging, "|------|---------------|---------------|---------|-------------|---------------|\n");
	}
}

void
CloseDB (
 void)
{
	if (pr_log)
	{
		fprintf (f_logging, "================================================================================\n");
		fprintf (f_logging, ".EOF\n");
		pclose (f_logging);
	}
	if (all_files)
		abc_fclose ("systab2");
	abc_fclose ("systables");
	abc_fclose ("syscolumns");
	abc_dbclose ("data");
}

void
process (
 char *table)
{
	sprintf (err_str,ML(mlUtilsMess125), table);
	rv_pr (err_str, 0, 2, 1);
	switch (check (table))
	{
	case	NEEDED:
		cc = rebuild (table);
		if (pr_log)
		{
			if (pr_lno > 58)
			{
				fprintf (f_logging, "================================================================================\n");
				fprintf (f_logging, ".PA\n");
				pr_lno = 4;
			}
			pr_lno++;
			fprintf (f_logging, "| %-4.4s |    %10ld |    %10ld |  YES    |  %10ld | %-11.11s   |\n",
				table,
				size_actual,
				size_theory,
				size_actual - size_theory,
				(cc == 0) ? "SUCESSFUL" : "UNSUCESSFUL");
		}
		else
		{
			/* Rebuild on %s was %sucessful\007",
					 table, (cc == 0) ? "S" : "Uns");*/
			if (cc == 0)
				sprintf (err_str,ML(mlUtilsMess126), table);
			else
				sprintf (err_str,ML(mlUtilsMess127), table);
	
			rv_pr (err_str, 0, 3, 1);
			print_at (0,0,"\n\n");
		}
		break;

	case	NOT_NEC:
		if (pr_log)
		{
			if (pr_lno > 58)
			{
				fprintf (f_logging, "================================================================================\n");
				fprintf (f_logging, ".PA\n");
				pr_lno = 4;
			}
			pr_lno++;
			fprintf (f_logging, "| %-4.4s |    %10ld |    %10ld |  NO     |  %10ld | < THRESHOLD   |\n",
				table,
				size_actual,
				size_theory,
				0L);
		}
		else
		{
			/* Rebuild on %s was deemed to be unneccessary*/
			sprintf (err_str,ML(mlUtilsMess128), table);
			rv_pr (err_str, 0, 3, 1);
			print_at (0,0,"\n\n");
		}
		break;

	case	NO_DSK:
		if (pr_log)
		{
			if (pr_lno > 58)
			{
				fprintf (f_logging, "================================================================================\n");
				fprintf (f_logging, ".PA\n");
				pr_lno = 4;
			}
			pr_lno++;
			fprintf (f_logging, "| %-4.4s |    %10ld |    %10ld |  NO     |  %10ld | INSUF. DISK   |\n",
				table,
				size_actual,
				size_theory,
				0L);
		}
		else
		{
			/* Insufficient disk space available to rebuild %s*/
			sprintf (err_str,ML(mlUtilsMess129), table);
			rv_pr (err_str, 0, 3, 1);
			print_at (0,0,"\n\n");
		}
		break;

	default:
		/*File: %s - Doesn't exist in the database*/
		sprintf (err_str,ML(mlUtilsMess130), table);
		rv_pr (err_str, 0, 3, 1);
		print_at (0,0,"\n\n");
		break;
	}
}

int
check (
 char *table)
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
		return (EXIT_SUCCESS);

	/*----------------------------------
	| Check if we are going to recover |
	| at least min_save %.             |
	----------------------------------*/
	size_actual = get_actual (DAT);
	size_theory = (tab_rec.nrows * (tab_rec.rowsize + 1));
	if (size_actual <= (size_theory * min_save) && condtnl)
		return (NOT_NEC);

	/*----------------------------------------------------
	| Make sure that there is enough space for 2 copies. |
	----------------------------------------------------*/
	size_total = size_actual + get_actual (IDX);
	size_total = (size_total + 511L) / 512L;
	if (size_total > get_actual (DSK))
		return (NO_DSK);

	/*---------------------------------------
	| MUST be necessary if we reached here.	|
	---------------------------------------*/
	return (NEEDED);
}

long	
get_actual (
 int ext)
{
	char	basename[128];
	char	filename[128];
	int	isfd;
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

/*===============================================
| This performs the ACTUAL rebuild of the table	|
| Returns:					|
|	0	- All went OK, table rebuilt.	|
|	Other	- Something went wrong.		|
===============================================*/
int	
rebuild (
 char *table)
{
	char	*sptr,
		old_name[128],
		new_name[128];
	int	i,
		old_fd,
		new_fd;
	long	uniq_id;
	struct	dictinfo	info;
	struct	keydesc		key;

	if (tab_rec.dirpath[0] != '/')
		sprintf (old_name, "%s/%s", _dbpath, clip (tab_rec.dirpath));
	else
		sprintf (old_name, "%s", clip (tab_rec.dirpath));
	strcpy (new_name, old_name);
	if (new_name[strlen (new_name) - 1] == '0')
		new_name[strlen (new_name) - 1] = '1';
	else
		new_name[strlen (new_name) - 1] = '0';

	old_fd = isopen (old_name, ISINOUT + ISEXCLLOCK);
	if (old_fd < 0)
		return (-1);
	if (isindexinfo (old_fd, (struct keydesc *) &info, 0) < 0)
	{
		isclose (old_fd);
		return (-1);
	}
	if (isindexinfo (old_fd, &key, 1) < 0)
	{
		isclose (old_fd);
		return (-1);
	}
	isuniqueid (old_fd, &uniq_id);
	iserase (new_name);
	new_fd = isbuild (new_name, info.di_recsize, &key, ISINOUT + ISEXCLLOCK);
	if (new_fd < 0)
	{
		isclose (old_fd);
		return (-1);
	}
	for (i = 2; i <= info.di_nkeys; i++)
	{
		if (isindexinfo (old_fd, &key, i) < 0)
		{
			isclose (old_fd);
			isclose (new_fd);
			iserase (new_name);
			return (-1);
		}
		if (isaddindex (new_fd, &key) < 0)
		{
			isclose (old_fd);
			isclose (new_fd);
			iserase (new_name);
			return (-1);
		}
	}
	issetunique (new_fd, uniq_id + 1);
	sptr = (char *) malloc (info.di_recsize);
	if (sptr == (char *) 0)
	{
		isclose (old_fd);
		isclose (new_fd);
		iserase (new_name);
		return (-1);
	}
	i = isread (old_fd, sptr, ISFIRST);
	while (i == 0)
	{
		if (iswrite (new_fd, sptr) < 0)
		{
			isclose (old_fd);
			isclose (new_fd);
			iserase (new_name);
			return (-1);
		}
		i = isread (old_fd, sptr, ISNEXT);
		if (i != 0 && iserrno != EENDFILE)
		{
			isclose (old_fd);
			isclose (new_fd);
			iserase (new_name);
			return (-1);
		}
	}

	isclose (old_fd);
	isclose (new_fd);

	iserase (old_name);
	isrename (new_name, old_name);

	return (EXIT_SUCCESS);
}
