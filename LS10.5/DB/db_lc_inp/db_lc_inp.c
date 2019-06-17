/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lc_inp.c,v 5.4 2001/11/07 09:32:36 scott Exp $
|  Program Name  : (db_lc_inp.c)
|  Program Desc  : (Add/ Maintain Customer Letter of Credit file) 
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow. | Date Written  : 16/11/92         |
|---------------------------------------------------------------------|
| $Log: db_lc_inp.c,v $
| Revision 5.4  2001/11/07 09:32:36  scott
| Updated as index not being reset.
|
| Revision 5.3  2001/11/06 08:06:19  scott
| Updated for S/C 00037
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lc_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lc_inp/db_lc_inp.c,v 5.4 2001/11/07 09:32:36 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

#define 	LSL_UPDATE      0
#define 	LSL_IGNORE      1
#define 	LSL_DELETE		2
#define 	LSL_DEFAULT		99

#define		LETT_DEL	 (cuch_rec.cuch_delete [0] == 'Y')

#define	FAIL_CURR ((envVarDbMcurr && \
		          (strcmp (cuch_rec.curr_code,local_rec.s_curr_code) < 0 || \
                    strcmp (cuch_rec.curr_code,local_rec.e_curr_code) > 0)))

#define	FAIL_LC  ((strcmp (cuch_rec.letter_no,local_rec.s_lc_no) < 0 || \
                    strcmp (cuch_rec.letter_no,local_rec.e_lc_no) > 0))

#define	FAIL_EXP ((cuch_rec.expiry_date < local_rec.s_expiry_date || \
                    cuch_rec.expiry_date > local_rec.e_expiry_date))

	/*
	 * Special fields and flags.
	 */
   	int		newLC 			= 0,
			envDbFind 		= FALSE,
			envDbCo 		= FALSE,
			cucdFound 		= FALSE,
			LC_INPUT 		= FALSE,
			LC_DISPLAY 		= FALSE,
			envVarDbMcurr	=	0;

	extern	int		TruePosition;

	char	save_key 	[11],
			disp_line 	[300],
			branchNo 	[3];

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cuchRecord	cuch_rec;
struct cucdRecord	cucd_rec;
struct crbkRecord	crbk_rec;
struct pocrRecord	pocr_rec;
struct cumrRecord	cumr_rec;
struct cuhdRecord	cuhd_rec;

	char	*data = "data";

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	db_curr_desc [41];
	char	bk_curr_desc [41];
	long	ldate_up;
	char	lc_exp_date [11];
	char	s_lc_no [16],
			e_lc_no [16];

	char	s_dbt_no [7],
			s_dbt_name [41],
			e_dbt_no [7],
			e_dbt_name [41];

	char	s_curr_code [4],
			s_curr_desc [41],
			e_curr_code [4],
			e_curr_desc [41];

	long	s_expiry_date,
			e_expiry_date;

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customer",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer Number      ", "Enter Customer Number. [SEARCH] Available.",
		 NE, NO,  JUSTLEFT, "", "", cumr_rec.dbt_no},
	{1, LIN, "name",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Customer Name        ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "db_curr_code",	 5, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code.       ", "",
		NA, NO,  JUSTLEFT, "", "", cuch_rec.curr_code},
	{1, LIN, "db_curr_desc",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.db_curr_desc},
	{1, LIN, "lc_no",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "Letter of Credit No  ", "Enter Letter of credit (LC) number for customer. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", cuch_rec.letter_no},
	{1, LIN, "bank_id",	 9, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "",  "Bank Code            ", "Enter Bank Code. [SEARCH] available.",
		 YES, NO,  JUSTLEFT, "", "", cuch_rec.bank_id},
	{1, LIN, "bk_name",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Name            ", "Enter Bank name.",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "bk_curr_code",	 11, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code.       ", "",
		NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "bk_curr_desc",	 11, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bk_curr_desc},
	{1, LIN, "lc_limit",	 13, 2, MONEYTYPE,
		"NN,NNN,NNN,NNN.NN", "          ",
		" ", "0", "Value Limit.         ", "Enter Letter of credit (LC) value limit.",
		 NO, NO,  JUSTRIGHT, "0", "99999999999.99", (char *)&cuch_rec.limit},
	{1, LIN, "lc_expiry",	 14, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.lc_exp_date, "Expiry Date          ", "Enter expiry date of Letter of Credit (LC)",
		 YES, NO, JUSTRIGHT, "", "", (char *)&cuch_rec.expiry_date},

	{1, LIN, "s_lc_no",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Letter of Credit No  ", "Enter Start LC number for customer. [SEARCH] available.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.s_lc_no},
	{1, LIN, "e_lc_no",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Letter of Credit No.", "Enter End LC number for customer. [SEARCH] available.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.e_lc_no},
	{1, LIN, "s_customer",	 6, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Customer Number      ", "Enter Customer Number. [SEARCH] Available.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.s_dbt_no},
	{1, LIN, "s_dbt_name",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name        ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_dbt_name},
	{1, LIN, "s_curr_code",	 9, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Start Currency Code  ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_curr_code},
	{1, LIN, "s_curr_desc",	 9, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_curr_desc},
	{1, LIN, "e_curr_code",	 10, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "End Currency Code    ", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_curr_code},
	{1, LIN, "e_curr_desc",	 10, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_curr_desc},
	{1, LIN, "s_lc_expiry",	 12, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Expiry Date    ", "Enter expiry Start date of LC.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.s_expiry_date},
	{1, LIN, "e_lc_expiry",	 13, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Expiry Date      ", "Enter expiry End date of LC.",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.e_expiry_date},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindCumr.h>
/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	Update 				(void);
void 	UpdateMenu 			(void);
void 	SrchCrbk 			(char *);
void 	SrchCuch 			(char *);
void 	SrchPocr 			(char *);
void 	DisplayLC 			(void);
int 	DisplayDetail 		(char *);
double 	TotalCuhd 			(long);
int 	heading 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	int	i;
	long	lc_exp = 0L;
	char 	*sptr;

	TruePosition	=	TRUE;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "db_lc_inp"))
		LC_INPUT = TRUE;

	if (!strcmp (sptr, "db_lc_dsp"))
		LC_DISPLAY = TRUE;

	/*
	 * Check for Multi-currency.
	 */
	sptr = get_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);

	if (LC_INPUT)
	{
		for (i = label ("s_lc_no"); i <= label ("e_lc_expiry"); i++)
			vars [i].scn = 2;

		for (i = label ("customer"); i <= label ("lc_expiry"); i++)
			vars [i].scn = 1;

		/*-----------------------
		| Hide currency fields. |
		-----------------------*/
		if (!envVarDbMcurr)
		{
			FLD ("db_curr_code") 	= ND;
			FLD ("db_curr_desc") 	= ND;
			FLD ("bk_curr_code") 	= ND;
			FLD ("bk_curr_desc") 	= ND;
			SCN_ROW ("lc_no")	 	= 6;
			SCN_ROW ("bank_id")	 	= 8;
			SCN_ROW ("bk_name")	 	= 9;
			SCN_ROW ("lc_limit") 	= 11;
			SCN_ROW ("lc_expiry")	= 12;
		}
	}
	else
	{
		for (i = label ("s_lc_no"); i <= label ("e_lc_expiry"); i++)
			vars [i].scn = 1;

		for (i = label ("customer"); i <= label ("lc_expiry"); i++)
			vars [i].scn = 2;

		/*-----------------------
		| Hide currency fields. |
		-----------------------*/
		if (!envVarDbMcurr)
		{
			FLD ("s_curr_code") 	= ND;
			FLD ("e_curr_code") 	= ND;
			FLD ("s_curr_desc") 	= ND;
			FLD ("e_curr_desc") 	= ND;
			SCN_ROW ("s_lc_expiry") = 9;
			SCN_ROW ("e_lc_expiry") = 10;
		}
	}
	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lc_exp = TodaysDate () + 180L;

	strcpy (local_rec.lc_exp_date, DateToString (lc_exp));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

	if (!LC_INPUT)
		swide ();

	init_vars (1);

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");


	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newLC 		= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		abc_unlock (cuch);

		no_edit (2);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		scn_display (1);

		edit_all ();
		if (restart) 
			continue;

		/*-------------------------
		| Update cr bank record.  |
		-------------------------*/
		if (LC_INPUT && !restart)
			Update ();

		if (LC_DISPLAY)
			DisplayLC ();

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
	abc_dbopen (data);

	open_rec (cuch, cuch_list, CUCH_NO_FIELDS, (LC_INPUT) ? "cuch_id_no"
							       						  : "cuch_id_no3");
	open_rec (cucd, cucd_list, CUCD_NO_FIELDS, "cucd_id_no");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind) ? "cumr_id_no3" 
							       						   : "cumr_id_no");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cuch);
	abc_fclose (cucd);
	abc_fclose (crbk);
    abc_fclose (cumr);
	abc_fclose (pocr);
	abc_fclose (cuhd);
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("customer"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (cumr_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("name");

		if (envVarDbMcurr)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
				sprintf (pocr_rec.description, "%40.40s"," ");
	
			sprintf (cuch_rec.curr_code, "%-3.3s", cumr_rec.curr_code);
			sprintf (local_rec.db_curr_desc, "%-40.40s", pocr_rec.description);
	
			DSP_FLD ("db_curr_code");
			DSP_FLD ("db_curr_desc");
		}

		return (EXIT_SUCCESS);
	}
	/*--------------------------------------------
	| Validate Creditor Number And Allow Search. |
	--------------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		strcpy (crbk_rec.bank_id,cuch_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess043));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("bk_name");

		if (envVarDbMcurr)
		{
			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, crbk_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
			if (cc)
				sprintf (pocr_rec.description, "%40.40s"," ");
	
			strcpy (local_rec.bk_curr_desc, pocr_rec.description);
	
			DSP_FLD ("bk_curr_code");
			DSP_FLD ("bk_curr_desc");
		}

		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Letter of Credit number. |
	-----------------------------------*/
	if (LCHECK ("lc_no"))
	{
		if (SRCH_KEY)
		{
			SrchCuch (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cuch_rec.co_no,comm_rec.co_no);
		cuch_rec.hhcu_hash = cumr_rec.hhcu_hash;
		newLC = find_rec (cuch, &cuch_rec, COMPARISON, "w");
		if (!newLC)
		{
			if (LETT_DEL)
			{
				sprintf (err_str,ML (mlDbMess108), cuch_rec.letter_no);
							
				print_mess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			entry_exit = TRUE;
			strcpy (crbk_rec.co_no,comm_rec.co_no);
			strcpy (crbk_rec.bank_id,cuch_rec.bank_id);
			cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess043));
				sleep (sleepTime);
				return (EXIT_SUCCESS);
			}

			if (envVarDbMcurr)
			{
				strcpy (pocr_rec.co_no, comm_rec.co_no);
				strcpy (pocr_rec.code, crbk_rec.curr_code);
				cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
				if (cc)
					sprintf (pocr_rec.description,"%40.40s"," ");
		
				strcpy (local_rec.bk_curr_desc,pocr_rec.description);
	
				DSP_FLD ("bk_curr_code");
				DSP_FLD ("bk_curr_desc");
			}
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Letter of Credit number. |
	-----------------------------------*/
	if (LCHECK ("s_lc_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.s_lc_no, "               ");
			strcpy (local_rec.e_lc_no, "~~~~~~~~~~~~~~~");
			DSP_FLD ("s_lc_no");
			DSP_FLD ("e_lc_no");
			skip_entry = goto_field (field, label ("s_customer"));
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCuch (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cuch_rec.co_no,comm_rec.co_no);
		strcpy (cuch_rec.letter_no, local_rec.s_lc_no);
		cc = find_rec (cuch, &cuch_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess079));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY && 
			strcmp (local_rec.s_lc_no,local_rec.e_lc_no) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------
	| Validate Letter of Credit number. |
	-----------------------------------*/
	if (LCHECK ("e_lc_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.e_lc_no, "~~~~~~~~~~~~~~~");
			DSP_FLD ("e_lc_no");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCuch (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cuch_rec.co_no,comm_rec.co_no);
		strcpy (cuch_rec.letter_no, local_rec.e_lc_no);
		cc = find_rec (cuch, &cuch_rec, COMPARISON, "r");
		if (cc)
		{
				print_mess (ML (mlStdMess079));
				sleep (sleepTime);
				return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.s_lc_no,local_rec.e_lc_no) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.s_dbt_no, "      ");
		strcpy (local_rec.s_dbt_name, "ALL Customers. ");
		strcpy (local_rec.s_curr_code, "   ");
		strcpy (local_rec.s_curr_desc, "Start Currency");
		strcpy (local_rec.e_curr_code, "~~~");
		strcpy (local_rec.e_curr_desc, "End Currency");
		local_rec.s_expiry_date = MonthStart (TodaysDate ());
		local_rec.e_expiry_date = MonthEnd (TodaysDate ());
		entry_exit = TRUE;
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("s_customer"))
	{
		if (dflt_used)
		{
			cumr_rec.hhcu_hash = 0L;
			strcpy (local_rec.s_dbt_no, "      ");
			strcpy (local_rec.s_dbt_name, "ALL Customers. ");
			DSP_FLD ("s_customer");
			DSP_FLD ("s_dbt_name");
		
			skip_entry = goto_field (field, label ("s_curr_code"));
			return (EXIT_SUCCESS);
		}
			
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.s_dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.s_dbt_name, cumr_rec.dbt_name);

		DSP_FLD ("s_dbt_name");

		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate Start Currency Code. |
	-------------------------------*/
	if (LCHECK ("s_curr_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.s_curr_code, "   ");
			strcpy (local_rec.s_curr_desc, "Start Currency");
			strcpy (local_rec.e_curr_code, "~~~");
			strcpy (local_rec.e_curr_desc, "End Currency");
			DSP_FLD ("s_curr_code");
			DSP_FLD ("s_curr_desc");
			DSP_FLD ("e_curr_code");
			DSP_FLD ("e_curr_desc");
			skip_entry = goto_field (field, label ("s_lc_expiry"));
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  local_rec.s_curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			sprintf (pocr_rec.description, "%40.40s"," ");

		strcpy (local_rec.s_curr_desc, pocr_rec.description);

		DSP_FLD ("s_curr_desc");

		if (prog_status != ENTRY && 
		     strcmp (local_rec.s_curr_code,local_rec.e_curr_code) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Start Currency Code. |
	-------------------------------*/
	if (LCHECK ("e_curr_code"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.e_curr_code, "~~~");
			strcpy (local_rec.e_curr_desc, "End Currency");
			DSP_FLD ("e_curr_code");
			DSP_FLD ("e_curr_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code,  local_rec.e_curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			sprintf (pocr_rec.description, "%40.40s"," ");

		strcpy (local_rec.e_curr_desc, pocr_rec.description);

		DSP_FLD ("e_curr_desc");

		if (strcmp (local_rec.s_curr_code,local_rec.e_curr_code) > 0)
		{
			/*sprintf (err_str, "Currency %s is LESS than Currency %s.",
					local_rec.e_curr_code, local_rec.s_curr_code);*/
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("e_lc_expiry"))
	{
		if (dflt_used)
		{
			local_rec.e_expiry_date	=	YearEnd () + 365;
		}

		DSP_FLD ("e_lc_expiry");
    	return (EXIT_SUCCESS); 
	}
    return (EXIT_SUCCESS); 
}

/*======================================
| Update Letter of Credit master file. |
======================================*/
void
Update (void)
{
	strcpy (cuch_rec.co_no, comm_rec.co_no);
	if (newLC)
	{
		strcpy (cuch_rec.cuch_delete, "N");
		strcpy (cuch_rec.stat_flag, "0");
		cuch_rec.create_date = StringToDate (local_rec.systemDate);
		cc = abc_add (cuch,&cuch_rec);
		if (cc) 
			file_err (cc, cuch, "DBADD");

		abc_unlock (cuch);
	}
	else 
		UpdateMenu ();
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE LETTER OF CREDIT RECORD WITH CHANGES MADE.   ",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO LETTER OF CREDIT RECORD.",
		  "" },
		{ " 3. DELETE LETTER OF CREDIT RECORD.                     ",
		  "" },
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
UpdateMenu (void)
{
	for (;;)
	{
	    mmenu_print ("          U P D A T E    S E L E C T I O N .          ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case LSL_DEFAULT :
		case LSL_UPDATE :
			strcpy (cuch_rec.stat_flag, "0");
			cc = abc_update (cuch ,&cuch_rec);
			if (cc)
				file_err (cc, cuch, "DBUPDATE");
			return;

		case LSL_IGNORE :
			abc_unlock (cuch);
			return;

		case LSL_DELETE :
			strcpy (cuch_rec.cuch_delete, "Y");
			cc = abc_update (cuch ,&cuch_rec);
			if (cc)
				file_err (cc, cuch, "DBUPDATE");
			return;
			break;

		default :
			break;
	    }
	}
}

/*=========================================
| Search routine for Creditors Bank File. |
=========================================*/
void
SrchCrbk (
 char*              key_val)
{
	_work_open (4,0,40);
	save_rec ("#Bank", "#Bank Name");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
		      !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id,crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND");
}

/*======================================
| Search routine for Letter of Credit. |
======================================*/
void
SrchCuch (
 char*              key_val)
{
	_work_open (15,0,40);
	save_rec ("#Letter Credit", "# Status");
	strcpy (cuch_rec.co_no, comm_rec.co_no);
	strcpy (cuch_rec.letter_no, key_val);
	cuch_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cuch, &cuch_rec, GTEQ, "r");
	while (!cc && !strncmp (cuch_rec.letter_no,key_val,strlen (key_val)) &&
		      	!strcmp (cuch_rec.co_no, comm_rec.co_no))
	{
		if (LC_INPUT && cuch_rec.hhcu_hash != cumr_rec.hhcu_hash)
		{
			cc = find_rec (cuch, &cuch_rec, NEXT, "r");
			continue;
		}
		cc = save_rec (cuch_rec.letter_no,
					 (LETT_DEL) ? "Deleted" : "Current");
		if (cc)
			break;
		
		cc = find_rec (cuch, &cuch_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cuch_rec.co_no,comm_rec.co_no);
	strcpy (cuch_rec.letter_no,temp_str);
	cuch_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cuch, &cuch_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, cuch, "DBFIND");

	return;
}

/*===========================
| Search for Currency Code. |
===========================*/
void
SrchPocr (
	char	*key_val)
{
	_work_open (3,0,40);
	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code, key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && !strncmp (pocr_rec.code,key_val,strlen (key_val)) && 
		      !strcmp (pocr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,temp_str);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, pocr, "DBFIND");
}

/*=======================================
| Display Letter of credit information. |
=======================================*/
void
DisplayLC (void)
{
	int	first_time = TRUE;
	char	tmp_date1 [11],
			tmp_date2 [11];
	char	tmp_amt1 [18],
			tmp_amt2 [18];
	char	wk_curr [4],
			wk_bank [6];

	abc_selfield (cuch, "cuch_id_no2");

	clear ();
	Dsp_prn_open (0, 0, 18, "LETTER OF CREDIT DISPLAY", 
				comm_rec.co_no, comm_rec.co_name,
				 (char *)0, (char *)0,
				 (char *)0, (char *)0);

	if (envVarDbMcurr)
	{
		Dsp_saverec ("CURR| BANK|                BANK                    |LETTER OF CREDIT|   DATE   |  DATE    |      AMOUNT      |       AMOUNT     ");
		Dsp_saverec ("CODE| CODE|             DESCRIPTION                |    NUMBER.     |  CREATED | EXPIRED  |     ORIGINAL     |      ALLOCATED   ");
	}
	else
	{
		Dsp_saverec (" BANK|                BANK                    |LETTER OF CREDIT|   DATE   |  DATE    |      AMOUNT      |       AMOUNT     ");
		Dsp_saverec (" CODE|             DESCRIPTION                |    NUMBER.     |  CREATED | EXPIRED  |     ORIGINAL     |      ALLOCATED   ");
	}

	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCREEN] [PREV SCREEN] [EDIT/END]");

	strcpy (cuch_rec.co_no, comm_rec.co_no);
	strcpy (cuch_rec.curr_code, local_rec.s_curr_code);
	strcpy (cuch_rec.bank_id  , "     ");
	strcpy (cuch_rec.letter_no, "               ");
	cc = find_rec (cuch, &cuch_rec, GTEQ, "r");
	while (!cc && !strcmp (cuch_rec.co_no, comm_rec.co_no))
	{
		cucdFound = FALSE;

		if (cumr_rec.hhcu_hash > 0L && 
		     cumr_rec.hhcu_hash != cuch_rec.hhcu_hash)
		{
			cc = find_rec (cuch, &cuch_rec, NEXT, "r");
			continue;
		}

		if ((FAIL_CURR) || (FAIL_LC) || (FAIL_EXP))
		{
			cc = find_rec (cuch, &cuch_rec, NEXT, "r");
			continue;
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		strcpy (crbk_rec.bank_id,cuch_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, crbk, "DBFIND");

		sprintf (tmp_date1,"%-10.10s",DateToString (cuch_rec.create_date));
		sprintf (tmp_date2,"%-10.10s",DateToString (cuch_rec.expiry_date));
		
		strcpy (tmp_amt1,comma_fmt (DOLLARS (cuch_rec.limit), 
						"NN,NNN,NNN,NNN.NN"));

		strcpy (tmp_amt2,comma_fmt (TotalCuhd (cuch_rec.hhch_hash),
						"NN,NNN,NNN,NNN.NN"));

		if (first_time)
		{
			strcpy (wk_curr, cuch_rec.curr_code);
			strcpy (wk_bank, cuch_rec.bank_id);
			first_time = FALSE;
		}
		if (envVarDbMcurr && strcmp (wk_curr, cuch_rec.curr_code))
		{
			Dsp_saverec ("^^GGGGEGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGG");
			strcpy (wk_curr, cuch_rec.curr_code);
		}
		if (strcmp (wk_bank, cuch_rec.bank_id))
		{
			if (envVarDbMcurr)
				Dsp_saverec ("^^GGGGEGGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGG");
			else
				Dsp_saverec ("^^GGGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGGG");
			strcpy (wk_bank, cuch_rec.bank_id);
		}
			sprintf (disp_line, 
				"%s%s%s^E%s^E%s ^E%s^E%s^E%s ^E%s ",
				 (envVarDbMcurr) ? cuch_rec.curr_code : "",
				 (envVarDbMcurr) ? " ^E" :"",
				cuch_rec.bank_id,
				crbk_rec.bank_name,
				cuch_rec.letter_no,
				tmp_date1, tmp_date2,
				tmp_amt1,  tmp_amt2);

		sprintf (save_key, "%010ld", cuch_rec.hhch_hash);

		if (cucdFound)
			Dsp_save_fn (disp_line, save_key);
		else
			Dsp_saverec (disp_line);

		cc = find_rec (cuch, &cuch_rec, NEXT, "r");
	}
	if (envVarDbMcurr)
		Dsp_saverec ("^^GGGGJGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^^1E N D   O F   R E P O R T^6^^GGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGG");
	else
		Dsp_saverec ("^^GGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^^1E N D   O F   R E P O R T^6^^GGGJGGGGGGGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGG");

	Dsp_srch_fn (DisplayDetail);

	Dsp_close ();
	abc_selfield (cuch, "cuch_id_no3");
}

/*=================================================
| Display letter of credit allocated to receipts. |
=================================================*/
int
DisplayDetail (
	char	*find_key)
{
	long	hhchHash;

	hhchHash = atol (find_key);

	Dsp_prn_open (6, 3, 10, "LETTER OF CREDIT DETAIL DISPLAY", 
				comm_rec.co_no, comm_rec.co_name,
				 (char *)0, (char *)0,
				 (char *)0, (char *)0);

	Dsp_saverec ("Receipt No| Receipt Date |     Amount.      ");
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END]");

	cucd_rec.hhch_hash = hhchHash;
	cucd_rec.rec_date = 0L;
	cc = find_rec (cucd, &cucd_rec, GTEQ, "r");
	while (!cc && cucd_rec.hhch_hash == hhchHash)
	{
		cc = find_hash (cuhd, &cuhd_rec, EQUAL, "r",
						cucd_rec.hhcp_hash);
		if (cc)
		{
			cc = find_rec (cucd, &cucd_rec, NEXT, "r");
			continue;
		}
		sprintf 
		(
			disp_line, 
			" %8.8s |  %10.10s  |%17.17s ",
			cuhd_rec.receipt_no,
			DateToString (cuhd_rec.date_payment),
			comma_fmt (DOLLARS (cuhd_rec.tot_amt_paid), "NN,NNN,NNN,NNN.NN")
		);
		Dsp_saverec (disp_line);
		cc = find_rec (cucd, &cucd_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
    return (EXIT_SUCCESS);
}

/*
 * Get total allocation.
 */
double
TotalCuhd (
	long	hhchHash)
{
	double	totalReceipt = 0.00;

	cucd_rec.hhch_hash 	= hhchHash;
	cucd_rec.rec_date 	= 0L;
	cc = find_rec (cucd, &cucd_rec, GTEQ, "r");
	while (!cc && cucd_rec.hhch_hash == hhchHash)
	{
		cuhd_rec.hhcp_hash	=	cucd_rec.hhcp_hash;
		cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cucd, &cucd_rec, NEXT, "r");
			continue;
		}
		totalReceipt += DOLLARS (cuhd_rec.tot_amt_paid);

		cucdFound = TRUE;

		cc = find_rec (cucd, &cucd_rec, NEXT, "r");
	}
	return (totalReceipt);
}
	
/*
 * Display Screen Headings                           
 */
int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		if (LC_INPUT)
			rv_pr (ML (mlDbMess109),20,0,1);
		else
			rv_pr (ML (mlDbMess110),48,0,1);

		line_at (1,0, (LC_INPUT) ? 80 : 132);

		if (LC_INPUT)
		{
			if (envVarDbMcurr)
			{
				box (0, 2, 80, 12);
				line_at (6, 1,79);
				line_at (8, 1,79);
				line_at (12,1,79);
			}
			else
			{
				box (0, 2, 80, 10);
				line_at (5, 1, 79);
				line_at (7, 1, 79);
				line_at (10,1, 79);
			}
		}
		else
		{
			box (0, 2, 132, (envVarDbMcurr) ? 11 : 8);
			line_at (5,1,131);
			line_at (8,1,131);
			if (envVarDbMcurr)
				line_at (11,1,131);
		}
		
		line_at (20,0, (LC_INPUT) ? 80 : 132);
		print_at (21, 0, ML (mlStdMess038),
					comm_rec.co_no,comm_rec.co_name);
		line_at (22,0, (LC_INPUT) ? 80 : 132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

