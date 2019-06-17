/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_tax_mnt.c,v 5.6 2002/10/07 03:39:32 robert Exp $
|  Program Name  : (so_tax_mnt.c)
|  Program Desc  : (Inventory Tax Maintenance)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (27/09/93)       |
|---------------------------------------------------------------------|
| $Log: sk_tax_mnt.c,v $
| Revision 5.6  2002/10/07 03:39:32  robert
| SC 4257 - fixed checkbox field
|
| Revision 5.5  2002/07/24 08:39:19  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/20 07:11:16  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/12/12 06:35:04  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_tax_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_tax_mnt/sk_tax_mnt.c,v 5.6 2002/10/07 03:39:32 robert Exp $";

#define		TOTSCNS		2
#define 	MAXLINES	2000
#define 	MAXWIDTH	150
#include 	<pslscr.h>
#include 	<minimenu.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define		SEL_UPDATE		0
#define		SEL_IGNORE		1
#define		SEL_DELETE		2
#define		DEFAULT		99

#define		BY_CAT	 (local_rec.allcType [0] == 'C')

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct inthRecord	inth_rec;
struct inthRecord	inth2_rec;
struct intdRecord	intd_rec;

	char	*data  = "data",
			*excf2 = "excf2",
			*inmr2 = "inmr2",
			*inth2 = "inth2",
			*intd2 = "intd2",
			*intd3 = "intd3",
			*intd4 = "intd4";

   	int  	newCode = FALSE;

	struct	storeRec {
		char	hashType [2];
		long	hash;
	} store [MAXLINES];

	extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	allcType [9];
	char	itemCatNo [17];
	char	numPrompt [43];
	char	desc [41];
	long	itemCatHash;
	char	systemDate [11];
	float	old_rate;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "tax_code",	 3, 2, CHARTYPE,
		"UU", "          ",
		" ", "","Tax Code             "," Enter Tax Code To Maintain. Search Available. ",
		NE, NO,  JUSTLEFT, "", "", inth_rec.tax_code},
	{1, LIN, "tax_desc",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description         ", " Enter Description For Tax. ",
		 NO, NO,  JUSTLEFT, "", "", inth_rec.tax_desc},
	{1, LIN, "tax_rate",	 5, 2, FLOATTYPE,
		"NN.NN", "          ",
		" ", "", "Tax Rate            ", " Enter Tax Rate As A Percentage. ",
		 YES, NO,  JUSTLEFT, "0.0", "99.99", (char *)&inth_rec.tax_rate},
	{1, LIN, "prv_rate",	 5, 45, FLOATTYPE,
		"NN.NN", "          ",
		" ", "", "Previous Rate       ", "",
		 NA, NO,  JUSTLEFT, "0.0", "99.99", (char *)&inth_rec.prv_rate},
	{1, LIN, "eff_date",	 6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Effective Date      ", " Enter Date That Tax Is Effective From. ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inth_rec.eff_date},
	{1, LIN, "chg_date",	 6, 45, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Date Last Changed   ", "",
		NA, NO, JUSTRIGHT, "", "", (char *)&inth_rec.chg_date},

	{2, TAB, "allc_type",	MAXLINES, 0, CHARTYPE,
		"UAAAAAAA", "          ",
		" ", " ", "  Type  ", " I)tem OR C)ategory. ",
		YES, NO,  JUSTLEFT, "CIci", "", local_rec.allcType},
	{2, TAB, "item_cat_no",	 0, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "      Number.     ", local_rec.numPrompt,
		 YES, NO,  JUSTLEFT, "", "", local_rec.itemCatNo},
	{2, TAB, "desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " D e s c r i p t i o n.          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{2, TAB, "item_cat_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.itemCatHash},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchInth 		(char *);
void 	SrchExcf 		(char *);
void 	tab_other 		(int);
void 	LoadItems 		(void);
void 	InitStore 		(void);
void 	Update 			(void);
int  	HashInTab 		(char, long);
int  	delete_line 	(void);
int  	ChkOtherTax 	(char, long);
int  	spec_valid 		(int field);
int  	heading 		(int scn);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	TruePosition	=	TRUE;	

	SETUP_SCR (vars);


	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	sprintf (local_rec.numPrompt, "%-40.40s", " ");

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

	OpenDB ();

	tab_row = 8;
	tab_col = 5;

	/*
	 * Beginning of input control loop .
	 */
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags .
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		lcount [2] = 0;

		/*
		 * Initialise screen and array.
		 */
		init_vars (1);
		InitStore ();
	
		/*
		 * Enter screen 1 linear input 
		 */
		abc_unlock (inth);
		heading (1);
		scn_display (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);

		if (newCode)
		{
			entry (2);
			if (restart)
				continue;
		}

		edit_all ();
		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
 	abc_alias (excf2, excf);
 	abc_alias (inmr2, inmr);
 	abc_alias (intd2, intd);
 	abc_alias (intd3, intd);
 	abc_alias (intd4, intd);
 	abc_alias (inth2, inth);

	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (excf2, excf_list, EXCF_NO_FIELDS, "excf_hhcf_hash");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inth,  inth_list, INTH_NO_FIELDS, "inth_id_no");
	open_rec (inth2, inth_list, INTH_NO_FIELDS, "inth_hhth_hash");
	open_rec (intd,  intd_list, INTD_NO_FIELDS, "intd_id_no");
	open_rec (intd2, intd_list, INTD_NO_FIELDS, "intd_id_no2");
	open_rec (intd3, intd_list, INTD_NO_FIELDS, "intd_hhbr_hash");
	open_rec (intd4, intd_list, INTD_NO_FIELDS, "intd_hhcf_hash");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (excf);
	abc_fclose (excf2);
	abc_fclose (inmr);
	abc_fclose (inmr2);
    abc_fclose (inth2);
	abc_fclose (inth);
	abc_fclose (intd);
	abc_fclose (intd2);
	abc_fclose (intd3);
	abc_fclose (intd4);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("tax_code"))
	{
		if (SRCH_KEY)
		{
			SrchInth (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inth_rec.co_no, comm_rec.co_no);
		newCode = find_rec (inth, &inth_rec, COMPARISON, "w");
		if (!newCode)
		{
			DSP_FLD ("tax_desc");
			DSP_FLD ("tax_rate");
			DSP_FLD ("prv_rate");
			DSP_FLD ("eff_date");
			DSP_FLD ("chg_date");
			local_rec.old_rate = inth_rec.tax_rate;
			LoadItems ();

			entry_exit = TRUE;
		}
		else
			local_rec.old_rate = 0.00;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("allc_type"))
	{
		if (last_char == DELLINE)
			return (delete_line ());

		if (dflt_used)
		{
			if (prog_status == ENTRY)
				strcpy (local_rec.allcType, "Item    ");
			else
				return (delete_line ());
		}

		if (local_rec.allcType [0] == 'C')
		{
			strcpy (local_rec.allcType, "Category");
			strcpy (local_rec.numPrompt, " Enter Category Number. Search Available. ");
			vars [label ("item_cat_no")].mask = "UUUUUUUUUUU     ";
		}
		else
		{
			strcpy (local_rec.allcType, "Item    ");
			strcpy (local_rec.numPrompt, " Enter Item Number. Search Available.     ");
			vars [label ("item_cat_no")].mask = "UUUUUUUUUUUUUUUU";
		}
		DSP_FLD ("allc_type");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_cat_no"))
	{
		if (SRCH_KEY)
		{
			if (BY_CAT)
 				SrchExcf (temp_str);
			else
				InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (BY_CAT)
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			sprintf (excf_rec.cat_no, "%-11.11s", local_rec.itemCatNo);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (HashInTab ('C', excf_rec.hhcf_hash))
			{
				print_mess (ML (mlSkMess112));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (ChkOtherTax ('C', excf_rec.hhcf_hash))
			{
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			sprintf (local_rec.desc, "%-40.40s", excf_rec.cat_desc);
			local_rec.itemCatHash = excf_rec.hhcf_hash;
			strcpy (store [line_cnt].hashType, "C");
			store [line_cnt].hash = excf_rec.hhcf_hash;
		}
		else
		{
			cc = FindInmr (comm_rec.co_no, local_rec.itemCatNo, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.itemCatNo);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			SuperSynonymError ();

			if (HashInTab ('I', inmr_rec.hhbr_hash))
			{
				print_mess (ML (mlSkMess113));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (ChkOtherTax ('I', inmr_rec.hhbr_hash))
			{
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			
			strcpy (local_rec.itemCatNo, inmr_rec.item_no);
			sprintf (local_rec.desc, "%-40.40s", inmr_rec.description);
			local_rec.itemCatHash = inmr_rec.hhbr_hash;
			strcpy (store [line_cnt].hashType, "I");
			store [line_cnt].hash = inmr_rec.hhbr_hash;
		}
		DSP_FLD ("item_cat_no");
		DSP_FLD ("desc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Tax Code Search.
 */
void
SrchInth (
	char	*keyValue)
{
	char	descStr [100];

	_work_open (2,0,50);
	save_rec ("#No", "#             Description                | Tax %");

	strcpy (inth_rec.co_no, comm_rec.co_no);
	sprintf (inth_rec.tax_code, "%-2.2s", keyValue);
	cc = find_rec (inth, &inth_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inth_rec.co_no, comm_rec.co_no) &&
	       !strncmp (inth_rec.tax_code, keyValue, strlen (keyValue)))
	{
		sprintf (descStr,"%-40.40s|%7.2f", inth_rec.tax_desc, inth_rec.tax_rate);
		cc = save_rec (inth_rec.tax_code, descStr);
		if (cc)
			break;

		cc = find_rec (inth, &inth_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inth_rec.co_no, comm_rec.co_no);
	sprintf (inth_rec.tax_code, "%-2.2s", temp_str);
	cc = find_rec (inth, &inth_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inth, "DBFIND");
}

/*
 * Category Search.
 */
void
SrchExcf (
 char *keyValue)
{
	_work_open (11,0,40);
	save_rec ("#Category", "#Description ");

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", keyValue);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (excf_rec.co_no, comm_rec.co_no) &&
	       !strncmp (excf_rec.cat_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
tab_other (
 int line_no)
{
	getval (line_no);
	if (BY_CAT)
		vars [label ("item_cat_no")].mask = "UUUUUUUUUUU     ";
	else
		vars [label ("item_cat_no")].mask = "UUUUUUUUUUUUUUUU";
}

/*
 * Check whether hash is already in table. 
 * Return TRUE if it is, else return FALSE. 
 */
int
HashInTab (
 char hashType, 
 long itemCatHash)
{
	int		i;
	int		numToChk;

	numToChk = (prog_status == ENTRY) ? line_cnt : lcount [2];
	for (i = 0; i < numToChk; i++)
	{
		if (i == line_cnt)
			continue;

		if (store [i].hashType [0] == hashType && store [i].hash == itemCatHash)
			return (TRUE);
	}

	return (FALSE);
}

/*
 * Check whether item/category is allocated to another 
 * tax code.                                          
 * Returns :                                         
 * FALSE - Item/Category not allocated.             
 * TRUE  - Item/Category allocated to another tax code
 */
int
ChkOtherTax (
	char	hashType, 
	long 	itemCatHash)
{
	char	fileToUse [6];
	char	allcTaxCode [8];


	if (hashType == 'C')
		strcpy (fileToUse, "intd4");
	else
		strcpy (fileToUse, "intd3");

	cc = find_hash (fileToUse, &intd_rec, COMPARISON, "r", itemCatHash);
	if (cc)
		return (FALSE);

	/*
	 * Check if Item / Category is currently allocated
	 * (on file) for the current tax code.           
	 */
	if (!newCode && intd_rec.hhth_hash == inth_rec.hhth_hash)
		return (FALSE);

	/*
	 * Item / Category is allocated to another tax code. 
	 */
	cc = find_hash (inth2, &inth2_rec, COMPARISON, "r", intd_rec.hhth_hash);
	if (cc)
		strcpy (allcTaxCode, "UNKNOWN");
	else
		sprintf (allcTaxCode, "%-2.2s", inth2_rec.tax_code);

	if (hashType == 'C')
		sprintf (err_str,ML (mlSkMess128), 
			clip (local_rec.itemCatNo),
			clip (allcTaxCode));
	else
		sprintf (err_str,ML (mlSkMess129),
			clip (local_rec.itemCatNo),
			clip (allcTaxCode));

	return (TRUE);
}

/*
 * Load items and categories allocated to this tax code.
 */
void
LoadItems (
 void)
{
	scn_set (2);
	lcount [2] = 0;
	
	/*
	 * Category allocations.
	 */
	intd_rec.hhth_hash = inth_rec.hhth_hash;
	intd_rec.hhcf_hash = 0L;
	cc = find_rec (intd2, &intd_rec, GTEQ, "r");
	while (!cc && intd_rec.hhth_hash == inth_rec.hhth_hash)
	{
		/*
		 * Lookup category record.
		 */
		cc = find_hash (excf2, &excf_rec, COMPARISON, "r", intd_rec.hhcf_hash);
		if (cc)
		{
			cc = find_rec (intd2, &intd_rec, NEXT, "r");
			continue;
		}
		strcpy (local_rec.allcType, "Category");
		sprintf (local_rec.itemCatNo, "%-11.11s     ", excf_rec.cat_no);
		sprintf (local_rec.desc,      "%-40.40s",      excf_rec.cat_desc);
		local_rec.itemCatHash = excf_rec.hhcf_hash;
		strcpy (store [lcount [2]].hashType, "C");
		store [lcount [2]].hash = excf_rec.hhcf_hash;
		putval (lcount [2]++);

		cc = find_rec (intd2, &intd_rec, NEXT, "r");
	}

	/*
	 * Item allocations. 
	 */
	intd_rec.hhth_hash = inth_rec.hhth_hash;
	intd_rec.hhbr_hash = 0L;
	cc = find_rec (intd, &intd_rec, GTEQ, "r");
	while (!cc && intd_rec.hhth_hash == inth_rec.hhth_hash)
	{
		/*
		 * Lookup item record.
		 */
		cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r", intd_rec.hhbr_hash);
		if (cc)
		{
			cc = find_rec (intd, &intd_rec, NEXT, "r");
			continue;
		}
		strcpy (local_rec.allcType, "Item    ");
		sprintf (local_rec.itemCatNo, "%-16.16s", inmr_rec.item_no);
		sprintf (local_rec.desc,      "%-40.40s", inmr_rec.description);
		local_rec.itemCatHash = inmr_rec.hhbr_hash;
		strcpy (store [lcount [2]].hashType, "I");
		store [lcount [2]].hash = inmr_rec.hhbr_hash;
		putval (lcount [2]++);

		cc = find_rec (intd, &intd_rec, NEXT, "r");
	}

	scn_set (1);
}

int
delete_line (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	/*
	 * entry
	 */
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	/*
	 * no lines to delete
	 */
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}

	/*
	 * delete lines
	 */
	lcount [2]--;
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		store [line_cnt].hash = store [line_cnt + 1].hash;
		strcpy (store [line_cnt].hashType, store [line_cnt + 1].hashType);
		if (line_cnt / TABLINES == this_page)
			line_display ();
	}

	/*
	 * blank last line - if required
	 */
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*
	 * zap buffer if deleted all
	 */
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
InitStore (
 void)
{
	int	i;

	for (i = 0;i < MAXLINES;i++)
	{
		store [i].hash = 0L;
		strcpy (store [i].hashType, " ");
	}
}

/*
 * Update tax code.
 */
void
Update (
 void)
{
	/*
	 * Add / Update Header.
	 */
	if (newCode)
	{
		strcpy (inth_rec.co_no, comm_rec.co_no);
		inth_rec.chg_date = TodaysDate ();
		cc = abc_add (inth, &inth_rec);
		if (cc)
			file_err (cc, inth, "DBADD");

		cc = find_rec (inth, &inth_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, inth, "DBFIND");
	}
	else
	{
		if (inth_rec.tax_rate != local_rec.old_rate)
		{
			inth_rec.prv_rate = local_rec.old_rate;
			inth_rec.chg_date = TodaysDate ();
		}
		cc = abc_update (inth, &inth_rec);
		if (cc)
			file_err (cc, inth, "DBUPDATE");
	}

	/*
	 * Delete old line records.
	 */
	intd_rec.hhth_hash = inth_rec.hhth_hash;
	intd_rec.hhbr_hash = 0L;
	cc = find_rec (intd, &intd_rec, GTEQ, "u");
	while (!cc && intd_rec.hhth_hash == inth_rec.hhth_hash)
	{
		cc = abc_delete (intd);
		if (cc)
			file_err (cc, intd, "DBDELETE");

		intd_rec.hhth_hash = inth_rec.hhth_hash;
		intd_rec.hhbr_hash = 0L;
		cc = find_rec (intd, &intd_rec, GTEQ, "u");
	}
	abc_unlock (intd);

	/*
	 * Add / Update Lines.
	 */
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		/*
		 * Add new allocation record.
		 */
		intd_rec.hhth_hash = inth_rec.hhth_hash;
		intd_rec.hhbr_hash = 0L;
		intd_rec.hhcf_hash = 0L;
		if (BY_CAT)
			intd_rec.hhcf_hash = local_rec.itemCatHash;
		else
			intd_rec.hhbr_hash = local_rec.itemCatHash;

		cc = abc_add (intd, &intd_rec);
		if (cc)
			file_err (cc, intd, "DBADD");
	}
	abc_unlock (inth);
}

int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	strcpy (err_str,ML (mlSkMess052));
	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);
	
	line_at (1,0,80);

	switch (scn)
	{
	case 	1:
		box (0, 2, 80, 4);
		scn_set (2);
		scn_write (2);
		scn_display (2);
		break;

	case	2:
		box (0, 2, 80, 4);
		scn_set (1);
		scn_write (1);
		scn_display (1);
		break;
	}

	line_at (20,0,80);
	line_at (22,0,80);

	print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

