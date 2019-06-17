/*=====================================================================
|  Copyright (C) 1996 - 1997 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bm_wuse_dsp.c  )                                 |
|  Program Desc  : ( Product costing display.                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmms, comm, inei, inmr, inum,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (25/03/88)      | Modified  by : Scott B. Darrow.  |
|  Date Modified : (28/01/92)      | Modified  by : Campbell Mander.  |
|  Date Modified : (03/02/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/957      | Modified  by : Marnie I Organo.  |
|                                                                     |
|  Comments      : (28/01/92) - Remove work center and add alternate. |
|  (03/02/92)    : Rename pcms to bmms and progname from pc to bm     |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to SEL_SIZE|
|  (03/09/957    : Modified for Multilingual Conversion.              |
|                                                                     |
| $Log: wuse_dsp.c,v $
| Revision 5.2  2001/08/09 08:24:52  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:25  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:46:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:36:51  gerry
| Force Revision No Start 2.0 Rel-15072000
|
| Revision 1.15  2000/06/13 05:01:49  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.14  2000/03/09 00:46:56  cam
| Changes for GVision compatibility.  Fixed display of data on screen 1 before
| displaying details.
|
| Revision 1.13  2000/02/24 06:44:16  gerry
| Corrected missed conversion of display () to DisplayData ()
|
| Revision 1.12  2000/02/18 01:15:42  scott
| Updated as shutdown () not defined as shutdown (void);
|
| Revision 1.11  1999/11/17 06:39:02  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/05 05:49:35  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.9  1999/09/29 10:10:08  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/23 05:49:22  scott
| Updated from Ansi Project.
|
| Revision 1.7  1999/09/15 04:37:21  scott
| Updated from Ansi.
|
| Revision 1.6  1999/06/14 23:14:05  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: wuse_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_wuse_dsp/wuse_dsp.c,v 5.2 2001/08/09 08:24:52 scott Exp $";

#define	X_OFF	10
#define	Y_OFF	7

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_bm_mess.h>

#define	SEL_SIZE	11

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
		int		ms_alt_no;
		int		ms_line_no;
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
			*inei	= "inei",
			*inmr	= "inmr",
			*inmr2	= "inmr2",
			*inum	= "inum";

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct {
	char	item_no[17];
	char	strength[6];
	char	desc1[36];
	char	desc2[41];
	char	std_uom[5];
	char	alt_uom[5];
	char	mtl_uom[5];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "item_no",	 2, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "strength",	 3, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "desc1",	 4, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc1},
	{1, LIN, "desc2",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "std_uom",	 3, 91, CHARTYPE,
		"AAAA", "          ",
		"", "", "Standard", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{1, LIN, "alt_uom",	 3, 110, CHARTYPE,
		"AAAA", "          ",
		"", "", "  Alternate", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom},

	{0, LIN, "",		 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void	shutdown_prog 	(void);
void	ReadMisc 		(void);
void	OpenDB 			(void);
void	CloseDB 		(void);
int		heading 		(int);
int		spec_valid 		(int);
void	DisplayData 	(void);
void	ProcessBmms 	(long);

int
main (
 int                argc,
 char*              argv[])
{
	SETUP_SCR (vars);

	init_scr ();
	clear ();
	set_tty ();
	set_masks ();

	OpenDB ();
	ReadMisc ();

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

		DisplayData ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec );
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
	open_rec (inei,  inei_list, inei_no_fields, "inei_id_no");
	open_rec (inmr,  inmr_list, inmr_no_fields, "inmr_id_no");
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
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	SearchFindClose ();
	abc_dbclose (data);
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

		rv_pr (ML(mlBmMess031), 48, 0, 1);

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

		move (0, 20);
		line (132);
		strcpy(err_str, ML(mlStdMess038));
		print_at (21,0, err_str, clip (comm_rec.tco_no), clip (comm_rec.tco_name));
		strcpy(err_str, ML(mlStdMess039));
		print_at (21,45, err_str,  clip (comm_rec.test_no), clip (comm_rec.test_name));
		move (0, 22);
		line (132);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
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
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		cc = find_hash (inum,&inum_rec,EQUAL, "r", inmr_rec.mr_std_uom);
		strcpy (local_rec.std_uom, (cc) ? "    " : inum_rec.um_uom);
		cc = find_hash (inum,&inum_rec,EQUAL, "r", inmr_rec.mr_alt_uom);
		strcpy (local_rec.alt_uom, (cc) ? "    " : inum_rec.um_uom);

		sprintf (local_rec.item_no, "%-16.16s", inmr_rec.mr_item_no);
		DSP_FLD ("item_no");
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
		}

		print_at (6,  69, ML(mlBmMess028), inei_rec.ei_std_batch);
		print_at (6,  90, ML(mlBmMess029), inei_rec.ei_min_batch);
		print_at (6, 111, ML(mlBmMess030), inei_rec.ei_max_batch);
		return(0);
	}
	return(0);
}

void
DisplayData (void)
{
	sprintf (err_str, "%s %s", ML ("WHERE USED REPORT FOR PRODUCT"), local_rec.item_no);
	Dsp_prn_open (10, 7, SEL_SIZE, err_str,
		(char *) 0, (char *) 0,
		(char *) 0, (char *) 0,
		(char *) 0, (char *) 0);

	Dsp_saverec (" ITEM NUMBER      | ALT NO. | DESCRIPTION                              | %WST |  QTY REQD.  | UOM  ");

	Dsp_saverec ("");

	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCN] [PREV SCN] [INPUT/END] ");

	ProcessBmms (inmr_rec.mr_hhbr_hash);
	Dsp_srch ();
	Dsp_close ();
}

/*================================================================
| Reads all orders on file that have the selected part on order  |
================================================================*/
void
ProcessBmms (
 long               hhbr_hash)
{
	int	found_any = FALSE;
	char	disp_line[200];

	strcpy (bmms_rec.ms_co_no, comm_rec.tco_no);
	bmms_rec.ms_mabr_hash = hhbr_hash;
	bmms_rec.ms_hhbr_hash = 0L;

	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while
	(
		!cc &&
		bmms_rec.ms_mabr_hash == hhbr_hash &&
		strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) == 0
	)
	{
		found_any = TRUE;
		cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r", bmms_rec.ms_hhbr_hash);
		if (!cc)
		{
			cc = find_hash (inum, &inum_rec, EQUAL, "r", bmms_rec.ms_uom);
			if (cc)
				strcpy (inum_rec.um_uom, "????");

			sprintf (disp_line, " %-16.16s ^E  %5d  ^E %-40.40s ^E %4.1f ^E %11.4f ^E %-4.4s ",
				inmr_rec.mr_item_no,
				bmms_rec.ms_alt_no,
				inmr_rec.mr_description,
				bmms_rec.ms_matl_wst_pc,
				bmms_rec.ms_matl_qty,
				inum_rec.um_uom);
			Dsp_saverec (disp_line);
		}
		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	/*-------------------------
	| Clean-up before we exit |
	-------------------------*/
	if (found_any)
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^***** E N D   O F   R E P O R T. *****^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");
	else
		Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^*** N O   U S E   F O R   I T E M. ***^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^");
	fflush (stdout);
}
