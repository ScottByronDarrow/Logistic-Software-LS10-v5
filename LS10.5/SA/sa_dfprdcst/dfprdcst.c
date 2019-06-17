/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_dfprdcst.c )                                  |
|  Program Desc  : ( Print Last 12 mnths sales by cat/cust or        )|
|                : ( item/cust.                                      )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sadf,                                 |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 28/05/91         |
|---------------------------------------------------------------------|
|  Date Modified : 24/07/91        | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (17/06/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (27/07/93)      | Modified  by : Simon Dubey.      |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (04/09/97)      | Modified  by  : Ana Marie Tario. |
|                                                                     |
|  Comments      : (24/07/91) - Added time into heading. Company      |
|                : report is no longer broken down by branch.         |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (17/06/93)    : DFT 7440 Mod to have a summary report              |
|  (27/06/93)    : removal of passing arg to shutdown_prog on line 744|
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (04/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                                                                     |
| $Log: dfprdcst.c,v $
| Revision 5.3  2002/07/17 09:57:47  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:31  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:36  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:57  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:37  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:09:28  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: dfprdcst.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_dfprdcst/dfprdcst.c,v 5.3 2002/07/17 09:57:47 scott Exp $";

#include	<ml_sa_mess.h>
#include	<ml_std_mess.h>
#include	<pslscr.h>
#define	MOD     1
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<get_lpno.h>

#define	NO_SRT_FLDS	41

#define	LINE	0
#define	BLNK	1

#define	QTY	0
#define	SAL	1
#define	CST	2

#define	CUST	0
#define	CAT	1
#define	GND	2

#define	BACK	(local_rec.back[0] == 'Y')
#define	ONITE	(local_rec.onight[0] == 'Y')
#define	SUMM 	(local_rec.sum_det[0] == 'S')

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"},
		{"comm_fiscal"},
	};

	int comm_no_fields = 7;

	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
		int	tfiscal;
	} comm_rec;

	/*=============================================
	| Sales Analysis Detail file By Item/Customer |
	=============================================*/
	struct dbview sadf_list[] ={
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
		{"sadf_sal_per1"},
		{"sadf_sal_per2"},
		{"sadf_sal_per3"},
		{"sadf_sal_per4"},
		{"sadf_sal_per5"},
		{"sadf_sal_per6"},
		{"sadf_sal_per7"},
		{"sadf_sal_per8"},
		{"sadf_sal_per9"},
		{"sadf_sal_per10"},
		{"sadf_sal_per11"},
		{"sadf_sal_per12"},
		{"sadf_cst_per1"},
		{"sadf_cst_per2"},
		{"sadf_cst_per3"},
		{"sadf_cst_per4"},
		{"sadf_cst_per5"},
		{"sadf_cst_per6"},
		{"sadf_cst_per7"},
		{"sadf_cst_per8"},
		{"sadf_cst_per9"},
		{"sadf_cst_per10"},
		{"sadf_cst_per11"},
		{"sadf_cst_per12"},
		{"sadf_sman"},
		{"sadf_area"},
	};

	int	sadf_no_fields = 43;

	struct	{
		char	co_no[3];
		char	br_no[3];
		char	year[2];
		long	hhbr_hash;
		long	hhcu_hash;
		float	qty_per[12];
		double	sal_per[12];
		double	cst_per[12];
		char	sman[3];
		char	area[3];
	} sadf_rec, sadf2_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
	};

	int	cumr_no_fields = 7;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_department[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
	} cumr_rec;

	/*====================================
	| Inventory Master File Base Record. |
	====================================*/
	struct dbview inmr_list[] ={
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_alpha_code"},
		{"inmr_maker_no"},
		{"inmr_alternate"},
		{"inmr_class"},
		{"inmr_description"},
		{"inmr_category"},
	};

	int inmr_no_fields = 9;

	struct {
		char	co_no[3];
		char	item_no[17];
		long	hhbr_hash;
		char	alpha_code[17];
		char	maker_no[17];
		char	alternate[17];
		char	_class[2];
		char	description[41];
		char	category[12];
	} inmr_rec, inmr2_rec;

	/*================================
	| External Category File Record. |
	================================*/
	struct dbview excf_list[] ={
		{"excf_co_no"},
		{"excf_cat_no"},
		{"excf_cat_desc"},
	};

	int excf_no_fields = 3;

	struct {
		char	co_no[3];
		char	cat_no[12];
		char	cat_desc[41];
	} excf_rec;

	/*=========================================
	| Establishment/Branch Master File Record |
	=========================================*/
	struct dbview esmr_list[] ={
		{"esmr_co_no"},
		{"esmr_est_no"},
		{"esmr_short_name"},
	};

	int	esmr_no_fields = 3;

	struct	{
		char	co_no[3];
		char	est_no[3];
		char	short_name[16];
	} esmr_rec;

FILE	*fout;
FILE	*fsort;

int	curr_mnth;
int	curr_year;
int	fiscal;
int	first_time = TRUE;
int	printed = FALSE;
int	first_group = TRUE;
int	BY_CO = FALSE;
int	BY_BR = FALSE;
int	BY_CAT = FALSE;
int	BY_ITEM = FALSE;
int	data_found;
int	cat_found;
int	item_found;

char	*_sort_read(FILE *srt_fil);
char	*srt_offset[128];
char	br_no[3];
char	prev_br[3];
char	curr_br[3];
char	curr_group[13];
char	curr_item[17];
char	prev_cust[7];
char	curr_cust[7];
char	old_item_desc[41];

double	C_tot[3][12];
double	cat_tot[3][12];
double	grand_tot[3][12];
double	ytd[3][3];
double	lst_ytd[3][3];
double	pft[14];
float	pc_pft[14];

	char *sadf  = "sadf";
	char *sadf2 = "sadf2";
	char *inmr  = "inmr";
	char *inmr2 = "inmr2";
	char *cumr  = "cumr";
	char *excf  = "excf";
	char *esmr  = "esmr";
	char *data  = "data";
	char *comm  = "comm";

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char 	back[2];
	char	onight[2];
	char 	back_desc[4];
	char	onight_desc[4];
	char	lp_str[3];
	int	lpno;
	char	v_mask[17];
	char	st_prmt[16];
	char	end_prmt[16];
	char	st_class[2];
	char	end_class[2];
	char	lower[17];
	char	lower_desc[41];
	char	upper[17];
	char	upper_desc[41];
	char	st_cust[7];
	char	st_cust_desc[41];
	char	end_cust[7];
	char	end_cust_desc[41];
	char	rep_by[2];
	char	rep_by_desc[8];
	char	sum_det[2];
	char	sum_det_desc[8];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "rep_by",	 4, 14, CHARTYPE,
		"U", "          ",
		" ", "C", "Report By:", " ",
		 NO, NO,  JUSTLEFT, "BC", "", local_rec.rep_by},
	{1, LIN, "rep_by_desc",	 4, 17, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.rep_by_desc},
	{1, LIN, "sum_det",	 4, 55, CHARTYPE,
		"U", "          ",
		" ", "D", "Detail Level :", "S)ummary Or D)etail - Default ",
		 NO, NO,  JUSTLEFT, "DS", "", local_rec.sum_det},
	{1, LIN, "sum_det_desc",	 4, 58, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.sum_det_desc},
	{1, LIN, "st_class",	 6, 14, CHARTYPE,
		"U", "          ",
		" ", "", "Start Class: ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.st_class},
	{1, LIN, "lower",	 7, 14, CHARTYPE,
		local_rec.v_mask, "          ",
		" ", "", local_rec.st_prmt, " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.lower},
	{1, LIN, "lower_desc",	 7, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.lower_desc},
	{1, LIN, "end_class",	 9, 14, CHARTYPE,
		"U", "          ",
		" ", "", "End Class: ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.end_class},
	{1, LIN, "upper",	10, 14, CHARTYPE,
		local_rec.v_mask, "          ",
		" ", "", local_rec.end_prmt, " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.upper},
	{1, LIN, "upper_desc",	10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.upper_desc},
	{1, LIN, "st_cust",	12, 14, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Start Customer:", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.st_cust},
	{1, LIN, "st_cust_desc",	12, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_cust_desc},
	{1, LIN, "end_cust",	13, 14, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "End Customer:", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.end_cust},
	{1, LIN, "end_cust_desc",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_cust_desc},
	{1, LIN, "lpno",	15, 14, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number:", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	16, 14, CHARTYPE,
		"U", "          ",
		" ", "N", "Background    :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "back_desc",	16, 17, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.back_desc},
	{1, LIN, "onight",	17, 14, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight     :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onight_desc",	17, 17, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.onight_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
#include <std_decs.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int run_prog (char *prog_name, char *prog_desc);
void proc_cat (char *brnch_no);
int proc_item (char *brnch_no);
int read_sadf (char *brnch_no);
int store_data (long int hhcu_hash, char *brnch_no);
int valid_cust (long int hhcu_hash);
int proc_sort (void);
int print_line (int line_type);
int print_tot (char *tot_type);
int calc_profit (int p_type);
int calc_ytd (char *year);
void head_output (void);
int categ_heading (void);
int item_heading (void);
int init_array (void);
char *month_name (int n);
char *_sort_read (FILE *srt_fil);
int spec_valid (int field);
void srch_cat (char *key_val);
void srch_item (char *key_val);
int heading (int scn);

	int		envVarDbCo		=	0;
	char	branchNumber[3];
/*=====================================================================
| Main Processing Routine.                                            |
=====================================================================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc != 3 && argc != 9)
	{
		print_at(0,0,mlSaMess743,argv[0]);
		print_at(1,0,mlStdMess036, argv[0]);
		print_at(2,0,mlSaMess732);
		print_at(3,0,mlSaMess733);
		print_at(4,0,mlSaMess713);
		print_at(5,0,mlSaMess714);
		print_at(6,0,mlSaMess715);
		print_at(7,0,mlSaMess722);
		print_at(8,0,mlSaMess716);
        return (EXIT_FAILURE);
	}

	SETUP_SCR( vars );

	if (argc == 9)
	{
		if (argv[6][0] == 'C')
			BY_CO = TRUE;
		else
			BY_BR = TRUE;

		if (argv[7][0] == 'C')
			BY_CAT = TRUE;
		else
			BY_ITEM = TRUE;

		local_rec.lpno = atoi(argv[1]);
		sprintf (local_rec.sum_det, "%-1.1s", argv[8]);
		if (BY_CAT)
		{
			sprintf(local_rec.st_class,"%-1.1s",argv[2]);
			sprintf(local_rec.lower,"%-11.11s",argv[2] + 1);
			sprintf(local_rec.end_class,"%-1.1s",argv[3]);
			sprintf(local_rec.upper,"%-11.11s",argv[3] + 1);
		}
		else
		{
			sprintf(local_rec.lower,"%-16.16s",argv[2]);
			sprintf(local_rec.upper,"%-16.16s",argv[3]);
		}

		sprintf(local_rec.st_cust,"%-6.6s",argv[4]);
		sprintf(local_rec.end_cust,"%-6.6s",argv[5]);
	}
	else
	{
		if (argv[2][0] == 'C')
		{
			BY_CAT = TRUE;
			strcpy(local_rec.st_prmt, "Start Category:");
			strcpy(local_rec.end_prmt,"End Category  :");
			strcpy(local_rec.v_mask, "AAAAAAAAAAA");
			FLD("st_class") = YES;
			FLD("end_class") = YES;
		}
		else
		{
			BY_ITEM = TRUE;
			strcpy(local_rec.st_prmt, "Start Item:");
			strcpy(local_rec.end_prmt,"End Item  :");
			strcpy(local_rec.v_mask, "AAAAAAAAAAAAAAAA");

			/*----------------------
			| Fix up screen layout |
			----------------------*/
			vars[label("lower")].row = 6;
			vars[label("lower_desc")].row = 6;
			vars[label("upper")].row = 7;
			vars[label("upper_desc")].row = 7;
			vars[label("st_cust")].row = 9;
			vars[label("st_cust_desc")].row = 9;
			vars[label("end_cust")].row = 10;
			vars[label("end_cust_desc")].row = 10;
			vars[label("lpno")].row = 12;
			vars[label("back")].row = 13;
			vars[label("back_desc")].row = 13;
			vars[label("onight")].row = 14;
			vars[label("onight_desc")].row = 14;
		}
		
		init_scr();
		set_tty();
		set_masks();
		init_vars(1);
	}

	OpenDB();


	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");
	DateToDMY (comm_rec.tdbt_date, NULL, &curr_mnth, &curr_year);

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;


	if (argc == 9)
	{
		abc_selfield(cumr, "cumr_hhcu_hash");

		sprintf(err_str,"Reading : Sales by %s", (BY_CAT) ? "Category" :"Item");
		dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);

		init_array();
		if (BY_CAT)
		{
			if (BY_CO)
				strcpy (br_no, "  ");
			else
				strcpy (br_no, comm_rec.test_no);

			head_output();
			proc_cat(br_no);
		}
		else
		{
			if (BY_CO)
				strcpy (br_no, "  ");
			else
				strcpy (br_no, comm_rec.test_no);

			head_output();
			proc_item(br_no);

		}
		print_tot("GND");
		sort_delete(fsort,sadf);
		fprintf(fout,".EOF\n");
		pclose(fout);
	
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	else
	{
		prog_exit = FALSE;
		while (!prog_exit)
		{
			/*---------------------
			| Reset control flags |
			---------------------*/
			entry_exit = 0;
			edit_exit = 0;
			prog_exit = 0;
			restart = 0;
			search_ok = 1;
			init_vars(1);		/*  set default values	*/
	
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
	
			if (run_prog (argv[0], argv[1]) == 1)
			{
				return (EXIT_SUCCESS);
			}
			prog_exit = TRUE;
		}
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
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

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec(sadf,sadf_list,sadf_no_fields,"sadf_id_no4");
	abc_alias(sadf2, sadf);
	open_rec(sadf2,sadf_list,sadf_no_fields,"sadf_id_no4");
	open_rec(inmr,inmr_list,inmr_no_fields,"inmr_id_no");
	abc_alias(inmr2, inmr);
	open_rec(inmr2,inmr_list,inmr_no_fields,"inmr_id_no");
	open_rec(cumr,cumr_list,cumr_no_fields,"cumr_id_no3");
	open_rec(excf,excf_list,excf_no_fields,"excf_id_no");
	open_rec(esmr,esmr_list,esmr_no_fields,"esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose(sadf);
	abc_fclose(sadf2);
	abc_fclose(inmr);
	abc_fclose(inmr2);
	abc_fclose(cumr);
	abc_fclose(excf);
	abc_fclose(esmr);
	abc_dbclose(data);
}

int
run_prog (
 char*  prog_name,
 char*  prog_desc)
{
	char	tmp_lower[17];
	char	tmp_upper[17];

	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	
	shutdown_prog ();
	
	if (BY_CAT)
	{
		sprintf(tmp_lower, 
			"%-1.1s%-11.11s", 
			local_rec.st_class, 
			local_rec.lower);

		sprintf(tmp_upper, 
			"%-1.1s%-11.11s", 
			local_rec.end_class, 
			local_rec.upper);
	}
	else
	{
		sprintf(tmp_lower, "%-16.16s", local_rec.lower);
		sprintf(tmp_upper, "%-16.16s", local_rec.upper);
	}

	if (ONITE)
	{
		if (fork() == 0)
        {   /* child */
			execlp("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				tmp_lower,
				tmp_upper,
				local_rec.st_cust,
				local_rec.end_cust,
				local_rec.rep_by,
				(BY_CAT) ? "C" : "I",
				local_rec.sum_det,
				prog_desc,(char *)0);
        }
	}
	else if (BACK)
	{
		if (fork() == 0)
        { /* child */
			execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				tmp_lower,
				tmp_upper,
				local_rec.st_cust,
				local_rec.end_cust,
				local_rec.rep_by,
				(BY_CAT) ? "C" : "I", 
				local_rec.sum_det,
				(char *)0);
        }
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.lp_str,
			tmp_lower,
			tmp_upper,
			local_rec.st_cust,
			local_rec.end_cust,
			local_rec.rep_by,
			(BY_CAT) ? "C" : "I", 
			local_rec.sum_det,
			(char *)0);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

void
proc_cat (
 char*  brnch_no)
{
	int	first_time = TRUE;
	char	tmp_group[13];

	fsort = sort_open(sadf);
	
	abc_selfield(inmr, "inmr_id_no_3");

	cat_found = FALSE;

	strcpy(inmr_rec.co_no, comm_rec.tco_no);
	sprintf(inmr_rec._class, "%-1.1s", local_rec.st_class);
	sprintf(inmr_rec.category, "%-11.11s", local_rec.lower);
	sprintf(inmr_rec.item_no, "%-16.16s", " ");
	/*------------------------------------
	| Process all inmr records that fall |
	| within the _class/category range    |
	| specified by the user		     |
	------------------------------------*/
	cc = find_rec(inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.co_no, comm_rec.tco_no) &&
	       strncmp(inmr_rec._class, local_rec.end_class, 1) <= 0)
	{
		/*-----------------------
		| Category out of range |
		-----------------------*/
		if ((strcmp(inmr_rec._class,local_rec.end_class)> 0) ||
		    (strcmp(inmr_rec.category,local_rec.upper) > 0))
			break;

		if (first_time)
		{
			sprintf(curr_group, 
				"%-1.1s%-11.11s", 
				inmr_rec._class, 
				inmr_rec.category);
			first_time = FALSE;
		}

		sprintf(tmp_group, 
			"%-1.1s%-11.11s", 
			inmr_rec._class, 
			inmr_rec.category);

		if (strcmp(tmp_group, curr_group))
		{
			if (!first_group && !SUMM)
				fprintf(fout, ".PA\n");

			categ_heading();

			proc_sort();

			sprintf(curr_group, 
				"%-1.1s%-11.11s", 
				inmr_rec._class, 
				inmr_rec.category);

			first_group = FALSE;
			cat_found = FALSE;
		}

		read_sadf(brnch_no);
			
		cc = find_rec(inmr, &inmr_rec, NEXT, "r");
	}

	if (cat_found)
	{
		if (!first_group && !SUMM)
			fprintf(fout, ".PA\n");

		categ_heading();
		proc_sort();

		first_group = FALSE;
	}

	abc_selfield(inmr, "inmr_id_no");
}

int
proc_item (
 char*  brnch_no)
{
	int	frst_time = TRUE;
	int	fst_time = TRUE;

	fsort = sort_open(sadf);
	
	item_found = FALSE;

	strcpy(inmr_rec.co_no, comm_rec.tco_no);
	sprintf(inmr_rec.item_no, "%-16.16s", local_rec.lower);
	/*------------------------------------
	| Process all inmr records that fall |
	| within the class/category range    |
	| specified by the user		     |
	------------------------------------*/
	cc = find_rec(inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.co_no, comm_rec.tco_no) &&
	       strcmp(inmr_rec.item_no, local_rec.upper) <= 0)
	{
		if (fst_time)
		{
			sprintf(curr_item, "%-16.16s", inmr_rec.item_no);
			sprintf(old_item_desc, "%-40.40s", inmr_rec.description);
			fst_time = FALSE;
		}

		if (strcmp(inmr_rec.item_no, curr_item))
		{
			if ( !frst_time && !SUMM)
				fprintf(fout, ".PA\n");
			else
				frst_time = FALSE;

			item_heading();
			proc_sort();

			sprintf(curr_item, "%-16.16s", inmr_rec.item_no);
			sprintf(old_item_desc, "%-40.40s", inmr_rec.description);

			item_found = FALSE;
		}

		read_sadf(brnch_no);

		cc = find_rec(inmr, &inmr_rec, NEXT, "r");
	}

	if (item_found)
	{
		if ( !frst_time && !SUMM)
			fprintf(fout, ".PA\n");

		item_heading();
		proc_sort();
		first_group = FALSE;
	}

	return (EXIT_SUCCESS);
}

int
read_sadf (
 char*  brnch_no)
{
	int	i;
	int	first_time;
	int	lcl_cc;
	long	curr_hhcu = 0L;

	/*---------------------------
	| Read all sadf records for |
	| current hhbr hash and year|
	| equal to "C"		    |
	---------------------------*/
	sadf_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sadf_rec.hhcu_hash = 0L;
	first_time = TRUE;
	data_found = FALSE;

	lcl_cc = find_rec(sadf, &sadf_rec, GTEQ, "r");
	while (!lcl_cc && sadf_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{

	       	if ((strcmp(sadf_rec.br_no, brnch_no) &&
	            strcmp(brnch_no, "  ")) ||
	            strcmp(sadf_rec.year, "C"))
		{
			lcl_cc = find_rec(sadf, &sadf_rec, NEXT, "r");
			continue;
		}

		if (sadf_rec.hhcu_hash != curr_hhcu)
		{
			if (!valid_cust(sadf_rec.hhcu_hash))
			{
				lcl_cc = find_rec(sadf, &sadf_rec, NEXT, "r");
				continue;
			}
		}

		data_found = TRUE;

		if (first_time)
		{
			curr_hhcu = sadf_rec.hhcu_hash;
			first_time = FALSE;
		}

		if (sadf_rec.hhcu_hash != curr_hhcu)
		{
			store_data(curr_hhcu, brnch_no);
			curr_hhcu = sadf_rec.hhcu_hash;
		}

		/*--------------------------------
		| Add to totals for current hhbr |
		| current hhcu			 |
		--------------------------------*/
		for (i = 0; i < 12; i++)
		{
			C_tot[QTY][i] += sadf_rec.qty_per[i];
			C_tot[SAL][i] += sadf_rec.sal_per[i];
			C_tot[CST][i] += sadf_rec.cst_per[i];
		}
		
		lcl_cc = find_rec(sadf, &sadf_rec, NEXT, "r");
	}

	if (data_found)
		store_data(curr_hhcu, brnch_no);

	if (BY_CAT)
		dsp_process("Category :", inmr_rec.category);
	else
		dsp_process("Item :", inmr_rec.item_no);

	return (EXIT_SUCCESS);
}

int
store_data (
 long int   hhcu_hash,
 char*      brnch_no)
{
	int	i;
	int	lcl_cc;
	char	data_str[1000];

	lst_ytd[QTY][CUST] = 0.00;
	lst_ytd[SAL][CUST] = 0.00;
	lst_ytd[CST][CUST] = 0.00;

	cc = find_hash(cumr, &cumr_rec, COMPARISON, "r", hhcu_hash);
	if (cc)
		file_err (cc, cumr, "DBFIND");

	/*---------------------------
	| Read all sadf records for |
	| current hhbr hash and year|
	| equal to "L"		    |
	---------------------------*/
	sadf2_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sadf2_rec.hhcu_hash = hhcu_hash;

	lcl_cc = find_rec(sadf2, &sadf2_rec, GTEQ, "r");
	while (!lcl_cc && 
	       sadf2_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       sadf2_rec.hhcu_hash == hhcu_hash)
	{
		if ((strcmp(sadf2_rec.br_no, brnch_no) &&
		    strcmp(brnch_no, "  ")) ||
		    strcmp(sadf2_rec.year, "L"))
		{
			lcl_cc = find_rec(sadf2, &sadf2_rec, NEXT, "r");
			continue;
		}

		calc_ytd("L");

		lcl_cc = find_rec(sadf2, &sadf2_rec, NEXT, "r");
	}

	sprintf(data_str, 
		"%1.1s%c%-6.6s%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f %ld\n",
		inmr_rec._class, 1,
		cumr_rec.cm_dbt_no, 1,
		C_tot[QTY][0],   1,
		C_tot[QTY][1],   1,
		C_tot[QTY][2],   1,
		C_tot[QTY][3],   1,
		C_tot[QTY][4],   1,
		C_tot[QTY][5],   1,
		C_tot[QTY][6],   1,
		C_tot[QTY][7],   1,
		C_tot[QTY][8],   1,
		C_tot[QTY][9],   1,
		C_tot[QTY][10],  1,
		C_tot[QTY][11],  1,
		C_tot[SAL][0],   1,
		C_tot[SAL][1],   1,
		C_tot[SAL][2],   1,
		C_tot[SAL][3],   1,
		C_tot[SAL][4],   1,
		C_tot[SAL][5],   1,
		C_tot[SAL][6],   1,
		C_tot[SAL][7],   1,
		C_tot[SAL][8],   1,
		C_tot[SAL][9],   1,
		C_tot[SAL][10],  1,
		C_tot[SAL][11],  1,
		C_tot[CST][0],   1,
		C_tot[CST][1],   1,
		C_tot[CST][2],   1,
		C_tot[CST][3],   1,
		C_tot[CST][4],   1,
		C_tot[CST][5],   1,
		C_tot[CST][6],   1,
		C_tot[CST][7],   1,
		C_tot[CST][8],   1,
		C_tot[CST][9],   1,
		C_tot[CST][10],  1,
		C_tot[CST][11],  1,
		lst_ytd[QTY][CUST], 1,
		lst_ytd[SAL][CUST], 1,
		lst_ytd[CST][CUST],
inmr_rec.hhbr_hash);
	sort_save(fsort, data_str);

	for (i = 0; i < 12; i++)
	{
		C_tot[QTY][i] = 0.00;
		C_tot[SAL][i] = 0.00;
		C_tot[CST][i] = 0.00;
	}

	lst_ytd[QTY][CUST] = 0.00;
	lst_ytd[SAL][CUST] = 0.00;
	lst_ytd[CST][CUST] = 0.00;

	cat_found = TRUE;
	item_found = TRUE;
	return (TRUE);
}

int
valid_cust (
 long int   hhcu_hash)
{
	/*-------------
	| Lookup cumr |
	-------------*/
	cc = find_hash(cumr, &cumr_rec, COMPARISON, "r", hhcu_hash);
	if (cc || 
	    strcmp(cumr_rec.cm_dbt_no, local_rec.st_cust) < 0 ||
	    strcmp(cumr_rec.cm_dbt_no, local_rec.end_cust) > 0)
		return(FALSE);

	return (TRUE);
}

int
proc_sort (void)
{
	char	*_sort_read(FILE *srt_fil);
	char	*sptr;
	int	i;

	abc_selfield(cumr, "cumr_id_no3");
	printed = FALSE;
	first_time = TRUE;

	fsort = sort_sort(fsort,sadf);
	sptr = _sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(curr_cust,"%-6.6s",srt_offset[1]);

		if (BY_CAT)
			dsp_process("Category :",curr_group + 1);
		else
			dsp_process("Item :",curr_item);

		if (first_time)
		{
			first_time = FALSE;
			strcpy(prev_cust,curr_cust);
		}

		if (strcmp(curr_cust,prev_cust))
			print_tot("CUST");

		for (i = 0; i < 12; i++)
		{
			C_tot[QTY][i] += atof(srt_offset[i + 2]);
			C_tot[SAL][i] += atof(srt_offset[i + 14]);
			C_tot[CST][i] += atof(srt_offset[i + 26]);
		}

		lst_ytd[QTY][CUST] += atof(srt_offset[38]);
		lst_ytd[SAL][CUST] += atof(srt_offset[39]);
		lst_ytd[CST][CUST] += atof(srt_offset[40]);

		strcpy(prev_cust,curr_cust);

		sptr = _sort_read(fsort);
	}

	if (printed)
	{
		if (BY_CAT)
			print_tot("CAT");
		else
			print_tot("ITEM");
	}

	sort_delete(fsort,sadf);
	fsort = sort_open(sadf);

	abc_selfield(cumr, "cumr_hhcu_hash");
	return (EXIT_SUCCESS);
}

int
print_line (
 int    line_type)
{
	if (line_type == LINE)
	{
		fprintf(fout, "!---------------!---------!---------!---------");
		fprintf(fout, "!---------!---------!---------!---------");
		fprintf(fout, "!---------!---------!---------!---------");
		fprintf(fout, "!----------!----------!\n");
	}
	else
	{
		fprintf(fout, "!               !         !         !         ");
		fprintf(fout, "!         !         !         !         ");
		fprintf(fout, "!         !         !         !         ");
		fprintf(fout, "!          !          !\n");
	}
	return (EXIT_SUCCESS);
}

int
print_tot (
 char*  tot_type)
{
	int	i;
	int	j;
	int	lcl_cc;
	char	tmp_desc[81];

	/*----------------------
	| Print Customer Total |
	----------------------*/
	if (!strcmp(tot_type, "CUST") ||
	    !strcmp(tot_type, "CAT") ||
	    !strcmp(tot_type, "ITEM"))
	{
		calc_ytd("C");

		/*---------------
		| Get Cust Name |
		---------------*/
		if (!SUMM)
		{
			strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
			sprintf(cumr_rec.cm_dbt_no, "%-6.6s", prev_cust);
			lcl_cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
			if (lcl_cc)
				sprintf(cumr_rec.cm_name, "%-40.40s", "Unknown Customer");

			expand(tmp_desc, cumr_rec.cm_name);
			fprintf(fout, ".LRP7\n");
			fprintf(fout, 
				"! %-6.6s  %-80.80s      !         !         !         !          !          !\n", 
				prev_cust, 
				tmp_desc);

			for (j = QTY; j <= CST; j++)
			{
				switch (j)
				{
				case QTY:
					fprintf(fout, "! QTY  ");
					break;
				case SAL:
					fprintf(fout, "! VALUE");
					break;
				case CST:
					fprintf(fout, "! COST ");
					break;
				}

				for (i = curr_mnth; i < 12; i++)
					fprintf(fout, " %8.0f!", C_tot[j][i]);

				for (i = 0; i < curr_mnth; i++)
					fprintf(fout, " %8.0f!", C_tot[j][i]);

				fprintf( fout, 
					" %9.0f! %9.0f!\n",
					ytd[j][CUST],
					lst_ytd[j][CUST]);
			}
		}

		calc_profit(CUST);

		if (!SUMM)
			print_line(LINE);

		for (i = 0; i < 12; i++)
		{
			cat_tot[QTY][i] += C_tot[QTY][i];
			cat_tot[SAL][i] += C_tot[SAL][i];
			cat_tot[CST][i] += C_tot[CST][i];
			C_tot[QTY][i] = 0.00;
			C_tot[SAL][i] = 0.00;
			C_tot[CST][i] = 0.00;
		}

		ytd[QTY][CAT] += ytd[QTY][CUST];
		ytd[SAL][CAT] += ytd[SAL][CUST];
		ytd[CST][CAT] += ytd[CST][CUST];

		lst_ytd[QTY][CAT] += lst_ytd[QTY][CUST];
		lst_ytd[SAL][CAT] += lst_ytd[SAL][CUST];
		lst_ytd[CST][CAT] += lst_ytd[CST][CUST];

		lst_ytd[QTY][CUST] = 0.00;
		lst_ytd[SAL][CUST] = 0.00;
		lst_ytd[CST][CUST] = 0.00;
	}

	/*----------------------
	| Print Category Total |
	----------------------*/
	if (!strcmp(tot_type, "CAT") ||
	    !strcmp(tot_type, "ITEM"))
	{
		fprintf(fout, ".LRP7\n");
		if (BY_CAT)
		{
			expand(tmp_desc, excf_rec.cat_desc);
			fprintf(fout, 
				"! TOTAL FOR CATEGORY %-1.1s %-11.11s  %-80.80s!         !          !          !\n", 
				curr_group, 
				curr_group + 1, 
				tmp_desc);
		}
		else
		{
			expand(tmp_desc, old_item_desc);
			fprintf(fout, 
				"! TOTAL FOR ITEM  %-16.16s  %-80.80s!         !          !          !\n", 
				curr_item, 
				tmp_desc);
		}

		for (j = QTY; j <=CST; j++)
		{
			switch (j)
			{
			case QTY:
				fprintf(fout, "! QTY  ");
				break;
			case SAL:
				fprintf(fout, "! VALUE");
				break;
			case CST:
				fprintf(fout, "! COST ");
				break;
			}

			for (i = curr_mnth; i < 12; i++)
				fprintf(fout, " %8.0f!", cat_tot[j][i]);

			for (i = 0; i < curr_mnth; i++)
				fprintf(fout, " %8.0f!", cat_tot[j][i]);

			fprintf( fout, 
				" %9.0f! %9.0f!\n",
				ytd[j][CAT],
				lst_ytd[j][CAT]);
		}

		calc_profit(CAT);

		print_line(LINE);

		for (i = 0; i < 12; i++)
		{
			grand_tot[QTY][i] += cat_tot[QTY][i];
			grand_tot[SAL][i] += cat_tot[SAL][i];
			grand_tot[CST][i] += cat_tot[CST][i];

			cat_tot[QTY][i] = 0.00;
			cat_tot[SAL][i] = 0.00;
			cat_tot[CST][i] = 0.00;
		}

		ytd[QTY][GND] += ytd[QTY][CAT];
		ytd[SAL][GND] += ytd[SAL][CAT];
		ytd[CST][GND] += ytd[CST][CAT];

		lst_ytd[QTY][GND] += lst_ytd[QTY][CAT];
		lst_ytd[SAL][GND] += lst_ytd[SAL][CAT];
		lst_ytd[CST][GND] += lst_ytd[CST][CAT];

		ytd[QTY][CAT] = 0.00;
		ytd[SAL][CAT] = 0.00;
		ytd[CST][CAT] = 0.00;

		lst_ytd[QTY][CAT] = 0.00;
		lst_ytd[SAL][CAT] = 0.00;
		lst_ytd[CST][CAT] = 0.00;
	}

	/*-------------------
	| Print Grand Total |
	-------------------*/
	if (!strcmp(tot_type, "GND"))
	{
		fprintf(fout, ".LRP7\n");
		fprintf(fout, "! GRAND TOTAL%-135.135s!\n", " ");

		for (j = QTY; j <= CST; j++)
		{
			switch (j)
			{
			case QTY:
				fprintf(fout, "! QTY  ");
				break;
			case SAL:
				fprintf(fout, "! VALUE");
				break;
			case CST:
				fprintf(fout, "! COST ");
				break;
			}

			for (i = curr_mnth; i < 12; i++)
				fprintf(fout, "%9.0f!", grand_tot[j][i]);

			for (i = 0; i < curr_mnth; i++)
				fprintf(fout, "%9.0f!", grand_tot[j][i]);

			fprintf( fout, 
				"%10.0f!%10.0f!\n",
				ytd[j][GND],
				lst_ytd[j][GND]);
		}
		calc_profit(GND);

		print_line(LINE);
	}
	return (EXIT_SUCCESS);
}

int
calc_profit (
 int    p_type)
{
	int	i;

	for (i = 0; i < 12; i++)
	{
		switch(p_type)
		{
		case CUST:
			pft[i] = C_tot[SAL][i] - C_tot[CST][i];
			if (C_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / C_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;

		case CAT:
			pft[i] = cat_tot[SAL][i] - cat_tot[CST][i];
			if (cat_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / cat_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;

		case GND:
			pft[i] = grand_tot[SAL][i] - grand_tot[CST][i];
			if (grand_tot[SAL][i] != 0.00)
				pc_pft[i] = (pft[i] / grand_tot[SAL][i] * 100);
			else
				pc_pft[i] = -99.00;
			break;
		}
	}

	pft[12] = ytd[SAL][p_type] - ytd[CST][p_type];
	if (ytd[SAL][p_type] != 0.00)
		pc_pft[12] = (pft[12] / ytd[SAL][p_type] * 100);
	else
		pc_pft[12] = -99.00;

	pft[13] = lst_ytd[SAL][p_type] - lst_ytd[CST][p_type];
	if (lst_ytd[SAL][p_type] != 0.00)
		pc_pft[13] = (pft[13] / lst_ytd[SAL][p_type] * 100);
	else
		pc_pft[13] = -99.00;

	/*--------------------
	| Print profit value |
	--------------------*/
	if (!SUMM || (SUMM && p_type != CUST))
	{
		fprintf(fout, "! PFT  ");
		for (i = curr_mnth; i < 12; i++)
			fprintf(fout, "%9.0f!", pft[i]);

		for (i = 0; i < curr_mnth; i++)
			fprintf(fout, "%9.0f!", pft[i]);

		fprintf( fout, 
			" %9.0f! %9.0f!\n",
			pft[12],
			pft[13]);
		/*----------------------
		| Print profit percent |
		----------------------*/
		fprintf(fout, "! %% PFT");
		for (i = curr_mnth; i < 12; i++)
			fprintf(fout, "%9.2f!", pc_pft[i]);

		for (i = 0; i < curr_mnth; i++)
			fprintf(fout, "%9.2f!", pc_pft[i]);

		fprintf( fout, 
			" %9.2f! %9.2f!\n",
			pc_pft[12],
			pc_pft[13]);	

	}
	return (EXIT_SUCCESS);
}

int
calc_ytd (
 char*  year)
{
	int	i;

	if (year[0] == 'C')
	{
		ytd[QTY][CUST] = 0.00;
		ytd[SAL][CUST] = 0.00;
		ytd[CST][CUST] = 0.00;
	}

	if (curr_mnth <= fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][CUST] += C_tot[QTY][i];
				ytd[SAL][CUST] += C_tot[SAL][i];
				ytd[CST][CUST] += C_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][CUST] += sadf2_rec.qty_per[i];
				lst_ytd[SAL][CUST] += sadf2_rec.sal_per[i];
				lst_ytd[CST][CUST] += sadf2_rec.cst_per[i];
			}
		}

		for (i = 0; i < curr_mnth; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][CUST] += C_tot[QTY][i];
				ytd[SAL][CUST] += C_tot[SAL][i];
				ytd[CST][CUST] += C_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][CUST] += sadf2_rec.qty_per[i];
				lst_ytd[SAL][CUST] += sadf2_rec.sal_per[i];
				lst_ytd[CST][CUST] += sadf2_rec.cst_per[i];
			}
		}

	}
	else
	{
		for (i = fiscal; i < curr_mnth; i++)
		{
			if (year[0] == 'C')
			{
				ytd[QTY][CUST] += C_tot[QTY][i];
				ytd[SAL][CUST] += C_tot[SAL][i];
				ytd[CST][CUST] += C_tot[CST][i];
			}
			else
			{
				lst_ytd[QTY][CUST] += sadf2_rec.qty_per[i];
				lst_ytd[SAL][CUST] += sadf2_rec.sal_per[i];
				lst_ytd[CST][CUST] += sadf2_rec.cst_per[i];
			}
		}
	}

	return (EXIT_SUCCESS);
}

void
head_output (void)
{
	char	*month_name(int n);
	int	i;

	if ((fout = popen("pformat","w")) == NULL)
	{
		sys_err("Error in pformat during (POPEN)",errno,PNAME);
	}
		
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".11\n");
	fprintf(fout,".L158\n");
	
	fprintf(fout,
		".ESales Analysis By %s For Last 12 Months (%s)\n",
		(BY_CAT) ? "Category" : "Item",
		(SUMM) ? "Summary" : "Detail");

	fprintf(fout,
		".ECOMPANY : %s - %s\n",
		comm_rec.tco_no,
		clip(comm_rec.tco_name));

	if (BY_CO)
		fprintf(fout,".EALL BRANCHES\n");
	else
	{
		strcpy(esmr_rec.co_no, comm_rec.tco_no);
		strcpy(esmr_rec.est_no, comm_rec.test_no);
		cc = find_rec(esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, esmr, "DBFIND");

		fprintf(fout,
			".EBRANCH : %-2.2s - %-15.15s\n",
			br_no,
			esmr_rec.short_name);
	}
		
	fprintf(fout,".EFOR THE MONTH OF %-9.9s\n",month_name(curr_mnth - 1));

	fprintf(fout,".B1\n");
	
	if ( BY_CAT )
		sprintf(err_str, 
		       "For Categories %-1.1s  %-11.11s to %-1.1s  %-11.11s", 
			local_rec.st_class, 
			local_rec.lower, 
			local_rec.end_class, 
			local_rec.upper);
	else
		sprintf(err_str, 
			"For Items %-1.1s  %-16.16s to %-1.1s  %-16.16s", 
			local_rec.st_class, 
			local_rec.lower, 
			local_rec.end_class, 
			local_rec.upper);

	fprintf(fout,".E%s\n",err_str);

	fprintf(fout,"==================================================");
	fprintf(fout,"==================================================");
	fprintf(fout,"=================================================\n");

	fprintf(fout,"!      ");
	for (i = curr_mnth; i < 12; i ++)
	{
		fprintf(fout, " %-3.3s %4d!", month_name(i), curr_year - 1);
	}
	for (i = 0; i < curr_mnth - 1; i ++)
	{
		fprintf(fout, " %-3.3s %4d!", month_name(i), curr_year);
	}
	fprintf(fout, " %-3.3s %4d!   YTD    !   LYTD   !\n", 
					 month_name(curr_mnth - 1), curr_year);

	fprintf(fout,"!---------------!---------!---------!---------");
	fprintf(fout,"!---------!---------!---------!---------");
	fprintf(fout,"!---------!---------!---------!---------");
	fprintf(fout,"!----------!----------!\n");

	fprintf(fout,".R=================================================");
	fprintf(fout,"===================================================");
	fprintf(fout,"=================================================\n");
}

int
categ_heading (void)
{
	char tmp_desc[81];
	int	lcl_cc;

	/*--------------------------
	| Get category description |
	--------------------------*/
	strcpy(excf_rec.co_no, comm_rec.tco_no);
	sprintf(excf_rec.cat_no, "%-11.11s", curr_group + 1);
	lcl_cc = find_rec(excf, &excf_rec, COMPARISON, "r");
	if (lcl_cc)
		sprintf(excf_rec.cat_desc, "%-40.40s", "Unknown Category");

	expand(tmp_desc, excf_rec.cat_desc);
	if (!SUMM)
		fprintf(fout, 
			"! %-1.1s %-11.11s  %-80.80s         !         !         !          !          !\n", 
			curr_group, 
			curr_group + 1, 
			tmp_desc);

	return (EXIT_SUCCESS);
}

int
item_heading (void)
{
	char tmp_desc[81];
	int	lcl_cc;

	/*----------------------
	| Get item description |
	----------------------*/
	strcpy(inmr2_rec.co_no, comm_rec.tco_no);
	sprintf(inmr2_rec.item_no, "%-16.16s", curr_item);
	lcl_cc = find_rec(inmr2, &inmr2_rec, COMPARISON, "r");
	if (lcl_cc)
		sprintf(inmr2_rec.description,"%-40.40s", "Unknown Item");

	expand(tmp_desc, inmr2_rec.description);
	if (!SUMM)
	{
		fprintf(fout, ".LRP9\n");
		fprintf(fout, 
			"! %-16.16s  %-80.80s      !         !         !          !          !\n", 
			curr_item, 
			tmp_desc);

	}
	return (EXIT_SUCCESS);
}

int
init_array (void)
{
	int	i;
	int	j;

	for (i = 0; i < 12; i++)
	{
		C_tot[QTY][i] = 0.00;
		C_tot[SAL][i] = 0.00;
		C_tot[CST][i] = 0.00;

		cat_tot[QTY][i] = 0.00;
		cat_tot[SAL][i] = 0.00;
		cat_tot[CST][i] = 0.00;

		grand_tot[QTY][i] = 0.00;
		grand_tot[SAL][i] = 0.00;
		grand_tot[CST][i] = 0.00;
	}
	
	for (j = CUST; j < GND; j++)
	{
		ytd[QTY][j] = 0.00;
		ytd[SAL][j] = 0.00;
		ytd[CST][j] = 0.00;
		lst_ytd[QTY][j] = 0.00;
		lst_ytd[SAL][j] = 0.00;
		lst_ytd[CST][j] = 0.00;
	}

	return (EXIT_SUCCESS);
}

char*
month_name (
 int    n)
{
	static char *name[] = {
		"JANUARY  ",
		"FEBRUARY ",
		"MARCH    ",
		"APRIL    ",
		"MAY      ",
		"JUNE     ",
		"JULY     ",
		"AUGUST   ",
		"SEPTEMBER",
		"OCTOBER  ",
		"NOVEMBER ",
		"DECEMBER ",
		"*********"
	};
	return ((n >= 0 && n <= 11) ? name[n] : name[12]);
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char*
_sort_read (
 FILE*  srt_fil)
{
	char	*sptr;
	char	*tptr;
	int	fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset[0] = sptr;

	tptr = sptr;
	while (fld_no < NO_SRT_FLDS)
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
spec_valid (
 int    field)
{
	if (LCHECK("rep_by"))
	{
		if (local_rec.rep_by[0] == 'C')
			strcpy(local_rec.rep_by_desc, "Company");
		else
			strcpy(local_rec.rep_by_desc, "Branch ");

		DSP_FLD("rep_by_desc");

		return(0);
	}

	if (LCHECK("sum_det"))
	{
		if (local_rec.sum_det[0] == 'S')
			strcpy(local_rec.sum_det_desc, "Summary");
		else
			strcpy(local_rec.sum_det_desc, "Detail ");

		DSP_FLD("sum_det_desc");

		return(0);
	}

	if (LCHECK("lower") || LCHECK("upper"))
	{
		if (dflt_used)
		{
			if (LCHECK("lower"))
			{
				if (BY_CAT)
				{
					strcpy(local_rec.lower, "           ");
					sprintf(local_rec.lower_desc, "%-40.40s", "First Category");
				}
				else
				{
					strcpy(local_rec.lower, "                ");
					sprintf(local_rec.lower_desc, "%-40.40s", "First Item");
				}
				DSP_FLD("lower");
				DSP_FLD("lower_desc");
				return(0);
			}
			else
			{
				if (BY_CAT)
				{
					strcpy(local_rec.upper, "~~~~~~~~~~~");
					sprintf(local_rec.upper_desc, "%-40.40s", "Last Category");
				}
				else
				{
					strcpy(local_rec.upper, "~~~~~~~~~~~~~~~~");
					sprintf(local_rec.upper_desc, "%-40.40s", "Last Item");
				}
				DSP_FLD("upper");
				DSP_FLD("upper_desc");
				return(0);
			}
		}

		if (SRCH_KEY)
		{
			if (BY_CAT)
				srch_cat(temp_str);
			else
				srch_item(temp_str);

			return(0);
		}

		if (BY_CAT)
		{
			strcpy(excf_rec.co_no, comm_rec.tco_no);
			sprintf(excf_rec.cat_no, "%-11.11s", temp_str);
			cc = find_rec(excf, &excf_rec, COMPARISON, "r");
			if (cc)
			{
				/*print_mess("\007 Category Not Found On File");*/
				print_mess(ML(mlStdMess004));
				sleep(2);
				clear_mess();
				return(1);
			}
			else
			{
				if (LCHECK("lower"))
					sprintf(local_rec.lower_desc, "%-40.40s", excf_rec.cat_desc);
				else
					sprintf(local_rec.upper_desc, "%-40.40s", excf_rec.cat_desc);
			}

			if (LCHECK("lower"))
				DSP_FLD("lower_desc");
			else
				DSP_FLD("upper_desc");
			return(0);
		}

		if (BY_ITEM)
		{
			strcpy(inmr_rec.co_no, comm_rec.tco_no);
			sprintf(inmr_rec.item_no, "%-16.16s", temp_str);
			cc = find_rec(inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				/*print_mess("\007 Item Not Found On File");*/
				print_mess(ML(mlStdMess001));
				sleep(2);
				clear_mess();
				return(1);
			}
			else
			{
				if (LCHECK("lower"))
					sprintf(local_rec.lower_desc, "%-40.40s", inmr_rec.description);
				else
					sprintf(local_rec.upper_desc, "%-40.40s", inmr_rec.description);
			}

			if (LCHECK("lower"))
				DSP_FLD("lower_desc");
			else
				DSP_FLD("upper_desc");
			return(0);
		}
		return(0);
	}

	if (LCHECK("st_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.st_cust, "      ");
			sprintf(local_rec.st_cust_desc, "%-40.40s", "First Customer");
			DSP_FLD("st_cust");
			DSP_FLD("st_cust_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
		sprintf(cumr_rec.cm_dbt_no, "%-6.6s", pad_num(temp_str));
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Customer Not Found On File ");*/
			print_mess(ML(mlStdMess021));
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.st_cust, "%-6.6s",cumr_rec.cm_dbt_no);
			sprintf(local_rec.st_cust_desc, "%-40.40s", cumr_rec.cm_name);
			DSP_FLD("st_cust");
			DSP_FLD("st_cust_desc");
			return(0);
		}
	}

	if (LCHECK("end_cust"))
	{
		if (dflt_used)
		{
			strcpy(local_rec.end_cust, "~~~~~~");
			sprintf(local_rec.end_cust_desc, "%-40.40s", "Last Customer");
			DSP_FLD("end_cust");
			DSP_FLD("end_cust_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no, comm_rec.tco_no);
		sprintf(cumr_rec.cm_dbt_no, "%-6.6s", pad_num(temp_str));
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			/*print_mess("\007 Customer Not Found On File ");*/
			print_mess(ML(mlStdMess021));
			sleep(2);
			clear_mess();
			return(1);
		}
		else
		{
			sprintf(local_rec.end_cust,"%-6.6s",cumr_rec.cm_dbt_no);
			sprintf(local_rec.end_cust_desc, "%-40.40s", cumr_rec.cm_name);
			DSP_FLD("end_cust");
			DSP_FLD("end_cust_desc");
			return(0);
		}
	}

	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*sprintf(err_str,"There are only %d on this system",no_lps());
			print_mess(err_str);*/
			print_mess(ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		return(0);
	}

	if (LCHECK("back"))
	{
		strcpy(local_rec.back_desc,(local_rec.back[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("back_desc");
		return(0);
	}

	if (LCHECK("onight"))
	{
		strcpy(local_rec.onight_desc,(local_rec.onight[0] == 'Y') ? "Yes" : "No ");
		DSP_FLD("onight_desc");
		return(0);
	}
	return (EXIT_SUCCESS);
}

/*=====================================================================
| Search for category                                                 |
=====================================================================*/
void
srch_cat (
 char*  key_val)
{   
	work_open();
	save_rec("#Category","#Category Description");
	strcpy(excf_rec.co_no,comm_rec.tco_no);
	sprintf(excf_rec.cat_no,"%-11.11s",key_val);
	cc = find_rec(excf,&excf_rec,GTEQ,"r");
	while (!cc && !strncmp(excf_rec.cat_no,key_val,strlen(key_val)) &&
		      !strcmp(excf_rec.co_no,comm_rec.tco_no))
	{
		cc = save_rec(excf_rec.cat_no,excf_rec.cat_desc);
		if (cc)
			break;
		cc = find_rec(excf,&excf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(excf_rec.co_no,comm_rec.tco_no);
	sprintf(excf_rec.cat_no,"%-16.16s",temp_str);
	cc = find_rec(excf,&excf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

/*=================
| Search for item |
=================*/
void
srch_item (
 char*  key_val)
{
	work_open();
	save_rec("#Item    ","#Item Description");
	strcpy(inmr_rec.co_no,comm_rec.tco_no);
	sprintf(inmr_rec.item_no,"%-16.16s",key_val);
	cc = find_rec(inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strncmp(inmr_rec.item_no,key_val,strlen(key_val)) &&
		      !strcmp(inmr_rec.co_no,comm_rec.tco_no))
	{
		cc = save_rec(inmr_rec.item_no,inmr_rec.description);
		if (cc)
			break;
		cc = find_rec(inmr,&inmr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;
	strcpy(inmr_rec.co_no,comm_rec.tco_no);
	sprintf(inmr_rec.item_no,"%-16.16s",temp_str);
	cc = find_rec(inmr,&inmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, inmr, "DBFIND");
}

int
heading (
 int    scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
		if (BY_CAT)
			rv_pr( ML(mlSaMess033),15,0,1);
		else
			rv_pr( ML(mlSaMess034),15,0,1);

		move(0,1);
		line(80);

		if (BY_CAT)
		{
			box(0,3,80,14);
			move(1,5);
			line(79);
			move(1,8);
			line(79);

			move(1,11);
			line(79);

			move(1,14);
			line(79);
		}
		else
		{
			box(0,3,80,11);
			move(1,5);
			line(79);
			move(1,8);
			line(79);

			move(1,11);
			line(79);
		}

		move(0,20);
		line(80);
		strcpy(err_str,ML(mlStdMess038));
		print_at(21,0, err_str, comm_rec.tco_no,comm_rec.tco_name);
		strcpy(err_str,ML(mlStdMess039));
		print_at(21,40,err_str,comm_rec.test_no,comm_rec.test_name);
		line_cnt = 0;
		scn_write(scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}
