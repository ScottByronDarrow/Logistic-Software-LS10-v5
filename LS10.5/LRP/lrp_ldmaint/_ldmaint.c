/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _ldmaint.c,v 5.2 2001/08/09 09:29:50 scott Exp $
|  Program Name  : ( sk_ldmaint.c   )                                 |
|  Program Desc  : ( Add / Update Inventory Lead-date Records     )   |
|---------------------------------------------------------------------|
|  Date Written  : (22/08/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: _ldmaint.c,v $
| Revision 5.2  2001/08/09 09:29:50  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:35  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:35  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/24 08:44:13  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _ldmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_ldmaint/_ldmaint.c,v 5.2 2001/08/09 09:29:50 scott Exp $";

#define MAXWIDTH	135
#define	MAXLINES	500
#define	TABLINES	10

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

	int		envCrCo;
	int		envCrFind;
	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
	
char 	*inis2 	= "inis2",
		*sumr2 	= "sumr2";

int		dspOpen		=	FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	update [1];
	char	prev_item [17];
	char	inv_date [11];
	char	item_no [17];
	char	inmr_desc [41];
	char	inis_desc [41];
	long	ld_ord_date;
	long	ld_sup_date;
	double	nor_cost;
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "item_no",	 3, 24, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.", " ",
		NE, NO, JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",		 3, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.inmr_desc},
	{1, LIN, "crd_no",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier No.", " ",
		NE, NO, JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "crd_name",	 4, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name.", " ",
		NA, NO, JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "sitem_no",	 5, 24, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", inmr_rec.item_no, "Supplier Item Number.", " ",
		NA, NO, JUSTLEFT, "", "", inis_rec.sup_part},
	{1, LIN, "sdesc",	 5, 88, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description.", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.inis_desc},
	{1, LIN, "priority",	 6, 24, CHARTYPE,
		"NN", "          ",
		" ", "1", "Supplier Priority.", " ",
		NA, NO, JUSTLEFT, "123456789", "", inis_rec.sup_priority},
	{2, TAB, "ld_ord",	 MAXLINES, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		"", "", "  Order by  ", " ",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.ld_ord_date},
	{2, TAB, "ld_sup",	 0, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		"", "", "  Supply by ", " ",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.ld_sup_date},
	{0, LIN, "", 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include <FindSumr.h>
/*=============================
| Local Function Prototypes.  |
=============================*/
int		heading 		(int);
int		spec_valid 		(int);
int		Update 			(void);
void	shutdown_prog 	(void);
void	CloseDB 		(void);
void	DisplaySupplier	(void);
void	LoadDates 		(void);
void	OpenDB 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	SETUP_SCR (vars);

	envCrCo		= atoi (get_env ("CR_CO"));
	envCrFind	= atoi (get_env ("CR_FIND"));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	init_vars (2);

	tab_row = 8;
	tab_col = 50;

	OpenDB ();

	strcpy (local_rec.inv_date, DateToString (comm_rec.inv_date));
	strcpy (branchNumber, (!envCrCo) ? " 0" : comm_rec.est_no);

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		if (dspOpen)
		{
			dspOpen = FALSE;
			Dsp_close ();
		}
		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			abc_unlock (inis);
			continue;
		}

		/*-------------------------------
		| Edit screen 2 tabular input . |
		-------------------------------*/
		heading (2);
		lcount [2] = 0;
		LoadDates ();
		scn_display (2);
		edit (2);
		if (restart)
		{
			abc_unlock (inis);
			continue;
		}

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
	abc_dbopen ("data");

	abc_alias (inis2, inis);
	abc_alias (sumr2, sumr);

	read_comm( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );

	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inld,  inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, (!envCrFind) 
											? "sumr_id_no" : "sumr_id_no3");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inis);
	abc_fclose (inis2);
	abc_fclose (inld);
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	SearchFindClose ();
	abc_dbclose ("data");
}


int
spec_valid (
 int    field)
{
	int		i,
			thisPage;

	/*-------------------------------
	| Validate Item Number  input . |
	-------------------------------*/
	if (LCHECK("item_no"))
	{
		/*-------------------------
		| Search for part number. |
		-------------------------*/
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML(mlStdMess001));
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		DSP_FLD ("item_no");

		strcpy (local_rec.inmr_desc, inmr_rec.description);
		DSP_FLD ("desc");

		DisplaySupplier ();
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Creditors Number Input. |
	----------------------------------*/
	if (LCHECK ("crd_no"))
	{
		if (dspOpen)
		{
			dspOpen = FALSE;
			Dsp_close ();
		}
		/*-----------------------
		| Serach for Suppliers. |
		-----------------------*/
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------------
		| Loopup Supplier master file. |
		------------------------------*/
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.crd_no, pad_num (sumr_rec.crd_no));
		strcpy (sumr_rec.est_no, branchNumber);
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("crd_name");

		/*--------------------------------------
		| Find Inventory Supplier master file. |
		--------------------------------------*/
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (inis_rec.co_no, "  ");
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, GTEQ, "r");
		if (!cc && inis_rec.hhbr_hash != inmr_rec.hhbr_hash &&
			       inis_rec.hhsu_hash != sumr_rec.hhsu_hash)
		{
			sprintf (err_str, ML(mlSkMess545) , sumr_rec.crd_no,
												inmr_rec.item_no);
			print_mess (err_str);

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate actual dates	|
	-----------------------*/
	if (LCHECK ("ld_ord") || LCHECK ("ld_sup"))
	{
		thisPage = line_cnt / TABLINES;
		if (last_char == INSLINE)
		{
			if (lcount [2] >= vars [label ("ld_ord")].row)
			{
				print_mess (ML(mlStdMess076));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			for (i = line_cnt, line_cnt = lcount [2]; line_cnt > i; line_cnt--)
			{
				getval (line_cnt - 1);
				putval (line_cnt);
				if (thisPage == line_cnt / TABLINES)
					line_display ();
			}
			lcount [2]++;
			line_cnt = i;

			local_rec.ld_ord_date = 0L;
			local_rec.ld_sup_date = 0L;
			putval (line_cnt);

			if (thisPage == line_cnt / TABLINES)
				blank_display ();

			init_ok = 0;
			prog_status = ENTRY;
			scn_entry (cur_screen);
			prog_status = !ENTRY;
			init_ok = 1;
			line_cnt = i;
			getval (line_cnt);

			return (EXIT_SUCCESS);
		}

		if ((dflt_used ||
		     (LCHECK ("ld_ord") && local_rec.ld_ord_date == 0L) ||
		     (LCHECK ("ld_sup") && local_rec.ld_sup_date == 0L)) &&
		    lcount [2] > 0)
		{
			lcount [2]--;

			for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				if (thisPage == line_cnt / TABLINES)
					line_display ();
			}

			local_rec.ld_ord_date = 0L;
			local_rec.ld_sup_date = 0L;
			putval (line_cnt);

			if (thisPage == line_cnt / TABLINES)
				blank_display ();

			line_cnt = i;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("ld_ord"))
	{
		if (local_rec.ld_sup_date < local_rec.ld_ord_date &&
		    prog_status != ENTRY)
		{
			print_mess (ML(mlSkMess547));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ld_sup"))
	{
		if (local_rec.ld_sup_date < local_rec.ld_ord_date)
		{
			print_mess (ML(mlSkMess547));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=======================================
| Stuff the current dates into screen 2	|
=======================================*/
void
LoadDates (void)
{
	inld_rec.hhis_hash = inis_rec.hhis_hash;
	inld_rec.ord_date = 0L;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	while (!cc && inld_rec.hhis_hash == inis_rec.hhis_hash)
	{
		local_rec.ld_ord_date = inld_rec.ord_date;
		local_rec.ld_sup_date = inld_rec.sup_date;
		putval (lcount [2]++);
		cc = find_rec (inld, &inld_rec, NEXT, "r");
	}
}

/*==================================================
| Update or add a record to inventory branch file. |
==================================================*/
int
Update (void)
{
	int	anyDates = FALSE;

	strcpy (local_rec.prev_item, inmr_rec.item_no);

	/*-----------------------
	| Remove old records	|
	-----------------------*/
	inld_rec.hhis_hash = inis_rec.hhis_hash;
	inld_rec.ord_date = 0L;
	cc = find_rec (inld, &inld_rec, GTEQ, "u");
	while (!cc && inld_rec.hhis_hash == inis_rec.hhis_hash)
	{
		cc = abc_delete (inld);
		cc = find_rec (inld, &inld_rec, GTEQ, "u");
	}

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		anyDates = TRUE;
		getval (line_cnt);

		inld_rec.hhis_hash = inis_rec.hhis_hash;
		inld_rec.ord_date = local_rec.ld_ord_date;
		inld_rec.sup_date = local_rec.ld_sup_date;

		cc = abc_add (inld, &inld_rec);
		if (cc)
			file_err (cc, inld, "DBADD");
	}
	if (anyDates)
		inis_rec.lead_time = 0.00;

	return (EXIT_SUCCESS);
}

void
DisplaySupplier (
 void)
{
	char	disp_str [200];
	/*-----------------------------------
	| setup supplier item display		|
	-----------------------------------*/
	dspOpen = TRUE;
	Dsp_open (30, 5, 11);
	Dsp_saverec ("Pri| Br | Wh |Supplier|              Supplier Name             ");
	Dsp_saverec ("No.| No | No | Number |                                        ");
	Dsp_saverec ("");

	inis2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis2_rec.sup_priority, "   ");
	strcpy (inis2_rec.co_no, "  ");
	strcpy (inis2_rec.br_no, "  ");
	strcpy (inis2_rec.wh_no, "  ");
	cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
	while (!cc && inis2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sumr2_rec.hhsu_hash	=	inis2_rec.hhsu_hash;
		cc = find_rec (sumr2, &sumr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}

		if (strcmp (inis2_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
		sprintf (disp_str, "%s ^E %2.2s ^E %2.2s ^E %6.6s ^E%40.40s",
				inis2_rec.sup_priority,
				inis2_rec.br_no,
				inis2_rec.wh_no,
				sumr2_rec.crd_no,
				sumr2_rec.crd_name);
			
		Dsp_saverec (disp_str);

		cc = find_rec (inis2, &inis2_rec, NEXT, "r");
	}

	Dsp_srch ();
}
int
heading (
 int    scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		clear ();
		rv_pr (ML(mlSkMess544), 40, 0, 1);

		print_at (0,90, ML(mlSkMess096) , local_rec.prev_item);

		move (0, 1);
		line (132);
		box (0, 2, 132, 4);

		move (0, 20);
		line (132);
		strcpy(err_str, ML(mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no, comm_rec.co_name);

		strcpy(err_str, ML(mlStdMess039));
		print_at (22,0, err_str, comm_rec.est_no, comm_rec.est_name);

		move (0, 23);
		line (132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;

		if (scn != 1)
		{
			scn_write (1);
			scn_display (1);
		}

		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
