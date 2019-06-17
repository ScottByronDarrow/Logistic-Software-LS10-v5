/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_sadf_area.c                                   |
|  Program Desc  : ( Sales Analysis by Sales Area Report.         )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: sadf_area.c,v $
| Revision 5.3  2002/07/17 09:57:47  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:04  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:35  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:40  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:48  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:00  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:29  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  2000/02/18 02:35:23  scott
| Updated to fix small compile warings errors found when compiled under Linux.
|
| Revision 1.9  1999/12/06 01:35:31  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/11/16 04:55:34  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.7  1999/09/29 10:12:51  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/17 07:27:35  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.5  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.4  1999/06/18 09:39:21  scott
| Updated for read_comm (), log for cvs, compile errors.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sadf_area.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf_area/sadf_area.c,v 5.3 2002/07/17 09:57:47 scott Exp $";
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <pr_format3.h>
#include <get_lpno.h>
#include <DateToString.h>
#include <pDate.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 7;

	struct {
		int		termno;
		char	co_no[3];
		char	co_name[41];
		char	est_no[3];
		char	est_name[41];
		long	dbt_date;
		int		fiscal;
	} comm_rec;

	/*=============================================
	| Sales Analysis Detail file By Item/Customer |
	=============================================*/
	struct dbview sadf_list[] ={
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
		{"sadf_sman"},
		{"sadf_area"},
	};

	int	sadf_no_fields = 43;

	struct	{
		char	co_no[3];
		char	br_no[3];
		char	year[2];
		long	hhbr_hash;
		long	hhcu_hash;
		float	qty_per[12];
		double	sal_per[12];
		double	cst_per[12];
		char	sman[3];
		char	area[3];
	} sadf_rec;

	/*=====================+
	 | External Area file. |
	 +=====================*/
#define	EXAF_NO_FIELDS	3

	struct dbview	exaf_list [EXAF_NO_FIELDS] =
	{
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
	};

	struct tag_exafRecord
	{
		char	co_no [3];
		char	area_code [3];
		char	area [41];
	}	exaf_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int exsf_no_fields = 3;

	struct {
		char	co_no[3];
		char	salesman_no[3];
		char	salesman[41];
	} exsf_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
	};

	int inmr_no_fields = 9;

	struct {
		char	co_no[3];
		char	item_no[17];
		long	hhbr_hash;
		char	alpha_code[17];
		char	maker_no[17];
		char	alternate[17];
		char	_class[2];
		char	description[41];
		char	category[12];
	} inmr_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	int excf_no_fields = 3;

	struct {
		char	co_no[3];
		char	cat_no[12];
		char	cat_desc[41];
	} excf_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_short_name"},
		{"esmr_est_name"},
	};

	int	esmr_no_fields = 4;

	struct	{
		char	co_no[3];
		char	est_no[3];
		char	short_name[16];
		char	name[41];
	} esmr_rec;

	char	currItem[17],
			prevItem[17],
			currCat[12],
			prevCat[12],
			currSman[3],
			prevSman[3],
			currArea[3],
			prevArea[3];

	int		dataFnd = FALSE;
	int		month;

	float	m_qty[5],
			m_sales[5],
			m_csale[5],
			y_qty[5],
			y_sales[5],
			y_csale[5];

	FILE	*fin,
			*fout,
			*fsort;

	char	*sadf = "sadf",
			*inmr = "inmr",
			*esmr = "esmr",
			*exsf = "exsf",
			*exaf = "exaf",
			*excf = "excf",
			*data = "data",
			*comm = "comm";

struct {
	char	brNo[3];
	char	brName[41];
	char	sArea[3];
	char	eArea[3];
	char	sCat[12];
	char	eCat[12];
	char	detSum[2];
	char	detSumDesc[9];
	char	showSman[2];
	char	showSmanDesc[4];
	char	costMgn[2];
	char	costMgnDesc[4];
	int		lpno;
	char	back[2];
	char	backDesc[4];
	char	onight[2];
	char	onightDesc[4];
	char	dummy[11];
    int     yendEnabled;
    int     yend;
    char    strYend [3];
    char    yendDesc [11];
} local_rec;
	
#define		SLEEP_TIME	2

#define		AREA	0
#define		SMAN	AREA + 3
#define		CAT		SMAN + 3
#define		ITEM	CAT + 12
#define		BR		ITEM + 17
#define		MQTY	BR + 3
#define		MSALE	MQTY + 11
#define		MCSALE	MSALE + 11
#define		YQTY	MCSALE + 11
#define		YSALE	YQTY + 11
#define		YCSALE	YSALE + 11

#define		ALL_BRS			(!strcmp (local_rec.brNo, " A"))
#define		SHOW_SMAN		(local_rec.showSman[0] == 'Y')
#define		DETAIL			(local_rec.detSum[0] == 'D')
#define		COST_MGN		(local_rec.costMgn[0] == 'Y')

static	struct	var	vars[]	=	
{
	{1, LIN, "brNo", 4, 23, CHARTYPE, 
		"UU", "          ", 
		" ", " ", " Branch Number      :", "Default is current branch. Enter [A] for All Branches.", 
		YES, NO, JUSTRIGHT, "", "", local_rec.brNo}, 
	{1, LIN, "brName", 4, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.brName}, 
	{1, LIN, "sArea", 6, 23, CHARTYPE, 
		"UU", "          ", 
		" ", "  ", " Sales Area From    :", "", 
		YES, NO, JUSTRIGHT, "", "", local_rec.sArea}, 
	{1, LIN, "eArea", 6, 62, CHARTYPE, 
		"UU", "          ", 
		" ", "~~", " To ", "", 
		YES, NO, JUSTRIGHT, "", "", local_rec.eArea}, 
	{1, LIN, "sCat", 7, 23, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "           ", " Category From      :", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.sCat}, 
	{1, LIN, "eCat", 7, 62, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "~~~~~~~~~~~", " To ", "", 
		YES, NO, JUSTLEFT, "", "", local_rec.eCat}, 
	{1, LIN, "detSum", 9, 23, CHARTYPE, 
		"U", "          ", 
		" ", "S", " Detailed / Summary :", "S(ummary) to report at Category level, D(etail) at Item level.", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.detSum}, 
	{1, LIN, "detSumDesc", 9, 23, CHARTYPE, 
		"AAAAAAAAA", "          ", 
		" ", "Summary ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.detSumDesc}, 
	{1, LIN, "showSman", 10, 23, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Show Salesman ?    :", "Y(es) to report by Salesman, N(o) to exclude salesman information.", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.showSman}, 
	{1, LIN, "showSmanDesc", 10, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "No ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.showSmanDesc}, 
	{1, LIN, "costMgn", 11, 23, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Print Cost Margin  :", "Y(es) to print Cost Margin, N(o) to exclude cost margin.", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.costMgn}, 
	{1, LIN, "costMgnDesc", 11, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "Yes", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.costMgnDesc}, 
	{1, LIN, "yend", 13, 23, INTTYPE, 
		"NN", "          ", 
		" ", local_rec.strYend, " Year End Month     :", "Enter 1-12.", 
		ND, NO, JUSTLEFT, "", "", (char *)&local_rec.yend}, 
	{1, LIN, "yendDesc", 13, 30, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.yendDesc}, 
	{1, LIN, "lpno", 13, 23, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer Number     :", "Enter valid printer number.  (Default is 1.)", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 14, 23, CHARTYPE, 
		"U", "          ", 
		" ", "Y", " Background         :", "Y(es) to run the program in background, N(o) to run in foreground", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 14, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "Yes", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onight", 15, 23, CHARTYPE, 
		"U", "          ", 
		" ", "N", " Overnight          :", "Y(es) to schedule the program to run overnight, N(o) to run now", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{1, LIN, "onightDesc", 15, 23, CHARTYPE, 
		"AAA", "          ", 
		" ", "No ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void OpenDB (void);
void CloseDB (void);
int run_prog (void);
void Process (void);
void GetData (char *prevBr, char *prevArea, char *prevCat, char *prevItem, char *prevSman);
void PrintData (void);
int CheckBreak (void);
void ProcData (int firstTime, int brkLevel);
void SumSales (char *data_line, int add);
void PrintLine (void);
void PrintTot (char *totType);
void InitOutput (void);
void InitArray (void);
void PrintHead (char *head_type);
void CalcMtd (void);
void CalcYtd (void);
int spec_valid (int field);
int heading (int scn);
void SrchEsmr (char *key_val);
void SrchExaf (char *key_val);
void SrchExcf (char *key_val);
int ValidMonthYearEnd (int month);
int DefaultYend (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	int	yendHolder;

	SETUP_SCR (vars);

	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *)&comm_rec);
	
	DateToDMY (comm_rec.dbt_date, NULL, &month, NULL);

	local_rec.yend = DefaultYend ();
    sprintf (local_rec.strYend, "%2d", local_rec.yend);

    /*-------------------------------------------------------------------
    | Make another copy of yend as entry (1); will reset local_rec.yend |
    | but we still need to pass this default one to next program        |
    -------------------------------------------------------------------*/
    yendHolder = local_rec.yend;
    sprintf (local_rec.yendDesc, "%-10.10s", MonthName (local_rec.yend));

	if (argc == 11)
	{
		strcpy (local_rec.brNo, argv[1]);
		strcpy (local_rec.sArea, argv[2]);
		strcpy (local_rec.eArea, argv[3]);
		strcpy (local_rec.sCat, argv[4]);
		strcpy (local_rec.eCat, argv[5]);
		strcpy (local_rec.detSum, argv[6]);
		strcpy (local_rec.showSman, argv[7]);
		strcpy (local_rec.costMgn, argv[8]);
		local_rec.lpno = atoi (argv[9]);
		local_rec.yend = atoi (argv [10]);

		Process ();
		
		CloseDB (); 
		FinishProgram ();
        return (EXIT_SUCCESS);
	}

	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
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

		prog_exit = TRUE;

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
	}

	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (sadf, sadf_list, sadf_no_fields, "sadf_id_no5");
	open_rec (inmr, inmr_list, inmr_no_fields, "inmr_hhbr_hash");
	open_rec (esmr, esmr_list, esmr_no_fields, "esmr_id_no");
	open_rec (exsf, exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (excf, excf_list, excf_no_fields, "excf_id_no");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (sadf);
	abc_fclose (inmr);
	abc_fclose (esmr);
	abc_fclose (exsf);
	abc_fclose (exaf);
	abc_fclose (excf);
	abc_dbclose (data);
}

int
run_prog (void)
{
	char	lpno[3];
	char	yend [3];

	sprintf (lpno, "%2d", local_rec.lpno);
	sprintf (yend, "%2d", local_rec.yend);
	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (local_rec.onight[0] == 'Y') 
	{
	    if (fork () == 0)
        {
			execlp ("ONIGHT",
				    "ONIGHT",
				    "sa_sadf_area",
				    local_rec.brNo,
				    local_rec.sArea,
				    local_rec.eArea,
				    local_rec.sCat,
				    local_rec.eCat,
				    local_rec.detSum,
				    local_rec.showSman,
				    local_rec.costMgn,
				    lpno,
				    yend,
				    "Sales Analysis By Sales Area",
                    (char *)0);
        }
	}
	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (local_rec.back[0] == 'Y') 
	{
		if (fork () == 0)
        {
		    execlp ("sa_sadf_area",
				    "sa_sadf_area",
				    local_rec.brNo,
				    local_rec.sArea,
				    local_rec.eArea,
				    local_rec.sCat,
				    local_rec.eCat,
				    local_rec.detSum,
				    local_rec.showSman,
				    local_rec.costMgn,
				    lpno,
				    yend,
                    (char *)0);
        }
	}
	else 
    {
		Process ();
    }
	return (EXIT_SUCCESS);
}

void
Process (void)
{
    char    prevArea[3],
            prevSman[3],
            prevCat[12],
            prevItem[17],
            prevBr[3];
	long	currHhcu,
			prevHhcu = 0L,
			currHhbr,
			prevHhbr = 0L;

	int		firstTime = TRUE;

	abc_selfield (inmr, "inmr_hhbr_hash");
	fsort = sort_open ("sale");

	dsp_screen (" Locating Sales Analysis Records ",
				comm_rec.co_no,
				comm_rec.co_name);

	strcpy (sadf_rec.co_no, comm_rec.co_no);
	strcpy (sadf_rec.year, "C");
	strcpy (sadf_rec.br_no, (ALL_BRS) ? "  " : local_rec.brNo);
	strcpy (sadf_rec.area, local_rec.sArea);
	sadf_rec.hhcu_hash = 0L;
	sadf_rec.hhbr_hash = 0L;
	for (cc = find_rec (sadf,&sadf_rec,GTEQ,"r");
		!cc && 
		!strcmp (sadf_rec.co_no, comm_rec.co_no) &&
		!strcmp (sadf_rec.year, "C");
		cc = find_rec (sadf, &sadf_rec, NEXT, "r"))
	{

		if ((!ALL_BRS && strcmp (sadf_rec.br_no, local_rec.brNo)) ||
			strcmp (sadf_rec.area, local_rec.eArea) > 0)
			break;

		if (ALL_BRS && (strcmp (sadf_rec.area, local_rec.eArea) > 0  ||
						strcmp (sadf_rec.area, local_rec.sArea) < 0))
			continue;

		currHhcu = sadf_rec.hhcu_hash;
		currHhbr = sadf_rec.hhbr_hash;

		cc = find_hash (inmr, &inmr_rec, EQUAL, "r", sadf_rec.hhbr_hash);
		if (cc ||
			strcmp (inmr_rec.category, local_rec.sCat) < 0 ||
			strcmp (inmr_rec.category, local_rec.eCat) > 0)
		{
			continue;
		}

		dsp_process (" Item : ", inmr_rec.item_no);

		if (!firstTime && (currHhcu != prevHhcu || currHhbr != prevHhbr))
		{
			GetData (prevBr, prevArea, prevCat, prevItem, prevSman);

			m_qty[0] = 0.00;
			m_sales[0] = 0.00;
			m_csale[0] = 0.00;
			y_qty[0] = 0.00;
			y_sales[0] = 0.00;
			y_csale[0] = 0.00;

			prevHhcu = currHhcu;
			prevHhbr = currHhbr;
		}

		CalcMtd ();
		CalcYtd ();

		strcpy (prevArea, sadf_rec.area);
		strcpy (prevSman, sadf_rec.sman);
		strcpy (prevCat, inmr_rec.category);
		strcpy (prevItem, inmr_rec.item_no);
		strcpy (prevBr, sadf_rec.br_no);

		firstTime = FALSE;
	}

	GetData (prevBr, prevArea, prevCat, prevItem, prevSman);

	PrintData ();
}

void
GetData (
 char *prevBr,
 char *prevArea,
 char *prevCat,
 char *prevItem,
 char *prevSman)
{
	char	dataStr[111];

	if (m_qty[0]   != 0 || m_sales[0] != 0 || m_csale[0] != 0 ||
     	y_qty[0]   != 0 || y_sales[0] != 0 || y_csale[0] != 0 )
	{
		if (SHOW_SMAN)
		{
			sprintf (dataStr, "%2.2s %2.2s %11.11s %16.16s %2.2s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n", 
				 prevArea,
				prevSman,
				prevCat,
				prevItem,
				prevBr,
				m_qty[0],
				m_sales[0],
				m_csale[0],
				y_qty[0],
				y_sales[0],
				y_csale[0]);
		}			
		else
		{
			sprintf (dataStr, "%2.2s %11.11s %16.16s %2.2s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n", 
				prevArea,
				prevCat,
				prevItem,
				prevBr,
				m_qty[0],
				m_sales[0],
				m_csale[0],
				y_qty[0],
				y_sales[0],
				y_csale[0]);
		}	

		sort_save (fsort, dataStr);
	}
}

void
PrintData (void)
{
	char	*sptr;
	int		firstTime,
			brkLevel;

	abc_selfield (inmr, "inmr_id_no");
	InitArray ();
	InitOutput ();
	firstTime = TRUE;

	fsort = sort_sort (fsort, "sale");

	dsp_screen (" Printing Sales Analysis Report ", comm_rec.co_no,
														comm_rec.co_name);

	sptr = sort_read (fsort);
	while (sptr != (char *)0)
	{
		sprintf (currItem, "%16.16s", (SHOW_SMAN) ? sptr + ITEM : sptr + 15);
		sprintf (currCat, "%11.11s", (SHOW_SMAN) ? sptr + CAT : sptr + 3);
		if (SHOW_SMAN)
			sprintf (currSman, "%2.2s", sptr + SMAN);
		sprintf (currArea, "%2.2s", sptr + AREA);

		dsp_process ("Item No :",currItem);

		if (firstTime)
		{
			strcpy (prevItem, currItem);
			strcpy (prevCat, currCat);
			if (SHOW_SMAN)
				strcpy (prevSman, currSman);
			strcpy (prevArea, currArea);
		}

		brkLevel = CheckBreak ();

		ProcData (firstTime,brkLevel);
		firstTime = FALSE;

		if (brkLevel == 0 || (!DETAIL && brkLevel == 1))
			SumSales (sptr, TRUE);
		else
			SumSales (sptr, FALSE);

		strcpy (prevItem, currItem);
		strcpy (prevCat, currCat);
		if (SHOW_SMAN)
			strcpy (prevSman, currSman);
		strcpy (prevArea, currArea);

		sptr = sort_read (fsort);
		dataFnd = TRUE;
	}

	if (dataFnd)
	{
		PrintLine ();
		if (DETAIL)
			PrintTot ("C");
		if (SHOW_SMAN)
			PrintTot ("S");
		PrintTot ("A");
		PrintTot ("G");
	}
	sort_delete (fsort, "sale");
}

int
CheckBreak (void)
{
	if (strcmp (currArea, prevArea))
		return (4);

	if (strcmp (currSman, prevSman))
		return (3);

	if (strcmp (currCat, prevCat))
		return (2);

	if (strcmp (currItem, prevItem))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
ProcData (
 int	firstTime,
 int	brkLevel)
{
	if (firstTime)
	{
		PrintHead ("A");
		if (SHOW_SMAN)
		{
			PrintHead ("S");
			if (DETAIL)
				PrintHead ("C");
		}
		else
		{
			if (DETAIL)
				PrintHead ("C");
		}
	}

	if (!firstTime && (brkLevel == 4 || brkLevel == 3 || brkLevel == 2))
	{
		PrintLine ();

		switch (brkLevel)
		{
		case	2:
			if (DETAIL)
			{
				PrintTot ("C");
				PrintHead ("C");
			}
			break;

		case	3:
			if (DETAIL)
				PrintTot ("C");

			if (SHOW_SMAN)
			{
				PrintTot ("S");
				PrintHead ("S");
			}
			if (DETAIL)
				PrintHead ("C");
			break;

		case	4:
			if (DETAIL)
				PrintTot ("C");

			if (SHOW_SMAN)
				PrintTot ("S");

			PrintTot ("A");
/*			if (DETAIL && SHOW_SMAN) */
			fprintf (fout, ".PA\n");

			PrintHead ("A");
			if (SHOW_SMAN)
				PrintHead ("S");
			if (DETAIL)
				PrintHead ("C");
			break;

		default	:
			break;
		}
	}

	if ((DETAIL && brkLevel == 1) || (!DETAIL && brkLevel == 2))
		PrintLine ();
}

void
SumSales (
 char	*data_line,
 int	add)
{
	char	*sptr = data_line;

	if (add)
	{
		m_qty[(DETAIL) ? 0 : 1]   += atof ((SHOW_SMAN) ? sptr + MQTY 
														: sptr + 35);
		m_sales[(DETAIL) ? 0 : 1] += atof ((SHOW_SMAN) ? sptr + MSALE 
														: sptr + 46);
		m_csale[(DETAIL) ? 0 : 1] += atof ((SHOW_SMAN) ? sptr + MCSALE 
														: sptr + 57);
		y_qty[(DETAIL) ? 0 : 1]   += atof ((SHOW_SMAN) ? sptr + YQTY 
														: sptr + 68);
		y_sales[(DETAIL) ? 0 : 1] += atof ((SHOW_SMAN) ? sptr + YSALE 
														: sptr + 79);
		y_csale[(DETAIL) ? 0 : 1] += atof ((SHOW_SMAN) ? sptr + YCSALE 
														: sptr + 90);
	}
	else
	{
		m_qty[(DETAIL) ? 0 : 1]   = atof ((SHOW_SMAN) ? sptr + MQTY 
														: sptr + 35);
		m_sales[(DETAIL) ? 0 : 1] = atof ((SHOW_SMAN) ? sptr + MSALE 
														: sptr + 46);
		m_csale[(DETAIL) ? 0 : 1] = atof ((SHOW_SMAN) ? sptr + MCSALE 
														: sptr + 57);
		y_qty[(DETAIL) ? 0 : 1]   = atof ((SHOW_SMAN) ? sptr + YQTY 
														: sptr + 68);
		y_sales[(DETAIL) ? 0 : 1] = atof ((SHOW_SMAN) ? sptr + YSALE 
														: sptr + 79);
		y_csale[(DETAIL) ? 0 : 1] = atof ((SHOW_SMAN) ? sptr + YCSALE 
														: sptr + 90);
	}
}

void
PrintLine (void)
{
	int		i;
	float	mMargin = 0.00;
	float	yMargin = 0.00;
	char	mMarg[8],
			yMarg[8];

	i = (DETAIL) ? 0 : 1;

	/*-----------------------------------------------
	| If sales & cost of sales  = 0.00, don't print	|
	-----------------------------------------------*/
	if (m_qty[i] == 0.00 && m_sales[i] == 0.00 &&
		m_csale[i] == 0.00 && y_qty[i] == 0.00 &&
		y_sales[i] == 0.00 && y_csale[i] == 0.00)
			return;

	if (DETAIL)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, prevItem);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			sprintf (inmr_rec.description, " No Description Found");

		fprintf (fout, "| %16.16s ", prevItem);
		fprintf (fout, "| %40.40s ", inmr_rec.description);
	}
	else
	{
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", prevCat);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
			strcpy (excf_rec.cat_desc, "No Description Found");

		fprintf (fout, "|   %11.11s    ", prevCat);
		fprintf (fout, "| %40.40s ", excf_rec.cat_desc);
	}

	if (COST_MGN)
	{
		if (m_sales[i] != 0.00)
			mMargin = (m_sales[i] - m_csale[i]) / m_sales[i] * 100.00;

		if (mMargin > 10000)
			strcpy (mMarg, "+******");
		else if (mMargin <= -10000)
			strcpy (mMarg, "-******");
		else
			sprintf (mMarg, "%7.1f", mMargin);

		if (y_sales[i] != 0.00)
			yMargin = (y_sales[i] - y_csale[i]) / y_sales[i] * 100.00;

		if (yMargin > 10000)
			strcpy (yMarg, "+******");
		else if (yMargin <= -10000)
			strcpy (yMarg, "-******");
		else
			sprintf (yMarg, "%7.1f", yMargin);
	}

	fprintf (fout, "|%10.2f", m_qty[i]);
	fprintf (fout, "|%10.2f", m_sales[i]);
	if (COST_MGN)
	{
		fprintf (fout, "|%10.2f", m_csale[i]);
		fprintf (fout, "|%10.10s", mMarg);
	}
	fprintf (fout, "|%10.2f", y_qty[i]);
	fprintf (fout, "|%10.2f", y_sales[i]);
	if (COST_MGN)
	{
		fprintf (fout, "|%10.2f", y_csale[i]);
		fprintf (fout, "|%10.10s|\n", yMarg);
	}
	else
		fprintf (fout, "|\n");
/**********

	Fix HERE!
	*********/
	if (SHOW_SMAN)
	{
		m_qty[i + 1] += m_qty[i];
		m_sales[i + 1] += m_sales[i];
		m_csale[i + 1] += m_csale[i];
		y_qty[i + 1] += y_qty[i];
		y_sales[i + 1] += y_sales[i];
		y_csale[i + 1] += y_csale[i];
	}
	else
	{
		m_qty[i + 2] += m_qty[i];
		m_sales[i + 2] += m_sales[i];
		m_csale[i + 2] += m_csale[i];
		y_qty[i + 2] += y_qty[i];
		y_sales[i + 2] += y_sales[i];
		y_csale[i + 2] += y_csale[i];
	}

	m_qty[i] = 0.00;
	m_sales[i] = 0.00;
	m_csale[i] = 0.00;
	y_qty[i] = 0.00;
	y_sales[i] = 0.00;
	y_csale[i] = 0.00;
}

void
PrintTot (
 char	*totType)
{
	int		j = 0;
	float	mMargin = 0.00;
	float	yMargin = 0.00;
	char	mMarg[8],
			yMarg[8];

	switch (totType[0])
	{
	case	'C':
		if (SHOW_SMAN)
			j = 1;
		else
			j = 2;

		sprintf (err_str, "%-18.18s %-40.40s", "Total For Category",
													excf_rec.cat_no);
		break;

	case	'S':
		j = 2;
		sprintf (err_str, "%-18.18s %-40.40s", "Total For Salesman", 
													exsf_rec.salesman);
		break;

	case	'A':
		j = 3;
		sprintf (err_str, "%-14.14s     %-40.40s", "Total For Area", 
												exaf_rec.area);
		break;

	case	'G':
		j = 4;
		sprintf (err_str, "%11.11s        %40.40s", "Grand Total", " ");
		break;
	}

	if (COST_MGN)
	{
		if (m_sales[j] != 0.00)
			mMargin = (m_sales[j] - m_csale[j]) / m_sales[j] * 100.00;

		if (mMargin > 10000)
			strcpy (mMarg, "+******");
		else if (mMargin <= -10000)
			strcpy (mMarg, "-******");
		else
			sprintf (mMarg, "%7.1f", mMargin);

		if (y_sales[j] != 0.00)
			yMargin = (y_sales[j] - y_csale[j]) / y_sales[j] * 100.00;

		if (yMargin > 10000)
			strcpy (yMarg, "+******");
		else if (yMargin <= -10000)
			strcpy (yMarg, "-******");
		else
			sprintf (yMarg, "%7.1f", yMargin);
	}

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|----------|----------");
	if (COST_MGN)
		fprintf (fout, "|----------|----------");
	fprintf (fout, "|----------|----------");
	if (COST_MGN)
		fprintf (fout, "|----------|----------");
	fprintf (fout, "|\n");

	fprintf (fout, "| %s ", err_str);
	fprintf (fout, "|%10.2f", m_qty[j]);
	fprintf (fout, "|%10.2f", m_sales[j]);

	if (COST_MGN)
	{
		fprintf (fout, "|%10.2f", m_csale[j]);
		fprintf (fout, "|%10.10s", mMarg);
	}
	fprintf (fout, "|%10.2f", y_qty[j]);
	fprintf (fout, "|%10.2f", y_sales[j]);
	if (COST_MGN)
	{
		fprintf (fout, "|%10.2f", y_csale[j]);
		fprintf (fout, "|%10.10s|\n", yMarg);
	}
	else
		fprintf (fout, "|\n");

	if (totType [0] != 'A')
	{
		fprintf (fout, "|------------------");
		fprintf (fout, "|------------------------------------------");
		fprintf (fout, "|----------|----------");
		if (COST_MGN)
			fprintf (fout, "|----------|----------");
		fprintf (fout, "|----------|----------");
		if (COST_MGN)
			fprintf (fout, "|----------|----------");
		fprintf (fout, "|\n");
	}

	if (totType[0] != 'G')
	{
		m_qty  [j + 1] += m_qty[ j ];
		m_sales[j + 1] += m_sales[ j ];
		m_csale[j + 1] += m_csale[ j ];
		y_qty  [j + 1] += y_qty[ j ];
		y_sales[j + 1] += y_sales[ j ];
		y_csale[j + 1] += y_csale[ j ];

		/*if (!SHOW_SMAN && totType[0] == 'C')
		{
			m_qty  [j + 2] += m_qty[ j ];
			m_sales[j + 2] += m_sales[ j ];
			m_csale[j + 2] += m_csale[ j ];
			y_qty  [j + 2] += y_qty[ j ];
			y_sales[j + 2] += y_sales[ j ];
			y_csale[j + 2] += y_csale[ j ];
		}*/
	}

	m_qty[ j ]   = 0.00;
	m_sales[ j ] = 0.00;
	m_csale[ j ] = 0.00;
	y_qty[ j ]   = 0.00;
	y_sales[ j ] = 0.00;
	y_csale[ j ] = 0.00;
}

void
InitOutput (void)
{
	int		i;

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	i = 12;
	i += (DETAIL) ? 1: 0;
	i += (SHOW_SMAN) ? 1 : 0;
		
	fprintf (fout, ".START%s\n", DateToString (TodaysDate ()));
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".%d\n", i);
	if (COST_MGN)
		fprintf (fout, ".L158\n");
	else
		fprintf (fout, ".L110\n");

	fprintf (fout, ".ESALES ANALYSIS BY SALES AREA\n");
	fprintf (fout, ".ECOMPANY : %s - %s\n", comm_rec.co_no,
					      clip (comm_rec.co_name));

	if (ALL_BRS)
		fprintf (fout,".EALL BRANCHES\n");
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.brNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		fprintf (fout,".EBRANCH : %-2.2s - %-s\n",
					local_rec.brNo, clip (esmr_rec.name));
	}
		
	fprintf (fout,
			 ".EFOR THE MONTH OF %s - YEAR END MONTH %s\n",
			 MonthName (month),
			 MonthName (local_rec.yend));
	fprintf (fout, ".EFor Area %2s to %2s\n", local_rec.sArea, local_rec.eArea);
	fprintf (fout, ".EFor Category %11s to %11s\n", local_rec.sCat, 
													local_rec.eCat);

	fprintf (fout, ".R==================================================");
	fprintf (fout, "==================================================");
	if (COST_MGN)
		fprintf (fout, "===================================================\n");
	else
		fprintf (fout, "=======\n");

	fprintf (fout, "==================================================");
	fprintf (fout, "==================================================");
	if (COST_MGN)
		fprintf (fout, "===================================================\n");
	else
		fprintf (fout, "=======\n");

	fprintf (fout, "|   Sales Area /   ");
	fprintf (fout, "|            Sales Area Name /             ");
	if (COST_MGN)
	{
		fprintf (fout, "|  <--------------MTD SALES ------------->  ");
		fprintf (fout, "|  <--------------YTD SALES ------------->  |\n");
	}
	else
		fprintf (fout, "| <--- MTD SALES ---> | <--- YTD SALES ---> |\n");

	if (SHOW_SMAN)
	{
		fprintf (fout, "| Salesman No. /   ");
		fprintf (fout, "|            Salesman Name /               ");
		if (COST_MGN)
			fprintf (fout, "|%87.87s|\n", " ");
		else
			fprintf (fout, "|%43.43s|\n", " ");
	}
	fprintf (fout, "|    Category  /   ");
	fprintf (fout, "|              Category Name /             ");

	if (DETAIL)
	{
		if (COST_MGN)
		{
			fprintf (fout, "|                                           ");
			fprintf (fout, "|                                           |\n");
		}
		else
			fprintf (fout, "|                     |                     |\n");

		fprintf (fout, "|      Product     ");
		fprintf (fout, "|            Product Description           ");
	}

	fprintf (fout, "| Quantity |   Sales  ");
	if (COST_MGN)
		fprintf (fout, "|   Cost   | %% Margin ");

	fprintf (fout, "| Quantity |   Sales  ");
	if (COST_MGN)
		fprintf (fout, "|   Cost   | %% Margin |\n");
	else
		fprintf (fout, "|\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|----------|----------");
	if (COST_MGN)
		fprintf (fout, "|----------|----------");
	fprintf (fout, "|----------|----------");
	if (COST_MGN)
		fprintf (fout, "|----------|----------");
	fprintf (fout, "|\n");
}

void
InitArray (void)
{
	int	j;

	for (j = 0; j < 5; j++)
	{
		m_qty[ j ]   = 0.00;
		y_qty[ j ]   = 0.00;
		m_sales[ j ] = 0.00;
		m_csale[ j ] = 0.00;
		y_sales[ j ] = 0.00;
		y_csale[ j ] = 0.00;
	}
}

void
PrintHead (
 char	*head_type)
{
	char	tmpCode[17],
			tmpDesc[41];

	switch (head_type[0])
	{
	case	'A':
		strcpy (exaf_rec.co_no, comm_rec.co_no);
		sprintf (exaf_rec.area_code, "%-2.2s", currArea);
		cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
		if (cc)
			sprintf (exaf_rec.area, "No Area Found%27.27s", " ");

		sprintf (tmpCode, "%-2.2s%14.14s", exaf_rec.area_code, " ");
		strcpy (tmpDesc, exaf_rec.area);
		break;

	case	'S':
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%-2.2s", currSman);
		cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
		if (cc)
			sprintf (exsf_rec.salesman, "No Salesman Found%23.23s", " ");

		sprintf  (tmpCode, "%-2.2s%14.14s", exsf_rec.salesman_no, " ");
		strcpy (tmpDesc, exsf_rec.salesman);
		break;

	case	'C':
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", currCat);
		cc = find_rec (excf, &excf_rec, EQUAL, "r");
		if (cc)
			sprintf (excf_rec.cat_desc, "No Description Found%20.20s", " ");

		sprintf (tmpCode, "%11.11s%5.5s", excf_rec.cat_no, " ");
		strcpy (tmpDesc, excf_rec.cat_desc);
		break;
	}

	expand (err_str, tmpDesc);

	if (COST_MGN)
		fprintf (fout, "| %16.16s  %-130.130s|\n", tmpCode, err_str);
	else
		fprintf (fout, "| %16.16s  %-85.85s |\n", tmpCode, err_str);
}

void
CalcMtd (void)
{
	m_qty[0]   += sadf_rec.qty_per[month - 1];
	m_sales[0] += sadf_rec.sal_per[month - 1];
	m_csale[0] += sadf_rec.cst_per[month - 1];
}

void
CalcYtd (void)
{
	int	i;

	if (month <= local_rec.yend)
	{
		for (i = local_rec.yend; i < 12; i++)
		{
			y_qty[0]   += sadf_rec.qty_per[ i ];
			y_sales[0] += sadf_rec.sal_per[ i ];
			y_csale[0] += sadf_rec.cst_per[ i ];
		}

		for (i = 0; i < month; i++)
		{
			y_qty[0]   += sadf_rec.qty_per[ i ];
			y_sales[0] += sadf_rec.sal_per[ i ];
			y_csale[0] += sadf_rec.cst_per[ i ];
		}

	}
	else
	{
		for (i = local_rec.yend; i < month; i++)
		{
			y_qty[0] += sadf_rec.qty_per[i];
			y_sales[0] += sadf_rec.sal_per[i];
			y_csale[0] += sadf_rec.cst_per[i];
		}
	}
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
			strcpy (local_rec.brName, "All Branches");
			DSP_FLD ("brName");
			return (EXIT_SUCCESS);
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

		strcpy (local_rec.brName, esmr_rec.name);
		DSP_FLD ("brName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sArea"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.sArea, "  ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.sArea, local_rec.eArea) > 0)
		{
			print_mess ("Start Area Must Be Less Than End Area\007");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eArea"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.eArea, "~~");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.sArea, local_rec.eArea) > 0)
		{
			print_mess ("End Area Must Be Greater Than Start Area\007");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

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
			return (EXIT_SUCCESS);;
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
				
		if (prog_status != ENTRY &&
			strcmp (local_rec.sCat, local_rec.eCat) > 0)
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
			return (EXIT_SUCCESS);;
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

		if (local_rec.detSum[0] == 'D')
			sprintf (local_rec.detSumDesc, "Detailed");
		else
			sprintf (local_rec.detSumDesc, "Summary ");

		DSP_FLD ("detSumDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("showSman"))
	{
		if (dflt_used)
			sprintf (local_rec.showSman, "N");

		switch (local_rec.showSman[0])
		{
			case 'Y':
				sprintf (local_rec.showSmanDesc, "Yes");
				break;
			case 'N':
				sprintf (local_rec.showSmanDesc, "No ");
				break;
		}

		DSP_FLD ("showSmanDesc");
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
  
        if (!ValidMonthYearEnd (local_rec.yend))
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

int
heading (
 int    scn)
{
	if (restart)
    {
		return (EXIT_FAILURE);
    }

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	sprintf (err_str,   " Sales Analysis by Sales Area ");

	rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);

	move (0,1);
	line (80);

	if (local_rec.yendEnabled)
	{
		box (0, 3, 80, 14);
		move (1, 14);
		line (78);
	}
	else
	{
		box (0, 3, 80, 12);
	}

	move (1, 5);
	line (78);
	move (1, 8);
	line (78);
	move (1, 12);
	line (78);

	move (0, 20);
	line (80);

	print_at (21, 1, "Co : %s - %s", comm_rec.co_no, comm_rec.co_name);
	move (0,22);
	line (80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
SrchEsmr (char *key_val)
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
		cc = save_rec (esmr_rec.est_no, esmr_rec.name);
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
SrchExaf (
 char	*key_val)
{
	work_open ();
	save_rec ("#Area", "#Description ");

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", key_val);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp  (exaf_rec.co_no, comm_rec.co_no) &&
	       !strncmp (exaf_rec.area_code, key_val, strlen (key_val)))
	{
		cc = save_rec (exaf_rec.area_code, exaf_rec.area);
		if (cc)
			break;

		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", temp_str);
	cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

void
SrchExcf (
 char	*key_val)
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

/*===================                                    
| Validate Month    |                       
===================*/                       
int                                                                           
ValidMonthYearEnd (        
 int    month)                                           
{   
    int valid = FALSE;                                                         
        
    if (month > 0 && month < 13)
    {   
        valid = TRUE;
    }   
        
    return (valid);
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

            vars[label ("lpno")].row = 15;
            vars[label ("back")].row = 16;
            vars[label ("backDesc")].row = 16;
            vars[label ("onight")].row = 17;
            vars[label ("onightDesc")].row = 17;
   
            FLD ("yend") = YES;
            FLD ("yendDesc") = NA;
        }
    }

    return (mnthYEnd);
}
