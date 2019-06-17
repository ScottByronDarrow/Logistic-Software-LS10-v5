/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: dd_qt_prt.c,v 5.4 2001/11/06 03:04:31 scott Exp $
|  Program Name  : (dd_qt_prt.c)
|  Program Desc  : (Direct Delivery Quotation Print)
|---------------------------------------------------------------------|
|  Date Written  :   /  /          | Author      : Anneliese Allen.   |
|---------------------------------------------------------------------|
| $Log: dd_qt_prt.c,v $
| Revision 5.4  2001/11/06 03:04:31  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dd_qt_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_qt_prt/dd_qt_prt.c,v 5.4 2001/11/06 03:04:31 scott Exp $";

#include	<pslscr.h>
#include	<string.h>
#include	<twodec.h>

#include	"schema"

struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct exsfRecord	exsf_rec;

/*
 * Table Names
 */
static char
	*data	= "data";

/*
 * Format line names & Report structures
 */
#include	<pr_format3.h>

#define	LINES		66	/* Lines on a page */
#define	HEADER		22	/* Lines in the header */
#define	FOOTER		15	/* Lines in the footer */

#define	HEADERBRK	HEADER			/* End line for header */
#define	FOOTERBRK	 (LINES - FOOTER)	/* Start line for footer */

	int		printerNo,
			lineNumber = 0,
			pageNumber = 0;

	char	statusFlag [2];
	FILE	*format,
			*pout;		/*stdlib*/

/*=====================================================================
| Local Function Prototypes
=====================================================================*/
void    OpenDB 			(void);
void    CloseDB 		(void);
FILE*   InitPrintOut 	(void);
void    EndPrintOut 	(void);
void    SkipTo 			(FILE *, int);
void    Header 			(void);
void    Footer 			(void);
void    CheckBreak 		(void);
void    ProcHeader 		(long);
void    ProcDetail 		(void);
void    PrintLine 		(void);
void    ReportEnd 		(long);
int     check_page 		(void);
int     IsMultiPrint 	(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	long	hashVal;

	if (scanf ("%d", &printerNo) == EOF || scanf ("%s", statusFlag) == EOF)
		return (EXIT_FAILURE);
	
	if (statusFlag [0] == 's')
		statusFlag [0] = 'S';

	/*------------------
	| Open format file |
	------------------*/
	if (!(format = pr_open ("dd_qt_prt.p")))
	{
		sys_err ("!fopen (dd_qt_prt.p)", errno, PNAME);
		return (-1);
	}

	OpenDB ();

	if (IsMultiPrint ())
	{
		if (scanf ("%ld", &hashVal) == EOF || hashVal)
		{
			sys_err ("Expected leading 0 in MultiPrint", -1, PNAME);
			return (-1);
		}
	}
	pout = InitPrintOut ();		/* Open printer output */

	/*---------------------------------
	| Keep on processing until last 0 |
	---------------------------------*/
	while (scanf ("%ld", &hashVal) != EOF)
	{
		if (hashVal)
			ProcHeader (hashVal);
	}

	/*----------
	| Clean up |
	----------*/
	EndPrintOut ();
	fclose (format);
	CloseDB (); 
	FinishProgram ();

	return (EXIT_SUCCESS);	
}

void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_hhdd_hash");
	open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (exsf);

	abc_dbclose (data);
}

/*============================================
 Printer initialization and termination stuff
=============================================*/
FILE*
InitPrintOut (void)
{
	if ((pout = popen ("pformat", "w")) == NULL)
	{
		sys_err ("!popen (pformat)", errno, PNAME);
		return (NULL);		/*NOTREACHED*/
	}

	fprintf (pout, ".START00/00/00\n");
	fprintf (pout, ".OP\n");
	fprintf (pout, ".PL0\n");
	fprintf (pout, ".LP%d\n", printerNo);
	fprintf (pout, ".2\n");
	fprintf (pout, ".PI12\n");
	fprintf (pout, ".L140\n");

	return (pout);
}	

void
EndPrintOut (void)
{
	fprintf (pout, ".EOF\n");
	pclose (pout);
}

/*==============
 Print utilities
================*/
void
SkipTo (
 FILE*  pOut,
 int    newLine)
{
	fprintf (pOut, ".B%d\n", newLine - lineNumber);
	lineNumber = newLine;
}

/*=====================
 Header and footer code
=======================*/
void
Header (void)
{
	pageNumber++;

	pr_format (format, pout, "HMARGIN",		0, 0);

	pr_format (format, pout, "QUOTE",		0, 0);
	pr_format (format, pout, "DATE",		1, ddhr_rec.dt_raised);
	pr_format (format, pout, "MARGIN1",		0, 0);
	pr_format (format, pout, "QUOTE_NO",	1, ddhr_rec.order_no);
	pr_format (format, pout, "MARGIN2",		0, 0);

	pr_format (format, pout, "CUST",		1, ddhr_rec.cus_ord_ref);
	pr_format (format, pout, "CUST_NAME",	1, cumr_rec.dbt_name);
	pr_format (format, pout, "CUST_ADR1",	1, cumr_rec.ch_adr1);
	pr_format (format, pout, "CUST_ADR2",	1, cumr_rec.ch_adr2);
	pr_format (format, pout, "CUST_ADR3",	1, cumr_rec.ch_adr3);
	pr_format (format, pout, "CUST_ADR4",	1, cumr_rec.ch_adr4);

	pr_format (format, pout, "MARGIN3",		0, 0);
	pr_format (format, pout, "PLEASURE",	0, 0);
	pr_format (format, pout, "MARGIN4",		0, 0);

	pr_format (format, pout, "HEADER",		0, 0);

	pr_format (format, pout, "MARGIN5",		0, 0);

	lineNumber = HEADERBRK;
}

void
Footer (void)
{
	SkipTo (pout, LINES);
}

void
CheckBreak (void)
{
	/*	Check for header/footer breaks
	*/
	if (!lineNumber || lineNumber >= FOOTERBRK)
	{
		if (pageNumber)
			Footer ();

		lineNumber = 0;
		Header ();
	}
}

/*===================================================
 Major processing routines on headers & detail lines
====================================================*/
void
ProcHeader (
	long	hashVal)
{
	ddhr_rec.hhdd_hash	=	hashVal;
	cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ddhr, "DBFIND");

	cumr_rec.hhcu_hash	=	ddhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	strcpy (exsf_rec.co_no, ddhr_rec.co_no);
	strcpy (exsf_rec.salesman_no, ddhr_rec.sman_code);
	cc = find_rec (exsf, &exsf_rec, EQUAL, "r");
	if (cc)
		file_err (cc, exsf, "DBFIND");

	ProcDetail ();
	ReportEnd (hashVal);	/* Print out report closure */
}

void
ProcDetail (void)
{
	int		cc2;

	/*
	 * Process all detail lines.
	 */
	ddln_rec.hhdd_hash 	= ddhr_rec.hhdd_hash;
	ddln_rec.line_no 	= 0L;
	cc = find_rec (ddln, &ddln_rec, GTEQ, "r");
	while (!cc && ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash)
	{
		inmr_rec.hhbr_hash	=	ddln_rec.hhbr_hash;
		cc2 = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc2)
			file_err (cc, inmr, "DBFIND");

		PrintLine ();

		cc = find_rec (ddln, &ddln_rec, NEXT, "r");
	}
}

void
PrintLine (void)
{
	CheckBreak ();	/* for page breaks */

	pr_format (format, pout, "QT_LINE", 1, inmr_rec.item_no);
	pr_format (format, pout, "QT_LINE", 2, ddln_rec.item_desc);
	pr_format (format, pout, "QT_LINE", 3, ddln_rec.q_order);
	pr_format (format, pout, "QT_LINE", 4, DOLLARS (ddln_rec.sale_price));
	lineNumber++;
}

void
ReportEnd (
	long	hashVal)
{
	SkipTo (pout, FOOTERBRK);

	pr_format (format, pout, "TERMS", 0, 0);
	lineNumber++;

	pr_format (format, pout, "CURRENCY", 1, cumr_rec.curr_code);
	lineNumber++;

	pr_format (format, pout, "SELL_TERM", 1, ddhr_rec.sell_terms);
	lineNumber++;

	pr_format (format, pout, "PAY_TERM", 1, ddhr_rec.pay_term);
	lineNumber++;

	pr_format (format, pout, "COMMENTS", 0, 0);
	lineNumber++;

	pr_format (format, pout, "COMM1", 1, ddhr_rec.stdin1);
	lineNumber++;

	pr_format (format, pout, "COMM2", 1, ddhr_rec.stdin2);
	lineNumber++;

	pr_format (format, pout, "COMM3", 1, ddhr_rec.stdin3);
	pr_format (format, pout, "COMM3", 2, ddhr_rec.sman_code);
	lineNumber++;

	SkipTo (pout, LINES);
}

/*
 * Override library routine to suppress page checking 
 */
int
check_page (void)
{
	return (EXIT_SUCCESS);
}

int
IsMultiPrint (void)
{
	return (statusFlag [0] == 'M');
}
