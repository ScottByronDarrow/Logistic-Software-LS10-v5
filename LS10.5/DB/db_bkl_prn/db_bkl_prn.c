/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_bkl_prn.c,v 5.4 2002/07/17 09:57:04 scott Exp $
|  Program Name  : (db_bkl_prn.c)
|  Program Desc  : (Print Bank lodgement forms)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 28/08/86         |
|---------------------------------------------------------------------|
| $Log: db_bkl_prn.c,v $
| Revision 5.4  2002/07/17 09:57:04  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/19 02:37:02  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_bkl_prn.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_bkl_prn/db_bkl_prn.c,v 5.4 2002/07/17 09:57:04 scott Exp $";

#include	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include 	<arralloc.h>

	/*
	 * The Following are needed for branding Routines.
	 */
	int		envDbCo = 0;
	char	branchNo [3];

#include	"schema"

struct commRecord	comm_rec;
struct blhdRecord	blhd_rec;
struct bldtRecord	bldt_rec;
struct bldtRecord	bldt2_rec;
struct crbkRecord	crbk_rec;
struct pocrRecord	pocrRec;
struct cuhdRecord	cuhd_rec;
struct esmrRecord	esmr_rec;

	char	*data  = "data",
			*bldt2  = "bldt2";

	int		currentLodgement;

	double 	totalReceipt 	= 0.00,
			totalBankCharge = 0.00,
			totalChequeFees = 0.00,
			totalLodgement 	= 0.00,
			totalCash 		= 0.00,
			totalCheque 	= 0.00;

	FILE	*pp;

	extern		int		TruePosition;

/*
 *	Structure for dynamic array,  for the chequeRec lines for qsort
 */
struct ChequeStruct
{
	char	cheque [9];
	long	bldtHash;
}	*chequeRec;
	DArray cheque_details;
	int	chequeCnt = 0;


/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	bank_id [6];
	char	bank_desc [41];
	long	lodge_no;
	char	currentLodgement [8];
	char	clr_lodge [2];
	char	print [2];
	int		printerNo;
} local_rec;

static struct	var vars [] ={
	{1, LIN, "bank_id",	4,	2, CHARTYPE,
		"UUUUU","          ",
		" ", " ", "Bank Code        ","Enter Bank Code.",
		NE, NO, JUSTLEFT, "", "", local_rec.bank_id},
	{1, LIN, "bank_desc",	4,	30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ", "", "","",
		NA, NO, JUSTLEFT, "", "", local_rec.bank_desc},
	{1, LIN, "lodge_no",	5,	2, LONGTYPE,
		"NNNNNNNNNN","          ",
		" ", "",  "Lodgement No.    ","Enter Lodgement Number. Default for current lodgement.",
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.lodge_no},
	{1, LIN, "currentLodgement",	5,	30, CHARTYPE,
		"AAAAAAA","          ",
		" ", "", "","",
		NA, NO, JUSTLEFT, "", "", local_rec.currentLodgement},
	{1, LIN, "clr_lodge",	7,	2, CHARTYPE,
		"U","          ",
		" ","N",  "Close Lodgement  ","Close Lodgement (Y/N).",
		NO, NO, JUSTLEFT, "YN", "", local_rec.clr_lodge},
	{1, LIN, "print",	9,	2, CHARTYPE,
		"U","          ",
		" "," ", " Print Lodgement  ","Print Lodgement (Y/N).",
		NO, NO, JUSTLEFT, "YN", "", local_rec.print},
	{1,LIN,"printerNo",		10,	2, INTTYPE,
		"NN","          ",
		" ","1", "Printer No       ","",
		NO, NO, JUSTLEFT, "", "", (char *)&local_rec.printerNo},

	{0,LIN,"",0,0,INTTYPE,
		"A","          ",
		" ","", "dummy"," ",
		YES,NO, JUSTRIGHT," "," ",local_rec.dummy}
};

/*
 * Local Function Prototypes.
 */
int		ChequeSort			 (const	void *,	const void *);
int 	spec_valid 			 (int);
int 	SrchBldt 			 (char *);
int 	PrintLine 			 (char *);
int 	heading 			 (int);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	shutdown_prog 		 (void);
void 	SrchCrbk 			 (char *);
void 	Process 			 (void);
void 	ReportHeading 		 (void);
void 	ClosePrint 			 (void);

/*
 * Main processing routine.
 */
int
main (
 int argc,
 char* argv [])
{
	TruePosition	=	TRUE;

	/*
	 * Open database.
	 */
	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	SETUP_SCR (vars);

	envDbCo = atoi (get_env ("DB_CO"));

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	/*
	 * Main control loop.
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		abc_unlock (blhd);

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Process ();
		prog_exit = TRUE;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Open Database Files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (bldt2, bldt);

	open_rec (blhd, blhd_list, BLHD_NO_FIELDS, "blhd_id_no");
	open_rec (bldt, bldt_list, BLDT_NO_FIELDS, "bldt_id_no");
	open_rec (bldt2,bldt_list, BLDT_NO_FIELDS, "bldt_bldt_hash");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (blhd);
	abc_fclose (bldt);
	abc_fclose (bldt2);
	abc_fclose (crbk);
	abc_fclose (cuhd);
	abc_fclose (pocr);
	abc_fclose (esmr);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	FinishProgram ();
}

int
spec_valid (int field)
{
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

        if (dflt_used)                              
        {
            strcpy (esmr_rec.co_no, comm_rec.co_no);
            strcpy (esmr_rec.est_no, comm_rec.est_no);
            cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
            if (!cc)                                                 
                strcpy (local_rec.bank_id, esmr_rec.dflt_bank);
        }   

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		sprintf (crbk_rec.bank_id, "%-5.5s", local_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess043));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Find lodgement header.
		 */
		abc_unlock (blhd);
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, local_rec.bank_id);
		cc = find_rec (blhd, &blhd_rec, COMPARISON, "w");
		if (cc)
		{
			print_mess (ML (mlStdMess078));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		FLD ("clr_lodge") = NO;
		sprintf (local_rec.bank_desc, "%-40.40s", crbk_rec.bank_name);
		DSP_FLD ("bank_desc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lodgement number.
	 */
	if (LCHECK ("lodge_no"))
	{
		if (SRCH_KEY)
		{
			SrchBldt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/*
			 * Check for lodgement number on file.
			 */
			bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
			bldt_rec.lodge_no = blhd_rec.nx_lodge_no;
			strcpy (bldt_rec.rec_type, " ");
			cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
			if (cc ||
				bldt_rec.hhbl_hash != blhd_rec.hhbl_hash ||
				bldt_rec.lodge_no != blhd_rec.nx_lodge_no)
			{
				print_mess (ML (mlDbMess141));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			strcpy (local_rec.currentLodgement, "Current");
			DSP_FLD ("currentLodgement");
			currentLodgement = TRUE;

			FLD ("clr_lodge") = NO;
			return (EXIT_SUCCESS);
		}
		currentLodgement = FALSE;
		strcpy (local_rec.currentLodgement, "       ");
		strcpy (local_rec.clr_lodge, "N");
		DSP_FLD ("clr_lodge");
		FLD ("clr_lodge") = NA;

		/*
		 * Check for lodgement number on file.
		 */
		bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
		bldt_rec.lodge_no = local_rec.lodge_no;
		strcpy (bldt_rec.rec_type, " ");
		cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
		if (cc ||
			bldt_rec.hhbl_hash != blhd_rec.hhbl_hash ||
			bldt_rec.lodge_no != local_rec.lodge_no)
		{
			print_mess (ML (mlStdMess117));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("currentLodgement");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("print"))
	{
		if (dflt_used)
		{
			if (!strcmp (local_rec.clr_lodge, "Y"))
				strcpy (local_rec.print, "N");
			else
				strcpy (local_rec.print, "Y");
		}

		if (!strcmp (local_rec.print, "N"))
		{
			local_rec.printerNo = 0;
			DSP_FLD ("printerNo");
			FLD ("printerNo") = NA;
		}
		else
			FLD ("printerNo") = YES;
	}

	if (LCHECK ("printerNo"))
	{
		if (FLD ("printerNo") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCrbk (
 char*              key_val)
{
	_work_open (5,0,40);
	save_rec ("#Bank", "#Bank Name");

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id, key_val, strlen (key_val)) &&
		   !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc)
		file_err (cc, crbk, "DBFIND");
}

int
SrchBldt (
	char	*keyValue)
{
	char lodgeStr [11];
	long prevLodge = -1;

	_work_open (10,0,40);
	save_rec ("#Lodgement", "#Bank");

	bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
	bldt_rec.lodge_no = 0;
	strcpy (bldt_rec.rec_type, " ");
	for (cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
		 !cc &&
		 bldt_rec.hhbl_hash == blhd_rec.hhbl_hash;
		 cc = find_rec (bldt, &bldt_rec, NEXT, "r"))
	{
		if (bldt_rec.lodge_no != prevLodge)
		{
			sprintf (lodgeStr, "%-10ld", bldt_rec.lodge_no);
			cc = save_rec (lodgeStr, blhd_rec.bank_id);
			if (cc)
				break;
			prevLodge = bldt_rec.lodge_no;
		}
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_FAILURE);

	local_rec.lodge_no = atol (temp_str);
	bldt_rec.hhbl_hash = blhd_rec.hhbl_hash;
	bldt_rec.lodge_no = local_rec.lodge_no;
	strcpy (bldt_rec.rec_type, " ");
	cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
	if (cc ||
		bldt_rec.hhbl_hash != blhd_rec.hhbl_hash ||
		bldt_rec.lodge_no != local_rec.lodge_no)
	{
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Process Lodgements.
 */
void
Process (void)
{
	int 	firstTime	=	0,
			i			=	0,
			printed		= 	0;

	char 	receipt [9],
		 	prevReceipt [9];

	/*
	 * Current lodgement number is the next lodgement from blhd.      
	 */
	if (currentLodgement)
		local_rec.lodge_no = blhd_rec.nx_lodge_no;

	/*
	 * Close the lodgement.
	 */
	if (local_rec.clr_lodge [0] == 'Y')
	{
		blhd_rec.nx_lodge_no++;
		cc = abc_update (blhd, &blhd_rec);
		if (cc)
			file_err (cc, blhd, "DBUPDATE");
	}
	abc_unlock (blhd);

	dsp_screen ("Sorting Bank Lodgements.",
				comm_rec.co_no,
				comm_rec.co_name);

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&cheque_details, &chequeRec,sizeof (struct ChequeStruct),10);
	chequeCnt = 0;

	bldt_rec.hhbl_hash 	= blhd_rec.hhbl_hash;
	bldt_rec.lodge_no 	= local_rec.lodge_no;
	strcpy (bldt_rec.rec_type, " ");
	cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
	for (cc = find_rec (bldt, &bldt_rec, GTEQ, "r");
		 !cc &&
		  bldt_rec.hhbl_hash == blhd_rec.hhbl_hash &&
		  bldt_rec.lodge_no == local_rec.lodge_no;
	     cc = find_rec (bldt, &bldt_rec, NEXT, "r"))
	{
		if (!strcmp (bldt_rec.rec_type, "I"))
			continue;

		if (bldt_rec.hhcp_hash)
		{
			cuhd_rec.hhcp_hash	=	bldt_rec.hhcp_hash;
			/*
		     * Find receipt.
		     */
			cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
			if (cc)
			{
				abc_unlock (bldt);
				continue;
			}
		}

		dsp_process ("Receipt : ", (bldt_rec.hhcp_hash)	? cuhd_rec.receipt_no
					 									: bldt_rec.sundry_rec);
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&cheque_details, chequeRec, chequeCnt))
			sys_err ("ArrChkLimit (chequeRec)", ENOMEM, PNAME);

		sprintf 
		 (
			chequeRec [chequeCnt].cheque, 
			"%-8.8s", 
			 (bldt_rec.hhcp_hash) ? cuhd_rec.receipt_no : bldt_rec.sundry_rec
		);
		chequeRec [i].bldtHash 	= bldt_rec.bldt_hash;
		chequeCnt++;
	}


	if (!strcmp (local_rec.print, "Y"))
	{
		ReportHeading ();

		dsp_screen ("Printing Bank Lodgement Form.",
					comm_rec.co_no,
					comm_rec.co_name);
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (chequeRec, chequeCnt, sizeof (struct ChequeStruct), ChequeSort);

	printed = TRUE;
	firstTime = TRUE;
	strcpy (prevReceipt, "        ");

	for (i = 0; i < chequeCnt; i++)
	{
		sprintf (receipt, "%-8.8s", chequeRec [i].cheque);
		bldt2_rec.bldt_hash	=	chequeRec [i].bldtHash;

		cc = find_rec (bldt2, &bldt_rec, EQUAL, "u");
		if (cc)
			file_err (cc, bldt2, "DBFIND");

		if (bldt_rec.hhcp_hash)
		{
			/*
		     * Find receipt.
		     */
			cuhd_rec.hhcp_hash	=	bldt_rec.hhcp_hash;
			cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
			if (cc)
			{
				abc_unlock (bldt2);
				continue;
			}

			/*
		     * Forward due draft ?
		     */
			if (bldt_rec.rec_type [0] == 'C' &&
				cuhd_rec.date_payment > local_rec.lsystemDate)
			{
				/*
				 * If we are closeing this lodegment move
				 * forward draft into next lodegment.   
				 */
				if (local_rec.clr_lodge [0] == 'Y')
				{
					bldt_rec.lodge_no = blhd_rec.nx_lodge_no;
					cc = abc_update (bldt2, &bldt_rec);
					if (cc)
						file_err (cc, bldt2, "DBUPDATE");
				}
				else
					abc_unlock (bldt2);

				continue;
			}

			bldt_rec.lodge_date = local_rec.lsystemDate;
			cc = abc_update (bldt2, &bldt_rec);
			if (cc)
				file_err (cc, bldt2, "DBUPDATE");
		}
		else
			abc_unlock (bldt2);

		/*
		 *      Print Receipt.      
		 * except for Direct Credits 
		 */
		if (bldt_rec.rec_type [0] != 'D')
		{
			if (strcmp (prevReceipt, receipt))
			{
				if (!firstTime)
				{
					dsp_process ("Receipt : ", prevReceipt);
					if (!strcmp (local_rec.print, "Y") &&
						bldt_rec.amount != 0.0)
					{
						printed = PrintLine (prevReceipt);
					}
				}
				else
					firstTime = FALSE;
				if (printed)
				{
					strcpy (prevReceipt, receipt);
					bldt2_rec = bldt_rec;
				}
				printed = 0;
			}
			else
			{
				bldt2_rec.amount += bldt_rec.amount;
				bldt2_rec.bank_chg += bldt_rec.bank_chg;
				bldt2_rec.chq_fees += bldt_rec.chq_fees;
			}
		}
	}
	if (!strcmp (local_rec.print, "Y"))
		printed = PrintLine (prevReceipt);

	abc_unlock (bldt2);

	if (!strcmp (local_rec.print, "Y"))
		ClosePrint ();

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&cheque_details);
}

/*
 * Open output.
 */
void
ReportHeading (void)
{

	if ((pp = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*
	 * Lookup currency code.
	 */
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", crbk_rec.curr_code);
	cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
	if (cc)
		sprintf (pocrRec.description, "%-40.40s", " ");
	clip (pocrRec.description);

	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp, ".NC2\n");
	fprintf (pp, ".LP%d\n", local_rec.printerNo);
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".B2\n");
	fprintf (pp, ".ELODGEMENTS JOURNAL AS AT %s\n", DateToString (comm_rec.dbt_date));
	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s\n", clip (comm_rec.co_name));
	fprintf (pp, ".B1\n");
	fprintf (pp,
			 ".C BANK : %s     CURRENCY : %s  %s\n",
			 clip (crbk_rec.bank_name),
			 crbk_rec.curr_code,
			 pocrRec.description);
	fprintf (pp, ".B1\n");
	fprintf (pp,
			 ".C LODGEMENT NUMBER : %010ld     LODGEMENT DATE : %s\n",
			 local_rec.lodge_no,
			 DateToString (comm_rec.dbt_date));

	fprintf (pp, "==========");
	fprintf (pp, "=============");
	fprintf (pp, "======================");
	fprintf (pp, "==========================");
	fprintf (pp, "================");
	fprintf (pp, "===============");
	fprintf (pp, "==============");
	fprintf (pp, "============");
	fprintf (pp, "==============================\n");

	fprintf (pp, "| REC. TYPE ");
	fprintf (pp, "|  NUMBER    ");
	fprintf (pp, "| CUSTOMER NAME                 ");
	fprintf (pp, "| BANK-BRANCH CODE        ");
	fprintf (pp, "| RECEIPT AMOUNT");
	fprintf (pp, "| BANK CHARGES ");
	fprintf (pp, "| CHEQUE FEES");
	fprintf (pp, "| DUE DATE ");
	fprintf (pp, "| LODGEMENT AMOUNT |\n");

	fprintf (pp, "|-----------");
	fprintf (pp, "|------------");
	fprintf (pp, "|-------------------------------");
	fprintf (pp, "|-------------------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|------------------|\n");

	fprintf (pp, ".R==========");
	fprintf (pp, "=============");
	fprintf (pp, "======================");
	fprintf (pp, "====================================");
	fprintf (pp, "================");
	fprintf (pp, "===============");
	fprintf (pp, "==============");
	fprintf (pp, "============");
	fprintf (pp, "====================\n");
}

/*
 * Bank Lodgement Lines.
 */
int
PrintLine (
	char	*receipt)
{
	double lodge_amt;

	if (bldt2_rec.rec_type [0] == 'I')
		return (EXIT_SUCCESS);

	if (bldt2_rec.amount == 0.00)
		return (EXIT_FAILURE);

	switch (bldt2_rec.rec_type [0])
	{
	case 'A':
		fprintf (pp, "| %-10.10s", "Cash");
		break;

	case 'C':
		fprintf (pp, "| %-10.10s", "Cheque");
		break;

	case 'B':
		fprintf (pp, "| %-10.10s", "Bank Draft");
		break;

	case 'D':
		fprintf (pp, "| %-10.10s", "Dir.Credit");
		break;

	}

	fprintf (pp, "|  %-8.8s  ", receipt);

	fprintf (pp,
			 "|%-30.30s ", bldt2_rec.dbt_name);

	fprintf (pp,
			 "|%-3.3s-%-20.20s ",
			 bldt2_rec.bank_code, bldt2_rec.branch_code);

	fprintf (pp,
			 "|%14.14s ",
			 comma_fmt (DOLLARS (bldt2_rec.amount), "NNN,NNN,NNN.NN"));

	fprintf (pp,
			 "|   %10.10s ",
			 comma_fmt (DOLLARS (bldt2_rec.bank_chg), "NNN,NNN.NN"));

	fprintf (pp,
			 "| %10.10s ",
			 comma_fmt (DOLLARS (bldt2_rec.chq_fees), "NNN,NNN.NN"));

	if (bldt2_rec.rec_type [0] == 'B')
		fprintf (pp, "|%-10.10s", DateToString (bldt2_rec.due_date));
	else
		fprintf (pp, "|%-10.10s", " ");

	lodge_amt = bldt2_rec.amount -
		bldt2_rec.bank_chg -
		bldt2_rec.chq_fees;
	fprintf (pp,
			 "|  %14.14s  |\n",
			 comma_fmt (DOLLARS (lodge_amt), "NNN,NNN,NNN.NN"));

	totalReceipt += bldt2_rec.amount;
	totalBankCharge += bldt2_rec.bank_chg;
	totalChequeFees += bldt2_rec.chq_fees;
	totalLodgement += lodge_amt;
	if (bldt2_rec.rec_type [0] == 'A')
		totalCash += bldt2_rec.amount;
	if (bldt2_rec.rec_type [0] == 'C')
		totalCheque += bldt2_rec.amount;
	return (EXIT_FAILURE);
}

/*
 * Bank Lodgement end of report routine.
 */
void
ClosePrint (void)
{
	fprintf (pp, "|-----------");
	fprintf (pp, "|------------");
	fprintf (pp, "|-------------------------------");
	fprintf (pp, "|-------------------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|------------------|\n");

	fprintf (pp, "| TOTALS    ");
	fprintf (pp, "| %-10.10s ", " ");
	fprintf (pp, "|                               ");
	fprintf (pp, "|                         ");

	fprintf (pp,
			 "|%14.14s ",
			 comma_fmt (DOLLARS (totalReceipt), "NNN,NNN,NNN.NN"));

	fprintf (pp,
			 "|   %10.10s ",
			 comma_fmt (DOLLARS (totalBankCharge), "NNN,NNN.NN"));

	fprintf (pp,
			 "| %10.10s ",
			 comma_fmt (DOLLARS (totalChequeFees), "NNN,NNN.NN"));

	fprintf (pp, "| %-8.8s ", " ");

	fprintf (pp,
			 "|  %14.14s  |\n",
			 comma_fmt (DOLLARS (totalLodgement), "NNN,NNN,NNN.NN"));

	fprintf (pp, "|-----------");
	fprintf (pp, "-------------");
	fprintf (pp, "--------------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "----------------");
	fprintf (pp, "---------------");
	fprintf (pp, "-------------");
	fprintf (pp, "-----------");
	fprintf (pp, "-------------------|\n");

	fprintf (pp, "| TOTAL CASH");
	fprintf (pp, "  %-10.10s ", " ");
	fprintf (pp, "                                ");
	fprintf (pp, "                          ");

	fprintf (pp,
			 " %14.14s ",
			 comma_fmt (DOLLARS (totalCash), "NNN,NNN,NNN.NN"));

	fprintf (pp,
			 "    %10.10s "," ");

	fprintf (pp,
			 "  %10.10s ", " ");

	fprintf (pp, "  %-8.8s ", " ");

	fprintf (pp,
			 "   %14.14s  |\n", " ");

	fprintf (pp, "|-----------");
	fprintf (pp, "-------------");
	fprintf (pp, "--------------------------------");
	fprintf (pp, "--------------------------");
	fprintf (pp, "----------------");
	fprintf (pp, "---------------");
	fprintf (pp, "-------------");
	fprintf (pp, "-----------");
	fprintf (pp, "-------------------|\n");

	fprintf (pp, "| TOTAL CHEQUE");
	fprintf (pp, "  %-8.8s ", " ");
	fprintf (pp, "                                ");
	fprintf (pp, "                          ");

	fprintf (pp,
			 " %14.14s ",
			 comma_fmt (DOLLARS (totalCheque), "NNN,NNN,NNN.NN"));

	fprintf (pp,
			 "    %10.10s "," ");

	fprintf (pp,
			 "  %10.10s ", " ");

	fprintf (pp, "  %-8.8s ", " ");

	fprintf (pp,
			 "   %14.14s  |\n", " ");
	fprintf (pp, ".EOF\n");
	pclose (pp);
}

int
heading (
 int                scn)
{
	if (scn != cur_screen)
		scn_set (scn);

	clear ();


	box (0, 3, 80, 7);
	line_at (6,1,79);
	line_at (8,1,79);
	line_at (1,0,80);
	line_at (21,0,80);

	rv_pr (ML (mlDbMess142), 28, 0, 1);

	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

	/*  reset this variable for new screen NOT page */
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

int 
ChequeSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct ChequeStruct a = * (const struct ChequeStruct *) a1;
	const struct ChequeStruct b = * (const struct ChequeStruct *) b1;

	result = strcmp (a.cheque, b.cheque);

	return (result);
}
