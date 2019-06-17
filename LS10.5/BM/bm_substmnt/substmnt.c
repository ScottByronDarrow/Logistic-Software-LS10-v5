/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bm_substmnt.c )                                  |
|  Program Desc  : ( Substitute Components Of a Bill              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmms, comm, excf, inmr, inum,                     |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  bmms,                                             |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written : 19/06/87          |
|---------------------------------------------------------------------|
|  Date Modified : (19/06/87)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (19/07/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (09/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (04/11/93)      | Modified  by : Campbell Mander.  |
|  Date Modified : (05/04/94)      | Modified  by : Roel Michels      |
|  Date Modified : (05/04/94)      | Modified  by : Dirk Heinsius.    |
|  Date Modified : (08/11/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (09/03/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      : (19/07/91) - Chgs for quick code & genl tidy-up.   |
|  (09/09/91)    : Added range for substitution.                      |
|  (03/02/92)    : Rename pcms to bmms and progname from pc to bm.    |
|  (04/11/93)    : HGP 9501. Remove inmr_price1 - 5.                  |
|  (05/04/94)    : PSL 10673 - find_hash with wrong index             |
|  (20/09/94)    : PSM 11278 - Fix error in update (changed index).   |
|  (08/11/94)    : PSL 11527 - fix errors with category               |
|  (09/03/97)    : Updated for Multilingual Conversion.				  |
|                                                                     |
| $Log: substmnt.c,v $
| Revision 5.4  2002/11/26 03:11:32  kaarlo
| LS01115 SC4172. Updated to include st_class and end_class validation.
|
| Revision 5.3  2001/08/20 23:31:36  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 08:24:38  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:24  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:46:16  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:36:49  gerry
| Force Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/06/13 05:01:48  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.15  1999/12/06 03:33:30  cam
| Changes for GVision compatibility.  Added sleep (2) and clear_mess () after print_mess ().
|
| Revision 1.14  1999/11/17 06:39:01  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/05 05:49:33  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.12  1999/10/01 07:39:08  scott
| Updated for function standards
|
| Revision 1.11  1999/09/29 10:10:07  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/23 05:49:21  scott
| Updated from Ansi Project.
|
| Revision 1.9  1999/09/15 04:37:19  scott
| Updated from Ansi.
|
| Revision 1.8  1999/06/14 23:14:04  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: substmnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_substmnt/substmnt.c,v 5.4 2002/11/26 03:11:32 kaarlo Exp $";

#include <pslscr.h>
#include <getnum.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>

#define	BY_ITEM		(local_rec.range_type[0] == 'I')

#define	OLD	0
#define	NEW	1

	/*==============================
	| Material Specification File. |
	==============================*/
	struct dbview bmms_list[] =
	{
		{"bmms_co_no"},
		{"bmms_hhbr_hash"},
		{"bmms_alt_no"},
		{"bmms_line_no"},
		{"bmms_mabr_hash"},
		{"bmms_uom"},
		{"bmms_matl_qty"},
		{"bmms_matl_wst_pc"},
		{"bmms_instr_no"}
	};

	int bmms_no_fields = 9;

	struct
	{
		char	ms_co_no[3];
		long	ms_hhbr_hash;
		int		ms_alt_no;
		int		ms_line_no;
		long	ms_mabr_hash;
		long	ms_uom;
		float	ms_matl_qty;
		float	ms_matl_wst_pc;
		int		ms_instr_no;
	} bmms2_rec, bmms_rec;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"}
	};

	int comm_no_fields = 3;

	struct
	{
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
	} comm_rec;

	/*===============================
	| External Category File Record |
	===============================*/
	struct dbview excf_list[] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
		{"excf_stat_flag"},
	};

	int	excf_no_fields = 4;

	struct
	{
		char	cf_co_no[3];
		char	cf_cat_no[12];
		char	cf_description[41];
		char	cf_stat_flag[2];
	} excf_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list[] =
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
		{"inmr_description2"},
		{"inmr_category"},
		{"inmr_quick_code"},
		{"inmr_abc_code"},
		{"inmr_abc_update"},
		{"inmr_serial_item"},
		{"inmr_lot_ctrl"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_pack_size"},
		{"inmr_source"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
		{"inmr_stat_flag"},
	};

	int	inmr_no_fields = 25;

	struct
	{
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_alpha_code[17];
		char	mr_super_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_description2[41];
		char	mr_category[12];
		char	mr_quick_code[9];
		char	mr_abc_code[2];
		char	mr_abc_update[2];
		char	mr_serial_item[2];
		char	mr_lot_ctrl[2];
		char	mr_costing_flag[2];
		char	mr_sale_unit[5];
		char	mr_pack_size[6];
		char	mr_source[3];
		long	mr_std_uom;
		long	mr_alt_uom;
		float	mr_uom_cfactor;
		char	mr_stat_flag[2];
	} inmr2_rec, inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list[] =
	{
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"},
	};

	int	inum_no_fields = 5;

	struct
	{
		char	um_uom_group[21];
		long	um_hhum_hash;
		char	um_uom[5];
		char	um_desc[41];
		float	um_cnv_fct;
	} inum_rec;

	char	*bmms	= "bmms",
			*bmms2	= "bmms2",
			*data	= "data",
			*comm	= "comm",
			*excf	= "excf",
			*inmr	= "inmr",
			*inmr2	= "inmr2",
			*inum	= "inum";

	char	run_type[2];

	int	new_item = 0;

/*----------------------------------
| uom_base[0]... is always old std |
| uom_base[1]... is always old alt |
| uom_base[2]... is always new alt |
----------------------------------*/
struct	{
	long	hash;
	char	grp[21];
	char	uom[5];
	float	cnv_fct;
} uom_from[64];

struct	{
	long	hash;
	char	grp[21];
	char	uom[5];
} uom_to;

struct	{
	long	std_hash;
	char	std_grp[21];
	char	std_uom[5];
	float	std_cnv_fct;

	long	alt_hash;
	char	alt_grp[21];
	char	alt_uom[5];
	float	alt_cnv_fct;
} uom_base[2];

float	old_alt_cfactor;
float	new_alt_cfactor;
float	conv_fact;
int	no_in_tab;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	long	old_hhbr_hash;
	char	old_prod_no[17];
	char	old_prod_desc[41];
	char	old_prod_desc2[41];
	long	old_uom[2];
	long	new_hhbr_hash;
	char	new_prod_no[17];
	char	new_prod_desc[41];
	char	new_prod_desc2[41];
	long	new_uom[2];
	char	range_type[2];
	char	st_item[17];
	char	st_item_desc[41];
	char	end_item[17];
	char	end_item_desc[41];
	char	st_class[2];
	char	st_cat[12];
	char	st_cat_desc[41];
	char	end_class[2];
	char	end_cat[12];
	char	end_cat_desc[41];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "old_prod",	 4, 23, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Existing Old Product  :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.old_prod_no},
	{1, LIN, "desc",		 5, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description           :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.old_prod_desc},
	{1, LIN, "desc",		 6, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                       :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.old_prod_desc2},
	{1, LIN, "new_prod",	 8, 23, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " New Substitute Product:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.new_prod_no},
	{1, LIN, "desc",		 9, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description           :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.new_prod_desc},
	{1, LIN, "desc",		10, 23, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                       :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.new_prod_desc2},
	{1, LIN, "st_item",	12, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Start Item   :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_item_desc",	12, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_item_desc},
	{1, LIN, "end_item",	13, 15, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " End Item     :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_item_desc",13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_item_desc},

	{1, LIN, "st_class",	12, 15, CHARTYPE,
		"U", "          ",
		" ", "A", " Start Class   :", "Input Start Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.st_class},
	{1, LIN, "st_cat",	13, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " Start Category:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat},
	{1, LIN, "st_cat_desc",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat_desc},
	{1, LIN, "end_class",	15, 15, CHARTYPE,
		"U", "          ",
		" ", "A", " End Class    :", "Input End Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.end_class},
	{1, LIN, "end_cat",	16, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " End Category :", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat},
	{1, LIN, "end_cat_desc",	16, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat_desc},
	
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void update (void);
int calc_conv (void);
void show_new (char *key_val);
void SrchExcf (char *key_val);
int heading (int scn);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{

	if (argc != 2)
	{
		/*printf("\007 Usage : %s <I(tem range) G(roup range)>\n", argv[0]);*/
		print_at(0,0, ML(mlBmMess700), argv[0]);
        return (EXIT_FAILURE);
	}
	sprintf(local_rec.range_type, "%-1.1s", argv[1]);

	SETUP_SCR( vars );

	if (!BY_ITEM)
	{
		FLD("st_item") 		= ND;
		FLD("st_item_desc") = ND;
		FLD("end_item") 	= ND;
		FLD("end_item_desc") = ND;

		FLD("st_class") = YES;
		FLD("end_class") = YES;
		FLD("st_cat") = YES;
		FLD("st_cat_desc") = NA;
		FLD("end_cat") = YES;
		FLD("end_cat_desc") = NA;
	}

	init_scr();
	set_tty();
	set_masks();
	init_vars(1);

	OpenDB();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		calc_conv();

		update();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias(bmms2,bmms);
	abc_alias(inmr2,inmr);

	open_rec(bmms,bmms_list,bmms_no_fields, "bmms_id_no_2");
	open_rec(bmms2,bmms_list,bmms_no_fields,"bmms_mabr_hash");
	open_rec(excf,excf_list,excf_no_fields,"excf_id_no");
	open_rec(inmr,inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec(inmr2,inmr_list,inmr_no_fields,"inmr_hhbr_hash");
	open_rec(inum,inum_list,inum_no_fields,"inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(bmms);
	abc_fclose(bmms2);
	abc_fclose(excf);
	abc_fclose(inmr);
	abc_fclose(inmr2);
	abc_fclose(inum);
	SearchFindClose ();

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	if (LCHECK("old_prod"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.old_prod_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.old_prod_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		if (strcmp(inmr_rec.mr_source, "RM") &&
		    strcmp(inmr_rec.mr_source, "MC"))
		{
			print_mess(ML(mlBmMess004));
			sleep(2);
			clear_mess();
			return(1);
		}

		strcpy(bmms_rec.ms_co_no,comm_rec.tco_no);
		bmms_rec.ms_mabr_hash = inmr_rec.mr_hhbr_hash;
		bmms_rec.ms_hhbr_hash = 0L;
		cc = find_rec(bmms,&bmms_rec,GTEQ,"r");
		if (cc || strcmp(bmms_rec.ms_co_no,comm_rec.tco_no) ||
			bmms_rec.ms_mabr_hash != inmr_rec.mr_hhbr_hash)
		{
			print_mess(ML(mlBmMess006));
			sleep(2);
			clear_mess();
			return(1);
		}

		local_rec.old_hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy(local_rec.old_prod_desc,inmr_rec.mr_description);
		strcpy(local_rec.old_prod_desc2,inmr_rec.mr_description2);
		local_rec.old_uom[0] = inmr_rec.mr_std_uom;
		local_rec.old_uom[1] = inmr_rec.mr_alt_uom;
		old_alt_cfactor = inmr_rec.mr_uom_cfactor;
		display_field(field + 1);
		display_field(field + 2);

		return(0);
	}

	if (LCHECK("new_prod"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.new_prod_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.new_prod_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		if (strcmp(inmr_rec.mr_source, "RM") &&
		    strcmp(inmr_rec.mr_source, "MC"))
		{
			print_mess(ML(mlBmMess004));
			sleep(2);
			clear_mess();
			return(1);
		}

		local_rec.new_hhbr_hash = inmr_rec.mr_hhbr_hash;

		strcpy(local_rec.new_prod_desc,inmr_rec.mr_description);
		strcpy(local_rec.new_prod_desc2,inmr_rec.mr_description2);
		local_rec.new_uom[0] = inmr_rec.mr_std_uom;
		local_rec.new_uom[1] = inmr_rec.mr_alt_uom;
		new_alt_cfactor = inmr_rec.mr_uom_cfactor;
		display_field(field + 1);
		display_field(field + 2);
		return(0);
	}

	if (LCHECK("st_item"))
	{
		if (FLD("st_item") == ND)
			return(0);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.st_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return(1);
		}

		SuperSynonymError ();

		strcpy(local_rec.st_item_desc,inmr_rec.mr_description);
		DSP_FLD("st_item_desc");

		return(0);
	}

	if (LCHECK("end_item"))
	{
		if (FLD("end_item") == ND)
			return(0);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}

		cc = FindInmr (comm_rec.tco_no, local_rec.end_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.end_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return(1);
		}
		SuperSynonymError ();

		strcpy(local_rec.end_item_desc,inmr_rec.mr_description);
		DSP_FLD("end_item_desc");

		return(0);
	}

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK("st_cat"))
	{
		if (FLD("st_cat") == ND)
			return(0);

		if (SRCH_KEY)
		{
			SrchExcf(temp_str);
			return(0);
		}

		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.st_cat);

		if (!dflt_used)
		{
			cc = find_rec(excf,&excf_rec,COMPARISON,"r");
			if (cc)
			{
				errmess(ML(mlStdMess004));
				sleep(5);
				return(1);
			}
		}
		else
		{
			sprintf(local_rec.st_cat,"%-11.11s","           ");
			sprintf(excf_rec.cf_description,"%-40.40s","BEGINNING OF RANGE");
		}
		if (prog_status != ENTRY && strcmp(local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			errmess(ML(mlStdMess006));
			sleep(5);
			return(0);
		}
		sprintf(local_rec.st_cat_desc,"%-40.40s",excf_rec.cf_description);
		display_field(field + 1);
		return(0);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK("end_cat"))
	{
		if (FLD("end_cat") == ND)
			return(0);

		if (SRCH_KEY)
		{
			SrchExcf(temp_str);
			return(0);
		}

		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.end_cat);

		if (!dflt_used)
		{
			cc = find_rec(excf,&excf_rec,COMPARISON,"r");
			if (cc)
			{
				errmess(ML(mlStdMess004));
				sleep(3);
				return(1);
			}
		}
		else
		{
			sprintf(local_rec.end_cat,"%-11.11s","~~~~~~~~~~~");
			sprintf(excf_rec.cf_description,"%-40.40s","END OF RANGE");
		}
		if (strcmp(local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			errmess(ML(mlStdMess006));
			sleep(3);
			return(1);
		}
		strcpy(local_rec.end_cat_desc,excf_rec.cf_description);
		display_field(field + 1);
		return(0);
	}

	/*----------------------
	| Validate start class |
	----------------------*/
	if (LCHECK ("st_class"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && 
				strcmp (local_rec.st_class, local_rec.end_class) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate end class |
	----------------------*/
	if (LCHECK ("end_class"))
	{
		if (strcmp (local_rec.st_class, local_rec.end_class) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		return (EXIT_SUCCESS);
	}
	
	return(0);
}

void
update (void)
{
	int		i;

	clear();

	print_at(0,0, ML(mlBmMess013)); 

	fflush(stdout);
	abc_selfield (bmms, "bmms_id_no");
	strcpy(bmms_rec.ms_co_no,comm_rec.tco_no);
	bmms_rec.ms_mabr_hash = local_rec.old_hhbr_hash;
	bmms_rec.ms_line_no = 0;
	cc = find_hash (bmms2, &bmms_rec, GTEQ, "u", local_rec.old_hhbr_hash);
	while (!cc && bmms_rec.ms_mabr_hash == local_rec.old_hhbr_hash)
	{
		if (find_hash(inmr2, &inmr2_rec, COMPARISON, "r",bmms_rec.ms_hhbr_hash))
		{
			abc_unlock (bmms2);
			cc = find_hash(bmms2,&bmms_rec,NEXT,"u",
				local_rec.old_hhbr_hash);
			continue;
		}

		if (BY_ITEM &&
			(strcmp(inmr2_rec.mr_item_no, local_rec.st_item) < 0 ||
		    strcmp(inmr2_rec.mr_item_no, local_rec.end_item) > 0))
		{
			abc_unlock (bmms2);
			cc = find_hash(bmms2,&bmms_rec,NEXT,"u",
				local_rec.old_hhbr_hash);
			continue;
		}

		if (!BY_ITEM &&
			((strcmp (inmr2_rec.mr_class, local_rec.st_class) < 0 &&
		    strcmp (inmr2_rec.mr_category, local_rec.st_cat) < 0) ||
			(strcmp (inmr2_rec.mr_class, local_rec.end_class) > 0 &&
			strcmp (inmr2_rec.mr_category, local_rec.end_cat) > 0)))
		{
			abc_unlock (bmms2);
			cc = find_hash (bmms2, &bmms_rec, NEXT, "u",
				local_rec.old_hhbr_hash);
			continue;
		}

		cc = find_hash(inum,&inum_rec,COMPARISON,"r",bmms_rec.ms_uom);
		if (cc)
		{
			abc_unlock (bmms2);
			cc = find_hash(bmms2,&bmms_rec,NEXT,"u",
				local_rec.old_hhbr_hash);
			continue;
		}

		cc = find_rec (bmms, &bmms_rec, EQUAL, "u");
		if (cc)
			file_err (cc, bmms, "DBFIND");

		putchar('U');
		fflush(stdout);

		bmms_rec.ms_mabr_hash = local_rec.new_hhbr_hash;

		if (strcmp(inum_rec.um_uom_group, uom_base[NEW].std_grp))
		{
			bmms_rec.ms_matl_qty *= conv_fact;

			for (i = 0; i < no_in_tab; i++)
			{
				if (bmms_rec.ms_uom == uom_from[i].hash)
				{
					bmms_rec.ms_matl_qty *= uom_from[i].cnv_fct;
					break;
				}
			}

			bmms_rec.ms_uom = uom_base[NEW].std_hash;
		}

		cc = abc_update(bmms,&bmms_rec);
		if (cc)
			file_err(cc, "bmms", "DBUPDATE");

		cc = find_hash (bmms2, &bmms_rec, GTEQ, "u", local_rec.old_hhbr_hash);
	}
	abc_selfield (bmms, "bmms_id_no_2");
}

int
calc_conv (void)
{
	char	tmp_err[21];
	int	invalid_uom = 0;

	/*------------------------
	| Check if conversion is |
	| possible between any of|
	| the UOMs		 |
	------------------------*/
	if (find_hash(inum, &inum_rec, COMPARISON, "r", local_rec.old_uom[0]))
		invalid_uom = 1;
	else
	{
		sprintf(uom_base[OLD].std_grp, "%-20.20s", inum_rec.um_uom_group);
		sprintf(uom_base[OLD].std_uom, "%-4.4s", inum_rec.um_uom);
		uom_base[OLD].std_hash = inum_rec.um_hhum_hash;
		uom_base[OLD].std_cnv_fct = inum_rec.um_cnv_fct;
	}

	if (find_hash(inum, &inum_rec, COMPARISON, "r", local_rec.old_uom[1]))
		invalid_uom = 2;
	else
	{
		sprintf(uom_base[OLD].alt_grp, "%-20.20s", inum_rec.um_uom_group);
		sprintf(uom_base[OLD].alt_uom, "%-4.4s", inum_rec.um_uom);
		uom_base[OLD].alt_hash = inum_rec.um_hhum_hash;
		uom_base[OLD].alt_cnv_fct = inum_rec.um_cnv_fct;
	}

	if (find_hash(inum, &inum_rec, COMPARISON, "r", local_rec.new_uom[0]))
		invalid_uom = 3;
	else
	{
		sprintf(uom_base[NEW].std_grp, "%-20.20s", inum_rec.um_uom_group);
		sprintf(uom_base[NEW].std_uom, "%-4.4s", inum_rec.um_uom);
		uom_base[NEW].std_hash = inum_rec.um_hhum_hash;
		uom_base[NEW].std_cnv_fct = inum_rec.um_cnv_fct;
	}

	if (find_hash(inum, &inum_rec, COMPARISON, "r", local_rec.new_uom[1]))
		invalid_uom = 4;
	else
	{
		sprintf(uom_base[NEW].alt_grp, "%-20.20s", inum_rec.um_uom_group);
		sprintf(uom_base[NEW].alt_uom, "%-4.4s", inum_rec.um_uom);
		uom_base[NEW].alt_hash = inum_rec.um_hhum_hash;
		uom_base[NEW].alt_cnv_fct = inum_rec.um_cnv_fct;
	}

	if (invalid_uom)
	{
		switch(invalid_uom)
		{
		case 1:
			strcpy(tmp_err, ML(mlStdMess064));
			break;

		case 2:
			strcpy(tmp_err, ML(mlStdMess065));
			break;

		case 3:
			strcpy(tmp_err, ML(mlStdMess066));
			break;

		case 4:
			strcpy(tmp_err, ML(mlStdMess067));
			break;
		}
		print_mess(tmp_err);
		sleep(3);
		clear_mess();
		return(FALSE);
	}

	/*---------------------------------
	| No conversion possible so force |
	| entry of conv. factor between   |
	| the std UOMs			  |
	---------------------------------*/
	if (strcmp(uom_base[OLD].std_grp, uom_base[NEW].std_grp) &&
	    strcmp(uom_base[OLD].std_grp, uom_base[NEW].alt_grp) &&
	    strcmp(uom_base[OLD].alt_grp, uom_base[NEW].std_grp) &&
	    strcmp(uom_base[OLD].alt_grp, uom_base[NEW].alt_grp))
	{
		sprintf(err_str,
			ML(mlBmMess011),
			uom_base[OLD].std_uom,
			uom_base[NEW].std_uom);

		do
		{
			rv_pr(err_str, 2, 23, 0);
			conv_fact = getfloat(60, 23, "NNNNNNN.NNNNNN");
			if (conv_fact <= 0.00)
			{
				print_mess(ML(mlBmMess007));
				sleep(2);
				clear_mess();
			}

		} while (conv_fact <= 0.00);
	}
	else
	{
		conv_fact = 0.00;
		/*-----------------------------------
		| Old Std and New Std in same group |
		-----------------------------------*/
		if (!strcmp(uom_base[OLD].std_grp, uom_base[NEW].std_grp))
		{
			conv_fact = uom_base[OLD].std_cnv_fct /
					uom_base[NEW].std_cnv_fct;
		}

		/*-----------------------------------
		| Old Std and New Alt in same group |
		-----------------------------------*/
		if (!strcmp(uom_base[OLD].std_grp, uom_base[NEW].alt_grp) &&
		    conv_fact == 0.00)
		{
			conv_fact = (uom_base[OLD].std_cnv_fct /
					uom_base[NEW].alt_cnv_fct) *
					new_alt_cfactor;
		}

		/*-----------------------------------
		| Old Alt and New Std in same group |
		-----------------------------------*/
		if (!strcmp(uom_base[OLD].alt_grp, uom_base[NEW].std_grp) &&
		    conv_fact == 0.00)
		{
			conv_fact = (uom_base[OLD].alt_cnv_fct /
					uom_base[NEW].std_cnv_fct) *
					old_alt_cfactor;
		}

		/*-----------------------------------
		| Old Alt and New Alt in same group |
		-----------------------------------*/
		if (!strcmp(uom_base[OLD].alt_grp, uom_base[NEW].alt_grp) &&
		    conv_fact == 0.00)
		{
			conv_fact = (uom_base[OLD].alt_cnv_fct /
					uom_base[NEW].alt_cnv_fct) *
					old_alt_cfactor *
					new_alt_cfactor;
		}
	}

	no_in_tab = 0;
	/*-----------------------
	| Load conversion table |
	-----------------------*/
	cc = find_hash(inum, &inum_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		if (strcmp(inum_rec.um_uom_group, uom_base[OLD].std_grp) &&
		    strcmp(inum_rec.um_uom_group, uom_base[OLD].alt_grp))
		{
			cc = find_hash(inum, &inum_rec, NEXT, "r", 0L);
			continue;
		}

		sprintf(uom_from[no_in_tab].grp,
			"%-20.20s",
			inum_rec.um_uom_group);
		sprintf(uom_from[no_in_tab].uom, "%-4.4s", inum_rec.um_uom);
		uom_from[no_in_tab].hash = inum_rec.um_hhum_hash;
		uom_from[no_in_tab].cnv_fct = inum_rec.um_cnv_fct / uom_base[OLD].std_cnv_fct;

		no_in_tab++;

		cc = find_hash(inum, &inum_rec, NEXT, "r", 0L);
	}

	return(TRUE);
}

void
show_new (
 char*              key_val)
{
	int	rc;

	work_open();
	save_rec("#New Product     ","#Product Description");
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",key_val);
	cc = find_rec(inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp(inmr_rec.mr_co_no,comm_rec.tco_no) && !strncmp(inmr_rec.mr_item_no,key_val,strlen(key_val)))
	{
		strcpy(bmms2_rec.ms_co_no,comm_rec.tco_no);
		bmms2_rec.ms_mabr_hash = inmr_rec.mr_hhbr_hash;
		bmms2_rec.ms_hhbr_hash = local_rec.old_hhbr_hash;
		rc = find_rec(bmms2,&bmms2_rec,COMPARISON,"r");

		strcpy(bmms_rec.ms_co_no,comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
		bmms_rec.ms_line_no = 0;
		cc = find_rec(bmms,&bmms_rec,GTEQ,"r");
		if ((cc || bmms_rec.ms_hhbr_hash != inmr_rec.mr_hhbr_hash) && rc)
		{
			cc = save_rec(inmr_rec.mr_item_no,inmr_rec.mr_description);
			if (cc)
				break;
		}
		cc = find_rec(inmr,&inmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf(inmr_rec.mr_item_no,"%-16.16s",temp_str);
	cc = find_rec(inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
		sys_err("Error in inmr During (DBFIND)",cc,PNAME);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*              key_val)
{
	work_open();
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no ,"%-11.11s",key_val);
	cc = find_rec(excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp(excf_rec.cf_cat_no,key_val,strlen(key_val)) && !strcmp(excf_rec.cf_co_no, comm_rec.tco_no))
	{
		cc = save_rec(excf_rec.cf_cat_no, excf_rec.cf_description);
		if (cc)
			break;

		cc = find_rec(excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no,"%-11.11s",temp_str);
	cc = find_rec(excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in excf During (DBFIND)",cc, PNAME);
}

int
heading (
 int                scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();

		rv_pr(ML(mlBmMess012),26,0,1);
		move(0,1);
		line(80);

		if (BY_ITEM)
			box(0,3,80,10);
		else
			box(0,3,80,13);

		move(1,7);
		line(79);

		if (BY_ITEM)
		{
			move(1,11);
			line(79);
		}
		else
		{
			move(1,11);
			line(79);
			move(1,14);
			line(79);
		}

		move(0,20);
		line(80);

		strcpy(err_str, ML(mlStdMess038));
		print_at(21,0, err_str , comm_rec.tco_no,comm_rec.tco_name);


		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
