/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_pf_aud.c,v 5.5 2002/07/17 09:58:10 scott Exp $
|  Program Name  : (so_pf_aud.c)
|  Program Desc  : (Pro-Forma Invoice Audit Report) 
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 04/12/89         |
|---------------------------------------------------------------------|
| $Log: so_pf_aud.c,v $
| Revision 5.5  2002/07/17 09:58:10  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/01/09 01:12:12  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.3  2001/08/09 09:21:39  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:51:37  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:20:02  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_pf_aud.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_pf_aud/so_pf_aud.c,v 5.5 2002/07/17 09:58:10 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <pr_format3.h>
#include <twodec.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>
#include <Costing.h>

#define	NOTAX	 (cohr_rec.tax_code [0] == 'A' || cohr_rec.tax_code [0] == 'B')
#define	GST	 (envVarGst [0] == 'Y')
#define POS	1
#define NEG	0
#define HEAD	1
#define LINE	0
#define ByCustomer	 (sortBy [0] == 'C')

	/*===========================
	| Special fields and flags. |
	===========================*/
	int		envVarDbCo		= 1,
			printerNo 		= 1,
			envVarDbMcurr	= 1,
			printed 		= 0,
			firstTime 		= 1,
			foundData 		= 0;

	char	findType [2],
			sortBy [2],
			branchNumber [3],
			envVarGstTaxName [4],
			envVarGst [2],
			systemDate [11],
			sup_part [17],
			prev_dbt [7],
			curr_dbt [7],
			prev_inv [9],
			curr_inv [9],
			prev_gp [13],
			curr_gp [13],
			prev_item [17],
			curr_item [17],
			prev_desc [41],
			Curr_desc [41],
			prev_date [11],
			curr_date [11],
			data_str [201],
			sort_str [201];	

	long	prev_hash [2],
			curr_hash [2];

	double	curr_sprice,
			prev_sprice,
			freight_other,
			gross [3],
			disc [3],
			nett [3],
			fght [3],
			tax [3],
			gst [3],
			inv_amt [3],
			qty_sup [3],
			ext_cost [3];

	float	envVarGstInclude = 0.00,
			gstDivide = 0.00,
			gstPc = 0.00;



	FILE	*fin;
	FILE	*fout;
	FILE	*fsort;

extern	int	TruePosition;
extern	int	EnvScreenOK;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct colnRecord	coln2_rec;
struct inmrRecord	inmr2_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	int  	printerNo;
	char 	sortBy [2];
	char 	sortByDesc [11];
	char 	back [2];
	char 	backDesc [11];
	char 	onite [2];
	char 	oniteDesc [11];
	long	startDate;
	long	endDate;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "sortBy", 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "C", "Sort By Customer/Category   ", " Enter Customer or Item Category ", 
		YES, NO, JUSTLEFT, "CI", "", local_rec.sortBy}, 
	{1, LIN, "sortByDesc", 4, 36, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		YES, NO, JUSTLEFT, "CI", "", local_rec.sortByDesc}, 
	{1, LIN, "startDate", 5, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Start Date                  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate}, 
	{1, LIN, "endDate", 6, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "End Date                    ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate}, 
	{1, LIN, "printerNo", 7, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number              ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 8, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background Y/N              ", "Enter Yes or No. ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 8, 36, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight Y/N               ", "Enter Yes or No. ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{1, LIN, "oniteDesc", 9, 36, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.oniteDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=======================
| Function Declarations |
=======================*/
double 	DoubleValues 		(char *);
double 	OutGst 				(double, int, int, int);
float 	FloatValues 		(char *);
int  	CheckBreak 			(void);
int  	FindIncc 			(void);
int  	FindInmr2 			(long);
int  	ProcessLine 		(long);
int  	ValidateDate 		(long);
int  	check_page 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
long 	DateValues 			(char *);
long 	HashValues 			(char *);
void	ProcessCData 		(int, int);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	InitArray 			(void);
void 	NormalGst 			(int);
void 	OpenDB 				(void);
void 	PrintGroupHeader 	(void);
void 	PrintLine 			(void);
void 	PrintTotals 		(char *);
void 	ProcessColn 		(long);
void 	ProcessSorted 		(void);
void 	ReadCohr 			(void);
void 	RunProgram 			(char *);
void 	SetBreak 			(char *);
void 	SetDefault 			(void);
void 	SplitGst 			(int);
void 	StoreData 			(void);
void 	SumValues 			(char *, int);
void 	ToLocal 			(void);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv  [])
{
	char	startDate [11];
	char	endDate [11];
	char	*sptr = chk_env ("DB_MCURR");

	EnvScreenOK		=	FALSE;
	TruePosition	=	TRUE;

	if (sptr)
		envVarDbMcurr = atoi (sptr);

	if (argc != 2 && argc != 6)
	{
		printf ("Usage : %s <find_flag> OR\007\n\r",argv [0]);
		printf ("Usage : %s <find_flag> <Sort By> <startDate> <endDate> <lpno>\007\n\r",argv [0]);
		printf ("Sort By : C (ustomer)\007\n\r");
		printf ("        : I (tem Category)\007\n\r");
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (envVarGst, "%-1.1s",get_env ("GST"));

	if (GST)
		sprintf (envVarGstTaxName, "%-3.3s",get_env ("GST_TAX_NAME"));
	else
		sprintf (envVarGstTaxName, "%-3.3s","TAX");

	envVarGstInclude = (float) (atof (get_env ("GST_INCLUSIVE")));
	if (envVarGstInclude != 0.00)
	{
		gstDivide = (float) ((100.00 + envVarGstInclude) / envVarGstInclude);
		gstPc  = envVarGstInclude;
	}
	else
	{
		gstDivide = 0.00;
		gstPc  = 0.00;
	}

	envVarDbCo = atoi (get_env ("DB_CO"));

	OpenDB ();


	if (envVarDbCo == 0)
		strcpy (branchNumber," 0");
	else
		strcpy (branchNumber,comm_rec.est_no);

	strcpy (findType, argv [1]);

	if (argc == 6)
	{
		sprintf (sortBy,"%-1.1s",argv [2]);
		sprintf (startDate,"%-10.10s",argv [3]);
		sprintf (endDate,"%-10.10s",argv [4]);
		printerNo 	= 	atoi (argv [5]);

		dsp_screen ("Processing : Printing Pro-Forma Invoices Audit.",comm_rec.co_no,comm_rec.co_name);

		ReadCohr ();

		HeadingOutput ();
		if (foundData)
			ProcessSorted ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();     		/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*=====================
	| Reset control flags |
	=====================*/
   	search_ok 	= TRUE;
   	entry_exit 	= TRUE;
   	prog_exit 	= FALSE;
   	restart 	= FALSE;
	init_vars (1);	
	SetDefault ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
	CloseDB (); 

    if (!restart) 
       RunProgram (argv [0]);
   else
		shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetDefault (
 void)
{
	strcpy (systemDate,DateToString (comm_rec.dbt_date));
	local_rec.printerNo = 1;
	strcpy (local_rec.back, 		"N");
	strcpy (local_rec.backDesc, 	ML ("No"));
	strcpy (local_rec.onite, 		"N");
	strcpy (local_rec.oniteDesc,	ML ("No"));
	strcpy (local_rec.sortBy,		"C");
	strcpy (local_rec.sortByDesc, 	ML ("Customer"));
	local_rec.endDate = comm_rec.dbt_date;
}

void
RunProgram (
 char *prog_name)
{
	char	printerString [3];	
	char	startDate [11];	
	char	endDate [11];	

	clear ();
	print_at (0,0, mlStdMess035);
	fflush (stdout);

	shutdown_prog ();

	local_rec.sortBy [1] = '\0';
	strcpy (startDate,  DateToString (local_rec.startDate));
	strcpy (endDate,    DateToString (local_rec.endDate));
	sprintf (printerString,"%2d",local_rec.printerNo);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onite [0] == 'Y')
	{ 
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				findType,
				local_rec.sortBy,
				startDate,
				endDate,
				printerString,
				"Pro-Forma Invoices Audit.", (char *)0);
		}
	}

	/*====================================
	| Test for forground or background . |
	====================================*/
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				findType,
				local_rec.sortBy,
				startDate,
				endDate,
				printerString, (char *)0);
		/*else
			exit (0);*/
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			findType,
			local_rec.sortBy,
			startDate,
			endDate,
			printerString, (char *)0);
	}
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	abc_alias ("inmr2",inmr);
	open_rec ("inmr2",inmr_list,INMR_NO_FIELDS,"inmr_hhbr_hash");
	open_rec (inmr,inmr_list,INMR_NO_FIELDS,"inmr_id_no");
	open_rec (incc,incc_list,INCC_NO_FIELDS,"incc_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose ("cumr");
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose ("inmr2");
	abc_dbclose ("data");
	CloseCosting ();
}

int
spec_valid (
 int field)
{
	char	valid_inp [2];

	if (strcmp (FIELD.label,"sortBy") == 0) 
	{
		if (local_rec.sortBy [0] == 'C')
			strcpy (local_rec.sortBy, ML ("C (ustomer)     "));
		else
			strcpy (local_rec.sortBy, ML ("I (tem Category)"));
		display_field (field);
		return (EXIT_SUCCESS);
	}

	if (strcmp (FIELD.label,"startDate") == 0) 
	{
		if (local_rec.startDate > local_rec.endDate)
		{
			/*---------------------------------------------
			| Start Date must not be later than End Date. |
			---------------------------------------------*/
			print_mess (ML (mlStdMess019));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (strcmp (FIELD.label,"endDate") == 0) 
	{
		if (local_rec.startDate > local_rec.endDate)
		{
			/*---------------------------------------------
			| Start Date must not be later than End Date. |
			---------------------------------------------*/
			print_mess (ML (mlStdMess019));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}


	if (strcmp (FIELD.label,"printerNo") == 0)
	{
		if (last_char == SEARCH)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			/*------------------
			| Invalid printer. |
			------------------*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Validate Field Selection background option. |
	---------------------------------------------*/
	if (strcmp (FIELD.label,"back") == 0) 
	{
		sprintf (valid_inp, "%1.1s", local_rec.back);

		if (valid_inp [0] == 'N')
			strcpy (local_rec.back, "No ");
		else
		{
			strcpy (local_rec.back, "Yes");
			if (local_rec.onite [0] == 'Y')
				strcpy (local_rec.onite, "No ");
		}

		display_field (field);
		display_field (field+1);
		return (EXIT_SUCCESS);
	}
	
	/*--------------------------------------------
	| Validate Field Selection overnight option. |
	--------------------------------------------*/
	if (strcmp (FIELD.label,"onight") == 0) 
	{
		sprintf (valid_inp, "%1.1s", local_rec.onite);

		if (valid_inp [0] == 'N')
			strcpy (local_rec.onite, "No ");
		else
		{
			if (local_rec.back [0] == 'Y')
				strcpy (local_rec.back, "No ");
			strcpy (local_rec.onite, "Yes");
		}

		display_field (field);
		display_field (field-1);
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}

/*=======================================================
| Process whole cohr file looking for type = findType.	|
=======================================================*/
void
ReadCohr (
 void)
{
	int	firstTime = TRUE;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, branchNumber);
	strcpy (cohr_rec.type,  findType);
	strcpy (cohr_rec.inv_no,"        ");

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	
	while (!cc && 
			!strcmp (cohr_rec.co_no,comm_rec.co_no) && 
			!strcmp (cohr_rec.br_no,branchNumber) && 
			cohr_rec.type [0] == findType [0]) 
	{
		if (ValidateDate (cohr_rec.date_raised))
		{
			if (firstTime)
			{
				fsort = sort_open ("so_pfaud");
				firstTime = FALSE;
				foundData = TRUE;
			}

			if (gstDivide != 0.00 && !NOTAX)
				SplitGst (TRUE);
			else
				NormalGst (TRUE);
		
			if (ByCustomer)
				cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",
							cohr_rec.hhcu_hash);

			if (cc && ByCustomer)
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;
			}
			else
			{
				/*-----------------
				| convert to local |
				-----------------*/
				if (ByCustomer && cohr_rec.exch_rate)
					ToLocal ();
			}

			if (ByCustomer)
			{
				freight_other	=	cohr2_rec.freight + 
						  			cohr2_rec.insurance + 
						  			cohr2_rec.other_cost_1 + 
						  			cohr2_rec.other_cost_2 + 
						  			cohr2_rec.other_cost_3 +
						  			cohr2_rec.sos + 
						  			cohr2_rec.item_levy;

		
				StoreData ();
			}
			else
				ProcessColn (cohr_rec.hhco_hash);

		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
}

/*--------------------------
| Process all order lines. |
--------------------------*/
void
ProcessColn (
	long	hhcoHash)
{
	coln_rec.hhco_hash = hhcoHash;
	coln_rec.line_no = 0;
	cc = find_rec (coln,&coln_rec,GTEQ,"r");

	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		if (gstDivide != 0.00 && !NOTAX)
			SplitGst (FALSE);
		else
			NormalGst (FALSE);

		if (coln_rec.q_order > 0.00)
			ProcessLine (coln_rec.hhbr_hash);

		cc = find_rec (coln,&coln_rec,NEXT,"r");
	}
}

int
ValidateDate (
 long inp_date)
{
	if (cohr_rec.date_raised >= local_rec.startDate && cohr_rec.date_raised <= local_rec.endDate)
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

void
StoreData (
 void)
{
	if (!cc && cohr2_rec.gross > 0.00)
	{
		sprintf 
		(
			data_str,
			"%12.12s %-16.16s %-8.8s %-10.10s %-40.40s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n", 
			" ",
			cumr_rec.dbt_no,
			cohr_rec.inv_no,
			DateToString (cohr_rec.date_raised),
			cumr_rec.dbt_name,
			DOLLARS (cohr2_rec.gross),
			DOLLARS (cohr2_rec.disc + cohr2_rec.ex_disc),
			DOLLARS (cohr2_rec.gross - (cohr2_rec.disc + cohr2_rec.ex_disc)),
			DOLLARS (freight_other), 
			(!GST) ? DOLLARS (cohr2_rec.tax) : 0.00, 
			(GST) ? DOLLARS (cohr2_rec.gst) : 0.00,
			DOLLARS (((cohr2_rec.gross + cohr2_rec.tax + 
			freight_other + cohr2_rec.gst) - 
			cohr2_rec.disc - cohr2_rec.ex_disc))
		); 
		sort_save (fsort,data_str);
	}
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
HeadingOutput (
 void)
{
	char	tax_name [10];

	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("so_pf_aud.p")) == 0) 
		sys_err ("Error in so_pf_aud.p During (FOPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n",printerNo);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".PI12\n");
	if (envVarDbMcurr && ByCustomer)
		fprintf (fout, ".16\n");
	else
		fprintf (fout, ".15\n");

	fprintf (fout, ".L153\n");
	fprintf (fout, ".EPRO-FORMA INVOICE REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany %s\n",clip (comm_rec.co_name));
	fprintf (fout, ".EBranch %s\n",clip (comm_rec.est_name));
	fprintf (fout, ".EWarehouse %s\n",clip (comm_rec.cc_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	if (envVarDbMcurr && ByCustomer)
		fprintf (fout, ".EAll Values In Local Currency \n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESORTED BY %-s\n", (ByCustomer) ? "CUSTOMER NUMBER" : "CLASS/CATEGORY");
	fprintf (fout, ".EFROM %10.10s ", DateToString (local_rec.startDate));
	fprintf (fout, "TO %10.10s\n", DateToString (local_rec.endDate));

	sprintf (tax_name, " %-3.3s  ", envVarGstTaxName);

	switch (sortBy [0])
	{
	case	'C':
		pr_format (fin,fout,"RULEOFF",0,0);
		pr_format (fin,fout,"RULER",0,0);
		pr_format (fin,fout,"HEAD1",1,tax_name);
		pr_format (fin,fout,"HEAD2",0,0);
		pr_format (fin,fout,"HEAD3",0,0);
		break;

	case	'I':
		pr_format (fin, fout,"RULEOFF1",0,0);
		pr_format (fin, fout,"RULER1",0,0);
		pr_format (fin,fout,"HEAD11",1,tax_name);
		pr_format (fin,fout,"HEAD12",0,0);
		pr_format (fin,fout,"HEAD13",0,0);
		break;
	}
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

/*====================================================
| Take out Gst from all the header lines of invoice. |
====================================================*/
void
SplitGst (
 int header)
{
    if (header)
    {
		cohr2_rec.gst 			= 0.00;
		cohr2_rec.gross     	= OutGst (cohr_rec.gross,     HEAD,POS,TRUE);
		cohr2_rec.freight   	= OutGst (cohr_rec.freight,   HEAD,POS,TRUE);
		cohr2_rec.insurance 	= OutGst (cohr_rec.insurance, HEAD,POS,TRUE);
		cohr2_rec.other_cost_1 	= OutGst (cohr_rec.other_cost_1, HEAD,POS,TRUE);
		cohr2_rec.other_cost_2 	= OutGst (cohr_rec.other_cost_2, HEAD,POS,TRUE);
		cohr2_rec.other_cost_3 	= OutGst (cohr_rec.other_cost_3, HEAD,POS,TRUE);
		cohr2_rec.disc      	= OutGst (cohr_rec.disc,      HEAD,NEG,TRUE);
		cohr2_rec.ex_disc   	= OutGst (cohr_rec.ex_disc,   HEAD,NEG,TRUE);
		cohr2_rec.erate_var 	= OutGst (cohr_rec.erate_var, HEAD,POS,TRUE);
		cohr2_rec.sos       	= OutGst (cohr_rec.sos,       HEAD,POS,TRUE);
		cohr2_rec.item_levy    	= OutGst (cohr_rec.item_levy, HEAD,POS,TRUE);
		cohr2_rec.tax       	= cohr_rec.tax;
    }
    else
    {
		coln2_rec.amt_gst 		= 0.00;
		coln2_rec.gross     	= OutGst (coln_rec.gross,      LINE,POS,TRUE);
		coln2_rec.amt_disc  	= OutGst (coln_rec.amt_disc,   LINE,NEG,TRUE);
		coln2_rec.amt_tax   	= OutGst (coln_rec.amt_tax,    LINE,POS,TRUE);
		coln2_rec.tax_pc    	= coln_rec.tax_pc;
		coln2_rec.gst_pc    	= gstPc;
    }
}

/*==============
| Extract Gst. |
==============*/
double	
OutGst (
	double	total_amt, 
	int		header, 
	int		pos, 
	int		add_gst)
{
	double	gst_amount = 0.00;

	if (total_amt == 0)
		return (0.00);

	gst_amount = no_dec (total_amt / gstDivide);
	
	total_amt -= no_dec (gst_amount);
	
	if (!add_gst)
		return (total_amt);

	if (header)
	{
		if (pos)
			cohr2_rec.gst += no_dec (gst_amount);
		else
			cohr2_rec.gst -= no_dec (gst_amount);
	}
	else
	{
		if (pos)
			coln2_rec.amt_gst += no_dec (gst_amount);
		else
			coln2_rec.amt_gst -= no_dec (gst_amount);
	}
	return (total_amt);
}

void
NormalGst (
 int header)
{
	if (header)
	{
		cohr2_rec.gross			= cohr_rec.gross;
		cohr2_rec.freight   	= cohr_rec.freight;
		cohr2_rec.insurance 	= cohr_rec.insurance;
		cohr2_rec.other_cost_1 	= cohr_rec.other_cost_1;
		cohr2_rec.other_cost_2 	= cohr_rec.other_cost_2;
		cohr2_rec.other_cost_3 	= cohr_rec.other_cost_3;
		cohr2_rec.tax 			= cohr_rec.tax;
		cohr2_rec.gst 			= cohr_rec.gst;
		cohr2_rec.disc 			= cohr_rec.disc;
		cohr2_rec.deposit 		= cohr_rec.deposit;
		cohr2_rec.ex_disc 		= cohr_rec.ex_disc;
		cohr2_rec.erate_var 	= cohr_rec.erate_var;
		cohr2_rec.sos 			= cohr_rec.sos;
		cohr2_rec.item_levy 	= cohr_rec.item_levy;
	}
	else
	{
		coln2_rec.tax_pc 		= coln_rec.tax_pc;
		coln2_rec.gst_pc 		= coln_rec.gst_pc;
		coln2_rec.gross 		= coln_rec.gross;
		coln2_rec.amt_disc 		= coln_rec.amt_disc;
		coln2_rec.amt_tax 		= coln_rec.amt_tax;
		coln2_rec.amt_gst 		= coln_rec.amt_gst;
		coln2_rec.sale_price 	= coln_rec.sale_price;
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		/*--------------------------------
		| Print Pro-Forma Invoices Audit |
		--------------------------------*/
		clear ();
		rv_pr (ML (mlSoMess341), 25,0,1);

		line_at (1,0,80);

		move (1,input_row);

		box (0,3,80,6);

		line_at (20,0,80);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);

		line_at (22,0,80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
ProcessSorted (
 void)
{
	char	*sptr;
	int	print_type;

	InitArray ();
	printed = FALSE;
	firstTime = TRUE;

	fsort = sort_sort (fsort,"so_pfaud");
	sptr = sort_read (fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf (sort_str,"%-200.200s",sptr);

		SetBreak (sort_str);

		switch (sortBy [0])
		{
		case	'C':
			dsp_process ("Customer No :",curr_dbt);
			break;

		case	'I':
			dsp_process ("Item No :",curr_item);
			break;

		}

		if (firstTime)
		{
			strcpy (prev_inv,curr_inv);
			strcpy (prev_desc,Curr_desc);
			strcpy (prev_date,curr_date);

			switch (sortBy [0])
			{
			case	'C':
				strcpy (prev_dbt,curr_dbt);
				break;

			case	'I':
				strcpy (prev_gp,curr_gp);
				strcpy (prev_item,curr_item);
				prev_hash [0] = curr_hash [0];
				prev_hash [1] = curr_hash [1];
				prev_sprice = curr_sprice;
				break;
			}
		}

		print_type = CheckBreak ();

		ProcessCData (print_type,firstTime);

		firstTime = 0;

		if (print_type == 0)
			SumValues (sptr + 89,1);
		else
			SumValues (sptr + 89,0);

		strcpy (prev_inv,curr_inv);
		strcpy (prev_desc,Curr_desc);
		strcpy (prev_date,curr_date);

		switch (sortBy [0])
		{
		case	'C':
			strcpy (prev_dbt,curr_dbt);
			break;

		case	'I':
			strcpy (prev_gp,curr_gp);
			strcpy (prev_item,curr_item);
			prev_hash [0] = curr_hash [0];
			prev_hash [1] = curr_hash [1];
			prev_sprice = curr_sprice;
			break;
		}

		sptr = sort_read (fsort);
	}
	if (printed)
	{
		PrintLine ();

		switch (sortBy [0])
		{
		case	'C':
			PrintTotals ("C");
			break;

		case	'I':
			PrintTotals ("G");
			break;

		}
	}
	PrintTotals ("E");
	sort_delete (fsort,"so_pfaud");
}

void
SetBreak (
 char *sort_str)
{
	switch (sortBy [0])
	{
	case	'C':
		sprintf (curr_dbt,"%-6.6s",sort_str + 13);
		break;

	case	'I':
		sprintf (curr_gp,"%-12.12s",sort_str);
		sprintf (curr_item,"%-16.16s",sort_str + 13);
		curr_sprice = DoubleValues (sort_str + 177);
		curr_hash [0] = HashValues (sort_str + 188);
		curr_hash [1] = HashValues (sort_str + 195);
		break;
	}
	sprintf (curr_inv,  "%-8.8s",   sort_str + 30);
	sprintf (curr_date, "%-10.10s", sort_str + 39);
	sprintf (Curr_desc, "%-40.40s", sort_str + 50);
}

int
CheckBreak (
 void)
{
	switch (sortBy [0])
	{
	case	'C':
		if (strcmp (curr_dbt,prev_dbt))
			return (2);
		if (strcmp (curr_inv,prev_inv))
			return (EXIT_FAILURE);
		break;

	case	'I':
		if (strcmp (curr_gp,prev_gp))
			return (3);
		if (strcmp (curr_inv,prev_inv) || strcmp (curr_item,prev_item) || curr_sprice != prev_sprice)
			return (EXIT_FAILURE);
		break;

	}
	return (EXIT_SUCCESS);
}

void
ProcessCData (
 int print_type, 
 int firstTime)
{
	if (firstTime && !ByCustomer)
		PrintGroupHeader ();

	if (!firstTime && print_type > 1) 
	{
		PrintLine ();

		switch (print_type)
		{
		case	2:
			PrintTotals ("C");
			break;

		case	3:
			PrintTotals ("G");
			PrintGroupHeader ();
			break;
		}

	}

	if (print_type == 1)
		PrintLine ();
}

void
PrintGroupHeader (
 void)
{
	pr_format (fin,fout,"GP_LINE",1,curr_gp);
	pr_format (fin,fout,"GP_LINE",2,curr_gp + 1);
}

void
PrintLine (
 void)
{
	if (ByCustomer)
	{
		pr_format (fin,fout,"LINE1",1,prev_dbt);
		pr_format (fin,fout,"LINE1",2,prev_inv);
		pr_format (fin,fout,"LINE1",3,prev_desc);
		pr_format (fin,fout,"LINE1",4,prev_date);
		pr_format (fin,fout,"LINE1",5,gross [0]);
		pr_format (fin,fout,"LINE1",6,disc [0]);
		pr_format (fin,fout,"LINE1",7,nett [0]);
		pr_format (fin,fout,"LINE1",8,fght [0]);
		pr_format (fin,fout,"LINE1",9, (GST) ? gst [0] : tax [0]);
		pr_format (fin,fout,"LINE1",10,inv_amt [0]);
	}
	else
	{
		cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",prev_hash [1]);

		pr_format (fin,fout,"COLN_LINE",1,prev_item);
		pr_format (fin,fout,"COLN_LINE",2,prev_desc);
		pr_format (fin,fout,"COLN_LINE",3,qty_sup [0]);
		pr_format (fin,fout,"COLN_LINE",4,prev_inv);
		pr_format (fin,fout,"COLN_LINE",5,cumr_rec.dbt_acronym);
		pr_format (fin,fout,"COLN_LINE",6,prev_date);
		pr_format (fin,fout,"COLN_LINE",7,gross [0]);
		pr_format (fin,fout,"COLN_LINE",8,ext_cost [0]);
		pr_format (fin,fout,"COLN_LINE",9,disc [0]);
		pr_format (fin,fout,"COLN_LINE",10,nett [0]);
		pr_format (fin,fout,"COLN_LINE",11, (GST) ? gst [0] : tax [0]);
		pr_format (fin,fout,"COLN_LINE",12,inv_amt [0]);
	}
	gross [1] += gross [0];
	disc [1] += disc [0];
	nett [1] += nett [0];
	tax [1] += tax [0];
	gst [1] += gst [0];
	inv_amt [1] += inv_amt [0];

	gross [2] += gross [0];
	disc [2] += disc [0];
	nett [2] += nett [0];
	tax [2] += tax [0];
	gst [2] += gst [0];
	inv_amt [2] += inv_amt [0];

	if (ByCustomer)
	{
		fght [1] += fght [0];
		fght [2] += fght [0];
		fght [0] = 0.00;
	}
	else
	{
		qty_sup [1] += qty_sup [0];
		ext_cost [1] += ext_cost [0];
		qty_sup [2] += qty_sup [0];
		ext_cost [2] += ext_cost [0];
		qty_sup [0] = 0.00;
		ext_cost [0] = 0.00;
	}

	gross [0] = 0.00;
	disc [0] = 0.00;
	nett [0] = 0.00;
	tax [0] = 0.00;
	gst [0] = 0.00;
	inv_amt [0] = 0.00;
}

void
SumValues (
 char *data_line, 
 int add)
{
	char	*sptr = data_line;

	if (ByCustomer)
	{
		if (add)
		{
			gross [0] += DoubleValues (sptr);
			disc [0] += DoubleValues (sptr + 11);
			nett [0] += DoubleValues (sptr + 22);
			fght [0] += DoubleValues (sptr + 33);
			tax [0] += DoubleValues (sptr + 44);
			gst [0] += DoubleValues (sptr + 55);
			inv_amt [0] += DoubleValues (sptr + 66);
		}
		else
		{
			gross [0] = DoubleValues (sptr);
			disc [0] = DoubleValues (sptr + 11);
			nett [0] = DoubleValues (sptr + 22);
			fght [0] = DoubleValues (sptr + 33);
			tax [0] = DoubleValues (sptr + 44);
			gst [0] = DoubleValues (sptr + 55);
			inv_amt [0] = DoubleValues (sptr + 66);
		}
	}
	else
	{
		if (add)
		{
			gross [0] += DoubleValues (sptr);
			disc [0] += DoubleValues (sptr + 11);
			nett [0] += DoubleValues (sptr + 22);
			ext_cost [0] += DoubleValues (sptr + 33);
			tax [0] += DoubleValues (sptr + 44);
			gst [0] += DoubleValues (sptr + 55);
			inv_amt [0] += DoubleValues (sptr + 66);
			qty_sup [0] += FloatValues (sptr + 77);
		}
		else
		{
			gross [0] = DoubleValues (sptr);
			disc [0] = DoubleValues (sptr + 11);
			nett [0] = DoubleValues (sptr + 22);
			ext_cost [0] = DoubleValues (sptr + 33);
			tax [0] = DoubleValues (sptr + 44);
			gst [0] = DoubleValues (sptr + 55);
			inv_amt [0] = DoubleValues (sptr + 66);
			qty_sup [0] = FloatValues (sptr + 77);
		}
	}
}

long	
HashValues (
 char *str)
{
	char	val [7];

	sprintf (val,"%-6.6s",str);
	return (atol (val));
}

long	
DateValues (
 char *str)
{
	char	val [9];

	sprintf (val,"%-8.8s",str);
	return (atol (val));
}

float	
FloatValues (
 char *str)
{
	char	val [11];

	sprintf (val,"%-10.10s",str);
	return ((float) (atof (val)));
}

double	
DoubleValues (
 char *str)
{
	char	val [11];

	sprintf (val,"%-10.10s",str);
	return (atof (val));
}

void
PrintTotals (
 char *tot_type)
{
	int	j = 0;

	switch (tot_type [0])
	{
	case	'C':
		j = 1;
		sprintf (err_str,"%-s","* TOTAL FOR CUSTOMER ");
		break;

	case	'G':
		j = 1;
		sprintf (err_str,"%-s","* TOTAL FOR GROUP   ");
		break;

	case	'I':
		j = 1;
		sprintf (err_str,"%-s","* TOTAL FOR ITEM  ");
		break;

	case	'E':
		j = 2;
		sprintf (err_str,"%-s","*** TOTAL FOR COMPANY");
		break;
	}

	switch (sortBy [0])
	{
	case	'C':
			pr_format (fin,fout,"HEAD3",0,0);
			pr_format (fin,fout,"CUST_TOT",1,err_str);
			pr_format (fin,fout,"CUST_TOT",2,gross [j]);
			pr_format (fin,fout,"CUST_TOT",3,disc [j]);
			pr_format (fin,fout,"CUST_TOT",4,nett [j]);
			pr_format (fin,fout,"CUST_TOT",5,fght [j]);
			pr_format (fin,fout,"CUST_TOT",6, (GST) ? gst [j] : tax [j]);
			pr_format (fin,fout,"CUST_TOT",7,inv_amt [j]);
			if (tot_type [0] == 'C')
				pr_format (fin,fout,"HEAD3",0,0);
			break;

	case	'I':
			pr_format (fin,fout,"HEAD13",0,0);
			pr_format (fin,fout,"TOT_LINE",1,err_str);
			pr_format (fin,fout,"TOT_LINE",2,qty_sup [j]);
			pr_format (fin,fout,"TOT_LINE",3,gross [j]);
			pr_format (fin,fout,"TOT_LINE",4,ext_cost [j]);
			pr_format (fin,fout,"TOT_LINE",5,disc [j]);
			pr_format (fin,fout,"TOT_LINE",6,nett [j]);
			pr_format (fin,fout,"TOT_LINE",7, (GST) ? gst [j] : tax [j]);
			pr_format (fin,fout,"TOT_LINE",8,inv_amt [j]);
			if (tot_type [0] == 'I')
				pr_format (fin,fout,"HEAD13",0,0);
			break;
	}
	gross [j] = 0.00;
	disc [j] = 0.00;
	nett [j] = 0.00;
	tax [j] = 0.00;
	gst [j] = 0.00;
	inv_amt [j] = 0.00;
	if (ByCustomer)
		fght [j] = 0.00;
	else
	{
		qty_sup [j] = 0.00;
		ext_cost [j] = 0.00;
	}
}

void
InitArray (
 void)
{
	int	j;

	for (j = 0; j < 3; j++)
	{
		gross [j] = 0.00;
		disc [j] = 0.00;
		nett [j] = 0.00;
		fght [j] = 0.00;
		tax [j] = 0.00;
		gst [j] = 0.00;
		inv_amt [j] = 0.00;
	}
}

/*========================
| Process Invoice Lines. |
========================*/
int
ProcessLine (
 long hhbr_hash)
{
	double	exten_price = 0.00,
		cost = 0.00,
		ex_cost = 0.00,
		ex_sale = 0.00;
	char	grp_code [13];

	cc = FindInmr2 (hhbr_hash);
	if (cc)
		return (cc);

	cc = FindIncc ();
	if (cc)
		return (cc);

	exten_price = ((coln2_rec.gross + coln2_rec.amt_tax) - coln2_rec.amt_disc);
	exten_price = no_dec (exten_price);

	if (coln_rec.cost_price == 0.00 || inmr_rec.costing_flag [0] == 'S')
	{
	    switch (inmr_rec.costing_flag [0])
	    {
	    case 'A':
	    case 'L':
	    case 'P':
	    case 'T':
	    	cost	=	FindIneiCosts
						(
							inmr_rec.costing_flag, 
							cohr_rec.br_no,
							alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash)
						);
	    	break;

	    case 'S':
			cost	=	FindInsfCost
						(
							incc_rec.hhwh_hash,
							0L,
							coln_rec.serial_no,
							"F"
						);
			if (cost == -1.00)
			{
				cost	=	FindInsfCost
							(
								incc_rec.hhwh_hash,
								0L,
								coln_rec.serial_no,
								"C"
							);
			}
		break;

	    case 'F':
			cost =	FindIncfCost 
					(
						incc_rec.hhwh_hash,
						incc_rec.closing_stock,
						coln_rec.q_order,
						TRUE,
						inmr_rec.dec_pt
					);
    		break;
   
	    case 'I':
			cost =	FindIncfCost 
					(
						incc_rec.hhwh_hash,
						incc_rec.closing_stock,
						coln_rec.q_order,
						FALSE,
						inmr_rec.dec_pt
					);
	    	break;
    
	    default:
	    	break;
	    }
	}
	else
		cost = DOLLARS (coln_rec.cost_price);

	if (cost < 0.00)
	{
		cost	=	FindIneiCosts
					(
						"L",
						cohr_rec.br_no,
						alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash)
					);
	}
	ex_sale = out_cost (coln2_rec.sale_price,inmr_rec.outer_size);
	ex_cost = out_cost (cost,inmr_rec.outer_size);

	ex_cost *= (double) coln_rec.q_order;

	sprintf (grp_code,"%-1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);
	sprintf (data_str,
			"%12.12s %-16.16s %-8.8s %-10.10s %-40.40s %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %06ld %06ld\n", 
		grp_code,
		inmr_rec.item_no,
		cohr_rec.inv_no,
		DateToString (cohr_rec.date_raised),
		coln_rec.item_desc,
		DOLLARS (coln2_rec.gross),
		DOLLARS (coln2_rec.amt_disc),
		DOLLARS (coln2_rec.gross + coln2_rec.amt_tax - coln2_rec.amt_disc),
		ex_cost,
		DOLLARS (coln2_rec.amt_tax), 
		DOLLARS (coln2_rec.amt_gst),
		DOLLARS ((coln2_rec.gross + coln2_rec.amt_tax + 
				  coln2_rec.amt_gst) - coln2_rec.amt_disc),
		coln_rec.q_order,
		DOLLARS (ex_sale),
		inmr_rec.hhbr_hash,
		cohr_rec.hhcu_hash);

	sort_save (fsort,data_str);

	return (EXIT_SUCCESS);
}

int
FindIncc (
 void)
{
	incc_rec.hhcc_hash = coln_rec.incc_hash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	return (cc);
}

int
FindInmr2 (
	long	hhbrHash)
{
	inmr2_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("inmr2",&inmr2_rec,COMPARISON,"r");
	if (cc == 0)
	{
  	    strcpy (inmr_rec.co_no,inmr2_rec.co_no);
  	    strcpy (inmr_rec.item_no,inmr2_rec.item_no);
	    cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	    sprintf (sup_part,"%-16.16s",inmr_rec.item_no);
	    if (cc == 0) 
	    {
	    	while (strcmp (inmr_rec.supercession,"                ") != 0)
		{
		    strcpy (inmr_rec.co_no,comm_rec.co_no);
		    strcpy (inmr_rec.item_no,inmr_rec.supercession);
		    cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		    if (cc != 0)
			    break;
		}
	    }
	}
	if (cc == 0)
	{
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		return (EXIT_SUCCESS);
	}
		
	if (cc)
		return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
}

void
ToLocal (
 void)
{
	cohr2_rec.gross 		/= cohr_rec.exch_rate;
	cohr2_rec.freight 		/= cohr_rec.exch_rate;
	cohr2_rec.insurance 	/= cohr_rec.exch_rate;
	cohr2_rec.disc 			/= cohr_rec.exch_rate;
	cohr2_rec.ex_disc 		/= cohr_rec.exch_rate;
	cohr2_rec.sos 			/= cohr_rec.exch_rate;
	cohr2_rec.item_levy 	/= cohr_rec.exch_rate;
	cohr2_rec.tax 			/= cohr_rec.exch_rate;
	cohr2_rec.gst 			/= cohr_rec.exch_rate;
	cohr2_rec.other_cost_1 	/= cohr_rec.exch_rate;
	cohr2_rec.other_cost_2 	/= cohr_rec.exch_rate;
	cohr2_rec.other_cost_3 	/= cohr_rec.exch_rate;
}
