/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( tr_delsheet.c    )                               |
|  Program Desc  : ( Transport Vehicle Delivery Sheet.            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  trve, comm, trhr, trzm, cohr, coln, inmr, 		  |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  trve,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Joy G. Medel    | Date Written  : 14/12/95         |
|---------------------------------------------------------------------|
|  Date Modified : (30/09/96)      | Modified by : Elena B. Cuaresma. |
|  Date Modified : (05/09/97)      | Modified by : Ana Marie C.Tario. |
|  Date Modified : (28/10/1997)    | Modified by : Campbell Mander.   |
|  Date Modified : (24/08/1998)    | Modified by : Elizabeth D. Paid. |
|  Comments      :                                                    |
|    (30/09/96)  : Updated for multiple UOM.                          |
|    (05/09/97)  : Incorporated multilingual conversion and DMY4 date.|
|  (28/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|  (24/08/1998)  : Fixed some bugs in the alignment of the report due |
|                : to the expansion of invoice number from 6 to 8.    |
|                : Modified to fixed bugs found during testing and    |
|                : invoked _sort_read().                              |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: delsheet.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_delsheet/delsheet.c,v 5.3 2002/07/17 09:58:12 scott Exp $";

#include <ml_tr_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

char 	*trve = "trve",
     	*comm = "comm",
		*trhr = "trhr",
		*trzm = "trzm",
		*cohr = "cohr",
		*cumr = "cumr",
		*coln = "coln",
		*inmr = "inmr",
		*inum = "inum",
		*data = "data",
		*sptr;

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
        {"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_cc_no"},
		{"comm_cc_short"}
	};

	int comm_no_fields = 9;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
        char    tco_short[16];
		char	test_no[3];
		char  	test_name[41];
		char	test_short[16];
		char	tcc_no[3];
		char	tcc_short[10];
	} comm_rec;

	/*================================+
	 | Transport vehicle file record. |
	 +================================*/
#define	TRVE_NO_FIELDS	12

	struct dbview	trve_list [TRVE_NO_FIELDS] =
	{
		{"trve_co_no"},
		{"trve_br_no"},
		{"trve_ref"},
		{"trve_desc"},
		{"trve_cap"},
		{"trve_fr_chg"},
		{"trve_hhmr_hash"},
		{"trve_vehi_type"},
		{"trve_hhve_hash"},
		{"trve_truck_type"},
		{"trve_avail"},
		{"trve_unav_res"},
	};

	struct tag_trveRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	vehicleNo [11];
		char	desc [41];
		float	cap;
		Money	fr_chg;
		long	hhmr_hash;
		char	vehi_type [11];
		long	hhve_hash;
		char	truck_type [11];
		char	avail [2];
		char	unav_res [31];
	}	trve_rec;

	/*========================+
	 | Transport Header file. |
	 +========================*/
#define	TRHR_NO_FIELDS	11

	struct dbview	trhr_list [TRHR_NO_FIELDS] =
	{
		{"trhr_trip_name"},
		{"trhr_hhve_hash"},
		{"trhr_del_date"},
		{"trhr_hhtr_hash"},
		{"trhr_driver"},
		{"trhr_rf_number"},
		{"trhr_act_date"},
		{"trhr_act_time"},
		{"trhr_fr_chg"},
		{"trhr_fr_zchg"},
		{"trhr_status"}
	};

	struct tag_trhrRecord
	{
		char	trip_name [13];
		long	hhve_hash;
		Date	del_date;
		long	hhtr_hash;
		char	driver [7];
		char	rf_number [11];
		Date	act_date;
		long	act_time;
		Money	fr_chg;
		Money	fr_zchg;
		char	status [2];
	}	trhr_rec;

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

	/*======+
	 | cohr |
	 +======*/
#define	COHR_NO_FIELDS	15

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_del_date"},
		{"cohr_no_kgs"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
		{"cohr_del_zone"},
		{"cohr_dl_name"},
		{"cohr_dl_add1"},
		{"cohr_dl_add2"},
		{"cohr_dl_add3"},
		{"cohr_hhtr_hash"}
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	inv_no [9];
		long	hhcu_hash;
		long	del_date;
		float	no_kgs;
		long	hhco_hash;
		Date	date_raised;
		Date	date_required;
		char	del_zone [7];
		char	dl_name [41];
		char	dl_add[3][41];
		long	hhtr_hash;
	}	cohr_rec;

	/*============================================+
	 | Customer Order/Invoice/Credit Detail File. |
	 +============================================*/
#define	COLN_NO_FIELDS	5

	struct dbview	coln_list [COLN_NO_FIELDS] =
	{
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_hhum_hash"},
		{"coln_q_order"},
	};

	struct tag_colnRecord
	{
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhum_hash;
		float	q_order;
	}	coln_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	4

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		long	hhcu_hash;
	}	cumr_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	4

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_std_uom"},
	};

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		long	std_uom;
	}	inmr_rec;

	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
	struct dbview	inum_list[] ={
		{"inum_uom_group"},
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"}
	};

	int inum_no_fields = 5;

	struct {
		char	uom_group [21];
		long	hhum_hash;
		char	uom [5];
		char	desc [41];
		float	cnv_fct;
	}	inum_rec;

	char	previousDelZone[7];

	FILE	*fout,
			*fsort;

	char *srt_offset[256];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	long 	deliveryDate;
	char	deliveryDateDescr[10];
	char 	vehicleNo[11];
	char	vehicleDescr[41];
	char	del_zone[7];
	char	zoneDescr[41];
    int     printerNumber;
	char 	backGroundProcess[2];
	char	overNightProcess[2];
	} local_rec;

struct {
	char	Date[11];
	char	vehicleNo[11];
	char	inv_no[9];
	char	dbt_no[7];
	char	dl_name[41];
	char	dl_add[3][41];
	char	del_zone[7];
	float	no_kgs;
	char	Date1[11];
	char	Date2[11];
	char	item_no[17];
	char	uom[4];
	double	q_order;
		}xRec;

static	struct	var	vars[] =
{
	{1, LIN, "deliveryDate",	 3, 19, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Delivery Date     :", "Enter Delivery Date. Default is All",
		 YES, NO, JUSTRIGHT, "", "", (char*)&local_rec.deliveryDate},
	{1, LIN, "deliveryDateDescr",	 3, 36, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", " ", " ", " ",
		 NA, NO, JUSTRIGHT, "", "", local_rec.deliveryDateDescr},
	{1, LIN, "vehicleNo",	 4, 19, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Vehicle No        :", "Enter Vehicle No. [SEARCH]. Default is All",
		 NO, NO, JUSTLEFT, "", "", local_rec.vehicleNo},
	{1, LIN, "vehicleDescr",	 4, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.vehicleDescr},
	{1, LIN, "del_zone",	 5, 19, CHARTYPE,
		"UUUUUU", "          ", 
		" ", " ", "Delivery Zone     :", "Enter Delivery Zone. [SEARCH]. Default is All",
		 YES, NO,  JUSTLEFT, "", "", local_rec.del_zone},
	{1, LIN, "zoneDescr",	 5, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.zoneDescr},
	{1, LIN, "printerNumber",	 7, 19, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number    :", " ",
		 YES, NO, JUSTRIGHT, "", "",(char *)&local_rec.printerNumber},
    {1,LIN, "backGroundProcess",    8, 19, CHARTYPE,
		"A", "          ",
		" ", "N", "Background (Y/N)  :", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.backGroundProcess},
	{1,LIN, "overNightProcess",    9, 19, CHARTYPE,
		"A", "          ",
		" ", "N", "Overnight  (Y/N)  :", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.overNightProcess}, 
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void head_output (void);
void process_file (void);
void blankLine (void);
void SrchZone (char *);
void SrchTrzm (char *);
void SrchTrve (char *);
int spec_valid (int);
int CheckVehicleForZone (void);
int store_data (float);
int display_report (void);
int heading (int);
char *_sort_read (FILE *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	SETUP_SCR 	(vars);
	OpenDB		();

	/*=============================
	| Set up required parameters  |
	=============================*/
	
	init_scr();			
	set_tty();         
	set_masks();	
	init_vars(1);

	while (prog_exit == 0)
	{
		/*===========================	
		|    Reset control flags    |
		============================*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);		

		/*=============================
		| Entry screen 1 linear input |
		=============================*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*============================	
		| Edit screen 1 linear input |
		============================*/
		heading(1);
		scn_display(1);
		edit(1);      
		if (restart)
			continue;
		if (!restart)
		{
			dsp_screen("Printing Delivery Sheet", 
						comm_rec.tco_no, comm_rec.tco_name);
            process_file();
			display_report(); 
		}
		prog_exit = 1;
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
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
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec( trve, trve_list, TRVE_NO_FIELDS, "trve_id_no" );
	open_rec( trzm, trzm_list, TRZM_NO_FIELDS, "trzm_id_no" );
	open_rec( trhr, trhr_list, TRHR_NO_FIELDS, "trhr_id_no" );
	open_rec( cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhtr_hash" );
	open_rec( cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash" );
	open_rec( coln, coln_list, COLN_NO_FIELDS, "coln_id_no" );
	open_rec( inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash" );
	open_rec( inum, inum_list, inum_no_fields, "inum_hhum_hash");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose(trve);
	abc_fclose(trzm);
	abc_fclose(trhr);
	abc_fclose(cohr);
	abc_fclose(cumr);
	abc_fclose(coln);
	abc_fclose(inmr);
	abc_fclose(inum);

	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	int deliveryDateFound = FALSE;
	int zone_found	= FALSE;
	int vehicleFound  = FALSE;
	int date_found  = FALSE;

	/*-------------------------
	| Validate Delivery Date. |
	-------------------------*/
	if (LCHECK("deliveryDate"))
	{
		if (dflt_used)
		{
			local_rec.deliveryDate = 0L;
 			sprintf (local_rec.deliveryDateDescr, "%-9.9s", "All Dates");
			DSP_FLD ("deliveryDate");
			DSP_FLD ("deliveryDateDescr");
   			return(0);
		}
		else
		{ 
			trhr_rec.del_date 	= local_rec.deliveryDate;
			trhr_rec.hhve_hash 	= 0L;
			cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
			while (!cc)
			{
			 	if (trhr_rec.del_date == local_rec.deliveryDate)
				{
					date_found = TRUE;
					break;
				}
				cc = find_rec (trhr,&trhr_rec, NEXT, "r");
			}
			if (!date_found)
			{
				print_mess(ML(mlTrMess002));
				sleep (sleepTime);
				clear_mess();
				return(1);
			}
		}
		if (local_rec.deliveryDate == 0L)
 			sprintf (local_rec.deliveryDateDescr, "%-9.9s", "All Dates");
		else
			strcpy (local_rec.deliveryDateDescr, "         ");

 		DSP_FLD ("deliveryDate");
		DSP_FLD ("deliveryDateDescr");
		return(0);
	} 


	/*-----------------------------------------
	| Validate Vehicle Number and Allow Search. |
	-------------------------------------------*/
	if (LCHECK("vehicleNo"))
	{
		if (dflt_used)
		{ 
			strcpy (local_rec.vehicleNo, "~~~~~~~~~~");
 			sprintf (local_rec.vehicleDescr, "%-40.40s", "All Vehicle");
			DSP_FLD ("vehicleNo");
			DSP_FLD ("vehicleDescr");
   			return(0);
		}

		if (SRCH_KEY)
        {
			SrchTrve(temp_str);
			return(0);
		}

		strcpy (trve_rec.co_no, comm_rec.tco_no);
		strcpy (trve_rec.br_no, comm_rec.test_no);
		strcpy (trve_rec.vehicleNo, local_rec.vehicleNo);
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
		{
			print_mess(ML(mlTrMess001));
			sleep (sleepTime);
			clear_mess();
			return(1);
		}
		abc_selfield (trhr, "trhr_hhve_hash");
		memset (&trhr_rec, 0, sizeof (trhr_rec));
		trhr_rec.hhve_hash = trve_rec.hhve_hash;
		cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
		while (!cc && trhr_rec.hhve_hash == trve_rec.hhve_hash)
		{
			vehicleFound = TRUE;
			if (trhr_rec.del_date == local_rec.deliveryDate)
			{
				deliveryDateFound = TRUE;
				break;
			}
			cc = find_rec (trhr,&trhr_rec, NEXT, "r"); 
		}
		if (vehicleFound)
		{
			if (!deliveryDateFound && local_rec.deliveryDate != 0L)
			{
				print_mess(ML(mlTrMess028));
				sleep (sleepTime);
				clear_mess();
				abc_selfield (trhr, "trhr_id_no");
				return(1);
			}
		}
		else
		{
			print_mess(ML(mlTrMess028));
			sleep (sleepTime);
			clear_mess();
			abc_selfield (trhr, "trhr_id_no");
			return(1);
		}

		abc_selfield (trhr, "trhr_id_no");
		strcpy (local_rec.vehicleDescr, trve_rec.desc);

 		DSP_FLD ("vehicleNo");
		DSP_FLD ("vehicleDescr");
		return(0);
	}

	/*------------------------------------------
	| Validate Delivery Zone and Allow Search. |
	------------------------------------------*/
	if (LCHECK("del_zone"))
	{
		zone_found = FALSE;
		if (SRCH_KEY) 
        {
			if (!strcmp (local_rec.vehicleNo, "~~~~~~~~~~"))
			 	SrchTrzm (temp_str);
			else
				SrchZone (temp_str);
			return(0);
		}

		if (dflt_used)
		{
			strcpy (local_rec.del_zone, "~~~~~~");
 			sprintf (local_rec.zoneDescr, "%-40.40s", "All Delivery Zones");
			DSP_FLD ("del_zone");
			DSP_FLD ("zoneDescr");
   			return(0);
		}

		memset( &trzm_rec, 0, sizeof(trzm_rec));
		strcpy (trzm_rec.co_no, comm_rec.tco_no);
		strcpy (trzm_rec.br_no, comm_rec.test_no);
		sprintf (trzm_rec.del_zone,"%-6.6s",local_rec.del_zone);
		cc = find_rec (trzm, &trzm_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlTrMess059));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.vehicleNo, "~~~~~~~~~~"))
		{
			if (CheckVehicleForZone ())
			{
				print_mess (ML(mlTrMess059));
				sleep (sleepTime);
				return(1);
			}
		}
		else
		{
			strcpy (previousDelZone, cohr_rec.del_zone);
			strcpy (trzm_rec.co_no, comm_rec.tco_no);
			strcpy (trzm_rec.br_no, comm_rec.test_no);
			strcpy (trzm_rec.del_zone, local_rec.del_zone);
			cc = find_rec (trzm,&trzm_rec, COMPARISON,"r");
			if (cc)
			{
				print_mess(ML(mlTrMess029));
				sleep (sleepTime);
				clear_mess();
				abc_selfield (trhr, "trhr_id_no");
				return(1);
			}
		}

		strcpy (local_rec.zoneDescr, trzm_rec.desc);

 		DSP_FLD("del_zone");
		DSP_FLD("zoneDescr");
		return(0);
	}

    /*-------------------------
    | Validate Printer Number |
    -------------------------*/
	if ( LCHECK("printerNumber") )
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}
		return(0);
	}
	return(0);
}

/*====================================
| Check if zone is valid for vehicle |
====================================*/
int
CheckVehicleForZone (
 void)
{
	abc_selfield (trhr, "trhr_hhve_hash");
	memset (&trhr_rec, 0, sizeof (trhr_rec));
	trhr_rec.hhve_hash = trve_rec.hhve_hash;
	cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
	while (!cc && trhr_rec.hhve_hash == trve_rec.hhve_hash)
	{
		cohr_rec.hhtr_hash = trhr_rec.hhtr_hash;
		cc = find_rec (cohr,&cohr_rec, GTEQ, "r");
		while (!cc && cohr_rec.hhtr_hash == trhr_rec.hhtr_hash)
		{
			if (!strcmp (cohr_rec.del_zone, local_rec.del_zone))
				return (EXIT_SUCCESS);
			
			cc = find_rec (cohr,&cohr_rec,NEXT,"r");
		}
		cc = find_rec (trhr,&trhr_rec, NEXT, "r");
	}
	abc_selfield (trhr, "trhr_id_no");
	return (EXIT_FAILURE);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	/*------------------
	| Open format file |
	------------------*/  
	if ((fout = popen("pformat","w"))==0)
		file_err (errno,"pformat","POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	fprintf (fout, ".PI16\n");
	fprintf (fout, ".L180\n");
	fprintf (fout, ".7\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".CDELIVERY SHEET\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".CCOMPANY %s\n", clip(comm_rec.tco_name));
	fprintf (fout, ".CAS AT %s\n", SystemTime ());
	fprintf (fout, ".B2\n");
	
	fprintf (fout, ".R=============");
	fprintf (fout, "============");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "=======");
	fprintf (fout, "===============\n");

	fprintf (fout, "=============");
	fprintf (fout, "============");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===================");
	fprintf (fout, "=======");
	fprintf (fout, "===============\n");

	fprintf (fout, "|  DELIVERY  ");
	fprintf (fout, "|  VEHICLE  ");
	fprintf (fout, "| PACKING");
	fprintf (fout, "| CUSTOMER ");
	fprintf (fout, "|             DELIVERY ADDRESS             ");
	fprintf (fout, "|DELIV.");
	fprintf (fout, "|  WEIGHT  ");
	fprintf (fout, "|  ORDER   ");
	fprintf (fout, "| SCHEDULE ");
	fprintf (fout, "|    ITEM NUMBER   ");
	fprintf (fout, "|  UOM ");
	fprintf (fout, "|   QUANTITY  |\n");

	fprintf (fout, "|    DATE    ");
	fprintf (fout, "| REFERENCE ");
	fprintf (fout, "| SLIP NO");
	fprintf (fout, "|  NUMBER  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "| ZONE ");
	fprintf (fout, "|  WEIGHT  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|  UOM ");
	fprintf (fout, "|   ORDERED   |\n");

	fprintf (fout, "|------------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------");
	fprintf (fout, "|-------------|\n");

	fflush 	(fout); 
	return;
}

void
process_file (
 void)
{
	float   std_cnv_fct = 0.00;
	float   q_order  	= 0.00;
	float   cnv_fct 	= 0.00;

	fsort = sort_open("tr_delsheet");

	if (!strcmp (local_rec.vehicleNo, "~~~~~~~~~~"))
		strcpy (local_rec.vehicleNo, "          ");

	memset (&trve_rec, 0, sizeof (trve_rec));
	strcpy (trve_rec.co_no, comm_rec.tco_no);
	strcpy (trve_rec.br_no, comm_rec.test_no);
	strcpy (trve_rec.vehicleNo, local_rec.vehicleNo);
	cc = find_rec (trve,&trve_rec, GTEQ, "r");
	while (!cc && !strcmp (trve_rec.co_no, comm_rec.tco_no) 
		       && !strcmp (trve_rec.br_no, comm_rec.test_no) 
			   && (!strcmp (trve_rec.vehicleNo, local_rec.vehicleNo) 
			   || !strcmp (local_rec.vehicleNo, "          ")))
	{
		memset (&trhr_rec, 0, sizeof (trhr_rec));
		trhr_rec.del_date = local_rec.deliveryDate;
		trhr_rec.hhve_hash = trve_rec.hhve_hash;
		cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
	 	while (!cc && (trhr_rec.del_date == local_rec.deliveryDate
				   ||  local_rec.deliveryDate == 0L))
	 	{
			if (trhr_rec.hhve_hash != trve_rec.hhve_hash)
			{
				cc = find_rec (trhr,&trhr_rec, NEXT, "r");
				continue;
			}

			cohr_rec.hhtr_hash = trhr_rec.hhtr_hash;
			cc = find_rec (cohr,&cohr_rec, GTEQ, "r");
			while (!cc && cohr_rec.hhtr_hash == trhr_rec.hhtr_hash)
			{
				if (!strcmp (cohr_rec.del_zone, local_rec.del_zone) ||
				    !strcmp (local_rec.del_zone, "~~~~~~" )) 
				{
					cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
					cc = find_rec (cumr,&cumr_rec, EQUAL, "r");
					if (cc)
						file_err (cc, cumr, "DBFIND");

					coln_rec.hhco_hash = cohr_rec.hhco_hash;
					coln_rec.line_no = 0L;
					cc = find_rec (coln,&coln_rec, GTEQ, "r");
					while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash )
					{
						inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
						cc = find_rec (inmr,&inmr_rec, EQUAL, "r");
						if ( cc )
							file_err (cc, inmr, "DBFIND" );

					
						cc = find_hash(inum, &inum_rec, EQUAL, "r", 
									   inmr_rec.std_uom);
						if ( cc )
							file_err (cc, inum, "DBFIND" );

						std_cnv_fct	=	inum_rec.cnv_fct;

						cc = find_hash(inum, &inum_rec, EQUAL, "r", 
									   coln_rec.hhum_hash);
						if ( cc )
						file_err (cc, inum, "DBFIND" );

						if ( std_cnv_fct == 0.00 )
							std_cnv_fct = 1;

						cnv_fct	=	inum_rec.cnv_fct / std_cnv_fct;

						if ( cnv_fct != 0.00 )
							q_order	= coln_rec.q_order / cnv_fct;
						else
							q_order	= coln_rec.q_order;

						store_data(q_order);
						cc = find_rec (coln,&coln_rec, NEXT, "r");
					}
				}
				cc = find_rec (cohr,&cohr_rec, NEXT, "r");
			}
			cc = find_rec (trhr,&trhr_rec, NEXT, "r");
		}
		cc = find_rec (trve,&trve_rec, NEXT, "r");
	}
}

int
store_data (
 float q_order)
{
 	char data_string [300];

	char	transportDate [11],
			deliveryDate  [11],
			createDate	  [11];

	strcpy (transportDate, DateToString (trhr_rec.del_date));
	strcpy (deliveryDate,  DateToString (cohr_rec.del_date));
	strcpy (createDate,    DateToString (cohr_rec.date_raised));
	
    sprintf (data_string,"%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s%c%f%c%s%c%s%c%s%c%s%c%s%c%f\n",
				transportDate, 					1, 		/* Offset = 0 */
				trve_rec.vehicleNo,  			1,	 	/* Offset = 1 */
				cohr_rec.inv_no,  				1, 		/* Offset = 2 */
				cumr_rec.dbt_no,  				1, 		/* Offset = 3 */
				cohr_rec.dl_add[0],  			1, 		/* Offset = 4 */
				cohr_rec.dl_add[1],  			1, 		/* Offset = 5 */
				cohr_rec.dl_add[2],  			1, 		/* Offset = 6 */
				cohr_rec.del_zone,  			1, 		/* Offset = 7 */
				cohr_rec.no_kgs,  				1, 		/* Offset = 8 */
				deliveryDate,					1, 		/* Offset = 9 */
				createDate,						1, 		/* Offset = 10 */
				inmr_rec.item_no,  				1, 		/* Offset = 11 */
				inum_rec.uom,  					1, 		/* Offset = 12 */
				cohr_rec.dl_name,				1, 		/* Offset = 13 */
				q_order); 								/* Offset = 14 */

	sort_save (fsort,data_string);
    return (EXIT_SUCCESS);
}

/*-----------------------
|save offsets for each  |
| numerical field.      |
-----------------------*/
char    *
_sort_read (
 FILE *srt_fil)
{
    char    *sptr;
    char    *tptr;
    int fld_no = 1;

    sptr = sort_read (srt_fil);
    if (!sptr)
    {
        return (sptr);
    }
   	srt_offset[0] = sptr;

    tptr = sptr;
    while (fld_no < 23)
    {
        tptr = strchr (tptr, 1);
        if (!tptr)
            break;
        *tptr = 0;
        tptr++;

        srt_offset[fld_no++] = sptr + (tptr - sptr);
    }
    return (sptr);
}

int
display_report (
 void)
{
    int 	firstTimePrinted = TRUE;
    int 	firstAddressLine = TRUE;
	char 	*sptr;
	
	char    previousDate[10];
	char    previousVehicleNo[11];
	char    previousInvoiceNo[9];
	char    previousAddress[41];

	memset (previousDate, 	   0, sizeof (previousDate));
	memset (previousVehicleNo, 0, sizeof (previousVehicleNo));
	memset (previousInvoiceNo, 0, sizeof (previousInvoiceNo));

	head_output();


	fsort = sort_sort(fsort, "tr_delsheet");
	sptr =  _sort_read(fsort);
	while (sptr != (char *)0)
	{
		sprintf (xRec.Date,  		"%-10.10s", srt_offset[0]);
		sprintf (xRec.vehicleNo, 	"%-10.10s", srt_offset[1]);
		sprintf (xRec.inv_no, 		"%-8.8s",	srt_offset[2]);
		sprintf (xRec.dbt_no, 		"%-6.6s",   srt_offset[3]);
		sprintf (xRec.dl_add[0],	"%-40.40s", srt_offset[4]);
		sprintf (xRec.dl_add[1],	"%-40.40s", srt_offset[5]);
		sprintf (xRec.dl_add[2],	"%-40.40s", srt_offset[6]);
		sprintf (xRec.del_zone, 	"%-6.6s", 	srt_offset[7]);
		xRec.no_kgs = 	atof(srt_offset[8]);
		sprintf (xRec.Date1, 		"%-10.10s", srt_offset[9]);
		sprintf (xRec.Date2, 		"%-10.10s", srt_offset[10]);
		sprintf (xRec.item_no, 		"%-16.16s", srt_offset[11]);
		sprintf (xRec.uom, 			"%-4.4s", 	srt_offset[12]);
		sprintf (xRec.dl_name,		"%-40.40s", srt_offset[13]);
		xRec.q_order = 	atof(srt_offset[14]);
		
		if (strcmp (xRec.Date , previousDate))
		{
			if (!firstTimePrinted)
			{
				blankLine();
				firstTimePrinted = TRUE;
			}
			fprintf(fout, "| %-10.10s ", xRec.Date);
		}
		else
		{
			firstTimePrinted = FALSE;
			fprintf(fout, "| %-10.10s ", " ");
		}

		
		if (strcmp (previousVehicleNo, xRec.vehicleNo))
		{
			if (!firstTimePrinted)
			{
				fprintf(fout, "|           ");
				blankLine();	
				fprintf(fout, "|            ");
			}
			fprintf(fout, "| %-10.10s", xRec.vehicleNo);
		}
		else
		{
			if (strcmp (xRec.Date , previousDate))
				fprintf(fout, "| %-10.10s", xRec.vehicleNo);
			else	
				fprintf(fout, "|           ");
		}

		if (strcmp (xRec.inv_no,previousInvoiceNo))
		{
			if (!firstTimePrinted && !strcmp (xRec.vehicleNo,previousVehicleNo))
			{
				blankLine();
				fprintf(fout, "| %-10.10s ", " ");
				fprintf(fout, "|           ");
			}

			fprintf(fout, "|%-8.8s", xRec.inv_no);
			fprintf(fout, "|  %-6.6s  ", xRec.dbt_no);
			if (strlen(clip(xRec.dl_name)) !=0)
				fprintf(fout, "| %-40.40s ", xRec.dl_name);

			fprintf(fout, "|%-6.6s", xRec.del_zone);
			fprintf(fout, "|%-10.10s", 
				comma_fmt((double)	xRec.no_kgs, "NNN,NNN.NN"));

			fprintf(fout, "|%-10.10s", xRec.Date1);
			fprintf(fout, "|%-10.10s", xRec.Date2);

		}
		else
		{
			fprintf(fout, "|%-8.8s", " ");
			fprintf(fout, "|  %-6.6s  ", " ");
			if (strlen(clip(xRec.dl_add [0])) !=0 && firstAddressLine)
			{
				if (strlen(clip(xRec.dl_add [0])) 	!=0 ) 
				{
					fprintf (fout, "| %-40.40s ", xRec.dl_add [0]);
					fprintf (fout, "|      ");
					fprintf (fout, "|          ");
					fprintf (fout, "|          ");
					fprintf (fout, "|          ");
				}
				firstAddressLine = FALSE;
			}
			else
			{
				fprintf(fout, "|                                          " );
				fprintf(fout, "|  %-2.2s  ", " ");
				fprintf(fout, "|%-10.10s"," "); 
				fprintf(fout, "|%-10.10s", " ");
				fprintf(fout, "|%-10.10s", " ");
			}
		}

		fprintf(fout, "| %-16.16s ", xRec.item_no);
		fprintf(fout, "| %-4.4s ", xRec.uom);
		fprintf(fout, "| %-10.10s  |\n", 
			comma_fmt((double)	xRec.q_order, "NNN,NNN.NN"));

		fprintf (fout, ".LRP4\n");
		
		strcpy (previousInvoiceNo, xRec.inv_no);
		strcpy (previousDate, xRec.Date);
		strcpy (previousAddress, xRec.dl_add [0]);
		sprintf (previousVehicleNo, "%-10.10s", xRec.vehicleNo);
		sptr	=	_sort_read (fsort);
	}

	fflush (fout);
	sort_delete (fsort, "tr_delsheet");
	return (EXIT_SUCCESS); 
}


void
blankLine (
 void)
{
	fprintf (fout, "|        ");
	fprintf (fout, "|          ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|      ");
	fprintf (fout, "|          ");
	fprintf (fout, "|          ");
	fprintf (fout, "|          ");
	fprintf (fout, "|                  ");
	fprintf (fout, "|      ");
	fprintf (fout, "|             |\n");
}

/*==================
| Search for trve. |
==================*/
void
SrchTrve (
 char	*key_val)
{
	int 	hhve_found = FALSE;

	work_open();
	save_rec ("#Vehicle", "#Description");
	strcpy (trve_rec.co_no, comm_rec.tco_no);
	strcpy (trve_rec.br_no, comm_rec.test_no);
	sprintf (trve_rec.vehicleNo,"%-10.10s",key_val);
	cc = find_rec (trve,&trve_rec,GTEQ,"r");
	while (!cc && !strcmp ( trve_rec.co_no, comm_rec.tco_no) &&
		      !strncmp(trve_rec.vehicleNo,key_val,strlen(key_val)))
	{
		hhve_found = FALSE;
		memset (&trhr_rec, 0, sizeof (trhr_rec));
		trhr_rec.del_date = local_rec.deliveryDate;
		trhr_rec.hhve_hash = trve_rec.hhve_hash;
		cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
	 	while (!cc && (trhr_rec.del_date == local_rec.deliveryDate
				   ||  local_rec.deliveryDate == 0L))
	 	{
			if (trhr_rec.hhve_hash == trve_rec.hhve_hash)
			{
				hhve_found = TRUE;
				break;
			}
			cc = find_rec (trhr,&trhr_rec, NEXT, "r");
		}
		if (hhve_found)
		{
			cc = save_rec(trve_rec.vehicleNo, trve_rec.desc);
			if (cc)
				break; 
			hhve_found = FALSE;
		}
		cc = find_rec (trve,&trve_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trve_rec.co_no, comm_rec.tco_no);
	strcpy (trve_rec.br_no, comm_rec.test_no);
	sprintf (trve_rec.vehicleNo,"%-10.10s", temp_str);
	cc = find_rec (trve,&trve_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, trve, "DBFIND" );
}

/*======================================
| Search for trzm - if default is All. |
======================================*/
void
SrchTrzm (
 char *key_val)
{
	work_open();
	save_rec ("#Zone Code","#Description");
	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.test_no);
	sprintf (trzm_rec.del_zone, "%-6.6s",key_val);
	cc = find_rec (trzm,&trzm_rec,GTEQ,"r");
	while (!cc && !strcmp ( trzm_rec.co_no, comm_rec.tco_no) &&
				  !strcmp ( trzm_rec.br_no, comm_rec.test_no) &&
	      		  !strncmp(trzm_rec.del_zone,key_val,strlen(key_val)))
	{
		cc = save_rec(trzm_rec.del_zone, trzm_rec.desc);
		if (cc)
			break;

		cc = find_rec (trzm,&trzm_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.test_no);
	sprintf (trzm_rec.del_zone, "%-6.6s",temp_str);
	cc = find_rec (trzm,&trzm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, trzm, "DBFIND" );
}

/*=======================================
| Search for trzm - if entered a value. |
=======================================*/
void
SrchZone (
 char *key_val)
{
	work_open();
	save_rec("#CD","#Description");
	strcpy (trve_rec.co_no, comm_rec.tco_no);
	strcpy (trve_rec.br_no, comm_rec.test_no);
	strcpy (trve_rec.vehicleNo, local_rec.vehicleNo);
	cc = find_rec (trve,&trve_rec, COMPARISON, "r");
	if (!cc && 	!strcmp (trve_rec.co_no, comm_rec.tco_no) &&
				!strcmp (trve_rec.br_no, comm_rec.test_no) &&
				!strcmp (trve_rec.vehicleNo, local_rec.vehicleNo))
	{
		abc_selfield (trhr, "trhr_hhve_hash");
		trhr_rec.hhve_hash = trve_rec.hhve_hash;
		cc = find_rec (trhr,&trhr_rec, GTEQ, "r");
		while (!cc && trhr_rec.hhve_hash == trve_rec.hhve_hash)
		{
			if (local_rec.deliveryDate > 0L && 
				local_rec.deliveryDate != trhr_rec.del_date)
			{
				cc = find_rec (trhr,&trhr_rec, NEXT, "r");
				continue;
			}
			cohr_rec.hhtr_hash = trhr_rec.hhtr_hash;
			cc = find_rec (cohr,&cohr_rec, GTEQ, "r");
			while (!cc && cohr_rec.hhtr_hash == trhr_rec.hhtr_hash)
			{
				if (!strcmp (previousDelZone, cohr_rec.del_zone))
				{
					cc = find_rec (cohr,&cohr_rec,NEXT,"r");
					continue;
				}
				strcpy (previousDelZone, cohr_rec.del_zone);
				strcpy (trzm_rec.co_no, cohr_rec.co_no);
				strcpy (trzm_rec.br_no, cohr_rec.br_no);
				strcpy (trzm_rec.del_zone, cohr_rec.del_zone);
				cc = find_rec (trzm,&trzm_rec,GTEQ,"r");
				while (!cc && !strcmp ( trzm_rec.co_no, cohr_rec.br_no) &&
					  	      !strcmp (trzm_rec.del_zone, cohr_rec.del_zone))
				{
					cc = save_rec(trzm_rec.del_zone,trzm_rec.desc);
					if (cc)
						break;

					cc = find_rec (trzm,&trzm_rec,NEXT,"r");
				}
				cc = find_rec (cohr,&cohr_rec,NEXT,"r");
			}
			cc = find_rec (trhr,&trhr_rec, NEXT, "r");
		}
		abc_selfield (trhr, "trhr_id_no");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (trzm_rec.co_no, comm_rec.tco_no);
	strcpy (trzm_rec.br_no, comm_rec.test_no);
	sprintf (trzm_rec.del_zone, "%-6.6s", temp_str);
	cc = find_rec (trzm,&trzm_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, trzm, "DBFIND" );
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);       /* Set a screen ready for manipulation */

		strcpy (err_str, ML(mlTrMess030));

		clear();
		rv_pr (err_str, (80 - strlen (err_str)) / 2, 0, 1);
        move (0,1);
        line (81);

		box(0,2,80,7);
		move (1,6);
		line (79);
 
        move (0,20);       
        line (80);
		strcpy (err_str,ML(mlStdMess038));
		print_at (21,1,err_str,comm_rec.tco_no,comm_rec.tco_short);
		strcpy (err_str,ML(mlStdMess039));
		print_at (21,30,err_str,comm_rec.test_no,comm_rec.test_short);
		strcpy (err_str,ML(mlStdMess099));
		print_at (21,55,err_str,comm_rec.tcc_no,comm_rec.tcc_short);
        move (0,22);
        line (80);
		scn_write(scn);   /* Display all screen prompts */
	}

    return (EXIT_SUCCESS);
}

