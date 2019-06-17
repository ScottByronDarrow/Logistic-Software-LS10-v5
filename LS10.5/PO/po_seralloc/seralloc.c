/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: seralloc.c,v 5.5 2002/07/24 08:39:06 scott Exp $
|  Program Name  : (po_seralloc.c )                                   |
|  Program Desc  : (Purchase Order Serial Input.                )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 08/01/90         |
|---------------------------------------------------------------------|
| $Log: seralloc.c,v $
| Revision 5.5  2002/07/24 08:39:06  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/06/20 07:22:11  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/06/19 07:00:42  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:16:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:15  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:42  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
|
| Revision 2.1  2000/09/07 02:31:15  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:34  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/01/14 03:20:13  cam
| Changes for GVision compatibility.  Add calls to sleep () and clear_mess ()
| after calls to print_mess ().
|
| Revision 1.13  1999/11/17 06:40:31  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/11/05 05:17:17  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.11  1999/10/14 03:04:26  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.10  1999/09/29 10:12:13  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/29 01:58:18  scott
| Updated from ansi testing
|
| Revision 1.8  1999/09/21 04:38:11  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:37  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: seralloc.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_seralloc/seralloc.c,v 5.5 2002/07/24 08:39:06 scott Exp $";

#define MAXWIDTH 	165
#define MAXLINES 	500
#define	STORE	store [line_cnt]

#include	<ml_po_mess.h>
#include	<ml_std_mess.h>
#include 	<pslscr.h>
#include	<twodec.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ineiRecord	inei_rec;
struct insfRecord	insf_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
	
	char	*data = "data", 
			*ser_space = "                         ";
 
	int		envVarCrCo 		= 0, 
			envVarCrFind 	= 0;

	char	branchNumber [3], 
			ser_status [2];

/*============================
| Local & Screen Structures. |
============================*/
struct storeRec {
	long	_hhwh_hash;
	long	_hhbr_hash;
	char	serial_number [26];
} store [MAXLINES];

struct {
	char	dummy [11];
	char	previousPo [sizeof pohr_rec.pur_ord_no];
	char	prev_crd_no [7];
	char 	crd_no [7];
	char 	pur_ord_no [sizeof pohr_rec.pur_ord_no];
	char	item_no [17];
	char	desc [41];
	char	ser_no [26];
	char 	po_no [sizeof pohr_rec.pur_ord_no];
	float	qty;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "crd_no", 	 4, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.prev_crd_no, "Supplier No.", " ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.crd_no}, 
	{1, LIN, "name", 	 4, 85, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name}, 
	{1, LIN, "pur_ord_no", 	 5, 20, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "A", "Purchase Order No.", " Default = ALL Purchase Orders for Supplier.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.pur_ord_no}, 
	{2, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "  Item Number.  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{2, TAB, "hhpl_hash", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "", " ", 
		 ND, NO,  JUSTLEFT, "", "", (char *) &poln_rec.hhpl_hash}, 
	{2, TAB, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "              Description               ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc}, 
	{2, TAB, "po_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUU", "          ", 
		" ", "", "  P.O. Number  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.po_no}, 
	{2, TAB, "ser_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "    Serial Number.       ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.ser_no}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};


#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	Update 				(void);
void 	SrchPohr 			(char *);
void 	PrintCoStuff 		(void);
void 	DeleteInsf 			(long, char *);
void 	AddInsf 			(long, long, char *);
int 	spec_valid 			(int);
int 	LoadPoln 			(long);
int 	LoadLines 			(long);
int 	IntFindSupercession (char *);
int 	CheckDupSerial 		(char *, long, int);
int	 	heading 			(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR (vars);


	init_scr ();
	set_tty (); 
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	envVarCrFind 	= atoi (get_env ("CR_FIND"));
	envVarCrCo 		= atoi (get_env ("CR_CO"));

	/*--------------------------- 
	| Open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	strcpy (local_rec.previousPo, "000000000000000");
	strcpy (local_rec.prev_crd_no, "000000");

	tab_row = 7;
	tab_col = 12;

	while (prog_exit == 0)
	{
		eoi_ok 		= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		lcount [2] 	= 0;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (2);
		scn_display (2);
		edit (2);

		if (restart)
			continue;

		Update ();
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
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	ReadMisc ();
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
                                                          : "sumr_id_no3");

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (inmr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (insf);
	abc_fclose (incc);
	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("ccmr", ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose ("ccmr");
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Creditor Number. |
	---------------------------*/
	if (LCHECK ("crd_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, zero_pad (local_rec.crd_no, 6));
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc) 
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("pur_ord_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.pur_ord_no, "ALL");
			DSP_FLD ("pur_ord_no");
			entry_exit = TRUE;
			cc = LoadPoln (0L);
			if (cc) 
			{
				restart = 1;
				return (EXIT_SUCCESS);
			}
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, local_rec.pur_ord_no);
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no, zero_pad (pohr_rec.pur_ord_no, 15));
		strcpy (pohr_rec.type, "O");
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "u");
		/*------------------------
		| Order already on file. |
		------------------------*/
		if (cc == 0) 
		{
			if (pohr_rec.status [0] == 'D')
			{
				print_mess (ML (mlPoMess001));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] == 'Y')
			{
				print_mess (ML (mlPoMess002));
				sleep (sleepTime);
				clear_mess ();
				abc_unlock (pohr);
				return (EXIT_FAILURE);
			}
	
			entry_exit = 1;
			if (LoadPoln (pohr_rec.hhpo_hash))
				return (EXIT_FAILURE);
		}
		else
		{
			print_mess (ML (mlStdMess048));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ser_no"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (CheckDupSerial (local_rec.ser_no, STORE._hhwh_hash, line_cnt))
		{
			sprintf (err_str, ML (mlStdMess097), local_rec.ser_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		abc_selfield (insf, "insf_id_no2");
		insf_rec.hhwh_hash = STORE._hhwh_hash;
		strcpy (insf_rec.serial_no, local_rec.ser_no);
		cc = find_rec (insf, &insf_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (err_str, ML (mlStdMess275), local_rec.ser_no, insf_rec.status);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		abc_selfield (insf, "insf_id_no");

		strcpy (STORE.serial_number, local_rec.ser_no);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=======================================================================
| Routine to read all poln records whose hash matches the one on the    |
| pohr record.                                                          |
=======================================================================*/
int
LoadPoln (
	long	hhpoHash)
{
	print_at (2, 1, ML (mlStdMess035));
	fflush (stdout);
	
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	init_vars (2);
	scn_set (2);
	lcount [2] = 0;
	
	abc_selfield (inmr, "inmr_hhbr_hash");

	/*-----------------------------------------------
	| Load all purchase orders lines for supplier	|
	-----------------------------------------------*/
	if (hhpoHash == 0L)
	{
		strcpy (pohr_rec.co_no, sumr_rec.co_no);
		strcpy (pohr_rec.br_no, comm_rec.est_no);
		strcpy (pohr_rec.pur_ord_no, "       ");
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.type, "O");
		cc = find_rec (pohr, &pohr_rec, GTEQ, "r");

		while (!cc && !strcmp (pohr_rec.co_no, comm_rec.co_no) && 
			      !strcmp (pohr_rec.br_no, comm_rec.est_no))
		{
			if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
			     pohr_rec.type [0] == 'O' && 
			     pohr_rec.drop_ship [0] != 'Y' && 
			     pohr_rec.status [0] != 'D')
				cc = LoadLines (pohr_rec.hhpo_hash);

			cc = find_rec (pohr, &pohr_rec, NEXT, "r");
		}
	}
	/*---------------------------------------
	| Load lines of specified purchase order|
	---------------------------------------*/
	else
		LoadLines (hhpoHash);

	abc_selfield (inmr, "inmr_id_no");

	vars [scn_start].row = lcount [2];
	scn_set (1);

	/*---------------------
	| No entries to edit. |
	---------------------*/
	if (lcount [2] == 0)
	{
		PauseForKey (0, 0, ML (mlPoMess004), 0);
		return (EXIT_FAILURE);
	}

	/*-------------------------
	| Normal exit - return 0. |
	-------------------------*/
	return (EXIT_SUCCESS);
}

int
LoadLines (
	long	hhpoHash)
{
	poln_rec.hhpo_hash = hhpoHash;
	poln_rec.line_no = 0; 

	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{
		/*------------------
		| Get part number. |
		------------------*/
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (strcmp (inmr_rec.supercession, "                "))
			{
				abc_selfield (inmr, "inmr_id_no");
				IntFindSupercession (inmr_rec.supercession);
				abc_selfield (inmr, "inmr_hhbr_hash");
			}
		}
		if (cc || inmr_rec.serial_item [0] != 'Y')
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		else
		{
			strcpy (local_rec.item_no, inmr_rec.item_no);

			incc_rec.hhcc_hash = poln_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "incc", "DBFIND");
		}

		/*---------------------
		| Setup local record. |
		---------------------*/
		sprintf (local_rec.desc, "%-40.40s", poln_rec.item_desc);
		strcpy (local_rec.po_no, pohr_rec.pur_ord_no);
		sprintf (local_rec.ser_no, "%-25.25s", poln_rec.serial_no);
		local_rec.qty = poln_rec.qty_ord - poln_rec.qty_rec;

		store [lcount [2]]._hhwh_hash = incc_rec.hhwh_hash;
		store [lcount [2]]._hhbr_hash = inmr_rec.hhbr_hash;

		if (local_rec.qty <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}

		if (local_rec.qty > 0.00)
			putval (lcount [2]++);

		cc = find_rec (poln, &poln_rec, NEXT, "r");

		if (lcount [2] % 25 == 0)
			putchar ('R');

		fflush (stdout);
		/*-------------------
		| Too many orders . |
		-------------------*/
		if (lcount [2] > MAXLINES) 
			break;
	}
    return (EXIT_SUCCESS);
}

/*===========================
| Updated relevent records. |
===========================*/
void
Update (
 void)
{
	abc_selfield (poln, "poln_hhpl_hash");
	abc_selfield (pohr, "pohr_hhpo_hash");
	scn_set (2);

	clear ();
	/*"Updating Serial items "*/
	print_at (0, 0, ML (mlStdMess035));fflush (stdout);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
	{
		getval (line_cnt);
		cc = find_rec (poln, &poln_rec, EQUAL, "u");
		if (cc)
			file_err (cc, "poln", "DBFIND");

		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "pohr", "DBFIND");

		if (!strcmp (local_rec.ser_no, ser_space) && 
		     !strcmp (poln_rec.serial_no, ser_space))
			continue;

		strcpy (ser_status, "F");

		/*-------------------------------------------------------------
		| Delete out serial record for local serial number if exists. |
		-------------------------------------------------------------*/
		DeleteInsf (store [line_cnt]._hhwh_hash, local_rec.ser_no);

		/*------------------------------------------------------------
		| Delete out serial record for poln serial number if exists. |
		------------------------------------------------------------*/
		DeleteInsf (store [line_cnt]._hhwh_hash, poln_rec.serial_no);

		if (strcmp (local_rec.ser_no, ser_space))
		{
			/*------------------------
			| Add new Serial Record. |
			------------------------*/
			AddInsf 
			(
				store [line_cnt]._hhwh_hash, 
				store [line_cnt]._hhbr_hash, 
				local_rec.ser_no
			);
		}

		strcpy (poln_rec.serial_no, local_rec.ser_no);
		cc = abc_update (poln, &poln_rec);
		if (cc)
			file_err (cc, "poln", "DBUPDATE");

		abc_unlock (poln);
	}
	abc_unlock (poln);
	abc_selfield (poln, "poln_id_no");
	abc_selfield (pohr, "pohr_id_no");
	
	scn_set (1);
	if (!strncmp (local_rec.pur_ord_no, "ALL", 3))
		strcpy (local_rec.previousPo, "ALL");
	else
		strcpy (local_rec.previousPo, pohr_rec.pur_ord_no);
	sprintf (local_rec.prev_crd_no, "%-6.6s", sumr_rec.crd_no);

}

/*==========================
| Search for order number. |
==========================*/
void
SrchPohr (
	char	*key_val)
{
	work_open ();
	save_rec ("#Purchase Order ", "#Date Raised");
	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", key_val);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && 
		!strncmp (pohr_rec.pur_ord_no, key_val, strlen (key_val)) && 
		!strcmp (pohr_rec.co_no, comm_rec.co_no) && 
	        !strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] == 'O' && pohr_rec.status [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
		  	    break;
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no, comm_rec.co_no);
	strcpy (pohr_rec.br_no, comm_rec.est_no);
	sprintf (pohr_rec.pur_ord_no, "%-15.15s", temp_str);
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (pohr_rec.type, "O");
	cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}

void
PrintCoStuff (
 void)
{
	move (0, 19);
	line (132);
	print_at (20, 0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (21, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 0, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);
}


/*===============================
| Validate Supercession Number. |
===============================*/
int
IntFindSupercession (
	char	*sitemNo)
{
	/*---------------------------------------
	| At the end of the supercession chain	|
	---------------------------------------*/
	if (!strcmp (sitemNo, "                "))
		return (EXIT_SUCCESS);

	/*-------------------------------
	| Find the superceeding item	|
	-------------------------------*/
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, sitemNo);
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (!cc)
		return (IntFindSupercession (inmr_rec.supercession));

	/*---------------------------------------
	| Couldn't find the superceeding item	|
	---------------------------------------*/
	print_mess (ML (mlPoMess005));
	sleep (sleepTime);
	clear_mess ();

	return (cc);
}

/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.	Return 1 if duplicate		|
=======================================================*/
int
CheckDupSerial (
	char 	*serialNo, 
	long 	hhwhHash, 
	int 	lineNo)
{
	int	i;
	int	noLines;

	noLines = (prog_status == ENTRY && (lcount [2] - 1 < line_cnt)) ? 
                      line_cnt : lcount [2] - 1;

	for (i = 0;i < noLines;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == lineNo)
			continue;

		/*-------------------------------------------------------------------
		| cannot duplicate item_no/serial_no unless serial no was not input	|
		-------------------------------------------------------------------*/
		if (!strcmp (store [i].serial_number, ser_space))
			continue;

		/*-------------------------------------------------------
		| Only compare serial numbers for the same item number	|
		-------------------------------------------------------*/
		if (store [i]._hhwh_hash == hhwhHash)
		{
			if (!strcmp (store [i].serial_number, serialNo))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*====================================================
| Delete serial item record as about to be re-added. |
====================================================*/
void
DeleteInsf (
	long	hhwhHash, 
	char	*serialNo)
{
	abc_selfield (insf, "insf_id_no2");

	insf_rec.hhwh_hash = hhwhHash;
	sprintf (insf_rec.serial_no, "%-25.25s", serialNo);
	cc = find_rec (insf, &insf_rec, COMPARISON, "r");
	if (cc)
		return;

	strcpy (ser_status, insf_rec.status);

	cc = abc_delete (insf);
	if (cc)
		file_err (cc, "insf", "DBDELETE");

	abc_selfield (insf, "insf_id_no");

	return;
}
	
/*========================================================
| Add serial item record will all information available. |
========================================================*/
void
AddInsf (
	long	hhwhHash, 
	long	hhbrHash, 
	char	*serialNo)
{
	insf_rec.hhwh_hash = hhwhHash;
	insf_rec.hhbr_hash = hhbrHash;
	strcpy (insf_rec.status, ser_status);
	strcpy (insf_rec.receipted, "N");
	sprintf (insf_rec.serial_no, "%-25.25s", serialNo);
	insf_rec.exch_rate   = pohr_rec.curr_rate;
	insf_rec.fob_fgn_cst = poln_rec.fob_fgn_cst;
	insf_rec.fob_nor_cst = poln_rec.fob_nor_cst;
	insf_rec.frt_ins_cst = poln_rec.frt_ins_cst;
	insf_rec.duty        = poln_rec.duty;
	insf_rec.licence     = poln_rec.licence;
	insf_rec.lcost_load  = poln_rec.lcost_load;

	insf_rec.land_cst    = poln_rec.fob_nor_cst +
				  			  poln_rec.lcost_load +
		          	  		  poln_rec.duty +
		          	  		  poln_rec.licence;

	insf_rec.istore_cost = poln_rec.land_cst;
	insf_rec.est_cost    = poln_rec.land_cst;
	strcpy (insf_rec.stat_flag, "E");

	strcpy (insf_rec.po_number, pohr_rec.pur_ord_no);
	strcpy (insf_rec.gr_number, "NOT YET");

	cc = abc_add (insf, &insf_rec);
	if (cc) 
		file_err (cc, "insf", "DBADD");
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		swide ();
		clear ();
		rv_pr (ML (mlPoMess006), 45, 0, 1);

		print_at (0, 92, ML (mlPoMess007), local_rec.prev_crd_no, local_rec.previousPo);
		fflush (stdout);
		move (0, 1);
		line (132);

		box (0, 3, 130, 2);
		if (scn == 1)
		{
			scn_set (2);
			scn_write (2);
			scn_display (2);
		}
		else
		{
			scn_set (1);
			scn_write (1);
			scn_display (1);
		}
		scn_set (scn);

		PrintCoStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
