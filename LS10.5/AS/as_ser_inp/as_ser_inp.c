/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_ser_inp   )                                   |
|  Program Desc  : ( Asset Service Maintenance & Display          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asmr, assp, asbr, asty, assc, asdt, asst    |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  asdt,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 08/09/94         |
|---------------------------------------------------------------------|
|  Date Modified : (03/03/95)      | Modified by : Ross S Baquillos.  |
|  Date Modified : (06/02/98)      | Modified by : Elena Cuaresma.    |
|                                                                     |
|  Comments      :  SMF 00130 - add validation for service date with  |
|  (03/03/95)    :              purchase date and current date.       |
|  (06/02/98)    :  9.10  New asset module for the standard version 9.|
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: as_ser_inp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_ser_inp/as_ser_inp.c,v 5.4 2002/07/18 06:07:09 scott Exp $";

#define 	MAXSCNS			2
#define 	MAXLINES		500
#define 	TABLINES		16

#define		SLEEP_TIME		3

#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>
#include <DateToString.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#if 0
#define	SEL_DELETE	2
#endif
#define	SEL_DEFAULT	99


	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
	};

	int	comm_no_fields = 4;

	struct tag_commRecord
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
	} comm_rec;

	/*=====================+
	 | Asset Master File |
	 +=====================*/
#define	ASMR_NO_FIELDS	18

	struct dbview	asmr_list [ASMR_NO_FIELDS] =
	{
		{"asmr_co_no"},
		{"asmr_br_no"},
		{"asmr_ass_group"},
		{"asmr_ass_no"},
		{"asmr_serial_no"},
		{"asmr_type"},
		{"asmr_brand"},
		{"asmr_pur_date"},
		{"asmr_hhas_hash"},
		{"asmr_desc"},
		{"asmr_spec1_code"},
		{"asmr_spec2_code"},
		{"asmr_crd_no"},
		{"asmr_hhsu_hash"},
		{"asmr_capacity"},
		{"asmr_status_code"},
		{"asmr_products"},
		{"asmr_hham_hash"}
	};

	struct tag_asmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	ass_group[6];
		char	ass_no[6];
		char	serial_no [26];
		char	type [4];
		char	brand [9];
		Date	pur_date;
		long	hhas_hash;
		char	desc [81];
		char	spec1_code [9];
		char	spec2_code [9];
		char	crd_no [7];
		long	hhsu_hash;
		double	capacity;
		char	status_code [3];
		char	products [81];
		long	hham_hash;
	}	asmr_rec;

	/*======+
	 | famr |
	 +======*/
#define	FAMR_NO_FIELDS	33

	struct dbview	famr_list [FAMR_NO_FIELDS] =
	{
		{"famr_co_no"},
		{"famr_ass_group"},
		{"famr_ass_no"},
		{"famr_famr_hash"},
		{"famr_ass_desc1"},
		{"famr_ass_desc2"},
		{"famr_ass_desc3"},
		{"famr_ass_desc4"},
		{"famr_pur_date"},
		{"famr_ass_life"},
		{"famr_cost_price"},
		{"famr_disp_date"},
		{"famr_disp_price"},
		{"famr_gl_crd_acc"},
		{"famr_gl_dbt_acc"},
		{"famr_gl_ass_acc"},
		{"famr_f_y_rule"},
		{"famr_f_y_amt"},
		{"famr_dep_rule"},
		{"famr_max_deprec"},
		{"famr_priv_use_tax"},
		{"famr_tax_open_val"},
		{"famr_tax_dtype"},
		{"famr_tax_pa_flag"},
		{"famr_tax_d_pc"},
		{"famr_tax_d_amt"},
		{"famr_int_open_val"},
		{"famr_int_dtype"},
		{"famr_int_pa_flag"},
		{"famr_int_d_pc"},
		{"famr_int_d_amt"},
		{"famr_gl_updated"},
		{"famr_stat_flag"}
	};

	struct tag_famrRecord
	{
		char	co_no [3];
		char	ass_group [6];
		char	ass_no [6];
		long	famr_hash;
		char	ass_desc [4] [41];
		Date	pur_date;
		char	ass_life [8];
		Money	cost_price;
		Date	disp_date;
		Money	disp_price;
		char	gl_crd_acc [17];
		char	gl_dbt_acc [17];
		char	gl_ass_acc [17];
		char	f_y_rule [2];
		Money	f_y_amt;
		char	dep_rule [2];
		Money	max_deprec;
		float	priv_use_tax;
		Money	tax_open_val;
		char	tax_dtype [2];
		char	tax_pa_flag [2];
		float	tax_d_pc;
		Money	tax_d_amt;
		Money	int_open_val;
		char	int_dtype [2];
		char	int_pa_flag [2];
		float	int_d_pc;
		Money	int_d_amt;
		Date	gl_updated;
		char	stat_flag [2];
	}	famr_rec;


	/*======+
	 | fatr |
	 +======*/
#define	FATR_NO_FIELDS	12

	struct dbview	fatr_list [FATR_NO_FIELDS] =
	{
		{"fatr_co_no"},
		{"fatr_group"},
		{"fatr_group_desc"},
		{"fatr_dep_rule"},
		{"fatr_nxt_asset"},
		{"fatr_ass_life"},
		{"fatr_max_depr"},
		{"fatr_tax_dtype"},
		{"fatr_tax_pa_flag"},
		{"fatr_int_dtype"},
		{"fatr_int_pa_flag"},
		{"fatr_stat_flag"}
	};

	struct tag_fatrRecord
	{
		char	co_no [3];
		char	group [6];
		char	group_desc [41];
		char	dep_rule [2];
		long	nxt_asset;
		char	ass_life [8];
		Money	max_depr;
		char	tax_dtype [2];
		char	tax_pa_flag [2];
		char	int_dtype [2];
		char	int_pa_flag [2];
		char	stat_flag [2];
	}	fatr_rec;

	/*============================+
	 | Asset Specification File |
	 +============================*/
#define	ASSP_NO_FIELDS	3

	struct dbview	assp_list [ASSP_NO_FIELDS] =
	{
		{"assp_co_no"},
		{"assp_spec_code"},
		{"assp_desc"}
	};

	struct tag_asspRecord
	{	
		char	co_no [3];
		char	spec_code [9];
		char	desc [41];
	}	assp_rec;

	/*====================+
	 | Asset Brand File |
	 +====================*/
#define	ASBR_NO_FIELDS	3

	struct dbview	asbr_list [ASBR_NO_FIELDS] =
	{
		{"asbr_co_no"},
		{"asbr_brand_code"},
		{"asbr_brand_desc"}
	};

	struct tag_asbrRecord
	{
		char	co_no [3];
		char	brand_code [9];
		char	brand_desc [41];
	}	asbr_rec;

	/*===================+
	 | Asset Type File |
	 +===================*/
#define	ASTY_NO_FIELDS	3

	struct dbview	asty_list [ASTY_NO_FIELDS] =
	{
		{"asty_co_no"},
		{"asty_type_code"},
		{"asty_type_desc"}
	};

	struct tag_astyRecord
	{
		char	co_no [3];
		char	type_code [4];
		char	type_desc [41];
	}	asty_rec;

	/*==========================+
	 | Asset Status Code File |
	 +==========================*/
#define	ASSC_NO_FIELDS	3

	struct dbview	assc_list [ASSC_NO_FIELDS] =
	{
		{"assc_co_no"},
		{"assc_stat_code"},
		{"assc_stat_desc"}
	};

	struct tag_asscRecord
	{
		char	co_no [3];
		char	stat_code [3];
		char	stat_desc [41];
	}	assc_rec;

	/*=====================+
	 | Asset Detail File |
	 +=====================*/
#define	ASDT_NO_FIELDS	5

	struct dbview	asdt_list [ASDT_NO_FIELDS] =
	{
		{"asdt_hhar_hash"},
		{"asdt_line_no"},
		{"asdt_ser_date"},
		{"asdt_ser_type"},
		{"asdt_remarks"}
	};

	struct tag_asdtRecord
	{
		long	hhas_hash;
		int		line_no;
		Date	ser_date;
		char	ser_type [3];
		char	remarks [61];
	}	asdt_rec;


	/*===========================+
	 | Asset Service Type File |
	 +===========================*/
#define	ASST_NO_FIELDS	3

	struct dbview	asst_list [ASST_NO_FIELDS] =
	{
		{"asst_co_no"},
		{"asst_ser_code"},
		{"asst_ser_desc"}
	};

	struct tag_asstRecord
	{
		char	co_no [3];
		char	ser_code [3];
		char	ser_desc [41];
	}	asst_rec;

char systemDate[11];
long lsystemDate;

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/

	char	*data  = "data";
	char	*assp  = "assp";
	char	*asmr  = "asmr";
	char	*famr  = "famr";
	char	*fatr  = "fatr";
	char	*asbr  = "asbr";
	char	*asty  = "asty";
	char	*assc  = "assc";
	char	*asdt  = "asdt";
	char	*asst  = "asst";

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
		char	dummy [11];
		char	desc1 [41];
		char	desc2 [41];
		char	brand_desc [41];
		char	spec1_desc [41];
		char	spec2_desc [41];
		char	crd_name [41];
		char	type_desc [41];
		char	status_desc [41];
		long	ser_date;
		char    ser_type [3];
		char	remarks [61];
		char	assetGroup [6];
		char	assetNumber [6];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "AssetGroup",	 2, 21, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset Group Code.  ", "Enter Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.assetGroup},
	{1, LIN, "group_desc",	 2, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", fatr_rec.group_desc},
	{1, LIN, "AssetNumber",	 3, 21, CHARTYPE,
		"UUUUU", "          ",
		"0", " ", "Asset number. 	 ", "Enter Asset number. <Default = new asset> [SEARCH] available. ",
		 NE, NO,  JUSTRIGHT, "0123456789", "", local_rec.assetNumber},
	{1, LIN, "serial_no",	 4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Serial No.  ", "Enter the Asset Serial Number",
		NE, NO,  JUSTLEFT, "", "", asmr_rec.serial_no},
	{1, LIN, "desc1",	 5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Description ", "",
		NA, NO, JUSTLEFT, "", "", local_rec.desc1},
	{1, LIN, "desc2",	 6, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "brand",	 8, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Brand Code  ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.brand},
	{1, LIN, "brand_desc",	 8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.brand_desc},
	{1, LIN, "spec1_code",	 10, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Spec One    ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.spec1_code},
	{1, LIN, "spec1_desc",	 10, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.spec1_desc},
	{1, LIN, "spec2_code",	 11, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Model No.   ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.spec2_code},
	{1, LIN, "spec2_desc",	 11, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.spec2_desc},
	{1, LIN, "type",	12, 21, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Asset Type        ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.type},
	{1, LIN, "type_desc",	12, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.type_desc},
	{1, LIN, "status_code",	13, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Status Code         ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.status_code},
	{1, LIN, "status_desc",	13, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.status_desc},
	{2, TAB, "ser_date",	MAXLINES, 1, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "Service Date", "Enter the Date the Asset was serviced",
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.ser_date},
	{2, TAB, "ser_type",	0, 0, CHARTYPE,
		"UU", "          ",
		" ", "", "TY", "Enter Asset Service Type Code  [SEARCH] for valid codes",
		YES, NO, JUSTLEFT, "", "", local_rec.ser_type},
	{2, TAB, "remarks",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "                     Service Remarks                        ", "Enter comments related to the service of Asset condition",
		YES, NO, JUSTLEFT, "", "", local_rec.remarks},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int spec_valid (int field);
void show_local (void);
void SrchFatr (char *key_val);
void SrchFamr (char *key_val);
void show_asmr (char *key_val);
void SrchAsst (char *key_val);
void load_asdt (void);
int heading (int scn);

static BOOL IsSpaces (char *str);
static void shutdown_prog (void);
static void OpenDB (void);
static void CloseDB (void);
static void update (void);

static BOOL	rec_found = FALSE;

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

long purchase_date = 0L;

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *)&comm_rec);

	set_masks();

	lsystemDate = TodaysDate ();
	strcpy (systemDate, DateToDDMMYY (lsystemDate));

	tab_row = 2;
	tab_col = 0;

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*-----------------------
        | Reset control flags
        -----------------------*/
        entry_exit = FALSE;
        edit_exit = FALSE;
        restart = FALSE;
        search_ok = TRUE;

		init_vars(1);
		init_vars(2);
		lcount [2] = 0;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry(1);

		if (prog_exit || restart)
			continue;
		
		heading (1);
		scn_display( 1 );
		edit(1);


		heading(2);
		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		scn_display(2);
		if (lcount[ 2 ] == 0)
			entry ( 2 );
		else
			edit ( 2 );

		if (!prog_exit && !restart)
		{
			edit_all();

			/*-----------------
			| Update records. |
			-----------------*/
			if (!prog_exit && !restart)
				update();
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
static void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
static void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (assp, assp_list, ASSP_NO_FIELDS, "assp_id_no");
	open_rec (asbr, asbr_list, ASBR_NO_FIELDS, "asbr_id_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no"); 
	open_rec (fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no"); 
	open_rec (asty, asty_list, ASTY_NO_FIELDS, "asty_id_no");
	open_rec (assc, assc_list, ASSC_NO_FIELDS, "assc_id_no");
	open_rec (asdt, asdt_list, ASDT_NO_FIELDS, "asdt_id_no");
	open_rec (asst, asst_list, ASST_NO_FIELDS, "asst_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	abc_fclose (assp);
	abc_fclose (asbr);
	abc_fclose (asmr);
	abc_fclose (famr);
	abc_fclose (fatr);
	abc_fclose (asty);
	abc_fclose (assc);
	abc_fclose (asdt);
	abc_fclose (asst);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{

	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("AssetGroup"))
	{
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (fatr_rec.co_no,comm_rec.tco_no);
		strcpy (fatr_rec.group,local_rec.assetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML("Asset group not found."));
			return (EXIT_FAILURE);
		}

		DSP_FLD ("group_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Fixed Asset number. |
	------------------------------*/
	if (LCHECK ("AssetNumber"))
	{

		if (SRCH_KEY)
		{
			SrchFamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (famr_rec.co_no,		comm_rec.tco_no);
		strcpy (famr_rec.ass_group,	local_rec.assetGroup);
		strcpy (famr_rec.ass_no,	local_rec.assetNumber);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML("Asset number not found."));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Asset Serial No. |
	----------------------------*/
	if (LCHECK ("serial_no"))
	{
		if (last_char == SEARCH)
		{
			show_asmr (temp_str);
			return 0;
		}

		if (!IsSpaces (asmr_rec.serial_no))
		{
			strcpy (asmr_rec.co_no, comm_rec.tco_no);
			strcpy (asmr_rec.br_no, comm_rec.test_no);
			strcpy (asmr_rec.ass_group, local_rec.assetGroup);
			strcpy (asmr_rec.ass_no,    local_rec.assetNumber);

			cc = find_rec (asmr, &asmr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML("A valid asset serial number must be entered."));
				sleep (sleepTime);
				clear_mess();
				rec_found = FALSE;
				return(1);
			}	
			else
			{
				purchase_date = asmr_rec.pur_date;
				show_local ();
				load_asdt ();
				rec_found = TRUE;
			}
		}
		else
			return (EXIT_FAILURE);

		DSP_FLD ("serial_no");
		return(0);
	}

	if (LCHECK ("ser_date"))
	{
		if (local_rec.ser_date > StringToDate(systemDate))
		{
			print_mess(ML("Service date must not be geater than today's date."));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		else if (local_rec.ser_date < purchase_date)
		{
			print_mess(ML("Service date must not be less than asset purchase date."));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		else
			return(0);
	}

	/*----------------------------
	| Validate Service Type Code |
	----------------------------*/
	if (LCHECK ("ser_type"))
	{
		if (SRCH_KEY)
		{
			SrchAsst (temp_str);
			return (EXIT_SUCCESS);
		}

		memset (&asst_rec, 0, sizeof asst_rec);
		if (!IsSpaces (local_rec.ser_type))
		{
			strcpy (asst_rec.co_no, comm_rec.tco_no);
			strcpy (asst_rec.ser_code, local_rec.ser_type);
			cc = find_rec (asst, &asst_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess(ML("A valid asset service type code must be entered."));
				sleep (sleepTime);
				clear_mess();
				return(1);
			}	
		}
		strcpy (local_rec.ser_type, asst_rec.ser_code);
		DSP_FLD ("ser_type");
		return(0);
	}


	if (LCHECK ("remarks"))
	{
		return(0);
	}

    return (EXIT_SUCCESS);
}

void
show_local (void)
{
	sprintf (local_rec.desc1,"%-40.40s", asmr_rec.desc);
	sprintf (local_rec.desc2,"%-40.40s",asmr_rec.desc+40);

	memset (&asbr_rec, 0, sizeof asbr_rec);
	if (!IsSpaces (asmr_rec.brand))
	{
		strcpy (asbr_rec.co_no, comm_rec.tco_no);
		strcpy (asbr_rec.brand_code, asmr_rec.brand);
		cc = find_rec (asbr, &asbr_rec, COMPARISON , "r");
	}
	strcpy (local_rec.brand_desc,asbr_rec.brand_desc);
	
	memset (&assp_rec, 0, sizeof assp_rec);
	if (!IsSpaces (asmr_rec.spec1_code))
	{
		strcpy (assp_rec.co_no, comm_rec.tco_no);
		strcpy (assp_rec.spec_code, asmr_rec.spec1_code);
		find_rec (assp, &assp_rec, COMPARISON , "r");
	}
	strcpy (local_rec.spec1_desc,assp_rec.desc);
	
	memset (&assp_rec, 0, sizeof assp_rec);
	if (!IsSpaces (asmr_rec.spec2_code))
	{
		strcpy (assp_rec.co_no, comm_rec.tco_no);
		strcpy (assp_rec.spec_code, asmr_rec.spec2_code);
		cc = find_rec (assp, &assp_rec, COMPARISON , "r");
	}
	strcpy (local_rec.spec2_desc,assp_rec.desc);

	memset (&asty_rec, 0, sizeof asty_rec);
	if (!IsSpaces (asmr_rec.type))
	{
		strcpy (asty_rec.co_no, comm_rec.tco_no);
		strcpy (asty_rec.type_code, asmr_rec.type);
		cc = find_rec (asty, &asty_rec, COMPARISON , "r");
	}
	strcpy (local_rec.type_desc,asty_rec.type_desc);

	memset (&assc_rec, 0, sizeof assc_rec);
	if (!IsSpaces (asmr_rec.status_code))
	{
		strcpy (assc_rec.co_no, comm_rec.tco_no);
		strcpy (assc_rec.stat_code, asmr_rec.status_code);
		cc = find_rec (assc, &assc_rec, EQUAL, "r");
	}
	strcpy (local_rec.status_desc,assc_rec.stat_desc);

}

/*===========================================
| Search for Fixed Asset Group master file. |
===========================================*/
void
SrchFatr (
 char*              key_val)
{
	work_open ();
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, 	comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.tco_no))
	{
		cc = save_rec (fatr_rec.group, fatr_rec.group_desc);
		if (cc)
			break;

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (fatr_rec.group, "%-5.5s", " ");
		return;
	}
	strcpy (fatr_rec.co_no, comm_rec.tco_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, fatr, "DBFIND");
}
/*============================================
| Search for Fixed Asset Number master file. |
============================================*/
void
SrchFamr (
 char*              key_val)
{
	work_open ();
	save_rec ("#Asset No", "#Asset Description");
	strcpy (famr_rec.co_no, 	comm_rec.tco_no);
	strcpy (famr_rec.ass_group, local_rec.assetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", key_val);
	cc = find_rec (famr, &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, 		comm_rec.tco_no) &&
				  !strcmp (famr_rec.ass_group, 	local_rec.assetGroup) &&
				  !strncmp (famr_rec.ass_no, key_val, strlen (key_val)))
	{
		cc = save_rec (famr_rec.ass_no, famr_rec.ass_desc [0]);
		if (cc)
			break;

		cc = find_rec (famr, &famr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (famr_rec.ass_no, "%-5.5s", " ");
		return;
	}
	strcpy (famr_rec.co_no, 	comm_rec.tco_no);
	strcpy (famr_rec.ass_group, local_rec.assetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", temp_str);
	cc = find_rec (famr, &famr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, famr, "DBFIND");
}

/*=======================================
| Search Serial No                        
=======================================*/
void
show_asmr (
 char*              key_val)
{
	char desc[41];

	work_open();
	save_rec("#Serial_no","#Description");

	strcpy (asmr_rec.co_no, comm_rec.tco_no);
	strcpy (asmr_rec.br_no, comm_rec.test_no);
	strcpy (asmr_rec.ass_group, local_rec.assetGroup);
	strcpy (asmr_rec.ass_no,    local_rec.assetNumber);
	strcpy (asmr_rec.serial_no, key_val);
	cc = find_rec (asmr, &asmr_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asmr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (asmr_rec.br_no, comm_rec.test_no) &&
			!strcmp (asmr_rec.ass_group, local_rec.assetGroup) &&
			!strcmp (asmr_rec.ass_no, local_rec.assetNumber) &&
			!strncmp (asmr_rec.serial_no,key_val,strlen(key_val)))
	{
		sprintf (desc, "%-40.40s", asmr_rec.desc);
		cc = save_rec (asmr_rec.serial_no, desc);
		if (cc)
			break;

		cc = find_rec (asmr, &asmr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();
	if (cc)
		return;

	strcpy (asmr_rec.co_no, comm_rec.tco_no);
	strcpy (asmr_rec.br_no, comm_rec.test_no);
	strcpy (asmr_rec.ass_group, local_rec.assetGroup);
	strcpy (asmr_rec.ass_no,    local_rec.assetNumber);
	strcpy (asmr_rec.serial_no,temp_str);
	cc = find_rec (asmr, &asmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, asmr, "DBFIND");
}

	
/*=============================================
| Search Asset System Specification Code File
===============================================*/
void
SrchAsst (
 char*              key_val)
{
	struct tag_asstRecord asst_bak;

	memcpy (&asst_bak, &asst_rec, sizeof asst_bak);

	work_open();
	save_rec("#Code","#Description");

	strcpy (asst_rec.co_no, comm_rec.tco_no);
	strcpy (asst_rec.ser_code, key_val);

	cc = find_rec (asst, &asst_rec, GTEQ, "r");

	while (!cc &&
				!strcmp (asst_rec.co_no, comm_rec.tco_no) &&
				!strncmp (asst_rec.ser_code, key_val, strlen(key_val)))
	{
		cc = save_rec (asst_rec.ser_code, asst_rec.ser_desc);
		if (cc)
			break;

		cc = find_rec (asst, &asst_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();

	if (!cc)
	{
		/*-----------------------
		| Read selected record
		-----------------------*/
		strcpy (asst_rec.co_no, comm_rec.tco_no);
		strcpy (asst_rec.ser_code, temp_str);
		cc = find_rec (asst, &asst_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, asst, "DBFIND");
	}

	if (cc)
		memcpy (&asst_rec, &asst_bak, sizeof asst_rec);
}


/*==================================
| Load asdt into local for screen 2 |
==================================*/
void
load_asdt (void)
{
	/*----------------------------
	| Set screen 2 - for putval. |
	----------------------------*/
	init_vars(2);
	lcount[2] = 0;
	
	memset (&asdt_rec, 0, sizeof asdt_rec);
	asdt_rec.hhas_hash = asmr_rec.hhas_hash;
	asdt_rec.line_no = 0;
	cc = find_rec (asdt, &asdt_rec, GTEQ, "r");
	while (!cc && asdt_rec.hhas_hash == asmr_rec.hhas_hash)
	{
		local_rec.ser_date = asdt_rec.ser_date ;
		strcpy (local_rec.ser_type, asdt_rec.ser_type) ;
		strcpy (local_rec.remarks, asdt_rec.remarks) ;

		putval( lcount[2]++);
		asdt_rec.line_no = lcount[2];
		cc = find_rec (asdt, &asdt_rec, NEXT, "r");
	}
	scn_set(1);
}


/*==================
| Updated records. |
==================*/
void
update (void)
{
int line_no;

	scn_set (2);
	for ( line_no = 0;  line_no < lcount [ 2 ] ; line_no++ )
	{
		getval (line_no);

		memset (&asdt_rec, 0, sizeof asdt_rec);
		asdt_rec.hhas_hash = asmr_rec.hhas_hash;
		asdt_rec.line_no = line_no;
		cc = find_rec (asdt, &asdt_rec, EQUAL, "u");
		asdt_rec.ser_date = local_rec.ser_date;
		strcpy (asdt_rec.ser_type, local_rec.ser_type);
		strcpy (asdt_rec.remarks, local_rec.remarks);

		if (cc) 
		{
			cc = abc_add (asdt, &asdt_rec);
			if (cc) 
				file_err(cc, asdt, "DBADD");
		}
		else
		{
			cc = abc_update (asdt, &asdt_rec);
			if (cc) 
				file_err(cc, asdt, "DBUPDATE");
		}
	}
	abc_unlock (asdt);
}


/*===========================
| edit () callback function |
===========================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
	
		clear();

		if (scn == 1)
		{
			centre_at (0,80,ML("%R Asset Service Maintenance "));
			box (0, 1, 80, 12);
			move (1, 7); line(79);
			move (1, 9); line(79);
		}
		else
		{
			centre_at (0,80,ML("%R Asset Service Details "));
			move (0, 1); line (80);
		}

		move (0, 20); line(80);
		print_at(21,0, ML(mlStdMess038), comm_rec.tco_no,comm_rec.tco_name);
		move (0, 22); line(80);
		move(1,input_row);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
