/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: bycatprt.c,v 5.3 2001/10/19 02:49:03 cha Exp $
|  Program Name  : (po_bycatprt.c)                                    |
|  Program Desc  : (Print Backlog by Category.                  )     |	
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, pohr, poln, inmr, inex,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 08/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (09/12/88)      | Modified  by  : Bee Chwee Lim.   |
|  Date Modified : (16/02/89)      | Modified  by  : Deven Nambiar.   |
|  Date Modified : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (24/04/89)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (19/09/90)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (07/07/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Print PNAME on top right of report.                |
|                : Modified program to allow a range of category to   |
|                : be selected as in so_bycatprt.c, use sk_group.i.c  |
|                : as the input program.			                  |
|                : Change Printing of land_cost from DOLLARS.         |
|                : (24/04/89) - Added tests to ignore -vi po (s).      |
|                : (19/09/90) - General Update for New Scrgen. S.B.D. |
|                : (07/07/92) - To include inex desc lines SC DFH 7287|
|  (13/09/97)    : Updated for Multilingual Conversion.				  |
|                :                                                    |
|                                                                     |
| $Log: bycatprt.c,v $
| Revision 5.3  2001/10/19 02:49:03  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:15:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:39  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:12  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:26  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/09 00:50:17  nz
| Due Date now prints on a line level
|
| Revision 2.0  2000/07/15 09:04:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  1999/12/06 01:32:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/10/14 03:04:20  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.7  1999/09/29 10:11:51  scott
| Updated to be consistant on function names.
|
| Revision 1.6  1999/09/21 04:37:54  scott
| Updated from Ansi project
|
| Revision 1.5  1999/06/17 10:06:14  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bycatprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_bycatprt/bycatprt.c,v 5.3 2001/10/19 02:49:03 cha Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct inexRecord	inex_rec;
struct inumRecord	inum_rec;

	int		printerNumber 	= 1,
			firstFlag 		= TRUE,
			firstItem 		= TRUE,
			afterTotal  	= FALSE,
			categoryDetails = 0;

	char	lower [13],
			upper [13],
			startClass [2],
			endClass [2],
			startCategory [12],
			endCategory [12],
			previousCategory [12];

	long	previousHash = 0L;

	float	qty 			  = 0.0,
			totalPoQty 		  = 0.0,
			totalCategoryQty  = 0.0,
			totalGrandQty 	  = 0.0;

	double	extend 			  = 0.00,
			totalShipValue 	  = 0.00,
			totalCatValue  	  = 0.00,
			totalGrandShip 	  = 0.00;

	FILE	*fout;

	float	StdCnvFct 	= 0.00;


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	HeadingOutput 		(void);
void 	PrintLine 			(void);
void 	ProcessFile 		(void);
void 	ProcessPoln 		(long);
void 	PrintPoln 			(long);
void 	PrintCategory 		(void);
void 	PrintTotalCategory 	(void);
void 	PrintInex 			(void);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 4)
	{
		print_at (0,0, mlPoMess722 ,argv [0]);
        return (EXIT_FAILURE);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/

	OpenDB ();

	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);

	sprintf (startClass,"%-1.1s",lower);
	sprintf (endClass,"%-1.1s",upper);
	sprintf (startCategory,"%-11.11s",lower + 1);
	sprintf (endCategory,"%-11.11s",upper + 1);

	dsp_screen ("Processing : Printing Purchase Order By Category.",comm_rec.co_no,comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)",errno,PNAME);
	HeadingOutput ();
	ProcessFile ();
	fprintf (fout,".EOF\n");
	pclose (fout);
	shutdown_prog ();
    return (EXIT_SUCCESS);
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec ("pohr",pohr_list,POHR_NO_FIELDS,"pohr_hhpo_hash");
	open_rec ("poln",poln_list,POLN_NO_FIELDS,"poln_hhbr_hash");
	open_rec ("sumr",sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash");
	open_rec ("inmr",inmr_list,INMR_NO_FIELDS,"inmr_id_no_3");
	open_rec ("inex",inex_list,INEX_NO_FIELDS,"inex_id_no");
	open_rec ("excf",excf_list,EXCF_NO_FIELDS,"excf_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("pohr");
	abc_fclose ("poln");
	abc_fclose ("sumr");
	abc_fclose ("inmr");
	abc_fclose ("inex");
	abc_fclose ("excf");
	abc_fclose (inum);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);

	fprintf (fout,".14\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L140\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".ECompany : %s - %s\n",
				comm_rec.co_no,clip (comm_rec.co_name));
	fprintf (fout,".EBranch  : %s - %s\n",
				comm_rec.est_no,clip (comm_rec.est_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".EPURCHASE ORDER BY CATEGORY\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout,".R=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"===========");
	fprintf (fout,"=======");
	fprintf (fout,"===========");
	fprintf (fout,"==============");
	fprintf (fout,"==========");
	fprintf (fout,"=================\n");

	fprintf (fout,"=================");
	fprintf (fout,"=========================================");
	fprintf (fout,"===========");
	fprintf (fout,"=======");
	fprintf (fout,"===========");
	fprintf (fout,"==============");
	fprintf (fout,"==========");
	fprintf (fout,"=================\n");

	fprintf (fout,"|      PART      ");
	fprintf (fout,"|              DESCRIPTION               ");
	fprintf (fout,"|   DATE   ");
	fprintf (fout,"| UOM. ");
	fprintf (fout,"| QUANTITY ");
	fprintf (fout,"|    LANDED   ");
	fprintf (fout,"| SUPPLIER");
	fprintf (fout,"|      P.O.     |\n");

	fprintf (fout,"|     NUMBER     ");
	fprintf (fout,"|                                        ");
	fprintf (fout,"|   DUE    ");
	fprintf (fout,"|      ");
	fprintf (fout,"| OUTSTDNG ");
	fprintf (fout,"|     COST    ");
	fprintf (fout,"| ACRONYM ");
	fprintf (fout,"|    NUMBER     |\n");
	PrintLine ();
	firstFlag = TRUE;
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------------------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|------");
	fprintf (fout,"|----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|---------");
	fprintf (fout,"|---------------|\n");

	fflush (fout);
}

void
ProcessFile (
 void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.inmr_class,startClass);
	sprintf (inmr_rec.category,"%-11.11s",startCategory);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");

	cc = find_rec ("inmr",&inmr_rec,GTEQ,"r");
	previousHash = 0L;
	strcpy (previousCategory,"           ");

	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && 
		       strcmp (inmr_rec.inmr_class,startClass) >= 0 && 
		       strcmp (inmr_rec.inmr_class,endClass) <= 0)
	{
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		if (strcmp (inmr_rec.category,startCategory) >= 0 && 
		     strcmp (inmr_rec.category,endCategory) <= 0)
		{
			firstItem = TRUE;
			ProcessPoln (inmr_rec.hhbr_hash);
		}

		cc = find_rec ("inmr",&inmr_rec,NEXT,"r");
	}

	if (categoryDetails > 1)
		PrintTotalCategory ();

	/*-----------------------------------------------
	| On last record,print the total backlog.	|
	-----------------------------------------------*/
	PrintLine ();
	fprintf (fout,"| ***** TOTAL PURCHASE ORDER ***** ");
	fprintf (fout,"                       ");
	fprintf (fout,"|          ");
	fprintf (fout,"|      ");
	fprintf (fout,"|%10.2f",totalGrandQty);
	fprintf (fout,"|%13.2f",totalGrandShip);
	fprintf (fout,"|         ");
	fprintf (fout,"|               |\n");
	fflush (fout);
}

void
ProcessPoln (
	long	hhbr_hash)
{
	double	qty_left = 0.0;

	totalPoQty = 0.0;
	totalShipValue = 0.0;

	cc = find_hash ("poln",&poln_rec,GTEQ,"r",hhbr_hash);
	while (!cc && poln_rec.hhbr_hash == hhbr_hash) 
	{
		qty_left = poln_rec.qty_ord - poln_rec.qty_rec;
		if (qty_left > 0.00)
		{
			cc = find_hash ("pohr",&pohr_rec,COMPARISON,"r",poln_rec.hhpo_hash);
			if (cc || pohr_rec.status [0] == 'D')
				return;
			
			dsp_process ("Category : ",inmr_rec.category);
			if (strcmp (previousCategory,inmr_rec.category) != 0)
			{
				if (!firstFlag && categoryDetails > 1)
					PrintTotalCategory ();

				else if (!firstFlag && categoryDetails == 1)
					PrintLine ();

				categoryDetails = 0;
				totalCategoryQty = 0.0;
				totalCatValue = 0.0;
				PrintCategory ();
				strcpy (previousCategory,inmr_rec.category);
			}
			PrintPoln (hhbr_hash);
		}	
		cc = find_hash ("poln",&poln_rec,NEXT,"r",hhbr_hash);
	}
}

void
PrintPoln (
 long hhbr_hash)
{
	float	PurCnvFct 	= 	0.00,
			CnvFct		=	0.00;

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	CnvFct	=	StdCnvFct / PurCnvFct;
	qty = poln_rec.qty_ord - poln_rec.qty_rec;

	extend 		= twodec (poln_rec.land_cst);
	extend 		= out_cost (extend,inmr_rec.outer_size);
	extend 		*= (double) qty;

	totalPoQty 		 += qty;
	totalShipValue 	 += extend;
 
	totalCategoryQty += qty;
	totalCatValue	 += extend;

	totalGrandQty 	 += qty;
	totalGrandShip 	 += extend;

	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	cc = find_rec ("sumr",&sumr_rec,COMPARISON,"r");
	if (cc)
		return;

	if (firstItem)
	{
		fprintf (fout,"|%-16.16s",inmr_rec.item_no);
		fprintf (fout,"|%-40.40s",poln_rec.item_desc);
	}
	else
	{
		fprintf (fout,"|                ");
		fprintf (fout,"|                                        ");
	}
 	fprintf (fout,"|%-10.10s",DateToString (poln_rec.due_date));
	fprintf (fout,"| %4.4s ", inum_rec.uom);
	fprintf (fout,"|%9.2f ",qty * CnvFct);
	fprintf (fout,"|%12.2f ",extend);
	fprintf (fout,"|%-9.9s",sumr_rec.acronym);
	fprintf (fout,"|%15.15s|\n",pohr_rec.pur_ord_no);
	fflush (fout);

	if (firstItem)
		PrintInex ();

	firstItem = FALSE;
	
	categoryDetails++;
}

void
PrintCategory (
 void)
{
	dsp_process ("Category No. : ",inmr_rec.category);
	fprintf (fout,".LRP2\n");
	if (afterTotal)
	{
		PrintLine ();
		afterTotal = FALSE;
	}
	strcpy (excf_rec.co_no,comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec ("excf",&excf_rec,COMPARISON,"r");
	if (cc)
		strcpy (excf_rec.cat_no,"No category description found.");

	fprintf (fout,"|* %-11.11s * ",inmr_rec.category);
	fprintf (fout,"|%-40.40s",excf_rec.cat_desc);
	fprintf (fout,"|          |      |          |             ");
	fprintf (fout,"|         |               |\n");
	firstFlag = FALSE;
}

/*===============================
| Print category totals.	    |
===============================*/
void
PrintTotalCategory (
 void)
{
	PrintLine ();
	fprintf (fout,".LRP2\n");
	fprintf (fout,"|TOTAL FOR CATEGORY : %-11.11s ",previousCategory);
	fprintf (fout,"                        |          ");
	fprintf (fout,"|      |%10.2f",totalCategoryQty);
	fprintf (fout,"|%13.2f",totalCatValue);
	fprintf (fout,"|         ");
	fprintf (fout,"|               |\n");
	afterTotal = TRUE;
}

void
PrintInex (
 void)
{
	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec ("inex", &inex_rec, GTEQ, "r");
	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		fprintf (fout,"|%-16.16s"," ");
		fprintf (fout,"|%-40.40s",inex_rec.desc);
		fprintf (fout,"|%-10.10s"," ");
		fprintf (fout,"| %-4.4s "," ");
		fprintf (fout,"| %8.2s "," ");
		fprintf (fout,"| %11.2s "," ");
		fprintf (fout,"|%-9.9s"," ");
		fprintf (fout,"|%15.15s|\n"," ");
		fflush (fout);
		cc = find_rec ("inex", &inex_rec, NEXT, "r");
	}
}
