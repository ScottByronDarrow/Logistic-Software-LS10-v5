/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sa_sadf_dsp.c,v 5.1 2002/03/11 08:39:54 scott Exp $
|  Program Name  : (sa_sadf_dsp.c)
|  Program Desc  : (DisplayItem Sales By Customer.)
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 26/10/88         |
|---------------------------------------------------------------------|
| $Log: sa_sadf_dsp.c,v $
| Revision 5.1  2002/03/11 08:39:54  scott
| Updated to remove disk based sort.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_sadf_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf_dsp/sa_sadf_dsp.c,v 5.1 2002/03/11 08:39:54 scott Exp $";

#include	<pslscr.h>
#include	<pr_format3.h>
#include	<ml_sa_mess.h>
#include	<ml_std_mess.h>
#include 	<arralloc.h>

#undef      PSIZE
#define		PSIZE		14
#define		CATG_SEL	 (sel_type [0] == 'C')
#define		RESET_LINE_NUM	ln_num = (ln_num >= PSIZE) ? ln_num % PSIZE : ln_num

#define		REP_CUST	 (cust_type [0] == 'C')
#define		REP_TYPE	 (cust_type [0] == 'T')
#define		REP_SMAN	 (cust_type [0] == 'S')

#define		BY_CUST		 (cust_prod [0] == 'C')		
#define		DETAIL		 (sum_det [0] == 'D')		
#define		BY_CAT		 (type [0] == 'A')		
#define		BY_SMAN		 (type [0] == 'S')		
#define		COST_MGN	 (costmgn [0] == 'Y')		

char		*HEADER = "                |                                        | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*AHEADER = " CATEGORY NO /  |         CATEGORY DESCRIPTION /         | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*THEADER = "  CUST TYPE /   |        CUST TYPE DESCRIPTION /         | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*SHEADER = " SALESMAN NO /  |            SALESMAN NAME /             | <-------- MTD SALES -----------> | <--------- YTD SALES ----------> ";
char		*ITEM_HEAD = "ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*ITEM_HEAD1 = "  ITEM NUMBER   |         ITEM     DESCRIPTION           |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*CUST_HEAD = "CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*CUST_HEAD1 = "    CUSTOMER    |            CUSTOMER     NAME           |   QTY  | SALES  |  COST  |%MARGIN|   QTY  | SALES  |  COST  |%MARGIN";
char		*SEPARATION = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char		*UNDERLINE = "===============================================================================================================================";

char		*XHEADER =    "                |                                        |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XAHEADER =   " CATEGORY NO /  |         CATEGORY DESCRIPTION /         |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XTHEADER =   "  CUST TYPE /   |        CUST TYPE DESCRIPTION /         |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XSHEADER =   " SALESMAN NO /  |            SALESMAN NAME /             |<-- MTD SALES -->|<-- YTD SALES -->";
char		*XITEM_HEAD = "ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |   QTY  | SALES  |   QTY  | SALES  ";
char		*XITEM_HEAD1 ="  ITEM NUMBER   |         ITEM     DESCRIPTION           |   QTY  | SALES  |   QTY  | SALES  ";
char		*XCUST_HEAD = "CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |   QTY  | SALES  |   QTY  | SALES  ";
char		*XCUST_HEAD1 ="    CUSTOMER    |            CUSTOMER     NAME           |   QTY  | SALES  |   QTY  | SALES  ";
char		*XSEPARATION = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";
char		*XUNDERLINE = "=============================================================================================";

#include	"schema"

struct commRecord	comm_rec;
struct sadfRecord	sadf_rec;
struct cumrRecord	cumr_rec;
struct exclRecord	excl_rec;
struct exsfRecord	exsf_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct esmrRecord	esmr_rec;

	float	*sadf_qty	=	&sadf_rec.qty_per1;
	double	*sadf_sal	=	&sadf_rec.sal_per1;
	double	*sadf_cst	=	&sadf_rec.cst_per1;

	FILE	*fin, 
		    *fout;

	int		cnt	=	0;
	char	dspStr 			[200], 
            cust_type 		[2], 
            startClass 		[2], 
    		endClass 		[2], 
	    	startCat 		[12], 
		    endCat 			[12], 
    		startType 		[4], 
		    endType 		[4], 
		    startSalesman 	[3], 
    		endSalesman 	[3], 
	    	startCustomer 	[7], 
		    endCustomer 	[7], 
    		lower 			[17], 
		    upper 			[17], 
		    startGroup 		[13], 
		    endGroup 		[13];
	
	int	ln_num = 0, 
		column = 0, 
		fiscal, 
		envDbCo = 0, 
		envDbFind = 0;

	char	branchNo [3], 
			br_no [3], 
			currSman [3], 
			prevBrNo [3], 
			currBrNo [3], 
			prevCat [12], 
			currCat [12], 
			prevItem [17], 
			currItem [17], 
			prevCust [7], 
			currCust [7], 
			prevType [4], 
			currType [4], 
			cust_prod [2], 
			prt_dsp [2], 
			sum_det [2], 
			type [2], 
			sel_type [2], 
			costmgn [2];

	int		found_data = 0, 
			first_time = TRUE, 
			printed = FALSE, 
			srt_all = FALSE, 
			end_all = FALSE, 
			lpno = 1, 
			ptr_offset [8], 
			curr_mnth, 
			BY_CO,
			READ_MASTER = FALSE;

	long	prevHash [2]	=	{0,0},
			currHash [2]	=	{0,0};

	float	mtdQty [5]	=	{0,0,0,0,0},
			ytdQty [5]	=	{0,0,0,0,0};

	double	mtdSales [5]	=	{0,0,0,0,0},
			mtdCostSale [5]	=	{0,0,0,0,0},
			ytdSales [5]	=	{0,0,0,0,0},
			ytdCostSale [5]	=	{0,0,0,0,0};

	struct {
		char	*heading;
		char	*by_what;
	} head_list [6] = {
		{"Customer"                 , "By Item   "}, 
		{"Cust Type"                , "By Customer"}, 
		{"Salesman"                 , "By Customer"}, 
		{"Category"                 , "By Item   "}, 
		{"Item  "                   , "By Customer"}, 
		{"Item (Category Selection)", "By Customer"}, 
	};

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[36];
	char	brNo		[sizeof sadf_rec.br_no];
	char	catNo		[sizeof inmr_rec.category];
	char	itemNo		[sizeof inmr_rec.item_no];
	char	custNo		[sizeof cumr_rec.dbt_no];
	char	custType	[sizeof cumr_rec.class_type];
	char	smanNo		[sizeof sadf_rec.sman];
	float	mtdQty;
	float	ytdQty;
	double	mtdSale;
	double	ytdSale;
	double	mtdCost;
	double	ytdCost;
	long	hhbrHash;
	long	hhcuHash;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

/*
 * Local Function Prototypes.
 */
int 	CalculateMTD 			(void);
int 	CalculateYTD 			(void);
int 	CheckBreak 				(void);
int 	CheckPage 				(void);
int 	ValidCustomer 			(void);
int 	ValidItem 				(void);
void 	CategoryHeader 			(void);
void 	CloseDB 				(void);
void 	CustomerHeader 			(void);
void 	DrawLine 				(void);
void 	InitArray 				(void);
void 	InitOutput 				(void);
void 	ItemHeader 				(void);
void 	OpenDB 					(void);
void 	PrintHeader 			(int, char *, int);
void 	PrintLine 				(void);
void 	PrintTotal 				(char *);
void 	ProcessCumr 			(void);
void 	ProcessCustomer 		(void);
void 	ProcessData 			(int, int);
void 	MainProcessData 		(void);
void 	ProcessInmr 			(void);
void 	ProcessProduct 			(void);
void 	SalesmanHeader 			(void);
void 	shutdown_prog 			(void);
void 	StoreCustomerData 		(void);
void 	StoreProductData 		(void);
void 	TypeHeader 				(void);

/*
 * Main Processing Routine . 
 */
int
main (
 int    argc, 
 char*  argv [])
{
	int	REPORT_BY = 0;
	char	*sptr;

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	strcpy (startClass, "A");
	strcpy (endClass, "Z");
	strcpy (startCat, "           ");
	strcpy (endCat, "~~~~~~~~~~~");
	strcpy (startType, "   ");
	strcpy (endType, "~~~");
	strcpy (startSalesman, "  ");
	strcpy (endSalesman, "~~");
	strcpy (startCustomer, "      ");
	strcpy (endCustomer, "~~~~~~");
	strcpy (lower, "                ");
	strcpy (upper, "~~~~~~~~~~~~~~~~");

	if (argc != 10)
	{
		print_at (0, 0, ML (mlSaMess728), argv [0]);
		print_at (1, 0, ML (mlSaMess729));
		print_at (2, 0, ML (mlSaMess730));
		print_at (3, 0, ML (mlSaMess731));
		print_at (4, 0, ML (mlSaMess732));
		print_at (5, 0, ML (mlSaMess733));
		print_at (6, 0, ML (mlSaMess734));
		print_at (7, 0, ML (mlSaMess735));
		print_at (8, 0, ML (mlSaMess736));
        return (EXIT_FAILURE);
	}

	sprintf (cust_prod, "%1.1s", argv [1]);
	sprintf (sum_det, "%1.1s", argv [2]);
	sprintf (type, "%1.1s", argv [3]);
	sprintf (sel_type, "%1.1s", argv [4]);
	sprintf (costmgn, "%-1.1s", argv [9]);

	init_scr ();
	set_tty ();

	sptr = chk_env ((BY_CUST) ? "SA_BYCUS" : "SA_BYPRD");
	READ_MASTER = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (BY_CUST)
	{
		switch (type [0])
		{
		case	'C' :
			/*
			 * Sales By Customer. 
			 */
			REPORT_BY = 0;
			strcpy (cust_type, "C");
			sprintf (startCustomer, "%-6.6s", argv [5]);
			sprintf (endCustomer, "%-6.6s", argv [6]);
			if (!strcmp (startCustomer, "      "))
				srt_all = TRUE;

			if (!strcmp (endCustomer, "~~~~~~"))
				end_all = TRUE;
			break;

		case	'T' :
			/*
			 * Sales By Customer Type. 
			 */
			REPORT_BY = 1;
			strcpy (cust_type, "T");
			sprintf (startType, "%-3.3s", argv [5]);
			sprintf (endType, "%-3.3s", argv [6]);
			if (!strcmp (startType, "   "))
				srt_all = TRUE;

			if (!strcmp (endType, "~~~"))
				end_all = TRUE;
			break;

		case	'S' :
			/*
			 * Sales By Salesman. 
			 */
			REPORT_BY = 2;
			strcpy (cust_type, "S");
			sprintf (startSalesman, "%-2.2s", argv [5]);
			sprintf (endSalesman, "%-2.2s", argv [6]);
			if (!strcmp (startSalesman, "  "))
				srt_all = TRUE;

			if (!strcmp (endSalesman, "~~"))
				end_all = TRUE;
			break;
		}
	}
	else 
	{
		if (BY_CAT || (!BY_CAT && CATG_SEL))
		{
			if (BY_CAT)
				REPORT_BY = 3;
			else
				REPORT_BY = 5;

			sprintf (startGroup, "%-12.12s", argv [5]);
			sprintf (endGroup, "%-12.12s", argv [6]);
			sprintf (startClass, "%1.1s", startGroup);
			sprintf (startCat, "%-11.11s", startGroup + 1);
			sprintf (endClass, "%1.1s", endGroup);
			sprintf (endCat, "%-11.11s", endGroup + 1);
			if (!strcmp (startCat, "           "))
				srt_all = TRUE;

			if (!strcmp (endCat, "~~~~~~~~~~~"))
				end_all = TRUE;
		}
		else	/* By Item */ 
		{
			REPORT_BY = 4;
			sprintf (lower, "%-16.16s", argv [5]);
			sprintf (upper, "%-16.16s", argv [6]);
			if (!strcmp (lower, "                "))
				srt_all = TRUE;

			if (!strcmp (upper, "~~~~~~~~~~~~~~~~"))
				end_all = TRUE;
		}
	}

	if (!strncmp (argv [8], "All", 3) || !strncmp (argv [8], "ALL", 3))
	{
		BY_CO = TRUE;
		strcpy (br_no, "  ");
	}
	else
	{
		BY_CO = FALSE;
		sprintf (br_no, "%-2.2s", argv [8]);
	}

	OpenDB ();

	DateToDMY (comm_rec.dbt_date, NULL, &curr_mnth, NULL);

	sptr = chk_env ("SA_YEND");
	fiscal = (sptr == (char *)0) ? comm_rec.fiscal : atoi (sptr);

	if (fiscal < 1 || fiscal > 12)
		fiscal = comm_rec.fiscal;

	swide ();
/*	clear ();
	crsr_off ();
	fflush (stdout);

	sprintf (err_str, "Processing : %s Sales %s %s", 
		head_list [REPORT_BY].heading, 
		 (REPORT_BY == 0) ? " " : head_list [REPORT_BY].by_what, 
		 (DETAIL) ? "(Detailed)" : "(Summary)");
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
*/
  
	if (BY_CUST)
	{
		if (envDbCo && envDbFind)
			READ_MASTER = TRUE;

		if (READ_MASTER)
			ProcessCumr ();
		else
			ProcessCustomer ();
	}
	else
	{
		if (READ_MASTER)
			ProcessInmr ();
		else
			ProcessProduct ();
	}


	InitOutput ();

	/*
	 * Process data in sort file 
	 */
	MainProcessData ();

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
 * Open data base files . 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sadf, sadf_list, SADF_NO_FIELDS, (BY_CUST) ? "sadf_id_no2" : "sadf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	if (BY_CUST)
	{
		if (REP_TYPE)
			open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");

		if (REP_SMAN)
			open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	}
	else
		open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

/*
 * Close data base files
 */
void
CloseDB (void)
{
	abc_fclose (sadf);
	if (BY_CUST)
	{
		if (REP_TYPE)
			abc_fclose (excl);

		if (REP_SMAN)
			abc_fclose (exsf);
		
		if (REP_CUST)
			abc_fclose (cumr);
	}
	else
		abc_fclose (excf);

	abc_fclose (inmr);
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_dbclose ("data");
}

/*
 * Read data into sort file, then process the information Read whole cumr file
 */
void
ProcessCumr (void)
{
	char	save_br [3];

	abc_selfield (cumr, "cumr_id_no");
	abc_selfield (sadf, "sadf_id_no3");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, br_no);
	if (!strcmp (br_no, "  "))
	{
		strcpy (cumr_rec.dbt_no, "      ");
		cc = find_rec (cumr , &cumr_rec, GTEQ, "r");
		if (cc)
			return;
	}

	if (REP_CUST)
		strcpy (cumr_rec.dbt_no, startCustomer);
	else
		strcpy (cumr_rec.dbt_no, "      ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (cumr_rec.est_no, br_no) && strcmp (br_no, "  "))
			break;
		
		if (!ValidCustomer ())
		{
			if (REP_CUST)
			{
			    if (strcmp (br_no, "  "))
				break;

			    strcpy (cumr_rec.dbt_no, "~~~~~~");
			    cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			    if (!cc)
			    {
				strcpy (save_br, cumr_rec.est_no);
				strcpy (cumr_rec.dbt_no, startCustomer);
				cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
				if (!cc && strcmp (cumr_rec.est_no, save_br))
					{
				    	cc = find_rec (cumr, &cumr_rec, LT, "r");
				    	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
					}
			    }
			    continue;
			}
			else
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}
		}
		sadf_rec.hhcu_hash = cumr_rec.hhcu_hash;
		sadf_rec.hhbr_hash = 0L;
		cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
		while (!cc && sadf_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			if (sadf_rec.year [0] != 'C')
			{
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
				continue;
			}

			if (BY_SMAN)
			{
				if (strcmp (sadf_rec.sman, startSalesman) < 0 || 
				     strcmp (sadf_rec.sman, endSalesman) > 0)
				{
					cc = find_rec (sadf, &sadf_rec, NEXT, "r");
					continue;
				}
			}
			CalculateMTD ();
			CalculateYTD ();

			/*
			 * Store if sales & cost of sales <> 0.00	
			 */
			if (mtdQty [0]   != 0 ||
			     mtdSales [0] != 0 ||
			     mtdCostSale [0] != 0 ||
			     ytdQty [0]   != 0 ||
			     ytdSales [0] != 0 ||
			     ytdCostSale [0] != 0)
					StoreCustomerData ();

			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	abc_selfield (cumr, "cumr_hhcu_hash");
}

int
ValidCustomer (void)
{
	int	store_ok = TRUE;

	switch (type [0])
	{
	case	'C':
		if (strcmp (cumr_rec.dbt_no, startCustomer) < 0 || 
		     strcmp (cumr_rec.dbt_no, endCustomer) > 0)
			store_ok = FALSE;
		break;

	case	'T':
		if (strcmp (cumr_rec.class_type, startType) < 0 || 
 		     strcmp (cumr_rec.class_type, endType) > 0)
			store_ok = FALSE;
		break;

	}
	return (store_ok);
}

/*
 * Read data into sort file, then process the information 
 * Read whole sadf file.                                 
 */
void
ProcessCustomer (void)
{
	char	rec_br [3];

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (sadf_rec.co_no, comm_rec.co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "C");
	strcpy (rec_br, br_no);
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;
	strcpy (sadf_rec.sman, "  ");

	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");

	while (!cc && 
	       !strcmp (sadf_rec.co_no, comm_rec.co_no) && 
	       (!strcmp (sadf_rec.br_no, br_no) || !strcmp (rec_br, "  ")))
	{
	       	if (strcmp (sadf_rec.year, "C"))
		{
			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			continue;
		}

		if (BY_SMAN && (strcmp (sadf_rec.sman, startSalesman) < 0 || 
			          strcmp (sadf_rec.sman, endSalesman) > 0))
		{
			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			continue;
		}
		cumr_rec.hhcu_hash = sadf_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");
			sprintf (cumr_rec.dbt_no, "%-6.6s", "DELETE");
			sprintf (cumr_rec.class_type, "%-3.3s", "DEL");
		}

		CalculateMTD ();
		CalculateYTD ();

		/*
		 * Store if sales & cost of sales <> 0.00	
		 */
		if (ValidCustomer () && (mtdQty [0] != 0 ||
		     mtdSales [0]  != 0 ||
		     mtdCostSale [0]  != 0 ||
		     ytdQty [0]    != 0 ||
		     ytdSales [0]  != 0 ||
		     ytdCostSale [0]  != 0))
				StoreCustomerData ();

		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}
}

/*
 * Read data into sort file, then process the information 
 * Read whole inmr file.                                 
 */
void
ProcessInmr (void)
{
	char	rec_br [3];

	if (BY_CAT || (!BY_CAT && CATG_SEL))
		abc_selfield (inmr, "inmr_id_no_3");
	else
		abc_selfield (inmr, "inmr_id_no");

	abc_selfield (sadf, "sadf_id_no4");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (rec_br, br_no);
	if (BY_CAT || (!BY_CAT && CATG_SEL))
	{
		strcpy (inmr_rec.inmr_class, startClass);
		strcpy (inmr_rec.category, startCat);
		sprintf (inmr_rec.item_no, "%16.16s", " ");
	}
	else
	{
		strcpy (inmr_rec.inmr_class, " ");
		sprintf (inmr_rec.category, "%11.11s", " ");
		strcpy (inmr_rec.item_no, lower);
	}
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		if (ValidItem () == 0)
		{
			if (!BY_CAT && !CATG_SEL)
				break;

			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
			
		sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sadf_rec.hhcu_hash = 0L;
		cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
		while (!cc && sadf_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (strcmp (sadf_rec.year, "C"))
			{
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
				continue;
			}

			if (strcmp (sadf_rec.co_no, comm_rec.co_no) || 
			    (strcmp (sadf_rec.br_no, br_no) 
			    && strcmp (rec_br, "  ")))
			{
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
				continue;
			}

			CalculateMTD ();
			CalculateYTD ();

			/*
			 * Store if sales & cost of sales <> 0.00	
			 */
			if (mtdQty [0]   != 0 ||
			     mtdSales [0] != 0 ||
			     mtdCostSale [0] != 0 ||
			     ytdQty [0]   != 0 ||
			     ytdSales [0] != 0 ||
			     ytdCostSale [0] != 0)
					StoreProductData ();

			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	abc_selfield (inmr, "inmr_hhbr_hash");
}

int
ValidItem (void)
{
	char	item_gp [13];
	int	store_ok = TRUE;

	switch (type [0])
	{
	case	'I':
		if (CATG_SEL)
		{
			sprintf (item_gp, "%-1.1s%-11.11s", inmr_rec.inmr_class, 
				inmr_rec.category);

			if (strcmp (item_gp, startGroup) < 0 || 
			    strcmp (item_gp, endGroup) > 0)
				store_ok = FALSE;
		break;
		}
		else
		{
			if (strcmp (inmr_rec.item_no, lower) < 0 || 
		    	    strcmp (inmr_rec.item_no, upper) > 0)
				store_ok = FALSE;
		}
		break;

	case	'A':
		sprintf (item_gp, "%-1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
		if (strcmp (item_gp, startGroup) < 0 || strcmp (item_gp, endGroup) > 0)
			store_ok = FALSE;
		break;

	}
	return (store_ok);
}

/*
 * Read data into sort file, then process the information 
 * Read whole sadf file.                                 
 */
void
ProcessProduct (void)
{
	char	rec_br [3];

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (sadf_rec.co_no, comm_rec.co_no);
	strcpy (sadf_rec.br_no, br_no);
	strcpy (sadf_rec.year, "C");
	strcpy (rec_br, br_no);
	sadf_rec.hhbr_hash = 0L;
	sadf_rec.hhcu_hash = 0L;
	strcpy (sadf_rec.sman, "  ");

	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");

	while (!cc && 
	       !strcmp (sadf_rec.co_no, comm_rec.co_no) && 
	       (!strcmp (sadf_rec.br_no, br_no) || !strcmp (rec_br, "  ")))
	{
	       	if (strcmp (sadf_rec.year, "C"))
		{
			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			continue;
		}

		inmr_rec.hhbr_hash = sadf_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inmr_rec.item_no, "DELETED ITEM    ");
			sprintf (inmr_rec.description, "%-40.40s", "DELETED ITEM");
			sprintf (inmr_rec.category, "%-11.11s", "DELETED    ");
		}

		CalculateMTD ();
		CalculateYTD ();

		/*
		 * Store if sales & cost of sales <> 0.00	
		 */
		if (ValidItem () && (mtdQty [0] != 0 ||
		     mtdSales [0] != 0 ||
		     mtdCostSale [0] != 0 ||
		     ytdQty [0]   != 0 ||
		     ytdSales [0] != 0 ||
		     ytdCostSale [0] != 0))
				StoreProductData ();

		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}
}

/*
 * Store product based data in sort file 
 */
void
StoreProductData (void)
{
	cumr_rec.hhcu_hash	=	sadf_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");
		sprintf (cumr_rec.dbt_no, "%-6.6s", "DELETE");
		sprintf (cumr_rec.class_type, "%-3.3s", "DEL");
	}

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
		"%-2.2s%-11.11s%-16.16s%-6.6s",
		(BY_CO) 	? "  " : sadf_rec.br_no, 
		(BY_CAT) 	? inmr_rec.category : " ",
		inmr_rec.item_no, 
		cumr_rec.dbt_no
	);
	strcpy (sortRec [sortCnt].brNo, 	sadf_rec.br_no);
	strcpy (sortRec [sortCnt].catNo, 	inmr_rec.category);
	strcpy (sortRec [sortCnt].itemNo, 	inmr_rec.item_no);
	strcpy (sortRec [sortCnt].custNo, 	cumr_rec.dbt_no);
	strcpy (sortRec [sortCnt].custType,cumr_rec.class_type);
	strcpy (sortRec [sortCnt].smanNo, 	sadf_rec.sman);
	sortRec [sortCnt].mtdQty	=	mtdQty [0];
	sortRec [sortCnt].ytdQty	=	ytdQty [0];
	sortRec [sortCnt].mtdSale	=	mtdSales [0];
	sortRec [sortCnt].ytdSale	=	ytdSales [0];
	sortRec [sortCnt].mtdCost	=	mtdCostSale [0];
	sortRec [sortCnt].ytdCost	=	ytdCostSale [0];
	sortRec [sortCnt].hhbrHash	=	sadf_rec.hhbr_hash;
	sortRec [sortCnt].hhcuHash	=	sadf_rec.hhcu_hash;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

/*
 * Store customer based data in sort file 
 */
void
StoreCustomerData (void)
{
	char	brNo [3];
	char	sortStr [4];

	if (BY_CO)
		strcpy (brNo, "  ");
	else
		sprintf (brNo, "%-2.2s", (envDbCo) ?  cumr_rec.est_no : sadf_rec.br_no);

	if (REP_CUST)
		strcpy (sortStr, "   ");
	else if (REP_TYPE)
		sprintf (sortStr, "%-3.3s", cumr_rec.class_type);
	else if (REP_SMAN)
		sprintf (sortStr, "%-3.3s", sadf_rec.sman);

	inmr_rec.hhbr_hash = sadf_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inmr_rec.item_no, "DELETED ITEM    ");
		sprintf (inmr_rec.description, "%-40.40s", "DELETED ITEM");
		sprintf (inmr_rec.category, "%-11.11s", "DELETED    ");
	}
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
		"%-2.2s%-3.3s%-6.6s%-16.16s",
		brNo,
		sortStr,
		cumr_rec.dbt_no,
		inmr_rec.item_no
	);
	strcpy (sortRec [sortCnt].brNo, 	sadf_rec.br_no);
	strcpy (sortRec [sortCnt].catNo, 	inmr_rec.category);
	strcpy (sortRec [sortCnt].itemNo, 	inmr_rec.item_no);
	strcpy (sortRec [sortCnt].custNo, 	cumr_rec.dbt_no);
	strcpy (sortRec [sortCnt].custType,cumr_rec.class_type);
	strcpy (sortRec [sortCnt].smanNo, 	sadf_rec.sman);
	sortRec [sortCnt].mtdQty	=	mtdQty [0];
	sortRec [sortCnt].ytdQty	=	ytdQty [0];
	sortRec [sortCnt].mtdSale	=	mtdSales [0];
	sortRec [sortCnt].ytdSale	=	ytdSales [0];
	sortRec [sortCnt].mtdCost	=	mtdCostSale [0];
	sortRec [sortCnt].ytdCost	=	ytdCostSale [0];
	sortRec [sortCnt].hhbrHash	=	sadf_rec.hhbr_hash;
	sortRec [sortCnt].hhcuHash	=	sadf_rec.hhcu_hash;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

void
MainProcessData (void)
{
	int		print_type;
	int		i;

	InitArray ();
	printed = FALSE;
	first_time = TRUE;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		printed = TRUE;

		sprintf (currBrNo, "%-2.2s", sortRec [i].brNo);
		if (BY_CUST)
		{
			sprintf (currType, "%-3.3s",	sortRec [i].custType);
			sprintf (currCust, "%-6.6s", 	sortRec [i].custNo);
			sprintf (currItem, "%-16.16s", sortRec [i].itemNo);
			currHash [0] = sortRec [i].hhcuHash;
			currHash [1] = sortRec [i].hhbrHash;
		}
		else
		{
			sprintf (currCat, "%-11.11s",	sortRec [i].catNo);
			sprintf (currItem, "%-16.16s", sortRec [i].itemNo);
			sprintf (currCust, "%-6.6s", 	sortRec [i].custNo);
			currHash [0] = sortRec [i].hhcuHash;
			currHash [1] = sortRec [i].hhbrHash;
		}

		if (first_time)
		{
			strcpy (prevBrNo, currBrNo);
			strcpy (prevCat, currCat);
			strcpy (prevItem, currItem);
			strcpy (prevCust, currCust);
			strcpy (prevType, currType);
			prevHash [0] = currHash [0];
			prevHash [1] = currHash [1];
		}

		print_type = CheckBreak ();

		ProcessData (first_time, print_type);
		first_time = 0;

		if (print_type == 0)
		{
			mtdQty [0]   	+= sortRec [i].mtdQty;
			mtdSales [0] 	+= sortRec [i].mtdSale;
			mtdCostSale [0] += sortRec [i].mtdCost;
			ytdQty [0]   	+= sortRec [i].ytdQty;
			ytdSales [0] 	+= sortRec [i].ytdSale;
			ytdCostSale [0] += sortRec [i].ytdCost;
		}
		else
		{
			mtdQty [0]   	= sortRec [i].mtdQty;
			mtdSales [0] 	= sortRec [i].mtdSale;
			mtdCostSale [0] = sortRec [i].mtdCost;
			ytdQty [0]   	= sortRec [i].ytdQty;
			ytdSales [0] 	= sortRec [i].ytdSale;
			ytdCostSale [0] = sortRec [i].ytdCost;
		}

		strcpy (prevBrNo, currBrNo);
		strcpy (prevCat, currCat);
		strcpy (prevItem, currItem);
		strcpy (prevCust, currCust);
		strcpy (prevType, currType);
		prevHash [0] = currHash [0];
		prevHash [1] = currHash [1];
	}
	if (printed)
	{
		PrintLine ();
		if (BY_CUST)
		{
			if (DETAIL)
			{
				PrintTotal ("C");
				if (REP_SMAN || REP_TYPE)
					PrintTotal (cust_type);
			}
			else
			{
				if (REP_SMAN || REP_TYPE)
					PrintTotal (cust_type);
			}
		}
		else
		{
			if (DETAIL)
				PrintTotal ((BY_CAT) ? "A" : "I");
		}
		PrintTotal ("B");
		PrintTotal ("E");
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	Dsp_srch ();
	Dsp_close ();
}
int
CheckBreak (void)
{
	if (strcmp (currBrNo, prevBrNo))
		return (EXIT_FAILURE);

	if (BY_CUST)
	{
		if (REP_TYPE || REP_SMAN)
			if (strcmp (currType, prevType))
				return (DETAIL ? 2 : 3);

		if (currHash [0] != prevHash [0])
			return (DETAIL ? 3 : 4);
		if (DETAIL && currHash [1] != prevHash [1])
			return (4);
	}
	else
	{
		if (BY_CAT)
		{
			if (strcmp (currCat, prevCat))
				return (DETAIL ? 3 : 4);
			if (DETAIL && currHash [1] != prevHash [1])
				return (4);
		}
		else
		{
			if (currHash [1] != prevHash [1])
				return (DETAIL ? 3 : 4);
			if (DETAIL && currHash [0] != prevHash [0])
				return (4);
		}
	}
	return (EXIT_SUCCESS);
}

void
CustomerHeader (void)
{
	if (DETAIL)
	{
		/*
		 * CUSTOMER SALES BY ITEM REPORT 
		 */
		sprintf (err_str, " %s ", ML (mlSaMess006));
		rv_pr (err_str, 50, 0, 1);
	}
	else
	{
		/*
		 * CUSTOMER SALES REPORT 
		 */
		sprintf (err_str, " %s ", ML (mlSaMess007));
		rv_pr (err_str, 54, 0, 1);
	}

	if (!srt_all)
	{
		column = 45;
		if (!end_all)
			sprintf (err_str, " FROM CUSTOMER %-6.6s TO CUSTOMER %-6.6s ", startCustomer, endCustomer);
		else
			sprintf (err_str, " FROM CUSTOMER %-6.6s TO ALL CUSTOMERS ", startCustomer);
	}
	else
	{
		column = 56;
		strcpy (err_str, " FOR ALL CUSTOMERS ");
	}
}

void
TypeHeader (void)
{
	if (DETAIL)
	{
		sprintf (err_str, " %s ", ML (mlSaMess008)); 
		rv_pr (err_str, 44, 0, 1);
	}
	else
	{
		sprintf (err_str, " %s ", ML (mlSaMess009));
		rv_pr (err_str, 48, 0, 1);
	}

	if (!srt_all)
	{
		column = 47;
		if (!end_all)
			sprintf (err_str, " FROM CUST TYPE %-3.3s TO CUST TYPE %-3.3s ", startType, endType);
		else
			sprintf (err_str, " FROM CUST TYPE %-3.3s TO ALL CUST TYPES ", startType);
	}
	else
	{
		column = 54;
		strcpy (err_str, " FOR ALL CUSTOMER TYPES ");
	}
}

void
SalesmanHeader (void)
{
	if (DETAIL)
	{
		sprintf (err_str, " %s ", ML (mlSaMess010));
		rv_pr (err_str, 44, 0, 1);
	}
	else
	{
		sprintf (err_str, " %s ", ML (mlSaMess011));
		rv_pr (err_str, 48, 0, 1);
	}

	if (!srt_all)
	{
		column = 48;
		if (!end_all)
			sprintf (err_str, " FROM SALESMAN  %-2.2s  TO SALESMAN  %-2.2s ", startSalesman, endSalesman);
		else
			sprintf (err_str, " FROM SALESMAN  %-2.2s  TO ALL SALESMEN ", startSalesman);
	}
	else
	{
		column = 57;
		strcpy (err_str, " FOR ALL SALESMEN ");
	}
}

void
CategoryHeader (void)
{
	if (DETAIL)
	{
		sprintf (err_str, " %s ", ML (mlSaMess012));
		rv_pr (err_str, 50, 0, 1);
	}
	else
	{
		sprintf (err_str, " %s ", ML (mlSaMess013));
		rv_pr (err_str, 53, 0, 1);
	}

	if (!srt_all)
	{
		if (!end_all)
		{
			column = 40;
			sprintf (err_str, " FROM CATEGORY %-11.11s TO CATEGORY %-11.11s ", startCat, endCat);
		}
		else
		{
			column = 43;
			sprintf (err_str, " FROM CATEGORY %-11.11s TO ALL CATEGORIES ", startCat);
		}
	}
	else 
	{
		column = 56;
		strcpy (err_str, " FOR ALL CATEGORIES ");
	}
}

void
ItemHeader (void)
{
	if (DETAIL)
	{
		sprintf (err_str, (CATG_SEL) ?  ML (mlSaMess029) : ML (mlSaMess014)); 
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);
	}
	else
	{
		sprintf (err_str, (CATG_SEL) ? ML (mlSaMess015) : ML (mlSaMess030));
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);
	}

	if (!srt_all)
	{
		if (!end_all)
		{
			if (CATG_SEL)
			{
				column = 45;
				sprintf (err_str, 
					" FROM GROUP %-1.1s %-11.11s TO GROUP %-1.1s %-11.11s ", 
					startClass, startCat, 
					endClass, endCat);
			}
			else
			{
				column = 39;
				sprintf (err_str, 
					" FROM ITEM %16.16s TO ITEM %16.16s ", 
					lower, upper);
			}
		}
		else
		{
			column = 45;
			if (CATG_SEL)
			{
				sprintf (err_str, 
					" FROM GROUP %-1.1s %-11.11s TO LAST GROUP ", 
					startClass, startCat);
			}
			else
				sprintf (err_str, " FROM ITEM %16.16s TO ALL ITEMS ", lower);
		}
	}
	else 
	{
		column = 58;
		if (CATG_SEL)
			strcpy (err_str, " FOR ALL GROUPS ");
		else
			strcpy (err_str, " FOR ALL ITEMS ");
	}
}

int
CheckPage (void)
{
	return (EXIT_SUCCESS);
}

void
ProcessData (
 int    first_time, 
 int    print_type)
{
	if (first_time)
	{
		if (DETAIL)
		{
			if (BY_CUST)
				PrintHeader (first_time, cust_type, TRUE);
			else
			if (BY_CAT)
				PrintHeader (first_time, "A", FALSE);
			else
				PrintHeader (first_time, "I", FALSE);

			if (REP_TYPE || REP_SMAN)
				PrintHeader (first_time, "C", FALSE);
		}
		else
		{
			if (BY_CUST && (REP_TYPE || REP_SMAN))
				PrintHeader (first_time, cust_type, FALSE);
		}
	}

	if (!first_time && (print_type == 1 || print_type == 2 || print_type == 3))
	{
		PrintLine ();

		switch (print_type)
		{
		case	1:
			if (BY_CUST)
			{
				if (DETAIL)
					PrintTotal ("C");

				if (!REP_CUST)
					PrintTotal (cust_type);

				PrintTotal ("B");
				if (!REP_CUST)
				{
					PrintHeader (first_time, cust_type, TRUE);
					if (DETAIL)
						PrintHeader (first_time, "C", FALSE);
				}
			}
			else
			{
				if (BY_CAT && DETAIL)
					PrintTotal ("A");
				PrintTotal ("B");
				if (BY_CAT && DETAIL)
					PrintHeader (first_time, "A", FALSE);
			}
			break;

		case	2:
			PrintTotal ("C");
			PrintTotal (cust_type);
			PrintHeader (first_time, cust_type, TRUE);
			PrintHeader (first_time, "C", FALSE);
			break;

		case	3:
			if (BY_CUST)
			{
				if (DETAIL)
				{
					PrintTotal ("C");
					PrintHeader (first_time, "C", FALSE);
				}
				else
				{
					PrintTotal (cust_type);
					PrintHeader (first_time, cust_type, TRUE);
				}
			}
			else
			{
				PrintTotal ((BY_CAT) ? "A" : "I");
				if (BY_CAT)
					PrintHeader (first_time, "A", FALSE);
				else
					PrintHeader (first_time, "I", FALSE);
			}
			break;

		default	:
			break;
		}
	}

	if (print_type == 4)
		PrintLine ();
}
void
PrintLine (void)
{
	register	int	i;
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	char	var_item [17];
	char	var_desc [41];
	int	m_mar_exceed = FALSE;
	int	y_mar_exceed = FALSE;
	char	margin_exceed [8];
	char	mnth_margin [8];
	char	year_margin [8];

	/*
	 * If sales & cost of sales  = 0.00, don't print	
	 */
	if (mtdQty [0] == 0.00 && mtdSales [0] == 0.00 &&
		mtdCostSale [0] == 0.00 && ytdQty [0] == 0.00 &&
		ytdSales [0] == 0.00 && ytdCostSale [0] == 0.00)
			return;

	i = (BY_CAT || DETAIL)  ? 1 : 2;
	if (REP_SMAN || REP_TYPE || (BY_CAT && !DETAIL))
		i  = 1;

	cumr_rec.hhcu_hash = prevHash [0];
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		sprintf (cumr_rec.dbt_name, "%40.40s", " ");

	inmr_rec.hhbr_hash = prevHash [1];
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inmr_rec.item_no, "DELETED ITEM    ");
		sprintf (inmr_rec.description, "%-40.40s", "DELETED ITEM");
		sprintf (inmr_rec.category, "%-11.11s", "DELETED    ");
	}
	
	if (COST_MGN)
	{
		if (mtdSales [0] != 0.00)
		{
			/*
			 * Check if margin is out of range 
			 */
			m_margin = (mtdSales [0] - mtdCostSale [0]) / mtdSales [0] * 100.00;
			if (m_margin >= 100000)
			{
				strcpy (margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (m_margin <= -10000)
			{
				strcpy (margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}

		if (ytdSales [0] != 0.00)
		{
			y_margin = (ytdSales [0] - ytdCostSale [0]) / ytdSales [0] * 100.00;
			/*
			 * Check if margin is out of range 
			 */
			if (y_margin >= 100000)
			{
				strcpy (margin_exceed, "+******");
				y_mar_exceed = TRUE;
			}

			if (y_margin <= -10000)
			{
				strcpy (margin_exceed, "-******");
				y_mar_exceed = TRUE;
			}
		}
	}

	if (BY_CUST)
	{
		sprintf (var_item, "%-16.16s", (DETAIL) ? prevItem : prevCust);
		strcpy (var_desc, (DETAIL) ? inmr_rec.description : cumr_rec.dbt_name);
	}
	else
	{
		if (BY_CAT)
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			sprintf (excf_rec.cat_no, "%-11.11s", prevCat);
			cc = find_rec (excf, &excf_rec, COMPARISON, "r");
			if (cc)
				strcpy (excf_rec.cat_desc, "No Description Found");
			sprintf (var_item, "%-16.16s", (DETAIL) ? prevItem : prevCat);
			strcpy (var_desc, (DETAIL) ? inmr_rec.description : excf_rec.cat_desc);
		}
		else
		{
			sprintf (var_item, "%-16.16s", (DETAIL) ? prevCust : prevItem);
			strcpy (var_desc, (DETAIL) ? cumr_rec.dbt_name : inmr_rec.description);
		}
	}

	if (COST_MGN)
	{
		if (m_mar_exceed)
			sprintf (mnth_margin, "%-7.7s", margin_exceed);
		else
			sprintf (mnth_margin, "%7.1f", m_margin);

		if (y_mar_exceed)
			sprintf (year_margin, "%-7.7s", margin_exceed);
		else
			sprintf (year_margin, "%7.1f", y_margin);
	}

	if (COST_MGN)
	{
		sprintf (dspStr, "%-16.16s^E%-40.40s^E%8.0f^E%8.0f^E%8.0f^E%-7.7s^E%8.0f^E%8.0f^E%8.0f^E%-7.7s", 
			var_item, 
			var_desc, 
			mtdQty [0], 
			mtdSales [0], 
			mtdCostSale [0], 
			mnth_margin, 
			ytdQty [0], 
			ytdSales [0], 
			ytdCostSale [0], 
			year_margin);
	}
	else
	{
		sprintf (dspStr, "%-16.16s^E%-40.40s^E%8.0f^E%8.0f^E%8.0f^E%8.0f", 
			var_item, 
			var_desc, 
			mtdQty [0], 
			mtdSales [0], 
			ytdQty [0], 
			ytdSales [0]);
	}
	Dsp_saverec (dspStr);
	ln_num++;
	RESET_LINE_NUM;

	mtdQty [i] 		+= mtdQty [0];
	mtdSales [i] 	+= mtdSales [0];
	mtdCostSale [i] += mtdCostSale [0];
	ytdQty [i] 		+= ytdQty [0];
	ytdSales [i] 	+= ytdSales [0];
	ytdCostSale [i] += ytdCostSale [0];

	mtdQty [0] 		= 0.00;
	mtdSales [0] 	= 0.00;
	mtdCostSale [0] = 0.00;
	ytdQty [0] 		= 0.00;
	ytdSales [0] 	= 0.00;
	ytdCostSale [0] = 0.00;
}

void
DrawLine (void)
{
	if (COST_MGN)
		Dsp_saverec (SEPARATION);
	else
		Dsp_saverec (XSEPARATION);
	ln_num++;
	RESET_LINE_NUM;
}

void
PrintTotal (
 char*  tot_type)
{
	float	m_margin = 0.00;
	float	y_margin = 0.00;
	int	j = 0;
	int	m_mar_exceed = FALSE;
	int	y_mar_exceed = FALSE;
	char	margin_exceed [8];
	char	mnth_margin [8];
	char	year_margin [8];

	switch (tot_type [0])
	{
	case	'C':
		j = 1;
		sprintf (err_str, "%-s %-6.6s %-19.19s", "Total For Customer", prevCust, cumr_rec.dbt_acronym);
		break;

	case	'S':
		j = (DETAIL) ? 2 : 1;
		sprintf (err_str, "%-s %-2.2s %-23.23s", "Total For Salesman", currSman, exsf_rec.salesman);
		break;

	case	'T':
		j = (DETAIL) ? 2 : 1;
		sprintf (err_str, "%-s %-25.25s", "Total For Cust Type", prevType);
		break;

	case	'I':
		j = 1;
		sprintf (err_str, "%-s %-30.30s", "Total For Item", prevItem);
		break;

	case	'A':
		j = 1;
		sprintf (err_str, "%-s  %-26.26s", "Total For Category", prevCat);
		break;

	case	'B':
		j = (DETAIL && (REP_SMAN || REP_TYPE)) ? 3 : 2;
		if (BY_CAT && !DETAIL)
			j = 1;

		if (BY_CO)
		{
			mtdQty  	[j + 1] += mtdQty [j];
			mtdSales [j + 1] += mtdSales [j];
			mtdCostSale [j + 1] += mtdCostSale [j];
			ytdQty  	[j + 1] += ytdQty [j];
			ytdSales [j + 1] += ytdSales [j];
			ytdCostSale [j + 1] += ytdCostSale [j];
			return;
		}

		/*
		 * Get branch name 
		 */
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, prevBrNo);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			sys_err ("Error in esmr during (DBFIND)", cc, PNAME);

		sprintf (err_str, "%-s %-2.2s %-25.25s", "Total For Branch", prevBrNo, esmr_rec.short_name);
		break;

	case	'E':
		if (!BY_CO)
			return;

		j = (DETAIL && (REP_SMAN || REP_TYPE)) ? 4 : 3;
		if (BY_CAT && !DETAIL)
			j = 2;
		sprintf (err_str, "%-s", "Total For Company ");
		break;
	}

	if (COST_MGN)
	{
		if (mtdSales [j] != 0.00)
		{
			m_margin = (mtdSales [j] - mtdCostSale [j]) / mtdSales [j] * 100.00;
			/*
			 * Check if margin is out of range 
			 */
			if (m_margin >= 100000)
			{
				strcpy (margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (m_margin <= -10000)
			{
				strcpy (margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}

		if (ytdSales [j] != 0.00)
		{
			y_margin = (ytdSales [j] - ytdCostSale [j]) / ytdSales [j] * 100.00;
			/*
			 * Check if margin is out of range 
			 */
			if (y_margin >= 100000)
			{
				strcpy (margin_exceed, "+******");
				m_mar_exceed = TRUE;
			}

			if (y_margin <= -10000)
			{
				strcpy (margin_exceed, "-******");
				m_mar_exceed = TRUE;
			}
		}
	}

/*
	if (!DETAIL || (tot_type [0] != 'B' && tot_type [0] != 'E'))
		DrawLine ();
*/

	if (COST_MGN)
	{
		if (m_mar_exceed)
			sprintf (mnth_margin, "%-7.7s", margin_exceed);
		else
			sprintf (mnth_margin, "%7.1f", m_margin);

		if (y_mar_exceed)
			sprintf (year_margin, "%-7.7s", margin_exceed);
		else
			sprintf (year_margin, "%7.1f", y_margin);
	}

	if (COST_MGN)
	{
		sprintf (dspStr, "   %-45.45s         ^E%8.0f^E%8.0f^E%8.0f^E%-7.7s^E%8.0f^E%8.0f^E%8.0f^E%-7.7s", 
			err_str, 
			mtdQty [j], 
			mtdSales [j], 
			mtdCostSale [j], 
			mnth_margin, 
			ytdQty [j], 
			ytdSales [j], 
			ytdCostSale [j], 
			year_margin);
	}
	else
	{
		sprintf (dspStr, "   %-45.45s         ^E%8.0f^E%8.0f^E%8.0f^E%8.0f", 
			err_str, 
			mtdQty [j], 
			mtdSales [j], 
			ytdQty [j], 
			ytdSales [j]);
	}
	Dsp_saverec (dspStr);
	ln_num++;
	RESET_LINE_NUM;
	
	DrawLine ();

	if (tot_type [0] != 'E')
	{
		mtdQty  [j + 1] 	+= mtdQty [j];
		mtdSales [j + 1] 	+= mtdSales [j];
		mtdCostSale [j + 1] += mtdCostSale [j];
		ytdQty  [j + 1] 	+= ytdQty [j];
		ytdSales [j + 1] 	+= ytdSales [j];
		ytdCostSale [j + 1] += ytdCostSale [j];
	}

	mtdQty [j] 		= 0.00;
	mtdSales [j] 	= 0.00;
	mtdCostSale [j] = 0.00;
	ytdQty [j] 		= 0.00;
	ytdSales [j] 	= 0.00;
	ytdCostSale [j] = 0.00;
}

void
InitOutput (void)
{

	int		st_mth;
	char	temp_str [21];

	DateToDMY (comm_rec.dbt_date, NULL, &st_mth, NULL);

/*
	clear ();
*/
	Dsp_open (0, 3, PSIZE);

	if (BY_CUST)
	{
		if (REP_CUST)
			CustomerHeader ();

		if (REP_TYPE)
			TypeHeader ();

		if (REP_SMAN)
			SalesmanHeader ();
	}
	else
	{
		if (BY_CAT)
			CategoryHeader ();
		else
			ItemHeader (); 
	}
		
	if (!strcmp (br_no, "  "))
		print_at (1, 59, "%s\n", ML (mlSaMess032));
	else
	{
		sprintf (temp_str, ML (mlStdMess039), br_no, " ");
		print_at (1, 59, "%s\n", temp_str);
	}
	print_at (2, column, err_str);

	if (BY_CUST)
	{
		if (REP_CUST)
		{
			if (COST_MGN)
				Dsp_saverec (HEADER);
			else
				Dsp_saverec (XHEADER);
		}

		if (REP_TYPE)
		{
			if (COST_MGN)
				Dsp_saverec (THEADER);
			else
				Dsp_saverec (XTHEADER);
		}

		if (REP_SMAN)
		{
			if (COST_MGN)
				Dsp_saverec (SHEADER);
			else
				Dsp_saverec (XSHEADER);
		}

		if (DETAIL)
		{
			if (COST_MGN)
				Dsp_saverec (CUST_HEAD);
			else
				Dsp_saverec (XCUST_HEAD);
		}
		else
		{
			if (COST_MGN)
				Dsp_saverec (CUST_HEAD1);
			else
				Dsp_saverec (XCUST_HEAD1);
		}
	}
	else
	{
		if (BY_CAT)
		{
			if (COST_MGN)
				Dsp_saverec (AHEADER);
			else
				Dsp_saverec (XAHEADER);
		}
		else
		{
			if (COST_MGN)
				Dsp_saverec (HEADER);
			else
				Dsp_saverec (XHEADER);
		}

		if (DETAIL)
		{
			if (COST_MGN)
				Dsp_saverec (ITEM_HEAD);
			else
				Dsp_saverec (XITEM_HEAD);
		}
		else
		{
			if (COST_MGN)
				Dsp_saverec (ITEM_HEAD1);
			else
				Dsp_saverec (XITEM_HEAD1);
		}
	}

	Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");
}

void
InitArray (void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		mtdQty [j] = 0.00;
		ytdQty [j] = 0.00;
		mtdSales [j] = 0.00;
		mtdCostSale [j] = 0.00;
		ytdSales [j] = 0.00;
		ytdCostSale [j] = 0.00;
	}
}

void
PrintHeader (
 int    first_time, 
 char*  head_type, 
 int    prt_head)
{
	char	var_code [17];
	char	var_desc [41];
	char	var_item [9];

	switch (head_type [0])
	{
	case	'C':
		cumr_rec.hhcu_hash = currHash [0];
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (cumr_rec.dbt_name, "%-40.40s", "Unknown Customer");
			sprintf (cumr_rec.dbt_no, "%-6.6s", "DELETE");
			sprintf (cumr_rec.class_type, "%-3.3s", "DEL");
		}
		strcpy (var_code, cumr_rec.dbt_no);
		strcpy (var_desc, cumr_rec.dbt_name);
		strcpy (var_item, "CUSTOMER");
		break;

	case	'T':
		strcpy (excl_rec.co_no, comm_rec.co_no);
		sprintf (excl_rec.class_type, "%-3.3s", currType);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
			strcpy (excl_rec.class_desc, "No Description Found");

		strcpy (var_code, excl_rec.class_type);
		strcpy (var_desc, excl_rec.class_desc);
		strcpy (var_item, " CLASS  ");
		break;

	case	'S':
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%-2.2s", currType);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		if (cc)
			strcpy (exsf_rec.salesman, "No Salesman Found");
		strcpy (var_code, exsf_rec.salesman_no);
		strcpy (currSman, exsf_rec.salesman_no);
		strcpy (var_desc, exsf_rec.salesman);
		strcpy (var_item, "SALESMAN");
		break;

	case	'A':
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", currCat);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			strcpy (excf_rec.cat_desc, "No Description Found");
		strcpy (var_code, excf_rec.cat_no);
		strcpy (var_desc, excf_rec.cat_desc);
		strcpy (var_item, "CATEGORY");
		break;

	case	'I':
		inmr_rec.hhbr_hash = currHash [1];
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inmr_rec.item_no, "DELETED ITEM    ");
			sprintf (inmr_rec.description, "%-40.40s", "DELETED ITEM");
			sprintf (inmr_rec.category, "%-11.11s", "DELETED    ");
		}
			
		strcpy (var_code, inmr_rec.item_no);
		strcpy (var_desc, inmr_rec.description);
		strcpy (var_item, "ITEM    ");
		break;
	}

	if (COST_MGN)
	{
		sprintf (dspStr, " * %s  %-16.16s  %-40.40s                                                        ", 
			var_item, 
			var_code, 
			var_desc);
	}
	else
	{
		sprintf (dspStr, " * %s  %-16.16s  %-40.40s                ", 
			var_item, 
			var_code, 
			var_desc);
	}
	Dsp_saverec (dspStr);
	ln_num++;
	RESET_LINE_NUM;
}

int
CalculateMTD (void)
{
	mtdQty [0] 	= sadf_qty [curr_mnth - 1];
	mtdSales [0] = sadf_sal [curr_mnth - 1];
	mtdCostSale [0] = sadf_cst [curr_mnth - 1];
	
	return (EXIT_SUCCESS);
}

int
CalculateYTD (void)
{
	int i;

	ytdQty [0] 	= 0.00;
	ytdSales [0] = 0.00;
	ytdCostSale [0] = 0.00;

	if (curr_mnth <= fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			ytdQty [0] 	+= sadf_qty [i];
			ytdSales [0] += sadf_sal [i];
			ytdCostSale [0] += sadf_cst [i];
		}

		for (i = 0; i < curr_mnth; i++)
		{
			ytdQty [0] 	+= sadf_qty [i];
			ytdSales [0] += sadf_sal [i];
			ytdCostSale [0] += sadf_cst [i];
		}

	}
	else
	{
		for (i = fiscal; i < curr_mnth; i++)
		{
			ytdQty [0] 	+= sadf_qty [i];
			ytdSales [0] += sadf_sal [i];
			ytdCostSale [0] += sadf_cst [i];
		}
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
