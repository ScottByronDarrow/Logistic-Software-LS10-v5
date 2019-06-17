/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_bal_fix.c,v 5.4 2001/08/09 09:18:08 scott Exp $
|  Program Name  : (sk_bal_fix.c  )                                   |
|  Program Desc  : (Re-balance and Fix Stock Files.             )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 12/03/90         |
|---------------------------------------------------------------------|
| $Log: sk_bal_fix.c,v $
| Revision 5.4  2001/08/09 09:18:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/06 23:44:40  scott
| RELEASE 5.0
|
| Revision 5.2  2001/07/25 02:18:51  scott
| Update - LS10.5
|
| Revision 5.0  2001/06/19 08:15:06  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/06 04:38:16  cha
| Updated to ensure no cursors is not blown in SQL, program is a hungry little fellow
|
| Revision 4.0  2001/03/09 02:36:33  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.4  2001/02/06 10:07:39  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
| Revision 3.3  2000/12/21 10:16:03  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.2  2000/11/22 00:53:14  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.1  2000/11/20 07:40:06  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:19:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/01 02:17:30  scott
| General Maintenance - added app.schema
|
| Revision 2.0  2000/07/15 09:10:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.34  2000/07/14 05:10:50  scott
| Updated for minor compile warnings on Linux
|
| Revision 1.33  2000/05/24 06:36:45  scott
| Updated to usage prompt, also added balance intr for each warehouse not just one
|
| Revision 1.32  2000/05/24 03:26:17  scott
| Updated for cosmetic change in useage, replaced C (heck with C(heck ...
|
| Revision 1.31  2000/03/15 10:39:34  scott
| S/C LSDI - 2662 / USL-16144
| Updated to use new environment SK_BAL_FIX_LOC.
| 1 = Use Item/Wh default location if set
| 0 = Use W/H default location.
| i.e use incc_location of 1 else use location master record.
|
| Revision 1.30  2000/03/03 08:24:22  marnie
| SC2608 - (IKHK) Modified to put the variance qty. to the warehouse default location.
|
| Revision 1.29  2000/02/14 06:25:10  scott
| Updated to correct programs that update database tables without locking the record. This does not cause a problem with the character based code but causes a problem with GVision.
|
| Revision 1.28  1999/12/10 04:16:02  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.27  1999/12/06 01:30:35  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.26  1999/11/03 07:31:52  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.25  1999/10/20 20:16:04  nz
| Updated to use picking locations to add back stock.
|
| Revision 1.24  1999/10/20 01:38:55  nz
| Updated for remainder of old routines.
|
| Revision 1.23  1999/10/13 02:41:50  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.22  1999/10/13 00:01:08  nz
| Updated for location error.
|
| Revision 1.21  1999/10/08 05:32:13  scott
| First Pass checkin by Scott.
|
| Revision 1.20  1999/07/29 07:24:37  scott
| Updated for locking.
|
| Revision 1.19  1999/07/23 00:03:54  scott
| Check option was still updating locations.
|
| Revision 1.18  1999/06/20 05:19:46  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_bal_fix.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_bal_fix/sk_bal_fix.c,v 5.4 2001/08/09 09:18:08 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>

#define	ERR_SORT	1
#define ERR_ORDER	2
#define ERR_COMM	3
#define ERR_BO		4
#define ERR_FORWARD	5
#define ERR_ONHAND	6
#define ERR_LOCAT	7
#define ERR_SERIAL	8
#define ERR_LOT		9

#define	UP_OK		 (fixReport [0] == 'U' || fixReport [0] == 'A')
#define	ALL_OK		 (fixReport [0] == 'A')

#define	SERIAL 		 (inmr_rec.serial_item [0] == 'Y' && \
	          	  	  inmr_rec.costing_flag [0] == 'S')

#define	SER_OK		 ((insf_rec.status [0] == 'F' || \
		    	       insf_rec.status [0] == 'C') && \
		    	       insf_rec.receipted [0] != 'N')

#define	DP			inmr_rec.dec_pt

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inisRecord	inis_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct insfRecord	insf_rec;
struct inumRecord	inum_rec;
struct inuvRecord	inuv_rec;
struct intrRecord	intr_rec;
struct inloRecord	inlo_rec;

	/*-----------------
	| Error Messages. |
	-----------------*/
	static char *err_desc [] = {
		"Sort code on Warehouse record Wrong.",
		"On Order quantity out of balance.",
		"Committed quantity out of balance.",
		"Backorder quantity out of balance.",
		"Forward order quantity out of balance.",
		"On Hand quantity out of balance.",
		"Location quantity out of balance.",
		"Serial item quantity out of balance.",
		"Lot quantity out of balance."
	};

	int	printerNumber = 1;
	
	int		firstTime		=	FALSE;
	int		errorFound		=	FALSE;
	int		envSkBalFixLoc	= FALSE;

	char	fixReport [2];

	float	totalIncc [5];

	FILE	*ftmp;

	char 	*data	= "data";

	float	StdCnvFct 		= 	1.00;
	long	LastLotDate		=	0L,
			currentHhccHash	=	0L;
	char	batchString [8];
	char	invalidString [8];

#include	<LocHeader.h>

/*=======================
| Function Declarations |
=======================*/
float 	TotalInsf 			(int);
float 	TotalLocations 		(long);
static 	int 	FloatZero 	(float);
void 	CalcInwuToLocation 	(void);
void 	CheckInwu 			(void);
void 	CloseDB 			(void);
void 	FindIncc 			(long, char *);
void 	HeadingOutput 		(void);
void 	InitML 				(void);
void 	InitTotal 			(void);
void 	LineOff 			(void);
void 	OpenDB 				(void);
void 	PrintErr 			(int, float, float, float);
void 	Process 			(void);
void 	ProcessIntr 		(long, long);
void 	shutdown_prog 		(void);
void	Reopen				(void);
void	Reclose				(void);

/*==========================
| Main Processing Routine. |
==========================*/
int    
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 3)
	{
		sprintf (err_str, "Usage : %s <printerNumber> <C(heck)/U(pdate)/A(ll Update)>", argv [0]);
		print_at (0,0,"%s\n\r", err_str);  
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);
	sprintf (fixReport, "%-1.1s", argv [2]);
	if (fixReport [0] != 'C' && fixReport [0] != 'U' && fixReport [0] != 'A')
	{
		sprintf (err_str, "Usage : %s <printerNumber> <C(heck)/U(pdate)/A(ll Update)>", argv [0]);
		print_at (0,0, "%s\n\r", err_str);
        return (EXIT_FAILURE);
	}

	strcpy (batchString, 	"  N/A  ");
	strcpy (invalidString, 	"INVALID");
	/*-------------------------------------
	| Check what default location to use. |
	-------------------------------------*/
	sptr = chk_env ("SK_BAL_FIX_LOC");
	envSkBalFixLoc	= (sptr == (char *)0) ? 0 : atoi (sptr);

	OpenDB ();

	init_scr ();
	InitML ();

	if (!strcmp (llctDefault, "          "))
	{
		print_mess ("Default location is not defined in warwehouse master");
		sleep (sleepTime);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}
		
	if (UP_OK)
		dsp_screen ("Processing : Checking and Updating Stock files.", comm_rec.co_no, comm_rec.co_name);
	else
		dsp_screen ("Processing : Checking Stock Files.", comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	HeadingOutput ();

	Process ();

	fprintf (ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (ftmp);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	if ((ftmp = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".11\n");
	fprintf (ftmp, ".L158\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESTOCK FILE INBALANCE REPORT\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());
	fprintf (ftmp, ".E NOTE Stock file (s) %s be updated.\n", (UP_OK) ? "Will" : "Won't");

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp, "|           DESCRIPTION OF ERROR.          ");
	fprintf (ftmp, "| OLD BALANCE ");
	fprintf (ftmp, "| NEW BALANCE ");
	fprintf (ftmp, "|    DIFF.    |\n");

	LineOff ();
}

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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	currentHhccHash	=	ccmr_rec.hhcc_hash;

	abc_selfield (ccmr, "ccmr_hhcc_hash");

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_hhwh_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	OpenLocation (currentHhccHash);
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (incc);
	abc_fclose (insf);
	abc_fclose (inwu);
	abc_fclose (inuv);
	abc_fclose (intr);
	abc_fclose (inis);
	abc_fclose (inlo);
	abc_fclose (ccmr);
	CloseLocation ();
	
	abc_dbclose (data);
}

/*==========================
| Main Processing Routine. |
==========================*/
void
Process (
 void)
{
	int		processCounter	=	0;
	char	group [29];

	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, (UP_OK) ? "u" : "r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		firstTime	= TRUE;
		errorFound	= FALSE;
		dsp_process ("Item No.", inmr_rec.item_no);
		sprintf (group,"%-1.1s%-11.11s%16.16s",
				inmr_rec.inmr_class,
				inmr_rec.category,
				inmr_rec.item_no);

		/*---------------------------------------------------------------
		| Not ideal but required to ensure number of cursors in SQL is 	|
		| not blown.													|
		---------------------------------------------------------------*/
		if (processCounter++ > 30)
		{
			Reclose ();
			Reopen ();
			processCounter = 0;
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct = (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = abc_add (inuv, &inuv_rec);
			if (cc)
				file_err (cc, inuv, "DBADD");
		}
		else
			abc_unlock (inuv);

		inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
		if (cc)
		{
			inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	=	inmr_rec.alt_uom;
			cc = abc_add (inuv, &inuv_rec);
			if (cc)
				file_err (cc, inuv, "DBADD");
		}
		else
			abc_unlock (inuv);

		inis_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (inis, &inis_rec, GTEQ, "r");
		while (!cc && inis_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	=	inis_rec.sup_uom;
			cc = find_rec (inuv, &inuv_rec, COMPARISON, "u");
			if (cc)
			{
				inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
				inuv_rec.hhum_hash	=	inis_rec.sup_uom;
				cc = abc_add (inuv, &inuv_rec);
				if (cc)
					file_err (cc, inuv, "DBADD");
			}
			else
				abc_unlock (inuv);

			cc = find_rec (inis, &inis_rec, NEXT, "u");
		}
		FindIncc (inmr_rec.hhbr_hash, group);

		if (totalIncc [0] != inmr_rec.on_order)
		{
			PrintErr 
			(
				ERR_ORDER, 
				inmr_rec.on_order, 
				totalIncc [0],
				totalIncc [0] - inmr_rec.on_order
			);
			inmr_rec.on_order = totalIncc [0];
		}

		if (totalIncc [1] != inmr_rec.committed)
		{
			PrintErr 
			(
				ERR_COMM, 
				inmr_rec.committed, 
				totalIncc [1],
				totalIncc [1] - inmr_rec.committed
			);
			inmr_rec.committed = totalIncc [1];
		}

		if (totalIncc [2] != inmr_rec.backorder)
		{
			PrintErr 
			(
				ERR_BO, 
				inmr_rec.backorder, 
				totalIncc [2],
			   	totalIncc [2] - inmr_rec.backorder
			);
			inmr_rec.backorder = totalIncc [2];
		}

		if (totalIncc [3] != inmr_rec.forward)
		{
			PrintErr 
			(
				ERR_FORWARD, 
				inmr_rec.forward, 
				totalIncc [3],
				totalIncc [3] - inmr_rec.forward
			);
			inmr_rec.forward = totalIncc [3];
		}

		if (totalIncc [4] != inmr_rec.on_hand)
		{
			PrintErr 
			(
				ERR_ONHAND, 
				inmr_rec.on_hand, 
				totalIncc [4],
				totalIncc [4] - inmr_rec.on_hand
			);
			inmr_rec.on_hand = totalIncc [4];
		}

		if (UP_OK)
		{
			if (errorFound == TRUE)
			{
				cc = abc_update (inmr,&inmr_rec);
				if (cc)
					file_err (cc, "inmr", "DBUPDATE");
			}	
			else
				abc_unlock (inmr);
		}
		if (errorFound == TRUE)
			LineOff ();
		
		cc = find_rec (inmr, &inmr_rec, NEXT, (UP_OK) ? "u" : "r");
	}
	abc_unlock (inmr);
}

void
LineOff (
 void)
{
	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------|\n");
}

void
PrintErr (
	int		err_number,
	float	old,
	float	_new,
	float	diff)
{
	if (FloatZero (diff))
		return;

	if (firstTime == TRUE)
	{
		fprintf (ftmp, "| %-16.16s ", inmr_rec.item_no);
		fprintf (ftmp, "| %-40.40s ", inmr_rec.description);
	}
	else
	{
		fprintf (ftmp, "| %-16.16s ", " ");
		fprintf (ftmp, "| %-40.40s ", " ");
	}
	fprintf (ftmp, "| %-40.40s ", err_desc [ err_number - 1 ]);
	firstTime = FALSE;

	fprintf (ftmp, "|%12.2f ", old);
	fprintf (ftmp, "|%12.2f ", _new);
	fprintf (ftmp, "|%12.2f |\n", diff);

	errorFound = TRUE;
}

void
InitTotal (
 void)
{
	int	i;

	for (i = 0; i < 5; i++)
		totalIncc [i] = 0.00;
}

/*======================
| Process incc records | 
======================*/
void
FindIncc (
 long	hhbr_hash,
 char	*group)
{
	float	lo_qty 			= 0.00;
	float	QuantityError 	= 0.00;
	float	tot_insf 		= 0.00;
	int		updateIncc;
	char	tempLocation [11];

	InitTotal ();
	incc_rec.hhbr_hash	=	hhbr_hash;
	cc = find_rec (incc, &incc_rec, GTEQ, (UP_OK) ? "u" : "r");
	while (!cc && incc_rec.hhbr_hash == hhbr_hash)
	{
		updateIncc = FALSE;

		if (strncmp (incc_rec.sort,group,28))
		{
			strcpy (incc_rec.sort,group);
			updateIncc = TRUE;
		}

		if (MULT_LOC)
		{
			lo_qty = TotalLocations (incc_rec.hhwh_hash);

			if (lo_qty != incc_rec.closing_stock)
			{
				QuantityError = incc_rec.closing_stock - lo_qty;
				
				PrintErr
				 (
					ERR_LOCAT, 
				   	lo_qty, 
				   	incc_rec.closing_stock, 
				   	QuantityError 
				);
				if (UP_OK)
				{
					/*----------------------------------------------
					| See if default location exists at warehouse. |
					----------------------------------------------*/
					cc = FindLocation 
						 (
							incc_rec.hhwh_hash,
							inmr_rec.std_uom,
							(envSkBalFixLoc) ? incc_rec.location
											 : llctDefault,
							PickLocation,
							&inlo_rec.inlo_hash
						);
					if (!cc)
					{
						abc_selfield (inlo, "inlo_inlo_hash");
						cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
						if (cc)
							file_err (cc, (char *)inlo, "DBFIND");
						
						strcpy (err_str, DateToString (TodaysDate ()));
						InLotLocation 
						 (
							inlo_rec.hhwh_hash,
							incc_rec.hhcc_hash,
							inlo_rec.hhum_hash,
							inlo_rec.uom,	
							inlo_rec.lot_no,
							inlo_rec.slot_no,
							inlo_rec.location,
							inlo_rec.loc_type,
							err_str,
							QuantityError,
							1.00,
							inlo_rec.loc_status,
							inlo_rec.pack_qty,
							inlo_rec.chg_wgt,
							inlo_rec.gross_wgt,
							inlo_rec.cu_metre,
							inlo_rec.sknd_hash
						);
					}
					else
					{
						strcpy (err_str, DateToString (TodaysDate ()));
						if (strcmp (incc_rec.location, "          "))
						{
							strcpy (tempLocation, (envSkBalFixLoc) 
											 ? incc_rec.location
											 : llctDefault);
						}
						else
							strcpy (tempLocation, llctDefault);

						InLotLocation 
						(
							incc_rec.hhwh_hash,
							incc_rec.hhcc_hash,
							inmr_rec.std_uom,
							inmr_rec.sale_unit,
							 (inmr_rec.lot_ctrl [0] != 'Y') ? batchString : invalidString,
							 (inmr_rec.lot_ctrl [0] != 'Y') ? batchString : invalidString,
							tempLocation,
							"L",
							(inmr_rec.lot_ctrl [0] != 'Y') ? "00/00/0000" : err_str,
							QuantityError,
							1.00,
							"A",
							0.00,
							0.00,
							0.00,
							0.00,
							0L
						);
					}
				}
			}
		}
		if (SERIAL)
		{
			tot_insf = TotalInsf (incc_rec.hhwh_hash);
			if (tot_insf != incc_rec.closing_stock)
			{
				QuantityError = tot_insf - incc_rec.closing_stock;
				PrintErr (ERR_SERIAL, 
					   incc_rec.closing_stock, 
					   tot_insf, 
					   QuantityError);

				if (UP_OK && QuantityError != 0.00)
				{
					incc_rec.opening_stock += QuantityError;
					incc_rec.closing_stock = 	incc_rec.opening_stock + 
				    		   				 	incc_rec.pur + 
										     	incc_rec.receipts -
											   	incc_rec.issues -
											   	incc_rec.sales +
											   	incc_rec.adj;
					updateIncc = TRUE;
				}
			}
		}
		totalIncc [0] += incc_rec.on_order;
		totalIncc [1] += incc_rec.committed;
		totalIncc [2] += incc_rec.backorder;
		totalIncc [3] += incc_rec.forward;
		totalIncc [4] += incc_rec.closing_stock;

		/*------------------------------------
		| Check if forecast option is blank. |
		------------------------------------*/
		if (incc_rec.ff_option [0] == ' ')
		{
			strcpy (incc_rec.ff_option, "A");
			updateIncc = TRUE;
		}
		/*----------------------------------
		| Check if method option is blank. |
		----------------------------------*/
		if (incc_rec.ff_method [0] == ' ')
		{
			strcpy (incc_rec.ff_method, "A");
			updateIncc = TRUE;
		}
		/*----------------------------------------
		| Check if allow replenishment is blank. |
		----------------------------------------*/
		if (incc_rec.allow_repl [0] == ' ')
		{
			strcpy (incc_rec.allow_repl, "E");
			updateIncc = TRUE;
		}
		/*-----------------------------
		| Check if ABC code is blank. |
		-----------------------------*/
		if (incc_rec.abc_code [0] == ' ')
		{
			strcpy (incc_rec.abc_code, "A");
			updateIncc = TRUE;
		}
		/*------------------------------------
		| Check if ABC update flag is blank. |
		------------------------------------*/
		if (incc_rec.abc_update [0] == ' ')
		{
			strcpy (incc_rec.abc_update, "Y");
			updateIncc = TRUE;
		}

		CalcInwuToLocation ();
		CheckInwu ();

		if (UP_OK)
		{
			if (updateIncc)
			{
				cc = abc_update (incc,&incc_rec);
				if (cc)
					file_err (cc, "incc", "DBUPDATE");
			}
			else
				abc_unlock (incc);
		}
		if (ALL_OK)
		{
			ProcessIntr 
			 (
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash
			);
		}
		cc = find_rec (incc, &incc_rec, NEXT, (UP_OK) ? "u" : "r");
	}
	if (UP_OK)
		abc_unlock (incc);
	
}

void
CalcInwuToLocation (
 void)
{
	float	LocUomQty		=	0.00,
			Diff			=	0.00;

	abc_selfield (inlo, "inlo_mst_loc");

	inwu_rec.hhwh_hash		=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash		=	0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ, (UP_OK) ? "u" : "r");
	while (!cc && inwu_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		LocUomQty			= 	0.00;
		inlo_rec.hhwh_hash 	= 	inwu_rec.hhwh_hash;
		inlo_rec.hhum_hash 	= 	inwu_rec.hhum_hash;
		strcpy (inlo_rec.location, "          ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		while (!cc && inlo_rec.hhwh_hash 	== 	inwu_rec.hhwh_hash &&
					  inlo_rec.hhum_hash 	== 	inwu_rec.hhum_hash)
		{
			LocUomQty	+= inlo_rec.qty;
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");

		}

		Diff			=	LocUomQty - inwu_rec.closing_stock;
		inwu_rec.adj 	+= 	Diff;

		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		if (UP_OK)
		{
			cc = abc_update (inwu,&inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBUPDATE");
		}
		cc = find_rec (inwu, &inwu_rec, NEXT, (UP_OK) ? "u" : "r");
	}
}

void
CheckInwu (
 void)
{
	int		new_inwu;

	float	opening_stock	=	0.00,
			receipts		=	0.00,
			pur				=	0.00,
			issues			=	0.00,
			adj				=	0.00,
			sales			=	0.00,
			closing_stock	=	0.00;

	inwu_rec.hhwh_hash		=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash		=	0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ, "r");
	while (!cc && inwu_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		opening_stock	+= 	inwu_rec.opening_stock;
		receipts		+= 	inwu_rec.receipts;
		pur				+= 	inwu_rec.pur;
		issues			+= 	inwu_rec.issues;
		adj				+= 	inwu_rec.adj;
		sales			+= 	inwu_rec.sales;
		closing_stock	+= 	inwu_rec.closing_stock;

		cc = find_rec (inwu, &inwu_rec, NEXT, "r");
	}
	inwu_rec.hhwh_hash		=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash		=	inmr_rec.std_uom;
	new_inwu = find_rec (inwu, &inwu_rec, COMPARISON, "u");
	if (new_inwu)
	{
		inwu_rec.opening_stock	=	0.00;
		inwu_rec.receipts		=	0.00;
		inwu_rec.pur			=	0.00;
		inwu_rec.issues			=	0.00;
		inwu_rec.adj			=	0.00;
		inwu_rec.sales			=	0.00;
		inwu_rec.closing_stock	=	0.00;
	}
	if (incc_rec.opening_stock != opening_stock)
		inwu_rec.opening_stock += (incc_rec.opening_stock - opening_stock);

	if (incc_rec.receipts != receipts)
		inwu_rec.receipts += (incc_rec.receipts - receipts);

	if (incc_rec.pur != pur)
		inwu_rec.pur += (incc_rec.pur - pur);

	if (incc_rec.issues != issues)
		inwu_rec.issues += (incc_rec.issues - issues);

	if (incc_rec.adj != adj)
		inwu_rec.adj += (incc_rec.adj - adj);

	if (incc_rec.sales != sales)
		inwu_rec.sales += (incc_rec.sales - sales);

	if (incc_rec.closing_stock != closing_stock)
		inwu_rec.closing_stock += (incc_rec.closing_stock - closing_stock);

	if (new_inwu)
		cc = abc_add (inwu, &inwu_rec);
	else
		cc = abc_update (inwu, &inwu_rec);
	if (cc)
		file_err (cc, inwu, "DBADD/DBUPDATE");
}

/*===================================
| Get total number of serial items. |
===================================*/
float	
TotalInsf (
 int hhwh_hash)
{
	float	total_ser = 0;

	cc = find_hash (insf, &insf_rec, GTEQ, "r", hhwh_hash);
	while (!cc && insf_rec.hhwh_hash == hhwh_hash)
	{
		if (SER_OK)
			total_ser++;

		cc = find_hash (insf, &insf_rec, NEXT, "r", hhwh_hash);
	}
	return (total_ser);
}

/*==================
| Total locations. | 
==================*/
float	
TotalLocations (
 long	HHWH_HASH)
{
	float	lo_qty 	= 0.00;
	float	CnvFct	= 0.00;

	abc_selfield (inlo, "inlo_hhwh_hash");
	inlo_rec.hhwh_hash 	= 	HHWH_HASH;
	cc = find_rec (inlo,&inlo_rec,GTEQ, (UP_OK) ? "u" : "r");
	while (!cc && inlo_rec.hhwh_hash == HHWH_HASH)
	{
		inum_rec.hhum_hash	=	inlo_rec.hhum_hash;

		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			inlo_rec.hhum_hash	=	inmr_rec.std_uom;
			strcpy (inlo_rec.uom, inmr_rec.sale_unit);
			inum_rec.cnv_fct = 1.00;
		}

		if (UP_OK)
		{
			CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
			inlo_rec.cnv_fct 	=	CnvFct;
			strcpy (inlo_rec.uom, inum_rec.uom);
			cc = abc_update (inlo, &inlo_rec);
			if (cc)
				file_err (cc, (char *)inlo, "DBUPDATE");
		
			inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	=	inlo_rec.hhum_hash;
			cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
			if (cc)
			{
				inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
				inuv_rec.hhum_hash	=	inlo_rec.hhum_hash;
				cc = abc_add (inuv, &inuv_rec);
				if (cc)
					file_err (cc, inuv, "DBADD");
			}
		}
		lo_qty += inlo_rec.qty;
		cc = find_rec (inlo, &inlo_rec, NEXT, (UP_OK) ? "u" : "r");
	}
	if (UP_OK)
		abc_unlock (inlo);

	return (lo_qty);
}

/*
 *	Minor support functions
 */
static int
FloatZero (
 float	m)
{
	return (fabs (m) < 0.006);
}

void
InitML (
 void)
{
	err_desc [0]	= strdup (ML ("Sort code on warehouse record wrong."));
	err_desc [1]	= strdup (ML ("On order quantity out of balance."));
	err_desc [2]	= strdup (ML ("Committed quantity out of balance."));
	err_desc [3]	= strdup (ML ("Backorder quantity out of balance."));
	err_desc [4]	= strdup (ML ("Forward order quantity out of balance."));
	err_desc [5]	= strdup (ML ("On hand quantity out of balance."));
	err_desc [6]	= strdup (ML ("Location quantity out of balance."));
	err_desc [7]	= strdup (ML ("Serial item quantity out of balance."));
	err_desc [8]	= strdup (ML ("Lot quantity out of balance."));
}

/*====================================================
| Create a balance B/F transaction to balance intr's |
====================================================*/
void
ProcessIntr (
 long	hhbr_hash,
 long	hhcc_hash)
{
	float	TotalQty		=	0.00;
	float	quantityDiff	=	0.00;

	intr_rec.hhbr_hash	=	hhbr_hash;
	cc = find_rec (intr, &intr_rec, GTEQ, "r");
	while (!cc && intr_rec.hhbr_hash == hhbr_hash)
	{
		if (intr_rec.hhcc_hash	!= 	hhcc_hash)
		{
			cc = find_rec (intr, &intr_rec, NEXT, "r");
			continue;
		}
		switch (intr_rec.type)
		{
		case 1:
		case 2:
		case 4:
		case 5:
		case 7:
		case 10:
		case 12:
			TotalQty	+=	intr_rec.qty;
			break;
	
		case 3:
		case 6:
		case 8:
		case 9:
		case 11:
		case 13:
			TotalQty	-=	intr_rec.qty;
			break;
	
		default:
			break;
		}
		cc = find_rec (intr, &intr_rec, NEXT, "r");
	}
	if ((no_dec (incc_rec.closing_stock) - no_dec (TotalQty)) == 0)
		return;

	quantityDiff	=	incc_rec.closing_stock - TotalQty;
	ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	intr_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	intr_rec.date		=	0L;
	strcpy (intr_rec.co_no, ccmr_rec.co_no);
	strcpy (intr_rec.br_no, ccmr_rec.est_no);
	intr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
	intr_rec.hhum_hash	=	inmr_rec.std_uom;
	intr_rec.type		=	1;
	intr_rec.date		=	0L;
	strcpy (intr_rec.batch_no, " ");
	strcpy (intr_rec.ref1, "OPENING");
	strcpy (intr_rec.ref2, "BALANCE");
	intr_rec.qty		=	quantityDiff;
	intr_rec.cost_price	=	0.00;
	intr_rec.sale_price	=	0.00;
	strcpy (intr_rec.stat_flag, "0");
	cc = abc_add (intr, &intr_rec);
	if (cc)
		file_err (cc, intr, "DBADD");
}
void
Reopen (void)
{
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (insf, insf_list, INSF_NO_FIELDS, "insf_hhwh_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_hhbr_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	OpenLocation (currentHhccHash);
}
void
Reclose (void)
{
	abc_fclose (inum);
	abc_fclose (incc);
	abc_fclose (insf);
	abc_fclose (inwu);
	abc_fclose (inuv);
	abc_fclose (intr);
	abc_fclose (inis);
	abc_fclose (inlo);
	abc_fclose (intr);
	abc_fclose (ccmr);
	CloseLocation ();
}
