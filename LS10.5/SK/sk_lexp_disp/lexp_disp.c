/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lexp_disp.c,v 5.3 2002/07/17 09:57:54 scott Exp $
|  Program Name  : (sk_lexp_disp.c               )                    |
|  Program Desc  : (Display / Print lot expiry information.     )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 27/03/89          |
|---------------------------------------------------------------------|
| $Log: lexp_disp.c,v $
| Revision 5.3  2002/07/17 09:57:54  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:18:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:11  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:13  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/22 06:43:01  scott
| Updated to add app.schema - removes code related to tables from program and
| allows for better quality contol.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
| Revision 4.0  2001/03/09 02:37:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/21 10:23:05  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:08  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:23  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:03  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:30:52  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/12/05 23:29:08  cam
| Changes for GVision compatibility
|
| Revision 1.11  1999/11/11 05:59:45  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.10  1999/11/03 07:32:06  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.9  1999/10/13 02:42:00  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.8  1999/10/12 21:20:33  scott
| Updated by Gerry from ansi project.
|
| Revision 1.7  1999/10/08 05:32:29  scott
| First Pass checkin by Scott.
|
| Revision 1.6  1999/06/20 05:20:11  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lexp_disp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_lexp_disp/lexp_disp.c,v 5.3 2002/07/17 09:57:54 scott Exp $";

#define	X_OFF	40
#define	Y_OFF	6
#include <pslscr.h>
#include <twodec.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

extern		int	EnvScreenOK;

#define	DISP   		 (local_rec.displayPrint [0] == 'D')
#define	PRINT		 (local_rec.displayPrint [0] == 'P')

#define	SEL_PSIZE	14

	char	disp_str [300];

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;

	char	 *data = "data";


	char	systemDate [11];

	extern	int		TruePosition;

	FILE	*fout;


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	long	ExpiryDate;
	char	displayPrint [2];
	char	displayPrintDesc [11];
	int		printerNumber;
} local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "st_dt", 4, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Expiry Date      ", "Enter expiry date for lots.", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ExpiryDate}, 
	{1, LIN, "displayPrint", 5, 2, CHARTYPE, 
		"U", "        ", 
		" ", "D", "Display / Print  ", "D(isplay or P(rint ", 
		YES, NO, JUSTLEFT, "DP", "", local_rec.displayPrint}, 
	{1, LIN, "displayPrintDesc", 5, 25, CHARTYPE, 
		"AAAAAAAAAA", "        ", 
		" ", "", "", "", 
		NA, NO, JUSTLEFT, "DP", "", local_rec.displayPrintDesc}, 
	{1, LIN, "printerNumber", 5, 65, INTTYPE, 
		"NN", "        ", 
		" ", "1", "Printer Number   ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNumber}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

#include	<LocHeader.h>
/*=======================
| Function Declarations |
=======================*/
void 	SetupDefault 	(void);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadMisc 		(void);
int  	spec_valid 		(int);
void 	ProcessLots 	(void);
int  	heading 		(int);
void 	OpenPrinter 	(void);
void 	ProcesBatch 	(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	TruePosition	=	TRUE;

	EnvScreenOK = FALSE;

	strcpy (systemDate, DateToString (TodaysDate ()));

	OpenDB ();

	SETUP_SCR (vars);

	init_scr ();
	set_tty (); 
	set_masks ();

	swide ();
	clear ();

	search_ok 	= TRUE;
	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	init_ok 	= TRUE;
	init_vars (1);	
	SetupDefault ();

	heading (1);
	scn_display (1);
	edit (1);

    if (!restart)
        ProcessLots ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
SetupDefault (
 void)
{
	strcpy (local_rec.displayPrint,"D");
	strcpy (local_rec.displayPrintDesc,"Display");
	local_rec.ExpiryDate = TodaysDate ();
	local_rec.printerNumber = 1;
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

	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_id_exp");

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (inlo);

	abc_dbclose ("data");
}

/*=============================================
| Get common info from commom database file . |
=============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr , ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr , &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("displayPrint"))
	{
		if (DISP)
		{
			strcpy (local_rec.displayPrintDesc,ML ("Display"));
			FLD ("printerNumber") = NA;
		}
		else
		{
			strcpy (local_rec.displayPrintDesc,ML ("Print"));
			FLD ("printerNumber") = YES;
		}
		DSP_FLD ("displayPrintDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNumber"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNumber = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNumber))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ProcessLots (
 void)
{
	if (PRINT)
	{

		dsp_screen ("Processing : Printing Expired Lots.", 
							comm_rec.co_no, comm_rec.co_name);

		OpenPrinter ();
		ProcesBatch ();
	
		fprintf (fout,".EOF\n");

		/*========================= 
		| Program exit sequence	. |
		=========================*/
		pclose (fout);
	}
	else
	{
		heading (2);
		Dsp_open (0,2,SEL_PSIZE);

		Dsp_saverec ("   ITEM NUMBER    |    I T E M    D E S C R I P T I O N    | W/H ON HAND |  BATCH NO  | ORDER QTY.  |REMAINING QTY| EXPIRY DATE ");
		Dsp_saverec ("");
		Dsp_saverec (" [REDRAW] [PRINT] [NEXT SCN] [PREV SCN] [EDIT/END]");
		ProcesBatch ();
		Dsp_srch ();
		Dsp_close ();
	}
	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		swide ();
		clear ();

		move (0,1);
		line (132);

		switch (scn)
		{
		case	1:
			rv_pr (ML (mlSkMess198) ,44,0,1);
			box (0,3,132,2);

			move (0,21);
			line (132);

			line_cnt = 0;
			scn_write (scn);

			break;

		case	2:
			rv_pr (ML (mlSkMess198) ,44,0,1);

			if (DISP)
				sprintf (err_str, ML (mlSkMess199),
							DateToString (local_rec.ExpiryDate));
			else	
				sprintf (err_str, ML (mlSkMess200),
							DateToString (local_rec.ExpiryDate));

			rv_pr (err_str,44,0,1);

			break;

		default:
			break;
		}
		/*  reset this variable for new screen NOT page	*/
		print_at (22,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_short);
		print_at (22,35,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_short);
		print_at (22,55,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_short);
	}
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
OpenPrinter (
 void)
{
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((fout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	/*=======================
	| Start output to file. |
	=======================*/
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	fprintf (fout, ".14\n");
	fprintf (fout, ".L158\n");

	fprintf (fout, ".B1\n");
	fprintf (fout, ".ESTOCK EXPIRED LOT REPORT.\n");
	fprintf (fout, ".E%s \n",clip (comm_rec.co_name));
	fprintf (fout, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %s\n",SystemTime ());

	fprintf (fout, ".R===================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============================\n");

	fprintf (fout, "===================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============================\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|    I T E M    D E S C R I P T I O N    ");
	fprintf (fout, "| W/H ON HAND |  BATCH NO  | ORDER QTY.  ");
	fprintf (fout, "|REMAINING QTY| EXPIRY DATE|\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|-------------|------------|-------------");
	fprintf (fout, "|-------------|------------|\n");

	fprintf (fout, ".PI12\n");
}

/*=======================
| Process Batch Reports |
=======================*/
void
ProcesBatch (
 void)
{
	int		batch_printed 	= FALSE;
	int		i;
	char	DateString [13];

	incc_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort, "%28.28s", " ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhcc_hash	== ccmr_rec.hhcc_hash)
	{
		inmr_rec.hhbr_hash = incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}
		batch_printed 	= FALSE;
	
		for (i = 0; i < (int)strlen (ValidLocations); i++)
		{
			inlo_rec.hhwh_hash		=	incc_rec.hhwh_hash;
			inlo_rec.loc_type [0]	=	ValidLocations [i];
			inlo_rec.expiry_date	=	1L;

			cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
			while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash &&
					  	  inlo_rec.loc_type [0] == ValidLocations [i] &&
					      inlo_rec.expiry_date <= local_rec.ExpiryDate)
			{
				sprintf (DateString," %s ",DateToString (inlo_rec.expiry_date));

				if (strcmp (inmr_rec.supercession,"                "))
				{
					if (PRINT)
					{
						fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
	       				fprintf (fout,"| SAME AS %16.16s%15.15s",
										inmr_rec.supercession," ");
						fprintf (fout, "|             |            |             ");
						fprintf (fout, "|             |            |\n");
					}
					else
					{
						sprintf (disp_str, " %16.16s  SAME AS %16.16s                ^E             ^E            ^E             ^E             ^E            ",
								inmr_rec.item_no, inmr_rec.supercession);
						Dsp_saverec (disp_str);
					}
					cc = find_rec (inlo, &inlo_rec, NEXT, "r");
					continue;
				}
				if (!batch_printed)
				{
					if (PRINT)
					{
						fprintf (fout,"| %-16.16s ",inmr_rec.item_no);
						fprintf (fout,"|%-40.40s",inmr_rec.description); 
						fprintf (fout,"|%12.2f ",incc_rec.closing_stock);
					}
					else
					{
	
						sprintf (disp_str, " %16.16s ^E%40.40s^E%12.2f ^E  %7.7s   ^E%12.2f ^E%12.2f ^E %s ",
									inmr_rec.item_no,
									inmr_rec.description,
									incc_rec.closing_stock,
									inlo_rec.lot_no,
									inlo_rec.rec_qty,
									inlo_rec.qty,
									DateString);
						Dsp_saverec (disp_str);
					}
				}
				else
				{
					if (PRINT)
					{
						fprintf (fout,"| %-16.16s "," ");
						fprintf (fout,"|%-40.40s"," "); 
						fprintf (fout,"|%-13.13s"," "); 
					}
					else
					{
						sprintf (disp_str, " %16.16s ^E%40.40s^E             ^E  %7.7s   ^E%12.2f ^E%12.2f ^E %s ",
								" ",
								" ",
								inlo_rec.lot_no,
								inlo_rec.rec_qty,
								inlo_rec.qty,
								DateString);
						Dsp_saverec (disp_str);
					}
				}
				batch_printed = TRUE;

				if (PRINT)
				{
					fprintf (fout,"|  %7.7s   ", inlo_rec.lot_no);
					fprintf (fout,"|%12.2f ",	inlo_rec.rec_qty);
					fprintf (fout,"|%12.2f ",	inlo_rec.qty);
					fprintf (fout,"| %s |\n",DateToString (inlo_rec.expiry_date));
				}
				cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			}
		}

		if (PRINT)
			dsp_process ("Item", inmr_rec.item_no);

		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
}
