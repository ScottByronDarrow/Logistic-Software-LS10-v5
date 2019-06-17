/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( fe_contmnt.c   )                                 |
|  Program Desc  : ( Maintain/Close/Display Forward Exchange Contracts|
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, fehr, feln, bkcr, crbk,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  fehr,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 02/08/94         |
|---------------------------------------------------------------------|
|  Date Modified : (22/08/94)      | Modified  by : Dirk Heinsius.    |
|  Date Modified : (11/09/97)      | Modified  by : Ana Marie Tario.  |
|                                                                     |
|  Comments      :                                                    |
|  (22/08/94)    : FCS 10796 - Update files upon closure option.      |
|  (11/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
| $Log: fe_contmnt.c,v $
| Revision 5.4  2002/07/25 11:17:28  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2002/07/05 03:50:02  kaarlo
| S/C 4035. Updated to change bank column from 4 chars to 5 chars.
|
| Revision 5.2  2001/08/09 09:13:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:24:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:58  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:26:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:39  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:56:16  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/01/17 05:24:44  ramon
| For GVision compatibility, I moved the position of the descriptions 3 chars. to the right.
|
| Revision 1.12  1999/12/06 01:46:58  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/17 06:40:05  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/08 05:00:57  scott
| Updated due to warnings when using -Wall compile flag.
|
| Revision 1.9  1999/10/01 07:48:37  scott
| Updated for standard function calls.
|
| Revision 1.8  1999/09/29 10:10:40  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:26:29  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 07:18:54  scott
| Updated from Ansi Project
|
| Revision 1.4  1999/06/15 00:12:12  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "fe_contmnt";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FE/fe_contmnt/fe_contmnt.c,v 5.4 2002/07/25 11:17:28 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_fe_mess.h>

#define	MAINT	(rep_type[0] == 'M')
#define	CLOSE	(rep_type[0] == 'C')
#define	ENQUIRE	(rep_type[0] == 'E')

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int  	newContract = 0;            

	char	rep_type[2];

	char	*data	= "data";
	char	*bkcr	= "bkcr";
	char	*crbk	= "crbk";
	char	*fehr	= "fehr";
	char	*feln	= "feln";
	char	*fetr	= "fetr";

	FILE	*pout;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"}
	};
	
	int comm_no_fields = 4;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
	} comm_rec;


	/*==============================
	| Forward Exchange Header file |
	==============================*/
	struct dbview fehr_list [] =
	{
		{"fehr_co_no"},
		{"fehr_cont_no"},
		{"fehr_hhfe_hash"},
		{"fehr_bank_id"},
		{"fehr_curr_code"},
		{"fehr_date_wef"},
		{"fehr_date_exp"},
		{"fehr_val_orig"},
		{"fehr_val_avail"},
		{"fehr_stat_flag"},
		{"fehr_exch_rate"},
		{"fehr_buy_sell"}
	};

	int	fehr_no_fields = 12;

	struct tag_fehrRecord
	{
		char	co_no [3];
		char	cont_no [7];
		long	hhfe_hash;
		char	bank_id [6];
		char	curr_code [4];
		long	date_wef;
		long	date_exp;
/* money */	double	val_orig;
/* money */	double	val_avail;
		char	stat_flag [2];
		double	exch_rate;
		char	buy_sell [2];
	} fehr_rec;


	/*==================================
	| Forward Exchange Assignment File |
	==================================*/
	struct dbview feln_list [] =
	{
		{"feln_hhfe_hash"},
		{"feln_index_by"},
		{"feln_index_hash"},
		{"feln_value"}
	};

	int	feln_no_fields = 4;

	struct tag_felnRecord
	{
		long	hhfe_hash;
		char	index_by[2];
		long	index_hash;
		double	value;
	} feln_rec;


	/*===================================
	| Forward Exchange Transaction File |
	===================================*/
	struct dbview fetr_list [] =
	{
		{"fetr_hhfe_hash"},
		{"fetr_index_by"},
		{"fetr_index_hash"},
		{"fetr_hhcp_hash"},
		{"fetr_value"}
	};

	int	fetr_no_fields = 5;

	struct tag_fetrRecord
	{
		long	hhfe_hash;
		char	index_by[2];
		long	index_hash;
		long	hhcp_hash;
		double	value;		/* money */
	} fetr_rec;

	/*======================
	| Currency File Record |
	======================*/
	struct dbview bkcr_list [] =
	{
		{"bkcr_co_no"},
		{"bkcr_bank_id"},
		{"bkcr_curr_code"},
		{"bkcr_description"},
		{"bkcr_ex1_factor"},
	};

	int	bkcr_no_fields = 5;

	struct tag_bkcrRecord
	{
		char	co_no [3];
		char	bank_id [6];
		char	curr_code [4];
		char	description [41];
		double	exch_rate;
	} bkcr_rec;

	/*=====================
	| Creditors Bank File |
	=====================*/
	struct dbview crbk_list [] =
	{
		{"crbk_co_no"},
		{"crbk_bank_id"},
		{"crbk_bank_name"},
		{"crbk_curr_code"},
	};

	int	crbk_no_fields = 4;

	struct tag_crbkRecord
	{
		char	co_no [3];
		char	bank_id [6];
		char	bank_name [41];
		char	curr_code [4];
	} crbk_rec;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	long	lsystemDate;
	char	systemDate[11];
	char 	bank_desc[41];
	char 	curr_desc[41];
	char 	buy_sell_desc[5];
	char 	active_desc[7];
	double	amt_available;		/* Contract amount less amt_transacted	*/
	double	amt_transacted;		/* Amount used. Sum of fetr amounts		*/
	double	amt_committed;		/* Amount committed. Sum of feln amounts*/
	double	amt_remaining;		/* Contract amount less amt_transacted	*/
								/* less amt_committed.					*/
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "cont_no",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Contract No    : ", "Enter Contract Number. Search available",
		NE, NO,  JUSTLEFT, "", "", fehr_rec.cont_no},
	{1, LIN, "bank_id",	 5, 18, CHARTYPE,
		"UUUUU", "          ",
		" ", "     ", "Bank Id        : ", "Enter Bank Code. Search available",
		YES, NO,  JUSTLEFT, "", "", fehr_rec.bank_id},
	{1, LIN, "bank_desc",	 5, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.bank_desc},
	{1, LIN, "date_wef",		 7, 18, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.systemDate, "Effective Date : ", "Enter date on which contract becomes effective.",
		YES, NO, JUSTRIGHT, "", "", (char*)&fehr_rec.date_wef},
	{1, LIN, "date_exp",		 7, 49, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", "00/00/00", "Expiry Date    : ", "Enter date on which contract expires.",
		YES, NO, JUSTRIGHT, "", "", (char*)&fehr_rec.date_exp},
	{1, LIN, "curr_code",	 9, 18, CHARTYPE,
		"UUU", "          ",
		" ", "   ", "Currency       : ", "Enter Currency. Search available.",
		YES, NO,  JUSTLEFT, "", "", fehr_rec.curr_code},
	{1, LIN, "curr_desc",	 9, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},
	{1, LIN, "exch_rate",	 10, 18, DOUBLETYPE,
		"NNNN.NNNN", "          ",
		" ", "", "Exchange Rate  : ", " ",
		YES, NO, JUSTRIGHT, "", "", (char*)&fehr_rec.exch_rate},
	{1, LIN, "amt_orig",	 11, 18, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "", "Contract Amt   : ", "Enter Amount of contract in contract currency.",
		YES, NO, JUSTRIGHT, "", "", (char*)&fehr_rec.val_orig},
	{1, LIN, "amt_transacted",	 12, 18, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "", "Transacted Amt : ", " ",
		NA, NO, JUSTRIGHT, "", "", (char*)&local_rec.amt_transacted},
	{1, LIN, "amt_available",	 13, 18, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "", "Available Amt  : ", " ",
		NA, NO, JUSTRIGHT, "", "", (char*)&local_rec.amt_available},
	{1, LIN, "amt_committed",	 14, 18, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "", "Committed Amt  : ", " ",
		NA, NO, JUSTRIGHT, "", "", (char*)&local_rec.amt_committed},
	{1, LIN, "amt_remaining",	 15, 18, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "", "Remaining Amt  : ", " ",
		NA, NO, JUSTRIGHT, "", "", (char*)&local_rec.amt_remaining},
	{1, LIN, "buy_sell_type",	 16, 18, CHARTYPE,
		"U", "          ",
		" ", "S", "Buy / Sell     : ", "B(uying) or S(elling) contract.",
		YES, NO,  JUSTLEFT, "BS", "", fehr_rec.buy_sell},
	{1, LIN, "buy_sell_desc",	 16, 21, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.buy_sell_desc},
	{1, LIN, "active_type",	 18, 18, CHARTYPE,
		"U", "          ",
		" ", "A", "Status         : ", "Contract status, A(ctive) or C(losed)",
		NA, NO,  JUSTLEFT, "AC", "", fehr_rec.stat_flag},
	{1, LIN, "active_desc",	 18, 21, CHARTYPE,
		"AAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.active_desc},
		
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=====================
| Function Prototypes |
=====================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void LoadAmounts (void);
void LoadDescriptions (void);
int spec_valid (int field);
int Update (void);
int heading (int scn);
void SrchFehr (char *key_val);
void SrchCrbk (char *key_val);
void SrchBkcr (char *key_val);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char *argv[])
{
	int		i;

	if (argc != 2)
	{
/*
		printf("Usage : %s < M(aintenance) C(losure) E(nquiry) >\007\n\r",argv[0]);*/
		print_at(0,0,ML(mlFeMess021),argv[0]);
		return (EXIT_FAILURE); /*exit(argc);*/
	}

	sprintf (rep_type, "%-1.1s", argv[1]);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR(vars);

	if (ENQUIRE)
	{
		for (i = label("bank_id"); i <= label("active_desc"); i++)
			vars[i].required = NA;
	}
	if (CLOSE)
	{
		for (i = label("bank_id"); i <= label("buy_sell_desc"); i++)
			vars[i].required = NA;
		FLD ("active_type") = YES;
	}

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0; 
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading(1);
		entry(1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading(1);
		scn_display(1);
		edit(1);

		if (restart)
			continue;

		if ((MAINT || CLOSE) && !restart)
			Update();

	}

	shutdown_prog();
    return (EXIT_SUCCESS);
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (bkcr, bkcr_list, bkcr_no_fields, "bkcr_id_no");
	open_rec (crbk, crbk_list, crbk_no_fields, "crbk_id_no");
	open_rec (fehr, fehr_list, fehr_no_fields, "fehr_id_no");
	open_rec (feln, feln_list, feln_no_fields, "feln_hhfe_hash");
	open_rec (fetr, fetr_list, fetr_no_fields, "fetr_hhfe_hash");
}



/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (bkcr);
	abc_fclose (crbk);
	abc_fclose (fehr);
	abc_fclose (feln);
	abc_fclose (fetr);

	abc_dbclose (data);
}

void
LoadAmounts (
 void)
{
	local_rec.amt_transacted = 0.00;
	local_rec.amt_committed  = 0.00;
	local_rec.amt_remaining  = 0.00;
	local_rec.amt_available  = 0.00;

	if (!newContract)
	{
		feln_rec.hhfe_hash = fehr_rec.hhfe_hash;
		cc = find_rec (feln, &feln_rec, GTEQ, "r");
    	while (!cc  && feln_rec.hhfe_hash == fehr_rec.hhfe_hash)
		{
			local_rec.amt_committed += feln_rec.value;
			cc = find_rec (feln, &feln_rec, NEXT, "r");
		}
	
		fetr_rec.hhfe_hash = fehr_rec.hhfe_hash;
		cc = find_rec (fetr, &fetr_rec, GTEQ, "r");
   		while (!cc  && fetr_rec.hhfe_hash == fehr_rec.hhfe_hash)
		{
			local_rec.amt_transacted += fetr_rec.value;
			cc = find_rec (fetr, &fetr_rec, NEXT, "r");
		}
	}

	local_rec.amt_available = fehr_rec.val_orig - local_rec.amt_transacted;
	local_rec.amt_remaining = fehr_rec.val_orig - local_rec.amt_transacted
											    - local_rec.amt_committed;
}


void
LoadDescriptions (
 void)
{
	/*-----------------------
	| Load bank description |
	-----------------------*/
	strcpy (crbk_rec.co_no,   comm_rec.tco_no);
	strcpy (crbk_rec.bank_id, fehr_rec.bank_id);
	cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
	if (cc)
	{
/*
		sprintf (err_str, "\007 Warning - Bank %s is not on file.", crbk_rec.bank_id);
		print_mess (err_str);*/
		print_mess(ML(mlStdMess043));
		sleep (sleepTime);
		clear_mess ();
		sprintf (local_rec.bank_desc, "%-40.40s", "Unknown Bank");
	}
	else
		sprintf (local_rec.bank_desc, "%-40.40s", crbk_rec.bank_name);
	DSP_FLD ("bank_desc");

	/*---------------------------
	| Load currency description |
	---------------------------*/
	strcpy (bkcr_rec.co_no,     comm_rec.tco_no);
	strcpy (bkcr_rec.bank_id,   fehr_rec.bank_id);
	strcpy (bkcr_rec.curr_code, fehr_rec.curr_code);
	cc = find_rec (bkcr, &bkcr_rec, EQUAL, "r");
	if (cc)
	{
/*
		sprintf (err_str, "\007 Currency %s is not on file for this bank.",
						   bkcr_rec.curr_code);
		print_mess (err_str);*/
		print_mess(ML(mlStdMess040));
		sleep (sleepTime);
		clear_mess ();
		sprintf (local_rec.bank_desc, "%-40.40s", "Unknown Currency");
	}
	else
		sprintf (local_rec.curr_desc, "%-40.40s", bkcr_rec.description);
	DSP_FLD ("curr_desc");

	/*---------------------------
	| Load buy_sell description |
	---------------------------*/
	switch (fehr_rec.buy_sell[0])
	{
		case 'B' : strcpy (local_rec.buy_sell_desc, "Buy ");
				   break;
		case 'S' : strcpy (local_rec.buy_sell_desc, "Sell");
				   break;
		default  : strcpy (local_rec.buy_sell_desc, fehr_rec.buy_sell);
				   break;
	}
	DSP_FLD ("active_desc");


	/*-------------------------
	| Load active description |
	-------------------------*/
	switch (fehr_rec.stat_flag[0])
	{
		case 'A' : strcpy (local_rec.active_desc, "Active"); 
				   break;
		case 'C' : strcpy (local_rec.active_desc, "Closed");
				   break;
		default  : strcpy (local_rec.active_desc, fehr_rec.stat_flag);
				   break;
	}
	DSP_FLD ("active_desc");
}


int
spec_valid (
 int field)
{
    if (LCHECK ("cont_no"))
	{
		if (dflt_used)
		{
			/*print_mess ("\007 No default available");*/
			print_mess(ML(mlStdMess007));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchFehr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		
		strcpy (fehr_rec.co_no, comm_rec.tco_no);
		if (MAINT)
		{
			cc = find_rec (fehr, &fehr_rec, EQUAL, "w");
			if (cc)
			{
				newContract = TRUE;
				strcpy (fehr_rec.stat_flag, "A");

				FLD ("bank_id") 		= YES;
				FLD ("date_wef") 		= YES;
				FLD ("curr_code") 		= YES;
				FLD ("exch_rate") 		= YES;
				FLD ("buy_sell_type") 	= YES;
				FLD ("active_type") 	= YES;
				LoadAmounts ();
			}
			else
			{
				newContract = FALSE;
				entry_exit = TRUE;

				LoadDescriptions ();
				LoadAmounts ();
				if (local_rec.amt_committed > 0.00
				||  local_rec.amt_transacted > 0.00)
				{
					FLD ("bank_id") 		= NA;
					FLD ("date_wef") 		= NA;
					FLD ("curr_code") 		= NA;
					FLD ("exch_rate") 		= NA;
					FLD ("buy_sell_type") 	= NA;
					FLD ("active_type") 	= NA;
				}
				else
				{
					FLD ("bank_id") 		= NA;
					FLD ("date_wef") 		= YES;
					FLD ("curr_code") 		= YES;
					FLD ("exch_rate") 		= YES;
					FLD ("buy_sell_type") 	= YES;
					FLD ("active_type") 	= YES;
				}
			}
		}
		else
		{
			cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
			if (cc)
			{
/*
				sprintf (err_str, "\007 Contract %s is not on file.", 
						 fehr_rec.cont_no);
				print_mess (err_str);*/
				print_mess(ML(mlStdMess075));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			newContract = FALSE;
			LoadDescriptions ();
			LoadAmounts ();
			entry_exit = TRUE;
		}
		return (EXIT_SUCCESS);
	}


    if (LCHECK ("bank_id"))
	{
		if (dflt_used)
		{
			/*print_mess ("\007 No default available");*/
			print_mess(ML(mlStdMess007));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}
		
		
		strcpy (crbk_rec.co_no,   comm_rec.tco_no);
		strcpy (crbk_rec.bank_id, fehr_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, "\007 Bank %s is not on file.", crbk_rec.bank_id);
			print_mess (err_str);*/
			print_mess(ML(mlStdMess043));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.bank_desc, "%-40.40s", crbk_rec.bank_name);
		DSP_FLD ("bank_desc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("date_wef"))
	{
		if (prog_status != ENTRY && fehr_rec.date_wef > fehr_rec.date_exp)
		{
			/*print_mess ("\007Effective Date  > Expiry Date.");*/
			print_mess(ML(mlFeMess022));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		
		if (fehr_rec.date_wef < local_rec.lsystemDate)
		{
			/*print_mess ("\007Warning - Effective Date < Current Date");*/
			print_mess(ML(mlFeMess023));
			sleep (sleepTime);
			clear_mess ();
		}
		return(0);             
	}


	if (LCHECK("date_exp"))
	{
		if (fehr_rec.date_exp <= fehr_rec.date_wef)
		{
			/*print_mess ("\007Expiry Date  <= Effective Date.");*/
			print_mess(ML(mlFeMess024));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);             
	}


    if (LCHECK ("curr_code"))
	{
		if (dflt_used)
		{
			strcpy (fehr_rec.curr_code, crbk_rec.curr_code);
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchBkcr (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (bkcr_rec.co_no,     comm_rec.tco_no);
		strcpy (bkcr_rec.bank_id,   fehr_rec.bank_id);
		strcpy (bkcr_rec.curr_code, fehr_rec.curr_code);
		cc = find_rec (bkcr, &bkcr_rec, EQUAL, "r");
		if (cc)
		{
/*
			sprintf (err_str, "\007 Currency %s is not on file for this bank.",
							   bkcr_rec.curr_code);*/
			print_mess (ML(mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.curr_desc, "%-40.40s", bkcr_rec.description);
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("amt_orig"))
	{
		if (fehr_rec.val_orig < (local_rec.amt_transacted + local_rec.amt_committed))
		{
			/*print_mess ("\007 Contract amount is less than the amount already allocated");*/
			print_mess(ML(mlFeMess027));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.amt_available = fehr_rec.val_orig - local_rec.amt_transacted;
		local_rec.amt_remaining = fehr_rec.val_orig - local_rec.amt_transacted
											    - local_rec.amt_committed;
		DSP_FLD ("amt_transacted");
		DSP_FLD ("amt_available");
		DSP_FLD ("amt_committed");
		DSP_FLD ("amt_remaining");
	}



    if (LCHECK ("buy_sell_type"))
	{
		switch (fehr_rec.buy_sell[0])
		{
			case 'B' : {
						/*print_mess ("\007 Forward Exchange contracts are not yet available for purchasing");*/
						print_mess(ML(mlFeMess025));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					   }
			case 'S' : strcpy (local_rec.buy_sell_desc, "Sell");
					   break;
			default  : {
						/*print_mess ("\007 Invalid buy-sell type");*/
						print_mess(ML(mlFeMess026));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					   }
		}
		DSP_FLD ("buy_sell_desc");
		return (EXIT_SUCCESS);
	}


    if (LCHECK ("active_type"))
	{
		switch (fehr_rec.stat_flag[0])
		{
			case 'A' : strcpy (local_rec.active_desc, "Active"); 
					   break;
			case 'C' : strcpy (local_rec.active_desc, "Closed");
					   break;
			default  : {
						print_mess(ML(mlFeMess028));
					/*	print_mess ("\007 Invalid active status");*/
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					   }
		}
		DSP_FLD ("active_desc");
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);             
}




/*===============================
| Update Forward Exchange file. |
===============================*/
int
Update (
 void)
{
	if (newContract)
	{
		fehr_rec.val_avail = local_rec.amt_available;
		cc = abc_add (fehr, &fehr_rec);
		if (cc)
			file_err (cc, fehr, "DBADD");
	}
	else
	{
		fehr_rec.val_avail = local_rec.amt_available;
		cc = abc_update (fehr, &fehr_rec);
		if (cc)
			file_err (cc, fehr, "DBFIND");
	}
	abc_unlock (fehr);
	return (EXIT_SUCCESS);
}



/*=========================
| Display Screen Heading. |
=========================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		move (0, 1);
		line (80);

		if (MAINT)
		{
			/*rv_pr(" Create Forward Exchange Contract.", 24, 0, 1);*/
			rv_pr(ML(mlFeMess001), 24, 0, 1);
			box (0, 3, 80, 15);
		}

		if (CLOSE)
		{
			/*rv_pr(" Close Forward Exchange Contract.", 24, 0, 1);*/
			rv_pr(ML(mlFeMess002), 24, 0, 1);
			box (0, 3, 80, 15);
		}

		if (ENQUIRE)
		{
			/*rv_pr(" Forward Exchange Contract Enquiry.", 24, 0, 1);*/
			rv_pr(ML(mlFeMess003), 24, 0, 1);
			box (0, 3, 80, 15);
		}

		move (1, 6);
		line (79);
		move (1, 8);
		line (79);
		move (1, 17);
		line (79);
		move (0, 20);
		line (80);

/*
		print_at(21,0," Company no. : %s   %s", comm_rec.tco_no, comm_rec.tco_name);*/
		print_at(21,0,ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);

		move (0, 22);
		line (80);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return 0;
}




/*================
| Search for FEC |
================*/
void
SrchFehr (
 char *key_val)
{
    work_open ();
	save_rec ("#Cont No","#Bank    Currency ");                       
	strcpy  (fehr_rec.co_no,   comm_rec.tco_no);
	sprintf (fehr_rec.cont_no, "%-3.3s", key_val);
	cc = find_rec (fehr, &fehr_rec, GTEQ, "r");
    while (!cc 
	&&     !strcmp (fehr_rec.co_no, comm_rec.tco_no) 
	&&     !strncmp(fehr_rec.cont_no, key_val, strlen (key_val)))
    {
		sprintf (err_str, "%-5.5s   %-3.3s", fehr_rec.bank_id, 
											 fehr_rec.curr_code);
	    cc = save_rec (fehr_rec.cont_no, err_str);                       
		if (cc)
			break;
		cc = find_rec (fehr, &fehr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy  (fehr_rec.co_no,   comm_rec.tco_no);
	sprintf (fehr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, fehr, "DBFIND");
}


/*=========================================
| Search routine for Creditors Bank File. |
=========================================*/
void
SrchCrbk (
 char *key_val)
{
	work_open ();
	strcpy (crbk_rec.co_no,   comm_rec.tco_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = save_rec ("#Bank Id ","#Bank Name             ");
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc 
	&& 	   !strncmp (crbk_rec.bank_id, key_val, strlen (key_val)) 
	&&     !strcmp  (crbk_rec.co_no,   comm_rec.tco_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no,   comm_rec.tco_no);
	strcpy (crbk_rec.bank_id, temp_str);
	cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND" );
}


/*=========================================
| Search routine for Creditors Bank File. |
=========================================*/
void
SrchBkcr (
 char *key_val)
{
	work_open ();
	strcpy (bkcr_rec.co_no,     comm_rec.tco_no);
	strcpy (bkcr_rec.bank_id,   fehr_rec.bank_id);
	strcpy (bkcr_rec.curr_code, key_val);
	cc = save_rec ("#Currency ","#Description             ");
	cc = find_rec (bkcr, &bkcr_rec, GTEQ, "r");
	while (!cc 
	&& 	   !strncmp (bkcr_rec.curr_code, key_val, strlen (key_val)) 
	&&     !strcmp  (bkcr_rec.bank_id,   fehr_rec.bank_id)
	&&     !strcmp  (bkcr_rec.co_no,     comm_rec.tco_no))
	{
		cc = save_rec (bkcr_rec.curr_code, bkcr_rec.description);
		if (cc)
			break;
		cc = find_rec (bkcr, &bkcr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (bkcr_rec.co_no,     comm_rec.tco_no);
	strcpy (bkcr_rec.bank_id,   fehr_rec.bank_id);
	strcpy (bkcr_rec.curr_code, temp_str);
	cc = find_rec (bkcr, &bkcr_rec, EQUAL, "r");
	if (cc) 
		file_err (cc, bkcr, "DBFIND" );
}

