/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( mh_purge.i.c    )                                |
|  Program Desc  : ( Input Program For Machine History Purges     )   |
|                  ( MH02                                         )   |
|---------------------------------------------------------------------|
|  Access files  :  mhdr, mhsd, comm, inmr, incc, insf, ccmr, cumr,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap    | Date Written  : 02/07/87         |
|---------------------------------------------------------------------|
|  Date Modified : (29/09/87)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (30/06/89)      | Modified  by  : Rog Gibbison.    |
|  Date Modified : (21/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (08/09/97)      | Modified  by  : Marnie Organo.   |
|  Date Modified : (13/08/99)      | Modified  by  : Mars dela Cruz   |
|                                                                     |
|  Comments      : Removed return from read_comm().                   |
|                : (30/06/89) - General Tidy Up & Debug               |
|                : (21/09/90) - General Update for New Scrgen. S.B.D. |
|                : (09/09/97) - Modified for Multilingual Conversion  |
|                : (13/08/99) - Modified for ANSI compliance.         |
|                                                                     |
|  Date Modified : (03/11/1997)    | Modified  by : Scott B Darrow    |
|   Comments     :   Updated for invoices number length change.       |

| $Log: mh_purge.i.c,v $
| Revision 5.3  2002/07/17 09:57:28  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:29:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:26  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:37  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:38  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:01:21  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/17 06:40:21  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/08 08:09:29  scott
| Updated for fix warnings due to usage of -Wall flag.
|
| Revision 1.9  1999/10/20 02:06:52  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/10/19 21:20:36  nz
| Updated from ansi testing
|
| Revision 1.7  1999/09/29 10:11:23  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 09:23:25  scott
| Updated from Ansi
|
| Revision 1.5  1999/06/15 03:05:30  scott
| Update for log + database name.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: mh_purge.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MH/mh_purge.i/mh_purge.i.c,v 5.3 2002/07/17 09:57:28 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_mh_mess.h>
#include <get_lpno.h>
#include <std_decs.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int ccmr_no_fields = 4;

	struct {
		char	cc_co_no[3];
		char	cc_est_no[3];
		char	cc_cc_no[3];
		long	cc_hhcc_hash;
	} ccmr_rec;

	/*=====================================
	| Machine History Detail Record File. |
	=====================================*/
	struct dbview mhdr_list[] ={
		{"mhdr_co_no"},
		{"mhdr_hhcc_hash"},
		{"mhdr_hhbr_hash"},
		{"mhdr_serial_no"},
		{"mhdr_sell_date"},
		{"mhdr_war_exp"},
	};

	int mhdr_no_fields = 6;

	struct {
		char	dr_co_no[3];
		long	dr_hhcc_hash;
		long	dr_hhbr_hash;
		char	dr_serial_no[26];
		long	dr_sell_date;
		long	dr_war_exp;
	} mhdr_rec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	systemDate[11];
	int	lpno;
	char	lp_str[2];
	char	back[2];
	char	onight[2];
	long	purge_date;
	char	purge[9];
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "lpno", 4, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No. ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 5, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "purge_date", 6, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "0", " Purge Cutoff Date. ", "All records with sold date older than this date will be purged. ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.purge_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================
| Function Prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int valid_sell_date (void);
int expired_warranty (void);
int  heading (int scn);

/*=========================== 
| Main Processing Routine . |
===========================*/
int main (
 int	argc, 
 char	*argv[])
{
	if (argc != 3) 
	{
		print_at (0,0,mlStdMess037,argv[0]);
		return (EXIT_SUCCESS);
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*==============================
	| Read common terminal record. |
	==============================*/
	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
   		entry_exit = 0;
   		prog_exit = 0;
   		restart = 0;
   		search_ok = 1;
	
		init_vars (1);
		heading (1);
		entry (1);

		if (prog_exit)
		{
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }

		heading (1);
		scn_display (1);
		edit (1);
		prog_exit = 1;
		if (restart)
		{
			shutdown_prog ();
			return (EXIT_SUCCESS);
		}
	}

	CloseDB (); 
	FinishProgram ();

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y') 
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				argv[1],
				local_rec.lp_str,
				local_rec.purge,
				argv[2],(char *) 0);
		}
		return (EXIT_SUCCESS);
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
			execlp(argv[1],
				argv[1],
				local_rec.lp_str,
				local_rec.purge, (char *)0);
		return (EXIT_SUCCESS);
	}

	execlp (argv[1],
		argv[1],
		local_rec.lp_str,
		local_rec.purge, (char *)0);

    return (EXIT_SUCCESS);	
}

/*========================	
| Program exit sequence. |
========================*/
void 
shutdown_prog (
 void)
{
	clear ();
	rset_tty ();
}

void 
OpenDB (
 void)
{
    abc_dbopen("data");

	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");

	strcpy (ccmr_rec.cc_co_no,comm_rec.tco_no);
	strcpy (ccmr_rec.cc_est_no,comm_rec.tes_no);
	strcpy (ccmr_rec.cc_cc_no,comm_rec.tcc_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)",cc,PNAME);

	abc_fclose ("ccmr");

	open_rec ("mhdr",mhdr_list,mhdr_no_fields,"mhdr_serial_id");
}

void 
CloseDB (
 void)
{
	abc_fclose("mhdr");
	abc_dbclose("data");
}
int 
spec_valid (
 int field)
{			
	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{	
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML(mlStdMess020));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.lp_str,"%d",local_rec.lpno);
		return (EXIT_SUCCESS);
	}
			
	if (LCHECK ("purge_date"))
	{
		strcpy (local_rec.purge,DateToString(local_rec.purge_date));
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================================
| Check if sell_date > nominated purge_date.	|
===============================================*/
int 
valid_sell_date (
 void)
{
	if (mhdr_rec.dr_sell_date > local_rec.purge_date)
		return(TRUE);
	else
		return(FALSE);
}

/*=======================================
| Check if warranty date is expired	|
=======================================*/
int 
expired_warranty (
 void)
{
	if (mhdr_rec.dr_war_exp <= StringToDate(local_rec.systemDate))
		return (TRUE);
	else
		return (FALSE);
}

int 
heading (
 int scn)
{

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML(mlMhMess022),18,0,1);
		move (0,1);
		line (80);

		box (0,3,80,3);

		move (0,20);
		line (80);
		move (0,21);
		print_at (21,0,ML(mlStdMess038),comm_rec.tco_no,clip(comm_rec.tco_name));
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
