/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sdmaint.c,v 5.6 2002/01/21 09:47:09 scott Exp $
|  Program Name  : (so_sdmaint.c)                                     |
|  Program Desc  : (Maintain Sales Order Shipping Details      )      |
|---------------------------------------------------------------------|
|  Date Written  : (10/07/92)      | Author       : Scott Darrow      |
|---------------------------------------------------------------------|
| $Log: so_sdmaint.c,v $
| Revision 5.6  2002/01/21 09:47:09  scott
| ..
|
| Revision 5.5  2002/01/21 09:42:14  scott
| ..
|
| Revision 5.4  2002/01/21 09:37:54  scott
| ..
|
| Revision 5.3  2002/01/21 09:16:10  scott
| Updated as fields required in index missing
|
| Revision 5.2  2001/08/09 09:22:05  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:52:03  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/02 05:21:49  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add sleep delay - did not work with LS10-GUI
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to ensure "0xff" is used instead of "~" for end of range.
|
| Revision 4.0  2001/03/09 02:41:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/03/08 02:12:16  scott
| Updated to increase the delivery address number from 0-999 to 0-32000
| This change did not require a change to the schema
| As a general practice all programs have had app.schema added and been cleaned
|
| Revision 3.2  2000/12/22 07:53:10  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/12/06 09:34:28  scott
| First release before testing - almost total re-write.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sdmaint.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sdmaint/so_sdmaint.c,v 5.6 2002/01/21 09:47:09 scott Exp $";

#define	TXT_REQD
#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <ml_tr_mess.h>

#define	SLEEP_TIME	2

#define NO_KEY(x)	 (vars [x].required == NA || \
			  		  (vars [x].required == NI && prog_status == ENTRY) || \
			  		  vars [x].required == ND)

#define	FREIGHT_CHG	 (sohr_rec.frei_req [0] == 'Y' && envAutomaticFreight)
#define	EXPORT_INV	 (cohr_rec.ord_type [0] == 'E')
#define	EXPORT_ORD	 (sohr_rec.ord_type [0] == 'E')
#define	_EXPORT		 (INVOICE && EXPORT_INV) || (!INVOICE && EXPORT_ORD)

#define	INVOICE		 (typeFlag [0] == 'I')
#define	INV_STR		 (INVOICE) ? "Invoice" : "Sales Order"
#define AUTO_SK_UP	 (createFlag [0] == envVarAutoSkUp [0])

extern	int	X_EALL;
extern	int	Y_EALL;
extern	int	TruePosition;

	int		envAutomaticFreight = FALSE;
	int		envSoFreightCharge 	= 3;
	int		envVarDbCo = 0;
	int		envVarDbFind = 0;
	int		noTax = 0;
	int		newMark = TRUE;

	char	branchNumber [3];
	char	typeFlag [2];
	char	findFlag [2];
	char	createFlag [2];
	char	envVarAutoSkUp [2];

	char	*data	=	"data";

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct sosdRecord	sosd_rec;
struct cudiRecord	cudi_rec;
struct exsiRecord	exsi_rec;
struct exafRecord	exaf_rec;
struct trshRecord	trsh_rec;
struct trzmRecord	trzm_rec;
struct trcmRecord	trcm_rec;
struct trclRecord	trcl_rec;
struct inmrRecord	inmr_rec;

	char	*scn_desc [] = 
			{
				"HEADER SCREEN.", 
				"SHIPPING DETAILS."
			};

/*===========================
| Local & Screen Structures |
===========================*/
struct {
	char	dummy [11];
	char	cust_no [7];
	char	inv_no [9];
	char	inv_prmpt [31];
	char	inv_label [15];
	char	other [3] [31];
	char	spinst [3] [61];
	char	prev_dbt_no [7];
	char	prev_inv_no [9];
	char	ins_det [31];
	char	del_name [41];
	char	del_add [3] [41];
	char	cons_no [17];
	double	insurance;
	double	deposit;
	double	freight;
	double	ex_disc;
	double	other_cost [3];
	int		no_cartons;
	char	sell_terms [4];
	char	sell_desc [31];
	char	pay_terms [41];
	char	defaultDelZone [7];
	char	origDelRequired [2];
	char	dflt_date [11];
	char	del_req [2];
	long	del_date;
	double	est_freight;
	float	no_kgs;

} local_rec;            

static	struct	var	vars [] =
{
	{1, LIN, "debtor", 	 2, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.prev_dbt_no, "Customer Number       ", " ", 
		 NE, NO,  JUSTLEFT, "", "", local_rec.cust_no}, 
	{1, LIN, "name", 	 2, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{1, LIN, local_rec.inv_label, 	 3, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		"0", local_rec.prev_inv_no, local_rec.inv_prmpt, " ", 
		 NE, NO, JUSTRIGHT, "", "", local_rec.inv_no}, 
	{1, LIN, "carrierCode", 	 5, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Carrier Code.         ", "Enter carrier code, [SEARCH] available.", 
		YES, NO,  JUSTLEFT, "", "", trcm_rec.carr_code}, 
	{1, LIN, "carr_desc", 	 5, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Description   ", " ", 
		 NA, NO,  JUSTLEFT, "", "", trcm_rec.carr_desc}, 
	{1, LIN, "deliveryZoneCode", 	 6, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", local_rec.defaultDelZone, "Delivery Zone         ", "Enter Delivery Zone Code [SEARCH]. ", 
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone}, 
	{1, LIN, "deliveryZoneDesc", 	 6, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Delivery Zone Desc    ", " ", 
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc}, 
	{1, LIN, "deliveryRequired", 	 7, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Delivery Reqd. (Y/N)  ", "Enter Y(es)) for Delivery. <default = N(o)> ", 
		 YES, NO, JUSTLEFT, "YN", "", local_rec.del_req}, 
	{1, LIN, "deliveryDate", 	7, 68, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.dflt_date, "Delivery Date         ", " ", 
		NA, NO,  JUSTLEFT, " ", "", (char *)&local_rec.del_date}, 
	{1, LIN, "cons_no", 	 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Consignment no.       ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.cons_no}, 
	{1, LIN, "no_cartons", 	 8, 68, INTTYPE, 
		"NNNN", "          ", 
		" ", "0", "Number Cartons.       ", " ", 
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.no_cartons}, 
	{1, LIN, "est_freight", 	 9, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Est Freight           ", " ", 
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.est_freight}, 
	{1, LIN, "tot_kg", 	 9, 68, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", "0.00", "Total Kgs.            ", " ", 
		 NA, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.no_kgs}, 
	{1, LIN, "freight", 	 10, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Freight Amount.       ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.freight}, 
	{1, LIN, "insurance", 	 10, 68, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Insurance             ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.insurance}, 
	{1, LIN, "deposit", 	11, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Deposit               ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.deposit}, 
	{1, LIN, "discount", 	11, 68, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Special Discount      ", " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.ex_disc}, 
	{1, LIN, "other_1", 	12, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", local_rec.other [0], " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.other_cost [0]}, 
	{1, LIN, "other_2", 	12, 68, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", local_rec.other [1], " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.other_cost [1]}, 
	{1, LIN, "other_3", 	13, 2, MONEYTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", local_rec.other [2], " ", 
		YES, NO, JUSTRIGHT, "0.00", "999999.99", (char *)&local_rec.other_cost [2]}, 
	{1, LIN, "pay_term", 	 14, 2, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", cumr_rec.crd_prd, "Payment Terms         ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.pay_terms}, 
	{1, LIN, "shipname", 	 16, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dbt_name, "Ship to name          ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.del_name}, 
	{1, LIN, "shipaddr1", 	16, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr1, "Ship to address 1     ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.del_add [0]}, 
	{1, LIN, "shipaddr2", 	17, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr2, "Ship to address 2     ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.del_add [1]}, 
	{1, LIN, "shipaddr3", 	17, 68, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cumr_rec.dl_adr3, "Ship to address 3     ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.del_add [2]}, 
	{1, LIN, "ship_method", 18, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Shipment method       ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [0]}, 
	{1, LIN, "spcode1", 	19, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Instruction 1         ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [1]}, 
	{1, LIN, "spcode2", 	20, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Instruction 2         ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.spinst [2]}, 
	{2, TXT, "text", 	5, 30, 0, 
		"", "          ", 
		" ", " ", "               S H I P P I N G   D E T A I L S              ", " ", 
		13, 60,  100, "", "", sosd_rec.text}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include	<proc_sobg.h>
#include 	<p_terms.h>
#include 	<FindCumr.h>

/*=======================
| Function Declarations |
=======================*/
int		SrchCudi 			(int);
int 	LoadDetails 		(void);
int  	LoadSosd 			(long);
int  	heading 			(int);
int  	spec_valid 			(int);
void	CalculateFreight	(float, double, double, double);
void	CloseDB 			(void);
void	CloseTransportFiles	(void);
void	OpenTransportFiles	(char *);
void	SrchTrcm			(char *);
void	SrchTrzm			(char *);
void 	CalculateTax 		(void);
void 	OpenDB 				(void);
void 	PrintCompanyStuff 	(void);
void 	SaveDetails 		(void);
void 	SrchCohr  			(char *);
void 	SrchExaf 			(char *);
void 	SrchExsi  			(char *);
void 	SrchPay 			(void);
void 	SrchSohr  			(char *);
void 	Update 				(void);
void 	shutdown_prog 		(void);
float 	GetOrderWeight 		(void);
float 	GetInvoiceWeight 	(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	char	*sptr;

	SETUP_SCR (vars);

	if (argc != 4)
	{
		print_at (0, 0, mlSoMess767, argv [0]);
		print_at (1, 0, mlSoMess768);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;

	switch (argv [1] [0])
	{
	case	'I':
	case	'i':
		strcpy (typeFlag, "I");
		strcpy (local_rec.inv_prmpt, "Invoice Number        ");
		strcpy (local_rec.inv_label, "invoice_no");
		break;

	case	'S':
	case	's':
		strcpy (typeFlag, "S");
		strcpy (local_rec.inv_prmpt, "Sales Order Number    ");
		strcpy (local_rec.inv_label, "order_no");
		break;

	default:
		print_at (0, 0, mlSoMess768);
		return (EXIT_FAILURE);
	}

	sprintf (findFlag, "%-1.1s", argv [2]);
	sprintf (createFlag, "%-1.1s", argv [3]);

	/*-------------------------------------
	| Check for Automatic freight charge. |
	-------------------------------------*/
	sptr = chk_env ("SO_AUTO_FREIGHT");
	envAutomaticFreight = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*--------------------------
	| How if Freight Charged.  |
	--------------------------*/
	sptr = chk_env ("SO_FREIGHT_CHG");
	envSoFreightCharge = (sptr == (char *) 0) ? 3 : atoi (sptr);

	sptr = chk_env ("SO_OTHER_1");
	sprintf (local_rec.other [0], "%-22.22s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_2");
	sprintf (local_rec.other [1], "%-22.22s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sptr = chk_env ("SO_OTHER_3");
	sprintf (local_rec.other [2], "%-22.22s", (sptr == (char *)0) 
						? "Other Costs." : sptr);

	sprintf (envVarAutoSkUp, "%-1.1s", get_env ("AUTO_SK_UP"));

	init_scr ();
	set_tty (); 
	_set_masks ("so_sdmaint.s");

	X_EALL = 50;
	Y_EALL = 6;

	for (i = 0;i < 2;i++)
		tab_data [i]._desc = scn_desc [i];

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	swide ();
	clear ();

	strcpy (local_rec.prev_inv_no, "00000000");
	strcpy (local_rec.prev_dbt_no, "000000");

	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		lcount [2] 	= 0;
		init_vars (1);	
		init_vars (2);	

		heading (1);
		scn_display (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (_EXPORT)
		{
			edit_ok (2);
			X_EALL = 50;
			Y_EALL = 6;

			scn_write (1);
			scn_display (1);
			scn_display (2);

			if (newMark)
				entry (2);
			else
				edit (2);
		}
		else
		{
			X_EALL = 0;
			Y_EALL = 0;
			no_edit (2);
#ifdef	GVISION
			scn_hide (2);
#endif	
		}

		if (prog_exit || restart)
			continue;

	
		edit_all ();

		if (restart)
			continue;

		Update ();
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence . |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? 
						"cumr_id_no" : "cumr_id_no3");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (sosd, sosd_list, SOSD_NO_FIELDS, (!INVOICE) ? 
						"sosd_id_no" : "sosd_id_no2");
	open_rec (exaf, exaf_list, EXAF_NO_FIELDS, "exaf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*========================
| Close Database Files . |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (sosd);
	abc_fclose (exaf);
	abc_fclose (inmr);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	i;
	int	val_pterms;

	/*-------------------------
	| Validate Customer Number. |
	-------------------------*/
	if (LCHECK ("debtor"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			cumr_rec.hhcu_hash = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.cust_no));
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("name");

		return (EXIT_SUCCESS);
	}
		
	/*--------------------------
	| Validate Invoice Number. |
	--------------------------*/
	if (LCHECK ("invoice_no")) 
	{
		if (SRCH_KEY)
		{
			SrchCohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cohr_rec.co_no, comm_rec.co_no);
		strcpy (cohr_rec.br_no, comm_rec.est_no);
		cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
		strcpy (cohr_rec.type, typeFlag);
		strcpy (cohr_rec.inv_no, local_rec.inv_no);

		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (cc)
		{
			print_mess (ML (mlStdMess115));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (cohr_rec.stat_flag [0] != findFlag [0])
		{
			print_mess (ML (mlSoMess076));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (cohr_rec.hhcu_hash != cumr_rec.hhcu_hash)
		{
			print_mess (ML (mlSoMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		LoadDetails ();

		newMark = FALSE;

		if (LoadSosd (cohr_rec.hhco_hash))
			newMark = TRUE;
		
		entry_exit = 1;
		return (EXIT_SUCCESS);
	}
		
	/*------------------------
	| Validate Order Number. |
	------------------------*/
	if (LCHECK ("order_no")) 
	{
		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		strcpy (sohr_rec.order_no, local_rec.inv_no);
		sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess102));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		if (sohr_rec.status [0] == 'P' || sohr_rec.status [0] == 'S')
		{
			print_mess (ML (mlSoMess076));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		LoadDetails ();

		newMark = FALSE;

		if (LoadSosd (sohr_rec.hhso_hash))
			newMark = TRUE;
		
		LoadSosd (sohr_rec.hhso_hash);

		entry_exit = 1;
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Payment Terms. |
	-------------------------*/
	if (LCHECK ("pay_term"))
	{
		val_pterms = FALSE;

		if (SRCH_KEY)
		{
			SrchPay ();
			return (EXIT_SUCCESS);
		}

		for (i = 0;strlen (p_terms [i]._pcode);i++)
		{
			if (!strncmp (local_rec.pay_terms, p_terms [i]._pcode, strlen (p_terms [i]._pcode)))
			{
				sprintf (local_rec.pay_terms, "%-40.40s", p_terms [i]._pterm);
				val_pterms = TRUE;
				break;
			}
		}
		if (!val_pterms)
		{
			print_mess (ML (mlStdMess136));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("pay_term");

		return (EXIT_SUCCESS);
	}
	/*---------------------------------------
	| Validate Shipment Name And Addresses. |
	---------------------------------------*/
	if (!strcmp (FIELD.label, "shipname") || 
	     !strncmp (FIELD.label, "shipaddr", 8)) 
	{
		if (SRCH_KEY)
		{
			open_rec (cudi, cudi_list, CUDI_NO_FIELDS, "cudi_id_no");

			i = SrchCudi (field - label ("shipname"));

			abc_fclose (cudi);
			if (i < 0)
				return (EXIT_SUCCESS);

			strcpy (local_rec.del_name, cudi_rec.name);
			strcpy (local_rec.del_add [0], cudi_rec.adr1);
			strcpy (local_rec.del_add [1], cudi_rec.adr2);
			strcpy (local_rec.del_add [2], cudi_rec.adr3);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("ship_method") || LCHECK ("spcode1") || LCHECK ("spcode2"))
	{
		i = field - label ("ship_method") ;

		open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			abc_fclose (exsi);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no, comm_rec.co_no);
		if (dflt_used)
		{
			if (i == 0)
				exsi_rec.inst_code = cumr_rec.inst_fg1;
			else if (i == 1)
				exsi_rec.inst_code = cumr_rec.inst_fg2;
			else if (i == 2)
				exsi_rec.inst_code = cumr_rec.inst_fg3;
		}
		else
			exsi_rec.inst_code = atoi (local_rec.spinst [i]);

		if (!find_rec (exsi, &exsi_rec, COMPARISON, "r"))
			sprintf (local_rec.spinst [i], "%-60.60s", exsi_rec.inst_text);
		abc_fclose (exsi);

		DSP_FLD (vars [field].label);
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Carrier Code. |
	------------------------*/
	if (LCHECK ("carrierCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
			{
				if (INVOICE)
					cohr_rec.freight	=	0.00;
				else
					sohr_rec.freight = 0.00;
			}

			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg, 
				trzm_rec.chg_kg, 
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			trcm_rec.trcm_hash	=	0L;
			return (EXIT_SUCCESS);
		}
			
		OpenTransportFiles ("trzm_trzm_hash");

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			CloseTransportFiles ();
			return (EXIT_SUCCESS);
		}
			
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
		if (!cc)
		{
			trcl_rec.trcm_hash = trcm_rec.trcm_hash;
			trcl_rec.trzm_hash = trzm_rec.trzm_hash;
			cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML (mlStdMess134));
				sleep (sleepTime);
				clear_mess ();

				CloseTransportFiles ();
				trcm_rec.trcm_hash	=	0L;
				return (EXIT_FAILURE);
			}
			CloseTransportFiles ();

			DSP_FLD ("carr_desc");

			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg, 
				trzm_rec.chg_kg, 
				trzm_rec.dflt_chg
			);

			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			return (EXIT_SUCCESS);
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		clear_mess ();

		CloseTransportFiles ();
		return (EXIT_FAILURE);
	}

	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("deliveryZoneCode"))
	{
		trcm_rec.markup_pc	= 0.00;
		trcl_rec.cost_kg	= 0.00;

		if (dflt_used)
		{
			if (FREIGHT_CHG)
				sohr_rec.freight = 0.00;

			strcpy (trzm_rec.del_zone, "      ");
			strcpy (trzm_rec.desc, "      ");
			trzm_rec.trzm_hash	=	0L;
			trzm_rec.dflt_chg	=	0.0;
			trzm_rec.chg_kg		=	0.0;
			CalculateFreight 
			 (
				trcm_rec.markup_pc, 
				trcl_rec.cost_kg, 
				trzm_rec.chg_kg, 
				trzm_rec.dflt_chg
			);
			DSP_FLD ("est_freight");
			DSP_FLD ("freight");
			DSP_FLD ("tot_kg");
			DSP_FLD ("deliveryZoneDesc");
			DSP_FLD ("deliveryZoneCode");
			return (EXIT_SUCCESS);
		}
		OpenTransportFiles ("trzm_id_no");

		if (prog_status == ENTRY && NO_KEY (field))
		{
			strcpy (trzm_rec.del_zone, cumr_rec.del_zone);
			DSP_FLD ("deliveryZoneCode");
		}
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			CloseTransportFiles ();
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			CloseTransportFiles ();
			return (EXIT_FAILURE); 
		}
		if (trcm_rec.trcm_hash > 0L)
		{
			strcpy (trcm_rec.co_no, comm_rec.co_no);
			strcpy (trcm_rec.br_no, comm_rec.est_no);
			cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
			if (!cc)
			{
				trcl_rec.trcm_hash = trcm_rec.trcm_hash;
				trcl_rec.trzm_hash = trzm_rec.trzm_hash;
				cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess134));
					sleep (sleepTime);
					clear_mess ();

					CloseTransportFiles ();
					return (EXIT_FAILURE);
				}
				CloseTransportFiles ();

				CalculateFreight 
				 (
					trcm_rec.markup_pc, 
					trcl_rec.cost_kg, 
					trzm_rec.chg_kg, 
					trzm_rec.dflt_chg
				);

				DSP_FLD ("est_freight");
				DSP_FLD ("freight");
				DSP_FLD ("tot_kg");
				return (EXIT_SUCCESS);
			}
		}
		DSP_FLD ("deliveryZoneDesc");
		DSP_FLD ("deliveryZoneCode");

		CloseTransportFiles ();
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate delivery required. |
	-----------------------------*/
	if (LCHECK ("deliveryRequired"))
	{
		/*--------------------
		| Schedule required. |
		--------------------*/
		if (sohr_rec.del_req [0] == 'Y')
		{
			move (0, 2);cl_line ();
			i = prmptmsg (ML (mlTrMess063) , "YyNn", 0, 2);
			if (i == 'N' || i == 'n') 
				return (EXIT_SUCCESS);

			sprintf (err_str, "tr_trsh_mnt O %010ld LOCK", sohr_rec.hhso_hash);
			sys_exec (err_str);
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
			trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
			cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (sohr_rec.s_timeslot, trsh_rec.sdel_slot);
				strcpy (sohr_rec.e_timeslot, trsh_rec.edel_slot);
				sohr_rec.del_date	=	trsh_rec.del_date;
				cc = abc_update (sohr, &sohr_rec);
				if (cc)
					file_err (cc, (char *)sohr, "DBUPDATE");
			}
			abc_fclose (trsh);
			heading (1);
			scn_write (1);
			scn_display (1);
			print_mess (ML (mlTrMess076));
			sleep (sleepTime);
			DSP_FLD ("deliveryDate");
		}
		else
		{
			/*----------------------------------------------------------
			| Schedule not required and was not an origional delivery. |
			----------------------------------------------------------*/
			if (local_rec.origDelRequired [0] == 'N')
				return (EXIT_SUCCESS);

			/*-------------------------------------------------------
			| Schedule not required but was set to delivery before. |
			-------------------------------------------------------*/
			open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_hhso_hash");
			trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
			cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
			while (!cc && trsh_rec.hhso_hash == sohr_rec.hhso_hash)
			{
				abc_delete (trsh);
				trsh_rec.hhso_hash	=	sohr_rec.hhso_hash;
				cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
			}
			abc_fclose (trsh);
		}
		return (EXIT_SUCCESS); 
	}
	return (EXIT_SUCCESS);
}

/*===========================================
| Load details for Invoice /  Order Number. |
===========================================*/
int
LoadDetails (
 void)
{
	char	workCarrier [5];

	OpenTransportFiles ("trzm_id_no");
		
	strcpy (trzm_rec.co_no,comm_rec.co_no);
	strcpy (trzm_rec.br_no,comm_rec.est_no);
	strcpy (trzm_rec.del_zone,
				(INVOICE) ? cohr_rec.del_zone : sohr_rec.del_zone);

	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (trzm_rec.del_zone, "      ");
		trzm_rec.trzm_hash	=	0L;
		trzm_rec.dflt_chg	=	0.0;
		trzm_rec.chg_kg		=	0.0;
	}
	if (INVOICE)
		strcpy (cohr_rec.del_zone,  trzm_rec.del_zone);
	else
		strcpy (sohr_rec.del_zone,  trzm_rec.del_zone);

	strcpy (trcm_rec.carr_code, 
				(INVOICE) ? cohr_rec.carr_code : sohr_rec.carr_code);
	sprintf (trcm_rec.carr_desc,"%40.40s", " ");
	local_rec.est_freight 		= 	0.00;
	trcm_rec.trcm_hash	=	0L;

	strcpy (workCarrier, (INVOICE) ? cohr_rec.carr_code : sohr_rec.carr_code);
	if (strcmp (workCarrier, "    ") && trzm_rec.trzm_hash > 0L)
	{
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		strcpy (trcm_rec.carr_code, 
					(INVOICE) ? cohr_rec.carr_code : sohr_rec.carr_code);
		cc = find_rec (trcm, &trcm_rec,COMPARISON,"r");
		if (cc)
			file_err (cc, (char *)trcm, "DBFIND");
		
		trcl_rec.trcm_hash = trcm_rec.trcm_hash;
		trcl_rec.trzm_hash = trzm_rec.trzm_hash;
		cc = find_rec (trcl, &trcl_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, (char *)trcl, "DBFIND");

		CalculateFreight 
		(
			trcm_rec.markup_pc, 
			trcl_rec.cost_kg,
			trzm_rec.chg_kg,
			trzm_rec.dflt_chg
		);
	}
	CloseTransportFiles ();

	strcpy (local_rec.spinst [0], 	 
				(INVOICE) ? cohr_rec.din_1 : sohr_rec.din_1);

	strcpy (local_rec.spinst [1], 	 
				(INVOICE) ? cohr_rec.din_2 : sohr_rec.din_2);

	strcpy (local_rec.spinst [2], 	 
				(INVOICE) ? cohr_rec.din_3 : sohr_rec.din_3);

	strcpy (local_rec.ins_det, 	 	
				(INVOICE) ? cohr_rec.ins_det : sohr_rec.ins_det);

	strcpy (local_rec.del_name, 	 	
				(INVOICE) ? cohr_rec.dl_name : sohr_rec.del_name);

	strcpy (local_rec.del_add [0], 	
				(INVOICE) ? cohr_rec.dl_add1 : sohr_rec.del_add1);

	strcpy (local_rec.del_add [1], 	 
				(INVOICE) ? cohr_rec.dl_add2 : sohr_rec.del_add2);

	strcpy (local_rec.del_add [2], 	 
				(INVOICE) ? cohr_rec.dl_add3 : sohr_rec.del_add3);

	strcpy (local_rec.cons_no, 	 	
				(INVOICE) ? cohr_rec.cons_no : sohr_rec.cons_no);

	strcpy (local_rec.pay_terms, 	 
				(INVOICE) ? cohr_rec.pay_terms : sohr_rec.pay_term);

	strcpy (local_rec.del_req, 	 
				(INVOICE) ? cohr_rec.del_req : sohr_rec.del_req);

	local_rec.del_date = 		 
				(INVOICE) ? cohr_rec.del_date : sohr_rec.del_date;

	local_rec.no_kgs = 		 
				(INVOICE) ? cohr_rec.no_kgs : sohr_rec.no_kgs;

	local_rec.insurance = 		 
				(INVOICE) ? cohr_rec.insurance : sohr_rec.insurance;

	local_rec.deposit = 		 
				(INVOICE) ? cohr_rec.deposit : sohr_rec.deposit;

	local_rec.freight = 		 
				(INVOICE) ? cohr_rec.freight : sohr_rec.freight;

	local_rec.ex_disc = 		 
				(INVOICE) ? cohr_rec.ex_disc : sohr_rec.discount;

	local_rec.other_cost [0] = 	 
				(INVOICE) ? cohr_rec.other_cost_1 : sohr_rec.other_cost_1;

	local_rec.other_cost [1] = 	 
				(INVOICE) ? cohr_rec.other_cost_2 : sohr_rec.other_cost_2;

	local_rec.other_cost [2] = 	 
				(INVOICE) ? cohr_rec.other_cost_3 : sohr_rec.other_cost_3;

	local_rec.no_cartons = 		 
				(INVOICE) ? cohr_rec.no_cartons : sohr_rec.no_cartons;

    return (EXIT_SUCCESS);
}

/*===========================================
| Save details for Invoice /  Order Number. |
===========================================*/
void
SaveDetails (
 void)
{
	if (INVOICE)
	{
		strcpy (cohr_rec.din_1, local_rec.spinst [0]);
		strcpy (cohr_rec.din_2, local_rec.spinst [1]);
		strcpy (cohr_rec.din_3, local_rec.spinst [2]);
		strcpy (cohr_rec.dl_name, local_rec.del_name);
		strcpy (cohr_rec.dl_add1, local_rec.del_add [0]);
		strcpy (cohr_rec.dl_add2, local_rec.del_add [1]);
		strcpy (cohr_rec.dl_add3, local_rec.del_add [2]);
		strcpy (cohr_rec.cons_no, local_rec.cons_no);
		strcpy (cohr_rec.pay_terms, local_rec.pay_terms);
		strcpy (cohr_rec.carr_code, trcm_rec.carr_code);
		strcpy (cohr_rec.del_zone, trzm_rec.del_zone);
		strcpy (cohr_rec.del_req, local_rec.del_req);
		cohr_rec.del_date		= local_rec.del_date;
		cohr_rec.no_kgs 		= local_rec.no_kgs;
		cohr_rec.insurance 		= local_rec.insurance;
		cohr_rec.deposit 		= local_rec.deposit;
		cohr_rec.freight 		= local_rec.freight;
		cohr_rec.ex_disc 		= local_rec.ex_disc;
		cohr_rec.other_cost_1 	= local_rec.other_cost [0];
		cohr_rec.other_cost_2 	= local_rec.other_cost [1];
		cohr_rec.other_cost_3 	= local_rec.other_cost [2];
		cohr_rec.no_cartons		= local_rec.no_cartons;

		CalculateTax ();

		strcpy (cohr_rec.stat_flag, createFlag);

		cc = abc_update (cohr, &cohr_rec);
		if (cc) 
			file_err (cc, "cohr", "DBUPDATE");
	}
	else
	{
		strcpy (sohr_rec.din_1, local_rec.spinst [0]);
		strcpy (sohr_rec.din_2, local_rec.spinst [1]);
		strcpy (sohr_rec.din_3, local_rec.spinst [2]);
		strcpy (sohr_rec.del_name, local_rec.del_name);
		strcpy (sohr_rec.del_add1, local_rec.del_add [0]);
		strcpy (sohr_rec.del_add2, local_rec.del_add [1]);
		strcpy (sohr_rec.del_add3, local_rec.del_add [2]);
		strcpy (sohr_rec.cons_no, local_rec.cons_no);
		strcpy (sohr_rec.pay_term, local_rec.pay_terms);
		strcpy (sohr_rec.carr_code, trcm_rec.carr_code);
		strcpy (sohr_rec.del_zone, trzm_rec.del_zone);
		strcpy (sohr_rec.del_req, local_rec.del_req);

		sohr_rec.del_date		= local_rec.del_date;
		sohr_rec.no_kgs 		= local_rec.no_kgs;
		sohr_rec.no_cartons		= local_rec.no_cartons;
		sohr_rec.insurance 		= local_rec.insurance;
		sohr_rec.deposit 		= local_rec.deposit;
		sohr_rec.freight 		= local_rec.freight;
		sohr_rec.discount 		= local_rec.ex_disc;
		sohr_rec.other_cost_1 	= local_rec.other_cost [0];
		sohr_rec.other_cost_2 	= local_rec.other_cost [1];
		sohr_rec.other_cost_3 	= local_rec.other_cost [2];

		cc = abc_update (sohr, &sohr_rec);
		if (cc) 
			file_err (cc, "sohr", "DBUPDATE");
	}
}

/*============================
| Calculate Tax for Invoice. |
============================*/
void
CalculateTax (
 void)
{
	double	value  = 0.00, 
		wk_value = 0.00; 

	if (cumr_rec.tax_code [0] == 'A' || 
	     cumr_rec.tax_code [0] == 'B')
		noTax = 1;
	else
		noTax = 0;

	cohr_rec.gst = 0.00;

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = 0;
	cc = find_rec (coln, &coln_rec, GTEQ, "u");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		cohr_rec.gst += coln_rec.amt_gst;

		strcpy (coln_rec.stat_flag, createFlag);

		cc = abc_update (coln, &coln_rec);
		if (cc) 
			file_err (cc, "coln", "DBUPDATE");

		cc = find_rec (coln, &coln_rec, NEXT, "u");
	}
	abc_unlock (coln);

	if (noTax)
		wk_value = 0.00;
	else
		wk_value = (double) (comm_rec.gst_rate / 100.00);

	value = cohr_rec.freight + 
		cohr_rec.sos + 
		cohr_rec.insurance -
		cohr_rec.ex_disc + 
		cohr_rec.other_cost_1 +
		cohr_rec.other_cost_2 + 
		cohr_rec.other_cost_3;

	wk_value *= value;
	cohr_rec.gst += wk_value;

	cohr_rec.gst = no_dec (cohr_rec.gst);
}

/*==================================
| Load shipping delails from sosd. |
==================================*/
int
LoadSosd (
 long hash)
{
	scn_set (2);
	lcount [2] = 0;

	sosd_rec.hhco_hash = hash;
	sosd_rec.hhso_hash = hash;
	sosd_rec.line_no = 0;
	cc = find_rec (sosd, &sosd_rec, GTEQ, "r");

	while (!cc && ((INVOICE && sosd_rec.hhco_hash == hash) || 
		      (!INVOICE && sosd_rec.hhso_hash == hash)))
	{
		putval (lcount [2]++);

		cc = find_rec (sosd, &sosd_rec, NEXT, "r");
	}
	scn_set (1);
	if (!lcount [2])
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*=======================================================
| Update invoice/order and shiping details if required. |
=======================================================*/
void
Update (
 void)
{
	clear ();
	
	print_at (0, 0, ML (mlSoMess202), INV_STR);
	fflush (stdout);
	
	SaveDetails ();
	
	print_at (1, 0, ML (mlSoMess203));
	fflush (stdout);

	scn_set (2);
	
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		sosd_rec.hhco_hash = (INVOICE) ? cohr_rec.hhco_hash
						    : sohr_rec.hhso_hash;

		sosd_rec.line_no = line_cnt;
		cc = find_rec (sosd, &sosd_rec, COMPARISON, "u");
		getval (line_cnt);
		if (cc)
		{
			putchar ('A');
			if (INVOICE)
			{
				sosd_rec.hhco_hash = cohr_rec.hhco_hash;
				sosd_rec.hhso_hash = cohr_rec.hhco_hash;
			}
			else
			{
				sosd_rec.hhso_hash = sohr_rec.hhso_hash;
				sosd_rec.hhco_hash = sohr_rec.hhso_hash;
			}
			sosd_rec.line_no = line_cnt;
			cc = abc_add (sosd, &sosd_rec);
			if (cc) 
				file_err (cc, "sosd", "DBADD");
		}
		else
		{
			if (INVOICE)
				sosd_rec.hhco_hash = cohr_rec.hhco_hash;
			else
				sosd_rec.hhso_hash = sohr_rec.hhso_hash;

			putchar ('U');
			cc = abc_update (sosd, &sosd_rec);
			if (cc) 
				file_err (cc, "sosd", "DBUPDATE");
		}
		fflush (stdout);
	}

	/*-----------------------------------------------
	| Delete All Extra Lines			|
	| if no line items then delete all		|
	-----------------------------------------------*/
	if (INVOICE)
	{
		sosd_rec.hhco_hash = cohr_rec.hhco_hash;
		sosd_rec.hhso_hash = cohr_rec.hhco_hash;
	}
	else
	{
		sosd_rec.hhso_hash = sohr_rec.hhso_hash;
		sosd_rec.hhco_hash = sohr_rec.hhso_hash;
	}
	sosd_rec.line_no = lcount [2];
	cc = find_rec (sosd, &sosd_rec, GTEQ, "u");

	while (!cc && ((INVOICE && 
		sosd_rec.hhco_hash == cohr_rec.hhco_hash) || 
		 (!INVOICE && sosd_rec.hhso_hash == sohr_rec.hhso_hash)))
	{
		putchar ('D');
		fflush (stdout);
		cc = abc_delete (sosd);
		if (cc) 
			file_err (cc, "sosd", "DBDELETE");

		if (INVOICE)
		{
			sosd_rec.hhco_hash = cohr_rec.hhco_hash;
			sosd_rec.hhso_hash = cohr_rec.hhco_hash;
		}
		else
		{
			sosd_rec.hhso_hash = sohr_rec.hhso_hash;
			sosd_rec.hhco_hash = sohr_rec.hhso_hash;
		}
		sosd_rec.line_no = lcount [2];
		cc = find_rec (sosd, &sosd_rec, GTEQ, "u");
	}
	abc_unlock (sosd);

	if (AUTO_SK_UP && INVOICE)
	{
		add_hash (comm_rec.co_no, 
				  comm_rec.est_no, 
				  "SU", 
				  0, 
				  cohr_rec.hhco_hash, 
				  0L, 
				  0L, 
				  (double) 0.00);
	}

	strcpy (local_rec.prev_dbt_no, local_rec.cust_no);
	strcpy (local_rec.prev_inv_no, local_rec.inv_no);

	recalc_sobg ();
}

/*============================
| Search for Invoice number. |
============================*/
void
SrchCohr (
 char *key_val)
{
	work_open ();
	save_rec ("#Inv No", "#Cust Order");
	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, typeFlag);
	sprintf (cohr_rec.inv_no, "%-8.8s", key_val);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");

	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (cohr_rec.br_no, comm_rec.est_no) && 
		      !strncmp (cohr_rec.inv_no, key_val, strlen (key_val)) &&
		       cumr_rec.hhcu_hash == cohr_rec.hhcu_hash)
	{
		if (cohr_rec.stat_flag [0] == findFlag [0] &&
		     cohr_rec.type [0] == typeFlag [0])
		{
			cc = save_rec (cohr_rec.inv_no, 
				       cohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		
		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.type, typeFlag);
	sprintf (cohr_rec.inv_no, "%-8.8s", temp_str);
	cohr_rec.hhcu_hash = cumr_rec.hhcu_hash;

	cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "cohr", "DBFIND");
}

/*==========================
| Search for order number. |
==========================*/
void
SrchSohr  (
 char *key_val)
{
	work_open ();
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (sohr_rec.order_no, "%-8.8s", key_val);

	save_rec ("#Sales Order No.", "#Customer Order Number.");
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && !strncmp (sohr_rec.order_no, key_val, strlen (key_val)) && 
		      !strcmp (sohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no, comm_rec.est_no) && 
		      sohr_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		if
		 (
			sohr_rec.status [0] != 'P' &&
			sohr_rec.status [0] != 'S'
		)
		{
			cc = save_rec (sohr_rec.order_no, 
				       sohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sohr_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "sohr", "DBFIND");
}

/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char	*key_val)
{
	char	wk_code [4];

	work_open ();
	save_rec ("#Spec Inst", "#Instruction description.");

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);

	cc = find_rec (exsi, &exsi_rec, GTEQ, "r");
	while (!cc && !strcmp (exsi_rec.co_no, comm_rec.co_no))
	{
		sprintf (wk_code, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wk_code, err_str);
		if (cc)
			break;

		cc = find_rec (exsi, &exsi_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no, comm_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi, &exsi_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "exsi", "DBFIND");
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
		file_err (cc, (char *)trzm, "DBFIND");

	return;
}
/*==========================
| Search for carrier code. |
==========================*/
void
SrchTrcm (
 char	*key_val)
{
	char	key_string [31];
	long 	currentZoneHash	=	trzm_rec.trzm_hash;

	_work_open (20, 11, 50);

	save_rec ("#Carrier", "# Rate Kg. | Carrier Name.");
	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) && 
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) && 
		      	  !strncmp (trcm_rec.carr_code, key_val, strlen (key_val)))
	{
		trcl_rec.trcm_hash	=	trcm_rec.trcm_hash;
		trcl_rec.trzm_hash	=	0L;
		cc = find_rec (trcl, &trcl_rec, GTEQ, "r");
		while (!cc && trcl_rec.trcm_hash == trcm_rec.trcm_hash)
		{
	
			if (currentZoneHash	> 0L && trcl_rec.trzm_hash != currentZoneHash)
			{
				cc = find_rec (trcl, &trcl_rec, NEXT, "r");
				continue;
			}
			trzm_rec.trzm_hash	=	trcl_rec.trzm_hash;
			cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (err_str, " %8.2f | %-40.40s", 
							trcl_rec.cost_kg, 
							trzm_rec.desc);

				sprintf (key_string, "%-4.4s-%-6.6s %010ld", 
							trcm_rec.carr_code, 
							trzm_rec.del_zone, 
							trzm_rec.trzm_hash);

				cc = save_rec (key_string, err_str);
				if (cc)
					break;
			}
			cc = find_rec (trcl, &trcl_rec, NEXT, "r");
		}
		cc = find_rec (trcm, &trcm_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code, "%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)trcm, "DBFIND");

	trzm_rec.trzm_hash	=	atol (temp_str + 12);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (!cc)
		sprintf (local_rec.defaultDelZone, "%-6.6s", trzm_rec.del_zone);
}

/*===========================
| Search for Payment Terms. |
===========================*/
void
SrchPay (
 void)
{
	int	i = 0;

	work_open ();
	save_rec ("#Cde", "#Payment Terms ");

	for (i = 0;strlen (p_terms [i]._pcode);i++)
	{
		cc = save_rec (p_terms [i]._pcode, p_terms [i]._pterm);
		if (cc)
			break;
	}
	cc = disp_srch ();
	work_close ();
}

/*==============================
| Calculate Defaults for Levy. |
==============================*/
void
CalculateFreight (
	float	freightMarkup, 
	double	carrierCostKg, 
	double	zoneCostKg, 
	double	zoneFixedAmount)
{
	float	totalKgs		= 0.0;
	double	freightValue	= 0.00;
	double	calcMarkup		= 0.00;

	if (INVOICE)
		cohr_rec.no_kgs		= 0.00;
	else
		sohr_rec.no_kgs		= 0.00;

	local_rec.est_freight 		= 0.00;

 	if (freightMarkup == 0.00 && carrierCostKg == 0.00 && 
		 zoneCostKg == 0.00 && zoneFixedAmount == 0.00)
	{
		return;
	}

	if (INVOICE)
		totalKgs	=	GetInvoiceWeight ();
	else
		totalKgs	=	GetOrderWeight ();

	/*-------------------------------
	| Cost by Kg by Carrier / Zone. |
	-------------------------------*/
	if (envSoFreightCharge == 3)
		freightValue = (double) totalKgs * carrierCostKg;

	/*---------------------
	| Cost by Kg by Zone. |
	---------------------*/
	if (envSoFreightCharge == 2)
		freightValue = (double) totalKgs * zoneCostKg;

	calcMarkup = (double) freightMarkup;
	calcMarkup *= freightValue;
	calcMarkup = DOLLARS (calcMarkup);
	
	freightValue += calcMarkup;
	freightValue = twodec (CENTS (freightValue));

	if (freightValue < comr_rec.frt_min_amt && freightValue > 0.00)
		local_rec.est_freight = comr_rec.frt_min_amt;
	else
		local_rec.est_freight = freightValue;
	
	if (FREIGHT_CHG && envAutomaticFreight)
	{
		if (envSoFreightCharge == 1)
		{
			if (INVOICE)
				cohr_rec.freight = CENTS (zoneFixedAmount);
			else
				sohr_rec.freight = CENTS (zoneFixedAmount);
			local_rec.est_freight 	 = CENTS (zoneFixedAmount);
		}
		else
			sohr_rec.freight = local_rec.est_freight;
	}
	if (INVOICE)
		cohr_rec.no_kgs = totalKgs;
	else
		sohr_rec.no_kgs = totalKgs;
		
	return;
}

float
GetInvoiceWeight (void)
{
	float	weight		= 0.00;
	float	totalKgs 	= 0.00;

	coln_rec.hhco_hash	=	cohr_rec.hhco_hash;
	coln_rec.line_no	=	0;
	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		inmr_rec.hhbr_hash	=	coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}
		weight = (inmr_rec.weight > 0.00) 	? inmr_rec.weight
										 	: comr_rec.frt_mweight;
		totalKgs += (weight * coln_rec.q_order);

		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
	return (totalKgs);
}
float
GetOrderWeight (void)
{
	float	weight		= 0.00;
	float	totalKgs 	= 0.00;

	soln_rec.hhso_hash	=	sohr_rec.hhso_hash;
	soln_rec.line_no	=	0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
		weight = (inmr_rec.weight > 0.00) 	? inmr_rec.weight
										 	: comr_rec.frt_mweight;
		totalKgs += (weight * soln_rec.qty_order);

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
	return (totalKgs);
}

/*===============================
| Open Transport related files. |
===============================*/
void
OpenTransportFiles (
 char	*ZoneIndex)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, ZoneIndex);
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
}
/*================================
| Close Transport related files. |
================================*/
void
CloseTransportFiles (
 void)
{
	abc_fclose (trzm);
	abc_fclose (trcm);
	abc_fclose (trcl);
}
void
PrintCompanyStuff (
 void)
{
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_short);
	print_at (22, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 90, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);
}

int
SrchCudi (
	int		indx)
{
	char	workString [170];

	_work_open (5,0,80);
	save_rec ("#DelNo","#Delivery Details");
	cudi_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= 0;
	cc = find_rec (cudi, &cudi_rec, GTEQ, "r");
	while (!cc && cudi_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{                        
		sprintf 
		(
			workString,"%s, %s, %s, %s",
			clip (cudi_rec.name),
			clip (cudi_rec.adr1),
			clip (cudi_rec.adr2),
			clip (cudi_rec.adr3)
		);
		sprintf (err_str, "%05d", cudi_rec.del_no);
		cc = save_rec (err_str, workString); 
		if (cc)
			break;

		cc = find_rec (cudi, &cudi_rec, NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (-1);

	cudi_rec.hhcu_hash 	= cumr_rec.hhcu_hash;
	cudi_rec.del_no 	= atoi (temp_str);
	cc = find_rec (cudi,&cudi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, cudi, "DBFIND");

	switch (indx)
	{
	case	0:
		sprintf (temp_str,"%-40.40s",cudi_rec.name);
		break;

	case	1:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr1);
		break;

	case	2:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr2);
		break;

	case	3:
		sprintf (temp_str,"%-40.40s",cudi_rec.adr3);
		break;

	default:
		break;
	}
	return (cudi_rec.del_no);
}
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		if (scn == 1)
			clear ();

		pr_box_lines (scn);

		if (INVOICE)
			rv_pr (ML (mlSoMess200), 46, 0, 1);
		else
			rv_pr (ML (mlSoMess201), 46, 0, 1);

		print_at (0, 90, ML (mlSoMess174), INV_STR, local_rec.prev_dbt_no, local_rec.prev_inv_no);


		PrintCompanyStuff ();
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
