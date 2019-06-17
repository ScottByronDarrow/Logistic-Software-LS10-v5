/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ir_disp.c,v 5.4 2002/07/17 09:57:53 scott Exp $
|  Program Name  : (sk_ir_disp.c)
|  Program Desc  : (Transfer Issues/Receipts Display)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 27/03/89          |
|---------------------------------------------------------------------|
| $Log: sk_ir_disp.c,v $
| Revision 5.4  2002/07/17 09:57:53  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/10/18 06:54:30  cha
| Clean and convert program sk_ir_disp.c (unmatched transfers).
| Modified by Scott.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ir_disp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_disp/sk_ir_disp.c,v 5.4 2002/07/17 09:57:53 scott Exp $";

#define	X_OFF	40
#define	Y_OFF	6
#include <pslscr.h>
#include <twodec.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

#define	DSP_ISSUE	 (local_rec.tranType [0] == 'I')
#define	DSP_ONE_I	 (local_rec.tranType [0] == 'O')
#define	DSP_TWO_I	 (local_rec.tranType [0] == 'T')
#define	DSP_REC		 (local_rec.tranType [0] == 'R')
#define	DSP_REQ		 (local_rec.tranType [0] == 'Q')
#define	DSP_UTRAN	 (local_rec.tranType [0] == 'U')
#define	BACKORDER	 (local_rec.type [0] == 'B')
#define	DISP   		 (local_rec.displayPrint [0] == 'D')
#define	PRINT		 (local_rec.displayPrint [0] == 'P')
#define	DAILY_ONLY	 (local_rec.tranFreq [0] == 'D')

#define	SEL_PSIZE	14

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;

	int		envDbCo 	= 0,
			envDbFind 	= 0,
			flipPrint 	= FALSE,
			print_ok 	= FALSE,
			pipeOpen 	= FALSE,
			NORMAL 		= TRUE,
			promptUser 	= TRUE;

	char	*data = "data";

	char	dispStr [300],
			systemDate [11];

	long	hhccHash = 0L,
			workDate = 0L;

	double	totalExtend = 0.00;
	extern	int	TruePosition;

	FILE	*fout;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	tranType [2];
	char	tranTypeDesc [21];
	char	tranFreq [2];
	char	tranFreqDesc [11];
	char	displayPrint [2];
	char	displayPrintDesc [11];
	long	del_no;
	char	type [2];
	char	is_br [3];
	char	rc_br [3];
	char	is_wh [3];
	char	rc_wh [3];
	int		lpno;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "tran_type",	 4, 2, CHARTYPE,
		"U", "        ",
		" ", "I", "Transaction Type.   ", "(O)ne step issues (T)wo step issues (I)ssues (R)eceipts  Re(Q)uests (U)nmatched Trans.",
		YES, NO,  JUSTLEFT, "IRQUTO", "", local_rec.tranType},
	{1, LIN, "tran_type_desc",	 4, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "        ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.tranTypeDesc},
	{1, LIN, "tranFreq",	 5, 2, CHARTYPE,
		"U", "        ",
		" ", "A", "Daily / ALL Trans.  ", " D(aily) Default = A(ll) ",
		YES, NO,  JUSTLEFT, "DA", "", local_rec.tranFreq},
	{1, LIN, "tranFreqDesc",	 5, 25, CHARTYPE,
		"AAAAAA", "        ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.tranFreqDesc},
	{1, LIN, "displayPrint",	 6, 2, CHARTYPE,
		"U", "        ",
		" ", "D", "Display / Print.    ", " ",
		YES, NO,  JUSTLEFT, "DP", "", local_rec.displayPrint},
	{1, LIN, "displayPrintDesc",	 6, 25, CHARTYPE,
		"AAAAAAAA", "        ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.displayPrintDesc},
	{1, LIN, "lpno",	 6, 60, INTTYPE,
		"NN", "        ",
		" ", "1", "Printer No.         ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{2, LIN, "type",	 4, 2, CHARTYPE,
		"U", "        ",
		" ", "T", "Transfer Type.      ", " T(ransfers) B(ackorder)",
		YES, NO, JUSTRIGHT, "", "", local_rec.type},
	{2, LIN, "del_no",	 5, 2, LONGTYPE,
		"NNNNNN", "        ",
		"0", "0", "Transfer No         ", " Default : All Outstanding Transfer Issues.",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.del_no},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
int  	GetWarehouse 		(long);
int  	heading 			(int);
int  	spec_valid 			(int);
int  	ValidateIthr 		(void);
void 	CloseDB 			(void);
void 	DisplayHeading 		(void);
void 	OpenDB 				(void);
void 	OtherTrans 			(void);
void 	PrintLine 			(void);
void 	PrintUnmatchedTrans (long);
void 	ProcessCcmr 		(long);
void 	ProcessItln 		(long);
void 	psl_print 			(void);
void 	ReadMisc 			(void);
void 	SetDefault 			(void);
void 	shutdown_prog 		(void);
void 	SrchIthr 			(char *);
void 	StartReport 		(void);
void 	UnmatchedTransfers 	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int		run_prog = TRUE;
	char	*sptr;

	TruePosition	=	TRUE;

	if (argc > 1)
	{
		if (argc != 5 && argc != 6)
		{
			print_at (0,0,mlSkMess106,argv [0]);
			print_at (1,0,mlSkMess503,argv [0]);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.tranType, "%-1.1s", (argc == 5) ? argv [1] : "U");
		sprintf (local_rec.tranFreq,"%-1.1s", argv [2]);
		sprintf (local_rec.displayPrint,"%-1.1s", (argc == 5) ? argv [3] : "P");
		local_rec.lpno = atoi (argv [4]);
		if (argc == 6)
		{
			sprintf (local_rec.type, "%-1.1s", argv [5]);
			local_rec.del_no = 0L;
			promptUser = FALSE;
		}
		run_prog = FALSE;

		if (!DSP_ISSUE && !DSP_ONE_I && !DSP_TWO_I && 
		     !DSP_REC   && !DSP_REQ   && !DSP_UTRAN)
		{
			print_at (0,0,mlSkMess106,argv [0]);
			print_at (1,0,mlSkMess503,argv [0]);
			return (EXIT_FAILURE);
		}
	}
	strcpy (systemDate, DateToString (TodaysDate ()));
	workDate = TodaysDate ();

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	if (!strncmp (sptr, "sk_ir_disp", 10))
		NORMAL = TRUE;
	else
		NORMAL = FALSE;

	OpenDB ();

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();

	if (run_prog)
	{
		swide ();
		clear ();

		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		print_ok 	= FALSE;
		init_vars 	(1);	
		SetDefault 	();

		heading 	(1);
		scn_display (1);
		edit 		(1);
        if (restart) 
		{
			shutdown_prog ();
            return (EXIT_SUCCESS);
        }
	}
	
	if (DSP_UTRAN)
	{
		if (promptUser)
		{
			search_ok 	= TRUE;
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			print_ok 	= FALSE;
			init_ok 	= TRUE;
			init_vars (2);	

			heading (2);
			entry (2);
            if (prog_exit || restart) 
			{
				shutdown_prog ();
                return (EXIT_SUCCESS);
            }
			heading (2);
			scn_display (2);
			edit (2);
		}
		UnmatchedTransfers ();
	}
	else
		OtherTrans ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefault (void)
{
	strcpy (local_rec.tranType, "I");
	strcpy (local_rec.tranTypeDesc, ML ("Issues"));

	strcpy (local_rec.tranFreq, "A");
	strcpy (local_rec.tranFreqDesc, ML ("All"));

	strcpy (local_rec.displayPrint,"D");
	strcpy (local_rec.displayPrintDesc, ML ("Display"));
	local_rec.lpno = 1;
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
	if (pipeOpen)
	{
		fprintf (fout,".EOF\n");
		pclose (fout);
	}
}

/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	ReadMisc ();
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (ccmr);
	abc_fclose (ithr);
	abc_fclose (itln);
	abc_dbclose (data);
}

/*
 * Get common info from commom database file .
 */
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr , ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr , &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	hhccHash = ccmr_rec.hhcc_hash;
	
	abc_selfield (ccmr,"ccmr_hhcc_hash");
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("tran_type"))
	{
		switch (local_rec.tranType [0])
		{
		case	'O':
			strcpy (local_rec.tranTypeDesc, ML ("One step issues"));
			break;

		case	'T':
			strcpy (local_rec.tranTypeDesc, ML ("Two step issues"));
			break;

		case	'I':
			strcpy (local_rec.tranTypeDesc, ML ("Issues"));
			break;

		case	'R':
			strcpy (local_rec.tranTypeDesc, ML ("Receipts"));
			break;

		case	'Q':
			strcpy (local_rec.tranTypeDesc, ML ("Request"));
			break;

		case	'U':
			strcpy (local_rec.tranTypeDesc, ML ("Unmatched Transfers"));
			break;

		}
		DSP_FLD ("tran_type_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("tranFreq"))
	{
		strcpy (local_rec.tranFreqDesc, (local_rec.tranFreq [0] == 'D') 
						? ML ("Daily") : ML ("All"));
		DSP_FLD ("tranFreqDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("displayPrint"))
	{
		if (DISP)
		{
			strcpy (local_rec.displayPrintDesc, ML("Display"));
			FLD ("lpno") = NA;
		}
		else
		{
			strcpy (local_rec.displayPrintDesc, ML("Print"));
			FLD ("lpno") = YES;
		}
		DSP_FLD ("displayPrintDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Purchase Order Number.
	 */
	if (LCHECK ("del_no"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIthr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ");
		strcpy (ithr_rec.type, "T");
		ithr_rec.del_no = local_rec.del_no;
		cc = find_rec (ithr,&ithr_rec,COMPARISON,"r");
		if (cc || ithr_rec.del_no != local_rec.del_no)
		{
			sprintf (err_str,ML (mlSkMess232),local_rec.del_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
UnmatchedTransfers (void)
{
	if (PRINT)
	{
	 	print_at (2,0,ML (mlSkMess039));
		if (!pipeOpen)
			StartReport ();
	}
	else
	{
		print_ok = TRUE;
		heading (3);
		Dsp_open (0,2,SEL_PSIZE);

		if (BACKORDER)
		{
			Dsp_saverec (" Transfer  |Iss |Iss |Rec |Rec | Backorder| UOM |Backorder |Backorder |  Item Number    |  Item Description                 ");
			Dsp_saverec ("  Number.  |Br. |Wh. |Br. |Wh. |   Date   |     | Quantity |  Cost    |                 |                                   ");
		}
		else
		{
			Dsp_saverec ("  Transfer |Iss |Iss |Rec |Rec |  Issue   | UOM |  Issue   |  Issue   |  Item Number    |  Item Description                 ");
			Dsp_saverec ("   Number  |Br. |Wh. |Br. |Wh. |   Date   |     | Quantity |  Cost    |                 |                                   ");
		}
		Dsp_saverec (" [REDRAW]  [PRINT]  [NEXT SCN]  [PREV SCN][EDIT / END]");
	}

	strcpy (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ");
	strcpy (ithr_rec.type, "T");
	ithr_rec.del_no = local_rec.del_no;
	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");
	while (!cc && 
		!strcmp (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ") && 
		ValidateIthr () && ithr_rec.type [0] == 'T')
	{
		PrintUnmatchedTrans (ithr_rec.hhit_hash);

		cc = find_rec (ithr,&ithr_rec,NEXT,"r");
	}
	if (DISP)
	{
		Dsp_srch ();
		Dsp_close ();
		print_ok = FALSE;
	}
}

void
PrintUnmatchedTrans (
	long	hhitHash)
{
	int		first_time = TRUE;
	char	del_no [7];
	float	wk_qty       = 0.00;

	itln_rec.hhit_hash = hhitHash;
	itln_rec.line_no = 0;
	cc = find_rec (itln, &itln_rec,GTEQ,"r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (DAILY_ONLY)
		{
			if (workDate != ithr_rec.iss_date && 
		 	     workDate != ithr_rec.iss_sdate)
			{
				cc = find_rec (itln,&itln_rec,NEXT,"r");
				continue;
			}
		}
		if (BACKORDER)
		{
			wk_qty = itln_rec.qty_border;
		}
		else
			wk_qty = itln_rec.qty_order;

		if (wk_qty <= 0.00)
		{
			cc = find_rec (itln, &itln_rec,NEXT,"r");
			continue;
		}
		sprintf (del_no,"%06ld",ithr_rec.del_no);
 
        inmr_rec.hhbr_hash = itln_rec.hhbr_hash;

		cc = find_hash (inmr,&inmr_rec,EQUAL,"r",itln_rec.hhbr_hash);
		if (cc)
			strcpy (inmr_rec.item_no,"????????????????");

		cc = GetWarehouse (itln_rec.i_hhcc_hash);
		strcpy (local_rec.is_br, (cc) ? "  " : ccmr_rec.est_no);
		strcpy (local_rec.is_wh, (cc) ? "  " : ccmr_rec.cc_no);
		cc = GetWarehouse (itln_rec.r_hhcc_hash);
		strcpy (local_rec.rc_br, (cc) ? "  " : ccmr_rec.est_no);
		strcpy (local_rec.rc_wh, (cc) ? "  " : ccmr_rec.cc_no);
		
		if (DISP)
		{
			sprintf (dispStr,"  %-6.6s   ^E %s ^E %s ^E %s ^E %s ^E %s ^E%-4.4s ^E%10.2f^E%10.2f^E %-16.16s^E %-34.34s",
				 (first_time) ? del_no : " ",
				local_rec.is_br,
				local_rec.is_wh,
				local_rec.rc_br,
				local_rec.rc_wh,
				DateToString (itln_rec.due_date),
                inmr_rec.sale_unit,
				wk_qty,
				itln_rec.cost,
				inmr_rec.item_no,
				itln_rec.item_desc);

			Dsp_saverec (dispStr);

			if (inmr_rec.serial_item [0] == 'Y')
			{
				sprintf (dispStr,"           ^E    ^E    ^E    ^E    ^E          ^E          ^E          ^E                 ^E SERIAL NO : %-25.25s",
				itln_rec.serial_no);

				Dsp_saverec (dispStr);
			}
		}
		else
		{
			fprintf (fout,"!   %-6.6s  ", (first_time) ? del_no : " ");
			fprintf (fout,"! %s ! %s ! %s ! %s ",local_rec.is_br,local_rec.is_wh,local_rec.rc_br,local_rec.rc_wh);

			fprintf (fout,"!%-10.10s",DateToString (ithr_rec.iss_date));

			fprintf (fout,"!%-4.4s ",inmr_rec.sale_unit);
			fprintf (fout,"! %10.2f ",wk_qty);
			fprintf (fout,"! %10.2f ",itln_rec.cost);
			fprintf (fout,"!%-16.16s ",inmr_rec.item_no);
			fprintf (fout,"!%-34.34s !\n",itln_rec.item_desc);
			if (inmr_rec.serial_item [0] == 'Y')
			{
				fprintf (fout,"!           ");
				fprintf (fout,"!    !    !    !    ");
				fprintf (fout,"!          ");
				fprintf (fout,"!            ");
				fprintf (fout,"!            ");
				fprintf (fout,"!                 ");
				fprintf (fout,"! SERIAL NO : %-25.25s   !\n", 
						itln_rec.serial_no); 
			}
		}
		first_time = FALSE;

		cc = find_rec (itln,&itln_rec,NEXT,"r");
	}
	if (!first_time)
	{
		if (DISP)
			Dsp_saverec ("^^GGGGGGGGGGGHGGGGHGGGGHGGGGHGGGGHGGGGGGGGGGHGGGGGHGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		else
			PrintLine ();
	}
}

void
OtherTrans (
 void)
{
	totalExtend = 0.00;

	if (PRINT)
	{
		if (!flipPrint)
			dsp_screen ("PRINTING ON-LINE TRANSFERS REPORT.",comm_rec.co_no,comm_rec.co_name);

		StartReport ();
	}
	else
	{
		print_ok = TRUE;
		heading (3);
		Dsp_open (0,2,SEL_PSIZE);

		if (DSP_ISSUE || DSP_ONE_I || DSP_TWO_I)
		{
			Dsp_saverec ("Trans |  Item  number  |       Item  Description        |C/S|  Issue to  | UOM |  Qty  |  Qty  |  Qty  |    Cost   |    Value.   ");
			Dsp_saverec ("Number|                |                                |   |            |     |Ordered|B/Order|Receipt|   Issue   |  Of Issue   ");
		}
		if (DSP_REC)
		{
			Dsp_saverec ("Trans |  Item  number  |       Item  Description        |C/S|Receipt From| UOM |  Qty  |  Qty  |  Qty  |    Cost   |    Value.   ");
			Dsp_saverec ("Number|                |                                |   |            |     |Ordered|B/Order|Receipt|  Receipt  |  Of Receipt ");
		}
		if (DSP_REQ)
		{
			Dsp_saverec ("Trans |  Item  number  |       Item  Description        |C/S|Request From| UOM |  Qty  |  Qty  |  Qty  |    Cost   |    Value.   ");
			Dsp_saverec ("Number|                |                                |   |            |     |Request|B/Order|To Supp|of  Request| of  Request ");
		}
		Dsp_saverec (" [REDRAW]  [PRINT]  [NEXT SCN]  [PREV SCN][EDIT / END]");
	}

	strcpy (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ");
	strcpy (ithr_rec.type," ");
	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");
	while (!cc && !strcmp (ithr_rec.co_no, (NORMAL) 
						? comm_rec.co_no : "  "))
	{
		if (DSP_REQ && ithr_rec.type [0] != 'R')
		{
			cc = find_rec (ithr,&ithr_rec,NEXT,"r");
			continue;
		}

		if ((DSP_TWO_I && ithr_rec.type [0] != 'U') ||
		   (!DSP_TWO_I && ithr_rec.type [0] == 'U'))
		{
			cc = find_rec (ithr,&ithr_rec,NEXT,"r");
			continue;
		}
		if ((DSP_ONE_I && ithr_rec.type [0] != 'M') ||
		   (!DSP_ONE_I && ithr_rec.type [0] == 'M'))
		{
			cc = find_rec (ithr,&ithr_rec,NEXT,"r");
			continue;
		}
		
		ProcessItln (ithr_rec.hhit_hash);

		cc = find_rec (ithr,&ithr_rec,NEXT,"r");
	}
	if (DISP)
	{
		sprintf (dispStr,"      ^E ** TOTALS **   ^E                                ^E   ^E            ^E     ^E       ^E       ^E       ^E           ^E %12.2f",totalExtend);
		Dsp_saverec (dispStr);
		Dsp_srch ();
		Dsp_close ();
		print_ok = FALSE;
	}
	else
	{
		fprintf (fout,"!           ");
		fprintf (fout,"!  *** TOTALS *** ");
		fprintf (fout,"!                                   ");
		fprintf (fout,"!   ");
		fprintf (fout,"!            ");
		fprintf (fout,"!     ");
		fprintf (fout,"!       ");
		fprintf (fout,"!       ");
		fprintf (fout,"!       ");
		fprintf (fout,"!           ");
		fprintf (fout,"! %12.2f!\n",totalExtend);
		fflush (fout);
	}
}

void
ProcessItln (
	long	hhitHash)
{
	char	iss_date [11],
			rec_date [11];

	float	wk_qty [3];
	long	wk_hash = 0L;
	double	extend = 0.00;
	double	tot_trans = 0.00;
	int		first_time = TRUE;
	
	itln_rec.hhit_hash = hhitHash;
	itln_rec.line_no = 0;
	cc = find_rec (itln, &itln_rec,GTEQ,"r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (DSP_ISSUE || DSP_REQ || DSP_ONE_I || DSP_TWO_I)
		{
			if (DAILY_ONLY)
			{
				if (workDate != ithr_rec.iss_date && 
				     workDate != ithr_rec.iss_sdate)
				{
					cc = find_rec (itln,&itln_rec,NEXT,"r");
					continue;
				}
			}
			if (DSP_REQ)
			{
				ProcessCcmr (itln_rec.r_hhcc_hash);
				wk_hash = itln_rec.i_hhcc_hash;
			}
			else
			{
				ProcessCcmr (itln_rec.r_hhcc_hash);
				wk_hash = itln_rec.i_hhcc_hash;
			}
		}
		else
		{
			if (workDate != ithr_rec.rec_date && DAILY_ONLY)
			{
				cc = find_rec (itln,&itln_rec,NEXT,"r");
				continue;
			}
			ProcessCcmr (itln_rec.i_hhcc_hash);
			wk_hash = itln_rec.r_hhcc_hash;
		}
		if (wk_hash != hhccHash) 
		{
			cc = find_rec (itln,&itln_rec,NEXT,"r");
			continue;
		}

        inmr_rec.hhbr_hash = itln_rec.hhbr_hash;

		cc = find_hash (inmr,&inmr_rec,EQUAL,"r",itln_rec.hhbr_hash);
		if (cc)
			strcpy (inmr_rec.item_no,"????????????????");

		if (first_time)
		{
			sprintf (iss_date,"%10.10s",DateToString (ithr_rec.iss_date));
			sprintf (rec_date,"%10.10s",DateToString (ithr_rec.rec_date));

			if (DISP)
			{
				sprintf (dispStr,"%06ld^E  Date Issue %10.10s     Date Receipt %10.10s     Internal Ref: %-16.16s",ithr_rec.del_no,
						 iss_date,
						 rec_date,
						 ithr_rec.tran_ref);
				Dsp_saverec (dispStr);
			}
			else
			{
				fprintf (fout,"!  %06ld   ",ithr_rec.del_no);
				fprintf (fout,"!  Date Issue %-10.10s    ",iss_date);
				fprintf (fout,"    Date Receipt %-10.10s   ",rec_date);
				fprintf (fout,"!   Trans Int Ref : %-39.39s          !\n",ithr_rec.tran_ref);
				fflush (fout);
			}
		}
		first_time = FALSE;

		wk_qty [0] = (itln_rec.qty_order + itln_rec.qty_rec);
		wk_qty [1] = itln_rec.qty_border;
		wk_qty [2] = itln_rec.qty_rec;
		
		if (DSP_ISSUE || DSP_ONE_I || DSP_TWO_I)
			extend = (itln_rec.qty_order + itln_rec.qty_rec) * itln_rec.cost;
		
		if (DSP_REC)
			extend =  itln_rec.qty_rec * itln_rec.cost;

		if (DSP_REQ)
		{
			extend =  itln_rec.qty_order * itln_rec.cost;
			wk_qty [0] = itln_rec.qty_order + itln_rec.qty_rec;
			wk_qty [1] = itln_rec.qty_border;
			wk_qty [2] = itln_rec.qty_order;
		}

		if (DISP)
		{
			sprintf (dispStr,"      ^E%16.16s^E%-32.32s^E %1.1s ^E%-2.2s-%-9.9s^E %-4.4s^E%7.1f^E%7.1f^E%7.1f^E%11.2f^E%13.2f",
				inmr_rec.item_no,
				itln_rec.item_desc,
				itln_rec.stock,
				ccmr_rec.cc_no,
				ccmr_rec.acronym,
				inmr_rec.sale_unit,
				wk_qty [0], wk_qty [1], wk_qty [2],
				itln_rec.cost,
				extend);

			Dsp_saverec (dispStr);
	
			if (inmr_rec.serial_item [0] == 'Y')
			{
				sprintf (dispStr,"      ^E                ^E SERIAL NO : %-25.25s^E   ^E            ^E       ^E       ^E       ^E           ^E             ",
					itln_rec.serial_no);

				Dsp_saverec (dispStr);
			}
		}
		else
		{
			fprintf (fout,"!           ");
			fprintf (fout,"!%-16.16s ",inmr_rec.item_no);
			fprintf (fout,"!%-34.34s ",itln_rec.item_desc);
			fprintf (fout,"! %-1.1s ",itln_rec.stock);
			fprintf (fout,"!%-2.2s-%-9.9s",ccmr_rec.cc_no,ccmr_rec.acronym);
			fprintf (fout,"! %-4.4s",inmr_rec.sale_unit);
			fprintf (fout,"!%7.1f", (itln_rec.qty_order + itln_rec.qty_rec));
			fprintf (fout,"!%7.1f",itln_rec.qty_border);
			fprintf (fout,"!%7.1f",itln_rec.qty_rec);
			fprintf (fout,"!%11.2f",itln_rec.cost);
			fprintf (fout,"!%13.2f!\n",extend);

			if (inmr_rec.serial_item [0] == 'Y')
			{
				fprintf (fout,"!           ");
				fprintf (fout,"!           ");
				fprintf (fout,"!  SERIAL NO : %-25.25s  ", itln_rec.serial_no);
				fprintf (fout,"!     ");
				fprintf (fout,"!   ");
				fprintf (fout,"!            ");
				fprintf (fout,"!       ");
				fprintf (fout,"!       ");
				fprintf (fout,"!       ");
				fprintf (fout,"!           ");
				fprintf (fout,"!             !\n");
			}
			fflush (fout);
		}

		tot_trans += extend;
		totalExtend    += extend;

		cc = find_rec (itln,&itln_rec,NEXT,"r");
	}
	if (!first_time)
	{
		if (DISP)
		{
			sprintf (dispStr,"      ^E                ^E ** Total for Trans. %06ld **  ^E   ^E            ^E     ^E       ^E       ^E       ^E           ^E %12.2f",ithr_rec.del_no,tot_trans);
			Dsp_saverec (dispStr);
			sprintf (dispStr,"^^GGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGHGGGGGGGGGGGGHGGGGGHGGGGGGGHGGGGGGGHGGGGGGGHGGGGGGGGGGGHGGGGGGGGGGGGG");
			Dsp_saverec (dispStr);
		}
		else
		{
			fprintf (fout,"!           ");
			fprintf (fout,"!                 ");
			fprintf (fout,"!  ** Total for Trans  %06ld **    ",ithr_rec.del_no);
			fprintf (fout,"!   ");
			fprintf (fout,"!            ");
			fprintf (fout,"!     ");
			fprintf (fout,"!       ");
			fprintf (fout,"!       ");
			fprintf (fout,"!       ");
			fprintf (fout,"!           ");
			fprintf (fout,"! %12.2f!\n",tot_trans);
			PrintLine ();
		}
	}
}

int
ValidateIthr (void)
{
	if (local_rec.del_no == 0L || ithr_rec.del_no == local_rec.del_no)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int
GetWarehouse (
	long	hhccHash)
{
	ccmr_rec.hhcc_hash	=	hhccHash;
	return (find_rec (ccmr,&ccmr_rec,COMPARISON,"r"));
}

void
SrchIthr (
	char *keyValue)
{
	char	del_no [9];

	_work_open (8,0,20);
	save_rec ("#Trans No","#Date - Line No");
	strcpy (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ");
	strcpy (ithr_rec.type, "T");
	ithr_rec.del_no = atol (keyValue);

	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");

	sprintf (del_no,"%08ld",ithr_rec.del_no);
	while (!cc && !strcmp (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ") && 
		      ithr_rec.type [0] == 'T')
	{
		sprintf (del_no,"%08ld",ithr_rec.del_no);
		sprintf (err_str, "%s", DateToString (ithr_rec.iss_date));
		cc = save_rec (del_no, err_str);
		if (cc)
			break;
		cc = find_rec (ithr,&ithr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ithr_rec.co_no, (NORMAL) ? comm_rec.co_no : "  ");
	strcpy (ithr_rec.type,"T");
	ithr_rec.del_no = atol (temp_str);
	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");
	if (cc || ithr_rec.del_no != atol (temp_str))
		file_err (cc, "ithr", "DBFIND");
}

void
ProcessCcmr (
	long	hhccHash)
{
	if (ccmr_rec.hhcc_hash == hhccHash)
		return;

	ccmr_rec.hhcc_hash = hhccHash;
	cc = find_rec (ccmr ,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		swide ();
		clear ();

		line_at (1,0,132);

		switch (scn)
		{
		case	1:
			if (NORMAL)
				rv_pr (ML (mlSkMess034),44,0,1);
			else
				rv_pr (ML (mlSkMess035),38,0,1);
			box (0,3,132,3);

			line_cnt = 0;
			scn_write (scn);

			break;

		case	2:
			box (0,3,132,2);

			DisplayHeading ();
			line_cnt = 0;
			scn_write (scn);
			break;

		case	3:
			DisplayHeading ();
			break;

		default:
			break;
		}
		/*  reset this variable for new screen NOT page	*/

		line_at (19,0,132);
		print_at (20,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		print_at (22,0,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_name);
	}
    return (EXIT_SUCCESS);
}

void
DisplayHeading (void)
{
	char	displayPrint [11];

	if (DISP)
		strcpy (displayPrint, ML ("Display"));
	else
		strcpy (displayPrint, ML ("Print  "));

	if (DSP_ISSUE || DSP_REC || DSP_ONE_I || DSP_TWO_I)
	{
		if (DAILY_ONLY)
		{
			if (DSP_REC)
				sprintf (err_str,ML (mlSkMess575),displayPrint,systemDate);
			else
				sprintf (err_str,ML (mlSkMess576),displayPrint,systemDate);
				
		}
		else
		{
			if (DSP_REC)
				sprintf (err_str,ML (mlSkMess036),displayPrint);
			else
				sprintf (err_str,ML (mlSkMess040),displayPrint);
		}
		rv_pr (err_str,45,0,1);
	}
	if (DSP_REQ)
	{
		if (DAILY_ONLY)
			sprintf (err_str,ML (mlSkMess037),displayPrint,systemDate);
		else
			sprintf (err_str,ML (mlSkMess502),displayPrint);
		rv_pr (err_str,45,0,1);
	}
	if (DSP_UTRAN)
	{
		sprintf (err_str,ML (mlSkMess038),displayPrint);
		rv_pr (err_str,45,0,1);
	}
}

void
StartReport (void)
{
	char	title_str [13];
	char	title_str2 [26];
	char	rep_head [61];

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	if (DSP_ISSUE || DSP_REC || DSP_ONE_I || DSP_TWO_I)
	{
		if (DAILY_ONLY)
		{
			if (DSP_REC)
				sprintf (rep_head,ML (mlSkMess577),systemDate);
			else
				sprintf (rep_head,ML (mlSkMess578),systemDate);
		}
		else
		{
			sprintf (rep_head,"Print All Stock Transfer %s Report", (DSP_REC) ? "Receipts" : "Issues");
		}

		sprintf (title_str,"%-12.12s", (DSP_REC) ? "RECEIPT FROM" : " ISSUE  TO  ");
		sprintf (title_str2,"%-25.25s", (DSP_REC) ? "  RECEIPT  ! OF RECEIPT  " : "   ISSUE   !  OF  ISSUE  ");
	}
	if (DSP_REQ)
	{
		if (DAILY_ONLY)
			sprintf (rep_head,ML (mlSkMess591),systemDate);
		else
			sprintf (rep_head,ML (mlSkMess689));

		sprintf (title_str,"%-12.12s","REQUEST FROM");
		sprintf (title_str2,"%-25.25s"," OF REQUEST!  OF REQUEST ");
	}
	if (DSP_UTRAN)
	{
		sprintf (rep_head,"%-s",ML (mlSkMess620));
		sprintf (title_str,"%-10.10s", (BACKORDER) ? "BACKORDER " : "UNMATCHED ");
	}

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n", local_rec.lpno);
	fprintf (fout,".11\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".E%-s\n",clip (rep_head));
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT : %s\n", SystemTime ());

	if (DSP_UTRAN)
	{
		fprintf (fout,".R============"); 
		fprintf (fout,"===================="); 
		fprintf (fout,"=====================================");    
		fprintf (fout,"==================");
		fprintf (fout,"===========================================\n");

		fprintf (fout,"============"); 
		fprintf (fout,"===================="); 
		fprintf (fout,"=====================================");    
		fprintf (fout,"==================");
		fprintf (fout,"===========================================\n");

		fprintf (fout,"! DELIVERY  ");
		fprintf (fout,"!ISS !ISS !REC !REC "); 
		fprintf (fout,"!%s! UOM ! %s ! %s ",title_str,title_str,title_str);
		fprintf (fout,"!  ITEM NUMBER    ");    
		fprintf (fout,"! ITEM DESCRIPTION                  !\n");

		fprintf (fout,"! DOCKET NO "); 
		fprintf (fout,"!BR. !WH. !BR. !WH. "); 
		fprintf (fout,"!   DATE   !     !  QUANTITY  !   COST     ");    
		fprintf (fout,"!                 ");
		fprintf (fout,"!                                   !\n");
	}
	else
	{
		fprintf (fout,".R============"); 
		fprintf (fout,"==================");    
		fprintf (fout,"==========================================");
		fprintf (fout,"====");
		fprintf (fout,"============="); 
		fprintf (fout,"========================");
		fprintf (fout,"===========================\n"); 

		fprintf (fout,"============"); 
		fprintf (fout,"==================");    
		fprintf (fout,"==========================================");
		fprintf (fout,"====");
		fprintf (fout,"============="); 
		fprintf (fout,"========================");
		fprintf (fout,"===========================\n"); 

		fprintf (fout,"! DELIVERY  ");
		fprintf (fout,"!  ITEM NUMBER    ");    
		fprintf (fout,"! ITEM DESCRIPTION                  ");
		fprintf (fout,"!S/C");
		fprintf (fout,"!%s",title_str);
		fprintf (fout,"! UOM !  QTY  !  QTY  !  QTY  !   COST    !    VALUE    !\n"); 

		fprintf (fout,"! DOCKET NO "); 
		fprintf (fout,"!                 "); 
		fprintf (fout,"!                                   "); 
		fprintf (fout,"!   ");
		fprintf (fout,"!            "); 
		fprintf (fout,"!     !ORDERED!B/ORDER!%s", (DSP_REQ) ? "REQUEST"
							      : "RECEIPT");
		fprintf (fout,"!%s!\n",title_str2);
	}

	PrintLine ();
	fflush (fout);
	pipeOpen = TRUE;
}

void
PrintLine (void)
{
	if (DSP_UTRAN)
	{
		fprintf (fout,"!-----------"); 
		fprintf (fout,"!----!----!----!----"); 
		fprintf (fout,"!----------!-----!------------!------------");    
		fprintf (fout,"!-----------------");
		fprintf (fout,"!-----------------------------------!\n");
	}
	else
	{
		fprintf (fout,"!-----------"); 
		fprintf (fout,"!-----------------");
		fprintf (fout,"!-----------------------------------");
		fprintf (fout,"!---");
		fprintf (fout,"!------------");
		fprintf (fout,"!-----!-------!-------!-------");
		fprintf (fout,"!-----------!-------------!\n");    
	}
}

void
psl_print (void)
{
	if (!print_ok)
		return;

	flipPrint = FALSE;

	local_rec.lpno = get_lpno (0);

	rv_pr (ML (mlStdMess035),41,7,1);

	if (DISP)
	{
		flipPrint = TRUE;
		strcpy (local_rec.displayPrintDesc, ML ("Print "));
	}

	if (DSP_UTRAN)
		UnmatchedTransfers ();
	else
		OtherTrans ();

	if (flipPrint)
		strcpy (local_rec.displayPrintDesc, ML ("Display"));

	flipPrint = FALSE;

	fprintf (fout,".EOF\n");
	pclose (fout);
	return;
}

