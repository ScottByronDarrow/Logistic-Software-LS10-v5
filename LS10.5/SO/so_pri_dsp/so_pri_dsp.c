/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_pri_dsp.c,v 5.4 2002/11/28 04:09:46 scott Exp $
|  Program Name  : (so_pri_prt.c)
|  Program Desc  : (Pricing  structure Print)
|---------------------------------------------------------------------|
|  Author        : Choo.           | Date Written  : 04/12/90         |
|---------------------------------------------------------------------|
| $Log: so_pri_dsp.c,v $
| Revision 5.4  2002/11/28 04:09:46  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_pri_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_pri_dsp/so_pri_dsp.c,v 5.4 2002/11/28 04:09:46 scott Exp $";

#define	MAXWIDTH	150
#define	X_OFF		0		
#define	Y_OFF		4		
#define	CUST	 	(reportType == 1)
#define	CTYPE	 	(reportType == 2)
#define	ITEM	 	(reportType == 3)
#define	NEW_PR 		(reportType == 4)

#include	<pslscr.h>		
#include	<ml_std_mess.h>		
#include	<ml_so_mess.h>		
#include	<get_lpno.h>		
#include	<twodec.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include <arralloc.h>

#include	"schema"

struct commRecord	comm_rec;
struct exclRecord	excl_rec;
struct excfRecord	excf_rec;
struct exsfRecord	exsf_rec;
struct cumrRecord	cumr_rec;
struct incpRecord	incp_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocr_rec;
struct exafRecord	exaf_rec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [41];
	char	key			[sizeof incp_rec.key]; 
	char	status		[sizeof incp_rec.status]; 
	char	customerNo	[sizeof cumr_rec.dbt_no];
	char	custDesc	[sizeof cumr_rec.dbt_name];
	char	itemNo		[sizeof inmr_rec.item_no];
	char	itemDesc	[sizeof inmr_rec.description];
	char	currCode	[sizeof incp_rec.curr_code];
	char	custType	[sizeof incp_rec.cus_type];
	char	typeDesc	[sizeof excl_rec.class_desc];
	char	areaCode	[sizeof incp_rec.area_code];
	char	areaDesc	[sizeof exaf_rec.area];
	int		priceType;
	Date	dateFrom;
	Date	dateTo;
	Money	price1;
	Money	price2;
	Money	price3;
	Money	price4;
	Money	price5;
	Money	price6;
	Money	price7;
	Money	price8;
	Money	price9;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);


	char	*data = "data";

	int		reportType 		= 0,
			envDbFind 		= 0,
			envDbCo 		= 0,
			dataSaved		= FALSE,
			envSkDbPriNum	= 0,
			envDbMcurr		= 0;

	char	branchNo 		[3],
			displayLine 	[250],
			displayHeading 	[250],
			Curr_code 		[4];

	char rep_desc [4][101];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	repSelect [2];
	char	repSelectDesc [15];
	char	currCode [4];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "price",	 3, 21, CHARTYPE,
		"U", "          ",
		" ", "C", "Price By.   ", "C-Customer, T-Type, I-Item.",
		YES, NO,  JUSTLEFT, "CTNI", "", local_rec.repSelect},
	{1, LIN, "priceDesc",	 3, 24, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.repSelectDesc},
	{1, LIN, "currCode",	 3, 120, CHARTYPE,
		"UUU", "          ",
		" ", Curr_code, "Currency.   ", "Enter currency to display.",
		ND, NO,  JUSTLEFT, "", "", local_rec.currCode},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*
 * Function Declarations 
 */
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	shutdown_prog 		(void);
int  	spec_valid 			(int);
void 	SrchPocr 			(char *);
int  	heading 			(int scn);
void 	ProcessData 		(void);
int  	FindItem 			(long);
int  	FindCustomer 		(long);
int  	FindType 			(char *);
int  	FindArea 			(char *);
void 	ProcessIncp 		(long, char *, char *);
void 	MainDisplayFunc 	(void);
void 	DisplayCustomer 	(void);
void 	DisplayClassType 	(void);
void 	DisplayItem 		(void);
void 	InitML 				(void);

/*
 * Main Processing Routine 
 */
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		envSkDbPriNum = 5;
	else
	{
		envSkDbPriNum = atoi (sptr);
		if (envSkDbPriNum > 9 || envSkDbPriNum < 1)
			envSkDbPriNum = 9;
	}

	/*
	 * Get native currency. 
	 */
	sprintf (Curr_code, "%-3.3s", get_env ("CURR_CODE"));

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	SETUP_SCR (vars);

	InitML ();
	init_scr ();			
	set_tty ();            
	set_masks ();		
	init_vars (1);	

	if (envDbMcurr)
		FLD ("currCode") = YES;

	envDbFind = atoi (get_env ("DB_FIND"));
	envDbCo = atoi (get_env ("DB_CO"));

	swide ();

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	/*
	 * Reset control flags 
	 */
   	entry_exit = 1;
   	prog_exit = 0;
   	restart = 0;

	while (prog_exit == 0)
	{
		/*
		 * Reset Control flags  
		 */
		entry_exit = FALSE;
		edit_exit  = FALSE;
		prog_exit  = FALSE;
		restart    = FALSE;
		init_ok    = TRUE;
		search_ok  = TRUE;
		dataSaved = FALSE;

		/*
		 * Entry screen 1 linear input.
		 */
		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;
		
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
            break; /* <-- new line */

		ProcessData ();
		if (sortCnt)
			MainDisplayFunc ();  
		else
		{
			print_mess (ML (mlStdMess211));
			sleep (sleepTime);	
			clear_mess ();
		}
		if (restart)
            break; /* <-- new line */
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Open database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incp, incp_list, INCP_NO_FIELDS, "incp_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
}

/*
 * Close database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (excl);
	abc_fclose (excf);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (incp);
	abc_fclose (pocr);
	abc_fclose (exaf);
	abc_dbclose (data);
}

/*
 * Get common info from commom database file . 
 */
void
ReadMisc (void)
{
	open_rec (comm, comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	if (find_rec (ccmr, &ccmr_rec, COMPARISON, "r"))
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Field Selection group 1. 
	 */
	if (LCHECK ("price")) 
	{
		switch (local_rec.repSelect [0])
		{
		case 'C':
			strcpy (local_rec.repSelectDesc, ML ("Customer      "));
			reportType = 1;
			break;
		case 'T':
			strcpy (local_rec.repSelectDesc, ML ("Customer Type "));
			reportType = 2;
			break;
		case 'I':
			strcpy (local_rec.repSelectDesc, ML ("Item          "));
			reportType = 3;
			break;
		case 'A':
			strcpy (local_rec.repSelectDesc, ML ("Customer/Area "));
			reportType = 4;
		}
		DSP_FLD ("priceDesc");
				
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("currCode"))
	{
		if (FLD ("currCode") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
			strcpy (local_rec.currCode, Curr_code);

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code, "%-3.3s", local_rec.currCode);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchPocr (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Currency Description");

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	sprintf (pocr_rec.code ,"%-3.3s", key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pocr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (pocr_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	sprintf (pocr_rec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSoMess187),48,0,1);

		box (0,2,132,1);
		line_at (1,0,132);
		line_at (20,0,132);

		box (0,2,132,1);

		print_at (21,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
ProcessData (void)
{
	char	tmp_curr [4];

	if (envDbMcurr)
		sprintf (tmp_curr, "%-3.3s", local_rec.currCode);
	else
		sprintf (tmp_curr, "%-3.3s", Curr_code);

	sprintf (incp_rec.key, "%2.2s    ", comm_rec.co_no);
	strcpy (incp_rec.curr_code, tmp_curr);
	strcpy (incp_rec.cus_type,"   ");
	strcpy (incp_rec.area_code,"  ");
	incp_rec.hhbr_hash 	= 0L;
	incp_rec.hhcu_hash 	= 0L;
	incp_rec.date_from 	= 0L;
	incp_rec.date_to 	= 0L;
	strcpy (incp_rec.status, (NEW_PR) ? "N" : "A");
	cc = find_rec (incp, &incp_rec, GTEQ, "r");
	while (!cc && !strncmp (incp_rec.key, comm_rec.co_no, 2))
	{
		if ((incp_rec.date_to < comm_rec.inv_date && !NEW_PR) ||
	       (strcmp (incp_rec.curr_code, tmp_curr)))
		{
			cc = find_rec (incp, &incp_rec, NEXT, "r");
			continue;
		}

		if (CUST)
		{
			if (incp_rec.hhcu_hash == 0L)
			{
				cc = find_rec (incp, &incp_rec, NEXT, "r");
				continue;
			}
			ProcessIncp (incp_rec.hhcu_hash, " ", " ");
		}

		if (CTYPE)
		{
			if (!strcmp (incp_rec.cus_type,"   "))
			{
				cc = find_rec (incp, &incp_rec, NEXT, "r");
				continue;
			}
			ProcessIncp (0L, " ", incp_rec.cus_type);
		}
		if (ITEM || NEW_PR)
		{
			if (strcmp (incp_rec.cus_type,"   ") ||
			     incp_rec.hhbr_hash == 0L ||
			     incp_rec.hhcu_hash != 0L)
			{
				cc = find_rec (incp, &incp_rec, NEXT, "r");
				continue;
			}
			ProcessIncp (0L, " ", " ");
		}
		cc = find_rec (incp, &incp_rec, NEXT, "r");
	}
}

/*
 * Find item using hash. 
 */
int
FindItem (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash = hhbrHash;
	return (find_rec (inmr, &inmr_rec, COMPARISON, "r"));
}

/*
 * Find Customer using hash. 
 */
int
FindCustomer (
	long	hhcuHash)
{
	cumr_rec.hhcu_hash	=	hhcuHash;
	return (find_rec (cumr, &cumr_rec, COMPARISON, "r"));
}

/*
 * Find Customer Type. 
 */
int
FindType (
	char	*custType)
{
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", custType);
	return (find_rec (excl, &excl_rec, COMPARISON, "r"));
}

/*
 * Find Area Code.
 */
int
FindArea (
	char	*areaCode)
{
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", areaCode);
	return (find_rec (exaf, &exaf_rec, COMPARISON, "r"));
}

void
ProcessIncp (
	long	hhcuHash, 
	char 	*areaCode,
	char 	*custType)
{
	if (!dataSaved)
	{
		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
		sortCnt = 0;
		dataSaved = TRUE;
	}

	if (incp_rec.hhcu_hash != 0L)
	{
		if (FindCustomer (incp_rec.hhcu_hash))
			return;
	}

	if (strcmp (incp_rec.area_code, "  "))
	{
		if (FindArea (incp_rec.area_code))
			return;
	}

	if (strcmp (incp_rec.cus_type, "   "))
	{
		if (FindType (incp_rec.cus_type))
			return;
	}

	if (FindItem (incp_rec.hhbr_hash))
		return;

	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	/*
	 * Load values into array element sortCnt.
	 */
	if (CUST)
	{
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%-6.6s%-16.16s%2.2s",
			cumr_rec.dbt_no, 
			inmr_rec.item_no, 
			incp_rec.area_code
		);
	}
	if (CTYPE)
	{
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%-3.3s%-16.16s",
			incp_rec.cus_type,
			inmr_rec.item_no
		);
	}
	if (ITEM || NEW_PR)
	{
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%-16.16s",
			inmr_rec.item_no
		);
	}
	sprintf (sortRec [sortCnt].key, 		"%-6.6s",	incp_rec.key);
	sprintf (sortRec [sortCnt].status, 		"%-1.1s", 	incp_rec.status);
	sprintf (sortRec [sortCnt].customerNo,	"%-6.6s",	cumr_rec.dbt_no);
	sprintf (sortRec [sortCnt].custDesc ,	"%-40.40s",	cumr_rec.dbt_name);
	sprintf (sortRec [sortCnt].itemNo	, 	"%-16.16s", inmr_rec.item_no);
	sprintf (sortRec [sortCnt].itemDesc	, 	"%-40.40s", inmr_rec.description);
	sprintf (sortRec [sortCnt].currCode, 	"%-3.3s", 	incp_rec.curr_code);
	sprintf (sortRec [sortCnt].custType, 	"%-3.3s", 	incp_rec.cus_type);
	sprintf (sortRec [sortCnt].typeDesc, 	"%-40.40s",	excl_rec.class_desc);
	sprintf (sortRec [sortCnt].areaCode, 	"%-2.2s", 	incp_rec.area_code);
	sprintf (sortRec [sortCnt].areaDesc, 	"%-40.40s",	exaf_rec.area);
	sortRec [sortCnt].priceType		=	atoi (cumr_rec.price_type);
	sortRec [sortCnt].dateFrom		=	incp_rec.date_from;
	sortRec [sortCnt].dateTo		=	incp_rec.date_to;
	sortRec [sortCnt].price1		=	DOLLARS (incp_rec.price1);
	sortRec [sortCnt].price2		=	DOLLARS (incp_rec.price2);
	sortRec [sortCnt].price3		=	DOLLARS (incp_rec.price3);
	sortRec [sortCnt].price4		=	DOLLARS (incp_rec.price4);
	sortRec [sortCnt].price5		=	DOLLARS (incp_rec.price5);
	sortRec [sortCnt].price6		=	DOLLARS (incp_rec.price6);
	sortRec [sortCnt].price7		=	DOLLARS (incp_rec.price7);
	sortRec [sortCnt].price8		=	DOLLARS (incp_rec.price8);
	sortRec [sortCnt].price9		=	DOLLARS (incp_rec.price9);
	/*
	 * Increment array counter.
	 */
	sortCnt++;
	print_at (0,0, "SortCnt [%d]", sortCnt);getchar ();
	return;
}

void
MainDisplayFunc (void)
{
	int		i;
	char	wsWork [12];

	rv_pr (rep_desc [ reportType - 1],50,3,1);

	sprintf (displayHeading," Co Br Wh|%-10.10s", comm_rec.price1_desc);

	for (i = 1; i < envSkDbPriNum; i++)
	{
		switch (i)
		{
			case	1:
				sprintf (wsWork, "|%-10.10s", comm_rec.price2_desc);
				strcat (displayHeading, wsWork);
			break;

			case	2:
				sprintf (wsWork, "|%-10.10s", comm_rec.price3_desc);
				strcat (displayHeading, wsWork);
			break;

			case	3:
				sprintf (wsWork, "|%-10.10s", comm_rec.price4_desc);
				strcat (displayHeading, wsWork);
			break;

			case	4:
				sprintf (wsWork, "|%-10.10s", comm_rec.price5_desc);
				strcat (displayHeading, wsWork);
			break;

			case	5:
				sprintf (wsWork, "|%-10.10s", comm_rec.price6_desc);
				strcat (displayHeading, wsWork);
			break;

			case	6:
				sprintf (wsWork, "|%-10.10s", comm_rec.price7_desc);
				strcat (displayHeading, wsWork);
			break;

			case	7:
				sprintf (wsWork, "|%-10.10s", comm_rec.price8_desc);
				strcat (displayHeading, wsWork);
			break;

			case	8:
				sprintf (wsWork, "|%-10.10s", comm_rec.price9_desc);
				strcat (displayHeading, wsWork);
			break;
		}
	}
	strcat (displayHeading, "|Date From.| Date to. ");

	for (i = envSkDbPriNum; i < 9; i++)
		strcat (displayHeading, "           ");

	if (CUST)
		DisplayCustomer ();

	if (CTYPE)
		DisplayClassType ();

	if (ITEM || NEW_PR)
		DisplayItem ();

	Dsp_srch ();
	Dsp_close ();

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
DisplayCustomer (void)
{
	int		i;

	long	prevHhbrHash 	= 0L,
			prevHhcuHash 	= 0L,
			hhbrHash		= 0L,
			hhcuHash		= 0L;

	char	dateFrom	[11],
			dateTo		[11];

	double	customerPrice	=	0.00;
	int		firstTime 		= TRUE;

	Dsp_prn_open 
	(
		0, 4, 14, 
		rep_desc [ reportType - 1],
		comm_rec.co_no,
		comm_rec.co_name,
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec (" Item Number    |   Item  Description                    |Area| Co Br Wh |Customer Price | Date From| Date to  ");
	Dsp_saverec ("");
	Dsp_saverec (" [Print] [Next] [Prev] [End]");

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		customerPrice = 0.00;
		if (sortRec [i].priceType == 1)
			customerPrice	=	sortRec [i].price1;
		else if (sortRec [i].priceType == 2)
			customerPrice	=	sortRec [i].price2;
		else if (sortRec [i].priceType == 3)
			customerPrice	=	sortRec [i].price3;
		else if (sortRec [i].priceType == 4)
			customerPrice	=	sortRec [i].price4;
		else if (sortRec [i].priceType == 5)
			customerPrice	=	sortRec [i].price5;
		else if (sortRec [i].priceType == 6)
			customerPrice	=	sortRec [i].price6;
		else if (sortRec [i].priceType == 7)
			customerPrice	=	sortRec [i].price7;
		else if (sortRec [i].priceType == 8)
			customerPrice	=	sortRec [i].price8;
		else if (sortRec [i].priceType == 9)
			customerPrice	=	sortRec [i].price9;

		if (firstTime || hhcuHash != prevHhcuHash)
		{
			if (!firstTime)
				Dsp_saverec ("^^GGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGHGGGGGGGGGGHGGGGGGGGGGGGGGGHGGGGGGGGGGHGGGGGGGGGGH");

			sprintf (displayLine, "%-6.6s (%-40.40s)        ^E    ^E          ^E               ^E          ^E",
				sortRec [i].customerNo, sortRec [i].custDesc);

			Dsp_saverec (displayLine);
			firstTime = FALSE;
		}
		strcpy (dateFrom, DateToString (sortRec [i].dateFrom));
		strcpy (dateTo,   DateToString (sortRec [i].dateTo));

		sprintf 
		(
			displayLine, 
			"%-16.16s^E%-40.40s^E %2.2s ^E %2.2s %2.2s %2.2s ^E  %12.2f ^E%-10.10s^E%-10.10s",
			sortRec [i].itemNo, 
			sortRec [i].itemDesc,
			sortRec [i].areaCode,
			sortRec [i].key, 
			sortRec [i].key + 2, 
			sortRec [i].key + 4, 
			customerPrice,
			dateFrom, 
			dateTo
		);

		Dsp_saverec (displayLine);

		prevHhbrHash = hhbrHash;
		prevHhcuHash = hhcuHash;
	}
}

void
DisplayClassType (void)
{
	int		i;
	int		j;
	char	prevItem 	[17];
	char	currItem 	[17];
	char	prevType 	[4];
	char	custType 	[4];
	double	price 		[9];
	int		firstTime = TRUE;
	char	wsWork [30];
	char	uline [140];
	char	dateFrom	[11],
			dateTo		[11];

	Dsp_prn_open 
	(
		0, 4, 14, 
		rep_desc [ reportType - 1],
		comm_rec.co_no,
		comm_rec.co_name,
		(char *) 0, 
		(char *) 0, 
		(char *) 0, 
		(char *) 0
	);

	Dsp_saverec (displayHeading);
	Dsp_saverec ("");
	Dsp_saverec (" [Print] [Next] [Prev] [End]");

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		sprintf (custType, "%-3.3s", 	sortRec [i].custType);
		sprintf (currItem, "%-16.16s",  sortRec [i].itemNo);
		price [0] = sortRec [i].price1;
		price [1] = sortRec [i].price2;
		price [2] = sortRec [i].price3;
		price [3] = sortRec [i].price4;
		price [4] = sortRec [i].price5;
		price [5] = sortRec [i].price6;
		price [6] = sortRec [i].price7;
		price [7] = sortRec [i].price8;
		price [8] = sortRec [i].price9;

		if (firstTime || strcmp (prevType, custType) || strcmp (prevItem, currItem))
		{
			if (!firstTime)
			{
				strcpy (uline, "^^GGGGGGGGGHGGGGGGGGGGH");
				for (j = 1; j < 11; j++)
				{
					if (j < envSkDbPriNum + 2)
						strcat (uline, "GGGGGGGGGGH");
					else
						strcat (uline, "GGGGGGGGGGG");
				}
				Dsp_saverec (uline);
			}
			sprintf 
			(
				displayLine, " %-3.3s (%-40.40s) %-16.16s (%-40.40s)",
				sortRec [i].custType,
				sortRec [i].typeDesc,
				currItem,
				sortRec [i].itemDesc
			);
			Dsp_saverec (displayLine);
			firstTime = FALSE;
		}

		sprintf 
		(
			displayLine, 
			"%2.2s %2.2s %2.2s ^E%10.2f",
			sortRec [i].key,
			sortRec [i].key + 2,
			sortRec [i].key + 4,
			price [0]
		);
		
		for (j = 1; j < envSkDbPriNum; j++)
		{
			sprintf (wsWork, "^E%10.2f", price [j]);
			strcat (displayLine, wsWork);
		}
		strcpy (dateFrom, DateToString (sortRec [i].dateFrom));
		strcpy (dateTo,   DateToString (sortRec [i].dateTo));

		sprintf (wsWork, "^E%-10.10s^E%-10.10s^E", dateFrom, dateTo);
		strcat (displayLine, wsWork);

		Dsp_saverec (displayLine);

		strcpy (prevType, custType);
		strcpy (prevItem, currItem);
	}
}

void
DisplayItem (void)
{
	int		i;
	int		j;
	long	hhbrHash	 = 0L,
			prevHhbrHash = 0L;
	double	price [9];
	char	wsWork [30];
	char	uline [140];
	int		firstTime = TRUE;

	char	dateFrom	[11],
			dateTo		[11];

	Dsp_prn_open (0, 4, 14, 
				 rep_desc [ reportType - 1],
			     comm_rec.co_no,
				 comm_rec.co_name,
				 (char *) 0, 
				 (char *) 0, 
				 (char *) 0, 
				 (char *) 0);

	Dsp_saverec (displayHeading);
	Dsp_saverec ("");
	Dsp_saverec (" [Print] [Next] [Prev] [End]");

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		price [0]  	= sortRec [i].price1;
		price [1]  	= sortRec [i].price2;
		price [2]  	= sortRec [i].price3;
		price [3]  	= sortRec [i].price4;
		price [4]  	= sortRec [i].price5;
		price [5]  	= sortRec [i].price6;
		price [6]  	= sortRec [i].price7;
		price [7]  	= sortRec [i].price8;
		price [8] 	= sortRec [i].price9;


		if (firstTime || prevHhbrHash != hhbrHash)
		{
			if (!firstTime)
			{
				strcpy (uline, "^^GGGGGGGGGHGGGGGGGGGGH");
				for (j = 1; j < 11; j++)
				{
					if (j < envSkDbPriNum + 2)
						strcat (uline, "GGGGGGGGGGH");
					else
						strcat (uline, "GGGGGGGGGGG");
				}
				Dsp_saverec (uline);
			}
			sprintf (displayLine, " %-16.16s (%-40.40s)", 
					sortRec [i].itemNo, sortRec [i].itemDesc);

			Dsp_saverec (displayLine);
			firstTime = FALSE;
		}

		sprintf (displayLine, "%-2.2s %-2.2s %-2.2s ^E%10.2f",
			sortRec [i].key, 
			sortRec [i].key + 2, 
			sortRec [i].key + 4, 
			price [0]);
		
		for (j = 1; j < envSkDbPriNum; j++)
		{
			sprintf (wsWork, "^E%10.2f", price [j]);
			strcat (displayLine, wsWork);
		}
		strcpy (dateFrom, DateToString (sortRec [i].dateFrom));
		strcpy (dateTo,   DateToString (sortRec [i].dateTo));

		sprintf (wsWork, "^E%-10.10s^E%-10.10s^E", dateFrom, dateTo);
		strcat (displayLine, wsWork);

		Dsp_saverec (displayLine);

		prevHhbrHash = hhbrHash;
	}
}

void
InitML (void)
{
	strcpy (rep_desc [0], ML (mlSoMess390));
	strcpy (rep_desc [1], ML (mlSoMess391));
	strcpy (rep_desc [2], ML (mlSoMess392));
	strcpy (rep_desc [3], ML (mlSoMess393));
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}
