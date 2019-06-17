/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (as_move_prn.c)                                    |
|  Program Desc  : (Asset Movement Report.                 	  )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, asmr, asmv, sumr                      |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Caloi  Escarrilla, Jr. | Date Written  : 02/13/95  |
|---------------------------------------------------------------------|
| Date Modified  : (27/08/95)  | Modified By : 	Ross Baquillos		  |
| Date Modified  : (06/02/98)  | Modified By : 	Elena Cuaresma		  |
|                :                                                    |
|                :                                                    |
| (27/08/95)     : SMF 00130 - Fix lots of bugs.                      |
| (06/02/98)     : 9.10  New asset module for the standard version 9. |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
#define	SLEEP_TIME  2	 

char	*PNAME = "$RCSfile: move_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_move_prn/move_prn.c,v 5.3 2002/07/17 09:56:55 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
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

	/*=============================
	| Supplier Master File         |
	=============================*/
	struct dbview sumr_list[] ={
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_crd_name"},
		{"sumr_hhsu_hash"},
	};

	int sumr_no_fields = 5 ;

	struct {
		char	co_no[3];
		char 	est_no[3];
		char	crd_no[7];
		char	crd_name[41];
		long	hhsu_hash;
	} sumr_rec;	 
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

	/*=============================
	| Asset Master File         |
	=============================*/
	struct dbview asmr_list[] ={
		{"asmr_co_no"},
		{"asmr_br_no"},
		{"asmr_serial_no"},
		{"asmr_type"},
		{"asmr_brand"},
		{"asmr_hhas_hash"},
		{"asmr_hhsu_hash"},
		{"asmr_capacity"},
	};

	int asmr_no_fields = 8;

	struct {
		char	co_no[3];
		char	br_no[3];
		char	serial_no[26];
		char	type[2];
		char	brand[9];
		long	hhas_hash;
		long	hhsu_hash;
		double	capacity;
	} asmr_rec;

	/*=============================
	| Customer Master File         |
	=============================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
		{"cumr_hhcu_hash"}
	};

	int cumr_no_fields = 5;

	struct {
		char	co_no   [3];
		char	est_no  [3];
		char	dbt_no  [7];
		char	dbt_name[41];
		long	hhcu_hash;
	} cumr_rec;

	/*============================+
	 | Asset Movement Type File |
	 +============================*/
#define	ASMT_NO_FIELDS	3

	struct dbview	asmt_list [ASMT_NO_FIELDS] =
	{
		{"asmt_co_no"},
		{"asmt_type_code"},
		{"asmt_desc"}
	};

	struct tag_asmtRecord
	{
		char	co_no [3];
		char	type_code [5];
		char	desc [41];
	}	asmt_rec;

	/*=============================
	| Asset Movement File         |
	=============================*/
#define	ASMV_NO_FIELDS	15

	struct dbview	asmv_list [ASMV_NO_FIELDS] =
	{
		{"asmv_hhar_hash"},
		{"asmv_line_no"},
		{"asmv_hham_hash"},
		{"asmv_report_no"},
		{"asmv_move_code"},
		{"asmv_move_desc"},
		{"asmv_serial_no"},
		{"asmv_source_type"},
		{"asmv_dest_type"},
		{"asmv_from_crdt"},
		{"asmv_to_crdt"},
		{"asmv_from_hhcu"},
		{"asmv_to_hhcu"},
		{"asmv_move_date"},
		{"asmv_vol_commit"}
	};

	struct tag_asmvRecord
	{
		long	hhas_hash;
		int		line_no;
		long	hham_hash;
		long	report_no;
		char	move_code [5];
		char	move_desc [41];
		char	serial_no [26];
		char	source_type [2];
		char	dest_type [2];
		long	from_sumr;
		long	to_sumr;
		long	from_hhcu;
		long	to_hhcu;
		Date	move_date;
		long	vol_commit;
	} asmv_rec, last_movement;	 



	struct {
		char	co_no[3];
		char	est_no[3];
		char	dbt_no[7];
		long	hhcu_hash;
	} temp_rec;

	FILE	*fout,
			*fsort;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	long	from_rnum;
	long	to_rnum;
	char	rnum_desc[2][21];         
	long	from_date;
	long	to_date;
	int		lpno;
	char	lp_str[3];
	char	fdate_str[11];         
	char	tdate_str[11];         
	char	frnum_str[9];         
	char	trnum_str[9];         
	char	temp[9];         
	char 	back[5];
	char	onite[5];
	char    br_no[3]; 
	char    br_desc[41]; 
	char    co_no[3]; 
	char    co_desc[41]; 
	char	dummy[11];
} local_rec;


static	struct	var	vars[] =
{
	{1, LIN, "co_no", 3, 21, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Company No. ", "Enter Company Number. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.co_no},
	{1, LIN, "co_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.co_desc},
	{1, LIN, "br_no", 5, 21, CHARTYPE,
		"NN", "          ",
		" ", "  ", "Branch No", "Enter Branch No. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_desc},
	{1, LIN, "rnum_start",	 7, 21, LONGTYPE,
		"NNNNNN", "          ",
		"0", "000000", "Start Report Number", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.from_rnum},
	{1, LIN, "rnum_desc1",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTRIGHT, "", "", local_rec.rnum_desc[0]},
	{1, LIN, "rnum_end",	 8, 21, LONGTYPE,
		"NNNNNN", "          ",
		"0", "999999", "End Report Number", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.to_rnum},
	{1, LIN, "rnum_desc2",	 8, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		NA, NO,  JUSTRIGHT, "", "", local_rec.rnum_desc[1]},
	{1, LIN, "date_start",	 10, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		"00/00/00", " ", "Start Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.from_date},
	{1, LIN, "date_end",	 11, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.to_date},
	{1, LIN, "lpnum",	 13, 21, INTTYPE,
		"NN", "          ",
		" ", " ", "Printer No.", " ",
		YES, NO, JUSTRIGHT, "", "",(char *)&local_rec.lpno},
	{1, LIN, "back",	 14, 21, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 14, 56, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

char	*data = "data",
		*asmr = "asmr",
		*asmv = "asmv",
		*sumr = "sumr",
		*comr = "comr",
		*asmt = "asmt",
		*esmr = "esmr",
		*cumr = "cumr";

long 	prev_hhas_hash, 
		curr_hhas_hash;

int 	finish, entry_count = 0,
		FILE_EMPTY ;
char	curr_company	[3],
		curr_branchNo		[3];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int run_prog (char *prog_name);
void head_output (void);
void print_line (void);
void process_file (void);
void store_data (void);
void process2 (void);
void head_display (void);
int heading (int scn);
void rnum_search (char *key_val);
void SrchEsmr (char *key_val);
void SrchComr (char *key_val);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	if (argc != 1 && argc != 8)
	{
		print_at(0,0,"Usage : %s <Start Movement Report Number> <End Movement Report Number> <Start Movement Date> <End Movement Date> <lpno>\n", argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	if (argc == 8)
	{
		local_rec.from_rnum = atol(argv[1]);
		local_rec.to_rnum = atol(argv[2]);
		local_rec.from_date = atol(argv[3]);
		local_rec.to_date = atol(argv[4]);
		sprintf (local_rec.br_no, "%2.2s", argv[5]);
		sprintf (local_rec.co_no, "%2.2s", argv[6]);
		local_rec.lpno = atoi(argv[7]);

		dsp_screen("Processing : Printing Asset Movement Report.",
					comm_rec.tco_no,comm_rec.tco_name);

		FILE_EMPTY = 1;
		process_file ();   

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();                      /*  get into raw mode		*/
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

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
		scn_display(1);     /* Display contents of screen */
		edit(1);            /* Edit a screen */
		if (restart)
			continue;

		if (run_prog (argv[0]) == 1)
        {
            return (EXIT_SUCCESS);
        }
		prog_exit = 1;
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
	abc_alias("asmv2", asmv);

	open_rec (asmt, asmt_list, ASMT_NO_FIELDS, "asmt_id_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec(asmr,asmr_list,asmr_no_fields,"asmr_hhas_hash");
	open_rec(asmv,asmv_list,ASMV_NO_FIELDS,"asmv_report_no"); 
	open_rec("asmv2",asmv_list,ASMV_NO_FIELDS,"asmv_id_no"); 
	open_rec(sumr,sumr_list,sumr_no_fields,"sumr_hhsu_hash");
	open_rec(cumr,cumr_list,cumr_no_fields,"cumr_hhcu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("asmt");
	abc_fclose("comr");
	abc_fclose("esmr");
	abc_fclose("asmr");
	abc_fclose("asmv2");
	abc_fclose("asmv");
	abc_fclose("cumr");
	abc_fclose("sumr");
	abc_dbclose("data");
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
			sprintf (local_rec.br_desc, "%s", ML("All Branchs"));
			display_field (label("br_no"));
			display_field (label("br_desc"));
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchEsmr (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		if ( strcmp (local_rec.co_no, "  "))
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
		return (EXIT_SUCCESS);
	}

	/*----------------------------------
	| Validate Movement Starting Entry |
	----------------------------------*/
	if ( LCHECK( "rnum_start") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.rnum_desc[0],"%-20.20s",ML("Start Report Number"));
			DSP_FLD("rnum_desc1");
			return(0);
		}

		if (last_char == SEARCH)
		{
			rnum_search(temp_str);
			return(0);
		}

		asmv_rec.report_no = atol(temp_str);
		cc = find_rec(asmv,&asmv_rec,COMPARISON,"r");

		if (cc)
		{
			errmess(ML("Report number not found."));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (prog_status != ENTRY && 
			(local_rec.from_rnum > local_rec.to_rnum))
		{
			errmess(ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		DSP_FLD("rnum_start");
		return(0);
	}

	/*--------------------------------
	| Validate Movement Ending Entry |
	--------------------------------*/
	if ( LCHECK("rnum_end") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.rnum_desc[1],"%-20.20s",ML("End Report Number"));
			DSP_FLD("rnum_desc2");
			return(0);
		}

		if (last_char == SEARCH)
		{
			rnum_search(temp_str);
			return(0);
		}

		asmv_rec.report_no = atol(temp_str);
		cc = find_rec(asmv,&asmv_rec,COMPARISON,"r");

		if (cc)
		{
			errmess(ML("Report number not found."));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (local_rec.from_rnum > local_rec.to_rnum)
		{
			errmess(ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		DSP_FLD("rnum_end");
		return(0);
	}

	/*---------------------------------
	| Validate Movement Starting Date |
	---------------------------------*/

	if ( LCHECK("date_start") )
	{
		if (dflt_used)
		{
			local_rec.from_date = 0L;
			DSP_FLD("date_start");
			return(0);
		}

		if ( local_rec.from_date > TodaysDate ())
		{
			errmess(ML(mlStdMess068));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}
		if ((local_rec.from_date > local_rec.to_date) && prog_status != ENTRY)
		{
			errmess(ML(mlStdMess019));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		DSP_FLD("date_start");
		return(0);
	}

	/*-------------------------------
	| Validate Movement Ending Date |
	-------------------------------*/

	if ( LCHECK("date_end") )
	{
		if (dflt_used)
		{
			local_rec.to_date = TodaysDate ();
			DSP_FLD("date_end");
			return(0);
		}

		if ( local_rec.to_date > TodaysDate ())
		{
			errmess(ML(mlStdMess068));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		if (local_rec.from_date > local_rec.to_date)
		{
			errmess(ML(mlStdMess026));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		DSP_FLD("date_end");
		return(0);
	}

	/*-------------------------
	| Validate Printer Number |
	-------------------------*/
	if ( LCHECK("lpnum") )
	{
		if(dflt_used)
		{
			local_rec.lpno = 1;
			return(0);
		}	

		if (last_char == SEARCH)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			errmess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		return(0);
	}

	/*-----------------------------------------------
	| Validate for Background or foreground process |
	-----------------------------------------------*/
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

	return(0);
}

int
run_prog (
 char*              prog_name)
{
	sprintf(local_rec.frnum_str,"%ld",local_rec.from_rnum);
	sprintf(local_rec.trnum_str,"%ld",local_rec.to_rnum);
	sprintf(local_rec.fdate_str,"%ld",local_rec.from_date);
	sprintf(local_rec.tdate_str,"%ld",local_rec.to_date);
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	
	clear();
	print_at(0,0, ML(mlStdMess035));
	fflush(stdout);

	CloseDB (); FinishProgram ();;
	rset_tty ();


	if (local_rec.onite[0] == 'Y')
	{

		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.frnum_str,
				local_rec.trnum_str,
				local_rec.fdate_str,
				local_rec.tdate_str,
				local_rec.br_no,
				local_rec.co_no,
				local_rec.lp_str,
				"Print  Asset Movement Report",(char *)0);
        return (EXIT_FAILURE);
	}
    else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.frnum_str,
				local_rec.trnum_str,
				local_rec.fdate_str,
				local_rec.tdate_str,
				local_rec.br_no,
				local_rec.co_no,
				local_rec.lp_str,(char *)0);
        return (EXIT_FAILURE);
	}
	else 
	{
		execlp(prog_name,
			prog_name,
			local_rec.frnum_str,
			local_rec.trnum_str,
			local_rec.fdate_str,
			local_rec.tdate_str,
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
	fprintf(fout,".PI16\n");
	fprintf(fout,".L212\n");

	fprintf(fout,".5\n");
	fprintf(fout,".B1\n");      
	fprintf(fout,".EASSET MOVEMENT REPORT\n");  
	fprintf(fout,".B1\n");
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf(fout,".B1\n");

}

void
print_line (void)
{
	fprintf(fout,".C|-------------------------");
	fprintf(fout,"|------");
	fprintf(fout,"|----------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|---------------------------------------|\n");  
	fflush(fout);
}

void
process_file (void)
{
	fsort = sort_open("as_movement");
	finish = 1;
	asmv_rec.line_no = 0;
	asmv_rec.hhas_hash = 0L;	
	prev_hhas_hash	   = 0L;	
	cc = find_rec("asmv2",&asmv_rec,GTEQ,"r");

	while (!cc)
	{		

		if ((asmv_rec.move_date >= local_rec.from_date)  && 
			(asmv_rec.move_date <= local_rec.to_date)    && 
			(asmv_rec.from_sumr >  0  					 ||
			 asmv_rec.from_hhcu >  0  					 ||
			 asmv_rec.to_sumr >  0  					 ||
			 asmv_rec.to_hhcu >  0) 					 &&
 		    (asmv_rec.report_no >= local_rec.from_rnum)  && 
			(asmv_rec.report_no <= local_rec.to_rnum)) 
				store_data();


		cc = find_rec("asmv2",&asmv_rec,NEXT,"r");	

	    if (!cc && (asmv_rec.report_no >= local_rec.from_rnum)  && 
			(asmv_rec.report_no <= local_rec.to_rnum)) 
			finish = 0;
		else
			finish = 1;
		
	}

	if (!FILE_EMPTY)
	{
		head_output ();
		process2 ();
		fprintf (fout,".EOF\n");
		pclose(fout);
	}
}

void
store_data (void)
{
	char 	data_string [130];
	long	from_location,
			to_location;
	char	from_loc_type [2],
			to_loc_type	[2];

	FILE_EMPTY = 0;
	cc = find_hash(asmr, &asmr_rec, EQUAL,"r", asmv_rec.hhas_hash);
	if (cc || 
	 	( strcmp (local_rec.co_no, "  ") 
			? strcmp(asmr_rec.co_no,local_rec.co_no) : 0) ||
	 	( strcmp (local_rec.br_no,"  ") 
			? strcmp(asmr_rec.br_no,local_rec.br_no) : 0)) 
		return;

	if (asmv_rec.from_hhcu > 0L)
	{
		from_location = asmv_rec.from_hhcu;
		strcpy (from_loc_type,"1");
	}
	else
	{
		from_location = asmv_rec.from_sumr;
		strcpy (from_loc_type,"2");
	}

	if (asmv_rec.to_hhcu > 0L)
	{
		to_location = asmv_rec.to_hhcu;
		strcpy (to_loc_type,"1");
	}
	else if (asmv_rec.to_sumr > 0L)
	{
		to_location = asmv_rec.to_sumr;
		strcpy (to_loc_type,"2");
	}
	else
	{
		to_location = 0L;
		strcpy (to_loc_type,"3");
	}

    sprintf(data_string,"%-2.2s|%-2.2s|%-26.26s|%11ld|%11d|%11ld|%-1.1s|%11ld|%-1.1s|%11ld|%11ld|%-4.4s|%11ld\n",
    asmr_rec.co_no, asmr_rec.br_no, asmr_rec.serial_no, asmv_rec.hhas_hash,
	asmv_rec.line_no, from_location, from_loc_type, to_location, to_loc_type,
	asmr_rec.hhsu_hash, asmv_rec.report_no,asmv_rec.move_code, asmv_rec.move_date );

	sort_save (fsort,data_string);
}

void
process2 (void)
{
	char 	from_str 		[41],
			to_str	 		[41],
			serial_no 		[27],
			from_hash_type  [2],
			to_hash_type	[2],
			move_code		[5],
			*sptr,
			prev_company	[3],
			prev_branchNo		[3];

	long	report_no;
	int		first_time = TRUE;

	long	asmv_hash,
			from_hash,
			to_hash,
			creditor_hash,
			prev_hhas_hash =  0L, 
			move_date;

	fsort = sort_sort(fsort, "as_movement");
	sptr =  sort_read(fsort);
	strcpy (prev_branchNo,"");
	strcpy (curr_company,"");

	while (sptr != (char *)0) 
	{

	  	sprintf (curr_company,  "%-2.2s",	 sptr );
	  	sprintf (curr_branchNo,    "%-2.2s",	 sptr + 3);
	  	sprintf (serial_no,     "%-26.26s",  sptr + 6);
		asmv_hash =  atol (sptr + 33);
		from_hash =  atol (sptr + 57);
		sprintf (from_hash_type,"%-1.1s",    sptr + 69);
		to_hash   =  atol (sptr + 71);
		sprintf (to_hash_type,  "%-1.1s",    sptr + 83);
		creditor_hash = atol (sptr + 85);
		report_no     = atol (sptr + 97);
		sprintf (move_code,     "%-4.4s",    sptr + 109);
		move_date     =  atol (sptr + 114);

	curr_hhas_hash = asmv_hash;


	if (from_hash_type[0] == '1')
	{
		memset (&cumr_rec, 0, sizeof cumr_rec); 
		cumr_rec.hhcu_hash = from_hash;
		cc = find_rec(cumr,&cumr_rec,COMPARISON,"r");
		if (!cc)
			sprintf(from_str,"%-40.40s", cumr_rec.dbt_name);
		else
			sprintf(from_str,"%-40.40s", " ");
	}
	else
	{	
		sumr_rec.hhsu_hash = from_hash;
		cc = find_rec(sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
			sprintf(from_str,"%-40.40s"," ");
		else
			sprintf(from_str,"%-40.40s",sumr_rec.crd_name);
	}

	if (to_hash_type[0] == '1')
	{
		memset (&cumr_rec, 0, sizeof cumr_rec);
		cumr_rec.hhcu_hash = to_hash;
		cc = find_rec(cumr,&cumr_rec,COMPARISON,"r");
		if (!cc)
			sprintf(to_str ,"%-40.40s", cumr_rec.dbt_name);
		else
			sprintf(to_str ,"%-40.40s", " ");
	}
	else 
	{	
		sumr_rec.hhsu_hash = to_hash;
		cc = find_rec(sumr,&sumr_rec,COMPARISON,"r");
		if (cc)
			sprintf(to_str,"%-40.40s"," ");
		else
			sprintf(to_str,"%-40.40s",sumr_rec.crd_name);
	}	

	entry_count++;

	if (strcmp (curr_branchNo, prev_branchNo) || 
		strcmp (curr_company, prev_company))
	{
		head_display ();
		if (!first_time)
			fprintf (fout, ".PA\n");
		first_time = FALSE;
	}
	else if (curr_hhas_hash != prev_hhas_hash)
		print_line();

	fprintf(fout,".LRP3\n"); 
		
	if (curr_hhas_hash != prev_hhas_hash)
	{
		prev_hhas_hash = asmv_hash;
		fprintf(fout,".C|%-25.25s", serial_no);
	}
	else
		fprintf(fout,".C|%-25.25s"," ");

	fprintf(fout,"|%6ld", report_no);
	fprintf(fout,"|%-10.10s", DateToString(move_date));
	fprintf(fout,"|%-40.40s", from_str);
	fprintf(fout,"|%-40.40s", to_str);

	strcpy (asmt_rec.type_code, move_code);
 	strcpy (asmt_rec.co_no, curr_company);
	cc = find_rec (asmt, &asmt_rec, COMPARISON, "r");
	if (!cc)
		fprintf(fout,"|%-39.39s|\n", asmt_rec.desc);
	else
		fprintf(fout,"|%-39.39s|\n", " ");

	strcpy (prev_branchNo, curr_branchNo);
	strcpy (prev_company, curr_company);

	sptr =  sort_read(fsort);

	}	

	sort_delete (fsort, "as_movement");

}

void
head_display (void)
{
	fprintf(fout, ".DS8\n");

	strcpy (comr_rec.co_no, curr_company);
	cc = find_rec (comr, &comr_rec, EQUAL, "r");
	if (!cc)
		fprintf(fout,".ECompany  : %s - %s\n", curr_company, clip(comr_rec.co_name));
	else
		fprintf(fout,".ECompany  : Unknown Branch\n");

	strcpy  (esmr_rec.co_no,  curr_company);
	sprintf (esmr_rec.br_no,  "%-2.2s", curr_branchNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (!cc)
		fprintf(fout,".EBranch  : %s - %s\n", curr_branchNo, clip(esmr_rec.br_name));
	else
		fprintf(fout,".EBranch  : Unknown Branch\n");

	fprintf(fout,".B1\n");

	fprintf(fout,".R%-23.23s======="," ");
	fprintf(fout,"=========");
	fprintf(fout,"==========================");
	fprintf(fout,"=========");
	fprintf(fout,"=========");
	fprintf(fout,"=========");
	fprintf(fout,"=========================================");
	fprintf(fout,"=======");
	fprintf(fout,"=======");
	fprintf(fout,"=========================================\n");

	fprintf(fout, ".CStart Report No. : %ld   ",   local_rec.from_rnum);
	fprintf(fout, "End Report No. : %ld   ",     local_rec.to_rnum);
	fprintf(fout, "Start Movement Date : %s   ", DateToString(local_rec.from_date));
	fprintf(fout, "End Movement Date : %s   \n", DateToString(local_rec.to_date));
	fprintf(fout,".C=======");
	fprintf(fout,"=========");
	fprintf(fout,"==========================");
	fprintf(fout,"=========");
	fprintf(fout,"=========");
	fprintf(fout,"=========");
	fprintf(fout,"=========================================");
	fprintf(fout,"=======");
	fprintf(fout,"=======");
	fprintf(fout,"=========================================\n");

	fprintf(fout,".C|     SERIAL  NUMBER      ");
	fprintf(fout,"|MVT NO");
	fprintf(fout,"| MVT DATE ");
	fprintf(fout,"|%-18.18sFROM%-18.18s"," ", " ");
	fprintf(fout,"|%-19.19sTO%-19.19s"  ," ", " ");
	fprintf(fout,"|        MOVEMENT  DESCRIPTION          |\n");

	fprintf(fout,".C|-------------------------");
	fprintf(fout,"|------");
	fprintf(fout,"|----------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|----------------------------------------");
	fprintf(fout,"|---------------------------------------|\n");  
	fflush(fout);
}

int
heading (
 int                scn)
{
	if ( restart ) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);    /* Set a screen ready for manipulation*/
	clear();
	rv_pr(ML("Print Asset Movement Report"),25,0,1);   /* Reverse video print*/
	move(0,1);
	line(80);

	box(0,2,80,12);

	move(1,4);
	line(79);

	move(1,6);
	line(79);

	move(1,9);
	line(79);

	move(1,12);
	line(79);

	print_at(20,0, ML(mlStdMess038), comm_rec.tco_no,clip(comm_rec.tco_name));
	print_at(21,0, ML(mlStdMess039), comm_rec.test_no,clip(comm_rec.test_name));

	move(0,22);
	line(80);
	line_cnt = 0;
	scn_write(scn);   /* Display all screen prompts */
    return (EXIT_SUCCESS);
}

void
rnum_search (
 char*              key_val)
{
	long temp;

	work_open ();
	save_rec ("# No","# Description");
	asmv_rec.report_no = 0L;
	cc = find_rec(asmv,&asmv_rec,GTEQ,"r");

	temp = -1L;
	while (!cc) 
	{
		if (temp != asmv_rec.report_no) 
		{
			sprintf(local_rec.temp,"%6ld",asmv_rec.report_no);
			cc = save_rec(local_rec.temp, " ");
			if (cc)
				break;
			temp = asmv_rec.report_no;
		}
		cc = find_rec(asmv,&asmv_rec,NEXT,"r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	asmv_rec.report_no = atol(temp_str);
	cc = find_rec(asmv,&asmv_rec,COMPARISON,"r");

	if (cc)
		file_err(cc, "asmv", "DBFIND");
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
	while (	!cc &&
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

