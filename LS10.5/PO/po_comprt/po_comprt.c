/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_comprt.c,v 5.2 2001/08/09 09:15:21 scott Exp $
|  Program Name  : (po_comprt.c   )                                   |
|  Program Desc  : (Print Purchase Order Completion Report.     )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohr, poln, inmr, sumr,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
|  Date Modified : (21/01/88)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (09/02/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (02/07/92)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (04/09/1997)    | Modified  by  : Jiggs A Veloz.   |
|                                                                     |
|  Comments      : Removed poln_land_cost from  program.              |
|                : (09/02/91) - Updated to only print current br po's |
|                : (02/07/92) - Updated to add .PI12 S/C DPL-6515     |
|                :                                                    |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 11.      |
|                :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: po_comprt.c,v $
| Revision 5.2  2001/08/09 09:15:21  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:20  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:32  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:06  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.10  1999/12/06 01:32:31  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.9  1999/11/05 05:17:08  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.8  1999/10/20 02:06:56  nz
| Updated for final changes on date routines.
|
| Revision 1.7  1999/10/14 03:04:21  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.6  1999/09/29 10:11:53  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/21 04:37:56  scott
| Updated from Ansi project
|
| Revision 1.4  1999/06/17 10:06:18  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_comprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_comprt/po_comprt.c,v 5.2 2001/08/09 09:15:21 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>

	int		printerNumber = 1;

	double	qtyReceipt 	= 0.0,
			qtyOrder 	= 0.0,
			qtyNow 		= 0.0;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inumRecord	inum_rec;

FILE	*pout;

/*======================= 
| Function Declarations |
=======================*/
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	EndReport 		 (void);
int 	ProcessFile 	 (void);
int 	ProcessPoln 	 (long);
int 	heading 		 (void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	if (argc < 2)
	{
		/*------------------------
		|Usage %s <lp_no> \007\n\r|
		-------------------------*/
		print_at (0,0, mlStdMess036,argv[0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv[1]);

	OpenDB ();

	dsp_screen ("Processing Purchases Completion Report.",comm_rec.co_no,comm_rec.co_name);
	heading ();

	cc = ProcessFile ();

	EndReport ();
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
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
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
| Process whole pohr file for company. printing any records which     |
| have goods received but are not marked as complete.                 |
| Returns: 0 if ok, non-zero if not ok.                               |
=====================================================================*/
int
ProcessFile (
 void)
{
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.pur_ord_no,"               ");

	cc = find_rec (pohr, &pohr_rec, GTEQ, "r");
	while (!cc && !strcmp (pohr_rec.co_no, comm_rec.co_no)
	           && !strcmp (pohr_rec.br_no, comm_rec.est_no))
	{
		dsp_process ("Order:", pohr_rec.pur_ord_no);
		if (pohr_rec.status[0] != 'D')
		{
			if (ProcessPoln (pohr_rec.hhpo_hash))
			{
				fprintf (pout,"|---------------");
				fprintf (pout,"|----------");
				fprintf (pout,"|----------------");
				fprintf (pout,"|-----");
				fprintf (pout,"|-----------");
				fprintf (pout,"|-----------");
				fprintf (pout,"|-----------");
				fprintf (pout,"|---------");
				fprintf (pout,"|------");
				fprintf (pout,"|--------------|\n");
				fflush (pout);
			}
		}
		cc = find_rec (pohr, &pohr_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
ProcessPoln (
 long	HHPO_HASH)
{
	float	qty_now = 0.00,
	     	percent = 0.00;

	float	StdCnvFct 	= 0.00,
			PurCnvFct 	= 0.00,
			CnvFct		= 0.00;

	int	printed = FALSE ;

	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
	if (cc)
	{
		strcpy (sumr_rec.crd_no,"000000");
		strcpy (sumr_rec.acronym,"UNKNOWN");
	}

	poln_rec.hhpo_hash	=	HHPO_HASH;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && HHPO_HASH == poln_rec.hhpo_hash)
	{
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec,EQUAL,"r");
		if (cc) 
			strcpy (inmr_rec.item_no,"Unknown part no.");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		CnvFct	=	StdCnvFct / PurCnvFct;

		qty_now = poln_rec.qty_ord - poln_rec.qty_rec;
		if (poln_rec.qty_ord != 0.00)
			percent = (poln_rec.qty_rec / poln_rec.qty_ord) * 100;
		else
			percent = 0.00;

		fprintf (pout,"|%-15.15s|",pohr_rec.pur_ord_no);
		fprintf (pout," %-9.9s|",  sumr_rec.acronym);
		fprintf (pout,"%-16.16s|", inmr_rec.item_no);
		fprintf (pout," %-4.4s|",  inum_rec.uom);
		fprintf (pout,"%10.2f |",  poln_rec.qty_ord * CnvFct);
		fprintf (pout,"%10.2f |",  poln_rec.qty_rec * CnvFct);
		fprintf (pout,"%10.2f |",  qty_now * CnvFct);
		fprintf (pout,"  %6.2f |", percent);
		if (poln_rec.pur_status[0] == 'C')
			fprintf (pout," YES  ");
		else
			fprintf (pout,"  NO  ");

		switch (poln_rec.pur_status[0])
		{
			case 'O' :
				fprintf (pout,"|PO OUTSTANDING|\n");
				break;
			case 'U' :
				fprintf (pout,"| UNAPPROVED.  |\n");
				break;
			case 'R' :
				fprintf (pout,"|GOODS RECEIVED|\n");
				break;
			case 'C' :
				fprintf (pout,"| GOODS COSTED |\n");
				break;
			case 'D' :
				fprintf (pout,"|TO BE DELETED.|\n");
				break;
			default : 
				fprintf (pout,"| UNKNOWN STAT.|\n");
				break;
		}
		fflush (pout);
		qtyOrder += poln_rec.qty_ord;
		qtyReceipt += poln_rec.qty_rec;
		qtyNow += qty_now;
		printed = TRUE;

		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	return (printed);
}

int
heading (
 void)
{
	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout,".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout,".LP%d\n", printerNumber);
	fprintf (pout,".PI12\n");
	fprintf (pout,".12\n");
	fprintf (pout,".L158\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".EPURCHASE ORDER COMPLETION LOG\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n", comm_rec.co_short,SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s / %s\n", clip (comm_rec.est_name), comm_rec.cc_name);
			
	fprintf (pout,".R================");
	fprintf (pout,"===========");
	fprintf (pout,"=================");
	fprintf (pout,"======");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"==========");
	fprintf (pout,"=======");
	fprintf (pout,"================\n");

	fprintf (pout,"================");
	fprintf (pout,"===========");
	fprintf (pout,"=================");
	fprintf (pout,"======");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"==========");
	fprintf (pout,"=======");
	fprintf (pout,"================\n");

	fprintf (pout,"|      P.O.     ");
	fprintf (pout,"| SUPPLIER ");
	fprintf (pout,"|     PART       ");
	fprintf (pout,"| UOM ");
	fprintf (pout,"|  QUANTITY ");
	fprintf (pout,"|  QUANTITY ");
	fprintf (pout,"|  QUANTITY ");
	fprintf (pout,"| PERCENT ");
	fprintf (pout,"|COSTED");
	fprintf (pout,"|PURCHASE ORDER|\n");

	fprintf (pout,"|     NUMBER    ");
	fprintf (pout,"| ACRONYM  ");
	fprintf (pout,"|    NUMBER      ");
	fprintf (pout,"|     ");
	fprintf (pout,"|  ORDERED  ");
	fprintf (pout,"|  RECEIVED ");
	fprintf (pout,"|OUTSTANDING");
	fprintf (pout,"| COMPLETE");
	fprintf (pout,"|      ");
	fprintf (pout,"| LINE STATUS  |\n");

	fprintf (pout,"|---------------");
	fprintf (pout,"|----------");
	fprintf (pout,"|----------------");
	fprintf (pout,"|-----");
	fprintf (pout,"|-----------");
	fprintf (pout,"|-----------");
	fprintf (pout,"|-----------");
	fprintf (pout,"|---------");
	fprintf (pout,"|------");
	fprintf (pout,"|--------------|\n");
		
	fflush (pout);
	return (EXIT_SUCCESS);
}

void
EndReport (
 void)
{
	float	percent = 0.00;

	if (qtyOrder != 0.00)
		percent = (float) ((qtyReceipt / qtyOrder) * 100);
	else
		percent = 0.00;

	fprintf (pout,"|               ");
	fprintf (pout,"|          ");
	fprintf (pout,"|  %-18.18s  ","TOTAL OUTSTANDING ");
	fprintf (pout,"|%10.2f ",  qtyOrder);
	fprintf (pout,"|%10.2f ",  qtyReceipt);
	fprintf (pout,"|%10.2f ",  qtyNow);
	fprintf (pout,"|  %6.2f ", percent);
	fprintf (pout,"|      ");
	fprintf (pout,"|              |\n");
	fprintf (pout,".EOF\n");
	pclose (pout);
}
