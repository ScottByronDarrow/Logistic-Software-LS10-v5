/*====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: co_curr.c,v 5.4 2002/07/25 11:17:29 scott Exp $
|  Program Name  : (co_curr.c)
|  Program Desc  : (Maintain Currency Table for System)
|---------------------------------------------------------------------|
|  Author        : Simon Spratt    | Date Written  : 09/02/96         |
|---------------------------------------------------------------------|
| $Log: co_curr.c,v $
| Revision 5.4  2002/07/25 11:17:29  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.3  2001/08/28 08:46:17  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 05:13:19  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:10  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: co_curr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/co_curr/co_curr.c,v 5.4 2002/07/25 11:17:29 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>
#include <GlUtils.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int  	new_item = 0;

	char	rep_type [2];

	FILE	*pout;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

POCR_STRUCT	pocr2Rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	operator_desc [11];
	char	gl_ctrl_acct [FORM_LEN + 1];
	char	gl_exch_var [FORM_LEN + 1];
	char 	glacct_desc [26];
	char 	exch_var_desc [26];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "co_no",	3, 15, CHARTYPE,
		"AA", "                          ",
		" ", " ", " Company No :", "Enter company number, [SEARCH]",
		 NE, NO,  JUSTRIGHT, "0123456789*-", "", comr_rec.co_no},
	{1, LIN, "co_name",	3, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","                          ",
		" ", " ", " Name       :", " ",
		 NA, NO,  JUSTLEFT, "", "", comr_rec.co_name},
	{1, LIN, "curr2_desc",	4, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "                          ",
		" ", " ", " Currency   :", "",
		 NA, NO,  JUSTLEFT, "", "", pocr2Rec.description},
	{1, LIN, "curr_code",	 6, 15, CHARTYPE,
		"UUU", "          ",
		" ", "", " Code       :", " ",
		YES, NO,  JUSTLEFT, "A", "Z", pocrRec.code},
	{1, LIN, "curr_desc",	 6, 65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Description:", " ",
		YES, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "curr_prime",	 7, 15, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Prime Unit :", "Enter prime unit of currency. (Singular)",
		YES, NO,  JUSTLEFT, "", "", pocrRec.prime_unit},
	{1, LIN, "sub_prime",	 7, 65, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", " Sub Unit   :", "Enter sub-unit that makes up the prime unit of currency. (Singular)",
		YES, NO,  JUSTLEFT, "", "", pocrRec.sub_unit},
	{1, LIN, "ex_factor",	 9, 15, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Curr Rate  :", "Enter the present exchange rate",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex1_factor},
	{1, LIN, "ex_factor",	 11, 15, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +1 Mth:", "Enter exchange rate + 1 month ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex2_factor},
	{1, LIN, "ex_factor",	 11, 65, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +2 Mth:", "Enter exchange rate + 2 months ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex2_factor},
	{1, LIN, "ex_factor",	 12, 15, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +3 Mth:", "Enter exchange rate + 3 months ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex3_factor},
	{1, LIN, "ex_factor",	12, 65, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +4 Mth:", "Enter exchange rate + 4 months ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex4_factor},
	{1, LIN, "ex_factor",	13, 15, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +5 Mth:", "Enter exchange rate + 5 months ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex5_factor},
	{1, LIN, "ex_factor",	13, 65, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", " Rate +6 Mth:", "Enter exchange rate + 6 months ",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ex6_factor},
	{1, LIN, "operator",	15, 15, CHARTYPE,
		"A", "          ",
		"*", "", " Operator   :", "Enter operator - * Multiply, / Divide ",
		YES, NO,  JUSTLEFT, "/*", "", pocrRec.pocr_operator},
	{1, LIN, "operator_desc",	15, 51, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"*", "", " ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.operator_desc},
	{1, LIN, "ex_date",	16, 15, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.systemDate, " Last Update:", "Enter date of update or RETURN for today.",
		YES, NO, JUSTRIGHT, "", "", (char*)&pocrRec.ldate_up},
	{1, LIN, "gl_acct",	18, 15, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ",
		" ", " "," Control A/C:", "Enter the control account. ",
		 YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.gl_ctrl_acct},
	{1, LIN, "glacct_desc",	18, 51, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA","                          ",
		"", "", " ", "",
		 NA, NO,  JUSTLEFT, "", "",local_rec.glacct_desc}, 
	{1, LIN, "exch_var",	19, 15, CHARTYPE,
		"NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN", "                          ",
		"0", "", " Var A/C    :", "Enter the exchange variance account. ",
		 YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.gl_exch_var},
	{1, LIN, "exch_var_desc",	19, 51, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA","                          ",
		"", "", " ", "",
		 NA, NO,  JUSTLEFT, "", "",local_rec.exch_var_desc}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Function prototypes.     |
==========================*/
int		main			 (int argc, char * argv []);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 (void);
int		spec_valid		 (int field);
void	update			 (void);
void	SrchPocr		 (char * key_val);
int		heading			 (int scn);
void	SrchComr		 (char * key_val);
int		read_comr		 (void);
int		CheckGlmr		 (char *gl_acc);
void	display_operator (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	/*------------------
	| Printer Number	|
	 ------------------*/
	sprintf (rep_type,"%-1.1s", argv [1]);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	swide ();
	vars [label ("gl_acct")].mask = GL_SetAccWidth (comm_rec.co_no, TRUE);
	vars [label ("exch_var")].mask = vars [label ("gl_acct")].mask;
	set_masks ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

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
		init_ok = TRUE;
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*------------------------------
		| Edit screen 1 linear input . |	
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		update ();

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

	open_rec (pocr,pocr_list,POCR_NO_FIELDS,"pocr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (pocr);
	abc_fclose (glmr);
	abc_dbclose ("data");
}

int
spec_valid (
 int	field)
{
	/*-------------------
	| Check comr company |
	 -------------------*/
	if (LCHECK ("co_no"))
	{
		if (dflt_used)
		{
			strcpy (comr_rec.co_no,comm_rec.co_no);
			return (read_comr ());
			
		}
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		return (read_comr ());
	}
	/*---------------
	| Validate code |
	---------------*/
    if (LCHECK ("curr_code"))
    {
		if (SRCH_KEY)
		{
               SrchPocr (temp_str);
		       return (EXIT_SUCCESS);
		}
		strcpy (pocrRec.co_no,comr_rec.co_no);
		cc = find_rec (pocr,&pocrRec,COMPARISON,"u");
		if (cc)
			new_item = TRUE;
		else
		{
			new_item = FALSE;
			entry_exit = TRUE;
			/*---------------------------------
			| Show GL exchange control account |
			 ---------------------------------*/
		  	strcpy (local_rec.gl_ctrl_acct,pocrRec.gl_ctrl_acct);
			CheckGlmr (pocrRec.gl_ctrl_acct);
			strcpy (local_rec.glacct_desc, glmrRec.desc);
			GL_FormAccNo (local_rec.gl_ctrl_acct, pocrRec.gl_ctrl_acct, 0);
			/*-------------------------------------
			| Show GL exchange fluctuation account |
			 -------------------------------------*/
		  	strcpy (local_rec.gl_exch_var, pocrRec.gl_exch_var);
			CheckGlmr (pocrRec.gl_exch_var);
			strcpy (local_rec.exch_var_desc, glmrRec.desc);
			GL_FormAccNo (local_rec.gl_exch_var, pocrRec.gl_exch_var, 0);
			/*------------------------------------------
			| Display operator for exchange calculation |
			 ------------------------------------------*/
			display_operator ();
		}
        return (EXIT_SUCCESS);
	}
	/*---------------
	| Check operator |
	 ---------------*/
	if (LCHECK ("operator"))
	{
		if (dflt_used)
		{
			strcpy (pocrRec.pocr_operator,"*");
			DSP_FLD ("operator");
			strcpy (local_rec.operator_desc,"(Multiply)");
			DSP_FLD ("operator_desc");
			return (EXIT_SUCCESS);
		}

		if (pocrRec.pocr_operator [0] == '*')
			strcpy (local_rec.operator_desc,"(Multiply)");
		else
			if (pocrRec.pocr_operator [0] == '/')
				strcpy (local_rec.operator_desc,"(Divide)");
			else
			{
				/*print_mess ("Operator must be * (Multiply) or / (Divide)");*/
				print_mess (ML (mlMenuMess218));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

		if (pocrRec.pocr_operator [0] == '*')
		{
			strcpy (pocrRec.pocr_operator, "/");
			strcpy (local_rec.operator_desc,"(Divide)");
			/*print_mess ("Sorry Operator * (Multiply) not yet supported.");*/
			print_mess (ML (mlMenuMess219));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("operator");
		DSP_FLD ("operator_desc");
		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Control Account. |
	---------------------------*/
	if (LCHECK ("gl_acct"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY) 
			return SearchGlmr_F (comr_rec.co_no, temp_str, "F*P");

		GL_StripForm (pocrRec.gl_ctrl_acct, local_rec.gl_ctrl_acct);

		cc = CheckGlmr (pocrRec.gl_ctrl_acct);
		if (cc)
		{
			/*print_mess ("Invalid General Ledger code.");*/
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.glacct_desc,glmrRec.desc);
		DSP_FLD ("glacct_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------------
	| Validate Exchange Variation Account |
	--------------------------------------*/
	if (LCHECK ("exch_var"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY) 
			return SearchGlmr_F (comr_rec.co_no, temp_str, "F*P");

		GL_StripForm (pocrRec.gl_exch_var, local_rec.gl_exch_var);

		cc = CheckGlmr (pocrRec.gl_exch_var);
		if (cc)
		{
/*
			print_mess ("Invalid General Ledger code.");*/
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.exch_var_desc,glmrRec.desc);
		DSP_FLD ("exch_var_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
update (void)
{
	strcpy (pocrRec.stat_flag, "0");
	strcpy (pocrRec.co_no,comr_rec.co_no);
	if (new_item)
	{
		cc = abc_add (pocr,&pocrRec);
		if (cc)
			sys_err ("Error in pocr During (DBADD)",cc,PNAME);
	}
	else
	{
		pocrRec.ldate_up = StringToDate (local_rec.systemDate);
		cc = abc_update (pocr,&pocrRec);
		if (cc)
			sys_err ("Error in pocr During (DBUPDATE)",cc,PNAME);
	}
	abc_unlock (pocr);
}

/*=========================
| Search for currency code |
 =========================*/
void
SrchPocr (
 char *	key_val)
{
	_work_open (3,0,40);
	save_rec ("#No.","#Currency code description");                       
	strcpy (pocrRec.co_no,comr_rec.co_no);
	sprintf (pocrRec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no,comr_rec.co_no) && 
		!strncmp (pocrRec.code,key_val,strlen (key_val)))
	{                        
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;
		cc = find_rec (pocr,&pocrRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;
	strcpy (pocrRec.co_no,comr_rec.co_no);
	sprintf (pocrRec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
	if (cc)
 	        sys_err ("Error in pocr During (DBFIND)", cc, PNAME);
}


/*====================================================
| Display Screen Heading.                            |
====================================================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlMenuMess121),46,0,2);
		box (0, 2, 132, 17);
		move (1,5);
		line (131);
		move (1,8);
		line (131);
		move (1,10);
		line (131);
		move (1,14);
		line (131);
		move (1,17);
		line (131);

		print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		move (1,input_row);
		line_cnt = 0;
		scn_write (scn); 
	}
	return (EXIT_SUCCESS);
}

/*--------------------
| Search company file |
  -------------------*/
void
SrchComr (
 char *	key_val)
{
	char	str_s [60];

	_work_open (2,0,46);
	save_rec ("#No", "#Company Name                           Curr");

	sprintf (comr_rec.co_no, "%-2.2s", key_val);
	cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc && !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
	{
		sprintf (str_s,"%-40.40s %-3.3s",comr_rec.co_name, comr_rec.base_curr);
		cc = save_rec (comr_rec.co_no, str_s);
		if (cc)
			break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_no, "%-2.2s", temp_str);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	return;
}
/*--------------------------------------
| read comr record to validate company |
 -------------------------------------*/
int
read_comr (void)
{
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
	{
		/*print_mess ("Company is not on file.");*/
		print_mess (ML (mlStdMess130));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	strcpy (pocr2Rec.co_no,comr_rec.co_no);
	strcpy (pocr2Rec.code,comr_rec.base_curr);
	cc = find_rec (pocr,&pocr2Rec,COMPARISON,"u");
	if (cc)
		sprintf (pocr2Rec.description,"New Currency");

	DSP_FLD ("co_no");
	DSP_FLD ("co_name");
	DSP_FLD ("curr2_desc");
	return (EXIT_SUCCESS);
}

/*----------------------------------------
| Read and check glmr for valide accounts |
 ----------------------------------------*/
int
CheckGlmr (
 char *	gl_acc)
{
		strcpy (glmrRec.co_no,comr_rec.co_no);
		strcpy (glmrRec.acc_no,gl_acc);
		if ((cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
			return print_err (ML (mlStdMess025));

		if (glmrRec.glmr_class [2][0] != 'P')
	    {
			print_err (ML (mlStdMess025));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
}

/*---------------------------------------------------------
| Display the operator description for currency conversion |
 ----------------------- ---------------------------------*/
void
display_operator (void)
{
		if (!strcmp (pocrRec.pocr_operator,"*"))
			strcpy (local_rec.operator_desc,"(Multiply)");
		else
			strcpy (local_rec.operator_desc,"(Divide)");

		DSP_FLD ("operator_desc");
}
