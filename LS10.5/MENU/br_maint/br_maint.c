/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: br_maint.c,v 5.3 2002/07/08 07:53:40 scott Exp $
|  Program Name  : (br_maint.c    )                                   |
|  Program Desc  : (Branch Master File Maintenence.             )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: br_maint.c,v $
| Revision 5.3  2002/07/08 07:53:40  scott
| S/C 004062 - Added delay on area code.
|
| Revision 5.2  2001/08/09 05:13:14  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:09  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:58  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:17  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/03/08 07:01:19  robert
| fixed alignments on addresses. address2 field accepts up 39 char (should be 40)
|
| Revision 3.2  2001/03/01 23:56:02  scott
| Updated to place sleep on error message that company is not on file.
| Updated to add app.schema and clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: br_maint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/br_maint/br_maint.c,v 5.3 2002/07/08 07:53:40 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

#define		BY_BRANCH	1
#define		BY_DEPART	2

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct exafRecord	exaf_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct crbkRecord	crbk_rec;

	char	*data = "data";

	int		newBranch 	= FALSE,
			envDbCo 	= FALSE,
			envDbFind 	= FALSE,
			envDbMcurr	= FALSE;

	char	*currentUser,
			branchNumber [3];

	int		SO_NUMBERS =	BY_BRANCH;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	dbt_no [7];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "co_no",	 3, 2, CHARTYPE,
		"NN", "          ",
		" ", "", "Company No.           ", " ",
		 NE, NO, JUSTRIGHT, "0", "99", esmr_rec.co_no},
	{1, LIN, "co_name",	 3, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", comr_rec.co_name},
	{1, LIN, "br_no",	 5, 2, CHARTYPE,
		"NN", "          ",
		" ", "", "Branch No.            ", " ",
		 NE, NO, JUSTRIGHT, "0", "99", esmr_rec.est_no},
	{1, LIN, "br_name",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name           ", " ",
		 NO, NO,  JUSTLEFT, "", "", esmr_rec.est_name},
	{1, LIN, "br_short",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Short Name            ", " ",
		 NO, NO,  JUSTLEFT, "", "", esmr_rec.short_name},
	{1, LIN, "br_addr1",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 1             ", " ",
		 NO, NO,  JUSTLEFT, "", "", esmr_rec.adr1},
	{1, LIN, "br_addr2",	9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 2             ", " ",
		 NO, NO,  JUSTLEFT, "", "", esmr_rec.adr2},
	{1, LIN, "br_addr3",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 3             ", " ",
		 NO, NO,  JUSTLEFT, "", "", esmr_rec.adr3},
	{1, LIN, "area_code",	11, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Area code             ", " ",
		 YES, NO, JUSTRIGHT, "", "", esmr_rec.area_code},
	{1, LIN, "area_desc",	11, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", exaf_rec.area},
	{1, LIN, "bank",		12, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Default Bank.         ", "Enter Default Bank.  [SEARCH] Available. ",
		 ND, NO,  JUSTLEFT, "", "", esmr_rec.dflt_bank},
	{1, LIN, "bank_desc",	12, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "next_sav",	14, 2, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Saved invoice No ", "Input Saved Invoice #",
		YES, NO, JUSTRIGHT, "", "", (char *)&esmr_rec.nx_sav_inv},
	{1, LIN, "csh_pref",	16, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Cash Invoice Prefix   ", " ",
		YES, NO,  JUSTLEFT, "", "", esmr_rec.csh_pref},
	{1, LIN, "next_csh",	16, 30, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Cash Invoice No    ", "Input Last Cash Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&esmr_rec.nx_csh_inv},
	{1, LIN, "chg_pref",	17, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Charge Invoice Prefix ", " ",
		YES, NO,  JUSTLEFT, "", "", esmr_rec.chg_pref},
	{1, LIN, "next_chg",	17, 30, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Charge Invoice No  ", "Input Last Charge Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&esmr_rec.nx_inv_no},
	{1, LIN, "man_pref",	18, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Debit Note Prefix     ", " ",
		YES, NO,  JUSTLEFT, "", "", esmr_rec.man_pref},
	{1, LIN, "next_man",	18, 30, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Debit Note No      ", "Input Last Manual Invoice #.",
		YES, NO, JUSTRIGHT, "", "", (char *)&esmr_rec.nx_man_no},
	{1, LIN, "crd_pref",	19, 2, CHARTYPE,
		"UU", "          ",
		" ", " ", "Credit Note  Prefix   ", " ",
		YES, NO,  JUSTLEFT, "", "", esmr_rec.crd_pref},
	{1, LIN, "next_crd",	19, 30, LONGTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Next Credit Note No     ", "Input Last Credit Note #",
		YES, NO, JUSTRIGHT, "", "", (char *)&esmr_rec.nx_crd_nte_no},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};


extern	int				TruePosition;
	void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
int		spec_valid		 (int);
void	SrchComr		 (char *);
void	SrchEsmr		 (char *);
void	SrchExaf		 (char *);
void	SrchCrbk		 (char *);
void	Update			 (void);
void	AddInei		 	 (void);
void	DisplayScreen	 (void);
void	ProgramStatus	 (int);
int		heading			 (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv  [])
{
	char	*sptr;

	currentUser = getenv ("LOGNAME");

	TruePosition	=	TRUE;

	/*--------------------------
	| Check for multi-currency |
	--------------------------*/
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SO_NUMBERS");
	SO_NUMBERS = (sptr == (char *)0) ? BY_BRANCH : atoi (sptr);

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();	
	set_masks 	();	
	init_vars 	(1);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	if (envDbMcurr)
	{
		FLD ("bank") 		= YES;
		FLD ("bank_desc") 	= NA;
	}

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		entry_exit	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newBranch 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		if (SO_NUMBERS == BY_DEPART)
		{
			FLD ("csh_pref")	=	ND;
			FLD ("chg_pref")	=	ND;
			FLD ("crd_pref")	=	ND;
			FLD ("man_pref")	=	ND;
			FLD ("next_csh")	=	ND;
			FLD ("next_chg")	=	ND;
			FLD ("next_crd")	=	ND;
			FLD ("next_man")	=	ND;
		}
		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;
			
		/*------------------------
		| Edit screen 1 linear . |
		------------------------*/
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
	
/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (crbk);
	abc_fclose (esmr);
	abc_fclose (exaf);
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/*--------------------------
	| Validate Company Number. |
	--------------------------*/
	if (LCHECK ("co_no"))
	{
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (comr_rec.co_no,esmr_rec.co_no);
		cc = find_rec (comr , &comr_rec, COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess130));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (esmr_rec.co_no [0] == '0')
		{
			print_mess (ML ("Sorry, Company 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("co_name");
		strcpy (err_str, ML (mlStdMess038));
		print_at (22,0,err_str, comr_rec.co_no, comr_rec.co_name);
	}

	/*-------------------------
	| Validate Branch Number. |
	-------------------------*/
	if (LCHECK ("br_no"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (esmr_rec.est_no [0] == '0')
		{
			print_mess (ML ("Sorry, Branch 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (!strcmp (esmr_rec.est_no, "  "))
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		newBranch = find_rec (esmr , &esmr_rec, COMPARISON,"w");
		if (!newBranch)
		{
			entry_exit = 1;
			DSP_FLD ("br_name");
			DSP_FLD ("br_short");
			DSP_FLD ("br_addr1");
			DSP_FLD ("br_addr2");
			DSP_FLD ("br_addr3");
			strcpy (err_str, ML (mlStdMess039));
			print_at (23,0,err_str, esmr_rec.est_no,esmr_rec.est_name);
			strcpy (exaf_rec.co_no,esmr_rec.co_no);
			strcpy (exaf_rec.area_code,esmr_rec.area_code);
			if (find_rec (exaf , &exaf_rec, COMPARISON,"r"))
			{
				strcpy (exaf_rec.area_code,"  ");
				sprintf (exaf_rec.area,"%40.40s", " ");
			}

			strcpy (comm_rec.co_no,  esmr_rec.co_no);
			strcpy (comm_rec.est_no, esmr_rec.est_no);
			strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

			/*----------------------
			| Lookup default bank. |
			----------------------*/
			if (envDbMcurr)
			{
				strcpy (crbk_rec.co_no, esmr_rec.co_no);
				sprintf (crbk_rec.bank_id, "%-5.5s", esmr_rec.dflt_bank);
				cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
				if (cc)
				{
					strcpy (esmr_rec.dflt_bank, "     ");
					sprintf (crbk_rec.bank_name, 
						"%-40.40s", 
						" ");
				}
			}
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Area Code. |
	---------------------*/
	if (LCHECK ("area_code")) 
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (prog_status == ENTRY)
			DSP_FLD ("area_code");

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,esmr_rec.co_no);
		strcpy (exaf_rec.area_code,esmr_rec.area_code);
		cc = find_rec (exaf , &exaf_rec, COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess108));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("area_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("bank"))
	{
		if (FLD ("bank") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no, esmr_rec.co_no);
		sprintf (crbk_rec.bank_id, "%-5.5s", esmr_rec.dflt_bank);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{	
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("bank_desc");

		return (EXIT_SUCCESS);
	}
	
	return (EXIT_SUCCESS);
}

/*==================
| Search for comr. |
==================*/
void
SrchComr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Co.","#Company Name.");
	sprintf (comr_rec.co_no,"%-2.2s",key_val);
	cc = find_rec (comr , &comr_rec, GTEQ,"r");
	while (!cc && !strncmp (comr_rec.co_no,key_val,strlen (key_val)))
	{
		cc = save_rec (comr_rec.co_no,comr_rec.co_name);
		if (cc)
			break;

		cc = find_rec (comr , &comr_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_no,"%-2.2s", temp_str);
	cc = find_rec (comr , &comr_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");
}
/*==================
| Search for esmr. |
==================*/
void
SrchEsmr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Br.","#Branch Name.");
	sprintf (esmr_rec.est_no,"%-2.2s",key_val);
	cc = find_rec (esmr , &esmr_rec, GTEQ,"r");
	while (!cc && !strcmp (comr_rec.co_no, esmr_rec.co_no) &&
		      !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no,esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr , &esmr_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no,comr_rec.co_no);
	sprintf (esmr_rec.est_no,"%-2.2s", temp_str);
	cc = find_rec (esmr , &esmr_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}

/*==================
| Search for area. |
==================*/
void
SrchExaf (
 char *	key_val)
{
	work_open ();
	save_rec ("#Ar","#Area.");
	strcpy (exaf_rec.co_no,esmr_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",key_val);
	cc = find_rec (exaf , &exaf_rec, GTEQ,"r");
	while (!cc && !strcmp (exaf_rec.co_no,esmr_rec.co_no) &&
		      !strncmp (exaf_rec.area_code,key_val,strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf , &exaf_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,esmr_rec.co_no);
	sprintf (exaf_rec.area_code,"%-2.2s",temp_str);
	cc = find_rec (exaf , &exaf_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

/*==================
| Search for bank. |
==================*/
void
SrchCrbk (
 char *	key_val)
{
	work_open ();
	save_rec ("#Bank","#Bank Name.");
	strcpy (crbk_rec.co_no, esmr_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-2.2s", key_val);
	cc = find_rec (crbk , &crbk_rec, GTEQ, "r");
	while (!cc && !strcmp (crbk_rec.co_no, esmr_rec.co_no) &&
		      !strncmp (crbk_rec.bank_id, key_val, strlen (key_val)))
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
}

void
Update (void)
{
	clear ();

	/*--------------------------------
	| Update existing Branch Record. |
	--------------------------------*/
	if (newBranch)
	{
		cc = abc_add (esmr, &esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBADD");

		abc_unlock ("esmr");

		AddInei ();
	}
	else
	{
		cc = abc_update (esmr, &esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBUPDATE");
	}
}

/*==============================
| Add inventory branch record. |
==============================*/
void
AddInei (void)
{
	int	mr_cnt = 0;
	
	DisplayScreen ();

	strcpy (inmr_rec.co_no, esmr_rec.co_no);
	strcpy (inmr_rec.item_no, "                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, esmr_rec.co_no))
	{
		mr_cnt++;
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, esmr_rec.est_no);
		inei_rec.std_batch = 1.00;
		inei_rec.min_stock = inmr_rec.min_quan;
		inei_rec.max_stock = inmr_rec.max_quan;
		inei_rec.safety_stock = inmr_rec.safety_stock;
		strcpy (inei_rec.abc_code, inmr_rec.abc_code);
		strcpy (inei_rec.abc_update, inmr_rec.abc_update);
		strcpy (inei_rec.stat_flag, "0");
		cc = abc_add (inei, &inei_rec);
		if (cc)
			file_err (cc, "inei", "DBADD");

		if (mr_cnt % 25 == 0)
		{
			rv_pr (ML (mlMenuMess062), 7,9,0);
			rv_pr (ML (mlMenuMess063),40,9,1);
			ProgramStatus (mr_cnt);
		}
		if (mr_cnt % 50 == 0)
		{
			rv_pr (ML (mlMenuMess062), 7,9,0);
			rv_pr (ML (mlMenuMess063),40,9,1);
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
DisplayScreen (void)
{
	clear ();
	crsr_off ();
	box (0,0,78,21);
	box (20,2,39,1);
	rv_pr (ML (mlMenuMess064), 22, 3, 1);

	box (4,8,30,1);
	box (38,8,30,1);
	box (5,15,66,1);
	rv_pr (ML (mlMenuMess062),   7,9,0);
	rv_pr (ML (mlMenuMess063),  40,9,1);
	rv_pr (ML (mlMenuMess065), 28,14,1);
	
}

void	
ProgramStatus (
 int	br_no)
{
	print_at (16,8,ML (mlStdMess035));
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

		rv_pr (ML (mlMenuMess066),24,0,1);
		line_at (1,0,80);

		box (0, 2, 80, (SO_NUMBERS == BY_BRANCH) ? 17 : 12);

		line_at (4,1,79);
		line_at (13,1,79);

		if (SO_NUMBERS == BY_BRANCH)
			line_at (15,1,79);
		
		if (!envDbMcurr)
			line_at (14,1,79);

		line_at (21,0,80);
		
		print_at (22,0,ML (mlStdMess038), comr_rec.co_no, comr_rec.co_name);
		print_at (23,0,ML (mlStdMess039), esmr_rec.est_no,esmr_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
