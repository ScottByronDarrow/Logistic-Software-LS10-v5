/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: delivery.c,v 5.4 2002/07/24 08:39:30 scott Exp $
|	Program Name : (tr_delivery.c)								  	  |
|	Program Desc : (Transport Delivery Confirmation.)      			  |
|                  (as per tr_del_conf program continuation. )   	  |
|---------------------------------------------------------------------|
|  Author        : Liza A. Santos  | Date Written  : 13/06/96         |
|---------------------------------------------------------------------|
| $Log: delivery.c,v $
| Revision 5.4  2002/07/24 08:39:30  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/06/26 06:08:43  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:22:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:53:48  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:21:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:42:50  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.4  2001/02/06 10:07:56  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
| Revision 3.3  2000/12/22 08:02:07  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.2  2000/12/11 05:59:00  scott
| Updated to fix core dump and clean code including addition of app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: delivery.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_delivery/delivery.c,v 5.4 2002/07/24 08:39:30 scott Exp $";

#define SLEEP_TIME	2
#define LONG_SLEEP_TIME	5
#define SCN_HEADER	  1
#define SCN_ITEMS	  2
#define MAXSCNS		  2
#define MAXLINES	200
#define TABLINES	 12
#define MAXHHBR 	200
#define MAXTRX	 	500
#define TRAN_TYPE   19

#include <pslscr.h>
#include <ml_tr_mess.h>
#include <hot_keys.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <LocHeader.h>
#include <CustomerService.h>

#define SR	store[ line_cnt ]
#define	LSR	store[lcount[SCN_ITEMS]]

#define		INVOICE 	(cohr_rec.type[0] == 'I' || cohr_rec.type[0] == 'T')

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	long	hhveHashPassed,
			hhtrHashPassed,
			hhcoHashPassed,
			hhcuHashPassed;

#include	<ser_value.h>
#include	<MoveRec.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct trveRecord	trve_rec;
struct trveRecord	trve2_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct trhrRecord	trhr2_rec;
struct trhrRecord	trhr3_rec;
struct cohrRecord	cohr_rec;
struct cumrRecord	cumr_rec;
struct extfRecord	extf_rec;

char	*trhr2	=	"trhr2",
		*data	=	"data";

	Money	*incc_prf	=	&incc_rec.c_prf_1;
	Money	*incc_val	=	&incc_rec.c_val_1;
	float	*incc_con	=	&incc_rec.c_1;

	float	ScreenDisc (float);
	float	ToStdUom (float);
	float	ToLclUom (float);

	struct	storeRec {
		char 	_item_no[17];
		char	_item_desc[41];
		char	_desc2[41];
		char	_UOM[5];
		char	_lot_ctrl[2];
		float	_order;
		float	_delivered;
		float	_returned;
		int		_line_no;
		long	_hhbr_hash;
		long	_hhwh_hash;
		long	_hhcc_hash;
		long	_hhco_hash;
		long	_hhcl_hash;
		long	_origHhbr;
		long	_hhum_hash;
		float	_origOrdQty;
		float	_cnv_fct;
		int		line_no;
		char 	_reas_code[3];
	} store[ MAXLINES ];


char	*scn_desc[] = {
		"HEADER SCREEN.",
		"ITEM SCREEN."
	};

/*===========================================
| The structure envVar groups the values of |
| environment settings together.            |
===========================================*/
struct tagEnvVar
{
	int		dbMcurr;
	int		reverseDiscount;
	int		dbNettUsed;
} envVar;

/*===========================
| Local & Screen Structures. |
============================*/
struct
{
	long	hhbr_hash;
	long	deliveryDate;
	long	deliveryDate2;
	long	actualDelDate;

	float	qtyOrdered;
	float	qtyDelivered;
	float	qtyReturn;

	char	invoiceNumber  [9];
	char	customerAddress	[41];
	char	customerNumber [7];
	char	tripNumber [13];
	char	tripNumber2 [13];
	char	driverCode [7];
	char	driverName [41];
	char	vehicleRef [11];
	char	vehicleRef2 [11];
	char	actualDelTime [8];
	char	item_no [17];
	char	item_desc [41];
	char	RfNumber [11];
	char	UOM 	[5];
	char	LL [2];
	char	dummy [11];
} local_rec;

static	struct	var	vars[] =
{
	{SCN_HEADER, LIN, "vehicleNumber",	 5, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "  ", "  Vehicle Reference   :", "Enter Vehicle No. [SEARCH]",
		NA, NO,  JUSTLEFT, "", "", local_rec.vehicleRef2},

	{SCN_HEADER, LIN, "expectedDelDate", 	6, 25, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "  Expected Del Date   :", " Enter Expected Date or use [SEARCH WINDOW #1].", 
		 NA, NO,  JUSTLEFT, " ", "", (char *)&local_rec.deliveryDate2}, 

	{SCN_HEADER, LIN, "tripNumber",	 7, 25, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", "  ", "  Trip Number         :", "Enter Trip No.",
		NA, NO,  JUSTLEFT, "", "", local_rec.tripNumber2},

	{SCN_HEADER, LIN, "rf_number",	 9, 25, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "  ", " RF Number            :", "Enter RF Number.",
		NO, NO,  JUSTLEFT, "", "", local_rec.RfNumber},

	{SCN_HEADER, LIN, "actualDelDate",	 11, 25, EDATETYPE,
		"DD/DD/DD", "          ", 
		" ", "  ", " Actual Delivery Date :", "Enter Delivery Date or Enter for System Date",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.actualDelDate},

	{SCN_HEADER, LIN, "actualDelTime",	 12, 25, CHARTYPE,
		"NN:NN:NN", "          ", 
		" ", " ", " Actual Delivery Time :", "Enter Delivery Time or Enter for System Time.",
		YES, NO,  JUSTLEFT, "", "", local_rec.actualDelTime} ,

	{SCN_HEADER, LIN, "driverNumber", 	14, 25, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "",   " Driver/Trucker       :", " Enter Driver/Trucker Code or [F5] for [Search] . ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.driverCode}, 

	{SCN_HEADER, LIN, "driverName", 	14, 45, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", "",   "", " ", 
		 NA, NO,  JUSTLEFT, " ", "", local_rec.driverName}, 

	{SCN_ITEMS, TAB, "itemNumber",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "    Item number     ", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.item_no},

	{SCN_ITEMS, TAB, "itemDesc",	 0, 0, CHARTYPE,
	    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
    	" ", "", "            Item Description              ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.item_desc},

	{SCN_ITEMS, TAB, "itemUom",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", " ", "UOM.", " Unit of Measure ",
		 NA, NO, JUSTLEFT, "", "", local_rec.UOM},

	{SCN_ITEMS, TAB, "quantityOrdered",	 0,  0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "   Quantity Order   ", "",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&local_rec.qtyOrdered},

	{SCN_ITEMS, TAB, "quantityDelivered",	 0,  0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "  Quantity Delivered  ", "",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.qtyDelivered},

	{SCN_ITEMS, TAB, "quantityReturn",	 0,  0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "  Quantity Returned  ", "",
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.qtyReturn},
	{SCN_ITEMS, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<MoveAdd.h>
/*=======================
| Function declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchExtf 		(char *);
void 	LoadDisp 		(int);
void 	Update 			(void);
void 	UpdateReturns 	(int);
void 	tab_other 		(int);
void 	CheckEnvironment (void);
int 	LoadPItems 		(void);
int 	CheckReturns 	(void);
int 	CalcMonth 		(long);
int 	heading 		(int);
int 	spec_valid 		(int);
float 	ScreenDisc 		(float);
float 	ToStdUom 		(float);
float 	ToLclUom 		(float);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	int		i;

	if (argc < 5)
	{
		print_at (0,0,mlTrMess700,argv[0]);
		return (EXIT_FAILURE);
	}

	hhveHashPassed = atol (argv[1]);	
	hhtrHashPassed = atol (argv[2]);	
	hhcoHashPassed = atol (argv[3]);	
	hhcuHashPassed = atol (argv[4]);	

	/*---------------------------------------------------------------------
	| Check environment variables and set values in the envVar structure. |
	---------------------------------------------------------------------*/
	CheckEnvironment ();

	SETUP_SCR 	 (vars);

	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_ITEMS, store, sizeof (struct storeRec));
#endif
	
	tab_col = 6;
	tab_row = 8;

	FLD ("itemNumber")			= NA;
	FLD ("itemDesc")			= NA;
	FLD ("quantityOrdered")		= NA;
	FLD ("quantityDelivered")	= YES;
	FLD ("quantityReturn")		= YES;
	tab_other (line_cnt);


	OpenDB ();
	clear ();

	FLD ("LL") 	=	ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") 	=	YES;

	while (!prog_exit)
	{
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	TRUE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		init_ok 	= 	TRUE;

		init_vars (SCN_HEADER);	

		for (i = 0; i < 2; i++)
			tab_data[i]._desc = scn_desc[i]; 

		LoadDisp (FALSE);

		if (line_cnt > lcount[2])
			line_cnt = lcount[2];

		while (prog_exit == 0 && !restart)
		{
			vars[label ("itemNumber")].row	=	MAXLINES;
			LoadPItems ();
			swide ();
			heading (SCN_ITEMS);	
			scn_display (SCN_ITEMS);
			edit (SCN_ITEMS);
			if (!restart)
			{
				do
				{
					edit_all ();
					if (restart)
						break;
					cc = CheckReturns ();
				} while (cc);
				
				strcpy (local_rec.vehicleRef2, local_rec.vehicleRef);
				local_rec.deliveryDate2 = local_rec.deliveryDate;
				strcpy (local_rec.tripNumber2, local_rec.tripNumber);
				if (!restart)
					Update ();
			}

			prog_exit = 1;
			restart   = 1; 
		}
	}

    /* call to shutdown_prog() moved here instead of
       up there two closing brackets above */ 
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	abc_alias (trhr2, trhr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_hhve_hash"); 
	open_rec (trhr,  trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash"); 
	open_rec (trhr2, trhr_list, TRHR_NO_FIELDS, "trhr_trip_name"); 
	open_rec (trln,  trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash"); 
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash"); 
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no"); 
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash"); 
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash"); 
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (extf,  extf_list, EXTF_NO_FIELDS, "extf_id_no"); 
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML ("Warehouse not found."));
	   	sleep (sleepTime);
		clear_mess ();
	   	return;
	}
	OpenLocation (ccmr_rec.hhcc_hash);
	LL_EditLoc	=	TRUE;
	LL_EditLot	=	TRUE;
	LL_EditDate	=	TRUE;
	LL_EditSLot	=	TRUE;
	IgnoreAvailChk	=	TRUE;

	abc_selfield ("ccmr", "ccmr_hhcc_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (trhr);
	abc_fclose (cohr);
	abc_fclose (trln);
	abc_fclose (trve);
	abc_fclose (cumr);
	abc_fclose (extf);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inwu);
	abc_fclose (coln);
	abc_fclose (incc);
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose ("move");
	CloseLocation ();
	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (scn != cur_screen)
		scn_set (scn);


	switch (scn)
	{
	case SCN_HEADER :

		clear ();
		rv_pr (ML (mlTrMess008), 52,0,1);
		box (0,4,132,10);
		move (1,8);  line (131);
		move (1,10); line (131);
		move (1,13); line (131);
		move (0,2);  line (131);
		move (0,21); line (131);
		LoadDisp (TRUE);
		scn_write (scn);
		scn_display (scn);
		break;

	case SCN_ITEMS :

		clear ();
		rv_pr (ML (mlTrMess008), 57,0,1);
		box (0,1,132,3);

		print_at (2,2,	ML (mlTrMess009), 	local_rec.vehicleRef);
		print_at (2,60,	ML (mlTrMess010), 	local_rec.invoiceNumber);
		print_at (3,2,	ML (mlTrMess011), 	DateToString (local_rec.deliveryDate));
		print_at (3,60,	ML (mlStdMess012), 	local_rec.customerNumber, " ");
		print_at (4,2,	ML (mlTrMess012), 	local_rec.tripNumber);
		print_at (4,60,	ML (mlTrMess013), 	local_rec.customerAddress);

		break;
	}

	print_at (22,0, ML(mlStdMess038), comm_rec.co_no,clip (comm_rec.co_short));
	print_at (22,50,ML(mlStdMess039),comm_rec.est_no,clip (comm_rec.est_short));
	print_at (22,95, ML(mlStdMess099),comm_rec.cc_no, comm_rec.cc_name);
	line_cnt = 0;	
	scn_write (scn);
	scn_display (scn);

    return (EXIT_SUCCESS);
}


/*==================
| Field Validation |
==================*/
int
spec_valid (
 int field)
{
	int		TempLine	=	0;
	long	time	=	0L;
	long	l_time	=	0L;
	char	time_str[6];

	if (LCHECK ("quantityDelivered"))
	{
		if (dflt_used)
			local_rec.qtyDelivered = local_rec.qtyOrdered;

		if (local_rec.qtyDelivered > local_rec.qtyOrdered)
		{
			errmess (ML (mlTrMess032));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (local_rec.qtyDelivered != 0.0)
		{
			SR._delivered	=	local_rec.qtyDelivered;
			putval (line_cnt);
		}
		else
		if (local_rec.qtyDelivered == 0.0)
		{
			SR._delivered = local_rec.qtyDelivered;
			putval (line_cnt);
		}
		return (EXIT_SUCCESS);
	}

	/*----------------------- 
	| Validate Date Entered	| 
 	-----------------------*/
	if (LCHECK ("quantityReturn"))
	{
		float	sumDeliveryReturn = 0.00;

		if (dflt_used)
			local_rec.qtyReturn = local_rec.qtyOrdered - local_rec.qtyDelivered;

		sumDeliveryReturn	=	local_rec.qtyDelivered 	+   local_rec.qtyReturn;
		if (sumDeliveryReturn > local_rec.qtyOrdered)
		{
			errmess (ML (mlTrMess014));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (FLD ("LL") != ND)
			FLD ("LL") 	=	NA;

		if (local_rec.qtyReturn >= 0L)
		{
			SR._returned	=	local_rec.qtyReturn;
			if (SK_BATCH_CONT || MULT_LOC)
			{
				FLD ("LL") 	=	YES;
				do
				{
					strcpy (local_rec.LL, "N");
					get_entry (field + 1);
					if (restart)
					{
						restart = FALSE;
						return (EXIT_FAILURE);
					}
				} while (spec_valid (field + 1));
			}
			putval (line_cnt);
		}
		return (EXIT_SUCCESS);
	}

  	/*--------------------------- 
  	| Validate Actual Del Dte	| 
  	---------------------------*/
	if (LCHECK ("actualDelDate"))
	{
		if (dflt_used)
			local_rec.actualDelDate = TodaysDate ();
		DSP_FLD ("actualDelDate");
		return (EXIT_SUCCESS);
	}

 	/*--------------------------- 
  	| Validate Actual Del Time	| 
  	---------------------------*/
	if (LCHECK ("actualDelTime"))
	{
		if (dflt_used)
			strcpy (time_str,TimeHHMMSS ());

		time	=	atot (time_str);
		if (time > 1440L)
		{
			print_mess (ML (mlTrMess033));
			sleep (sleepTime);
			strcpy (local_rec.actualDelTime  ,"        "); 
			return 1;
		}
		else if (time == 1440L)
			l_time -= time;

		sprintf (local_rec.actualDelTime , "%8.8s",time_str);

		DSP_FLD ("actualDelTime");

		return (EXIT_SUCCESS);
	}

 	/*--------------------------- 
  	| Validate Actual Del Time	| 
  	---------------------------*/
	if (LCHECK ("driverNumber"))
	{
		if (SRCH_KEY)
		{
			SrchExtf (temp_str);
			return (EXIT_SUCCESS);
		}

  		strcpy (extf_rec.co_no, comm_rec.co_no);
		strcpy (extf_rec.code, local_rec.driverCode);
		cc = find_rec (extf, &extf_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlTrMess005));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.driverName, extf_rec.name);
		DSP_FLD ("driverNumber");
		DSP_FLD ("driverName");
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		LL_EditLoc	=	TRUE; 
		LL_EditLot	=	TRUE;
		LL_EditDate	=	TRUE;
		LL_EditSLot	=	TRUE;
		
		TempLine	=	lcount[2];
		InputExpiry = TRUE;	

		cc = DisplayLL
			 (
				line_cnt,
				tab_row + 3,
				tab_col + 3,
				4,
				SR._hhwh_hash,
				SR._hhum_hash,
				SR._hhcc_hash,
				SR._UOM,
				local_rec.qtyReturn,
				SR._cnv_fct,
				local_rec.actualDelDate,
				FALSE,
				 (local_rec.LL[0] == 'Y'),
				SR._lot_ctrl
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount[2] = (line_cnt + 1 > lcount[2]) ? line_cnt + 1 : lcount[2];
		scn_write (2);
		scn_display (2);
		lcount[2] = TempLine;
		if (cc)
			return (EXIT_FAILURE);
		else
			return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==========================
| Search for Trucker file. |
==========================*/
void
SrchExtf (
 char	*key_val)
{
	work_open ();
	strcpy  (extf_rec.co_no,  comm_rec.co_no);
	sprintf (extf_rec.code,  "%-6.6s", key_val);
	save_rec ("#TC", "#Trucker Name");
	cc = find_rec (extf, &extf_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp  (extf_rec.co_no, comm_rec.co_no) 
	&&     !strncmp (extf_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (extf_rec.code, extf_rec.name);
		if (cc)
			break;

		cc = find_rec (extf, &extf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (extf_rec.co_no, comm_rec.co_no);
	sprintf (extf_rec.code, "%-6.6s", temp_str);
	cc = find_rec (extf, &extf_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, (char *)extf, "DBFIND");
}

void
LoadDisp (
	int		displayFields)
{
	trve_rec.hhve_hash = hhveHashPassed;
	cc = find_rec (trve, &trve_rec, EQUAL, "r");
	if (!cc)
	{
		strcpy (local_rec.vehicleRef, trve_rec.ref);
		strcpy (local_rec.vehicleRef2, local_rec.vehicleRef);
	}

	trhr_rec.hhtr_hash = hhtrHashPassed;
	cc = find_rec (trhr, &trhr_rec, EQUAL,"r");
	if (!cc)
	{
		local_rec.deliveryDate = trhr_rec.del_date;
		strcpy (local_rec.tripNumber, trhr_rec.trip_name);
		strcpy (local_rec.driverCode, trhr_rec.driver);
		local_rec.deliveryDate2 = local_rec.deliveryDate;
		strcpy (local_rec.tripNumber2, local_rec.tripNumber);
		if (!strcmp (local_rec.driverCode, "      "))
			FLD ("driverNumber") = NA;
	}

	cohr_rec.hhco_hash = hhcoHashPassed;
	cc = find_rec (cohr, &cohr_rec, EQUAL,"r");
	if (!cc)
	{
		strcpy (local_rec.invoiceNumber, cohr_rec.inv_no);	
		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc)
		{
			strcpy (local_rec.customerAddress, cumr_rec.dl_adr1);
			strcpy (local_rec.customerNumber, cumr_rec.dbt_no);
		}

	}
	local_rec.actualDelDate = TodaysDate ();
	strcpy (local_rec.actualDelTime, TimeHHMMSS());

	if (INVOICE)
	{
		vars[label ("quantityOrdered")].prmpt 	= strdup (" Quantity Order "); 
		vars[label ("quantityDelivered")].prmpt = strdup (" Qty Delivered  "); 
		vars[label ("quantityReturn")].prmpt 	= strdup (" Qty Returned   "); 
	}
	else
	{
		vars[label ("quantityOrdered")].prmpt 	= strdup (" Qty Requested  "); 
		vars[label ("quantityDelivered")].prmpt = strdup (" Qty Recovered  "); 
		vars[label ("quantityReturn")].prmpt 	= strdup (" Qty Outstanding"); 
	}
	if (displayFields)
	{
		DSP_FLD ("actualDelDate");
		DSP_FLD ("actualDelTime");
		DSP_FLD ("vehicleNumber");
		DSP_FLD ("expectedDelDate");
		DSP_FLD ("tripNumber");
		DSP_FLD ("driverNumber");
	}
	return;
}

/*==============================================
| Load details from Presales Branch/Warehouse. |
===============================================*/
int
LoadPItems (
 void)
{
	float	std_cnv_fct = 0.00;

	abc_selfield (inmr, "inmr_hhbr_hash");

	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	scn_set (SCN_ITEMS);
	lcount[SCN_ITEMS] = 0;
	vars[scn_start].row = 2;

	coln_rec.hhco_hash = hhcoHashPassed;
	coln_rec.line_no   = 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc  && coln_rec.hhco_hash == hhcoHashPassed) 
	{
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
   		incc_rec.hhbr_hash 	= coln_rec.hhbr_hash;
   		incc_rec.hhcc_hash 	= coln_rec.incc_hash;
   		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
		
		local_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (local_rec.item_desc, inmr_rec.description);

		LSR._hhwh_hash	=	incc_rec.hhwh_hash;
		LSR._hhbr_hash	=	inmr_rec.hhbr_hash;
		LSR._hhcc_hash	=	incc_rec.hhcc_hash;
		LSR._hhum_hash	=	coln_rec.hhum_hash;
		strcpy (LSR._item_no, 	inmr_rec.item_no);
		strcpy (LSR._item_desc, inmr_rec.description);
		strcpy (LSR._desc2,		inmr_rec.description2);
		strcpy (LSR._lot_ctrl,	inmr_rec.lot_ctrl);

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		std_cnv_fct	=	inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		line_cnt	= lcount[SCN_ITEMS];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		LSR._cnv_fct 		=	inum_rec.cnv_fct/std_cnv_fct;
		strcpy (local_rec.UOM, 		 inum_rec.uom);
		strcpy (LSR._UOM, inum_rec.uom);

		local_rec.qtyOrdered	=	ToLclUom (coln_rec.q_order);
		local_rec.qtyDelivered	=	local_rec.qtyOrdered - 
									ToLclUom (coln_rec.qty_del);
		local_rec.qtyReturn		=	ToLclUom (coln_rec.qty_ret);

		LSR._order 				=	ToLclUom (coln_rec.q_order);
		LSR._delivered			=	ToLclUom (coln_rec.q_order) -
									ToLclUom (coln_rec.qty_del);
		LSR._returned			=	ToLclUom (coln_rec.qty_ret);
		LSR._line_no			=	coln_rec.line_no;
		LSR._hhco_hash			=	coln_rec.hhco_hash;


		putval (lcount[SCN_ITEMS]++);
		if (lcount[SCN_ITEMS] > MAXLINES)
			break; 

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}

	vars[scn_start].row = lcount[SCN_ITEMS]; 
	scn_set (1);
	if (lcount[SCN_ITEMS] == 0)
	{
		print_mess (ML (mlTrMess037));
		sleep (sleepTime);
	}
	return ((lcount[SCN_ITEMS]) ? 0 : 2);
}
/*=======================
| Updates The Files		|
=======================*/
void
Update (
 void)
{
	double	totalTaxAmt	=	0.00,
			totalLine 	=	0.00,
			totalDisc	=	0.00,
			totalTax 	=	0.00,
			totalGst 	=	0.00;

	int		noTax;

	clear ();
	print_at (0,0, ML (mlTrMess015));
	fflush (stdout);

	trhr_rec.hhtr_hash = hhtrHashPassed;
	cc	= find_rec (trhr, &trhr_rec, EQUAL, "u");
	if (!cc)
	{
		trhr_rec.act_date = local_rec.actualDelDate;
		trhr_rec.act_time = atot (local_rec.actualDelTime);
		strcpy (trhr_rec.rf_number, local_rec.RfNumber);
		cc = abc_update (trhr, &trhr_rec);
		if (cc)
		{
			abc_unlock (trhr);
			file_err (cc, (char *)trhr, "DBUPDATE");
		}
	}
	abc_unlock (trhr);

	cohr_rec.hhco_hash = hhcoHashPassed;
	cc	= find_rec (cohr, &cohr_rec, EQUAL,"u");
	if (cc)
	{
		abc_unlock (cohr);
		file_err (cc, (char *)cohr, "DBFIND");
	}

	if (cohr_rec.tax_code[0] == 'A' || cohr_rec.tax_code[0] == 'B')
		noTax = TRUE;
	else
		noTax = FALSE;

	cohr_rec.gross = 0.00;
	cohr_rec.disc  = 0.00;
	cohr_rec.tax   = 0.00;
	cohr_rec.gst   = 0.00;

	scn_set (SCN_ITEMS);
	for (line_cnt	=	0;line_cnt < lcount[SCN_ITEMS];line_cnt++)
	{
		getval (line_cnt);

		coln_rec.hhco_hash  = hhcoHashPassed;
		coln_rec.line_no	= SR._line_no;
		cc	=	find_rec (coln, &coln_rec, EQUAL, "u");
		if (!cc)
		{
			inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, EQUAL, "u");
			if (cc)
			{
				abc_unlock (inmr);
				file_err (cc, (char *)inmr, "DBFIND");
			}

			strcpy (coln_rec.status		,"D"); 
			coln_rec.q_order 	=   ToStdUom (SR._delivered);
			coln_rec.qty_del	=	ToStdUom (SR._delivered);    
			coln_rec.qty_ret	=	ToStdUom (SR._returned);

			if (coln_rec.bonus_flag[0] == 'Y')
			{	
				coln_rec.tax_pc  	= 0.00;
				coln_rec.gst_pc  	= 0.00;
				coln_rec.sale_price = 0.00;
				coln_rec.disc_pc 	= 0.00;
			}
			if (noTax)
			{
				coln_rec.tax_pc 	= 0.00;
				coln_rec.gst_pc 	= 0.00;
			}

			/*-------------------------------
			| Perform Invoice Calculations	|
			-------------------------------*/
			totalLine = coln_rec.q_order 
					* out_cost (coln_rec.sale_price, inmr_rec.outer_size);
			totalLine = no_dec (totalLine);

			if (noTax)
				totalTaxAmt = 0.00;
			else
			{
				totalTaxAmt = (double) coln_rec.q_order;
				totalTaxAmt *= out_cost (inmr_rec.tax_amount,inmr_rec.outer_size);
				totalTaxAmt = no_dec (totalTaxAmt);
			}

			totalDisc = (double) ScreenDisc (coln_rec.disc_pc);
			totalDisc = DOLLARS (totalDisc);
			totalDisc *= totalLine;
			totalDisc = no_dec (totalDisc);

			if (noTax)
				totalTax = 0.00;
			else
			{
				totalTax = (double) coln_rec.tax_pc;
				totalTax = DOLLARS (totalTax);

				if (cumr_rec.tax_code[0] == 'D')
					totalTax *= totalTaxAmt;
				else
				{
					if (envVar.dbNettUsed)
						totalTax *= (totalLine - totalDisc);
					else
						totalTax *= totalLine;
				}
				totalTax = no_dec (totalTax);
			}

			if (noTax)
				totalGst = 0.00;
			else
			{
				totalGst = (double) coln_rec.gst_pc;
				totalGst = DOLLARS (totalGst);
				if (envVar.dbNettUsed)
					totalGst *= ((totalLine - totalDisc) + totalTax);
				else
					totalGst *= (totalLine + totalTax);
			}

			coln_rec.gross    = totalLine;
			coln_rec.amt_disc = totalDisc;
			coln_rec.amt_tax  = totalTax;
			coln_rec.amt_gst  = totalGst;

			cohr_rec.gross += totalLine;
			cohr_rec.disc  += totalDisc;
			cohr_rec.tax   += totalTax;
			cohr_rec.gst   += totalGst;

			coln_rec.hhsl_hash	=	0L;

			if (coln_rec.qty_ret != 0.00)
				UpdateReturns (line_cnt); 

			cc = abc_update (coln, &coln_rec);
			if (cc)
				file_err (cc, (char *)coln, "DBUPDATE");

			abc_unlock (coln);

		}

		abc_unlock (coln);
	}

	strcpy (cohr_rec.status		,"D"); 
	strcpy (cohr_rec.type	, (INVOICE) ? "I" : "C"); 
	cc = abc_update (cohr, &cohr_rec);
	if (cc)
		file_err (cc, (char *)cohr, "DBUPDATE");

	abc_unlock (cohr);

	/*-------------------------------------------
	| Create a log file record for sales Order. |
	-------------------------------------------*/
	LogCustService 
	(
		cohr_rec.hhco_hash,
		0L,
		cumr_rec.hhcu_hash,
		cohr_rec.cus_ord_ref,
		cohr_rec.cons_no,
		cohr_rec.carr_code,
		cohr_rec.del_zone,
		LOG_DELIVERY
	);
}

void
UpdateReturns (
 int	line_cnt)
{
	double	returnGross	=	0.00,
			returnDisc	=	0.00,
			local_nett	=	0.00,
			local_amt	=	0.00,
			local_cost	=	0.00,
			cost1		=	0.00,
			cost2		=	0.00,
			wk_value	=	0.00;

	int		InvMonth	=	0,
			i			=	0,
			NoLots		=	TRUE;

	InvMonth = CalcMonth (cohr_rec.date_raised) - 1;

	returnGross		=	coln_rec.qty_ret * 
						out_cost (coln_rec.sale_price, inmr_rec.outer_size);
	returnGross 	= 	no_dec (returnGross);

	returnDisc = (double) ScreenDisc (coln_rec.disc_pc);
	returnDisc = DOLLARS (returnDisc);
	returnDisc *= returnGross;
	returnDisc = no_dec (returnDisc);

	local_amt	=	 (returnGross - returnDisc);
	local_cost	=	coln_rec.cost_price;

	/*-----------------------------
	| Calculate local nett value. |
	-----------------------------*/
	if (envVar.dbMcurr && cohr_rec.exch_rate != 0.00)
		local_nett = no_dec (local_amt / cohr_rec.exch_rate);
	else
		local_nett = (local_amt);

	inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (inmr);
		return;
	}
	
   	incc_rec.hhbr_hash = coln_rec.hhbr_hash;
   	incc_rec.hhcc_hash = coln_rec.incc_hash;
   	cc = find_rec (incc, &incc_rec, COMPARISON, "u");
   	if (cc)
	{
		abc_unlock (inmr);
		abc_unlock (incc);
		return;
	}

	ccmr_rec.hhcc_hash	=	coln_rec.incc_hash;
	cc = find_rec ("ccmr", &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		abc_unlock (inmr);
		abc_unlock (incc);
		return;
	}
	
	if (INVOICE)
	{
		inmr_rec.on_hand   += coln_rec.qty_ret;
		incc_rec.receipts  += coln_rec.qty_ret;
		incc_rec.closing_stock = incc_rec.opening_stock +
									incc_rec.pur +
									incc_rec.receipts +
									incc_rec.adj -
									incc_rec.issues -
									incc_rec.sales;
	}
	else
	{
		inmr_rec.on_hand   -= coln_rec.qty_ret;
		incc_rec.issues    += coln_rec.qty_ret;
		incc_rec.closing_stock = incc_rec.opening_stock +
									incc_rec.pur +
									incc_rec.receipts +
									incc_rec.adj -
									incc_rec.issues -
									incc_rec.sales;
	}

	cost1 = local_nett;
	wk_value = out_cost (local_cost, inmr_rec.outer_size);
	cost2 = (wk_value * coln_rec.qty_ret);
	if (INVOICE)
	{
		incc_prf [InvMonth]	-= (cost1 - cost2);
		incc_con [InvMonth]	-= coln_rec.qty_ret;
		incc_val [InvMonth]	-= local_nett;
	}
	else
	{
		incc_prf [InvMonth]	+= (cost1 - cost2);
		incc_con [InvMonth]	+= coln_rec.qty_ret;
		incc_val [InvMonth]	+= local_nett;
	}

	cc = abc_update (incc, &incc_rec);
	if (cc) 
		file_err (cc, (char *)incc, "DBUPDATE");

	cc = abc_update (inmr, &inmr_rec);
	if (cc) 
		file_err (cc, (char *)inmr, "DBUPDATE");

	/*--------------------------------------
	| Find Warehouse unit of measure file. |
	--------------------------------------*/
	inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash	=	coln_rec.hhum_hash;
	cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
	if (cc)
	{
		memset (&inwu_rec, 0, sizeof (inwu_rec));
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = abc_add (inwu, &inwu_rec);
		if (cc)
			file_err (cc, (char *)inwu, "DBADD");

		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	coln_rec.hhum_hash;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
			file_err (cc, (char *)inwu, "DBFIND");
	}
	if (INVOICE)
	{
		inwu_rec.receipts		+= 	coln_rec.qty_ret;
		inwu_rec.closing_stock 	= 	inwu_rec.opening_stock +
							 		inwu_rec.pur +
							 		inwu_rec.receipts +
							 		inwu_rec.adj -
							 		inwu_rec.issues -
							 		inwu_rec.sales;
	}
	else
	{
		inwu_rec.issues			+= 	coln_rec.qty_ret;
		inwu_rec.closing_stock 	= 	inwu_rec.opening_stock +
							 		inwu_rec.pur +
							 		inwu_rec.receipts +
							 		inwu_rec.adj -
							 		inwu_rec.issues -
							 		inwu_rec.sales;
	}
	
	cc = abc_update (inwu,&inwu_rec);
	if (cc)
		file_err (cc, "inwu", "DBUPDATE");

	if (MULT_LOC || SK_BATCH_CONT)
		UpdateLotLocation (line_cnt, (INVOICE) ? FALSE : TRUE);

	NoLots	=	TRUE;
	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (line_cnt, i))
			break;

		NoLots	=	FALSE;
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd 
		 (
			cohr_rec.co_no,  
			cohr_rec.br_no,
			ccmr_rec.cc_no,
			incc_rec.hhbr_hash, 
			incc_rec.hhcc_hash,
			SR._hhum_hash,
			local_rec.actualDelDate,
			(INVOICE) ? 2 : 3, 
			GetLotNo (line_cnt, i),
			inmr_rec.inmr_class, 		
			inmr_rec.category, 
			local_rec.RfNumber,
			cohr_rec.inv_no,
			GetBaseQty (line_cnt, i),
			local_nett / GetBaseQty (line_cnt, i),
			local_cost
		); 
	}
	if (NoLots)
	{
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd 
		 (
			cohr_rec.co_no,  
			cohr_rec.br_no,
			ccmr_rec.cc_no,
			incc_rec.hhbr_hash, 
			incc_rec.hhcc_hash,
			SR._hhum_hash,
			local_rec.actualDelDate,
			(INVOICE) ? 2 : 3, 
			" ",
			inmr_rec.inmr_class, 		
			inmr_rec.category, 
			local_rec.RfNumber,
			cohr_rec.inv_no,
			 (float) ToStdUom (coln_rec.qty_ret),
			local_nett / coln_rec.qty_ret,
			local_cost
		); 
	}
}

int
CheckReturns (
 void)
{
	float	totalQuantityReturn		=	0.00;
	float	totalQtyDelivered		=	0.00;

	scn_set (SCN_ITEMS);
	for (line_cnt	=	0;line_cnt < lcount[SCN_ITEMS];line_cnt++)
	{
		getval (line_cnt);
		totalQuantityReturn	+= local_rec.qtyReturn;
		totalQtyDelivered	+= local_rec.qtyDelivered;
	}

	if	 (twodec (totalQuantityReturn) != 0.00 &&
		 (!strcmp (local_rec.RfNumber, "          ") ||
		 local_rec.actualDelDate <= 0L))
	{
		print_mess (ML (mlTrMess016));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (twodec (totalQtyDelivered) != 0.00 && local_rec.actualDelDate <= 0L)
	{
		print_mess (ML (mlTrMess017));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

/*==========================
| Reverse Screen Discount. |
==========================*/
float	
ScreenDisc (
 float	DiscountPercent)
{
	if (envVar.reverseDiscount)
		return (DiscountPercent * -1);

	return (DiscountPercent);
}


float	
ToStdUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("itemUom")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR._cnv_fct;

	return (cnvQty);
}

float	
ToLclUom (
 float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("itemUom")))
		return (lclQty);

	if (SR._cnv_fct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR._cnv_fct;

	return (cnvQty);
}

void
tab_other (
 int line_no)
{
	if (cur_screen != SCN_ITEMS)
		return;

	/*-------------------------------------
	| Display the Second Item Description |
	-------------------------------------*/
	print_at (6,1, ML (mlStdMess250), store[line_no]._desc2); 
}

int	
CalcMonth (
 long	InvDate)
{
	int		monthPeriod;

	DateToDMY (InvDate, NULL, &monthPeriod, NULL);
	return (monthPeriod);
}

/*=====================================
| Check environment variables and     |
| set values in the envVar structure. |
=====================================*/
void
CheckEnvironment (
 void)
{
	char	*sptr;

	sptr = chk_env ("DB_MCURR");
	envVar.dbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("SO_DISC_REV");
	envVar.reverseDiscount = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*--------------------------------
	| Check if nett pricing is used. |
	--------------------------------*/
	sptr = chk_env ("DB_NETT_USED");
	envVar.dbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);
}
