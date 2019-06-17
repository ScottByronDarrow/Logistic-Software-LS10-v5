/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_purlog.c,v 5.3 2001/10/19 03:02:35 cha Exp $
|  Program Name  : (po_purlog.c   )                                   |
|  Program Desc  : (Print Purchase Order Log.                   )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohr, poln, inmr, sumr,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (podb)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (podb)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
|  Date Modified : (06/08/87)      | Modified  by  : Scott B. Darrow. |
|  Date Modified : (13/12/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (17/04/89)      | Modified  by  : Huon Butterworth |
|                                                                     |
|  Comments      : Included item description in report ;              |
|                : Expanded the report format so that the details     |
|                : aren't crammed up together.                        |
|                : (17/04/89) Changed MONEYTYPEs to DOUBLETYPEs.      |
|                                                                     |
| $Log: po_purlog.c,v $
| Revision 5.3  2001/10/19 03:02:35  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:16:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:11  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:54  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:10  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:38  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:28  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:32:45  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/10/14 03:04:24  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.7  1999/09/29 10:12:07  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/21 04:38:08  scott
| Updated from Ansi project
|
| Revision 1.5  1999/06/17 10:06:33  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_purlog.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_purlog/po_purlog.c,v 5.3 2001/10/19 03:02:35 cha Exp $";

#define		NO_SCRGEN
#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#define	NOHEADLINES	4

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inumRecord	inum_rec;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
 
	int		printerNumber = 1;	/* Line printer number			*/

	long	startDate	=	0L,
			endDate		=	0L;

	float	purchaseQuantity = 0.00;
	double	purchaseAmount 	 = 0.00;

	char	*head [NOHEADLINES] = {
			"===========================================================================================================================================",
			"|   DATE   |      ITEM        |                   ITEM                   | UOM. |  QUANTITY  |   AMOUNT   |  SUPPLIER  |       P.O.       |",
			"|   DUE    |     NUMBER       |                DESCRIPTION               | UOM. |            | OF SHIPMNT |  ACRONYM   |      NUMBER      |",
			"|----------|------------------|------------------------------------------|------|------------|------------|------------|------------------|"
		};

    FILE	*pout;


/*======================= 
| Function Declarations |
=======================*/
void	OpenDB 			(void);
void	CloseDB 		(void);
void	ProcessFile 	(void);
void	ReportHeading 	(char *, char *);
void	EndReport 		(void);
int		ProcessPoln 	(long);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 4)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		print_at (1,0,mlPoMess702);
		print_at (2,0,mlPoMess703);
		return (EXIT_FAILURE);
	}

	printerNumber 	= atoi (argv [1]);
	startDate 		= StringToDate (argv [2]);
	if (startDate < 0L)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		print_at (1,0,mlPoMess702);
		print_at (2,0,mlPoMess703);

		print_at (0,0,mlStdMess111);
		return (EXIT_FAILURE);
	}

	endDate = StringToDate (argv [3]);
	if (endDate < 0L)
	{
		print_at (0,0,mlStdMess036,argv [0]);
		print_at (1,0,mlPoMess702);
		print_at (2,0,mlPoMess703);
		print_at (0,0,mlStdMess111);
		return (EXIT_FAILURE);
	}
	endDate = StringToDate (argv [3]);

	OpenDB ();

	dsp_screen (" Processing Purchases Log.",comm_rec.co_no,comm_rec.co_name);
	ReportHeading (argv [2],argv [3]);

	ProcessFile ();

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
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_co_no");
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
	abc_dbclose ("data");
}

/*=====================================================================
| Process whole pohr file for company. printing any records which are |
| later than startDate and less than or equal to endDate.           |
| Returns: 0 if ok,non-zero if not ok.                               |
====================================================================*/
void
ProcessFile (
 void)
{
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	cc = find_rec ("pohr",&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no))
	{
		if (pohr_rec.status [0] != 'D')
			cc = ProcessPoln (pohr_rec.hhpo_hash);

		cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
	}
}

int
ProcessPoln (
	long	hhpoHash)
{
	float	qty = 0.00;
	double	balance = 0.00;
	double	extend = 0.00;

	float	StdCnvFct 	= 1.00,
			PurCnvFct 	= 1.00,
			CnvFct		= 1.00;
	
	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec ("sumr", &sumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (sumr_rec.crd_no,"000000");
		strcpy (sumr_rec.acronym,"UNKNOWN");
	}

	poln_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec ("poln", &poln_rec, GTEQ, "r");

	while (!cc && poln_rec.hhpo_hash == hhpoHash) 
	{
		if (startDate <= poln_rec.due_date && poln_rec.due_date <= endDate)
		{
			inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
			if (cc) 
				strcpy (inmr_rec.item_no,"Unknown part no.");

			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
			StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

			inum_rec.hhum_hash	=	poln_rec.hhum_hash;
			cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
			PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
			CnvFct	=	StdCnvFct / PurCnvFct;

			dsp_process ("P/O No : ",pohr_rec.pur_ord_no);

			qty = poln_rec.qty_ord - poln_rec.qty_rec;
			if (qty <= 0.00)
			{
				cc = find_rec ("poln", &poln_rec, NEXT, "r");
				continue;
			}
			balance = (double) qty;
			extend = twodec (poln_rec.land_cst);
			extend = out_cost (extend, inmr_rec.outer_size);
			balance *= extend;

			fprintf (pout,"|%10.10s",DateToString (poln_rec.due_date));
			fprintf (pout,"| %-16.16s ",inmr_rec.item_no);
			fprintf (pout,"| %-40.40s ",inmr_rec.description);
			fprintf (pout,"| %-4.4s ", inum_rec.uom);
			fprintf (pout,"| %9.2f  ",qty * CnvFct);
			fprintf (pout,"| %10.2f ",balance);
			fprintf (pout,"| %-9.9s  ",sumr_rec.acronym);
			fprintf (pout,"| %-16.16s |\n",pohr_rec.pur_ord_no);

			purchaseQuantity += qty;
			purchaseAmount += balance; 
		}
		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
    return (EXIT_SUCCESS);
}

void
ReportHeading (
 char	*date1,
 char	*date2)
{
	int	i;

	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout,".LP%d\n",printerNumber);
	fprintf (pout,".13\n");
	fprintf (pout,".L142\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".EPURCHASES LOG\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n",comm_rec.co_short,SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".CFROM: %s to %s\n",date1,date2);
	fprintf (pout,".B1\n");
	fprintf (pout,".R%s\n",head [0]);
	for (i = 0; i < NOHEADLINES;i++) 
		fprintf (pout,"%s\n",head [i]);
}

void
EndReport (
 void)
{
	fprintf (pout,"|          | %-16.16s | %40.40s |      | %9.0f  | %10.2f |            |                  |\n.EOF\n","TOTAL ORDERS"," ",purchaseQuantity,purchaseAmount);
	pclose (pout);

}
