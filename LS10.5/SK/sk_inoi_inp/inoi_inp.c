/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: inoi_inp.c,v 5.9 2002/07/25 11:17:36 scott Exp $
|  Program Name  : (sk_inoi_inp.c)
|  Program Desc  : (Outlet Inventory maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (08/06/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: inoi_inp.c,v $
| Revision 5.9  2002/07/25 11:17:36  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.8  2002/07/24 08:39:13  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/06/20 07:11:01  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2001/11/20 07:35:21  scott
| Updated to not display tabular screen of no items exist for customer.
|
| Revision 5.5  2001/10/11 04:02:03  robert
| Updated to not allow entry (2) to be called when in OI_STAKE mode
|
| Revision 5.4  2001/09/24 03:32:01  robert
| Updated to not allow entry (2) to be called when in OI_DISPLAY mode
|
| Revision 5.3  2001/08/09 09:18:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:03  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:09  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inoi_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_inoi_inp/inoi_inp.c,v 5.9 2002/07/25 11:17:36 scott Exp $";

#define SLEEP_TIME	2
#define MAXLINES	500
#define TABLINES	10

#include <pslscr.h>
#include <ml_std_mess.h>

typedef int	BOOL;

#include <minimenu.h>

#define	UPDATE	0
#define	SEL_IGNORE	1
#define	SEL_DELETE	2
#define	DEFAULT	99

#define	INOI_MAINT		0
#define	INOI_DISPLAY	1
#define	INOI_STAKE		2

#define	OI_MAINT		 (ProgType == INOI_MAINT)
#define	OI_STAKE		 (ProgType == INOI_STAKE)
#define	OI_DISPLAY		 (ProgType == INOI_DISPLAY)

#define	OLD_LINE		 (line_cnt < old_lines)

#define LCL_SCR_WIDTH 130

	FILE	*fsort,
			*fout;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct cuitRecord	cuit_rec;
struct cumrRecord	cumr_rec;
struct inoiRecord	inoi_rec;

	int		envDbFind 		= 0,
			envDbCo 		= 0,
			ProgType 		= INOI_MAINT,
			lpno 			= 1,
			ReportRunning 	= FALSE,
			old_lines 		= 0;

	char 	branchNumber [3],
			systemDate [11];

	long	lsystemDate;

static char	*data  = "data",
			*inmr2 = "inmr2";

struct 	storeRec {
    long    hhbrHash;
} store [MAXLINES];

static char *scn_desc [] =
{
	"HEADER SCREEN",
	"ITEM DETAILS SCREEN"
};

/*============================
| Local & Screen Structures. |
============================*/
static	struct
{
	char	cust_no [7];
	char	item_no [17];
	long	stake_date;
	float	tot_cust;
	float	tot_disp;
	float	tot_store;
	float	otot_disp;
	float	otot_store;
	long	item_exp;
	long	lst_pullout;
	int		shelf_life;
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "cust_no",	2, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer Number    ", "Enter Customer Number, Full Search Available. ",
		NE, NO,  JUSTLEFT, "", "", local_rec.cust_no} ,
	{1, LIN, "cumr_dbt_name",	2, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name} ,
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Item Number   ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no} ,
	{2, TAB, "item_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "   I t e m    D e s c r i p t i o n.    ", "",
		NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{2, TAB, "hhbrHash",	0, 0, LONGTYPE,
		"NNNNNNNN", "          ",
		" ", " ", "", "",
		ND, NO,  JUSTLEFT, "", "", (char *)&inmr_rec.hhbr_hash},
	{2, TAB, "exp_date",	0, 0, INTTYPE,
		"NNNNNN", "          ",
		" ", " ", "", "",
		ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.shelf_life},
	{2, TAB, "tot_disp", 0, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Total Display.", " Enter total quantity in display area.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_disp},
	{2, TAB, "tot_store", 0, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Total Storage.", " Enter total quantity in storare area.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_store},
	{2, TAB, "tot_cust", 0, 2, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", "Tot. Customer.", "",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_cust},
	{2, TAB, "otot_disp", 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.otot_disp},
	{2, TAB, "otot_store", 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.otot_store},
	{2, TAB, "stake_date", 0, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, " Stock Take.", " Enter stock take date. <return> = today's date.",
		 NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.stake_date},
	{2, TAB, "item_exp", 0, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Expiry Date.", " Enter Expiry date.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.item_exp},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy} 
};

#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
static 	void shutdown_prog  (void);
static	void OpenDB 	 	(void);
static 	void CloseDB 		(void);
int  	spec_valid 			(int);
static 	void LoadOI 		(void);
static 	void UpdateOI 		(void);
static 	BOOL IsItemInTable 	(int, long);
int  	heading 			(int);
void 	OpenAudit 			(void);
void 	PrintAudit 			(void);
void 	CloseAudit 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	int		i;
	int		field;
	char	*sptr;

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	/*---------------------
	| Check program name. |
	---------------------*/
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	
	if (!strcmp (sptr, "sk_inoi_inp"))
		ProgType = INOI_MAINT;

	if (!strcmp (sptr, "sk_inoi_dsp"))
		ProgType = INOI_DISPLAY;

	if (!strcmp (sptr, "sk_inoi_stk"))
		ProgType = INOI_STAKE;

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	if (argc > 2)
		lpno = atoi (argv [1]);

	tab_row = 6;
	tab_col = 0;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR	 (vars);

	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	for (i = 0;i < 2;i++)
		tab_data [i]._desc = scn_desc [i];

	if (OI_DISPLAY)
	{
		for (field = label ("item_no"); FIELD.scn != 0; field++)
			if (FIELD.required != ND)
				FIELD.required = NA;
	}
	swide ();

	OpenDB ();
	
	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		lcount [2]	=	0;
		init_vars (1);
		/*-------------------------------
		| Enter screen 1 linear input
		-------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*------------------------
		| Screen 2 tabular input
		------------------------*/
		LoadOI ();

		if ((OI_DISPLAY || OI_STAKE) && !lcount [2])
		{
			print_mess (ML ("No item lines found."));
			sleep (sleepTime);
			continue;
		}

		heading (2);
		scn_display (2);
		if (!lcount [2] || OI_STAKE)
		{
			if (OI_STAKE)
			{
				FLD ("item_no") 	= NA;
				init_ok = TRUE;
				eoi_ok 	= FALSE;
			}
			entry (2);
		}
		else
			edit (2);

		if (prog_exit || restart)
			continue;

		vars [scn_start].row = MAXLINES;
		FLD ("item_no") 	= NO;

		edit_all ();

		if (!restart)
			UpdateOI ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);;
}

/*========================
| Program exit sequence. |
========================*/
static void 
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static void 
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inoi, inoi_list, INOI_NO_FIELDS, "inoi_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
							  							  : "cumr_id_no3");
	abc_alias (inmr2, inmr);
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
    open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cuit,  cuit_list, CUIT_NO_FIELDS, "cuit_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void 
CloseDB (
 void)
{
    abc_fclose (inmr2);
	abc_fclose (inmr);
	abc_fclose (inoi);
	abc_fclose (cumr);
	abc_fclose (cuit);
	abc_fclose (inei);

	SearchFindClose ();
	abc_dbclose (data);
}

int 
spec_valid (
 int field)
{
	/*-------------------------
	| Validate Customer No.
	-------------------------*/
	if (LCHECK ("cust_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return 0;
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.cust_no, 6));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		DSP_FLD ("cumr_dbt_name");
		return 0;
	}
	
	/*-------------------------
	| Validate Item number    |
	-------------------------*/
	if (LCHECK ("item_no"))
	{
		if (OI_STAKE && OLD_LINE)
		{
			getval (line_cnt);
			
			line_display ();
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch 
			 (
				comm_rec.co_no, 
				temp_str,
				cumr_rec.hhcu_hash,
				cumr_rec.item_codes
			);
			return 0;
	  	}

		cc = 	FindInmr 
				 (
					comm_rec.co_no, 
					local_rec.item_no,
					cumr_rec.hhcu_hash,
					cumr_rec.item_codes
				);
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{			
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}
		SuperSynonymError ();
	
		sprintf (local_rec.item_no, "%-16.16s", inmr_rec.item_no);

		if (IsItemInTable (vars [field].scn, inmr_rec.hhbr_hash))
		{
			errmess (ML (mlStdMess204));
			sleep (sleepTime);
			clear_mess ();
			return 1;
		}

		if (prog_status == ENTRY ||
			store [line_cnt].hhbrHash != inmr_rec.hhbr_hash)
		{
			/*-----------------------
			| Item added or changed
			-----------------------*/
			strcpy (local_rec.item_no, inmr_rec.item_no);
			store [line_cnt].hhbrHash = inmr_rec.hhbr_hash;
			line_display ();
		}
		DSP_FLD ("item_desc");
		return 0;
	}

	/*------------------------------------
	| Validate quantity in display area. |
	------------------------------------*/
	if (LCHECK ("tot_disp"))
	{
		if (OI_STAKE && dflt_used)
		{
			local_rec.tot_disp = local_rec.otot_disp;
			DSP_FLD ("tot_disp");
		}
			
		local_rec.tot_cust = local_rec.tot_disp + local_rec.tot_store;
		DSP_FLD ("tot_cust");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Validate quantity in storeage area. |
	-------------------------------------*/
	if (LCHECK ("tot_store"))
	{
		if (OI_STAKE && dflt_used)
		{
			local_rec.tot_store = local_rec.otot_store;
			DSP_FLD ("tot_store");
		}
		local_rec.tot_cust = local_rec.tot_disp + local_rec.tot_store;
		DSP_FLD ("tot_cust");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Stock take date. |
	---------------------------*/
	if (LCHECK ("stake_date"))
	{
		if (dflt_used)
			local_rec.stake_date = lsystemDate;
		
		DSP_FLD ("stake_date");
		
		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate Expiry date. |
	-----------------------*/
	if (LCHECK ("item_exp"))
	{
		if (dflt_used && local_rec.shelf_life)
			local_rec.item_exp = lsystemDate + (long) local_rec.shelf_life;
		
		DSP_FLD ("item_exp");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*====================================
| Load Outlet inventory information. |
====================================*/
static void 
LoadOI (
 void)
{
	old_lines = 0;
			
	print_mess (ML (mlStdMess035));

    /*-----------------------
    | Set screen for putval |
    -----------------------*/
    scn_set (2);
    init_vars (2);

	memset (store, 0, sizeof store);
    lcount [2] = 0;

	inoi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	inoi_rec.hhbr_hash	=	0L;
	inoi_rec.stake_date	=	0L;
	
    cc = find_rec (inoi, &inoi_rec, GTEQ, "r");
    while (	!cc && inoi_rec.hhcu_hash == cumr_rec.hhcu_hash)
    {
		inmr_rec.hhbr_hash = inoi_rec.hhbr_hash;
        cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
		if (cc)
		{
    		cc = find_rec (inoi, &inoi_rec, NEXT, "r");
			continue;
		}
		inei_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		local_rec.shelf_life = (cc) ? 0: inei_rec.expiry_prd1 * 30;

		strcpy (local_rec.item_no, inmr_rec.item_no);

		local_rec.tot_disp		= inoi_rec.tot_disp;
		local_rec.tot_store		= inoi_rec.tot_store;
		local_rec.item_exp		= inoi_rec.item_exp;
		local_rec.lst_pullout	= inoi_rec.lst_pullout;
		local_rec.stake_date	= inoi_rec.stake_date;

		if (OI_STAKE)
		{
			local_rec.otot_disp		= local_rec.tot_disp;
			local_rec.otot_store	= local_rec.tot_store;
			local_rec.stake_date	= lsystemDate;
			if (local_rec.shelf_life)
				local_rec.item_exp = lsystemDate + (long) local_rec.shelf_life;
			else
				local_rec.item_exp = 0L;

			local_rec.lst_pullout	= 0L;
		}
		local_rec.tot_cust		= inoi_rec.tot_store + inoi_rec.tot_disp;

        /*--------------------------------
        | Put Value Into Tabular Screen. |
        --------------------------------*/
		store [lcount [2]].hhbrHash = inoi_rec.hhbr_hash;

		putval (lcount [2]++);

		if (lcount [2] == MAXLINES)
		{
			errmess (ML (mlStdMess008));
			sleep (sleepTime);
			clear_mess ();
			break;
		}
        cc = find_rec (inoi, &inoi_rec, NEXT, "r");
    }

	if (OI_STAKE)
	{
		old_lines = lcount [2];
		vars [scn_start].row = lcount [2];
	}

	clear_mess ();
}

/*===============================
| Update detail records in inoi | 
===============================*/
static void	
UpdateOI (
 void)
{
	int		NewOI;

	if (OI_DISPLAY)
		return;

	ReportRunning = FALSE;

	print_mess (ML (mlStdMess035));

    scn_set (2);

	/*--------------------
	| Write new records. |
	--------------------*/
   	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
   	{
    	getval (line_cnt);

		/*-------------------------
		| Remove existing records |
		-------------------------*/
		inoi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		inoi_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inoi_rec.stake_date = 	local_rec.stake_date;
		NewOI = find_rec (inoi, &inoi_rec, COMPARISON, "r");
	
		inoi_rec.tot_disp 		=	local_rec.tot_disp;
		inoi_rec.tot_store 		=	local_rec.tot_store;
		inoi_rec.stake_date		=	local_rec.stake_date;
		inoi_rec.item_exp 		=	local_rec.item_exp;
		inoi_rec.lst_pullout 	=	local_rec.lst_pullout;
		
		if (NewOI)
			cc = abc_add (inoi, &inoi_rec);
		else
			cc = abc_update (inoi, &inoi_rec);

		if (cc)
			file_err (cc, inoi, "DBADD/DBUPDATE");

		/*--------------
		| Print Audit. |
		--------------*/
		PrintAudit ();
	}
	clear_mess ();

	/*--------------------
	| Close Print Audit. |
	--------------------*/
	if (ReportRunning)
		CloseAudit ();
}

/*===============================================
| Return TRUE if item has been entered in table |
===============================================*/
static BOOL 
IsItemInTable (
 int scn, 
 long hhbrHash)
{
	int i, last;

	last = (prog_status == ENTRY ? line_cnt : lcount [scn]);

	for (i = 0; i < last; i++)
	{
		if (hhbrHash == store [i].hhbrHash && i != line_cnt)
			return TRUE;
	}
	return FALSE;
}

/*===========================
| edit () callback function |
===========================*/
int 
heading (
 int scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	switch (scn)
	{
	case 1:
		clear ();
			
		if (OI_MAINT)
			strcpy (err_str, ML (" Outlet Inventory Maintenance "));

		if (OI_DISPLAY)
			strcpy (err_str, ML (" Outlet Inventory Display "));

		if (OI_STAKE)
			strcpy (err_str, ML (" Outlet Inventory Stock Take Entry "));

		rv_pr (err_str, ( (LCL_SCR_WIDTH - (int) strlen (err_str)) / 2), 0, 1);
		box (0, 1, LCL_SCR_WIDTH, 1);

		move (0, 19); line (LCL_SCR_WIDTH);
		print_at (20,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
		move (0, 22); line (LCL_SCR_WIDTH);
		break;
		
	case 2:
		/*---------------------------------------------------------
		| Other screens must be redrawn if this screen is redrawn |
		---------------------------------------------------------*/
		heading (1);
		scn_display (1);

		break;

	default:
		break;
	}
	scn_write (scn);	/* Draw prompts only */
    return (EXIT_SUCCESS);
}
/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if (ReportRunning)
		return;
		
	ReportRunning = TRUE;

	if ( (fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",lpno);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".11\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".ECUSTOMER STOCK TAKE AUDIT AS AT %s", SystemTime ());
	fprintf (fout, ".EFOR %s - %s\n", cumr_rec.dbt_no, cumr_rec.dbt_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany %s\n",clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  %s\n", clip (comm_rec.est_name));

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "===========");
	fprintf (fout, "============\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|           ITEM DESCRIPTION               ");
	fprintf (fout, "|  TOTAL DISPLAY ");
	fprintf (fout, "| TOTAL STORAGE  ");
	fprintf (fout, "| TOTAL CUSTOMER ");
	fprintf (fout, "|STOCK TAKE");
	fprintf (fout, "| EXPIRY   |\n");

	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|   STOCK HELD   ");
	fprintf (fout, "|   STOCK HELD   ");
	fprintf (fout, "|   STOCK HELD.  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   DATE   |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");
}

/*======================================================
| Print Audit of Commission for Each Salesman/Invoice. |
======================================================*/
void
PrintAudit (
 void)
{
	/*-------------------
	| Open print Audit. |
	-------------------*/
	OpenAudit ();

	fprintf (fout, "| %16.16s ", 	local_rec.item_no);
	fprintf (fout, "| %40.40s ", 	inmr_rec.description);
	fprintf (fout, "| %14.2f ",		local_rec.tot_disp);
	fprintf (fout, "| %14.2f ",		local_rec.tot_store); 
	fprintf (fout, "| %14.2f ",		local_rec.tot_cust); 
	fprintf (fout, "|%10.10s",		DateToString (local_rec.stake_date));
	fprintf (fout, "|%10.10s|\n",DateToString (local_rec.item_exp));
}

/*=====================
| Print Page footers. |
=====================*/
void
CloseAudit (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

