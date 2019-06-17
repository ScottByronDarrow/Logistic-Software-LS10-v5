/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_costupd.c,v 5.6 2002/11/22 08:20:03 kaarlo Exp $
|  Program Name  : (sk_costupd.c)
|  Program Desc  : (MFG Cost Update)
|---------------------------------------------------------------------|
|  Date Written  : (23/04/92)      |  Author      : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: sk_costupd.c,v $
| Revision 5.6  2002/11/22 08:20:03  kaarlo
| LS01128 SC4185. Updated to add validation on start and end class.
|
| Revision 5.5  2002/11/22 08:17:43  kaarlo
| . 
|
| Revision 5.3  2001/08/09 09:18:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:46  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:58  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_costupd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_costupd/sk_costupd.c,v 5.6 2002/11/22 08:20:03 kaarlo Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <number.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct bmmsRecord	bmms_rec;
struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;

	char	*data	= "data";
	char	*inmr2	= "inmr2";

#define	BY_ITEM		(local_rec.rangeType [0] == 'I')

	int		levelNo = 0;
	double	req_cost;
	double	batch_cost;

/*-----------------------------
| Local and screen structure  |
-----------------------------*/
struct
{
	char	st_item [17];
	char	st_item_desc [41];
	char	end_item [17];
	char	end_item_desc [41];

	int		bom_alt;
	int		rtg_alt;

	char	st_class [2];
	char	st_cat [12];
	char	st_cat_desc [41];
	char	end_class [2];
	char	end_cat [12];
	char	end_cat_desc [41];

	char	rangeType [2];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_item",	 3, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item     :", "Start Manufacturing Item.",
		NO, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_item_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_item_desc},
	{1, LIN, "end_item",	 4, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", "End Item       :", "End Manufacturing Item.",
		NO, NO,  JUSTLEFT, "", "", local_rec.end_item},
	{1, LIN, "end_item_desc", 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_item_desc},

	{1, LIN, "st_class",	 3, 18, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class    :", "Input Start Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.st_class},
	{1, LIN, "st_cat",	 4, 18, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category :", "Start Category.",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat},
	{1, LIN, "st_cat_desc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.st_cat_desc},
	{1, LIN, "end_class",	 6, 18, CHARTYPE,
		"U", "          ",
		" ", "A", "End Class      :", "Input End Class A-Z.",
		ND, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.end_class},
	{1, LIN, "end_cat",	 7, 18, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~", "End Category   :", "End Category.",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat},
	{1, LIN, "end_cat_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.end_cat_desc},

	{1, LIN, "bom_alt", 6, 18, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "BOM Number     :", "Default BOM Number.",
		NO, NO,  JUSTRIGHT, "1", "32767", (char *) &local_rec.bom_alt},
	{1, LIN, "rtg_alt", 7, 18, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "Routing Number :", "Default Routing Number.",
		NO, NO,  JUSTRIGHT, "1", "32767", (char *) &local_rec.rtg_alt},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	RunProgram 			(char *, char *);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	SrchExcf 			(char *);
void 	Process 			(void);
double 	UpdateItem 			(long, float, float, int);
void 	CalcRtgCost 		(long, float);
double 	CalcBomCost 		(long, float);
double 	GetUOM 				(long);

extern	int	manufacturingSrch;

/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 3 && argc != 6)
	{
		print_at (0,0,mlSkMess298,argv [0]);
		print_at (0,0,mlSkMess299,argv [0]);
		return (EXIT_FAILURE);
	}

	if (argc == 3)
	{
		sprintf (local_rec.rangeType, "%-1.1s", argv [2]);

		OpenDB ();

		SETUP_SCR (vars);

		if (!BY_ITEM)
		{
			FLD ("st_item") 	 	= ND;
			FLD ("st_item_desc") 	= ND;
			FLD ("end_item") 		= ND;
			FLD ("end_item_desc") 	= ND;

			FLD ("st_class") 		= NO;
			FLD ("end_class") 		= NO;
			FLD ("st_cat") 			= NO;
			FLD ("st_cat_desc") 	= NA;
			FLD ("end_cat") 		= NO;
			FLD ("end_cat_desc") 	= NA;
			vars [label ("bom_alt")].row = 9;
			vars [label ("rtg_alt")].row = 10;
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
			entry_exit 			= FALSE;
			edit_exit 			= FALSE;
			prog_exit 			= FALSE;
			restart 			= FALSE;
			init_ok 			= TRUE;
			search_ok 			= TRUE;
			manufacturingSrch	= TRUE;

			/*---------------------
			| Entry Screen Input  |
			---------------------*/
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;

			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			RunProgram (argv [0], argv [1]);
		}
	}
	else
	{
		sprintf (local_rec.rangeType, "%-1.1s", argv [1]);

		OpenDB ();

		if (BY_ITEM)
		{
			sprintf (local_rec.st_item, "%-16.16s", argv [2]);
			sprintf (local_rec.end_item, "%-16.16s", argv [3]);
		}
		else
		{
			sprintf (local_rec.st_class,  "%-1.1s",   argv [2]);
			sprintf (local_rec.st_cat,    "%-11.11s", argv [2] + 1);
			sprintf (local_rec.end_class, "%-1.1s",   argv [3]);
			sprintf (local_rec.end_cat,   "%-11.11s", argv [3] + 1);
		}

		local_rec.bom_alt = atoi (argv [4]);
		local_rec.rtg_alt = atoi (argv [5]);

		Process ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (inmr2, inmr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (excf,  excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	if (BY_ITEM)
		open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	else
		open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (bmms);
	abc_fclose (excf);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (rghr);
	abc_fclose (rgln);
	SearchFindClose ();
	abc_dbclose (data);
}

void
RunProgram (
 char *prog_name, 
 char *prog_desc)
{
	char	tmp_lower [17];
	char	tmp_upper [17];
	char	bom_char [6];
	char	rtg_char [6];

	shutdown_prog ();

	if (BY_ITEM)
	{
		sprintf(tmp_lower, "%-16.16s", local_rec.st_item);
		sprintf(tmp_upper, "%-16.16s", local_rec.end_item);
	}
	else
	{
		sprintf (tmp_lower, "%-1.1s%-11.11s",
			local_rec.st_class,
			local_rec.st_cat);

		sprintf(tmp_upper, "%-1.1s%-11.11s",
			local_rec.end_class,
			local_rec.end_cat);
	}

	sprintf (bom_char, "%d", local_rec.bom_alt);
	sprintf (rtg_char, "%d", local_rec.rtg_alt);

	execlp
	(
		prog_name,
		prog_name,
		local_rec.rangeType,
		tmp_lower,
		tmp_upper,
		bom_char,
		rtg_char,
		(char *) 0
	);
}
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		centre_at (0, 80,ML(mlSkMess618));

		move (0, 1);
		line (80);

		if (BY_ITEM)
		{
			box (0, 2, 80, 5);
			move (0, 5);
			PGCHAR (10);
			line (79);
			PGCHAR (11);
		}
		else
		{
			box (0, 2, 80, 8);
			move (0, 5);
			PGCHAR (10);
			line (79);
			PGCHAR (11);
			move (0, 8);
			PGCHAR (10);
			line (79);
			PGCHAR (11);
		}

		move (0, 20);
		line (80);

		print_at ( 21, 0,ML(mlStdMess038),clip (comm_rec.co_no),
				    clip (comm_rec.co_name));

		print_at ( 21, 45,ML(mlStdMess039),clip (comm_rec.est_no),
					clip (comm_rec.est_name));

		move (0, 22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("st_item"))
	{
		if (FLD ("st_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.st_item_desc, "%-40.40s", "First Item");
			DSP_FLD ("st_item_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.st_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_item_desc, "%-40.40s", " ");
			DSP_FLD ("st_item_desc");
			return (EXIT_FAILURE);
		}
		DSP_FLD ("st_item");
		SuperSynonymError ();

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML(mlStdMess235));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_item_desc, "%-40.40s", " ");
			DSP_FLD ("st_item_desc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_item, local_rec.end_item) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_item_desc, "%-40.40s", " ");
			DSP_FLD ("st_item_desc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.st_item, inmr_rec.item_no);
		strcpy (local_rec.st_item_desc, inmr_rec.description);
		DSP_FLD ("st_item");
		DSP_FLD ("st_item_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_item"))
	{
		if (FLD ("end_item") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.end_item_desc, "%-40.40s", "Last Item");
			DSP_FLD ("end_item_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.end_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.end_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_item_desc, "%-40.40s", " ");
			DSP_FLD ("end_item_desc");
			return (EXIT_FAILURE);
		}
		DSP_FLD ("end_item");
		SuperSynonymError ();

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML(mlStdMess235));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_item_desc, "%-40.40s", " ");
			DSP_FLD ("st_item_desc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_item, local_rec.end_item) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_item_desc, "%-40.40s", " ");
			DSP_FLD ("end_item_desc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.end_item, inmr_rec.item_no);
		strcpy (local_rec.end_item_desc, inmr_rec.description);
		DSP_FLD ("end_item");
		DSP_FLD ("end_item_desc");

		return (EXIT_SUCCESS);
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

	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("st_cat"))
	{
		if (FLD ("st_cat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.st_cat_desc, "%-40.40s", "First Category");
			DSP_FLD ("st_cat_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.st_cat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_cat_desc, "%-40.40s", " ");
			DSP_FLD ("st_cat_desc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_cat, local_rec.end_cat) > 0)
		{
			print_mess (ML(mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_cat_desc, "%-40.40s", " ");
			DSP_FLD ("st_cat_desc");
			return (EXIT_SUCCESS);
		}

		sprintf (local_rec.st_cat, "%-11.11s", excf_rec.cat_no);
		sprintf (local_rec.st_cat_desc, "%-40.40s", excf_rec.cat_desc);
		DSP_FLD ("st_cat");
		DSP_FLD ("st_cat_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("end_cat"))
	{
		if (FLD ("end_cat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.end_cat_desc, "%-40.40s", "Last Category");
			DSP_FLD ("end_cat_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		strcpy (excf_rec.cat_no, local_rec.end_cat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_cat_desc, "%-40.40s", " ");
			DSP_FLD ("end_cat_desc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_cat, local_rec.end_cat) > 0 )
		{
			print_mess (ML(mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.end_cat_desc, "%-40.40s", " ");
			DSP_FLD ("end_cat_desc");
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.end_cat, excf_rec.cat_no);
		strcpy (local_rec.end_cat_desc, excf_rec.cat_desc);
		DSP_FLD ("end_cat");
		DSP_FLD ("end_cat_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char *key_val)
{
	work_open ();
	save_rec ("#Cat No.", "#Description");

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc &&
		!strncmp (excf_rec.cat_no, key_val, strlen (key_val)) &&
		!strcmp (excf_rec.co_no, comm_rec.co_no))
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
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
Process (
 void)
{
	if (BY_ITEM)
	{
		sprintf (inmr_rec.item_no, "%-16.16s", local_rec.st_item);
		dsp_screen ("Reading Bills By Item",
			comm_rec.co_no,
			comm_rec.co_name);
	}
	else
	{
		sprintf (inmr_rec.inmr_class, "%-1.1s", local_rec.st_class);
		sprintf (inmr_rec.category, "%-11.11s", local_rec.st_cat);
		dsp_screen ("Reading Bills By Group",
			comm_rec.co_no,
			comm_rec.co_name);
	}

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		((BY_ITEM &&
		strcmp (inmr_rec.item_no, local_rec.end_item) <= 0) ||
		(!BY_ITEM &&
		strcmp (inmr_rec.inmr_class, local_rec.end_class) <= 0)))
	{
		if (!BY_ITEM &&
			!strcmp (inmr_rec.inmr_class, local_rec.end_class) &&
			strcmp (inmr_rec.category, local_rec.end_cat) > 0)
			break;

		dsp_process ("Item :", inmr_rec.item_no);

		levelNo = 0;

		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, EQUAL, "u");
		if (cc)
			inei_rec.std_batch = 1.00;

		UpdateItem 
		(
			inmr_rec.hhbr_hash,
			inei_rec.std_batch,
			1.00,
			TRUE
		); /* update flag */

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

}

double 
UpdateItem (
	long	hhbrHash, 
	float 	qtyRequired, 
	float 	cnvFct, 
	int 	updateFlag)
{
	double	bom_cost;
	float	cnv_fct;

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = hhbrHash;
	bmms_rec.alt_no = local_rec.bom_alt;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (cc ||
		strcmp (bmms_rec.co_no, comm_rec.co_no) ||
		bmms_rec.hhbr_hash != hhbrHash ||
		bmms_rec.alt_no != local_rec.bom_alt)
	{
		return (-1.00);
	}

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.rtg_alt;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
		return (-1.00);

	levelNo++;
	bom_cost = CalcBomCost (hhbrHash, cnvFct);

	batch_cost = 0.00;
	req_cost = 0.00;

	CalcRtgCost (hhbrHash, qtyRequired);
	levelNo--;

	if (updateFlag)
	{
		inei_rec.hhbr_hash = hhbrHash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, EQUAL, "u");
		if (cc)
			return (-1.00);
		cc = find_hash (inmr2, &inmr2_rec, COMPARISON, "r", hhbrHash);
		if (cc)
			return (-1.00);

		inei_rec.last_cost = (batch_cost + (bom_cost / (double) cnvFct));
		cnv_fct = inmr2_rec.outer_size / inei_rec.std_batch;
		inei_rec.last_cost *= (double) cnv_fct;
		inei_rec.last_cost = twodec (inei_rec.last_cost);
		inei_rec.date_lcost = comm_rec.inv_date;

		cc = abc_update (inei, &inei_rec);
		if (cc)
			file_err (cc, inei, "DBUPDATE");
	}

	return (req_cost + bom_cost);
}

void
CalcRtgCost (
	long	hhbrHash, 
	float	qtyRequired)
{
	double	tmp_cost;
	long	tmp_time;

	inei_rec.hhbr_hash = hhbrHash;
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, EQUAL, "r");
	if (cc)
		inei_rec.std_batch = 1.00;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.rtg_alt;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (!cc)
	{
		rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgln_rec.seq_no = 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		while (!cc &&
			rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
		{
			/* calculate resource cost for the std batch */
			tmp_time = rgln_rec.setup + rgln_rec.run + rgln_rec.clean;
			tmp_cost = rgln_rec.rate + rgln_rec.ovhd_var;
			tmp_cost *= (double) tmp_time;
			tmp_cost /= (double) 60.00;
			tmp_cost += rgln_rec.ovhd_fix;
			tmp_cost *= (double) rgln_rec.qty_rsrc;
			batch_cost += DOLLARS (tmp_cost);

			/* time required for the required resource amount */
			rgln_rec.run *= (float) (qtyRequired);
			rgln_rec.run /= (float) (inei_rec.std_batch);

			/* calculate resource cost for the required qantity */
			tmp_time = rgln_rec.setup + rgln_rec.run + rgln_rec.clean;
			tmp_cost = rgln_rec.rate + rgln_rec.ovhd_var;
			tmp_cost *= (double) tmp_time;
			tmp_cost /= (double) 60.00;
			tmp_cost += rgln_rec.ovhd_fix;
			tmp_cost *= (double) rgln_rec.qty_rsrc;
			req_cost += DOLLARS (tmp_cost);

			cc = find_rec (rgln, &rgln_rec, NEXT, "r");
		}
	}
}

double	
CalcBomCost (
	long	hhbrHash, 
	float 	cnvFct)
{
	double	conv_factor,
			bom_cost = 0.00,
			tmp_cost,
			req_qty,
			last_cost,
			quantity;
	int		save_line,
			flag = FALSE;

	cc = 0;
	while (!cc &&
		bmms_rec.hhbr_hash == hhbrHash &&
		bmms_rec.alt_no == local_rec.bom_alt)
	{
		flag = FALSE;
		inei_rec.hhbr_hash = bmms_rec.mabr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inei, "DBFIND");

		save_line = bmms_rec.line_no;

		if (inei_rec.date_lcost != comm_rec.inv_date)
		{
			UpdateItem (bmms_rec.mabr_hash,
					inei_rec.std_batch,
					1.00,
					TRUE);

			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = hhbrHash;
			bmms_rec.alt_no = local_rec.bom_alt;
			bmms_rec.line_no = save_line;
			cc = find_rec (bmms, &bmms_rec, EQUAL, "r");
			if (cc)
				file_err (cc, bmms, "DBFIND");

			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc)
				file_err (cc, inei, "DBFIND");
		}

		cc = find_hash (inmr2, &inmr2_rec, EQUAL, "r", bmms_rec.mabr_hash);
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		/* calculate required quantity in the std uom */
		conv_factor = GetUOM (bmms_rec.uom);
		req_qty = bmms_rec.matl_wst_pc;
		req_qty += 100.00;
		req_qty /= 100.00;
		req_qty *= bmms_rec.matl_qty;
		req_qty /= conv_factor;

		/* calc conversion factor for quantity required */
		conv_factor = req_qty;
		conv_factor /= inei_rec.std_batch;

		last_cost	=	UpdateItem 
						(
							bmms_rec.mabr_hash,
							req_qty,
							conv_factor,
							FALSE
						);
		if (last_cost < 0.00)
			flag = FALSE;
		else
			flag = TRUE;

		strcpy (bmms_rec.co_no, comm_rec.co_no);
		bmms_rec.hhbr_hash = hhbrHash;
		bmms_rec.alt_no = local_rec.bom_alt;
		bmms_rec.line_no = save_line;
		cc = find_rec (bmms, &bmms_rec, EQUAL, "r");
		if (cc)
			file_err (cc, bmms, "DBFIND");

		inei_rec.hhbr_hash = bmms_rec.mabr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inei, "DBFIND");

		cc = find_hash (inmr2, &inmr2_rec, EQUAL, "r", bmms_rec.mabr_hash);
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		bmms_rec.matl_wst_pc += 100.00;
		bmms_rec.matl_wst_pc /= 100.00;
		quantity = bmms_rec.matl_qty * bmms_rec.matl_wst_pc;
		quantity *= cnvFct;

		if (!flag)
		{
			conv_factor = GetUOM (bmms_rec.uom);
			inei_rec.last_cost /= conv_factor;

			tmp_cost = out_cost (inei_rec.last_cost, inmr2_rec.outer_size);
			tmp_cost *= quantity;
		}
		else
			tmp_cost = last_cost;
			
		bom_cost += tmp_cost;

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	return (bom_cost);
}

double 
GetUOM (
	long	hhumHash)
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
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.alt_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.std_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", hhumHash);
	if (cc)
		file_err (cc, inum, "DBFIND");
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr2_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToDbl (&result));
}

