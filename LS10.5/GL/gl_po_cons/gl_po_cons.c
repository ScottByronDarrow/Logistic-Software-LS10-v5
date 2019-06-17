/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: gl_po_cons.c,v 5.4 2001/10/17 08:55:14 cha Exp $
|  Program Name  : (gl_po_cons.c)
|  Program Desc  : (Purchase Order General Ledger Audit Report.)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 12/08/87         |
|---------------------------------------------------------------------|
| $Log: gl_po_cons.c,v $
| Revision 5.4  2001/10/17 08:55:14  cha
| Updated to ensure consistent rounding (ISSUE # 00061).
| Changes made by Scott.
|
| Revision 5.3  2001/08/09 09:13:51  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:27:27  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:53  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: gl_po_cons.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/GL/gl_po_cons/gl_po_cons.c,v 5.4 2001/10/17 08:55:14 cha Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_gl_mess.h>

#define	MAX_ACCS	2000
#define	RECEIPT		 (receiptCostingFlag [0] == 'R')
#define	ACT_COST	 (envVarPoActCost [0] == 'Y' && \
					  pohr_rec.stat_flag [0] != 'Q' && \
					  pohr_rec.drop_ship [0] != 'Y')
#define	CRD_LINE	 (pogl_rec.qty_rec < 0.00)


#include	"schema"

struct commRecord	comm_rec;
struct poglRecord	pogl_rec;
struct poghRecord	pogh_rec;
struct pohrRecord	pohr_rec;
struct ccmrRecord	ccmr_rec;
struct powkRecord	powk_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct poshRecord	posh_rec;

	FILE	*ftmp;

	double	linesTotal = 0.00,
			groupTotal = 0.00,
			grandTotal = 0.00;

	char	purchaseStatus 		[2],
			envVarPoActCost 	[2],
			receiptCostingFlag 	[2],
			dropShipFlag 		[2];

	char	*data	= "data";

	int		processPID		= 0,
			envVarPoActVar 	= 1;

	struct {
		char	debitAccount [MAXLEVEL + 1];
		double	debitAmount;
		char	creditAccount [MAXLEVEL + 1];
		double	creditAmount;
	} glRec [MAX_ACCS];

/*
 * Local Function Prototypes.
 */
void 	EndReport 		 (void);
void 	HeadingOutput 	 (int);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	ProcessAll 		 (void);
void 	ProcessPidFunc 	 (void);
void 	ProcessPogl 	 (long);
int 	CreateDetail 	 (long);
void 	PrintPost 		 (char *,char *,char *,char *,long,char *,long,double);
void 	AddGlWorkFile 	 (char *, char *, char *, long, char *, long, double);
void 	GlStoreInto 	 (char *, char *, double);

/*
 * Main Processing Routine.
 */
int
main (
 int                argc,
 char*              argv [])
{
	int		i;
	char	*sptr;

	if (argc < 5)
	{
		print_at (0,0,"Usage:  %s <lpno> <purchaseStatus> <R(eceipt)/C(osting)> <Direct Delivery [Y/N]> [Optional PID]",argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("PO_ACT_VAR");
	envVarPoActVar = (sptr == (char *) 0) ? 1 : atoi (sptr);

	sprintf (envVarPoActCost, 	"%-1.1s", get_env ("PO_ACT_COST"));
	sprintf (purchaseStatus,  	"%-1.1s", argv [2]);
	sprintf (receiptCostingFlag,"%-1.1s", argv [3]);
	sprintf (dropShipFlag,   	"%-1.1s", argv [4]);
	if (argc == 6)
		processPID	=	atoi (argv [5]);
	
	if (receiptCostingFlag [0] != 'R' && receiptCostingFlag [0] != 'C')
	{
		print_at (1,0,"Usage:  %s <lpno> <purchaseStatus> <R (eceipt)/C (osting)> <Direct Delivery [Y/N]>",argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Actual COst update not done as.
	 */
	if (!ACT_COST && !RECEIPT)
		return (EXIT_FAILURE);

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	for (i = 0; i < MAX_ACCS; i++)
	{
		sprintf (glRec [i].debitAccount,"%-*.*s", 
							MAXLEVEL,MAXLEVEL,"----------------");
		glRec [i].debitAmount = 0.00;
		sprintf (glRec [i].creditAccount,"%-*.*s", 
							MAXLEVEL,MAXLEVEL,"----------------");
		glRec [i].creditAmount = 0.00;
	}

	init_scr ();

	if (!processPID)
	{
		if (RECEIPT)
			dsp_screen ("Printing Goods Receipt to G/L Audit.",
					 	comm_rec.co_no, comm_rec.co_name);
		else
			dsp_screen ("Printing Actual Cost Update to G/L Audit.",
					 	comm_rec.co_no, comm_rec.co_name);
	 }

	/*
	 * Open pipe work file to Pformat.  Note that no pipe is opened for  
	 * Direct Deliveries as these no update G/L here and  consequently
	 * do not require an audit trail.   
 	 */

	if (dropShipFlag [0] != 'Y')
	{
		if ((ftmp = popen ("pformat","w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);

		HeadingOutput (atoi (argv [1]));
	}

	if (processPID)
		ProcessPidFunc ();
	else
		ProcessAll ();


	/*
	 * Program exit sequence.
	 */
	if (dropShipFlag [0] != 'Y')
	{
		EndReport ();
		pclose (ftmp);
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}



/*
 * End of main
 */
void
EndReport (void)
{
	int		i;

	double 	add_dbt = 0.00,
	 		add_crd = 0.00;

	fprintf (ftmp, "|===============|===============|=============");
	fprintf (ftmp, "|==================|==============");
	fprintf (ftmp, "|==================|==============|\n");
	fprintf (ftmp, "|%15.15s",		" ");
	fprintf (ftmp, "|%15.15s",		"  GRAND");
	fprintf (ftmp, "| %11.11s ",	"TOTALS     ");
	fprintf (ftmp, "| %16.16s ",		"  DEBIT  ");
	fprintf (ftmp, "| %12.2f ",		grandTotal);
	fprintf (ftmp, "| %16.16s ",		"  CREDIT ");
	fprintf (ftmp, "| %12.2f |\n",	grandTotal);
	fprintf (ftmp, ".PA\n");
	fprintf (ftmp, "| GENERAL LEDGER ACCOUNT TOTALS               ");
	fprintf (ftmp, "|                  |              ");
	fprintf (ftmp, "|                  |              |\n");
	for (i = 0; i < MAX_ACCS; i++)
	{
		if (strncmp (glRec [i].debitAccount, "----------------", MAXLEVEL))
		{
			fprintf (ftmp, "|                                             ");
			fprintf (ftmp, "| %16.16s ",  glRec [i].debitAccount);
			fprintf (ftmp, "| %12.2f ", glRec [i].debitAmount);
			fprintf (ftmp, "|                  |              |\n");
			add_dbt += glRec [i].debitAmount;
		}
		if (strncmp (glRec [i].creditAccount, "----------------", MAXLEVEL))
		{
			fprintf (ftmp, "|                                             ");
			fprintf (ftmp, "|                  |              ");
			fprintf (ftmp, "| %16.16s ",     glRec [i].creditAccount);
			fprintf (ftmp, "| %12.2f |\n", glRec [i].creditAmount);
			add_crd += glRec [i].creditAmount;
		}
	}
	fprintf (ftmp, "|=============================================");
	fprintf (ftmp, "|==================|==============");
	fprintf (ftmp, "|==================|==============|\n");

	fprintf (ftmp, "| BREAKDOWN GENERAL LEDGER TOTALS             ");
	fprintf (ftmp, "|                  |%13.2f ", 	 add_dbt);
	fprintf (ftmp, "|                  |%13.2f |\n", add_crd);
    fprintf (ftmp, ".EOF\n");
}

/*
 * Start Out Put To Standard Print.
 */
void
HeadingOutput (
 int                prnt_num)
{
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.gl_date), PNAME);
	fprintf (ftmp, ".LP%d\n",prnt_num);

	/*
	 * Print Selected Accounts Type.
	 */
	fprintf (ftmp, ".12\n");
	fprintf (ftmp, ".L158\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E%s\n",comm_rec.co_name);
	fprintf (ftmp, ".B1\n");

	fprintf (ftmp, ".E AS AT :%s\n",SystemTime ());
	fprintf (ftmp, ".B1\n");
	if (RECEIPT)
		fprintf (ftmp, ".ESTOCK RECEIPT GENERAL LEDGER AUDIT\n");
	else
		fprintf (ftmp, ".ESTOCK ACTUAL COSTING GENERAL LEDGER AUDIT\n");

	fprintf (ftmp, ".R==============================================");
	fprintf (ftmp, "==================================");
	fprintf (ftmp, "===================================\n");
 
	fprintf (ftmp, "==============================================");
	fprintf (ftmp, "==================================");
	fprintf (ftmp, "===================================\n");

	fprintf (ftmp, "| GOODS RECEIPT |PURCHASE ORDER |  CATEGORY.  ");
	fprintf (ftmp, "|       DEBIT      |    DEBIT     ");
	fprintf (ftmp, "|       CREDIT     |    CREDIT    |\n");

	fprintf (ftmp, "|    NUMBER     |    NUMBER     |             ");
	fprintf (ftmp, "|      ACCOUNT     |    AMOUNT    ");
	fprintf (ftmp, "|      ACCOUNT     |    AMOUNT    |\n");

	fprintf (ftmp, "|---------------|---------------|-------------");
	fprintf (ftmp, "|------------------|--------------");
	fprintf (ftmp, "|------------------|--------------|\n");

}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	if (processPID)
		open_rec (pogh, pogh_list, POGH_NO_FIELDS, "pogh_pid_id");
	else
	{
		open_rec (pogh, pogh_list, POGH_NO_FIELDS, (RECEIPT) ? "pogh_up_id"
						   									: "pogh_id_no2");
	}
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_id_no");
	open_rec (powk, powk_list, POWK_NO_FIELDS, "powk_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (posh, posh_list, POSH_NO_FIELDS, "posh_id_no");
	OpenGlmr ();
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (pogh);
	abc_fclose (pogl);
	abc_fclose (posh);
	abc_fclose (powk);
	abc_fclose (inmr);
	abc_fclose (pohr);
	GL_Close ();
	abc_dbclose (data);
}

void
ProcessPidFunc (void)
{
	strcpy (pogh_rec.co_no, comm_rec.co_no);
	pogh_rec.pid	=	processPID;
	cc = find_rec (pogh, &pogh_rec, GTEQ, "u");

	while (!cc && 
		   !strcmp (pogh_rec.co_no,comm_rec.co_no) &&
	       pogh_rec.pid	== processPID)
	{
		/*
		 * Goods Received
		 */
		if ((pogh_rec.pur_status [0] == purchaseStatus [0] &&
	         pogh_rec.gl_status [0] == purchaseStatus [0]) &&
		   ((dropShipFlag [0] == 'Y' && pogh_rec.drop_ship [0] == 'Y') ||
	        (dropShipFlag [0] != 'Y' && pogh_rec.drop_ship [0] != 'Y')))
		{
			pohr_rec.hhpo_hash	=	pogh_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			if (cc)
				strcpy (pohr_rec.stat_flag, "N");

			if (!processPID)
				dsp_process ("GR NO", pogh_rec.gr_no);

			ProcessPogl (pogh_rec.hhgr_hash);
		    if (RECEIPT)
		    	strcpy (pogh_rec.gl_status, (ACT_COST) ? " " : "D");
		   	else
		   		strcpy (pogh_rec.gl_status, "D");

			if (CRD_LINE)
		   		strcpy (pogh_rec.gl_status, "D");

		   	cc = abc_update (pogh, &pogh_rec);
		   	if (cc)
				file_err (cc, "pogh", "DBUPDATE");
		}
		else
			abc_unlock (pogh);

		cc = find_rec (pogh, &pogh_rec, NEXT, "u");
	}
	abc_unlock (pogh);
}

void
ProcessAll (void)
{
	strcpy (pogh_rec.co_no, comm_rec.co_no);
	strcpy (pogh_rec.br_no, comm_rec.est_no);
	sprintf (pogh_rec.gr_no, "%15.15s", " ");
	pogh_rec.hhsu_hash = 0L;
	cc = find_rec (pogh, &pogh_rec, GTEQ, "u");

	while (!cc && 
		   !strcmp (pogh_rec.co_no,comm_rec.co_no) &&
	     (!strcmp (pogh_rec.br_no,comm_rec.est_no) || !RECEIPT))
	{
		/*
		 * Goods Received
		 */
		if ((pogh_rec.pur_status [0] == purchaseStatus [0] &&
	         pogh_rec.gl_status [0] == purchaseStatus [0]) &&
		   ((dropShipFlag [0] == 'Y' && pogh_rec.drop_ship [0] == 'Y') ||
	        (dropShipFlag [0] != 'Y' && pogh_rec.drop_ship [0] != 'Y')))
		{
			pohr_rec.hhpo_hash	=	pogh_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			if (cc)
				pohr_rec.stat_flag [0] = 'N';

			if (!processPID)
				dsp_process ("GR NO", pogh_rec.gr_no);

			ProcessPogl (pogh_rec.hhgr_hash);
		    if (RECEIPT)
		    	strcpy (pogh_rec.gl_status, (ACT_COST) ? " " : "D");
		   	else
		   		strcpy (pogh_rec.gl_status, "D");

			if (CRD_LINE)
		   		strcpy (pogh_rec.gl_status, "D");

		   	cc = abc_update (pogh, &pogh_rec);
		   	if (cc)
				file_err (cc, "pogh", "DBUPDATE");
		}
		else
			abc_unlock (pogh);

		cc = find_rec (pogh, &pogh_rec, NEXT, "u");
	}
	abc_unlock (pogh);
}


void
ProcessPogl (
	long	hhglHash)
{
	int		printed = FALSE;

	pogl_rec.hhgr_hash 	= hhglHash;
	pogl_rec.line_no 	= 0;
	
	groupTotal = 0.00;

	cc = find_rec (pogl, &pogl_rec, GTEQ, "u");
	while (!cc && pogl_rec.hhgr_hash == hhglHash)
	{
	   	if (pogl_rec.pur_status [0] != purchaseStatus [0] ||
	        pogl_rec.gl_status [0]  != purchaseStatus [0])
		{
		   	abc_unlock (pogl);
			cc = find_rec (pogl, &pogl_rec, NEXT, "u");
			continue;
		}

		/*
		 * Do not perform any gl postings for Direct-Delivery P/O s 
		 */
		if (pohr_rec.drop_ship [0] != 'Y')
		{
			printed = TRUE;
			CreateDetail (pogl_rec.hhcc_hash);
		}
		
		if (RECEIPT)
			strcpy (pogl_rec.gl_status, (ACT_COST) ? " " : "D");
		else
		   	strcpy (pogl_rec.gl_status, "D");

		/*
		 * Must be a credit so actual costing does not apply.
		 */
		if (CRD_LINE)
			strcpy (pogl_rec.gl_status, "D");

		cc = abc_update (pogl, &pogl_rec);
		if (cc)
			file_err (cc, "pogl", "DBUPDATE");

		cc = find_rec (pogl, &pogl_rec, NEXT, "u");
	}
	abc_unlock (pogl);
	if (!printed)
		return;

	fprintf (ftmp, "|               |               |             ");
	fprintf (ftmp, "|                  |              ");
	fprintf (ftmp, "|                  |              |\n");

	fprintf (ftmp, "|%15.15s", 		pogh_rec.gr_no);
	fprintf (ftmp, "|%15.15s", 		" ");
	fprintf (ftmp, "| %11.11s ", 	" GR TOTALS ");
	fprintf (ftmp, "| %16.16s ", 		"  DEBIT  ");
	fprintf (ftmp, "| %12.2f ", 	groupTotal);
	fprintf (ftmp, "| %16.16s ", 		"  CREDIT ");
	fprintf (ftmp, "| %12.2f |\n",	groupTotal);

	fprintf (ftmp, "|---------------|---------------|-------------");
	fprintf (ftmp, "|------------------|--------------");
	fprintf (ftmp, "|------------------|--------------|\n");
}


/*
 * Create Details of estimated / Actual cost update. |
 */
int
CreateDetail (
	long	hhccHash)
{

/*
 *            ==========================================     
 *            Details of estimated / Actual cost update.    
 *            ==========================================   
 *                                                        
 * Details on proceedure as follows :                           
 *                                                             
 *  ESTIMATED COSTING: (landed cost)                          
 *  ------------------                                            
 *    Purchase Order Receipt Time.                                 
 *    ----------------------------                               
 *    Debit  Stock purchases account from excg for category.        
 *    Credit Stock purchases account from excg for category.        
 *    NOTE that no other postings required.                          
 *                                                                   
 *  ACTUAL COSTING:                                                  
 *  ---------------                                                
 *    Purchase Order Receipt Time. (Land Cost)                     
 *    ----------------------------                                   
 *    Debit  -> Debit Stock purchases account from excg for category.
 *    Credit -> Stock clearing account from glat at branch.    
 *                                                              
 *    Purchase Order Costing Time.                               
 *    ----------------------------                                
 *   (At Landed Cost)                                              
 *    Debit  -> Stock clearing account from glat at branch.         
 *    Credit -> Debit Stock purchases account from excg for category.
 *                                                                  
 *   (At Actual Cost)                                                
 *    Debit  -> Debit  Stock purchases account from excg for category.
 *    Credit -> Credit Stock purchases account from excg for category. 
 *                                                               
 *  IF (PO_ACT_VAR)                                                
 *   (At Diff between Estimate and Actual using Qty Left.)        
 *    Debit  -> COS Variance Account from glat at Branch.           
 *    Credit -> Debit Stock purchases account from excg for category.
 *  ELSE                                                             
 *    Debit  -> Debit  Stock purchases account from excg for category. 
 *    Credit -> Credit Stock purchases account from excg for category.
 *                                                                     
 */

	char	debitAccount [MAXLEVEL + 1],
			creditAccount [MAXLEVEL + 1];

	long	debitHash 	= 0L,
			creditHash 	= 0L;

	float	qty_left = 0.00;
	double	value = 0.00,
			old_val = 0.00,
			new_val = 0.00;

	inmr_rec.hhbr_hash	=	pogl_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		inmr_rec.outer_size = 1.00;

	ccmr_rec.hhcc_hash = hhccHash;
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	value	=	twodec (pogl_rec.land_cst);
	value 	= 	out_cost (value, inmr_rec.outer_size);

	if (CRD_LINE)
		linesTotal =  value * (pogl_rec.qty_rec * -1);
	else
		linesTotal =  value * pogl_rec.qty_rec;
	linesTotal =  twodec (linesTotal);
	groupTotal += linesTotal;
	grandTotal += linesTotal;

	/*
	 * Note that costing is the reverse of a receipt.
	 */
	if (RECEIPT)
	{
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"PURCHASE D",
			" ",
			pogl_rec.cat_code
		);
		strcpy (debitAccount, glmrRec.acc_no);
        debitHash = glmrRec.hhmr_hash;

		if (pohr_rec.stat_flag [0] == 'Q')
		{
			GL_GLI 
			(
				ccmr_rec.co_no,
				ccmr_rec.est_no,
				ccmr_rec.cc_no,
				"PO CLEAR Q",
				" ",
				pogl_rec.cat_code
			);
			strcpy (creditAccount, glmrRec.acc_no);
        	creditHash = glmrRec.hhmr_hash;
		}
		else
		{
			if (ACT_COST)
			{
				GL_GLI 
				(
					ccmr_rec.co_no,
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					"PO CLEAR N",
					" ",
					pogl_rec.cat_code
				);
				strcpy (creditAccount, glmrRec.acc_no);
				creditHash = glmrRec.hhmr_hash;
			}
			else
			{
				GL_GLI 
				(
					ccmr_rec.co_no,
					ccmr_rec.est_no,
					ccmr_rec.cc_no,
					"PURCHASE C",
					" ",
					pogl_rec.cat_code
				);
				strcpy (creditAccount, glmrRec.acc_no);
				creditHash = glmrRec.hhmr_hash;
			}
		}
		if (CRD_LINE)
		{
			PrintPost 
			 (	
				pogh_rec.gr_no, 
				pogl_rec.po_number, 
		   	    pogl_rec.cat_code, 
	 	   	    creditAccount, 
				creditHash, 
				debitAccount, 
				debitHash,
				linesTotal
			);
		}
		else
		{
			PrintPost 
			 (	
				pogh_rec.gr_no, 
				pogl_rec.po_number, 
		   	    pogl_rec.cat_code, 
				debitAccount, 
				debitHash,
	 	   	    creditAccount, 
				creditHash, 
				linesTotal
			);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * The rest of the processing is for Actual cost update only.
	 * Reverse out Original posting from estimated costing. 
	 */
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"PO CLEAR N",
		" ",
		pogl_rec.cat_code
	);

	strcpy (debitAccount, glmrRec.acc_no);
	debitHash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"PURCHASE D",
		" ",
		pogl_rec.cat_code
	);
	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;
	
	PrintPost 
	(	
		pogh_rec.gr_no,
		pogl_rec.po_number, 
		pogl_rec.cat_code, 
		debitAccount, 
		debitHash, 
		creditAccount, 
		creditHash, 
		linesTotal
	);

	/*
	 * Create new Actual Costing Update.
	 */
	value = out_cost (pogl_rec.act_cst, inmr_rec.outer_size);

	linesTotal =  value * pogl_rec.qty_rec;
	linesTotal =  twodec (linesTotal);
	groupTotal += linesTotal;
	grandTotal += linesTotal;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"PURCHASE D",
		" ",
		pogl_rec.cat_code
	);
	strcpy (debitAccount, glmrRec.acc_no);
	debitHash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"PURCHASE C",
		" ",
		pogl_rec.cat_code
	);

	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;
	
	PrintPost 
	(	
		pogh_rec.gr_no, 
		pogl_rec.po_number, 
		pogl_rec.cat_code, 
		debitAccount, 
		debitHash, 
		creditAccount, 
		creditHash, 
		linesTotal
	);

	/*
	 * Create new Estimated/Actual Cost Variance.
	 */
	old_val = out_cost (pogl_rec.land_cst, inmr_rec.outer_size);
	new_val = out_cost (pogl_rec.act_cst,  inmr_rec.outer_size);
	value	= new_val - old_val;
	
	qty_left = pogl_rec.qty_rec - pogl_rec.qty_left;
	if (qty_left <= 0.00)
		return (EXIT_SUCCESS);

	linesTotal =  value * qty_left;
	linesTotal =  twodec (linesTotal);
	groupTotal += linesTotal;
	grandTotal += linesTotal;

	if (envVarPoActVar)
	{
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"PO ACOST V",
			" ",
			pogl_rec.cat_code
		);
		strcpy (debitAccount, glmrRec.acc_no);
		debitHash = glmrRec.hhmr_hash;

		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"PURCHASE D",
			" ",
			pogl_rec.cat_code
		);
		strcpy (creditAccount, glmrRec.acc_no);
        creditHash = glmrRec.hhmr_hash;
	}
	else
	{
		GL_GLI 
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"COSTSALE D",
			" ",
			pogl_rec.cat_code
		);
		strcpy (debitAccount, glmrRec.acc_no);
		debitHash = glmrRec.hhmr_hash;

		GL_GLI   
		(
			ccmr_rec.co_no,
			ccmr_rec.est_no,
			ccmr_rec.cc_no,
			"PURCHASE D",
			" ",
			pogl_rec.cat_code
		);
		strcpy (creditAccount, glmrRec.acc_no);
		creditHash = glmrRec.hhmr_hash;
	}
	PrintPost 
	(
		pogh_rec.gr_no, 
		pogl_rec.po_number, 
		pogl_rec.cat_code, 
		debitAccount, 
		debitHash, 
		creditAccount, 
		creditHash, 
		linesTotal
	);
    return (EXIT_SUCCESS);
}

/*
 * Print line and post details to general ledger work file.
 */
void
PrintPost (
	char	*grinNumber, 
	char	*PoNumber, 
	char	*categoryCode, 
	char	*debitAccount,
	long	debitHash,
	char	*creditAccount,
	long	creditHash,
	double	total)
{
	fprintf (ftmp, "|%15.15s",	 grinNumber);
	fprintf (ftmp, "|%15.15s",	 PoNumber);
	fprintf (ftmp, "| %11.11s ", categoryCode);

	if (total < 0.00)
	{
		fprintf (ftmp, "| %-16.16s ", 	creditAccount);
		fprintf (ftmp, "| %12.2f ", 	total * -1);
		fprintf (ftmp, "| %-16.16s ", 	debitAccount);
		fprintf (ftmp, "| %12.2f |\n", 	total * -1);
	}
	else
	{
		fprintf (ftmp, "| %-16.16s ", 	debitAccount);
		fprintf (ftmp, "| %12.2f ", 	total);
		fprintf (ftmp, "| %-16.16s ", 	creditAccount);
		fprintf (ftmp, "| %12.2f |\n", 	total);
	}

	/*
	 * Add general Ledger Transactions to Purchase order work file.
	 */
	AddGlWorkFile 
	 (	
		grinNumber,
		PoNumber,
		debitAccount,
		debitHash,
		creditAccount,
		creditHash,
		total
	);

	/*
	 * Sumarise General Ledger Accounts.
	 */
	GlStoreInto (debitAccount, creditAccount, total);
}

/*
 * Add general Ledger Transactions to Purchase order worrk file.
 */
void
AddGlWorkFile (
	char	*grinNumber,
	char	*porderNumber,
	char	*debitPurchaseAcc,
	long	debitHash,
	char	*creditPurchaseAcc,
	long	creditHash,
	double	amt)
{
	/*
	 * If receipt is by Supplier and not shipment.
	 */
	if (!find_hash (sumr, &sumr_rec, EQUAL, "r", pogh_rec.hhsu_hash))
		strcpy (powk_rec.user_ref, sumr_rec.crd_no);
	else
		strcpy (powk_rec.user_ref, " ");

	strcpy (powk_rec.co_no, comm_rec.co_no);
	strcpy (powk_rec.br_no, comm_rec.est_no);
	strcpy (powk_rec.sort,  porderNumber);
	sprintf (powk_rec.dbt_acc, "%-*.*s", MAXLEVEL,MAXLEVEL, debitPurchaseAcc);
	sprintf (powk_rec.crd_acc, "%-*.*s", MAXLEVEL,MAXLEVEL, creditPurchaseAcc);
	powk_rec.dbt_hash 	= debitHash;
	powk_rec.crd_hash 	= creditHash;
	powk_rec.amount 	= amt;

	if (RECEIPT)
	{
		if (pogh_rec.rec_by [0] == 'P')
		{
			sprintf (powk_rec.narrative, "PO # %-15.15s", porderNumber);
			sprintf (powk_rec.alt_desc1, "GR # %-15.15s", grinNumber);
		}

		if (pogh_rec.rec_by [0] == 'G')
		{
			sprintf (powk_rec.narrative, "GR # %-15.15s", grinNumber);
			sprintf (powk_rec.alt_desc1, "PO # %-15.15s", porderNumber);
		}

		if (pogh_rec.rec_by [0] == 'S')
		{
			strcpy (posh_rec.co_no, comm_rec.co_no);
			posh_rec.hhsh_hash	=	pogh_rec.hhsh_hash;
			cc = find_rec (posh, &posh_rec, COMPARISON, "r");
			if (cc)
			{
				if (pogh_rec.rec_by [0] == 'G')
					sprintf (powk_rec.narrative, "GR # %-15.15s", grinNumber);
			}
			else
				sprintf (powk_rec.narrative, "SH # %-15.15s", posh_rec.csm_no);

			sprintf (powk_rec.alt_desc1, "PO # %-15.15s", porderNumber);
			sprintf (powk_rec.alt_desc2, "GR # %-15.15s", grinNumber);
		}
	}
	else
	{
		if (pogh_rec.cst_by [0] == 'P')
		{
			sprintf (powk_rec.narrative, "PO # %-15.15s", porderNumber);
			sprintf (powk_rec.alt_desc1, "GR # %-15.15s", grinNumber);
		}

		if (pogh_rec.cst_by [0] == 'G')
		{
			sprintf (powk_rec.narrative, "GR # %-15.15s", grinNumber);
			sprintf (powk_rec.alt_desc1, "PO # %-15.15s", porderNumber);
		}

		if (pogh_rec.cst_by [0] == 'S')
		{
			strcpy (posh_rec.co_no, comm_rec.co_no);
			posh_rec.hhsh_hash	=	pogh_rec.hhsh_hash;
			cc = find_rec (posh, &posh_rec, COMPARISON, "r");
			if (cc)
			{
				if (pogh_rec.rec_by [0] == 'G')
					sprintf (powk_rec.narrative, "GR # %-15.15s", grinNumber);
			}
			else
				sprintf (powk_rec.narrative, "SH # %-15.15s", posh_rec.csm_no);

			sprintf (powk_rec.alt_desc1, "PO # %-15.15s", porderNumber);
			sprintf (powk_rec.alt_desc2, "GR # %-15.15s", grinNumber);
		}
	}
	if (RECEIPT && pohr_rec.stat_flag [0] == 'Q')
	{
		sumr_rec.hhsu_hash	=	pogh_rec.hhsu_hash;
		if (find_rec (sumr, &sumr_rec, EQUAL, "r"))
			strcpy (sumr_rec.crd_no, "      ");

		sprintf (powk_rec.user_ref, "%-6.6s",   sumr_rec.crd_no);
		sprintf (powk_rec.narrative,"%-15.15s", pohr_rec.pur_ord_no);
		sprintf (powk_rec.alt_desc1, "GR # %-15.15s", grinNumber);
	}
	strcpy (powk_rec.stat_flag, "N");
	cc = abc_add (powk, &powk_rec);
	if (cc)
		file_err (cc, powk, "DBADD");
}


/*
 * Store General Ledger Account Values.  
 */
void
GlStoreInto (
	char	*debitAccount,
	char	*creditAccount,
	double	debitCreditAmount)
{
	int		found = 0,
			dbt_pos = -1,
			crd_pos = -1,
			i;

	for (i = 0; i < MAX_ACCS && !found; i++) 
	{
		if (!strncmp (glRec [i].debitAccount, "----------------",MAXLEVEL) && 
		    !strncmp (glRec [i].creditAccount, "----------------",MAXLEVEL))
			break;

		if (!strncmp (debitAccount, glRec [i].debitAccount, MAXLEVEL))
		{
			glRec [i].debitAmount += debitCreditAmount;
			dbt_pos = 1;
		}

		if (!strncmp (creditAccount, glRec [i].creditAccount, MAXLEVEL))
		{
			glRec [i].creditAmount += debitCreditAmount;
			crd_pos = 1;
		}
	}

	if (dbt_pos == -1)
	{
		sprintf (glRec [i].debitAccount, "%-*.*s", MAXLEVEL, MAXLEVEL, debitAccount);
		glRec [i].debitAmount = debitCreditAmount;
	}

	if (crd_pos == -1)
	{
		sprintf (glRec [i].creditAccount, "%-*.*s", MAXLEVEL, MAXLEVEL, creditAccount);
		glRec [i].creditAmount = debitCreditAmount;
	}
}
