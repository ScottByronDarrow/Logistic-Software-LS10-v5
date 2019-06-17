/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lot_disprt.c,v 5.4 2002/07/17 09:57:56 scott Exp $
|  Program Name  : (lot_disprt.c)                                     |
|  Program Desc  : (Print Lot Quantities Discrepancies Report  )      |
|                  (And Bin Locations Quantities Discrepancies.)      |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 27/09/93         |
|---------------------------------------------------------------------|
| $Log: lot_disprt.c,v $
| Revision 5.4  2002/07/17 09:57:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/09/19 03:12:18  robert
| Updated to avoid overlapping of descriptions
|
| Revision 5.2  2001/08/09 09:18:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:22  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:37:40  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 10:46:36  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:12  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lot_disprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_lot_disprt/lot_disprt.c,v 5.4 2002/07/17 09:57:56 scott Exp $";

#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>

#define	MAXREC		100

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;

	char	*data = "data";
 
	FILE	*fout;

	struct 
	{
		char	bin_loc [11];
		char	lot_no [8];
		float	qty;
		char 	est_no [3];
		char 	cc_no [3];
	} store_bin [MAXREC];

	struct 
	{
		char	est_no [3];
		char	cc_no [3];
		double	on_hand;
	} storeTotalBin [MAXREC];

	int 	recCountBin;
	int 	totalCntBin;

	int		loopCnt;
	int 	printed = FALSE;
	char	QtyStr [30];

#include	<LocHeader.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	startItemNo [17];
	char	st_item_desc [41];
	char	endItemNo [17];
	char	ed_item_desc [41];
	char	co_br_wh [2];
	char	cobrwh_desc [10];
	int		lp_no;
	char	str_lp_no [3];
	char	backgrd [2];
	char	backgrd_ans [4];
	char	overnight [2];
	char	overnight_ans [4];
	char 	dummy [11];
	char	systemDate [11];
	char	dflt_qty [15];
	char	rep_qty [10];
} local_rec;

static struct var vars [] =
{
	{1, LIN, "startItemNo", 4, 30, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "           ", 
		" ", " ", "Start Item No               : ", "Full search available.", 
		YES, NO, JUSTLEFT, "", "", local_rec.startItemNo}, 
	{1, LIN, "st_item_desc", 5, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		" ", " ", "Description                 : ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.st_item_desc}, 
	{1, LIN, "endItemNo", 7, 30, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "           ", 
		" ", "~~~~~~~~~~~~~~~~", "End Item No                 : ", "Full search available.", 
		YES, NO, JUSTLEFT, "", "", local_rec.endItemNo}, 
	{1, LIN, "ed_item_desc", 8, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "           ", 
		" ", " ", "Description                 : ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.ed_item_desc}, 
	{1, LIN, "cobrwh", 10, 30, CHARTYPE, 
		"U", "          ", 
		" ", "C", "C(ompany/B(ranch/W(arehouse : ", "C(ompany)  B(ranch)  W(arehouse) - Default to C(ompany", 
		YES, NO, JUSTLEFT, "BCW", "", local_rec.co_br_wh}, 
	{1, LIN, "cobrwh_desc", 10, 34, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		"", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.cobrwh_desc}, 
	{1, LIN, "lp_no", 12, 30, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No                  : ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lp_no}, 
	{1, LIN, "bk_grd", 13, 30, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background                  : ", "Enter Yes or No", 
		YES, NO, JUSTLEFT, "", "", local_rec.backgrd}, 
	{1, LIN, "bk_grd_ans", 13, 34, CHARTYPE, 
		"AAA", "          ", 
		"", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backgrd_ans}, 
	{1, LIN, "o_night", 14, 30, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight                   : ", "Enter Yes or No", 
		YES, NO, JUSTLEFT, "", "", local_rec.overnight}, 
	{1, LIN, "o_night_ans", 14, 34, CHARTYPE, 
		"AAA", "          ", 
		"", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.overnight_ans}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
void 	RunProg 		 (char *);
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
int  	spec_valid 		 (int);
void 	ProcInmr 		 (void);
int  	ValidCcmr 		 (void);
int  	ReadIncc 		 (void);
void 	ProcInlo 		 (void);
int  	heading 		 (int);
void 	InitOutput 		 (void);
void 	PrintHeading 	 (void);
int  	check_page 		 (void);

			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		before,
			after;

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);

	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	SETUP_SCR (vars);

	OpenDB ();

	if (argc > 1)
	{
		if (argc < 5)
		{
			print_at (0,0,mlSkMess509,argv [0]);
			return (EXIT_FAILURE);
		}

		if (strncmp (argv [1], "                ", 1))
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			sprintf (inmr_rec.item_no, "%-16.16s", argv [1]);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, (char *)inmr, "DBFIND");
			strcpy (local_rec.startItemNo, inmr_rec.item_no);
		}
		else
			strcpy (local_rec.startItemNo, argv [1]);

		if (strncmp (argv [2], "~~~~~~~~~~~~~~~~", 16))
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			sprintf (inmr_rec.item_no, "%-16.16s", argv [2]);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, (char *)inmr, "DBFIND");
			strcpy (local_rec.endItemNo, inmr_rec.item_no);
		}
		else
			strcpy (local_rec.endItemNo, argv [2]);

		switch (argv [3] [0])
		{
			case 'C' :
				strcpy (local_rec.co_br_wh, argv [3]);
				strcpy (local_rec.cobrwh_desc, "COMPANY");
				break;
			case 'B' :
				strcpy (local_rec.co_br_wh, argv [3]);
				strcpy (local_rec.cobrwh_desc, "BRANCH");
				break;
			case 'W' :
				strcpy (local_rec.co_br_wh, argv [3]);
				strcpy (local_rec.cobrwh_desc, "WAREHOUSE");
				break;
			default :
				print_at (0,0,mlSkMess509,argv [0]);
				return (EXIT_FAILURE);
		}

		local_rec.lp_no = atoi (argv [4]);

		/*-----------------------------
		| Process Records in Database.|
		-----------------------------*/
		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcInmr ();

		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

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
	
		RunProg (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProg (
 char *progname)
{
	sprintf (local_rec.str_lp_no, "%2d", local_rec.lp_no);
	strcpy (err_str, ML (mlSkMess486));
	
	shutdown_prog ();

	if (local_rec.overnight [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp 
			(
				"ONIGHT",
				"ONIGHT",
				progname,
				local_rec.startItemNo,
				local_rec.endItemNo,
				local_rec.co_br_wh,
				local_rec.str_lp_no,
				err_str, (char *)0
			);
		}
	}
	else if (local_rec.backgrd [0] == 'Y')
	{
		if (fork () == 0)
			execlp 
			(
				progname,
				progname,
				local_rec.startItemNo,
				local_rec.endItemNo,
				local_rec.co_br_wh,
				local_rec.str_lp_no,
				 (char *)0
			 );
	}
	else 
	{
		execlp 
		(
			progname,
			progname,
			local_rec.startItemNo,
			local_rec.endItemNo,
			local_rec.co_br_wh,
			local_rec.str_lp_no,
			 (char *)0
		 );
	}
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (ccmr);
	abc_fclose (incc);
	abc_fclose (inlo);
	CloseLocation ();
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startItemNo") || LCHECK ("endItemNo")) 
	{
		if (dflt_used)
		{
			if (LCHECK ("startItemNo"))
			{
				strcpy (local_rec.st_item_desc, "First Item");
				DSP_FLD ("st_item_desc");
			}
			else
			{
				strcpy (local_rec.ed_item_desc, "Last Item");
				DSP_FLD ("ed_item_desc");
			}
			return (EXIT_SUCCESS);
		}
		
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if ((LCHECK ("endItemNo") || prog_status != ENTRY) &&
			strncmp (local_rec.startItemNo, local_rec.endItemNo, 16) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		clear_mess ();
	
		if (LCHECK ("startItemNo"))
		{
			cc = FindInmr (comm_rec.co_no, local_rec.startItemNo, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.startItemNo);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
		}
		else
		{
			cc = FindInmr (comm_rec.co_no, local_rec.endItemNo, 0L, "N");
			if (!cc)
			{
				strcpy (inmr_rec.co_no, comm_rec.co_no);
				strcpy (inmr_rec.item_no, local_rec.endItemNo);
				cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			}
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		if (LCHECK ("startItemNo"))
		{
			strcpy (local_rec.st_item_desc, inmr_rec.description);
			DSP_FLD ("st_item_desc");
		}
		else
		{
			strcpy (local_rec.ed_item_desc, inmr_rec.description);
			DSP_FLD ("ed_item_desc");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cobrwh"))
	{
		DSP_FLD ("cobrwh_desc");
		switch (local_rec.co_br_wh [0])
		{
		case	'C':
			strcpy (local_rec.cobrwh_desc, "Company  ");
			break;
		case	'B':
			strcpy (local_rec.cobrwh_desc, "Branch   ");
			break;
		case	'W':
			strcpy (local_rec.cobrwh_desc, "Warehouse");
			break;
		}
		DSP_FLD ("cobrwh_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lp_no"))
	{
		if (FLD ("lp_no") != NA) 
		{
			if (SRCH_KEY)
			{
				local_rec.lp_no = get_lpno (0);
				return (EXIT_SUCCESS);
			}
	
			if (!valid_lp (local_rec.lp_no))
			{
				print_mess (ML (mlStdMess020));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bk_grd"))
	{
		strcpy (local_rec.backgrd_ans, " ");
		DSP_FLD ("bk_grd_ans");
		if (local_rec.backgrd [0] == 'Y')
			strcpy (local_rec.backgrd_ans, "Yes");
		else
			strcpy (local_rec.backgrd_ans, "No ");
		DSP_FLD ("bk_grd_ans");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("o_night"))
	{
		strcpy (local_rec.overnight_ans, " ");
		DSP_FLD ("o_night_ans");
		if (local_rec.overnight [0] == 'Y')
			strcpy (local_rec.overnight_ans, "Yes");
		else
			strcpy (local_rec.overnight_ans, "No ");
		DSP_FLD ("o_night_ans");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcInmr (
 void)
{
	int		recordsFoundBin = FALSE;
	int 	breakFlagBin = FALSE;

	PrintHeading ();

	/*----------------------------------------
	| read each inmr record for this company |
	----------------------------------------*/
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItemNo);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc)
	{
		/*----------------------------------
		| Process inmr (item records) only |
		| if the record is within the      |
		| required selection.              |
		----------------------------------*/
		if ((strncmp (local_rec.startItemNo, inmr_rec.item_no, 16) <= 0) &&
			 (strncmp (inmr_rec.item_no, local_rec.endItemNo, 16) <= 0))
		{
			for (loopCnt = 0; loopCnt < MAXREC; loopCnt++)
			{
				strcpy (storeTotalBin [loopCnt].est_no, 	" ");
				strcpy (storeTotalBin [loopCnt].cc_no, 	" ");
				storeTotalBin [loopCnt].on_hand 	= 0.00;

				*store_bin [loopCnt].bin_loc 	= '\0';
				*store_bin [loopCnt].lot_no 	= '\0';
				*store_bin [loopCnt].est_no 	= '\0';
				*store_bin [loopCnt].cc_no 		= '\0';
				store_bin [loopCnt].qty 		= 0.00;
			}
			recCountBin 	= 0;
			totalCntBin 	= 0;
			breakFlagBin 	= FALSE;
			recordsFoundBin = FALSE;

			/*-------------------------------
			| set key for ccmr record read. |
			-------------------------------*/
			if (local_rec.co_br_wh [0] == 'C')
			{
				strcpy (ccmr_rec.co_no, comm_rec.co_no);
				abc_selfield (ccmr, "ccmr_co_no");
			}
			else
			{
				if (local_rec.co_br_wh [0] == 'B')
				{
					strcpy (ccmr_rec.co_no, comm_rec.co_no);
					strcpy (ccmr_rec.est_no, comm_rec.est_no);
					*ccmr_rec.cc_no = '\0';
				}
				else
				{
					strcpy (ccmr_rec.co_no, comm_rec.co_no);
					strcpy (ccmr_rec.est_no, comm_rec.est_no);
					strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
				}
				abc_selfield (ccmr, "ccmr_id_no");
			}

			cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
			while (!cc && !ValidCcmr ()) 
			{
				if (!ReadIncc ())
				{
					if (MULT_LOC || SK_BATCH_CONT)
					{
						inlo_rec.hhwh_hash 		= 	incc_rec.hhwh_hash;
						inlo_rec.hhum_hash		=	0L;
						inlo_rec.loc_type [0]	=	' ';
						strcpy (inlo_rec.location, "          ");
						cc = find_rec ("inlo", &inlo_rec, GTEQ, "r");
						if (!cc)
						{
							strncpy (storeTotalBin [totalCntBin].est_no, ccmr_rec.est_no, 2);
							strncpy (storeTotalBin [totalCntBin].cc_no, ccmr_rec.cc_no, 2);
							storeTotalBin [totalCntBin].on_hand = 
								no_dec (incc_rec.closing_stock);
							totalCntBin ++;
						}
						while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
						{
							if (!recordsFoundBin)
								recordsFoundBin = TRUE;
	
							/*--------------------------------------------
							| load inlo details into table to be printed |
							--------------------------------------------*/
							strncpy (store_bin [recCountBin].bin_loc, inlo_rec.location, 10);
							strncpy (store_bin [recCountBin].lot_no, inlo_rec.lot_no, 7);
							store_bin [recCountBin].qty = (float) (no_dec (inlo_rec.qty));
	
							strcpy (store_bin [recCountBin].est_no, ccmr_rec.est_no);
							strcpy (store_bin [recCountBin].cc_no, ccmr_rec.cc_no);
	
							recCountBin ++;
	
							if (recCountBin >= MAXREC)
							{
								breakFlagBin = TRUE;
								break;
							}

							cc = find_rec ("inlo", &inlo_rec, NEXT, "r");
						}
					}
				}
				cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
			}

			if (recordsFoundBin)
				ProcInlo ();

			if (breakFlagBin)
			{
				fprintf (fout, "| BIN LOCATION RECORDS HAVE EXCEEDED 100");
				fprintf (fout, "                             ");
				fprintf (fout, "                             ");
				if (!MULT_LOC)
					fprintf (fout, "|\n");
				else
					fprintf (fout, "                           |\n");
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	/*------------------------------
	| nothing printed so tell user |
	------------------------------*/
	if (!printed)
	{
		if (MULT_LOC)
			fprintf (fout, ".ENO LOT & BIN LOCATION DISCREPANCIES FOUND\n");
		else
			fprintf (fout, ".ENO LOT DISCREPANCIES FOUND\n");
	}

	/*---------------------
	| print rule off line |
	---------------------*/
	fprintf (fout, "==================================================");
	fprintf (fout, "==================================================");
	if (!MULT_LOC)
		fprintf (fout, "\n");
	else
		fprintf (fout, "===========================\n");

}

int
ValidCcmr (
 void)
{
	if (local_rec.co_br_wh [0] == 'C' && 
		!strncmp (ccmr_rec.co_no, comm_rec.co_no, 2))
		return (EXIT_SUCCESS);

	if (local_rec.co_br_wh [0] == 'B' && 
		!strncmp (ccmr_rec.co_no, comm_rec.co_no, 2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.est_no, 2))
		return (EXIT_SUCCESS);

	if (local_rec.co_br_wh [0] == 'W' && 
		!strncmp (ccmr_rec.co_no, comm_rec.co_no, 2) &&
		!strncmp (ccmr_rec.est_no, comm_rec.est_no, 2) &&
		!strncmp (ccmr_rec.cc_no, comm_rec.cc_no, 2))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
ReadIncc (
 void)
{
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;

	cc = find_rec (incc, &incc_rec, COMPARISON, "r");

	return (cc);
}

/*===============================================================
| print records only if there is a difference between the bin   |
| quantities and the on hand quantities. If something is    	|
| printed set 'printed' to TRUE else set it to FALSE.          	|
===============================================================*/
void
ProcInlo (
 void)
{
	int		count;
	float	whRemainingQty 	= 0.00,
			brRemainingQty 	= 0.00,
			coRemainingQty 	= 0.00,
			onHandTotal 	= 0.00,
			binTotal 		= 0.00,
			totalEst		= 0.00;
	char	tmp_est [3],
			tmp_cc [3];

	loopCnt	= 0;
	strcpy (tmp_est, storeTotalBin [loopCnt].est_no);
	strcpy (tmp_cc, storeTotalBin [loopCnt].cc_no);

	/*---------------------------------------
	| total all storeTotalBin on_hand qty's |
	---------------------------------------*/
	for (loopCnt = 0; loopCnt < totalCntBin; loopCnt ++)
		onHandTotal += storeTotalBin [loopCnt].on_hand;

	/*--------------------------------------------------------------------
	| total all store lot quantities to be compared with the on_hand qty |
	--------------------------------------------------------------------*/
	for (loopCnt = 0; loopCnt < recCountBin; loopCnt ++)
		binTotal += store_bin [loopCnt].qty;

	if (onHandTotal != binTotal)
	{
		if (!printed)
			printed = TRUE;

		dsp_process ("Item No :", inmr_rec.item_no);

		loopCnt = 0;
		strcpy (tmp_est, store_bin [loopCnt].est_no);
		strcpy (tmp_cc, store_bin [loopCnt].cc_no);

		/*---------------------------
		| print details of item etc |
		---------------------------*/
		fprintf (fout, "|-------------------------------------------");
		fprintf (fout, "--------------------------------------------");
		fprintf (fout, "--------------------------------------------|\n");
		fprintf (fout, "| %-16.16s | %-40.40s |",
			inmr_rec.item_no,
			inmr_rec.description);
		fprintf (fout, " %-2.2s | %-2.2s ",
			store_bin [loopCnt].est_no,
			store_bin [loopCnt].cc_no);
		sprintf (err_str, local_rec.rep_qty, store_bin [loopCnt].qty);
		fprintf (fout, "|  %-10.10s  |%14s",
			store_bin [loopCnt].lot_no, err_str);

		sprintf (err_str, local_rec.rep_qty, store_bin [loopCnt].qty);
		fprintf (fout, "|  %-10.10s  |%14s|\n",
			store_bin [loopCnt].bin_loc, err_str);

		whRemainingQty += store_bin [loopCnt].qty;
		brRemainingQty += store_bin [loopCnt].qty;
		coRemainingQty += store_bin [loopCnt].qty;

		/*---------------------------------------
		| print more inbc records (bin details) |
		---------------------------------------*/
		for (loopCnt = 1; loopCnt < recCountBin; loopCnt ++)
		{
			if (strncmp (tmp_est, store_bin [loopCnt].est_no, 2) ||
				strncmp (tmp_cc, store_bin [loopCnt].cc_no, 2))
			{
				/*---------------------------------------------------
				| print on hand qty for the warehouse and/or branch |
				---------------------------------------------------*/
				for (count = 0; count < totalCntBin; count ++)
				{
					if (!strncmp (tmp_est, storeTotalBin [count].est_no, 2) &&
						!strncmp (tmp_cc, storeTotalBin [count].cc_no, 2))
						break;
				}

				fprintf (fout, "|                                   ");
				fprintf (fout, "                                    ");
				fprintf (fout, "                              ");
				fprintf (fout, "|--------------|--------------|\n");

				fprintf (fout, "|                                   ");
				fprintf (fout, "              Warehouse On Hand Qty ");
				fprintf (fout, ":                             ");
				sprintf (err_str, local_rec.rep_qty, whRemainingQty);
				sprintf (QtyStr, local_rec.rep_qty,
						storeTotalBin [count].on_hand);
				fprintf (fout, "|%14s|%14s|\n", QtyStr, err_str);
	
				whRemainingQty = 0.00;

				totalEst = 0.00;
				if (local_rec.co_br_wh [0] != 'W' && 
					strncmp (tmp_est, store_bin [loopCnt].est_no, 2))
				{
					for (count = 0; count < totalCntBin; count++)
					{
						if (!strncmp (tmp_est, storeTotalBin [count].est_no, 2))
							totalEst += storeTotalBin [count].on_hand;
					}
					fprintf (fout, "|                                   ");
					fprintf (fout, "                                    ");
					fprintf (fout, "                              ");
					fprintf (fout, "|--------------|--------------|\n");

					fprintf (fout, "|                                   ");
					fprintf (fout, "                 Branch On Hand Qty ");
					fprintf (fout, ":                             ");
					sprintf (err_str, local_rec.rep_qty, brRemainingQty);
					sprintf (QtyStr, local_rec.rep_qty, totalEst);
					fprintf (fout, "|%14s|%14s|\n", QtyStr, err_str);
					brRemainingQty = 0.00;
				}

				fprintf (fout, "|-------------------------------------------");
				fprintf (fout, "--------------------------------------------");
				fprintf (fout, "--------------------------------------------|\n");
				fprintf (fout, "| %-16.16s | %-40.40s |",
					inmr_rec.item_no,
					inmr_rec.description);
				fprintf (fout, " %-2.2s | %-2.2s ",
					store_bin [loopCnt].est_no,
					store_bin [loopCnt].cc_no);
				sprintf (err_str, local_rec.rep_qty, store_bin [loopCnt].qty);
				fprintf (fout, "|  %-10.10s  |%14s",
					store_bin [loopCnt].lot_no, err_str);
				sprintf (err_str, local_rec.rep_qty, store_bin [loopCnt].qty);
				fprintf (fout, "|  %-10.10s  |%14s|\n",
					store_bin [loopCnt].bin_loc, err_str);
			}
			else
			{
				sprintf (err_str, local_rec.rep_qty, store_bin [loopCnt].qty);
				fprintf (fout, "| %16.16s | %40.40s |    |    "," "," ");
				fprintf (fout, "|  %-10.10s  |%14s|  %-10.10s  |%14s|\n",
					store_bin [loopCnt].lot_no, err_str,
					store_bin [loopCnt].bin_loc, err_str);
			}

			whRemainingQty += store_bin [loopCnt].qty;
			brRemainingQty += store_bin [loopCnt].qty;
			coRemainingQty += store_bin [loopCnt].qty;

			strcpy (tmp_est, store_bin [loopCnt].est_no);
			strcpy (tmp_cc, store_bin [loopCnt].cc_no);
		}

		/*-------------------------------------
		| print on hand qty for the warehouse |
		| branch and company                  |
		-------------------------------------*/
		for (count = 0; count < totalCntBin; count ++)
		{
			if (!strncmp (tmp_est, storeTotalBin [count].est_no, 2) &&
				!strncmp (tmp_cc, storeTotalBin [count].cc_no, 2))
				break;
		}

		fprintf (fout, "|                                   ");
		fprintf (fout, "                                    ");
		fprintf (fout, "                              ");
		fprintf (fout, "|--------------|--------------|\n");
		
		fprintf (fout, "|                                   ");
		fprintf (fout, "              Warehouse On Hand Qty ");
		fprintf (fout, ":                             ");
		sprintf (err_str, local_rec.rep_qty, whRemainingQty);
		sprintf (QtyStr, local_rec.rep_qty, storeTotalBin [count].on_hand);
		fprintf (fout, "|%14s|%14s|\n", QtyStr, err_str);
		whRemainingQty = 0.00;

		totalEst = 0.00;
		if (local_rec.co_br_wh [0] != 'W')
		{
			for (count = 0; count < totalCntBin; count++)
			{
				if (!strncmp (tmp_est, storeTotalBin [count].est_no, 2))
					totalEst += storeTotalBin [count].on_hand;
			}
			fprintf (fout, "|                                   ");
			fprintf (fout, "                                    ");
			fprintf (fout, "                              ");
			fprintf (fout, "|--------------|--------------|\n");

			fprintf (fout, "|                                   ");
			fprintf (fout, "                 Branch On Hand Qty ");
			fprintf (fout, ":                             ");
			sprintf (err_str, local_rec.rep_qty, brRemainingQty);
			sprintf (QtyStr, local_rec.rep_qty, totalEst);
			fprintf (fout, "|%14s|%14s|\n", QtyStr, err_str);
			brRemainingQty = 0.00;
		}

		if (local_rec.co_br_wh [0] == 'C')
		{
			/*-----------------------------------------------
			| print total of bin quantities and on hand qty |
			-----------------------------------------------*/
			fprintf (fout, "|                                   ");
			fprintf (fout, "                                    ");
			fprintf (fout, "                              ");
			fprintf (fout, "|--------------|--------------|\n");

			fprintf (fout, "|                                   ");
			fprintf (fout, "                Company On Hand Qty ");
			fprintf (fout, ":                             ");
			sprintf (err_str, local_rec.rep_qty, coRemainingQty);
			sprintf (QtyStr, local_rec.rep_qty, onHandTotal);
			fprintf (fout, "|%14s|%14s|\n", QtyStr, err_str);
		}
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
		
		if (!MULT_LOC)
			rv_pr (ML (mlSkMess486), 24, 0, 1);
		else
			rv_pr (ML (mlSkMess487),17 ,0 ,1);
		move (0, 1);
		line (80);

		box (0, 3, 80, 11);
		move (1, 6);
		line (79);
		move (1, 9);
		line (79);
		move (1, 11);
		line (79);

		move (0, 20);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,clip (comm_rec.co_short));
		strcpy (err_str,ML (mlStdMess039));
		print_at (21,30,err_str,comm_rec.est_no,clip (comm_rec.est_short));
		strcpy (err_str,ML (mlStdMess099));
		print_at (21,60,err_str,comm_rec.cc_no,clip (comm_rec.cc_short));
		move (0, 22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*==========================================
| Initialize for Screen or Printer Output. |
==========================================*/
void
InitOutput (
 void)
{
	if (MULT_LOC)
	{
		dsp_screen (" Printing Lot & Bin Location Quantities Discrepancies Report",
				comm_rec.co_no, comm_rec.co_name);
	}
	else
	{
		dsp_screen (" Printing Lot Quantities Discrepancies Report",
				comm_rec.co_no, comm_rec.co_name);
	}
	/*----------------------
	| Open pipe to pformat | 
	----------------------*/
	if ((fout = popen ("pformat", "w")) == NULL)
		file_err (cc, "pformat", "POPEN");

	/*---------------------------------
	| Initialize printer for output.  |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lp_no);
	fprintf (fout, ".PI12\n");
	if (!MULT_LOC)
		fprintf (fout, ".L115\n");
	else
		fprintf (fout, ".L133\n");
}

/*==============================
| Headings for printed output. |
==============================*/
void
PrintHeading (
 void)
{
	if (!MULT_LOC)
		fprintf (fout, ".ELOT QUANTITIES DISCREPANCIES REPORT\n");
	else
		fprintf (fout, ".ELOT & BIN LOCATION QUANTITIES DISCREPANCIES REPORT\n");

	fprintf (fout, ".ECOMPANY NAME : %s\n",
		clip (comm_rec.co_name));

	fprintf (fout, ".ESTART ITEM : %s   END ITEM : %s\n",
		local_rec.startItemNo,
		local_rec.endItemNo);

	fprintf (fout, ".ELEVEL : %s\n",
  		local_rec.cobrwh_desc);

	fprintf (fout, ".B1\n");

	if (MULT_LOC)
		fprintf (fout, "==============================");
	fprintf (fout, "===================================");
	fprintf (fout, "==================================");
	fprintf (fout, "==================================\n");

	fprintf (fout, "|   I T E M  N O   |");
	fprintf (fout, "      I T E M  D E S C R I P T I O N      |");
	fprintf (fout, " BR | WH |    LOT NO    |  REMAINING   |");
	if (!MULT_LOC)
		fprintf (fout, "\n");
	else
		fprintf (fout, "   LOCATION   |  REMAINING   |\n");

	fprintf (fout, "|                  |");
	fprintf (fout, "                                          |");
	fprintf (fout, " NO | NO |              |     QTY      |");
	if (!MULT_LOC)
		fprintf (fout, "\n");
	else
		fprintf (fout, "              |     QTY      |\n");
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

