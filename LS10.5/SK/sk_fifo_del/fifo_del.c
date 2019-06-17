/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: fifo_del.c,v 5.4 2001/11/28 10:19:02 cha Exp $
|  Program Name  : (sk_fifo_del.c & sk_fifo_chk.c) 
|  Program Desc  : (Delete fifo records with not enough qtys)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 14/05/88         |
|---------------------------------------------------------------------|
| $Log: fifo_del.c,v $
| Revision 5.4  2001/11/28 10:19:02  cha
| Updated as OpenInei () missing.
|
| Revision 5.3  2001/08/09 09:18:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:53  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:05  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fifo_del.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_fifo_del/fifo_del.c,v 5.4 2001/11/28 10:19:02 cha Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<Costing.h>

#define	FIFO	 (inmr_rec.costing_flag [0] == 'F')
#define	LIFO	 (inmr_rec.costing_flag [0] == 'I')
#define	UP_OK	 (fixReport [0] == 'U')

#include	"schema"

struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;
struct ccmrRecord	ccmr_rec;

	char	fixReport [2];
	int		fifoCheckFlag = FALSE;
	int		printerNumber = 1;

	FILE	*ftmp;

/*=======================
| Function Declarations |
=======================*/
void	ProcessIncc		(long);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	CheckEnoughIncf (long, int, float);
void 	AddMissingIncf 	(long, float);
void 	HeadingOutput	(void);

/*======================
| Main Processing Loop |
======================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	fifoCheckFlag = FALSE;

	if (!strncmp (sptr, "sk_fifo_chk",11))
	{
		fifoCheckFlag = TRUE;
	}

	if (fifoCheckFlag)
	{
 		if (argc < 3)
		{
			print_at (0,0,mlSkMess479,argv [0]);
			return (EXIT_FAILURE);
		}
		printerNumber = atoi (argv [1]);
		sprintf (fixReport, "%-1.1s", argv [2]);
		if (fixReport [0] != 'C' && fixReport [0] != 'U')
		{
			print_at (0,0,mlSkMess479,argv [0]);
			return (EXIT_FAILURE);
		}
	}
	init_scr ();

	OpenDB ();

	if (fifoCheckFlag)
	{
		dsp_screen ("Inventory FIFO checking utility.", 
					comm_rec.co_no,comm_rec.co_short);
	}
	else
	{
		dsp_screen ("Inventory FIFO record cleaning utility.", 
					comm_rec.co_no,comm_rec.co_short);
	}
	
	if (fifoCheckFlag)
	{
		/*=================================
		| Open pipe work file to pformat. |
 		=================================*/
		HeadingOutput ();
	}
	
	/*----------------------------------------
	| Process all items for Current Company. |
	----------------------------------------*/
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, "                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		if (!FIFO && !LIFO)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		dsp_process ("Item No",inmr_rec.item_no);

		/*--------------------------
		| Process whole incc file. |
		--------------------------*/
		ProcessIncc (inmr_rec.hhbr_hash);

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	if (fifoCheckFlag)
	{
		fprintf (ftmp,".EOF\n");
		pclose (ftmp);
	}
	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

/*===============================================
| Process all incc records for given hhbr_hash. |
===============================================*/
void
ProcessIncc (
	long	hhbrHash)
{
	incc_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhbr_hash == hhbrHash)
	{
		if (fifoCheckFlag)
		{
			CheckIncfSeq (incc_rec.hhwh_hash);

			if (incc_rec.closing_stock <= 0.00)
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}
			CheckEnoughIncf
			(
				incc_rec.hhwh_hash, 
				(FIFO) ? TRUE : FALSE,
				n_dec (incc_rec.closing_stock, inmr_rec.dec_pt)
			);
		}
		else
		{
			if (incc_rec.closing_stock <= 0.00)
				incc_rec.closing_stock = 0.00;
			
			PurgeIncf
			(
				incc_rec.hhwh_hash, 
				(FIFO) ? TRUE : FALSE,
				incc_rec.closing_stock
			);
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
}

/*===========================
|	Open Database Files.	|
===========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	OpenInei ();
}

/*===========================
|	Close Database Files.	|
===========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ccmr);
	CloseCosting ();
	abc_dbclose ("data");
}

/*============================
| Add Required FIFO records. |
============================*/
void
CheckEnoughIncf (
	long	hhwhHash,
	int		fifoFlag,
	float	passedQty)
{
	float	fifoQty = 0.00;

	cc = FindIncf (hhwhHash,fifoFlag,"u");
	while (!cc && incc_rec.hhwh_hash == incfRec.hhwh_hash)
	{
		fifoQty += n_dec (incfRec.fifo_qty, inmr_rec.dec_pt);
		cc = FindIncf (0L,fifoFlag,"u");
	}
	fifoQty = n_dec (fifoQty, inmr_rec.dec_pt);

	if (passedQty > fifoQty)
		AddMissingIncf (hhwhHash,n_dec ((passedQty - fifoQty),inmr_rec.dec_pt));

	abc_unlock (incf);
	return;
}

/*============================
| Add Required FIFO records. |
============================*/
static	int
CheckFifoDate (
	long	hhwhHash)
{
	long	LastFifo	=	0L,
			ThisFifo	=	0L;

	long	LastSeq		=	0,
			ThisSeq		=	0;

	incfRec.hhwh_hash	=	hhwhHash;
	incfRec.fifo_date	=	0L;
	incfRec.seq_no		=	0;
	 
	cc = find_rec (incf, &incfRec, GTEQ, "u");
	while (!cc && incfRec.hhwh_hash == hhwhHash)
	{
		ThisFifo 	=	incfRec.fifo_date;
		ThisSeq 	=	incfRec.seq_no;
		if (ThisFifo == LastFifo && ThisSeq == LastSeq)
		{
			incfRec.seq_no++;

			cc = abc_update (incf,&incfRec);
			if (cc)
				file_err (cc, incf, "DBUPDATE");
		
			return (CheckFifoDate (hhwhHash));
		}
		else
			abc_unlock (incf);

		LastFifo 	=	incfRec.fifo_date;
		LastSeq 	=	incfRec.seq_no;

		cc = find_rec (incf, &incfRec, NEXT, "u");
	}
	abc_unlock (incf);
	return (EXIT_SUCCESS);
}

void
AddMissingIncf (
	long	hhwhHash,
	float	passedQty)
{
	double	costUsed	=	0.00;
	long	dateUsed	=	0L;

	ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
	if (find_rec (ccmr,&ccmr_rec,COMPARISON,"r"))
		return;

	strcpy (ineiRec.est_no, ccmr_rec.est_no);
	ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
	if (find_rec (inei,&ineiRec,COMPARISON,"r"))
		return;

	if (UP_OK)
	{
		costUsed	=	(ineiRec.std_cost > 0.00) 	? ineiRec.std_cost 
													: ineiRec.last_cost;
		dateUsed	=	(ineiRec.date_lcost) 		? ineiRec.date_lcost 
													: TodaysDate();
		cc 	= 	AddIncf
				(
					hhwhHash,
					dateUsed,
					costUsed,
					costUsed,
					passedQty,
					" ",
					costUsed,
					0.00,
					0.00,
					0.00,
					0.00,
					costUsed,
					"A"
				);
		if (cc)
			file_err (cc, incf, "DBADD");
	}

	fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
	fprintf (ftmp, "| %40.40s ", inmr_rec.description);
	fprintf (ftmp, "| %2.2s | %2.2s ",ccmr_rec.est_no,ccmr_rec.cc_no);
	fprintf (ftmp, "|     %13.2f ", passedQty);
	fprintf (ftmp, "|%10.2f ", costUsed);
	fprintf (ftmp, "|%-10.10s ", DateToString (dateUsed));
	fprintf (ftmp, "|%12.2f |\n", costUsed * passedQty);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	if ((ftmp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".PI12\n");

	fprintf (ftmp, ".11\n");
	fprintf (ftmp, ".L158\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESTOCK FIFO/LIFO RECORD SHORTAGE REPORT.\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());
	fprintf (ftmp, ".E NOTE Stock file (s) %s be updated.\n",
					 (UP_OK) ? "Will" : "Won't");

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==========");
	fprintf (ftmp, "====================");
	fprintf (ftmp, "============");
	fprintf (ftmp, "============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==========");
	fprintf (ftmp, "====================");
	fprintf (ftmp, "============");
	fprintf (ftmp, "============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp, "| BR | WH ");
	fprintf (ftmp, "| QUANTITY SHORTAGE ");
	fprintf (ftmp, "| LAST COST ");
	fprintf (ftmp, "| LCOST DATE");
	fprintf (ftmp, "|   EXTENDED  |\n");

	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|----|----");
	fprintf (ftmp, "|-------------------");
	fprintf (ftmp, "|-----------");
	fprintf (ftmp, "|-----------");
	fprintf (ftmp, "|-------------|\n");
}
