/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _rvw_mnt.c,v 5.3 2002/07/25 11:17:29 scott Exp $
|  Program Name  : (ff_rvw_mnt.c)                               
|  Program Desc  : (Review Period Maintenance.) 
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _rvw_mnt.c,v $
| Revision 5.3  2002/07/25 11:17:29  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.2  2001/08/09 09:29:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:48  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/30 05:42:44  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:15:40  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:48  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.11  2000/06/13 05:02:05  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.10  2000/02/07 09:42:37  scott
| S/C LSDI-2455 - Updated to line up prompts. Program found to be missing from menu.
|
| Revision 1.9  1999/12/22 03:40:45  scott
| Updated to change database name to data.
|
| Revision 1.8  1999/12/06 01:34:21  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/17 06:40:15  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/10/27 07:33:03  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.5  1999/09/29 10:10:52  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:42  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 09:20:47  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 07:27:06  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _rvw_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_rvw_mnt/_rvw_mnt.c,v 5.3 2002/07/25 11:17:29 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_lrp_mess.h>

#define	ITEM	 (local_rec.catitem [0] == 'I')
#define	ALL_BR	 (!strcmp (local_rec.br_no, "GLOBAL"))

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int	newReviewPeriod = FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct excfRecord	excf_rec;
struct ffprRecord	ffpr_rec;
struct inmrRecord	inmr_rec;

	char	*data	= "data";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	catitem [9];
	char	stock [17];
	char	desc [41];
	char	br_no [7];
	char	br_name [41];
	char	mask [17];
	char	prmpt [21];
	float	rvw_per;
} local_rec;

static	struct	var	vars [] =
{
	/*----------------
	| Customer  Area |
	----------------*/
	{1, LIN, "catitem",	 4, 15, CHARTYPE,
		"U", "          ",
		" ", "I", "Category/Item  ", "",
		 NE, NO, JUSTRIGHT, "CI", "", local_rec.catitem},
	{1, LIN, "stock",	 5, 15, CHARTYPE,
		local_rec.mask, "          ",
		" ", "I", local_rec.prmpt, "",
		YES, NO,  JUSTLEFT, "", "", local_rec.stock},
	{1, LIN, "desc",	 6, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "br_no",	 8, 15, CHARTYPE,
		"UU", "          ",
		" ", "", "Branch         ", " Default For Global ",
		 NO, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_name",	 9, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_name},
	{1, LIN, "rvw_per",	11, 15, FLOATTYPE,
		"NNNN.NN", "          ",
		" ", "", "Review Period  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.rvw_per},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);
int 	GetReview 		(void);
int 	Update 			(void);
void	SrchEsmr 		(char *);
void 	SrchExcf 		(char *);

int
main (
 int    argc,
 char*  argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	strcpy (local_rec.mask, "UUUUUUUUUUUUUUUU");
	strcpy (local_rec.prmpt, "Item Number    ");

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) & comm_rec);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
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

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ffpr, ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (esmr);
	abc_fclose (excf);
	abc_fclose (ffpr);
	abc_fclose (inmr);
	SearchFindClose ();
	abc_dbclose (data);
}

int
heading (
 int    scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlLrpMess026), 20, 0, 1);
		move (0, 1);
		line (80);

		box (0, 3, 80, 8);

		move (1, 7);
		line (79);
		move (1, 10);
		line (79);

		move (0, 20);
		line (80);
		print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		move (0, 22);
		line (80);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("catitem"))
	{
		if (ITEM)
		{
			strcpy (local_rec.catitem, "Item    ");
			strcpy (local_rec.prmpt, "Item Number    ");
			strcpy (local_rec.mask, "UUUUUUUUUUUUUUUU");
		}
		else
		{
			strcpy (local_rec.catitem, "Category");
			strcpy (local_rec.prmpt, "Category       ");
			strcpy (local_rec.mask, "UUUUUUUUUUU");
		}
		
		vars [label ("stock")].prmpt = strdup (ML (local_rec.prmpt));
		display_prmpt (label ("stock"));
		DSP_FLD ("stock");
		DSP_FLD ("catitem");
	}

	if (LCHECK ("stock"))
	{
		if (ITEM)
		{
			if (SRCH_KEY)
			{
				InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
				return (EXIT_SUCCESS);
			}

			cc = FindInmr (comm_rec.co_no, local_rec.stock, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.stock);
				cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				print_mess (ML (mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			SuperSynonymError ();

			sprintf (local_rec.stock, "%-16.16s", inmr_rec.item_no);
			sprintf (local_rec.desc, "%-40.40s", inmr_rec.description);
			DSP_FLD ("desc");
		}
		else
		{
			if (SRCH_KEY)
			{
				SrchExcf (temp_str);
				return (EXIT_SUCCESS);
			}

			strcpy (excf_rec.co_no, comm_rec.co_no);
			sprintf (excf_rec.cat_no, "%-11.11s", local_rec.stock);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			sprintf (local_rec.desc, "%-40.40s", excf_rec.cat_desc);
			DSP_FLD ("desc");
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("br_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.br_no, "GLOBAL");
			DSP_FLD ("br_no");
			sprintf (local_rec.br_name, "%-40.40s", "COMPANY WIDE");
			DSP_FLD ("br_name");

			cc = GetReview ();
			if (cc)
			{
				newReviewPeriod = FALSE;
				entry_exit = TRUE;
			}
			else
				newReviewPeriod = TRUE;

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%2.2s", temp_str);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);

		}
		sprintf (local_rec.br_no, "%2.2s    ", esmr_rec.est_no);
		sprintf (local_rec.br_name, "%-40.40s", esmr_rec.est_name);
		DSP_FLD ("br_no");
		DSP_FLD ("br_name");

		cc = GetReview ();
		if (cc)
		{
			newReviewPeriod = FALSE;
			entry_exit = TRUE;
		}
		else
			newReviewPeriod = TRUE;

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
GetReview (void)
{
	if (ITEM)
	{
		sprintf (ffpr_rec.category, "%-11.11s", " ");
		ffpr_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sprintf (ffpr_rec.br_no,
			"%2.2s",
			ALL_BR ? "  " : local_rec.br_no);

		abc_selfield (ffpr, "ffpr_id_no");
		cc = find_rec (ffpr, &ffpr_rec, COMPARISON, "u");
		if (cc)
			return (FALSE);

		local_rec.rvw_per = ffpr_rec.review_prd;

		return (TRUE);
	}
	else
	{
		sprintf (ffpr_rec.category, "%-11.11s", local_rec.stock);
		ffpr_rec.hhbr_hash = 0L;
		sprintf (ffpr_rec.br_no,
			"%2.2s",
			ALL_BR ? "  " : local_rec.br_no);

		abc_selfield (ffpr, "ffpr_id_no_1");
		cc = find_rec (ffpr, &ffpr_rec, COMPARISON, "u");
		if (cc)
			return (FALSE);

		local_rec.rvw_per = ffpr_rec.review_prd;

		return (TRUE);
	}
}

int
Update (void)
{
	if (ITEM)
	{
		sprintf (ffpr_rec.category, "%-11.11s", " ");
		ffpr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	}
	else
	{
		sprintf (ffpr_rec.category, "%-11.11s", local_rec.stock);
		ffpr_rec.hhbr_hash = 0L;
	}

	sprintf (ffpr_rec.br_no,
		"%2.2s",
		ALL_BR ? "  " : local_rec.br_no);

	ffpr_rec.review_prd = local_rec.rvw_per;

	if (newReviewPeriod)
	{
		cc = abc_add (ffpr, &ffpr_rec);
		if (cc)
			file_err (cc, ffpr, "DBADD");
	}
	else
	{
		cc = abc_update (ffpr, &ffpr_rec);
		if (cc)
			file_err (cc, ffpr, "DBUPDATE");
	}

	return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char*  key_val)
{
	work_open ();
	save_rec ("#Br", "#Branch Name");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", " ");

	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category", "#Category Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);

	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strcmp (excf_rec.co_no, comm_rec.co_no)&&
	       !strncmp (excf_rec.cat_no, key_val, strlen (key_val)))
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

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}
