/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_cre_claim.c )                                 |
|  Program Desc  : ( Customer Rebates Claim Report.               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, inrb, cura, curh, inmr, pocr,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 04/11/93         |
|---------------------------------------------------------------------|
|  Date Modified : (13/12/95)      | Modified by : Anneliese Allen    |
|                : (27/07/96)      | Modified by : Scott B Darrow.    |
|                : (05/09/97)      | Modified by : Leah Manibog.      |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|  (13/12/95)    : Modified to allow for addition of include/exclude  |
|                : flag on cura.                                      |
|                :                                                    |
|  (27/07/96)    : Updated to fix include/exclude flag. Think its     |
|                : now correct.                                       |
|  (05/09/97)    : Updated for Multilingual Conversion and            |
|                :                                                    |
| $Log: cre_claim.c,v $
| Revision 5.4  2002/07/17 09:57:52  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/19 03:30:54  robert
| Updated to avoid overlapping of descriptions and fixed alignment
|
| Revision 5.2  2001/08/09 09:18:19  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:44:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:15:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:36:51  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:47  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:10:33  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.20  2000/06/15 05:39:46  jinno
| Updated to change FindCumr() local routine to IntFindCumr to ensure no
| conflict will exist with new routine FindCumr() that is about to be
| introduced.
|
| Revision 1.19  1999/12/06 01:30:41  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.18  1999/11/11 05:59:35  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.17  1999/11/03 07:31:55  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.16  1999/10/20 01:38:56  nz
| Updated for remainder of old routines.
|
| Revision 1.15  1999/10/13 02:41:54  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.14  1999/10/13 00:57:23  scott
| Updated from ansi code testing
|
| Revision 1.13  1999/10/12 21:20:31  scott
| Updated by Gerry from ansi project.
|
| Revision 1.12  1999/10/08 05:32:17  scott
| First Pass checkin by Scott.
|
| Revision 1.11  1999/06/20 05:19:52  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
---------------------------------------------------------------------*/
char	*PNAME = "$RCSfile: cre_claim.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cre_claim/cre_claim.c,v 5.4 2002/07/17 09:57:52 scott Exp $";
#define CCMAIN

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

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

	/*=======================
	| Inventory Rebate File |
	=======================*/
	struct dbview inrb_list [] =
	{
		{"inrb_reb_flag"},
		{"inrb_link_hash"},
		{"inrb_reb_code"},
		{"inrb_cycle"},
		{"inrb_description"},
		{"inrb_basis"},
		{"inrb_reb_type"},
		{"inrb_start_date"},
		{"inrb_end_date"},
		{"inrb_reb_qty1"},
		{"inrb_reb_qty2"},
		{"inrb_reb_qty3"},
		{"inrb_reb_qty4"},
		{"inrb_reb_qty5"},
		{"inrb_reb_qty6"},
		{"inrb_reb_val1"},
		{"inrb_reb_val2"},
		{"inrb_reb_val3"},
		{"inrb_reb_val4"},
		{"inrb_reb_val5"},
		{"inrb_reb_val6"},
		{"inrb_ant_reb_pc"},
		{"inrb_la_cycle_rep"}
	};

	int	inrb_no_fields = 23;

	struct tag_inrbRecord
	{
		char	reb_flag [2];
		long	link_hash;
		char	reb_code [6];
		int		cycle;
		char	description [41];
		char	basis [2];
		char	reb_type [2];
		long	start_date;
		long	end_date;
		double	reb_qty [6];
		double	reb_value [6];
		float	ant_reb_pc;
		int		last_cycle;
	} inrb_rec;

	/*======================
	| Currency File Record |
	======================*/
	struct dbview pocr_list [] =
	{
		{"pocr_co_no"},
		{"pocr_code"},
		{"pocr_ex1_factor"},
	};

	int	pocr_no_fields = 3;

	struct tag_pocrRecord
	{
		char	co_no [3];
		char	code [4];
		double	ex1_factor;
	} pocr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_sellgrp"},
		{"inmr_weight"}
	};

	int	inmr_no_fields = 6;

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		char	sellgrp [7];
		float	weight;
	} inmr_rec;

	/*===============================
	| Customers Rebate History File |
	===============================*/
	struct dbview curh_list [] =
	{
		{"curh_hhbr_hash"},
		{"curh_hhcu_hash"},
		{"curh_ord_date"},
		{"curh_ord_qty"},
		{"curh_inv_no"},
		{"curh_line_cost"}
	};

	int	curh_no_fields = 6;

	struct tag_curhRecord
	{
		long	hhbr_hash;
		long	hhcu_hash;
		long	ord_date;
		float	ord_qty;
		char	inv_no [9];
		double	line_cost;
	} curh_rec;

	/*=======================
	| Customer   Master File |
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
	| Customer Rebate Assignment File |
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
			*curh = "curh",
			*inmr = "inmr",
			*pocr = "pocr",
			*data = "data";

	char	branchNumber[3];
	int		envDbCo = FALSE;
	int		envDbFind  = FALSE;
	long	fromDate;
	long	toDate;
	int		FirstTime;
	int		REPRINT = FALSE;

	FILE	*fsort, *fout;

struct	{
	char	dummy[11];
	char	scust [7];
	char	scustdesc [41];
	char	ecust [7];
	char	ecustdesc [41];
	char	back [2];
	char	backdesc [4];
	char	onite [2];
	char	onitedesc [4];
	int		lpno;
	long	date;
	char	lpstr [2];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "scust",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer       :", "Enter Start Customer, Default = Start Of File, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.scust},
	{1, LIN, "scustdesc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.scustdesc},
	{1, LIN, "ecust",	 5, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "End Customer         :", "Enter End Customer, Default = End Of File, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ecust},
	{1, LIN, "ecustdesc",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ecustdesc},
	{1, LIN, "lpno",	 7, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number       :  ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{1, LIN, "back",	8, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Background           :  ", "Enter (Y)es or (N)o",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backdesc",	8, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.backdesc},
	{1, LIN, "onite",	9, 24, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight            :  ", "Enter (Y)es or (N)o",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "onitedesc",9, 34, CHARTYPE,
		"AAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.onitedesc},
	{2, LIN, "cust",	 4, 24, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer             :", "Enter Customer For Reprint, Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.scust},
	{2, LIN, "custdesc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.scustdesc},
	{2, LIN, "date", 6, 24, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Date To Reprint     :  ", "Enter Date To Reprint As At ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.date},
	{2, LIN, "lpno",	 7, 24, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer number      :  ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};


#include <FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
void ShutdownProg (void);
void OpenDB (void);
void CloseDB (void);
int  heading (int scn);
int  spec_valid (int field);
void Process (void);
int  IntFindCumr (char *rec, int Errors);
void RunProg (char *name, char *description);
void HeadOut (void);
void OutPutInrb (void);
void DefineSection (void);
int  LoopInrb (float *qty, float *wgt, double *tot, double *reb);
void PrntTots (float *qty, float *wgt, double *tot, double *reb);
int  LoadCurh (void);
void PutIntoSort (void);
void PrintLines (float *cusQty, float *cusWgt, double *cusTot, double *TotReb);
int  NotActive (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	if (strncmp (argv [0], "sk_cre_claim", 12))
		REPRINT = TRUE;

	if (!REPRINT && argc != 2 && argc != 4)
	{
		print_at (0,0, mlSkMess210, argv [0]);
		print_at (1,0, mlSkMess111, argv [0]);
		return (EXIT_FAILURE);
	}

	init_scr ();
	set_tty ();

	if (argc != 4)
	{
		SETUP_SCR (vars);
		set_masks ();
		if (!REPRINT)
			init_vars (1);
		else
			init_vars (2);
	}

	OpenDB ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	strcpy (branchNumber, (envDbCo) ? comm_rec.test_no : " 0");

	if (!REPRINT && argc == 4)
	{
		local_rec.lpno = atoi (argv [1]);

		sprintf (local_rec.scust, "%6.6s", argv [2]);
		sprintf (local_rec.ecust, "%6.6s", argv [3]);

		Process ();
		ShutdownProg ();
        return (EXIT_SUCCESS);
	}

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = TRUE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;

		if (!REPRINT)
		{
			init_vars (1);
			heading (1);
			entry (1);
		}
		else
		{
			init_vars (2);
			heading (2);
			entry (2);
		}

		if (restart || prog_exit)
			continue;

		if (!REPRINT)
		{
			heading (1);
			scn_display (1);
			edit (1);
		}
		else
		{
			heading (2);
			scn_display (2);
			edit (2);
		}

		if (restart)
			continue;

		RunProg (argv[0], argv[1]);
	}
	ShutdownProg ();
	return (EXIT_SUCCESS);
}

void
ShutdownProg (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (cumr, cumr_list, cumr_no_fields, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	open_rec (inrb, inrb_list, inrb_no_fields, "inrb_id_no1");
	open_rec (cura, cura_list, cura_no_fields, "cura_id_no2");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_sell");
	open_rec (curh, curh_list, curh_no_fields, "curh_hhbr_hash");
	open_rec (pocr, pocr_list, pocr_no_fields, "pocr_id_no");
}

void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (inrb);
	abc_fclose (cura);
	abc_fclose (inmr);
	abc_fclose (curh);
	abc_fclose (pocr);
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

	if (REPRINT)
		sprintf (err_str,ML(mlSkMess669));
	else 
		sprintf (err_str,ML(mlSkMess157));

	clip (err_str);
	strcat (err_str, " ");
	rv_pr (err_str, (40 - (strlen (err_str) / 2)), 0, 1);

	move (0, 1);
	line (80);
	
	if (!REPRINT)
	{
		move (0, 6);
		line (80);
		box (0, 3, 80, 6);		
	}
	else
	{
		move (0, 5);
		line (80);
		box (0, 3, 80, 4);		
	}

	move (0, 21);
	line (80);

	print_at (22, 0, ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("date"))
	{
		if (local_rec.date > TodaysDate ())
		{
			print_mess (ML(mlStdMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("scust"))
	{
		if (REPRINT)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.scust, "%6.6s", " ");
			sprintf (local_rec.scustdesc, "%-40.40s", ML("Start Of File "));
			DSP_FLD ("scustdesc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindCumr (local_rec.scust, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.scust, cumr_rec.cm_dbt_no);
		strcpy (local_rec.scustdesc, cumr_rec.cm_name);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.scust, local_rec.ecust) > 0)
			{
				print_mess (ML(mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		DSP_FLD ("scustdesc");
		return(0);
	}

	if (LCHECK ("ecust"))
	{
		if (REPRINT)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.ecust, "%6.6s", "~~~~~~");
			sprintf (local_rec.ecustdesc, "%-40.40s", ML("End Of File "));
			DSP_FLD ("ecustdesc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindCumr (local_rec.ecust, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.ecustdesc, cumr_rec.cm_name);
		strcpy (local_rec.ecust, cumr_rec.cm_dbt_no);

		if (strcmp (local_rec.scust, local_rec.ecust) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("ecustdesc");
		return(0);
	}

	if (LCHECK ("cust"))
	{
		if (!REPRINT)
			return (EXIT_SUCCESS);

		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindCumr (local_rec.scust, TRUE))
			return (EXIT_FAILURE);

		/*----------------------
		| reprint so want both
		| start and end to be same
		--------------------------*/
		strcpy (local_rec.scust, 	cumr_rec.cm_dbt_no);
		strcpy (local_rec.scustdesc, cumr_rec.cm_name);
		strcpy (local_rec.ecust, cumr_rec.cm_dbt_no);
		strcpy (local_rec.ecustdesc, cumr_rec.cm_name);

		DSP_FLD ("custdesc");
		return(0);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
			strcpy (local_rec.backdesc, "Yes");
		else
			strcpy (local_rec.backdesc, "No ");

		DSP_FLD ("backdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'Y')
			strcpy (local_rec.onitedesc, "Yes");
		else
			strcpy (local_rec.onitedesc, "No ");

		DSP_FLD ("onitedesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=========================
| read through entered range
| of customers, for each customer
| reset the customers total,
| and read thru all cura belonging
| to that customer, for each cura
| find the corresponding inrb record
| and all inmrs that have belong to the
| selling group of the the cura
=========================================*/
void  
Process (
 void)
{
	float	custQty = 0.00;
	float 	custWgt = 0.00;
	double 	custTot = 0.00;
	double 	custReb = 0.00;

	int		fstTime = TRUE;

	dsp_screen ("Rebate Claims", comm_rec.tco_no, comm_rec.tco_name);
	memset(&cumr_rec, 0, sizeof(cumr_rec));
	strcpy (cumr_rec.cm_co_no, comm_rec.tco_no);
	strcpy (cumr_rec.cm_est_no, branchNumber);
	sprintf(cumr_rec.cm_dbt_no, "%-6.6s", local_rec.scust);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc &&
			!strcmp (cumr_rec.cm_co_no, comm_rec.tco_no) &&
			(strcmp (cumr_rec.cm_dbt_no, local_rec.ecust) < 1)
		  )
		  {
				dsp_process ("Customer ", cumr_rec.cm_dbt_no);

				custQty = 0.00;
				custWgt = 0.00;
				custTot = 0.00;
				custReb = 0.00;

				if (fstTime)
				{
					HeadOut ();
					fstTime = FALSE;
				}

				if (LoopInrb (&custQty, &custWgt, &custTot, &custReb))
					PrntTots (&custQty, &custWgt, &custTot, &custReb);

				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		  }

		  if (!fstTime)
		  {
			fprintf (fout, ".EOF\n");
			pclose (fout);
		  }
}

int
IntFindCumr (
 char *rec, 
 int Errors)
{
		sprintf (cumr_rec.cm_dbt_no, "%6.6s", rec);
		strcpy (cumr_rec.cm_co_no, comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no, branchNumber);
		pad_num (cumr_rec.cm_dbt_no);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc && Errors) 
		{
			print_mess (ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
		}
		return (cc); 
}

void
RunProg (
 char *name, 
 char *description)
{
	sprintf (local_rec.lpstr, "%d", local_rec.lpno);
	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onite[0] == 'Y') 
	{
		if (fork () == 0)
			execlp("ONIGHT",
				"ONIGHT",
				name,
				local_rec.lpstr,
				local_rec.scust,
				local_rec.ecust,
				description,(char *)0);
		else
			prog_exit = 1;
	}
	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp(name,
				name,
				local_rec.lpstr,
				local_rec.scust,
				local_rec.ecust,
				(char *)0);
		else
			prog_exit = 1;
	}
    else
	    Process ();
}

void
HeadOut (
 void)
{
	FirstTime = TRUE;

	if ((fout = popen ("pformat", "w")) == (FILE *) NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L135\n");

	fprintf(fout,".ECompany %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));

	sprintf (err_str, 
			 ".ECustomer Rebate Claim Report %s",
			 (REPRINT) ? "- Reprint" : " ");
	fprintf (fout, "%s\n", clip (err_str));

	if (!REPRINT)
		fprintf (fout, ".E AS AT %s", SystemTime ());
	else
		fprintf (fout, ".E For Date %s\n", DateToString (local_rec.date));

	if (!REPRINT)
		fprintf (fout, 
			 ".E From Customers '%s' to '%s'\n", 
			 local_rec.scust,
			 local_rec.ecust);
	else
		fprintf (fout, ".E For Customer '%s'\n", local_rec.scust);

	fprintf (fout, ".B1\n");

	fprintf (fout, "=======================================================");
	fprintf (fout, "==========================================");
	fprintf (fout, "==============================\n");
	fprintf (fout, "| I T E M   NO.    ");
	fprintf (fout, "|   D E S C R I P T I O N                 ");
	fprintf (fout, "| Date Sold  ");
	fprintf (fout, "|   INV NO  ");
	fprintf (fout, "|  Quantity  ");
	fprintf (fout, "|   Weight   ");
	fprintf (fout, "|  Sale Price |\n");
	fprintf (fout, "|------------------------------------------------------");
	fprintf (fout, "--------------------------------------------");
	fprintf (fout, "---------------------------|\n");

	fprintf (fout, ".R=======================================================");
	fprintf (fout, "============================================");
	fprintf (fout, "============================\n");

}

void
OutPutInrb (
 void)
{
	char	sdate [11];
	char	fdate [11];

	sprintf (fdate, "%-10.10s",	DateToString (fromDate));
	sprintf (sdate, "%-10.10s",	DateToString (inrb_rec.start_date));

	fprintf (fout, ".LRP7\n");
	fprintf (fout, 
			 "| Rebate Code : %s - %s %59.59s|\n",
			 inrb_rec.reb_code,
			 inrb_rec.description,
			 " ");
	fprintf (fout, 
			 "| Rebates Live Is From %-10.10s to %-10.10s (%2d Monthly Cycle) %57.57s|\n",
			 sdate,
			 DateToString (inrb_rec.end_date),
			 inrb_rec.cycle,
			 " ");

	fprintf (fout, 
			 "| This Cycle Is From %-10.10s to %-10.10s  %77.77s|\n",
			 fdate,
			 DateToString (toDate - 1),
			 " ");

	fprintf (fout, "|%125.125s|\n", " ");
	fprintf (fout, "|------------------------------------------------------");
	fprintf (fout, "--------------------------------------------");
	fprintf (fout, "---------------------------|\n");
}

void
DefineSection (
 void)
{
	char	sdate [11];
	char	fdate [11];

	strcpy (sdate, DateToString (inrb_rec.start_date));
	strcpy (fdate, DateToString (fromDate));

	fprintf (fout, ".DS6\n");
	fprintf (fout, 
			 "| Customer : %s - %s   %61.61s|\n", 
			 cumr_rec.cm_dbt_no, 
			 cumr_rec.cm_name,
			 " ");
	fprintf (fout, 
			 "| Rebate Code : %s - %s   %59.59s|\n",
			 inrb_rec.reb_code,
			 inrb_rec.description,
			 " ");
	fprintf (fout, 
			 "| Rebates Live Is From %-10.10s to %-10.10s (%2d Monthly Cycle) %57.57s  |\n",
			 sdate,
			 DateToString (inrb_rec.end_date),
			 inrb_rec.cycle,
			 " ");

	fprintf (fout, 
			 "| This Cycle Is From %-10.10s to %-10.10s    %77.77s|\n",
			 fdate,
			 DateToString (toDate - 1),
			 " ");

	fprintf (fout, "|%125.125s|\n", " ");
	fprintf (fout, "|--------------------------------------------------------");
	fprintf (fout, "------------------------------------------");
	fprintf (fout, "---------------------------|\n");
}

int
LoopInrb (
 float *qty, 
 float *wgt, 
 double *tot, 
 double *reb)
{
	/*------------------
	| loop thru all rebates
	-----------------------*/
	int		loop = 0;
	float	lcl_qty = *qty;
	float	lcl_wgt = *wgt;
	double	lcl_tot = *tot;
	double	lcl_reb = *reb;

	strcpy (inrb_rec.reb_flag, "C");
	inrb_rec.link_hash = cumr_rec.cm_hhcu_hash;
	strcpy (inrb_rec.reb_code, "     ");
	inrb_rec.cycle	=	0;
	cc = find_rec (inrb, &inrb_rec, GTEQ, REPRINT ? "r" : "u");
	while (!cc &&
			inrb_rec.reb_flag [0] == 'C' &&
			inrb_rec.link_hash == cumr_rec.cm_hhcu_hash
		 )
		 {
			/*------------------
			| check to see if rebate
			| active yet/still
			-------------------------*/
			if (NotActive ())
			{
				abc_unlock (inrb);
				cc = find_rec (inrb, &inrb_rec, NEXT, REPRINT ? "r" : "u");
				continue;
			}

			DefineSection ();
			if (!loop && !FirstTime)
			{
				fprintf (fout, ".PA\n");
			}
			else
			{
				if (!FirstTime)
					OutPutInrb ();
			}

			FirstTime = FALSE;
			loop++;
			fsort = sort_open ("cusre");

			/*----------------
			| for each inrb rec
			| that we find, find
			| all cura records
			----------------------*/
			cura_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
			sprintf (cura_rec.rebate, "%5.5s", inrb_rec.reb_code);
			sprintf (cura_rec.sellgrp, "%6.6s", " ");
			cc = find_rec (cura, &cura_rec, GTEQ, "r");

			while (!cc && 
					cura_rec.hhcu_hash == cumr_rec.cm_hhcu_hash && 
					!strcmp (cura_rec.rebate, inrb_rec.reb_code)
				  )
			{
				/*---------------------------------------------------
				| Need to check incl/excl flag on cura.  If exclude |
				| get all selling groups and check each one against |
				| cura to see if it should be exluded               |
				---------------------------------------------------*/
				if (cura_rec.incl_flag[0] == 'I')
					LoadCurh ();

				cc = find_rec (cura, &cura_rec, NEXT, "r");
			}

			fsort = sort_sort (fsort, "cusre");
			PrintLines (&lcl_qty, &lcl_wgt, &lcl_tot, &lcl_reb);

			*qty    = lcl_qty;
			*wgt    = lcl_wgt;
			*tot    = lcl_tot;
			*reb 	= lcl_reb;

		/*	sort_delete (fsort, "cusre");*/

			if (!REPRINT)
			{
				inrb_rec.last_cycle++;
				cc = abc_update (inrb, &inrb_rec);
				if (cc)
					file_err (cc, inrb, "DBUPDATE");
			}
			cc = find_rec (inrb, &inrb_rec, NEXT, REPRINT ? "r" : "u");
		 }
		 abc_unlock (inrb);
		 return (loop);
}

void
PrntTots (
 float *qty, 
 float *wgt, 
 double *tot, 
 double *reb)
{
	fprintf (fout, "| Customer Totals     ");
	fprintf (fout, "                                                   ");
	fprintf (fout, 
			 "   %8.8s  | %10.2f | %10.2f | %11.2f |\n", 
			 " ", 
			 *qty, 
			 *wgt, 
			 *tot);
	fprintf (fout, "| Total Rebate For Customer          ");
	fprintf (fout, "                                                         ");
	fprintf (fout, "                  | %11.2f |\n", *reb);
	fprintf (fout, "|   %122.122s|\n", " "); /*new*/
}

int
LoadCurh (
 void)
{
	int	found = FALSE;

	/*-----------------------
	| find all inmr belonging
	| to cura selling group
	-----------------------*/
	strcpy (inmr_rec.co_no, comm_rec.tco_no);
	strcpy (inmr_rec.sellgrp, cura_rec.sellgrp);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc &&
			!strcmp (inmr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (inmr_rec.sellgrp, cura_rec.sellgrp)
		  )
	  {
			/*-----------------
			| load curh records
			| into sort file
			------------------*/
			memset(&curh_rec, 0, sizeof(curh_rec));
			cc = find_hash (curh, &curh_rec, GTEQ, "r", inmr_rec.hhbr_hash);
			while (!cc && curh_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				if (curh_rec.ord_date >= fromDate &&
					curh_rec.ord_date <= toDate &&
					curh_rec.hhcu_hash == cumr_rec.cm_hhcu_hash)
				{
						PutIntoSort ();
						found = TRUE;
				}

				cc = find_hash (curh, &curh_rec, NEXT, "r", inmr_rec.hhbr_hash);
			}

			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	  }

	  return (found);
}

void
PutIntoSort (
 void)
{
	/*--------------------
	| see doco for offsets
	---------------------*/
    char cBuffer [256];
	sprintf (cBuffer, 
			 "%-16.16s%-40.40s %10.10s %10.2f %-9.9s %11.2f %10.2f\n",
			 inmr_rec.item_no,
			 inmr_rec.description,
			 DateToString (curh_rec.ord_date),
			 curh_rec.ord_qty,
			 curh_rec.inv_no,
			 DOLLARS(curh_rec.line_cost),
			 inmr_rec.weight * curh_rec.ord_qty
			);
    sort_save (fsort, cBuffer);
}

void
PrintLines (
 float *cusQty, 
 float *cusWgt, 
 double *cusTot, 
 double *TotReb)
{
	char	*sptr;
	float	curhQty = 0.00;
	float	curhWgt = 0.00;
	double	curhTot = 0.00;
	double	rebTot = 0.00;
	int		count = 0;

	sptr = sort_read (fsort);

	while (sptr)
	{
		curhQty += atof (sptr + 68);
		curhWgt += atof (sptr + 101);
		curhTot += atof (sptr + 89);
		count++;

		fprintf (fout, 
				"| %16.16s | %40.40s| %10.10s | %9.9s | %10.10s | %10.10s | %11.11s |\n",
				sptr,
				sptr + 16,
				sptr + 57,
				sptr + 79,
				sptr + 68,
				sptr + 101,
				sptr + 89
			   );
		sptr = sort_read (fsort);

	}

	if (count)
	{
		fprintf (fout, "|--------------------------------------------");
		fprintf (fout, "-----------------------------------------------------");
		fprintf (fout, "----------------------------|\n");
	}
	fprintf (fout, "| Totals              ");
	fprintf (fout, "                                                    ");
	fprintf (fout, 
			 "  %9.9s | %10.2f | %10.2f | %11.2f |\n", 
			 " ",
			 curhQty, 
			 curhWgt, 
			 curhTot);

	/*---------------
	| work out rebate
	----------------*/
	fprintf (fout, "| Rebate Total");
	if (curhTot == (double) 0.00)
	{
		rebTot = 0.00;
		fprintf (fout, "  - Not Available %92.92s|\n", " ");
	}
	else
	{
		/*-------------------
		| work out break point
		----------------------*/

		for (count = 5; count >= 0; count--)
		{
			/*----------------------
			| on the basis of value
			-----------------------*/
			if (inrb_rec.basis [0] == 'V' &&
				curhTot >= inrb_rec.reb_qty [count])
			{
				if (inrb_rec.reb_qty [count] == 0.00)
					continue;

				/*-----------------------
				| if by value then return
				| rebTot = value
				------------------------*/
				if (inrb_rec.reb_type[0] == 'V')
					rebTot = inrb_rec.reb_value [count];

				if (inrb_rec.reb_type[0] == 'P')
					rebTot = curhTot * (inrb_rec.reb_value [count] / 100.00);
				break;
			}

			/*----------------------
			| on the basis of Units
			-----------------------*/
			if (inrb_rec.basis [0] == 'U' &&
				curhQty >= inrb_rec.reb_qty [count])
			{
				if (inrb_rec.reb_qty [count] == 0.00)
					continue;

				/*-----------------------
				| if by Units can Not be
				| rebate by percentage
				------------------------*/
				if (inrb_rec.reb_type[0] == 'V')
					rebTot = inrb_rec.reb_value [count];
				break;
			}

			/*----------------------
			| on the basis of Weight
			-----------------------*/
			if (inrb_rec.basis [0] == 'W' &&
				curhWgt >= inrb_rec.reb_qty [count])
			{
				if (inrb_rec.reb_qty [count] == 0.00)
					continue;

				/*-----------------------
				| if by Weight can Not be
				| rebate by percentage
				------------------------*/
				if (inrb_rec.reb_type[0] == 'V')
					rebTot = inrb_rec.reb_value [count];
				break;
			}
		}

		switch (inrb_rec.basis [0])
		{
		case 'U' :
			sprintf (err_str, "- Based On %-8.8s", "Quantity");
			break;
		case 'V' :
			sprintf (err_str, "- Based On %-8.8s", "Value");
			break;
		case 'W' :
			sprintf (err_str, "- Based On %-8.8s", "Weight");
			break;
		}
		fprintf (fout, "%-20.20s%78.78s| %11.2f |\n", err_str, " ", rebTot);
	}

	fprintf (fout, "|===========================================");
	fprintf (fout, "========================================================");
	fprintf (fout, "==========================|\n"); /*new*/
	fprintf (fout, "|%125.125s|\n", " ");

	*cusQty += curhQty;
	*cusWgt += curhWgt;
	*cusTot += curhTot;
	*TotReb += rebTot;
}

int
NotActive (
 void)
{
	static 	long	today = -1L;
	static 	int mdy [3];
	int	newmdy [3];
	long	vardate	= 0L;


	if (today == -1L)
	{
		today = TodaysDate ();
	}

	if (!REPRINT)
	{
		DateToDMY (inrb_rec.start_date, &mdy [0], &mdy [1], &mdy [2]);
	}
	else
	{
		vardate = inrb_rec.start_date;
		/*if (vardate >= local_rec.date)*/
		if (vardate >= today)
			return (TRUE);


		/*--------------------------
		| obtain start and end dates
		--------------------------*/
		/*while (vardate <= local_rec.date)*/
		while (vardate <= today)
		{
			/*---------------------
			| Read Until Greater Than
			| Then go back one cycle 
			| For end date and 2 cycles
			| For start date              
			----------------------*/
			DateToDMY (vardate, &mdy [0], &mdy [1], &mdy [2]);
			mdy [1] += inrb_rec.cycle;
			if (mdy [1] > 12)
			{
				mdy [1] = mdy [1] % 12;
				mdy [2]++;   /* increment the year */
			}
			vardate = DMYToDate (mdy [0], mdy [1], mdy [2]);
		}

		/*----------------------
		| we have now found the 
		| cycle which is greater
		| than the date required
		| so now subtract one cycle
		| for toDate and a further one
		| for the fromDate
		-------------------------------*/
		DateToDMY (vardate, &mdy [0], &mdy [1], &mdy [2]);
		mdy [1] -= inrb_rec.cycle;
		if (mdy [1] < 1)
		{
			mdy [1] = mdy [1] + 12;
			mdy [2]--;   /* decrement the year */
		}
		toDate = DMYToDate (mdy [0], mdy [1], mdy [2]);

		DateToDMY (toDate, &mdy [0], &mdy [1], &mdy [2]);
		mdy [1] -= inrb_rec.cycle;
		if (mdy [1] < 1)
		{
			mdy [1] = mdy [1] + 12;
			mdy [2]--;   /* decrement the year */
		}
		fromDate = DMYToDate (mdy [0], mdy [1], mdy [2]);
		if (fromDate < inrb_rec.start_date ||
			fromDate > inrb_rec.end_date)
			return (TRUE);

		return (FALSE);
	}

	/*-------------------------------------------------------------
	| *****   SHOULD NEVER GET THIS FAR IF IT IS REPRINT    *******
	-------------------------------------------------------------*/

	newmdy [0] = mdy [0];
	newmdy [1] = mdy [1];
	newmdy [2] = mdy [2];

	/*------------------
	| Add number of cycles
	| from inrb
	| to get fromDate if
	| not reprint
	--------------------*/
	if (!REPRINT)
		newmdy [1] += inrb_rec.cycle * inrb_rec.last_cycle;

	if (newmdy [1] > 12)
	{
		newmdy [1] = newmdy [1] % 12;
		newmdy [2]++;   /* increment the year */
	}

	/*-------------------------
	| convert back to long date
	-------------------------*/
	if (!REPRINT)
		fromDate = DMYToDate (newmdy [0], newmdy [1], newmdy [2]);


	/*---------------------------------
	| compare date to start processing
	| from with start and end date
	--------------------------------*/
	if (today < fromDate || fromDate > inrb_rec.end_date)
			return (TRUE);
	
	/*------------------
	| Add one cycle
	| to get toDate
	--------------------*/
	newmdy [1] += inrb_rec.cycle;

	if (newmdy [1] > 12)
	{
		newmdy [1] = newmdy [1] % 12;
		newmdy [2]++;   /* increment the year */
	}

	/*-------------------------
	| convert back to long date
	-------------------------*/
	toDate = DMYToDate (newmdy [0], newmdy [1], newmdy [2]);

	/*-------------------
	| today has to be 
	| greater than toDate
	---------------------*/
	if (today <= toDate)
		return (TRUE);

	return (FALSE);
}

