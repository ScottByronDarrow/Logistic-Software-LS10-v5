/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_dep_prn.c)                                    |
|  Program Desc  : ( Print Asset Deployment by Salesman Code   )      |	
|                  (                                           )      |
|---------------------------------------------------------------------|
|  Access files  :  comm, exsf, cumr, asmv, asmr, assc, asbr, exaf    |
|  Database      : (Data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos. | Date Written  : 02/14/95         |
|---------------------------------------------------------------------|
|  Date Modified : (04/09/95)      | Modified  by  : Ronnel L. Amanca |
|  Date Modified : (08/20/95)      | Modified  by  : Rommel S. Maldia |
|  Date Modified : (03/13/96)      | Modified  by  : Alan G. Rivera   |
|  Date Modified : (06/02/98)      | Modified  by  : Elena B. Cuaresma|
|                :                                                    |
|  Comments      :                                                    |
|                :  SMF 00034 - Fixed bugs, added some report features|
|				 :				as per SMFI requests.				  |
|  (03/13/96)    :  SMF 00075 - Removed field cumr_mkt_rep            |
|  (06/02/98)    :  9.10  New asset module for the standard version 9.|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: as_dep_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_dep_prn/as_dep_prn.c,v 5.3 2002/07/17 09:56:54 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_tr_mess.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include    <DateToString.h>

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

	/*=============================================
	| file exsf {External Salesman file Record} . |
	=============================================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_area_code"},
		{"exsf_sell_grp"}
	};

	int exsf_no_fields = 5;

	struct {
		char 	sf_co_no[3];
		char 	sf_sal_no[3];
		char 	sf_sal_man[41];
		char 	sf_area_code[3];
		char	sell_grp[3];
	} exsf_rec;

	/*====================
	| Sales group file |
	====================*/
	struct dbview sasg_list [] =
	{
		{"sasg_co_no"},
		{"sasg_sell_grp"},
		{"sasg_desc"}
	};

	int	sasg_no_fields = 3;

	struct tag_sasgRecord
	{
		char	co_no [3];
		char	sell_grp [3];
		char	desc [41];
	} sasg_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	8

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_sman_code"},
	};

	struct tag_cumrRecord
	{
		char	cm_co_no [3];
		char	cm_est_no [3];
		char	cm_department [3];
		char	cm_dbt_no [7];
		long	cm_hhcu_hash;
		char	cm_name [41];
		char	cm_dbt_acronym [10];
		char	cm_sman_code [3];
	}	cumr_rec;

#define	ASMV_NO_FIELDS	5 

	struct dbview	asmv_list [ASMV_NO_FIELDS] =
	{
		{"asmv_hhar_hash"},
		{"asmv_line_no"},
		{"asmv_hham_hash"},
		{"asmv_serial_no"},
		{"asmv_to_hhcu"},
	};

	struct tag_asmvRecord
	{
		long	hhas_hash;
		int		line_no;
		long	hhfm_hash;
		char	serial_no [26];
		long	to_hhcu;
	}	asmv_rec;

	/*===================+
	 | Asset Master File |
	 +===================*/
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

	/*====================
	| External Area file |
	====================*/
	struct dbview exaf_list [] =
	{
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
	};

	int	exaf_no_fields = 3;

	struct tag_exafRecord
	{
		char	co_no [3];
		char	code [3];
		char	desc [41];
	} exaf_rec;


	char	branchNo[3];

	FILE *fout,
		 *fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
    char    s_area_code[3];
	char    e_area_code[3];
	char    area_name [2][41];
    char    s_dbt_no[7];
    char    e_dbt_no[7];
    char    dbt_name[2][41];
	int	lpno;
	char	lp_str[3];
	char 	back[5];
	char	onite[5];
	char    br_no[3]; 
	char    br_desc[41]; 
	char	st_sell_grp[3];
	char	end_sell_grp[3];
	char	st_sasg_desc[41];
	char	end_sasg_desc[41];
	char	co_no[3];
	char    co_desc[41]; 
} local_rec;

char 	*asmr	= "asmr",
		*asmv	= "asmv",	
		*data	= "data",
		*sasg	= "sasg",
		*exaf	= "exaf",
		*exsf	= "exsf",
		*cumr2	= "cumr2",	
		*cumr	= "cumr",
		*cuot	= "cuot",
		*asbr	= "asbr",
		*assc	= "assc",
		*comr	= "comr",
		*esmr	= "esmr";

static	struct	var	vars[] =
{
	{1, LIN, "co_no", 3, 18, CHARTYPE,
		"NN", "          ",
		" ", "~~", "Company No. ", "Enter Company Number. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.co_no},
	{1, LIN, "co_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.co_desc},
	{1, LIN, "br_no", 5, 18, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Branch Number. ", "Enter Branch . [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_desc},
	{1, LIN, "st_sell_grp",	 7, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Sales Group", "Enter Sales Group Code. [SEARCH].",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_sell_grp},
	{1, LIN, "st_sasg_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.st_sasg_desc},
	{1, LIN, "end_sell_grp",	 8, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "End Sales Group", "Enter Sales Group Code. [SEARCH].",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_sell_grp},
	{1, LIN, "end_sasg_desc",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.end_sasg_desc},
	{1, LIN, "s_area_code", 10, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Area Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_area_code},
	{1, LIN, "s_area_name",	 10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_name[0]},
	{1, LIN, "e_area_code", 11, 18, CHARTYPE,
		"UU", "          ",
		" ", "~~", "End   Area Code ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_area_code},
	{1, LIN, "e_area_name",	 11, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.area_name[1]},
	{1, LIN, "s_dbt_no", 13, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_dbt_no},
	{1, LIN, "s_dbt_name",	 13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dbt_name[0]},
	{1, LIN, "e_dbt_no", 14, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End   Customer", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_dbt_no},
	{1, LIN, "e_dbt_name",	 14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dbt_name[1]},
	{1, LIN, "lpno",	 16, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 17, 18, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight", 17,56, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int);
void SrchSasg (char *);
void SrchExaf (char *);
void SrchExsf (char *);
int run_prog (char *);
void head_output (void);
void process_file (void);
void store_data (void);
void display_report (void);
void DefinedSection (void);
void DefinedSection2 (char *, char *, char *, char *);
void print_line (void);
int heading (int);
void SrchEsmr (char *);
void SrchComr (char *);

char	curr_co [3];
char	curr_branchNo [3];


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int            argc,
 char*          argv[])
{
	if (argc != 1 && argc != 10)
	{
		print_at(0,0, mlTrMess703, argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	read_comm ( comm_list, comm_no_fields, (char *) &comm_rec );

	if (argc == 10)
	{
		sprintf (local_rec.st_sell_grp,"%-2.2s",argv[1]);
		sprintf (local_rec.end_sell_grp,"%-2.2s",argv[2]);
		sprintf (local_rec.s_area_code,"%-2.2s",argv[3]);
		sprintf (local_rec.e_area_code,"%-2.2s",argv[4]);
		sprintf (local_rec.s_dbt_no   ,"%-6.6s",argv[5]);
		sprintf (local_rec.e_dbt_no   ,"%-6.6s",argv[6]);
		sprintf (local_rec.br_no   	 ,"%2.2s",argv[7]);
		sprintf (local_rec.co_no   	 ,"%2.2s",argv[8]);
		local_rec.lpno = atoi (argv[9]);

		dsp_screen("Processing : Asset Deployment Report.",
					comm_rec.tco_no,comm_rec.tco_name);

		head_output ();

		process_file ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr();				/*  sets terminal from termcap	*/
	set_tty();              /*  get into raw mode			*/
	set_masks();			/*  setup print using masks		*/
	init_vars(1);			/*  set default values			*/

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
        {
            return (EXIT_SUCCESS);
        }

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

	abc_alias(cumr2, cumr);
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (asmv, asmv_list, ASMV_NO_FIELDS, "asmv_id_no"); 
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (exsf, exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec (exaf, exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec (assc, assc_list, ASSC_NO_FIELDS, "assc_id_no");
	open_rec (sasg, sasg_list, sasg_no_fields, "sasg_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(asmr);
    abc_fclose(asmv);
	abc_fclose(cumr2);	
	abc_fclose(cumr);
	abc_fclose(esmr);
	abc_fclose(comr);
	abc_fclose(exsf);
	abc_fclose(exaf);	
	abc_fclose(sasg);	
	abc_fclose(assc);	
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
			sprintf (local_rec.co_desc, "%s", ML("All Company"));
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
		return (EXIT_SUCCESS);
	}

	/*------------------------------------------
	| Validate Branch Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK("br_no"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.br_no, "~~");
			sprintf (local_rec.br_desc, "%-40.40s", ML("All Branchs"));
			display_field (label("br_no"));
			display_field (label("br_desc"));
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchEsmr (temp_str);
  			return (EXIT_SUCCESS);
		}
		
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

	/*----------------------------------
	| Validate Start Sales Group Code. |
	----------------------------------*/
	if (LCHECK("st_sell_grp"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.st_sell_grp, "%-2.2s", " ");
			strcpy (local_rec.st_sasg_desc, ML("Start Selling Group"));
			DSP_FLD ("st_sasg_desc");
			DSP_FLD ("st_sell_grp");
			return (EXIT_SUCCESS);
		}

		FLD ("end_sell_grp") = YES;
		if (SRCH_KEY)
		{
			SrchSasg(temp_str);
			return(0);
		}

		if  (prog_status != ENTRY &&
				 strcmp(local_rec.st_sell_grp, local_rec.end_sell_grp) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess();
			return(1);
		}

		if ( !strcmp (local_rec.st_sell_grp, "  " ) )
		{
			errmess (ML("Sales group code cannot be blank!"));
			sleep(1);	
			clear_mess();
			return(1);
		}
		strcpy( sasg_rec.co_no, comm_rec.tco_no );
		strcpy( sasg_rec.sell_grp, local_rec.st_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		if ( cc )
		{
			errmess( ML("Sales group not found.") );
			sleep(1);
			clear_mess();
			return( 1 );
		}

		sprintf (local_rec.st_sasg_desc, "%-40.40s", sasg_rec.desc);
		DSP_FLD ("st_sasg_desc");
		return(0);
	}

	/*--------------------------------
	| Validate End Sales Group Code. |
	--------------------------------*/
	if (LCHECK("end_sell_grp"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.end_sell_grp, "~~");
			strcpy (local_rec.end_sasg_desc, ML("End Selling Group"));
			DSP_FLD ("end_sell_grp");
			DSP_FLD ("end_sasg_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSasg(temp_str);
			return(0);
		}

		if (strcmp(local_rec.st_sell_grp, local_rec.end_sell_grp) > 0)
		{
			print_mess(ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess();
			return(1);
		}

		if ( !strcmp (local_rec.end_sell_grp, "  " ) )
		{
			errmess (ML("Sales group code cannot be blank!"));
			sleep(1);	
			clear_mess();
			return(1);
		}
		strcpy( sasg_rec.co_no, comm_rec.tco_no );
		strcpy( sasg_rec.sell_grp, local_rec.end_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		if (cc)
		{
			errmess( ML("Sales group not found.") );
			sleep(1);
			clear_mess();
			return( 1 );
		}
		sprintf (local_rec.end_sasg_desc, "%-40.40s", sasg_rec.desc);
		DSP_FLD ("end_sasg_desc");
		return(0);
	} 
	/*----------------------+
	|  Validate Area Code 1 |
	------------------------*/
	if ( LCHECK( "s_area_code") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.area_name[0], ML("Start Area Code"));
			DSP_FLD("s_area_name");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchExaf(temp_str);
			return(0);
		}
		if (prog_status != ENTRY &&
			strcmp(local_rec.s_area_code, local_rec.e_area_code) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strlen (clip (comm_rec.tco_no)))
			strcpy (exaf_rec.co_no, comm_rec.tco_no);
		else
			strcpy (exaf_rec.co_no, comm_rec.tco_no);

	    strcpy(exaf_rec.code,local_rec.s_area_code);
		cc = find_rec(exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess108));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.area_name[0],exaf_rec.desc);
		DSP_FLD( "s_area_name" );
		return(0);
	}

	/*----------------------+
	|  Validate Area Code 2 |
	------------------------*/
	if ( LCHECK( "e_area_code") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.area_name[1],"%-40.40s", ML("End   Area Code"));
			DSP_FLD("e_area_name");
			return(0);
		}
		if (SRCH_KEY)
		{
			SrchExaf(temp_str);
			return(0);
		}
		if (strcmp(local_rec.s_area_code,local_rec.e_area_code) > 0)
		{
			print_mess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strlen (clip (comm_rec.tco_no)))
			strcpy (exaf_rec.co_no, comm_rec.tco_no);
		else
			strcpy (exaf_rec.co_no, comm_rec.tco_no);

		strcpy(exaf_rec.code,local_rec.e_area_code);
		cc = find_rec(exaf,&exaf_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess108));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.area_name[1],exaf_rec.desc);
		DSP_FLD( "e_area_name" );
		return(0);
	}

	/*------------------+
	| Validate Customer |
	+-------------------*/
	if ( LCHECK( "s_dbt_no") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.dbt_name[0],"%-40.40s", ML("Start Customer"));
			DSP_FLD("s_dbt_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			char	temp [3];
			char	tempBranch [3];

			if (strlen (clip(comm_rec.tco_no)))
				strcpy  (temp, comm_rec.tco_no);

			if (strcmp (local_rec.br_no, "~~"))
				strcpy(tempBranch, local_rec.br_no);
			else
				strcpy(tempBranch, comm_rec.test_no);

			CumrSearch (comm_rec.tco_no, tempBranch, temp_str);

			if (strlen (clip(comm_rec.tco_no)))
				strcpy  (comm_rec.tco_no, temp);
			return(0);
		}
		if (prog_status != ENTRY &&
		    strcmp(local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strcmp (comm_rec.tco_no, "  "))
			strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		else
			strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		if (strcmp (local_rec.br_no, "~~"))
			strcpy(cumr_rec.cm_est_no, local_rec.br_no);
		else
			strcpy(cumr_rec.cm_est_no, comm_rec.test_no);

		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.s_dbt_no));
		cc = find_rec(cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.dbt_name[0],"%-40.40s",cumr_rec.cm_name);
		DSP_FLD( "s_dbt_name" );
		return(0);
	}

	if ( LCHECK("e_dbt_no") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.dbt_name[1],"%-40.40s", ML("End   Customer"));
			DSP_FLD("e_dbt_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			char	temp [3];
			char	tempBranch [3];

			if (strlen (clip(comm_rec.tco_no)))
				strcpy  (temp, comm_rec.tco_no);

			if (strcmp (local_rec.br_no, "~~"))
				strcpy(tempBranch, local_rec.br_no);
			else
				strcpy(tempBranch, comm_rec.test_no);

			CumrSearch (comm_rec.tco_no, tempBranch, temp_str);

			if (strlen (clip(comm_rec.tco_no)))
				strcpy  (comm_rec.tco_no, temp);
			return(0);
		}

		if (strcmp(comm_rec.tco_no, "  "))
			strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
		else
			strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);

		if (strcmp(local_rec.br_no,"~~"))
			strcpy(cumr_rec.cm_est_no, local_rec.br_no);
		else
			strcpy(cumr_rec.cm_est_no, comm_rec.test_no);
		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.e_dbt_no));
		cc = find_rec(cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(1);
			clear_mess();
			return(1);
		}
		if (strcmp(local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			errmess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}
		sprintf(local_rec.dbt_name[1],cumr_rec.cm_name);
		DSP_FLD("e_dbt_name");
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

/*========================
| Search on Sales group. |
========================*/
void
SrchSasg (
 char*              key_val)
{
	work_open();
	save_rec("#Code","#Sales Group Description");
	strcpy(sasg_rec.co_no,comm_rec.tco_no);
	sprintf(sasg_rec.sell_grp,"%-2.2s",key_val);
	cc = find_rec("sasg",&sasg_rec,GTEQ,"r");

	while (!cc && !strcmp(sasg_rec.co_no,comm_rec.tco_no) &&
		      !strncmp(sasg_rec.sell_grp,key_val,strlen(key_val)))
	{
		cc = save_rec(sasg_rec.sell_grp,sasg_rec.desc);
		if (cc)
			break;

		cc = find_rec("sasg",&sasg_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(sasg_rec.co_no,comm_rec.tco_no);
	sprintf(sasg_rec.sell_grp,"%-2.2s",temp_str);
	cc = find_rec("sasg",&sasg_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "sasg", "DBFIND");
} 

void
SrchExaf (
 char*              key_val)
{
	work_open();
	save_rec("#Cd","#Area Description");
	if (strlen (clip (comm_rec.tco_no)))
		strcpy(exaf_rec.co_no,comm_rec.tco_no);
	else
		strcpy(exaf_rec.co_no, comm_rec.tco_no);
	sprintf(exaf_rec.code,"%-2.2s",key_val);
	cc = find_rec("exaf",&exaf_rec,GTEQ,"r");

	while (	!cc && 
			(strlen (clip (comm_rec.tco_no)) ?
				!strcmp(exaf_rec.co_no, comm_rec.tco_no) :
				!strcmp(exaf_rec.co_no, comm_rec.tco_no)) &&
		      !strncmp(exaf_rec.code,key_val,strlen(key_val)))
	{
			cc = save_rec(exaf_rec.code,exaf_rec.desc);
			if (cc)
				break;
		cc = find_rec("exaf",&exaf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	if (strlen (clip (comm_rec.tco_no)))
		strcpy(exaf_rec.co_no,comm_rec.tco_no);
	else
		strcpy(exaf_rec.co_no, comm_rec.tco_no);
	sprintf(exaf_rec.code,"%-2.2s",temp_str);
	cc = find_rec("exaf",&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "exaf", "DBFIND");
}

void
SrchExsf (
 char*              key_val)
{
	work_open();
	save_rec("#Salesman","#Salesman's Name");
	if (strlen (clip (comm_rec.tco_no)))
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	else
		strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
	sprintf(exsf_rec.sf_sal_no,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");

	while 	(!cc && 
			(strlen (clip (comm_rec.tco_no)) ?
				!strcmp(exsf_rec.sf_co_no, comm_rec.tco_no) :
				!strcmp(exsf_rec.sf_co_no, comm_rec.tco_no)) &&
			 !strncmp(exsf_rec.sf_sal_no,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_sal_no,exsf_rec.sf_sal_man);
		if (cc)
			break;

		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	if (strlen (clip (comm_rec.tco_no)))
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	else
		strcpy(exsf_rec.sf_co_no, comm_rec.tco_no);
	sprintf(exsf_rec.sf_sal_no,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, "exsf", "DBFIND");
}

int
run_prog (
 char*              prog_name)
{
	sprintf (local_rec.lp_str,"%d",local_rec.lpno);
	
	CloseDB (); FinishProgram ();;
	rset_tty ();
	clear ();
	print_at (0,0,ML(mlStdMess035));
	fflush (stdout);

	if (local_rec.onite[0] == 'Y')
	{
		if (fork() == 0)
			execlp ("ONIGHT",
                    "ONIGHT",
                    prog_name,
                    local_rec.st_sell_grp,
                    local_rec.end_sell_grp,
                    local_rec.s_area_code,
                    local_rec.e_area_code,
                    local_rec.s_dbt_no,
                    local_rec.e_dbt_no,
                    local_rec.br_no,
                    local_rec.co_no,
                    local_rec.lp_str,
                    "Print Asset Deployment Report by Salesman Code  ",(char *)0);
        return (EXIT_FAILURE);
	}
    else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp (prog_name,
                    prog_name,
                    local_rec.st_sell_grp,
                    local_rec.end_sell_grp,
                    local_rec.s_area_code,
                    local_rec.e_area_code,
                    local_rec.s_dbt_no,
                    local_rec.e_dbt_no,
                    local_rec.br_no,
                    local_rec.co_no,
                    local_rec.lp_str,(char *)0);
        return (EXIT_FAILURE);
	}
	else 
	{
		execlp (prog_name,
                prog_name,
                local_rec.st_sell_grp,
                local_rec.end_sell_grp,
                local_rec.s_area_code,
                local_rec.e_area_code,
                local_rec.s_dbt_no,
                local_rec.e_dbt_no,
                local_rec.br_no,
                local_rec.co_no,
                local_rec.lp_str,(char *)0);
	}
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (void)
{
	if ((fout = popen("pformat","w")) == (FILE *) NULL)
		file_err(errno, "pformat", "POPEN" );

	fprintf(fout,".START%s <%s>\n",DateToString (TodaysDate()),PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);

	fprintf(fout,".5\n");
	fprintf(fout,".PI16\n");
	fprintf(fout,".L180\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".EASSET DEPLOYMENT REPORT\n");
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf(fout,".R======================================");
	fprintf(fout,"===========================================");
	fprintf (fout, "======================");
	fprintf(fout,"==============================");
	fprintf(fout,"==============================\n");
}

void
process_file (void)
{
	int last_line_no;	
	struct tag_asmvRecord last_record;

	fsort = sort_open("r_dep_asset");
	memset (&asmr_rec,0,sizeof asmr_rec);

	if (!strcmp (local_rec.co_no, "~~"))
   		strcpy (asmr_rec.co_no, "  ");
	else
   		strcpy (asmr_rec.co_no, local_rec.co_no);

	if (!strcmp (local_rec.br_no, "~~"))
   		strcpy (asmr_rec.br_no, "  ");
	else
   		strcpy (asmr_rec.br_no, local_rec.br_no);

	cc = find_rec(asmr,&asmr_rec,GTEQ,"r");
    while (	!cc &&  
			strcmp (asmr_rec.co_no, local_rec.co_no) <= 0 &&
			strcmp (asmr_rec.br_no, local_rec.br_no) <= 0)
    {  	
		last_line_no = -1;
		memset (&asmv_rec,0,sizeof asmv_rec);
		asmv_rec.hhas_hash = asmr_rec.hhas_hash;
		asmv_rec.line_no = 0;
		cc = find_rec (asmv,&asmv_rec,GTEQ,"r");
		if (!cc)
			last_record = asmv_rec;	

		while (!cc && (asmv_rec.hhas_hash == asmr_rec.hhas_hash))
		{
			if (asmv_rec.to_hhcu > 0)
				last_line_no = asmv_rec.line_no;
			else
				last_line_no = -1;
			cc = find_rec (asmv,&asmv_rec,NEXT,"r");
			if (!cc && asmv_rec.hhas_hash == asmr_rec.hhas_hash)
				last_record = asmv_rec;
		}

		if (last_line_no > -1)
	    {	
			asmv_rec = last_record;
			memset(&cumr_rec, 0, sizeof cumr_rec);
			cumr_rec.cm_hhcu_hash = asmv_rec.to_hhcu;
			cc = find_rec(cumr2,&cumr_rec,COMPARISON,"r");
			if (!cc &&
				strcmp(cumr_rec.cm_dbt_no,local_rec.s_dbt_no) >=0 &&
				strcmp(cumr_rec.cm_dbt_no,local_rec.e_dbt_no) <= 0)
			{
					sprintf(exsf_rec.sf_co_no, "%-2.2s", asmr_rec.co_no);
					strcpy (exsf_rec.sf_sal_no, cumr_rec.cm_sman_code);
					cc = find_rec(exsf,&exsf_rec, COMPARISON,"r");

				if (!cc 
					&& asmr_rec.hhsu_hash > 0L
					&& strcmp(exsf_rec.sf_area_code,local_rec.s_area_code) >= 0
					&& strcmp(exsf_rec.sf_area_code,local_rec.e_area_code) <=0)
				{
					strcpy (exaf_rec.co_no, exsf_rec.sf_co_no);
					strcpy (exaf_rec.code, exsf_rec.sf_area_code);
					cc = find_rec (exaf, &exaf_rec, EQUAL, "r");
	
					if (!cc)
					{
						strcpy (sasg_rec.co_no, asmr_rec.co_no);
						strcpy (sasg_rec.sell_grp, local_rec.st_sell_grp);
						cc = find_rec (sasg, &sasg_rec, GTEQ, "r");
						while (!cc && !strcmp (sasg_rec.co_no, asmr_rec.co_no)
					   			   &&  strcmp (sasg_rec.sell_grp, local_rec.end_sell_grp) <= 0)  
						{
				
							if (!strcmp (sasg_rec.sell_grp, exsf_rec.sell_grp))
							{
								store_data ();
								break;
							}
							cc = find_rec (sasg, &sasg_rec, NEXT, "r");
						}
					}
				}
			}
		 }
		cc = find_rec(asmr,&asmr_rec,NEXT,"r");
	}

	display_report ();
	fflush (fout);
	sort_delete (fsort, "r_dep_asset");
}

void
store_data (void)
{
	char data_string [100];

    sprintf(data_string,"%-2.2s %-2.2s %-2.2s %-2.2s %11ld %-5.5s %-5.5s %-25.25s %-2.2s\n",
    asmr_rec.co_no,
	asmr_rec.br_no,
	sasg_rec.sell_grp,
	exsf_rec.sf_area_code,
	cumr_rec.cm_hhcu_hash,
	asmr_rec.ass_group,
	asmr_rec.ass_no,
    asmr_rec.serial_no,
	exsf_rec.sf_sal_no);

	sort_save (fsort,data_string);
}


void
display_report (void)
{
	char 	*sptr;
	char 	prev_salesman [3],
		 	prev_area     [3]; 
	long	hhcu_hash,
			prev_hhcu_hash;
    char 	salesman_no   [3], 
		 	salesman_name [41],
		 	area_code     [3],
			sell_grp	  [3],
			prev_sell	  [3],
		 	area_name	  [41],
	     	outlet_name   [41],	
	 	 	serial_no     [26],
		 	brand_code    [9],
		 	status_code   [3],
		 	ass_group  	  [6],
		 	ass_number 	  [6],
			prev_co	  	  [3], 	
			prev_branchNo	  [3];
			
	int 	first_time = TRUE;
	int		first      = TRUE;	

	
	fsort = sort_sort(fsort, "r_dep_asset");
	sptr =  sort_read(fsort);
	strcpy (prev_salesman, "");
	strcpy (prev_area, "");
	prev_hhcu_hash = 0L;
	strcpy (prev_branchNo ,"");
	strcpy (prev_co    ,"");
	strcpy (prev_sell, "");

	while (sptr != (char *)0)
	{
		sprintf (curr_co, 		"%-2.2s", sptr);
	  	sprintf (curr_branchNo,  	"%-2.2s",  sptr + 3);
		sprintf (sell_grp, 		"%-2.2s", sptr + 6);
		sprintf (area_code  , 	"%-2.2s",  sptr + 9);
		hhcu_hash  = atol (sptr + 12);
		sprintf (ass_group  , 	"%-5.5s",	sptr + 24); 
		sprintf (ass_number , 	"%-5.5s", sptr + 30); 
		sprintf (serial_no  , 	"%-25.25s",sptr + 36); 
		sprintf (salesman_no, "	%-2.2s", sptr + 62);
	
		/* Look for Area Name */
		strcpy(exaf_rec.co_no, curr_co);
		sprintf(exaf_rec.code,"%-2.2s", area_code);
		cc = find_rec(exaf,&exaf_rec,COMPARISON,"r");
		if (!cc)
			sprintf (area_name,"%-40.40s", exaf_rec.desc);
		else
			sprintf (area_name, "%-40.40s", " ");

		strcpy (asmr_rec.co_no,     curr_co);
		strcpy (asmr_rec.br_no,		curr_branchNo);
		strcpy (asmr_rec.ass_group,	ass_group);
		strcpy (asmr_rec.ass_no,	ass_number);
		strcpy (asmr_rec.serial_no, serial_no);
		cc = find_rec (asmr, &asmr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (brand_code,  "%-8.8s",asmr_rec.brand);
			sprintf (status_code, "%-2.2s",asmr_rec.status_code);
		}
		else
		{
			sptr = sort_read (fsort);
			continue;
		}

		memset  (&cumr_rec,0,sizeof cumr_rec);
		cumr_rec.cm_hhcu_hash = hhcu_hash;
		cc = find_rec(cumr2,&cumr_rec,COMPARISON,"r");
		if (!cc)
		{
			sprintf (outlet_name,"%-40.40s", cumr_rec.cm_name);
			strcpy(exsf_rec.sf_co_no, curr_co);
			strcpy(exsf_rec.sf_sal_no,salesman_no);
			cc = find_rec(exsf,&exsf_rec,COMPARISON,"r");
			if (!cc)
				sprintf (salesman_name,"%-40.40s", exsf_rec.sf_sal_man);
			else
				sprintf (salesman_name,"%-40.40s", " ");
		}
		else
		{
			sptr = sort_read (fsort);
			continue;
		}
	
		strcpy  (esmr_rec.co_no,  curr_co);
		sprintf (esmr_rec.br_no,  "%-2.2s", curr_branchNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}
		
		strcpy (sasg_rec.co_no, curr_co);
		strcpy (sasg_rec.sell_grp, sell_grp);
		cc = find_rec (sasg, &sasg_rec, EQUAL, "r");
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}

		/*============================================
		|  First time it will print the header       |
		|  change header if branch changes           |
		============================================*/

		if (strcmp (sell_grp, prev_sell) || strcmp (curr_co, prev_co) ||
			strcmp (curr_branchNo,prev_branchNo)) 
		{
			if (!first_time)
			{
				if (strcmp (curr_co, prev_co) ) 
				{
					DefinedSection();
					fprintf (fout, ".PA\n");
				}
				else
					print_line();
			}
			else 
			{
				DefinedSection();
				first_time = FALSE;
			}
			if (first)
			{
				fprintf (fout,"| Selling Group - %-3.3s (%-40.40s)     ",
							 sell_grp, sasg_rec.desc); 
				fprintf (fout, "Branch - %-3.3s (%40.40s)%38.38s|\n", 
						curr_branchNo, esmr_rec.br_name, " "); 
				strcpy (prev_sell, sell_grp);
				first = FALSE;
			}
			else
			if (!first)
			{
				DefinedSection2 (sell_grp, sasg_rec.desc, curr_branchNo,
								 esmr_rec.br_name);
				fprintf (fout,".PA\n");
			}
		}
		else
			first_time = TRUE;

		/*-------------+
		|  Print Data  |	
		+-------------*/
		if (!first_time) 
		{
			fprintf (fout,"|  %-2.2s  ",    area_code); 	
			fprintf (fout,"|%-40.40s", 	area_name); 	
			fprintf (fout,"| %-6.6s ",	cumr_rec.cm_dbt_no); 	
			fprintf (fout,"|%-40.40s",  outlet_name);
		}
		else if (!strcmp(prev_area,area_code)) 
		{
			fprintf (fout,"|  %-2.2s  ", " "); 	
			fprintf (fout,"|%-40.40s", " "); 	
			if (prev_hhcu_hash == hhcu_hash)
			{
				fprintf (fout,"| %-6.6s ", " "); 	
				fprintf (fout,"|%-40.40s",   " ");
			}
			else
			{
				fprintf (fout,"| %-6.6s ", cumr_rec.cm_dbt_no); 	
				fprintf (fout,"|%-40.40s",   outlet_name);
			}
		}
		else
		{
			fprintf (fout,"|  %-2.2s  ",     area_code); 	
			fprintf (fout,"|%-40.40s",   area_name); 	
			fprintf (fout,"| %-6.6s ",   cumr_rec.cm_dbt_no); 	
			fprintf (fout,"|%-40.40s",   outlet_name);
		}
		fprintf (fout,"|%-20.20s",  serial_no); 	
		fprintf (fout,"| %-8.8s",   brand_code); 	
		fprintf (fout,"|%-8.8s", 	asmr_rec.type);
		fprintf (fout,"| %-10.10s", asmr_rec.spec2_code); 
		fprintf (fout," |    %-2.2s    |\n",status_code); 	

		sprintf (prev_salesman, "%-2.2s",salesman_no);
		sprintf (prev_area,     "%-2.2s",area_code);
		prev_hhcu_hash = hhcu_hash;
		strcpy (prev_branchNo, curr_branchNo);
		strcpy (prev_co,    curr_co);

		sptr = sort_read (fsort);
	}
}


void
DefinedSection (void)
{

	fprintf (fout, ".DS7\n");
	
	strcpy (comr_rec.co_no, curr_co);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
    if (!cc)
		fprintf(fout,".ECompany : %s - %s\n",
				curr_co, clip(comr_rec.co_name));
	else
		fprintf(fout,".ECompany :    - Unknown\n");


	fprintf(fout,".B1\n");

	if (!strcmp (local_rec.st_sell_grp, "  "))
		sprintf (local_rec.st_sasg_desc, "%-40.40s", "START");
	else
	{	
		strcpy( sasg_rec.co_no, curr_co);
		strcpy( sasg_rec.sell_grp, local_rec.st_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		sprintf (local_rec.st_sasg_desc, "%-40.40s", sasg_rec.desc);
	}	

	if (!strcmp (local_rec.end_sell_grp, "~~"))
		sprintf (local_rec.end_sasg_desc, "%-40.40s", "END");
	else
	{
		strcpy( sasg_rec.co_no, curr_co);
		strcpy( sasg_rec.sell_grp, local_rec.end_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		sprintf (local_rec.end_sasg_desc, "%-40.40s", sasg_rec.desc);
	}

	fprintf(fout,"Start Selling Group: ");
	fprintf(fout,"%-15.15s",local_rec.st_sasg_desc);
	fprintf(fout," End Selling Group: ");
	fprintf(fout,"%-15.15s",local_rec.end_sasg_desc);

	fprintf(fout," / ");	
	fprintf(fout,"Start Area: ");
	if (!strlen (clip(local_rec.s_area_code)))
		fprintf(fout,"START");
	else
		fprintf(fout,"%-2.2s",local_rec.s_area_code);
	fprintf(fout," End Area: ");
	if (!strcmp(local_rec.e_area_code,"~~"))
		fprintf(fout,"END   ");
	else
		fprintf(fout,"%-2.2s",local_rec.e_area_code);

	fprintf(fout,"Start Customer: ");
	if (!strlen (clip(local_rec.s_dbt_no)))
		fprintf(fout,"START ");
	else
		fprintf(fout,"%-6.6s",local_rec.s_dbt_no);
	fprintf(fout," End Customer: ");
	if (!strcmp(local_rec.e_dbt_no,"~~~~~~"))
		fprintf(fout,"END\n");
	else
		fprintf(fout,"%-6.6s\n",local_rec.e_dbt_no);


	fprintf(fout,"=========");
	fprintf(fout,"===========");
	fprintf(fout,"===================");
	fprintf(fout,"===========================================");
	fprintf(fout,"==============================");
	fprintf (fout, "======================");
	fprintf(fout,"=============================\n");

	fprintf(fout,"| AREA ");
	fprintf(fout,"|           AREA   NAME                  ");
	fprintf(fout,"|CUSTOMER");
	fprintf(fout,"|           OUTLET  NAME                 "); 
	fprintf(fout,"|      SERIAL NO.    ");
	fprintf(fout,"|  BRAND  ");
	fprintf(fout,"|  TYPE  ");
	fprintf(fout,"|  MODEL NO. ");
	fprintf(fout,"|  STATUS  |\n");


	fprintf(fout,"| CODE ");
	fprintf(fout,"|%-40.40s", " ");
	fprintf(fout,"|  CODE  ");
	fprintf(fout,"|%-40.40s"," ");
	fprintf(fout,"|%-20.20s"," ");
	fprintf(fout,"|%-9.9s"," ");
	fprintf(fout,"|%-8.8s"," ");
	fprintf(fout,"|%-11.11s"," ");
	fprintf(fout," | %-8.8s |\n"," ");
	print_line();
	fflush(fout);
}
	
void
DefinedSection2 (
 char*              sell_grp,
 char*              desc,
 char*              curr_branchNo,
 char*              br_name) 
{

	fprintf (fout, ".DS8\n");
	
	strcpy (comr_rec.co_no, curr_co);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
    if (!cc)
		fprintf(fout,".ECompany : %s - %s\n",
				curr_co, clip(comr_rec.co_name));
	else
		fprintf(fout,".ECompany :    - Unknown\n");


	fprintf(fout,".B1\n");

	if (!strcmp (local_rec.st_sell_grp, "  "))
		sprintf (local_rec.st_sasg_desc, "%-40.40s", "START");
	else
	{	
		strcpy( sasg_rec.co_no, curr_co);
		strcpy( sasg_rec.sell_grp, local_rec.st_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		sprintf (local_rec.st_sasg_desc, "%-40.40s", sasg_rec.desc);
	}	

	if (!strcmp (local_rec.end_sell_grp, "~~"))
		sprintf (local_rec.end_sasg_desc, "%-40.40s", "END");
	else
	{
		strcpy( sasg_rec.co_no, curr_co);
		strcpy( sasg_rec.sell_grp, local_rec.end_sell_grp );
		cc = find_rec("sasg", &sasg_rec, COMPARISON, "r");
		sprintf (local_rec.end_sasg_desc, "%-40.40s", sasg_rec.desc);
	}

	fprintf(fout,"Start Selling Group: ");
	fprintf(fout,"%-15.15s",local_rec.st_sasg_desc);
	fprintf(fout," End Selling Group: ");
	fprintf(fout,"%-15.15s",local_rec.end_sasg_desc);

	fprintf(fout," / ");	
	fprintf(fout,"Start Area: ");
	if (!strlen (clip(local_rec.s_area_code)))
		fprintf(fout,"START");
	else
		fprintf(fout,"%-2.2s",local_rec.s_area_code);
	fprintf(fout," End Area: ");
	if (!strcmp(local_rec.e_area_code,"~~"))
		fprintf(fout,"END   ");
	else
		fprintf(fout,"%-2.2s",local_rec.e_area_code);

	fprintf(fout,"Start Customer: ");
	if (!strlen (clip(local_rec.s_dbt_no)))
		fprintf(fout,"START ");
	else
		fprintf(fout,"%-6.6s",local_rec.s_dbt_no);
	fprintf(fout," End Customer: ");
	if (!strcmp(local_rec.e_dbt_no,"~~~~~~"))
		fprintf(fout,"END\n");
	else
		fprintf(fout,"%-6.6s\n",local_rec.e_dbt_no);


	fprintf(fout,"=========");
	fprintf(fout,"===========");
	fprintf(fout,"===================");
	fprintf(fout,"===========================================");
	fprintf(fout,"==============================");
	fprintf (fout, "======================");
	fprintf(fout,"=============================\n");

	fprintf(fout,"| AREA ");
	fprintf(fout,"|           AREA   NAME                  ");
	fprintf(fout,"|CUSTOMER");
	fprintf(fout,"|           OUTLET  NAME                 "); 
	fprintf(fout,"|      SERIAL NO.    ");
	fprintf(fout,"|  BRAND  ");
	fprintf(fout,"|  TYPE  ");
	fprintf(fout,"|  MODEL NO. ");
	fprintf(fout,"|  STATUS  |\n");


	fprintf(fout,"| CODE ");
	fprintf(fout,"|%-40.40s", " ");
	fprintf(fout,"|  CODE  ");
	fprintf(fout,"|%-40.40s"," ");
	fprintf(fout,"|%-20.20s"," ");
	fprintf(fout,"|%-9.9s"," ");
	fprintf(fout,"|%-8.8s"," ");
	fprintf(fout,"|%-11.11s"," ");
	fprintf(fout," | %-8.8s |\n"," ");
	print_line();
	fprintf (fout,"| Selling Group - %-3.3s (%-40.40s)     ",
				 sell_grp, desc); 
	fprintf (fout, "Branch - %-3.3s (%40.40s)%38.38s|\n", curr_branchNo, 
				br_name, " ");
	fflush(fout);
}

void
print_line (void)
{	
	fprintf (fout,"|------|----------------------------------------|");
	fprintf (fout,"--------|----------------------------------------|");
	fprintf (fout,"--------------------|---------|--------|");
	fprintf (fout, "------------|----------|\n");
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
	rv_pr(ML(" Asset Deployment Report "),25,0,1);
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
	move(0,20);
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
	strcpy  (esmr_rec.co_no,  comm_rec.tco_no);
	sprintf (esmr_rec.br_no,  "%-2.2s", key_val);
	save_rec ("#Br", "#Branch Description");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc 
	&&     (strlen (comm_rec.tco_no) ?
				!strcmp  (esmr_rec.co_no, comm_rec.tco_no) :
				!strcmp  (esmr_rec.co_no, comm_rec.tco_no))
	&&     !strncmp (esmr_rec.br_no, key_val, strlen (key_val)) )
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

