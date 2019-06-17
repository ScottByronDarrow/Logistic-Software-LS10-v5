/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_acc_lt.c,v 5.3 2001/08/09 09:17:54 scott Exp $
|  Program Name  : (sk_acc_lt.c)
|  Program Desc  : (Accumulative Lead Time Calculation Program)
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 19/05/94         |
|---------------------------------------------------------------------|
| $Log: sk_acc_lt.c,v $
| Revision 5.3  2001/08/09 09:17:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:31  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:41  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_acc_lt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_acc_lt/sk_acc_lt.c,v 5.3 2001/08/09 09:17:54 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<number.h>
#include	<twodec.h>
#include	<Costing.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct bmmsRecord	bmms_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct inumRecord	inum_rec;

	char	*data	= "data",
			*inmr2	= "inmr2";

#define	MAX_RECS	100
int		LOWER_LEVELS;					/* Update lower manufactured items. */
int		COUNT = 0;
struct {
	long	hhbr_hash;
	double	lead_time;
} upd_rec [MAX_RECS];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	st_item [17];
	char	st_desc [41];
	char	ed_item [17];
	char	ed_desc [41];
	int		bom_no;
	int		rtg_no;
	char 	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_item",	 4, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item               : ", "Start Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_item},
	{1, LIN, "st_desc",	 5, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item Description   : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "ed_item",	 6, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", "~~~~~~~~~~~~~~~~", " End Item                 : ", "End Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.ed_item},
	{1, LIN, "ed_desc",	 7, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " End Item Description     : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.ed_desc},
	{1, LIN, "bom_no",	9, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " BOM Number               : ", "Default BOM Number.",
		YES, NO, JUSTRIGHT, "0", "32767",(char *)&local_rec.bom_no},
	{1, LIN, "rtg_no",	10, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " RTG Number               : ", "Default RTG Number.",
		YES, NO, JUSTRIGHT, "0", "32767",(char *)&local_rec.rtg_no},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<FindInmr.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadCcmr 			(void);
int  	spec_valid 			(int);
void 	ProcAccLeadTime 	(void);
double 	UpdateMfgItem 		(long, int, float);
double 	CalcMaterials 		(long);
float 	GetUom 				(long);
void 	CalcResources 		(long, double *, double *, float);
void 	Update 				(long, double, double);
int  	heading 			(int);

extern	int		manufacturingSrch;
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	int		i;

	SETUP_SCR (vars);

	OpenDB ();

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		COUNT		= FALSE;
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
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
		i = prmptmsg (ML (mlSkMess470),"YyNnAa",6,16);
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

		ProcAccLeadTime ();
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
	ReadCcmr ();

	abc_alias (inmr2, inmr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (incc);
	abc_fclose (bmms);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (inum);

	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

void
ReadCcmr (
 void)
{
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	manufacturingSrch	=	TRUE;

	if (LCHECK ("st_item"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.st_desc, "%-40.40s", "First Manufactured Item");
			DSP_FLD ("st_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.st_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.st_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlSkMess419));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");
			return (EXIT_FAILURE);
		}

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML (mlSkMess531));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.st_item, local_rec.ed_item) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.st_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.st_desc, inmr_rec.description);
		DSP_FLD ("st_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ed_item"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.ed_desc, "%-40.40s", "Last Manufactured Item");
			DSP_FLD ("ed_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.ed_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.ed_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("ed_desc");

			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlSkMess419));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("ed_desc");
			return (EXIT_FAILURE);
		}

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			sprintf (err_str,
					"Item %s Is Not A Manufactured Item",
					local_rec.ed_item);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("st_desc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.st_item, local_rec.ed_item) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.ed_desc, "%-40.40s", " ");
			DSP_FLD ("ed_desc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.ed_desc, inmr_rec.description);
		DSP_FLD ("ed_desc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------------------------------
| Process selected items. If the item is a manufactured item, |
| calculate the accumulative lead time.                       |
-------------------------------------------------------------*/
void
ProcAccLeadTime (
 void)
{
	dsp_screen (" Calculating Accumulative Lead Time For Manufactured Items ",
		comm_rec.co_no,
		comm_rec.co_name);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.st_item);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	/* Read for all the items within the users selection. */
	while (!cc &&
		strcmp (inmr_rec.item_no, local_rec.st_item) >= 0 &&
		strcmp (inmr_rec.item_no, local_rec.ed_item) <= 0)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");

		if (!cc &&
			 (!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP")))
		{
			cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
			if (cc)
				ineiRec.std_batch = 1.00;

			UpdateMfgItem (inmr_rec.hhbr_hash, TRUE, ineiRec.std_batch);
		}
		
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

/*----------------------------------------------
| Calculate the accumulative lead time of the  |
| manufactured item, down to the lowest level. |
----------------------------------------------*/
double 
UpdateMfgItem (
 long _hhbr_hash, 
 int upd_flag, 
 float _req_qty)
{
	int		i;
	int		flag = TRUE;
	double	mat_lt = 0.00;		/* Total Material Lead Time */
	double	rtg_lt = 0.00;		/* Total Resource Lead Time - batch size */
	double	rtg_req = 0.00;		/* Total Resource Lead Time - required qty */

	for (i = 0; i < COUNT; i ++)
	{
		if (_hhbr_hash == upd_rec [i].hhbr_hash)
		{
			mat_lt = upd_rec [i].lead_time;
			flag = FALSE;
		}
	}

	if (flag)
	{
		strcpy (bmms_rec.co_no, comm_rec.co_no);
		bmms_rec.hhbr_hash = _hhbr_hash;
		bmms_rec.alt_no = local_rec.bom_no;
		bmms_rec.line_no = 0;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
		if (cc ||
			strcmp (bmms_rec.co_no, inmr_rec.co_no) ||
			bmms_rec.hhbr_hash != _hhbr_hash ||
			bmms_rec.alt_no != local_rec.bom_no)
		{
			return (-1.00);
		}

		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash = _hhbr_hash;
		rghr_rec.alt_no = local_rec.rtg_no;
		cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
		if (cc)
			return (-1.00);

		mat_lt = CalcMaterials (_hhbr_hash);
	}

	CalcResources (_hhbr_hash, &rtg_lt, &rtg_req, _req_qty);

	if (upd_flag)
		Update (_hhbr_hash, mat_lt, rtg_lt);

	return (twodec (mat_lt) + twodec (rtg_req));
}

/*-------------------------------------------------
| Calculates the total material lead times at all |
| levels of the manufactured item passed.         |
| Returns the total material lead time (includes  |
| lower manufactured items total costs (both      |
| material and resource costs).                   |
-------------------------------------------------*/
double 
CalcMaterials (
 long _hhbr_hash)
{
	double	total_lead_time = 0.00,
			lead_time = 0.00;
	int		line_no,
			flag;
	float	cnv_fct,
			req_qty;

	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash = _hhbr_hash;
	bmms_rec.alt_no = local_rec.bom_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (bmms_rec.co_no, inmr_rec.co_no) &&
		bmms_rec.hhbr_hash == _hhbr_hash &&
		bmms_rec.alt_no == local_rec.bom_no)
	{
		flag = TRUE;
		inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "inmr2", "DBFIND");

		if (!strcmp (inmr2_rec.source, "BP") ||
			!strcmp (inmr2_rec.source, "BM") ||
			!strcmp (inmr2_rec.source, "MC") ||
			!strcmp (inmr2_rec.source, "MP"))
		{
			line_no = bmms_rec.line_no;

			/* calculate required quantity in the std uom */
			cnv_fct = GetUom (bmms_rec.uom);
			req_qty = bmms_rec.matl_wst_pc;
			req_qty += 100.00;
			req_qty /= 100.00;
			req_qty *= bmms_rec.matl_qty;
			req_qty /= cnv_fct; /* divid by cnv_fct to get std uom qty */

			lead_time = UpdateMfgItem (bmms_rec.mabr_hash,
					LOWER_LEVELS,
					req_qty);

			if (lead_time < 0.00)
				flag = TRUE;
			else
				flag = FALSE;

			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = _hhbr_hash;
			bmms_rec.alt_no = local_rec.bom_no;
			bmms_rec.line_no = line_no;
			cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "bmms", "DBFIND");

			inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, "inmr2", "DBFIND");
		}
		
		if (flag)
		{
			/*-----------------------------------
			| Read the lead time for this item. |
			-----------------------------------*/
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
				lead_time = 0.00;
			else
				lead_time = incc_rec.lead_time;
		}

		if (lead_time > total_lead_time)
			total_lead_time = twodec (lead_time);

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	return (twodec (total_lead_time));
}

float 
GetUom (
 long _hhum_hash)
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
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.alt_uom);
	if (cc)
		file_err (cc, "inum", "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.std_uom);
	if (cc)
		file_err (cc, "inum", "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", _hhum_hash);
	if (cc)
		file_err (cc, "inum", "DBFIND");
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
| Calculates the total resource lead time at all    |
| levels of the manufactured item passed.           |
| Returns the total resource lead time for the item |
| hash  passed.                                     |
---------------------------------------------------*/
void
CalcResources (
	long	hhbrHash, 
	double *_rtg_lt, 
	double *_rtg_req, 
	float _req_qty)
{
	long	std_time = 0L,
			req_time = 0L;

	cc = FindInei (hhbrHash, comm_rec.est_no, "r");
	if (cc)
		ineiRec.std_batch = 1.00;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.rtg_no;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (!cc)
	{
		rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgln_rec.seq_no = 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		while (!cc &&
			rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
		{
			/* Total time for the standard batch size. */
			std_time += rgln_rec.setup;
			std_time += rgln_rec.run;
			std_time += rgln_rec.clean;

			/* Total time for the required quantity. */
			rgln_rec.run *= _req_qty;
			rgln_rec.run /= ineiRec.std_batch;
			req_time += rgln_rec.setup;
			req_time += rgln_rec.run;
			req_time += rgln_rec.clean;

			cc = find_rec (rgln, &rgln_rec, NEXT, "r");
		}
	}
	*_rtg_lt = ((double) std_time / 60.00) / 40.00;
	*_rtg_req = ((double) req_time / 60.00) / 40.00;
}

void
Update (
 long _hhbr_hash, 
 double _mat_lt, 
 double _rtg_lt)
{
	cc = find_hash (inmr2, &inmr2_rec, COMPARISON, "r", _hhbr_hash);
	if (cc)
		file_err (cc, "inmr2", "DBFIND");

	dsp_process ("Item Number :", inmr2_rec.item_no);

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = _hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "incc", "DBFIND");

	incc_rec.lead_time = (float) twodec (_rtg_lt);
	incc_rec.acc_mlt = (float) twodec (_mat_lt) + (float) twodec (_rtg_lt);
	incc_rec.last_bom = local_rec.bom_no;
	incc_rec.last_rtg = local_rec.rtg_no;

	if (COUNT < MAX_RECS)
	{
		upd_rec [COUNT].hhbr_hash = _hhbr_hash;
		upd_rec [COUNT].lead_time = twodec (_mat_lt);
		COUNT ++;
	}

	cc = abc_update (incc, &incc_rec);
	if (cc)
		file_err (cc, "incc", "DBUPDATE");

	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		snorm ();
		
		rv_pr (ML (mlSkMess471), 21, 0, 1);

		move (0, 1); line (80);

		box (0, 3, 80, 7);
		move (1, 8); line (79);

		move (0, 20); line (80);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,clip (comm_rec.co_short));
		strcpy (err_str,ML (mlStdMess039));
		print_at (21,40,err_str,comm_rec.est_no,clip (comm_rec.est_short));

		move (0, 22); line (80);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

