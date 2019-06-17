/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_trgen.c,v 5.2 2001/08/09 09:30:03 scott Exp $
|  Program Name  : ( lrp_trgen.c     )                                |
|  Program Desc  : ( Generate Transfer Orders.                    )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: lrp_trgen.c,v $
| Revision 5.2  2001/08/09 09:30:03  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:54  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:44  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:55  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/01/30 04:16:10  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:15:42  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:51  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.10  2000/02/18 01:42:17  scott
| Updated for small warning error when compile under Linux
|
| Revision 1.9  1999/12/06 01:34:22  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.8  1999/10/27 07:33:04  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.7  1999/10/04 06:48:00  scott
| Updated for comm error.
|
| Revision 1.6  1999/09/29 10:10:53  scott
| Updated to be consistant on function names.
|
| Revision 1.5  1999/09/17 07:26:46  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.4  1999/09/16 09:20:48  scott
| Updated from Ansi Project
|
| Revision 1.3  1999/06/15 07:27:06  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_trgen.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_trgen/lrp_trgen.c,v 5.2 2001/08/09 09:30:03 scott Exp $";

#include 	<pslscr.h>	
#include 	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_lrp_mess.h>
#include    <proc_sobg.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct comrRecord	comr_rec;
struct ffwkRecord	ffwk_rec;
struct ithrRecord	ithr_rec;
struct ithrRecord	ithr2_rec;
struct itlnRecord	itln_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;

	float	*ffwk_cons	=	&ffwk_rec.cons_1;

	int		printerNumber 		=	1,
			envVarSoFwdRel		=	0,
			envVarSkTrMaxLines	=	0;

	char	filename [15];

	long	hhitHash,
			hhccHash,
			TransferNumber;

	FILE	*fin,
			*fout;

	char	*currentUser;

	char	 *ithr2	=	"ithr2";

	long    lsystemDate	= 0L,
			RhhccHash 	= 0L;

#include <RealCommit.h>

/*=============================
| Local Function Prototypes.  |
=============================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	HeadingOutput 	(void);
void 	Process 		(void);
void 	AddIthr 		(void);
void 	FindTrNo 		(void);
void 	LogError 		(int);
int 	AddItln 		(int);
int 	IntFindInmr 	(long);
int 	CheckIthr 		(long);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int    argc,
	char*  argv [])
{
	char	*sptr;
	currentUser = getenv ("LOGNAME");

	if (argc < 2)
	{
		print_at (0,0, "Usage %s <LPNO>", argv [0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi(argv [1]);

	lsystemDate = TodaysDate ();

	sptr = chk_env ("SO_FWD_REL");
	envVarSoFwdRel = (sptr == (char *) 0) ? FALSE : atoi (sptr);

	/*---------------------------------------------------
	| Check limit on maximum lines per transfer header.	|
	---------------------------------------------------*/
	sptr = chk_env ("SK_TR_MAX_LINES");
	envVarSkTrMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	init_scr ();

	OpenDB ();

	dsp_screen ("Processing : Generating Transfer Orders.", comm_rec.co_no, comm_rec.co_name);

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)",errno,PNAME);
	
	HeadingOutput ();

	Process ();

	fprintf (fout,".EOF\n");
	pclose (fout);

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
	rset_tty ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (ithr2, ithr);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ffwk, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no_2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	open_rec (ithr2,ithr_list, ITHR_NO_FIELDS, "ithr_id_no4");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");

	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,	comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND" );

	hhccHash = ccmr_rec.hhcc_hash;
	
	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (incc);
	abc_fclose (comr);
	abc_fclose (ccmr);
	abc_fclose (ffwk);
	abc_fclose (inmr);
	abc_fclose (ithr);
	abc_fclose (ithr2);
	abc_fclose (itln);
	abc_fclose (soic);
	abc_dbclose ("data");
}

void
HeadingOutput (void)
{
	fprintf (fout, ".START%s\n",DateToString(comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");

	fprintf (fout, ".15\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L150\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ELRP - TRANSFER ORDER GENERATION REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EFROM : %s\n", comm_rec.co_name);
	fprintf (fout, ".E     : %s\n", comm_rec.est_name);
	fprintf (fout, ".E     : %s\n", comm_rec.cc_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime());

	fprintf (fout,".R=====================");
	fprintf (fout,"===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	fprintf (fout,"================\n");

	fprintf (fout,"=====================");
	fprintf (fout,"===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	fprintf (fout,"================\n");

	fprintf (fout,"| Br /Wh - Warehouse ");
	fprintf (fout,"|   PART NUMBER.   ");
	fprintf (fout,"|            PART DESCRIPTION              ");
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"| STD ");
	fprintf (fout,"|  UNIT COST   ");
	fprintf (fout,"|   EXTENDED   |\n");

	fprintf (fout,"| No /No - Acronym   ");
	fprintf (fout,"|                  ");
	fprintf (fout,"|                                          ");
	fprintf (fout,"|   ORDERED  ");
	fprintf (fout,"| BACKORDER  ");
	fprintf (fout,"| UOM ");
	fprintf (fout,"|              ");
	fprintf (fout,"|      COST    |\n");

	fprintf (fout,"|--------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|--------------");
	fprintf (fout,"|--------------|\n");

	fflush(fout);
}

void
Process (void)
{
	int		line_no = 0;

	RhhccHash	= 	0L;

	ffwk_rec.hhcc_hash 	= hhccHash;
	sprintf (filename,"TRN FROM %s/%s", comm_rec.est_no,comm_rec.cc_no);
	strcpy (ffwk_rec.filename, filename);
	strcpy (ffwk_rec.crd_no,"      ");
	sprintf (ffwk_rec.sort,"%-34.34s", " ");
	
	cc = find_rec (ffwk, &ffwk_rec, GTEQ, "u");
	while (!cc && 
	       ffwk_rec.hhcc_hash == hhccHash && 
	       !strcmp (ffwk_rec.filename, filename))
	{
		/*---------------------------------------
		| If Purchase Order Already Generated	|
		---------------------------------------*/
		if (ffwk_rec.hhit_hash != 0L || ffwk_rec.stat_flag [0] != 'U')
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
		if (ffwk_rec.order_qty <= 0.00)
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
		if (strncmp (ffwk_rec.filename, "TRN", 3))
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
		/*-------------------------------------------
		| New supplier or Purchase Order too long	|
		-------------------------------------------*/
		if (RhhccHash != ffwk_rec.r_hhcc_hash || 
			 (envVarSkTrMaxLines && line_no >= envVarSkTrMaxLines))
		{
			ccmr2_rec.hhcc_hash	=	ffwk_rec.r_hhcc_hash; 
			cc = find_rec (ccmr, &ccmr2_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "ccmr", "DBFIND");

			RhhccHash = ffwk_rec.r_hhcc_hash;

			/*-----------------------------------
			| Find next purchase order number	|
			-----------------------------------*/
			FindTrNo ();
			line_no = 0;
			AddIthr();
		}

		if (AddItln (line_no++))
		{
			line_no--;
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
		recalc_sobg();

		ffwk_rec.hhit_hash = hhitHash;
		cc = abc_update (ffwk,&ffwk_rec);
		if (cc)
			file_err(cc, "ffwk", "DBUPDATE" );

		cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
	}
	abc_unlock (ffwk);
}

/*===================
| Find Item record. |
===================*/
int
IntFindInmr (
 long   hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	return (find_rec (inmr, &inmr_rec, COMPARISON, "r"));
}

void
AddIthr (void)
{
	static	int		init_done;

	strcpy (ithr_rec.co_no,		comm_rec.co_no);
	strcpy (ithr_rec.type,		"M");
	ithr_rec.del_no		=		TransferNumber;
	ithr_rec.hhit_hash	=		0L;
	ithr_rec.iss_sdate	=		lsystemDate;
	ithr_rec.iss_date	=		comm_rec.inv_date;
	ithr_rec.rec_date	=		0L;
	sprintf (ithr_rec.tran_ref, "TRAN TO %s/%s/%s",
									ccmr2_rec.co_no,
									ccmr2_rec.est_no,
									ccmr2_rec.cc_no);
	sprintf (ithr_rec.op_id, "%-14.14s", currentUser);

	strcpy (ithr_rec.time_create, TimeHHMM());
	ithr_rec.date_create	=	TodaysDate ();
	strcpy (ithr_rec.cons_no,	" ");
	strcpy (ithr_rec.carr_code,	" ");
	strcpy (ithr_rec.carr_area,	" ");
	ithr_rec.no_cartons		=	0;
	ithr_rec.frt_cost		=	0;
	ithr_rec.no_kgs			=	0;
	strcpy (ithr_rec.full_supply,	"N");
	strcpy (ithr_rec.printed,		"N");
	strcpy (ithr_rec.stat_flag,		"0");

	sprintf (err_str, "%010ld", TransferNumber);
	dsp_process ("Transfer", err_str);

	cc = abc_add (ithr,&ithr_rec);
	if (cc)
		file_err (cc, "ithr", "DBADD" );

	cc = find_rec (ithr,&ithr_rec,EQUAL,"u");
	if (cc)
		file_err (cc, "ithr", "DBFIND" );

	hhitHash = ithr_rec.hhit_hash;

	abc_unlock (ithr);

	/*-------------------------------
	| Head Audit Page wih Supplier	|
	-------------------------------*/
	fprintf (fout, ".PD| Transfer Number %010ld / Created by : %-14.14s / On : %-10.10s - %-5.5s %60.60s|\n",
		ithr_rec.del_no,
		ithr_rec.op_id,
		DateToString (ithr_rec.date_create), 
		ithr_rec.time_create, " "
	);
	if (init_done)
		fprintf (fout,".PA\n");

	init_done = 1;
}

int
AddItln (
 int    line_no)
{
	float	realCommitted;
	float	act_avail;

	if (IntFindInmr (ffwk_rec.hhbr_hash))
	{
		LogError (TRUE);
		return (EXIT_FAILURE);
	}
	incc_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	incc_rec.hhcc_hash	=	ffwk_rec.hhcc_hash;
	cc = find_rec ("incc", &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "incc", "DBFIND");

	/*---------------------------------
	| Calculate Actual Qty Committed. |
	---------------------------------*/
	realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,incc_rec.hhcc_hash);
	if (envVarSoFwdRel)
		act_avail = incc_rec.closing_stock -
					incc_rec.committed -
					realCommitted - 
					incc_rec.backorder -
					incc_rec.forward;
	else
		act_avail = incc_rec.closing_stock -
					realCommitted - 
					incc_rec.committed -
					incc_rec.backorder;
	
	if (ffwk_rec.sugg_qty < act_avail)
	{
		itln_rec.qty_order	=	ffwk_rec.sugg_qty;
		itln_rec.qty_border	=	0.00;
	}
	else
	{
		if (act_avail > 0)
		{
			itln_rec.qty_order	=	act_avail;
			itln_rec.qty_border	=	ffwk_rec.sugg_qty - act_avail;
		}
		else
		{
			itln_rec.qty_order	=	0.00;
			itln_rec.qty_border	=	ffwk_rec.sugg_qty;
		}
	}

	itln_rec.itff_hash			=	0L;
	itln_rec.hhit_hash			=	hhitHash;
	itln_rec.line_no			=	line_no;
	itln_rec.hhbr_hash			=	inmr_rec.hhbr_hash;
	itln_rec.r_hhbr_hash		=	inmr_rec.hhbr_hash;
	itln_rec.i_hhcc_hash		=	hhccHash;
	itln_rec.r_hhcc_hash		=	RhhccHash;
	itln_rec.hhum_hash			=	inmr_rec.std_uom;
	strcpy (itln_rec.tran_ref,		ithr_rec.tran_ref);
	strcpy (itln_rec.serial_no, 	" ");
	itln_rec.qty_rec			=	0.00;
	itln_rec.cost				=	ffwk_rec.cost_price;
	itln_rec.duty				=	ffwk_rec.uplift_amt;
	strcpy (itln_rec.stock,			"S");
	if (itln_rec.qty_order == 0.00)
		strcpy (itln_rec.status, "B");
	else
		strcpy (itln_rec.status, "M");

	itln_rec.due_date			=	lsystemDate;
	strcpy (itln_rec.full_supply,	"N");
	strcpy (itln_rec.item_desc,	inmr_rec.description);
	strcpy (itln_rec.stat_flag, "0");

	cc = abc_add(itln,&itln_rec);
	if (cc)
		file_err(cc, "itln", "DBADD" );

	add_hash
	(
		comm_rec.co_no,
		ccmr_rec.est_no,
		"RC",
		0,
		itln_rec.hhbr_hash,
		itln_rec.i_hhcc_hash,
		0L,
		(double) 0.00
	);
	add_hash
	(
		comm_rec.co_no,
		ccmr2_rec.est_no,
		"RC",
		0,
		itln_rec.r_hhbr_hash,
		itln_rec.r_hhcc_hash,
		0L,
		(double) 0.00
	);
	/*-------------------
	| Write Audit Log	|
	-------------------*/
	fprintf (fout,"| %s /%s - %s ",	ccmr2_rec.est_no,
									ccmr2_rec.cc_no,
									ccmr2_rec.acronym);
	fprintf (fout,"| %s ",			inmr_rec.item_no);
	fprintf (fout,"| %s ",			inmr_rec.description);
	fprintf (fout,"|%11.2f ",		itln_rec.qty_order);
	fprintf (fout,"|%11.2f ",		itln_rec.qty_border);
	fprintf (fout,"| %-3.3s ",		inmr_rec.sale_unit);
	fprintf (fout,"|%13.2f ", 		itln_rec.cost + itln_rec.duty);
	fprintf (fout,"|%13.2f |\n",	(itln_rec.cost + itln_rec.duty) * 
								 	itln_rec.qty_order);
	fflush(fout);
	return (EXIT_SUCCESS);
}

void
FindTrNo (void)
{
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"u");
	if (cc)
		file_err(cc, "comr", "DBFIND" );

	while (CheckIthr(++comr_rec.nx_del_no) == 0);

	TransferNumber = comr_rec.nx_del_no;

	cc = abc_update (comr,&comr_rec);
	if (cc)
		file_err(cc, "comr", "DBUPDATE" );

	abc_unlock (comr);
}

/*=========================================
| Log error as it is better than sys_err. |
=========================================*/
void
LogError (
 int    item)
{
	fprintf (fout,"| *** ERROR IN GENERATION OF STOCK TRANSFERS. FILE NAME : %-14.14s / SORT KEY", ffwk_rec.filename);
	fprintf (fout," : [%-34.34s] NO ITEM", ffwk_rec.sort);
	fprintf (fout,"NUMBER FOUND.  |\n");
}

/*==================================
| Check for purchase order number. |
==================================*/
int
CheckIthr (
	long   TranNo)
{
	strcpy (ithr2_rec.co_no,comm_rec.co_no);
	ithr2_rec.del_no	=	TranNo;
	return (find_rec (ithr2, &ithr2_rec, COMPARISON,"r"));
}
