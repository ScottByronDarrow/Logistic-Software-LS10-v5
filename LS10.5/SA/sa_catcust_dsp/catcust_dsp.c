/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_catcust_dsp.c     )                           |
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
| $Log: catcust_dsp.c,v $
| Revision 5.2  2001/08/09 09:16:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:09  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:20  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:30  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:49  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:17  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:35:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/16 04:55:30  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.8  1999/11/16 00:57:20  scott
| Updated for define of PAGE_SIZE on IBM.
|
| Revision 1.7  1999/09/29 10:12:42  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:25  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 02:01:50  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/06/18 09:39:19  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: catcust_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_catcust_dsp/catcust_dsp.c,v 5.2 2001/08/09 09:16:45 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <DateToString.h>
#include <pDate.h>


#define	SLEEP_TIME	2
#define	INT_PAGE_SIZE	14

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
	{1, LIN, "yend", 11, 23, INTTYPE, 
		"NN", "          ", 
		" ", local_rec.strYend, " Year End Month     :", "Enter 1-12.", 
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.yend}, 
	{1, LIN, "yendDesc", 11, 30, CHARTYPE, 
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
| Local Function Prototypes.                                                |
=====================================================================*/
int 	ValidCommandLine 	(int, char **);
int 	heading 			(int);
int 	spec_valid 			(int);
int 	GotData 			(int, float, float, float);
int 	DefaultYend 		(void);
void	OpenDB 				(void);
void 	CloseDB 			(void);
void 	GetSalesData 		(FILE *, int, int, int);
void 	CalcYTD 			(int, int, double *, float *, double *);
void 	StringToStructure 	(struct salesRecord *, char *);
void 	DisplayLine 		(int, int, char *, char *, struct salesRecord *);
void 	CalcMargin 			(int, struct salesRecord *data);
void 	DisplaySalesData 	(FILE *sortFile, int, int, int, int);
void 	SrchEsmr 			(char *);
void 	SrchExcf 			(char *);
void 	InitOutput 			(int, int, int);
void 	AddTotals 			(struct salesRecord *, struct salesRecord *, struct salesRecord *, struct salesRecord *, struct salesRecord *);
void 	ZeroTotals 			(struct salesRecord *);


int
main (
 int	argc,
 char	*argv [])
{
	int		currMonth;
	int		yendHolder;
	int		detail = FALSE;
	int		margin = FALSE;
	int		byBranch = TRUE;
	FILE	*fsort;

	/*---------------------------
	| check command arguments	|
	---------------------------*/
	if (!ValidCommandLine (argc, argv))
	{
		printf ("\007Usage; %s\n", argv [0]);
		return (EXIT_FAILURE);
	}
	else
	{
		OpenDB ();
		read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);    

		SETUP_SCR (vars);

		DateToDMY (comm_rec.dbt_date, NULL, &currMonth, NULL);

		local_rec.yend = DefaultYend ();
		sprintf (local_rec.strYend, "%2d", local_rec.yend);
		/*-------------------------------------------------------------------
		| Make another copy of yend as entry (1); will reset local_rec.yend |
		| but we still need to pass this default one to next program        |
		-------------------------------------------------------------------*/
		yendHolder = local_rec.yend;
		sprintf (local_rec.yendDesc, "%-10.10s", MonthName (local_rec.yend));
		
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
			
			if (local_rec.detSum [0] == 'D')
				detail = TRUE;
			if (local_rec.costMgn [0] == 'Y')
				margin = TRUE;
			if (!strcmp (local_rec.brNo, " A"))
				byBranch = FALSE;

			/*---------------------------------------------------------------
			| If SA_ENTER_YEND wasn't enabled local_rec.yend will be reset  |
			| so we use the default we copied at the start before entry (1);|
			---------------------------------------------------------------*/
			if (local_rec.yendEnabled == FALSE)
			{
				local_rec.yend = yendHolder;
			}

			sprintf (err_str,
					 "Processing : Category by Customer Sales %s",
					 (detail) ? "(Detailed)" : "(Summary)");
			dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);                      
			fsort = sort_open ("sale");
			GetSalesData (fsort, currMonth, local_rec.yend, byBranch);
			
			sprintf (err_str,
					 "Sorting : Category by Customer Sales %s",
					 (detail) ? "(Detailed)" : "(Summary)");
			dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);                      
			fsort = sort_sort (fsort, "sale");
			DisplaySalesData (fsort, local_rec.yend, detail, margin, byBranch);

			sort_delete (fsort, "sale");

			prog_exit = TRUE;
		}
	   
		clear ();
		fflush (stdout);
		crsr_on ();
		rset_tty ();
		snorm ();

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
	char	dataStr [250];

    float   mMargin;
    float   yQty;
    float   yMargin;
    double  ySales;
    double  yCost;

    float   *qty;
    double  *sales;
    double  *cost;

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
				cc = find_hash (cumr, &cumr_rec, EQUAL,"r", sadf_rec.hhcu_hash);
			
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
                | This is the data stuff        |
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
| Calculate the year to date figures    |                      
=======================================*/                               
void         
CalcYTD (
 int    month,                 
 int    fiscal,
 double *sales,                                              
 float  *qty,
 double *cost)
{   
    int i;                                                   
            
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
 int    currMonth,
 float  yQty,                                        
 float  ySales,                                      
 float  yCost)                                       
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
    int     p;
    char    temp [20];

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
DisplayLine (
 int	margin,
 int    type,
 char	*str1,
 char	*str2,
 struct salesRecord *data)
{
	char	temp [145];
	
	switch (type)
	{
    case 1:
		if (margin)
			sprintf (temp,
					 " %-16.16s^E %-39.39s^E        ^E        ^E        ^E        ^E        ^E        ^E        ^E",
					 str1,
					 str2);

		if (!margin)
        	sprintf (temp,
				 " %-16.16s^E %-39.39s^E        ^E        ^E        ^E        ^E        ^E",
				 str1,
				 str2);
        break;

    case 2:
		if (margin)
			sprintf (temp,
					 " %-16.16s^E %-39.39s^E%8.0f^E%8.0f^E%8.0f^E%8.2f^E%8.0f^E%8.0f^E%8.0f^E%8.2f",
					 str1,
					 str2,
					 data->mQty,
					 data->mSales,
					 data->mCost,
				  	 data->mMargin,
					 data->yQty,
					 data->ySales,
					 data->yCost,
					 data->yMargin);

		if (!margin)
			sprintf (temp,
					 " %-16.16s^E %-39.39s^E%8.0f^E%8.0f^E%8.0f^E%8.0f^E%8.0f^E%8.0f",
					 str1,
					 str2,
					 data->mQty,
					 data->mSales,
					 data->mCost,
					 data->yQty,
					 data->ySales,
					 data->yCost);
        break;

    case 3:
		if (margin)
			sprintf (temp,
					 " %-20.20s %-36.36s^E%8.0f^E%8.0f^E%8.0f^E%8.2f^E%8.0f^E%8.0f^E%8.0f^E%8.2f",
					 str1,
					 str2,
					 data->mQty,
					 data->mSales,
					 data->mCost,
				  	 data->mMargin,
					 data->yQty,
					 data->ySales,
					 data->yCost,
					 data->yMargin);

		if (!margin)
			sprintf (temp,
					 " %-20.20s %-36.36s^E%8.0f^E%8.0f^E%8.0f^E%8.0f^E%8.0f^E%8.0f",
					 str1,
					 str2,
					 data->mQty,
					 data->mSales,
					 data->mCost,
					 data->yQty,
					 data->ySales,
					 data->yCost);
        break;
	}

	Dsp_saverec (temp);
}

/*===============================================================
| Get the string form the sort file and jam it into a structure |
===============================================================*/
void    
CalcMargin (                                   
 int    margin,            
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

/*===================================
| Display the data on the screen	|
===================================*/
void
DisplaySalesData (
 FILE	*sortFile,
 int	yend,
 int	dtl,
 int	mgn,
 int	byBranch)
{
	int		firstTime = TRUE;
	char	*sptr;
    char    pItem [17];
    char    cItem [17];
	char	pCat [12];
	char	cCat [12];
	char	pCust [7];
	char	cCust [7];
	char	cCustDesc [41];
    char    pItemDesc [41];
	char	*line = "------------------------------------------------------------------------------------------------------------------------------------";

	struct salesRecord temp;
	struct salesRecord item;
	struct salesRecord customer;
	struct salesRecord category;
	struct salesRecord branch;

	struct salesRecord *ptrTemp;
	struct salesRecord *ptrItem;
	struct salesRecord *ptrCust;
	struct salesRecord *ptrCat;	
	struct salesRecord *ptrBr;

	ptrTemp = &temp;
	ptrItem = &item;
	ptrCust = &customer;
	ptrCat = &category;
	ptrBr = &branch;

	ZeroTotals (ptrItem);
	ZeroTotals (ptrCat);
	ZeroTotals (ptrCust);
	ZeroTotals (ptrBr);

	InitOutput (dtl, mgn, yend);

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
				DisplayLine (mgn, 1, cCust, cCustDesc, ptrTemp); 
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
				DisplayLine (mgn, 2, pItem, pItemDesc, ptrItem); 
            }  
            ZeroTotals (ptrItem);

            if (strcmp (cCust, pCust) || (strcmp (cCat, pCat)))
            {
                if (dtl)
                {
					DisplayLine (mgn, 3, "Total For Customer", "", ptrCust);
                }
                else
                {
					DisplayLine (mgn, 2, pCust, cCustDesc, ptrCust);
                }
                ZeroTotals (ptrItem);
                ZeroTotals (ptrCust);
            }

            if (strcmp (cCat, pCat))
            {  
				DisplayLine (mgn, 3, "Total For Category", pCat, ptrCat); 

                ZeroTotals (ptrItem);
                ZeroTotals (ptrCust);
                ZeroTotals (ptrCat);
            }

            if (strcmp (cCust, pCust) && dtl)
            {
				sprintf (cCustDesc, "%40.40s", sptr + 33);
				DisplayLine (mgn, 1, cCust, cCustDesc, ptrTemp); 
            }
        }

        AddTotals (ptrTemp, ptrItem, ptrCat, ptrCust, ptrBr);
        CalcMargin (mgn, ptrItem);
        CalcMargin (mgn, ptrCust);
        CalcMargin (mgn, ptrCat);

        sprintf (pCust, "%6.6s", sptr + 11);
        sprintf (pCat, "%11.11s", sptr);
        sprintf (pItem, "%17.17s", sptr + 17);
        sprintf (pItem, "%16.16s", sptr + 17);
        sprintf (pItemDesc, "%-40.40s", sptr + 73);
		sprintf (cCustDesc, "%40.40s", sptr + 33);
    }

    if (dtl)
    {
        DisplayLine (mgn, 3, "Total For Customer", "", ptrCust);
    }
    else
    {
        DisplayLine (mgn, 2, pCust, cCustDesc, ptrCust);
    }

    DisplayLine (mgn, 3, "Total For Category", pCat, ptrCat);

    CalcMargin (mgn, ptrBr);
	if (byBranch)
	{
		DisplayLine (mgn, 3, "Total For Branch", "", ptrBr);
	}
	else
	{
		DisplayLine (mgn, 3, "Total For Company", "", ptrBr);
	}

	Dsp_saverec (line);
	
	Dsp_srch ();
	Dsp_close ();
}

/*===================================
| Check command line args are OK	|
===================================*/
int
ValidCommandLine (
 int	argCount,
 char	**argValue)
{
	int OK = FALSE;

	if (argCount == 1)
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
        box (0, 3, 80, 8);
        move (1, 10);
        line (79);
    }
    else
    {
        box (0, 3, 80, 6);
    }

    move (1, 7);
    line (79);

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
spec_valid (int field)
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
    return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char   *key_val)
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
 char   *key_val)
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
void
InitOutput (
 int	detail,
 int	margin,
 int	fiscal)
{
	char	temp [100];

	clear ();
	swide ();

	Dsp_open (0, 1, INT_PAGE_SIZE);

	sprintf (temp,
			 " CATEGORY BY CUSTOMER%sREPORT - YEAR END MONTH %s (lines rounded - totals accurate) ",
			 detail ? " DETAIL " : " ",
			 MonthName (fiscal));                                    
    rv_pr (temp, (132 - strlen (temp)) / 2,0,1);
	
	if (margin)
	   Dsp_saverec ("                 |                                        "
				 "| <---------- MTD SALES ----------> "
				 "| <---------- YTD SALES ----------> ");

	if (!margin)
	   Dsp_saverec ("                 |                                        "
				 "| <------ MTD SALES -----> "
				 "| <------ YTD SALES -----> ");

	if (detail)
	{
		if (margin)
		Dsp_saverec(" CUSTOMER / ITEM |      CUSTOMER NAME / PRODUCT           "
					 "|  QTY   | SALES  |  COST  |%MARGIN |   QTY  "
					 "|  SALES |  COST  |%MARGIN ");
		if (!margin)
		Dsp_saverec(" CUSTOMER / ITEM |      CUSTOMER NAME / PRODUCT           "
					 "|  QTY   | SALES  |  COST  |   QTY  "
					 "|  SALES |  COST  ");
	}
	else
	{
		if (margin)
		Dsp_saverec("   CUSTOMER      |            CUSTOMER NAME               "
					 "|  QTY   | SALES  |  COST  |%MARGIN |   QTY  "
					 "|  SALES |  COST  |%MARGIN ");

		if (!margin)
		Dsp_saverec("   CUSTOMER      |            CUSTOMER NAME               "
					 "|  QTY   | SALES  |  COST  |   QTY  "
					 "|  SALES |  COST  ");
	}

    Dsp_saverec (" [REDRAW] [NEXT] [PREV] [EDIT/END] ");
}

/*===============================
| Add the sales record totals	|
===============================*/
void
AddTotals (
 struct salesRecord *Z,
 struct salesRecord *A,
 struct salesRecord *B,
 struct salesRecord *C,
 struct salesRecord *D)
{
	/*-----------------------------------------------
	| The values in Z get added to A, B, C and D	|
	-----------------------------------------------*/
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
 struct salesRecord *thisStruct)
{
	thisStruct->mQty 	= 0.00;
	thisStruct->mSales 	= 0.00;
	thisStruct->mCost 	= 0.00;
	thisStruct->mMargin = 0.00;
	thisStruct->yQty 	= 0.00;
	thisStruct->ySales 	= 0.00;
	thisStruct->yCost 	= 0.00;
	thisStruct->yMargin = 0.00;
}
