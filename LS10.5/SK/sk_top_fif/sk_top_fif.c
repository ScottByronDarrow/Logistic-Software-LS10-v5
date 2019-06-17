/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_top_fif.c,v 5.4 2002/07/17 09:58:01 scott Exp $
|  Program Name  : (sk_top_fif.c)
|  Program Desc  : (Print Top 50 Items Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 16/09/88         |
|---------------------------------------------------------------------|
| $Log: sk_top_fif.c,v $
| Revision 5.4  2002/07/17 09:58:01  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/11/23 04:28:53  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_top_fif.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_top_fif/sk_top_fif.c,v 5.4 2002/07/17 09:58:01 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define		DFLT_ITEMS	50
#define		MAX_TOP		50
#define		LCL_ALL		-1000
#define		ONIGHT		(local_rec.onite [0] == 'Y')
#define		BACK		(local_rec.back [0] == 'Y')
#define		BY_BR		(co_br [0] == 'B')
#define		BY_CO		(co_br [0] == 'C')
#define		MTD		 	(local_rec.sales_type [0] == 'M')
#define		YTD		 	(local_rec.sales_type [0] == 'Y')
#define		SA_DT_NONE	(envSaProd == 0)
#define		SA_DT_SAPC	(envSaProd == 1)
#define		SA_DT_SADF	(envSaProd == 2)

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
	char	programDesc [81];
	char	co_br [2];
	int		numberItems 	= 0, 
			envSaProd 		= 0;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sapcRecord	sapc_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;
struct sadfRecord	sadf_rec;

	float	*sadf_qty	=	&sadf_rec.qty_per1;
	double	*sadf_sal	=	&sadf_rec.sal_per1;
	double	*sadf_cst	=	&sadf_rec.cst_per1;

	float	*incc_qty	=	&incc_rec.c_1;
	double	*incc_val	=	&incc_rec.c_val_1;
	double	*incc_prf	=	&incc_rec.c_prf_1;

	double	mtdSales [2]	=	{0.0, 0.0}, 
			ytdSales [2]	=	{0.0, 0.0}, 
			mtdCost [2]		=	{0.0, 0.0}, 
			ytdCost [2]		=	{0.0, 0.0};

	float	mtdQty [2]		=	{0.0, 0.0}, 
			ytdQty [2]		=	{0.0, 0.0};
				

	int		fiscalPeriod	= 0,
			printed 		= FALSE;

	long	hhbrHash;

	char	sort_str [201], 
			data_str [201], 
			br_no [3], 
			item_no [17];


	FILE	*fin, 
			*fout, 
			*fsort;

	extern	int	TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char 	back [8];
	char 	back_desc [8];
	char 	onite [8];
	char 	onite_desc [8];
	char 	sales_type [6];
	char 	sales_type_desc [6];
	char 	co_br [9];
	char 	co_br_desc [9];
	char 	dflt_no [4];
	int  	numberItems;
	int  	lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "sales_to_dt", 3, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "M(TD) Y(TD)           ", " ", 
		YES, NO, JUSTLEFT, "MY", "", local_rec.sales_type}, 
	{1, LIN, "sales_to_dt_desc", 3, 30, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.sales_type_desc}, 
	{1, LIN, "by_what", 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Company or Branch     ", "C=Company / B=Branch ", 
		YES, NO, JUSTLEFT, "CB", "", local_rec.co_br}, 
	{1, LIN, "by_what_desc", 4, 30, CHARTYPE, 
		"AAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.co_br_desc}, 
	{1, LIN, "numberItems", 5, 2, INTTYPE, 
		"NNN", "          ", 
		" ", "0", "Number of Items.      ", "Enter number of Items OR <RETURN> for All items ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.numberItems}, 
	{1, LIN, "dflt_no", 5, 30, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.dflt_no}, 
	{1, LIN, "lpno", 7, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number        ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background Y/N        ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "back_desc", 8, 30, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.back_desc}, 
	{1, LIN, "onite", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight Y/N         ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite}, 
	{1, LIN, "onite_desc", 9, 30, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onite_desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*======================= 
| Function Declarations |
=======================*/
float 	CalculateMargin 	(double, double);
int  	ExceedTop 			(int);
int  	heading 			(int);
int  	spec_valid 			(int);
int  	ValidBranch			(char *);
void 	AddInccSales 		(void);
void 	AddSadfSales 		(void);
void 	AddSapcSales 		(void);
void 	CloseDB 			(void);
void 	CompanyLine 		(void);
void 	InitValues 			(void);
void 	OpenDB 				(void);
void 	PrintLine			(int);
void 	PrintRuler 			(void);
void 	PrintTotalsal 		(int);
void 	ProcessSorted 		(void);
void 	ReadIncc 			(void);
void 	ReadSadf 			(void);
void 	ReadSapc 			(void);
void 	ReportHeading 		(void);
void 	RunProgram 			(char *);
void 	SetBreak 			(char *);
void 	SetupDefault 		(void);
void 	shutdown_prog 		(void);
void 	StoreData 			(long);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;
	envSaProd = atoi (get_env ("SA_PROD"));

	if (argc != 2 && argc != 5) 
	{
		print_at (0, 0, mlSkMess094, argv [0]);
		print_at (0, 0, mlSkMess095, argv [0]);
		return (EXIT_FAILURE);
	}
	
	/*
	 * Read common terminal record.
	 */
	OpenDB ();

	sptr = chk_env ("SA_YEND");
	fiscalPeriod = (sptr == (char *)0) ? comm_rec.fiscal : atoi (sptr);

	if (fiscalPeriod < 1 || fiscalPeriod > 12)
		fiscalPeriod = comm_rec.fiscal;

	if (argc == 5)
	{
		local_rec.lpno = atoi (argv [1]);
		sprintf (local_rec.sales_type, "%-1.1s", argv [2]);
		sprintf (co_br, "%-1.1s", argv [3]);

		if (strncmp (argv [4], "ALL", 3))
			numberItems = atoi (argv [4]);
		else
			numberItems = LCL_ALL;
			
		dsp_screen ("Top x Items Report.", comm_rec.co_no, comm_rec.co_name);

		fsort = sort_open ("top_item");

		if (SA_DT_SADF)
			ReadSadf ();

		if (SA_DT_SAPC)
			ReadSapc ();

		if (SA_DT_NONE)
			ReadIncc ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
		sprintf (programDesc, "%.80s", argv [1]);

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit 	= TRUE;
   	search_ok 	= TRUE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	init_vars (1);	

	SetupDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;

	if (!restart) 
		RunProgram (argv [0]);

	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");

	if (SA_DT_SAPC)
		open_rec (sapc, sapc_list, SAPC_NO_FIELDS, "sapc_hhbr_hash");

	if (SA_DT_SADF)
		open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no4");

	if (SA_DT_NONE)
	{
		open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhcc_hash");
		open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	}

}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	if (SA_DT_SAPC)
		abc_fclose (sapc);

	if (SA_DT_SADF)
		abc_fclose (sadf);

	if (SA_DT_NONE)
	{
		abc_fclose (incc);
		abc_fclose (ccmr);
	}

	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'N')
			strcpy (local_rec.back_desc, "No ");
		else
			strcpy (local_rec.back_desc, "Yes");

		DSP_FLD ("back_desc");
		return (EXIT_SUCCESS);
	}
	
	/*---------------------------------------------
	| Validate Field Selection Overnight  option. |
	---------------------------------------------*/
	if (LCHECK ("onite")) 
	{
		if (local_rec.onite [0] == 'N')
			strcpy (local_rec.onite_desc, "No ");
		else
			strcpy (local_rec.onite_desc, "Yes");

		DSP_FLD ("onite_desc");
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
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("sales_to_dt")) 
	{
		if (local_rec.sales_type [0] == 'M')
			strcpy (local_rec.sales_type_desc, "M(TD)");
		else
			strcpy (local_rec.sales_type_desc, "Y(TD)");

		DSP_FLD ("sales_to_dt_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("by_what")) 
	{
		if (local_rec.co_br [0] == 'C')
			strcpy (local_rec.co_br_desc, "Company");
		else
			strcpy (local_rec.co_br_desc, "Branch ");

		DSP_FLD ("by_what_desc");
		CompanyLine ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("numberItems")) 
	{
		if (dflt_used || local_rec.numberItems == 0)
		{
			local_rec.numberItems = 0;
			strcpy (local_rec.dflt_no, "ALL");
		}
		else
			strcpy (local_rec.dflt_no, "   ");

		DSP_FLD ("numberItems");
		DSP_FLD ("dflt_no");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SetupDefault (
 void)
{
	local_rec.lpno = 1;
	strcpy (local_rec.back, "N");
	strcpy (local_rec.back_desc, "No ");
	strcpy (local_rec.onite, "N");
	strcpy (local_rec.onite_desc, "No ");
	strcpy (local_rec.sales_type, "M");
	strcpy (local_rec.sales_type_desc, "M(TD)");
	strcpy (local_rec.co_br, "C");
	strcpy (local_rec.co_br_desc, "Company");
	local_rec.numberItems = DFLT_ITEMS;
	strcpy (local_rec.dflt_no, "   ");
}

void
RunProgram (
 char *progname)
{
	char	lp_str [3];	
	char	no_items [4];	

	shutdown_prog ();

	sprintf (lp_str, "%2d", local_rec.lpno);
	local_rec.co_br [1]  = '\0';

	if (strncmp (local_rec.dflt_no, "ALL", 3) != 0)
		sprintf (no_items, "%3d", local_rec.numberItems);
	else
		sprintf (no_items, "%-3.3s", local_rec.dflt_no);

	/*====================================
	| Test for forground or background . |
	====================================*/
	if (ONIGHT)
	{
		if (fork () == 0)
			execlp ("ONIGHT", 
				"ONIGHT", 
				progname, 
				lp_str, 
				local_rec.sales_type, 
				local_rec.co_br, 
				no_items, 
				programDesc, (char *)0);
		/*else
			exit (0);*/
	}
	else if (BACK)
	{
		if (fork () == 0)
			execlp (progname, 
				progname, 
				lp_str, 
				local_rec.sales_type, 
				local_rec.co_br, 
				no_items, (char *)0);
		/*else
			exit (0);*/
	}
	else 
	{
		execlp (progname, 
			progname, 
			lp_str, 
			local_rec.sales_type, 
			local_rec.co_br, 
			no_items, (char *)0);
	}
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlSkMess093), 25, 0, 1);

		box (0, 2, 80, 7);
		line_at (1,0,80);
		line_at (6,1,79);
		line_at (20,0,80);

		CompanyLine ();
		move (1, input_row);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
CompanyLine (
 void)
{
	if (BY_CO)
	{
		print_at (21, 0, ML (mlStdMess038), 
			comm_rec.co_no, comm_rec.co_name);
	}
	else
	{
		print_at (21, 0, ML (mlStdMess038), 
			comm_rec.co_no, clip (comm_rec.co_short)); 
		print_at (22, 0, ML (mlStdMess039), 
			comm_rec.est_no, clip (comm_rec.est_name));
	}
}

int
ValidBranch (
 char *br_no)
{
	if (BY_CO || (BY_BR && !strcmp (br_no, comm_rec.est_no)))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*============================================================================
| Process and read file using sapc Data, i.e detailed S/A data held on sapc. |
============================================================================*/
void
ReadSapc (
 void)
{
	long	hhbrHash;

	sapc_rec.hhbr_hash = 0L;
	cc = find_hash (sapc, &sapc_rec, GTEQ, "r", 0L);
	hhbrHash = sapc_rec.hhbr_hash;

	while (!cc)
	{
		if (strcmp (sapc_rec.co_no, comm_rec.co_no) || 
		    !ValidBranch (sapc_rec.br_no))
		{
			cc = find_hash (sapc, &sapc_rec, NEXT, "r", 0L);
			continue;
		}

		if (hhbrHash != sapc_rec.hhbr_hash)
		{
			StoreData (hhbrHash);
			InitValues ();
			hhbrHash = sapc_rec.hhbr_hash;
		}
		AddSapcSales ();
		cc = find_hash (sapc, &sapc_rec, NEXT, "r", 0L);
	}
	StoreData (hhbrHash);
	ProcessSorted ();
}

/*============================================================================
| Process and read file using sadf Data, i.e detailed S/A data held on sadf. |
============================================================================*/
void
ReadSadf (
 void)
{
	long	hhbrHash;

	sadf_rec.hhbr_hash = 0L;
	sadf_rec.hhcu_hash = 0L;
	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");
	hhbrHash = sadf_rec.hhbr_hash;
	while (!cc)
	{
		if (strcmp (sadf_rec.co_no, comm_rec.co_no) || 
		            !ValidBranch (sadf_rec.br_no) ||
			       sadf_rec.year [0] != 'C')
		{
			cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			continue;
		}

		if (hhbrHash != sadf_rec.hhbr_hash)
		{
			StoreData (hhbrHash);
			InitValues ();
			hhbrHash = sadf_rec.hhbr_hash;
		}
		AddSadfSales ();
		cc = find_rec (sadf, &sadf_rec, NEXT, "r");
	}
	StoreData (hhbrHash);
	ProcessSorted ();
}

/*
 * Process and read file using incc Data, i.e No detailed S/A data held.
 */
void
ReadIncc (void)
{
	long	hhbrHash;

	incc_rec.hhcc_hash	=	0L;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	hhbrHash = incc_rec.hhbr_hash;
	while (!cc)
	{
		if (incc_rec.hhcc_hash != ccmr_rec.hhcc_hash)
		{
			ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
			cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}
		}
		if (strcmp (ccmr_rec.co_no, comm_rec.co_no) || 
		            !ValidBranch (ccmr_rec.est_no))
		{
			cc = find_hash (incc, &incc_rec, NEXT, "r", 0L);
			continue;
		}

		if (hhbrHash != incc_rec.hhbr_hash)
		{
			StoreData (hhbrHash);
			InitValues ();
			hhbrHash = incc_rec.hhbr_hash;
		}
		AddInccSales ();
		cc = find_hash (incc, &incc_rec, NEXT, "r", 0L);
	}
	StoreData (hhbrHash);
	ProcessSorted ();
}

/*
 * Process and read file using incc Data, i.e No detailed S/A data held.
 */
void
AddSapcSales (void)
{
	mtdQty [0]   += sapc_rec.mtd_qty;
	mtdSales [0] += DOLLARS (sapc_rec.mtd_sales);
	mtdCost [0]  += DOLLARS (sapc_rec.mtd_csale);
	ytdSales [0] += DOLLARS (sapc_rec.ytd_sales);
	ytdCost [0]  += DOLLARS (sapc_rec.ytd_csale);
	ytdQty [0]   += sapc_rec.ytd_qty;
}

void
AddSadfSales (void)
{
	int		startMonth, 
			endMonth, 
			monthNum, 
			period = 0, 
			i = 0;

	DateToDMY (comm_rec.inv_date, NULL, &monthNum, NULL);
	startMonth = fiscalPeriod + 1;
	endMonth   = monthNum;  

	mtdQty [0]   += sadf_qty [endMonth -1];
	mtdSales [0] += sadf_sal [endMonth -1];
	mtdCost [0]  += sadf_cst [endMonth -1];

	if (endMonth < startMonth)
		endMonth += 12;

	for (i = startMonth; i <= endMonth; i++)
	{
		period = i % 12;
		if (period == 0)
			period = 12;

		ytdQty [0]   += sadf_qty [period -1];
		ytdSales [0] += sadf_sal [period -1];
		ytdCost [0]  += sadf_cst [period -1];
	}
	
}

void
AddInccSales ( void)
{
	int		startMonth, 
			endMonth, 
			monthNum, 
			period = 0, 
			i = 0;

	DateToDMY (comm_rec.inv_date, NULL, &monthNum, NULL);
	startMonth = fiscalPeriod + 1;
	endMonth   = monthNum; 

	mtdQty [0]   += incc_qty [endMonth -1];
	mtdSales [0] += DOLLARS (incc_val [endMonth -1]);
	mtdCost [0]  += DOLLARS (incc_val [endMonth -1] - incc_prf [endMonth -1]);

	if (endMonth < startMonth)
		endMonth += 12;

	for (i = startMonth; i <= endMonth; i++)
	{
		period = i % 12;
		if (period == 0)
			period = 12;

		ytdQty [0]   += incc_qty [period -1];
		ytdSales [0] += DOLLARS (incc_val [period -1]);
		ytdCost [0]  += DOLLARS (incc_val [period -1] - incc_prf [period -1]);
	}
}

void
StoreData (
	long	hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
	{
		sprintf (inmr_rec.description, "%-40.40s", "Unknown Item");
		sprintf (inmr_rec.item_no, "%-16.16s", "DELETE");
	}

	dsp_process ("Item No : ", inmr_rec.item_no);

	sprintf 
	(
		data_str, 
		"%010.2f %010.2f %010.2f %010.2f %010.2f %010.2f %6ld\n", 
		(MTD) ? mtdSales [0] : ytdSales [0], 
		(MTD) ? mtdCost  [0] : ytdCost 	[0], 
		(MTD) ? mtdQty 	 [0] : ytdQty 	[0], 
		(MTD) ? ytdSales [0] : mtdSales [0], 
		(MTD) ? ytdCost  [0] : mtdCost	[0], 
		(MTD) ? ytdQty 	 [0] : mtdQty 	[0], 
		hhbrHash
	);
	sort_save (fsort, data_str);
}

void
ReportHeading (void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".13\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	if (numberItems == LCL_ALL)
		fprintf (fout, ".ETOP ITEMS SALES ANALYSIS REPORT\n");
	else
		fprintf (fout, ".ETOP %d ITEMS SALES ANALYSIS REPORT\n", 
								numberItems);

	fprintf (fout, ".ESORTED BY %s TO DATE SALES VALUES\n", 
						(MTD) ? "MONTH" : "YEAR");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	if (BY_BR)
	{
		fprintf (fout, ".E%s %-s\n", clip (comm_rec.est_no), 
					    clip (comm_rec.est_name));
	}
	else
		fprintf (fout, ".B1\n");

	fprintf (fout, ".R=====================");
	fprintf (fout, "========================================");
	fprintf (fout, "============================================");
	fprintf (fout, "=============================================\n");

	fprintf (fout, "=====================");
	fprintf (fout, "========================================");
	fprintf (fout, "============================================");
	fprintf (fout, "=============================================\n");

	fprintf (fout, "|NO|  ITEM NUMBER   |");
	fprintf (fout, "            DESCRIPTION                 ");
	fprintf (fout, "|             M O N T H   T O   D A T E     ");
	fprintf (fout, "|              Y E A R   T O   D A T E      |\n");

	fprintf (fout, "|  |                |");
	fprintf (fout, "                                        ");
	fprintf (fout, "|  QTY   |    SALES   |    COST    | MARGIN ");
	fprintf (fout, "|  QTY   |    SALES   |    COST    | MARGIN |\n");
	PrintRuler ();
	fflush (fout);
}

void
PrintRuler (void)
{
	fprintf (fout, "|--|----------------|");
	fprintf (fout, "----------------------------------------");
	fprintf (fout, "|--------|------------|------------|--------");
	fprintf (fout, "|--------|------------|------------|--------|\n");
	fflush (fout);

}

float	
CalculateMargin (
	double	sale, 
	double	cost)
{
	double	marg = 0.0;

	if (sale != 0)
		marg = (sale - cost) / sale * 100;

	return ((float) marg);
}

void
SetBreak (
	char	*sort_str)
{
	if (MTD)
	{
		mtdSales [0]  = atof (sort_str);
		mtdCost  [0]  = atof (sort_str + 11);
		mtdQty   [0]  = (float) (atof (sort_str + 22));
		ytdSales [0]  = atof (sort_str + 33);
		ytdCost  [0]  = atof (sort_str + 44);
		ytdQty   [0]  = (float) (atof (sort_str + 55));
	}
	else
	{
		ytdSales [0] = atof (sort_str);
		ytdCost  [0] = atof (sort_str + 11);
		ytdQty   [0] = (float) (atof (sort_str + 22));
		mtdSales [0] = atof (sort_str + 33);
		mtdCost  [0] = atof (sort_str + 44);
		mtdQty   [0] = (float) (atof (sort_str + 55));
	}
	hhbrHash = atol (sort_str + 66);
}

int
ExceedTop (
	int	numberPrinted)
{
	if (numberItems == LCL_ALL || numberPrinted < numberItems)  
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void
ProcessSorted (void)
{
	char	*sptr;
	int		numberPrinted = 0;

	InitValues ();
	ReportHeading ();
	
	fsort = dsort_sort (fsort, "top_item");
	sptr = sort_read (fsort);
	while (sptr != (char *) 0 && !ExceedTop (numberPrinted))
	{
		sprintf (sort_str, "%-100.100s", sptr);

		SetBreak (sort_str);

		inmr_rec.hhbr_hash	=	hhbrHash;
		if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
		{
			sptr = sort_read (fsort);
			continue;
		}
		dsp_process ("Item No : ", inmr_rec.item_no);
		PrintLine (numberPrinted);
		numberPrinted++;
		
		InitValues ();

		sptr = sort_read (fsort);
	}
	if (numberPrinted != 0)
	{
		PrintTotalsal (numberPrinted);
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
	sort_delete (fsort, "top_item");
}

void
InitValues (void)
{
	mtdSales [0] = 0.00;
	ytdSales [0] = 0.00;
	mtdCost [0]  = 0.00;
	ytdCost [0]  = 0.00;
	mtdQty [0]   = 0.00;
	hhbrHash   = atol (sort_str + 69);
	ytdQty [0]   = 0.00;
}

void
PrintLine (
	int		i)
{
	float	margin = 0.00;

	fprintf (fout, "!%02d!%-16.16s", i + 1, inmr_rec.item_no);
	fprintf (fout, "!%40.40s", inmr_rec.description);
	fprintf (fout, "!%8.2f",    mtdQty [0]);
	fprintf (fout, "!%12.2f",   mtdSales [0]);
	fprintf (fout, "!%12.2f",   mtdCost [0]);
	margin = CalculateMargin (mtdSales [0], mtdCost [0]);
	fprintf (fout, "!%8.2f",    margin);

	fprintf (fout, "!%8.2f",    ytdQty [0]);
	fprintf (fout, "!%12.2f",   ytdSales [0]);
	fprintf (fout, "!%12.2f",   ytdCost [0]);
	margin = CalculateMargin (ytdSales [0], ytdCost [0]);
	fprintf (fout, "!%8.2f!\n", margin);

	fflush (fout);

	mtdSales [1] += mtdSales 	[0];
	ytdSales [1] += ytdSales 	[0];
	mtdCost  [1] += mtdCost 	[0];
	ytdCost  [1] += ytdCost 	[0];
	mtdQty   [1] += mtdQty 		[0];
	ytdQty   [1] += ytdQty 		[0];

}

void
PrintTotalsal (
	int		numberPrinted)
{
	float	margin = 0.00;

	PrintRuler ();
	fprintf (fout, "!  !%-16.16s", " ");
	fprintf (fout, "! GRAND TOTALS FOR TOP %d ITEMS          ", numberPrinted);
	fprintf (fout, "!%8.2f", mtdQty [1]);
	fprintf (fout, "!%12.2f", mtdSales [1]);
	fprintf (fout, "!%12.2f", mtdCost [1]);
	margin = CalculateMargin (mtdSales [1], mtdCost [1]);
	fprintf (fout, "!%7.2f ", margin);

	fprintf (fout, "!%8.2f", ytdQty [1]);
	fprintf (fout, "!%12.2f", ytdSales [1]);
	fprintf (fout, "!%12.2f", ytdCost [1]);
	margin = CalculateMargin (ytdSales [1], ytdCost [1]);
	fprintf (fout, "!%7.2f !\n", margin);

	fflush (fout);
}
