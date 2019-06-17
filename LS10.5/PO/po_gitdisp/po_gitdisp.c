/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_gitdisp.c,v 5.6 2002/02/01 06:00:44 robert Exp $
|  Program Name  : (po_gitdisp.c  )                                   |
|  Program Desc  : (Supplier Goods In Transit Display.          )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
|  Date Modified : (05/09/87)      | Modified  by  : Scott B. Darrow. |
|                : (05/10/88)      | Modified  by  : Bee Chwee Lim.   |
|                : (20/02/88)      | Modified  by  : Scott Darrow.    |
|                : (17/04/89)      | Modified  by  : Huon Butterworth |
|                : (19/09/90)      | Modified  by  : Scott Darrow.    |
|                : (08/02/93)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (01/09/95)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (04/09/1997)    | Modified  by : Jiggs A Veloz..   |
|                                                                     |
|  Comments      : Change program to use the display utility & add    |
|                : new search & screen generator.                     |
|       20/02/89 : Fixed problem with date due to printf/DateToString |
|       17/04/89 : Changed MONEYTYPEs to DOUBLETYPEs.                 |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|       17/04/89 : Changed MONEYTYPEs to DOUBLETYPEs.                 |
|    (08/02/93)  : Updated for Hoskyns S/C PSL-8484.                  |
|  (01/09/95)    : PDL P0001 - Updated to change PAGE_SIZE to PSIZE   |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
| $Log: po_gitdisp.c,v $
| Revision 5.6  2002/02/01 06:00:44  robert
| SC 00747 - added delay on error message
|
| Revision 5.5  2001/10/23 07:39:19  scott
| Updated to check and correct rounding.
| Changes to ensure ALL inputs and reports round the same way.
|
| Revision 5.4  2001/10/19 02:54:33  cha
| Fix   Issue # 00629 by Scott.
|
| Revision 5.3  2001/08/28 10:12:12  robert
| additional update for LS10.5-GUI
|
| Revision 5.2  2001/08/09 09:15:32  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:51  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:23  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:42  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:25  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:41  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:06  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:13  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/11/11 06:43:13  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.12  1999/11/05 05:17:10  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.11  1999/10/14 03:04:22  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.10  1999/10/01 07:44:24  scott
| Updated to make function calls standard
|
| Revision 1.9  1999/09/29 10:11:55  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:37:59  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:23  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_gitdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_gitdisp/po_gitdisp.c,v 5.6 2002/02/01 06:00:44 robert Exp $";

#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>

#ifdef PSIZE
#undef PSIZE
#endif

#define	PSIZE	13
#define	X_OFF	lp_x_off 
#define	Y_OFF	lp_y_off 

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct poslRecord	posl_rec;
struct posdRecord	posd_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;

extern	int		lp_x_off,
			lp_y_off;
int			envVarCrCo 		= 0,
			envVarCrFind	= 0;

	char 	*data			= 	"data",
			*fifteenSpaces	=	"               ",
			head_str [200],
			branchNumber [3];

	double	orderAmount 	= 0.00,
			totalAmount 	= 0.00;

/*---------------------------
| Local & Screen Structure  |
---------------------------*/
struct {
	char	startPoNumber [16],
			startSupplier [7],
			startSupplierName [41],
			endPoNumber [16],
			endSupplier [7],
			endSupplierName [41],
			dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startSupplier",	 4, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "      ",    "Start Supplier No.:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplier},
	{1, LIN, "sname",	 4, 70, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",    "Name              :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startSupplierName},
	{1, LIN, "endSupplier",	 5, 20, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~",    "End Supplier No.  :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplier},
	{1, LIN, "ename",	 5, 70, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",    "Name              :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endSupplierName},
	{1, LIN, "startPoNumber",	 7, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Start P/Order No. :", "Please start enter purchase order or <return> for Start of file.",
		YES, NO,  JUSTLEFT, "", "", local_rec.startPoNumber},
	{1, LIN, "endPoNumber",	 8, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "~", "End P/Order No.   :", "Please end enter purchase order or <return> for End of file.",
		YES, NO,  JUSTLEFT, "", "", local_rec.endPoNumber},
	{0, LIN, "",	 0, 0, CHARTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<get_lpno.h>
#include	<FindSumr.h>

/*=======================
| Function Declarations |
=======================*/
void 	CloseDB 		(void);
void 	DisplayHeading 	(void);
void 	DspShipment 	(char *);
void 	OpenDB 			(void);
void 	ProcData 		(void);
void 	PrtTotal 		(int);
void 	SrchPohr 		(char *);
void 	shutdown_prog 	(void);
int 	heading 		(int);
int 	spec_valid 		(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char* argv [])
{
	SETUP_SCR (vars);

	init_scr 	();
	clear 		();
	set_tty 	();
	set_masks 	();

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		totalAmount = 0.00;
		orderAmount = 0.00;

		/*---------------------
		| Reset Control Flags |
		---------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		search_ok 	= TRUE;
		init_vars (1);

		/*---------------------
		| Entry Screen Input  |
		---------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;
	
		/*--------------------
		| Edit Screen Input  |
		--------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		ProcData ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (pohr , pohr_list, POHR_NO_FIELDS, "pohr_id_no3");
	open_rec (poln , poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec (inmr , inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr , sumr_list, SUMR_NO_FIELDS,(!envVarCrFind) ? "sumr_id_no" 
							       					   		   : "sumr_id_no3");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (posl, posl_list, POSL_NO_FIELDS, "posl_hhpl_hash");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inmr);
	abc_fclose (posl);
	abc_fclose (posh);
	abc_fclose (posd);
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*==================
| Heading routine. |
==================*/
void
DisplayHeading (
 void)
{
	clear ();
	crsr_off ();
	/*--------------------------------------
	| Start P/O # %s  To End P/O # %s  /   |
	| Start Supplier %s  To End Supplier % |
	--------------------------------------*/
	sprintf (head_str,  ML (mlPoMess103), 
					local_rec.startPoNumber, local_rec.endPoNumber, 
					local_rec.startSupplier, local_rec.endSupplier);
	
	print_at (0, 3, head_str);
		
	line_at (1,3,130);
	
	line_at (21,0,132);
	print_at (22,0, ML (mlStdMess038), 
					comm_rec.co_no, clip (comm_rec.co_name));
	print_at (23,0,ML (mlStdMess039), 
					comm_rec.est_no,clip (comm_rec.est_name));
}

/*=============================
| Special Validation Routine. |
=============================*/
int
spec_valid (
 int field)
{
	/*--------------------------------
	| Start Supplier Search routine. |
	--------------------------------*/
	if (LCHECK ("startSupplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.startSupplier));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (!dflt_used)
		{
			if (cc)
			{
				/*--------------------
				| Supplier not found. |
				---------------------*/
				errmess (ML (mlStdMess022));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
		{
			sprintf (local_rec.startSupplier, "%-6.6s", "      ");
			sprintf (sumr_rec.crd_name, "%-40.40s", ML ("START OF RANGE"));
		}
		if (prog_status != ENTRY && strcmp (local_rec.startSupplier,local_rec.endSupplier) > 0)
		{
			/*----------------
			| Invalid range. |
			----------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.startSupplierName,"%-40.40s",sumr_rec.crd_name);
		DSP_FLD ("sname");
		DSP_FLD ("startSupplier");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| End Supplier Search routine. |
	------------------------------*/
	if (LCHECK ("endSupplier"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.endSupplier));
		cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
		if (!dflt_used)
		{
			if (cc)
			{
				/*---------------------
				| Supplier not found. |
				---------------------*/
				errmess (ML (mlStdMess022));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
		{
			sprintf (local_rec.endSupplier, "%-6.6s", "~~~~~~");
			sprintf (sumr_rec.crd_name, "%-40.40s", ML ("END OF RANGE"));
		}
		if (strcmp (local_rec.startSupplier,local_rec.endSupplier) > 0)
		{
			/*----------------
			| Invalid Range. |
			----------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		sprintf (local_rec.endSupplierName,"%-40.40s",sumr_rec.crd_name);
		DSP_FLD ("ename");
		DSP_FLD ("endSupplier");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------
	| Start Purchase Order Search routine. |
	--------------------------------------*/
	if (LCHECK ("startPoNumber"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.startPoNumber,15));
		cc = find_rec ("pohr",&pohr_rec,COMPARISON,"r");
		if (!dflt_used)
		{
			if (cc)
			{
				/*--------------------
				| P/Order not found. |
				--------------------*/
				errmess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
			strcpy (local_rec.startPoNumber, fifteenSpaces);
		
		
		if (prog_status != ENTRY && strcmp (local_rec.startPoNumber,local_rec.endPoNumber) > 0)
		{
			/*--------------- 
			| Invalid Range |
			---------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("startPoNumber");
	
		if (pohr_rec.status [0] == 'D')
		{
			/*-------------------- 
			| P/Order not found. |
			--------------------*/ 
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| End Purchase Order Search routine. |
	------------------------------------*/
	if (LCHECK ("endPoNumber"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.endPoNumber,15));
		cc = find_rec ("pohr",&pohr_rec,COMPARISON,"r");
		if (!dflt_used)
		{
			if (cc)
			{
				/*-----------------------
				| P/Order No not found. |
				-----------------------*/
				errmess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		else
			sprintf (local_rec.endPoNumber, "%-15.15s", "~");
		
		if (strcmp (local_rec.startPoNumber,local_rec.endPoNumber) > 0)
		{
			/*----------------
			| Invalid Range. |
			----------------*/
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("endPoNumber");
	
		if (pohr_rec.status [0] == 'D')
		{
			/*--------------------
			| P/Order not found. |
			--------------------*/
			errmess (ML (mlStdMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Purchase order Search. | 
========================*/
void
SrchPohr (
 char *key_val)
{
	work_open ();
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);
	save_rec ("#P/Order Number ","#Date"); 
	cc = find_rec ("pohr",&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
		      	  !strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)))
	{
		if (pohr_rec.type [0] == 'O' && pohr_rec.status [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec ("pohr",&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}

/*===========================================
| Main Processing Loop of shipment display. |
===========================================*/
void
ProcData (
 void)
{
	int		Printed = FALSE;
	char	PurNumber [26];

	abc_selfield (sumr , "sumr_hhsu_hash");

	DisplayHeading ();
	lp_x_off = 0;
	lp_y_off = 2;
	Dsp_prn_open (0, 2, PSIZE, head_str, 
			 comm_rec.co_no, comm_rec.co_name,
			 comm_rec.est_no, comm_rec.est_name,
			 (char *) 0, (char *) 0);

	Dsp_saverec ("PURCHASE ORDER |  ITEM NUMBER   |ITEM DESCRIPTION|QUANTITY|QUANTITY|    TOTAL    |  SHIPMENT  |   ETD    |   ETA    |   SUPPLIER  ");
	Dsp_saverec ("    NUMBER     |                |                |OUTSTAND| TO SHIP|    COST     |  NUMBER    |   DATE   |   DATE   | INVOICE NO. ");

	Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCN]  [PREV SCN]  [INPUT/END] ");

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.pur_ord_no, local_rec.startPoNumber);
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
		       strcmp (pohr_rec.pur_ord_no,local_rec.startPoNumber) >= 0 && 
		       strcmp (pohr_rec.pur_ord_no,local_rec.endPoNumber) <= 0)
	{
		Printed = FALSE;
		sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}
		if (strcmp (sumr_rec.crd_no,local_rec.startSupplier) < 0 || 
		     strcmp (sumr_rec.crd_no,local_rec.endSupplier) > 0)
		{
			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
			continue;
		}
		sprintf (PurNumber, "^1%s^6", pohr_rec.pur_ord_no);
		poln_rec.hhpo_hash 	= pohr_rec.hhpo_hash;
		poln_rec.line_no 	= 0;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
		{
			posl_rec.hhpl_hash = poln_rec.hhpl_hash;
			cc = find_rec (posl, &posl_rec, GTEQ, "r");
			while (!cc && posl_rec.hhpl_hash == poln_rec.hhpl_hash)
			{
				if (posl_rec.ship_qty <= 0.00)
				{
					cc = find_rec (posl, &posl_rec, NEXT, "r");
					continue;
				}

				strcpy (posh_rec.co_no, comm_rec.co_no);
				posh_rec.hhsh_hash = posl_rec.hhsh_hash;
				cc = find_rec (posh, &posh_rec, COMPARISON, "r");
				if (!cc)
				{
					Printed = TRUE;
					/*
					strcpy (PurNumber, fifteenSpaces);
					*/
				}
				DspShipment (PurNumber);
					
				cc = find_rec (posl, &posl_rec, NEXT, "r");
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		if (Printed)
			PrtTotal (TRUE);

		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	PrtTotal (FALSE);
	Dsp_srch ();
	Dsp_close ();
	abc_selfield (sumr , (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
}

/*===============================
| Display Shipment Information. |
===============================*/
void
DspShipment (
 char	*DspPurNo)
{
	char	disp_str [200];
	char	wk_date1 [11],
			wk_date2 [11];
	float	pord_qty 	= 0.00;
	double	shipValue 	= 0.00;

	pord_qty = poln_rec.qty_ord - poln_rec.qty_rec;
	strcpy (posd_rec.co_no, comm_rec.co_no);
	posd_rec.hhsh_hash	=	posh_rec.hhsh_hash;
	posd_rec.hhpo_hash	=	poln_rec.hhpo_hash;
	cc = find_rec (posd, &posd_rec, COMPARISON, "r");
	if (cc)
		return;

	inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return;
	
	sprintf (wk_date1, "%10.10s", DateToString (posh_rec.ship_depart));
	sprintf (wk_date2, "%10.10s", DateToString (posh_rec.ship_arrive));
	if (posh_rec.ship_depart <= 0L)
		strcpy (wk_date1, " Not Set. ");

	if (posh_rec.ship_arrive <= 0L)
		strcpy (wk_date2, " Not Set. ");

	shipValue = twodec (poln_rec.land_cst);
	shipValue = out_cost (shipValue, inmr_rec.outer_size);
	shipValue *= (double) posl_rec.ship_qty;

	sprintf (disp_str, "%s^E%-16.16s^E%-16.16s^E%7.0f ^E%7.0f ^E%13.13s^E%12.12s^E%10.10s^E%10.10s^E%13.13s",
		DspPurNo,
		inmr_rec.item_no,
		inmr_rec.description,
		pord_qty,
		posl_rec.ship_qty,
		comma_fmt (shipValue, "NNNNNN,NNN.NN"), 
		posh_rec.csm_no,
		wk_date1, wk_date2,
		posd_rec.inv_no);
	
	orderAmount += shipValue;
	totalAmount += shipValue;
	Dsp_saverec (disp_str);
}

/*========================================
| Display Order totals and Grand Totals. |
========================================*/
void
PrtTotal (
 int	OrdTotal)
{
	char	env_line [200];
	sprintf (env_line, "               ^E                ^E %s ^E        ^E        ^E%13.13s^E            ^E          ^E          ^E             ",

		 (OrdTotal) ? "^2 ORDER TOTAL  ^6" : "^1 GRAND TOTAL  ^6",
		 (OrdTotal) 	? comma_fmt (orderAmount, "NNNNNN,NNN.NN") 
		 	 		: comma_fmt (totalAmount, "NNNNNN,NNN.NN"));

	Dsp_saverec (env_line);

	if (OrdTotal)
		Dsp_saverec ("^^GGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGG");
	else
		Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGG");
	orderAmount = 0.00;
}

int
heading (
 int scn)
{
	swide ();
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		/*------------------------------
		| DISPLAY P/ORDERS BY SUPPLIER |
		------------------------------*/
		clear ();
		sprintf (err_str, " %s ", ML (mlPoMess104));
		rv_pr (err_str, 50,0,1);
		line_at (1,0, 132);

		box (0,3,130,5);
		line_at (6,1,129);
		line_at (21,0,132);

		print_at (22,0,ML (mlStdMess038),
						comm_rec.co_no,comm_rec.co_name);
		print_at (23,0,ML (mlStdMess039),
						comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
