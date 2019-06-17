/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bm_muse_dsp.c  )                                 |
|  Program Desc  : ( Product material usage display.              )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmin, bmms, comm, inei, inmr, inum,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
|  Date Modified : (25/03/88)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (19/07/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (10/09/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (11/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (28/01/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (08/04/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (04/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/03/94)      | Modified  by : Roel Michels      |
|  Date Modified : (04/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (22/09/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Marnie I Organo.  |
|                                                                     |
|  Comments      : (19/07/91) - Chgs for quick code & genl tidy-up.   |
|  (10/09/91)    : Changes as reqd for DPL's BOM                      |
|  (11/12/91)    : Fix layout of std/alt measure                      |
|  (28/01/92)    : Removed work centers and added alternates.         |
|  (03/02/92)    : Rename pcms to bmms and progname from pc to bm.    |
|  (08/04/92)    : Fix problems to fix handling of alternate BOM's.   |
|  (04/08/92)    : BOM Instructions. S/C KIL 7480                     |
|  (30/03/94)    : PSL 10673                                          |
|  (04/10/94)    : PSL 11299 - calc lower levels for the req qty      |
|                : (MFG cutover)                                      |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to SEL_PSIZE   |
|  (22/09/96)    : SEL - Updated to fix looping on alternates.        |
|  (03/09/97)    : Modified for Multilingual Conversion.              |
|                                                                     |
| $Log: muse_dsp.c,v $
| Revision 5.3  2002/10/15 09:05:36  scott
| Updated as quantity was wrong after 2nd level.
|
| Revision 5.2  2001/08/09 08:24:36  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:46:14  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:46  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:36:48  gerry
| Force Revision No Start 2.0 Rel-15072000
|
| Revision 1.19  2000/06/13 05:01:46  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.18  2000/04/12 11:08:51  ramon
| For GVision sake, I changed some of the parameters of Dsp_saverec() calls.
|
| Revision 1.17  2000/01/28 08:44:39  scott
| Updated as [NEXT] [PREV].... line not standard.
|
| Revision 1.16  2000/01/28 08:42:24  scott
| Updated as index on inmr not being reset if default bom not on file.
|
| Revision 1.15  1999/12/09 02:34:03  cam
| Changes for GVision compatibility.  Added calls to heading () and scn_display ()
| before calling display ().
|
| Revision 1.14  1999/11/17 06:39:00  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.13  1999/11/05 05:49:32  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.12  1999/09/29 10:10:06  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/23 05:49:20  scott
| Updated from Ansi Project.
|
| Revision 1.10  1999/09/15 04:37:18  scott
| Updated from Ansi.
|
| Revision 1.9  1999/06/14 23:14:04  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: muse_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_muse_dsp/muse_dsp.c,v 5.3 2002/10/15 09:05:36 scott Exp $";

#define	X_OFF	0
#define	Y_OFF	7
#define	DETAILED		 (run_type[0] == 'D' || run_type[0] == 'd')
#define	SUMMARY			 (run_type[0] == 'S' || run_type[0] == 's')

#include <pslscr.h>
#include <get_lpno.h>
#include <number.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>

#define	SEL_PSIZE	10

	/*===============================
	| BoM Material INstruction File |
	===============================*/
	struct dbview bmin_list[] =
	{
		{"bmin_co_no"}, 
		{"bmin_hhbr_hash"}, 
		{"bmin_alt_no"}, 
		{"bmin_line_no"}, 
		{"bmin_tline"}, 
		{"bmin_text"}, 
	};

	int	bmin_no_fields = 6;

	struct
	{
		char	in_co_no[3];
		long	in_hhbr_hash;
		int		in_alt_no;
		int		in_line_no;
		int		in_tline;
		char	in_text[41];
	} bmin_rec;

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
		{"bmms_instr_no"}, 
	};

	int	bmms_no_fields = 10;

	struct
	{
		char	ms_co_no[3];
		long	ms_hhbr_hash;
		int		ms_alt_no;
		int		ms_line_no;
		char	ms_cons[2];
		long	ms_mabr_hash;
		long	ms_uom;
		float	ms_matl_qty;
		float	ms_matl_wst_pc;
		int		ms_instr_no;
	} bmms2_rec, bmms_rec;

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
		int		term;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		long	tinv_date;
	} comm_rec;

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
		{"inei_prd_multiple"}, 
		{"inei_dflt_bom"}, 
	};

	int	inei_no_fields = 7;

	struct
	{
		long	ei_hhbr_hash;
		char	ei_est_no[3];
		float	ei_std_batch;
		float	ei_min_batch;
		float	ei_max_batch;
		float	ei_prd_multiple;
		int		ei_dflt_bom;
	} inei_rec, inei2_rec;

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
		{"inmr_description"}, 
		{"inmr_description2"}, 
		{"inmr_quick_code"}, 
		{"inmr_costing_flag"}, 
		{"inmr_source"}, 
		{"inmr_std_uom"}, 
		{"inmr_alt_uom"}, 
		{"inmr_uom_cfactor"}, 
		{"inmr_stat_flag"}, 
		{"inmr_dflt_bom"}, 
	};

	int	inmr_no_fields = 18;

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
		char	mr_description[41];
		char	mr_description2[41];
		char	mr_quick_code[9];
		char	mr_costing_flag[2];
		char	mr_source[3];
		long	mr_std_uom;
		long	mr_alt_uom;
		float	mr_uom_cfactor;
		char	mr_stat_flag[2];
		int		mr_dflt_bom;
	} inmr_rec;

	/*================================
	| Inventory Unit of Measure File |
	================================*/
	struct dbview inum_list[] =
	{
		{"inum_hhum_hash"}, 
		{"inum_uom"}, 
		{"inum_uom_group"}, 
		{"inum_cnv_fct"}, 
	};

	int	inum_no_fields = 4;

	struct
	{
		long	um_hhum_hash;
		char	um_uom[5];
		char	um_uom_group [21];
		float	um_cnv_fct;
	} inum_rec;

	char	*dots = "...................................................";

	char	*bmin	= "bmin", 
			*bmms	= "bmms", 
			*bmms2	= "bmms2", 
			*data	= "data", 
			*comm	= "comm", 
			*inei	= "inei", 
			*inmr	= "inmr", 
			*inum	= "inum";

	char	env_line[200];

	int		lpno = 1;
	int		level_count;

	char	run_type[2];

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct
{
	char	item_no[17];
	int		alt_no;
	char	strength[6];
	char	desc1[36];
	char	desc2[41];
	char	std_uom[5];
	char	alt_uom[5];
	char	mtl_uom[5];
	char	dummy[11];
	float	qty_req;
	long	hhbrHash;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "item_no", 	 2, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "alt_no", 	 2, 55, INTTYPE, 
		"NNNNN", "          ", 
		" ", "1", "Alternate No", " ", 
		YES, NO,  JUSTRIGHT, "1", "32767",  (char *)&local_rec.alt_no}, 
	{1, LIN, "strength", 	 3, 18, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Strength", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength}, 
	{1, LIN, "desc1", 	 4, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc1}, 
	{1, LIN, "desc2", 	 5, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2}, 
	{1, LIN, "std_uom", 	 3, 91, CHARTYPE, 
		"AAAA", "          ", 
		"", "", "Standard", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_uom}, 
	{1, LIN, "alt_uom", 	 3, 110, CHARTYPE, 
		"AAAA", "          ", 
		"", "", "Alternate ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom}, 
	{1, LIN, "qty_req", 	 6, 18, FLOATTYPE, 
		"NNNNNNN.NNNNNN", "          ", 
		" ", "1", "Qty Reqd.", " ", 
		YES, NO,  JUSTLEFT, "", "",  (char *) &local_rec.qty_req}, 
	{0, LIN, "", 		 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
int spec_valid (int field);
void alt_search (char *key_val);
void display (void);
int proc_bmms (long hhbr_hash);
void proc_bmin (void);
void proc_levels (long hhbr_hash, float _cnv_fct);
float GetUom (long _hhum_hash);
int heading (int scn);

int
main (
 int                argc,
 char*              argv[])
{
	if (argc < 2)
	{
		print_at (0,0,ML(mlBmMess701),argv[0]);
        return (EXIT_FAILURE);
	}
	sprintf (run_type, "%-1.1s", argv[1]);

	if (!DETAILED && !SUMMARY)
	{
		print_at (0,0,ML(mlBmMess701),argv[0]);
        return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);

	init_scr ();
	clear ();
	set_tty ();
	set_masks ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

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
		scn_display (1);
		display ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (bmms2, bmms);

	open_rec (bmin,  bmin_list, bmin_no_fields, "bmin_id_no");
	open_rec (bmms,  bmms_list, bmms_no_fields, "bmms_id_no");
	open_rec (bmms2, bmms_list, bmms_no_fields, "bmms_id_no");
	open_rec (inei,  inei_list, inei_no_fields, "inei_id_no");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (inum,  inum_list, inum_no_fields, "inum_hhum_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (bmin);
	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	SearchFindClose ();

	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
 	float	tmp_qty;
	long	mltpl;

	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
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
			print_mess (ML(mlStdMess001));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.mr_item_no);
		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_std_uom);
		strcpy (local_rec.std_uom,  (cc) ? "    " : inum_rec.um_uom);
		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_alt_uom);
		strcpy (local_rec.alt_uom,  (cc) ? "    " : inum_rec.um_uom);

		local_rec.hhbrHash = inmr_rec.mr_hhbr_hash;
		sprintf (local_rec.item_no, "%-16.16s", inmr_rec.mr_item_no);
		
		sprintf (local_rec.desc1, "%-35.35s", inmr_rec.mr_description);
		sprintf (local_rec.strength, "%-5.5s", inmr_rec.mr_description + 35);
		sprintf (local_rec.desc2, "%-40.40s", inmr_rec.mr_description2);
		DSP_FLD ("item_no");
		DSP_FLD ("strength");
		DSP_FLD ("desc1");
		DSP_FLD ("desc2");
		DSP_FLD ("std_uom");
		DSP_FLD ("alt_uom");

		strcpy (inei_rec.ei_est_no, comm_rec.test_no);
		inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
		{
			inei_rec.ei_std_batch = 1;
			inei_rec.ei_min_batch = 1;
			inei_rec.ei_max_batch = 1;
			inei_rec.ei_prd_multiple = 0;
		}

		print_at (6,  69, ML(mlBmMess028), inei_rec.ei_std_batch);
		print_at (6,  90, ML(mlBmMess029), inei_rec.ei_min_batch);
		print_at (6, 111, ML(mlBmMess030), inei_rec.ei_max_batch);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_no"))
	{
		if (SRCH_KEY)
		{
			alt_search (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/* read branch record for default bom no */
			strcpy (inei_rec.ei_est_no, comm_rec.test_no);
			inei_rec.ei_hhbr_hash = local_rec.hhbrHash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			/* if branch default is 0, read company default bom no */
			if (cc ||
				inei_rec.ei_dflt_bom <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				if ((cc = find_hash (inmr, &inmr_rec, EQUAL, 
					"r", local_rec.hhbrHash)))
					file_err (cc, inmr, "DBFIND");
				if (inmr_rec.mr_dflt_bom <= 0)
				{
					print_mess (ML(mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					abc_selfield (inmr, "inmr_id_no");
					return (EXIT_FAILURE);
				}
				local_rec.alt_no = inmr_rec.mr_dflt_bom; 
				DSP_FLD ("alt_no");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.alt_no = inei_rec.ei_dflt_bom;
				DSP_FLD ("alt_no");
			}
		}

		strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.alt_no;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
		if (!cc && 
		    !strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		    bmms_rec.ms_hhbr_hash == inmr_rec.mr_hhbr_hash &&
		    bmms_rec.ms_alt_no == local_rec.alt_no)
			return (EXIT_SUCCESS);

		print_mess (ML(mlStdMess002));
		sleep (sleepTime);
		clear_mess ();

		return (EXIT_FAILURE);
	}

	if (LCHECK ("qty_req"))
	{
		if (local_rec.qty_req <= 0.00)
		{
			print_mess (ML(mlBmMess021));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}

		if (inei_rec.ei_prd_multiple != 0.00)
		{
			tmp_qty = local_rec.qty_req - inei_rec.ei_std_batch;
			mltpl = tmp_qty / inei_rec.ei_prd_multiple;
			tmp_qty -= (mltpl * inei_rec.ei_prd_multiple);
			if (tmp_qty > 0L)
				local_rec.qty_req = inei_rec.ei_std_batch +
						inei_rec.ei_prd_multiple +
						mltpl * inei_rec.ei_prd_multiple;
			if (tmp_qty < 0L)
				local_rec.qty_req = inei_rec.ei_std_batch +
						mltpl * inei_rec.ei_prd_multiple;
			DSP_FLD ("qty_req");
		}

		if (local_rec.qty_req < inei_rec.ei_min_batch)
		{
			print_mess (ML(mlBmMess022));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}

		if (local_rec.qty_req > inei_rec.ei_max_batch && inei_rec.ei_max_batch != 0.00)
		{
			print_mess (ML(mlBmMess023));
			sleep(2);
			clear_mess();
			return (EXIT_FAILURE);
		}

		clear_mess ();
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
alt_search (
 char*              key_val)
{
	char	alt_str[6];
	int	curr_alt;

	work_open ();
	save_rec ("#Alt.", "#");

	curr_alt = 0;
	strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
	bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
	bmms_rec.ms_alt_no = 1;
	bmms_rec.ms_line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while  (!cc &&
		!strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		bmms_rec.ms_hhbr_hash == inmr_rec.mr_hhbr_hash)
	{
		sprintf (alt_str, "%5d", bmms_rec.ms_alt_no);

		if (curr_alt == bmms_rec.ms_alt_no)
		{
			cc = find_rec (bmms, &bmms_rec, NEXT, "r");
			continue;
		}

		curr_alt = bmms_rec.ms_alt_no;
		cc = save_rec (alt_str, "");
		if (cc)
			break;
		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
	bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
	bmms_rec.ms_alt_no = atoi (temp_str);
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
}

void
display (void)
{
	char	WorkStr[81];
	if (DETAILED)
		strcpy (WorkStr, ML("MATERIAL USAGE REPORT (DETAILED) FOR PRODUCT"));
	else
		strcpy (WorkStr, ML("MATERIAL USAGE REPORT (SUMMARY) FOR PRODUCT"));

	sprintf (err_str, "%s %-16.16s", WorkStr, local_rec.item_no);
	
	Dsp_prn_open (0, 7, SEL_PSIZE, err_str,  (char *) 0,  (char *) 0,  (char *) 0,  (char *) 0,  (char *) 0,  (char *) 0);

	Dsp_saverec ("          L E V E L          |    MATERIAL    |         D E S C R I P T I O N         |Sr|UOM |   QUANTITY   |%WST|    TOTAL      ");
	Dsp_saverec ("           C O D E           |                |                                       |  |    |     PER      |    | QTY REQUIRED  ");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");

	proc_bmms (inmr_rec.mr_hhbr_hash);
	Dsp_srch ();
	Dsp_close ();
}

/*================================================================
| Reads all orders on file that have the selected part on order  |
| Returns: 0 if ok, -1 if aborted .                              |
================================================================*/
int
proc_bmms (
 long               hhbr_hash)
{ 
	float	cnv_fct;
	int		found = FALSE;
	double	tot_qty = 0.00;

	abc_selfield (inmr, "inmr_hhbr_hash");
	strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
	bmms_rec.ms_hhbr_hash = hhbr_hash;
	bmms_rec.ms_alt_no = local_rec.alt_no;
	bmms_rec.ms_line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while  (!cc && 
		bmms_rec.ms_hhbr_hash == hhbr_hash &&
		bmms_rec.ms_alt_no == local_rec.alt_no)
	{
	    cc = find_hash (inmr, &inmr_rec, COMPARISON, "r", bmms_rec.ms_mabr_hash);
	    if (cc)
	    {
			cc = find_rec (bmms, &bmms_rec, NEXT, "r");
			continue;
	    }
	    cc = find_hash (inum, &inum_rec, EQUAL, "r", bmms_rec.ms_uom);
	    strcpy (local_rec.mtl_uom,  (cc) ? "    " : inum_rec.um_uom);

		tot_qty = (double) bmms_rec.ms_matl_wst_pc;
		tot_qty += 100.00;
		tot_qty /= 100.00;
		tot_qty *= (double) bmms_rec.ms_matl_qty;
		tot_qty /= (double) inei_rec.ei_std_batch;
		/*
		tot_qty *= ( (double) local_rec.qty_req /
				 (double) inei_rec.ei_std_batch);
		 */

	    sprintf (env_line, ".1%-26.26s ^E%-16.16s^E%-39.39s^E%-2.2s^E%-4.4s^E%14.6f^E%4.1f^E%14.6f", 
		" ", 
		inmr_rec.mr_item_no, 
		inmr_rec.mr_description, 
		inmr_rec.mr_source, 
		local_rec.mtl_uom, 
		bmms_rec.ms_matl_qty, 
		bmms_rec.ms_matl_wst_pc, 
		tot_qty * (double) local_rec.qty_req);
	    Dsp_saverec (env_line);

	    proc_bmin ();

	    if (DETAILED)
	    {
			level_count = 2;
			strcpy (inei2_rec.ei_est_no, comm_rec.test_no);
			inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
			if (find_rec (inei, &inei2_rec, EQUAL, "r"))
				inei2_rec.ei_std_batch = 1.00;

			cnv_fct = GetUom (bmms_rec.ms_uom);
			cnv_fct = (float) tot_qty / cnv_fct;
			cnv_fct /= inei2_rec.ei_std_batch;
			proc_levels (bmms_rec.ms_mabr_hash, cnv_fct);
	    }

	    cc = find_rec (bmms, &bmms_rec, NEXT, "r");

	    found = TRUE;
	}

	if (found)
	    Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^****** E N D  O F  R E P O R T ******^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	else
	    Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^***   NO SPECIFICATION FOR ITEM    **^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
	fflush (stdout);

	abc_selfield (inmr, "inmr_id_no");
	return (EXIT_SUCCESS);
}

void
proc_bmin (void)
{
	strcpy (bmin_rec.in_co_no, bmms_rec.ms_co_no);
	bmin_rec.in_hhbr_hash = bmms_rec.ms_hhbr_hash;
	bmin_rec.in_alt_no = bmms_rec.ms_alt_no;
	bmin_rec.in_line_no = bmms_rec.ms_line_no;
	bmin_rec.in_tline = 0;
	cc = find_rec (bmin, &bmin_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (bmin_rec.in_co_no, bmms_rec.ms_co_no) &&
		bmin_rec.in_hhbr_hash == bmms_rec.ms_hhbr_hash &&
		bmin_rec.in_alt_no == bmms_rec.ms_alt_no &&
		bmin_rec.in_line_no == bmms_rec.ms_line_no
	)
	{
		sprintf (env_line, "%-29.29s^E%-16.16s^E%-39.39s^E%-2.2s^E%-4.4s^E%-14.14s^E%-4.4s^E%-14.14s", 
			" ", 
			" ", 
			bmin_rec.in_text, 
			" ", 
			" ", 
			" ", 
			" ", 
			" ");
		Dsp_saverec (env_line);
		cc = find_rec (bmin, &bmin_rec, NEXT, "r");
	}
}

void
proc_levels (
 long               hhbr_hash,
 float              _cnv_fct)
{
	float	cnv_fct;
	int		left_pos;
	int		save_line;
	int		save_alt;
	double	tot_qty = 0.00;

	strcpy (bmms2_rec.ms_co_no, comm_rec.tco_no);
	bmms2_rec.ms_hhbr_hash = hhbr_hash;
	bmms2_rec.ms_alt_no = local_rec.alt_no;
	bmms2_rec.ms_line_no = 0;
	cc = find_rec (bmms2, &bmms2_rec, GTEQ, "r");
	while (!cc && bmms2_rec.ms_hhbr_hash == hhbr_hash 
			   && bmms2_rec.ms_alt_no == local_rec.alt_no)
	{
	    save_line 	= bmms2_rec.ms_line_no;
	    save_alt 	= bmms2_rec.ms_alt_no;
		inmr_rec.mr_hhbr_hash = bmms2_rec.ms_mabr_hash;
	    cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	    if (cc)
	    {
			cc = find_rec (bmms2, &bmms2_rec, NEXT, "r");
			continue;
	    }

		inum_rec.um_hhum_hash = bmms2_rec.ms_uom;
	    cc = find_rec (inum, &inum_rec, EQUAL, "r");
	    strcpy (local_rec.mtl_uom,  (cc) ? "    " : inum_rec.um_uom);

		tot_qty = (double) bmms2_rec.ms_matl_wst_pc;
		tot_qty += 100.00;
		tot_qty /= 100.00;
		tot_qty *= (double) bmms2_rec.ms_matl_qty;

	    left_pos  = (level_count > 25) ? 26 : level_count;
	    sprintf (err_str, "%-*.*s%d", left_pos, left_pos, dots, level_count);
	    sprintf (env_line, "%-28.28s ^E%-16.16s^E%-39.39s^E%-2.2s^E%-4.4s^E%14.6f^E%4.1f^E%14.6f", 
				err_str, 
				inmr_rec.mr_item_no, 
				inmr_rec.mr_description, 
				inmr_rec.mr_source, 
				local_rec.mtl_uom, 
				bmms2_rec.ms_matl_qty, 
				bmms2_rec.ms_matl_wst_pc, 
				(tot_qty * _cnv_fct * local_rec.qty_req));

		Dsp_saverec (env_line);

	    level_count++;

		strcpy (inei2_rec.ei_est_no, comm_rec.test_no);
		inei_rec.ei_hhbr_hash = inmr_rec.mr_hhbr_hash;
		if (find_rec (inei, &inei2_rec, EQUAL, "r"))
			inei2_rec.ei_std_batch = 1.00;

		cnv_fct = GetUom (bmms2_rec.ms_uom);
		cnv_fct = (float) tot_qty / cnv_fct;
		cnv_fct /= inei2_rec.ei_std_batch;

	    proc_levels (bmms2_rec.ms_mabr_hash, cnv_fct);

	    level_count--;

	    strcpy (bmms2_rec.ms_co_no, comm_rec.tco_no);
	    bmms2_rec.ms_line_no 	= save_line;
	    bmms2_rec.ms_alt_no 	= save_alt;
	    bmms2_rec.ms_hhbr_hash = hhbr_hash;
	    cc = find_rec (bmms2, &bmms2_rec, GTEQ, "r");
	    if (cc)
			file_err (cc, bmms2, "DBFIND");

	    cc = find_rec (bmms2, &bmms2_rec, NEXT, "r");
	}
}

float
GetUom (
 long               _hhum_hash)
{
	char	std_group [21], 
			alt_group [21];
	number	std_cnv_fct, 
			alt_cnv_fct, 
			cnv_fct, 
			result, 
			uom_cfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_alt_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.um_uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.um_cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.mr_std_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.um_uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.um_cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", _hhum_hash);
	if (cc)
		file_err (cc, inum, "DBFIND");
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.um_cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.um_uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr_rec.mr_uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

int
heading (
 int                scn)
{
	swide ();

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		if (DETAILED)
			rv_pr (ML(mlBmMess024), 50, 0, 1);
		else
			rv_pr (ML(mlBmMess025), 52, 0, 1);

		box (0, 1, 132, 5);

		move (67, 1); PGCHAR (8);
		move (67, 2); PGCHAR (4);
		move (67, 3); PGCHAR (4);
		move (67, 4); PGCHAR (10); line (64); PGCHAR (11);
		move (67, 5); PGCHAR (4);
		move (67, 6); PGCHAR (4);
		move (67, 7); PGCHAR (9);

		rv_pr (ML(mlBmMess026), 88, 2, FALSE);
		rv_pr (ML(mlBmMess027), 89, 5, FALSE);

		print_at (6,  69, ML(mlBmMess028), inei_rec.ei_std_batch);
		print_at (6,  90, ML(mlBmMess029), inei_rec.ei_min_batch);
		print_at (6, 111, ML(mlBmMess030), inei_rec.ei_max_batch);

		move (0, 20);
		line (132);
		strcpy(err_str, ML(mlStdMess038));
		print_at (21, 0, err_str, clip (comm_rec.tco_no), clip (comm_rec.tco_name));
		strcpy(err_str, ML(mlStdMess039));
		print_at (21, 45, err_str,  clip (comm_rec.test_no), clip (comm_rec.test_name));
		move (0, 22);
		line (132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
