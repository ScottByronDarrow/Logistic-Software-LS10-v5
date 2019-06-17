/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_cusreass.c  )                                 |
|  Program Desc  : ( Customer Rebates Linking Program.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, inrb, cura,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cura,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 03/11/93         |
|---------------------------------------------------------------------|
|  Date Modified : (12/12/95)      | Modified by : Anneliese Allen.   |
|  Date Modified : (08/09/97)      | Modified by : Leah Manibog.      |
|  																	  |
|  Comment		 :			     									  |
|  (12/12/95)    : Reverse logic so selling groups are assigned to    |
|                : rebate codes rather then rebates being assigned to |
|                : customer/selling groups & add Include/Exclude flag |
|  (08/09/97)    : Updated for Multilingual Conversion                |
|  																	  |
| $Log: cusreass.c,v $
| Revision 5.2  2001/08/09 09:18:24  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:28  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:48  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:10:38  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/01/19 03:05:33  cam
| Changes for GVision compatibility.  Fixed field description, and search.
|
| Revision 1.16  1999/11/25 10:24:17  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.15  1999/11/11 05:59:36  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.14  1999/11/03 07:31:57  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.13  1999/10/20 01:38:56  nz
| Updated for remainder of old routines.
|
| Revision 1.12  1999/10/19 21:54:24  scott
| Updated from ansi testing
|
| Revision 1.11  1999/10/13 02:41:56  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.10  1999/10/12 21:20:31  scott
| Updated by Gerry from ansi project.
|
| Revision 1.9  1999/10/08 05:32:19  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:19:55  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
---------------------------------------------------------------------*/
char	*PNAME = "$RCSfile: cusreass.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cusreass/cusreass.c,v 5.2 2001/08/09 09:18:24 scott Exp $";
#define CCMAIN

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <tabdisp.h>

#define	SINGLE	1
#define	MULTI	0

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int	comm_no_fields = 7;

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		char	tcc_no [3];
		char	tcc_name [41];
	} comm_rec;

	/*=====================================
	| Inventory Selling and Selling Groups |
	=====================================*/
	struct dbview ingp_list [] =
	{
		{"ingp_co_no"},
		{"ingp_code"},
		{"ingp_desc"},
		{"ingp_type"},
		{"ingp_sell_reg_pc"}
	};

	int	ingp_no_fields = 5;

	struct tag_ingpRecord
	{
		char	co_no [3];
		char	code [7];
		char	desc [41];
		char	type [2];
		float	sell_reg_pc;
	} ingp_rec;

	/*=======================
	| Inventory Rebate File |
	=======================*/
	struct dbview inrb_list [] =
	{
		{"inrb_link_hash"},
		{"inrb_reb_flag"},
		{"inrb_reb_code"},
		{"inrb_cycle"},
		{"inrb_description"},
		{"inrb_basis"},
		{"inrb_reb_type"},
		{"inrb_start_date"},
		{"inrb_end_date"},
	};

	int	inrb_no_fields = 9;

	struct tag_inrbRecord
	{
		long	link_hash;
		char	reb_flag [2];
		char	reb_code [6];
		int		cycle;
		char	description [41];
		char	basis [2];
		char	reb_type [2];
		long	start_date;
		long	end_date;
	} inrb_rec;

	/*=======================
	| Creditors Master File |
	=======================*/
	struct dbview cumr_list [] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	int	cumr_no_fields = 6;

	struct tag_cumrRecord
	{
		char	cm_co_no [3];
		char	cm_est_no [3];
		char	cm_dbt_no [7];
		long	cm_hhcu_hash;
		char	cm_name [41];
		char	cm_acronym [10];
	} cumr_rec;

	/*==================================
	| Customers Rebate Assignment File |
	==================================*/
	struct dbview cura_list [] =
	{
		{"cura_hhcu_hash"},
		{"cura_sellgrp"},
		{"cura_rebate"},
		{"cura_incl_flag"}
	};

	int	cura_no_fields = 4;

	struct tag_curaRecord
	{
		long	hhcu_hash;
		char	sellgrp [7];
		char	rebate [6];
		char	incl_flag[2];
	} cura_rec;

	char	*comm = "comm",
			*cumr = "cumr",
			*cura = "cura",
			*inrb = "inrb",
			*ingp = "ingp",
			*data = "data";

	char	branchNumber[3];
	int		envDbCo = FALSE;
	int		envDbFind  = FALSE;
	int		deletion = FALSE;
	int		noInTab;

struct	{
	char	inclFlag[2];
	char	inclFlagDesc[8];
	char	dummy[11];
} local_rec;

	char mlsk [100][3];

static struct var vars [] = 
{
	{1, LIN, "cust",	 3, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer    :", "Enter Customer's Name - Search Available ",
		YES, NO,  JUSTLEFT, "", "", cumr_rec.cm_dbt_no},
	{1, LIN, "custdesc",	 4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Cust Name   :", " ",
		NA, NO,  JUSTLEFT, "", "", cumr_rec.cm_name},
	{1, LIN, "rebate",	 5, 15, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Rebate :", "Enter Rebate Code - Search Available ",
		YES, NO,  JUSTLEFT, "", "", inrb_rec.reb_code},
	{1, LIN, "cycle",	 6, 15, INTTYPE,
		"NN", "          ",
		" ", "", "Cycle :", "Enter the Rebate Cycle - Search Available",
		YES, NO,  JUSTLEFT, "1", "99", (char *)&inrb_rec.cycle},
	{1, LIN, "rebatedesc",	 7, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description :", " ",
		NA, NO,  JUSTLEFT, "", "", inrb_rec.description},
	{1, LIN, "inclExcl",	 8, 15, CHARTYPE,
		"U", "          ",
		" ", "I", "Include/Exclude :", "I(nclude) or E(xclude) following selling groups",
		YES, NO,  JUSTLEFT, "IE", "", local_rec.inclFlag},
	{1, LIN, "inclFlagDesc",	 8, 18, CHARTYPE,
		"UAAAAAA", "          ",
		" ", "Include", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.inclFlagDesc},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

static	int	TagFunc (int c, KEY_TAB *psUnused);
static	int	ExitFunc (int c, KEY_TAB *psUnused);

static	KEY_TAB list_keys [] =
{
   { "[T]OGGLE ",		'T',		TagFunc,
	"Tag Line.",					"A" },
   { "[^A]ccept ALL",		CTRL('A'), 	TagFunc,
	"Tag All Lines.",				"A" },
   { NULL,			FN1, 		ExitFunc,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		ExitFunc,
	"Exit and update the database.",				"A" },
   END_KEYS
};


#include	<FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
void OpenDB (void);
void CloseDB (void);
int  heading (int scn);
int  spec_valid (int field);
void Process (void);
void TagLine (int line_no);
void Update (void);
void LoadGrps (void);
void SrchRebCode (char *);
void SrchCycle (char *);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc,
 char * argv[])
{

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	strcpy (branchNumber, (envDbCo) ? comm_rec.test_no : " 0");

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = FALSE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;
		init_vars (1);

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
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (cumr, 
			  cumr_list, 
			  cumr_no_fields, 
			  (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	open_rec (inrb, inrb_list, inrb_no_fields, "inrb_id_no1");
	open_rec (ingp, ingp_list, ingp_no_fields, "ingp_id_no2");
	open_rec (cura, cura_list, cura_no_fields, "cura_id_no2");
}

void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (inrb);
	abc_fclose (ingp);
	abc_fclose (cura);
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
	rv_pr (ML(mlSkMess158), (40 - (strlen (ML(mlSkMess158)) / 2)), 0, 1);

	move (0, 1);
	line (80);
	move (0, 21);
	line (80);

	print_at (22, 0, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);

	box (0, 2, 80, 6);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("cust"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.cm_co_no, comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no, branchNumber);
		pad_num (cumr_rec.cm_dbt_no);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc) 
		{
			print_mess (ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("custdesc");
		return(0);
	}

	if (LCHECK ("rebate"))
	{
		if (SRCH_KEY)
		{
			SrchRebCode (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inrb_rec.reb_flag, "C");
		inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
		inrb_rec.cycle = 0;
		cc = find_rec (inrb, &inrb_rec, GTEQ, "r");
		if (cc || inrb_rec.link_hash != cumr_rec.cm_hhcu_hash)
		{
			print_mess (ML(mlSkMess159));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Allow Search On Cycle Within Rebate Code.  |
	--------------------------------------------*/

	if (LCHECK ("cycle"))
	{
		if (SRCH_KEY)
		{
			SrchCycle (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inrb_rec.reb_flag, "C");
		inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
		cc = find_rec (inrb, &inrb_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML(mlSkMess159));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("rebatedesc");

		/*--------------------------------
		| At this point pick up the first |
		| cura for rebate (if it exists)  |
		| to get default include flag.    |
		--------------------------------*/

		cura_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
		sprintf (cura_rec.rebate, "%5.5s", inrb_rec.reb_code);
		sprintf (cura_rec.sellgrp, "%-6.6s", " ");
		cc = find_rec (cura, &cura_rec, GTEQ, "r");
		if (!cc && cura_rec.hhcu_hash == cumr_rec.cm_hhcu_hash
			&& !strcmp (cura_rec.rebate, inrb_rec.reb_code))
		{
			strcpy (local_rec.inclFlag, cura_rec.incl_flag);
			FLD ("inclExcl") = NI;

			if (local_rec.inclFlag[0] == 'I')
				strcpy (local_rec.inclFlagDesc, "Include");
			else
				strcpy (local_rec.inclFlagDesc, "Exclude");

			DSP_FLD ("inclFlagDesc");
		}
		else
			FLD ("inclExcl") = YES;

	}

	if (LCHECK ("inclExcl"))
	{

		if (dflt_used)
			strcpy (local_rec.inclFlag, "I");

		if (local_rec.inclFlag[0] == 'I')
			strcpy (local_rec.inclFlagDesc, "Include");
		else
			strcpy (local_rec.inclFlagDesc, "Exclude");

		DSP_FLD ("inclFlagDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	/*------------
	| Open table |
	------------*/
	tab_open("cusass", list_keys, 8, 26, 9, FALSE);
	tab_add("cusass", 
		"#. %-13.13s  %-15.15s  %-3.3s ",
		"SELLING GROUP",
		"",
		"TAG");

	LoadGrps ();

	if (noInTab == 0)
	{
		tab_add("cusass", "There Are No Selling Groups");
		tab_add("cusass", "On File.");
		tab_display("cusass", TRUE);
		sleep(2);
		tab_close("cusass", TRUE);
		return;
	}
	else
	{
		tab_scan("cusass");
	}
	
	if (!restart)
		Update();

	tab_close("cusass", TRUE);
}

static int 
TagFunc (
 int c, 
 KEY_TAB *psUnused)
{
	int	i;
	int	st_line;
	char	get_buf[100];

	st_line = tab_tline("cusass");

	if (c == 'T')
		TagLine(st_line);
	else
	{
		for (i = st_line; i < noInTab; i++)
			TagLine(i);

		tab_display("cusass", TRUE);
		tab_get ("cusass", get_buf, EQUAL, st_line);
		redraw_keys("cusass");
	}


	return(c);
}

void
TagLine (
 int line_no)
{
	char	get_buf[100];
	char	curr_stat[2];
	char	new_stat[2];

	tab_get ("cusass", get_buf, EQUAL, line_no);
	sprintf (curr_stat, "%-1.1s", get_buf + 34);

	if (curr_stat[0] == '*')
		strcpy (new_stat, " ");
	else
		strcpy (new_stat, "*");

	tab_update("cusass",
		"%-34.34s%-1.1s ",
		get_buf,
		new_stat);
}

/*
static int 	ShowFunc (int iUnused, KEY_TAB *psUnused)
{
	char	get_buf[100];
	int	lineno;
	
	strcpy (mlsk[0], ML ("Cust"));
	strcpy (mlsk[1], ML ("Selling Group"));
	strcpy (mlsk[2], ML ("Rebate Code"));

	lineno = tab_tline("cusass");

	cc = tab_get ("cusass", get_buf, EQUAL, lineno);
	if (cc)
		sys_err ("Error In Retrieving Line", cc, PNAME);

	Dsp_open (2, 5, 10);
	
	sprintf (err_str, 
			"       Cust - %6.6s   Selling Grp - %6.6s   Rebate Code %5.5s            ",
			cumr_rec.cm_dbt_no,
			ingp_rec.code,
			get_buf + 1);

	sprintf (err_str, "     %s - %6.6s    %s  - %6.6s      %s   %5.5s ", 
			mlsk[0], cumr_rec.cm_dbt_no, mlsk[1], ingp_rec.code, 
			mlsk[2], get_buf + 1);
	Dsp_saverec (err_str);

	sprintf (err_str, "Description                              Basis   Type  Cy Start    End     ");
	Dsp_saverec (err_str);

	sprintf (err_str, "          [REDRAW SCREEN]   [NEXT SCREEN]   [PREVIOUS SCREEN]   [END]  ");
	Dsp_saverec (err_str);

	---------------
	| find records
	---------------
	memset (&inrb_rec, 0, sizeof (inrb_rec));
	strcpy (inrb_rec.reb_flag, "C");
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
	sprintf (inrb_rec.reb_code, "%5.5s", get_buf + 1);
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc && 
			inrb_rec.reb_flag [0] == 'C' && 
			inrb_rec.link_hash == cumr_rec.cm_hhcu_hash &&
			!strncmp (inrb_rec.reb_code, (get_buf + 1), 5)
		  )
		  {
			char	basis [8];
			char	end_date [11];

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

			sprintf (end_date, "%s", DateToString (inrb_rec.end_date));

			sprintf (err_str, 
					"%40.40s %7.7s %5.5s %2d %10.10s %10.10s",
					inrb_rec.description,
					basis,
					(inrb_rec.reb_type [0] == 'V') ? "Value" : " PC  ",
					inrb_rec.cycle,
					DateToString (inrb_rec.start_date),
					end_date
					);
			Dsp_saverec (err_str);

			cc = find_rec (inrb, &inrb_rec, NEXT, "r");
		  }

	Dsp_srch ();
	Dsp_close ();
	heading (1);
	scn_display (1);
	tab_display("cusass", TRUE);
	redraw_keys("cusass");
	return(0);
}
*/

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
Update (
 void)
{
	/*------------------------
	| in all cases will loop
	| thru reading line by line
	--------------------------*/
	int		count;
	char	get_buf[100];
	char	curr_stat[2];

	for (count = 0; count < noInTab; count++)
	{
		tab_get ("cusass", get_buf, EQUAL, count);

		sprintf(curr_stat, "%-1.1s", get_buf + 34);
		/*---------------
		| find records
		---------------*/
		cura_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
		sprintf (cura_rec.sellgrp, "%-6.6s", get_buf + 1);
		sprintf (cura_rec.rebate, "%5.5s", inrb_rec.reb_code);
		strcpy (cura_rec.incl_flag, local_rec.inclFlag);
		cc = find_rec (cura, &cura_rec, EQUAL, "u");
		if (!cc)
		{
			if (curr_stat [0] == '*')
			{
				abc_unlock (cura);
				continue;
			}
			else
			{
				cc = abc_delete (cura);
				if (cc)
					file_err (cc, cura, "DBDELETE");
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
				cc = abc_add (cura, &cura_rec);
				if (cc)
					file_err (cc, cura, "DBADD");
			}
		}
	}
}

void
LoadGrps (
 void)
{
	/*-------------------
	| Load In Existing Links
	---------------------*/
	cura_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
	sprintf (cura_rec.rebate, "%5.5s", inrb_rec.reb_code);
	sprintf (cura_rec.sellgrp, "%-6.6s", " ");
	cc = find_rec (cura, &cura_rec, GTEQ, "r");

	while (!cc && 
			cura_rec.hhcu_hash == cumr_rec.cm_hhcu_hash &&
			!strcmp (cura_rec.rebate, inrb_rec.reb_code))
	{
		tab_add("cusass", " %-6.6s  %-22.22s  %-3.3s ",
			cura_rec.sellgrp,	
			"",
			" * ");
		noInTab++;
		cc = find_rec (cura, &cura_rec, NEXT, "r");
	}
	/*------------------
	| read through ingps
	| and load unique rebate 
	| codes into tab window
	-----------------------*/

	memset (&ingp_rec, 0, sizeof (ingp_rec));
	strcpy (ingp_rec.co_no, comm_rec.tco_no);
	strcpy (ingp_rec.type, "S");
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");

	while (!cc && 
			!strcmp (ingp_rec.co_no, comm_rec.tco_no) && 
			ingp_rec.type[0] == 'S'
		  )
		  {
			/*------------------------
			| check to make sure not
			| already assigned
			------------------------*/
			cura_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
			sprintf (cura_rec.sellgrp, "%-6.6s", ingp_rec.code);
			sprintf (cura_rec.rebate, "%5.5s", inrb_rec.reb_code);
			cc = find_rec (cura, &cura_rec, EQUAL, "r");
			if (!cc)
			{
				cc = find_rec (ingp, &ingp_rec, NEXT, "r");
				continue;
			}
			tab_add("cusass", " %-6.6s  %-22.22s  %-3.3s ",
				ingp_rec.code,	
				"",
				"   ");
			noInTab++;
			
			cc = find_rec (ingp, &ingp_rec, NEXT, "r");
		  }

}

void
SrchRebCode (
 char *key_val)
{
	char	cy_str [3];

	work_open ();
	save_rec ("#Code  Cycle", "#Description");

	sprintf (inrb_rec.reb_code, "%-5.5s", key_val);

	strcpy (inrb_rec.reb_flag, "C");
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
	inrb_rec.cycle = 0;
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc)
	{
		if (inrb_rec.link_hash == cumr_rec.cm_hhcu_hash &&
			!strcmp (inrb_rec.reb_flag, "C"))
		{
				sprintf (err_str,"%5.5s  %2d  ", 
						inrb_rec.reb_code, inrb_rec.cycle) ;
				cc = save_rec (err_str, inrb_rec.description);
		}

		if (cc)
			break;

		cc = find_rec (inrb, &inrb_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		inrb_rec.cycle = 0;
		inrb_rec.description [0] = '\0';
		return;
	}

	sprintf (inrb_rec.reb_code, "%-5.5s", temp_str);
	sprintf (cy_str, "%-2.2s", temp_str + 7);

	inrb_rec.cycle = atoi (cy_str);
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash; 
	strcpy (inrb_rec.reb_flag, "C");

	cc = find_rec (inrb, &inrb_rec, COMPARISON, "w");
	if (cc)
		file_err (cc, inrb, "DBFIND");

}

void
SrchCycle (
 char *key_val)
{
	char	rebateCode[6];
	char	cycle_str [3];

	work_open ();
	save_rec ("#Cy", "#Description");

	strcpy (rebateCode, inrb_rec.reb_code);
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
	inrb_rec.cycle = 0;
	strcpy (inrb_rec.reb_flag, "C");
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc)
	{
		sprintf (cycle_str,"%2d",inrb_rec.cycle);

		if (!strcmp (rebateCode, inrb_rec.reb_code) &&
			inrb_rec.link_hash == cumr_rec.cm_hhcu_hash &&
			!strcmp (inrb_rec.reb_flag, "C"))

			cc = save_rec (cycle_str, inrb_rec.description);
		else
			break;

		if (cc)
			break;

		cc = find_rec (inrb, &inrb_rec, NEXT, "r");
	}

	cc = disp_srch ();
	inrb_rec.cycle = atoi(temp_str);
	work_close ();
	if (cc)
		return;

	strcpy (inrb_rec.reb_code, rebateCode);
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash; 
	strcpy (inrb_rec.reb_flag, "C");

	cc = find_rec (inrb, &inrb_rec, COMPARISON, "w");
	if (cc)
		file_err (cc, inrb, "DBFIND");
}


