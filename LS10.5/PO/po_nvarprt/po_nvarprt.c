/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_nvarprt.c,v 5.2 2001/08/09 09:15:55 scott Exp $
|  Program Name  : ( po_nvarprt.c   )                                 |
|  Program Desc  : ( Purchase Order NEW variance report.          )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohs,     ,     ,     ,     ,     ,     ,   |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  pohs,     ,     ,     ,     ,     ,               |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (19/03/93)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (31/03/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (12/09/97)      | Modified  by : Ana Marie Tario.  |
|                                                                     |
|  Comments      :                                                    |
|  (19/03/93)    : EGC 8303                                           |
|  (31/03/94)    : HGP 10469. Removal of $ signs.                     |
|  (12/09/97)    : Incorporated multilingual conversion and DMY4 date.|
|                :                                                    |
|                                                                     |
| $Log: po_nvarprt.c,v $
| Revision 5.2  2001/08/09 09:15:55  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:04  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:00  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:33  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:05:24  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.11  1999/11/11 06:43:16  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.10  1999/11/05 05:17:13  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.9  1999/09/29 10:12:04  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:38:06  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:31  scott
| Updated to remove old read_comm(), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_nvarprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_nvarprt/po_nvarprt.c,v 5.2 2001/08/09 09:15:55 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_po_mess.h>
#include	<dsp_screen.h>

#include	"schema"

struct commRecord	comm_rec;
struct pohsRecord	pohs_rec;

	char	*data	= "data";

	FILE	*pout;
	char	last_po_no [16];
	char	systemDate [11];
	double	est_tot = 0.00,
			act_tot = 0.00,
			qvr_tot = 0.00,
			pvr_tot = 0.00,
			max_pct_var = 0.00;

	int		printerNumber,
			first_time = TRUE;

	long	fst_rec_dte,
			lst_rec_dte,
			fst_cst_dte,
			lst_cst_dte;

static	struct	var	vars[] =
{
	{1, LIN, "fst_rec_dte",	 3, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Receipt Date", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &fst_rec_dte},
	{1, LIN, "lst_rec_dte",	 4, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "End Receipt Date  ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &lst_rec_dte},
	{1, LIN, "fst_cst_dte",	 5, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Costing Date", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &fst_cst_dte},
	{1, LIN, "lst_cst_dte",	 6, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", systemDate, "End Costing Date  ", " ",
		 NO, NO,  JUSTLEFT, "", "", (char *) &lst_cst_dte},
	{1, LIN, "percent",	 7, 20, DOUBLETYPE,
		"NNNNN.NN", "          ",
		" ", "0.00", "Minimum variance %", " ",
		 NO, NO,  JUSTLEFT, "0.00", "99999.99", (char *) &max_pct_var},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"", "          ",
		"", "", "", " ",
		YES, NO, JUSTRIGHT, "", "", err_str},
};


/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);
void 	Process 		(void);
void 	PrintData 		(double);
void 	InitOutput 		(void);
void 	PrintTotals 	(void);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char *argv[])
{

    if (argc < 2)
	{
		print_at (0,0, mlStdMess036, argv[0]);
        return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv[1]);

	strcpy (systemDate , DateToString (TodaysDate ()));

	SETUP_SCR (vars);

	/*----------------------------
	| setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	while (!prog_exit)
	{
		init_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Process ();
        break;
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 void)
{
	if (!first_time)
		pclose (pout);

	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open Database Files . |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm( comm_list, COMM_NO_FIELDS, (char *) &comm_rec );
	open_rec (pohs, pohs_list, POHS_NO_FIELDS, "pohs_id_no2");
}

/*=======================
| Close Database Files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (pohs);
	abc_dbclose (data);
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
		centre_at (0, 80,ML(mlPoMess149));
		move (0, 1);
		line (81);
		box (0, 2, 80, 5);

		line_cnt = 0;
		scn_write (scn);
	}
	move (0, 21);
	line (81);
	strcpy(err_str,ML(mlStdMess038));
	print_at (22, 0, err_str, comm_rec.co_no, comm_rec.co_name);

    return(0);
}

int
spec_valid (
 int field)
{
	return (EXIT_SUCCESS);
}

void
Process (
 void)
{
	double	pct_var;

	dsp_screen ("Processing:", comm_rec.co_no, comm_rec.co_name);
	strcpy (pohs_rec.co_no, comm_rec.co_no);
	strcpy (pohs_rec.br_no, comm_rec.est_no);
	strcpy (pohs_rec.stat_flag, "C");
	cc = find_rec (pohs, &pohs_rec, GTEQ, "u");
	while
	(
		!cc &&
		!strcmp (pohs_rec.co_no, comm_rec.co_no) &&
		!strcmp (pohs_rec.br_no, comm_rec.est_no) &&
		!strcmp (pohs_rec.stat_flag, "C")
	)
	{
		pct_var = fabs (((pohs_rec.act_cost - pohs_rec.est_cost) / pohs_rec.act_cost) * 100.00);
		if
		(
			pohs_rec.printed[0] == 'Y' ||
			pohs_rec.date_receipt < fst_rec_dte ||
			pohs_rec.date_receipt > lst_rec_dte ||
			pohs_rec.date_cost < fst_cst_dte ||
			pohs_rec.date_cost > lst_cst_dte ||
			pct_var < max_pct_var
		)
		{
			abc_unlock (pohs);
			cc = find_rec (pohs, &pohs_rec, NEXT, "u");
			continue;
		}

		PrintData (pct_var);

		strcpy (pohs_rec.printed, "Y");
		cc = abc_update (pohs, &pohs_rec);
		if (cc)
			file_err (cc, "pohs", "DBUPDATE");

		cc = find_rec (pohs, &pohs_rec, NEXT, "u");
	}
	abc_unlock (pohs);
}

void
PrintData (
 double pct_var)
{
	if (first_time)
		InitOutput ();
	first_time = FALSE;

	fprintf (pout, "|%-15.15s", strcmp (last_po_no, pohs_rec.pur_ord_no) ? pohs_rec.pur_ord_no : " ");
	fprintf (pout, "|%-15.15s", pohs_rec.gr_no);
	fprintf (pout, "|%-10.10s", DateToString (pohs_rec.date_receipt));
	fprintf (pout, "|%-10.10s", DateToString (pohs_rec.date_cost));
	fprintf (pout, "|%14.2f ", pohs_rec.est_cost);
	fprintf (pout, "|%14.2f ", pohs_rec.act_cost);
	fprintf (pout, "|%14.2f ", pohs_rec.prc_var);
	fprintf (pout, "|%14.2f ", pohs_rec.qty_var);
	fprintf (pout, "|%14.2f ", (pohs_rec.prc_var + pohs_rec.qty_var));
	fprintf (pout, "|%8.2f|\n", pct_var);

	strcpy (last_po_no, pohs_rec.pur_ord_no);
	est_tot += pohs_rec.est_cost;
	act_tot += pohs_rec.act_cost;
	qvr_tot += pohs_rec.qty_var;
	pvr_tot += pohs_rec.prc_var;
}

void
InitOutput (
 void)
{
	char	dates[4][11];

	strcpy (dates[0], DateToString (fst_rec_dte));
	strcpy (dates[1], DateToString (lst_rec_dte));
	strcpy (dates[2], DateToString (fst_cst_dte));
	strcpy (dates[3], DateToString (lst_cst_dte));

	pout = popen ("pformat", "w");
	if (pout == (FILE *) 0)
		file_err (1, "pformat", "POPEN");

	fprintf (pout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (pout, ".LP%d\n", printerNumber);
	fprintf (pout, ".12\n");
	fprintf (pout, ".L132\n");
	fprintf (pout, ".PI10\n");
	fprintf (pout, ".EPURCHASE RECEIPT/COSTING VARIANCE REPORT\n");
	fprintf (pout, ".CCompany : %s %s\n", comm_rec.co_name, comm_rec.co_name);
	fprintf (pout, ".CBranch  : %s %s\n", comm_rec.est_name, comm_rec.est_name);
	fprintf (pout, ".B1\n");
	fprintf
	(
		pout,
		"         RECEIPTED BETWEEN %s AND %s : COSTED BETWEEN %s AND %s : WHERE VARIANCE EXCEEDS %8.2f%%\n",
		dates[0],
		dates[1],
		dates[2],
		dates[3],
		max_pct_var
	);
	fprintf (pout, "================================================================================================================================================\n");
	fprintf (pout, "| PURCHASE ORDER| GOODS RECEIPT |   DATE   |   DATE   |     ESTIMATED |     ACTUAL    |    PRICE      |   QUANTITY    |     TOTAL     |VARIANCE|\n");
	fprintf (pout, "|    NUMBER     |     NUMBER    | RECEIPTED|  COSTED  |     AMOUNT    |     AMOUNT    |VARIANCE VALUE |VARIANCE VALUE |VARIANCE VALUE |  %%AGE  |\n");
	fprintf (pout, "|---------------|---------------|----------|----------|---------------|---------------|---------------|---------------|---------------|--------|\n");
	fprintf (pout, ".R================================================================================================================================================\n");
}

void
PrintTotals (
 void)
{
	double	pct_var;

	pct_var = fabs (((act_tot - est_tot) / act_tot) * 100.00);

	fprintf (pout, ".LRP3\n");

	fprintf (pout, "|---------------|---------------|----------|----------|---------------|---------------|---------------|---------------|---------------|--------|\n");

	fprintf (pout, "|%-15.15s", " ");
	fprintf (pout, "|%-15.15s", " ");
	fprintf (pout, "| %-8.8s ", " ");
	fprintf (pout, "| %-8.8s ", "Total");
	fprintf (pout, "|%14.2f ", est_tot);
	fprintf (pout, "|%14.2f ", act_tot);
	fprintf (pout, "|%14.2f ", pvr_tot);
	fprintf (pout, "|%14.2f ", qvr_tot);
	fprintf (pout, "|%14.2f ", (qvr_tot + pvr_tot));
	fprintf (pout, "|%8.2f|\n", pct_var);
}
