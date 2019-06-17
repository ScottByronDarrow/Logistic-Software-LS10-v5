/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_globmnt.c,v 5.6 2002/07/25 11:17:34 scott Exp $
|  Program Name  : (sk_globmnt.c) 
|  Program Desc  : (Global Price Maintenance)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 20/10/93         |
|---------------------------------------------------------------------|
| $Log: sk_globmnt.c,v $
| Revision 5.6  2002/07/25 11:17:34  scott
| Updated to ensure if (prog_exit || restart) was correct.
|
| Revision 5.5  2001/11/05 01:40:42  scott
| Updated from Testing.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_globmnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_globmnt/sk_globmnt.c,v 5.6 2002/07/25 11:17:34 scott Exp $";

#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>


#include	"schema"

struct commRecord	comm_rec;
struct inprRecord	inpr_rec;
struct ingpRecord	ingp_rec;
struct ccmrRecord	ccmr_rec;
struct esmrRecord	esmr_rec;
struct inguRecord	ingu_rec;
struct inmrRecord	inmr_rec;
struct pocrRecord	pocrRec;
struct sumrRecord	sumr_rec;

	Money	*ingu_price	=	&ingu_rec.price_1;
	Money	*ingu_sig	=	&ingu_rec.sig_1;

	char	*data   = "data";

	char	envCurrCode [4];
	int		envCrFind;
	int		envCrCo;
	int		envDbMcurr = FALSE;
	int		envSkDbPriNum;
	int		envSkCusPriLvl;
	char	branchNumber [3];
	int		doesNotExist;

struct
{
	char	filedesc [41];
	char	curr [4];
	char	br [3];
	char	brdesc [41];
	char	wh [3];
	char	whdesc [41];
	char	range [2];
	char	rangedesc [14];
	char	ssellgrp [7];
	char	sselldesc [41];
	char	esellgrp [7];
	char	eselldesc [41];
	char	sbuygrp [7];
	char	sbuydesc [41];
	char	ebuygrp [7];
	char	ebuydesc [41];
	char	sitem [17];
	char	sitemdesc [41];
	char	eitem [17];
	char	eitemdesc [41];
	char	ssupp [7];
	char	ssuppdesc [41];
	char	esupp [7];
	char	esuppdesc [41];
	float	uplift;
	char	roundtype [2];
	char	typedesc [8];
	int		price_type;
	char	price_desc [16];
	char	dummy [11];
} local_rec;

	extern	int		TruePosition,
					EnvScreenOK;
static	struct	var	vars [] =
{
	{1, LIN, "price",	3, 2, INTTYPE,
		"N", "          ",
		"", "",  "Price Type          ", "Enter Price Type To Update. ",
		NE, NO, JUSTLEFT, "1", "9", (char *) &local_rec.price_type},
	{1, LIN, "price_desc",3, 39, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.price_desc},
	{1, LIN, "file",	4, 2, CHARTYPE,
		"UUUUUU", "          ",
		"", "",  "File Name           ", "Enter File Name For Reference / Recall, Search Available.",
		NE, NO, JUSTLEFT, "", "", ingu_rec.file_code},
	{1, LIN, "filedesc",	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",  "File Description    ", "",
		YES, NO, JUSTLEFT, "", "", local_rec.filedesc},
	{1, LIN, "curr",	6, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ",  "Currency            ", "Currency For Global Price Update, Default=Lcl Currency, Search Available",
		YES, NO, JUSTLEFT, "", "", local_rec.curr},
	{1, LIN, "currdesc",	7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "Currency Desc.      ", "",
		NA, NO, JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "effdate",	8, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ","Effective Date      ", "",
		YES, NO, JUSTLEFT, "", "", (char *) &ingu_rec.eff_date},
	{1, LIN, "br",	10, 2, CHARTYPE,
		"NN", "          ",
		" ", " ","Branch Number       ", "Default = A)ll, Search Available",
		YES, NO, JUSTRIGHT, "", "", local_rec.br},
	{1, LIN, "brdesc",	10, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.brdesc},
	{1, LIN, "wh",	11, 2, CHARTYPE,
		"NN", "          ",
		" ", " ","Warehouse Number    ", "Default = A)ll, Search Available",
		YES, NO, JUSTRIGHT, "", "", local_rec.wh},
	{1, LIN, "whdesc",	11, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "",  "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.whdesc},
	{1, LIN, "range",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "S",  "Range Type          ", "Enter Range Type B)uying S)elling P) Supplier I)tem, Default = S)elling",
		YES, NO, JUSTLEFT, "BSPI", "", local_rec.range},
	{1, LIN, "rangedesc",	13, 39, CHARTYPE,
		" AAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rangedesc},
	{1, LIN, "sbuygrp",	14, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Buying Group  ", "Enter Start Buying Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sbuygrp},
	{1, LIN, "sbuydesc",	14, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sbuydesc},
	{1, LIN, "ebuygrp",	15, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Buying Group    ", "Enter End Buying Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ebuygrp},
	{1, LIN, "ebuydesc",	15, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ebuydesc},
	{1, LIN, "ssellgrp",	14, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Selling Group ", "Enter Start Selling Group, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssellgrp},
	{1, LIN, "sselldesc",	14, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sselldesc},
	{1, LIN, "esellgrp",	15, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Selling Group   ", "Enter End Selling Group, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esellgrp},
	{1, LIN, "eselldesc",	15, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eselldesc},
	{1, LIN, "sitem",	14, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ",  "Start Item          ", "Enter Start Item, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.sitem},
	{1, LIN, "sitemdesc",	14, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.sitemdesc},
	{1, LIN, "eitem",	15, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ",  "End Item            ", "Enter End Item, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.eitem},
	{1, LIN, "eitemdesc",	15, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.eitemdesc},
	{1, LIN, "ssupp",	14, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "Start Supplier      ", "Enter Start Supplier, Default = Start Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.ssupp},
	{1, LIN, "ssuppdesc",	14, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.ssuppdesc},
	{1, LIN, "esupp",	15, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",  "End Supplier        ", "Enter End Supplier, Default = End Of File, Full Search Available",
		ND, NO, JUSTLEFT, "", "", local_rec.esupp},
	{1, LIN, "esuppdesc",	15, 39, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.esuppdesc},
	{1, LIN, "uplift",	 17, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", " ",  "Uplift        ", "Enter % Uplift ",
		 YES, NO,  JUSTRIGHT, "-99.99", "99.99", (char *) &local_rec.uplift},
	{1, LIN, "roundtype",	17, 39, CHARTYPE,
		"U", "          ",
		" ", "U", "Rounding   ", "U)p D)own N)earest, Default = Up",
		 YES, NO,  JUSTLEFT, "UDN", "", local_rec.roundtype},
	{1, LIN, "typedesc",	17, 65, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.typedesc},
	{2, LIN, "upto1",	4, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Up To Value 1  ", "Enter Value That Rounding Applies To ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price_1},
	{2, LIN, "sig1",	4, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "Rounding Level 1  ", "Enter Rounding Value",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig_1},
	{2, LIN, "upto2",	5, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            2  ", "Enter Value That Rounding Applies To ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price_2},
	{2, LIN, "sig2",	5, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               2  ", "Enter Rounding Value",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig_2},
	{2, LIN, "upto3",	6, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            3  ", "Enter Value That Rounding Applies To ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price_3},
	{2, LIN, "sig3",	6, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               3  ", "Enter Rounding Value",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig_3},
	{2, LIN, "upto4",	7, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            4  ", "Enter Value That Rounding Applies To ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price_4},
	{2, LIN, "sig4",	7, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               4  ", "Enter Rounding Value",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig_4},
	{2, LIN, "upto5",	8, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "            5  ", "Enter Value That Rounding Applies To ",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.price_5},
	{2, LIN, "sig5",	8, 40, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", " ", "               5  ", "Enter Rounding Value",
		 YES, NO,  JUSTRIGHT, "", "", (char *) &ingu_rec.sig_5},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>
/*=======================
| Function Declarations |
=======================*/
const char	*GetPriceDesc 		(int);
int  	FindGrp 				(char *, int);
int  	heading 				(int);
int  	IntFindSumr 			(char *, int);
int  	ReadCcmr 				(int);
int  	ReadEsmr 				(int);
int  	ReadPocr 				(char *, int);
int  	spec_valid 				(int);
void 	CloseDB 				(void);
void 	GetDesc 				(void);
void 	LoadFields 				(void);
void 	OpenDB 					(void);
void 	SetUpRange 				(int);
void 	shutdown_prog 			(void);
void 	SrchCcmr 				(char *);
void 	SrchEsmr 				(char *);
void 	SrchIngp 				(char *);
void 	SrchIngu 				(char *);
void 	SrchPocr 				(char *);
void 	Update 					(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;
	
	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	sptr	= get_env ("CURR_CODE");
	if (sptr == (char *)0)
		return (EXIT_FAILURE);
	else
		sprintf (envCurrCode, "%3.3s", sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("CR_FIND");
	envCrFind = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("CR_CO");
	envCrCo = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr)
	{
		envSkDbPriNum = atoi (sptr);
		if (envSkDbPriNum > 9 || envSkDbPriNum < 1)
			envSkDbPriNum = 9;
	}
	else
		envSkDbPriNum = 5;

	sptr = chk_env ("SK_CUSPRI_LVL");
	if (sptr == (char *)0)
		envSkCusPriLvl = 0;
	else
		envSkCusPriLvl = atoi (sptr);

	SETUP_SCR (vars);

	if (envSkCusPriLvl == 0) 			/* Company Level Pricing */
	{
		FLD ("br")     			= ND;
		FLD ("brdesc") 			= ND;
		FLD ("wh")     			= ND;
		FLD ("whdesc") 			= ND;
		SCN_ROW ("range")		= 10;
		SCN_ROW ("rangedesc")	= 10;
		SCN_ROW ("sbuygrp")		= 11;
		SCN_ROW ("sbuydesc")	= 11;
		SCN_ROW ("ebuygrp")		= 12;
		SCN_ROW ("ebuydesc")	= 12;
		SCN_ROW ("ssellgrp")	= 11;
		SCN_ROW ("sselldesc")	= 11;
		SCN_ROW ("esellgrp")	= 12;
		SCN_ROW ("eselldesc")	= 12;
		SCN_ROW ("sitem")		= 11;
		SCN_ROW ("sitemdesc")	= 11;
		SCN_ROW ("eitem")		= 12;
		SCN_ROW ("eitemdesc")	= 12;
		SCN_ROW ("ssupp")		= 11;
		SCN_ROW ("ssuppdesc")	= 11;
		SCN_ROW ("esupp")		= 12;
		SCN_ROW ("esuppdesc") 	= 12;
		SCN_ROW ("uplift")		= 14;
		SCN_ROW ("roundtype")	= 14;
		SCN_ROW ("typedesc")	= 14;
	}

	if (envSkCusPriLvl == 1) 			/* Branch Level Pricing */
	{
		FLD ("wh")     			= ND;
		FLD ("whdesc") 			= ND;
		SCN_ROW ("range")		= 12;
		SCN_ROW ("rangedesc")	= 12;
		SCN_ROW ("sbuygrp")		= 13;
		SCN_ROW ("sbuydesc")	= 13;
		SCN_ROW ("ebuygrp")		= 14;
		SCN_ROW ("ebuydesc")	= 14;
		SCN_ROW ("ssellgrp")	= 13;
		SCN_ROW ("sselldesc")	= 13;
		SCN_ROW ("esellgrp")	= 14;
		SCN_ROW ("eselldesc")	= 14;
		SCN_ROW ("sitem")		= 13;
		SCN_ROW ("sitemdesc")	= 13;
		SCN_ROW ("eitem")		= 14;
		SCN_ROW ("eitemdesc")	= 14;
		SCN_ROW ("ssupp")		= 13;
		SCN_ROW ("ssuppdesc")	= 13;
		SCN_ROW ("esupp")		= 14;
		SCN_ROW ("esuppdesc")	= 14;
		SCN_ROW ("uplift")		= 16;
		SCN_ROW ("roundtype")	= 16;
		SCN_ROW ("typedesc")	= 16;
	}

	if (!envDbMcurr)
	{
		sprintf (local_rec.curr, "%3.3s", envCurrCode);
		FLD ("curr") 	 = NA;
		FLD ("currdesc") = ND;
	}
	/*------------------------------
	| Read common terminal record. |
	------------------------------*/
	OpenDB ();
	
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");
	init_scr ();
	set_tty (); 
	set_masks ();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
		init_vars (1);
		init_vars (2);
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
			
		if (prog_exit || restart)
			continue;

		if (doesNotExist)
		{
			heading (2);
			entry (2);

			if (restart)
				continue;
		}

		edit_all ();

		if (restart)
			continue;
		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sumr,sumr_list,SUMR_NO_FIELDS,(envCrFind) ? "sumr_id_no3" 
														: "sumr_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (ingp, ingp_list, INGP_NO_FIELDS, "ingp_id_no2");
	open_rec (ingu, ingu_list, INGU_NO_FIELDS, "ingu_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (inpr, inpr_list, INPR_NO_FIELDS, "inpr_hhgu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ingp);	
	abc_fclose (sumr);	
	abc_fclose (inmr);	
	abc_fclose (ingu);	
	abc_fclose (esmr);	
	abc_fclose (ccmr);	
	abc_fclose (pocr);	
	abc_fclose (inpr);	
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (FLD ("curr") == NA && cur_screen == 1)
	{
		sprintf (local_rec.curr, "%3.3s", envCurrCode);
		DSP_FLD ("curr");
	}

	if (LCHECK ("price"))
	{
		if (SRCH_KEY)
			return (EXIT_FAILURE);

		if (local_rec.price_type > envSkDbPriNum)
		{
			sprintf (err_str, ML (mlSkMess218), envSkDbPriNum); 
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.price_desc, GetPriceDesc (local_rec.price_type));
		DSP_FLD ("price_desc");
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("sig", 3))
	{
		int	count = atoi (FIELD.label + 3);
		count --;

		if (ingu_price [count] == 0.00)
		{
			print_mess (ML (mlSkMess219));
			sleep (sleepTime);
			clear_mess ();

			/*---------------------
			| force it to be zero |
			----------------------*/
			ingu_sig [count] = 0.00;
			return (EXIT_SUCCESS);
		}

		if (ingu_sig [count] >= ingu_price [count]) 
		{
			print_mess (ML (mlSkMess220));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if ((ingu_sig [count] / ingu_price [count]) > 0.2)
		{
			print_mess (ML (mlSkMess221));
			sleep (sleepTime);
			clear_mess ();

			/*-------------------------
			| this is a warning only  |
			| therefore no return (EXIT_FAILURE);|
			--------------------------*/
		}
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("upto", 4))
	{
		int	count = atoi (FIELD.label + 4);
		count --;

		if (!ingu_price [count])
		{
			int	i;
			for (i = count; i < 5; i++)
			{
				ingu_price [i] = 0.00;
				ingu_sig [i] = 0.00;
			}
			entry_exit = TRUE;
			scn_display (2);
			return (EXIT_SUCCESS);
		}

		/*--------------------------------
		| make sure last entered no == 0.00
		--------------------------------*/
		if (count)
		{
			if (ingu_price [count - 1] == 0.00)
			{
				print_mess (ML (mlSkMess333));
				sleep (sleepTime);
				clear_mess ();
				/*----------------
				| force it to be zero
				--------------------*/
				ingu_price [count] = 0.00;
				return (EXIT_SUCCESS);
			}
		}

		/*--------------------------------
		| make sure larger than last value
		--------------------------------*/
		if (count)
		{
			if (ingu_price [count] <= ingu_price [count - 1])
			{
				print_mess (ML (mlSkMess222));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*--------------------------------
		| make sure less than next value
		| and that value is not zero.    
		--------------------------------*/
		if (prog_status != ENTRY && count != 4)
		{
			if ((ingu_price [count] >= ingu_price [count + 1]) &&
				ingu_price [count + 1] != 0.00)
			{
				print_mess (ML (mlSkMess223));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if ((ingu_sig [count] / ingu_price [count]) > 0.2)
		{
			print_mess (ML (mlSkMess221));
			sleep (sleepTime);
			clear_mess ();

			/*
			 * this is a warning only therefore no return (EXIT_FAILURE);
			 */
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("roundtype"))
	{
		GetDesc ();
		DSP_FLD ("typedesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("effdate"))
	{
		if (dflt_used)
		{
			ingu_rec.eff_date = comm_rec.inv_date;
			DSP_FLD ("effdate");
			return (EXIT_SUCCESS);
		}

		if (ingu_rec.eff_date < comm_rec.inv_date)
		{
			print_mess (ML (mlSkMess331));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr"))
	{
		if (FLD ("curr") == NA)
		{
			DSP_FLD ("curr");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			if (ReadPocr (envCurrCode, TRUE))
				return (EXIT_FAILURE);
			strcpy (local_rec.curr, pocrRec.code);
		}
		else
		{
			if (ReadPocr (local_rec.curr, TRUE))
				return (EXIT_FAILURE);
		}
	
		DSP_FLD ("currdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("br"))
	{
		if (envSkCusPriLvl == 0)
		{
			strcpy (local_rec.br, "  ");
			strcpy (local_rec.wh, "  ");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (ReadEsmr (TRUE))
			return (EXIT_FAILURE);

		DSP_FLD ("br");
		DSP_FLD ("brdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh"))
	{
		if (envSkCusPriLvl == 1)
		{
			strcpy (local_rec.wh, "  ");
			return (EXIT_SUCCESS);
		}

		if (FLD ("wh") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (ReadCcmr (TRUE))
			return (EXIT_FAILURE);

		DSP_FLD ("wh");
		DSP_FLD ("whdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("file"))
	{
		strcpy (ingu_rec.co_no, comm_rec.co_no);

		if (SRCH_KEY)
		{
			SrchIngu (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_FAILURE);

		ingu_rec.price_type = local_rec.price_type;
		doesNotExist = find_rec (ingu, &ingu_rec, EQUAL, "u");
		if (doesNotExist)
		{
			return (EXIT_SUCCESS);
		}
		else
		{
			/*
			 * do not allow edit if inprs exist for this hhgu_hash
			 */
			inpr_rec.hhgu_hash	=	ingu_rec.hhgu_hash;
			cc = find_rec (inpr, &inpr_rec, EQUAL, "r");
			if (!cc)
			{
				print_mess (ML (mlSkMess332));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}

			LoadFields ();
			scn_display (1);
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
	}

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("sitem"))
	{
		if (FLD ("sitem") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.sitem, "%16.16s", " ");
			strcpy (local_rec.sitemdesc, ML ("Start Of File "));
			DSP_FLD ("sitemdesc");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.sitem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.sitem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.sitem, inmr_rec.item_no);
		strcpy (local_rec.sitemdesc, inmr_rec.description);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sitem, local_rec.eitem) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		DSP_FLD ("sitemdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eitem"))
	{
		if (FLD ("eitem") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			sprintf (local_rec.eitem, "%16.16s", "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.eitemdesc, ML ("End Of File "));
			DSP_FLD ("eitemdesc");
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.eitem, "~~~~~~~~~~~~~~~~"))
			strcpy (inmr_rec.description, ML ("End Of File "));

		cc = FindInmr (comm_rec.co_no, local_rec.eitem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.eitem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc && strcmp (local_rec.eitem, "~~~~~~~~~~~~~~~~"))
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.eitem,inmr_rec.item_no);
		strcpy (local_rec.eitemdesc, inmr_rec.description);

		if (strcmp (local_rec.sitem, local_rec.eitem) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("eitemdesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| supplier Validate Creditor Number. 
	---------------------------*/
	if (LCHECK ("ssupp"))
	{
		if (FLD ("ssupp") == ND)
			return (EXIT_SUCCESS);


		if (dflt_used)
		{
			sprintf (local_rec.ssupp, "%6.6s", " ");
			strcpy (local_rec.ssuppdesc, ML ("Start Of File "));
			DSP_FLD ("ssuppdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.ssupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.ssupp, sumr_rec.crd_no);
		strcpy (local_rec.ssuppdesc, sumr_rec.crd_name);

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		DSP_FLD ("ssuppdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esupp"))
	{
		if (FLD ("esupp") == ND)
			return (EXIT_SUCCESS);


		if (!strcmp (local_rec.esupp, "~~~~~~"))
		{
			strcpy (local_rec.esuppdesc, ML ("End Of File "));
			strcpy 	 (sumr_rec.crd_name,	local_rec.esuppdesc);
		}

		if (dflt_used)
		{
			sprintf (local_rec.esupp, "%6.6s", "~~~~~~");
			strcpy (local_rec.esuppdesc, ML ("End Of File "));
			strcpy 	 (sumr_rec.crd_name,	local_rec.esuppdesc);
			DSP_FLD ("esuppdesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (IntFindSumr (local_rec.esupp, TRUE))
			return (EXIT_FAILURE);

		strcpy (local_rec.esuppdesc, sumr_rec.crd_name);
		strcpy (local_rec.esupp, sumr_rec.crd_no);

		if (strcmp (local_rec.ssupp, local_rec.esupp) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("esuppdesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * if range entered was by buying group
	 */
	if (LCHECK ("sbuygrp"))
	{
		if (FLD ("sbuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.sbuygrp, "      ");
			strcpy (ingp_rec.desc, ML ("Start Of File"));
			
		}
		else
		{
			strcpy (ingp_rec.type, "B");
			if (FindGrp (local_rec.sbuygrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sbuydesc, ingp_rec.desc);
		DSP_FLD ("sbuydesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ebuygrp"))
	{
		if (FLD ("ebuygrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.ebuygrp, "~~~~~~"))
			strcpy (ingp_rec.desc, ML ("End Of File"));

		
		if (dflt_used)
		{
			strcpy (local_rec.ebuygrp, "~~~~~~");
			strcpy (ingp_rec.desc, ML ("End Of File"));
		}
		else
		{
			strcpy (ingp_rec.type, "B");
			if (FindGrp (local_rec.ebuygrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.sbuygrp, local_rec.ebuygrp) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.ebuydesc, ingp_rec.desc);
		DSP_FLD ("ebuydesc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| if range entered was by
	| selling group
	---------------------------*/
	if (LCHECK ("ssellgrp"))
	{
		if (FLD ("ssellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.ssellgrp, "      ");
			strcpy (ingp_rec.desc, ML ("Start Of File"));
			
		}
		else
		{
			strcpy (ingp_rec.type, "S");
			if (FindGrp (local_rec.ssellgrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY)
		{
			if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
			{
				print_mess (ML (mlStdMess017));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.sselldesc, ingp_rec.desc);
		DSP_FLD ("sselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("esellgrp"))
	{
		if (FLD ("esellgrp") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchIngp (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (local_rec.esellgrp, "~~~~~~"))
			strcpy (ingp_rec.desc, ML ("End Of File"));

		
		if (dflt_used)
		{
			strcpy (local_rec.esellgrp, "~~~~~~");
			strcpy (ingp_rec.desc, ML ("End Of File"));
		}
		else
		{
			strcpy (ingp_rec.type, "S");
			if (FindGrp (local_rec.esellgrp, TRUE))
				return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.ssellgrp, local_rec.esellgrp) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.eselldesc, ingp_rec.desc);
		DSP_FLD ("eselldesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("range"))
	{
		static char	last_range [3] = {"  "};

		/*
		FLD ("sbuygrp")   = ND;
		FLD ("ebuygrp")   = ND;
		FLD ("sbuydesc")  = ND;
		FLD ("ebuydesc")  = ND;
		FLD ("ssellgrp")  = ND;
		FLD ("esellgrp")  = ND;
		FLD ("sselldesc") = ND;
		FLD ("eselldesc") = ND;
		FLD ("sitem")  	  = ND;
		FLD ("eitem")     = ND;
		FLD ("sitemdesc") = ND;
		FLD ("eitemdesc") = ND;
		FLD ("ssupp")     = ND;
		FLD ("esupp")     = ND;
		FLD ("ssuppdesc") = ND;
		FLD ("esuppdesc") = ND;
		*/

		if (prog_status != ENTRY && local_rec.range [0] == last_range [0])
			return (EXIT_SUCCESS);

#ifndef GVISION
		if (envSkCusPriLvl == 0)
		{
			print_at (10, 2, "%70.70s", " ");
			print_at (11, 2, "%70.70s", " ");
	 	}
		else
		if (envSkCusPriLvl == 1)
		{
			print_at (12, 2, "%70.70s", " ");
			print_at (13, 2, "%70.70s", " ");
		}
		else
		{
			print_at (14, 2, "%70.70s", " ");
			print_at (15, 2, "%70.70s", " ");
		}
#endif

		SetUpRange (FALSE);

		scn_write (1);
		DSP_FLD ("rangedesc");

		/*----------------------
		| if change the range then 
		| do get_entry's for both
		| new fields
		------------------------*/
		if (prog_status != ENTRY && local_rec.range [0] != last_range [0])
		{
			DSP_FLD ("range");
			/*  sellgrp */
			if (local_rec.range [0] == 'S')
			{
				do
				{
					get_entry (label ("ssellgrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ssellgrp")));
				DSP_FLD ("ssellgrp");

				do
				{
					get_entry (label ("esellgrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("esellgrp")));
				DSP_FLD ("esellgrp");
			}

			/*  buygrp */
			if (local_rec.range [0] == 'B')
			{
				do
				{
					get_entry (label ("sbuygrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("sbuygrp")));
				DSP_FLD ("sbuygrp");

				do
				{
					get_entry (label ("ebuygrp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ebuygrp")));
				DSP_FLD ("ebuygrp");
			}

			/*  item */
			if (local_rec.range [0] == 'I')
			{
				do
				{
					get_entry (label ("sitem"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("sitem")));
				DSP_FLD ("sitem");

				do
				{
					get_entry (label ("eitem"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("eitem")));
				DSP_FLD ("eitem");
			}

			/*  supplier */
			if (local_rec.range [0] == 'P')
			{
				do
				{
					get_entry (label ("ssupp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("ssupp")));
				DSP_FLD ("ssupp");

				do
				{
					get_entry (label ("esupp"));
					if (restart)
						return (EXIT_SUCCESS);
				} while (spec_valid (label ("esupp")));
				DSP_FLD ("esupp");
			}
		}

		strcpy (last_range, local_rec.range);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
SrchIngp (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description ");

	if (local_rec.range [0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", key_val);
	cc = find_rec (ingp, &ingp_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingp_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingp_rec.code, key_val, strlen (key_val)))
	{
		if (ingp_rec.type [0] == 'S' && local_rec.range [0] == 'S')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (ingp_rec.type [0] == 'B' && local_rec.range [0] == 'B')
				cc = save_rec (ingp_rec.code, ingp_rec.desc);
		if (cc)
			break;

		cc = find_rec (ingp, &ingp_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingp_rec.co_no, comm_rec.co_no);
	sprintf (ingp_rec.code, "%-6.6s", temp_str);
	if (local_rec.range [0] == 'S')
		strcpy (ingp_rec.type, "S");
	else
		strcpy (ingp_rec.type, "B");
	cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingp, "DBFIND");
}

int
heading (
 int scn)
{
	int		ws_lines;

	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();

	sprintf (err_str, ML (mlSkMess340));  
	rv_pr (err_str, (80 - strlen (clip (err_str))) / 2, 0, 1);
	
	if (envSkCusPriLvl == 0) 		/* Branch Level Pricing */
		ws_lines = 0;
	else
	if (envSkCusPriLvl == 1) 		/* Branch Level Pricing */
		ws_lines = 2;
	else
		ws_lines = 3;

	if (scn == 1)
	{
		box (0, 2, 80, 12 + ws_lines);

		if (!envDbMcurr)
			line_at (7,1,79);
		
		line_at (9,1,79);
		if (envSkCusPriLvl > 0)
		{
			line_at (9 + ws_lines, 1, 79);
		}
		line_at (13 + ws_lines, 1, 79);
	}
	else
		box (0, 3, 80, 5);

	line_at (21, 0, 80);

	strcpy (err_str, ML (mlStdMess038));
	print_at (22, 0,err_str, comm_rec.co_no, comm_rec.co_name);

	line_at (1, 0, 80);
	
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

void
Update (void)
{
	strcpy (ingu_rec.curr_code, local_rec.curr);
	strcpy (ingu_rec.file_desc, local_rec.filedesc);

	if (!strcmp (local_rec.br, "~~"))
		strcpy (local_rec.br, "  ");
	if (!strcmp (local_rec.wh, "~~"))
		strcpy (local_rec.wh, "  ");

	strcpy (ingu_rec.br_no, local_rec.br);
	strcpy (ingu_rec.wh_no, local_rec.wh);
	strcpy (ingu_rec.apply_to, local_rec.range);

	if (ingu_rec.apply_to [0] == 'B')
	{
		sprintf (ingu_rec.st_range, "%-6.6s", local_rec.sbuygrp);
		sprintf (ingu_rec.end_range, "%-6.6s", local_rec.ebuygrp);
	}

	if (ingu_rec.apply_to [0] == 'S')
	{
		sprintf (ingu_rec.st_range, "%-6.6s", local_rec.ssellgrp);
		sprintf (ingu_rec.end_range, "%-6.6s", local_rec.esellgrp);
	}

	if (ingu_rec.apply_to [0] == 'I')
	{
		sprintf (ingu_rec.st_range, "%-16.16s", local_rec.sitem);
		sprintf (ingu_rec.end_range, "%-16.16s", local_rec.eitem);
	}

	if (ingu_rec.apply_to [0] == 'P')
	{
		sprintf (ingu_rec.st_range, "%-6.6s", local_rec.ssupp);
		sprintf (ingu_rec.end_range, "%-6.6s", local_rec.esupp);
	}

	ingu_rec.uplift = local_rec.uplift;
	strcpy (ingu_rec.rounding, local_rec.roundtype);

	if (doesNotExist)
		cc = abc_add (ingu, &ingu_rec);
	else
		cc = abc_update (ingu, &ingu_rec);
	
	if (cc)
		file_err (cc, ingu, (doesNotExist) ? "DBADD" : "DBUPDATE");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
LoadFields (void)
{
	strcpy (local_rec.filedesc, ingu_rec.file_desc);

	/*
	 * read and load currency
	 */
	strcpy (local_rec.curr, ingu_rec.curr_code);
	if (ReadPocr (local_rec.curr, FALSE))
		return;

	/*-------------------------
	| read and load branch
	-----------------------*/
	strcpy (local_rec.br, ingu_rec.br_no);
	if (ReadEsmr (FALSE))
		return;

	/*
	 * read and load warehouse
	 */
	strcpy (local_rec.wh, ingu_rec.wh_no);
	if (ReadCcmr (FALSE))
		return;

	/*
	 * read and load range read different files depending upon type
	 */
	strcpy (local_rec.range, ingu_rec.apply_to);
	SetUpRange (TRUE);

	scn_write (1);
	DSP_FLD ("rangedesc");

	/*
	 * load uplift
	 */
	local_rec.uplift = ingu_rec.uplift;

	/*
	 * load rounding
	 */
	local_rec.roundtype [0] = ingu_rec.rounding [0];
	GetDesc ();
}

void
SrchIngu (
char 	*key_val)
{
	_work_open (6,0,40);
	save_rec ("#File", "#Description ");

	strcpy (ingu_rec.co_no, comm_rec.co_no);
	sprintf (ingu_rec.file_code, "%-6.6s", key_val);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ingu_rec.co_no, comm_rec.co_no) &&
	       !strncmp (ingu_rec.file_code, key_val, strlen (key_val)))
	{
		if (ingu_rec.price_type != local_rec.price_type)
		{
			cc = find_rec (ingu, &ingu_rec, NEXT, "r");
			continue;
		}
		cc = save_rec (ingu_rec.file_code, ingu_rec.file_desc);
		if (cc)
			break;

		cc = find_rec (ingu, &ingu_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ingu_rec.co_no, comm_rec.co_no);
	sprintf (ingu_rec.file_code, "%-6.6s", temp_str);
	ingu_rec.price_type = local_rec.price_type;
	cc = find_rec (ingu, &ingu_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ingu, "DBFIND");
}

int
ReadPocr (
	char 	*rec, 
	int 	Errors)
{
	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%3.3s", rec);
	cc = find_rec (pocr, &pocrRec, EQUAL, "r");
	if (cc)
	{
		if (Errors)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
		}
		else
			file_err (cc, pocr, "DBFIND");
	}
	return (cc);
}

int
ReadEsmr (
	int 	Errors)
{
	if (dflt_used || !strcmp (local_rec.br, "  "))
	{
		cc = FALSE;
		strcpy (local_rec.br, "~~");
		strcpy (local_rec.brdesc, ML ("ALL"));
		if (envSkCusPriLvl == 2)
		{
			FLD ("wh") = NA;
			strcpy (local_rec.wh, "~~");
			strcpy (local_rec.whdesc, ML ("ALL"));
			DSP_FLD ("wh");
			DSP_FLD ("whdesc");
		}
	}
	else
	{
		if (prog_status == ENTRY && envSkCusPriLvl == 2)
		{
			strcpy (local_rec.wh, "  ");
			strcpy (local_rec.whdesc, " ");
			DSP_FLD ("wh");
			DSP_FLD ("whdesc");
		}
		strcpy (esmr_rec.co_no, ingu_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			if (Errors)
			{
				print_mess (ML (mlStdMess073));
				sleep (sleepTime);
				clear_mess ();
			}
			else
				file_err (cc, esmr, "DBFIND");
		}
		if (envSkCusPriLvl == 2)
			FLD ("wh") = YES;
		sprintf (local_rec.brdesc, "%s %-35.35s", local_rec.br, esmr_rec.est_name);
	}
	DSP_FLD ("brdesc");
	return (cc);
}

int
ReadCcmr (
	int 	Errors)
{
	if (dflt_used || !strcmp (local_rec.wh, "  "))
	{
		cc = FALSE;
		strcpy (local_rec.whdesc, ML ("ALL"));
		strcpy (local_rec.wh, "~~");
	}
	else
	{
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.br);
		strcpy (ccmr_rec.cc_no, local_rec.wh);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			if (Errors)
			{
				print_mess (ML (mlStdMess100));
				sleep (sleepTime);
				clear_mess ();
			}
			else
				file_err (cc, ccmr, "DBFIND");
		}
		sprintf (local_rec.whdesc, "%s %-35.35s", local_rec.wh, ccmr_rec.name);
	}
	DSP_FLD ("whdesc");
	return (cc);
}

/*
 * If load is true, copy in from ingu else blank them, this will also
 * mean reading ingp or sumr or inmr for descriptions
 */
void
SetUpRange (
	int		load)
{
	FLD ("sbuygrp")   = ND;
	FLD ("ebuygrp")   = ND;
	FLD ("sbuydesc")  = ND;
	FLD ("ebuydesc")  = ND;
	FLD ("ssellgrp")  = ND;
	FLD ("esellgrp")  = ND;
	FLD ("sselldesc") = ND;
	FLD ("eselldesc") = ND;
	FLD ("sitem")  	  = ND;
	FLD ("eitem")     = ND;
	FLD ("sitemdesc") = ND;
	FLD ("eitemdesc") = ND;
	FLD ("ssupp")     = ND;
	FLD ("esupp")     = ND;
	FLD ("ssuppdesc") = ND;
	FLD ("esuppdesc") = ND;

	/*
	 * if choice is buying group
	 */
	if (local_rec.range [0] == 'B')
	{
		FLD ("sbuygrp")  = YES;
		FLD ("ebuygrp")  = YES;
		FLD ("sbuydesc") = NA;
		FLD ("ebuydesc") = NA;

		sprintf (local_rec.rangedesc, "%-13.13s", ML (mlSkMess075));

		if (load)
		{
			sprintf (local_rec.sbuygrp, "%-6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.sbuygrp, "      "))
				strcpy (ingp_rec.desc, ML ("Start Of File"));
			else
			{
				strcpy (ingp_rec.type, "B");
				if (FindGrp (local_rec.sbuygrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.sbuydesc, ingp_rec.desc);

			sprintf (local_rec.ebuygrp, "%-6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.ebuygrp, "~~~~~~"))
				strcpy (ingp_rec.desc, ML ("End Of File"));
			else
			{
				strcpy (ingp_rec.type, "B");
				if (FindGrp (local_rec.ebuygrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.ebuydesc, ingp_rec.desc);
		}
		else
		{
			sprintf (local_rec.sbuygrp, "%6.6s", " ");
			sprintf (local_rec.ebuygrp, "%6.6s", "~~~~~~");
			sprintf (local_rec.sbuydesc, "%40.40s", " ");
			sprintf (local_rec.ebuydesc, "%40.40s", " ");
		}
		return;
	}

	/*
	 * if choice is selling group
	 */
	if (local_rec.range [0] == 'S')
	{
		FLD ("ssellgrp")  = YES;
		FLD ("esellgrp")  = YES;
		FLD ("sselldesc") = NA;
		FLD ("eselldesc") = NA;

		sprintf (local_rec.rangedesc, "%-13.13s", ML (mlSkMess074));
		if (load)
		{
			sprintf (local_rec.ssellgrp, "%-6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.ssellgrp, "      "))
			{
				strcpy (ingp_rec.desc, ML ("Start Of File"));
			}
			else
			{
				strcpy (ingp_rec.type, "S");
				if (FindGrp (local_rec.ssellgrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.sselldesc, ingp_rec.desc);

			sprintf (local_rec.esellgrp, "%-6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.esellgrp, "~~~~~~"))
			{
				strcpy (ingp_rec.desc, ML ("End Of File"));
			}
			else
			{
				strcpy (ingp_rec.type, "S");
				if (FindGrp (local_rec.esellgrp, FALSE))
					file_err (cc, ingp, "DBFIND");
			}
			strcpy (local_rec.eselldesc, ingp_rec.desc);
		}
		else
		{
			sprintf (local_rec.ssellgrp, "%6.6s", " ");
			sprintf (local_rec.esellgrp, "%6.6s", "~~~~~~");
			sprintf (local_rec.sselldesc, "%40.40s", " ");
			sprintf (local_rec.eselldesc, "%40.40s", " ");
		}
		return;
	}

	/*
	 * if choice is item
	 */
	if (local_rec.range [0] == 'I')
	{
		FLD ("sitem")  = YES;
		FLD ("eitem")  = YES;
		FLD ("sitemdesc") = NA;
		FLD ("eitemdesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", ML ("Item"));

		if (load)
		{
			/* start item */
			sprintf (local_rec.sitem, "%16.16s", ingu_rec.st_range);
			if (!strcmp (local_rec.sitem, "                "))
			{
				strcpy (local_rec.sitemdesc, ML ("Start Of File "));
			}
			else
			{
				cc = FindInmr (comm_rec.co_no, local_rec.sitem, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.co_no, comm_rec.co_no);
					strcpy (inmr_rec.item_no, local_rec.sitem);
					cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				}
				if (cc)
					file_err (cc, inmr, "DBFIND");

				SuperSynonymError ();
				strcpy (local_rec.sitem, inmr_rec.item_no);
				strcpy (local_rec.sitemdesc, inmr_rec.description);
			}

			/*  end item */
			sprintf (local_rec.eitem, "%16.16s", ingu_rec.end_range);
			if (!strcmp (local_rec.eitem, "~~~~~~~~~~~~~~~~"))
			{
				strcpy (local_rec.eitemdesc, ML ("End Of File "));
			}
			else
			{
				cc = FindInmr (comm_rec.co_no, local_rec.eitem, 0L, "N");
				if (!cc)
				{
					strcpy (inmr_rec.co_no, comm_rec.co_no);
					strcpy (inmr_rec.item_no, local_rec.eitem);
					cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
				}
				if (cc)
					file_err (cc, inmr, "DBFIND");

				SuperSynonymError ();
				strcpy (local_rec.eitem, inmr_rec.item_no);
				strcpy (local_rec.eitemdesc, inmr_rec.description);
			}
		}
		else
		{
			sprintf (local_rec.sitem, "%16.16s", " ");
			sprintf (local_rec.eitem, "%16.16s", "~~~~~~~~~~~~~~~~");
			sprintf (local_rec.sitemdesc, "%34.34s", " ");
			sprintf (local_rec.eitemdesc, "%34.34s", " ");
		}
		return;
	}

	/*
	 * if choice is supplier
	 */
	if (local_rec.range [0] == 'P')
	{
		FLD ("ssupp")  = YES;
		FLD ("esupp")  = YES;
		FLD ("ssuppdesc") = NA;
		FLD ("esuppdesc") = NA;
		sprintf (local_rec.rangedesc, "%-13.13s", "Supplier");

		if (load)
		{
			sprintf (local_rec.ssupp, "%6.6s", ingu_rec.st_range);
			if (!strcmp (local_rec.ssupp, "      "))
			{
				strcpy (local_rec.ssuppdesc, ML ("Start Of File "));
			}
			else
			{
				cc = IntFindSumr (local_rec.ssupp, TRUE);
				if (cc)
					file_err (cc, sumr, "DBFIND");

				strcpy (local_rec.ssupp, sumr_rec.crd_no);
				strcpy (local_rec.ssuppdesc, sumr_rec.crd_name);
			}

			sprintf (local_rec.esupp, "%6.6s", ingu_rec.end_range);
			if (!strcmp (local_rec.esupp, "~~~~~~"))
			{
				strcpy (local_rec.esuppdesc, ML ("End Of File "));
			}
			else
			{
				cc = IntFindSumr (local_rec.esupp, TRUE);
				if (cc)
					file_err (cc, sumr, "DBFIND");

				strcpy (local_rec.esupp, sumr_rec.crd_no);
				strcpy (local_rec.esuppdesc, sumr_rec.crd_name);
			}
		}
		else
		{
			sprintf (local_rec.ssupp, "%6.6s", " ");
			sprintf (local_rec.esupp, "%6.6s", "~~~~~~");
			sprintf (local_rec.ssuppdesc, "%40.40s", " ");
			sprintf (local_rec.esuppdesc, "%40.40s", " ");
		}
		return;
	}
}

int
FindGrp (
	char 	*rec, 
	int 	Errors)
{
		sprintf (ingp_rec.code, "%-6.6s", rec);
		strcpy (ingp_rec.co_no, comm_rec.co_no);
		cc = find_rec (ingp, &ingp_rec, EQUAL, "r");
		if (cc && Errors 
			   && strncmp (rec, "~~~~~~", 6)
			   && strncmp (rec, "      ", 6))
		{
			if (ingp_rec.type [0] == 'B')
			{
				print_mess (ML (mlStdMess207));
			}
			else
			{
				print_mess (ML (mlStdMess208));
			}

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
}

int
IntFindSumr (
	char 	*rec, 
	int 	Errors)
{
		sprintf (sumr_rec.crd_no, "%6.6s", rec);
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		pad_num (sumr_rec.crd_no);
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc && Errors 
			   && strncmp (rec, "~~~~~~", 6)
			   && strncmp (rec, "      ", 6))
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS); 
}

void
SrchCcmr (
	char	*key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Description ");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br);
	sprintf (ccmr_rec.cc_no, "%-2.2s", key_val);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		   !strcmp	 (ccmr_rec.est_no, local_rec.br) &&
	       !strncmp (ccmr_rec.cc_no, key_val, strlen (key_val)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.br);
	sprintf (ccmr_rec.cc_no, "%-2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
SrchPocr (
	char 	*key_val)
{
	_work_open (3,0,40);
	save_rec ("#No", "#Description ");

	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pocrRec.co_no, comm_rec.co_no) &&
	       !strncmp (pocrRec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (pocrRec.code, pocrRec.description);
		if (cc)
			break;

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pocrRec.co_no, comm_rec.co_no);
	sprintf (pocrRec.code, "%-3.3s", temp_str);
	cc = find_rec (pocr, &pocrRec, EQUAL, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

void
SrchEsmr (
 char *key_val)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Branch Name");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
GetDesc (void)
{
	if (local_rec.roundtype [0] == 'U')
		sprintf (local_rec.typedesc, "%-7.7s", ML ("Up"));
	if (local_rec.roundtype [0] == 'D')
		sprintf (local_rec.typedesc, "%-7.7s", ML ("Down"));
	if (local_rec.roundtype [0] == 'N')
		sprintf (local_rec.typedesc, "%-7.7s", ML ("Nearest"));
}


/*
 * Routine to get price desctiptions from comm record. 
 */
const char	*
GetPriceDesc (
	int		priceNo)
{
	static	char	priceDesc [16];

	strcpy (priceDesc, " ");

	switch (priceNo)
	{
		case	1:	
			strcpy (priceDesc,	comm_rec.price1_desc);
			break;
		case	2:	
			strcpy (priceDesc, comm_rec.price2_desc);
			break;
		case	3:	
			strcpy (priceDesc, comm_rec.price3_desc);
			break;
		case	4:	
			strcpy (priceDesc, comm_rec.price4_desc);
			break;
		case	5:	
			strcpy (priceDesc, comm_rec.price5_desc);
			break;
		case	6:	
			strcpy (priceDesc, comm_rec.price6_desc);
			break;
		case	7:	
			strcpy (priceDesc, comm_rec.price7_desc);
			break;
		case	8:	
			strcpy (priceDesc, comm_rec.price8_desc);
			break;
		case	9:	
			strcpy (priceDesc, comm_rec.price9_desc);
			break;
	}
	return (priceDesc);
}
