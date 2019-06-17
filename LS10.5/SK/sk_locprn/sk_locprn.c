/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_locprn.c,v 5.5 2002/07/17 09:57:55 scott Exp $
|  Program Name  : (sk_locprn & sk_loc_del & sk_locmprn & sk_loc_dsp  |
|  Program Desc  : (Print Stock Location Report.            )         |
|                  (Stock Location delete .                 )         |
|                  (Stock location print by Location master.)         |
|                  (Stock location display by stock loc Master.)      |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/01/88         |
|---------------------------------------------------------------------|
| $Log: sk_locprn.c,v $
| Revision 5.5  2002/07/17 09:57:55  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/09/25 09:59:39  robert
| Updated to add validation on From and To Location
|
| Revision 5.3  2001/08/14 02:54:11  scott
| Updated for new delete wizard
|
| Revision 5.2  2001/08/09 09:18:56  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:14  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.2  2001/04/05 00:48:29  scott
| Updated to use LTEQ and PREVIOUS calls to work with SQL and CISAM
|
| Revision 4.1  2001/03/22 06:31:43  scott
| Updated as keyed value for start location was using a GTEQ instead of EQUAL
|
| Revision 4.0  2001/03/09 02:37:36  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:10  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:05  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.20  2000/01/06 08:47:50  marnie
| C2289- Modified to be able to select printer when printing Print Stock by Item Group/Location.
|
| Revision 1.19  1999/12/06 01:30:53  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.18  1999/11/11 05:59:46  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.17  1999/11/07 22:53:19  cam
| Added #include <signal.h> for GVision compile.
|
| Revision 1.16  1999/11/03 07:32:06  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.15  1999/10/13 02:42:01  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.14  1999/10/08 05:32:30  scott
| First Pass checkin by Scott.
|
| Revision 1.13  1999/06/20 05:20:11  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: sk_locprn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_locprn/sk_locprn.c,v 5.5 2002/07/17 09:57:55 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<signal.h>
#include 	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<DeleteControl.h>

#define	DSP_LOC		 (by_loc_dsp == TRUE)

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;
struct inloRecord	inlo_rec;
struct lomrRecord	lomr_rec;

	int	printerNumber ;

	int	by_cat_loc = FALSE,
		by_loc_mst = FALSE,
		by_loc_dsp = FALSE,
		by_loc_del = FALSE;
		
	int	first_loc = TRUE;

	FILE	*fout;

	char	lower [13], 
			upper [13]; 

	char	disp_str [300];

	long	envSkLocDaysDel = 0L;


#include	<LocHeader.h>

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	locn_from [11];
	char	locn_to [11];
	char	f_desc [41];
	char	t_desc [41];
	int		printerNumber;
	char	lp_str [3];
	char 	back [6];
	char	onite [6];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "locn_from",	 4, 22, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "From Location     :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.locn_from},
	{1, LIN, "f_desc",	 5, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description       :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.f_desc},
	{1, LIN, "locn_to",	 7, 22, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "To Location       :", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.locn_to},
	{1, LIN, "t_desc",	 8, 22, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description       :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.t_desc},
	{1, LIN, "printerNumber",		10, 22, INTTYPE,
		"NN", "          ",
		" ", "1",        "Printer Number    :", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber},
	{1, LIN, "back",		11, 22, CHARTYPE,
		"U", "          ",
		" ", "N (o",      "Background        :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	12, 22, CHARTYPE,
		"U", "          ",
		" ", "N (o",      "Overnight         :", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
void loc_output (void);
void locm_output (void);
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void ReadMisc (void);
int  spec_valid (int field);
void loc_proc (void);
void pr_loc (void);
void locm_proc (void);
void pr_locm (void);
void print_categ (int first_time);
void run_prog (char *prog_name);
void delete_locs (void);
int  heading (int scn);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	SETUP_SCR (vars);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "sk_locprn"))
		by_cat_loc = TRUE;

	if (!strcmp (sptr, "sk_locmprn"))
		by_loc_mst = TRUE;

	if (!strcmp (sptr, "sk_loc_dsp"))
		by_loc_dsp = TRUE;

	if (!strcmp (sptr, "sk_loc_del"))
		by_loc_del = TRUE;

	sptr = chk_env ("SK_LOC_DAYS_DEL");
	envSkLocDaysDel = (sptr == (char *)0) ? 120L : atol (sptr);

	OpenDB ();

	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "INVENTORY-LOCATIONS");
	if (!cc)
	{
		envSkLocDaysDel = (long) delhRec.purge_days;
	}
	/*-----------------------------------------------
	| No point in running reports if not multi bin. |
	-----------------------------------------------*/
    if (!MULT_LOC && !by_cat_loc) {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }

	/*-------------------------------------
	| Delete all locations with no stock. |
	-------------------------------------*/
	if (by_loc_del)
	{
		dsp_screen ("Processing : Delete unused locations.", 
					comm_rec.co_no, comm_rec.co_name);
		delete_locs ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Print locations by class/category/item. |
	-----------------------------------------*/
	if (by_cat_loc)
	{
		if (argc < 4)
		{
			print_at (0,0, mlSkMess111, argv [0]);
			return (EXIT_FAILURE);
		}

		printerNumber = atoi (argv [1]);
		sprintf (lower,"%-12.12s",argv [2]);
		sprintf (upper,"%-12.12s",argv [3]);

		dsp_screen ("Processing : Stock Location Report.", 
					comm_rec.co_no, comm_rec.co_name);

		local_rec.printerNumber = printerNumber;
		loc_output ();
	
		loc_proc ();
	
		fprintf (fout,".EOF\n");

		/*========================= 
		| Program exit sequence	. |
		=========================*/
		pclose (fout);

		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*---------------------------------------------
	| Print/display locations by location master. |
	---------------------------------------------*/
	if ( (by_loc_mst || by_loc_dsp) && argc == 4)
	{
		if (!DSP_LOC)
		{
			/*--------------------------------------------------
			| If child process is backgd, then set to raw mode |
			--------------------------------------------------*/
			if (for_chk () != 0)
				signal (SIGINT,SIG_IGN);
		}

		local_rec.printerNumber = atoi (argv [1]);
		sprintf (local_rec.locn_from, "%-10.10s", argv [2]);
		sprintf (local_rec.locn_to,   "%-10.10s", argv [3]);

		/*-------------------
		| Display locations |
		-------------------*/
		if (DSP_LOC)
		{
			init_scr ();
			swide ();
			set_tty ();

			print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);
	
			print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

			print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

			move (0,23);
			line (130);
		}
		else
		{
			dsp_screen ("Stock master location printout.",
						comm_rec.co_no,
						comm_rec.co_name);
		}

		locm_output ();
	
		locm_proc ();

		if (DSP_LOC)
		{
			Dsp_srch ();
			Dsp_close ();
		}
		else
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	/*========================= 
	| Program exit sequence	. |
	=========================*/
	init_scr ();
	swide ();
	set_tty ();
	set_masks ();
	init_vars (1);

	if (DSP_LOC)
	{
		FLD ("printerNumber")   = ND ;
		FLD ("back")   = ND ;
		FLD ("onight") = ND ;
	}

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

	
		run_prog (argv [0]);

		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
loc_output (
 void)
{
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ( (fout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	/*=======================
	| Start output to file. |
	=======================*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	fprintf (fout, ".14\n");
	fprintf (fout, ".L130\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESTOCK LOCATION REPORT.\n");
	fprintf (fout, ".E%s \n",clip (comm_rec.co_name));
	fprintf (fout, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %s\n",SystemTime ());

	fprintf (fout, ".R===================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================================================\n");

	fprintf (fout, "===================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================================================\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|    I T E M    D E S C R I P T I O N    ");
	fprintf (fout, "| W/H ON HAND |  LOCATION  | UOM. |   ON HAND.  |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|-------------|------------|------|-------------|\n");

	fprintf (fout, ".PI12\n");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
locm_output (
 void)
{
	char	head_text [300];

	if (DSP_LOC)
	{
		sprintf (head_text, " Location Master Display from Location (%s) TO (%s) ",
				local_rec.locn_from, local_rec.locn_to);

		Dsp_prn_open (0, 0, 14, head_text, 
					comm_rec.co_no, comm_rec.co_name,
					comm_rec.est_no, comm_rec.est_name,
					comm_rec.cc_no, comm_rec.cc_name);

	   	sprintf (err_str,  ".             L O C A T I O N   M A S T E R   D I S P L A Y   F R O M   L O C A T I O N  : (%s)  TO  (%s)        ", local_rec.locn_from , local_rec.locn_to);
		Dsp_saverec (err_str);
		Dsp_saverec (" LOCATION |          LOCATION DESCRIPTION          | ITEM  NUMBER   |           ITEM   DESCRIPTION           | UOM. |  QUANTITY ");
		Dsp_saverec (" [REDRAW]  [NEXT SCREEN]  [PREV SCREEN]  [INPUT / END]");
		return;
	}
	
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ( (fout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");
	
	/*=======================
	| Start output to file. |
	=======================*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	fprintf (fout, ".14\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESTOCK LOCATION MASTER REPORT.\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s \n",clip (comm_rec.co_name));
	fprintf (fout, ".EBRANCH: %s \n",clip (comm_rec.est_name));
	fprintf (fout, ".EWAREHOUSE: %s \n",clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %s\n",SystemTime ());

	fprintf (fout, ".R=============");
	fprintf (fout, "===========================================");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	fprintf (fout, "==============\n");

	fprintf (fout, "=============");
	fprintf (fout, "===========================================");
	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	fprintf (fout, "==============\n");

	fprintf (fout, "|  LOCATION  ");
	fprintf (fout, "| L O C A T I O N    D E S C R I P T I O N ");
	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|    I T E M      D E S C R I P T I O N    ");
	fprintf (fout, "| UOM. ");
	fprintf (fout, "|  QUANTITY. |\n");

	fprintf (fout, "|------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------");
	fprintf (fout, "|------------|\n");

	fprintf (fout, ".PI12\n");
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
	abc_dbopen ("data");

	ReadMisc ();

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (lomr, lomr_list, LOMR_NO_FIELDS, "lomr_id_no");
	abc_selfield (inlo, "inlo_mst_id");
	OpenLocation (ccmr_rec.hhcc_hash);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inlo);
	abc_fclose (lomr);
	CloseLocation ();

	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("ccmr", ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec ("ccmr",&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose ("ccmr");
}

/*=============================
| Special validation section. |
=============================*/
int
spec_valid (
 int field)
{
	/*--------------------------
	| Validate Location from . |
	--------------------------*/
	if (LCHECK ("locn_from"))
	{
		if (SRCH_KEY)
		{
			SearchLomr (ccmr_rec.hhcc_hash, temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			lomr_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			strcpy (lomr_rec.location, "          ");
			cc = find_rec (lomr, &lomr_rec, GTEQ, "r");
			if (!cc && lomr_rec.hhcc_hash == ccmr_rec.hhcc_hash)
				strcpy (local_rec.locn_from, lomr_rec.location);
		}
		
		if (prog_status == EDIT && strcmp (local_rec.locn_from, local_rec.locn_to) > 0)
		{
			errmess (ML ("From Location cannot be greater than To Location"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		lomr_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		strcpy (lomr_rec.location, local_rec.locn_from);
		cc = find_rec (lomr, &lomr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess209));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.f_desc, lomr_rec.desc);
		DSP_FLD ("f_desc");
		return (EXIT_SUCCESS);
	}
	/*------------------------
	| Validate Location to . |
	------------------------*/
	if (LCHECK ("locn_to"))
	{
		if (SRCH_KEY)
		{
			SearchLomr (ccmr_rec.hhcc_hash, temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			lomr_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
			memset ((char *)lomr_rec.location,0xff,sizeof (lomr_rec.location));
			cc = find_rec (lomr, &lomr_rec, LTEQ, "r");
			if (!cc && lomr_rec.hhcc_hash == ccmr_rec.hhcc_hash)
				strcpy (local_rec.locn_to, lomr_rec.location);
		}

		if (strcmp (local_rec.locn_to,local_rec.locn_from) < 0)
		{
			errmess (ML ("To Location cannot be less than From Location"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		lomr_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
		strcpy (lomr_rec.location, local_rec.locn_to);
		cc = find_rec (lomr, &lomr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess209));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.t_desc, lomr_rec.desc);
		DSP_FLD ("t_desc");
		return (EXIT_SUCCESS);
	}

	/*==========================
	| Validate printer number. |
	==========================*/
	if (LCHECK ("printerNumber"))
	{
		if (F_HIDE (label ("printerNumber")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
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
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') 
							? "Y (es" : "N (o ");
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite [0] == 'Y') 
							? "Y (es" : "N (o ");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*=============================================================
| Process locations using incc_sort. i.e Class/Category/Item. |
=============================================================*/
void
loc_proc (
 void)
{
	char	old_group [13];
	char	new_group [13];
	int	first_time = TRUE;

	/*-------------------
	|	read first incc	|
	-------------------*/
	fflush (fout);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-28.28s",lower);
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	sprintf (old_group,"%12.12s",lower);

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash &&
			 strncmp (incc_rec.sort,upper,12) <= 0)
	{
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);

		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");
		if (cc)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		sprintf (new_group,"%1.1s%-11.11s",
					inmr_rec.inmr_class,inmr_rec.category);

		dsp_process (" Item: ", inmr_rec.item_no);

		if (strcmp (new_group,old_group) || first_time)
		{
			strcpy (old_group,new_group);
			print_categ (first_time);
			first_time = FALSE;
		}
		pr_loc ();
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
}

/*===================
| Process locations |
===================*/
void
pr_loc (
 void)
{
	int	loc_printed = FALSE;
	float	loc_total = 0.00;

	fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
	if (strcmp (inmr_rec.supercession,"                "))
	{
		fprintf (fout,"| SAME AS %16.16s%15.15s", inmr_rec.supercession," ");
		fprintf (fout,"|%13.13s", " ");
		fprintf (fout,"|%12.12s", " ");
		fprintf (fout,"| %4.4s ", " ");
		fprintf (fout,"|%13.13s|\n"," ");
		return;
	}
	if (!MULT_LOC)
	{
		fprintf (fout,"|%-40.40s",	inmr_rec.description); 
		fprintf (fout,"|%12.2f ",	incc_rec.closing_stock);
		fprintf (fout,"| %10.10s ",	incc_rec.location);
		fprintf (fout,"| %4.4s ",	inmr_rec.sale_unit);
		fprintf (fout,"|%12.2f |\n",	incc_rec.closing_stock);
		return;
	}

	inlo_rec.hhwh_hash 		= incc_rec.hhwh_hash;
	inlo_rec.hhum_hash 		= 0L;
	inlo_rec.loc_type [0]	=	' ';
	strcpy (inlo_rec.location,"          ");
	cc = find_rec (inlo, &inlo_rec, GTEQ,"r");
	while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		if (!loc_printed)
		{
			fprintf (fout,"|%-40.40s",inmr_rec.description); 
			fprintf (fout,"|%12.2f ",incc_rec.closing_stock);
		}
		else
		{
			fprintf (fout,"| %-16.16s "," ");
			fprintf (fout,"|%-40.40s"," "); 
			fprintf (fout,"|%-13.13s"," "); 
		}
		loc_printed = TRUE;

		if (inlo_rec.cnv_fct == 0.00)
			inlo_rec.cnv_fct = 1.00;

		fprintf (fout,"| %10.10s ",inlo_rec.location);
		fprintf (fout,"| %4.4s ",inlo_rec.uom);
		fprintf (fout,"|%12.2f |\n",inlo_rec.qty / inlo_rec.cnv_fct);
		loc_total += inlo_rec.qty;

		cc = find_rec (inlo, &inlo_rec, NEXT,"r");
	}
	if (!loc_printed)
	{
		fprintf (fout,"|%-40.40s",	inmr_rec.description); 
		fprintf (fout,"|%12.2f ",	incc_rec.closing_stock);
		fprintf (fout,"| %10.10s ",	incc_rec.location);
		fprintf (fout,"| %4.4s ",	inmr_rec.sale_unit);
		fprintf (fout,"|%12.2f |\n",	incc_rec.closing_stock);
	}
	else
	{
		if (loc_total != incc_rec.closing_stock)
		{
			fprintf (fout,"| %-16.16s "," ");
			fprintf (fout,"|%-40.40s"," "); 
			fprintf (fout,"|%-13.13s"," "); 
			fprintf (fout,"|** ERROR. **");
			fprintf (fout,"|      ");
			fprintf (fout,"|%12.2f |\n", incc_rec.closing_stock - 
						   loc_total);
		}
	}
	return;
}

/*==========================================
| Process locations using location record. |
==========================================*/
void
locm_proc (
 void)
{
	/*-------------------------------
	| Read first location record.	|
	-------------------------------*/
	fflush (fout);

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (incc, "incc_hhwh_hash");
	abc_selfield (inlo, "inlo_location");

	first_loc = TRUE;

	lomr_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (lomr_rec.location,"%-10.10s", local_rec.locn_from);
	cc = find_rec (lomr, &lomr_rec, GTEQ, "r");

	while (!cc && lomr_rec.hhcc_hash == ccmr_rec.hhcc_hash &&
			strcmp (lomr_rec.location, local_rec.locn_to) <= 0)
	{
		if (!DSP_LOC)
			dsp_process (" Location: ", lomr_rec.location);

		pr_locm ();

		cc = find_rec (lomr, &lomr_rec, NEXT, "r");
	}
	if (DSP_LOC)
	{
		Dsp_saverec ("^^GGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGJGGGGGGGGGGG");
		Dsp_saverec ("                                               ^^GGGGGGGG^^ End of Report ^^GGGGGGGG^^");
	}
}

/*===================
| Process locations |
===================*/
void
pr_locm (
 void)
{
	char	old_locn [11];
	char	new_locn [11];

	strcpy (inlo_rec.location, lomr_rec.location);
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");

	while (!cc && !strcmp (inlo_rec.location, lomr_rec.location))
	{
		incc_rec.hhwh_hash	=	inlo_rec.hhwh_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");

		/*--------------------------------------------------------
		| An error finding incc or incc record is not in correct |
        | warehouse.                                             |
		--------------------------------------------------------*/
		if (cc || incc_rec.hhcc_hash != lomr_rec.hhcc_hash)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}
		
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}
		if (first_loc)
			sprintf (old_locn,"%-10.10s", lomr_rec.location);

		first_loc = FALSE;

		sprintf (new_locn,"%-10.10s", lomr_rec.location);

		if (strcmp (new_locn, old_locn))
		{
			if (DSP_LOC)
				Dsp_saverec ("^^GGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGHGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGHGGGGGGHGGGGGGGGGGG");
			else
			{
		    		fprintf (fout,"|------------");
		    		fprintf (fout,"|------------------------------");
		    		fprintf (fout,"------------|------------------");
		    		fprintf (fout,"|------------------------------");
		    		fprintf (fout,"------------|------|------------|\n");
			}
			strcpy (old_locn, new_locn);
		}
		if (DSP_LOC)
		{
			if (inlo_rec.cnv_fct == 0.00)
				inlo_rec.cnv_fct = 1.00;

			sprintf (disp_str, "%10.10s^E%40.40s^E%16.16s^E%40.40s^E %4.4s ^E%10.2f ",
				     lomr_rec.location ,
				     lomr_rec.desc ,
				     inmr_rec.item_no,
				     inmr_rec.description,
				     inlo_rec.uom,
				     inlo_rec.qty / inlo_rec.cnv_fct);

			Dsp_saverec (disp_str);

			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}
		fprintf (fout, "| %10.10s ", lomr_rec.location);
		fprintf (fout, "| %40.40s ", lomr_rec.desc);

		fprintf (fout, "| %16.16s ", inmr_rec.item_no);
		fprintf (fout, "| %40.40s ", inmr_rec.description);
		fprintf (fout, "| %4.4s ", inlo_rec.uom);
		fprintf (fout, "| %10.2f |\n", inlo_rec.qty / inlo_rec.cnv_fct);
		cc = find_rec (inlo, &inlo_rec, NEXT, "r");
	}
	return;
}

void		
print_categ (
 int first_time)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (fout, ".PD|%-107.107s|\n",err_str);
	if (!first_time)
		fprintf (fout, ".PA\n");
}

void
run_prog (
 char *prog_name)
{
	sprintf (local_rec.lp_str,"%2d",local_rec.printerNumber);
	
	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
		{
			execlp ("ONIGHT",
				"ONIGHT",
				prog_name,
				local_rec.lp_str,
				local_rec.locn_from,
				local_rec.locn_to,
				"Location master file printout", (char *)0);
		}
		/*else
			exit (0);*/
	}
	else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.locn_from,
				local_rec.locn_to,
				 (char *)0);
		/*else
			exit (0);*/
	}
	else 
	{
		execlp (prog_name,
			prog_name,
			local_rec.lp_str,
			local_rec.locn_from,
			local_rec.locn_to,
			 (char *)0);
	}
}

/*==========================
| Delete unused locations. |
==========================*/
void
delete_locs (
 void)
{
	abc_selfield (inlo, "inlo_mst_id");
	
	inlo_rec.hhwh_hash = 0L;
	inlo_rec.hhum_hash = 0L;
	strcpy (inlo_rec.location, "          ");
	strcpy (inlo_rec.lot_no, "       ");
	cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
	while (!cc)
	{
		dsp_process (" Location: ", inlo_rec.location);
		
		if (inlo_rec.qty == 0.00 && 
				inlo_rec.date_create + envSkLocDaysDel < TodaysDate ())
		{
			abc_delete (inlo);
			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		}	
		else
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
	}
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (mlSkMess237),40,0,1);
		move (0,1);
		line (130);

		box (0, 3, 131, (DSP_LOC) ? 5 : 9);

		move (1,6);
		line (130);

		if (!DSP_LOC)
		{
			move (1,9);
			line (130);
		}

		move (0,19);
		line (130);

		print_at (20,0, ML (mlStdMess038), 
					comm_rec.co_no, comm_rec.co_name);

		print_at (21,0, ML (mlStdMess039), 
					comm_rec.est_no, comm_rec.est_name);

		print_at (22,0, ML (mlStdMess099), 
					comm_rec.cc_no, comm_rec.cc_name);

		move (0,23);
		line (130);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
