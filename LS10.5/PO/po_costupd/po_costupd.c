/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: po_costupd.c,v 5.3 2001/08/09 09:15:23 scott Exp $
|  Program Name  : (po_costupd.c  )                                   |
|  Program Desc  : (Update Goods Actual Costs in Stock.        )      |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author       :                   |
|---------------------------------------------------------------------|
| $Log: po_costupd.c,v $
| Revision 5.3  2001/08/09 09:15:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:36:47  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:28  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_costupd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_costupd/po_costupd.c,v 5.3 2001/08/09 09:15:23 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<MoveRec.h>
#include	<Costing.h>

#define	FIFO		 (inmr_rec.costing_flag [0] == 'F')
#define	LIFO		 (inmr_rec.costing_flag [0] == 'I')
#define	SERIAL 		 (inmr_rec.costing_flag [0] == 'S')
#define	AVGE 		 (inmr_rec.costing_flag [0] == 'A')
#define	ACT_COST	 (envVarPoActCost [0] == 'Y')
#define	HR_COSTED	 (pogh_rec.pur_status [0] == 'C')
#define	DT_COSTED	 (pogl_rec.pur_status [0] == 'C')

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct poghRecord	pogh_rec;
struct poglRecord	pogl_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct pocaRecord	poca_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct pogdRecord	pogd_rec;
struct sumrRecord	sumr_rec;

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/
	char 	*data = "data";

	FILE	*fout;

	int		printerNumber 	= 1,
			missingFifo 	= FALSE,
			summaryReport 	= FALSE,
			envVarPoVarRept = 0;

	char	systemDate [11],
			envVarPoActCost [2],
			*serialSpace 	= "                         ",
			*fifteenSpaces 	= "               ";

	long	lsystemDate		= 0L,
			hhpoHash 		= 0L;

	float	qtySold	= 0.00,
			StdCnvFct 		= 1.00;

	double	oldCost 		= 0.00,
			newCost 		= 0.00,
			outOldCost 		= 0.00,
			outNewCost 		= 0.00,
			grTotalActual 	= 0.00,
			grTotalEstimate = 0.00,
			grTotalGlVar 	= 0.00,
			grTotalSkVar 	= 0.00,
			totalActual  	= 0.00,
			totalEstimate 	= 0.00,
			totalGlVar 		= 0.00,
			totalSkVar 		= 0.00;

#include	<MoveAdd.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	Process 			(void);
void 	ProcessPogl 		(long);
void 	ProcessStock 		(long, long);
void 	PrintTotal 			(char *);
void 	PrintGrand 			(void);
void 	IntFindSupercession (char *);
void 	ProcessAudit 		(void);
void 	OpenAudit 			(void);
void 	CloseAudit 			(void);
void 	PrintLine 			(void);
void 	PrintGrinSummary 	(void);
float 	StockLeft 			(long, float, int, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 2)
	{
		/*-------------------
		| Usage : %s <lpno> |
		-------------------*/
		print_at (0,0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------------------------
	| Check for sales order analysis. |
	---------------------------------*/
	sptr = chk_env ("PO_VAR_REPT");
	if (sptr == (char *)0)
		envVarPoVarRept = 0;
	else
		envVarPoVarRept = atoi (sptr);

	sprintf (envVarPoActCost, "%1.1s", get_env ("PO_ACT_COST"));
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "po_qkcstupd"))
		sprintf (envVarPoActCost, "N");

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	init_scr ();

	printerNumber = atoi (argv [1]);

	/*----------------------- 
	| Open database files . |
	-----------------------*/
	OpenDB ();

	/*--------------------------
	| Start audit pipe output. |
	--------------------------*/
	OpenAudit ();

	dsp_screen ("Updating Costings to Stock",comm_rec.co_no,comm_rec.co_name);

	Process ();

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
	CloseAudit ();
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
	MoveOpen	=	TRUE;

	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (posd, posd_list, POSD_NO_FIELDS, "posd_id_no");
	open_rec (pogh, pogh_list, POGH_NO_FIELDS, "pogh_id_no");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");

	OpenInsf ();
	OpenIncf ();
	abc_selfield (insf, "insf_hhbr_id");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (poln);
	abc_fclose (pohr);
	abc_fclose (ccmr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inum);
	abc_fclose ("move");
	CloseCosting ();
	abc_dbclose (data);
}

void
Process (
 void)
{
	strcpy (pogh_rec.co_no,comm_rec.co_no);
	pogh_rec.hhsu_hash = 0L;
	strcpy (pogh_rec.gr_no,fifteenSpaces);
	cc = find_rec (pogh, &pogh_rec, GTEQ, "u");

	while (!cc && !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		/*-------------------
		| Goods Received	|
		-------------------*/
		if (HR_COSTED)
		{
			strcpy (posh_rec.co_no,comm_rec.co_no);
			posh_rec.hhsh_hash = pogh_rec.hhsh_hash;
			cc = find_rec (posh,&posh_rec,COMPARISON,"u");
			if (!cc)
			{
				strcpy (posh_rec.status,"D");
				cc = abc_update (posh,&posh_rec);
				if (cc)
					file_err (cc, "posh", "DBUPDATE");
			}
			else
				abc_unlock (posh);

			ProcessPogl (pogh_rec.hhgr_hash);
			strcpy (pogh_rec.pur_status,(ACT_COST) ? "A" : "D");
			strcpy (pogh_rec.gl_status, (ACT_COST) ? "A" : "D");
			cc = abc_update (pogh,&pogh_rec);
			if (cc)
				file_err (cc, "pogh", "DBUPDATE");
		}

		abc_unlock (pogh);
		cc = find_rec (pogh, &pogh_rec, NEXT, "u");
	}
	abc_unlock (pogh);
}

/*===================================
| Process Goods receipt Line items. |
===================================*/
void
ProcessPogl (
	long	hhglHash)
{
	int		found = 0;
	char	lastGrin [16];

	strcpy (lastGrin,fifteenSpaces);
	pogl_rec.hhgr_hash = hhglHash;
	pogl_rec.line_no = 0;

	cc = find_rec (pogl,&pogl_rec,GTEQ,"u");
	while (!cc && pogl_rec.hhgr_hash == hhglHash)
	{
		if (DT_COSTED)
		{
			inmr_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
			cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
			if (strcmp (inmr_rec.supercession,"                "))
			{
				abc_selfield (inmr,"inmr_id_no");
				IntFindSupercession (inmr_rec.supercession);
				abc_selfield (inmr,"inmr_hhbr_hash");
			}
			if (cc)
				continue;

			/*------------------
			| Get part number. |
			------------------*/
			inum_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
			StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

			/*---------------------------------------
			| Process stock file with Actual Costs. |
			---------------------------------------*/
			ProcessStock (inmr_rec.hhbr_hash,pogl_rec.hhcc_hash);

			/*----------------------------------------------
			| Update Quantity left from Details processed. |
			----------------------------------------------*/
			pogl_rec.qty_left = (float)
                    ((pogl_rec.qty_rec > qtySold) ? pogl_rec.qty_rec - qtySold : 0.00);

			strcpy (pogl_rec.pur_status,(ACT_COST) ? "A" : "D");
			strcpy (pogl_rec.gl_status, (ACT_COST) ? "A" : "D");
			cc = abc_update (pogl,&pogl_rec);
			if (cc)
				file_err (cc, "pogl", "DBUPDATE");

			found = 1;
			strcpy (lastGrin, pogh_rec.gr_no);
		}
		else
			abc_unlock (pogl);

		cc = find_rec (pogl,&pogl_rec,NEXT,"u");
	}
	if (found)
		PrintTotal (lastGrin);

	abc_unlock (pogl);
}

/*================
| Process Stock. |
================*/
void
ProcessStock (
	long	hhbrHash, 
	long	hhccHash)
{
	float	remainStock 	= 0.00,
			PurCnvFct 		= 0.00,
			CnvFct			= 0.00,
			closing 		= 0.00;

	double	varianceEach		= 0.00,
			varianceSkExtend 	= 0.00,
			varianceGlExtend	= 0.00,
			calculateOther 		= 0.00;

	missingFifo 	= FALSE;
	qtySold 		= 0.00;
	oldCost 		= 0.00;
	newCost 		= 0.00;

	oldCost = pogl_rec.land_cst;
	newCost = pogl_rec.act_cst;

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, incc, "DBFIND");

	ccmr_rec.hhcc_hash	=	hhccHash;
	cc = find_rec (ccmr,&ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	
	cc = FindInei (hhbrHash, ccmr_rec.est_no, "u");
	if (cc)
		file_err (cc, inei, "DBFIND");

	poln_rec.hhpl_hash	=	pogl_rec.hhpl_hash;
	cc = find_rec (poln,&poln_rec,COMPARISON,"u");
	if (!cc)
	{
		if ((poln_rec.qty_ord - poln_rec.qty_rec) > 0.00)
			strcpy (poln_rec.pur_status,"c");
		else
			strcpy (poln_rec.pur_status,"D");

		cc = abc_update (poln,&poln_rec);
		if (cc)
			file_err (cc, "poln", "DBUPDATE");
	}

	if (poln_rec.hhpo_hash != hhpoHash)
	{
		pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
		cc = find_rec (pohr, &pohr_rec, COMPARISON, "u");
		if (!cc)
		{
			strcpy (pohr_rec.status,"c");
			cc = abc_update (pohr,&pohr_rec);
			if (cc)
				file_err (cc, "pohr", "DBUPDATE");

			hhpoHash = poln_rec.hhpo_hash;
		}
	}
	dsp_process (" Item No. : ",inmr_rec.item_no);
	
	/*-------------------------------------
	| No stock receipted so don't update. |
	-------------------------------------*/
	if (pogl_rec.qty_rec == 0.00) 
	{
		abc_unlock (inei);
		return;
	}

	/*---------------------------------------------------------------------
	| System is set to use actual costing to update relevant information. |
	---------------------------------------------------------------------*/
	if (ACT_COST)
	{
		double	NewCost		=	pogl_rec.act_cst;

		ineiRec.last_cost 	= NewCost;
		ineiRec.date_lcost 	= (lsystemDate > comm_rec.inv_date) 
										? lsystemDate : comm_rec.inv_date;
		cc = abc_update (inei,&ineiRec);
		if (cc) 
			file_err (cc, inei, "DBUPDATE");
	}
	else
		abc_unlock (inei);

	closing = (float) ((incc_rec.closing_stock < 0.00) ? 0.00 : incc_rec.closing_stock);

	if (closing < pogl_rec.qty_rec)
		qtySold = pogl_rec.qty_rec - closing;

	strcpy (incfRec.gr_number,fifteenSpaces);

	switch (inmr_rec.costing_flag [0])
	{
	case 'F':
	case 'I':
		if (inmr_rec.costing_flag [0] == 'F')
			remainStock = 	StockLeft
						 	(
								incc_rec.hhwh_hash,
					      		incc_rec.closing_stock,
					      		TRUE, pogh_rec.gr_no
							);
		else
			remainStock = 	StockLeft
						 	(
								incc_rec.hhwh_hash,
					      		incc_rec.closing_stock,
					      		FALSE, pogh_rec.gr_no
							);
	
		remainStock = (float) (twodec (remainStock));
				
		if (strcmp (incfRec.gr_number,pogh_rec.gr_no))
			missingFifo = TRUE;
		
		qtySold = pogl_rec.qty_rec - remainStock;
		if (qtySold < 0.00)
			qtySold = 0.00;
	
		qtySold 	= (qtySold > pogl_rec.qty_rec) ? pogl_rec.qty_rec : qtySold;
		qtySold 	= (float) (twodec (qtySold));
		remainStock 	= pogl_rec.qty_rec - qtySold;

		if (missingFifo)
		{
			printf("FIFO RECORD MISSING/NOT FOUND");fflush(stdout);
			sleep (60);
			abc_unlock (incf);
			break;
		}

		if (ACT_COST)
		{
			oldCost 			= incfRec.fifo_cost;
			incfRec.fifo_cost 	= newCost;
			incfRec.act_cost 	= oldCost;
		}
		else
		{
			oldCost = incfRec.fifo_cost;
			incfRec.act_cost = newCost;
		}

		strcpy (incfRec.stat_flag,"A");
		cc = abc_update (incf,&incfRec);
		if (cc)
			file_err (cc, incf, "DBUPDATE");

		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			inmr_rec.std_uom,
			incfRec.fifo_date,
			3,
			"REVERSE",
			inmr_rec.inmr_class,
			inmr_rec.category,
			pogl_rec.po_number,
			pogh_rec.gr_no,
			remainStock,
			0.00,
			CENTS (oldCost)
		);
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			inmr_rec.std_uom,
			incfRec.fifo_date,
			5,
			"REVALUE",
			inmr_rec.inmr_class,
			inmr_rec.category,
			pogl_rec.po_number,
			pogh_rec.gr_no,
			remainStock,
			0.00,
			CENTS (newCost)
		);
		break;

	case 'S':
		insfRec.hhbr_hash = hhbrHash;
		strcpy (insfRec.status, "F");
		sprintf (insfRec.serial_no,"%-25.25s",pogl_rec.serial_no);
		cc = find_rec (insf,&insfRec,COMPARISON,"u") ;
		if (cc)
		{
			abc_unlock (insf);
			strcpy (insfRec.status, "S");
			cc = find_rec (insf,&insfRec,COMPARISON,"u") ;
			if (cc)
			{
				abc_unlock (insf);
				strcpy (insfRec.status, "C");
				cc = find_rec (insf,&insfRec,COMPARISON,"u") ;
				if (cc)
				{
					abc_unlock (insf);
					strcpy (insfRec.status, "T");
					cc = find_rec (insf,&insfRec,COMPARISON,"u") ;
				}
			}
		}
		if (cc)
			return;

		oldCost = insfRec.est_cost;

		if (insfRec.status [0] == 'S')
			qtySold = 1;
		else
			remainStock = 1;

		if (ACT_COST)
		{
			insfRec.exch_rate   = pogh_rec.exch_rate;
			insfRec.fob_fgn_cst = pogl_rec.fob_fgn_cst;
			insfRec.fob_nor_cst = pogl_rec.fob_nor_cst;
			insfRec.frt_ins_cst = pogl_rec.frt_ins_cst;
			insfRec.duty        = pogl_rec.duty;
			insfRec.licence     = pogl_rec.licence;
			insfRec.lcost_load  = pogl_rec.lcost_load;

			
			calculateOther = pogl_rec.fob_nor_cst +
							 pogl_rec.duty +
							 pogl_rec.licence;

			insfRec.other_cst   = 	newCost -
									calculateOther;

			insfRec.istore_cost = newCost;
			insfRec.land_cst    = newCost;
			insfRec.act_cost    = newCost;

			strcpy (insfRec.po_number,pogl_rec.po_number);
			strcpy (insfRec.gr_number,pogh_rec.gr_no);
			strcpy (insfRec.stat_flag,"A");

			/*----------------------------
			| Find Line one (FOB) GOODS. |
			----------------------------*/
			strcpy (posd_rec.co_no, comm_rec.co_no);
			posd_rec.hhsh_hash = posh_rec.hhsh_hash;
			posd_rec.hhpo_hash = poln_rec.hhpo_hash;
			cc = find_rec (posd, &posd_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (insfRec.crd_invoice, fifteenSpaces);
				insfRec.hhsu_hash = 0L;
			}
			else
			{
				strcpy (insfRec.crd_invoice, posd_rec.inv_no);
				insfRec.hhsu_hash = pogh_rec.hhsu_hash;
			}
			if (insfRec.final_costing [0] != 'Y')
				strcpy (insfRec.final_costing, "N");

			cc = abc_update (insf,&insfRec);
			if (cc) 
				file_err (cc, "insf", "DBUPDATE");
		}
		else
			abc_unlock (insf);
		break;

	default:
		break;
	}

	varianceEach = oldCost - newCost;

	outNewCost = out_cost (newCost, inmr_rec.outer_size);
	outOldCost = out_cost (oldCost, inmr_rec.outer_size);

	varianceGlExtend = (outOldCost * qtySold) - (outNewCost * qtySold);
	varianceSkExtend = (outOldCost * remainStock) - (outNewCost * remainStock);

	if (summaryReport)
	{	
		fprintf (fout,".PA\n");
		summaryReport = FALSE;
	}
	inum_rec.hhum_hash	=	pogl_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	fprintf (fout, "!%15.15s", 	pogh_rec.gr_no);
	fprintf (fout, "!%15.15s", 	pogl_rec.po_number);
	fprintf (fout, "!%16.16s", 	inmr_rec.item_no);
	fprintf (fout, "!%s",	 	inum_rec.uom);
	fprintf (fout, "!%7.0f", 	pogl_rec.qty_rec * CnvFct);
	fprintf (fout, "!%11.2f", 	oldCost);
	fprintf (fout, "!%11.2f", 	newCost);
	fprintf (fout, "!%12.2f", 	outOldCost * pogl_rec.qty_rec);
	fprintf (fout, "!%12.2f", 	outNewCost * pogl_rec.qty_rec);
	fprintf (fout, "!%10.2f", 	varianceEach);
	fprintf (fout, "!%7.0f", 	qtySold	* CnvFct);
	fprintf (fout, "!%11.2f",  	varianceSkExtend);
	fprintf (fout, "!%11.2f!%s\n",varianceGlExtend, (missingFifo) ? "*" : " ");
	
	if (strcmp (pogl_rec.serial_no, serialSpace))
	{
		fprintf (fout,"!         ");
		fprintf (fout,"! SERIAL NO : %-25.25s    ", pogl_rec.serial_no);
		fprintf (fout,"!           ");
		fprintf (fout,"!           ");
		fprintf (fout,"!            ");
		fprintf (fout,"!            ");
		fprintf (fout,"!          ");
		fprintf (fout,"!       ");
		fprintf (fout,"!           ");
		fprintf (fout,"!           !\n");
		
	}

	grTotalActual 	+= outNewCost * pogl_rec.qty_rec;
	grTotalEstimate += outOldCost * pogl_rec.qty_rec;
	grTotalGlVar 	+= varianceGlExtend;
	grTotalSkVar 	+= varianceSkExtend;

	totalActual 	+= outNewCost * pogl_rec.qty_rec;
	totalEstimate 	+= outOldCost * pogl_rec.qty_rec;
	totalGlVar 		+= varianceGlExtend;
	totalSkVar 		+= varianceSkExtend;

	/*--------------------------------------
	| Process Audit with information Sold. |
	--------------------------------------*/
	if (envVarPoVarRept)
		ProcessAudit ();

	return;
}

/*========================================
| Print total for goods receipts number. |
========================================*/
void
PrintTotal (
 char *gr_no)
{
	PrintLine ();

	fprintf (fout,"! TOTAL ACTUAL VALUE FOR RECEIPTED GR# %15.15s", gr_no);
	fprintf (fout,"!       ");
	fprintf (fout,"!           ");
	fprintf (fout,"!           ");
	fprintf (fout,"!%12.2f ", grTotalEstimate);
	fprintf (fout,"!%12.2f ", grTotalActual);
	fprintf (fout,"!          ");
	fprintf (fout,"!       ");
	fprintf (fout,"!%11.2f",grTotalSkVar);
	fprintf (fout,"!%11.2f!\n",grTotalGlVar);

	PrintLine ();

	PrintGrinSummary ();
	fflush (fout);
	
	grTotalActual 	= 0.00;
	grTotalEstimate = 0.00;
	grTotalGlVar 	= 0.00;
	grTotalSkVar 	= 0.00;
	return;
}
/*========================================
| Print total for goods receipts number. |
========================================*/
void
PrintGrand (
 void)
{
	PrintLine ();
	fprintf (fout,"!               ");
	fprintf (fout,"!               ");
	fprintf (fout,"! TOTAL RECEIPTED");
	fprintf (fout,"!    ");
	fprintf (fout,"!       ");
	fprintf (fout,"!           ");
	fprintf (fout,"!           ");
	fprintf (fout,"!%12.2f", totalEstimate);
	fprintf (fout,"!%12.2f", totalActual);
	fprintf (fout,"!          ");
	fprintf (fout,"!       ");
	fprintf (fout,"!%11.2f", totalSkVar);
	fprintf (fout,"!%11.2f!\n", totalGlVar);

	fflush (fout);
}

void
IntFindSupercession (
 char *item_no)
{
	if (!strcmp (item_no,"                "))
		return;

	abc_selfield (inmr,"inmr_id_no");
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s",item_no);
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (!cc)
		IntFindSupercession (inmr_rec.supercession);
}

/*-----------------------------
| Create audit records        |
-----------------------------*/
void
ProcessAudit (
 void)
{
	double	hi_cost,
		low_cost;

	if (!envVarPoVarRept)
		return;

	open_rec (poca,poca_list,POCA_NO_FIELDS,"poca_id_no");

	if (pogl_rec.act_cst != poln_rec.land_cst)
	{
		strcpy (poca_rec.co_no,comm_rec.co_no);
		strcpy (poca_rec.br_no,ccmr_rec.est_no);
		strcpy (poca_rec.type,"E");
		strcpy (poca_rec.item_cat,inmr_rec.category);
		strcpy (poca_rec.item_no,inmr_rec.item_no);
		poca_rec.line_no = pogl_rec.line_no;
		strcpy (poca_rec.item_desc,inmr_rec.description);
		strcpy (poca_rec.po_no,pogl_rec.po_number);
		strcpy (poca_rec.gr_no,pogh_rec.gr_no);
		poca_rec.est_cst = poln_rec.land_cst;
		poca_rec.act_cst = pogl_rec.act_cst;
		poca_rec.date_print = pogl_rec.rec_date;
		strcpy (poca_rec.status,"C");
		cc = abc_add (poca,&poca_rec);
		if (cc)
			file_err (cc, poca, "DBADD");
	}

	strcpy (poca_rec.co_no,comm_rec.co_no);
	strcpy (poca_rec.br_no,ccmr_rec.est_no);
	strcpy (poca_rec.type,"V");
	strcpy (poca_rec.item_cat,inmr_rec.category);
	strcpy (poca_rec.item_no,inmr_rec.item_no);
	strcpy (poca_rec.gr_no,pogh_rec.gr_no);
	poca_rec.line_no = pogl_rec.line_no;
	if (find_rec (poca,&poca_rec,COMPARISON,"u"))
	{
		abc_fclose (poca);
		return;
	}

	hi_cost  = poca_rec.est_cst + (poca_rec.est_cst * .1);
	low_cost = poca_rec.est_cst - (poca_rec.est_cst * .1);
	if (pogl_rec.act_cst > hi_cost || pogl_rec.act_cst < low_cost)
	{
	    	poca_rec.act_cst = pogl_rec.act_cst;
	    	strcpy (poca_rec.status,"C");
	    	cc = abc_update (poca,&poca_rec);
	    	if (cc)
			file_err (cc, poca, "DBUPDATE");
	}
	else
	{
	    	cc = abc_delete (poca);
	    	if (cc)
			file_err (cc, poca, "DBDELETE");
	}
	if (qtySold != 0)
	{

		strcpy (poca_rec.co_no,comm_rec.co_no);
		strcpy (poca_rec.br_no,ccmr_rec.est_no);
		strcpy (poca_rec.type,"X");
		strcpy (poca_rec.item_cat,inmr_rec.category);
		strcpy (poca_rec.item_no,inmr_rec.item_no);
		poca_rec.line_no = pogl_rec.line_no;
		strcpy (poca_rec.item_desc,inmr_rec.description);
		strcpy (poca_rec.po_no,pogl_rec.po_number);
		strcpy (poca_rec.gr_no,pogh_rec.gr_no);
		poca_rec.act_cst = outNewCost * qtySold;
		poca_rec.est_cst = outOldCost * qtySold;
		poca_rec.date_print = pogl_rec.rec_date;
		strcpy (poca_rec.status,"C");
		cc = abc_add (poca,&poca_rec);
		if (cc)
		file_err (cc, poca, "DBADD");
	}
	abc_fclose (poca);
	return;
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL) 
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".14\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".E%s\n",     clip (comm_rec.co_name));
	fprintf (fout,".E%s/ %s\n", clip (comm_rec.est_name),
				   clip (comm_rec.cc_name));
	fprintf (fout,".B1\n");
	if (ACT_COST)
	{
		fprintf (fout,".ERECEIPT COSTING UPDATE AUDIT REPORT.\n");
		fprintf (fout,".CNOTE : Your Stock system is based on Actual Cost and stock has been update to reflect this Audit.\n");
		fprintf (fout,".C       Please also note that the General Ledger is updated from the figures reported in the Summary Report.\n");
	}
	else
	{
		fprintf (fout,".ERECEIPT COSTING AUDIT REPORT.\n");
		fprintf (fout,".CNOTE : Your Stock system is based on Estimated Cost and stock has not been updated with Actual Costs.\n");
		fprintf (fout,".C       Please also note that the General Ledger is updated from the figures reported in the Summary Report.\n");
	}

	fprintf (fout,".Eas at %-24.24s\n",SystemTime ());

	fprintf (fout,".R================");
	fprintf (fout,"================");
	fprintf (fout,"=================");
	fprintf (fout,"=====");
	fprintf (fout,"========");
	fprintf (fout,"============");
	fprintf (fout,"============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"===========");
	fprintf (fout,"========");
	fprintf (fout,"============");
	fprintf (fout,"=============\n");

	fprintf (fout,"================");
	fprintf (fout,"================");
	fprintf (fout,"=================");
	fprintf (fout,"=====");
	fprintf (fout,"========");
	fprintf (fout,"============");
	fprintf (fout,"============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"===========");
	fprintf (fout,"========");
	fprintf (fout,"============");
	fprintf (fout,"=============\n");

	fprintf (fout,"! GOODS RECEIPT ");
	fprintf (fout,"! PURCHASE ORDER");
	fprintf (fout,"!   ITEM NUMBER  ");
	fprintf (fout,"!UOM ");
	fprintf (fout,"!  QTY  ");
	fprintf (fout,"! ESTIMATED ");
	fprintf (fout,"!ACTUAL UNIT");
	fprintf (fout,"! TOTAL COST ");
	fprintf (fout,"! TOTAL COST ");
	fprintf (fout,"! UNIT COST");
	fprintf (fout,"!  QTY  ");
	fprintf (fout,"!  VARIANCE ");
	fprintf (fout,"!  VARIANCE !\n");

	fprintf (fout,"!    NUMBER     ");
	fprintf (fout,"!    NUMBER     ");
	fprintf (fout,"!                ");
	fprintf (fout,"!    ");
	fprintf (fout,"!       ");
	fprintf (fout,"! UNIT COST ");
	fprintf (fout,"!    COST   ");
	fprintf (fout,"!   ESTIMATE ");
	fprintf (fout,"!    ACTUAL  ");
	fprintf (fout,"!  VARIANCE");
	fprintf (fout,"!  SOLD ");
	fprintf (fout,"!  STOCK    ");
	fprintf (fout,"!    G/L    !\n");
	
	PrintLine ();
	fflush (fout);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	PrintGrand ();
	fprintf (fout,".EOF\n");
	pclose (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout,"!---------------");
	fprintf (fout,"!---------------");
	fprintf (fout,"!----------------");
	fprintf (fout,"!----");
	fprintf (fout,"!-------");
	fprintf (fout,"!-----------");
	fprintf (fout,"!-----------");
	fprintf (fout,"!------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!----------");
	fprintf (fout,"!-------");
	fprintf (fout,"!-----------");
	fprintf (fout,"!-----------!\n");
	fflush (fout);
}

/*===============================================
| Reduce incf_fifo_qty's for incf records		|
| to allow stock transfers between warehouses	|
| to work correctly.							|
===============================================*/
float	
StockLeft (
	long	hhwhHash,
	float	closingStock,
	int		fifoFlag,
	char	*grinNumber)
{
	float	qty = closingStock;
	
	if (qty < 0.00)
		qty = 0.00;

	cc = FindIncf (hhwhHash, !fifoFlag,"u");
	while (!cc && hhwhHash == incfRec.hhwh_hash)
	{
		/*------------------------------------------------------
		| Check for Same GR# and same receipt Quantity as same |
		| product may be receipted on same GR#.                |
		------------------------------------------------------*/
		if (!strncmp (incfRec.gr_number,grinNumber,15))
			return (qty);

		/*-----------------------
		| need to reduce ff_qty	|
		-----------------------*/
		if (qty < incfRec.fifo_qty)
			qty = 0.00;
		else
			qty  -= incfRec.fifo_qty;

		abc_unlock (incf);
		cc = FindIncf (0L,!fifoFlag,"u");
	}
	abc_unlock (incf);
	/*------------------------------------
	| Bad fifo as exact match not found. |
	------------------------------------*/
	strcmp (incfRec.gr_number,fifteenSpaces);
	incfRec.fifo_qty = 0.00;

	return (0.00);
}

void
PrintGrinSummary (
 void)
{
	double	tot_fgn = 0.00;
	double	tot_loc = 0.00;
	double	tot_gst = 0.00;

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash");
	open_rec (pogd,pogd_list,POGD_NO_FIELDS,"pogd_id_no");
	
	fprintf (fout,".LRP17\n");
	fprintf (fout,"!%-47.47sG O O D S  R E C E I P T   A C T U A L  C O S T  S U M M A R Y%45.45s!\n"," "," ");
	fprintf (fout,"!                  ");
	fprintf (fout,"!======================");
	fprintf (fout,"!============");
	fprintf (fout,"!========");
	fprintf (fout,"!=================");
	fprintf (fout,"!=====");
	fprintf (fout,"!=================");
	fprintf (fout,"!============");
	fprintf (fout,"!=================");
	fprintf (fout,"!                 !\n");

	fprintf (fout,"!                  ");
	fprintf (fout,"!     DESCRIPTION.     ");
	fprintf (fout,"!   SPREAD   ");
	fprintf (fout,"!SUPPLIER");
	fprintf (fout,"!    INVOICE NO   ");
	fprintf (fout,"!CURR.");
	fprintf (fout,"! FOREIGN VALUE.  ");
	fprintf (fout,"! EXCH RATE. ");
	fprintf (fout,"!  LOCAL  VALUE.  ");
	fprintf (fout,"!                 !\n");

	fprintf (fout,"!                  ");
	fprintf (fout,"!----------------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!--------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!-----");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!                 !\n");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhgr_hash = pogh_rec.hhgr_hash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd, &pogd_rec, GTEQ, "u");
	while (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) && 
                      pogd_rec.hhgr_hash == pogh_rec.hhgr_hash)
	{
		fprintf (fout,"!                  ");
		fprintf (fout,"! %-20.20s !",pogd_rec.category);
		if (pogd_rec.allocation [0] == 'D')
			fprintf (fout,"  Value.    !");

		if (pogd_rec.allocation [0] == 'W')
			fprintf (fout,"  Weight.   !");

		if (pogd_rec.allocation [0] == 'V')
			fprintf (fout,"  Volume.   !");

		if (pogd_rec.allocation [0] == ' ')
			fprintf (fout,"  None.     !");

		if (pogd_rec.hhsu_hash == 0L)
			fprintf (fout,"        !");
		else
		{
			sumr_rec.hhsu_hash	=	pogd_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
			if (cc)
				fprintf (fout,"Unknown.!");
			else
				fprintf (fout," %6.6s !", sumr_rec.crd_no);
		}
		fprintf (fout," %-15.15s !",pogd_rec.invoice);
		fprintf (fout," %-3.3s !",  pogd_rec.currency);
		fprintf (fout," %15.2f !",  pogd_rec.foreign);
		fprintf (fout," %10.4f !",  pogd_rec.exch_rate);
		fprintf (fout," %15.2f !",  pogd_rec.nz_value);
		fprintf (fout,"                 !\n");

		tot_fgn += pogd_rec.foreign;
		tot_loc += pogd_rec.nz_value;
		tot_gst += pogd_rec.nz_gst;

		strcpy (pogd_rec.cost_edit, "D");
		cc = abc_update (pogd, &pogd_rec);
		if (cc)
			file_err (cc, "pogd", "DBUPDATE");

		cc = find_rec (pogd, &pogd_rec, NEXT, "u");
	}
	abc_unlock (pogd);
	fprintf (fout,"!                  ");
	fprintf (fout,"! TOTALS               ");
	fprintf (fout,"!            ");
	fprintf (fout,"!        ");
	fprintf (fout,"!                 ");
	fprintf (fout,"!     ");
	fprintf (fout,"! %15.2f ",tot_fgn);
	fprintf (fout,"!            ");
	fprintf (fout,"! %15.2f ",tot_loc);
	fprintf (fout,"!                 !\n");

	fprintf (fout,"!                  ");
	fprintf (fout,"!----------------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!--------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!-----");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!                 !\n");
	fprintf (fout,"!%-154.154s!\n"," ");
	summaryReport = TRUE;
	abc_fclose (sumr);
	abc_fclose (pogd);
}
