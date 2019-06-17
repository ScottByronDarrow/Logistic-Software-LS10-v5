/*=====================================================================
|  Copyright (C) 1999 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: pc_wodel.c,v 5.4 2002/07/24 08:39:00 scott Exp $
|  Program Name  : (pc_wodel.c    )                                 |
|  Program Desc  : (Works Order Deletion Program.               )   |
|---------------------------------------------------------------------|
|  Date Written  : 17/02/92        | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: pc_wodel.c,v $
| Revision 5.4  2002/07/24 08:39:00  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:20:14  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:14:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:18  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_wodel.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_wodel/pc_wodel.c,v 5.4 2002/07/24 08:39:00 scott Exp $";

#define	TXT_REQD
#define	TABLINES	6
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<proc_sobg.h>

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcoiRecord	pcoi_rec;
struct pcrqRecord	pcrq_rec;
struct pcwcRecord	pcwc_rec;
struct pcwlRecord	pcwl_rec;
struct pcwoRecord	pcwo_rec;
struct rgrsRecord	rgrs_rec;
struct esmrRecord	esmr_rec;

	char	acronym [10];

	char	*data	= "data",
			*pcwo2	= "pcwo2";

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	LOT_CTRL	 (inmr_rec.lot_ctrl [0] == 'Y')

	FILE	*fout;

	int		envVarPcGenNum	=	FALSE,
			printerOpen		=	FALSE,
			batchFlag		=	FALSE;

	long	hhwoHash		=	0L,
			currHhcc		=	0L,
			wip_mat_hash	=	0L, 
			wip_lbr_hash	=	0L, 
			wip_ovh_hash	=	0L, 
			wip_oth_hash	=	0L, 
			del_job_hash	=	0L;

	char	wip_mat_acc [MAXLEVEL + 1], 
			wip_lbr_acc [MAXLEVEL + 1], 
			wip_ovh_acc [MAXLEVEL + 1], 
			wip_oth_acc [MAXLEVEL + 1], 
			del_job_acc [MAXLEVEL + 1],
			localCurrency [4],
			mlWoDel [10][101],
			oldStatus [2];

struct storeRec 
{
	long	hhcuHash;
	float	customerQty;
} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	order_no [8];
	char	batch_no [11];
	char	reqBrNo [3];
	char	reqBrName [16];
	char	reqWhNo [3];
	char	reqWhName [16];
	char	recBrNo [3];
	char	recBrName [16];
	char	recWhNo [3];
	char	recWhName [16];
	char	status [21];
	int		priority;
	char	sys_dte [11];
	long	lsys_dte;
	long	cre_dte;
	long	hhbr_hash;
	char	item_no [17];
	int		bom_alt;
	int		rtg_alt;
	int		bom_alt_old;
	int		rtg_alt_old;
	char	strength [6];
	char	desc [36];
	char	desc2 [41];
	char	std_uom [5];
	char	alt_uom [5];
	float	qty_rqd;
	char	std_batch_str [15];
	float	std_batch;
	float	min_batch;
	float	max_batch;
	float	prd_mult;
	char	customerNo [7];
	char	customerName [41];
	float	customerQty;
	long	rqd_dte;
	char	text_line [71];
	char	dummy [11];

	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "order_no", 	 2, 18, CHARTYPE, 
		"UUUUUUU", "          ", 
		" ", "", "Order Number", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.order_no}, 
	{1, LIN, "batch_no", 	 2, 43, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Batch Number", " ", 
		NO, NO,  JUSTLEFT, "", "", local_rec.batch_no}, 
	{1, LIN, "priority", 	 2, 95,INTTYPE, 
		"N", "          ", 
		" ", "5", "Priority", " ", 
		NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.priority}, 
	{1, LIN, "cre_dte", 	 2, 118, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Date Raised", " ", 
		NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.cre_dte}, 
	{1, LIN, "item_no", 	 4, 18, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.item_no}, 
	{1, LIN, "bom_alt", 	 4, 55, INTTYPE, 
		"NNNNN", "          ", 
		" ", "1", "BOM Alt. No. : ", " ", 
		NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.bom_alt}, 
	{1, LIN, "rtg_alt", 	 5, 55, INTTYPE, 
		"NNNNN", "          ", 
		" ", "1", "Routing No.  : ", " ", 
		NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.rtg_alt}, 
	{1, LIN, "strength", 	 5, 18, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Strength", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.strength}, 
	{1, LIN, "desc", 		 7, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.desc}, 
	{1, LIN, "desc2", 	 8, 18, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.desc2}, 
	{1, LIN, "std_uom", 	 5, 88, CHARTYPE, 
		"AAAA", "          ", 
		"", "", "Standard", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.std_uom}, 
	{1, LIN, "alt_uom", 	 5, 110, CHARTYPE, 
		"AAAA", "          ", 
		"", "", "  Alternate ", " ", 
		NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom}, 
	{1, LIN, "qty_rqd", 	10, 18, FLOATTYPE, 
		"NNNNNNN.NNNNNN", "          ", 
		" ", local_rec.std_batch_str, "Quantity Reqd", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_rqd}, 
	{1, LIN, "rqd_dte", 	11, 18, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.sys_dte, "Reqd Date", " ", 
		NA, NO, JUSTLEFT, "", "", (char *) &local_rec.rqd_dte}, 
	{1, LIN, "reqBrNo",	 10, 60, CHARTYPE,
		"AA", "          ",
		" ", " ", "Req. Br:", "Requesting Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqBrNo},
	{1, LIN, "reqBrName",	 10, 65, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqBrName},
	{1, LIN, "reqWhNo",	 10, 105, CHARTYPE,
		"AA", "          ",
		" ", " ", "Wh:", "Requesting Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqWhNo},
	{1, LIN, "reqWhName",	 10, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqWhName},
	{1, LIN, "recBrNo",	 11, 60, CHARTYPE,
		"AA", "          ",
		" ", " ", "Rec. Br:", "Receiving Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recBrNo},
	{1, LIN, "recBrName",	 11, 65, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recBrName},
	{1, LIN, "recWhNo",	 11, 105, CHARTYPE,
		"AA", "          ",
		" ", " ", "Wh:", "Receiving Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recWhNo},
	{1, LIN, "recWhName",	 11, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recWhName},

	{2, TAB, "customerNo", 	MAXLINES, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "000000", "Customer", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customerNo}, 
	{2, TAB, "customerName", 	0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "               Customer Name              ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customerName}, 
	{2, TAB, "customerQty", 	0, 1, FLOATTYPE, 
		"NNNNNNN.NNNNNN", "          ", 
		" ", "0.00", "       Quantity ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.customerQty}, 

	{3, TXT, "spec_inst", 	13, 70, 0, 
		"", "          ", 
		" ", " ", "S P E C I A L   I N S T R U C T I O N S", " ", 
		7, 60, 100, "", "", local_rec.text_line}, 

	{0, LIN, "", 		 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<SrchPcwo2.h>

/*===============================
| function prototypes |
====================*/
void 	InitML 					 (void);
void 	shutdown_prog 			 (void);
int 	ReadDefault 			 (void);
int 	heading 				 (int);
int 	spec_valid 				 (int);
int 	DisplayDetails 			 (int);
int 	DeleteFunc 				 (void);
void 	OpenDB 					 (void);
void 	CloseDB 				 (void);
void 	LoadCustomerDetails 	 (void);
void 	LoadSpecialInstructions (void);
void 	ReportStock 			 (void);
void 	AddGlpc 				 (long, char *, char *, double, char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv [])
{
	char	*sptr;
	int		i;

	if (argc != 2)
	{
		print_at (0,0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}

	/*-------------------------------------------------------
	| Works order number is M (anually or S (ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		envVarPcGenNum = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envVarPcGenNum = TRUE;

	local_rec.printerNo = atoi (argv [1]);
	printerOpen = FALSE;

	SETUP_SCR (vars);


	/*-------------------------------
	| Setup required parameters	|
	-------------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	OpenDB ();

	InitML ();

	if (ReadDefault ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	strcpy (local_rec.sys_dte, DateToString (TodaysDate ()));
	local_rec.lsys_dte = TodaysDate ();

	swide ();
	tab_row = 13;
	tab_col = 0;

	/*=======================================
	| Beginning of input control loop	|
	=======================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags	|
		-----------------------*/
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;

		lcount [2] = 0;
		lcount [3] = 0;
		init_vars (3);
		init_vars (2);
		init_vars (1);
		/*-------------------------------
		| Enter screen 1 linear input	|
		-------------------------------*/
		strcpy (local_rec.status, "                    ");
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			abc_unlock (pcwo);
			abc_unlock (pcwo2);
			continue;
		}

		heading (1);
		scn_display (1);
		crsr_on ();
		i = prmptmsg (ML (mlStdMess151), "YyNn", 0, 23);
		if (i == 'Y' || i == 'y')
			DeleteFunc ();
		else
		{
			abc_unlock (pcwo);
			abc_unlock (pcwo2);
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
InitML (
 void)
{
	strcpy (mlWoDel [1], ML ("Std"));
	strcpy (mlWoDel [2], ML ("Min"));
	strcpy (mlWoDel [3], ML ("Max"));
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	if (printerOpen)
		pclose (fout);

	CloseDB (); 
	FinishProgram ();
}

int
ReadDefault (void)
{
	abc_selfield (glmr, "glmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	currHhcc = ccmr_rec.hhcc_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		inmr_rec.category
	);
	wip_mat_hash = glmrRec.hhmr_hash;
	strcpy (wip_mat_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D LABR",
		" ",
		inmr_rec.category
	);
	wip_lbr_hash = glmrRec.hhmr_hash;
	strcpy (wip_lbr_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MACH",
		" ",
		inmr_rec.category
	);
	wip_ovh_hash = glmrRec.hhmr_hash;
	strcpy (wip_ovh_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D OTH ",
		" ",
		inmr_rec.category
	);
	wip_oth_hash = glmrRec.hhmr_hash;
	strcpy (wip_oth_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MIS SK ISS",
		" ",
		inmr_rec.category
	);
	del_job_hash = glmrRec.hhmr_hash;
	strcpy (del_job_acc, glmrRec.acc_no);

	abc_selfield (glmr, "glmr_hhmr_hash");

	return (EXIT_SUCCESS);
}

/*=======================
| Open data base files	|
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
	abc_alias (pcwo2, pcwo);

	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcoi,  pcoi_list, PCOI_NO_FIELDS, "pcoi_id_no");
	open_rec (pcrq,  pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwl,  pcwl_list, PCWL_NO_FIELDS, "pcwl_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	OpenGlmr ();
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcgl);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcoi);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwl);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgrs);
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose (glmr);
	GL_Close ();

	abc_dbclose (data);
}

/*=======================
| Display heading scrn	|
=======================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		swide ();
		clear ();
		centre_at (0, 132, ML (mlPcMess029));


		line_at (3,1,131);
		line_at (6,67,64);
		line_at (9,1,131);

		rv_pr (local_rec.status, 59, 2, FALSE);
		strcpy (err_str,ML (mlPcMess014));
		rv_pr (err_str, 88, 4, FALSE);
		strcpy (err_str,ML (mlPcMess014));
		rv_pr (err_str, 89, 7, FALSE);

		print_at (8,  69, "%-4.4s %14.6f", mlWoDel [1], inei_rec.std_batch);
		print_at (8,  90, "%-4.4s %14.6f", mlWoDel [2], inei_rec.min_batch);
		print_at (8, 111, "%-4.4s %14.6f", mlWoDel [3], inei_rec.max_batch);

		box (0, 1, 132, 10);
		box (66, 3, 65, 5);
		scn_write (1);
		if (scn != 1)
			scn_display (1);

		line_at (21,0,132);

		print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_short);
		print_at (22, 40,ML (mlStdMess039), comm_rec.est_no,comm_rec.est_short);
		print_at (22, 55,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_short);

		rv_pr (err_str,13,12,TRUE);
		scn_write (2);
		scn_display (2);

		scn_display (3);
		scn_set (scn);

		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;
	}
	return (EXIT_FAILURE);
}

/*===============================
| Validate entered field (s)	|
===============================*/
int
spec_valid (
 int field)
{
	int		flag;

	if (LCHECK ("order_no"))
	{
		if (dflt_used)
		{
			FLD ("batch_no") = YES;
			return (EXIT_SUCCESS);
		}
		else
			FLD ("batch_no") = NA;

		if (SRCH_KEY)
		{
			SearchOrder (temp_str, "PFIAR", comm_rec.est_no, comm_rec.cc_no);
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		if (envVarPcGenNum)
			strcpy (pcwo_rec.order_no, zero_pad (local_rec.order_no, 7));
		else
			strcpy (pcwo_rec.order_no, local_rec.order_no);
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			print_mess (ML (mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			
		flag = DisplayDetails (FALSE);
		if (flag)
			sprintf (local_rec.order_no, "%-7.7s", " ");
	
		return (flag);
	}

	if (LCHECK ("batch_no"))
	{
		if (SRCH_KEY)
		{
			SearchBatch (temp_str, "PFIAR", comm_rec.est_no, comm_rec.cc_no);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		strcpy (pcwo_rec.batch_no, local_rec.batch_no);
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			sprintf (err_str,ML (mlPcMess138), local_rec.batch_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.batch_no, "%-10.10s", " ");
			return (EXIT_FAILURE);
		}
			
		flag = DisplayDetails (TRUE);
		if (flag)
			sprintf (local_rec.batch_no, "%-10.10s", " ");
	
		return (flag);
	}

	return (EXIT_SUCCESS);
}

int 
DisplayDetails (
 int flag)
{
	batchFlag = flag;

	hhwoHash = pcwo_rec.hhwo_hash;
	strcpy (oldStatus, pcwo_rec.order_status);

	switch (pcwo_rec.order_status [0])
	{
	case	'P':
		strcpy (local_rec.status, "STATUS: Planned     ");
		break;

	case	'F':
		strcpy (local_rec.status, "STATUS: Firm Planned");
		break;

	case	'I':
		strcpy (local_rec.status, "STATUS: Issuing     ");
		break;

	case	'A':
		strcpy (local_rec.status, "STATUS: Allocated   ");
		break;

	case	'R':
		strcpy (local_rec.status, "STATUS: Released    ");
		break;

	case	'D':
		print_mess (ML (mlPcMess004));
		sleep (sleepTime);
		clear_mess ();
		abc_unlock (pcwo);
		return (EXIT_FAILURE);

	case	'C':
		print_mess (ML (mlPcMess136));
		sleep (sleepTime);
		clear_mess ();
		abc_unlock (pcwo);
		return (EXIT_FAILURE);

	default:
		print_mess (ML (mlPcMess005));
		sleep (sleepTime);
		clear_mess ();
		abc_unlock (pcwo);
		return (EXIT_FAILURE);
	};

	if (pcwo_rec.act_prod_qty != 0.00)
	{
		print_mess (ML (mlPcMess135));
		sleep (sleepTime);
		clear_mess ();
		abc_unlock (pcwo);
		return (EXIT_FAILURE);
	}

	rv_pr (local_rec.status, 59, 2, FALSE);

	if (flag)
	{
		strcpy (local_rec.order_no, pcwo_rec.order_no);
		DSP_FLD ("order_no");
	}
	else
	{
		strcpy (local_rec.batch_no, pcwo_rec.batch_no);
		DSP_FLD ("batch_no");
	}

	strcpy (local_rec.reqBrNo, pcwo_rec.req_br_no);
	strcpy (local_rec.reqWhNo, pcwo_rec.req_wh_no);
	strcpy (local_rec.recBrNo, pcwo_rec.rec_br_no);
	strcpy (local_rec.recWhNo, pcwo_rec.rec_wh_no);

	/* find requesting branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.reqBrNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.reqBrName, esmr_rec.short_name);
	DSP_FLD ("reqBrNo");
	DSP_FLD ("reqBrName");

	/* find receiving branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.recBrNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.recBrName, esmr_rec.short_name);
	DSP_FLD ("recBrNo");
	DSP_FLD ("recBrName");

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.reqWhName, ccmr_rec.acronym);
	DSP_FLD ("reqWhNo");
	DSP_FLD ("reqWhName");

	/* find receiving warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.recBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.recWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.recWhName, ccmr_rec.acronym);
	DSP_FLD ("recWhNo");
	DSP_FLD ("recWhName");

	local_rec.cre_dte = pcwo_rec.create_date;
	DSP_FLD ("cre_dte");

	local_rec.priority = pcwo_rec.priority;
	DSP_FLD ("priority");

	cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.hhbr_hash);
	if (cc)
		file_err (cc, inmr, "DBFIND");

	if (ReadDefault ())

	strcpy (local_rec.item_no, inmr_rec.item_no);
	sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
	sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
	sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
	local_rec.hhbr_hash = inmr_rec.hhbr_hash;
	DSP_FLD ("item_no");
	DSP_FLD ("strength");
	DSP_FLD ("desc");
	DSP_FLD ("desc2");

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.std_uom);
	strcpy (local_rec.std_uom, (cc) ? "    " : inum_rec.uom);
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.alt_uom);
	strcpy (local_rec.alt_uom, (cc) ? "    " : inum_rec.uom);
	DSP_FLD ("std_uom");
	DSP_FLD ("alt_uom");

	strcpy (inei_rec.est_no, comm_rec.est_no);
	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (inei, &inei_rec, EQUAL, "r");
	if (cc)
	{
		inei_rec.std_batch = 1;
		inei_rec.min_batch = 1;
		inei_rec.max_batch = 1;
		inei_rec.prd_multiple = 0;
	}
	local_rec.std_batch	= inei_rec.std_batch;
	local_rec.min_batch	= inei_rec.min_batch;
	local_rec.max_batch	= inei_rec.max_batch;
	local_rec.prd_mult	= inei_rec.prd_multiple;
	sprintf (local_rec.std_batch_str, "%14.6f", inei_rec.std_batch);

	print_at (8,  69, "Std. %14.6f", inei_rec.std_batch);
	print_at (8,  90, "Min. %14.6f", inei_rec.min_batch);
	print_at (8, 111, "Max. %14.6f", inei_rec.max_batch);

	local_rec.bom_alt = pcwo_rec.bom_alt;
	local_rec.bom_alt_old = pcwo_rec.bom_alt;
	DSP_FLD ("bom_alt");

	local_rec.rtg_alt = pcwo_rec.rtg_alt;
	local_rec.rtg_alt_old = pcwo_rec.rtg_alt;
	DSP_FLD ("rtg_alt");

	local_rec.rqd_dte = pcwo_rec.reqd_date;
	DSP_FLD ("rqd_dte");

	local_rec.qty_rqd = pcwo_rec.prod_qty;
	DSP_FLD ("qty_rqd");

	LoadCustomerDetails ();

	LoadSpecialInstructions ();

	entry_exit = TRUE;

	return (EXIT_SUCCESS);
}

/*------------------
| Load debtor info |
------------------*/
void
LoadCustomerDetails (
 void)
{
	scn_set (2);
	lcount [2] = 0;

	pcwl_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcwl_rec.hhcu_hash = 0L;
	cc = find_rec (pcwl, &pcwl_rec, GTEQ, "r");
	while (!cc && pcwl_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		cc = find_hash (cumr, &cumr_rec, EQUAL, "r", pcwl_rec.hhcu_hash);
		if (!cc)
		{
			store [lcount [2]].hhcuHash = pcwl_rec.hhcu_hash;
			store [lcount [2]].customerQty = pcwl_rec.cust_qty;
			strcpy (local_rec.customerNo, cumr_rec.dbt_no);
			strcpy (local_rec.customerName, cumr_rec.dbt_name);
			local_rec.customerQty = pcwl_rec.cust_qty;
			putval (lcount [2]++);
		}
		cc = find_rec (pcwl, &pcwl_rec, NEXT, "r");
	}
	scn_display (2);

	scn_set (1);
	return;
}

void
LoadSpecialInstructions (
 void)
{
	scn_set (3);
	lcount [3] = 0;

	pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcoi_rec.line_no = 0;
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	while (!cc && pcoi_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		strcpy (local_rec.text_line, pcoi_rec.text);
		putval (-1);
		lcount [3]++;
		cc = find_rec (pcoi, &pcoi_rec, NEXT, "r");
	}
	scn_display (3);

	scn_set (1);
	return;
}

/*------------------------
| Update pcwo to 'D' etc |
------------------------*/
int
DeleteFunc (
 void)
{
	char	curr_wc [11];
	int		first_time = TRUE;
	long	tmp_time;
	double	del_mat	= 0.00, 		/* Consumed Material	*/
			del_tot	= 0.00, 		/* TOTAL consumed	*/
			tmp_lbr	= 0.00, 
			tmp_ovh	= 0.00; 

	pcms_rec.hhwo_hash = hhwoHash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == hhwoHash)
	{
		ReportStock ();
		del_mat += pcms_rec.amt_issued;

		/*-----------------------------------
		| add sobg record for recalculation |
		| of BOM committed qty              |
		-----------------------------------*/
		add_hash (comm_rec.co_no, 
				 comm_rec.est_no, 
				 "RC",
				 0,
				 pcms_rec.mabr_hash, 
				 currHhcc, 
				 0L, 
				 (double) 0); /* decrease the committed qty of the mat. items */

		abc_unlock (pcms);
		cc = find_rec (pcms, &pcms_rec, NEXT, "u");
	}
	abc_unlock (pcms);

	pcln_rec.hhwo_hash = hhwoHash;
	pcln_rec.seq_no = 0;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (!cc && !find_hash (pcwc, &pcwc_rec, EQUAL, "r", pcln_rec.hhwc_hash))
		strcpy (curr_wc, pcwc_rec.work_cntr);

	while
	 (
		!cc &&
		pcln_rec.hhwo_hash == hhwoHash
	)
	{
		pcrq_rec.hhwo_hash = pcln_rec.hhwo_hash;
		pcrq_rec.seq_no = pcln_rec.seq_no;
		pcrq_rec.line_no = pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
		/*-----------------------------------------------------------
		| If we can't find the pcrq, I think it's fair to assume	|
		| that it doesn't have any act.	times on it.!!				|
		-----------------------------------------------------------*/
		if (cc)
		{
			cc = find_rec (pcln, &pcln_rec, NEXT, "r");
			continue;
		}
		tmp_time = pcrq_rec.act_setup +
			   pcrq_rec.act_run +
			   pcrq_rec.act_clean;
		tmp_lbr = (double) tmp_time * pcln_rec.rate / 60.00;
		tmp_ovh = (double) tmp_time * pcln_rec.ovhd_var / 60.00;

		if (pcln_rec.seq_no < pcwo_rec.rtg_seq)
		{
			del_mat += tmp_lbr;
			del_mat += tmp_ovh;
			del_mat += pcln_rec.ovhd_fix;
			cc = find_rec (pcln, &pcln_rec, NEXT, "r");
			continue;
		}

		if (first_time && pcln_rec.seq_no == pcwo_rec.rtg_seq)
		{
			first_time = FALSE;
			cc = find_hash (pcwc, &pcwc_rec, EQUAL, "r", pcln_rec.hhwc_hash);
			if (cc)
				file_err (cc, pcwc, "DBFIND");
			strcpy (curr_wc, pcwc_rec.work_cntr);
		}

		if (tmp_lbr != 0.00 || tmp_ovh != 0.00)
		{
			cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", pcln_rec.hhrs_hash);
			if (cc)
				file_err (cc, rgrs, "DBFIND");

			cc = find_hash (pcwc, &pcwc_rec, EQUAL, "r", pcln_rec.hhwc_hash);
			if (cc)
				file_err (cc, pcwc, "DBFIND");
		}

		if (tmp_lbr != 0.00)
		{
			cc = find_hash (glmr, &glmrRec, EQUAL, "r", rgrs_rec.dir_hash);
			if (cc)
				file_err (cc, glmr, "DBFIND");

			AddGlpc
			 (
				rgrs_rec.dir_hash, 
				glmrRec.acc_no, 
				pcwc_rec.work_cntr, 
				tmp_lbr, 
				"2"
			);
			del_tot += tmp_lbr;
		}

		if (tmp_ovh != 0.00)
		{
			AddGlpc
			 (
				wip_ovh_hash, 
				wip_ovh_acc, 
				pcwc_rec.work_cntr, 
				tmp_ovh, 
				"2"
			);
			del_tot += tmp_ovh;
		}

		del_mat += pcln_rec.ovhd_fix;
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	del_tot += del_mat;
	AddGlpc (wip_mat_hash, wip_mat_acc, curr_wc, del_mat, "2");
	AddGlpc (del_job_hash, del_job_acc, "          ", del_tot, "1");

	/*-------------------------
	| Remove any pcrq records |
	-------------------------*/
	if (oldStatus [0] == 'A' || oldStatus [0] == 'R')
	{
		pcrq_rec.hhwo_hash = hhwoHash;
		pcrq_rec.seq_no = 0;
		pcrq_rec.line_no = 0;
		cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
		while (!cc && pcrq_rec.hhwo_hash == hhwoHash)
		{
			cc = abc_delete (pcrq);
			if (cc)
				file_err (cc, pcrq, "DBDELETE");

			pcrq_rec.hhwo_hash = hhwoHash;
			pcrq_rec.seq_no = 0;
			pcrq_rec.line_no = 0;
			cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
		}
		abc_unlock (pcrq);
	}

	/*-------------------------------
	| Close pipe to pformat if open |
	-------------------------------*/
	if (printerOpen)
	{
		fprintf (fout, ".EOF\n");
		fflush (fout);
		pclose (fout);
		printerOpen = FALSE;
	}

	/*-----------------------------------
	| add sobg record for recalculation |
	| of final product's on order qty   |
	-----------------------------------*/
	add_hash (pcwo_rec.co_no, /* At manufactured branch/warehouse */
			 pcwo_rec.br_no, 
			 "RP", 
			 0,
			 pcwo_rec.hhbr_hash, 
			 pcwo_rec.hhcc_hash, 
			 0L, 
			 (double) 0); /* decrease the on order qty for the final product */

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, pcwo_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
	if ( (cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r")))
		file_err (cc, ccmr, "DBFIND");
	add_hash (pcwo_rec.co_no, /* At requesting branch/warehouse */
			 pcwo_rec.req_br_no, 
			 "RP", 
			 0,
			 pcwo_rec.hhbr_hash, 
			 ccmr_rec.hhcc_hash, 
			 0L, 
			 (double) 0); /* decrease the on order qty for the final product */

	/*---------------------------
	| Update pcwo to status 'D' |
	---------------------------*/
	strcpy (pcwo_rec.order_status, "D");
	if (batchFlag)
		cc = abc_update (pcwo2, &pcwo_rec);
	else
		cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, batchFlag ? pcwo2 : pcwo, "DBUPDATE");

	recalc_sobg ();

	return (EXIT_SUCCESS);
}

/*---------------------------------------------
| Report on stock that can't be automatically |
| deissued and receipted into stock. Stock in |
| this category will need to be written on    |
---------------------------------------------*/
void
ReportStock (
 void)
{
	char	ord_status [13];
	char	rtg_seq [4];

	if (!printerOpen)
	{
		if ( (fout = popen ("pformat", "w")) == (FILE *)0)
			sys_err ("Error in pformat during (POPEN)", errno, PNAME);

		printerOpen = TRUE;

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.printerNo);
	
		fprintf (fout, ".15\n");
		fprintf (fout, ".PI10\n");
		fprintf (fout, ".L120\n");

		fprintf (fout, ".B1\n");
		fprintf (fout, ".ESTOCK IMBALANCE AFTER WORKS ORDER DELETION\n");
		fprintf (fout, ".ECOMPANY %s : %s\n", 
			comm_rec.co_no, clip (comm_rec.co_name));
		fprintf (fout, ".EBRANCH %s : %s\n", 
			comm_rec.est_no, clip (comm_rec.est_name));
		fprintf (fout, ".EWAREHOUSE %s : %s\n",
			comm_rec.cc_no, clip (comm_rec.cc_name));
		fprintf (fout, ".B1\n");

		fprintf (fout, ".CWORKS ORDER NUMBER: %-7.7s", pcwo_rec.order_no);
		fprintf (fout, "BATCH NUMBER: %-10.10s\n", pcwo_rec.batch_no);
		fprintf (fout, ".CITEM: %-16.16s   ", inmr_rec.item_no);
		fprintf (fout, "DESCRIPTION: %-40.40s\n", inmr_rec.description);
	
		strcpy (ord_status, "            ");
		strcpy (rtg_seq, "N/A");
		switch (oldStatus [0])
		{
		case	'P':
			strcpy (ord_status, "Planned     ");
			break;
	
		case	'F':
			strcpy (ord_status, "Firm Planned");
			break;
	
		case	'I':
			strcpy (ord_status, "Issued      ");
			break;
	
		case	'A':
			strcpy (ord_status, "Allocated   ");
			break;
	
		case	'R':
			strcpy (ord_status, "Released    ");
			sprintf (rtg_seq, "%2d ", pcwo_rec.rtg_seq);
			break;
		}
		fprintf (fout, ".CSTATUS WHEN DELETED: %-10.10s     ", ord_status);
		fprintf (fout, "ROUTING SEQUENCE WHEN DELETED: %-3.3s\n", rtg_seq);
	
		fprintf (fout, "========================================");
		fprintf (fout, "========================================");
		fprintf (fout, "=======================================\n");
	
		fprintf (fout, "|     MATERIAL     ");
		fprintf (fout, "|               DESCRIPTION                ");
		fprintf (fout, "| ISSUED FOR SEQ. ");
		fprintf (fout, "| QUANTITY REQUIRED ");
		fprintf (fout, "| QUANTITY ISSUED |\n");
	
		fprintf (fout, "|------------------");
		fprintf (fout, "+------------------------------------------");
		fprintf (fout, "+-----------------");
		fprintf (fout, "+-------------------");
		fprintf (fout, "+-----------------|\n");

		fprintf (fout, ".R======================================");
		fprintf (fout, "========================================");
		fprintf (fout, "=========================================\n");
	}

	cc = find_hash (inmr, &inmr2_rec, COMPARISON, "r", pcms_rec.mabr_hash);
	if (cc)
	{
		strcpy (inmr2_rec.item_no, "UNKNOWN ITEM    ");
		sprintf (inmr2_rec.description, "%-40.40s", "UNKNOWN ITEM");
	}

	fprintf (fout, "| %-16.16s ", inmr2_rec.item_no);
	fprintf (fout, "| %-40.40s ", inmr2_rec.description);
	fprintf (fout, "|       %2d        ", pcms_rec.iss_seq);
	fprintf (fout, "|  %14.6f   ", pcms_rec.matl_qty);
	fprintf (fout, "| %14.6f  |\n", pcms_rec.qty_issued);

	fflush (fout);
}

/*===============================
| Add a trans to the pcgl file.	|
| NB: amount should be in cents	|
===============================*/
void
AddGlpc
 (
	long	hash,
	char	*acc, 
	char	*wc, 
	double	amount,
	char	*type
)
{
	int		periodMonth;

	if (amount == 0.00)
		return; 
	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, "19");
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;
	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no,"%02d", periodMonth);

	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", wc);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 	= amount;
	pcgl_rec.loc_amount = amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	strcpy (pcgl_rec.acc_no, acc);
	pcgl_rec.hhgl_hash = hash;

	strcpy (pcgl_rec.jnl_type, type);
	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, pcgl, "DBADD");
}

