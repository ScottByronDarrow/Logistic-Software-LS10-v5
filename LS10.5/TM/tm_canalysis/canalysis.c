/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: canalysis.c,v 5.3 2001/11/14 03:34:01 scott Exp $
|  Program Name  : (tm_canalysis.c) 
|  Program Desc  : (Telemarketing Campaign Analysis)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 06/08/91         |
|---------------------------------------------------------------------|
| $Log: canalysis.c,v $
| Revision 5.3  2001/11/14 03:34:01  scott
| Updated to use app.schema
| Updated to clean code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: canalysis.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TM/tm_canalysis/canalysis.c,v 5.3 2001/11/14 03:34:01 scott Exp $";

#define	X_OFF	0
#define	Y_OFF	3
#include	<pslscr.h>		
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_tm_mess.h>

#define	ALL_CAMPS 	 (!strcmp (local_rec.campgn, "ALL ") || \
					 !strcmp (local_rec.campgn, " ALL"))

FILE	*fsort;
char	*sptr;

#include	"schema"

struct commRecord	comm_rec;
struct tmopRecord	tmop_rec;
struct tmcfRecord	tmcf_rec;
struct tmchRecord	tmch_rec;

int		noInCalls [2];
int		noOutCalls [2];
double	noInSales [2];
double	noOutSales [2];

extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy [11];
	char	campgn [5];
	char	campgn_name [61];
	char	st_op [15];
	char	st_op_name [41];
	char	end_op [15];
	char	end_op_name [41];
	int	lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "campgn", 4, 2, CHARTYPE, 
		"NNNN", "          ", 
		" ", "", "Campaign Number  ", " Enter Campaign. Default for ALL ", 
		NO, NO, JUSTRIGHT, "0123456789", "", local_rec.campgn}, 
	{1, LIN, "campgn_name", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Campaign Desc.   ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.campgn_name}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

void 	OpenDB 		(void);
void 	CloseDB 	(void);
void 	SrchTmcf 	(char *);
void 	Process 	(void);
void 	CalcStats 	(void);
int 	spec_valid 	(int);
int 	heading 	(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
	int argc, 
	char *argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/

	OpenDB ();

	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		search_ok 	= TRUE;
		entry_exit 	= TRUE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);	

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (!restart) 
			Process ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);   
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (tmcf, tmcf_list, TMCF_NO_FIELDS, "tmcf_id_no");
	open_rec (tmch, tmch_list, TMCH_NO_FIELDS, "tmch_hhcf_hash");
	open_rec (tmop, tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
}

void
CloseDB (void)
{
	abc_fclose (tmcf);
	abc_fclose (tmch);
	abc_fclose (tmop);
	abc_dbclose ("data");
}

int
spec_valid (
	int field)
{
	/*--------------------------
	| Validate Campaign Number |
	--------------------------*/
	if (LCHECK ("campgn"))
	{
		if (SRCH_KEY)
		{
			SrchTmcf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.campgn, "ALL ");
			strcpy (err_str, ML ("ALL CAMPAIGNS"));
			DSP_FLD ("campgn");
			DSP_FLD ("campgn_name");
			return (EXIT_SUCCESS);
		}

		strcpy (tmcf_rec.co_no, comm_rec.co_no);
		tmcf_rec.campaign_no = atoi (local_rec.campgn);
		cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTmMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (err_str, "%s, ", clip (tmcf_rec.c_name1));
		strcat (err_str, clip (tmcf_rec.c_name2));

		sprintf (local_rec.campgn_name, "%-60.60s", err_str);
		DSP_FLD ("campgn_name");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*==========================================
| Search routine for Campaign Header File. |
==========================================*/
void
SrchTmcf (
	char *key_val)
{
	_work_open (4,0,40);
	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (key_val);
	save_rec ("#No.", "#Campaign Description.");
	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
		tmcf_rec.campaign_no >= atoi (key_val))
	{
		sprintf (err_str, "%4d", tmcf_rec.campaign_no);
		cc = save_rec (err_str, tmcf_rec.c_name1);
		if (cc)
			break;

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	tmcf_rec.campaign_no = atoi (temp_str);
	cc = find_rec (tmcf, &tmcf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, tmcf, "DBFIND");
}

void
Process (
	void)
{
	char	tmp_date [11];
	int	first_time = TRUE;

	strcpy (tmcf_rec.co_no, comm_rec.co_no);
	if (ALL_CAMPS)
		tmcf_rec.campaign_no = 0;
	else
		tmcf_rec.campaign_no = atoi (local_rec.campgn);

	cc = find_rec (tmcf, &tmcf_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (tmcf_rec.co_no, comm_rec.co_no) &&
	      (ALL_CAMPS || (!ALL_CAMPS && 
		tmcf_rec.campaign_no == atoi (local_rec.campgn))))
	{
		if (first_time)
		{
			/*-----------------
			| Open Dsp window |
			-----------------*/
			Dsp_prn_open (0, 2, 16, 
		     		"                        Telemarketing Campaign Analysis                        ", (char *) 0, 
	             		 (char *) 0, (char *) 0, (char *) 0, (char *) 0, (char *) 0);
		
			Dsp_saverec ("  Operator   |Operator Name           |Calls|Cmpgn Tme|Avg Tme/Call| Sales    ");
			Dsp_saverec ("");
			Dsp_saverec (" [PRINT] [NEXT] [PREV] [EDIT/END]  ");
			first_time = FALSE;
		}

		sprintf (err_str, 
			"^1 Campaign Number : %4d  Name : %-40.40s ^6", 
			tmcf_rec.campaign_no, 
			tmcf_rec.c_name1);
		Dsp_saverec (err_str);

		sprintf (err_str, 
			"                             ^1 : %-40.40s ^6", tmcf_rec.c_name2);
		Dsp_saverec (err_str);

		sprintf (tmp_date, "%-10.10s", DateToString (tmcf_rec.start_date));
		sprintf (err_str, 
			" Campaign Manager: %-16.16s Strt Date: %-10.10s End Date: %-10.10s", 
			tmcf_rec.c_manager, 
			tmp_date, 
			DateToString (tmcf_rec.end_date));
		Dsp_saverec (err_str);

		sprintf (err_str, " Campaign Objectives : %-50.50s ", tmcf_rec.c_obj1);
		Dsp_saverec (err_str);
		sprintf (err_str, "                     : %-50.50s ", tmcf_rec.c_obj2);
		Dsp_saverec (err_str);
		sprintf (err_str, "                     : %-50.50s ", tmcf_rec.c_obj3);
		Dsp_saverec (err_str);
		sprintf (err_str, "                     : %-50.50s ", tmcf_rec.c_obj4);
		Dsp_saverec (err_str);
		sprintf (err_str, "                     : %-50.50s ", tmcf_rec.c_obj5);
		Dsp_saverec (err_str);

		CalcStats ();

		Dsp_saverec (" ");

		sprintf (err_str, 
			" Budget IN Calls   : %5d         Actual IN Calls   : %5d ", 
			tmcf_rec.bg_incalls, 
			noInCalls [0]);
		Dsp_saverec (err_str);
		sprintf (err_str, 
			" Budget OUT Calls  : %5d         Actual OUT Calls  : %5d ", 
			tmcf_rec.bg_outcalls, 
			noOutCalls [0]);
		Dsp_saverec (err_str);

		Dsp_saverec (" ");

		sprintf (err_str, 
			" Budget IN Sales   : %7.2f       Actual IN Sales   : %7.2f ", 
			tmcf_rec.bg_insales, 
			noInSales [0]);
		Dsp_saverec (err_str);
		sprintf (err_str, 
			" Budget OUT Sales  : %7.2f       Actual OUT Sales  : %7.2f ", 
			tmcf_rec.bg_outsales, 
			noOutSales [0]);
		Dsp_saverec (err_str);

		Dsp_saverec (" ");

		sprintf (err_str, 
			" Budget Sales Value: %7.2f       Actual Sales Value: %7.2f ", 
			0.00, 
			0.00);
		Dsp_saverec (err_str);

		cc = find_rec (tmcf, &tmcf_rec, NEXT, "r");
	}

	Dsp_srch ();
	Dsp_close ();

	return;
}

void
CalcStats (
	void)
{

	noInCalls [0] = 0;
	noOutCalls [0] = 0;
	noInSales [0] = 0.00;
	noOutSales [0] = 0.00;

	cc = find_hash (tmch, &tmch_rec, GTEQ, "r", tmcf_rec.hhcf_hash);
	while (!cc && tmch_rec.hhcf_hash == tmcf_rec.hhcf_hash)
	{
		if (tmch_rec.io_flag [0] == 'I')
			noInCalls [0]++;
		else
			noOutCalls [0]++;

		if (tmch_rec.io_flag [0] == 'I')
			noInSales [0] += tmch_rec.ord_value;
		else
			noOutSales [0] += tmch_rec.ord_value;

		cc = find_hash (tmch, &tmch_rec, NEXT, "r", tmcf_rec.hhcf_hash);
	}

	noInCalls [1]  += noInCalls [0];
	noOutCalls [1] += noOutCalls [0];
	noInSales [1]  += noInSales [0];
	noOutSales [1] += noOutSales [0];

	return;
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
	int scn)
{
	char	hdng_date [11];

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
	
		strcpy (hdng_date, DateToString (TodaysDate ()));
		rv_pr (hdng_date, 69, 0, 0);

		line_at (1,0,80);

		rv_pr (ML (mlTmMess066), 20, 0, 1);

		box (0, 3, 80, 2);

		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
