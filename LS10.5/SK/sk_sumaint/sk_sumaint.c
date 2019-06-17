/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_sumaint.c,v 5.4 2001/10/09 06:02:56 robert Exp $
|  Program Name  : (sk_sumaint.c) 
|  Program Desc  : (Add / Update Branch Supplier Records) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/1986        |
|---------------------------------------------------------------------|
| $Log: sk_sumaint.c,v $
| Revision 5.4  2001/10/09 06:02:56  robert
| updated to increase input mask of Currency Code from 2 to 3 characters
|
| Revision 5.3  2001/08/09 09:20:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:46:00  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:36  scott
| Update - LS10.5
|
| Revision 4.5  2001/05/23 06:37:35  scott
| Updated to make weight 4 decimal places and volume 3 decimal places as per inventory item UOM maintenance.
|
| Revision 4.4  2001/05/08 08:42:40  cha
| Updated as message was till in supplier Pur UOM.
|
| Revision 4.3  2001/05/07 03:30:17  cha
| Updated to fix problem with conversion
|
| Revision 4.2  2001/05/05 07:10:55  scott
| Updated to allow entry of min/normal order qty and order multiple in base UOM
|
| Revision 4.1  2001/04/28 04:39:01  scott
| Updates for strange part of cents
|
| Revision 4.0  2001/03/09 02:39:12  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/02/05 01:49:52  scott
| Update to only display records from inis depending on level.
|
| Revision 3.1  2001/01/30 09:47:51  scott
| Updated to use app.schema
|
| Revision 3.0  2000/10/10 12:21:24  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:32  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:12:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.27  2000/06/13 07:58:00  scott
| Updated to reset fields if not new record and supplier UOM changed.
|
| Revision 1.26  2000/06/13 05:03:34  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.25  2000/06/02 07:10:11  ana
| SC2754 - 16216 Saved the ord_multiple, min_order and norm_order in standard uom.Input quantities are based on the supplier UOM. - USL
|
| Revision 1.24  2000/05/31 05:41:30  scott
| Updated to give detailed message to user regarding minimum and normal order
| quantities and the base UOM.
|
| Revision 1.23  2000/05/22 05:59:01  scott
| Updated to prevent stocking and purchase UOM from being in different groups.
| Programs that use multiple UOM don't allow this so have to ensure maintenance function does now not allow. Will investigate what it required to allow this when it becomes an issue with client.
|
| Revision 1.22  2000/02/07 08:11:30  ana
| (07/02/2000) SC2463/15942 Corrected validation of inis.
|
| Revision 1.21  2000/02/07 00:30:29  scott
| Updated to change order lot size error message to match prompts.
|
| Revision 1.20  2000/01/21 01:36:30  cam
| Changes for GVision compatibility.  Fix calls to print_mess ().
|
| Revision 1.19  2000/01/18 20:53:11  cam
| Changes for GVision compatibility.  Fixed display of Dsp window in
| DisplaySupplier ()
|
| Revision 1.18  2000/01/17 07:16:14  marnie
| Modified to update inis_lcost_date from system date.
|
| Revision 1.17  2000/01/12 03:45:15  ronnel
| 12/01/2000 Modified to update last cost when changing the cost.
|
| Revision 1.16  2000/01/05 06:14:52  scott
| Updated to address issues related to outer size as reported by LSANZ from IED.
|
| Revision 1.15  2000/01/02 03:02:46  scott
| Updated to fix problem with outersize.
|
| Revision 1.14  1999/11/11 06:00:09  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.13  1999/11/09 03:57:31  scott
| Updated to use default from supplier for lead times if not defined as "N".
| S/C 2009 ASL.
|
| Revision 1.12  1999/10/27 06:55:13  scott
| Updated for error creating multiple inis records.
|
| Revision 1.11  1999/10/26 03:37:35  scott
| Updated for missing language translations
|
| Revision 1.10  1999/10/13 02:42:17  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.9  1999/10/12 21:20:45  scott
| Updated by Gerry from ansi project.
|
| Revision 1.8  1999/10/08 05:32:57  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:49  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|$Log: sk_sumaint.c,v $
|Revision 5.4  2001/10/09 06:02:56  robert
|updated to increase input mask of Currency Code from 2 to 3 characters
|
|Revision 5.3  2001/08/09 09:20:11  scott
|Updated to add FinishProgram () function
|
|Revision 5.2  2001/08/06 23:46:00  scott
|RELEASE 5.0
|
|Revision 5.1  2001/07/25 02:19:36  scott
|Update - LS10.5
|
|Revision 4.5  2001/05/23 06:37:35  scott
|Updated to make weight 4 decimal places and volume 3 decimal places as per inventory item UOM maintenance.
|
|Revision 4.4  2001/05/08 08:42:40  cha
|Updated as message was till in supplier Pur UOM.
|
|Revision 4.3  2001/05/07 03:30:17  cha
|Updated to fix problem with conversion
|
|Revision 4.2  2001/05/05 07:10:55  scott
|Updated to allow entry of min/normal order qty and order multiple in base UOM
|
|Revision 4.1  2001/04/28 04:39:01  scott
|Updates for strange part of cents
|
|Revision 4.0  2001/03/09 02:39:12  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.2  2001/02/05 01:49:52  scott
|Update to only display records from inis depending on level.
|
|Revision 3.1  2001/01/30 09:47:51  scott
|Updated to use app.schema
|
|Revision 3.0  2000/10/10 12:21:24  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.1  2000/09/07 02:31:32  scott
|Updated to add new suppier search as per stock and customer searches.
|
|Revision 2.0  2000/07/15 09:12:05  gerry
|Forced Revision No Start 2.0 Rel-15072000
|
|Revision 1.27  2000/06/13 07:58:00  scott
|Updated to reset fields if not new record and supplier UOM changed.
|
|Revision 1.26  2000/06/13 05:03:34  scott
|New Search routine that allow multi level searches using
|Item number, description, Alpha code, alternate no, maker number, selling group,
|buying group using AND or OR searches.
|New search beings routines into the library to speed up GVision.
|See seperate release notes.
|
|Revision 1.25  2000/06/02 07:10:11  ana
|SC2754 - 16216 Saved the ord_multiple, min_order and norm_order in standard uom.Input quantities are based on the supplier UOM. - USL
|
|Revision 1.24  2000/05/31 05:41:30  scott
|Updated to give detailed message to user regarding minimum and normal order
|quantities and the base UOM.
|
|Revision 1.23  2000/05/22 05:59:01  scott
|Updated to prevent stocking and purchase UOM from being in different groups.
|Programs that use multiple UOM don't allow this so have to ensure maintenance function does now not allow. Will investigate what it required to allow this when it becomes an issue with client.
|
|Revision 1.22  2000/02/07 08:11:30  ana
|(07/02/2000) SC2463/15942 Corrected validation of inis.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_sumaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_sumaint/sk_sumaint.c,v 5.4 2001/10/09 06:02:56 robert Exp $";

#define MAXWIDTH	135
#include <pslscr.h>
#include <minimenu.h>
#include <number.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <arralloc.h>
#include <errno.h>

#define 	SEL_UPDATE		0
#define 	SEL_IGNORE		1
#define 	SEL_DELETE		2
#define 	DEFAULT			99

	extern	int	TruePosition;

	int		byCompany 		=	FALSE,
			byBranch		=	FALSE,
			byWarehouse		=	FALSE;
		
	int		newItem 	= 0,
			envVarCrCo,
			envVarCrFind;

	int		dspOpen = FALSE;

	char	*tptr,
			*currentUser;

	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct esmrRecord	esmr_rec;
struct ineiRecord	inei_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inmrRecord	inmr_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocr_rec;
struct podtRecord	podt_rec;
struct polhRecord	polh_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct inccRecord	incc_rec;
struct inuvRecord	inuv_rec;


	char	*ccmr2	= "ccmr2",
			*inis2	= "inis2",
			*inum2	= "inum2",
			*sumr2	= "sumr2",
			*data	= "data";

char	sp_prompt [42];
char	lp_prompt [42];
char	envPoShipDefault [2];

int		EDIT_ONLY = FALSE;

char	messageNOQ [61],
		messageMOQ [61],
		messageOLS [61],
		promptNOQ [31],
		promptMOQ [31],
		promptOLS [31];

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	update [1];
	char	previousItem [17];
	char	stockDate [11];
	char	systemDate [11];
	char	itemNumber [17];
	char	itemDesc [41];
	double	supplierCost;
	double	normalCost;
	char	stdUom [5];
	char	stdUomDesc [31];
	char	supUom [5];
	char	supUomDesc [31];
	float	oldLeadTime;
	char	oldPriority [3];
	char	defaultSupplier [sizeof sumr_rec.crd_no];
	char	PriorityDefault [3];
	char	PriorityMessage [133];
	float	minimumOrder;
	float	normalOrder;
	float	orderMultiple;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNumber",	 3, 2,   CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number.         ", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "desc",		 3, 70,   CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Item Description    ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc},
	{1, LIN, "crd_no",	 4, 2,   CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultSupplier, "Supplier Number.     ", " ",
		 NE, NO,  JUSTLEFT, "", "", sumr_rec.crd_no},
	{1, LIN, "crd_name",	 4, 70,   CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name       ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "cr_code",	 5, 2,   CHARTYPE,
		"UUU", "          ",
		" ", "", "Currency Code.       ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.code},
	{1, LIN, "cr_desc",	 6, 2,   CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Description.         ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "cf_code",	 5, 70,   CHARTYPE,
		"UU", "          ",
		" ", "", "Country/Freight Code    ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocf_rec.code},
	{1, LIN, "cf_desc",	 6, 70,   CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", "", "Description.            ", " ",
		 NA, NO,  JUSTLEFT, "", "", pocf_rec.description},
	{1, LIN, "priority",	 8, 2,   CHARTYPE,
		"UU", "          ",
		" ", local_rec.PriorityDefault, "Supplier Priority    ", local_rec.PriorityMessage,
		YES, NO,  JUSTLEFT, "CBW123456789", "", inis_rec.sup_priority},
	{1, LIN, "sitem_no",	8, 70,   CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", inmr_rec.item_no, "Supplier Item No.       ", " ",
		YES, NO,  JUSTLEFT, "", "", inis_rec.sup_part},
	{1, LIN, "supUom",	10, 2,  CHARTYPE,
		"AAAA", "          ",
		" ", "", "Purchase UOM         ", "",
		YES, NO, JUSTLEFT, "", "", local_rec.supUom},
	{1, LIN, "supUomDesc",	10, 28,  CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.supUomDesc},
	{1, LIN, "fob_cost",	10, 70, DOUBLETYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "0", (char *) sp_prompt, "Enter Latest Price in Suppliers Native Currency. ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.supplierCost},
	{1, LIN, "stdUom",	11, 2,  CHARTYPE,
		"AAAA", "          ",
		" ", "", "Standard UOM         ", "",
		NA, NO, JUSTLEFT, "", "", local_rec.stdUom},
	{1, LIN, "stdUomDesc",	11, 28,  CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.stdUomDesc},
	{1, LIN, "normalCost",	11, 70, DOUBLETYPE,
		"NNNNNNNN.NNNN", "          ",
		" ", "0", (char *) lp_prompt, " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.normalCost},
	{1, LIN, "pur_conv",	12, 2,  FLOATTYPE,
		"NNNNNNN.NNNN", "          ",
		" ", "", "Pur Conv Factor.  ", "",
		NA, NO, JUSTRIGHT, "", "", (char *) &inis_rec.pur_conv},
	{1, LIN, "ldate",	12, 70,  EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.stockDate, "Latest Cost Date.       ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *) &inis_rec.lcost_date},
	{1, LIN, "dflt_lead",14, 2,  CHARTYPE,
		"U", "          ",
		" ", " ", "Default Ship Method. ", "Enter default shipment method. A(ir) / S(ea) / L(and).",
		YES, NO, JUSTRIGHT, "LSA", "", inis_rec.dflt_lead},
	{1, LIN, "leadsea",		14, 70,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Lead Time (Sea.)        ", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inis_rec.sea_time},
	{1, LIN, "leadair",		15, 2,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Lead Time (Air.)     ", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inis_rec.air_time},
	{1, LIN, "leadland",		15, 70,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Lead Time (Land)        ", "Enter Lead times in days.",
		YES, NO, JUSTRIGHT, "0", "999.99", (char *) &inis_rec.lnd_time},
	{1, LIN, "minimumOrder",	17, 2,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", promptMOQ, messageMOQ,
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.minimumOrder},
	{1, LIN, "normalOrder",	17, 35,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", promptNOQ, messageNOQ,
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.normalOrder},
	{1, LIN, "orderMultiple",	17, 70,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1", promptOLS, messageOLS,
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.orderMultiple},
	{1, LIN, "weight",	18, 2,  FLOATTYPE,
		"NNNNN.NNNN", "          ",
		" ", "0", "Weight.              ", "Weight in Kg per Unit.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inis_rec.weight},
	{1, LIN, "pallet_size",	18, 35,  FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "0", "Pallet Size          ", "Pallet Size in cubic metres.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inis_rec.pallet_size},
	{1, LIN, "volume",	18, 70,  FLOATTYPE,
		"NNNNNN.NNN", "          ",
		" ", "0", "Volume.                 ", "Volume in cubic metres per Unit.",
		YES, NO, JUSTRIGHT, "0", "99999.99", (char *) &inis_rec.volume},
	{1, LIN, "duty",		19, 2,   CHARTYPE,
		"UU", "          ",
		" ", inmr_rec.duty, "Duty Code            ", "Blank if no duty applies ",
		 NO, NO,  JUSTLEFT, "", "", inis_rec.duty},
	{1, LIN, "d_desc",	19, 70,   CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Duty Description        ", " ",
		 NA, NO,  JUSTLEFT, "", "", podt_rec.description},
	{1, LIN, "licence",	20, 2,   CHARTYPE,
		"UU", "          ",
		" ", inmr_rec.licence, "Licence Code         ", "Blank if no licence applies",
		 NO, NO,  JUSTLEFT, "", "", inis_rec.licence},
	{1, LIN, "l_desc",	20, 70,   CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Licence Desc.           ", " ",
		 NA, NO,  JUSTLEFT, "", "", polh_rec.type},
	{0, LIN, "",		 0,  0,    INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*---------------------------------------------------------------
|	Structure for dynamic array,  for the uom lines for qsort	|
---------------------------------------------------------------*/
struct UomSort
{
	char	uomSort [25];
	char	uom [5];
	char	uomDesc [41];
}	*UOM;
	DArray uom_d;
	int		UomCnt = 0;

#include <FindSumr.h>

/*=======================
| Function Declarations |
=======================*/
int  	ProcessSupplier 		(void);
int  	UomSortDesc 			(const void *, const void *);
int  	ValidUOM 				(void);
int  	heading 				(int scn);
int  	spec_valid 				(int);
void 	CalculateConversion 	(void);
void 	CloseDB 				(void);
void 	DisplaySupplier 		(void);
void 	GetOtherDetails 		(void);
void 	OpenDB 					(void);
void 	SrchInum 				(char *);
void 	SrchPodt 				(char *);
void 	SrchPolh 				(char *);
void 	Update 					(void);
void 	shutdown_prog 			(void);
void 	UpdateMenu 				(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 2)
	{
		print_at(0,0, "Usage [%s] C(ompany) B(ranch) or W(arehouse)",argv [0]);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;

	if (argv [1][0] == 'C')
	{
		byCompany = TRUE;
		strcpy (local_rec.PriorityDefault, "C1");
		strcpy (local_rec.PriorityMessage, "First character represents C(ompany) followed by a priority. i.e a range from C1 to C9.");
	}
	else if (argv [1][0] == 'B')
	{
		byBranch = TRUE;
		strcpy (local_rec.PriorityDefault, "B1");
		strcpy (local_rec.PriorityMessage, "First character represents B(ranch) followed by a priority. i.e a range from B1 to B9.");
	}
	else if (argv [1][0] == 'W')
	{
		byWarehouse = TRUE;
		strcpy (local_rec.PriorityDefault, "W1");
		strcpy (local_rec.PriorityMessage, "First character represents W(arehouse) followed by a priority. i.e a range from W1 to W9.");
	}
	else
	{
		print_at(0,0, "Usage [%s] C(ompany) B(ranch) or W(arehouse)",argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	currentUser = getenv ("LOGNAME");

	/*---------------------------------------------
	| Shipment Default. A (ir) / L (and) / S (ea) |
	---------------------------------------------*/
	sptr = chk_env ("PO_SHIP_DEFAULT");
	if (sptr == (char *) 0)
		sprintf (envPoShipDefault, "S");
	else
	{
		switch (*sptr)
		{
		case	'S':
		case	's':
			sprintf (envPoShipDefault, "S");
			break;

		case	'L':
		case	'l':
			sprintf (envPoShipDefault, "L");
			break;

		case	'A':
		case	'a':
			sprintf (envPoShipDefault, "A");

		default:
			sprintf (envPoShipDefault, "S");
			break;
		}
	}

	envVarCrCo 	= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	strcpy (local_rec.stockDate, DateToString (comm_rec.inv_date));
	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	strcpy (branchNumber, (envVarCrCo == 0) ? " 0" : comm_rec.est_no);

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		EDIT_ONLY	= FALSE;
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		strcpy (local_rec.defaultSupplier, " ");
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newItem		= FALSE;

		local_rec.orderMultiple = 0.00;
		local_rec.minimumOrder 	= 0.00;
		local_rec.normalOrder 	= 0.00;
		local_rec.supplierCost 	= 0.00;
		inis_rec.fob_cost 		= 0.00;

		abc_unlock (inis);

		if (dspOpen)
		{
			dspOpen = FALSE;
			Dsp_close ();
		}

		if (!EDIT_ONLY)
		{
			/*-------------------------------
			| Enter screen 1 linear input . |
			-------------------------------*/
			search_ok = 1;
			strcpy (sp_prompt, "Supplier Price          ");
			strcpy (lp_prompt, "Local Price             ");
			strcpy (promptNOQ, "Normal Ord Qty.      ");
			strcpy (promptMOQ, "Min Order qty.       ");
			strcpy (promptOLS, "Order Lot size       ");
			heading (1);
			entry (1);
			if (prog_exit || restart)
				continue;
		}

		/*------------------------------
		| Edit screen 1 linear input . |
		------------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		if (inis_rec.pur_conv <= 0.00)
		{
			print_mess (ML(mlSkMess561));
			sleep (sleepTime);
			clear_mess ();
			EDIT_ONLY = TRUE;
			continue;
		}

		Update ();
	}
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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (ccmr2, ccmr);
	abc_alias (inis2, inis);
	abc_alias (sumr2, sumr);
	abc_alias (inum2, inum);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (polh,  polh_list, POLH_NO_FIELDS, "polh_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, 
		 (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inuv, inuv_list, INUV_NO_FIELDS, "inuv_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (ccmr2);
	abc_fclose (inei);
	abc_fclose (inis);
	abc_fclose (inis2);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (podt);
	abc_fclose (polh);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (incc);
	abc_fclose (inuv);
	abc_fclose (pocr);
	abc_fclose (pocf);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*-------------------------------
	| Validate Item Number  input . |
	-------------------------------*/
	if (LCHECK ("itemNumber"))
	{
		/*-------------------------
		| Search for part number. |
		-------------------------*/
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

	    if (inmr_rec.outer_size == 0.00)
			inmr_rec.outer_size = 1.0;

		DSP_FLD ("itemNumber");

		strcpy (local_rec.itemDesc, inmr_rec.description);
		DSP_FLD ("desc");

		/*----------------
		| Lookup Std UOM |
		----------------*/
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (local_rec.stdUom, "%-4.4s", " ");
			sprintf (local_rec.stdUomDesc, "%-30.30s", " ");
		}
		else
		{
			sprintf (local_rec.stdUom, "%-4.4s", inum_rec.uom);
			sprintf (local_rec.stdUomDesc, "%-30.30s", inum_rec.desc);
		}

		sprintf (lp_prompt, "Local Price %4.4s * %4.1f ", local_rec.stdUom,inmr_rec.outer_size);
		sprintf (promptNOQ, "Normal Ord Qty.(%s)", local_rec.stdUom);
		sprintf (promptMOQ, "Min Order qty. (%s)", local_rec.stdUom);
		sprintf (promptOLS, "Order Lot size (%s)", local_rec.stdUom);
		scn_write (1);
		DSP_FLD ("stdUom");
		DSP_FLD ("stdUomDesc");

		/*-------------------------------------------
		| Find Branch Inventory master File Record. |
		-------------------------------------------*/
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DisplaySupplier();
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Creditors Number Input. |
	----------------------------------*/
	if (LCHECK ("crd_no"))
	{
		if (dspOpen)
		{
			dspOpen = FALSE;
			Dsp_close ();
		}

		/*-----------------------
		| Search for Suppliers. |
		-----------------------*/
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		/*------------------------------
		| Lookup Supplier master file. |
		------------------------------*/
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.crd_no, zero_pad (sumr_rec.crd_no,6));
		strcpy (sumr_rec.est_no, branchNumber);
		cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*--------------------------------------------
		| Get currency file record for display only. |
		--------------------------------------------*/
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, sumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*---------------------------------------------------
		| Get Country/Freight file record for display only. |
		---------------------------------------------------*/
		strcpy (pocf_rec.co_no, comm_rec.co_no);
		strcpy (pocf_rec.code, sumr_rec.ctry_code);
		cc = find_rec (pocf, &pocf_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess118));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("crd_name");
		DSP_FLD ("cr_code");
		DSP_FLD ("cr_desc");
		DSP_FLD ("cf_code");
		DSP_FLD ("cf_desc");

		/*--------------------------------------
		| Find Inventory Supplier master file. |
		--------------------------------------*/
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, (byCompany) ? "  " : comm_rec.est_no);
		strcpy (inis_rec.wh_no, (!byWarehouse) ? "  " : comm_rec.cc_no);
		inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
		inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (inis, &inis_rec, COMPARISON, "w");
		if (!cc)
		{
			GetOtherDetails ();
			newItem = 0;
			entry_exit = 1;
			return (EXIT_SUCCESS);
		}
		else
		{
			cl_box (20,5,60,14);
			erase_box (20,5,60,14);
			move (1,7);  line (130);
			move (1,9);  line (130);
			move (1,13); line (130);
			move (1,16); line (130);
			scn_display (1);
			scn_write (1);
		}
		newItem = 1;

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Validate Supplier Priority Input. |
	-----------------------------------*/
	if (LCHECK ("priority"))
	{
		if (byCompany == TRUE && inis_rec.sup_priority [0] != 'C')
		{
			sprintf (err_str, "%s C", ML ("First character must be"));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (byBranch == TRUE && inis_rec.sup_priority [0] != 'B')
		{
			sprintf (err_str, "%s B", ML ("First character must be"));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (byWarehouse == TRUE && inis_rec.sup_priority [0] != 'W')
		{
			sprintf (err_str, "%s W", ML ("First character must be"));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*--------------------------------------------------
		| Item has no Supplier Record so it is a new item. |
		--------------------------------------------------*/
		cc = ProcessSupplier ();
		return (cc);
	}

	if (LCHECK ("supUom"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		sprintf (inum_rec.uom, "%-4.4s", temp_str);
		cc = find_rec ("inum2", &inum_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inis_rec.sup_uom = inum_rec.hhum_hash;
		sprintf (local_rec.supUomDesc, "%-30.30s", inum_rec.desc);

		sprintf (sp_prompt, "Supplier Price per %-4.4s ", local_rec.supUom);
		scn_write (1);
		CalculateConversion ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		if (strcmp (inum_rec.uom_group, inum_rec.uom_group))
		{
			print_mess (ML ("Currently Stocking and Purchase UOM group must be the same"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pur_conv");
		DSP_FLD ("supUomDesc");

		local_rec.normalCost = local_rec.supplierCost * (double) fourdec (inis_rec.pur_conv);
		if (pocr_rec.ex1_factor != 0.00)
		{
			local_rec.normalCost /= pocr_rec.ex1_factor;
		}
		else
			local_rec.normalCost = 0.00;

		local_rec.normalCost *= inmr_rec.outer_size;

		DSP_FLD ("normalCost");
		DSP_FLD ("fob_cost");

		sprintf (messageNOQ, ML ("Normal Order Qty must be in UOM of (%s)"),
								local_rec.stdUom);
		sprintf (messageMOQ, ML ("Minimum Order Qty must be in UOM of (%s)"),
								local_rec.stdUom);
		sprintf (messageOLS, ML ("Order Lot Size must be in UOM of (%s)"),
								local_rec.stdUom);

		sprintf (sp_prompt, "Supplier Price per %-4.4s ", local_rec.supUom);
		sprintf (promptNOQ, "Normal Ord Qty.(%s)", local_rec.stdUom);
		sprintf (promptMOQ, "Min Order qty. (%s)", local_rec.stdUom);
		sprintf (promptOLS, "Order Lot size (%s)", local_rec.stdUom);

		return (EXIT_SUCCESS);
	}

	/*----------------
	| Validate Duty. |
	----------------*/
	if (LCHECK ("duty"))
	{
		if (SRCH_KEY)
		{
			SrchPodt (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inis_rec.duty, "  "))
		{
			DSP_FLD ("d_desc");
			return (EXIT_SUCCESS);
		}

		strcpy (podt_rec.co_no, comm_rec.co_no);
		strcpy (podt_rec.code, inis_rec.duty);
		cc = find_rec (podt, &podt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess124));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("d_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate Licence. |
	-------------------*/
	if (LCHECK ("licence"))
	{
		if (SRCH_KEY)
		{
			SrchPolh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (!strcmp (inis_rec.licence, "  "))
		{
			DSP_FLD ("l_desc");
			return (EXIT_SUCCESS);
		}

		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);
		strcpy (polh_rec.lic_cate, inis_rec.licence);
		strcpy (polh_rec.lic_no, "          ");
		cc = find_rec (polh, &polh_rec, GTEQ, "r");
		if (cc || strcmp (polh_rec.lic_cate, inis_rec.licence))
		{
			print_mess (ML(mlStdMess154));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("l_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate Conv. Fct |
	--------------------*/
	if (LCHECK ("pur_conv"))
	{
		if (inis_rec.pur_conv <= 0)
		{
			print_mess (ML(mlSkMess321));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		clear_mess ();
		if (!ValidUOM ())
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate FOB Cost. |
	--------------------*/
	if (LCHECK ("fob_cost"))
	{
		local_rec.normalCost = local_rec.supplierCost * (double) fourdec (inis_rec.pur_conv);
		if (pocr_rec.ex1_factor != 0.00)
		{
			local_rec.normalCost /= pocr_rec.ex1_factor;
		}
		else
			local_rec.normalCost = 0.00;

		local_rec.normalCost *= inmr_rec.outer_size;
		
		inis_rec.lcost_date = StringToDate(local_rec.systemDate);
		DSP_FLD ("normalCost");
		DSP_FLD ("ldate");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Minimum Order	|
	---------------------------*/
	if (LCHECK ("minimumOrder"))
	{
		double x, y, fraction, integer; 
		float zfloat;

		if (local_rec.minimumOrder < 0.00)
		{
			print_mess (ML(mlSkMess322));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		x = local_rec.minimumOrder;
		y = local_rec.orderMultiple;
		if (local_rec.orderMultiple != 0.00)	
		{
			x = x / y;
			zfloat = x;	/* Baby do your magic */
			fraction = modf (zfloat, &integer);
			if (twodec (fraction) != 0.00)
			{
				print_mess (ML ("Please enter another Minimum Order Quantity."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		inis_rec.min_order = local_rec.minimumOrder;
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Normal Order		|
	---------------------------*/
	if (LCHECK ("normalOrder"))
	{
		if (local_rec.normalOrder < 0.00)
		{
			print_mess (ML(mlSkMess322));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		inis_rec.norm_order = local_rec.normalOrder;
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Order Multiple	|
	---------------------------*/
	if (LCHECK ("orderMultiple"))
	{
		double x, y, fraction, integer; 
		float zfloat;
		if (local_rec.orderMultiple < 0.00)
		{
			print_mess (ML(mlSkMess322));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		x = local_rec.minimumOrder;
		y = local_rec.orderMultiple;
		if (local_rec.orderMultiple != 0.00)	
		{
			x = x / y;
			zfloat = x;	/* Baby do your magic */
			fraction = modf (zfloat, &integer);
			if (twodec (fraction) != 0.00)
			{
				print_mess (ML("Min Order Quantity must be Multiple of the order Lot size."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		inis_rec.ord_multiple = local_rec.orderMultiple;
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Default lead time. |
	-----------------------------*/
	if (LCHECK ("dflt_lead"))
	{
		if (dflt_used)
		{
			if (sumr_rec.ship_method [0] == ' ' || 
				sumr_rec.ship_method [0] == 'N')
				strcpy (inis_rec.dflt_lead, envPoShipDefault);
			else
				strcpy (inis_rec.dflt_lead, sumr_rec.ship_method);
		}
		DSP_FLD ("dflt_lead");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
CalculateConversion (
 void)
{
	number	cnv_fct;
	number	std_cnv_fct;
	number	alt_cnv_fct;

	FLD ("pur_conv") = NA;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.pur_conv = 1.00;
		return;
	}

	inum2_rec.hhum_hash	=	inis_rec.sup_uom;
	cc = find_rec (inum, &inum2_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.pur_conv = 1.00;
		return;
	}

	if (strcmp (inum_rec.uom_group, inum2_rec.uom_group))
	{
		inis_rec.pur_conv = 1.00;
		FLD ("pur_conv") = YES;
		return;
	}
	
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);
	NumFlt (&alt_cnv_fct, inum2_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| std uom cnv_fct / sup uom cnv_fct = pur conv factor      |
	----------------------------------------------------------*/
	NumDiv (&std_cnv_fct, &alt_cnv_fct, &cnv_fct);

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	inis_rec.pur_conv = NumToFlt (&cnv_fct);
	FLD ("pur_conv") = NA;

	return;
}

/*==========================================
| Check whether uom is valid compared with |
| the dec_pt and the conversion factor.    |
| eg. std uom = kg     iss uom = gm        |
|     conv.fact = 1000 dec_pt = 2          |
|     issue 5 gm, converts to 0.005 kg     |
|     round to 2 dec_pt, new qty = 0.01 kg |
|     or 10gm                              |
|This is incorrect and not allowed.        |
==========================================*/
int
ValidUOM (
 void)
{
	long	numbers [7];
	int		ans;

	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (inis_rec.pur_conv > numbers [inmr_rec.dec_pt])
	{
		sprintf (err_str, ML(mlSkMess460), inmr_rec.dec_pt);
		ans = prmptmsg (err_str, "YyNn", 1, 23);
		move (1, 23);
		cl_line ();
		if (ans == 'n' || ans == 'N')
			return (FALSE);
	}
	return (TRUE);
}

/*==================================================
| Process other details once inis record is found. |
==================================================*/
void
GetOtherDetails (
 void)
{
	local_rec.oldLeadTime = inis_rec.lead_time;
	strcpy (local_rec.oldPriority, inis_rec.sup_priority);

	local_rec.minimumOrder 	= inis_rec.min_order;
	local_rec.normalOrder 	= inis_rec.norm_order;
	local_rec.orderMultiple = inis_rec.ord_multiple;
	local_rec.supplierCost 	= inis_rec.fob_cost / (double) fourdec (inis_rec.pur_conv);

	if (pocr_rec.ex1_factor != 0.00)
	{
		local_rec.normalCost = inis_rec.fob_cost;
		local_rec.normalCost /= pocr_rec.ex1_factor;
	}
	else
		local_rec.normalCost = 0.00;

	local_rec.normalCost *= inmr_rec.outer_size;

	DSP_FLD ("normalCost");

	if (strcmp (inis_rec.duty, "  "))
	{
		strcpy (podt_rec.co_no, comm_rec.co_no);
		strcpy (podt_rec.code, inis_rec.duty);
		cc = find_rec (podt, &podt_rec, COMPARISON, "r");
		if (cc)
			strcpy (inis_rec.duty, "  ");
		else
			DSP_FLD ("d_desc");

	}
	if (strcmp (inis_rec.licence, "  "))
	{
		strcpy (polh_rec.co_no, comm_rec.co_no);
		strcpy (polh_rec.est_no, comm_rec.est_no);
		strcpy (polh_rec.lic_cate, inis_rec.licence);
		strcpy (polh_rec.lic_no, "          ");
		cc = find_rec (polh, &polh_rec, GTEQ, "r");
		if (cc || strcmp (polh_rec.lic_cate, inis_rec.licence))
			strcpy (inis_rec.licence, "  ");
		else
			DSP_FLD ("l_desc");
	}

	inum_rec.hhum_hash	=	inis_rec.sup_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (local_rec.supUom, "%-4.4s", " ");
		sprintf (local_rec.supUomDesc, "%-30.30s", " ");
	}
	else
	{
		sprintf (local_rec.supUom, "%-4.4s", inum_rec.uom);
		sprintf (local_rec.supUomDesc, "%-30.30s", inum_rec.desc);
	}
	sprintf (messageNOQ, ML ("Normal Order Qty must be in UOM of (%s)"),
							local_rec.stdUom);
	sprintf (messageMOQ, ML ("Minimum Order Qty must be in UOM of (%s)"),
							local_rec.stdUom);
	sprintf (messageOLS, ML ("Order Lot Size must be in UOM of (%s)"),
							local_rec.stdUom);

	sprintf (sp_prompt, "Supplier Price per %-4.4s ", local_rec.supUom);
	sprintf (promptNOQ, "Normal Ord Qty.(%s)", local_rec.stdUom);
	sprintf (promptMOQ, "Min Order qty. (%s)", local_rec.stdUom);
	sprintf (promptOLS, "Order Lot size (%s)", local_rec.stdUom);
}

int
ProcessSupplier (
 void)
{
	inis2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis2_rec.sup_priority, inis_rec.sup_priority);
	strcpy (inis2_rec.co_no, inmr_rec.co_no);
	strcpy (inis2_rec.br_no,(byBranch || byWarehouse) ? comm_rec.est_no : "  ");
	strcpy (inis2_rec.wh_no, (byWarehouse) ? comm_rec.cc_no : "  ");
	cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
	if (!cc && !strcmp (inis2_rec.sup_priority, inis_rec.sup_priority) &&
				inis2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sumr2_rec.hhsu_hash	=	inis2_rec.hhsu_hash;
		cc = find_rec (sumr2, &sumr2_rec, COMPARISON, "r");
		if (!cc && sumr_rec.hhsu_hash != sumr2_rec.hhsu_hash)
		{
			sprintf (err_str, ML ("Item %s is on file for supplier %s as priority %s."), 
			clip (inmr_rec.item_no), 
			sumr2_rec.crd_no, 
			inis_rec.sup_priority);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*==================================================
| Update or add a record to inventory branch file. |
==================================================*/
void
Update (
 void)
{
	char	up_priority [3];
	long	hhsu_hash;

	strcpy (up_priority, inis_rec.sup_priority);

	/*-------------------------
	| Supplier record is new. |
	-------------------------*/
	if (newItem == 1)
	{
		hhsu_hash = sumr_rec.hhsu_hash;
	    clear ();
	    print_at (0,0, ML(mlStdMess035));
	    strcpy (inis_rec.stat_flag, "0");
		if (byCompany)
		{
	    	strcpy (inis_rec.co_no, comm_rec.co_no);
	    	strcpy (inis_rec.br_no, "  ");
	    	strcpy (inis_rec.wh_no, "  ");
		}
		else if (byBranch)
		{
	    	strcpy (inis_rec.co_no, comm_rec.co_no);
	    	strcpy (inis_rec.br_no, comm_rec.est_no);
	    	strcpy (inis_rec.wh_no, "  ");
		} 	
		else if (byWarehouse)
		{
	    	strcpy (inis_rec.co_no, comm_rec.co_no);
	    	strcpy (inis_rec.br_no, comm_rec.est_no);
	    	strcpy (inis_rec.wh_no, comm_rec.cc_no);
		}
	    inis_rec.hhbr_hash = inmr_rec.hhbr_hash;
	    inis_rec.hhsu_hash = hhsu_hash;

		if (inis_rec.dflt_lead [0] == 'S')
			inis_rec.lead_time = inis_rec.sea_time;

		if (inis_rec.dflt_lead [0] == 'A')
			inis_rec.lead_time = inis_rec.air_time;

		if (inis_rec.dflt_lead [0] == 'L')
			inis_rec.lead_time = inis_rec.lnd_time;

		inis_rec.fob_cost = local_rec.supplierCost * (double) fourdec (inis_rec.pur_conv);
	    cc = abc_add (inis, &inis_rec);
	    if (cc)
			file_err (cc, inis, "DBADD");
	}
	/*----------------------------
	| Existing Supplier record . |
	----------------------------*/
	else
	    UpdateMenu ();

	/*---------------------( Step One )--------------------------
	| Unlink all Inventory branch records (inei) for    		|
	| currently maintained item from supplier records (inis) 	|
	-----------------------------------------------------------*/
	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inei_rec.est_no, "  ");
	cc = find_rec (inei, &inei_rec, GTEQ, "u");
	while (!cc && inei_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (inei_rec.hhis_hash != 0L)
		{
			inei_rec.hhis_hash	=	0L;
			cc = abc_update ("inei", &inei_rec);
			if (cc)
				file_err (cc, inei, "DBUPDATE");
		}
		else
			abc_unlock (inei);

		cc = find_rec (inei, &inei_rec, NEXT, "u");
	}

	/*----------------------( Step Two )--------------------------
	| Find all inis records for current company and branch.		 |
	| If inventory branch record (inei) has not been linked to	 |
	| a supplier then link it to inventory supplier record(inis) |
	|                                                            |
	|                       ( Step Two.2 )                       |
	| If inventory branch record (inei) is being linked to a  	 |
	| supplier record that does not have a priority of one then  |
	| update the inventory supplier record (inis) priority to be |
	| one as no priority one supplier could have existed.        |
	|                                                            |
	|                       ( Step Two.3 )                       |
	| If an inventory branch record (inei) is updated then so    |
	| should all warehouse records within the branch be updated  |
	| with the priority of the inventory supplier (inis) lead    |
	| times.                                                     |
	------------------------------------------------------------*/
	inis2_rec.hhbr_hash		=	inmr_rec.hhbr_hash;
	strcpy (inis2_rec.sup_priority, "  ");
	strcpy (inis2_rec.co_no,	"  ");
	strcpy (inis2_rec.br_no,	"  ");
	strcpy (inis2_rec.wh_no,	"  ");
	cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
	while (!cc && inis2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		inuv_rec.hhum_hash	=	inis2_rec.sup_uom;
		cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
		if (cc)
		{
			inuv_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
			inuv_rec.hhum_hash	=	inis2_rec.sup_uom;
			inuv_rec.volume		=	inis2_rec.volume;
			cc = abc_add (inuv, &inuv_rec);
			if (cc)
				file_err (cc, inuv, "DBADD");
		}
		if (inis2_rec.sup_priority [1] != '1')
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
	    inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	    strcpy (inei_rec.est_no, inis2_rec.br_no);
	    cc = find_rec (inei, &inei_rec, COMPARISON, "w");
	    if (!cc)
		{
	    	inei_rec.hhis_hash = inis2_rec.hhis_hash;
	
	    	cc = abc_update (inei, &inei_rec);
	    	if (cc)
				file_err (cc, inei, "DBUPDATE");

			strcpy (ccmr_rec.co_no,  comm_rec.co_no);
			strcpy (ccmr_rec.est_no, "  ");
			strcpy (ccmr_rec.cc_no,  "  ");
			cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
			while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
			{
				incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
				incc_rec.hhbr_hash = inis2_rec.hhbr_hash;
				cc = find_rec (incc, &incc_rec, COMPARISON, "u");
				if (!cc) 
				{
					/* incc_lead_time stored in weeks */
					incc_rec.lead_time = inis2_rec.lead_time / 7;
	    			cc = abc_update (incc, &incc_rec);
	    			if (cc)
						file_err (cc, incc, "DBUPDATE");
				}
				cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
			}
		}
		else
			abc_unlock (inei);

		cc = find_rec (inis2, &inis2_rec, NEXT, "r");
	}
	strcpy (local_rec.previousItem, inmr_rec.item_no);
	return;
}

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE INVENTORY SUPPLIER RECORD WITH CHANGES MADE.   ", 
		  "" }, 
		{ " 2. IGNORE CHANGES JUST MADE TO INVENTORY SUPPLIER RECORD.", 
		  "" }, 
		{ " 3. DELETE INVENTORY SUPPLIER RECORD.                     ", 
		  "" }, 
		{ ENDMENU }
	};

/*===================
| Update mini menu. |
===================*/
void
UpdateMenu (
 void)
{
	for (;;)
	{
	    mmenu_print ("            U P D A T E    S E L E C T I O N .            ", upd_menu, 0);
	    switch (mmenu_select (upd_menu))
	    {
		case DEFAULT :
		case SEL_UPDATE :
			if (inis_rec.dflt_lead [0] == 'S')
				inis_rec.lead_time = inis_rec.sea_time;

			if (inis_rec.dflt_lead [0] == 'A')
				inis_rec.lead_time = inis_rec.air_time;

			if (inis_rec.dflt_lead [0] == 'L')
				inis_rec.lead_time = inis_rec.lnd_time;

			strcpy (inis_rec.stat_flag, "0");

			inis_rec.fob_cost = local_rec.supplierCost * (double) fourdec (inis_rec.pur_conv);
			cc = abc_update (inis, &inis_rec);
			if (cc)
				file_err (cc, inis, "DBUPDATE");
			
			return;

		case SEL_IGNORE :
			abc_unlock (inis);
			return;

		case SEL_DELETE :
			abc_unlock (inis);
			cc = abc_delete (inis);
			if (cc)
				file_err (cc, inis, "DBDELETE");
			return;
			break;

		default :
			break;
	    }
	}
}

/*==============================
| Search for Duty master file. |
==============================*/
void
SrchPodt (
 char	*key_val)
{
	work_open ();
	save_rec ("#Dt", "#Duty Description");
	strcpy (podt_rec.co_no, comm_rec.co_no);
	sprintf (podt_rec.code, "%-2.2s", key_val);
	cc = find_rec (podt, &podt_rec, GTEQ, "r");
	while (!cc && !strncmp (podt_rec.code, key_val, strlen (key_val)) &&
		      !strcmp (podt_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (podt_rec.code, podt_rec.description);
		if (cc)
			break;
		cc = find_rec (podt, &podt_rec, NEXT, "r");
	}
	disp_srch ();
	work_close ();
	strcpy (podt_rec.co_no, comm_rec.co_no);
	sprintf (podt_rec.code, "%-2.2s", temp_str);
	cc = find_rec (podt, &podt_rec, COMPARISON, "r");
}

/*=================================
| Search for Licence master file. |
=================================*/
void
SrchPolh (
 char	*key_val)
{
	char	lic_cat [3];

	work_open ();
	save_rec ("#Lc", "#Licence Description");
	strcpy (lic_cat, "  ");
	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	sprintf (polh_rec.lic_cate, "%-2.2s", key_val);
        strcpy (polh_rec.lic_no, "          ");
	cc = find_rec (polh, &polh_rec, GTEQ, "r");
	while (!cc &&
               !strncmp (polh_rec.lic_cate, key_val, strlen (key_val)) &&
               !strcmp (polh_rec.co_no, comm_rec.co_no) &&
               !strcmp (polh_rec.est_no, comm_rec.est_no))
	{
		if (strcmp (polh_rec.lic_cate, lic_cat))
		{
		    cc = save_rec (polh_rec.lic_cate, polh_rec.type);
		    if (cc)
			break;
		    strcpy (lic_cat, polh_rec.lic_cate);
		}
		cc = find_rec (polh, &polh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (polh_rec.co_no, comm_rec.co_no);
	strcpy (polh_rec.est_no, comm_rec.est_no);
	sprintf (polh_rec.lic_cate, "%-2.2s", temp_str);
        strcpy (polh_rec.lic_no, "          ");
	cc = find_rec (polh, &polh_rec, GTEQ, "r");
	if (cc)
		file_err (cc, polh, "DBFIND");
}

void
SrchInum (
 char	*key_val)
{
	int		i;

	/*-----------------------------
	| Allocate the initial array. |
	-----------------------------*/
	ArrAlloc (&uom_d, &UOM, sizeof (struct UomSort), 100);
	UomCnt = 0;

	work_open ();
	save_rec ("#UOM", "#UOM Description ");
	sprintf (inum_rec.uom_group, "%-20.20s", " ");
	inum_rec.hhum_hash = 0L;

	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while (!cc)
	{
		if (strncmp (inum_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum, &inum_rec, NEXT, "r");
			continue;
		}
		/*-------------------------------------------------
		| Check the array size before adding new element. |
		-------------------------------------------------*/
		if (!ArrChkLimit (&uom_d, UOM, UomCnt))
			sys_err ("ArrChkLimit (UOM)", ENOMEM, PNAME);

		/*----------------------------------------- 
		| Load values into array element shipCnt. |
		-----------------------------------------*/
		sprintf (UOM [UomCnt].uomSort,"%-20.20s%-4.4s",inum_rec.uom_group,
													   inum_rec.uom);
		sprintf (UOM [UomCnt].uom, 		"%-4.4s", 	inum_rec.uom);
		sprintf (UOM [UomCnt].uomDesc, 	"%-40.40s", inum_rec.desc);

		/*--------------------------
		| Increment array counter. |
	   	--------------------------*/
		UomCnt++;

		cc = find_rec (inum, &inum_rec, NEXT, "r");
	}
	/*------------------------------------------- 
	| Sort the array in item description order. |
	-------------------------------------------*/
	qsort (UOM, UomCnt, sizeof (struct UomSort), UomSortDesc);

	for (i = 0; i < UomCnt; i++)
	{
		cc = save_rec (UOM [i].uom, UOM [i].uomDesc);
		if (cc)
			break;
	}
	/*---------------------------
	| Free up the array memory. |
	---------------------------*/
	ArrDelete (&uom_d);

	cc = disp_srch ();
	work_close ();

	return;
}

void
DisplaySupplier (
 void)
{
	char	displayString [200];
	int		firstSupplier	=	FALSE;

	/*-----------------------------------
	| setup supplier item display		|
	-----------------------------------*/
	dspOpen = TRUE;
	Dsp_open (20, 5, 11);
	Dsp_saverec ("Pri| Br | Wh |Supplier|              Supplier Name             ");
	Dsp_saverec ("No.| No | No | Number |                                        ");
	Dsp_saverec ("");

	strcpy (local_rec.defaultSupplier, " ");
	inis2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inis2_rec.sup_priority, "   ");
	strcpy (inis2_rec.co_no, "  ");
	strcpy (inis2_rec.br_no, "  ");
	strcpy (inis2_rec.wh_no, "  ");
	cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
	while (!cc && inis2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sumr2_rec.hhsu_hash	=	inis2_rec.hhsu_hash;
		cc = find_rec (sumr2, &sumr2_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}

		if (strcmp (inis2_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
		if (byCompany && inis2_rec.sup_priority [0] != 'C')
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
		if (byBranch && inis2_rec.sup_priority [0] != 'B')
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
		if (byWarehouse && inis2_rec.sup_priority [0] != 'W')
		{
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
			continue;
		}
		if (firstSupplier == FALSE)
			strcpy (local_rec.defaultSupplier, sumr2_rec.crd_no);

		firstSupplier = TRUE;
	
		sprintf 
		(
			displayString, 
			"%s ^E %2.2s ^E %2.2s ^E %6.6s ^E%40.40s",
			inis2_rec.sup_priority,
			inis2_rec.br_no,
			inis2_rec.wh_no,
			sumr2_rec.crd_no,
			sumr2_rec.crd_name
		);
		Dsp_saverec (displayString);

		cc = find_rec (inis2, &inis2_rec, NEXT, "r");
	}
	Dsp_srch ();
}

int 
UomSortDesc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct UomSort a = *(const struct UomSort *) a1;
	const struct UomSort b = *(const struct UomSort *) b1;

	result = strcmp (a.uomSort, b.uomSort);

	return (result);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		swide ();
		clear ();

		if (byCompany)
			rv_pr (ML(" Inventory Supplier Maintenance by Company"), 34,0,1);
		if (byBranch)
			rv_pr (ML(" Inventory Supplier Maintenance by Branch"), 34,0,1);
		if (byWarehouse)
			rv_pr (ML(" Inventory Supplier Maintenance by Warehouse"), 34,0,1);
		
		print_at (0,90, ML(mlSkMess096), local_rec.previousItem);
		move (0, 1);
		line (130);

		box (0, 2, 131, 18);
		line_at (7, 1,130);
		line_at (9, 1,130);
		line_at (13,1,130);
		line_at (16,1,130);
		
		if (byCompany || byBranch)
		{
			sprintf (err_str, ML(mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);
			rv_pr (err_str, 1, 22, 0);
		}
		if (byBranch)
		{
			sprintf (err_str, ML(mlStdMess039),comm_rec.est_no,comm_rec.est_name);
			rv_pr (err_str, 60, 22, 0);
		}
		if (byWarehouse)
		{
			sprintf (err_str, ML(mlStdMess276),
							comm_rec.co_no, comm_rec.co_name,
							comm_rec.est_no,comm_rec.est_name,
							comm_rec.cc_no,comm_rec.cc_name);
			rv_pr (err_str, 1, 22, 0);
		}

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
