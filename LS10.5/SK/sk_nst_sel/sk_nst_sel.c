/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sk_nst_sel.c   )                                 |
|  Program Desc  : ( Stock Take Freeze Input.                     )   |
|                  ( StockSheet Print Input.                      )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, incc,     , insc, lomr, excf,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 21/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (12/07/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (28/07/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (28/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (05/11/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/06/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (14/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (04/02/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (01/10/97)      | Modified  by  : Leah Manibog. 	  |
|                                                                     |
|  Comments      : (28/09/90) - General Update for New Scrgen. S.B.D. |
|                : (08/07/91) - Updated to fix S/C PSL-4735.          |
|                : (05/11/91) - Updated for Mod S/C DFH-5940          |
|    (16/06/92)  : exclude values of SK_IVAL_CLASS from stock take    |
|                : SC DFH 7096                                        |
|  (14/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (04/02/94)    : DHL 10209. Add insc_serial_take.                   |
|  (01/10/97)    : Updated for Multilingual Conversion.               |
|                                                                     |
| $Log: sk_nst_sel.c,v $
| Revision 5.5  2002/11/28 06:21:02  robert
| SC0052 - Updated to wait for subprocess to finish
|
| Revision 5.4  2002/07/17 09:57:57  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/28 08:46:38  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.2  2001/08/09 09:19:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:15  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:49  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:31  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nst_sel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_sel/sk_nst_sel.c,v 5.5 2002/11/28 06:21:02 robert Exp $";

#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<pslscr.h>
#include 	<get_lpno.h>

#define		STAKE_REP	( !strncmp(prog_run, "sk_nst_vsel", 11) )
#define		FREEZE		( !strncmp(progname, "sk_freeze",    9) )
#define		FRZ_NOR 	( !strncmp(progname, "sk_nst_frz",  10) )
#define		FRZ_SER 	( !strncmp(progname, "sk_nst_sfrz", 11) )

#ifdef GVISION
extern 		bool	gbSysExec;
#endif

	/*================================================================
	| Special fields and flags  ##################################.  |
	================================================================*/
	char	prog_run[41];
	char	progname[41];
	char	progdesc[21];
	int	new_insc = FALSE;
	int	pid = 0;
	int	programRun = FALSE;

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"},
	};

	int comm_no_fields = 8;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	tes_no[3];
		char	tes_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*=======================================
	| External Category Record (file excf). |
	=======================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
		{"excf_stat_flag"}
	};

	int excf_no_fields = 4;

	struct {
		char 	cf_co_no[3];
		char 	cf_cat_no[12];
		char 	cf_description[41];
		char 	cf_stat_flag[2];
	} excf_rec;

	/*=================================
	| Cost Centre Master File (ccmr). |
	=================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_name"},
		{"ccmr_reports_ok"}
	};

	int ccmr_no_fields = 6;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_cc_no[3];
		long	cm_hhcc_hash;
		char	cm_name[41];
		char	cm_reports_ok[2];
	} ccmr_rec;

	/*===========================
	|  Stock Take Control File. |
	===========================*/
	struct dbview insc_list[] ={
		{"insc_hhcc_hash"},
		{"insc_stake_code"},
		{"insc_start_date"},
		{"insc_start_time"},
		{"insc_frz_date"},
		{"insc_frz_time"},
		{"insc_description"},
		{"insc_serial_take"},
	};

	int insc_no_fields = 8;

	struct {
		long	sc_hhcc_hash;
		char	sc_stake_code[2];
		long	sc_start_date;
		char	sc_start_time[6];
		long	sc_frz_date;
		char	sc_frz_time[6];
		char	sc_description[41];
		char	sc_serial_take[2];
	} insc_rec;

	/*=======================
	| Location Master File. |
	=======================*/
	struct dbview lomr_list[] ={
		{"lomr_hhcc_hash"},
		{"lomr_location"},
		{"lomr_desc"},
	};

	int lomr_no_fields = 3;

	struct {
		long	lo_hhcc_hash;
		char	lo_location[11];
		char	lo_desc[41];
	} lomr_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_alpha_code"},
		{"inmr_supercession"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_quick_code"},
		{"inmr_abc_code"},
	};

	int inmr_no_fields = 11;

	struct {
		char 	mr_co_no[3];
		char 	mr_item_no[17];
		long 	mr_hhbr_hash;
		long 	mr_hhsi_hash;
		char 	mr_alpha_code[17];
		char 	mr_super_no[17];
		char 	mr_maker_no[17];
		char 	mr_alternate[17];
		char 	mr_class[2];
		char 	mr_description[41];
		char 	mr_quick_code[9];
		char 	mr_abc_code[2];
	} inmr_rec;

	int		mult_loc = 0;
	int		sk_st_pfrz = 0;
	char	*inval_cls;
 	char 	*result;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	s_cat[12];
	char	s_class[2];
	char	s_desc[41];
	char	e_cat[12];
	char	e_class[2];
	char	e_desc[41];
	char	s_group[17];
	char	e_group[17];
	char	s_item[17];
	char	e_item[17];
	char	stk_mode[2];
	char	mode_desc[41];
	int		lpno;
	char	lp_str[3];
	char	by_what[2];
	char	rep_by[11];
} local_rec;

static	struct	var	vars[] =
{
	/*-----------------------
	| By Class-Category	|
	-----------------------*/
	{1, LIN, "st_class",	 5, 25, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class", "Input Start Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.s_class},
	{1, LIN, "st_cat",	 6, 25, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "Start Category", "Input Start Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_cat},
	{1, LIN, "sdesc",	 7, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_desc},
	{1, LIN, "en_class",	 9, 25, CHARTYPE,
		"U", "          ",
		" ", "Z", "End Class", "Input End Class A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.e_class},
	{1, LIN, "en_cat",	10, 25, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", " ", "End Category", "Input End Inventory Category.",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_cat},
	{1, LIN, "edesc",	11, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_desc},
	{1, LIN, "stk_mode1",	13, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.stk_mode},
	{1, LIN, "mdesc",	14, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mode_desc},
	{1, LIN, "lpno1",	13, 60, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	/*-------------------
	| By Product        |
	-------------------*/
	{2, LIN, "st_item",	 5, 25, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start Item", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_item},
	{2, LIN, "sdesc",	 6, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_desc},
	{2, LIN, "en_item",	 8, 25, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "End  Item", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_item},
	{2, LIN, "edesc",	 9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_desc},
	{2, LIN, "stk_mode2",	11, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.stk_mode},
	{2, LIN, "mdesc",	12, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mode_desc},
	{2, LIN, "lpno2",	11, 60, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	/*-------------------
	| By ABC Code       |
	-------------------*/
	{3, LIN, "st_abc",	 5, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Start ABC Code", "Input ABC Code A-D.",
		YES, NO,  JUSTLEFT, "ABCD", "", local_rec.s_class},
	{3, LIN, "en_abc",	 6, 25, CHARTYPE,
		"U", "          ",
		" ", "", "End ABC Code", "Input ABC Code A-D.",
		YES, NO,  JUSTLEFT, "ABCD", "", local_rec.e_class},
	{3, LIN, "stk_mode3",	 8, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.stk_mode},
	{3, LIN, "mdesc",	 9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mode_desc},
	{3, LIN, "lpno3",	 8, 60, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	/*-------------------
	| By Location       |
	-------------------*/
	{4, LIN, "st_loc",	 5, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Start Location", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_cat},
	{4, LIN, "sdesc",	 6, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.s_desc},
	{4, LIN, "en_loc",	 8, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "End Location", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_cat},
	{4, LIN, "sdesc",	 9, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.e_desc},
	{4, LIN, "stk_mode4",	11, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.stk_mode},
	{4, LIN, "mdesc",	12, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mode_desc},
	{4, LIN, "lpno4",	11, 60, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer No", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},

	{5, LIN, "stk_mode5",	 5, 25, CHARTYPE,
		"U", "          ",
		" ", "", "Stock Take Selection", "Input Stock Take Selection Code A-Z.",
		YES, NO,  JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.stk_mode},
	{5, LIN, "mdesc",	 6, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description  ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.mode_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================= 
| Function Declarations |
=======================*/
void run_prog (char *prog_name);
void shutdown_prog (void);
int  spec_valid (int field);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void srch_categ (char *key_val);
void srch_locn (char *key_val);
void update_insc (void);
int  heading (int scn);
void not_valid (void);
void prt_warning (void);
void wline (int y_pos, char *desc);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	int	scn_no;
	char	*sptr;

	/*-------------------------------
	|	parameters					|
	|	1:	program name			|
	|	2:	'G' - by class-cat		|
	|		'I' - by item    		|
	|		'A' - by abc_codes		|
	|		'L' - by location 		|
	|		'B' - by location/item	|
	|	3:	optional PID			|
	-------------------------------*/
	if (argc != 3 && argc != 4) 
	{
		print_at(0,0, mlSkMess636, argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);
	
	sptr = chk_env("MULT_LOC");
	if (sptr != (char *)0)
		mult_loc = atoi(sptr);

	sptr = chk_env("SK_ST_PFRZ");
	if (sptr != (char *)0)
		sk_st_pfrz = atoi(sptr);

	sptr = chk_env("SK_IVAL_CLASS");
	if (sptr)
		inval_cls = strdup(sptr);
	else
		inval_cls = "ZKPN";
	upshift(inval_cls); 

	sprintf(prog_run,"%-.40s",argv[0]);
	sprintf(progname,"%-.40s",argv[1]);
	sprintf(local_rec.by_what,"%-1.1s",argv[2]);

	/*---------------------------------------------------------------------
	| Stock take selection before freezing not valid before serial items. |
	---------------------------------------------------------------------*/
	if ( FRZ_SER )
		sk_st_pfrz = FALSE;

	if (argc == 4)
	{
		pid = atoi(argv[3]);
		FLD( "lpno1" ) = ND;
		FLD( "lpno2" ) = ND;
		FLD( "lpno3" ) = ND;
		FLD( "lpno4" ) = ND;
	}

	switch (local_rec.by_what[0])
	{
	case	'G':
	case	'g':
		scn_no = 1;
		strcpy(local_rec.by_what,"G");
		strcpy(progdesc,"By Product Group");
		break;

	case	'I':
	case	'i':
		scn_no = 2;
		strcpy(local_rec.by_what,"I");
		strcpy(progdesc,"By Item");
		break;

	case	'A':
	case	'a':
		scn_no = 3;
		strcpy(local_rec.by_what,"A");
		strcpy(progdesc,"By ABC Code");
		break;

	case	'L':
	case	'l':
		scn_no = 4;
		strcpy(local_rec.by_what,"L");
		strcpy(progdesc,"By Location");
		if (mult_loc)
		{
			vars[label("st_loc") + 1].required = NA;
			vars[label("en_loc") + 1].required = NA;
		}
		break;

	case	'F':
	case	'f':
		strcpy(progdesc,"(Create Stock Take)");
		scn_no = 5;
		break;

	case	'B':
	case	'b':
		scn_no = 4;
		strcpy(local_rec.by_what,"B");
		strcpy(progdesc,"By Location/Item Number");
		if (mult_loc)
		{
			vars[label("st_loc") + 1].required = NA;
			vars[label("en_loc") + 1].required = NA;
		}
		break;

	default:
		print_at(0,0, mlSkMess670);
		return (EXIT_FAILURE);
	}

	if ( FRZ_NOR || FRZ_SER || FREEZE )
	{
		switch (local_rec.by_what[0])
		{
		case	'G':
			vars[label("stk_mode1") + 1].required = (sk_st_pfrz) ? NA : YES;
			break;
	
		case	'I':
			vars[label("stk_mode2") + 1].required = (sk_st_pfrz) ? NA : YES;
			break;
	
		case	'A':
			vars[label("stk_mode3") + 1].required = (sk_st_pfrz) ? NA : YES;
			break;

		case	'L':
		case	'B':
			vars[label("stk_mode4") + 1].required = (sk_st_pfrz) ? NA : YES;
			break;

		case	'F':
			vars[label("stk_mode5") + 1].required = (sk_st_pfrz) ? YES : NA;
			break;
	
		}
		FLD( "lpno1" ) = ND;
		FLD( "lpno2" ) = ND;
		FLD( "lpno3" ) = ND;
		FLD( "lpno4" ) = ND;
	}

	strcpy(local_rec.lp_str,"1");

	init_scr();
	set_tty();

	if ( !sk_st_pfrz && FREEZE )
	{
		not_valid();

		rset_tty();
		return (EXIT_FAILURE);
	}

	set_masks();
	init_vars(scn_no);

	OpenDB();


	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit	= FALSE;
   	prog_exit	= FALSE;
   	restart		= FALSE;
   	search_ok	= TRUE;

	init_vars(scn_no);
	heading(scn_no);
	entry(scn_no);
    if (restart || prog_exit) {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }

	heading(scn_no);
	scn_display(scn_no);
	edit(scn_no);
	prog_exit = 1;
    if (restart) 
	{
		shutdown_prog();
		return (EXIT_FAILURE);
    }

	update_insc();
	
	if ( !FREEZE )
		run_prog(argv[1]);

    shutdown_prog();
	return (EXIT_SUCCESS);
}

void
run_prog (
 char *prog_name)
{
#ifndef GVISION
	void 	(*old_sigvec) ();
	old_sigvec	=	signal (SIGCLD, SIG_DFL);
#endif	

	programRun = TRUE;
	switch (local_rec.by_what[0])
	{
	case	'G':
		sprintf(local_rec.s_group,"%1.1s%-11.11s",local_rec.s_class,local_rec.s_cat);
		sprintf(local_rec.e_group,"%1.1s%-11.11s",local_rec.e_class,local_rec.e_cat);
		break;

	case	'I':
		sprintf(local_rec.s_group,"%-16.16s",local_rec.s_item);
		sprintf(local_rec.e_group,"%-16.16s",local_rec.e_item);
		break;

	case	'A':
		sprintf(local_rec.s_group,"%-1.1s",local_rec.s_class);
		sprintf(local_rec.e_group,"%-1.1s",local_rec.e_class);
		break;

	case	'L':
	case	'B':
		sprintf(local_rec.s_group,"%-10.10s",local_rec.s_cat);
		sprintf(local_rec.e_group,"%-10.10s",local_rec.e_cat);
		break;
	}

	CloseDB (); 
	FinishProgram ();

#ifdef GVISION
	// wait to subprogram to finish
	gbSysExec = TRUE;

	if (pid)
	{
		sprintf(err_str,"%d",pid);
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.s_group,
			local_rec.e_group,
			local_rec.by_what,
			local_rec.stk_mode,
			err_str,(char *)0);
		/*exit(1);*/
	}
	else if (STAKE_REP)
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.s_group,
			local_rec.e_group,
			local_rec.stk_mode,(char *)0);
		/*exit(1);*/
	}
	else
	{
		execlp(prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.s_group,
			local_rec.e_group,
			local_rec.by_what,
			local_rec.stk_mode,(char *)0);
		/*exit(1);*/
	}
#else
	switch (fork ())
	{
	case -1:
		signal (SIGCLD, old_sigvec);
		return;
	case 0:
		/*
		 *	Child process
		 */
		if (pid)
		{
			sprintf(err_str,"%d",pid);
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.s_group,
				local_rec.e_group,
				local_rec.by_what,
				local_rec.stk_mode,
				err_str,(char *)0);
			/*exit(1);*/
		}
		else if (STAKE_REP)
		{
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.s_group,
				local_rec.e_group,
				local_rec.stk_mode,(char *)0);
			/*exit(1);*/
		}
		else
		{
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.s_group,
				local_rec.e_group,
				local_rec.by_what,
				local_rec.stk_mode,(char *)0);
			/*exit(1);*/
		}
		signal (SIGCLD, old_sigvec);		
		exit (1);

	default:
		/*
		 *	Parent process
		 */
		wait ((int *)0);
	}
#endif
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	if (!programRun)
		CloseDB ();
	
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK("st_cat") )
	{
		if (SRCH_KEY)
		{
			srch_categ(temp_str);
			return(0);
		}
		
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.s_cat);

		if (!dflt_used)
		{
			cc = find_rec("excf",&excf_rec,COMPARISON,"r");
			if (cc) 
			{
				print_mess(ML(mlStdMess004));
				sleep(3);
				clear_mess ();
				return(1); 
			}
		}
		else
		{
			sprintf(local_rec.s_cat,"%-11.11s","           ");
			sprintf(excf_rec.cf_description,"%-40.40s","BEGINNING OF RANGE");
		}

		if (prog_status != ENTRY && strcmp(local_rec.s_cat,local_rec.e_cat) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(5);
			clear_mess ();
			return(1); 
		}
		sprintf(local_rec.s_desc,"%-40.40s",excf_rec.cf_description);
		display_field (field + 1);
		return(0);
	}


	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK("en_cat"))
	{
		if (SRCH_KEY)
		{
			srch_categ(temp_str);
			return(0);
		}
		
		strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
		strcpy(excf_rec.cf_cat_no,local_rec.e_cat);
		
		if (!dflt_used)
		{
			cc = find_rec("excf",&excf_rec,COMPARISON,"r");
			if (cc) 
			{
				print_mess(ML(mlStdMess004));
				sleep(3);
				clear_mess ();
				return(1); 
			}
		}
		else
		{
			sprintf(local_rec.e_cat,"%-11.11s","~~~~~~~~~~~");
			sprintf(excf_rec.cf_description,"%-40.40s","END OF RANGE");
		}
		if (strcmp(local_rec.s_cat,local_rec.e_cat) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(3);
			clear_mess ();
			return(1); 
		}
		strcpy(local_rec.e_desc,excf_rec.cf_description);
		display_field(field + 1);
		return(0);
	}

	/*------------------------
	| Validate Item Number . |
	------------------------*/
	if (LCHECK("st_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.tco_no, local_rec.s_item, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
				strcpy (inmr_rec.mr_item_no, local_rec.s_item);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				print_mess(ML(mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
			SuperSynonymError ();
			if ((result = strstr (inval_cls,inmr_rec.mr_class)))
			{
				sprintf(err_str, ML(mlSkMess211) , inmr_rec.mr_item_no);
				print_mess(err_str);
				clear_mess ();
				return(1);
			}
		}
		else
		{
			sprintf(local_rec.s_item,"%-16.16s","                ");
			sprintf(inmr_rec.mr_description,"%-40.40s","START OF RANGE");
		}

		DSP_FLD("st_item");
		display_field(field + 1);
		if (prog_status != ENTRY && strcmp(local_rec.s_item,local_rec.e_item) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(5);
			clear_mess ();
			return(1); 
		}
		strcpy(local_rec.s_desc,inmr_rec.mr_description);
		display_field(field + 1);
		return(0);
	}

	if (LCHECK("en_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return(0);
		}
		if (!dflt_used)
		{
			cc = FindInmr (comm_rec.tco_no, local_rec.e_item, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
				strcpy (inmr_rec.mr_item_no, local_rec.e_item);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
			if (cc)
			{
				print_mess (ML(mlStdMess001));
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
			SuperSynonymError ();

			if ((result = strstr (inval_cls, inmr_rec.mr_class)))
			{
				sprintf(err_str, ML(mlSkMess211) ,inmr_rec.mr_item_no);
				print_mess(err_str);
				sleep (sleepTime);
				clear_mess ();
				return(1);
			}
		}
		else
		{
			sprintf(local_rec.e_item,"%-16.16s","~~~~~~~~~~~~~~~~");
			sprintf(inmr_rec.mr_description,"%-40.40s","END OF RANGE");
		}

		DSP_FLD("en_item");
		display_field(field + 1);
		if (strcmp(local_rec.s_item,local_rec.e_item) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(3);
			clear_mess ();
			return(1); 
		}
		strcpy(local_rec.e_desc,inmr_rec.mr_description);
		display_field(field + 1);
		return(0);
	}

	if (LCHECK("st_loc"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.s_cat,"%-11.11s","           ");
			sprintf(local_rec.s_desc,"%-40.40s","START LOCATION");
			display_field(field + 1);
			return(0);
		}

		if (mult_loc)
		{
			if (SRCH_KEY)
			{
				srch_locn(temp_str);
				return(0);
			}

			lomr_rec.lo_hhcc_hash = ccmr_rec.cm_hhcc_hash;
			sprintf(lomr_rec.lo_location,"%-10.10s",local_rec.s_cat);
			cc = find_rec("lomr",&lomr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess(ML(mlStdMess209));
				sleep (sleepTime);
				clear_mess ();
				return(1); 
			}
			strcpy(local_rec.s_desc,lomr_rec.lo_desc);
			display_field(field + 1);
		}
		if (prog_status != ENTRY && strcmp(local_rec.s_cat,local_rec.e_cat) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(5);
			clear_mess ();
			return(1); 
		}
		return(0);
	}

	if (LCHECK("en_loc") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.e_cat,"%-11.11s","~~~~~~~~~~~");
			sprintf(local_rec.e_desc,"%-40.40s","END LOCATION");
			display_field(field + 1);
			return(0);
		}
		if (mult_loc)
		{
			if (SRCH_KEY)
			{
				srch_locn(temp_str);
				return(0);
			}

			lomr_rec.lo_hhcc_hash = ccmr_rec.cm_hhcc_hash;
			sprintf(lomr_rec.lo_location,"%-10.10s",local_rec.e_cat);
			cc = find_rec("lomr",&lomr_rec,COMPARISON,"r");
			if (cc)
			{
				print_mess(ML(mlStdMess209));
				sleep (sleepTime);
				clear_mess ();
				return(1); 
			}
			strcpy(local_rec.e_desc,lomr_rec.lo_desc);
			display_field(field + 1);
		}
		if (strcmp(local_rec.s_cat,local_rec.e_cat) > 0 )
		{
			print_mess(ML(mlStdMess006));
			sleep(3);
			clear_mess ();
			return(1); 
		}
		return(0);
	}

	if (strncmp(FIELD.label,"stk_mode",8) == 0) 
	{
		insc_rec.sc_hhcc_hash = ccmr_rec.cm_hhcc_hash;
		strcpy(insc_rec.sc_stake_code,local_rec.stk_mode);
		cc = find_rec("insc",&insc_rec,COMPARISON,"r");
		if ( !cc )
		{
			if ( FREEZE || (( FRZ_NOR || FRZ_SER ) && !sk_st_pfrz))
			{
				sprintf(err_str, ML(mlSkMess370) ,ccmr_rec.cm_cc_no);
				print_mess(err_str);
				sleep(3);
				clear_mess ();
				return(1);
			}
			else
			{
				strcpy(local_rec.mode_desc,insc_rec.sc_description);
				display_field(field + 1);
			}
		}
		else
		{
			if ( FREEZE || (( FRZ_NOR || FRZ_SER ) && !sk_st_pfrz))
				new_insc = TRUE;
			else
			{
				sprintf(err_str, ML(mlSkMess047) ,local_rec.stk_mode);
				print_mess(err_str);
				sleep(3);
				clear_mess ();
				return(1);
			}
		}
		return(0);
	}

	if ( (strncmp(FIELD.label,"lpno",4) == 0) && (FLD(FIELD.label) != ND) ) 
	{
		if (!local_rec.lpno)
			local_rec.lpno = 1;
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		sprintf(local_rec.lp_str,"%2d",local_rec.lpno);
		return(0);
	}
	return(0);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	ReadMisc();

	open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec("insc",insc_list,insc_no_fields,"insc_id_no");
	if (mult_loc)
		open_rec("lomr",lomr_list,lomr_no_fields,"lomr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose("excf");
	abc_fclose("inmr");
	abc_fclose("insc");
	if (mult_loc)
		abc_fclose("lomr");
	SearchFindClose ();
	abc_dbclose("data");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");
	strcpy(ccmr_rec.cm_co_no,comm_rec.tco_no);
	strcpy(ccmr_rec.cm_est_no,comm_rec.tes_no);
	strcpy(ccmr_rec.cm_cc_no,comm_rec.tcc_no);
	cc = find_rec("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err( cc, "ccmr", "DBFIND" );

	abc_fclose("ccmr");
}

/*==================================
| Search for Category master file. |
==================================*/
void
srch_categ (
 char *key_val)
{
	work_open();
	save_rec("#Cat Code","#Description");
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no ,"%-11.11s",key_val);
	cc = find_rec("excf",&excf_rec,GTEQ,"r");
	while (!cc && !strncmp(excf_rec.cf_cat_no,key_val,strlen(key_val)) && 
			!strcmp(excf_rec.cf_co_no,comm_rec.tco_no))
	{
		cc = save_rec(excf_rec.cf_cat_no,excf_rec.cf_description);
		if (cc)
			break;

		cc = find_rec("excf",&excf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excf_rec.cf_co_no,comm_rec.tco_no);
	sprintf(excf_rec.cf_cat_no,"%-11.11s",temp_str);
	cc = find_rec("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		file_err( cc, "excf", "DBFIND" );
}

void
srch_locn (
 char *key_val)
{
	work_open();
	save_rec("#Locn Code","#Description");
	lomr_rec.lo_hhcc_hash = ccmr_rec.cm_hhcc_hash;
	sprintf(lomr_rec.lo_location ,"%-10.10s",key_val);
	cc = find_rec("lomr",&lomr_rec,GTEQ,"r");
	while (!cc && !strncmp(lomr_rec.lo_location,key_val,strlen(key_val)) && 
		       lomr_rec.lo_hhcc_hash == ccmr_rec.cm_hhcc_hash)
	{
		cc = save_rec(lomr_rec.lo_location,lomr_rec.lo_desc);
		if (cc)
			break;

		cc = find_rec("lomr",&lomr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	lomr_rec.lo_hhcc_hash = ccmr_rec.cm_hhcc_hash;
	sprintf(lomr_rec.lo_location ,"%-10.10s",temp_str);
	cc = find_rec("lomr",&lomr_rec,COMPARISON,"r");
	if (cc)
		file_err( cc, "lomr", "DBFIND" );
}

void
update_insc (
 void)
{
	if (new_insc)
	{
		insc_rec.sc_hhcc_hash = ccmr_rec.cm_hhcc_hash;
		strcpy(insc_rec.sc_stake_code,local_rec.stk_mode);
		insc_rec.sc_start_date = TodaysDate (); 
		sprintf (insc_rec.sc_start_time, "%-5.5s", TimeHHMM ());
		strcpy(insc_rec.sc_description,local_rec.mode_desc);
		insc_rec.sc_frz_date = (FREEZE) ? 0L : insc_rec.sc_start_date;
		strcpy(insc_rec.sc_frz_time, (FREEZE) ? "00:00" 
						      : insc_rec.sc_start_time);
		strcpy(insc_rec.sc_serial_take, (FRZ_SER) ? "Y" : "N");
		
		cc = abc_add("insc",&insc_rec);
		if (cc)
			file_err( cc, "insc", "DBADD" );

		return;
	}

	if ( FREEZE )
		return;
		
	insc_rec.sc_hhcc_hash = ccmr_rec.cm_hhcc_hash;
	strcpy(insc_rec.sc_stake_code,local_rec.stk_mode);
	cc = find_rec("insc", &insc_rec, COMPARISON, "u");
	if ( cc )
	{
		abc_unlock( "insc" );
		return;
	}

	if ( insc_rec.sc_frz_date == 0L )
	{
		insc_rec.sc_frz_date = TodaysDate (); 
		sprintf(insc_rec.sc_frz_time,"%s", TimeHHMM());
		cc = abc_update("insc",&insc_rec);
		if (cc)
			file_err( cc, "insc", "DBUPDATE" );
	}
	else
		abc_unlock( "insc" );

	return;
}

/*=================================================================
| Heading concerns itself with clearing the screen,painting the   |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		sprintf(err_str, ML(mlSkMess371) ,progdesc);

		rv_pr(err_str,(80 - strlen(err_str)) / 2,0,1);
		move(0,1);
		line(80);

		switch (scn)
		{
		case	1:
			move(1,8);
			line(79);
			move(1,12);
			line(79);
			box(0,4,80,10);
			break;

		case	2:
			move(1,7);
			line(79);
			move(1,10);
			line(79);
			box(0,4,80,8);
			break;

		case	3:
			move(1,7);
			line(79);
			box(0,4,80,5);
			break;

		case	4:
			move(1,7);
			line(79);
			move(1,10);
			line(79);
			box(0,4,80,8);
			break;

		case	5:
			box(0,4,80,2);
			prt_warning();
			break;
		}

		move(0,20);
		line(80);

/*		print_at(21, 0, "Co. : %s  %s    Br. : %s  %s    Wh. : %s  %s",comm_rec.tco_no,clip(comm_rec.tco_short),comm_rec.tes_no,clip(comm_rec.tes_short),comm_rec.tcc_no,comm_rec.tcc_short);
*/
		print_at(21, 0, ML(mlStdMess038),
				comm_rec.tco_no,
				clip(comm_rec.tco_short));

		print_at(21, 20, ML(mlStdMess039),
				comm_rec.tes_no,
				clip(comm_rec.tes_short));

		print_at(21, 50, ML(mlStdMess099),
				comm_rec.tcc_no,
				comm_rec.tcc_short);

		move(0,22);
		line(80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}

void
not_valid (
 void)
{
	clear();
/*
	wline(0,"As your system is set up to create stock take selection");
	wline(1,"at the time of freezing stock this option is Invalid for you.");
	wline(3,"However if this should not be the case you will need to change");
	wline(4,"the environment Variable SK_ST_PFRZ .");
	wline(6,"            Key < return > to continue.");
*/
	wline(0, ML(mlSkMess372));
	wline(1, ML(mlSkMess373));
	wline(3, ML(mlSkMess374));
	wline(4, ML(mlSkMess375));
	wline(6, ML(mlSkMess376));
    PauseForKey (0, 0, "", 0);
}

void
prt_warning (
 void)
{
   	if ( !sk_st_pfrz )
		return;

	box(8,7,64,12);
/*
	wline(8 ,"Your system is set to allow stock take selection to be");
	wline(9 ,"created before stock is frozen. The only exception to");
	wline(10 ,"this is serial items which must be frozen before any options");
	wline(11 ,"are used. For additional details refer to your Manual.   ");
	wline(13,"Stock take options you can run before stock is Frozen :");
	wline(14,"  a) Stock take Sheet print. ( all options )");
	wline(15,"  b) Stock take Count input. ( all options )");
	wline(16,"The following options Cannot be run until stock is frozen :");
	wline(17,"  a) Stock Take Valuation reports.");
	wline(18,"  b) Stock Take Variation reports.");
	wline(19,"  c) Stock take Update.");
*/
	wline(8 , ML(mlSkMess377));
	wline(9 , ML(mlSkMess378));
	wline(10 , ML(mlSkMess379));
	wline(11 , ML(mlSkMess380));
	wline(13 , ML(mlSkMess381));
	wline(14 , ML(mlSkMess382));
	wline(15 , ML(mlSkMess383));
	wline(16 , ML(mlSkMess384));
	wline(17 , ML(mlSkMess385));
	wline(18 , ML(mlSkMess386));
	wline(19 , ML(mlSkMess387));

}

void
wline (
 int y_pos, 
 char *desc)
{
/*	move( 10, y_pos );*/

	print_at(y_pos, 10, desc);

	fflush(stdout);
}
	
