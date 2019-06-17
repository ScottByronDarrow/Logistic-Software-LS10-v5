/*====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( bm_delete.c   )                                  |
|  Program Desc  : ( Delete Product Specification.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmin, bmms, comm, inmr,                           |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  bmin, bmms,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (12/09/91)      | Author       : Campbell Mander   |
|---------------------------------------------------------------------|
|  Date Modified : (28/01/92)      | Modified  by : Campbell Mander   |
|  Date Modified : (03/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (30/03/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (04/10/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (03/09/97)      | Modified  by : Ana Marie Tario.  |
|                                                                     |
|  Comments      : (28/01/92) - Add Alternates.                       |
|  (03/08/92)    : BOM instructions S/C KIL 7480                      |
|  (30/03/94)    : DPL 10230 - set the alt number to the items default|
|  (04/10/94)    : PSL 11299 - Upgrade to ver9 - MFG cutover - no code|
|                : changes                                            |
|                                                                     |
| $Log: bm_delete.c,v $
| Revision 5.2  2001/08/09 08:24:32  scott
| Updated to add FinishProgram ();
|
| Revision 5.1  2001/08/06 22:52:19  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 07:38:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:44:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:44  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:36:42  gerry
| Force Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/06/13 05:01:45  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.9  1999/11/17 06:38:59  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.8  1999/11/05 05:49:30  scott
| Updated to fix warning messages from use of -Wall compile flags.
|
| Revision 1.7  1999/09/29 10:10:05  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/23 05:49:19  scott
| Updated from Ansi Project.
|
| Revision 1.5  1999/09/15 04:37:18  scott
| Updated from Ansi.
|
| Revision 1.4  1999/06/14 23:14:03  scott
| Update to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_delete.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_delete/bm_delete.c,v 5.2 2001/08/09 08:24:32 scott Exp $";

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
		{"bmms_matl_qty"},
		{"bmms_matl_wst_pc"},
		{"bmms_instr_no"}
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
		float	ms_matl_qty;
		float	ms_matl_wst_pc;
		int		ms_instr_no;
	} bmms_rec, bmms2_rec;

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

	int	comm_no_fields = 4;

	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
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
			*bmms	= "bmms",
			*bmms2	= "bmms2",
			*comm	= "comm",
			*data	= "data",
			*inmr	= "inmr",
			*inei	= "inei";

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy[11];
	long	hhbr_hash;
	char	del_item[17];
	int		alt_no;
	char	desc[36];
	char	desc2[41];
	char	strength[6];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "del_item",	 4, 13, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", " Item Number :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.del_item},
	{1, LIN, "alt_no",	 5, 13, INTTYPE,
		"NNNNN", "          ",
		" ", "0", " Alternate No:", " ",
		YES, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.alt_no},
	{1, LIN, "desc",	 6, 13, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "strength",	 6, 65, CHARTYPE,
		"AAAAA", "          ",
		" ", "", " Strength:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "desc2", 7, 13, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{0, LIN, "",	 0, 0, INTTYPE,
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
int delete_bill (void);
int heading (int);

extern	int	manufacturingSrch;
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	  
	SETUP_SCR (vars);

	manufacturingSrch	=	TRUE;

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
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		delete_bill();

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

	abc_alias (bmms2, bmms);

	open_rec (bmin,  bmin_list, bmin_no_fields, "bmin_id_no");
	open_rec (bmms,  bmms_list, bmms_no_fields, "bmms_id_no");
	open_rec (bmms2, bmms_list, bmms_no_fields, "bmms_mabr_hash");
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
	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (inmr);
	abc_fclose (inei);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
       
    if (LCHECK("del_item"))
    {
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		cc = FindInmr (comm_rec.tco_no, local_rec.del_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.del_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			return(1);
		}
		SuperSynonymError ();

		if (strcmp(inmr_rec.mr_source, "MC") &&
		    strcmp(inmr_rec.mr_source, "MP") &&
		    strcmp(inmr_rec.mr_source, "BM") &&
		    strcmp(inmr_rec.mr_source, "BP"))
		{
			print_mess(ML(mlBmMess008));
			sleep(2);
			clear_mess();

			return(1);
		}

		local_rec.hhbr_hash = inmr_rec.mr_hhbr_hash;

		cc = find_hash(bmms2,&bmms2_rec,COMPARISON,"r", local_rec.hhbr_hash);	
		if (!cc)
		{
			print_mess(ML(mlBmMess009));
			sleep(2);
			clear_mess();
			return(1);
		}
		
		sprintf(local_rec.desc, "%-35.35s", inmr_rec.mr_description);
		sprintf(local_rec.strength, "%-5.5s", &inmr_rec.mr_description[35]);
		strcpy(local_rec.desc2,inmr_rec.mr_description2);
		DSP_FLD("desc");
		DSP_FLD("strength");
		DSP_FLD("desc2");
        return(0);
	}

	if (LCHECK("alt_no"))
	{
		if (dflt_used)
		{
			/* read branch record for default bom no */
			strcpy (inei_rec.est_no, comm_rec.test_no);
			inei_rec.hhbr_hash = local_rec.hhbr_hash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			/* if branch default is 0, read company default bom no */
			if (cc ||
				inei_rec.dflt_bom <= 0)
			{
				abc_selfield (inmr, "inmr_hhbr_hash");
				cc = find_hash (inmr, &inmr_rec, EQUAL,"r",local_rec.hhbr_hash);
				if (cc)
					file_err (cc, inmr, "DBFIND");
				if (inmr_rec.mr_dflt_bom <= 0)
				{
					print_mess (ML(mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.alt_no = inmr_rec.mr_dflt_bom; 
				DSP_FLD ("alt_no");
				abc_selfield (inmr, "inmr_id_no");
			}
			else
			{
				local_rec.alt_no = inei_rec.dflt_bom;
				DSP_FLD ("alt_no");
			}
		}

		if (SRCH_KEY)
		{
			alt_search(temp_str);
			return(0);
		}
		strcpy(bmms_rec.ms_co_no,comm_rec.tco_no);
		bmms_rec.ms_hhbr_hash = local_rec.hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.alt_no;
		bmms_rec.ms_line_no = 0;
		cc = find_rec(bmms,&bmms_rec,GTEQ,"r");	
		if (!cc && 
		    !strcmp(bmms_rec.ms_co_no, comm_rec.tco_no) &&
		    bmms_rec.ms_hhbr_hash == local_rec.hhbr_hash &&
		    bmms_rec.ms_alt_no == local_rec.alt_no)
		{
			return(0);
		}
		print_mess(ML(mlStdMess002));
		sleep(2);
		clear_mess();
		return(1);
	}

    return(0);             
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
	while (!cc &&
	       !strcmp (bmms_rec.ms_co_no, comm_rec.tco_no) &&
	       bmms_rec.ms_hhbr_hash == inmr_rec.mr_hhbr_hash )
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

/*----------------------------------
| Delete all bmms records for hash |
----------------------------------*/
int
delete_bill (void)
{

	clear();
	print_at(0,1,ML(mlStdMess035));

	fflush(stdout);

	strcpy(bmms_rec.ms_co_no,comm_rec.tco_no);
	bmms_rec.ms_hhbr_hash = local_rec.hhbr_hash;
	bmms_rec.ms_alt_no = local_rec.alt_no;
	bmms_rec.ms_line_no = 0;

	cc = find_rec(bmms,&bmms_rec,GTEQ,"r");	
	while (!cc && 
	       !strcmp(bmms_rec.ms_co_no, comm_rec.tco_no) && 
	       bmms_rec.ms_hhbr_hash == local_rec.hhbr_hash &&
	       bmms_rec.ms_alt_no == local_rec.alt_no)
	{
		putchar('D');
		fflush(stdout);

		bmms_rec.ms_hhbr_hash = local_rec.hhbr_hash;
		cc = abc_delete(bmms);
		if (cc)
	       		file_err (cc, bmms, "DBDELETE");

		bmms_rec.ms_hhbr_hash = local_rec.hhbr_hash;
		bmms_rec.ms_alt_no = local_rec.alt_no;
		bmms_rec.ms_line_no = 0;
		cc = find_rec(bmms,&bmms_rec,GTEQ,"r");	
	}

	strcpy (bmin_rec.in_co_no, comm_rec.tco_no);
	bmin_rec.in_hhbr_hash = local_rec.hhbr_hash;
	bmin_rec.in_alt_no = local_rec.alt_no;
	bmin_rec.in_line_no = 0;

	cc = find_rec (bmin, &bmin_rec, GTEQ, "r");	
	while
	(
		!cc &&
		!strcmp (bmin_rec.in_co_no, comm_rec.tco_no) &&
		bmin_rec.in_hhbr_hash == local_rec.hhbr_hash &&
		bmin_rec.in_alt_no == local_rec.alt_no
	)
	{
		bmin_rec.in_hhbr_hash = local_rec.hhbr_hash;
		cc = abc_delete (bmin);
		if (cc)
	       		file_err (cc, bmin, "DBDELETE");

		bmin_rec.in_hhbr_hash = local_rec.hhbr_hash;
		bmin_rec.in_alt_no = local_rec.alt_no;
		bmin_rec.in_line_no = 0;
		cc = find_rec (bmin, &bmin_rec, GTEQ, "r");	
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
			scn_set(scn);
		clear();
		rv_pr(ML(mlBmMess032) ,26,0,1);

		move(0,1);
		line(80);

		box(0,3,80,4);

		move(0,20);
		line(80);
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0,err_str, comm_rec.tco_no, comm_rec.tco_name);
		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
