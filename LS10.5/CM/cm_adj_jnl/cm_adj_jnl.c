/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (cm_adj_jnl.c   )                                |
|  Program Desc  : (Adjustment Journals For Contracts           )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, glmr, cmtr, cmhr, cmcb, cmcm, cmcd,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  cmtr, cmcb, glwk,     ,     ,     ,     ,         |
|  Database      : ( )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (06/04/93)      | Author       : Simon Dubey.      |
|---------------------------------------------------------------------|
|  Date Modified : (17/06/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (03/12/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (15/11/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (25/02/96)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (09/06/96)      | Modified  by  : Andy Yuen        |
|  Date Modified : (19/07/96)      | Modified  by  : Andy Yuen        |
|  Date Modified : (03/09/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (09/10/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (13/10/97)      | Modified  by  : Marnie Organo    |
|  Date Modified : (02/09/1999)    | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      :                                                    |
| (17/06/93)    : EGC 9139 To default non contract lines to prev line|
| (03/12/93)    : EGC 10060 - was not resetting flag after adding cmcb
| (15/11/95)    : PDL - Updated for version 9.                       |
| (25/02/96)    : PDL - Updated for new Multi Currency G/L           |
| (09/06/96)    : Use DELETE.h instead of FindPocr to get |
|                  base currency.                                     |
| (19/07/96)    : Add lpno as input parameter.                       |
| (03/09/97)    : Modified for Multilingual Conversion .             | 
| (09/10/97)    : Fixed report affected by change of length of       |
|                : glwk_user ref from 6 to 8.                         |     
| (13/10/97)    : Added ML for array.                                |
| $Log: cm_adj_jnl.c,v $
| Revision 5.7  2002/07/25 11:17:26  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.6  2002/07/24 08:38:41  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/18 06:12:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.4  2002/07/03 04:21:37  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/08/09 08:56:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/09 01:35:55  scott
| RELEASE 5.0
|
| Revision 5.1  2001/08/06 22:55:59  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:01:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:07:26  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:11:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:38:17  gerry
| Forced Revsions No Start to 2.0 Rel-15072000
|
| Revision 1.29  2000/03/06 06:37:58  gerry
| Fixed core dumps due to char * declarartions ---> made it char [].
|
| Revision 1.28  2000/03/01 03:07:57  gerry
| Reverted back const char *data to char* data
|
| Revision 1.27  2000/02/29 08:57:33  gerry
| New make and app.schema
|
| Revision 1.26  1999/12/07 19:54:53  cam
| Changes for GVision compatibility.  Added calls to heading () and scn_display ()
| before edit ().
|
| Revision 1.25  1999/12/06 01:32:18  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.24  1999/11/25 10:23:26  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.23  1999/11/17 06:39:04  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.22  1999/11/08 04:35:33  scott
| Updated to correct warnings from usage of -Wall flag on compiler.
|
| Revision 1.21  1999/10/16 05:10:06  nz
| Updated for warning
|
| Revision 1.20  1999/10/16 04:56:19  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.19  1999/10/01 07:48:17  scott
| Updated for standard function calls.
|
| Revision 1.18  1999/09/29 10:10:11  scott
| Updated to be consistant on function names.
|
| Revision 1.17  1999/09/17 04:40:03  scott
| Updated for ctime -> SystemTime, datejul -> DateToString etc.
|
| Revision 1.16  1999/09/16 04:44:35  scott
| Updated from Ansi Project
|
| Revision 1.14  1999/06/14 07:33:53  scott
| Updated to add log in heading + updated for new gcc compiler.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "cm_adj_jnl";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_adj_jnl/cm_adj_jnl.c,v 5.7 2002/07/25 11:17:26 scott Exp $";

#define		MAXLINES 500

#include <pslscr.h>	
#include <GlUtils.h>	
#include <twodec.h>	
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#define	CREDIT		(local_rec.dc_flag [0] == 'C')
#define	DEBIT		(local_rec.dc_flag [0] == 'D')
#define	LCL_TAB_ROW 3
#define	LCL_TAB_COL 10
#define TAB_SIZE 90 + MAXLEVEL
#define TAB_1   11
#define TAB_2   19
#define TAB_3   20 + MAXLEVEL
#define TAB_4   26 + MAXLEVEL
#define TAB_5   44 + MAXLEVEL
#define TAB_6   56 + MAXLEVEL
#define TAB_7   67 + MAXLEVEL

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cmcdRecord	cmcd_rec;
struct cmcmRecord	cmcm_rec;
struct cmcmRecord	cmcm2_rec;
struct cmcbRecord	cmcb_rec;
struct cmtrRecord	cmtr_rec;
struct cmhrRecord	cmhr_rec;

char		*data	= "data",
			*cmcm2	= "cmcm2";

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	char 	lcl_acc_type [] = "**P";
	char	loc_curr [4];

struct	storeRec {
	double	values;
} store [MAXLINES];

   	int		jnl_proof 		= 0;
   	int		printerNumber   = 0;

	char	gl_desc [6][21] ;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	dc_flag [2];
	float	sale;
	char	flag [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "jnldate",	 4, 15, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", "01/01/01", "Journal Date", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &glwkRec.tran_date},
	{2, TAB, "cont_no",	 MAXLINES, 1, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Cont No. ", "Enter Contract Number, Search Available. ",
		 YES, NO,  JUSTLEFT, "", "", cmhr_rec.cont_no},
	{2, TAB, "costhd",	 0, 1, CHARTYPE,
		"UUUU", "          ",
		" ", " ",  " Csthd ", "Enter Costhead Code, Search Available ",
		 YES, NO,  JUSTLEFT, "", "", cmcm_rec.ch_code},
	{2, TAB, "cost_desc",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		 ND, NO,  JUSTLEFT, "", "", cmcm_rec.desc},
	{2, TAB, "glacct",	0, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "Enter GL Account, Search Available - [SEARCH] for Contract Specific Codes, [NUMBER SEARCH] for General Search ",
		YES, NO,  JUSTLEFT, "1234567890*", "", glwkRec.acc_no},
	{2, TAB, "d/c",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "", " D/C ", "D (ebit) or C (redit). ",
		YES, NO,  JUSTLEFT, "DC", "", local_rec.dc_flag},
	{2, TAB, "glamt",	 0, 1, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "0", "   Cost Amount   ", " ",
		YES, NO, JUSTRIGHT, "0", "9999999999", (char *) &glwkRec.amount},
	{2, TAB, "glmr",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", "",
		ND, NO, JUSTLEFT, "", "", glmrRec.desc},
	{2, TAB, "sale",	 0, 1, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", "  Sale %   ", " ",
		NI, NO, JUSTRIGHT, "0", "999.99", (char *) &local_rec.sale},
	{2, TAB, "refer",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "User Reference ", "Default Available ",
		 NO, NO,  JUSTLEFT, "", "", glwkRec.user_ref},
	{2, TAB, "desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Narrative    ", "Default Available",
		 NO, NO,  JUSTLEFT, "", "", glwkRec.narrative},
	{2, TAB, "hash",  0, 1, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		ND, NO, JUSTLEFT, "", "", (char *) &cmhr_rec.hhhr_hash},
	{2, TAB, "flag",  0, 1, CHARTYPE,
		"U", "          ",
		"", "", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.flag},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Main Processing Routine. |
==========================*/
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
double	PrintJnlTotal	(void);
void	ProofTrans		(void);
void	Update			(void);
int		heading			(int);
void	SrchCmcm		(char *);
void	SrchCmcb		(void);
void	SrchCmhr		(char *);
void	FindDesc		(void);
void	AddNewCmcbs		(void);
int		NotInContract	(void);
void	tab_other		(int);
int		GetAcct			(void);
void	ShowCmhrsGl		(void);
void	AddCmtr			(void);
void	initML			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	int	i;

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		return (EXIT_SUCCESS);
	}
	printerNumber = atoi (argv [1]);

	tab_row = 3;
	tab_col = 10;

	OpenDB (); 

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	GL_SetAccWidth (comm_rec.co_no, TRUE);
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	initML ();		

	tab_data [0]._desc = ML ("Journal Header Screen");
	tab_data [1]._desc = ML ("Journal Detail Screen");

	init_vars (1);

	while (!prog_exit)
	{
		for (i = 0; i < MAXLINES; i++)
			store [i].values = 0.00;

		entry_exit = edit_exit = prog_exit = restart = 0;
		search_ok = 1;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart) 
			continue;
		/*-------------------------------
		| Enter screen 2 linear input . |
		-------------------------------*/
		heading (2);
		entry (2);
		if (prog_exit || restart) 
			continue;

		edit_all ();

		if (restart) 
			continue;

		ProofTrans ();

		/*-------------------------------------------
		| re-edit tabular if proof total incorrect. |
		-------------------------------------------*/
		while (jnl_proof)
		{
			edit_all ();
			if (restart)
				break;

			if (prog_exit)
			{
				prog_exit = 0;
				continue;
			}
			ProofTrans ();
		}
		if (prog_exit || restart) 
			continue;

		/*-----------------------------------
		| Create entry on transaction file. |
		-----------------------------------*/
		if (jnl_proof == 0)
			Update ();

	}
	/*------------------------
	| Program exit sequence. |
	------------------------*/
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec); 

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (loc_curr, "%-3.3s", comr_rec.base_curr);
	abc_fclose (comr);

	abc_alias (cmcm2, cmcm);
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2,cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	OpenGlmr ();
	OpenGlwk ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cmhr);
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmtr);
	GL_CloseBatch (printerNumber);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	static int first_cont;
	int		i	=	0;
	double  tot	=	0.00;
	char	last_narr [sizeof glwkRec.narrative] = "                    ";
	char	last_ref [sizeof glwkRec.user_ref]  = "               ";

	if (line_cnt == 0)
		first_cont = TRUE;
	else
		first_cont = FALSE;

	/*---------------------------------
	| call for pr_jntot so that it is |
	| recalculated even if a chnge is |
	| made to sub edit mode           |	
	---------------------------------*/

	if (!LCHECK ("jnldate"))
		tot = PrintJnlTotal ();

	if (LCHECK ("costhd"))
	{
		if (FLD ("costhd") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			if (search_key == FN5)
			{
				SrchCmcm (temp_str);
				return (EXIT_SUCCESS);
			}
			else
			{
				SrchCmcb ();
				return (EXIT_SUCCESS);
			}
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");

		while (cc)
		{
			i= prmptmsg (ML (mlCmMess016), "YNyn", 18, 19);

			if (i == 'n' || i == 'N')
			{
				print_at (19,18, "%-100.100s", " ");
				return (EXIT_FAILURE);
			}

			i = line_cnt;
			putval (line_cnt);

			snorm ();
			* (arg) = "cm_costhd";
			* (arg+ (1)) = (char *)0;
			shell_prog (2);
			swide ();
			heading (2);

			line_cnt = i;
			if (prog_status == ENTRY)
				lcount [2] = line_cnt;

			scn_display (2);
			getval (line_cnt);
			line_display ();

			cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
			if (!cc)
				AddNewCmcbs ();
		}

		if (NotInContract ())
		{
			i= prmptmsg (ML (mlCmMess016), "YNyn", 18, 19);

			print_at (19,18, "%-100.100s", " ");

			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			
			AddNewCmcbs ();
		}

		if (cmcb_rec.dtl_lvl [0] != 'A')
		{
			print_mess (ML (mlCmMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cont_no"))
	{
		if (dflt_used)
		{
			if ((first_cont || tot == 0.00) && prog_status == ENTRY)
			{
				local_rec.flag [0] = 'Y';
				tab_other (line_cnt);
				return (EXIT_FAILURE);
			}

			/*---------------
			| for edit mode |
			---------------*/
			if (local_rec.flag [0] == 'Y')
				return (EXIT_FAILURE);

			sprintf (cmcd_rec.text, "%-70.70s", " ");
			sprintf (cmcm_rec.ch_code, "%-4.4s", " ");
			sprintf (cmcm_rec.desc, "%-40.40s", " ");
			tab_other (line_cnt);
			return (EXIT_SUCCESS);
		}
		else
			pad_num (cmhr_rec.cont_no);

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");

		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cmhr_rec.status [0] == 'H' || 
		     cmhr_rec.status [0] == 'B' ||
		     cmhr_rec.status [0] == 'C')
		{
			print_mess (ML (mlCmMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		first_cont = FALSE;
		tab_other (line_cnt);
		cc = TRUE;
		while (prog_status != ENTRY && cc)
		{	
			DSP_FLD ("cont_no");
			get_entry (cur_field + 1);
			cc = spec_valid (cur_field + 1);
			DSP_FLD ("costhd");
		}
		cc = FALSE;
		return (EXIT_SUCCESS);
	}
	/*------------------------
	| Validate Journal Date. |
	------------------------*/

	if (LCHECK ("jnldate"))
	{
		if (dflt_used)
			glwkRec.tran_date = comm_rec.gl_date;

		return (EXIT_SUCCESS);
	}
			
	/*------------------------------------------
	| Validate General Ledger Account Number . |
	------------------------------------------*/
	if (LCHECK ("glacct")) 
	{
		if (dflt_used && strcmp (cmhr_rec.cont_no, "      "))
		{
			/*-----------------------
			| copy across wip gl acc |
			-----------------------*/
			strcpy (glwkRec.acc_no, cmhr_rec.wip_glacc);
			DSP_FLD ("glacct");
		}

		if (SRCH_KEY)
		{
			if (search_key == FN4)
			{
				ShowCmhrsGl ();
				return (EXIT_SUCCESS);
			}	
			else
			{
				cc = SearchGlmr (	comm_rec.co_no, 
					       	temp_str, 
						lcl_acc_type
					     );
				return (cc);
			}
		}

		cc = GetAcct ();
		if (cc)
		{
			print_mess (ML (mlStdMess024));
			sleep (sleepTime);
			clear_mess ();
			return (cc);
		}

		if (glmrRec.glmr_class [2][0] != 'P')
	    {
			print_mess (ML (mlStdMess025));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (lcl_acc_type [0] == '*')
			lcl_acc_type [0] = glmrRec.glmr_class [0][0];
		else
		{
			if (lcl_acc_type [0] != glmrRec.glmr_class [0][0])
			{
				print_mess (ML (mlCmMess019));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		if (prog_status != ENTRY)
		{
			if (local_rec.dc_flag [0] == ' ')
			{
				strcpy (local_rec.dc_flag, "D");
				glwkRec.amount = 0.00;
				store [line_cnt].values = 0.00;
				DSP_FLD ("glamt");
				DSP_FLD ("d/c");
				tot = PrintJnlTotal ();
			}
		}
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*==============================
	| Calculate/reprint run total. |
	==============================*/
	if (LCHECK ("glamt") || LCHECK ("d/c")) 
	{
		store [line_cnt].values = no_dec ((DEBIT) ? glwkRec.amount
					            : glwkRec.amount * -1);

		tot = PrintJnlTotal ();

		if (LCHECK ("glamt") && strcmp (cmhr_rec.cont_no, "      ")) 
		{
			local_rec.sale = cmhr_rec.est_prof;
			DSP_FLD ("sale");
		}
		else
		{
			local_rec.sale = 0.00;
			DSP_FLD ("sale");
		}
	    	return (EXIT_SUCCESS);
	}
			
	/*-----------------------------
	| Update total if ENTRY mode. |
	-----------------------------*/
	if (LCHECK ("desc")) 
	{
		if (end_input) 
			return (EXIT_SUCCESS);

		if (dflt_used)  
		{
			if (strcmp (cmcm_rec.ch_code, "    "))
				sprintf (glwkRec.narrative,"Costhead %-4.4s", 
							cmcm_rec.ch_code);
			else
				strcpy (glwkRec.narrative, last_narr);

		}
		strcpy (last_narr, glwkRec.narrative);
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Update reference. |
	-------------------*/
	if (LCHECK ("refer")) 
	{
		if (end_input) 
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			if (strcmp (cmhr_rec.cont_no, "      "))
				strcpy (glwkRec.user_ref, cmhr_rec.cont_no);
			else
				strcpy (glwkRec.user_ref, last_ref);

		}
		strcpy (last_ref, glwkRec.user_ref);
		DSP_FLD ("refer");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*============================
| Print Journal proof total. |
============================*/
double
PrintJnlTotal (void)
{
	int	i;
	double	total = 0.00;
	
		
	for (i = 0; i < MAXLINES; i++)
		total += no_dec (store [i].values);

	print_at (0, 90, ML (mlCmMess151),DOLLARS (total));
	return (total);
}

void
ProofTrans (void)
{
	int	i;
	double	chk_total = 0.00;
	
	for (i = 0; i < MAXLINES; i++)
		chk_total += no_dec (store [i].values);

	if (chk_total == 0.00) 
		jnl_proof = 0;
	else
	{
		jnl_proof = 1;
		sprintf (err_str, ML (mlCmMess060), DOLLARS (chk_total));
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
	}	
}

void
Update (void)
{
	int		monthPeriod;

	/*-------------------------------
	| Add transactions to glwk file.|
	-------------------------------*/
	strcpy (glwkRec.tran_type,"24");

	DateToDMY (glwkRec.tran_date, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no,"%02d", monthPeriod);

	glwkRec.post_date = TodaysDate ();
	sprintf (glwkRec.sys_ref,"%010ld", (long) comm_rec.term);
	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.stat_flag,"2");

	/*-------------------- 
	| Set to tab screen. |
	--------------------*/
	scn_set (2); 

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);	

		glwkRec.ci_amt 		= glwkRec.amount;
		glwkRec.loc_amount 	= glwkRec.amount;
		glwkRec.exch_rate 	= 1.00;
		strcpy (glwkRec.currency, loc_curr);
		strcpy (glmrRec.co_no,comm_rec.co_no);
		strcpy (glmrRec.acc_no, glwkRec.acc_no);

		GetAcct ();
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;

		strcpy (glwkRec.jnl_type, (DEBIT) ? "1" : "2");

		strcpy (glwkRec.alt_desc1, " ");
		strcpy (glwkRec.alt_desc2, " ");
		strcpy (glwkRec.alt_desc3, " ");
		strcpy (glwkRec.batch_no, " ");
		GL_AddBatch ();

		if (strcmp (cmhr_rec.cont_no, "      "))
			AddCmtr ();
	}
	strcpy (glwkRec.narrative,"                    ");

}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		if (scn == 1)
		{
			snorm ();
			box (0,3,80,1);
		}
		else
		{
			swide ();
			PrintJnlTotal ();
		}

		rv_pr (ML (mlCmMess010), (scn == 1) ? 25:51,0,1);
		line_at (1,0, (scn == 1) ? 80 : 132);
		line_at (20,0, (scn == 1) ? 80 : 132);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no,
					     comm_rec.co_name);
		line_at (22,0, (scn == 1) ? 80 : 132);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
SrchCmcm (
 char *	key_val)
{
	_work_open (4,0,40);
	save_rec ("#Costhead", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", key_val);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, key_val, strlen (key_val)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmcb (void)
{
	int cc1;

	_work_open (4,0,40);
	save_rec ("#Code", "#Costhead Description");

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;

	cc1 = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
		cc1 = TRUE;

	if (!cc1)
		cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",cmcb_rec.hhcm_hash);

	while (!cc && !cc1 &&
	       !strcmp (cmcm2_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmcm2_rec.ch_code, cmcm2_rec.desc);
		if (cc)
			break;

		cc1 = find_rec (cmcb, &cmcb_rec, NEXT, "r");
		if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
			cc1 = TRUE;

		if (!cc1)
			cc=find_hash (cmcm2,&cmcm2_rec,EQUAL,"r",
						cmcb_rec.hhcm_hash);
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmhr (
 char *	key_val)
{
	_work_open (8,0,30);
	save_rec ("#Cont. No.", "#Customer Order No.");
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);

	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
			!strncmp (cmhr_rec.cont_no, key_val,strlen (key_val)))
	{
		if (cmhr_rec.status [0] == 'H' || 
		     cmhr_rec.status [0] == 'B' ||
		     cmhr_rec.status [0] == 'C')
		{
			cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
			continue;
		}

		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
FindDesc (void)
{
	cmcd_rec.line_no = 0;
	cmcd_rec.stat_flag [0] = 'D';
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmcd, "DBFIND");
}

void
AddNewCmcbs (void)
{
	strcpy (cmcb_rec.budg_type, "V");
	strcpy (cmcb_rec.dtl_lvl, "A");
	cmcb_rec.budg_cost = 0.00;
	cmcb_rec.budg_qty = 0.00;
	cmcb_rec.budg_value = 0.00;
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = abc_add (cmcb, &cmcb_rec);
	if (cc)
		file_err (cc, cmcb, "DBADD");
}

int
NotInContract (void)
{

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = find_rec (cmcb, &cmcb_rec, EQUAL, "r");
	return (cc);
}

void
tab_other (
 int	line_no)
{
	if (prog_status == ENTRY || line_no < lcount [2])
	{
		if (strcmp (cmhr_rec.cont_no, "      "))
		{
			FLD ("glacct") = NE;
			sprintf (glwkRec.acc_no, "%-*.*s", 
									MAXLEVEL,MAXLEVEL,cmhr_rec.wip_glacc);
			DSP_FLD ("glacct");
		}
		else
			FLD ("glacct") = YES;

		if (strcmp (cmhr_rec.cont_no, "      "))
		{
			FLD ("costhd") = YES;
			FindDesc ();
		}
		else
		{
			FLD ("costhd") = NA;
			print_at (tab_row + line_no + 2, 23, "    ");
			sprintf (cmcd_rec.text, "%-70.70s", " ");
		}

		print_at (16, 18, ML (mlStdMess069), cmhr_rec.cont_no,
					cmcd_rec.text);
		print_at (17, 18, ML (mlStdMess070), 
					cmcm_rec.ch_code, cmcm_rec.desc);
		print_at (18, 18, ML (mlStdMess087), glmrRec.desc);
	}
	else
	{
		print_at (16, 18, ML (mlStdMess069), " ", " ");
		print_at (17, 18, ML (mlStdMess070), " ", " ");
		print_at (18, 18, ML (mlStdMess087), " ");
	}
}

int
GetAcct (void)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (glwkRec.acc_no, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	return (cc);
}

void
ShowCmhrsGl (void)
{
	_work_open (16,0,40);
	sprintf (err_str, "#%-*.*s", MAXLEVEL,MAXLEVEL,"Account  Number");
	save_rec (err_str, "#Description");

	cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");

	save_rec (cmhr_rec.wip_glacc, gl_desc [0]);
	save_rec (cmhr_rec.lab_glacc, gl_desc [1]);
	save_rec (cmhr_rec.o_h_glacc, gl_desc [2]);
	save_rec (cmhr_rec.sal_glacc, gl_desc [3]);
	save_rec (cmhr_rec.cog_glacc, gl_desc [4]);
	save_rec (cmhr_rec.var_glacc, gl_desc [5]);
	save_rec (cmhr_rec.int_glacc, gl_desc [6]);
		
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (glwkRec.acc_no, "%-*.*s", 
						MAXLEVEL,MAXLEVEL,cmhr_rec.int_glacc);
}

void
AddCmtr (void)
{
	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");

	cmtr_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmtr_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	cmtr_rec.hhbr_hash = cmcm_rec.hhbr_hash;
	cmtr_rec.sale_price = glwkRec.amount * (1.00 + (local_rec.sale / 100.00));
	cmtr_rec.sale_price *= (DEBIT) ? 1.00 : -1.00;
	cmtr_rec.cost_price = glwkRec.amount;
	cmtr_rec.cost_price *= (DEBIT) ? 1.00 : -1.00;
	cmtr_rec.qty = 1.00;
	cmtr_rec.disc_pc = 0.00;
	cmtr_rec.date = TodaysDate ();

	strcpy (cmtr_rec.time, TimeHHMMSS ());

	cc = abc_add (cmtr, &cmtr_rec);
	if (cc)
		file_err (cc, cmtr, "DBADD");
}

void
initML (void)
{
	strcpy (gl_desc [0], ML (mlCmMess199));
	strcpy (gl_desc [1], ML (mlCmMess200));
	strcpy (gl_desc [2], ML (mlCmMess201));
	strcpy (gl_desc [3], ML (mlCmMess202));
	strcpy (gl_desc [4], ML (mlCmMess203));
	strcpy (gl_desc [5], ML (mlCmMess204));
	strcpy (gl_desc [6], ML (mlCmMess205));
}
