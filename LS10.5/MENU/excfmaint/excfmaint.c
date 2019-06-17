/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (excfmaint.c   )                                   |
|  Program Desc  : (Maintain Cetegory File Record.              )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, excf,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  excf,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (27/04/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (05/10/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (30/11/92)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (18/05/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (04/09/97)      | Modified  by  : Jiggs A Veloz    |
|  Date Modified : (30/08/1999)    | Modified  by  : Alvin Misalucha  |
|                                                                     |
|  Comments      : 21/02/89 add excf_max_rate, del excf_?_code_rate R |
|  (05/10/90)    : General Update for New Scrgen. S.B.D.              |
|  (30/11/92)    : Updated to add audit for S/C DFT-8024.             |
|  (18/05/93)    : Updated to controlled drugs flag. S/C PSM-8848.    |
|  (04/09/97)    : SEL - Multilingual Conversion. Replaced printf w/  |
|                :       print_at.                                    |
|  (30/08/1999)  : Converted to ANSI format.                          |
|                :                                                    |
|                :                                                    |
| $Log: excfmaint.c,v $
| Revision 5.4  2002/04/11 03:42:12  scott
| Updated to add comments to audit files.
|
| Revision 5.3  2001/10/05 02:58:15  cha
| Added code to produce audit files.
|
| Revision 5.2  2001/08/09 05:13:28  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:23  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:08:21  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:29:37  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/27 03:47:18  jason
| Corrected the initialisation of newCode to FALSE. If not seen then that is
| the only time it is going to be TRUE.
|
| Revision 3.0  2000/10/10 12:16:01  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/08/28 02:27:14  scott
| General Maintenance - Added app.schema
|
| Revision 2.1  2000/08/21 07:00:29  val
| Updated changes.                    
|
| Revision 2.0  2000/07/15 09:00:12  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/05/18 05:33:54  scott
| Updated to prevent blank category from being added.
|
| Revision 1.11  1999/12/06 01:47:13  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/25 10:23:59  scott
| Updated to remove c++ comment lines and replace with standard 'C'
|
| Revision 1.9  1999/11/16 09:41:55  scott
| Updated for warnings due to usage of -Wall flag.
|
| Revision 1.8  1999/09/29 10:11:07  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/17 07:26:56  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.6  1999/09/16 04:11:39  scott
| Updated from Ansi Project
|
| Revision 1.5  1999/06/15 02:35:59  scott
| Update to add log + change database name + general look.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: excfmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/excfmaint/excfmaint.c,v 5.4 2002/04/11 03:42:12 scott Exp $";

#include <pslscr.h>
#include <ml_menu_mess.h>
#include <ml_std_mess.h>
#include <DBAudit.h>

	char	*currentUser;

#include	"schema"

struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct posdtupRecord	posdtup_rec;

	int		envVarPosInstalled 	= FALSE,
			envVarSkContDrugs 	= FALSE,
   			newCode				= FALSE;

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct {
	char	dummy [11];
	char	prev_code [12];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",	 4, 31, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Category Code", " ",
		 NE, NO,  JUSTLEFT, "", "", excf_rec.cat_no},
	{1, LIN, "desc",	 5, 31, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description  ", " ",
		YES, NO,  JUSTLEFT, "", "", excf_rec.cat_desc},
	{1, LIN, "max_disc",	 7, 31, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Max Discount.", "Input maximum Discount allowed for category.",
		YES, NO,  JUSTLEFT, "0.00", "100.00", (char *)&excf_rec.max_disc},
	{1, LIN, "min_marg",	8, 31, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Min Margin Percent.", "Input minimum Margin allowed for category.",
		YES, NO,  JUSTLEFT, "0.00", "100.00", (char *)&excf_rec.min_marg},
	{1, LIN, "gp_mkup",	9, 31, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0", "Gross profit Markup.", "Input GP margin required for category. ",
		YES, NO,  JUSTLEFT, "0.00", "100.00", (char *)&excf_rec.gp_mkup},
	{1, LIN, "item_alloc",	11, 31, CHARTYPE,
		"U", "          ",
		" ", "Y", "Allocate Items to Category.", "Input Y (es) is items can be allocated to category.",
		YES, NO,  JUSTLEFT, "YN", "", excf_rec.item_alloc},
	{1, LIN, "no_days",	12, 31, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Min no of days trans are held.", "Enter min number of days transactions are to stay on file. ",
		YES, NO,  JUSTLEFT, "0", "9999", (char *)&excf_rec.no_days},
	{1, LIN, "no_trans",	13, 31, INTTYPE,
		"NNNN", "          ",
		" ", "0", "Min no of transactions held.", "Enter min number of transaction to hold after min-days expires.",
		YES, NO,  JUSTLEFT, "0", "9999", (char *)&excf_rec.no_trans},
	{1, LIN, "controlledDrugs",	15, 31, CHARTYPE,
		"U", "          ",
		" ", "N", "Controlled Drugs Category.", "Input Y (es) is category is for Controlled drugs.",
		ND, NO,  JUSTLEFT, "YN", "", excf_rec.cont_drugs},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*==========================
| Function prototypes      |
==========================*/
int		main			 (int argc, char * argv []);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	Update			 (void);
void	SrchExcf		 (char *);
void	PrintOtherBits	 (void);
void	UpdatePosdtup	 (void);
int		heading			 (int);

/*--------------------------
| Main Processing Routine. |
--------------------------*/
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;

	sptr = chk_env ("SK_CONT_DRUGS");
	envVarSkContDrugs = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*------------------------------
	| Check for POS installation.  |
	------------------------------*/
	sptr = chk_env ("POS_INSTALLED");
	envVarPosInstalled = (sptr == (char *)0) ? 0 : atoi (sptr);

	currentUser = getenv ("LOGNAME");

	SETUP_SCR (vars);

	FLD ("controlledDrugs") = (envVarSkContDrugs) ? YES : ND;

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newCode		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		heading (1);
		entry (1);
		if (restart || prog_exit) 
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

/*=========================
| Program exit sequence	. |
=========================*/
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

	open_rec (excf,excf_list,EXCF_NO_FIELDS,"excf_id_no");
	if (envVarPosInstalled)
		open_rec (posdtup, posdtup_list, POSDTUP_NO_FIELDS, "pos_no1");

	/*
	 * Open audit file.
	 */
	OpenAuditFile ("ProductCategoryMaster.txt");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	if (envVarPosInstalled)
		abc_fclose (posdtup);
	abc_fclose (excf);
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose ("data");
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (excf_rec.cat_no, "           "))
		{
			print_mess (ML ("Category cannot be blank"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		newCode = FALSE;
		strcpy (excf_rec.co_no,comm_rec.co_no);
		cc = find_rec (excf,&excf_rec,COMPARISON,"U");
		if (cc) 
			newCode = TRUE;
		else    
		{
			/*
	 		 * Save old record.
	 		 */
			SetAuditOldRec (&excf_rec, sizeof (excf_rec));
			entry_exit = 1;
		}

		return (EXIT_SUCCESS);
	}

	if (!strncmp (FIELD.label,"no_",3))
	{
		PrintOtherBits ();
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (void)
{
	clear ();

	/*--------------------------------------------------
	| Add or Update product group description record . |
	--------------------------------------------------*/

	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.stat_flag,"0");
	if (!envVarSkContDrugs)
		strcpy (excf_rec.cont_drugs, "N");

	if (newCode == TRUE)
	{
		cc = abc_add (excf,&excf_rec);
		if (cc) 
			file_err (cc, excf, "DBADD");

		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	}
	else
	{
		/*
		 * Update changes audit record.
		 */
		 sprintf (err_str, "%s : %s (%s)", ML ("Category"), excf_rec.cat_no, excf_rec.cat_desc);
		 AuditFileAdd (err_str, &excf_rec, excf_list, EXCF_NO_FIELDS);
		cc = abc_update (excf,&excf_rec);
		if (cc) 
			file_err (cc, excf, "DBUPDATE");

	}
	if (envVarPosInstalled)
		UpdatePosdtup ();	

	abc_unlock (excf);
	strcpy (local_rec.prev_code,excf_rec.cat_no);
}

void
SrchExcf (
 char *	key_val)
{
	work_open ();
	save_rec ("#Category","#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",key_val);
	cc = find_rec (excf,&excf_rec,GTEQ,"r");

	while (!cc && !strcmp (excf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (excf_rec.cat_no,key_val,strlen (key_val)))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf,&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf,&excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

void
PrintOtherBits (void)
{
	print_at (12,40,"                                    ");
	print_at (13,40,"                                    ");

	
	/*------------------------------
	| system default will be used. |
	------------------------------*/
	if (excf_rec.no_days == 0)
		print_at (12, 40, ML (mlMenuMess196));

	/*------------------------------------
	| transaction will never be deleted. |
	------------------------------------*/
	if (excf_rec.no_days == 9999)
		print_at (12, 40, ML (mlMenuMess197)); 

	if (excf_rec.no_trans == 0)
		print_at (13, 40, ML (mlMenuMess196));

	if (excf_rec.no_trans == 9999)
		print_at (13, 40, ML (mlMenuMess197)); 
	
	fflush (stdout);
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
		/*------------------------------------
		| External Category File Maintenance |
		------------------------------------*/
		rv_pr (ML (mlMenuMess147), 20,0,1);

		/*---------------
		| Last Code: %s |
		---------------*/
		print_at (0,55, ML (mlMenuMess217), local_rec.prev_code);
		line_at (1,0,80);

		if (scn == 1)
		{
			box (0,3,80, (envVarSkContDrugs) ? 12 : 10);
			line_at (6,1,79);
			line_at (10,1,79);

			if (envVarSkContDrugs)
				line_at (14,1,79);
		
		}
		PrintOtherBits ();

		line_at (20,0,80);
		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (21,0, "%s", err_str);
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*=======================
| Updated pos record (s) |
=======================*/
void
UpdatePosdtup (void)
{
    posdtup_rec.pos_no = 0;
	strcpy (posdtup_rec.file_name,excf);
	posdtup_rec.record_hash = excf_rec.hhcf_hash;

	abc_add (posdtup,&posdtup_rec);
}


