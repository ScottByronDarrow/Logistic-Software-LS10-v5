/*=====================================================================
|  Copyright (C) 1988 - 1994 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( ts_lbl_prt.c   )                                 |
|  Program Desc  : ( Print Labels For Mailers.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 17/06/92         |
|---------------------------------------------------------------------|
|  Date Modified : (04/09/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (06/10/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (13/10/92)      | Modified  by  : Campbell Mander  |
|  Date Modified : (06/09/93)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (03/11/94)      | Modified  by  : Dirk Heinsius.   |
|  Date Modified : (29/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (04/09/97)      | Modified  by  : Leah Manibog.    |
|                :                                                    |
|                                                                     |
|  Comments      : (04/09/92) - Fix for problem found by Allen.       |
|                : SC 7639 DPL.                                       |
|                :                                                    |
|  (06/10/92)    : Change printing methodology for SC 7876 DPL.       |
|                : Sort labels into order by customer name.           |
|                :                                                    |
|  (13/10/92)    : Make labels operator specific. SC 7941 DPL.        |
|                :                                                    |
|  (06/09/93)    : HGP 9745. Updated for Post_code.                   |
|                :                                                    |
|  (03/11/94)    : INF 11364 Fix problems with horizontal spacing and |
|                : the number of labels accross the page.             |
|  (29/05/96)    : Updated to Fix problems with DateToString.   			  |
|  (04/09/97)    : Updated for Multilingual Conversion and            |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ts_lbl_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_lbl_prt/ts_lbl_prt.c,v 5.3 2002/07/17 09:58:17 scott Exp $";

#define	MOD	5
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_ts_mess.h>

FILE	*fout;
FILE	*fsort;

static	char	*mth[] = {
	"January", "February", "March"    , "April"  , "May"     , "June",
	"July"   , "August"  , "September", "October", "November", "December"
};

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
	};

	int comm_no_fields = 3;
	
	struct {
		int  	termno;
		char 	tco_no[3];
		char 	tco_name[41];
	} comm_rec;

	/*=================================
	| Company Master File Base Record |
	=================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_adr1"},
		{"comr_co_adr2"},
		{"comr_co_adr3"},
	};

	int		comr_no_fields = 5;

	struct	{
		char	mr_co_no[3];
		char	mr_co_name[41];
		char	mr_co_adr[3][41];
	} comr_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_contact_name"},
		{"cumr_phone_no"},
		{"cumr_fax_no"},
		{"cumr_post_code"},
		{"cumr_area_code"},
	};

	int		cumr_no_fields = 12;

	struct	CUMR_REC
	{
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_ch_addr[3][41];
		char	cm_contact_name[21];
		char	cm_phone_no[16];
		char	cm_fax_no[16];
		char	cm_post_code[11];
		char	cm_area[3];
	} cumr_rec, cumr_store[9];

	/*====================
	| External Area file |
	====================*/
	struct dbview exaf_list[] ={
		{"exaf_co_no"},
		{"exaf_area_code"},
		{"exaf_area"},
		{"exaf_rate"},
		{"exaf_stat_flag"},
	};

	int		exaf_no_fields = 5;

	struct	{
		char	af_co_no[3];
		char	af_area_code[3];
		char	af_area[41];
		double	af_rate;	/* money */
		char	af_stat_flag[2];
	} exaf_rec;

	/*======================================
	| Tele-Sales Letters Sent History file |
	======================================*/
	struct dbview tslb_list[] ={
		{"tslb_co_no"},
		{"tslb_operator"},
		{"tslb_hhlb_hash"},
		{"tslb_hhcu_hash"},
		{"tslb_hhlh_hash"},
		{"tslb_date_sent"},
		{"tslb_time_sent"},
		{"tslb_label_prt"},
		{"tslb_stat_flag"},
	};

	int		tslb_no_fields = 9;

	struct	{
		char	lb_co_no[3];
		char	lb_operator[15];
		long	lb_hhlb_hash;
		long	lb_hhcu_hash;
		long	lb_hhlh_hash;
		long	lb_date_sent;
		long	lb_time_sent;
		char	lb_label_prt[2];
		char	lb_stat_flag[2];
	} tslb_rec;

	/*===============================
	| Tele-Sales Letter Header file |
	===============================*/
	struct dbview tslh_list[] ={
		{"tslh_co_no"},
		{"tslh_let_code"},
		{"tslh_let_desc"},
		{"tslh_hhlh_hash"},
		{"tslh_lett_type"},
	};

	int		tslh_no_fields = 5;

	struct	{
		char	lh_co_no[3];
		char	lh_lett_code[11];
		char	lh_lett_desc[41];
		long	lh_hhlh_hash;
		char	lh_lett_type[2];
	} tslh_rec;

	/*===============================
	| Tele-Sales Letter Detail File |
	===============================*/
	struct dbview tsln_list[] ={
		{"tsln_hhlh_hash"},
		{"tsln_line_no"},
		{"tsln_desc"},
	};

	int		tsln_no_fields = 3;

	struct	{
		long	ln_hhlh_hash;
		int		ln_line_no;
		char	ln_desc[79];
	} tsln_rec;

	/*==============================
	| Tele-Marketing OPerator file |
	==============================*/
	struct dbview tmop_list[] ={
		{"tmop_co_no"},
		{"tmop_op_id"},
		{"tmop_op_name"},
	};

	int		tmop_no_fields = 3;

	struct	{
		char	op_co_no[3];
		char	op_id[15];
		char	op_name[41];
	} tmop_rec;

	/*-------------------------------
	| Set up pointers to file names |
	-------------------------------*/
	char	*data    = "data",
	    	*comm    = "comm",
	    	*comr    = "comr",
	    	*cumr    = "cumr",
	    	*tslb    = "tslb",
	    	*tslb2   = "tslb2",
	    	*tslh    = "tslh",
	    	*tsln    = "tsln",
	    	*tmop    = "tmop",
	    	*exaf    = "exaf";

	int		labels_hor = 0;
	int		labels_vrt = 0;
	int		label_head = 0;
	int		label_tail = 0;
	int		label_hspc = 40;
	int		label_vspc = 0;
	int		lbl_dist;
	int		no_lab_lines = 0;
	int		pipe_open = FALSE;
	int		part_printed;
	
	char	filename[100];
	char	ts_number[15];
	char	lcl_logname[15];

#include <ts_commands.h>
/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy[11];
	char	systemDate[11];
	long	lsystemDate;
	char	oprtr[15];
	char	op_desc[41];
	char	lett_code[11];
	char	lett_desc[41];
	int		lpno;
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "operator",	 3, 12, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", " ", " Operator   :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.oprtr},
	{1, LIN, "op_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.op_desc},
	{1, LIN, "code",	 5, 12, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Letter Code:", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.lett_code},
	{1, LIN, "desc",	 6, 12, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", tslh_rec.lh_lett_desc, " Description:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.lett_desc},
	{1, LIN, "lpno",	 8, 12, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No.:", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&local_rec.lpno},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
int spec_valid (int field);
void SrchTslh (char* key_val);
void SrchTmop (char* key_val);
int read_tsln (long shash);
int process (void);
void init_cumr (void);
void prt_horiz (int no_labs);
void prvoid_head (void);
void print_head (void);
void open_label (void);
void print_vert (void);
void print_tail (void);
void prvoid_vert (void);
void prvoid_tail (void);
void parse (char *wrk_prt, int label_def, int num_hor, int no_labs);
int valid_cmd (char *wk_str, int label_def);
void subst_cmd (int cmd, int lab_no);
char *gdate (long cur_date);
int heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr = getenv("LOGNAME");

	if (sptr)
	{
		sprintf(lcl_logname, "%-14.14s", sptr);
		upshift(lcl_logname);
	}
	else
		sprintf(lcl_logname, "%-14.14s", "DEFAULT");

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	OpenDB(); 	

	ReadMisc (); 	

	init_scr();
	set_tty();
	set_masks();

	/*-------------------
	| Set control flags |
	-------------------*/
	search_ok = 1;
	entry_exit = 0;
	edit_exit = 0;
	prog_exit = 0;
	restart = 0;
	init_vars(1);	

	heading(1);
	entry(1);
	if (prog_exit || restart)
    {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }

	heading(1);
	scn_display(1);
	edit(1);
	if (restart)
    {
		shutdown_prog();
        return (EXIT_SUCCESS);
    }

	process();

	if (pipe_open)
	{
		fprintf(fout, ".EOF\n");
		fflush(fout);
		pclose(fout);
	}

	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
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

	abc_alias(tslb2, tslb);

	open_rec(cumr,  cumr_list, cumr_no_fields, "cumr_hhcu_hash");
	open_rec(exaf,  exaf_list, exaf_no_fields, "exaf_id_no");
	open_rec(tslb,  tslb_list, tslb_no_fields, "tslb_id_no3");
	open_rec(tslb2, tslb_list, tslb_no_fields, "tslb_hhlb_hash");
	open_rec(tslh,  tslh_list, tslh_no_fields, "tslh_id_no");
	open_rec(tsln,  tsln_list, tsln_no_fields, "tsln_id_no");
	open_rec(tmop,  tmop_list, tmop_no_fields, "tmop_id_no");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(cumr);
	abc_fclose(exaf);
	abc_fclose(tslb);
	abc_fclose(tslb2);
	abc_fclose(tslh);
	abc_fclose(tsln);
	abc_fclose(tmop);
	abc_dbclose(data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec(comr, comr_list, comr_no_fields, "comr_co_no");

	strcpy(comr_rec.mr_co_no, comm_rec.tco_no);
	cc = find_rec(comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err( cc, "comr", "DBFIND" );

	abc_fclose(comr);
}

int
spec_valid (
 int    field)
{

	if (LCHECK("operator"))
	{
		if (FLD("operator") == NA)
		{
			sprintf(local_rec.oprtr, "%-14.14s", lcl_logname);
			strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
			sprintf(tmop_rec.op_id, "%-14.14s", local_rec.oprtr);
			cc = find_rec(tmop, &tmop_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf(local_rec.oprtr, "%-14.14s", " ");
				sprintf(local_rec.op_desc, "%-40.40s", "ALL OPERATORS");
			}
			else
			{
				sprintf(local_rec.op_desc, "%-40.40s",tmop_rec.op_name);
			}

			FLD("operator") = YES;
			DSP_FLD("operator");
			DSP_FLD("op_desc");
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.oprtr, "%-14.14s", " ");
			sprintf(local_rec.op_desc, "%-40.40s", "ALL OPERATORS");
			DSP_FLD("op_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchTmop(temp_str);
			return(0);
		}
		
		strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
		sprintf(tmop_rec.op_id, "%-14.14s", local_rec.oprtr);
		cc = find_rec(tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Operator not found on file ");*/
			print_mess(ML(mlStdMess168));
			sleep(2);
			clear_mess();
			return(1);
		}

		sprintf(local_rec.op_desc, "%-40.40s", tmop_rec.op_name);
		DSP_FLD("op_desc");

		return(0);
	}

	/*-----------------------
	| Validate Letter Code  |
	-----------------------*/
	if ( LCHECK("code") )
	{
	    	if (SRCH_KEY)
	    	{
			SrchTslh(temp_str);
			return(0);
	    	}

	    	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	    	sprintf(tslh_rec.lh_lett_code, "%-10.10s", local_rec.lett_code);
	    	if ( !find_rec(tslh, &tslh_rec, COMPARISON, "r") )
	    	{
			if (tslh_rec.lh_lett_type[0] != 'L')
			{
				/*print_mess("\007Not a label definition mailer");*/
				print_mess(ML(mlTsMess051));
				sleep(2);	
				clear_mess();
				return(1);
			}

			labels_hor = 0;
			labels_vrt = 0;
			label_head = 0;
			label_tail = 0;
			label_hspc = 40;
			label_vspc = 0;

			if ( read_tsln( tslh_rec.lh_hhlh_hash ) )
			{
				/*print_mess("\007Invalid Mailer Format For Printing Labels");*/
				print_mess(ML(mlTsMess052));
				sleep(2);
				clear_mess();
				return(1);
			}

	    		sprintf(local_rec.lett_desc, 
				"%-40.40s", 
				tslh_rec.lh_lett_desc);
			DSP_FLD("desc");
	    	}
		else
		{
			/*print_mess("\007Mailer Format Not On File");*/
			print_mess(ML(mlTsMess081));
			sleep(2);
			clear_mess();
			return(1);
		}

	    	return(0);
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			/*print_mess("\007 Invalid Printer ");*/
			print_mess(ML(mlStdMess020));
			sleep(2);
			clear_mess();
			return(1);
		}

		return(0);
	}

	return(0);
}

/*----------------------------------------
| Search routine for Letter master file. |
----------------------------------------*/
void
SrchTslh (
 char*  key_val)
{
	work_open();
	save_rec("#Code", "#Description");
	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	sprintf(tslh_rec.lh_lett_code, "%-10.10s", key_val);
	cc = find_rec("tslh", &tslh_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp(tslh_rec.lh_co_no, comm_rec.tco_no) &&
	          !strncmp(tslh_rec.lh_lett_code, key_val, strlen(key_val)))
	{
		if (tslh_rec.lh_lett_type[0] != 'L')
		{
			cc = find_rec("tslh", &tslh_rec, NEXT, "r");
			continue;
		}

		cc = save_rec(tslh_rec.lh_lett_code, tslh_rec.lh_lett_desc);
		if (cc)
			break;

		cc = find_rec("tslh", &tslh_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(tslh_rec.lh_co_no, comm_rec.tco_no);
	sprintf(tslh_rec.lh_lett_code, "%-10.10s", key_val);
	cc = find_rec("tslh", &tslh_rec, COMPARISON, "r");
	if (cc)
		sys_err("Error in tslh During (DBFIND)", cc, PNAME);

	sprintf(local_rec.lett_desc, "%-40.40s", tslh_rec.lh_lett_desc);
}

/*------------------------------------------
| Search routine for Operator master file. |
------------------------------------------*/
void
SrchTmop (
 char*  key_val)
{
	work_open();
	save_rec("#Code", "#Description");
	strcpy(tmop_rec.op_co_no, comm_rec.tco_no);
	sprintf(tmop_rec.op_id, "%-14.14s", key_val);
	cc = find_rec(tmop, &tmop_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp(tmop_rec.op_co_no, comm_rec.tco_no) &&
	          !strncmp(tmop_rec.op_id, key_val, strlen(key_val)))
	{
		cc = save_rec(tmop_rec.op_id, tmop_rec.op_name);
		if (cc)
			break;

		cc = find_rec(tmop, &tmop_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
}

/*--------------------------
| Read Letter Detail Lines |
--------------------------*/
int
read_tsln (
 long   shash)
{
	tsln_rec.ln_hhlh_hash = shash;
	tsln_rec.ln_line_no = 0;
	cc = find_rec(tsln, &tsln_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, tsln, "DBFIND");

	parse (tsln_rec.ln_desc, TRUE, 1, 1);
	if (labels_hor == 0 || labels_vrt == 0)
		return(1);
	
	return(cc);
}

/*--------------------------
| Process customers within |
| selected range           |
--------------------------*/
int
process (void)
{
	int		label_fnd;
	int		x_lab;
	int		y_lab;
	int		lab_no;
	char	lab_no_str[6];
	char	*sptr;
    char    sort_temp[50];

	/*-----------------------------
	| Get label format definition |
	-----------------------------*/
	tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
	tsln_rec.ln_line_no = 0;
	cc = find_rec(tsln, &tsln_rec, COMPARISON, "r");
	if (cc)
		file_err(cc, tsln, "DBFIND");

	parse (tsln_rec.ln_desc, TRUE, 1, 1);
	if (labels_hor == 0 || labels_vrt == 0)
	{
		sys_err("Error in label format during column definition", 
			-1, 
			PNAME);
	}

	dsp_screen("Processing Labels", comm_rec.tco_no, comm_rec.tco_name);

	/*-----------------
	| Open sort file. |
	-----------------*/
	fsort = sort_open("label");

	/*--------------
	| Print labels |
	--------------*/
	label_fnd = FALSE;
	lab_no = 1;
	x_lab = 0;
	y_lab = 0;
	strcpy(tslb_rec.lb_co_no, comm_rec.tco_no);
	sprintf(tslb_rec.lb_operator, "%-14.14s", local_rec.oprtr);
	strcpy(tslb_rec.lb_label_prt, "N");
	tslb_rec.lb_date_sent = 0L;
	tslb_rec.lb_time_sent = 0L;
	tslb_rec.lb_hhcu_hash = 0L;
	cc = find_rec(tslb, &tslb_rec, GTEQ, "r");
	while (!cc && !strcmp(tslb_rec.lb_co_no, comm_rec.tco_no))
	{
		if ((strcmp(tslb_rec.lb_operator, local_rec.oprtr) &&
		     strcmp(local_rec.oprtr, "              ")) ||
		     strcmp(tslb_rec.lb_label_prt, "N"))
		{
			cc = find_rec(tslb, &tslb_rec, NEXT, "r");
			continue;
		}

		sprintf(lab_no_str, "%5d", lab_no++);
		dsp_process("Label No ", lab_no_str);

		cc = find_hash(cumr, &cumr_rec, COMPARISON, "r", tslb_rec.lb_hhcu_hash);
		if (cc)
		{
			cc = find_rec(tslb, &tslb_rec, NEXT, "r");
			continue;
		}

		/*-----------------
		| Save sort data. |
		-----------------*/
        sprintf (sort_temp,
                 "%-6.6s %010ld\n",
			     cumr_rec.cm_dbt_no,
			     cumr_rec.cm_hhcu_hash);
		sort_save (fsort, sort_temp);

		/*---------------------
		| Update tslb record. |
		---------------------*/
		cc = find_hash(tslb2, &tslb_rec, COMPARISON, "u", tslb_rec.lb_hhlb_hash);
		if (cc)
			file_err(cc, tslb, "DBFIND");

		strcpy(tslb_rec.lb_label_prt, "Y");
		cc = abc_update(tslb2, &tslb_rec);
		if (cc)
			file_err(cc, tslb2, "DBUPDATE");

		label_fnd = TRUE;
		cc = find_rec(tslb, &tslb_rec, NEXT, "r");
	}

	if (!label_fnd)
	{
		sort_delete(fsort, "label");
		return(FALSE);
	}

	/*--------------------
	| Process sort file. |
	--------------------*/
	fsort = sort_sort(fsort, "label");
	sptr = sort_read(fsort);
	while (sptr)
	{
		if (x_lab >= labels_hor)
		{
			if (y_lab >= labels_vrt)
			{
				print_tail ();
				y_lab = 0;
			}

			if (y_lab == 0)
				print_head ();

			prt_horiz(x_lab);
			x_lab = 0;
			y_lab++;

			if (y_lab < labels_vrt)
				print_vert();
		
			fflush(fout);
		}

		/*----------------------------
		| Initialise store cust info |
		----------------------------*/
		if (x_lab == 0)
			init_cumr();

		/*-----------------------------
		| Lookup customer information |
		-----------------------------*/
		cc = find_hash(cumr,&cumr_rec, COMPARISON,"r", atol(sptr + 7));
		if (cc)
		{
			sptr = sort_read(fsort);
			continue;
		}

		/*----------------------------
		| Store customer information |
		----------------------------*/
		memcpy ( (char *)&cumr_store[x_lab],
			   (char *)&cumr_rec,
			   sizeof (struct CUMR_REC));

		x_lab++;


		sptr = sort_read(fsort);
	}

	/*------------------------
	| Print remaining labels |
	------------------------*/
	if (x_lab > 0)
	{
		if (y_lab >= labels_vrt)
		{
			print_tail();
			y_lab = 0;
		}

		if (y_lab == 0)
			print_head ();

		prt_horiz(x_lab);
		x_lab = 0;
		y_lab++;
	
		if (y_lab < labels_vrt)
			print_vert();
	}

	/*--------------------
	| Print blank labels |
	--------------------*/
	if (y_lab < labels_vrt)
	{
        int i;
		for (i = y_lab; i < labels_vrt; i++)
		{
			fprintf(fout, ".B%d\n", no_lab_lines);

			if (i + 1 != labels_vrt)
				print_vert();
		}
		fflush(fout);
	}

	/*-------------
	| Print tail. |
	-------------*/
	print_tail();

	sort_delete(fsort, "label");

	return(TRUE);
}

/*------------------------
| Initialise storage for |
| customer records.      |
------------------------*/
void
init_cumr (void)
{	
	int		i;

	for (i = 0; i < labels_hor; i++)
	{
		sprintf (cumr_store[i].cm_dbt_no,       "%-6.6s",   " ");
		cumr_store[i].cm_hhcu_hash = 0L;
		sprintf (cumr_store[i].cm_name,         "%-40.40s", " ");
		sprintf (cumr_store[i].cm_acronym,      "%-9.9s",   " ");
		sprintf (cumr_store[i].cm_ch_addr[0],   "%-40.40s", " ");
		sprintf (cumr_store[i].cm_ch_addr[1],   "%-40.40s", " ");
		sprintf (cumr_store[i].cm_ch_addr[2],   "%-40.40s", " ");
		sprintf (cumr_store[i].cm_contact_name, "%-20.20s", " ");
		sprintf (cumr_store[i].cm_phone_no,     "%-15.15s", " ");
		sprintf (cumr_store[i].cm_fax_no,       "%-15.15s", " ");
		sprintf (cumr_store[i].cm_post_code,    "%-11.11s", " ");
		sprintf (cumr_store[i].cm_area,         "%-2.2s",   " ");
	}
}

/*==================================
| Print letters for leads selected |
==================================*/
void
prt_horiz (
 int    no_labs)
{
	char	parse_str[201];

	static	int		first_label;

	if (no_lab_lines == 0)
		first_label = TRUE;

	/*-------------------------
	| Start at line 1. Line 0 |
	| defines the number of   |
	| columns to be printed.  |
	-------------------------*/
	tsln_rec.ln_hhlh_hash = tslh_rec.lh_hhlh_hash;
	tsln_rec.ln_line_no = 1;
	cc = find_rec(tsln, &tsln_rec, GTEQ, "r");
	while (!cc && tsln_rec.ln_hhlh_hash == tslh_rec.lh_hhlh_hash)
	{
		if (first_label)
			no_lab_lines++;
	
		sprintf(parse_str, "%s", clip( tsln_rec.ln_desc ));
		parse (parse_str, FALSE, labels_hor, no_labs);

		cc = find_rec(tsln, &tsln_rec, NEXT, "r");
	}

	first_label = FALSE;
}

/*------------------
| Print head lines |
------------------*/
void
print_head (void)
{
	/*-------------------------------
	| Open file if not already open |
	-------------------------------*/
	if (!pipe_open)
		open_label();

	fprintf(fout, ".B%d\n", label_head);

	fflush(fout);
}

/*-------------------------------
| Open file to output labels to |
-------------------------------*/
void
open_label (void)
{
	if (!pipe_open)
	{
		if ((fout = popen("pformat", "w")) == 0) 
		{
			sys_err("Error in opening pformat During (POPEN)", 
				errno, 
				PNAME);
		}

		fprintf(fout, ".START00/00/00\n");
		fprintf(fout, ".LP%d\n", local_rec.lpno);
		fprintf(fout, ".OP\n");
		fprintf(fout, ".SO\n");

		fprintf(fout, ".2\n");
		fprintf(fout, ".PI10\n");
		fprintf(fout, ".L132\n");

		pipe_open = TRUE;
	}
}

/*------------------------
| Print vertical spacing |
------------------------*/
void
print_vert (void)
{
	fprintf(fout, ".B%d\n", label_vspc);

	fflush(fout);
}

/*------------------
| Print tail lines |
------------------*/
void
print_tail (void)
{
	fprintf(fout, ".B%d\n", label_tail);

	fflush(fout);
}

/*---------------------------------------
| Substitute data for dot commands      |
| and print no_labs labels horizontally |
---------------------------------------*/
void
parse (
 char*  wrk_prt,
 int    label_def,
 int    num_hor,
 int    no_labs)
{
	int		i;
	int		cmd;
	int		dist_left;
	char	*cptr;
	char	*dptr;
	char	*wk_prt = strdup (wrk_prt);

	for (i = 0; i < num_hor; i++)
	{
		dist_left = label_hspc;
		part_printed = TRUE;

		/*--------------------------------
		| Restore the copy of the format |
		| line for each iteration.       |
		--------------------------------*/
		if (i != 0)
		{
			free(wk_prt);
			wk_prt = strdup (wrk_prt);
		}

		/*-----------------------
		| look for dot command	|
		-----------------------*/
		cptr = strchr (wk_prt, '.');
		dptr = wk_prt;
		while (cptr)
		{
			part_printed = FALSE;

			/*----------------------
			| print line up to now |
			----------------------*/
			*cptr = (char)NULL;
			if (cptr != wk_prt && !label_def)
			{
				part_printed = TRUE;
				fprintf(fout, "%s", dptr);
				dist_left -= strlen(dptr);
			}

			/*----------------------------
			| check if valid dot command |
			----------------------------*/
			cmd = valid_cmd(cptr + 1, label_def);
			if (cmd == -2)
				dptr = cptr + 8;
			else
			{
				if (cmd >= CUR_DAT)
				{
					/*---------------------------------
					| Substitute data for dot command |
					---------------------------------*/
					subst_cmd(cmd, i);
					dist_left -= lbl_dist;

					/*---------------------------------
					| Prepare to look for dot command |
					---------------------------------*/
					dptr = cptr + 8;
				}
				else
				{
					/*-------------------
					| Not a dot command |
					| Just a full stop. |
					-------------------*/
					fprintf (fout, ".");
					dist_left--;
					part_printed = TRUE;

					/*---------------------------------
					| Prepare to look for dot command |
					---------------------------------*/
					dptr = cptr + 1;
				}
			}

			cptr = strchr (dptr, '.');
		}

		/*--------------------
		| print rest of line |
		--------------------*/
		if (part_printed && !label_def)
		{
			if (dptr)
			{
				fprintf(fout, "%s", dptr);
				dist_left -= strlen(dptr);
			}
		}

		/*------------------------------
		| Print inter-label horizontal |
		| spacing if this is NOT the   |
		| last label on the line.      |
		------------------------------*/
		if (i + 1 != num_hor && !label_def)
		{
			if (dist_left > 0)
				fprintf (fout, "%-*.*s", dist_left, dist_left, " ");
		}
	}

	if (!label_def)
		fprintf(fout, "\n");

	free (wk_prt);
}

/*--------------------------------------
| Validate dot commands                |
| Return -1 If dot command is actually |
|           just a full stop.          |
| Return -2 If a label command has     |
|           been processed.            |
--------------------------------------*/
int
valid_cmd (
 char*  wk_str,
 int    label_def)
{
	int		i;

	/*----------------
	| Label commands |
	----------------*/
	if (!strncmp(wk_str, "LBL_", 4))
	{
		if (!strncmp(&wk_str[4], "HZ", 2))
			labels_hor = atoi(wk_str + 6);

		if (!strncmp(&wk_str[4], "VT", 2))
			labels_vrt = atoi(wk_str + 6);

		if (!strncmp(&wk_str[4], "HD", 2))
			label_head = atoi(wk_str + 6);

		if (!strncmp(&wk_str[4], "TL", 2))
			label_tail = atoi(wk_str + 6);

		if (!strncmp(&wk_str[4], "HS", 2))
			label_hspc = atoi(wk_str + 6);

		if (!strncmp(&wk_str[4], "VS", 2))
			label_vspc = atoi(wk_str + 6);

		return(-2);
	}

/***
	if (label_def)
		return(-2);
***/

	/*----------------------------------------
	| Dot command is last character on line. |
	----------------------------------------*/
	if ( !strlen( wk_str ) )
		return( -1 );

	/*---------------------
	| Normal dot commands |
	---------------------*/
	for (i = 0; i < N_CMDS; i++)
	{
		if (!strncmp(wk_str, dot_cmds[i].command, 7))
			return(i);
	}

	return(-1);
}

/*----------------------------------------------
| Substitute valid .commands with actual data. |
----------------------------------------------*/
void
subst_cmd (
 int    cmd,
 int    lab_no)
{
	char	*pr_sptr;

	switch (cmd)
	{
	/*-------------------------------
	| System Date, format dd/mm/yy. |
	-------------------------------*/
	case	CUR_DAT:
		part_printed = TRUE;
		fprintf(fout, "%10.10s" , local_rec.systemDate);
		lbl_dist = 8;
		break;

	/*-------------------
	| Full system date. |
	-------------------*/
	case FUL_DAT:
		part_printed = TRUE;
		pr_sptr = gdate(local_rec.lsystemDate);
		fprintf(fout, "%s", pr_sptr);
		lbl_dist = 17;
		break;

	/*---------------
	| Company Name. |
	---------------*/
	case	CO_NAME:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", comr_rec.mr_co_name);
		lbl_dist = 40;
		break;

	/*--------------------
	| Company Address 1. |
	--------------------*/
	case	CO_ADR1:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", comr_rec.mr_co_adr[0]);
		lbl_dist = 40;
		break;

	/*--------------------
	| Company Address 2. |
	--------------------*/
	case	CO_ADR2:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", comr_rec.mr_co_adr[1]);
		lbl_dist = 40;
		break;

	/*--------------------
	| Company Address 3. |
	--------------------*/
	case	CO_ADR3:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", comr_rec.mr_co_adr[2]);
		lbl_dist = 40;
		break;

	/*------------------
	| Prospect Number. |
	------------------*/
	case	DB_NUMB:
		part_printed = TRUE;
		fprintf(fout, "%-6.6s", cumr_store[lab_no].cm_dbt_no);
		lbl_dist = 6;
		break;

	/*-------------------
	| Prospect Acronym. |
	-------------------*/
	case	DB_ACRO:
		part_printed = TRUE;
		fprintf(fout, "%-9.9s", cumr_store[lab_no].cm_acronym);
		lbl_dist = 9;
		break;

	/*----------------
	| Prospect Name. |
	----------------*/
	case	DB_NAME:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", cumr_store[lab_no].cm_name);
		lbl_dist = 40;
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR1:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", cumr_store[lab_no].cm_ch_addr[0]);
		lbl_dist = 40;
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR2:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", cumr_store[lab_no].cm_ch_addr[1]);
		lbl_dist = 40;
		break;

	/*-------------------
	| Prospect Address. |
	-------------------*/
	case	DB_ADR3:
		part_printed = TRUE;
		fprintf(fout, "%-40.40s", cumr_store[lab_no].cm_ch_addr[2]);
		lbl_dist = 40;
		break;

	/*--------------------------
	| Prospect Contact Name 1. |
	--------------------------*/
	case	DB_CNT1:
		part_printed = TRUE;
		fprintf(fout, "%-20.20s", cumr_store[lab_no].cm_contact_name);
		lbl_dist = 20;
		break;

	/*----------
	| Area No. |
	----------*/
	case	AR_NUMB:
		part_printed = TRUE;
		fprintf(fout, "%2.2s", cumr_store[lab_no].cm_area);
		lbl_dist = 2;
		break;

	/*------------
	| Area Name. |
	------------*/
	case	AR_NAME:
		strcpy(exaf_rec.af_co_no, comm_rec.tco_no);
		sprintf(exaf_rec.af_area_code, "%2.2s", cumr_rec.cm_area);
		cc = find_rec(exaf, &exaf_rec, COMPARISON, "r");
		if (cc)
			sprintf(exaf_rec.af_area, "%-40.40s", " ");

		part_printed = TRUE;
		fprintf(fout, "%-40.40s", exaf_rec.af_area);
		lbl_dist = 40;
		break;

	/*------------
	| Post Code. |
	------------*/
	case	DB_POST:
		part_printed = TRUE;
		fprintf(fout, "%-11.11s", cumr_store[lab_no].cm_post_code);
		lbl_dist = 4;
		break;

	/*---------------
	| Phone Number. |
	---------------*/
	case	PHN_NUM:
		part_printed = TRUE;
		fprintf(fout, "%-15.15s", cumr_store[lab_no].cm_phone_no);
		lbl_dist = 15;
		break;

	/*-------------
	| Fax Number. |
	-------------*/
	case	FAX_NUM:
		part_printed = TRUE;
		fprintf(fout, "%-15.15s", cumr_store[lab_no].cm_fax_no);
		lbl_dist = 15;
		break;

	default:
		break;
	}

	fflush(fout);
}

/*====================================================
| gdate(long-date) returns date in 23 January 1986 . |
====================================================*/
char *
gdate (
 long   cur_date)
{
	int		day,
			mon,
			year;
	char	full_date[18];

	DateToDMY (cur_date, &day, &mon, &year);
	sprintf(full_date, "%2d %s %d", day, mth[mon -1], year);
	sprintf(err_str, "%-17.17s", full_date);
	return (err_str);
}

/*----------------
| Print Heading. |
----------------*/
int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		clear();
	
		rv_pr(ML(mlTsMess053), 24, 0, 1);
		
		move(0,1);
		line(80);

		box(0, 2, 80, 6);
		move(1, 4);
		line(79);
		move(1, 7);
		line(79);

		move(0, 20);
		line(80);
		print_at(21,0, 
			ML(mlStdMess038), 
			comm_rec.tco_no, 
			comm_rec.tco_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
