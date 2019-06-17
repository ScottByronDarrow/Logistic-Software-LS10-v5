/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_inal_mnt.c,v 5.7 2002/07/24 08:39:13 scott Exp $
|  Program Name  : (sk_inal_mnt.c)
|  Program Desc  : (Inventory advertising Levy Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : (19th Apr 2001)  |
|---------------------------------------------------------------------|
| $Log: sk_inal_mnt.c,v $
| Revision 5.7  2002/07/24 08:39:13  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/06/20 07:10:55  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/01/30 07:14:09  robert
| SC 00720 - Updated to fix box alignment on LS10-GUI
|
| Revision 5.3  2001/08/09 09:18:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:57  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_inal_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inal_mnt/sk_inal_mnt.c,v 5.7 2002/07/24 08:39:13 scott Exp $";

#define		TABLINES	8

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct inalRecord	inal_rec;
struct inalRecord	inal2_rec;
struct inalRecord	inal3_rec;
struct inmrRecord	inmr_rec;
struct esmrRecord	esmr_rec;
struct pocrRecord	pocrRec;

	char	*data = "data",
			*inal2 = "inal2",
			*inal3 = "inal3";

	extern	int		TruePosition;
	extern	int		stopDateSearch;

   	int		newCode		= FALSE,
			dspOpen		= FALSE,
			envDbCo		= 0,
			foundInal	= FALSE,
			displayScn2	= FALSE;

	char	systemDate [11],
			branchNumber [3],
			envCurrCode [4];

	struct storeRec {
		char	currencyStore [4];
	} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy 		 [11];
	char	itemNumber 	 [sizeof inmr_rec.item_no];
	char	branchNo 	 [3];
	char	branchName 	 [41];
	char	currencyCode [4];
	Date	dateFrom;
	Date	dateTo;
	Money	levyValue;
	float	levyPercent;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNumber",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number     ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "itemDescription",	 4, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description     ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{1, LIN, "branchNo",	6, 2, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Branch          ", "Enter Branch Number. Default For All Branches.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.branchNo},
	{1, LIN, "branchName",	6, 50, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.branchName},
	{1, LIN, "dateFrom",	8, 2, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", systemDate, "Date From       ", "Search will find valid records.",
		 NE, NO,  JUSTLEFT, "", "", (char *)&local_rec.dateFrom},
	{1, LIN, "dateTo",	8, 50, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", "00/00/00", "Date To         ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.dateTo},
	{2, TAB, "currencyCode",	 MAXLINES, 3, CHARTYPE,
		"UUU", "        ",
		" ", " ", "Currency", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.currencyCode},
	{2, TAB, "currencyDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "        ",
		" ", " ", "Currency Description.                  ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{2, TAB, "levyValue",	0, 2, DOUBLETYPE,
		"NNNNNN.NNNN", "          ",
		" ", "0", "  Levy Value   ", "Note : Set levy value and percent to zero to delete record.",
		YES, NO, JUSTRIGHT, "0", "9999999.9999", (char *)&inal_rec.value},
	{2, TAB, "levyPercent",	0, 4, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Levy Percent", "Note : Set levy value and percent to zero to delete record.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *)&inal_rec.percent},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
int  	CheckCurrCode 		 (char *);
int  	CheckDates 			 (long, long, long);
int  	CheckOverlap 		 (int, char *);
int  	DeleteLine 			 (void);
int  	heading 			 (int);
int  	spec_valid 			 (int);
int  	ValidMcurrItem 		 (void);
void 	CloseDB 			 (void);
void	DisplayRecords		 (void);
void 	OpenDB 				 (void);
void 	shutdown_prog 		 (void);
void	SrchEsmr 			 (char *);
void	SrchInal 			 (void);
void 	SrchPocr 			 (char *);
void 	Update 				 (void);

/*==========================
| Main processing routine. |
==========================*/
int
main (
	int		argc,
	char 	*argv [])
{
	TruePosition	=	TRUE;
	stopDateSearch	=	TRUE;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);


	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));

	strcpy (systemDate, DateToString (TodaysDate ()));

	envDbCo 	= atoi (get_env ("DB_CO"));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);
	tab_row = 10;
	tab_col = 28;

	OpenDB ();

	strcpy (branchNumber, (!envDbCo) ? " 0" : comm_rec.est_no);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		abc_unlock (inal);
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		search_ok  = TRUE;
		displayScn2	=	FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		if (dspOpen)
		{
			dspOpen	=	FALSE;
			Dsp_close ();
		}

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------
		| Choose currency. |
		------------------*/
		if (!foundInal)
		{
			heading (2);
			entry (2);
			if (restart)
				continue;
		}

		displayScn2	=	TRUE;
		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		edit_all ();

		if (restart)
			continue;

		Update ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
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

	abc_alias (inal2, inal);
	abc_alias (inal3, inal);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inal,  inal_list, INAL_NO_FIELDS, "inal_id_no");
	open_rec (inal2, inal_list, INAL_NO_FIELDS, "inal_id_no");
	open_rec (inal3, inal_list, INAL_NO_FIELDS, "inal_id_no2");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (esmr);
	abc_fclose (inal);
	abc_fclose (inal2);
	abc_fclose (inal3);
	abc_fclose (pocr);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i;
	int		chkLines;
	char	wkFrom [11];
	char	wkTo [11];
	int		noConflict;

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("itemNumber"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.itemNumber,inmr_rec.item_no);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDescription");

		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Branch Number. |
	-------------------------*/
	if (LCHECK ("branchNo"))
	{

		if (dflt_used)
		{
			strcpy (local_rec.branchNo, "  ");
			strcpy (local_rec.branchName, ML ("All Branches"));
			DSP_FLD ("branchName");
			DisplayRecords ();
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.branchNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.branchName, esmr_rec.est_name);
		DSP_FLD ("branchName");
		DisplayRecords ();
		return (EXIT_SUCCESS);
	}
	/*---------------------
	| Validate date from. |
	---------------------*/
	if (LCHECK ("dateFrom"))
	{
		if (dspOpen)
		{
			dspOpen	=	FALSE;
			Dsp_close ();
		}
		if (SRCH_KEY)
		{
			SrchInal ();
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && local_rec.dateTo < local_rec.dateFrom)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		newCode = FALSE;

#ifndef	GVISION
		cl_box (1,9,23,10);
		erase_box (1,9,23,10);
		move (1,9);  line (26);
#endif
		return (ValidMcurrItem ());
	}
		
	/*-------------------
	| Validate date to. |
	-------------------*/
	if (LCHECK ("dateTo"))
	{
		if (local_rec.dateTo < local_rec.dateFrom)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("currencyCode"))
	{
		if (FLD ("currencyCode") == ND)
			return (EXIT_SUCCESS);

		if (prog_status == ENTRY && last_char == FN16)
		{
			blank_display ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used || last_char == DELLINE)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!CheckCurrCode (local_rec.currencyCode))
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*--------------------
		| Already in table ? |
		--------------------*/
		chkLines = (prog_status == ENTRY) ? line_cnt : lcount [2];
		for (i = 0; i < chkLines; i++)
		{
			if (!strcmp (store [i].currencyStore, local_rec.currencyCode)) 
			{
				/*-----------------------------------------------------------
				| if currency found on a current line then return as it 	|
				| will not be on any other lines and also we do NOT want	|
				| to check overlap											|
				-----------------------------------------------------------*/
			    if (i == line_cnt)
					return (EXIT_SUCCESS);

				print_mess (ML ("Levy already specified for this currency"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*--------------------
		| Check for overlap. |
		--------------------*/
		noConflict = TRUE;

		strcpy (inal2_rec.br_no, local_rec.branchNo);
		sprintf (inal2_rec.curr_code, local_rec.currencyCode);
		inal2_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inal2_rec.date_from = 0L;
	
		cc = find_rec (inal2, &inal2_rec, GTEQ, "r");
		while (!cc && 
		       !strcmp (inal2_rec.br_no, local_rec.branchNo) &&
		       !strcmp (inal2_rec.curr_code, local_rec.currencyCode) &&
		       inal2_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			/*---------------------------
			| Check for current record. |
			---------------------------*/
			if (!newCode &&
			    inal2_rec.date_from == local_rec.dateFrom)
			{
				cc = find_rec (inal2, &inal2_rec, NEXT, "r");
				continue;
			}
	
			if (local_rec.dateFrom >= inal2_rec.date_from &&
			    local_rec.dateFrom <= inal2_rec.date_to)
			{
				noConflict = FALSE;
				break;
			}
			
			if (local_rec.dateTo >= inal2_rec.date_from &&
			    local_rec.dateTo <= inal2_rec.date_to)
			{
				noConflict = FALSE;
				break;
			}
	
			if (inal2_rec.date_from >= local_rec.dateFrom &&
				inal2_rec.date_from <= local_rec.dateTo)
			{
				noConflict = FALSE;
				break;
			}
	
			if (inal2_rec.date_to >= local_rec.dateFrom &&
				inal2_rec.date_to <= local_rec.dateTo)
			{
				noConflict = FALSE;
				break;
			}
	
			cc = find_rec (inal2, &inal2_rec, NEXT, "r");
		}

		if (noConflict == FALSE)
		{
			strcpy (wkFrom, DateToString (inal2_rec.date_from));
			strcpy (wkTo  , DateToString (inal2_rec.date_to));
			sprintf (err_str, ML (mlSkMess469), wkFrom, wkTo);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (store [line_cnt].currencyStore, "%-3.3s", local_rec.currencyCode);	
	
		DSP_FLD ("currencyDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=============================
| Check validity of data      |
=============================*/
int
ValidMcurrItem (void)
{
	/*----------------------------
	| Load existing inal records |
	| into tabular screen.       |
	----------------------------*/	
	foundInal = FALSE;
	strcpy (inal_rec.br_no, local_rec.branchNo);
	inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal_rec.date_from = local_rec.dateFrom;
	strcpy (inal_rec.curr_code, "   ");
	cc = find_rec (inal3, &inal_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (inal_rec.br_no, local_rec.branchNo) &&
	       inal_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       inal_rec.date_from == local_rec.dateFrom)
	{
		if (!foundInal)
		{
			scn_set (2);
			lcount [2] = 0;
		}
	
		foundInal = TRUE;

		local_rec.dateTo = inal_rec.date_to;
		sprintf (local_rec.currencyCode, "%-3.3s", inal_rec.curr_code);
		sprintf (store [lcount [2]].currencyStore, "%-3.3s", inal_rec.curr_code);
		if (!CheckCurrCode (local_rec.currencyCode))
		{
			cc = find_rec (inal3, &inal_rec, NEXT, "r");
			continue;
		}
		putval (lcount [2]++);
		cc = find_rec (inal3, &inal_rec, NEXT, "r");
	}

	newCode = !foundInal;

	if (foundInal)
	{
		scn_display (2);
		scn_set (1);
		scn_display (1);
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------
	| No records exist. Check for overlap. |
	--------------------------------------*/
	strcpy (inal_rec.br_no, local_rec.branchNo);
	inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal_rec.date_from = 0L;
	strcpy (inal_rec.curr_code, "   ");
	cc = find_rec (inal3, &inal_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (inal_rec.br_no, local_rec.branchNo) &&
	       inal_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (local_rec.dateFrom >= inal_rec.date_from && 
		    local_rec.dateFrom <= inal_rec.date_to)
		{
			print_mess (ML (mlSkMess392));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		
		cc = find_rec (inal3, &inal_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*==================================
| Check validity of currency code. |
==================================*/
int
CheckCurrCode (
	char	*checkCurrency)
{
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", checkCurrency);
	cc = find_rec (pocr, &pocrRec, EQUAL, "r");
	if (cc)
		return (FALSE);

	return (TRUE);
}

/*==============================
| Check for overlaps in dates. |
==============================*/
int
CheckOverlap (
	int		checkType,
	char 	*checkCurrency)
{
	strcpy (inal2_rec.br_no, local_rec.branchNo);
	sprintf (inal2_rec.curr_code, "%-3.3s", checkCurrency);
	inal2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal2_rec.date_from = 0L;
	cc = find_rec (inal2, &inal2_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (inal2_rec.br_no, local_rec.branchNo) &&
           !strcmp (inal2_rec.curr_code, checkCurrency) &&
	       inal2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!checkType)
		{
			if (local_rec.dateFrom >= inal2_rec.date_from && 
			    local_rec.dateFrom <= inal2_rec.date_to)
				return (FALSE);
		}
		else
		{
			/*---------------------------
			| Check for current record. |
			---------------------------*/
			if (local_rec.dateFrom == inal2_rec.date_from)
			{
				cc = find_rec (inal2, &inal2_rec, NEXT, "r");
				continue;
			}

			if (local_rec.dateTo >= inal2_rec.date_from && 
			    local_rec.dateTo <= inal2_rec.date_to)
				return (FALSE);
		}

		cc = find_rec (inal2, &inal2_rec, NEXT, "r");
	}

	return (TRUE);
}

/*==========================
| Perform checks on dates. |
==========================*/
int
CheckDates (
	long	hhbrHash, 
	long	dateFrom, 
	long	dateTo)
{
	char	tmpCurr [4];

	sprintf (tmpCurr, "%-3.3s", local_rec.currencyCode);

	strcpy (inal2_rec.br_no, local_rec.branchNo);
	sprintf (inal2_rec.curr_code, "%-3.3s", tmpCurr);
	inal2_rec.hhbr_hash = hhbrHash;
	inal2_rec.date_from = 0L;

	cc = find_rec (inal2, &inal2_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inal2_rec.br_no, local_rec.branchNo) &&
	       !strcmp (inal2_rec.curr_code, tmpCurr) &&
	       inal2_rec.hhbr_hash == hhbrHash)
	{
		/*---------------------------
		| Check for current record. |
		---------------------------*/
		if (!newCode &&
		    inal2_rec.date_from == local_rec.dateFrom)
		{
			cc = find_rec (inal2, &inal2_rec, NEXT, "r");
			continue;
		}

		if (dateFrom >= inal2_rec.date_from &&
		    dateFrom <= inal2_rec.date_to)
			return (EXIT_FAILURE);
		
		if (dateTo >= inal2_rec.date_from &&
		    dateTo <= inal2_rec.date_to)
			return (EXIT_FAILURE);

		if (inal2_rec.date_from >= dateFrom &&
			inal2_rec.date_from <= dateTo)
			return (EXIT_FAILURE);

		if (inal2_rec.date_to >= dateFrom &&
			inal2_rec.date_to <= dateTo)
			return (EXIT_FAILURE);

		cc = find_rec (inal2, &inal2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*=========================
| Search for pocr record. |
=========================*/
void
SrchPocr (
 char *key_val)
{
	work_open ();
	save_rec ("#Currency.","#Description.");

	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pocrRec.co_no, comm_rec.co_no) && 
	       !strncmp (pocrRec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

/*=====================================
| Search for inal record using id_no2 |
=====================================*/
void
SrchInal (void)
{
	char	wsCurrCode [4];
	long	wsDateFrom;
	char	dateStringFrom [11];
	char	dateStringTo [11];

	_work_open (10,0,10);
	save_rec ("#From Date ", "# To Date");

	wsDateFrom = 0L;
	strcpy (wsCurrCode, "   ");
	strcpy (inal_rec.br_no, local_rec.branchNo);
	inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal_rec.date_from = 0L;
	sprintf (inal_rec.curr_code, "%-3.3s", " ");
	cc = find_rec (inal3, &inal_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inal_rec.br_no, local_rec.branchNo) &&
	       inal_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (inal_rec.date_from != wsDateFrom)
		{
			strcpy (wsCurrCode, inal_rec.curr_code);
			wsDateFrom = inal_rec.date_from;
			strcpy (dateStringFrom, DateToString (inal_rec.date_from));
			strcpy (dateStringTo, 	DateToString (inal_rec.date_to));
			cc = save_rec (dateStringFrom, dateStringTo);
			if (cc)
				break;
		}

		cc = find_rec (inal3, &inal_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	return;
}

/*=======================================
| Main update routine for levy records. |
=======================================*/
void
Update (void)
{
	int		i;
	int		rec_ok;
	char	tmpCurr [4];
	long	date_to;

	date_to = local_rec.dateTo;

	/*-------------------------------------------
	| Delete any records that exist on inal but |
	| don't have a line in the tabular screen.  |
	-------------------------------------------*/
	strcpy (inal_rec.br_no, local_rec.branchNo);
	inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal_rec.date_from = local_rec.dateFrom;
	strcpy (inal_rec.curr_code, "   ");
	cc = find_rec (inal3, &inal_rec, GTEQ, "u");
	while (!cc && 
	       !strcmp (inal_rec.br_no, local_rec.branchNo) &&
	       inal_rec.hhbr_hash == inmr_rec.hhbr_hash && 
	       inal_rec.date_from == local_rec.dateFrom)
	{
		/*----------------------------------------------
		| Check that record exists in tablular screen. |
		----------------------------------------------*/
		rec_ok = FALSE;
		for (i = 0; i < lcount [2]; i++)
		{
			if (!strcmp (store [i].currencyStore, inal_rec.curr_code))
			{
				rec_ok = TRUE;
				break;
			}
		}

		if (!rec_ok)
		{
			/*----------------
			| Delete record. |
			----------------*/
			strcpy (tmpCurr, inal_rec.curr_code);
			cc = abc_delete (inal3);
			if (cc)
				file_err (cc, inal3, "DBDELETE");

			/*-------------------
			| Read next record. |
			-------------------*/
			strcpy (inal_rec.br_no, local_rec.branchNo);
			inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
			inal_rec.date_from = local_rec.dateFrom;
			strcpy (inal_rec.curr_code, tmpCurr);

			cc = find_rec (inal3, &inal_rec, GTEQ, "u");
		}
		else
		{
			abc_unlock (inal3);
			cc = find_rec (inal3, &inal_rec, NEXT, "u");
		}
	}

	/*-----------------------------------------
	| Add / Update records in tabular screen. |
	-----------------------------------------*/
	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		strcpy (inal_rec.br_no, local_rec.branchNo);
		inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inal_rec.date_from = local_rec.dateFrom;
		strcpy (inal_rec.curr_code, store [i].currencyStore);
		cc = find_rec (inal3, &inal_rec, EQUAL, "u");
		if (cc)
		{
			getval (i);
			strcpy (inal_rec.stat_flag, "0");
			inal_rec.date_to = date_to;
			if (inal_rec.value > 0.0 || inal_rec.percent > 0.00)
			{
				cc = abc_add (inal3, &inal_rec);
				if (cc)
					file_err (cc, inal3, "DBADD");
			}
		}
		else
		{
			getval (i);
			inal_rec.date_to = date_to;
			if (inal_rec.value > 0.0 || inal_rec.percent > 0.00)
			{
				cc = abc_update (inal3, &inal_rec);
				if (cc)
					file_err (cc, inal3, "DBUPDATE");
			}
			else
			{
				cc = abc_delete (inal3);
				if (cc)
					file_err (cc, inal3, "DBDELETE");
			}
		}
	}
	
	return;
}

/*================================
| Search for branch master file. |
================================*/
void
SrchEsmr (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Name  ");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*==================================
| Delete line from tabular screen. |
==================================*/
int
DeleteLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		strcpy (store [line_cnt].currencyStore, store [line_cnt + 1].currencyStore);

		getval (line_cnt + 1);
		putval (line_cnt);
	}

	strcpy (local_rec.currencyCode, "   ");
	inal_rec.value		= 0.00;
	inal_rec.percent	= 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	scn_display (2);
	return (EXIT_SUCCESS);
}

/*=========================================
| Display existing records for help user. |
=========================================*/
void
DisplayRecords (void)
{
	long	wsDateFrom;
	char	wsCurrCode [4],
			dateStringFrom [11],
			dateStringTo [11];

	/*-----------------------------------
	| setup supplier item display		|
	-----------------------------------*/
	dspOpen = TRUE;
	Dsp_open (1, 9, 6);
	Dsp_saverec ("   Date   |   Date   ");
	Dsp_saverec ("   From   |    to    ");
	Dsp_saverec ("");

	wsDateFrom = 0L;
	strcpy (wsCurrCode, "   ");
	strcpy (inal_rec.br_no, local_rec.branchNo);
	inal_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inal_rec.date_from = 0L;
	sprintf (inal_rec.curr_code, "%-3.3s", " ");
	cc = find_rec (inal3, &inal_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inal_rec.br_no, local_rec.branchNo) &&
	       inal_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (inal_rec.date_from != wsDateFrom)
		{
			strcpy (wsCurrCode, inal_rec.curr_code);
			wsDateFrom = inal_rec.date_from;
			strcpy (dateStringFrom, DateToString (inal_rec.date_from));
			strcpy (dateStringTo, 	DateToString (inal_rec.date_to));
			sprintf 
			 (
				err_str, 
				"%s^E%s",
				dateStringFrom,
				dateStringTo
			);
			Dsp_saverec (err_str);
		}
		cc = find_rec (inal3, &inal_rec, NEXT, "r");
	}
	Dsp_srch ();
}
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	strcpy (err_str, ML ("Inventory Levy Maintenance."));
	rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);

	box (0, 3, 132, 5);

	line_at (1,0,132);

	if (scn == 1)
	{
		scn_set (2);
		scn_write (2);
		if (displayScn2)
			scn_display (2);
		scn_set (1);
	}
	else
	{
		scn_set (1);
		scn_write (1);
		scn_display (1);
		scn_set (2);
	}

#ifndef GVISION
	box (28, 9, 80, 10);
#endif

	line_at (5,1,131);
	line_at (7,1,131);
	line_at (21,0,132);
	print_at (22,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (23,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
