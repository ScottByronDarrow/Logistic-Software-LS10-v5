/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_16wkprt.c,v 5.3 2001/10/19 02:39:44 cha Exp $
|  Program Name  : ( po_16wkprt.c   )                                 |
|  Program Desc  : ( Creditors 16 week projection.                )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohr, poln, inmr, sumr,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pohr, poln,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
|  Date Modified : (13/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (17/04/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (13/12/93)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Change Printing of land_cost from DOLLARS.         |
|  (13/12/93)    : HGP 9504 Update for Direct Deliveries.             |
|  (13/09/97)    : Updated for Multilingual Conversion.               |
|                :                                                    |
|                                                                     |
| $Log: po_16wkprt.c,v $
| Revision 5.3  2001/10/19 02:39:44  cha
| Fix Issue # 00627 by Scott
|
| Revision 5.2  2001/08/09 09:15:12  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:27  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:58  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:18  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:08  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:24  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:04:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:32:25  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/05 05:17:06  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.10  1999/10/14 03:04:19  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/09/29 10:11:50  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:37:50  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:06  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_16wkprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_16wkprt/po_16wkprt.c,v 5.3 2001/10/19 02:39:44 cha Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>

#define	NOHEADLINES	4

	char	*data	= "data";
 
	int		lpno = 1;	/* Line printer number			*/

	long	start_date,
			lastsumr,	/* Hash of last supplier record	*/
			last_hash,	/* Hash of last pohr record	*/
			wk_start,
			wk_end;

	double	gt_qty = 0.0,
			gt_amt = 0.0,
			wk_qty,
			wk_amt;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inumRecord	inum_rec;

	char	*head [NOHEADLINES] = {
			"=====================================================================================",
			"|   DATE   |     PART       | UOM. |QUANTITY.|  AMOUNT  | SUPPLIER |      P.O.      |",
			"|   DUE    |    NUMBER      |      |         |OF SHIPMNT| ACRONYM  |     NUMBER     |",
			"|----------|----------------|------|---------|----------|----------|----------------|"
		},
			*blnk_lin = "|          |                |      |         |          |          |                |";

FILE	*pout;


/*======================= 
| Function Declarations |
=======================*/
void 	EndReport 	(void);
void 	OpenDB 		(void);
void 	CloseDB 	(void);
int 	ProcessFile (void);
int 	ProcPoln 	(void);
int 	heading 	(void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 3) 
	{
		print_at (0,0, mlPoMess721, argv [0]);
        return (EXIT_FAILURE);
	}

	lpno = atoi (argv [1]);
	start_date = StringToDate (argv [2]);

	OpenDB ();

	dsp_screen ("Processing 16 week projection.",
				 comm_rec.co_no,
				 comm_rec.co_name);

	heading ();

	cc = ProcessFile ();

	EndReport ();
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inum);
	abc_dbclose (data);
}


/*==========================================================================
| Process whole poln file for each 1 week period for all 16 weeks.         |
| Printing any records which are fall into the week start and end dates.   |
| Returns: 0 if ok,non-zero if not ok.                                    |
==========================================================================*/
int
ProcessFile (
 void)
{
	int	i;
	int	rcode = 0;

	lastsumr = 0L;

	/*-----------
	| 16 weeks. |
	-----------*/
	for (i = 1; i < 17; i++) 
	{	
		wk_start 	= start_date + (7 * (i - 1));
		wk_end 		= wk_start + 6;
		wk_qty 		= wk_amt = 0.0;
		fprintf (pout, "| WEEK %2d  |END  %10.10s |%s\n",
					   i, DateToString (wk_end), blnk_lin + 29);

		dsp_process ("Item : ", inmr_rec.item_no);
		last_hash = 0L;
		/*----------------------- 
		| Read whole poln file. |
		-----------------------*/
		poln_rec.hhpo_hash	=	0L;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc) 
		{
			if (poln_rec.hhpo_hash != last_hash) 
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, COMPARISON, "r");
				if (!cc && pohr_rec.status [0] != 'D')
					last_hash = poln_rec.hhpo_hash;
			}
			if (!strcmp (pohr_rec.co_no, comm_rec.co_no))
			{
				if (poln_rec.due_date >= wk_start && 
					poln_rec.due_date <= wk_end)
				{
					cc = ProcPoln ();
					if (cc) 	
					{
						rcode = cc;
						break;
					}
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		fprintf (pout, "|          |TOTAL THIS WEEK |      |%9.0f|%10.2f|%s\n%s\n",
					   wk_qty,
					   wk_amt,
					   blnk_lin + 57,
					   head [3]);
		gt_qty += wk_qty;
		gt_amt += wk_amt;
	}

	return (rcode);
}

int
ProcPoln (
 void)
{
	double	wk_qty 		= 0.00,
			balance 	= 0.00;

	float	StdCnvFct 	= 0.00,
			PurCnvFct 	= 0.00,
			CnvFct		= 0.00;

	/*------------------------
	| Change of supplier no. |
	------------------------*/
	if (lastsumr != pohr_rec.hhsu_hash) 
	{
		sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			strcpy (sumr_rec.crd_no, "000000");
			strcpy (sumr_rec.acronym, "UNKNOWN");
		}
		else 
			lastsumr = pohr_rec.hhsu_hash;
	}

	/*------------------
	| Get part number. |
	------------------*/
	inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inmr_rec.item_no, "Unknown part no.");
		StdCnvFct	=	1.00;
	}
	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float)((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float)((cc) ? 1.00 : inum_rec.cnv_fct);

	CnvFct	=	StdCnvFct / PurCnvFct;
	wk_qty 	= (double) (poln_rec.qty_ord - poln_rec.qty_rec);
	if (wk_qty <= 0.00)
		return (EXIT_SUCCESS);

	balance = twodec (poln_rec.land_cst);
	balance = out_cost (balance, inmr_rec.outer_size); 
	balance *= wk_qty;

	fprintf (pout, "|%10.10s|", DateToString (poln_rec.due_date));
	fprintf (pout, "%-16.16s|", inmr_rec.item_no);
	fprintf (pout, " %-4.4s |", inum_rec.uom);
	fprintf (pout, "%9.2f|", wk_qty * CnvFct);
	fprintf (pout, "%10.2f|",balance);
	fprintf (pout, " %-9.9s|", sumr_rec.acronym);
	fprintf (pout, "%-15.15s%c|\n", pohr_rec.pur_ord_no, 
				   pohr_rec.drop_ship [0] == 'Y' ? '*' : ' ');

	wk_qty += wk_qty;
	wk_amt += balance;

	return (EXIT_SUCCESS);
}

/*===================================================================
| Routine to open the pipe to standard print and send initial data. |
===================================================================*/
int
heading (
 void)
{
	int	i;

	if ((pout = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout, ".LP%d\n", lpno);
	fprintf (pout, ".11\n");
	fprintf (pout, ".L92\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ESUPPLIER P/O 16 WEEK PROJECTION\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".E%s AS AT %s\n", comm_rec.co_short, SystemTime());
	fprintf (pout, ".B1\n");
	fprintf (pout, "P/O's marked * are Direct Deliveries. Stock is shipped directly to customer.\n");
	fprintf (pout, ".R%s\n", head [0]);
	for (i = 0; i < NOHEADLINES;i++) 
		fprintf (pout, "%s\n", head [i]);

    return (EXIT_SUCCESS);
}

/*===================================================
| Routine to end report. Prints bottom line totals. |
===================================================*/
void
EndReport (
 void)
{
	fprintf (pout,"|          |%-16.16s|      |%9.0f|%10.2f|          |                |\n.EOF\n", "TOTAL BACKLOG", gt_qty, gt_amt);
	pclose (pout);
}
