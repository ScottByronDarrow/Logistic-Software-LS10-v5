/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (sa_close.c    )                                   |
|  Program Desc  : (Close Sales Analysis File sale.             )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, cusa, esmr, ryhs, sale, sadf          |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cusa,  ryhs, sale, sadf,     ,     ,              |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (13/07/87)      | Modified by : Scott B. Darrow.   |
|                : (07/05/89)      | Modified by : Scott B. Darrow.   |
|                : (07/07/89)      | Modified by : Fui Choo Yap.      |
|                : (13/08/90)      | Modified by : Scott Darrow.      |
|                : (10/04/91)      | Modified by : Scott Darrow.      |
|                : (24/07/91)      | Modified by : Trevor van Bremen  |
|                : (26/11/92)      | Modified by : Anneliese Allen.   |
|                : (10/12/92)      | Modified by : Trevor van Bremen  |
|                : (10/03/93)      | Modified by : Jonathan Chen      |
|                : (14/05/93)      | Modified by : Scott Darrow.      |
|                : (14/05/93)      | Modified by : Roel Michels       |
|                : (22/11/95)      | Modified by : Scott B Darrow.    |
|                : (27/05/96)      | Modified by : Jiggs A Veloz.     |
|                                                                     |
|  Comments      : (07/05/89) - Added delete of sapc at year end.     |
|                : (07/07/89) - Added delete of ryhs at year end.     |
|                : (10/04/91) - Updated to fix S/C ABC-5074           |
|                : (24/07/91) - Allow for separate bonus-stock S/A    |
|                : (26/11/92) - Look up records for deletion/update   | 
|                               via a key. DFT 7755                   |
|     (10/12/92) : After much effort/time, decided to re-order the    |
|                  read and update. DFT 7755.                         |
|     (10/03/93) : PSL 8620 - Removed use of 'isrecnum' and replaced  |
|                  it with call to abc_rowid ()                       |
|     (14/05/93) : Updated to fix S/C OCT-8639. cusa record 'L'       |
|                : being initilised wrong.                            |
|     (12/04/94) : PSL 10673 - Online conversion                      |
|                :                                                    |
|     (22/11/95) : PDL Updated to fix compile error on SCO.           |
|                :                                                    |
|     (27/05/96) : Updated to fix problems in DateToString.           |
|                                                                     |
| $Log: sa_close.c,v $
| Revision 5.1  2001/08/09 09:16:49  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:13:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/12 07:47:04  cha
| Updated call to COMPARISON on sale2
|
| Revision 4.0  2001/03/09 02:34:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:53  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/01 02:28:07  scott
| Updated to use app.schema
| Updated to remove use of abc_rowid as not compatable with GVision and SQL.
| General clean up of code.
|
| Revision 2.0  2000/07/15 09:09:22  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/03/25 06:37:31  gerry
| Replace call to _open_rec with open_rec for Oracle compatibility
|
| Revision 1.14  1999/12/06 01:35:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/16 04:55:31  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.12  1999/10/16 01:11:20  nz
| Updated for pjulmdy routines
|
| Revision 1.11  1999/09/29 10:12:45  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 07:27:31  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.9  1999/09/17 00:17:11  scott
| SC1824 Removed deletion and updating of inmu.
|
| Revision 1.8  1999/09/16 02:01:51  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:19  scott
| Updated for read_comm (), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_close.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_close/sa_close.c,v 5.1 2001/08/09 09:16:49 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define	CO_DBT		 (envVarCoClose [0] == '1')
#define	HeldOverPeriod	12

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cusaRecord	cusa_rec;
struct esmrRecord	esmr_rec;
struct ryhsRecord	ryhs_rec;
struct saleRecord	sale_rec;
struct saleRecord	sale2_rec;
struct sadfRecord	sadf_rec;
struct sadfRecord	sadf2_rec;

Money	*cusa_val	=	&cusa_rec.val1;
float	*sadf_qty 	=	&sadf_rec.qty_per1;
double	*sadf_sal 	=	&sadf_rec.sal_per1;
double	*sadf_cst 	=	&sadf_rec.cst_per1;
float	*sadf2_qty 	=	&sadf2_rec.qty_per1;
double	*sadf2_sal 	=	&sadf2_rec.sal_per1;
double	*sadf2_cst 	=	&sadf2_rec.cst_per1;

/*==========
 Table Names
============*/
static char		*data	= "data",
				*sale2	= "sale2",
				*sadf2	= "sadf2";
/*=========
| Globals |
=========*/
	int		mth				 = 0,
			yend		     = 0,
			half_ytd 		 = 0,
			sa_h_ytd 		 = 0,
			envVarSaYend 	 = 0,
			envVarSaSepBonus = FALSE,
			envVarSaProd 	 = 0,
			closeAllBranches = FALSE;

	char	envVarCoClose [6],
			dbt_date [9],
			dsp_mth [3],
			workCoNo [3],
			workBrNo [3];

/*=====================================================================
| Local Function declarations.
=====================================================================*/
void	OpenDB 			(void);
void	CloseDB 		(void);
void	ProcessFile 	(void);
void	ProcessYtdSadf 	(char *, char *);
void	ProcessCurrSadf	(char *, char *);
int		ValidRecord 	(char *, char *, char *);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr = chk_env ("CO_CLOSE");

	if (!sptr)
		sprintf (envVarCoClose,"%-5.5s","11111");
	else
		sprintf (envVarCoClose,"%-5.5s",sptr);

	sptr = chk_env ("SA_PROD");
	if (!sptr)
		envVarSaProd = FALSE;
	else
		envVarSaProd = atoi (sptr);

	sptr = chk_env ("SA_SEP_BONUS");
	if (!sptr)
		envVarSaSepBonus = FALSE;
	else
		envVarSaSepBonus = (*sptr == 'Y' || *sptr == 'y');

	init_scr ();
	clear ();

	OpenDB ();


	sptr = chk_env ("SA_YEND");
	if (!sptr)
		envVarSaYend = comm_rec.fiscal;
	else
		envVarSaYend = atoi (sptr);

	if (envVarSaYend < 1 || envVarSaYend > 12)
		envVarSaYend = comm_rec.fiscal;

	sa_h_ytd = envVarSaYend + 6;
	if (sa_h_ytd > 12)
		sa_h_ytd -= 12;

	DateToDMY (comm_rec.dbt_date, NULL, &mth, NULL);

	if (CO_DBT)
	{
		sprintf (err_str, "Sales analysis %s end close for Company",
			 (mth == envVarSaYend) ? "Year" : "Month");

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		closeAllBranches = TRUE;
	}
	else
	{
		sprintf (err_str, "Sales analysis %s end close for Branch %s - %s",
				 (mth == envVarSaYend) ? "Year" : "Month" ,
				comm_rec.est_no,clip (comm_rec.est_name));

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		closeAllBranches = FALSE;
	}

	if (mth == sa_h_ytd)
		half_ytd = 1;
	else
		half_ytd = 0;

	yend = (mth == envVarSaYend) ? TRUE : FALSE;
	mth++;
	if (mth == 13)
		mth = 1;

	ProcessFile ();
	CloseDB (); 
	FinishProgram ();
	
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (sadf2, sadf);
	abc_alias (sale2, sale);

	open_rec (sale2, sale_list, SALE_NO_FIELDS, "sale_id_no");
	open_rec (sale,  sale_list, SALE_NO_FIELDS, "sale_id_no");
	open_rec (cusa,  cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	open_rec (sadf,  sadf_list, SADF_NO_FIELDS, "sadf_id_no");
	open_rec (sadf2, sadf_list, SADF_NO_FIELDS, "sadf_id_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (ryhs,  ryhs_list, RYHS_NO_FIELDS, "ryhs_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (sale);
	abc_fclose (sale2);
	abc_fclose (sadf);
	abc_fclose (cusa);
	abc_fclose (cumr);
	abc_fclose (ryhs);
	abc_fclose (sadf2);
	abc_fclose (esmr);
	abc_dbclose (data);
}

void
ProcessFile (void)
{
	double	hold_amt = 0.00;
	int		i,
			cusa_hdr = FALSE;

	/*---------------------------------------------------
	| Delete last year sales analysis transactions out. |
	---------------------------------------------------*/
	sprintf (dsp_mth, "%02d", mth);

	sprintf (sale_rec.key, "%2.2s%2.2s    ", comm_rec.co_no, (CO_DBT) ? "  " : comm_rec.est_no);
	sprintf (sale_rec.category, "%11.11s", "  ");
	sprintf (sale_rec.sman, "%2.2s", "  ");
	sprintf (sale_rec.area, "%2.2s", "  ");
	sprintf (sale_rec.ctype, "%3.3s", "  ");
	sprintf (sale_rec.dbt_no, "%6.6s", "  ");
	sprintf (sale_rec.year_flag, "%1.1s", "  ");
	sprintf (sale_rec.period, "%2.2s", "  ");

	cc = find_rec (sale, (char *) &sale_rec, GTEQ, "r");
	while (!cc) 	
	{
		sprintf (workCoNo, "%-2.2s", sale_rec.key);
		sprintf (workBrNo, "%-2.2s", sale_rec.key + 2);

		/*-------------------------------------------------------
		| If company changes OR branch changes AND we are doing	|
		| a branch-level close, break out of the control loop.	|
		-------------------------------------------------------*/
		if (strcmp (workCoNo, comm_rec.co_no) ||
		   (strcmp (workBrNo, comm_rec.est_no) && !CO_DBT))
			break;

		/*-------------------------------------------------------
		| Ignore records which are NOT the correct ones.		|
		| ie: Invalid month OR invalid year.					|
		-------------------------------------------------------*/
		if (strcmp (sale_rec.period, dsp_mth) ||
		    strcmp (sale_rec.year_flag, "L"))
		{
			cc = find_rec (sale, (char *) &sale_rec, NEXT, "r");
			continue;
		}

		dsp_process ("Tran : ", "Delete");

		strcpy (sale2_rec.key, sale_rec.key);
		strcpy (sale2_rec.category, sale_rec.category);
		strcpy (sale2_rec.sman, sale_rec.sman);
		strcpy (sale2_rec.area, sale_rec.area);
		strcpy (sale2_rec.ctype, sale_rec.ctype);
		strcpy (sale2_rec.dbt_no, sale_rec.dbt_no);
		strcpy (sale2_rec.year_flag, sale_rec.year_flag);
		strcpy (sale2_rec.period, sale_rec.period);

		cc = find_rec (sale, (char *) &sale_rec, NEXT, "r");

		i = find_rec (sale2, (char *) &sale2_rec, COMPARISON, "r");
		if (i)
			file_err (i, "sale2", "DBFIND");

		i = abc_delete (sale2);
		if (i)
			file_err (i, "sale2", "DBDELETE");
	}
	/*---------------------------------------------------
	| Update Current year transactions to last year. 	|
	| NB: Need to close and reopen sale2 with no index. |
	---------------------------------------------------*/
	sprintf (sale_rec.key, "%2.2s%2.2s    ", 
					comm_rec.co_no, (CO_DBT) ? "  " : comm_rec.est_no);
	sprintf (sale_rec.category, "%11.11s", "  ");
	sprintf (sale_rec.sman, "%2.2s", "  ");
	sprintf (sale_rec.area, "%2.2s", "  ");
	sprintf (sale_rec.ctype, "%3.3s", "  ");
	sprintf (sale_rec.dbt_no, "%6.6s", "  ");
	sprintf (sale_rec.year_flag, "%1.1s", "  ");
	sprintf (sale_rec.period, "%2.2s", "  ");

	cc = find_rec (sale, (char *) &sale_rec, GTEQ, "r");
	while (!cc) 	
	{
		sprintf (workCoNo, "%-2.2s", sale_rec.key);
		sprintf (workBrNo, "%-2.2s", sale_rec.key + 2);

		/*-------------------------------------------------------
		| If company changes OR branch changes AND we are doing	|
		| a branch-level close, break out of the control loop.	|
		-------------------------------------------------------*/
		if (strcmp (workCoNo, comm_rec.co_no) ||
		   (strcmp (workBrNo, comm_rec.est_no) && !CO_DBT))
			break;

		if (sale_rec.year_flag [0] == 'L')
		{
			cc = find_rec (sale, (char *) &sale_rec, NEXT, "r");
			continue;
		}
		if (!strcmp (sale_rec.period,dsp_mth))
		{
			dsp_process ("Tran : ", "Update");

			/*----------------------------------------------------
			| Updated to remove abc_rowid.						 |
			| As sale_id_no is a unique key we can copy complete |
			| structure over and find using EQUAL.				 |
			----------------------------------------------------*/
			memcpy 
			(
				(char *) &sale2_rec, 
				(char *) &sale_rec, 
				sizeof (struct saleRecord)
			);
			i = find_rec (sale2, &sale2_rec,COMPARISON, "u");
			if (i)
				file_err (i, "sale2", "DBFIND");

			strcpy (sale2_rec.year_flag, "L");
			i = abc_update (sale2, (char *) &sale2_rec);
			if (i)
				file_err (i, "sale2", "DBUPDATE");
		}
		/*---------------------------
		| Process Held Over Period. |
		---------------------------*/
		if (!strcmp (sale_rec.period,"13"))
		{
			dsp_process ("Tran : ", "Update");

			/*----------------------------------------------------
			| Updated to remove abc_rowid.						 |
			| As sale_id_no is a unique key we can copy complete |
			| structure over and find using EQUAL.				 |
			----------------------------------------------------*/
			memcpy 
			(
				(char *) &sale2_rec, 
				(char *) &sale_rec, 
				sizeof (struct saleRecord)
			);
			i = find_rec (sale2, &sale2_rec,COMPARISON, "u");
			if (i)
				file_err (i, "sale2", "DBFIND");

			strcpy (sale2_rec.period, dsp_mth);
			i = abc_update (sale2, (char *) &sale2_rec);
			if (i)
				file_err (i, "sale2", "DBUPDATE");
		}
		cc = find_rec (sale, (char *) &sale_rec, NEXT, "r");
	}
	/*-----------------------------------------------
	| Process Customer History Sales Analysis file. |
	-----------------------------------------------*/
	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no, (CO_DBT) ? "  " : comm_rec.est_no);
	strcpy (cumr_rec.dbt_no,"      ");

	cc = find_rec (cumr, (char *) &cumr_rec, GTEQ, "r");
	while (!cc && ValidRecord (cumr_rec.co_no, comm_rec.est_no,
					    cumr_rec.est_no))
	{
		hold_amt = 0.00;
		cusa_hdr = FALSE;
		
		for (i = 0; i < 13; i ++)
			cusa_val [i] = 0.00;

		/*--------------------------
		| Find Current year record |
		--------------------------*/
		cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cusa_rec.year,"C");
		cc = find_rec (cusa, (char *) &cusa_rec, COMPARISON, "u");
		if (!cc)
		{
			hold_amt 					= cusa_val [mth - 1];
			cusa_val [mth - 1] 			= cusa_val [HeldOverPeriod];
			cusa_val [HeldOverPeriod] 	= 0.00;
		
			cc = abc_update (cusa, (char *) &cusa_rec);
			if (cc)
				file_err (cc, "cusa", "DBUPDATE");
	
			dsp_process ("Customer : ", cumr_rec.dbt_no);

			cusa_hdr = TRUE;
		}
		/*--------------------------------------------
		| Find Last year record if on file else add. |
		--------------------------------------------*/
		cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cusa_rec.year,"L");
		cc = find_rec (cusa, (char *) &cusa_rec, COMPARISON, "u");
		if (cc)
		{
			if (cusa_hdr)
			{
				for (i = 0; i < 13; i ++)
					cusa_val [i] = 0.00;
				
				cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
				strcpy (cusa_rec.year,"L");
				cusa_val [mth - 1] = hold_amt;
				if ((cc = abc_add (cusa, (char *) &cusa_rec)))
					file_err (cc, "cusa", "DBADD");
			}
		}
		else 
		{
			cusa_val [mth - 1] = hold_amt;
			cc = abc_update (cusa, (char *) &cusa_rec);
			if (cc) 
			   file_err (cc, "cusa", "DBUPDATE");
		}
		cc = find_rec (cumr, (char *) &cumr_rec, NEXT, "r");
	}

	if (!envVarSaProd)
		return;
	
	/*------------------------------------------------------------
	| Process detailed sales analysis, clear year to date value. |
	------------------------------------------------------------*/
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, (closeAllBranches) ? "  " : comm_rec.est_no);
	cc = find_rec (esmr, (char *) &esmr_rec, GTEQ, "r");
	while (!cc && ValidRecord (esmr_rec.co_no, comm_rec.est_no,esmr_rec.est_no))
	{
		ProcessYtdSadf (esmr_rec.co_no, esmr_rec.est_no);

		cc = find_rec (esmr, (char *) &esmr_rec, NEXT, "r");
	}
	/*--------------------------------------------------------
	| Process detailed sales analysis, process current year. |
	--------------------------------------------------------*/
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, (closeAllBranches) ? "  " : comm_rec.est_no);
	cc = find_rec (esmr, (char *) &esmr_rec, GTEQ, "r");
	while (!cc && ValidRecord (esmr_rec.co_no, comm_rec.est_no,
				    esmr_rec.est_no))
	{
		ProcessCurrSadf (esmr_rec.co_no, esmr_rec.est_no);

		cc = find_rec (esmr, (char *) &esmr_rec, NEXT, "r");
	}

	/*-------------------------------
	| Update Royalty History	|
	-------------------------------*/
	strcpy (ryhs_rec.co_no,comm_rec.co_no);
	ryhs_rec.hhry_hash = 0L;
	strcpy (ryhs_rec.publish,"    ");
	ryhs_rec.roy_pc = 0.00;
	strcpy (ryhs_rec.roy_basis,"    ");
	cc = find_rec (ryhs, (char *) &ryhs_rec, GTEQ, "u");
	while (!cc && !strcmp (ryhs_rec.co_no,comm_rec.co_no))
	{
		dsp_process ("Royalty/Customer","delete");
		if (!yend)
		{
			ryhs_rec.mtd_gross 	= 0.00;
			ryhs_rec.mtd_nett 	= 0.00;
			ryhs_rec.mtd_roy 	= 0.00;
			ryhs_rec.mtd_qty 	= 0.00;
			if (half_ytd)
			{
				ryhs_rec.htd_gross 	= 0.00;
				ryhs_rec.htd_nett 	= 0.00;
				ryhs_rec.htd_roy 	= 0.00;
				ryhs_rec.htd_qty 	= 0.00;
			}
			cc = abc_update (ryhs, (char *) &ryhs_rec);
			if (cc) 
	    	    		sys_err ("Error in ryhs During (DBUPDATE)",cc,PNAME);
			abc_unlock (ryhs);
			cc = find_rec (ryhs, (char *) &ryhs_rec, NEXT, "u");
		}
		else
		{
			abc_unlock (ryhs);
			abc_delete (ryhs);
			cc = find_rec (ryhs, (char *) &ryhs_rec, GTEQ, "u");
		}
	}
	abc_unlock (ryhs);
}

int
ValidRecord (
	char	*_co_no,
	char	*_br_no,
	char	*_act_br_no)
{
	if (!closeAllBranches &&
			!strcmp (_co_no, comm_rec.co_no) && 
			!strcmp (_br_no, _act_br_no))
		return (TRUE);

	if (closeAllBranches && !strcmp (_co_no, comm_rec.co_no))
		return (TRUE);

	return (FALSE);
}
/*=============================================
| Process detailed sales analysis, Last year. |
=============================================*/
void
ProcessYtdSadf (
	char*  co_no,
	char*  br_no)
{
	strcpy (sadf_rec.co_no, co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "L");
	strcpy (sadf_rec.sman, "  ");
	strcpy (sadf_rec.area, "  ");
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;

	cc = find_rec (sadf, (char *) &sadf_rec, GTEQ, "u");
	while (!cc && !strcmp (sadf_rec.co_no, co_no) &&
	              !strcmp (sadf_rec.br_no, br_no) &&
		      sadf_rec.year [0] == 'L')
	{
		sadf_qty [mth - 1] = 0.00;
		sadf_sal [mth - 1] = 0.00;
		sadf_cst [mth - 1] = 0.00;

		cc = abc_update (sadf, (char *) &sadf_rec);
		if (cc)
			file_err (cc, "sadf", "DBUPDATE");

		cc = find_rec (sadf, (char *) &sadf_rec, NEXT, "u");
	}

	if (!envVarSaSepBonus)
		return;

	strcpy (sadf_rec.co_no, co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "l");
	strcpy (sadf_rec.sman, "  ");
	strcpy (sadf_rec.area, "  ");
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;

	cc = find_rec (sadf, (char *) &sadf_rec, GTEQ, "u");
	while (!cc && !strcmp (sadf_rec.co_no, co_no) &&
	              !strcmp (sadf_rec.br_no, br_no) &&
		      sadf_rec.year [0] == 'l')
	{
		sadf_qty [mth - 1] = 0.00;
		sadf_sal [mth - 1] = 0.00;
		sadf_cst [mth - 1] = 0.00;

		cc = abc_update (sadf, (char *) &sadf_rec);
		if (cc)
			file_err (cc, "sadf", "DBUPDATE");

		cc = find_rec (sadf, (char *) &sadf_rec, NEXT, "u");
	}
}

/*================================================
| Process detailed sales analysis, Current year. |
================================================*/
void
ProcessCurrSadf (
	char*  co_no,
	char*  br_no)
{
	int	i;

	strcpy (sadf_rec.co_no, co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "C");
	strcpy (sadf_rec.sman, "  ");
	strcpy (sadf_rec.area, "  ");
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;

	cc = find_rec (sadf, (char *) &sadf_rec, GTEQ, "u");
	while (!cc && !strcmp (sadf_rec.co_no, co_no) &&
	              !strcmp (sadf_rec.br_no, br_no) &&
		      sadf_rec.year [0] == 'C')
	{
		sadf2_rec = sadf_rec;

		strcpy (sadf2_rec.year, "L");
		
		cc = find_rec (sadf2, (char *) &sadf2_rec, COMPARISON, "u");
		if (cc)
		{
			if (sadf_qty [mth - 1] != 0.00 ||
			    sadf_sal [mth - 1] != 0.00 ||
			    sadf_cst [mth - 1] != 0.00)
			{
				for (i = 0; i < 13; i++)
				{
					sadf2_qty [i] = 0.00;
					sadf2_sal [i] = 0.00;
					sadf2_cst [i] = 0.00;
				}
				sadf2_qty [mth - 1] = sadf_qty [mth - 1];
				sadf2_sal [mth - 1] = sadf_sal [mth - 1];
				sadf2_cst [mth - 1] = sadf_cst [mth - 1];
				if ((cc = abc_add (sadf2, (char *) &sadf2_rec)))
					file_err (cc, "sadf2", "DBADD");
			}
			abc_unlock (sadf2);
		}
		else
		{
			sadf2_qty [mth - 1] = sadf_qty [mth - 1];
			sadf2_sal [mth - 1] = sadf_sal [mth - 1];
			sadf2_cst [mth - 1] = sadf_cst [mth - 1];
			cc = abc_update (sadf2, (char *) &sadf2_rec);
			if (cc)
				file_err (cc, "sadf2", "DBUPDATE");
		
		}
		sadf_qty [mth - 1] 			=  sadf_qty [HeldOverPeriod];
		sadf_sal [mth - 1] 			=  sadf_sal [HeldOverPeriod];
		sadf_cst [mth - 1] 			=  sadf_cst [HeldOverPeriod];
		sadf_qty [HeldOverPeriod]	=  0.00;
		sadf_sal [HeldOverPeriod]	=  0.00;
		sadf_cst [HeldOverPeriod]	=  0.00;
		cc = abc_update (sadf, (char *) &sadf_rec);
		if (cc)
			file_err (cc, "sadf", "DBUPDATE");

		cc = find_rec (sadf, (char *) &sadf_rec, NEXT, "u");
	}

	if (!envVarSaSepBonus)
		return;

	strcpy (sadf_rec.co_no, co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "c");
	strcpy (sadf_rec.sman, "  ");
	strcpy (sadf_rec.area, "  ");
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;

	cc = find_rec (sadf, (char *) &sadf_rec, GTEQ, "u");
	while (!cc && !strcmp (sadf_rec.co_no, co_no) &&
	              !strcmp (sadf_rec.br_no, br_no) &&
		      sadf_rec.year [0] == 'c')
	{
		sadf2_rec = sadf_rec;

		strcpy (sadf2_rec.year, "l");
		
		cc = find_rec (sadf2, (char *) &sadf2_rec, COMPARISON, "u");
		if (cc)
		{
			if (sadf_qty [mth - 1] != 0.00 ||
			    sadf_sal [mth - 1] != 0.00 ||
			    sadf_cst [mth - 1] != 0.00)
			{
				for (i = 0; i < 13; i++)
				{
					sadf2_qty [i] = 0.00;
					sadf2_sal [i] = 0.00;
					sadf2_cst [i] = 0.00;
				}
				sadf2_qty [mth - 1] = sadf_qty [mth - 1];
				sadf2_sal [mth - 1] = sadf_sal [mth - 1];
				sadf2_cst [mth - 1] = sadf_cst [mth - 1];
				cc = abc_add (sadf2, &sadf2_rec);
				if (cc)
					file_err (cc, "sadf2", "DBADD");
			}
			abc_unlock (sadf2);
		}
		else
		{
			sadf2_qty [mth - 1] = sadf_qty [mth - 1];
			sadf2_sal [mth - 1] = sadf_sal [mth - 1];
			sadf2_cst [mth - 1] = sadf_cst [mth - 1];
			cc = abc_update (sadf2, (char *) &sadf2_rec);
			if (cc)
				file_err (cc, "sadf2", "DBUPDATE");
		
		}
		sadf_qty [mth - 1] 			=  sadf_qty [HeldOverPeriod];
		sadf_sal [mth - 1] 			=  sadf_sal [HeldOverPeriod];
		sadf_cst [mth - 1] 			=  sadf_cst [HeldOverPeriod];
		sadf_qty [HeldOverPeriod]	=	0.00;
		sadf_sal [HeldOverPeriod]	=	0.00;
		sadf_cst [HeldOverPeriod]	=	0.00;
		cc = abc_update (sadf, (char *) &sadf_rec);
		if (cc)
			file_err (cc, "sadf", "DBUPDATE");

		cc = find_rec (sadf, (char *) &sadf_rec, NEXT, "u");
	}
}
