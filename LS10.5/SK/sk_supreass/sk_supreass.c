/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_supreass.c,v 5.2 2002/03/22 02:24:57 robert Exp $
|  Program Name  : (sk_supreass.c)
|  Program Desc  : (Supplier Rebates Linking Program)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 03/11/93         |
|---------------------------------------------------------------------|
| $Log: sk_supreass.c,v $
| Revision 5.2  2002/03/22 02:24:57  robert
| Updated to fixed compilation error on LS10-GUI
|
| Revision 5.1  2002/02/26 04:57:11  scott
| S/C-00736
| SKPI10-3-Supplier Buying Group Assignment; the Start Date is truncated.
|
---------------------------------------------------------------------*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_supreass.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_supreass/sk_supreass.c,v 5.2 2002/03/22 02:24:57 robert Exp $";

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <tabdisp.h>

#define	SINGLE	1
#define	MULTI	0

#include	"schema"

struct commRecord	comm_rec;
struct ingpRecord	ingp_rec;
struct inrbRecord	inrb_rec;
struct sumrRecord	sumr_rec;
struct suraRecord	sura_rec;

	char	*data = "data";

	extern	int	TruePosition;

	char	branchNumber [3];
	int		envDbCo = FALSE;
	int		cr_find  = FALSE;
	int		deletion = FALSE;
	int		noInTab;
	char	lastCode [7];

struct	{
	char	dummy [11];
} local_rec;

static struct var vars [] = 
{
	{1, LIN, "supp",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Supplier Number  ", "Enter Supplier's Name - Search Available ",
		YES, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "suppdesc",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name    ", " ",
		NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "buygrp",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Buying Group     ", "Enter Buying Group - Search Available ",
		YES, NO,  JUSTLEFT, "", "", ingp_rec.code},
	{1, LIN, "buygrpdesc",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description      ", " ",
		NA, NO,  JUSTLEFT, "", "", ingp_rec.desc},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

/*
 * Callback Declarations 
 */
static	int	TagFunc 	(int, KEY_TAB *);
static	int	ExitFunc 	(int, KEY_TAB *);
static	int	ShowFunc 	(int, KEY_TAB *);

#ifndef	GVISION
static	KEY_TAB list_keys [] =
{
   { " [T]OGGLE ",		'T',		TagFunc,
	"Tag Line.",					"A" },
   { " [^A]ccept All",		CTRL ('A'), 	TagFunc,
	"Tag All Lines.",				"A" },
   { " [S]HOW ",		'S', 	ShowFunc,
	"Show Detailed Information For Rebate Code.",				"A" },
   { NULL,			FN1, 		ExitFunc,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		ExitFunc,
	"Exit and update the database.",				"A" },
   END_KEYS
};
#else
static	KEY_TAB list_keys [] =
{
   { " TOGGLE ",		'T',		TagFunc,
	"Tag Line.",					"A" },
   { " ACCEPT ALL ",		CTRL ('A'), 	TagFunc,
	"Tag All Lines.",				"A" },
   { " SHOW DETAILS ",		'S', 	ShowFunc,
	"Show Detailed Information For Rebate Code.",				"A" },
   { NULL,			FN1, 		ExitFunc,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		ExitFunc,
	"Exit and update the database.",				"A" },
	END_KEYS
};
#endif


#include	<FindSumr.h>
/*
 * Function Declarations 
 */
void OpenDB 		(void);
void CloseDB 		(void);
int  heading 		(int);
int  spec_valid 	(int);
void SrchIngp 		(char *);
void Process 		(void);
void TagLine 		(int);
void Update 		(void);
void LoadGrps 		(void);


/*
 * Main Processing Routine 
 */
int
main (
	int 	argc,
	char 	*argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	envDbCo = atoi (get_env ("CR_CO"));
	cr_find  = atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;
		init_vars (1);
		sprintf (lastCode, "%6.6s", " ");

		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		noInTab = 0;

		heading (1);
		scn_display (1);
		Process ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sumr, 
			  sumr_list, 
			  SUMR_NO_FIELDS, 
			 (cr_find) ? "sumr_id_no3" : "sumr_id_no");
	open_rec (inrb, inrb_list, INRB_NO_FIELDS, "inrb_id_no1");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (sura, sura_list, SURA_NO_FIELDS, "sura_id_no");
}

void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (inrb);
	abc_fclose (ingp);
	abc_fclose (sura);
	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlSkMess464), (40 - (strlen (ML (mlSkMess464)) / 2)), 0, 1);

	line_at (1,0,80);
	line_at (21,0,80);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	box (0, 3, 80, 4);		

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate Creditor Number. 
	 */
	if (LCHECK ("supp"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		pad_num (sumr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("suppdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("buygrp"))
	{
		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ingp_rec.co_no, comm_rec.co_no);
		strcpy (ingp_rec.type, "B");

		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		
		if (cc)
		{
			print_mess (ML (mlSkMess551));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("buygrpdesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchIngp (
 char *key_val)
{
	_work_open (6,0,40);
	save_rec ("#Code", "#Description ");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", key_val);

	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type [0] == 'B')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		else
			break;

		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	strcpy (ingp_rec.type, "B");
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	cc = find_rec (ingp, &ingp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

void
Process (void)
{
	/*
	 * Open table 
	 */
	tab_open ("supass", list_keys, 8, 16, 9, FALSE);
	tab_add ("supass", 
		"# %-11.11s %-25.25s  %-3.3s ",
		"REBATE CODE",
		" ",
		"TAG");

	LoadGrps ();

	if (noInTab == 0)
	{
		tab_add ("supass", "There Are No Valid Rebates");
		tab_add ("supass", "For Selected Supplier.");
		tab_display ("supass", TRUE);
		sleep (sleepTime);
		tab_close ("supass", TRUE);
		return;
	}
	else
	{
		tab_scan ("supass");
	}
	
	if (!restart)
		Update ();

	tab_close ("supass", TRUE);
}

static int 
TagFunc (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [100];

	st_line = tab_tline ("supass");

	if (c == 'T')
		TagLine (st_line);
	else
	{
		for (i = st_line; i < noInTab; i++)
			TagLine (i);

		tab_display ("supass", TRUE);
		tab_get ("supass", get_buf, EQUAL, st_line);
		redraw_keys ("supass");
	}

	return (c);
}

void
TagLine (
 int line_no)
{
	char	get_buf [100];
	char	curr_stat [2];
	char	new_stat [2];

	tab_get ("supass", get_buf, EQUAL, line_no);
	sprintf (curr_stat, "%-1.1s", get_buf + 42);

	if (curr_stat [0] == '*')
		strcpy (new_stat, " ");
	else
		strcpy (new_stat, "*");

	tab_update ("supass", "%-42.42s%-1.1s ", get_buf, new_stat);

	return;
}

static int 	
ExitFunc (
 int c, 
 KEY_TAB *psUnused)
{
	if (c == FN1)
		restart = TRUE;
	
	return (c);
}

void
Update (void)
{
	/*
	 * in all cases will loop thru reading line by line
	 */
	int		count;
	char	get_buf [100];
	char	curr_stat [2];

	for (count = 0; count < noInTab; count++)
	{
		tab_get ("supass", get_buf, EQUAL, count);

		sprintf (curr_stat, "%-1.1s", get_buf + 42);

		/*
		 * find records
		 */
		sura_rec.hhsu_hash = sumr_rec.hhsu_hash;
		sprintf (sura_rec.buygrp, "%-6.6s", ingp_rec.code);
		sprintf (sura_rec.rebate, "%5.5s", get_buf + 1);
		
		cc = find_rec (sura, &sura_rec, EQUAL, "u");
		if (!cc)
		{
			if (curr_stat [0] == '*')
			{
				abc_unlock (sura);
				continue;
			}
			else
			{
				cc = abc_delete (sura);
				if (cc)
					file_err (cc, sura, "DBDELETE");
			}
		}
		else
		{
			if (curr_stat [0] != '*')
			{
				continue;
			}
			else
			{
				cc = abc_add (sura, &sura_rec);
				if (cc)
					file_err (cc, sura, "DBADD");
			}
		}
	}
}

void
LoadGrps (void)
{
	/*
	 * Load In Existing Links
	 */
	sura_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (sura_rec.buygrp, "%-6.6s", ingp_rec.code);
	sprintf (sura_rec.rebate, "%5.5s", " ");
	cc = find_rec (sura, &sura_rec, GTEQ, "r");

	while (!cc && 
			sura_rec.hhsu_hash == sumr_rec.hhsu_hash &&
			!strcmp (sura_rec.buygrp, ingp_rec.code))
	{
		tab_add ("supass", " %-5.5s  %-31.31s  %-3.3s ",
			sura_rec.rebate,	
			" ",
			" * ");
		noInTab++;
		cc = find_rec (sura, &sura_rec, NEXT, "r");
	}
	/*
	 * read through inrbs and load unique rebate codes into tab window
	 */

	memset (&inrb_rec, 0, sizeof (inrb_rec));
	strcpy (inrb_rec.reb_flag, "S");
	inrb_rec.link_hash = sumr_rec.hhsu_hash;

	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc && 
			inrb_rec.reb_flag [0] == 'S' && 
			inrb_rec.link_hash == sumr_rec.hhsu_hash)
	{
		if (strcmp (inrb_rec.reb_code, lastCode))
		{
			/*
			 * check to make sure not already assigned
			 */
			sura_rec.hhsu_hash = sumr_rec.hhsu_hash;
			sprintf (sura_rec.buygrp, "%-6.6s", ingp_rec.code);
			sprintf (sura_rec.rebate, "%5.5s", inrb_rec.reb_code);
			cc = find_rec (sura, &sura_rec, EQUAL, "r");
			if (!cc)
			{
				cc = find_rec (inrb, &inrb_rec, NEXT, "r");
				continue;
			}
			tab_add 
			(
				"supass", 
				" %-5.5s  %-31.31s  %-3.3s ",
				inrb_rec.reb_code,	
				" ",
				"   "
			);
			noInTab++;
			strcpy (lastCode, inrb_rec.reb_code);
		}
		cc = find_rec (inrb, &inrb_rec, NEXT, "r");
	}

}

static int 
ShowFunc (
	int 	iUnused, 
	KEY_TAB *psUnused)
{
	char	get_buf [100];
	int	lineno;

	lineno = tab_tline ("supass");

	cc = tab_get ("supass", get_buf, EQUAL, lineno);
	if (cc)
		sys_err ("Error In Retrieving Line", cc, PNAME);

	Dsp_open (0, 5, 10);
	sprintf (err_str, 
			"       Supp - %6.6s    Buying Grp - %6.6s   Rebate Code %5.5s               ",
			sumr_rec.crd_no,
			ingp_rec.code,
			get_buf + 1);
	Dsp_saverec (err_str);

	sprintf (err_str, "Description                            Basis   Type  Cy Start      End        ");
	Dsp_saverec (err_str);

	sprintf (err_str, " [REDRAW] [NEXT] [PREV] [EDIT/END] ");
	Dsp_saverec (err_str);

	/*
	 * find records
	 */
	memset (&inrb_rec, 0, sizeof (inrb_rec));
	strcpy (inrb_rec.reb_flag, "S");
	inrb_rec.link_hash = sumr_rec.hhsu_hash;
	sprintf (inrb_rec.reb_code, "%5.5s", get_buf + 1);
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc && 
			inrb_rec.reb_flag [0] == 'S' && 
			inrb_rec.link_hash == sumr_rec.hhsu_hash &&
			!strncmp (inrb_rec.reb_code, (get_buf + 1), 5))
	{
		char	basis [8];
		char	endDate [11];

		switch (inrb_rec.basis [0])
		{
		case 'V' :
				sprintf (basis, "%7.7s", "Value  ");
				break;
		case 'U' :
				sprintf (basis, "%7.7s", "Units  ");
				break;
		case 'W' :
				sprintf (basis, "%7.7s", "Weight ");
				break;
		}

		strcpy (endDate, DateToString (inrb_rec.end_date));

		sprintf 
		(
			err_str, 
			"%38.38s %7.7s %5.5s %2d %10.10s %10.10s",
			inrb_rec.description,
			basis,
			(inrb_rec.reb_type [0] == 'V') ? "Value" : " PC  ",
			inrb_rec.cycle,
			DateToString (inrb_rec.start_date),
			endDate
		);
		Dsp_saverec (err_str);

		cc = find_rec (inrb, &inrb_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();

#ifndef GVISION
	heading (1);
	scn_display (1);
#endif	/* GVISION */

	tab_display ("supass", TRUE);
	redraw_keys ("supass");
	return (EXIT_SUCCESS);
}

