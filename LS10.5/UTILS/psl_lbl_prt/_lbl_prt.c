/*=====================================================================
|  Copyright (C) 1988 - 1993 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (psl_lbl_prrt.c  )                                 |
|  Program Desc  : (Accepts input & prints labels                  )  |
|                  (DO NOT CHANGE cumr struct / sumr struct           |
|                    WITHOUT CHANGING cumr_fields /sumr_fields        |
|                    IN psl_labels.c                                ) |
|---------------------------------------------------------------------|
|  Access files  : comm ,lbhr ,lbln ,cumr ,sumr ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : 06/01/93        |  Author     : Simon Dubey.       |
|---------------------------------------------------------------------|
|  Date Modified : (18/08/93)      | Modified by : Jonathan Chen      |
|  Date Modified : (21/09/93)      | Modified by : Campbell Mander.   |
|  Date Modified : (06/03/96)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (12/09/97)      | Modified  by  : Roanna Marcelino |
|                                                                     |
|     (18/08/93) : HGP 9649 Fix to compile under SVR4                 |
|     (21/09/93) : HGP 9864. Increase sumr_cont_no to 15 chars.       |
|  (06/03/96)    : PDL - Updated for Rexroth Modifications.           |
|  (12/09/97)    : Modified for Multilingual Conversion.              |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _lbl_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/psl_lbl_prt/_lbl_prt.c,v 5.3 2002/07/17 09:58:18 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <pr_format3.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_utils_mess.h>

	/*=======
	| Label |
	=======*/
	struct dbview lbhr_list[] ={
		{"lbhr_filename"},
		{"lbhr_across"},
		{"lbhr_down"},
		{"lbhr_x_size"},
		{"lbhr_y_size"},
		{"lbhr_x_offset"},
		{"lbhr_y_offset"},
		{"lbhr_x_sp"},
		{"lbhr_y_sp"},
		{"lbhr_lines"},
	};

	int	lbhr_no_fields = 10;

	struct	{
		char	hr_filename[14];
		int		hr_across;
		int		hr_down;
		int		hr_x_size;
		int		hr_y_size;
		int		hr_x_offset;
		int		hr_y_offset;
		int		hr_x_sp;
		int		hr_y_sp;
		int		hr_lines;
	} lbhr_rec;

	/*=========================
	| Label line descriptions |
	=========================*/
	struct dbview lbln_list[] ={
		{"lbln_filename"},
		{"lbln_line_no"},
		{"lbln_field_no"},
		{"lbln_field_name"},
		{"lbln_fld_master"},
		{"lbln_field_size"},
	};

	int	lbln_no_fields = 6;

	struct	{
		char	ln_filename[14];
		int		ln_line_no;
		int		ln_field_no;
		char	ln_field_name[19];
		int		ln_fld_master;
		int		ln_field_size;
	} lbln_rec;


	/*====================
	| System Common File |
	====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct	{
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
	} comm_rec;

	/*==================================
	| Customer Master File Base Record |
	==================================*/
/****************  SEE NOTES IN HEADER *********************/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_ch_adr4"},
		{"cumr_dl_adr1"},
		{"cumr_dl_adr2"},
		{"cumr_dl_adr3"},
		{"cumr_dl_adr4"},
		{"cumr_post_code"},
		{"cumr_contact_name"},
		{"cumr_contact2_name"},
		{"cumr_contact3_name"},
		{"cumr_phone_no"},
		{"cumr_fax_no"},
		{"cumr_sman_code"},
		{"cumr_mail_label"},
	};

	int	cumr_no_fields = 21;

	struct	{
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_ch_adr1[41];
		char	cm_ch_adr2[41];
		char	cm_ch_adr3[41];
		char	cm_ch_adr4[41];
		char	cm_dl_adr1[41];
		char	cm_dl_adr2[41];
		char	cm_dl_adr3[41];
		char	cm_dl_adr4[41];
		char	cm_post_code[11];
		char	cm_contact_name[21];
		char	cm_contact2_name[21];
		char	cm_contact3_name[21];
		char	cm_phone_no[16];
		char	cm_fax_no[16];
		char	cm_sman_code[3];
		char	cm_mail_label[2];
	} cumr_rec;

	/*=======================
	| Supplier Master File |
	=======================*/
/****************  SEE NOTES IN HEADER *********************/
	struct dbview sumr_list[] ={
		{"sumr_co_no"},
		{"sumr_est_no"},
		{"sumr_crd_no"},
		{"sumr_crd_name"},
		{"sumr_acronym"},
		{"sumr_adr1"},
		{"sumr_adr2"},
		{"sumr_adr3"},
		{"sumr_adr4"},
		{"sumr_cont_name"},
		{"sumr_cont2_name"},
		{"sumr_cont3_name"},
		{"sumr_cont_no"},
	};

	int	sumr_no_fields = 13;

	struct	{
		char	sm_co_no[3];
		char	sm_est_no[3];
		char	sm_crd_no[7];
		char	sm_name[41];
		char	sm_acronym[10];
		char	sm_adr1[41];
		char	sm_adr2[41];
		char	sm_adr3[41];
		char	sm_adr4[41];
		char	sm_cont_name[21];
		char	sm_cont2_name[21];
		char	sm_cont3_name[21];
		char	sm_cont_no[16];
	} sumr_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
	};

	int exsf_no_fields = 3;

	struct {
		char	sf_co_no[3];
		char	sf_salesman_no[3];
		char	sf_salesman[41];
	} exsf_rec;

	char	*lbhr	=	"lbhr",
			*lbln	=	"lbln",
			*cumr	=	"cumr",
			*sumr	=	"sumr",
			*exsf	=	"exsf",
			*comm	=	"comm",
			*data	=	"data";


	int		Customers	  = FALSE;
	int 	Suppliers = FALSE;
	int		chars_across;
	int		envDbFind;
	int		cr_find;
	int		envDbCo;
	int		total_fields;
	FILE    *fout;
	FILE    *fin;

typedef	struct	LBL_STR
{
	struct	LBL_STR	*next;
	struct	LBL_STR	*prev;
	char	print_str[900];
} linknode;

#define	TAB_NULL	((linknode *) NULL)
linknode	*tab_head = TAB_NULL;
linknode	*tab_curr = TAB_NULL;
linknode	*tab_temp = TAB_NULL;
linknode	*tab_tail = TAB_NULL;

#define	CHAR    1
#define	LONG    2
#define	DOUBLE  3

/*===========================
| Local & Screen Structures |
===========================*/
struct {		
	char	dummy[11];
	char	filename[11];
	int		lpno;
	char	dbt_no[2][7];
	char	crd_no[2][7];
	int		sort[3][3];
	char	description[2][41];
	char	FileMask[11];
	char	s_sman[3];
	char	e_sman[3];
	char	s_name[41];
	char	e_name[41];
} local_rec;            

static	struct	var	vars[] =
{
	{1, LIN, "filename",	 3, 22, CHARTYPE,
		local_rec.FileMask, "          ",
		" ", "", " Label Code        :", "Enter Label Code - Search Available",
		 YES, NO,  JUSTLEFT, "", "", local_rec.filename},
	{2, LIN, "start_dbt_no",	 3, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",      "Start Customer :", "Enter Start Customer - Search Available",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dbt_no[0]},
	{2, LIN, "start_crd_no",	 3, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ",      "Start Supplier :", "Enter Start Supplier - Search Available",
		 ND, NO,  JUSTLEFT, "", "", local_rec.crd_no[0]},
	{2, LIN, "start_description",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[0]},
	{2, LIN, "end_dbt_no",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Customer   :", "Enter End Customer - Search Available",
		 ND, NO,  JUSTLEFT, "", "", local_rec.dbt_no[1]},
	{2, LIN, "end_crd_no",	 4, 18, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier   :", "Enter End Supplier - Search Available",
		 ND, NO,  JUSTLEFT, "", "", local_rec.crd_no[1]},
	{2, LIN, "end_description",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.description[1]},
	{2, LIN, "s_sman",	 6, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "Start Salesman ", " Default is All ",
		YES, NO, JUSTRIGHT, "", "", local_rec.s_sman},
	{2, LIN, "s_name",	 7, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.s_name},
	{2, LIN, "e_sman",	 8, 18, CHARTYPE,
		"UU", "          ",
		" ", " ", "End Salesman   ", " Default is All ",
		YES, NO, JUSTRIGHT, "", "", local_rec.e_sman},
	{2, LIN, "e_name",	9, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Name ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.e_name},
	{2, LIN, "lpno",	 11, 18, INTTYPE,
		"NN", "          ",
		" ", "1",      "Printer No.   :", "Enter Printer Number - Search Available",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{2, LIN, "sort",	 13, 18, INTTYPE,
		"NN", "          ",
		" ", "0",      "Primary Sort  :", "Enter Field To Sort On - Search Available",
		YES, NO, JUSTRIGHT, "0", "99", (char *) &local_rec.sort[0][0]},
	{2, LIN, "sort1", 14, 18, INTTYPE,
		"NN", "          ",
		" ", "0",      "Secondary Sort:", "Enter Field To Sort On - Search Available",
		ND, NO, JUSTRIGHT, "0", "99", (char *) &local_rec.sort[1][0]},
	{2, LIN, "sort2", 15, 18, INTTYPE,
		"NN", "          ",
		" ", "0",      "Tertiary Sort :", "Enter Field To Sort On - Search Available",
		ND, NO, JUSTRIGHT, "0", "99", (char *) &local_rec.sort[2][0]},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include <FindCumr.h>
#include <FindSumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void make_p_file (void);
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
int spec_valid (int field);
void srch_lbhr (char *key_val);
void load_labels (void);
int get_cumr (char *key_val);
int get_sumr (char *key_val);
void load_db_data (void);
void load_cr_data (void);
void print_data (void);
void get_total_fields (void);
void save_line (char *tmp_str);
void get_pos (char *tmp_str);
void update_sort_arr (int fld, int off);
int check_page (void);
void page_break (void);
void srch_fld_names (void);
void sman_srch (char *key_val);
int heading (int scn);

	int		envVarCrCo		=	0;
	char	crBranchNumber[3];
/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv[])
{
	char	*sptr;
	char	tmp[17];

	if (argc < 2)
	{
		/*printf ("Usage %s : C)ustomers S)uppliers\n", argv[0]);*/
		print_at (0,0,ML(mlUtilsMess142), argv[0]);
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);


	if (!strncmp (argv[1], "C", 1))
	{
		Customers = TRUE;
		envDbFind = atoi(get_env("DB_FIND"));
		envDbCo = atoi(get_env("DB_CO"));
		strcpy (local_rec.FileMask, "C-AAAAAAAA");
		FLD("s_sman")	=	YES;
		FLD("e_sman")	=	YES;
		FLD("s_name")	=	NA;
		FLD("e_name")	=	NA;
	}
	else
	{
		Suppliers = TRUE;
		cr_find = atoi(get_env("CR_FIND"));
		envDbCo = atoi(get_env("CR_CO"));
		strcpy (local_rec.FileMask, "S-AAAAAAAA");
		FLD("s_sman")	=	ND;
		FLD("e_sman")	=	ND;
		FLD("s_name")	=	ND;
		FLD("e_name")	=	ND;
		vars[label ("lpno")].row = 6;
		vars[label ("sort")].row = 8;
		vars[label ("sort1")].row = 9;
		vars[label ("sort2")].row = 10;
	}

	OpenDB ();

	sptr = chk_env ("CR_CO");
	envVarCrCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (crBranchNumber, (envVarCrCo) ? comm_rec.test_no : " 0");

	init_scr();
	set_tty();
	set_masks();

	load_labels ();

	prog_exit = 0;
	while (prog_exit == 0)
	{
		search_ok = 1;
		init_ok = TRUE;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_vars(1);	
		init_vars(2);	

		heading (1);
		entry(1);
		if (prog_exit || restart)
			continue;

		heading (2);
		entry(2);

		if (restart)
			continue;

		edit_all ();

		if (restart)
			continue;
		
		make_p_file ();

		/*----------------------
		| re-set tab_head and  |
		| tab_tail to TAB_NULL |
		---------------------*/
		tab_head = TAB_NULL;
		tab_curr = TAB_NULL;
		tab_temp = TAB_NULL;
		tab_tail = TAB_NULL;

		dsp_screen("Processing Data",comm_rec.tco_no,comm_rec.tco_name);
		if (Customers)
			load_db_data ();
		else
			load_cr_data ();
		print_data ();	

		sprintf (tmp, "./%s.p", clip(lbhr_rec.hr_filename));
		fclose (fin);
		cc = unlink (tmp);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
make_p_file (void)
{
	char	tmp[17];
	char    *dptr;
	int     count;
	int     count2;

	chars_across = (lbhr_rec.hr_x_size + lbhr_rec.hr_x_sp + 1) *
							lbhr_rec.hr_across;

	sprintf (tmp, "%s.p", clip(lbhr_rec.hr_filename));
	
	dptr = clip (tmp);

	cc = find_rec ("lbhr", &lbhr_rec, COMPARISON, "r");

	fout = fopen (dptr, "w");

	sprintf (dptr , "HMARGIN   .B%d", lbhr_rec.hr_y_offset);
	fprintf (fout, "%s\n", dptr);

	sprintf (dptr , "MARGIN    .B%d", lbhr_rec.hr_y_sp);
	fprintf (fout, "%s\n", dptr);

	count   = lbhr_rec.hr_across;

	sprintf (dptr, "LABEL     %*.*s",
					lbhr_rec.hr_x_offset,
					lbhr_rec.hr_x_offset,
					" "
		);
	fprintf (fout, "%s", dptr);

	while (count)
	{
		count2  = lbhr_rec.hr_x_size ;

		fprintf (fout, "^");
		while (count2)
		{
			fprintf (fout, "A");
			count2--;
		}

		fprintf (fout, "^");

		sprintf (dptr, "%*.*s",
				  lbhr_rec.hr_x_sp,
				  lbhr_rec.hr_x_sp,
				  " "
			);

		fprintf (fout, "%s", dptr);

		count --;
	}

	fprintf (fout, "\n");

	count = lbhr_rec.hr_lines -  lbhr_rec.hr_y_offset -
		((lbhr_rec.hr_y_size + lbhr_rec.hr_y_sp) *
					lbhr_rec.hr_down);

	fprintf (fout, "NEXT      .B%d\n", count);
	fclose (fout);
}

void
OpenDB (void)
{
	abc_dbopen(data);

	read_comm(comm_list, comm_no_fields, (char *) &comm_rec);
	open_rec (lbln, lbln_list, lbln_no_fields, "lbln_id_no");
	open_rec (lbhr, lbhr_list, lbhr_no_fields, "lbhr_filename");
	open_rec (cumr, cumr_list, cumr_no_fields, "cumr_id_no3");
	open_rec (sumr, sumr_list, sumr_no_fields, "sumr_id_no3");
	open_rec (exsf, exsf_list, exsf_no_fields, "exsf_id_no");
}

void
CloseDB (void)
{
	abc_fclose (lbhr);
	abc_fclose (lbln);
	abc_fclose (cumr);
	abc_fclose (sumr);
	abc_fclose (exsf);
	abc_dbclose (data);
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

int
spec_valid (
 int                field)
{

	if (LNCHECK ("sort", 4))
	{
		if (SRCH_KEY)
		{
			srch_fld_names ();
			return (EXIT_SUCCESS);
		}
	}

	if (LCHECK ("filename"))
	{
		if (SRCH_KEY)
		{
			srch_lbhr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (Customers)
		{
			if (local_rec.filename[0] != 'c' &&
					local_rec.filename[0] != 'C' )
			{
				print_mess (ML(mlUtilsMess135));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			;
		}
		else
		{
			if (local_rec.filename[0] != 's' &&
					local_rec.filename[0] != 'S' )
			{
				print_mess (ML(mlUtilsMess135));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			else
			;
		}

		sprintf (lbhr_rec.hr_filename, "%2.2s%s", comm_rec.tco_no,
						          local_rec.filename);

		cc = find_rec (lbhr, &lbhr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlStdMess233));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		total_fields = 0;
		get_total_fields ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sort"))
	{
		if (local_rec.sort[0][0] == 0)
		{
			FLD ("sort1") = ND;
			FLD ("sort2") = ND;
			local_rec.sort[1][0] = 0;
			local_rec.sort[2][0] = 0;
			scn_write (2);
		}
		else
		{
			FLD ("sort1") = YES;
			scn_write (2);
		}

		if (local_rec.sort[0][0] > total_fields)
		{
			sprintf (err_str, ML(mlUtilsMess139), total_fields);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess();
			return(1);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sort1"))
	{
		if (local_rec.sort[1][0] == 0)
		{
			FLD ("sort2") = ND;
			local_rec.sort[2][0] = 0;
			scn_write (2);
		}
		else
		{
			FLD ("sort2") = YES;
			scn_write (2);
		}

		if (local_rec.sort[1][0] > total_fields)
		{
			sprintf (err_str,ML(mlUtilsMess139), total_fields);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess();
			return(1);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("sort2"))
	{
		if (local_rec.sort[2][0] > total_fields)
		{
			sprintf (err_str,ML(mlUtilsMess139), total_fields);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess();
			return(1);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}

		if (!valid_lp(local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			return(1);
		}
		return(0);
	}

	if (LCHECK ("start_dbt_no"))
	{
		if (FLD("start_dbt_no") == ND)
			return(0);

		if (SRCH_KEY)
		{
			CumrSearch 
			(
				comm_rec.tco_no,
				(envDbCo) ? comm_rec.test_no : " 0",
				temp_str
			);
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[0],"%-40.40s",ML("** First Customer **"));
			DSP_FLD("start_description");
			return(0);
		}

		if (prog_status != ENTRY &&
		     strcmp (pad_num (local_rec.dbt_no[0]),
						local_rec.dbt_no[1]) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.dbt_no[0]));
		
		if (get_cumr(local_rec.dbt_no[0]))
		{
			print_mess(ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(local_rec.description[0],cumr_rec.cm_name);
		DSP_FLD("start_description");
		return(0);
	}

	if (LCHECK ("start_crd_no"))
	{
		if (FLD("start_crd_no") == ND)
			return(0);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, crBranchNumber, temp_str);
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[0],"%-40.40s",ML("** First Supplier **"));
			DSP_FLD("start_description");
			return(0);
		}

		if (prog_status != ENTRY &&
		     strcmp (pad_num (local_rec.crd_no[0]),
						local_rec.crd_no[1]) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(sumr_rec.sm_crd_no,pad_num(local_rec.crd_no[0]));

		if (get_sumr(local_rec.crd_no[0]))
		{
			print_mess(ML(mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(local_rec.description[0],sumr_rec.sm_name);
		DSP_FLD("start_description");
		return(0);
	}

	if (LCHECK ("end_dbt_no"))
	{
		if (FLD("end_dbt_no") == ND)
			return(0);

		if (SRCH_KEY)
		{
			CumrSearch 
			(
				comm_rec.tco_no,
				(envDbCo) ? comm_rec.test_no : " 0",
				temp_str
			);
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[1],"%-40.40s",ML("** Last Customer **"));
			DSP_FLD("end_description");
			return(0);
		}

		strcpy(cumr_rec.cm_dbt_no,pad_num(local_rec.dbt_no[1]));
		
		if (strcmp (local_rec.dbt_no[0], cumr_rec.cm_dbt_no) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (get_cumr(local_rec.dbt_no[1]))
		{
			print_mess(ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(local_rec.description[1],cumr_rec.cm_name);
		DSP_FLD("end_description");
		return(0);
	}
			
	if (LCHECK ("end_crd_no"))
	{
		if (FLD("end_crd_no") == ND)
			return(0);

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.tco_no, crBranchNumber, temp_str);
			return(0);
		}

		if (dflt_used)
		{
			sprintf(local_rec.description[1],"%-40.40s",ML("** Last Supplier **"));
			DSP_FLD("end_description");
			return(0);
		}

		strcpy(sumr_rec.sm_crd_no,pad_num(local_rec.crd_no[1]));

		if (strcmp (local_rec.crd_no[0], sumr_rec.sm_crd_no) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		if (get_sumr(local_rec.crd_no[1]))
		{
			print_mess(ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return(1);
		}

		strcpy(local_rec.description[1],sumr_rec.sm_name);
		DSP_FLD("end_description");
		return(0);
	}

	if (LCHECK ("s_sman"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.s_sman,"%-2.2s","  ");
			sprintf(local_rec.e_sman,"%-2.2s","  ");
			sprintf(local_rec.s_name,"%-40.40s",ML("Start Salesman"));
			sprintf(local_rec.e_name,"%-40.40s",ML("End   Salesman"));

			DSP_FLD("s_sman");
			DSP_FLD("e_sman");
			DSP_FLD("s_name");
			DSP_FLD("e_name");
			return(0);
		}
		FLD("e_sman") = YES;

		if (prog_status != ENTRY && strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
			print_mess (ML(mlStdMess018));
			sleep(2);
			return(1);
		}

		if (SRCH_KEY)
		{
			sman_srch(temp_str);
			return(0);
		}
		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.s_sman);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess(ML (mlStdMess135));
			sleep(2);
			return(1);
		}
		sprintf(local_rec.s_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.s_name,exsf_rec.sf_salesman);

		DSP_FLD("s_sman");
		DSP_FLD("s_name");
		return(0);
	}

	if (LCHECK ("e_sman"))
	{
		if (dflt_used)
		{
			sprintf(local_rec.e_sman,"%-2.2s","~~");
			sprintf(local_rec.e_name,"%-40.40s",ML("End   Salesman"));
			display_field(label("e_sman"));
			display_field(label("e_name"));
			return(0);
		}

		if (SRCH_KEY)
		{
			sman_srch(temp_str);
			return(0);
		}

		strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
		sprintf(exsf_rec.sf_salesman_no,"%-2.2s",local_rec.e_sman);
		cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");	
		if (cc)
		{
			print_mess(ML (mlStdMess135));
			sleep(2);
			return(1);
		}

		if (strcmp(local_rec.s_sman,local_rec.e_sman) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep(2);
			return(1);
		}
		sprintf(local_rec.e_sman,"%-2.2s",exsf_rec.sf_salesman_no);
		strcpy(local_rec.e_name,exsf_rec.sf_salesman);
		DSP_FLD("e_sman");
		DSP_FLD("e_name");
		return(0);
	}
	return(0);
}

/*======================
| Search for Label Code |
=======================*/
void
srch_lbhr (
 char*              key_val)
{
	char    tmp_str[13];

	work_open();
	save_rec("#Label Code   ","#");
	sprintf (tmp_str, "%2.2s%s", comm_rec.tco_no, key_val);
	clip (tmp_str);
	strcpy (lbhr_rec.hr_filename, tmp_str);

	cc = find_rec("lbhr",&lbhr_rec,GTEQ,"r");
	while (!cc && !strncmp(lbhr_rec.hr_filename, tmp_str, strlen(tmp_str)))
	{
		if (Customers && (lbhr_rec.hr_filename[2] != 'C' && 
		 		lbhr_rec.hr_filename[2] != 'c'))
		{
			cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
			continue;	
		}
		
		if (!Customers && (lbhr_rec.hr_filename[2] != 'S' && 
		 		lbhr_rec.hr_filename[2] != 's'))
		{
			cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
			continue;	
		}
		
		/*===========================
		| plus 2 b'cos we don't want |
		| co no showing (do we ??) |
		============================*/
		cc = save_rec(lbhr_rec.hr_filename + 2, " ");
		if (cc)
			break;
		cc = find_rec("lbhr",&lbhr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	sprintf (lbhr_rec.hr_filename , "%2.2s%s", comm_rec.tco_no, temp_str);
	cc = find_rec("lbhr",&lbhr_rec,COMPARISON,"r");
	if (cc)
		file_err(cc,"lbhr","DBFIND");
}

void
load_labels (void)
{
	if (Customers)
	{
		FLD ("start_dbt_no") = YES;
		FLD ("end_dbt_no") = YES;
	}
	else
	{
		FLD ("start_crd_no") = YES;
		FLD ("end_crd_no") = YES;
	}
}

int
get_cumr (
 char*              key_val)
{
	char	_branchNumber[3];

	strcpy(_branchNumber,(envDbCo) ? comm_rec.test_no : " 0");

	strcpy(cumr_rec.cm_co_no,comm_rec.tco_no);
	strcpy(cumr_rec.cm_est_no,_branchNumber);
	
	cc = find_rec("cumr",&cumr_rec,GTEQ,"r");
	while (!cc && !strcmp(cumr_rec.cm_co_no,comm_rec.tco_no))
	{
		/*--------------------------------------------
		| If Debtors Branch Owned && Correct Branch. |
		--------------------------------------------*/
		if (!envDbFind && strcmp(cumr_rec.cm_est_no,_branchNumber))
			break;

		if (!strcmp (key_val, cumr_rec.cm_dbt_no))
			return (EXIT_SUCCESS);

		cc = find_rec("cumr",&cumr_rec,NEXT,"r");
	}
	return (EXIT_FAILURE);
}

int
get_sumr (
 char*              key_val)
{
	char	_branchNumber[3];

	strcpy(_branchNumber,(!envDbCo) ? " 0" : comm_rec.test_no);

	strcpy(sumr_rec.sm_co_no,comm_rec.tco_no);
	strcpy(sumr_rec.sm_est_no,_branchNumber);

	cc = find_rec("sumr",&sumr_rec,GTEQ,"r");
	while (!cc && !strcmp(sumr_rec.sm_co_no,comm_rec.tco_no))
	{
		/*---------------------------------------
		| Not Company Owned & Branch is Wrong	|
		---------------------------------------*/
		if (!cr_find && strcmp(sumr_rec.sm_est_no,_branchNumber))
			break;
		
		if (!strcmp (key_val, sumr_rec.sm_crd_no))
			return (EXIT_SUCCESS);

		cc = find_rec("sumr",&sumr_rec,NEXT,"r");
	}
	return (EXIT_FAILURE);
}

void
load_db_data (void)
{
	char	tmp_str[900];
	char    tmp_str2[73];  /* largest width of one label is 72 */
	int	offset = 0;
	int	field_count = 0;

	strcpy (cumr_rec.cm_co_no,  comm_rec.tco_no );
	strcpy (cumr_rec.cm_est_no, comm_rec.test_no);
	sprintf (cumr_rec.cm_dbt_no, "%-6.6s", local_rec.dbt_no[0]);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	while (!cc) 
	{
		if (strcmp(cumr_rec.cm_co_no,comm_rec.tco_no))
				break;

		if (strcmp(cumr_rec.cm_dbt_no,local_rec.dbt_no[1]) > 0)
				break;

		if (strcmp (cumr_rec.cm_sman_code, local_rec.s_sman) < 0 ||
			 strcmp (cumr_rec.cm_sman_code, local_rec.e_sman) > 0 ||
			 cumr_rec.cm_mail_label[0] != 'Y')
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		dsp_process ("Customer", cumr_rec.cm_dbt_no);
		/*===================
		| read lbln records |
		===================*/
		strcpy (lbln_rec.ln_filename, lbhr_rec.hr_filename);
		lbln_rec.ln_line_no = 0;
		lbln_rec.ln_field_no = 0;
		
		tmp_str[0] = '\0';
		cc = find_rec (lbln, &lbln_rec, GTEQ, "r");

		while(!cc && !strcmp(lbln_rec.ln_filename,lbhr_rec.hr_filename))
		{
			tmp_str2[0] = '\0';
			switch (lbln_rec.ln_fld_master)
			{
				/*========
				| blank  |
				=========*/
				case 99 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							" ");
					break;
				case 0 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_co_no);
					break;
				case 1 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_est_no);
					break;
				case 2 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_dbt_no);
					break;
				case 3 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_name);
					break;
				case 4 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_acronym);
					break;
				case 5 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_ch_adr1  );
					break;
				case 6 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_ch_adr2  );
					break;
				case 7 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_ch_adr3  );
					break;
				case 8 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_ch_adr4  );
					break;
				case 9 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_dl_adr1);
					break;
				case 10:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_dl_adr2 );
					break;
				case 11:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_dl_adr3);
					break;
				case 12:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_dl_adr4);
					break;
				case 13:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_post_code);
					break;
				case 14:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_contact_name);
					break;
				case 15:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_contact2_name);
					break;
				case 16:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_contact3_name);
					break;
				case 17:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_phone_no);
					break;
				case 18:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_fax_no);
					break;
				case 19:
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							cumr_rec.cm_sman_code);
					break;
			}

			if (lbln_rec.ln_fld_master != 99)
				update_sort_arr (++field_count,offset);
			offset += lbln_rec.ln_field_size;
			strcat (tmp_str, tmp_str2);
			cc = find_rec (lbln, &lbln_rec, NEXT, "r");
		}
		if (tmp_str [0])
			save_line (tmp_str);
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
}

void
load_cr_data (void)
{
	char	tmp_str[900];
	char    tmp_str2[73];  /* largest width of one label is 72 */
	int	offset = 0;
	int	field_count = 0;

	strcpy (sumr_rec.sm_co_no,  comm_rec.tco_no );
	strcpy (sumr_rec.sm_est_no, comm_rec.test_no);
	sprintf (sumr_rec.sm_crd_no, "%-6.6s", local_rec.crd_no[0]);
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");

	while (!cc) 
	{
		if (strcmp(sumr_rec.sm_crd_no,local_rec.crd_no[1]) > 0)
				break;

		if (strcmp(sumr_rec.sm_co_no,comm_rec.tco_no))
				break;

		dsp_process ("Supplier", sumr_rec.sm_crd_no);
		/*===================
		| read lbln records |
		===================*/
		strcpy (lbln_rec.ln_filename, lbhr_rec.hr_filename);
		lbln_rec.ln_line_no = 0;
		lbln_rec.ln_field_no = 0;
		
		tmp_str[0] = '\0';
		cc = find_rec (lbln, &lbln_rec, GTEQ, "r");

		while(!cc && !strcmp(lbln_rec.ln_filename,lbhr_rec.hr_filename))
		{

			tmp_str2[0] = '\0';
			switch (lbln_rec.ln_fld_master)
			{
				/*========
				| blank  |
				=========*/
				case 99 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							" ");
					break;
				case 0 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_co_no);
					break;
				case 1 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_est_no);
					break;
				case 2 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_crd_no);
					break;
				case 3 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_name);
					break;
				case 4 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_acronym);
					break;
				case 5 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_adr1);
					break;
				case 6 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_adr2   );
					break;
				case 7 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_adr3   );
					break;
				case 8 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_adr4   );
					break;
				case 9 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_cont_name);
					break;
				case 10 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_cont2_name);
					break;
				case 11 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_cont3_name);
					break;
				case 12 :
					sprintf (tmp_str2, "%-*.*s",
							lbln_rec.ln_field_size,
							lbln_rec.ln_field_size,
							sumr_rec.sm_cont_no);
					break;
			}

			if (lbln_rec.ln_fld_master != 99)
				update_sort_arr(++field_count,offset);
			offset += lbln_rec.ln_field_size;
			strcat (tmp_str, tmp_str2);
			cc = find_rec (lbln, &lbln_rec, NEXT, "r");
		}
		if (tmp_str [0])
			save_line (tmp_str);
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
}

void
print_data (void)
{
	int	count;
	int	count1;
	int	offset = 0;
	char	tmp[21];
	char	tmp_str[900];

	sprintf (tmp, "./%s.p", clip(lbhr_rec.hr_filename));

	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During (POPEN)", errno,PNAME);

	fprintf (fout, ".START00/00/00\n");
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".OP\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".PL0\n");

	if ((fin = fopen (tmp , "r")) == (FILE *)NULL)
	{
		sprintf (err_str, "Error in %s During (FOPEN)", tmp);
		sys_err (err_str, errno, PNAME);
	}

	tab_curr = tab_head;
	pr_format (fin, fout, "HMARGIN", 0, 0);

	while (tab_curr)
	{
		for (count1 = 0; count1 < lbhr_rec.hr_y_size; count1++)
		{
			tab_curr = tab_head;
			strcpy (tmp_str, tab_curr->print_str);

			for (count = 1; count <= lbhr_rec.hr_across; count++)
			{
				offset = count1 * lbhr_rec.hr_x_size;

				pr_format(fin,fout,"LABEL",count,
							tmp_str + offset);

				if (tab_curr->next)
				{
					tab_curr = tab_curr->next;
					strcpy (tmp_str, tab_curr->print_str);
				}
				else
					sprintf (tmp_str, "%-899.899s", " ");
			}
		}

		pr_format (fin, fout, "MARGIN", 0, 0);

		/*------------------------------
		| check if page_break needed   |
		------------------------------*/
		page_break ();

		/*------------------------------
		| OK we've printed them labels |
		| lets now move ahead to the   |
		| next ones                    |
		------------------------------*/
		for (count = 1; count <= lbhr_rec.hr_across; count++)
			if (tab_head->next)
				tab_head = tab_head->next;
			else
				break;

		/*-------------------
		| no more records   |
		-------------------*/
		if (count <= lbhr_rec.hr_across)
		{
			tab_curr = TAB_NULL;
			break;
		}

		tab_curr = tab_head;
		
	}

	/*--------------------
	| means one left over |
	--------------------*/
	if (tab_curr)
	{
		for (count1 = 0; count1 < lbhr_rec.hr_y_size; count1++)
		{
			offset = count1 * lbhr_rec.hr_x_size;
			pr_format(fin,fout,"LABEL", 1,
					tab_curr->print_str + offset);

			for (count = 2; count <= lbhr_rec.hr_across; count++)
			{
				offset = count1 * lbhr_rec.hr_x_size;

				pr_format(fin,fout,"LABEL",count, " ");

			}
		}
	}

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
get_total_fields (void)
{
	strcpy (lbln_rec.ln_filename, lbhr_rec.hr_filename);
	lbln_rec.ln_line_no = 0;
	lbln_rec.ln_field_no = 0;

	cc = find_rec (lbln, &lbln_rec, GTEQ, "r");

	while (!cc && !strcmp (lbln_rec.ln_filename, lbhr_rec.hr_filename))
	{
		if (lbln_rec.ln_fld_master != 99)
			total_fields++;

		cc = find_rec (lbln, &lbln_rec, NEXT, "r");
	}
}

/*======================
put into dynamic array |
based upon sort fields |
======================*/
void
save_line (
 char*              tmp_str)
{
	get_pos (tmp_str);
	strcpy (tab_curr->print_str, tmp_str);
}

void
get_pos (
 char*              tmp_str)
{
	tab_curr = (linknode *) malloc (sizeof (linknode));
	if (tab_curr == TAB_NULL)
		sys_err ("Error in get_pos () During (MALLOC)",errno,PNAME);

	
	/*------------------------
	| means it is first time |
	------------------------*/
	if (tab_head == TAB_NULL)
	{
		tab_head = tab_curr;
		tab_tail = tab_curr;
		tab_curr->prev = TAB_NULL;
		tab_curr->next = TAB_NULL;
		return;
	}
	/*------------------------
	| no sorting             |
	| means put it on the end|
	------------------------*/
	if (local_rec.sort[0][0] == 0)
	{
		tab_tail->next = tab_curr;
		tab_curr->prev = tab_tail;
		tab_curr->next = TAB_NULL;
		tab_tail = tab_curr;
		return;
	}	

	/*----------------------
	| else find out where  |
	| it should go based   |
	| on up to three sorts |
	----------------------*/
	/*------------------
	| this will not be |
	| the first time   |
	------------------*/
	/*----------------------------------
	| compare tab_temp with tmp_str     |
	| at position sort[0][1] for length |
	| of sort[0][2]                     |
	----------------------------------*/

	tab_temp = tab_tail;

	while (tab_temp)
	{
		if (strncmp (tab_temp->print_str + local_rec.sort[0][1],
					tmp_str + local_rec.sort[0][1],
					local_rec.sort[0][2]) < 1)
		{
			if (tab_temp->next)
			{
				tab_temp->next->prev = tab_curr;
				tab_curr->next = tab_temp->next;
			}
			else
			{
				tab_curr->next = TAB_NULL;
				tab_tail = tab_curr;
			}

			tab_temp->next = tab_curr;
			tab_curr->prev = tab_temp;
			return;
		}
		
		/*----------------------------
		| if the strncmp == 0 then   |
		| use secondary /teriary sort|
		----------------------------*/
		if (strncmp (tab_temp->print_str + local_rec.sort[0][1],
					tmp_str + local_rec.sort[0][1],
					local_rec.sort[0][2]) == 0)
		{
			/*----------------
			| no sort exists |
			----------------*/
			if (local_rec.sort[1][0] == 0)
			{
				if (tab_temp->next)
				{
					tab_temp->next->prev = tab_curr;
					tab_curr->next = tab_temp->next;
				}
				else
				{
					tab_curr->next = TAB_NULL;
					tab_tail = tab_curr;
				}

				tab_temp->next = tab_curr;
				tab_curr->prev = tab_temp;
				return;
			}

			/*------------------------------
			| same methodology as 1st sort |
			------------------------------*/
			if (strncmp(tab_temp->print_str + local_rec.sort[1][1],
						tmp_str + local_rec.sort[1][1],
						local_rec.sort[1][2]) < 1)
			{
				if (tab_temp->next)
				{
					tab_temp->next->prev = tab_curr;
					tab_curr->next = tab_temp->next;
				}
				else
				{
					tab_curr->next = TAB_NULL;
					tab_tail = tab_curr;
				}
	
				tab_temp->next = tab_curr;
				tab_curr->prev = tab_temp;
				return;
			}
		
			/*----------------------------
			| if the strncmp == 0 then   |
			| use            teriary sort|
			----------------------------*/
			if (strncmp (tab_temp->print_str + local_rec.sort[1][1],
						tmp_str + local_rec.sort[1][1],
						local_rec.sort[1][2]) == 0)
			{
				/*----------------
				| no sort exists |
				----------------*/
				if (local_rec.sort[2][0] == 0)
				{
					if (tab_temp->next)
					{
						tab_temp->next->prev = tab_curr;
						tab_curr->next = tab_temp->next;
					}
					else
					{
						tab_curr->next = TAB_NULL;
						tab_tail = tab_curr;
					}

					tab_temp->next = tab_curr;
					tab_curr->prev = tab_temp;
					return;
				}

				/*------------------------------
				| same methodology as 2st sort |
				------------------------------*/
				if (strncmp(tab_temp->print_str + 
							local_rec.sort[2][1],
							tmp_str + 
							local_rec.sort[2][1],
							local_rec.sort[2][2]) < 1)
				{
					if (tab_temp->next)
					{
						tab_temp->next->prev = tab_curr;
						tab_curr->next = tab_temp->next;
					}
					else
					{
						tab_curr->next = TAB_NULL;
						tab_tail = tab_curr;
					}
	
					tab_temp->next = tab_curr;
					tab_curr->prev = tab_temp;
					return;
				}
			}
		}
		/*---------------------------
		| work way back thru' list  |
		---------------------------*/
		tab_temp = tab_temp->prev;
	}
	/*-----------------------
	| if breaks out here it |
	| means it is at head   |
	-----------------------*/
	tab_curr->next = tab_head;
	tab_head->prev = tab_curr;
	tab_curr->prev = TAB_NULL;
	tab_head = tab_curr;
}

/*======================
| Update sorted array. |
======================*/
void
update_sort_arr (
 int                fld,
 int                off)
{
	int x;

	for (x = 0; x < 3; x++)
	{
		if (fld == local_rec.sort[x][0])
		{
			local_rec.sort[x][1] = off;
			local_rec.sort[x][2] = lbln_rec.ln_field_size;
		}
	}
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

/*=======================
| Check for page break. |
=======================*/
void
page_break (void)
{
	static int	number_down = 0;
	if (++number_down == lbhr_rec.hr_down)
	{
		pr_format (fin, fout, "NEXT", 0, 0);
		pr_format (fin, fout, "HMARGIN", 0, 0);
		number_down = 0;
	}
}

/*======================
| Search for fields     |
=======================*/
void
srch_fld_names (void)
{
	int	fld_count = 0;
	char	char_fld[3];

	work_open();
	save_rec("#Fld ","#Field Names  ");

	strcpy (lbln_rec.ln_filename, lbhr_rec.hr_filename);
	lbln_rec.ln_line_no = 0;
	lbln_rec.ln_field_no  = 0;

	cc = find_rec (lbln, &lbln_rec, GTEQ, "r");
	while (!cc && !strcmp (lbln_rec.ln_filename, lbhr_rec.hr_filename))
	{
		if (strncmp (lbln_rec.ln_field_name, "blank", 5))
		{
			sprintf (char_fld, "%2d", ++fld_count);
			cc = save_rec(char_fld, lbln_rec.ln_field_name);
			if (cc)
				break;
		}
		cc = find_rec (lbln, &lbln_rec, NEXT, "r");
	}

	cc = disp_srch();
	work_close();
}

/*==================
| Salesman Search. |
==================*/
void
sman_srch (
 char*              key_val)
{
	work_open();
	save_rec("#Sm","#Salesman Name");
	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",key_val);
	cc = find_rec("exsf",&exsf_rec,GTEQ,"r");
	while (!cc && !strcmp(exsf_rec.sf_co_no,comm_rec.tco_no) && 
			      !strncmp(exsf_rec.sf_salesman_no,key_val,strlen(key_val)))
	{
		cc = save_rec(exsf_rec.sf_salesman_no,exsf_rec.sf_salesman);
		if (cc)
			break;
		cc = find_rec("exsf",&exsf_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(exsf_rec.sf_co_no,comm_rec.tco_no);
	sprintf(exsf_rec.sf_salesman_no,"%-2.2s",temp_str);
	cc = find_rec("exsf",&exsf_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsf", "DBFIND");
}

/*================
| Print Heading. |
================*/
int
heading (
 int                scn)
{
	clear ();

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);

		if (Customers)
			strcpy (err_str,ML(mlUtilsMess140));
		else
			strcpy (err_str,ML(mlUtilsMess141));
		

		rv_pr(err_str,40 - (strlen(err_str) / 2),0,1);
		
		move(0,1);
		line(80);

		if (scn == 1)
			box(0,2,80,1);
		if (scn == 2)
		{
			if (Customers)
			{
				box(0,2,80,13);
				move(1,5);
				line(79);
				move(1,10);
				line(79);
				move(1,12);
				line(79);
			}
			else
			{
				move(1,5);
				line(79);
				move(1,7);
				line(79);
				box(0,2,80,8);
			}
		}

		move(0,21);
		line(80);

		print_at (22,0,ML(mlStdMess038), comm_rec.tco_no, comm_rec.tco_name);


		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write(scn);
	}
    return (EXIT_SUCCESS);
}
