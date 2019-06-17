/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_rf_stinp.c,v 5.3 2002/07/17 09:57:58 scott Exp $
|  Program Name  : ( sk_rf_stinp.c  )                                 |
|  Program Desc  : ( Stock Take Input - RF Terminals              )   |
|                  (                                              )   |
-----------------------------------------------------------------------
| $Log: sk_rf_stinp.c,v $
| Revision 5.3  2002/07/17 09:57:58  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:19:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:40  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:18  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:37  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:02  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 1.2  2000/09/05 00:32:32  scott
| New program
|
| Revision 1.1  2000/09/05 00:13:33  scott
| New programs to perform stock take on RF terminal.
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_rf_stinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_rf_stinp/sk_rf_stinp.c,v 5.3 2002/07/17 09:57:58 scott Exp $";

#define	MAXWIDTH	150
#define	MAXLINES	500

#include 	<pslscr.h>
#include 	<getnum.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<LocHeader.h>

#define	AUDIT		 (local_rec.audit [0] == 'Y')
#define	SK_HHBR_HASH	store [0]._hhbr_hash
#define	SK_HHUM_HASH	store [0]._hhum_hash
#define	SK_HHWH_HASH	store [0]._hhwh_hash
#define	SK_QUANTITY		store [0]._qty
#define	SK_CNV_FCT		store [0]._cnv_fct
#define	SK_UOM			store [0]._uom
#define	SR				store [0]
#define	LL_SPECIAL		 (MULT_LOC && !SK_BATCH_CONT)
#define	LOT_ITEM		 (SR._lot_ctrl [0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct inscRecord	insc_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct sttfRecord	sttf_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

	FILE	*fout;

	char	*data 	= "data",
			*inum2 	= "inum2";

	struct	{
		long	_hhbr_hash;
		long	_hhwh_hash;
		long	_hhcc_hash;
		long	_hhum_hash;
		float	_qty;
		char	_uom [5];
		char	_uom_group [21];
		float	_cnv_fct;
		float	_StdCnvFct;
		char	_lot_ctrl [2];
	} store [2];

	double		TotalLineQty 	=	0.00;

	int		envVarSkStPfrz = 0, 
			envVarStExpDate = 0;

	char	systemDate [11];
	char	lot_ctrl_dflt [2];
	long	lsystemDate;

	char	*envVarSkIvalClass;
 	char 	*result;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	audit [2];
	char	auditDesc [6];
	int		lpno;
	long	page_no;
	long	lst_page;
	double	qty;
	double	disp_tqty;
	long	exp_date;
	char	dflt_qty [15];
	char	rep_qty [10];
	char	uom [5];
	int		dec_pt;
	long	hhwh_hash;
	char	LL [2];
	char	location [11];
	char	locationType [2];
	char	shortStDesc [20];
} local_rec;

#define	DUMM	0
#define	HEAD	1
#define	ITEM	2

static	struct	var	vars [] =
{
	{HEAD, LIN, "stake_code", 	 2, 1, CHARTYPE, 
		"U", "          ", 
		" ", "",  "ST Code ", "", 
		 NE, NO,  JUSTLEFT, "", "", insc_rec.stake_code}, 
	{HEAD, LIN, "stake_desc", 	 3, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.shortStDesc}, 
	{HEAD, LIN, "stake_date", 	 4, 1, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "",  "Date    ", "", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&insc_rec.start_date}, 
	{HEAD, LIN, "stake_time", 	 5, 1, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "",  "Time    ", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.start_time}, 
	{HEAD, LIN, "audit", 	 6, 1, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Audit ? ", "", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.audit}, 
	{HEAD, LIN, "auditDesc", 	 6, 9, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.auditDesc}, 
	{HEAD, LIN, "lpno", 	7, 1, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 

	{ITEM, LIN, "item_no", 	2, 1, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "", "", 
		YES, NO,  JUSTLEFT, "", "", inmr_rec.item_no}, 
	{ITEM, LIN, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "          D e s c r i p t i o n         ", "", 
		 ND, NO,  JUSTLEFT, "", "", inmr_rec.description}, 
	{ITEM, LIN, "UOM", 	 4, 1, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", "", "Enter Input Unit of Measure [SEARCH]",
		NI, NO,  JUSTLEFT, "", "", local_rec.uom}, 
	{ITEM, LIN, "hhwh_hash",	 0,  0, LONGTYPE,
		"NNNNNNNNN", "          ",
		" ", " ", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.hhwh_hash},
	{ITEM, LIN, "qty", 	 6, 1, DOUBLETYPE, 
		local_rec.dflt_qty, "          ", 
		" ", "0", "", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty}, 
	{ITEM, LIN, "location", 	 8, 1, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "", "", 
		YES, NO,  JUSTLEFT, "", "", local_rec.location}, 
	{ITEM, LIN, "exp_date", 	 6, 1, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.exp_date}, 
	{ITEM, LIN, "dec_pt",	 0, 0, INTTYPE,
		"N", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.dec_pt},
	{DUMM, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

float	TotalQty = 0.00;
/*======================= 
| Function Declarations |
========================*/
int 	heading 		(int);
int  	ValidQuantity 	(double, int);
int  	spec_valid 		(int);
void 	CloseDB 		(void);
void 	HeadAudit 		(void);
void 	OpenDB 			(void);
void 	ReadCcmr 		(void);
void	RfErrorLine 	(char *);
void 	SrchInsc 		(void);
void 	SrchInum 		(char *, int);
void 	RfSearchLOC 			(int, long, char *);
void 	TailAudit 		(double);
void 	UpdateData 		(void);
void 	UpdateStdStake 	(int, int);
void 	tab_other 		(int);

extern	int		TruePosition;
extern	int		NO_SRCH_LINES;
extern	int		SR_X_POS;
extern	int		SR_Y_POS;

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;
	int		after, 
			before;

	TruePosition	=	TRUE;
	NO_SRCH_LINES	=	4;
	SR_X_POS		=	0;
	SR_Y_POS		=	0;

	strcpy (lot_ctrl_dflt, "N");

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);

	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ( (sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;

	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		envVarSkStPfrz = atoi (sptr);

	sptr = get_env ("ST_EXP_DATE");
	if (sptr != (char *)0)
		envVarStExpDate = atoi (sptr);
	else
		envVarStExpDate = 0;

	if (!envVarStExpDate)
		FLD ("exp_date") = ND;

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		envVarSkIvalClass = strdup (sptr);
	else
		envVarSkIvalClass = strdup ("ZKPN");

	upshift (envVarSkIvalClass); 

	tab_row	= 3;
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	FLD ("location") = ND;

	if (MULT_LOC && !SK_BATCH_CONT)
		FLD ("location") = YES;

	entry_exit	= FALSE;
	prog_exit	= FALSE;
	restart		= FALSE;
	search_ok	= TRUE;

	heading (HEAD);
	entry (HEAD);

	if (AUDIT)
		HeadAudit ();

	TotalQty = 0.00;
	prog_exit = 0;
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		heading (ITEM);
		entry (ITEM);
		if (restart || prog_exit)
			continue;

		UpdateData ();
	}
	if (AUDIT)
		TailAudit (TotalQty);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	char	rfErrorString [41];

	long	dummyHash	=	0L;

	if (LCHECK ("stake_code"))
	{
		if (SRCH_KEY)
		{
			SrchInsc ();
			return (EXIT_SUCCESS);
		}
		insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (insc, &insc_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (rfErrorString, ML ("Code %s not found"));
			sprintf (err_str, rfErrorString, insc_rec.stake_code);
			RfErrorLine (err_str);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.shortStDesc, "%-18.18s", insc_rec.description);
		DSP_FLD ("stake_desc");
		DSP_FLD ("stake_date");
		DSP_FLD ("stake_time");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("audit"))
	{
		strcpy (local_rec.auditDesc, (AUDIT) ? "Yes" : "No ");
		DSP_FLD ("auditDesc");
		FLD ("lpno") = (AUDIT) ? YES : NA;
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			RfErrorLine (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		if (last_char	==	EOI)
		{
			prog_exit	=	TRUE;
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
			RfErrorLine (ML ("Search Win.to big"));
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			RfErrorLine (ML ("Item Not found"));
			return (EXIT_FAILURE); 
		}
		/*
		SuperSynonymError ();
		*/

		result = strrchr (envVarSkIvalClass, inmr_rec.inmr_class [0]);
		if (result)
		{
			RfErrorLine (ML ("Invalid Class"));
			return (EXIT_FAILURE);
		}

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			RfErrorLine (ML ("WH.Item Not Found" ));
			return (EXIT_FAILURE); 
		}
		if (incc_rec.stat_flag [0] != insc_rec.stake_code [0] &&
		   (incc_rec.stat_flag [0] != '0' || !envVarSkStPfrz))
		{
			RfErrorLine (ML ("ST Code Error"));
			return (EXIT_FAILURE); 
		}

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			RfErrorLine (ML ("Invalid UOM"));
			return (EXIT_FAILURE);
		}
		if (inum_rec.cnv_fct == 0.00)
		{
			RfErrorLine (ML ("UOM Conv.Err"));
			return (EXIT_FAILURE);
		}

		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		strcpy (SR._uom, inum_rec.uom);
		strcpy (SR._uom_group, inum_rec.uom_group);
		SR._StdCnvFct 		= inum_rec.cnv_fct;
		SK_HHBR_HASH 		= inmr_rec.hhbr_hash;
		SK_HHWH_HASH 		= incc_rec.hhwh_hash;
		SR._hhcc_hash 		= incc_rec.hhcc_hash;
		local_rec.dec_pt	= inmr_rec.dec_pt;
		local_rec.hhwh_hash = incc_rec.hhwh_hash;
		strcpy (SR._lot_ctrl, inmr_rec.lot_ctrl);

		if (prog_status == ENTRY)
			SK_QUANTITY = 0.00;

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Unit of Measure |
	--------------------------*/
	if (LCHECK ("UOM"))
	{
		if (F_NOKEY (label ("UOM")) || dflt_used)
			strcpy (local_rec.uom, SR._uom);

		if (SRCH_KEY)
		{
			SrchInum (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR._uom_group);
		strcpy (inum2_rec.uom, local_rec.uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			RfErrorLine (ML ("Invalid UOM"));
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR._hhbr_hash, inum2_rec.hhum_hash))
		{
			RfErrorLine (ML ("Invalid UOM"));
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.uom, inum2_rec.uom);
		SR._hhum_hash = inum2_rec.hhum_hash;
		strcpy (SR._uom, inum2_rec.uom);
        if ( inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR._cnv_fct = inum2_rec.cnv_fct / SR._StdCnvFct;
		strcpy (SR._uom, inum2_rec.uom);
		strcpy (SR._uom_group, inum2_rec.uom_group);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inmr", "DBFIND");

		local_rec.qty = n_dec (atof (temp_str), inmr_rec.dec_pt);
		DSP_FLD ("qty");

		if (!ValidQuantity (local_rec.qty, inmr_rec.dec_pt))
		{
			RfErrorLine (ML ("Invalid Qty"));
			return (EXIT_FAILURE);
		}
		SR._qty = (float) local_rec.qty;

		DSP_FLD ("qty");

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

    /*--------------------                                                 
    | Validate Location. |                                                 
    --------------------*/                                                
    if (LCHECK ("location"))                                                     
    {                                                                           
		if (FLD ("location") == ND)
			return (EXIT_SUCCESS);

        if (SRCH_KEY)
        {                                                                       
			SearchLOC (TRUE, SR._hhwh_hash, temp_str);
			return (EXIT_SUCCESS);
	   	}
		cc = 	FindLocation 
				 (
					SR._hhwh_hash,
					SR._hhum_hash,
					local_rec.location,
					ValidLocations,
					&dummyHash
				);
		if (cc)
		{
			cc	=	CheckLocation 
					(
						SR._hhcc_hash, 
						local_rec.location, 
						local_rec.locationType
					);
			if (cc)
			{
				RfErrorLine (ML ("Invalid Location"));
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
    }
	if (LCHECK ("exp_date"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*--------------------------------------------
| Checks if the quantity entered by the user |
| valid quantity that can be saved to a      |
| float variable without any problems of     |
| losing figures after the decimal point.    |
| eg. if _dec_pt is 2 then the greatest      |
| quantity the user can enter is 99999.99    |
--------------------------------------------*/
int
ValidQuantity (
 double _qty, 
 int _dec_pt)
{
	/*--------------------------------
	| Quantities to be compared with |
	| with the user has entered.     |
	--------------------------------*/
	double	compare [7];
	
	compare [0] = 9999999.00;
	compare [1] = 999999.90;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, ML (mlSkMess238), _qty, compare [_dec_pt]);
		RfErrorLine (err_str);
		return (FALSE);
	}

	return (TRUE);
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

	abc_alias (inum2, inum);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2,inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, 
							(envVarStExpDate) ? "sttf_id_no4" : "sttf_id_no3");
	OpenLocation (ccmr_rec.hhcc_hash);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (insc);
	abc_fclose (sttf);
	abc_fclose (inum);
	abc_fclose (inum2);
	CloseLocation ();
	SearchFindClose ();
	abc_dbclose (data);
}

void
ReadCcmr (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char 	*key_val,
 int	line_cnt)
{
	_work_open (1,4,5);
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, SR._uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR._uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR._hhbr_hash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, SR._uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inum2", "DBFIND");
}

void
SrchInsc (
 void)
{
	_work_open (1,4,5);
	save_rec ("#C", "#Desc.");
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code, " ");
	cc = find_rec (insc, &insc_rec, GTEQ, "r");
	while (!cc && insc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		sprintf (err_str, "%-9.9s", insc_rec.description);
		cc = save_rec (insc_rec.stake_code, err_str);
		if (cc)
			break;

		cc = find_rec (insc, &insc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code, temp_str);
	cc = find_rec (insc, &insc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "insc", "DBFIND");
}

void
UpdateData (
 void)
{


	scn_set (ITEM);

	UpdateStdStake (line_cnt, LL_SPECIAL);
	
	TotalQty += ( (float) (TotalLineQty));

}

void
HeadAudit (
 void)
{
	if ( (fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".10\n");
	fprintf (fout, ".L156\n");
	fprintf (fout, ".ESTOCK TAKE INPUT AUDIT\n");
	fprintf (fout, ".E%s %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E%s %s\n", comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".E%s %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());

	fprintf (fout, ".R   =================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "===============");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "=\n");

	fprintf (fout, "   =================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "===============");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "=\n");

	fprintf (fout, "   |  ITEM  NUMBER  ");
	fprintf (fout, "|          D E S C R I P T I O N         ");
	fprintf (fout, "| PAGE # ");
	if (MULT_LOC)
		fprintf (fout, "| LOCATION ");
	fprintf (fout, "| UOM");
	if (SK_BATCH_CONT)
		fprintf (fout, "| LOT NO.");
	fprintf (fout, "|   COUNTED    ");
	if (envVarStExpDate)
		fprintf (fout, "|EXPIRY DATE");
	fprintf (fout, "|\n");

	fprintf (fout, "   |----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
	if (envVarStExpDate)
		fprintf (fout, "|-----------");
	fprintf (fout, "|\n");
	fflush (fout);
}

void
TailAudit (
 double	tot_qty)
{
	fprintf (fout, "   |----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
	if (envVarStExpDate)
		fprintf (fout, "|-----------");
	fprintf (fout, "|\n");

	fprintf (fout, "   |                ");
	fprintf (fout, "                                TOTAL    ");
	fprintf (fout, "         ");
	fprintf (fout, "     ");
	if (SK_BATCH_CONT)
		fprintf (fout, "         ");
	if (MULT_LOC)
		fprintf (fout, "           ");
	sprintf (err_str, local_rec.rep_qty, tot_qty);
	fprintf (fout, " %14s", err_str);
	if (envVarStExpDate)
		fprintf (fout, "|           ");
	fprintf (fout, "|\n");

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
tab_other (
 int line_no)
{
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
		box (0,0,20,8);

		if (scn == 1)
			rv_pr (ML ("STOCK COUNT INPUT"), 1, 1, 1);
		if (scn == 2)
		{
			rv_pr (ML ("ITEM NUMBER"), 1, 1, 1);
			rv_pr (ML ("UOM"), 1, 3, 1);
			rv_pr (ML ("QUANTITY"), 1, 5, 1);
			rv_pr (ML ("LOCATION"), 1, 7, 1);
		}
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
void
UpdateStdStake (
 int	line_cnt,
 int	LLSpecial)
{
	TotalLineQty	=	0.00;

	abc_selfield ("sttf", "sttf_id_no3");

	sttf_rec.hhwh_hash = SK_HHWH_HASH;
	sttf_rec.hhum_hash = SK_HHUM_HASH;
	if (LLSpecial)
	{
		strcpy (sttf_rec.location, 	local_rec.location);
		strcpy (sttf_rec.lot_no,	"  N/A  ");
		strcpy (sttf_rec.slot_no,	"  N/A  ");
	}
	else
	{
		strcpy (sttf_rec.location, 	"          ");
		strcpy (sttf_rec.lot_no,	"       ");
		strcpy (sttf_rec.slot_no,	"       ");
	}
	cc = find_rec (sttf, &sttf_rec, EQUAL, "u");
	if (cc)
	{
		sttf_rec.hhwh_hash 	=	SK_HHWH_HASH;
		sttf_rec.hhum_hash 	=	SK_HHUM_HASH;
		if (LLSpecial)
		{
			strcpy (sttf_rec.location, 	local_rec.location);
			strcpy (sttf_rec.lot_no,	"  N/A  ");
			strcpy (sttf_rec.slot_no,	"  N/A  ");
		}
		else
		{
			strcpy (sttf_rec.location, 	"          ");
			strcpy (sttf_rec.lot_no,	"       ");
			strcpy (sttf_rec.slot_no,	"       ");
		}
		sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
		sttf_rec.qty 		= (float) SK_QUANTITY * SK_CNV_FCT;
		sttf_rec.page_no 	= local_rec.page_no;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		cc = abc_add (sttf, &sttf_rec);
		if (cc)
			file_err (cc, "sttf", "DBADD");
	}
	else
	{
		sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		sttf_rec.hhum_hash 	= 	SK_HHUM_HASH;
		sttf_rec.qty += (float)	SK_QUANTITY * SK_CNV_FCT;
		cc = abc_update (sttf, &sttf_rec);
		if (cc)
			file_err (cc, "sttf", "DBUPDATE");
	}
	if (AUDIT)
	{
		fprintf (fout, "   |%-16.16s", inmr_rec.item_no);
		fprintf (fout, "|%-40.40s", inmr_rec.description);
		fprintf (fout, "| %6ld ", local_rec.page_no);
		if (LLSpecial)
			fprintf (fout, "|%-10.10s", local_rec.location);
		fprintf (fout, "|%-4.4s", SK_UOM);
		sprintf (err_str, local_rec.rep_qty, SK_QUANTITY * SK_CNV_FCT);
		fprintf (fout, "|%14s", err_str);
		if (envVarStExpDate)
			fprintf (fout, "|%10.10s ", DateToString (local_rec.exp_date));
		fprintf (fout, "|\n");

		TotalLineQty += SK_QUANTITY * SK_CNV_FCT;
	}
}

void
RfErrorLine 
(
	char	*rfMessage)
{
	print_at (9,1, "%R%-18.18s", rfMessage);
	sleep (sleepTime);
	box (0,0,20,8);
}
