/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (tr_trcm_inp.c )                                   |
|  Program Desc  : (TRansport Carrier Master maintanence.         )   |
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
char	*PNAME = "$RCSfile: trcm_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_trcm_inp/trcm_inp.c,v 5.6 2002/07/24 08:39:32 scott Exp $";

/*===========================================
| These next two lines for 132 tabular only |
===========================================*/
#define MAXWIDTH 	150
#define MAXLINES 	112
#define TABLINES 	4

#define	CARR_HEADER		1
#define	CARR_DETAILS	2

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_tr_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int  	newCarrier = 0;	

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 7;
	
	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tco_short[16];
		char	test_no[3];
		char	test_name[41];
		long	t_dbt_date;
	} comm_rec;

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
		char	carr_addr [4] [41];
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

	char	*scn_desc[] = {
		"Carrier Header information.",
		"Carrier Detail lines."
	};

	char	*trcm	=	"trcm",
			*trcl	=	"trcl",
			*trzm	=	"trzm",
			*data	=	"data";

struct	storeRec {
	char	workDeliveryZone [7];
} store [MAXLINES];
	
	extern	int		TruePosition;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	double	cost_kg;
} local_rec;

static	struct	var	vars[]	={	

	{CARR_HEADER, LIN, "carrierCode", 2, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Carrier Code    : ", "[SEARCH] to Search for valid carriers.", 
		NE, NO, JUSTLEFT, "", "", trcm_rec.carr_code}, 
	{CARR_HEADER, LIN, "carrierName", 2, 70, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Name    : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_desc}, 
	{CARR_HEADER, LIN, "carrierAddress1", 3, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Carrier Address : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_addr [0]}, 
	{CARR_HEADER, LIN, "carrierAddress2", 4, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------- : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_addr [1]}, 
	{CARR_HEADER, LIN, "carrierAddress3", 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------- : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_addr [2]}, 
	{CARR_HEADER, LIN, "carrierAddress4", 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "--------------- : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.carr_addr [3]}, 
	{CARR_HEADER, LIN, "carrierContract", 3, 70, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "NONE", "Carrier Contact : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.contact_name}, 
	{CARR_HEADER, LIN, "carrierPhone", 4, 70, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Carrier Phone   : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.phone}, 
	{CARR_HEADER, LIN, "carrierFax", 5, 70, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "N/A", "Carrier Fax No  : ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.fax_no}, 
	{CARR_HEADER, LIN, "carrierMarkup", 6, 70, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0.00", "Carrier Markup %: ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.markup_pc}, 
	{CARR_HEADER, LIN, "contractStart", 8, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "0", "Start Contact   : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.cont_start}, 
	{CARR_HEADER, LIN, "contractEnd", 8, 70, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "0", "End   Contact   : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.cont_end}, 
	{CARR_HEADER, LIN, "carrierComment", 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "General Comments: ", " ", 
		YES, NO, JUSTLEFT, "", "", trcm_rec.comment}, 
	{CARR_HEADER, LIN, "minWeight", 11, 2, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Minimum Weight : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.min_wgt}, 
	{CARR_HEADER, LIN, "maxWeight", 11, 70, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Maximum Weight : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.max_wgt}, 
	{CARR_HEADER, LIN, "minVolume", 12, 2, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Minimum Volume : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.min_vol}, 
	{CARR_HEADER, LIN, "maxVolume", 12, 70, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Maximum Volume : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.max_vol}, 
	{CARR_HEADER, LIN, "dailyCharge", 13, 2, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Daily   Charge : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.day_charge}, 
	{CARR_HEADER, LIN, "weeklyCharge", 13, 40, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Weekly  Charge : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.wky_charge}, 
	{CARR_HEADER, LIN, "weeklyCharge", 13, 70, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0.00", "Monthly Charge : ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&trcm_rec.mth_charge}, 
	{CARR_DETAILS, TAB, "deliveryZone",	 MAXLINES, 5, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Del. Zone Code    ", "Enter Delivery Zone Code [SEARCH]. ",
		 YES, NO, JUSTLEFT, "", "", trzm_rec.del_zone},
	{CARR_DETAILS, TAB, "deliveryZoneDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "       Delivery Zone Description.       ", " ",
		NA, NO,  JUSTLEFT, "", "", trzm_rec.desc},
	{CARR_DETAILS, TAB, "cost_kg", 0, 2, DOUBLETYPE, 
		"NNNNNNN.NN", "          ", 
		" ", "0.00", " Cost Per Kg. ", "", 
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.cost_kg}, 
	{CARR_DETAILS, TAB, "trzm_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO, JUSTLEFT, "", "", (char *)&trzm_rec.trzm_hash},

	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*=======================
| Function Declarations |
=======================*/
static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
int spec_valid (int field);
int CheckDuplicateZone (char *del_zone, int line_no);
void LoadTrcl (void);
void SrchTrcm (char *key_val);
static void Update (void);
void SrchTrzm (char *key_val);
int heading (int scn);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		i;

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (CARR_DETAILS, store, sizeof (struct storeRec));
#endif

	TruePosition	=	TRUE;

	for (i = 0; i < 2; i++)
		tab_data[ i ]._desc = scn_desc[ i ];

	init_vars (1);			/*  set default values		*/

	tab_row = 15;
	tab_col = 20;

	OpenDB ();

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit	= 	FALSE;
		edit_exit	= 	FALSE;
		prog_exit	= 	FALSE;
		restart		= 	FALSE;
		search_ok	=	TRUE;

		lcount [CARR_DETAILS] = 0;
		init_vars (CARR_HEADER);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (CARR_HEADER);
		entry (CARR_HEADER);

		if (prog_exit || restart)
			continue;
		
		scn_write (CARR_HEADER);
		scn_display (CARR_HEADER);
		scn_display (CARR_DETAILS);

		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		if (lcount [CARR_DETAILS] == 0)
			entry (CARR_DETAILS);
		else
			edit (CARR_DETAILS);

		/*if (restart)
			continue;*/

		edit_all ();

		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
static	void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static	void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (trcm, trcm_list, TRCM_NO_FIELDS, "trcm_id_no");
	open_rec (trcl, trcl_list, TRCL_NO_FIELDS, "trcl_id_no");
	open_rec (trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
static	void
CloseDB (
 void)
{
	abc_fclose (trcm);
	abc_fclose (trcl);
	abc_fclose (trzm);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
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
			
		strcpy (trcm_rec.co_no, comm_rec.tco_no);
		strcpy (trcm_rec.br_no, comm_rec.test_no);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (trcm);
			newCarrier = TRUE;
			return (EXIT_SUCCESS);
		}
		entry_exit = TRUE;

		/*-------------------------------------------
		| Load TRansport Carrier Lines information. |
		-------------------------------------------*/
		LoadTrcl ();
		
		scn_write (CARR_HEADER);
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate delivery Zone. |
	-------------------------*/
	if (LCHECK ("deliveryZone"))
	{
		if (SRCH_KEY)
		{
			SrchTrzm (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (trzm_rec.co_no, comm_rec.tco_no);
		strcpy (trzm_rec.br_no, comm_rec.test_no);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlTrMess059));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].workDeliveryZone, trzm_rec.del_zone);

		if (CheckDuplicateZone ( trzm_rec.del_zone, line_cnt ))
		{	
			errmess (ML (mlTrMess070));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("deliveryZone");
		DSP_FLD ("deliveryZoneDesc");

		return (EXIT_SUCCESS);
	}
	/*-----------------------
	| Validate cost per kg. |
	-----------------------*/
	if (LCHECK ("cost_kg"))
	{
		if (dflt_used && prog_status == ENTRY)
			local_rec.cost_kg	=	trzm_rec.chg_kg;
	
		DSP_FLD ("cost_kg");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*==================================
| Check for duplicate zone number. |
==================================*/
int
CheckDuplicateZone (
 char	*del_zone,
 int		line_no)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [CARR_DETAILS];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == line_no)
			continue;

		/*-----------------
		| duplicate zone. |
		-----------------*/
		if (!strcmp (store [i].workDeliveryZone, del_zone))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
/*====================================
| Load items from carrier line file. |
====================================*/
void
LoadTrcl (
 void)
{
	/*-----------------------------------------
	| Set screen CarrierDetails - for putval. |
	-----------------------------------------*/
	init_vars (CARR_DETAILS);
	lcount [CARR_DETAILS] = 0;
	abc_selfield (trzm, "trzm_trzm_hash");

	trcl_rec.trcm_hash = trcm_rec.trcm_hash;
	trcl_rec.trzm_hash = 0L;

	cc = find_rec (trcl, &trcl_rec, GTEQ, "r");
	while (!cc && trcm_rec.trcm_hash == trcl_rec.trcm_hash)
	{
		trzm_rec.trzm_hash	=	trcl_rec.trzm_hash;
		cc	=	find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (trcl, &trcl_rec, NEXT, "r");
			continue;
		}
		strcpy (store [lcount [CARR_DETAILS]].workDeliveryZone, trzm_rec.del_zone);
		local_rec.cost_kg = trcl_rec.cost_kg;

		putval (lcount [CARR_DETAILS]++);

		cc = find_rec (trcl, &trcl_rec, NEXT, "r");
	}

	newCarrier = (lcount[CARR_DETAILS] == 0);
	
	scn_set (CARR_HEADER);

	abc_selfield (trzm, "trzm_id_no");
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
/*===========================
| Update trcm/trcl records. |
===========================*/
static	void
Update (
 void)
{
	clear ();

	strcpy (trcm_rec.stat_flag, "0");
	if (newCarrier)
	{
		cc = abc_add (trcm, &trcm_rec);
		if (cc)
			file_err (cc, trcm, "DBADD");

		strcpy (trcm_rec.co_no, comm_rec.tco_no);
		strcpy (trcm_rec.br_no, comm_rec.test_no);
		cc = find_rec (trcm, &trcm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, trcm, "DBADD");
	}
	else
	{
		cc = abc_update (trcm, &trcm_rec);
		if (cc)
			file_err (cc, trcm, "DBUPDATE");
	}

	scn_set (CARR_DETAILS);

	for (line_cnt = 0; line_cnt < lcount [CARR_DETAILS]; line_cnt++)
	{
		getval (line_cnt);

		trcl_rec.trcm_hash = trcm_rec.trcm_hash;
		trcl_rec.trzm_hash = trzm_rec.trzm_hash;
		cc = find_rec (trcl, &trcl_rec, COMPARISON, "u");
		if (cc)
		{
			trcl_rec.cost_kg = local_rec.cost_kg;

			abc_unlock ("trcl");
			cc = abc_add (trcl, &trcl_rec);
			if (cc)
				file_err (cc, trcl, "DBADD");
		}
		else
		{
			trcl_rec.cost_kg = local_rec.cost_kg;
			cc = abc_update (trcl, &trcl_rec);
			if (cc)
				file_err (cc,trcl, "DBUPDATE");
		}
	}
	scn_set (CARR_HEADER);
}

/*=========================
| Search for Zome Master. |
=========================*/
void
SrchTrzm (
 char *key_val)
{
	work_open ();

	save_rec ("#Zone. ","#Zone Description");

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.test_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", key_val);
	cc = find_rec (trzm, &trzm_rec, GTEQ, "r");
	while (!cc && !strcmp (trzm_rec.co_no, comm_rec.tco_no) &&
				  !strcmp (trzm_rec.br_no, comm_rec.test_no) &&
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

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.test_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, trzm, "DBFIND");

	return;
}

int
heading (
 int scn)
{
	if (restart) 
	{
		abc_unlock (trcm);
		return (EXIT_SUCCESS);
	}

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();
	rv_pr (ML (mlTrMess069),45,0,1);

	if (scn	== CARR_HEADER)
	{
		box (0,1,131,12);
		move (1,7);
		line (130);
		move (1,10);
		line (130);
		scn_set (CARR_DETAILS);
		scn_write (CARR_DETAILS);
		scn_display (CARR_DETAILS);
	}
	if (scn	== CARR_DETAILS)
	{
		box (0,1,131,12);
		move (1,7);
		line (130);
		move (1,10);
		line (130);
		scn_set (CARR_HEADER);
		scn_write (CARR_HEADER);
		scn_display (CARR_HEADER);
	}

	move (0,21);
	line (131);
	print_at (22,0,ML (mlStdMess038), comm_rec.tco_no, 	comm_rec.tco_short);
	print_at (22,25,ML (mlStdMess039), comm_rec.test_no, comm_rec.test_name);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	move (1,input_row);
	scn_write (scn);

    return (EXIT_SUCCESS);
}
