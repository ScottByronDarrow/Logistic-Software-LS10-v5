/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_wodisp.c   )                                  |
|  Program Desc  : ( Display/Print Work Orders Statuses.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pcwo, ccmr, inmr, inei,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 31/08/93         |
|---------------------------------------------------------------------|
|  Date Modified : (12/10/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (15/10/93)        Modified By   : Aroha Merrilees. |
|  Date Modified : (24/01/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (17/03/94)        Modified By   : Aroha Merrilees. |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Leah Manibog.     |
|                                                                     |
|  Comments      : (12/10/93) DPL 9671 - increased qtys to 4 decimal  |
|                : places & display in order of w/o number if for all |
|                : status.                                            |
|  (15/10/93)    : DPL 9671 - changing print format for required,     |
|                : received, rejected qty's & batch size to %14.6f    |
|                : to cater for 9999999.999999                        |
|  (24/01/94)    : DPL 9673 - added pcwo_wh_no, for pcwo_id_no index. |
|  (15/03/94)    : DPL 10617 - default level to branch, not crash out |
|                : if inmr or inei record not found.                  |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (03/09/97)    : Updated for Multilingual Conversion.               |
|                :                                                    |
|                                                                     |
| $Log: pc_wodisp.c,v $
| Revision 5.4  2002/07/17 09:57:31  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:14:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:35:21  scott
| RELEASE 5.0
|
| Revision 5.1  2001/06/23 14:28:02  cha
| Updated to fix prompt being overridden
|
| Revision 5.0  2001/06/19 08:10:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:22  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.13  2000/03/07 09:46:34  ramon
| For GVision compatibility, I moved the description fields 3 chars to the right.
|
| Revision 1.12  1999/11/12 10:37:49  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.11  1999/09/29 10:11:43  scott
| Updated to be consistant on function names.
|
| Revision 1.10  1999/09/17 08:26:28  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.9  1999/09/13 07:03:22  marlene
| *** empty log message ***
|
| Revision 1.8  1999/09/09 06:12:39  marlene
| *** empty log message ***
|
| Revision 1.7  1999/06/17 07:40:52  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_wodisp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_wodisp/pc_wodisp.c,v 5.4 2002/07/17 09:57:31 scott Exp $";

#define	X_OFF	2
#define	Y_OFF	4

#include <pslscr.h>
#include <get_lpno.h>
#include <pr_format3.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>

#ifdef PSIZE
#undef PSIZE
#endif
#define PSIZE	12

#define COMPANY		( local_rec.cobrwh[0] == 'C' )
#define BRANCH		( local_rec.cobrwh[0] == 'B' )
#define WAREHOUSE	( local_rec.cobrwh[0] == 'W' )

#define PRINTER		( local_rec.dpflag[0] == 'P' )

#define ALL_STAT	( local_rec.wkstat[0] == '*' )

char	*g_line = "^^GGJGGJGGGGGGGGGJGGGGGGGGGGGGJGJGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGG"; 

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
	};

	int	comm_no_fields = 7;

	struct tag_commRecord
	{
		int		term;
		char	co_no [3];
		char	co_name [41];
		char	est_no [3];
		char	est_name [41];
		char	cc_no [3];
		char	cc_name [41];
	} comm_rec;

	/*=====================================
	| Production Control Works Order File |
	=====================================*/
	struct dbview pcwo_list [] =
	{
		{"pcwo_co_no"},
		{"pcwo_br_no"},
		{"pcwo_wh_no"},
		{"pcwo_order_no"},
		{"pcwo_reqd_date"},
		{"pcwo_priority"},
		{"pcwo_hhbr_hash"},
		{"pcwo_hhcc_hash"},
		{"pcwo_prod_qty"},
		{"pcwo_act_prod_qty"},
		{"pcwo_act_rej_qty"},
		{"pcwo_order_status"},
		{"pcwo_batch_no"},
	};

	int	pcwo_no_fields = 13;

	struct tag_pcwoRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	order_no [8];
		long	reqd_date;
		int		priority;
		long	hhbr_hash;
		long	hhcc_hash;
		float	prod_qty;
		float	act_prod_qty;
		float	act_rej_qty;
		char	order_status [2];
		char	batch_no [10];
	} pcwo_rec;


	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list [] =
	{
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
	};

	int	ccmr_no_fields = 4;

	struct tag_ccmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	cc_no [3];
		long	hhcc_hash;
	} ccmr_rec;

	/*===================================
	| Inventory Master File Base Record |
	===================================*/
	struct dbview inmr_list [] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_dec_pt"},
	};

	int	inmr_no_fields = 4;

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		int		dec_pt;
	} inmr_rec;


	/*===========================================
	| Inventory Establishment/Branch Stock File |
	===========================================*/
	struct dbview inei_list [] =
	{
		{"inei_hhbr_hash"},
		{"inei_est_no"},
		{"inei_std_batch"},
	};

	int	inei_no_fields = 3;

	struct tag_ineiRecord
	{
		long	hhbr_hash;
		char	est_no [2];
		float	std_batch;
	} inei_rec;

	char	*data = "data",
			*comm = "comm",
			*pcwo = "pcwo",
			*ccmr = "ccmr",
			*inmr = "inmr",
			*inei = "inei";
 
	FILE	*fin,
			*fout;

	int	nothing_printed = TRUE;

	char	tempdate[11];


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	long	st_date;
	long	ed_date;
	char 	wkstat [2];
	char 	disp_wkstat [22];
	char	cobrwh [2];
	char	disp_cobrwh [22];
	char 	dpflag [2];
	char 	disp_dpflag [22];
	int		lp_no;
	char 	dummy [11];
	char	systemDate [11];
} local_rec;

static struct	var vars[] =
{
	{1, LIN, "startdate", 4, 30, EDATETYPE, 
		"DD/DD/DD", "           ", 
		" ", "00/00/00", " Start Required Date         : ", "Default to 00/00/00", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.st_date}, 
	{1, LIN, "enddate", 5, 30, EDATETYPE, 
		"DD/DD/DD", "           ", 
		" ", local_rec.systemDate, " End Required Date           : ", "Default to today's date ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ed_date}, 
	{1, LIN, "work_stat", 7, 30, CHARTYPE,  
		"U", "          ", 
		" ", "*", " Work Order Status           : ", "W/O Status : *(all,  P(lanned,  F(irmed,  I(ssuing,  A(llocated,  R(eleased,  C(losing,  Z(-Closed,  D(eleted - Default *(all", 
		YES, NO, JUSTLEFT, "PFIARCZD*", "", local_rec.wkstat}, 
	{1, LIN, "disp_work_stat", 7, 35, CHARTYPE,  
		"AAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.disp_wkstat}, 
	{1, LIN, "cobrwh", 9, 30, CHARTYPE, 
		"U", "          ", 
		" ", "B", " C(ompany/B(ranch/W(arehouse : ",  "C(ompnay) B(ranch) W(arehouse) - Default B(ranch", 
		YES, NO, JUSTLEFT, "BCW", "", local_rec.cobrwh}, 
	{1, LIN, "disp_cobrwh", 9, 35, CHARTYPE, 
		"AAAAAAAAAAAA", "          ", 
		" ", " ", "",  " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.disp_cobrwh}, 
	{1, LIN, "dpflag", 11, 30, CHARTYPE, 
		"U", "          ", 
		" ", "D", " D(isplay or P(rint          : ", "Default D(isplay", 
		YES, NO, JUSTLEFT, "DP", "", local_rec.dpflag}, 
	{1, LIN, "disp_dpflag", 11, 35, CHARTYPE, 
		"AAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.disp_dpflag}, 
	{1, LIN, "lp_no", 12, 30, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer No                  : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lp_no}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          "
		, " ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};


/*======================
| function prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int ProcPcwo (void);
int ReadCcmr (void);
void ReadInmr (void);
void ReadInei (void);
int ProcLog (void);
int heading (int scn);
void InitOutput (void);
void PrintHeading (void);
void DisplayHeading (void);			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv[])
{
	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if ((fin = pr_open ("pc_wodisp.p")) == NULL)
		sys_err ("Error in pc_wodisp.p During (FOPEN)", cc, PNAME);

	while (prog_exit == 0)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);
		crsr_on ();

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		/*============================
		| Process Orders in Database.|
		============================*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcPcwo ();

		if (PRINTER)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	fclose (fin);
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (pcwo, pcwo_list, pcwo_no_fields, "pcwo_id_no2");
	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_hhcc_hash");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (inei, inei_list, inei_no_fields, "inei_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (pcwo);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startdate") || LCHECK ("enddate")) 
	{
		if (LCHECK ("startdate") && dflt_used)
			return (EXIT_SUCCESS);

		if ((LCHECK ("enddate") || prog_status != ENTRY) &&
			local_rec.st_date > local_rec.ed_date)
		{
			errmess (ML(mlStdMess019));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("work_stat"))
	{
		strcpy (local_rec.disp_wkstat, "         ");
		DSP_FLD ("disp_work_stat");
		switch (local_rec.wkstat[0])
		{
		case	'P':
			strcpy (local_rec.disp_wkstat, "Planned  ");
			break;
		case	'F':
			strcpy (local_rec.disp_wkstat, "Firmed   ");
			break;
		case	'I':
			strcpy (local_rec.disp_wkstat, "Issuing  ");
			break;
		case	'A':
			strcpy (local_rec.disp_wkstat, "Allocated");
			break;
		case	'R':
			strcpy (local_rec.disp_wkstat, "Released ");
			break;
		case	'C':
			strcpy (local_rec.disp_wkstat, "Closing  ");
			break;
		case	'Z':
			strcpy (local_rec.disp_wkstat, "Z-Closed ");
			break;
		case	'D':
			strcpy (local_rec.disp_wkstat, "Deleted  ");
			break;
		case	'*':
			strcpy (local_rec.disp_wkstat, "*-ALL    ");
			break;
		}
		DSP_FLD ("disp_work_stat");
		return (EXIT_SUCCESS);
	}

	if (LCHECK("cobrwh"))
	{
		strcpy (local_rec.disp_cobrwh, "         ");
		DSP_FLD ("disp_cobrwh");
		switch (local_rec.cobrwh[0])
		{
		case	'C':
			strcpy (local_rec.disp_cobrwh, "Company  ");
			break;
		case	'B':
			strcpy (local_rec.disp_cobrwh, "Branch   ");
			break;
		case	'W':
			strcpy (local_rec.disp_cobrwh, "Warehouse");
			break;
		}
		DSP_FLD ("disp_cobrwh");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dpflag"))
	{
		strcpy (local_rec.disp_dpflag, "       ");
		DSP_FLD ( "disp_dpflag" );
		switch (local_rec.dpflag[0])
		{
		case	'D':
			strcpy (local_rec.disp_dpflag, "Display");
			FLD ("lp_no") = NA;
			local_rec.lp_no = 0;
			DSP_FLD ("lp_no");
			break;

		case	'P':
			strcpy (local_rec.disp_dpflag, "Print  ");
			FLD ("lp_no") = YES;
			if (prog_status != ENTRY)
			{
				/*print_mess ("Please ensure a printer number has been entered.");*/
				print_mess (ML(mlPcMess052));
				sleep (sleepTime);
			}
			break;
		}
		DSP_FLD ("disp_dpflag");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lp_no"))
	{
		if (FLD ("lp_no") != NA) 
		{
			if (last_char == SEARCH)
			{
				local_rec.lp_no = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lp_no))
			{
				/*print_mess ("Invalid Printer");*/
				print_mess (ML(mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ProcPcwo (
 void)
{
	nothing_printed = TRUE;

	if (!PRINTER)
	{
		Dsp_nc_prn_open (0, 5, PSIZE, err_str, 
			comm_rec.co_no, comm_rec.co_name, 
			comm_rec.est_no, comm_rec.est_name, 
			(char *)0,  (char *)0);

		Dsp_saverec ("BR|WH|WORKS ORD|   BATCH    |S|  REQUIRED  |     I T E M      |    REQ 'D     |    REC 'D     |    REJ 'D     |    BATCH      ");
		Dsp_saverec ("NO|NO|   NO    |     NO     |T|    DATE    |        NO        |     QTY       |     QTY       |     QTY       |     SIZE      ");
		Dsp_saverec (" [Redraw] [Print] [Next] [Prev] [End/Input] ");
	}
	else
		PrintHeading ();

	strcpy (pcwo_rec.co_no,  comm_rec.co_no);
	if (BRANCH || WAREHOUSE)
	{
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		if (WAREHOUSE)
			strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		else
			strcpy (pcwo_rec.wh_no, " ");
	}
	else
	{
		strcpy (pcwo_rec.br_no,  " ");
		strcpy (pcwo_rec.wh_no, " ");
	}
	if (!ALL_STAT)
	{
		strcpy (pcwo_rec.order_status,  local_rec.wkstat);
		pcwo_rec.reqd_date = 0L;
		pcwo_rec.priority = 0;
		abc_selfield (pcwo, "pcwo_id_no2");
	}
	else
	{
		strcpy (pcwo_rec.order_no, " ");
		abc_selfield (pcwo, "pcwo_id_no");
	}

	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");

	while (!cc)
	{
		if (local_rec.st_date <= pcwo_rec.reqd_date && 
			pcwo_rec.reqd_date <= local_rec.ed_date && 
			(ALL_STAT || (!ALL_STAT && 
			!strncmp (pcwo_rec.order_status, local_rec.wkstat, 1))))
		{
			if (!ReadCcmr ())
			{
				if (nothing_printed)
					nothing_printed = FALSE;

				ReadInmr ();
				ReadInei ();

				/*----------------------------
				| print to screen or printer |
				----------------------------*/
				ProcLog ();
			}
		}

		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	if (!nothing_printed && !PRINTER)
		Dsp_saverec (g_line);

	if (!PRINTER)
	{
		Dsp_srch ();
		Dsp_close ();
	}
	return (EXIT_SUCCESS);
}

int
ReadCcmr (
 void)
{
	ccmr_rec.hhcc_hash = pcwo_rec.hhcc_hash;

	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DB_FIND");

	if (COMPANY && (!strncmp (ccmr_rec.co_no, comm_rec.co_no ,2)))
		return (EXIT_SUCCESS);

	if (BRANCH && (!strncmp (ccmr_rec.co_no, comm_rec.co_no ,2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.est_no ,2)))
		return (EXIT_SUCCESS);

	if (WAREHOUSE && (!strncmp (ccmr_rec.co_no, comm_rec.co_no ,2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.est_no ,2) &&
		!strncmp (ccmr_rec.cc_no, comm_rec.cc_no ,2)))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void
ReadInmr (
 void)
{
	inmr_rec.hhbr_hash = pcwo_rec.hhbr_hash;

	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");

	if (cc)
	{
		sprintf (inmr_rec.item_no, "%-16.16s", "Item No");
		sprintf (inmr_rec.description,
				"No item found for this hash [%ld]",
				pcwo_rec.hhbr_hash);
		inmr_rec.dec_pt = 4;
	}

	if (inmr_rec.dec_pt > 4)
		dec_pt = 4;
	else
		dec_pt = inmr_rec.dec_pt;
}

void
ReadInei (
 void)
{
	inei_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	strcpy (inei_rec.est_no, comm_rec.est_no);

	cc = find_rec (inei, &inei_rec, COMPARISON, "r");

	if (cc)
		inei_rec.std_batch = 0.00;
}

int
ProcLog (
 void)
{
	char	disp_str [200];

	if (PRINTER)
	{
		dsp_process ("Item No :", inmr_rec.item_no);

		pr_format (fin, fout, "WO_LINE", 1, pcwo_rec.br_no);
		pr_format (fin, fout, "WO_LINE", 2, pcwo_rec.wh_no);
		pr_format (fin, fout, "WO_LINE", 3, pcwo_rec.order_no);
		pr_format (fin, fout, "WO_LINE", 4, pcwo_rec.batch_no);
		pr_format (fin, fout, "WO_LINE", 5, pcwo_rec.order_status);
		pr_format (fin, fout, "WO_LINE", 6, DateToString(pcwo_rec.reqd_date));
		pr_format (fin, fout, "WO_LINE", 7, inmr_rec.item_no);
		pr_format (fin, fout, "WO_LINE", 8, inmr_rec.description);
		pr_format (fin, fout, "WO_LINE", 9, n_dec (pcwo_rec.prod_qty, dec_pt));
		pr_format (fin, fout, "WO_LINE", 10,n_dec (pcwo_rec.act_prod_qty, dec_pt));
		pr_format (fin, fout, "WO_LINE", 11, n_dec (pcwo_rec.act_rej_qty, dec_pt));
		pr_format (fin, fout, "WO_LINE", 12, twodec (inei_rec.std_batch));
	}
	else
	{
		sprintf (disp_str, "%2.2s|%2.2s| %7.7s | %10.10s |%1.1s| %10.10s | %16.16s |%14.6f |%14.6f |%14.6f |%14.6f ",
			pcwo_rec.br_no,
			pcwo_rec.wh_no,
			pcwo_rec.order_no,
			pcwo_rec.batch_no,
			pcwo_rec.order_status,
			DateToString (pcwo_rec.reqd_date),
			inmr_rec.item_no,
			n_dec (pcwo_rec.prod_qty, dec_pt),
			n_dec (pcwo_rec.act_prod_qty, dec_pt),
			n_dec (pcwo_rec.act_rej_qty, dec_pt),
			twodec (inei_rec.std_batch));

		Dsp_saverec (disp_str);
	}
	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();
		
		rv_pr (ML(mlPcMess056), 42, 0, 1);

		box (0, 3, 132, 9);
		line_at (1,0,132);
		line_at (6,1,131);
		line_at (8,1,131);
		line_at (10,1,131);
		line_at (19,0,132);

		print_at (20,0, ML(mlStdMess038), 
			comm_rec.co_no, 
			clip (comm_rec.co_name));

		print_at (21,0, ML(mlStdMess039), 
			comm_rec.est_no, 
			clip (comm_rec.est_name)); 

		print_at (22,0, ML(mlStdMess099), 
			comm_rec.cc_no, 
			clip (comm_rec.cc_name));


		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);

}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	if (PRINTER)
	{
		dsp_screen (" Printing Words Orders Status Report",
					comm_rec.co_no, comm_rec.co_name);
		/*----------------------
		| Open pipe to pformat | 
 		----------------------*/
		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", cc, PNAME);
	
		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.lp_no);
		fprintf (fout, ".11\n");
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L157\n");
	}
	else  /*  DISPLAY  */
		DisplayHeading ();
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (void)
{
  	pr_format (fin, fout, "HEAD1", 0, 0);
  	pr_format (fin, fout, "HEAD2", 1, clip (comm_rec.co_name));
  	pr_format (fin, fout, "HEAD3", 1, DateToString (local_rec.st_date));
  	pr_format (fin, fout, "HEAD3", 2, DateToString (local_rec.ed_date));
	pr_format (fin, fout, "HEAD4", 1, local_rec.disp_wkstat);
	pr_format (fin, fout, "HEAD4", 2, local_rec.disp_cobrwh);
	pr_format (fin, fout, "MARGIN1", 0, 0);
  	pr_format (fin, fout, "LINE1", 0, 0);
  	pr_format (fin, fout, "HEAD5", 0, 0);
  	pr_format (fin, fout, "HEAD6", 0, 0);
  	pr_format (fin, fout, "LINE2", 0, 0);
  	pr_format (fin, fout, "RULER", 0, 0);
}

/*======================================================
| DISPLAY SCREEN.                                      |
| Display Heading at screen after clearing the screen. |
======================================================*/
void
DisplayHeading (
 void)
{
	strcpy (tempdate, DateToString (local_rec.ed_date));

	clear ();
 	rv_pr (ML(mlPcMess057), 25, 0, 1); 


 	print_at (2, 2, ML(mlPcMess045), clip (comm_rec.co_name));
	print_at (3, 2, ML(mlPcMess053), DateToString (local_rec.st_date)); 
	print_at (3, 15, ML(mlPcMess054), tempdate);
	print_at (4, 2, ML(mlPcMess055), local_rec.disp_wkstat);
	print_at (4, 15, ML(mlPcMess048), local_rec.disp_cobrwh);
}


