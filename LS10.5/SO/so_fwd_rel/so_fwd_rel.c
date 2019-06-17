/*=====================================================================
|  Copyright (C) 1988 - 1999 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( so_fwd_rel.c                                 )   |
|  Program Desc  : ( Forward Order Release.                       )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, sohr, soln, inmr, ccmr,                     |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :  sohr, soln,                                       |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : 12/01/89        | Modified  by  : Scott Darrow.    |
|  Date Modified : 01/06/89        | Modified  by  : Scott Darrow.    |
|  Date Modified : 10/07/89        | Modified  by  : Scott Darrow.    |
|  Date Modified : 27/07/90        | Modified  by  : Scott Darrow.    |
|  Date Modified : (30/01/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (27/09/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (16/09/93)      | Modified  by  : Campbell Mander. |
|  Date Modified : (24/01/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (24/01/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (17/02/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/05/94)      | Modified by : Scott B Darrow.    |
|  Date Modified : (07/11/94)      | Modified by : Aroha Merrilees.   |
|  Date Modified : (17/09/97)      | Modified by : Marnie Organo.     |
|                :                                                    |
|  Comments      : Fixed Status flag as they where all stuffed up.    |
|                : 01/06/89 - added so_fwd_avl                        |
|                : 10/07/89 - Removed addition of sobg for "CK".      |
|                : 27/07/90 - Major update inclusing Comment and non  |
|                :            stock lines.                            |
|                : (30/01/91) - Fixed serial items not being released.|
|                : (27/09/91) - Updated to allow for program to be    |
|                :              run in overnight.                     |
|  (16/09/93)    : HGP 9503. Increase cus_ord_ref to 20 chars.        |
|  (24/01/94)    : DHL 10168. Tidy for global mods before logic change|
|  (24/01/94)    : DHL 10168. Ripple changes through from PSL 8946.   |
|  (17/02/94)    : PSL 10456. Fix error message when running Due Date |
|                : option.                                            |
|  (11/05/94)    : HGP 10565 Updated to add <RealCommit.h>.           |
|  (07/11/94)    : PSL 11299 - mfg cutover - avail less qc qty        |
|  (17/09/97)    : Updated for Multilingual Conversion.               |
|                :                                                    |
| $Log: so_fwd_rel.c,v $
| Revision 5.2  2001/08/09 09:21:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:16  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:19:35  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:40:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:22:20  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:13:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.18  1999/11/19 03:16:36  scott
| Updated due to conflict with INT_MIN on RS6000
|
| Revision 1.17  1999/11/16 23:40:16  scott
| Updated to remove PNAME as not available with Ctrl-P
|
| Revision 1.16  1999/11/16 00:58:57  scott
| Updated for define of MIN conflict on IBM
|
| Revision 1.15  1999/11/04 04:53:55  scott
| Updated to fix warnings due to setting -Wall flag on compiler.
|
| Revision 1.14  1999/10/13 04:01:56  nz
| Updated to ensure read_comm in correct place.
|
| Revision 1.13  1999/09/29 10:13:41  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/29 08:45:22  scott
| Updated from Ansi Project
|
| Revision 1.11  1999/07/13 11:44:17  ana
| (13/07/1999) Corrected validation of forward orders - SC1664.
|                                                         |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_fwd_rel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_fwd_rel/so_fwd_rel.c,v 5.2 2001/08/09 09:21:14 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<proc_sobg.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>

#define	MANUAL		0
#define	DATE		1
#define	BY_DATE		(selection == DATE)
#define	InternalMIN(X,Y)	( X < Y ) ? X : Y ;

#define	COMMENT_LINE	( incc_rec.cc_sort[0] == 'Z' && hold_comments)
#define	NON_STOCK	( incc_rec.cc_sort[0] == 'N' )
#define	FORWARD		( soln_rec.ln_status[0] == 'F' )
#define	PHANTOM		( incc_rec.cc_sort[0] == 'P' )

	int	selection;
	int	ser_release = FALSE;

	char	*data = "data",
			*comm = "comm",
			*ccmr = "ccmr",
			*incc = "incc",
			*inmr = "inmr",
			*sohr = "sohr",
			*sokt = "sokt",
			*soln = "soln";

	char	*ser_space = "                         ";

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
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
	} comm_rec;

	/*==========================
	| Sales Order Header File. |
	==========================*/
	struct dbview sohr_list[] ={
		{"sohr_co_no"},
		{"sohr_br_no"},
		{"sohr_order_no"},
		{"sohr_cus_ord_ref"},
		{"sohr_hhso_hash"},
		{"sohr_dt_required"},
		{"sohr_status"},
		{"sohr_stat_flag"},
	};

	int sohr_no_fields = 8;

	struct {
		char	hr_co_no[3];
		char	hr_br_no[3];
		char	hr_order_no[9];
		char	hr_cus_ord_ref[21];
		long	hr_hhso_hash;
		long	hr_dt_required;
		char	hr_status[2];
		char	hr_stat_flag[2];
	} sohr_rec;

	/*================================
	| Sales Order Detail Lines File. |
	================================*/
	struct dbview soln_list[] ={
		{"soln_hhso_hash"},
		{"soln_line_no"},
		{"soln_hhbr_hash"},
		{"soln_hhcc_hash"},
		{"soln_hhsl_hash"},
		{"soln_serial_no"},
		{"soln_qty_order"},
		{"soln_qty_bord"},
		{"soln_due_date"},
		{"soln_status"},
		{"soln_stat_flag"}
	};

	int soln_no_fields = 11;

	struct {
		long	ln_hhso_hash;
		int		ln_line_no;
		long	ln_hhbr_hash;
		long	ln_hhcc_hash;
		long	ln_hhsl_hash;
		char	ln_serial_no[26];
		float	ln_qty_order;
		float	ln_qty_bord;
		long	ln_due_date;
		char	ln_status[2];
		char	ln_stat_flag[2];
	} soln_rec;

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
		char	mr_co_no[3];
		char	mr_est_no[3];
		char	mr_cc_no[3];
		long	mr_hhcc_hash;
	} ccmr_rec;


	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_hhbr_hash"},
		{"inmr_hhsi_hash"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_stat_flag"}
	};

	int inmr_no_fields = 5;

	struct {
		long	im_hhbr_hash;
		long	im_hhsi_hash;
		char	im_serial_item[2];
		char	im_costing_flag[2];
		char	im_stat_flag[2];
	} inmr_rec;

	/*====================================
	| Inventory Cost centre Base Record. |
	====================================*/
	struct dbview incc_list[] ={
		{"incc_hhcc_hash"},
		{"incc_hhbr_hash"},
		{"incc_sort"},
		{"incc_committed"},
		{"incc_backorder"},
		{"incc_closing_stock"},
		{"incc_qc_qty"},
		{"incc_stat_flag"}
	};

	int incc_no_fields = 8;

	struct {
		long	cc_hhcc_hash;
		long	cc_hhbr_hash;
		char	cc_sort[29];
		float	cc_committed;
		float	cc_backorder;
		float	cc_closing_stock;
		float	cc_qc_qty;
		char	cc_stat_flag[2];
	} incc_rec;

	/*===========================
	| Sales Order kitting file. |
	===========================*/
	struct dbview sokt_list[] ={
		{"sokt_co_no"},
		{"sokt_hhbr_hash"},
		{"sokt_line_no"},
		{"sokt_mabr_hash"},
		{"sokt_matl_qty"}
	};

	int sokt_no_fields = 5;

	struct {
		char	kt_co_no[3];
		long	kt_hhbr_hash;
		int		kt_line_no;
		long	kt_mabr_hash;
		float	kt_matl_qty;
	} sokt_rec;

	float	cstock = 0.00;
	int		con_orders = 0,
			hold_comments = 0;
	int		QC_APPLY = FALSE,
			SK_QC_AVL = FALSE;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	systemDate[11];
	long	start_date;
	long	end_date;
	char	order_no[9];
	char	cus_ord_ref[21];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "start_date",	 4, 12, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", "00/00/00", "Start Date", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.start_date},
	{1, LIN, "end_date",	 4, 45, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", local_rec.systemDate, "End Date", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.end_date},
	{1, LIN, "order_no",	 4, 12, CHARTYPE,
		"NNNNNNNN", "          ",
		"0", "0", "Order No", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.order_no},
	{1, LIN, "cus_ord_ref",	 4, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Cust. Ref.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.cus_ord_ref},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include 	<RealCommit.h>
/*======================= 
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
int  spec_valid (int field);
void srch_order (char *key_val);
void process (void);
void proc_soln (long hhso_hash);
void find_soln (void);
void fwd_rel (void);
void backorder (void);
void update_sohr (long hhso_hash);
int  heading (int scn);
int  check_serial (long hhbr_hash);
float proc_phant (long hhbr_hash);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;
	long	offset = 0L;

	SETUP_SCR (vars);

	/*---------------------------------------
	| check forward for order consolidation	|
	---------------------------------------*/
	sptr = chk_env ("CON_ORDERS");
	if (sptr == (char *)0)
		con_orders = 0;
	else
		con_orders = atoi (sptr);

	sptr = chk_env ("SO_COMM_HOLD");
	if (sptr == (char *)0)
		hold_comments = 0;
	else
		hold_comments = atoi (sptr);

	if (argc < 2)
	{
		print_at (0,0,mlSoMess728,argv[0]);
		print_at (1,0,mlSoMess729);
		print_at (2,0,mlSoMess730);
		print_at (3,0,mlSoMess731);

		return (EXIT_FAILURE);
	}
	
	switch (argv[1][0])
	{
	case	'D':
	case	'd':
		selection = DATE;
		FLD ("order_no") = ND;
		FLD ("cus_ord_ref") = ND;
		break;

	case	'M':
	case	'm':
		FLD ("start_date") = ND;
		FLD ("end_date") = ND;
		selection = MANUAL;
		break;

	default:
		print_at (1,0,mlSoMess728, argv[0]);
		print_at (2,0,mlSoMess729);
		return (EXIT_FAILURE);
		break;
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	if (argc > 2)
	{
		offset = atol (argv[2]);

		selection = DATE;
		local_rec.start_date = 0L;
		local_rec.end_date   = StringToDate (local_rec.systemDate) + offset;
		OpenDB ();
		process ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
 	}

	/* QC module is active or not. */
	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	/* Whether to include QC qty in available stock. */
	SK_QC_AVL = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;
	
	init_scr ();
	set_tty ();

	set_masks ();
	init_vars (1);

	OpenDB ();

	/*=====================
	| Reset control flags |
	=====================*/
	while (prog_exit == 0)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		process ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (sohr, sohr_list, sohr_no_fields, "sohr_id_no2");
	open_rec (soln, soln_list, soln_no_fields, "soln_hhbr_hash");
	open_rec (soln, soln_list, soln_no_fields, "soln_hhbr_hash");
	open_rec (incc, incc_list, incc_no_fields, "incc_id_no");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec(soic, soic_list, soic_no_fields, "soic_id_no2");
}

void
CloseDB (
 void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose(soic);
	abc_dbclose (data);
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, ccmr_no_fields, "ccmr_id_no");

	strcpy (ccmr_rec.mr_co_no,  comm_rec.tco_no);
	strcpy (ccmr_rec.mr_est_no, comm_rec.test_no);
	strcpy (ccmr_rec.mr_cc_no,  comm_rec.tcc_no);

	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	char	s_date[11];
	char	e_date[11];

	if (LCHECK ("start_date"))
	{
		if (prog_status == ENTRY)
			return(0);

		if (local_rec.start_date > local_rec.end_date)
		{
			strcpy (s_date, DateToString (local_rec.start_date));
			strcpy (e_date, DateToString (local_rec.end_date));
			print_mess (ML(mlStdMess019));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_date"))
	{
		if (local_rec.start_date > local_rec.end_date)
		{
			strcpy (s_date, DateToString (local_rec.start_date));
			strcpy (e_date, DateToString (local_rec.end_date));
			print_mess (ML(mlStdMess019));
			return (EXIT_FAILURE);
		}
		return(0);
	}

	if (LCHECK ("order_no"))
	{
		if (FLD ("order_no") == ND)
			return (EXIT_SUCCESS);

		abc_selfield (sohr,"sohr_id_no2");

		if (last_char == EOI)
		{
			prog_exit = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			srch_order (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.order_no, "ALL     ");
			display_field (field);
			return (EXIT_SUCCESS);
		}

		strcpy (sohr_rec.hr_co_no, comm_rec.tco_no);
		strcpy (sohr_rec.hr_br_no, comm_rec.test_no);
		strcpy (sohr_rec.hr_order_no, local_rec.order_no);
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
		if (cc || sohr_rec.hr_status[0] != 'F')
		{
			abc_unlock (sohr);
			print_mess (ML(mlStdMess122));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.cus_ord_ref, sohr_rec.hr_cus_ord_ref);
		display_field (field + 1);

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=====================
| Search for Order no |
======================*/
void
srch_order (
 char *key_val)
{
	work_open ();
	save_rec ("#Order","#Date ");                       
	strcpy (sohr_rec.hr_co_no, comm_rec.tco_no);
	strcpy (sohr_rec.hr_br_no, comm_rec.test_no);
	sprintf (sohr_rec.hr_order_no, "%-8.8s", key_val);
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && 
		   !strncmp (sohr_rec.hr_order_no, key_val, strlen (key_val)) &&
		   !strcmp (sohr_rec.hr_co_no, comm_rec.tco_no) && 
		   !strcmp (sohr_rec.hr_br_no, comm_rec.test_no))
	{ 
		if (sohr_rec.hr_status[0] == 'F')
		{
			sprintf (err_str,
					 "%s %s", 
					 sohr_rec.hr_cus_ord_ref,
					 DateToString (sohr_rec.hr_dt_required));

			cc = save_rec (sohr_rec.hr_order_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (sohr_rec.hr_co_no, comm_rec.tco_no);
	strcpy (sohr_rec.hr_br_no, comm_rec.test_no);
	sprintf (sohr_rec.hr_order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sohr, "DBFIND");
}

void
process (
 void)
{
	char	s_date[11];
	char	e_date[11];

	abc_selfield (sohr, "sohr_hhso_hash");

	if (BY_DATE)
	{
		strcpy (s_date, DateToString (local_rec.start_date));
		strcpy (e_date, DateToString (local_rec.end_date));
		sprintf (err_str, 
				 "Forward Order Release %s to %s",
				 s_date, 
				 e_date);
		dsp_screen (err_str, comm_rec.tco_no, comm_rec.tco_name);
		
		find_soln ();
	}
	else
	{
		if (!strcmp (local_rec.order_no, "ALL     "))
		{
			dsp_screen ("Forward Order Release - All Orders",
						comm_rec.tco_no,
						comm_rec.tco_name);
			find_soln ();
		}
		else
		{
			sprintf (err_str,
					 "Forward Order Release - Order %s",
					 sohr_rec.hr_order_no);
			dsp_screen (err_str, comm_rec.tco_no, comm_rec.tco_name);
			proc_soln (sohr_rec.hr_hhso_hash);
		}
	}
}

/*=================================================
| Process soln lines for selective forward Order. |
=================================================*/
void
proc_soln (
 long hhso_hash)
{
	float	realCommitted;

	abc_selfield(soln, "soln_id_no");

	soln_rec.ln_hhso_hash = hhso_hash;
	soln_rec.ln_line_no = 0;

	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.ln_hhso_hash == hhso_hash)
	{
		if (!FORWARD || soln_rec.ln_hhcc_hash != ccmr_rec.mr_hhcc_hash)
		{
			abc_unlock (soln);
			cc = find_rec (soln, &soln_rec, NEXT, "u");
			continue;
		}
		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", soln_rec.ln_hhbr_hash);
		if (cc)
			file_err (cc, inmr, "DBFIND");
			
		sprintf (err_str, " : Line %4d", soln_rec.ln_line_no);
		dsp_process (sohr_rec.hr_order_no, err_str);

		if (PHANTOM)
			cstock = proc_phant (soln_rec.ln_hhbr_hash);
		else
		{
			incc_rec.cc_hhcc_hash = soln_rec.ln_hhcc_hash;
			incc_rec.cc_hhbr_hash = alt_hash (inmr_rec.im_hhbr_hash,
											  inmr_rec.im_hhsi_hash);
			cc = find_rec(incc, &incc_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (soln);
				cc = find_rec (soln, &soln_rec, NEXT, "u");
				continue;
			}
			/*---------------------------------
			| Calculate Actual Qty Committed. |
			---------------------------------*/
			realCommitted = RealTimeCommitted ( incc_rec.cc_hhbr_hash,
												incc_rec.cc_hhcc_hash );

			cstock = incc_rec.cc_closing_stock -
					 realCommitted -
					 incc_rec.cc_committed -
					 incc_rec.cc_backorder;
			if (QC_APPLY && SK_QC_AVL)
				cstock -= incc_rec.cc_qc_qty;
		}
	
		if (check_serial (incc_rec.cc_hhbr_hash))
			ser_release = TRUE;
		else
			ser_release = FALSE;

		/*---------------------------------------------------------
		| Release if closing stock > 0 or 'Z' or 'N' Class items. |
		---------------------------------------------------------*/
		if (cstock > 0.00 || COMMENT_LINE || NON_STOCK || ser_release)
			fwd_rel ();
		else
			backorder ();

		abc_unlock (soln);
		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);
}

/*===============================
| Find sales order line record. |
===============================*/
void
find_soln (
 void)
{
	long	hhbr_hash = 0L;
	float	realCommitted;

	abc_selfield (soln, "soln_hhbr_hash");

	cc = find_hash (soln, &soln_rec, GTEQ, "u", 0L);
	while (!cc)
	{
		if (BY_DATE && 
			(soln_rec.ln_due_date < local_rec.start_date || 
			 soln_rec.ln_due_date > local_rec.end_date))
		{
			abc_unlock (soln);
			cc = find_hash (soln, &soln_rec, NEXT, "u", 0L);
			continue;
		}

		if (!FORWARD || soln_rec.ln_hhcc_hash != ccmr_rec.mr_hhcc_hash)
		{
			abc_unlock (soln);
			cc = find_hash (soln, &soln_rec, NEXT, "u", 0L);
			continue;
		}

		cc = 0;

		if (hhbr_hash != soln_rec.ln_hhbr_hash)
		{
			cc = find_hash (inmr, &inmr_rec, EQUAL, "r", soln_rec.ln_hhbr_hash);
			if (cc)
				file_err (cc, inmr, "DBFIND");
			incc_rec.cc_hhcc_hash = soln_rec.ln_hhcc_hash;
			incc_rec.cc_hhbr_hash = alt_hash (inmr_rec.im_hhbr_hash,
							 				  inmr_rec.im_hhsi_hash);

			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (soln);
				cc = find_hash (soln, &soln_rec, NEXT, "u", 0L);
				continue;
			}
			/*---------------------------------
			| Calculate Actual Qty Committed. |
			---------------------------------*/
			realCommitted = RealTimeCommitted ( incc_rec.cc_hhbr_hash,
												incc_rec.cc_hhcc_hash );

			cstock = incc_rec.cc_closing_stock -
					 realCommitted -
					 incc_rec.cc_committed -
					 incc_rec.cc_backorder;
			if (QC_APPLY && SK_QC_AVL)
				cstock -= incc_rec.cc_qc_qty;

			hhbr_hash = soln_rec.ln_hhbr_hash;
		}
		if (check_serial (incc_rec.cc_hhbr_hash))
			ser_release = TRUE;
		else
			ser_release = FALSE;

		if (cstock > 0.00 || COMMENT_LINE || NON_STOCK || ser_release)
			fwd_rel ();
		else
			backorder ();

		abc_unlock (soln);

		cc = find_hash (soln, &soln_rec, NEXT, "u", 0L);
	}
	abc_unlock(soln);
}

/*========================
| Release forward order. |
========================*/
void
fwd_rel (
 void)
{
	float	qty_order = 0.0;
	float	qty_supp = 0.0;
	float	qty_left = 0.0;

	qty_order = soln_rec.ln_qty_order + soln_rec.ln_qty_bord;
	qty_supp  = soln_rec.ln_qty_order + soln_rec.ln_qty_bord;

	if (COMMENT_LINE || NON_STOCK || ser_release)
		cstock = soln_rec.ln_qty_order + soln_rec.ln_qty_bord;

	/*-----------------------------------------
	| cstock need to be held & reduced        |
	| as each valid soln record is processed. |
	-----------------------------------------*/
	if (cstock > 0.00 || COMMENT_LINE || NON_STOCK || ser_release)
	{
		qty_left = cstock;
		qty_supp = qty_order;

		if (cstock <= qty_order)
			qty_supp = InternalMIN (qty_left, cstock);

		if ( qty_order < qty_supp )
			qty_supp = InternalMIN (qty_left, qty_order);

		soln_rec.ln_qty_order = qty_supp;
		soln_rec.ln_qty_bord  = qty_order - qty_supp;
	
		cstock -= qty_supp;

		strcpy (soln_rec.ln_status, "C");
		strcpy (soln_rec.ln_stat_flag, (con_orders) ? "M" : "R");
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, soln, "DBUPDATE");

		/*------------------------------------------
		| Add record for later updating of record. |
		------------------------------------------*/
		add_hash (comm_rec.tco_no, comm_rec.test_no, "RC", 0,
		  	  	  soln_rec.ln_hhbr_hash, soln_rec.ln_hhcc_hash, 
			  	  0L, (double) 0.00 );

		update_sohr (soln_rec.ln_hhso_hash);
	}
	else
		backorder ();

}

/*=============================================
| Not enough stock so release as a Backorder. |
=============================================*/
void
backorder (
 void)
{
	strcpy (soln_rec.ln_status, "B");
	strcpy (soln_rec.ln_stat_flag, "B");
	soln_rec.ln_qty_bord += soln_rec.ln_qty_order;
	soln_rec.ln_qty_order = 0.00;
	cc = abc_update (soln, &soln_rec);
	if (cc)
		file_err (cc, soln, "DBUPDATE");
		
	/*------------------------------------------
	| Add record for later updating of record. |
	------------------------------------------*/
	add_hash (comm_rec.tco_no, comm_rec.test_no, "RC", 0,
	  	  	  soln_rec.ln_hhbr_hash, soln_rec.ln_hhcc_hash, 
		  	  0L, (double) 0.00);

	cc = find_hash (sohr, &sohr_rec, COMPARISON, "u", soln_rec.ln_hhso_hash);
	if (!cc)
	{
		if (sohr_rec.hr_status[0] != 'C')
		{
			strcpy (sohr_rec.hr_status,"B");
			strcpy (sohr_rec.hr_stat_flag,"B");
			cc = abc_update (sohr, &sohr_rec);
			if (cc)
				file_err (cc, sohr, "DBUPDATE");
		}
		abc_unlock (sohr);
	}
}

/*====================================
| Update Header for Lines Processed. |
====================================*/
void
update_sohr (
 long hhso_hash)
{
	cc = find_hash(sohr, &sohr_rec, COMPARISON, "u", hhso_hash);
	if (!cc)	
	{
		strcpy (sohr_rec.hr_status, "C");
		strcpy (sohr_rec.hr_stat_flag, (con_orders) ? "M" : "R");
		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, sohr, "DBUPDATE");

		abc_unlock (sohr);
	}
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

		rv_pr (ML(mlSoMess038), 26, 0, 1);

		move (0, 1);
		line (80);

		box (0, 3, 80, 1);
		
		move (0, 19);

		print_at (20,0,ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);
		print_at (21,0,ML(mlStdMess039), comm_rec.test_no, comm_rec.test_name);
		print_at (22,0,ML(mlStdMess099), comm_rec.tcc_no, comm_rec.tcc_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
/*============================================================
| Check if item is a serial item and a serial number exists. |
============================================================*/
int
check_serial (
 long hhbr_hash)
{
	if (inmr_rec.im_serial_item[0] == 'Y' && 
		strcmp (soln_rec.ln_serial_no, ser_space))
	{
		return (TRUE);
	}
	
	return (FALSE);
}

/*=============================================
| Specific code to handle single level Bills. |
=============================================*/
float	
proc_phant (
 long hhbr_hash)
{
	int		first_time = TRUE;
	float	min_qty = 0.00,
			on_hand = 0.00;
	float	realCommitted;

	open_rec (sokt, sokt_list, sokt_no_fields, "sokt_hhbr_hash");

	cc = find_hash (sokt, &sokt_rec, GTEQ, "r", hhbr_hash);
	while (!cc && sokt_rec.kt_hhbr_hash == hhbr_hash)
	{
		incc_rec.cc_hhcc_hash = ccmr_rec.mr_hhcc_hash;
		incc_rec.cc_hhbr_hash = sokt_rec.kt_mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_hash (sokt, &sokt_rec, NEXT, "r", hhbr_hash);
			continue;
		}

		/*---------------------------------
		| Calculate Actual Qty Committed. |
		---------------------------------*/
		realCommitted = RealTimeCommitted ( incc_rec.cc_hhbr_hash,
											incc_rec.cc_hhcc_hash );
	
		on_hand = incc_rec.cc_closing_stock -
		   	  	  incc_rec.cc_committed -
				  realCommitted -
		   	  	  incc_rec.cc_backorder;
		if (QC_APPLY && SK_QC_AVL)
			on_hand -= incc_rec.cc_qc_qty;
		
		on_hand /= sokt_rec.kt_matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_hash (sokt, &sokt_rec, NEXT, "r", hhbr_hash);
	}
	abc_fclose (sokt);

	return (min_qty);
}



