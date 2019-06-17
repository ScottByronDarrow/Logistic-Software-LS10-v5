/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_acq_prn.c)                                    |
|  Program Desc  : ( Asset Systen Acquisition Report.  	  )           |	
|                  (                                      ) 		  |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr, asmr, asty, inmr, inex,               |
|  Database      : (Data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos  | Date Written  : 02/14/95         |
|---------------------------------------------------------------------|
|  Date Modified : (07/04/95)      | Modified  by  : Rene N. Bergado  |
|  Date Modified : (09/08/95)      | Modified  by  : Ronnel L Amanca  |
|  Date Modified : (05/02/98)      | Modified  by  : Elena B Cuaresma |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|  (04/07/95)    : Added the following fields in the report:          |
|				        Brand, Model No., Serial No.                  |
|                  Provided a selection criteria for acquisition date.|
|  (09/08/95)	 : SMF 00130. Testing and Fixes.			          |
|  (05/02/98)	 : 9.10  - New asset module.    			          |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: as_acq_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_acq_prn/as_acq_prn.c,v 5.5 2002/07/17 09:56:52 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#define     GREATER_THAN_START(x) 	 ((strcmp(x,local_rec.s_crdt) >=0) || !strcmp(local_rec.s_crdt,"      "))
#define     LESS_THAN_END(x) 	     ((strcmp(local_rec.e_crdt,x) >=0) || !strcmp(local_rec.e_crdt,"~~~~~~"))

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int comm_no_fields = 5;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	 /*============================+
	 | Company Master File Record. |
	 +============================*/
#define	COMR_NO_FIELDS	2

	struct dbview	comr_list [COMR_NO_FIELDS] =
	{
		{"comr_co_no"},
		{"comr_co_name"}
	};

	struct tag_comrRecord
	{
		char	co_no [3];
		char	co_name [41];
	}	comr_rec;

	/*==========================================+
	 | Establishment/Branch Master File Record. |
	 +==========================================*/
#define	ESMR_NO_FIELDS	3

	struct dbview	esmr_list [ESMR_NO_FIELDS] =
	{
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_est_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	br_name [41];
	}	esmr_rec;

	/*========================
	| Creditors Master File. |
	========================*/
#define	SUMR_NO_FIELDS	6

	struct dbview	sumr_list [SUMR_NO_FIELDS] =
	{
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_hhsu_hash"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
	};

	struct tag_sumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	crd_no [7];
		long	hhsu_hash;
		char	crd_name [41];
		char	acronym [10];
	}	sumr_rec;


	/*===================+
	 | Asset Master File |
	 +===================*/
#define	ASMR_NO_FIELDS	16

	struct dbview	asmr_list [ASMR_NO_FIELDS] =
	{
		{"asmr_co_no"},
		{"asmr_br_no"},
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



	 /*==================+
	 | Freezer Type File |
	 +==================*/
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


	int	envDbCo = 0,
		cr_find = 0;

	FILE	*fout,
			*fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	s_crdt[7];
	char	e_crdt[7];
	char	crdt_name[2][41];
    char    s_type_code[4];
	char    type_name [2][41];
	char    e_type_code[4];
	int		lpno;
	char	lp_str[3];
	char 	back[5];
	char	onite[5];
	char    co_no[3]; 
	char    co_desc[41]; 
	char    br_no[3]; 
	char    br_desc[41]; 
	long	from_date;
	long	to_date;
	char	from_date_str[11];
	char	to_date_str[11];
} local_rec;

char *asmr="asmr",
	 *sumr="sumr",
	 *esmr="esmr",
	 *comr="comr",
	 *data="data",
     *asty="asty";	


static	struct	var	vars[] =
{
	{1, LIN, "co_no", 3, 18, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Company No. ", "Enter Company Number. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.co_no},
	{1, LIN, "co_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.co_desc},
	{1, LIN, "br_no", 5, 18, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Branch Number    ", "Enter Branch No.. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_desc},
	{1, LIN, "from_crdt",	 7, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier No. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_crdt},
	{1, LIN, "name1",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.crdt_name[0]},
	{1, LIN, "to_crdt",	 8, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End   Supplier No. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_crdt},
	{1, LIN, "name2",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.crdt_name[1]},

	{1, LIN, "s_type_code", 10, 18, CHARTYPE,
		"UUU", "          ",
		"", " ", "Start Type Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_type_code},

	{1, LIN, "s_type_name",	 10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.type_name[0]},
	{1, LIN, "e_type_code", 11, 18, CHARTYPE,
		"UUU", "          ",
		" ", "~~~", "End   Type Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_type_code},
	{1, LIN, "e_type_name",	 11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.type_name[1]},

	{1, LIN, "st_acq_date",	 13, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Acq'n Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.from_date},

	{1, LIN, "end_acq_date",	 14, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "        "  , "End Acq'n Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.to_date},

	{1, LIN, "lpno",	 16, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 17, 18, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 17, 56, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindSumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void SrchAsty (char *);
void head_output (void);
void store_data (void);
void head_display (void);
void SrchEsmr (char *);
void SrchComr (char *);
int process_file (void);
int display_report (void);
int heading (int scn);
int spec_valid (int);
int run_prog (char *);

int 	FILE_EMPTY = 1;
char	curr_branchNumber 	[3],
		prev_branchNumber[41],
        curr_co 	[3],
		branchNumber		[3];

char  supplier  [7],
	  type_code [2],
	  previous  [7],
	  prev_co	[41],
	  brand 	[9],
	  model 	[9],
	  serial_no [26];

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{

	SETUP_SCR (vars);

	envDbCo = atoi(get_env("CR_CO"));
	cr_find = atoi(get_env("CR_FIND"));

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 10)
	{
		sprintf (local_rec.s_crdt,		"%-6.6s",argv[1]);
		sprintf (local_rec.e_crdt,		"%-6.6s",argv[2]);
		sprintf (local_rec.s_type_code,	"%-3.3s",argv[3]);
		sprintf (local_rec.e_type_code,	"%-3.3s",argv[4]);
		sprintf (local_rec.br_no,		"%2.2s",argv[5]);
		sprintf (local_rec.co_no,		"%2.2s",argv[6]);

		local_rec.from_date 	= atol(argv[7]);
		local_rec.to_date 		= atol(argv[8]);
		local_rec.lpno 			= atoi(argv[9]);


        dsp_screen ("Processing : Printing Acquisition Report.",
                    comm_rec.tco_no,comm_rec.tco_name);

		head_output ();
		process_file ();
		display_report ();
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();          /*  get into raw mode		*/
	set_masks();		/*  setup print using masks	*/
	init_vars(1);		/*  set default values		*/

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit	= 0;
		edit_exit	= 0;
		prog_exit	= 0;
		restart		= 0;
		search_ok	= 1;
		init_vars(1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading(1);
		scn_display(1);
		edit(1);
		if (restart)
			continue;

		if (run_prog (argv[0]) == 1)
            return (EXIT_FAILURE);
        
		prog_exit = 1;
	}
	shutdown_prog ();
	abc_dbclose (data);

    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(sumr,sumr_list,SUMR_NO_FIELDS,(envDbCo) ? "sumr_id_no" 
							    : "sumr_id_no3");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (asty, asty_list, ASTY_NO_FIELDS, "asty_id_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(sumr);
	abc_fclose(esmr);
	abc_fclose(comr);
    abc_fclose(asty);
	abc_fclose(asmr);			
}


int
spec_valid (
 int                field)
{
	/*------------------------------------------
	| Validate Company Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK("co_no"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.co_no, "  ");
			sprintf (local_rec.co_desc, "%s", ML("All Company"));
			DSP_FLD ("co_no");
			DSP_FLD ("co_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchComr (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (comr_rec.co_no, local_rec.co_no);
		cc = find_rec (comr, &comr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML(mlStdMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.co_desc, comr_rec.co_name);
		DSP_FLD ("co_no");
		DSP_FLD ("co_desc");
	}

	/*------------------------------------------
	| Validate Branch Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK("br_no"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.br_no, "  ");
			sprintf (local_rec.br_desc, "%-40.40s", ML("All Branches"));
			display_field (label("br_no"));
			display_field (label("br_desc"));
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchEsmr (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		if (strcmp (local_rec.co_no, "  "))
			strcpy (esmr_rec.co_no, local_rec.co_no);
		else
			strcpy (esmr_rec.co_no, comm_rec.tco_no);
		strcpy (esmr_rec.br_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML(mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.br_desc, esmr_rec.br_name);
		DSP_FLD ("br_no");
		DSP_FLD ("br_desc");
	}

	/*-------------------
	| Validate Creditor |
	-------------------*/
	if ( LCHECK( "from_crdt") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.crdt_name[0],"%-40.40s", ML("Start Creditor"));
			DSP_FLD("name1");
			return(0);
		}

		if (SRCH_KEY)
		{
			char	temp[3];
			if (strcmp (local_rec.co_no, "  "))
			{
				strcpy (temp, comm_rec.tco_no);
				strcpy (comm_rec.tco_no, local_rec.co_no);
			}
			
			SumrSearch(comm_rec.tco_no, branchNumber, temp_str);

			if (strcmp (local_rec.co_no, "  "))
				strcpy (comm_rec.tco_no, temp);
			return(0);
		}
		if (prog_status != ENTRY &&
			strcmp(local_rec.s_crdt,local_rec.e_crdt) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}

		memset(&sumr_rec,0,sizeof sumr_rec);
		if (strcmp (local_rec.co_no, "  "))
			strcpy(sumr_rec.co_no,local_rec.co_no);
		else
			strcpy(sumr_rec.co_no,comm_rec.tco_no);
		if (envDbCo)
			strcpy(sumr_rec.est_no,local_rec.br_no);
		strcpy(sumr_rec.crd_no,local_rec.s_crdt);
		if (!strcmp (local_rec.br_no, "  "))
		{
			cc = find_rec("sumr",&sumr_rec,GTEQ,"r");
			if (strcmp (sumr_rec.crd_no, local_rec.s_crdt))
				cc = 0;
		}
		else
			cc = find_rec("sumr",&sumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess022));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.crdt_name[0],sumr_rec.crd_name);
		DSP_FLD( "name1" );
		return(0);
	}

	if ( LCHECK("to_crdt") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.crdt_name[1],"%-40.40s", ML("End   Creditor"));
			DSP_FLD("name2");
			return(0);
		}

		if (SRCH_KEY)
		{
			SumrSearch(comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		memset(&sumr_rec,0,sizeof sumr_rec);
		if (strcmp (local_rec.co_no, "  "))
			strcpy(sumr_rec.co_no, local_rec.co_no);
		else
			strcpy(sumr_rec.co_no, comm_rec.tco_no);

		if (envDbCo)
			strcpy(sumr_rec.est_no,local_rec.br_no);
		strcpy(sumr_rec.crd_no,local_rec.s_crdt);
		if (!strcmp (local_rec.br_no, "  "))
		{
			cc = find_rec("sumr",&sumr_rec,GTEQ,"r");
			if (strcmp (sumr_rec.crd_no, local_rec.s_crdt))
				cc = 0;
		}
		else
			cc = find_rec("sumr",&sumr_rec,COMPARISON,"r");

		if (cc)
		{
			print_mess(ML(mlStdMess022));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strcmp(local_rec.s_crdt,local_rec.e_crdt) > 0)
		{
			errmess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.crdt_name[1],sumr_rec.crd_name);
		DSP_FLD("name2");
		return(0);
	}

	/*----------------------+
	| Validate Spec. Code 1 |
	------------------------*/
	if ( LCHECK( "s_type_code") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.type_name[0],ML("Start Type Code"));
			DSP_FLD("s_type_name");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchAsty(temp_str);
			return(0);
		}
		if (prog_status != ENTRY &&
			strcmp(local_rec.s_type_code,local_rec.e_type_code) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strcmp (local_rec.co_no, "  "))
			strcpy(asty_rec.co_no, local_rec.co_no);
		else
			strcpy(asty_rec.co_no,comm_rec.tco_no);
		strcpy(asty_rec.type_code,local_rec.s_type_code);
		cc = find_rec(asty,&asty_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess227));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.type_name[0],asty_rec.type_desc);
		DSP_FLD( "s_type_name" );
		return(0);
	}



	/*----------------------+
	| Validate Spec. Code 2 |
	------------------------*/
	if ( LCHECK( "e_type_code") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.type_name[1],"%-40.40s", ML("End   Type Code"));
			DSP_FLD("e_type_name");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchAsty(temp_str);
			return(0);
		}
		if (strcmp(local_rec.s_type_code,local_rec.e_type_code) > 0)
		{
			print_mess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strcmp (local_rec.co_no, "  "))
			strcpy(asty_rec.co_no, local_rec.co_no);
		else
			strcpy(asty_rec.co_no, comm_rec.tco_no);
		strcpy(asty_rec.type_code,local_rec.e_type_code);
		cc = find_rec(asty,&asty_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess227));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.type_name[1],asty_rec.type_desc);
		DSP_FLD( "e_type_name" );
		return(0);
	}	


	/*----------------------------------------
	| Validate Freezer Start Acquisition Date |
	----------------------------------------*/

	if ( LCHECK("st_acq_date") )
	{
		local_rec.to_date = TodaysDate();
		if ( local_rec.from_date > TodaysDate ())
		{
			print_mess(ML(mlStdMess068));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (local_rec.from_date > local_rec.to_date) 
		{
			print_mess(ML(mlStdMess019));
			sleep(1);
			clear_mess();
			return(1);
		}

		DSP_FLD("st_acq_date");
		return(0);
	}

	/*---------------------------------------
	| Validate  Freezer Acquisition End Date|
	---------------------------------------*/

	if ( LCHECK("end_acq_date") )
	{
		if (dflt_used)
		{
			local_rec.to_date = TodaysDate ();
			DSP_FLD("end_acq_date");
			return(0);
		}

		if ( local_rec.to_date > TodaysDate ())
		{
			print_mess(ML(mlStdMess068));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (local_rec.from_date > local_rec.to_date)
		{
			errmess(ML(mlStdMess058));
			sleep(1);
			clear_mess();
			return(1);
		}

		DSP_FLD("end_acq_date");
		return(0);

	}

	if ( LCHECK("lpno") )
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		return(0);
	}

	if ( LCHECK("back") )
	{
		strcpy(local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD( "back" );
		return(0);
	}

	if ( LCHECK("onight") )
	{
		strcpy(local_rec.onite,(local_rec.onite[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD( "onight" );
		return(0);
	}

	return (EXIT_SUCCESS);
}



/*===========================
| Search Freezer Type File. |
===========================*/
void
SrchAsty (
 char*              key_val)
{
	struct tag_astyRecord asty_bak;

	memcpy (&asty_bak, &asty_rec, sizeof asty_bak);

	work_open();
	save_rec("#Code","#Description");

	if (strcmp (local_rec.co_no, "  "))
		strcpy (asty_rec.co_no, local_rec.co_no);
	else	
		strcpy (asty_rec.co_no, comm_rec.tco_no);
	strcpy (asty_rec.type_code, key_val);

	cc = find_rec (asty, &asty_rec, GTEQ, "r");

	while (	!cc &&
			(strcmp (local_rec.co_no, "  ") ?
				!strcmp (asty_rec.co_no, local_rec.co_no) :
				!strcmp (asty_rec.co_no, comm_rec.tco_no)) &&
			!strncmp (asty_rec.type_code, key_val, strlen(key_val)))
	{
		cc = save_rec (asty_rec.type_code, asty_rec.type_desc);
		if (cc)
			break;

		cc = find_rec (asty, &asty_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close ();
	if (!cc)
	{
		/*-----------------------
		| Read selected record	|
		-----------------------*/
		if (strcmp (local_rec.co_no, "  "))
			strcpy (asty_rec.co_no, local_rec.co_no);
		else	
			strcpy (asty_rec.co_no, comm_rec.tco_no);
		strcpy (asty_rec.type_code, temp_str);
		cc = find_rec (asty, &asty_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, asty, "DBFIND");
	}

	if (cc)
		memcpy (&asty_rec, &asty_bak, sizeof asty_rec);
}

int
run_prog (
 char	*prog_name)
{

	sprintf (local_rec.lp_str, "%d", local_rec.lpno);
	sprintf (local_rec.from_date_str, "%ld", local_rec.from_date);
	sprintf (local_rec.to_date_str, "%ld", local_rec.to_date);

	CloseDB (); 
	FinishProgram ();

	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	if (local_rec.onite[0] == 'Y')
	{
		if (fork() == 0)
        {
            execlp ("ONIGHT",
                    "ONIGHT",
                    prog_name,
                    local_rec.s_crdt,
                    local_rec.e_crdt,
                    local_rec.s_type_code,
                    local_rec.e_type_code,
                    local_rec.br_no,
                    local_rec.co_no,
                    local_rec.from_date_str,
                    local_rec.to_date_str,
                    local_rec.lp_str,
                    "Print Freezer Acquisition By Supplier Code.",
                    (char *)0);
        }
        return (EXIT_FAILURE);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
        {
            execlp (prog_name,
                    prog_name,
                    local_rec.s_crdt,
                    local_rec.e_crdt,
                    local_rec.s_type_code,
                    local_rec.e_type_code,
                    local_rec.br_no,
                    local_rec.co_no,
                    local_rec.from_date_str,
                    local_rec.to_date_str,
                    local_rec.lp_str,
                    (char *)0);
        }
        return (EXIT_FAILURE);
	}
	else 
	{
        execlp (prog_name,
                prog_name,
                local_rec.s_crdt,
                local_rec.e_crdt,
                local_rec.s_type_code,
                local_rec.e_type_code,
                local_rec.br_no,
                local_rec.co_no,
                local_rec.from_date_str,
                local_rec.to_date_str,
                local_rec.lp_str,
                (char *)0);
	}
    return (EXIT_SUCCESS);
}


/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (void)
{
	if((fout = popen ("pformat","w")) == NULL)
		sys_err("Error in pformat During (POPEN)",errno, PNAME);

	fprintf(fout,".START%s <%s>\n",DateToString (TodaysDate()),PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI16\n");

	fprintf(fout,".5\n");
	fprintf(fout,".L212\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".EASSET ACQUISITION REPORT BY SUPPLIER CODE.\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".R%-23.23s================================================"," ");
	fprintf(fout,"===========================================================");
	fprintf(fout,"==========================================================\n");

	fflush(fout);
}


int
process_file (void)
{
	char	*sptr;
	char	br_no[3];

	fsort = sort_open("r_acq_asset");
	FILE_EMPTY = 1;
	
	memset (&sumr_rec,0,sizeof sumr_rec);
	sprintf (br_no,"%-2.2s",local_rec.br_no);
	

	if (strcmp (local_rec.co_no, "  "))
    	strcpy (sumr_rec.co_no, local_rec.co_no);
	else
    	strcpy (sumr_rec.co_no, "  ");

	/*RLA add */
	sptr = chk_env ("CR_CO");
    if (!strcmp (sptr,"0"))
		strcpy (sumr_rec.est_no, " 0");
	if (strcmp  (sptr,"0"))
	{
		if (strcmp (local_rec.br_no, "  "))
    		strcpy (sumr_rec.est_no, local_rec.br_no);
		else
    		strcpy (sumr_rec.est_no, "  ");
	}
	/* end RLA add */

	strcpy (sumr_rec.crd_no, zero_pad(local_rec.s_crdt,6));

	cc = find_rec(sumr,&sumr_rec,GTEQ,"r");
	while (!cc && 
	 	   ( strcmp (local_rec.co_no, "  ")
				? !strcmp(sumr_rec.co_no,local_rec.co_no) : 1) && 
			(!strcmp (sumr_rec.est_no, local_rec.br_no) ||
			 !envDbCo))
	{
		 if ( GREATER_THAN_START(sumr_rec.crd_no) && 
		      LESS_THAN_END(sumr_rec.crd_no ))  
		{
				memset(&asmr_rec,0,sizeof asmr_rec);
				asmr_rec.hhsu_hash = sumr_rec.hhsu_hash;
	    		cc = find_rec(asmr,&asmr_rec,GTEQ,"r");

	    		while (!cc && asmr_rec.hhsu_hash == sumr_rec.hhsu_hash)
				{
					if ((!strcmp (sumr_rec.est_no, local_rec.br_no) ||
						 !strcmp (br_no,"  ") ||
					     !envDbCo) &&
		    			( strcmp (local_rec.co_no, "  ") 
						? !strcmp(sumr_rec.co_no,local_rec.co_no) : 1 ))
					        {
						if (strcmp(asmr_rec.type, local_rec.s_type_code) >= 0 &&
							strcmp(asmr_rec.type, local_rec.e_type_code) <= 0 && 
                            asmr_rec.pur_date >= local_rec.from_date && 
                            asmr_rec.pur_date <= local_rec.to_date )
							
						{
								if (!strcmp(asmr_rec.br_no,local_rec.br_no) ||
								    !strcmp(br_no,"  "))
								{
									store_data();
								}
						}

					}
					cc = find_rec (asmr, &asmr_rec, NEXT, "r");
				}
		}
		cc = find_rec("sumr",&sumr_rec,NEXT,"r");

	}
	return (EXIT_SUCCESS);
}

void
store_data (void)
{
	char data_string [80];

	FILE_EMPTY = 0;
    sprintf(data_string,"%-2.2s %-2.2s %-2.2s %-6.6s %-3.3s %-8.8s %-8.8s %-25.25s\n",
    asmr_rec.co_no,
	asmr_rec.br_no,
    sumr_rec.est_no,
    sumr_rec.crd_no,
	asmr_rec.type,
	asmr_rec.brand,
	asmr_rec.spec2_code,
	asmr_rec.serial_no);
	sort_save (fsort,data_string);
}

int
display_report (void)
{
	int   tot_fr_purchased = 0,
		  first_time       = TRUE;
	char  *sptr;
	
	fsort = sort_sort(fsort, "r_acq_asset");
	sptr =  sort_read(fsort);

	strcpy (brand,     "        ");
	strcpy (model,     "        ");
	strcpy (supplier,  "      ");
	strcpy (previous,  "      ");
	strcpy (prev_branchNumber,"  ");
	strcpy (prev_co,   "  ");
	strcpy (type_code,   " ");
	sprintf(serial_no, "%-25.25s", " ");

	while (sptr != (char *)0) 
	{
	  	sprintf (curr_co,     "%-2.2s",  sptr );
	  	sprintf (curr_branchNumber,  "%-2.2s",  sptr + 3);
		sprintf (branchNumber,       "%-2.2s",  sptr + 6);
	  	sprintf (supplier,    "%-6.6s",  sptr + 9);
		sprintf (type_code,   "%-3.3s",  sptr + 16);
		sprintf (brand,   "%-8.8s",  sptr + 20);
		sprintf (model,   "%-8.8s",  sptr + 29);
		sprintf (serial_no,   "%-25.25s",  sptr + 38);


		if (first_time)
			head_display();
			/*head_output();*/


		if (strcmp(supplier,previous) || 
			strcmp (curr_co, prev_co) ||
			strcmp (curr_branchNumber, prev_branchNumber))
		{
			if (!first_time && strcmp(supplier,previous))
			{
				fprintf(fout,".C|%-57.57s"," ");
				fprintf(fout,"|             ");
				fprintf(fout,"|  Total no. of Assets purchased : %3d  ",
						tot_fr_purchased);
				fprintf(fout,"%-52.52s|\n"," ");
			}
		

			if (strcmp (curr_co, prev_co) ||
				strcmp (curr_branchNumber, prev_branchNumber))
			{
				head_display();

				if (!first_time)
					fprintf (fout, ".PA\n");
			}

			tot_fr_purchased = 1;	
			memset(&sumr_rec,0,sizeof sumr_rec);
			strcpy(sumr_rec.co_no,curr_co);
			if (envDbCo)
				strcpy(sumr_rec.est_no,branchNumber);
			strcpy (sumr_rec.crd_no, supplier);
			cc = find_rec(sumr,&sumr_rec,COMPARISON,"r");
			if (cc)
				memset (&sumr_rec, 0, sizeof sumr_rec);

	    	cc = find_hash(asmr,&asmr_rec,GTEQ,"r",sumr_rec.hhsu_hash);	


			if (!cc)
			{
				fprintf(fout,".LRP5\n");
				if (!first_time &&
					!strcmp (curr_co, prev_co) &&
					!strcmp (curr_branchNumber, prev_branchNumber))
				{
					fprintf(fout,".C|------------|--------------------------");
					fprintf(fout,"------------------|-------------|");
					fprintf(fout,"--------------------------------------------|");
					fprintf(fout,"---------|");
					fprintf(fout,"---------|");
					fprintf(fout,"--------------------------|\n");
				}
				first_time = FALSE;
				fprintf(fout,".C| %-6.6s     |  %-146.146s  |\n",										    supplier,sumr_rec.crd_name);
			}
			fprintf(fout,".C|%57.57s"," ");
		}
		else
		{
			tot_fr_purchased++;
			fprintf(fout,".C|%57.57s"," ");
		}

   		fprintf(fout, "|     %-3.3s     ",   type_code);
		strcpy (asty_rec.co_no, curr_co);
		strcpy (asty_rec.type_code, type_code);
		cc = find_rec (asty, &asty_rec, EQUAL, "r");
		if (!cc)
   			fprintf(fout, "|  %-40.40s  |", asty_rec.type_desc);
		else
   			fprintf(fout, "|  %-40.40s  |", " ");
		fprintf(fout," %-8.8s|", brand);
		fprintf(fout," %-8.8s|", model);
		fprintf(fout," %-25.25s|",serial_no);
		fprintf(fout,"\n");
		
		sptr =  sort_read(fsort);
		sprintf (previous,  "%-6.6s", supplier);
		sprintf (prev_co,	"%-2.2s", curr_co);
		sprintf (prev_branchNumber,"%-2.2s", curr_branchNumber);

	}

	if (!FILE_EMPTY)
	{
		fprintf(fout,".C|%-57.57s"," ");
		fprintf(fout,"|             ");
		fprintf(fout,"|  Total no. of Assets purchased : %d  ",
				tot_fr_purchased);
		fprintf(fout,"%-54.54s|\n"," ");
	}	

	fprintf(fout,".EOF\n");
	fflush (fout);
	sort_delete (fsort, "r_acq_asset");
	return(0);

}

void
head_display (void)
{
	fprintf (fout, ".DS10\n");

	sprintf (comr_rec.co_no,  "%-2.2s", curr_co);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (!cc)
		fprintf(fout,".ECompany : %s - %s\n",curr_co, comr_rec.co_name);
	else
		fprintf(fout,".ECompany : Unknown Company\n");

	strcpy  (esmr_rec.co_no,  curr_co);
	sprintf (esmr_rec.br_no,  "%-2.2s", curr_branchNumber);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (!cc)
		fprintf(fout,".EBranch : %s - %s\n", curr_branchNumber, esmr_rec.br_name);
	else
		fprintf(fout,".EBranch : Unknown Branch\n");
	
	fprintf(fout,".B1\n");
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf(fout,".B1\n");
	fprintf(fout,".CStart Supplier: %-6.6s End Supplier: %-6.6s /  ",
				local_rec.s_crdt, local_rec.e_crdt);
	fprintf(fout,"Start Freezer Type : %-3.3s  End Freezer Type: %-3.3s / ", 
				local_rec.s_type_code, local_rec.e_type_code);
	fprintf (fout," Start Acquisition Date : %s ",DateToString(local_rec.from_date ));
	fprintf (fout," End Acquisition Date : %s\n", DateToString(local_rec.to_date));
 				
    
	fprintf(fout,".C============================================");
	fprintf(fout,"============================================");
	fprintf(fout,"==========");
	fprintf(fout,"==========");
	fprintf(fout,"===========================");
	fprintf(fout,"==============================\n");
	fprintf(fout,".C|  SUPPLIER  ");
	fprintf(fout,"|              SUPPLIER NAME                 ");
	fprintf(fout,"|    ASSET    ");
	fprintf(fout,"|          ASSET TYPE DESCRIPTION            |");
	fprintf(fout,"  BRAND  |");
	fprintf(fout,"  MODEL  |");
	fprintf(fout,"     S E R I A L   N O.   |\n");

	fprintf(fout,".C|   NUMBER   ");
	fprintf(fout,"|  %-40.40s  "," ");
	fprintf(fout,"|  TYPE CODE  |%-44.44s|"," ");
	fprintf(fout,"         |");
	fprintf(fout,"   NO.   |");
	fprintf(fout,"                          |\n");

	fprintf(fout,".C|------------|--------------------------");
	fprintf(fout,"------------------|-------------|");
	fprintf(fout,"--------------------------------------------|");
	fprintf(fout,"---------|");
	fprintf(fout,"---------|");
	fprintf(fout,"--------------------------|\n");
	fflush (fout);

}

int
heading (
 int                scn)
{
	if ( restart )
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);
	clear();
	rv_pr(ML(" Freezer Acquisition Report "),25,0,1);
	move(0,1);
	line(80);

	box(0,2,80,15);
	move(1,4);
	line(79);

	move(1,6);
	line(79);

	move(1,9);
	line(79);

    move(1,12);
    line(79);

    move(1,15);
    line(79);

	move(0,19);
	line(80);
	print_at(20,0, ML(mlStdMess038), comm_rec.tco_no,clip(comm_rec.tco_name)); 
	print_at(21,0, ML(mlStdMess039), comm_rec.test_no,clip(comm_rec.test_name));
	move(0,22);
	line(80);
	line_cnt = 0;
	scn_write(scn);
    return (EXIT_SUCCESS);
}

/*================================
| Search for Branch master file. |
================================*/
void
SrchEsmr (
 char*              key_val)
{
	work_open ();
	if (strcmp (local_rec.co_no, "  "))
		strcpy  (esmr_rec.co_no,  local_rec.co_no);
	else
		strcpy  (esmr_rec.co_no,  comm_rec.tco_no);
	sprintf (esmr_rec.br_no,  "%-2.2s", key_val);
	save_rec ("#Br", "#Branch Description");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		   (strcmp (local_rec.co_no, "  ") ?
        		!strcmp  (esmr_rec.co_no, local_rec.co_no) :
        		!strcmp  (esmr_rec.co_no, comm_rec.tco_no)) &&
           !strncmp (esmr_rec.br_no, key_val, strlen (key_val)) )
	{
		cc = save_rec (esmr_rec.br_no, esmr_rec.br_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (strcmp (local_rec.co_no, "  "))
		strcpy  (esmr_rec.co_no,  local_rec.co_no);
	else
		strcpy  (esmr_rec.co_no,  comm_rec.tco_no);
	sprintf (esmr_rec.br_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
	 	file_err (cc, esmr, "DBFIND");
}

/*================================
| Search for Company master file. |
================================*/
void
SrchComr (
 char*              key_val)
{
	work_open ();
	sprintf (comr_rec.co_no,  "%-2.2s", key_val);
	save_rec ("#CO", "#Company Description");
	cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc 
	&&     !strncmp (comr_rec.co_no, key_val, strlen (key_val)) )
	{
		cc = save_rec (comr_rec.co_no, comr_rec.co_name);
		if (cc)
			break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_no, "%-2.2s", temp_str);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (cc)
	 	file_err (cc, comr, "DBFIND");
}
