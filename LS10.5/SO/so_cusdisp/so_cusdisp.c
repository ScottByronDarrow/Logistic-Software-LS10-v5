/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_cusdisp.c,v 5.11 2002/07/30 07:30:29 scott Exp $
|  Program Name  : (so_stkdisp.c & so_cusdisp.c )                     |
|  Program Desc  : (Sales Order Stock Display and              )      |
|                  (Sales Order Customer Display               )      |
|---------------------------------------------------------------------|
|  Date Written  : (21/06/88)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Id: so_cusdisp.c,v 5.11 2002/07/30 07:30:29 scott Exp $
|---------------------------------------------------------------------|
| $Log : $
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_cusdisp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_cusdisp/so_cusdisp.c,v 5.11 2002/07/30 07:30:29 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <ml_so_mess.h>
#include <ml_std_mess.h>

#define	CUS_DISP	2
#define	STK_DISP	5
#define	C_LINE		 (inmr_rec.inmr_class [0] == 'Z')
#define	BONUS_ITEM	 (soln_rec.bonus_flag [0] == 'Y')

#define		VAL_ITLN 	 (itln_rec.status [0] == 'T' || \
						  itln_rec.status [0] == 'B' || \
						  itln_rec.status [0] == 'M' || \
						  itln_rec.status [0] == 'U')
#define		VAL_TRANSIT 	 (itln_rec.status [0] == 'T')

#define		PHANTOM		 (inmr_rec.inmr_class [0] == 'P')
#define		NON_SYN		 (inmr_rec.hhsi_hash == 0L)

	int		envVarDbCo 			= FALSE,
			byCustomer 			= 1,
			linePrinted			= 0,
			envVarRepTax 		= FALSE,
			envVarDbFind 		= FALSE,
			envVarDbMcurr		= FALSE,
			envVarDbNettUsed 	= TRUE,
			envVarCnNettUsed 	= TRUE;

	char	branchNumber [3],
			displayLine [300],
			dateOrdered [11],
			dateDue [11],
			envVarCurrCode [4],
			mlCusDisp [20] [101];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct pocrRecord	pocr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct soktRecord	sokt_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;

	char 	*inmr2 = "inmr2",
			*data = "data";
	
	float	orderQuantity 	= 0.0,
			totalQuantity 	= 0.0,
			StdCnvFct 		= 0.00,
			CnvFct			= 0.00;
	
	double	orderAmount 	= 0.0,
			totalAmount 	= 0.0,
			balance 		= 0.0;

	char	*item_label	 = "item_no",
		    *item_mask	 = "UUUUUUUUUUUUUUUU",
		    *item_prompt = "Item No.",
	    	*desc_prompt = "Item Description";

	struct	{
		char	*statusType;
		char	*statusDesc;
	} status [] = {
		{"",""},
		{"B",  "Backorder       "},
		{"R",  "Released line.  "},
		{"S",  "Selected line.  "},
		{"P",  "Packing slip.   "},
		{"M",  "Manual Release  "},
		{"F",  "Forward Order   "},
		{"G",  "Scheduled Order "},
		{"C",  "Credit Check    "},
		{"O",  "Over Margin     "},
		{"H",  "Credit Hold     "},
		{"T",  "Transfer        "},
		{"U",  "Unconfirmed TR  "},
		{"W",  "Works Order B/O "},
		{"",""}
	};
    
	char	statusDesc [31];
	char	systemDesc [18];
	
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	value [17];
	char	order_no [9];
	char	desc [41];
	char	previousValue [17];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "debtor",	 4, 22, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.previousValue, "Customer No.", " ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.value},
	{1, LIN, "name",	 4, 80, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Customer Name", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "order_no",	 5, 22, CHARTYPE,
		"UUUUUUUU", "        ",
		"0", "0", "Sales Order No.", " Default : All Current Orders ",
		YES, NO, JUSTRIGHT, "", "", local_rec.order_no},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>

/*=======================
| Function Declarations |
=======================*/
char* 	NewStrStr 			(char *);
int  	FindPackingSlip 	(long);
int  	ProcessSohr 		(void);
int  	ProcessSoln 		(int);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	CalcBalances 		(int, int);
void 	CloseDB 			(void);
void 	DisplayCustomer		(void);
void 	DisplayInfo 		(void);
void 	DisplayItem 		(void);
void 	InitML 				(void);
void 	OpenDB 				(void);
void 	PrintTotals 		(int, int);
void 	ProcessBOM 			(long);
void 	ProcessItemSohr		(int);
void 	ProcessSynonym 		(long);
void 	SrchSohr 			(char *);
void 	shutdown_prog 		(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr;

	/*---------------------------
	| Check for multi-currency. |
	---------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (envVarDbMcurr)
	{
		/*----------------------
		| Get native currency. |
		----------------------*/
		sprintf (envVarCurrCode,"%-3.3s",get_env ("CURR_CODE"));
	}
	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *)0) ? envVarDbNettUsed : atoi (sptr);

	SETUP_SCR (vars);

	envVarRepTax = atoi (get_env ("REP_TAX"));

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];
	byCustomer = !strcmp (sptr, "so_cusdisp");

	if (!byCustomer)
	{
		vars [0].label = item_label;
		vars [0].mask = item_mask;
		vars [0].prmpt = item_prompt;
		vars [1].prmpt = desc_prompt;
		vars [2].scn = 0;
	}

	init_scr ();
	set_tty (); 
	set_masks ();

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind  = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	InitML ();

	strcpy (local_rec.previousValue, (byCustomer) ? "000000" : " ");

	while (prog_exit == 0)
	{
		linePrinted 	= 0;
		search_ok 		= TRUE;
		entry_exit 		= FALSE;
		edit_exit 		= FALSE;
		prog_exit 		= FALSE;
		restart 		= FALSE;
		init_ok 		= TRUE;
		init_vars (1);	
		totalQuantity 	= 0.00;
		totalAmount 	= 0.00;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		DisplayInfo ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===============================
| Multi-lingual initialisation. |
===============================*/
void
InitML (
 void)
{
    int 	i;

	strcpy (mlCusDisp [1], ML ("TOTAL"));
	strcpy (mlCusDisp [2], ML ("ORDER TOTAL"));
	strcpy (mlCusDisp [3], ML ("CUSTOMER TOTAL"));
	strcpy (mlCusDisp [4], ML ("SERIAL NUMBER"));
	strcpy (mlCusDisp [5], ML ("Full Sup Trans."));
	strcpy (mlCusDisp [6], ML ("TRANSFER"));
	strcpy (mlCusDisp [7], ML ("FOR CUST"));
	strcpy (mlCusDisp [8], ML ("FOR STCK"));
	strcpy (mlCusDisp [9], ML ("Full Sup Order"));
	strcpy (mlCusDisp [10], ML ("Ex BOM/KIT Item."));
	strcpy (mlCusDisp [11], ML ("Ex Synonym Item."));
	strcpy (mlCusDisp [12], ML ("BOM/KIT Item."));
	strcpy (mlCusDisp [13], ML ("BONUS ITEM  "));

    for (i = 1; strlen (status [i].statusType); i++)
		status [i].statusDesc = strdup (ML (status [i].statusDesc));
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

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	if (byCustomer)
    {
		open_rec (cumr,cumr_list,CUMR_NO_FIELDS, (!envVarDbFind) 
						? "cumr_id_no" : "cumr_id_no3");
    }
	else
		open_rec (cumr,cumr_list,CUMR_NO_FIELDS,"cumr_hhcu_hash");

	open_rec (inmr,inmr_list,INMR_NO_FIELDS, (byCustomer) ? "inmr_hhbr_hash" 
							 : "inmr_id_no");

	open_rec (sohr,sohr_list,SOHR_NO_FIELDS, (byCustomer) ? "sohr_hhcu_hash" 
							 : "sohr_hhso_hash");

	open_rec (soln,soln_list,SOLN_NO_FIELDS, (byCustomer) ? "soln_id_no" 
							 : "soln_hhbr_hash");

	if (!byCustomer)
	{
		abc_alias (inmr2, inmr);
		open_rec (sokt,sokt_list,SOKT_NO_FIELDS,"sokt_mabr_hash");
		open_rec (inmr2,inmr_list,INMR_NO_FIELDS,"inmr_hhsi_hash");
		open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
		open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
		open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	}
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cohr,cohr_list,COHR_NO_FIELDS,"cohr_hhco_hash");
	open_rec (coln,coln_list,COLN_NO_FIELDS,"coln_hhsl_hash");
	open_rec (inum,inum_list,INUM_NO_FIELDS,"inum_hhum_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (inum);
	if (!byCustomer)
	{
		abc_fclose (sokt);
		abc_fclose (inmr2);
		abc_fclose (ithr);
		abc_fclose (itln);
		abc_fclose (ccmr);
		abc_fclose (pocr);
	}
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	order_found = FALSE;

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("debtor"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber,temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.value));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			/*------------------------------------------------
			| Customer %s is not on File.", local_rec.value) |
			------------------------------------------------*/
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.desc,cumr_rec.dbt_name);
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("order_no"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}

		order_found = FALSE;

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		sohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
		while (!cc && sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			if (!strcmp (sohr_rec.order_no,local_rec.order_no))
			{
				order_found = TRUE;
				break;
			}
			cc = find_rec (sohr,&sohr_rec,NEXT,"r");
		}
		if (!order_found)
		{
			/*------------------------------------
			| Sales Order No. %s is not on file.|
			-----------------------------------*/
			errmess (ML (mlStdMess102));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK (item_label))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			abc_selfield (inmr,"inmr_id_no");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.value, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.value);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-------------------------
			| Item %s is not on File. |
			-------------------------*/
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.value,inmr_rec.item_no);
		strcpy (local_rec.desc,inmr_rec.description);
		display_field (field);
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}
		
	return (EXIT_SUCCESS);
}

void
DisplayInfo (
 void)
{
	if (byCustomer)
	{
		DisplayCustomer ();
		strcpy (local_rec.previousValue,cumr_rec.dbt_no);
	}
	else
	{
		DisplayItem ();
		strcpy (local_rec.previousValue,inmr_rec.item_no);
	}
}

void
DisplayCustomer (
 void)
{
	heading (CUS_DISP);
	Dsp_open (0,2,14);
	Dsp_saverec (" ORDER   |   DATE   |BR|WH|   DATE   | UOM |  QUANTITY  |    VALUE    |   ITEM NUMBER    |     STATUS     | P/SLIP |SYSTEM  NOTES");
	sprintf (err_str, 
				" NUMBER  | ORDERED  |NO|WH|   DUE    |     |  ON ORDER  |    (%3.3s)    |                  |                | NUMBER |             ", cumr_rec.curr_code);
	Dsp_saverec (err_str);
	Dsp_saverec (" [REDRAW]  [NEXT SCREEN]   [PREV SCREEN] [INPUT/END]");

	if (strcmp (local_rec.order_no,"00000000"))
	{
		ProcessSohr ();
		linePrinted = 0;
	}
	else
	{
		sohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
		while (!cc && cumr_rec.hhcu_hash == sohr_rec.hhcu_hash)
		{
			ProcessSohr ();
			cc = find_rec (sohr,&sohr_rec,NEXT,"r");
		}
	}
	if (linePrinted)
		PrintTotals (CUS_DISP,TRUE);

	Dsp_srch ();

	Dsp_close ();
}

void
DisplayItem (
 void)
{
	int	i;
	float	orderQuantity = 0.00;
	char	tran_date [11],
		    tran_value [13];

	char	star [2];

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;

	heading (STK_DISP);
	Dsp_open (0,2,14);
	Dsp_saverec (" CUST.| ACRONYM  |BR|WH|   DATE   |   DATE   | UOM |   QUANTITY  |  VALUE ON  |  ORDER   |     STATUS      | P/SLIP |SYSTEM NOTES");

	if (envVarDbMcurr)
		sprintf (err_str, " NO.  |          |NO|NO|  ORDERED |   DUE    |     |   ON ORDER  | ORDER (%3.3s)|  NUMBER  |                 | NUMBER |            ", envVarCurrCode);
	else
		strcpy (err_str, " NO.  |          |NO|NO|  ORDERED |   DUE    |     |   ON ORDER  |  ON ORDER  |  NUMBER  |                 | NUMBER |            ");

	Dsp_saverec (err_str);

	Dsp_saverec (" [REDRAW]  [NEXT SCREEN]   [PREV SCREEN] [INPUT/END]");

	soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		strcpy (systemDesc, " ");
		if ((soln_rec.qty_order + soln_rec.qty_bord) > 0.00 || C_LINE)
        {
			ProcessItemSohr (FALSE);
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}

	ProcessBOM (inmr_rec.hhbr_hash);

	if (NON_SYN)
	{
		open_rec (inmr2,inmr_list,INMR_NO_FIELDS,"inmr_hhsi_hash");
		ProcessSynonym (inmr_rec.hhbr_hash);
	}

	/*---------------------------------------------------------------
	| Note that the order value for transfers is based on item cost |
	| which is always in local currency.                            |
	---------------------------------------------------------------*/
	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.r_hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.r_hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!VAL_ITLN)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		if (inum_rec.hhum_hash != itln_rec.hhum_hash)
		{
			inum_rec.hhum_hash = itln_rec.hhum_hash;  
			cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
		}
		if (cc)
		{
			strcpy (inum_rec.uom, inmr_rec.sale_unit);
			inum_rec.cnv_fct = 1;
		}
		CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
		switch (itln_rec.status [0])
		{
		case	'B':
		case	'T':
			orderQuantity = itln_rec.qty_border / CnvFct;
			break;

		case	'U':
		case	'M':
			orderQuantity = (itln_rec.qty_border + itln_rec.qty_order) / CnvFct;
			break;

		default:
			orderQuantity = 0.00;
			break;
		}
		if (orderQuantity <= 0.00)
		{
			cc = find_rec (itln, &itln_rec, NEXT, "r");
			continue;
		}
		balance = orderQuantity;
		balance *= out_cost (itln_rec.cost, inmr_rec.outer_size);

		sprintf (tran_date, "%-10.10s", DateToString (itln_rec.due_date));
		sprintf (tran_value,"%11.2f ",balance);

		ccmr_rec.hhcc_hash	= itln_rec.i_hhcc_hash;
		if (find_rec (ccmr,&ccmr_rec,EQUAL,"r"))
		{
			strcpy (ccmr_rec.est_no,"??");
			strcpy (ccmr_rec.cc_no,"??");
		}
		ithr_rec.hhit_hash	=	itln_rec.hhit_hash;
		cc = find_rec (ithr,&ithr_rec,EQUAL,"r");
		if (cc)
			ithr_rec.del_no = 0L;
		
		strcpy (star, (ithr_rec.full_supply [0] == 'Y') ? "*" : " ");
		if (ithr_rec.full_supply [0] == 'Y')
			strcpy (systemDesc, mlCusDisp [5]);

		strcpy (statusDesc, "??????????????????????????????");
		for (i = 1; strlen (status [i].statusType); i++)
		{
			if (itln_rec.status [0] == status [i].statusType [0])
			{
				strcpy (statusDesc,status [i].statusDesc);
				break;
			}
		}
	sprintf (displayLine,
			 "^1%-9.9s%8.8s^6^E%s^E%s^E%s^E          ^E%4.4s ^E%12.2f ^E%s^E%s%08ld ^E %s^E        ^E%-16.16s",
			 mlCusDisp [6],
			 (itln_rec.stock [0] == 'C') ? mlCusDisp [7] : mlCusDisp [8],
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			tran_date,
			inum_rec.uom,
			orderQuantity,
			tran_value,
			star,
			ithr_rec.del_no,
			statusDesc,
			systemDesc);

		Dsp_saverec (displayLine);
		totalQuantity += orderQuantity;
		totalAmount += CENTS (balance);
		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}

	if (linePrinted)
		PrintTotals (STK_DISP,TRUE);

	Dsp_srch ();

	Dsp_close ();
}

void
ProcessItemSohr (
 int BOM)
{
	char	bal_value [13];
	char	star [2];
	char	pslip_no [9];
	float	orderQuantity = 0.00;
	int		i;

    sohr_rec.hhso_hash = soln_rec.hhso_hash;
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
	if (cc)
		return;

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
	if (cc)
		return;

	if (ccmr_rec.hhcc_hash != soln_rec.hhcc_hash)
	{
		ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
		cc = find_rec (ccmr,&ccmr_rec, COMPARISON, "r");
		if (cc)
			return;
	}

	if (soln_rec.qty_order == 0.0 && soln_rec.qty_bord > 0.0 && soln_rec.status [0] == 'M')
		strcpy (soln_rec.status, "B");
	strcpy (statusDesc, "??????????????????????????????");
	for (i = 1; strlen (status [i].statusType); i++)
	{
		if (soln_rec.status [0] == status [i].statusType [0])
		{
			strcpy (statusDesc,status [i].statusDesc);
			break;
		}
	}

 	CalcBalances ((BOM) ? TRUE: FALSE, TRUE); 

	if (BONUS_ITEM)
		strcpy (bal_value,mlCusDisp [13]);
	else
		sprintf (bal_value,"%11.2f",DOLLARS (balance));

	if (soln_rec.status [0] == 'S' ||
		 soln_rec.status [0] == 'P' ||
		 soln_rec.status [0] == 'D' ||
		 soln_rec.status [0] == 'I' ||
		 soln_rec.status [0] == 'B')
	{
		cc = FindPackingSlip (soln_rec.hhsl_hash);
		sprintf (pslip_no, "%-8.8s", (cc) ? ML (mlStdMess272) : cohr_rec.inv_no);
	}
	else
		strcpy (pslip_no, ML (mlStdMess272));

	strcpy (star, (sohr_rec.full_supply [0] == 'Y') ? "*" : " ");
	if (sohr_rec.full_supply [0] == 'Y')
		strcpy (systemDesc, mlCusDisp [9]);

	if (inum_rec.hhum_hash != soln_rec.hhum_hash)
	{
		inum_rec.hhum_hash = soln_rec.hhum_hash;  
		cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
	}
	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}
	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
	orderQuantity = (soln_rec.qty_order + soln_rec.qty_bord) / CnvFct;

	if (BOM)
		orderQuantity *= sokt_rec.matl_qty;

	sprintf (displayLine,"%s^E %s^E%s^E%s^E%s^E%s^E%s ^E%12.2f ^E%s ^E%s%s ^E %16.16s^E%-8.8s^E%s",
		cumr_rec.dbt_no,
		cumr_rec.dbt_acronym,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		dateOrdered,
		dateDue,
        inum_rec.uom,
		orderQuantity,
		bal_value,
		star,
		sohr_rec.order_no,
		statusDesc,
		pslip_no,
		systemDesc);

	Dsp_saverec (displayLine);

	linePrinted++;
	
	if (strcmp (soln_rec.serial_no, "                         "))
	{
		sprintf (displayLine,
				 " %-13.13s : %25.25s     ^E     ^E             ^E            ^E        ^E                 ^E      ^E            ",
				 mlCusDisp [4],
				 soln_rec.serial_no);

		Dsp_saverec (displayLine);

		linePrinted++;
	}
}

void
ProcessBOM (
	long	hhbrHash)
{
	sokt_rec.mabr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.mabr_hash == hhbrHash)
	{
		soln_rec.hhbr_hash	=	sokt_rec.hhbr_hash;
	    cc = find_rec (soln,&soln_rec,GTEQ,"r");
	    while (!cc && soln_rec.hhbr_hash == sokt_rec.hhbr_hash)
	    {
			strcpy (systemDesc, mlCusDisp [10]);
			ProcessItemSohr (TRUE);
	    	cc = find_rec (soln,&soln_rec,NEXT,"r");
	    }
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
}

void
ProcessSynonym (
	long	hhbrHash)
{
	inmr2_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr2_rec, GTEQ, "r");
	while (!cc && inmr2_rec.hhsi_hash == hhbrHash)
	{
		soln_rec.hhbr_hash = inmr2_rec.hhbr_hash;
		cc = find_rec (soln,&soln_rec,GTEQ,"r");
		while (!cc && soln_rec.hhbr_hash == inmr2_rec.hhbr_hash)
		{
			strcpy (systemDesc, mlCusDisp [11]);
			if (soln_rec.qty_order + soln_rec.qty_bord)
			{
				ProcessItemSohr (FALSE);
			}
			cc = find_rec (soln,&soln_rec,NEXT,"r");
		}
		cc = find_rec (inmr2, &inmr2_rec, NEXT, "r");
	}
}

void
CalcBalances (
	int		BOM, 
	int		in_local_curr)
{
	double	l_total	=	0.00,
			l_disc	=	0.00,
			l_tax	=	0.00,
			l_gst	=	0.00;

	balance	=	0.00;

	sprintf (dateOrdered, "%10.10s", DateToString (sohr_rec.dt_raised));
	sprintf (dateDue, "%10.10s", DateToString (soln_rec.due_date));

	if (soln_rec.bonus_flag [0] != 'Y')
	{
		orderQuantity = soln_rec.qty_order + soln_rec.qty_bord;
		if (BOM)
			orderQuantity *= sokt_rec.matl_qty;
			
		l_total	=	(double) orderQuantity;
		l_total	*=	out_cost (soln_rec.sale_price, inmr_rec.outer_size);
		l_total	=	no_dec (l_total);

		l_disc	=	(double) soln_rec.dis_pc;
		l_disc	*=	l_total;
		l_disc	=	DOLLARS (l_disc);
		l_disc	=	no_dec (l_disc);

		if (envVarRepTax)
		{
			l_tax	=	(double) soln_rec.tax_pc;
			if (sohr_rec.tax_code [0] == 'D')
				l_tax *= l_total;
			else
			{
				if (envVarDbNettUsed)
					l_tax	*=	(l_total + soln_rec.item_levy + l_disc);
				else
					l_tax	*=	(l_total + soln_rec.item_levy);
			}
			l_tax	=	DOLLARS (l_tax);
		}
		l_tax	=	no_dec (l_tax);

		l_gst	=	(double) soln_rec.gst_pc;
		if (envVarDbNettUsed)
			l_gst	*=	(l_total - l_disc) + l_tax + soln_rec.item_levy;
		else
			l_gst	*=	(l_total + l_tax + soln_rec.item_levy);

		l_gst	=	DOLLARS (l_gst);
			
		if (envVarDbNettUsed)
			balance	=	l_total - l_disc + l_tax + l_gst + soln_rec.item_levy;
		else
			balance	=	l_total + l_tax + l_gst + soln_rec.item_levy;
	
		if (in_local_curr == TRUE)
		{
			/*------------------------------------------------
			| Convert soln debtors balance to local_currency |
			------------------------------------------------*/
			strcpy (pocr_rec.co_no,	comm_rec.co_no);
			strcpy (pocr_rec.code, 	cumr_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pocr, "DBFIND");
	
			if (pocr_rec.ex1_factor != 0.00)
				balance = balance / pocr_rec.ex1_factor;
		}
		totalAmount += balance;
	}
	totalQuantity += orderQuantity;
}

void
PrintTotals (
 int indx, 
 int key_total)
{
	if (indx == CUS_DISP)
	{
		if (key_total)
		{
			/*-----------------------
			| Customer Totals	|
			-----------------------*/
			sprintf (displayLine,
					 "%-27.27s^1%-11.11s^6     ^E%11.2f ^E%12.2f ^E%-60.60s",
					 " ",
					 mlCusDisp [2],
					 totalQuantity,
					 DOLLARS (totalAmount),
					 " ");
			Dsp_saverec (displayLine);
		}
		else
		{
			/*---------------
			| Order Totals	|
			---------------*/
			sprintf (displayLine,
					 "%-24.24s^1%-14.14s^6     ^E%11.2f ^E%12.2f ^E%-62.62s",
					 " ",
					 mlCusDisp [3],
					 orderQuantity,
					 DOLLARS (orderAmount),
					 " ");
			Dsp_saverec (displayLine);

			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");
		}
	}
	else
	{
		sprintf (displayLine,
				 "%-45.45s^1%-5.5s^6 ^E%12.2f ^E%11.2f ^E%-53.53s",
				 " ",
				 mlCusDisp [1],
				 totalQuantity,
				 DOLLARS (totalAmount),
				 " ");
		Dsp_saverec (displayLine);
	}
}

int
ProcessSohr (void)
{
	int	firstLine = 1;

	if (sohr_rec.hhcu_hash != cumr_rec.hhcu_hash)
		return (EXIT_SUCCESS);

	orderQuantity 	= 0.00;
	orderAmount 	= 0.00;

	soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln,&soln_rec,GTEQ,"r");

	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");

		if (!cc && (C_LINE || (soln_rec.qty_order + soln_rec.qty_bord) > 0.00))
		{
			if (ProcessSoln (firstLine))
				return (EXIT_FAILURE);

			firstLine = 0;
			linePrinted = 1;
		}
		cc = find_rec (soln,&soln_rec,NEXT,"r");
	}
	if (!firstLine)
		PrintTotals (CUS_DISP,FALSE);
	return (EXIT_SUCCESS);
}

int
ProcessSoln (
	int 	firstLine)
{
	char	bal_value [13];
	char	pslip_no [9];
	char	star [2];
	float	DspQty;
	int	i;

    inmr_rec.hhbr_hash = soln_rec.hhbr_hash;
	cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
        strcpy (inmr_rec.item_no,"Unknown Item No.");

	inum_rec.hhum_hash = soln_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	StdCnvFct = inum_rec.cnv_fct;

	if (inum_rec.hhum_hash != soln_rec.hhum_hash)
	{
		inum_rec.hhum_hash = soln_rec.hhum_hash;  
		cc = find_rec (inum, &inum_rec, EQUAL, "r"); 
	}
	if (cc)
	{
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		inum_rec.cnv_fct = 1;
	}

	CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
	if (soln_rec.hhcc_hash != ccmr_rec.hhcc_hash)
	{
		ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");
	}

	CalcBalances (FALSE, FALSE);

	CnvFct	= inum_rec.cnv_fct / StdCnvFct;
	DspQty	= (soln_rec.qty_order + soln_rec.qty_bord) / CnvFct;

	if (BONUS_ITEM)
		strcpy (bal_value,mlCusDisp [13]);
	else
	{
		orderAmount += balance;
		sprintf (bal_value,"%12.2f",DOLLARS (balance));
	}
	if (soln_rec.qty_order == 0.0 && soln_rec.qty_bord > 0.0 && soln_rec.status [0] == 'M')
		strcpy (soln_rec.status, "B");
	strcpy (statusDesc, "??????????????????????????????");
	for (i = 1; strlen (status [i].statusType); i++)
	{
		if (soln_rec.status [0] == status [i].statusType [0])
		{
			strcpy (statusDesc,status [i].statusDesc);
			break;
		}
	}

	if
	 (
		soln_rec.status [0] == 'S' ||
		soln_rec.status [0] == 'P' ||
		soln_rec.status [0] == 'D' ||
		soln_rec.status [0] == 'I' ||
		soln_rec.status [0] == 'B'
	)
	{
		cc = FindPackingSlip (soln_rec.hhsl_hash);
		sprintf (pslip_no, "%-8.8s", (cc) ? ML (mlStdMess272) : cohr_rec.inv_no);
	}
	else
		strcpy (pslip_no, ML (mlStdMess272));

	strcpy (star, (sohr_rec.full_supply [0] == 'Y') ? "*" : " ");

	if (sohr_rec.full_supply [0] == 'Y')
		strcpy (systemDesc, mlCusDisp [9]);
	else
		strcpy (systemDesc, "               ");

	if (PHANTOM)
		strcpy (systemDesc, mlCusDisp [12]);

	sprintf (displayLine,"%s%-8.8s^E%10.10s^E%-2.2s^E%-2.2s^E%s^E %s^E%11.2f ^E%s ^E %-16.16s ^E%-16.16s^E%-8.8s^E%s",
		 (!firstLine) ? " " : star,
		 (!firstLine) ? " " : sohr_rec.order_no,
		 (!firstLine) ? " " : dateOrdered,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		dateDue,
		inum_rec.uom,
		DspQty,
		bal_value,
		inmr_rec.item_no,
		statusDesc,
		pslip_no,
		systemDesc);

	cc = Dsp_saverec (displayLine);
	if (cc)
		return (cc);


	if (strcmp (soln_rec.serial_no, "                         "))
	{
		sprintf (displayLine,
				 "        ^E          ^E  ^E  ^E          ^E     ^E             ^E            ^E %-14.14s: %-25.25s^E             ",
				 mlCusDisp [4],
				 soln_rec.serial_no);
		cc = Dsp_saverec (displayLine);
		if (cc)
			return (cc);
	}
	return (EXIT_SUCCESS);
}

/*==========================
| Search for order number. |
==========================*/
void
SrchSohr (
 char *key_val)
{
	_work_open (8,0,40);
	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sprintf (sohr_rec.order_no,"%-8.8s",key_val);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash; 

	save_rec ("#Order No","#Customer Order No.   ");
	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while
	 (
		!cc &&
		sohr_rec.hhcu_hash == cumr_rec.hhcu_hash &&
		!strcmp (sohr_rec.co_no,comm_rec.co_no)
	)
	{
		if (!strncmp (sohr_rec.order_no,key_val,strlen (key_val)))
		{
			cc = save_rec (sohr_rec.order_no,
				       sohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.est_no);
	sprintf (sohr_rec.order_no,"%-8.8s",temp_str);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (sohr,&sohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, sohr, "DBFIND");
}

int
FindPackingSlip (
	long	hhslHash)
{
	/*-----------------------------------------
	| Find coln record for current hhslHash. |
	-----------------------------------------*/
	coln_rec.hhsl_hash	=	hhslHash;
 	cc = find_rec (coln,&coln_rec,COMPARISON,"r");
	if (cc)
		return (cc);

	/*-------------------------------------------------------------
	| Cohr record is already found so don't bother to find again. |
	-------------------------------------------------------------*/
	if (coln_rec.hhco_hash == cohr_rec.hhco_hash)
		return (EXIT_SUCCESS);

	cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
 	return (find_rec (cohr,&cohr_rec,COMPARISON,"r"));
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		swide ();
		clear ();

		/*-------------------------------------
		| Sales Order Display by Customer. 	  |
		| Sales Order Display by Item Number. |
		-------------------------------------*/
		if (byCustomer)
			strcpy (err_str, ML (mlSoMess073)); 
		else
			strcpy (err_str, ML (mlSoMess074));

		rv_pr (err_str, (130 - strlen (err_str)) / 2, 0, 1);

		move (0,1);
		line (132);

		switch (scn)
		{
		case	1:
			box (0,3,132, (byCustomer) ? 2 : 1);

			line_cnt = 0;
			scn_write (scn);
			move (0,21);
			line (132);
			break;

		case	CUS_DISP:
			sprintf (err_str, " %s - %s", cumr_rec.dbt_no,
							clip (cumr_rec.dbt_name));
			us_pr (err_str, (130 - strlen (err_str)) / 2, 1, 1);
			break;

		case	STK_DISP:
			sprintf (err_str, " %s - %s", inmr_rec.item_no,
						clip (inmr_rec.description));
			us_pr (err_str, (130 - strlen (err_str)) / 2, 1, 1);
			break;

		default:
			break;
		}
		sprintf (err_str, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, "%s", err_str);

		sprintf (err_str,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_short);
		print_at (22,48, "%s", err_str);
		move (0,23);
		line (132);
	}

    return (EXIT_SUCCESS);
}
