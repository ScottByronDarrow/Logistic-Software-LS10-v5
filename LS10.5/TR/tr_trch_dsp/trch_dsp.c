/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (tr_trch_dsp.c )                                   |
|  Program Desc  : (TRansport Carrier History Display.              ) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (26/03/1999)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: trch_dsp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_trch_dsp/trch_dsp.c,v 5.3 2001/10/17 08:15:06 robert Exp $";

#define	X_OFF	5
#define	Y_OFF	4

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>
#include <ml_so_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#ifdef PageSizeInternal
#undef PageSizeInternal
#endif

#define	PageSizeInternal	15
#define	VALID_CARR ((strcmp (trch_rec.carr_code, local_rec.startCarrier) >= 0) \
	             && (strcmp (trch_rec.carr_code, local_rec.endCarrier) <= 0))

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	t_inv_date;
	} comm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
	};

	int	cumr_no_fields = 2;

	struct	{
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
	} cumr_rec;

	/*==========================================
	| Cost Centre/Warehouse Master File Record |
	==========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
	};

	int	ccmr_no_fields = 3;

	struct	{
		char	cc_co_no[3];
		char	cc_est_no[3];
		char	cc_cc_no[3];
	} ccmr_rec;

	/*================================+
	 | TRansport Carrier Master File. |
	 +================================*/
#define	TRCM_NO_FIELDS	26

	struct dbview	trcm_list [TRCM_NO_FIELDS] =
	{
		{"trcm_co_no"},
		{"trcm_br_no"},
		{"trcm_carr_code"},
		{"trcm_trcm_hash"},
		{"trcm_carr_desc"},
		{"trcm_carr_name"},
		{"trcm_carr_addr1"},
		{"trcm_carr_addr2"},
		{"trcm_carr_addr3"},
		{"trcm_carr_addr4"},
		{"trcm_min_wgt"},
		{"trcm_max_wgt"},
		{"trcm_min_vol"},
		{"trcm_max_vol"},
		{"trcm_bonded"},
		{"trcm_phone"},
		{"trcm_fax_no"},
		{"trcm_contact_name"},
		{"trcm_cont_start"},
		{"trcm_cont_end"},
		{"trcm_comment"},
		{"trcm_markup_pc"},
		{"trcm_day_charge"},
		{"trcm_wky_charge"},
		{"trcm_mth_charge"},
		{"trcm_stat_flag"}
	};

	struct tag_trcmRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	carr_code [5];
		long	trcm_hash;
		char	carr_desc [41];
		char	carr_name [41];
		char	carr_addr1 [41];
		char	carr_addr2 [41];
		char	carr_addr3 [41];
		char	carr_addr4 [41];
		float	min_wgt;
		float	max_wgt;
		float	min_vol;
		float	max_vol;
		char	bonded [2];
		char	phone [16];
		char	fax_no [16];
		char	contact_name [21];
		long	cont_start;
		long	cont_end;
		char	comment [61];
		float	markup_pc;
		double	day_charge;
		double	wky_charge;
		double	mth_charge;
		char	stat_flag [2];
	}	trcm_rec;

	/*=================================+
	 | TRansport Carrier History File. |
	 +=================================*/
#define	TRCH_NO_FIELDS	15

	struct dbview	trch_list [TRCH_NO_FIELDS] =
	{
		{"trch_co_no"},
		{"trch_br_no"},
		{"trch_wh_no"},
		{"trch_ref_no"},
		{"trch_date"},
		{"trch_hhcu_hash"},
		{"trch_cons_no"},
		{"trch_carr_code"},
		{"trch_del_zone"},
		{"trch_no_cartons"},
		{"trch_no_kgs"},
		{"trch_est_frt_cst"},
		{"trch_act_frt_cst"},
		{"trch_cumr_chg"},
		{"trch_stat_flag"}
	};

	struct tag_trchRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	wh_no [3];
		char	ref_no [9];
		Date	date;
		long	hhcu_hash;
		char	cons_no [17];
		char	carr_code [5];
		char	del_zone [7];
		int		no_cartons;
		float	no_kgs;
		double	est_frt_cst;
		double	act_frt_cst;
		char	cumr_chg [2];
		char	stat_flag [2];
	}	trch_rec;

	/*==============================+
	 | TRansport Carrier Line File. |
	 +==============================*/
#define	TRCL_NO_FIELDS	4

	struct dbview	trcl_list [TRCL_NO_FIELDS] =
	{
		{"trcl_trcm_hash"},
		{"trcl_trzm_hash"},
		{"trcl_cost_kg"},
		{"trcl_stat_flag"}
	};

	struct tag_trclRecord
	{
		long	trcm_hash;
		long	trzm_hash;
		double	cost_kg;
		char	stat_flag [2];
	}	trcl_rec;

	/*=============================+
	 | TRansport Zone Maintenance. |
	 +=============================*/
#define	TRZM_NO_FIELDS	7

	struct dbview	trzm_list [TRZM_NO_FIELDS] =
	{
		{"trzm_co_no"},
		{"trzm_br_no"},
		{"trzm_del_zone"},
		{"trzm_desc"},
		{"trzm_dflt_chg"},
		{"trzm_chg_kg"},
		{"trzm_trzm_hash"}
	};

	struct tag_trzmRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	del_zone [7];
		char	desc [41];
		double	dflt_chg;
		double	chg_kg;
		long	trzm_hash;
	}	trzm_rec;

	char	*trzm	=	"trzm",
			*trcl	=	"trcl",
			*trch	=	"trch",
			*trcm	=	"trcm",
			*cumr	=	"cumr";

/*============================
| Local & Screen Structures. |
============================*/
float	est_tot[4];
float	act_tot[4];
int		ptr_offset[7];
int		DISPLAY_REP;
int		DETAILED;
char	currentBranch[3];
char	printBarnch[3];
char	currentWarehouse[3];
char	printWarehouse[3];
char	currentCarrier[5];
char	printCarrier[5];
long	currentDate;
char	printDate[11];
char	dsp_str[200];
char	branchNo[3];
char	data_str[300];
char	rep_type[2];

FILE	*fout,
		*fsort;

struct {
	char	dummy[11];
	long	lsystemDate;
	long	startDate;
	long	endDate;
	char	startCarrier[5];
	char	startCarrierDesc[41];
	char	endCarrier[5];
	char	endCarrierDesc[41];
	char	prntscn[31];
	char	type[31];
	char	prntscn_desc[31];
	char	type_desc[31];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "startDate",	 4, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Start Date             :", "Default = 1st of Month.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate},
	{1, LIN, "endDate",	 5, 25, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "End Date               :", "Default = today.",
		 NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate},

	{1, LIN, "startCarrier",	 7, 25, CHARTYPE,
		"UUUU", "          ",
		" ", "    ", "Start Carrier code     :", "Default = First Carrier.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.startCarrier},
	{1, LIN, "startCarrierDesc",	 7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "    ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startCarrierDesc},
	{1, LIN, "endCarrier",	 8, 25, CHARTYPE,
		"UUUU", "          ",
		" ", "~~~~", "End Carrier code       :", "Default = Last Carrier.",
		 NO, NO,  JUSTLEFT, "", "", local_rec.endCarrier},
	{1, LIN, "endCarrierDesc", 8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "    ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endCarrierDesc},

	{1, LIN, "r_type",	10, 25, CHARTYPE,
		"U", "          ",
		" ", "S", "S(ummary) /D(etailed)  :", " ",
		YES, NO,  JUSTLEFT, "SD", "", local_rec.type},
	{1, LIN, "r_type_desc",	10, 28, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.type_desc},
	{1, LIN, "prntscn",	11, 25, CHARTYPE,
		"U", "          ",
		" ", "S", "P(rinter) or S(creen)  :", " ",
		YES, NO,  JUSTLEFT, "PS", "", local_rec.prntscn},
	{1, LIN, "prntscn_desc",	11, 28, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.prntscn_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*extern	int		DSP_UTILS_lpno;*/

/*=====================
 Function declarations
======================*/
void runProgram (char *programName, char *programDesc);
void shutdown_prog (void);
int spec_valid (int field);
void SrchTrcm (char *key_val);
int heading (int scn);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
void headingOutput (void);
void ProcessFreightHistory (void);
void processSortedFreight (void);
char *_sort_read (FILE *srt_fil);
void init_array (void);
void set_breaks (char *tptr);
int delta_carr (char *tptr);
int delta_wh (char *tptr);
int delta_br (char *tptr);
int delta_date (char *tptr);
void print_total (char *type);

/*==========================
| Main Processing Routine  |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{

	if (argc != 1 && argc != 7)
	{
		print_at (0,0,mlSoMess704,argv[0]);
		print_at (1,0,mlSoMess706);
		print_at (2,0,mlSoMess707);
		print_at (3,0,mlSoMess708);
		print_at (4,0,mlSoMess709); 
		print_at (5,0,mlSoMess710);
		print_at (6,0,mlSoMess711);
        return (EXIT_FAILURE);
	}

	if (argc == 7)
	{
		DISPLAY_REP = (argv[1][0] == 'S');
		DETAILED = (argv[2][0] == 'D');
	}
	
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	if (argc == 1 || (argc == 7 && DISPLAY_REP))
	{
		init_scr ();		/*  sets terminal from termcap	*/
		set_tty ();		/*  get into raw mode		*/
		set_masks ();		/*  setup print using masks	*/
		init_vars (1);		/*  set default values		*/
		if (DETAILED)
			swide ();
	}

	ReadMisc ();
	OpenDB ();

	if (argc == 7)
	{
		local_rec.startDate 	= StringToDate (argv[3]);
		local_rec.endDate 		= StringToDate (argv[4]);
		sprintf (local_rec.startCarrier, "%-4.4s", argv[5]);
		sprintf (local_rec.endCarrier,	 "%-4.4s", argv[6]);

		if (!DISPLAY_REP)
			dsp_screen ("Processing : Printing Freight Report.",
				comm_rec.tco_no,
				comm_rec.tco_name);

		headingOutput ();

		ProcessFreightHistory ();

		if (DISPLAY_REP)
		{
			Dsp_srch ();
			Dsp_close ();
		}
		else
			Dsp_print ();

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);		/*  set default values		*/

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

		runProgram (argv[0], argv[1]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
runProgram (
 char	*programName,
 char	*programDesc)
{
	char	tmp_startDate[11];
	char	tmp_endDate[11];

	strcpy (tmp_startDate, DateToString (local_rec.startDate));
	strcpy (tmp_endDate,DateToString (local_rec.endDate));
	
	if (local_rec.prntscn[0] == 'S')
	{
		clear ();
		fflush (stdout);
	}
	execlp 
	(
		programName,
		programName,
		local_rec.prntscn,
		local_rec.type,
		tmp_startDate,
		tmp_endDate,
		local_rec.startCarrier,
		local_rec.endCarrier, (char *)0
	);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();;
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startDate")) 
	{
		if (dflt_used)
		{
			local_rec.startDate = MonthStart (local_rec.lsystemDate);
			DSP_FLD ("startDate");
			return (EXIT_SUCCESS);
		}
		if ((prog_status == EDIT) && (local_rec.startDate > local_rec.endDate))
		{
			errmess ("Start Date is greater than End Date");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endDate")) 
	{
		if (dflt_used)
		{
			local_rec.endDate = local_rec.lsystemDate;
			DSP_FLD ("endDate");
			return (EXIT_SUCCESS);
		}
		if (local_rec.endDate < local_rec.startDate)
		{
			errmess ("End Date is less than Start Date");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startCarrier"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.startCarrierDesc, "%-40.40s", "First Carrier");
			DSP_FLD ("startCarrierDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			return (EXIT_SUCCESS);
		}
	
		strcpy (trcm_rec.co_no, comm_rec.tco_no);
		strcpy (trcm_rec.br_no, "  ");
		strcpy (trcm_rec.carr_code, "    ");
		cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
		while (!cc && !strcmp (trcm_rec.co_no, comm_rec.tco_no))
		{
			if (!strcmp (trcm_rec.carr_code, local_rec.startCarrier)) {
				strcpy (local_rec.startCarrierDesc, trcm_rec.carr_desc);
				DSP_FLD ("startCarrierDesc");

				return (EXIT_SUCCESS);
			}
			cc = find_rec (trcm, &trcm_rec, NEXT, "r");
		}

		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("endCarrier"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.endCarrierDesc, "%-40.40s", "Last Carrier");
			DSP_FLD ("endCarrierDesc");
	
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTrcm (temp_str);
			return (EXIT_SUCCESS);
		}
	
		strcpy (trcm_rec.co_no, comm_rec.tco_no);
		strcpy (trcm_rec.br_no, "  ");
		strcpy (trcm_rec.carr_code, "    ");
		cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
		while (!cc)
		{
			if (!strcmp (trcm_rec.carr_code, local_rec.endCarrier))
			{
				strcpy (local_rec.endCarrierDesc, trcm_rec.carr_desc);
				DSP_FLD ("endCarrierDesc");

				return (EXIT_SUCCESS);
			}

			cc = find_rec ("trcm", &trcm_rec, NEXT, "r");
		}
		print_mess (ML (mlStdMess134));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("r_type"))
	{
		strcpy (local_rec.type_desc, (local_rec.type[0] == 'D') ? "Detailed" : "Summary ");
		DSP_FLD ("r_type_desc");
	}
	
	if (LCHECK ("prntscn"))
	{
		strcpy (local_rec.prntscn_desc, (local_rec.prntscn[0] == 'P') ? "Printer" : "Screen ");
		DSP_FLD ("prntscn_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*=====================
| Search for Carrier. |
=====================*/
void
SrchTrcm (
 char	*key_val)
{
	work_open ();
	save_rec ("#Carrier","#Carrier  Description ");

	strcpy (trcm_rec.co_no, comm_rec.tco_no);
	strcpy (trcm_rec.br_no, comm_rec.test_no);
	sprintf (trcm_rec.carr_code,"%-4.4s", key_val);
	cc = find_rec (trcm, &trcm_rec, GTEQ, "r");
	while (!cc && !strcmp (trcm_rec.co_no, comm_rec.tco_no) &&
		      	  !strcmp (trcm_rec.br_no, comm_rec.test_no) &&
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

	strcpy (trcm_rec.co_no, comm_rec.tco_no);
	strcpy (trcm_rec.br_no, comm_rec.test_no);
	sprintf (trcm_rec.carr_code,"%-4.4s", temp_str);
	cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trcm, "DBFIND");
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
		rv_pr (ML (mlSoMess010),25,0,1);
		move (0,1);
		line (80);

		move (0,6);
		line (80);

		move (0,9);
		line (80);

		box (0,3,80,8);

		move (0,20);
		line (80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str, comm_rec.tco_no, "\0");
		strcpy (err_str, ML (mlStdMess039));
		print_at (21,13,err_str,comm_rec.test_no,comm_rec.test_name);
		move (0,22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");
	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
	open_rec (trch, trch_list, TRCH_NO_FIELDS, "trch_id_no2");
	open_rec (cumr, cumr_list, cumr_no_fields, "cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (trzm);
	abc_fclose (trcl);
	abc_fclose (trcm);
	abc_fclose (trch);
	abc_fclose (cumr);
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	abc_dbopen ("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	open_rec ("ccmr",ccmr_list,ccmr_no_fields,"ccmr_id_no");
	strcpy (ccmr_rec.cc_co_no,comm_rec.tco_no);
	strcpy (ccmr_rec.cc_est_no,comm_rec.test_no);
	strcpy (ccmr_rec.cc_cc_no,comm_rec.tcc_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
headingOutput (
 void)
{
	char	tmp_startDate [11];
	char	tmp_endDate	  [11];
	int		lcl_x_coord	=	0;

	if (DETAILED)
		lcl_x_coord = 0;
	else
		lcl_x_coord = 15;

	strcpy (tmp_startDate, 	DateToString (local_rec.startDate));
	strcpy (tmp_endDate,	DateToString (local_rec.endDate));

	sprintf 
	(
		err_str, 
		ML (mlTrMess071), 
		tmp_startDate, 
		tmp_endDate,
		local_rec.startCarrier, 
		local_rec.endCarrier
	);

	if (DISPLAY_REP)
	{
		if (DETAILED)
		{
			rv_pr (err_str, (132 - (int) strlen (err_str)) / 2,2,1);
			rv_pr (ML (mlSoMess010), 55,0,0);
		}
		else
		{
			rv_pr (err_str, (80 - (int) strlen (err_str)) / 2,2,1);
			rv_pr (ML (mlSoMess010), 27,0,0);
		}

		Dsp_prn_open 
		(
			lcl_x_coord, 
			4, 
			PageSizeInternal, 
			err_str, 
			comm_rec.tco_no, 
			comm_rec.tco_name, 
			(char *)0, 
			(char *)0, 
			(char *)0, 
			(char *)0
		);
	}
	else
	{
		Dsp_nd_prn_open 
		(
			lcl_x_coord, 
			4, 
			PageSizeInternal, 
			err_str, 
			comm_rec.tco_no, 
			comm_rec.tco_name, 
			(char *)0, 
			(char *)0, 
			(char *)0, 
			(char *)0
	);
	}

	if (DETAILED)
	{
		Dsp_saverec ("Reference|    Date   | Carrier  |   Consignment    | Zone | Cust.  | No. Cartons | Total Kgs.  |  Estimated  |    Actual   ");
		Dsp_saverec (" Number  |Transaction| Number.  |   Note Number.   |Number|Number  |             |             |    charge   |     charge  ");
	}
	else
	{
		Dsp_saverec ("    Date       |  Estimated   |     Actual   ");
		Dsp_saverec ("  of charges   |    charge    |     Charge   ");
	}
	Dsp_saverec ("[REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");
	return;
}

void
ProcessFreightHistory (
 void)
{
	fsort = sort_open ("freight");

	strcpy (trch_rec.co_no, comm_rec.tco_no);
	strcpy (trch_rec.br_no, "  ");
	strcpy (trch_rec.wh_no, "  ");
	trch_rec.date = 0L;

	cc = find_rec ("trch", &trch_rec, GTEQ, "r");
	while (!cc && !strcmp (trch_rec.co_no, comm_rec.tco_no))
	{
		if (trch_rec.date < local_rec.startDate ||
		    trch_rec.date > local_rec.endDate || !VALID_CARR)
		{
			cc = find_rec ("trch", &trch_rec, NEXT, "r");
			continue;
		}
		if (!DISPLAY_REP)
			dsp_process ("Reading Ref No : ", trch_rec.ref_no);

		if (DETAILED)
		{
			sprintf (data_str, "%2.2s%2.2s%4.4s%010ld%-16.16s",
					trch_rec.br_no,
					trch_rec.wh_no,
					trch_rec.carr_code,
					trch_rec.date,
					trch_rec.cons_no);
		}
		else
		{
				sprintf (data_str, "%010ld%2.2s%2.2s%4.4s%-16.16s",
					trch_rec.date,
					trch_rec.br_no,
					trch_rec.wh_no,
					trch_rec.carr_code,
					trch_rec.cons_no);
		}
		clip (data_str);
		sprintf (err_str, "%c%s%c%s%c%ld%c%d%c%.2f%c%.2f%c%.2f\n",
				1, trch_rec.ref_no,
				1, trch_rec.del_zone,
				1, trch_rec.hhcu_hash,
				1, trch_rec.no_cartons,
				1, trch_rec.no_kgs,
				1, trch_rec.est_frt_cst,
				1, trch_rec.act_frt_cst);
		strcat (data_str, err_str);
		sort_save (fsort, data_str);
		cc = find_rec ("trch", &trch_rec, NEXT, "r");
	}
	processSortedFreight ();
}

void
processSortedFreight (
 void)
{
	char	*_sort_read (FILE *srt_fil);
	char	*sptr;
	int		first_time;
	int		tot_printed = FALSE;

	init_array ();
	first_time = TRUE;

	fsort = sort_sort (fsort,"freight");

	sptr = _sort_read (fsort);
	if (sptr)
		set_breaks (sptr);

	while (sptr != (char *)0)
	{
		if (!DETAILED)
			tot_printed = FALSE;

		/*---------------------
		| Get customer number |
		---------------------*/
		cumr_rec.cm_hhcu_hash	=	atol (sptr + ptr_offset[2]);
		cc = find_rec ("cumr", &cumr_rec, COMPARISON, "r");
		if (cc)
			strcpy (cumr_rec.cm_dbt_no, "DELETE");
	
		if (DETAILED)
		{
			sprintf 
			(
				dsp_str, 
				" %-8.8s^E %-10.10s^E   %4.4s   ^E %-16.16s ^E%-6.6s^E %-6.6s ^E   %5d     ^E%12.2f ^E%12.2f ^E%12.2f ",

				sptr + ptr_offset[0],			/* ref */
				DateToString (atol (sptr + 8)),		/* date */
				sptr + 4,						/* carrier */
				sptr + 18,						/* cons no */
				sptr + ptr_offset[1],			/* area */
				cumr_rec.cm_dbt_no,				/* cust */
				atoi (sptr + ptr_offset[3]),	/* no ctns */
				atof (sptr + ptr_offset[4]),	/* no kgs */
				atof (sptr + ptr_offset[5]),	/* est chrg */
				atof (sptr + ptr_offset[6]));	/* act chrg */
		}
		else
		{
			sprintf (dsp_str, "  %10.10s   ^E %13.2f ^E %13.2f",
				DateToString (atol (sptr)),			/* date */
				atof (sptr + ptr_offset[5]),	/* est chrg */
				atof (sptr + ptr_offset[6]));	/* act chrg */
		}

		est_tot[0] += atof (sptr + ptr_offset[5]);
		est_tot[1] += atof (sptr + ptr_offset[5]);
		est_tot[2] += atof (sptr + ptr_offset[5]);
		est_tot[3] += atof (sptr + ptr_offset[5]);
			
		act_tot[0] += atof (sptr + ptr_offset[6]);
		act_tot[1] += atof (sptr + ptr_offset[6]);
		act_tot[2] += atof (sptr + ptr_offset[6]);
		act_tot[3] += atof (sptr + ptr_offset[6]);
	
		if (DETAILED)
			Dsp_saverec (dsp_str);

		sptr = _sort_read (fsort);

		if (sptr)
		{
			if (DETAILED)
			{
				if (delta_carr (sptr))
				{
					print_total ("C");
					tot_printed = TRUE;
				}
	
				if (delta_wh (sptr))
				{
					print_total ("W");
					tot_printed = TRUE;
				}
	
				if (delta_br (sptr))
				{
					print_total ("B");
					tot_printed = TRUE;
				}
	
				if (tot_printed)
				{
					Dsp_saverec ("^^GGGGGGGGGIGGGGGGGGGGGIGGGGGGGGGGIGGGGGGGGGGGGGGGGGGIGGGGGGIGGGGGGGGIGGGGGGGGGGGGGIGGGGGGGGGGGGGIGGGGGGGGGGGGGIGGGGGGGGGGGGG^^");
					tot_printed = FALSE;
				}
			}
			else
			{
				if (delta_date (sptr))
				{
					print_total ("S");
					tot_printed = TRUE;
				}
			}
		}
	}
	
	if (DETAILED)
	{
		print_total ("C");
		print_total ("W");
		print_total ("B");
	}
	else
		if (!tot_printed)
			print_total ("S");

	print_total ("G");

	sort_delete (fsort,"freight");
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char	*
_sort_read (
 FILE *srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 0;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	tptr = sptr;
	while (fld_no < 7)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;
		ptr_offset[fld_no++] = tptr - sptr;
	}

	return (sptr);
}

void
init_array (
 void)
{
	int	i;

	for (i = 0; i < 4; i++)
	{
		est_tot[i] = 0.00;
		act_tot[i] = 0.00;
	}
}

void
set_breaks (
 char *tptr)
{
	if (DETAILED)
	{
		sprintf (currentCarrier, "%-4.4s", tptr + 4);
		sprintf (printCarrier, 	 "%-4.4s", currentCarrier);

		sprintf (currentWarehouse, "%2.2s", tptr + 2);
		sprintf (printWarehouse, "%2.2s", currentWarehouse);

		sprintf (currentBranch, "%2.2s", tptr);
		sprintf (printBarnch, "%2.2s", currentBranch);
	}
	else
	{
		currentDate = atol (tptr);
		strcpy (printDate, DateToString (currentDate));
	}

}

int
delta_carr (
 char *tptr)
{
	char	tmp_carr [5];
	char	tmp_wh [3];
	char	tmp_br [3];

	sprintf (tmp_carr, "%-4.4s", tptr + 4);
	sprintf (tmp_wh, "%2.2s", tptr + 2);
	sprintf (tmp_br, "%2.2s", tptr);

	if (strcmp (currentCarrier, tmp_carr) ||
	    strcmp (currentWarehouse, tmp_wh) ||
	    strcmp (currentBranch, tmp_br))
	{
		sprintf (printCarrier, 	"%-4.4s", currentCarrier);
		sprintf (currentCarrier,"%-4.4s", tmp_carr);
		return (TRUE);
	}

	return (FALSE);
}

int
delta_wh (
 char *tptr)
{
	char	tmp_wh [3];
	char	tmp_br [3];

	sprintf (tmp_wh, "%2.2s", tptr + 2);
	sprintf (tmp_br, "%2.2s", tptr);

	if (strcmp (currentWarehouse, tmp_wh) || 
	    strcmp (currentBranch, tmp_br))
	{
		sprintf (printWarehouse, "%2.2s", currentWarehouse);
		sprintf (currentWarehouse, "%2.2s", tmp_wh);
		return (TRUE);
	}

	return (FALSE);
}

int
delta_br (
 char *tptr)
{
	char	tmp_br [3];

	sprintf (tmp_br, "%2.2s", tptr);

	if (strcmp (currentBranch, tmp_br))
	{
		sprintf (printBarnch, "%2.2s", currentBranch);
		sprintf (currentBranch, "%2.2s", tmp_br);
		return (TRUE);
	}
	return (FALSE);
}

int
delta_date (
 char *tptr)
{
	long	tmp_date;

	tmp_date = atol (tptr);

	if (currentDate != tmp_date)
	{
		strcpy (printDate, DateToString (currentDate));
		currentDate = tmp_date;
		return (TRUE);
	}

	return (FALSE);
}

void
print_total (
 char *type)
{
	switch (type[0])
	{
	case 'C':
		sprintf (dsp_str, " Total For carrier %4.4s         ^E                  ^E      ^E        ^E             ^E             ^E%12.2f ^E%12.2f ",
			printCarrier,
			est_tot[0],
			act_tot[0]);
		Dsp_saverec (dsp_str);

		sprintf (printCarrier, "%-4.4s", currentCarrier);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;

		break;

	case 'W':
		sprintf (dsp_str, " Total For warehouse : %2.2s       ^E                  ^E      ^E        ^E             ^E             ^E%12.2f ^E%12.2f ",
			printWarehouse,
			est_tot[1],
			act_tot[1]);
		Dsp_saverec (dsp_str);

		sprintf (printWarehouse, "%2.2s", currentWarehouse);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;
		est_tot[1] = 0.00;
		act_tot[1] = 0.00;

		break;

	case 'B':
		sprintf (dsp_str, " Total For branch    : %2.2s       ^E                  ^E      ^E        ^E             ^E             ^E%12.2f ^E%12.2f ",
			printBarnch,
			est_tot[2],
			act_tot[2]);
		Dsp_saverec (dsp_str);

		sprintf (printBarnch, "%2.2s", currentBranch);

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;
		est_tot[1] = 0.00;
		act_tot[1] = 0.00;
		est_tot[2] = 0.00;
		act_tot[2] = 0.00;

		break;

	case 'G':
		if (DETAILED)
		{
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGHGGGGGGHGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGGHGGGGGGGGGGGGG^^");
			sprintf (dsp_str, " GRAND TOTAL :                  ^E                  ^E      ^E        ^E             ^E             ^E%12.2f ^E%12.2f ",
				est_tot[3],
				act_tot[3]);
			Dsp_saverec (dsp_str);
			Dsp_saverec ("^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGJGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGGJGGGGGGGGGGGGG^^");
		}
		else
		{
			Dsp_saverec ("^^GGGGGGGGGGGGGGGHGGGGGGGGGGGGGGHGGGGGGGGGGGGGG^^");
			sprintf (dsp_str, " GRAND TOTAL : ^E%13.2f ^E%13.2f ",
				est_tot[3],
				act_tot[3]);
			Dsp_saverec (dsp_str);
			Dsp_saverec ("^^GGGGGGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGG^^");
		}

		break;

	case 'S':
		sprintf (dsp_str, "  %-10.10s   ^E%13.2f ^E%13.2f ",
			printDate,
			est_tot[0],
			act_tot[0]);
		Dsp_saverec (dsp_str);

		strcpy (printDate, DateToString (currentDate));

		est_tot[0] = 0.00;
		act_tot[0] = 0.00;

		break;
	}
}

