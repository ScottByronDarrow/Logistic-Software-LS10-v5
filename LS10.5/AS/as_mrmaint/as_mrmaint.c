/*=====================================================================
|  Copyright (C) 1999 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: as_mrmaint.c,v 5.4 2002/09/09 05:12:27 scott Exp $
|  Program Name  : (as_mrmaint)
|  Program Desc  : (Asset Specification Maintenance) 
|---------------------------------------------------------------------|
|  Author        : Lawrence Barnes | Date Written  : 08/09/94         |
|---------------------------------------------------------------------|
| $Log: as_mrmaint.c,v $
| Revision 5.4  2002/09/09 05:12:27  scott
| General Maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: as_mrmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_mrmaint/as_mrmaint.c,v 5.4 2002/09/09 05:12:27 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <minimenu.h>
#include <DateToString.h>

typedef int BOOL;

#define	SEL_UPDATE	0
#define	SEL_IGNORE	1
#define	DEFAULT	99

#include	"schema"

struct commRecord	comm_rec;
struct asmrRecord	asmr_rec;
struct famrRecord	famr_rec;
struct fatrRecord	fatr_rec;
struct asspRecord	assp_rec;
struct asbrRecord	asbr_rec;
struct astyRecord	asty_rec;
struct asscRecord	assc_rec;
struct asstRecord	asst_rec;
struct sumrRecord	sumr_rec;

char 	systemDate [11];
long 	lsystemDate;

	/*
	 * Special fields and flags
	 */

	char	*data  = "data";

	int	envDbCo = 0,
		cr_find = 0;

	char	branchNumber [3];

MENUTAB upd_menu [] =
{
	{ " 1. UPDATE RECORD WITH CHANGES MADE.   ",
	  "" },
	{ " 2. IGNORE CHANGES JUST MADE TO RECORD.",
	  "" },
	{ ENDMENU }
};

/*
 * Local & Screen Structures. 
 */
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
	char	products1 [41];
	char	products2 [41];
	char	AssetGroup [6];
	char	AssetNumber [6];
} local_rec;

static	struct	var	vars [] =
{
	{ 1, LIN, "AssetGroup",	 2, 21, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset Group Code.  ", "Enter Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.AssetGroup },
	{ 1, LIN, "AssetNumber",	 2, 60, CHARTYPE,
		"UUUUU", "          ",
		"0", " ", "Asset number. 	 ", "Enter Asset number. <Default = new asset> [SEARCH] available. ",
		 NE, NO,  JUSTRIGHT, "0123456789", "", local_rec.AssetNumber },
	{ 1, LIN, "serial_no",	 3, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Serial No.  ", "Enter the Asset Serial Number",
		NE, NO,  JUSTLEFT, "", "", asmr_rec.serial_no },
	{ 1, LIN, "desc1",	 4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Description ", "Enter the Asset Description",
		NO, NO, JUSTLEFT, "", "", local_rec.desc1 },
	{ 1, LIN, "desc2",	 5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Desc cont.  ", "Asset Description cont'd...",
		NO, NO, JUSTLEFT, "", "", local_rec.desc2 },
	{ 1, LIN, "brand",	 6, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Brand Code  ", "Enter the Asset Brand Code [SEARCH]",
		YES, NO, JUSTLEFT, "", "", asmr_rec.brand },
	{ 1, LIN, "brand_desc",	 6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.brand_desc },
	{ 1, LIN, "spec1_code",	 8, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Spec One    ", "Enter the Asset Specification Code One [SEARCH]",
		YES, NO, JUSTLEFT, "", "", asmr_rec.spec1_code },
	{ 1, LIN, "spec1_desc",	 8, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.spec1_desc },
	{ 1, LIN, "spec2_code",	 9, 21, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Asset Model No.   ", "Enter the Asset Model No. [SEARCH]",
		NO, NO, JUSTLEFT, "", "", asmr_rec.spec2_code },
	{ 1, LIN, "spec2_desc",	 9, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.spec2_desc },
	{ 1, LIN, "crd_no",	11, 21, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Supplier Number     ", "Enter the Supplier Number [SEARCH]",
		YES, NO, JUSTLEFT, "", "", asmr_rec.crd_no },
	{ 1, LIN, "crd_name",	11, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.crd_name },
	{ 1, LIN, "type",	13, 21, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Asset Type  ", "Enter the Asset Type Code [SEARCH]",
		YES, NO, JUSTLEFT, "", "", asmr_rec.type },
	{ 1, LIN, "type_desc",	13, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.type_desc },
	{ 1, LIN, "pur_date",	14, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "Purchase Date       ", "Enter the Asset Purchase Date",
		NO, NO, JUSTRIGHT, "", "", (char *) &asmr_rec.pur_date },
	{ 1, LIN, "status_code",	15, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Status Code         ", "Enter the Asset Status Code [SEARCH]",
		YES, NO, JUSTLEFT, "", "", asmr_rec.status_code },
	{ 1, LIN, "status_desc",	15, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.status_desc },
	{ 1, LIN, "capacity",	16, 21, DOUBLETYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Capacity (Volume)   ", "Enter the Asset Capacity",
		NO, NO, JUSTRIGHT, "00000.00", "99999.99", (char *) &asmr_rec.capacity },
	{ 1, LIN, "products1",	18, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Assigned Products   ", "Enter the Narrative for Assigned Products",
		NO, NO, JUSTLEFT, "", "", local_rec.products1 },
	{ 1, LIN, "products2",	19, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Assigned Prod cont. ", "Assigned Products cont'd...",
		NO, NO, JUSTLEFT, "", "", local_rec.products2 },
	{ 0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy }
};

#include <FindSumr.h>

/*
 * Local Function Prototypes.
 */
int 	spec_valid 		 (int field);
void 	show_local 		 (void);
void 	SrchFatr 		 (char *);
void 	SrchFamr 		 (char *);
void 	SrchAsmr 		 (char *);
void 	SrchArbr  		 (char *);
void 	SrchAssb  		 (char *);
void 	SrchAsty 		 (char *);
void 	SrchAssc 		 (char *);
int 	heading 		 (int);

static BOOL IsSpaces 	 (char *str);
static void shutdown_prog (void);
static void OpenDB 		 (void);
static void CloseDB 	 (void);
static void Update 		 (void);

static BOOL	new_record = FALSE;


static BOOL
IsSpaces (
 char*              str)
{
	/*-----------------------------
	| Return TRUE if str contains |
	| only white space or nulls   |
	-----------------------------*/
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
}


/*==========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	envDbCo = atoi (get_env ("CR_CO"));
	cr_find = atoi (get_env ("CR_FIND"));

	init_scr ();
	set_tty ();

	OpenDB ();

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	set_masks ();
	init_vars (1);	

	lsystemDate = TodaysDate ();
	strcpy (systemDate, DateToDDMMYY (lsystemDate));

	/*===================================
	| Beginning of input control loop . |
	===================================*/
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

		new_record = FALSE;
		init_vars (1);


		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (!prog_exit && !restart) 
		{
			/*------------------------
			| Edit screen 1 linear . |
			------------------------*/
			heading (1);
			scn_display (1);
			edit (1);
			if (!prog_exit && !restart) 
				Update ();
		}
		abc_unlock (asmr);
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
	abc_alias ("sumr2", sumr);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	open_rec (assp, assp_list, ASSP_NO_FIELDS, "assp_id_no");
	open_rec (asbr, asbr_list, ASBR_NO_FIELDS, "asbr_id_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (asty, asty_list, ASTY_NO_FIELDS, "asty_id_no");
	open_rec (assc, assc_list, ASSC_NO_FIELDS, "assc_id_no");
	open_rec (asst, asst_list, ASST_NO_FIELDS, "asst_id_no");
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no"); 
	open_rec (fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no"); 
	open_rec (sumr, sumr_list,SUMR_NO_FIELDS, (!cr_find) ? "sumr_id_no" 
							    					  : "sumr_id_no3");
	open_rec ("sumr2",sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash"); 
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
	abc_fclose (asty);
	abc_fclose (assc);
	abc_fclose (asst);
	abc_fclose (famr);
	abc_fclose (fatr);
	abc_fclose (sumr);
	abc_fclose ("sumr2");

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

		strcpy (fatr_rec.co_no,comm_rec.co_no);
		strcpy (fatr_rec.group,local_rec.AssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML ("Asset group not found."));
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Fixed Asset number. |
	------------------------------*/
	if (LCHECK ("AssetNumber"))
	{
		int		i;
		char	cmd [50];



		if (SRCH_KEY)
		{
			SrchFamr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (famr_rec.co_no,comm_rec.co_no);
		strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
		strcpy (famr_rec.ass_no,	local_rec.AssetNumber);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc && strcmp (local_rec.AssetNumber, "     "))
		{
			i	=	prmptmsg (ML ("Asset number not found - create [Y/N]? "), "YyNn", 1, 23);
			if (i == 'Y' || i == 'y')			
			{
				sprintf (cmd, "fa_maint %s %s", 	local_rec.AssetGroup, 
												local_rec.AssetNumber);

				
				cc = sys_exec (cmd);

				heading (1);
				scn_display (1);

				if (!cc)
				{
					strcpy (famr_rec.co_no,comm_rec.co_no);
					strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
					strcpy (famr_rec.ass_no,	zero_pad (local_rec.AssetNumber, 5));
					cc = find_rec (famr, &famr_rec, COMPARISON, "r");
					if (!cc)
					{
						asmr_rec.pur_date	=	famr_rec.pur_date;
						DSP_FLD ("pur_date");

						print_mess (ML ("Asset number was successfully added."));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_SUCCESS);
					}
					else
					{
						print_mess (ML ("Asset number was not added.  Try again."));
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
				}
				else
				{
					print_mess (ML ("Asset number was not added.  Try again."));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
			else
			{
				clear_mess ();
				return (EXIT_FAILURE);
			}
			
		}

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Asset Serial No. |
	----------------------------*/
	if (LCHECK ("serial_no"))
	{
		if (SRCH_KEY)
		{
			SrchAsmr (temp_str);
			return 0;
		}

		if (!IsSpaces (asmr_rec.serial_no))
		{
			strcpy (asmr_rec.co_no, 	comm_rec.co_no);
			strcpy (asmr_rec.br_no, 	comm_rec.est_no);
			strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
			strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
			cc = find_rec (asmr, &asmr_rec, COMPARISON, "u");
			if (cc)
			{
				new_record = TRUE;
				entry_exit = FALSE;
			}
			else
			{
				new_record = FALSE;
				entry_exit = TRUE;
				show_local ();
			}
		}
		else
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Asset Description|
	----------------------------*/
	if (LCHECK ("desc1") || LCHECK ("desc2"))
	{
		sprintf (asmr_rec.desc, "%-40.40s%-40.40s",
				local_rec.desc1, local_rec.desc2);
		return (EXIT_SUCCESS);
	}

	/*---------------------
	| Validate Brand Code |
	---------------------*/
	if (LCHECK ("brand"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchArbr (temp_str);
			return 0;
		}
		memset (&asbr_rec, 0, sizeof asbr_rec);
		if (!IsSpaces (asmr_rec.brand))
		{
			strcpy (asbr_rec.co_no, comm_rec.co_no);
			strcpy (asbr_rec.brand_code, asmr_rec.brand);
			cc = find_rec (asbr, &asbr_rec, COMPARISON , "r");
			if (cc)
			{
				print_mess (ML ("Asset brand code not found"));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.brand_desc, asbr_rec.brand_desc);
		DSP_FLD ("brand_desc");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Validate Spec Code One |
	------------------------*/
	if (LCHECK ("spec1_code"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchAssb (temp_str);
			return 0;
		}
		memset (&assp_rec, 0, sizeof assp_rec);
		if (!IsSpaces (asmr_rec.spec1_code))
		{
			strcpy (assp_rec.co_no, comm_rec.co_no);
			strcpy (assp_rec.spec_code, asmr_rec.spec1_code);
			cc = find_rec (assp, &assp_rec, COMPARISON , "r");
				if (cc)
				{
					print_mess (ML ("Asset spec. code one not found."));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
		}
		strcpy (local_rec.spec1_desc, assp_rec.desc);
		DSP_FLD ("spec1_desc");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Asset Spec Code Two |
	--------------------------------*/
	if (LCHECK ("spec2_code"))
	{
		if (SRCH_KEY)
		{
			SrchAssb (temp_str);
			return 0;
		}
		memset (&assp_rec, 0, sizeof assp_rec);
		if (!IsSpaces (asmr_rec.spec2_code))
		{
			strcpy (assp_rec.co_no, comm_rec.co_no);
			strcpy (assp_rec.spec_code, asmr_rec.spec2_code);
			cc = find_rec (assp, &assp_rec, COMPARISON , "r");
				if (cc)
				{
					print_mess (ML ("Asset spec. code two not found."));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
		}
		strcpy (local_rec.spec2_desc, assp_rec.desc);
		DSP_FLD ("spec2_desc");
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Supplier Number   |
	----------------------------*/
	if (LCHECK ("crd_no"))
	{
		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			strcpy (local_rec.crd_name, sumr_rec.crd_name);
			return 0;
		}
		memset (&sumr_rec, 0, sizeof sumr_rec);
		if (!IsSpaces (asmr_rec.crd_no))
		{
			strcpy (sumr_rec.co_no, comm_rec.co_no);
			strcpy (sumr_rec.est_no, branchNumber);
			strcpy (sumr_rec.crd_no, zero_pad (asmr_rec.crd_no,6));
			cc = find_rec (sumr, &sumr_rec, COMPARISON , "r");
			if (cc)
			{
				print_mess (ML (mlStdMess022));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
				asmr_rec.hhsu_hash = sumr_rec.hhsu_hash;
		}
		strcpy (local_rec.crd_name, sumr_rec.crd_name);
		DSP_FLD ("crd_name");
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Asset Type      |
	----------------------------*/
	if (LCHECK ("type"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchAsty (temp_str);
			return 0;
		}
		memset (&asty_rec, 0, sizeof asty_rec);
		if (!IsSpaces (asmr_rec.type))
		{
			strcpy (asty_rec.co_no, comm_rec.co_no);
			strcpy (asty_rec.type_code, asmr_rec.type);
			cc = find_rec (asty, &asty_rec, COMPARISON , "r");
			if (cc)
			{
				print_mess (ML ("Asset type not found."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.type_desc, asty_rec.type_desc);
		DSP_FLD ("type_desc");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Asset Purchase Date|
	-------------------------------*/
	if (LCHECK ("pur_date"))
	{
		if (asmr_rec.pur_date > StringToDate (systemDate))
		{
			print_mess (ML ("Purchase date must be greater than today's date."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
			return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Status Code       | 
	----------------------------*/
	if (LCHECK ("status_code"))
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			SrchAssc (temp_str);
			return 0;
		}
		memset (&assc_rec, 0, sizeof assc_rec);
		if (!IsSpaces (asmr_rec.status_code))
		{
			strcpy (assc_rec.co_no, comm_rec.co_no);
			strcpy (assc_rec.stat_code, asmr_rec.status_code);
			cc = find_rec (assc, &assc_rec, EQUAL, "r");
			if (cc)
			{
				print_mess (ML ("Asset status code not found."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.status_desc, assc_rec.stat_desc);
		DSP_FLD ("status_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Asset Capacity   |
	-----------------------------*/
	if (LCHECK ("capacity"))
	{
		return (EXIT_SUCCESS);
	}

	/*----------------------------
	| Validate Assigned Products |
	----------------------------*/
	if (LCHECK ("products1") || LCHECK ("products2"))
	{
		sprintf (asmr_rec.products, "%-40.40s%-40.40s",
				local_rec.products1, local_rec.products2);
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}

/*=======================================
| Put into local_rec for display        |
=======================================*/
void
show_local (void)
{
	sprintf (local_rec.desc1, "%-40.40s", asmr_rec.desc);
	sprintf (local_rec.desc2, "%-40.40s", asmr_rec.desc+40);

	memset (&asbr_rec, 0, sizeof asbr_rec);
	if (!IsSpaces (asmr_rec.brand))
	{
		strcpy (asbr_rec.co_no, comm_rec.co_no);
		strcpy (asbr_rec.brand_code, asmr_rec.brand);
		cc = find_rec (asbr, &asbr_rec, COMPARISON , "r");
	}
	strcpy (local_rec.brand_desc, asbr_rec.brand_desc);
	
	memset (&assp_rec, 0, sizeof assp_rec);
	if (!IsSpaces (asmr_rec.spec1_code))
	{
		strcpy (assp_rec.co_no, comm_rec.co_no);
		strcpy (assp_rec.spec_code, asmr_rec.spec1_code);
		find_rec (assp, &assp_rec, COMPARISON , "r");
	}
	strcpy (local_rec.spec1_desc, assp_rec.desc);
	
	memset (&assp_rec, 0, sizeof assp_rec);
	if (!IsSpaces (asmr_rec.spec2_code))
	{
		strcpy (assp_rec.co_no, comm_rec.co_no);
		strcpy (assp_rec.spec_code, asmr_rec.spec2_code);
		cc = find_rec (assp, &assp_rec, COMPARISON , "r");
	}
	strcpy (local_rec.spec2_desc, assp_rec.desc);

	sumr_rec.hhsu_hash = asmr_rec.hhsu_hash;
	cc = find_rec ("sumr2", &sumr_rec, COMPARISON , "r");
	if (!cc)
		strcpy (local_rec.crd_name, sumr_rec.crd_name);
	else
		sprintf (local_rec.crd_name, "%-40.40s", " ");

	memset (&asty_rec, 0, sizeof asty_rec);
	if (!IsSpaces (asmr_rec.type))
	{
		strcpy (asty_rec.co_no, comm_rec.co_no);
		strcpy (asty_rec.type_code, asmr_rec.type);
		cc = find_rec (asty, &asty_rec, COMPARISON , "r");
	}
	strcpy (local_rec.type_desc, asty_rec.type_desc);

	memset (&assc_rec, 0, sizeof assc_rec);
	if (!IsSpaces (asmr_rec.status_code))
	{
		strcpy (assc_rec.co_no, comm_rec.co_no);
		strcpy (assc_rec.stat_code, asmr_rec.status_code);
		cc = find_rec (assc, &assc_rec, EQUAL, "r");
	}

	if (asmr_rec.hhsu_hash <= 0)
		sumr_rec.hhsu_hash = asmr_rec.hhsu_hash;
	else 
		cc = 1;
	cc = find_rec (sumr, &sumr_rec, COMPARISON , "r");
	if (!cc &&
		!strcmp (sumr_rec.co_no, comm_rec.co_no) &&
		!strcmp (sumr_rec.est_no, branchNumber))
			strcpy (local_rec.crd_name, sumr_rec.crd_name);
	else
			sprintf (local_rec.crd_name, "%-40.40s", " ");

	strcpy (local_rec.status_desc, assc_rec.stat_desc);

	sprintf (local_rec.products1, "%-40.40s", asmr_rec.products);
	sprintf (local_rec.products2, "%-40.40s", asmr_rec.products+40);
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
	strcpy (fatr_rec.co_no, 	comm_rec.co_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.co_no))
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
	strcpy (fatr_rec.co_no, comm_rec.co_no);
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
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", key_val);
	cc = find_rec (famr, &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (famr_rec.ass_group, local_rec.AssetGroup) &&
				  !strncmp (famr_rec.ass_no, key_val, strlen (key_val)))
	{
		cc = save_rec (famr_rec.ass_no, famr_rec.ass_desc1);
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
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", temp_str);
	cc = find_rec (famr, &famr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, famr, "DBFIND");
}

/*=======================================
| Search Serial No                      |
=======================================*/
void
SrchAsmr (
 char*              key_val)
{
	char desc [41];

	work_open ();
	save_rec ("#Serial no", "#Description");

	strcpy (asmr_rec.co_no, comm_rec.co_no);
	strcpy (asmr_rec.br_no, comm_rec.est_no);
	strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
	strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
	strcpy (asmr_rec.serial_no, key_val);
	cc = find_rec (asmr, &asmr_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asmr_rec.co_no, comm_rec.co_no) &&
			!strcmp (asmr_rec.br_no, comm_rec.est_no) &&
			!strcmp (asmr_rec.ass_group, local_rec.AssetGroup) &&
			!strcmp (asmr_rec.ass_no, local_rec.AssetNumber) &&
			!strncmp (asmr_rec.serial_no, key_val, strlen (key_val)))
	{
		sprintf (desc, "%-40.40s", asmr_rec.desc);
		cc = save_rec (asmr_rec.serial_no, desc);
		if (cc)
			break;

		cc = find_rec (asmr, &asmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (asmr_rec.co_no, comm_rec.co_no);
	strcpy (asmr_rec.br_no, comm_rec.est_no);
	strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
	strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
	strcpy (asmr_rec.serial_no, temp_str);
	cc = find_rec (asmr, &asmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, asmr, "DBFIND");
}

/*==================
| Search for asbr. |
==================*/
void
SrchArbr (	
 char*              key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");

	strcpy (asbr_rec.co_no, comm_rec.co_no);
	strcpy (asbr_rec.brand_code, key_val);

	cc = find_rec (asbr, &asbr_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asbr_rec.co_no, comm_rec.co_no) &&
			!strncmp (asbr_rec.brand_code, key_val, strlen (key_val)))
	{
		cc = save_rec (asbr_rec.brand_code, asbr_rec.brand_desc);
		if (cc)
			break;

		cc = find_rec (asbr, &asbr_rec, NEXT, "r");

	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (asbr_rec.co_no, comm_rec.co_no);
	strcpy (asbr_rec.brand_code, temp_str);
	cc = find_rec (asbr, &asbr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, asbr, "DBFIND");
}


/*==================
| Search for assp. |
==================*/
void
SrchAssb (	
 char*              key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");

	strcpy (assp_rec.co_no, comm_rec.co_no);
	sprintf (assp_rec.spec_code,"%-8.8s", key_val);

	cc = find_rec (assp, &assp_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (assp_rec.co_no, comm_rec.co_no) &&
			!strncmp (assp_rec.spec_code, key_val, strlen (key_val)))
	{
		cc = save_rec (assp_rec.spec_code, assp_rec.desc);
		if (cc)
			break;

		cc = find_rec (assp, &assp_rec, NEXT, "r");

	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (assp_rec.co_no, comm_rec.co_no);
	strcpy (assp_rec.spec_code, temp_str);
	cc = find_rec (assp, &assp_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, assp, "DBFIND");
}

/*==================
| Search for asty. |
==================*/
void
SrchAsty (	
 char*              key_val)
{
	char temp_desc [41];
	char temp_code [5];

	work_open ();
	save_rec ("#Type", "#Description");

	strcpy (asty_rec.co_no, comm_rec.co_no);
	strcpy (asty_rec.type_code, key_val);

	cc = find_rec (asty, &asty_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asty_rec.co_no, comm_rec.co_no) &&
			!strncmp (asty_rec.type_code, key_val, strlen (key_val)))
	{
		sprintf (temp_code, "%-4.4s",asty_rec.type_code);
		sprintf (temp_desc, "%-40.40s",asty_rec.type_desc);
		cc = save_rec (temp_code, temp_desc);
		if (cc)
			break;

		cc = find_rec (asty, &asty_rec, NEXT, "r");

	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (asty_rec.co_no, comm_rec.co_no);
	strcpy (asty_rec.type_code, temp_str);
	cc = find_rec (asty, &asty_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, asty, "DBFIND");
}


/*==================
| Search for assc. |
==================*/
void
SrchAssc (	
 char*              key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");

	strcpy (assc_rec.co_no, comm_rec.co_no);
	strcpy (assc_rec.stat_code, key_val);

	cc = find_rec (assc, &assc_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (assc_rec.co_no, comm_rec.co_no) &&
			!strncmp (assc_rec.stat_code, key_val, strlen (key_val)))
	{
		cc = save_rec (assc_rec.stat_code, assc_rec.stat_desc);
		if (cc)
			break;

		cc = find_rec (assc, &assc_rec, NEXT, "r");

	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (assc_rec.co_no, comm_rec.co_no);
	strcpy (assc_rec.stat_code, temp_str);
	cc = find_rec (assc, &assc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, assc, "DBFIND");
}


/*==================
| Updated records. |
==================*/
void
Update (void)
{
	if (new_record)
	{
		cc = abc_add (asmr, &asmr_rec);
		if (cc) 
			file_err (cc, asmr, "DBADD");
	}
	else
	{
		BOOL exitLoop = FALSE;

		for (;;)
		{
	    	mmenu_print ("   U P D A T E   S E L E C T I O N   ", upd_menu, 0);
	    	switch (mmenu_select (upd_menu))
	    	{
			case DEFAULT :
			case SEL_UPDATE :
				cc = abc_update (asmr, &asmr_rec);
				if (cc) 
					file_err (cc, asmr, "DBUPDATE");
				exitLoop = TRUE;
				break;
	
			case SEL_IGNORE :
				abc_unlock (asmr);
				exitLoop = TRUE;
				break;

			default :
				break;
	    	}

			if (exitLoop)
				break;
		}
	}
	abc_unlock (asmr);
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

		clear ();

		centre_at (0, 80, ML ("%R Asset Master File Maintenance "));

		box (0, 1, 80, 18);

		move (1, 7); line (79);
		move (1, 10); line (79);
		move (1, 12); line (79);
		move (1, 17); line (79);

		move (0, 21);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		move (0, 22); line (80);
	
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
