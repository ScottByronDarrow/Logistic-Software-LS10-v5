/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bm_wuse_prt.c  )                                 |
|  Program Desc  : ( Product costing print.                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmms, comm, excf, inei, inmr, inum,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 12/09/91         |
|---------------------------------------------------------------------|
|  Date Modified : (09/10/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (10/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (11/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (28/01/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Ana Marie C Tario.|
|                                                                     |
|  Comments      : (09/10/91) - Converted pcwc to hhwc_hash.          |
|  (10/12/91)    : Don't print items that are not used in a bill.     |
|  (11/12/91)    : Set defaults for st/end class                      |
|  (28/01/92)    : Remove Work Center and add Alternate.              |
|  (03/02/92)    : Rename pcms to bmms and progname from pc to bm     |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (03/09/97)    : Incorporate multilingual conversion and DMY4 date. |
|                                                                     |
| $Log: wuse_prt.c,v $
| Revision 5.4  2002/07/17 09:56:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/20 23:31:37  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 08:24:53  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:25  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:30  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:46:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:36:52  gerry
| Force Revision No Start 2.0 Rel-15072000
|
| Revision 1.18  2000/06/13 05:01:50  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.17  2000/01/27 05:07:21  ramon
| Removed the preprocessor definition for CloseDB (); FinishProgram ();; FinishProgram ();.
|
| Revision 1.16  2000/01/21 10:18:20  ramon
| Added a description fields for background and overnight fields.
|
| Revision 1.15  2000/01/19 11:04:35  ramon
| Removed the CloseDB (); FinishProgram ();; FinishProgram (); in run_prog() to remove the GVsion ML error in shutdown_prog().
|
| Revision 1.14  2000/01/19 07:28:17  ramon
| Added save_rec() function call for headings for GVision compatibility.
|
| Revision 1.13  1999/11/17 06:39:02  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/11/05 05:49:35  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.11  1999/10/01 07:39:08  scott
| Updated for function standards
|
| Revision 1.10  1999/09/29 10:10:09  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/23 05:49:23  scott
| Updated from Ansi Project.
|
| Revision 1.8  1999/09/15 04:37:21  scott
| Updated from Ansi.
|
| Revision 1.7  1999/08/28 04:53:06  scott
| Updated to remove ttod ()
|
| Revision 1.6  1999/06/14 23:14:05  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: wuse_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_wuse_prt/wuse_prt.c,v 5.4 2002/07/17 09:56:56 scott Exp $";

#define	X_OFF	10
#define	Y_OFF	7

#include <ml_bm_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>
#include <DateToString.h>


#define	BY_ITEM		(local_rec.range_type[0] == 'I')

FILE	*fout;

	/*=============================
	| Material Specification File |
	=============================*/
	struct dbview bmms_list[] =
	{
		{"bmms_co_no"},
		{"bmms_hhbr_hash"},
		{"bmms_alt_no"},
		{"bmms_line_no"},
		{"bmms_cons"},
		{"bmms_mabr_hash"},
		{"bmms_uom"},
		{"bmms_matl_qty"},
		{"bmms_matl_wst_pc"},
	};

	int	bmms_no_fields = 9;

	struct
	{
		char	ms_co_no[3];
		long	ms_hhbr_hash;
		int	ms_alt_no;
		int	ms_line_no;
		char	ms_cons[2];
		long	ms_mabr_hash;
		long	ms_uom;
		float	ms_matl_qty;
		float	ms_matl_wst_pc;
	} bmms_rec;

	/*=====================================
	| File comm	{System Common file}. |
	=====================================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 7;

	struct
	{
		int	term;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		long	tinv_date;
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

	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list[] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_std_batch"},
		{"inei_min_batch"},
		{"inei_max_batch"},
	};

	int	inei_no_fields = 5;

	struct
	{
		long	ei_hhbr_hash;
		char	ei_est_no[3];
		float	ei_std_batch;
		float	ei_min_batch;
		float	ei_max_batch;
	} inei_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list[] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_class"},
		{"inmr_category"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_description"},
		{"inmr_description2"},
		{"inmr_quick_code"},
		{"inmr_costing_flag"},
		{"inmr_source"},
		{"inmr_std_uom"},
		{"inmr_alt_uom"},
		{"inmr_uom_cfactor"},
		{"inmr_scrap_pc"},
		{"inmr_stat_flag"},
	};

	int	inmr_no_fields = 20;

	struct
	{
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		long	mr_hhsi_hash;
		char	mr_class[2];
		char	mr_category[12];
		char	mr_alpha_code[17];
		char	mr_super_no[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_description[41];
		char	mr_description2[41];
		char	mr_quick_code[9];
		char	mr_costing_flag[2];
		char	mr_source[3];
		long	mr_std_uom;
		long	mr_alt_uom;
		float	mr_uom_cfactor;
		float	mr_scrap_pc;
		char	mr_stat_flag[2];
	} inmr_rec;

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
		*data	= "data",
		*comm	= "comm",
		*excf	= "excf",
		*inei	= "inei",
		*inmr	= "inmr",
		*inmr2	= "inmr2",
		*inum	= "inum";

	char	tmp_std_uom[5];
	char	tmp_alt_uom[5];
	char	head_item_no[17];
	char	head_item_desc[41];
	char	curr_group[13];
	char	group_desc[81];

	int	output_open = FALSE;
	int	first_item = TRUE;
	int	first_group = TRUE;
	int	grp_head;

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct {
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
	char	range_type[2];
	int	lpno;
	char	lp_str[3];
	char	back[4];
	char	onight[4];
	char	back_desc[4];
	char	onight_desc[4];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{

	{1, LIN, "st_item",	 3, 12, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Start Item:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_item_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_item_desc},
	{1, LIN, "end_item",	 4, 12, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " End Item:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_item_desc", 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_item_desc},

	{1, LIN, "st_class",	 3, 15, CHARTYPE,
		"U", "          ",
		" ", "A", " Start Class:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_class},
	{1, LIN, "st_cat",	 4, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " Start Category:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat},
	{1, LIN, "st_cat_desc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat_desc},
	{1, LIN, "end_class",	 6, 15, CHARTYPE,
		"U", "          ",
		" ", "Z", " End Class:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_class},
	{1, LIN, "end_cat",	 7, 15, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " End Category:", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat},
	{1, LIN, "end_cat_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat_desc},

	{1, LIN, "lpno",		 6, 12, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No:", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",		 7, 12, CHARTYPE,
		"U", "          ",
		" ", "N", " Background:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.back},
	{1, LIN, "back_desc",	 7, 15, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",	 8, 12, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.onight},
	{1, LIN, "onight_desc",	 8, 15, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onight_desc},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void ReadMisc (void);
void OpenDB (void);
void CloseDB (void);
int run_prog (char *prog_name, char *prog_desc);
int heading (int scn);
int spec_valid (int field);
void SrchExcf (char *key_val);
void process (void);
void head_output (void);
void item_head (void);
void group_head (void);
void proc_bmms (long hhbr_hash);

int
main (
 int                argc,
 char*              argv[])
{

	if (argc != 3 && argc != 5)
	{
		print_at(0,0,ML(mlBmMess702), argv[0]);
		print_at(1,0,ML(mlBmMess703), argv[0]);
		print_at(2,0,ML(mlBmMess704));
		print_at(3,0,ML(mlBmMess705));
		print_at(4,0,ML(mlBmMess706));
        return (EXIT_FAILURE);
	}

	if (argc == 3)
	{
		sprintf(local_rec.range_type, "%-1.1s", argv[2]);

		OpenDB ();
    	ReadMisc ();

		SETUP_SCR (vars);

		if (!BY_ITEM)
		{
			FLD("st_item") = ND;
			FLD("st_item_desc") = ND;
			FLD("end_item") = ND;
			FLD("end_item_desc") = ND;

			vars[label("lpno")].row = 9;
			vars[label("back")].row = 10;
			vars[label("back_desc")].row = 10;
			vars[label("onight")].row = 11;
			vars[label("onight_desc")].row = 11;

			FLD("st_class") = YES;
			FLD("end_class") = YES;
			FLD("st_cat") = NO;
			FLD("st_cat_desc") = NA;
			FLD("end_cat") = NO;
			FLD("end_cat_desc") = NA;
		}

		init_scr ();
		clear ();
		set_tty ();
		set_masks ();

		while (prog_exit == 0)
		{
			/*---------------------
			| Reset Control flags |
			---------------------*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			init_ok = 1;
			search_ok = 1;

			/*---------------------
			| Entry Screen Input  |
			---------------------*/
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;

			heading (1);
			scn_display(1);
			edit (1);
			if (restart)
				continue;

			run_prog (argv[0], argv[1]);
			shutdown_prog ();
            return (EXIT_SUCCESS);
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
	{
		sprintf(local_rec.range_type, "%-1.1s", argv[1]);

		OpenDB ();
    	ReadMisc ();

		if (BY_ITEM)
		{
			sprintf(local_rec.st_item, "%-16.16s", argv[3]);
			sprintf(local_rec.end_item, "%-16.16s", argv[4]);
		}
		else
		{
			sprintf(local_rec.st_class,  "%-1.1s",   argv[3]);
			sprintf(local_rec.st_cat,    "%-11.11s", argv[3] + 1);
			sprintf(local_rec.end_class, "%-1.1s",   argv[4]);
			sprintf(local_rec.end_cat,  "%-11.11s", argv[4] + 1);
		}

		local_rec.lpno = atoi(argv[2]);

		process();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
ReadMisc (void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	strcpy (comm_rec.tco_name, clip (comm_rec.tco_name));
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (inmr2, inmr);

	open_rec (bmms,  bmms_list, bmms_no_fields, "bmms_id_no_2");
	open_rec (excf,  excf_list, excf_no_fields, "excf_id_no");
	open_rec (inei,  inei_list, inei_no_fields, "inei_id_no");
	open_rec (inmr,  inmr_list, inmr_no_fields, (BY_ITEM) ? "inmr_id_no" :
								"inmr_id_no_3");
	open_rec (inmr2, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, inum_no_fields, "inum_hhum_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (bmms);
	abc_fclose (excf);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	SearchFindClose ();
	abc_dbclose (data);
}

int
run_prog (
 char*              prog_name,
 char*              prog_desc)
{
	char	tmp_lower[17];
	char	tmp_upper[17];

	sprintf(local_rec.lp_str,"%d",local_rec.lpno);

	CloseDB (); 
	FinishProgram ();

	if (BY_ITEM)
	{
		sprintf(tmp_lower, "%-16.16s", local_rec.st_item);
		sprintf(tmp_upper, "%-16.16s", local_rec.end_item);
	}
	else
	{
		sprintf(tmp_lower,
			"%-1.1s%-11.11s",
			local_rec.st_class,
			local_rec.st_cat);

		sprintf(tmp_upper,
			"%-1.1s%-11.11s",
			local_rec.end_class,
			local_rec.end_cat);
	}

	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
            execlp ("ONIGHT",
                    "ONIGHT",
                    prog_name,
                    local_rec.range_type,
                    local_rec.lp_str,
                    tmp_lower,
                    tmp_upper,
                    prog_desc,
                    (char *)0);
        return (EXIT_FAILURE);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
            execlp (prog_name,
                    prog_name,
                    local_rec.range_type,
                    local_rec.lp_str,
                    tmp_lower,
                    tmp_upper, (char *)0);
		else
        return (EXIT_FAILURE);
	}
	else
	{
        execlp (prog_name,
                prog_name,
                local_rec.range_type,
                local_rec.lp_str,
                tmp_lower,
                tmp_upper,
                (char *)0);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int                scn)
{

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr ( ML(mlBmMess034), 23, 0, 1);

		move (0, 1);
		line (80);

		if (BY_ITEM)
		{
			box (0, 2, 80, 6);
			move (1, 5);
			line (79);
		}
		else
		{
			box (0, 2, 80, 9);
			move (1, 5);
			line (79);
			move (1, 8);
			line (79);
		}

		move (0, 20);
		line (80);
		move (0, 21);
		strcpy(err_str,ML(mlStdMess038));
		print_at (21,0,err_str, clip (comm_rec.tco_no),
			clip (comm_rec.tco_name));
		strcpy(err_str,ML(mlStdMess039));
		print_at (21,40,err_str, clip (comm_rec.test_no),
			clip (comm_rec.test_name));

		move (0, 22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		return(0);
	}

	if (LCHECK("back"))
	{
		strcpy (local_rec.back_desc, (local_rec.back[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("back_desc");
		return(0);
	}

	if (LCHECK("onight"))
	{
		strcpy (local_rec.onight_desc, (local_rec.onight[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("onight_desc");
		return(0);
	}

	if (LCHECK("st_item"))
	{
		if (FLD("st_item") == ND)
			return(0);

		if (dflt_used)
		{
			sprintf(local_rec.st_item, "%-16.16s", " ");
			sprintf(local_rec.st_item_desc, "%-40.40s", "First Item");

			DSP_FLD("st_item");
			DSP_FLD("st_item_desc");
			return(0);
		}

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
			sleep (sleepTime);
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

		if (dflt_used)
		{
			strcpy(local_rec.end_item, "~~~~~~~~~~~~~~~~");
			sprintf(local_rec.end_item_desc, "%-40.40s", "Last Item");

			DSP_FLD("end_item");
			DSP_FLD("end_item_desc");
			return(0);
		}

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
			sleep (sleepTime);
			return(1);
		}

		SuperSynonymError ();

		strcpy(local_rec.end_item_desc,inmr_rec.mr_description);
		DSP_FLD("end_item_desc");

		return(0);
	}

	if (LCHECK("st_class"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return(0);
		}

		return(0);
	}

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK("st_cat"))
	{
		if (FLD("st_cat") == ND)
			return(0);

		if (dflt_used)
		{
			sprintf(local_rec.st_cat, "%-11.11s", " ");
			sprintf(local_rec.st_cat_desc, "%-40.40s", "First Category");

			DSP_FLD("st_cat");
			DSP_FLD("st_cat_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExcf(temp_str);
			return(0);
		}

		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.st_cat);

		cc = find_rec("excf",&excf_rec,COMPARISON,"r");
		if (cc)
		{
			errmess(ML(mlStdMess004));
			return(1);
		}

		if (prog_status != ENTRY &&
		    strcmp(local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			errmess(ML(mlStdMess017));
			sleep(5);
			return(0);
		}
		sprintf(local_rec.st_cat_desc, "%-40.40s", excf_rec.cf_description);

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

		if (dflt_used)
		{
			strcpy(local_rec.end_cat, "~~~~~~~~~~~");
			sprintf(local_rec.end_cat_desc, "%-40.40s", "Last Category");

			DSP_FLD("end_cat");
			DSP_FLD("end_cat_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchExcf(temp_str);
			return(0);
		}

		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.end_cat);

		cc = find_rec("excf",&excf_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess004));
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_cat,local_rec.end_cat) > 0 )
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.end_cat_desc,excf_rec.cf_description);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*              key_val)
{
	work_open();
	save_rec ("#Category No.", "#Category Description");
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no ,"%-11.11s",key_val);
	cc = find_rec("excf", &excf_rec, GTEQ, "r");
	while (!cc && !strncmp(excf_rec.cf_cat_no,key_val,strlen(key_val)) && 
				  !strcmp(excf_rec.cf_co_no, comm_rec.tco_no))
	{
		cc = save_rec(excf_rec.cf_cat_no, excf_rec.cf_description);
		if (cc)
			break;

		cc = find_rec("excf", &excf_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no,"%-11.11s",temp_str);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in excf During (DBFIND)",cc, PNAME);
}

void
process (void)
{
	char	tmp_group[13];

	first_item = TRUE;

	if (BY_ITEM)
	{
		sprintf(inmr_rec.mr_item_no, "%-16.16s", local_rec.st_item);
		dsp_screen("Reading Bills By Item",
			comm_rec.tco_no,
			comm_rec.tco_name);
	}
	else
	{
		sprintf(inmr_rec.mr_class, "%-1.1s", local_rec.st_class);
		sprintf(inmr_rec.mr_category, "%-11.11s", local_rec.st_cat);
		dsp_screen("Reading Bills By Group",
			comm_rec.tco_no,
			comm_rec.tco_name);
	}

	strcpy(inmr_rec.mr_co_no, comm_rec.tco_no);
	cc = find_rec("inmr", &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.mr_co_no, comm_rec.tco_no) &&
		((BY_ITEM &&
		strcmp(inmr_rec.mr_item_no,local_rec.end_item) <= 0) ||
		(!BY_ITEM &&
		strcmp(inmr_rec.mr_class,local_rec.end_class) <= 0)))
	{
		if (!strcmp(inmr_rec.mr_class, local_rec.end_class) &&
		    strcmp(inmr_rec.mr_category, local_rec.end_cat) > 0)
			break;

		if (first_item)
			head_output();

		dsp_process("Item :", inmr_rec.mr_item_no);

		if (!BY_ITEM)
		{
			sprintf(tmp_group,
				"%-1.1s%-11.11s",
				inmr_rec.mr_class,
				inmr_rec.mr_category);

			if (first_group || strcmp(curr_group, tmp_group))
			{
				strcpy(curr_group, tmp_group);
				group_head();
				grp_head = TRUE;
			}
		}

		item_head();

		proc_bmms (inmr_rec.mr_hhbr_hash);

		first_item = FALSE;
		cc = find_rec("inmr", &inmr_rec, NEXT, "r");
	}

	if (output_open)
	{
		fprintf(fout, ".EOF\n");
		fclose(fout);
	}
}

void
head_output (void)
{
	/*----------------------
	| Open pipe to pformat |
	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)", errno,PNAME);

	output_open = TRUE;

	/*---------------------------------
	| Initialize printer for output.  |
	---------------------------------*/
	fprintf(fout,".START%s <%s>\n",DateToString (TodaysDate()),PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".11\n");
	fprintf(fout,".PI10\n");
	fprintf(fout,".L132\n");
	fprintf(fout,".EWHERE USED REPORT\n");
	fprintf(fout,".E%s - %s\n",comm_rec.tco_no, clip(comm_rec.tco_name));
	fprintf(fout,".B1\n");
	if (BY_ITEM)
		fprintf(fout,
			".CItem Range From : %-16.16s To %-16.16s\n",
			local_rec.st_item,
			local_rec.end_item);
	else
		fprintf(fout,
			".CGroup Range From : %-1.1s %-11.11s To %-1.1s %-11.11s\n",
			local_rec.st_class,
			local_rec.st_cat,
			local_rec.end_class,
			local_rec.end_cat);

	fprintf(fout,".B1\n");

	fprintf(fout,"==========================================");
	fprintf(fout,"==========================================");
	fprintf(fout,"======================================\n");

	fprintf(fout,"| ITEM NUMBER      |  MATERIAL NUMBER  | ALT NO ");
	fprintf(fout,"|  DESCRIPTION                             ");
	fprintf(fout,"| %%WASTE | QTY REQUIRED | UOM |\n");

	fprintf(fout,"|------------------+-------------------+--------");
	fprintf(fout,"+------------------------------------------");
	fprintf(fout,"+--------+--------------+-----|\n");

	fprintf(fout,".R=========================================");
	fprintf(fout,"===========================================");
	fprintf(fout,"======================================\n");
}

void
item_head (void)
{
	cc = find_hash("inum", &inum_rec, COMPARISON, "r", inmr_rec.mr_std_uom);
	if (cc)
		sprintf(tmp_std_uom, "%-4.4s", " ");
	else
		sprintf(tmp_std_uom, "%-4.4s", inum_rec.um_uom);

	cc = find_hash("inum", &inum_rec, COMPARISON, "r", inmr_rec.mr_alt_uom);
	if (cc)
		sprintf(tmp_alt_uom, "%-4.4s", " ");
	else
		sprintf(tmp_alt_uom, "%-4.4s", inum_rec.um_uom);

	inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
	strcpy(inei_rec.ei_est_no, comm_rec.test_no);
	cc = find_rec("inei", &inei_rec, COMPARISON, "r");
	if (cc)
	{
		inei_rec.ei_std_batch = 0.00;
		inei_rec.ei_min_batch = 0.00;
		inei_rec.ei_max_batch = 0.00;
	}

	sprintf(head_item_no, "%-16.16s", inmr_rec.mr_item_no);
	sprintf(head_item_desc, "%-40.40s", inmr_rec.mr_description);

}

void
group_head (void)
{
	strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no, "%-11.11s", curr_group + 1);
	cc = find_rec("excf", &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf(excf_rec.cf_description, "%-40.40s","Unknown Category");

	expand(group_desc, excf_rec.cf_description);
}

/*================================================================
| Reads all orders on file that have the selected part on order  |
================================================================*/
void
proc_bmms (
 long               hhbr_hash)
{
	int	first_line;
	int	found_any = FALSE;
	char	print_str[200];

	first_line = TRUE;

	strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
	bmms_rec.ms_mabr_hash = hhbr_hash;
	bmms_rec.ms_hhbr_hash = 0L;

	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while ( !cc &&
		!strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		bmms_rec.ms_mabr_hash == hhbr_hash )
	{
		found_any = TRUE;
		cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r",
				bmms_rec.ms_hhbr_hash);
		if (!cc)
		{
			if (grp_head)
			{
				fprintf(fout,
					".PD| GROUP:   %-1.1s    %-11.11s    %-80.80s             |\n",
					curr_group,
					curr_group + 1,
					group_desc);

				if (!first_group)
					fprintf(fout, ".PA\n");

				first_group = FALSE;
				grp_head = FALSE;
			}

			if (first_line)
			{
				fprintf(fout, ".LRP3\n");
				fprintf(fout,
					"| %-16.16s | Std %-4.4s Alt %-4.4s |        | %-40.40s |%-29.29s|\n",
					head_item_no,
					tmp_std_uom,
					tmp_alt_uom,
					head_item_desc,
					" ");
			}

			cc = find_hash (inum, &inum_rec, EQUAL, "r",
					bmms_rec.ms_uom);
			if (cc)
				strcpy (inum_rec.um_uom, "????");

			sprintf (print_str, "|                  | %-16.16s  | %5d  | %-40.40s |  %4.1f  |  %11.4f | %-4.4s|\n",
				inmr_rec.mr_item_no,
				bmms_rec.ms_alt_no,
				inmr_rec.mr_description,
				bmms_rec.ms_matl_wst_pc,
				bmms_rec.ms_matl_qty,
				inum_rec.um_uom);
			fprintf(fout, print_str);

			first_line = FALSE;
		}
		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	/*-------------------------
	| Clean-up before we exit |
	-------------------------*/
	if (found_any)
	{
		fprintf(fout,"|------------------+-------------------+-------");
		fprintf(fout,"-+------------------------------------------");
		fprintf(fout,"+-----------------------------|\n");
	}

	fflush (stdout);
}
