/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_catcust_prt.c     )                           |
|  Program Desc  : ( Sales Analysis Report - Category by customer )   |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: catcust_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_catcust_prt/catcust_prt.c,v 5.3 2002/07/17 09:57:45 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <get_lpno.h>
#include <DateToString.h>
#include <pDate.h>

#define	SLEEP_TIME	2

	/*=====================+
	 | System Common File. |
	 +=====================*/
#define	COMM_NO_FIELDS	7

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	struct tag_commRecord
	{
		int		term;
		char	co_no [3];
		char	co_name [41];
		char	est_no [3];
		char	est_name [41];
		long	dbt_date;
		int		fiscal;
	}	comm_rec;

	/*==============================================+
	 | Sales Analysis Detail file By Item/Customer. |
	 +==============================================*/
#define	SADF_NO_FIELDS	41

	struct dbview	sadf_list [SADF_NO_FIELDS] =
	{
		{"sadf_co_no"},
		{"sadf_br_no"},
		{"sadf_year"},
		{"sadf_hhbr_hash"},
		{"sadf_hhcu_hash"},
		{"sadf_qty_per1"},
		{"sadf_qty_per2"},
		{"sadf_qty_per3"},
		{"sadf_qty_per4"},
		{"sadf_qty_per5"},
		{"sadf_qty_per6"},
		{"sadf_qty_per7"},
		{"sadf_qty_per8"},
		{"sadf_qty_per9"},
		{"sadf_qty_per10"},
		{"sadf_qty_per11"},
		{"sadf_qty_per12"},
		{"sadf_sal_per1"},
		{"sadf_sal_per2"},
		{"sadf_sal_per3"},
		{"sadf_sal_per4"},
		{"sadf_sal_per5"},
		{"sadf_sal_per6"},
		{"sadf_sal_per7"},
		{"sadf_sal_per8"},
		{"sadf_sal_per9"},
		{"sadf_sal_per10"},
		{"sadf_sal_per11"},
		{"sadf_sal_per12"},
		{"sadf_cst_per1"},
		{"sadf_cst_per2"},
		{"sadf_cst_per3"},
		{"sadf_cst_per4"},
		{"sadf_cst_per5"},
		{"sadf_cst_per6"},
		{"sadf_cst_per7"},
		{"sadf_cst_per8"},
		{"sadf_cst_per9"},
		{"sadf_cst_per10"},
		{"sadf_cst_per11"},
		{"sadf_cst_per12"},
	};

	struct tag_sadfRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	year [2];
		long	hhbr_hash;
		long	hhcu_hash;
		float	qty_per [12];
		double	sal_per [12];
		double	cst_per [12];
	}	sadf_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	5

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_category"},
	};

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		char	category [12];
	}	inmr_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	3

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
	};

	struct tag_cumrRecord
	{
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_name [41];
	}	cumr_rec;

	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	4

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
		{"esmr_short_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	est_name [41];
		char	short_name [16];
	}	esmr_rec;

	/*================================+
	 | External Category File Record. |
	 +================================*/
#define	EXCF_NO_FIELDS	3

	struct dbview	excf_list [EXCF_NO_FIELDS] =
	{
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	struct tag_excfRecord
	{
		char	co_no [3];
		char	cat_no [12];
		char	cat_desc [41];
	}	excf_rec;

	struct
	{
		char    brNo [3];
		char    brAll [4];
		char    brName [41];
		char    sCat [12];
		char    eCat [12];
		char    detSum [2];
		char    detSumDesc [9];
		char    costMgn [2];
		char    costMgnDesc [4];
		int     lpno;
		char    back[2];
		char    backDesc[4];
		char    onight[2];
		char    onightDesc[4];
		int     yendEnabled;
		int     yend;
		char    strYend [3];
		char    yendDesc [11];
		char    dummy [11];
	}	local_rec;                                                           

	struct salesRecord
	{
		float	mQty;
		double	mSales;
		double	mCost;
		float	mMargin;
		float	yQty;
		double	ySales;
		double	yCost;
		float	yMargin;
	};

static	struct	var	vars[]	=	
{
	{1, LIN, "brNo", 4, 23, CHARTYPE, 
		"UU", "          ", 
		" ", " ", " Branch Number      :", "Default is current branch. Enter [A] for All Branches.", 
		YES, NO, JUSTRIGHT, "", "", local_rec.brNo}, 
	{1, LIN, "brAll", 4, 24, CHARTYPE, 
		"UNN", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTRIGHT, "", "", local_rec.brAll}, 
	{1, LIN, "brName", 4, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.brName}, 
	{1, LIN, "detSum", 5, 23, CHARTYPE, 
		"U", "          ", 
		" ", "S", " Detailed / Summary :", "S(ummary) to report at Customer level, D(etail) at Item level.", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.detSum}, 
	{1, LIN, "detSumDesc", 5, 23, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", "Summary ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.detSumDesc}, 
	{1, LIN, "costMgn", 6, 23, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Print Cost Margin  :", "Y(es) to print Cost Margin, N(o) to exclude cost margin.", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.costMgn}, 
	{1, LIN, "costMgnDesc", 6, 23, CHARTYPE, 
		"AAA", "          ",
		" ", "Yes", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.costMgnDesc}, 
	{1, LIN, "sCat", 8, 23, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "           ", " Category From      :", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.sCat}, 
	{1, LIN, "eCat", 9, 23, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "~~~~~~~~~~~", " Category To        :", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.eCat}, 
	{1, LIN, "lpno", 11, 23, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer Number     :", "Enter valid printer number.  (Default is 1.)", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 12, 23, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Background         :", "Y(es) to run the program in background, N(o) to run in foreground", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 12, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "Yes", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 13, 23, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight          :", "Y(es) to schedule the program to run overnight, N(o) to run now", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{1, LIN, "onightDesc", 13, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "No ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc}, 
	{1, LIN, "yend", 15, 23, INTTYPE, 
		"NN", "          ", 
		" ", local_rec.strYend, " Year End Month     :", "Enter 1-12.", 
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.yend}, 
	{1, LIN, "yendDesc", 15, 30, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.yendDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*=======================
| Table names			|
=======================*/
static char 	*data = "data",
				*esmr = "esmr",
				*excf = "excf",
				*sadf = "sadf",
				*inmr = "inmr",
				*cumr = "cumr";

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void CloseDB (void);
int run_prog (void);
void Process (void);
void GetSalesData (FILE *sortFile, int currMonth, int fiscal, int byBranch);
void CalcYTD (int month, int fiscal, double *sales, float *qty, double *cost);
int GotData (int currMonth, float yQty, float ySales, float yCost);
void StringToStructure (struct salesRecord *destination, char *string);
void CalcMargin (int margin, struct salesRecord *data);
void PrintLine (FILE *fOut, int margin, int type, char *str1, char *str2, struct salesRecord *data);
void PrintSalesData (FILE *sortFile, int yend, int dtl, int mgn, int byBranch);
int ValidCommandLine (int argCount, char *argValue[]);
int heading (int scn);
int spec_valid (int field);
void SrchEsmr (char *key_val);
void SrchExcf (char *key_val);
int DefaultYend (void);
FILE *InitOutput (int detail, int margin, int fiscal, int lpno, int byBranch, char *sCat, char *eCat);
void EndPrintOut (FILE *pOutStream, int margin);
void AddTotals (struct salesRecord *Z, struct salesRecord *A, struct salesRecord *B, struct salesRecord *C, struct salesRecord *D);
void ZeroTotals (struct salesRecord *thisStruct);


int
main (
 int	argc,
 char*  argv[])
{
	int	yendHolder;	

	/*---------------------------
	| check command arguments	|
	---------------------------*/
	if (!ValidCommandLine (argc, argv))
	{
		printf ("\007Usage; %s\n", argv [0]);
        printf ("Usage; %s ", argv [0]);
        printf ("<brNo|A> <start cat> <end cat> ");
		printf ("<D|S> <Y|N> <lpno> <year end>\n");
		return (EXIT_FAILURE);
	}
	else
	{
		OpenDB ();
		read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);    

		SETUP_SCR (vars);

		local_rec.yend = DefaultYend ();
		sprintf (local_rec.strYend, "%2d", local_rec.yend);
		/*-------------------------------------------------------------------
		| Make another copy of yend as entry (1); will reset local_rec.yend |
		| but we still need to pass this default one to next program        |
		-------------------------------------------------------------------*/
		yendHolder = local_rec.yend;

		sprintf (local_rec.yendDesc, "%-10.10s", MonthName (local_rec.yend));
		
		if (argc == 8)
		{
			strcpy (local_rec.brNo, argv [1]);
			strcpy (local_rec.sCat, argv [2]);
			strcpy (local_rec.eCat, argv [3]);
			strcpy (local_rec.detSum, argv [4]);
			strcpy (local_rec.costMgn, argv  [5]);
			local_rec.lpno = atoi (argv [6]);
			local_rec.yend = atoi (argv [7]);
	   
			Process ();

			CloseDB (); 
			FinishProgram ();
			return (EXIT_SUCCESS);
		}

		init_scr ();
		set_tty ();
		set_masks ();
	   
		prog_exit   = FALSE;
	   
		while (!prog_exit)
		{
			restart     = FALSE;
			search_ok   = TRUE;
			init_ok     = TRUE;
			entry_exit  = FALSE;
			edit_exit   = FALSE;
			prog_exit   = FALSE;

			init_vars (1);
			/*------------------------------
			| Enter screen 1 linear input. |
			------------------------------*/
			heading (1);
			entry (1);

			if (prog_exit || restart)
				continue;

			edit (1);
			if (restart)
				continue;

			/*---------------------------------------------------------------
			| If SA_ENTER_YEND wasn't enabled local_rec.yend will be reset  |
			| so we use the default we copied at the start before entry (1);|
			---------------------------------------------------------------*/
			if (local_rec.yendEnabled == FALSE)
			{
				local_rec.yend = yendHolder;
			}

			if (run_prog () == 1)
			{
				return (EXIT_SUCCESS);
			}
			prog_exit = TRUE;
		}
	   
		CloseDB (); 
		FinishProgram ();
	}

	return (EXIT_SUCCESS);
}

/*===============================
| Open Database and record sets	|
===============================*/
void
OpenDB (void)
{
    abc_dbopen (data);
    
    open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
    open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_cat");
    open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no4");
}

/*===============================
| Close Database and record sets|
===============================*/
void
CloseDB (void)
{
    abc_fclose (cumr);
    abc_fclose (esmr);
    abc_fclose (excf);
    abc_fclose (inmr);
    abc_fclose (sadf);

    abc_dbclose (data);
}

/*===================================
| Check when and how to run report	|
===================================*/
int
run_prog (void)
{
    char    lpno [3];
    char    yend [3];
   
    sprintf (lpno, "%2d", local_rec.lpno);
    sprintf (yend, "%2d", local_rec.yend);

    /*--------------------------------
    | Test for Overnight Processing. |
    --------------------------------*/
    if (local_rec.onight [0] == 'Y')
    {
        if (fork() == 0)
		{
            execlp ("ONIGHT",
					"ONIGHT",
					"sa_catcust_prt",
					local_rec.brNo,
					local_rec.sCat,
					local_rec.eCat,
					local_rec.detSum,
					local_rec.costMgn,
					lpno, yend,
					"Sales Analysis By Category By Customer",
					NULL);
		}
    }
    /*------------------------------------
    | Test for forground or background . |
    ------------------------------------*/
    else if (local_rec.back [0] == 'Y')
    {
        if (fork() == 0)
		{
            execlp ("sa_catcust_prt",
					"sa_catcust_prt",
					local_rec.brNo,
					local_rec.sCat,
					local_rec.eCat,
					local_rec.detSum,
					local_rec.costMgn,
					lpno,
					yend,
					NULL);
		}
    }
    else
        Process ();

	return (EXIT_SUCCESS);
}

/*===========================
| Process the sales report	|
===========================*/
void
Process (void)
{
	int		currMonth;
	int		detail = FALSE;
	int		margin = FALSE;
	int		byBranch = TRUE;
	FILE	*fsort;

	DateToDMY (comm_rec.dbt_date, NULL, &currMonth, NULL);

	if (local_rec.detSum [0] == 'D')
		detail = TRUE;
	if (local_rec.costMgn [0] == 'Y')
		margin = TRUE;
	if (!strcmp (local_rec.brNo, " A"))
		byBranch = FALSE;


    if (local_rec.back [0] == 'N' && local_rec.onight [0] == 'N')
	{
		sprintf (err_str,
				 "Processing : Category by Customer Sales %s",
				 (detail) ? "(Detailed)" : "(Summary)");
		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}
	fsort = sort_open ("sale");
	GetSalesData (fsort, currMonth, local_rec.yend, byBranch);
	
    if (local_rec.back [0] == 'N' && local_rec.onight [0] == 'N')
	{
		sprintf (err_str,
				 "Sorting : Category by Customer Sales %s",
				 (detail) ? "(Detailed)" : "(Summary)");
		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}
	fsort = sort_sort (fsort, "sale");
	PrintSalesData (fsort, local_rec.yend, detail, margin, byBranch);

	sort_delete (fsort, "sale");
}

/*===========================================================
| Get the relevent sales data from data base into sort file	|
===========================================================*/
void
GetSalesData (
 FILE	*sortFile,
 int	currMonth,
 int	fiscal,
 int	byBranch)
{
	char	dataStr [200];

	float	mMargin;
	float	yQty;
	float	yMargin;
	double	ySales;
	double	yCost;

	float	*qty;
	double	*sales;
	double	*cost;

	qty = &yQty;
	sales = &ySales;
	cost = &yCost;

	/*---------------------------------------------------------------
	| Outer loop for every item in category range for this company	|
	---------------------------------------------------------------*/
	memset (&inmr_rec, 0, sizeof (inmr_rec));
	sprintf (inmr_rec.co_no, "%-2.2s", comm_rec.co_no);
	sprintf (inmr_rec.category, "%-11.11s", local_rec.sCat);

	for (cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
		 !cc &&
		 !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		 strcmp (inmr_rec.category, local_rec.eCat) <= 0;
		 cc = find_rec (inmr, &inmr_rec, NEXT, "r"))
	{
		/*-----------------------------------------------------------
		| Middle loop for every sales analysis detail for this item	|
		-----------------------------------------------------------*/
		if (local_rec.back[0] == 'N' && local_rec.onight[0] == 'N')
			dsp_process ("Item No : ", inmr_rec.item_no);

		memset (&sadf_rec, 0, sizeof (sadf_rec));

		sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
		sadf_rec.hhcu_hash = 0L;

		for (cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
			 !cc &&
			  sadf_rec.hhbr_hash == inmr_rec.hhbr_hash;
			 cc = find_rec (sadf, &sadf_rec, NEXT, "r"))
		{
			if (strcmp (sadf_rec.year, "C") ||
			    strcmp (sadf_rec.co_no, comm_rec.co_no) ||
				((byBranch) ? (strcmp (sadf_rec.br_no, local_rec.brNo))
							  : FALSE))
			   continue;

			mMargin = 0.00;
			yQty = 0.00;
			ySales = 0.00;
			yCost = 0.00;
			yMargin = 0.00;
			
			CalcYTD (currMonth, fiscal, sales, qty, cost);

			if (sadf_rec.sal_per [currMonth - 1] != 0.00)
			{
				mMargin = (sadf_rec.sal_per [currMonth - 1]
							- sadf_rec.cst_per [currMonth - 1])
							/ sadf_rec.sal_per [currMonth - 1] * 100.00;
			}

			if (ySales != 0.00)
			{
				yMargin = (ySales - yCost) / ySales * 100.00;
			}

			/*-----------------------------------------------
			| Only write to sort file if there are sales	|
			-----------------------------------------------*/
			if (GotData (currMonth, yQty, ySales, yCost))
			{
				/*-----------------------------------
				| Find the Customer name and number	|
				-----------------------------------*/
				cc = find_hash (cumr, &cumr_rec, EQUAL, "r",sadf_rec.hhcu_hash);
			
				/*-------------------------------
				| This is the sort order stuff	|
				-------------------------------*/
				sprintf (dataStr,
						 "%11.11s%6.6s%16.16s",
						 inmr_rec.category,
						 cumr_rec.dbt_no,
						 inmr_rec.item_no);
				sort_save (sortFile, dataStr);

				/*-------------------------------
				| This is the data stuff		|
				-------------------------------*/
				sprintf (dataStr,
				 "%-40.40s%-40.40s",
						 cumr_rec.dbt_name,
						 inmr_rec.description);
				sort_save (sortFile, dataStr);

				sprintf (dataStr,
				 "%12.2f%12.2f%12.2f%12.2f%12.2f%12.2f%12.2f%12.2f\n",
						 sadf_rec.qty_per [currMonth - 1],
						 sadf_rec.sal_per [currMonth - 1],
						 sadf_rec.cst_per [currMonth - 1],
						 mMargin,
						 yQty,
						 ySales,
						 yCost,
						 yMargin);
				sort_save (sortFile, dataStr);
			}
		}
	}
}

/*=======================================
| Calculate the year to date figures	|
=======================================*/
void
CalcYTD (
 int	month,
 int	fiscal,
 double	*sales,
 float	*qty,
 double	*cost)
{
	int	i;

	if (month <= fiscal)                                           
	{
		for (i = fiscal; i < 12; i++)
		{
			*(qty) += sadf_rec.qty_per [i];
			*(sales) += sadf_rec.sal_per [i];
			*(cost) += sadf_rec.cst_per [i];
		}

		for (i = 0; i < month; i++)
		{
			*(qty) += sadf_rec.qty_per [i];
			*(sales) += sadf_rec.sal_per [i];
			*(cost) += sadf_rec.cst_per [i];
		}
	}  
	else
	{  
		for (i = fiscal; i < month; i++)
		{
			*(qty) += sadf_rec.qty_per [i];
			*(sales) += sadf_rec.sal_per [i];
			*(cost) += sadf_rec.cst_per [i];
		}
	}
    *sales = twodec (*(sales));
    *cost = twodec (*(cost));
}

/*===========================
| Checks to see if any data |                                
===========================*/                                      
int                                                          
GotData (                  
 int	currMonth,
 float	yQty,
 float	ySales,
 float	yCost)                                                   
{   
    int gotData = FALSE;                                 
                
	if (sadf_rec.qty_per [currMonth - 1] ||
		sadf_rec.sal_per [currMonth - 1] ||
		sadf_rec.cst_per [currMonth - 1] ||
		yQty ||
		ySales ||
		yCost)
	{
            gotData = TRUE;               
    }            
    return gotData;
}


/*===============================================================
| Get the string form the sort file and jam it into a structure	|
===============================================================*/
void
StringToStructure (
 struct salesRecord *destination,
 char	*string)
{
	int		p;
	char	temp [20];

	p = 113;

	sprintf (temp, "%12.12s", string + p);
	destination->mQty = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->mSales = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->mCost = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->mMargin = atof (temp);
	p += 12;

	sprintf (temp, "%12.12s", string + p);
	destination->yQty = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->ySales = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->yCost = atof (temp);
	p += 12;
	sprintf (temp, "%12.12s", string + p);
	destination->yMargin = atof (temp);
}

/*===============================================================
| Get the string form the sort file and jam it into a structure	|
===============================================================*/
void
CalcMargin (
 int	margin,
 struct salesRecord *data)
{
	data->mMargin = 0.00;
	data->yMargin = 0.00;

	if (margin)
	{
		if (data->mSales)
		{
			data->mMargin = ((data->mSales - data->mCost) / data->mSales) * 100;
		}

		if (data->ySales)
		{
			data->yMargin = ((data->ySales - data->yCost) / data->ySales) * 100;
		}
	}
}

/*===============================================================
| Get the string form the sort file and jam it into a structure	|
===============================================================*/
void
PrintLine (
 FILE	*fOut,
 int	margin,
 int    type,
 char	*str1,
 char	*str2,
 struct salesRecord *data)
{
	switch (type)
	{
	case 1:
		fprintf (fOut, "| %-16.16s| %-39.39s", str1, str2);
		fprintf (fOut, "|        |        |        |        |        ");
		if (margin)
		{
			fprintf (fOut, "|        |        |        |\n");
		}
		else
		{
			fprintf (fOut, "|        |\n");
		}
		break;

	case 2:
		fprintf (fOut, "| %-16.16s", str1);
		fprintf (fOut, "| %-39.39s", str2);
		fprintf (fOut, "|%7.0f ", data->mQty);
		fprintf (fOut, "|%7.0f ", data->mSales);
		fprintf (fOut, "|%7.0f ", data->mCost);
		if (margin)
			fprintf (fOut, "|%7.2f ", data->mMargin);
		fprintf (fOut, "|%7.0f ", data->yQty);
		fprintf (fOut, "|%7.0f ", data->ySales);
		fprintf (fOut, "|%7.0f ", data->yCost);
		if (margin)
			fprintf (fOut, "|%7.2f ", data->yMargin);
		fprintf (fOut, "|\n");
		break;

	case 3:
		fprintf (fOut, "| %-20.20s %-36.36s", str1, str2);
		fprintf (fOut, "|%7.0f ", data->mQty);
		fprintf (fOut, "|%7.0f ", data->mSales);
		fprintf (fOut, "|%7.0f ", data->mCost);
		if (margin)
			fprintf (fOut, "|%7.2f ", data->mMargin);
		fprintf (fOut, "|%7.0f ", data->yQty);
		fprintf (fOut, "|%7.0f ", data->ySales);
		fprintf (fOut, "|%7.0f ", data->yCost);
		if (margin)
			fprintf (fOut, "|%7.2f ", data->yMargin);
		fprintf (fOut, "|\n");
		break;
	}
}

/*===================
| Print the data	|
===================*/
void
PrintSalesData (
 FILE*  sortFile,
 int	yend,
 int	dtl,
 int	mgn,
 int	byBranch)
{
	FILE	*fOut;
	int		firstTime = TRUE;
	char	*sptr;
	char	pItem [17];
	char	cItem [17];
	char	pCat [12];
	char	cCat [12];
	char	pCust [7];
	char	cCust [7];
	char	cCustDesc [41];
	char	pItemDesc [41];

	struct salesRecord item;
	struct salesRecord category;
	struct salesRecord customer;
	struct salesRecord branch;
	struct salesRecord temp;

	struct salesRecord *ptrItem;
	struct salesRecord *ptrCat;	
	struct salesRecord *ptrCust;
	struct salesRecord *ptrBr;
	struct salesRecord *ptrTemp;

	ptrItem = &item;
	ptrCat = &category;
	ptrCust = &customer;
	ptrBr = &branch;
	ptrTemp = &temp;

	ZeroTotals (ptrItem);
	ZeroTotals (ptrCat);
	ZeroTotals (ptrCust);
	ZeroTotals (ptrBr);

	fOut = InitOutput (dtl,
					   mgn,
					   yend,
					   local_rec.lpno,
					   byBranch,
					   local_rec.sCat,
					   local_rec.eCat);

	/*-------------------------------
	| Spin through sorted sort file	|
	-------------------------------*/
	for (sptr = sort_read (sortFile);
		 sptr;
		 sptr = sort_read (sortFile))
	{
		/*-----------------------------------------------
		| Read string into temp structure of variables	|
		-----------------------------------------------*/
		ZeroTotals (ptrTemp);
		StringToStructure (ptrTemp , sptr);

		if (firstTime)
		{
			sprintf (pCat, "%11.11s", sptr);
			sprintf (cCat, "%11.11s", sptr);
			sprintf (pCust, "%6.6s", sptr + 11);
			sprintf (cCust, "%6.6s", sptr + 11);
			sprintf (cCustDesc, "%40.40s", sptr + 33);
			sprintf (pItem, "%16.16s", sptr + 17);
			sprintf (cItem, "%16.16s", sptr + 17);
			
			if (dtl)
			{
				PrintLine (fOut, mgn, 1, cCust, cCustDesc, ptrTemp); 
			}
			firstTime = FALSE;
		}

		sprintf (cCat, "%11.11s", sptr);
		sprintf (cCust, "%6.6s", sptr + 11);
		sprintf (cItem, "%16.16s", sptr + 17);

		if (strcmp (cCust, pCust) ||
			strcmp (cCat, pCat) ||
			strcmp (cItem, pItem ))
		{
            if (dtl)
            {
				PrintLine (fOut, mgn, 2, pItem, pItemDesc, ptrItem); 
            }
            ZeroTotals (ptrItem);

            if (strcmp (cCust, pCust) || (strcmp (cCat, pCat)))
            {
                if (dtl)
                {
					PrintLine (fOut, mgn, 3, "Total For Customer", "", ptrCust);
                }
                else
                {
					PrintLine (fOut, mgn, 2, pCust, cCustDesc, ptrCust); 
                }
                ZeroTotals (ptrItem);
                ZeroTotals (ptrCust);
            }

			if (strcmp (cCat, pCat))
			{
				PrintLine (fOut, mgn, 3, "Total For Category", pCat, ptrCat); 

				ZeroTotals (ptrItem);
				ZeroTotals (ptrCust);
				ZeroTotals (ptrCat);
			}
			
            if (strcmp (cCust, pCust) && dtl)
            {
				sprintf (cCustDesc, "%40.40s", sptr + 33);
				PrintLine (fOut, mgn, 1, cCust, cCustDesc, ptrTemp); 
            }
		}

		AddTotals (ptrTemp, ptrItem, ptrCat, ptrCust, ptrBr);
		CalcMargin (mgn, ptrItem);
		CalcMargin (mgn, ptrCust);
		CalcMargin (mgn, ptrCat);

		sprintf (pCust, "%6.6s", sptr + 11);
		sprintf (pCat, "%11.11s", sptr);
        sprintf (pItem, "%16.16s", sptr + 17);
        sprintf (pItemDesc, "%-40.40s", sptr + 73);
		sprintf (cCustDesc, "%40.40s", sptr + 33);
	}

	if (dtl)
	{
		PrintLine (fOut, mgn, 3, "Total For Customer", "", ptrCust); 
	}
	else
	{
		PrintLine (fOut, mgn, 2, pCust, cCustDesc, ptrCust); 
	}

	PrintLine (fOut, mgn, 3, "Total For Category", pCat, ptrCat); 

	CalcMargin (mgn, ptrBr);
	if (byBranch)
	{
		PrintLine (fOut, mgn, 3, "Total For Branch", "", ptrBr); 
	}
	else
	{
		PrintLine (fOut, mgn, 3, "Total For Company", "", ptrBr); 
	}
	
	EndPrintOut (fOut, mgn);
}

/*===================================
| Check command line args are OK	|
===================================*/
int
ValidCommandLine (
 int	argCount,
 char*  argValue[])
{
	int OK = FALSE;

	if (argCount == 1 || argCount == 8)
	{
		OK = TRUE;
	}

	return (OK);
}

int
heading (
 int    scn)
{
    if (restart)
        return (EXIT_FAILURE);
   
    if (scn != cur_screen)
        scn_set (scn);
   
    clear ();
   
    sprintf (err_str,   " Sales Analysis by Category by Customer ");
    rv_pr (err_str, (80 - strlen (clip (err_str))) / 2, 0, 1);

    move (0,1);
    line (80);

    if (local_rec.yendEnabled)
    {
        box (0, 3, 80, 12);
        move (1, 14);
        line (79);
    }
    else
    {
        box (0, 3, 80, 10);
    }

    move (1, 7);
    line (80);

    move (1, 10);
    line (80);

    move (0, 20);
    line (80);

    print_at (21, 1, "Co : %s - %s", comm_rec.co_no, comm_rec.co_name);
    move (0,22);
    line (80);

    line_cnt = 0;
    scn_write (scn);

    return (EXIT_SUCCESS);
}

int
spec_valid (
 int    field)
{
    if (LCHECK ("brNo"))
    {
        if (dflt_used)
        {
            strcpy (local_rec.brNo, comm_rec.est_no);
            strcpy (local_rec.brName, comm_rec.est_name);
        }
   
        if (!strcmp (local_rec.brNo, " A"))
        {
            strcpy (local_rec.brNo, " A");
            strcpy (local_rec.brAll, "All");
            strcpy (local_rec.brName, "All Branches");
            FLD ("brAll") = NA;
            DSP_FLD ("brAll");
            DSP_FLD ("brName");
            return (EXIT_SUCCESS);
        }
        else
        {
            strcpy (local_rec.brAll, "   ");
            DSP_FLD ("brAll");
            FLD ("brAll") = ND;
        }

        if (SRCH_KEY)
        {
            SrchEsmr (temp_str);
            return (EXIT_SUCCESS);
        }

        if (strcmp (local_rec.brNo, " A") == 0)
        {
            sprintf (local_rec.brName, "All Branches%28.28s", " ");
            DSP_FLD ("brName");
            return (EXIT_SUCCESS);
        }

        strcpy (esmr_rec.co_no, comm_rec.co_no);
        strcpy (esmr_rec.est_no, local_rec.brNo);
        cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
        if (cc)
        {
            print_mess ("Branch Not On File");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

        strcpy (local_rec.brName, esmr_rec.est_name);
        DSP_FLD ("brName");
        return (EXIT_SUCCESS);
    }

    if (LCHECK ("sCat"))
    {
        if (dflt_used)
        {
            strcpy (local_rec.sCat, "           ");
            return (EXIT_SUCCESS);
        }

        if (SRCH_KEY)
        {
            SrchExcf (temp_str);
            return (EXIT_SUCCESS);
        }

        strcpy (excf_rec.co_no, comm_rec.co_no);
        strcpy (excf_rec.cat_no, local_rec.sCat);
        cc = find_rec (excf, &excf_rec, EQUAL, "r");
        if (cc)
        {
            print_mess ("Category Not On File");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

        if (prog_status != ENTRY && strcmp (local_rec.sCat, local_rec.eCat) > 0)
        {
            print_mess ("Start Selection Must Be Less Than End Selection\007");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

        return (EXIT_SUCCESS);
    }

    if (LCHECK ("eCat"))
    {
        if (dflt_used)
        {
            strcpy (local_rec.eCat, "~~~~~~~~~~~");
            return (EXIT_SUCCESS);
        }

        if (SRCH_KEY)
        {
            SrchExcf (temp_str);
            return (EXIT_SUCCESS);
        }

        strcpy (excf_rec.co_no, comm_rec.co_no);
        strcpy (excf_rec.cat_no, local_rec.eCat);
        cc = find_rec (excf, &excf_rec, EQUAL, "r");
        if (cc)
        {
            print_mess ("Category Not On File");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
   
        if (strcmp (local_rec.eCat, local_rec.sCat) < 0)
        {
            print_mess ("End Selection Must Be Greater Than Start Selection\007");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }
   
        return (EXIT_SUCCESS);
    }
   
    if (LCHECK ("detSum"))
    {
        if (dflt_used)
            sprintf (local_rec.detSum, "S");
   
        if (local_rec.detSum [0] == 'D')
            sprintf (local_rec.detSumDesc, "Detailed");
        else
            sprintf (local_rec.detSumDesc, "Summary ");
   
        DSP_FLD ("detSumDesc");
        return (EXIT_SUCCESS);
    }
   
    if (LCHECK ("costMgn"))
    {
        if (dflt_used)
            sprintf (local_rec.costMgn, "Y");
   
        switch (local_rec.costMgn[0])
        {
            case 'Y':
                sprintf (local_rec.costMgnDesc, "Yes");
                break;
            case 'N':
                sprintf (local_rec.costMgnDesc, "No ");
                break;
        }
   
        DSP_FLD ("costMgnDesc");
        return (EXIT_SUCCESS);
    }

    /*---------------------------------------------------
    | Validate Field Selection Year End Month option.   |
    ---------------------------------------------------*/
    if (LCHECK ("yend"))
    {
        if (F_NOKEY (label ("yend")))
        {
            return (EXIT_SUCCESS);
        }

        if (SRCH_KEY)
        {
            return (EXIT_SUCCESS);
        }

        if (local_rec.yend < 1 || local_rec.yend > 12)
        {
            print_mess ("\007 Invalid Month Year End. ");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

        sprintf (local_rec.yendDesc, "%-10.10s", MonthName (local_rec.yend));

        DSP_FLD ("yendDesc");
        return (EXIT_SUCCESS);
	}

    if (LCHECK ("lpno"))
    {
        if (SRCH_KEY)
        {
            local_rec.lpno = get_lpno (0);
            return (EXIT_SUCCESS);
        }
   
        if (!valid_lp (local_rec.lpno))
        {
            print_mess ("\007 Invalid Printer NUmber ");
            sleep (sleepTime);
            clear_mess ();
            return (EXIT_FAILURE);
        }

        return (EXIT_SUCCESS);
    }

    if (LCHECK ("back"))
    {
        if (dflt_used)
            sprintf (local_rec.back, "N");

        switch (local_rec.back[0])
        {
            case 'Y':
                sprintf (local_rec.backDesc, "Yes");
                break;
            case 'N':
                sprintf (local_rec.backDesc, "No ");
                break;
        }

        DSP_FLD ("backDesc");
        return (EXIT_SUCCESS);
    }

    if (LCHECK ("onight"))
    {
        if (dflt_used)
            sprintf (local_rec.onight, "N");

        switch (local_rec.onight[0])
        {
            case 'Y':
                sprintf (local_rec.onightDesc, "Yes");
                break;
            case 'N':
                sprintf (local_rec.onightDesc, "No ");
                break;
        }

        DSP_FLD ("onightDesc");
        return (EXIT_SUCCESS);
    }
    return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char*  key_val)
{
    work_open ();
    save_rec ("#Br", "#Short Name  ");

    strcpy (esmr_rec.co_no, comm_rec.co_no);             
    sprintf (esmr_rec.est_no, "%-2.2s", key_val);
    for (cc = find_rec (esmr, &esmr_rec, GTEQ, "r");                            
        !cc &&                  
           !strcmp  (esmr_rec.co_no, comm_rec.co_no) &&
           !strncmp (esmr_rec.est_no, key_val, strlen (key_val));
        cc = find_rec (esmr, &esmr_rec, NEXT, "r"))
    {   
        cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
        if (cc)              
            break;
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
SrchExcf (
 char*  key_val)
{
    work_open ();
    save_rec ("#Category", "#Description ");                               
        
    strcpy (excf_rec.co_no, comm_rec.co_no);                            
    sprintf (excf_rec.cat_no, "%-11.11s", key_val);
    cc = find_rec (excf, &excf_rec, GTEQ, "r");
    while (!cc &&                                                  
           !strcmp  (excf_rec.co_no, comm_rec.co_no) &&                     
           !strncmp (excf_rec.cat_no, key_val, strlen (key_val)))
    {   
        cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
        if (cc)
            break;                                      
   
        cc = find_rec (excf, &excf_rec, NEXT, "r");
    }
   
    cc = disp_srch ();
    work_close ();
    if (cc)
        return;
   
    strcpy (excf_rec.co_no, comm_rec.co_no);
    sprintf (excf_rec.cat_no, "%-11.11s", temp_str);
    cc = find_rec (excf, &excf_rec, EQUAL, "r");
    if (cc)
        file_err (cc, excf, "DBFIND");
}

/*===========================
| Default Month Year End    |
===========================*/
int
DefaultYend (void)
{ 
    int     mnthYEnd;
    int     enterYend;
    char    *sptr;
  
    mnthYEnd = comm_rec.fiscal;
    local_rec.yendEnabled = FALSE;
        
    sptr = chk_env ("SA_YEND");
    if (sptr)
    {
         mnthYEnd = atoi (sptr);
    }
  
    sptr = chk_env ("SA_ENTER_YEND");
    if (sptr)
    {
        enterYend = atoi (sptr);
        if (enterYend == 1)
        {
            local_rec.yendEnabled = TRUE;

			FLD ("yend") = YES;
            FLD ("yendDesc") = NA;
        }
    }
  
    return (mnthYEnd);
} 

/*===========================
| Initialise Output screen	|
===========================*/
FILE*
InitOutput (
 int	detail,
 int	margin,
 int	fiscal,
 int	lpno,
 int	byBranch,
 char*  sCat,
 char*  eCat)
{
    FILE    *pOutStream = popen ("pformat", "w");
    if (!pOutStream)
        file_err (errno, PNAME, "popen()");

    /*--------------------------------
    |  Format output to Page header. |
    --------------------------------*/
    fprintf (pOutStream, ".START%s <%s>\n", DateToString (TodaysDate ()), PNAME);
    fprintf (pOutStream, ".LP%d\n", lpno);
   
    /* next 15 lines describes heading */
    fprintf (pOutStream, ".15\n");
    fprintf (pOutStream, ".PI12\n");

    fprintf (pOutStream, ".PL60\n");
    fprintf (pOutStream, ".L132\n");

    fprintf (pOutStream, ".B1\n");
	fprintf (pOutStream,
			 ".ECATEGORY BY CUSTOMER%sREPORT - YEAR END MONTH %s\n",
			 detail ? " DETAIL " : " ",
			 MonthName (fiscal));                                    
   
	if (byBranch)
	{
		fprintf (pOutStream, ".EFor Branch %s\n", clip (comm_rec.est_name));
	}
	else
	{
		fprintf (pOutStream, ".EFor Company %s\n", clip (comm_rec.co_name));
	}

    fprintf (pOutStream, ".C(lines rounded - totals accurate)\n");
    fprintf (pOutStream, ".ESELECTION RANGE : %s TO %s\n", sCat, eCat);
    fprintf (pOutStream, ".B1\n");

	if (margin)
	{
		fprintf (pOutStream, ".LINE132-\n");
	}
	else
	{
		fprintf (pOutStream, ".LINE114-\n");
	}

	fprintf (pOutStream, "|                 ");
	fprintf (pOutStream, "|                                        ");
	if (margin)
	{
		fprintf (pOutStream, "| <---------- MTD SALES ----------> ");
		fprintf (pOutStream, "| <---------- YTD SALES ----------> |\n");
	}
	else
	{
		fprintf (pOutStream, "| <----- MTD SALES ------> ");
		fprintf (pOutStream, "| <----- YTD SALES ------> |\n");
	}

	if (detail)
	{
		fprintf (pOutStream, "| CUSTOMER / ITEM ");
		fprintf (pOutStream, "|        CUSTOMER NAME / PRODUCT         ");
	}
	else
	{
		fprintf (pOutStream, "|   CUSTOMER      ");
		fprintf (pOutStream, "|            CUSTOMER NAME               ");
	}

	fprintf (pOutStream, "|  QTY   | SALES  |  COST  ");
	if (margin)
		fprintf (pOutStream, "|%%MARGIN ");
	fprintf (pOutStream, "|  QTY   | SALES  |  COST  ");
	if (margin)
		fprintf (pOutStream, "|%%MARGIN ");
	fprintf (pOutStream, "|\n");

	fprintf (pOutStream, "|-----------------");
	fprintf (pOutStream, "|----------------------------------------|--------");
	fprintf (pOutStream, "|--------|--------|--------|--------|--------");
	if (margin)
		fprintf (pOutStream, "|--------|--------");
	fprintf (pOutStream, "|\n");

	fprintf (pOutStream, ".R-----------------");
	fprintf (pOutStream, "---------------------------------------------------");
	fprintf (pOutStream, "----------------------------------------------");
	if (margin)
		fprintf (pOutStream, "------------------");
	fprintf (pOutStream, "\n");
  
	return (pOutStream);
}

/*=======================
| Close print stream    |
=======================*/
void
EndPrintOut (
 FILE*  pOutStream,
 int    margin)
{ 
	if (margin)
	{
		fprintf (pOutStream, ".LINE132-\n");
	}
	else
	{
		fprintf (pOutStream, ".LINE114-\n");
	}

    fprintf (pOutStream, ".EOF\n");
    pclose (pOutStream);
}  

/*===============================
| Add the sales record totals	|
===============================*/
void
AddTotals (
 struct salesRecord*    Z,
 struct salesRecord*    A,
 struct salesRecord*    B,
 struct salesRecord*    C,
 struct salesRecord*    D)
{
	/*-------------------------------------------
	| The values in Z get added to A, B and C	|
	-------------------------------------------*/
	A->mQty += Z->mQty;
	B->mQty += Z->mQty;
	C->mQty += Z->mQty;
	D->mQty += Z->mQty;

	A->mSales += Z->mSales;
	B->mSales += Z->mSales;
	C->mSales += Z->mSales;
	D->mSales += Z->mSales;

	A->mCost += Z->mCost;
	B->mCost += Z->mCost;
	C->mCost += Z->mCost;
	D->mCost += Z->mCost;

	A->yQty += Z->yQty;
	B->yQty += Z->yQty;
	C->yQty += Z->yQty;
	D->yQty += Z->yQty;

	A->ySales += Z->ySales;
	B->ySales += Z->ySales;
	C->ySales += Z->ySales;
	D->ySales += Z->ySales;

	A->yCost += Z->yCost;
	B->yCost += Z->yCost;
	C->yCost += Z->yCost;
	D->yCost += Z->yCost;
}

/*===============================
| Zero the sales record totals	|
===============================*/
void
ZeroTotals (
 struct salesRecord*    thisStruct)
{
	thisStruct->mQty = 0.00;
	thisStruct->mSales = 0.00;
	thisStruct->mCost = 0.00;
	thisStruct->mMargin = 0.00;
	thisStruct->yQty = 0.00;
	thisStruct->ySales = 0.00;
	thisStruct->yCost = 0.00;
	thisStruct->yMargin = 0.00;
}
