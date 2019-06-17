/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindCumr.c,v 5.3 2001/08/28 08:46:08 scott Exp $
-----------------------------------------------------------------------
| $Log: FindCumr.c,v $
| Revision 5.3  2001/08/28 08:46:08  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/06 22:40:51  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
| Revision 5.0  2001/06/19 06:59:11  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.5  2000/10/09 09:46:12  scott
| Updated to add strdup as possible error could occur.
| LS/10 - GUI please note.
|
| Revision 2.4  2000/10/09 04:43:05  scott
| Updated to add strdup, did not cause an error but better safe than sorry
|
| Revision 2.3  2000/09/06 05:21:52  scott
| Updated for spelling, should have been "AND" not "END"
|
| Revision 2.2  2000/09/05 01:50:26  scott
| *** empty log message ***
|
| Revision 2.1  2000/09/05 01:29:47  scott
| Updated from testing while implementing into standard.
|
| Revision 2.0  2000/07/15 07:17:12  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.2  2000/06/16 01:13:13  scott
| New Find Routines
|
| Revision 1.1  2000/06/15 02:29:22 jinno 
| New Function that brings find and searches for customer into library.
| Same features that now exists in stock search routines.
| Functions will be converted to GVision back end for performance.
=====================================================================*/

#include 	<ring_menu.h>
#include	<std_decs.h>

#define     _DB_DESC(i)     _db_curr_menu[i].description
#define     _DB_PRMT(i)     _db_curr_menu[i].prompt
#define     _DB_COLM(i)     _db_curr_menu[i].col

/*------------------------------------------------------------
| Define local structures with names that will not conflict. |
------------------------------------------------------------*/
static	const	char				/*------------------------------*/
	*srdb	=	"_srenvDbFindcumr";   /* Customer Master File			*/
									/*------------------------------*/
	struct	srdbRecord	srdbRec;

									/*------------------------------*/
	extern	char	err_str [];		/* External from pslscr.h		*/
	extern	int		ringClearLine;	/* External from ring_menu.h	*/
	extern	int		_wide;			/* Wide from tcap.h				*/
	extern	int		search_key;		/* External from pslscr.h		*/
	extern	int		last_char;		/* External from pslscr.h		*/
	extern	int		check_search		(char *, char *, int *);
									/*------------------------------*/

	/*===========================+
	 | SeaRch file for Creditor. |
	 +===========================*/
#define	SRDB_NO_FIELDS	8

	static struct dbview	srdb_list [SRDB_NO_FIELDS] =
	{
		{"srdb_co_no"},
		{"srdb_br_no"},
		{"srdb_dbt_no"},
		{"srdb_hhcu_hash"},
		{"srdb_acronym"},
		{"srdb_sman_code"},
		{"srdb_contact_name"},
		{"srdb_name"}
	};

	struct srdbRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dbt_no [7];
		long	hhcu_hash;
		char	acronym [10];
		char	sman_code [3];
		char	contact_name [21];
		char	name [41];
	};


static char	validTypeFlags		[6];	/* Valid search values.				*/
char	cusNumber 				[7],	/* User for input values from ring	*/
		cusName 				[41],	/*                                 	*/
		cusAcronym 				[10],	/*                                 	*/
		cusContact 				[21],	/*                                 	*/
		cusSman 				[3];	/*                                 	*/
static char	branchNumber		[3];	/* Branch Number.					*/
										/*----------------------------------*/
static int	searchFilesOpen	=	FALSE,	/* Set when search fils opened.		*/
		andFlag				=	TRUE,	/* AND OR flag for search.			*/
		envDbtFind			=	0,		/* Environment for customer.		*/
		envDbtCo			=	0,		/* Environment for customer.		*/
		envDbtSearchPopup	=	0;		/* Environment define search popup.	*/
										/*----------------------------------*/
/*=============================
| Search Function prototypes. |
=============================*/
int				CumrFindError 		(char *);
void 			DBSearchInput 		(void);
void			DBSearchFindOpen	(void);
void			DBSearchFindClose	(void);

/*====================
| Main find function. |
=====================*/
int	
FindCumr (						/*--------------------------------------*/
	char	*companyNo,			/* Company number						*/
	char	*branchNo,			/* Branch number						*/
	char	*srchCusNo)			/* Customer number				  		*/
{								/*--------------------------------------*/
	int		recordFound = 0;

	DBSearchFindOpen	();

	strcpy (branchNumber, (envDbtCo) ? branchNo : " 0");

	open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_dbt_id"
													   	   : "srdb_dbt_id2");

	strcpy (srdbRec.co_no, companyNo);
	strcpy (srdbRec.br_no, branchNumber);
	sprintf (srdbRec.dbt_no, "%-6.6s", srchCusNo);
	recordFound = find_rec (srdb, &srdbRec, EQUAL, "r");

	if (recordFound)
	{
		abc_fclose (srdb);
		return (EXIT_FAILURE);
	}
	strcpy (srchCusNo, strdup (srdbRec.dbt_no));
	abc_fclose (srdb);
	return (EXIT_SUCCESS);
}

/*===============================
| Find standard warning message. |
================================*/
int
CumrFindError (
 char*  message)
{
	int	i;

	for (i = 0; i < 5; i++)
	{
		errmess (message);
		sleep (2);
	}

	return (EXIT_FAILURE);
}

/*==================================
| Search for Customer master file.  |
===================================*/
void
CumrSearch (
	char	*companyNo,
	char	*branchNo,
	char	*searchValue)
{

	int		noSearch	=	0,
			noFind 		= 	1;
	int		cc			=	0,
			valid 		= 	1,
			breakOut	=	0;

	char	selectTypeFlag [2];
	char	*sptr = (*searchValue == '*') ? (char *)0 : searchValue;

	DBSearchFindOpen ();

	strcpy (branchNumber, (envDbtCo) ? branchNo : " 0");

	switch (search_key)
	{
	case	FN4:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags);
		break;

	case	FN9:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 1);
		break;

	case	FN10:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 2);
		break;

	case	FN11:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 3);
		break;

	case	FN12:
		sprintf (selectTypeFlag, "%-1.1s", validTypeFlags + 4);
		break;

	default:
		abc_fclose	(srdb);
		return;
	}

    if (!strlen (searchValue) && envDbtSearchPopup)
	{
		DBSearchInput ();
		sprintf (selectTypeFlag, "T");
	}

	_work_open (8,0,60);

	switch (selectTypeFlag [0])
	{
	case	'N':
	case	'n':
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_dbt_id"
															  : "srdb_dbt_id2");
		save_rec ("#Number","# Customer Name.");
		break;

	case	'A':
	case	'a':
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_acr_id"
															  : "srdb_acr_id2");
		save_rec ("#Number","# Acronym   Customer Name.");
		break;

	case	'C':
	case	'c':
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_cnt_id"
															  : "srdb_cnt_id2");
		save_rec ("#Number","#    Contact Name.     Customer Name.");
		break;

	case	'S':
	case	's':
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_sal_id"
															  : "srdb_sal_id2");
		save_rec ("#Number","#Sman Code Customer Name.");
		break;

	case	'D':
	case	'd':
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_des_id"
															  : "srdb_des_id2");
		save_rec ("#Number","# Customer Name.");
		break;

	default:
		open_rec (srdb, srdb_list, SRDB_NO_FIELDS, (envDbtFind) ? "srdb_dbt_id"
															  : "srdb_dbt_id2");
		save_rec ("#Number","# Customer Name.");
		break;
	}

	strcpy (srdbRec.co_no, companyNo);
	strcpy (srdbRec.br_no, branchNumber);
	sprintf (srdbRec.dbt_no,      "%-6.6s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srdbRec.acronym,     "%-9.9s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srdbRec.sman_code,   "%-2.2s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srdbRec.contact_name,"%-20.20s",(sptr != (char *)0) ? sptr : " ");
	sprintf (srdbRec.name,        "%-40.40s",(sptr != (char *)0) ? sptr : " ");

	cc = find_rec (srdb, &srdbRec, GTEQ, "r");
	while (!cc && !strcmp (srdbRec.co_no, companyNo))
	{
		/*--------------------------------------------
		| If Customer Branch Owned && Correct Branch. |
		--------------------------------------------*/
		if (!envDbtFind && strcmp (srdbRec.br_no, branchNumber))
			break;
		
		switch (selectTypeFlag [0])
		{
		case	'N':
		case	'n':
			valid = check_search (srdbRec.dbt_no, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;

		case	'A':
		case	'a':
			valid = check_search (srdbRec.acronym, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srdbRec.acronym,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;

		case	'S':
		case	's':
			valid = check_search (srdbRec.sman_code, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srdbRec.sman_code,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;

		case	'C':
		case	'c':
			valid = check_search (srdbRec.contact_name, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srdbRec.contact_name,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;

		case	'D':
		case	'd':
			valid = check_search (srdbRec.name, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;

		case	'T':
		case	't':

			noSearch    =   0;
			noFind      =   0;

			if (strlen (cusNumber))
			{                 
				valid = check_search (srdbRec.dbt_no, cusNumber, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
					sprintf (err_str, "NUMB-> %s", srdbRec.name);
			}                    

			if (strlen (cusName))
			{                 
				valid = check_search (srdbRec.name, cusName, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
					sprintf (err_str, "NAME-> %s", srdbRec.name);
			}                    

			if (strlen (cusAcronym))
			{                 
				valid = check_search (srdbRec.acronym, cusAcronym, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "ACRO-> (%s) %-30.30s", srdbRec.acronym, srdbRec.name);
				}
			}                    

			if (strlen (cusContact))
			{                 
				valid = check_search (srdbRec.contact_name, cusContact, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "CONT-> (%s) %-30.30s", srdbRec.contact_name, srdbRec.name);
				}
			}                    

			if (strlen (cusSman))
			{                 
				valid = check_search (srdbRec.sman_code, cusSman, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "SMAN-> (%s) %-30.30s", srdbRec.sman_code, srdbRec.name);
				}
			}                    
			valid = FALSE;
			if (andFlag == TRUE && noSearch > 0 && (noSearch == noFind))
				valid = TRUE;
			if (andFlag == FALSE && noFind > 0)
				valid = TRUE;

			break;

		default:
			valid = check_search (srdbRec.dbt_no, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srdbRec.name);
			break;
		}

		if (valid)
		{
			cc = save_rec (srdbRec.dbt_no, err_str);
			if (cc)
				break;
		}
		else
		{
			if (breakOut && selectTypeFlag [0] != 'T')
				break;
		}

		cc = find_rec (srdb,&srdbRec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&srdbRec, 0, sizeof (srdbRec));
		abc_fclose	(srdb);
		return;
	}
	abc_selfield (srdb, (envDbtFind) ? "srdb_dbt_id" : "srdb_dbt_id2");

	strcpy (srdbRec.co_no, companyNo);
	strcpy (srdbRec.br_no, branchNumber);
	sprintf (srdbRec.dbt_no, "%-6.6s", searchValue);
	cc = find_rec (srdb, &srdbRec, COMPARISON, "r");
	if (cc)
		file_err (cc, "srdb", "DBFIND");

	strcpy (searchValue, strdup (srdbRec.dbt_no));
	abc_fclose	(srdb);
}

void
DBSearchFindOpen (void)
{
	char 	*sptr;

	if (searchFilesOpen	== TRUE)
		return;

	sptr		=	chk_env ("DB_FIND");
	envDbtFind	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	sptr		=	chk_env ("DB_CO");
	envDbtCo	=	(sptr == (char *)0) ? 0 : atoi (sptr);

	sptr		=	chk_env ("DB_SER");
	if (sptr ==  (char *)0)
		sprintf (validTypeFlags, "%-5.5s", "ANTCD");
	else
		sprintf (validTypeFlags, "%-5.5s", sptr);

	sptr		=	chk_env ("DB_SEARCH_POPUP");
	envDbtSearchPopup		=	(sptr == (char *)0) ? 0 : atoi (sptr);

	abc_alias (srdb, "srdb");

	searchFilesOpen = TRUE;
}


int		srchCusNumberSelect			(void);
int		srchCusNameSelect			(void);
int		srchCusAcronSelect			(void);
int		srchCusContNameSelect		(void);
int		srchCusSmanSelect			(void);
int		DBAndOrFunc					(void);
char	DBsrchFiller[41]	=	"No value entered for selected field.   ";
char	DBAndOrFiller[11]	=	"[AND SRCH]";
void	DBsrchClearDesc		(void);


menu_type	_db_curr_menu [] =
{
	{"Number",	 DBsrchFiller, srchCusNumberSelect,		 "Nn", },
	{"Name",	 DBsrchFiller, srchCusNameSelect,		 "Nn", },
	{"Acronym",	 DBsrchFiller, srchCusAcronSelect,		 "Aa", },
	{"Contact",	 DBsrchFiller, srchCusContNameSelect,	 "Cc", },
	{"Salesman", DBsrchFiller, srchCusSmanSelect,		 "Ss", },
	{DBAndOrFiller,	 "",	   DBAndOrFunc,				 "OoAa", },
	{"RESTART",		 "",	   _no_option, "",		 FN1, EXIT | SELECT},
	{"EDIT/END",	 "",	   _no_option, "", 		 FN16, EXIT | SELECT},
	{"", },
};

/*==========================
| Main Processing Routine. |
==========================*/
void
DBSearchInput (void)
{
	int		i;

	for (i = 0; i < 5; i++)
		sprintf (_DB_DESC (i), "%-40.40s", strdup (ML(DBsrchFiller)));

	sprintf (_DB_PRMT (5), strdup ("[AND SRCH]"));

	andFlag			=	TRUE;

	strcpy (cusNumber, ""); 
	strcpy (cusName, "");
	strcpy (cusAcronym, "");
	strcpy (cusContact, "");
	strcpy (cusSman , "");

	ringClearLine	=	FALSE;
	cl_box (0,0,56,2);
	run_menu (_db_curr_menu, "", 1);
	ringClearLine	=	TRUE;
	last_char	=	FN3;
}

/*=======================================================================
| Customer number selected, clear description line, input value etc. 	|
=======================================================================*/
int		
srchCusNumberSelect (void)
{
	DBsrchClearDesc ();
	getalpha (_DB_COLM (0), 2, "UUUUUU", cusNumber);
	if (strlen (cusNumber))
		sprintf (_DB_DESC (0), "%-40.40s", strdup (cusNumber));
	DBsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*===================================================================
| Customer name selected, clear description line, input value etc. 	|
===================================================================*/
int		
srchCusNameSelect (void)
{
	DBsrchClearDesc ();
	getalpha (_DB_COLM (1), 2, "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", cusName);
	if (strlen (cusName))
		sprintf (_DB_DESC (1), "%-40.40s", strdup (cusName));
	DBsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*=======================================================================
| Customer Acronym selected, clear description line, input value etc. 	|
=======================================================================*/
int		
srchCusAcronSelect (void)
{
	DBsrchClearDesc ();
	getalpha (_DB_COLM (2), 2, "UUUUUUUUU", cusAcronym);
	if (strlen (cusAcronym))
		sprintf (_DB_DESC (2), "%-40.40s", strdup (cusAcronym));
	DBsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*===========================================================================
| Customer contact Name selected, clear description line, input value etc. 	|
===========================================================================*/
int		
srchCusContNameSelect (void)
{
	DBsrchClearDesc ();
	getalpha (_DB_COLM (3), 2, "UUUUUUUUUUUUUUUUUUUU", cusContact);
	if (strlen (cusContact))
		sprintf (_DB_DESC (3), "%-40.40s", strdup (cusContact));
	DBsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*============================================================================
| Customer Salesman Code  selected, clear description line, input value etc.  |
=============================================================================*/
int		
srchCusSmanSelect (void)
{
	DBsrchClearDesc ();
	getalpha (_DB_COLM (4), 2, "UUUUUU", cusSman);
	if (strlen (cusSman))
		sprintf (_DB_DESC (4), "%-40.40s", strdup (cusSman));
	DBsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*=======================
| And / Or Function 	|
=======================*/
int
DBAndOrFunc (void)
{
	if (andFlag	== FALSE)
	{
		sprintf (_DB_PRMT (5), strdup ("[AND SRCH]"));
		andFlag	= TRUE;
	}
	else
	{
		sprintf (_DB_PRMT (5), strdup ("[OR  SRCH]"));
		andFlag	= FALSE;
	}
	rv_pr (_DB_PRMT (5), _DB_COLM (5), 1,1);
	return (EXIT_SUCCESS);
}

/*===================
| Clear Function 	|
===================*/
void
DBsrchClearDesc (void)
{
	print_at (2,1, "%54.54s", " ");
}
