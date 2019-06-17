/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_sreb_mnt.c,v 5.1 2002/08/01 01:43:34 scott Exp $
|  Program Name  : (sk_sreb_mnt.c)
|  Program Desc  : (Supplier Rebate Maintenance)
|---------------------------------------------------------------------|
|  Author        : Irfan Gohir.    | Date Written  : 26/10/93         |
|---------------------------------------------------------------------|
| $Log: sk_sreb_mnt.c,v $
| Revision 5.1  2002/08/01 01:43:34  scott
| Updated to convert to app.schema and clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sreb_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sreb_mnt/sk_sreb_mnt.c,v 5.1 2002/08/01 01:43:34 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct inrbRecord	inrb_rec;
struct sumrRecord	sumr_rec;

	double	*reb_qty	=	&inrb_rec.reb_qty1;
	double	*reb_val	=	&inrb_rec.reb_val1;

	char	*data   = "data";

/* Globals */

int		envDbCo;
int		cr_find;
int		envDbFind = FALSE;
int		updateFlag;
char	rebateCode [6];
char	sysDate [11];
long	scHash;
char	rebFlag [2];
long	todayDate;
int		tempCycle = 0;
char	qtyPrompt [6][31];
char	qtyComment [6][31];

struct
{
	char	basisDesc [8];
	char	rebTypeDesc [12];
	char	dummy [11];
}local_rec;
	
static	struct	var	vars [] =
{
	{1, LIN, "crd_no",	3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Supplier             ", "Enter Supplier Number, Full Search Available",
		NE, NO, JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "crd_name",	3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "dbt_no",	3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Customer             ", "Enter Customer Number, Full Search Available",
		NE, NO, JUSTLEFT, "", "", cumr_rec.dbt_no},
	{1, LIN, "dbt_name",	3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "reb_code",	4, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Rebate Code          ", "Enter Rebate Code, Full Search Available",
		NE, NO, JUSTLEFT, "", "", inrb_rec.reb_code},
	{1, LIN, "cycle",	5, 2, INTTYPE,
		"NN", "         ",
		" ", " ", "Cycle Duration       ", "Enter Cycle (in months), Full Search Available",
		NE, NO, JUSTLEFT, "", "", (char *)&inrb_rec.cycle},
	{1, LIN, "reb_desc",	6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description          ", "",
		 YES, NO,  JUSTLEFT, "", "", inrb_rec.description},
	{1, LIN, "basis",	8, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Basis                ", "Enter Basis  V)alue, U)nits, W)eight. Default = V)alue",
		 YES, NO,  JUSTLEFT, "VUW", "", inrb_rec.basis},
	{1, LIN, "basisdesc",	8, 26, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.basisDesc},
	{1, LIN, "reb_type",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "V", "Rebate Type          ", "Enter Rebate Type V)alue, P)ercentage. Default = V)alue",
		 YES, NO,  JUSTLEFT, "PV", "", inrb_rec.reb_type},
	{1, LIN, "rebtypedesc",	9, 26, CHARTYPE,
		"AAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.rebTypeDesc},
	{1, LIN, "ant_reb_pc",	10, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ",  "Anticipated Rebate % ", "Enter Anticipated Rebate Percent",
		YES, NO, JUSTLEFT, "", "", (char *)&inrb_rec.ant_reb_pc},
	{1, LIN, "start_date",	8, 40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date           ", "",
		YES, NO, JUSTLEFT, "", "", (char *)&inrb_rec.start_date},
	{1, LIN, "end_date",	9, 40, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date             ", "",
		YES, NO, JUSTLEFT, "", "", (char *)&inrb_rec.end_date},
	{1, LIN, "rq1",	 13, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [0], qtyComment [0],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty1},
	{1, LIN, "rv1",	 13, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",  "Value 1.             ", "Enter Rebate Value 1",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val1},
	{1, LIN, "rp1",	 13, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",   "Percentage 1.        ", "Enter Rebate Percentage 1",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val1},
	{1, LIN, "rq2",	 14, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [1], qtyComment [1],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty2},
	{1, LIN, "rv2",	 14, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",   "Value 2.             ", "Enter Rebate Value 2",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val2},
	{1, LIN, "rp2",	 14, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",   "Percentage 2.        ", "Enter Rebate Percentage 2",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val2},
	{1, LIN, "rq3",	 15, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [2], qtyComment [2],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty3},
	{1, LIN, "rv3",	 15, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",   "Value 3.             ", "Enter Rebate Value 3",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val3},
	{1, LIN, "rp3",	 15, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",  "Percentage 3.        ", "Enter Rebate Percentage 3",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val3},
	{1, LIN, "rq4",	 16, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [3], qtyComment [3],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty4},
	{1, LIN, "rv4",	 16, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",  "Value 4.             ", "Enter Rebate Value 4",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val4},
	{1, LIN, "rp4",	 16, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",  "Percentage 4.        ", "Enter Rebate Percentage 4",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val4},
	{1, LIN, "rq5",	 17, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [4], qtyComment [4],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty5},
	{1, LIN, "rv5",	 17, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",  "Value 5.             ", "Enter Rebate Value 5",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val5},
	{1, LIN, "rp5",	 17, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",  "Percentage 5.        ", "Enter Rebate Percentage 5",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val5},
	{1, LIN, "rq6",	 18, 2, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "0", qtyPrompt [5], qtyComment [5],
		 YES, NO,  JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_qty6},
	{1, LIN, "rv6",	 18, 40, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		"", "",  "Value 6.             ", "Enter Rebate Value 6",
		 YES, NO, JUSTRIGHT, "0", "9999999999.99", (char *) &inrb_rec.reb_val6},
	{1, LIN, "rp6",	 18, 40, DOUBLETYPE,
		"NN.NN", "          ",
		"", "",  "Percentage 6.        ", "Enter Rebate Percentage 6",
		 ND, NO, JUSTRIGHT, "0", "99.99", (char *) &inrb_rec.reb_val6},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

extern	int	TruePosition;
#include <FindSumr.h>
#include <FindCumr.h>
/*
 * Function Declarations 
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	Initialise 		(char *);
int  	spec_valid 		(int);
void 	SetPrompt 		(void);
void 	SrchRebCode 	(char *);
void 	SrchCycle 		(char *);
int  	heading 		(int);
void 	InitVars 		(void);
void 	Update 			(void);
void 	shutdown_prog 	(void);

	int		envVarDbCo		=	0;
	char	branchNumber [3];

/*
 * Main Processing Routine. 
 */
int
main (
	int 	argc, 
	char 	*argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	TruePosition = TRUE;

	init_scr ();
	set_tty (); 

	Initialise (argv [0]);

	/*
	 * Read common terminal record. 
	 */
	OpenDB ();

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

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
		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind)
						? "cumr_id_no" : "cumr_id_no3");
	open_rec (inrb, inrb_list, INRB_NO_FIELDS, "inrb_id_no1");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!cr_find) 
						? "sumr_id_no" : "sumr_id_no3");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (inrb);	
	abc_fclose (sumr);	
	abc_fclose (cumr);	
	abc_dbclose (data);
}

void
Initialise (
	char	*name)
{
	int	 fieldNum;
	char *sptr;


	memset (&inrb_rec, 0, sizeof (inrb_rec));
	memset (&local_rec, 0, sizeof (local_rec));

	strcpy (sysDate, DateToString (TodaysDate ()));
	inrb_rec.start_date = TodaysDate ();
	todayDate = TodaysDate ();

	sptr = strrchr (name, 'c');

	if (sptr)
	{
		FLD ("crd_no") 		= ND;
		FLD ("crd_name") 	= ND;
		strcpy (inrb_rec.reb_flag, "C");
		strcpy (rebFlag, "C");
		envDbCo = atoi (get_env ("DB_CO"));
		envDbFind = atoi (get_env ("DB_FIND"));
	}
	else
	{ 
		FLD ("dbt_no") 		= ND;
		FLD ("dbt_name") 	= ND;
		strcpy (inrb_rec.reb_flag, "S");
		strcpy (rebFlag, "S");
		envDbCo = atoi (get_env ("CR_CO"));
		cr_find = atoi (get_env ("CR_FIND"));
	}
	
	for (fieldNum = 0; fieldNum < 6; fieldNum++)
	{
		sprintf (qtyPrompt [fieldNum], "Quantity %1.1d.       ", fieldNum+1);
		sprintf (qtyComment [fieldNum], "Enter Rebate Quantity %1.1d.", fieldNum+1);
	}

}

int
spec_valid (
 int field)
{
	int 	i;
	int 	count;

	/*
	 * Validate Customer Number And Allow Search. 
	 */
	if (LCHECK ("dbt_no"))
	{
		if (FLD ("dbt_no") == ND)
			return (EXIT_SUCCESS);

		strncpy (cumr_rec.co_no, comm_rec.co_no, 2);
		strncpy (cumr_rec.est_no, branchNumber, 2);
		pad_num (cumr_rec.dbt_no);


		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			scHash = cumr_rec.hhcu_hash;
			return (EXIT_SUCCESS);
		}

		cc = find_rec (cumr, &cumr_rec, COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		scHash = cumr_rec.hhcu_hash;
		DSP_FLD ("dbt_name");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Creditor Number And Allow Search. 
	 */
	if (LCHECK ("crd_no"))
	{
		if (FLD ("crd_no") == ND)
			return (EXIT_SUCCESS);

		strncpy (sumr_rec.co_no, comm_rec.co_no, 2);
		strncpy (sumr_rec.est_no, comm_rec.est_no, 2);
		pad_num (sumr_rec.crd_no);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			scHash = sumr_rec.hhsu_hash;
			return (EXIT_SUCCESS);
		}

		cc = find_rec (sumr, &sumr_rec, COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		scHash = sumr_rec.hhsu_hash;
		DSP_FLD ("crd_name");
		return (EXIT_SUCCESS);
	}

	/*
	 * Allow Search On Rebate Code For Supplier/Cust.  
	 */
	if (LCHECK ("reb_code"))
	{
		inrb_rec.link_hash = scHash; 

		if (SRCH_KEY)
		{
			SrchRebCode (temp_str);
			strncpy (rebateCode, inrb_rec.reb_code, 5);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_FAILURE);
		else
		{
			strncpy (rebateCode, inrb_rec.reb_code, 5);
			return (EXIT_SUCCESS);
		}
	}

	/*
	 * Allow Search On Cycle Within Rebate Code.  
	 */
	if (LCHECK ("cycle"))
	{
		updateFlag = FALSE;

		if (SRCH_KEY)
		{
			SrchCycle (temp_str);
			updateFlag = TRUE;
			return (EXIT_SUCCESS);
		}

		if (dflt_used && tempCycle)
		{
			SetPrompt ();
		 	inrb_rec.cycle = tempCycle;
			tempCycle = 0;
			updateFlag = TRUE;
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (inrb_rec.cycle < 1)
			return (EXIT_FAILURE);

		strncpy (inrb_rec.reb_flag, rebFlag, 1);
		cc = find_rec (inrb, &inrb_rec,COMPARISON,"u");
		if (!cc)
		{
			SetPrompt ();
			scn_display (1);
			updateFlag = TRUE;
			entry_exit = TRUE;
		}
		else
		{
			InitVars ();
			scn_display (1);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Default Start Date to Today If Adding Record 
	 */
	if (LCHECK ("start_date"))
	{
		if (dflt_used)
		{
			strcpy (sysDate, DateToString (TodaysDate ()));
			inrb_rec.start_date = StringToDate (sysDate);
			return (EXIT_SUCCESS);
		}

		if (inrb_rec.start_date == 0)
		{
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			

		if (todayDate > inrb_rec.start_date)
		{
			i = prmptmsg (ML (mlSkMess131),"YyNn",1, 20);
			move (1, 20);
			cl_line ();
			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			else
				return (EXIT_SUCCESS);
		}

		if (inrb_rec.end_date < inrb_rec.start_date && prog_status != ENTRY)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Check End Date Is Not Before Start Date      
	 */
	if (LCHECK ("end_date"))
	{
		if (inrb_rec.end_date == 0)
		{
			print_mess (ML (mlStdMess111));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inrb_rec.end_date < inrb_rec.start_date)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((inrb_rec.end_date - inrb_rec.start_date) < (inrb_rec.cycle * 30))
		{
			i = prmptmsg (ML (mlSkMess132),"YyNn",1, 20);
			move (1, 20);
			cl_line ();
			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			else
				return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Check Anticipated Rebate is within range 0 - 100 If Percentage 
	 */
	if (LCHECK ("ant_reb_pc"))
	{
		if (inrb_rec.ant_reb_pc < 0.0 ||
			inrb_rec.ant_reb_pc > 100.0)
		{
		    print_mess (ML (mlSkMess133));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Expand Basis  and Set Prompts
	 */
	if (LCHECK ("basis"))
	{
		SetPrompt ();
		return (EXIT_SUCCESS);
	}

	/*
	 * Expand Rebate Type 
	 */
	if (LCHECK ("reb_type"))
	{
		strcpy (local_rec.rebTypeDesc,
			 (inrb_rec.reb_type [0] == 'V') ? "V)alue      " : "P)ercentage");
		if (inrb_rec.reb_type [0] == 'V')
		{
			FLD ("rv1") = YES;
			FLD ("rv2") = YES;
			FLD ("rv3") = YES;
			FLD ("rv4") = YES;
			FLD ("rv5") = YES;
			FLD ("rv6") = YES;
			FLD ("rp1") = ND;
			FLD ("rp2") = ND;
			FLD ("rp3") = ND;
			FLD ("rp4") = ND;
			FLD ("rp5") = ND;
			FLD ("rp6") = ND;
		}
		else
		{
			FLD ("rv1") = ND;
			FLD ("rv2") = ND;
			FLD ("rv3") = ND;
			FLD ("rv4") = ND;
			FLD ("rv5") = ND;
			FLD ("rv6") = ND;
			FLD ("rp1") = YES;
			FLD ("rp2") = YES;
			FLD ("rp3") = YES;
			FLD ("rp4") = YES;
			FLD ("rp5") = YES;
			FLD ("rp6") = YES;
		}
		DSP_FLD ("rebtypedesc");
		for (i = 0; i < 6; i++)
		{
			print_at (13 + i, 33, "                                          ");
			reb_val [i] = 0.00;
		}
		heading (1);
		scn_display (1);
		return (EXIT_SUCCESS);
	}

	/*
	 * Check Rebate Qty More Than Previous and if 0 
	 */
	if (LNCHECK ("rq", 2))
	{
		i = atoi (FIELD.label + 2);
		i--;

		if (!i && reb_qty [i] == 0.00)
		{
			print_mess (ML (mlSkMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * if dflt used or zero entered then set all below to zero as wel
		 */
		if (dflt_used || reb_qty [i] == 0.00)
		{
			for (count = i; count < 6; count++)
			{
				reb_qty [count] = 0.00;
				reb_val [count] = 0.00;
			}

			entry_exit = TRUE;
			if (prog_status != ENTRY)
				scn_display (1);
			return (EXIT_SUCCESS);
		}

		/*
		 * compare value keyed with later keyed value - has to be smaller
		 */
		if (prog_status != ENTRY && i < 5 && i != 0)
		{
			for (count = i; count < 5; count++)
			{
				if (reb_qty [i] >= reb_qty [count + 1] && reb_qty [count + 1] > 0.00)
				{
					sprintf (err_str, ML (mlSkMess134), i, count + 1);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}

		/*
		 * compare value keyed with prev keyed value has to be bigger
		 */
		if (i && reb_qty [i] != 0.00 &&
			reb_qty [i] <= reb_qty [i -1])
		{
			sprintf (err_str, ML (mlSkMess135), i + 1, i);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (i && reb_qty [i-1] == 0.00 && reb_qty [i] != 0.00)
		{
			print_mess (ML (mlSkMess122));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Check Rebate Value In Range 0 - 100 If Percentage 
	 */
	if (LNCHECK ("rv", 2) || LNCHECK ("rp", 2))
	{
		if (FLD (FIELD.label) == ND)
			return (EXIT_SUCCESS);

		i = atoi (FIELD.label + 2);
		i--;

		if (!strncmp (inrb_rec.reb_type, "P", 1) &&
			 (reb_val [i] <= 0.00 ||
			reb_val [i] >= 100.00))
		{
			sprintf (err_str,ML (mlSkMess136),reb_val [i]);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SetPrompt (
 void)
{
	int i;


	switch	 (toupper (inrb_rec.basis [0]))
	{
	case	'V':
		sprintf (local_rec.basisDesc, "V)alue ");
		for (i = 0; i < 6; i++)
		{
			sprintf (qtyPrompt [i],  " Value    %1.1d. ", i+1);
			sprintf (qtyComment [i], " Enter Rebate Value %1.1d. ", i+1);
		}
		FLD ("reb_type") = YES;
		FLD ("rebtypedesc") = NA;
		break;

	case	'U':
		sprintf (local_rec.basisDesc,"U)nits ");
		for (i = 0; i < 6; i++)
		{
			sprintf (qtyPrompt [i], " Quantity %1.1d. ", i+1);
			sprintf (qtyComment [i], " Enter Rebate Quantity %1.1d. ", i+1);
		}
		inrb_rec.reb_type [0] = 'V';
		FLD ("reb_type") = ND;
		FLD ("rebtypedesc") = ND;
		break;

	case	'W':
		sprintf (local_rec.basisDesc, "W)eight");
		for (i = 0; i < 6; i++)
		{
			sprintf (qtyPrompt [i], " Weight   %1.1d. ", i+1);
			sprintf (qtyComment [i], " Enter Rebate Weight %1.1d. ", i+1);
		}
		inrb_rec.reb_type [0] = 'V';
		FLD ("reb_type") = ND;
		FLD ("rebtypedesc") = ND;
		break;
	}

	display_prmpt (label ("rq1"));
	display_prmpt (label ("rq2"));
	display_prmpt (label ("rq3"));
	display_prmpt (label ("rq4"));
	display_prmpt (label ("rq5"));
	display_prmpt (label ("rq6"));

	if (inrb_rec.reb_type [0] == 'V')
		sprintf (local_rec.rebTypeDesc, "V)alue     ");
	else if (inrb_rec.reb_type [0] == 'P')
		sprintf (local_rec.rebTypeDesc, "P)ercentage");

	DSP_FLD ("reb_type");
	DSP_FLD ("rebtypedesc");
	DSP_FLD ("basisdesc");

	return;
}

void
SrchRebCode (
 char *keyValue)
{
	char	cy_str [3];

	work_open ();
	save_rec ("#Code  Cycle", "#Description");

	sprintf (inrb_rec.reb_code, "%-5.5s", keyValue);

	strncpy (inrb_rec.reb_flag, rebFlag, 1);
	inrb_rec.cycle = 0;
	tempCycle = 0;
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc)
	{
		if (inrb_rec.link_hash == scHash &&
			!strncmp (inrb_rec.reb_flag, rebFlag, 1))
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
		InitVars ();
		return;
	}

	strncpy (rebateCode, keyValue, 5);
	strncpy (inrb_rec.reb_code, keyValue, 5);
	strncpy (cy_str, keyValue + 7, 2);
	inrb_rec.cycle = atoi (cy_str);
	tempCycle = inrb_rec.cycle;

	inrb_rec.link_hash = scHash; 
	strncpy (inrb_rec.reb_flag, rebFlag, 1);

	cc = find_rec (inrb, &inrb_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inrb, "DBFIND");

	DSP_FLD ("cycle");
	DSP_FLD ("reb_desc");
}

void
SrchCycle (
	char	*keyValue)
{
	char	cycle_str [3];

	work_open ();
	save_rec ("#Cy", "#Description");

	inrb_rec.cycle = 0;
	strncpy (inrb_rec.reb_flag, rebFlag, 1);
	cc = find_rec (inrb, &inrb_rec, GTEQ, "r");

	while (!cc)
	{
		sprintf (cycle_str,"%2d",inrb_rec.cycle);

		if (!strcmp (rebateCode, inrb_rec.reb_code) &&
			inrb_rec.link_hash == scHash &&
			!strncmp (inrb_rec.reb_flag, rebFlag, 1))

			cc = save_rec (cycle_str, inrb_rec.description);
		else
			break;

		if (cc)
			break;

		cc = find_rec (inrb, &inrb_rec, NEXT, "r");
	}

	cc = disp_srch ();
	inrb_rec.cycle = atoi (keyValue);
	work_close ();
	strncpy (inrb_rec.reb_code, rebateCode, 5);
	if (cc)
		return;

	inrb_rec.link_hash = scHash; 
	strncpy (inrb_rec.reb_flag, rebFlag, 1);

	cc = find_rec (inrb, &inrb_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inrb, "DBFIND");
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

	if (rebFlag [0] == 'S')
		sprintf (err_str,ML (mlSkMess045));
	else
		sprintf (err_str,ML (mlSkMess046));

	rv_pr (err_str, (80 - (strlen (clip (err_str)))) / 2, 0, 1);

	box (0, 2, 80, 16);

	line_at (7, 1,79);
	line_at (11,1,79);
	line_at (21,0,80);

	print_at (22, 1,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	line_at (1,0,80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
InitVars (void)
{
	int	i;

	inrb_rec.basis [0] = '\0';
	inrb_rec.reb_type [0] = '\0';
	inrb_rec.start_date = 0;
	inrb_rec.end_date = 0;
	for (i = 0; i < 6; i++)
	{
		reb_qty [i] = 0;
		reb_val [i] = 0;
	}
	inrb_rec.ant_reb_pc = 0;
	inrb_rec.la_cycle_rep = 0;
}

void
Update (void)
{
	if (updateFlag)
	{
		cc = abc_update (inrb, &inrb_rec);
		if (cc)
			file_err (cc, inrb, "DBUPDATE");
	}
	else
	{
		cc = abc_add (inrb, &inrb_rec);
		if (cc)
			file_err (cc, inrb, "DBADD");
	}
	return;

}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}
