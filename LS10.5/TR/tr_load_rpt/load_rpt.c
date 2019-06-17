/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( tr_load_rpt.c)                                   |
|  Program Desc  : ( Printing of Transport Load Report)		    	  |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  cumr, inmr, comm, cohr, coln, trhr, trln,         |
|                :  trve, extf,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      :                                                    |
|---------------------------------------------------------------------|
|  Date Written  : 05/22/96        |  Author : Edz Monserrate         |
|---------------------------------------------------------------------|
|  Date Modified : 06/02/96        |  Modified By:  Liza Santos       |
|  Date Modified : 01/10/96        |  Modified By:  Jiggs Veloz       |
|  Date Modified : 11/09/97        |  Modified By:  Roanna Marcelino  |
|  Date Modified : 14/10/97        |  Modified By:  Roanna Marcelino  |
|  Date Modified : (28/10/1997)    | Modified by : Campbell Mander.   |
|  (01/10/96)	 :	Changed report format,removed TOTAL and included  |
|				 :	ordered uom.	 								  |
|  (11/09/97)	 :	Modified for Multilingual Conversion.             |
|  (14/10/97)	 :	Fixed Mldb Error.                                 |
|  (28/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|				 :													  |
|                                                                     |
|====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: load_rpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TR/tr_load_rpt/load_rpt.c,v 5.3 2002/07/17 09:58:12 scott Exp $";

#include 	<pslscr.h>
#include	<get_lpno.h>
#include 	<dsp_process2.h>
#include 	<dsp_screen.h>
#include 	<ml_std_mess.h>
#include 	<ml_tr_mess.h>

static char	*data	= "data",
		    *cumr	= "cumr",
			*inmr	= "inmr",
			*inum	= "inum",
			*cohr	= "cohr",
			*coln	= "coln",
			*trhr	= "trhr",
			*trln	= "trln",
			*trve	= "trve",
			*extf	= "extf";

	/*===================================+
     | Customer Master File Base Record. |	
	 +===================================*/
#define	CUMR_NO_FIELDS 5	

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"}
	};

	struct tag_cumrRecord
	{
		char	co_no [3];
		char	est_no [3];
		char	dbt_no [7];
		long	hhcu_hash;
		char	dbt_name [41];
	}	cumr_rec;


	/*====================================+
	 | Inventory Master File Base Record. |
	 +====================================*/
#define	INMR_NO_FIELDS 4	

	struct dbview	inmr_list [INMR_NO_FIELDS] =
	{
		{"inmr_item_no"},
		{"inmr_hhbr_hash"},
		{"inmr_description"},
		{"inmr_std_uom"}
	};

	struct tag_inmrRecord
	{
		char	item_no [17];
		long	hhbr_hash;
		char	description [41];
		int		std_uom; 
	}	inmr_rec;


	/*=====================+
	 | System Common File. |
	 +=====================*/
#define	COMM_NO_FIELDS	7

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
	};

	struct tag_commRecord
	{
		int		term;
		char	co_no [3];
		char	co_name [41];
		char	co_short [16];
		char	est_no [3];
		char	est_name [41];
		char	est_short [16];
	}	comm_rec;

	/*======+
	 | cohr |
	 +======*/
#define	COHR_NO_FIELDS	7

	struct dbview	cohr_list [COHR_NO_FIELDS] =
	{
		{"cohr_co_no"},
		{"cohr_br_no"},
		{"cohr_inv_no"},
		{"cohr_hhcu_hash"},
		{"cohr_no_kgs"},
		{"cohr_hhco_hash"},
		{"cohr_hhtr_hash"}
	};

	struct tag_cohrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	inv_no [9];
		long	hhcu_hash;
		float	no_kgs;
		long	hhco_hash;
		long	hhtr_hash;
	}	cohr_rec;


	/*============================================+
	 | Customer Order/Invoice/Credit Detail File. |
	 +============================================*/
#define	COLN_NO_FIELDS	8

	struct dbview	coln_list [COLN_NO_FIELDS] =
	{
		{"coln_hhcl_hash"},
		{"coln_hhco_hash"},
		{"coln_line_no"},
		{"coln_hhbr_hash"},
		{"coln_q_order"},
		{"coln_qty_del"},
		{"coln_item_desc"},
		{"coln_hhum_hash"},
	};

	struct tag_colnRecord
	{
		long	hhcl_hash;
		long	hhco_hash;
		int		line_no;
		long	hhbr_hash;
		float	q_order;
		float	qty_del;
		char	item_desc [41];
		long	hhum_hash;
	}	coln_rec;

	/*======+
	 | trhr |
	 +======*/
#define	TRHR_NO_FIELDS	6

	struct dbview	trhr_list [TRHR_NO_FIELDS] =
	{
		{"trhr_trip_name"},
		{"trhr_hhve_hash"},
		{"trhr_del_date"},
		{"trhr_hhtr_hash"},
		{"trhr_driver"},
		{"trhr_status"}
	};

	struct tag_trhrRecord
	{
		char	trip_name [13];
		long	hhve_hash;
		long	del_date;	
		long	hhtr_hash;
		char	driver [7];
		char	status [2];
	}	trhr_rec;

	/*======+
	 | trln |
	 +======*/
#define	TRLN_NO_FIELDS	3

	struct dbview	trln_list [TRLN_NO_FIELDS] =
	{
		{"trln_hhtr_hash"},
		{"trln_hhco_hash"},
		{"trln_hhln_hash"}
	};

	struct tag_trlnRecord
	{
		long	hhtr_hash;
		long	hhco_hash;
		long	hhln_hash;
	}	trln_rec;


	/*================================+
	 | Transport vehicle file record. |
	 +================================*/
#define	TRVE_NO_FIELDS	5

	struct dbview	trve_list [TRVE_NO_FIELDS] =
	{
		{"trve_co_no"},
		{"trve_br_no"},
		{"trve_ref"},
		{"trve_desc"},
		{"trve_hhve_hash"},
	};

	struct tag_trveRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	ref [11];
		char	desc [41];
		long	hhve_hash;
	}	trve_rec;

	/*========================+
	 | External Trucker file. |
	 +========================*/
#define	EXTF_NO_FIELDS	3

	struct dbview	extf_list [EXTF_NO_FIELDS] =
	{
		{"extf_co_no"},
		{"extf_code"},
		{"extf_name"}
	};

	struct tag_extfRecord
	{
		char	co_no [3];
		char	code [7];
		char	name [41];
	}	extf_rec;

	/*=================================+
	 | Inventory Unit of Measure File. |
	 +=================================*/
#define	INUM_NO_FIELDS	4

	struct dbview	inum_list [INUM_NO_FIELDS] =
	{
		{"inum_hhum_hash"},
		{"inum_uom"},
		{"inum_desc"},
		{"inum_cnv_fct"},
	};

	struct tag_inumRecord
	{
		long	hhum_hash;
		char	uom [5];
		char	desc [41];
		float	cnv_fct;
	}	inum_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	vehi_ref [11];
	char	vehi_desc[41];
	long    hhve_hash;
	long	del_date; 
	char	date_desc[10];
	char	trip_name[13];
	char	sequence [2];
	int		lpno;
	char	lp_str[3];
	char	dummy[11];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "vehi_ref",	 4, 18, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "          ", "Vehicle Ref. No.   :", "Enter Vehicle Reference No. (Default All) [SEARCH]",
		 YES, NO,  JUSTLEFT, "", "", local_rec.vehi_ref},
	{1, LIN, "vehi_desc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTRIGHT, "", "", local_rec.vehi_desc},
	{1, LIN, "del_date",	 6, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Exp. Delivery Date :", "Enter Expected Date (Default All) [Search]",
		 YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.del_date},
	{1, LIN, "date_desc",	 6, 34, CHARTYPE,
		"AAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTRIGHT, "", "", local_rec.date_desc},
	{1, LIN, "trip_name",	 8, 18, CHARTYPE,
		"UUUUUUUUUUUU", "          ",
		" ", " ", "Trip Name          :", "Enter Trip Name (Defualt All) [SEARCH]",
		 YES, NO,  JUSTLEFT, "", "", local_rec.trip_name},
	{1, LIN, "sequence",	 10, 18, CHARTYPE,
		"U", "          ",
		" ", "D", "Sequence (I/D)     :", "Enter [I]tem or [D]estination",
		 YES, NO,  JUSTRIGHT, "ID", "", local_rec.sequence},
	{1, LIN, "lpno",	 12, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer            : ", "Printer Number ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================
Special fields and flags
=======================*/
FILE 	*fout,
		*fsort;
char 	*sptr,
		est_no 		[3],
		est_name	[41],
		dbt_no		[7],
		prev_dbt	[7],
		dbt_name	[41],
		item_no		[17],
		prev_item	[17],
		description	[41],
		uom			[5],
		inv_no      [9],
		ref			[11],
		prev_ref	[11],
		dr_code		[7],
		dr_name		[41],
		d_date	    [8],
		q_load	    [11],
		q_del	    [11],
		prev_trip_name	[13] = "", 
		this_trip	[13],	
		curr_co		[3],
		driver_code	[7], 
		driver_name	[41], 
		cust_no		[7], 
		cust_name	[41], 
		it_no		[17],
		it_desc		[41],
	    pk_uom		[5],
		pk_uom_desc	[41];

float   qty_del,
		q_order,
		std_qty_del,
		std_q_order;

long    del_date,
		prev_del_date;


/*=======================
| Function declarations |
=======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void end_prnt (void);
void run_prog (char *);
void head_output (void);
void process_file (void);
void get_extf (void);
void get_cohr_cumr (void);
void get_inmr_inum (void);
void store_data (void);
void display_report (void);
void print_tail (void);
void head_display (char *, char *, char *, char *, char *);
void SrchTrve (char *);
void SrchTrhrDate (char *);
void SrchTrhrTrip (char *);
int spec_valid (int);
int heading (int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	workDateStr [11];
	if (argc != 1 && argc != 6)
	{
		print_at(0,0,mlStdMess036, argv[0]);
        return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);
	OpenDB();

	if (argc == 6)
	{
		sprintf(local_rec.vehi_ref,"%-10.10s",argv[1]);
		local_rec.del_date = atol(argv[2]);
		sprintf(local_rec.trip_name,"%-12.12s",argv[3]);
		sprintf(local_rec.sequence,"%-1.1s",argv[4]);
		local_rec.lpno = atoi(argv[5]);

		sprintf (workDateStr, "%ld",local_rec.del_date);
		sprintf (local_rec.lp_str, "%d", local_rec.lpno);
		
		if (!strcmp(local_rec.sequence, "D"))
		{
			if (fork() != 0)
			{
				clear();
				wait((int *)0);
			}
			else
			{
				execlp( "tr_delmani",
					"tr_delmani",
					local_rec.vehi_ref,
					workDateStr,
					local_rec.trip_name,
					local_rec.sequence,
					local_rec.lp_str,
					(char *) 0);
			}
		}
        else
		{
			dsp_screen("Printing Transport Load Report",
					comm_rec.co_no,comm_rec.co_name);
			head_output();
			process_file(); 	

			fprintf (fout, ".EOF\n");
        	end_prnt();
		}

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/* ============================
	| Set up required parameters  |
	=============================*/
	init_scr();			
	set_tty();         
	set_masks();	
	init_vars(1);

	while (prog_exit == 0)
	{
		/*=========================	
		|   Reset control flags   |
		=========================*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars(1);

	    /*=============================	
		| Entry screen 1 linear input |
	    =============================*/
		heading(1);
		entry(1);
		if (restart || prog_exit)
			continue;

	    /*============================	
		| Edit screen 1 linear input |
		============================*/
		heading(1);
		scn_display(1);
		edit(1);      
		if (restart)
			continue;

	    run_prog (argv[0]);	
		prog_exit = 1;
	}
	shutdown_prog();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	open_rec( cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec( inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec( inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec( cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec( coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec( trhr, trhr_list, TRHR_NO_FIELDS, "trhr_id_no");
	open_rec( trln, trln_list, TRLN_NO_FIELDS, "trln_hhtr_hash");
	open_rec( trve, trve_list, TRVE_NO_FIELDS, "trve_id_no");
	open_rec( extf, extf_list, EXTF_NO_FIELDS, "extf_id_no");
 }

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose(cumr);
	abc_fclose(inmr);
	abc_fclose(inum);
	abc_fclose(cohr);
	abc_fclose(coln);
	abc_fclose(trhr);
	abc_fclose(trln);
	abc_fclose(trve);
	abc_fclose(extf);
	abc_dbclose(data);
}

int
spec_valid (
 int field)
{
	 int date_found = FALSE;
	 int trip_found = FALSE;
	/*-----------------------------------------------------
	| Validate Vehicle Reference Number And Allow Search. |
	-----------------------------------------------------*/
	if (LCHECK("vehi_ref"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.vehi_ref, "          ");
			sprintf (local_rec.vehi_desc, "%-40.40s", "All Vehicle Units");
			local_rec.hhve_hash = 0L;
			DSP_FLD ("vehi_ref");
			DSP_FLD ("vehi_desc");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
		{
	 		SrchTrve (temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (trve_rec.co_no, comm_rec.co_no);
		strcpy (trve_rec.br_no, comm_rec.est_no);
		strcpy (trve_rec.ref, local_rec.vehi_ref);
		cc = find_rec (trve, &trve_rec, EQUAL, "r");
		if (cc)
		{
			/*Vehicle Unit %s is not on file., local_rec.vehi_ref*/
			errmess (ML(mlTrMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.vehi_ref, trve_rec.ref);
		strcpy (local_rec.vehi_desc, trve_rec.desc);
		local_rec.hhve_hash = trve_rec.hhve_hash;
		DSP_FLD ("vehi_ref");
		DSP_FLD ("vehi_desc");
	}
       
 /*-------------------------- 
  |	Validate Delivery Date 	| 
  -------------------------*/
	if (LCHECK ("del_date"))
	{
		if (dflt_used) 
		{
			local_rec.del_date = 0L; 
			sprintf (local_rec.date_desc, "%-9.9s", "All Dates");	
			DSP_FLD ("del_date");
			DSP_FLD ("date_desc");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchTrhrDate (temp_str);
			FLD ("trip_name") = NA;
			trhr_rec.del_date = atol(temp_str);
			sprintf (trhr_rec.trip_name,"%-12.12s",temp_str + 10);
			strcpy (local_rec.trip_name, trhr_rec.trip_name);
			local_rec.del_date = trhr_rec.del_date;
			DSP_FLD ("trip_name");
			if (local_rec.del_date == 0L || !strcmp (local_rec.trip_name, "            "))      
				FLD ("trip_name") = YES;
			return(0);
		}
		if (local_rec.hhve_hash == 0)
		{
			date_found = FALSE;
			trhr_rec.hhve_hash = local_rec.hhve_hash;
			trhr_rec.del_date  = local_rec.del_date; 
			cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
			while (!cc && !date_found)
			{
				if (local_rec.del_date == trhr_rec.del_date)
					date_found = TRUE;
				else
					cc = find_rec(trhr,&trhr_rec,NEXT,"r");
			}
			if ( !date_found )
			{
				errmess (ML(mlTrMess002));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			trhr_rec.hhve_hash = local_rec.hhve_hash;
			trhr_rec.del_date  = local_rec.del_date; 
			cc = find_rec(trhr,&trhr_rec,EQUAL,"r");
			if (cc)
			{
				errmess (ML(mlTrMess002));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		local_rec.del_date = trhr_rec.del_date;
		sprintf (local_rec.date_desc, "%-9.9s", "         ");	
		DSP_FLD ("del_date");
		DSP_FLD ("date_desc");
	}

	/*--------------------- 
	| Validate Trip Name  | 
	---------------------*/
	if (LCHECK ("trip_name")) 
	{
		if (F_NOKEY(field))
		{
			FLD ("trip_name") = YES;
			return(0);
		}

		FLD ("trip_name") = YES;
		if (dflt_used) 
		{
			sprintf (local_rec.trip_name, "%-12.12s", "All Trips   ");
			DSP_FLD ("trip_name");
			return(0);
		}

		if (SRCH_KEY)
		{
			SrchTrhrTrip (temp_str);
			return(0);
		}
		if (local_rec.hhve_hash == 0)
		{
			trip_found = FALSE;
			trhr_rec.hhve_hash = local_rec.hhve_hash;
			trhr_rec.del_date  = local_rec.del_date; 
			strcpy (trhr_rec.trip_name, local_rec.trip_name); 
			cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
			while (!cc && !trip_found)
			{
				if (!strcmp (trhr_rec.trip_name, local_rec.trip_name)) 
					trip_found = TRUE;
				else
					cc = find_rec(trhr,&trhr_rec,NEXT,"r");
			}
			if (!trip_found)
			{
				errmess (ML(mlTrMess003));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			strcpy (trhr_rec.trip_name, local_rec.trip_name); 
			trhr_rec.hhve_hash = local_rec.hhve_hash;
			trhr_rec.del_date  = local_rec.del_date; 
			cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
			while (!cc)
			{
				if (!strcmp (trhr_rec.trip_name, local_rec.trip_name)) 
					break;
				else
					cc = find_rec(trhr,&trhr_rec,NEXT,"r");
			}
			if (cc)
			{
				errmess (ML(mlTrMess003));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		strcpy (local_rec.trip_name, trhr_rec.trip_name);	
		DSP_FLD ("trip_name");
		local_rec.del_date = trhr_rec.del_date;
		sprintf (local_rec.date_desc, "%-9.9s", "         ");	
		DSP_FLD ("del_date");
		DSP_FLD ("date_desc");
	}

	/*-------------
	| Printer No. | 
	-------------*/
	if (LCHECK("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return(0);
		}
		if (!valid_lp(local_rec.lpno))
		{
			print_mess(ML(mlStdMess020));
			sleep(2);
			clear_mess();
			return(1);
		}
		return(0);
	}
	return(0);
}

void
end_prnt (
 void)
{
	pclose(fout);
}

void
run_prog (
 char *prog_name)
{
	char d_date[11];
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	sprintf(d_date,"%ld",local_rec.del_date);

	shutdown_prog ();
	CloseDB (); 
	execlp(prog_name, prog_name, local_rec.vehi_ref, d_date, local_rec.trip_name, local_rec.sequence, local_rec.lp_str,(char *)0);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_output (
 void)
{
	if ((fout = popen("pformat","w")) == (FILE *) NULL)
		file_err(errno, "pformat", "POPEN" );

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf(fout,".LP%d\n",local_rec.lpno);
	fprintf(fout,".PI16\n");
	fprintf(fout,".6\n");
	fprintf(fout,".L120\n");
	fprintf(fout,".B1\n");
	fprintf(fout,".C%s\n", clip(comm_rec.co_name));
	fprintf(fout,".CTransport Load Report\n\n");
	fprintf(fout,".CAS AT %s\n", SystemTime());			
	fprintf(fout,".R===================");
	fprintf(fout,"==========================================");
	fprintf(fout,"======");
	fprintf(fout,"=============="); 
	fprintf(fout,"========"); 
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");
	fflush (fout); 
}

void
process_file (
 void)
{
	fsort = sort_open("tr_load_rpt");
	strcpy (trve_rec.co_no, comm_rec.co_no);
	strcpy (trve_rec.br_no, comm_rec.est_no);
	strcpy (trve_rec.ref, local_rec.vehi_ref);
	cc = find_rec (trve,&trve_rec,GTEQ,"r");
	while (!cc 
		&& !strcmp (trve_rec.co_no, comm_rec.co_no) 
		&& !strcmp (trve_rec.br_no, comm_rec.est_no) 
		&& (!strcmp (trve_rec.ref, local_rec.vehi_ref) 
				|| !strcmp (local_rec.vehi_ref, "          ")))
	{
			trhr_rec.hhve_hash = trve_rec.hhve_hash;
			trhr_rec.del_date  = local_rec.del_date;
            strcpy (trhr_rec.trip_name, local_rec.trip_name);
			cc = find_rec (trhr, &trhr_rec, GTEQ,"r");
			while (!cc 
				&& trhr_rec.hhve_hash == trve_rec.hhve_hash 
				&& (trhr_rec.del_date == local_rec.del_date 
						|| local_rec.del_date == 0l) 
				&& (!strcmp (local_rec.trip_name, "All Trips   ") 
						|| !strcmp (trhr_rec.trip_name,local_rec.trip_name)))	
			{
				get_extf();
				trln_rec.hhtr_hash = trhr_rec.hhtr_hash;
				cc = find_rec (trln, &trln_rec, GTEQ, "r");
				while (!cc && trln_rec.hhtr_hash == trhr_rec.hhtr_hash)
				{
					get_cohr_cumr();
					coln_rec.hhco_hash = trln_rec.hhco_hash;
					coln_rec.line_no = 0;	
					cc = find_rec (coln, &coln_rec, GTEQ, "r");
					while (!cc && coln_rec.hhco_hash == trln_rec.hhco_hash)
					{
						get_inmr_inum();
						store_data();
						cc = find_rec (coln, &coln_rec, NEXT, "r");
					}
					cc = find_rec (trln, &trln_rec, NEXT, "r");
				}
				cc = find_rec (trhr, &trhr_rec, NEXT,"r");
	    	}
		    cc = find_rec (trve, &trve_rec, NEXT,"r");
   	}
    display_report();
    fflush (fout);
    sort_delete (fsort,"tr_load_rpt");
}

void
get_extf (
 void)
{
	strcpy (extf_rec.co_no, comm_rec.co_no);
	strcpy (extf_rec.code, trhr_rec.driver);
	cc = find_rec (extf, &extf_rec, EQUAL, "r");
	if (!cc)
	{
		sprintf ( driver_code, "%-6.6s", extf_rec.code);
		sprintf ( driver_name, "%-40.40s", extf_rec.name);
	}
}

void
get_cohr_cumr (
 void)
{
	cohr_rec.hhco_hash = trln_rec.hhco_hash;
	cc = find_rec (cohr, &cohr_rec, EQUAL, "r");
	if (!cc)  	
	{
		cumr_rec.hhcu_hash = cohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (!cc)
		{
			sprintf ( cust_no, "%-6.6s", cumr_rec.dbt_no);
			sprintf ( cust_name, "%-40.40s", cumr_rec.dbt_name);
		}
	}	
}

void					
get_inmr_inum (
 void)
{
	float 	std_cnv_fct = (float) 0,
			cnv_fct;
	
	inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (!cc)
	{
		sprintf ( it_no,   "%-16.16s", inmr_rec.item_no);
		sprintf ( it_desc, "%-40.40s", inmr_rec.description);
		inum_rec.hhum_hash = inmr_rec.std_uom;	
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (!cc)
		{	
			std_cnv_fct = inum_rec.cnv_fct;
		}
	}
	inum_rec.hhum_hash = coln_rec.hhum_hash;
	cc = find_rec(inum, &inum_rec, EQUAL, "r");
	if (!cc)
	{
		sprintf ( pk_uom, "%-4.4s", inum_rec.uom);

		if 	( std_cnv_fct == 0.00 )
			std_cnv_fct = 1;

		cnv_fct		=	inum_rec.cnv_fct/std_cnv_fct;
		std_q_order	=	coln_rec.q_order/cnv_fct;
		std_qty_del	=	coln_rec.qty_del/cnv_fct;

	}
}

void
store_data (
 void)
{
	char data_string [350];
	sprintf (data_string,
			 "%-2.2s %-40.40s %-12.12s %-16.16s %-40.40s %-6.6s %-40.40s %-4.4s %-8.8s %-10.2f %-10.2f %-10.10s %-6.6s %-40.40s %-11.11ld\n",
			 comm_rec.est_no, 
			 comm_rec.est_name,
			 trhr_rec.trip_name,
			 it_no, 
			 it_desc,
			 cust_no, 
			 cust_name, 
			 pk_uom, 
			 cohr_rec.inv_no, 
			 std_qty_del,
			 std_q_order, 
			 trve_rec.ref,
			 driver_code, 
			 driver_name, 
			 trhr_rec.del_date);
	sort_save (fsort, data_string);
}

void
display_report (
 void)
{
	int 	first_time = TRUE,
			first_item = TRUE;
	char	str_ord[11];
	char	str_del[11];

	memset (prev_ref,  0, sizeof (prev_ref));
	memset (prev_item, 0, sizeof (prev_item));
	memset (prev_dbt,  0, sizeof (prev_dbt));
	memset (prev_trip_name, 0, sizeof (prev_trip_name));

	prev_del_date = 0L;
	fsort =  sort_sort(fsort, "tr_load_rpt");
	sptr  =  sort_read(fsort);
	while (sptr != (char *)0)
	{
		sprintf (est_no,      "%-2.2s",   sptr);
		sprintf (est_name,    "%-40.40s", sptr + 3);	
		sprintf (this_trip,   "%-12.12s", sptr + 44);   
		sprintf (item_no,     "%-16.16s", sptr + 57);
		sprintf (description, "%-40.40s", sptr + 74);
		sprintf (dbt_no,      "%-6.6s",   sptr + 115);
		sprintf (dbt_name,    "%-40.40s", sptr + 122);   
		sprintf (uom,         "%-4.4s",   sptr + 163);
		sprintf (inv_no,      "%-8.8s",   sptr + 168);
		qty_del  = atof(sptr + 177);
		q_order  = atof(sptr + 188);
		sprintf (ref,         "%-10.10s", sptr + 199);
		sprintf (dr_code,     "%-6.6s",   sptr + 210);
		sprintf (dr_name,     "%-40.40s", sptr + 217);   
		del_date = atol(sptr + 258);
		sprintf (d_date,      "%-10.10s", DateToString (del_date));
		sprintf (str_ord,  "%-10.10s", comma_fmt((double)q_order,"NNNNNN.NN"));
		sprintf (str_del,  "%-10.10s", comma_fmt((double)qty_del,"NNNNNN.NN"));

		if (strcmp (prev_trip_name, this_trip) ) 
		{
			if (!first_time)
			{
				head_display (ref, d_date, this_trip, dr_code, dr_name);
				print_tail();
				fprintf (fout, ".PA\n");
			}
			else
			{
				head_display (ref, d_date, this_trip, dr_code, dr_name);
				first_time = FALSE;
			}
			strcpy (prev_trip_name, this_trip);
		}
		
		if (strcmp (prev_item, item_no) )
		{
			memset (prev_dbt,0,sizeof (prev_dbt));
			if (first_item == TRUE)
			{
				fprintf (fout,"| %-16.16s ", item_no);
				fprintf (fout," %-40.40s ", description);
				fprintf (fout," %-4.4s ", " ");
				fprintf (fout," %-40.40s       |\n", " ");
				strcpy (prev_item, item_no);
				first_item = FALSE;
			}
			else
			{
				fprintf (fout,"| %-16.16s ", item_no);
				fprintf (fout," %-40.40s ", description);
				fprintf (fout," %-4.4s ", " ");
				fprintf (fout," %-40.40s       |\n", " ");
				first_item = FALSE;
				strcpy (prev_item, item_no);
			}
			if (strcmp (prev_item, item_no) )
				strcpy (prev_item, item_no);

			if (strcmp (prev_dbt, dbt_no)) 
			{
				fprintf (fout,"|    %-6.6s        ", dbt_no);
				fprintf (fout," %-40.40s ",   dbt_name);
				fprintf (fout," %-4.4s ",     uom);
				fprintf (fout,"  %-10.10s   ", str_ord);
				if (qty_del == 0)
				{
					strcpy (q_load, "__________");	
					fprintf (fout,"         %-10.10s   ", q_load);
				}		
				else
					fprintf (fout,"         %-10.10s   ", str_del);
				fprintf (fout," %-8.8s  |\n", inv_no); 	
				strcpy (prev_dbt, dbt_no);
			}
		}
		else
		{
			if (strcmp (prev_dbt, dbt_no)) 
			{
				fprintf (fout,"|    %-6.6s        ", dbt_no);
				fprintf (fout," %-40.40s ",   dbt_name);
				fprintf (fout," %-4.4s ",     uom);
				fprintf (fout,"  %-10.10s   ", str_ord);
				if (qty_del == 0)
				{
					strcpy (q_load, "__________");	
					fprintf (fout,"         %-10.10s   ", q_load);
				}		
				else
					fprintf (fout,"         %-10.10s   ", str_del);
				fprintf (fout," %-8.8s  |\n", inv_no); 	
				strcpy (prev_dbt, dbt_no);
			}
			else
			{
				fprintf (fout,"|    %-6.6s        ", "      ");
				fprintf (fout," %-40.40s ",   " ");
				fprintf (fout," %-4.4s ",     uom);
				fprintf (fout,"  %-10.10s   ", str_ord);
				if (qty_del == 0)
				{
					strcpy (q_load, "__________");	
					fprintf (fout,"         %-10.10s   ", q_load);
				}		
				else
					fprintf (fout,"         %-10.10s   ", str_del);
				fprintf (fout," %-8.8s  |\n", inv_no); 	
				strcpy (prev_dbt, dbt_no);
			}
			
		}
		sptr = sort_read (fsort);
	}
	print_tail();
}

void
print_tail (
 void)
{
	fprintf(fout,"===================");
	fprintf(fout,"==========================================");
	fprintf(fout,"======");
	fprintf(fout,"=============="); 
	fprintf(fout,"========"); 
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");
	fprintf(fout,"| PREPARED BY:                 WAREHOUSE CHECKER            WAREHOUSE FOREMAN               RECEIVED BY            |\n");
	fprintf(fout,"|                                                                                                                  |\n");
	fprintf(fout,"|                                                                                                                  |\n");
	fprintf(fout,"| ______________________       ______________________       ______________________          ______________________ |\n");
	fprintf(fout,"| Sign over printed name       Sign over printed name       Sign over printed name          Sign over printed name |\n");

}

void
head_display (
 char *vehi_no, 
 char *deli_date, 
 char *trip, 
 char *dcode, 
 char *driver)
{
	fprintf (fout, ".DS7\n");	/*7*/
	fprintf(fout,"===================");
	fprintf(fout,"==========================================");
	fprintf(fout,"======");
	fprintf(fout,"=============="); 
	fprintf(fout,"========"); 
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");
	fprintf(fout,"| VEHICLE NO :  %-10.10s                                                                                         |\n", vehi_no);
	fprintf(fout,"| TRIP NAME  :  %-12.12s", trip);
	fprintf(fout,"                      EXPECTED DELIVERY DATE :  %-8.8s                               |\n", deli_date);
	fprintf(fout,"| TRUCKER    :  %-6.6s    %-40.40s                                                 |\n", dcode, driver);
	fprintf(fout,"===================");
	fprintf(fout,"==========================================");
	fprintf(fout,"======");
	fprintf(fout,"=============="); 
	fprintf(fout,"========"); 
	fprintf(fout,"=============");
	fprintf(fout,"==============\n");
	fprintf(fout,"|   ITEM NUMBER    ");
	fprintf(fout,"|              DESCRIPTION                "); 
	fprintf(fout,"| UOM "); 
	fprintf(fout,"| ORDER QTY   ");
	fprintf(fout,"|       ");
	fprintf(fout,"| LOAD QTY   ");
	fprintf(fout,"|   P/S      |\n");
	fprintf(fout,"|------------------");
	fprintf(fout,"|-----------------------------------------");
	fprintf(fout,"|-----");
	fprintf(fout,"|-------------"); 
	fprintf(fout,"|-------"); 
	fprintf(fout,"|------------");
	fprintf(fout,"|-------------\n");
	fflush(fout);
}

int
heading (
 int scn)
{
	if ( restart ) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set(scn);    
	clear();
	rv_pr(ML(mlTrMess050),29,0,1);
	move(0,1);
	line(80);
	box(0,3,80,9);
	move(1,5);
	line(79);	
	move(1,7);
	line(79);
	move(1,9);
	line(79);
	move(1,11);
	line(79);	
	print_at (21,1,ML(mlStdMess038), curr_co, 
				clip (comm_rec.co_name));
	move(0,22);
	line(80);
	line_cnt = 0;
	scn_write(scn);   
	return (EXIT_SUCCESS);
}


/*=====================================
| Search for Transport Vehicle File . |
=====================================*/
void
SrchTrve (
 char *key_val)
{
	work_open();
	strcpy(trve_rec.co_no, comm_rec.co_no);
    strcpy(trve_rec.br_no, comm_rec.est_no); 
	sprintf(trve_rec.ref,"%-10.10s",key_val);
	save_rec("#Vehicle #","#Description");
	cc = find_rec(trve,&trve_rec,GTEQ,"r");
	while (!cc && !strcmp( trve_rec.co_no, comm_rec.co_no) &&
				  !strcmp( trve_rec.br_no, comm_rec.est_no) &&
	      		  !strncmp(trve_rec.ref,key_val,strlen(key_val)))
	{
		cc = save_rec(trve_rec.ref,trve_rec.desc);
		if (cc)
			break;
		cc = find_rec(trve,&trve_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy( trve_rec.co_no, comm_rec.co_no);
    strcpy(trve_rec.br_no, comm_rec.est_no); 
	sprintf(trve_rec.ref,"%-10.10s", temp_str);
	cc = find_rec(trve,&trve_rec,COMPARISON,"r");
	if (cc)
		file_err(cc, trve, "DBFIND" );
}

/*==================================================
| Search for Delivery Date Transport Header File . |
==================================================*/
void
SrchTrhrDate (
 char *key_val)
{
	char date_trp[25];
	work_open();
	save_rec("#Del Date    Trip Name", "#");
	trhr_rec.hhve_hash = local_rec.hhve_hash;
	trhr_rec.del_date  = StringToDate(key_val);
	cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
	while (!cc &&  !strncmp(DateToString(trhr_rec.del_date),key_val,strlen(key_val)) && (trhr_rec.hhve_hash == local_rec.hhve_hash || local_rec.hhve_hash == 0L))
	{
		sprintf (date_trp,"%-10.10s  %-12.12s",DateToString(trhr_rec.del_date),trhr_rec.trip_name);
		cc = save_rec(date_trp,"");
		if (cc)
			break;
		cc = find_rec(trhr,&trhr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	{
		sprintf (err_str,ML(mlTrMess007), trve_rec.ref); 
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess();
		return;
	}
	if (local_rec.hhve_hash > 0)
	{
		trhr_rec.hhve_hash = local_rec.hhve_hash;
		trhr_rec.del_date = StringToDate(temp_str);
		cc = find_rec(trhr,&trhr_rec,COMPARISON,"r");
		if (cc)
			file_err(cc, trhr, "DBFIND" );
	}
}

/*================================================
| Search for Trip Number Transport Header File . |
================================================*/
void
SrchTrhrTrip (
 char *key_val)
{
	work_open();
	save_rec("#Trip Number", "#Del Date");
	sprintf (trhr_rec.trip_name, "%-12.12s", key_val);
	trhr_rec.hhve_hash = local_rec.hhve_hash;
	trhr_rec.del_date  = local_rec.del_date; 
	cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
	while (!cc &&  !strncmp (trhr_rec.trip_name,key_val,strlen(key_val)) && 
		(trhr_rec.hhve_hash == local_rec.hhve_hash || 
		local_rec.hhve_hash == 0L) && 
		(trhr_rec.del_date == local_rec.del_date || local_rec.del_date == 0L)) 
	{
		strcpy (err_str,DateToString (trhr_rec.del_date));
		cc = save_rec(trhr_rec.trip_name, err_str);
		if (cc)
			break;
		cc = find_rec(trhr,&trhr_rec,NEXT,"r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
	{
		print_mess (ML(mlTrMess003));
		sleep (sleepTime);
		clear_mess();
		return;
	}
	if (local_rec.hhve_hash > 0)
	{
		sprintf (trhr_rec.trip_name, "%-12.12s", temp_str);
		trhr_rec.hhve_hash = local_rec.hhve_hash;
		trhr_rec.del_date  = local_rec.del_date; 
		cc = find_rec(trhr,&trhr_rec,GTEQ,"r");
		while (!cc)
		{
			if (!strcmp (trhr_rec.trip_name, temp_str)) 
					break;
			else
				cc = find_rec(trhr,&trhr_rec,NEXT,"r");
		}
		if (cc)
			file_err(cc, trhr, "DBFIND" );
	}
}
