/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sa_allrep.i.c,v 5.3 2002/07/17 09:57:45 scott Exp $
|  Program Name  : (sa_allrep.i.c)
|  Program Desc  : (Selection for Sales Analysis Reports)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/04/87         |
|---------------------------------------------------------------------|
| $Log: sa_allrep.i.c,v $
| Revision 5.3  2002/07/17 09:57:45  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2002/03/04 07:59:05  scott
| ..
|
| Revision 5.1  2002/03/04 04:59:21  scott
| S/C 00791 - Sales Analysis; SAMR2-Sales Analysis (Budget); SAMR3-Sales Analysis (Normal) WINDOWS CLIENT (1) No delay in error display at fields 'Printer Number', 'Enter Lower Value, 'Enter Upper Value' CHAR-BASED / WINDOWS CLIENT (2) Range is not validated at fields 'Enter Lower Value, 'Enter Upper Value' SAMR8-Customer Two Year Analysis WINDOWS CLIENT (3) No delay in error display at fields 'Area No', 'Printer' SAMR10-Maintain Budgets WINDOWS CLIENT (4) No delay in error display in second screen, whether field is 'Salesman', 'Category', 'Area', or 'Class Type'
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_allrep.i.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_allrep.i/sa_allrep.i.c,v 5.3 2002/07/17 09:57:45 scott Exp $";

#include <pslscr.h>		
#include <ml_std_mess.h>		
#include <ml_sa_mess.h>		

#define	SALES	0
#define	CATG	1
#define	C_CATG	2
#define	A_CODE	3
#define	C_TYPE	4

#define	LOWER	label ("lower")	/* field number of lower field		*/
#define	UPPER	label ("upper")	/* field number of upper field		*/

	char 	temp [150];
	char	branchNo [3];
	int		envDbFind;
	int		envDbCo;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cudpRecord	cudp_rec;
struct exsfRecord	exsf_rec;
struct excfRecord	excf_rec;
struct cumrRecord	cumr_rec;
struct exafRecord	exaf_rec;
struct exclRecord	excl_rec;

char	*categoryMask 	= "UUUUUUUUUUU", 
		*c_categoryMask = "UUUUUU", 
		*ctypeMask 		= "UUU", 
		*otherMask 		= "UU";

#include	<get_lpno.h>

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	noPrompt [11];
	char	yesPrompt [11];
	int		printerNo;
	char	subrange [2];
	char	subrangeDesc [8];
	char	back [2];
	char	backDesc [8];
	char	onight [2];
	char	onightDesc [8];
	char	byWho [5][2];
	char	byWhoDesc [5][8];
	char	byWhat [4][2];
	char	byWhatDesc [4][8];
	char	dp_no [3];
	char	wh_no [3];
	char	v_mask [12];
	char	lower [12];
	char	lowerDesc [36];
	char	upper [12];
	char	upperDesc [36];
} local_rec;

extern	int		TruePosition;

static	struct	var	vars []	={	

	{1, LIN, "printerNo", 3, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number         ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "subrange", 3, 40, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Subrange               ", " Total By Category Subranges ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.subrange}, 
	{1, LIN, "subrange_desc", 3, 66, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.subrangeDesc}, 
	{1, LIN, "back", 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background             ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 4, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.backDesc}, 
	{1, LIN, "onight", 4, 40, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight              ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "onightDesc", 4, 66, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.onightDesc}, 
	{1, LIN, "by_sman", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Salesman            ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWho [0]}, 
	{1, LIN, "by_catg", 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Category            ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWho [1]}, 
	{1, LIN, "by_c_cat", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Customer/Category   ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWho [2]}, 
	{1, LIN, "by_area", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Area                ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWho [3]}, 
	{1, LIN, "by_ctype", 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Class Type          ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWho [4]}, 
	{1, LIN, "by_sman_desc", 6, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhoDesc [0]}, 
	{1, LIN, "by_catg_desc", 7, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhoDesc [1]}, 
	{1, LIN, "by_c_cat_desc", 8, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhoDesc [2]}, 
	{1, LIN, "by_area_desc", 9, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhoDesc [3]}, 
	{1, LIN, "by_ctype_desc", 10, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhoDesc [4]}, 
	{1, LIN, "by_co", 12, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Company             ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWhat [0]}, 

	{1, LIN, "by_br", 13, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Branch              ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWhat [1]}, 
	{1, LIN, "by_dp", 14, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Department          ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWhat [2]}, 
	{1, LIN, "by_wh", 15, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "By Warehouse           ", "Enter Y(es) or N(o). ", 
		YES, YES, JUSTRIGHT, "NY", "", local_rec.byWhat [3]}, 

	{1, LIN, "by_co_desc", 12, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhatDesc [0]}, 
	{1, LIN, "by_br_desc", 13, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhatDesc [1]}, 
	{1, LIN, "by_dp_desc", 14, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhatDesc [2]}, 
	{1, LIN, "by_wh_desc", 15, 28, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "YN", "", local_rec.byWhatDesc [3]}, 

	{1, LIN, "dep", 14, 40, CHARTYPE, 
		"AA", "          ", 
		" ", " 1", "Enter Department No    ", " ", 
		YES, NO, JUSTRIGHT, "0123456789", "", local_rec.dp_no}, 
	{1, LIN, "wh", 15, 40, CHARTYPE, 
		"AA", "          ", 
		" ", " 1", "Enter Warehouse No     ", " ", 
		YES, NO, JUSTRIGHT, "0123456789", "", local_rec.wh_no}, 
	{1, LIN, "lower", 17, 2, CHARTYPE, 
		local_rec.v_mask, "          ", 
		" ", "",  "Enter Lower Value      ", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.lower}, 
	{1, LIN, " ", 17, 40, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.lowerDesc}, 
	{1, LIN, " ", 18, 40, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.upperDesc}, 
	{1, LIN, "upper", 18, 2, CHARTYPE, 
		local_rec.v_mask, "          ", 
		" ", "", "Enter Upper Value      ", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.upper}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>
/*
 * Local Function Prototypes
 */
int 	FindAnalysis 			(void);
int 	FindRange 				(int, char *, char *);
int 	FindReport 				(void);
int 	heading 				(int);
int 	RedrawAnalysis 			(int, int);
int 	RedrawRep 				(int, int);
int 	RunProgram 				(char *);
int 	spec_valid 				(int);
void 	ChangeMask 				(int);
void 	CloseDB 				(void);
void 	CoLine 					(void);
void 	OpenDB 					(void);
void 	SetDefault 				(void);
void 	shutdown_prog 			(void);
void 	SrchExaf 				(char *);
void 	SrchExcf				(char *);
void 	SrchExcl 				(char *);
void 	SrchExsf 				(char *);

/*
 * Main Processing Routine 
 */
int
main (
 int    argc, 
 char*  argv [])
{
	if (argc != 2)
	{
		print_at (0, 0, ML (mlSaMess719) , argv [0]);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;
	
	SETUP_SCR (vars);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/
		
	OpenDB ();

	/*
	 * Reset control flags 
	 */
   	search_ok 	= TRUE;
   	entry_exit 	= TRUE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	init_vars (1);	
	SetDefault ();

	/*
	 * Edit screen 1 linear input. 
	 */
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
	rset_tty ();
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

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
}

int
spec_valid (
	int		field)
{
	/*
	 * Validate Field Selection Lpno option.	
	 */
	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection Subrange option.	
	 */
	if (LCHECK ("subrange")) 
	{
		strcpy (local_rec.subrangeDesc, (local_rec.subrange [0] == 'N') 
											? local_rec.noPrompt
											: local_rec.yesPrompt);
		DSP_FLD ("subrange_desc");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection background option. 
	 */
	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'N') 
											? local_rec.noPrompt
											: local_rec.yesPrompt);
		DSP_FLD ("backDesc");

		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.onight, "N");
			strcpy (local_rec.onightDesc, local_rec.noPrompt);
			DSP_FLD ("onight");
			DSP_FLD ("onightDesc");
		}
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection overnight option. 
	 */
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, (local_rec.onight [0] == 'N') 
										? local_rec.noPrompt 
										: local_rec.yesPrompt);
		DSP_FLD ("onightDesc");

		if (local_rec.onight [0] == 'Y')
		{
			strcpy (local_rec.back, "N");
			strcpy (local_rec.backDesc, local_rec.noPrompt);
			DSP_FLD ("back");
			DSP_FLD ("backDesc");
		}
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_sman"))
	{
		RedrawRep (field, 0);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_catg"))
	{
		RedrawRep (field, 1);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_c_cat"))
	{
		RedrawRep (field, 2);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_area"))
	{
		RedrawRep (field, 3);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_ctype"))
	{
		RedrawRep (field, 4);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_co"))
	{
		RedrawAnalysis (field, 0);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_br"))
	{
		RedrawAnalysis (field, 1);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_dp"))
	{
		RedrawAnalysis (field, 2);
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("by_wh"))
	{
		RedrawAnalysis (field, 3);
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Department Number Input. 
	 */
	if (LCHECK ("dep"))
	{
		strcpy (cudp_rec.co_no, comm_rec.co_no);
		strcpy (cudp_rec.br_no, comm_rec.est_no);
		strcpy (cudp_rec.dp_no, local_rec.dp_no);

		cc = find_rec (cudp, &cudp_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess084));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		CoLine ();
		return (EXIT_SUCCESS);
	}
		
	/*
	 * Validate Warehouse Number Input. 
	 */
	if (LCHECK ("wh"))
	{
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no, local_rec.wh_no);

		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess100));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
		
	if (LCHECK ("lower"))
	{
		cc = FindRange (field, local_rec.lower, local_rec.lowerDesc);
		if (cc)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
		}
		return (cc);
	}

	if (LCHECK ("upper"))
	{
		cc = FindRange (field, local_rec.upper, local_rec.upperDesc);
		if (cc)
		{
			print_mess (ML (mlStdMess006));
			sleep (sleepTime);
		}
		return (cc);
	}
	return (EXIT_SUCCESS);
}

int
FindRange (
	int    field, 
	char*  fld_value, 
	char*  fld_desc)
{
	int	dis_fld = (field == LOWER) ? field + 1 : field - 1;

	switch (FindReport ())
	{
	case SALES:
		if (SRCH_KEY)
		{
			SrchExsf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exsf_rec.co_no, comm_rec.co_no);
		sprintf (exsf_rec.salesman_no, "%2.2s", fld_value);
		strcpy ((field == LOWER) ? local_rec.lower : local_rec.upper, exsf_rec.salesman_no);
		cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
		sprintf (fld_desc, "%-35.35s", (!cc) ? exsf_rec.salesman : " ");
		break;

	case CATG:
		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", fld_value);
		strcpy ((field == LOWER) ? local_rec.lower : local_rec.upper, excf_rec.cat_no);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		sprintf (fld_desc, "%-35.35s", (!cc) ? excf_rec.cat_desc : " ");
		break;

	case C_CATG:
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", fld_value);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		strcpy ((field == LOWER) ? local_rec.lower : local_rec.upper, cumr_rec.dbt_no);
		sprintf (fld_desc, "%-35.35s", (!cc) ? cumr_rec.dbt_name : " ");
		break;

	case A_CODE:
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (exaf_rec.co_no, comm_rec.co_no);
		sprintf (exaf_rec.area_code, "%2.2s", fld_value);
		strcpy ((field == LOWER) ? local_rec.lower : local_rec.upper, exaf_rec.area_code);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		sprintf (fld_desc, "%-35.35s", (!cc) ? exaf_rec.area : " ");
		break;

	case C_TYPE:
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (excl_rec.co_no, comm_rec.co_no);
		sprintf (excl_rec.class_type, "%-3.3s", fld_value);
		strcpy ((field == LOWER) ? local_rec.lower : local_rec.upper, excl_rec.class_type);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		sprintf (fld_desc, "%-35.35s", (!cc) ? excl_rec.class_desc : " ");
		break;
	}
	if (prog_status != ENTRY && strcmp (local_rec.lower,local_rec.upper) > 0 && field == LOWER)
	{
		errmess (ML (mlStdMess006));
		sleep (sleepTime);
		return (EXIT_FAILURE); 
	}

	if (strcmp (local_rec.lower,local_rec.upper) > 0 && field == UPPER)
	{
		errmess (ML (mlStdMess006));
		sleep (sleepTime);
		return (EXIT_FAILURE); 
	}
	display_field (field);
	display_field (dis_fld);
	return (cc);
}

int
RedrawRep (
 int    field, 
 int    indx)
{
	int	i;
	int	option;

	if (local_rec.byWho [indx][0] == 'N')
		option = FindReport ();
	else
		option = indx;

	ChangeMask (option);

	for (i = 0;i < 5;i++)
	{
		if (i == option)
		{
			strcpy (local_rec.byWho [i], "Y");
			strcpy (local_rec.byWhoDesc [i], local_rec.yesPrompt);
		}
		else
		{
			strcpy (local_rec.byWho [i], "N");
			strcpy (local_rec.byWhoDesc [i], local_rec.noPrompt);
		}
		display_field (field + i - indx);
		display_field ((field + i - indx) + 5);
	}
	return (EXIT_SUCCESS);
}

int
RedrawAnalysis (
 int    field, 
 int    indx)
{
	int	i;
	int	option;

	if (local_rec.byWhat [indx][0] == 'N')
		option = FindAnalysis ();
	else
		option = indx;

	for (i = 0;i < 4;i++)
	{
		if (i == option)
		{
			strcpy (local_rec.byWhat [i], "Y");
			strcpy (local_rec.byWhatDesc [i], local_rec.yesPrompt);
		}
		else
		{
			strcpy (local_rec.byWhat [i], "N");
			strcpy (local_rec.byWhatDesc [i], local_rec.noPrompt);
		}
		display_field (field + i - indx);
		display_field ((field + i - indx) + 4);
	}
	return (EXIT_SUCCESS);
}

void
ChangeMask (
 int    file_id)
{
	char	*new_mask = (char *) 0;

	switch (file_id)
	{
	case SALES:
	case A_CODE:
		vars [LOWER].just = JUSTRIGHT;
		vars [UPPER].just = JUSTRIGHT;
		new_mask = otherMask;
		break;

	case CATG:
		vars [LOWER].just = JUSTLEFT;
		vars [UPPER].just = JUSTLEFT;
		new_mask = categoryMask;
		break;

	case C_CATG:
		vars [LOWER].just = JUSTLEFT;
		vars [UPPER].just = JUSTLEFT;
		new_mask = c_categoryMask;
		break;

	case C_TYPE:
		vars [LOWER].just = JUSTLEFT;
		vars [UPPER].just = JUSTLEFT;
		new_mask = ctypeMask;
		break;
	}

	strcpy (local_rec.v_mask, new_mask);

#ifdef GVISION
	MaskChanged (LOWER);
	MaskChanged (UPPER);
#endif	/* GVISION */

	strcpy (local_rec.lower, string (11, " "));
	display_field (LOWER);
	strcpy (local_rec.lower, string (strlen (new_mask), " "));
	display_field (LOWER);

	sprintf (local_rec.lowerDesc, "%35.35s", " ");
	display_field (LOWER + 1);

	strcpy (local_rec.upper, string (11, " "));
	display_field (UPPER);
	sprintf (local_rec.upperDesc, "%35.35s", " ");
	display_field (UPPER - 1);

	strcpy (local_rec.upper, string (strlen (new_mask), "~"));
	display_field (UPPER);
}

void
SetDefault (void)
{
	int	i;

	ChangeMask (SALES);
	local_rec.printerNo = 1;

	strcpy (local_rec.noPrompt, ML ("(No )"));
	strcpy (local_rec.yesPrompt, ML ("(Yes)"));

	strcpy (local_rec.subrange, "Y");
	strcpy (local_rec.subrangeDesc, local_rec.yesPrompt);

	strcpy (local_rec.back, "N");
	strcpy (local_rec.backDesc, local_rec.noPrompt);

	strcpy (local_rec.onight, "N");
	strcpy (local_rec.onightDesc, local_rec.noPrompt);

	for (i = 0;i < 5;i++)
	{
		if (i == 0)
		{
			strcpy (local_rec.byWho [0], "Y");
			strcpy (local_rec.byWhoDesc [0], local_rec.yesPrompt);

			strcpy (local_rec.byWhat [0], "Y");
			strcpy (local_rec.byWhatDesc [0], local_rec.yesPrompt);
		}
		else
		{
			strcpy (local_rec.byWho [i], "N");
			strcpy (local_rec.byWhoDesc [i], local_rec.noPrompt);
			if (i < 4)
			{
				strcpy (local_rec.byWhat [i], "N");
				strcpy (local_rec.byWhatDesc [i], local_rec.noPrompt);
			}
		}
	}
	strcpy (local_rec.wh_no, comm_rec.cc_no);

	strcpy (cudp_rec.co_no, comm_rec.co_no);
	strcpy (cudp_rec.br_no, comm_rec.est_no);
	strcpy (cudp_rec.dp_no, "  ");

	cc = find_rec (cudp, &cudp_rec, GTEQ, "r");
	strcpy (local_rec.dp_no, cudp_rec.dp_no);
}

int
RunProgram (
 char*  filename)
{
	char	dp_wh [3];

	if (FindAnalysis () == 4)
		strcpy (dp_wh, local_rec.wh_no);
	else
		strcpy (dp_wh, local_rec.dp_no);

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y') 
    {
		sprintf 
		(
			err_str, 
			"ONIGHT \"%s\" \"%d\" \"%s\" \"%s\" \"%d\" \"%d\" \"%s\" \"%s\" \"%s\"",
			filename,
			local_rec.printerNo,
			local_rec.lower, 
			local_rec.upper, 
			FindReport (),
			FindAnalysis (),
			dp_wh, 
			local_rec.subrange, 
			ML (mlSaMess043)
		);
		SystemExec (err_str, TRUE);
	}
	/*
	 * Test for forground or background 
	 */
	else 
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%d\" \"%s\" \"%s\" \"%d\" \"%d\" \"%s\" \"%s\"",
			filename,
			local_rec.printerNo,
			local_rec.lower, 
			local_rec.upper, 
			FindReport (),
			FindAnalysis (),
			dp_wh, 
			local_rec.subrange
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
FindReport (void)
{
	int	i;

	for (i = 4;i > 0 && local_rec.byWho [i][0] == 'N';i--)
		;

	return ((local_rec.byWho [i][0] == 'Y') ? i : 0);
}

int
FindAnalysis (void)
{
	int	i;

	for (i = 3;i > 0 && local_rec.byWhat [i][0] == 'N';i--)
		;

	return ((local_rec.byWhat [i][0] == 'Y') ? i : 0);
}

void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);

	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (cudp);
	abc_fclose (ccmr);
	abc_fclose (exsf);
	abc_fclose (excf);
	abc_fclose (cumr);
	abc_fclose (exaf);
	abc_fclose (excl);
	abc_dbclose ("data");
}

/*
 * Search for area code	
 */
void
SrchExaf (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Area Code Description");
	strcpy (exaf_rec.co_no, comm_rec.co_no);
	sprintf (exaf_rec.area_code, "%-2.2s", keyValue);
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strncmp (exaf_rec.area_code, keyValue, strlen (keyValue)) && 
				  !strcmp (exaf_rec.co_no, comm_rec.co_no))
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
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

/*
 * Search for salesman code
 */
void
SrchExsf (
	char	*keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Salesman Code Description");
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", keyValue);
	cc = find_rec (exsf, &exsf_rec, GTEQ, "r");
	while (!cc && !strncmp (exsf_rec.salesman_no, keyValue, strlen (keyValue)) && 
				  !strcmp (exsf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exsf_rec.salesman_no, exsf_rec.salesman);
		if (cc)
			break;
		cc = find_rec (exsf, &exsf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exsf_rec.co_no, comm_rec.co_no);
	sprintf (exsf_rec.salesman_no, "%-2.2s", temp_str);
	cc = find_rec (exsf, &exsf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");
}

/*
 * Search for class type	
 */
void
SrchExcl (
	char	*keyValue)
{
	_work_open (3, 0, 40);
	save_rec ("#No", "#Customer Class Description");
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", keyValue);
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strncmp (excl_rec.class_type, keyValue, strlen (keyValue)) &&
				  !strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type, excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (excl_rec.co_no, comm_rec.co_no);
	sprintf (excl_rec.class_type, "%-3.3s", temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

/*
 * Search for category	
 */
void
SrchExcf (
	char	*keyValue)
{
	_work_open (11,0,40);
	save_rec ("#Category", "#Category Description");
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-16.16s", keyValue);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no, keyValue, strlen (keyValue)) && 
				  !strcmp (excf_rec.co_no, comm_rec.co_no))
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
	sprintf (excf_rec.cat_no, "%-16.16s", temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input                      
 */
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		
		rv_pr (ML (mlSaMess016), 22, 0, 1);

		if (scn == 1)
			box (0, 2, 80, 16);

		line_at (5, 1,79);
		line_at (11,1,79);
		line_at (16,1,79);
		line_at (1,0,79);

		CoLine ();
		line_cnt = 0;
		scn_write (scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

void
CoLine (void)
{
	print_at (20, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21, 0, ML (mlStdMess039), 
					 comm_rec.est_no, clip (comm_rec.est_name));
	print_at (22, 0, ML (mlStdMess085), 
					 cudp_rec.dp_no, clip (cudp_rec.dp_name));

}
