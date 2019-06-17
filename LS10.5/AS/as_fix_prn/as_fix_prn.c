/*=====================================================================
|  Copyright (C) 1996 - 1998 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( as_fix_prn.c)                                    |
|  Program Desc  : ( Asset Repair Report.                 	      )   |	
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, asmr, asdt, asst, assc                      |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,    ,    ,    ,    ,    ,    ,               |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Caloi  Escarrilla, Jr. | Date Written  : 02/06/95  |
|---------------------------------------------------------------------|
|  Modified by   : (25/03/95)             | Ross S. Baquillos         |
|  Modified by   : (06/02/98)             | Elena B. Cuaresma         |
|                :                                                    |
|  Comments      :                                                    |
|  (25/03/95)    :  SMF 00130 - fix bugs                              |
|  (06/02/98)    :  9.10  New asset module for standard version 9.    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
#define	SLEEP_TIME		2

char	*PNAME = "$RCSfile: as_fix_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/AS/as_fix_prn/as_fix_prn.c,v 5.3 2002/07/17 09:56:54 scott Exp $";

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
	| Asset Master File         |
	=============================*/
	struct dbview asmr_list[] ={
		{"asmr_co_no"},
		{"asmr_br_no"},
		{"asmr_ass_group"},
		{"asmr_ass_no"},
		{"asmr_serial_no"},
		{"asmr_type"},
		{"asmr_brand"},
		{"asmr_hhas_hash"},
		{"asmr_desc"},
		{"asmr_status_code"},
	};

	int asmr_no_fields = 10;

	struct {
		char	co_no[3];
		char	br_no[3];
		char	ass_group[6];
		char	ass_no[6];
		char	serial_no[26];
		char	type[4];
		char	brand[9];
		long	hhas_hash;
		char	desc[81];
		char	status_code[3];
	} asmr_rec;


	/*======+
	 | asdt |
	 +======*/
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


	/*===========================
	| Asset Service Type File |
	===========================*/
	struct dbview asst_list[] ={
		{"asst_co_no"},
		{"asst_ser_code"},
		{"asst_ser_desc"},
	};

	int asst_no_fields = 3;

	struct {
		char	co_no[3];
		char	ser_code[3];    
		char	ser_desc[41];   
	} asst_rec;	 

	/*===========================
	| Asset Service Type File |
	===========================*/
	struct dbview assc_list[] ={
		{"assc_co_no"},
		{"assc_stat_code"},
		{"assc_stat_desc"},
	};

	int assc_no_fields = 3;

	struct {
		char	co_no[3];
		char	stat_code[3];    
		char	stat_desc[41];   
	} assc_rec;	 

	FILE	*fout;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	from_ser_no[26];
	char	to_ser_no[26];
	char	snum_desc[2][81];
	char	from_brand[9];
	char	to_brand[9];
	char	brand_desc[2][13];
	char	from_sercode[3];
	char	to_sercode[3];
	char	ser_desc[2][41];
	long	from_date;         
	long	to_date;         
	int		lpno;
	char	lp_str[3];
	char	fdate_str[11];         
	char	tdate_str[11];         
	char 	back[5];
	char	onite[5];
	char	dummy[11];
} local_rec;


static	struct	var	vars[] =
{
	{1, LIN, "serial_start",	 4, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Asset Start", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.from_ser_no},
	{1, LIN, "snum_desc1",	 4, 49, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.snum_desc[0]},
	{1, LIN, "serial_end",	 5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "~~~~~~~~", "Asset End", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.to_ser_no},
	{1, LIN, "snum_desc2",	 5, 49, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.snum_desc[1]},
	{1, LIN, "brand_start",	 7, 21, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", "Brand Start", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.from_brand},
	{1, LIN, "brand_desc1",	 7, 49, CHARTYPE,
		"AAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.brand_desc[0]},
	{1, LIN, "brand_end",	 8, 21, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "~~~~~~~~", "Brand End", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.to_brand},
	{1, LIN, "brand_desc2",	 8, 49, CHARTYPE,
		"AAAAAAAAAAAA", "          ",
		" ", "", " ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.brand_desc[1]},
	{1, LIN, "sertype_start",	 10, 21, CHARTYPE,
		"UU", "          ",
		" ", " ", "Service type Start", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.from_sercode},
	{1, LIN, "service_desc1",	 10, 49, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.ser_desc[0]},
	{1, LIN, "sertype_end",	 11, 21, CHARTYPE,
		"UU", "          ",
		" ", "~~", "Service type End", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.to_sercode},
	{1, LIN, "service_desc2",	 11, 49, CHARTYPE,
		"UU", "          ",
		" ", " ", " ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.ser_desc[1]},
	{1, LIN, "date_start",	 13, 21, EDATETYPE,
		"00/00/00", "          ",
		" ", " ", "Service Start Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.from_date},
	{1, LIN, "date_end",	 14, 21, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Service End Date", " ",
		YES, NO, JUSTLEFT, "", "", (char *)&local_rec.to_date},
	{1, LIN, "lpnum",	 16, 21, INTTYPE,
		"NN", "          ",
		" ", " ", "Printer No.", " ",
		YES, NO, JUSTRIGHT, "", "",(char *)&local_rec.lpno},
	{1, LIN, "backg",	 17, 21, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	 17, 63, CHARTYPE,
		"U", "          ",
		" ", "N(o ", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototype.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
int run_prog (char *prog_name);
void head_output (void);
void print_line (void);
void process_file (void);
int heading (int scn);
void asmr_search (char *key_val);
void brand_search (char *key_val);
void asst_search (char *key_val);

char	*data = "data",
		*asmr = "asmr",
		*asdt = "asdt",
		*asst = "asst",
		*assc = "assc";

int 	line_count = 0;

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv[])
{
	if (argc != 1 && argc != 10)
	{
		print_at(0,0, ML("Usage : %s <Start Asset> <End Asset> <Start Brand> <End Brand> <Start Service Code> <End Service Code> <Start Service Date> <End Service Date> <lpno>\n"));
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	OpenDB ();

	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	if (argc == 10)
	{
		sprintf(local_rec.from_ser_no,"%-25.25s",argv[1]);
		sprintf(local_rec.to_ser_no,"%-25.25s",argv[2]);
		sprintf(local_rec.from_brand,"%-8.8s",argv[3]);
		sprintf(local_rec.to_brand,"%-8.8s",argv[4]);
		sprintf(local_rec.from_sercode,"%-2.2s",argv[5]);
		sprintf(local_rec.to_sercode,"%-2.2s",argv[6]);
		local_rec.from_date = atol(argv[7]);
		local_rec.to_date = atol(argv[8]);
		local_rec.lpno = atoi(argv[9]);


		dsp_screen("Processing : Printing Asset Repair Report.",
					comm_rec.tco_no,comm_rec.tco_name);

		head_output();
		process_file ();  
		fprintf(fout,".EOF\n");
		pclose(fout);
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
	swide();

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
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading(1);
		scn_display(1);     /* Display contents of screen 	*/
		edit(1);            /* Edit a screen 				*/
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

	abc_alias("asmr2", 	asmr);
	open_rec(asmr,	 	asmr_list,	asmr_no_fields,	"asmr_id_no3");
	open_rec("asmr2",	asmr_list,	asmr_no_fields,	"asmr_id_no2"); 
	open_rec(asdt,		asdt_list,	ASDT_NO_FIELDS,	"asdt_id_no"); 
	open_rec(asst,		asst_list,	asst_no_fields,	"asst_id_no"); 
	open_rec(assc,		assc_list,	assc_no_fields,	"assc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("asmr");
	abc_fclose("asdt");
	abc_fclose("asst");
	abc_fclose("assc");
	abc_dbclose("data");
}

int
spec_valid (
 int                field)
{
	/*---------------------------------
	| Validate Asset Starting Entry |
	---------------------------------*/
	if ( LCHECK( "serial_start") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.snum_desc[0],	ML("Asset Start"));
			DSP_FLD("snum_desc1");
			return(0);
		} 

		if (last_char == SEARCH)
		{
			asmr_search (temp_str);
			return(0);
		}
		if (prog_status != ENTRY && 
			strcmp(local_rec.from_ser_no,local_rec.to_ser_no) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}

		strcpy(asmr_rec.co_no,comm_rec.tco_no);
		strcpy(asmr_rec.br_no,comm_rec.test_no);
		strcpy(asmr_rec.serial_no,local_rec.from_ser_no);
		cc = find_rec(asmr,&asmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML("Asset not found."));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.snum_desc[0],clip(asmr_rec.desc));
		DSP_FLD( "snum_desc1" );
		return(0);
	}

	/*-------------------------------
	| Validate Asset Ending Entry |
	-------------------------------*/

	if ( LCHECK("serial_end") )
	{
		if (dflt_used)
		{
			strcpy(local_rec.snum_desc[1],	ML("Asset End"));
			DSP_FLD("snum_desc2");
			return(0);
		}

		if (last_char == SEARCH)
		{
			asmr_search(temp_str);
			return(0);
		}
		strcpy(asmr_rec.co_no,comm_rec.tco_no);
		strcpy(asmr_rec.br_no,comm_rec.test_no);
		strcpy(asmr_rec.serial_no,local_rec.to_ser_no);
		cc = find_rec(asmr,&asmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML("Asset not found."));
			sleep(1);
			clear_mess();
			return(1);
		}
		if (strcmp(local_rec.from_ser_no,local_rec.to_ser_no) > 0)
		{
			errmess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.snum_desc[1],clip(asmr_rec.desc));
		DSP_FLD("snum_desc2");
		return(0);
	}
	/*-------------------------------
	| Validate Brand Starting Entry |
	--------------------------------*/

	if ( LCHECK("brand_start") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.brand_desc[0],"%-12.12s",	ML("Brand Start"));
			DSP_FLD("brand_desc1");
			return(0);
		}

		if (last_char == SEARCH)
		{
			brand_search (temp_str);
			return(0);
		}
		strcpy(asmr_rec.co_no,comm_rec.tco_no);
		strcpy(asmr_rec.br_no,comm_rec.test_no);
		strcpy(asmr_rec.brand,local_rec.from_brand);
		cc = find_rec("asmr2",&asmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess073));
			sleep(1);
			clear_mess();
			return(1);
		}
		if (prog_status != ENTRY && 
			strcmp(local_rec.from_brand,local_rec.to_brand) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}
		DSP_FLD("brand_start"); 
		return(0);
	}

	/*-----------------------------
	| Validate Brand Ending Entry |
	------------------------------*/

	if ( LCHECK("brand_end") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.brand_desc[1],"%-12.12s",	ML("Brand End"));
			DSP_FLD("brand_desc2");
			return(0);
		}

		if (last_char == SEARCH)
		{
			brand_search(temp_str);
			return(0);
		}
		strcpy(asmr_rec.co_no,comm_rec.tco_no);
		strcpy(asmr_rec.br_no,comm_rec.test_no);
		strcpy(asmr_rec.brand,local_rec.to_brand);
		cc = find_rec("asmr2",&asmr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML(mlStdMess073));
			sleep(1);
			clear_mess();
			return(1);
		}
		if (strcmp(local_rec.from_brand,local_rec.to_brand) > 0)
		{
			errmess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}
		DSP_FLD("brand_end");  
		return(0);
	}

	/*---------------------------------------------
	| Validate Asset Starting Service Type Code |
	----------------------------------------------*/

	if ( LCHECK("sertype_start") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.ser_desc[0],"%-40.40s",ML("Service Type Start"));
			DSP_FLD("service_desc1");
			return(0);
		}

		if (last_char == SEARCH)
		{
			asst_search (temp_str);
			return(0);
		}
		strcpy(asst_rec.co_no,comm_rec.tco_no);
		strcpy(asst_rec.ser_code,local_rec.from_sercode);
		cc = find_rec(asst,&asst_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML("Service type not found."));
			sleep(1);
			clear_mess();
			return(1);
		}
		if (prog_status != ENTRY && 
			strcmp(local_rec.from_sercode,local_rec.to_sercode) > 0)
		{
			print_mess(ML(mlStdMess017));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.ser_desc[0],asst_rec.ser_desc);
		DSP_FLD("service_desc1"); 
		return(0);
	}

	/*------------------------------------
	| Validate Service type Ending Entry |
	-------------------------------------*/

	if ( LCHECK("sertype_end") )
	{
		if (dflt_used)
		{
			sprintf(local_rec.ser_desc[1],"%-40.40s",	ML("Service Type end"));
			DSP_FLD("service_desc2");
			return(0);
		}

		if (last_char == SEARCH)
		{
			asst_search(temp_str);
			return(0);
		}

		strcpy(asst_rec.co_no,comm_rec.tco_no);
		strcpy(asst_rec.ser_code,local_rec.from_sercode);
		cc = find_rec(asst,&asst_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess(ML("Service type not found."));
			sleep(1);
			clear_mess();
			return(1);
		}

		if (strcmp(local_rec.from_sercode,local_rec.to_sercode) > 0)
		{
			errmess(ML(mlStdMess018));
			sleep(1);
			clear_mess();
			return(1);
		}
		strcpy(local_rec.ser_desc[1],asst_rec.ser_desc);
		DSP_FLD("service_desc2");  
		return(0);
	}

	/*----------------------------------------
	| Validate Asset Starting Service Date |
	----------------------------------------*/

	if ( LCHECK("date_start") )
	{

		if ( local_rec.from_date > TodaysDate ())
		{
			print_mess(ML(mlStdMess068));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		if ((local_rec.from_date > local_rec.to_date) && prog_status != ENTRY)
		{
			print_mess(ML(mlStdMess019));
			sleep(SLEEP_TIME);
			clear_mess();
			return(1);
		}

		DSP_FLD("date_start");
		return(0);
	}

	/*------------------------------------
	| Validate Service Date Ending Entry |
	-------------------------------------*/

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
			print_mess(ML(mlStdMess068));
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
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		return(0);
	}
	/*-----------------------------------------------
	| Validate for Background or foreground process |
	------------------------------------------------*/

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
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	sprintf(local_rec.fdate_str,"%ld",local_rec.from_date);
	sprintf(local_rec.tdate_str,"%ld",local_rec.to_date);
	
	clear();
	print_at(0,0, ML(mlStdMess035));
	fflush(stdout);

	CloseDB (); FinishProgram ();;
	rset_tty();


	if (local_rec.onite[0] == 'Y')
	{

		if (fork() == 0)
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.from_ser_no,
				local_rec.to_ser_no,
				local_rec.from_brand,
				local_rec.to_brand,
				local_rec.from_sercode,
				local_rec.to_sercode,
				local_rec.from_date,
				local_rec.to_date,
				local_rec.lp_str,
				"Print Asset Repair Report",(char *)0);
        return (EXIT_FAILURE);
	}
	else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
				prog_name,
				local_rec.from_ser_no,
				local_rec.to_ser_no,
				local_rec.from_brand,
				local_rec.to_brand,
				local_rec.from_sercode,
				local_rec.to_sercode,
				local_rec.from_date,
				local_rec.to_date,
				local_rec.lp_str,(char *)0);
        return (EXIT_FAILURE);
	}
	else 
	{

		execlp(prog_name,
			prog_name,
			local_rec.from_ser_no,
			local_rec.to_ser_no,
			local_rec.from_brand,
			local_rec.to_brand,
			local_rec.from_sercode,
			local_rec.to_sercode,
			local_rec.fdate_str,
			local_rec.tdate_str,
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

	fprintf(fout, ".START%s <%s>\n", DateToString (TodaysDate()),PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);

	fprintf(fout,".15\n");
	fprintf(fout,".PI16\n");
	fprintf(fout,".L230\n");
	fprintf(fout,".B1\n");      
	fprintf(fout,".ECompany : %s - %s\n",   
				comm_rec.tco_no,clip(comm_rec.tco_name));
	fprintf(fout,".EBranch  : %s - %s\n",
				comm_rec.test_no,clip(comm_rec.test_name));
	fprintf(fout,".B1\n");
	fprintf(fout,".EASSET REPAIR REPORT\n");  
	fprintf(fout,".B1\n");
	fprintf(fout,".EAS AT %-24.24s\n",SystemTime ());
	fprintf(fout,".B1\n");

	fprintf(fout,".R%-29.29s=========="," ");
	fprintf(fout,"===========");
	fprintf(fout,"===========");
	fprintf(fout,"=============");
	fprintf(fout,"===========");
	fprintf(fout,"===================");
	fprintf(fout,"===================");
	fprintf(fout,"===================");
	fprintf(fout,"===========================================");
	fprintf(fout,"===============\n");

	if (strlen (clip (local_rec.from_ser_no)))
		fprintf(fout, ".CStart Serial No. : %s ", clip(local_rec.from_ser_no));
	else
		fprintf(fout, ".CStart Serial No. : START ");
	fprintf(fout, "End Serial No. : %s  / ", clip(local_rec.to_ser_no));

	if (strlen (clip (local_rec.from_brand)))
		fprintf(fout, "Start Brand : %s ", local_rec.from_brand);
	else
		fprintf(fout, "Start Brand : START ");
	fprintf(fout, "End Brand : %s  / ", local_rec.to_brand);

	if (strlen (clip (local_rec.from_sercode)))
		fprintf(fout, "Start Service Code : %s ", local_rec.from_sercode);
	else
		fprintf(fout, "Start Service Code : START ");
	fprintf(fout, "End Service Code : %s  / ", local_rec.to_sercode);

	fprintf(fout, "Start Date : %s  ", 		   DateToString(local_rec.from_date));
	fprintf(fout, "End Date : %s  \n", 		   DateToString(local_rec.to_date));

	fprintf(fout,".C==========");
	fprintf(fout,"===========");
	fprintf(fout,"===========");
	fprintf(fout,"=============");
	fprintf(fout,"===========");
	fprintf(fout,"===================");
	fprintf(fout,"===================");
	fprintf(fout,"===================");
	fprintf(fout,"===========================================");
	fprintf(fout,"===============\n");

	fprintf(fout,".C|      SERIAL    NUMBER      ");
	fprintf(fout,"|    BRAND    ");
	fprintf(fout,"| ASSET TYPE ");
	fprintf(fout,"|SERVICE DATE"); 
	fprintf(fout,"|              SERVICE  TYPE                 ");
	fprintf(fout,"|             SERVICE  REMARKS               ");
	fprintf(fout,"|  STATUS  |\n");
	print_line ();

	fflush(fout);
}

void
print_line (void)
{
	fprintf(fout,".C|----------------------------");
	fprintf(fout,"|-------------");
	fprintf(fout,"|------------");
	fprintf(fout,"|------------");
	fprintf(fout,"|--------------------------------------------");
	fprintf(fout,"|--------------------------------------------");
	fprintf(fout,"|----------|\n");
	fflush(fout);
}

void
process_file (void)
{
	int first	=	FALSE;

	strcpy(asmr_rec.co_no,comm_rec.tco_no);
	strcpy(asmr_rec.br_no,comm_rec.test_no);
	strcpy(asmr_rec.serial_no,local_rec.from_ser_no);

	cc = find_rec(asmr,&asmr_rec,GTEQ,"r");

	while (!cc && !strcmp(asmr_rec.co_no,comm_rec.tco_no) && 
			!strcmp(asmr_rec.br_no,comm_rec.test_no) &&
			strcmp(asmr_rec.serial_no,local_rec.from_ser_no) >= 0 && 
			strcmp(asmr_rec.serial_no,local_rec.to_ser_no) <= 0)
	{
		fprintf(fout,".LRP3\n");

		if (strcmp(asmr_rec.brand,local_rec.from_brand) < 0 || 
			strcmp(asmr_rec.brand,local_rec.to_brand) > 0)  
		{
			cc = find_rec(asmr,&asmr_rec,NEXT,"r");
			first = TRUE;
			continue;
		}
	
		asdt_rec.hhas_hash = asmr_rec.hhas_hash;
		asdt_rec.line_no = 0;

		cc = find_rec(asdt,&asdt_rec,GTEQ,"r");

		while (!cc && (asdt_rec.hhas_hash == asmr_rec.hhas_hash) && 
				(asdt_rec.ser_date >= local_rec.from_date) && 
				(asdt_rec.ser_date <= local_rec.to_date) ) 
		{
			if ( strcmp(asdt_rec.ser_type,local_rec.from_sercode) >= 0 || 
				 strcmp(asdt_rec.ser_type,local_rec.to_sercode) <= 0 ) 
			{
				strcpy(asst_rec.co_no, comm_rec.tco_no);
				strcpy(asst_rec.ser_code,asdt_rec.ser_type);
				cc = find_rec(asst,&asst_rec,COMPARISON,"r");
				if (cc)
					sprintf (asst_rec.ser_desc, "%-40.40s", 
							ML("Unknown Service Type."));

				if (first && !strcmp(asst_rec.ser_code,asdt_rec.ser_type))
				{
					line_count++;
					if (line_count ==2)
					{
						line_count = 1;
						print_line ();
					}
					fprintf(fout,".C| %-25.25s  ", asmr_rec.serial_no);
					fprintf(fout,"|  %-8.8s   ", asmr_rec.brand);
					fprintf(fout,"|    %-3.3s     ", asmr_rec.type);
					fprintf(fout,"| %-10.10s " ,
							 DateToString(asdt_rec.ser_date));
					fprintf(fout,"|  %-40.40s  ", asst_rec.ser_desc);
					fprintf(fout,"|  %-40.40s  ", asdt_rec.remarks);
					fprintf(fout,"|    %-2.2s    |\n", asmr_rec.status_code);
					first = FALSE;
				}
				else if (!first && !strcmp(asst_rec.ser_code,asdt_rec.ser_type))
				{
					fprintf(fout,".C| %-25.25s  ", " ");
					fprintf(fout,"|  %-8.8s   ", " ");
					fprintf(fout,"|  %-7.7s   ", " ");
					fprintf(fout,"| %-10.10s ",
							 DateToString(asdt_rec.ser_date));
					fprintf(fout,"|  %-40.40s  ", asst_rec.ser_desc);
					fprintf(fout,"|  %-40.40s  "   , asdt_rec.remarks);
					fprintf(fout,"|    %-2.2s    |\n", asmr_rec.status_code);
				}
			}

			cc = find_rec(asdt,&asdt_rec,NEXT,"r");
			if (asmr_rec.hhas_hash != asdt_rec.hhas_hash)
				first = TRUE;
			
		}
		cc = find_rec(asmr,&asmr_rec,NEXT,"r");
	}
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
	rv_pr(ML("Print Asset Repair Report"),50,0,1);  
	move(0,1);
	line(132);

	box(0,3,132,14);
	move(1,6);
	line(131);
	move(1,9);
	line(131);
	move(1,12);
	line(131);
	move(1,15);
	line(131);

	move(0,20);
	line(132);
	move(0,21);
	print_at(21,0, ML(mlStdMess038), comm_rec.tco_no,clip(comm_rec.tco_name));
	print_at(22,0, ML(mlStdMess039), comm_rec.test_no,clip(comm_rec.test_name));
	line_cnt = 0;
	scn_write(scn); 
    return (EXIT_SUCCESS);
}

void
asmr_search (
 char*              key_val)
{
	char temp[26];
	char prev_ser_no[26];

	sprintf(temp,"%-25.25s", " ");
	sprintf(prev_ser_no,"%-25.25s", " ");

	work_open ();
	save_rec ("#Serial No.","#Description");
	strcpy(asmr_rec.co_no,comm_rec.tco_no);
	strcpy(asmr_rec.br_no,comm_rec.test_no);    
	strcpy(asmr_rec.serial_no,key_val);    
	cc = find_rec(asmr,&asmr_rec,GTEQ,"r");

	while (!cc && !strcmp(asmr_rec.co_no,comm_rec.tco_no) &&  
			!strcmp(asmr_rec.br_no, comm_rec.test_no) && 
			!strncmp(asmr_rec.serial_no,key_val, strlen(key_val)))  
	{
		if (!strcmp(asmr_rec.serial_no,temp) && 
			!strcmp(asmr_rec.serial_no,prev_ser_no))
		{
			strcpy(prev_ser_no,asmr_rec.serial_no);
			cc = find_rec (asmr,&asmr_rec,NEXT,"r");
			continue;
		}	
		else if (strcmp(asmr_rec.serial_no,temp) && 
				 strcmp(asmr_rec.serial_no,prev_ser_no))
		{
			cc = save_rec (asmr_rec.serial_no, asmr_rec.desc);
			if (cc)
				break;
		}

		strcpy(prev_ser_no,asmr_rec.serial_no);

		cc = find_rec (asmr,&asmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (asmr_rec.co_no,comm_rec.tco_no);
	strcpy (asmr_rec.br_no,comm_rec.test_no);   
	sprintf(asmr_rec.serial_no,"%-25.25s", key_val);
	cc = find_rec (asmr,&asmr_rec, COMPARISON,"r");
	if (cc)
		file_err(cc, "asmr", "DBFIND");
}

void
brand_search (
 char*              key_val)
{
	char temp[9];
	char prev_brand[9];

	sprintf(temp,"%-8.8s", " ");
	sprintf(prev_brand,"%-8.8s", " ");

	work_open ();
	save_rec ("#Brand", "#Description");
	strcpy(asmr_rec.co_no,comm_rec.tco_no);
	strcpy(asmr_rec.br_no,comm_rec.test_no);    
	sprintf(asmr_rec.brand,"%-8.8s", key_val); 
	cc = find_rec("asmr2",&asmr_rec,GTEQ,"r");

	while (!cc && !strcmp(asmr_rec.co_no,comm_rec.tco_no) &&  
			!strcmp(asmr_rec.br_no, comm_rec.test_no) && 
			!strncmp(asmr_rec.brand,key_val, strlen(key_val)))  
	{
		if (!strcmp(asmr_rec.brand,temp) && !strcmp(asmr_rec.brand,prev_brand))
		{
			strcpy(prev_brand,asmr_rec.brand);
			cc = find_rec ("asmr2",&asmr_rec,NEXT,"r");
			continue;
		}	
		else if (strcmp(asmr_rec.brand,temp) && strcmp(asmr_rec.brand,prev_brand))
		{
			cc = save_rec (asmr_rec.brand, " ");
			if (cc)
				break;
		}
		strcpy(prev_brand,asmr_rec.brand);
		cc = find_rec ("asmr2",&asmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (asmr_rec.co_no,comm_rec.tco_no);
	strcpy (asmr_rec.br_no,comm_rec.test_no);   
	sprintf(asmr_rec.brand,"%-25.25s", key_val);
	cc = find_rec ("asmr2",&asmr_rec, COMPARISON,"r");
	if (cc)
		file_err(cc, "asmr", "DBFIND");
}

void
asst_search (
 char*              key_val)
{
	work_open ();
	save_rec ("#Code", "# Description");
	strcpy(asst_rec.co_no,comm_rec.tco_no);
	strcpy(asst_rec.ser_code,key_val);
	cc = find_rec(asst,&asst_rec,GTEQ,"r");

	while (!cc && !strcmp(asst_rec.co_no,comm_rec.tco_no) &&  
			!strncmp(asst_rec.ser_code,key_val,strlen(key_val)))  
	{
		cc = save_rec (asst_rec.ser_code,asst_rec.ser_desc);
		if (cc)
			break;

		cc = find_rec (asst,&asst_rec,NEXT,"r");
	}

	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy (asst_rec.co_no,comm_rec.tco_no);
	strcpy(asst_rec.ser_code,temp_str);
	cc = find_rec (asst,&asst_rec, COMPARISON,"r");
	if (cc)
		file_err(cc, "asst", "DBFIND");
}
