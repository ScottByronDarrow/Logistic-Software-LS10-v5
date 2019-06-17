/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_cuqbrep.c,v 5.4 2001/09/19 03:22:31 robert Exp $
|  Program Name  : (sk_cuqbrep.c)
|  Program Desc  : (Customer Inventory Default Quantity Break (Report)
|---------------------------------------------------------------------|
|  Author        : Irfan Gohir.    | Date Written  : 20/10/93         |
|---------------------------------------------------------------------|
| $Log: sk_cuqbrep.c,v $
| Revision 5.4  2001/09/19 03:22:31  robert
| Updated to avoid overlapping of description
|
| Revision 5.3  2001/08/09 09:18:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:48  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:01  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_cuqbrep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cuqbrep/sk_cuqbrep.c,v 5.4 2001/09/19 03:22:31 robert Exp $";

#define	CCMAIN
#define	MOD		5
#define REPTITLE "Customer Inventory Default Quantity Break Report"
#define SLEEP_TIME	2

#include <pslscr.h>	
#include <signal.h>	
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>


#include	"schema"

struct commRecord	comm_rec;
struct ingpRecord	ingp_rec;
struct cuqbRecord	cuqb_rec;
struct excfRecord	excf_rec;

	float	*cuqb_qty_brk	=	&cuqb_rec.qty_brk1;

	FILE	*fout;
	FILE	*fsort;

	char	*data   = "data",
			*cuqb2	= "cuqb2",
			*cuqb3	= "cuqb3";

	int		numOfBreaks;
	int		numOfPrices;

struct
{
	char	range [2];
	char	updated [2];
	char	upddesc [5];
	char	rangedesc [14];
	char	scat [12];
	char	ecat [12];
	char	scatdesc [41];
	char	ecatdesc [41];
	char	ssellgrp [7];
	char	sselldesc [41];
	char	esellgrp [7];
	char	eselldesc [41];
	char	sbuygrp [7];
	char	sbuydesc [41];
	char	ebuygrp [7];
	char	ebuydesc [41];
	int		lpno;
	char	lp_str [3];
	char 	back [2];
	char 	backdesc [4];
	char	onite [2];
	char	onitedesc [4];
	char	dummy [11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "range",	4, 24, CHARTYPE,
		"U", "          ",
		" ", "S",  "Range Type          :", "Enter Range Type B)uying S)elling C)ategory, Default = S)elling",
		YES, NO, JUSTLEFT, "BSC", "", local_rec.range},
	{1, LIN, "rangedesc",	4, 34, CHARTYPE,
		"AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangedesc},
	{1, LIN, "updated",	5, 24, CHARTYPE,
		"U", "          ",
		" ", "A", "Updated             :", "Enter Y)es N)o A)ll Default = A)ll",
		YES, NO, JUSTLEFT, "YNA", "", local_rec.updated},
	{1, LIN, "upddesc",	5, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.upddesc},
	{1, LIN, "sbuygrp", 7, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Buying Group  :", "Enter Start Buying Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sbuygrp},
	{1, LIN, "sbuydesc",	7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sbuydesc},
	{1, LIN, "ebuygrp",	8, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Buying Group    :", "Enter End Buying Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ebuygrp},
	{1, LIN, "ebuydesc",	8, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ebuydesc},
	{1, LIN, "ssellgrp",	7, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Selling Group :", "Enter Start Selling Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssellgrp},
	{1, LIN, "sselldesc",	7, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sselldesc},
	{1, LIN, "esellgrp",	8, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Selling Group   :", "Enter End Selling Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esellgrp},
	{1, LIN, "eselldesc",	8, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eselldesc},
	{1, LIN, "scat",	7, 24, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ",  "Start Category      :", "Enter Start Category, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.scat},
	{1, LIN, "scatdesc",	7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.scatdesc},
	{1, LIN, "ecat",	8, 24, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ",  "End Category        :", "Enter End Category, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ecat},
	{1, LIN, "ecatdesc",	8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ecatdesc},
    {1, LIN, "lpno", 10, 24, INTTYPE, "NN", "          ", " ", "1",
	    "Printer number      :  ", " ", YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno},
    {1, LIN, "back", 11, 24, CHARTYPE, "U", "          ", " ", "N",
	    "Background          :  ", "Enter (Y)es or (N)o", YES, NO, JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backdesc",	11, 34, CHARTYPE,
		"AAA", "          ",
		" ", "No ", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backdesc},
    {1, LIN, "onite", 12,24, CHARTYPE, "U", "          ", " ", "N",
        "Overnight           :  ", "Enter (Y)es or (N)o", YES, NO, JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onitedesc",12, 34, CHARTYPE,
		"AAA", "          ",
		" ", "No ", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.onitedesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/* Global variables declared here */

char	repType [2];
char	*sptr;
char	group [12];
char	systemDate [11];
char	firstCol [15];
char	line1 [158];
char	line2 [158];
char	line3 [158];
char	col1 [200];
int		upperLimit;
int		lowerLimit;


/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	CheckNumPrices 		(void);
void 	CheckQtyBreaks 		(void);
void 	CopyArgs 			(char *, char *, char *, char *, char *);
void 	ProcessReport 		(void);
void 	ProcessByBuygrp 	(void);
void 	ProcessBySellgrp 	(void);
void 	ProcessByCat 		(void);
int  	spec_valid 			(int);
void 	SrchIngp 			(char *);
int  	heading 			(int);
void 	shutdown_prog 		(void);
void 	SrchExcf 			(char *);
void 	BuildSortFile 		(void);
void 	HeaderOutput 		(void);
void 	ReportPrint 		(void);
void 	SetReportCols 		(void);
void 	RunProgram 			(char *, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{

	if (argc != 2 && argc != 6)
	{
		print_at(0,0, mlSkMess210,argv[0]);
		print_at(0,0, mlSkMess604,argv[0]);
		return (EXIT_FAILURE);
	}


	SETUP_SCR (vars);

	CheckQtyBreaks ();

	if (numOfBreaks == 0)
	{
		print_at (0,0, mlSkMess306);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	CheckNumPrices ();

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();

	if (argc == 6)
	{
		CopyArgs (argv [1], argv [2], argv [3], argv [4], argv [5]);
		ProcessReport ();
		SetReportCols ();
		HeaderOutput ();
		ReportPrint ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	
	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
		init_vars (1);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
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
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (cuqb, cuqb_list, CUQB_NO_FIELDS, "cuqb_id_buygrp");
	abc_alias (cuqb2, cuqb);
	open_rec (cuqb2, cuqb_list, CUQB_NO_FIELDS, "cuqb_id_sellgrp");
	abc_alias (cuqb3, cuqb);
	open_rec (cuqb3, cuqb_list, CUQB_NO_FIELDS, "cuqb_id_cat");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ingp);	
	abc_fclose (cuqb);	
	abc_fclose (cuqb2);	
	abc_fclose (cuqb3);	
	abc_fclose (excf);	
	abc_dbclose (data);
}

void
CheckNumPrices (void)
{
	char	*eptr = chk_env ("SK_DBPRINUM");

	if (eptr)
	{
		numOfPrices = atoi (eptr);
		if (numOfPrices > 9 || numOfPrices < 1)
			numOfPrices = 9;
	}
	else
		numOfPrices = 5;
}

void
CheckQtyBreaks (void)
{
	char	*eptr = chk_env ("SK_DBQTYNUM");

	if (eptr)
	{
		numOfBreaks = atoi (eptr);
		if (numOfBreaks > 9 || numOfBreaks < 0)
			numOfBreaks = 9;
	}
	else
		numOfBreaks = 0;
}

void
CopyArgs (
 char *arg2, 
 char *arg3, 
 char *arg4, 
 char *arg5, 
 char *arg6)
{
	strncpy (local_rec.range, arg2, 1);
	strncpy (local_rec.updated, arg3, 1);
	switch	(toupper(local_rec.range [0]))
	{
	case	'B':
		strncpy (local_rec.sbuygrp, arg4, 6);
		strncpy (local_rec.ebuygrp, arg5, 6);
		break;
	case	'C':
		strncpy (local_rec.scat, arg4, 11);
		strncpy (local_rec.ecat, arg5, 11);
		break;
	case	'S':
		strncpy (local_rec.ssellgrp, arg4, 6);
		strncpy (local_rec.esellgrp, arg5, 6);
		break;
	}
	sprintf (local_rec.lp_str,"%-2.2s", arg6);
}

void
ProcessReport (void)
{

	fsort = sort_open ("datfile");
	
	switch	(toupper (local_rec.range [0]))
	{
	case	'B':
		sprintf (firstCol, "Buying Group");
		ProcessByBuygrp ();
		break;
	case	'S':
		sprintf (firstCol, "Selling Group");
		ProcessBySellgrp ();
		break;
	case	'C':
		sprintf (firstCol, "Category");
		ProcessByCat ();
		break;
	}
	fsort = sort_sort (fsort, "datfile");
	sptr = sort_read (fsort);
}

void
ProcessByBuygrp (void)
{
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strncpy (cuqb_rec.buygrp, local_rec.sbuygrp, 6);

	cuqb_rec.price_type = 0;

	cc = find_rec (cuqb, &cuqb_rec, FIRST, "r");

	dsp_screen ("Processing : Customer Quantity Break Report.",
				comm_rec.co_no,comm_rec.co_name);

	while (!cc) 
	{
		upperLimit = strcmp (cuqb_rec.buygrp, local_rec.sbuygrp);
		lowerLimit = strcmp (cuqb_rec.buygrp, local_rec.ebuygrp);
		
		if (strlen (clip (cuqb_rec.buygrp)) > 0 &&
		   !strcmp (cuqb_rec.co_no, comm_rec.co_no) &&
			upperLimit >= 0 && lowerLimit <= 0 &&
			(!strncmp (cuqb_rec.update_flag, local_rec.updated, 1) ||
			 !strncmp (local_rec.updated, "A", 1)))
		{
			dsp_process ("Buying Group :",cuqb_rec.buygrp);
			strncpy (group, cuqb_rec.buygrp, 6);
			BuildSortFile ();
		}
		cc = find_rec (cuqb, &cuqb_rec, NEXT, "r");
	}
}

void
ProcessBySellgrp (
 void)
{
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strncpy (cuqb_rec.sellgrp, local_rec.ssellgrp, 6);

	cuqb_rec.price_type = 0;

	cc = find_rec (cuqb2, &cuqb_rec, FIRST, "r");

	dsp_screen ("Processing : Customer Quantity Break Report.",
				comm_rec.co_no,comm_rec.co_name);

	while (!cc) 
	{
		upperLimit = strcmp (cuqb_rec.sellgrp, local_rec.ssellgrp);
		lowerLimit = strcmp (cuqb_rec.sellgrp, local_rec.esellgrp);
		
		if (strlen (clip (cuqb_rec.sellgrp)) > 0 &&
		   !strcmp (cuqb_rec.co_no, comm_rec.co_no) &&
			upperLimit >= 0 && lowerLimit <= 0 &&
			(!strncmp (cuqb_rec.update_flag, local_rec.updated, 1) ||
			 !strncmp (local_rec.updated, "A", 1)))
		{
			dsp_process ("Selling Group :",cuqb_rec.sellgrp);
			strncpy (group, cuqb_rec.sellgrp, 6);
			BuildSortFile ();
		}
		cc = find_rec (cuqb2, &cuqb_rec, NEXT, "r");
	}

}

void
ProcessByCat (
 void)
{
	strcpy (cuqb_rec.co_no, comm_rec.co_no);
	strcpy (cuqb_rec.category, local_rec.scat);
	cuqb_rec.price_type = 0;
	cc = find_rec (cuqb3, &cuqb_rec, GTEQ, "r");

	dsp_screen ("Processing : Customer Quantity Break Report.",
				comm_rec.co_no,comm_rec.co_name);

	while (!cc) 
	{
		upperLimit = strcmp (cuqb_rec.category, local_rec.scat);
		lowerLimit = strcmp (cuqb_rec.category, local_rec.ecat);
		
		if (strlen (clip (cuqb_rec.category)) > 0 &&
		   !strcmp (cuqb_rec.co_no, comm_rec.co_no) &&
			upperLimit >= 0 && lowerLimit <= 0 &&
			(!strncmp (cuqb_rec.update_flag, local_rec.updated, 1) ||
			 !strncmp (local_rec.updated, "A", 1)))
		{
			dsp_process ("Category :",cuqb_rec.category);
			strcpy (group, cuqb_rec.category);
			BuildSortFile ();
		}
		cc = find_rec (cuqb3, &cuqb_rec, NEXT, "r");
	}

}

int
spec_valid (
 int field)
{

	/*--------------------------
	| if range entered was by buying group
	---------------------------*/
	if (LCHECK ("sbuygrp"))
	{
		if (FLD ("sbuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.sbuygrp, "      ");
			strcpy (ingp_rec.desc, "Start Of File");
			
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, local_rec.sbuygrp);
			strcpy (ingp_rec.type, "B");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] == 'S')
				{
					print_mess (ML(mlSkMess605));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess (ML(mlStdMess207));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sbuydesc, ingp_rec.desc);
		DSP_FLD ("sbuydesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ebuygrp"))
	{
		if (FLD ("ebuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.ebuygrp, "~~~~~~");
			strcpy (ingp_rec.desc, ML ("End Of File"));
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, local_rec.ebuygrp);
			strcpy (ingp_rec.type, "B");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] == 'S')
				{
					print_mess (ML(mlSkMess605));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess (ML(mlStdMess207));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ebuydesc, ingp_rec.desc);
		DSP_FLD ("ebuydesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| selling group
	---------------------------*/
	if (LCHECK ("ssellgrp"))
	{
		if (FLD ("ssellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.ssellgrp, "      ");
			strcpy (ingp_rec.desc, ML ("Start Of File"));
			
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, local_rec.ssellgrp);
			strcpy (ingp_rec.type, "S");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] != 'S')
				{
					print_mess (ML(mlStdMess234));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess (ML(mlStdMess208));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sselldesc, ingp_rec.desc);
		DSP_FLD ("sselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esellgrp"))
	{
		if (FLD ("esellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.esellgrp, "~~~~~~");
			strcpy (ingp_rec.desc, "End Of File");
		}
		else
		{
			strcpy (ingp_rec.co_no, comm_rec.co_no);
			strcpy (ingp_rec.code, local_rec.esellgrp);
			strcpy (ingp_rec.type, "S");

			cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
			
			if (!cc)
			{
				if (ingp_rec.type[0] != 'S')
				{
					print_mess (ML(mlStdMess234));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (cc)
			{
				print_mess (ML(mlStdMess208));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.eselldesc, ingp_rec.desc);
		DSP_FLD ("eselldesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| category 
	---------------------------*/
	if (LCHECK ("scat"))
	{
		if (FLD ("scat") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.scat, "           ");
			strcpy (excf_rec.cat_desc, "Start Of File");
			
		}
		else
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			strcpy (excf_rec.cat_no, local_rec.scat);

			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			
			if (cc)
			{
				print_mess (ML(mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.scat, local_rec.ecat) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.scatdesc, excf_rec.cat_desc);
		DSP_FLD ("scatdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ecat"))
	{
		if (FLD ("ecat") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		
		if (dflt_used)
		{
			strcpy (local_rec.ecat, "~~~~~~~~~~~");
			strcpy (excf_rec.cat_desc, "End Of File");
		}
		else
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			strcpy (excf_rec.cat_no, local_rec.ecat);

			cc = find_rec (excf, &excf_rec, EQUAL, "r");
			
			if (cc)
			{
				print_mess (ML(mlStdMess004));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (strcmp (local_rec.scat, local_rec.ecat) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ecatdesc, excf_rec.cat_desc);
		DSP_FLD ("ecatdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("range"))
	{
		FLD ("sbuygrp")   = ND;
		FLD ("ebuygrp")   = ND;
		FLD ("sbuydesc")  = ND;
		FLD ("ebuydesc")  = ND;
		FLD ("ssellgrp")  = ND;
		FLD ("esellgrp")  = ND;
		FLD ("sselldesc") = ND;
		FLD ("eselldesc") = ND;
		FLD ("scat")      = ND;
		FLD ("scatdesc")  = ND;
		FLD ("ecat")      = ND;
		FLD ("ecatdesc")  = ND;

#ifndef GVISION
		print_at (7, 2, "%70.70s", " ");
		print_at (8, 2, "%70.70s", " ");
#endif	/* GVISION */

		/*-------------------------
		| if choice is buying group
		--------------------------*/
		if (local_rec.range[0] == 'B')
		{
			FLD ("sbuygrp")  = YES;
			FLD ("ebuygrp")  = YES;
			FLD ("sbuydesc") = NA;
			FLD ("ebuydesc") = NA;
			strcpy (local_rec.rangedesc, "Buying Group");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}

		/*-------------------------
		| if choice is selling group
		--------------------------*/
		if (local_rec.range[0] == 'S')
		{
			FLD ("ssellgrp")  = YES;
			FLD ("esellgrp")  = YES;
			FLD ("sselldesc") = NA;
			FLD ("eselldesc") = NA;
			strcpy (local_rec.rangedesc, "Selling Group");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}

		/*-------------------------
		| if choice is category
		--------------------------*/
		if (local_rec.range[0] == 'C')
		{
			FLD ("scat")  = YES;
			FLD ("ecat")  = YES;
			FLD ("scatdesc")  = NA;
			FLD ("ecatdesc")  = NA;
			strcpy (local_rec.rangedesc, "Category");
			scn_write (1);
			DSP_FLD ("rangedesc");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("updated"))
	{
		if (!strncmp (local_rec.updated, "A", 1))
			strcpy (local_rec.upddesc, "All");
		else
			strcpy (local_rec.upddesc,
				(local_rec.updated [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("upddesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			if (!valid_lp (local_rec.lpno))
			{
				print_mess (ML(mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	sprintf (local_rec.lp_str, "%d", local_rec.lpno);

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backdesc,
			(local_rec.back [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("backdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		strcpy (local_rec.onitedesc,
			(local_rec.onite [0] == 'Y') ? "Yes" : "No ");
		DSP_FLD ("onitedesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchIngp (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	if (local_rec.range[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type [0] == 'S' && local_rec.range [0] == 'S')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (ingp_rec.type [0] == 'B' && local_rec.range [0] == 'B')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	if (local_rec.range[0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
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

	rv_pr (ML(mlSkMess606), 20, 0, 1);

	box (0, 3, 80, 9);

	move (1, 6);
	line (79);
	move (1, 9);
	line (79);
	move (0, 21);
	line (80);

	/*(22, 1, "Co : %s - %s", comm_rec.co_no, comm_rec.co_name);*/

	print_at (22, 1, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	move (0,1);
	line (80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}


void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
SrchExcf (
 char *key_val)
{
	work_open ();
	save_rec ("#Category No.    ", "#Category Description    ");

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");

	while (!cc && !strcmp (excf_rec.co_no, comm_rec.co_no) && 
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
	sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
	cc = find_rec (excf, &excf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
BuildSortFile (void)
{
	char	dataStr [250];
	char	qtybr [12];
	int		count;

	if (cuqb_rec.price_type > numOfPrices)
		return;

	sprintf (dataStr, "%-13.13s ", group);

	switch	(cuqb_rec.price_type)
	{
		case	1:
			strcat (dataStr, comm_rec.price1_desc);
		break;

		case	2:
			strcat (dataStr, comm_rec.price2_desc);
		break;

		case	3:
			strcat (dataStr, comm_rec.price3_desc);
		break;

		case	4:
			strcat (dataStr, comm_rec.price4_desc);
		break;

		case	5:
			strcat (dataStr, comm_rec.price5_desc);
		break;

		case	6:
			strcat (dataStr, comm_rec.price6_desc);
		break;

		case	7:
			strcat (dataStr, comm_rec.price7_desc);
		break;

		case	8:
			strcat (dataStr, comm_rec.price8_desc);
		break;

		case	9:
			strcat (dataStr, comm_rec.price9_desc);
		break;
	}

	for (count = 0; count < numOfBreaks; count++)
	{
		sprintf (qtybr, "%10.2f ", cuqb_qty_brk [count]);
		strncat (dataStr, qtybr, 11);
	}
	
	strncat (dataStr, cuqb_rec.update_flag, 1);
	strcat (dataStr, "\n");

    sort_save (fsort, dataStr);
}

void
HeaderOutput (
 void)
{
	if ((fout = popen("pformat","w")) == (FILE *)NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.lpno);

	fprintf (fout,".17\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",REPTITLE);
	fprintf (fout,".B1\n");
	fprintf (fout,".ECOMPANY   %s : %s\n",comm_rec.co_no,clip(comm_rec.co_name));
	fprintf (fout,".EBRANCH    %s : %s\n",comm_rec.est_no,clip(comm_rec.est_name));
	fprintf (fout,".EWAREHOUSE %s : %s\n",comm_rec.cc_no,clip(comm_rec.cc_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %s\n",SystemTime());
	fprintf (fout,".B1\n");
	switch	(toupper(local_rec.range[0]))
	{
	case	'B':
		fprintf (fout,".EStart Buying Group %s      End Buying Group %s\n",
						local_rec.sbuygrp, 
						local_rec.ebuygrp);
		fprintf (fout,".B1\n");
		fprintf (fout, line1);
		fprintf (fout,"\n");
		fprintf (fout, col1);
		fprintf (fout, line2);
		fprintf (fout,"\n");
		break;

	case	'C':
		fprintf (fout,".EStart Category %s     End Category %s\n",
						local_rec.scat, 
						local_rec.ecat);
		fprintf (fout,".B1\n");
		fprintf (fout, line1);
		fprintf (fout,"\n");
		fprintf (fout, col1);
		fprintf (fout, line2);
		fprintf (fout,"\n");
		break;

	case	'S':
		fprintf (fout,".EStart Selling Group %s     End Selling Group %s\n",
						local_rec.ssellgrp, 
						local_rec.esellgrp);
		fprintf (fout,".B1\n");
		fprintf (fout, line1);
		fprintf (fout,"\n");
		fprintf (fout, col1);
		fprintf (fout, line2);
		fprintf (fout,"\n");
	}

	fprintf (fout, line3);
	fprintf (fout,"\n");

}

void
ReportPrint (
 void)
{
	int 	i;
	char 	updated [2];
	char	this_group[13];
	char	prev_group[13];
	int		first_time = 1;

	while (sptr)
	{
		i = strlen (sptr) - 1;	
		sprintf (updated,	"%c",		sptr [i]);	 /* Get Updated Stat */
		sprintf (this_group,"%-13.13s",	sptr);
		if (!first_time && strcmp (prev_group, this_group))
		{
			fprintf (fout, "%s\n", line2);
		}
		fprintf (fout, 		"!%-13.13s",sptr);	     /* Get & Print the group */
		sptr+=14;
		fprintf (fout, 		"!%-15.15s",sptr);		 /* Get Price Type */
		sptr+=15;
		for (i = 0; i < numOfBreaks; i++)			 /* Get & Print Qty Brks */
		{
			fprintf (fout,"!%10.2f", atof (sptr)); 
			sptr+=11;
		}
		fprintf (fout,"!    %-1.1s   !\n", updated); /* Print Updated Stat */
		fprintf (fout, ".LRP%d\n", numOfPrices - 1);
		first_time = 0;
		strcpy  (prev_group, this_group);
		sptr = sort_read (fsort);
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
	sort_delete (fout, "datfile");
}

void
SetReportCols (
 void)
{
	int 	i;
	char	qtyCol [12];

	sprintf (line1, "===============================");
	sprintf (line2, "|-------------|---------------|");
	sprintf (line3, ".R===============================");
	switch	(toupper (local_rec.range [0]))
	{
	case	'B':
		sprintf (col1,  "|Buying Group |");
		break;
	case	'C':
		sprintf (col1,  "|   Category  |");
		break;
	case	'S':
		sprintf (col1,  "|Selling Group|");
	}
	strcat (col1, "  Price Type   |");

	for (i = 0; i < numOfBreaks; i++)
	{
		sprintf (qtyCol, " Qty Brk %d!", i+1);
		strcat (line1,"===========");
		strcat (line2,"----------|");
		strcat (line3,"===========");
		strcat (col1, qtyCol);
	}
	
	strcat (col1, "Updated |\n");
	strcat (line1,"=========");
	strcat (line2,"--------|");
	strcat (line3,"=========");
}

void
RunProgram (
 char *prog_name, 
 char *prog_desc)
{
	shutdown_prog ();

	switch	(toupper (local_rec.range [0]))
	{
	case	'B':
		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.sbuygrp,
					local_rec.ebuygrp,
					local_rec.lp_str,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp (prog_name,
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.sbuygrp,
					local_rec.ebuygrp,
					local_rec.lp_str, (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				local_rec.range,
				local_rec.updated,
				local_rec.sbuygrp,
				local_rec.ebuygrp,
				local_rec.lp_str, (char *)0);
		}
		break;

	case	'C':
		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.scat,
					local_rec.ecat,
					local_rec.lp_str,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp(prog_name,
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.scat,
					local_rec.ecat,
					local_rec.lp_str,(char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				local_rec.range,
				local_rec.updated,
				local_rec.scat,
				local_rec.ecat,
				local_rec.lp_str, (char *)0);
		}
		break;

	case	'S':
		if (local_rec.onite [0] == 'Y')
		{
			if (fork () == 0)
			{
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.ssellgrp,
					local_rec.esellgrp,
					local_rec.lp_str,
					prog_desc, (char *)0);
			}
		}
		else if (local_rec.back [0] == 'Y')
		{
			if (fork () == 0)
				execlp (prog_name,
					prog_name,
					local_rec.range,
					local_rec.updated,
					local_rec.ssellgrp,
					local_rec.esellgrp,
					local_rec.lp_str, (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				local_rec.range,
				local_rec.updated,
				local_rec.ssellgrp,
				local_rec.esellgrp,
				local_rec.lp_str, (char *)0);
		}
	}
    prog_exit = 1;
}

