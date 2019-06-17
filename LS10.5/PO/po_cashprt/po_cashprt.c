/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_cashprt.c,v 5.3 2001/10/17 08:58:52 cha Exp $
|  Program Name  : (po_cashprt.c   )                                  |
|  Program Desc  : (Print Purchase Order Commitments Report.    )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, pohr, poln, pocr,     ,     ,     ,         |
|                :      ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
|  Date Modified : (10/08/87)      | Modified  by  : Vicki Seal.      |
|  Date Modified : (29/08/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (14/11/88)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (17/04/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (28/11/91)      | Modified  by  : Campbell Mander. |
|  Date Modified : (27/05/96)      | Modified  by  : Jiggs Veloz.     |
|  Date Modified : (13/09/97)      | Modified  by  : Leah Manibog.    |
|                                                                     |
|  Comments      : Print PNAME at top right of report.                |
|                : Change Printing of                                 |
|                : poln_duty,poln_licence,poln_fob_fgn_cst,           |
|                : poln_frt_ins_cst from DOLLARS.                     |
|                :                                                    |
| (28/11/91)    : Put a .PI12 in heading.                             |
|                :                                                    |
| (27/05/96)    : Updated to fix problems in DateToString.            |
| (13/09/97)    : Updated for Multilingual Conversion.				  |
|                :                                                    |
|                                                                     |
| $Log: po_cashprt.c,v $
| Revision 5.3  2001/10/17 08:58:52  cha
| Updated as getchar left in program.
| Changes made by Scott.
|
| Revision 5.2  2001/08/09 09:15:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:43  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:07  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/16 06:52:14  scott
| Updated to extend max currencies allowed from 100 to 500
|
| Revision 4.0  2001/03/09 02:32:25  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:14  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:28  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:04:58  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/07/13 22:22:33  scott
| Updated to add app.schema and clean code.
|
| Revision 1.15  2000/07/04 06:44:16  johno
| Fix Lineup problem in heading
|
| Revision 1.14  2000/06/28 07:25:57  ramil
| SC#3056 LSANZ 16450 Modified to use the poln_due_date rather than po_hr_due_date in po validation.
|
| Revision 1.13  1999/12/06 01:32:30  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/05 05:17:07  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.11  1999/10/16 04:56:37  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.10  1999/10/14 03:04:20  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/10/13 22:51:17  cam
| Remove max_work
|
| Revision 1.8  1999/09/29 10:11:52  scott
| Updated to be consistant on function names.
|
| Revision 1.7  1999/09/21 04:37:55  scott
| Updated from Ansi project
|
| Revision 1.6  1999/06/17 10:06:16  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_cashprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_cashprt/po_cashprt.c,v 5.3 2001/10/17 08:58:52 cha Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include 	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>

#define	MAXCURR		300

	char	Curr_code [4];
 
	long	lsystemDate;
	int		maxCurrency = 0;
	int		printerNumber = 1;
	int		mth_sub;
	int		curr_sub;
	int		cur_month;
	int		cur_year;
	int		due_month;
	int		due_year;

	FILE	*pout;

	double	lc_duty [8];
	double	lc_lic [8];
	double	lc_other [8];

	struct {
		char	currency [4];
		float	fgn_mth [8];
  		float	loc_mth [8];
	} store [MAXCURR];
	
	char	*mth_name [] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
	};

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct pocrRecord	pocr_rec;

/*======================= 
| Function Declarations |
=======================*/
void	shutdown_prog 	(void);
void	OpenDB 			(void);
void	LoadCurrency 	(void);
void	CloseDB 		(void);
void	ProcessFile 	(void);
void	ProcessPohr 	(void);
int		ValidatePohr 	(void);
void	SetupSubs 		(char *);
void	ProcessPoln 	(long);
void	ProcessPoln 	(long);
int		heading 		(void);
void	EndReport 		(void);


/*========================== 
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	sprintf (Curr_code,"%-3.3s",get_env ("CURR_CODE"));

	if (argc < 2)
	{
		print_at (0,0, mlStdMess036 ,argv [0]);
        return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);

	lsystemDate = TodaysDate ();

	DateToDMY (lsystemDate, NULL, &cur_month, &cur_year);
	
	OpenDB ();

	LoadCurrency ();
	ProcessFile ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	EndReport ();
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec ("pohr", pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
	open_rec ("poln", poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec ("inmr", inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec ("pocr", pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*===========================================
| Load and clear overseas and Local totals. |
===========================================*/
void
LoadCurrency (
 void)
{
	int		j;
	/*------------
	| zero table |
	------------*/
	for (j = 0; j < 7; j++)
	{
		lc_other [j] = 0.0;
		lc_duty [j]  = 0.0;
		lc_lic [j]   = 0.0;
	}
	/*---------------------
	| load currency codes |
	---------------------*/
	maxCurrency = 0;
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,"   ");
	cc = find_rec ("pocr",&pocr_rec,GTEQ,"r");
	while (!cc && !strcmp (pocr_rec.co_no,comm_rec.co_no))
	{
		strcpy (store [maxCurrency].currency,pocr_rec.code);
		for (j = 0; j < 7; j++)
		{
			store [maxCurrency].fgn_mth [j] = 0.0;
			store [maxCurrency].loc_mth [j] = 0.0;
		}
		if (++maxCurrency == MAXCURR)
			break;
		cc = find_rec ("pocr",&pocr_rec,NEXT,"r");
	}
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (pocr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inmr);
	abc_dbclose ("data");
}

void
ProcessFile (
 void)
{
	dsp_screen (" Processing Cash Commitments Report.",comm_rec.co_no,comm_rec.co_name);

	heading ();

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,"  ");
	strcpy (pohr_rec.pur_ord_no,"               ");
	cc = find_rec ("pohr",&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no))
	{
		if (pohr_rec.status [0] == 'D')
		{
			cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
			continue;
		}
		ProcessPohr ();
		cc = find_rec ("pohr",&pohr_rec,NEXT,"r");
	}
}

void
ProcessPohr (
 void)
{
	dsp_process ("Order:",pohr_rec.pur_ord_no);
	ProcessPoln (pohr_rec.hhpo_hash);
}

void
SetupSubs (
	char	*curr_code)
{
	mth_sub  = 0;
	curr_sub = 0;

	if (due_year == cur_year)
	{
		mth_sub = (due_month - cur_month);
	}
	else 
	{
		if (due_year == cur_year + 1)
			mth_sub = (12 - cur_month) + due_month;
		else
			mth_sub = 6;
	}
	if (mth_sub > 6)
		mth_sub = 6;

	while (curr_sub < maxCurrency && strcmp (pohr_rec.curr_code,store [curr_sub].currency))
		curr_sub++;

	if (strcmp (curr_code,store [curr_sub].currency))
	{
		sprintf (err_str,"Currency %-3.3s invalid on P/O %s",pohr_rec.curr_code,pohr_rec.pur_ord_no);
		sys_err (err_str,cc,PNAME);
	}
		
}

int
GetValidPoln (long due_date)
{
    if (due_date == 0L)
	{
		DateToDMY (lsystemDate, NULL, &due_month, &due_year);
		if (due_month == 12)
		{
			due_month = 1;
			due_year++;
		}
		else
			due_month++;
	}
	else
		DateToDMY (poln_rec.due_date, NULL, &due_month, &due_year);

	if (due_year < cur_year)
		return (FALSE);
	if (due_year == cur_year && due_month < cur_month)
		return (FALSE);

	return (TRUE);
}

void
ProcessPoln (
 long hhpo_hash)
{
	float	qty_outst;
	double	fgn_cst = 0.00;
	double	frt_fgn = 0.00;
	double	nor_cst = 0.00;
	double	wk_value = 0.00;

	poln_rec.hhpo_hash	=	pohr_rec.hhpo_hash;
	cc = find_rec ("poln", &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpo_hash)
	{
	 	cc = GetValidPoln (poln_rec.due_date);
	 	if (cc == 0)
		 {
			 cc = find_rec ("poln", &poln_rec, NEXT, "r");
			 continue;
		 }

		SetupSubs (pohr_rec.curr_code);
		
		qty_outst = poln_rec.qty_ord - poln_rec.qty_rec;
		if (qty_outst > 0.00)
		{
			inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
			cc = find_rec ("inmr", &inmr_rec, COMPARISON,"r");
			if (cc)
				inmr_rec.outer_size = 1.00;

			wk_value = out_cost (poln_rec.fob_fgn_cst, inmr_rec.outer_size);

			fgn_cst = wk_value * (double) qty_outst;
			if (poln_rec.exch_rate != 0.00)
			{
				nor_cst = fgn_cst;
				nor_cst /= (double) poln_rec.exch_rate;
			}
			else
				nor_cst = 0.00;

			nor_cst = twodec (nor_cst);
			nor_cst += poln_rec.frt_ins_cst;

			wk_value = out_cost (poln_rec.frt_ins_cst, inmr_rec.outer_size);

			frt_fgn = wk_value * (double) qty_outst;
			frt_fgn = twodec (frt_fgn);
			frt_fgn *= poln_rec.exch_rate;

			fgn_cst += twodec (frt_fgn);

			store [curr_sub].fgn_mth [mth_sub] += (float) fgn_cst;
			store [curr_sub].loc_mth [mth_sub] += (float) nor_cst;
			wk_value = out_cost (poln_rec.duty, inmr_rec.outer_size);
			lc_duty [mth_sub] += wk_value * (double) qty_outst;

			wk_value = out_cost (poln_rec.lcost_load,inmr_rec.outer_size);
			lc_other [mth_sub] += wk_value * (double) qty_outst;

			wk_value = out_cost (poln_rec.licence, inmr_rec.outer_size);
			lc_lic [mth_sub] += wk_value * (double) qty_outst;
		}
		cc = find_rec ("poln", &poln_rec, NEXT, "r");
	}
}

int
heading (
 void)
{
	int	i;
	int	mth;
	int	yr = cur_year;

	if ((pout = popen ("pformat","w")) == NULL) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout,".START%s<%s>\n",DateToString (comm_rec.crd_date),PNAME);
	fprintf (pout,".LP%d\n",printerNumber);
	fprintf (pout,".12\n");
	fprintf (pout,".PI12\n");
	fprintf (pout,".L150\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".EPURCHASES CASH COMMITMENTS REPORT\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n",comm_rec.co_short,SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".R======================================================================================================================================================\n");
	fprintf (pout,"|====================================================================================================================================================|\n");

	fprintf (pout,"|CURR");
	for (i = 0; i < 6; i++)
	{
		mth = cur_month + i - 1;
		if (mth > 11)
		{
			mth -= 12;
			yr = cur_year + 1;
		}
		fprintf (pout,"|     %-3.3s %4d    ",mth_name [mth],yr);
	}
	fprintf (pout,"|    > %-3.3s %4d   ",mth_name [mth],yr);
	fprintf (pout,"|       TOTAL     |\n");

	fprintf (pout,"|    ");
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. ",Curr_code);
	fprintf (pout,"|   FGN      %-3.3s. |\n",Curr_code);

	fprintf (pout,"|----|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|\n");
	fflush (pout);

	return (EXIT_SUCCESS);
}

void
EndReport (
 void)
{
	int	i = 0;
	int	j;
	double	fgn_tot;
			double	loc_tot;
	double	loc_gtot [7];

	/*---------------------------
	| zero table				|
	---------------------------*/
	for (j = 0; j < 7; j++)
		loc_gtot [j] = 0.0;

	for (i = 0;i < maxCurrency;i++)
	{
		fgn_tot = 0.0;
		loc_tot = 0.0;
		fprintf (pout,"|%-3.3s ",store [i].currency);
		for (j = 0; j < 7; j++)
		{
			fprintf (pout,"|%8.0f",store [i].fgn_mth [j]);
			fprintf (pout," %8.0f",store [i].loc_mth [j]);
			loc_gtot [j] += (double) store [i].loc_mth [j];
			fgn_tot += (double) store [i].fgn_mth [j];
			loc_tot += (double) store [i].loc_mth [j];
		}
		fprintf (pout,"|%8.0f",fgn_tot);
		fprintf (pout," %8.0f|\n",loc_tot);
		fflush (pout);
	}

	loc_tot = 0.0;
	fprintf (pout,"|DUTY");
	for (j = 0; j < 7; j++)
	{
		fprintf (pout,"|       %10.0f",lc_duty [j]);
		loc_tot += lc_duty [j];
		loc_gtot [j] += lc_duty [j];
	}
	fprintf (pout,"|       %10.0f|\n",loc_tot);

	loc_tot = 0.0;
	fprintf (pout,"|LIC.");
	for (j = 0; j < 7; j++)
	{
		fprintf (pout,"|         %8.0f",lc_lic [j]);
		loc_tot += lc_lic [j];
		loc_gtot [j] += lc_lic [j];
	}
	fprintf (pout,"|         %8.0f|\n",loc_tot);

	loc_tot = 0.0;
	fprintf (pout,"|OTH.");
	for (j = 0; j < 7; j++)
	{
		fprintf (pout,"|         %8.0f",lc_other [j]);
		loc_tot += lc_other [j];
		loc_gtot [j] += lc_other [j];
	}
	fprintf (pout,"|         %8.0f|\n",loc_tot);

	fprintf (pout,"|----|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|\n");

	loc_tot = 0.0;
	fprintf (pout,"|TOT.");
	for (j = 0; j < 7; j++)
	{
		fprintf (pout,"|         %8.0f",loc_gtot [j]);
		loc_tot += loc_gtot [j];
	}
	fprintf (pout,"|         %8.0f|\n",loc_tot);
	fprintf (pout,".EOF\n");
	pclose (pout);
}
