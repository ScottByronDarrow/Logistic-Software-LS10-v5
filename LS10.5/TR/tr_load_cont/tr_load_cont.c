/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: tr_load_cont.c,v 5.5 2001/10/25 07:54:32 scott Exp $
|	Program Name : (tr_load_cont.c )				  	  			  |
|	Program Desc : (Transport Container Loading. )      			  |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : 13th Dec 2000    |
|---------------------------------------------------------------------|
| $Log: tr_load_cont.c,v $
| Revision 5.5  2001/10/25 07:54:32  scott
| Updated to make changes related to container and seals.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tr_load_cont.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_load_cont/tr_load_cont.c,v 5.5 2001/10/25 07:54:32 scott Exp $";

#define LCL_SCR_WIDTH	132

/*----------------------------------------------------
| Defines the offset values used for the tag screen. |
----------------------------------------------------*/
#define	OFF_SEQ_NO		2
#define	OFF_INV_NO		10
#define	OFF_CUST_NO		23
#define	OFF_CUST_REF	31
#define	OFF_NO_PLATE	53
#define	OFF_CONTAINER	70
#define	OFF_SEAL_NO		87
#define	OFF_WEIGHT		104
#define	OFF_VOLUME		116
#define	OFF_STATUS		126
#define	OFF_HHCO_HASH	129
#define	OFF_TRAN_TYPE	141

/*----------------------------------
| Format String for tab functions. |
----------------------------------*/
char	*formatString = "  %03d     %-11.11s  %-6.6s  %-20.20s  %-15.15s  %-15.15s  %-15.15s %11.4f %9.3f  %1.1s  %010ld  %1.1s";

#include <ctype.h>
#include <pslscr.h>
#include <twodec.h>
#include <hot_keys.h>
#include <assert.h>
#include <pslscr.h>
#include <minimenu.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <tabdisp.h>

#define ERROR       1
#define SUCCESS     0

typedef	int	BOOL;

/*---------------------------
| Standard appschema stuff. |
----------------------------*/
#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cohrRecord	cohr_rec;
struct cohrRecord	cohr2_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct inuvRecord	inuv_rec;
struct skcmRecord	skcm_rec;
struct skcmRecord	skcm2_rec;
struct skndRecord	sknd_rec;
struct skniRecord	skni_rec;
struct sknhRecord	sknh_rec;

char	*transportFile 	=	"transportFile", 
		*cohr2			=	"cohr2", 
		*skcm2			=	"skcm2", 
		*data			=	"data";

FILE	*fout, 
		*fsort;

	int		noInTab 		= 0, 
			overloadWeight	= 0,
			overloadVolume	= 0;

	int		CheckValidCohr (long);

	char 	bufferData [256];

/*-------------------------
| Functions for hot keys. |
-------------------------*/
static	int	AllocRemoveFunc	(int, KEY_TAB *);
static	int	ViewFunc	 	(int, KEY_TAB *);
static	int	InputSealFunc 	(int, KEY_TAB *);
static	int	RestartFunc	 	(int, KEY_TAB *);
static	int	ExitFunc	 	(int, KEY_TAB *);
static	int	SeqChange	 	(int, KEY_TAB *);

#ifdef	GVISION
	static KEY_TAB listKeys [] =
	{
	   { " Allocate ", 		'A', 	AllocRemoveFunc, 
		"Allocate Packing Slip that is not allocated To a Container ",	"A" }, 
	   { " Remove ", 		'R', 	AllocRemoveFunc, 
		"Remove Packing Slip that is allocated or committed to a Container ","R"}, 
	   { " Input Seal ", 		'U', 	InputSealFunc, 
		"Input Seal Number",	"U" }, 
	   { " View ", 			'V', 	ViewFunc, 
		"View a Packing Slip for highlighed line", 						"A" }, 
	   { " Sequence Up ", 		'+', 	SeqChange, 
		"Add one to Load Sequence Number for Allocated packing slip", "S" }, 
	   { " Sequence Down ", 		'-', 	SeqChange, 
		"Subtract one from Load Sequence Number for Allocated packing Slip", 		"S" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};
#else
	static KEY_TAB listKeys [] =
	{
	   { " [A]llocate ", 		'A', 	AllocRemoveFunc, 
		"Allocate Packing Slip that is not allocated To a Container ",	"A" }, 
	   { " [R]emove ", 		'R', 	AllocRemoveFunc, 
		"Remove Packing Slip that is allocated or committed to a Container ","R"}, 
	   { " [U]pdate Seal ", 		'U', 	InputSealFunc, 
		"Input Seal Number",	"U" }, 
	   { " [V]iew ", 			'V', 	ViewFunc, 
		"View a Packing Slip for highlighed line", 						"A" }, 
	   { " [+]to Sequence", 		'+', 	SeqChange, 
		"Add one to Load Sequence Number for Allocated packing slip", "S" }, 
	   { " [-]from Sequence", 		'-', 	SeqChange, 
		"Subtract one from Load Sequence Number for Allocated packing Slip", 		"S" }, 
	   { NULL, 				FN1, 	RestartFunc, 
		"Exit without update.", 						"A" }, 
	   { NULL, 				FN16, 	ExitFunc, 
		"Exit and update the database.", 			"A" }, 
	   END_KEYS
	};
#endif
/*
 * Local record structure variables.
 */
struct
{
	float	weight;
	float	volume;
	float	capacityWeight;
	float	capacityVolume;
	long	firstDate;
	long	lastDate;
	long	lsystemDate;
	char	documentNumber [9];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "containerCode", 2, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Container No.       : ", "Enter Container No. [SEARCH]", 
		NE, NO, JUSTLEFT, "", "", skcm_rec.container}, 
	{1, LIN, "desc", 2, 66, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description         : ", "Enter Vehicle Description", 
		NA, NO, JUSTLEFT, "", "", skcm_rec.desc}, 
	{1, LIN, "containerSeal", 3, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Container Seal      : ", "Enter Container Seal", 
		ND, NO, JUSTLEFT, "", "", skcm_rec.last_seal}, 
	{1, LIN, "docNumber", 4, 2, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"", " ",  "Document Number     : ", "Enter packing slip or Collection Note number <default all> ", 
		NO, NO, JUSTRIGHT, "", "", local_rec.documentNumber}, 
	{1, LIN, "firstDate", 	5,  2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ",   "First Required Date : ", " Enter Start of Required Date.", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.firstDate}, 
	{1, LIN, "lastDate", 	5, 66, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Last  Required Date : ", " Enter End Required Date.", 
		 NO, NO,  JUSTLEFT, " ", "", (char *)&local_rec.lastDate}, 
	{1, LIN, "capacity_weight", 	 6, 2, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "  ", "Capacity Weight     : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.capacityWeight}, 
	{1, LIN, "capacity_volume", 	 6, 66, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "  ", "Capacity Volume     : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.capacityVolume}, 
	{1, LIN, "weight", 	 7, 2, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", "  ", "Weight Allocated    : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.weight}, 
	{1, LIN, "volume", 	 7, 66, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", "  ", "Volume Allocated    : ", "", 
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.volume}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * The structure capacity groups the values together. 
 */
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

/*
 * Function Declarations.
 */
int 	LoadAllocated 				(void);
int 	LoadContainers 				(void);
int 	TestTag 					(int, char *);
int 	heading 					(int);
int 	spec_valid 					(int);
static	BOOL	IsSpaces			(char *);
void 	AllocateContainer 			(long, int, char *);
void 	CalculateCapacity 			(long);
void 	ClearCapacity 				(void);
void 	CloseDB 					(void);
void 	LoadCohrRecords 			(char *);
void 	OpenDB 						(void);
void 	Process 					(void);
void 	RemoveContainer 			(long, int);
void 	DocumentFind 				(char *, char *, char *);
void 	SrchDocument 				(char *);
void 	SrchSkcm 					(char *);
void 	Update 						(void);
void 	heading2 					(void);
void 	shutdown_prog 				(void);

/*============================
|	Main Processing Routine  |
============================*/
int
main (
	int 	argc, 
	char 	*argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	set_masks 	 ();
	OpenDB		 ();
	clear		 ();

	local_rec.lsystemDate = TodaysDate ();
	
	while (!prog_exit)
	{
		search_ok 		= 	TRUE;
		entry_exit 		= 	FALSE;
		edit_exit 		= 	FALSE;
		prog_exit 		= 	FALSE;
		restart 		= 	FALSE;
		init_ok 		= 	TRUE;
		overloadWeight	=	0;
		overloadVolume	=	0;
		init_vars 	 (1);	
		heading 	 (1);
		entry 		 (1);   
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
	clear		();
  	snorm		();  
	print_at (0,0,ML (mlStdMess035));
	CloseDB		();
	rset_tty	();
	crsr_on		();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	abc_alias (cohr2, cohr);
	abc_alias (skcm2, skcm);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (cohr,  cohr_list, COHR_NO_FIELDS, "cohr_id_no2"); 
	open_rec (cohr2, cohr_list, COHR_NO_FIELDS, "cohr_id_no2"); 
	open_rec (coln,  coln_list, COLN_NO_FIELDS, "coln_id_no"); 
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash"); 
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash"); 
	open_rec (inuv,  inuv_list, INUV_NO_FIELDS, "inuv_id_no"); 
	open_rec (sknd,  sknd_list, SKND_NO_FIELDS, "sknd_sknd_hash"); 
	open_rec (skni,  skni_list, SKNI_NO_FIELDS, "skni_hhcl_hash"); 
	open_rec (sknh,  sknh_list, SKNH_NO_FIELDS, "sknh_sknh_hash"); 
	open_rec (skcm,  skcm_list, SKCM_NO_FIELDS, "skcm_id_no"); 
	open_rec (skcm2, skcm_list, SKCM_NO_FIELDS, "skcm_skcm_hash"); 
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
	abc_fclose (inmr);
	abc_fclose (inuv);
	abc_fclose (sknd);
	abc_fclose (skni);
	abc_fclose (sknh);
	abc_fclose (skcm);
	abc_fclose (skcm2);
	abc_dbclose (data);
}

/*===================
|  Field Validation |
===================*/
int
spec_valid (
	int		field)
{
	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("containerCode"))
	{
		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (SUCCESS);
		}
		if (dflt_used)
		{ 
			if (IsSpaces (skcm_rec.container))
			{
				/*-----------------------------
				| Container Number not found. |
				-----------------------------*/
				print_mess (ML (mlTrMess083));
				sleep (sleepTime);
				clear_mess ();
				return (ERROR);
			}
		}
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		cc = find_rec (skcm, &skcm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlTrMess083));
			sleep (sleepTime);
			clear_mess ();
			return (ERROR);
		}
		strcpy (skcm_rec.last_seal, " ");
		local_rec.capacityWeight 	= 	skcm_rec.max_wgt;
		local_rec.capacityVolume 	= 	skcm_rec.max_vol;

		DSP_FLD ("containerSeal");
       	DSP_FLD ("containerCode");
       	DSP_FLD ("desc");
		DSP_FLD ("capacity_weight");
		DSP_FLD ("capacity_volume");
		return (SUCCESS);
	}
	/*----------------------
	| Validate first date. |
	----------------------*/
	if (LCHECK ("firstDate"))
	{
		if (dflt_used) 
			local_rec.firstDate = 0L;

		if (local_rec.firstDate > local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess068));
			sleep	 (2);
			return 	 (ERROR);
		}
		DSP_FLD ("firstDate");
		return (SUCCESS);
	}
	/*---------------------
	| Validate Last date. |
	---------------------*/
	if (LCHECK ("lastDate"))
	{
		if (dflt_used) 
			local_rec.lastDate = local_rec.lsystemDate;

		if (local_rec.lastDate < local_rec.firstDate)
		{
			print_mess (ML (mlStdMess026));
			sleep 	 (2);
			return 	 (ERROR);
		}
		
		if (local_rec.lastDate > local_rec.lsystemDate)
		{
			print_mess (ML (mlStdMess013));
			sleep	 (2);
			return 	 (ERROR);
		}

		DSP_FLD ("lastDate");
		return (SUCCESS);
	}

	/*---------------------------
	| Validate Document Number.	|
	---------------------------*/
	if (LCHECK ("docNumber")) 
	{
		if (FLD ("docNumber") == NA)
			return (SUCCESS);

		if (dflt_used && !strlen (clip (local_rec.documentNumber)))
		{
			strcpy (local_rec.documentNumber, "        ");
			return (SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchDocument (temp_str);
			return (SUCCESS);
		}

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (cohr2_rec.co_no, comm_rec.co_no);
		strcpy (cohr2_rec.br_no, comm_rec.est_no);
		strcpy (cohr2_rec.type, "T");
		sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
		cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		if (cc) 
		{
			strcpy (cohr2_rec.co_no, comm_rec.co_no);
			strcpy (cohr2_rec.br_no, comm_rec.est_no);
			strcpy (cohr2_rec.type, "P");
			sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
			cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		}
		if (cc) 
		{
			strcpy (cohr2_rec.co_no, comm_rec.co_no);
			strcpy (cohr2_rec.br_no, comm_rec.est_no);
			strcpy (cohr2_rec.type, "N");
			sprintf (cohr2_rec.inv_no, local_rec.documentNumber);
			cc = find_rec ("cohr2", &cohr2_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlTrMess035));
			sleep (sleepTime);
			return (ERROR); 
		}
		return (SUCCESS);
	}
	return (SUCCESS);
}

/*=============================
| Clear capacity information. |
=============================*/
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
/*=================================
| Calculate capacity information. |
=================================*/
void
CalculateCapacity (
	long	hhcoHash)
{
	capacity.volumeLine		=	0.00;
	capacity.weightLine		=	0.00;
	capacity.volume			=	0.00;
	capacity.weightItems	=	0.00;

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
	return;
}

/*==============================
| Main tab processing routine. |
==============================*/
void
Process (
 void)
{
	rv_pr (ML ("? for help on Tag Keys"), 108, 0, 1);
	/*------------
	| Open table |
	------------*/
	tab_open (transportFile, listKeys, 8, 2, 9, FALSE);

	tab_add 
	(
		transportFile, 
		"#%8.8s  %-11.11s  %-6.6s  %-20.20s  %15.15s  %15.15s  %15.15s  %10.10s  %8.8s  %1.1s ", 
		"Load Seq", 
		"Document No", 
		"Cust. ", 
		"Customer P.O. Number", 
		"Plate/Grin. No.", 
		"Container No   ", 
		"Seal Number    ", 
		"Weight", 
		"Volume", 
		"A"
	);

	/*----------------------------
	| Load Container information |
	----------------------------*/
	LoadContainers ();
	DSP_FLD ("weight");
	DSP_FLD ("volume");

	if (noInTab == 0)
	{
		tab_add (transportFile, "There Are No Valid Documents For ");
		tab_add (transportFile, "The Selected Required Date.");

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

/*==============================
| Load Container call function |
==============================*/
int
LoadContainers (
 void)
{
	noInTab = 0;
	ClearCapacity ();
	abc_selfield (cohr, "cohr_id_no2");
	LoadCohrRecords ("T");
	LoadCohrRecords ("P");
	LoadCohrRecords ("N");
	return (SUCCESS);
}

/*========================
| Load Recors from cohr. |
========================*/
void
LoadCohrRecords (
 char	*loadType)
{
	char	tranNumber	[12], 
			lastUserRef	[21], 
			lastPlateNo	[15],
			statusString [2];

	abc_selfield (cohr, "cohr_id_no2");
	abc_selfield (coln, "coln_id_no");

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
		/*--------------------------------------
		| Validate delivery and assembly date. |
		--------------------------------------*/
		if ((cohr_rec.date_required > local_rec.lastDate &&
			cohr_rec.date_required > local_rec.firstDate) ||
			cohr_rec.date_required < local_rec.firstDate) 
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*---------------------------
		| Validate document number. |
		---------------------------*/
		if ( strlen (clip (local_rec.documentNumber)))
		{
			if (strcmp (cohr_rec.inv_no, local_rec.documentNumber))
			{
				cc = find_rec (cohr, &cohr_rec, NEXT, "r");
				continue;	
			}
		}
		/*----------------
		| Read Customer. |
		----------------*/
		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (cohr, &cohr_rec, NEXT, "r");
			continue;
		}
		/*-----------------------------------------------
		| Calculate Capacity for each line on document. |
		-----------------------------------------------*/
		CalculateCapacity (cohr_rec.hhco_hash);

		if (loadType [0] == 'T' || loadType [0] == 'P')
			sprintf (tranNumber, "PS %s", cohr_rec.inv_no);
		else
			sprintf (tranNumber, "CN %s", cohr_rec.inv_no);

		strcpy (lastUserRef, " ");
		strcpy (lastPlateNo, " ");
		coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
		coln_rec.line_no	=	0;
		cc = find_rec (coln, &coln_rec, GTEQ, "r");
		while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
		{
			/*
			 * Read number plate issue detail information. |
			 */
			skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
			cc = find_rec (skni, &skni_rec, COMPARISON, "r");
			if (cc)
			{
				skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
				skni_rec.sknd_hash	=	0L;
				skni_rec.hhsl_hash	=	0L;
				strcpy (skni_rec.container, " ");
				strcpy (skni_rec.seal_no, " ");
				skni_rec.load_seq = 0;
				strcpy (skni_rec.cus_ord_ref, cohr_rec.cus_ord_ref);
				skni_rec.qty_issued = coln_rec.q_order;
				cc = abc_add (skni, &skni_rec);
				if (cc)
					file_err (cc, skni, "DBADD");

				cc = find_rec (skni, &skni_rec, COMPARISON, "r");
				if (cc)
					file_err (cc, skni, "DBFIND");
			}
			strcpy (sknh_rec.plate_no, "N/A");
			/*
			 * Find number plate detail. 
			 */
			sknd_rec.sknd_hash	=	skni_rec.sknd_hash;
			cc = find_rec (sknd, &sknd_rec, COMPARISON, "r");
			if (!cc)
			{
				/*
			 	* Read number plate header information.
			 	*/
				sknh_rec.sknh_hash	=	sknd_rec.sknh_hash;
				cc = find_rec (sknh, &sknh_rec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (coln, &coln_rec, NEXT, "r");
					continue;
				}
			}
			/*--------------------------------------------------------
			| Only add one line for each order ref and number plate. |
			--------------------------------------------------------*/
			if (strcmp (skni_rec.cus_ord_ref, lastUserRef) ||
			    strcmp (sknh_rec.plate_no, 	  lastPlateNo))
		   	{
				/*-------------------------------------------
				| Document is not allocated to a container. |
				-------------------------------------------*/
				if (!strcmp (skni_rec.container, "               "))
					strcpy (statusString, "N");
				else
				{
					/*---------------------------------------
					| Document is allocated to a container. |
					---------------------------------------*/
					strcpy (statusString, "C");
					if (!strcmp (skni_rec.container, skcm_rec.container))
					{
						local_rec.weight += capacity.weightLine;
						local_rec.volume += capacity.volumeLine;
					}
				}

				/*-----------------------------
				| Add record to tab routines. |
				-----------------------------*/
				tab_add 
				(
					transportFile, 
					formatString, 
					skni_rec.load_seq, 
					tranNumber, 
					cumr_rec.dbt_no, 
					skni_rec.cus_ord_ref, 
					sknh_rec.plate_no, 
					skni_rec.container, 
					skni_rec.seal_no, 
					capacity.weightLine, 
					capacity.volumeLine, 
					statusString,
					cohr_rec.hhco_hash, 
					cohr_rec.type 
				);
				strcpy (lastUserRef, skni_rec.cus_ord_ref);
				strcpy (lastPlateNo, sknh_rec.plate_no);
				++noInTab;

			}
			cc = find_rec (coln, &coln_rec, NEXT, "r");
		}
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
}
				
/*==================================
| Restart key processing function. |
==================================*/
static	int
RestartFunc (
 int 	key, 
 KEY_TAB *psUnused)
{
	assert (key == FN1);

	restart = TRUE;
	return key;
}

/*===============================
| Exit key processing function. |
===============================*/
static	int
ExitFunc (
 int 	key, 
 KEY_TAB *psUnused)
{
	assert (key == FN16);

	return key;
}

/*===============================
| Remove and Allocate function. |
===============================*/
static int 
AllocRemoveFunc (
 int key, 
 KEY_TAB *psUnused)
{
 	int		result;
	int		st_line;
	char	ans;
	char	tempStatus [2];

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
					vars [label ("weight")].col + 1, 	
					vars [label ("weight")].row, 
					overloadWeight
				);
			}
			else
			{
				fl_pr 
				 (
					vars [label ("weight")].prmpt, 
					vars [label ("weight")].col + 1, 	
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
					vars [label ("volume")].col + 1, 	
					vars [label ("volume")].row, 
					overloadVolume
				);
			}
			else
			{
				fl_pr 
				 (
					vars [label ("volume")].prmpt, 
					vars [label ("volume")].col + 1, 	
					vars [label ("volume")].row, 
					0
				);
			}
			tab_update
			(
				transportFile, 
				formatString, 
				0,
				bufferData + OFF_INV_NO,
				bufferData + OFF_CUST_NO,
				bufferData + OFF_CUST_REF,
				bufferData + OFF_NO_PLATE,
				skcm_rec.container,
				bufferData + OFF_SEAL_NO,
				atof (bufferData + OFF_WEIGHT),
				atof (bufferData + OFF_VOLUME),
				"A",
				atol (bufferData + OFF_HHCO_HASH),
				bufferData + OFF_TRAN_TYPE
			);
			DSP_FLD ("weight");
			DSP_FLD ("volume");
		}
		else
		{
			print_mess (ML ("Invalid key for current status"));
			sleep (sleepTime);
		}
	} 
	else if (key == 'R')
	{
		result = TestTag (st_line, "R");
		if (!result)
		{
			tab_get (transportFile, bufferData, EQUAL, st_line);
			sprintf (tempStatus, "%1.1s", bufferData + OFF_STATUS);
			if (tempStatus [0] == 'C' || tempStatus [0] == 'A')
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
						vars [label ("weight")].col + 1, 	
						vars [label ("weight")].row, 
						overloadWeight
					);
				}
				else
				{
					fl_pr 
					 (
						vars [label ("weight")].prmpt, 
						vars [label ("weight")].col + 1, 	
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
						vars [label ("volume")].col + 1, 	
						vars [label ("volume")].row, 
						overloadVolume
					);
				}
				else
				{
					fl_pr 
					 (
						vars [label ("volume")].prmpt, 
						vars [label ("volume")].col + 1, 	
						vars [label ("volume")].row, 
						0
					);
				}
			}
			tab_update
			(
				transportFile, 
				formatString, 
				0,
				bufferData + OFF_INV_NO,
				bufferData + OFF_CUST_NO,
				bufferData + OFF_CUST_REF,
				bufferData + OFF_NO_PLATE,
				" ",
				bufferData + OFF_SEAL_NO,
				atof (bufferData + OFF_WEIGHT),
				atof (bufferData + OFF_VOLUME),
				"N",
				atol (bufferData + OFF_HHCO_HASH),
				bufferData + OFF_TRAN_TYPE
			);
		}
		else
		{
			print_mess (ML ("Invalid key for current status"));
			sleep (sleepTime);
		}
	}
	else
	{
		print_mess (ML ("Invalid key for current status"));
		sleep (sleepTime);
	}
	return (key);
}
	
/*================================
| Load Sequence change function. |
================================*/
static	int 
SeqChange (
	int		key, 
	KEY_TAB *psUnused)
{
	int		seqNumber	=	0;
	int		result, 
			st_line;

	st_line = tab_tline (transportFile);

	result = TestTag (st_line, "S");
	if (!result)
	{
		tab_get (transportFile, bufferData, EQUAL, st_line);
		seqNumber	=	atoi (bufferData + OFF_SEQ_NO);
		if (key == '+')
			seqNumber++;
		else 
		{
			if (seqNumber > 0)
				seqNumber--;
		}

		tab_update
		(
			transportFile, 
			"  %03d     %-s", 
			seqNumber, 
			bufferData + OFF_INV_NO
		);
	}
	else
	{
		print_mess (ML ("Must be allocated to Container to update sequence"));
		sleep (sleepTime);
	}
	return (key);
}

/*=======================================
| Test if key pressed allowed for line. |
=======================================*/
int
TestTag (
 int	line_no, 
 char	*Tag)
{
	char	statusString [2];

	tab_get (transportFile, bufferData, EQUAL, line_no);
	sprintf (statusString, "%1.1s", bufferData + OFF_STATUS);

	if (Tag [0] == 'S' && statusString [0] == 'A')
		return (SUCCESS);

	if (Tag [0] == 'R' && statusString [0] == 'A')
		return (SUCCESS);

	if (Tag [0] == 'R' && statusString [0] == 'C')
		return (SUCCESS);

	if (Tag [0] == 'A' && (statusString [0] == 'N' || err_str [0] == 'R'))
		return (SUCCESS);

	return (ERROR);
}

/*====================================
| View document for a selected line. |
====================================*/
int
ViewFunc (
 int	key, 
 KEY_TAB *psUnused)
{
	char	runString	[100], 
			tranNumber 	[12],
			tranType 	[2];
	int		st_line;

	abc_selfield (cumr, "cumr_id_no");
	st_line = tab_tline (transportFile);
	tab_get (transportFile, bufferData, EQUAL, st_line);
	sprintf (tranNumber, "%-11.11s", bufferData + OFF_INV_NO);
	sprintf (tranType, "%-1.1s", bufferData + OFF_TRAN_TYPE);

	sprintf (cumr_rec.dbt_no, "%-6.6s", bufferData + OFF_CUST_NO);
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, comm_rec.est_no);
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "cumr", "DBFIND");
	
	sprintf 
	(
		runString, 
		"%-2.2s %-2.2s %-1.1s %010ld %-10.10s", 
		comm_rec.co_no, 
		comm_rec.est_no, 
		tranType,
		cumr_rec.hhcu_hash, 
		tranNumber + 3
	); 
	box (40, 21, 40, 1);
	rv_pr (ML (mlStdMess035), 41, 22, 1);

	if (tranType [0] == 'T' || tranType [0] == 'P')
	{
		arg [0]     = "so_sinvdisp";
		arg [1]		= "N";
		arg [2]		= "7";
		arg [3]		= "T";
		arg [4]		= "so_invoice.s";
		arg [5] = "0";
		arg [6] = runString;
		arg [7] = (char *)0;
	}
	else 
	{
		arg [0]     = "so_ccn_sdsp";
		arg [1]		= "N";
		arg [2]		= "7";
		arg [3]		= "so_ccn_inp.s";
		arg [4] = "0";
		arg [5] = runString;
		arg [6] = (char *)0;
	}
	shell_prog (2); 

	heading (1);
	scn_display (1); 
	Dsp_heading ();
	redraw_table (transportFile);
	abc_selfield (cumr, "cumr_hhcu_hash");
	return (SUCCESS);
}

/*==================
| Update function. |
==================*/
void
Update (void)
{
	int		count		=	0,
			seqNumber	=	0;
	char	currStatus [2];
	char	sealNumber [sizeof skni_rec.seal_no];
	long	hhcoHash;

	for (count = 0; count < noInTab; count++)
	{
		tab_get (transportFile, bufferData, EQUAL, count);

		hhcoHash	=	atol (bufferData + OFF_HHCO_HASH);
		seqNumber	=	atoi (bufferData + OFF_SEQ_NO);
		sprintf (sealNumber, "%-15.15s", bufferData + OFF_SEAL_NO);
		sprintf (currStatus, "%-1.1s", 	 bufferData + OFF_STATUS);
		if (currStatus [0] == 'A' || currStatus [0] == 'C')
			AllocateContainer (hhcoHash, seqNumber, sealNumber);
		else if (currStatus [0] == 'R' || currStatus [0] == 'N')
			RemoveContainer (hhcoHash, seqNumber);
	}
	return;
}

/*===========================================================
| Function - Remove container from number plate.			|
===========================================================*/
void
RemoveContainer (
	long	hhcoHash,
	int		seqNumber)
{
	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (coln, "coln_id_no");

	/*---------------------------
	| Find Packing slip header. |
	---------------------------*/
	cohr_rec.hhco_hash	=	hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*----------------------------
	| Find Packing line details. |
	----------------------------*/
	coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
	coln_rec.line_no	=	0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		/*-------------------------------------------------------------
		| Find number plate details that belong to packing slip line. |
		-------------------------------------------------------------*/
		skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		cc = find_rec (skni, &skni_rec, GTEQ, "u");
		while (!cc && skni_rec.hhcl_hash == coln_rec.hhcl_hash)
		{
			/*--------------------------------------------------------
			| Clear our contailer and seal number and update record. |
			--------------------------------------------------------*/
			strcpy (skni_rec.container, " ");
			cc = abc_update (skni, &skni_rec);
			if (cc)
				file_err (cc, skni, "DBUPDATE");

			cc = find_rec (skni, &skni_rec, NEXT, "u");
		}
		abc_unlock (skni);

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
}

/*=============================================================================
| Function - Allocate contailer and seal information to number plate details. |
=============================================================================*/
void
AllocateContainer (
	long	hhcoHash,
	int		seqNumber,
	char	*sealNumber)
{
	abc_selfield (cohr, "cohr_hhco_hash");
	abc_selfield (coln, "coln_id_no");

	/*---------------------------
	| Find Packing slip header. |
	---------------------------*/
	cohr_rec.hhco_hash	=	hhcoHash;
	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*----------------------------
	| Find Packing line details. |
	----------------------------*/
	coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
	coln_rec.line_no	=	0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		/*-------------------------------------------------------------
		| Find number plate details that belong to packing slip line. |
		-------------------------------------------------------------*/
		skni_rec.hhcl_hash	=	coln_rec.hhcl_hash;
		cc = find_rec (skni, &skni_rec, GTEQ, "u");
		while (!cc && skni_rec.hhcl_hash == coln_rec.hhcl_hash)
		{
			/*--------------------------------------------------------
			| Clear our contailer and seal number and update record. |
			--------------------------------------------------------*/
			skni_rec.load_seq	=		seqNumber;
			strcpy (skni_rec.container, skcm_rec.container);
			sprintf (skni_rec.seal_no, 	"%-15.15s", sealNumber);
			cc = abc_update (skni, &skni_rec);
			if (cc)
				file_err (cc, skni, "DBUPDATE");

			cc = find_rec (skni, &skni_rec, NEXT, "u");
		}
		abc_unlock (skni);

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
}
	
/*=========================
| Normal heading routine. |
=========================*/
int
heading (
 int scn)
{
	if (restart)
		return (SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();
	strcpy (err_str, ML (mlTrMess084));
	rv_pr (err_str, ((LCL_SCR_WIDTH - (int) strlen (err_str)) / 2), 0, 1);
	
	box (0, 1, LCL_SCR_WIDTH, 6);
	scn_set (1);
	scn_write (1);
	scn_display (1);

	line_at (3,1,LCL_SCR_WIDTH - 1);
	line_at (20,0,LCL_SCR_WIDTH);
	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

    return (SUCCESS);
}

/*==========================
| Search for p_slip number |
==========================*/
void
SrchDocument (
 char	*keyValue)
{
	work_open ();
	save_rec ("#Document No", "#Customer Name ");

	DocumentFind (keyValue, "T", "PS");
	DocumentFind (keyValue, "N", "CN");
	DocumentFind (keyValue, "P", "PS");

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (temp_str, temp_str + 3);
}

/*
 * Main search on documents.
 */
void
DocumentFind (
	char	*keyValue, 
	char	*Type, 
	char	*Code)
{
	strcpy (cohr2_rec.co_no, comm_rec.co_no);
	strcpy (cohr2_rec.br_no, comm_rec.est_no);
	sprintf (cohr2_rec.inv_no, "%-8.8s", keyValue);
	sprintf (cohr2_rec.type, Type);

	cc = find_rec (cohr2, &cohr2_rec, GTEQ, "r");
	while (!cc && !strncmp (cohr2_rec.inv_no, keyValue, strlen (keyValue)) && 
				  !strcmp (cohr2_rec.co_no, comm_rec.co_no) && 
				  !strcmp (cohr2_rec.br_no, comm_rec.est_no) &&
				  cohr2_rec.type [0] == Type [0])
	{
		if (CheckValidCohr (cohr2_rec.hhco_hash))
		{
			cumr_rec.hhcu_hash	=	cohr2_rec.hhcu_hash;
			cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, "cumr", "DBFIND");

			sprintf (err_str, "%-2.2s %s", Code, cohr2_rec.inv_no);
			cc = save_rec (err_str, cumr_rec.dbt_no);
			if (cc)
				break;
		}
		cc = find_rec (cohr2, &cohr2_rec, NEXT, "r");
	}
}

/*
 * Check if searched line is valid.
 */
int
CheckValidCohr (
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
			return (EXIT_FAILURE);

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for skcm.
 */
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
		file_err (cc, skcm, "DBFIND");
}

static BOOL
IsSpaces (
	char	*str)
{ 
	/*
	 * Return TRUE if str contains only white space or nulls
	 */
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
}
	
/*
 * Input seal number.
 */
static int
InputSealFunc (
 int        c,
 KEY_TAB *  psUnused)
{
	char	sealNumber [sizeof skni_rec.seal_no];
	char	tempStatus [2];

	int	y_pos;

	tab_get (transportFile, bufferData, CURRENT, 0);

	sprintf (tempStatus, "%1.1s", bufferData + OFF_STATUS);
	if (tempStatus [0] != 'A' && tempStatus [0] != 'C' )
	{
		print_mess (ML ("Line not allocated to container"));
		sleep (sleepTime);
		clear_mess ();
		return (ERROR);
	}
	y_pos = tab_sline (transportFile);

	sprintf (sealNumber, "%-15.15s", bufferData + OFF_SEAL_NO);
	crsr_on ();
	getalpha (OFF_SEAL_NO + 3, y_pos, "UUUUUUUUUUUUUUU", sealNumber);
	crsr_off ();

	tab_update 
	(
		transportFile, 
		formatString,
		atoi (bufferData + OFF_SEQ_NO),
		bufferData + OFF_INV_NO,
		bufferData + OFF_CUST_NO,
		bufferData + OFF_CUST_REF,
		bufferData + OFF_NO_PLATE,
		bufferData + OFF_CONTAINER,
		sealNumber,
		atof (bufferData + OFF_WEIGHT),
		atof (bufferData + OFF_VOLUME),
		bufferData + OFF_STATUS,
		atol (bufferData + OFF_HHCO_HASH),
		bufferData + OFF_TRAN_TYPE
	);
	return (c);
}
