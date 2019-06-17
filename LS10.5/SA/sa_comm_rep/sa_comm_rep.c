/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sa_comm_rep.c,v 5.4 2002/07/18 07:55:24 scott Exp $
|  Program Name  : (sa_comm_rep.c) 
|  Program Desc  : (Sales commission report and display)
|---------------------------------------------------------------------|
|  Date Written  : (07/07/1998)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: sa_comm_rep.c,v $
| Revision 5.4  2002/07/18 07:55:24  scott
| Updated to fix invalid printer problem.
|
| Revision 5.3  2002/07/17 09:57:46  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2002/03/06 05:49:35  scott
| S/C 00790 - SAMK14- Display Sales Commissions; WINDOWS CLIENT  (1)  When <enter> is pressed at first field 'Start Sales Person', default is space, then press <enter> in all fields.  .  But when focus goes back to first field, press F12, the error message is 'Salesman Not Found'  CHAR-BASED / WINDOWS CLIENT
| (2) Other characters are accepted by the field Display/Print.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_comm_rep.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_comm_rep/sa_comm_rep.c,v 5.4 2002/07/18 07:55:24 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_sa_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <arralloc.h>

#define	P_SIZE		15
#define	DSP_SCREEN	 (local_rec.disppr [0] == 'D')
#define	PRINTER		 (local_rec.disppr [0] != 'D')
#define	SUMMARY		 (local_rec.detSumFlag [0] == 'S')

#define	CF			comma_fmt


	FILE	*fout;

char 	*data	=	"data", 
		*exsf2 	= 	"exsf2";
	

	char	*sptr; 

	char	prevSman [3], 
			currSman [3], 
			prevDesc [41],
			branchNo [3];
		
	int 	envDbCo 		= 0, 
			envDbFind 		= 0,
			envSaCommPayment = 1,
			envSaCommission = 0;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct exsfRecord	exsf_rec;
struct exsfRecord	exsf2_rec;
struct saclRecord	sacl_rec;
struct sachRecord	sach_rec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[27];
	char	salesman	[3];
	char	acronym		[10];
	char	invNo		[9];
	char	recNo		[9];
	char	saleFlag	[2];
	long	invDate;
	long	recDate;
	long	hhsfHash;
	Money	invAmt;
	Money	recAmt;
	Money	commAmt;
	Money	commVal;
	float	commRate;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *, 	const void *);

/*
 *	Screen Input Variables
 */
struct {
	   char 	startSman 		[3];
	   char 	endSman 		[3];
	   char		detSumFlag 		[2];
	   char		startSmanDesc 	[41];
	   char		endSmanDesc 	[41];
	   char		detSumDesc 		[41];
       char     disppr 			[2];
       char     systemDate 		[11];
	   char		dummy 			[2];
	   long 	endDate;
	   long     startDate;
       int      printerNo;
	   } local_rec;

int	printerNo;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "startSman", 	 4, 2, CHARTYPE, 
		"UU", "        ", 
		" ", " ", "Start Sales Person  ", "Default is ALL", 
		YES, NO, JUSTLEFT, "", "", local_rec.startSman}, 
	{1, LIN, "startSmanDesc", 	 4, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Salesman Name       ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.startSmanDesc}, 
	{1, LIN, "endSman", 	 5, 2, CHARTYPE, 
		"UU", "        ", 
		" ", "0", "End Sales Person    ", "Default is ALL", 
		YES, NO, JUSTLEFT, "", "", local_rec.endSman}, 
	{1, LIN, "endSmanDesc", 	 5, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Salesman Name       ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.endSmanDesc}, 
	{1, LIN, "det_or_sum", 	 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Detail/Summary      ", " Enter S(ummary) or D(etail) ", 
		 NO, NO, JUSTLEFT, "DS", "", local_rec.detSumFlag}, 
	{1, LIN, "DorS_desc", 	 7, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "S", "", "", 
		 NA, NO, JUSTLEFT, "", "", local_rec.detSumDesc}, 
	{1, LIN, "start_date", 	8, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Start Date          ", " Default is Start of month. ", 
		 NO, NO, JUSTLEFT, " ", "", (char *)&local_rec.startDate}, 
	{1, LIN, "end_date", 	8, 60, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "End Date            ", " Default is Date today.  ", 
		 NO, NO, JUSTLEFT, " ", "", (char *)&local_rec.endDate}, 
	{1, LIN, "disppr", 		 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Display/Print       ", "D(isplay, P(rint   ", 
		NO, NO, JUSTRIGHT, "DP", "", local_rec.disppr}, 
	{1, LIN, "printerNo", 	10, 60, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number      ", "Printer Number     ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	InitOutput 		(void);
int 	heading 		(int);
void 	SetDefaults 	(void);
void 	ProcessData 	(void);
void 	PrintData 		(void);
int 	DisplayData 	(void);
void 	SrchExsf 		(char *);

/*
 * Main Processing Routine                                            
 */
int
main (
 int    argc, 
 char*  argv [])
{
	char*   sptr;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	sptr = chk_env ("SA_COMM_PAYMENT");
	envSaCommPayment = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SA_COMMISSION");
	envSaCommission = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();


	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_ok 	= TRUE;
		prog_status = FALSE;
		init_vars (1);	

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
		InitOutput ();

		ProcessData ();

		if (PRINTER)
			PrintData ();
		else
			DisplayData ();

		if (PRINTER)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	abc_alias (exsf2, exsf);
	 
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_hhsf_hash");
	open_rec (sacl, sacl_list, SACL_NO_FIELDS, "sacl_sach_hash");
	open_rec (sach, sach_list, SACH_NO_FIELDS, "sach_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exsf2, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	abc_fclose (exsf);
	abc_fclose (sacl);
	abc_fclose (sach);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (esmr);
	abc_fclose (exsf2);
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{
 	/*
  	 * Validate Starting Salesman  
  	 */
	if (LCHECK ("startSman")) 
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strcmp (local_rec.startSman, "  "))
		{
			sprintf (local_rec.startSman, 	"%-2.2s", "  ");
			sprintf (local_rec.endSman, 	"%-2.2s", "~~");

			sprintf (local_rec.startSmanDesc, "%-40.40s", 	ML (mlStdMess172));
			sprintf (local_rec.endSmanDesc,   "%-40.40s", 	ML (mlStdMess172));

			FLD ("endSman") = NA;

			DSP_FLD ("startSman");
			DSP_FLD ("endSman");
			DSP_FLD ("startSmanDesc");
			DSP_FLD ("endSmanDesc");
			return (EXIT_SUCCESS);
		}
		FLD ("endSman") = YES;
		
		if (prog_status != ENTRY && 
		    strncmp (local_rec.endSman, "~~", 2) && 
		    strcmp (local_rec.startSman, local_rec.endSman) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}


		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.startSman);

		cc = find_rec (exsf2, &exsf_rec, COMPARISON, "r");	
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.startSman, "%2.2s", exsf_rec.salesman_no);
		strcpy (local_rec.startSmanDesc, exsf_rec.salesman);
		
		DSP_FLD ("startSman");
		DSP_FLD ("startSmanDesc");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Salesman  
	 */
	if (LCHECK ("endSman")) 
	{
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strcmp (local_rec.endSman, "~~"))
		{
			sprintf (local_rec.endSman, "%-2.2s", "~~");
			sprintf (local_rec.startSmanDesc, "%-40.40s", ML (mlStdMess172));
			cc = find_rec (exsf2, &exsf_rec, LAST, "r");
			DSP_FLD ("endSman");
			DSP_FLD ("endSmanDesc");
			cc = find_rec (exsf2, &exsf_rec, LAST, "r");
			return (EXIT_SUCCESS);
		}


		if (strncmp (local_rec.startSman, "  ", 2) == 0 && 
		    strncmp (local_rec.endSman, "~~", 2)) 
		{
			print_mess (ML (mlStdMess172));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (exsf_rec.co_no, comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.endSman);
		cc = find_rec (exsf2, &exsf_rec, COMPARISON, "r");	
		if (cc)
		{
			print_mess (ML (mlStdMess135));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.endSman, "%2.2s", exsf_rec.salesman_no);
		strcpy (local_rec.endSmanDesc, exsf_rec.salesman);

		if (strcmp (local_rec.startSman, local_rec.endSman) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("endSman");
		DSP_FLD ("endSmanDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("det_or_sum"))
	{
		if (local_rec.detSumFlag [0] == 'S') 
		{
			strcpy (local_rec.detSumDesc, "Summary");
			DSP_FLD ("DorS_desc");
			return (EXIT_SUCCESS);
		}
		else 
		{
			strcpy (local_rec.detSumDesc, "Detail");
			DSP_FLD ("DorS_desc");
			return (EXIT_SUCCESS);
		}
	}

 	/*
  	 * Validate Date Entered 		
  	 */
	if (LCHECK ("start_date"))
	{
		if (dflt_used) 
			local_rec.startDate = MonthStart (StringToDate (local_rec.systemDate));

		if (local_rec.startDate > StringToDate (local_rec.systemDate))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

 	/*
  	 * Validate Date Entered 		
  	 */
	if (LCHECK ("end_date"))
	{
		if (dflt_used) 
			local_rec.endDate = StringToDate (local_rec.systemDate);

		if (local_rec.endDate < local_rec.startDate)
		{
			print_mess (ML (mlStdMess013));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		if (local_rec.endDate > StringToDate (local_rec.systemDate))
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("end_date");
		return (EXIT_SUCCESS);
	}


    /*
     * Validate Printer/Display Option
     */
	if (LCHECK ("disppr"))
	{
		if (DSP_SCREEN)
		{
			local_rec.printerNo 	= 	0;
			FLD ("printerNo")		=	NA;
		}
		else
		{
			FLD ("printerNo")		=	YES;
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Printer Number
	 */
	if (LCHECK ("printerNo"))
	{
		if (dflt_used || F_NOKEY (label ("printerNo")))
		{
			local_rec.printerNo = 1;
			return (EXIT_SUCCESS);
		}	

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

/*
 * Initialize for Screen or Printer Output.
 */
void
InitOutput (void)
{
	char	wkDate [2][11];

	strcpy (wkDate [0], DateToString (local_rec.startDate));
	strcpy (wkDate [1], DateToString (local_rec.endDate));

	if (PRINTER)
	{
		dsp_screen (" Printing Sales Commissions", 
					comm_rec.co_no, comm_rec.co_name);

		/*
		 * Open format file
		 */
		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.printerNo);
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L%d\n", (SUMMARY) ? 100 : 134);
		fprintf (fout, ".11\n");
		fprintf (fout, ".B1\n");
		fprintf (fout, ".ESALESMAN COMMISSION LISTING\n");
		fprintf (fout, ".B1\n");
		fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
		fprintf (fout, ".EAS AT %s\n", SystemTime ());
		fprintf (fout, ".C Start Date : %10.10s  /  End Date : %10.10s  /  Start Salesperson : %2.2s  /  End Salesperson : %2.2s  /  Report is %9.9s\n", 
				wkDate [0], wkDate [1], 
				local_rec.startSman, local_rec.endSman, 
				 (SUMMARY) ? "Summary" : "Detailed");
		
		if (SUMMARY)
		{
			fprintf (fout, ".R=====");
			fprintf (fout, "===========================================");
			fprintf (fout, "=================================================\n");

			fprintf (fout, "=====");
			fprintf (fout, "===========================================");
			fprintf (fout, "=================================================\n");

			fprintf (fout, "| SM ");
			fprintf (fout, "|      SALES PERSON DESCRIPTION.           ");
			fprintf (fout, "|     INVOICE   ");
			fprintf (fout, "|     AMOUNT    ");
			fprintf (fout, "|   COMMISSION  |\n");

			fprintf (fout, "| NO ");
			fprintf (fout, "|                                          ");
			fprintf (fout, "|      TOTAL    ");
			fprintf (fout, "|    COLLECTED  ");
			fprintf (fout, "|     AMOUNT    |\n");

			fprintf (fout, "|----");
			fprintf (fout, "|------------------------------------------");
			fprintf (fout, "|---------------");
			fprintf (fout, "|---------------");
			fprintf (fout, "|---------------|\n");
		}
		else
		{
			fprintf (fout, ".R===========");
			fprintf (fout, "===========");
			fprintf (fout, "=============");
			fprintf (fout, "===========");
			fprintf (fout, "================");
			fprintf (fout, "=========");
			fprintf (fout, "=============");
			fprintf (fout, "================");
			fprintf (fout, "============");
			fprintf (fout, "=================\n");

			fprintf (fout, "===========");
			fprintf (fout, "===========");
			fprintf (fout, "=============");
			fprintf (fout, "===========");
			fprintf (fout, "================");
			fprintf (fout, "=========");
			fprintf (fout, "=============");
			fprintf (fout, "================");
			fprintf (fout, "============");
			fprintf (fout, "=================\n");

			fprintf (fout, "|COMMISSION");
			fprintf (fout, "| INVOICE  ");
			fprintf (fout, "|  INVOICE   ");
			fprintf (fout, "| CUSTOMER ");
			fprintf (fout, "|     INVOICE   ");
			fprintf (fout, "|RECEIPT ");
			fprintf (fout, "|  RECEIPT   ");
			fprintf (fout, "|     AMOUNT    ");
			fprintf (fout, "|COMMISSION ");
			fprintf (fout, "|   COMMISSION  |\n");

			fprintf (fout, "|  LEVEL   ");
			fprintf (fout, "|  NUMBER  ");
			fprintf (fout, "|    DATE    ");
			fprintf (fout, "|  ACRONYM ");
			fprintf (fout, "|     AMOUNT    ");
			fprintf (fout, "| NUMBER ");
			fprintf (fout, "|    DATE    ");
			fprintf (fout, "|    COLLECTED  ");
			fprintf (fout, "|    RATE   ");
			fprintf (fout, "|     AMOUNT    |\n");

			fprintf (fout, "|----------");
			fprintf (fout, "|----------");
			fprintf (fout, "|------------");
			fprintf (fout, "|----------");
			fprintf (fout, "|---------------");
			fprintf (fout, "|--------");
			fprintf (fout, "|------------");
			fprintf (fout, "|---------------");
			fprintf (fout, "|-----------");
			fprintf (fout, "|---------------|\n");
		}
	}
	else
	{
		clear ();

 		rv_pr (ML ("Salesman Commission Display"), 50, 0, 1); 

		move (0, 1);
		line (132);

		strcpy (err_str, ML ("Start Date : %10.10s  /  End Date : %10.10s  /  Start Salesperson : %2.2s  /  End Salesperson : %2.2s  /  Report is %9.9s"));
		print_at (2, 0, err_str, 
				wkDate [0], wkDate [1], 
				local_rec.startSman, local_rec.endSman, 
				 (SUMMARY) ? "Summary" : "Detailed");
	}
}


int
heading (
 int    scn)
{
	if (restart) 
		return (EXIT_FAILURE);
	
	swide ();
	clear ();

	strcpy (err_str, ML (" Sales Commission Report "));

	rv_pr (err_str, (130 - strlen (err_str)) / 2, 0, 1);

	line_at (1, 0, 132);

	switch (scn)
	{
	case	1:
		box (0, 3, 132, 7);
		line_at (6, 1, 131);
		line_at (9, 1, 131);
		line_at (21, 0, 132);
		break;

	default:
		break;
	}

	line_cnt	=	0;
	scn_write (scn);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

    return (EXIT_SUCCESS);
}

void
SetDefaults (void)
{
	strcpy (local_rec.startSman, 	"  ");
	strcpy (local_rec.endSman, 		"~~");
	strcpy (local_rec.detSumFlag, 	"S");
	strcpy (local_rec.startSmanDesc, 	ML ("All Salespersons"));
	strcpy (local_rec.endSmanDesc, 		ML ("All Salespersons"));
	strcpy (local_rec.detSumDesc, 		ML ("Summary"));
	local_rec.startDate 	= StringToDate (local_rec.systemDate);
	local_rec.endDate 		= StringToDate (local_rec.systemDate);
	local_rec.printerNo 	= 1;
	strcpy (local_rec.disppr, "D");
	DSP_FLD ("startSman");
	DSP_FLD ("start_date");
	DSP_FLD ("end_date");
	DSP_FLD ("startSmanDesc");
	DSP_FLD ("endSman");
	DSP_FLD ("endSmanDesc");
	DSP_FLD ("det_or_sum");
	DSP_FLD ("DorS_desc");
	DSP_FLD ("disppr");
	FLD ("endSman") 	= NA;
	FLD ("printerNo") 	= NA;

}

void
ProcessData (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	if (!strcmp (local_rec.startSman, "  "))
	{
		strcpy (local_rec.startSman, "  ");
		strcpy (local_rec.endSman, "~~");
	}

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	strcpy (exsf_rec.salesman_no, local_rec.startSman);
	cc = find_rec (exsf2, &exsf_rec, GTEQ, "r");	

	while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (exsf_rec.salesman_no, local_rec.endSman) > 0)
			break;
	
		if (exsf_rec.com_status [0] != 'C')
		{
			cc = find_rec (exsf2, &exsf_rec, NEXT, "r");	
			continue;
		}

		sach_rec.hhsf_hash = exsf_rec.hhsf_hash;
		sach_rec.hhcu_hash	=	0L;
		sach_rec.hhci_hash	=	0L;
		cc = find_rec (sach, &sach_rec, GTEQ, "r");
		while (!cc && sach_rec.hhsf_hash == exsf_rec.hhsf_hash)	
		{
			cumr_rec.hhcu_hash = sach_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sach, &sach_rec, NEXT, "r");
				continue;
			}
			cuin_rec.hhci_hash = sach_rec.hhci_hash;
			cc = find_rec (cuin, &cuin_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (sach, &sach_rec, NEXT, "r");
				continue;
			}
			if (envSaCommPayment)
			{
				sacl_rec.sach_hash =  sach_rec.sach_hash; 
				cc = find_rec (sacl, &sacl_rec, GTEQ, "r");
				while (!cc	&& sacl_rec.sach_hash ==  sach_rec.sach_hash)
				{
					if ((sacl_rec.rec_date > local_rec.endDate ||
						sacl_rec.rec_date < local_rec.startDate) ||
						sacl_rec.rec_amt == 0.00)
					{
						cc = find_rec (sacl, &sacl_rec, NEXT, "r");
						continue;
					}
	
					cuhd_rec.hhcp_hash = sacl_rec.hhcp_hash;
					cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "r");
					if (cc)
						file_err (cc, cuin, "DBFIND");

					/*
					 * Check the array size before adding new element.
					 */
					if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
						sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

					sprintf 
					(
						sortRec [sortCnt].sortCode, 
						"%2.2s%9.9s%8.8s", 
						exsf_rec.salesman_no, 
						cumr_rec.dbt_acronym, 	
						cuin_rec.inv_no
					);
					strcpy (sortRec [sortCnt].salesman, 	exsf_rec.salesman_no);
					strcpy (sortRec [sortCnt].acronym, cumr_rec.dbt_acronym);
					strcpy (sortRec [sortCnt].invNo, 	cuin_rec.inv_no);
					strcpy (sortRec [sortCnt].recNo, 	cuhd_rec.receipt_no);
					sortRec [sortCnt].invDate 	= cuin_rec.date_of_inv;
					sortRec [sortCnt].invAmt	= DOLLARS (sach_rec.inv_amt);
					sortRec [sortCnt].recAmt	= DOLLARS (sacl_rec.rec_amt);
					sortRec [sortCnt].recDate 	= sacl_rec.rec_date;
					sortRec [sortCnt].commRate	= sach_rec.com_rate;
					sortRec [sortCnt].commAmt 	= DOLLARS (sacl_rec.com_amt);
					sortRec [sortCnt].commVal	= DOLLARS (sach_rec.com_val);
					sortRec [sortCnt].hhsfHash	= exsf_rec.hhsf_hash;
					strcpy (sortRec [sortCnt].saleFlag, sach_rec.sale_flag);
					/*
					 * Increment array counter.
					 */
					sortCnt++;
					cc = find_rec (sacl, &sacl_rec, NEXT, "r");
				}
			}
			else
			{
				if (cuin_rec.date_of_inv > local_rec.endDate ||
					cuin_rec.date_of_inv < local_rec.startDate) 
				{
					cc = find_rec (sach, &sach_rec, NEXT, "r");
					continue;
				}
				/*
				 * Check the array size before adding new element.
				 */
				if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
					sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

				sprintf 
				(
					sortRec [sortCnt].sortCode, 
					"%2.2s%9.9s%8.8s", 
					exsf_rec.salesman_no, 
					cumr_rec.dbt_acronym, 	
					cuin_rec.inv_no
				);
				strcpy (sortRec [sortCnt].salesman, exsf_rec.salesman_no);
				strcpy (sortRec [sortCnt].acronym, cumr_rec.dbt_acronym);
				strcpy (sortRec [sortCnt].invNo, 	cuin_rec.inv_no);
				sortRec [sortCnt].invDate 		= cuin_rec.date_of_inv;
				sortRec [sortCnt].invAmt		= DOLLARS (sach_rec.inv_amt);
				strcpy (sortRec [sortCnt].recNo, "PAY SALE");
				sortRec [sortCnt].recAmt		= DOLLARS (sacl_rec.rec_amt);
				sortRec [sortCnt].recDate 		= sacl_rec.rec_date;
				sortRec [sortCnt].commRate		= sach_rec.com_rate;
				sortRec [sortCnt].commAmt 		= DOLLARS (sacl_rec.com_amt);
				sortRec [sortCnt].commVal		= DOLLARS (sach_rec.com_val);
				sortRec [sortCnt].hhsfHash		= exsf_rec.hhsf_hash;
				strcpy (sortRec [sortCnt].saleFlag, sach_rec.sale_flag);
				/*
				 * Increment array counter.
				 */
				sortCnt++;
			}
			cc = find_rec (sach, &sach_rec, NEXT, "r");
		}
		cc = find_rec (exsf2, &exsf_rec, NEXT, "r");		
	}
}

void
PrintData (void)
{
	int		i;

	char	wkDate1 [13], 
			wkDate2 [13];

	char	wkAmt [4][15];

	double	TotalSman [3];
	double	TotalGrand [3];

	int		FirstTime	=	TRUE;

	strcpy (prevSman, "");
	strcpy (prevDesc, "");

	for (i = 0; i < 3; i++)
	{
		TotalSman [i]	=	0.00;
		TotalGrand [i]	=	0.00;
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		exsf_rec.hhsf_hash	=	sortRec [i].hhsfHash;
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, exsf, "DBFIND");

		sprintf (currSman, "%-2.2s", sortRec [i].salesman);
		
		dsp_process ("Salesman", currSman);
					
		if (strcmp (prevSman, currSman))
		{
			if (!FirstTime)
			{
				strcpy (wkAmt [0], CF (TotalSman [0], "NNN,NNN,NNN.NN"));
				strcpy (wkAmt [1], CF (TotalSman [1], "NNN,NNN,NNN.NN"));
				strcpy (wkAmt [2], CF (TotalSman [2], "NNN,NNN,NNN.NN"));

				if (SUMMARY)
				{
					fprintf (fout, "| %2.2s ", prevSman);
					fprintf (fout, "| %40.40s ", prevDesc);
					fprintf (fout, "|%14.14s ", 	wkAmt [0]);
					fprintf (fout, "|%14.14s ", 	wkAmt [1]);
					fprintf (fout, "|%14.14s |\n", 	wkAmt [2]);
				}
				else
				{
					fprintf (fout, "| TOTAL FOR SALESMAN  ");
					fprintf (fout, "|            ");
					fprintf (fout, "|          ");
					fprintf (fout, "|%14.14s ", wkAmt [0]);
					fprintf (fout, "|        ");
					fprintf (fout, "|            ");
					fprintf (fout, "|%14.14s ", wkAmt [1]);
					fprintf (fout, "|           ");
					fprintf (fout, "|%14.14s |\n", wkAmt [2]);
				}
				TotalSman [0]	=	0.00;
				TotalSman [1]	=	0.00;
				TotalSman [2]	=	0.00;
			}

			if (!SUMMARY)
			{
				if (FirstTime)
				{
					fprintf (fout, "|%s - %s", currSman, exsf_rec.salesman);
					fprintf (fout, "                ");
					fprintf (fout, "         ");
					fprintf (fout, "             ");
					fprintf (fout, "                ");
					fprintf (fout, "            ");
					fprintf (fout, "                |\n");
				}
				fprintf (fout, ".PD|%s - %s", currSman, exsf_rec.salesman);
				fprintf (fout, "                ");
				fprintf (fout, "         ");
				fprintf (fout, "             ");
				fprintf (fout, "                ");
				fprintf (fout, "            ");
				fprintf (fout, "                |\n");

				if (!FirstTime)
					fprintf (fout, ".PA\n");

			}
			FirstTime = FALSE;
		} 
		strcpy (wkAmt [0], CF (sortRec [i].invAmt, 	"NNN,NNN,NNN.NN"));
		strcpy (wkAmt [1], CF (sortRec [i].recAmt, 	"NNN,NNN,NNN.NN"));
		strcpy (wkAmt [2], CF (sortRec [i].commRate,"NNN,NNN.NN"));
		strcpy (wkAmt [3], CF (sortRec [i].commAmt, "NNN,NNN,NNN.NN"));

		TotalSman [0]	+=	sortRec [i].invAmt;
		TotalSman [1]	+=	sortRec [i].recAmt;
		TotalSman [2]	+=	sortRec [i].commAmt;
		TotalGrand [0]	+=	sortRec [i].invAmt;
		TotalGrand [1]	+=	sortRec [i].recAmt;
		TotalGrand [2]	+=	sortRec [i].commAmt;

		strcpy (wkDate1, DateToString (sortRec [i].invDate));
		strcpy (wkDate2, DateToString (sortRec [i].recDate));

		if (!SUMMARY)
		{
			fprintf (fout, "|%10.10s", 
			   (sortRec [i].saleFlag [0] == 'M') ? "S.MANAGER " : "S.PERSON. ");
			fprintf (fout, "| %8.8s ", 		sortRec [i].invNo);
			fprintf (fout, "| %10.10s ", 	wkDate1);
			fprintf (fout, "| %9.9s", 		sortRec [i].acronym);
			fprintf (fout, "|%14.14s ", 	wkAmt [0]);
			fprintf (fout, "|%8.8s", 		sortRec [i].recNo);
			fprintf (fout, "| %10.10s ", 	wkDate2);
			fprintf (fout, "|%14.14s ", 	wkAmt [1]);
			fprintf (fout, "|%10.10s ", 	wkAmt [2]);
			fprintf (fout, "|%14.14s |\n", 	wkAmt [3]);
		}
					
		strcpy (prevSman, currSman);	
		strcpy (prevDesc, exsf_rec.salesman);	
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	strcpy (wkAmt [0], CF (TotalSman [0], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [1], CF (TotalSman [1], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [2], CF (TotalSman [2], "NNN,NNN,NNN.NN"));

	if (SUMMARY)
	{
		fprintf (fout, "| %2.2s ", 	prevSman);
		fprintf (fout, "| %40.40s ", prevDesc);
		fprintf (fout, "|%14.14s ", 	wkAmt [0]);
		fprintf (fout, "|%14.14s ", 	wkAmt [1]);
		fprintf (fout, "|%14.14s |\n", 	wkAmt [2]);
	}
	else
	{
		fprintf (fout, "| TOTAL FOR SALESMAN  ");
		fprintf (fout, "|            ");
		fprintf (fout, "|          ");
		fprintf (fout, "|%14.14s ", wkAmt [0]);
		fprintf (fout, "|        ");
		fprintf (fout, "|            ");
		fprintf (fout, "|%14.14s ", wkAmt [1]);
		fprintf (fout, "|           ");
		fprintf (fout, "|%14.14s |\n", wkAmt [2]);
	}
	strcpy (wkAmt [0], CF (TotalGrand [0], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [1], CF (TotalGrand [1], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [2], CF (TotalGrand [2], "NNN,NNN,NNN.NN"));

	if (SUMMARY)
	{
		fprintf (fout, "|----");
		fprintf (fout, "|------------------------------------------");
		fprintf (fout, "|---------------");
		fprintf (fout, "|---------------");
		fprintf (fout, "|---------------|\n");

		fprintf (fout, "| *** GRAND TOTAL ***                           ");
		fprintf (fout, "|%14.14s ", 	wkAmt [0]);
		fprintf (fout, "|%14.14s ", 	wkAmt [1]);
		fprintf (fout, "|%14.14s |\n", 	wkAmt [2]);
	}
	else
	{
		fprintf (fout, "|----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|------------");
		fprintf (fout, "|----------");
		fprintf (fout, "|---------------");
		fprintf (fout, "|--------");
		fprintf (fout, "|------------");
		fprintf (fout, "|---------------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|---------------|\n");

		fprintf (fout, "| *** GRAND TOTAL *** ");
		fprintf (fout, "|            ");
		fprintf (fout, "|          ");
		fprintf (fout, "|%14.14s ", wkAmt [0]);
		fprintf (fout, "|        ");
		fprintf (fout, "|            ");
		fprintf (fout, "|%14.14s ", wkAmt [1]);
		fprintf (fout, "|           ");
		fprintf (fout, "|%14.14s |\n", wkAmt [2]);
	}
}

int
DisplayData (void)
{
	int		i;

	int		LineCounter	=	0;
	char	wkDate1 [11], 
			wkDate2 [11];

	char	wkAmt [4][15];
	char	disp_str [200];

	double	TotalSman [3];
	double	TotalGrand [3];

	int		FirstTime	=	TRUE;

	if (SUMMARY)
	{
		Dsp_open (12, 3, P_SIZE); 
		Dsp_saverec (" SM |      SALES PERSON DESCRIPTION.           |     INVOICE   |     AMOUNT    |   COMMISSION  ");
		Dsp_saverec (" NO |                                          |      TOTAL    |   COLLECTED   |     AMOUNT    ");
	}
	else
	{
		Dsp_open (0, 3, P_SIZE); 
		Dsp_saverec ("COMMISSION| INVOICE  |  INVOICE   | CUSTOMER |     INVOICE   |RECEIPT |  RECEIPT   |     AMOUNT    |COMMISSION |   COMMISSION  ");
		Dsp_saverec ("  LEVEL   |  NUMBER  |    DATE    |  ACRONYM |     AMOUNT    | NUMBER |    DATE    |    COLLECTED  |    RATE   |     AMOUNT    ");

	}
	Dsp_saverec (" [NEXT SCN][PREV SCN][EDIT/END] ");
	
	strcpy (prevSman, "");
	strcpy (prevDesc, "");

	for (i = 0; i < 3; i++)
	{
		TotalSman [i]	=	0.00;
		TotalGrand [i]	=	0.00;
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		exsf_rec.hhsf_hash	=	sortRec [i].hhsfHash;
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			continue;

		sprintf (currSman, "%-2.2s", sortRec [i].salesman);
		
		if (strcmp (prevSman, currSman))
		{
			if (!FirstTime)
			{
				strcpy (wkAmt [0], CF (TotalSman [0], "NNN,NNN,NNN.NN"));
				strcpy (wkAmt [1], CF (TotalSman [1], "NNN,NNN,NNN.NN"));
				strcpy (wkAmt [2], CF (TotalSman [2], "NNN,NNN,NNN.NN"));

				if (SUMMARY)
				{
					sprintf (disp_str, " %2.2s ^E %40.40s ^E%14.14s ^E%14.14s ^E%14.14s ", 
							prevSman, prevDesc, wkAmt [0], wkAmt [1], wkAmt [2]);
					Dsp_saverec (disp_str);
				}
				else
				{
					sprintf (disp_str, "^1TOTAL FOR SALESMAN^6   ^E            ^E          ^E%14.14s ^E        ^E            ^E%14.14s ^E           ^E%14.14s ", 
							wkAmt [0], wkAmt [1], wkAmt [2]);
	
					Dsp_saverec (disp_str);
					Dsp_saverec ("^^GGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGGGGG");
				}
				TotalSman [0]	=	0.00;
				TotalSman [1]	=	0.00;
				TotalSman [2]	=	0.00;

				if (!SUMMARY)
				{
					for (i = LineCounter; i < P_SIZE - 4; i++)
						Dsp_saverec (" ");
				}
				LineCounter = 0;
			}
			FirstTime = FALSE;

			if (!SUMMARY)
			{
				sprintf (disp_str, "^1%s - (%40.40s)^6                                                                              ", 
								currSman, exsf_rec.salesman);
				Dsp_saverec (disp_str);
			}
		} 
		else
		{
			if (!SUMMARY)
			{
				if (LineCounter++ >= P_SIZE - 2)
				{
					sprintf (disp_str, "^1%s - (%40.40s)^6                                                                              ", 
									currSman, exsf_rec.salesman);
				
					Dsp_saverec (disp_str);
					LineCounter = 0;
				}
			}
		}

		strcpy (wkAmt [0], CF (sortRec [i].invAmt, 	"NNN,NNN,NNN.NN"));
		strcpy (wkAmt [1], CF (sortRec [i].recAmt, 	"NNN,NNN,NNN.NN"));
		strcpy (wkAmt [2], CF (sortRec [i].commRate,"NNN,NNN.NN"));
		strcpy (wkAmt [3], CF (sortRec [i].commAmt, "NNN,NNN,NNN.NN"));

		TotalSman [0]	+=	sortRec [i].invAmt;
		TotalSman [1]	+=	sortRec [i].recAmt;
		TotalSman [2]	+=	sortRec [i].commAmt;

		TotalGrand [0]	+=	sortRec [i].invAmt;
		TotalGrand [1]	+=	sortRec [i].recAmt;
		TotalGrand [2]	+=	sortRec [i].commAmt;


		if (!SUMMARY)
		{
			strcpy (wkDate1, DateToString (sortRec [i].invDate));
			strcpy (wkDate2, DateToString (sortRec [i].recDate));

			sprintf 
			(
				disp_str, 
				"%-10.10s^E %-8.8s ^E %10.10s ^E %9.9s^E%14.14s ^E%8.8s^E %10.10s ^E%14.14s ^E%10.10s ^E%14.14s ", 
				(sortRec [i].saleFlag [0] == 'M') ? "S.MANAGER " : "S.PERSON. ", 
				sortRec [i].invNo, 				/*	Invoice Number.		*/
				wkDate1, 						/*  Date of Invoice. 	*/
				sortRec [i].acronym, 			/*  Customer Acronym. 	*/
				wkAmt [0], 						/*  Invoice Amount. 	*/
				sortRec [i].recNo, 			/*  Receipt No			*/
				wkDate2, 						/*  Receipt Date		*/
				wkAmt [1], 						/*	Commission Value 	*/
				wkAmt [2], 						/*	Commission Rate 	*/
				wkAmt [3]						/*	Commission Value. 	*/
			);

			Dsp_saverec (disp_str);
		}
					
		strcpy (prevSman, currSman);	
		strcpy (prevDesc, exsf_rec.salesman);	
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	strcpy (wkAmt [0], CF (TotalSman [0], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [1], CF (TotalSman [1], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [2], CF (TotalSman [2], "NNN,NNN,NNN.NN"));

	if (SUMMARY)
	{
		sprintf (disp_str, " %2.2s ^E %40.40s ^E%14.14s ^E%14.14s ^E%14.14s ", 
					prevSman, prevDesc, wkAmt [0], wkAmt [1], wkAmt [2]);
		Dsp_saverec (disp_str);
		Dsp_saverec ("^^GGGGEGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGG");
	}
	else
	{
		sprintf (disp_str, "^1TOTAL FOR SALESMAN^6   ^E            ^E          ^E%14.14s ^E        ^E            ^E%14.14s ^E           ^E%14.14s ", 
							wkAmt [0], wkAmt [1], wkAmt [2]);

		Dsp_saverec (disp_str);
		Dsp_saverec ("^^GGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGGGEGGGGGGGGGGGGGGGEGGGGGGGGGGGEGGGGGGGGGGGGGGG");
	}
	strcpy (wkAmt [0], CF (TotalGrand [0], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [1], CF (TotalGrand [1], "NNN,NNN,NNN.NN"));
	strcpy (wkAmt [2], CF (TotalGrand [2], "NNN,NNN,NNN.NN"));

	if (SUMMARY)
	{
		sprintf (disp_str, "^1*** GRAND TOTAL ***^6                            ^E%14.14s ^E%14.14s ^E%14.14s ", 
							wkAmt [0], wkAmt [1], wkAmt [2]);
		Dsp_saverec (disp_str);

		Dsp_saverec ("^^GGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGG");
	}
	else
	{
		sprintf (disp_str, "^1*** GRAND TOTAL***^6   ^E            ^E          ^E%14.14s ^E        ^E            ^E%14.14s ^E           ^E%14.14s ", 
							wkAmt [0], wkAmt [1], wkAmt [2]);

		Dsp_saverec (disp_str);
		Dsp_saverec ("^^GGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGGGJGGGGGGGGGGGGGGGJGGGGGGGGGGGJGGGGGGGGGGGGGGG");
	}
			
	Dsp_srch ();
	Dsp_close ();
	return (EXIT_SUCCESS);
}

void
SrchExsf (
 char*  key_val)
{
    _work_open (2, 0, 40);
	save_rec ("#No", "#Salesman Name");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", key_val);
	cc = find_rec (exsf2, &exsf_rec, GTEQ, "r");
    while (!cc && !strcmp (exsf_rec.co_no, comm_rec.co_no) 
			&& !strncmp (exsf_rec.salesman_no, key_val, strlen (key_val)))
   	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf2, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf2, &exsf_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in exsf During (DBFIND)", cc, PNAME);
	return;
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
