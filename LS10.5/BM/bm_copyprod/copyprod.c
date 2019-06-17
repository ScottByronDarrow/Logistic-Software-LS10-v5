/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (bm_copyprod.c  )                                  |
|  Program Desc  : (Copy Product Specification.                   )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmin, bmms, comm, inmr,                           |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  bmin, bmms,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written : 19/06/87          |
|---------------------------------------------------------------------|
|  Date Modified : (19/06/87)      | Modified  by : Roger Gibbison.   |
|  Date Modified : (19/07/91)      | Modified  by : Trevor van Bremen |
|  Date Modified : (09/09/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (10/12/91)      | Modified  by : Campbell Mander.  |
|  Date Modified : (28/01/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (03/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (04/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (03/09/97)      | Modified  by : Marnie Organo.    |
|                                                                     |
|  Comments      : (19/07/91) - Chgs for quick code & genl tidy-up.   |
|  (09/09/91)    : Added ability to maintain new bill after it is     |
|                : copied.                                            |
|  (10/12/91)    : Copy uom and work centre details across. Fix bill  |
|                : maintenance option                                 |
|  (28/01/92)    : Remove work centers and add alternates.            |
|  (03/02/92)    : Rename pcms to bmms + genl tidy.                   |
|  (03/08/92)    : Incorporate bom instructions S/C KIL 7480          |
|  (30/03/94)    : DPL 10230 - set the alt number to the items default|
|  (04/10/94)    : PSL 11299 - Upgrade to ver9 - mfg cutover - no     |
|                : code changes.                                      |
|  (03/09/97)    : Modified for Multilingual Conversion  .            |
|                                                                     |
| $Log: copyprod.c,v $
| Revision 5.2  2001/08/09 08:24:29  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:18  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 07:34:04  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/09 06:00:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 4.0  2001/03/09 01:07:10  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:43  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:35:47  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.12  2000/06/13 05:01:39  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.11  1999/12/23 21:21:38  cam
| Changes for GVision compatibility.  Fix display, sleep, clear of error messages.
|
| Revision 1.10  1999/12/10 04:06:05  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.9  1999/11/17 06:38:59  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/05 05:49:28  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.7  1999/09/29 10:10:05  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/23 05:49:16  scott
| Updated from Ansi Project.
|
| Revision 1.5  1999/09/15 04:37:14  scott
| Updated from Ansi.
|
| Revision 1.4  1999/06/14 23:13:53  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: copyprod.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_copyprod/copyprod.c,v 5.2 2001/08/09 08:24:29 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>

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

	/*==============================
	| Material Specification File. |
	==============================*/
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
		{"bmms_iss_seq"},
	};

	int bmms_no_fields = 11;

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
		int		ms_iss_seq;
	} bmms2_rec, bmms_rec;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
	};

	int comm_no_fields = 4;

	struct
	{
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no [3];
	} comm_rec;

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
		{"inmr_source"},
		{"inmr_dflt_bom"},
	};

	int	inmr_no_fields = 15;

	struct	{
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
		char	mr_source[3];
		int		mr_dflt_bom;
	} inmr_rec;

	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list [] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_dflt_bom"},
	};

	int	inei_no_fields = 3;

	struct tag_ineiRecord
	{
		long	hhbr_hash;
		char	est_no [3];
		int		dflt_bom;
	} inei_rec;

	char	*bmin	= "bmin",
			*bmin2	= "bmin2",
			*bmms	= "bmms",
			*bmms2	= "bmms2",
			*comm	= "comm",
			*data	= "data",
			*inmr	= "inmr",
			*inei	= "inei";

		char	run_type[2];
		char	pr_desc1[26],
		pr_desc2[26];

	int	new_item = 0;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	long	old_hhbr_hash;
	char	old_prod_no[17];
	char	old_prod_desc[41];
	char	old_prod_desc2[41];
	long	new_hhbr_hash;
	char	new_prod_no[17];
	char	new_prod_desc[41];
	char	new_prod_desc2[41];
	int		old_alt_no;
	int		new_alt_no;
	int		bill_mnt;
	long	hhbr_hash;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "old_prod",	 4, 28, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", pr_desc1, " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.old_prod_no},
	{1, LIN, "old_alt_no",	 5, 28, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Alternate No:", " ",
		YES, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.old_alt_no},
	{1, LIN, "old_desc1",		 6, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.old_prod_desc},
	{1, LIN, "old_desc2",		 7, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.old_prod_desc2},
	{1, LIN, "new_prod",	 9, 28, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", pr_desc2, " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.new_prod_no},
	{1, LIN, "new_alt_no",	10, 28, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Alternate No:", " ",
		YES, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.new_alt_no},
	{1, LIN, "new_desc1",		11, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.new_prod_desc},
	{1, LIN, "new_desc2",		12, 28, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.new_prod_desc2},
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
int spec_valid (int);
void alt_search (char *);
void update (void);
void copy_instr (void);
int maintain_bill (void);
void show_new (char *);
int heading (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	sprintf (pr_desc1,"%-25.25s", "Existing Product.");
	sprintf (pr_desc2,"%-25.25s", "New Product.");

	SETUP_SCR (vars );

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	while (prog_exit == 0)
	{
		entry_exit	= 0;
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		search_ok	= 1;
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		local_rec.bill_mnt = prmptmsg (ML (mlBmMess018), "YyNn", 20, 23);
		update ();

		if (local_rec.bill_mnt == 'Y' || local_rec.bill_mnt == 'y')
        {
			prog_exit = maintain_bill ();
        }
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

	abc_alias (bmin2, bmin);
	abc_alias (bmms2, bmms);

	open_rec (bmin,  bmin_list, bmin_no_fields, "bmin_id_no");
	open_rec (bmin2, bmin_list, bmin_no_fields, "bmin_id_no");
	open_rec (bmms,  bmms_list, bmms_no_fields, "bmms_id_no");
	open_rec (bmms2, bmms_list, bmms_no_fields, "bmms_id_no_2");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (inei,  inei_list, inei_no_fields, "inei_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (bmin);
	abc_fclose (bmin2);
	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (inmr);
	abc_fclose (inei);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("old_prod"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
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
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (bmms_rec.ms_co_no,comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
		bmms_rec.ms_line_no = 0;
		cc = find_rec (bmms,&bmms_rec,GTEQ,"r");
		if (cc || strcmp (bmms_rec.ms_co_no,comm_rec.tco_no) ||
			bmms_rec.ms_hhbr_hash != inmr_rec.mr_hhbr_hash)
		{
			print_mess (ML (mlBmMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.old_hhbr_hash = inmr_rec.mr_hhbr_hash;
		strcpy (local_rec.old_prod_desc,inmr_rec.mr_description);
		strcpy (local_rec.old_prod_desc2,inmr_rec.mr_description2);
		DSP_FLD ("old_prod");
		DSP_FLD ("old_desc1");
		DSP_FLD ("old_desc2");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("old_alt_no"))
	{
		if (dflt_used)
		{
			/* read branch record for default bom no */
			strcpy (inei_rec.est_no, comm_rec.test_no);
			inei_rec.hhbr_hash = local_rec.old_hhbr_hash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			/* if branch default is 0, read company default bom no */
			if (cc ||
				inei_rec.dflt_bom <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.mr_hhbr_hash	=	local_rec.old_hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr, "DBFIND");

				if (inmr_rec.mr_dflt_bom <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.old_alt_no = inmr_rec.mr_dflt_bom; 
				DSP_FLD ("old_alt_no");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.old_alt_no = inei_rec.dflt_bom;
				DSP_FLD ("old_alt_no");
			}
		}

		if (SRCH_KEY)
		{
			local_rec.hhbr_hash = local_rec.old_hhbr_hash;
			alt_search (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = local_rec.old_hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.old_alt_no;
		bmms_rec.ms_line_no = 0;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
		if (!cc &&
		    !strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		    bmms_rec.ms_hhbr_hash == local_rec.old_hhbr_hash &&
		    bmms_rec.ms_alt_no == local_rec.old_alt_no)
			return (EXIT_SUCCESS);

		print_mess (ML (mlStdMess002));
		sleep (sleepTime);
		clear_mess ();

		return (EXIT_FAILURE);
	}

	if (LCHECK ("new_prod"))
	{
		if (SRCH_KEY)
		{
			show_new (temp_str);
			return (EXIT_SUCCESS);
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
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.new_hhbr_hash = inmr_rec.mr_hhbr_hash;

		if (strcmp (inmr_rec.mr_source, "MC") &&
		    strcmp (inmr_rec.mr_source, "MP") &&
		    strcmp (inmr_rec.mr_source, "BM") &&
		    strcmp (inmr_rec.mr_source, "BP"))
		{
			print_mess (ML (mlBmMess008));
			sleep (sleepTime);
			clear_mess ();

			return (EXIT_FAILURE);
		}

		strcpy (bmms2_rec.ms_co_no,comm_rec.tco_no);
		bmms2_rec.ms_mabr_hash = local_rec.new_hhbr_hash;
		bmms2_rec.ms_hhbr_hash = local_rec.old_hhbr_hash;
		cc = find_rec (bmms2,&bmms2_rec,COMPARISON,"r");
		if (!cc)
		{
			print_mess (ML (mlBmMess009));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.new_prod_desc,inmr_rec.mr_description);
		strcpy (local_rec.new_prod_desc2,inmr_rec.mr_description2);
		DSP_FLD ("new_prod");
		DSP_FLD ("new_desc1");
		DSP_FLD ("new_desc2");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_alt_no"))
	{
		if (dflt_used)
		{
			/* read branch record for default bom no */
			strcpy (inei_rec.est_no, comm_rec.test_no);
			inei_rec.hhbr_hash = local_rec.new_hhbr_hash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			/* if branch default is 0, read company default bom no */
			if (cc || inei_rec.dflt_bom <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				inmr_rec.mr_hhbr_hash	=	local_rec.new_hhbr_hash;
				cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr, "DBFIND");

				if (inmr_rec.mr_dflt_bom <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.new_alt_no = inmr_rec.mr_dflt_bom; 
				DSP_FLD ("new_alt_no");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.new_alt_no = inei_rec.dflt_bom;
				DSP_FLD ("new_alt_no");
			}
		}

		if (SRCH_KEY)
		{
			local_rec.hhbr_hash = local_rec.new_hhbr_hash;
			alt_search (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = local_rec.new_hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.new_alt_no;
		bmms_rec.ms_line_no = 0;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
		if (!cc &&
		    !strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		    bmms_rec.ms_hhbr_hash == local_rec.new_hhbr_hash &&
		    bmms_rec.ms_alt_no == local_rec.new_alt_no)
		{
			print_mess (ML (mlStdMess003));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.new_prod_desc,inmr_rec.mr_description);
		strcpy (local_rec.new_prod_desc2,inmr_rec.mr_description2);
		display_field (field + 1);
		display_field (field + 2);

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
	bmms_rec.ms_hhbr_hash = local_rec.hhbr_hash;
	bmms_rec.ms_alt_no = 1;
	bmms_rec.ms_line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while  (!cc &&
		!strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
		bmms_rec.ms_hhbr_hash == local_rec.hhbr_hash )
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
update (void)
{

	clear ();
	print_at (0,0, ML (mlBmMess019));

	fflush (stdout);
	strcpy (bmms_rec.ms_co_no,comm_rec.tco_no);
	bmms_rec.ms_hhbr_hash = local_rec.old_hhbr_hash;
	bmms_rec.ms_alt_no = local_rec.old_alt_no;
	bmms_rec.ms_line_no = 0;
	cc = find_rec (bmms,&bmms_rec,GTEQ,"r");
	while  (!cc &&
		!strcmp (bmms_rec.ms_co_no,comm_rec.tco_no) &&
		bmms_rec.ms_hhbr_hash == local_rec.old_hhbr_hash &&
		bmms_rec.ms_alt_no == local_rec.old_alt_no)
	{
		putchar ('A');
		fflush (stdout);

		copy_instr ();

		bmms_rec.ms_hhbr_hash = local_rec.new_hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.new_alt_no;
		cc = abc_add (bmms2,&bmms_rec);
		if (cc)
			file_err (cc, bmms2, "DBADD");
		cc = find_rec (bmms,&bmms_rec,NEXT,"r");
	}

	return;
}

void
copy_instr (void)
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
		!strcmp (bmin_rec.in_co_no, comm_rec.tco_no) &&
		bmin_rec.in_hhbr_hash == local_rec.old_hhbr_hash &&
		bmin_rec.in_alt_no == local_rec.old_alt_no &&
		bmin_rec.in_line_no == bmms_rec.ms_line_no
	)
	{
		bmin_rec.in_hhbr_hash = local_rec.new_hhbr_hash;
		bmin_rec.in_alt_no = local_rec.new_alt_no;
		abc_add (bmin2, &bmin_rec);
		cc = find_rec (bmin, &bmin_rec, NEXT, "r");
	}
}

int
maintain_bill (void)
{
	sprintf 
	(
		err_str, 
		"bm_maint %ld %d", 
		local_rec.new_hhbr_hash, 
		local_rec.new_alt_no
	);

	if (sys_exec (err_str))
		return (TRUE);

	return (FALSE);
}

void
show_new (
 char*              key_val)
{
	int	rc;

	work_open ();
	save_rec ("#New Product     ","#Product Description");
	strcpy (inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf (inmr_rec.mr_item_no,"%-16.16s",key_val);
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp (inmr_rec.mr_co_no,comm_rec.tco_no) && 
		   !strncmp (inmr_rec.mr_item_no,key_val,strlen (key_val)))
	{
		strcpy (bmms2_rec.ms_co_no,comm_rec.tco_no);
		bmms2_rec.ms_mabr_hash = inmr_rec.mr_hhbr_hash;
		bmms2_rec.ms_hhbr_hash = local_rec.old_hhbr_hash;
		rc = find_rec (bmms2,&bmms2_rec,COMPARISON,"r");

		strcpy (bmms_rec.ms_co_no,comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = inmr_rec.mr_hhbr_hash;
		bmms_rec.ms_line_no = 0;
		cc = find_rec (bmms,&bmms_rec,GTEQ,"r");
		if ((cc || bmms_rec.ms_hhbr_hash != inmr_rec.mr_hhbr_hash) && rc)
		{
			cc = save_rec (inmr_rec.mr_item_no,inmr_rec.mr_description);
			if (cc)
				break;
		}
		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (inmr_rec.mr_co_no,comm_rec.tco_no);
	sprintf (inmr_rec.mr_item_no,"%-16.16s",temp_str);
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, inmr, "DBFIND");
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
		rv_pr (ML (mlBmMess020),26,0,1);

		move (0,1);
		line (80);

		box (0,3,80,9);

		move (1,8);
		line (79);

		move (0,20);
		line (80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0, err_str,comm_rec.tco_no,comm_rec.tco_name);
		move (0,22);
		line (80);
		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
