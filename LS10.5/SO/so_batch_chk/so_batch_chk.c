/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_batch_chk.c,v 5.1 2002/02/22 09:13:11 scott Exp $
|  Program Name  : (so_batch_chk.c)        
|  Program Desc  : (Sales Order Batch Report)
|---------------------------------------------------------------------|
|  Date Written  : 12/02/92        | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: so_batch_chk.c,v $
| Revision 5.1  2002/02/22 09:13:11  scott
| S/C 00822 - Clean up program - no real fault found.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_batch_chk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_batch_chk/so_batch_chk.c,v 5.1 2002/02/22 09:13:11 scott Exp $";

#define	X_OFF	lp_x_off
#define	Y_OFF	lp_y_off

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>
#include 	<arralloc.h>

#ifdef GVISION
#include <RemoteFile.h>
#define	fopen	Remote_fopen
#define	fgets	Remote_fgets
#define	fclose	Remote_fclose
#endif	/* GVISION */

char *UNDERLINE = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG^^";

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;

	char	*data	= "data";

extern int	lp_x_off;
extern int	lp_y_off;
char	validStatus [16];

	struct	
	{
		char	*statCode;
		char	*statDesc;
	} status [] = {

		{"P	", 	"Proforma Invoice"},
		{"X7", 	"Transaction not yet posted."},
		{"6", 	"Transaction flagged for posting"},
		{"4", 	"Transaction Posted (phase 1 of 5)"},
		{"3", 	"Transaction posted (phase 2 of 5)"},
		{"2", 	"Transaction posted (phase 3 of 5)"},
		{"1", 	"Transaction posted (phase 4 of 5)"},
		{"0", 	"Transaction posted (phase 5 of 5)"},
		{"9D", 	"Transaction posting COMPLETE."},
		{"", 	""}
	};
/*
 *	Structure for dynamic array.
 */
struct SortArray
{
	char	sortKey			[21];
	char	batchNo			[sizeof	cohr_rec.batch_no];
	char	invoiceNo		[sizeof	cohr_rec.inv_no];
	char	customerNo		[sizeof	cumr_rec.dbt_no];
	char	customerName	[sizeof	cumr_rec.dbt_name];
	long	customerDate;
	char	customerStatus	[sizeof	cohr_rec.stat_flag];
}	*sortRec;
	DArray sort_d;
	int	sortCounter = 0;

/*===========================
| Local & Screen Structures |
===========================*/
struct
{
	char	dummy [11];
	int		printerNo;
	char	br_no [3];
	char	br_name [41];
	char	type [2];
	char	type_desc [9];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "branch",	 3, 12, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, " Branch : ", " Enter Branch No. Default Is Current Branch ",
		YES, NO,  JUSTRIGHT, "0", "99", local_rec.br_no},
	{1, LIN, "br_name",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.br_name},
	{1, LIN, "type",	 3, 90, CHARTYPE,
		"U", "          ",
		" ", "I", " Type : ", " I(nvoice) / C(redit) ",
		YES, NO,  JUSTLEFT, "IC", "", local_rec.type},
	{1, LIN, "type_desc",	 3, 93, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "IC", "", local_rec.type_desc},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include	<get_lpno.h>

/*
 * Function Declarations
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessFile 		(void);
char 	*GetTranStatus 		(char *);
int  	spec_valid 			(int);
void 	SrchEsmr 			(char *);
int  	heading 			(int);
int		SortSort 			(const void *, const void *);


/*
 * Main Processing Routine 
 */
int
main (
	int		argc,
	char	*argv [])
{
	if (argc != 2)
	{
		print_at (0,0,mlSoMess774, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (validStatus, "%-15.15s", argv [1]);
	clip (validStatus);

	SETUP_SCR (vars);

	/*
	 * Setup required parameters	
	 */
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*
		 * Reset control flags . 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;
		init_vars (1);

		/*
		 * Enter screen 1 linear input  
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		heading (1);
		scn_display (1);
		ProcessFile ();
	}	/* end of input control loop	*/

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

	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*
 * Close data base files	
 */
void
CloseDB (void)
{
	abc_fclose (cohr);
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_dbclose (data);
}



void
ProcessFile (void)
{
	char	dispStr [200];
	int		dataFound	=	FALSE;
	int		i;

	/*
	 * Setup Array with 500 entries to start. 
	 */
	ArrAlloc (&sort_d, &sortRec, sizeof (struct SortArray), 100);

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, local_rec.br_no);
	sprintf (cohr_rec.type, "%-1.1s", local_rec.type);
	strcpy (cohr_rec.inv_no, "        ");
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc &&
	       !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cohr_rec.br_no, local_rec.br_no) &&
	       !strncmp (cohr_rec.type, local_rec.type, 1))
	{
		if (!strchr (validStatus, cohr_rec.stat_flag [0]))
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}

		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check the array size before adding new element. 
		 */
		if (!ArrChkLimit (&sort_d, sortRec, sortCounter))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCounter. 
		 */
		sprintf 
		(
			sortRec [sortCounter].sortKey, 
			"%s%s%s",
			cohr_rec.batch_no,
			cohr_rec.inv_no,
			cumr_rec.dbt_no
		);
		strcpy (sortRec [sortCounter].batchNo,		cohr_rec.batch_no);
		strcpy (sortRec [sortCounter].invoiceNo,	cohr_rec.inv_no);
		strcpy (sortRec [sortCounter].customerNo,	cumr_rec.dbt_no);
		strcpy (sortRec [sortCounter].customerName, cumr_rec.dbt_name);
		sortRec [sortCounter].customerDate	=	cohr_rec.date_raised;
		strcpy (sortRec [sortCounter].customerStatus, cohr_rec.stat_flag);

		/*
		 * Increment array counter. 
		 */
		sortCounter++;

		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCounter, sizeof (struct SortArray), SortSort);

	dataFound = FALSE;

	lp_x_off = 0;
	lp_y_off = 5;
	Dsp_prn_open 
	(
		0, 
		5, 
		12, 
		" Posted Invoice Batch Check Report ", 
		comm_rec.co_no, 
		comm_rec.co_name,
		local_rec.br_no, 
		local_rec.br_name,
		(char *)0, 
		(char *)0
	);

	Dsp_saverec ("BATCH NO|INVOICE NO|CUSTOMER|             CUSTOMER NAME              |DATE RAISED|                POST STATUS                  ");
	Dsp_saverec ("");
	Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");

	for (i = 0; i < sortCounter; i++)
	{
		sprintf 
		(
			dispStr,
			" %6.6s ^E %8.8s ^E %-6.6s ^E%-40.40s^E%-10.10s ^E%-45.45s",
			sortRec [i].batchNo,
			sortRec [i].invoiceNo,
			sortRec [i].customerNo,
			sortRec [i].customerName,
			DateToString (sortRec [i].customerDate),
			GetTranStatus (sortRec [i].customerStatus)
		);
		Dsp_saverec (dispStr);
		dataFound = TRUE;
	}

	if (dataFound)
		Dsp_saverec (UNDERLINE);

	Dsp_srch ();
	Dsp_close ();
	/*
	 * Free up the array memory. 
	 */
	ArrDelete (&sort_d);
}

/*
 * Gets posting status description 
 */
char *
GetTranStatus (
	char	*stat)
{
	int		i;

	for (i = 0; strlen (status [i].statCode); i++)
	{
		if (strchr (status [i].statCode, stat [0]))
			return (status [i].statDesc);
	}
	return (ML ("status not found"));
}

int
spec_valid (
 int field)
{
	if (LCHECK ("branch"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		sprintf (esmr_rec.est_no, "%2.2s", local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.br_name, "%-40.40s", esmr_rec.est_name);
		DSP_FLD ("br_name");
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("type"))
	{
		if (local_rec.type [0] == 'I')
			strcpy (local_rec.type_desc, "Invoice");
		else
			strcpy (local_rec.type_desc, "Credit ");

		DSP_FLD ("type_desc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char *keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Branch Name");
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", keyValue);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (esmr_rec.co_no, comm_rec.co_no) &&
		!strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in esmr During (DBFIND)", cc, PNAME);
}

/*
 * Sort Function.
 */
int 
SortSort (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct SortArray a = * (const struct SortArray *) a1;
	const struct SortArray b = * (const struct SortArray *) b1;

	result = strcmp (a.sortKey, b.sortKey);

	return (result);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		swide ();
		clear ();

		rv_pr (ML (mlSoMess211), 52, 0, 1);

		line_at (1,0,132);

		if (scn == 1)
			box (0, 2, 132, 1);

		line_at (21,0,132);
		
		print_at (22, 0, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (22,45, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_set (scn);
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
