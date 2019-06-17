/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: vehimaint.c,v 5.5 2002/07/17 09:58:13 scott Exp $
|  Program Name  : (tr_vehimaint.c  )                                |
|  Program Desc  : (Transport Vehicle File Maintenance.          )   |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel.   | Date Written  : 07/12/95         |
|---------------------------------------------------------------------|
| $Log: vehimaint.c,v $
| Revision 5.5  2002/07/17 09:58:13  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/04/03 07:50:14  robert
| Updated to fixed display issue on LS10-GUI
|
| Revision 5.3  2002/02/08 02:38:38  kaarlo
| Fix "Print Vehicle Details" box alignment problem.
|
| Revision 5.2  2001/08/09 09:23:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:53:57  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:21:56  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/30 08:38:38  scott
| Updated ensure weight = four decimal and volume = 3 decimal places.
|
| Revision 4.0  2001/03/09 02:43:09  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/22 08:16:06  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/12/19 03:09:17  scott
| Updated to add add.schema, clean code and add printing option.
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: vehimaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_vehimaint/vehimaint.c,v 5.5 2002/07/17 09:58:13 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <GlUtils.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <pr_format3.h>
#include <get_lpno.h>

#define PRN_FIELD(x)  		 (vars [label (x)].prmpt)

extern	int	GV_link_cnt, GV_cur_level, GV_max_level;

#define	SCN_HEADER	1

typedef int BOOL;  

FILE	*fin, *fout;	/* Defines for pformat and pr_format			*/

#include	"schema"

struct commRecord	comm_rec;
struct trveRecord	trve_rec;
struct trcmRecord	trcm_rec;

char 	*data 	= "data",
		*glmr2 	= "glmr2";

		int		printerNumber	=	0;
		int		printingReport	=	FALSE;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
} local_rec;

char	loc_acc [MAXLEVEL + 1];

static	struct	var	vars []	=	
{
	{SCN_HEADER, LIN, "vehicle", 2, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Vehicle No.     : ", "Enter Vehicle No. [SEARCH]", 
		NE, NO, JUSTLEFT, "", "", trve_rec.ref}, 
	{SCN_HEADER, LIN, "desc", 3, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description     : ", "Enter Vehicle Description", 
		NO, NO, JUSTLEFT, "", "", trve_rec.desc}, 

	{SCN_HEADER, LIN, "carrierCode", 5, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Carrier Code    : ", " [SEARCH] to Search for valid carriers.", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_code}, 
	{SCN_HEADER, LIN, "carrierName", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", trcm_rec.carr_desc}, 
	{SCN_HEADER, LIN, "avail", 6, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Available (Y/N) : ", "Is Vehicle Available? (Y/N)", 
		NO, NO, JUSTLEFT, "YN", "", trve_rec.avail}, 
	{SCN_HEADER, LIN, "unav_res", 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Un-Avail.Reason : ", "Reason Vehicle is Un-available", 
		NO, NO, JUSTLEFT, "", "", trve_rec.unav_res}, 
	{SCN_HEADER, LIN, "typ", 8, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Vehicle Type    : ", "Enter Vehicle Type.", 
		NO, NO, JUSTLEFT, "", "", trve_rec.truck_type}, 
	{SCN_HEADER, LIN, "fr_cost", 8, 40, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "Freight Cost    : ", "Enter Freight Cost.", 
		NO, NO, JUSTRIGHT, "0", "999999999.99", (char *)&trve_rec.fr_chg}, 
	{SCN_HEADER, LIN, "min_wgt", 10, 2, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "Min Weight (kg) : ", "Enter Minimum Cargo Weight for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.min_wgt}, 
	{SCN_HEADER, LIN, "max_wgt", 10, 40, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "Max Weight (kg) : ", "Enter Maximum Cargo Weight for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.max_wgt}, 
	{SCN_HEADER, LIN, "min_vol", 11, 2, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "Min Volume (cu.m): ", "Enter Minimum cargo Volume for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.min_vol}, 
	{SCN_HEADER, LIN, "max_vol", 11, 40, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "Max Volume (cu.m): ", "Enter Maximum cargo Volume for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.max_vol}, 
	{SCN_HEADER, LIN, "min_hei", 12, 2, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Min Height (cm) : ", "Enter Minimum Cargo Height for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.min_hei}, 
	{SCN_HEADER, LIN, "max_hei", 12, 40, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Max Height (cm) : ", "Enter Maximum Cargo Height for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.max_hei}, 
	{SCN_HEADER, LIN, "min_wid", 13, 2, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Min Width (cm) : ", "Enter Minimum cargo Width for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.min_wid}, 
	{SCN_HEADER, LIN, "max_wid", 13, 40, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Max Width (cm) : ", "Enter Maximum cargo Width for vehicle", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&trve_rec.max_wid}, 
	{SCN_HEADER, LIN, "per_month", 15, 2, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Month       : ", "Enter Rate Amount per Month", 
		NO, NO, JUSTRIGHT, "", "", (char *)&trve_rec.per_month}, 
	{SCN_HEADER, LIN, "per_day", 15, 40, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Day         : ", "Enter Rate Amount per Day", 
		NO, NO, JUSTRIGHT, "", "", (char *)&trve_rec.per_day}, 
	{SCN_HEADER, LIN, "per_trip", 16, 2, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Trip        : ", "Enter Rate Amount per Trip", 
		NO, NO, JUSTRIGHT, "", "", (char *)&trve_rec.per_trip}, 
	{SCN_HEADER, LIN, "per_wgt", 16, 40, MONEYTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "per Kilo        : ", "Enter Rate Amount per Kilo", 
		NO, NO, JUSTRIGHT, "", "", (char *)&trve_rec.per_weight}, 
	{SCN_HEADER, LIN, "per_vol", 17, 2, MONEYTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "per Cubic Meter : ", "Enter Rate Amount per  Cubic Meter", 
		NO, NO, JUSTRIGHT, "", "", (char *)&trve_rec.per_vol}, 
	{SCN_HEADER, LIN, "acc_no", 19, 2, CHARTYPE, 
		GlMask, "                          ", 
		" ", " ", "G/L Account     : ", " ", 
		YES, NO, JUSTLEFT, "0123456789*-", "", loc_acc}, 
	{SCN_HEADER, LIN, "acc_desc", 19, 51, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "             ", 
		" ", "", "", " ", 
		NA, NO, JUSTLEFT, "", "", glmrRec.desc}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

extern	int		TruePosition;
static int	NewVehicle = TRUE;


/*=======================
| Function Declarations |
=======================*/
static BOOL IsSpaces (char *str);
void 	shutdown_prog (void);
void	PrintVehicleHeader		 (void);
void 	OpenDB (void);
void 	CloseDB (void);
int 	spec_valid (int);
int 	ResetFields (void);
void 	SrchTrcm (char *);
void 	SrchTrve (char *);
void 	Update (void);
void	OpenPrintAudit			 (void);
void	ClosePrintAudit			 (void);
int 	heading (int);


/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int argc, 
 char * argv [])
{
	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	OpenDB		 ();
	GL_SetMask	 (GlFormat);
	set_masks 	 ();
	clear		 ();
	
	TruePosition	=	TRUE;

    while (prog_exit == 0)
	{
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	FALSE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		NewVehicle	= 	TRUE;
		init_ok 	= 	TRUE;
		init_vars 	 (1);	
		heading 	 (1);
		entry 		 (1);   
		eoi_ok		=   TRUE;
		while (!restart && !prog_exit)
		{
			heading (1);
			scn_display (1);
			edit (1);

			if (!restart)
			{
				Update ();
				restart = TRUE;
			}
			else
				restart = TRUE;
		}
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

static BOOL
IsSpaces (
 char *str)
{ 
	/*-----------------------------
	| Return TRUE if str contains
	| only white space or nulls
	-----------------------------*/
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
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

/*======================= 
| Open data base files. |
======================= */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	abc_alias (glmr2, glmr);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (trve, trve_list, TRVE_NO_FIELDS, "trve_id_no");
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");

	OpenGlmr ();
	open_rec (glmr2, glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (trve);
	abc_fclose (trcm);
	GL_Close ();
	abc_fclose (glmr2);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------
	| Validate Vehicle Number. |
	--------------------------*/
	if (LCHECK ("vehicle"))
	{
		if (SRCH_KEY)
        {
			SrchTrve (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{ 
			if (IsSpaces (trve_rec.ref))
			{
				/*---------------------------
				| Vehicle Number not found. |
				---------------------------*/
				print_mess (ML (mlStdMess218));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (trve_rec.co_no, comm_rec.co_no); 
		strcpy (trve_rec.br_no, comm_rec.est_no); 
		NewVehicle = find_rec (trve,&trve_rec, COMPARISON, "u");
        if (!NewVehicle)
		{
			glmrRec.hhmr_hash = trve_rec.hhmr_hash;
			cc = find_rec (glmr2, &glmrRec,EQUAL, "r");
			if (!cc)
			{
				sprintf (loc_acc, "%-*.*s", MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
				DSP_FLD ("acc_no");
				DSP_FLD ("acc_desc");
			}
			trve_rec.max_wgt	=	trve_rec.cap;

			abc_selfield (trcm, "trcm_trcm_hash");
			trcm_rec.trcm_hash	=	trve_rec.trcm_hash;
			cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (trcm_rec.carr_code, " ");
				trcm_rec.trcm_hash	=	0L;
				strcpy (trcm_rec.carr_desc, " ");
			}
			abc_selfield (trcm, "trcm_id_no");
			DSP_FLD ("vehicle");
			DSP_FLD ("carrierCode");
			DSP_FLD ("carrierName");
			entry_exit 	= 	TRUE;  
			return (EXIT_SUCCESS); 
		}
       	DSP_FLD ("vehicle");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate carrier code. |
	------------------------*/
	if (LCHECK ("carrierCode"))
	{
		if (SRCH_KEY)
		{
	 		SrchTrcm (temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (trcm_rec.co_no, comm_rec.co_no);
		strcpy (trcm_rec.br_no, comm_rec.est_no);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "u");
		if (cc)
		{
			sprintf (err_str, ML (mlTrMess068), trcm_rec.carr_code);
			errmess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------
	| Validate Available. |
	---------------------*/
	if (LCHECK ("avail"))
	{
		FLD ("unav_res") = (trve_rec.avail [0] == 'Y') ? NA : YES;
		return (EXIT_SUCCESS);
	}
	
	/*-------------------
	| Validate min_wgt. |
	-------------------*/
	if (LCHECK ("min_wgt"))
	{
		if (dflt_used)
			trve_rec.min_wgt = atof (prv_ntry);
		
		if (trve_rec.max_wgt && trve_rec.max_wgt < trve_rec.min_wgt)
		{
			/*---------------------------------
			| Max Weight less than Min Weight |
			---------------------------------*/
			print_mess (ML (mlTrMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("min_wgt");
		return (EXIT_SUCCESS);
	}


	/*-------------------
	| Validate max_wgt. |
	-------------------*/
	if (LCHECK ("max_wgt"))
	{
		if (dflt_used)
			trve_rec.max_wgt = atof (prv_ntry);
			
		if (trve_rec.max_wgt && trve_rec.max_wgt < trve_rec.min_wgt)
		{
			/*----------------------------------
			| Max Weight less than Min Weight |
			----------------------------------*/
			print_mess (ML (mlTrMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("max_wgt");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate min_vol. |
	-------------------*/
	if (LCHECK ("min_vol"))
	{
		if (dflt_used)
			trve_rec.min_vol = atof (prv_ntry);
		
		if (trve_rec.max_vol && trve_rec.max_vol < trve_rec.min_vol)
		{
			/*---------------------------------
			| Max Volume less than Min Volume |
			---------------------------------*/
			print_mess (ML (mlTrMess041));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("min_vol");
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate max_vol. |
	-------------------*/
	if (LCHECK ("max_vol"))
	{
		if (dflt_used)
			trve_rec.max_vol = atof (prv_ntry);
			
		if (trve_rec.max_vol && trve_rec.max_vol < trve_rec.min_vol)
		{
			/*---------------------------------
			| Max Volume less than Min Volume |
			---------------------------------*/
			print_mess (ML (mlTrMess041));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("max_vol");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate min_hei. |
	-------------------*/
	if (LCHECK ("min_hei"))
	{
		if (dflt_used)
			trve_rec.min_hei = atof (prv_ntry);
		
		if (trve_rec.max_hei && trve_rec.max_hei < trve_rec.min_hei)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess072));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("min_hei");
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate max_hei. |
	-------------------*/
	if (LCHECK ("max_hei"))
	{
		if (dflt_used)
			trve_rec.max_hei = atof (prv_ntry);
			
		if (trve_rec.max_hei && trve_rec.max_hei < trve_rec.min_hei)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess073));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("max_hei");
		return (EXIT_SUCCESS);
	}

	/*-------------------
	| Validate min_wid. |
	-------------------*/
	if (LCHECK ("min_wid"))
	{
		if (dflt_used)
			trve_rec.min_wid = atof (prv_ntry);
		
		if (trve_rec.max_wid && trve_rec.max_wid < trve_rec.min_wid)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess074));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("min_wid");
		return (EXIT_SUCCESS);
	}
	/*-------------------
	| Validate max_wid. |
	-------------------*/
	if (LCHECK ("max_wid"))
	{
		if (dflt_used)
			trve_rec.max_wid = atof (prv_ntry);
			
		if (trve_rec.max_wid && trve_rec.max_wid < trve_rec.min_wid)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess075));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("max_wid");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Rate per Month. |
	--------------------------*/
	if (LCHECK ("per_month"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			trve_rec.per_month = atof (prv_ntry);

		if (trve_rec.per_month	< 	0.00)
		{
			/*-----------------------------------------
			| Rate per Month cannot be less than Zero |
			-----------------------------------------*/
			print_mess (ML (mlTrMess042));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.per_month == 0.00)
			return (ResetFields ());

		trve_rec.per_day	=	0.00;
		trve_rec.per_trip	=	0.00;
		trve_rec.per_weight	=	0.00;
		trve_rec.per_vol	=	0.00;
		DSP_FLD ("per_month");
		DSP_FLD ("per_day");
		DSP_FLD ("per_trip");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		FLD ("per_day")		=	NI;
		FLD ("per_trip")	=	NI;
		FLD ("per_wgt")		=	NI;
		FLD ("per_vol")		=	NI;
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Rate per Day. |
	------------------------*/
	if (LCHECK ("per_day"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			trve_rec.per_day = atof (prv_ntry);

		if (trve_rec.per_day	< 	0.00)
		{
			/*----------------------------------------
			| Rate per Day cannot be less than Zero |
			----------------------------------------*/
			print_mess (ML (mlTrMess043));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.per_day == 0.00)
			return (ResetFields ());

		trve_rec.per_month	=	0.00;
		trve_rec.per_trip	=	0.00;
		trve_rec.per_weight	=	0.00;
		trve_rec.per_vol	=	0.00;
		DSP_FLD ("per_month");
		DSP_FLD ("per_day");
		DSP_FLD ("per_trip");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		FLD ("per_month")	=	NI;
		FLD ("per_trip")	=	NI;
		FLD ("per_wgt")		=	NI;
		FLD ("per_vol")		=	NI;
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Rate per Trip. |
	-------------------------*/
	if (LCHECK ("per_trip"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			trve_rec.per_trip = atof (prv_ntry);

		if (trve_rec.per_trip	< 	0.00)
		{
			/*----------------------------------------
			| Rate per Trip cannot be less than Zero |
			----------------------------------------*/
			print_mess (ML (mlTrMess044));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.per_trip == 0.00)
			return (ResetFields ());

		trve_rec.per_month	=	0.00;
		trve_rec.per_day	=	0.00;
		trve_rec.per_weight	=	0.00;
		trve_rec.per_vol	=	0.00;
		DSP_FLD ("per_month");
		DSP_FLD ("per_day");
		DSP_FLD ("per_trip");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		FLD ("per_month")	=	NI;
		FLD ("per_day")		=	NI;
		FLD ("per_wgt")		=	NI;
		FLD ("per_vol")		=	NI;
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Rate per Weight. |
	---------------------------*/
	if (LCHECK ("per_wgt"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			trve_rec.per_weight = atof (prv_ntry);

		if (trve_rec.per_weight	< 	0.00)
		{
			/*------------------------------------------
			| Rate per Weight cannot be less than Zero |
			------------------------------------------*/
			print_mess (ML (mlTrMess049));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.per_weight == 0.00)
			return (ResetFields ());

		trve_rec.per_month	=	0.00;
		trve_rec.per_day	=	0.00;
		trve_rec.per_trip	=	0.00;
		trve_rec.per_vol	=	0.00;
		DSP_FLD ("per_month");
		DSP_FLD ("per_day");
		DSP_FLD ("per_trip");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		FLD ("per_month")	=	NI;
		FLD ("per_day")		=	NI;
		FLD ("per_trip")	=	NI;
		FLD ("per_vol")		=	NI;
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate Rate per Volume. |
	---------------------------*/
	if (LCHECK ("per_vol"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (dflt_used)
			trve_rec.per_vol = atof (prv_ntry);

		if (trve_rec.per_vol	< 	0.00)
		{
			/*------------------------------------------
			| Rate per Volume cannot be less than Zero |
			------------------------------------------*/
			print_mess (ML (mlTrMess048));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (trve_rec.per_vol == 0.00)
			return (ResetFields ());

		trve_rec.per_day	=	0.00;
		trve_rec.per_trip	=	0.00;
		trve_rec.per_weight	=	0.00;
		trve_rec.per_month	=	0.00;
		DSP_FLD ("per_month");
		DSP_FLD ("per_day");
		DSP_FLD ("per_trip");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		FLD ("per_month")	=	NI;
		FLD ("per_day")		=	NI;
		FLD ("per_trip")	=	NI;
		FLD ("per_wgt")		=	NI;
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate General Ledger Account Number. |
	-----------------------------------------*/
	if (LCHECK ("acc_no")) 
	{
		if (SRCH_KEY)
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));

		strcpy (glmrRec.co_no, comm_rec.co_no);
		GL_FormAccNo (loc_acc, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			/*--------------------
			| Account not found. |
			--------------------*/
			print_mess (ML (mlStdMess186));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'F' ||
		    glmrRec.glmr_class [2][0] != 'P')
		{
			/*-------------------------------------------------
			| Account MUST be of a 'Financial Posting' Class. |
			-------------------------------------------------*/
			print_mess (ML (mlTrMess045));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("acc_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ResetFields (
 void)
{
	FLD ("per_month")		=	YES;
	FLD ("per_day")			=	YES;
	FLD ("per_trip")			=	YES;
	FLD ("per_wgt")			=	YES;
	FLD ("per_vol")			=	YES;
    return (EXIT_SUCCESS);
}

/*=====================
| Search for Carrier. |
=====================*/
void
SrchTrcm (
 char	*key_val)
{
	_work_open (4,0,40);
	save_rec ("#No. ","#Carrier  Description ");

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code,"%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.co_no) &&
		      	  !strcmp (trcm_rec.br_no, comm_rec.est_no) &&
		      	  !strncmp (trcm_rec.carr_code, key_val,strlen (key_val)))
	{
		cc = save_rec (trcm_rec.carr_code, trcm_rec.carr_desc);
		if (cc)
			break;

		cc = find_rec (trcm, &trcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trcm_rec.co_no, comm_rec.co_no);
	strcpy (trcm_rec.br_no, comm_rec.est_no);
	sprintf (trcm_rec.carr_code,"%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)trcm, "DBFIND");
}
/*==================
| Search for trve. |
==================*/
void
SrchTrve (
 char	*key_val)
{
	_work_open (10,0,40);
	save_rec ("#Vehicle #","#Description");
	strcpy (trve_rec.co_no, comm_rec.co_no);
    strcpy (trve_rec.br_no, comm_rec.est_no); 
	sprintf (trve_rec.ref,"%-10.10s",key_val);
	cc = find_rec (trve,&trve_rec,GTEQ,"r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.co_no) &&
			      !strcmp (trve_rec.br_no, comm_rec.est_no) &&
	      		  !strncmp (trve_rec.ref,key_val,strlen (key_val)))
	{
		cc = save_rec (trve_rec.ref,trve_rec.desc);
		if (cc)
			break;
		cc = find_rec (trve,&trve_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.co_no);
    strcpy (trve_rec.br_no, comm_rec.est_no); 
	sprintf (trve_rec.ref,"%-10.10s", temp_str);
	cc = find_rec (trve,&trve_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)trve, "DBFIND");
}

void
Update (
 void)
{
	/*----------------------
	| Open Printing Audit. |
	----------------------*/
	OpenPrintAudit ();

	clear ();
	strcpy (trve_rec.co_no,comm_rec.co_no);
	strcpy (trve_rec.br_no,comm_rec.est_no);
	trve_rec.hhmr_hash	=	glmrRec.hhmr_hash;
	trve_rec.trcm_hash	=	trcm_rec.trcm_hash;
	trve_rec.cap		=	trve_rec.max_wgt; 

	/*----------------------------------
	| Update existing Vehicle Record. |
	----------------------------------*/
	if (!NewVehicle)
	{
		cc = abc_update (trve,&trve_rec);
		if (cc)
			file_err (cc, (char *)trve, "DBUPDATE");
	}
	else 
	{
		cc = abc_add (trve,&trve_rec);
		if (cc)
			file_err (cc, (char *)trve, "DBADD");

		abc_unlock (trve);
	}
	/*-----------------------
	| Print Vehicle Header. |
	-----------------------*/
	PrintVehicleHeader ();

	/*--------------------
	| Close Print Audit. |
	--------------------*/
	ClosePrintAudit ();
}

/*=============================================
| Main heading routine for printing of audit. |
=============================================*/
void
OpenPrintAudit (void)
{
	int		key;

	/*-------------------------------------------------
	| Get Printer Selection for printing of Contract. |
	-------------------------------------------------*/
	strcpy (err_str, ML ("Enter 'P' to print Vehicle details"));

#ifdef GVISION
	heading (SCN_HEADER);
	scn_display (SCN_HEADER);
	key = PauseForKey (22,0, err_str, 0);
#else
	key = PauseForKey (23,0, err_str, 0);
#endif

	if ((key == 'P' || key == 'p') && !printerNumber)
		printerNumber = get_lpno (0);

	if (printerNumber && (key == 'P' || key == 'p'))
		printingReport	=	TRUE;
	else
		printingReport	=	FALSE;

	if (printingReport == FALSE)
		return;

	if ((fin = pr_open ("tr_vehimaint.p")) == NULL)
		file_err (errno, "tr_vehimaint.p", "PR_OPEN");

	if ((fout = popen ("pformat", "w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".SO\n");
	fprintf (fout,".6\n");
	fprintf (fout,".L80\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".E%s\n", ML ("MASTER VEHICLE MAINTENANCE"));
	fprintf (fout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (fout,".B2\n");
	fprintf (fout,".E%s %s : ", ML ("Branch"), clip (comm_rec.est_name));
	fprintf (fout,"%s %s\n", ML ("Warehouse"), clip (comm_rec.cc_name));
}

/*=====================================================
| Routine to close the print audit trail output file. |
=====================================================*/
void
ClosePrintAudit (void)
{
	if (printingReport == FALSE)
		return;

	fprintf (fout,".EOF\n");
	pclose (fout);
	fclose (fin);
}

/*====================================
| Print Contract Header Information. |
====================================*/
void
PrintVehicleHeader (void)
{
	if (printingReport == FALSE)
		return;

	pr_format (fin, fout, "RULETOP", 0,0);
	pr_format (fin, fout, "VEHIHEAD2",1, PRN_FIELD ("vehicle"));
	pr_format (fin, fout, "VEHIHEAD2",2, trve_rec.ref);
	pr_format (fin, fout, "VEHIHEAD2",1, PRN_FIELD ("desc"));
	pr_format (fin, fout, "VEHIHEAD2",2, trve_rec.desc);
	pr_format (fin, fout, "VEHIHEAD6",0, 0);
	pr_format (fin, fout, "VEHIHEAD5",1, PRN_FIELD ("carrierCode"));
	pr_format (fin, fout, "VEHIHEAD5",2, trcm_rec.carr_code);
	pr_format (fin, fout, "VEHIHEAD5",3, trcm_rec.carr_desc);
	pr_format (fin, fout, "VEHIHEAD6",0, 0);
	pr_format (fin, fout, "VEHIHEAD2",1, PRN_FIELD ("avail"));
	pr_format (fin, fout, "VEHIHEAD2",2, trve_rec.avail);
	pr_format (fin, fout, "VEHIHEAD2",1, PRN_FIELD ("unav_res"));
	pr_format (fin, fout, "VEHIHEAD2",2, trve_rec.unav_res);
	pr_format (fin, fout, "VEHIHEAD6",0, 0);
	pr_format (fin, fout, "VEHIHEAD2",1, PRN_FIELD ("typ"));
	pr_format (fin, fout, "VEHIHEAD2",2, trve_rec.truck_type);

	pr_format (fin, fout, "VEHIHEAD6",0, 0);
	pr_format (fin, fout, "VEHIHEAD3",1, PRN_FIELD ("fr_cost"));
	pr_format (fin, fout, "VEHIHEAD3",2, trve_rec.fr_chg);

	pr_format (fin, fout, "VEHIHEAD4",1, PRN_FIELD ("min_wgt"));
	pr_format (fin, fout, "VEHIHEAD4",2, trve_rec.min_wgt);
	pr_format (fin, fout, "VEHIHEAD4",3, PRN_FIELD ("max_wgt"));
	pr_format (fin, fout, "VEHIHEAD4",4, trve_rec.max_wgt);

	pr_format (fin, fout, "VEHIHEAD4",1, PRN_FIELD ("min_vol"));
	pr_format (fin, fout, "VEHIHEAD4",2, trve_rec.min_vol);
	pr_format (fin, fout, "VEHIHEAD4",3, PRN_FIELD ("max_vol"));
	pr_format (fin, fout, "VEHIHEAD4",4, trve_rec.max_vol);

	pr_format (fin, fout, "VEHIHEAD4",1, PRN_FIELD ("min_hei"));
	pr_format (fin, fout, "VEHIHEAD4",2, trve_rec.min_hei);
	pr_format (fin, fout, "VEHIHEAD4",3, PRN_FIELD ("max_hei"));
	pr_format (fin, fout, "VEHIHEAD4",4, trve_rec.max_hei);

	pr_format (fin, fout, "VEHIHEAD4",1, PRN_FIELD ("min_wid"));
	pr_format (fin, fout, "VEHIHEAD4",2, trve_rec.min_wid);
	pr_format (fin, fout, "VEHIHEAD4",3, PRN_FIELD ("max_wid"));
	pr_format (fin, fout, "VEHIHEAD4",4, trve_rec.max_wid);

	pr_format (fin, fout, "VEHIHEAD3",1, PRN_FIELD ("per_month"));
	pr_format (fin, fout, "VEHIHEAD3",2, trve_rec.per_month);
	pr_format (fin, fout, "VEHIHEAD3",1, PRN_FIELD ("per_day"));
	pr_format (fin, fout, "VEHIHEAD3",2, trve_rec.per_day);
	pr_format (fin, fout, "VEHIHEAD3",1, PRN_FIELD ("per_trip"));
	pr_format (fin, fout, "VEHIHEAD3",2, trve_rec.per_trip);
	pr_format (fin, fout, "VEHIHEAD3",1, PRN_FIELD ("per_wgt"));
	pr_format (fin, fout, "VEHIHEAD3",2, trve_rec.per_weight);

	pr_format (fin, fout, "VEHIHEAD5",1, PRN_FIELD ("acc_no"));
	pr_format (fin, fout, "VEHIHEAD5",2, loc_acc);
	pr_format (fin, fout, "VEHIHEAD5",3, glmrRec.desc);
	pr_format (fin, fout, "RULETOP", 0,0);
}
int
heading (
 int	scn)
{
	char string [50] = "";

	if (!restart) 
	{
		/*-------------------------------------------
		| Transport Vehicle Master File Maintenance |
		-------------------------------------------*/
		if (scn != cur_screen)
			scn_set (scn);

		sprintf (string, " %s ", ML (mlTrMess047));

		clear ();
		rv_pr (string, (80 - strlen (string))/2,0,1);

		box (0,1,80,18);
		move (1,4);
		line (79);
		move (1,9);
		line (79);
		move (1,14);
		line (79);
		move (1,18);
		line (79);

		/*--------------
		| RENTAL RATES |
		--------------*/
		rv_pr (ML (mlTrMess046), 26,14,1);
 
		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_short);
		print_at (21,29,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_short);
		print_at (21,53,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_short);
        move (0,22);       
        line (80);
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}
