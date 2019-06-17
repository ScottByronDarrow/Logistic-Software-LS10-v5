/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_cqtyupd.c  )                                  |
|  Program Desc  : ( Customer Qty Break Update                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cuqb, inpr,     ,    ,     ,     ,          |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cuqb, inpr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 18/10/93         |
|---------------------------------------------------------------------|
|  Date Modified : (04/09/97)      | Modified  by : Ana Marie C Tario.|
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
| $Log: sk_cqtyupd.c,v $
| Revision 5.2  2001/08/09 09:18:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:19  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:49  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/11/25 10:24:16  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.12  1999/11/11 05:59:32  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.11  1999/11/03 07:31:54  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/13 02:41:54  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.9  1999/10/12 21:20:31  scott
| Updated by Gerry from ansi project.
|
| Revision 1.8  1999/10/08 05:32:17  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:19:52  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
char	*PNAME = "$RCSfile: sk_cqtyupd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cqtyupd/sk_cqtyupd.c,v 5.2 2001/08/09 09:18:18 scott Exp $";

#define	CCMAIN
#define	MOD		5
#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <hot_keys.h>

#include <tabdisp.h>

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_price1_desc"},
		{"comm_price2_desc"},
		{"comm_price3_desc"},
		{"comm_price4_desc"},
		{"comm_price5_desc"},
		{"comm_price6_desc"},
		{"comm_price7_desc"},
		{"comm_price8_desc"},
		{"comm_price9_desc"},
	};

	int	comm_no_fields = 13;

	struct	{
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	price_desc[9][16];
	} comm_rec;

	/*======================================
	| Customer Quantity Break Default file |
	======================================*/
	struct dbview cuqb_list [] =
	{
		{"cuqb_co_no"},
		{"cuqb_price_type"},
		{"cuqb_buygrp"},
		{"cuqb_sellgrp"},
		{"cuqb_category"},
		{"cuqb_qty_brk1"},
		{"cuqb_qty_brk2"},
		{"cuqb_qty_brk3"},
		{"cuqb_qty_brk4"},
		{"cuqb_qty_brk5"},
		{"cuqb_qty_brk6"},
		{"cuqb_qty_brk7"},
		{"cuqb_qty_brk8"},
		{"cuqb_qty_brk9"},
		{"cuqb_update_flag"}
	};

	int	cuqb_no_fields = 15;

	struct tag_cuqbRecord
	{
		char	co_no [3];
		int		price_type;
		char	buygrp [7];
		char	sellgrp [7];
		char	category [12];
		float	qty_brk[9];
		char	upd[3];
	} cuqb_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_co_no"},
		{"inmr_hhbr_hash"},
		{"inmr_category"},
		{"inmr_sellgrp"},
		{"inmr_buygrp"},
	};

	int	inmr_no_fields = 5;

	struct tag_inmrRecord
	{
		char	co_no [3];
		long	hhbr_hash;
		char	category [12];
		char	sellgrp [7];
		char	buygrp [7];
	} inmr_rec;

	/*======================
	| Inventory Price File |
	======================*/
	struct dbview inpr_list [] =
	{
		{"inpr_hhbr_hash"},
		{"inpr_price_type"},
		{"inpr_br_no"},
		{"inpr_wh_no"},
		{"inpr_curr_code"},
		{"inpr_price_by"},
		{"inpr_qty_brk1"},
		{"inpr_qty_brk2"},
		{"inpr_qty_brk3"},
		{"inpr_qty_brk4"},
		{"inpr_qty_brk5"},
		{"inpr_qty_brk6"},
		{"inpr_qty_brk7"},
		{"inpr_qty_brk8"},
		{"inpr_qty_brk9"},
	};

	int	inpr_no_fields = 15;

	struct tag_inprRecord
	{
		long	hhbr_hash;
		int		price_type;
		char	est_no [3];
		char	wh_no [3];
		char	curr_code [4];
		char	price_by[2];
		double	qty_brk[9];
	} inpr_rec;

	char	*comm   = "comm",
			*data   = "data",
			*inpr   = "inpr",
			*inmr   = "inmr",
			*cuqb	= "cuqb";

		int	numInTab;
		int	numQtyBrks;
		int	numOfPrices;

struct
{
	char	range[2];
	char	rangedesc[14];
	int		price_type;
	char	price_desc[16];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "range",	4, 24, CHARTYPE,
		"U", "          ",
		" ", "S",  "Range Type          :", "Enter Range To Use For Updating, B)uying S)elling C)ategory",
		YES, NO, JUSTLEFT, "BSC", "", local_rec.range},
	{1, LIN, "rangedesc",	4, 34, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangedesc},
	{1, LIN, "price",	5, 24, INTTYPE,
		"N", "          ",
		"", "",  "Price Type          :", "Enter Price Type To Update. ",
		YES, NO, JUSTLEFT, "1", "9", (char *) &local_rec.price_type},
	{1, LIN, "pricedesc",5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.price_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

static	int	toggle_func(int c, KEY_TAB *psUnused);
static	int	delete_func(int c, KEY_TAB *psUnused);
static	int	exit_func(int c, KEY_TAB *psUnused);

static	KEY_TAB list_keys [] =
{
   { "[T]OGGLE LINE",		'T',		toggle_func,
	"Toggle Line For Update.",					"A" },
   { "[^A]ccept ALL",		CTRL('A'), 	toggle_func,
	"Toggle All Lines.",				"A" },
   { "[D]ELETE",		'D', 		delete_func,
	"Delete Record.", 						"D" },
   { NULL,			FN1, 		exit_func,
	"Exit Without Update.",						"A" },
   { NULL,			FN16, 		exit_func,
	"Exit And Update The Database.",				"A" },
   END_KEYS
};


/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
int  heading (int scn);
void process (void);
void shutdown_prog (void);
void load_selling (void);
void load_buying (void);
void load_cat (void);
void toggle_line (int line_no, int multi);
void Head (int width);
void update (void);
int  readCuqb (int fstTime);
void CatBrks (void);
void CatAmts (char (*array)[11]);
int  UpdInpr (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv[])
{
	char	*sptr = chk_env ("SK_DBQTYNUM");
	if (sptr)
		numQtyBrks = atoi (sptr);
	else
		numQtyBrks = 0;

	if (numQtyBrks < 0 || numQtyBrks > 9)
		numQtyBrks = 9;
		
	sptr = chk_env ("SK_DBPRINUM");
	if (sptr)
	{
		numOfPrices = atoi (sptr);
		if (numOfPrices > 9 || numOfPrices < 1)
			numOfPrices = 9;
	}
	else
		numOfPrices = 5;

	SETUP_SCR (vars);
	init_scr ();
	set_tty (); 

	if (!numQtyBrks)
	{
		clear ();
		strcpy(err_str,ML(mlSkMess478));
		rv_pr (err_str, 10, (80 - strlen (clip (err_str))) /2, 1);
		PauseForKey (0, 0, "", 0);
		return (EXIT_FAILURE);
	}

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();

	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
		init_vars (1);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		swide ();
		clear ();
		Head (132);
		process ();
		snorm ();
		clear ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (cuqb, cuqb_list, cuqb_no_fields, "cuqb_id_sellgrp");
	open_rec (inpr, inpr_list, inpr_no_fields, "inpr_id_no2");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_sell");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cuqb);	
	abc_fclose (inpr);	
	abc_fclose (inmr);	
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{

	if (LCHECK ("price"))
	{
		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.price_type > numOfPrices)
		{
			sprintf (err_str, ML(mlSkMess218),numOfPrices);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.price_desc, 
				comm_rec.price_desc [local_rec.price_type - 1]);
		DSP_FLD ("pricedesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("range"))
	{
		/*-------------------------
		| if choice is buying group
		--------------------------*/
		if (local_rec.range[0] == 'B')
		{
			strcpy (local_rec.rangedesc, "Buying Group");
			abc_selfield (inmr, "inmr_id_buy");
		}

		/*-------------------------
		| if choice is selling group
		--------------------------*/
		if (local_rec.range[0] == 'S')
		{
			strcpy (local_rec.rangedesc, "Selling Group");
			abc_selfield (inmr, "inmr_id_sell");
		}

		/*-------------------------
		| if choice is category
		--------------------------*/
		if (local_rec.range[0] == 'C')
		{
			strcpy (local_rec.rangedesc, "Category");
			abc_selfield (inmr, "inmr_id_cat");
		}

		DSP_FLD ("rangedesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	box (0, 3, 80, 2);
	Head (80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
process (
 void)
{
	/*------------
	| Open table |
	------------*/
	int	width;

	width = (132 - (20 + (numQtyBrks * 10))) / 2;
	tab_open("upd_lst", list_keys, 3, width, 14, FALSE);

	/*-------------------
	| if user chooses S
	| then load all selling
	| cuqb records
	----------------------*/
	if (local_rec.range[0] == 'S')
		load_selling ();

	/*-------------------
	| if user chooses B
	| then load all buying
	| cuqb records
	----------------------*/
	if (local_rec.range[0] == 'B')
		load_buying ();

	/*-------------------
	| if user chooses C
	| then load all category
	| cuqb records
	----------------------*/
	if (local_rec.range[0] == 'C')
		load_cat ();

	if (!numInTab)
	{
		tab_add ("upd_lst", "No Valid Records To Load ");
		tab_display ("upd_lst", TRUE);
		sleep (sleepTime);
		tab_close ("upd_lst", TRUE);
		return;
	}

	tab_scan ("upd_lst");

	if (!restart)
		update ();
	
	tab_close ("upd_lst", TRUE);
	return;
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
load_selling (
 void)
{
	int	fstTime = TRUE;

	sprintf (err_str, "# %-11.11s ", "SELLING");
	CatBrks ();
	tab_add("upd_lst", err_str);

	/*-----------------------
	| reselect cuqb index
	------------------------*/
	abc_selfield (cuqb, "cuqb_id_sellgrp");

	while (readCuqb (fstTime))    /*   called by all load options */
	{
		char	qtyStr[9][11];
		int		count;

		fstTime = FALSE;
		/*---------------------
		| check to make sure 
		| still reading selling groups
		-----------------------------*/
		if (!strcmp (cuqb_rec.sellgrp, "      ") || cuqb_rec.upd[0] == 'Y')
			continue;

		for (count = 0; count < 9; count++)
		{
			if (cuqb_rec.qty_brk[count] != 0.00)
				sprintf (qtyStr[count], "%10.2f", cuqb_rec.qty_brk[count]);
			else
				sprintf (qtyStr[count], "%10.10s", " ");
		}

		sprintf (err_str, " %-11.11s", cuqb_rec.sellgrp);
		CatAmts (qtyStr);
		tab_add("upd_lst", err_str);
		numInTab++;
	}
}

void
load_buying (
 void)
{
	int	fstTime = TRUE;

	sprintf (err_str, "# %-11.11s ", "BUYING");
	CatBrks ();
	tab_add("upd_lst", err_str);

	/*-----------------------
	| reselect cuqb index
	------------------------*/
	abc_selfield (cuqb, "cuqb_id_buygrp");

	while (readCuqb (fstTime))    /*   called by all load options */
	{
		char	qtyStr[9][11];
		int		count;

		fstTime = FALSE;

		/*---------------------
		| check to make sure 
		| still reading buying groups
		-----------------------------*/
		if (!strcmp (cuqb_rec.buygrp, "      ") || cuqb_rec.upd[0] == 'Y')
			continue;

		for (count = 0; count < 9; count++)
		{
			if (cuqb_rec.qty_brk[count] != 0.00)
				sprintf (qtyStr[count], "%10.2f", cuqb_rec.qty_brk[count]);
			else
				sprintf (qtyStr[count], "%10.10s", " ");
		}

		sprintf (err_str, " %-11.11s", cuqb_rec.buygrp);
		CatAmts (qtyStr);
		tab_add("upd_lst", err_str);
		numInTab++;
	}
}

void
load_cat (
 void)
{
	int	fstTime = TRUE;

	sprintf (err_str, "# %-11.11s ", "CATEGORY");
	CatBrks ();
	tab_add("upd_lst", err_str);

	/*-----------------------
	| reselect cuqb index
	------------------------*/
	abc_selfield (cuqb, "cuqb_id_cat");

	while (readCuqb (fstTime))    /*   called by all load options */
	{
		char	qtyStr[9][11];
		int		count;

		fstTime = FALSE;

		/*---------------------
		| check to make sure 
		| still reading cats
		-----------------------------*/
		if (!strcmp (cuqb_rec.category, "           ")||cuqb_rec.upd[0] == 'Y')
			continue;

		for (count = 0; count < 9; count++)
		{
			if (cuqb_rec.qty_brk[count] != 0.00)
				sprintf (qtyStr[count], "%10.2f", cuqb_rec.qty_brk[count]);
			else
				sprintf (qtyStr[count], "%10.10s", " ");
		}

		sprintf (err_str, " %-11.11s", cuqb_rec.category);
		CatAmts (qtyStr);
		tab_add("upd_lst", err_str);
		numInTab++;
	}
}


static int 
toggle_func (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf[200];

	st_line = tab_tline("upd_lst");

	if (c == 'T')
	{
		toggle_line(st_line, FALSE);
	}
	else
	{
		for (i = st_line; i < numInTab; i++)
			toggle_line(i, TRUE);

		tab_display("upd_lst", TRUE);
		cc = tab_get("upd_lst", get_buf, EQUAL, st_line);
		if (cc)
			sys_err ("Error in Retrieving Record", cc, PNAME);
		redraw_keys("upd_lst");
	}

	return(c);
}

void
toggle_line (
 int line_no, 
 int multi)
{
	char	get_buf[200];
	char	curr_stat[2];
	char	new_stat[2];

	cc = tab_get("upd_lst", get_buf, EQUAL, line_no);
	if (cc)
		sys_err ("Error in Retrieving Record", cc, PNAME);

	sprintf(curr_stat, "%-1.1s", get_buf + (13 + (numQtyBrks * 10)));

	if (curr_stat[0] == '*')
	{
		new_stat[0] = ' ';
	}
	else
	{
		if (curr_stat[0] == 'D')   /* tagged for deletion */
		{
			if (!multi)
			{
				/*print_mess ("Can Not Tag A Line That Is Included For Deletion - UnTag First \007");*/
				print_mess(ML(mlSkMess541));
				sleep (sleepTime);
				clear_mess ();
			}
			return;
		}
		else
		{
			new_stat[0] = '*';
		}
	}

	tab_update("upd_lst",
		"%*.*s%-1.1s%5.5s",
		(13 + (numQtyBrks * 10)),
		(13 + (numQtyBrks * 10)),
		get_buf,
		new_stat, " ");

	return;
}

static int 
delete_func (
 int c, 
 KEY_TAB *psUnused)
{
	char	get_buf[200];
	char	curr_stat[2];
	char	new_stat[7];

	cc = tab_get("upd_lst", get_buf, CURRENT, 0);
	if (cc)
		sys_err ("Error in Retrieving Record", cc, PNAME);

	sprintf(curr_stat, "%-1.1s", get_buf + (13 + (numQtyBrks * 10)));

	if (curr_stat[0] == '*')
	{
/*
		print_mess ("Can Not Delete A Line That Is Included For Update - UnTag First \007");*/
		print_mess(ML(mlSkMess476));
		sleep (sleepTime);
		clear_mess ();
		return (c);
	}
	else
	{
		if (curr_stat[0] == 'D')     /* already taggged for deletion */
			strcpy (new_stat, "      ");
		else
			strcpy (new_stat, "DELETE");
	}
		
	tab_update("upd_lst",
		"%*.*s%6.6s",
		(13 + (numQtyBrks * 10)),
		(13 + (numQtyBrks * 10)),
		get_buf,
		new_stat);

	return(c);
}

static int 
exit_func (
 int c, 
 KEY_TAB *psUnused)
{
	if (c == FN1)
		restart = TRUE;

	return(c);
}

void
Head (
 int width)
{
	strcpy(err_str,ML(mlSkMess477));
	rv_pr (err_str, (width - (strlen (clip (err_str)))) / 2, 0, 1);
	move (0, 21);
	line (width);
	strcpy(err_str,ML(mlStdMess038));
	print_at (22, 1, err_str, comm_rec.tco_no, comm_rec.tco_name);
	move (0,1);
	line (width);
}

void
update (
 void)
{
	int 	count;
	char	get_buf [200];
	char	status[2];

	for (count = 0; count < numInTab; count++)
	{
		cc = tab_get ("upd_lst", get_buf, EQUAL, count);
		if (cc)
			sys_err ("Error in Retrieving Record", cc, PNAME);

		sprintf (status, "%-1.1s", get_buf + (13 + (numQtyBrks * 10)));

		memset (&cuqb_rec, 0, sizeof (cuqb_rec));
		strcpy (cuqb_rec.co_no, comm_rec.tco_no);
		cuqb_rec.price_type = local_rec.price_type;

		if (local_rec.range[0] == 'S')
		{
			sprintf (cuqb_rec.sellgrp, "%6.6s", get_buf + 1);
		}

		if (local_rec.range[0] == 'B')
		{
			sprintf (cuqb_rec.buygrp, "%6.6s", get_buf + 1);
		}

		if (local_rec.range[0] == 'C')
		{
			sprintf (cuqb_rec.category, "%11.11s", get_buf + 1);
		}

		/*----------------------------
		| if not tagged for anything
		| then go no further - back to
		| the top of the for statement
		------------------------------*/
		if (status[0] == ' ')
			continue;

		cc = find_rec (cuqb, &cuqb_rec, EQUAL, "u");
		if (cc)
			file_err (cc, cuqb, "DBFIND");

		/*-------------------
		| if DELETE do not
		| need to look up inmr
		| simply delete cuqb
		---------------------*/
		if (status[0] == 'D')
		{
			cc = abc_delete (cuqb);
			if (cc)
				file_err (cc, cuqb, "DBDELETE");
			continue;
		}

		/*-----------------------
		| now depending on the 
		| range find all inmr for
		| that range and then
		| update the releveant inpr
		| If inpr records updated
		| then the cuqb will be flagged
		-----------------------*/
		if (UpdInpr ())
		{
			strcpy (cuqb_rec.upd, "Y");
			cc = abc_update (cuqb, &cuqb_rec);
			if (cc)
				file_err (cc, cuqb, "DBUPDATE");
		}
		else
			abc_unlock (cuqb);
	}
}

int
readCuqb (
 int fstTime)
{
	if (fstTime)
	{
		numInTab = 0;
		memset (&cuqb_rec, 0, sizeof (cuqb_rec));
		strcpy (cuqb_rec.co_no, comm_rec.tco_no);
		cuqb_rec.price_type = local_rec.price_type;
	}

	cc = find_rec (cuqb, &cuqb_rec, (fstTime) ? GTEQ : NEXT, "r");
	if (!cc && 
		!strcmp (cuqb_rec.co_no, comm_rec.tco_no) &&
		cuqb_rec.price_type == local_rec.price_type)
	{
		return (TRUE);
	}
	else
		return (FALSE);
}

void
CatBrks (
 void)
{
	int		count;
	char 	lcl_str[10];

	for (count = 0; count < numQtyBrks; count++)
	{
		sprintf (lcl_str, " QTY BRK%d ", count + 1);
		strncat (err_str, lcl_str, 10);
	}

	strncat (err_str, "TAGGED", 6);
}

void
CatAmts (
 char (*array)[11])
{
		int		count;

		for (count = 0; count < numQtyBrks; count++)
			strncat (err_str, array[count], 11);
		
		strncat (err_str, "      ", 6);
}

int
UpdInpr (
 void)
{
	int	updated = FALSE;

	strcpy (inmr_rec.co_no, comm_rec.tco_no);

	/*-------------------
	| rest of index to be
	| initilaised 
	---------------*/
	if (local_rec.range[0] == 'S')
		strcpy (inmr_rec.sellgrp, cuqb_rec.sellgrp);

	if (local_rec.range[0] == 'B')
		strcpy (inmr_rec.buygrp, cuqb_rec.buygrp);

	if (local_rec.range[0] == 'C')
		strcpy (inmr_rec.category, cuqb_rec.category);

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc)
	{
		if (local_rec.range[0] == 'S')
			if (strcmp (inmr_rec.sellgrp, cuqb_rec.sellgrp))
				return(updated);

		if (local_rec.range[0] == 'B')
			if (strcmp (inmr_rec.buygrp, cuqb_rec.buygrp))
				return(updated);

		if (local_rec.range[0] == 'C')
			if (strcmp (inmr_rec.category, cuqb_rec.category))
				return(updated);

		/*----------------------------
		| now use the hhbr_hash to
		| look up the inpr records
		---------------------------*/
		inpr_rec.price_type = local_rec.price_type;
		inpr_rec.hhbr_hash = inmr_rec.hhbr_hash;

		cc = find_rec (inpr, &inpr_rec, GTEQ, "u");
		while (!cc && 
				inpr_rec.hhbr_hash == inmr_rec.hhbr_hash &&
				inpr_rec.price_type == local_rec.price_type)
		{
			int	count;

			if (inpr_rec.price_by[0] == 'V')
			{
				abc_unlock (inpr);
				cc = find_rec (inpr, &inpr_rec, NEXT, "u");
				continue;
			}

			updated = TRUE;

			for (count = 0; count < numQtyBrks; count++)
				inpr_rec.qty_brk[count] = (double) cuqb_rec.qty_brk[count];

			/*--------------------------------
			| set rest to zero - just in case
			| env variable changed
			---------------------------------*/
			for (; count < 9; count++)
				inpr_rec.qty_brk[count] = (double) 0.00;

			cc = abc_update (inpr, &inpr_rec);
			if (cc)
				file_err (cc, inpr, "DBUPDATE");

			cc = find_rec (inpr, &inpr_rec, NEXT, "u");
		}
		abc_unlock (inpr);
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	return (updated);
}
