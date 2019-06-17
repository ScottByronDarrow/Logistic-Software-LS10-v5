/*=====================================================================
|  Copyright (C) 1985 - 2000 LogisticSoftware Limited                 |
|=====================================================================|
|	$Id: tr_manage.c,v 5.8 2002/07/16 02:46:09 scott Exp $
|	Program Name : (tr_manage.c)								  	  |
|	Program Desc : (Transport Management Program.)      			  |
|---------------------------------------------------------------------|
|   Author        : Rommel Maldia   | Date Written  : 12/14/95        |
|---------------------------------------------------------------------|
|                :                                                    |
|	$Log: tr_manage.c,v $
|	Revision 5.8  2002/07/16 02:46:09  scott
|	Updated from service calls and general maintenance.
|	
|	Revision 5.7  2002/06/25 08:45:47  scott
|	Updated to add sleepTime
|	
|	Revision 5.6  2002/04/18 04:34:26  scott
|	Updated as status wrong causing view not to work;
|	
|	Revision 5.5  2002/01/09 03:29:16  robert
|	Updated to remove tab_display command in function heading2 as it is not
|	necessary and it makes cursor disapper during entry in LS10-GUI
|	
|	Revision 5.4  2001/10/25 07:54:33  scott
|	Updated to make changes related to container and seals.
|	
|	Revision 5.3  2001/08/23 11:30:12  scott
|	Updated from scotts machine
|	
|	Revision 5.2  2001/08/09 09:23:03  scott
|	Updated to add FinishProgram () function
|	
|	Revision 5.1  2001/08/06 23:53:51  scott
|	RELEASE 5.0
|	
|	Revision 5.0  2001/06/19 08:21:44  robert
|	LS10-5.0 New Release as of 19 JUNE 2001
|	
|	Revision 4.1  2001/05/30 08:38:33  scott
|	Updated ensure weight = four decimal and volume = 3 decimal places.
|	
|	Revision 4.0  2001/03/09 02:42:57  scott
|	LS10-4.0 New Release as at 10th March 2001
|	
|	Revision 3.4  2001/03/06 02:31:47  scott
|	Updated to for buttons on LS10-GUI
|	
|	Revision 3.3  2000/12/22 08:09:16  ramon
|	Updated to correct the errors when compiled in LS10-GUI.
|	
|	Revision 3.2  2000/12/19 00:03:07  scott
|	Updated to clean up and add container selection
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tr_manage.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_manage/tr_manage.c,v 5.8 2002/07/16 02:46:09 scott Exp $";

#define LCL_SCR_WIDTH	132

/*----------------------------------------------------
| Defines the offset values used for the tag screen. |
----------------------------------------------------*/
#define	OFF_INV_NO		0
#define	OFF_CUST_NO		13
#define	OFF_DEL_ADD		21
#define	OFF_ZONE_CODE	63
#define	OFF_DEL_CODE	71
#define	OFF_ASS_CODE	74
#define	OFF_WEIGHT		76
#define	OFF_VOLUME		86
#define	OFF_DEL_DATE	97
#define	OFF_ASM_DATE	109
#define	OFF_TIME_SLOTS	121
#define	OFF_STATUS		126
#define	OFF_HHCO_HASH	129
#define	OFF_HHIT_HASH	141
#define	OFF_TRAN_TYPE	152
/*
AAAAAAAAAAA..AAAAAA..AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA..AAAAAA..A..A.NNNNNNNNN.NNNNNNNNN..DDDDDDDDDD..DDDDDDDDDD..A-AA.A..NNNNNNNNNN..NNNNNNNNNN.A
*/

#define	NO_DELIVERIES (cohr_rec.asm_hash == 0L && cohr_rec.hhtr_hash == 0L)
#define	DEL_AND_ASM 	(cohr_rec.asm_hash != 0L && cohr_rec.hhtr_hash != 0L)
#define	DEL_DATE		(cohr_rec.del_date > 0L)
#define	ASM_DATE		(cohr_rec.asm_date > 0L)
#define	DEL_DONE		(cohr_rec.hhtr_hash != 0L)
#define	ASM_DONE		(cohr_rec.asm_hash  != 0L)
#define	ASM_REQ			(cohr_rec.asm_req [0] == 'Y')
#define	DEL_REQ			(cohr_rec.del_req [0] == 'Y')
#define	SAME_DATES		(cohr_rec.del_date == cohr_rec.asm_date || !ASM_REQ)

#include <ctype.h>
#include <pslscr.h>
#include <twodec.h>
#include <hot_keys.h>
#include <assert.h>
#include <pslscr.h>
/*#include <ttyctl.h>*/
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <tabdisp.h>

typedef	int	BOOL;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct trveRecord	trve_rec;
struct trveRecord	trve2_rec;
struct trlnRecord	trln_rec;
struct trhrRecord	trhr_rec;
struct trhrRecord	trhr2_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct ithrRecord	ithr_rec;
struct ithrRecord	ithr2_rec;
struct itlnRecord	itln_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct trzmRecord	trzm_rec;
struct extfRecord	extf_rec;
struct inuvRecord	inuv_rec;
struct sohrRecord	sohr_rec;
struct trshRecord	trsh_rec;
struct trztRecord	trzt_rec;
struct trzcRecord	trzc_rec;
struct skcmRecord	skcm_rec;
struct skniRecord	skni_rec;

char	*formatString = "%-11.11s  %-6.6s  %-40.40s  %-6.6s  %1.1s  %1.1s %9.4f %9.3f  %-10.10s  %10.10s  %1.1s-%1.1s%1.1s %-1.1s  %010ld  %010ld %1.1s", 
		*transportFile =  "transportFile", 
		*cohr2	=	"cohr2", 
		*data	=	"data", 
		*ithr2	=	"ithr2", 
		*trhr2	=	"trhr2", 
		*trve2	=	"trve2";

int		noInTab = 0, 
		Screen2 = FALSE, 
		DStat	= FALSE;

FILE	*fout, 
		*fsort;

	int		envVarSoFrChg;
	int		overloadWeight		=	0, 
			overloadVolume		=	0;
	char	deliveryDate [11], 
			assemblyDate [11], 
			deliveryFlag [2], 
			assemblyFlag [2], 
			startTimeSlot [2], 
			endTimeSlot [2], 
			scheduleFlag [2], 
			bufferData [256];

static	int	AlloFunc	(int, KEY_TAB *);
static	int	RemoveFunc	(int, KEY_TAB *);
static	int	ViewFunc	(int, KEY_TAB *);
static	int	TimeSlotFunc (int, KEY_TAB *);
static	int	RestartFunc	(int, KEY_TAB *);
static	int	ExitFunc	(int, KEY_TAB *);
static	int	MoveFunc	(int, KEY_TAB *);

#ifdef	GVISION
	static KEY_TAB list1_keys [] =
	{
	   { " Allocate ", 		'A', 	AlloFunc, 
		"Allocate Document to Vehicle", 		"A" }, 
	   { " Remove ", 		'R', 	RemoveFunc, 
		"Remove Document From Vehicle ", 		"A" }, 
	   { " Move ", 	 		'M', 	MoveFunc, 
		"Move Document To Another Vehicle", 		"A" }, 
	   { " View ", 			'V', 	ViewFunc, 
		"View Document", 						"A" }, 
	   { " Schedules ", 	'S', 	TimeSlotFunc, 
		"View Scheduled Delivery times", 						"A" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};

	static KEY_TAB list2_keys [] =
	{
	   { " Remove ", 		'R', 	RemoveFunc, 
		"Remove Document From Vehicle ", 		"A" }, 
	   { " Move ", 	 		'M', 	MoveFunc, 
		"Move Document To Another Vehicle", 		"A" }, 
	   { " View ", 			'V', 	ViewFunc, 
		"View Document", 						"A" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};
#else
	static KEY_TAB list1_keys [] =
	{
	   { " [A]llocate ", 		'A', 	AlloFunc, 
		"Allocate Document to Vehicle", 		"A" }, 
	   { " [R]emove ", 		'R', 	RemoveFunc, 
		"Remove Document From Vehicle ", 		"A" }, 
	   { " [M]ove ", 	 		'M', 	MoveFunc, 
		"Move Document To Another Vehicle", 		"A" }, 
	   { " [V]iew ", 			'V', 	ViewFunc, 
		"View Document", 						"A" }, 
	   { " [S]chedules ", 	'S', 	TimeSlotFunc, 
		"View Scheduled Delivery times", 						"A" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};

	static KEY_TAB list2_keys [] =
	{
	   { " [R]emove ", 		'R', 	RemoveFunc, 
		"Remove Document From Vehicle ", 		"A" }, 
	   { " [M]ove ", 	 		'M', 	MoveFunc, 
		"Move Document To Another Vehicle", 		"A" }, 
	   { " [V]iew ", 			'V', 	ViewFunc, 
		"View Document", 						"A" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};
#endif

struct
{
	float	weight;
	float	volume;
	long	hhve_hash;
	long	del_date;
	long	del_date2;
	long	firstdate;
	long	lastdate;
	char	option [2];
	float	capacityWeight;
	float	capacityVolume;
	double	vehicleFreight;
	double	zoneFreight;
	char	tripNumber [13];
	char	trip2Number [13];
	double	vehicleFreight2;
	double	zoneFreight2;
	long	lsystemDate;
	char	ref [11];
	char	ref2 [11];
	char	desc [41];
	char	desc2 [41];
	char	dummy [11];
	char	driver [7];
	char	documentNumber [9];
	char	deliveryZone [7];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "vehicle", 	 2, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "  ", "Vehicle Reference     : ", "Enter Vehicle No. or [SEARCH]", 
		YES, NO,  JUSTLEFT, "", "", local_rec.ref}, 
	{1, LIN, "vehi_desc", 	 2, 42, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc}, 
	{1, LIN, "capacity_weight", 	 2, 94, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "  ", "Capacity Weight       : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.capacityWeight}, 
	{1, LIN, "capacity_volume", 	 3, 94, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "  ", "Capacity Volume       : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.capacityVolume}, 
	{1, LIN, "weight", 	 4, 94, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "  ", "Weight Allocated      : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.weight}, 
	{1, LIN, "volume", 	 5, 94, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "  ", "Volume Allocated      : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.volume}, 
	{1, LIN, "del_date", 	3, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "Expected Del Date     : ", " Enter Expected Date or [Search] . Default is Date today.", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.del_date}, 

	{1, LIN, "tripNumber", 	 3, 46, CHARTYPE, 
		"UUUUUUUUUUUU", "          ", 
		" ", "  ", "Trip Number           : ", "Enter Trip No.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.tripNumber}, 
	{1, LIN, "option", 	 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N",  "Option (N/A)          : ", "Enter [N] for Not Allocated and [A] for Allocated.", 
		YES, NO,  JUSTRIGHT, "NA", "", local_rec.option}, 
	{1, LIN, "driver", 	 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "  ", "Driver/Trucker        : ", "Enter Driver or Trucker.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.driver}, 
	{1, LIN, "containerCode", 5, 46, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ",  "Container No.         : ", "Enter Container No. [SEARCH]", 
		NO, NO, JUSTLEFT, "", "", skcm_rec.container}, 
	{1, LIN, "docNumber", 6, 2, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"", "",  "Document Number       : ", "Enter packing slip, Collection Note or Transfer  number <default all> ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.documentNumber}, 
	{1, LIN, "deliveryZone", 	 6, 46, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Delivery Zone         : ", "Enter Delivery Zone [SEARCH].", 
		YES, NO,  JUSTLEFT, "", "", local_rec.deliveryZone}, 
	{1, LIN, "firstdate", 	7,  2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "First Required Date   : ", " Enter Start of Required Date.", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.firstdate}, 
	{1, LIN, "lastdate", 	7, 46, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Last  Required Date   : ", " Enter End Required Date.", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.lastdate}, 
	{1, LIN, "zoneFreight", 	 6, 94, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amount (Zone) : ", "This is the amount for each trip with a zone.", 
		YES, NO, JUSTRIGHT, "0.00", "999999999.99", (char *)&local_rec.zoneFreight}, 
	{1, LIN, "vehicleFreight", 	 7, 94, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amt.(Vehicle) : ", "This is the amount of freight per kg. ", 
		YES, NO, JUSTRIGHT, "0.00", "999999999.99", (char *)&local_rec.vehicleFreight}, 
	{2, LIN, "vehicle2", 	 12, 32, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "  ", "Vehicle Reference     : ", "Enter Vehicle No. [SEARCH]", 
		YES, NO,  JUSTLEFT, "", "", local_rec.ref2}, 
	{2, LIN, "vehi_desc2", 	 13, 32, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "  ", "Vehicle Description   : ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2}, 
	{2, LIN, "del_date2", 	14, 32, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "Expected Delivery Date: ", " Default is Date today.", 
		 YES, NO,  JUSTLEFT, " ", "", (char *)&local_rec.del_date2}, 

	{2, LIN, "move_to", 	 16, 32, CHARTYPE, 
		"UUUUUUUUUUUU", "          ", 
		" ", "  ", "Trip Number           : ", "Enter Trip No.", 
		YES, NO,  JUSTLEFT, "", "", local_rec.trip2Number}, 
	{2, LIN, "vehicleFreight2", 	 17, 32, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amt. (Vehicle) : ", "This is the amount of freight per kg. ", 
		YES, NO, JUSTRIGHT, "0.00", "999999999.99", (char *)&local_rec.vehicleFreight2}, 
	{2, LIN, "zoneFreight2", 	 18, 32, MONEYTYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amount (Zone) : ", "This is the amount for each trip with a zone.", 
		YES, NO, JUSTRIGHT, "0.00", "999999999.99", (char *)&local_rec.zoneFreight2}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*=====================================================
| The structure capacity groups the values together.  |
=====================================================*/
struct tagCapacityStructure
{
	float	volume;
	float	volumeLine;
	float	weightItems;
	float	weightLine;
	float	weightHeader;
	float	maxHeight;
	float	maxWidth;
} capacity;

extern	int	TruePosition;


#include	<tr_schedule.h>
/*=======================
| Function Declarations |
=======================*/
int 	LoadAllocated 				(void);
int 	LoadConts 					(void);
int 	MovePSlip 					(int);
int 	PopUp 						(int);
int 	TestTag 					(int, char *);
int 	heading 					(int);
int 	spec_valid 					(int field);
int		CheckContainer 				(long);
static 	BOOL IsYesResponse 			(char *);
static 	char *GetNewTripNumber 		(char *);
static 	int CreateTrip 				(int);
static 	int GetAvailableDeliveries 	(long, long, char *);
void 	AllocateTrip 				(long, long, long, long, char *);
void 	CalculateCapacity 			(char *, long, long);
void 	ClearCapacity 				(void);
void 	CloseDB 					(void);
void 	LoadCohrNotAllocated 		(char *);
void 	LoadIthrNotAllocated 		(void);
void 	OpenDB 						(void);
void 	Process 					(void);
void 	RemoveTrip 					(long, long, long, long, char *);
void 	ShowDocumentNumber 			(char *);
void 	SrchCohr 					(char *, char *, char *);
void 	SrchDelDate 				(char *);
void 	SrchExtf 					(char *);
void 	SrchIthr 					(char *);
void 	SrchSkcm 					(char *);
void 	SrchTrhr 					(char *);
void 	SrchTrve 					(char *);
void 	SrchTrzm 					(char *);
void 	TagLine 					(int, char *);
void 	Update 						(void);
void 	UpdateTrhr 					(char *);
void 	heading2 					(void);
void 	shutdown_prog 				(void);

/*============================
|	Main Processing Routine  |
============================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;

	sptr = chk_env ("SO_FR_CHG");
	envVarSoFrChg = (sptr == (char *)0) ? 0 : atoi (sptr);

	SETUP_SCR 	(vars);
	init_scr 	();
	set_tty 	();
	set_masks 	();
	
	OpenDB		();
	clear		();

	FLD ("containerCode") 	= NO;

	local_rec.lsystemDate = TodaysDate ();
	
	while (prog_exit == 0)
	{
		search_ok 		= 	TRUE;
		entry_exit 		= 	FALSE;
		edit_exit 		= 	FALSE;
		prog_exit 		= 	FALSE;
		restart 		= 	FALSE;
		init_ok 		= 	TRUE;
		overloadWeight	=	0;
		overloadVolume	=	0;
		init_vars 	(1);	
		heading 	(1);
		entry 		(1);   
		if (!restart && !prog_exit)
		{
			heading (1);
			scn_display (1);
			Process ();
		}

		else if (restart)
			continue;
		else
			prog_exit = TRUE;
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
	CloseDB	();
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	abc_alias (trve2, trve);
	abc_alias (trhr2, trhr);
	abc_alias (cohr2, cohr);
	abc_alias (ithr2, ithr);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_hhtr_hash"); 
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2"); 
	open_rec (coln,  coln_list, COLN_NO_FIELDS, 	"coln_id_no"); 
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash"); 
	open_rec (extf,  extf_list, EXTF_NO_FIELDS, "extf_id_no"); 
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, 	"inmr_hhbr_hash"); 
	open_rec (inuv,  inuv_list, INUV_NO_FIELDS, "inuv_id_no"); 
	open_rec (ithr,  ithr_list, ITHR_NO_FIELDS, "ithr_hhtr_hash"); 
	open_rec (ithr2, ithr_list, ITHR_NO_FIELDS, "ithr_id_no"); 
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_id_no"); 
	open_rec (trhr,  trhr_list, TRHR_NO_FIELDS, "trhr_id_no"); 
	open_rec (trhr2, trhr_list, TRHR_NO_FIELDS, "trhr_hhtr_hash"); 
	open_rec (trln,  trln_list, TRLN_NO_FIELDS, "trln_id_no"); 
	open_rec (trsh,  trsh_list, TRSH_NO_FIELDS, "trsh_hhco_hash"); 
	open_rec (trve,  trve_list, TRVE_NO_FIELDS, "trve_id_no"); 
	open_rec (trve2, trve_list, TRVE_NO_FIELDS, "trve_id_no"); 
	open_rec (trzm,  trzm_list, TRZM_NO_FIELDS, "trzm_id_no"); 
	open_rec (skcm,  skcm_list, SKCM_NO_FIELDS, "skcm_id_no"); 
	open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhcl_hash"); 
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cohr);
	abc_fclose (cohr2);
	abc_fclose (coln);
	abc_fclose (cumr);
	abc_fclose (extf);
	abc_fclose (inmr);
	abc_fclose (inuv);
	abc_fclose (ithr);
	abc_fclose (ithr2);
	abc_fclose (itln);
	abc_fclose (trhr);
	abc_fclose (trhr2);
	abc_fclose (trln);
	abc_fclose (trsh);
	abc_fclose (trve);
	abc_fclose (trve2);
	abc_fclose (trzm);
	abc_fclose (skcm);
	abc_fclose (skni);
	abc_dbclose (data);
}

/*===================
|  Field Validation |
===================*/
int
spec_valid (
 int field)
{
	if (LCHECK ("vehicle"))
	{
		if (dflt_used) 
		{
			errmess (ML (mlTrMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (SRCH_KEY)
		{
			SrchTrve (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trve_rec.co_no, 	comm_rec.co_no);
		strcpy (trve_rec.br_no, 	comm_rec.est_no);
		strcpy (trve_rec.ref, 		local_rec.ref);
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlTrMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.avail [0] == 'N')
		{
			sprintf (err_str, ML (mlTrMess060), trve_rec.unav_res);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.desc, trve_rec.desc);
		local_rec.hhve_hash			=	trve_rec.hhve_hash;
		local_rec.capacityWeight 	= 	trve_rec.max_wgt;
		local_rec.capacityVolume 	= 	trve_rec.max_vol;
		local_rec.vehicleFreight 	= 	trve_rec.fr_chg;
		DSP_FLD ("vehi_desc");
		DSP_FLD ("capacity_weight");
		DSP_FLD ("capacity_volume");
		DSP_FLD ("vehicleFreight");
		if (prog_status == ENTRY)
			strcpy (local_rec.tripNumber, "            ");

		return (EXIT_SUCCESS);
	}

	/*----------------------- 
	| Validate Date Entered	| 
	-----------------------*/
	if (LCHECK ("del_date"))
	{
		if (dflt_used) 
			local_rec.del_date = local_rec.lsystemDate;

		if (local_rec.del_date > local_rec.lsystemDate)
		{
			errmess (ML (mlTrMess034));
			sleep	(sleepTime);
			return 	(1);
		}

		if (SRCH_KEY)
		{
			SrchDelDate (temp_str);
			strcpy (local_rec.driver, 	trhr_rec.driver);
			strcpy (local_rec.tripNumber, 	trhr_rec.trip_name);
			local_rec.del_date 			= 	trhr_rec.del_date;
			local_rec.vehicleFreight	=	trhr_rec.fr_chg;
			local_rec.zoneFreight		=	trhr_rec.fr_zchg;
			if (strcmp (local_rec.tripNumber, "            "))
			{
				DSP_FLD ("tripNumber");
				DSP_FLD ("driver");
				DSP_FLD ("vehicleFreight");
				FLD ("driver") 	= NA;
				skip_entry 		= 1;
				return (EXIT_SUCCESS);
			}
			else
				return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------
	| Validate Trip Number. |
	-----------------------*/
	if (LCHECK ("tripNumber"))
	{
		int		copy_last 	= 	0;

		if (F_NOKEY (field))
		{
			FLD ("tripNumber") = YES;
			return (EXIT_SUCCESS);
		}

		FLD ("tripNumber") = YES;

		if (SRCH_KEY)
		{
			SrchTrhr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used) 
		{
			strcpy (trhr_rec.trip_name, "            ");
			strcpy (trhr_rec.driver, 	"      ");
			strcpy (trhr_rec.status , 	" ");
			trhr_rec.hhve_hash 	= 	trve_rec.hhve_hash;
			trhr_rec.del_date 	= 	local_rec.del_date;
			trhr_rec.act_date 	= 	TodaysDate ();
			strcpy (err_str, TimeHHMMSS ());
			trhr_rec.act_time 	= 	atot (err_str);

			cc = CreateTrip (1);
			if (cc)
				file_err (cc, "CreateTrip", "DBFunction");

			if (trhr_rec.status [0] == 'D')
				DStat = TRUE;
			else
				DStat = FALSE;

			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.tripNumber, "            "))
		{
			abc_selfield (trhr, "trhr_trip_name");
			
			strcpy (trhr_rec.trip_name, local_rec.tripNumber);
			cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
			if (!cc && !strcmp (trhr_rec.trip_name, local_rec.tripNumber))
			{
				strcpy (local_rec.driver, trhr_rec.driver);
				local_rec.vehicleFreight	= trhr_rec.fr_chg;
				local_rec.zoneFreight		= trhr_rec.fr_zchg;
				DStat = FALSE;
				cohr_rec.hhtr_hash = trhr_rec.hhtr_hash; 
				cc = find_rec (cohr, &cohr_rec, GTEQ, "r"); 
				while (!cc)
				{
					if (cohr_rec.hhtr_hash == trhr_rec.hhtr_hash  &&
					     cohr_rec.status [0] == 'D')
					{
						DStat = TRUE;
						break;
					}
					cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				} 
				ithr_rec.hhtr_hash = trhr_rec.hhtr_hash; 
				cc = find_rec (ithr, &ithr_rec, GTEQ, "r"); 
				while (!cc)
				{
					if (ithr_rec.hhtr_hash == trhr_rec.hhtr_hash  &&
					     ithr_rec.type [0] == 'D')
					{
						DStat = TRUE;
						break;
					}
					cc = find_rec (ithr, &ithr_rec, NEXT, "r");
				} 

				copy_last = (IsYesResponse (ML (mlTrMess057)));
					
				if (copy_last && DStat && strcmp (trhr_rec.driver, "      "))
					FLD ("driver") = NA;

				if (!copy_last)
				{
					clear_mess ();

					last_char = prmptmsg (ML (mlTrMess019), "YyNn", 1, 23);
					if (last_char =='Y' || last_char =='y')
					{

						strcpy (trhr_rec.trip_name , "            ");
						trhr_rec.hhve_hash 	= 	trve_rec.hhve_hash;
						trhr_rec.del_date 	= 	local_rec.del_date;
						strcpy (trhr_rec.driver, 	"      ");
						trhr_rec.act_date 	= 	TodaysDate ();
						strcpy (err_str, TimeHHMMSS ());
						trhr_rec.act_time 	= 	atot (err_str);
						strcpy (trhr_rec.status , " ");

						cc = CreateTrip (1);
						if (cc)
							file_err (cc, "CreateTrip", "DBFunction");
						else 
							DSP_FLD ("tripNumber");

						if (trhr_rec.status [0] == 'D')
							DStat = TRUE;
						else
							DStat = FALSE;
					}
					else
						return (EXIT_FAILURE);
				}
			}

			abc_selfield (trhr, "trhr_id_no");
		}

		abc_selfield (trhr, "trhr_id_no");
		DSP_FLD ("driver");
		DSP_FLD ("vehicleFreight");
		return (EXIT_SUCCESS);
	}


	/*----------------------+
	|  Validate Vehicle2    |
	-----------------------*/
	if (LCHECK ("vehicle2"))
	{
		if (dflt_used) 
		{
			errmess (ML (mlTrMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (SRCH_KEY)
		{
			SrchTrve (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trve2_rec.co_no, comm_rec.co_no);
		strcpy (trve2_rec.br_no, comm_rec.est_no);
		strcpy (trve2_rec.ref, 	 local_rec.ref2);
		cc = find_rec (trve2, &trve2_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlTrMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.desc2, trve2_rec.desc);
		local_rec.vehicleFreight2	=	trve2_rec.fr_chg;
		DSP_FLD ("vehi_desc2");
		DSP_FLD ("vehicleFreight2");
		return (EXIT_SUCCESS);
	}


	/*----------------------- 
	| Validate Date Entered	| 
	-----------------------*/
	if (LCHECK ("del_date2"))
	{
		if (dflt_used) 
			local_rec.del_date2 = local_rec.lsystemDate;

		if (local_rec.del_date2 > local_rec.lsystemDate)
		{
			errmess (ML (mlTrMess034));
			sleep	(sleepTime);
			return 	(1);
		}

		if (!strcmp (local_rec.ref2 , local_rec.ref)    &&
			 local_rec.del_date == local_rec.del_date2  &&
			 !strcmp (local_rec.tripNumber , local_rec.trip2Number))
		{
			errmess (ML (mlTrMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("del_date2");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Trip Number 2. |
	-------------------------*/
	if (LCHECK ("move_to"))
	{
		if (F_NOKEY (field))
		{
			FLD ("move_to") = YES;
			return (EXIT_SUCCESS);
		}

		FLD ("move_to") = YES;

		if (dflt_used) 
		{
			strcpy (trhr_rec.trip_name , "            ");
			trhr_rec.hhve_hash 	= 	trve2_rec.hhve_hash;
			trhr_rec.del_date 	= 	local_rec.del_date2;
			strcpy (trhr_rec.driver, 	"      ");
			trhr_rec.act_date 	= 	TodaysDate ();
			strcpy (err_str, TimeHHMMSS ());
			trhr_rec.act_time 	= 	atot (err_str);
			strcpy (trhr_rec.status , " ");

			cc = CreateTrip (2);
			if (cc)
				file_err (cc, "CreateTrip", "DBFunction");
			return (EXIT_SUCCESS);
		}

		if (strcmp (local_rec.trip2Number, " "))
		{
			if (!strcmp (local_rec.ref2 , local_rec.ref) &&
				local_rec.del_date == local_rec.del_date2 &&
				!strcmp (local_rec.tripNumber , local_rec.trip2Number))
			{
				errmess (ML (mlTrMess020));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			else
				strcpy (trhr_rec.trip_name, local_rec.tripNumber);

			abc_selfield (trhr, "trhr_trip_name");

			strcpy (trhr_rec.trip_name, local_rec.trip2Number);
			cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
			if (!cc  && !strcmp (trhr_rec.trip_name, local_rec.trip2Number))
			{
				if (trhr_rec.status [0] != 'D')
				{
					last_char = prmptmsg (ML (mlTrMess021), "YyNn", 1, 23);
					if (last_char =='Y' || last_char =='y')
					{
						strcpy (trhr_rec.trip_name , "            ");
						trhr_rec.hhve_hash 		= 	trve2_rec.hhve_hash;
						trhr_rec.del_date 		= 	local_rec.del_date2;
						strcpy (trhr_rec.driver, 	"      ");
						trhr_rec.act_date 		= 	TodaysDate ();
						strcpy (err_str, TimeHHMMSS ());
						trhr_rec.act_time 	= 	atot (err_str);
						strcpy (trhr_rec.status , " ");

						cc = CreateTrip (2);
						if (cc)
							file_err (cc, "CreateTrip", "DBFunction");
						else
							DSP_FLD ("move_to");
					}

					abc_selfield (trhr, "trhr_id_no");
				}
				else
				{
					errmess (ML (mlTrMess022));
					sleep (sleepTime);
					clear_mess ();
					abc_selfield (trhr, "trhr_id_no");
					return (EXIT_FAILURE);
				}
			}
		}
		return (EXIT_SUCCESS);
	}

	/*---------------------
	|  Validate Option    |
	---------------------*/
	if (LCHECK ("option"))
	{
		FLD ("firstdate") 		= NO;
		FLD ("lastdate") 		= NO;
		FLD ("deliveryZone")	= NO;
		FLD ("zoneFreight")		= NO;
		FLD ("docNumber")		= NO;
		FLD ("vehicleFreight") 	= NO;
		FLD ("driver") 			= NO;
		FLD ("containerCode") 	= NO;
		if (local_rec.option [0] == 'A')
		{
			FLD ("firstdate") 		= NA;
			FLD ("lastdate") 		= NA;
			FLD ("deliveryZone")	= NA;
			FLD ("zoneFreight")		= NA;
			FLD ("docNumber")		= NA;
			FLD ("vehicleFreight") 	= NA;
			FLD ("driver") 			= NA;
			FLD ("containerCode") 	= NA;
			DSP_FLD ("firstdate");
			DSP_FLD ("lastdate");
			DSP_FLD ("deliveryZone"); 
			DSP_FLD ("docNumber"); 
			DSP_FLD ("vehicleFreight");
			DSP_FLD ("driver");
			DSP_FLD ("containerCode");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate first document date. |
	-------------------------------*/
	if (LCHECK ("firstdate"))
	{
		if (dflt_used) 
			local_rec.firstdate = 0L;

		if (local_rec.firstdate > local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess068));
			sleep	(sleepTime);
			return 	(1);
		}

		DSP_FLD ("firstdate");
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Last document date. |
	------------------------------*/
	if (LCHECK ("lastdate"))
	{
		if (dflt_used) 
			local_rec.lastdate = local_rec.lsystemDate;

		if (local_rec.lastdate < local_rec.firstdate)
		{
			print_mess (ML (mlStdMess026));
			sleep 	(sleepTime);
			return 	(1);
		}
		
		if (local_rec.lastdate > local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess013));
			sleep	(sleepTime);
			return 	(1);
		}

		DSP_FLD ("lastdate");
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Delivery Zone. |
	-------------------------*/
	if (LCHECK ("deliveryZone"))
	{
		if (FLD ("deliveryZone") == NA)
		{
			FLD ("zoneFreight")		= NA;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (trzm_rec.del_zone, "      ");
			strcpy (local_rec.deliveryZone, "      ");
			trzm_rec.trzm_hash		=	0L;
			local_rec.zoneFreight 	= 	0.00;
			FLD ("zoneFreight")		= 	NA;
			DSP_FLD ("zoneFreight");
			return (EXIT_SUCCESS);
		}
		else
			FLD ("zoneFreight")		= 	NO;

		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.est_no);
		strcpy (trzm_rec.del_zone, local_rec.deliveryZone);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		local_rec.zoneFreight 			= 	trzm_rec.dflt_chg;
		DSP_FLD ("zoneFreight");
		DSP_FLD ("deliveryZone");

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("containerCode"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTrMess083));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
       	DSP_FLD ("containerCode");
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate pack Slip Number.	|
	-------------------------------*/
	if (LCHECK ("docNumber")) 
	{
		if (FLD ("docNumber") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.documentNumber, "        ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			ShowDocumentNumber (temp_str);
			return (EXIT_SUCCESS);
		}

		/*---------------------------------------------
		| Check if Transport Picking slip is on file. |
		---------------------------------------------*/
		strcpy (cohr2_rec.co_no, comm_rec.co_no);
		strcpy (cohr2_rec.br_no, comm_rec.est_no);
		strcpy (cohr2_rec.type, "T");
		sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
		cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		if (cc) 
		{
			/*-----------------------------------
			| Check if Picking slip is on file. |
			-----------------------------------*/
			strcpy (cohr2_rec.co_no, comm_rec.co_no);
			strcpy (cohr2_rec.br_no, comm_rec.est_no);
			strcpy (cohr2_rec.type, "P");
			sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
			cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		}
		if (cc) 
		{
			/*--------------------------------------
			| Check if Collection note is on file. |
			--------------------------------------*/
			strcpy (cohr2_rec.co_no, comm_rec.co_no);
			strcpy (cohr2_rec.br_no, comm_rec.est_no);
			strcpy (cohr2_rec.type, "N");
			sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
			cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-------------------------------------
			| Check if Stock Transfer is on file. |
			-------------------------------------*/
			strcpy (ithr2_rec.co_no, comm_rec.co_no);
			strcpy (ithr2_rec.type,  "T");
			ithr2_rec.del_no	=	atol (local_rec.documentNumber);
			cc = find_rec (ithr2, &ithr_rec, COMPARISON, "r");
		}
		/*---------------------
		| No documents found. |
		---------------------*/
		if (cc)
		{
			errmess (ML (mlTrMess035));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}
	/*------------------
	| Validate Driver. |
	------------------*/
	if (LCHECK ("driver"))
	{
		if (FLD ("driver") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.driver, "      ");
			DSP_FLD ("driver");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExtf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (extf_rec.co_no, comm_rec.co_no);
		strcpy (extf_rec.code, local_rec.driver);
		cc = find_rec (extf, &extf_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlTrMess005)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		} 

		DSP_FLD ("driver");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Freight Cost. |
	------------------------*/
	if (LCHECK ("vehicleFreight"))
	{
		if (dflt_used)
		{
			if (trhr_rec.fr_chg != 0.00)
				local_rec.vehicleFreight 	= trhr_rec.fr_chg;
			else
				local_rec.vehicleFreight 	= trve_rec.fr_chg;
		}
		else
			trve_rec.fr_chg 	= local_rec.vehicleFreight;

		DSP_FLD ("vehicleFreight");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Clear capacity fields. |
========================*/
void
ClearCapacity (
 void)
{
	capacity.volume			=	0.00;
	capacity.volumeLine		=	0.00;
	capacity.weightItems	=	0.00;
	capacity.weightLine		=	0.00;
	capacity.weightHeader	=	0.00;
	capacity.maxHeight		=	0.00;
	capacity.maxWidth		=	0.00;
}

/*============================
| Calculate capacity fields. |
============================*/
void
CalculateCapacity (
	char	*tripNumber, 
	long	hhcoHash, 
	long	hhitHash)
{
	capacity.volumeLine	=	0.00;
	capacity.weightLine	=	0.00;

	if (tripNumber == (char *) 0)
	{
		if (hhcoHash)
		{
			coln_rec.hhco_hash	=	hhcoHash;
			coln_rec.line_no	=	0;
			cc = find_rec (coln, &coln_rec, GTEQ, "r");
			while (!cc && hhcoHash == coln_rec.hhco_hash)
			{
				inuv_rec.hhbr_hash	=	coln_rec.hhbr_hash;
				inuv_rec.hhum_hash	=	coln_rec.hhum_hash;
				cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
				if (!cc)
				{
					if (capacity.maxHeight < inuv_rec.height) 
						capacity.maxHeight =	inuv_rec.height;

					if (capacity.maxWidth < inuv_rec.width) 
						capacity.maxWidth =	inuv_rec.width;

					capacity.volume			+=	inuv_rec.volume;
					capacity.weightItems	+=	inuv_rec.weight;
				}
				cc = find_rec (coln, &coln_rec, NEXT, "r");
			}
			capacity.weightHeader	+=	cohr_rec.no_kgs;
			if (capacity.weightHeader > capacity.weightItems)
				capacity.weightLine	=	capacity.weightHeader;
			else
				capacity.weightLine	=	capacity.weightItems;

			capacity.volumeLine	=	capacity.volume;
		}
		else if (hhitHash)
		{
			itln_rec.hhit_hash	=	hhcoHash;
			itln_rec.line_no	=	0;
			cc = find_rec (itln, &itln_rec, GTEQ, "r");
			while (!cc && hhitHash == itln_rec.hhit_hash)
			{
				inuv_rec.hhbr_hash	=	itln_rec.hhbr_hash;
				inuv_rec.hhum_hash	=	itln_rec.hhum_hash;
				cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
				if (!cc)
				{
					if (capacity.maxHeight < inuv_rec.height) 
						capacity.maxHeight =	inuv_rec.height;

					if (capacity.maxWidth < inuv_rec.width) 
						capacity.maxWidth =	inuv_rec.width;

					capacity.volume			+=	inuv_rec.volume;
					capacity.weightItems	+=	inuv_rec.weight;
				}
				cc = find_rec (itln, &itln_rec, NEXT, "r");
			}
			capacity.weightLine	=	capacity.weightItems;
			capacity.volumeLine	=	capacity.volume;
		}
	}
	else
	{
		abc_selfield (trhr, "trhr_trip_name");
		strcpy (trhr_rec.trip_name, local_rec.trip2Number);
		cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
		if (!cc)
		{
			cohr_rec.hhtr_hash = trhr_rec.hhtr_hash;
			cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
			while (!cc && cohr_rec.hhtr_hash == trhr_rec.hhtr_hash)
			{
				coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
				coln_rec.line_no	=	0;
				cc = find_rec (coln, &coln_rec, GTEQ, "r");
				while (!cc && cohr_rec.hhco_hash == coln_rec.hhco_hash)
				{
					inuv_rec.hhbr_hash	=	coln_rec.hhbr_hash;
					inuv_rec.hhum_hash	=	coln_rec.hhum_hash;
					cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
					if (!cc)
					{
						if (capacity.maxHeight < inuv_rec.height) 
							capacity.maxHeight =	inuv_rec.height;
		
						if (capacity.maxWidth < inuv_rec.width) 
							capacity.maxWidth =	inuv_rec.width;
		
						capacity.volume			+=	inuv_rec.volume;
						capacity.weightItems	+=	inuv_rec.weight;
					}
					cc = find_rec (coln, &coln_rec, NEXT, "r");
				}
				capacity.weightHeader	+=	cohr_rec.no_kgs;
		
				if (capacity.weightHeader > capacity.weightItems)
					capacity.weightLine	=	capacity.weightHeader;
				else
					capacity.weightLine	=	capacity.weightItems;

				capacity.volumeLine	=	capacity.volume;
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			}
			ithr_rec.hhtr_hash = trhr_rec.hhtr_hash;
			cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
			while (!cc && ithr_rec.hhtr_hash == trhr_rec.hhtr_hash)
			{
				itln_rec.hhit_hash	=	ithr_rec.hhit_hash;
				itln_rec.line_no	=	0;
				cc = find_rec (itln, &itln_rec, GTEQ, "r");
				while (!cc && ithr_rec.hhit_hash == itln_rec.hhit_hash)
				{
					inuv_rec.hhbr_hash	=	itln_rec.hhbr_hash;
					inuv_rec.hhum_hash	=	itln_rec.hhum_hash;
					cc = find_rec (inuv, &inuv_rec, COMPARISON, "r");
					if (!cc)
					{
						if (capacity.maxHeight < inuv_rec.height) 
							capacity.maxHeight =	inuv_rec.height;
		
						if (capacity.maxWidth < inuv_rec.width) 
							capacity.maxWidth =	inuv_rec.width;
		
						capacity.volume			+=	inuv_rec.volume;
						capacity.weightItems	+=	inuv_rec.weight;
					}
					cc = find_rec (itln, &itln_rec, NEXT, "r");
				}
				capacity.volumeLine	=	capacity.volume;
				capacity.weightLine	=	capacity.weightItems;
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			}
		}
		abc_selfield (trhr, "trhr_id_no");
	}
	
	return;
}

void
Process (
 void)
{
	/*------------
	| Open table |
	------------*/
	if (local_rec.option [0] == 'A')
		tab_open (transportFile, list2_keys, 8, 0, 9, FALSE);
	else
		tab_open (transportFile, list1_keys, 8, 0, 9, FALSE);

	tab_add (transportFile, 
			"#%-11.11s  %-6.6s  %-40.40s  %-6.6s  %1.1s  %1.1s  %8.8s  %8.8s  %-10.10s  %10.10s  %3.3s %-2.2s " , 
			 "Document No", 
			 "Cust. ", 
			 "Delivery Address", 
			 " Zone ", 
			 "D", 
			 "A", 
			 "Weight", 
			 "Volume", 
			 "Del. Date", 
			 "Asm. Date", 
			 "T/S", 
			 "St");

	LoadConts ();

	if (noInTab == 0)
	{
		tab_add (transportFile, ML ("There Are No Valid Packing Slips For "));
		tab_add (transportFile, ML ("The Selected Required Date."));

		tab_display (transportFile, TRUE);
		sleep (sleepTime);
		tab_close (transportFile, TRUE);
		return;
	}
	else
		tab_scan (transportFile);

	if (!restart)
		Update ();

	tab_close (transportFile, TRUE);
	return;
}

int
LoadConts (void)
{
	noInTab = 0;
	if (local_rec.option [0] == 'A')
		LoadAllocated ();
	else
	{
		ClearCapacity ();
		abc_selfield (cohr, "cohr_id_no2");
		LoadCohrNotAllocated ("T");
		LoadCohrNotAllocated ("P");
		LoadCohrNotAllocated ("N");
		LoadIthrNotAllocated ();
	}
	return (EXIT_SUCCESS);
}

int
LoadAllocated (void)
{

	char	invoiceNumber [12], 
			tranType [2];

	ClearCapacity ();
	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (ithr, "ithr_hhit_hash");
	abc_selfield (trhr, "trhr_trip_name");
	strcpy (trhr_rec.trip_name , local_rec.tripNumber); 
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (!cc)
	{
		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		trln_rec.hhco_hash = 0L;
		trln_rec.hhit_hash = 0L;
		cc = find_rec (trln, &trln_rec, GTEQ, "r");
		while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			strcpy (deliveryFlag, " ");
			strcpy (assemblyFlag, " ");
			strcpy (deliveryDate, "          ");
			strcpy (assemblyDate, "          ");
			if (trln_rec.hhco_hash != 0L)
			{
				cohr_rec.hhco_hash = trln_rec.hhco_hash;
				cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (trln, &trln_rec, NEXT, "r");
					continue;
				}
				cumr_rec.hhcu_hash = cohr_rec.hhcu_hash; 
				cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (trln, &trln_rec, NEXT, "r");
					continue;
				}
				strcpy (tranType, cohr_rec.type);
				sprintf (invoiceNumber, "%s %s", 
					(cohr_rec.type [0] == 'N') ? "CN" : "PS", cohr_rec.inv_no);

				if (cohr_rec.asm_date > 0L)
					strcpy (assemblyDate, DateToString (cohr_rec.asm_date));	
				if (cohr_rec.del_date > 0L)
					strcpy (deliveryDate, DateToString (cohr_rec.asm_date));	

				abc_selfield (trsh, "trsh_hhco_hash");
				trsh_rec.hhco_hash	=	cohr_rec.hhco_hash;
				cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
				strcpy (startTimeSlot, (cc) ? cohr_rec.s_timeslot 
										    : trsh_rec.sdel_slot);
				strcpy (endTimeSlot,   (cc) ? cohr_rec.e_timeslot 
											: trsh_rec.edel_slot);
				strcpy (scheduleFlag, (cc) 	? " " 
											: "*");
			}
			if (trln_rec.hhit_hash != 0L)
			{
				ithr_rec.hhit_hash = trln_rec.hhit_hash;
				cc = find_rec (ithr, &ithr_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (trln, &trln_rec, NEXT, "r");
					continue;
				}
				sprintf (invoiceNumber, "TR %08ld", ithr_rec.del_no); 
				if (ithr_rec.del_date > 0L)
					strcpy (deliveryDate, DateToString (ithr_rec.del_date));	

				abc_selfield (trsh, "trsh_hhit_hash");
				trsh_rec.hhit_hash	=	ithr_rec.hhit_hash;
				cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
				strcpy (startTimeSlot, 	(cc) ? ithr_rec.s_timeslot 
											 : trsh_rec.sdel_slot);
				strcpy (endTimeSlot, 	(cc) ? ithr_rec.e_timeslot 
											 : trsh_rec.edel_slot);
				strcpy (scheduleFlag, (cc) ? " " : "*");
			}
			++noInTab;

			CalculateCapacity 
			(
				NULL, 
				trln_rec.hhco_hash, 
				trln_rec.hhit_hash
			);

			if (cohr_rec.hhco_hash)
			{
				strcpy (deliveryFlag,(cohr_rec.del_req [0] == 'Y') ? "Y" : "N");
				strcpy (assemblyFlag,(cohr_rec.asm_req [0] == 'Y') ? "Y" : "N");
			}
			if (trln_rec.hhit_hash)
			{
				strcpy (deliveryFlag,(ithr_rec.del_req [0] == 'Y') ? "Y" : "N");
				strcpy (assemblyFlag, "N");
			}
			
			tab_add 
			(
				transportFile, 
				formatString, 
				invoiceNumber, 
				(trln_rec.hhco_hash) ? cumr_rec.dbt_no : " N/A  ", 
				(trln_rec.hhco_hash) ? cohr_rec.dl_add1 : ithr_rec.tran_ref, 
				(trln_rec.hhco_hash) ? cohr_rec.del_zone : ithr_rec.del_zone, 
				deliveryFlag, 
				assemblyFlag, 
				capacity.weightLine, 
				capacity.volumeLine, 
				deliveryDate, 
				assemblyDate, 
				startTimeSlot, 
				endTimeSlot, 
				scheduleFlag, 
				"A", 
				(trln_rec.hhco_hash) ? trln_rec.hhco_hash : 0L, 
				(trln_rec.hhit_hash) ? trln_rec.hhit_hash : 0L, 
				"t"
			);
			cc = find_rec (trln, &trln_rec, NEXT, "r");
		}
	}
	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (ithr, "ithr_hhtr_hash");
	abc_selfield (trhr, "trhr_id_no");
	return (EXIT_SUCCESS);
}

void
LoadCohrNotAllocated (
 char	*loadType)
{
	char	invoiceNumber [12];

	memset (&cohr_rec, 0, sizeof cohr_rec);
	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);

	sprintf (cohr_rec.type, "%-1.1s", loadType);
	sprintf (cohr_rec.inv_no, "%-8.8s", " ");
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) &&
				!strcmp (cohr_rec.br_no, comm_rec.est_no) &&
				cohr_rec.type [0] == loadType [0])
	{
		/*------------------------------------------------------------
		| If both normal delivery and assembly delivery are non-zero |
		| then delivery has already occured.                         |
		------------------------------------------------------------*/
		if (DEL_AND_ASM)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;	
		}
		/*-------------------------------------------------------
		| If no delivery date and no assembly date then ignore. |
		-------------------------------------------------------*/
		if (!DEL_DATE && !ASM_DATE)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;	
		}
		/*--------------------------------------
		| Validate delivery and assembly date. |
		--------------------------------------*/
		if ((cohr_rec.del_date > local_rec.lastdate &&
			cohr_rec.del_date > local_rec.firstdate) ||
			cohr_rec.del_date < local_rec.firstdate) 
		{
			if ((cohr_rec.asm_date > local_rec.lastdate &&
				cohr_rec.asm_date > local_rec.firstdate) ||
				cohr_rec.asm_date < local_rec.firstdate) 
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;	
			}
		}
		/*-------------------------
		| Validate delivery zone. |
		-------------------------*/
		if (strcmp (trzm_rec.del_zone, "      "))
		{
			if (strcmp (cohr_rec.del_zone, trzm_rec.del_zone))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;	
			}
		}
		/*---------------------------
		| Validate document number. |
		---------------------------*/
		if (strcmp (local_rec.documentNumber, "        "))
		{
			if (strcmp (cohr_rec.inv_no, local_rec.documentNumber))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;	
			}
		}
		/*---------------------
		| Validate Container. |
		---------------------*/
		if (strcmp (skcm_rec.container, "               "))
		{
			if (CheckContainer (cohr_rec.hhco_hash))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;	
			}
		}
		abc_selfield (trsh, "trsh_hhco_hash");
		trsh_rec.hhco_hash	=	cohr_rec.hhco_hash;
		cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
		strcpy (startTimeSlot, 	(cc) ? cohr_rec.s_timeslot 
									 : trsh_rec.sdel_slot);
		strcpy (endTimeSlot, 	(cc) ? cohr_rec.e_timeslot 
									 : trsh_rec.edel_slot);
		strcpy (scheduleFlag, (cc) ? " " : "*");

		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
	
		strcpy (deliveryFlag, " ");
		strcpy (assemblyFlag, " ");
		strcpy (deliveryDate, "          ");
		strcpy (assemblyDate, "          ");

		if (cohr_rec.asm_date > 0L)
			strcpy (assemblyDate, DateToString (cohr_rec.asm_date));
		if (cohr_rec.del_date > 0L)
			strcpy (deliveryDate, DateToString (cohr_rec.del_date));

		strcpy (deliveryFlag, (cohr_rec.del_req [0] == 'Y') ? "Y" : "N");
		strcpy (assemblyFlag, (cohr_rec.asm_req [0] == 'Y') ? "Y" : "N");
			
		CalculateCapacity (NULL, cohr_rec.hhco_hash, 0L);

		if (loadType [0] == 'T' || loadType [0] == 'P')
			sprintf (invoiceNumber, "PS %s", cohr_rec.inv_no);
		else
			sprintf (invoiceNumber, "CN %s", cohr_rec.inv_no);

		if (NO_DELIVERIES && SAME_DATES)
		{
			tab_add 
			(
				transportFile, 
				formatString, 
				invoiceNumber, 
				cumr_rec.dbt_no, 
				cohr_rec.dl_add1, 
				cohr_rec.del_zone, 
				deliveryFlag, 
				assemblyFlag, 
				capacity.weightLine, 
				capacity.volumeLine, 
				deliveryDate, 
				assemblyDate, 
				startTimeSlot, 
				endTimeSlot, 
				scheduleFlag, 
				"N", 
				cohr_rec.hhco_hash, 
				0L, 
				cohr_rec.type
			);
			++noInTab;
		}
		else
		{
			if ((!SAME_DATES || DEL_DONE) && !ASM_DONE && ASM_DATE)
			{
				tab_add 
				(
					transportFile, 
					formatString, 
					invoiceNumber, 
					cumr_rec.dbt_no, 
					cohr_rec.dl_add1, 
					cohr_rec.del_zone, 
					deliveryFlag, 
					assemblyFlag, 
					0.00, 
					0.00, 
					"00/00/0000", 
					assemblyDate, 
					startTimeSlot, 
					endTimeSlot, 
					scheduleFlag, 
					"N", 
					cohr_rec.hhco_hash, 
					0L, 
					cohr_rec.type
				);
				++noInTab;
			}
			if ((!SAME_DATES || ASM_DONE) && !DEL_DONE && DEL_DATE)
			{
				tab_add 
				(
					transportFile, 
					formatString, 
					invoiceNumber, 
					cumr_rec.dbt_no, 
					cohr_rec.dl_add1, 
					cohr_rec.del_zone, 
					deliveryFlag, 
					assemblyFlag, 
					capacity.weightLine, 
					capacity.volumeLine, 
					deliveryDate, 
					"00/00/0000", 
					startTimeSlot, 
					endTimeSlot, 
					scheduleFlag, 
					"N", 
					cohr_rec.hhco_hash, 
					0L, 
					cohr_rec.type
				);
				++noInTab;
			}
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
}

/*==================================
| Check if searched line is valid. |
==================================*/
int
CheckContainer (
	long	hhcoHash)
{
	coln_rec.hhco_hash	=	hhcoHash;
	coln_rec.line_no	=	0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == hhcoHash)
	{
		skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		cc = find_rec (skni, &skni_rec, COMPARISON, "r");
		if (!cc)
		{
			if (!strcmp (skcm_rec.container, skni_rec.container))
				return (EXIT_SUCCESS);
		}
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	return (EXIT_FAILURE);
}
void
LoadIthrNotAllocated (
 void)
{
	char	invoiceNumber [12];

	strcpy (ithr_rec.co_no, comm_rec.co_no);
	strcpy (ithr_rec.type, "T");
	ithr_rec.del_no	=	0L;
	cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
	while (!cc && !strcmp (ithr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (ithr_rec.type, "T"))
	{
		if ((ithr_rec.iss_date > local_rec.lastdate &&
			ithr_rec.iss_date > local_rec.firstdate) ||
			ithr_rec.iss_date < local_rec.firstdate) 
		{
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
			continue;	
		}
		if (ithr_rec.hhtr_hash != 0L || ithr_rec.del_req [0] != 'Y')
		{
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
			continue;	
		}  
		/*-------------------------
		| Validate delivery zone. |
		-------------------------*/
		if (strcmp (trzm_rec.del_zone, "      "))
		{
			if (strcmp (ithr_rec.del_zone, trzm_rec.del_zone))
			{
				cc = find_rec (ithr, &ithr_rec, NEXT, "r");
				continue;	
			}
		}

		CalculateCapacity 
		(
			NULL, 
			0L, 
			ithr_rec.hhit_hash
		);

		strcpy (deliveryFlag, " ");
		strcpy (assemblyFlag, " ");
		strcpy (deliveryDate, "          ");
		strcpy (assemblyDate, "          ");

		strcpy (deliveryFlag, (ithr_rec.del_req [0] == 'Y') ? "Y" : "N");
		strcpy (assemblyFlag, "N");
		if (ithr_rec.del_date > 0L)
			strcpy (deliveryDate, DateToString (ithr_rec.del_date));

		abc_selfield (trsh, "trsh_hhit_hash");
		trsh_rec.hhit_hash	=	ithr_rec.hhit_hash;
		cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
		strcpy (startTimeSlot, 	(cc) ? ithr_rec.s_timeslot 
									 : trsh_rec.sdel_slot);
		strcpy (endTimeSlot, 	(cc) ? ithr_rec.e_timeslot 
									 : trsh_rec.edel_slot);
		strcpy (scheduleFlag, (cc) ? " " : "*");

		sprintf (invoiceNumber, "TR %08ld", ithr_rec.del_no); 
		tab_add 
		(
			transportFile, 
			formatString, 
			invoiceNumber, 
			" N/A  ", 
			ithr_rec.tran_ref, 
			ithr_rec.del_zone, 
			deliveryFlag, 
			assemblyFlag, 
			capacity.weightLine, 
			capacity.volumeLine, 
			deliveryDate, 
			assemblyDate, 
			startTimeSlot, 
			endTimeSlot, 
			scheduleFlag, 
			"N", 
			0L, 
			ithr_rec.hhit_hash, 
			"T"
		);
		++noInTab;
		cc = find_rec (ithr, &ithr_rec, NEXT, "r");
	}

	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (ithr, "ithr_hhtr_hash");
	return ;
}
				
static	int
RestartFunc (
 int 	key, 
 KEY_TAB *psUnused)
{
	assert (key == FN1);

	restart = TRUE;
	return key;
}

static	int
ExitFunc (
 int 	key, 
 KEY_TAB *psUnused)
{
	assert (key == FN16);

	return key;
}

/*==============
| Tag routine. |
==============*/
static int 
AlloFunc (
 int key, 
 KEY_TAB *psUnused)
{
 	int		result;
	int		st_line;
	char	ans;

	if (DStat)
	{
		print_err (ML (mlTrMess023));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	st_line = tab_tline (transportFile);

	if (key == 'A')
	{
		result = TestTag (st_line, "A");
		if (!result)
		{
			tab_get (transportFile, bufferData, EQUAL, st_line);
			local_rec.weight += atof (bufferData + OFF_WEIGHT);
			local_rec.volume += atof (bufferData + OFF_VOLUME);
			if (local_rec.weight > local_rec.capacityWeight && 
				local_rec.capacityWeight > 0.00)
			{
				if (!overloadWeight)
				{
					ans = prmptmsg (ML (mlTrMess024), "YyNn", 1, 23);
					if (ans == 'N' || ans == 'n')
					{
						local_rec.weight -= atof (bufferData + OFF_WEIGHT);
						local_rec.volume -= atof (bufferData + OFF_VOLUME);
						return (key);	
					}
				}
			
				overloadWeight++;
				fl_pr 
				(
					vars [label ("weight")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
					vars [label ("weight")].row, 
					overloadWeight
				);
			}
			else
			{
				fl_pr 
				(
					vars [label ("weight")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
					vars [label ("weight")].row, 
					0
				);
			}
			if (local_rec.volume > local_rec.capacityVolume &&
				local_rec.capacityVolume > 0.00)
			{
				if (!overloadVolume)
				{
					ans = prmptmsg (ML (mlTrMess054), "YyNn", 1, 23);
					if (ans == 'N' || ans == 'n')
					{
						local_rec.volume -= atof (bufferData + OFF_VOLUME);
						return (key);	
					}
				}
				overloadVolume++;
				fl_pr 
				(
					vars [label ("volume")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
					vars [label ("volume")].row, 
					overloadVolume
				);
			}
			else
			{
				fl_pr 
				(
					vars [label ("volume")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
					vars [label ("volume")].row, 
					0
				);
			}

			TagLine (st_line, "A");
			DSP_FLD ("weight");
			DSP_FLD ("volume");
		}
	} 
	return (key);
}

static	int 
RemoveFunc (
 int	key, 
 KEY_TAB *psUnused)
{
	int		result, 
			st_line;
	char	tmpstr [30];

	if (DStat)
	{
		print_err (ML (mlTrMess023));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	st_line = tab_tline (transportFile);

	if (key == 'R')
	{
		result = TestTag (st_line, "R");
		if (!result)
		{
			tab_get (transportFile, bufferData, EQUAL, st_line);
			sprintf (tmpstr, "%1.1s", bufferData + OFF_STATUS);
			if (tmpstr [0] == 'A')
			{
				local_rec.weight -= atof (bufferData + OFF_WEIGHT);
				local_rec.volume -= atof (bufferData + OFF_VOLUME);
				DSP_FLD ("weight");
				DSP_FLD ("volume");

				if (local_rec.weight > local_rec.capacityWeight && 
					local_rec.capacityWeight > 0.00)
				{
					fl_pr 
					(
						vars [label ("weight")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
						vars [label ("weight")].row, 
						overloadWeight
					);
				}
				else
				{
					fl_pr 
					(
						vars [label ("weight")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
						vars [label ("weight")].row, 
						0
					);
				}
				if (local_rec.volume > local_rec.capacityVolume && 
					local_rec.capacityVolume > 0.00)
				{
					fl_pr 
					(
						vars [label ("volume")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
						vars [label ("volume")].row, 
						overloadVolume
					);
				}
				else
				{
					fl_pr 
					(
						vars [label ("volume")].prmpt, 
#ifdef	GVISION
					vars [label ("weight")].col,
#else
					vars [label ("weight")].col + 1,
#endif
						vars [label ("volume")].row, 
						0
					);
				}
			}

			TagLine (st_line, "R");
		}
	}
	return (key);
}

static	int 
MoveFunc (
	int		key, 
	KEY_TAB *psUnused)
{
	int		result, 
			st_line;
	char	tmpstr [30];

	if (DStat)
	{
		print_err (ML (mlTrMess023));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	st_line = tab_tline (transportFile);

	if (key == 'M')
	{
		result = TestTag (st_line, "M");
		if (!result)
		{
			cc = PopUp (st_line);
			if (!restart && !cc)
			{
				tab_get (transportFile, bufferData, EQUAL, st_line);
				sprintf (tmpstr, "%1.1s", bufferData + OFF_STATUS);
				if (tmpstr [0] == 'A')
				{
					local_rec.weight -= atof (bufferData + OFF_WEIGHT);
					local_rec.volume -= atof (bufferData + OFF_VOLUME);
				}
				DSP_FLD ("weight");
				DSP_FLD ("volume");
				TagLine (st_line, "M");
			}
			restart = 0;
			prog_exit = 0;
		}
	}

	redraw_table (transportFile);

	return (key);
}

int
TestTag (
 int	line_no, 
 char	*Tag)
{
	tab_get (transportFile, bufferData, EQUAL, line_no);
	sprintf (err_str, "%1.1s", bufferData + OFF_STATUS);

	if (Tag [0] == 'R' && err_str [0] == 'A')
		return (EXIT_SUCCESS);

	if (Tag [0] == 'A' && (err_str [0] == 'N' || err_str [0] == 'R'))
		return (EXIT_SUCCESS);

	if (Tag [0] == 'M' && (err_str [0] == 'A' || err_str [0] == 'T' || err_str [0] == 'N'))
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
PopUp (
 int	line_no)
{
	int	stat = 0;

	Screen2 = TRUE;
	prog_exit = 0;
	while (prog_exit == 0)
	{
		heading2 ();
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	FALSE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		init_ok 	= 	TRUE;
		init_vars (2);
		scn_set (2);
		scn_write (2);
		entry (2);
		if (!restart && !prog_exit)
		{
			heading2 ();
			edit (2);
			if (!restart)
				stat = MovePSlip (line_no);
			prog_exit = TRUE;
		}
		line_at (20,0,LCL_SCR_WIDTH);

		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_at (23,0,LCL_SCR_WIDTH);
		prog_exit = TRUE;
	}
	Screen2 = FALSE;
	search_ok 	= 	TRUE;
	entry_exit 	= 	FALSE;
	edit_exit 	= 	FALSE;
	prog_exit 	= 	FALSE;
	init_ok 	= 	TRUE;
	return (stat);
}

void
heading2 (
 void)
{
	//tab_display (transportFile, TRUE); // remove, not necessary, cursor not shown on LS10-GUI 
	cl_box (30, 11, 70, 8);
	move (31, 15);
	line (68);
	rv_pr (ML (mlTrMess025), 60, 11, 1);
	rv_pr (ML (mlTrMess036), 54, 19, 1);
	scn_write (2);
	scn_display (2);
	return;
}

int
MovePSlip (
 int	line_no)
{
	char	ans, 
			Invoice [12], 
			tranType [2];
	float	tempWeight	=	0.00;

	ClearCapacity ();

	abc_selfield (cohr, "cohr_id_no");
	abc_selfield (ithr, "ithr_id_no");
	abc_selfield (cumr, "cumr_hhcu_hash");
	tab_get (transportFile, bufferData, EQUAL, line_no);
	sprintf (Invoice, "%-11.11s", bufferData + OFF_INV_NO);
	sprintf (tranType, "%-1.1s", bufferData + OFF_TRAN_TYPE);

	CalculateCapacity (local_rec.trip2Number, 0L, 0L);

	if (capacity.weightItems > capacity.weightHeader)
		tempWeight	=	capacity.weightItems;
	else
		tempWeight	=	capacity.weightHeader;
	
	if (tempWeight > trve2_rec.max_wgt)
	{
		ans = prmptmsg (ML (mlTrMess024), "YyNn", 1, 23);
		if (ans == 'N' || ans == 'n')
			return (EXIT_FAILURE);	
	}
	if (capacity.volume > trve2_rec.max_vol)
	{
		ans = prmptmsg (ML (mlTrMess054), "YyNn", 1, 23);
		if (ans == 'N' || ans == 'n')
			return (EXIT_FAILURE);	
	}
	strcpy (trhr_rec.trip_name, local_rec.trip2Number);
	strcpy (trhr_rec.driver, local_rec.driver);
	trhr_rec.fr_chg		=	local_rec.vehicleFreight2;
	trhr_rec.fr_zchg	=	local_rec.zoneFreight2;
	
	cc = abc_add (trhr, &trhr_rec);
	if (cc)
		file_err (cc, "trhr", "DBADD");
	
	abc_selfield (trhr2, "trhr_trip_name");
	
	strcpy (trhr2_rec.trip_name, local_rec.trip2Number);
	cc = find_rec (trhr2, &trhr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, trhr2, "DBFIND");
	
	abc_selfield (trhr2, "trhr_hhtr_hash");
	abc_selfield (trhr, "trhr_trip_name");

	strcpy (trhr_rec.trip_name, local_rec.trip2Number);
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "trhr", "DBFIND");

	if (tranType [0] == 'T')
	{
		strcpy (ithr_rec.co_no, comm_rec.co_no);
		ithr_rec.del_no	=	atol (Invoice + 3);
		strcpy (ithr_rec.type, "T");
		cc = find_rec (ithr, &ithr_rec, EQUAL, "u");
		if (!cc)
		{           
			trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
			trln_rec.hhco_hash = 0L;
			trln_rec.hhit_hash = ithr_rec.hhit_hash;
			cc = find_rec (trln, &trln_rec, EQUAL, "u");
			if (!cc)
			{
				trln_rec.hhtr_hash = trhr2_rec.hhtr_hash;
				cc = abc_update (trln, &trln_rec);		
				if (cc)
					file_err (cc, "trln", "DBUPDATE");
			}
			else
			{
				trln_rec.hhtr_hash = trhr2_rec.hhtr_hash;
				trln_rec.hhco_hash = 0L;
				trln_rec.hhit_hash = ithr_rec.hhit_hash;
				trln_rec.hhln_hash = 0L;
				cc = abc_add (trln, &trln_rec);
				if (cc)
					file_err (cc, "trln", "DBADD");
			}
			ithr_rec.hhit_hash =  trhr_rec.hhtr_hash;
			cc = abc_update (ithr, &ithr_rec);
			if (cc)
				file_err (cc, "ithr", "DBUPDATE");
		}
		else
			abc_unlock (ithr);
	}
	else
	{
		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		strcpy (cohr_rec.inv_no, Invoice + 3);
		strcpy (cohr_rec.type, tranType);
		cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
		if (!cc)
		{           
			cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "cumr", "DBFIND");

			trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
			trln_rec.hhco_hash = cohr_rec.hhco_hash;
			trln_rec.hhit_hash = 0L;
			cc = find_rec (trln, &trln_rec, EQUAL, "u");
			if (!cc)
			{
				trln_rec.hhtr_hash = trhr2_rec.hhtr_hash;
				cc = abc_update (trln, &trln_rec);		
				if (cc)
					file_err (cc, "trln", "DBUPDATE");
			}
			else
			{
				trln_rec.hhtr_hash = trhr2_rec.hhtr_hash;
				trln_rec.hhco_hash = cohr_rec.hhco_hash;
				trln_rec.hhit_hash = 0L;
				trln_rec.hhln_hash = 0L;
				cc = abc_add (trln, &trln_rec);
				if (cc)
					file_err (cc, "trln", "DBADD");
			}
			if (SAME_DATES)
			{
				cohr_rec.hhtr_hash 	=  trhr_rec.hhtr_hash;
				cohr_rec.asm_hash 	=  trhr_rec.hhtr_hash;
			}
			else
			{
				if (!ASM_DONE && ASM_DATE)
					cohr_rec.asm_hash 	=  trhr_rec.hhtr_hash;
				
				if (!DEL_DONE && DEL_DATE)
					cohr_rec.hhtr_hash 	=  trhr_rec.hhtr_hash;
			}
			cc = abc_update (cohr, &cohr_rec);
			if (cc)
				file_err (cc, "cohr", "DBUPDATE");

		}
		else
			abc_unlock (cohr); 
	}

	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (ithr, "ithr_hhtr_hash");
	abc_selfield (cumr, "cumr_hhcu_hash");

	if (envVarSoFrChg)
		UpdateTrhr (local_rec.trip2Number);

	return (EXIT_SUCCESS);
}

void
TagLine (
 int		line_no, 
 char	*Tag)
{
	tab_get 
	(
		transportFile, 
		bufferData, 
		EQUAL, 
		line_no
	);
	tab_update
	(
		transportFile, 
		"%*.*s%-1.1s  %s", 
		OFF_STATUS, 
		OFF_STATUS, 
		bufferData, 
		Tag, 
		bufferData + OFF_HHCO_HASH
	);
}

int
TimeSlotFunc (
 int	key, 
 KEY_TAB *psUnused)
{
	int		i;
	char	leftStr [81], 
			rightStr [81], 
			disp_str [161];
	int		dayWeek, 
			noTrips = 0;

	char	workStartTime [6], 
			workEndTime [6], 
			tempAvailableStr [11], 
			tempAllocatedStr [11], 
			startAmPmFlag [3], 
			endAmPmFlag [3];

	open_rec (trzt,  trzt_list, TRZT_NO_FIELDS, "trzt_id_no"); 
	open_rec (trzc,  trzc_list, TRZC_NO_FIELDS, "trzc_id_no"); 
	abc_selfield (trsh, "trsh_id_no");

	Dsp_open (10, 3, 12);
	Dsp_saverec ("Code|Start Time| End Time | No Trips |Aval.Trips|Code|Start Time| End Time | No Trips |Aval.Trips");
	Dsp_saverec ("");
	Dsp_saverec (" [EDIT/END]");

	for (i = 0; strlen (TC [i].time_code); i++)
	{
		dayWeek	=	DayOfWeek (local_rec.del_date);

		strcpy (trzt_rec.co_no, comm_rec.co_no);
		strcpy (trzt_rec.time_code, TC [i].time_code);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
		{
			trzt_rec.start_time	=	0L;
			trzt_rec.end_time	=	0L;
		}
		strcpy (startAmPmFlag, "AM");
		strcpy (endAmPmFlag, "AM");
		if (trzt_rec.start_time > 719)
		{
			trzt_rec.start_time -= 720;
			strcpy (startAmPmFlag, "PM");
		}
		if (trzt_rec.end_time > 719)
		{
			trzt_rec.end_time -= 720;
			strcpy (endAmPmFlag, "PM");
		}

		strcpy (workStartTime, ttoa (trzt_rec.start_time, "NN:NN"));
		strcpy (workEndTime, ttoa (trzt_rec.end_time, "NN:NN"));

		trzc_rec.capacity = -1.00;
		if (trzm_rec.trzm_hash != 0L)
		{
			trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;	
			trzc_rec.del_dcode	=	dayWeek + 1;	
			strcpy (trzc_rec.time_slot, trzt_rec.time_code);
			cc = find_rec (trzc, &trzc_rec, COMPARISON, "r");
			if (cc)
				trzc_rec.capacity = -2.00;

			noTrips	=	GetAvailableDeliveries
						(
							trzm_rec.trzm_hash, 
							local_rec.del_date, 
							trzt_rec.time_code
						);
		}
		if (trzc_rec.capacity == -1.00)
		{
			strcpy (tempAvailableStr, " No Zone. ");
			strcpy (tempAllocatedStr, " No Zone. ");
		}
		else if (trzc_rec.capacity == -2.00)
		{
			strcpy (tempAvailableStr, "No T.Slots");
			strcpy (tempAllocatedStr, "No T.Slots");
		}
		else if (trzc_rec.capacity == 0.00)
		{
			strcpy (tempAvailableStr, "          ");
			strcpy (tempAllocatedStr, "          ");
		}
		else
		{
			sprintf (tempAvailableStr, " %8.0f ", trzc_rec.capacity);
			sprintf (tempAllocatedStr, " %8.0f ", trzc_rec.capacity - noTrips);
		}

		if (i % 2)
		{

			sprintf (rightStr, "  %s ^E %s %s ^E %s %s ^E%10.10s^E%10.10s", 	
					trzt_rec.time_code, 
					workStartTime, 
					startAmPmFlag, 
					workEndTime, 
					endAmPmFlag, 
					tempAvailableStr, 
					tempAllocatedStr);

			sprintf (disp_str, "%s^E%s", leftStr, rightStr);
	
			Dsp_saverec (disp_str);
		}
		else
		{
			sprintf (leftStr, "  %s ^E %s %s ^E %s %s ^E%10.10s^E%10.10s", 	
					trzt_rec.time_code, 
					workStartTime, 
					startAmPmFlag, 
					workEndTime, 
					endAmPmFlag, 
					tempAvailableStr, 
					tempAllocatedStr);
		}
	}
	abc_fclose (trzt);
	Dsp_srch ();
	Dsp_close ();
	heading (1);
	scn_display (1); 
	Dsp_heading ();
	redraw_table (transportFile);
	return (EXIT_SUCCESS);
}

int
ViewFunc (
 int	key, 
 KEY_TAB *psUnused)
{
	char	run_string [100], 
			Invoice [12], 
			tranType [2];
	int		st_line;

	abc_selfield (cumr, "cumr_id_no");
	st_line = tab_tline (transportFile);
	tab_get (transportFile, bufferData, EQUAL, st_line);
	sprintf (Invoice, "%-11.11s", bufferData + OFF_INV_NO);
	sprintf (tranType, "%-1.1s", bufferData + OFF_TRAN_TYPE);

	if (tranType [0] == 't')
	{
		errmess (ML ("Sorry, Transfers cannot yet be viewed."));
		sleep (sleepTime);
		return (EXIT_SUCCESS);
	}

	sprintf (cumr_rec.dbt_no, "%-6.6s", bufferData + OFF_CUST_NO);
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, comm_rec.est_no);
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "cumr", "DBFIND");
	
	sprintf 
	(
		run_string, 
		"%-2.2s %-2.2s %s %010ld %-10.10s", 
		comm_rec.co_no, 
		comm_rec.est_no, 
		tranType, 
		cumr_rec.hhcu_hash, 
		Invoice + 3
	); 

	box (40, 21, 40, 1);
	rv_pr (ML (mlStdMess035), 41, 22, 1);

	if (tranType [0] == 'N')
	{
		arg [0] = "so_ccn_sdsp";
		arg [1]	= "N";
		arg [2]	= "7";
		arg [3]	= "so_ccn_inp.s";
		arg [4] = "0";
		arg [5] = run_string;
		arg [6] = (char *)0;
	}
	else
	{
		arg [0] = "so_sinvdisp";
		arg [1]	= "N";
		arg [2]	= "7";
		arg [3]	= "T";
		arg [4]	= "so_invoice.s";
		arg [5] = "0";
		arg [6] = run_string;
		arg [7] = (char *)0;
	}

	shell_prog (2); 

	heading (1);
	scn_display (1); 
	Dsp_heading ();
	redraw_table (transportFile);
	abc_selfield (cumr, "cumr_hhcu_hash");
	return (EXIT_SUCCESS);
}

/*==================
| Update function. |
==================*/
void
Update (
 void)
{
	int		count;
	char	curr_stat [2];
	long	hhcoHash, 
			hhitHash;
	long	deliveryDate, 
			assemblyDate;
	char	timeSlot [2];

	for (count = 0; count < noInTab; count++)
	{
		tab_get (transportFile, bufferData, EQUAL, count);

		hhcoHash		=	atol (bufferData + OFF_HHCO_HASH);
		hhitHash		=	atol (bufferData + OFF_HHIT_HASH);
		deliveryDate	=	StringToDate (bufferData + OFF_DEL_DATE);
		assemblyDate	=	StringToDate (bufferData + OFF_ASM_DATE);
		sprintf (timeSlot, 	"%-1.1s", bufferData + OFF_TIME_SLOTS);
		sprintf (curr_stat, "%-1.1s", bufferData + OFF_STATUS);

		if (curr_stat [0] == 'A')
		{
			AllocateTrip 
			(
				hhcoHash, 
				hhitHash, 
				deliveryDate, 
				assemblyDate, 
				timeSlot
			);
		}
		else if (curr_stat [0] == 'R')
		{
			RemoveTrip 
			(
				hhcoHash, 
				hhitHash, 
				deliveryDate, 
				assemblyDate, 
				timeSlot
			);
		}
	}
	if (envVarSoFrChg)
		UpdateTrhr (local_rec.tripNumber);
	
	return;
}

/*===========================================================
| Function - Remove a trip                         			|
|   If line being removed from a trip is a Packing slip or  |
|   Collection note the hhco_hash will be non zero.			|
|   If line being removed from a trip is a transfer the 	|
|   hhit_hash will be non zero.								|
===========================================================*/
void
RemoveTrip (
	long	hhcoHash, 
	long	hhitHash, 
	long	deliveryDate, 
	long	assemblyDate, 
	char	*timeSlot)
{
	abc_selfield (trhr, "trhr_trip_name");

	strcpy (trhr_rec.trip_name, local_rec.tripNumber);
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (cc)
	{
		abc_selfield (trhr, "trhr_id_no");
		return;
	}

	/*-----------------------------------------
	| Check if trip has already been deleted. |
	-----------------------------------------*/
	if (trhr_rec.status [0] == 'D')
	{
		print_mess (ML (mlTrMess023));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
	if (trzm_rec.trzm_hash)
	{
		if (hhcoHash)
			abc_selfield (trsh, "trsh_hhco_hash");
		else
			abc_selfield (trsh, "trsh_hhit_hash");
		
		trsh_rec.hhco_hash	=	hhcoHash;
		trsh_rec.hhit_hash	=	hhitHash;
		cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
		if (!cc)
		{
			cc = abc_delete (trsh);
			if (cc)
				file_err (cc, "trsh", "DBDELETE");
		}
	}
	/*--------------------------------
	| Find existing trip and delete. |
	--------------------------------*/
	trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
	trln_rec.hhco_hash = hhcoHash;
	trln_rec.hhit_hash = hhitHash;
	cc = find_rec (trln, &trln_rec, EQUAL, "r");
	if (!cc)
	{
		cc = abc_delete (trln);
		if (cc)
			file_err (cc, "trln", "DBDELETE");
	
		/*---------------------------------------------------------------
		| if (hhcoHash is non zero then cohr record should exist. 		|
		| We are going to find it and set the transport hash to    		|
		| zero. We have to perform some other processing for freight 	|
		| charges also.                                              	|
		---------------------------------------------------------------*/
		if (hhcoHash)
		{
			abc_selfield (cohr2, "cohr_hhco_hash");

			cohr2_rec.hhco_hash	=	hhcoHash;
			cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "u");
			if (!cc)
			{
				if (assemblyDate <= 0L && deliveryDate > 0L)
					cohr2_rec.hhtr_hash = 0L;

				if (deliveryDate <= 0L && assemblyDate > 0L)
					cohr2_rec.asm_hash = 0L;

				if (deliveryDate == assemblyDate)
				{
					cohr2_rec.hhtr_hash = 0L;
					cohr2_rec.asm_hash = 0L;
				}
				if (envVarSoFrChg)
					cohr2_rec.freight   = 0.00;

				cc = abc_update (cohr2, &cohr2_rec);
				if (cc)
					file_err (cc, cohr2, "DBUPDATE");

				if (envVarSoFrChg)
				{
					coln_rec.hhco_hash = cohr2_rec.hhco_hash;
					coln_rec.line_no   = 0;
					cc = find_rec (coln, &coln_rec, GTEQ, "u");
					while (!cc && coln_rec.hhco_hash == cohr2_rec.hhco_hash)
					{
						coln_rec.freight = 0.00;
						cc = abc_update (coln, &coln_rec);
						if (cc)
							file_err (cc, "coln", "DBUPDATE");

						cc = find_rec (coln, &coln_rec, NEXT, "u");
					}
					abc_unlock (coln);
				}
			}
			else
				abc_unlock (cohr2);
		}
		/*---------------------------------------------------------------
		| if (hhitHash is non zero then ithr record should exist. 		|
		| We are going to find it and set the transport hash to    		|
		| zero.                                                         |
		---------------------------------------------------------------*/
		if (hhitHash)
		{
			ithr_rec.hhit_hash	=	hhitHash;
			cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
			if (!cc)
			{
				ithr_rec.hhtr_hash =  0L;
				cc = abc_update (ithr, &ithr_rec);
				if (cc)
					file_err (cc, "ithr", "DBUPDATE");
			}
			else
				abc_unlock (ithr);
		}
	}
	abc_selfield (trhr, "trhr_id_no");
}
		

/*===========================================================
| Function - Allocate lines to a trip.             			|
|   Allocate lines to a trip by setting the hhtr hash 		|
===========================================================*/
void
AllocateTrip (
	long	hhcoHash, 
	long	hhitHash, 
	long	deliveryDate, 
	long	assemblyDate, 
	char	*timeSlot)
{
	abc_selfield (trhr, "trhr_trip_name");

	strcpy (trhr_rec.trip_name, local_rec.tripNumber);
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (cc)
	{
		/*--------------------------------
		| Find existing trip and delete. |
		--------------------------------*/
		if (trhr_rec.status [0] == 'D')
		{
			print_mess (ML (mlTrMess023));
			sleep (sleepTime);
			clear_mess ();
			return;
		}
		
		strcpy (trhr_rec.driver, 	local_rec.driver);
		strcpy (trhr_rec.trip_name, 	local_rec.tripNumber);
		trhr_rec.hhve_hash 	= 	local_rec.hhve_hash;
		trhr_rec.del_date 	= 	local_rec.del_date;
		trhr_rec.fr_chg		=	local_rec.vehicleFreight;
		trhr_rec.fr_zchg	=	local_rec.zoneFreight;

		cc = abc_add (trhr, &trhr_rec);
		if (cc)
			file_err (cc, "trhr", "DBADD");

		strcpy (trhr_rec.trip_name, local_rec.tripNumber);
		cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "trhr", "DBFIND");
	}
	else
	{
		strcpy (trhr_rec.driver, 	local_rec.driver);
		strcpy (trhr_rec.trip_name, 	local_rec.tripNumber);
		trhr_rec.del_date 		= 	local_rec.del_date;
		trhr_rec.fr_chg			=	local_rec.vehicleFreight;
		trhr_rec.fr_zchg		=	local_rec.zoneFreight;
		cc = abc_update (trhr, &trhr_rec);
		if (cc)
			file_err (cc, "trhr", "DBUPDATE");
	}
	trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
	trln_rec.hhco_hash = hhcoHash;
	trln_rec.hhit_hash = hhitHash;
	cc = find_rec (trln, &trln_rec, EQUAL, "r");
	if (cc)
	{
		cc = abc_add (trln, &trln_rec);
		if (cc)
			file_err (cc, "trln", "DBADD");
	}
	if (trzm_rec.trzm_hash)
	{
		if (hhcoHash)
			abc_selfield (trsh, "trsh_hhco_hash");
		else
			abc_selfield (trsh, "trsh_hhit_hash");

		trsh_rec.hhco_hash	=	hhcoHash;
		trsh_rec.hhit_hash	=	hhitHash;
		cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
		if (cc)
		{
			trsh_rec.trzm_hash	=	trzm_rec.trzm_hash;
			trsh_rec.del_date	=	local_rec.del_date;
			sprintf (trsh_rec.sdel_slot, "%-1.1s", timeSlot);
			sprintf (trsh_rec.edel_slot, "%-1.1s", timeSlot);
			trsh_rec.hhit_hash	=	hhitHash;
			trsh_rec.hhco_hash	=	hhcoHash;
			cc = abc_add (trsh, &trsh_rec);
			if (cc)
				file_err (cc, "trsh", "DBADD");
		}
		else
		{
			trsh_rec.trzm_hash	=	trzm_rec.trzm_hash;
			trsh_rec.del_date	=	local_rec.del_date;
			sprintf (trsh_rec.sdel_slot, "%-1.1s", timeSlot);
			sprintf (trsh_rec.edel_slot, "%-1.1s", timeSlot);
			trsh_rec.hhco_hash	=	hhcoHash;
			trsh_rec.hhit_hash	=	hhitHash;
			cc = abc_update (trsh, &trsh_rec);
			if (cc)
				file_err (cc, "trsh", "DBUPDATE");

		}
	}
	/*---------------------------------------------------------------
	| if (hhcoHash is non zero then cohr record should exist. 		|
	| We are going to find it and set the transport hash     		|
	---------------------------------------------------------------*/
	if (hhcoHash)
	{
		abc_selfield (cohr2, "cohr_hhco_hash");

		cohr2_rec.hhco_hash	=	hhcoHash;
		cc = find_rec (cohr2, &cohr2_rec, COMPARISON, "u");
		if (!cc)
		{
			if (assemblyDate <= 0L && deliveryDate > 0L)
			{
				cohr2_rec.hhtr_hash = trhr_rec.hhtr_hash;
				if (cohr2_rec.asm_date <= 0L)
					cohr2_rec.asm_hash = trhr_rec.hhtr_hash;
			}

			if (deliveryDate == 0L && assemblyDate > 0L)
			{
				cohr2_rec.asm_hash 	= trhr_rec.hhtr_hash;
				if (cohr2_rec.del_date <= 0L)
					cohr2_rec.hhtr_hash = trhr_rec.hhtr_hash;
			}

			if (deliveryDate == assemblyDate)
			{
				cohr2_rec.asm_hash 	= trhr_rec.hhtr_hash;
				cohr2_rec.hhtr_hash = trhr_rec.hhtr_hash;
			}
			if (cohr2_rec.type [0] == 'P')
				strcpy (cohr2_rec.type, "T");

			cc = abc_update (cohr2, &cohr2_rec);
			if (cc)
				file_err (cc, cohr2, "DBUPDATE");
		}
		else
			abc_unlock (cohr2);
	}
	/*---------------------------------------------------------------
	| if (hhitHash is non zero then ithr record should exist. 		|
	| We are going to find it and set the transport hash    		|
	---------------------------------------------------------------*/
	if (hhitHash)
	{
		ithr_rec.hhit_hash	=	hhitHash;
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
		if (!cc)
		{
			ithr_rec.hhtr_hash =  trhr_rec.hhtr_hash;
			cc = abc_update (ithr, &ithr_rec);
			if (cc)
				file_err (cc, "ithr", "DBUPDATE");
		}
		else
			abc_unlock (ithr);
	}
	abc_selfield (trhr, "trhr_id_no");
}
	
int
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();
	strcpy (err_str, ML (mlTrMess026));
	rv_pr (err_str, ((LCL_SCR_WIDTH - (int) strlen (err_str)) / 2), 0, 1);
	
	box (0, 1, LCL_SCR_WIDTH, 6);
	scn_set (1);
	scn_write (1);
	scn_display (1);

	line_at (20,0,LCL_SCR_WIDTH);
	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	if (Screen2)
		heading2 ();

    return (EXIT_SUCCESS);
}

/*==================
| Search for skcm. |
==================*/
void
SrchSkcm (
 char	*keyValue)
{
	work_open ();
	save_rec ("#Container", "#Description");
	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", keyValue);
	cc = find_rec (skcm, &skcm_rec, GTEQ, "r");
	while (!cc && !strcmp (skcm_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm_rec.container, keyValue, strlen (keyValue)))
	{
		cc = save_rec (skcm_rec.container, skcm_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm, &skcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container, "%-15.15s", temp_str);
	cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)skcm, "DBFIND");
}
/*==========================
| Search for p_slip number |
==========================*/
void
ShowDocumentNumber (
	char	*key_val)
{
	work_open ();
	save_rec ("#Document No", "#Customer Name ");

	SrchCohr (key_val, "T", "PS");
	SrchCohr (key_val, "P", "PS");
	SrchCohr (key_val, "N", "CN");
	SrchIthr (key_val);

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (temp_str, temp_str + 3);
}

void
SrchCohr (
 char	*key_val, 
 char	*Type, 
 char	*Code)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	sprintf (cohr2_rec.inv_no, "%-8.8s", key_val);
	sprintf (cohr2_rec.type, Type);

	cc = find_rec (cohr2, &cohr2_rec, GTEQ, "r");
	while (!cc && !strncmp (cohr2_rec.inv_no, key_val, strlen (key_val)) && 
				  !strcmp (cohr2_rec.co_no, comm_rec.co_no) && 
				  !strcmp (cohr2_rec.br_no, comm_rec.est_no) &&
				  cohr2_rec.type [0] == Type [0])
	{
		cumr_rec.hhcu_hash	=	cohr2_rec.hhcu_hash;
		cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cumr", "DBFIND");

		sprintf (err_str, "%-2.2s %s", Code, cohr2_rec.inv_no);
		cc = save_rec (err_str, cumr_rec.dbt_no);
		if (cc)
			break;
		
		cc = find_rec (cohr2, &cohr2_rec, NEXT, "r");
	}
}
void
SrchIthr (
 char	*key_val)
{
	strcpy (ithr_rec.co_no, comm_rec.co_no);
	ithr_rec.del_no	=	atol (key_val);
	strcpy (ithr_rec.type, "T");

	cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
	while (!cc && !strcmp (ithr_rec.co_no, comm_rec.co_no) && 
				  ithr_rec.type [0] == 'T')
	{
		cumr_rec.hhcu_hash	=	cohr2_rec.hhcu_hash;
		cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cumr", "DBFIND");

		sprintf (err_str, "TR %08ld", ithr_rec.del_no);
		cc = save_rec (err_str, ithr_rec.tran_ref);
		if (cc)
			break;
		
		cc = find_rec (ithr, &ithr_rec, NEXT, "r");
	}
}
void
SrchTrve (
 char	*key_val)
{
	work_open ();
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", key_val);
	save_rec ("#Vehicle", "#Description");
	cc = find_rec (trve, &trve_rec, GTEQ, "r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
		          !strcmp (trve_rec.br_no, comm_rec.est_no) &&
		          !strncmp (trve_rec.ref, key_val, strlen (key_val)))
	{
		cc = save_rec (trve_rec.ref, trve_rec.desc);
		if (cc)
			break;
		cc = find_rec (trve, &trve_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	sprintf (trve_rec.ref, "%-10.10s", temp_str);
	cc = find_rec (trve, &trve_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "trve", "DBFIND");
}

/*================================================
| Search for Trip Number Transport Header File . |
================================================*/
void
SrchTrhr (
 char *key_val)
{
	work_open ();
	save_rec ("#Trip Number", "#Del Date");
	sprintf (trhr_rec.trip_name, "%-12.12s", key_val);
	trhr_rec.hhve_hash = local_rec.hhve_hash;
	trhr_rec.del_date  = local_rec.del_date; 
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");
	while (!cc && 
				!strncmp (trhr_rec.trip_name, key_val, strlen (key_val)) && 
				trhr_rec.hhve_hash == trve_rec.hhve_hash && 
                trhr_rec.del_date == local_rec.del_date)
			
	{
		strcpy (err_str, DateToString (trhr_rec.del_date));
		cc = save_rec (trhr_rec.trip_name, err_str);
		if (cc)
			break;
		cc = find_rec (trhr, &trhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		print_mess (ML (mlTrMess003));
		sleep (sleepTime);
		clear_mess ();
		return;
	}
}

/*====================+
| Search for Date     |
---------------------*/
void
SrchDelDate (
 char	*key_val)
{
 	char 	date_trp [25];

	work_open ();
	save_rec ("#Tr Date  Trip Number", "#Vehicle Number");
	
	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate (key_val);
	cc = find_rec (trhr, &trhr_rec, GTEQ, "r");

	while (!cc 
		    &&  !strncmp (DateToString (trhr_rec.del_date), key_val, strlen (key_val))
			&& trhr_rec.hhve_hash == trve_rec.hhve_hash)
	{
		if (!strcmp (trhr_rec.status, " "))
		{
			sprintf (date_trp, "%-10.10s %-12.12s", 
						DateToString (trhr_rec.del_date), trhr_rec.trip_name);
			cc = save_rec (date_trp, trve_rec.ref);
			if (cc)
				break;

		}
		cc = find_rec (trhr, &trhr_rec, NEXT, "r");

	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char *key_val)
{
	work_open ();

	save_rec ("#Zone. ", "#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.est_no) &&
				  !strncmp (trzm_rec.del_zone, key_val, strlen (key_val)))
	{
		cc = save_rec (trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;

		cc = find_rec (trzm, &trzm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.est_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "trzm", "DBFIND");

	return;
}

void
SrchExtf (
 char	*key_val)
{
	work_open ();
	save_rec ("#Cd", "#Trucker's Name");
	strcpy (extf_rec.co_no, comm_rec.co_no);
	sprintf (extf_rec.code, "%-6.6s", key_val);
	cc = find_rec ("extf", &extf_rec, GTEQ, "r");

	while (!cc && !strcmp (extf_rec.co_no, comm_rec.co_no) &&
		      !strncmp (extf_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (extf_rec.code, extf_rec.name);
		if (cc)
			break;

		cc = find_rec ("extf", &extf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (extf_rec.co_no, comm_rec.co_no);
	sprintf (extf_rec.code, "%-6.6s", temp_str);
	cc = find_rec ("extf", &extf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "extf", "DBFIND");
}
			
/*===============================================================
| Miscellaneous Functions
===============================================================*/
static BOOL
IsYesResponse (
 char *prompt)
{
	int 	ch;
	char 	promptYN [LCL_SCR_WIDTH + 1];

	strcpy (promptYN, prompt);

	/*-------------------------------
	| Substitute ? for standard Y/N  
	-------------------------------*/
	if (!strchr (promptYN, '?'))
		strcat (clip (promptYN), "?");

	strcpy (strrchr (promptYN, '?'), "? (Y/N) ");

	assert (strlen (promptYN) < sizeof promptYN);

	ch = prmptmsg (promptYN, "YyNn", 0, 23);
	return (ch == 'y' || ch == 'Y');
}


/*----------------------+
| Assign New Trip No.  |
----------------------*/
static char *
GetNewTripNumber (
 char *dest)
{
	time_t	tloc	=	time (NULL);

	struct tm	*tme;

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	comr_rec.nx_trip_no++;

	cc = abc_update (comr, &comr_rec);
	if (cc)
		file_err (cc, "comr", "DBUPDATE");

	tme = localtime (&tloc);
	sprintf (dest, "%04d%02d%02d%04ld", 
					tme->tm_year + 1900, 
					tme->tm_mon + 1, 
					tme->tm_mday, 
					comr_rec.nx_trip_no);

	return dest;
}


/*----------------------+
| Create New Trip No.  |
----------------------*/
static int
CreateTrip (
 int trp)
{
	switch (trp) {

		case 	1: 
				GetNewTripNumber (local_rec.tripNumber);   
				strcpy (trhr_rec.trip_name, local_rec.tripNumber);
				break;

		case 	2:
				GetNewTripNumber (local_rec.trip2Number);   
				strcpy (trhr_rec.trip_name, local_rec.trip2Number);
				break;
		}

	return (EXIT_SUCCESS);
}

void
UpdateTrhr (
 char *tripNumber)
{
	float	tot_kgs		= 0.00;
	double	tot_freight = 0.00;
	float	weight		= 0.00;

	
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);

	tot_kgs	= 0.00;
	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (trhr, "trhr_trip_name");
	strcpy (trhr_rec.trip_name , tripNumber); 
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (!cc && !strcmp (trhr_rec.trip_name, tripNumber))
	{
		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		trln_rec.hhco_hash = 0L;
		trln_rec.hhit_hash = 0L;
		cc = find_rec (trln, &trln_rec, GTEQ, "r");
		while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			cohr_rec.hhco_hash = trln_rec.hhco_hash;
			cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
			if (!cc && cohr_rec.hhco_hash == trln_rec.hhco_hash)
			{
				coln_rec.hhco_hash = cohr_rec.hhco_hash;
				coln_rec.line_no   = 0;
				cc = find_rec (coln, &coln_rec, GTEQ, "r");
				while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
				{
					inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
					cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
					if (!cc && inmr_rec.hhbr_hash == coln_rec.hhbr_hash)
					{
						weight	=	(inmr_rec.weight > 0.00) ?
									inmr_rec.weight  :
									comr_rec.frt_mweight;

						tot_kgs	+= coln_rec.q_order * weight;
					}

					cc = find_rec (coln, &coln_rec, NEXT, "r");
				}
			}
			
			cc = find_rec (trln, &trln_rec, NEXT, "r");
		}
	}

	if (tot_kgs == 0.00)
		tot_kgs = 1.00;

	strcpy (trhr_rec.trip_name , tripNumber); 
	cc = find_rec (trhr, &trhr_rec, EQUAL, "r");
	if (!cc && !strcmp (trhr_rec.trip_name, tripNumber))
	{
		trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
		trln_rec.hhco_hash = 0L;
		trln_rec.hhit_hash = 0L;
		cc = find_rec (trln, &trln_rec, GTEQ, "r");
		while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			cohr_rec.hhco_hash = trln_rec.hhco_hash;
			cc = find_rec (cohr, &cohr_rec, EQUAL, "u");
			if (!cc && cohr_rec.hhco_hash == trln_rec.hhco_hash)
			{
				tot_freight	= 0.00;
				coln_rec.hhco_hash = cohr_rec.hhco_hash;
				coln_rec.line_no   = 0;
				cc = find_rec (coln, &coln_rec, GTEQ, "u");
				while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
				{
					inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
					cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
					if (!cc && inmr_rec.hhbr_hash == coln_rec.hhbr_hash)
					{
						weight	=	(inmr_rec.weight > 0.00) ?
									inmr_rec.weight  :
									comr_rec.frt_mweight;

						if (trhr_rec.fr_chg == 0.00)
							coln_rec.freight = 0.00;
						else
							coln_rec.freight = (double) ((coln_rec.q_order 
											 * weight)
											 /	tot_kgs)
											 *  trhr_rec.fr_chg;

						cc = abc_update (coln, &coln_rec);
						if (cc)
							file_err (cc, "coln", "DBUPDATE");

						tot_freight	+= coln_rec.freight;
					}
					abc_unlock (coln);

					cc = find_rec (coln, &coln_rec, NEXT, "u");
				}
				
				if (envVarSoFrChg == 1)
					cohr_rec.freight = tot_freight + trhr_rec.fr_zchg;
				else
				{
					if (envVarSoFrChg == 2)
						cohr_rec.freight = 0.00;
				}
				cc = abc_update (cohr, &cohr_rec);
				if (cc)
					file_err (cc, "cohr", "DBUPDATE");
				
				abc_unlock (cohr);
			}
			cc = find_rec (trln, &trln_rec, NEXT, "r");
		}
	}
	abc_selfield (cohr, "cohr_hhtr_hash");
	abc_selfield (trhr, "trhr_id_no");
}

/*===================================================================
| Get available deliveries for a zone, delivery date and time slot. |
===================================================================*/
static	int
GetAvailableDeliveries (
 long	trzmHash, 
 long	deliveryDate, 
 char	*deliverySlot)
{
	int		noDeliveries	=	0;
	
	trsh_rec.trzm_hash	=	trzmHash;
	trsh_rec.del_date	=	deliveryDate;
	sprintf (trsh_rec.sdel_slot, "%-1.1s", deliverySlot);
	cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
	while (!cc && trsh_rec.trzm_hash == trzmHash &&
				  trsh_rec.del_date	== deliveryDate &&
				  trsh_rec.sdel_slot [0] == deliverySlot [0])
	{
		noDeliveries++;
		cc = find_rec (trsh, &trsh_rec, NEXT, "r");
	}
	return (noDeliveries);
}
