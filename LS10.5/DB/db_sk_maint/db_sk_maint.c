/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_sk_maint.c,v 5.9 2002/07/24 08:38:49 scott Exp $
|  Program Name  : (db_sk_maint.c)
|  Program Desc  : (Customer Product Codes Maintenance)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius.  | Date Written  : 08/09/93         |
|---------------------------------------------------------------------|
| $Log: db_sk_maint.c,v $
| Revision 5.9  2002/07/24 08:38:49  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/18 06:24:15  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.7  2002/07/18 03:30:17  scott
| S/C 004031
|
| Revision 5.6  2002/07/16 07:03:51  scott
| Updated from service calls and general maintenance.
|
| Revision 5.5  2002/07/01 05:15:00  robert
| Name structure storeRec (for use with SetSortArray)
|
| Revision 5.4  2002/06/26 04:34:19  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/06/26 04:26:54  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/03/01 02:48:05  scott
| Updated for alignment
|
| Revision 5.1  2001/12/07 04:05:15  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_sk_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_sk_maint/db_sk_maint.c,v 5.9 2002/07/24 08:38:49 scott Exp $";

#define 	MAXSCNS			2
#define 	MAXLINES		1000
#include 	<pslscr.h>
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>

#define		ScreenWidth		132


	/*============================
	| Special fields and flags   |
	============================*/
	int		envDbCo = 0;
	int		envDbFind = 0;
	int		new_cust = TRUE;

	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct cuitRecord	cuit_rec;
struct cuitRecord	cuit2_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct inmrRecord	inmr_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	dbt_no [7];
	char	dbt_name [41];
	char	last_dbt_no [7];
	char	copy_flag [2];
	char	copy_cust [7];
	char	copy_name [41];
	char	std_item [17];
	long	std_hash;
	char	std_desc [41];
	char	dbt_item [17];
	char	dbt_desc [41];
} local_rec;

extern	int		TruePosition;
struct storeRec
{
	long	hhbrHash;
	char	item_no [17];
} store [MAXLINES];

	char	*data	= "data",
			*inmr2	= "inmr2";

static	struct	var	vars [] =
{
	{1, LIN, "dbt_no",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer Number      ", "Enter Customer Number",
		 NE, NO,  JUSTLEFT, "", "", local_rec.dbt_no},
	{1, LIN, "dbt_name",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name                 ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dbt_name},
	{1, LIN, "copy_cust",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", "Copy From Customer   ", "Copy Item Codes from another customer.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.copy_cust},
	{1, LIN, "copy_name",	 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.copy_name},

	{2, TAB, "dbt_item",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "CUSTOMER ITEM NO", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.dbt_item},
	{2, TAB, "dbt_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "CUSTOMER ITEM DESCRIPTION               ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.dbt_desc},
	{2, TAB, "std_item",	0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "STANDARD ITEM NO", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.std_item},
	{2, TAB, "std_hash",	0, 0, LONGTYPE,
		"NNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "0", "", (char *) &local_rec.std_hash},
	{2, TAB, "std_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "  STANDARD ITEM DESCRIPTION             ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include 	<FindCumr.h>	

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
int 	LoadCuit 			(void);
int 	CopyCuit 			(void);
void 	Update 				(void);
int 	heading 			(int);
int 	FindTableIndex 		(long, char *, int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB 	();
	swide 	();
	tab_row = 3;
	tab_col = 8;


	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*
	 * Beginning of input control loop.
	 */
	strcpy (local_rec.dbt_no, 	   "000000");
	strcpy (local_rec.last_dbt_no, "000000");

	while (prog_exit == 0) 
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		prog_status	= ENTRY;
		init_vars (1);
		init_vars (2);
		lcount [2]	= 0;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		heading (2);
		scn_display (2);

		/*
		 * Enter screen 2 tabular input
		 */
		if (new_cust == TRUE)
			entry (2);
		else
			edit (2);

		if (prog_exit || restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		/*
		 * Update selection status.    
		 */
		Update ();

	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}


/*
 * Program exit sequence
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
	abc_alias (inmr2, inmr);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
		  					       						    : "cumr_id_no3");

	open_rec (cuit, cuit_list, CUIT_NO_FIELDS, "cuit_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}	

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuit);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	SearchFindClose ();
	abc_dbclose (data);
}


int
spec_valid (
	int		field)
{
	int		ws_ix;
	int		this_page;
	int		ws_table_max;

	/*
	 * Validate Customer Number And Allow Search.
	 */
	if (LCHECK ("dbt_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.dbt_name, cumr_rec.dbt_name);
		DSP_FLD ("dbt_name");

		new_cust = TRUE;

		/*----------------------------------
		| Read all Customer Item Codes.    |
		----------------------------------*/
		if (LoadCuit ())
		{
			new_cust = FALSE;
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------------
	| Validate Copy Customer Number And Allow Search. |
	-----------------------------------------------*/
	if (LCHECK ("copy_cust"))
	{
		strcpy (local_rec.copy_name, " ");
		DSP_FLD ("copy_name");

		if (dflt_used)
		{
			strcpy (local_rec.copy_cust, " ");
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
  			return (EXIT_SUCCESS);
		}

		if (strcmp (pad_num (local_rec.copy_cust), local_rec.dbt_no) == 0)
		{
			errmess (ML (mlDbMess179));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		strcpy (cumr2_rec.co_no,comm_rec.co_no);
		strcpy (cumr2_rec.est_no, branchNo);
		strcpy (cumr2_rec.dbt_no, zero_pad (local_rec.copy_cust, 6));
		cc = find_rec (cumr, &cumr2_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.copy_name, cumr2_rec.dbt_name);
		DSP_FLD ("copy_name");

		if (CopyCuit ())
			new_cust = FALSE;

		if ((prog_status != ENTRY) && (new_cust == FALSE))
			heading (1);

		entry_exit = 1;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Customer Number And Allow Search.
	 */
	if (LCHECK ("std_item"))
	{
		if (dflt_used)
		{
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			if (prog_status == ENTRY)
				putval (line_cnt);

			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.std_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.std_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		if (prog_status == ENTRY)
			ws_table_max = line_cnt;
		else
			ws_table_max = lcount [2];

		ws_ix = FindTableIndex (inmr_rec.hhbr_hash, 
							   "                ",
							   ws_table_max);
		if (ws_ix < ws_table_max && ws_ix != line_cnt)
		{
			sprintf (err_str, ML (mlDbMess180),
							  clip (store [ws_ix].item_no), 
							  clip (inmr_rec.item_no));
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.std_item, inmr_rec.item_no);
		strcpy (local_rec.std_desc,inmr_rec.description);
		local_rec.std_hash = inmr_rec.hhbr_hash;
		store [line_cnt].hhbrHash = local_rec.std_hash;
		DSP_FLD ("std_desc");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Invoice Hold Referance. |
	----------------------------------*/
	if (LCHECK ("dbt_item"))
	{
		if (dflt_used && prog_status != ENTRY) /* Delete Item */
		{
			lcount [2]--;
			this_page = line_cnt / TABLINES;

			/*---------------------------
			| Delete item off display 	|
			---------------------------*/
			for (ws_ix = line_cnt; line_cnt < lcount [2]; line_cnt++)
			{
				store [line_cnt].hhbrHash = store [line_cnt + 1].hhbrHash; 
				strcpy (store [line_cnt].item_no, store [line_cnt + 1].item_no); 
				getval (line_cnt + 1);
				putval (line_cnt);
				if (this_page == line_cnt / TABLINES)
					line_display ();
			}
			sprintf (local_rec.std_item,"%16.16s"," ");
			local_rec.std_hash = 0L;
			sprintf (local_rec.std_desc,"%40.40s"," ");
			sprintf (local_rec.dbt_item,"%16.16s"," ");
			sprintf (local_rec.dbt_desc,"%40.40s"," ");

			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				line_display ();
			line_cnt = ws_ix;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}

		if (prog_status == ENTRY)
		{
			local_rec.std_hash = 0L;
			sprintf (local_rec.std_item,"%16.16s"," ");
			sprintf (local_rec.std_desc,"%40.40s"," ");
			sprintf (local_rec.dbt_desc,"%40.40s"," ");
			putval (line_cnt);
		}
		if (!strcmp (local_rec.dbt_item, "                "))
		{
			errmess (ML (mlDbMess181));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			errmess (ML (mlDbMess182));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.dbt_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.dbt_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (!cc)
		{
			sprintf (err_str,ML (mlDbMess183), local_rec.dbt_item);
			cc = prmptmsg (err_str, "YyNn", 0, 22);
			/*
			print_at (22, 0,"                                                                                                     ");
			*/
			if (cc == 'N' || cc == 'n')
				return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		if (prog_status == ENTRY)
			ws_table_max = line_cnt;
		else
			ws_table_max = lcount [2];

		ws_ix = FindTableIndex (0L, local_rec.dbt_item, ws_table_max);
		if (ws_ix < ws_table_max && ws_ix != line_cnt)
		{
			sprintf (err_str, ML (mlDbMess187), 
							  local_rec.dbt_item);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].item_no, local_rec.dbt_item);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*
 * Load cuit data for table 2 display 
 */
int
LoadCuit (void)
{
	int		cc1;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (2);

	init_vars (2);

	lcount [2] = 0;

	/*
	 * Prevents entry if not all lines loaded.
	 */
	cuit_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuit_rec.item_no, "                ");
	cc = find_rec (cuit, &cuit_rec, GTEQ, "r");
	while (!cc && cuit_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		/*
		 * Put Value Into Tabular Screen.
		 */
		inmr_rec.hhbr_hash = cuit_rec.hhbr_hash;
		cc1 = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc1)
		{
			local_rec.std_hash = inmr_rec.hhbr_hash;
			strcpy (local_rec.std_item, inmr_rec.item_no);
			strcpy (local_rec.std_desc, inmr_rec.description);
			strcpy (local_rec.dbt_item, cuit_rec.item_no);
			strcpy (local_rec.dbt_desc, cuit_rec.item_desc);

			store [lcount [2]].hhbrHash = inmr_rec.hhbr_hash;
			strcpy (store [lcount [2]].item_no, cuit_rec.item_no);
	
			putval (lcount [2]++);
		}

		cc = find_rec (cuit, &cuit_rec, NEXT, "r");
	}
	/*
	 * Return to screen 1.
	 */
	scn_set (1);

	return (lcount [2]);
}

/*
 * Copy cuit data from one customer to another
 */
int
CopyCuit (void)
{
	int		cc1;
	int		start_lcount;
	int		ws_ix;

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (2);

	start_lcount = lcount [2];

	/*
	 * Prevents entry if not all lines loaded.
	 */
	cuit_rec.hhcu_hash = cumr2_rec.hhcu_hash;
	strcpy (cuit_rec.item_no, "                ");
	cc = find_rec (cuit, &cuit_rec, GTEQ, "r");

	while (!cc && cuit_rec.hhcu_hash == cumr2_rec.hhcu_hash)
	{
		/*
		 * Put Value Into Tabular Screen.
		 */
		inmr_rec.hhbr_hash = cuit_rec.hhbr_hash;
		cc1 = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (!cc1)
		{
			ws_ix = FindTableIndex (cuit_rec.hhbr_hash, cuit_rec.item_no, lcount [2]);
			if (ws_ix == lcount [2])
			{
				local_rec.std_hash = inmr_rec.hhbr_hash;
				strcpy (local_rec.std_item, inmr_rec.item_no);
				strcpy (local_rec.std_desc, inmr_rec.description);
				strcpy (local_rec.dbt_item, cuit_rec.item_no);
				strcpy (local_rec.dbt_desc, cuit_rec.item_desc);
	
				store [lcount [2]].hhbrHash = inmr_rec.hhbr_hash;
				strcpy (store [lcount [2]].item_no, cuit_rec.item_no);
	
				putval (lcount [2]++);
			}
		}

		cc = find_rec (cuit, &cuit_rec, NEXT, "r");
	}
	/*
	 * Return to screen 1.
	 */
	scn_set (1);

	return (lcount [2] - start_lcount);
}


void
Update (void)
{
   	/*
   	 * Set to Tabular Screen (s) to Update Discount Details.
   	 */
   	scn_set (2);
	
	cuit_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuit_rec.item_no, "                ");
	cc = find_rec (cuit, &cuit_rec, GTEQ, "r");
	while (!cc && cuit_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		cc = abc_delete (cuit);
	  	if (cc)
			file_err (cc, cuit, "DBDELETE");

		cc = find_rec (cuit, &cuit_rec, GTEQ, "r");
	}
   	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
   	{
    	getval (line_cnt);

		cuit_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cuit_rec.hhbr_hash = local_rec.std_hash;
		strcpy (cuit_rec.item_no, local_rec.dbt_item);
		strcpy (cuit_rec.item_desc, local_rec.dbt_desc);

		cc = abc_add (cuit, &cuit_rec);
		if (cc)
			file_err (cc, cuit, "DBADD");
	}
	strcpy (local_rec.last_dbt_no, local_rec.dbt_no);
}

/*
 * Screen Heading.
 */
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (mlDbMess189), (ScreenWidth -40)/2, 0, 1);

		print_at (0, ScreenWidth - 24,ML (mlDbMess188), local_rec.last_dbt_no);
		line_at (1,0,ScreenWidth - 1);

		switch (scn)
		{
		case  1 :
			box (0, 3, ScreenWidth - 1, 3);
			break;
		
		case  2 :
			break;

		}
		line_at (20,1,ScreenWidth - 1);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		fflush (stdout);
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
FindTableIndex (
	long	wsHhbrHash,
	char	*wsItemNo,
	int		wsMaxItems)
{
	int				i;

	for (i = 0; i < wsMaxItems; i++)
	{
		if ((wsHhbrHash != 0L) &&
		   (strcmp (wsItemNo, "                ") != 0))
		{
			if ((wsHhbrHash == store [i].hhbrHash) ||
			   (strcmp (wsItemNo, store [i].item_no) == 0))
				return (i);
		}
		if (wsHhbrHash != 0L)
		{
			if (wsHhbrHash == store [i].hhbrHash)
				return (i);
		}
		else
		{
			if (strcmp (wsItemNo, store [i].item_no) == 0)
				return (i);
		}
	}
	return (i);
}
