/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_ar_wodisp.c,v 5.0 2002/06/05 06:05:40 scott Exp $
|  Program Name  : (pc_wodisp.c)
|  Program Desc  : (Display Archive Work Orders Status)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow    Date Written  : 5th June 2002    |
|---------------------------------------------------------------------|
| $Log: pc_ar_wodisp.c,v $
| Revision 5.0  2002/06/05 06:05:40  scott
| 	{1, LIN, "mfgBrNo",	 2, 18, CHARTYPE,
| 		"AA", "          ",
| 		" ", comm_rec.est_no, "Mfg Branch  :", "Manufacturing Branch.",
| 		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgBrNo},
| 	{1, LIN, "mfgBrName",	 2, 25, CHARTYPE,
| 		"UUUUUUUUUUUUUUU", "          ",
| 		" ", "", "", "",
| 		NA, NO,  JUSTLEFT, "", "", local_rec.mfgBrName},
| 	{1, LIN, "mfgWhNo",	 2, 75, CHARTYPE,
| 		"AA", "          ",
| 		" ", comm_rec.cc_no, "Mfg Warehouse:", "Manufacturing Warehouse.",
| 		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgWhNo},
| 	{1, LIN, "mfgWhName",	 2, 82, CHARTYPE,
| 		"UUUUUUUUU", "          ",
| 		" ", "", "", "",
| 		NA, NO,  JUSTLEFT, "", "", local_rec.mfgWhName},
| 	{1, LIN, "orderNumber",	 3, 18, CHARTYPE,
| 		"UUUUUUU", "          ",
| 		" ", " ", "Order Number:", " ",
| 		NO, NO,  JUSTLEFT, "", "", local_rec.orderNumber},
|
| Revision 1.1  2002/06/05 04:05:25  scott
| First Release
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_ar_wodisp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_ar_wodisp/pc_ar_wodisp.c,v 5.0 2002/06/05 06:05:40 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_pc_mess.h>


#include	"schema"

struct commRecord	comm_rec;
struct arpcwoRecord	pcwo_rec;
struct ccmrRecord	ccmr_rec;
struct esmrRecord	esmr_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;

	char	*data = "data";
 

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char	systemDate [11];
	char	mfgBrNo [3];
	char	mfgWhNo [3];
	char	mfgWhName [21];
	char	mfgBrName [21];
	char	orderNumber [8];
} local_rec;

extern	int	TruePosition;

static struct	var vars [] =
{
	{1, LIN, "mfgBrNo",	 2, 2, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no,"Manufacturing Branch      ", "Enter Manufacturing Branch.",
		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgBrNo},
	{1, LIN, "mfgBrName",	 2, 40, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.mfgBrName},
	{1, LIN, "mfgWhNo",	 3, 2, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.cc_no, "Manufacturing Warehouse   ", "Enter Manufacturing Warehouse.",
		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgWhNo},
	{1, LIN, "mfgWhName",	 3, 40, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.mfgWhName},
	{1, LIN, "orderNumber",	 4, 2, CHARTYPE,
		"UUUUUUU", "          ",
		" ", " ", "Order Number              ", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.orderNumber},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          "
		, " ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};


/*
 * function prototypes 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadInmr 		(void);
void 	ReadInei 		(void);
void	SrchCcmr 		(char *, char *);
void	SrchEsmr 		(char *);
void	Srchpcwo 		(char *, char *, char *);
int 	spec_valid 		(int);
int 	ProcessArPcwo 	(void);
int 	ProcLog 		(void);
int 	heading 		(int);
/*
 * Main Processing Routine 
 */
int
main (
 int  argc, 
 char *argv [])
{
	TruePosition = TRUE;

	SETUP_SCR (vars);

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	OpenDB ();

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
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
	
		/*
		 * Process Orders in Database.
		 */
		ProcessArPcwo ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (arpcwo, arpcwo_list, ARPCWO_NO_FIELDS,"arpcwo_id_no");
	open_rec (ccmr,   ccmr_list,   CCMR_NO_FIELDS, 	"ccmr_id_no");
	open_rec (esmr,   esmr_list,   ESMR_NO_FIELDS, 	"esmr_id_no");
	open_rec (inmr,   inmr_list,   INMR_NO_FIELDS, 	"inmr_hhbr_hash");
	open_rec (inei,   inei_list,   INEI_NO_FIELDS, 	"inei_id_no");
}
/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (arpcwo);
	abc_fclose (ccmr);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	/*
	 * Validate Manufacturing Branch Number.
	 */
	if (LCHECK ("mfgBrNo"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (esmr_rec.co_no,	comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.mfgBrNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Branch Not found. 
			 */
			print_mess (ML(mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.mfgBrName, esmr_rec.short_name);
		DSP_FLD ("mfgBrName");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Manufacturing Warehouse Number.
	 */
	if (LCHECK ("mfgWhNo"))
	{

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str, local_rec.mfgBrNo);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.mfgBrNo);
		strcpy (ccmr_rec.cc_no,  local_rec.mfgWhNo);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Warehouse not found.
			 */
			print_mess (ML(mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.mfgWhName, ccmr_rec.acronym);
		DSP_FLD ("mfgWhNo");
		DSP_FLD ("mfgWhName");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate works order number.
	 */
	if (LCHECK ("orderNumber"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);
			
		if (SRCH_KEY)
		{
			Srchpcwo 
			(
				temp_str,
				local_rec.mfgBrNo,
				local_rec.mfgWhNo
			);
			return (EXIT_SUCCESS);
		}

		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, local_rec.mfgBrNo);
		strcpy (pcwo_rec.wh_no, local_rec.mfgWhNo);
		strcpy (pcwo_rec.order_no, local_rec.orderNumber);
		cc = find_rec (arpcwo, &pcwo_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 *  Released Works Order not found.
			 */
			print_mess (ML(mlPcMess090));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ProcessArPcwo (void)
{
	Dsp_nc_prn_open 
	(
		3, 
		5, 
		8, 
		err_str, 
		comm_rec.co_no, 
		comm_rec.co_name, 
		comm_rec.est_no, 
		comm_rec.est_name, 
		(char *)0, 
		(char *)0
	);

	Dsp_saverec ("BR|WH|WORKS ORD|   BATCH    |  REQUIRED  |     I T E M      |    REQ 'D     |    REC 'D     |    REJ 'D     |    BATCH      ");
	Dsp_saverec ("NO|NO|   NO    |     NO     |    DATE    |        NO        |     QTY       |     QTY       |     QTY       |     SIZE      ");
	Dsp_saverec (" [NEXT] [PREV] [EDIT/END] ");

	strcpy (pcwo_rec.co_no,  comm_rec.co_no);
	strcpy (pcwo_rec.br_no,  local_rec.mfgBrNo);
	strcpy (pcwo_rec.wh_no,  local_rec.mfgWhNo);
	strcpy (pcwo_rec.order_no, local_rec.orderNumber);

	cc = find_rec (arpcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && !strcmp (pcwo_rec.co_no, comm_rec.co_no) &&
				  !strcmp (pcwo_rec.br_no, local_rec.mfgBrNo) &&
				  !strcmp (pcwo_rec.wh_no, local_rec.mfgWhNo))
	{
		ReadInmr ();
		ReadInei ();

		/*
		 * print to screen or printer 
		 */
		ProcLog ();

		if (strlen (clip (local_rec.orderNumber)))
			break;

		cc = find_rec (arpcwo, &pcwo_rec, NEXT, "r");
	}
	Dsp_srch ();
	Dsp_close ();
	
	return (EXIT_SUCCESS);
}

void
ReadInmr (void)
{
	inmr_rec.hhbr_hash = pcwo_rec.hhbr_hash;

	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inmr_rec.item_no, ML ("Item Not Found"));
		strcpy (inmr_rec.description, "");
		inmr_rec.dec_pt = 2;
	}

	if (inmr_rec.dec_pt > 4)
		dec_pt = 4;
	else
		dec_pt = inmr_rec.dec_pt;
}

void
ReadInei (void)
{
	inei_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	strcpy (inei_rec.est_no, local_rec.mfgBrNo);

	cc = find_rec (inei, &inei_rec, COMPARISON, "r");

	if (cc)
		inei_rec.std_batch = 0.00;
}

int
ProcLog (
 void)
{
	char	disp_str [200];

	sprintf 
	(
		disp_str, 
		"%2.2s|%2.2s| %7.7s | %10.10s | %10.10s | %16.16s |%14.6f |%14.6f |%14.6f |%14.6f ",
		pcwo_rec.br_no,
		pcwo_rec.wh_no,
		pcwo_rec.order_no,
		pcwo_rec.batch_no,
		DateToString (pcwo_rec.reqd_date),
		inmr_rec.item_no,
		n_dec (pcwo_rec.prod_qty, dec_pt),
		n_dec (pcwo_rec.act_prod_q, dec_pt),
		n_dec (pcwo_rec.act_rej_q, dec_pt),
		twodec (inei_rec.std_batch)
	);
	Dsp_saverec (disp_str);
	
	return (EXIT_SUCCESS);
}

void
SrchEsmr (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#Br", "#Branch Description");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", keyValue);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (esmr_rec.co_no, comm_rec.co_no) &&
		!strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
	{
		sprintf (err_str,
				"(%-15.15s) %-40.40s",
				esmr_rec.short_name,
				esmr_rec.est_name);
		save_rec (esmr_rec.est_no, err_str);

	    cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
	char	*keyValue, 
	char	*br_no)
{
	_work_open (2,0,40);
	save_rec ("#Wh", "#Warehouse Description");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, br_no) &&
		!strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
	{
		sprintf (err_str, "(%-9.9s) %-40.40s", ccmr_rec.acronym, ccmr_rec.name);
		save_rec (ccmr_rec.cc_no, err_str);

	    cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
Srchpcwo (
	char	*key_val, 
	char	*branchNo, 
	char	*warehouseNo)
{
	abc_selfield (inmr, "inmr_hhbr_hash");

	_work_open (7,0,75);
	save_rec ("#W/O No.", "#Batch No.  (Item Number     ) Item Description");

	strcpy (pcwo_rec.co_no, comm_rec.co_no);
	strcpy (pcwo_rec.br_no, branchNo);
	strcpy (pcwo_rec.wh_no, warehouseNo);
	sprintf (pcwo_rec.order_no, "%-7.7s", key_val);
	cc = find_rec (arpcwo, &pcwo_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pcwo_rec.co_no, comm_rec.co_no) &&
		!strcmp (pcwo_rec.br_no, branchNo) &&
		!strcmp (pcwo_rec.wh_no, warehouseNo) &&
		!strncmp (pcwo_rec.order_no, key_val, strlen (key_val)))
	{
		inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (inmr_rec.item_no, "%-16.16s", " ");
			strcpy (inmr_rec.description, ML ("DELETED ITEM"));
		}
		sprintf 
		(
			err_str, 
			"%-10.10s (%-16.16s) %-40.40s",
			pcwo_rec.batch_no,
			inmr_rec.item_no,
			inmr_rec.description
		);
		cc = save_rec (pcwo_rec.order_no, err_str);
		if (cc)
			break;
		
		cc = find_rec (arpcwo, &pcwo_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (pcwo_rec.co_no, comm_rec.co_no);
	strcpy (pcwo_rec.br_no, branchNo);
	strcpy (pcwo_rec.wh_no, warehouseNo);
	sprintf (pcwo_rec.order_no, "%-7.7s", temp_str);
	cc = find_rec (arpcwo, &pcwo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, arpcwo, "DBFIND");
}

int
heading (
	int		scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();
	
	rv_pr (ML ("Archive Works Order Display"), 46, 0, 1);

	box (0, 1, 132, 3);
	line_at (19,0,132);

	print_at (20,0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
	print_at (21,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name); 
	print_at (22,0, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_name);

	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
