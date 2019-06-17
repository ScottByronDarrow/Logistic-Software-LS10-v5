/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_serdisp.c,v 5.6 2002/12/01 04:48:18 scott Exp $
|  Program Name  : (sk_serdisp.c  )                                   |
|  Program Desc  : (600 Machinery Serial Item Info Display      )     |
|---------------------------------------------------------------------|
|  Author        : Jim Bougher     | Date Written  : 09/08/90         |
|---------------------------------------------------------------------|
| $Log: sk_serdisp.c,v $
| Revision 5.6  2002/12/01 04:48:18  scott
| SC0053 - Platinum Logistics LS10.5.2.2002-12-01
|
| Revision 5.5  2002/07/17 09:57:59  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/01/31 02:30:18  cha
| Updated to fix S/C 725.
| Made the screen wide, to fit the description.
| Also updated to ignore non-serial items.
| Updated to make sure that serial records are found in Oracle.
|
| Revision 5.3  2001/10/17 09:08:31  cha
| Updated as getchar left in program.
| Changes made by Scott.
|
| Revision 5.2  2001/08/09 09:19:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:50  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:39  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.3  2001/05/17 04:19:42  robert
| updated to correct label for Previous Item menu
|
| Revision 4.2  2001/05/08 07:06:24  robert
| Updated for the modified LS10-GUI RingMenu
|
| Revision 4.1  2001/03/22 06:14:18  scott
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.0  2001/03/09 02:38:54  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:33  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:21:16  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:56  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  2000/04/12 04:57:35  ramon
| Added sleep () calls after displaying the error message.
|
| Revision 1.11  1999/12/01 16:24:28  alvin
| Fixed code that causes access error on GVision.
|
| Revision 1.10  1999/11/25 09:41:03  scott
| Updated as _work_open commented out for ?????? reason
|
| Revision 1.9  1999/11/03 07:32:33  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.8  1999/10/08 05:32:54  scott
| First Pass checkin by Scott.
|
| Revision 1.7  1999/06/20 05:20:43  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_serdisp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_serdisp/sk_serdisp.c,v 5.6 2002/12/01 04:48:18 scott Exp $";

#define	X_OFF	lpXoff
#define	Y_OFF	lpYoff

#include <pslscr.h>		/*  Gen. C Screen Handler Header          */
#include <ring_menu.h>
#include <get_lpno.h>
#include <pr_format3.h>
#include <ml_std_mess.h>


	int		lpXoff,
			lpYoff;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct insfRecord	insf_rec;
struct insfRecord	insf2_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct poshRecord	posh_rec;
struct posdRecord	posd_rec;
struct mhdrRecord	mhdr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inprRecord	inpr_rec;
struct sumrRecord	sumr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct cumrRecord	cumr_rec;

	char 	*insf2	=	"insf2",
			*inmr2	=	"inmr2",
			*incc2	=	"incc2";

	int		printerNumber = 1;
	int		printerOpened	=	FALSE;

	FILE	*fout;
	FILE	*fin;

	char	head_text [2] [180];

	long	curr_hhcc_hash = 0L;
	char	*blank_ser = "                         ";
	char	*GetPriceDesc 		(int);

struct	{
	char	*c_type;
	char	*c_desc;
} cost_desc [] = {
	{"L",	"Last   "},
	{"A",	"Average"},
	{"P",	"Prev.  "},
	{"T",	"Std.   "},
	{"F",	"FIFO   "},
	{"I",	"LIFO   "},
	{"S",	"Serial "},
	{"",	"Unknown"},
};

char	*std_foot = " [NEXT SCN] [PREV SCN] [EDIT/END]";

/*=======================
| Callback Declarations |
=======================*/
int 	SerialDisplay 	(void);
int 	CostDisplay 	(void);
int 	PrintOut 		(void);
int 	ClearReDraw		(void);
int 	NextDisplay 	(void);
int 	PrevDisplay 	(void);

int		main_open	=	FALSE;


struct	store_rec {
	char	OrderNo [9];
	char	InvoiceNo [9];
	char	CustomerNo [7];
	char	CustomerName [41];
	char	Supplier [7];
	char	PurchaseOrderNo [sizeof pohr_rec.pur_ord_no];
	char	POrdDate [11];
	char	PDueDate [11];
	double	SalePrice;
	long	DateInvoiced;
	long	DateRaised;
	char	SupplierInv [16];
	char	Destination [21];
	char	Vessel [21];
	char	ETA [11];
	float	ExchRate;
	char	ManSerialNo [21];
	float	sf_pd_exch;
	double	sf_freight;
	double	sf_cif;
	double	sf_fob_loc;
	double	land_cost;
	double	total_cst;
	char	CostingComplete [4];

} SER_rec;

#ifndef GVISION
menu_type	_main_menu [] = {
{ "Other Stock Nos ", "Other Numbers This Item", SerialDisplay, "Ss",	},
{ "Costing ", "Costing this Stock Number", CostDisplay, "Cc",    	},
{ "Print ", "Print Out This Information", PrintOut, "Pp", 	   	},
{ "<FN03>", "Redraw Display", ClearReDraw, "", FN3,			},
{ "<FN14>", "Display Next Item", NextDisplay, "", FN14,		},
{ "<FN15>", "Display Previous Item", PrevDisplay, "", FN15,		},
{ "<FN16>", "Exit Display", _no_option, "", FN16, EXIT | SELECT		},
{ "",									},
};
#else
menu_type	_main_menu [] = {
{0, "Other Stock Nos ", "Other Numbers This Item", SerialDisplay,	},
{0, "Costing ", "Costing this Stock Number", CostDisplay, },
{0, "Print ", "Print Out This Information", PrintOut, },
{0, "<FN03>", "Redraw Display", ClearReDraw, FN3,			},
{0, "<FN14>", "Display Next Item", NextDisplay, FN14,		},
{0, "<FN15>", "Display Previous Item", PrevDisplay, FN15,		},
{0, "",									},
};
#endif

	int		wh_flag = TRUE;
	int		display_ok = TRUE;
	int		clear_ok = TRUE;
	char	disp_str [100];
	char	dbt_no [7];
	float	tot_qty;
	double	tot_amount;
	long	hhwh_hash;
	double	itemPrice [9];
	char	Curr_code [4];

	extern	int		TruePosition;

	int	numPrices;

struct {
	char 	dummy [11];
	char	serial_no [26];
	char	item_no [17];
	char 	prev_item [17];
	char	cost_type [15];
	char	ex_desc [50];
	double	l_cost;
	double	cost_price;
	float	available;
	float	mtd_sales;
	float	ytd_sales;
	long	hhwh_hash;
} local_rec;

static struct	var vars [] ={
	{1, LIN, "ser_no", 	2, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Serial Number   ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.serial_no},

	{1, LIN, "itemno", 	3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Item Number     ", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.item_no},

	{1, LIN, "desc", 	3, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO, JUSTLEFT, "", "", inmr_rec.description},

    {0, LIN, "",  0, 0, INTTYPE,
        "A", "          ",
        " ", "", "dummy", " ",
        YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		hash_passed	=	FALSE;

#include <ser_value.h>
#include <FindInmr.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
int  	spec_valid 				(int);
void 	ChangeData 				(int);
void 	ReDraw 					(void);
void 	AllDisplay 				(void);
void 	GetPrices 				(void);
void 	SaveSerialItem 			(char *, char *);
void 	ProcessItem 			(void);
int  	heading 				(int);
void 	SrchInsf 				(char *);
int  	FindItem 				(long);
int  	SerialFindOrder 		(long, char *, int);
int  	FindShipment 			(long, char *);
int  	FindMhdr 				(char *);
void 	FindSupplier 			(long);
int  	check_page 				(void);
int  	Dsp_heading 			(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	input_row = 20,
	error_line = 20;

	TruePosition	=	TRUE;
	/*--------------------------
	| Get local currency code. |
	--------------------------*/
	sprintf (Curr_code, "%-3.3s", get_env ("CURR_CODE"));

	/*----------------------------
	| Get number of price types. |
	----------------------------*/
	sptr = chk_env ("SK_DBPRINUM");
	numPrices = (sptr == (char *)0) ? 9 : atoi (sptr);

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);
	swide();

	OpenDB ();

	sprintf (local_rec.prev_item,"%16s"," ");

	if (argc == 4)
		hash_passed	=	TRUE;

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		display_ok 	= FALSE;
		clear_ok 	= TRUE;
		search_ok 	= TRUE;
		init_vars (1);

		heading (1);
		crsr_on ();
		if (!hash_passed)
			entry (1);
		else
		{
			insf_rec.hhwh_hash	=	atol (argv [1]);
			sprintf (insf_rec.serial_no, "%-25.25s", argv [2]);
			sprintf (local_rec.serial_no, "%-25.25s", argv [2]);
			sprintf (insf_rec.status, "%-1.1s",		 argv [3]);
			cc = find_rec (insf2, &insf_rec, COMPARISON, "r");
			if (!cc)
				cc = FindItem (insf_rec.hhwh_hash);
			if (!cc)
			{
				curr_hhcc_hash = ccmr_rec.hhcc_hash;
				strcpy (local_rec.item_no,inmr_rec.item_no);
			}
			else
				entry (1);

			hash_passed	=	FALSE;
		}

		if (prog_exit || restart)
			continue;

		FLD ("ser_no")	=	ND;
		FLD ("itemno")	=	ND;
		FLD ("desc")	=	ND;
		display_ok 		= 	TRUE;
		clear_ok 		= 	FALSE;



		heading (1);
		if (inmr_rec.serial_item[0] == 'Y')
#ifndef GVISION
			run_menu (_main_menu,"",input_row);
#else
        	run_menu (NULL, _main_menu);
#endif
		FLD ("ser_no")	=	YES;
		FLD ("itemno")	=	YES;
		FLD ("desc")	=	NA;

		strcpy (local_rec.prev_item,inmr_rec.item_no);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===============================
| Program exit sequence.		|
===============================*/
void
shutdown_prog (
 void)
{
	CloseDB ();
	FinishProgram ();
}

/*===============================
| Open data base files.			|
===============================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2,inmr);
	abc_alias (incc2,incc);
	abc_alias (insf2,insf);

	open_rec (insf,		insf_list,	INSF_NO_FIELDS,	"insf_id_no3");
	open_rec (insf2,	insf_list,	INSF_NO_FIELDS,	"insf_id_no");
	open_rec (incc, 	incc_list, 	INCC_NO_FIELDS,	"incc_hhwh_hash");
	open_rec (incc2, 	incc_list, 	INCC_NO_FIELDS,	"incc_hhbr_hash");
	open_rec (inmr,		inmr_list,	INMR_NO_FIELDS,	"inmr_hhbr_hash");
	open_rec (inmr2,	inmr_list,	INMR_NO_FIELDS,	"inmr_id_no");
	open_rec (inpr,		inpr_list,	INPR_NO_FIELDS,	"inpr_id_no");
	open_rec (ccmr, 	ccmr_list, 	CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cumr,		cumr_list,	CUMR_NO_FIELDS,	"cumr_hhcu_hash");
	open_rec (cohr, 	cohr_list, 	COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (coln, 	coln_list, 	COLN_NO_FIELDS, "coln_hhbr_hash");
	open_rec (pohr,		pohr_list,	POHR_NO_FIELDS,	"pohr_hhpo_hash");
	open_rec (poln,		poln_list,	POLN_NO_FIELDS,	"poln_hhbr_hash");
	open_rec (posh,		posh_list,	POSH_NO_FIELDS,	"posh_id_no");
	open_rec (posd,		posd_list,	POSD_NO_FIELDS,	"posd_hhpo_hash");
	open_rec (sohr, 	sohr_list, 	SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln, 	soln_list, 	SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (mhdr, 	mhdr_list, 	MHDR_NO_FIELDS, "mhdr_serial_no");
	open_rec (sumr,		sumr_list,	SUMR_NO_FIELDS,	"sumr_hhsu_hash");
}

/*===============================
| Close data base files.		|
===============================*/
void
CloseDB (
 void)
{
	if (main_open)
		Dsp_close ();

	abc_fclose (insf);
	abc_fclose (insf2);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inpr);
	abc_fclose (ccmr);
	abc_fclose (cumr);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (posh);
	abc_fclose (posd);
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (mhdr);
	abc_fclose (sumr);

	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	int serno = FALSE;
	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("ser_no"))
	{
		FLD ("itemno")	=	NA;
		serno = TRUE;
		if (dflt_used)
		{
			DSP_FLD ("ser_no");
			FLD ("itemno")	=	YES;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchInsf (temp_str);
			sprintf(inmr_rec.description,"%40s"," ");
				DSP_FLD ("desc");
			return (EXIT_SUCCESS);
		}

		sprintf (insf_rec.serial_no,"%-25.25s",local_rec.serial_no);
		/*	insf_rec.hhwh_hash = 0;*/
		insf_rec.hhwh_hash = hhwh_hash;
		strcpy (insf_rec.status," ");
		cc = find_rec (insf, &insf_rec, GTEQ, "r");
		if (cc || strncmp (insf_rec.serial_no,local_rec.serial_no,25))
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		cc = FindItem (insf_rec.hhwh_hash);
		switch (cc)
		{
		case	1:
			sprintf (err_str, ML ("No Warehouse record for Serial No %s."),
														local_rec.serial_no);
			break;
		case	2:
			sprintf (err_str, ML ("Serial No %s does not belong to current company."),
														local_rec.serial_no);
			break;
		case	3:
			sprintf (err_str, ML ("Serial No %s is not on file for item."),local_rec.serial_no);
			break;
		case	0:
			break;
		}
		serno = TRUE;
		curr_hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (local_rec.item_no,inmr_rec.item_no);
		strcpy (local_rec.serial_no,insf_rec.serial_no);
		local_rec.hhwh_hash = insf_rec.hhwh_hash;

		DSP_FLD ("ser_no");
		sprintf(inmr_rec.description,"%40s"," ");
				DSP_FLD ("desc");
		return (EXIT_SUCCESS);

	}
	/*-------------------------------
	| Validate item number.			|
	-------------------------------*/
	if (LCHECK ("itemno"))
	{

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		/*-----------------------
		| find item				|
		-----------------------*/
		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L,"N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no,comm_rec.co_no);
			strcpy (inmr_rec.item_no,local_rec.item_no);
			cc = find_rec ("inmr2",&inmr_rec,COMPARISON,"r");
		}


		/*---------------------------
		| find failed				|
		---------------------------*/
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (serno)
		{
			sprintf(inmr_rec.description,"%40s"," ");
			DSP_FLD ("desc");
		}

		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec ("incc2", &incc_rec, COMPARISON, "r");
		serno = FALSE;
		if (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{

			insf_rec.hhwh_hash = incc_rec.hhwh_hash;
			strcpy (insf_rec.status," ");
			sprintf (insf_rec.serial_no,"%25s"," ");
			cc = find_rec ("insf2", &insf_rec, GTEQ, "r");

			if (cc && insf_rec.hhwh_hash != incc_rec.hhwh_hash)
			{

				print_mess (ML (mlStdMess201));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

			return (EXIT_SUCCESS);

		}
		else
		{

			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*===========================
| display next item			|
===========================*/
int
NextDisplay (
 void)
{
	ChangeData (NEXT);
	ReDraw ();
    return (EXIT_SUCCESS);
}

int
ClearReDraw (
 void)
{
	clear ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

/*===============================
| display prevoius item			|
===============================*/
int
PrevDisplay (
 void)
{
	ChangeData (PREVIOUS);
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
ChangeData (
 int key)
{
	char	last_serial [26];

	/*---------------------------------------
	| perform find in appropriate direction	|
	---------------------------------------*/
	strcpy (last_serial,insf_rec.serial_no);

	cc = find_rec (insf,&insf_rec, (key == NEXT) ? NEXT : PREVIOUS,"r");
	while (!cc && FindItem (insf_rec.hhwh_hash))
	{
		cc = find_rec (insf,&insf_rec, (key == NEXT) ? NEXT : PREVIOUS,"r");
	}

	if (cc)
	{
		print_mess (ML (mlStdMess245));
		sprintf (insf_rec.serial_no,last_serial);
		insf_rec.hhwh_hash = hhwh_hash;
		strcpy (insf_rec.status," ");
		cc = find_rec (insf, &insf_rec, GTEQ, "r");
		strcpy (local_rec.serial_no,insf_rec.serial_no);

	}
	strcpy (local_rec.serial_no,insf_rec.serial_no);
	strcpy (local_rec.item_no,inmr_rec.item_no);
}

void
ReDraw (
 void)
{
	clear_ok = FALSE;
	heading (1);
}

/*-----------------------
| Display whole screen. |
-----------------------*/
void
AllDisplay (
 void)
{
	long	HHCU_HASH;
	char	wk_date [11];
	char	Status [20];
	char	priceStr [9] [38];
	int		SerFound;
	int		i;

	if (main_open)
		Dsp_close ();

	main_open = TRUE;

	sprintf (head_text [0], ". Serial Number: %-25.25s  W/hse No : %-2.2s   %-19.19s",
						local_rec.serial_no,
						ccmr_rec.cc_no,ccmr_rec.acronym);
	sprintf (head_text [1], ". Item Number  : %16.16s      %-40.40s",
						inmr_rec.item_no, inmr_rec.description);

	Dsp_nc_prn_open (0, 1, 14, head_text [0],
				comm_rec.co_no, comm_rec.co_name,
				comm_rec.est_no, comm_rec.est_name,
				 (char *) 0, (char *) 0);

	Dsp_saverec (head_text [0]);
	Dsp_saverec (head_text [1]);

	Dsp_saverec ("");
	/*--------------------
	| Display misc data. |
	--------------------*/
	display_ok = 1;

	memset ((char *) &SER_rec, '\0', sizeof (struct store_rec));

	SerFound = FindMhdr (insf_rec.serial_no);

	if (!SerFound)
		strcpy (SER_rec.ManSerialNo,"                ");

	cc = 	SerialFindOrder
			 (
				incc_rec.hhbr_hash,
				insf_rec.serial_no,
				SerFound
			);

	if (cc)
	{
		memset ((char *) &SER_rec, '\0', sizeof (struct store_rec));
		//memcpy ((char *) &SER_rec, '\0', sizeof (struct store_rec));
	}

	cc = FindShipment (incc_rec.hhbr_hash, insf_rec.serial_no);


	if (insf_rec.status [0] == 'C')
	{
		if (insf_rec.hhcu_hash == 0L)
			HHCU_HASH = sohr_rec.hhcu_hash;
		else
			HHCU_HASH = insf_rec.hhcu_hash;

		cumr_rec.hhcu_hash	=	HHCU_HASH;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no," ");
			strcpy (cumr_rec.dbt_name," ");
		}
	}
	else if (insf_rec.status [0] == 'S')
	{
		if (insf_rec.hhcu_hash == 0L)
			HHCU_HASH = cohr_rec.hhcu_hash;
		else
			HHCU_HASH = insf_rec.hhcu_hash;

		cumr_rec.hhcu_hash	=	HHCU_HASH;
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			strcpy (cumr_rec.dbt_no,"      ");
			strcpy (cumr_rec.dbt_name,"                              ");
		}
	}
	else if (insf_rec.status [0] == 'F')
	{
		strcpy (SER_rec.InvoiceNo, " ");
		strcpy (SER_rec.OrderNo,   " ");
		SER_rec.DateRaised	=	0L;
		strcpy (SER_rec.CustomerNo," ");
		strcpy (SER_rec.CustomerName, " ");
		cc = 1;
	}

	/*------------------------
	| Read prices from inpr. |
	------------------------*/
	GetPrices ();

	/*-----------------------
	| Set up price strings. |
	-----------------------*/
	for (i = 0; i < 9; i++)
	{
		if (i >= numPrices)
			sprintf (priceStr [i], "%-37.37s", " ");
		else
		{
			sprintf (priceStr [i], "%-16.16s       %14.2f",
					GetPriceDesc (i), DOLLARS (itemPrice [i]));
		}
	}
	strcpy (SER_rec.CostingComplete, (insf_rec.act_cost == 0.00) ? "No " : "Yes");

	sprintf (disp_str, " Estimated Cost         %14.2f ^E Purchase Order Number %-15.15s ", insf_rec.est_cost, SER_rec.PurchaseOrderNo);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Actual Cost            %14.2f ^E Order Date                 %-10.10s ", insf_rec.act_cost, SER_rec.POrdDate);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %-37.37s ^E Supplier Invoice      %-15.15s ",
						priceStr [0], SER_rec.SupplierInv);
	Dsp_saverec (disp_str);

	if (insf_rec.date_in > 0L)
		strcpy (wk_date, DateToString (insf_rec.date_in));
	else
		strcpy (wk_date, "          ");

	sprintf (disp_str, " %-37.37s ^E Date Into Warehouse        %-10.10s ",
						priceStr [1], wk_date);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %-37.37s ^E Date Required              %-10.10s ",
						priceStr [2], SER_rec.PDueDate);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %-37.37s ^E Destination           %-15.15s ", priceStr [3], SER_rec.Destination);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " %-37.37s ^E Vessel                %-15.15s ", priceStr [3], SER_rec.Vessel);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Exchange Rate          %14.2f ^E Estimated Time of Arrival  %-10.10s ", SER_rec.ExchRate, SER_rec.ETA);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Costing complete                  %3.3s ^E Goods Receipt Number          %-7.7s ", SER_rec.CostingComplete, insf_rec.gr_number);
	Dsp_saverec (disp_str);

	if (sohr_rec.dt_raised > 0L)
		strcpy (wk_date, DateToString (sohr_rec.dt_raised));
	else
		strcpy (wk_date, "          ");
	sprintf (disp_str, " Manuf. Serial No.    %16.16s ^E Sales Order Date           %-10.10s ", SER_rec.ManSerialNo, wk_date);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Date Invoiced              %10.10s ^E Sales Order Number           %-8.8s ", DateToString (SER_rec.DateInvoiced), sohr_rec.order_no);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Sale Price             %14.2f ^E Customer                       %-6.6s ", SER_rec.SalePrice, cumr_rec.dbt_no);
	Dsp_saverec (disp_str);

	sprintf (disp_str, " Invoice Number               %8.8s ^E %-36.36s ", insf_rec.invoice_no, cumr_rec.dbt_name);
	Dsp_saverec (disp_str);

	strcpy (Status, "Unknown Status. ");
	if (insf_rec.status [0] == 'S')
		strcpy (Status, "Sold to Customer");
	if (insf_rec.status [0] == 'F')
		strcpy (Status, "Free For Sale   ");
	if (insf_rec.status [0] == 'C')
		strcpy (Status, "Committed       ");

	sprintf (disp_str, " Status of Serial # (%16.16s) ^E Location                   %-10.10s ", Status, insf_rec.location);
	Dsp_saverec (disp_str);

	Dsp_srch ();
}

void
GetPrices (
 void)
{
	int		i;

	for (i = 0; i < 9; i++)
		itemPrice [i] = 0.00;

	for (i = 0; i < numPrices; i++)
	{
		inpr_rec.hhgu_hash  = 0L;
		inpr_rec.hhbr_hash  = inmr_rec.hhbr_hash;
		inpr_rec.price_type = (i + 1);
		strcpy (inpr_rec.br_no, "  ");
		strcpy (inpr_rec.wh_no, "  ");
		sprintf (inpr_rec.curr_code, "%-3.3s", Curr_code);
		sprintf (inpr_rec.area_code, "%-2.2s", "  ");
		sprintf (inpr_rec.cust_type, "%-3.3s", "   ");
		cc = find_rec (inpr, &inpr_rec, COMPARISON, "r");
		if (!cc)
			itemPrice [i] = inpr_rec.base;
	}
}

int
SerialDisplay (
 void)
{
	Dsp_open (0,4,11);
	Dsp_saverec ("                   S T O C K   N U M B E R S   D I S P L A Y                  ");
	Dsp_saverec (" Br | Wh |   Stock Number.   |  Status.   |   Value.     | Number |  Acronym. ");
	Dsp_saverec (std_foot);

	SaveSerialItem ("F","Free");
	SaveSerialItem ("C","Committed");
	SaveSerialItem ("I","In Transit");
	SaveSerialItem ("S","Sold");
	Dsp_srch ();
	Dsp_close ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
SaveSerialItem (
 char *status,
 char *stat_desc)
{
	char	number [7];
	char	acronym [10];
	int	serial_ok;

	incc2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec ("incc2", &incc2_rec, COMPARISON, "r");
	while (!cc && incc2_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		insf2_rec.hhwh_hash = incc2_rec.hhwh_hash;
		strcpy (insf2_rec.status,status);
		strcpy (insf2_rec.serial_no,"                         ");
		cc = find_rec ("insf2", &insf2_rec, GTEQ, "r");
		while (!cc && insf2_rec.hhwh_hash == incc2_rec.hhwh_hash && !strcmp (insf2_rec.status,status))
		{
			if (insf2_rec.status [0] != status [0])
			{
				cc = find_rec ("insf2", &insf2_rec, NEXT, "r");
				continue;
			}

			if ((status [0] == 'S' || status [0] == 'C') && insf2_rec.hhcu_hash != 0L)
			{
				cumr_rec.hhcu_hash	=	insf2_rec.hhcu_hash;
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (cc)
					serial_ok = 0;
				else
				{
					sprintf (number,"%-6.6s",cumr_rec.dbt_no);
					sprintf (acronym,"%-9.9s",cumr_rec.dbt_acronym);
					serial_ok = 1;
				}
			}
			else
			{
				strcpy (number,"  N/A ");
				strcpy (acronym,"   N/A   ");
				serial_ok = 1;
			}

			if (serial_ok)
			{
				cc = FindItem (insf2_rec.hhwh_hash);
				if (!cc)
				{
					sprintf (disp_str," %2.2s ^E %2.2s ^E %-11.11s       ^E %-10s ^E %12.2f ^E %s ^E %s ",
						ccmr_rec.est_no,
						ccmr_rec.cc_no,
						insf2_rec.serial_no,
						stat_desc,
						ser_value (insf2_rec.est_cost,insf2_rec.act_cost),
						number,
						acronym);
					Dsp_saverec (disp_str);
				}
			}
			cc = find_rec ("insf2", &insf2_rec, NEXT, "r");
		}
		cc = find_rec ("incc2", &incc2_rec, NEXT, "r");
	}
}

int
PrintOut (
 void)
{
	if (!printerOpened)
	{
		if ((fin = pr_open ("sk_serdisp.p")) == NULL)
			sys_err ("Error in sk_serdisp.p During (PROPEN)",errno,PNAME);
	}
	printerOpened 	=	TRUE;
	lpXoff	=	1;
	lpYoff	=	2;

	printerNumber	=	get_lpno (0);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (DBOPEN)",errno,PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".PL66\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".OP\n");
	fprintf (fout,".2\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L120\n");
	fflush (fout);
	if (pohr_rec.hhsu_hash	> 0L)
		FindSupplier (pohr_rec.hhsu_hash);
	else
		FindSupplier (insf_rec.hhsu_hash);


	SER_rec.sf_pd_exch = (float) (insf_rec.exch_rate + insf_rec.exch_var);
	if (insf_rec.exch_var < 0.00001)
		SER_rec.sf_pd_exch = 0;

	SER_rec.sf_fob_loc = 0;
	if (insf_rec.exch_rate > 0.00001)
		SER_rec.sf_fob_loc = insf_rec.fob_fgn_cst / insf_rec.exch_rate;

	SER_rec.sf_freight = insf_rec.frt_ins_cst;
	if (insf_rec.exch_rate > 0.00001)
		SER_rec.sf_freight = insf_rec.frt_ins_cst / insf_rec.exch_rate;

	SER_rec.sf_cif = SER_rec.sf_fob_loc +
 		 SER_rec.sf_freight ;
	SER_rec.land_cost = SER_rec.sf_cif * insf_rec.lcost_load * 0.01;
	SER_rec.total_cst = SER_rec.sf_cif + SER_rec.land_cost + insf_rec.prep_cost + insf_rec.duty;
	ProcessItem ();
	fprintf (fout,".EOF\n");
	pclose (fout);
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
ProcessItem (
 void)
{
	GetPrices ();

	pr_format (fin,fout,"HEAD1",0,0);
	pr_format (fin,fout,"HEAD2",0,0);
	pr_format (fin,fout,"HEAD3",0,0);

	pr_format (fin,fout,"LINE1",1,insf_rec.serial_no);
	pr_format (fin,fout,"LINE1",2,ccmr_rec.cc_no);
	pr_format (fin,fout,"LINE1",3,ccmr_rec.acronym);

	pr_format (fin,fout,"LINE2",1,inmr_rec.item_no);
	pr_format (fin,fout,"LINE2",2,inmr_rec.description);

	pr_format (fin,fout,"LINE3",0,0);

	pr_format (fin,fout,"LINE4",1,insf_rec.est_cost);
	pr_format (fin,fout,"LINE4",2,SER_rec.PurchaseOrderNo);

	pr_format (fin,fout,"LINE5",1,insf_rec.act_cost);
	pr_format (fin,fout,"LINE5",2,SER_rec.POrdDate);

	pr_format (fin,fout,"LINE6",1,comm_rec.price1_desc);
	pr_format (fin,fout,"LINE6",2,itemPrice [0]);
	pr_format (fin,fout,"LINE6",3,SER_rec.SupplierInv);

	pr_format (fin,fout,"LINE7",1,comm_rec.price2_desc);
	pr_format (fin,fout,"LINE7",2,itemPrice [1]);
	pr_format (fin,fout,"LINE7",3,insf_rec.date_in);

	pr_format (fin,fout,"LINE8",1,comm_rec.price3_desc);
	pr_format (fin,fout,"LINE8",2,itemPrice [2]);
	pr_format (fin,fout,"LINE8",3,SER_rec.PDueDate);

	pr_format (fin,fout,"LINE9",1,comm_rec.price4_desc);
	pr_format (fin,fout,"LINE9",2,itemPrice [3]);
	pr_format (fin,fout,"LINE9",3,SER_rec.Destination);

	pr_format (fin,fout,"LINE10",1,SER_rec.Vessel);

	pr_format (fin,fout,"LINE11",1,SER_rec.ExchRate);
	pr_format (fin,fout,"LINE11",2,SER_rec.ETA);

	pr_format (fin,fout,"LINE12",1,SER_rec.CostingComplete);
	pr_format (fin,fout,"LINE12",2,insf_rec.gr_number);

	pr_format (fin,fout,"LINE13",1,SER_rec.ManSerialNo);
	pr_format (fin,fout,"LINE13",2,sohr_rec.dt_raised);

	pr_format (fin,fout,"LINE14",1,SER_rec.DateInvoiced);
	pr_format (fin,fout,"LINE14",2,sohr_rec.order_no);

	pr_format (fin,fout,"LINE15",1,SER_rec.SalePrice);
	pr_format (fin,fout,"LINE15",2,cumr_rec.dbt_no);

	pr_format (fin,fout,"LINE16",1,insf_rec.invoice_no);
	pr_format (fin,fout,"LINE16",2,cumr_rec.dbt_name);

	pr_format (fin,fout,"LINE17",1,insf_rec.status);
	pr_format (fin,fout,"LINE17",2,insf_rec.location);

	pr_format (fin,fout,"LINE18",0,0);
	pr_format (fin,fout,"LINE19",0,0);
	pr_format (fin,fout,"LINE20",0,0);

	pr_format (fin,fout,"LINE21",1,sumr_rec.curr_code);
	pr_format (fin,fout,"LINE21",2,insf_rec.fob_fgn_cst);
	pr_format (fin,fout,"LINE21",3,SER_rec.sf_pd_exch);
	pr_format (fin,fout,"LINE22",1,insf_rec.exch_rate);
	pr_format (fin,fout,"LINE22",2,insf_rec.act_cost);
	pr_format (fin,fout,"LINE23",1,Curr_code);
	pr_format (fin,fout,"LINE23",2,SER_rec.sf_fob_loc);
	pr_format (fin,fout,"LINE24",1,Curr_code);
	pr_format (fin,fout,"LINE24",2,SER_rec.sf_freight);
	pr_format (fin,fout,"LINE25",1,Curr_code);
	pr_format (fin,fout,"LINE25",2,SER_rec.sf_cif);
	pr_format (fin,fout,"LINE26",1,insf_rec.duty);
	pr_format (fin,fout,"LINE27",1,SER_rec.land_cost);
	pr_format (fin,fout,"LINE28",1,insf_rec.prep_cost);
	pr_format (fin,fout,"LINE29",1,SER_rec.total_cst);
	pr_format (fin,fout,"LINE30",0,0);

}

int
heading (
 int scn)
{


	if (!restart)
	{
		if (scn == 1)
			scn_set (scn);

		if (clear_ok)
			clear ();

		rv_pr (ML (" Stock Number Display. "),18,0,1);

		//box (0,1,80,2);
		box (0,1,132,2);

		print_at (23,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		//line_at (22,0,80);
		line_at (22,0,132);
		line_cnt = 0;
		scn_write (scn);
	}

	if (display_ok)
	{
		sprintf (insf_rec.serial_no,"%-25.25s",local_rec.serial_no);

		//insf_rec.hhwh_hash = local_rec.hhwh_hash;
		insf_rec.hhwh_hash = 0L;

		strcpy (insf_rec.status," ");
		cc = find_rec (insf, &insf_rec, GTEQ, "r");
		if (cc || strncmp (insf_rec.serial_no,local_rec.serial_no,25) || inmr_rec.serial_item[0] == 'N')
		{

			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		AllDisplay ();

		scn_display (1);

		clear_ok = FALSE;
	}
    return (EXIT_SUCCESS);
}

/*===========================
| Search for serial number. |
===========================*/
void
SrchInsf (
 char *key_val)
{
	_work_open (15,25,16);
	save_rec ("#Serial No","#Item Number");
	sprintf (insf_rec.serial_no,"%-25.25s",key_val);
	insf_rec.hhwh_hash = 0L;
	strcpy (insf_rec.status," ");
	cc = find_rec (insf, &insf_rec, GTEQ, "r");
	while (!cc && !strncmp (insf_rec.serial_no,key_val,strlen (key_val)))
	{
		cc = FindItem (insf_rec.hhwh_hash);
		if (!cc)
		{
			sprintf (err_str,"%-25.25s %10ld",insf_rec.serial_no,insf_rec.hhwh_hash);
			cc = save_rec (err_str,inmr_rec.item_no);
			if (cc)
				break;
		}
		cc = find_rec (insf, &insf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (insf_rec.serial_no,"%-25.25s",temp_str);
	insf_rec.hhwh_hash = atol (temp_str + 26);
	strcpy (insf_rec.status," ");
	cc = find_rec (insf, &insf_rec, GTEQ, "r");
	if (cc || strncmp (insf_rec.serial_no,temp_str,25) || insf_rec.hhwh_hash != atol (temp_str + 27))
		sys_err ("Error in insf During (DBFIND)",cc,PNAME);

	strcpy (temp_str, insf_rec.serial_no);
	hhwh_hash = insf_rec.hhwh_hash;
}

int
FindItem (
 long	hhwhHash)
{
	incc_rec.hhwh_hash	=	hhwhHash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);

	ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc || strcmp (ccmr_rec.co_no,comm_rec.co_no))
		return (2);

	inmr_rec.hhbr_hash = incc_rec.hhbr_hash;
	cc = find_rec ("inmr",&inmr_rec,COMPARISON,"r");
	if (cc)
		return (3);

	return (EXIT_SUCCESS);
}

int
SerialFindOrder (
 long	hhbrHash,
 char	*SER_NO,
 int	SerFound)
{
	int		found = FALSE;

	switch (insf_rec.status [0])
	{
	case	'C':
		soln_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhbr_hash == hhbrHash)
		{
			if (!strncmp (soln_rec.serial_no,SER_NO,25))
			{
				sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
				cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
				if (cc)
				{
					cc = find_rec (soln, &soln_rec, NEXT, "r");
					continue;
				}
				found	=	TRUE;
				break;
			}
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		SER_rec.SalePrice		=	 ((100 - soln_rec.dis_pc) *
								 	DOLLARS (soln_rec.sale_price));
		SER_rec.DateInvoiced 	= 0L;
		break;

	case	'S':

		coln_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (coln,&coln_rec, GTEQ,"r");
		while (!cc && coln_rec.hhbr_hash == hhbrHash)
		{
			if (!strncmp (coln_rec.serial_no,SER_NO,25))
			{
				cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
				cc = find_rec (cohr,&cohr_rec,COMPARISON,"r");
				if (cc)
				{
					cc = find_rec (coln,&coln_rec, NEXT,"r");
					continue;
				}
				found	=	TRUE;
				break;
			}
			cc = find_rec (coln,&coln_rec, NEXT,"r");
		}
		SER_rec.SalePrice		=	0.00;
		SER_rec.DateInvoiced 	=	0L;
		if (found)
		{
			SER_rec.SalePrice = (100 - coln_rec.disc_pc) * DOLLARS (coln_rec.sale_price) * .01;
			SER_rec.DateInvoiced = cohr_rec.date_required;
		}
		else
		{
			if (SerFound)
			{
				SER_rec.SalePrice		=	DOLLARS (mhdr_rec.val_nzd);
				SER_rec.DateInvoiced 	=	mhdr_rec.sell_date;
			}
		}
		break;
	}
	return (found);
}

int
FindShipment (
 long	hhbrHash,
 char	*SER_NO)
{
	char	wk_date [11];

	int		pohr_found = FALSE;

	poln_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec ("poln", &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
		if (!strncmp (poln_rec.serial_no, SER_NO, 25))
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
 			cc = find_rec ("pohr", &pohr_rec, COMPARISON, "r");
			if (!cc)
				pohr_found = TRUE;
			break;
		}
		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}

	if (pohr_found)
	{
		strcpy (SER_rec.PurchaseOrderNo, pohr_rec.pur_ord_no);
		if (pohr_rec.date_raised > 0L)
			strcpy (wk_date, DateToString (pohr_rec.date_raised));
		else
			strcpy (wk_date, "          ");

		strcpy (SER_rec.POrdDate,  wk_date);

		if (pohr_rec.due_date > 0L)
			strcpy (wk_date, DateToString (pohr_rec.due_date));
		else
			strcpy (wk_date, "          ");

		strcpy (SER_rec.PDueDate,  wk_date);
		if (pohr_rec.hhsh_hash == 0)
		{
			strcpy (SER_rec.SupplierInv, insf_rec.crd_invoice);
			strcpy (SER_rec.Destination,      "               ");
			strcpy (SER_rec.Vessel,           "               ");
			strcpy (SER_rec.ETA,         "        ");
			SER_rec.ExchRate		=	 (float) (pohr_rec.curr_rate);
		}
		else
		{
			posd_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
			cc = find_rec (posd, &posd_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (SER_rec.SupplierInv, posd_rec.inv_no);

				strcpy (posh_rec.co_no, pohr_rec.co_no);
				posh_rec.hhsh_hash = pohr_rec.hhsh_hash;
				cc = find_rec (posh, &posh_rec, COMPARISON, "r");
				strcpy (SER_rec.Destination, posh_rec.destination);
				strcpy (SER_rec.Vessel,      posh_rec.vessel);

				if (posh_rec.ship_arrive > 0L)
					strcpy (wk_date, DateToString (posh_rec.ship_arrive));
				else
					strcpy (wk_date, "          ");
				strcpy (SER_rec.ETA,    wk_date);
				SER_rec.ExchRate = posh_rec.ex_rate;
			}
			else
			{
				strcpy (SER_rec.SupplierInv,  insf_rec.crd_invoice);
				strcpy (SER_rec.Destination,       "               ");
				strcpy (SER_rec.Vessel,            "               ");
				strcpy (SER_rec.ETA,          "        ");
				SER_rec.ExchRate = (float) (pohr_rec.curr_rate);
			}
		}
	}
	else
	{
		if (strlen (clip (insf_rec.po_number)) != 0)
			strcpy (SER_rec.PurchaseOrderNo, insf_rec.po_number);
		else
			strcpy (SER_rec.PurchaseOrderNo,"UNKNOWN");
		SER_rec.ExchRate = (float) (insf_rec.exch_rate);
		strcpy (SER_rec.SupplierInv,  insf_rec.crd_invoice);
		strcpy (SER_rec.Destination,      "               ");
		strcpy (SER_rec.Vessel,           "               ");
		strcpy (SER_rec.POrdDate,         "        ");
		strcpy (SER_rec.PDueDate,         "        ");
		strcpy (SER_rec.ETA,         "        ");

	}
	return (EXIT_SUCCESS);
}

int
FindMhdr (
 char	*SER_NO)

{
	strcpy (mhdr_rec.serial_no, SER_NO);
	cc = find_rec (mhdr, &mhdr_rec, COMPARISON, "r");
	if (cc)
		return (FALSE);

	strcpy (SER_rec.ManSerialNo, mhdr_rec.chasis_no);
	return (TRUE);
}

int
CostDisplay (
 void)
{
	Dsp_open (0, 4, 11);
	Dsp_saverec ("                      C O S T I N G        D I S P L A Y                   ");
	Dsp_saverec ("");
	Dsp_saverec (std_foot);

	if (pohr_rec.hhsu_hash > 0L)
		FindSupplier (pohr_rec.hhsu_hash);
	else
		FindSupplier (insf_rec.hhsu_hash);

	sprintf (disp_str,
			 " Supplier     %6.6s   %30.30s ",
			 sumr_rec.crd_no,
			 sumr_rec.crd_name);

	Dsp_saverec (disp_str);
	Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG");

	SER_rec.sf_pd_exch = (float) (insf_rec.exch_rate + insf_rec.exch_var);
	if (insf_rec.exch_var < 0.00001)
		SER_rec.sf_pd_exch = 0.00;

	sprintf (disp_str,
			 " FOB Cost (%3.3s)         %12.2f ^E Paid Exchange Rate        %9.4f",
			 sumr_rec.curr_code,
			 insf_rec.fob_fgn_cst,
			 insf_rec.pd_rate);

	Dsp_saverec (disp_str);

	sprintf (disp_str,
			 " Exchange Rate             %9.4f ^E Paid Rate Cost       %12.2f",
			 insf_rec.exch_rate,
			 insf_rec.paid_cost);
	Dsp_saverec (disp_str);

	SER_rec.sf_fob_loc = 0;
	if (insf_rec.exch_rate > 0.00001)
		SER_rec.sf_fob_loc = insf_rec.fob_fgn_cst / insf_rec.exch_rate;

	sprintf (disp_str,
			 " FOB Cost (%s)         %12.2f ^E Sold Flag               %-1.1s",
			 Curr_code,
			 SER_rec.sf_fob_loc,
			 insf_rec.des_flag);
	Dsp_saverec (disp_str);

	SER_rec.sf_freight = insf_rec.frt_ins_cst;
	if (insf_rec.exch_rate > 0.00001)
		SER_rec.sf_freight = insf_rec.frt_ins_cst / insf_rec.exch_rate;

	sprintf (disp_str, " Freight & Ins (%s)    %12.2f ^E", Curr_code,SER_rec.sf_freight);
	Dsp_saverec (disp_str);

	SER_rec.sf_cif = SER_rec.sf_fob_loc + SER_rec.sf_freight;
	sprintf (disp_str, " CIF (%s)              %12.2f ^E", Curr_code,SER_rec.sf_cif);
	Dsp_saverec (disp_str);
	sprintf (disp_str, " Duty                   %12.2f ^E", insf_rec.duty);
	Dsp_saverec (disp_str);

	SER_rec.land_cost = SER_rec.sf_cif * insf_rec.lcost_load * 0.01;
	sprintf (disp_str, " Landing Costs          %12.2f ^E", SER_rec.land_cost);
	Dsp_saverec (disp_str);
	sprintf (disp_str, " Preparation Costs      %12.2f ^E", insf_rec.prep_cost);
	Dsp_saverec (disp_str);

	SER_rec.total_cst = SER_rec.sf_cif + SER_rec.land_cost + insf_rec.prep_cost + insf_rec.duty;
	sprintf (disp_str, " Total Cost             %12.2f ^E", SER_rec.total_cst);
	Dsp_saverec (disp_str);
	Dsp_srch ();
	Dsp_close ();
	ReDraw ();
    return (EXIT_SUCCESS);
}

void
FindSupplier (
 long	hhsuHash)
{
	sumr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
	if (cc)
	{
		strcpy (sumr_rec.crd_no,"      ");
		strcpy (sumr_rec.crd_name,"                              ");
		strcpy (sumr_rec.curr_code, Curr_code);
	}
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

int
Dsp_heading (
 void)
{
	int		main_check = main_open;

	main_open = FALSE;
	ReDraw ();
	if (main_check)
		Dsp_close ();
	main_open = main_check;
    return (EXIT_SUCCESS);
}

char	*
GetPriceDesc (
	int		priceNumber)
{

	switch (priceNumber)
	{
		case	0:
			return (comm_rec.price1_desc);
		break;

		case	1:
			return (comm_rec.price2_desc);
		break;

		case	2:
			return (comm_rec.price3_desc);
		break;

		case	3:
			return (comm_rec.price4_desc);
		break;

		case	4:
			return (comm_rec.price5_desc);
		break;

		case	5:
			return (comm_rec.price6_desc);
		break;

		case	6:
			return (comm_rec.price7_desc);
		break;

		case	7:
			return (comm_rec.price8_desc);
		break;

		case	8:
			return (comm_rec.price9_desc);
		break;

		default:
			return ("??????????");
	}
}
