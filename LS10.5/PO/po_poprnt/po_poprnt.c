/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_poprnt.c,v 5.6 2001/10/22 04:28:23 scott Exp $
|  Program Name  : (po_poprnt.c)                                      |
|  Program Desc  : (Print Purchase Order by Supplier.    	 )        |	
|                  (                                             )    |
|                  (                                             )    |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap.   | Date Written  : 31/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (07/03/91)      | Modified  by  : Scott Darrow.    |
|  Date Modified : (13/09/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (07/07/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (30/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (11/04/94)      | Modified  by  : Roel Michels     |
|  Date Modified : (17/10/94)      | Modified  by  : Aroha Merrilees. |
|  Date Modified : (02/10/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (22/11/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (09/10/96)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (12/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (07/03/91) - General Update.                       |
|  (13/09/91)    : Mods for UOM.                                      |
|  (07/07/92)    : To include inex desc lines SC DFH 7287.            |
|  (30/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (11/04/94)    : PSL 10673 - Online conversion                      |
|  (17/10/94)    : PSL-V9 11474 - change conversion factor calcul.    |
|  (17/10/94)    : PSL-V9 11474 - change conversion factor calcul.    |
|  (02/10/95)    : FRA 00001. Updated to add non stock items.         |
|  (22/11/95)    : FRA 00001. Updated to allow printing of comment    |
|                :            lines for all products.                 |
|  (09/10/96)    : SEL - Updated to fix rounding of float.            |
|  (12/09/97)    : Incorporated multilingual conversion adnd DMY4 date|
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
| $Log: po_poprnt.c,v $
| Revision 5.6  2001/10/22 04:28:23  scott
| Updated for syntax
|
| Revision 5.5  2001/10/22 04:18:39  scott
| Updated from testing
|
| Revision 5.4  2001/10/19 03:01:23  cha
| Corrected some small compile errors.
|
| Revision 5.3  2001/10/19 02:59:34  cha
| Fix Issue # 00627 by Scott.
|
| Revision 5.2  2001/08/09 09:15:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:10  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:53  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/15 03:36:26  scott
| Updated to add sleep (2) for warning messages - LS10-GUI.
|
| Revision 4.0  2001/03/09 02:33:05  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:37  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/07 02:31:13  scott
| Updated to add new suppier search as per stock and customer searches.
|
| Revision 2.0  2000/07/15 09:05:28  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.21  1999/12/06 01:32:41  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.20  1999/11/11 06:43:17  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.19  1999/11/05 05:17:15  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.18  1999/10/14 03:04:24  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.17  1999/09/29 10:12:06  scott
| Updated to be consistant on function names.
|
| Revision 1.16  1999/09/21 04:38:07  scott
| Updated from Ansi project
|
| Revision 1.15  1999/08/23 10:59:12  gerry
| SC 1873 - Addition of new env var envVarPoPrintGross to control printing of gross or net amounts in the PO report
|
| Revision 1.14  1999/06/17 10:06:33  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_poprnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_poprnt/po_poprnt.c,v 5.6 2001/10/22 04:28:23 scott Exp $";

#include <pslscr.h>
#include <ml_po_mess.h>
#include <ml_std_mess.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>

#define	COSTING		 (costing [0] == 'C')
#define	PORDER		 (costing [0] == 'P')
#define	CREDIT		 (costing [0] == 'R')
	
#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct inisRecord	inis_rec;
struct inmrRecord	inmr_rec;
struct ponsRecord	pons_rec;
struct inumRecord	inum_rec;
struct pogdRecord	pogd_rec;
struct pohrRecord	pohr_rec;
struct inexRecord	inex_rec;
struct polnRecord	poln_rec;
struct sumrRecord	sumr_rec;

	int		line_no = 0;
	int		printerNumber = 1;
	int		envVarCrCo = 0;
	int		envVarCrFind = 0;
	int		envVarPoPrintGross;
	int		automaticPrint;
	int		inisFound;

	double	printTotal [6];

	char	branchNumber [3];
	char	costing [2];
	char	uom_desc [5];

	long	hhpoHash;

	FILE	*fout;

	float	StdCnvFct 	= 1.00;
	float	PurCnvFct 	= 1.00;
	float	CnvFct		= 1.00;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	sup_no [7];
	char	sup_name [41];
	char	purchaseOrderNo [16];
	int		no_copies;
	char	po_prmpt [17];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "sup_no",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier No   ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.sup_no},
	{1, LIN, "name",	 5, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Supplier Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", sumr_rec.crd_name},
	{1, LIN, "purchaseOrderNo",	 6, 18, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", local_rec.po_prmpt, " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.purchaseOrderNo},
	{1, LIN, "no_copies",	 7, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "# Copies", " ",
		YES, NO, JUSTRIGHT, "1", "99", (char *)&local_rec.no_copies},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
int		CheckPogd		(void);
int		FindInis		(long);
int		heading			(int);
int		spec_valid		(int);
void	CloseDB			(void);
void	HeadingOutput	(void);
void	OpenDB			(void);
void	PrintCHeading	(void);
void	PrintCpoln		(void);
void	PrintGrSum		(void);
void	PrintHeading	(void);
void	PrintInex		(void);
void	PrintPoSum		(void);
void	PrintPoln		(void);
void	ProcessFile		(void);
void	ReadMisc		(void);
void	RunReport		(void);
void	SrchPohr		(char	*);
void	shutdown_prog	(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	strcpy (costing, "P");

	if (!strcmp (sptr, "po_cstprn"))
		strcpy (costing, "C");

	if (!strcmp (sptr, "po_cnprnt"))
		strcpy (costing, "R");


	if (argc < 2)
	{
		print_at (0,0,mlPoMess732,argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	automaticPrint = FALSE;
	if (argc == 3)
	{
		hhpoHash = atol (argv [2]);
		automaticPrint = TRUE;
	}
	envVarCrCo 	 = atoi (get_env ("CR_CO"));
	envVarCrFind = atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	sptr = chk_env ("PO_PRINT_GROSS");
	envVarPoPrintGross = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (!automaticPrint)
	{
		SETUP_SCR (vars);

		if (CREDIT)
			strcpy (local_rec.po_prmpt, "Purchase Crd No");
		else
			strcpy (local_rec.po_prmpt, "Purchase Ord No");
		/*---------------------------
		| Setup required parameters |
		---------------------------*/
		init_scr ();
		set_tty ();
		set_masks ();
		init_vars (1);


		while (prog_exit == 0)
		{
			/*---------------------
			| Reset control flags |
			---------------------*/
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			init_vars (1);
	
			/*----------------------------
			| Entry screen 1 linear input |
			----------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;
	
			/*----------------------------
			| Edit screen 1 linear input |
			----------------------------*/
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;
	
			RunReport ();
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate hhpo hash |
	--------------------*/
	if (!find_hash (pohr, &pohr_rec, COMPARISON, "r", hhpoHash))
	{
		if (!find_hash (sumr, &sumr_rec, COMPARISON, "r", pohr_rec.hhsu_hash))
			RunReport ();
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
	abc_dbopen ("data");

	ReadMisc ();

	if (automaticPrint)
		open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	else
		open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no");
	if (automaticPrint)
	{
		open_rec (sumr,sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash");
	}
	else
	{
		open_rec (sumr,sumr_list,SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
														   : "sumr_id_no3");
	}
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pogd, pogd_list, POGD_NO_FIELDS, "pogd_id_no3");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
	open_rec (pons, pons_list, PONS_NO_FIELDS, "pons_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (inex);
	abc_fclose (inum);
	abc_fclose (pogd);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (sumr);
	abc_fclose (pons);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);
}

int
spec_valid (
 int field)
{
	/*---------------------------
	| Validate Supplier Number. |
	---------------------------*/
	if (LCHECK ("sup_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sumr_rec.co_no,comm_rec.co_no);
		strcpy (sumr_rec.est_no,branchNumber);
		strcpy (sumr_rec.crd_no,pad_num (local_rec.sup_no));
		cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("name");
		return (EXIT_SUCCESS);
	}

	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK ("purchaseOrderNo"))
	{
		if (SRCH_KEY)
		{
			SrchPohr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pohr_rec.co_no,comm_rec.co_no);
		strcpy (pohr_rec.br_no,comm_rec.est_no);
		strcpy (pohr_rec.type, (CREDIT) ? "C" : "O");
		pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (pohr_rec.pur_ord_no,zero_pad (local_rec.purchaseOrderNo, 15));
		cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlStdMess122));
			return (EXIT_FAILURE);
		}
		if (pohr_rec.status [0] == 'D')
		{
			errmess (ML (mlPoMess001));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
RunReport (
 void)
{
	if (COSTING)
		dsp_screen ("Processing : Printing Single Purchase Order Costing.",comm_rec.co_no,comm_rec.co_name);
	else
		dsp_screen ("Processing : Printing Single Purchase Order.",comm_rec.co_no,comm_rec.co_name);

	HeadingOutput ();
	ProcessFile ();
	if (COSTING && CheckPogd ())
		PrintGrSum ();

	if (PORDER || CREDIT)
		PrintPoSum ();

	fprintf (fout,".EOF\n");
	pclose (fout);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	int	i;

	for (i = 0; i < 6; i++)
		printTotal [i] = 0.00;

	line_no = 0;

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBPOPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".NC%d\n", (automaticPrint) ? 1 : local_rec.no_copies);
	fprintf (fout,".LP%d\n",printerNumber);

	fprintf (fout,".23\n");
	fprintf (fout,".PI16\n");
	fprintf (fout,".L170\n");

	if (COSTING)
		fprintf (fout,".E%s\n","PURCHASE ORDER COSTING PRINT.");
	if (PORDER)
		fprintf (fout,".E%s\n", "PURCHASE ORDER PRINT.");
	if (CREDIT)
		fprintf (fout,".E%s\n", "PURCHASE RETURN PRINT.");
				          
	if (pohr_rec.status [0] == 'U')
		fprintf (fout,".E*** NOTE : THIS P/O IS NOT VALID AS IT HAS NOT BEEN APPROVED. ***\n");
	else
		fprintf (fout,".B1\n");
	fprintf (fout,".E: %-40.40s\n",comr_rec.co_name);
	fprintf (fout,".E: %-40.40s\n",comr_rec.co_adr1);
	fprintf (fout,".E: %-40.40s\n",comr_rec.co_adr2);
	fprintf (fout,".E: %-40.40s\n",comr_rec.co_adr3);
	fprintf (fout,".B1\n");

	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout,".B1\n");

	if (COSTING)
	{
		fprintf (fout,"======");
		fprintf (fout,"=================");
		fprintf (fout,"=====================================");
		fprintf (fout,"========");
		fprintf (fout,"========");
		fprintf (fout,"=============");
		fprintf (fout,"=============");
		fprintf (fout,"=============");
		fprintf (fout,"=============");
		fprintf (fout,"=============");
		fprintf (fout,"=============");
		fprintf (fout,"==========\n");
	}
	else
	{
		fprintf (fout,"=============");
		fprintf (fout,"===================");
		fprintf (fout,"===================");
		fprintf (fout,"===========================================");	
		fprintf (fout,"=============");
		fprintf (fout,"===============");
		fprintf (fout,"===============");
		fprintf (fout,"============\n");
	}

	if (CREDIT)
		fprintf (fout,"Contact Name     : %-40.40s   Purchase Return No : %-15.15s    ",pohr_rec.contact,pohr_rec.pur_ord_no);
	else
		fprintf (fout,"Contact Name     : %-40.40s   Purchase Order No : %-15.15s    ",pohr_rec.contact,pohr_rec.pur_ord_no);

	fprintf (fout,"Delivery Instructions\n");

	fprintf (fout,"Supplier Name    : %-40.40s   Date Raised       : %s\n",
			sumr_rec.crd_name,DateToString (pohr_rec.date_raised));

	fprintf (fout,"Supplier Address : %-40.40s   Date Required     : %s   ",
			sumr_rec.adr1,DateToString (pohr_rec.due_date));

	fprintf (fout,"%-s\n",clip (pohr_rec.delin1));
	fprintf (fout,"                 : %-40.40s   Currency          : %-8.8s   ",
		sumr_rec.adr2, pohr_rec.curr_code);

	fprintf (fout,"%-s\n",clip (pohr_rec.delin2));
	fprintf (fout,"                 : %-40.40s   %31.31s",sumr_rec.adr3," ");
	fprintf (fout,"%-s\n",clip (pohr_rec.delin3));
	fprintf (fout,"Supplier Terms   : %-30.30s\n",pohr_rec.sup_term_pay);

	fprintf (fout,".B1\n");
	
	if (COSTING)
		PrintCHeading ();
	else
		PrintHeading ();
}

/*====================================
| Process Purchase order line items. |
====================================*/
void
ProcessFile (
 void)
{
	poln_rec.hhpo_hash = pohr_rec.hhpo_hash;
	poln_rec.line_no = 0;

	cc = find_rec (poln,&poln_rec,GTEQ,"r");
	while (!cc && poln_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		cc = find_hash (inmr,&inmr_rec,COMPARISON,"r",poln_rec.hhbr_hash);
		if (PORDER || CREDIT)
		{
			inisFound = FindInis (poln_rec.hhbr_hash);
			PrintPoln ();
		}
		else
			PrintCpoln ();

		cc = find_rec (poln,&poln_rec,NEXT,"r");
	}
}

/*=================================
| Find inventory Supplier Record. |
=================================*/
int
FindInis (
 long	hhbrHash)
{
	inis_rec.hhbr_hash = hhbrHash;
	inis_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	cc = find_rec (inis,&inis_rec,COMPARISON,"r");
	if (cc)
	{
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"r");
	}
	if (cc)
	{
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis,&inis_rec,COMPARISON,"r");
	}
	if (cc)
	{
		sprintf (inis_rec.sup_part,"%-16.16s"," ");
		return (FALSE);
	}
	return (TRUE);
}

/*==================================
| Print Purchase Order Line items. |
==================================*/
void
PrintPoln (
 void)
{
	double	extend = 0.00;
	int		FirstPons = TRUE;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	if (CREDIT && poln_rec.pur_status [0] != 'O')
		return;

	if (!CREDIT && (poln_rec.qty_ord - poln_rec.qty_rec <= 0.00))
			return;

	if (envVarPoPrintGross)
	{
		poln_rec.grs_fgn_cst = twodec (poln_rec.grs_fgn_cst);
		poln_rec.grs_fgn_cst = 	out_cost 
								(
									poln_rec.grs_fgn_cst, 
									inmr_rec.outer_size
								);
	}
	else
	{
		poln_rec.fob_fgn_cst = twodec (poln_rec.fob_fgn_cst);
		poln_rec.fob_fgn_cst = 	out_cost 
								(
									poln_rec.fob_fgn_cst,
									inmr_rec.outer_size
								);
	}

	if (CREDIT)
		extend = (double) poln_rec.qty_rec;
	else
		extend = (double) poln_rec.qty_ord - poln_rec.qty_rec;

	extend = twodec (extend);
	if (envVarPoPrintGross)
		extend *= poln_rec.grs_fgn_cst;
	else
		extend *= poln_rec.fob_fgn_cst;
	extend = twodec (extend);

	FirstPons = TRUE;
	if (!check_class (inmr_rec.inmr_class))
	{
		fprintf (fout, "| %3d "     , ++line_no);
		fprintf (fout, "| %16.16s " , inis_rec.sup_part);
		fprintf (fout, "| %16.16s " , inmr_rec.item_no);
		fprintf (fout, "| %40.40s " , poln_rec.item_desc);
		if (CREDIT)
			fprintf (fout,"| %10.2f ", (double) (poln_rec.qty_rec) * CnvFct);
		else
		{
			fprintf (fout, "| %10.2f "  , 
			 (double) (poln_rec.qty_ord - poln_rec.qty_rec) * CnvFct);
		}

		fprintf (fout, "| %-4.4s "  , inum_rec.uom);
		if (envVarPoPrintGross)
			fprintf (fout, "| %12.2f "  , poln_rec.grs_fgn_cst / CnvFct);
		else
			fprintf (fout, "| %12.2f "  , poln_rec.fob_fgn_cst / CnvFct);
		fprintf (fout, "|%13.2f "   , extend);
		fprintf (fout, "| %10.10s |\n", DateToString (poln_rec.due_date));
		FirstPons = FALSE;
	}
	pons_rec.hhpl_hash = poln_rec.hhpl_hash;
	pons_rec.line_no = 0;
	cc = find_rec (pons, &pons_rec, GTEQ, "r");
	while (!cc && pons_rec.hhpl_hash == poln_rec.hhpl_hash)
	{
		FirstPons = FALSE;
		fprintf (fout, "| %3d "     , ++line_no);
		fprintf (fout, "|                  ");
		fprintf (fout, "|                  ");
		fprintf (fout, "| %40.40s " , pons_rec.desc);
		if (FirstPons)
		{
			fprintf (fout, "| %10.2f "  , 
					 (poln_rec.qty_ord - poln_rec.qty_rec) * CnvFct);

			fprintf (fout, "| %-4.4s "  , inum_rec.uom);
			if (envVarPoPrintGross)
				fprintf (fout, "| %12.2f "  , poln_rec.grs_fgn_cst / CnvFct);
			else
				fprintf (fout, "| %12.2f "  , poln_rec.fob_fgn_cst / CnvFct);
			fprintf (fout, "|%13.2f "   , extend);
			fprintf (fout, "| %10.10s |\n", DateToString (poln_rec.due_date));
		}
		else
		{
			fprintf (fout, "|            ");
			fprintf (fout, "|      ");
			fprintf (fout, "|              ");
			fprintf (fout, "|              ");
			fprintf (fout, "|          |\n");
		}
		cc = find_rec (pons, &pons_rec, NEXT, "r");
	}
	PrintInex ();
	printTotal [0] += extend;
}

/*==================================
| Print Purchase Order Line items. |
==================================*/
void
PrintCpoln (
 void)
{
	double	wk_tot = 0.00;
	float	quantity = 0.00;
	int		FirstPons;

	quantity = poln_rec.qty_ord - poln_rec.qty_rec;

	if (quantity <= 0.00)
		return;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	poln_rec.hhum_hash;
	cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
	PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	CnvFct	=	StdCnvFct / PurCnvFct;

	if (check_class (inmr_rec.inmr_class))
	{
		FirstPons = TRUE;
		pons_rec.hhpl_hash = poln_rec.hhpl_hash;
		pons_rec.line_no = 0;
		cc = find_rec (pons, &pons_rec, GTEQ, "r");
		while (!cc && pons_rec.hhpl_hash == poln_rec.hhpl_hash)
		{
			fprintf (fout,"|%3d",++line_no);
			fprintf (fout,"|                ");
			fprintf (fout,"|%30.30s", pons_rec.desc);
			if (FirstPons)
			{
				fprintf (fout,"| %4.4s ",inum_rec.uom);
				fprintf (fout,"|%6.0f",quantity * CnvFct);
				wk_tot = twodec (poln_rec.fob_fgn_cst);
				wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [0] += wk_tot;

				fprintf (fout,"|%11.2f ",  poln_rec.fob_fgn_cst/CnvFct);
				fprintf (fout,"|%9.4f",    poln_rec.exch_rate);
				fprintf (fout,"|%11.2f ",  poln_rec.frt_ins_cst/CnvFct);
				wk_tot = twodec (poln_rec.frt_ins_cst);
				wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [1] += wk_tot;

				fprintf (fout,"|%11.2f ",  poln_rec.duty/ CnvFct);
				wk_tot = out_cost (poln_rec.duty, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [2] += wk_tot;

				fprintf (fout,"|%11.2f ",  poln_rec.licence/ CnvFct);
				wk_tot = out_cost (poln_rec.licence, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [3] += wk_tot;

				fprintf (fout,"|%11.2f ",  poln_rec.lcost_load/ CnvFct);
				wk_tot = out_cost (poln_rec.lcost_load, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [4] += wk_tot;

				fprintf (fout,"|%11.2f ",  poln_rec.land_cst/ CnvFct);
				wk_tot = twodec (poln_rec.land_cst);
				wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
				wk_tot *= quantity;
				wk_tot = twodec (wk_tot);
				printTotal [5] += wk_tot;

				fprintf (fout,"|%10.10s|\n",  DateToString (poln_rec.due_date));
			}
			else
			{
				fprintf (fout,"|      ");
				fprintf (fout,"|      ");
				fprintf (fout,"|            ");
				fprintf (fout,"|         ");
				fprintf (fout,"|            ");
				fprintf (fout,"|            ");
				fprintf (fout,"|            ");
				fprintf (fout,"|            ");
				fprintf (fout,"|            ");
				fprintf (fout,"|        |\n");
			}
			cc = find_rec (pons, &pons_rec, NEXT, "r");
		}
	}
	else
	{
		fprintf (fout,"|%3d",++line_no);
		fprintf (fout,"|%16.16s",inmr_rec.item_no);
		fprintf (fout,"|%30.30s",poln_rec.item_desc);
		fprintf (fout,"| %4.4s ", inum_rec.uom);
		fprintf (fout,"|%6.0f",quantity * CnvFct);

		wk_tot = twodec (poln_rec.fob_fgn_cst);
		wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [0] += wk_tot;

		fprintf (fout,"|%11.2f ",  poln_rec.fob_fgn_cst/CnvFct);
		fprintf (fout,"|%9.4f",  poln_rec.exch_rate);

		fprintf (fout,"|%11.2f ",  poln_rec.frt_ins_cst/CnvFct);
		wk_tot = out_cost (poln_rec.frt_ins_cst, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [1] += wk_tot;

		fprintf (fout,"|%11.2f ",  poln_rec.duty/CnvFct);
		wk_tot = out_cost (poln_rec.duty, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [2] += wk_tot;

		fprintf (fout,"|%11.2f ",  poln_rec.licence/CnvFct);
		wk_tot = out_cost (poln_rec.licence, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [3] += wk_tot;

		fprintf (fout,"|%11.2f ",  poln_rec.lcost_load/CnvFct);
		wk_tot = twodec (poln_rec.lcost_load);
		wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [4] += wk_tot;

		fprintf (fout,"|%11.2f ",  poln_rec.land_cst/CnvFct);
		wk_tot = twodec (poln_rec.land_cst);
		wk_tot = out_cost (wk_tot, inmr_rec.outer_size);
		wk_tot *= quantity;
		wk_tot = twodec (wk_tot);
		printTotal [5] += wk_tot;

		fprintf (fout,"|%10.10s|\n",  DateToString (poln_rec.due_date));
	}
	PrintInex ();

}

void
PrintHeading (
 void)
{
	fprintf (fout,".R=============");
	fprintf (fout,"===================");
	fprintf (fout,"===================");
	fprintf (fout,"===========================================");	
	fprintf (fout,"=============");
	fprintf (fout,"===============");
	fprintf (fout,"===============");
	fprintf (fout,"==============\n");

	fprintf (fout,"=============");
	fprintf (fout,"===================");
	fprintf (fout,"===================");
	fprintf (fout,"===========================================");	
	fprintf (fout,"=============");
	fprintf (fout,"===============");
	fprintf (fout,"===============");
	fprintf (fout,"==============\n");

	fprintf (fout,"|LINE ");
	fprintf (fout,"| SUPPLIER PART NO ");
	fprintf (fout,"|     ITEM  NO     ");
	fprintf (fout,"|     ITEM  DESCRIPTION                    ");	
	fprintf (fout,"|  QUANTITY  ");
	fprintf (fout,"| UOM  ");
	fprintf (fout,"|     COST     ");
	fprintf (fout,"|  TOTAL COST  ");
	fprintf (fout,"|  DUE DATE  |\n");

	fprintf (fout,"|-----");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");	
	fprintf (fout,"|------------");
	fprintf (fout,"|------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|------------|\n");
}

void
PrintCHeading (
 void)
{
	fprintf (fout,".R====");
	fprintf (fout,"=================");
	fprintf (fout,"===============================");
	fprintf (fout,"=======");
	fprintf (fout,"=======");
	fprintf (fout,"=============");
	fprintf (fout,"==========");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"============\n");

	fprintf (fout,"====");
	fprintf (fout,"=================");
	fprintf (fout,"===============================");
	fprintf (fout,"=======");
	fprintf (fout,"=======");
	fprintf (fout,"=============");
	fprintf (fout,"==========");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"============\n");

	fprintf (fout,"|L/#");
	fprintf (fout,"|  ITEM NUMBER.  ");
	fprintf (fout,"|      ITEM DESCRIPTION.       ");
	fprintf (fout,"| UOM. ");
	fprintf (fout,"| QTY. ");
	fprintf (fout,"| COST (FGN) ");
	fprintf (fout,"|EXCH RATE");
	fprintf (fout,"| F&I (LOC)  ");
	fprintf (fout,"|   DUTY.    ");
	fprintf (fout,"|  LICENCE.  ");
	fprintf (fout,"|INT/BANK/OTH");
	fprintf (fout,"| LAND COST. ");
	fprintf (fout,"| DUE DATE |\n");

	fprintf (fout,"|---");
	fprintf (fout,"|----------------");
	fprintf (fout,"|------------------------------");
	fprintf (fout,"|------");
	fprintf (fout,"|------");
	fprintf (fout,"|------------");
	fprintf (fout,"|---------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|----------|\n");
}

/*==================================
| Search on Purchase Order Number. |
==================================*/
void
SrchPohr (
 char *key_val)
{
	work_open ();
	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type, (CREDIT) ? "C" : "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",key_val);

	if (CREDIT)
		save_rec ("#Purchase Order ","#Date"); 
	else
		save_rec ("#Purchase Return","#Date"); 
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (pohr_rec.br_no,comm_rec.est_no) && 
		      !strncmp (pohr_rec.pur_ord_no,key_val,strlen (key_val)))
	{
		if (CREDIT && pohr_rec.type [0] == 'O')
			break;

		if (!CREDIT && pohr_rec.type [0] == 'C')
			break;

		if (pohr_rec.hhsu_hash == sumr_rec.hhsu_hash && 
		     pohr_rec.type [0] != 'D')
		{
			strcpy (err_str, DateToString (pohr_rec.date_raised));
			cc = save_rec (pohr_rec.pur_ord_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type, (CREDIT) ? "C" : "O");
	pohr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	sprintf (pohr_rec.pur_ord_no,"%-15.15s",temp_str);
	cc = find_rec (pohr,&pohr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "pohr", "DBFIND");
}

int
CheckPogd (
 void)
{
	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
	pogd_rec.line_no = 0;
	cc = find_rec (pogd, &pogd_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void		
PrintPoSum (
 void)
{
	fprintf (fout,"|-----");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");	
	fprintf (fout,"|------------");
	fprintf (fout,"|------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|------------|\n");

	fprintf (fout,"|     ");
	fprintf (fout,"|                  ");
	fprintf (fout,"|                  ");
	fprintf (fout,"|                                          ");	
	fprintf (fout,"|            ");
	fprintf (fout,"|      ");
	fprintf (fout,"| TOTAL COSTS  ");
	fprintf (fout,"|%13.2f ", printTotal [0]);
	fprintf (fout,"|            |\n");
}

void
PrintGrSum (
 void)
{
	double	tot_fgn = 0.00;
	double	tot_loc = 0.00;
		
	abc_selfield (sumr,"sumr_hhsu_hash");

	fprintf (fout,"|---");
	fprintf (fout,"|----------------");
	fprintf (fout,"|------------------------------");
	fprintf (fout,"|------");
	fprintf (fout,"|------");
	fprintf (fout,"|------------");
	fprintf (fout,"|---------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|----------|\n");
	fprintf (fout,"|   ");
	fprintf (fout,"| TOTAL VALUE OF ");
	fprintf (fout,"  COSTS EXTENDED.              ");
	fprintf (fout,"|      ");
	fprintf (fout,"|      ");
	fprintf (fout,"|%12.2f", printTotal [0]);
	fprintf (fout,"|         ");
	fprintf (fout,"|%12.2f", printTotal [1]);
	fprintf (fout,"|%12.2f", printTotal [2]);
	fprintf (fout,"|%12.2f", printTotal [3]);
	fprintf (fout,"|%12.2f", printTotal [4]);
	fprintf (fout,"|%12.2f", printTotal [5]);
	fprintf (fout,"|          |\n");
	
	fprintf (fout,".LRP17\n");
	fprintf (fout,"====");
	fprintf (fout,"=================");
	fprintf (fout,"===============================");
	fprintf (fout,"=======");
	fprintf (fout,"=======");
	fprintf (fout,"=============");
	fprintf (fout,"==========");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"=============");
	fprintf (fout,"============\n");

	fprintf (fout,"!%-48.48sP U R C H A S E  O R D E R   C O S T I N G   S U M M A R Y%58.58s!\n"," "," ");
	fprintf (fout,"!                 ");
	fprintf (fout,"=======================");
	fprintf (fout,"=============");
	fprintf (fout,"=========");
	fprintf (fout,"==================");
	fprintf (fout,"======");
	fprintf (fout,"==================");
	fprintf (fout,"=============");
	fprintf (fout,"==================");
	fprintf (fout,"=                            !\n");

	fprintf (fout,"!                 ");
	fprintf (fout,"!     DESCRIPTION.     ");
	fprintf (fout,"!   SPREAD   ");
	fprintf (fout,"!SUPPLIER");
	fprintf (fout,"!    INVOICE NO   ");
	fprintf (fout,"!CURR.");
	fprintf (fout,"! FOREIGN VALUE.  ");
	fprintf (fout,"! EXCH RATE. ");
	fprintf (fout,"!  LOCAL  VALUE.  ");
	fprintf (fout,"!                            !\n");

	fprintf (fout,"!                 ");
	fprintf (fout,"!----------------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!--------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!-----");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!------------");
	fprintf (fout,"!-----------------");
	fprintf (fout,"!                            !\n");

	strcpy (pogd_rec.co_no, comm_rec.co_no);
	pogd_rec.hhpo_hash = pohr_rec.hhpo_hash;
	pogd_rec.line_no = 0;

	cc = find_rec (pogd, &pogd_rec, GTEQ, "r");
	while (!cc && !strcmp (pogd_rec.co_no, comm_rec.co_no) && 
                      pogd_rec.hhpo_hash == pohr_rec.hhpo_hash)
	{
		fprintf (fout,"!                 !");
		fprintf (fout," %-20.20s !",pogd_rec.category);
		if (pogd_rec.allocation [0] == 'D')
			fprintf (fout,"  Value.    !");

		if (pogd_rec.allocation [0] == 'W')
			fprintf (fout,"  Weight.   !");

		if (pogd_rec.allocation [0] == 'V')
			fprintf (fout,"  Volume.   !");

		if (pogd_rec.allocation [0] == ' ')
			fprintf (fout,"  None.     !");

		if (pogd_rec.hhsu_hash == 0L)
			fprintf (fout,"        !");
		else
		{
			cc = find_hash (sumr,&sumr_rec,EQUAL,"r",pogd_rec.hhsu_hash);
			if (cc)
				fprintf (fout,"Unknown.!");
			else
				fprintf (fout," %6.6s !", sumr_rec.crd_no);
		}
		fprintf (fout," %-15.15s !",pogd_rec.invoice);
		fprintf (fout," %-3.3s !",  pogd_rec.currency);
		fprintf (fout," %15.2f !",  pogd_rec.foreign);
		fprintf (fout," %10.4f !",  pogd_rec.exch_rate);
		fprintf (fout," %15.2f !",  pogd_rec.nz_value);
		fprintf (fout,"                            !\n");

		tot_fgn += pogd_rec.foreign;
		tot_loc += pogd_rec.nz_value;

		cc = find_rec (pogd, &pogd_rec, NEXT, "r");
	}
	fprintf (fout,"!                 ");
	fprintf (fout,"! TOTALS               ");
	fprintf (fout,"!            ");
	fprintf (fout,"!        ");
	fprintf (fout,"!                 ");
	fprintf (fout,"!     ");
	fprintf (fout,"! %15.2f ",tot_fgn);
	fprintf (fout,"!            ");
	fprintf (fout,"! %15.2f ",tot_loc);
	fprintf (fout,"!                            !\n");
	fprintf (fout,"!                 ");
	fprintf (fout,"=======================");
	fprintf (fout,"=============");
	fprintf (fout,"=========");
	fprintf (fout,"==================");
	fprintf (fout,"======");
	fprintf (fout,"==================");
	fprintf (fout,"=============");
	fprintf (fout,"==================");
	fprintf (fout,"=                            !\n");

	fprintf (fout,"!                 NOTE : Costing summary t");
	fprintf (fout,"otals may vary to Purchase order totals due to c");
	fprintf (fout,"ost spreading being done on a per each basis. ");
	fprintf (fout,"                             !\n");
	
	abc_selfield (sumr, (!envVarCrFind) ? "sumr_id_no" : "sumr_id_no3");
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
		if (COSTING)
			rv_pr (ML (mlPoMess150),22,0,1);
		if (PORDER)
			rv_pr (ML (mlPoMess151),25,0,1);
		if (CREDIT)
			rv_pr (ML (mlPoMess152),25,0,1);
		move (0,1);
		line (80);

		box (0,3,80,4);

		move (0,20);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,clip (comm_rec.co_name));
		strcpy (err_str,ML (mlStdMess039));
		print_at (21,40,err_str,comm_rec.est_no,clip (comm_rec.est_name));

		move (0,22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintInex (
 void)
{

	inex_rec.hhbr_hash = inmr_rec.hhbr_hash;
	inex_rec.line_no   = 0;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");

	if (cc)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (PORDER || CREDIT)
		{
			fprintf (fout, "| %3s "     , " ");
			fprintf (fout, "| %16.16s " ,  " ");
			fprintf (fout, "| %16.16s " ,  " ");
			fprintf (fout, "| %40.40s " ,inex_rec.desc);
			fprintf (fout, "| %10.2s " , " ");

			fprintf (fout, "| %-4.4s "  ,  " ");
			fprintf (fout, "| %12.2s "  ,  " ");
			fprintf (fout, "|%13.2s "   ,  " ");
			fprintf (fout, "| %10.10s |\n",  " ");
			
		}
		else
		{
			fprintf (fout,"|%3s"," ");
			fprintf (fout,"|%16.16s"," ");
			fprintf (fout,"|%30.30s",inex_rec.desc);
			fprintf (fout,"|%6.0s"," ");
			fprintf (fout,"|%6.0s"," ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%9.4s",  " ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%11.2s ",  " ");
			fprintf (fout,"|%10.10s|\n",  " ");
		}
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
}
