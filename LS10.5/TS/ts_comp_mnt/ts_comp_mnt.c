/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ts_comp_mnt.c,v 5.2 2002/07/24 08:39:34 scott Exp $
|  Program Name  : (ts_comp_mnt.c)
|  Program Desc  : (Maintain Complimentary Items)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 27/12/91         |
|---------------------------------------------------------------------|
| $Log: ts_comp_mnt.c,v $
| Revision 5.2  2002/07/24 08:39:34  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.1  2002/06/26 06:32:34  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_comp_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_comp_mnt/ts_comp_mnt.c,v 5.2 2002/07/24 08:39:34 scott Exp $";

/*===========================================
| These next two lines for 132 tabular only |
===========================================*/
#define MAXWIDTH 150
#define MAXLINES 200

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_ts_mess.h>

	char	*inmr2 = "inmr2";

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct tsciRecord	tsci_rec;

	int	new_code = 0;
	int	max_item = 0;

struct	storeRec {
	long	hhbrHash;
} store [MAXLINES];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	item_no [17];
	char	desc [41];
	long	hhbrHash;
	char	comp_item_no [17];
	char	comp_desc [41];
	char	comment [41];
	long	comp_hhbr;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "item_no",	 4, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number : ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Item Desc   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},

	{2, TAB, "comp_item_no",	MAXLINES, 1, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "   Item Number    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.comp_item_no},
	{2, TAB, "comp_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                Description                ", " ",
		 NA, NO,  JUSTLEFT, " ", "", local_rec.comp_desc},
	{2, TAB, "comment",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                 Comment                  ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.comment},
	{2, TAB, "comp_hhbr",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", ""," ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.comp_hhbr},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
int 	DeleteLine 		(void);
void 	SetArray 		(void);
void 	LoadComps 		(void);
void 	Update 			(void);
int 	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int 
main (
 int    argc,
 char*  argv [])
{
	SETUP_SCR (vars);


	init_scr ();	
	set_tty ();
	set_masks ();		
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();

	
	tab_row = 7;
	tab_col = 10;

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*-------------------------------
		| Load Tabular,set new_code	|
		-------------------------------*/
		LoadComps ();

		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		if (new_code == 1)
		{
			SetArray ();
			heading (2);
			entry (2);
		}

		if (restart)
			continue;

		heading (2);
		scn_display (2);
		edit (2);
		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (tsci,  tsci_list, TSCI_NO_FIELDS, "tsci_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (tsci);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int    field)
{
	int	i = 0;
 
	if (LCHECK ("item_no"))
	{
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
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (local_rec.desc,inmr_rec.description);
		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		local_rec.hhbrHash = inmr_rec.hhbr_hash;
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Item No in Tabular	|
	-------------------------------*/
	if (LCHECK ("comp_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		/*-----------------------------
		| Delete file if default used |
		-----------------------------*/
		if (dflt_used)
			return (DeleteLine ());


		cc = FindInmr (comm_rec.co_no, local_rec.comp_item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.comp_item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (inmr_rec.hhbr_hash == local_rec.hhbrHash)
		{
			print_mess (ML (mlTsMess084));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		for (i = 0; i < lcount [2]; i++)
		{
			if (inmr_rec.hhbr_hash == store [i].hhbrHash &&
			    i != line_cnt)
			{
				/*print_mess ("\007 Item Already Complimentary ");*/
				print_mess (ML (mlTsMess085));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		local_rec.comp_hhbr = inmr_rec.hhbr_hash;
		strcpy (local_rec.comp_item_no, inmr_rec.item_no);
		strcpy (local_rec.comp_desc,inmr_rec.description);
		DSP_FLD ("comp_item_no");
		DSP_FLD ("comp_desc");
	}
	return (EXIT_SUCCESS);
}

/*---------------------------------
| delete line if default was used  |
----------------------------------*/
int
DeleteLine (void)
{
	int	i = 0;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*print_mess ("\007 Cannot Delete Lines on Entry ");*/
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [2] < 1)
	{
		/*print_mess ("\007 No Lines To Delete ");*/
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	lcount [2]--;
	this_page = line_cnt/TABLINES;
	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		store [line_cnt].hhbrHash = store [line_cnt + 1].hhbrHash;
		getval (line_cnt + 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	sprintf (local_rec.comp_item_no,"%-16.16s"," ");
	sprintf (local_rec.comp_desc,"%-40.40s"," ");
	sprintf (local_rec.comment,"%-40.40s"," ");
	local_rec.comp_hhbr = 0L;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
SetArray (void)
{
	int	i = 0;

	for (i = 0; i < MAXLINES; i++)
		store [i].hhbrHash = 0L;
}

void
LoadComps (void)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount [2] = 0;

	tsci_rec.hhbr_hash = local_rec.hhbrHash;
	tsci_rec.line_no = 0;
	cc = find_rec (tsci, &tsci_rec, GTEQ, "r");
	while (!cc && tsci_rec.hhbr_hash == local_rec.hhbrHash)
	{
		cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r", tsci_rec.mabr_hash);
		if (cc)
		{
			cc = find_rec (tsci, &tsci_rec, NEXT, "r");
			continue;
		}

		strcpy (local_rec.comp_item_no, inmr_rec.item_no);
		strcpy (local_rec.comp_desc, inmr_rec.description);
		sprintf (local_rec.comment, "%-40.40s", tsci_rec.comment);
		local_rec.comp_hhbr = inmr_rec.hhbr_hash;
		store [lcount [2]].hhbrHash = inmr_rec.hhbr_hash;
		putval (lcount [2]++);

		if (lcount [2] >= MAXLINES)
			break;

		cc = find_rec (tsci, &tsci_rec, NEXT, "r");
	}
	max_item = lcount [2];
	new_code = (lcount [2] == 0);
	scn_set (1);
}

void
Update (void)
{
	int 	cnt = 0;
	int 	i = 0;
	
	scn_set (2);

	for (i = 0;i < lcount [2];i++)
	{
		getval (i);
		
		tsci_rec.hhbr_hash = local_rec.hhbrHash;
		tsci_rec.line_no = cnt++;
		cc = find_rec (tsci, &tsci_rec, COMPARISON, "u");

		tsci_rec.mabr_hash = local_rec.comp_hhbr;
		sprintf (tsci_rec.comment, "%-40.40s", local_rec.comment);
		strcpy (tsci_rec.stat_flag, "0");

		if (cc)
		{
			cc = abc_add (tsci, &tsci_rec);
			if (cc)
				file_err (cc, tsci, "DBADD");
		}
		else
		{
			cc = abc_update (tsci, &tsci_rec);
			if (cc)
				file_err (cc, tsci, "DBUPDATE");
		}
	}

	tsci_rec.hhbr_hash = local_rec.hhbrHash;
	tsci_rec.line_no = i;
	cc = find_rec (tsci, &tsci_rec, GTEQ, "u");
	while (!cc && tsci_rec.hhbr_hash == local_rec.hhbrHash)
	{
		cc = abc_delete (tsci);
		if (cc)
			file_err (cc, tsci, "DBDELETE");

		tsci_rec.hhbr_hash = local_rec.hhbrHash;
		tsci_rec.line_no = i;
		cc = find_rec (tsci, &tsci_rec, GTEQ, "u");
	}

	abc_unlock (tsci);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		swide ();
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		box (0,3,132,2);

		if (scn == 2)
		{
			scn_write (1);
			scn_display (1);
		}

		rv_pr (ML (mlTsMess010),51,0,1);
		move (0,1);
		line (132);

		move (0,20);
		line (132);

		/*print_at (21,0," Co. : %s  %s : Branch : %s %s ",
			comm_rec.co_no, clip (comm_rec.co_name),
			comm_rec.est_no, clip (comm_rec.est_name));*/
		print_at (21,0,ML (mlStdMess038),
			comm_rec.co_no, clip (comm_rec.co_name));
		print_at (21,45,ML (mlStdMess039),
			comm_rec.est_no, clip (comm_rec.est_name));
		move (0,22);
		line (132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
        return (EXIT_SUCCESS);
	}
	else
    {
        /* unlock database file (in case of restart in locked record)*/
		abc_unlock (inmr);
        return (EXIT_FAILURE);
    }
}
