/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_com_alloc.c )                                 |
|  Program Desc  : ( Sales Order Salesman Commission Allocation.  )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (11/06/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: com_alloc.c,v $
| Revision 5.2  2001/08/09 09:16:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/02/20 05:13:53  robert
| added entry_exit=TRUE under the dflt_used block
|
| Revision 3.1  2001/02/20 03:54:23  scott
| Updated to add dflt_used.
|
| Revision 3.0  2000/10/10 12:18:54  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:36  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:23  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/06/13 05:02:34  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.9  1999/12/06 01:35:23  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/16 04:55:32  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.7  1999/09/29 10:12:45  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:31  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 02:01:51  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/06/18 09:39:20  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: com_alloc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_com_alloc/com_alloc.c,v 5.2 2001/08/09 09:16:50 scott Exp $";

#define		MAXSCNS		10
#define		TABLINES	14
#define		MAXWIDTH	200
#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>

#define		SCN_MAIN			1
#define		SCN_SELL_GRP		2
#define		SCN_CATEGORY		3
#define		SCN_UD_CODE 		4
#define		SCN_ITEM_NO 		5

#define		BySellingGroup		1
#define		ByCategory			2
#define		ByUDCode			3
#define		ByItemNo			4

extern	int		X_EALL;
extern	int		Y_EALL;
extern	int		SR_X_POS;
extern	int		SR_Y_POS;

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int	comm_no_fields = 7;

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		char	tcc_no [3];
		char	tcc_name [41];
	} comm_rec;

	/*=====================================+
	 | Inventory Buying and Selling Groups |
	 +=====================================*/
#define	INGP_NO_FIELDS	5
	struct dbview	ingp_list [INGP_NO_FIELDS] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	}	ingp_rec;

	/*=========================+
	 | External Salesman File. |
	 +=========================*/
#define	EXSF_NO_FIELDS	26

	struct dbview	exsf_list [EXSF_NO_FIELDS] =
	{
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_hhsf_hash"},
		{"exsf_logname"},
		{"exsf_salesman"},
		{"exsf_sell_type"},
		{"exsf_sell_grp"},
		{"exsf_sell_pos"},
		{"exsf_area_code"},
		{"exsf_route_no"},
		{"exsf_carr_code"},
		{"exsf_up_sman1"},
		{"exsf_up_sman2"},
		{"exsf_up_sman3"},
		{"exsf_sale_stat"},
		{"exsf_com_status"},
		{"exsf_com_type"},
		{"exsf_com_pc"},
		{"exsf_com_min"},
		{"exsf_sman_com"},
		{"exsf_lev1_com"},
		{"exsf_lev2_com"},
		{"exsf_lev3_com"},
		{"exsf_lev4_com"},
		{"exsf_stat_flag"},
		{"exsf_update"}
	};

	struct tag_exsfRecord
	{
		char	co_no [3];
		char	sman_no [3];
		long	hhsf_hash;
		char	logname [15];
		char	sman_name [41];
		char	sell_type [3];
		char	sell_grp [3];
		char	sell_pos [3];
		char	area_code [3];
		char	route_no [9];
		char	carr_code [5];
		long	up_sman1;
		long	up_sman2;
		long	up_sman3;
		char	sale_stat [2];
		char	com_status [2];
		char	com_type [2];
		float	com_pc;
		Money	com_min;
		float	sman_com;
		float	level[4];
		char	stat_flag [2];
		char	update [2];
	}	exsf_rec;

	/*=====================================+
	 | Customers Salesman Commission file. |
	 +=====================================*/
#define	CUSC_NO_FIELDS	15

	struct dbview	cusc_list [CUSC_NO_FIELDS] =
	{
		{"cusc_hhsf_hash"},
		{"cusc_alloc_type"},
		{"cusc_sellgrp"},
		{"cusc_category"},
		{"cusc_spec_no"},
		{"cusc_ud_code"},
		{"cusc_hhbr_hash"},
		{"cusc_com_type"},
		{"cusc_com_pc"},
		{"cusc_com_min"},
		{"cusc_sman_com"},
		{"cusc_lev1_com"},
		{"cusc_lev2_com"},
		{"cusc_lev3_com"},
		{"cusc_lev4_com"}
	};

	struct tag_cuscRecord
	{
		long	hhsf_hash;
		int		alloc_type;
		char	sellgrp [7];
		char	category [12];
		int		spec_no;
		char	ud_code [3];
		long	hhbr_hash;
		char	com_type [2];
		float	com_pc;
		Money	com_min;
		float	sman_com;
		float	level[4];
	}	cusc_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
#define	CUMR_NO_FIELDS	6
	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	struct tag_cumrRecord
	{
		char	cm_co_no [3];
		char	cm_est_no [3];
		char	cm_dbt_no [7];
		long	cm_hhcu_hash;
		char	cm_name [41];
		char	cm_acronym [10];
	} cumr_rec;

	/*==============================
	| Inventory Master File (inmr) |
    ==============================*/
#define	INMR_NO_FIELDS	12
	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
		{"inmr_quick_code"},
	};

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		long 	mr_hhsi_hash;
		char 	mr_alpha_code[17];
		char 	mr_super_no[17];
		char 	mr_maker_no[17];
		char 	mr_alternate[17];
		char 	mr_class[2];
		char 	mr_description[41];
		char 	mr_category[12];
		char 	mr_quick_code[9];
	} inmr_rec;

	/*================================+
	 | External Category File Record. |
	 +================================*/
#define	EXCF_NO_FIELDS	5
	struct dbview	excf_list [EXCF_NO_FIELDS] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_hhcf_hash"},
		{"excf_cat_desc"},
		{"excf_stat_flag"}
	};

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		long	hhcf_hash;
		char	cat_desc [41];
		char	stat_flag [2];
	}	excf_rec;

	/*================================================+
	 | Inventory User Defined Specification Type file |
	 +================================================*/
#define	IUDS_NO_FIELDS	4
	struct dbview	iuds_list [IUDS_NO_FIELDS] =
	{
		{"iuds_co_no"},
		{"iuds_spec_no"},
		{"iuds_spec_desc"},
		{"iuds_dflt_code"}
	};

	struct tag_iudsRecord
	{
		char	co_no [3];
		int		spec_no;
		char	spec_desc [16];
		char	dflt_code [3];
	}	iuds_rec;

	/*================================================+
	 | Inventory User Defined Specification Type file |
	 +================================================*/
#define	IUDC_NO_FIELDS	4
	struct dbview	iudc_list [IUDC_NO_FIELDS] =
	{
		{"iudc_co_no"},
		{"iudc_spec_no"},
		{"iudc_code"},
		{"iudc_desc"}
	};

	struct tag_iudcRecord
	{
		char	co_no [3];
		int		spec_no;
		char	code [3];
		char	desc [41];
	}	iudc_rec;

	char	*comm = "comm",
			*cumr = "cumr",
			*exsf = "exsf",
			*excf = "excf",
			*cusc = "cusc",
			*ingp = "ingp",
			*inmr = "inmr",
			*iuds = "iuds",
			*iudc = "iudc",
			*data = "data";

	char	branchNo[3];
	int		envDbCo = FALSE;
	int		envDbFind  = FALSE;

	int		ByWhat	=	1;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	item_no[17];
} local_rec;

static struct var vars [] = 
{
	/*----------------
	| Salesman   	 |
	----------------*/
	{SCN_MAIN, LIN, "sman_code",	 2, 24, CHARTYPE,
		"UU", "          ",
		" ", "", "Salesman No.", "",
		 NE, NO, JUSTRIGHT, "", "", exsf_rec.sman_no},
	{SCN_MAIN, LIN, "sman_name",	 3, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Salesman name.", " ",
		NA, NO,  JUSTLEFT, "", "", exsf_rec.sman_name},
	{SCN_SELL_GRP, TAB, "sellgrp",	MAXLINES, 5, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Selling  Group ", "Enter item selling group. [SEARCH] ",
		 YES, NO,  JUSTLEFT, "", "", ingp_rec.code},
	{SCN_SELL_GRP, TAB, "selldesc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Selling Group Description.       ", " ",
		 NA, NO,  JUSTLEFT, "", "", ingp_rec.desc},
	{SCN_SELL_GRP, TAB, "com_type",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "N", "Comm. Type", " G=(% of Gross), M=(% of margin), N=(% of Nett)",
		YES, NO,  JUSTLEFT, "GMN", "", cusc_rec.com_type},
	{SCN_SELL_GRP, TAB, "sman_com",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 1  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.sman_com},
	{SCN_SELL_GRP, TAB, "level1",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 2  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[0]},
	{SCN_SELL_GRP, TAB, "level2",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 3  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[1]},
	{SCN_SELL_GRP, TAB, "level3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 4  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[2]},
	{SCN_SELL_GRP, TAB, "level4",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 5  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[3]},
	{SCN_CATEGORY, TAB, "cat_no",	MAXLINES, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "    Category    ", "Enter item category. [SEARCH].",
		NO, NO,  JUSTLEFT, "", "", excf_rec.cat_no},
	{SCN_CATEGORY, TAB, "cat_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            Description.                ", " ",
		 NA, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{SCN_CATEGORY, TAB, "com_type",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "N", "Comm. Type", " G=(% of Gross), M=(% of margin), N=(% of Nett)",
		YES, NO,  JUSTLEFT, "GMN", "", cusc_rec.com_type},
	{SCN_CATEGORY, TAB, "sman_com",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 1  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.sman_com},
	{SCN_CATEGORY, TAB, "level1",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 2  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[0]},
	{SCN_CATEGORY, TAB, "level2",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 3  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[1]},
	{SCN_CATEGORY, TAB, "level3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 4  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[2]},
	{SCN_CATEGORY, TAB, "level4",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 5  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[3]},
	{SCN_UD_CODE, TAB, "spec_no",	MAXLINES, 0, INTTYPE,
		"NN", "          ",
		" ", " ", "SN", "Enter specification number.",
		YES, NO, JUSTLEFT, "", "", (char *)&iuds_rec.spec_no},
	{SCN_UD_CODE, TAB, "spec_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Specification. ", "",
		NA, NO, JUSTLEFT, "", "", iuds_rec.spec_desc},
	{SCN_UD_CODE, TAB, "ucode",	0, 0, CHARTYPE,
		"UU", "          ",
		" ", " ", "UD", "Enter User Defined Code [Search]",
		NO, NO, JUSTLEFT, "", "", iudc_rec.code},
	{SCN_UD_CODE, TAB, "udesc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "    User Defined Code Description.      ", "",
		 NA, NO,  JUSTLEFT, "", "", iudc_rec.desc},
	{SCN_UD_CODE, TAB, "com_type",	 0, 1, CHARTYPE,
		"U", "          ",
		" ", "N", "CType", "Commission type. G=(% of Gross), M=(% of margin), N=(% of Nett)",
		YES, NO,  JUSTLEFT, "GMN", "", cusc_rec.com_type},
	{SCN_UD_CODE, TAB, "sman_com",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 1  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.sman_com},
	{SCN_UD_CODE, TAB, "level1",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 2  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[0]},
	{SCN_UD_CODE, TAB, "level2",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 3  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[1]},
	{SCN_UD_CODE, TAB, "level3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 4  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[2]},
	{SCN_UD_CODE, TAB, "level4",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 5  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[3]},
	{SCN_ITEM_NO, TAB, "item_no",	 MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "    Item no.    ", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{SCN_ITEM_NO, TAB, "item_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "               Description              ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.mr_description},
	{SCN_ITEM_NO, TAB, "hhbr_hash",	 0, 0, LONGTYPE,
		"NNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&inmr_rec.mr_hhbr_hash},
	{SCN_ITEM_NO, TAB, "com_type",	 0, 5, CHARTYPE,
		"U", "          ",
		" ", "N", "Comm. Type", " G=(% of Gross), M=(% of margin), N=(% of Nett)",
		YES, NO,  JUSTLEFT, "GMN", "", cusc_rec.com_type},
	{SCN_ITEM_NO, TAB, "sman_com",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 1  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.sman_com},
	{SCN_ITEM_NO, TAB, "level1",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 2  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[0]},
	{SCN_ITEM_NO, TAB, "level2",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 3  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[1]},
	{SCN_ITEM_NO, TAB, "level3",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 4  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[2]},
	{SCN_ITEM_NO, TAB, "level4",	 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.0", "  Level 5  ", "Input as a commission amoumt of as a percent ",
		YES, NO,  JUSTLEFT, "", "", (char *)&cusc_rec.level[3]},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

char	*scn_desc[] = {
			"SALESMAN SELECTION SCREEN .",
			"ALLOC. BY SELLING GROUP SCREEN.",
			"ALLOC. BY CATEGORY  SCREEN.",
			"ALLOC. BY UD CODE SCREEN.",
			"ALLOC. BY ITEM NUMBER SCREEN."
		};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void CloseDB (void);
int spec_valid (int);
void SrchIngp (char *);
void SrchExsf (char *);
void LoadTabular (void);
void Update (void);
void SrchExcf (char *);
void SrchIudc (char *);
void SrchIuds (char *);
static int DeleteLine (int);
int heading (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	int		i;

	SETUP_SCR( vars );

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	swide();

	for (i = 0;i < 5;i++)
		tab_data[i]._desc = scn_desc[i];

	tab_col = 0;
	tab_row = 5;

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	strcpy (branchNo, (envDbCo) ? comm_rec.test_no : " 0");

	X_EALL		=	40;
	Y_EALL		=	4;
	SR_X_POS	=	40;
	SR_Y_POS	=	4;

	while (!prog_exit)
	{
		search_ok  = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		init_vars (1);

		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		edit_all();

		if (restart || prog_exit)
			continue;

		Update ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no" );
	open_rec (cusc, cusc_list, CUSC_NO_FIELDS, "cusc_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (iuds, iuds_list, IUDS_NO_FIELDS, "iuds_id_no");
	open_rec (iudc, iudc_list, IUDC_NO_FIELDS, "iudc_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cusc);
	abc_fclose (ingp);
	abc_fclose (exsf);
	abc_fclose (iuds);
	abc_fclose (iudc);
	abc_fclose (excf);
	abc_fclose (inmr);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
	/*----------------------------
	| Validate Instruction Code. |
	----------------------------*/
	if (LCHECK("sman_code"))
	{
		if (SRCH_KEY)
		{
			SrchExsf(temp_str);
			return(0);
		}
	
		strcpy(exsf_rec.co_no,comm_rec.tco_no);
		cc = find_rec("exsf", &exsf_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		LoadTabular ();
		entry_exit = TRUE;
		DSP_FLD ("sman_name");
		return(0);
	}
	/*--------------------
	| Validate Category. |
	--------------------*/
	if (LCHECK ("cat_no"))
	{
		if (last_char == DELLINE)
			return ( DeleteLine (SCN_CATEGORY));

		if (dflt_used)
		{
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no, comm_rec.tco_no);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("cat_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Selling Group. |
	-------------------------*/
	if (LCHECK ("sellgrp"))
	{
		if (last_char == DELLINE)
			return ( DeleteLine (SCN_SELL_GRP));

		if (dflt_used)
        {
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ingp_rec.co_no, comm_rec.tco_no);
		strcpy (ingp_rec.type, "S");

		cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
		
		if (cc)
		{
			errmess (ML (mlStdMess208));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("selldesc");
		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK("item_no"))
	{
		if (dflt_used)
        {
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return ( DeleteLine (SCN_ITEM_NO));

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			return(1);
		}
		SuperSynonymError ();
		
		strcpy(local_rec.item_no,inmr_rec.mr_item_no);
		DSP_FLD ("item_no");
		DSP_FLD ("item_desc");
		return(0);
	}
	/*----------------------------------
	| Validate User specification Code |
	----------------------------------*/
	if (LCHECK ("spec_no"))
	{
		if (dflt_used)
        {
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return ( DeleteLine (SCN_UD_CODE));

		if (SRCH_KEY)
		{
			SrchIuds (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (iuds_rec.co_no,comm_rec.tco_no);
		cc = find_rec ("iuds", &iuds_rec, COMPARISON,"r");
		if (cc)
		{
			errmess (ML ("User Defined Code is not on file"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("spec_desc");
		return(0);
	}

	/*--------------------
	| Validate User Code |
	--------------------*/
	if (LCHECK ("ucode"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (last_char == DELLINE)
			return ( DeleteLine (SCN_UD_CODE));

		if (SRCH_KEY)
		{
			SrchIudc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (iudc_rec.co_no,comm_rec.tco_no);
		iudc_rec.spec_no = iuds_rec.spec_no;
		cc = find_rec ("iudc", &iudc_rec, COMPARISON,"r");
		if (cc)
		{
			errmess (ML ("User Defined Code is not on file"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("udesc");
		return (EXIT_SUCCESS);
	}
	/*----------------------------------------
	| Validate Salesmans commission percent. |
	----------------------------------------*/
	if (LCHECK ("sman_com"))
	{
		if (dflt_used)
		{
			cusc_rec.sman_com = exsf_rec.sman_com;
		}
		return (EXIT_SUCCESS);
	}
			
	/*-------------------------
	| Validate Level Percent. |
	-------------------------*/
	if (LCHECK ("level1"))
	{
		if (dflt_used)
		{
			cusc_rec.level[0] = exsf_rec.level[0];
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Level Percent. |
	-------------------------*/
	if (LCHECK ("level2"))
	{
		if (dflt_used)
		{
			cusc_rec.level[1] = exsf_rec.level[1];
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Level Percent. |
	-------------------------*/
	if (LCHECK ("level3"))
	{
		if (dflt_used)
		{
			cusc_rec.level[2] = exsf_rec.level[2];
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Level Percent. |
	-------------------------*/
	if (LCHECK ("level4"))
	{
		if (dflt_used)
		{
			cusc_rec.level[3] = exsf_rec.level[3];
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=====================================================================
| Item Group Search.                                                  |
=====================================================================*/
void
SrchIngp (
 char*  key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "S");
	sprintf (ingp_rec.code, "%-6.6s", key_val);

	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.tco_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type[0] == 'S')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		else
			break;

		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "S");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

/*==================
| Salesman Search. |
==================*/
void
SrchExsf (
 char*  key_val)
{
	work_open();
	save_rec("#SM","#Salesman's Name");
	strcpy(exsf_rec.co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sman_no,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");

	while (!cc && !strcmp(exsf_rec.co_no,comm_rec.tco_no) &&
				  !strncmp(exsf_rec.sman_no,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sman_no,exsf_rec.sman_name);
		if (cc)
			break;

		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sman_no,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "exsf", "DBFIND");
}

/*=====================================================================
| Update function. |
=====================================================================*/
void	
LoadTabular (void)
{
	abc_selfield (cusc, "cusc_id_no");

	/*--------------------------------
	| Load Records by Selling Group. |
	--------------------------------*/
	scn_set (SCN_SELL_GRP);
	lcount [SCN_SELL_GRP]	=	0;

	memset (&cusc_rec,0,sizeof(cusc_rec));

	cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
	cusc_rec.alloc_type	=	BySellingGroup;
	cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	while (!cc && cusc_rec.hhsf_hash == exsf_rec.hhsf_hash &&
				  cusc_rec.alloc_type == BySellingGroup )
	{
		strcpy (ingp_rec.co_no, comm_rec.tco_no);
		strcpy (ingp_rec.code,  cusc_rec.sellgrp);
		strcpy (ingp_rec.type, "S");
		if ( !find_rec (ingp, &ingp_rec, COMPARISON, "r") )
			putval (lcount [SCN_SELL_GRP]++);

		cc = find_rec (cusc, &cusc_rec, NEXT, "r");
	}

	/*---------------------------
	| Load Records by Category. |
	---------------------------*/
	scn_set (SCN_CATEGORY);
	lcount [SCN_CATEGORY]	=	0;
	memset (&cusc_rec,0,sizeof(cusc_rec));

	cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
	cusc_rec.alloc_type	=	ByCategory;
	cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	while (!cc && cusc_rec.hhsf_hash == exsf_rec.hhsf_hash &&
				  cusc_rec.alloc_type == ByCategory )
	{
		strcpy (excf_rec.co_no, comm_rec.tco_no);
		strcpy (excf_rec.cat_no,cusc_rec.category);
		if ( !find_rec (excf, &excf_rec, COMPARISON, "r") )
			putval (lcount [SCN_CATEGORY]++);

		cc = find_rec (cusc, &cusc_rec, NEXT, "r");
	}

	scn_set (SCN_UD_CODE);
	lcount [SCN_UD_CODE ]	=	0;
	
	memset (&cusc_rec,0,sizeof(cusc_rec));

	cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
	cusc_rec.alloc_type	=	ByUDCode;
	cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	while (!cc && cusc_rec.hhsf_hash == exsf_rec.hhsf_hash &&
				  cusc_rec.alloc_type == ByUDCode )
	{
		strcpy (iuds_rec.co_no,comm_rec.tco_no);
		iuds_rec.spec_no	=	cusc_rec.spec_no;
		cc = find_rec ("iuds", &iuds_rec, COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (cusc, &cusc_rec, NEXT, "r");
			continue;
		}

		strcpy (iudc_rec.co_no,comm_rec.tco_no);
		iudc_rec.spec_no = iuds_rec.spec_no;
		strcpy (iudc_rec.code, cusc_rec.ud_code);
		if ( !find_rec ("iudc", &iudc_rec, COMPARISON,"r"))
				putval (lcount [SCN_UD_CODE]++);

		cc = find_rec (cusc, &cusc_rec, NEXT, "r");
	}

	scn_set (SCN_ITEM_NO);
	lcount [SCN_ITEM_NO ]	=	0;
	abc_selfield (inmr, "inmr_hhbr_hash");
	
	memset (&cusc_rec,0,sizeof(cusc_rec));

	cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
	cusc_rec.alloc_type	=	ByItemNo;
	cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	while (!cc && cusc_rec.hhsf_hash == exsf_rec.hhsf_hash &&
				  cusc_rec.alloc_type == ByItemNo )
	{
		inmr_rec.mr_hhbr_hash = cusc_rec.hhbr_hash;
		if ( !find_rec (inmr, &inmr_rec, COMPARISON, "r") )
		{
			strcpy (local_rec.item_no, inmr_rec.mr_item_no);
			putval (lcount [SCN_ITEM_NO]++);
		}

		cc = find_rec (cusc, &cusc_rec, NEXT, "r");
	}
	abc_selfield (inmr, "inmr_id_no");
	scn_set (SCN_MAIN);
}

/*==================
| Update function. |
==================*/
void	
Update (void)
{
	int		NewCusc;
	abc_selfield (cusc, "cusc_id_no");

	/*----------------------------------
	| Delete out all existing records. |
	----------------------------------*/
	cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
	cusc_rec.alloc_type	=	0;
	cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	while (!cc && cusc_rec.hhsf_hash == exsf_rec.hhsf_hash)
	{
		print_mess( "Removing old records" );
		cc = abc_delete (cusc);
		if (cc)
			file_err (cc, cusc, "DBDELETE");

		cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
		cusc_rec.alloc_type	=	0;
		cc = find_rec (cusc, &cusc_rec, GTEQ, "r");
	}

	abc_selfield (cusc, "cusc_id_no2");

	scn_set (SCN_SELL_GRP);
	for (line_cnt = 0;line_cnt < lcount [SCN_SELL_GRP];line_cnt++) 
	{
		memset (&cusc_rec,0,sizeof(cusc_rec));

		getval (line_cnt);
		cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
		strcpy (cusc_rec.sellgrp, ingp_rec.code );
		NewCusc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
		
		getval (line_cnt);

		cusc_rec.alloc_type	=	BySellingGroup;
		if (NewCusc)
		{
			cc = abc_add (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBADD");
		}
		else
		{
			cc = abc_update (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBUPDATE");
		}
	}
	scn_set (SCN_CATEGORY);
	for (line_cnt = 0;line_cnt < lcount [SCN_CATEGORY];line_cnt++) 
	{
		memset (&cusc_rec,0,sizeof(cusc_rec));

		getval (line_cnt);
		cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
		strcpy (cusc_rec.category, excf_rec.cat_no );
		NewCusc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
		
		getval (line_cnt);

		cusc_rec.alloc_type	=	ByCategory;
		if (NewCusc)
		{
			cc = abc_add (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBADD");
		}
		else
		{
			cc = abc_update (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBUPDATE");
		}
			
	}
	scn_set (SCN_UD_CODE);
	for (line_cnt = 0;line_cnt < lcount[ SCN_UD_CODE ];line_cnt++) 
	{
		memset (&cusc_rec,0,sizeof(cusc_rec));

		getval (line_cnt);

		cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
		cusc_rec.spec_no	=	iuds_rec.spec_no;
		strcpy (cusc_rec.ud_code, iudc_rec.code);
		NewCusc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
		
		getval (line_cnt);

		cusc_rec.alloc_type	=	ByUDCode;
		if (NewCusc)
		{
			cc = abc_add (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBADD");
		}
		else
		{
			cc = abc_update (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBUPDATE");
		}
	}
	scn_set (SCN_ITEM_NO);
	for (line_cnt = 0;line_cnt < lcount [SCN_ITEM_NO];line_cnt++) 
	{
		memset (&cusc_rec,0,sizeof(cusc_rec));

		getval (line_cnt);

		cusc_rec.hhsf_hash 	=	exsf_rec.hhsf_hash;
		cusc_rec.hhbr_hash	=	inmr_rec.mr_hhbr_hash;
		NewCusc = find_rec (cusc, &cusc_rec, COMPARISON, "r");
		
		getval (line_cnt);

		cusc_rec.alloc_type	=	ByItemNo;
		if (NewCusc)
		{
			cc = abc_add (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBADD");
		}
		else
		{
			cc = abc_update (cusc, &cusc_rec);
			if (cc)
				file_err (cc, cusc, "DBUPDATE");
		}
	}
	abc_selfield (cusc, "cusc_id_no");
}

/*=====================================================================
| Search for Category master file.                                    |
=====================================================================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	strcpy (excf_rec.co_no, comm_rec.tco_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	save_rec ("#Cat No", "#Category Description");
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
		      !strcmp (excf_rec.co_no, comm_rec.tco_no))
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

	strcpy (excf_rec.co_no, comm_rec.tco_no);
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, excf, "DBFIND");
}

/*=======================================
| Item user defined specification code. |
=======================================*/
void
SrchIudc (
 char*  key_val)
{
	work_open();
	save_rec("#Cd","#Description");
	strcpy (iudc_rec.co_no,comm_rec.tco_no);
	iudc_rec.spec_no = iuds_rec.spec_no;
	strcpy (iudc_rec.code,key_val);
	cc = find_rec ("iudc", &iudc_rec, GTEQ, "r");

	while (!cc && !strcmp(iudc_rec.co_no,comm_rec.tco_no) &&
		iudc_rec.spec_no == iuds_rec.spec_no &&
		!strncmp(iudc_rec.code,key_val,strlen(key_val)))
	{
		cc = save_rec(iudc_rec.code,iudc_rec.desc);
		if (cc)
			break;

		cc = find_rec("iudc",&iudc_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(iudc_rec.co_no,comm_rec.tco_no);
	iudc_rec.spec_no = iuds_rec.spec_no;
	sprintf(iudc_rec.code,"%-2.2s",temp_str);
	cc = find_rec("iudc",&iudc_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "iudc", "DBFIND");
}

/*=======================================
| Item user defined specification code. |
=======================================*/
void
SrchIuds (
 char*  key_val)
{
	work_open();
	save_rec("#Cd","#Description");
	strcpy (iuds_rec.co_no,comm_rec.tco_no);
	iuds_rec.spec_no = atoi (key_val);
	cc = find_rec ("iuds", &iuds_rec, GTEQ, "r");

	while (!cc && !strcmp(iuds_rec.co_no,comm_rec.tco_no) )
	{
		sprintf( err_str, "%02d", iuds_rec.spec_no);
		cc = save_rec (err_str, iuds_rec.spec_desc);
		if (cc)
			break;

		cc = find_rec("iuds",&iuds_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(iuds_rec.co_no,comm_rec.tco_no);
	iuds_rec.spec_no = atoi (temp_str);
	cc = find_rec("iuds",&iuds_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "iuds", "DBFIND");
}

/*=====================================================================
| Delete Line.                                                        |
=====================================================================*/
static int	
DeleteLine (
 int    ScreenNo)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (" Cannot Delete Lines on Entry ");
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [ScreenNo] == 0)
	{
		print_mess (" Cannot Delete Line - No Lines to Delete ");
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [ScreenNo]--;

	for (i = line_cnt, line_cnt = 0;line_cnt < lcount [ScreenNo];line_cnt++)
	{
		if (line_cnt >= i)
			getval (line_cnt + 1);
		else
			getval (line_cnt);
		
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	memset (&cusc_rec,0,sizeof(cusc_rec));

	if ( ScreenNo == SCN_SELL_GRP )
	{
		strcpy (ingp_rec.code, "      ");
		sprintf (ingp_rec.desc, "%40.40s", " ");
	}
	if ( ScreenNo == SCN_CATEGORY )
	{
		strcpy (excf_rec.cat_no, "           ");
		sprintf (excf_rec.cat_desc, "%40.40s", " ");
	}
	if ( ScreenNo == SCN_UD_CODE )
	{
		iuds_rec.spec_no	=	0;
		strcpy (iuds_rec.spec_desc,"               ");
		strcpy (iudc_rec.code, "  ");
		sprintf (iudc_rec.desc,"%40.40s", " ");
	}
	if ( ScreenNo == SCN_ITEM_NO )
	{
		sprintf (local_rec.item_no, "%16.16s", " ");
		sprintf (inmr_rec.mr_description, "%40.40s", " ");
	}
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*==================
| Heading routine. |
==================*/
int
heading (
 int    scn)
{
	if (restart)
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);

	if (scn == SCN_MAIN)
	{
		clear ();
		sprintf (err_str, " Salesman Commission Allocation Maintenance ");
		rv_pr (err_str, (65 - (strlen (err_str) / 2)), 0, 1);

		move (0, 21);
		line (132);

		print_at (22, 0, "Co : %s - %s", comm_rec.tco_no, comm_rec.tco_name);

		box (0, 1, 132, 2);		
	}
	else
	{
		/*-------------------------------
		| Other screens must be redrawn
		| if this screen is redrawn
		-------------------------------*/
		scn_display ( 1 );

		box(tab_col, tab_row - 1, 130, TABLINES + 2);
	}
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
