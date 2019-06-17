/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: FindSumr.c,v 5.3 2001/08/28 08:46:09 scott Exp $
-----------------------------------------------------------------------
| $Log: FindSumr.c,v $
| Revision 5.3  2001/08/28 08:46:09  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/06 22:40:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 00:43:27  scott
| New library for 10.5
|
| Revision 5.0  2001/06/19 06:59:12  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:52:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:34:17  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.4  2000/10/09 09:46:13  scott
| Updated to add strdup as possible error could occur.
| LS/10 - GUI please note.
|
| Revision 2.3  2000/10/09 04:43:06  scott
| Updated to add strdup, did not cause an error but better safe than sorry
|
| Revision 2.2  2000/09/06 09:42:52  ramon
| Updated to correct the spelling, should have been "AND" and not "END".
|
| Revision 2.1  2000/09/06 05:51:09  scott
| Updated to apply changes found with FindCumr.c to FindSumr.c
|
| Revision 2.0  2000/07/15 07:17:12  gerry
| Forced revision no. to 2.0 - Rel-15072000
|
| Revision 1.4  2000/06/16 05:55:12  scott
| Changes word NAME to NUMB.
|
| Revision 1.3  2000/06/16 01:13:14  scott
| New Find Routines
|
| Revision 1.1  2000/06/15 05:07:02  jinno
| New Function that brings find and searches for supplier into library.
| Same features that now exists in stock search routines.
| Functions will be converted to GVision back end for performance.
=====================================================================*/

#include 	<ring_menu.h>
#include	<std_decs.h>

#define		_CR_DESC(i)		_cr_curr_menu[i].description 
#define		_CR_PRMT(i)		_cr_curr_menu[i].prompt 
#define		_CR_COLM(i)		_cr_curr_menu[i].col 

/*------------------------------------------------------------
| Define local structures with names that will not conflict. |
------------------------------------------------------------*/
static	const	char				/*------------------------------*/
	*srcr	=	"_srcr_findsumr";   /* Supplier Master File			*/
									/*------------------------------*/
	struct	srcrRecord	srcrRec;

									/*------------------------------*/
	extern	char	err_str [];		/* External from pslscr.h		*/
	extern	int		ringClearLine;	/* External from ring_menu.h	*/
	extern	int		_wide;			/* Wide from tcap.h				*/
	extern	int		search_key;		/* External from pslscr.h		*/
	extern	int		last_char;		/* External from pslscr.h		*/
	extern	int		check_search		 (char *, char *, int *);
									/*------------------------------*/

	/*===========================+
	 | SeaRch file for Creditor. |
	 +===========================*/
#define	SRCR_NO_FIELDS	8

	static struct dbview	srcr_list [SRCR_NO_FIELDS] =
	{
		{"srcr_co_no"},
		{"srcr_br_no"},
		{"srcr_crd_no"},
		{"srcr_hhsu_hash"},
		{"srcr_acronym"},
		{"srcr_type_code"},
		{"srcr_contact_name"},
		{"srcr_name"}
	};

	struct srcrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	crd_no [7];
		long	hhsu_hash;
		char	acronym [10];
		char	type_code [7];
		char	contact_name [21];
		char	name [41];
	};

										/*----------------------------------*/
static	char	validTypeFlags	 [6];	/* Valid search values.				*/
char	supNumber 				 [7],	/* User for input values from ring	*/
		supName 				 [41],	/*                                 	*/
		supAcronym 				 [10],	/*                                 	*/
		supContact 				 [21],	/*                                 	*/
		supType 				 [7];	/*                                 	*/
static	char	branchNumber	 [3];	/* Branch number.					*/
										/*----------------------------------*/
static 	int	searchFilesOpen	=	FALSE,	/* Set when search fils opened.		*/
		andFlag				=	TRUE,	/* AND OR flag for search.			*/
		envCrFind			=	0,		/* Environment for Supplier     	*/
		envCrCo				=	0,		/* Environment for supplier    		*/
		envCrSearchPopup	=	0;		/* Environment define search popup.	*/
										/*----------------------------------*/
/*=============================
| Search Function prototypes. |
=============================*/
int		SumrFindError 		 (char *);
void 	CRSearchInput 		 (void);
void	CRSearchFindOpen	 (void);
void	CRSearchFindClose 	 (void);

/*====================
| Main find function. |
=====================*/
int	
FindSumr (						/*--------------------------------------*/
	char	*companyNo,			/* Company number						*/
	char	*branchNo,			/* Branch number						*/
	char	*srchSupNo)			/* Supplier number				  		*/
{								/*--------------------------------------*/
	int		recordFound = 0;

	CRSearchFindOpen	 ();

	strcpy (branchNumber, (envCrCo) ? branchNo : " 0");

	open_rec (srcr, srcr_list, SRCR_NO_FIELDS, (envCrFind) ? "srcr_crd_id"
														   : "srcr_crd_id2");

	strcpy (srcrRec.co_no, companyNo);
	strcpy (srcrRec.br_no, branchNumber);
	sprintf (srcrRec.crd_no, "%-6.6s", srchSupNo);
	recordFound = find_rec (srcr, &srcrRec, EQUAL, "r");

	if (recordFound)
	{
		abc_fclose (srcr);
		return (EXIT_FAILURE);
	}
	strcpy (srchSupNo, strdup (srcrRec.crd_no));
	abc_fclose (srcr);
	return (EXIT_SUCCESS);
}

/*===============================
| Find standard warning message. |
================================*/
int
SumrFindError (
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
| Search for Supplier master file. |
==================================*/
void
SumrSearch (
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

	CRSearchFindOpen ();

	strcpy (branchNumber, (envCrCo) ? branchNo : " 0");

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
		abc_fclose	 (srcr);
		return;
	}

    if (!strlen (searchValue) && envCrSearchPopup)
	{
		CRSearchInput ();
		sprintf (selectTypeFlag, "S");
	}

	_work_open (6,0,60);

	switch (selectTypeFlag [0])
	{
	case	'N':
	case	'n':
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_crd_id"
													   	      : "srcr_crd_id2");
		save_rec ("#Number","# Supplier Name.");
		break;

	case	'A':
	case	'a':
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_acr_id"
													   	      : "srcr_acr_id2");
		save_rec ("#Number","# Acronym   Supplier Name.");
		break;

	case	'C':
	case	'c':
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_cnt_id"
													   	      : "srcr_cnt_id2");
		save_rec ("#Number","#    Contact Name.     Supplier Name.");
		break;

	case	'T':
	case	't':
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_sal_id"
													   	      : "srcr_sal_id2");
		save_rec ("#Number","#Type Code Supplier Name.");
		break;

	case	'D':
	case	'd':
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_des_id"
													   	      : "srcr_des_id2");
		save_rec ("#Number","# Supplier Name.");
		break;

	default:
		open_rec (srcr, srcr_list,SRCR_NO_FIELDS, (envCrFind) ? "srcr_crd_id"
													   	      : "srcr_crd_id2");
		save_rec ("#Number","# Supplier Name.");
		break;
	}

	strcpy (srcrRec.co_no, companyNo);
	strcpy (srcrRec.br_no, branchNumber);
	sprintf (srcrRec.crd_no,      "%-6.6s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srcrRec.acronym,     "%-9.9s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srcrRec.type_code,   "%-6.6s",  (sptr != (char *)0) ? sptr : " ");
	sprintf (srcrRec.contact_name,"%-20.20s",(sptr != (char *)0) ? sptr : " ");
	sprintf (srcrRec.name,        "%-30.30s",(sptr != (char *)0) ? sptr : " ");

	cc = find_rec (srcr, &srcrRec, GTEQ, "r");
	while (!cc && !strcmp (srcrRec.co_no, companyNo))
	{
		/*--------------------------------------------
		| If Customer Branch Owned && Correct Branch. |
		--------------------------------------------*/
		if (!envCrFind && strcmp (srcrRec.br_no, branchNumber))
			break;
		
		switch (selectTypeFlag [0])
		{
		case	'N':
		case	'n':
			valid = check_search (srcrRec.crd_no, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;

		case	'A':
		case	'a':
			valid = check_search (srcrRec.acronym, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srcrRec.acronym,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;

		case	'T':
		case	't':
			valid = check_search (srcrRec.type_code, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srcrRec.type_code,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;

		case	'C':
		case	'c':
			valid = check_search (srcrRec.contact_name, searchValue, &breakOut);
			sprintf (err_str, "(%s) %-*.*s",
					srcrRec.contact_name,
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;

		case	'D':
		case	'd':
			valid = check_search (srcrRec.name, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;

		case	'S':
		case	's':

			noSearch    =   0;
			noFind      =   0;

			if (strlen (supNumber))
			{                 
				valid = check_search (srcrRec.crd_no, supNumber, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
					sprintf (err_str, "NUMB-> %s", srcrRec.name);
			}                    

			if (strlen (supName))
			{                 
				valid = check_search (srcrRec.name, supName, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
					sprintf (err_str, "NAME-> %s", srcrRec.name);
			}                    

			if (strlen (supAcronym))
			{                 
				valid = check_search (srcrRec.acronym, supAcronym, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "ACRO-> (%s) %-30.30s", srcrRec.acronym, srcrRec.name);
				}
			}                    

			if (strlen (supContact))
			{                 
				valid = check_search (srcrRec.contact_name, supContact, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "CONT-> (%s) %-30.30s", srcrRec.contact_name, srcrRec.name);
				}
			}                    

			if (strlen (supType))
			{                 
				valid = check_search (srcrRec.type_code, supType, &breakOut);
				noSearch++;
				noFind  += valid;
				if (valid)
				{
					sprintf (err_str, "TYPE-> (%s) %-30.30s", srcrRec.type_code, srcrRec.name);
				}
			}                    
			valid = FALSE;
			if (andFlag == TRUE && noSearch > 0 && (noSearch == noFind))
				valid = TRUE;
			if (andFlag == FALSE && noFind > 0)
				valid = TRUE;

			break;

		default:
			valid = check_search (srcrRec.crd_no, searchValue, &breakOut);
			sprintf (err_str, " %-*.*s",
					(_wide) ? 40 : 36,
					(_wide) ? 40 : 36,
					srcrRec.name);
			break;
		}

		if (valid)
		{
			cc = save_rec (srcrRec.crd_no, err_str);
			if (cc)
				break;
		}
		else
		{
			if (breakOut && selectTypeFlag [0] != 'S')
				break;
		}

		cc = find_rec (srcr,&srcrRec,NEXT,"r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		memset (&srcrRec, 0, sizeof (srcrRec));
		abc_fclose (srcr);
		return;
	}
	abc_selfield (srcr, (envCrFind) ? "srcr_crd_id" : "srcr_crd_id2");

	strcpy (srcrRec.co_no, companyNo);
	strcpy (srcrRec.br_no, branchNumber);
	sprintf (srcrRec.crd_no, "%-6.6s", searchValue);
	cc = find_rec (srcr, &srcrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, "srcr", "DBFIND");

	strcpy (searchValue, strdup (srcrRec.crd_no));
	abc_fclose (srcr);
}

void
CRSearchFindOpen (void)
{
	char	*sptr;

	if (searchFilesOpen	== TRUE)
		return;

	sptr		=	chk_env ("CR_FIND");
	envCrFind	=	 (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr		=	chk_env ("CR_CO");
	envCrCo		=	 (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr		=	chk_env ("CR_SER");
	if (sptr  == (char *)0)
		sprintf (validTypeFlags, "%-5.5s", "ANTCD");
	else
		sprintf (validTypeFlags, "%-5.5s", sptr);

	sptr	=	chk_env ("CR_SEARCH_POPUP");
	envCrSearchPopup	=	 (sptr == (char *)0) ? 0 : atoi (sptr);

	abc_alias (srcr, "srcr");

	searchFilesOpen = TRUE;
}


int		srchSupNumberSelect	 		(void);
int		srchSupNameSelect	 		(void);
int		srchSupAcronSelect		 	(void);
int		srchSupContNameSelect	 	(void);
int		srchSupTypeSelect	 		(void);
int		CRAndOrFunc		    		(void);
char	CRsrchFiller [41]	=	"No value entered for selected field.   ";
char	CRAndOrFiller [12]	=	"[AND SRCH]";
void	CRsrchClearDesc		 (void);


menu_type	_cr_curr_menu[] =
{
	{"Number", 	CRsrchFiller, srchSupNumberSelect, 		"Nn", },
	{"Name", 	CRsrchFiller, srchSupNameSelect,		"Nn", },
	{"Acronym",	CRsrchFiller, srchSupAcronSelect, 		"Aa", },
	{"Contact", CRsrchFiller, srchSupContNameSelect, 	"Cc", },
	{"Type", 	CRsrchFiller, srchSupTypeSelect, 		"Tt", },
	{CRAndOrFiller,	 "", 	  CRAndOrFunc, 				"oOaA", },
	{"RESTART",		 "", 	  _no_option, "",		  FN1, EXIT | SELECT},
	{"EDIT/END",	 "", 	  _no_option, "", 		  FN16, EXIT | SELECT},
	{"", },
};

/*==========================
| Main Processing Routine. |
==========================*/
void
CRSearchInput (void)
{
	int		i;

	for (i = 0; i < 5; i++)
		sprintf (_CR_DESC (i), "%-40.40s", strdup (ML (CRsrchFiller)));

	sprintf (_CR_PRMT (5), strdup ("[AND SRCH]"));

	andFlag			=	TRUE;

	strcpy (supNumber, ""); 
	strcpy (supName, "");
	strcpy (supAcronym, "");
	strcpy (supContact, "");
	strcpy (supType , "");

	ringClearLine	=	FALSE;
	cl_box (0,0,56,2);
	run_menu (_cr_curr_menu, "", 1);
	ringClearLine	=	TRUE;
	last_char	=	FN3;
}

/*=======================================================================
| Supplier number selected, clear description line, input value etc. 	|
=======================================================================*/
int		
srchSupNumberSelect (void)
{
	CRsrchClearDesc ();
	getalpha (_CR_COLM (0), 2, "UUUUUU", supNumber);
	if (strlen (supNumber))
		sprintf (_CR_DESC (0), "%-40.40s", strdup (supNumber));
	CRsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*===================================================================
| Supplier name selected, clear description line, input value etc. 	|
===================================================================*/
int		
srchSupNameSelect (void)
{
	CRsrchClearDesc ();
	getalpha (_CR_COLM (1), 2, "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", supName);
	if (strlen (supName))
		sprintf (_CR_DESC (1), "%-40.40s", strdup (supName));
	CRsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*=======================================================================
| Supplier Acronym selected, clear description line, input value etc. 	|
=======================================================================*/
int		
srchSupAcronSelect (void)
{
	CRsrchClearDesc ();
	getalpha (_CR_COLM (2), 2, "UUUUUUUUU", supAcronym);
	if (strlen (supAcronym))
		sprintf (_CR_DESC (2), "%-40.40s", strdup (supAcronym));
	CRsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*===========================================================================
| Supplier contact Name selected, clear description line, input value etc. 	|
===========================================================================*/
int		
srchSupContNameSelect (void)
{
	CRsrchClearDesc ();
	getalpha (_CR_COLM (3), 2, "UUUUUUUUUUUUUUUUUUUU", supContact);
	if (strlen (supContact))
		sprintf (_CR_DESC (3), "%-40.40s", strdup (supContact));
	CRsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*===========================================================================
| Supplier Type Code  selected, clear description line, input value etc. 	|
===========================================================================*/
int		
srchSupTypeSelect (void)
{
	CRsrchClearDesc ();
	getalpha (_CR_COLM (4), 2, "UUUUUU", supType);
	if (strlen (supType))
		sprintf (_CR_DESC (4), "%-40.40s", strdup (supType));
	CRsrchClearDesc ();
	return (EXIT_SUCCESS);
}

/*=======================
| And / Or Function 	|
=======================*/
int
CRAndOrFunc (void)
{
	if (andFlag	== FALSE)
	{
		sprintf (_CR_PRMT (5), strdup ("[AND SRCH]"));
		andFlag	= TRUE;
	}
	else
	{
		sprintf (_CR_PRMT (5), strdup ("[OR  SRCH]"));
		andFlag	= FALSE;
	}
	rv_pr (_CR_PRMT (5), _CR_COLM (5), 1,1);
	return (EXIT_SUCCESS);
}

/*===================
| Clear Function 	|
===================*/
void
CRsrchClearDesc (void)
{
	print_at (2,1, "%54.54s", " ");
}
