/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_lists.i.c,v 5.3 2002/07/17 09:57:02 scott Exp $
|  Program Name  : (cr_lists.i.c)
|  Program Desc  : (Selection for debtors listings)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/01/87         |
|---------------------------------------------------------------------|
| $Log: cr_lists.i.c,v $
| Revision 5.3  2002/07/17 09:57:02  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:52:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:34  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_lists.i.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_lists.i/cr_lists.i.c,v 5.3 2002/07/17 09:57:02 scott Exp $";

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_cr_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;

/*==============
 Table names
===============*/
static char
	*data	= "data",
	*sumr2	= "sumr2";

/*======
 Globals
========*/
#define	SCREENWIDTH	80

	static char	branchNumber [3];

extern	int	TruePosition;
extern	int	EnvScreenOK;

/*============================
| Local & Screen Structures. |
============================*/
static struct
{
	int		printerNo;

	char	back 			[2],
			backDesc		[11],
			onight 			[2],
			onightDesc 		[11],
			listType		[2],
			listTypeDesc	[11],
			listBy 			[2],
			listByDesc		[11],
			sortBy 			[2],
			sortByDesc 		[11],
			yesWord			[11],
			noWord			[11],
			startSupplier 	[10],
			startAcronym 	[10],
			startName 		[41],
			endSupplier 	[10],
			endAcronym 		[10],
			endName 		[41],
			paymentMethod 	[2],
			paymentMethodDesc [11];

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "printerNo",	 3, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No                ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo},
	{1, LIN, "back",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background                ", "Enter Yes or No. ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 4, 32, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight                 ", "Enter Y)es or N)o. ",
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 5, 32, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc},
	{1, LIN, "listType",	 7, 2, CHARTYPE,
		"U", "          ",
		" ", "S", "Listing Type              ", "Enter F)ull or S)hort",
		YES, NO,  JUSTLEFT, "FS", "", local_rec.listType},
	{1, LIN, "listTypeDesc",	 7, 32, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.listTypeDesc},
	{1, LIN, "listBy",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "B", "By Company/Branch         ", "Enter C)ompany or B)ranch",
		YES, NO,  JUSTLEFT, "CB", "", local_rec.listBy},
	{1, LIN, "listByDesc",	 9, 32, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.listByDesc},
	{1, LIN, "sortBy",	11, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Sort By Acronym / Number  ", "Enter Acronym or Number",
		YES, NO,  JUSTLEFT, "AN", "", local_rec.sortBy},
	{1, LIN, "sortByDesc",	 11, 32, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.sortByDesc},

	{1, LIN, "suppFrom",	 13, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier            ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplier},
	{1, LIN, "name1",	 13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startName},
	{1, LIN, "suppTo",	 14, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End   Supplier            ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplier},
	{1, LIN, "name2",	 14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endName},

	{1, LIN, "paymentMethod",	 16, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Pay Method                ", "Enter All, Cheque, Draft, Transfer",
		YES, NO,  JUSTRIGHT, "ACDT", "", local_rec.paymentMethod},
	{1, LIN, "paymentMethodDesc",	 16, 35, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.paymentMethodDesc},

	{0}
};

int		envCrFind;

#include <FindSumr.h>

/*===========================
| Local function prototypes |
===========================*/
int		spec_valid		 (int);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	SetupDefault	 (void);
void	RunProgram		 (void);
int		heading			 (int);
int		GetSumr			 (char *);
int		GetAcronym		 (char *);
void	SrchAcronym		 (char *);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	/*==============================
	 Open db and init comm record
	==============================*/
	envCrFind = atoi (get_env ("CR_FIND"));
	OpenDB ();

	strcpy (branchNumber, atoi (get_env ("CR_CO")) ? comm_rec.est_no : " 0");

	TruePosition	=	TRUE;
	EnvScreenOK 	=	FALSE;

	/*=====================
	| Reset control flags |
	=====================*/
	SETUP_SCR (vars);

	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
	init_vars 	 (1);

   	entry_exit	= TRUE;
   	prog_exit	= FALSE;
   	restart		= FALSE;
	search_ok	= TRUE;

	SetupDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);

	CloseDB (); 
	FinishProgram ();

	if (!restart)
		RunProgram ();

	return (EXIT_SUCCESS);
}

/*===================
 Validation routines
====================*/
int
spec_valid (
 int field)
{
	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc,
			(local_rec.back[0] == 'N') ? local_rec.noWord : local_rec.yesWord);
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc,
		   (local_rec.onight[0] == 'N') ? local_rec.noWord : local_rec.yesWord);
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("listType"))
	{
		strcpy (local_rec.listTypeDesc,
			(local_rec.listType [0] == 'F') ? ML ("Full   ") : ML ("Short "));
		DSP_FLD ("listTypeDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("listBy"))
	{
		strcpy (local_rec.listByDesc,
			(local_rec.listBy [0] == 'B') ? ML ("Branch ") : ML ("Company "));
		DSP_FLD ("listByDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sortBy"))
	{
		strcpy (local_rec.sortByDesc,
			(local_rec.sortBy [0] == 'A') ? ML ("Acronym ") : ML ("Number "));
			
		if (local_rec.sortBy [0] == 'A')
		{
			vars [label ("suppFrom")].mask 	= strdup ("UUUUUUUUU");	
			vars [label ("suppTo")].mask 	= strdup ("UUUUUUUUU");	
			vars [label ("suppFrom")].prmpt = strdup (ML ("Start Acronym  "));
			vars [label ("suppTo")].prmpt 	= strdup (ML ("End Acronym    "));

			heading (1);
			scn_display (1);
			DSP_FLD ("suppFrom");
			DSP_FLD ("suppTo");
		}
		else
		{
			vars [label ("suppFrom")].mask 	= strdup ("UUUUUU");	
			vars [label ("suppTo")].mask 	= strdup ("UUUUUU");	
			vars [label ("suppFrom")].prmpt = strdup (ML ("Start Supplier"));
			vars [label ("suppTo")].prmpt 	= strdup (ML ("End Supplier   "));
			heading (1);
			scn_display (1);
			DSP_FLD ("suppFrom");
			DSP_FLD ("suppTo");
		}
		DSP_FLD ("sortByDesc");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Supplier |
	-------------------*/
	if (LCHECK ("suppFrom"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startName, ML ("Start Supplier"));
			*local_rec.startSupplier = '\0';
			*local_rec.startAcronym = '\0';
			DSP_FLD ("name1");
			return (EXIT_SUCCESS);
		}

		if (local_rec.sortBy [0] == 'A')
		{
			if (SRCH_KEY)
			{
				SrchAcronym (temp_str);
				return (EXIT_SUCCESS);
			}
		}
		else
		{
			if (SRCH_KEY)
			{
				SumrSearch (comm_rec.co_no, branchNumber, temp_str);
				return (EXIT_SUCCESS);
			}
		}

		if (local_rec.sortBy [0] == 'A')
		{
			if (!GetAcronym (local_rec.startSupplier))
				return (EXIT_FAILURE);
		}
		else
		{
			if (!GetSumr (local_rec.startSupplier))
				return (EXIT_FAILURE);
		}

		strcpy (local_rec.startAcronym, sumr_rec.acronym);
		if ( (strcmp (local_rec.startSupplier, local_rec.endSupplier) > 0 && 
				local_rec.sortBy [0] == 'N') ||
			 (strcmp (local_rec.startAcronym, local_rec.endAcronym) > 0 && 
				local_rec.sortBy [0] == 'A'))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.startName, sumr_rec.crd_name);
		strcpy (local_rec.startAcronym, sumr_rec.acronym);
		DSP_FLD ("name1");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("suppTo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endName, ML ("End Supplier"));
			strcpy (local_rec.endAcronym, "~~~~~~~~~");
			DSP_FLD ("name2");
			return (EXIT_SUCCESS);
		}

			
		if (SRCH_KEY)
		{
			if (local_rec.sortBy [0] == 'A')
				SrchAcronym (temp_str);
			else
				SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (local_rec.sortBy [0] == 'A')
		{
			if (!GetAcronym (local_rec.endSupplier))
				return (EXIT_FAILURE);
		}
		else
		{
			if (!GetSumr (local_rec.endSupplier))
				return (EXIT_FAILURE);
		}


		strcpy (local_rec.endAcronym, sumr_rec.acronym);
		if ( (strcmp (local_rec.startSupplier, local_rec.endSupplier) > 0 && 
				local_rec.sortBy [0] == 'N') ||
			 (strcmp (local_rec.startAcronym, local_rec.endAcronym) > 0 && 
				local_rec.sortBy [0] == 'A'))
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.endName, sumr_rec.crd_name);
		strcpy (local_rec.endAcronym, sumr_rec.acronym);
		DSP_FLD ("name2");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("paymentMethod"))
	{
		char	desc [10];

		switch (*local_rec.paymentMethod)
		{
		case 'A'	:
			strcpy (desc, ML ("All     "));
			break;
		case 'C'	:
			strcpy (desc, ML ("Cheque  "));
			break;
		case 'D'	:
			strcpy (desc, ML ("Draft   "));
			break;
		case 'T'	:
			strcpy (desc, ML ("Transfer"));
			break;
		}
		strcpy (local_rec.paymentMethodDesc, desc);
		DSP_FLD ("paymentMethodDesc");
	}

	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (sumr2, sumr);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, envCrFind ? "sumr_id_no3" : "sumr_id_no");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, envCrFind ? "sumr_id_no2" : "sumr_id_no4");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_dbclose (data);
}

void
SetupDefault (void)
{
	strcpy (local_rec.yesWord, ML ("Yes "));
	strcpy (local_rec.noWord,  ML ("No  "));
	local_rec.printerNo = 1;
	strcpy (local_rec.backDesc, 	local_rec.noWord);
	strcpy (local_rec.back, 		"N");
	strcpy (local_rec.onightDesc, 	local_rec.noWord);
	strcpy (local_rec.onight, 		"N");
	strcpy (local_rec.listTypeDesc, ML ("Full"));
	strcpy (local_rec.listType, 	"F");
	strcpy (local_rec.listByDesc, 	ML ("Branch"));
	strcpy (local_rec.listBy, 		"B");
	strcpy (local_rec.sortByDesc, 	ML ("Number"));
	strcpy (local_rec.sortBy, 		"N");
	strcpy (local_rec.startSupplier,"      ");
	strcpy (local_rec.endSupplier, 	"~~~~~~");
	strcpy (local_rec.startAcronym, "         ");
	strcpy (local_rec.endAcronym, 	"~~~~~~~~~");
	strcpy (local_rec.paymentMethodDesc, ML ("All"));
	strcpy (local_rec.paymentMethod,"A");
}

void
RunProgram (
 void)
{
	char	begStr [10], endStr [10];

	if (*local_rec.sortBy == 'A')
	{
		/*----------------
		 Sort by Acronym (it's possible they may be back to front)
		----------------*/
		if (strcmp (local_rec.startAcronym, local_rec.endAcronym) > 0)
		{
			strcpy (begStr, local_rec.endAcronym);
			strcpy (endStr, local_rec.startAcronym);
		}
		else
		{
			strcpy (begStr, local_rec.startAcronym);
			strcpy (endStr, local_rec.endAcronym);
		}

	}
	else
	{
		/*------------------------
		 Sort by Supplier Number
		-------------------------*/
		strcpy (begStr, local_rec.startSupplier);
		strcpy (endStr, local_rec.endSupplier);
	}

	/*================================
	| Test for Overnight Processing. |
	================================*/
	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str,
			"ONIGHT cr_lists \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			local_rec.printerNo,
			local_rec.listType,
			local_rec.listBy,
			local_rec.sortBy,
			begStr,
			endStr,
			local_rec.paymentMethod,
			ML (mlCrMess191)
		);
		SystemExec	(err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str,
			"cr_lists \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
			local_rec.printerNo,
			local_rec.listType,
			local_rec.listBy,
			local_rec.sortBy,
			begStr,
			endStr,
			local_rec.paymentMethod
		);
		SystemExec	(err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		rv_pr (ML (mlCrMess124), (SCREENWIDTH - strlen (ML (mlCrMess124))) / 2, 0, 1);

		line_at (6, 0, SCREENWIDTH);
		line_at (8, 0, SCREENWIDTH);
		line_at (10,0, SCREENWIDTH);
		line_at (12,0, SCREENWIDTH);
		line_at (15,0, SCREENWIDTH);
		line_at (1, 0, SCREENWIDTH);
		box (0, 2, SCREENWIDTH, 14);

		/* for company & br */
		line_at (20, 0, SCREENWIDTH);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str, comm_rec.co_no, clip (comm_rec.co_short));
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0,err_str, comm_rec.est_no, clip (comm_rec.est_name));
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*=================
 Support utilities
==================*/
int
GetSumr (
 char *	crdt)
{
	
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	sprintf (sumr_rec.crd_no,"%-6.6s", crdt);
	if (find_rec (sumr, &sumr_rec, COMPARISON, "r"))
	{
		print_mess (ML (mlStdMess022));
		sleep (sleepTime);
		return (FALSE);
	}
	return (TRUE);
}

int
GetAcronym (
 char *	crdt)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	sprintf (sumr_rec.acronym, "%-9.9s", crdt);
	if (find_rec (sumr2, &sumr_rec, COMPARISON, "r"))
	{
		print_mess (ML (mlStdMess022));
		sleep (sleepTime);
		return (FALSE);
	}
	return (TRUE);
}

void
SrchAcronym (
 char *	key_val)
{
	work_open ();
	save_rec ("#Acronym    ", "# Supplier            Description              ");
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);	
	sprintf (sumr_rec.acronym,"%-9.9s",key_val);
	cc = find_rec (sumr2, &sumr_rec, GTEQ, "r");
	while (!cc  &&
		   !strcmp (sumr_rec.co_no, comm_rec.co_no)  &&
		   !strncmp (sumr_rec.acronym, key_val , strlen (key_val)) &&
		  !strcmp (sumr_rec.est_no, branchNumber))
	{

		sprintf (err_str,"%-9.9s %-40.40s",sumr_rec.crd_no, sumr_rec.crd_name);

		cc = save_rec (sumr_rec.acronym, err_str);
		if (cc)
			break;


		cc = find_rec (sumr2, &sumr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;


	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);	
	sprintf (sumr_rec.acronym,"%-9.9s",temp_str);
	cc = find_rec ("sumr2",&sumr_rec, COMPARISON,"r");
	if (cc)
		sys_err ("Error in sumr During (DBFIND)",cc, PNAME);
}
