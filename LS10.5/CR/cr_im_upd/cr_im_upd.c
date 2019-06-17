/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_im_upd.c,v 5.3 2001/09/21 09:36:00 robert Exp $
|  Program Name  : (cr_im_upd.c)
|  Program Desc  : (Supplier Invoice automatic authorisation)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 01/10/93         |
|---------------------------------------------------------------------|
| $Log: cr_im_upd.c,v $
| Revision 5.3  2001/09/21 09:36:00  robert
| Updated to correct key event problem with LS10-GUI
|
| Revision 5.2  2001/08/09 08:51:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:28  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:03:15  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/27 02:37:32  scott
| Updated to change interface code from "PO VAR VAL" to "POVARIANCE"
| Two interfaces where defined being "PO VAR VAL" and "PO VAR QTY" but the
| later was never used.
| Also fixed core dump at exit.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_im_upd.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_im_upd/cr_im_upd.c,v 5.3 2001/09/21 09:36:00 robert Exp $";

#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <pslscr.h>
#include <GlUtils.h>
#include <twodec.h>
#include <dsp_screen.h>
#include <dsp_process.h>

#define		PO_KEY		0
#define		SH_KEY		1
#define		GR_KEY		2
#define		DD_KEY		3
#define		SEL_PSIZE	12
#define		COSTED		(pogl_rec.pur_status [0] == 'A' || \
						 pogl_rec.pur_status [0] == 'P')

#define		INVOICE		(suin2_rec.type [0] == '1')

	int		printerNumber 	= 1,	
			envVarGst		= 0,
			InvoiceApproved = FALSE,
			selectKey 		= 99,
			SORT_OPEN 		= FALSE,
			IM_UPDATE 		= TRUE;

	char 	*suin2	= 	"suin2",
			*posh2	= 	"posh2",
			*data	=	"data",
			*fifteenSpaces	=	"               ";

	char	displayString [150],
			envVarGstTaxName [4];

	double	invoiceTotal [6]	= {0,0,0,0,0,0},
			grTotal [6]			= {0,0,0,0,0,0},
			envVarPoValTol		= 0.00;
	float	envVarPoPerTol		= 0.00,
			valueTotal 			= 0.00;


#include	"schema"

struct comrRecord	comr_rec;
struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct suinRecord	suin_rec;
struct suinRecord	suin2_rec;
struct poghRecord	pogh_rec;
struct poglRecord	pogl_rec;
struct suidRecord	suid_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct ddhrRecord	ddhr_rec;
struct ddlnRecord	ddln_rec;
struct ddshRecord	ddsh_rec;
struct inmrRecord	inmr_rec;
struct poshRecord	posh_rec;
struct poshRecord	posh2_rec;

FILE	*pout,
		*fsort;

struct	{
	char	dummy [11];
	char	docNo [16];
	char	refNo [16];
	char	shipNo [3];
	char	recType [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "recType",	 2, 22, CHARTYPE,
		"U", "          ",
		" ", "P", "Receipt Type", "Enter G(oods Receipt), P(urchase order), S(hipment) or D(irect Delivery Shipment). DEFAULT = P(urchase order).",
		 YES, NO,  JUSTLEFT, "GPSD", "", local_rec.recType},
	{1, LIN, "docNo",	 2, 60, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Document no. ", "Enter Referance no. [SEARCH KEY] Available. ",
		ND, NO,  JUSTLEFT, "", "", local_rec.docNo},
	{1, LIN, "shipNo",	 2, 100, CHARTYPE,
		"NN", "          ",
		" ", " ", "Shipment no. ", "Enter Referance no. [SEARCH KEY] Available. ",
		ND, NO,  JUSTLEFT, "", "", local_rec.shipNo},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

char	localCurrency [4];

/*===========================
| Local function prototypes |
===========================*/	
int		CheckInput		 	(char *);
int		FindGlmr		 	(char *);
int		SrchDdsh		 	(char *);
int		heading			 	(int);
int		spec_valid		 	(int);
void	ApproveInvoice	 	(char *, char *);
void	ChangeInvoice	 	(char *, char *);
void	CloseDB		 	 	(void);
void	DispAudit		 	(void);
void	DisplayHeadingInfo	(void);
void	EndReport		 	(void);
void	OpenDB			 	(void);
void	PrintHeaderInfo	 	(void);
void	PrntAudit		 	(char *, char *);
void	ProcInvoices	 	(char *, char *, int);
void	ProcessData		 	(void);
void	ProcessDocuments 	(char *, char *);
void	ProcessSuin		 	(void);
void	ProcessSumr		 	(void);
void	SrchPogh		 	(char *);
void	WriteGlTransaction	(int, char *, double, double);
void	WriteGlVariance	 	(int, char *, double, char *, char *, long);
void	shutdown_prog	 	(void);

/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "cr_im_upd"))
		IM_UPDATE = TRUE;
	else
		IM_UPDATE = FALSE;

	if (argc < 2) 
	{
		print_at (0,0,"Usage: %s <printerNumber>",argv [0]);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*--------------------------
	| Check if actual costing. |
	--------------------------*/
	sptr = chk_env ("PO_ACT_COST");
	if ( (sptr == (char *) 0)
	|| (*sptr != 'Y' && *sptr != 'y'))
	{
		print_at (0,0,"Option %s is not available with estimated costing.",argv [0]);
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	/*-----------------------
	| Check if gst applies. |
	-----------------------*/
	sptr = chk_env ("GST");
	if (sptr == (char *)0)
		envVarGst = 0;
	else
		envVarGst = (*sptr == 'Y' || *sptr == 'y');

	if (envVarGst)
		sprintf (envVarGstTaxName, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (envVarGstTaxName, "%-3.3s", "Tax");

	SETUP_SCR (vars);

	/*-------------------------------------
	| Get purchase order value tolerance. |
	-------------------------------------*/
	sptr = chk_env ("PO_VAL_TOL");
	envVarPoValTol = (sptr == (char *)0) ? 0.00 : atof (sptr);

	/*------------------------------------------
	| Get purchase order percentage tolerance. |
	------------------------------------------*/
	sptr = chk_env ("PO_PER_TOL");
	envVarPoPerTol = (float) ( (sptr == (char *)0) ? 0.00 : atof (sptr));

	printerNumber = atoi (argv [1]);

	OpenDB ();
	
	if (IM_UPDATE)
	{
		dsp_screen ("Supplier Automatic Invoice Approval Update.",
										comm_rec.co_no, comm_rec.co_name);

		/*-------------------------------
		| Produce audit report heading. |
		-------------------------------*/
		PrintHeaderInfo ();

		/*-----------------------
		| Process all suppliers |
		-----------------------*/
		ProcessSumr ();

		/*-------------------------------
		| Produce audit report trailer. |
		-------------------------------*/
		EndReport ();

	}
	else
	{
		/*----------------------------
		| Setup required parameters. |
		----------------------------*/
		init_scr ();
		set_tty ();

		swide ();
		clear ();

		set_masks ();

		while (prog_exit == 0) 
		{
			entry_exit	= FALSE;
			edit_exit	= FALSE;
			prog_exit	= FALSE;
			restart		= FALSE;
			search_ok	= TRUE;
			init_vars (1);

			/*------------------------------
			| Enter screen 1 linear input. |
			------------------------------*/
			heading (1);
			entry (1);
			if (prog_exit || restart) 
				 break;

			heading (1);
			scn_display (1);

			abc_selfield (sumr, "sumr_hhsu_hash");
			DisplayHeadingInfo ();
			ProcessDocuments (local_rec.refNo, local_rec.recType);
		}	
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

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	abc_alias (suin2, suin);
	abc_alias (posh2, posh);

	open_rec (sumr ,sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
	open_rec (suin ,suin_list, SUIN_NO_FIELDS, "suin_hhsu_hash");
	open_rec (suin2,suin_list, SUIN_NO_FIELDS, "suin_im_id");
	open_rec (pogh ,pogh_list, POGH_NO_FIELDS, "pogh_id_no");
	open_rec (pogl ,pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (posh ,posh_list, POSH_NO_FIELDS, "posh_csm_id");
	open_rec (posh2,posh_list, POSH_NO_FIELDS, "posh_id_no");
	open_rec (suid ,suid_list, SUID_NO_FIELDS, "suid_id_no");
	open_rec (pohr ,pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln ,poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (ddhr ,ddhr_list, DDHR_NO_FIELDS, "ddhr_hhdd_hash");
	open_rec (ddln ,ddln_list, DDLN_NO_FIELDS, "ddln_id_no2");
	open_rec (ddsh ,ddsh_list, DDSH_NO_FIELDS, "ddsh_hhds_hash");
	open_rec (inmr ,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	OpenGlmr ();
	OpenPocr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*======================== 
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suin2);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (suid);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	abc_fclose (posh2);
	abc_fclose (ddhr);
	abc_fclose (ddln);
	abc_fclose (ddsh);
	abc_fclose (inmr);
	GL_CloseBatch (printerNumber);
	GL_Close ();

	abc_dbclose (data);
}

/*============================
| Read gl account master.    |
============================*/
int
FindGlmr (
 char *	account)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P') 
		return (2);
		
	return (EXIT_SUCCESS);
}

/*=============================
| Special validation section. |
=============================*/
int
spec_valid (
 int field)
{
	/*------------------------
	| Validate receipt type. |
	------------------------*/
	if (LCHECK ("recType"))
	{
		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.recType [0] == 'D')
		{
			FLD ("docNo") = YES;
			FLD ("shipNo") = YES;
		}
		else
		{
			FLD ("docNo") = YES;
			FLD ("shipNo") = ND;
		}
		heading (1);

		if (prog_status != ENTRY)
		{
			/*---------------------------------------
			| Force document number to be re-input. |
			---------------------------------------*/
			do
			{
				get_entry (field + 1);
				if (restart)
					break;
			} while (spec_valid (field + 1));
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Check document no. |
	--------------------*/
	if (LCHECK ("docNo"))
	{
		if (local_rec.recType [0] == 'P') 
			abc_selfield (pogh, "pogh_po_id");

		if (local_rec.recType [0] == 'S') 
			abc_selfield (pogh, "pogh_sh_id");

		if (local_rec.recType [0] == 'D') 
			abc_selfield (pogh, "pogh_dd_id");

		if (local_rec.recType [0] == 'G') 
			abc_selfield (pogh, "pogh_id_no2");

		if (SRCH_KEY)
		{
			SrchPogh (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used || !strcmp (local_rec.docNo, fifteenSpaces))
		{
			if (local_rec.recType [0] == 'G') 
				errmess (ML (mlStdMess049));

			if (local_rec.recType [0] == 'P' ||
				 local_rec.recType [0] == 'D') 
				errmess (ML (mlStdMess048));

			if (local_rec.recType [0] == 'S') 
				errmess (ML (mlStdMess050));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (pogh_rec.co_no,comm_rec.co_no);
		if (local_rec.recType [0] == 'G') 
		{
			strncpy (pogh_rec.gr_no, local_rec.docNo, 15);
			strncpy (local_rec.refNo, local_rec.docNo,15);
		}

		if (local_rec.recType [0] == 'P') 
		{
			strcpy (pogh_rec.pur_ord_no, zero_pad (local_rec.docNo, 15));
			strncpy (local_rec.refNo, local_rec.docNo, 15);
		}

		if (local_rec.recType [0] == 'S') 
		{
        	strcpy (posh_rec.co_no, comm_rec.co_no);
        	strcpy (posh_rec.csm_no, zero_pad (local_rec.docNo,12));
        	cc = find_rec (posh, &posh_rec, EQUAL, "r");
        	if (cc)
            	pogh_rec.hhsh_hash	=	(cc) ? -1 : posh_rec.hhsh_hash;
        
			strcpy (local_rec.refNo, local_rec.docNo);
		}
		if (local_rec.recType [0] == 'D')
		{
			abc_selfield (pohr, "pohr_id_no3");

			/*----------------------------
			| Check if order is on file. |
			----------------------------*/
			strcpy (pohr_rec.co_no,comm_rec.co_no);
			strcpy (pohr_rec.br_no,comm_rec.est_no);
			strcpy (pohr_rec.pur_ord_no, zero_pad (local_rec.docNo, 15));
	
			cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
			abc_selfield (pohr, "pohr_hhpo_hash");
			if (cc)
			{
				errmess (ML (mlStdMess048));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			if (pohr_rec.status [0] == 'U')
			{
				print_mess (ML ("Purchase order must be approved first."));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	
			if (pohr_rec.stat_flag [0] == 'Q')
			{
				print_mess (ML ("Invoice/Credit may not be entered for quick purchase order."));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			if (pohr_rec.drop_ship [0] != 'Y')
			{
				print_mess (ML ("Normal Purchase Orders must be processed through the Purchase Order receipt type."));
				sleep (sleepTime);
		
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}

		cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
		if (cc)
		{
			if (local_rec.recType [0] == 'G') 
				print_mess (ML (mlStdMess049));
			if (local_rec.recType [0] == 'P') 
				print_mess (ML (mlStdMess048));
			if (local_rec.recType [0] == 'S') 
				print_mess (ML (mlStdMess050));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Check dd shipment no. |
	-----------------------*/
	if (LCHECK ("shipNo"))
	{
		if (local_rec.recType [0] != 'D') 
			return (EXIT_SUCCESS);

		abc_selfield (ddsh, "ddsh_id_no");

		if (SRCH_KEY)
		{
			SrchDdsh (temp_str);
			return (EXIT_SUCCESS);
		}

		/*-------------------------------
		| Check if shipment is on file. |
		-------------------------------*/
		ddsh_rec.hhdd_hash = pohr_rec.hhdd_hash;
		ddsh_rec.hhsu_hash = pohr_rec.hhsu_hash;
		sprintf (ddsh_rec.ship_no, "%2.2s", clip (local_rec.shipNo));
		cc = find_rec (ddsh, &ddsh_rec, EQUAL, "r");
		abc_selfield (ddsh, "ddsh_hhds_hash");
		if (cc)
		{
			print_mess (ML ("Shipment not found."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		switch (ddsh_rec.stat_flag [0])
		{
			case 'A' : print_mess (ML ("Shipment for purchase order has not yet been confirmed."));
					   sleep (sleepTime);
					   return (EXIT_FAILURE);
			case 'C' : print_mess (ML ("Shipment has not yet been despatch confirmed."));
					   sleep (sleepTime);
					   return (EXIT_FAILURE);
			default	 : break;
		}


		strcpy (pogh_rec.co_no,comm_rec.co_no);
		pogh_rec.hhds_hash = ddsh_rec.hhds_hash;

		cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML ("Purchase order not found."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.refNo, "%07ld", ddsh_rec.hhds_hash);
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}
/*==========================
| Process whole sumr file. |
==========================*/
void
ProcessSumr (
 void)
{
	/*-------------------------------
	| Process supplier master file. |
	-------------------------------*/
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, "  ");
	strcpy (sumr_rec.crd_no, "      ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r"); 
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		dsp_process ("Supplier", sumr_rec.crd_no);

		/*-----------------------------------------
		| Read all invoices for current supplier. |
		-----------------------------------------*/
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, GTEQ, "r");
		while (!cc && suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			/*----------------------------------------------------------
			| IF (Invoice is not Approved) AND (receipt type equals    |
            |      P(urchase), G(oods Receipt) or S(hipment number     |
            | THEN ProcessSuin ()                                      |
			----------------------------------------------------------*/
			if (suin_rec.approved [0] == 'N' && 
				(suin_rec.rec_type [0] == 'P' ||
				 suin_rec.rec_type [0] == 'G' ||
				 suin_rec.rec_type [0] == 'D' ||
				 suin_rec.rec_type [0] == 'S'))
			{
				ProcessSuin ();
			}
			cc = find_rec (suin, &suin_rec, NEXT, "r");
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r"); 
	}
	if (SORT_OPEN == TRUE)
	{
		abc_selfield (sumr, "sumr_hhsu_hash");
		ProcessData ();
		sort_delete (fsort, "cr_im_app");
		SORT_OPEN = FALSE;
	}
}

/*====================
| Process suin file. |
====================*/
void
ProcessSuin (
 void)
{
	char	dataString [21];

	if (SORT_OPEN == FALSE)
	{	
		fsort = sort_open ("cr_im_app");
		SORT_OPEN = TRUE;
	}

	/*----------------------------------------------------------
    |       Fields within sort string.                  Offset |
	|----------------------------------------------------------|
	| (1)  Receipt type                                     0  |
	| (2)  Document number                                  2  |
	----------------------------------------------------------*/
	sprintf (dataString,"%-1.1s %-15.15s\n",suin_rec.rec_type, suin_rec.doc_no);
	sort_save (fsort, dataString);
}

/*==============================
| Process Data from sort file. |
==============================*/
void
ProcessData (
 void)
{
	char *	sptr;
	char	recType [2],
			previousRecType [2],
			docNo [16],
			previousDocNo [16];

	int		first_time = TRUE; 

	abc_selfield (sumr, "sumr_hhsu_hash");

	fsort = sort_sort (fsort, "cr_im_app");
	sptr = sort_read (fsort);

	while (sptr != (char *) 0)
	{
		sprintf (recType,"%-1.1s", 	 sptr + 0);
		sprintf (docNo,  "%-15.15s", sptr + 2);

		if (first_time || strcmp (previousDocNo,docNo) || strcmp (previousRecType,recType))
		{
			if (!first_time)
				ProcessDocuments (previousDocNo, previousRecType);

			strcpy (previousRecType, recType);
			strcpy (previousDocNo, docNo);

		}
		first_time = FALSE;
		sptr = sort_read (fsort);
	}
	ProcessDocuments (previousDocNo, previousRecType);
}

/*====================
| Process document . |
====================*/
void
ProcessDocuments (
 char *	docNo,
 char *	recType)
{
	int		i;
	double	nor_cst;
	int		dropShipment = FALSE;

	for (i = 0; i < 6; i++)
		grTotal [i] = 0.00;

	/*-------------------------------------------------
	| Select relevent key if not already selected.    |
	|  P(urchase order), S(hipment), G(oods Receipt). |
	-------------------------------------------------*/
	if (recType [0] == 'P' && selectKey != PO_KEY) 
	{
		abc_selfield (pogh, "pogh_po_id");
		selectKey = PO_KEY;
	}

	if (recType [0] == 'S' && selectKey != SH_KEY) 
	{
		abc_selfield (pogh, "pogh_sh_id");
		selectKey = SH_KEY;
	}

	if (recType [0] == 'D' && selectKey != DD_KEY) 
	{
		abc_selfield (pogh, "pogh_dd_id");
		selectKey = DD_KEY;
	}

	if (recType [0] == 'G' && selectKey != GR_KEY) 
	{
		abc_selfield (pogh, "pogh_id_no2");
		selectKey = GR_KEY;
	}
	strcpy (pogh_rec.co_no,comm_rec.co_no);

	if (recType [0] == 'G') 
		strncpy (pogh_rec.gr_no, docNo, strlen (pogh_rec.gr_no));

	if (recType [0] == 'P') 
		strcpy (pogh_rec.pur_ord_no, zero_pad (docNo, 15));

	if (recType [0] == 'D') 
		pogh_rec.hhds_hash = atol (docNo);

	if (recType [0] == 'S') 
	{
        strcpy (posh_rec.co_no, comm_rec.co_no);
        sprintf (posh_rec.csm_no, "%-12.12s", zero_pad (docNo, 12));
        cc = find_rec (posh, &posh_rec, EQUAL, "r");
        if (cc)
            file_err (cc, "posh", "DBFIND");
        pogh_rec.hhsh_hash = posh_rec.hhsh_hash;
	}

	/*----------------------
	| Find goods receipts. |
	----------------------*/
	cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
	while (!cc && !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		if (recType [0] == 'G' && strncmp (pogh_rec.gr_no, docNo, strlen (pogh_rec.gr_no)))
			break;

		if (recType [0] == 'P' && strcmp (pogh_rec.pur_ord_no,zero_pad (docNo,15)))
			break;

		if (recType [0] == 'D' && pogh_rec.hhds_hash != atol (docNo))
			break;
		
		if (recType [0] == 'S' && pogh_rec.hhsh_hash != posh_rec.hhsh_hash)
			break;
		
		if (pogh_rec.drop_ship [0] == 'Y')
			dropShipment = TRUE;

		/*----------------------------------------------
		| Process all lines on goods receipt to obtain |
		| values required for comparison.              |
		----------------------------------------------*/
		pogl_rec.hhgr_hash = pogh_rec.hhgr_hash;
		pogl_rec.line_no = 0;
		cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
		while (!cc && pogl_rec.hhgr_hash == pogh_rec.hhgr_hash)
		{
			/*---------------------------------------------------------------
			| Line has already been updated from previous auto cost update. |
			---------------------------------------------------------------*/
			if (pogl_rec.auto_cost)
			{
				cc = find_rec (pogl, &pogl_rec, NEXT, "r");
				continue;
			}
			inmr_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
			cc = find_rec ("inmr", &inmr_rec, EQUAL, "r");
			if (cc || inmr_rec.outer_size == 0.00)
				inmr_rec.outer_size = 1.00;

			nor_cst = pogl_rec.fob_nor_cst - pogl_rec.frt_ins_cst;
			grTotal [0] += 	(pogl_rec.fob_fgn_cst * pogl_rec.qty_rec) / 
							inmr_rec.outer_size;
			grTotal [1] += 	(nor_cst * pogl_rec.qty_rec) / 
							inmr_rec.outer_size;
			grTotal [2] += 	(pogl_rec.frt_ins_cst * pogl_rec.qty_rec) / 
							inmr_rec.outer_size;
			grTotal [3] += 	(pogl_rec.duty        * pogl_rec.qty_rec) / 
							inmr_rec.outer_size;
			grTotal [4] += 	(pogl_rec.lcost_load  * pogl_rec.qty_rec) / 
							inmr_rec.outer_size;
			if (COSTED)
			{
				grTotal [5] += (pogl_rec.act_cst * pogl_rec.qty_rec) / 
								inmr_rec.outer_size;
			}
			else
			{
				grTotal [5] += (pogl_rec.land_cst * pogl_rec.qty_rec) / 
								inmr_rec.outer_size;
			}

			cc = find_rec (pogl, &pogl_rec, NEXT, "r");
		}
		cc = find_rec (pogh, &pogh_rec, NEXT, "r");
	}
	InvoiceApproved = FALSE;

	ProcInvoices (docNo, recType, dropShipment);

	if (!InvoiceApproved)
		return;

	strcpy (pogh_rec.co_no,comm_rec.co_no);
	if (recType [0] == 'G') 
		strncpy (pogh_rec.gr_no, docNo, strlen (pogh_rec.gr_no));

	if (recType [0] == 'P') 
		strcpy (pogh_rec.pur_ord_no, zero_pad (docNo, 15));

	if (recType [0] == 'D') 
		pogh_rec.hhds_hash = atol (docNo);

	if (recType [0] == 'S') 
		pogh_rec.hhsh_hash = posh_rec.hhsh_hash;

	/*----------------------
	| Find goods receipts. |
	----------------------*/
	cc = find_rec (pogh, &pogh_rec, GTEQ, "r");
	while (!cc && !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		if (recType [0] == 'G' && strncmp (pogh_rec.gr_no, docNo, strlen (pogh_rec.gr_no)))
			break;

		if (recType [0] == 'P' && strcmp (pogh_rec.pur_ord_no,zero_pad (docNo,15)))
			break;

		if (recType [0] == 'D' && pogh_rec.hhds_hash != atol (docNo))
			break;

		if (recType [0] == 'S' && pogh_rec.hhsh_hash != posh_rec.hhsh_hash)
			break;

		/*---------------------------------------------------------
		| Process all lines on goods receipt to update auto_cost. |
		---------------------------------------------------------*/
		pogl_rec.hhgr_hash 	= pogh_rec.hhgr_hash;
		pogl_rec.line_no 	= 0;
		cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
		while (!cc && pogl_rec.hhgr_hash == pogh_rec.hhgr_hash)
		{
			pogl_rec.auto_cost = 1;
			cc = abc_update (pogl, &pogl_rec);
			if (cc)
				file_err (cc, "pogl", "DBUPDATE");
				
			cc = find_rec (pogl, &pogl_rec, NEXT, "u");
		}
		abc_unlock (pogl);
		cc = find_rec (pogh, &pogh_rec, NEXT, "r");
	}
	return;
}
		
/*===================================================================
| Routine to open the pipe to standard print and send initial data. |
===================================================================*/
void
PrintHeaderInfo (
 void)
{
	if ( (pout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout, ".LP%d\n", printerNumber);
	fprintf (pout, ".PI12\n");
	fprintf (pout, ".13\n");
	fprintf (pout, ".L158\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".EPURCHASE AUTHORISATION EXCEPTION AUDIT.\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".E%s AS AT %s\n",clip (comm_rec.co_name),SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".C NOTE : Value tolerance = %.2f , exception shown as '*' / Percentage tolerance = %.2f , exception shown as '+' \n",envVarPoValTol,envVarPoPerTol);

	fprintf (pout, ".R=================================================");
	fprintf (pout, "=============================================");
	fprintf (pout, "==============================================\n");
				
	fprintf (pout, "=================================================");
	fprintf (pout, "=============================================");
	fprintf (pout, "==============================================\n");

	fprintf (pout, "| DOC |     REF NO.   | SUPP |CUR|     INVOICE   ");
	fprintf (pout, "| COST TYPE 1. | COST TYPE 1. | COST TYPE 2. ");
	fprintf (pout, "| COST TYPE 5. |   OTHER      |    TOTAL     |\n");

	fprintf (pout, "|     |               |      |   |               ");
	fprintf (pout, "|      FOB     |     FOB      |   FREIGHT    ");
	fprintf (pout, "|     DUTY     |              |              |\n");

	fprintf (pout, "|     |               |      |   |               ");
	fprintf (pout, "| FOREIGN VALUE| LOCAL VALUE  | LOCAL  VALUE ");
	fprintf (pout, "| LOCAL  VALUE |    COSTS     |     COSTS    |\n");

	fprintf (pout, "|-----|---------------|------|---|---------------");
	fprintf (pout, "|--------------|--------------|--------------");
	fprintf (pout, "|--------------|--------------|--------------|\n");

	fflush (pout);
}

/*=============================
| Display header information. |
=============================*/
void
DisplayHeadingInfo (
 void)
{
	if (local_rec.recType [0] == 'P')
		sprintf (err_str, "Purchase order number %s", local_rec.docNo);

	if (local_rec.recType [0] == 'S')
		sprintf (err_str, "Shipment number %s", local_rec.docNo);

	if (local_rec.recType [0] == 'G')
		sprintf (err_str, "Goods Receipt number %s", local_rec.docNo);

	Dsp_prn_open (4, 3, SEL_PSIZE, err_str, comm_rec.co_no,  comm_rec.co_name,
		     								(char *) 0, (char *) 0 ,
		     								(char *) 0, (char *) 0);

	Dsp_saverec ("SUPPLIER|CURR|     INVOICE    | COST TYPE 1. | COST TYPE 1. | COST TYPE 2. | COST TYPE 5. |   OTHER      |    TOTAL     ");
	Dsp_saverec ("        |    |                | FOB FGN VALUE| FOB LOC VALUE| FR. LOC VALUE| DUTY LOC VAL |    COSTS     |     COSTS    ");

	Dsp_saverec ("");
}

/*===================================================
| Routine to end report. Prints bottom line totals. |
===================================================*/
void
EndReport (
 void)
{
	fprintf (pout,".EOF\n");
	pclose (pout);
}

/*===================
| Process invoices. |
===================*/
void
ProcInvoices (
	char 	*docNo,
	char 	*recType,
	int		dropShipment)
{
	int		i;
	int		recordFound = FALSE,
			failedValueTest = FALSE,
			failedPercentTest = FALSE;

	float	per_tot = 0.00;

	valueTotal = 0.00;

	for (i = 0; i < 6; i++)
		invoiceTotal [i] = 0.00;

	/*---------------------------------------------
	| Read all invoices for current receipt type. |
	---------------------------------------------*/
	sprintf (suin2_rec.rec_type, "%-1.1s", recType);
	sprintf (suin2_rec.doc_no,   "%-15.15s", docNo);
	cc = find_rec (suin2, &suin2_rec, GTEQ, "r");
	while (!cc && !strcmp (suin2_rec.rec_type, recType) &&
				   !strncmp (suin2_rec.doc_no,   docNo, strlen (docNo)))
	{
		/*--------------------------------------------------
		| Selection screen applies and invoice not correct |
		--------------------------------------------------*/
		if (!IM_UPDATE && (suin2_rec.approved [0] == 'Y' || 
						     suin2_rec.rec_type [0] == 'N'))
		{
			cc = find_rec (suin2, &suin2_rec, NEXT, "r");
			continue;
		}
		
		/*------------------------------------------------------------
		| Find supplier, if not correct company find next record as  |
        | valid document number may exist in another company.        |
		------------------------------------------------------------*/
		sumr_rec.hhsu_hash = suin2_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc || strcmp (sumr_rec.co_no, comm_rec.co_no))
		{
			cc = find_rec (suin2, &suin2_rec, NEXT, "r");
			continue;
		}
		/*-----------------------------------------
		| Should only be one suid record for each |
		| invoice so find line zero record only.  |
		-----------------------------------------*/
		suid_rec.hhsi_hash = suin2_rec.hhsi_hash;
		suid_rec.line_no = 0;
		cc = find_rec (suid, &suid_rec, GTEQ, "r");
		if (!cc && suid_rec.hhsi_hash == suin2_rec.hhsi_hash)
		{
			/*------------------------------------------
			| Write an audit and add records for later |
            | percentage and value tolerance checks .  |
			------------------------------------------*/
			if (IM_UPDATE)
				PrntAudit (docNo, recType);
			else
				DispAudit ();

			recordFound = TRUE;
		}
		cc = find_rec (suin2, &suin2_rec, NEXT, "r");
	}

	/*------------------------------------
	| Print totals and check tolerances. |
	------------------------------------*/
	if (grTotal [5] == 0.00)
		per_tot = 0.00;
	else
		per_tot = (float) (fabs ( ( (invoiceTotal [5] - grTotal [5]) / grTotal [5]) * 100));

	valueTotal = (float) (grTotal [5] - invoiceTotal [5]);

	/*-----------------------------
	| Check percentage tolerance. |
	-----------------------------*/
	if (per_tot > envVarPoPerTol)
		failedPercentTest = TRUE;

	/*------------------------
	| Check value tolerance. |
	------------------------*/
	if (fabs (valueTotal) > envVarPoValTol)
		failedValueTest = TRUE;
	
	if (!recordFound)
	{
		if (!IM_UPDATE)
		{	
			print_mess (ML ("No unapproved invoices could be found."));
			sleep (sleepTime);
		}
		return;
	}

	if (IM_UPDATE)
	{
		fprintf (pout, "|     |               |      TOTAL INVOICE VALUE ");
		fprintf (pout, "|%13.2f |%13.2f |%13.2f ",
							invoiceTotal [0], invoiceTotal [1], invoiceTotal [2]);

		fprintf (pout, "|%13.2f |%13.2f |%13.2f |\n",
							invoiceTotal [3], invoiceTotal [4], invoiceTotal [5]);

		fprintf (pout, "|     |               |      TOTAL RECEIPT VALUE ");
		fprintf (pout, "|%13.2f |%13.2f |%13.2f ",
							grTotal [0], grTotal [1], grTotal [2]);

		fprintf (pout, "|%13.2f |%13.2f |%13.2f |%s%s\n",
							grTotal [3], grTotal [4], grTotal [5],
							(failedValueTest) ? "*" : "" ,
							(failedPercentTest) ? "+" : "");

		fprintf (pout, "|-----|---------------|------|---|---------------");
		fprintf (pout, "|--------------|--------------|--------------");
		fprintf (pout, "|--------------|--------------|--------------|\n");
	
		/*-----------------------------------------------------
		| Nothing gets updated as value of percebtage failed. |
		-----------------------------------------------------*/
		if (failedPercentTest || failedValueTest)
			return;
		
		InvoiceApproved = TRUE;
		ApproveInvoice (docNo, recType);
	}
	else
	{
		Dsp_saverec ("^^GGGGGGGGEGGGGEGGGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");

		sprintf (displayString, "      TOTAL INVOICE VALUE     ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ",
							invoiceTotal [0], invoiceTotal [1], invoiceTotal [2],
							invoiceTotal [3], invoiceTotal [4], invoiceTotal [5]);

		Dsp_saverec (displayString);

		sprintf (displayString, "      TOTAL RECEIPT VALUE     ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ^E%13.2f ",
							grTotal [0], grTotal [1], grTotal [2],
							grTotal [3], grTotal [4], grTotal [5]);

		Dsp_saverec (displayString);

		Dsp_saverec ("^^GGGGGGGGJGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGG");
	
		Dsp_srch ();
		crsr_off ();
		while (1)
		{
			if (dropShipment)
			{
				i = prmptmsg (ML ("A)uthorise all invoices / Q)uit"), "AaQq", 0, 20);
	
				if (i == 'A' || i == 'a' || i == 'Q' || i == 'q')
					break;
			}
			else
			{
				i = prmptmsg (ML ("A)uthorise all invoices / C)hange receipt type to normal / S)anction all invoices changing receipt type to normal / Q)uit"), "AaCcSsQq", 0, 20);
	
				if (i == 'A' || i == 'a' || i == 'C' || i == 'c' || 
				     i == 'S' || i == 's' || i == 'Q' || i == 'q')
					break;
			}
		}
		Dsp_close ();
		crsr_on ();

		/*---------------------------
		| Quit and make no changes. |
		---------------------------*/
		if (i == 'Q' || i == 'q')
			return;

		/*-----------------------------------------------------
		| Change invoice to normal type and leave unapproved. |
		-----------------------------------------------------*/
		if (i == 'C' || i == 'c')
		{
			if (CheckInput (ML ("Are you sure you want to change receipt type to normal [Y/N]?")))
				return;

			ChangeInvoice (docNo, recType);
		}

		/*--------------------------------------------------
		| Change invoice to normal type and then approved. |
		--------------------------------------------------*/
		if (i == 'S' || i == 's')
		{
			if (CheckInput (ML ("Are you sure you want to sanction all invoices changing receipt type to normal [Y/N]?")))
				return;

			ApproveInvoice (docNo, recType);
			ChangeInvoice (docNo, recType);
		}
		/*------------------
		| Approve invoice. |
		------------------*/
		if (i == 'A' || i == 'a')
		{
			if (CheckInput (ML ("Are you sure you want to authorize all invoices [Y/N]?")))
				return;

			ApproveInvoice (docNo, recType);
			InvoiceApproved = TRUE;
		}
	}
	return;
}

/*=======================================
| Perform a double check in user input. |
=======================================*/
int
CheckInput (
 char *	mess)
{
	int		mlen,
			i;

	mlen = (132 - strlen (mess)) / 2;
	i = prmptmsg (mess, "YyNn", mlen, 21);
	if (i == 'N' || i == 'n')
		return (TRUE);

	return (FALSE);
}

/*================================
| Change invoice to normal type. |
================================*/
void
ChangeInvoice (
 char *docNo,
 char *recType)
{
	abc_selfield (suin, "suin_hhsi_hash");

	/*-----------------------------------------------------------
	| Process invoices changing then to 'N'ormal type invoices. |
	-----------------------------------------------------------*/
	sprintf (suin2_rec.rec_type, "%-1.1s", recType);
	sprintf (suin2_rec.doc_no,   "%-15.15s", docNo);
	cc = find_rec (suin2, &suin2_rec, GTEQ, "r");
	while (!cc && !strcmp (suin2_rec.rec_type, recType) &&
				   !strncmp (suin2_rec.doc_no,   docNo,strlen (docNo)))
	{
		suin_rec.hhsi_hash = suin2_rec.hhsi_hash;
		cc = find_rec (suin, &suin_rec, EQUAL, "u");
		if (cc)
			file_err (cc, suin, "DBFIND");

		strcpy (suin_rec.rec_type, "N");
		strcpy (suin_rec.doc_no,   fifteenSpaces);
		suin_rec.cst_type = 0;
		cc = abc_update (suin, &suin_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");

		cc = find_rec (suin2, &suin2_rec, NEXT, "r");
	}
}

/*===================
| Approve invoices. |
===================*/
void
ApproveInvoice (
 char *	docNo,
 char *	recType)
{
	int		DEBIT;
	int		firstSuin = TRUE;
	int		dropShipment = FALSE;
	long	glDate = 0L;

	/*---------------------------------------------------------------
	| A suppliers invoice matched to a drop shipment purchase order |
	| should only match to a single purchase order. i.e. 1 supplier |
	| invoice to 1 drop ship purchase. (checked in cr_im_inp).      |
	| We rely on this here for calculating whether any variance     |
	| should be posted to the drop ship clearing account or to the  |
	| normal P/O variance account.                                  |
	---------------------------------------------------------------*/

	/*---------------------------------------------------------------
	| Note that if there is more than one invoice relating to a     |
	| particular purchase order AND there is variance to be posted  |
	| then the variance will be posted to the same period as the    |
	| first invoice processed.                                      |
	---------------------------------------------------------------*/

	double	gl_fx_amt 		= 0.00;
	double	gl_loc_amt 		= 0.00;
	double	gl_fx_gst 		= 0.00;
	double	gl_loc_gst 		= 0.00;
	double	gl_fx_amt_gst 	= 0.00;
	double	gl_loc_amt_gst 	= 0.00;

	/*------------------------------------------------------------
	| Process same as before but this time update suin and suid. |
	------------------------------------------------------------*/
	sprintf (suin2_rec.rec_type, "%-1.1s", recType);
	sprintf (suin2_rec.doc_no,   "%-15.15s", docNo);
	cc = find_rec (suin2, &suin2_rec, GTEQ, "u");
	while (!cc && !strcmp (suin2_rec.rec_type, recType) &&
				   !strncmp (suin2_rec.doc_no,   docNo, strlen (docNo)))
	{
		/*------------------------------------------------------------
		| Find supplier, if not correct company find next record as  |
		| valid document number may exist in another company.        |
		------------------------------------------------------------*/
		sumr_rec.hhsu_hash = suin2_rec.hhsu_hash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc || strcmp (sumr_rec.co_no, comm_rec.co_no))
		{
			abc_unlock (suin2);
			cc = find_rec (suin2, &suin2_rec, NEXT, "r");
			continue;
		}
		/*-----------------------------------------
		| Should only be one suid record for each |
		| invoice so find line zero only.         |
		-----------------------------------------*/
		suid_rec.hhsi_hash = suin2_rec.hhsi_hash;
		suid_rec.line_no = 0;
		cc = find_rec (suid, &suid_rec, GTEQ, "u");
		if (!cc && suid_rec.hhsi_hash == suin2_rec.hhsi_hash)
		{
			/*-------------------------------------------
			| Check if suid relates to a drop shipment. |
			-------------------------------------------*/

			if (!dropShipment)
			{
				/*-------------------------------------------------
				| Select relevent key if not already selected.    |
				|  P(urchase order), S(hipment), G(oods Receipt). |
				-------------------------------------------------*/
				if (recType [0] == 'P' && selectKey != PO_KEY) 
				{
					abc_selfield (pogh, "pogh_po_id");
					selectKey = PO_KEY;
				}

				if (recType [0] == 'S' && selectKey != SH_KEY) 
				{
					abc_selfield (pogh, "pogh_sh_id");
					selectKey = SH_KEY;
				}

				if (recType [0] == 'D' && selectKey != DD_KEY) 
				{
					abc_selfield (pogh, "pogh_dd_id");
					selectKey = DD_KEY;
				}

				if (recType [0] == 'G' && selectKey != GR_KEY) 
				{
					abc_selfield (pogh, "pogh_id_no2");
					selectKey = GR_KEY;
				}
				strcpy (pogh_rec.co_no,comm_rec.co_no);
				if (recType [0] == 'G') 
					strncpy (pogh_rec.gr_no, docNo, strlen (pogh_rec.gr_no));

				if (recType [0] == 'P') 
					strcpy (pogh_rec.pur_ord_no, zero_pad (docNo, 15));

				if (recType [0] == 'D') 
					pogh_rec.hhds_hash = atol (docNo);

				if (recType [0] == 'S') 
				{
        			strcpy (posh_rec.co_no, comm_rec.co_no);
        			strcpy (posh_rec.csm_no, zero_pad (docNo, 12));
        			cc = find_rec (posh, &posh_rec, EQUAL, "r");
        			pogh_rec.hhsh_hash = (cc) ? -1 : posh_rec.hhsh_hash;
				}
				cc = find_rec (pogh, &pogh_rec, EQUAL, "r");
				if (cc)
				{
					abc_unlock ("suid");
					return;
				}
	
				if (pogh_rec.drop_ship [0] == 'Y')
					dropShipment = TRUE;
			}

			if (firstSuin)
			{
				glDate		 = suin2_rec.gl_date;
				firstSuin = FALSE;
			}
				
			/*------------------------------------
			| Post to purchase clearing account. |
			------------------------------------*/
			gl_fx_amt	= 	suin2_rec.amt - suin2_rec.gst;
			gl_loc_amt	= 	suin2_rec.amt - suin2_rec.gst;
			gl_loc_amt 	/= 	suin2_rec.exch_rate;
			gl_loc_amt 	= 	no_dec (gl_loc_amt);

			gl_fx_gst	= 	suin2_rec.gst;
			gl_loc_gst	= 	suin2_rec.gst;
			gl_loc_gst 	/= 	suin2_rec.exch_rate;
			gl_loc_gst 	= 	no_dec (gl_loc_gst);

			gl_fx_amt_gst 	= 	suin2_rec.amt;
			gl_loc_amt_gst 	= 	suin2_rec.amt;
			gl_loc_amt_gst	/= 	suin2_rec.exch_rate;
			gl_loc_amt_gst 	= 	no_dec (gl_loc_amt_gst);

			cc = FindGlmr (suid_rec.gl_acc_no);
			if (cc)
					file_err (cc, glmr, "DBFIND");

			strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");
			WriteGlTransaction 
			(
				TRUE, 
				glmrRec.acc_no, 
				gl_fx_amt, 
				gl_loc_amt
			);

			/*------------------------------------
			| Post to suppliers control account. |
			------------------------------------*/
			cc = FindGlmr (sumr_rec.gl_ctrl_acct);
			if (cc)
			{
				cc = FindPocr (sumr_rec.co_no, sumr_rec.curr_code, "r");
				if (!cc)
					cc = FindGlmr (pocrRec.gl_ctrl_acct);

				if (cc)
				{
					GL_GLI 
					(
						comm_rec.co_no,
						comm_rec.est_no,
						"  ",
						"ACCT PAY  ",
						"   ",
						" "
					);
				}
			}
			strcpy (glwkRec.jnl_type, (INVOICE) ? "2" : "1");
			WriteGlTransaction 
			(
				FALSE, 
				glmrRec.acc_no, 
				gl_fx_amt_gst, 
				gl_loc_amt_gst
			);

			/*----------------------
			| Post to gst account. |
			----------------------*/
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"G.S.T PAID",
				"   ",
				" "
			);
			strcpy (glwkRec.jnl_type, (INVOICE) ? "1" : "2");
			WriteGlTransaction 
			(
				FALSE, 
				glmrRec.acc_no, 
				gl_fx_gst, 
				gl_loc_gst
			);

			abc_delete (suid);
		}
		abc_unlock ("suid");

		strcpy (suin2_rec.approved, "Y");
		cc = abc_update (suin2, &suin2_rec);
		if (cc)
			file_err (cc, suin, "DBUPDATE");

		cc = find_rec (suin2, &suin2_rec, NEXT, "u");
	}
	abc_unlock (suin2);

	/*----------------------------
	| Post to variation account. |
	----------------------------*/
	if (valueTotal < 0.00)
	{
		valueTotal *= -1;
		DEBIT = TRUE;
	}
	else
		DEBIT = FALSE;

	strcpy (glwkRec.jnl_type, (DEBIT) ? "2" : "1");
	if (dropShipment)
	{
		/*----------------------
		| Post to gst account. |
		----------------------*/
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"PO CLEAR D",
			"   ",
			" "
		);
	}
	else
	{
		/*----------------------
		| Post to gst account. |
		----------------------*/
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			"PO CLEAR N",
			"   ",
			" "
		);
	}
	if (cc)
			file_err (cc, glmr, "DBFIND");
		
	WriteGlVariance 
	(
		1, 
		glmrRec.acc_no, 
		CENTS (valueTotal), 
		docNo, 
		recType , 
		glDate
	);

	strcpy (glwkRec.jnl_type, (DEBIT) ? "1" : "2");

	/*----------------------
	| Post to gst account. |
	----------------------*/
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"POVARIANCE",
		"   ",
		" "
	);
		
	WriteGlVariance 
	(
		0, 
		glmrRec.acc_no, 
		CENTS (valueTotal), 
		docNo, 
		recType, 
		glDate
	);
}

/*=======================================================================
| Write General ledger transaction for Suppliers and Purchase clearing. |
=======================================================================*/
void
WriteGlTransaction (
 int	first_rec,
 char *	acct_no,
 double	fx_amnt,
 double	loc_amnt)
{
	int		month;

	if (fx_amnt == 0.00 && loc_amnt == 0.00)
		return;

	if (loc_amnt < 0.00)
		loc_amnt *= -1;
	if (fx_amnt < 0.00)
		fx_amnt *= -1;
	
	/*-----------------------
	| Setup Period.         |
	-----------------------*/
	DateToDMY (suin2_rec.gl_date, NULL, &month, NULL);

	/*-------------------------------------------
	| Add transaction for account if required . |
	-------------------------------------------*/
	strcpy (glwkRec.tran_type, (INVOICE) ? " 7" : " 8");
	strcpy (glwkRec.co_no,	comm_rec.co_no);
	strcpy (glwkRec.est_no,	comm_rec.est_no);
	strcpy (glwkRec.chq_inv_no, (first_rec) ? suin2_rec.inv_no 
											    : fifteenSpaces);
	sprintf (glwkRec.sys_ref,	"%010ld", (long) comm_rec.term);
	sprintf (glwkRec.acc_no,	"%-*.*s", MAXLEVEL,MAXLEVEL,acct_no);
	strcpy (glwkRec.narrative,	suid_rec.narrative);
	strcpy (glwkRec.alt_desc1,	suid_rec.pf_ref);
	strcpy (glwkRec.alt_desc2,	suid_rec.po_no);
	strcpy (glwkRec.alt_desc3,	suid_rec.gr_no);
	strcpy (glwkRec.batch_no,	" ");
	sprintf (glwkRec.name,"%-30.30s", (first_rec) ? sumr_rec.crd_name
													  : " ");
	sprintf (glwkRec.acronym, "%-9.9s", (first_rec) ? sumr_rec.crd_no 
														 : " ");
	strcpy (glwkRec.user_ref,suid_rec.user_ref);
	strcpy (glwkRec.stat_flag,"2");
	sprintf (glwkRec.period_no, "%02d", month);
	glwkRec.hhgl_hash 	= 	glmrRec.hhmr_hash;
	glwkRec.tran_date 		= 	suin2_rec.gl_date;
	glwkRec.post_date 	= 	TodaysDate ();
	glwkRec.ci_amt		=	fx_amnt;
	glwkRec.o1_amt		=	0.00;
	glwkRec.exch_rate	=	suin2_rec.exch_rate;
	glwkRec.o2_amt		=	0.00;
	glwkRec.o3_amt		=	0.00;
	glwkRec.o4_amt		=	(first_rec) ? suin2_rec.gst : 0.00;
	glwkRec.amount		=	fx_amnt;
	glwkRec.loc_amount	=	loc_amnt;
	strcpy (glwkRec.currency, suin2_rec.currency);

	if (glwkRec.post_date < 0)
		glwkRec.post_date = 0;

	GL_AddBatch ();
}

/*================================================
| Write General ledger transaction for Variance. |
================================================*/
void
WriteGlVariance (
 int	first_rec,
 char *	acct_no,
 double	amnt,
 char *	docNo,
 char *	recType,
 long	varDate)
{
	int		month;
	char	wk_ref [16];

	if (amnt == 0.00)
		return;

	/*-----------------------
	| Setup Period.         |
	-----------------------*/
	DateToDMY (varDate, NULL, &month, NULL);

	sprintf (wk_ref, "%-15.15s", docNo);
	if (recType [0] == 'D')
	{
		cc = find_hash (ddsh, &ddsh_rec, EQUAL, "r", atol (docNo));
		if (cc)
		{
			sprintf (wk_ref, "ERROR%6ld", atol (docNo));
		}
		else
		{
			ddln_rec.hhdd_hash = ddsh_rec.hhdd_hash;
			ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
			cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
			if (!cc && 
				ddln_rec.hhdd_hash == ddsh_rec.hhdd_hash &&
				ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
			{
				cc = find_hash (poln, &poln_rec, EQUAL, "r", ddln_rec.hhpl_hash);
				if (cc)
					file_err (cc, "poln", "DBFIND");

				cc = find_hash (pohr, &pohr_rec, EQUAL, "r", poln_rec.hhpo_hash);
				if (cc)
					file_err (cc, "pohr", "DBFIND");
	
				sprintf (wk_ref, "%-15.15s", pohr_rec.pur_ord_no);
			}
			else
				file_err (cc, "ddln", "DBFIND");
		}
	}

	/*-------------------------------------------
	| Add transaction for account if required . |
	-------------------------------------------*/
	strcpy (glwkRec.tran_type, (INVOICE) ? " 7" : " 8");
	strcpy (glwkRec.co_no,	comm_rec.co_no);
	strcpy (glwkRec.est_no,	comm_rec.est_no);
	if (first_rec)
		sprintf (glwkRec.chq_inv_no,"%-15.15s", wk_ref);
	else
		strcpy (glwkRec.chq_inv_no, fifteenSpaces);
	sprintf (glwkRec.sys_ref,	"%010ld", (long) comm_rec.term);
	sprintf (glwkRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,acct_no);
	strcpy (glwkRec.narrative,	suid_rec.narrative);
	strcpy (glwkRec.alt_desc1,	suid_rec.pf_ref);
	strcpy (glwkRec.alt_desc2,	suid_rec.po_no);
	strcpy (glwkRec.alt_desc3,	suid_rec.gr_no);
	strcpy (glwkRec.batch_no,	" ");
	if (first_rec)
	{
		if (recType [0] == 'D')
			sprintf (glwkRec.name,"DIR-DEL VARIANCE");
		else
			sprintf (glwkRec.name,"%s VAR.", wk_ref);
	}
	else
		strcpy (glwkRec.name," ");
	strcpy (glwkRec.acronym, " ");
	strcpy (glwkRec.user_ref,suid_rec.user_ref);
	strcpy (glwkRec.stat_flag,"2");
	sprintf (glwkRec.period_no, "%02d", month);
	glwkRec.hhgl_hash 	= 	glmrRec.hhmr_hash;
	glwkRec.tran_date 		= 	varDate;
	glwkRec.post_date 	= 	TodaysDate ();
	glwkRec.ci_amt		=	amnt;
	glwkRec.o1_amt		=	0.00;
	glwkRec.exch_rate	=	1.00;
	glwkRec.o2_amt		=	0.00;
	glwkRec.o3_amt		=	0.00;
	glwkRec.o4_amt		=	0.00;
	glwkRec.amount		=	amnt;
	glwkRec.loc_amount	=	amnt;
	strcpy (glwkRec.currency, localCurrency);

	if (glwkRec.post_date < 0)
		glwkRec.post_date = 0;

	GL_AddBatch ();
}

/*============================
| Write Audit for each line. |
============================*/
void
PrntAudit (
 char *	docNo,
 char *	recType)
{
	double	inv_amt;

	if (recType [0] == 'G')
		fprintf (pout, "|G.REC");

	if (recType [0] == 'S')
		fprintf (pout, "|SHIP.");

	if (recType [0] == 'P')
		fprintf (pout, "|P/ORD");

	if (recType [0] == 'D')
		fprintf (pout, "|D.DEL");

	if (recType [0] == 'D')
	{
		cc = find_hash (ddsh, &ddsh_rec, EQUAL, "r", atol (docNo));
		if (cc)
		{
			fprintf (pout, "| ERROR %8ld", atol (docNo));
		}
		else
		{
			ddln_rec.hhdd_hash = ddsh_rec.hhdd_hash;
			ddln_rec.hhds_hash = ddsh_rec.hhds_hash;
			cc = find_rec (ddln, &ddln_rec, GTEQ, "r"); 
			if (!cc && 
				ddln_rec.hhdd_hash == ddsh_rec.hhdd_hash &&
				ddln_rec.hhds_hash == ddsh_rec.hhds_hash)
			{
				cc = find_hash (poln, &poln_rec, EQUAL, "r", ddln_rec.hhpl_hash);
				if (cc)
					file_err (cc, "poln", "DBFIND");
	
				cc = find_hash (pohr, &pohr_rec, EQUAL, "r", poln_rec.hhpo_hash);
				if (cc)
					file_err (cc, "pohr", "DBFIND");
	
				fprintf (pout, "|%-15.15s", pohr_rec.pur_ord_no);
			}
			else
				file_err (cc, "ddln", "DBFIND");
		}
	}
	else
		fprintf (pout, "|%-15.15s", docNo);
	fprintf (pout, "|%6.6s",  sumr_rec.crd_no);
	fprintf (pout, "|%3.3s" ,  suin2_rec.currency);
	fprintf (pout, "|%-15.15s",  suin2_rec.inv_no);

	if (suin2_rec.exch_rate == 0.00)
		suin2_rec.exch_rate = 1.00;

	inv_amt = suin2_rec.amt - suin2_rec.gst;

	/*----------------------
	| Process Goods value. |
	----------------------*/
	if (suin2_rec.cst_type == 1)
	{
		fprintf (pout,"|%13.2f ",DOLLARS (inv_amt));
		fprintf (pout,"|%13.2f ",DOLLARS (inv_amt / suin2_rec.exch_rate));
		invoiceTotal [0] += DOLLARS (inv_amt);
		invoiceTotal [1] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	else
	{
		fprintf (pout, "|              ");
		fprintf (pout, "|              ");
	}

	/*------------------
	| Process Freight. |
	------------------*/
	if (suin2_rec.cst_type == 2)
	{
		fprintf (pout, "|%13.2f ", DOLLARS (inv_amt /suin2_rec.exch_rate)) ;
		invoiceTotal [2] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	else
		fprintf (pout, "|              ");

	/*---------------
	| Process Duty. |
	---------------*/
	if (suin2_rec.cst_type == 6)
	{
		fprintf (pout, "|%13.2f ", DOLLARS (inv_amt /suin2_rec.exch_rate)) ;
		invoiceTotal [3] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	else
		fprintf (pout, "|              ");

	/*---------------------
	| Process all others. |
	---------------------*/
	if (suin2_rec.cst_type == 3 || 
		 suin2_rec.cst_type == 4 || 
		 suin2_rec.cst_type == 5 || 
		 suin2_rec.cst_type > 6)
	{
		fprintf (pout, "|%13.2f ", DOLLARS (inv_amt /suin2_rec.exch_rate));
		invoiceTotal [4] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	else
		fprintf (pout, "|              ");

	/*-----------------------
	| keep a running total. |
	-----------------------*/
	invoiceTotal [5] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	fprintf (pout, "|%13.2f |\n",DOLLARS (inv_amt / suin2_rec.exch_rate));
}

/*============================
| Write Audit for each line. |
============================*/
void
DispAudit (
 void)
{
	char	disp_amt [150];
	double	inv_amt;

	if (suin2_rec.exch_rate == 0.00)
		suin2_rec.exch_rate = 1.00;

	inv_amt = suin2_rec.amt - suin2_rec.gst;

	if (suin2_rec.cst_type == 1)
	{
		sprintf (disp_amt, "%13.2f ^E%13.2f ^E              ^E              ^E              ^E%13.2f ",
					DOLLARS (inv_amt),
					DOLLARS (inv_amt / suin2_rec.exch_rate),
					DOLLARS (inv_amt / suin2_rec.exch_rate));

		invoiceTotal [0] += DOLLARS (inv_amt);
		invoiceTotal [1] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	if (suin2_rec.cst_type == 2)
	{
		sprintf (disp_amt, "              ^E              ^E%13.2f ^E              ^E              ^E%13.2f ",
					DOLLARS (inv_amt / suin2_rec.exch_rate),
					DOLLARS (inv_amt / suin2_rec.exch_rate));
		invoiceTotal [2] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	if (suin2_rec.cst_type == 6)
	{
		sprintf (disp_amt, "              ^E              ^E              ^E%13.2f ^E              ^E%13.2f ",
					DOLLARS (inv_amt / suin2_rec.exch_rate),
					DOLLARS (inv_amt / suin2_rec.exch_rate));
		invoiceTotal [3] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	/*---------------------
	| Process all others. |
	---------------------*/
	if (suin2_rec.cst_type == 3 || 
		 suin2_rec.cst_type == 4 || 
		 suin2_rec.cst_type == 5 || 
		 suin2_rec.cst_type > 6)
	{
		sprintf (disp_amt, "              ^E              ^E              ^E              ^E%13.2f ^E%13.2f ",
					DOLLARS (inv_amt / suin2_rec.exch_rate),
					DOLLARS (inv_amt / suin2_rec.exch_rate));
		invoiceTotal [4] += DOLLARS (inv_amt / suin2_rec.exch_rate);
	}
	sprintf (displayString, " %6.6s ^E %3.3s^E%-15.15s ^E%s",
							sumr_rec.crd_no,
							suin2_rec.currency,
							suin2_rec.inv_no,
							disp_amt);
	invoiceTotal [5] += DOLLARS (inv_amt / suin2_rec.exch_rate);

	Dsp_saverec (displayString);
}

/*=======================
| Search for Shipments. |
=======================*/
int
SrchDdsh (
 char *	key_val)
{
	work_open ();
	save_rec ("#Ship No", "#Due Date    ");
	ddsh_rec.hhdd_hash = pohr_rec.hhdd_hash;
	ddsh_rec.hhsu_hash = pohr_rec.hhsu_hash;
	strcpy (ddsh_rec.ship_no, "  ");
	cc = find_rec (ddsh, &ddsh_rec, GTEQ, "r");

	while (!cc && 
		   ddsh_rec.hhdd_hash == pohr_rec.hhdd_hash &&
		   ddsh_rec.hhsu_hash == pohr_rec.hhsu_hash)
	{
		strcpy (err_str, DateToString (ddsh_rec.due_date));
		cc = save_rec (ddsh_rec.ship_no, err_str);
		if (cc)
			break;
		cc = find_rec (ddsh, &ddsh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	return (cc);
}

void
SrchPogh (
 char *	key_val)
{
	char	dsp_str [2][81];

	work_open ();

	strcpy (pogh_rec.co_no,comm_rec.co_no);

	if (local_rec.recType [0] == 'P' ||
		 local_rec.recType [0] == 'D') 
	{
		save_rec ("#P/O  No.","#      System Reference     | Date Raised");
	}

	if (local_rec.recType [0] == 'S') 
	{
		save_rec ("#Ship No.","#      System Reference     | Date Raised");
		pogh_rec.hhsh_hash = 0L;
	}

	if (local_rec.recType [0] == 'G') 
	{
		save_rec ("#GRIN No.","#      System Reference     | Date Raised");
		sprintf (pogh_rec.gr_no,"%-15.15s",key_val);
	}

	strcpy (pogh_rec.co_no,comm_rec.co_no);
	cc = find_rec (pogh,&pogh_rec,GTEQ,"r");
	while (!cc && !strcmp (pogh_rec.co_no,comm_rec.co_no))
	{
		if (local_rec.recType [0] == 'D')
		{
			if (!strcmp (pogh_rec.pur_ord_no, fifteenSpaces) ||
				pogh_rec.hhds_hash == 0L)
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			if (strncmp (pogh_rec.pur_ord_no,key_val,strlen (key_val)))
				break;

			sprintf (dsp_str [0], "%-15.15s", pogh_rec.pur_ord_no);

			sprintf (dsp_str [1], "Goods Rec: %15.15s | %s ", pogh_rec.gr_no,
										DateToString (pogh_rec.date_raised));
		}

		if (local_rec.recType [0] == 'P')
		{
			if (!strcmp (pogh_rec.pur_ord_no, fifteenSpaces) ||
				pogh_rec.hhds_hash > 0L)
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			if (strncmp (pogh_rec.pur_ord_no,key_val,strlen (key_val)))
				break;

			sprintf (dsp_str [0], "%-15.15s", pogh_rec.pur_ord_no);

			sprintf (dsp_str [1], "Goods Rec: %15.15s | %s ", pogh_rec.gr_no,
										DateToString (pogh_rec.date_raised));
		}

		if (local_rec.recType [0] == 'G')
		{
			if (!strcmp (pogh_rec.gr_no, fifteenSpaces))
			{
				cc = find_rec (pogh, &pogh_rec, NEXT, "r");
				continue;
			}
			if (strncmp (pogh_rec.gr_no,key_val,strlen (key_val)))
				break;

			sprintf (dsp_str [0], "%-15.15s", pogh_rec.gr_no);
			sprintf (dsp_str [1], "P/Order  : %15.15s | %s ", pogh_rec.pur_ord_no,
										DateToString (pogh_rec.date_raised));
		}

		if (local_rec.recType [0] == 'S')
		{
            if (pogh_rec.hhsh_hash == 0L)
            {
                cc = find_rec (pogh, &pogh_rec, NEXT, "r");
                continue;
            }

            strcpy (posh2_rec.co_no, comm_rec.co_no);
            posh2_rec.hhsh_hash = pogh_rec.hhsh_hash;
            cc = find_rec (posh2, &posh2_rec, EQUAL, "r");
            if (cc)
            {
                cc = find_rec (pogh, &pogh_rec, NEXT, "r");
                continue;
            }
            sprintf (dsp_str [0], "%-12.12s", posh2_rec.csm_no);

			sprintf (dsp_str [1], "Goods Rec: %15.15s | %s ", pogh_rec.gr_no,
										DateToString (pogh_rec.date_raised));
		}
		cc = save_rec (dsp_str [0], dsp_str [1]);
		if (cc)
			break;

		cc = find_rec (pogh,&pogh_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pogh_rec.co_no,comm_rec.co_no);

	if (local_rec.recType [0] == 'D') 
		return;

	if (local_rec.recType [0] == 'P') 
		sprintf (pogh_rec.pur_ord_no,"%-15.15s", temp_str);

	if (local_rec.recType [0] == 'S') 
    {
        strcpy (posh_rec.co_no, comm_rec.co_no);
        sprintf (posh_rec.csm_no, "%-12.12s", temp_str);
        cc = find_rec (posh, &posh_rec, EQUAL, "r");
        if (cc)
            file_err (cc, "posh", "DBFIND");
        pogh_rec.hhsh_hash = posh_rec.hhsh_hash;
    }

	if (local_rec.recType [0] == 'G') 
		sprintf (pogh_rec.gr_no,"%-15.15s", temp_str);

	cc = find_rec (pogh,&pogh_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pogh", "DBFIND");
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();
	rv_pr (ML ("Manual Invoice Approval Entry"),47,0,1);

	box (0, 1, 131, 1);

	line_at (21,1,131);

	print_at (22,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}

