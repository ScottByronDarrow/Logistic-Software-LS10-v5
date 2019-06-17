/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: db_colrpt.c,v 5.5 2002/08/08 06:12:21 scott Exp $
|  Program Name  : (db_colprt.c)
|  Program Desc  : (Customer collection report)
|=====================================================================|
| $Log: db_colrpt.c,v $
| Revision 5.5  2002/08/08 06:12:21  scott
| Updated as cheque should not include discounts.
|
| Revision 5.4  2002/07/17 09:57:06  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/03/12 04:30:15  scott
| Updated to remove disk based sorting.
|
| Revision 5.2  2001/08/09 08:23:19  scott
| Added FinishProgram ();
|
| Revision 5.1  2001/08/06 23:21:47  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:04:08  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:24:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:13:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:48:54  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 08:52:14  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:28:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/10/29 03:15:23  scott
| Updated for warning due to usage of -Wall flag on compiler.
|
| Revision 1.10  1999/10/26 03:38:09  scott
| Updated for missing language translations
|
| Revision 1.9  1999/10/05 07:33:52  scott
| Updated from ansi project.
|
| Revision 1.8  1999/06/14 23:25:44  scott
| Updated to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_colrpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_colrpt/db_colrpt.c,v 5.5 2002/08/08 06:12:21 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include 	<arralloc.h>
#include 	<ml_std_mess.h>

#define		BYSALESMAN	 (local_rec.SortBy [0] == 'S')
#define		DETAILED	 (local_rec.DetailedSummary [0] == 'D')

FILE	*pp;
	
#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct pocrRecord	pocr_rec;
struct exsfRecord	exsf_rec;

	char	*data = "data",
			*cumr2 = "cumr2";

	int		FirstTime		 = TRUE,
			envVarDbNettUsed = TRUE,
			envVarDbMcurr 	 = FALSE;
	char	branchNumber [3];

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [21];
	char	custNo		[sizeof cumr_rec.dbt_no]; 
	char	custAcronym	[sizeof cumr_rec.dbt_acronym]; 
	char	smanCode	[sizeof cumr_rec.sman_code];
	long	hhcpHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	long	StartDate;
	long	EndDate;
	long	lsystemDate;
	int		printerNo;
	char	StartSalesman [3],
			DStartSalesman [41],
			EndSalesman [3],
			DEndSalesman [41];
	char	DetailedSummary [2];
	char	SortBy [2];
} local_rec;

extern	int		TruePosition;

static	struct	var	vars []	= {	
	{1, LIN, "StartDate",	 3, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Start Date                ", "Enter Date for Starting Transaction - Default is start ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.StartDate},
	{1, LIN, "EndDate",	 4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End   Date                ", "Enter Date for Ending Transaction - Default is Today ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.EndDate},
	{1, LIN, "DetailSummary", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Detailed / Summary (D/S)  ", "Enter D(etailed) or S(ummary) default = 'S'> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.DetailedSummary}, 
	{1, LIN, "SortBy", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Sort By S/C               ", "Enter S(alesman) or C(ustomer) default = 'S'> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.SortBy}, 
	{1, LIN, "StartSalesman", 9, 2, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Start Salesman code       ", "Enter start salesman code. <default = spaces> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.StartSalesman}, 
	{1, LIN, "sman_desc", 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "  ","Salesman Description      ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DStartSalesman}, 
	{1, LIN, "EndSalesman", 11, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "~~", "End Salesman code         ", "Enter end salesman code. <default = end of range> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.EndSalesman}, 
	{1, LIN, "DEndSalesman", 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Salesman Description      ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.DEndSalesman}, 
	{1, LIN, "printerNo",	 14, 2, INTTYPE,
		"NN", "          ",
		" ", "1","Printer Number            ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};
	double	TotalSalesman [3],
			TotalReport [3];

/*
 * Local Function Prototypes.
 */
void	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
int 	spec_valid 		(int);
void 	SrchExsf 		(char *);
void 	ProcessCustomer (void);
void 	SaveChqLine 	(void);
void 	ProcessReport 	(void);
void 	PrintSalesman 	(char *);
void 	PrintLine 		(long);
void 	PrintTotal 		(int);
void 	PrintGrand  	(void);
int 	heading 		(int);

int
main (
	int		argc,
	char	*argv [])
{
	int		envVarDbCo;
	char	*sptr;

	TruePosition	=	TRUE;
	local_rec.lsystemDate = TodaysDate ();

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

 	envVarDbCo = atoi (get_env ("DB_CO"));

	SETUP_SCR (vars);

	init_scr	();			/*  sets terminal from termcap	*/
	set_tty		();         /*  get into raw mode			*/
	set_masks	();			/*  setup print using masks		*/
	init_vars	(1);		/*  set default values			*/

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;
		init_vars (1);	

		/*-----------------------------
		| Edit screen 1 linear input. |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit) 
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart) 
			continue;

		if (!restart)
		{ 
			dsp_screen ("Printing Customer Transaction Listing.",
					comm_rec.co_no,comm_rec.co_name);

			/*
			 * Allocate the initial array.
			 */
			ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
			sortCnt = 0;

			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNumber);
			strcpy (cumr_rec.dbt_no, "      ");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
						!strcmp (cumr_rec.est_no, branchNumber))
			{
				/*-------------------------------------
				| Check salesman start and end range. |
				-------------------------------------*/
				if (strcmp (cumr_rec.sman_code, local_rec.StartSalesman) < 0 ||
					strcmp (cumr_rec.sman_code, local_rec.EndSalesman) > 0)
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}
				dsp_process ("Customer : ",cumr_rec.dbt_no);
				ProcessCustomer ();
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			}
			ProcessReport ();
		}
	}
	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	open_rec (comm, comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
    abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (cuin);
    abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (comm);
	abc_fclose (pocr);
	abc_fclose (exsf);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (int field)
{
	if (LCHECK ("SortBy")) 
	{
		if (BYSALESMAN)
		{
			FLD ("StartSalesman")	=	YES;
			FLD ("EndSalesman")		=	YES;
		}
		else
		{
			FLD ("StartSalesman")	=	NA;
			FLD ("EndSalesman")		=	NA;
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("printerNo")) 
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess ("\007 Invalid Printer. ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate start group |
	----------------------*/
	if (LCHECK ("StartSalesman"))
	{
		if (FLD ("StartSalesman") == NA)
		{
			sprintf (local_rec.StartSalesman,"%-2.2s","  ");
			sprintf (exsf_rec.salesman,"%-40.40s","BEGINNING OF RANGE");
			sprintf (local_rec.DStartSalesman,"%-40.40s",exsf_rec.salesman);
			display_field (field + 1);
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no, local_rec.StartSalesman);

		if (!dflt_used)
		{
			cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");
			if (cc) 
			{
				sprintf (err_str, "Salesman %s in not on file.",local_rec.StartSalesman);
				errmess (err_str);
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.StartSalesman,"%-2.2s","  ");
			sprintf (exsf_rec.salesman,"%-40.40s","BEGINNING OF RANGE");
		}
		if (prog_status != ENTRY && strcmp (local_rec.StartSalesman,local_rec.EndSalesman) > 0)
		{
			sprintf (err_str, "NOTE Range<%s is now GREATER than %s>",local_rec.StartSalesman,local_rec.EndSalesman);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_SUCCESS); 
		}
		sprintf (local_rec.DStartSalesman,"%-40.40s",exsf_rec.salesman);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate end group |
	--------------------*/
	if (LCHECK ("EndSalesman"))
	{
		if (FLD ("EndSalesman") == NA)
		{
			sprintf (local_rec.EndSalesman,"%-2.2s","~~");
			sprintf (exsf_rec.salesman,"%-40.40s","END OF RANGE");
			strcpy (local_rec.DEndSalesman,exsf_rec.salesman);
			display_field (field + 1);
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exsf_rec.co_no,comm_rec.co_no);
		strcpy (exsf_rec.salesman_no,local_rec.EndSalesman);
		
		if (!dflt_used)
		{
			cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");
			if (cc) 
			{
				sprintf (err_str, "Salesman %s in not on file.",local_rec.EndSalesman);
				errmess (err_str);
				return (EXIT_FAILURE); 
			}
		}
		else
		{
			sprintf (local_rec.EndSalesman,"%-2.2s","~~");
			sprintf (exsf_rec.salesman,"%-40.40s","END OF RANGE");
		}
		if (strcmp (local_rec.StartSalesman,local_rec.EndSalesman) > 0)
		{
			sprintf (err_str, "Invalid Range < %s is GREATER than %s>",local_rec.StartSalesman,local_rec.EndSalesman);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.DEndSalesman,exsf_rec.salesman);
		display_field (field + 1);
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Start Date. |
	----------------------*/
	if (LCHECK ("StartDate"))
	{
		if (dflt_used)
		{
			local_rec.StartDate = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			local_rec.StartDate > local_rec.EndDate)
		{
			print_mess ("Start Date Must Be Less Than End Date ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate End Date. |
	--------------------*/
	if (LCHECK ("EndDate"))
	{
		if (dflt_used)
		{
			local_rec.EndDate = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (local_rec.StartDate > local_rec.EndDate)
		{
			print_mess ("End Date Must Be Greater Than Start Date ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
HeadingOutput (void)
{
	char	wk_date [2] [11];

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp,".LP%d\n",local_rec.printerNo);
	fprintf (pp,".PI12\n");
	fprintf (pp,".%d\n", (BYSALESMAN) ? 13 : 12);
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".ECOLLECTION REPORT BY SALESMAN\n");

	sprintf (wk_date [0], DateToString (local_rec.StartDate));
	sprintf (wk_date [1], DateToString (local_rec.EndDate));

	fprintf (pp,".CAll Collections from %s to %s.\n", wk_date [0],wk_date [1]);
	fprintf (pp,".CSorted By %s\n", (BYSALESMAN) ? "Salesman" : "Customer");

	if (BYSALESMAN)
	{
		fprintf (pp,".C From Salesman : %s %s  TO  Salesman %s %s\n",
						local_rec.StartSalesman,
						clip (local_rec.DStartSalesman),
						local_rec.EndSalesman,
						clip (local_rec.DEndSalesman));
	}
	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp, ".R==================================");
	if (envVarDbMcurr)
		fprintf (pp, "==============");
	fprintf (pp, "===============================================");
	fprintf (pp, "=====================================");
	if (DETAILED)
		fprintf (pp, "==========================\n");
	else
		fprintf (pp, "\n");

	fprintf (pp, "==================================");
		fprintf (pp, "==============");
	fprintf (pp, "===============================================");
	fprintf (pp, "=====================================");
	if (DETAILED)
		fprintf (pp, "==========================\n");
	else
		fprintf (pp, "\n");

	fprintf (pp, "| DATE  OF | RECEIPT| AMOUNT  OF  ");
	if (envVarDbMcurr)
		fprintf (pp, "| AMOUNT  OF  ");
	fprintf (pp, "|BANK|        BRANCH      |  ALTERNATE DRAWER  ");
	fprintf (pp, "|  CHEQUE  |      CUSTOMER NAME      ");
	if (DETAILED)
		fprintf (pp, "|DOCUMENT|   DOCUMENT    |\n");
	else
		fprintf (pp, "|\n");

	fprintf (pp, "|  PAYMENT | NUMBER |   RECEIPT   ");
	if (envVarDbMcurr)
		fprintf (pp, "| LOCAL VAULE ");
	fprintf (pp, "|CODE|         CODE       |      DETAILS       ");
	fprintf (pp, "| DUE DATE |                         ");
	if (DETAILED)
		fprintf (pp, "| NUMBER |    AMOUNT     |\n");
	else
		fprintf (pp, "|\n");

	fprintf (pp, "|----------|--------|-------------");
	if (envVarDbMcurr)
		fprintf (pp, "|-------------");
	fprintf (pp, "|----|--------------------|--------------------");
	fprintf (pp, "|----------|-------------------------");
	if (DETAILED)
		fprintf (pp, "|--------|---------------|\n");
	else
		fprintf (pp, "|\n");

}


/*======================
| Search for salesman. |
======================*/
void
SrchExsf (
 char*              key_val)
{
	work_open ();
	save_rec ("#Sm","#Salesman.");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec (exsf,&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
		      !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf,&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec (exsf,&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

void
ProcessCustomer (void)
{
	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, "        ");
	cuhd_rec.index_date	=	0L;

	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuhd_rec.hhcu_hash) 
	{
		/*---------------------------------
		| Check date start and end range. |
		---------------------------------*/
		if (cuhd_rec.date_payment < local_rec.StartDate ||
		    cuhd_rec.date_payment > local_rec.EndDate)
		{
			cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
			continue;
		}
		SaveChqLine ();
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}
}

void
SaveChqLine (void)
{
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	/*
	 * Load values into array element sortCnt.
	 */
	sprintf 
	(
		sortRec [sortCnt].sortCode, 
		"%s%010ld", 
		(BYSALESMAN) ? cumr_rec.sman_code : cumr_rec.dbt_acronym,
		cuhd_rec.date_payment
	);
	strcpy (sortRec [sortCnt].custNo,		cumr_rec.dbt_no);
	strcpy (sortRec [sortCnt].custAcronym,	cumr_rec.dbt_acronym);
	strcpy (sortRec [sortCnt].smanCode,		cumr_rec.sman_code);
	sortRec [sortCnt].hhcpHash 	= cuhd_rec.hhcp_hash;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

void
ProcessReport (void)
{
	int		i;
	long	hhcpHash	=	0L;
	char	PrevSalesman [3],
			CurrSalesman [3],
			PrevCustomer [7],
			CurrCustomer [7];

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	if (sortCnt)
	{
		strcpy (PrevSalesman, "  ");
		strcpy (PrevCustomer, "      ");
	}
	HeadingOutput ();

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (CurrCustomer, 	"%6.6s", sortRec [i].custNo);
		sprintf (CurrSalesman, 	"%2.2s", sortRec [i].smanCode);
		hhcpHash	=	sortRec [i].hhcpHash;

		if (BYSALESMAN)
		{
			if (strcmp (PrevSalesman, CurrSalesman))
			{
				if (!FirstTime)
					PrintTotal (TRUE);
				
				PrintSalesman (CurrSalesman);
				FirstTime = FALSE;
				strcpy (PrevSalesman, CurrSalesman);
			}
			PrintLine (hhcpHash);
		}
		else
		{
			if (strcmp (PrevCustomer, CurrCustomer))
			{
				if (!FirstTime)
					PrintTotal (TRUE);

				FirstTime = FALSE;
				strcpy (PrevCustomer, CurrCustomer);
			}
			PrintLine (hhcpHash);
		}
	}

	PrintTotal (FALSE);
	PrintGrand ();
	fprintf (pp, ".EOF\n");
	pclose (pp);

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
}

void
PrintSalesman (
	char	*CurrSalesman)
{
	char	PrintStr [200];

	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", CurrSalesman);
	cc = find_rec ("exsf", &exsf_rec, COMPARISON, "r");
	if (cc)
		sprintf (exsf_rec.salesman, "%40.40s", "Unknown");

	sprintf (PrintStr, "| %s - %40.40s  ", exsf_rec.salesman_no, exsf_rec.salesman);
	if (envVarDbMcurr)
		strcat (PrintStr, "              ");
	strcat (PrintStr, "                                ");
	strcat (PrintStr, "                                     ");
	if (DETAILED)
		strcat (PrintStr, "                         |\n");
	else
		strcat (PrintStr, "|\n");

	fprintf (pp, ".PD%s", PrintStr);
	if (!FirstTime)
		fprintf (pp, ".PA\n");
}

void
PrintLine (
	long	hhcpHash)
{
	double	balance = 0.00;
	double	lcl_bal = 0.00;
	int		FirstDetail	=	TRUE;


	abc_selfield (cuhd, "cuhd_hhcp_hash");

	cuhd_rec.hhcp_hash	=	hhcpHash;
	cc = find_rec (cuhd, &cuhd_rec, COMPARISON, "r");
	if (cc)
		return;

	cumr2_rec.hhcu_hash	=	cuhd_rec.hhcu_hash;
	cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
	if (cc)
		return;

	strcpy (pocr_rec.co_no, cumr2_rec.co_no);
	strcpy (pocr_rec.code, cumr2_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = cuhd_rec.tot_amt_paid;

	if (balance != 0.00)
		lcl_bal = balance / pocr_rec.ex1_factor;
	else
		lcl_bal = 0.00;

	fprintf (pp, "|%10.10s", DateToString (cuhd_rec.date_payment));
	fprintf (pp, "|%8.8s", cuhd_rec.receipt_no);
	fprintf (pp, "|%12.2f ",	DOLLARS (balance));
	if (envVarDbMcurr)
		fprintf (pp, "|%12.2f ",	DOLLARS (lcl_bal));

	TotalSalesman [0]	+=	DOLLARS (balance);
	TotalSalesman [1]	+=	DOLLARS (lcl_bal);
	TotalReport [0]		+=	DOLLARS (balance);
	TotalReport [1]		+=	DOLLARS (lcl_bal);
	fprintf (pp, "|%s ", cuhd_rec.db_bank);
	fprintf (pp, "|%s", cuhd_rec.db_branch);
	fprintf (pp, "|%s", cuhd_rec.alt_drawer);

	if (cuhd_rec.due_date == 0L)
		cuhd_rec.due_date = cuhd_rec.date_payment;

	fprintf (pp, "|%-10.10s", DateToString (cuhd_rec.due_date));
	
	fprintf (pp, "|%-25.25s", cumr2_rec.dbt_name);

	if (!DETAILED)
		fprintf (pp, "|\n");

	else
	{
		FirstDetail	=	TRUE;
		cudt_rec.hhcp_hash	=	cuhd_rec.hhcp_hash;
		cc = find_rec ("cudt", &cudt_rec, GTEQ , "r");	
		while (!cc && cudt_rec.hhcp_hash == cuhd_rec.hhcp_hash)
		{
			cuin_rec.hhci_hash	=	cudt_rec.hhci_hash;
			cc = find_rec ("cuin", &cuin_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec ("cudt", &cudt_rec, NEXT, "r");	
				continue;
			}
			if (!FirstDetail)
			{
				fprintf (pp, "|          |        |             ");
				if (envVarDbMcurr)
					fprintf (pp, "|             ");
				fprintf (pp, "|    |                    |                    ");
				fprintf (pp, "|          |                         ");
			}
			FirstDetail = FALSE;

			fprintf (pp, "|%s|%14.2f |\n", cuin_rec.inv_no,
												DOLLARS (cudt_rec.amt_paid_inv));

			TotalSalesman [2]	+=	DOLLARS (cudt_rec.amt_paid_inv);
			TotalReport [2]		+=	DOLLARS (cudt_rec.amt_paid_inv);
			cc = find_rec ("cudt", &cudt_rec, NEXT , "r");	
		}
	}
}

void
PrintTotal (
 int                LineOff)
{
	fprintf (pp, "| ** SUB TOTAL **   |%12.2f ", TotalSalesman [0]);
	if (envVarDbMcurr)
		fprintf (pp, "|%12.2f ", TotalSalesman [1]);
	fprintf (pp, "|    |                    |                    ");
	fprintf (pp, "|          |                         ");
	if (DETAILED)
		fprintf (pp, "|        |%14.2f |\n", TotalSalesman [2]);
	else
		fprintf (pp, "|\n");

	if (LineOff)
	{
		fprintf (pp, "|----------|--------|-------------");
		if (envVarDbMcurr)
			fprintf (pp, "|-------------");
		fprintf (pp, "|----|--------------------|--------------------");
		fprintf (pp, "|----------|-------------------------");
		if (DETAILED)
			fprintf (pp, "|--------|---------------|\n");
		else
			fprintf (pp, "|\n");
	}

	TotalSalesman [0]	=	0.00;
	TotalSalesman [1]	=	0.00;
	TotalSalesman [2]	=	0.00;
}

void
PrintGrand (void)
{
	fprintf (pp, "|==========|========|=============");
	if (envVarDbMcurr)
		fprintf (pp, "|=============");
	fprintf (pp, "|====|====================|====================");
	fprintf (pp, "|==========|=========================");
	if (DETAILED)
		fprintf (pp, "|========|===============|\n");
	else
		fprintf (pp, "|\n");
	fprintf (pp, "| ** GRAND TOTAL ** |%12.2f ", TotalReport [0]);
	if (envVarDbMcurr)
		fprintf (pp, "|%12.2f ", TotalReport [1]);
	fprintf (pp, "|    |                    |                    ");
	fprintf (pp, "|          |                         ");
	if (DETAILED)
		fprintf (pp, "|        |%14.2f |\n", TotalReport [2]);
	else
		fprintf (pp, "|\n");

	TotalReport [0]	=	0.00;
	TotalReport [1]	=	0.00;
	TotalReport [2]	=	0.00;
}

/*
 * Heading concerns itself with clearing the screen, painting the  
 * screen overlay in preparation for input                        
 */
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (" Customer Transaction Processing Selection. "),20,0,1);

		line_at (1, 0, 80);

		move (1,input_row);
		if (scn == 1)
		{
			box (0,2,80,12);
			line_at (5, 1, 79);
			line_at (8, 1, 79);
			line_at (13, 1, 79);
		}

		sprintf (err_str, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name);
		print_at (20, 0, err_str);

		sprintf (err_str, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
		print_at (21, 0, err_str);
		line_at (22, 0, 80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
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
