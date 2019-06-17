/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_tax_upd.c,v 5.3 2001/12/13 00:52:34 scott Exp $
|  Program Name  : (so_tax_upd.c)
|  Program Desc  : (Inventory Tax Update And Audit)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : (01/10/93)       |
|---------------------------------------------------------------------|
| $Log: sk_tax_upd.c,v $
| Revision 5.3  2001/12/13 00:52:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_tax_upd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_tax_upd/sk_tax_upd.c,v 5.3 2001/12/13 00:52:34 scott Exp $";

#include 	<ml_sk_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define		NOREC	0
#define		ITEM	1
#define		CAT		2

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct inthRecord	inth_rec;
struct intdRecord	intd_rec;

	char	*data  = "data";

	int		printerNo	=	1,
			envGst		=	0;

	FILE	*pout;

	char	envGstTaxName [4];

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	stItem [17];
	char	stDesc [41];
	char	endItem [17];
	char	endDesc [41];
	char	stCat [17];
	char	stcDesc [41];
	char	endCat [17];
	char	endcDesc [41];
	long	lsystemDate;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_item",	 4, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Start Item :", " Enter Start Item, Search Available ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.stItem},
	{1, LIN, "st_desc",	 4, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.stDesc},
	{1, LIN, "end_item",	 5, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " End Item   :", " Enter End Item, Search Available ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "end_desc",	 5, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endDesc},
	{1, LIN, "st_cat",	 4, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", " Start Cat :", " Enter Start Category, Search Available ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.stCat},
	{1, LIN, "st_cdesc",	 4, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.stcDesc},
	{1, LIN, "end_cat",	 5, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", " End Cat   :", " Enter End Category, Search Available ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, LIN, "end_cdesc",	 5, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.endcDesc},
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
void 	Update 			(char *);
void 	SetTax 			(int *, int, float, char *);
void 	GetCat 			(void);
void 	OpenAud 		(char *);
void 	CloseAud 		(void);
void 	WriteAud 		(float, float, int);
void 	SetPrompts 		(char *);
void 	SrchExcf 		(char *);
int  	DateOk 			(long);
int  	NoTax 			(int);
int  	CheckInCatRange (void);
int  	spec_valid 		(int);
int  	heading 		(int);


/*
 * Main Processing Routine.
 */
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;
	char	option [2];

	if (argc != 3)
	{
		print_at (0,0,ML (mlSkMess511),argv [0]);
		return (EXIT_FAILURE);
	}
	else
		printerNo = atoi (argv [1]);

	if (argv [2][0] != 'i' &&
		argv [2][0] != 'I' &&
		argv [2][0] != 'c' &&
		argv [2][0] != 'C')
	{
		print_at (0,0,ML (mlSkMess511),argv [0]);
		return (EXIT_FAILURE);
	}
	else
		option [0] = toupper (argv [2][0]);

	SETUP_SCR (vars);
	SetPrompts (option);

	local_rec.lsystemDate = TodaysDate ();

	/*
	 * Check if gst applies.
	 */
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envGst = 0;
	else
		envGst = (*sptr == 'Y' || *sptr == 'y');

	if (envGst)
		sprintf (envGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envGstTaxName, "%-3.3s", "TAX");

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*
	 * Beginning of input control loop.
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
		init_vars (1);
	
		/*
		 * Enter screen 1 linear input  
		 */
		heading (1);
		scn_display (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);

		edit (1);
		if (restart)
			continue;

		Update (option);
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
 * Open data base files .
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inth, inth_list, INTH_NO_FIELDS, "inth_hhth_hash");
	open_rec (intd, intd_list, INTD_NO_FIELDS, "intd_hhbr_hash");
}

/*
 * Close data base files .
 */
void
CloseDB (void)
{
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (inth);
	abc_fclose (intd);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("st_item"))
	{
		if (FLD ("st_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.stItem, "                ");
			strcpy (local_rec.stDesc, ML ("Start Of File"));
			DSP_FLD ("st_item");
			DSP_FLD ("st_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.stItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.stItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.stItem, local_rec.endItem) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		SuperSynonymError ();

		strcpy (local_rec.stDesc, inmr_rec.description);
		DSP_FLD ("st_item");
		DSP_FLD ("st_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_item"))
	{
		if (FLD ("end_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.endDesc, ML ("End Of File"));
			DSP_FLD ("end_item");
			DSP_FLD ("end_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endItem, local_rec.stItem) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.endDesc, inmr_rec.description);
		DSP_FLD ("end_item");
		DSP_FLD ("end_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_cat"))
	{
		if (FLD ("st_cat") == ND)
			return (EXIT_SUCCESS);

		if (last_char == FN16 && prog_status == ENTRY)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.stCat, "           ");
			strcpy (local_rec.stcDesc, ML ("Start Of File"));
			DSP_FLD ("st_cat");
			DSP_FLD ("st_cdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.stCat);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.stCat, local_rec.endCat) > 0)
			{
				print_mess (ML (mlStdMess018));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		strcpy (local_rec.stcDesc, excf_rec.cat_desc);
		DSP_FLD ("st_cat");
		DSP_FLD ("st_cdesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_cat"))
	{
		if (FLD ("end_cat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endCat, "~~~~~~~~~~~");
			strcpy (local_rec.endcDesc, ML ("End Of File"));
			DSP_FLD ("end_cat");
			DSP_FLD ("end_cdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.endCat);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.endCat, local_rec.stCat) < 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endcDesc, excf_rec.cat_desc);
		DSP_FLD ("end_cat");
		DSP_FLD ("end_cdesc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update tax code.
 */
void
Update (
	char	*option)
{
	int		fstTime = TRUE;
	int		type;

	sprintf (err_str, 
			"Inventory %-3.3s Update By %s", envGstTaxName,
			 (option [0] == 'I') ? "Item" : "Category");
					
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (inmr_rec.co_no, comm_rec.co_no);

	/*
	 * if option is byItem then use stItem else need to extract
	 * items for Category starting with first item then checking against
	 * catogory.
	 */
	if (option [0] != 'I')
	{
		strcpy (local_rec.stItem, "                ");
		strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
	}

	strcpy (inmr_rec.item_no, local_rec.stItem);

	cc = find_rec (inmr, &inmr_rec, GTEQ, "u");

	while (!cc && 
			 (!strcmp (inmr_rec.co_no, comm_rec.co_no)) && 
			 (strcmp (inmr_rec.item_no, local_rec.endItem) < 1))
		{
			if (option [0] == 'C' && !CheckInCatRange ())
			{
				abc_unlock (inmr);
				cc = find_rec (inmr, &inmr_rec, NEXT, "u");
				continue;
			}
			dsp_process ("Item :", inmr_rec.item_no);
			/*
			 * find tax record for item, if this does not exist then
			 * find tax record for category if this does not exist
			 * then set tax to 0.00 An audit will be called from SetTax ()
			 */
			type = ITEM;
			if (NoTax (type))
			{
				type = CAT;
				if (NoTax (type))
				{
					/*
					 * means no record so set to zero
					 */
					type = NOREC;
					SetTax (&fstTime, type, 0.00, option);
					abc_unlock (inmr);
					cc = find_rec (inmr, &inmr_rec, NEXT, "u");
					continue;
				}
				else
				{
					/*
					 * cat record exists
					 */
					if (DateOk (inth_rec.eff_date))
						SetTax (&fstTime,type,inth_rec.tax_rate,option);
					else
						abc_unlock (inmr);
				}
			}
			else
			{
				if (DateOk (inth_rec.eff_date))
					SetTax (&fstTime, type, inth_rec.tax_rate, option);
				else
				{
					type = CAT;
					/*
					 * records exists but not yet effective
					 */
					if (NoTax (type) || 
						 (!NoTax (type) && !DateOk (inth_rec.eff_date)))
					{
						abc_unlock (inmr);
					}
					else
						SetTax (&fstTime,type,inth_rec.tax_rate,option);
				}
			}
			cc = find_rec (inmr, &inmr_rec, NEXT, "u");
		}
		abc_unlock (inmr);

		if (!fstTime)
			CloseAud ();
}

int
NoTax (
	int		type)
{

	abc_selfield (intd, (type == CAT) ? "intd_hhcf_hash" : "intd_hhbr_hash");
	if (type == CAT)
		GetCat ();

	memset (&intd_rec, 0, sizeof (intd_rec));
	intd_rec.hhbr_hash = (type == CAT) ? 0L : inmr_rec.hhbr_hash;
	intd_rec.hhcf_hash = (type == CAT) ? excf_rec.hhcf_hash : 0L;

	cc = find_rec (intd, &intd_rec, EQUAL, "r");
	if (!cc)
	{
		inth_rec.hhth_hash = intd_rec.hhth_hash;
		cc = find_rec (inth, &inth_rec, EQUAL, "r");
	}
	return (cc);
}

void
SetTax (
	int 	*fstTime, 
	int 	type, 
	float 	newPc, 
	char 	*option)
{
	float	oldPc;

	if (envGst)
		oldPc = inmr_rec.gst_pc;
	else
		oldPc = inmr_rec.tax_pc;

	if (*fstTime)
		OpenAud (option);
	
	if (oldPc != newPc)
	{
		WriteAud (oldPc, newPc, type);

		if (envGst)
			inmr_rec.gst_pc = newPc;
		else
			inmr_rec.tax_pc = newPc;

		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");
	}
	else
		abc_unlock (inmr);

	*fstTime = FALSE;
}

void
GetCat (
 void)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no, inmr_rec.category);
	cc = find_rec (excf, &excf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

int
DateOk (
 long NewDate)
{

	if (NewDate > comm_rec.inv_date)
		return (FALSE);
	return (TRUE);
}

void
OpenAud (
	char	*option)
{
	if ((pout = popen ("pformat", "w")) == NULL)
		sys_err ("Error In Opening pformat (POPEN)", errno, PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pout, ".LP%d\n", printerNo);
	fprintf (pout, ".PI12\n");
	fprintf (pout, ".11\n");
	fprintf (pout, ".L150\n");
	fprintf (pout, ".E Company %s - %s\n", comm_rec.co_no, comm_rec.co_name);
	fprintf (pout, ".E %-3.3s Rate Update Audit\n", envGstTaxName);
	sprintf (err_str, 
			 "%s", 
			 (option [0] == 'C') ?
			 "In The Category Range" : "");

	fprintf (pout, 
			 ".E For Items %s (%s) %s\n", 
			 err_str,
			 (option [0] == 'C') ? local_rec.stCat : local_rec.stItem,
			 (option [0] == 'C') ? clip (local_rec.stcDesc) : 
								  clip (local_rec.stDesc));
	fprintf (pout, 
			 ".E to (%s) %s\n", 
			 (option [0] == 'C') ? local_rec.endCat : local_rec.endItem,
			 (option [0] == 'C') ? clip (local_rec.endcDesc) :
								  clip (local_rec.endDesc));
	fprintf (pout, ".E As At %s\n", SystemTime ());
	fprintf (pout, "==============================================================================================================================================\n");
	fprintf (pout, "|                "); 
	fprintf (pout, "|                                        "); 
	fprintf (pout, "|%-3.3s ", envGstTaxName);
	fprintf (pout, "|                                        "); 
	fprintf (pout, "| EFFECTIVE");
	fprintf (pout, "| OLD  ");
	fprintf (pout, "| NEW  ");
	fprintf (pout, "|           |\n");

	fprintf (pout, "|  I T E M       "); 
	fprintf (pout, "|   D E S C R I P T I O N                "); 
	fprintf (pout, "|CODE");
	fprintf (pout, "|   D E S C R I P T I O N                "); 
	fprintf (pout, "|   DATE   ");
	fprintf (pout, "| RATE ");
	fprintf (pout, "| RATE ");
	fprintf (pout, "|UPDATED BY |\n");
	fprintf (pout, "----------------------------------------------------------------------------------------------------------------------------------------------\n");
	fprintf (pout, ".R==============================================================================================================================================\n");

}

void
CloseAud (void)
{
	fprintf (pout, ".EOF\n");
	pclose (pout);
}

void
WriteAud (
	float 	oldPc, 
	float 	newPc, 
	int 	type)
{
	char	*desc [] = {"NO RECORD", "BY ITEM", "BY CATEGORY"};

	fprintf (pout, "|%16.16s", inmr_rec.item_no); 
	fprintf (pout, "|%40.40s", inmr_rec.description); 
	fprintf (pout, "| %2.2s ", (type) ? inth_rec.tax_code : " ");
	fprintf (pout, "|%40.40s", (type) ? inth_rec.tax_desc : " "); 
	fprintf (pout, "|%10.10s", (type) ? DateToString (inth_rec.eff_date) : " ");
	fprintf (pout, "|%6.2f",   oldPc);
	fprintf (pout, "|%6.2f",   newPc);
	fprintf (pout, "|%-11.11s|\n",desc [type]);
}

void
SetPrompts (
	char	*type)
{
	FLD ("st_item")   = (type [0] == 'I') ?  YES : ND;
	FLD ("st_desc")   = (type [0] == 'I') ?  NA  : ND;
	FLD ("end_item")  = (type [0] == 'I') ?  YES : ND;
	FLD ("end_desc")  = (type [0] == 'I') ?  NA  : ND;
	FLD ("st_cat")    = (type [0] == 'I') ?  ND  : YES;
	FLD ("st_cdesc")  = (type [0] == 'I') ?  ND  : NA;
	FLD ("end_cat")   = (type [0] == 'I') ?  ND  : YES;
	FLD ("end_cdesc") = (type [0] == 'I') ?  ND  : NA;
}

/*
 * Category Search.
 */
void
SrchExcf (
 char *key_val)
{
	work_open ();
	save_rec ("#Category", "#Description ");

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (excf_rec.co_no, comm_rec.co_no) &&
	       !strncmp (excf_rec.cat_no, key_val, strlen (key_val)))
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


/*
 * Check item category is within range.
 */
int
CheckInCatRange (void)
{
	if ((strcmp (inmr_rec.category, local_rec.stCat) >= 0) &&
		 (strcmp (inmr_rec.category, local_rec.endCat) <= 0))
		return (TRUE);

	return (FALSE);
}

int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	sprintf (err_str, ML (mlSkMess500), envGstTaxName); 
	rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);
	
	box (0, 3, 132, 2);
	line_at (1, 0,132);
	line_at (20,0,132);
	line_at (22,0,132);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21, 0, err_str,comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
