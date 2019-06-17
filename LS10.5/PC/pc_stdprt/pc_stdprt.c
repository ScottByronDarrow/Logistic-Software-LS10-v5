/*====================================================================|
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_stdprt.c    )                                 |
|  Program Desc  : ( Receipt Costing Printout.                    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  bmms, comm, inei, inmr, inum,     ,     ,     ,   |
|                :  pcwc, rghr, rgln, rgrs,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :      ,     ,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (08/06/92)      | Author       : Simon Dubey.      |
|---------------------------------------------------------------------|
|  Date Modified : (10/04/94)      | Modified  by : Roel Michels      |
|  Date Modified : (04/09/1997)    | Modified  by : Jiggs A Veloz     |
|                                                                     |
|  Comments      :                                                    |
|                :                                                    |
|  This was modifed by Trev just before he left SC KIL 6525. put      |
|  back into sccs by simon.                                           |
|  (10/04/94)    : PSL 10673 - Online conversion                      |
|                :                                                    |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at. Changed dates from char8 to 10.      |
|                :                                                    |
| $Log: pc_stdprt.c,v $
| Revision 5.3  2002/07/17 09:57:29  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:14:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:05  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:25  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:34  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:17:07  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:14  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.12  2000/07/14 02:35:53  scott
| Updated as general maintenance to add app.schema and correct item search.
|
| Revision 1.11  1999/11/12 10:37:48  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.10  1999/10/20 02:06:54  nz
| Updated for final changes on date routines.
|
| Revision 1.9  1999/09/29 10:11:38  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/17 08:26:25  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.7  1999/09/13 07:03:17  marlene
| *** empty log message ***
|
| Revision 1.6  1999/09/09 06:12:33  marlene
| *** empty log message ***
|
| Revision 1.5  1999/06/17 07:40:47  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_stdprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_stdprt/pc_stdprt.c,v 5.3 2002/07/17 09:57:29 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<get_lpno.h>
#include	<ml_pc_mess.h>
#include	<ml_std_mess.h>

FILE	*fout;

#include	"schema"

struct bmmsRecord	bmms_rec;
struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcwcRecord	pcwc_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct rgrsRecord	rgrs_rec;

	char	*data	= "data",
			*inmr2	= "inmr2";

	int		pformat_open = FALSE;


int	first_time = TRUE;
struct
{
	int		seq_no;
	long	hhwc_hash;
	double	rate;
	double	ovhd_var;
	double	ovhd_fix;
	long	time;
	int		qty_rsrc;
	char	type [2];
	int		instr_no;
} store [MAXLINES];
int	no_stored;

struct
{
	char	type;
	char	desc [9];
} res_type [] = {
	{'L', "Labour"},
	{'M', "Machine"},
	{'Q', "QC Check"},
	{'O', "Other"},
	{'S', "Special"},
	{0, ""},
};
int	no_res = 5;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	systemDate [11];
	long	lsystemDate;

	long	st_hash;
	char	st_item_no [17];
	char	st_desc [41];
	int		st_alt_no;
	long	end_hash;
	char	end_item_no [17];
	char	end_desc [41];
	int		end_alt_no;
	int		lpno;
	char	back [4];
	char	onight [4];

	char	res_desc [9];
	char	std_uom [5];
	char	alt_uom [5];
	char	mtl_uom [5];
	float	std_batch;
	float	min_batch;
	float	max_batch;
	double	tot_cost;
	long	tot_time;

	double	mat_tot_std;
	double	rtg_res_std;
	double	rtg_ovh_std;
	double	bch_tot_std;
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "st_item_no",	 4, 14, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " Start Item     ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.st_item_no},
	{1, LIN, "st_desc",	 4, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.st_desc},
	{1, LIN, "end_item_no",	 5, 14, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", " End Item       ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.end_item_no},
	{1, LIN, "end_desc",	 5, 34, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.end_desc},
	{1, LIN, "st_alt_no",	 7, 14, INTTYPE,
		"NN", "          ",
		" ", " 1", " From Alternate ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.st_alt_no},
	{1, LIN, "end_alt_no",	 8, 14, INTTYPE,
		"NN", "          ",
		" ", " 1", " To Alternate   ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.end_alt_no},
	{1, LIN, "lpno",		 9, 14, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer No     ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================
| function prototypes |
=====================*/

void shutdown_prog (void);
void ReadMisc (void);
void OpenDB (void);
void CloseDB (void);
int heading (int scn);
int spec_valid (int field);
void process (void);
void proc_bom (void);
void head_output (void);
void print_inmr (void);
void print_bom (void);
void proc_bmms (void);
void print_rtg (void);
void proc_lines (void);
void calc_vals (void);
double get_uom (long int iss_uom);
void print_summary (void);
/*===========================
| Main Processing Routine . |
===========================*/
int
main(
 int  argc, 
 char *argv [])
{
	SETUP_SCR (vars);

	if (argc != 2 )
	{
		/*----------------------------
		| Usage : %s <description>\n |
		----------------------------*/
		print_at (0,0, mlPcMess712, argv [0]);
		return (EXIT_FAILURE);;
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();
	OpenDB ();
	ReadMisc ();
	

	/*-------------------------------
	| Setup required parameters	|
	-------------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

	/*=======================================
	| Beginning of input control loop	|
	=======================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags	|
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		init_vars (1);
		/*-------------------------------
		| Enter screen 1 linear input	|
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);  
		if (restart)
			continue;

		dsp_screen ("Printing Manufactured Products Report ", comm_rec.co_no, comm_rec.co_name);

		process ();

		if (pformat_open)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog(
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*===============================================
| Get common info from commom database file	|
===============================================*/
void
ReadMisc(
 void)
{


	read_comm( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB(
 void)
{
	abc_dbopen (data);
	abc_alias(inmr2, inmr);

	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB(
 void)
{
	abc_fclose (bmms);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (pcwc);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (rgrs);

	abc_dbclose (data);
}

/*=======================
| Display heading scrn	|
=======================*/
int
heading(
 int scn)
{
	if (!restart)
	{
		clear ();
		/*------------------------------------------
		| %R Manufactured Products Costing Report |
		------------------------------------------*/
		centre_at (0, 80, ML(mlPcMess088) );

		box (0, 3, 80, 6);
		move (0, 1);
		line (80);

		move (0, 6);
		PGCHAR (10);
		line (79);
		PGCHAR (11);

		move (0, 21);
		line (80);
		print_at (22, 0, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);

		scn_set (scn);

		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;

		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

/*===============================
| Validate entered field(s)	|
===============================*/
int
spec_valid(
 int field)
{
	if (LCHECK ("st_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (local_rec.st_item_no, "       ");
			sprintf (local_rec.st_desc, "%-35.35s", " ");
			DSP_FLD ("st_item_no");
			DSP_FLD ("st_desc");
			if (strcmp (local_rec.st_item_no, local_rec.end_item_no))
			{
				local_rec.st_alt_no 	= 1;
				local_rec.end_alt_no 	= 1;
				FLD ("st_alt_no") 		= NA;
				FLD ("end_alt_no") 		= NA;
				DSP_FLD ("st_alt_no");
				DSP_FLD ("end_alt_no");
			}
			else
			{
				FLD ("st_alt_no") 		= YES;
				FLD ("end_alt_no") 		= NO;
			}
			return (EXIT_SUCCESS);
		}
		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.st_item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.st_item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.st_hash = inmr_rec.hhbr_hash;
		sprintf(local_rec.st_desc, "%-40.40s", inmr_rec.description);

		DSP_FLD ("st_item_no");
		DSP_FLD ("st_desc");

		if (strcmp (local_rec.st_item_no, local_rec.end_item_no))
		{
			local_rec.st_alt_no 	= 1;
			local_rec.end_alt_no 	= 1;
			FLD ("st_alt_no") 		= NA;
			FLD ("end_alt_no") 		= NA;
			DSP_FLD ("st_alt_no");
			DSP_FLD ("end_alt_no");
		}
		else
		{
			FLD ("st_alt_no") 	= YES;
			FLD ("end_alt_no") 	= NO;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.end_item_no, "~~~~~~~~~~~~~~~");
			strcpy (local_rec.end_desc, " ");
			DSP_FLD ("end_item_no");
			DSP_FLD ("end_desc");
			if (strcmp (local_rec.st_item_no, local_rec.end_item_no))
			{
				local_rec.st_alt_no 	= 1;
				local_rec.end_alt_no 	= 1;
				FLD ("st_alt_no") 		= NA;
				FLD ("end_alt_no") 		= NA;
				DSP_FLD ("st_alt_no");
				DSP_FLD ("end_alt_no");
			}
			else
			{
				FLD ("st_alt_no") 	= YES;
				FLD ("end_alt_no") 	= NO;
			}
			return (EXIT_SUCCESS);
		}

		clear_mess ();
	
		cc = FindInmr (comm_rec.co_no, local_rec.end_item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.end_item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.end_hash = inmr_rec.hhbr_hash;
		sprintf (local_rec.end_desc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("end_item_no");
		DSP_FLD ("end_desc");

		if (strcmp (local_rec.st_item_no, local_rec.end_item_no))
		{
			local_rec.st_alt_no 	= 1;
			local_rec.end_alt_no 	= 1;
			FLD ("st_alt_no") 		= NA;
			FLD ("end_alt_no") 		= NA;
			DSP_FLD ("st_alt_no");
			DSP_FLD ("end_alt_no");
		}
		else
		{
			FLD ("st_alt_no") 		= YES;
			FLD ("end_alt_no") 		= NO;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("st_alt_no"))
	{
		if (FLD ("st_alt_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.st_alt_no = 1;
			DSP_FLD ("st_alt_no");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_alt_no"))
	{
		if (FLD ("end_alt_no") == NA)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.end_alt_no = local_rec.st_alt_no;
			DSP_FLD ("end_alt_no");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*-------------------------
			| Invalid Printer NUmber | 
			-------------------------*/
			print_mess ( ML(mlStdMess020) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*---------------------------------
| Print BOM and Routing for all   |
| products in the specified range |
---------------------------------*/
void
process(
 void)
{
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", local_rec.st_item_no);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");

	while
	(
		!cc &&
		!strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		strcmp (inmr_rec.item_no, local_rec.end_item_no) <= 0
	)
	{
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			inei_rec.std_batch = 0.00;
			inei_rec.min_batch = 0.00;
			inei_rec.max_batch = 0.00;
		}

		local_rec.std_batch = inei_rec.std_batch;
		local_rec.min_batch = inei_rec.min_batch;
		local_rec.max_batch = inei_rec.max_batch;

		dsp_process ("Item No :", inmr_rec.item_no);

		/*----------------
		| get routing no |
		----------------*/
		strcpy (rghr_rec.co_no, comm_rec.co_no); 
		strcpy (rghr_rec.br_no, comm_rec.est_no); 
		rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
		rghr_rec.alt_no = local_rec.st_alt_no;
		cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
		while
		(
			!cc &&
			!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
			!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
			rghr_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			rghr_rec.alt_no <= local_rec.end_alt_no
		)
		{
			proc_bom ();
			cc = find_rec (rghr, &rghr_rec, NEXT, "r");
		}

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

void
proc_bom(
 void)
{
	/*-------------------------------
	| If no bom exists to match the	|
	| route, ignore the route!.	|
	-------------------------------*/
	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
	bmms_rec.alt_no = rghr_rec.alt_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if
	(
		cc ||
		strcmp (bmms_rec.co_no, comm_rec.co_no) ||
		bmms_rec.hhbr_hash != inmr_rec.hhbr_hash ||
		bmms_rec.alt_no != rghr_rec.alt_no
	)
		return;

	/*---------------
	| Print Report	|
	---------------*/
	if (first_time)
	{
		first_time = FALSE;
		head_output ();
	}

	print_inmr ();
	print_bom ();
	print_rtg ();
	print_summary ();
}

/*---------------------
| Prepare Output Pipe |
---------------------*/
void
head_output(
 void)
{
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	pformat_open = TRUE;

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.lpno);

	fprintf (fout, ".9\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".ECOMPANY %s : %s\n",
		comm_rec.co_no, clip (comm_rec.co_name));

	fprintf (fout, ".EBRANCH %s : %s\n",
		comm_rec.est_no, clip (comm_rec.est_name));

	fprintf (fout, ".EWAREHOUSE %s : %s\n",
		comm_rec.cc_no, clip (comm_rec.cc_name));

	fprintf (fout, ".B1\n");

	fprintf (fout, ".EMANUFACTURED PRODUCTS COSTING REPORT\n");

	fprintf (fout, ".R=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");
}

/*-------------------
| Print inmr Details |
--------------------*/
void
print_inmr(
 void)
{
	fprintf (fout, "=====================================================");
	fprintf (fout, "=====================================================");
	fprintf (fout, "===================================================\n");

	fprintf (fout, "|%-155.155s|\n", " ");

	fprintf (fout, "| ITEM NUMBER : ");
	fprintf (fout, "%-16.16s%-45.45s", inmr_rec.item_no, " ");

	fprintf (fout, "STRENGTH          : ");
	fprintf (fout, "%-5.5s%-54.54s|\n", &inmr_rec.description [35], " ");

	fprintf (fout, "| DESCRIPTION : ");
	fprintf (fout, "%-35.35s %-25.25s", inmr_rec.description, " ");
	fprintf (fout, "BOM ALTERNATE     ");
	fprintf (fout, ": %2d%-57.57s|\n", bmms_rec.alt_no, " ");

	fprintf (fout, "| DESCRIPTION ");
	fprintf (fout, ": %-40.40s %-20.20s", inmr_rec.description2, " ");

	fprintf (fout, "ROUTING ALTERNATE ");
	fprintf (fout, ": %2d%-57.57s|\n", rghr_rec.alt_no, " ");

	fprintf (fout, "|%-155.155s|\n", " ");

	/*----------------
	| Lookup std uom |
	----------------*/
	inum_rec.hhum_hash = inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		strcpy (inum_rec.uom, "UNKN");
	sprintf (local_rec.std_uom, "%-4.4s", inum_rec.uom);
	fprintf (fout, "| STANDARD UNIT OF MEASURE : ");
	fprintf (fout, "%-4.4s%-44.44s", inum_rec.uom, " ");

	/*----------------
	| Lookup alt uom |
	----------------*/

	cc = find_hash (inum, &inum_rec, COMPARISON, "r", inmr_rec.alt_uom);
	if (cc)
		strcpy (inum_rec.uom, "UNKN");

	sprintf (local_rec.alt_uom, "%-4.4s", inum_rec.uom);
	fprintf (fout, "ALTERNATE UNIT OF MEASURE : ");
	fprintf (fout, "%-4.4s%-47.47s|\n", inum_rec.uom, " ");

	fprintf (fout, "|%-155.155s|\n", " ");
	fprintf (fout, "| STANDARD BATCH : ");
	fprintf (fout, "%14.6f%-30.30s", local_rec.std_batch, " ");

	fprintf (fout,"MINIMUM BATCH : %14.6f%30.30s", local_rec.min_batch," ");

	fprintf (fout, "MAXIMUM BATCH : %14.6f   |\n", local_rec.max_batch);

	fprintf (fout, "|----------------------------------------------------");
	fprintf (fout, "-----------------------------------------------------");
	fprintf (fout, "--------------------------------------------------|\n");
}

/*-------------------------
| Print Bill Of Materials |
-------------------------*/
void
print_bom(
 void)
{
	fprintf (fout,".DS4\n");

	fprintf (fout,".EBILL OF MATERIALS\n");

	fprintf (fout, "|-------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------------------------");
	fprintf (fout, "--------");
	fprintf (fout, "-------");
	fprintf (fout, "-----------------");
	fprintf (fout, "------------");
	fprintf (fout, "--------------");
	fprintf (fout, "--------------------|\n");

	fprintf (fout, "|        MATERIAL        |");
	fprintf (fout, "           D E S C R I P T I O N           |");
	fprintf (fout, "   STRNGTH   |");
	fprintf (fout, "    UOM     |");
	fprintf (fout, "     STANDARD QTY.    |");
	fprintf (fout, "    STD. COST    |");
	fprintf (fout, "     EXT. STD.    |\n");

	fprintf (fout, "|------------------------+");
	fprintf (fout, "-------------------------------------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "------------+");
	fprintf (fout, "----------------------+");
	fprintf (fout, "-----------------+");
	fprintf (fout, "------------------|\n");

	fprintf (fout,".EBILL OF MATERIALS\n");

	fprintf (fout, "|-------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------------------------");
	fprintf (fout, "--------");
	fprintf (fout, "-------");
	fprintf (fout, "-----------------");
	fprintf (fout, "------------");
	fprintf (fout, "--------------");
	fprintf (fout, "--------------------|\n");

	fprintf (fout, "|        MATERIAL        |");
	fprintf (fout, "           D E S C R I P T I O N           |");
	fprintf (fout, "   STRNGTH   |");
	fprintf (fout, "    UOM     |");
	fprintf (fout, "     STANDARD QTY.    |");
	fprintf (fout, "    STD. COST    |");
	fprintf (fout, "     EXT. STD.    |\n");

	fprintf (fout, "|------------------------+");
	fprintf (fout, "-------------------------------------------+");
	fprintf (fout, "-------------+");
	fprintf (fout, "------------+");
	fprintf (fout, "----------------------+");
	fprintf (fout, "-----------------+");
	fprintf (fout, "------------------|\n");

	proc_bmms ();

	return;
}

/*----------------------
| Process bmms records |
----------------------*/
void
proc_bmms(
 void)
{
	double	tot_std = 0.00,
		tmp_val;
	double	get_uom (long int iss_uom),
		cnv_fct;

	cc = 0;
	while
	(
		!cc &&
		inmr_rec.hhbr_hash == bmms_rec.hhbr_hash &&
		bmms_rec.alt_no == rghr_rec.alt_no
	)
	{
		cc = find_hash (inmr2, &inmr2_rec, COMPARISON, "r", bmms_rec.mabr_hash);
		if (cc)
		{
			cc = find_rec (bmms, &bmms_rec, NEXT, "r");
			continue;
		}

		inei_rec.hhbr_hash = inmr2_rec.hhbr_hash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		cc = find_rec (inei, &inei_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (inei_rec.hzrd_class, "UNKN");
			inei_rec.std_cost = (double) 0.00;
		}

		cnv_fct = get_uom (bmms_rec.uom);
		cc = find_hash (inum, &inum_rec, EQUAL, "r", bmms_rec.uom);
		strcpy (local_rec.mtl_uom, (cc) ? "    " : inum_rec.uom);

		fprintf (fout, ".LRP3\n");

		fprintf (fout, "| %-22.22s ",	inmr2_rec.item_no);
		fprintf (fout, "| %-41.41s ",	inmr2_rec.description);
		fprintf (fout, "| %-11.11s ",	&inmr2_rec.description [35]);
		fprintf (fout, "| %-10.10s ",	local_rec.mtl_uom);
		fprintf (fout, "| %20.6f ",	bmms_rec.matl_qty);

		bmms_rec.matl_wst_pc += 100.00;
		bmms_rec.matl_wst_pc /= 100.00;
		inei_rec.std_cost *= bmms_rec.matl_wst_pc;
		inei_rec.std_cost /= cnv_fct;

		fprintf (fout, "| %15.2f ",	inei_rec.std_cost);

		tmp_val = inei_rec.std_cost * bmms_rec.matl_qty;
		tot_std += tmp_val;

		fprintf (fout, "| %16.2f ",	tmp_val);
		fprintf (fout, "|\n");

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	local_rec.mat_tot_std = CENTS (tot_std);

	fflush (fout);
	return;
}

/*---------------
| Print Routing |
---------------*/
void
print_rtg(
 void)
{
	calc_vals ();

	fprintf (fout, ".DS9\n");
	fprintf (fout,".B1\n");
	fprintf (fout, ".EMANUFACTURED PRODUCTS COSTING REPORT\n");
	fprintf (fout,".B1\n");
	fprintf (fout, ".EROUTING DETAILS\n");
	fprintf (fout, "%20.20s"," ");
	fprintf (fout, "     Product Code: %-16.16s  %-35.35s  ",
		inmr_rec.item_no, inmr_rec.description);

	fprintf (fout, "Alternate No : %d     \n", rghr_rec.alt_no);

	fprintf (fout, "========================================");
	fprintf (fout, "========================================");
	fprintf (fout, "========================================");
	fprintf (fout, "======================================\n");

	fprintf (fout, "%20.20s"," ");
	fprintf (fout, "|SEQ");
	fprintf (fout, "|  WORK  ");
	fprintf (fout, "|RESOURCE");
	fprintf (fout, "|        RESOURCE        ");
	fprintf (fout, "| RESOURCE ");
	fprintf (fout, "|NO ");
	fprintf (fout, "|   RATE  ");
	fprintf (fout, "|  FIXED  ");
	fprintf (fout, "|    S T A N D A R D    ");
	fprintf (fout, "|   STANDARD |\n");

	fprintf (fout, "%20.20s"," ");
	fprintf (fout, "|NO ");
	fprintf (fout, "| CENTRE ");
	fprintf (fout, "|        ");
	fprintf (fout, "|      DESCRIPTION       ");
	fprintf (fout, "|   TYPE   ");
	fprintf (fout, "|OPS");
	fprintf (fout, "| PER HOUR");
	fprintf (fout, "| OVERHEAD");
	fprintf (fout, "| SETUP ");
	fprintf (fout, "|  RUN  ");
	fprintf (fout, "| CLEAN ");
	fprintf (fout, "|     COST   |\n");

	fprintf (fout, "%20.20s"," ");
	fprintf (fout, "|---");
	fprintf (fout, "+--------");
	fprintf (fout, "+--------");
	fprintf (fout, "+------------------------");
	fprintf (fout, "+----------");
	fprintf (fout, "+---");
	fprintf (fout, "+---------");
	fprintf (fout, "+---------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+-------");
	fprintf (fout, "+------------|\n");

	fprintf (fout,".PA\n");

	proc_lines ();

	return;
}

/*--------------------
| Process rgln lines |
--------------------*/
void
proc_lines(
 void)
{
	int	i;
	long	std_tmp_time;
	double	std_line_cost,
		std_ovhd_cost;

	local_rec.rtg_res_std = 0.00;
	local_rec.rtg_ovh_std = 0.00;

	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgln_rec.seq_no = 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{

		cc = find_hash(pcwc,&pcwc_rec,EQUAL, "r",rgln_rec.hhwc_hash);
		if (cc)
			strcpy (pcwc_rec.work_cntr, "NOT FND.");

		cc = find_hash(rgrs,&rgrs_rec,EQUAL, "r",rgln_rec.hhrs_hash);
		if (cc)
		{
			strcpy (rgrs_rec.code, "NOT FND.");
			strcpy (rgrs_rec.desc, "NOT FND.");
			strcpy (rgrs_rec.type, " ");
		}

		strcpy (local_rec.res_desc, "NOT FND ");
		for (i = 0; i < no_res; i++)
		{
			if (rgrs_rec.type [0] == res_type [i].type)
			{
				sprintf (local_rec.res_desc, "%-8.8s", res_type [i].desc);
				break;
			}
		}

		std_tmp_time =  rgln_rec.setup;
		std_tmp_time += rgln_rec.run;
		std_tmp_time += rgln_rec.clean;

		std_line_cost = (double) std_tmp_time * rgln_rec.rate;
		std_line_cost /= 60.00;
		std_line_cost *= (double) rgln_rec.qty_rsrc;

		std_ovhd_cost = (double) std_tmp_time * rgln_rec.ovhd_var;
		std_ovhd_cost /= 60.00;
		std_ovhd_cost += rgln_rec.ovhd_fix;
		std_ovhd_cost *= (double) rgln_rec.qty_rsrc;

		fprintf (fout, ".LRP2\n");

		fprintf (fout, "%20.20s"," ");
		fprintf (fout, "|%3d",		rgln_rec.seq_no);
		fprintf (fout, "|%-8.8s",	pcwc_rec.work_cntr);
		fprintf (fout, "|%-8.8s",	rgrs_rec.code);
		fprintf (fout, "|%-24.24s",	rgrs_rec.desc);
		fprintf (fout, "| %-8.8s ",	local_rec.res_desc);
		fprintf (fout, "|%3d",		rgln_rec.qty_rsrc);
		fprintf (fout, "|%9.2f",	DOLLARS (rgln_rec.rate));
		fprintf (fout, "|         ");
		fprintf (fout, "|%s",		ttoa (rgln_rec.setup, "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (rgln_rec.run,   "NNNN:NN"));
		fprintf (fout, "|%s",		ttoa (rgln_rec.clean, "NNNN:NN"));
		fprintf (fout, "| %10.2f ",	DOLLARS (std_line_cost));
		fprintf (fout, "|%s\n",		" ");
		local_rec.rtg_res_std += std_line_cost;

		fprintf (fout, "%20.20s"," ");
		fprintf (fout, "|   |        |        |Overhead Content        |          |   ");
		fprintf (fout, "|%9.2f",	DOLLARS (rgln_rec.ovhd_var));
		fprintf (fout, "|%9.2f",	DOLLARS (rgln_rec.ovhd_fix));
		fprintf (fout, "|       |       |       ");
		fprintf (fout, "| %10.2f ",	DOLLARS (std_ovhd_cost));
		fprintf (fout, "|%s\n",		" ");
		local_rec.rtg_ovh_std += std_ovhd_cost;

		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}
}

/*----------------------------------------
| Calculate running cost/time of routing |
----------------------------------------*/
void
calc_vals(
 void)
{
	double	cur_cost;
	long	cur_time,
		tmp_time = 0L;
	int	i;

	no_stored = 0;

	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;		
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{

		store [no_stored].seq_no		= rgln_rec.seq_no;
		store [no_stored].hhwc_hash	= rgln_rec.hhwc_hash;
		store [no_stored].rate		= rgln_rec.rate;
		store [no_stored].ovhd_var	= rgln_rec.ovhd_var;
		store [no_stored].ovhd_fix	= rgln_rec.ovhd_fix;
		store [no_stored].time		= rgln_rec.setup +
						  rgln_rec.run +
						  rgln_rec.clean;
		store [no_stored].qty_rsrc	= rgln_rec.qty_rsrc;
		cc = find_hash(rgrs,&rgrs_rec,EQUAL, "r",rgln_rec.hhrs_hash);
		if (cc)
			strcpy (store [no_stored].type, "*");
		else
			strcpy (store [no_stored].type, rgrs_rec.type);
		store [no_stored++].instr_no = rgln_rec.instr_no;
		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}

	local_rec.tot_time = 0L;
	local_rec.tot_cost = 0.00;

	for (tmp_time = 0L, i = 0; i < no_stored; i++)
	{
		cur_time = store [i].time;
		if (cur_time > tmp_time)
			tmp_time = cur_time;

		if (i + 1 == no_stored || store [i].seq_no != store [i+1].seq_no)
		{
			local_rec.tot_time += tmp_time;
			tmp_time = 0L;
		}

		cur_cost = (store [i].rate + store [i].ovhd_var) * (double) cur_time;
		cur_cost /= 60.00;
		cur_cost += store [i].ovhd_fix;
		cur_cost *= store [i].qty_rsrc;
		local_rec.tot_cost += cur_cost;
	}
	return;
}

double	
get_uom(
 long int iss_uom)
{
	char	std_group [21],
			alt_group [21];
	double	std_cnv_fct,
			alt_cnv_fct,
			wrk_cnv_fct;

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.std_uom);
	if (cc)
		file_err (cc, "inum", "DBFIND");
	strcpy (std_group, inum_rec.uom_group);
	std_cnv_fct = inum_rec.cnv_fct;

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.alt_uom);
	if (cc)
		file_err (cc, "inum", "DBFIND");
	strcpy (alt_group, inum_rec.uom_group);
	alt_cnv_fct = inum_rec.cnv_fct;

	cc = find_hash (inum, &inum_rec, EQUAL, "r", iss_uom);
	if (cc)
		file_err (cc, "inum", "DBFIND");

	/*---------------------------------------
	| If the issuing UOM group differs from	|
	| the items alternate UOM group then	|
	| assume it belongs to the items	|
	| standard UOM group.			|
	---------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		wrk_cnv_fct = inum_rec.cnv_fct / std_cnv_fct;
	else
		wrk_cnv_fct = (inum_rec.cnv_fct / alt_cnv_fct) / inmr_rec.uom_cfactor;

	return (wrk_cnv_fct);
}

void
print_summary(
 void)
{
	local_rec.bch_tot_std = local_rec.mat_tot_std +
				local_rec.rtg_res_std +
				local_rec.rtg_ovh_std;
	fprintf (fout, ".LRP9\n");

	fprintf (fout, "|-----------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
	fprintf (fout, "|%-90.90s                 |     COST PER UNIT     |    COST PER BATCH     |\n", " ");
	fprintf (fout, "|%-90.90s                 |                       |                       |\n", " ");
	fprintf (fout, "|%-90.90sMaterial         |     %10.2f        |    %10.2f         |\n",
		" ",
		DOLLARS (local_rec.mat_tot_std / local_rec.std_batch),
		DOLLARS (local_rec.mat_tot_std));
	fprintf (fout, "|%-90.90sResource         |     %10.2f        |    %10.2f         |\n",
		" ",
		DOLLARS (local_rec.rtg_res_std / local_rec.std_batch),
		DOLLARS (local_rec.rtg_res_std));
	fprintf (fout, "|%-90.90sOverhead         |     %10.2f        |    %10.2f         |\n",
		" ",
		DOLLARS (local_rec.rtg_ovh_std / local_rec.std_batch),
		DOLLARS (local_rec.rtg_ovh_std));

	fprintf (fout, "|%-90.90s                 | --------------------- | --------------------- |\n", " ");
	fprintf (fout, "|%-90.90sFinished Goods   |     %10.2f        |    %10.2f         |\n",
		" ",
		DOLLARS (local_rec.bch_tot_std / local_rec.std_batch),
		DOLLARS (local_rec.bch_tot_std));
}


