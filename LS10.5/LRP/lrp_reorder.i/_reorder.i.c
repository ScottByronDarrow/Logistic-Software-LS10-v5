/*=====================================================================
|  Copyright (C) 1986 - 2000 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( lrp_reorder.i.c )                                |
|  Program Desc  : ( Input group And Class For Reorder Review     )   |
|                  ( Report                                       )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: _reorder.i.c,v $
| Revision 5.5  2002/07/17 09:57:23  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/11/08 10:30:39  scott
| Updated as report Summary/Detailed prompt has values in wrong place.
|
| Revision 5.3  2001/09/11 02:10:54  cha
| SE-227. Updated to put delays in error messages.
|
| Revision 5.2  2001/08/09 09:29:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:38  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/26 09:08:56  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:15:39  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:47  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.12  2000/07/10 01:52:30  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.11  2000/05/23 04:23:44  scott
| S/C LSANZ-16336 / LSDI-2922
| Updated to disallow summary and details options if database option selected.
| Options relate to print selection only.
|
| Revision 1.10  2000/01/21 00:08:42  cam
| Changes for GVision compatibility.  Separated descriptions fields from input
| fields.
|
| Revision 1.9  1999/12/22 03:40:11  scott
| Updated to change database name to data.
|
| Revision 1.8  1999/12/06 01:34:20  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.7  1999/11/17 06:40:15  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/10/27 07:33:02  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.5  1999/09/29 10:10:51  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/17 07:26:42  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.3  1999/09/16 09:20:46  scott
| Updated from Ansi Project
|
| Revision 1.2  1999/06/15 07:27:05  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _reorder.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_reorder.i/_reorder.i.c,v 5.5 2002/07/17 09:57:23 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_lrp_mess.h>

#define	OUTPUT_FILE	 (local_rec.output_dev [0] == 'D')

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct ffwkRecord	ffwk_rec;

	char	*prog_name;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	output_dev [2];
	char	output_dev_desc [10];
	int		printerNumber;
	char	printerString [3];
	char	back [2];
	char	back_desc [6];
	char	onight [2];
	char	onight_desc [6];
	char	start_class [2];
	char	start_cat [12];
	char	end_class [2];
	char	end_cat [12];
	char	s_group [13];
	char	s_desc [41];
	char	e_group [13];
	char	e_desc [41];
	char	filename [15];
	char	sort_key [2];
	char	sort_key_desc [15];
	char	all_items [2];
	char	all_items_desc [6];
	char	reorder_flag [2];
	char	reorder_flag_desc [6];
	float	conting;
	char	conting_str [7];
	char	summary [2];
	char	summary_desc [10];
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "output_dev",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "P", "Output file name               ", " P(rinter or D(atabase ",
		YES, NO, JUSTLEFT, "PD", "", local_rec.output_dev},
	{1, LIN, "output_dev_desc",	 3, 36, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.output_dev_desc},
	{1, LIN, "printerNumber",		 3, 50, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "back",		 4, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background                     ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "back_desc",	 4, 36, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",	 4, 50, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight      ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc",	 4, 68, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onight_desc},
	{1, LIN, "start_class",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class                    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.start_class},
	{1, LIN, "start_cat",	 7, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category                 ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.start_cat},
	{1, LIN, "end_class",	 8, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "End Class                      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_class},
	{1, LIN, "end_cat",	 9, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category                   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_cat},
	{1, LIN, "filename",	11, 2, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", "",  "Enter Database file name.      ", " For output to D(atabase ",
		NA, NO,  JUSTLEFT, "", "", local_rec.filename},
	{1, LIN, "sort_key",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "G", "Sort by                        ", " S(upplier G(roup I(tem Number ",
		YES, NO, JUSTRIGHT, "SGI", "", local_rec.sort_key},
	{1, LIN, "sort_key_desc",	 12, 36, CHARTYPE,
		"AAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.sort_key_desc},
	{1, LIN, "all_items",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "All Items                      ", " Process items with net requirement == 0.00 Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.all_items},
	{1, LIN, "all_items_desc",	 13, 36, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.all_items_desc},
	{1, LIN, "reorder_flag",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Include no reorder items       ", " Process items with reorder flag set to no.",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.reorder_flag},
	{1, LIN, "reorder_flag_desc",	 14, 36, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.reorder_flag_desc},
	{1, LIN, "conting",      15, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Additional Contingency         ", " Number of Weeks of Contingency ",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *)&local_rec.conting},
	{1, LIN, "summary",      16, 2, CHARTYPE,
		"U", "          ",
		" ", "D",    "S(ummary / D(etailed           ", " Enter S(ummary) or D(etailed)",
		YES, NO, JUSTRIGHT, "SD", "", local_rec.summary},
	{1, LIN, "summary_desc",	 16, 36, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.summary_desc},
	{0, LIN, "",		 0,  0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

extern	int	TruePosition;
extern	int	EnvScreenOK;

/*============================
| Local Function Prototypes. |
============================*/
void 	shutdown_prog 	(void);
void 	LoadDefault 	(void);
int 	spec_valid 		(int);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadMisc 		(void);
void 	SrchExcf 		(char *);
void 	SrchFfwk 		(char *);
int 	heading 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int        argc,
 char*      argv [])
{
	if (argc != 3) 
	{
		print_at (0,0,mlStdMess037,argv [0]);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	SETUP_SCR (vars);

	prog_name = argv [2];

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();
	ReadMisc ();

	/*=====================
	| Reset control flags |
	=====================*/
	init_vars (1);
	init_ok = FALSE;

	LoadDefault ();
	heading (1);

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		scn_display (1);
		edit (1);
		if (restart)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		if (OUTPUT_FILE &&
            !strcmp (local_rec.filename,"              "))
		{
			print_mess (ML (mlLrpMess038));
			continue;
		}
		prog_exit = TRUE;
	}
	
	rset_tty ();

	sprintf (local_rec.printerString,"%d",local_rec.printerNumber);
	sprintf (local_rec.conting_str,"%6.2f",local_rec.conting);
	sprintf (local_rec.s_group,"%1.1s%-11.11s",
				local_rec.start_class,
				local_rec.start_cat);
	sprintf (local_rec.e_group,"%1.1s%-11.11s",
				local_rec.end_class,
				local_rec.end_cat);

	clear ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight [0] == 'Y') 
	{
		print_at (0,0,mlStdMess035);
		fflush (stdout);
		execlp 
		(
			"ONIGHT",
			"ONIGHT",
			argv [1],
			local_rec.printerString,
			local_rec.conting_str,
			local_rec.output_dev,
			local_rec.filename,
			local_rec.s_group,
			local_rec.e_group,
			local_rec.sort_key,
			local_rec.all_items,
			local_rec.reorder_flag,
			local_rec.summary,
			argv [2], (char *) 0
		);
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y') 
	{
		if (fork () != 0)
		{
			clear ();
			print_at (0,0,mlStdMess035);
			fflush (stdout);
		}
		else
        {
			execlp 
			 (
				argv [1],
				argv [1],
				local_rec.printerString,
				local_rec.conting_str,
				local_rec.output_dev,
				local_rec.filename,
				local_rec.s_group,
				local_rec.e_group,
				local_rec.sort_key,
				local_rec.all_items,
				local_rec.reorder_flag,
				local_rec.summary,
				 (char *) 0
			);
		}
	}
	else 
	{
		clear ();
		print_at (0,0,mlStdMess035);
		fflush (stdout);

		execlp 
		(
			argv [1],
			argv [1],
			local_rec.printerString,
			local_rec.conting_str,
			local_rec.output_dev,
			local_rec.filename,
			local_rec.s_group,
			local_rec.e_group,
			local_rec.sort_key,
			local_rec.all_items,
			local_rec.reorder_flag,
			local_rec.summary,
			 (char *) 0
		);
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

void
LoadDefault (void)
{
	scn_set (1);

	strcpy (local_rec.output_dev,		"P");
	strcpy (local_rec.output_dev_desc,	"Printer");
	strcpy (local_rec.back,				"N");
	strcpy (local_rec.back_desc,		"No");
	strcpy (local_rec.onight,			"N");
	strcpy (local_rec.onight_desc,		"No");
  	strcpy (local_rec.start_class,		"A");
  	strcpy (local_rec.start_cat,		"           ");
  	strcpy (local_rec.end_class,		"A");
  	strcpy (local_rec.end_cat,			"~~~~~~~~~~~");
	sprintf (local_rec.filename,		"%-14.14s"," ");
	strcpy (local_rec.sort_key,			"G");
	strcpy (local_rec.sort_key_desc,	"Group");
	strcpy (local_rec.all_items,		"N");
	strcpy (local_rec.all_items_desc,	"No");
	strcpy (local_rec.reorder_flag,		"N");
	strcpy (local_rec.reorder_flag_desc,"No");
	strcpy (local_rec.summary,			"D");
	strcpy (local_rec.summary_desc,		"Detailed");
	local_rec.printerNumber 		= 1;
	local_rec.conting 	= 0.00;
}

int
spec_valid (
 int    field)
{
	/*----------------------
	| Validate output dev  |
	----------------------*/
	if (LCHECK ("output_dev")) 
	{
		if (OUTPUT_FILE)
		{
			FLD ("printerNumber")		=	NA;
			FLD ("filename")	= 	YES;
			FLD ("summary")		=	NA;
			strcpy (local_rec.output_dev,"D");
			strcpy (local_rec.output_dev_desc,"Database");
		}
		else
		{
			FLD ("printerNumber")		= 	YES;
			FLD ("filename")	= 	NA;
			FLD ("summary")		=	YES;
			strcpy (local_rec.output_dev,"P");
			strcpy (local_rec.output_dev_desc,"Printer ");
		}
		DSP_FLD ("output_dev_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_cat")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.start_cat,"           ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.start_cat);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_cat")) 
	{
		if (dflt_used)
		{
			strcpy (local_rec.end_cat,"~~~~~~~~~~~");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.end_cat);
		cc = find_rec (excf,&excf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back")) 
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.back_desc,	"Yes");
			strcpy (local_rec.onight,		"N");
			strcpy (local_rec.onight_desc,	"No");
			DSP_FLD ("onight");
			DSP_FLD ("onight_desc");
		}
		else
			strcpy (local_rec.back_desc,	"No");
		DSP_FLD ("back_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight")) 
	{
		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.onight_desc,	"Yes");
			strcpy (local_rec.back,			"N");
			strcpy (local_rec.back_desc,	"No");
			DSP_FLD ("onight_desc");
			DSP_FLD ("back");
			DSP_FLD ("back_desc");
		}
		else
			strcpy (local_rec.onight_desc,	"No");

		DSP_FLD ("onight_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sort_key")) 
	{
		switch (local_rec.sort_key [0])
		{
		case	'S':
			strcpy (local_rec.sort_key_desc,"Supplier   ");
			break;

		case	'G':
			strcpy (local_rec.sort_key_desc,"Group      ");
			break;

		case	'I':
			strcpy (local_rec.sort_key_desc,"Item Number");
		}
		DSP_FLD ("sort_key_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("filename")) 
	{
		if (SRCH_KEY)
			SrchFfwk (temp_str);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("all_items")) 
	{
		if (local_rec.all_items [0] == 'Y')
			strcpy (local_rec.all_items_desc,"Yes");
		else
			strcpy (local_rec.all_items_desc,"No");

		DSP_FLD ("all_items_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("reorder_flag")) 
	{
		if (local_rec.reorder_flag [0] == 'Y')
			strcpy (local_rec.reorder_flag_desc,"Yes");
		else
			strcpy (local_rec.reorder_flag_desc,"No");

		DSP_FLD ("reorder_flag_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("summary")) 
	{
		if (local_rec.summary [0] == 'S')
			strcpy (local_rec.summary_desc,"Summary ");
		else
			strcpy (local_rec.summary_desc,"Detailed");

		DSP_FLD ("summary_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

    open_rec (ffwk, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (ffwk);
	abc_fclose (excf);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc, PNAME);
	abc_fclose (ccmr);
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*      key_val)
{
	work_open ();
	save_rec ("#Category","#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
				  !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
}

/*===============================
| Search for Database filename	|
===============================*/
void
SrchFfwk (
 char*      key_val)
{
	char	last_name [15];

	work_open ();
	save_rec ("#Filename","# ");
	ffwk_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (ffwk_rec.filename,"%-14.14s",key_val);
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"r");
	strcpy (last_name,"              ");
	while (!cc && !strncmp (ffwk_rec.filename,key_val,strlen (key_val)) && 
		ffwk_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strcmp (last_name,ffwk_rec.filename))
		{
			cc = save_rec (ffwk_rec.filename," ");
			if (cc)
				break;
		}
		strcpy (last_name,ffwk_rec.filename);
		cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (prog_name,20,0,1);
	move (0,1);
	line (80);

	box (0,2,80,14);

	move (1,5);
	line (79);

	move (1,10);
	line (79);

	move (0,18);
	line (80);
	print_at (19,1,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	print_at (20,1,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
	print_at (21,1,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_name);
	move (0,22);
	line (80);
	move (1,input_row);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	
    return (EXIT_SUCCESS);
}
