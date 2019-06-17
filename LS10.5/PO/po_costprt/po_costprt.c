/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_costprt.c,v 5.1 2001/08/09 09:15:22 scott Exp $
|  Program Name  : (po_costprt.c   )                                  |
|  Program Desc  : (Print Goods Receipts Report.                )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
| $Log: po_costprt.c,v $
| Revision 5.1  2001/08/09 09:15:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.0  2001/06/19 08:11:12  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:31  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:21  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:33  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:07  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/10 04:14:55  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.11  1999/12/06 01:32:31  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.10  1999/11/05 05:17:08  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.9  1999/10/20 02:06:56  nz
| Updated for final changes on date routines.
|
| Revision 1.8  1999/10/14 03:04:21  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.7  1999/10/13 22:57:02  cam
| Removed max_work
|
| Revision 1.6  1999/09/29 10:11:53  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/21 04:37:57  scott
| Updated from Ansi project
|
| Revision 1.4  1999/06/17 10:06:18  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_costprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_costprt/po_costprt.c,v 5.1 2001/08/09 09:15:22 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	char	selectType[2];
 
	int printerNumber = 1;	/* Line printer number			*/

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct poghRecord	pogh_rec;
struct poglRecord	pogl_rec;
struct poshRecord	posh_rec;
FILE	*pout;

static	char	*fifteenSpaces	=	"               ";

/*======================== 
| Function Declarations. |
========================*/
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcessPogh 		(void);
void 	ProcessFile 		(long);
void 	ProcessPogl 		(long);
void 	ReportHeading 		(void);
void	RuleOffLine 		(void);
char	*GetShipmentNo 		(long);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	long	passedHash;
	if (argc < 4)
	{
		print_at (0,0, mlPoMess719, argv[0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv[1]);
	sprintf (selectType,"%1.1s",argv[2]);
	if (selectType[0] != 'S' && selectType[0] != 'G' && selectType[0] != 'A')
	{
		print_at (0,0, "%s\n\r", ML (mlPoMess093));
		return (EXIT_FAILURE);
	}

	passedHash = atol (argv[3]);

	OpenDB ();

	dsp_screen (" Processing Goods Receipts Report.",comm_rec.co_no,comm_rec.co_name);
	ReportHeading ();

	ProcessFile (passedHash);

	fprintf (pout,".EOF");
	pclose (pout);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
    open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_hhsh_hash");
	open_rec (pogh, pogh_list, POGH_NO_FIELDS, (selectType[0] == 'G')
									? "pogh_hhgr_hash" : "pogh_id_no");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (posh);
	abc_fclose (sumr);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_dbclose ("data");
}

void
ProcessFile (
	long	passedHash)
{

	if (selectType[0] == 'A')
	{
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		pogh_rec.hhsu_hash = 0L;
		strcpy (pogh_rec.gr_no,fifteenSpaces);
		cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
		while (!cc && !strcmp (pogh_rec.co_no, comm_rec.co_no))
		{
			if (pogh_rec.pur_status[0] == 'D')
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			dsp_process ("Order:",pogh_rec.gr_no);
			ProcessPogh ();
			ProcessPogl (pogh_rec.hhgr_hash);

			RuleOffLine ();
			cc = find_rec (pogh, &pogh_rec, NEXT, "r");
		}
	}
	if (selectType[0] == 'G')
	{
		pogh_rec.hhgr_hash = passedHash;
		cc = find_rec (pogh, &pogh_rec, COMPARISON, "r");
		if (!cc)
		{
			dsp_process ("Order:",pogh_rec.gr_no);
			ProcessPogh ();
			ProcessPogl (pogh_rec.hhgr_hash);
			RuleOffLine ();
			fflush (pout);
		}
	}
	if (selectType[0] == 'S')
	{
		pogh_rec.hhsu_hash = passedHash;
		strcpy (pogh_rec.co_no, comm_rec.co_no);
		strcpy (pogh_rec.gr_no,fifteenSpaces);
		cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
		while (!cc && !strcmp (pogh_rec.co_no, comm_rec.co_no) &&
					  pogh_rec.hhsu_hash == passedHash)
		{
			if (pogh_rec.pur_status[0] == 'D')
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			dsp_process ("Order:",pogh_rec.gr_no);
			ProcessPogh ();
			ProcessPogl (pogh_rec.hhgr_hash);
			RuleOffLine ();
			cc = find_rec (pogh, &pogh_rec, NEXT, "r");
		}
	}
}

void
ProcessPogh (
 void)
{
	if (sumr_rec.hhsu_hash != pogh_rec.hhsu_hash)
	{
		sumr_rec.hhsu_hash = pogh_rec.hhsu_hash;
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
			sprintf (sumr_rec.crd_name,"** Not on File **");
	}
	fprintf 
	(
		pout, 
		"| GRIN : %15.15s / Supplier : %-6.6s (%-40.40s / Currency : %3.3s @ %8.4f |\n",
		pogh_rec.gr_no, 
		sumr_rec.crd_no, 
		sumr_rec.crd_name, 
		sumr_rec.curr_code, 
		pogh_rec.exch_rate
	);
	fprintf 
	(
		pout, 
		"| Shipment Number : %-12.12s  /  Date Raised : %-10.10s  /  %-20.20s %28.28s|\n",
		GetShipmentNo (pogh_rec.hhsh_hash), 
		DateToString (pogh_rec.date_raised), 
		(pogh_rec.pur_status[0] == 'P') ? "Costing Commenced" : " ",
		" "
	);
}

void
ProcessPogl (
 long	hhgrHash)
{
	pogl_rec.hhgr_hash	=	hhgrHash;
	pogl_rec.line_no	=	0;
	cc = find_rec (pogl,&pogl_rec,GTEQ,"r");
	while (!cc && pogl_rec.hhgr_hash == hhgrHash) 
	{
		inmr_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc) 
			strcpy (inmr_rec.item_no,"Unknown part no.");

		fprintf (pout, "|%15.15s", pogl_rec.po_number);
		fprintf (pout, "|%16.16s", inmr_rec.item_no);
		fprintf (pout, "|%9.2f ",  pogl_rec.qty_rec);
		fprintf (pout, "|%9.2f ",  pogl_rec.fob_fgn_cst);
		fprintf (pout, "|%9.2f ",  pogl_rec.frt_ins_cst);
		fprintf (pout, "|%9.2f ",  pogl_rec.duty);
		fprintf (pout, "|%9.2f ",  pogl_rec.licence);
		fprintf (pout, "|%10.2f ", out_cost (pogl_rec.land_cst,inmr_rec.outer_size));
		fprintf (pout, "|%4.4s",   inmr_rec.sale_unit);
		fprintf (pout, "|%10.10s|\n", DateToString (pogl_rec.rec_date));

		cc = find_rec (pogl,&pogl_rec,NEXT,"r");
	}
}

void
ReportHeading (
 void)
{
	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout,".LP%d\n",printerNumber);
	fprintf (pout,".PI12\n");
	fprintf (pout,".11\n");
	fprintf (pout,".L158\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".EGOODS RECEIPTS COSTINGS LIST.\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n",comm_rec.co_short,SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s / %s\n",clip (comm_rec.est_name),comm_rec.cc_name);

	fprintf (pout, ".R================");
	fprintf (pout, "=================");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "============");
	fprintf (pout, "=====");
	fprintf (pout, "===========\n");

	fprintf (pout, "================");
	fprintf (pout, "=================");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "============");
	fprintf (pout, "=====");
	fprintf (pout, "===========\n");

	fprintf (pout, "|PURCHASE ORDER ");
	fprintf (pout, "|  ITEM NUMBER   ");
	fprintf (pout, "| QTY RCVD ");
	fprintf (pout, "| FOB (FGN)");
	fprintf (pout, "|  FRT/INS ");
	fprintf (pout, "|    DUTY  ");
	fprintf (pout, "|  LICENCE ");
	fprintf (pout, "| LAND COST ");
	fprintf (pout, "|UNIT");
	fprintf (pout, "|DATE RCVD|\n");

	fprintf (pout, "|---------------");
	fprintf (pout, "|----------------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|-----------");
	fprintf (pout, "|----");
	fprintf (pout, "|---------|\n");

	fflush (pout);
}


char*
GetShipmentNo (
	long    hhshHash)
{
				
	posh_rec.hhsh_hash	=	hhshHash;
	cc = find_rec (posh, &posh_rec, EQUAL, "r");
	if (cc)  
		strcpy (err_str, " N/A ");
	else
		sprintf (err_str, "%-12.12s", posh_rec.csm_no);

	return (err_str);
}           
void
RuleOffLine (void)
{
	fprintf (pout, "|---------------");
	fprintf (pout, "|----------------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|-----------");
	fprintf (pout, "|----");
	fprintf (pout, "|---------|\n");
}

