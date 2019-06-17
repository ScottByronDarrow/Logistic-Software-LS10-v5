/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lists.c,v 5.5 2002/07/23 10:01:28 scott Exp $
|  Program Name  : (db_lists.c)
|  Program Desc  : (Customer listings)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/01/87         |
|---------------------------------------------------------------------|
| $Log: db_lists.c,v $
| Revision 5.5  2002/07/23 10:01:28  scott
| Updated to use new sort functions.
|
| Revision 5.4  2001/12/04 05:08:04  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/04 00:47:44  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lists.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lists/db_lists.c,v 5.5 2002/07/23 10:01:28 scott Exp $";

#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<arralloc.h>

#define	SMAN	1

#define		SORT_DBT			1
#define		SORT_ACR			2
#define		SORT_SMAN_DBT		3
#define		SORT_SMAN_ACR		4
#define		SORT_CTYPE_SMAN		5
#define		SORT_SMAN_CTYPE		6

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct exsfRecord	exsf_rec;
struct exclRecord	excl_rec;

	char	*data = "data";

	int		validOK 	= 0,
			reportType 	= 1,
			runType 	= 1,
			sortType 	= SORT_ACR,
			printerNo 	= 1,
			dspaced 	= 0,
			envDbMcurr 	= FALSE;

	FILE	*pp;

	char	branchNo [3];
	
	static char *runDesc [] = {
		"CUSTOMER LISTING BY COMPANY",
		"CUSTOMER LISTING BY BRANCH"
	};

	static char *reportDesc [] = {
 		"Analysis (Full Name and Address Listing) ",
 		"Analysis (Short Name and Address Listing) ",
 		"Analysis (Full Name , Address and Delivery Listing) ",
 		"Analysis (Full Master file listing) ",
 		"Analysis (Stop Credit Listing) ",
 		"Analysis (Customer Payment Term Listing) "
	};

	static char *sortDesc [] = {
		"Sorted By Customer Number.",
		"Sorted By Customer Acronym.",
		"Sorted By Salesman / Number.",
		"Sorted By Salesman / Acronym.",
		"Sorted By Customer Type / Salesman.",
		"Sorted By Salesman / Customer Type."
	};

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [27];
	char	branchNo	[sizeof cumr_rec.est_no]; 
	char	classType	[sizeof cumr_rec.class_type];
	char	smanCode	[sizeof cumr_rec.sman_code];
	char	acronym		[sizeof cumr_rec.dbt_acronym];
	char	customerNo	[sizeof cumr_rec.dbt_no];
	long	hhcuHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	shutdown_prog 		(void);
void 	StartReport 		(int);
void 	StoreData 			(void);
void 	ProcessSorted		(void);
void 	PrintSalesman 		(int);
void 	PrintCustomerType 	(int);
void 	ProcessFile 		(void);
void 	EndReport 			(void);
void 	HeadingOne 			(void);
void 	HeadingTwo 			(void);
void 	HeadingThree 		(void);
void 	HeadingSix 			(void);
int		SortFunc			(const	void *,	const void *);

int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	if (argc != 5)
	{
		print_at (0,0,mlDbMess708,argv [0]);
        return (EXIT_FAILURE);
	}

	printerNo 	= atoi (argv [1]);
	reportType 	= atoi (argv [2]);
	runType 	= atoi (argv [3]);
	sortType 	= atoi (argv [4]);

 	dspaced = atoi (get_env ("DB_DSPACED"));

	OpenDB ();

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0 ? FALSE : atoi (sptr));

	dsp_screen ("Printing Customers Listings.",comm_rec.co_no,comm_rec.co_name);

	StartReport (printerNo);

	if (sortType == SORT_DBT || sortType == SORT_SMAN_DBT)
	{
		abc_selfield (cumr, "cumr_id_no");
		strcpy (cumr_rec.dbt_acronym, "         ");
		strcpy (cumr_rec.dbt_no, "      ");
	}
	else
	{
     	abc_selfield (cumr, "cumr_id_no2");
		strcpy (cumr_rec.dbt_no, "      ");
		strcpy (cumr_rec.dbt_acronym, "         ");
	}

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, "  ");

	if (runType == 2)
		strcpy (cumr_rec.est_no, comm_rec.est_no);

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no)) 
	{
		validOK = 1;
	
		switch (runType)	
		{
 		    case 2:
			if (strcmp (cumr_rec.est_no, comm_rec.est_no))
				validOK = 0;
			break;
			
		}
		if (validOK == 0)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
	
		if (sortType > SORT_ACR)
			StoreData ();
		else
			ProcessFile ();
	
		dsp_process ("Customer : ",cumr_rec.dbt_no);
	
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	if (sortType > SORT_ACR)
		ProcessSorted ();

	EndReport ();
	pclose (pp);
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (exsf);
	abc_fclose (excl);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
StartReport (
	int		printerNo)
{
	if (sortType > SORT_ACR)
	{
		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
		sortCnt = 0;
	}
	/*
	 * Open pipe to pformat 
 	 */
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*
	 * Start output to standard print.
	 */
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date),PNAME);
	fprintf (pp,".LP%d\n",printerNo);
	fprintf (pp,".15\n");
	fprintf (pp,".PI12\n");

	switch (reportType)	
	{
	    	case 1:
	    	case 3:
	    	case 5:
			HeadingOne ();
	        	break;

	    	case 2:
			HeadingTwo ();
	        	break;

	    	case 4:
			HeadingThree ();
	        	break;

			case 6:
			HeadingSix ();
				break;

	    	default:
			HeadingOne ();
	        	break;
	}
		fprintf (pp,".PI12\n");
}

void
StoreData (void)
{
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	switch (sortType)
	{
	case	SORT_SMAN_DBT:
		sprintf (sortRec [sortCnt].sortCode, "%s%s%s", 
					cumr_rec.est_no,		
					cumr_rec.sman_code,	
					cumr_rec.dbt_no);
		break;

	case	SORT_SMAN_ACR:
		sprintf (sortRec [sortCnt].sortCode, "%s%s%s", 
					cumr_rec.est_no,
					cumr_rec.sman_code,
					cumr_rec.dbt_acronym);
		break;

	case	SORT_CTYPE_SMAN:
		sprintf (sortRec [sortCnt].sortCode, "%s%s%s", 
					cumr_rec.est_no,
					cumr_rec.class_type,
					cumr_rec.sman_code);
		break;

	case	SORT_SMAN_CTYPE:
		sprintf (sortRec [sortCnt].sortCode, "%s%s%s", 
					cumr_rec.est_no,
					cumr_rec.sman_code,	
					cumr_rec.class_type);
		break;
	}
	/*
	 * Load values into array element sortCnt.
	 */
	strcpy (sortRec [sortCnt].branchNo, cumr_rec.est_no);
	strcpy (sortRec [sortCnt].classType, cumr_rec.class_type);
	strcpy (sortRec [sortCnt].smanCode,  cumr_rec.sman_code);
	strcpy (sortRec [sortCnt].customerNo,cumr_rec.dbt_no);
	strcpy (sortRec [sortCnt].acronym,	 cumr_rec.dbt_acronym);
	sortRec [sortCnt].hhcuHash = cumr_rec.hhcu_hash;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

void
ProcessSorted (void)
{
	char	oldSalesman [3];
	char	newSalesman [3];
	char	oldCustType [4];
	char	newCustType [4];
	int		firstTime = 1,
			i;

	strcpy (oldSalesman,"  ");
	strcpy (oldCustType,"   ");
	abc_selfield (cumr, "cumr_hhcu_hash");

	for (i = 0; i < sortCnt; i++)
	{
		if (sortType == SORT_CTYPE_SMAN)
			sprintf (newCustType,"%-3.3s", sortRec [i].classType);
		else
			sprintf (newSalesman,"%-2.2s", sortRec [i].smanCode);

		cumr_rec.hhcu_hash	=	sortRec [i].hhcuHash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (cumr_rec.dbt_name," ");

		if (sortType == SORT_CTYPE_SMAN)
		{
			if (firstTime || strcmp (oldCustType,newCustType))
			{
				strcpy (oldCustType,newCustType);
				PrintCustomerType (firstTime);
			}
			firstTime = 0;
		}
		else
		{
			if (firstTime || strcmp (oldSalesman,newSalesman))
			{
				strcpy (oldSalesman,newSalesman);
				PrintSalesman (firstTime);
			}
			firstTime = 0;
		}
		ProcessFile ();
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

void
PrintSalesman (
	int		firstTime)
{
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",cumr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		strcpy (exsf_rec.salesman," ");

	expand (err_str,exsf_rec.salesman);

	switch (reportType)
	{
	case 4:
		if (envDbMcurr)
			fprintf (pp,".PD| %-2.2s  %-149.149s|\n",cumr_rec.sman_code,err_str);
		else
			fprintf (pp,".PD| %-2.2s  %-144.144s|\n",cumr_rec.sman_code,err_str);
		break;
	
	case 2:
		if (envDbMcurr)
			fprintf (pp,".PD| %-2.2s  %-82.82s|\n",cumr_rec.sman_code,err_str);
		else	
			fprintf (pp,".PD| %-2.2s  %-77.77s|\n",cumr_rec.sman_code,err_str);
		break;
	
	default:
		if (envDbMcurr)
			fprintf (pp,".PD| %-2.2s  %-151.151s|\n",cumr_rec.sman_code,err_str);
		else
			fprintf (pp,".PD| %-2.2s  %-146.146s|\n",cumr_rec.sman_code,err_str);
		break;
	}
	
	if (!firstTime)
		fprintf (pp,".PA\n");
}

void
PrintCustomerType (
	int		firstTime)
{
	strcpy (excl_rec.co_no,comm_rec.co_no);
	sprintf (excl_rec.class_type,"%-3.3s",cumr_rec.class_type);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		strcpy (excl_rec.class_desc," ");

	expand (err_str,excl_rec.class_desc);

	switch (reportType)
	{
	case 4:
		if (envDbMcurr)
			fprintf (pp,".PD| %-3.3s %-149.149s|\n",cumr_rec.class_type,err_str);
		else
			fprintf (pp,".PD| %-3.3s %-144.144s|\n",cumr_rec.class_type,err_str);
		break;
	
	case 2:
		if (envDbMcurr)
			fprintf (pp,".PD| %-3.3s %-82.82s|\n",cumr_rec.class_type,err_str);
		else	
			fprintf (pp,".PD| %-3.3s %-77.77s|\n",cumr_rec.class_type,err_str);
		break;
	
	default:
		if (envDbMcurr)
			fprintf (pp,".PD| %-3.3s %-151.151s|\n",cumr_rec.class_type,err_str);
		else
			fprintf (pp,".PD| %-3.3s %-146.146s|\n",cumr_rec.class_type,err_str);
		break;
	}
	
	if (!firstTime)
		fprintf (pp,".PA\n");
}
		 
/*
 * Validate and print lines.
 */
void
ProcessFile (void)
{
	char	poDesc [11],
			crDesc [4],
			rtDesc [11],
			wk_desc [130],
			full_desc [300];

	int 	ok_print = 0,
		 	check_same = 0;

	/*----------------------------------------------------------
	| Report Types are as follows :                            |
	| 1 = Analysis (Full Name and Address Listing)             |
	| 2 = Analysis (Short Name and Address Listing)            |
	| 3 = Analysis (Full Name , Address and Devivery Listing)  |
	| 4 = Analysis (Full Master file listing)                  |
	| 5 = Analysis (Stop Credit Listing)                       |
	| 6 = Analysis (Customer Payment Term Listing)             |
	----------------------------------------------------------*/

	switch (reportType)	
	{
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 6:
			ok_print = 1;
	    		break;
	    case 5:
		if (cumr_rec.stop_credit [0] == 'Y')
			ok_print = 1;
	    	break;
	}


	if (ok_print == 0)
		return;

	if (cumr_rec.po_flag [0] == 'Y')
		strcpy (poDesc, ML ("YES"));
	else
		strcpy (poDesc, ML ("NO"));
	
	sprintf (crDesc, "A%02d", cumr_rec.payment_flag);
	
	if (cumr_rec.stop_credit [0] == 'Y')
		strcpy (rtDesc, ML ("YES"));
	else
		strcpy (rtDesc, ML ("NO."));

	check_same = 0;
	if (!strcmp (cumr_rec.ch_adr1,cumr_rec.dl_adr1))
		check_same++;

	if (!strcmp (cumr_rec.ch_adr2,cumr_rec.dl_adr2))
		check_same++;

	if (!strcmp (cumr_rec.ch_adr3,cumr_rec.dl_adr3))
		check_same++;

	/*--------------------------------------
	|  Analysis (Full Master file listing) | 
	--------------------------------------*/
	if (reportType == 4)
	{
		fprintf (pp, "|%6.6s|", cumr_rec.dbt_no);
		fprintf (pp, "%9.9s|",  cumr_rec.dbt_acronym);
		fprintf (pp, " %2.2s |",  cumr_rec.est_no);
		fprintf (pp, " %2.2s |",  cumr_rec.department);

		if (cumr_rec.acc_type [0] == 'O')
			fprintf (pp, " OPEN |");
		else
			fprintf (pp, " B/F. |");

		fprintf (pp, " %3.3s  |",  cumr_rec.class_type);
		fprintf (pp, "  %1.1s   |",cumr_rec.price_type);
		fprintf (pp, " %-3.3s |",  poDesc);
		if (envDbMcurr)
		{
			fprintf (pp, " %-3.3s  |",cumr_rec.curr_code);
			fprintf (pp, "  %3.3s |", rtDesc);
			fprintf (pp, "%20.20s|",cumr_rec.credit_ref);
			fprintf	 (pp, "%17.17s|",cumr_rec.contact_name);
			fprintf (pp, "%15.15s|",cumr_rec.phone_no);
			fprintf (pp, "%s%20.20s|",cumr_rec.bank_code,
						clip (cumr_rec.branch_code));

			fprintf (pp, " %2.2s |", cumr_rec.area_code);
			fprintf (pp, " %2.2s |\n", cumr_rec.sman_code);
		}
		else
		{ 
			fprintf (pp, "  %3.3s |", rtDesc);
			fprintf (pp, "%20.20s|",cumr_rec.credit_ref);
			fprintf	 (pp, "%17.17s|",cumr_rec.contact_name);
			fprintf (pp, "%15.15s|",cumr_rec.phone_no);
			fprintf (pp, "%s%20.20s|",cumr_rec.bank_code,
						clip (cumr_rec.branch_code));

			fprintf (pp, " %2.2s |", cumr_rec.area_code);
			fprintf (pp, " %2.2s |\n", cumr_rec.sman_code);
		}
		/*----------------------------------------------------------
		| Customer charge to and delivery address are not the same. |
		----------------------------------------------------------*/
		if (check_same != 3)
		{
			sprintf (full_desc, "%s, %s, %s, %s, %s, %s, %s",
								clip (cumr_rec.dbt_name),
								clip (cumr_rec.ch_adr1),
								clip (cumr_rec.ch_adr2),
								clip (cumr_rec.ch_adr3),
								clip (cumr_rec.dl_adr1),
								clip (cumr_rec.dl_adr2),
								clip (cumr_rec.dl_adr3));
		}
		else
		{
			sprintf (full_desc, "%s, %s, %s, %s",
								clip (cumr_rec.dbt_name),
								clip (cumr_rec.ch_adr1),
								clip (cumr_rec.ch_adr2),
								clip (cumr_rec.ch_adr3));
		}

		if (envDbMcurr)
			fprintf (pp, "|  : %-150.150s  |\n", full_desc);
		else
			fprintf (pp, "|  : %-143.143s  |\n", full_desc);
 		if (dspaced)
		{
			fprintf (pp, "|       ");
			fprintf (pp, "          ");
			fprintf (pp, "     ");
			fprintf (pp, "     ");
			fprintf (pp, "       ");
			fprintf (pp, "       ");
			fprintf (pp, "       ");
			fprintf (pp, "      ");
			if (envDbMcurr)
			{
				fprintf (pp, "       ");
				fprintf (pp, "       ");
				fprintf (pp, "                     ");
				fprintf (pp, "                  ");
				fprintf (pp, "                ");
				fprintf (pp, "                        ");
				fprintf (pp, "     ");
				fprintf (pp, "    |\n");
			}
			else
			{
				fprintf (pp, "       ");
				fprintf (pp, "                     ");
				fprintf (pp, "                  ");
				fprintf (pp, "                ");
				fprintf (pp, "                        ");
				fprintf (pp, "     ");
				fprintf (pp, "    |\n");
			}
		}
	
	}

	/*-----------------------------------------------+
	| This is for the Customer Payment Term Listing. |	
	+-----------------------------------------------*/
	else if (reportType == 6)
	{
		fprintf (pp, "| %-6.6s|", cumr_rec.dbt_no);
		fprintf (pp, " %-9.9s|", cumr_rec.dbt_acronym);
		fprintf (pp, " %-40.40s|", clip (cumr_rec.ch_adr1));
		fprintf (pp, " %-15.15s|", cumr_rec.phone_no);
		fprintf (pp, " %-15.15s|", cumr_rec.fax_no);
		fprintf (pp, " %-20.20s|", clip (cumr_rec.contact_name));
		fprintf (pp, " %-3.3s%-2.2s  |", cumr_rec.crd_prd, cumr_rec.chq_prd);
		fprintf (pp, " %-20.20s|", clip (cumr_rec.spec_note1)); 
		fprintf (pp, "%8.8s|\n", "        ");

		/*------------------+
		| Multiline address |
		+------------------*/

		if (strlen (clip (cumr_rec.ch_adr2)) ||
			strlen (clip (cumr_rec.contact2_name)) ||
			strlen (clip (cumr_rec.spec_note2)))
		{
			fprintf (pp, "| %-6.6s|", "      ");
			fprintf (pp, " %-9.9s|", "         ");
			fprintf (pp, " %-40.40s|", clip (cumr_rec.ch_adr2));
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-20.20s|", clip (cumr_rec.contact2_name));
			fprintf (pp, " %-3.3s%-2.2s  |", "   ", "  ");
			fprintf (pp, " %-20.20s|", clip (cumr_rec.spec_note2)); 
			fprintf (pp, "%-8.8s|\n", "        ");
		}

		if (strlen (clip (cumr_rec.ch_adr3)) ||
			strlen (clip (cumr_rec.contact3_name)))
		{
			fprintf (pp, "| %-6.6s|", "      ");
			fprintf (pp, " %-9.9s|", "         ");
			fprintf (pp, " %-40.40s|", clip (cumr_rec.ch_adr3));
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-20.20s|", clip (cumr_rec.contact3_name));
			fprintf (pp, " %-3.3s%-2.2s  |", "   ", "  ");
			fprintf (pp, " %-20.20s|", "                    "); 
			fprintf (pp, "%-8.8s|\n", "        ");
		}
	
		if (strlen (clip (cumr_rec.ch_adr4)))
		{
			fprintf (pp, "| %-6.6s|", "      ");
			fprintf (pp, " %-9.9s|", "         ");
			fprintf (pp, " %-40.40s|", clip (cumr_rec.ch_adr4));
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-15.15s|", "               ");
			fprintf (pp, " %-20.20s|", "                    "); 
			fprintf (pp, " %-3.3s%-2.2s  |", "   ", "  ");
			fprintf (pp, " %-20.20s|", "                    "); 
			fprintf (pp, "%-8.8s|\n", "        ");
		}
	}

	/*-------------------------------
	| This is for the rest of them. |	
	-------------------------------*/
	else
	{	
		fprintf (pp, "|%6.6s|", cumr_rec.dbt_no);
		fprintf (pp, "%4.4s|",  rtDesc);

		if (reportType == 2)
			fprintf (pp, "%35.35s|",cumr_rec.dbt_name);
		/*-------------------------------------------
		| Analysis (Short Name and Address Listing) |	
		-------------------------------------------*/
		if (reportType != 2)
		{
			fprintf (pp, "%40.40s|",cumr_rec.dbt_name);
			if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			{
				sprintf (wk_desc,"%s,%s,%s",
						clip (cumr_rec.ch_adr1),
						clip (cumr_rec.ch_adr2),
						clip (cumr_rec.ch_adr3));

				fprintf (pp, "%-60.60s|",wk_desc);
			}
			else
			{
				sprintf (wk_desc,"%s, %s, %s",
						clip (cumr_rec.ch_adr1),
						clip (cumr_rec.ch_adr2),
						clip (cumr_rec.ch_adr3));

				fprintf (pp, "%-63.63s|",wk_desc);
			}
		}
		fprintf (pp, "%15.15s|",cumr_rec.phone_no);
		fprintf (pp, " %3.3s|", cumr_rec.class_type);
		if (envDbMcurr)
		{
			fprintf (pp, "%-3.3s |", cumr_rec.curr_code);
			fprintf (pp, "%3.3s|",  poDesc);
			fprintf (pp, "%9.9s|",  cumr_rec.dbt_acronym);
			if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
				fprintf (pp, "%2.2s|\n",cumr_rec.sman_code);
			else
				fprintf (pp,"\n");
		}
		else
		{
			fprintf (pp, "%3.3s|",  poDesc);
			fprintf (pp, "%9.9s|",  cumr_rec.dbt_acronym);
			if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
				fprintf (pp, "%2.2s|\n",cumr_rec.sman_code);
			else
				fprintf (pp,"\n");
		}

		/*------------------------------------------------------
		| Analysis (Full Name , Address and Delivery Listing)  |
		------------------------------------------------------*/
		if (reportType == 3)
		{
		    /*----------------------------------------------------------
		    | Customer charge to and delivery address are not the same. |
		    ----------------------------------------------------------*/
		    if (check_same != 3)
		    {
				fprintf (pp, "|      |");
				fprintf (pp, "    |");
				fprintf (pp, "                                        |");
				if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
				{
					sprintf (wk_desc,"%s,%s,%s", clip (cumr_rec.dl_adr1),
												 clip (cumr_rec.dl_adr2),
												 clip (cumr_rec.dl_adr3));
					fprintf (pp, "%-60.60s|",wk_desc);
				}
				else
				{
					sprintf (wk_desc,"%s, %s, %s",  clip (cumr_rec.dl_adr1),
													clip (cumr_rec.dl_adr2),
													clip (cumr_rec.dl_adr3));
					fprintf (pp, "%-63.63s|",wk_desc);
				}
				fprintf (pp, "               |");
				fprintf (pp, "    |");

				if (envDbMcurr)
				{
					fprintf (pp, "    |");
					fprintf (pp, "   |");
					fprintf (pp, "         |");
					if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
						fprintf (pp, "  |\n");
					else
						fprintf (pp,"\n");
				}
				else
				{
					fprintf (pp, "   |");
					fprintf (pp, "         |");
					if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
						fprintf (pp, "  |\n");
					else
						fprintf (pp,"\n");
				}
		    }
		}

 		if (dspaced)
		{
			fprintf (pp, "|      |");
			fprintf (pp, "    |");
	
			if (reportType == 2)
				fprintf (pp, "                                   |");
			if (reportType != 2)
			{
				fprintf (pp, "                                        |");
				if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
					fprintf (pp, "%-60.60s|"," ");
				else
					fprintf (pp, "%-63.63s|"," ");
			}
			fprintf (pp, "               |");
			fprintf (pp, "    |");
			if (envDbMcurr)
			{
				fprintf (pp, "    |");
				fprintf (pp, "   |");
				fprintf (pp, "         |");
				if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
					fprintf (pp, "  |\n");
				else
					fprintf (pp,"\n");
			}
			else
			{
				fprintf (pp, "   |");
       		 		fprintf (pp, "         |");
				if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
					fprintf (pp, "  |\n");
				else
					fprintf (pp,"\n");
			}
		}
	}

}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (void)
{
	fprintf (pp,".EOF\n");
}

/*==========================================================
| Headings for the following types of reports.             |
| 1 = Analysis (Full Name and Address Listing)             |
| 3 = Analysis (Full Name , Address and Devivery Listing)  |
| 5 = Analysis (Stop Credit Listing)                       |
==========================================================*/
void
HeadingOne (void)
{
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",runDesc [runType - 1]);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".C%s %s\n",reportDesc [reportType - 1],
				sortDesc [sortType -1]);

	if (runType != 1)
		fprintf (pp,".B1\n.E%s AS AT : %s\n.B1\n",
				clip (comm_rec.est_name),SystemTime ());
	else
		fprintf (pp,".B1\n.EAS AT : %-25.25s\n.B1\n", SystemTime ());

	fprintf (pp, ".R========");
	fprintf (pp, "=====");
	fprintf (pp, "=========================================");
	fprintf (pp, "======================================");
	fprintf (pp, "======================");
	fprintf (pp, "=================");
	fprintf (pp, "=====");
	if (envDbMcurr)
	{
		fprintf (pp, "=====");
		fprintf (pp, "====");
		fprintf (pp, "=============\n");
	}
	else
	{
		fprintf (pp, "====");
		fprintf (pp, "=============\n");
	}

	fprintf (pp, "========");
	fprintf (pp, "=====");
	fprintf (pp, "=========================================");
	fprintf (pp, "======================================");
	fprintf (pp, "======================");
	fprintf (pp, "=================");
	fprintf (pp, "=====");
	if (envDbMcurr)
	{
		fprintf (pp, "=====");
		fprintf (pp, "====");
		fprintf (pp, "=============\n");
	}
	else
	{
		fprintf (pp, "====");
		fprintf (pp, "=============\n");
	}

	fprintf (pp, "| CUST.|");
	fprintf (pp, "STOP|");
	fprintf (pp, "       C U S T O M E R  N A M E         |");
	fprintf (pp, "                      D E T A I L S");
	if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
		fprintf (pp, "                         |");
	else
		fprintf (pp, "                            |");
	fprintf (pp, "      PHONE    |");
	fprintf (pp, "CUST|");
	if (envDbMcurr)
	{
		fprintf (pp, "CURR|");
		fprintf (pp, "P/O|");
		fprintf (pp, "CUSTOMER |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "SM|\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "P/O|");
		fprintf (pp, "CUSTOMER |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "SM|\n");
		else
			fprintf (pp, "\n");
	}

	fprintf (pp, "| CODE |");
	fprintf (pp, "CRDT|");
	fprintf (pp, "                                        |");
	fprintf (pp, "                                   ");
	if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
		fprintf (pp, "                         |");
	else
		fprintf (pp, "                            |");
	fprintf (pp, "     NUMBER    |");
	fprintf (pp, "TYPE|");
	if (envDbMcurr)
	{
		fprintf (pp, "CODE|");
		fprintf (pp, "REQ|");
		fprintf (pp, " ACRONYM |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "  |\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "REQ|");
		fprintf (pp, " ACRONYM |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "  |\n");
		else
			fprintf (pp, "\n");
	}

	fprintf (pp, "|------|");
	fprintf (pp, "----|");
	fprintf (pp, "----------------------------------------|");
	fprintf (pp, "-----------------------------------");
	if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
		fprintf (pp, "-------------------------|");
	else
		fprintf (pp, "----------------------------|");
	fprintf (pp, "---------------|");
	fprintf (pp, "----|");
	if (envDbMcurr)
	{
		fprintf (pp, "----|");
		fprintf (pp, "---|");
       	fprintf (pp, "---------|");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "--|\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "---|");
 		fprintf (pp, "---------|");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "--|\n");
		else
			fprintf (pp, "\n");
	}
}

/*==========================================================
| Headings for the following types of reports.             |
| 2 = Analysis (Short Name and Address Listing)            |
==========================================================*/
void
HeadingTwo (void)
{
	fprintf (pp,".L92\n");
	fprintf (pp,".E%s\n",runDesc [runType - 1]);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".C%s %s\n",reportDesc [reportType - 1],
				sortDesc [sortType -1]);

	if (runType != 1)
		fprintf (pp,".B1\n.E%s AS AT : %s\n.B1\n",
			clip (comm_rec.est_name),SystemTime ());
	else
		fprintf (pp,".B1\n.EAS AT : %s\n.B1\n", SystemTime ());

	fprintf (pp, ".R========");
	fprintf (pp, "=====");
	fprintf (pp, "====================================");
	fprintf (pp, "================");
	fprintf (pp, "=====");
	if (envDbMcurr)
	{
		fprintf (pp, "=====");
		fprintf (pp, "====");
		fprintf (pp, "==========");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "===\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "====");
		fprintf (pp, "==========");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "===\n");
		else
			fprintf (pp, "\n");
	}


	fprintf (pp, "========");
	fprintf (pp, "=====");
	fprintf (pp, "====================================");
	fprintf (pp, "================");
	fprintf (pp, "=====");
	if (envDbMcurr)
	{
		fprintf (pp, "=====");
		fprintf (pp, "====");
		fprintf (pp, "==========");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "===\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "====");
		fprintf (pp, "==========");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "===\n");
		else
			fprintf (pp, "\n");
	}

        fprintf (pp, "| CUST.|");
	fprintf (pp, "STOP|");
	fprintf (pp, "     C U S T O M E R  N A M E      |");
	fprintf (pp, "      PHONE    |");
	fprintf (pp, "CUST|");
	if (envDbMcurr)
	{
		fprintf (pp, "CURR|");
		fprintf (pp, "P/O|");
		fprintf (pp, "CUSTOMER |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "SM|\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "P/O|");
		fprintf (pp, "CUSTOMER |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "SM|\n");
		else
			fprintf (pp, "\n");
	}

	fprintf (pp, "| CODE |");
	fprintf (pp, "CRDT|");
	fprintf (pp, "                                   |");
	fprintf (pp, "     NUMBER    |");
	fprintf (pp, "TYPE|");
	if (envDbMcurr)
	{
		fprintf (pp, "CODE|");
		fprintf (pp, "REQ|");
		fprintf (pp, " ACRONYM |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "  |\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "REQ|");
		fprintf (pp, " ACRONYM |");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "  |\n");
		else
			fprintf (pp, "\n");
	}

	fprintf (pp, "|------|");
	fprintf (pp, "----|");
	fprintf (pp, "-----------------------------------|");
	fprintf (pp, "---------------|");
	fprintf (pp, "----|");
	if (envDbMcurr)
	{
		fprintf (pp, "----|");
		fprintf (pp, "---|");
		fprintf (pp, "---------|");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "--|\n");
		else
			fprintf (pp, "\n");
	}
	else
	{
		fprintf (pp, "---|");
		fprintf (pp, "---------|");
		if (sortType != SORT_SMAN_DBT && sortType != SORT_SMAN_ACR)
			fprintf (pp, "--|\n");
		else
			fprintf (pp, "\n");
	}
}

/*==========================================================
| Headings for the following types of reports.             |
| 4 = Analysis (Full Master file listing)                  |
==========================================================*/
void
HeadingThree (void)
{
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",runDesc [runType - 1]);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".C%s %s\n",reportDesc [reportType - 1],
			sortDesc [sortType -1]);

	if (runType != 1)
		fprintf (pp,".B1\n.E%s AS AT : %s\n.B1\n",
				clip (comm_rec.est_name),SystemTime ());
	else
		fprintf (pp,".B1\n.EAS AT : %s\n.B1\n", SystemTime ());

	fprintf (pp, ".R========");
	fprintf (pp, "==========");
	fprintf (pp, "=====");
	fprintf (pp, "=====");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "======");
	if (envDbMcurr)
	{
       		fprintf (pp, "=======");
		fprintf (pp, "=======");
		fprintf (pp, "=====================");
		fprintf (pp, "==================");
		fprintf (pp, "================");
		fprintf (pp, "========================");
		fprintf (pp, "=====");
		fprintf (pp, "=====\n");
	}
	else
	{
		fprintf (pp, "=======");
		fprintf (pp, "=====================");
		fprintf (pp, "==================");
		fprintf (pp, "================");
		fprintf (pp, "========================");
		fprintf (pp, "=====");
		fprintf (pp, "=====\n");
	}

	fprintf (pp, "========");
	fprintf (pp, "==========");
	fprintf (pp, "=====");
	fprintf (pp, "=====");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "======");
	if (envDbMcurr)
	{
		fprintf (pp, "=======");
		fprintf (pp, "=======");
		fprintf (pp, "=====================");
		fprintf (pp, "==================");
		fprintf (pp, "================");
		fprintf (pp, "========================");
		fprintf (pp, "=====");
		fprintf (pp, "=====\n");
	}
	else
	{
		fprintf (pp, "=======");
		fprintf (pp, "=====================");
		fprintf (pp, "==================");
		fprintf (pp, "================");
		fprintf (pp, "========================");
		fprintf (pp, "=====");
		fprintf (pp, "=====\n");
	}

	fprintf (pp, "|CUST. |");
	fprintf (pp, "CUSTOMER |");
	fprintf (pp, " BR |");
	fprintf (pp, " DP |");
	fprintf (pp, " ACC .|");
	fprintf (pp, " CUST.|");
	fprintf (pp, " PRICE|");
	fprintf (pp, " P/O |");
	if (envDbMcurr)
	{	
		fprintf (pp, " CURR |");
		fprintf (pp, " STOP |");
		fprintf (pp, "  CREDIT REFERENCE. |");
		fprintf (pp, "  CONTACT NAME.  |");
		fprintf (pp, "   PHONE NO    |");
		fprintf (pp, "   BANK AND BRANCH     |");
		fprintf (pp, "AREA|");
		fprintf (pp, "SMAN|\n");
	}
	else
	{
		fprintf (pp, " STOP |");
		fprintf (pp, "  CREDIT REFERENCE. |");
		fprintf (pp, "  CONTACT NAME.  |");
		fprintf (pp, "   PHONE NO    |");
		fprintf (pp, "   BANK AND BRANCH     |");
		fprintf (pp, "AREA|");
		fprintf (pp, "SMAN|\n");
	}

	fprintf (pp, "|NUMBER|");
	fprintf (pp, " ACRONYM |");
	fprintf (pp, " NO |");
	fprintf (pp, " NO |");
	fprintf (pp, " TYPE |");
	fprintf (pp, " TYPE |");
	fprintf (pp, " TYPE |");
	fprintf (pp, " REQ |");
	if (envDbMcurr)
	{
		fprintf (pp, " CODE |");
		fprintf (pp, "CREDIT|");
		fprintf (pp, "                    |");
		fprintf (pp, "                 |");
		fprintf (pp, "               |");
		fprintf (pp, "        DETAILS        |");
		fprintf (pp, "CODE|");
		fprintf (pp, "CODE|\n");
	}
	else
	{
		fprintf (pp, "CREDIT|");
		fprintf (pp, "                    |");
		fprintf (pp, "                 |");
		fprintf (pp, "               |");
		fprintf (pp, "        DETAILS        |");
		fprintf (pp, "CODE|");
		fprintf (pp, "CODE|\n");
	}

	fprintf (pp, "|------|");
	fprintf (pp, "---------|");
	fprintf (pp, "----|");
	fprintf (pp, "----|");
	fprintf (pp, "------|");
	fprintf (pp, "------|");
	fprintf (pp, "------|");
	fprintf (pp, "-----|");
	fprintf (pp, "------|");
	if (envDbMcurr)
	{
		fprintf (pp, "------|");
		fprintf (pp, "--------------------|");
		fprintf (pp, "-----------------|");
		fprintf (pp, "---------------|");
		fprintf (pp, "-----------------------|");
		fprintf (pp, "----|");
		fprintf (pp, "----|\n");
	}
	else
	{
		fprintf (pp, "--------------------|");
		fprintf (pp, "-----------------|");
		fprintf (pp, "---------------|");
		fprintf (pp, "-----------------------|");
		fprintf (pp, "----|");
		fprintf (pp, "----|\n");
	}
}

/*=========================================================+
| Heading for 6 = Analysis (Customer Payment Term Listing) |
+=========================================================*/
void
HeadingSix (void)
{
	fprintf (pp,".L158\n");
	fprintf (pp,".E%s\n",runDesc [runType - 1]);
	fprintf (pp,".B1\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".B1\n");
	fprintf (pp,".C%s %s\n",reportDesc [reportType - 1],
			sortDesc [sortType -1]);

	if (runType != 1)
		fprintf (pp,".B1\n.E%s AS AT : %s\n.B1\n",
				clip (comm_rec.est_name),SystemTime ());
	else
		fprintf (pp,".B1\n.EAS AT : %s\n.B1\n", SystemTime ());

	fprintf (pp, ".R=========");
	fprintf (pp,   "===========");
	fprintf (pp,   "==========================================");
	fprintf (pp,   "=================");
	fprintf (pp,   "=================");
	fprintf (pp,   "======================");
	fprintf (pp,   "=========");
	fprintf (pp,   "======================");
	fprintf (pp,   "=========\n");

	fprintf (pp,   "=========");
	fprintf (pp,   "===========");
	fprintf (pp,   "==========================================");
	fprintf (pp,   "=================");
	fprintf (pp,   "=================");
	fprintf (pp,   "======================");
	fprintf (pp,   "=========");
	fprintf (pp,   "======================");
	fprintf (pp,   "=========\n");

	fprintf (pp,   "| CUST. |");
	fprintf (pp,   "CUSTOMER  |");
	fprintf (pp,   " M A I L I N G    A D D R E S S          |");
	fprintf (pp,   " PHONE NO       |");
	fprintf (pp,   " FAX NO         |");
	fprintf (pp,   " CONTACT PERSON      |");
	fprintf (pp,   " PAYMENT|");
	fprintf (pp,   " COLLECTION          |");
	fprintf (pp,   " PAYDATE|\n");

	fprintf (pp,   "| NO    |");
	fprintf (pp,   " ACRONYM  |");
	fprintf (pp,   "                                         |");
	fprintf (pp,   "                |");
	fprintf (pp,   "                |");
	fprintf (pp,   "                     |");
	fprintf (pp,   " TERM   |");
	fprintf (pp,   " REQUIREMENT         |");
	fprintf (pp,   "        |\n");

	fprintf (pp,   "|-------|");
	fprintf (pp,   "----------|");
	fprintf (pp,   "-----------------------------------------|");
	fprintf (pp,   "----------------|");
	fprintf (pp,   "----------------|");
	fprintf (pp,   "---------------------|");
	fprintf (pp,   "--------|");
	fprintf (pp,   "---------------------|");
	fprintf (pp,   "--------|\n");
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
