/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_sup_disc.c  )                                 |
|  Program Desc  : ( Supplier Discount Maintenance.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, ingp, suds, sumr,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  suds,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : (11/10/93)       |
|---------------------------------------------------------------------|
|  Date Modified : (02/11/95)      | Modified  by :  Scott B Darrow.  |
|  Date Modified : (25/04/96)      | Modified by : Scott B Darrow.    |
|  Date Modified : (11/09/97)      | Modified by : Roanna Marcelino   |
|                                                                     |
|  Comments      :                                                    |
|    (02/11/95)  : FRA - Updated to fix input of buying groups as     |
|                :       Program also allowed selling groups to be    |
|                :       input.                                       |
|  (25/04/96)    : FRA - Updated for reverse discount.                |
|  (11/09/97)    : Modified for Multilingual Conversion.              |
|                                                                     |
| $Log: sup_disc.c,v $
| Revision 5.7  2002/07/25 11:17:37  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.6  2002/07/24 08:39:18  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/18 07:15:56  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/06/20 07:11:16  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/01/30 10:40:57  robert
| SC 00733 - Updated to fix data update on quantity breaks
|
| Revision 5.2  2001/08/09 09:20:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:46:02  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:17  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:26  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:33  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:12:06  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/02/18 02:40:46  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.11  1999/11/11 06:00:09  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.10  1999/11/03 07:32:38  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.9  1999/10/13 02:42:18  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/08 05:32:58  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:51  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sup_disc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sup_disc/sup_disc.c,v 5.7 2002/07/25 11:17:37 scott Exp $";

#define 	MAXSCNS			2
#define 	MAXLINES		6
#define 	TABLINES		6
#include 	<pslscr.h>
#include 	<minimenu.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define		SEL_UPDATE			0
#define		SEL_IGNORE			1
#define		SEL_DELETE			2
#define		DEFAULT			99

#define		SLEEP_TIME		3
#define		ScreenWidth		80


	/*============================
	| Special fields and flags   |
	============================*/
	int		envDbCo = 0;
	int		cr_find = 0;
	int		ins_flag = 0;
	int		new_entry = TRUE;

	char	branchNumber[3];

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 8;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		char	test_short[16];
		long	t_dbt_date;
	} comm_rec;

	/*=====================================
	| Inventory Buying and Selling Groups |
	=====================================*/
	struct dbview ingp_list [] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	int	ingp_no_fields = 5;

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	} ingp_rec;

	/*==============================
	| Supplier Discount Price File |
	==============================*/
	struct dbview suds_list [] =
	{
		{"suds_hhsu_hash"},
		{"suds_buy_group"},
		{"suds_reg_pc"},
		{"suds_qty_brk1"},
		{"suds_qty_brk2"},
		{"suds_qty_brk3"},
		{"suds_qty_brk4"},
		{"suds_qty_brk5"},
		{"suds_qty_brk6"},
		{"suds_disca_pc1"},
		{"suds_disca_pc2"},
		{"suds_disca_pc3"},
		{"suds_disca_pc4"},
		{"suds_disca_pc5"},
		{"suds_disca_pc6"},
		{"suds_discb_pc1"},
		{"suds_discb_pc2"},
		{"suds_discb_pc3"},
		{"suds_discb_pc4"},
		{"suds_discb_pc5"},
		{"suds_discb_pc6"},
		{"suds_discc_pc1"},
		{"suds_discc_pc2"},
		{"suds_discc_pc3"},
		{"suds_discc_pc4"},
		{"suds_discc_pc5"},
		{"suds_discc_pc6"},
		{"suds_cumulative"},
		{"suds_anticipated"}
	};

	int	suds_no_fields = 29;

	struct tag_sudsRecord
	{
		long	hhsu_hash;
		char	buy_group [7];
		float	reg_pc;
		float	qty_brk[6];
		float	disca_pc[6];
		float	discb_pc[6];
		float	discc_pc[6];
		char	cumulative [3];
		float	anticipated;
	} suds_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview sumr_list [] =
	{
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
		{"sumr_curr_code"},
	};

	int	sumr_no_fields = 7;

	struct tag_sumrRecord
	{
		char	sm_co_no [3];
		char	sm_est_no [3];
		char	sm_crd_no [7];
		long	sm_hhsu_hash;
		char	sm_name [41];
		char	sm_acronym [10];
		char	sm_curr_code [4];
	} sumr_rec;

struct {
	char 	dummy[11];
	char	crd_no[7];
	char	last_crd_no[7];
	char	crd_name[41];
	char	buy_group[7];
	char	last_buy_group[7];
	char	buy_desc[41];
	float	reg_pc;
	char	cum_disc[2];
	float	qty_brk;
	float	disc_a;
	float	disc_b;
	float	disc_c;
} local_rec;

struct storeRec {
	float	_qty_brk;
	float	_disc_a;
	float	_disc_b;
	float	_disc_c;
} store[MAXLINES];

	int		ReverseDiscount =	FALSE;

	char	*data	= "data";
	char	*comm	= "comm";
	char	*ingp	= "ingp";
	char	*suds	= "suds";
	char	*sumr	= "sumr";

static	struct	var	vars[] =
{
	{1, LIN, "crd_no",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", " Supplier Number : ", "Enter Supplier Number. Full search available. Default for last supplier.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no},
	{1, LIN, "crd_name",	 4, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.crd_name},
	{1, LIN, "buy_group",	 5, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ", " Buying Group    : ", "Enter Suppliers Buying Group. Default for last buying group.",
		 NE, NO,  JUSTLEFT, "", "", local_rec.buy_group},
	{1, LIN, "buy_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.buy_desc},
	{1, LIN, "reg_pc",	6, 18, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " Regulatory pc   : ", "Enter Regulatory Percentage",
		 NO, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&local_rec.reg_pc},
	{1, LIN, "cum_disc",	6, 55, CHARTYPE,
		"U", "          ",
		" ", "N", "Cumulative Discounts : ", "Y(es) Apply discounts cumulatively, N(o) Apply discounts as absolutes.",
		YES, NO,  JUSTLEFT, "NY", "", local_rec.cum_disc},

	{2, TAB, "qty_brk",	MAXLINES, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", " QTY BREAK ", " ",
		 NO, NO,  JUSTRIGHT, "0.00", "99999.99", (char *)&local_rec.qty_brk},
	{2, TAB, "disc_a",	MAXLINES, 4, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " DISCOUNT A ", "",
		 NO, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&local_rec.disc_a},
	{2, TAB, "disc_b",	MAXLINES, 4, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " DISCOUNT B ", "",
		 NO, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&local_rec.disc_b},
	{2, TAB, "disc_c",	MAXLINES, 4, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", " DISCOUNT C ", "",
		 NO, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&local_rec.disc_c},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include 	<FindSumr.h>	
/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int  spec_valid (int field);
void load_suds (void);
void update (void);
void update_menu (void);
int  delete_line (void);
int  insert_line (void);
int  heading (int scn);
void ingp_search (char *key_val);
float ScreenDisc (float DiscountPercent);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;
	int		i;

	SETUP_SCR (vars);


	init_scr();			/*  sets terminal from termcap	*/
	set_tty();
	set_masks();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars(1);			/*  set default values		*/

	envDbCo = atoi(get_env("CR_CO"));
	cr_find = atoi(get_env("CR_FIND"));

	OpenDB();

	tab_row = 9;
	tab_col = 14;

	strcpy(branchNumber, (envDbCo) ? comm_rec.test_no : " 0");

	sptr = chk_env("SO_DISC_REV");
	ReverseDiscount = ( sptr == (char *)0 ) ? FALSE : atoi ( sptr );

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	strcpy(local_rec.crd_no, "000000");
	strcpy(local_rec.last_crd_no, "000000");
	strcpy(local_rec.buy_group, "000000");
	strcpy(local_rec.last_buy_group, "000000");

	while (prog_exit == 0) 
	{
		entry_exit 	= 0;
		edit_exit 	= 0;
		prog_exit 	= 0;
		restart 	= 0;
		search_ok 	= 1;
		prog_status	= ENTRY;
		init_vars(1);
		init_vars(2);
		lcount[2] 	= 0;
		
		// clear quantity break values
		for (i = 0; i < 6; i++)
		{
			store[i]._qty_brk 	= 0.00;
			store[i]._disc_a 	= 0.00;
			store[i]._disc_b 	= 0.00;
			store[i]._disc_c 	= 0.00;
		}

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart) 
			continue;

		heading(2);
		/*------------------------------
		| Enter screen 2 tabular input.|
		------------------------------*/
		if (new_entry == TRUE)
			entry(2);
		else
			edit(2);

		if (prog_exit || restart) 
			continue;

		edit_all();
		if (restart) 
			continue;

		/*------------------------------
		| Update selection status.     |
		------------------------------*/
		update();

	}	
	shutdown_prog();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen(data);
	read_comm(comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec(ingp, ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec(suds, suds_list, suds_no_fields, "suds_id_no");
	open_rec(sumr, sumr_list, sumr_no_fields, (!cr_find) ? "sumr_id_no"
		  					       : "sumr_id_no3");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose(ingp);
	abc_fclose(sumr);
	abc_fclose(suds);
	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	int		cc = 0;

	/*--------------------------------------------
	| Validate Supplier Number And Allow Search. |
	--------------------------------------------*/
	if (LCHECK("crd_no"))
	{
		if (dflt_used && strcmp(local_rec.last_crd_no, "000000") != 0)
		{
			strcpy(local_rec.crd_no, local_rec.last_crd_no);
			strcpy(local_rec.crd_name, sumr_rec.sm_name);
			DSP_FLD( "crd_no" );
			DSP_FLD( "crd_name" );
			return(0);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, branchNumber, temp_str);
  			return(0);
		}
		
		strcpy(sumr_rec.sm_co_no,comm_rec.tco_no);
		strcpy(sumr_rec.sm_est_no, branchNumber);
		strcpy(sumr_rec.sm_crd_no,pad_num(local_rec.crd_no));
		cc = find_rec(sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess(ML(mlStdMess022));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		strcpy(local_rec.crd_name, sumr_rec.sm_name);
		DSP_FLD( "crd_name" );

		return(0);
	}

	/*-----------------------------------------
	| Validate Buying Group And Allow Search. |
	-----------------------------------------*/
	if (LCHECK("buy_group"))
	{
		if (dflt_used && strcmp(local_rec.last_buy_group, "000000") != 0)
		{
			strcpy(local_rec.buy_group, local_rec.last_buy_group);
			strcpy(local_rec.buy_desc, ingp_rec.desc);
			DSP_FLD( "buy_group" );
			DSP_FLD( "buy_desc" );
			load_suds();
			return(0);
		}

		if (SRCH_KEY)
		{
	 		ingp_search(temp_str);
  			return(0);
		}
		
		strcpy(ingp_rec.co_no,comm_rec.tco_no);
		strcpy(ingp_rec.type, "B");
		strcpy(ingp_rec.code,local_rec.buy_group);
		cc = find_rec(ingp, &ingp_rec, EQUAL, "r");
		if (cc)
		{
			/*Buying group %s is not on file.*/
			print_mess(ML(mlStdMess207));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		strcpy(local_rec.buy_desc, ingp_rec.desc);
		DSP_FLD( "buy_desc" );

		/*------------------------------------
		| Read Supplier Quantity Breaks.    |
		------------------------------------*/
		load_suds();
		return(0);
	}
	/*--------------------------
	| Validate quantity break. |
	--------------------------*/
	if (LCHECK("qty_brk"))
	{
		if (last_char == INSLINE)
			return(insert_line());

		if (dflt_used || last_char == DELLINE)
			return(delete_line());

		if(local_rec.qty_brk == 0.00)
		{
			if (prog_status == ENTRY)
				return(delete_line());

			while(line_cnt < lcount[2])
				cc = delete_line();
			return(cc);
		}

		if ((line_cnt > 0) &&
			(local_rec.qty_brk <= store[line_cnt - 1]._qty_brk))
		{
			/*Quantity break must be greater than previous break.*/
			print_mess(ML(mlSkMess104));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		if ((line_cnt < 5) && (store[line_cnt + 1]._qty_brk > 0.00) &&
			(local_rec.qty_brk >= store[line_cnt + 1]._qty_brk))
		{
			/*Quantity break must be less than succeeding break.*/
			print_mess(ML(mlSkMess105));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		store[line_cnt]._qty_brk = local_rec.qty_brk;
		return(0);
	}
	/*----------------------
	| Validate Discount A. |
	----------------------*/
	if (LCHECK("disc_a"))
	{
		store[line_cnt]._disc_a = local_rec.disc_a;
		return(0);
	}
	/*----------------------
	| Validate Discount B. |
	----------------------*/
	if (LCHECK("disc_b"))
	{
		store[line_cnt]._disc_b = local_rec.disc_b;
		return(0);
	}
	/*----------------------
	| Validate Discount C. |
	----------------------*/
	if (LCHECK("disc_c"))
	{
		store[line_cnt]._disc_c = local_rec.disc_c;
		return(0);
	}
	return(0);
}	

/*=======================================
| Load suds data for table 2 display 	|
=======================================*/
void
load_suds (
 void)
{
	int		i;

	suds_rec.hhsu_hash = sumr_rec.sm_hhsu_hash;
	strcpy(suds_rec.buy_group, ingp_rec.code);
	cc = find_rec(suds, &suds_rec, EQUAL, "u");
	if (!cc)
		new_entry = FALSE;
	else
		new_entry = TRUE;

	lcount[2] = 0;
	if (new_entry == FALSE)
	{
		entry_exit = 1;
		local_rec.reg_pc = ScreenDisc (suds_rec.reg_pc);
		strcpy(local_rec.cum_disc, suds_rec.cumulative);
		DSP_FLD("reg_pc");
		DSP_FLD("cum_disc");
		/*----------------------------
		| Set screen 2 - for putval. |
		----------------------------*/
		scn_set( 2 );
	
		init_vars( 2 );
		for (i = 0; i < 6; i++)
		{
			store[i]._qty_brk 	= suds_rec.qty_brk[i];
			store[i]._disc_a 	= ScreenDisc (suds_rec.disca_pc[i]);
			store[i]._disc_b 	= ScreenDisc (suds_rec.discb_pc[i]);
			store[i]._disc_c 	= ScreenDisc (suds_rec.discc_pc[i]);
			local_rec.qty_brk 	= suds_rec.qty_brk[i];
			local_rec.disc_a 	= ScreenDisc (suds_rec.disca_pc[i]);
			local_rec.disc_b 	= ScreenDisc (suds_rec.discb_pc[i]);
			local_rec.disc_c 	= ScreenDisc (suds_rec.discc_pc[i]);
			if (store[i]._qty_brk > 0.00)
				putval(lcount[2]++);
		}
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			store[lcount[2]]._qty_brk = 0.00;
			store[lcount[2]]._disc_a = 0.00;
			store[lcount[2]]._disc_b = 0.00;
			store[lcount[2]]._disc_c = 0.00;
		}
	}
	return;
}

void
update (
 void)
{
	int		i;

   	/*------------------------------------------------------
   	| Set to Tabular Screen(s) to Update Discount Details. |
   	------------------------------------------------------*/
   	scn_set(2);
	
	suds_rec.hhsu_hash = sumr_rec.sm_hhsu_hash;
	strcpy(suds_rec.buy_group, local_rec.buy_group);
	suds_rec.reg_pc = ScreenDisc (local_rec.reg_pc);
	strcpy(suds_rec.cumulative, local_rec.cum_disc);
	for (i = 0; i < 6; i++)
	{
		suds_rec.qty_brk[i] 	= store[i]._qty_brk;
		suds_rec.disca_pc[i] 	= ScreenDisc (store[i]._disc_a);
		suds_rec.discb_pc[i] 	= ScreenDisc (store[i]._disc_b);
		suds_rec.discc_pc[i] 	= ScreenDisc (store[i]._disc_c);
	}
	if (new_entry == TRUE)
	{
		cc = abc_add(suds, &suds_rec);
		if (cc)
			file_err( cc, suds, "DBADD" );
	}
	else
		update_menu();
	
	strcpy(local_rec.last_crd_no, local_rec.crd_no);
	strcpy(local_rec.last_buy_group, local_rec.buy_group);
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE.       ",
		  "Update Supplier Discount Record With Changes Made." },
		{ " 2. IGNORE.       ",
		  "Ignore Changes Just Made To Supplier Discount Record." },
		{ " 3. DELETE.       ",
		  "Delete Supplier Discount Record." },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
update_menu (
 void)
{
	for (;;)
	{
	    mmenu_print (" Update Selection. ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case DEFAULT :
		case SEL_UPDATE :
			cc = abc_update(suds, &suds_rec);
			if (cc)
				file_err( cc, suds, "DBUPDATE" );
			return;

		case SEL_IGNORE :
			abc_unlock (suds);
			return;

		case SEL_DELETE :
			abc_unlock (suds);
			cc = abc_delete (suds);
			if (cc)
				file_err( cc, suds, "DBDELETE" );
			return;
		default :
			break;
	    }
	}
}

/*==============
| Delete line. |
==============*/
int
delete_line (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY && ins_flag == 0)
	{
		blank_display();
		store[line_cnt]._qty_brk = 0.00;
		store[line_cnt]._disc_a = 0.00;
		store[line_cnt]._disc_b = 0.00;
		store[line_cnt]._disc_c = 0.00;
		return(1);
	}

	if (lcount[2] == 0)
	{
		/*Cannot Delete Line - No Lines to Delete */
		print_mess(ML(mlStdMess032));
		sleep(2);
		clear_mess();
		return(1);
	}

	lcount[2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount[2]; line_cnt++)
	{
		store[line_cnt]._qty_brk = store[line_cnt + 1]._qty_brk;
		store[line_cnt]._disc_a = store[line_cnt + 1]._disc_a;
		store[line_cnt]._disc_b = store[line_cnt + 1]._disc_b;
		store[line_cnt]._disc_c = store[line_cnt + 1]._disc_c;

		getval(line_cnt + 1);
		putval(line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display();
	}

	while (line_cnt <= lcount[2])
	{
		store[line_cnt]._qty_brk = 0.00;
		store[line_cnt]._disc_a = 0.00;
		store[line_cnt]._disc_b = 0.00;
		store[line_cnt]._disc_c = 0.00;

		if (this_page == line_cnt / TABLINES)
			blank_display();

		line_cnt++;
	}

	line_cnt = i;
	getval(line_cnt);
	if (ins_flag == 1)
		entry_exit = 1;
	return(0);
}

int
insert_line (
 void)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*Cannot Insert Lines On Entry*/
		print_mess(ML(mlStdMess005));
		sleep(2);
		return(1);
	}

	if (lcount[2] >= vars[label("qty_brk")].row)
	{
		/*Cannot Insert Line - Table is Full*/
		print_mess(ML(mlStdMess076));
		sleep(2);
		return(1);
	}

	for (i = line_cnt,line_cnt = lcount[2];line_cnt > i;line_cnt--)
	{
		store[line_cnt]._qty_brk = store[line_cnt - 1]._qty_brk;
		store[line_cnt]._disc_a = store[line_cnt - 1]._disc_a;
		store[line_cnt]._disc_b = store[line_cnt - 1]._disc_b;
		store[line_cnt]._disc_c = store[line_cnt - 1]._disc_c;

		getval(line_cnt - 1);
		putval(line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display();
	}
	lcount[2]++;
	line_cnt = i;

	store[i]._qty_brk 	= suds_rec.qty_brk[i];
	store[i]._disc_a 	= ScreenDisc (suds_rec.disca_pc[i]);
	store[i]._disc_b 	= ScreenDisc (suds_rec.discb_pc[i]);
	store[i]._disc_c 	= ScreenDisc (suds_rec.discc_pc[i]);
	local_rec.qty_brk 	= suds_rec.qty_brk[i];
	local_rec.disc_a 	= ScreenDisc (suds_rec.disca_pc[i]);
	local_rec.disc_b 	= ScreenDisc (suds_rec.discb_pc[i]);
	local_rec.disc_c 	= ScreenDisc (suds_rec.discc_pc[i]);
	putval(line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display();

	ins_flag = 1;
	init_ok = 0;
	prog_status = ENTRY;
	scn_entry(cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	ins_flag = 0;
	line_cnt = i;
	getval(line_cnt);
	return(0);
}

/*-----------------
| Screen Heading. |
-----------------*/		
int
heading (
 int scn)
{
	int		i;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();

		fflush(stdout);
		rv_pr(ML(mlSkMess049), (ScreenWidth -32)/2, 0, 1);

		print_at(0,ScreenWidth - 22,ML(mlSkMess051), local_rec.last_crd_no);

		strcpy(err_str,ML(mlSkMess050));
		i = strlen(err_str);
		rv_pr(err_str, (ScreenWidth - i)/2, 2, 1);

		fflush(stdout);
		move(0, 1);
		line(ScreenWidth - 1);

		move(1, input_row);
		switch (scn)
		{
		case  1 :
			if (prog_status != ENTRY)
			{
				scn_set(2);
				scn_write(2);
				scn_display(2);
#ifndef GVISION
				box(tab_col, tab_row - 1, 52, TABLINES + 2);
#endif
			}
			box(0, 3, ScreenWidth - 1, 3);
			scn_set(1);
			scn_write(1);
			scn_display(1);
			break;
		
		case  2 :
			box(0, 3, ScreenWidth - 1, 3);
			scn_set(1);
			scn_write(1);
			scn_display(1);
			scn_set(2);
#ifndef GVISION
			box(tab_col, tab_row - 1, 52, TABLINES + 2);
#endif
			scn_write(2);
			scn_display(2);
			break;

		}
		move(1, 20);
		line(ScreenWidth - 1);
		print_at(21,0,ML(mlStdMess038), 
				comm_rec.tco_no, comm_rec.tco_short);
		print_at(21,45,ML(mlStdMess039), 
				comm_rec.test_no, comm_rec.test_short);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		fflush(stdout);
	}
    return (EXIT_SUCCESS);
}

void
ingp_search (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy(ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
			ingp_rec.type[0] == 'B' &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc (
 float	DiscountPercent)
{
	if ( ReverseDiscount )
		return (DiscountPercent * -1);

	return (DiscountPercent);
}

