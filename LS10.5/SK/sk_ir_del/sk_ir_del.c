/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (so_ir_del.c)                                      |
|  Program Desc  : (Selective Order Line Delete BY Item Order      )  |
|                  (No or Customer.                                )  |
|---------------------------------------------------------------------|
|  Access files  :  comm, sohr, soln, inmr, cumr, sobg                |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates files :  sohr, soln, sobg                                  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 25/03/91         |
|---------------------------------------------------------------------|
|  Date Modified : (26/08/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/05/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (20/05/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (17/09/97)      | Modified  by  : Elizabeth D. Paid|
|                :                                                    |
|  Comments      : (26/08/91) - Included Transfer Number for each line|
|  (11/05/94)    : HGP 10565. Add extra parameter to add_hash () for  |
|                : Real-time Committal of stock.                      |
|  (20/05/94)    : HGP 10565. Disallow entry of quantities larger     |
|                : than the original order quantity.                  |
|  (17/09/94)    :  SEL       Multilingual Conversion, changed printf |
|                :            to print_at                             |
|                                                                     |
| $Log: sk_ir_del.c,v $
| Revision 5.2  2001/08/09 09:18:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:00  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:20:16  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:10:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.17  2000/06/13 05:02:56  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.16  2000/02/21 02:56:18  scott
| Updated to increase MAXLINES to 1000.
|
| Revision 1.15  2000/01/18 01:40:59  marnie
| SC2226-Modified to lengthen the itff_hash allowed.
|
| Revision 1.14  1999/12/10 04:16:05  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.13  1999/12/06 01:30:50  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/22 06:59:05  scott
| Updated to ensure itff is deleted.
|
| Revision 1.11  1999/11/11 05:59:42  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.10  1999/11/09 04:19:19  scott
| S/C 2020 ASL.
| Updated to fix cause of item not being deleted.
|
| Revision 1.9  1999/11/09 04:15:54  scott
| Format changes only
|
| Revision 1.8  1999/11/03 07:32:03  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.7  1999/10/08 05:32:26  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:20:03  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ir_del.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_del/sk_ir_del.c,v 5.2 2001/08/09 09:18:45 scott Exp $";

#define		MAXLINES	1000
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <proc_sobg.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#include "schema"

#define	SERIAL_ITEM	 (inmr_rec.mr_serial_item[0] == 'Y')

#define	DEL_COMPANY	 	(del_by[0] == 'C')
#define	DEL_BRANCH	 	(del_by[0] == 'B')
#define	DEL_WAREHOUSE	(del_by[0] == 'W')

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
		{"comm_inv_date"},
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
	} comm_rec;

	/*==============================
	| Inventory Master File (inmr) |
	==============================*/
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
		{"inmr_category"},
		{"inmr_quick_code"},
		{"inmr_serial_item"},
		{"inmr_costing_flag"},
		{"inmr_sale_unit"},
		{"inmr_outer_size"},
		{"inmr_on_hand"}
	};

	int inmr_no_fields = 17;

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
		char 	mr_category[12];
		char 	mr_quick_code[9];
		char 	mr_serial_item[2];
		char 	mr_costing_flag[2];
		char 	mr_sale_unit[5];
		float 	mr_outer_size;
		float 	mr_on_hand;
	} inmr_rec;

	struct	ccmrRecord	ccmr_rec;
	struct	ithrRecord	ithr_rec;
	struct	itlnRecord	itln_rec,
						wkln_rec;
	struct	itffRecord	itff_rec;
	struct	insfRecord	insf_rec;

	char 	*wkln = "wkln",
			*inmr = "inmr",
			*comm = "comm";

	int	pipe_open = FALSE;

	char	del_by[2];

	char	*ser_space = "                         ";
	int		printerNumber;
	long	wk_hhbr_hash = 0L;

	float	old_total = 0.00,
			new_total = 0.00,
			old_grand = 0.00,
			new_grand = 0.00;
		
	FILE	*pp;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	item_no[17];
	char	item_desc[41];
	char	br_no[3];
	char	wh_no[3];
	char	type[2];
	long	tr_no;
	long	tr_date;
	float	tr_order;
	float	tr_supp;
	float	tr_bord;
	char	line_changed[2];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "item_no",	 3, 22, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "ALL", "Item Number          ", "Default = All Items.",
		NE, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "desc",		 4, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Description          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_desc},
	{1, LIN, "type",		 5, 22, CHARTYPE,
		"U", "          ",
		" ", "B", "B / O / T ", "B(ackorders) O(ne step Issues) T(wo step Issues) ",
		YES, NO,  JUSTLEFT, "BOT", "", local_rec.type},

	{2, TAB, "tr_item",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item  Number  ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.mr_item_no},
	{2, TAB, "br_no",	 0, 0, CHARTYPE,
		"AA", "          ",
		" ", " ", "BR", " ",
		 NA, NO, JUSTRIGHT, "", "", local_rec.br_no},
	{2, TAB, "wh_no",	 0, 0, CHARTYPE,
		"AA", "          ",
		" ", " ", "WH", " ",
		 NA, NO, JUSTRIGHT, "", "", local_rec.wh_no},
	{2, TAB, "tfer_no",	 0, 0, LONGTYPE,
		"NNNNNNN", "          ",
		" ", " ", "Tran No.", " ",
		 NA, NO, JUSTRIGHT, "", "",(char *)&local_rec.tr_no},
	{2, TAB, "tr_date",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Tran Date", " ",
		 NA, NO, JUSTRIGHT, "", "",(char *)&local_rec.tr_date},
	{2, TAB, "c/s",		 0, 2, CHARTYPE,
		"U", "          ",
		" ", " ", "C/S", " ",
		 NA, NO, JUSTRIGHT, "", "", itln_rec.stock},
	{2, TAB, "tr_order",	 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", " Order Qty", " ",
		 NA, NO, JUSTRIGHT, "0", "99999.99",(char *)&local_rec.tr_order},
	{2, TAB, "tr_supp",	 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", " Tran Qty.", " ",
		YES, NO, JUSTRIGHT, "0", "99999.99",(char *)&local_rec.tr_supp},
	{2, TAB, "tr_bord",	 0, 1, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", " B/O  Qty.", " ",
		YES, NO, JUSTRIGHT, "", "",(char *)&local_rec.tr_bord},
	{2, TAB, "l_change",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "", local_rec.line_changed},
	{2, TAB, "hash",		 0, 0, LONGTYPE,
		"NNNNNNNN", "          ",
		" ", " ", "", " ",
		 ND, NO, JUSTRIGHT, "", "",(char *)&itln_rec.itff_hash},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================= 
| Function Declarations |
=======================*/
int  DeleteIthr 	(long);
void DeleteItff 	(long);
int  heading 		(int);
int  spec_valid 	(int);
void CloseAudit 	(void);
void CloseDB 		(void);
void LoadAll 		(long);
void OpenAudit 		(void);
void OpenDB 		(void);
void PrintDetail 	(long, long, long);
void PrintGrand 	(void);
void ProcessItln 	(long);
void RuleOff 		(void);
void Update 		(void);
void UpdateInsf 	(long, char *);
void shutdown_prog 	(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	int	i;

	if (argc != 3)	
	{
		print_at (0,0,mlSkMess225,argv[0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv[1]);
	sprintf (del_by, "%-1.1s", argv[2]);

	if (!DEL_COMPANY && !DEL_BRANCH && !DEL_WAREHOUSE)
	{
		print_at (0,0,mlSkMess225,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	tab_col = 0;
	tab_row = 7;

	init_scr ();
	set_tty (); 
	set_masks ();

	OpenDB ();

	while (prog_exit == 0)
	{
		search_ok	= TRUE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		lcount[2]	= FALSE;
		init_vars (1);	

		/*------------------------------
		| Enter screen 1 linear input. |
		| Turn screen initialise on.   |
		------------------------------*/
		init_ok	= TRUE;
		eoi_ok	= FALSE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		init_ok = FALSE;
		eoi_ok = FALSE;

		i = prmptmsg (ML (mlSkMess227), "YyNn",40,5);
		print_at (5,40,"                                     ");
		fflush (stdout);

		scn_write (1);
		scn_display (1);
		scn_write (2);
		scn_display (2);
		
		if (i == 'N' || i == 'n')
			edit (2);
		else
			entry (2);

		eoi_ok = TRUE;
		if (restart)
			continue;

		prog_status = ! (ENTRY);

		edit_all ();
		if (restart)
			break;

		if (restart)
			continue;

		Update ();

		/*--------------------------
		| Clear old table entries. |
		--------------------------*/
		init_vars (2);
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
	print_at (0,0,ML (mlStdMess035));
	recalc_sobg ();

	if (pipe_open)
		CloseAudit ();

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
	abc_dbopen ("data");

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	abc_alias (wkln, itln);
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_id_no");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
	open_rec (itff, itff_list, ITFF_NO_FIELDS, "itff_itff_hash");
	open_rec (wkln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_hhbr_id");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (ithr);
	abc_fclose (itff);
	abc_fclose (itln);
	abc_fclose (insf);
	abc_fclose (ccmr);
	SearchFindClose ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.item_no, "ALL");
			strcpy (local_rec.item_desc,"ALL ITEMS");
			wk_hhbr_hash = 0L;

			DSP_FLD ("item_no");
			DSP_FLD ("desc");
			wk_hhbr_hash = 0L;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.tco_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		
		cc = FindInmr (comm_rec.tco_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.mr_co_no, comm_rec.tco_no);
			strcpy (inmr_rec.mr_item_no, local_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc) 
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.item_desc, inmr_rec.mr_description);
		wk_hhbr_hash = inmr_rec.mr_hhbr_hash;
		
		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| Validate Order type and load data. |
	------------------------------------*/ 
	if (LCHECK ("type"))
	{
		DSP_FLD ("type");

		LoadAll (wk_hhbr_hash) ;

		if (lcount[2] == 0)
		{
			/* ("No valid Lines");*/

			print_mess (ML (mlSkMess565));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate tabular item number. |
	-------------------------------*/
	if (LCHECK ("tr_item"))
	{
		if (prog_status == ENTRY)
			getval (line_cnt);

		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Quantity Supplied. |
	-----------------------------*/
	if (LCHECK ("tr_supp"))
	{
		if (dflt_used)
			local_rec.tr_supp = (float) (atof (prv_ntry));
		else
			strcpy (local_rec.line_changed, "Y");

		if (local_rec.tr_supp + local_rec.tr_bord > local_rec.tr_order)
		{
			print_mess (ML (mlSkMess226));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.tr_order = local_rec.tr_supp + local_rec.tr_bord;
		DSP_FLD ("tr_order");

		strcpy (local_rec.line_changed, "Y");

		return (EXIT_SUCCESS);
	}
	/*--------------------------------
	| Validate Quantity Backordered. |
	--------------------------------*/
	if (LCHECK ("tr_bord"))
	{
		if (dflt_used)
			local_rec.tr_bord = (float) (atof (prv_ntry));
		else
			strcpy (local_rec.line_changed, "Y");

		if (local_rec.tr_supp + local_rec.tr_bord > local_rec.tr_order)
		{
			print_mess (ML (mlSkMess226));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.tr_order = local_rec.tr_supp + local_rec.tr_bord;
		DSP_FLD ("tr_order");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================================
| Load all valid itln for given item (s). |
========================================*/
void
LoadAll (
 long _hhbr_hash)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (2);
	lcount[2] = 0;
	vars[scn_start].row = 0;

	print_mess (ML (mlStdMess035));

	if (_hhbr_hash == 0L)
	{
		strcpy (inmr_rec.mr_co_no,comm_rec.tco_no);
		strcpy (inmr_rec.mr_item_no,"                ");
		cc = find_rec (inmr,&inmr_rec, GTEQ,"r");
		while (!cc && !strcmp (inmr_rec.mr_co_no,comm_rec.tco_no))
		{
			/*-----------------------------------
			| Process lines for multiple items. |
			-----------------------------------*/
			ProcessItln (inmr_rec.mr_hhbr_hash);

			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
		}
	}
	else
		ProcessItln (_hhbr_hash);

	vars[scn_start].row = lcount[2];
	scn_set (1);

}

/*==============================================================
| Process itln records for valid company / branch / warehouse. |
==============================================================*/
void
ProcessItln (
 long _hhbr_hash)
{
	cc = find_hash (itln,&itln_rec,GTEQ,"r", _hhbr_hash);
	while (!cc && itln_rec.hhbr_hash == _hhbr_hash) 
	{
		cc = find_hash (ccmr, &ccmr_rec,EQUAL,"r", 
						itln_rec.i_hhcc_hash);

		if (cc)
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}
		/*--------------------------------------
		| Delete backorders where status = 'B' |
		--------------------------------------*/
		if (local_rec.type[0] == 'B' && itln_rec.status[0] != 'B')
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}
		/*------------------------------------
		| Delete one-step where status = 'M' |
		------------------------------------*/
		if (local_rec.type[0] == 'O' && itln_rec.status[0] != 'M')
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}
		/*------------------------------------
		| Delete two-step where status = 'U' |
		------------------------------------*/
		if (local_rec.type[0] == 'T' && itln_rec.status[0] != 'U')
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}

		if ((DEL_BRANCH || DEL_WAREHOUSE) && 
			strcmp (comm_rec.test_no,ccmr_rec.est_no))
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}
		if (DEL_WAREHOUSE &&
			strcmp (comm_rec.tcc_no,ccmr_rec.cc_no))
		{
			cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
			continue;
		}
		strcpy (local_rec.br_no, ccmr_rec.est_no);
		strcpy (local_rec.wh_no, ccmr_rec.cc_no);
		cc = find_hash (ithr,&ithr_rec,EQUAL,"r", itln_rec.hhit_hash);
		if (cc)
			local_rec.tr_no = 0L;
		else
			local_rec.tr_no = ithr_rec.del_no;

		local_rec.tr_date = itln_rec.due_date;
		local_rec.tr_order = itln_rec.qty_order + 
				     itln_rec.qty_border;

		local_rec.tr_supp =  itln_rec.qty_order;
		local_rec.tr_bord =  itln_rec.qty_border;

		strcpy (local_rec.line_changed, "N");

	    	putval (lcount[2]++);
	    	if (lcount[2] > MAXLINES) 
				break;
	
		cc = find_hash (itln,&itln_rec,NEXT,"r", _hhbr_hash);
	}
	return;
}
/*===============================================
| Delete header as all lines have been deleted. |
===============================================*/
int
DeleteIthr (
 long _hhit_hash)
{
	cc = find_hash (ithr,&ithr_rec,EQUAL,"r", _hhit_hash);
	if (cc)
		return (EXIT_SUCCESS);

	return (abc_delete (ithr));
}
/*==================================
| Print Audit for lines processed. |
==================================*/
void
PrintDetail (
 long hhbr_hash, 
 long hhcc_hash, 
 long hhit_hash)
{
	if (ccmr_rec.hhcc_hash != hhcc_hash)
	{
		if (find_hash (ccmr, &ccmr_rec, EQUAL, "r", hhcc_hash))
			return;
	}

	if (inmr_rec.mr_hhbr_hash != hhbr_hash)
	{
		if (find_hash (inmr, &inmr_rec, EQUAL, "r", hhbr_hash))
			return;
	}

	if (ithr_rec.hhit_hash != hhit_hash)
	{
		if (find_hash (ithr, &ithr_rec, EQUAL, "r", hhit_hash))
			return;
		
	}
	fprintf (pp,"|%s", ccmr_rec.est_no);
	fprintf (pp,"|%s", ccmr_rec.cc_no);
	fprintf (pp,"|%s ", inmr_rec.mr_item_no);
	fprintf (pp,"| %s",inmr_rec.mr_description);
	fprintf (pp,"| %s ",ithr_rec.tran_ref);
	fprintf (pp,"| %1.1s ", itln_rec.stock);
	fprintf (pp,"| %s ", DateToString (itln_rec.due_date));
	fprintf (pp,"|%9.2f ", itln_rec.qty_order + itln_rec.qty_border);
	fprintf (pp,"|%9.2f |\n", local_rec.tr_supp + local_rec.tr_bord);

	old_total += itln_rec.qty_order + itln_rec.qty_border;
	new_total += local_rec.tr_supp + local_rec.tr_bord;
	old_grand += itln_rec.qty_order + itln_rec.qty_border;
	new_grand += local_rec.tr_supp + local_rec.tr_bord;
}

void
RuleOff (
 void)
{
	fprintf (pp,"|--");
	fprintf (pp,"|--");
	fprintf (pp,"|-----------------");
	fprintf (pp,"|-----------------------------------------");
	fprintf (pp,"|------------------");
	fprintf (pp,"|---");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------|\n");
	
	fprintf (pp,"|  ");
	fprintf (pp,"|  ");
	fprintf (pp,"| TOTAL FOR ITEM  ");
	fprintf (pp,"|                                         ");
	fprintf (pp,"|                  ");
	fprintf (pp,"|   ");
	fprintf (pp,"|          ");
	fprintf (pp,"|%9.2f ",    old_total);
	fprintf (pp,"|%9.2f |\n", new_total);

	fprintf (pp,"|--");
	fprintf (pp,"|--");
	fprintf (pp,"|-----------------");
	fprintf (pp,"|-----------------------------------------");
	fprintf (pp,"|------------------");
	fprintf (pp,"|---");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------|\n");
	
	fprintf (pp,".LRP6\n");

	old_total = 0.00;
	new_total = 0.00;
}

void
PrintGrand (
 void)
{
	fprintf (pp,"|==");
	fprintf (pp,"|==");
	fprintf (pp,"|=================");
	fprintf (pp,"|=========================================");
	fprintf (pp,"|==================");
	fprintf (pp,"|===");
	fprintf (pp,"|==========");
	fprintf (pp,"|==========");
	fprintf (pp,"|==========|\n");
	
	fprintf (pp,"|  ");
	fprintf (pp,"|  ");
	fprintf (pp,"| GRAND TOTAL     ");
	fprintf (pp,"|                                         ");
	fprintf (pp,"|                  ");
	fprintf (pp,"|   ");
	fprintf (pp,"|          ");
	fprintf (pp,"|%9.2f ",    old_grand);
	fprintf (pp,"|%9.2f |\n", new_grand);
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat","w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf (pp,".SO\n");
	fprintf (pp,".LP%d\n",printerNumber);
	fprintf (pp,".PI12\n");
	if (DEL_COMPANY)
		fprintf (pp,".9\n");

	if (DEL_BRANCH)
		fprintf (pp,".10\n");

	if (DEL_WAREHOUSE)
		fprintf (pp,".11\n");

	fprintf (pp,".L158\n");
	if (local_rec.type[0] == 'B')
		fprintf (pp,".E DELETED STOCK TRANSFER BACKORDERS\n");

	if (local_rec.type[0] == 'O')
		fprintf (pp,".E DELETED ONE-STEP STOCK TRANSFERS\n");

	if (local_rec.type[0] == 'T')
		fprintf (pp,".E DELETED ONE-STEP STOCK TRANSFERS\n");

	fprintf (pp,".EAS AT %s\n",SystemTime ());

	fprintf (pp,".ECOMPANY : %s - %s\n",
			clip (comm_rec.tco_no),clip (comm_rec.tco_name));

	if (DEL_BRANCH || DEL_WAREHOUSE)
	{
		fprintf (pp,".EBRANCH : %s - %s\n",
			clip (comm_rec.test_no),clip (comm_rec.test_name));
	}
	
	if (DEL_WAREHOUSE)
	{
		fprintf (pp,".EWAREHOUSE : %s - %s\n",
			clip (comm_rec.tcc_no),clip (comm_rec.tcc_name));
	}
	fprintf (pp,".B1\n");

	fprintf (pp,".R===");
	fprintf (pp,"===");
	fprintf (pp,"==================");
	fprintf (pp,"==========================================");
	fprintf (pp,"===================");
	fprintf (pp,"====");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"============\n");

	fprintf (pp,"===");
	fprintf (pp,"===");
	fprintf (pp,"==================");
	fprintf (pp,"==========================================");
	fprintf (pp,"===================");
	fprintf (pp,"====");
	fprintf (pp,"===========");
	fprintf (pp,"===========");
	fprintf (pp,"============\n");

	fprintf (pp,"|BR");
	fprintf (pp,"|WH");
	fprintf (pp,"|   ITEM NUMBER   ");
	fprintf (pp,"|             ITEM DESCRIPTION            ");
	fprintf (pp,"|  TRANS INT REF.  ");
	fprintf (pp,"|C/S");
	fprintf (pp,"|   DATE   ");
	fprintf (pp,"| OLD QTY. ");
	fprintf (pp,"| NEW QTY. |\n");

	fprintf (pp,"|--");
	fprintf (pp,"|--");
	fprintf (pp,"|-----------------");
	fprintf (pp,"|-----------------------------------------");
	fprintf (pp,"|------------------");
	fprintf (pp,"|---");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------");
	fprintf (pp,"|----------|\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp,".EOF\n");
	pclose (pp);
}

void
UpdateInsf (
 long hhbr_hash, 
 char *serial_no)
{
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"C");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, "insf", "DBUPDATE");

		abc_unlock (insf);
		return;
	}
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"S");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, "insf", "DBUPDATE");

		abc_unlock (insf);
		return;
	}
	insf_rec.hhbr_hash = hhbr_hash;
	sprintf (insf_rec.serial_no, "%-25.25s",serial_no);
	strcpy (insf_rec.status,"T");
	if (!find_rec (insf,&insf_rec,COMPARISON,"u"))
	{
		strcpy (insf_rec.status,"F");
		cc = abc_update (insf,&insf_rec);
		if (cc)
			file_err (cc, "insf", "DBUPDATE");

		abc_unlock (insf);
		return;
	}
}

/*=================
| Update routine. |
=================*/
void
Update (
 void)
{
	long	change_hash = 0L;

	clear ();

	fflush (stdout);
	
	scn_set (2);

	abc_selfield (itln, "itln_itff_hash");
	abc_selfield (inmr, "inmr_hhbr_hash");

	if (local_rec.type[0] == 'B')
		print_at (0,0,ML (mlStdMess035));

	if (local_rec.type[0] == 'O')
		print_at (0,0,ML (mlStdMess035));

	if (local_rec.type[0] == 'T')
		print_at (0,0,ML (mlStdMess035));

	for (line_cnt = 0; line_cnt < lcount[2] ; line_cnt++)
	{
		getval (line_cnt);
	
		cc = find_hash (itln,&itln_rec,EQUAL,"u", itln_rec.itff_hash);
		if (cc)
		{
			abc_unlock (itln);
			continue;
		}
		if (local_rec.line_changed[0] == 'N')
		{
			abc_unlock (itln);
			continue;
		}
		if (!pipe_open)
		{
			change_hash = itln_rec.hhbr_hash;
			OpenAudit ();
			pipe_open = TRUE;
		}

		if (change_hash != itln_rec.hhbr_hash)
			RuleOff ();

		change_hash = itln_rec.hhbr_hash;

		PrintDetail (itln_rec.hhbr_hash, 
		  	      itln_rec.i_hhcc_hash,
			      itln_rec.hhit_hash);

		if (local_rec.tr_supp <= 0.00 && local_rec.tr_bord <= 0.00)
		{
			DeleteItff (itln_rec.itff_hash);
 			cc = abc_delete (itln);
			if (cc)
				file_err (cc, "itln", "DBDELETE");
		}
		else
		{
			itln_rec.qty_order  = local_rec.tr_supp;
			itln_rec.qty_border = local_rec.tr_bord;
			cc = abc_update (itln, &itln_rec);
			if (cc)
				file_err (cc, "itln", "DBUPDATE");
		}
		add_hash 
		(
			ccmr_rec.co_no, 
			ccmr_rec.est_no,
			"RC", 
			0,
			itln_rec.hhbr_hash,
			itln_rec.i_hhcc_hash, 
			0L,
			(double) 0.00
		);

		add_hash 
		(
			ccmr_rec.co_no, 
			ccmr_rec.est_no,
			"RC", 
			0,
			(itln_rec.r_hhbr_hash == 0L) ? itln_rec.hhbr_hash 
								  : itln_rec.r_hhbr_hash,
			itln_rec.r_hhcc_hash, 
			0L,
			(double) 0.00
		);
	
		if (SERIAL_ITEM && strcmp (itln_rec.serial_no,ser_space))
		{
			UpdateInsf (itln_rec.hhbr_hash, 
				     itln_rec.serial_no);
		}

		wkln_rec.hhit_hash = itln_rec.hhit_hash;
		wkln_rec.line_no = 0;
		cc = find_rec (wkln,&wkln_rec,GTEQ,"r");
		if (cc || wkln_rec.hhit_hash != itln_rec.hhit_hash)
			DeleteIthr (itln_rec.hhit_hash);
	}
	
	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (itln, "itln_hhbr_hash");

	if (pipe_open)
	{
		RuleOff ();
		PrintGrand ();
	}
	return;
}

void 
DeleteItff (
	long	itffHash)
{

	itff_rec.itff_hash	=	itffHash;
	cc = find_rec ("itff", &itff_rec, GTEQ, "r");
	while (!cc && itff_rec.itff_hash == itffHash)
	{
		abc_delete ("itff");
		itff_rec.itff_hash	=	itffHash;
		cc = find_rec ("itff", &itff_rec, GTEQ, "r");
	}
}
int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	rv_pr (ML (mlSkMess228),15,0,1);

	scn_set ((scn == 1) ? 2 : 1);
	scn_write ((scn == 1) ? 2 : 1);
	scn_display ((scn == 1) ? 2 : 1);

	box (0,2,80,3);

	move (0,1);
	line (80);

	if (DEL_COMPANY)
	{
		move (0,21);
		line (80);

		print_at (22,0,ML (mlStdMess038),comm_rec.tco_no,comm_rec.tco_name);
		move (0,23);
		line (80);
	}
	
	if (DEL_BRANCH)
	{
		move (0,20);
		line (80);

		print_at (21,0,ML (mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.test_no,comm_rec.test_name);
		move (0,23);
		line (80);
	}
	if (DEL_WAREHOUSE)
	{
		move (0,19);
		line (80);

		print_at (20,0,ML (mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		print_at (21,0, ML (mlStdMess039), comm_rec.test_no,comm_rec.test_name);
		print_at (22,0,ML (mlStdMess099), comm_rec.tcc_no,comm_rec.tcc_name);
		move (0,23);
		line (80);
	}
	scn_write (scn);
    return (EXIT_SUCCESS);
}
