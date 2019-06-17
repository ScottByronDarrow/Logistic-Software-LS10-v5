/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_altbdg.c,v 5.5 2001/10/09 22:47:59 scott Exp $
|  Program Name  : (gl_altbdg.c) 
|  Program Desc  : (General Ledger Budget Maintenance) 
|                  (Alternate version to input $ values) 
|---------------------------------------------------------------------|
| $Log: gl_altbdg.c,v $
| Revision 5.5  2001/10/09 22:47:59  scott
| Updated from scotts machine
|
| Revision 5.4  2001/09/20 03:17:54  cha
| Updated to make sure that the value of accountNo
| is properly updated. This is for LS10-GUI.
|
| Revision 5.3  2001/08/09 09:13:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:01  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:26  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_altbdg.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_altbdg/gl_altbdg.c,v 5.5 2001/10/09 22:47:59 scott Exp $";

/*
 *   Include file dependencies  
 */
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_gl_mess.h>
#include <GlUtils.h>
#include <twodec.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

/*
 * Constants, defines and stuff  
 */
	char	*data	= "data";

	char    baseCurr [4];
	int	    exist_flg = FALSE;
	int     PV_budg_no;
	const char *mth_nam [] =
	{
		"January.       ",
		"February.      ",
		"March.         ",
		"April.         ",
		"May.           ",
		"June.          ",
		"July.          ",
		"August.        ",
		"September.     ",
		"October.       ",
		"November.      ",
		"December.      "
	};

    extern int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct
{
	int		loc_year;
	int		loc_bdg_no;
	double	totFgnSpread;
	double	totLocSpread;
	double	fgnBudget		 [12];
	double	loc_bdg			 [12];
	char	previousAccNo	 [FORM_LEN + 1];
	char	localAccountNo	 [FORM_LEN + 1];
	char	locCurr			 [4];
	char	loc_df_budg		 [3];
	char	loc_df_year		 [5];
	char	loc_mth_fx_nam	 [12] [31];
	char	spread_title	 [30];
	char	fx_spread_title	 [30];
	char	dummy			 [10];
} local_rec;


/*
 * NB: BE SURE TO CHECK THE CODE IN 'spec_valid' WHEN CHANGING 
 *     THE ORDER OF THE FIELDS IN THE 'vars' STRUCTURE. (TvB).
 */
static	struct	var	vars []	=	
{
	{1, LIN, "accountNo", 3, 2, CHARTYPE, 
		/*"NNNNNNNNNNNNNNNNNNNNNNNNNNN"*/"NNNNNNNNNNNNNNNN", "          ", 
		"", "", "Account number ", " ", 
		NE, NO, JUSTLEFT, "0123456789-", "", local_rec.localAccountNo}, 
	{1, LIN, "currCode", 3, 45, CHARTYPE, 
		"AAA", "          ", 
		"", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.locCurr}, 
	{1, LIN, "budg_no", 4, 2, INTTYPE, 
		"NN", "          ", 
		"", local_rec.loc_df_budg, "Budget Number  ", " ", 
		NE, NO, JUSTLEFT, "", "", (char *) &local_rec.loc_bdg_no}, 
	{1, LIN, "year", 4, 45, INTTYPE, 
		"NNNN", "          ", 
		"", local_rec.loc_df_year, "Year           ", " ", 
		NE, NO, JUSTLEFT, "", "", (char *) &local_rec.loc_year}, 
	{1, LIN, "totFgnSpread", 5, 2, MONEYTYPE, 
		"N,NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.fx_spread_title, " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "9999999999.99", (char *) &local_rec.totFgnSpread}, 
	{1, LIN, "totLocSpread", 5, 45, MONEYTYPE, 
		"N,NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.spread_title, " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "9999999999.99", (char *) &local_rec.totLocSpread}, 
	{1, LIN, "fx_bdg01", 7, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [0], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [0]}, 
	{1, LIN, "bdg01", 7, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [0]}, 
	{1, LIN, "fx_bdg02", 8, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [1], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [1]}, 
	{1, LIN, "bdg02", 8, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [1]}, 
	{1, LIN, "fx_bdg03", 9, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [2], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [2]}, 
	{1, LIN, "bdg03", 9, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [2]}, 
	{1, LIN, "fx_bdg04", 10, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [3], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [3]}, 
	{1, LIN, "bdg04", 10, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [3]}, 
	{1, LIN, "fx_bdg05", 11, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [4], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [4]}, 
	{1, LIN, "bdg05", 11, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [4]}, 
	{1, LIN, "fx_bdg06", 12, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [5], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [5]}, 
	{1, LIN, "bdg06", 12, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [5]}, 
	{1, LIN, "fx_bdg07", 13, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [6], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [6]}, 
	{1, LIN, "bdg07", 13, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [6]}, 
	{1, LIN, "fx_bdg08", 14, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [7], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [7]}, 
	{1, LIN, "bdg08", 14, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [7]}, 
	{1, LIN, "fx_bdg09", 15, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [8], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [8]}, 
	{1, LIN, "bdg09", 15, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [8]}, 
	{1, LIN, "fx_bdg10", 16, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [9], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [9]}, 
	{1, LIN, "bdg10", 16, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [9]}, 
	{1, LIN, "fx_bdg11", 17, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [10], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [10]}, 
	{1, LIN, "bdg11", 17, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [10]}, 
	{1, LIN, "fx_bdg12", 18, 2, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", local_rec.loc_mth_fx_nam [11], " ", 
		YES, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.fgnBudget [11]}, 
	{1, LIN, "bdg12", 18, 45, MONEYTYPE, 
		"NNN,NNN,NNN.NN", "          ", 
		" ", "0", "", " ", 
		NA, NO, JUSTRIGHT, "-999999999.99", "999999999.99", (char *) &local_rec.loc_bdg [11]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 
};
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	Update 			(void);
void 	PostBudget 		(char *, int, int, char *, double, double);

/*
 * Main Processing Routine.
 */
int
main (
 int  argc, 
 char *argv [])
{
	TruePosition	=	TRUE;
	SETUP_SCR (vars);

	/*
	 * Setup required parameters.
	 */
	init_scr ();
	set_tty ();

	set_masks ();
	init_vars (1);

	OpenDB ();

	vars [label ("accountNo")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);

	strcpy (local_rec.previousAccNo,"0000000000000000");
	sprintf (local_rec.spread_title, "Value (%3.3s)", baseCurr);

	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		strcpy (local_rec.fx_spread_title, "Spread Amount");

		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Edit screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		/*
		 * Update g/l acc record.
		 */
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	int		loop;
	char	*budg_no = getenv ("BUDGET");

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (budg_no == (char *) NULL)
		PV_budg_no = 1;
	else
		PV_budg_no = atoi (budg_no);

	sprintf (local_rec.loc_df_budg, "%-2.2d", PV_budg_no);
	sprintf (local_rec.loc_df_year,"%-4.4d", (fisc_year (comm_rec.gl_date)+1));
	GL_PostSetup (comm_rec.co_no, PV_budg_no);

	OpenGlbd ();
	OpenGlmr ();
	OpenGlpd ();
	for (loop = 0; loop < 12; loop++)
    {
		sprintf (local_rec.loc_mth_fx_nam [loop], 
                 "%-10.10s",
                 mth_nam [ (loop + comm_rec.fiscal) % 12]);
    }

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

}

/*
 * Close data base files.
 */
void
CloseDB (
 void)
{
	GL_PostTerminate ();
	GL_Close ();
	abc_dbclose (data);
}

/*
 * Standard Screen Heading Routine
 */
int
heading (
	int	scn)
{
	if (restart)
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
        
	clear ();

	rv_pr (ML (mlGlMess028), 25,0,1);

	line_at (1,0,80);

	if (scn == 1)
	{
		box (0, 2, 79, 16);
		line_at (6,1,78);
	}

	line_at (20,0,80);

	print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
int
spec_valid (
	int		field)
{
	int	i;
	double	tmpFgnVal	=	0.00,
			tmpLocVal	=	0.00;
	

	/*
	 * Validate Account Number.
	 */
	if (LCHECK ("accountNo"))
	{
		if (vars [label ("accountNo")].mask [0] == '*')
		{
			print_mess (ML (mlGlMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (SRCH_KEY)
		{
			if (!strlen (temp_str))
				strcpy (temp_str, GL_GetfUserCode ());

			SearchGlmr_F (comm_rec.co_no, temp_str, "***");
			strcpy (local_rec.localAccountNo, temp_str);
		}

		GL_FormAccNo (local_rec.localAccountNo, glmrRec.acc_no, 0);
		

		strcpy (glmrRec.co_no, comm_rec.co_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess024));
			return (EXIT_FAILURE);
		}

		if (!strncmp (glmrRec.acc_no, "0000000000000000",MAXLEVEL))
		{
			sprintf (err_str, ML (mlGlMess055),
							MAXLEVEL, MAXLEVEL, "0000000000000000");
			errmess (err_str);
			return (EXIT_FAILURE);
		}

		/*
		 * Store currency information and refresh screen for currency display
		 */
		strcpy (local_rec.locCurr, glmrRec.curr_code);
		cc = FindPocr (comm_rec.co_no, local_rec.locCurr, "r");
		if (cc)
		{
			errmess (ML (mlStdMess040));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.fx_spread_title, "Spread Amount (%3.3s)", local_rec.locCurr);
		DSP_FLD ("currCode");
		scn_write (1);
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Budget Number.
	 */
	if (LCHECK ("budg_no"))
	{
		if (local_rec.loc_bdg_no < 1)
		{
			print_mess (ML (mlGlMess002));
			return (EXIT_FAILURE);
		}
		GL_PostBudget (local_rec.loc_bdg_no);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("year"))
	{
		exist_flg = FALSE;
		local_rec.totLocSpread = 0.00;
		for (i = 0; i < 12; i++)
		{
			glpdRec.hhmr_hash	= glmrRec.hhmr_hash;
			glpdRec.year		= local_rec.loc_year;
			glpdRec.budg_no		= local_rec.loc_bdg_no;
			glpdRec.prd_no		= i + 1;
			cc = find_rec (glpd, &glpdRec, COMPARISON, "r");
			if (cc)
			{
				local_rec.loc_bdg [i] 	= 0.00;
				local_rec.fgnBudget [i] = 0.00;
			}
			else
			{
				local_rec.totLocSpread += glpdRec.balance;
				local_rec.totFgnSpread += glpdRec.fx_balance;
				local_rec.loc_bdg [i]  	= glpdRec.balance;
				local_rec.fgnBudget [i] = glpdRec.fx_balance;
				if (glpdRec.balance != 0.00)
                {
					exist_flg = TRUE;
                }
			}
			display_field (label ("totLocSpread") + (i * 2) + 1);
			display_field (label ("totLocSpread") + (i * 2) + 2);
		}
		DSP_FLD ("totLocSpread");
		DSP_FLD ("totFgnSpread");

		for (i = 0; i < 13; i++)
        {
			vars [label ("totFgnSpread") + (i * 2)].required = (exist_flg) ? NI : YES;
        }

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Spread amount for current.
	 */
	if (LCHECK ("totFgnSpread"))
	{
		if ((local_rec.totFgnSpread == 0.00) || 
            (exist_flg && prog_status == ENTRY))
        {
			return (EXIT_SUCCESS);
        }

		tmpFgnVal	= 0.00;
		tmpLocVal 	= 0.00;
		local_rec.totLocSpread = CurrencyLocAmt (local_rec.totFgnSpread);
		for (i = 0; i < 12; i++)
		{
			local_rec.fgnBudget [i] 	= no_dec (local_rec.totFgnSpread / 12);
			local_rec.loc_bdg [i] 		= no_dec (local_rec.totLocSpread / 12);
			tmpFgnVal 					+= local_rec.fgnBudget [i];
			tmpLocVal 					+= local_rec.loc_bdg [i];
			display_field (field + ((i * 2) + 2));
			display_field (field + ((i * 2) + 3));
			if (prog_status == ENTRY)
            {
				vars [label ("totFgnSpread") + (i * 2) + 2].required = NI;
            }
		}

		local_rec.totFgnSpread = tmpFgnVal;
		local_rec.totLocSpread = tmpLocVal;
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg01"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [0] = CurrencyLocAmt (local_rec.fgnBudget [0]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg01");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg02"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [1] = CurrencyLocAmt (local_rec.fgnBudget [1]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg02");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg03"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [2] = CurrencyLocAmt (local_rec.fgnBudget [2]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg03");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg04"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [3] = CurrencyLocAmt (local_rec.fgnBudget [3]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg04");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg05"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [4] = CurrencyLocAmt (local_rec.fgnBudget [4]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg05");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg06"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [5] = CurrencyLocAmt (local_rec.fgnBudget [5]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg06");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg07"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [6] = CurrencyLocAmt (local_rec.fgnBudget [6]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg07");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg08"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [7] = CurrencyLocAmt (local_rec.fgnBudget [7]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg08");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg09"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [8] = CurrencyLocAmt (local_rec.fgnBudget [8]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg09");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg10"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [9] = CurrencyLocAmt (local_rec.fgnBudget [9]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg10");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg11"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [10] = CurrencyLocAmt (local_rec.fgnBudget [10]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg11");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("fx_bdg12"))
	{
		local_rec.totFgnSpread = 0.00;
		local_rec.totLocSpread = 0.00;
		local_rec.loc_bdg [11] = CurrencyLocAmt (local_rec.fgnBudget [11]);
		for (i = 0; i < 12; i++)
		{
			local_rec.totFgnSpread += local_rec.fgnBudget [i];
			local_rec.totLocSpread += local_rec.loc_bdg [i];
		}
		DSP_FLD ("totFgnSpread");
		DSP_FLD ("totLocSpread");
		DSP_FLD ("bdg12");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Update (void)
{
	int	i;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	GL_PostStamp ();
	for (i = 0; i < 12; i++)
	{
		PostBudget 
		(
			glmrRec.acc_no, 
		   	local_rec.loc_year, 
		   	i + 1, 
		   	glmrRec.curr_code,
		   	local_rec.fgnBudget [i], 
		   	local_rec.loc_bdg [i]
		);
	}

	strcpy (local_rec.previousAccNo, glmrRec.acc_no);
	sprintf (local_rec.loc_df_budg, "%-2.2d", local_rec.loc_bdg_no);
	sprintf (local_rec.loc_df_year, "%-4.4d", local_rec.loc_year);
	return;
}

void
PostBudget (
	char   	*accountNo,
	int    	year,
	int    	period,
	char   	*currCode,
	double 	fgnValue,
	double 	locValue)
{
	glpdRec.hhmr_hash	= glmrRec.hhmr_hash;
	glpdRec.budg_no		= local_rec.loc_bdg_no;
	glpdRec.prd_no		= period;
	glpdRec.year		= year;
	cc = find_rec (glpd, &glpdRec, EQUAL, "r");
	if (!cc)
	{
		fgnValue	-= glpdRec.fx_balance;
		locValue	-= glpdRec.balance;
	}
	_PostAccount (accountNo, year, period, currCode, fgnValue, locValue);
}	

/* [ end of file ] */
