/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: _fcst_upd.c,v 5.3 2001/09/19 06:09:09 robert Exp $
|  Program Name  : (lrp_fcst_upd.c)                                   |
|  Program Desc  : (Inventory Summary Report                       )  |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
| $Log: _fcst_upd.c,v $
| Revision 5.3  2001/09/19 06:09:09  robert
| Updated to change "ALL" marker to "~~"
|
| Revision 5.2  2001/08/09 09:29:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:20  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:28  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/24 00:15:38  scott
| Updated to add app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _fcst_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_fcst_upd/_fcst_upd.c,v 5.3 2001/09/19 06:09:09 robert Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_lrp_mess.h>

#define	ALL_BR		 (!strcmp (local_rec.startBranch, "~~"))
#define	ALL_WH		 (!strcmp (local_rec.startWh, "~~"))

#define	MAX_BR		99

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct inccRecord	incc_rec;

struct	{
	char	num [3];
	char	name [9];
	long	val_hhcc [20];
	int	num_hhcc;
} branches [MAX_BR + 1];

float	avail_tot [13];
float	tot_order;

int		numberBranches;
int		editOnly;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startClass [2];
	char	endClass [2];
	char	lower [17];
	char	lower_desc [41];
	char	upper [17];
	char	upper_desc [41];
	char	startBranch [7];
	char	startBranchDesc [41];
	char	endBranch [7];
	char	endBranchDesc [41];
	char	startWh [7];
	char	startWhDesc [41];
	char	endWh [7];
	char	endWhDesc [41];
	char	forecastOption [2];
	char	forecastMethod [2];
	float	sfty_stk;
} local_rec;

static	struct	var vars [] =
{
	{1, LIN, "startClass",	 4, 16, CHARTYPE,
		"U", "          ",
		" ", "A", " Start Class  ", "Enter Class A-Z ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.startClass},
	{1, LIN, "lower",	 5, 16, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " Start Category ", "Enter Category or [SEARCH] ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.lower},
	{1, LIN, "lower_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.lower_desc},
	{1, LIN, "endClass",	 7, 16, CHARTYPE,
		"U", "          ",
		" ", "A", " End Class  ", "Enter Class A-Z ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.endClass},
	{1, LIN, "upper",	8, 16, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", " End Category  ", "Enter Category or [SEARCH] ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.upper},
	{1, LIN, "upper_desc",	8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.upper_desc},

	{1, LIN, "startBranch", 10, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " Start Branch ", "Enter branch or [SEARCH] ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.startBranch},
	{1, LIN, "startBranchDesc",	10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startBranchDesc},
	{1, LIN, "endBranch", 11, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " End Branch ", "Enter branch or [SEARCH] ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.endBranch},
	{1, LIN, "endBranchDesc",	11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endBranchDesc},

	{1, LIN, "startWh", 13, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " Start Warehouse ", "Enter warehouse or [SEARCH] ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.startWh},
	{1, LIN, "startWhDesc",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startWhDesc},
	{1, LIN, "endWh", 14, 16, CHARTYPE,
		"UU", "          ",
		" ", "", " End Warehouse ", "Enter warehouse or [SEARCH] ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.endWh},
	{1, LIN, "endWhDesc",	14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endWhDesc},

	{1, LIN, "forecastOption", 16, 16, CHARTYPE,
		"U", "          ",
		" ", "", " Forecast Option ", "Enter forecast option A(utomatic) or P(redetermined) ",
		 YES, NO,  JUSTLEFT, "AP", " Must Be A or P ", local_rec.forecastOption},
	{1, LIN, "forecastMethod", 16, 53, CHARTYPE,
		"U", "          ",
		" ", "", " Forecast Method ", " Must Be A, B, C, D ",
		 ND, NO,  JUSTLEFT, "ABCD", "", local_rec.forecastMethod},
	{1, LIN, "sfty_stk", 17, 16, FLOATTYPE,
		"NN.NN", "          ",
		" ", "", " Safety Stock ", "Enter safety stock in weeks. ",
		 YES, NO,  JUSTRIGHT, "0123456789.", "",(char *)&local_rec.sfty_stk},
	{0, LIN, "", 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SrchExcf 			(char *);
void 	SrchEsmr 			(char *);
void 	SrchCcmr 			(char *);
int 	Process 			(void);
int 	ValidIncc 			(void);
int 	LoadEsmr 			(void);
int 	LoadCcmr 			(void);
int 	spec_valid 			(int);
int 	heading 			(int);


/*=====================================================================
| Main Processing Routine.                                            |
=====================================================================*/
int
main (
 int    argc,
 char*  argv [])
{
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	editOnly	= FALSE;
	prog_exit	= FALSE;
	while (!prog_exit)
	{
		if (!editOnly)
		{
			/*---------------------
			| Reset control flags |
			---------------------*/
			entry_exit	= FALSE;
			edit_exit	= FALSE;
			prog_exit	= FALSE;
			restart		= FALSE;
			search_ok	= TRUE;
			init_vars (1);		/*  set default values	*/
	
			/*----------------------------
			| Entry screen 1 linear input |
			----------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;
		}

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			editOnly = FALSE;
			continue;
		}
	
		if (local_rec.forecastOption [0] == 'P' && 
			local_rec.forecastMethod [0] == ' ')
		{
			print_mess (ML (mlLrpMess030));
			sleep (sleepTime);
			clear_mess ();
			prog_exit = FALSE;
			editOnly = TRUE;
			continue;
		}

		LoadEsmr ();
		if (!restart)
			Process ();

		prog_exit = TRUE;
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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr,inmr_list,INMR_NO_FIELDS,"inmr_id_no_3");
	open_rec (incc,incc_list,INCC_NO_FIELDS,"incc_hhbr_hash");
	open_rec (excf,excf_list,EXCF_NO_FIELDS,"excf_id_no");
	open_rec (esmr,esmr_list,ESMR_NO_FIELDS,"esmr_id_no");
	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (excf);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_dbclose ("data");
}

/*-------------------------------
| Process within category range |
-------------------------------*/
int
Process (void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", local_rec.startClass);
	sprintf (inmr_rec.category, "%-11.11s", local_rec.lower);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");

	dsp_screen (ML (mlLrpMess036), comm_rec.co_no, comm_rec.co_name);

	/*------------------------------------
	| Process all inmr records that fall |
	| within the class/category range    |
	| selected by the user		     	 |
	------------------------------------*/
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       strncmp (inmr_rec.inmr_class, local_rec.endClass, 1) <= 0)
	{
		dsp_process ("Item: ", inmr_rec.item_no);
		/*-----------------------
		| Category out of range |
		-----------------------*/
		if ((!strcmp (inmr_rec.inmr_class,local_rec.endClass)) &&
		    (strcmp (inmr_rec.category,local_rec.upper) > 0))
			break;

		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		incc_rec.hhcc_hash = 0L;
		cc = find_rec (incc, &incc_rec, GTEQ, "u");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (!ALL_BR && !ValidIncc ())
			{
				abc_unlock (incc);
				cc = find_rec (incc, &incc_rec, NEXT, "u");
				continue;
			}

			sprintf (incc_rec.ff_option,"%-1.1s",local_rec.forecastOption);
	
			if (local_rec.forecastOption [0] == 'P')
				sprintf (incc_rec.ff_method, "%-1.1s", local_rec.forecastMethod);
			incc_rec.safety_stock = local_rec.sfty_stk;

			cc = abc_update (incc, &incc_rec);
			if (cc)
				file_err (cc, incc, "DBUPDATE");
				
			cc = find_rec (incc, &incc_rec, NEXT, "u");
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

/*----------------------------
| Check if incc is valid for |
| range of Br/Wh selected    |
----------------------------*/
int
ValidIncc (void)
{
	int	i;
	int	j;

	for (i = 0; i < numberBranches; i++)
	{
		for (j = 0; j < branches [i].num_hhcc; j++)
		{
			if (incc_rec.hhcc_hash == branches [i].val_hhcc [j])
				return (TRUE);
		}
	}

	return (FALSE);
}

/*---------------------------
| Load Branch names and     |
| list of valid hhcc hashes |
| for those branches        |
---------------------------*/
int
LoadEsmr (void)
{
	int	i;

	for (i = 0; i < MAX_BR; i++)
	{
		strcpy (branches [i].num, "  ");
		strcpy (branches [i].name, "        ");
		branches [i].num_hhcc = 0;
	}

	if (ALL_BR)
		return (EXIT_SUCCESS);

	numberBranches = 0;
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", local_rec.startBranch);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       strcmp (esmr_rec.est_no, local_rec.endBranch) <= 0 &&
	       numberBranches < 12)
	{
		sprintf (branches [numberBranches].num, "%2.2s", esmr_rec.est_no);
		sprintf (branches [numberBranches].name, "%-8.8s", esmr_rec.short_name);

		LoadCcmr ();
		numberBranches++;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	
	return (EXIT_SUCCESS);
}

int
LoadCcmr (void)
{
	branches [numberBranches].num_hhcc = 0;
	
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, esmr_rec.est_no);
	if (ALL_WH)
		strcpy (ccmr_rec.cc_no, "  ");
	else
		sprintf (ccmr_rec.cc_no, "%2.2s", local_rec.startWh);

	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (ccmr_rec.est_no, esmr_rec.est_no) &&
	       (ALL_WH || (!ALL_WH &&
	       strcmp (ccmr_rec.cc_no, local_rec.endWh) <= 0)))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
			continue;
		}
		branches [numberBranches].val_hhcc [branches [numberBranches].num_hhcc++] = ccmr_rec.hhcc_hash;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("startBranch"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startBranch, "~~");
			strcpy (local_rec.endBranch, "~~");
			sprintf (local_rec.startBranchDesc, "%-40.40s", "All Branches");
			sprintf (local_rec.endBranchDesc, "%-40.40s", "All Branches");
			FLD ("endBranch") = NA;
			DSP_FLD ("startBranch");
			DSP_FLD ("endBranch");
			DSP_FLD ("startBranchDesc");
			DSP_FLD ("endBranchDesc");

			FLD ("startWh") = NA;
			FLD ("endWh") = NA;
			sprintf (local_rec.startWh, "%2.2s","~~");
			sprintf (local_rec.startWhDesc, "%-40.40s", "All Warehouses");
			sprintf (local_rec.endWh, "%2.2s","~~");
			sprintf (local_rec.endWhDesc, "%-40.40s", "All Warehouses");
			DSP_FLD ("startWh");
			DSP_FLD ("startWhDesc");
			DSP_FLD ("endWh");
			DSP_FLD ("endWhDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		FLD ("endBranch") = YES;

		strcpy (esmr_rec.co_no,comm_rec.co_no);
		sprintf (esmr_rec.est_no,"%2.2s",local_rec.startBranch);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startBranch, "%2.2s ",esmr_rec.est_no);
		sprintf (local_rec.startBranchDesc, "%-40.40s",esmr_rec.est_name);
		DSP_FLD ("startBranch");
		DSP_FLD ("startBranchDesc");

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("endBranch"))
	{
		if (FLD ("endBranch") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no,comm_rec.co_no);
		sprintf (esmr_rec.est_no,"%2.2s",local_rec.endBranch);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endBranch, "%2.2s ",esmr_rec.est_no);
		sprintf (local_rec.endBranchDesc, "%-40.40s",esmr_rec.est_name);
		DSP_FLD ("endBranch");
		DSP_FLD ("endBranchDesc");

		if (!strcmp (local_rec.startBranch, local_rec.endBranch) && !ALL_BR)
		{
			FLD ("startWh") = NO;
			FLD ("endWh") = NO;
		}
		else
		{
			FLD ("startWh") = NA;
			FLD ("endWh") = NA;
			sprintf (local_rec.startWh, "%2.2s","~~");
			sprintf (local_rec.startWhDesc, "%-40.40s", "All Warehouses");
			sprintf (local_rec.endWh, "%2.2s","~~");
			sprintf (local_rec.endWhDesc, "%-40.40s", "All Warehouses");
			DSP_FLD ("startWh");
			DSP_FLD ("startWhDesc");
			DSP_FLD ("endWh");
			DSP_FLD ("endWhDesc");
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startWh"))
	{
		if (FLD ("startWh") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.startWh, "~~");
			strcpy (local_rec.endWh, "~~");
			sprintf (local_rec.startWhDesc, "%-40.40s", "All Warehouses");
			sprintf (local_rec.endWhDesc, "%-40.40s", "All Warehouses");
			FLD ("endWh") = NA;
			DSP_FLD ("startWh");
			DSP_FLD ("endWh");
			DSP_FLD ("startWhDesc");
			DSP_FLD ("endWhDesc");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}
		FLD ("endWh") = YES;

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		sprintf (ccmr_rec.est_no,"%2.2s",local_rec.startBranch);
		sprintf (ccmr_rec.cc_no,"%2.2s",local_rec.startWh);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			print_mess (ML ("Warehouse is not available for LRP"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.startWh, "%2.2s ",ccmr_rec.cc_no);
		sprintf (local_rec.startWhDesc, "%-40.40s",ccmr_rec.name);
		DSP_FLD ("startWh");
		DSP_FLD ("startWhDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endWh"))
	{
		if (FLD ("endWh") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}
		FLD ("endWh") = YES;

		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		sprintf (ccmr_rec.est_no,"%2.2s",local_rec.startBranch);
		sprintf (ccmr_rec.cc_no,"%2.2s",local_rec.endWh);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			print_mess (ML ("Warehouse is not available for LRP"));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.endWh, "%2.2s ",ccmr_rec.cc_no);
		sprintf (local_rec.endWhDesc, "%-40.40s",ccmr_rec.name);
		DSP_FLD ("endWh");
		DSP_FLD ("endWhDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lower") || LCHECK ("upper"))
	{
		if (dflt_used)
		{
			if (LCHECK ("lower"))
			{
				strcpy (local_rec.lower, "           ");
				sprintf (local_rec.lower_desc, "%-40.40s", "First Category");
				DSP_FLD ("lower");
				DSP_FLD ("lower_desc");
				return (EXIT_SUCCESS);
			}
			else
			{
				strcpy (local_rec.upper, "~~~~~~~~~~~");
				sprintf (local_rec.upper_desc, "%-40.40s", "Last Category");
				DSP_FLD ("upper");
				DSP_FLD ("upper_desc");
				return (EXIT_SUCCESS);
			}
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
		cc = find_rec ("excf", &excf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			if (LCHECK ("lower"))
				sprintf (local_rec.lower_desc, "%-40.40s",excf_rec.cat_desc);
			else
				sprintf (local_rec.upper_desc, "%-40.40s",excf_rec.cat_desc);
		}

		if (LCHECK ("lower"))
			DSP_FLD ("lower_desc");
		else
			DSP_FLD ("upper_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("forecastOption"))
	{
		if (local_rec.forecastOption [0] == 'A')
		{
			FLD ("forecastMethod") = ND;
			rv_pr ("                         ", 37, 16, 0);
		}
		else
		{
			FLD ("forecastMethod") = YES;
			DSP_FLD ("forecastMethod");
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=======================
| Search for category	|
=======================*/
void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category","#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",key_val);
	cc = find_rec ("excf",&excf_rec,GTEQ,"r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) &&
		      !strcmp (excf_rec.co_no,comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec ("excf",&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-16.16s",temp_str);
	cc = find_rec (excf, &excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

/*=====================================================================
| Search for branch
=====================================================================*/
void
SrchEsmr (
 char*  key_val)
{
	work_open ();
	save_rec ("#Br","#Branch Description");
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = find_rec (esmr,&esmr_rec,GTEQ,"r");
	while (!cc && !strcmp (esmr_rec.co_no,comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no,esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr,&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
 char*  key_val)
{
	work_open ();
	save_rec ("#Wh","#Warehouse Description");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	sprintf (ccmr_rec.est_no,"%2.2s",local_rec.startBranch);
	sprintf (ccmr_rec.cc_no,"%2.2s"," ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) &&
	       !strncmp (ccmr_rec.est_no,local_rec.startBranch, 2))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
			continue;
		}
		cc = save_rec (ccmr_rec.cc_no,ccmr_rec.name);
		if (cc)
			break;
		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	sprintf (ccmr_rec.est_no,"%2.2s",local_rec.startBranch);
	sprintf (ccmr_rec.cc_no,"%2.2s",key_val);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlLrpMess036), 22, 0, 1);

		move (0,1);
		line (80);

		box (0,3,80,14);
		move (1,6);
		line (79);

		move (1,9);
		line (79);

		move (1,12);
		line (79);

		move (1,15);
		line (79);

		move (0,20);
		line (80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,  err_str, 
					comm_rec.co_no,comm_rec.co_name);

		strcpy (err_str, ML (mlStdMess039));
		print_at (21,45, err_str,
					comm_rec.est_no,comm_rec.est_name);

		move (0,22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
