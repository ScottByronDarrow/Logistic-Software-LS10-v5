/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (lrp_demand.i.c )                                  |
|  Program Desc  : (Input lpno & misc flags for Demand Forecast )     |
|                  (Report                                      )     |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 18/12/87         |
|---------------------------------------------------------------------|
|  Date Modified : (18/12/87)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (27/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/09/97)      | Modified  by  : Ana Marie Tario. |
|  Date Modified : (15/10/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (27/09/90) - General Update for New Scrgen. S.B.D. |
|                : (16/09/97) - Incorporated multilingual conversion. |
|                : (15/10/97) - Corrected mldb error.                 |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: _demand.i.c,v $
| Revision 5.5  2002/11/25 03:16:34  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.4  2002/07/17 09:57:21  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/11 02:03:10  cha
| SE-225. Updated to put delays in error messages.
|
| Revision 5.2  2001/08/09 09:29:42  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:22  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:33  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.18  2000/06/13 06:15:55  scott
| S/C LSANZ 16400
| Updated to allow for demand type '6' related to production issues.
| NOTE : Please see release notes on new search.
| sk_mrmaint, sk_delete, psl_sr_gen and sch.srsk must be installed/rebuilt.
| Updated as demand types off by one
|
| Revision 1.17  2000/06/13 05:29:03  scott
| S/C LSANZ 16400
| Updated to allow for demand type '6' related to production issues.
| NOTE : Please see release notes on new search.
| sk_mrmaint, sk_delete, psl_sr_gen and sch.srsk must be installed/rebuilt.
|
| Revision 1.16  2000/01/21 01:10:36  cam
| Changes for GVision compatibility.  Separated description fields from input
| fields.
|
| Revision 1.15  1999/12/10 04:09:20  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.14  1999/12/06 01:34:15  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/17 06:40:09  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/11/03 00:22:13  scott
| Updated to change environment FF_ to LRP_
|
| Revision 1.11  1999/10/27 07:32:57  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.10  1999/10/13 21:32:55  scott
| General cleanup after ansi project
|
| Revision 1.9  1999/09/29 10:10:41  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:26:31  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 09:20:38  scott
| Updated from Ansi Project
|
| Revision 1.6  1999/06/16 04:12:30  scott
| Updated for possible cause why some items are not updated.
|
| Revision 1.5  1999/06/15 07:27:02  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _demand.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_demand.i/_demand.i.c,v 5.5 2002/11/25 03:16:34 scott Exp $";

#include <pslscr.h>
#include <ml_lrp_mess.h>
#include <ml_std_mess.h>

#undef	TABLINES
int	TABLINES = 10;

#include <get_lpno.h>

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;

	char	programName [140];

/*============================ 
| Local & Screen Structures. |
============================*/
	char	val_methods [5];
	int		num_methods;

struct {
	char	dummy [11];
	int		printerNumber;
	char	printerString [3];
	char	noMonths [3];
	char	background [2];
	char	background_desc [6];
	char	overNight [2];
	char	overNightDesc [6];
	char	startClass [2];
	char	startCat [12];
	char	endClass [2];
	char	endCat [12];
	char	startGroup [13];
	char	endGroup [13];
	char	print [2];
	char	print_desc [10];
	char	update [2];
	char	update_desc [10];
	char	manual [2];
	char	manual_desc [10];
	char	abcCodes [5];
	char	selected [2];
	char	selected_desc [10];
	char	forecastMethods [6];
	char	method [2];
	int		historyMonths;
	char	TransfersOK [2];
	char	TransfersOKDesc [10];
	char	LostSalesOK [2];
	char	LostSalesOKDesc [10];
	char	pcIssues [2];
	char	pcIssuesDesc [10];
	char	demandIncluded [6];
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "startClass",	 3, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class                  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startClass},
	{1, LIN, "startCat",	 3, 40, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category            ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startCat},
	{1, LIN, "endClass",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "End Class                    ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endClass},
	{1, LIN, "endCat",	 4, 40, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category              ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, LIN, "update",	6, 2, CHARTYPE,
		"U", "          ",
		" ", "Y", "Update Method/Demand         ", " Update Forecast Method & Weekly Demand - Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.update},
	{1, LIN, "update_desc",	6, 34, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.update_desc},
	{1, LIN, "manual",	6, 40, CHARTYPE,
		"U", "          ",
		" ", "B", "Forecast Option           ", " Items with Forecast Option M (anual), A(utomatic) or B(oth) ",
		YES, NO, JUSTRIGHT, "MAB", "", local_rec.manual},
	{1, LIN, "manual_desc",	6, 69, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.manual_desc},
	{1, LIN, "forecastMethods",	7, 2, CHARTYPE,
		"UUUU", "          ",
		" ", val_methods, "Forecast Methods             ", val_methods,
		YES, NO, JUSTLEFT, val_methods, "", local_rec.forecastMethods},
	{1, LIN, "abcCodes",	7, 40, CHARTYPE,
		"UUUU", "          ",
		" ", "ABCD", "ABC Codes                 ", " Include Items with these ABC Codes (Any Combination of A/B/C/D) ",
		YES, NO, JUSTLEFT, "ABCD", "", local_rec.abcCodes},
	{1, LIN, "historyMonths",	8, 2, INTTYPE,
		"NN", "          ",
		" ", "36",   "History Months Used          ", "Enter Number of History Months Used in Calculations ",
		YES, NO, JUSTRIGHT, "1", "36", (char *)&local_rec.historyMonths},
	{1, LIN, "transfersok",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Demand to include transfers  ", "Enter Y(es to include transfers, N(o) to exclude transfers.",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.TransfersOK},
	{1, LIN, "transfersok_desc",	9, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.TransfersOKDesc},
	{1, LIN, "lostsalesok",	10, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Demand to include lost sales ", "Enter Y(es to include lost sales, N(o) to exclude lost sales.",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.LostSalesOK},
	{1, LIN, "lostsalesok_desc",	10, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.LostSalesOKDesc},
	{1, LIN, "pcIssues",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Demand to include Prod.Iss.  ", "Enter Y(es to include production issues, N(o) to exclude production issues.",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.pcIssues},
	{1, LIN, "pcIssuesDesc",	11, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.pcIssuesDesc},
	{1, LIN, "print",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N",    "Print Report                 ", " Print Report Y(es) / N(o) ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.print},
	{1, LIN, "print_desc",	13, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.print_desc},
	{1, LIN, "selected",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "B",    "Print Method                 ", " A(ll) or B(est fit only) ",
		YES, NO, JUSTRIGHT, "AB", "", local_rec.selected},
	{1, LIN, "selected_desc",	14, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.selected_desc},
	{1, LIN, "printerNumber",		 16, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number               ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNumber},
	{1, LIN, "background",		 17, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background                   ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.background},
	{1, LIN, "background_desc",	17, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.background_desc},
	{1, LIN, "overNight",	 17, 40, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight                 ", " Y(es N(o ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.overNight},
	{1, LIN, "overNightDesc",	17, 69, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.overNightDesc},
	{0, LIN, "",		 0,  0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

extern	int	TruePosition;
extern	int	EnvScreenOK;

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void LoadDefault (int);
int spec_valid (int);
void OpenDB (void);
void CloseDB (void);
void SrchExcf (char *key_val);
int heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	if (argc != 3) 
	{
		print_at (0,0,mlStdMess037,argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("LRP_METHODS");
	if (sptr == (char *) NULL)
	{
		print_at (0,0,mlLrpMess032);
		return (EXIT_FAILURE);
	}
	else
	{
		sprintf (val_methods,"%-4.4s", sptr);
		num_methods = strlen (val_methods);
	}

	strcpy (programName, argv [2]);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/

	LoadDefault (1);
	heading (1);

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		scn_display (1);
		edit (1);
		if (restart)
        {
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		prog_exit = 1;
	}
	
	strcpy (err_str,ML (mlStdMess035));
	shutdown_prog ();

	sprintf (local_rec.startGroup,"%1.1s%-11.11s",
										local_rec.startClass,
										local_rec.startCat);
	sprintf (local_rec.endGroup,"%1.1s%-11.11s",
										local_rec.endClass,
										local_rec.endCat);
	sprintf (local_rec.printerString,    "%d", local_rec.printerNumber);
	sprintf (local_rec.noMonths, "%02d", local_rec.historyMonths);
		
	strcpy (local_rec.demandIncluded, "1");

	if (local_rec.TransfersOK [0] == 'Y')
		strcat (local_rec.demandIncluded, "2");

	if (local_rec.LostSalesOK [0] == 'Y')
		strcat (local_rec.demandIncluded, "3");

	if (local_rec.pcIssues [0] == 'Y')
		strcat (local_rec.demandIncluded, "4");

	clear ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.overNight [0] == 'Y') 
	{
		print_at (0,0,err_str);
		fflush (stdout);

		execlp ("ONIGHT",
			    "ONIGHT",
			    argv [1],
			    local_rec.printerString,
			    local_rec.startGroup,
			    local_rec.endGroup,
			    local_rec.print,
			    local_rec.update,
			    local_rec.manual,
			    local_rec.selected,
			    local_rec.forecastMethods,
			    local_rec.abcCodes,
			    local_rec.noMonths,
			    local_rec.demandIncluded,
			    argv [2], (char *) 0);
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.background [0] == 'Y') 
	{
		if (fork () != 0)
		{
			clear ();
			print_at (0,0,err_str);
			fflush (stdout);
		}
		else
			execlp (argv [1],
				    argv [1],
				    local_rec.printerString,
				    local_rec.startGroup,
				    local_rec.endGroup,
				    local_rec.print,
				    local_rec.update,
				    local_rec.manual,
				    local_rec.selected,
				    local_rec.forecastMethods,
				    local_rec.abcCodes,
				    local_rec.noMonths,
				    local_rec.demandIncluded,
				    (char *) 0);
	}
	else 
	{
		clear ();
		print_at (0,0,err_str);
		fflush (stdout);

		execlp (argv [1],
                argv [1],
                local_rec.printerString,
                local_rec.startGroup,
                local_rec.endGroup,
                local_rec.print,
                local_rec.update,
                local_rec.manual,
                local_rec.selected,
                local_rec.forecastMethods,
                local_rec.abcCodes,
                local_rec.noMonths,
                local_rec.demandIncluded,
                (char *) 0);
	}
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
LoadDefault (
 int    scn)
{
	init_vars (scn);
	init_ok = 1;

	scn_set (scn);

	strcpy (local_rec.startClass,		"A");
	strcpy (local_rec.startCat,			"           ");
	strcpy (local_rec.endClass,			"Z");
	strcpy (local_rec.endCat,			"~~~~~~~~~~~");
	strcpy (local_rec.update,			"N");
	strcpy (local_rec.update_desc,		"No");
	strcpy (local_rec.print,			"Y");
	strcpy (local_rec.print_desc,		"Yes");
	strcpy (local_rec.manual,			"B");
	strcpy (local_rec.manual_desc,		"Both");
	strcpy (local_rec.forecastMethods, clip (val_methods));
	strcpy (local_rec.abcCodes,			"ABCD");
	local_rec.historyMonths 			= 36;
	strcpy (local_rec.selected,			"B");
	strcpy (local_rec.selected_desc,	"Best");
	local_rec.printerNumber 			= 1;
	strcpy (local_rec.background,		"N");
	strcpy (local_rec.background_desc,	"No");
	strcpy (local_rec.overNight,		"N");
	strcpy (local_rec.overNightDesc,	"No ");
	strcpy (local_rec.TransfersOK, 		"N");
	strcpy (local_rec.TransfersOKDesc, 	"No");
	strcpy (local_rec.LostSalesOK, 		"N");
	strcpy (local_rec.LostSalesOKDesc, 	"No");
	strcpy (local_rec.pcIssues, 		"N");
	strcpy (local_rec.pcIssuesDesc, 	"No");
	init_ok = 0;
}

int
spec_valid (
 int    field)
{
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

	if (LCHECK ("background"))
	{
		if (local_rec.background [0] == 'Y')
		{
			strcpy (local_rec.background_desc,"Yes");
			strcpy (local_rec.overNight,"N");
			strcpy (local_rec.overNightDesc,"No");
			DSP_FLD ("overNight");
			DSP_FLD ("overNightDesc");
		}
		else
			strcpy (local_rec.background_desc,"No");

		DSP_FLD ("background_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("overNight"))
	{
		if (local_rec.overNight [0] == 'Y')
		{
			strcpy (local_rec.overNightDesc,"Yes");
			strcpy (local_rec.background,"N");
			strcpy (local_rec.background_desc,"No");
			DSP_FLD ("background");
			DSP_FLD ("background_desc");
		}
		else
			strcpy (local_rec.overNightDesc,"No");
		DSP_FLD ("overNightDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startCat,"           ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.startCat);
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

	if (LCHECK ("endCat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endCat,"~~~~~~~~~~~");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.endCat);
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

	if (LCHECK ("print"))
	{
		if (local_rec.print [0] == 'Y')
			strcpy (local_rec.print_desc,"Yes");
		else
		{
			strcpy (local_rec.print,"N");
			strcpy (local_rec.print_desc,"No");
			strcpy (local_rec.update,"Y");
			strcpy (local_rec.update_desc,"Yes");
			DSP_FLD ("update");
			DSP_FLD ("update_desc");
		}
		DSP_FLD ("print");
		DSP_FLD ("print_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("manual"))
	{
		if (local_rec.manual [0] == 'M')
			strcpy (local_rec.manual_desc,"Manual");
		else
		{
			if (local_rec.manual [0] == 'A')
				strcpy (local_rec.manual_desc,"Auto");
			else
				strcpy (local_rec.manual_desc,"Both");
		}
		DSP_FLD ("manual_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("selected"))
	{
		strcpy (local_rec.selected_desc,
			 (local_rec.selected [0] == 'A') ? "All " : "Best");

		DSP_FLD ("selected_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("update"))
	{
		strcpy (local_rec.update_desc, (local_rec.update [0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("update_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("transfersok"))
	{
		strcpy (local_rec.TransfersOKDesc, (local_rec.TransfersOK [0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("transfersok_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lostsalesok"))
	{
		strcpy (local_rec.LostSalesOKDesc, (local_rec.LostSalesOK [0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("lostsalesok_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("pcIssues"))
	{
		strcpy (local_rec.pcIssuesDesc, (local_rec.pcIssues [0] == 'Y') ? "Yes" : "No");
		DSP_FLD ("pcIssuesDesc");
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

 	open_rec (excf,excf_list,EXCF_NO_FIELDS,"excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
    abc_fclose (excf);
	abc_dbclose ("data");
}

/*==================================
| Search for Category master file. |
==================================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category","#Description");
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
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "excf", "DBFIND");
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (programName,40 - strlen (programName)/2,0,1);
		move (0,1);
		line (80);

		if (scn == 1)
		{
			box (0,2,80,15);
			move (1,5);
			line (79);
			move (1,12);
			line (79);
			move (1,15);
			line (79);
		}

		move (0,19);
		line (80);
		print_at (20,1,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (21,1,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		move (0,22);
		line (80);
		move (1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
