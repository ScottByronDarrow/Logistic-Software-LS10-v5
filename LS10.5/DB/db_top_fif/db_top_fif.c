/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_top_fif.c,v 5.5 2002/07/17 09:57:09 scott Exp $
|  Program Name  : (db_top_fif.c) 
|  Program Desc  : (Print Top 50 customers Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_top_fif.c,v $
| Revision 5.5  2002/07/17 09:57:09  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/11/27 02:42:47  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/11/26 09:02:47  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_top_fif.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_top_fif/db_top_fif.c,v 5.5 2002/07/17 09:57:09 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>

#define	NORMAL		0
#define	AREA		1
#define	CUST_TYPE	2

#define	DFLT_CUSTS	50
#define	LCL_ALL		-1000

#define	TYPE		program [typeFlag]
#define	MTD		 	(salesType [0] == 'M')
#define	YTD		 	(salesType [0] == 'Y')
#define	CURR_YR		(local_rec.currLast [0] == 'C')
#define	LAST_YR		(local_rec.currLast [0] == 'L')
#define	TOP_REP		(local_rec.topBottom [0] == 'T')
#define	CF(x)		comma_fmt (DOLLARS (x), "NNN,NNN,NNN,NNN.NN")

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cusaRecord	cusa_rec;
struct exafRecord	exaf_rec;
struct exclRecord	excl_rec;
struct pocrRecord	pocr_rec;

	Money	*cusa_val	=	&cusa_rec.val1;

	double	fgnYtdSales	= 0.00,
			fgnMtdSales	= 0.00,
			fgnYtdTotal	= 0.00,
			fgnMtdTotal	= 0.00,
			locYtdSales	= 0.00,
			locMtdSales	= 0.00,
			locYtdTotal	= 0.00,
			locMtdTotal	= 0.00;

	int		printerNo 		= 1,
			startMonth		= 0,
			endMonth		= 0,
			typeFlag		= 0,
			numberSaved 	= 0,
			startCustomer 	= 0;

	FILE	*pp;
	FILE	*fsort;

	char	*sort_read (FILE *);
	char	reportType 	[2],
			salesType 	[2],
			Select 		[4],
			programDesc [81],
			noPrompt	[11],
			yesPrompt	[11],
			mtdPrompt	[11],
			ytdPrompt	[11],
			currPrompt	[11],
			lastPrompt	[11],
			topPrompt	[11],
			botPrompt	[11];

	struct	{
		char	*_type;
		char	*_desc;
		char	*_head;
	} program [] = {
		{"N","    ","ALL "},
		{"A","Area","AREA"},
		{"C","Type","TYPE"},
		{"","",""},
	};

	extern	int	TruePosition;


/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	reportType [2];
	char 	back [8];
	char 	backDesc [8];
	char 	onite [8];
	char 	onightDesc [8];
	char 	currLast [8];
	char 	currLastDesc [8];
	char 	salesType [6];
	char 	salesTypeDesc [6];
	char 	select [9];
	char	desc [41];
	char 	defaultNo [4];
	char	topBottom [9];
	char	topBottomDesc [9];
	int  	numberCusts;
	int  	printerNo;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "sales_to_dt", 3, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "M(TD) Y(TD)              ", " ", 
		YES, NO, JUSTLEFT, "MY", "", local_rec.salesType}, 
	{1, LIN, "sales_to_dt_desc", 3, 32, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.salesTypeDesc}, 
	{1, LIN, "period", 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "C", "C(urrent) L(ast) Year    ", " ", 
		YES, NO, JUSTLEFT, "CL", "", local_rec.currLast}, 
	{1, LIN, "period_desc", 4, 32, CHARTYPE, 
		"AAAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.currLastDesc}, 
	{1, LIN, "numberCusts", 5, 2, INTTYPE, 
		"NNN", "          ", 
		" ", " ", "Number of Customers      ", "Enter number of Customers OR <RETURN> for All Customers ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.numberCusts}, 
	{1, LIN, "defaultNo", 5, 32, CHARTYPE, 
		"AAA", "          ", 
		" ", " ", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.defaultNo}, 
	{1, LIN, "topBottom", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "T", "T(op 50) or B(ottom 50)  ", "", 
		NO, NO, JUSTLEFT, "TB", "", local_rec.topBottom}, 
	{1, LIN, "topBottomDesc", 6, 32, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.topBottomDesc}, 
	{1, LIN, "area", 8, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Area Code                ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.select}, 
	{1, LIN, "cus_type", 8, 2, CHARTYPE, 
		"UUU", "          ", 
		" ", "", "Customer Type            ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.select}, 
	{1, LIN, "desc", 8, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.desc}, 
	{1, LIN, "printerNo", 8, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number           ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background Y/N           ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "backDesc", 9, 32, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.backDesc}, 
	{1, LIN, "onite", 10, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight Y/N            ", "Enter Y(es) or N(o). ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onite}, 
	{1, LIN, "onightDesc", 10, 32, CHARTYPE, 
		"AAA", "          ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.onightDesc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SetupDetails 		(void);
int 	spec_valid 			(int);
void 	SrchExcl 			(char *);
void 	SrchExaf 			(char *);
void 	RunProgram 			(char *);
void 	ProcessCumr 		(void);
int 	ValidateCustomer 	(void);
void 	ProcessSorted 		(void);
int 	ExceedTop 			(int);
void 	ReportHeading 		(void);
void 	ProcessCusa			(void);
void 	StoreData 			(void);
void 	CalcSales 			(void);
int 	heading 			(int scn);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	TruePosition	=	TRUE;

	if (argc != 8 && argc != 3)
	{
		print_at (0,0,mlDbMess701,argv [0]);
		print_at (1,0,mlDbMess702,argv [0]);
		print_at (2,0,mlDbMess703);
		print_at (3,0,mlDbMess704);
		print_at (4,0,mlDbMess705);
		return (EXIT_FAILURE);
	}

	if (argc == 8)
	{
		printerNo = atoi (argv [1]);

		strcpy (local_rec.reportType, argv [2]);
		switch (local_rec.reportType [0])
		{
		case	'A':
			typeFlag = AREA;
			break;

		case	'C':
			typeFlag = CUST_TYPE;
			break;

		default:
			typeFlag = NORMAL;
			break;
		}

		OpenDB ();

		sprintf (salesType,"%-1.1s",argv [3]);
		sprintf (local_rec.currLast,"%-1.1s",argv [4]);

		dsp_screen ("Sales By Customer Report.", 
			comm_rec.co_no, comm_rec.co_name);


		switch (typeFlag)
		{
		case	NORMAL:
			strcpy (Select,"   ");
			break;

		case	AREA:
			sprintf (Select,"%-2.2s",argv [5]);
			strcpy (exaf_rec.co_no,comm_rec.co_no);
			sprintf (exaf_rec.area_code,"%-2.2s",Select);
			cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
			if (cc)
				strcpy (exaf_rec.area,"No Description Found");
			break;

		case	CUST_TYPE:
			sprintf (Select,"%-3.3s",argv [5]);
			strcpy (excl_rec.co_no,comm_rec.co_no);
			sprintf (excl_rec.class_type,"%-3.3s",Select);
			cc = find_rec (excl, &excl_rec, COMPARISON, "r");
			if (cc)
				strcpy (excl_rec.class_desc,"No Description Found");
			break;
		}

		if (strncmp (argv [6], "ALL", 3))
			local_rec.numberCusts = atoi (argv [6]);
		else
			local_rec.numberCusts = LCL_ALL;

		strcpy (local_rec.topBottom, argv [7]);

		ReportHeading ();

		ProcessCumr ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
	{

		SETUP_SCR (vars);

		sprintf (programDesc,"%.80s", argv [1]);
		strcpy (local_rec.reportType, argv [2]);
		switch (local_rec.reportType [0])
		{
		case	'N':
		case	'n':
			typeFlag = NORMAL;
			FLD ("area") = ND;
			FLD ("cus_type") = ND;
			FLD ("desc") = ND;
			break;
	
		case	'A':
		case	'a':
			SCN_ROW ("printerNo")	=	10;
			SCN_ROW ("back")		= 	11;
			SCN_ROW ("backDesc")	=	11;
			SCN_ROW ("onite")		= 	12;
			SCN_ROW ("onightDesc")	=	12;
			typeFlag = AREA;
			FLD ("cus_type") = ND;
			break;
	
		case	'C':
		case	'c':
			SCN_ROW ("printerNo")	=	10;
			SCN_ROW ("back")		= 	11;
			SCN_ROW ("backDesc")	=	11;
			SCN_ROW ("onite")		= 	12;
			SCN_ROW ("onightDesc")	=	12;
			typeFlag = CUST_TYPE;
			FLD ("area") = ND;
			break;
		}

		OpenDB ();
	}

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
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

	SetupDetails ();

	/*-----------------------------
	| Edit screen 1 linear input. |
	-----------------------------*/
	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;

	if (restart)
    {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }
	
	if (!restart)
		RunProgram (argv [0]);

	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (cusa, cusa_list, CUSA_NO_FIELDS, "cusa_id_no");
	switch (typeFlag)
	{
	case	AREA:
		open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
		break;
	case	CUST_TYPE:
		open_rec (excl, excl_list, EXCL_NO_FIELDS, "excl_id_no");
		break;
	}
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (pocr);
	abc_fclose (cusa);
	switch (typeFlag)
	{
	case	AREA:
		abc_fclose (exaf);
		break;
	case	CUST_TYPE:
		abc_fclose (excl);
		break;
	}
	abc_dbclose ("data");
}
void
SetupDetails (void)
{
	strcpy (noPrompt,  	ML ("No "));
	strcpy (yesPrompt, 	ML ("Yes"));
	strcpy (mtdPrompt, 	ML ("MTD"));
	strcpy (ytdPrompt, 	ML ("YTD"));
	strcpy (currPrompt, ML ("Current"));
	strcpy (lastPrompt, ML ("Last   "));
	strcpy (topPrompt, 	ML ("Top   "));
	strcpy (botPrompt, 	ML ("Bottom"));
	
	local_rec.printerNo = 1;
	strcpy (local_rec.back,		"N");
	strcpy (local_rec.onite,	"N");
	strcpy (local_rec.salesType,"M");
	strcpy (local_rec.currLast,	"C");
	strcpy (local_rec.topBottom,"T");
	strcpy (local_rec.backDesc, 	noPrompt);
	strcpy (local_rec.onightDesc,	noPrompt);
	strcpy (local_rec.salesTypeDesc,mtdPrompt);
	strcpy (local_rec.currLastDesc, currPrompt);
	strcpy (local_rec.topBottomDesc,topPrompt);
	local_rec.numberCusts = DFLT_CUSTS;
	strcpy (local_rec.defaultNo,"   ");
}

int
spec_valid (
 int                field)
{
	/*
	 * Validate Field Selection background option.
	 */
	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'N')
			strcpy (local_rec.backDesc, noPrompt);
		else
			strcpy (local_rec.backDesc, yesPrompt);

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection Overnight  option.
	 */
	if (LCHECK ("onite")) 
	{
		if (local_rec.onite [0] == 'N')
			strcpy (local_rec.onightDesc, noPrompt);
		else
			strcpy (local_rec.onightDesc, yesPrompt);

		DSP_FLD ("onightDesc");
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
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("sales_to_dt")) 
	{
		if (local_rec.salesType [0] == 'M')
			strcpy (local_rec.salesTypeDesc, mtdPrompt);
		else
			strcpy (local_rec.salesTypeDesc, ytdPrompt);

		DSP_FLD ("sales_to_dt_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("period")) 
	{
		if (local_rec.currLast [0] == 'C')
			strcpy (local_rec.currLastDesc, currPrompt);
		else
			strcpy (local_rec.currLastDesc, lastPrompt);

		DSP_FLD ("period_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("area")) 
	{
		if (SRCH_KEY)
		{
			SrchExaf (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exaf_rec.co_no,comm_rec.co_no);
		sprintf (exaf_rec.area_code,"%-2.2s",local_rec.select);
		cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess108));
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.desc,exaf_rec.area);
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate customer type and allow search.
	 */
	if (LCHECK ("cus_type"))
	{
		if (SRCH_KEY)
		{
			SrchExcl (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (excl_rec.co_no,comm_rec.co_no);
		sprintf (excl_rec.class_type,"%-3.3s",local_rec.select);
		cc = find_rec (excl, &excl_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess170));
			return (EXIT_FAILURE); 
		}
		strcpy (local_rec.desc,excl_rec.class_desc);
		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("numberCusts")) 
	{
		if (dflt_used || local_rec.numberCusts == 0)
		{
			local_rec.numberCusts = 0;
			strcpy (local_rec.defaultNo, ML ("ALL"));
		}
		else
			strcpy (local_rec.defaultNo,"   ");

		DSP_FLD ("numberCusts");
		DSP_FLD ("defaultNo");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("topBottom")) 
	{
		if (local_rec.topBottom [0] == 'T')
			strcpy (local_rec.topBottomDesc, topPrompt);
		else
			strcpy (local_rec.topBottomDesc, botPrompt);

		DSP_FLD ("topBottomDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search routine for Cust. Class Type.    
 */
void
SrchExcl (
	char	*key_val)
{
	_work_open (3,0,40);
	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,key_val);
	save_rec ("#No.","#Customer Type Description");
	cc = find_rec (excl, &excl_rec, GTEQ, "r");
	while (!cc && !strncmp (excl_rec.class_type, key_val,strlen (key_val)) && 
				  !strcmp (excl_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excl_rec.class_type,excl_rec.class_desc);
		if (cc)
			break;
		cc = find_rec (excl, &excl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excl_rec.co_no,comm_rec.co_no);
	strcpy (excl_rec.class_type,temp_str);
	cc = find_rec (excl, &excl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excl, "DBFIND");
}

/*
 * Search routine for Area Master File.
 */
void
SrchExaf (
 char*              key_val)
{
	_work_open (2,0,40);
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,key_val);
	save_rec ("#No.","#Area Description");
	cc = find_rec (exaf, &exaf_rec, GTEQ, "r");
	while (!cc && !strncmp (exaf_rec.area_code,key_val,strlen (key_val)) && 		      !strcmp (exaf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (exaf_rec.area_code,exaf_rec.area);
		if (cc)
			break;
		cc = find_rec (exaf, &exaf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (exaf_rec.co_no,comm_rec.co_no);
	strcpy (exaf_rec.area_code,temp_str);
	cc = find_rec (exaf, &exaf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exaf, "DBFIND");
}

void
RunProgram (
	char	*progname)
{
	char	noCusts [4];	

	rset_tty ();

	clear ();
	print_at (0,0,ML (mlStdMess035));
	shutdown_prog ();

	if (strncmp (local_rec.defaultNo,"ALL",3) != 0)
		sprintf (noCusts,"%3d",local_rec.numberCusts);
	else
		sprintf (noCusts,"%-3.3s",local_rec.defaultNo);

	/*
	 * Test for forground or background.
	 */
	if (local_rec.onite [0] == 'Y')
	{
		sprintf 
		(
			err_str,
			"ONIGHT \"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", 
			progname,
			local_rec.printerNo,
			local_rec.reportType,
			local_rec.salesType,
			local_rec.currLast,
			local_rec.select,
			noCusts,
			local_rec.topBottom,
			programDesc
		);
		SystemExec (err_str, TRUE);
	}
    else 
	{
		sprintf 
		(
			err_str,
			"\"%s\" \"%d\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", 
			progname,
			local_rec.printerNo,
			local_rec.reportType,
			local_rec.salesType,
			local_rec.currLast,
			local_rec.select,
			noCusts,
			local_rec.topBottom
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

void
ProcessCumr (void)
{
	strcpy (cumr_rec.co_no,comm_rec.co_no);
	strcpy (cumr_rec.est_no,"  ");
	strcpy (cumr_rec.dbt_acronym,"         ");

	fsort = sort_open ("top_fif");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		if (!ValidateCustomer ())
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Customer : ", cumr_rec.dbt_acronym);

		fgnMtdSales 	= 0.0;
		fgnYtdSales 	= 0.0;
		locMtdSales 	= 0.0;
		locYtdSales 	= 0.0;
		ProcessCusa ();
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	ProcessSorted ();
}

int
ValidateCustomer (void)
{
	switch (typeFlag)
	{
	case	NORMAL:
		return (EXIT_FAILURE);

	case	AREA:
		if (!strncmp (cumr_rec.area_code,Select,2))
			return (EXIT_FAILURE);

	case	CUST_TYPE:
		if (!strncmp (cumr_rec.class_type,Select,3))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
ProcessSorted (void)
{
	char	*sptr;
	char	tmp_head [20];
	int		num_printed = 0;
	int		i = 1;

	abc_selfield (cumr, "cumr_hhcu_hash");

	fsort = dsort_sort (fsort, "top_fif");

	if (!TOP_REP)
	{
		startCustomer = numberSaved - local_rec.numberCusts;
		if (startCustomer < 0)
			startCustomer = 0;

		sptr = sort_read (fsort);
		while (sptr && num_printed++ < startCustomer)
			sptr = sort_read (fsort);

		i = num_printed;
	}
	else
		sptr = sort_read (fsort);

	while (sptr && !ExceedTop (num_printed))
	{
		cumr_rec.hhcu_hash	=	atol (sptr + 60);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (MTD)
			{
				fgnMtdSales 	= atof (sptr);
				fgnYtdSales 	= atof (sptr + 15);
				locMtdSales 	= atof (sptr + 30);
				locYtdSales 	= atof (sptr + 45);
			}
			else
			{
				fgnYtdSales 	= atof (sptr);
				fgnMtdSales 	= atof (sptr + 15);
				locYtdSales 	= atof (sptr + 30);
				locMtdSales 	= atof (sptr + 45);
			}

			fprintf (pp, "|%02d|  %6.6s  |", i++, cumr_rec.dbt_no);
			fprintf (pp, "%40.40s",      cumr_rec.dbt_name);
			fprintf (pp, "|%18.18s ",    CF (fgnMtdSales));
			fprintf (pp, "|%18.18s ",    CF (fgnYtdSales));
			fprintf (pp, "|%3.3s ",      cumr_rec.curr_code);
			fprintf (pp, "|%18.18s ",    CF (locMtdSales));
			fprintf (pp, "|%18.18s |\n", CF (locYtdSales));

			fgnMtdTotal		+= fgnMtdSales;
			fgnYtdTotal		+= fgnYtdSales;
			locMtdTotal 	+= locMtdSales;
			locYtdTotal 	+= locYtdSales;
			num_printed++;
		}
		sptr = sort_read (fsort);
	}

	sort_delete (fsort,"top_fif");

	fprintf (pp, "|--|----------|");
	fprintf (pp, "----------------------------------------");
	fprintf (pp, "|-------------------|-------------------|----");
	fprintf (pp, "|-------------------|-------------------|\n");
	if (local_rec.numberCusts == LCL_ALL)
		strcpy (tmp_head, "ALL       ");
	else
	{
		sprintf (tmp_head, "%s %03d", (TOP_REP) ? "TOP   " : "BOTTOM",
											local_rec.numberCusts);
	}

	fprintf (pp, "|  |          |GRAND TOTALS FOR %s CUSTOMERS   ", tmp_head);
	fprintf (pp, "|                   ");
	fprintf (pp, "|                   ");
	fprintf (pp, "|    ");
	fprintf (pp, "|%18.18s ",    CF (locMtdTotal));
	fprintf (pp, "|%18.18s |\n", CF (locYtdTotal));
	fprintf (pp, ".EOF\n");
}

int
ExceedTop (
	int		num_printed)
{
	if (TOP_REP)
	{
		if (local_rec.numberCusts == LCL_ALL || 
		    num_printed < local_rec.numberCusts)  
			return (EXIT_SUCCESS);
	}
	else
	{
		if (local_rec.numberCusts == LCL_ALL || 
		    num_printed < (local_rec.numberCusts + startCustomer + 1))  
			return (EXIT_SUCCESS);
	}
	return (EXIT_FAILURE);
}

void
ReportHeading (void)
{
	char	toDateStr [14];
	char	periodStr [8];
	char	sortStr [50];
	char	tmp_head [20];

	if ((pp = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "popen");

	sprintf (periodStr,"%-7.7s", (CURR_YR) ? "CURRENT" : " LAST  ");
	sprintf (toDateStr,"%-13.13s", (MTD) ? "MONTH TO DATE" : "YEAR TO DATE ");
	sprintf (sortStr, "SORTED ON %s YEAR - %s SALES",periodStr,toDateStr);

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp, ".LP%d\n",printerNo);
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	if (local_rec.numberCusts == LCL_ALL)
		strcpy (tmp_head, "ALL");
	else
	{
		sprintf (tmp_head, 
			"%s %03d", 
			(TOP_REP) ? "TOP" : "BOTTOM",
			local_rec.numberCusts);
	}
	fprintf (pp, ".E%s CUSTOMER SALES ANALYSIS REPORT\n", tmp_head);
	switch (typeFlag)
	{
	case NORMAL:
		fprintf (pp, ".ECUSTOMER %s %s\n",TYPE._head,Select);
		break;

	case AREA:
		fprintf (pp, ".ECUSTOMER %s %s %s\n",TYPE._head,Select,clip (exaf_rec.area));
		break;

	case CUST_TYPE:
		fprintf (pp, ".ECUSTOMER %s %s %s\n",TYPE._head,Select,clip (excl_rec.class_desc));
		break;

	}

	fprintf (pp, ".E%s\n",clip (sortStr));
	fprintf (pp,".B1\n");
	fprintf (pp, ".E%s AS AT %s\n", clip (comm_rec.co_short),SystemTime ());

	fprintf (pp, ".R===============");
	fprintf (pp, "========================================");
	fprintf (pp, "=============================================");
	fprintf (pp, "=========================================\n");

	fprintf (pp, "===============");
	fprintf (pp, "========================================");
	fprintf (pp, "=============================================");
	fprintf (pp, "=========================================\n");

	fprintf (pp, "|NO| CUSTOMER |");
	fprintf (pp, "            N   A   M   E               ");
	fprintf (pp, "|  MONTH TO DATE    |   YEAR TO DATE    |CURR");
	fprintf (pp, "|  MONTH TO DATE    |   YEAR TO DATE    |\n");

	fprintf (pp, "|  |  NUMBER  |");
	fprintf (pp, "                                        ");
	fprintf (pp, "|    BASE AMOUNT    |    BASE AMOUNT    |CODE");
	fprintf (pp, "|    VALUE LOCAL    |    VALUE LOCAL    |\n");


	fprintf (pp, "|--|----------|");
	fprintf (pp, "----------------------------------------");
	fprintf (pp, "|-------------------|-------------------|----");
	fprintf (pp, "|-------------------|-------------------|\n");

	fprintf (pp, ".PI12\n");
}

void
ProcessCusa (void)
{
	cusa_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cusa_rec.year [0] = (CURR_YR) ? 'C' : 'L';
	cc = find_rec (cusa, &cusa_rec, COMPARISON, "r");
	if (!cc)
		StoreData ();
}

void
StoreData (void)
{
	char	data_str [110];

	CalcSales ();

	strcpy (pocr_rec.co_no, comm_rec.co_no);
	strcpy (pocr_rec.code,  cumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		pocr_rec.ex1_factor	= 1.00;

	if (pocr_rec.ex1_factor == 0.00)
		pocr_rec.ex1_factor	= 1.00;
		
	locMtdSales = fgnMtdSales / pocr_rec.ex1_factor;
	locYtdSales = fgnYtdSales / pocr_rec.ex1_factor;

	if (locMtdSales != 0.00 || locYtdSales != 0.00)
	{

		sprintf (data_str,"%014.2f %014.2f %014.2f %014.2f %6ld\n",
			(MTD) ? (fgnMtdSales) : (fgnYtdSales),
			(MTD) ? (fgnYtdSales) : (fgnMtdSales),
			(MTD) ? (locMtdSales) : (locYtdSales),
			(MTD) ? (locYtdSales) : (locMtdSales),
			cumr_rec.hhcu_hash);

		sort_save (fsort,data_str);
		numberSaved++;
	}
}

/*
 * Calculate sales.
 */
void
CalcSales (void)
{
	int		i = 0;
	int		period = 0;

	DateToDMY (comm_rec.dbt_date, NULL, &endMonth, NULL);
	startMonth = comm_rec.fiscal + 1;

	fgnMtdSales += cusa_val [endMonth - 1];
	if (endMonth < startMonth)
		endMonth += 12;

	for (i = startMonth; i <= endMonth; i++)
	{
		period = i % 12;
		if (period == 0)
			period = 12;
		
		fgnYtdSales += cusa_val [period - 1];
	}
}

/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input                        
 */
int
heading (
	int		scn)
{
	if (restart) 
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlDbMess184),25,0,1);

	box (0,2,80,(typeFlag == NORMAL) ? 8 : 10);
	line_at (1,0,80);
	line_at (7,1,79);
	line_at (19,0,80);

	if (typeFlag != NORMAL)
		line_at (9,1,79);

	print_at (20,0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_short);
	print_at (21,0,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}
