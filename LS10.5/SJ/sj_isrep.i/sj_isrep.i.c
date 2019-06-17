/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( sj_isrep.i.c   )                                 |
|  Program Desc  : ( Input Program for Service invoice Summary    )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cudp,     ,     ,     ,     ,     ,         |
|  Database      : (stck)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 25/09/87         |
|---------------------------------------------------------------------|
|  Date Modified : (25/11/88)      | Modified  by  : B.C.Lim          |
|  Date Modified : (06/12/88)      | Modified  by  : B.C.Lim          |
|  Date Modified : (27/11/89)      | Modified  by  : Fui Choo Yap.    |
|  Date Modified : (16/10/97)      | Modified  by  : Roanna Marcelino |
|  Date Modified : (02/09/99)      | Modified  by  : Mars dela Cruz   |
|                                                                     |
|  Comments      : Tidy up program to use new screen generator.       |
|                : Change this to be the input program for sj_jobrep, |
|                : sj_jobrp2 & sj_emprep as well.                     |
|     (27/11/89) : Change moneytype fields to doubletype.             |
|     (16/10/97) : Updated to change sjis_invno from 6 to 8 chars.    |
|                                                                     |
| $Log: sj_isrep.i.c,v $
| Revision 5.3  2002/07/17 09:57:49  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:31  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:32  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:14:20  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:35:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:19:19  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:09:53  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.12  1999/12/06 01:34:26  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.11  1999/11/17 06:40:47  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.10  1999/11/16 05:58:32  scott
| Updated to fix warning errors due to -Wall flag.
|
| Revision 1.9  1999/09/29 10:12:58  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/24 05:06:34  scott
| Updated from Ansi
|
| Revision 1.7  1999/06/20 02:30:30  scott
| Updated for log required on cvs + removed old read_comm() + fixed warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_isrep.i.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_isrep.i/sj_isrep.i.c,v 5.3 2002/07/17 09:57:49 scott Exp $";

#include <ml_std_mess.h>
#include <ml_sj_mess.h>
#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dp_no"},
		{"comm_dp_name"},
		{"comm_dbt_date"},
	};

	int comm_no_fields = 8;

	struct {
		int		termno;
		char	tco_no[3];
		char	tco_name[41];
		char	tes_no[3];
		char	tes_name[41];
		char	tdp_no[3];
		char	tdp_name[41];
		long	tdbt_date;
	} comm_rec;

	/*====================================
	| Service Job Invoice Summary  File. |
	====================================*/
	struct dbview sjis_list[] ={
		{"sjis_co_no"},
		{"sjis_est_no"},
		{"sjis_dp_no"},
		{"sjis_invno"},
		{"sjis_order_no"},
		{"sjis_date"},
		{"sjis_chg_client"},
		{"sjis_end_client"},
		{"sjis_cost_estim"},
		{"sjis_invoice_cost"},
		{"sjis_invoice_chg"},
	};

	int sjis_no_fields = 11;

	struct {
		char	is_co_no[3];
		char	is_est_no[3];
		char	is_dp_no[3];
		char	is_invno[9];
		long	is_order_no;
		long	is_date;
		long	is_chg_client;
		long	is_end_client;
		double	is_cost_estim;
		double	is_invoice_cost;
		double	is_invoice_chg;
	} sjis_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
	};

	int cumr_no_fields = 4;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
	} cumr_rec;

	char	systemDate[11],
			prog_desc[51];

	char	*prog_name;

	int		lp_no = 1;
	int		input_wk = TRUE;

	double  tot_cost = 0.0,
			tot_estim = 0.0,
			tot_invd = 0.0,
			margin = 0.0;

	float	margin_pct = 0.0;

	FILE	*popen(const char *, const char *),
			*fout;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	char	back[5];
	char	onight[5];
	char	lpno[2];
	int		lp_no;
	char	rtype[9];
	char	s_fr_date[11];
	char	s_to_date[11];
	long	fr_date;
	long	to_date;
} local_rec;

static	struct	var	vars[]	={	

	{1, LIN, "lpno", 4, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", " Printer number ", " ", 
		YES, NO, JUSTRIGHT, "123456789", "", (char *)&local_rec.lp_no}, 
	{1, LIN, "back", 5, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", " Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 5, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", " Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{1, LIN, "from_date", 7, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, " From date ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.fr_date}, 
	{1, LIN, "to_date", 8, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, " To date ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.to_date}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*=====================
| Function Prototypes |
======================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
void set_dflt (void);
void print_tots (void);
void head_output (void);
void proc_sjis (void);
int heading (int scn);
int spec_valid (int field);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char *argv[])
{
	if (argc != 1 && argc != 4 && argc != 5)
	{
		print_at (0,0, mlStdMess037,argv[0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	switch (argc)
	{
	case 1 :
		prog_name = "sj_isrep.i";
		sprintf (prog_desc,"%-50.50s",ML(mlSjMess049));
		break;
	case 4 :
		prog_name = argv[1];
		sprintf (prog_desc,"%-50.50s",argv[2]);
		if (argv[3][0] == ' ')
		{
			input_wk = FALSE;
			vars[label ("from_date")].scn = 0;
		}
		else
			input_wk = TRUE;
		break;
	default :
		break;
	}
		
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	strcpy (systemDate, DateToString (TodaysDate()));

	/*==============================
	| Read common terminal record. |
	==============================*/
	OpenDB ();
	read_comm( comm_list, comm_no_fields, (char *) &comm_rec );

	if (argc == 5)
	{
		sprintf (prog_desc,"%-50.50s",argv[1]);
		lp_no = atoi (argv[2]);

		local_rec.fr_date = StringToDate (argv[3]);

		if (local_rec.fr_date < 0L)
		{
			print_at (0,0,ML (mlSjMess045),argv[3]);
			return (EXIT_FAILURE);
		}

		local_rec.to_date = StringToDate (argv[4]);

		if (local_rec.to_date < 0L)
		{
			print_at (0,0,ML (mlStdMess046),argv[4]);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.s_fr_date,"%-10.10s",argv[3]);
		sprintf (local_rec.s_to_date,"%-10.10s",argv[4]);

		head_output ();  /* initialise report            */
		proc_sjis (); 	/* Process invoice summary file */
		print_tots ();     /* print totals for report      */
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*=====================
	| Reset control flags |
	=====================*/
   	entry_exit = 0;
   	prog_exit = 0;
   	restart = 0;
   	search_ok = TRUE;

	init_vars (1);

	set_dflt ();

	heading (1);
	scn_display (1);
	edit (1);
	prog_exit = 1;
	if (restart)
	{
		shutdown_prog ();
        return (EXIT_SUCCESS);	
	}
	shutdown_prog ();

	strcpy (local_rec.s_fr_date,DateToString(local_rec.fr_date));
	strcpy (local_rec.s_to_date,DateToString(local_rec.to_date));

	sprintf (local_rec.lpno,"%2d",local_rec.lp_no);

	clear ();
	print_at (0,0,ML (mlStdMess035));
	
	fflush (stdout);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight[0] == 'Y')
	{
		if (fork() == 0)
		{
			if (input_wk)
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					argv[2],
					local_rec.lpno,
					local_rec.s_fr_date,
					local_rec.s_to_date,
					argv[2],(char *)0);
			else
				execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					argv[2],
					local_rec.lpno,
					argv[2],(char *)0);

		}
	}
	else
	/*====================================
	| Test for forground or background . |
	====================================*/
	if (local_rec.back[0] == 'Y') 
	{
		if (fork() == 0)
		{
			if (input_wk)
				execlp(prog_name,
					prog_name,
					argv[2],
					local_rec.lpno,
					local_rec.s_fr_date,
					local_rec.s_to_date,(char *)0);
			else
				execlp(prog_name,
					prog_name,
					argv[2],
					local_rec.lpno,(char *)0);
		}
	}
	else 
	{
		if (input_wk)
			execlp(prog_name,
				prog_name,
				argv[2],
				local_rec.lpno,
				local_rec.s_fr_date,
				local_rec.s_to_date,(char *)0);
		else
			execlp(prog_name,
				prog_name,
				argv[2],
				local_rec.lpno,(char *)0);
	}
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

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	open_rec ("sjis", sjis_list, sjis_no_fields, "sjis_id_no3");
	open_rec ("cumr", cumr_list, cumr_no_fields, "cumr_hhcu_hash");
}

void
CloseDB (
 void)
{
	abc_fclose ("sjis");
	abc_fclose ("cumr");
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("from_date"))
	{
		if (local_rec.fr_date > StringToDate(systemDate))
		{
			print_mess (ML(mlStdMess086));
			return (EXIT_FAILURE);
		}

		if (local_rec.fr_date > local_rec.to_date)
		{
			print_mess (ML (mlStdMess019));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("to_date"))
	{
		if (local_rec.to_date < local_rec.fr_date )
		{
			print_mess (ML (mlStdMess019));
			return (EXIT_FAILURE);
		}

		if (local_rec.to_date > StringToDate(systemDate))
		{
			print_mess (ML (mlStdMess226));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);	
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lp_no = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if ( !valid_lp (local_rec.lp_no))
		{
			errmess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back,(local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight,(local_rec.onight[0] == 'Y') ? "Y(es" : "N(o ");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
set_dflt (
 void)
{
	local_rec.lp_no = 1;
	strcpy (local_rec.back,"N(o ");
	strcpy (local_rec.onight,"N(o ");
	local_rec.fr_date = StringToDate (systemDate);
	local_rec.to_date = StringToDate (systemDate);
}

void
print_tots (
 void)
{
	/*==============
	| Print totals |
	================*/
	margin = tot_invd - tot_cost;
	if (tot_invd != 0.00)
		margin_pct = (float) (margin / tot_invd * 100.0);
	else
		margin_pct = 0.0;

	fprintf (fout,"|            |        |          |          | Totals   | %9.2f |  %9.2f | %9.2f | %9.2f | %9.2f |\n",
		tot_cost,
		tot_estim,
		tot_invd,
		margin,
		margin_pct);
	fprintf (fout,".EOF\n");
}

void
head_output (
 void)
{
	dsp_screen ("Printing Service Invoice Summary",comm_rec.tco_no,comm_rec.tco_name);

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
	fprintf (fout,".LP%d\n",lp_no);
	fprintf (fout,".12\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L124\n");
	fprintf (fout,".ECOMPANY %s - %s\n",comm_rec.tco_no,clip(comm_rec.tco_name));

	fprintf (fout,".R|====================================================================================================================|\n");
	fprintf (fout,".ESERVICE INVOICE SUMMARY\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime());
	fprintf (fout,".EFROM %-10.10s  TO %-10.10s\n",
		local_rec.s_fr_date,
		local_rec.s_to_date);
	fprintf (fout,"\n");

	fprintf (fout,"|===================================================================================================================|\n");

	fprintf (fout,"|            |        |  Charge  |   End    |          |           |            |           |           |  Margin   |\n");
	fprintf (fout,"|  Invoice   | Order  |  Client  |  Client  |   Date   |   Cost    |  Estimate  |  Invoice  |  Margin   |    %%      |\n");
	fprintf (fout,"|------------|--------|----------|----------|----------|-----------|------------|-----------|-----------|-----------|\n");
}

void
proc_sjis (
 void)
{
	long	last_order = 0L;
	int		first_time = TRUE;
	int		printed = FALSE;
	char	order_no[9];
	char	chg_dbt_no[8];
	char	end_dbt_no[8];
	
	strcpy (sjis_rec.is_co_no,comm_rec.tco_no);
	strcpy (sjis_rec.is_est_no,comm_rec.tes_no);
	strcpy (sjis_rec.is_dp_no,comm_rec.tdp_no);
	sjis_rec.is_order_no = 0L;
	sjis_rec.is_date     = local_rec.fr_date;
	sprintf (sjis_rec.is_invno,"%-8.8s"," ");
	cc = find_rec ("sjis",&sjis_rec,GTEQ,"r");

	while (!cc && !strcmp (sjis_rec.is_co_no,comm_rec.tco_no) && 
				  !strcmp (sjis_rec.is_est_no,comm_rec.tes_no) && 
				  !strcmp (sjis_rec.is_dp_no,comm_rec.tdp_no) && 
				  sjis_rec.is_date >= local_rec.fr_date && 
				  sjis_rec.is_date <= local_rec.to_date)
	{
		printed = TRUE;
		sprintf (order_no,"%8ld",sjis_rec.is_order_no);
		dsp_process ("Order No :",order_no);

		tot_cost 	+= sjis_rec.is_invoice_cost;
		tot_invd 	+= sjis_rec.is_invoice_chg;

		/*================================
		|  only accumulate estimated once |
		|  per order                      |
		=================================*/
		if (last_order != sjis_rec.is_order_no)
		{
			tot_estim += sjis_rec.is_cost_estim;
			last_order = sjis_rec.is_order_no;
			if (!first_time)
				fprintf (fout,"|------------|--------|----------|----------|----------|-----------|------------|-----------|-----------|-----------|\n");
			first_time = FALSE;
		}

		margin = sjis_rec.is_invoice_chg - sjis_rec.is_invoice_cost;

		if (margin != 0.0)
			margin_pct = (float) ( (margin / sjis_rec.is_invoice_chg) * 100.0);
		else
			margin_pct = 0.0;

		cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sjis_rec.is_chg_client);
		if (cc)
			strcpy (chg_dbt_no,"Unknown");
		else
			strcpy (chg_dbt_no,cumr_rec.cm_dbt_no);

		cc = find_hash ("cumr",&cumr_rec,COMPARISON,"r",sjis_rec.is_end_client);
		if (cc)
			strcpy (end_dbt_no,"Unknown");
		else
			strcpy (end_dbt_no,cumr_rec.cm_dbt_no);

		fprintf (fout,"| %-10.10s |%8ld| %-7.7s  | %-7.7s  | %-8.8s | %9.2f |  %9.2f | %9.2f | %9.2f | %9.2f |\n",
			sjis_rec.is_invno,
			sjis_rec.is_order_no,
			chg_dbt_no,
			end_dbt_no,
			DateToString(sjis_rec.is_date),
			sjis_rec.is_invoice_cost,
			sjis_rec.is_cost_estim,
			sjis_rec.is_invoice_chg,
			margin,
			margin_pct);

		cc = find_rec ("sjis",&sjis_rec,NEXT,"r");
	}
	if (printed)
		fprintf (fout,"|------------|--------|----------|----------|----------|-----------|------------|-----------|-----------|-----------|\n");
	
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	int	y = (80 - strlen(clip(prog_desc))) / 2;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (prog_desc,y,0,1);
		move (0,1);
		line (80);

		box (0,3,80, (!input_wk) ? 2 : 5);
		if (input_wk)
		{
			move (1,6);
			line (79);
		}

		move (0,19);
		line (80);
		strcpy (err_str,ML(mlStdMess038));
		print_at (20,1,err_str,comm_rec.tco_no,clip(comm_rec.tco_name));
		strcpy (err_str,ML(mlStdMess039));
		print_at (21,1,err_str,comm_rec.tes_no,clip(comm_rec.tes_name));
		move (0,22);
		line (80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}
