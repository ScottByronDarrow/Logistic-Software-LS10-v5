/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (as_yield_prn.c)                                   |
|  Program Desc  : (Print Asset Yield Report by Area Code  )          |	
|                  (                                       )          |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, asmv, asmr, assc, asbr, exaf          |
|  Database      : (Data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Ross Baquillos. | Date Written  : 02/14/95         |
|---------------------------------------------------------------------|
|  Date Modified : (28/07/95)      |   Ross S Baquillos.              |
|                : (03/13/96)      |   Alan G. Rivera                 |
|                : (06/02/98)      |   Elena Cuaresma                 |
|                : (10/07/98)      |   Elena Cuaresma                 |
|                :                                                    |
|  Comments      :													  | 
|  (28/07/95)    : SMF 00130 - Updated to fix bug on every first page.|
|  (03/13/96)    : SMR 00075 - Removed field cumr_mkt_rep             |
|  (06/02/98)    : 9.10  New asset module for the standard version 9. |
|  (10/07/98)    : 9.10  Removed sams.                                |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: yield_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_yield_prn/yield_prn.c,v 5.3 2002/07/17 09:56:55 scott Exp $";

#define NOT_ZERO(x)		 (fabs (x) > 0.00001)

#include <pslscr.h>
#include <ml_std_mess.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <DateToString.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;

	struct {
		int	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		long	tdbt_date;
	} comm_rec;

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
		{"esmr_short_name"},
	};

	struct tag_esmrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	short_name [41];
	}	esmr_rec;

	/*=============================================
	| file exsf {External Salesman file Record} . |
	=============================================*/
	struct dbview exsf_list [] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_area_code"}
	};

	int exsf_no_fields = 4;

	struct {
		char 	sf_co_no [3];
		char 	sf_sal_no [3];
		char 	sf_sal_man [41];
		char 	sf_area_code [3];
	} exsf_rec;


	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	10

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_class_type"},
		{"cumr_area_code"},
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
		char	class_type [4];
		char	area_code [3];
		char	sman_code [3];
	}	cumr_rec;

	/*======+
	 | sadf |
	 +======*/
#define	SADF_NO_FIELDS	19

	struct dbview	sadf_list [SADF_NO_FIELDS] =
	{
		{"sadf_co_no"},
		{"sadf_br_no"},
		{"sadf_year"},
		{"sadf_hhbr_hash"},
		{"sadf_hhcu_hash"},
		{"sadf_qty_per1"},
		{"sadf_qty_per2"},
		{"sadf_qty_per3"},
		{"sadf_qty_per4"},
		{"sadf_qty_per5"},
		{"sadf_qty_per6"},
		{"sadf_qty_per7"},
		{"sadf_qty_per8"},
		{"sadf_qty_per9"},
		{"sadf_qty_per10"},
		{"sadf_qty_per11"},
		{"sadf_qty_per12"},
		{"sadf_sman"},
		{"sadf_area"}
	};

	struct tag_sadfRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	year [2];
		long	hhbr_hash;
		long	hhcu_hash;
		float	qty_per [12];
		char	sman [3];
		char	area [3];
	}	sadf_rec;

	/*======+
	 | inmr |
	 +======*/
#define	INMR_NO_FIELDS	2

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_hhbr_hash"},
		{"inmr_weight"},
	};

	struct tag_inmrRecord
	{
		long	hhbr_hash;
		float	weight;
	}	inmr_rec;

	/*======+
	 | asmv |
	 +======*/
#define	ASMV_NO_FIELDS	6

	struct dbview	asmv_list [ASMV_NO_FIELDS] =
	{
		{"asmv_hhar_hash"},
		{"asmv_line_no"},
		{"asmv_hham_hash"},
		{"asmv_serial_no"},
		{"asmv_to_hhcu"},
		{"asmv_vol_commit"}
	};

	struct tag_asmvRecord
	{
		long	hhar_hash;
		int		line_no;
		long	hham_hash;
		char	serial_no [26];
		long	to_hhcu;
		long	vol_commit;
	}	asmv_rec;



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
		char	ass_group [6];
		char	ass_no [6];
		char	serial_no [26];
		char	type [4];
		char	brand [9];
		Date	pur_date;
		long	hhar_hash;
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


	float	weight	= 0.00;

	FILE *fout,
		 *fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	co_no [3];
	char	co_name [41];
	char	s_dbt_no [7];
	char	e_dbt_no [7];
    char    dbt_name [2] [41];
	int		lpno;
	char	lp_str [3];
	char 	back [5];
	char	onite [5];
} local_rec;

char	*asmr	=	"asmr",
		 *asmv	=	"asmv",	
		 *data	=	"data",
		 *exaf	=	"exaf",
		 *exsf	=	"exsf",
		 *cumr2	=	"cumr2",	
		 *cumr	=	"cumr",
		 *comr	=	"comr",
		 *cuot	=	"cuot",
		 *asbr	=	"asbr",
		 *assc	=	"assc",
		 *sadf	=	"sadf",
		 *inmr	=	"inmr",
		 *esmr	=	"esmr";

static	struct	var	vars [] =
{
	{1, LIN, "co_no", 4, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "Company Number ", " ",
		YES, NO,  JUSTRIGHT, "", "", local_rec.co_no},
	{1, LIN, "co_name",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.co_name},
	{1, LIN, "s_dbt_no", 6, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Customer ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.s_dbt_no},
	{1, LIN, "s_dbt_name",	 6, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dbt_name [0]},
	{1, LIN, "e_dbt_no", 7, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End   Customer ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.e_dbt_no},
	{1, LIN, "e_dbt_name",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dbt_name [1]},
	{1, LIN, "lpno",	 9, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 10, 18, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 10, 60, CHARTYPE,
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
| Local Function Prototypes.
=====================================================================*/
int		spec_valid 		 (int);
int 	RunProgram 		 (char *);
void	shutdown_prog 	 (void);
void	OpenDB 			 (void);
void	CloseDB 		 (void);
void 	SrchComr 		 (char *);
void 	SrchExaf 		 (char *);
void 	SrchExsf 		 (char *);
void 	HeadingOutput 	 (void);
void 	Defined_Section (void);
void 	ProcessFile 	 (void);
void 	StoreData 		 (void);
void 	DislayReport 	 (void);
void 	PrintSpaces 	 (void);
int 	heading 		 (int);


	char	company [3];
	int		envVarDbCo		=	0;
	char	branchNumber [3];

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	if (argc != 1 && argc != 6)
	{
		print_at (0,0, ML ("Usage : %s <Company> <Company Name>"), argv [0]);
		print_at (1,0, ML ("           <Start Customer> <End Customer>"));
		print_at (2,0, ML ("           <lpno>"));
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	if (argc == 6)
	{
		sprintf (local_rec.co_no,       "%-2.2s",argv [1]);
		sprintf (local_rec.co_name,     "%-40.40s",argv [2]);
		sprintf (local_rec.s_dbt_no   ,"%-6.6s",argv [3]);
		sprintf (local_rec.e_dbt_no   ,"%-6.6s",argv [4]);
		local_rec.lpno = atoi (argv [5]);

		dsp_screen ("Processing : Asset Yield Report.",
					local_rec.co_no,local_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		HeadingOutput ();
		ProcessFile ();
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();				/*  sets terminal from termcap	*/
	set_tty ();              /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/
	init_vars (1);			/*  set default values			*/

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

		if (RunProgram (argv [0]) == 1)
        {
            return (EXIT_SUCCESS);
		    prog_exit = 1;
        }
	}
	shutdown_prog ();
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
	abc_dbopen (data);
	abc_alias (cumr2, cumr);
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS,"cumr_hhcu_hash");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (asmr, asmr_list, ASMR_NO_FIELDS, "asmr_id_no");
	open_rec (asmv, asmv_list, ASMV_NO_FIELDS, "asmv_id_no"); 
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (exsf, exsf_list, exsf_no_fields, "exsf_id_no");
	open_rec (exaf, exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec (assc, assc_list, ASSC_NO_FIELDS, "assc_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (sadf, sadf_list, SADF_NO_FIELDS, "sadf_id_no2");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (asmr);
    abc_fclose (asmv);
	abc_fclose (cumr2);	
	abc_fclose (cumr);
	abc_fclose (comr);
	abc_fclose (exsf);
	abc_fclose (exaf);	
	abc_fclose (assc);	
	abc_fclose (esmr);	
	abc_fclose (sadf);	
	abc_fclose (inmr);	
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*------------------------------------------
	| Validate Company Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("co_no"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.co_no, "  ");
			sprintf (local_rec.co_name, "%s", ML ("All Company"));
			DSP_FLD ("co_no");
			DSP_FLD ("co_name");
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
			errmess (ML (mlStdMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.co_name, comr_rec.co_name);
		DSP_FLD ("co_no");
		DSP_FLD ("co_name");
	}

	/*------------------+
	| Validate Customer |
	+-------------------*/
	if (LCHECK ("s_dbt_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.dbt_name [0],"%-40.40s",ML ("Start Customer"));
			DSP_FLD ("s_dbt_name");
			return (EXIT_SUCCESS);
		}

		if (last_char == SEARCH)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY &&
		    strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no,branchNumber);
		strcpy (cumr_rec.cm_dbt_no,pad_num (local_rec.s_dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.dbt_name [0],"%-40.40s",cumr_rec.cm_name);
		DSP_FLD ("s_dbt_name");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("e_dbt_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.dbt_name [1],"%-40.40s", ML ("End   Customer"));
			DSP_FLD ("e_dbt_name");
			return (EXIT_SUCCESS);
		}

		if (last_char == SEARCH)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.cm_co_no,comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no,branchNumber);
		strcpy (cumr_rec.cm_dbt_no,pad_num (local_rec.e_dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.dbt_name [1],cumr_rec.cm_name);
		DSP_FLD ("e_dbt_name");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("lpno"))
	{
		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
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
	&&     !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
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

void
SrchExaf (
 char*              key_val)
{
	work_open ();
	save_rec ("#Code","#Area Description");
	strcpy (exaf_rec.co_no,comm_rec.tco_no);
	sprintf (exaf_rec.code,"%-2.2s",key_val);
	cc = find_rec ("exaf",&exaf_rec,GTEQ,"r");

	while (!cc && !strcmp (exaf_rec.co_no,comm_rec.tco_no) &&
		      !strncmp (exaf_rec.code,key_val,strlen (key_val)))
	{
			cc = save_rec (exaf_rec.code,exaf_rec.desc);
			if (cc)
				break;
		cc = find_rec ("exaf",&exaf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exaf_rec.co_no,comm_rec.tco_no);
	sprintf (exaf_rec.code,"%-2.2s",temp_str);
	cc = find_rec ("exaf",&exaf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exaf", "DBFIND");
}

void
SrchExsf (
 char*              key_val)
{
	work_open ();
	save_rec ("#Code","#Salesman's Name");
	strcpy (exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf (exsf_rec.sf_sal_no,"%-2.2s",key_val);
	cc = find_rec ("exsf",&exsf_rec,GTEQ,"r");

	while (!cc && !strcmp (exsf_rec.sf_co_no,comm_rec.tco_no) &&
				  !strncmp (exsf_rec.sf_sal_no,key_val,strlen (key_val)))
	{
		cc = save_rec (exsf_rec.sf_sal_no,exsf_rec.sf_sal_man);
		if (cc)
			break;

		cc = find_rec ("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf (exsf_rec.sf_sal_no,"%-2.2s",temp_str);
	cc = find_rec ("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

int
RunProgram (
 char*              prog_name)
{
	sprintf (local_rec.lp_str,"%d",local_rec.lpno);
	
	clear ();
	print_at (0,0, ML (mlStdMess035));
	fflush (stdout);

	CloseDB (); 
	FinishProgram ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.co_no,
				local_rec.co_name,
				local_rec.s_dbt_no,
				local_rec.e_dbt_no,
				local_rec.lp_str,
				"Print Asset Yield Report by Customer ", (char *)0);
        return (EXIT_FAILURE);
	}
    else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.co_no,
				local_rec.co_name,
				local_rec.s_dbt_no,
				local_rec.e_dbt_no,
				local_rec.lp_str, (char *)0);
        return (EXIT_FAILURE);
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.co_no,
			local_rec.co_name,
			local_rec.s_dbt_no,
			local_rec.e_dbt_no,
			local_rec.lp_str, (char *)0);
	}
    return (EXIT_SUCCESS);
}


/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	fprintf (fout,".START%s <%s>\n",DateToString (TodaysDate ()),PNAME);
	fprintf (fout,".LP%d\n",local_rec.lpno);

	fprintf (fout,".8\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L132\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EASSET YIELD REPORT BY CUSTOMER\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout,".R  =====================================");
	fprintf (fout,"==============================================");
	fprintf (fout,"=============================================\n");

	fflush (fout);
}

void
Defined_Section (void)
{
	fprintf (fout,".DS7\n");

    strcpy (comr_rec.co_no, company);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (!cc)
		fprintf (fout,".ECompany : %s - %s\n",
					company,clip (comr_rec.co_name));
	else
		fprintf (fout,".ECompany : %s - Unknown Company\n", company);

	fprintf (fout,".B1\n");
	sprintf (temp_str,"%6.6s",local_rec.s_dbt_no);
	fprintf (fout,".CStart Customer Number : ");
	if (!strlen (clip (temp_str)))
		fprintf (fout,"START");
	else
		fprintf (fout,"%-6.6s",local_rec.s_dbt_no);

	fprintf (fout,"  End Customer Number : ");

	if (!strcmp (local_rec.e_dbt_no,"~~~~~~"))
		fprintf (fout,"END   \n");
	else
		fprintf (fout,"%-6.6s\n",local_rec.e_dbt_no);

	fprintf (fout,".C==========================================");
	fprintf (fout,"=========================================");
	fprintf (fout,"=============================================\n");

	fprintf (fout,".C|   BUSINESS    ");
	fprintf (fout,"|       SERIAL  NUMBER       ");
	fprintf (fout,"|  ASSET  ");
	fprintf (fout,"|     VOLUME      ");
	fprintf (fout,"|     VOLUME      ");
	fprintf (fout,"|    VARIANCE     ");
	fprintf (fout,"|    VARIANCE     |\n");

	fprintf (fout,".C|     UNIT      ");
	fprintf (fout,"|                            ");
	fprintf (fout,"|  TYPE   ");
	fprintf (fout,"|    COMMITTED    ");
	fprintf (fout,"|     TO-DATE     ");
	fprintf (fout,"|     VOLUME      ");
	fprintf (fout,"|    PERCENTAGE   |\n");

	fprintf (fout,".C|---------------");
	fprintf (fout,"|----------------------------");
	fprintf (fout,"|---------");
	fprintf (fout,"|-----------------");
	fprintf (fout,"|-----------------");
	fprintf (fout,"|-----------------");
	fprintf (fout,"|-----------------|\n");
	fflush (fout);
}

void
ProcessFile (void)
{
	int 	ok_entry,
			last_line_no;	
	struct 	tag_asmvRecord last_record;
	int		monthPeriod;

	fsort = sort_open ("r_asset_yld");

	memset (&asmr_rec,0,sizeof asmr_rec);
    strcpy (asmr_rec.co_no, local_rec.co_no);
    strcpy (asmr_rec.br_no, "  ");
	cc = find_rec (asmr,&asmr_rec,GTEQ,"r");
    while (!cc && 
		  (!strcmp (local_rec.co_no, "  ") ||
		   !strcmp (asmr_rec.co_no,local_rec.co_no)))
    { 	
		last_line_no = -1;
		memset (&asmv_rec,0,sizeof asmv_rec);
		asmv_rec.hhar_hash = asmr_rec.hhar_hash;
		asmv_rec.line_no = 0;
		cc = find_rec (asmv,&asmv_rec,GTEQ,"r");
		if (!cc)
			last_record = asmv_rec;	

		while ((!cc) && (asmv_rec.hhar_hash == asmr_rec.hhar_hash))
		{
			if (asmv_rec.to_hhcu > 0)
				last_line_no = asmv_rec.line_no;
			else
				last_line_no = -1;
			cc = find_rec (asmv,&asmv_rec,NEXT,"r");
			if (!cc && asmv_rec.hhar_hash == asmr_rec.hhar_hash)
				last_record = asmv_rec;
		}

		if (last_line_no > -1)
	    {	
			asmv_rec = last_record;
			DateToDMY (comm_rec.tdbt_date, NULL, &monthPeriod, NULL);

			strcpy (sadf_rec.co_no,	comm_rec.tco_no);
			strcpy (sadf_rec.br_no,	comm_rec.test_no);
			strcpy (sadf_rec.year,	"C");
			sadf_rec.hhcu_hash	=	asmv_rec.to_hhcu;
			sadf_rec.hhbr_hash	=	0L;
			sprintf (sadf_rec.sman,	"%-2.2s", "  ");
			sprintf (sadf_rec.area,	"%-2.2s", "  ");
        	cc = find_rec (sadf, &sadf_rec, GTEQ, "r");


			ok_entry = 0;

			while (!cc)
			{
 				if (sadf_rec.hhcu_hash == asmv_rec.to_hhcu)
				{
					inmr_rec.hhbr_hash	=	sadf_rec.hhbr_hash;
					cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
					if (cc)
						weight	= 0.00;
					else
						weight	= inmr_rec.weight;


					weight	*=	sadf_rec.qty_per [monthPeriod - 1];	
					
					StoreData ();
					ok_entry = 1;
				}
				cc = find_rec (sadf, &sadf_rec, NEXT, "r");
			}

			if (!ok_entry)
			{
				weight = 0.00;
				StoreData ();
			}

		}

		cc = find_rec (asmr,&asmr_rec,NEXT,"r");
	}
	DislayReport ();
	fflush (fout);
	sort_delete (fsort, "r_asset_yld");
}

void
StoreData (void)
{
	char data_string [75];

	memset (&cumr_rec, 0, sizeof cumr_rec);
	cumr_rec.cm_hhcu_hash = asmv_rec.to_hhcu;
	cc = find_rec (cumr2, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML ("No Hash Found!"));
		sleep (sleepTime);	
		clear_mess ();
		return ;
	}	

	if ((!strcmp (local_rec.s_dbt_no, "  ") ||
		  strcmp (cumr_rec.cm_dbt_no, local_rec.s_dbt_no) >= 0) &&
	    (!strcmp (local_rec.e_dbt_no, "  ") ||
          strcmp (cumr_rec.cm_dbt_no,local_rec.e_dbt_no) <= 0))
	{
		sprintf (data_string,"%-2.2s %-6.6s %-2.2s %-26.26s %11ld %15.2f%-3.3s\n",   			asmr_rec.co_no, cumr_rec.cm_dbt_no, 
	            asmr_rec.br_no, asmr_rec.serial_no, asmv_rec.vol_commit,
				weight,
				asmr_rec.type);

		sort_save (fsort,data_string);
	}
}

void
DislayReport (void)
{
	char 	*sptr;

	char    group [15],
			prev_group [15],
			prev_company [3],
			branch [3],
			prev_branch [3],
			branch_short [16],
			serial [27],
			prev_serial [27],
			astype [4],
			prev_astype [4],
			dbt_no [7],
			last_dbt_no [7],
			prev_dbt_no [7];

	int		last_page,
			first_page,
			total,
			okay_print,
			next_branch = FALSE,
	   		first_time = TRUE;
	long	cvolume,
			prev_cvolume,
			tot_cvolume,
			tot_cust_cvolume;
	float	weight,
			percent,
			tot_prcnt,
			volume_todate,
			tot_cust_vtodate;
	double  var_volume,
			tot_var_volume;
	
	fsort = sort_sort (fsort, "r_asset_yld");
	sptr =  sort_read (fsort);

	strcpy (group,"");
	strcpy (prev_astype,"");
	strcpy (prev_serial,"");
	strcpy (prev_dbt_no,"");
	strcpy (prev_branch,"");
	strcpy (prev_company,"");
	prev_cvolume 	 = 0L;
	tot_cvolume  	 = 0L;
	last_page 		 = FALSE;
	first_page       = FALSE;	
	total			 = FALSE;	
	first_page       = TRUE;
	volume_todate 	 = 0.00;
	tot_cust_vtodate = 0.00;
	tot_cust_cvolume = 0L;
	okay_print		 = TRUE;	

	while (last_page || sptr != (char *)0)
	{
		if (last_page)
		{
			sprintf (company,"%-2.2s", 	" ");
			sprintf (dbt_no, "%-6.6s", 	" ");
			sprintf (branch, "%-2.2s", 	" ");
			sprintf (serial, "%-26.26s"," ");
			cvolume = 0L;
			weight  = 0.00;
			sprintf (astype, "%-3.3s",  " ");
		}
		else
		{
			sprintf (company,"%-2.2s", 	sptr);
			sprintf (dbt_no, "%-6.6s", 	sptr + 3);
			sprintf (branch, "%-2.2s", 	sptr + 10);
			sprintf (serial, "%-26.26s",sptr + 13);
			cvolume = atol (sptr + 40);
			weight  = (float) atof (sptr + 52);
			sprintf (astype, "%-3.3s",  sptr + 68);
		}

		strcpy (prev_group, group);

		sprintf (group, "%-2.2s %-6.6s %-2.2s", 
						company, dbt_no, branch);

		if (first_time)
		{
			strcpy (esmr_rec.co_no, company);
			strcpy (esmr_rec.br_no, branch);
			cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
			if (!cc)
				sprintf (branch_short, "%-15.15s", esmr_rec.short_name);
			else
				sprintf (branch_short, "%-15.15s", ML ("Unknown        "));
			Defined_Section ();
		}

		if (strlen (prev_serial) && strcmp (serial, prev_serial))
		{
			print_at (10,25,ML ("Printing : Serial No. %s"),prev_serial);
			tot_cvolume += prev_cvolume;

			if (okay_print && next_branch)
				PrintSpaces ();

			next_branch = TRUE;

			if (okay_print)
				fprintf (fout,".C|%s", branch_short);
			else 
				fprintf (fout,".C|%-15.15s", " ");

			okay_print = FALSE;

			fprintf (fout,"| %-26.26s ",	    prev_serial);
			fprintf (fout,"|   %-2.2s    ",	prev_astype);
			fprintf (fout,"|   %11ld   ",	prev_cvolume);

			if (strcmp (prev_group, group))
			{
				fprintf (fout,"| %15.2f ",volume_todate);
				var_volume = tot_cvolume - volume_todate;

				if (var_volume > 0.00)
					fprintf (fout,"| (%15.2f)",		var_volume);
				else
				{
					var_volume = var_volume * (-1);
					fprintf (fout,"| %15.2f ",		var_volume);
				}	

				if (tot_cvolume == 0L)
					percent = 0.0;
				else
					percent = volume_todate/tot_cvolume*100;

				fprintf (fout,"|      %5.2f%%     |\n", percent);

				tot_cust_cvolume 	  += tot_cvolume;
				tot_cust_vtodate 	  += volume_todate;

				volume_todate = 0.00;
				tot_cvolume   = 0L;

			}
			else
			{
				fprintf (fout,"|                 ");
				fprintf (fout,"|                 ");
				fprintf (fout,"|                 |\n");
			}

		}

		if (strcmp (prev_dbt_no, dbt_no) || strcmp (branch, prev_branch))
		{
			okay_print = TRUE;
			strcpy (esmr_rec.co_no, company);
			strcpy (esmr_rec.br_no, branch);
			cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
			if (!cc)
				sprintf (branch_short, "%-15.15s", esmr_rec.short_name);
			else
				sprintf (branch_short, "%-15.15s", ML ("Unknown        "));
		}


		if (strlen (prev_dbt_no) && strcmp (prev_dbt_no, dbt_no))
		{
			PrintSpaces ();

			fprintf (fout,".C|    TOTALS     ");
			fprintf (fout,"|                            ");
			fprintf (fout,"|         ");
			fprintf (fout,"|   %11ld   ",	tot_cust_cvolume);
			fprintf (fout,"| %15.2f ",		tot_cust_vtodate);

			tot_var_volume = tot_cust_cvolume - tot_cust_vtodate;

			if (tot_var_volume > 0.00)
				fprintf (fout,"| (%15.2f)",	tot_var_volume);
			else
			{
				tot_var_volume = tot_var_volume * (-1);
				fprintf (fout,"| %15.2f ",	tot_var_volume);
			}	

			if (tot_cust_cvolume == 0L)
				tot_prcnt = 0.0;
			else
				tot_prcnt = tot_cust_vtodate/tot_cust_cvolume*100;

			fprintf (fout,"|      %5.2f%%     |\n", tot_prcnt);

			if (!last_page && strcmp (prev_company, company))
			{
				Defined_Section ();
				fprintf (fout, ".PA\n");
			}
			else
			if (!last_page && !first_time && 
				strcmp (prev_dbt_no, dbt_no))
			{
				fprintf (fout,".C|---------------");
				fprintf (fout,"|----------------------------");
				fprintf (fout,"|---------");
				fprintf (fout,"|-----------------");
				fprintf (fout,"|-----------------");
				fprintf (fout,"|-----------------");
				fprintf (fout,"|-----------------|\n");
			}

			tot_cust_cvolume = 0L;
			tot_cust_vtodate = 0L;
		}

		first_time = FALSE;	

		if (!last_page && strcmp (prev_dbt_no, dbt_no))
		{
			next_branch = FALSE;
			fprintf (fout, ".LRP2\n");
			if (!strcmp (local_rec.co_no, "  "))
			{
				strcpy (cumr_rec.cm_co_no,local_rec.co_no);
				strcpy (cumr_rec.cm_est_no,branch);
				strcpy (cumr_rec.cm_dbt_no,dbt_no);
				cc = find_rec (cumr,&cumr_rec,GTEQ,"r");
				while (!cc && strcmp (cumr_rec.cm_dbt_no, dbt_no))
					cc = find_rec (cumr,&cumr_rec,NEXT,"r");
				if (!strcmp (cumr_rec.cm_dbt_no, dbt_no))
					cc = 0;
			}
			else
			{
				strcpy (cumr_rec.cm_co_no,local_rec.co_no);
				strcpy (cumr_rec.cm_est_no,branch);
				strcpy (cumr_rec.cm_dbt_no,dbt_no);
				cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
			}

			if (!cc)
				fprintf (fout,".C|  Customer No. %-6.6s  -  %-40.40s%-60.60s|\n",
						dbt_no, cumr_rec.cm_name, " ");
			else
			{
				fprintf (fout,
						".C|  Customer No. %-6.6s  -  %-40.40s%-60.60s|\n",
						dbt_no, " ", " ");
			}

		}

		prev_cvolume = cvolume;
		strcpy (last_dbt_no, prev_dbt_no);
		strcpy (prev_dbt_no, dbt_no);
		strcpy (prev_serial, serial);
		strcpy (prev_astype, astype);
		strcpy (prev_company,company);
		strcpy (prev_branch, branch);

		if (!strcmp (serial, prev_serial))
				volume_todate += weight;


		if (last_page)
			last_page = 0;
		else
		{
			sptr =  sort_read (fsort);
			if (sptr == (char *)0)
				last_page = 1;
		}
	}

}

void
PrintSpaces (void)
{	
	fprintf (fout,".C|               ");
	fprintf (fout,"|                            ");
	fprintf (fout,"|         ");
	fprintf (fout,"|                 ");
	fprintf (fout,"|                 ");
	fprintf (fout,"|                 ");
	fprintf (fout,"|                 |\n");
}

int
heading (
 int                scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (" Asset Yield Report "),25,0,1);
	move (0,1);
	line (80);

	box (0,3,80,7);
	move (1,5);
	line (79);

	move (1,8);
	line (79);

	move (0,20);
	line (80);

	print_at (21,0, ML (mlStdMess038), comm_rec.tco_no,clip (comm_rec.tco_name));
	print_at (22,0, ML (mlStdMess039), comm_rec.test_no,clip (comm_rec.test_name));

	line_cnt = 0;
	scn_write (scn);

    return (EXIT_SUCCESS);
}
