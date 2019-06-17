/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sa_dfcat.c    )                                  |
|  Program Desc  : ( Print Last 12 mnths sales by category           )|
|                : (                                                 )|
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, sadf,                                 |
|  Database      : (sale)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : ( N/A)                                             |
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 28/05/91         |
|---------------------------------------------------------------------|
|  Date Modified : 24/07/91        | Modified  by  : Campbell Mander. |
|  Date Modified : (15/08/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (31/03/94)      | Modified  by  : Campbell Mander. |
|  Date Modified : (27/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (16/09/97)      | Modified  by  : Rowena S Maandig |
|                                                                     |
|  Comments      : Added time into heading. Company report no longer  |
|                : has breakdown by branch.                           |
|  (15/08/92)    : Changes for HP port. S/C INF 7619                  |
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (27/05/96)    : Updated to fix problems in DateToString.                |
|  (16/09/97)    : Updated to incorporate multilingual conversion.    |
|                :                                                    |
|                                                                     |
| $Log: sa_dfcat.c,v $
| Revision 5.2  2001/08/09 09:16:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/07 00:06:28  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:13:34  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:34:43  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:56  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:26  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:35:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/16 04:55:33  scott
| Updated to fix warning found when compiled with -Wall flag.
|
| Revision 1.10  1999/10/16 01:11:21  nz
| Updated for pjulmdy routines
|
| Revision 1.9  1999/09/29 10:12:49  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 07:27:34  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.7  1999/09/16 02:01:52  scott
| Updated from Ansi Project.
|
| Revision 1.6  1999/06/18 09:39:20  scott
| Updated for read_comm(), log for cvs, compile errors.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sa_dfcat.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SA/sa_dfcat/sa_dfcat.c,v 5.2 2001/08/09 09:16:56 scott Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_sa_mess.h>

#define	LINE		0
#define	BLNK		1

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
		char	df_co_no[3];
		char	df_br_no[3];
		char	df_year[2];
		long	df_hhbr_hash;
		long	df_hhcu_hash;
		float	df_qty_per[12];
		double	df_sal_per[12];
		double	df_cst_per[12];
		char	df_sman[3];
		char	df_area[3];
	} sadf_rec;

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
		char	mr_co_no[3];
		char	mr_item_no[17];
		long	mr_hhbr_hash;
		char	mr_alpha_code[17];
		char	mr_maker_no[17];
		char	mr_alternate[17];
		char	mr_class[2];
		char	mr_description[41];
		char	mr_category[12];
	} inmr_rec;

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
		char	cf_co_no[3];
		char	cf_cat_no[12];
		char	cf_cat_desc[41];
	} excf_rec;

FILE	*fout;
FILE	*fsort;

int	curr_mnth;
int	curr_year;
int	fiscal;
int	first_time = TRUE;
int	printed = FALSE;
int	BY_CO = FALSE;
int	BY_BR = FALSE;

char	*srt_offset[128];
char	prev_group[13];
char	curr_group[13];

double	L_tot[12];
double	C_tot[12];
double	cat_tot[12];
double	grand_tot[12];
double	mtd[3];
double	ytd[3];
double	lst_ytd[3];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char 	back[4];
	char	onight[4];
	char	lp_str[3];
	int	lpno;
	char	st_group[13];
	char	end_group[13];
	char	end_cat[12];
	char	rep_by[8];
} local_rec;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void process (void);
void proc_sorted (void);
int proc_data (int print_type);
void sum_tot (void);
int clear_tot (char *type);
int print_line (int line_type);
int print_tot (char *tot_type, int pg_brk);
int calc_ytd (void);
void head_output (void);
void init_array (void);
char *month_name (int n);
char *_sort_read (FILE *srt_fil);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc != 5)
	{
		print_at(0,0,mlSaMess727,argv[0]);
		return (EXIT_FAILURE);
	}

	if (argv[4][0] == 'C')
		BY_CO = TRUE;
	else
		BY_BR = TRUE;

	sprintf(local_rec.st_group,"%-12.12s",argv[2]);
	sprintf(local_rec.end_group,"%-12.12s",argv[3]);
	sprintf(local_rec.end_cat,"%-11.11s",local_rec.end_group + 1);
	local_rec.lpno = atoi(argv[1]);

	DateToDMY (comm_rec.tdbt_date, NULL, &curr_mnth, &curr_year);

	sptr = chk_env("SA_YEND");
	fiscal = ( sptr == (char *)0 ) ? comm_rec.tfiscal : atoi( sptr );

	if ( fiscal < 1 || fiscal > 12 )
		fiscal = comm_rec.tfiscal;

	OpenDB();

	sprintf(err_str,"Reading : Sales by Category");
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
	process();
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
OpenDB(void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec("sadf",sadf_list,sadf_no_fields,"sadf_id_no4");
	open_rec("inmr",inmr_list,inmr_no_fields,"inmr_id_no_3");
	open_rec("excf",excf_list,excf_no_fields,"excf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose("sadf");
	abc_fclose("inmr");
	abc_fclose("excf");
	abc_dbclose("data");
}

void
process (void)
{
	char	data_str[200];
	char	tmp_group[13];
	int	lcl_cc;

	fsort = sort_open("sadf");
	
	strcpy(inmr_rec.mr_co_no, comm_rec.tco_no);
	sprintf(inmr_rec.mr_class, "%-1.1s", local_rec.st_group);
	sprintf(inmr_rec.mr_category, "%-11.11s", local_rec.st_group + 1);
	sprintf(inmr_rec.mr_item_no, "%-16.16s", " ");
	/*------------------------------------
	| Process all inmr records that fall |
	| within the class/category range    |
	| specified by the user		     |
	------------------------------------*/
	cc = find_rec("inmr", &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp(inmr_rec.mr_co_no, comm_rec.tco_no) &&
	       strncmp(inmr_rec.mr_class, local_rec.end_group, 1) <= 0)
	{
		/*-----------------------
		| Category out of range |
		-----------------------*/
		if ((!strncmp(inmr_rec.mr_class,local_rec.end_group, 1)) &&
		    (strcmp(inmr_rec.mr_category,local_rec.end_cat) > 0))
			break;

		strcpy (excf_rec.cf_co_no, comm_rec.tco_no);
		sprintf(excf_rec.cf_cat_no, "%-11.11s", inmr_rec.mr_category);
		lcl_cc = find_rec("excf", &excf_rec, COMPARISON, "r");
		if (lcl_cc)
		{
			strcpy(excf_rec.cf_cat_no, "           ");
			sprintf(excf_rec.cf_cat_desc, "%-40.40s", "Unknown Category");
		}

		sadf_rec.df_hhbr_hash = inmr_rec.mr_hhbr_hash;
		sadf_rec.df_hhcu_hash = 0L;

		lcl_cc = find_rec("sadf", &sadf_rec, GTEQ, "r");
		while (!lcl_cc && sadf_rec.df_hhbr_hash == inmr_rec.mr_hhbr_hash)
		{
			if (BY_BR && strcmp(sadf_rec.df_br_no,comm_rec.test_no))
			{
				lcl_cc = find_rec("sadf", &sadf_rec, NEXT, "r");
				continue;

			}
			sprintf(tmp_group, 
				"%-1.1s%-11.11s", 
				inmr_rec.mr_class, 
				excf_rec.cf_cat_no);

			sprintf(data_str, 
				"%2.2s%c%-12.12s%c%-1.1s%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f%c%.2f\n",
				(BY_CO) ? "  " : sadf_rec.df_br_no, 1,
				tmp_group, 1,
				sadf_rec.df_year, 1,
				sadf_rec.df_sal_per[0], 1,
				sadf_rec.df_sal_per[1], 1,
				sadf_rec.df_sal_per[2], 1,
				sadf_rec.df_sal_per[3], 1,
				sadf_rec.df_sal_per[4], 1,
				sadf_rec.df_sal_per[5], 1,
				sadf_rec.df_sal_per[6], 1,
				sadf_rec.df_sal_per[7], 1,
				sadf_rec.df_sal_per[8], 1,
				sadf_rec.df_sal_per[9], 1,
				sadf_rec.df_sal_per[10], 1,
				sadf_rec.df_sal_per[11]);
			sort_save(fsort, data_str);

			dsp_process("Category :",excf_rec.cf_cat_no);

			lcl_cc = find_rec("sadf", &sadf_rec, NEXT, "r");
		}
			
		cc = find_rec("inmr", &inmr_rec, NEXT, "r");
	}

	sprintf(err_str,"Printing : Sales by Category");
	dsp_screen(err_str,comm_rec.tco_no,comm_rec.tco_name);
	proc_sorted();

	fprintf(fout,".EOF\n");
	pclose(fout);
}

void
proc_sorted (void)
{
	char	*sptr;
	int	print_type;

	head_output();

	init_array();

	printed = FALSE;
	first_time = TRUE;

	fsort = sort_sort(fsort,"sadf");
	sptr = _sort_read(fsort);

	while (sptr != (char *)0)
	{
		printed = TRUE;

		sprintf(curr_group,"%-12.12s",srt_offset[1]);

		dsp_process("Category :",curr_group + 1);

		if (first_time)
		{
			first_time = FALSE;
			strcpy(prev_group,curr_group);
		}

		print_type = 0;
		if (strcmp(curr_group,prev_group))
			print_type = 1;
		
		/*-------------------------------
		| Print category or branch total|
		| if change in cat or br. If no	|
		| change then accumulate totals |
		-------------------------------*/
		proc_data(print_type);
		strcpy(prev_group,curr_group);

		sptr = _sort_read(fsort);
	}

	if (printed)
	{
		print_tot("C", FALSE);
		print_tot("G", FALSE);
	}

	sort_delete(fsort,"sadf");
}

int
proc_data (
 int    print_type)
{
	switch (print_type)
	{
	case 0:
		sum_tot();
		break;

	case 1:
		print_tot("C", FALSE);
		clear_tot("C");
		sum_tot();
		break;
	}
	return(0);
}

void
sum_tot (void)
{
	int	i;

	if (srt_offset[2][0] == 'C')
	{
		for (i = 0; i < 12; i++)
			C_tot[i] += atof(srt_offset[i + 3]);
	}

	if (srt_offset[2][0] == 'L')
	{
		for (i = 0; i < 12; i++)
			L_tot[i] += atof(srt_offset[i + 3]);
	}

}

int
clear_tot (
 char*  type)
{
	int	i;

	for (i = 0; i < 12; i++)
	{
		C_tot[i] = 0.00;
		L_tot[i] = 0.00;
		cat_tot[i] = 0.00;
	}

	return(0);
}

int
print_line (
 int    line_type)
{
	if (line_type == LINE)
	{
		fprintf(fout, "!----------!----------!----------!----------");
		fprintf(fout, "!----------!----------!----------!----------");
		fprintf(fout, "!----------!----------!----------!----------");
		fprintf(fout, "!----------!----------!\n");
	}
	else
	{
		fprintf(fout, "!          !          !          !          ");
		fprintf(fout, "!          !          !          !          ");
		fprintf(fout, "!          !          !          !          ");
		fprintf(fout, "!          !          !\n");
	}
	return (EXIT_SUCCESS);
}


int
print_tot (
 char*  tot_type,
 int    pg_brk)
{
	int	i;
	int	lcl_cc;
	char	tmp_desc[81];

	switch (tot_type[0])
	{
	case	'C':
		calc_ytd();

		/*--------------------------
		| Get category description |
		--------------------------*/
		strcpy(excf_rec.cf_co_no, comm_rec.tco_no);
		sprintf(excf_rec.cf_cat_no, "%-11.11s", prev_group + 1);
		lcl_cc = find_rec("excf", &excf_rec, COMPARISON, "r");
		if (lcl_cc)
			sprintf(excf_rec.cf_cat_desc, "%-40.40s", "Unknown Category");

		/*----------------------
		| Print Category Total |
		----------------------*/
		expand(tmp_desc, excf_rec.cf_cat_desc);
		fprintf(fout, ".LRP6\n");
		fprintf(fout, 
			"! %-1.1s %-11.11s  %-80.80s  !          !          !          !          !          !\n", 
			prev_group, 
			prev_group + 1, 
			tmp_desc);

		for (i = curr_mnth; i < 12; i++)
			fprintf(fout, "! %9.0f", C_tot[i]);

		for (i = 0; i < curr_mnth - 1; i++)
			fprintf(fout, "! %9.0f", C_tot[i]);

		fprintf( fout, 
			"! %9.0f! %9.0f! %9.0f!\n",
			mtd[0],
			ytd[0],
			lst_ytd[0]);

		print_line(BLNK);

		for (i = 0; i < 12; i++)
			grand_tot[i] += C_tot[i];

		if (tot_type[0] == 'C')
			break;

	case	'G':
		fprintf(fout, ".LRP3\n");
		print_line(LINE);
		fprintf(fout, "! GRAND TOTAL %-140.140s!\n", " ");
		for (i = curr_mnth; i < 12; i++)
			fprintf(fout, "!%10.0f", grand_tot[i]);

		for (i = 0; i < curr_mnth - 1; i++)
			fprintf(fout, "!%10.0f", grand_tot[i]);

		fprintf( fout, "!%10.0f!%10.0f!%10.0f!\n",mtd[2],ytd[2],lst_ytd[2]);
		break;
	}
	return(0);
}

int
calc_ytd (void)
{
	int	i;

	ytd[0] = 0.00;
	lst_ytd[0] = 0.00;

	mtd[0] = C_tot[curr_mnth - 1];
	mtd[1] += mtd[0];
	mtd[2] += mtd[0];

	if (curr_mnth < fiscal)
	{
		for (i = fiscal; i < 12; i++)
		{
			ytd[0] += C_tot[i];
			lst_ytd[0] += L_tot[i];
		}

		for (i = 0; i < curr_mnth; i++)
		{
			ytd[0] += C_tot[i];
			lst_ytd[0] += L_tot[i];
		}

	}
	else
	{
		for (i = fiscal; i < curr_mnth; i++)
		{
			ytd[0] += C_tot[i];
			lst_ytd[0] += L_tot[i];
		}
	}

	ytd[1] += ytd[0];
	ytd[2] += ytd[0];

	lst_ytd[1] += lst_ytd[0];
	lst_ytd[2] += lst_ytd[0];

	return (EXIT_SUCCESS);
}

void
head_output (void)
{
	int	i;

	if ((fout = popen("pformat","w")) == NULL)
		sys_err("Error in pformat during (POPEN)",errno,PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".START%s\n",clip(err_str));
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI12\n");
	fprintf(fout,".12\n");
	fprintf(fout,".L158\n");
	
	fprintf(fout,".ESales Analysis By Category For Last 12 Months\n");
	fprintf(fout,
		".ECOMPANY : %s - %s\n",
		comm_rec.tco_no,
		clip(comm_rec.tco_name));

	if (BY_CO)
		fprintf(fout,".EALL BRANCHES\n");
	else
	{
		fprintf(fout,
			".EBRANCH : %-2.2s - %-15.15s\n",
			comm_rec.test_no,
			comm_rec.test_name);
	}
		
	fprintf(fout,".EFOR THE MONTH OF %-9.9s\n",month_name(curr_mnth - 1));

	fprintf(fout,".B1\n");
	fprintf(fout,"All Figures are Sales Values\n");
	fprintf(fout,".B1\n");
	sprintf(err_str, 
		"For Categories %-1.1s  %-11.11s to %-1.1s  %-11.11s", 
		local_rec.st_group, 
		local_rec.st_group + 1, 
		local_rec.end_group, 
		local_rec.end_group + 1);

	fprintf(fout,".E%s\n",err_str);

	fprintf(fout,"====================================================");
	fprintf(fout,"====================================================");
	fprintf(fout,"===================================================\n");

	for (i = curr_mnth; i < 12; i ++)
		fprintf(fout, "! %-3.3s %4d ", month_name(i), curr_year - 1);
	for (i = 0; i < curr_mnth - 1; i ++)
		fprintf(fout, "! %-3.3s %4d ", month_name(i), curr_year);
	fprintf(fout, "! %-3.3s %4d !   YTD    !   LYTD   !\n", month_name(curr_mnth - 1), curr_year);

	fprintf(fout,"!----------!----------!----------!----------");
	fprintf(fout,"!----------!----------!----------!----------");
	fprintf(fout,"!----------!----------!----------!----------");
	fprintf(fout,"!----------!----------!\n");

	fprintf(fout,".R====================================================");
	fprintf(fout,"====================================================");
	fprintf(fout,"===================================================\n");

}

void
init_array (void)
{
	int	j;

	for (j = 0; j < 12; j++)
	{
		L_tot[j] = 0.00;
		C_tot[j] = 0.00;
		cat_tot[j] = 0.00;
		grand_tot[j] = 0.00;
	}
	
	for (j = 0; j < 3; j++)
	{
		mtd[j] = 0.00;
		ytd[j] = 0.00;
		lst_ytd[j] = 0.00;
	}
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
	return((n >= 0 && n <= 11) ? name[n] : name[12]);
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
	while (fld_no < 15)
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
