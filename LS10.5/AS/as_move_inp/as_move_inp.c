/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: as_move_inp.c,v 5.3 2002/07/24 08:38:40 scott Exp $
|  Program Name  : (as_move_inp.c)
|  Program Desc  : (Asset Movement Maintenance & Details)
|---------------------------------------------------------------------|
| $Log: as_move_inp.c,v $
| Revision 5.3  2002/07/24 08:38:40  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.2  2002/07/18 06:07:09  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.1  2002/06/21 03:07:44  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: as_move_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_move_inp/as_move_inp.c,v 5.3 2002/07/24 08:38:40 scott Exp $";

#define 	MAXLINES		500
#define 	MAXSCNS			2
#define 	MAXWIDTH		300
#define 	TABLINES		8

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<minimenu.h>
#include    <DateToString.h>

typedef int BOOL;

#define	SEL_UPDATE			0
#define	SEL_IGNORE			1
#if 0
#define	SEL_DELETE			2
#endif
#define	SEL_DEFAULT			99


#define ADD_EDIT        0
#define DISPLAY_ONLY    1
#define CUSTOMER       'C' 
#define	SUPPLIER       'S' 

     /*
	  *  Added Asset   Movements Option   
      */

 	MENUTAB screen_opt []= 
		{
			{"Add / Edit Asset Movements",""},
			{"Display    Asset Movements",""},
			{ENDMENU}
		};
	 
#include	"schema"

struct cumrRecord	cumr_rec;
struct sumrRecord	sumr_rec;
struct asmtRecord	asmt_rec;
struct commRecord	comm_rec;
struct asmrRecord	asmr_rec;
struct famrRecord	famr_rec;
struct fatrRecord	fatr_rec;
struct asbrRecord	asbr_rec;
struct asstRecord	asst_rec;
struct asdtRecord	asdt_rec;
struct asmvRecord	asmv_rec;


char systemDate [11];
long lsystemDate;

	/*
	 * Special fields and flags
	 */

	char	*data  = "data";
	char	*sumr2 = "sumr2";
	char	*cumr2 = "cumr2";
	char	*asmv2 = "asmv2";

	struct	storeRec {
		long	fromHhcu;
		long	toHhcu;
		long	fromHhsu;
		long	toHhsu;
		long	lastModDate;
	} store [MAXLINES];


struct {
		char	last_cust [60];
		long	last_date;
	} location; 

/*
 * Local & Screen Structures. 
 */
struct {
		char	dummy [11];
		char	AssetGroup [6];
		char	AssetNumber [6];
		char	serial_no [26];
		char	desc1 [41];
		char	desc2 [41];
		char	brand_desc [41];
		long	hhas_hash;
		int		line_no;
		long	hham_hash;
		long	report_no;
		char	move_desc [41];
		char	move_code [5];
		char	move_tdesc [41];
		char	s_type [2];
		char	d_type [2];
		long	from_hhcu;
		long	to_hhcu;
		long	from_crdt;
		long	to_crdt;
		char	source [7];
		char	source_desc [41];
		char	dest [7];
		char    dest_desc [41];
		long	move_date;
		long	vol_com;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "AssetGroup",	 2, 21, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset Group Code.  ", "Enter Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.AssetGroup},
	{1, LIN, "group_desc",	 2, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", fatr_rec.group_desc},
	{1, LIN, "AssetNumber",	 3, 21, CHARTYPE,
		"UUUUU", "          ",
		"0", " ", "Asset number. 	 ", "Enter Asset number. <Default = new asset> [SEARCH] available. ",
		 NE, NO,  JUSTRIGHT, "0123456789", "", local_rec.AssetNumber},
	{1, LIN, "serial_no",	 4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Serial No.  ", "Enter the Asset Serial Number",
		NE, NO,  JUSTLEFT, "", "", local_rec.serial_no},
	{1, LIN, "desc1",	 5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Description ", "",
		NA, NO, JUSTLEFT, "", "", local_rec.desc1},
	{1, LIN, "desc2",	 5, 61, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "brand",	 6, 21, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "Asset Brand Code  ", "",
		NA, NO, JUSTLEFT, "", "", asmr_rec.brand},
	{1, LIN, "brand_desc",	 6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.brand_desc},
	{2, TAB, "report_no",MAXLINES, 3, LONGTYPE,
		"NNNNNN", "          ",
		" ", " ", "Report No.", "Enter the Report Number",
		YES, NO, JUSTRIGHT, "000000", "999999", (char *) &local_rec.report_no},
	{2, TAB, "move_desc",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "            - R E M A R K S -             ", "Enter a description of why the asset was moved",
		YES, NO, JUSTLEFT, "", "", local_rec.move_desc},
	{2, TAB, "move_code",  0, 1, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Mvmnt code", "Enter Movement Code [SEARCH]",
		YES, NO, JUSTLEFT, "", "", local_rec.move_code},
	{2, TAB, "move_tdesc",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.move_tdesc},
	{2, TAB, "s_type",  0, 4, CHARTYPE,
		"U", "          ",
		" ", " ", "Source Type", "Enter type of source : (C)ustomer, (S)upplier",
		YES, NO, JUSTLEFT, "COS", "", local_rec.s_type},
	{2, TAB, "source",  0, 0, CHARTYPE,
		"AAAAAA", "          ",
		" ", " ", " Source ", "Enter Customer Number [SEARCH]",
		YES, NO, JUSTLEFT, "", "", local_rec.source},
	{2, TAB, "source_desc",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.source_desc},
	{2, TAB, "d_type",  0, 3, CHARTYPE,
		"U", "     ",
		" ", " ", "Dest  Type", "Enter type of dest. : (C)ustomer, (S)upplier",
		YES, NO, JUSTLEFT, "CSO", "", local_rec.d_type},
  	{2, TAB, "dest",  0, 0, CHARTYPE,
		"AAAAAA", "          ",
		" ", " ", "  Dest  ", "Enter Destination. [SEARCH]",
		YES, NO, JUSTLEFT, "", "", local_rec.dest},
	{2, TAB, "dest_desc",  0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.dest_desc},
	{2, TAB, "move_date",  0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "Move Date ", "Enter Movement Date",
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.move_date},
	{2, TAB, "vol_commit",  0, 0, LONGTYPE,
		"NNNNNNNN", "          ",
		" ", "0", "Vol Commit.", "Enter Volume committed.",
		YES, NO, JUSTLEFT, "", "", (char *) &local_rec.vol_com},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}
};

#include <FindCumr.h>
#include <FindSumr.h>
/*
 * Local Function Prototypes
 */
void 	ChooseTrans		(void);
int 	spec_valid 		(int);
void 	tab_other 		(int);
void 	ShowLocal 		(void);
void 	SrchFatr 		(char *);
void 	SrchFamr 		(char *);
void 	SrchAsmr 		(char *);
void 	SrchAstm 		(char *);
void 	LoadAsmv 		(void);
int 	heading 		(int);

static BOOL IsSpaces 		(char *str);
static void shutdown_prog 	(void);
static void OpenDB 			(void);
static void CloseDB 		(void);
static void Update 			(void);

static BOOL	rec_found = FALSE;

static BOOL
IsSpaces (
	char	*str)
{
	/*
	 * Return TRUE if str contains only white space or nulls
	 */
 	while (*str)
 	{
		if (!isspace (*str))
			return FALSE;
		str++;
	}
	return TRUE;
}


int		add_edit = 0,
		line_number = 0;
long	prev_date = 0L;

int		envDbCo     = 0,
		envCrCo     = 0,
		envCrFind		 	= 0,
		envDbFind		 	= 0;

char	db_branchNumber [3],
		cr_branchNumber [3];

char	curr_stype [2],
		curr_dtype [2];	

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char	*argv [])
{
	envDbCo = atoi (get_env ("DB_CO"));
	envCrCo = atoi (get_env ("CR_CO"));

	envDbFind  = atoi (get_env ("DB_FIND"));
	envCrFind  = atoi (get_env ("CR_FIND"));

	SETUP_SCR (vars);

	init_scr ();				/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks		*/

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	swide ();


	OpenDB ();

	strcpy (db_branchNumber, (envDbCo) ? comm_rec.est_no : " 0");
	strcpy (cr_branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	lsystemDate = TodaysDate ();
	strcpy (systemDate, DateToDDMMYY (lsystemDate));

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

		init_vars (1);
		init_vars (2);
		lcount [2]	=	0;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);

		entry (1);

		if (!prog_exit && !restart)
		{	
			/*------------------------------
			|   Choose Display/Add_Edit    |
			------------------------------*/
			ChooseTrans ();	

			/*-------------------------------
			| Enter screen 2 Tabular input. |
			-------------------------------*/
			LoadAsmv ();

			strcpy (curr_dtype, local_rec.d_type);
			strcpy (curr_stype, local_rec.s_type);

			if (local_rec.d_type [0] == 'C')
					FLD ("vol_commit") = YES;
			else    
					FLD ("vol_commit") = NA;

			if (lcount == 0)
				entry (2);
			else
				edit (2);

				if (!prog_exit && !restart)
				{
					edit_all ();

					/*-----------------
					| Update records. |
					-----------------*/
					if (!prog_exit && !restart)
						Update ();
				}
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
ChooseTrans (void)
{
	mmenu_print ("   Type of Transaction", screen_opt, 0);
	switch (mmenu_select (screen_opt))
	{
		case ADD_EDIT	: 
						add_edit = 1;
						FLD ("report_no") = YES;
						FLD ("move_desc") = YES;
						FLD ("move_code") = YES;
						FLD ("s_type")    = YES;
						FLD ("d_type")    = YES;
						FLD ("source")    = YES;
						FLD ("dest")      = YES;
						FLD ("move_desc") = YES;
						FLD ("move_date") = YES;
						FLD ("vol_commit") = YES;
						break;
		case DISPLAY_ONLY :
						add_edit = 0;
						FLD ("report_no") = NA;
						FLD ("move_desc") = NA;
						FLD ("move_code") = NA;
						FLD ("s_type")    = NA;
						FLD ("d_type")    = NA;
						FLD ("source")    = NA;
						FLD ("dest")      = NA;
						FLD ("move_desc") = NA;
						FLD ("move_date") = NA;
						FLD ("vol_commit") = NA;
						break;	
	}


}

/*
 * Program exit sequence. 
 */
static void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files . 
 */
static void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

	abc_alias (cumr2, cumr);
	abc_alias (sumr2, sumr);
	abc_alias (asmv2, asmv);


	open_rec (asbr, asbr_list, ASBR_NO_FIELDS, "asbr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
								 (envDbCo) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, 
								 (envCrCo) ? "sumr_id_no" : "sumr_id_no3");
	open_rec (sumr2,sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no"); 
	open_rec (fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no"); 
	open_rec (asst, asst_list, ASST_NO_FIELDS, "asst_id_no");
	open_rec (asmt, asmt_list, ASMT_NO_FIELDS, "asmt_id_no");
	open_rec (asdt, asdt_list, ASDT_NO_FIELDS, "asdt_id_no");
	open_rec (asmv, asmv_list, ASMV_NO_FIELDS, "asmv_id_no");
	open_rec (asmv2,asmv_list, ASMV_NO_FIELDS, "asmv_hham_hash");
}

/*
 * Close data base files . 
 */
static void
CloseDB (void)
{
	abc_fclose (asbr);
	abc_fclose (sumr);
	abc_fclose (sumr2);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (asmt);
	abc_fclose (asmr);
	abc_fclose (famr);
	abc_fclose (fatr);
	abc_fclose (asst);
	abc_fclose (asdt);
	abc_fclose (asmv);
	abc_fclose (asmv2);

	abc_dbclose (data);
}

int
spec_valid (
	int		field)
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

		strcpy (famr_rec.co_no,comm_rec.co_no);
		strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
		strcpy (famr_rec.ass_no,	local_rec.AssetNumber);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML ("Asset number not found."));
			return (EXIT_FAILURE);
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

		memset (&asmr_rec, 0, sizeof asmr_rec);
		if (!IsSpaces (local_rec.serial_no))
		{
			strcpy (asmr_rec.co_no, 	comm_rec.co_no);
			strcpy (asmr_rec.br_no, 	comm_rec.est_no);
			strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
			strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
			strcpy (asmr_rec.serial_no, local_rec.serial_no);
			cc = find_rec (asmr, &asmr_rec, COMPARISON, "r");
			if (cc)
			{
				print_mess (ML ("A valid asset serial number must be entered."));
				sleep (sleepTime);
				clear_mess ();
				rec_found = FALSE;
				return (EXIT_FAILURE);
			}	
			else
			{
				ShowLocal ();
				rec_found = TRUE;
			}
		}
		else
			return (EXIT_FAILURE);

		DSP_FLD ("serial_no");
		DSP_FLD ("desc1");
		DSP_FLD ("desc2");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("last_cust"))
	{
		return (EXIT_SUCCESS);
	}


	if (LCHECK ("last_date"))
	{
		return (EXIT_SUCCESS);
	}

	/*---------------------------+
	| Validate Report No.        |
	----------------------------*/
	if (LCHECK ("report_no"))
	{
		if (local_rec.report_no < 1)
		{
			print_mess (ML ("A report no. greater than 000000 must be entered."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Movement Description |
	-------------------------------*/
	if (LCHECK ("move_desc"))
	{
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Move To Customer     |
	-------------------------------*/
	if (LCHECK ("move_code"))
	{
		if (SRCH_KEY)
		{
			SrchAstm (temp_str);
			return 0;
		}

		memset (&asmt_rec, 0, sizeof asmt_rec);
		if (!IsSpaces (local_rec.move_code))
		{
			strcpy (asmt_rec.co_no, comm_rec.co_no);
			strcpy (asmt_rec.type_code, local_rec.move_code);
			cc = find_rec (asmt, &asmt_rec, COMPARISON , "r");
			if (cc)
			{
				print_mess (ML ("Movement not found."));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		strcpy (local_rec.move_tdesc, asmt_rec.desc);
		DSP_FLD ("move_tdesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("s_type"))
	{
		if (strcmp (curr_stype, local_rec.s_type))
		{
			store [line_cnt].fromHhcu = 0L;
			store [line_cnt].fromHhsu = 0L;
			sprintf (local_rec.source, "%-6.6s", " ");
			DSP_FLD ("source");
		}
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Move From Customer   |
	-------------------------------*/
	if (LCHECK ("source"))
	{
		if (local_rec.s_type [0] == CUSTOMER)
		{
			if (SRCH_KEY)
			{
				CumrSearch (comm_rec.co_no, db_branchNumber, temp_str);
				sprintf (local_rec.source, "%-6.6s", cumr_rec.dbt_no);
				return 0;
			}

			memset (&cumr_rec, 0, sizeof cumr_rec);
			if (!IsSpaces (local_rec.source))
			{
				strcpy (cumr_rec.co_no, comm_rec.co_no);
				strcpy (cumr_rec.est_no, db_branchNumber);
				strcpy (cumr_rec.dbt_no, local_rec.source);
				cc = find_rec (cumr, &cumr_rec, COMPARISON , "r");
				if (cc)
				{
					print_mess (ML (mlStdMess021));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				else
					store [line_cnt].fromHhcu = cumr_rec.hhcu_hash;
			}
			else
					store [line_cnt].fromHhcu = 0L;

			strcpy (local_rec.source_desc, cumr_rec.dbt_name);
			store [line_cnt].fromHhsu = 0L;
		}
		else if (local_rec.s_type [0] == SUPPLIER)
		{
			/*-------------------
			| Validate Creditor |
			-------------------*/

			if (SRCH_KEY)
			{
				SumrSearch (comm_rec.co_no, cr_branchNumber, temp_str);
				sprintf (local_rec.source, "%-6.6s", sumr_rec.crd_no);
				return (EXIT_SUCCESS);
			}

			if (!IsSpaces (local_rec.source))
			{
				memset (&sumr_rec,0,sizeof sumr_rec);
				strcpy (sumr_rec.co_no,comm_rec.co_no);
				strcpy (sumr_rec.est_no, cr_branchNumber);
				strcpy (sumr_rec.crd_no,local_rec.source);
				cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
				if (cc)
				{
					print_mess (ML (mlStdMess022));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				else
					store [line_cnt].fromHhsu = sumr_rec.hhsu_hash;
			}
			else
					store [line_cnt].fromHhsu = 0L;

			strcpy (local_rec.source_desc,sumr_rec.crd_name);
			store [line_cnt].fromHhcu = 0L;
		}

		DSP_FLD ("source");
		return (EXIT_SUCCESS);

	}

	if (LCHECK ("d_type"))
	{
		if (strcmp (curr_dtype, local_rec.d_type))
		{
			store [line_cnt].toHhcu = 0L;
			store [line_cnt].toHhsu = 0L;
			sprintf (local_rec.dest,  "%-6.6s", " ");
			if (curr_dtype [0] == 'C')
			{
				local_rec.vol_com = 0L;
				DSP_FLD ("vol_commit");
			}

			if (line_cnt+1 < lcount [2])
			{
				store [line_cnt + 1].fromHhcu = 0L;
				store [line_cnt + 1].fromHhsu = 0L;
			}
			DSP_FLD ("dest");
		}
	
		if (local_rec.d_type [0] == 'C')
			FLD ("vol_commit") = YES;
		else
			FLD ("vol_commit") = NA;
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Move To Customer     |
	-------------------------------*/
	if (LCHECK ("dest"))
	{
		if (local_rec.d_type [0] == CUSTOMER)
		{
			if (SRCH_KEY)
			{
				CumrSearch (comm_rec.co_no, db_branchNumber, temp_str);
				sprintf (local_rec.dest, "%-6.6s", cumr_rec.dbt_no);
				return 0;
			}

			sprintf (local_rec.dest, "%-6.6s", local_rec.dest);
			memset (&cumr_rec, 0, sizeof cumr_rec);
			if (!IsSpaces (local_rec.dest))
			{
				strcpy (cumr_rec.co_no, comm_rec.co_no);
				strcpy (cumr_rec.est_no, db_branchNumber);
				strcpy (cumr_rec.dbt_no, local_rec.dest);
				cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
				if (cc)
				{
					print_mess (ML (mlStdMess021));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				store [line_cnt].toHhcu = cumr_rec.hhcu_hash;
				if (line_cnt + 1 < lcount [2])
					store [line_cnt + 1].fromHhcu = cumr_rec.hhcu_hash;
				
			}
			else
			{
				store [line_cnt].toHhcu = 0L;		
				if (line_cnt + 1 < lcount [2])
					store [line_cnt + 1].fromHhcu = 0L ;
			}

			store [line_cnt].toHhsu  = 0L; 
			store [line_cnt + 1].fromHhsu = 0L; 
			strcpy (local_rec.dest_desc, cumr_rec.dbt_name);
		}
		else if (local_rec.d_type [0] == SUPPLIER)
		{
			if (SRCH_KEY)
			{
				SumrSearch (comm_rec.co_no, cr_branchNumber, temp_str);
				sprintf (local_rec.dest, "%-6.6s", sumr_rec.crd_no);
				return (EXIT_SUCCESS);
			}

			sprintf (local_rec.dest, "%-6.6s", local_rec.dest);
			if (!IsSpaces (local_rec.dest))
			{
				memset (&sumr_rec,0,sizeof sumr_rec);
				strcpy (sumr_rec.co_no, comm_rec.co_no);
				strcpy (sumr_rec.est_no, cr_branchNumber);
				strcpy (sumr_rec.crd_no,local_rec.dest);
				cc = find_rec (sumr,&sumr_rec,EQUAL,"r");
				if (cc)
				{
					print_mess (ML (mlStdMess022));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}

				store [line_cnt].toHhsu = sumr_rec.hhsu_hash;
				if (line_cnt + 1 < lcount [2])
					store [line_cnt + 1].fromHhsu = store [line_cnt].toHhsu;
			}
			else
			{
				store [line_cnt].fromHhsu = 0L;
				if (line_cnt + 1 < lcount [2])
					store [line_cnt].fromHhsu = 0L;
			}

			store [line_cnt].toHhcu			= 0L;
			store [line_cnt + 1].fromHhcu 	= 0L;

			strcpy (local_rec.dest_desc,sumr_rec.crd_name);
		}

		DSP_FLD ("dest");
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Asset Movement Date |
	--------------------------------*/
	if (LCHECK ("move_date"))
	{
		if (prev_date > local_rec.move_date)
		{
			print_mess (ML ("Movement date must be greater or equal to previous date."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
			store [line_cnt].lastModDate = local_rec.move_date;
		return (EXIT_SUCCESS);
	}
	
	if (LCHECK ("vol_commit"))
	{
		return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}

void
tab_other (
 int                line_no)
{
	char str_from     [60], 
		 str_to       [60],
		 str_location [120],
		 str_move_desc [120];
	int	plus;	


	if (add_edit &&
	  (line_no == 0 ||
	  (line_no == 1 && last_char == UP_KEY)))	
	{
		FLD ("s_type") = YES;
		FLD ("source") = YES;
	}
	else 						          
	{
		FLD ("s_type") = NA;
		FLD ("source") = NA;
	}
		
   if (add_edit)  
	{

			if (line_no < lcount [2])
			{
				line_cnt =line_no;

				if (line_no)
					prev_date = store [line_no - 1].lastModDate;
				else
					prev_date = store [line_no].lastModDate;


 				if ((line_no &&
				 	  store [line_no - 1].toHhsu <= 0L &&
				 	  store [line_no - 1].toHhcu  >  0L) ||
 					 (!line_no &&
				 	  store [0].fromHhsu <= 0L && store [0].fromHhcu >  0L))
				{
						/*-----------------------------------+
						|	update screen display from_hhcu	 |
						+-----------------------------------*/
					memset (&cumr_rec, 0, sizeof cumr_rec);
					strcpy (cumr_rec.co_no, comm_rec.co_no);
					strcpy (cumr_rec.est_no, db_branchNumber);

					if (!line_no)
						cumr_rec.hhcu_hash = store [line_no].fromHhcu;
					else
						cumr_rec.hhcu_hash = store [line_no - 1].toHhcu;
					cc = find_hash (cumr2, &cumr_rec, COMPARISON , "r",
					cumr_rec.hhcu_hash);
					if (!cc)
					{
						strcpy (local_rec.s_type, "C");
						strcpy (local_rec.source, cumr_rec.dbt_no); 
						strcpy (local_rec.source_desc, cumr_rec.dbt_name);
						DSP_FLD ("s_type");
						DSP_FLD ("source");
					}
				}
 				else if ((line_no &&
					 	  store [line_no - 1].toHhcu <= 0L &&
						  store [line_no - 1].toHhsu >  0L) ||
 						 (!line_no &&
					 	  store [0].fromHhcu <= 0L &&
						  store [0].fromHhsu >  0L))
				{
						/*-----------------------------------+
						|	update screen display from_hhcu	 |
						+-----------------------------------*/
				    if (!line_no)
					{
						cc = find_hash (sumr2, &sumr_rec, COMPARISON , "r",
								store [line_no].fromHhsu);
					}
					else
					{
						cc = find_hash (sumr2, &sumr_rec, COMPARISON , "r",
								store [line_no - 1].toHhsu);
					}

					if (!cc)
					{
				
						strcpy (local_rec.s_type, "S");
						strcpy (local_rec.source, sumr_rec.crd_no);
						strcpy (local_rec.source_desc, sumr_rec.crd_name);
						if (line_no)
						{
							DSP_FLD ("s_type");
							DSP_FLD ("source");
						}
					}
				}
				else
				{
					strcpy (local_rec.s_type, " ");
					sprintf (local_rec.source, "%-6.6s","  ");
					sprintf (local_rec.source_desc, "%-40.40s"," ");
					DSP_FLD ("s_type");
					DSP_FLD ("source");
				}

			}
			putval (line_no);
	}

	if (local_rec.d_type [0] == 'C')
			FLD ("vol_commit") = YES;
	else    
			FLD ("vol_commit") = NA;

	if (!strlen (clip (local_rec.move_code)))
		sprintf (str_move_desc, "%-100.100s", " ");
	else
		sprintf (str_move_desc, "%-100.100s", local_rec.move_tdesc);
		
	if (local_rec.s_type [0] == CUSTOMER)
		sprintf (str_from,ML ("From Cust.  %-40.40s "),local_rec.source_desc);
	else if (local_rec.s_type [0] == SUPPLIER)
		sprintf (str_from, ML ("From Supplier     %-40.40s "), local_rec.source_desc);
	else
		sprintf (str_from, "                  %-40.40s ", " ");

	if (local_rec.d_type [0] == CUSTOMER)
		sprintf (str_to,ML ("To   Cust.  %-40.40s "),local_rec.dest_desc);
	else if (local_rec.d_type [0] == SUPPLIER)
		sprintf (str_to, ML ("To   Supplier     %-40.40s "), local_rec.dest_desc);
	else
		sprintf (str_to, "                  %-40.40s ", " ");

	sprintf (str_location,  "%-50.50s%-50.50s", str_from, str_to);

	if (line_no >= lcount [2])
	{
		sprintf (str_location,  ML ("From%-45.45s To %-47.47s"), " ", " ");
		sprintf (str_move_desc, "%-100.100s", " ");
	}

	print_at (8,2, ML ("Movement "));
	rv_pr (str_location,  23, 8, 1);
	print_at (9,2, ML ("Movement Description "));
	rv_pr (str_move_desc, 23, 9, 1);

	if (last_char == UP_KEY || last_char == DOWN_KEY)
	{
		if (last_char == UP_KEY)
			plus = -1;
		else
			plus = 1;

		if (store [line_no + plus].fromHhcu == 0L) 
			strcpy (curr_stype, "S");
		else if (store [line_no + plus].fromHhcu == 0L &&
			store [line_no + plus].fromHhsu == 0L)
			strcpy (curr_stype, "O");
		else if (store [line_no + plus].toHhsu == 0L)
			strcpy (curr_stype, "C");

		if (store [line_no + plus].toHhcu == 0L)
			strcpy (curr_dtype, "S");
		else if (store [line_no + plus].toHhcu == 0L &&
			store [line_no + plus].toHhsu == 0L)
			strcpy (curr_dtype, "O");
		else if (store [line_no + plus].toHhsu == 0L)
			strcpy (curr_dtype, "C");
	}
	else
	{
		strcpy (curr_dtype, local_rec.d_type);
		strcpy (curr_stype, local_rec.s_type);
	}
}


void
ShowLocal (void)
{
	sprintf (local_rec.desc1,"%-40.40s", asmr_rec.desc);
	sprintf (local_rec.desc2,"%-40.40s",asmr_rec.desc+40);

	if (!IsSpaces (asmr_rec.brand))
	{
		strcpy (asbr_rec.co_no, comm_rec.co_no);
		strcpy (asbr_rec.brand_code, asmr_rec.brand);
		cc = find_rec (asbr, &asbr_rec, COMPARISON , "r");
	}
	strcpy (local_rec.brand_desc,asbr_rec.brand_desc);

	sprintf (location.last_cust, " ");
	memset (&asmv_rec, 0, sizeof asmv_rec);
	cc = find_hash (asmv2, &asmv_rec, COMPARISON , "r", asmr_rec.hham_hash);
	if (!cc)
	{		
		memset (&cumr_rec, 0, sizeof cumr_rec);
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, db_branchNumber);
		cumr_rec.hhcu_hash = asmv_rec.to_hhcu;
		cc = find_hash (cumr2, &cumr_rec, COMPARISON , "r",
	 	  	 cumr_rec.hhcu_hash);
		if (!cc)
			strcpy (location.last_cust, cumr_rec.dbt_name);

		location.last_date = asmv_rec.move_date;
	}
}

/*
 * Search for Fixed Asset Group master file. 
 */
void
SrchFatr (
	char	*key_val)
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
/*
 * Search for Fixed Asset Number master file. 
 */
void
SrchFamr (
	char	*key_val)
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

/*
 * Search Serial No                        
 */
void
SrchAsmr (
	char	*key_val)
{
	char desc [41];	

	work_open ();
	save_rec ("#Serial_no","#Description");

	strcpy (asmr_rec.co_no, 	comm_rec.co_no);
	strcpy (asmr_rec.br_no, 	comm_rec.est_no);
	strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
	strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
	strcpy (asmr_rec.serial_no, key_val);
	cc = find_rec (asmr, &asmr_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asmr_rec.co_no, 	comm_rec.co_no) &&
			!strcmp (asmr_rec.br_no, 	comm_rec.est_no) &&
			!strcmp (asmr_rec.ass_group,local_rec.AssetGroup) &&
			!strcmp (asmr_rec.ass_no,	local_rec.AssetNumber) &&
			!strncmp (asmr_rec.serial_no,key_val,strlen (key_val)))
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

}

/*
 * Search Movement Type 
 */
void
SrchAstm (
	char	*key_val)
{
	char desc [41];	

	work_open ();
	save_rec ("#Code","#Description");
	strcpy (asmt_rec.co_no, comm_rec.co_no);
	strcpy (asmt_rec.type_code, key_val);
	cc = find_rec (asmt, &asmt_rec, GTEQ, "r");

	while (!cc &&
			!strcmp (asmt_rec.co_no, comm_rec.co_no) &&
			!strncmp (asmt_rec.type_code,key_val,strlen (key_val)))
	{
		sprintf (desc, "%-40.40s", asmt_rec.desc);
		cc = save_rec (asmt_rec.type_code, desc);
		if (cc)
			break;

		cc = find_rec (asmt, &asmt_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (asmt_rec.co_no, comm_rec.co_no);
	strcpy (asmt_rec.type_code, temp_str);
	cc = find_rec (asmt, &asmt_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, asmt, "DBFIND");
}


/*
 * Load asmv into local for screen 2 
 */
void
LoadAsmv (void)
{
	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (2);
	init_vars (2);
	lcount [2] = 0;
	
	memset (&asmv_rec, 0, sizeof asmv_rec);

	asmv_rec.hhar_hash = asmr_rec.hhas_hash;
	asmv_rec.line_no = 0;
	cc = find_rec (asmv, &asmv_rec, GTEQ, "r");
	while (!cc && asmv_rec.hhar_hash == asmr_rec.hhas_hash)
	{
		sprintf (local_rec.source,        "%-6.6s", " ");
		sprintf (local_rec.source_desc,   "%-40.40s", " ");
		sprintf (local_rec.dest,          "%-6.6s", " ");
		sprintf (local_rec.dest_desc,     "%-40.40s", " ");

		strcpy (local_rec.move_code, asmv_rec.move_code);
		strcpy (local_rec.move_desc, asmv_rec.move_desc) ;

		local_rec.report_no 			= asmv_rec.report_no ;
	    local_rec.move_date 			= asmv_rec.move_date ;
	    local_rec.vol_com   			= asmv_rec.vol_commit;
		store [lcount [2]].fromHhcu 	= asmv_rec.from_hhcu ;
		store [lcount [2]].toHhcu 		= asmv_rec.to_hhcu ;
		store [lcount [2]].fromHhsu 	= asmv_rec.from_crdt ;
		store [lcount [2]].toHhsu 		= asmv_rec.to_crdt ;
		store [lcount [2]].lastModDate 	= asmv_rec.move_date ;

		strcpy (asmt_rec.co_no, comm_rec.co_no);
		strcpy (asmt_rec.type_code, asmv_rec.move_code);
		cc = find_rec (asmt, &asmt_rec, EQUAL, "r");
		if (!cc)
			strcpy (local_rec.move_tdesc, asmt_rec.desc);

		if (asmv_rec.source_type [0] == CUSTOMER)
		{
			cumr_rec.hhcu_hash = asmv_rec.from_hhcu;
			cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
			if (!cc)
			{
				strcpy (local_rec.source, 		cumr_rec.dbt_no);
				strcpy (local_rec.source_desc, 	cumr_rec.dbt_name);
			}
			else
			{
				store [lcount [2]].fromHhcu = 0L;
				strcpy (asmv_rec.source_type, " ");
			}
		}
		else if (asmv_rec.source_type [0] == SUPPLIER)
		{
			sumr_rec.hhsu_hash = asmv_rec.from_crdt;
			cc = find_rec (sumr2, &sumr_rec, COMPARISON , "r");
			if (!cc)
			{
				strcpy (local_rec.source, 		sumr_rec.crd_no);
				strcpy (local_rec.source_desc, 	sumr_rec.crd_name);
			}
			else
			{
				store [lcount [2]].fromHhsu = 0L;
				strcpy (asmv_rec.source_type, " ");
			}
		}

		if (asmv_rec.dest_type [0] == CUSTOMER)
		{
			cumr_rec.hhcu_hash = asmv_rec.to_hhcu;
			cc = find_rec (cumr2, &cumr_rec, COMPARISON , "r");
			if (!cc)
			{
				strcpy (local_rec.dest, cumr_rec.dbt_no);
				strcpy (local_rec.dest_desc, cumr_rec.dbt_name);
			}
			else
			{
				store [lcount [2]].toHhcu = 0L; 
				strcpy (asmv_rec.dest_type, " ");
			}
		}
		else if (asmv_rec.dest_type [0] == SUPPLIER)
		{
			sumr_rec.hhsu_hash = asmv_rec.to_crdt;
			cc = find_rec (sumr2, &sumr_rec, COMPARISON , "r");
			if (!cc)
			{
				strcpy (local_rec.dest, sumr_rec.crd_no);
				strcpy (local_rec.dest_desc, sumr_rec.crd_name);
			}
			else
			{
				store [lcount [2]].toHhsu = 0L;
				strcpy (asmv_rec.dest_type, " ");
			}
		}

		strcpy (local_rec.s_type, asmv_rec.source_type);
		strcpy (local_rec.d_type, asmv_rec.dest_type);

		putval (lcount [2]++);
		asmv_rec.line_no++;
		cc = find_rec (asmv, &asmv_rec, NEXT, "r");
	}

	line_cnt = 0;
	heading (2);
	scn_display (2);

}

/*
 * Update records.  
 */
void
Update (void)
{
	int 	line_no;
	char 	last_dest_type [2];
	long	lastHhsh,
			lastHhcu;

	scn_set (2);
	
	strcpy (last_dest_type, " ");
	lastHhcu = 0L;
	lastHhsh = 0L;

	for (line_no = 0;  line_no < lcount [2] ; line_no++)
	{
		getval (line_no);
		memset (&asmv_rec, 0, sizeof asmv_rec);
		asmv_rec.hhar_hash = asmr_rec.hhas_hash;
		asmv_rec.line_no = line_no;

		cc = find_rec (asmv, &asmv_rec, EQUAL, "u");
		
		asmv_rec.vol_commit = local_rec.vol_com ;
		asmv_rec.report_no = local_rec.report_no;
		strcpy (asmv_rec.move_code, local_rec.move_code);
		strcpy (asmv_rec.move_desc, local_rec.move_desc);

		if (line_no >= 1)
		{
			strcpy (asmv_rec.source_type, last_dest_type);
			asmv_rec.from_hhcu = lastHhcu;
			asmv_rec.from_crdt = lastHhsh;
		}
		else
		{
			strcpy (asmv_rec.source_type, local_rec.s_type);
			asmv_rec.from_hhcu = store [line_no].fromHhcu;
			asmv_rec.from_crdt = store [line_no].fromHhsu;
		}

		strcpy (asmv_rec.dest_type,   local_rec.d_type);
		strcpy (last_dest_type,       local_rec.d_type);
		lastHhcu = store [line_no].toHhcu;
		lastHhsh = store [line_no].toHhsu;

		asmv_rec.to_hhcu = store [line_no].toHhcu;
		asmv_rec.to_crdt = store [line_no].toHhsu;
		asmv_rec.move_date = local_rec.move_date;

		if (cc) 
		{
			cc = abc_add (asmv, &asmv_rec);
			if (cc) 
				file_err (cc, asmv, "DBADD");
		}
		else
		{
			cc = abc_update (asmv, &asmv_rec);
			if (cc) 
				file_err (cc, asmv, "DBUPDATE");
		}
	}
	abc_unlock (asmv);

	/*
	 * Update master record asmr with ID of last asset movement  
	 */
	memset (&asmv_rec, 0, sizeof asmv_rec);
	asmv_rec.hhar_hash = asmr_rec.hhas_hash;
	asmv_rec.line_no = lcount [2] - 1; /* last movement entered */

	cc = find_rec (asmv, &asmv_rec, COMPARISON, "r");

	strcpy (asmr_rec.co_no, 	comm_rec.co_no);
	strcpy (asmr_rec.br_no, 	comm_rec.est_no);
	strcpy (asmr_rec.ass_group, local_rec.AssetGroup);
	strcpy (asmr_rec.ass_no, 	local_rec.AssetNumber);
	strcpy (asmr_rec.serial_no, local_rec.serial_no);
	cc = find_rec (asmr, &asmr_rec, EQUAL, "u");
	if (!cc) 
	{
		asmr_rec.hham_hash = asmv_rec.hham_hash;
		cc = abc_update (asmr, &asmr_rec);
		if (cc) 
			file_err (cc, asmr, "DBUPDATE");
	}
	abc_unlock (asmr);
}

/*
 * edit () callback function 
 */
int
heading (
 int                scn)
{
	if (restart)
    	return (EXIT_SUCCESS);
	

	if (scn != cur_screen)
		scn_set (scn);

	switch (scn)
	{
	case 1:
		clear ();
		centre_at (0,132,ML ("%R Asset Movement Maintenance and Details"));
		box (0, 1, 132, 8);
		line_at (21,0,132);
		print_at (22,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		line_at (23,0,132);
		break;
	case 2:
		heading (1);

		print_at (5,3,  ML ("Last Location "));
		if (strlen (clip (location.last_cust)))
			print_at (5,25, ML ("Customer %s"), location.last_cust);
		print_at (5,90,ML ("Date %s"), DateToString (location.last_date));

		line_at (7,1,131);
		scn_display (1);
		tab_row = 11;
		tab_col = 1;
		getval (line_cnt);
		break;
	}

	scn_write (scn);
	return (EXIT_SUCCESS);
}

