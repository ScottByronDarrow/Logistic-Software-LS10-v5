/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_rollup.c,v 5.3 2001/08/09 09:19:48 scott Exp $
|  Program Name  : ( sk_rollup.c   )                                  |
|  Program Desc  : ( Standard Cost Rollup Calculation Program     )   |
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 24/03/94         |
|---------------------------------------------------------------------|
| $Log: sk_rollup.c,v $
| Revision 5.3  2001/08/09 09:19:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:44  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:28  scott
| Update - LS10.5
|
| Revision 4.1  2001/03/21 00:25:02  scott
| Updated to change message and performed general code clean up.
|
| Revision 4.0  2001/03/09 02:38:44  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/16 07:25:40  scott
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_rollup.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_rollup/sk_rollup.c,v 5.3 2001/08/09 09:19:48 scott Exp $";

#include	<pslscr.h>
#include	<ml_sk_mess.h>
#include	<ml_std_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<number.h>
#include	<twodec.h>

#include	"schema"

#define		TD		twodec

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct bmmsRecord	bmms_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct ineiRecord	inei_rec;
struct inemRecord	inem_rec;
struct inumRecord	inum_rec;
struct rgrsRecord	rgrs_rec;

	char	*data	= "data",
			*inmr2	= "inmr2";

int		LOWER_LEVELS;				/* Update lower manufactured items. */
struct {
	double	directLabour 	[3];	/* Variable Labour costs 	*/
	double	directMachine 	[3];	/* Variable Machine costs 	*/
	double	directQcCheck 	[3];	/* Variable QC-Check costs 	*/
	double	directSpecial 	[3];	/* Variable Special costs 	*/
	double	directOther 	[3];	/* Variable Other costs 	*/
	double	fixedLabour 	[3];	/* Fixed Labour costs 		*/
	double	fixedMachine 	[3];	/* Fixed Machine costs 		*/
	double	fixedQcCheck 	[3];	/* Fixed QC-Check costs 	*/
	double	fixedSpecial 	[3];	/* Fixed Special costs 		*/
	double	fixedOther 		[3];	/* Fixed Other costs 		*/
} reqRec, batchRec;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	startItem [17];
	char	trueStart [17];
	char	startDesc [41];
	char	endItem [17];
	char	trueEnd [17];
	char	endDesc [41];
	int		bomNumber;
	int		routeNumber;
	char 	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item                 ", "Start Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startDesc",	 5, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item Description     ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startDesc},
	{1, LIN, "endItem",	 6, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", "~~~~~~~~~~~~~~~~", " End Item                   ", "End Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endDesc",	 7, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " End Item Description       ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endDesc},
	{1, LIN, "bomNumber",	9, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " BOM Number                 ", "Default BOM Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.bomNumber},
	{1, LIN, "routeNumber",	10, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " Routing Number             ", "Default Routing Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.routeNumber},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ProcStdRollup 		(void);
void 	InitStruct 			(void);
void 	CalcResources 		(long, float);
void 	Update 				(long, double);
double 	UpdateMfgItem 		(long, int, float, float);
double 	CalcMaterials 		(long, float);
float 	GetUom 				(long);
int  	spec_valid 			(int);
int  	heading 			(int);

extern	int		manufacturingSrch;
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv[])
{
	int		i;

	manufacturingSrch	=	TRUE;

	SETUP_SCR (vars);

	OpenDB ();

	init_scr 	();			/*  sets terminal from termcap	*/
	set_tty 	();
	set_masks 	();			/*  setup print using masks	*/
	init_vars 	(1);		/*  set default values		*/

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		crsr_on ();

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		/*-------------------------------------------------
		| Update the lower level manufactured components. |
		-------------------------------------------------*/
		crsr_on ();
		i = prmptmsg (ML(mlSkMess499),"YyNnAa", 1, 16);
		if (i == 'Y' || i == 'y')
			LOWER_LEVELS = TRUE;
		else
		if (i == 'N' || i == 'n')
			LOWER_LEVELS = FALSE;
		else
		if (i == 'A' || i == 'a')
			continue;
	
		clear ();
		crsr_off ();

		ProcStdRollup ();
	}
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
	CloseDB  ();
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

	abc_alias (inmr2, inmr);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (bmms, bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (rghr, rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln, rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (inem, inem_list, INEM_NO_FIELDS, "inem_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (bmms);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (inei);
	abc_fclose (inum);
	abc_fclose (rgrs);
	abc_fclose (inem);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startItem"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startDesc, "%-40.40s", ML ("First Manufactured Item"));
			memset ((char *)local_rec.trueStart,0,sizeof (local_rec.trueStart));
			DSP_FLD ("startDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess(ML(mlSkMess531));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.trueStart, local_rec.startItem);
		strcpy (local_rec.startDesc, inmr_rec.description);
		DSP_FLD ("startDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (dflt_used)
		{
			memset ((char *)local_rec.trueEnd,0xff,sizeof (local_rec.trueEnd));
			sprintf (local_rec.endDesc, "%-40.40s", ML ("Last Manufactured Item"));
			DSP_FLD ("endDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess(ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("endDesc");

			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess(ML(mlSkMess531));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("endDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.trueEnd, local_rec.endItem);
		strcpy (local_rec.endDesc, inmr_rec.description);
		DSP_FLD ("endDesc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*-------------------------------------------------------------
| Process selected items. If the item is a manufactured item, |
| calculate the standard cost.                                |
-------------------------------------------------------------*/
void
ProcStdRollup (
 void)
{
	dsp_screen (" Calculating Standard Costs For Manufactured Item's ",
		comm_rec.co_no,
		comm_rec.co_name);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.trueStart);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	/* Read for all the items within the users selection. */
	while (!cc &&
		strcmp (inmr_rec.item_no, local_rec.trueStart) >= 0 &&
		strcmp (inmr_rec.item_no, local_rec.trueEnd) <= 0)
	{
		/*------------------------------------------------------------------
		| Calculate the standard cost if the items is a manufactured item. |
		------------------------------------------------------------------*/
		if (!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP"))
		{
			inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_batch = 1.00;

			UpdateMfgItem 
			(
				inmr_rec.hhbr_hash,
				TRUE,
				inei_rec.std_batch,
				1.00
			);
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

/*----------------------------------------------
| Calculate the total standard cost of the     |
| manufactured item, down to the lowest level. |
----------------------------------------------*/
double 
UpdateMfgItem (
	long 	hhbrHash, 
	int 	updateFlag, 
	float 	requiredQuantity, 
	float 	conversionFactor)
{
	int		i;
	double	materialCost = 0.00;			/* Total Material Costs */

	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash 	= hhbrHash;
	bmms_rec.alt_no 	= local_rec.bomNumber;
	bmms_rec.line_no 	= 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (cc || strcmp (bmms_rec.co_no, inmr_rec.co_no) ||
			  bmms_rec.hhbr_hash != hhbrHash ||
			  bmms_rec.alt_no != local_rec.bomNumber)
		  return (-1.00);

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.routeNumber;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
		return (-1.00);
	
	materialCost = CalcMaterials (hhbrHash, conversionFactor);

	InitStruct ();

	CalcResources (hhbrHash, requiredQuantity);

	if (updateFlag)
		Update (hhbrHash, materialCost / conversionFactor);

	for (i = 0; i < 3; i ++)
	{
		materialCost +=	reqRec.directLabour 	 [i];
		materialCost +=	reqRec.directMachine  [i];
		materialCost +=	reqRec.directQcCheck [i];
		materialCost +=	reqRec.directSpecial  [i];
		materialCost +=	reqRec.directOther    [i];
		materialCost +=	reqRec.fixedLabour   [i];
		materialCost +=	reqRec.fixedMachine  [i];
		materialCost +=	reqRec.fixedQcCheck [i];
		materialCost +=	reqRec.fixedSpecial  [i];
		materialCost +=	reqRec.fixedOther    [i];
	}

	return (materialCost);
}

/*--------------------------------------------
| Initialise the required quantity and batch |
| quantity record variables for standard and |
| actual breakdown costs for setup, run and  |
| clean times.                               |
--------------------------------------------*/
void
InitStruct (void)
{
	int		i;

	for (i = 0; i < 3; i ++)
	{
		reqRec.directLabour		[i] 	= 0.00;
		reqRec.directMachine	[i] 	= 0.00;
		reqRec.directQcCheck   	[i] 	= 0.00;
		reqRec.directSpecial    [i] 	= 0.00;
		reqRec.directOther      [i] 	= 0.00;
		reqRec.fixedLabour     	[i] 	= 0.00;
		reqRec.fixedMachine    	[i] 	= 0.00;
		reqRec.fixedQcCheck   	[i] 	= 0.00;
		reqRec.fixedSpecial    	[i] 	= 0.00;
		reqRec.fixedOther      	[i] 	= 0.00;

		batchRec.directLabour   [i] 	= 0.00;
		batchRec.directMachine  [i] 	= 0.00;
		batchRec.directQcCheck 	[i] 	= 0.00;
		batchRec.directSpecial  [i] 	= 0.00;
		batchRec.directOther    [i] 	= 0.00;
		batchRec.fixedLabour   	[i] 	= 0.00;
		batchRec.fixedMachine  	[i] 	= 0.00;
		batchRec.fixedQcCheck 	[i] 	= 0.00;
		batchRec.fixedSpecial  	[i] 	= 0.00;
		batchRec.fixedOther    	[i] 	= 0.00;
	}
}

/*---------------------------------------------------
| Calculates the total material costs at all levels |
| of the manufactured item passed.                  |
| Returns the total material cost (includes lower   |
| manufactured items total costs (both material and |
| resource costs).                                  |
---------------------------------------------------*/
double 
CalcMaterials (
	long	hhbrHash, 
	float	conversionFct)
{
	double	totalMaterialCost 	= 0.00,
			tempValue 			= 0.00,
			stdCost 			= 0.00;
	float	cnvFct				= 0.00,
			reqQuantity			= 0.00,
			quantity			= 0.00;
	int		lineNumber			= 0,
			flag				= 0;

	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash 	= hhbrHash;
	bmms_rec.alt_no 	= local_rec.bomNumber;
	bmms_rec.line_no 	= 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while (!cc &&	/* calculate material costs */
		!strcmp (bmms_rec.co_no, inmr_rec.co_no) &&
		bmms_rec.hhbr_hash == hhbrHash &&
		bmms_rec.alt_no == local_rec.bomNumber)
	{
		flag = FALSE;
		inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		if (!strcmp (inmr2_rec.source, "BP") ||
			!strcmp (inmr2_rec.source, "BM") ||
			!strcmp (inmr2_rec.source, "MC") ||
			!strcmp (inmr2_rec.source, "MP"))
		{
			lineNumber = bmms_rec.line_no;

			if (LOWER_LEVELS)
			{
				inei_rec.hhbr_hash = bmms_rec.mabr_hash;
				strcpy (inei_rec.est_no, comm_rec.est_no);
				cc = find_rec (inei, &inei_rec, COMPARISON, "r");
				if (cc)
					inei_rec.std_batch = 1.00;

				UpdateMfgItem (bmms_rec.mabr_hash,
						LOWER_LEVELS,
						inei_rec.std_batch,
						1.00);

				strcpy (bmms_rec.co_no, comm_rec.co_no);
				bmms_rec.hhbr_hash 	= hhbrHash;
				bmms_rec.alt_no 	= local_rec.bomNumber;
				bmms_rec.line_no 	= lineNumber;
				cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
				if (cc)
					file_err (cc, bmms, "DBFIND");
			}

			/* calculate required quantity in the std uom */
			cnvFct = GetUom (bmms_rec.uom);
			reqQuantity = bmms_rec.matl_wst_pc;
			reqQuantity += 100.00;
			reqQuantity /= 100.00;
			reqQuantity *= bmms_rec.matl_qty;
			reqQuantity /= cnvFct; /* divid by cnvFct to get std uom qty */

			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_batch = 1.00;

			/* calc conversion factor for quantity required */
			cnvFct = reqQuantity;
			cnvFct /= inei_rec.std_batch;

			stdCost =	UpdateMfgItem 	
						(
							bmms_rec.mabr_hash,
							FALSE,
							reqQuantity,
							cnvFct
						);
			if (stdCost < 0.00)
				flag = FALSE;
			else
				flag = TRUE;

			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash 	= hhbrHash;
			bmms_rec.alt_no 	= local_rec.bomNumber;
			bmms_rec.line_no 	= lineNumber;
			cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, bmms, "DBFIND");

			inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inmr2, "DBFIND");

			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_batch = 1.00;
		}
		else
		{
			/*---------------------------------
			| Calculate the std cost for this |
			| item.                           |
			---------------------------------*/
			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_cost = 0.00;
		}

		bmms_rec.matl_wst_pc += 100.00;
		bmms_rec.matl_wst_pc /= 100.00;
		quantity = bmms_rec.matl_qty * bmms_rec.matl_wst_pc;
		quantity *= conversionFct;

		if (!flag)
		{
			cnvFct = GetUom (bmms_rec.uom);
			inei_rec.std_cost /= cnvFct;

			tempValue = out_cost (inei_rec.std_cost, inmr2_rec.outer_size);
			tempValue *= quantity;
		}
		else
			tempValue = stdCost;

		totalMaterialCost += tempValue;

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}
	return (totalMaterialCost);
}

float 
GetUom (
	long	hhumHash)
{
	char	std_group [21],
			alt_group [21];
	number	std_cnv_fct,
			alt_cnv_fct,
			cnv_fct,
			result,
			uom_cfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	inmr2_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	hhumHash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr2_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

/*---------------------------------------------------
| Calculates the total resource costs at all levels |
| of the manufactured item passed.                  |
| Returns the total resource cost for the item hash |
| passed.                                           |
---------------------------------------------------*/
void
CalcResources (
 long hhbrHash, 
 float _req_qty)
{
	double	reqCost [3];	/* 0 - setup cost 1 - run cost 2 - clean cost */ 
	double	reqOvhd [3];
	double	bchCost;		/* std batch run variable cost */
	double	bchOvhd;		/* std batch run overhead cost */
	float	runTime;

	inei_rec.hhbr_hash = hhbrHash;
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
		inei_rec.std_batch = 1.00;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.routeNumber;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (!cc)
	{
		rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgln_rec.seq_no = 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		while (!cc &&
			rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
		{
			/* Total run standard cost for the resource. */
			bchCost = (double) rgln_rec.run * rgln_rec.rate;
			bchCost /= 60.00;
			bchCost *= (double) rgln_rec.qty_rsrc;

			/* divid the fixed overhead costs by three for */
			/* the setup, run and clean cost calculations. */
			rgln_rec.ovhd_fix /= 3.00;

			/* Total run overhead cost for the resource. */
			bchOvhd = (double) rgln_rec.run * rgln_rec.ovhd_var;
			bchOvhd /= 60.00;
			bchOvhd += (double) rgln_rec.ovhd_fix;
			bchOvhd *= (double) rgln_rec.qty_rsrc;

			/* Time required for the required resource amount. */
			runTime  = (float) (rgln_rec.run);
			runTime *= _req_qty;
			runTime /= inei_rec.std_batch;

			/* Total standard costs for the required resource amount. */
			reqCost [0] = (double) rgln_rec.setup * rgln_rec.rate;
			reqCost [0] /= 60.00;
			reqCost [0] *= (double) rgln_rec.qty_rsrc;
			reqCost [1] = (double) runTime * rgln_rec.rate;
			reqCost [1] /= 60.00;
			reqCost [1] *= (double) rgln_rec.qty_rsrc;
			reqCost [2] = (double) rgln_rec.clean * rgln_rec.rate;
			reqCost [2] /= 60.00;
			reqCost [2] *= (double) rgln_rec.qty_rsrc;

			/* Total overhead costs for the required resource amount. */
			reqOvhd [0] = (double) rgln_rec.setup * rgln_rec.ovhd_var;
			reqOvhd [0] /= 60.00;
			reqOvhd [0] += (double) rgln_rec.ovhd_fix;
			reqOvhd [0] *= (double) rgln_rec.qty_rsrc;
			reqOvhd [1] = (double) runTime * rgln_rec.ovhd_var;
			reqOvhd [1] /= 60.00;
			reqOvhd [1] += (double) rgln_rec.ovhd_fix;
			reqOvhd [1] *= (double) rgln_rec.qty_rsrc;
			reqOvhd [2] = (double) rgln_rec.clean * rgln_rec.ovhd_var;
			reqOvhd [2] /= 60.00;
			reqOvhd [2] += (double) rgln_rec.ovhd_fix;
			reqOvhd [2] *= (double) rgln_rec.qty_rsrc;

			/* check resource type, and add to appropriate total cost */
			rgrs_rec.hhrs_hash	=	rgln_rec.hhrs_hash;
			cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
			if (!cc)
			{
				switch (rgrs_rec.type [0])
				{
				case	'L': /* Labour */
					reqRec.directLabour 	[0] += DOLLARS (reqCost [0]);
					reqRec.fixedLabour 		[0] += DOLLARS (reqOvhd [0]);
					reqRec.directLabour 	[1] += DOLLARS (reqCost [1]);
					reqRec.fixedLabour 		[1] += DOLLARS (reqOvhd [1]);
					reqRec.directLabour 	[2] += DOLLARS (reqCost [2]);
					reqRec.fixedLabour 		[2] += DOLLARS (reqOvhd [2]);
					batchRec.directLabour 	[0] += DOLLARS (reqCost [0]);
					batchRec.fixedLabour 	[0] += DOLLARS (reqOvhd [0]);
					batchRec.directLabour 	[1] += DOLLARS (bchCost);
					batchRec.fixedLabour 	[1] += DOLLARS (bchOvhd);
					batchRec.directLabour 	[2] += DOLLARS (reqCost [2]);
					batchRec.fixedLabour 	[2] += DOLLARS (reqOvhd [2]);
					break;
				case	'M': /* Machine */
					reqRec.directMachine 	[0] += DOLLARS (reqCost [0]);
					reqRec.fixedMachine 	[0] += DOLLARS (reqOvhd [0]);
					reqRec.directMachine 	[1] += DOLLARS (reqCost [1]);
					reqRec.fixedMachine 	[1] += DOLLARS (reqOvhd [1]);
					reqRec.directMachine 	[2] += DOLLARS (reqCost [2]);
					reqRec.fixedMachine 	[2] += DOLLARS (reqOvhd [2]);
					batchRec.directMachine 	[0] += DOLLARS (reqCost [0]);
					batchRec.fixedMachine 	[0] += DOLLARS (reqOvhd [0]);
					batchRec.directMachine 	[1] += DOLLARS (bchCost);
					batchRec.fixedMachine 	[1] += DOLLARS (bchOvhd);
					batchRec.directMachine 	[2] += DOLLARS (reqCost [2]);
					batchRec.fixedMachine 	[2] += DOLLARS (reqOvhd [2]);
					break;
				case	'Q': /* QC-Check */
					reqRec.directQcCheck 	[0] += DOLLARS (reqCost [0]);
					reqRec.fixedQcCheck 	[0] += DOLLARS (reqOvhd [0]);
					reqRec.directQcCheck 	[1] += DOLLARS (reqCost [1]);
					reqRec.fixedQcCheck 	[1] += DOLLARS (reqOvhd [1]);
					reqRec.directQcCheck 	[2] += DOLLARS (reqCost [2]);
					reqRec.fixedQcCheck 	[2] += DOLLARS (reqOvhd [2]);
					batchRec.directQcCheck 	[0] += DOLLARS (reqCost [0]);
					batchRec.fixedQcCheck 	[0] += DOLLARS (reqOvhd [0]);
					batchRec.directQcCheck 	[1] += DOLLARS (bchCost);
					batchRec.fixedQcCheck 	[1] += DOLLARS (bchOvhd);
					batchRec.directQcCheck 	[2] += DOLLARS (reqCost [2]);
					batchRec.fixedQcCheck 	[2] += DOLLARS (reqOvhd [2]);
					break;
				case	'S': /* Special */
					reqRec.directSpecial 	[0] += DOLLARS (reqCost [0]);
					reqRec.fixedSpecial 	[0] += DOLLARS (reqOvhd [0]);
					reqRec.directSpecial 	[1] += DOLLARS (reqCost [1]);
					reqRec.fixedSpecial 	[1] += DOLLARS (reqOvhd [1]);
					reqRec.directSpecial 	[2] += DOLLARS (reqCost [2]);
					reqRec.fixedSpecial 	[2] += DOLLARS (reqOvhd [2]);
					batchRec.directSpecial 	[0] += DOLLARS (reqCost [0]);
					batchRec.fixedSpecial 	[0] += DOLLARS (reqOvhd [0]);
					batchRec.directSpecial 	[1] += DOLLARS (bchCost);
					batchRec.fixedSpecial 	[1] += DOLLARS (bchOvhd);
					batchRec.directSpecial 	[2] += DOLLARS (reqCost [2]);
					batchRec.fixedSpecial 	[2] += DOLLARS (reqOvhd [2]);
					break;
				case	'O': /* Other */
					reqRec.directOther 		[0] += DOLLARS (reqCost [0]);
					reqRec.fixedOther 		[0] += DOLLARS (reqOvhd [0]);
					reqRec.directOther 		[1] += DOLLARS (reqCost [1]);
					reqRec.fixedOther 		[1] += DOLLARS (reqOvhd [1]);
					reqRec.directOther 		[2] += DOLLARS (reqCost [2]);
					reqRec.fixedOther 		[2] += DOLLARS (reqOvhd [2]);
					batchRec.directOther 	[0] += DOLLARS (reqCost [0]);
					batchRec.fixedOther 	[0] += DOLLARS (reqOvhd [0]);
					batchRec.directOther 	[1] += DOLLARS (bchCost);
					batchRec.fixedOther 	[1] += DOLLARS (bchOvhd);
					batchRec.directOther 	[2] += DOLLARS (reqCost [2]);
					batchRec.fixedOther 	[2] += DOLLARS (reqOvhd [2]);
					break;
				}
			}
			cc = find_rec (rgln, &rgln_rec, NEXT, "r");
		}
	}
}

void
Update (
	long 	hhbrHash, 
	double 	materialCost)
{
	float	cnvFct	=	0.00;
	int		i;

	inmr2_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr2, "DBFIND");

	dsp_process ("Item Number :", inmr2_rec.item_no);

	inei_rec.hhbr_hash = hhbrHash;
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, inei, "DBFIND");

	cnvFct = inmr2_rec.outer_size / inei_rec.std_batch;

	/* update standard cost */
	inei_rec.std_cost = TD (materialCost * cnvFct);
	for (i = 0; i < 3; i ++)
	{
		inei_rec.std_cost	+= TD (batchRec.directLabour 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.directMachine 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.directQcCheck 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.directSpecial 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.directOther 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.fixedLabour 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.fixedMachine 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.fixedQcCheck 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.fixedSpecial 	[i]	* cnvFct);
		inei_rec.std_cost	+= TD (batchRec.fixedOther 		[i]	* cnvFct);
	}
	inei_rec.std_cost = TD (inei_rec.std_cost);

	/* update last bom and last rtg used */
	inei_rec.last_bom		= local_rec.bomNumber;
	inei_rec.last_rtg		= local_rec.routeNumber;

	cc = abc_update (inei, &inei_rec);
	if (cc)
		file_err (cc, inei, "DBUPDATE");

	/* update standard cost breakdown records */
	for (i = 0; i < 6; i ++)
	{
		inem_rec.hhbr_hash			= hhbrHash;
		strcpy (inem_rec.est_no,	comm_rec.est_no);

		switch (i)
		{
			case	0:
				strcpy (inem_rec.res_type, "T"); /* materials */
				break;

			case	1:
				strcpy (inem_rec.res_type, "L"); /* labour */
				break;

			case	2:
				strcpy (inem_rec.res_type, "M"); /* materials */
				break;

			case	3:
				strcpy (inem_rec.res_type, "Q"); /* qc-check */
				break;

			case	4:
				strcpy (inem_rec.res_type, "S"); /* special */
				break;
			case	5:
				strcpy (inem_rec.res_type, "O"); /* other */
				break;
		}
		cc = find_rec (inem, &inem_rec, EQUAL, "u");

		switch (i)
		{
			case	0:
			strcpy (inem_rec.res_type, "T"); /* materials */
			inem_rec.setup_std_dir	= TD (materialCost * cnvFct);
			break;

			case	1:
			strcpy (inem_rec.res_type, "L"); /* labour */
			inem_rec.setup_std_dir	= TD (batchRec.directLabour [0] * cnvFct);
			inem_rec.setup_std_fix	= TD (batchRec.fixedLabour  [0] * cnvFct);
			inem_rec.run_std_dir	= TD (batchRec.directLabour [1] * cnvFct);
			inem_rec.run_std_fix	= TD (batchRec.fixedLabour  [1] * cnvFct);
			inem_rec.clean_std_dir	= TD (batchRec.directLabour [2] * cnvFct);
			inem_rec.clean_std_fix	= TD (batchRec.fixedLabour  [2] * cnvFct);
			break;
		
			case	2:
			strcpy (inem_rec.res_type, "M"); /* materials */
			inem_rec.setup_std_dir	= TD (batchRec.directMachine [0] * cnvFct);
			inem_rec.setup_std_fix	= TD (batchRec.fixedMachine  [0] * cnvFct);
			inem_rec.run_std_dir	= TD (batchRec.directMachine [1] * cnvFct);
			inem_rec.run_std_fix	= TD (batchRec.fixedMachine  [1] * cnvFct);
			inem_rec.clean_std_dir	= TD (batchRec.directMachine [2] * cnvFct);
			inem_rec.clean_std_fix	= TD (batchRec.fixedMachine  [2] * cnvFct);
			break;
		
			case	3:
			strcpy (inem_rec.res_type, "Q"); /* qc-check */
			inem_rec.setup_std_dir	= TD (batchRec.directQcCheck [0] * cnvFct);
			inem_rec.setup_std_fix	= TD (batchRec.fixedQcCheck  [0] * cnvFct);
			inem_rec.run_std_dir	= TD (batchRec.directQcCheck [1] * cnvFct);
			inem_rec.run_std_fix	= TD (batchRec.fixedQcCheck  [1] * cnvFct);
			inem_rec.clean_std_dir	= TD (batchRec.directQcCheck [2] * cnvFct);
			inem_rec.clean_std_fix	= TD (batchRec.fixedQcCheck  [2] * cnvFct);
			break;
		
			case	4:
			strcpy (inem_rec.res_type, "S"); /* special */
			inem_rec.setup_std_dir	= TD (batchRec.directSpecial [0] * cnvFct);
			inem_rec.setup_std_fix	= TD (batchRec.fixedSpecial  [0] * cnvFct);
			inem_rec.run_std_dir	= TD (batchRec.directSpecial [1] * cnvFct);
			inem_rec.run_std_fix	= TD (batchRec.fixedSpecial  [1] * cnvFct);
			inem_rec.clean_std_dir	= TD (batchRec.directSpecial [2] * cnvFct);
			inem_rec.clean_std_fix	= TD (batchRec.fixedSpecial  [2] * cnvFct);
			break;
		
			case	5:
			strcpy (inem_rec.res_type, "O"); /* other */
			inem_rec.setup_std_dir	= TD (batchRec.directOther [0] * cnvFct);
			inem_rec.setup_std_fix	= TD (batchRec.fixedOther  [0] * cnvFct);
			inem_rec.run_std_dir	= TD (batchRec.directOther [1] * cnvFct);
			inem_rec.run_std_fix	= TD (batchRec.fixedOther  [1] * cnvFct);
			inem_rec.clean_std_dir	= TD (batchRec.directOther [2] * cnvFct);
			inem_rec.clean_std_fix	= TD (batchRec.fixedOther  [2] * cnvFct);
			break;
		}

		if (cc)
		{
			if (inem_rec.setup_std_dir	== 0.00 &&
				inem_rec.setup_std_fix	== 0.00 &&
				inem_rec.run_std_dir	== 0.00 &&
				inem_rec.run_std_fix	== 0.00 &&
				inem_rec.clean_std_dir	== 0.00 &&
				inem_rec.clean_std_fix	== 0.00)
				continue;

			if ((cc = abc_add (inem, &inem_rec)))
				file_err (cc, inem, "DBADD");
		}
		else
		{
			if ((cc = abc_update (inem, &inem_rec)))
				file_err (cc, inem, "DBUPDATE");
		}
	}
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
	snorm ();
	
	rv_pr (ML(mlSkMess497), 24, 0, 1);

	box (0, 3, 80, 7);

	line_at (1,0,80);
	line_at (8,1,79);
	line_at (20,0,80);

	print_at (21,0, ML(mlStdMess038),comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML(mlStdMess039),comm_rec.est_no,comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}

