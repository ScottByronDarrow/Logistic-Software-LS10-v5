/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( so_by_ps.c     )                                 |
|  Program Desc  : ( Print Packing slips by sales order by item.  )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, inmr, incc,  excf, ccmr,                    |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow    | Date Written  : 06/04/96         |
|---------------------------------------------------------------------|
|  Date Modified : (  /  /  )      | Modified  by  :                  |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|  Comments      :                                                    |
|   (  /  /  )   :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_by_ps.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_by_ps/so_by_ps.c,v 5.2 2001/08/09 09:20:51 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

	/*=====================
	| System Common File. |
	=====================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_cc_no"},
		{"comm_cc_name"},
		{"comm_inv_date"},
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	tcc_no[3];
		char	tcc_name[41];
		long	tinv_date;
	} comm_rec;

	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS	6

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_co_no"},
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_sale_unit"},
		{"inmr_stat_flag"}
	};

	struct tag_inmrRecord
	{
		char	co_no [3];
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		char	sale_unit [5];
		char	stat_flag [2];
	}	inmr_rec;

	/*======+
	 | cohr |
	 +======*/
#define	COHR_NO_FIELDS	10

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_dp_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_type"},
		{"cohr_hhso_hash"},
		{"cohr_hhco_hash"},
		{"cohr_date_raised"},
		{"cohr_date_required"},
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	dp_no [3];
		char	inv_no [9];
		long	hhcu_hash;
		char	type [2];
		long	hhso_hash;
		long	hhco_hash;
		Date	date_raised;
		Date	date_required;
	}	cohr_rec;

	/*============================================+
	 | Customer Order/Invoice/Credit Detail File. |
	 +============================================*/
#define	COLN_NO_FIELDS	7

	struct dbview	coln_list [COLN_NO_FIELDS] =
	{
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_hhsl_hash"},
		{"coln_q_order"},
		{"coln_q_backorder"},
		{"coln_stat_flag"}
	};

	struct tag_colnRecord
	{
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		long	hhsl_hash;
		float	q_order;
		float	q_backorder;
		char	stat_flag [2];
	}	coln_rec;

	/*==========================+
	 | Sales Order Header File. |
	 +==========================*/
#define	SOHR_NO_FIELDS	2

	struct dbview	sohr_list [SOHR_NO_FIELDS] =
	{
		{"sohr_order_no"},
		{"sohr_hhso_hash"},
	};

	struct tag_sohrRecord
	{
		char	order_no [9];
		long	hhso_hash;
	}	sohr_rec;

	/*================================+
	 | Sales Order Detail Lines File. |
	 +================================*/
#define	SOLN_NO_FIELDS	3

	struct dbview	soln_list [SOLN_NO_FIELDS] =
	{
		{"soln_hhso_hash"},
		{"soln_hhsl_hash"},
		{"soln_stat_flag"}
	};

	struct tag_solnRecord
	{
		long	hhso_hash;
		long	hhsl_hash;
		char	stat_flag [2];
	}	soln_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	3

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_acronym"},
	};

	struct tag_cumrRecord
	{
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_acronym [10];
	}	cumr_rec;

int	lpno = 1;

FILE	*ftmp;
FILE	*fsort;

char	*srt_offset[256];


/*=======================
| Function Declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void StartReport (void);
void ProcessFile (void);
void ReportPrint (void);
char *_sort_read (FILE *srt_fil);

	
/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv[])
{
	lpno = atoi(argv[1]);

	OpenDB();

	init_scr();

	dsp_screen ("Packing Slip / Sales Order Report",
						comm_rec.tco_no, comm_rec.tco_name);
	StartReport();

	ProcessFile();

	ReportPrint();

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	fprintf (ftmp, ".EOF\n");
	pclose(ftmp);

	shutdown_prog();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen("data");

	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );
	open_rec ("cohr", cohr_list, COHR_NO_FIELDS, "cohr_id_no2");
	open_rec ("coln", coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec ("sohr", sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec ("soln", soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose ("cohr");
	abc_fclose ("coln");
	abc_fclose ("inmr");
	abc_fclose ("cumr");
	abc_fclose ("sohr");
	abc_fclose ("soln");
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
StartReport (
 void)
{

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen("pformat","w")) == 0)
		sys_err("Error in pformat During (POPEN)", errno, PNAME);

	fprintf(ftmp, ".START%s<%s>\n", DateToString (comm_rec.tinv_date), PNAME);
	fprintf(ftmp, ".LP%d\n",lpno);
	fprintf(ftmp, ".PI12\n");
	fprintf(ftmp, ".13\n");
	fprintf(ftmp, ".L158\n");

	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".EPACKING SLIP / ORDER / ITEM REPORT.\n");
	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".ECompany : %s - %s\n",comm_rec.tco_no, comm_rec.tco_name);
	fprintf(ftmp, ".EBranch  : %s - %s\n",comm_rec.test_no,comm_rec.test_name);
	fprintf(ftmp, ".B1\n");
	fprintf(ftmp, ".E AS AT : %s\n",SystemTime());

	fprintf (ftmp, ".R=========");
	fprintf (ftmp, "=========");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===================");
	fprintf (ftmp, "=========================================");
	fprintf (ftmp, "=========");
	fprintf (ftmp, "============");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "============\n");

	fprintf (ftmp, "=========");
	fprintf (ftmp, "=========");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===================");
	fprintf (ftmp, "=========================================");
	fprintf (ftmp, "=========");
	fprintf (ftmp, "============");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "===========");
	fprintf (ftmp, "============\n");

	fprintf (ftmp, "| P/SLIP ");
	fprintf (ftmp, "| ORDER  ");
	fprintf (ftmp, "|UOM.");
	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|             ITEM DESCRIPTION           ");
	fprintf (ftmp, "|CUSTOMER");
	fprintf (ftmp, "| CUSTOMER  ");
	fprintf (ftmp, "|   DATE   ");
	fprintf (ftmp, "|   DATE   ");
	fprintf (ftmp, "| QUANTITY ");
	fprintf (ftmp, "| QUANTITY |\n");

	fprintf (ftmp, "| NUMBER ");
	fprintf (ftmp, "| NUMBER ");
	fprintf (ftmp, "|    ");
	fprintf (ftmp, "|                  ");
	fprintf (ftmp, "|                                        ");
	fprintf (ftmp, "| NUMBER ");
	fprintf (ftmp, "|  ACRONYM  ");
	fprintf (ftmp, "|  RAISED  ");
	fprintf (ftmp, "| REQUIRED ");
	fprintf (ftmp, "|  ORDERED ");
	fprintf (ftmp, "|BACK ORDER|\n");

	fprintf (ftmp, "|--------");
	fprintf (ftmp, "|--------");
	fprintf (ftmp, "|----");
	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|----------------------------------------");
	fprintf (ftmp, "|--------");
	fprintf (ftmp, "|-----------");
	fprintf (ftmp, "|----------");
	fprintf (ftmp, "|----------");
	fprintf (ftmp, "|----------");
	fprintf (ftmp, "|----------|\n");
}

/*===============
| Process file. |
===============*/
void
ProcessFile (
 void)
{
	char	data_str[200];

	fsort = sort_open ("so_by_ps");
	
	strcpy (cohr_rec.co_no, comm_rec.tco_no);
	strcpy (cohr_rec.br_no, comm_rec.test_no);
	strcpy (cohr_rec.type,  "P");
	strcpy (cohr_rec.inv_no, "        ");
	cc = find_rec ("cohr", &cohr_rec, GTEQ, "r");
	while (!cc && !strcmp (cohr_rec.co_no, comm_rec.tco_no) &&
			      !strcmp (cohr_rec.br_no, comm_rec.test_no) &&
				  cohr_rec.type[0] == 'P')
	{
		coln_rec.hhco_hash 	= cohr_rec.hhco_hash;
		coln_rec.line_no 	= 0;
		cc = find_rec ("coln", &coln_rec, GTEQ, "r");
		while (!cc && coln_rec.hhco_hash == cohr_rec.hhco_hash)
		{
			soln_rec.hhsl_hash = coln_rec.hhsl_hash;
			cc = find_rec ("soln", &soln_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("coln", &coln_rec, NEXT, "r");
				continue;
			}
			sohr_rec.hhso_hash = soln_rec.hhso_hash;
			cc = find_rec ("sohr", &sohr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("coln", &coln_rec, NEXT, "r");
				continue;
			}
			inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
			cc = find_rec ("inmr", &inmr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("coln", &coln_rec, NEXT, "r");
				continue;
			}
			cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
			cc = find_rec ("cumr", &cumr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("coln", &coln_rec, NEXT, "r");
				continue;
			}
			sprintf (data_str, "%s%c%s%c%s%c%s%c%s%c%s%c%s%c%ld%c%ld%c%f%c%f\n",
						cohr_rec.inv_no,		1,		/* Offset = 0	*/
						sohr_rec.order_no,		1,		/* Offset = 1	*/
						inmr_rec.sale_unit,		1,		/* Offset = 2	*/
						inmr_rec.item_no,		1,		/* Offset = 3	*/
						inmr_rec.description,	1,		/* Offset = 4	*/
						cumr_rec.dbt_no,		1,		/* Offset = 5	*/
						cumr_rec.dbt_acronym,	1,		/* Offset = 6	*/
						cohr_rec.date_raised,	1,		/* Offset = 7	*/
						cohr_rec.date_required,	1,		/* Offset = 8	*/
						coln_rec.q_order,		1,		/* Offset = 9	*/
						coln_rec.q_backorder);			/* Offset = 10	*/

			sort_save (fsort, data_str);

			cc = find_rec ("coln", &coln_rec, NEXT, "r");
		}
		cc = find_rec ("cohr", &cohr_rec, NEXT, "r");
	}
}

/*=======================
| Print Report details. |
=======================*/
void
ReportPrint (
 void)
{
	char	OldPS [9],
			NewPS [9];

	int		FirstTime = TRUE;

	char	*sptr;

	strcpy (OldPS, "        ");

	fsort = sort_sort (fsort, "so_by_ps");

	sptr = _sort_read (fsort);
	while (sptr)
	{
		sprintf (NewPS, "%-8.8s", srt_offset [0]);

		if ( strcmp (OldPS, NewPS) )
		{
			dsp_process ("P/Slip", NewPS);

			if ( !FirstTime )
			{
				fprintf (ftmp, "|--------");
				fprintf (ftmp, "|--------");
				fprintf (ftmp, "|----");
				fprintf (ftmp, "|------------------");
				fprintf (ftmp, "|----------------------------------------");
				fprintf (ftmp, "|--------");
				fprintf (ftmp, "|-----------");
				fprintf (ftmp, "|----------");
				fprintf (ftmp, "|----------");
				fprintf (ftmp, "|----------");
				fprintf (ftmp, "|----------|\n");
			}
			FirstTime = FALSE;
			fprintf (ftmp, "|%8.8s", 		srt_offset [0]);
		}
		else
			fprintf (ftmp, "|        ");

		fprintf (ftmp, "|%8.8s", 		srt_offset [1]);
		fprintf (ftmp, "|%4.4s", 		srt_offset [2]);

		fprintf (ftmp, "| %16.16s ", 	srt_offset [3]);
		fprintf (ftmp, "|%-40.40s", 	srt_offset [4]);
		fprintf (ftmp, "|%8.8s", 		srt_offset [5]);
		fprintf (ftmp, "| %-9.9s ", 	srt_offset [6]);
		fprintf (ftmp, "|%10.10s", 		DateToString (atol (srt_offset [7])));
		fprintf (ftmp, "|%10.10s", 		DateToString (atol (srt_offset [8])));
		fprintf (ftmp, "| %8.2f ", 		atof (srt_offset [9]));
		fprintf (ftmp, "| %8.2f |\n",	atof (srt_offset [10]));

		sprintf (OldPS, srt_offset [0]);

		sptr = _sort_read (fsort);
	}
	sort_delete (fsort, "so_by_ps");
}
				
/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char *
_sort_read (
 FILE *srt_fil)
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
	while (fld_no <= 10)
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
