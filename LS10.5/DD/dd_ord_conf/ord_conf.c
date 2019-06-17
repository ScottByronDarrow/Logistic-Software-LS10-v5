/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ord_conf.c,v 5.3 2001/11/06 03:04:29 scott Exp $
|  Program Name  : (dd_ord_conf.c)
|  Program Desc  : (Direct Delivery Order Confirmation Print Program. 
|---------------------------------------------------------------------|
|  Date Written  : 24/06/94        | Author      : Anneliese Allen.   |
|---------------------------------------------------------------------|
| $Log: ord_conf.c,v $
| Revision 5.3  2001/11/06 03:04:29  scott
| Updated from testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ord_conf.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DD/dd_ord_conf/ord_conf.c,v 5.3 2001/11/06 03:04:29 scott Exp $";

#include	<pslscr.h>
#include	<string.h>
#include	<twodec.h>

#include	"schema"

struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct cumrRecord	cumr_rec;
struct ddshRecord	ddsh_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
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

/** Page setup **/
#define	LINES		66	/* Lines on a page */
#define	HEADER		22	/* Lines in the header */
#define	FOOTER		14	/* Lines in the footer */

#define	HEADERBRK	HEADER			/* End line for header */
#define	FOOTERBRK	 (LINES - FOOTER)	/* Start line for footer */

	int		printerNo	= 0,
			lineNumber 	= 0,
			pageNumber 	= 0,
			firstTime 	= TRUE;
	
	long	prevHhdsHash	=	0L;

	char	statusFlag [2],
			programName [12];

	FILE	*format,
			*pout;		/*stdlib*/

/*
 * Local Function Prototypes
 */
void    OpenDB 				 (void);
void    CloseDB 			 (void);
FILE*   InitPrintOut 		 (void);
void    EndPrintOut 		 (void);
void    SkipTo 				 (FILE *, int);
void    Header 				 (void);
void    Footer 				 (void);
void    CheckBreak 			 (void);
void    ProcHeader 			 (long);
void    ProcDetail 			 (void);
void    PrintLine 			 (void);
void    ReportEnd 			 (void);
int     check_page 			 (void);
int     IsMultiPrint 		 (void);

/*
 * Main Processing Routine.
 */
int
main (
 int    argc,
 char*  argv [])
{
	long	hashVal;
	char	*sptr;

	if (scanf ("%d", &printerNo) == EOF || scanf ("%s", statusFlag) == EOF)
		return (EXIT_FAILURE);
	
	if (statusFlag [0] == 's')
		statusFlag [0] = 'S';

	if (statusFlag [0] == 'm')
		statusFlag [0] = 'M';

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	sprintf (programName, "%-11.11s", sptr);

	/*
	 * Open format file
	 */
	if (! (format = pr_open ("dd_ord_conf.p")))
	{
		sys_err ("!fopen (dd_ord_conf.p)", errno, PNAME);
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

	/*
	 * Keep on processing until last 0
	 */
	while (scanf ("%ld", &hashVal) != EOF)
	{
		if (hashVal)
			ProcHeader (hashVal);
	}

	/*
	 * Clean up
	 */
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
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (ddhr, ddhr_list, DDHR_NO_FIELDS, "ddhr_hhdd_hash");
	open_rec (exsf, exsf_list, EXSF_NO_FIELDS, "exsf_id_no");
	if (!strcmp (programName, "dd_ord_conf"))
	{
		open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_id_no");
		open_rec (ddsh, ddsh_list, DDSH_NO_FIELDS, "ddsh_id_no3");
	}
	else
	{
		open_rec (ddln, ddln_list, DDLN_NO_FIELDS, "ddln_hhds_hash");
		open_rec (ddsh, ddsh_list, DDSH_NO_FIELDS, "ddsh_hhds_hash");
	}
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (exsf);

	abc_dbclose (data);
}

/*
 * Printer initialization and termination stuff
 */
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

/*
 * Print utilities
 */
void
SkipTo (
 FILE*  pOut,
 int    newLine)
{
	fprintf (pOut, ".B%d\n", newLine - lineNumber);
	lineNumber = newLine;
}

/*
 * Header and footer code
 */
void
Header (void)
{
	pageNumber++;

	pr_format (format, pout, "HMARGIN",		0, 0);

	pr_format (format, pout, "ORD_CONF",	0, 0);
	pr_format (format, pout, "DATE",		1, ddhr_rec.dt_raised);
	pr_format (format, pout, "MARGIN1",		0, 0);
	pr_format (format, pout, "REF_NO",		1, ddhr_rec.order_no);
	pr_format (format, pout, "REF_NO",		2, ddsh_rec.ship_no);
	pr_format (format, pout, "MARGIN2",		0, 0);

	pr_format (format, pout, "CUST",		1, ddhr_rec.cus_ord_ref);
	pr_format (format, pout, "CUST_NAME",	1, cumr_rec.dbt_name);
	pr_format (format, pout, "CUST_ADR1",	1, cumr_rec.ch_adr1);
	pr_format (format, pout, "CUST_ADR2",	1, cumr_rec.ch_adr2);
	pr_format (format, pout, "CUST_ADR3",	1, cumr_rec.ch_adr3);
	pr_format (format, pout, "CUST_ADR3",	2, exsf_rec.salesman);
	pr_format (format, pout, "CUST_ADR4",	1, cumr_rec.ch_adr4);
	pr_format (format, pout, "CUST_ADR4",	2, ddhr_rec.op_id);

	pr_format (format, pout, "MARGIN3",		0, 0);
	pr_format (format, pout, "HEADER",		0, 0);
	pr_format (format, pout, "MARGIN4",		0, 0);

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

/*
 * Major processing routines on headers & detail lines
 */
void
ProcHeader (
	long	hashVal)
{
	firstTime = TRUE;

	if (!strcmp (programName, "dd_ord_conf"))
	{
		ddhr_rec.hhdd_hash	=	hashVal;
		cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, ddhr, "DBFIND");
	}
	else
	{
		ddsh_rec.hhds_hash = hashVal;
		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "r");
		if (cc)
			file_err (cc, ddhr, "DBFIND");

		ddhr_rec.hhdd_hash	=	ddsh_rec.hhdd_hash;
		cc = find_rec (ddhr, &ddhr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, ddhr, "DBFIND");
	}
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
	ReportEnd ();	/* Print out report closure */
}

void
ProcDetail (void)
{
	int		cc2 = 0,
			cc3,
			cc4;

	/*
	 * If supplier confirmation only find ddlns for this 
	 * shipment record, else process all detail lines. 
	 */
	if (!strcmp (programName, "dd_sup_conf"))
	{
		ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
		cc = find_rec (ddln, &ddln_rec, GTEQ, "r");
	}
	else
	{
		ddln_rec.hhdd_hash = ddhr_rec.hhdd_hash;
		ddln_rec.line_no = 0L;
		cc = find_rec (ddln, &ddln_rec, GTEQ, "r");
	}
	while (!cc && ((!strcmp (programName, "dd_ord_conf")) ?
					ddln_rec.hhdd_hash == ddhr_rec.hhdd_hash :
					ddln_rec.hhds_hash == ddsh_rec.hhds_hash))
	{
		if (!strcmp (programName, "dd_ord_conf"))
		{
			ddsh_rec.hhdd_hash = ddln_rec.hhdd_hash;
			ddsh_rec.hhds_hash = ddln_rec.hhds_hash;
			cc4 = find_rec (ddsh, &ddsh_rec, EQUAL, "r");
			if (cc2)
				file_err (cc4, ddsh, "DBFIND");
		}

		inmr_rec.hhbr_hash	=	ddln_rec.hhbr_hash;
		cc2 = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc2)
			file_err (cc2, inmr, "DBFIND");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc3 = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc3)
			file_err (cc3, inum, "DBFIND");

		if (!firstTime && prevHhdsHash	!= ddsh_rec.hhds_hash)
			ReportEnd ();	/* Print out report closure */

		PrintLine ();

		if (firstTime)
			firstTime = FALSE;

		prevHhdsHash = ddsh_rec.hhds_hash;
		cc = find_rec (ddln, &ddln_rec, NEXT, "r");
	}
}

void
PrintLine (void)
{
	CheckBreak ();	/* for page breaks */

	pr_format (format, pout, "ORD_LINE", 1, inmr_rec.item_no);
	pr_format (format, pout, "ORD_LINE", 2, ddln_rec.item_desc);
	pr_format (format, pout, "ORD_LINE", 3, ddln_rec.q_order);
	pr_format (format, pout, "ORD_LINE", 4, inum_rec.uom);
	pr_format (format, pout, "ORD_LINE", 5, DOLLARS (ddln_rec.sale_price));
	lineNumber++;
}

void
ReportEnd (void)
{
	SkipTo (pout, FOOTERBRK);

	pr_format (format, pout, "TERMS", 0, 0);
	lineNumber++;

	pr_format (format, pout, "CURRENCY", 1, cumr_rec.curr_code);
	pr_format (format, pout, "CURRENCY", 2, ddsh_rec.vessel);
	lineNumber++;

	pr_format (format, pout, "SELL_TERM", 1, "   ");
	pr_format (format, pout, "SELL_TERM", 2, ddsh_rec.ship_depart);
	pr_format (format, pout, "SELL_TERM", 3, ddsh_rec.ship_arrive);
	lineNumber++;

	pr_format (format, pout, "PAY_TERM", 1, ddhr_rec.pay_term);
	pr_format (format, pout, "PAY_TERM", 2, ddsh_rec.mark0);
	lineNumber++;

	pr_format (format, pout, "LOAD_PORT", 1, ddsh_rec.port_orig);
	pr_format (format, pout, "LOAD_PORT", 2, ddsh_rec.mark1);
	lineNumber++;

	pr_format (format, pout, "DISC_PORT", 1, ddsh_rec.port_dsch);
	pr_format (format, pout, "DISC_PORT", 2, ddsh_rec.mark2);
	lineNumber++;

	pr_format (format, pout, "FIN_DEST", 1, ddsh_rec.port_dest);
	pr_format (format, pout, "FIN_DEST", 2, ddsh_rec.mark3);
	lineNumber++;

	pr_format (format, pout, "MARK5", 1, ddsh_rec.mark4);
	lineNumber++;

	pr_format (format, pout, "MARGIN6", 0, 0);
	lineNumber++;

	pr_format (format, pout, "COMM1", 1, ddhr_rec.din_1);
	lineNumber++;

	pr_format (format, pout, "COMM2", 1, ddhr_rec.din_2);
	lineNumber++;

	pr_format (format, pout, "COMM3", 1, ddhr_rec.din_3);
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
