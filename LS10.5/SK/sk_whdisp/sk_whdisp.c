/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_whdisp.c,v 5.6 2002/12/01 04:48:18 scott Exp $
|  Program Name  : (sk_whdisp.c)
|  Program Desc  : (Display all Inventory Branch records)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_whdisp.c,v $
| Revision 5.6  2002/12/01 04:48:18  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.5  2002/11/28 02:37:08  kaarlo
| LS01007 SC4024. Updated to fix this problem ==> "Defaults to item sam even when no default is set in Environment".
|
| Revision 5.4  2001/08/28 10:12:35  robert
| additional update for LS10.5-GUI
|
| Revision 5.3  2001/08/09 09:20:25  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:46:10  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:40  scott
| Update - LS10.5
|
| Revision 4.3  2001/05/08 07:08:18  robert
| Updated for the modified LS10-GUI RingMenu
|
| Revision 4.2  2001/04/05 00:51:49  scott
| Updated to use LTEQ and PREVIOUS calls to work with SQL and CISAM
|
| Revision 4.1  2001/03/22 05:38:53  scott
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_whdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_whdisp/sk_whdisp.c,v 5.6 2002/12/01 04:48:18 scott Exp $";

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

#define	TXT_REQD
#include	<pslscr.h>
#include	<hot_keys.h>
#include	<ring_menu.h>
#include	<chk_ring_sec.h>
#include	<graph.h>
#include	<getnum.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inprRecord	inpr_rec;

	float	*incc_cons	=	&incc_rec.c_1;

	char	*data	= "data",
			*inmr2	= "inmr2";

	char	*sixteenSpaces = "                ",
			envVarCurrCode [4];

	int		MTD = TRUE;
	int		numPrices;

struct
{
	int		wh_active;		/* TRUE if incc found	*/
	char	wh_est_no [3];
	char	wh_acronym [10];
	long	wh_hhcc_hash;
	float	wh_open_stock,
			wh_mtd_pur,
			wh_mtd_iss,
			wh_mtd_rec,
			wh_mtd_adj,
			wh_mtd_sal,
			wh_close_stock,
			wh_ytd_pur,
			wh_ytd_iss,
			wh_ytd_rec,
			wh_ytd_adj,
			wh_ytd_sal,
			wh_yt_sales,
			wh_min_stk,
			wh_max_stk;
	double	wh_slct_cost;	/* FIFO/AVGE. (Depends on inmr_costing_flag) */
} wh [101];

	int	valid_idxs [12];
	char	sup_part [17];
	char	wk_part [17];

char	*std_foot = " [PRINT] [NEXT] [PREV] [EDIT/END] ";

/*=======================
| Callback Declarations |
=======================*/
int ClearRedraw 	(void);
int headPrint 		(void);
int nextDisplay 	(void);
int prevDisplay 	(void);
int MtdSelect 		(void);
int YtdSelect 		(void);

#ifndef GVISION
menu_type	_main_menu [] =
{
	{" [REDRAW]", "Redraw Display", ClearRedraw, "", FN3,			},
	{" [PRINT]", "Print Screen", headPrint, "", FN5,			},
	{" [NEXT SCREEN]", "Display Next Item", nextDisplay, "", FN14,		},
	{" [PREV SCREEN]", "Display Previous Item", prevDisplay, "", FN15,	},
	{" [INPUT/END]", "Exit Display", _no_option, "", FN16, EXIT | SELECT	},
	{"<MTD>", "Display Month to Date Values", MtdSelect, "Mm", },
	{"<YTD>", "Display Year to Date Values", YtdSelect, "Yy", },
	{"",								},
};
#else
menu_type	_main_menu [] =
{
	{0, " [REDRAW]", "Redraw Display", ClearRedraw, FN3,			},
	{0, " [PRINT]", "Print Screen", headPrint, FN5,			},
	{0, " [NEXT SCREEN]", "Display Next Item", nextDisplay, FN14,		},
	{0, " [PREV SCREEN]", "Display Previous Item", prevDisplay, FN15,	},
	{0, "<MTD>", "Display Month to Date Values", MtdSelect, },
	{0, "<YTD>", "Display Year to Date Values", YtdSelect, },
	{0, "",								},
};
#endif

char	*month_nm [] =
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
	"",
};

	char	dspString [20] [200],
			head_text [134];
	extern int		lp_x_off,
			lp_y_off;
	int		wh_idx,
			firstTime 		= TRUE,
			mainWindowOpen 	= FALSE,
			hashPassed 		= TRUE,
			displayOk 		= TRUE,
			clearOk 		= TRUE;
	long	hhbrPassed 		= 0L;
	double	itemPrice [9]	= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

struct
{
	char	dummy [11];
	char	slct_prmt [14];
	char	prev_item [17];
} local_rec;


    extern int _wide;

#include	<get_lpno.h>
#include    <FindInmr.h>

/*=======================
| Function Declarations |
=======================*/
int  	heading 			(int);
int  	spec_valid 			(int);
int  	GetItemNumber 		(void);
void	PrintCoStuff 		(void); 
void 	shutdown_prog 		(void);
void 	ReadMisc 			(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	FindSuper 			(char *);
void 	ChangeData 			(int);
void 	LoadData 			(void);
void 	ShellProgram 		(int);
void 	ReDraw 				(void);
void 	AllDisplay 			(void);
void 	GetPrices 			(void);
void 	PrintCompanyStuff 	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc == 2)
	{
		hashPassed = TRUE;
		hhbrPassed = atol (argv [1]);
	}

	input_row = 20;

	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf (envVarCurrCode, "%-3.3s", get_env ("CURR_CODE"));
	
	/*----------------------------
	| Get number of price types. |
	----------------------------*/
	sptr = chk_env ("SK_DBPRINUM");
	numPrices = (sptr == (char *)0) ? 9 : atoi (sptr);
	
	init_scr ();
	set_tty ();

	swide ();

	OpenDB ();
	ReadMisc ();
/*
	_chk_ring_sec (_main_menu, "sk_fulldisp");
*/

	sprintf (local_rec.prev_item, "%16s", " ");

	while (prog_exit == 0)
	{
		crsr_off ();
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		displayOk 	= FALSE;
		clearOk 	= TRUE;
		search_ok 	= TRUE;

		if (firstTime)
		{
			heading (1);
			firstTime = FALSE;
		}
		else
		{
			move (0, 20);
			cl_line ();
			move (0, 21);
			cl_line ();
		}

		GetItemNumber ();
		if (prog_exit || restart)
			continue;

		displayOk = TRUE;
		clearOk = FALSE;
		heading (1);
#ifndef GVISION
		run_menu (_main_menu, "", input_row);
#else
        run_menu (NULL, _main_menu);
#endif
		strcpy (local_rec.prev_item, inmr_rec.item_no);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{

	wh_idx = 0;
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, "  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (wh [wh_idx].wh_est_no,	ccmr_rec.est_no);
		strcpy (wh [wh_idx].wh_acronym,	clip (ccmr_rec.acronym));
		wh [wh_idx].wh_hhcc_hash		= ccmr_rec.hhcc_hash;
		wh [wh_idx].wh_active		= FALSE;
		wh [wh_idx].wh_open_stock	= 0.00;
		wh [wh_idx].wh_mtd_pur		= 0.00;
		wh [wh_idx].wh_mtd_iss		= 0.00;
		wh [wh_idx].wh_mtd_rec		= 0.00;
		wh [wh_idx].wh_mtd_adj		= 0.00;
		wh [wh_idx].wh_mtd_sal		= 0.00;
		wh [wh_idx].wh_close_stock	= 0.00;
		wh [wh_idx].wh_ytd_pur		= 0.00;
		wh [wh_idx].wh_ytd_iss		= 0.00;
		wh [wh_idx].wh_ytd_rec		= 0.00;
		wh [wh_idx].wh_ytd_adj		= 0.00;
		wh [wh_idx].wh_ytd_sal		= 0.00;
		wh [wh_idx].wh_yt_sales		= 0.00;
		wh [wh_idx].wh_min_stk		= 0.00;
		wh [wh_idx].wh_max_stk		= 0.00;
		wh [wh_idx].wh_slct_cost		= 0.00;
		wh_idx++;
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	};
	abc_fclose (ccmr);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);

	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inpr,  inpr_list, INPR_NO_FIELDS, "inpr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inpr);
	abc_fclose (inmr2);
	CloseCosting ();
	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (clearOk)
		{
			swide ();
			clear ();
		}
		crsr_off ();
		rv_pr (ML (mlSkMess505), 49, 0, 1);

		print_at (0, 104,ML (mlSkMess096), local_rec.prev_item);

		move (0, 20);
		cl_line ();
		move (0, 21);
		cl_line ();
	}

	if (displayOk)
	{
		AllDisplay ();
		clearOk = FALSE;
	}
	else
		box (0, 1, 132, 17);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	return (EXIT_SUCCESS);
}

int
GetItemNumber (
 void)
{
	last_char = 0;

	while (TRUE)
	{
		crsr_on ();
		print_at (20, 3,ML (mlSkMess097));

		if (hashPassed)
		{
			hashPassed = FALSE;
			abc_selfield (inmr, "inmr_hhbr_hash");
			inmr_rec.hhbr_hash = hhbrPassed;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
			if (cc || hhbrPassed == 0)
				getalpha (20, 20, "UUUUUUUUUUUUUUUU", (char *) wk_part);
			else
				strcpy (wk_part, inmr_rec.item_no);
			abc_selfield (inmr, "inmr_id_no");
		}
		else
			getalpha (20, 20, "UUUUUUUUUUUUUUUU", (char *) wk_part);
		sprintf (inmr_rec.item_no, "%-16.16s", wk_part);

		crsr_off ();
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		if (last_char == FN1)
		{
			restart = TRUE;
			return (EXIT_SUCCESS);
		}

		if (last_char == FN_HELP || last_char == HELP ||
		     last_char == HELP1 || last_char == EXIT_OUT ||
		     (last_char > FN16 && last_char <= FN32))
		{
			ShellProgram (last_char);
			heading (1);
			continue;
		}
		if (last_char == REDRAW)
		{
			heading (1);
			continue;
		}
		if (SRCH_KEY)
		{
			/*---------------------------
			| by company search			|
			----------------------------*/
			strcpy (temp_str, wk_part);
			InmrSearch (comm_rec.co_no, temp_str,0L,"N");
			strcpy (wk_part, clip (temp_str));
			heading (1);
			continue;
		}
		/*-----------------------
		| find item				|
		------------------------*/
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		strcpy (wk_part,inmr_rec.item_no);

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		/*---------------------------
		| find failed				|
		---------------------------*/
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			abc_selfield (inmr,"inmr_id_no");
			continue;
		}
		strcpy (wk_part,inmr_rec.item_no);

		/*---------------------------
		| find supercession			|
		---------------------------*/
		sprintf (inmr2_rec.item_no, "%-16.16s", " ");
		sprintf (inmr2_rec.description, "%-40.40s", " ");
		FindSuper (inmr_rec.supercession);
	
		line_at (22,0,132);
		if (strcmp (inmr2_rec.item_no, sixteenSpaces))
		{
			sprintf (err_str,ML (mlSkMess532),inmr_rec.item_no,inmr2_rec.item_no); 
			rv_pr (err_str,28,22,1);
		}

		/*-----------------------------------
		| load the data appropriately		|
		-----------------------------------*/
		LoadData ();
		return (EXIT_SUCCESS);
	}
}

/*===============================
| Validate Supercession Number. |
===============================*/
void
FindSuper (
 char *item_no)
{
	/*---------------------------------------
	| end of supercession chain		        |
	---------------------------------------*/
	if (!strcmp (item_no,"                "))
		return;

	/*---------------------------------------
	| search next link in chain		        |
	---------------------------------------*/
	strcpy (inmr2_rec.co_no,comm_rec.co_no);
	strcpy (inmr2_rec.item_no,item_no);
	cc = find_rec (inmr2,&inmr2_rec,COMPARISON,"r");
	if (!cc)
		FindSuper (inmr2_rec.supercession);
}

int
ClearRedraw (
 void)
{
	clear ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

/*===========================
| display next item			|
===========================*/
int
nextDisplay (
 void)
{
	ChangeData (FN14);
	ReDraw ();
    return (EXIT_SUCCESS);
}

/*===============================
| display prevoius item			|
===============================*/
int
prevDisplay (
 void)
{
	ChangeData (FN15);
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
ChangeData (
 int key)
{
	/*---------------------------------------
	| perform find in appropriate direction	|
	---------------------------------------*/
	strcpy (local_rec.prev_item, inmr_rec.item_no);
	cc = find_rec (inmr, &inmr_rec, (key == FN14) ? GTEQ : LTEQ, "r");
	if (!cc)
		cc = find_rec (inmr, &inmr_rec, (key == FN14) ? NEXT : PREVIOUS, "r");
	if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.prev_item);
		cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	}
	strcpy (wk_part,inmr_rec.item_no);

	/*---------------------------------------
	| find supercession					    |
	---------------------------------------*/
	sprintf (inmr2_rec.item_no, "%-16.16s", " ");
	sprintf (inmr2_rec.description, "%-40.40s", " ");
	FindSuper (inmr_rec.supercession);

	line_at (22,0,132);
	if (strcmp (inmr2_rec.item_no, sixteenSpaces))
	{
		sprintf (err_str,ML (mlSkMess532),inmr_rec.item_no,inmr2_rec.item_no); 
		rv_pr (err_str,28,22,1);
	}
	LoadData ();
}

void
LoadData (
 void)
{
	int		i,
			wh_no,
			loop;
	double	co_cost;

	wh [100].wh_active		= 0;
	wh [100].wh_open_stock	= 0.00;
	wh [100].wh_mtd_pur		= 0.00;
	wh [100].wh_mtd_iss		= 0.00;
	wh [100].wh_mtd_rec		= 0.00;
	wh [100].wh_mtd_adj		= 0.00;
	wh [100].wh_mtd_sal		= 0.00;
	wh [100].wh_close_stock	= 0.00;
	wh [100].wh_ytd_pur		= 0.00;
	wh [100].wh_ytd_iss		= 0.00;
	wh [100].wh_ytd_rec		= 0.00;
	wh [100].wh_ytd_adj		= 0.00;
	wh [100].wh_ytd_sal		= 0.00;
	wh [100].wh_yt_sales	= 0.00;
	wh [100].wh_min_stk		= 0.00;
	wh [100].wh_max_stk		= 0.00;
	wh [100].wh_slct_cost	= 0.00;

	for (wh_no = 0; wh_no < 12; wh_no++)
		valid_idxs [wh_no] = -1;

	for (wh_no = loop = 0; loop < wh_idx; loop++)
	{
	    if (ineiRec.hhbr_hash != inmr_rec.hhbr_hash ||
		strcmp (ineiRec.est_no, wh [loop].wh_est_no))
	    {
			if (FindInei (inmr_rec.hhbr_hash, wh [loop].wh_est_no, "r"))
			{
				ineiRec.avge_cost = 0.00;
				ineiRec.last_cost = 0.00;
				ineiRec.prev_cost = 0.00;
				ineiRec.min_stock = 0.00;
				ineiRec.max_stock = 0.00;
			}
	    }
	    incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	    incc_rec.hhcc_hash = wh [loop].wh_hhcc_hash;
	    cc = find_rec (incc, &incc_rec, EQUAL, "r");
	    if (cc)
	    {
			wh [loop].wh_active			= FALSE;
			wh [loop].wh_open_stock		= 0.00;
			wh [loop].wh_mtd_pur		= 0.00;
			wh [loop].wh_mtd_iss		= 0.00;
			wh [loop].wh_mtd_rec		= 0.00;
			wh [loop].wh_mtd_adj		= 0.00;
			wh [loop].wh_mtd_sal		= 0.00;
			wh [loop].wh_close_stock	= 0.00;
			wh [loop].wh_ytd_pur		= 0.00;
			wh [loop].wh_ytd_iss		= 0.00;
			wh [loop].wh_ytd_rec		= 0.00;
			wh [loop].wh_ytd_adj		= 0.00;
			wh [loop].wh_ytd_sal		= 0.00;
			wh [loop].wh_yt_sales		= 0.00;
			wh [loop].wh_min_stk		= 0.00;
			wh [loop].wh_max_stk		= 0.00;
			wh [loop].wh_slct_cost		= 0.00;
	    }
	    else
	    {
			if (wh_no < 10)
				valid_idxs [wh_no] = loop;

			wh_no++;
			wh [loop].wh_active			= TRUE;
			wh [loop].wh_open_stock		= incc_rec.opening_stock;
			wh [loop].wh_mtd_pur		= incc_rec.pur;
			wh [loop].wh_mtd_iss		= incc_rec.issues;
			wh [loop].wh_mtd_rec		= incc_rec.receipts;
			wh [loop].wh_mtd_adj		= incc_rec.adj;
			wh [loop].wh_mtd_sal		= incc_rec.sales;
			wh [loop].wh_close_stock	= incc_rec.closing_stock;
			wh [loop].wh_ytd_pur		= incc_rec.ytd_pur;
			wh [loop].wh_ytd_iss		= incc_rec.ytd_issues;
			wh [loop].wh_ytd_rec		= incc_rec.ytd_receipts;
			wh [loop].wh_ytd_adj		= incc_rec.ytd_adj;
			wh [loop].wh_ytd_sal		= incc_rec.ytd_sales;
			wh [loop].wh_yt_sales		= 0.00;
			for (i = 0; i < 12; i++)
				wh [loop].wh_yt_sales	+= incc_cons [i];

			wh [loop].wh_min_stk	= ineiRec.min_stock;
			wh [loop].wh_max_stk	= ineiRec.max_stock;

		switch (inmr_rec.costing_flag [0])
		{
		case 'L':
		    strcpy (local_rec.slct_prmt, ML ("Last Cost"));
		    wh [loop].wh_slct_cost = ineiRec.last_cost;
		    break;

		case 'A':
		    strcpy (local_rec.slct_prmt, ML ("Average Cost"));
		    wh [loop].wh_slct_cost = ineiRec.avge_cost;
		    break;

		case 'F':
		    strcpy (local_rec.slct_prmt, ML ("FIFO Cost"));
		    wh [loop].wh_slct_cost = FindIncfValue (incc_rec.hhwh_hash, incc_rec.closing_stock, TRUE, TRUE, inmr_rec.dec_pt);
		    break;

		case 'I':
		    strcpy (local_rec.slct_prmt, ML ("LIFO Cost"));
		    wh [loop].wh_slct_cost = FindIncfValue (incc_rec.hhwh_hash, incc_rec.closing_stock, TRUE, FALSE, inmr_rec.dec_pt);
		    break;

		case 'S':
		    strcpy (local_rec.slct_prmt, ML ("Serial Cost"));
		    wh [loop].wh_slct_cost = FindInsfValue (incc_rec.hhwh_hash, TRUE);
		    break;

		default:
		    break;
		}

		if (wh [loop].wh_slct_cost == 0.00)
		    wh [loop].wh_slct_cost = ineiRec.avge_cost;

		if (wh [loop].wh_slct_cost == 0.00)
		    wh [loop].wh_slct_cost = ineiRec.last_cost;

		wh [100].wh_active++;
		wh [100].wh_open_stock		+= wh [loop].wh_open_stock;
		wh [100].wh_mtd_pur			+= wh [loop].wh_mtd_pur;
		wh [100].wh_mtd_iss			+= wh [loop].wh_mtd_iss;
		wh [100].wh_mtd_rec			+= wh [loop].wh_mtd_rec;
		wh [100].wh_mtd_adj			+= wh [loop].wh_mtd_adj;
		wh [100].wh_mtd_sal			+= wh [loop].wh_mtd_sal;
		wh [100].wh_close_stock		+= wh [loop].wh_close_stock;
		wh [100].wh_ytd_pur			+= wh [loop].wh_ytd_pur;
		wh [100].wh_ytd_iss			+= wh [loop].wh_ytd_iss;
		wh [100].wh_ytd_rec			+= wh [loop].wh_ytd_rec;
		wh [100].wh_ytd_adj			+= wh [loop].wh_ytd_adj;
		wh [100].wh_ytd_sal			+= wh [loop].wh_ytd_sal;
		wh [100].wh_yt_sales		+= wh [loop].wh_yt_sales;
		wh [100].wh_min_stk			+= wh [loop].wh_min_stk;
		wh [100].wh_max_stk			+= wh [loop].wh_max_stk;
		co_cost = wh [loop].wh_slct_cost * wh [loop].wh_close_stock;
		wh [100].wh_slct_cost		+= co_cost;
	    }
	}
	if (wh [100].wh_close_stock != 0.00)
		wh [100].wh_slct_cost /= wh [100].wh_close_stock;
}

void
ShellProgram (
 int sh_type)
{
	int	indx = 0;
	int	status;
	char	wk_out [10];
	char	*wk_ptr;
	char	*sptr;
	char	*curr_user;

	/*--------------------------------------------------------------
	| Only print message if shell_prog is run from inside scrgen.c |
	--------------------------------------------------------------*/
	if (sh_type == FN_HELP || sh_type == HELP || sh_type == EXIT_OUT || sh_type == HELP1 || sh_type > FN16)
		rv_pr (ML (mlStdMess035), 0, 1, 1);

	/*-------------------------------------------------------
	| Shell out option for program window and program help. |
	-------------------------------------------------------*/
	if (sh_type == EXIT_OUT || sh_type == HELP || sh_type == HELP1)
	{
		if (sh_type == EXIT_OUT)
			*(arg) = "run_extern";
		else
			*(arg) = "prog_help";

		*(arg+ (1)) = PNAME;
		*(arg+ (2)) = (char *) 0;
	}
	/*-------------------------------------------------------
	| Shell out option for program window and program help. |
	-------------------------------------------------------*/
	if (sh_type > FN16 && sh_type < FN32)
	{
		curr_user = getenv ("LOGNAME");
		sptr = strdup (curr_user);
		upshift (sptr);
		sprintf (wk_out, "%s.FN%d", sptr, sh_type - (FN1 - 1));
		free (sptr);
		wk_ptr = chk_env (wk_out);

		if (wk_ptr == (char *) 0)
		{
			sprintf (wk_out, "FN%d", sh_type - (FN1 - 1));
			wk_ptr = chk_env (wk_out);
		}

		if (wk_ptr == (char *) 0)
			return;

		if (* (wk_ptr + strlen (wk_ptr) - 1) == '~')
			* (wk_ptr + strlen (wk_ptr) - 1) = '\0';

		sptr = strchr (wk_ptr, '~');

		while (sptr != (char *) 0)
		{
			*sptr = '\0';
			* (arg + (indx++)) = wk_ptr;

			wk_ptr = sptr + 1;
			sptr = strchr (wk_ptr, '~');
		}

		* (arg + (indx++)) = wk_ptr;

		* (arg + (indx)) = (char *) 0;
	}
	/*-----------------------------------------
	| Shell out option for function key help. |
	-----------------------------------------*/
	if (sh_type == FN_HELP)
	{
		* (arg) = "help_fn";
		* (arg + (1)) = (char *) 0;
	}

	if ((fork ()) == 0)
	{
		status = execvp (arg [0], arg);
		prog_exit = 1;
        return;
	}
	else
		wait (&status);

	set_tty ();

	if ((sh_type == FN_HELP || sh_type == HELP || sh_type == EXIT_OUT || sh_type == HELP1 || sh_type > FN16) /*&& _wide*/)
		swide ();
}

void
ReDraw (
 void)
{
	clearOk = FALSE;
	heading (1);
}

int
headPrint (
 void)
{
	lp_x_off = 0;
	lp_y_off = 1;
	Dsp_print ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
AllDisplay (
 void)
{
	int		i;
	int		loop;
	int		wh_no;
	char	priceStr [9] [30];

	crsr_off ();
	/*-------------------
	| display misc data |
	-------------------*/
	displayOk = 1;

	/*---------------------
	| print data for item |
	---------------------*/
	sprintf (head_text, ". Item Number  : %16.16s  (%40.40s)                                                      ",
		inmr_rec.item_no,
		inmr_rec.description);

	if (mainWindowOpen)
		Dsp_close ();

	mainWindowOpen = TRUE;

	Dsp_nc_prn_open (0, 1, 14, head_text,
				comm_rec.co_no, comm_rec.co_name,
				comm_rec.est_no, comm_rec.est_short,
				 (char *) 0, (char *) 0);

	Dsp_saverec (head_text);
	sprintf (dspString [0], "             |  COMPANY");
	sprintf (dspString [1], "Opening Stock^E^1%9.2f^6", wh [100].wh_open_stock);
	if (MTD)
	{
		sprintf (dspString [2], "MTD Purchases^E^1%9.2f^6", wh [100].wh_mtd_pur);
		sprintf (dspString [3], "MTD Receipts ^E^1%9.2f^6", wh [100].wh_mtd_rec);
		sprintf (dspString [4], "MTD Issues   ^E^1%9.2f^6", wh [100].wh_mtd_iss);
		sprintf (dspString [5], "MTD Adjustmts^E^1%9.2f^6", wh [100].wh_mtd_adj);
		sprintf (dspString [6], "MTD Sales    ^E^1%9.2f^6", wh [100].wh_mtd_sal);
	}
	else
	{
		sprintf (dspString [2], "YTD Purchases^E^1%9.2f^6", wh [100].wh_ytd_pur);
		sprintf (dspString [3], "YTD Receipts ^E^1%9.2f^6", wh [100].wh_ytd_rec);
		sprintf (dspString [4], "YTD Issues   ^E^1%9.2f^6", wh [100].wh_ytd_iss);
		sprintf (dspString [5], "YTD Adjustmts^E^1%9.2f^6", wh [100].wh_ytd_adj);
		sprintf (dspString [6], "YTD Sales    ^E^1%9.2f^6", wh [100].wh_ytd_sal);
	}
	sprintf (dspString [7], "Closing Stock^E^1%9.2f^6", wh [100].wh_close_stock);

	strcpy  (dspString [8], "^^GGGGGGGGGGGGGEGGGGGGGGG");
	sprintf (dspString [9], "12 Mth Sales ^E^1%9.2f^6", wh [100].wh_yt_sales);

	sprintf (dspString [10], "%-13.13s^E^1%9.2f^6", local_rec.slct_prmt, wh [100].wh_slct_cost);
	sprintf (dspString [11], "Min/Max Stock^E^1%4.0f/%4.0f^6", wh [100].wh_min_stk, wh [100].wh_max_stk);
	strcpy  (dspString [12], "^^GGGGGGGGGGGGGJGGGGGGGGG");

	/*------------------------
	| Read prices from inpr. |
	------------------------*/
	GetPrices ();

	/*-----------------------
	| Set up price strings. |
	-----------------------*/
	for (i = 0; i < 9; i++)
	{
		if (i >= numPrices)
		{
			sprintf (priceStr [i], "%-20.20s", " ");
			continue;
		}
		switch (i)
		{
		case 0:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price1_desc, DOLLARS (itemPrice [i]));
			break;

		case 1:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price2_desc, DOLLARS (itemPrice [i]));
			break;

		case 2:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price3_desc, DOLLARS (itemPrice [i]));
			break;

		case 3:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price4_desc, DOLLARS (itemPrice [i]));
			break;

		case 4:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price5_desc, DOLLARS (itemPrice [i]));
			break;

		case 5:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price6_desc, DOLLARS (itemPrice [i]));
			break;

		case 6:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price7_desc, DOLLARS (itemPrice [i]));
			break;

		case 7:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price8_desc, DOLLARS (itemPrice [i]));
			break;

		case 8:
			sprintf (priceStr [i], "^1%-10.10s^6:%9.2f", 
							comm_rec.price9_desc, DOLLARS (itemPrice [i]));
			break;
		}
	}
	
	sprintf (dspString [13], 
			"%-24.24s  %-24.24s  %-24.24s  %-24.24s  %-24.24s",
			priceStr [0],
			priceStr [1],
			priceStr [2],
			priceStr [3],
			priceStr [4]);

	sprintf (dspString [14], 
			"%-24.24s  %-24.24s  %-24.24s  %-24.24s",
			priceStr [5],
			priceStr [6],
			priceStr [7],
			priceStr [8]);

	for (loop = 0; loop < 12; loop++)
	{
		wh_no = valid_idxs [loop];
		if (wh_no == -1)
			continue;

		sprintf (err_str, "|%9.9s", wh [wh_no].wh_acronym);
		strcat (dspString [0], err_str);
		sprintf (err_str, "^E%9.2f", wh [wh_no].wh_open_stock);
		strcat (dspString [1], err_str);
		if (MTD)
		{
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_mtd_pur);
			strcat (dspString [2], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_mtd_rec);
			strcat (dspString [3], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_mtd_iss);
			strcat (dspString [4], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_mtd_adj);
			strcat (dspString [5], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_mtd_sal);
			strcat (dspString [6], err_str);
		}
		else
		{
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_ytd_pur);
			strcat (dspString [2], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_ytd_rec);
			strcat (dspString [3], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_ytd_iss);
			strcat (dspString [4], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_ytd_adj);
			strcat (dspString [5], err_str);
			sprintf (err_str, "^E%9.2f", wh [wh_no].wh_ytd_sal);
			strcat (dspString [6], err_str);
		}
		sprintf (err_str, "^E%9.2f", wh [wh_no].wh_close_stock);
		strcat (dspString [7], err_str);

		strcat (dspString [8], "EGGGGGGGGG");

		sprintf (err_str, "^E%9.2f", wh [wh_no].wh_yt_sales);
		strcat (dspString [9], err_str);

		sprintf (err_str, "^E%9.2f", wh [wh_no].wh_slct_cost);

		strcat (dspString [10], err_str);
		sprintf (err_str, "^E%4.0f/%4.0f", wh [wh_no].wh_min_stk, 
						   wh [wh_no].wh_max_stk);
		strcat (dspString [11], err_str);
		strcat (dspString [12], "JGGGGGGGGG");
	}
	strcat (dspString [0], "|");
	for (loop = 1; loop < 13; loop++)
	{
		if (loop == 8 || loop == 12)
		{
			if (loop == 12)
				strcat (dspString [loop], "D");
			else
				strcat (dspString [loop], "E");
		}
		else
			strcat (dspString [loop], "^E");
	}

	while (strlen (dspString [0]) < 130)
		strcat (dspString [0], " ");
	Dsp_saverec (dspString [0]);

	Dsp_saverec ("");

	for (loop = 1; loop < 15; loop++)
		Dsp_saverec (dspString [loop]);

	Dsp_srch ();
	PrintCoStuff ();
}

void
GetPrices (
 void)
{
	int		i;

	for (i = 0; i < 9; i++)
		itemPrice [i] = 0.00;

	for (i = 0; i < numPrices; i++)
	{
		inpr_rec.hhgu_hash  = 0L;
		inpr_rec.hhbr_hash  = inmr_rec.hhbr_hash;
		inpr_rec.price_type = (i + 1);
		strcpy (inpr_rec.br_no, "  ");
		strcpy (inpr_rec.wh_no, "  ");
		sprintf (inpr_rec.curr_code, "%-3.3s", envVarCurrCode);
		sprintf (inpr_rec.area_code, "%-2.2s", "  ");
		sprintf (inpr_rec.cust_type, "%-3.3s", "   ");
		cc = find_rec (inpr, &inpr_rec, COMPARISON, "r");
		if (!cc)
			itemPrice [i] = inpr_rec.base;
	}
}

void
PrintCoStuff (
 void)
{
	print_at (23,0,  ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_short);
	print_at (23,35, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (23,70, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_name);
}

int
MtdSelect (
 void)
{
	MTD = TRUE;
	AllDisplay ();
    return (EXIT_SUCCESS);
}

int
YtdSelect (
 void)
{
	MTD = FALSE;
	AllDisplay ();
    return (EXIT_SUCCESS);
}

