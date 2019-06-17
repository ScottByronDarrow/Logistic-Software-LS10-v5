/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: trackdsp.c,v 5.3 2001/11/08 10:25:59 scott Exp $
|  Program Name  : (so_trackdsp.c)
|  Program Desc  : (Customer Order Track display)
|---------------------------------------------------------------------|
|  Date Written  : (28/04/89)      | Author       : Fui Choo Yap.     |
|---------------------------------------------------------------------|
| $Log: trackdsp.c,v $
| Revision 5.3  2001/11/08 10:25:59  scott
| Updated to remove disk sort and replace with qsort
| Updated to use app.schema
| Updated to fix selection from || to &&
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: trackdsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_trackdsp/trackdsp.c,v 5.3 2001/11/08 10:25:59 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <arralloc.h>

#define	X_OFF	0
#define	Y_OFF	2

#define		CREDIT		(cohr_rec.type [0] == 'C')
#define		INVOICE		(cohr_rec.type [0] == 'I')
#define		P_SLIP		(cohr_rec.type [0] == 'P')
#define		DIS_SCN		2

#define		MAX_PRINTED_LINE	10

	int		envDbCo = 0,
			envDbFind = 0;
	
	char	branchNumber [3];
	char	dispDataString [300];
	
	char	*data = "data";

#include	"schema"

struct commRecord	comm_rec;
struct cohrRecord	cohr_rec;
struct cumrRecord	cumr_rec;
struct sohrRecord	sohr_rec;
struct sosfRecord	sosf_rec;

	int		linesPrintedCount	=	0;

/*
 *	Structure for dynamic array,  for the lotRec lines for qsort
 */
struct TrkStruct
{
	char	sortCode [27];
	char	cusOrdRef	[sizeof sosf_rec.cus_ord_ref]; 
	char	consNo		[sizeof sosf_rec.cons_no];
	long	hhcoHash; 
	long	hhsoHash;
}	*trkRec;
	DArray trk_details;
	int	trkCnt = 0;

int		TrkSort			(const	void *,	const void *);

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	name [41];
	char	customerNumber [7];
	char	customerOrderRef [21];
	char	consignmentNumber [17];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "debtor",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "",  "Customer Number          : ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.customerNumber},
	{1, LIN, "name",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name            : ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "customerOrderRef",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "*", "Customer Order Reference : ", "Return will default to all ",
		YES, NO,  JUSTLEFT, "", "", local_rec.customerOrderRef},
	{1, LIN, "consignmentNumber",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAA", "          ",
		" ", "*", "Consignment Number       : ", "Return will default to all ",
		YES, NO,  JUSTLEFT, "", "", local_rec.consignmentNumber},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include	<FindCumr.h>
/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int  	spec_valid 			(int);
void 	DisplayTrackingInfo (void);
void 	CheckPageDisplay 	(void);
void 	ProcessOrderRef 	(long, char *, char *);
void 	PrintCompanyDetails (void);
int  	heading 			(int);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");


	while (prog_exit == 0)
	{
		search_ok	=	TRUE;
		entry_exit	= 	FALSE;
		edit_exit	= 	FALSE;
		prog_exit	=	FALSE;
		restart		=	FALSE;
		init_ok		=	TRUE;
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

		heading (1);
		scn_display (1);
		DisplayTrackingInfo ();
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
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

    open_rec (cumr,cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" 
						    	  : "cumr_id_no3");

	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (sosf, sosf_list, SOSF_NO_FIELDS, "sosf_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (sohr);
	abc_fclose (sosf);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("debtor"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.customerNumber));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==========================================
| Display information based on user input. |
==========================================*/
void
DisplayTrackingInfo (void)
{
	char	AmPmFlag [3],
			workTime [6];
	long	hhcoHash	=	0L,
			hhsoHash	=	0L;
	int		i			= 	0,
			processCnt	= 	0;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&trk_details, &trkRec, sizeof (struct TrkStruct), 10);
	trkCnt = 0;

	ProcessOrderRef 
	(
		cumr_rec.hhcu_hash,
		clip (local_rec.customerOrderRef), 
		clip (local_rec.consignmentNumber)
	);
	strcpy (err_str, "CUSTOMER  ORDER  TRACKING  SYSTEM "); 

	Dsp_prn_open (0,7,10, err_str, comm_rec.co_no,  comm_rec.co_name,
				      comm_rec.est_no, comm_rec.est_name,
				     (char *) 0, (char *) 0);
				      
	Dsp_saverec ("                  O r d e r   T r a c k i n g   D e t a i l s                ");
	Dsp_saverec ("");

	Dsp_saverec (" [REDRAW][PRINT][NEXT SCN][PREV SCN][EDIT/END] ");

	/*
	 * Sort the array in item description order.
	 */
	qsort (trkRec, trkCnt, sizeof (struct TrkStruct), TrkSort);

	/*
	 * Step through the sorted array getting the appropriate records.
	 */
	for (processCnt = 0; processCnt < trkCnt; processCnt++)
	{
		linesPrintedCount	=	0;
		hhcoHash	=	 trkRec [processCnt].hhcoHash;
		hhsoHash	=	 trkRec [processCnt].hhsoHash;

		if (hhcoHash)
		{	
			cohr_rec.hhco_hash	=	hhcoHash;
			cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
			if (cc)
				continue;

			abc_selfield (sosf, "sosf_hhco_hash");
			sosf_rec.hhco_hash	=	hhcoHash;
			cc = find_rec (sosf, &sosf_rec, COMPARISON, "r");
			if (cc)
				continue;
		}
		if (hhsoHash)
		{	
			sohr_rec.hhso_hash	=	hhsoHash;
			cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
			if (!cc)
			{
				abc_selfield (sosf, "sosf_hhso_hash");
				sosf_rec.hhso_hash	=	hhsoHash;
				cc = find_rec (sosf, &sosf_rec, COMPARISON, "r");
				if (cc)
					continue;
			}
			else
				sohr_rec.hhso_hash	=	0L;
		}
		sprintf (dispDataString, "^1Customer Order Reference^6 : %-20.20s",
				(cohr_rec.hhco_hash != 0L) ? cohr_rec.cus_ord_ref : sohr_rec.cus_ord_ref);
		Dsp_saverec (dispDataString);
		CheckPageDisplay ();

		if (sohr_rec.hhso_hash)
		{
			sprintf (dispDataString, "Order Number             : %-8.8s",
				sohr_rec.order_no);
			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (cohr_rec.hhco_hash)
		{
			sprintf (dispDataString, "Packing / Invoice Number : %-8.8s",
								cohr_rec.inv_no);
			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		sprintf (dispDataString, "Consignment Number       : %-16.16s",
			(cohr_rec.hhco_hash != 0L) ? cohr_rec.cons_no : sohr_rec.cons_no);
		Dsp_saverec (dispDataString);
		CheckPageDisplay ();

		if (sosf_rec.ocre_date > 0L)
		{
			if (sosf_rec.ocre_time > 719)
			{
				sosf_rec.ocre_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.ocre_time, "NN:NN"));

			sprintf (dispDataString, "Order Date               : %-10.10s     /  Order Time   : %-5.5s %-2.2s",
				DateToString (sosf_rec.pcre_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (sosf_rec.oprn_date > 0L)
		{
			if (sosf_rec.oprn_time > 719)
			{
				sosf_rec.oprn_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.oprn_time, "NN:NN"));

			sprintf (dispDataString, "Confirmation Date        : %-10.10s     /  Confirm Time : %-5.5s %-2.2s",
				DateToString (sosf_rec.oprn_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (sosf_rec.pcre_date > 0L)
		{
			if (sosf_rec.pcre_time > 719)
			{
				sosf_rec.pcre_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.pcre_time, "NN:NN"));

			sprintf (dispDataString, "Picking Document Created : %-10.10s     /  Picking Time : %-5.5s %-2.2s",
				DateToString (sosf_rec.pcre_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (sosf_rec.pprn_date > 0L)
		{
			if (sosf_rec.pprn_time > 719)
			{
				sosf_rec.pprn_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.pprn_time, "NN:NN"));

			sprintf (dispDataString, "Goods Picked             : %-10.10s     /  Picking Time : %-5.5s %-2.2s",
				DateToString (sosf_rec.pprn_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (sosf_rec.odes_date > 0L)
		{
			if (sosf_rec.odes_time > 719)
			{
				sosf_rec.odes_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.odes_time, "NN:NN"));

			sprintf (dispDataString, "Goods Dispatched         : %-10.10s     /  Dispatch Time: %-5.5s %-2.2s",
				DateToString (sosf_rec.odes_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}
		if (sosf_rec.odel_date > 0L)
		{
			if (sosf_rec.odel_time > 719)
			{
				sosf_rec.odel_time -= 720;
				strcpy (AmPmFlag, "PM");
			}
			else
				strcpy (AmPmFlag, "AM");
			
			strcpy (workTime, ttoa (sosf_rec.odel_time, "NN:NN"));

			sprintf (dispDataString, "Goods Delivered          : %-10.10s     /  Delivery Time: %-5.5s %-2.2s",
				DateToString (sosf_rec.odel_date),
				workTime, AmPmFlag);

			Dsp_saverec (dispDataString);
			CheckPageDisplay ();
		}

		if (linesPrintedCount < MAX_PRINTED_LINE - 1)
		{
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
			CheckPageDisplay ();
		}
		for (i = linesPrintedCount; i < MAX_PRINTED_LINE; i++)
			Dsp_saverec (" ");
	}
	Dsp_saverec ("                              ^1END OF SEARCH^6 ");
	Dsp_srch ();
	Dsp_close ();
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&trk_details);
}

void
CheckPageDisplay (
 void)
{
	linesPrintedCount++;
	if (linesPrintedCount > MAX_PRINTED_LINE)
		linesPrintedCount = 0;
}

/*====================================================
| Process Invoice Lines for given order hash (hhso). |
====================================================*/
void
ProcessOrderRef (
	long	hhcuHash,
	char	*orderRef,
	char	*consNo)
{
	int		breakOut	= 0,
			valid1		= 0,
			valid2		= 0;

	abc_selfield (sosf, "sosf_id_no4");

	sosf_rec.hhcu_hash	=	hhcuHash;
	sprintf (sosf_rec.cus_ord_ref, "%-20.20s", " ");
	cc = find_rec (sosf, &sosf_rec, GTEQ, "r");
	while (!cc && sosf_rec.hhcu_hash == hhcuHash)
	{
		valid1	=	check_search (sosf_rec.cus_ord_ref, orderRef, &breakOut);
		valid2	=	check_search (sosf_rec.cons_no, consNo, &breakOut);
		if (valid1 && valid2)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&trk_details, trkRec, trkCnt))
				sys_err ("ArrChkLimit (trkRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element trkCnt.
			 */
			sprintf (trkRec [trkCnt].sortCode, "%-20.20s%-16.16s",
							sosf_rec.cus_ord_ref, sosf_rec.cons_no);
			strcpy (trkRec [trkCnt].cusOrdRef, sosf_rec.cus_ord_ref);
			strcpy (trkRec [trkCnt].consNo, sosf_rec.cons_no);
			trkRec [trkCnt].hhcoHash = sosf_rec.hhco_hash;
			trkRec [trkCnt].hhsoHash = sosf_rec.hhso_hash;
			/*
			 * Increment array counter.
			 */
			trkCnt++;
		}
		cc = find_rec (sosf, &sosf_rec, NEXT, "r");
	}
	return;
}

void
PrintCompanyDetails (void)
{
	line_at (21,0,79);
	print_at (22,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	print_at (23,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
}

int 
TrkSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct TrkStruct a = * (const struct TrkStruct *) a1;
	const struct TrkStruct b = * (const struct TrkStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	clear ();

	rv_pr (ML ("Sales Order Tracking Display"),25,0,1);

	line_at (1,0,79);
	box (0,2,79,4);

	/*  reset this variable for _new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	PrintCompanyDetails ();
    return (EXIT_SUCCESS);
}
