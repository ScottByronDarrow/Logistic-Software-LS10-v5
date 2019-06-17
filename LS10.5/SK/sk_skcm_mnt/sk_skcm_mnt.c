/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_skcm_mnt.c,v 5.5 2002/07/17 09:58:00 scott Exp $
|  Program Name  : (sk_skcm_mnt.c    )                                |
|  Program Desc  : (Stock Container Master File Maintenance.      )   |
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : Dec 12th 2000    |
|---------------------------------------------------------------------|
| $Log: sk_skcm_mnt.c,v $
| Revision 5.5  2002/07/17 09:58:00  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2002/04/03 07:56:51  robert
| SC00787 - Updated to fixed display issue on LS10-GUI
|
| Revision 5.3  2002/02/07 10:34:50  kaarlo
| Fix "Print Vehicle Details" box alignment.
|
| Revision 5.2  2001/08/09 09:19:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:52  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:17:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/05/30 08:40:32  scott
| Updated to ensure weight = 4 decimal places and volume = 3 decimal places.
|
| Revision 4.0  2001/03/09 02:38:56  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.5  2000/12/20 04:07:24  ramon
| Updated to remove the errors when compiled in LS10-GUI.
|
| Revision 1.4  2000/12/19 04:10:09  scott
| Updated to add audit report.
|
| Revision 1.3  2000/12/13 03:00:57  scott
| Updated to change field name to container and length to 15
|
| Revision 1.2  2000/12/12 08:07:53  scott
| Updated to change working of Vehicle to Container.
|
| Revision 1.1  2000/12/12 07:30:38  scott
| New Program to maintain container master file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_skcm_mnt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_skcm_mnt/sk_skcm_mnt.c,v 5.5 2002/07/17 09:58:00 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <pr_format3.h>


#define PRN_FIELD(x)  		(vars [label(x)].prmpt)

		int		printerNumber	=	0;
		int		printingReport	=	FALSE;

#define	SCN_HEADER	1
#define	ST_SUCCESS	0
#define	ST_ERROR	1

#define	UPDATE		0
#define	LSL_IGNORE	1
#define	LSL_DELETE	2
#define	DEFAULT		99

typedef int BOOL;  

#include	"schema"

struct commRecord	comm_rec;
struct skcmRecord	skcm_rec;
struct skcsRecord	skcs_rec;

FILE	*fin, *fout;	/* Defines for pformat and pr_format			*/

char 	*data 	= "data";

MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
		  "" },
		{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
		  "" },
		{ " 3. DELETE RECORD.                     ",
		  "" }, 
		{ ENDMENU }
	};

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
} local_rec;

static	struct	var	vars[]	=	
{
	{SCN_HEADER, LIN, "containerCode", 2, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Container No.   : ", "Enter Container No. [SEARCH]", 
		NE, NO, JUSTLEFT, "", "", skcm_rec.container}, 
	{SCN_HEADER, LIN, "desc", 2, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description     : ", "Enter Container Description", 
		NO, NO, JUSTLEFT, "", "", skcm_rec.desc}, 
	{SCN_HEADER, LIN, "statusCode", 4, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Status Code     : ", "[SEARCH] to Search for valid Status Code.", 
		YES, NO, JUSTLEFT, "", "", skcm_rec.stat_code}, 
	{SCN_HEADER, LIN, "statusCodeDesc", 4, 60, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description     : ", "",
		NA, NO, JUSTLEFT, "", "", skcs_rec.desc}, 
	{SCN_HEADER, LIN, "min_wgt", 6, 2, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "Min Weight (kg) : ", "Enter Minimum Cargo Weight for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.min_wgt}, 
	{SCN_HEADER, LIN, "max_wgt", 6, 40, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "Max Weight (kg) : ", "Enter Maximum Cargo Weight for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.max_wgt}, 
	{SCN_HEADER, LIN, "std_wgt", 6, 80, FLOATTYPE, 
		"NNNNN.NNNN", "          ", 
		" ", " ", "Std Weight (kg) : ", "Enter Standard Cargo Weight for Container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.std_wgt}, 
	{SCN_HEADER, LIN, "min_vol", 7, 2, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "Min Volume(cu.m): ", "Enter Minimum cargo Volume for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.min_vol}, 
	{SCN_HEADER, LIN, "max_vol", 7, 40, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "Max Volume(cu.m): ", "Enter Maximum cargo Volume for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.max_vol}, 
	{SCN_HEADER, LIN, "std_vol", 7, 80, FLOATTYPE, 
		"NNNNNN.NNN", "          ", 
		" ", " ", "Std Volume(cu.m): ", "Enter Standard cargo Volume for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.std_vol}, 
	{SCN_HEADER, LIN, "min_hei", 9, 2, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Min Height (cm) : ", "Enter Minimum Cargo Height for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.min_hei}, 
	{SCN_HEADER, LIN, "max_hei", 9, 40, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Max Height (cm) : ", "Enter Maximum Cargo Height for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.max_hei}, 
	{SCN_HEADER, LIN, "std_hei", 9, 80, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Std Height (cm) : ", "Enter STandard Cargo Height for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.std_hei}, 
	{SCN_HEADER, LIN, "min_wid", 10, 2, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Min Width  (cm) : ", "Enter Minimum cargo Width for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.min_wid}, 
	{SCN_HEADER, LIN, "max_wid", 10, 40, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Max Width  (cm) : ", "Enter Maximum cargo Width for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.max_wid}, 
	{SCN_HEADER, LIN, "std_wid", 10, 80, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Std Width  (cm) : ", "Enter Standard cargo Width for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.std_wid}, 
	{SCN_HEADER, LIN, "min_dth", 11, 2, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Min Depth  (cm) : ", "Enter Minimum cargo Depth for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.min_dth}, 
	{SCN_HEADER, LIN, "max_dth", 11, 40, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Max Depth  (cm) : ", "Enter Maximum cargo Depth for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.max_dth}, 
	{SCN_HEADER, LIN, "std_dth", 11, 80, FLOATTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "Std Depth  (cm) : ", "Enter Standard cargo Depth for container", 
		NO, NO, JUSTRIGHT, "0", "9999999", (char *)&skcm_rec.std_dth}, 
	{SCN_HEADER, LIN, "per_wgt", 13, 2, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Kilo        : ", "Enter Rate Amount per Kilo", 
		NO, NO, JUSTRIGHT, "0", "999999999.99", (char *)&skcm_rec.per_wgt}, 
	{SCN_HEADER, LIN, "per_vol", 14, 2, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Cubic Meter : ", "Enter Rate Amount per Cubic Meter", 
		NO, NO, JUSTRIGHT, "0", "999999999.99", (char *)&skcm_rec.per_vol}, 
	{SCN_HEADER, LIN, "per_cont", 15, 2, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		" ", " ", "per Container   : ", "Enter Rate Amount Container", 
		NO, NO, JUSTRIGHT, "0", "999999999.99", (char *)&skcm_rec.per_cont}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

extern	int		TruePosition;
static int	newContainer = TRUE;


/*=======================
| Function Declarations |
=======================*/
static BOOL IsSpaces 			(char *);
int 	spec_valid 				(int);
int 	heading 				(int);
int		SkcmDelOk 				(void);
void 	shutdown_prog 			(void);
void 	CloseDB 				(void);
void 	OpenDB 					(void);
void	SrchSkcs 				(char *);
void 	SrchSkcm 				(char *);
void 	Update 					(void);
void	PrintContainerHeader	(void);
void	OpenPrintAudit			(void);
void	ClosePrintAudit			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int 
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR 	 (vars);
	init_scr 	 ();
	set_tty 	 ();
	OpenDB		 ();
	set_masks 	 ();
	
	TruePosition	=	TRUE;

    while (prog_exit == 0)
	{
		search_ok 		= 	TRUE;
		entry_exit 		= 	FALSE;
		edit_exit 		= 	FALSE;
		prog_exit 		= 	FALSE;
		restart 		= 	FALSE;
		newContainer	= 	TRUE;
		init_ok 		= 	TRUE;
		init_vars 	 (1);	
		heading 	 (1);
		entry 		 (1);   
		eoi_ok			=   TRUE;
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
	/*--------------------------------------------------------
	| Return TRUE if str contains only white space or nulls. |
	--------------------------------------------------------*/
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

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (skcm, skcm_list, SKCM_NO_FIELDS, "skcm_id_no");
	open_rec (skcs, skcs_list, SKCS_NO_FIELDS, "skcs_id_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (skcm);
	abc_fclose (skcs);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*----------------------------
	| Validate Container Number. |
	----------------------------*/
	if (LCHECK ("containerCode"))
	{
		if (SRCH_KEY)
        {
			SrchSkcm (temp_str);
			return (ST_SUCCESS);
		}

		if (dflt_used)
		{ 
			if (IsSpaces (skcm_rec.container))
			{
				/*-----------------------------
				| Container Number not found. |
				-----------------------------*/
				print_mess (ML (mlTrMess083));
				sleep (sleepTime);
				clear_mess ();
				return (ST_ERROR);
			}
		}
		strcpy (skcm_rec.co_no, comm_rec.co_no); 
		newContainer = find_rec (skcm,&skcm_rec, COMPARISON, "u");
        if (!newContainer)
		{
			strcpy (skcs_rec.co_no, comm_rec.co_no);
			strcpy (skcs_rec.code, skcm_rec.stat_code);
			cc = find_rec (skcs, &skcs_rec, COMPARISON, "r");
			if (cc)
			{
				strcpy (skcs_rec.code, " ");
				strcpy (skcs_rec.desc, " ");
			}
			DSP_FLD ("statusCode");
			DSP_FLD ("statusCodeDesc");
			entry_exit 	= 	TRUE;  
			return (ST_SUCCESS); 
		}
       	DSP_FLD ("containerCode");
       	DSP_FLD ("desc");
		return (ST_SUCCESS);
	}

	/*------------------------
	| Validate status code. |
	------------------------*/
	if (LCHECK ("statusCode"))
	{
		if (SRCH_KEY)
		{
	 		SrchSkcs (temp_str);
			return (ST_SUCCESS);
		}
			
		strcpy (skcs_rec.co_no, comm_rec.co_no);
		strcpy (skcs_rec.code, skcm_rec.stat_code);
		cc = find_rec (skcs, &skcs_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess080));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("statusCodeDesc");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate min_wgt. |
	-------------------*/
	if (LCHECK ("min_wgt"))
	{
		if (dflt_used)
			skcm_rec.min_wgt = (float) atof (prv_ntry);
		
		if (skcm_rec.max_wgt && skcm_rec.max_wgt < skcm_rec.min_wgt)
		{
			/*---------------------------------
			| Max Weight less than Min Weight |
			---------------------------------*/
			print_mess (ML (mlTrMess040));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("min_wgt");
		return (ST_SUCCESS);
	}

	/*-------------------
	| Validate max_wgt. |
	-------------------*/
	if (LCHECK ("max_wgt"))
	{
		if (dflt_used)
			skcm_rec.max_wgt = (float) atof (prv_ntry);
			
		if (skcm_rec.max_wgt && skcm_rec.max_wgt < skcm_rec.min_wgt)
		{
			/*----------------------------------
			| Max Weight less than Min Weight |
			----------------------------------*/
			print_mess (ML (mlTrMess040));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("max_wgt");
		return (ST_SUCCESS);
	}

	/*-------------------
	| Validate min_vol. |
	-------------------*/
	if (LCHECK ("min_vol"))
	{
		if (dflt_used)
			skcm_rec.min_vol = (float) atof (prv_ntry);
		
		if (skcm_rec.max_vol && skcm_rec.max_vol < skcm_rec.min_vol)
		{
			/*---------------------------------
			| Max Volume less than Min Volume |
			---------------------------------*/
			print_mess (ML (mlTrMess041));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("min_vol");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate max_vol. |
	-------------------*/
	if (LCHECK ("max_vol"))
	{
		if (dflt_used)
			skcm_rec.max_vol = (float) atof (prv_ntry);
			
		if (skcm_rec.max_vol && skcm_rec.max_vol < skcm_rec.min_vol)
		{
			/*---------------------------------
			| Max Volume less than Min Volume |
			---------------------------------*/
			print_mess (ML (mlTrMess041));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("max_vol");
		return (ST_SUCCESS);
	}

	/*-------------------
	| Validate min_hei. |
	-------------------*/
	if (LCHECK ("min_hei"))
	{
		if (dflt_used)
			skcm_rec.min_hei = (float) atof (prv_ntry);
		
		if (skcm_rec.max_hei && skcm_rec.max_hei < skcm_rec.min_hei)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess072));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("min_hei");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate max_hei. |
	-------------------*/
	if (LCHECK ("max_hei"))
	{
		if (dflt_used)
			skcm_rec.max_hei = (float) atof (prv_ntry);
			
		if (skcm_rec.max_hei && skcm_rec.max_hei < skcm_rec.min_hei)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess073));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("max_hei");
		return (ST_SUCCESS);
	}

	/*-------------------
	| Validate min_wid. |
	-------------------*/
	if (LCHECK ("min_wid"))
	{
		if (dflt_used)
			skcm_rec.min_wid = (float) atof (prv_ntry);
		
		if (skcm_rec.max_wid && skcm_rec.max_wid < skcm_rec.min_wid)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess074));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("min_wid");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate max_wid. |
	-------------------*/
	if (LCHECK ("max_wid"))
	{
		if (dflt_used)
			skcm_rec.max_wid = (float) atof (prv_ntry);
			
		if (skcm_rec.max_wid && skcm_rec.max_wid < skcm_rec.min_wid)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess075));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("max_wid");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate min_dth. |
	-------------------*/
	if (LCHECK ("min_dth"))
	{
		if (dflt_used)
			skcm_rec.min_dth = (float) atof (prv_ntry);
		
		if (skcm_rec.max_dth && skcm_rec.max_dth < skcm_rec.min_dth)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess077));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("min_dth");
		return (ST_SUCCESS);
	}
	/*-------------------
	| Validate max_dth. |
	-------------------*/
	if (LCHECK ("max_dth"))
	{
		if (dflt_used)
			skcm_rec.max_dth = (float) atof (prv_ntry);
			
		if (skcm_rec.max_dth && skcm_rec.max_dth < skcm_rec.min_dth)
		{
			/*---------------------------------
			| Max Height less than Min Height |
			---------------------------------*/
			print_mess (ML (mlTrMess078));
			sleep (sleepTime);
			return (ST_ERROR);
		}
		DSP_FLD ("max_dth");
		return (ST_SUCCESS);
	}

	/*--------------------------
	| Validate Rate per Month. |
	--------------------------*/
	if (LCHECK ("per_wgt"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (ST_SUCCESS);

		if (dflt_used)
			skcm_rec.per_wgt = atof (prv_ntry);

		if (skcm_rec.per_wgt == 0.00)
			return (ST_SUCCESS);

		skcm_rec.per_vol	=	0.00;
		skcm_rec.per_cont	=	0.00;
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		DSP_FLD ("per_cont");

		return (ST_SUCCESS);
	}

	/*------------------------
	| Validate Rate per Day. |
	------------------------*/
	if (LCHECK ("per_vol"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (ST_SUCCESS);

		if (dflt_used)
			skcm_rec.per_vol = atof (prv_ntry);

		if (skcm_rec.per_vol == 0.00)
			return (ST_SUCCESS);

		skcm_rec.per_wgt	=	0.00;
		skcm_rec.per_cont	=	0.00;
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		DSP_FLD ("per_cont");
		return (ST_SUCCESS);
	}

	/*------------------------------
	| Validate Rate per Container. |
	------------------------------*/
	if (LCHECK ("per_cont"))
	{
		if (prog_status == ENTRY && F_NOKEY (field))
			return (ST_SUCCESS);

		if (dflt_used)
			skcm_rec.per_cont = atof (prv_ntry);

		if (skcm_rec.per_cont == 0.00)
			return (ST_SUCCESS);

		skcm_rec.per_wgt	=	0.00;
		skcm_rec.per_vol	=	0.00;
		DSP_FLD ("per_cont");
		DSP_FLD ("per_wgt");
		DSP_FLD ("per_vol");
		return (ST_SUCCESS);
	}
	return (ST_SUCCESS);
}

void
SrchSkcs (
 char *key_val)
{
	_work_open (1,2,20);
	save_rec ("#CD", "#Code Description");
	strcpy (skcs_rec.co_no, comm_rec.co_no);
	sprintf (skcs_rec.code, "%-2.2s", key_val);

	cc = find_rec (skcs, &skcs_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (skcs_rec.co_no, comm_rec.co_no) &&
		   !strncmp (skcs_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (skcs_rec.code, skcs_rec.desc);
		if (cc)
			break;

		cc = find_rec (skcs, &skcs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (skcs_rec.desc, "%-40.40s", " ");
		return;
	}

	strcpy (skcs_rec.co_no, comm_rec.co_no);
	sprintf (skcs_rec.code, "%-2.2s", temp_str);
	cc = find_rec (skcs, &skcs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)skcs, "DBFIND");
}
/*==================
| Search for skcm. |
==================*/
void
SrchSkcm (
 char	*key_val)
{
	work_open ();
	save_rec ("#Container","#Description");
	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container,"%-15.15s",key_val);
	cc = find_rec (skcm,&skcm_rec,GTEQ,"r");
	while (!cc && !strcmp (skcm_rec.co_no, comm_rec.co_no) &&
	      		  !strncmp (skcm_rec.container,key_val,strlen (key_val)))
	{
		cc = save_rec (skcm_rec.container,skcm_rec.desc);
		if (cc)
			break;
		cc = find_rec (skcm,&skcm_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (skcm_rec.co_no, comm_rec.co_no);
	sprintf (skcm_rec.container,"%-15.15s", temp_str);
	cc = find_rec (skcm,&skcm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, (char *)skcm, "DBFIND");
}

void
Update (
 void)
{
	int		exitLoop;


	if (newContainer)
	{
		strcpy (skcm_rec.co_no,comm_rec.co_no);
		cc = abc_add (skcm,&skcm_rec);
		if (cc)
			file_err (cc, (char *)skcm, "DBADD");
	}
	else
	{
		exitLoop = FALSE;
		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N .   ",upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case UPDATE :
				cc = abc_update (skcm,&skcm_rec);
				if (cc)
					file_err (cc, (char *)skcm, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case LSL_IGNORE :
				abc_unlock (skcm);
				exitLoop = TRUE;
				break;
	
			case LSL_DELETE :
				if (SkcmDelOk ())
				{
					clear_mess ();
					cc = abc_delete (skcm);
					if (cc)
						file_err (cc, (char *)skcm, "DBUPDATE");
				}
				exitLoop = TRUE;
				break;
		
			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (skcm);

	/*----------------------
	| Open Printing Audit. |
	----------------------*/
	OpenPrintAudit ();

	/*-----------------------
	| Print Vehicle Header. |
	-----------------------*/
	PrintContainerHeader ();

	/*--------------------
	| Close Print Audit. |
	--------------------*/
	ClosePrintAudit ();
}

/*===========================
| Check whether it is OK to |
| delete the iuds record.   |
| Files checked are :       |
===========================*/
int
SkcmDelOk (
 void)
{
	/*-----------------------
	| Currently no checking |
	-----------------------*/
    return (EXIT_FAILURE);
}

int
heading (
 int	scn)
{
	char string[50] = "";

	if (!restart) 
	{
		/*---------------------------------------------
		| Transport Container Master File Maintenance |
		---------------------------------------------*/
		if (scn != cur_screen)
			scn_set (scn);

		sprintf (string, " %s ", ML (mlTrMess081));

		clear ();
		swide ();
		rv_pr (string, (132 - strlen (string))/2,0,1);

		box (0,1,132,14);
		move (1,3);
		line (131);
		move (1,5);
		line (131);
		move (1,8);
		line (131);
		move (1,12);
		line (131);

		/*-----------------
		| Container RATES |
		-----------------*/
		sprintf (string, " %s ", ML (mlTrMess079));
		rv_pr (string, (132 - strlen (string)) / 2,12,1);
 
        move (0,20);       
        line (132);
		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
        move (0,22);       
        line (132);
		scn_write (scn);
	}
    return (ST_SUCCESS);
}


/*=============================================
| Main heading routine for printing of audit. |
=============================================*/
void
OpenPrintAudit (void)
{
	int		key;

	/*--------------------------------------------------
	| Get Printer Selection for printing of container. |
	--------------------------------------------------*/
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

	if ((fin = pr_open ("sk_skcm_mnt.p")) == NULL)
		file_err (errno, "sk_skcm_mnt.p", "PR_OPEN");

	if ((fout = popen ("pformat", "w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".SO\n");
	fprintf (fout,".6\n");
	fprintf (fout,".L80\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".E%s\n", ML ("MASTER CONTAINER MAINTENANCE"));
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

/*=====================================
| Print Container Header Information. |
=====================================*/
void
PrintContainerHeader (void)
{
	if (printingReport == FALSE)
		return;

	pr_format (fin, fout, "RULETOP", 0,0);
	pr_format (fin, fout, "CONTHEAD2",1, PRN_FIELD ("containerCode"));
	pr_format (fin, fout, "CONTHEAD2",2, skcm_rec.container);
	pr_format (fin, fout, "CONTHEAD2",1, PRN_FIELD ("desc"));
	pr_format (fin, fout, "CONTHEAD2",2, skcm_rec.desc);
	pr_format (fin, fout, "CONTHEAD6",0, 0);
	pr_format (fin, fout, "CONTHEAD2",1, PRN_FIELD ("statusCode"));
	pr_format (fin, fout, "CONTHEAD2",2, skcm_rec.stat_code);
	pr_format (fin, fout, "CONTHEAD2",1, PRN_FIELD ("statusCodeDesc"));
	pr_format (fin, fout, "CONTHEAD2",2, skcs_rec.desc);

	pr_format (fin, fout, "CONTHEAD4",1, PRN_FIELD ("min_wgt"));
	pr_format (fin, fout, "CONTHEAD4",2, skcm_rec.min_wgt);
	pr_format (fin, fout, "CONTHEAD4",3, PRN_FIELD ("max_wgt"));
	pr_format (fin, fout, "CONTHEAD4",4, skcm_rec.max_wgt);

	pr_format (fin, fout, "CONTHEAD4",1, PRN_FIELD ("min_vol"));
	pr_format (fin, fout, "CONTHEAD4",2, skcm_rec.min_vol);
	pr_format (fin, fout, "CONTHEAD4",3, PRN_FIELD ("max_vol"));
	pr_format (fin, fout, "CONTHEAD4",4, skcm_rec.max_vol);

	pr_format (fin, fout, "CONTHEAD4",1, PRN_FIELD ("min_hei"));
	pr_format (fin, fout, "CONTHEAD4",2, skcm_rec.min_hei);
	pr_format (fin, fout, "CONTHEAD4",3, PRN_FIELD ("max_hei"));
	pr_format (fin, fout, "CONTHEAD4",4, skcm_rec.max_hei);

	pr_format (fin, fout, "CONTHEAD4",1, PRN_FIELD ("min_wid"));
	pr_format (fin, fout, "CONTHEAD4",2, skcm_rec.min_wid);
	pr_format (fin, fout, "CONTHEAD4",3, PRN_FIELD ("max_wid"));
	pr_format (fin, fout, "CONTHEAD4",4, skcm_rec.max_wid);

	pr_format (fin, fout, "CONTHEAD4",1, PRN_FIELD ("min_dth"));
	pr_format (fin, fout, "CONTHEAD4",2, skcm_rec.min_dth);
	pr_format (fin, fout, "CONTHEAD4",3, PRN_FIELD ("max_dth"));
	pr_format (fin, fout, "CONTHEAD4",4, skcm_rec.max_dth);

	pr_format (fin, fout, "CONTHEAD3",1, PRN_FIELD ("per_wgt"));
	pr_format (fin, fout, "CONTHEAD3",2, skcm_rec.per_wgt);
	pr_format (fin, fout, "CONTHEAD3",1, PRN_FIELD ("per_vol"));
	pr_format (fin, fout, "CONTHEAD3",2, skcm_rec.per_vol);
	pr_format (fin, fout, "CONTHEAD3",1, PRN_FIELD ("per_cont"));
	pr_format (fin, fout, "CONTHEAD3",2, skcm_rec.per_cont);
	pr_format (fin, fout, "RULETOP", 0,0);
}
