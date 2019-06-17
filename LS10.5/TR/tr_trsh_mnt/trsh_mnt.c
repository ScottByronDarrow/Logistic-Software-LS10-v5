/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( tr_trsh_mnt.c )                                  |
|  Program Desc  : ( Transport Zone Maintenance.                  )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (10/03/1999)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: trsh_mnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_trsh_mnt/trsh_mnt.c,v 5.3 2002/07/25 11:17:39 scott Exp $";

#define		TABLINES	12
#define		MAXSCNS		1
#include <pslscr.h>
#include <DateToString.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <minimenu.h>
#include <pDate.h>

#define	MAX_DISPLAY_TRIPS	8

	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct tag_commRecord
	{
		int		termno;
		char	co_no [3];
		char	co_name [41];
		char	es_no [3];
		char	es_name [41];
	} comm_rec;

	/*=============================+
	 | TRansport Zone Maintenance. |
	 +=============================*/
#define	TRZM_NO_FIELDS	6

	struct dbview	trzm_list [TRZM_NO_FIELDS] =
	{
		{"trzm_co_no"},
		{"trzm_br_no"},
		{"trzm_del_zone"},
		{"trzm_desc"},
		{"trzm_dflt_chg"},
		{"trzm_trzm_hash"}
	};

	struct tag_trzmRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	del_zone [7];
		char	desc [41];
		double	dflt_chg;
		long	trzm_hash;
	}	trzm_rec;

	/*==========================================+
	 | TRansport Timeslot Capacity Maintenance. |
	 +==========================================*/
#define	TRZC_NO_FIELDS	4

	struct dbview	trzc_list [TRZC_NO_FIELDS] =
	{
		{"trzc_trzm_hash"},
		{"trzc_del_dcode"},
		{"trzc_time_slot"},
		{"trzc_capacity"}
	};

	struct tag_trzcRecord
	{
		long	trzm_hash;
		int		del_dcode;
		char	time_slot [2];
		float	capacity;
	}	trzc_rec;

	/*==================================+
	 | TRansport Zone Time Maintenance. |
	 +==================================*/
#define	TRZT_NO_FIELDS	4

	struct dbview	trzt_list [TRZT_NO_FIELDS] =
	{
		{"trzt_co_no"},
		{"trzt_time_code"},
		{"trzt_start_time"},
		{"trzt_end_time"}
	};

	struct tag_trztRecord
	{
		char	co_no [3];
		char	time_code [2];
		long	start_time;
		long	end_time;
	}	trzt_rec;

	/*=========================+
	 | TRansport SHedule file. |
	 +=========================*/
#define	TRSH_NO_FIELDS	7

	struct dbview	trsh_list [TRSH_NO_FIELDS] =
	{
		{"trsh_trzm_hash"},
		{"trsh_del_date"},
		{"trsh_sdel_slot"},
		{"trsh_edel_slot"},
		{"trsh_hhco_hash"},
		{"trsh_hhso_hash"},
		{"trsh_hhit_hash"}
	};

	struct tag_trshRecord
	{
		long	trzm_hash;
		Date	del_date;
		char	sdel_slot [2];
		char	edel_slot [2];
		long	hhco_hash;
		long	hhso_hash;
		long	hhit_hash;
	}	trsh_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	7

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_del_zone"},
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_name [41];
		char	dbt_acronym [10];
		char	del_zone [7];
	}	cumr_rec;

	/*==============================================+
	 | Customer P-slip/ Invoice/Credit Header File. |
	 +==============================================*/
#define	COHR_NO_FIELDS	13

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_del_zone"},
		{"cohr_del_req"},
		{"cohr_del_date"},
		{"cohr_s_timeslot"},
		{"cohr_e_timeslot"},
		{"cohr_hhco_hash"},
		{"cohr_status"},
		{"cohr_stat_flag"},
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	inv_no [9];
		long	hhcu_hash;
		char	type [2];
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	s_timeslot [2];
		char	e_timeslot [2];
		long	hhco_hash;
		char	status [2];
		char	stat_flag [2];
	}	cohr_rec;

	/*==========================+
	 | Sales Order Header File. |
	 +==========================*/
#define	SOHR_NO_FIELDS	12

	struct dbview	sohr_list [SOHR_NO_FIELDS] =
	{
		{"sohr_co_no"},
		{"sohr_br_no"},
		{"sohr_order_no"},
		{"sohr_hhcu_hash"},
		{"sohr_hhso_hash"},
		{"sohr_del_zone"},
		{"sohr_del_req"},
		{"sohr_del_date"},
		{"sohr_s_timeslot"},
		{"sohr_e_timeslot"},
		{"sohr_status"},
		{"sohr_stat_flag"}
	};

	struct tag_sohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	order_no [9];
		long	hhcu_hash;
		long	hhso_hash;
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	s_timeslot [2];
		char	e_timeslot [2];
		char	status [2];
		char	stat_flag [2];
	}	sohr_rec;

	/*=================================+
	 | Inventory Transfer Header File. |
	 +=================================*/
#define	ITHR_NO_FIELDS	11

	struct dbview	ithr_list [ITHR_NO_FIELDS] =
	{
		{"ithr_co_no"},
		{"ithr_type"},
		{"ithr_del_no"},
		{"ithr_hhit_hash"},
		{"ithr_tran_ref"},
		{"ithr_del_zone"},
		{"ithr_del_req"},
		{"ithr_del_date"},
		{"ithr_s_timeslot"},
		{"ithr_e_timeslot"},
		{"ithr_stat_flag"}
	};

	struct tag_ithrRecord
	{
		char	co_no [3];
		char	type [2];
		long	del_no;
		long	hhit_hash;
		char	tran_ref [17];
		char	del_zone [7];
		char	del_req [2];
		Date	del_date;
		char	s_timeslot [2];
		char	e_timeslot [2];
		char	stat_flag [2];
	}	ithr_rec;

char	systemDate[11];

char	*data  = "data",
		*cohr  = "cohr",
		*sohr  = "sohr",
		*cumr  = "cumr",
		*ithr  = "ithr",
		*trzm  = "trzm",
		*trzc  = "trzc",
		*trsh  = "trsh",
		*trzt  = "trzt";

#include	<tr_schedule.h>
	
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11],
			defaultDate [11],
			defaultDelZone [7],
			defaultStartSlot [2],
			defaultEndSlot [2],
			del_dcode [2],
			del_dcode_desc [12],
			startTimeSlot [2],
			endTimeSlot [2],
			documentNo [9];
	long	desiredDate,
			startTimeLeft,
			startTimeRight,
			endTimeLeft,
			endTimeRight,
			invoiceHash,
			orderHash,
			transferHash;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "docNumber", 2, 2, CHARTYPE, 
		"UUUUUUUU", "        ", 
		"0", "0",  "Document Number        : ", "Enter packing slip, Collection Note or Transfer  number <default all> ", 
		NE, NO, JUSTRIGHT, "", "", local_rec.documentNo}, 
	{1, LIN, "desiredDate", 	2, 60, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.defaultDate,   "Desired Delivery Date : ", " Enter Desired Delivery. Default is Date today.", 
		 YES, NO,  JUSTLEFT, " ", "", (char *)&local_rec.desiredDate}, 
	{1, LIN, "deliveryZoneCode",	 3, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", local_rec.defaultDelZone, "Delivery Zone Code     : ", "Enter Delivery Zone Code [SEARCH]. ",
		 NE, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{1, LIN, "deliveryZoneDesc",	 3, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Zone Desc.   : ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{1, LIN, "startTimeSlot",	 4, 2, CHARTYPE,
		"U", "          ",
		" ", local_rec.defaultStartSlot, "Start Time Slot        : ", "Enter Start Time Slot. [SEARCH]. ",
		 YES, NO, JUSTLEFT, "A", "X", local_rec.startTimeSlot},
	{1, LIN, "startTimeLeft",	 4, 30, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start-", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeLeft},
	{1, LIN, "endTimeLeft",	 4, 45, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "End-", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeLeft},
	{1, LIN, "endTimeSlot",	 4, 60, CHARTYPE,
		"U", "          ",
		" ", local_rec.defaultEndSlot, "End   Time Slot       : ", "Enter End Time Slot. [SEARCH]. ",
		 YES, NO, JUSTLEFT, "A", "X", local_rec.endTimeSlot},
	{1, LIN, "startTimeRight",	 4, 87, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "Start-", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.startTimeRight},
	{1, LIN, "endTimeRight",	 4, 102, TIMETYPE,
		"NN:NN", "          ",
		" ", "", "End-", " ",
		NA, NO,  JUSTLEFT, "0", "1440", (char *)&local_rec.endTimeRight},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/* where are these located? */
extern	int	TruePosition;
/*extern	int	EnvScreenOK;   */

int		knowInvoice		=	FALSE;
int		knowCredit		=	FALSE;
int		knowTransfer	=	FALSE;
int		knowOrder		=	FALSE;
char	runningType [3];
long	runningHash	=	0L;
int		recordLocked	=	FALSE;
int		dspOpen			=	FALSE;

/*=======================
| Function Declarations |
=======================*/
static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
int spec_valid (int);
void SrchTimeZone (void);
void ShowDocumentNumber (char *);
void SearchSohr (char *, char *);
void SearchCohr (char *, char *, char *);
void SearchIthr (char *);
void SrchTrzm (char *);
void TimeSlotFunc (void);
static void update (void);
int GetAvailableDeliveries (long, long, char *);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);
	init_scr ();
	swide ();
	set_tty ();
	set_masks ();

	TruePosition	=	TRUE;

	tab_row	=	7;
	tab_col	=	2;
	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate()));
	strcpy (local_rec.defaultDate, systemDate);

	if (argc > 2)
	{
		if (argc == 4)
			recordLocked	=	TRUE;

		sprintf (runningType, "%-1.1s", argv[1]);
		runningHash	=	atol (argv [2]);
		FLD ("docNumber") = NA;
	
		if (runningType[0] == 'T')
			knowTransfer =	TRUE;
		if (runningType[0] == 'P' || runningType[0] == 'I')
			knowInvoice	 =	TRUE;
		if (runningType[0] == 'N' || runningType[0] == 'C')
			knowCredit	 =	TRUE;
		if (runningType[0] == 'O')
			knowOrder	 =	TRUE;
	}

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit 	= FALSE; 
		edit_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		local_rec.invoiceHash	=	0L;
		local_rec.transferHash	=	0L;

		if (dspOpen)
		{
			Dsp_close ();
			dspOpen = FALSE;
		}

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		scn_write (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		update ();

		if (runningHash)
			break;

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
static void 
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static void 
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");
	open_rec (trzt, trzt_list, TRZT_NO_FIELDS, "trzt_id_no");
	open_rec (trzc, trzc_list, TRZC_NO_FIELDS, "trzc_id_no");
	open_rec (trsh, trsh_list, TRSH_NO_FIELDS, "trsh_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
static void 
CloseDB (
 void)
{
	abc_fclose (trzm);
	abc_fclose (trzc);
	abc_fclose (trzt);
	abc_fclose (trsh);
	abc_fclose (cohr);
	abc_fclose (sohr);
	abc_fclose (ithr);
	abc_fclose (cumr);

	abc_dbclose (data);
}

int 
spec_valid (
 int field)
{
	/*-------------------------------
	| Validate pack Slip Number.	|
	-------------------------------*/
	if (LCHECK("docNumber")) 
	{
		if (F_NOKEY (label ("docNumber") ))
		{
			if (knowInvoice || knowCredit)
			{
				abc_selfield (cohr, "cohr_hhco_hash");
				cohr_rec.hhco_hash	=	runningHash;
				cc = find_rec (cohr, &cohr_rec, COMPARISON, "r");
				if (!cc)
				{
					local_rec.invoiceHash	=	cohr_rec.hhco_hash;
					strcpy (local_rec.defaultDate, systemDate);
					if (cohr_rec.del_date > 0L)
						strcpy (local_rec.defaultDate, DateToString (cohr_rec.del_date));
					strcpy (local_rec.defaultStartSlot,cohr_rec.s_timeslot);
					strcpy (local_rec.defaultEndSlot, cohr_rec.e_timeslot);
					strcpy (local_rec.defaultDelZone, cohr_rec.del_zone);
					strcpy (local_rec.documentNo, cohr_rec.inv_no);
					DSP_FLD ("docNumber");
					abc_selfield (cohr, "cohr_id_no2");
				}
				skip_entry	=	goto_field (field, label ("desiredDate"));
				FLD ("docNumber") = YES;
			}
			if (knowOrder)
			{
				abc_selfield (sohr, "sohr_hhso_hash");
				sohr_rec.hhso_hash	=	runningHash;
				cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
				if (!cc)
				{
					local_rec.orderHash	=	sohr_rec.hhso_hash;
					strcpy (local_rec.defaultDate, systemDate);
					if (sohr_rec.del_date > 0L)
						strcpy (local_rec.defaultDate, DateToString (sohr_rec.del_date));
					strcpy (local_rec.defaultStartSlot,sohr_rec.s_timeslot);
					strcpy (local_rec.defaultEndSlot, sohr_rec.e_timeslot);
					strcpy (local_rec.defaultDelZone, sohr_rec.del_zone);
					
					strcpy (local_rec.documentNo, sohr_rec.order_no);
					DSP_FLD ("docNumber");
					abc_selfield (sohr, "sohr_id_no2");
				}
				skip_entry	=	goto_field (field, label ("desiredDate"));
				FLD ("docNumber") = YES;
			}
			if (knowTransfer)
			{
				abc_selfield (ithr, "ithr_hhit_hash");
				ithr_rec.hhit_hash	=	runningHash;
				cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
				if (!cc)
				{
					local_rec.transferHash	=	ithr_rec.hhit_hash;
	
					strcpy (local_rec.defaultDate, systemDate);
					if (ithr_rec.del_date > 0L)
						strcpy (local_rec.defaultDate, DateToString (ithr_rec.del_date));
					strcpy (local_rec.defaultDelZone, ithr_rec.del_zone);
					strcpy (local_rec.defaultStartSlot, ithr_rec.s_timeslot);
					strcpy (local_rec.defaultEndSlot, ithr_rec.e_timeslot);
					sprintf (local_rec.documentNo,"%08ld",ithr_rec.del_no);
					DSP_FLD ("docNumber");
					abc_selfield (ithr, "ithr_id_no");
				}
				skip_entry	=	goto_field (field, label ("desiredDate"));
				FLD ("docNumber") = YES;
			}
			if (strcmp (local_rec.defaultStartSlot, " "))
				FLD ("startTimeSlot") = NI;
			if (strcmp (local_rec.defaultEndSlot,   " "))
				FLD ("endTimeSlot") = NI;
			if (strcmp (local_rec.defaultDelZone,  "      "))
			{
				FLD ("deliveryZoneCode") = NI;
				strcpy (trzm_rec.del_zone, local_rec.defaultDelZone);
			}

			DSP_FLD ("startTimeSlot");
			DSP_FLD ("endTimeSlot");
			DSP_FLD ("deliveryZoneCode");
			DSP_FLD ("deliveryZoneDesc");
			FLD ("docNumber") = NI;
			return (EXIT_SUCCESS);
		}
		local_rec.invoiceHash	=	0L;
		local_rec.orderHash		=	0L;
		local_rec.transferHash	=	0L;

		if (SRCH_KEY)
		{
			ShowDocumentNumber(temp_str);
			return(0);
		}

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		strcpy (cohr_rec.co_no,comm_rec.co_no);
		strcpy (cohr_rec.br_no,comm_rec.es_no);
		strcpy (cohr_rec.type,"T");
		sprintf (cohr_rec.inv_no, zero_pad (local_rec.documentNo, 8));
		cc = find_rec ("cohr",&cohr_rec,COMPARISON,"r");
		if (cc) 
		{
			strcpy (cohr_rec.co_no,comm_rec.co_no);
			strcpy (cohr_rec.br_no,comm_rec.es_no);
			strcpy (cohr_rec.type,"N");
			sprintf (cohr_rec.inv_no, zero_pad (local_rec.documentNo,8));
			cc = find_rec ("cohr",&cohr_rec,COMPARISON,"r");
			if (cc)
			{
				strcpy (sohr_rec.co_no,comm_rec.co_no);
				strcpy (sohr_rec.br_no,comm_rec.es_no);
				sprintf (sohr_rec.order_no,zero_pad(local_rec.documentNo,8));
				cc = find_rec ("sohr",&sohr_rec,COMPARISON,"r");
				if (cc)
				{
					strcpy (ithr_rec.co_no, comm_rec.co_no);
					strcpy (ithr_rec.type,  "T");
					ithr_rec.del_no	=	atol (local_rec.documentNo);
					cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
					if (cc)
					{
						print_mess(ML(mlTrMess035));
						sleep (sleepTime);
						clear_mess ();
						return(1); 
					}
					else
						local_rec.transferHash	=	ithr_rec.hhit_hash;
				}
				else
					local_rec.orderHash	=	sohr_rec.hhso_hash;
			}
			else
				local_rec.invoiceHash	=	cohr_rec.hhco_hash;
		}
		else
			local_rec.invoiceHash	=	cohr_rec.hhco_hash;

		if ((local_rec.invoiceHash && cohr_rec.del_req [0] == 'N') ||
		    (local_rec.orderHash && sohr_rec.del_req [0] == 'N') ||
		    (local_rec.transferHash && ithr_rec.del_req [0] == 'N'))
		{
			sprintf (err_str, ML(mlTrMess064), local_rec.documentNo);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return(1); 
		}
		if (local_rec.orderHash && sohr_rec.status [0] == 'P') 
		{
			sprintf (err_str, ML(mlTrMess065), local_rec.documentNo);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return(1); 
		}

		strcpy (local_rec.defaultDate, systemDate);
		if (local_rec.invoiceHash && cohr_rec.del_date > 0L)
		{
			strcpy (local_rec.defaultDate, 		DateToString (cohr_rec.del_date));
			strcpy (local_rec.defaultDelZone, 	cohr_rec.del_zone);
			strcpy (local_rec.defaultStartSlot, cohr_rec.s_timeslot);
			strcpy (local_rec.defaultEndSlot, 	cohr_rec.e_timeslot);
		}

		if (local_rec.transferHash && ithr_rec.del_date > 0L)
		{
			strcpy (local_rec.defaultDate, 		DateToString (ithr_rec.del_date));
			strcpy (local_rec.defaultDelZone, 	ithr_rec.del_zone);
			strcpy (local_rec.defaultStartSlot, ithr_rec.s_timeslot);
			strcpy (local_rec.defaultEndSlot, 	ithr_rec.e_timeslot);
		}
		if (local_rec.orderHash && sohr_rec.del_date > 0L)
		{
			strcpy (local_rec.defaultDate, 		DateToString (sohr_rec.del_date));
			strcpy (local_rec.defaultDelZone, 	sohr_rec.del_zone);
			strcpy (local_rec.defaultStartSlot, sohr_rec.s_timeslot);
			strcpy (local_rec.defaultEndSlot, 	sohr_rec.e_timeslot);
		}

		return(0);
	}
	/*----------------------------
	| Validate Reference Number. |
	----------------------------*/
	if (LCHECK ("deliveryZoneCode"))
	{
		if (F_NOKEY (label ("deliveryZoneCode") ) && dflt_used)
			strcpy (trzm_rec.del_zone, local_rec.defaultDelZone);

		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (trzm_rec.co_no, comm_rec.co_no);
		strcpy (trzm_rec.br_no, comm_rec.es_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			if (local_rec.invoiceHash)
			{
				cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
				cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
				if (!cc)
					strcpy (trzm_rec.del_zone, cumr_rec.del_zone);

				cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			}
			if (local_rec.orderHash)
			{
				cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
				cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
				if (!cc)
					strcpy (trzm_rec.del_zone, cumr_rec.del_zone);

				cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
			}
			if (cc)
			{
				print_mess(ML(mlTrMess059));
				sleep (sleepTime);
				clear_mess ();
				return(1); 
			}
		}
		if (prog_status != ENTRY)
			TimeSlotFunc ();

		DSP_FLD ("deliveryZoneDesc");
		return(0);
	}
	if (LCHECK ("desiredDate"))
	{
		if (prog_status != ENTRY)
			TimeSlotFunc ();

		return (EXIT_SUCCESS);
	}
	/*------------------
	| Start Time Slot. |
	------------------*/
	if (LCHECK ("startTimeSlot"))
	{
		if (F_NOKEY (label ("startTimeSlot") ) && dflt_used)
			strcpy (local_rec.startTimeSlot, local_rec.defaultStartSlot);

		if (SRCH_KEY)
		{
			SrchTimeZone ();
			sprintf (local_rec.startTimeSlot, "%-1.1s", temp_str);
			return (EXIT_SUCCESS);
		}	

		strcpy (trzt_rec.co_no, comm_rec.co_no);
		strcpy (trzt_rec.time_code, local_rec.startTimeSlot);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlTrMess061));
			sleep (sleepTime);
			clear_mess ();
			return(1); 
		}
		local_rec.startTimeLeft	=	trzt_rec.start_time;
		local_rec.endTimeLeft	=	trzt_rec.end_time;

		DSP_FLD ("startTimeLeft");
		DSP_FLD ("endTimeLeft");

		if (prog_status != ENTRY)
			TimeSlotFunc ();
		return (EXIT_SUCCESS);
	}
	/*----------------
	| End Time Slot. |
	----------------*/
	if (LCHECK ("endTimeSlot"))
	{
		if (F_NOKEY (label ("endTimeSlot") ) && dflt_used)
			strcpy (local_rec.endTimeSlot, local_rec.defaultEndSlot);

		if (SRCH_KEY)
		{
			SrchTimeZone ();
			sprintf (local_rec.endTimeSlot, "%-1.1s", temp_str);
			return (EXIT_SUCCESS);
		}	
		strcpy (trzt_rec.co_no, comm_rec.co_no);
		strcpy (trzt_rec.time_code, local_rec.endTimeSlot);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlTrMess061));
			sleep (sleepTime);
			clear_mess ();
			return(1); 
		}
		local_rec.startTimeRight	=	trzt_rec.start_time;
		local_rec.endTimeRight		=	trzt_rec.end_time;

		DSP_FLD ("startTimeRight");
		DSP_FLD ("endTimeRight");

		TimeSlotFunc ();
		return (EXIT_SUCCESS);
	}
	return(0);
}

void
SrchTimeZone (
 void)
{
	int		i;
	work_open();
	save_rec("#C","#Time Slot Description.");
	for (i = 0;strlen (TCN [i].time_code);i++)
	{
		sprintf (err_str,"%s - %s", TCN [i].time_start, TCN [i].time_end);
		cc = save_rec(TCN  [i].time_code, err_str);
		if (cc)
			break;
	}
	cc = disp_srch();
	work_close();
}
/*==========================
| Search for p_slip number |
==========================*/
void
ShowDocumentNumber (
 char	*key_val)
{
	work_open();
	save_rec("#Document No","#Document Description.");

	SearchCohr (key_val, "T", "PS");
	SearchCohr (key_val, "N", "CN");
	SearchSohr (key_val, "SO");
	SearchIthr (key_val);

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (temp_str, temp_str + 3);
}

void
SearchSohr (
 char	*key_val,
 char	*Code)
{
	char	searchDesc [51];

	strcpy (sohr_rec.co_no,comm_rec.co_no);
	strcpy (sohr_rec.br_no,comm_rec.es_no);
	sprintf (sohr_rec.order_no,"%-8.8s",key_val);

	cc = find_rec (sohr,&sohr_rec,GTEQ,"r");
	while (!cc && !strncmp(sohr_rec.order_no,key_val,strlen(key_val)) && 
				  !strcmp (sohr_rec.co_no,comm_rec.co_no) && 
				  !strcmp (sohr_rec.br_no,comm_rec.es_no))
	{
		if (sohr_rec.del_req [0] == 'N' || sohr_rec.status [0] == 'P')
		{
			cc = find_rec (sohr, &sohr_rec, NEXT,"r");
			continue;
		}
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cumr", "DBFIND");

		sprintf (searchDesc, "%s - %s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		sprintf (err_str, "%-2.2s %s", Code, sohr_rec.order_no);
		cc = save_rec (err_str, searchDesc);
		if (cc)
			break;
		
		cc = find_rec (sohr,&sohr_rec,NEXT,"r");
	}
}

void
SearchCohr (
 char	*key_val,
 char	*Type,
 char	*Code)
{
	char	searchDesc [51];

	strcpy (cohr_rec.co_no,comm_rec.co_no);
	strcpy (cohr_rec.br_no,comm_rec.es_no);
	sprintf (cohr_rec.inv_no,"%-8.8s",key_val);
	sprintf (cohr_rec.type, Type);

	cc = find_rec (cohr,&cohr_rec,GTEQ,"r");
	while (!cc && !strncmp(cohr_rec.inv_no,key_val,strlen(key_val)) && 
				  !strcmp (cohr_rec.co_no,comm_rec.co_no) && 
				  !strcmp (cohr_rec.br_no,comm_rec.es_no) &&
				  cohr_rec.type[0] == Type[0])
	{
		if (cohr_rec.del_req [0] == 'N')
		{
			cc = find_rec (cohr, &cohr_rec, NEXT,"r");
			continue;
		}
		cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
		cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cumr", "DBFIND");

		sprintf (searchDesc, "%s - %s", cumr_rec.dbt_no, cumr_rec.dbt_name);
		sprintf (err_str, "%-2.2s %s", Code, cohr_rec.inv_no);
		cc = save_rec (err_str, searchDesc);
		if (cc)
			break;
		
		cc = find_rec (cohr,&cohr_rec,NEXT,"r");
	}
}

void
SearchIthr (
 char	*key_val)
{
	char	searchDesc [51];

	strcpy (ithr_rec.co_no,comm_rec.co_no);
	ithr_rec.del_no	=	atol (key_val);
	strcpy (ithr_rec.type, "T");

	cc = find_rec (ithr,&ithr_rec,GTEQ,"r");
	while (!cc && !strcmp (ithr_rec.co_no,comm_rec.co_no) && 
				  ithr_rec.type[0] == 'T')
	{
		if (ithr_rec.del_req [0] == 'N')
		{
			cc = find_rec (ithr, &ithr_rec, NEXT,"r");
			continue;
		}
		sprintf (err_str, "TR %08ld", ithr_rec.del_no);
		sprintf (searchDesc, "%-50.50s", ithr_rec.tran_ref);
		cc = save_rec (err_str, searchDesc);
		if (cc)
			break;
		
		cc = find_rec (ithr,&ithr_rec,NEXT,"r");
	}
}
/*=========================
| Search for Zome Master. |
=========================*/
void 
SrchTrzm (
 char *key_val)
{
	work_open ();

	save_rec("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.es_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.co_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.es_no) &&
				  !strncmp (trzm_rec.del_zone, key_val, strlen (key_val)))
	{
		cc = save_rec (trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;
		
		cc = find_rec (trzm, &trzm_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.co_no);
	strcpy (trzm_rec.br_no, comm_rec.es_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");

	return;
}

void
TimeSlotFunc (
 void)
{
	int		enoughRecords	=	TRUE;
	int		i;
	char	disp_str [256];
	int		noCapacity	=	TRUE;
	int		dayWeek,
			noTrips	=	0,
			tripsUsed,
			tripLoop;

	long	startDate;
	char	workStartTime [6],
			workEndTime [6],
			startAmPmFlag[3],
			endAmPmFlag[3];

	char	displayDays	 [MAX_DISPLAY_TRIPS] [11],
			displayDates [MAX_DISPLAY_TRIPS] [11];
	int		displayDOW	 [MAX_DISPLAY_TRIPS];
	char	displayTimeSlot [26] [36];
	char	displayAvail [26][161];
	int		noDeliveried [26];

	if (dspOpen)
	{
		Dsp_close ();
		dspOpen = FALSE;
	}

	startDate	=	local_rec.desiredDate;

	trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
	trzc_rec.del_dcode	=	0;
	strcpy (trzc_rec.time_slot, " ");
	cc = find_rec (trzc, &trzc_rec, GTEQ, "r");
	if (cc || trzc_rec.trzm_hash != trzm_rec.trzm_hash)
	{
		Dsp_open (1, 5, 12);
		dspOpen = TRUE;

		sprintf (disp_str, "    TIME   SLOT      | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s ",
			" ", " ", " ", " ", " ", " ", " ", " ");
		Dsp_saverec (disp_str);
		Dsp_saverec ("");
		Dsp_saverec ("");
		Dsp_saverec (ML (mlTrMess066));
		Dsp_srch ();
		return;
	}

	while (enoughRecords)
	{
		dayWeek	=	DayOfWeek (startDate) + 1;
		trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
		trzc_rec.del_dcode	=	dayWeek;
		strcpy (trzc_rec.time_slot, " ");
		cc = find_rec (trzc, &trzc_rec, GTEQ, "r");
		if (!cc && trzc_rec.trzm_hash == trzm_rec.trzm_hash &&
				   trzc_rec.del_dcode == dayWeek)
		{
			DateToFmtString (startDate, "%A", displayDays [noTrips]);
			displayDOW	 [noTrips]	=	dayWeek;
			strcpy (displayDates [noTrips++], DateToString (startDate));
		}
		if (noTrips >= MAX_DISPLAY_TRIPS)
			break;

		startDate++;
	}
	if (!noTrips)
	{
		Dsp_open (1, 5, 12);
		dspOpen = TRUE;

		Dsp_saverec (" ");
		Dsp_saverec (ML (mlTrMess066));
		Dsp_saverec (" ");
		Dsp_srch ();
		return;
	}
	
	Dsp_open (1, 5, 12);
	dspOpen = TRUE;

	sprintf (disp_str, "    TIME   SLOT      | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s | %-10.10s ",
			displayDays [ 0 ], displayDays [ 1 ], displayDays [ 2 ],
			displayDays [ 3 ], displayDays [ 4 ], displayDays [ 5 ],
			displayDays [ 6 ], displayDays [ 7 ]);
	Dsp_saverec (disp_str);
	sprintf (disp_str, "                     | %10.10s | %10.10s | %10.10s | %10.10s | %10.10s | %10.10s | %10.10s | %10.10s ",
			displayDates [ 0 ], displayDates [ 1 ], displayDates [ 2 ],
			displayDates [ 3 ], displayDates [ 4 ], displayDates [ 5 ],
			displayDates [ 6 ], displayDates [ 7 ]);
	Dsp_saverec (disp_str);
	Dsp_saverec ("");

	for (i = 0; strlen (TCN [i].time_code); i++)
	{
		noDeliveried [i] = TRUE;
		strcpy (trzt_rec.co_no, comm_rec.co_no);
		strcpy (trzt_rec.time_code, TCN [i].time_code);
		cc = find_rec (trzt, &trzt_rec, COMPARISON, "r");
		if (cc)
		{
			trzt_rec.start_time	=	0L;
			trzt_rec.end_time	=	0L;
		}
		strcpy (startAmPmFlag, "AM");
		strcpy (endAmPmFlag, "AM");
		if (trzt_rec.start_time > 719)
		{
			trzt_rec.start_time -= 720;
			strcpy (startAmPmFlag, "PM");
		}
		if (trzt_rec.end_time > 719)
		{
			trzt_rec.end_time -= 720;
			strcpy (endAmPmFlag, "PM");
		}
		strcpy (workStartTime, ttoa (trzt_rec.start_time, "NN:NN"));
		strcpy (workEndTime, ttoa (trzt_rec.end_time, "NN:NN"));

		sprintf ( displayTimeSlot [i], "[%s] %s%s - %s%s",
						trzt_rec.time_code,
						workStartTime, 
						startAmPmFlag,
						workEndTime,
						endAmPmFlag
		);

		strcpy (displayAvail [i], "");
		for (tripLoop = 0; tripLoop < noTrips; tripLoop++)
		{
			trzc_rec.capacity = -1.00;
			trzc_rec.trzm_hash	=	trzm_rec.trzm_hash;
			trzc_rec.del_dcode	=	displayDOW [tripLoop];
			strcpy (trzc_rec.time_slot, trzt_rec.time_code);
			cc = find_rec (trzc, &trzc_rec, COMPARISON, "r");
			if (cc)
				trzc_rec.capacity = -2.00;

			
			startDate	=	StringToDate (displayDates [tripLoop]);
			tripsUsed	=	GetAvailableDeliveries
							( 
								trzm_rec.trzm_hash,
								startDate,
								trzt_rec.time_code
							);

			if (trzc_rec.capacity == -1.00)
				strcpy (err_str, " No Zone.   ^E");
			else if (trzc_rec.capacity == -2.00)
				strcpy (err_str, " No T.Slot  ^E");
			else if (trzc_rec.capacity == 0.00)
				strcpy (err_str, "            ^E");
			else
			{
				noDeliveried [i] = FALSE;
				if (tripsUsed > 0.0)
				{
					sprintf (err_str, "%4.0f / ^2%-4.0f^6 ^E", trzc_rec.capacity, trzc_rec.capacity - tripsUsed);
				}
				else
					sprintf (err_str, "%4.0f / ^1%-4.0f^6 ^E", trzc_rec.capacity, trzc_rec.capacity - tripsUsed);
			}
	
			strcat (displayAvail[i], err_str);
		}
	
	}
	for (i = 0; strlen (TCN [i].time_code); i++)
	{
		if (!noDeliveried [i])
		{
			sprintf (disp_str, "%s^E%s", displayTimeSlot [i], displayAvail [i]);
			Dsp_saverec (disp_str);
			noCapacity	=	FALSE;
		}
	}
	if (noCapacity)
	{
		Dsp_saverec (" ");
		Dsp_saverec (ML (mlTrMess066));
		Dsp_saverec (" ");
	}
	Dsp_srch ();
	return;
}
/*==================
| Updated records. |
==================*/
static void 
update (
 void)
{

	if (local_rec.invoiceHash)
	{
		abc_selfield (trsh, "trsh_hhco_hash");
		trsh_rec.hhco_hash	=	local_rec.invoiceHash;
		cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		while (!cc && trsh_rec.hhco_hash == local_rec.invoiceHash)
		{
			abc_delete (trsh);
			trsh_rec.hhco_hash	=	local_rec.invoiceHash;
			cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		}
	}
	if (local_rec.transferHash)
	{
		abc_selfield (trsh, "trsh_hhit_hash");
		trsh_rec.hhit_hash	=	local_rec.transferHash;
		cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		while (!cc && trsh_rec.hhit_hash == local_rec.transferHash)
		{
			abc_delete (trsh);
			trsh_rec.hhit_hash	=	local_rec.transferHash;
			cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		}
	}
	if (local_rec.orderHash)
	{
		abc_selfield (trsh, "trsh_hhso_hash");
		trsh_rec.hhso_hash	=	local_rec.orderHash;
		cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		while (!cc && trsh_rec.hhso_hash == local_rec.invoiceHash)
		{
			abc_delete (trsh);
			trsh_rec.hhso_hash	=	local_rec.orderHash;
			cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
		}
	}

	trsh_rec.hhco_hash	=	local_rec.invoiceHash;
	trsh_rec.hhso_hash	=	local_rec.orderHash;
	trsh_rec.hhit_hash	=	local_rec.transferHash;
	cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
	while (!cc)
	{
		abc_delete (trsh);
		trsh_rec.hhco_hash	=	local_rec.invoiceHash;
		trsh_rec.hhso_hash	=	local_rec.orderHash;
		trsh_rec.hhit_hash	=	local_rec.transferHash;
		cc = find_rec (trsh, &trsh_rec, COMPARISON, "r");
	}
	trsh_rec.hhco_hash	=	local_rec.invoiceHash;
	trsh_rec.hhso_hash	=	local_rec.orderHash;
	trsh_rec.hhit_hash	=	local_rec.transferHash;
	trsh_rec.trzm_hash	=	trzm_rec.trzm_hash;
	trsh_rec.del_date	=	local_rec.desiredDate;
	strcpy (trsh_rec.sdel_slot, local_rec.startTimeSlot);
	strcpy (trsh_rec.edel_slot, local_rec.endTimeSlot);
	cc = abc_add (trsh, &trsh_rec);
	if (cc)
		file_err (cc, "trsh", "DBADD");

	if (local_rec.invoiceHash && !recordLocked)
	{
		abc_selfield (cohr, "cohr_hhco_hash");
		cohr_rec.hhco_hash	=	local_rec.invoiceHash;
		cc = find_rec (cohr, &cohr_rec, COMPARISON, "u");
		if (!cc)
		{
			cohr_rec.del_date	=	local_rec.desiredDate;
			strcpy (cohr_rec.del_zone, trzm_rec.del_zone);
			strcpy (cohr_rec.s_timeslot, local_rec.startTimeSlot);
			strcpy (cohr_rec.e_timeslot, local_rec.endTimeSlot);
			cc = abc_update (cohr, &cohr_rec);
			if (cc)
				file_err (cc, cohr, "DBUPDATE");
		}
		else
			abc_unlock (cohr);

		abc_selfield (cohr, "cohr_id_no2");
	}
	if (local_rec.orderHash && !recordLocked)
	{
		abc_selfield (sohr, "sohr_hhso_hash");
		sohr_rec.hhso_hash	=	local_rec.orderHash;
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
		if (!cc)
		{
			sohr_rec.del_date	=	local_rec.desiredDate;
			strcpy (sohr_rec.del_zone, trzm_rec.del_zone);
			strcpy (sohr_rec.s_timeslot, local_rec.startTimeSlot);
			strcpy (sohr_rec.e_timeslot, local_rec.endTimeSlot);
			cc = abc_update (sohr, &sohr_rec);
			if (cc)
				file_err (cc, sohr, "DBUPDATE");
		}
		else
			abc_unlock (sohr);

		abc_selfield (sohr, "sohr_id_no2");
	}

	if (local_rec.transferHash && !recordLocked)
	{
		abc_selfield (ithr, "ithr_hhit_hash");
		ithr_rec.hhit_hash	=	local_rec.transferHash;
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
		if (!cc)
		{
			ithr_rec.del_date	=	local_rec.desiredDate;
			strcpy (ithr_rec.s_timeslot, local_rec.startTimeSlot);
			strcpy (ithr_rec.e_timeslot, local_rec.endTimeSlot);
			strcpy (ithr_rec.del_zone, trzm_rec.del_zone);
			cc = abc_update (ithr, &ithr_rec);
			if (cc)
				file_err (cc, ithr, "DBUPDATE");
		}
		else
			abc_unlock (ithr);

		abc_selfield (ithr, "ithr_id_no");
	}
	abc_selfield (trsh, "trsh_id_no");
}

/*===================================================================
| Get available deliveries for a zone, delivery date and time slot. |
===================================================================*/
int
GetAvailableDeliveries (
 long	trzmHash,
 long	deliveryDate,
 char	*deliverySlot)
{
	int		noDeliveries	=	0;
	
	trsh_rec.trzm_hash	=	trzmHash;
	trsh_rec.del_date	=	deliveryDate;
	sprintf (trsh_rec.sdel_slot, "%-1.1s", " ");
	cc = find_rec (trsh, &trsh_rec, GTEQ, "r");
	while (!cc && trsh_rec.trzm_hash == trzmHash &&
				  trsh_rec.del_date	== deliveryDate)
	{                       
		if (strcmp (deliverySlot, trsh_rec.sdel_slot) >= 0 &&
			strcmp (deliverySlot, trsh_rec.edel_slot) <= 0) 
			noDeliveries++;
		cc = find_rec (trsh, &trsh_rec, NEXT, "r");
	}
	return (noDeliveries);
}
/*===========================
| edit () callback function |
===========================*/
int 
heading (
 int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide();

	rv_pr (ML (mlTrMess062), 40, 0, 1);

	box (0, 1, 130, 3);
	
	print_at (22,0, mlStdMess038, comm_rec.co_no, comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
