/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( sa_salesm.i.c )                                  |
|  Program Desc  : ( Sales Analysis By Customer & Item Input         )|
|                : ( Program.                                        )|
| $Id: sa_sadf.i.c,v 5.3 2002/07/17 09:57:47 scott Exp $
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, cumr, sapc, esmr, excl, excf, exsf    |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 25/10/88         |
|---------------------------------------------------------------------|
|  Date Modified : 01/12/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : 16/12/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : 22/12/88        | Modified  by  : B.C.Lim.         |
|  Date Modified : 11/01/89        | Modified  by  : B.C.Lim.         |
|  Date Modified : 13/01/89        | Modified  by  : B.C.Lim.         |
|  Date Modified : 05/07/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 01/11/89        | Modified  by  : Fui Choo Yap.    |
|  Date Modified : 06/06/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 10/10/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 12/12/91        | Modified  by  : Campbell Mander. |
|  Date Modified : 15/04/92        | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen.|
|  Date Modified : (04/09/97)      | Modified  by : Leah Manibog.     |
|  Date Modified : (14/10/97)      | Modified  by : Leah Manibog.     |
|  Date Modified : (15/10/97)      | Modified  by : Leah Manibog.     |
|  Date Modified : (04/11/1997)    | Modified  by : Jiggs Veloz.      |
|                                                                     |
|  Comments      : Change program for OCT.                            |
|                : Add pad_num to dbt_no.                             |
|                : Change default ALL to All.                         |
|                : Add new option i.e SALESMAN to the program.        |
|                : Change the heading Product to Item, change the cumr|
|                : search according to the br_no entered instead of   |
|                : using the comm_est_no as usual.                    |
|                : Change the Customer's Barnch # default value to    |
|                : current comm_est no instead of All Branch because  |
|                : can't do validate/search for all branch (standard   |
|                : search takes comm_est for search).                 |
|     (05/07/89) : Prompt for option to print Cost-Margin.            |
|     (01/11/89) : Change BR default to current branch.               |
|     (06/06/91) : Modified for DFH. No `output to` argument received |
|                : anymore as display and print are now two separate  |
|                : programs.                                          |
|     (10/10/91) : SC 5983 DPL. Branch search was not functioning.    |
|     (12/12/91) : SC 6138 PSM. Added optional selection by category  |
|                : for item by cust reports.                          |
|     (15/04/92) : Upper argument to BY PRODUCT/BY ITEM (ITEM SEL.)   |
|                : was incorrect.                                     |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (04/09/97)    : Updated for Multilingual Conversion.				  | 
|  (14/10/97)    : Fixed MLDB error.								  | 
|  (15/10/97)    : Fixed bug at RunProgram.							  | 
|  (04/11/1997)  : Changed no_lps () to valid_lp ().					  | 
|                                                                     |
| $Log: sa_sadf.i.c,v $
| Revision 5.3  2002/07/17 09:57:47  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:02  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:33  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:47  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:59  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:28  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.14  2000/07/14 03:30:57  scott
| Updated to add app.schema
| Cleaned up code and added standard stock search.
|
| Revision 1.13  2000/04/25 09:00:41  marnie
| SC2811 - LSANZ16251 - Modified to correct the printing of Item Sales by Customer Report.
|
| Revision 1.12  2000/03/07 09:12:14  ambhet
| SC# 16081 / 2614 - Modified to correct the background and overnight hanging.
|
| Revision 1.11  1999/12/06 01:35:30  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/16 04:55:33  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.9  1999/09/29 10:12:50  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:35  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:21  scott
| Updated for read_comm (), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_sadf.i.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_sadf.i/sa_sadf.i.c,v 5.3 2002/07/17 09:57:47 scott Exp $";

#define		CATEGORY	 (by_type [0] == 'A')

#define		ITEM_SEL	 (sel_type [0] == 'I')

#include	<pslscr.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_sa_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct esmrRecord	esmr_rec;
struct excfRecord	excf_rec;
struct exclRecord	excl_rec;
struct exsfRecord	exsf_rec;

char	branchNumber [3];
char	commBranchNumber [3];
char	cbranchNo_name [41];
char	title [51];
char	br_comment [61];

int	envDbCo = 0;
int	envDbFind = 0;
int	by_cust = TRUE;
int	by_display = TRUE;
int	alt_title = FALSE;
char	by_type [2];
char	sel_type [2];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	br_no [4];
	char	br_name [41];
	char	s_class [2];
	char	e_class [2];
	char	s_cat [12];
	char	s_catdesc [41];
	char	e_cat [12];
	char	e_catdesc [41];
	char	s_prod [17];
	char	s_prodesc [41];
	char	e_prod [17];
	char	e_prodesc [41];
	char	s_type [4];
	char	s_typedesc [41];
	char	e_type [4];
	char	e_typedesc [41];
	char	s_cust [7];
	char	s_name [41];
	char	e_cust [7];
	char	e_name [41];
	char	det_summ [2];
	char	rep_type [9];
	char	cost_mgn [5];
	char	back [5];
	char	onight [5];
	int	lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "br_no", 4, 21, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Branch No ", br_comment, 
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no}, 
	{1, LIN, "br_name", 4, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.br_name}, 
	{1, LIN, "i_detail_summ", 5, 21, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Detailed / Summary ", " D(etailed or S(ummary ", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.det_summ}, 
	{1, LIN, "i_rep_type", 5, 42, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "Summary", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type}, 
	{1, LIN, "s_class", 7, 21, CHARTYPE, 
		"U", "          ", 
		" ", "A", "Start Class ", " Input Start Class A-Z ", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.s_class}, 
	{1, LIN, "s_cat", 8, 21, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "All", "Start Category ", " Default is All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.s_cat}, 
	{1, LIN, "s_catdesc", 8, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.s_catdesc}, 
	{1, LIN, "e_class", 9, 21, CHARTYPE, 
		"U", "          ", 
		" ", "Z", "End Class ", " Input End Class A-Z ", 
		YES, NO, JUSTLEFT, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "", local_rec.e_class}, 
	{1, LIN, "e_cat", 10, 21, CHARTYPE, 
		"UUUUUUUUUUU", "          ", 
		" ", "All", "End Category ", " Default is All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.e_cat}, 
	{1, LIN, "e_catdesc", 10, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_catdesc}, 
	{1, LIN, "s_prod", 7, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "All", "Start Item ", " Default is All ", 
		ND, NO, JUSTLEFT, "", "", local_rec.s_prod}, 
	{1, LIN, "s_prodesc", 8, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Description ", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.s_prodesc}, 
	{1, LIN, "e_prod", 9, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "All", "End Item ", " Default is All ", 
		ND, NO, JUSTLEFT, "", "", local_rec.e_prod}, 
	{1, LIN, "e_prodesc", 10, 21, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "Description ", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.e_prodesc}, 
	{1, LIN, "cost_mgn", 11, 21, CHARTYPE, 
		"UUUU", "          ", 
		" ", "Y(es", "Print Cost-Margin", " Y(es) | N(o) ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.cost_mgn}, 
	{1, LIN, "p_lpno", 13, 21, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "p_back", 14, 21, CHARTYPE, 
		"UUUU", "          ", 
		" ", "N(o", "Background ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "p_onight", 14, 60, CHARTYPE, 
		"UUUU", "          ", 
		" ", "N(o", "Overnight ", " ", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{2, LIN, "br_no", 4, 21, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Branch No ", br_comment, 
		YES, NO, JUSTRIGHT, "", "", local_rec.br_no}, 
	{2, LIN, "br_name", 4, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.br_name}, 
	{2, LIN, "c_detail_summ", 6, 21, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Detailed / Summary ", " D(etailed or S(ummary ", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.det_summ}, 
	{2, LIN, "c_rep_type", 6, 42, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "Summary", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.rep_type}, 
	{2, LIN, "s_type", 8, 21, CHARTYPE, 
		"UUU", "          ", 
		" ", "All", "Start Cust Type ", " Default is All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.s_type}, 
	{2, LIN, "s_typedesc", 8, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "All Customer Types", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.s_typedesc}, 
	{2, LIN, "e_type", 9, 21, CHARTYPE, 
		"UUU", "          ", 
		" ", "All", "End Cust Type ", " Default is All ", 
		YES, NO, JUSTLEFT, "", "", local_rec.e_type}, 
	{2, LIN, "e_typedesc", 9, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "All Customer Types", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_typedesc}, 
	{2, LIN, "s_cust_no", 8, 21, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "All", "Start Customer ", " Default is All ", 
		ND, NO, JUSTLEFT, "", "", local_rec.s_cust}, 
	{2, LIN, "s_name", 8, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "All Customers", " ", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.s_name}, 
	{2, LIN, "e_cust_no", 9, 21, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "All", "End Customer ", " Default is All ", 
		ND, NO, JUSTLEFT, "", "", local_rec.e_cust}, 
	{2, LIN, "e_name", 9, 42, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "All Customers", " ", " ", 
		ND, NO, JUSTLEFT, "", "", local_rec.e_name}, 
	{2, LIN, "cost_mgn", 10, 21, CHARTYPE, 
		"UUUU", "          ", 
		" ", "Y(es", "Print Cost-Margin", " Y(es) | N(o) ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.cost_mgn}, 
	{2, LIN, "c_lpno", 12, 21, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.lpno}, 
	{2, LIN, "c_back", 13, 21, CHARTYPE, 
		"UUUU", "          ", 
		" ", "N(o", "Background", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{2, LIN, "c_onight", 13, 60, CHARTYPE, 
		"UUUU", "          ", 
		" ", "N(o", "Overnight", " ", 
		NO, NO, JUSTLEFT, "YN", "", local_rec.onight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes
=====================================================================*/
int 	RunProgram 			(char *);
void 	SetupProductLabel 	(void);
void 	SetupCustomerLabel 	(void);
void 	LoadDefault 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	SrchEsmr 			(char *);
void 	SrchExcf 			(char *);
void 	SrchExcl 			(char *);
void 	SrchExsf 			(char *);
int 	heading 			(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	int	scn_number = 1;

	if (argc != 5)
	{
		print_at (0,0, ML (mlSaMess720) ,argv [0]);
		print_at (1,0, ML (mlSaMess721));
		print_at (2,0, ML (mlSaMess722));
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	if (!strcmp (argv [1], "sa_sadf_prt"))
    {
		by_display = FALSE;
    }
	else
	{
		FLD ("p_lpno") 		= ND;
		FLD ("c_lpno") 		= ND;
		FLD ("p_onight") 	= ND;
		FLD ("c_onight") 	= ND;
		FLD ("p_back") 		= ND;
		FLD ("c_back") 		= ND;
	}

	switch (argv [2] [0])
	{
	case 'C':
	case 'c':
		by_cust = TRUE;
		break;

	case 'P':
	case 'p':
		by_cust = FALSE;
		break;

	default:
		print_at (0,0, ML (mlSaMess723) ,argv [0]);
        return (EXIT_FAILURE);
	}

	sprintf (sel_type, "%-1.1s", argv [4]);

	if (by_cust)
	{
		scn_number = 2;
		switch (argv [3] [0])
		{
		case 'C':
		case 'c':
			strcpy (by_type,"C");
			SetupCustomerLabel ();
			break;

		case 'T':
		case 't':
			strcpy (by_type,"T");
			break;

		case 'S':
		case 's':
			strcpy (by_type,"S");
			vars [label ("s_type")].mask 		= "UU";
			vars [label ("s_type")].just 		= JUSTRIGHT;
			vars [label ("s_type")].prmpt 		= "Start Salesman ";
			vars [label ("s_typedesc")].deflt 	= "All Salesmen ";
			vars [label ("e_type")].mask 		= "UU";
			vars [label ("e_type")].just 		= JUSTRIGHT;
			vars [label ("e_type")].prmpt 		= "End Salesman ";
			vars [label ("e_typedesc")].deflt 	= "All Salesmen ";

			break;

		default:
			print_at (0,0, ML (mlSaMess724) ,argv [0]);
            return (EXIT_FAILURE);
		}
	}
	else
	{
		scn_number = 1;
		switch (argv [3] [0])
		{
		case 'I':
		case 'i':
			if (ITEM_SEL)
			{
				strcpy (by_type,"I");
				SetupProductLabel ();
			}
			else
			{
				strcpy (by_type,"A");
				alt_title = TRUE;
			}
			break;

		case 'A':
		case 'a':
			strcpy (by_type,"A");
			break;

		case 'C':
		case 'c':
			if (sel_type [0] == 'I')
			{
				strcpy (by_type,"C");
				SetupProductLabel ();
			}
			else
			{
				if (sel_type [0] == 'C')
					strcpy (by_type,"A");
			}
			break;

		default:
			print_at (0,0, ML (mlSaMess725) ,argv [0]);
            return (EXIT_FAILURE);
		}
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (scn_number);
               
	OpenDB ();

	strcpy (commBranchNumber,comm_rec.est_no);
	strcpy (cbranchNo_name,comm_rec.est_name);

	envDbCo		= atoi (get_env ("DB_CO"));
	envDbFind	= atoi (get_env ("DB_FIND"));

	strcpy (branchNumber,(!envDbCo) ? " 0" : comm_rec.est_no);
	
	swide ();
	clear ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/

	/*-----------------------
	| Reset control flags . |
	-----------------------*/
	entry_exit	= FALSE;
	edit_exit	= FALSE;
	prog_exit	= FALSE;
	restart		= FALSE;
	init_ok		= TRUE;
	search_ok	= TRUE;
	init_vars (scn_number);
	crsr_on ();

	LoadDefault ();

	/*------------------------------
	| Edit screen 1 linear input . |	
	------------------------------*/
	heading (scn_number);
	scn_display (scn_number);
	edit (scn_number);
	if (restart)
	{
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	if (RunProgram (argv [1]) == 1)
	{
		return (EXIT_SUCCESS);
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
RunProgram (
 char*  prog_name)
{
	char	start_field [17];
	char	end_field [17];
	char	cust_prod [2];
	char	printer_no [3];
	char	cat_item_type_cust [2];

	sprintf (printer_no,"%2d",local_rec.lpno);

	if (by_cust)
	{
		strcpy (cust_prod,"C");
		switch (by_type [0])
		{
		case	'T' :
			/* Customer Types */
			strcpy (cat_item_type_cust,"T");
			if (!strcmp (local_rec.s_type,"All")) 
				strcpy (start_field,"                ");
			else
				sprintf (start_field,"%-3.3s             ",local_rec.s_type);
			if (!strcmp (local_rec.e_type,"All")) 
				strcpy (end_field,"~~~             ");
			else
				sprintf (end_field,"%-3.3s             ",local_rec.e_type);
			break;

		case	'S' :
			/* Salesmen */
			strcpy (cat_item_type_cust,"S");
			if (!strcmp (local_rec.s_type,"All")) 
				strcpy (start_field,"                ");
			else
				sprintf (start_field,"%-2.2s              ",local_rec.s_type);
			if (!strcmp (local_rec.e_type,"All")) 
				strcpy (end_field,"~~              ");
			else
				sprintf (end_field,"%-2.2s              ",local_rec.e_type);
			break;

		case	'C' :
			/* Customers */
			strcpy (cat_item_type_cust,"C");
			if (!strcmp (local_rec.s_cust,"All   ")) 
				strcpy (start_field,"                ");
			else
				sprintf (start_field,"%-6.6s          ",local_rec.s_cust);
			if (!strcmp (local_rec.e_cust,"All   ")) 
				strcpy (end_field,"~~~~~~          ");
			else
				sprintf (end_field,"%-6.6s          ",local_rec.e_cust);
			break;
		}
	}
	else
	{
		strcpy (cust_prod,"P");
		if (CATEGORY)
		{
			if (alt_title)
				strcpy (cat_item_type_cust,"I");
			else
				strcpy (cat_item_type_cust,"A");

			if (sel_type [0] == 'C')
				strcpy (cat_item_type_cust,"C");

			if (!strcmp (local_rec.s_cat,"All        ")) 
				sprintf (start_field,"%1.1s%-11.11s    ",local_rec.s_class," ");
			else
				sprintf (start_field,"%1.1s%-11.11s    ",local_rec.s_class,local_rec.s_cat);
			if (!strcmp (local_rec.e_cat,"All        ")) 
				sprintf (end_field,"%1.1s~~~~~~~~~~~    ",local_rec.e_class);
			else
				sprintf (end_field,"%1.1s%-11.11s    ",local_rec.e_class,local_rec.e_cat);
		}
		else
		{
			if (sel_type [0] == 'I')
				strcpy (cat_item_type_cust,"C");
			else
				strcpy (cat_item_type_cust,"I");
			if (!strcmp (local_rec.s_prod,"All             ")) 
				strcpy (start_field,"                ");
			else
				sprintf (start_field,"%-16.16s",local_rec.s_prod);
			if (!strcmp (local_rec.e_prod,"All             ")) 
				strcpy (end_field,"~~~~~~~~~~~~~~~~");
			else
				sprintf (end_field,"%-16.16s",local_rec.e_prod);
		}
	}

	rset_tty ();

	clear ();

	fflush (stdout);
	/*--------------------------------
	| Test for Overnight Processing. | 
	--------------------------------*/
	if (!by_display && local_rec.onight [0] == 'Y') 
	{
		if (fork () == 0)
        {
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				cust_prod,
				local_rec.det_summ,
				cat_item_type_cust,  	/*  argv [3]  */
				sel_type,
				start_field,
				end_field,
				printer_no,
				local_rec.br_no,
				local_rec.cost_mgn,
				"sa_pscust", (char *)0);
        }
	}
	/*------------------------------------
	| Test for forground or background . |
	------------------------------------*/
	else if (!by_display && local_rec.back [0] == 'Y') 
	{
		if (fork () == 0)
        {
			execlp (prog_name,
                    prog_name,
                    cust_prod,
                    local_rec.det_summ,
                    cat_item_type_cust,  	/*  argv [3]  */
                    sel_type,
                    start_field,
                    end_field,
                    printer_no,
                    local_rec.br_no,
                    local_rec.cost_mgn,
                    (char *)0);
        }
	}
	else 
	{
		execlp (prog_name,
                prog_name,
		        cust_prod,
		        local_rec.det_summ,
		        cat_item_type_cust, 	/*  argv [3]  */
		        sel_type,
		        start_field,
		        end_field,
		        printer_no,
		        local_rec.br_no,
		        local_rec.cost_mgn,
                (char *)0);
		return (EXIT_FAILURE);
	}
    shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
SetupProductLabel (void)
{
	FLD ("s_class")   = ND;
	FLD ("s_cat")     = ND;
	FLD ("s_catdesc") = ND;
	FLD ("e_class")   = ND;
	FLD ("e_cat")     = ND;
	FLD ("e_catdesc") = ND;
	FLD ("s_prod")    = YES;
	FLD ("s_prodesc") = NA;
	FLD ("e_prod")    = YES;
	FLD ("e_prodesc") = NA;
}

void
SetupCustomerLabel (void)
{
	FLD ("s_type")     = ND;
	FLD ("s_typedesc") = ND;
	FLD ("e_type")     = ND;
	FLD ("e_typedesc") = ND;
	FLD ("s_cust_no")  = YES;
	FLD ("s_name")     = NA;
	FLD ("e_cust_no")  = YES;
	FLD ("e_name")     = NA;
}

/*=====================================================================
| Load in default values.                                             |
=====================================================================*/
void
LoadDefault (void)
{
	strcpy (br_comment,"Default To Current Branch; Type <A> for All Branches");
	sprintf (local_rec.br_no,"%-3.3s",comm_rec.est_no);
	sprintf (local_rec.br_name,"%-40.40s",comm_rec.est_name);

	strcpy (local_rec.det_summ,"S");
	strcpy (local_rec.rep_type,"Summary ");
	strcpy (local_rec.cost_mgn,"Y(es");

	if (by_cust)
	{
		switch (by_type [0])
		{
		case	'T' :
			sprintf (title,"%-50.50s", ML (mlSaMess009));

			strcpy (local_rec.s_type,"All");
			strcpy (local_rec.e_type,"All");
			sprintf (local_rec.s_typedesc,"%-40.40s","All Customer Types ");
			sprintf (local_rec.e_typedesc,"%-40.40s","All Customer Types ");
			break;

		case	'S' :
			sprintf (title,"%-50.50s", ML (mlSaMess011));

			strcpy (local_rec.s_type,"All");
			strcpy (local_rec.e_type,"All");
			sprintf (local_rec.s_typedesc,"%-40.40s","All Salesmen ");
			sprintf (local_rec.e_typedesc,"%-40.40s","All Salesmen ");
			break;

		case	'C' :
			sprintf (title,"%-50.50s", ML (mlSaMess007));

			strcpy (local_rec.s_cust,"All   ");
			strcpy (local_rec.e_cust,"All   ");
			sprintf (local_rec.s_name,"%-40.40s","All Customers ");
			sprintf (local_rec.e_name,"%-40.40s","All Customers ");
			break;
		}
	}
	else
	{
		if (CATEGORY)
		{
			if (alt_title)
				sprintf (title,"%-50.50s", ML (mlSaMess015));
			else
				sprintf (title,"%-50.50s", ML (mlSaMess012));
			
			strcpy (local_rec.s_cat,"All        ");
			strcpy (local_rec.e_cat,"All        ");
			strcpy (local_rec.s_class,"A");
			strcpy (local_rec.e_class,"Z");
			sprintf (local_rec.s_catdesc,"%-40.40s","All Categories ");
			sprintf (local_rec.e_catdesc,"%-40.40s","All Categories ");
		}
		else
		{
			sprintf (title,"%-50.50s", ML (mlSaMess015));

			strcpy (local_rec.s_prod,"All             ");
			strcpy (local_rec.e_prod,"All             ");
			sprintf (local_rec.s_prodesc,"%-40.40s","All Items ");
			sprintf (local_rec.e_prodesc,"%-40.40s","All Items ");
		}
	}

	if (!by_display)
	{
		local_rec.lpno = 1;
		strcpy (local_rec.back,"N(o");
		strcpy (local_rec.onight,"N(o");
	}
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm ( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec ("excf",excf_list,EXCF_NO_FIELDS,"excf_id_no");
	open_rec ("excl",excl_list,EXCL_NO_FIELDS,"excl_id_no");
	open_rec ("exsf",exsf_list,EXSF_NO_FIELDS,"exsf_id_no");
	if (envDbFind == 0)
		open_rec ("cumr",cumr_list,CUMR_NO_FIELDS,"cumr_id_no");
	else
		open_rec ("cumr",cumr_list,CUMR_NO_FIELDS,"cumr_id_no3");
	open_rec ("inmr",inmr_list,INMR_NO_FIELDS,"inmr_id_no");
	open_rec ("esmr",esmr_list,ESMR_NO_FIELDS,"esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose ("excf");
	abc_fclose ("excl");
	abc_fclose ("exsf");
	abc_fclose ("cumr");
	abc_fclose ("inmr");
	abc_fclose ("esmr");
	abc_dbclose ("data");
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("br_no"))
	{
		if (dflt_used)
		{
			strcpy (br_comment,"Default To Current Branch; Type <A> for All Branches");
			sprintf (local_rec.br_no,"%-3.3s",commBranchNumber);
			strcpy (local_rec.br_name,cbranchNo_name);
			strcpy (branchNumber,local_rec.br_no);

			DSP_FLD ("br_name");

			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.br_no," A") == 0)
		{
			strcpy (local_rec.br_no,"All");
			strcpy (local_rec.br_name,"All Branches");
			DSP_FLD ("br_name");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
               SrchEsmr (temp_str);
		       return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no,comm_rec.co_no);
		sprintf (esmr_rec.est_no,"%2.2s",local_rec.br_no);
		cc = find_rec ("esmr",&esmr_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (envDbCo == 0)
			strcpy (branchNumber," 0");
		else
		{
			strcpy (branchNumber,local_rec.br_no);
			strcpy (comm_rec.est_no,local_rec.br_no);
		}

		sprintf (local_rec.br_no,"%2.2s ",esmr_rec.est_no);
		strcpy (local_rec.br_name,esmr_rec.est_name);
		DSP_FLD ("br_no");
		DSP_FLD ("br_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("c_detail_summ") || LCHECK ("i_detail_summ"))
	{
		if (by_cust)
		{
			switch (by_type [0])
			{
			case	'T' :
				if (local_rec.det_summ [0] == 'S')
					sprintf (title,"%-50.50s", ML (mlSaMess009));
				else
					sprintf (title,"%-50.50s", ML (mlSaMess008));

				break;

			case	'S' :
				if (local_rec.det_summ [0] == 'S')
					sprintf (title,"%-50.50s", ML (mlSaMess011));
				else
					sprintf (title,"%-50.50s", ML (mlSaMess010));

				break;

			case	'C' :
				if (local_rec.det_summ [0] == 'S')
					sprintf (title,"%-50.50s", ML (mlSaMess007));
				else
					sprintf (title,"%-50.50s", ML (mlSaMess006));
				break;
			}
		}
		else
		{
			if (CATEGORY)
			{
				if (local_rec.det_summ [0] == 'S')
					sprintf (title,"%-50.50s", ML (mlSaMess012));
				else
					sprintf (title,"%-50.50s", ML (mlSaMess017));
			}
			else
			{
				if (local_rec.det_summ [0] == 'S')
					sprintf (title,"%-50.50s", ML (mlSaMess015));
				else
					sprintf (title,"%-50.50s", ML (mlSaMess014));
			}
		}

		strcpy (local_rec.rep_type,
			 (local_rec.det_summ [0] == 'D') ? "Detail" : "Summary");

		display_field (field + 1);
		move (20,0);
		cl_line ();
		rv_pr (title, (132 - strlen (clip (title)) + 2) / 2,0,1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_cat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.s_cat,"All        ");
			strcpy (local_rec.e_cat,"All        ");
			strcpy (local_rec.s_catdesc,"All Categories");
			strcpy (local_rec.e_catdesc,"All Categories");
			FLD ("e_cat") = NA;
			DSP_FLD ("s_cat");
			DSP_FLD ("e_cat");
			DSP_FLD ("s_catdesc");
			DSP_FLD ("e_catdesc");

			return (EXIT_SUCCESS);
		}
		FLD ("e_cat") = YES;
		
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (prog_status != ENTRY && 
		    strcmp (local_rec.e_cat,"All        ") && 
		    strcmp (local_rec.s_cat,local_rec.e_cat) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.s_cat);
		cc = find_rec ("excf",&excf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.s_catdesc,excf_rec.cat_desc);
		DSP_FLD ("s_catdesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_cat"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.e_cat,"All        ");
			strcpy (local_rec.e_catdesc,"All Categories");
			DSP_FLD ("e_cat");
			DSP_FLD ("e_catdesc");

			return (EXIT_SUCCESS);
		}
		
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.s_cat,"All        ") == 0 && 
		    strcmp (local_rec.e_cat,"All        "))
		{
			print_mess (ML (mlStdMess169));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (excf_rec.co_no,comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",local_rec.e_cat);
		cc = find_rec ("excf",&excf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML (mlStdMess004));
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.s_cat, local_rec.e_cat) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.e_catdesc,excf_rec.cat_desc);
		DSP_FLD ("e_catdesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_prod")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.s_prod,"%-16.16s","All             ");
			sprintf (local_rec.e_prod,"%-16.16s","All             ");
			sprintf (local_rec.s_prodesc,"%-40.40s","All Items");
			sprintf (local_rec.e_prodesc,"%-40.40s","All Items");
			FLD ("e_prod") = NA;
			DSP_FLD ("s_prod");
			DSP_FLD ("e_prod");
			DSP_FLD ("s_prodesc");
			DSP_FLD ("e_prodesc");

			return (EXIT_SUCCESS);
		}
		FLD ("e_prod") = YES;

		if (prog_status != ENTRY && 
		    strcmp (local_rec.e_prod,"All             ") && 
		    strcmp (local_rec.s_prod,local_rec.e_prod) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.s_prod, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.s_prod);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.s_prodesc,inmr_rec.description);
		DSP_FLD ("s_prodesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_prod")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.e_prod,"%-16.16s","All             ");
			sprintf (local_rec.e_prodesc,"%-40.40s","All Items");
			DSP_FLD ("e_prod");
			DSP_FLD ("e_prodesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (strcmp (local_rec.s_prod,"All             ") == 0 && 
		    strcmp (local_rec.e_prod,"All             "))
		{
			print_mess (ML (mlStdMess133));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.e_prod, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.e_prod);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (strcmp (local_rec.s_prod,local_rec.e_prod) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.e_prodesc,inmr_rec.description);
		DSP_FLD ("e_prodesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_type")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.s_type,"%-3.3s","All");
			sprintf (local_rec.e_type,"%-3.3s","All");
			if (by_type [0] == 'T')
			{
				sprintf (local_rec.s_typedesc, "%-40.40s","All Customer Types");
				sprintf (local_rec.e_typedesc, "%-40.40s","All Customer Types");
			}
			else
			{
				sprintf (local_rec.s_typedesc, "%-40.40s", "All Salesmen");
				sprintf (local_rec.e_typedesc, "%-40.40s", "All Salesmen");
			}

			FLD ("e_type") = NA;
			DSP_FLD ("s_type");
			DSP_FLD ("e_type");
			DSP_FLD ("s_typedesc");
			DSP_FLD ("e_typedesc");

			return (EXIT_SUCCESS);
		}
		FLD ("e_type") = YES;

		if (prog_status != ENTRY && 
		    strcmp (local_rec.e_type,"All") && 
		    strcmp (local_rec.s_type,local_rec.e_type) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			if (by_type [0] == 'T')
				SrchExcl (temp_str);
			else
				SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (by_type [0] == 'T')
		{
			strcpy (excl_rec.co_no,comm_rec.co_no);
			sprintf (excl_rec.class_type,
				"%3.3s",
				local_rec.s_type);

			cc = find_rec ("excl",&excl_rec,COMPARISON,"r");	
			if (cc)
			{
				print_mess (ML (mlStdMess170));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.s_typedesc,excl_rec.class_desc);
		}
		else
		{
			strcpy (exsf_rec.co_no,comm_rec.co_no);
			sprintf (exsf_rec.salesman_no, "%-2.2s", local_rec.s_type);

			cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");	
			if (cc)
			{
				print_mess (ML (mlStdMess135));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.s_type, "%2.2s ", exsf_rec.salesman_no);
			strcpy (local_rec.s_typedesc,exsf_rec.salesman);
		}

		DSP_FLD ("s_type");
		DSP_FLD ("s_typedesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_type")) 
	{
		if (dflt_used)
		{
			sprintf (local_rec.e_type,"%-3.3s","All");
			if (by_type [0] == 'T')
				sprintf (local_rec.e_typedesc, "%-40.40s", "All Salesmen");
			else
				sprintf (local_rec.e_typedesc, "%-40.40s", "All Salesmen");

			DSP_FLD ("e_type");
			DSP_FLD ("e_typedesc");

			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			if (by_type [0] == 'T')
				SrchExcl (temp_str);
			else
				SrchExsf (temp_str);

			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.s_type,"All") == 0 && 
		    strcmp (local_rec.e_type,"All"))
		{
			if (by_type [0] == 'T')
				strcpy (err_str, ML (mlStdMess171));
			else
				strcpy (err_str, ML (mlStdMess172));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (by_type [0] == 'T')
		{
			strcpy (excl_rec.co_no,comm_rec.co_no);
			sprintf (excl_rec.class_type,"%3.3s",local_rec.e_type);
			cc = find_rec ("excl",&excl_rec,COMPARISON,"r");	
			if (cc)
			{
				print_mess (ML (mlStdMess170));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.e_typedesc,excl_rec.class_desc);
		}
		else
		{
			strcpy (exsf_rec.co_no,comm_rec.co_no);
			sprintf (exsf_rec.salesman_no,"%-2.2s",local_rec.e_type);
			cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");	
			if (cc)
			{
				print_mess (ML (mlStdMess135));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.e_type,"%2.2s ",exsf_rec.salesman_no);
			strcpy (local_rec.e_typedesc,exsf_rec.salesman);
		}

		if (strcmp (local_rec.s_type,local_rec.e_type) > 0)
		{
			if (by_type [0] == 'T')
				sprintf (err_str, ML (mlStdMess017));
			else
				sprintf (err_str, ML (mlStdMess017));

			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("e_type");
		DSP_FLD ("e_typedesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_cust_no"))
	{
		if (SRCH_KEY)
		{
		   CumrSearch (comm_rec.co_no, branchNumber, temp_str);
		   return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.s_cust,"%-6.6s","All   ");
			sprintf (local_rec.e_cust,"%-6.6s","All   ");
			sprintf (local_rec.s_name,"%-40.40s","All Customers");
			sprintf (local_rec.e_name,"%-40.40s","All Customers");
			FLD ("e_cust_no") = NA;
			DSP_FLD ("s_cust_no");
			DSP_FLD ("s_name");
			DSP_FLD ("e_cust_no");
			DSP_FLD ("e_name");

			return (EXIT_SUCCESS);
		}
		FLD ("e_cust_no") = YES;

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		sprintf (cumr_rec.dbt_no,"%-6.6s",pad_num (local_rec.s_cust));
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (prog_status != ENTRY && 
		    strcmp (local_rec.e_cust,"All   ") && 
		    strcmp (local_rec.s_cust,local_rec.e_cust) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.s_name,cumr_rec.dbt_name);
		DSP_FLD ("s_cust_no");
		DSP_FLD ("s_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_cust_no"))
	{
		if (SRCH_KEY)
		{
		   CumrSearch (comm_rec.co_no, branchNumber, temp_str);
		   return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			sprintf (local_rec.e_cust,"%-6.6s","All   ");
			sprintf (local_rec.e_name,"%-40.40s","All Customers");
			DSP_FLD ("e_cust_no");
			DSP_FLD ("e_name");

			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.s_cust,"All   ") == 0 && 
		    strcmp (local_rec.e_cust,"All   "))
		{
			print_mess (ML (mlStdMess173));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		sprintf (cumr_rec.dbt_no,"%-6.6s",pad_num (local_rec.e_cust));
		cc = find_rec ("cumr",&cumr_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.s_cust,local_rec.e_cust) > 0)
		{
			errmess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.e_name,cumr_rec.dbt_name);
		DSP_FLD ("e_cust_no");
		DSP_FLD ("e_name");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cost_mgn"))
	{
		strcpy (local_rec.cost_mgn,
			 (local_rec.cost_mgn [0] == 'Y') ? "Y(es" : "N(o");

		display_field (field);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("c_lpno") || LCHECK ("p_lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("c_back") || LCHECK ("p_back"))
	{
		strcpy (local_rec.back,(local_rec.back [0] == 'Y') ? "Y(es" : "N(o");
		display_field (field);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("c_onight") || LCHECK ("p_onight"))
	{
		strcpy (local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o");
		display_field (field);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchEsmr (
 char*  key_val)
{
	work_open ();
	save_rec ("#Br No.    ","#Br Name           ");
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	sprintf (esmr_rec.est_no,"%2.2s",key_val);
	cc = find_rec ("esmr",&esmr_rec,GTEQ,"r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no,comm_rec.co_no) && 
	       !strncmp (esmr_rec.est_no,key_val,strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec ("esmr",&esmr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec ("esmr", &esmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in esmr During (DBFIND)",cc,PNAME);
}

void
SrchExcf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Category No","#Category Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",key_val);
	cc = find_rec ("excf",&excf_rec,GTEQ,"r");
	while (!cc && !strcmp (excf_rec.co_no,comm_rec.co_no) && 
				  !strncmp (excf_rec.cat_no,key_val,strlen (key_val)))
	{
		cc = save_rec (excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec ("excf",&excf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec ("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in excf During (DBFIND)",cc,PNAME);
}

void
SrchExcl (
 char*  key_val)
{
	work_open ();
	save_rec ("#Type","#Customer Type");
	strcpy (excl_rec.co_no,comm_rec.co_no);
	sprintf (excl_rec.class_type,"%-3.3s",key_val);
	cc = find_rec ("excl",&excl_rec,GTEQ,"r");
	while (!cc && !strcmp (excl_rec.co_no,comm_rec.co_no) && 
				  !strncmp (excl_rec.class_type,key_val,strlen (key_val)))
	{
		cc = save_rec (excl_rec.class_type,excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec ("excl",&excl_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excl_rec.co_no,comm_rec.co_no);
	sprintf (excl_rec.class_type,"%-3.3s",temp_str);
	cc = find_rec ("excl",&excl_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in excl During (DBFIND)",cc,PNAME);
}

void
SrchExsf (
 char*  key_val)
{
	work_open ();
	save_rec ("#Sm","#Salesman Name");
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",key_val);
	cc = find_rec ("exsf",&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp (exsf_rec.co_no,comm_rec.co_no) && 
				  !strncmp (exsf_rec.salesman_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.salesman_no,exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec ("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no,comm_rec.co_no);
	sprintf (exsf_rec.salesman_no,"%-2.2s",temp_str);
	cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		sys_err ("Error in exsf During (DBFIND)",cc,PNAME);
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		rv_pr (title, (132 - strlen (clip (title)) + 2) / 2,0,1);
		
		move (0,1);
		line (132);

		switch (scn)
		{
		case 1:
			move (1,6);
			line (131);
			if (by_display)
				box (0,3,132,8);
			else
			{
				box (0,3,132,11);
				move (1,12);
				line (131);
			}
			break;

		case 2:
			move (1,5);
			line (131);
			if (by_display)
			{
				box (0,3,132,7);
				move (1,7);
				line (131);
			}
			else
			{
				box (0,3,132,10);
				move (1,7);
				line (131);
				move (1,11);
				line (131);
			}
			break;
		}

		move (0,20);
		line (132);

		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

		move (0,22);
		line (132);
		/* Reset this variable for new screen NOT page */
		line_cnt = 0; 
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
