/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_marg.i.c,v 5.3 2002/07/17 09:57:56 scott Exp $
|  Program Name  : ( sk_marg.i.c  )                                   |
|  Program Desc  : ( Sales Margin Exception Input.                )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison. | Date Written  : 03/02/88         |
|---------------------------------------------------------------------|
| $Log: sk_marg.i.c,v $
| Revision 5.3  2002/07/17 09:57:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:19:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:20  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/21 04:12:04  scott
| Updated to ensure default on start and end selection takes into account
| high end character set. Start range is space and and range is 0xff
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_marg.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_marg.i/sk_marg.i.c,v 5.3 2002/07/17 09:57:56 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define BY_ITEM		 (local_rec.group_by [0] == 'I') 
#define BY_CUST		 (local_rec.group_by [0] == 'C') 

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;

	char	systemDate [11];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	group_by [12];
	char	group_by_desc [12];
	char	mask [17];
	char	desc [10];
	char	lower [17];
	char	upper [17];
	char	st_desc [41];
	char	ed_desc [41];
	float	margin;
	char	marg_pc [7];
	char	type [8];
	char	type_desc [8];
	long 	start_dt;
	long 	end_dt;
	char	s_date [11];
	char	e_date [11];
	char	all_br [6];
	char	all_br_desc [6];
	char	stat [2];
	char	stat_flag [17];
	char	stat_flag_desc [17];
	char 	back [6];
	char 	back_desc [6];
	char 	onight [6];
	char 	onight_desc [6];
	char	prog_desc [61];
	int		printerNumber;
	char	lp_str [2];
	char	trueStart [17];
	char	trueEnd [17];
} local_rec;
	
static	struct	var	vars [] =
{
	{1, LIN, "gp_by",	 4, 18, CHARTYPE,
		"U", "          ",
		" ", "I", " Group By. ", " I(nventory) Or C(ustomer) ",
		YES, NO,  JUSTLEFT, "IC", "", local_rec.group_by},
	{1, LIN, "gp_by_desc",	 4, 21, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.group_by_desc},
	{1, LIN, "item_lower",	 5, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Lower Bound. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.lower},
	{1, LIN, "cust_lower",	 5, 18, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", " ", " Lower Bound. ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.lower},
	{1, LIN, "st_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "item_upper",	 6, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", " Upper Bound. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.upper},
	{1, LIN, "cust_upper",	 6, 18, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "~~~~~~~~~~~~~~~~", " Upper Bound. ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.upper},
	{1, LIN, "ed_desc",	 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ed_desc},
	{1, LIN, "margn",	 7, 18, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", " Margin % ", "Enter 0.0 for All Items.",
		YES, NO, JUSTRIGHT, "-99.99", "100.00", (char *)&local_rec.margin},
	{1, LIN, "ab",	 8, 18, CHARTYPE,
		"U", "          ",
		" ", "A", " A(bove) Or B(elow) ", " ",
		YES, NO,  JUSTLEFT, "AB", "", local_rec.type},
	{1, LIN, "ab_desc",	 8, 21, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.type_desc},
	{1, LIN, "st_dt",	 9, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", " Start Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start_dt},
	{1, LIN, "en_dt",	10, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, " End Date.", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_dt},
	{1, LIN, "all_br",	11, 18, CHARTYPE,
		"U", "          ",
		" ", "Y", " All Branches.", " Y(es) N(o) ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.all_br},
	{1, LIN, "all_br_desc",	 11, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.all_br_desc},
	{1, LIN, "stat_flag",	12, 18, CHARTYPE,
		"U", "          ",
		" ", "B", " Before/After Post", "Enter B(efore Posting) A(fter Posting)",
		YES, NO,  JUSTLEFT, "AB", "", local_rec.stat_flag},
	{1, LIN, "stat_flag_desc",	 12, 21, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.stat_flag_desc},
	{1, LIN, "printerNumber",	14, 18, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No. ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",	15, 18, CHARTYPE,
		"U", "          ",
		" ", "N", " Background . ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "back_desc", 15, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",16, 18, CHARTYPE,
		"U", "          ",
		" ", "N", " Overnight . ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc", 16, 21, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.onight_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	SetDefaults 	(void);
void 	RunProgram 		(char *);
void 	SrchCumr 		(char *, char *);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
int  	heading 		(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 3)
	{
		print_at (0,0,mlStdMess037,argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (local_rec.prog_desc,"%s",clip (argv [2]));

	strcpy (systemDate, DateToString (TodaysDate ()));

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();    		/*  get into raw mode			*/
	set_masks ();		/*  setup print using masks		*/
	init_vars (1);		/*  set default values			*/
						/*------------------------------*/

	OpenDB ();

	/*---------------------
	| Reset control flags |
	---------------------*/
   	entry_exit 	= FALSE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	search_ok 	= TRUE;
	init_vars (1);			/*  set default values		*/

	SetDefaults ();
	/*----------------------------
	| Edit screen 1 linear input |
	----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);

	rset_tty ();

    if (!restart) 
	    RunProgram (argv [1]);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefaults (
 void)
{
	strcpy (local_rec.group_by, "I");
	strcpy (local_rec.group_by_desc, ML ("Inventory"));
	strcpy (local_rec.lower,"                ");
	strcpy (local_rec.upper,"~~~~~~~~~~~~~~~~");
	FLD ("item_upper") 	= YES;
	FLD ("item_lower") 	= YES;
	FLD ("cust_upper") 	= ND;
	FLD ("cust_lower") 	= ND;
	local_rec.margin = 50.00;
	local_rec.printerNumber = 1;
	strcpy (local_rec.type, "A");
	strcpy (local_rec.type_desc, "Above");
	local_rec.start_dt = 0L;
	local_rec.end_dt = StringToDate (systemDate);
	strcpy (local_rec.all_br, "Y");
	strcpy (local_rec.all_br_desc, ML ("Yes"));
	strcpy (local_rec.stat_flag, "B");
	strcpy (local_rec.stat_flag_desc, ML ("Before Posting"));
	strcpy (local_rec.back, "N");
	strcpy (local_rec.back_desc, ML ("No "));
	strcpy (local_rec.onight, "N");
	strcpy (local_rec.onight_desc, ML ("No "));
	memset ((char *)local_rec.trueStart,0,	 sizeof (local_rec.trueStart));
	memset ((char *)local_rec.trueEnd,	0xff,sizeof (local_rec.trueEnd));
}

void
RunProgram (
 char *programName)
{
	sprintf (local_rec.lp_str,"%d",local_rec.printerNumber);
	sprintf (local_rec.marg_pc,"%6.2f",local_rec.margin);
	strcpy (local_rec.s_date,DateToString (local_rec.start_dt));
	strcpy (local_rec.e_date,DateToString (local_rec.end_dt));
	local_rec.type [1] = (char) NULL;
	local_rec.all_br [1] = (char) NULL;
	local_rec.stat_flag [1] = (char) NULL;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	if (local_rec.onight [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				programName,
				local_rec.lp_str,
				local_rec.group_by,
				local_rec.trueStart,
				local_rec.trueEnd,
				local_rec.marg_pc,
				local_rec.type,
				local_rec.s_date,
				local_rec.e_date,
				local_rec.all_br,
				local_rec.stat_flag,
				local_rec.prog_desc, (char *)0);
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (programName,
				programName,
				local_rec.lp_str,
				local_rec.group_by,
				local_rec.trueStart,
				local_rec.trueEnd,
				local_rec.marg_pc,
				local_rec.type,
				local_rec.s_date,
				local_rec.e_date,
				local_rec.all_br,
				local_rec.stat_flag, (char *)0);
	}
	else 
	{
		execlp (programName,
			programName,
			local_rec.lp_str,
			local_rec.group_by,
			local_rec.trueStart,
			local_rec.trueEnd,
			local_rec.marg_pc,
			local_rec.type,
			local_rec.s_date,
			local_rec.e_date,
			local_rec.all_br,
			local_rec.stat_flag, (char *)0);
	}
}

int
spec_valid (
 int field)
{
	if (LCHECK ("gp_by"))
	{
#ifndef GVISION
		rv_pr ("                  ", 20, 5, 0);
		rv_pr ("                  ", 20, 6, 0);
#endif	/* GVISION */
		if (BY_ITEM)
		{
			strcpy (local_rec.group_by, "I");
			strcpy (local_rec.group_by_desc, ML ("Inventory"));
			strcpy (local_rec.lower, "                ");
			strcpy (local_rec.upper, "~~~~~~~~~~~~~~~~");
			memset ((char *)local_rec.trueStart,0,sizeof (local_rec.trueStart));
			memset ((char *)local_rec.trueEnd,0xff,sizeof (local_rec.trueEnd));
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			FLD ("item_upper") = YES;
			FLD ("item_lower") = YES;
			FLD ("cust_upper") = ND;
			FLD ("cust_lower") = ND;
			display_prmpt (label ("item_lower"));
			display_prmpt (label ("item_upper"));
			DSP_FLD ("item_lower");
			DSP_FLD ("item_upper");
		}
		else
		{
			strcpy (local_rec.group_by, "C");
			strcpy (local_rec.group_by_desc, ML ("Customer "));
			memset ((char *)local_rec.trueStart,0,sizeof (local_rec.trueStart));
			memset ((char *)local_rec.trueEnd,0xff,sizeof (local_rec.trueEnd));
			strcpy (local_rec.lower, "         ");
			strcpy (local_rec.upper, "~~~~~~~~~");
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			FLD ("item_upper") = ND;
			FLD ("item_lower") = ND;
			FLD ("cust_upper") = YES;
			FLD ("cust_lower") = YES;
			display_prmpt (label ("cust_lower"));
			display_prmpt (label ("cust_upper"));
			DSP_FLD ("cust_lower");
			DSP_FLD ("cust_upper");
		}

		DSP_FLD ("gp_by_desc");
		DSP_FLD ("st_desc");
		DSP_FLD ("ed_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_lower") || LCHECK ("cust_lower"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (BY_ITEM)
		{
			if (SRCH_KEY)
			{
				InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
				return (EXIT_SUCCESS);
			}


			cc = FindInmr (comm_rec.co_no, local_rec.lower, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.lower);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			strcpy (local_rec.st_desc,inmr_rec.description);

			DSP_FLD ("st_desc");
			if (prog_status != ENTRY && strcmp (local_rec.lower,local_rec.upper) > 0)
			{
				print_mess (ML (mlStdMess006));
				return (EXIT_FAILURE);
			}
			SuperSynonymError ();
			sprintf (local_rec.trueStart, "%-16.16s", local_rec.lower);
			return (cc);
		}
		else
		if (BY_CUST)
		{
			if (SRCH_KEY)
			{
				SrchCumr (temp_str,"         ");
				return (EXIT_SUCCESS);
			}

			if (prog_status != ENTRY && strcmp (local_rec.lower,local_rec.upper) > 0)
			{
				/*Lower Bound <=  Upper Bound*/
				print_mess (ML (mlStdMess006));
				return (EXIT_FAILURE);
			}

			strcpy (cumr_rec.co_no,comm_rec.co_no);
			sprintf (cumr_rec.dbt_acronym,"%-9.9s",local_rec.lower);
			cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess021));
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.trueStart, "%-9.9s", local_rec.lower);
			strcpy (local_rec.st_desc, cumr_rec.dbt_name);
			DSP_FLD ("st_desc");
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("item_upper") || LCHECK ("cust_upper"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (BY_ITEM)
		{
			if (SRCH_KEY)
			{
				InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
				return (EXIT_SUCCESS);
			}

			cc = FindInmr (comm_rec.co_no, local_rec.upper, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.upper);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			strcpy (local_rec.ed_desc,inmr_rec.description);
			DSP_FLD ("ed_desc");
			if (strcmp (local_rec.lower,local_rec.upper) > 0)
			{
				print_mess (ML (mlStdMess006));
				return (EXIT_FAILURE);
			}
			SuperSynonymError ();
			sprintf (local_rec.trueEnd, "%-9.9s", local_rec.upper);
			return (cc);
		}
		else
		if (BY_CUST)
		{
			if (SRCH_KEY)
			{
				SrchCumr (temp_str,local_rec.lower);
				return (EXIT_SUCCESS);
			}

			if (strcmp (local_rec.lower,local_rec.upper) > 0)
			{
				/*Lower Bound <=  Upper Bound*/
				print_mess (ML (mlStdMess006));
				return (EXIT_FAILURE);
			}

			strcpy (cumr_rec.co_no,comm_rec.co_no);
			sprintf (cumr_rec.dbt_acronym,"%-9.9s",local_rec.upper);
			cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess (ML (mlStdMess021));
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.ed_desc, cumr_rec.dbt_name);
			DSP_FLD ("ed_desc");
			sprintf (local_rec.trueEnd, "%-9.9s", local_rec.upper);
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("st_dt") || LCHECK ("en_dt"))
	{
		if (local_rec.start_dt > local_rec.end_dt)
		{
			print_mess (ML ("Invalid Range."));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ab"))
	{
		strcpy (local_rec.type_desc, (local_rec.type [0] == 'A') ? "Above" : "Below");
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("all_br"))
	{
		strcpy (local_rec.all_br_desc, 
					(local_rec.all_br [0] == 'Y') ? ML ("Yes"): ML ("No "));
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("stat_flag"))
	{
		strcpy (local_rec.stat_flag_desc, 
			(local_rec.stat_flag [0] == 'A') ? ML ("After Posting ") : ML ("Before Posting"));
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back_desc, 
				(local_rec.back [0] == 'Y') ? ML ("Yes") : ML ("No "));
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight_desc, 
				(local_rec.onight [0] == 'Y') ? ML ("Yes") : ML ("No "));
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==========================================
| Search routine for customer master file. |
==========================================*/
void
SrchCumr (
	char *keyValue, 
	char *lowValue)
{
	work_open ();
	save_rec ("#Acronym","#Cust Name");
	strcpy (cumr_rec.co_no,comm_rec.co_no);
	sprintf (cumr_rec.dbt_acronym,"%-9.9s",keyValue);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strncmp (cumr_rec.dbt_acronym,keyValue,strlen (keyValue)) &&
				  !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		if (strcmp (cumr_rec.dbt_acronym,lowValue) >= 0)
		{
			cc = save_rec (cumr_rec.dbt_acronym, cumr_rec.dbt_name);
			if (cc)
				break;
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cumr_rec.co_no,comm_rec.co_no);
	sprintf (cumr_rec.dbt_acronym,"%-9.9s",temp_str);
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no4");
}

/*=======================
| Close database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (cumr);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
heading (
 int scn)
{
	int	y = (80 - strlen (local_rec.prog_desc)) / 2;

	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (local_rec.prog_desc,y,0,1);

	line_at (1,0,80);

	box (0,3,80,13);

	line_at (13,1,79);
	line_at (20,0,80);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
