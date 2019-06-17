/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _gst_aud.c,v 5.3 2002/07/17 09:57:25 scott Exp $
|  Program Name  : (psl_gst_aud.c)   
|  Program Desc  : (Print G.S.T Audits)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 06/06/91         |
|---------------------------------------------------------------------|
| $Log: _gst_aud.c,v $
| Revision 5.3  2002/07/17 09:57:25  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 05:13:43  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:35  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _gst_aud.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/psl_gst_aud/_gst_aud.c,v 5.3 2002/07/17 09:57:25 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_menu_mess.h>

#define	ADD_VAL 	 (!strcmp (extr_rec.jnl_type , " 4") || \
		  	  		 !strcmp (extr_rec.jnl_type , " 8"))

#define	SUB_VAL 	 (!strcmp (extr_rec.jnl_type , " 5") || \
		  	  		 !strcmp (extr_rec.jnl_type , " 7"))

#define	GST_RETURN	 (!strcmp (prog_name, "psl_gst_ret"))

#define	SUMMARY		 (summary [0] == 'S')

extern	int	GV_fiscal;

	float	gst_include = 0.00;
	float	gst_div = 1.00;
	float	gst_pc = 0.00;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct extrRecord	extr_rec;

	double	run_sal_val = 0.00,
			run_gst_val = 0.00,
			typ_sal_val = 0.00,
			typ_gst_val = 0.00,
			grd_sal_val = 0.00,
			grd_gst_val = 0.00;

	int		lpno,
			first_time = TRUE;

	char	gst_code [4],
			lower [3],
			upper [3],
			type [2],
			summary [2],
			prog_name [100];

	long	run_no = 0L;
	long	min_date = 0L,
			max_date = 0L;

	FILE *	fout;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	back [5];
	char	onight [5];
	char	tr_type [2];
	char	tr_summary [2];
	int	s_per;
	int	e_per;
	int	lpno;
	char	lp_str [3];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "start_per", 5, 20, INTTYPE, 
		"NN", "          ", 
		" ", "0", "Start Period", " ", 
		YES, NO, JUSTLEFT, "0", "12", (char *) &local_rec.s_per}, 
	{1, LIN, "end_per", 6, 20, INTTYPE, 
		"NN", "          ", 
		" ", "0", "End  Period", " ", 
		YES, NO, JUSTLEFT, "0", "12", (char *) &local_rec.e_per}, 
	{1, LIN, "tr_type", 7, 20, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Transaction type.", "C (ustomer) S (upplier) B (oth).", 
		YES, NO, JUSTLEFT, "CSB", "", local_rec.tr_type}, 
	{1, LIN, "tr_summary", 7, 60, CHARTYPE, 
		"U", "          ", 
		" ", "D", "Summary report .", "D (etailed) S (ummary).", 
		YES, NO, JUSTLEFT, "DS", "", local_rec.tr_summary}, 
	{1, LIN, "lpno", 9, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer number ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 
	{1, LIN, "back", 10, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Background ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 11, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N(o ", "Overnight ", " ", 
		YES, NO, JUSTRIGHT, "YN", "", local_rec.onight}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

/*============================
| Local function prototypes  |
============================*/
void	run_prog		 (void);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB		 (void);
int		spec_valid		 (int);
void	head_aud		 (void);
void	head_ret		 (void);
void	print_line		 (void);
int		proc_aud		 (void);
void	proc_line		 (void);
void	pr_run_tot		 (void);
void	pr_typ_tot		 (char *);
void	pr_grp_tot		 (void);
int		heading			 (int);
int		proc_ret		 (void);
void	min_max_date	 (long);


/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char dsp_comment [40];

	if (argc != 1 && argc != 6)
	{
		/*Usage : %s or
		  Usage : %s <lpno> <lower> <upper> <type> <summary>,argv [0]*/
		print_at (0,0,ML (mlMenuMess090),argv [0]);
		return (EXIT_FAILURE);
	}
	SETUP_SCR (vars);

	/*---------------
	| Read gst code. |
	----------------*/

	sprintf (gst_code, "%-3.3s", get_env ("GST_TAX_NAME"));

	OpenDB ();

	if (comm_rec.gst_rate != 0.00)
	{
		gst_div = (float) (( (100.00 + comm_rec.gst_rate) / comm_rec.gst_rate));
		gst_pc  = comm_rec.gst_rate;
	}
	else
	{
		gst_div = 1.00;
		gst_pc  = 0.00;
	}
	sprintf (prog_name, "%-11.11s", argv [0]);

	/*===========================
	| Open main database files. |  
	===========================*/
	if (argc == 6)
	{
		lpno = atoi (argv [1]);
		sprintf (lower,"%02d", atoi (argv [2]));
		sprintf (upper,"%02d", atoi (argv [3]));
		sprintf (type,"%-1.1s",argv [4]);
		sprintf (summary,"%-1.1s",argv [5]);

		sprintf (dsp_comment, "Processing %-3.3s Report.", gst_code);

		dsp_screen (dsp_comment, comm_rec.co_no, comm_rec.co_name);

		if (GST_RETURN) 
		{
			head_ret ();
			proc_ret ();
		}
		else
		{
			head_aud ();
			proc_aud ();
		}
		fprintf (fout,".EOF\n");
		pclose (fout);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	if (GST_RETURN)
	{
		FLD ("tr_type") = NA;
		FLD ("tr_summary") = ND;
	}
	else
	{
		FLD ("tr_type") = YES;
		FLD ("tr_summary") = YES;
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	while (prog_exit == 0)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

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

		run_prog ();
		prog_exit = 1;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
run_prog (
 void)
{	
	char	s_per [3],
			e_per [3],
			comment [20];

	rset_tty ();


	sprintf (s_per,   "%02d", local_rec.s_per);
	sprintf (e_per,   "%02d", local_rec.e_per);
	sprintf (comment, "Print %-3.3s. Report.", gst_code);

	clear ();
	/*Busy, ....*/
	print_at (0, 0, ML (mlStdMess035));

	fflush (stdout);

	/*================================
	| Test for Overnight Processing. | 
	================================*/
	if (local_rec.onight [0] == 'Y')
	{ 
		if (fork () == 0)
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.lp_str,
					s_per,
					e_per,
					local_rec.tr_type,
					local_rec.tr_summary,
					comment, 
					 (char *)0);
		else
			return;
	}
	/*====================================
	| Test for forground or background . |
	====================================*/
	else
	if (local_rec.back [0] == 'Y') 
		{
			if (fork () != 0)
				return;
			else
				execlp (prog_name,
					prog_name,
					local_rec.lp_str,
					s_per,
					e_per, 
					local_rec.tr_type, 
					local_rec.tr_summary,
					 (char *)0);
		}
		else 
		{
			execlp (prog_name,
				prog_name,
				local_rec.lp_str,
				s_per,
				e_per,
				local_rec.tr_type, 
				local_rec.tr_summary,
				 (char *)0);
		}
	shutdown_prog ();
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();;
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,comr_list,COMR_NO_FIELDS,"comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in comr During (DBFIND)",cc,PNAME);

	abc_fclose (comr);

	open_rec (extr,extr_list,EXTR_NO_FIELDS,"extr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (extr);
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	short	fdmy [3];

	if (LCHECK ("start_per") && dflt_used)
	{
		get_fdmy (fdmy, comm_rec.gl_date);
		local_rec.s_per = fdmy [1];
		DSP_FLD ("start_per");
		display_field (field);
	}
	
	if (LCHECK ("end_per") && dflt_used)
	{
		get_fdmy (fdmy, comm_rec.gl_date);
		local_rec.e_per = fdmy [1];
		DSP_FLD ("end_per");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("tr_type"))
	{
		if (FLD ("tr_type") == NA)
		{
			strcpy (local_rec.tr_type, "B");
			DSP_FLD ("tr_type");
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("tr_summary"))
	{
		if (FLD ("tr_summary") == ND)
			strcpy (local_rec.tr_summary, "D");
		
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
			/*This system has only %d printers",no_lps ()*/
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.lp_str,"%d",local_rec.lpno);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') ? "Y(es" : "N(o ");
		display_field (field);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onight, (local_rec.onight [0] == 'Y') ? "Y(es" : "N(o ");
		display_field (field);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_aud (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",lpno);
	fprintf (fout,".13\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L100\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	if (type [0] == 'C')
		fprintf (fout,".E%-3.3s AUDIT REPORT. (INPUT TAX ONLY)\n", gst_code);

	if (type [0] == 'S')
		fprintf (fout,".E%-3.3s AUDIT REPORT. (OUTPUT TAX ONLY)\n", gst_code);

	if (type [0] == 'B')
		fprintf (fout,".E%-3.3s AUDIT REPORT. (INPUT & OUTPUT TAX)\n", gst_code);
	
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s REPORT FROM G/L PERIOD %s TO G/L PERIOD %s\n",
			 (SUMMARY) ? "SUMMARY" : "DETAILED", lower, upper);
	fprintf (fout,".B1\n");
	fprintf (fout,".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout,".R=========");
	fprintf (fout,"============");
	fprintf (fout,"==============");
	fprintf (fout,"==============");
	fprintf (fout,"==================");
	fprintf (fout,"==============");
	fprintf (fout,"===============\n");

	fprintf (fout,"=========");
	fprintf (fout,"============");
	fprintf (fout,"==============");
	fprintf (fout,"==============");
	fprintf (fout,"==================");
	fprintf (fout,"==============");
	fprintf (fout,"===============\n");

	fprintf (fout,"| RUN NO ");
	fprintf (fout,"| GL PERIOD ");
	fprintf (fout,"| DATE POSTED ");
	if (type [0] == 'C')
		fprintf (fout,"| CUSTOMER NO.");

	if (type [0] == 'S')
		fprintf (fout,"| SUPPLIER NO ");

	if (type [0] == 'B')
		fprintf (fout,"| DBT/CRD NO. ");

	fprintf (fout,"|INVOICE/CREDIT NO");
	fprintf (fout,"| GROSS VALUE ");
	fprintf (fout,"|  %-3.3s AMOUNT |\n", gst_code);

	print_line ();
	fflush (fout);
}
/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
head_ret (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)",errno,PNAME);

	fprintf (fout,".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",lpno);
	fprintf (fout,".OP\n");
	fprintf (fout,".6\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L80\n");
	fprintf (fout,".C%s\n",clip (comm_rec.co_name));
	fprintf (fout,".B1\n");

	fprintf (fout,".C%c.%c.%c.   T A X   R E T U R N\n", 
			gst_code [0], gst_code [1], gst_code [2]);

	fprintf (fout,".B1\n");

	fflush (fout);
}

void
print_line (
 void)
{
	fprintf (fout,"|--------");
	fprintf (fout,"|-----------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|-----------------");
	fprintf (fout,"|-------------");
	fprintf (fout,"|-------------|\n");
	fflush (fout);
}

/*====================================
| Process File by Co/Class/Category. |
====================================*/
int
proc_aud (
 void)
{
	int	data_found = FALSE;

	if (type [0] == 'C' || type [0] == 'B')
	{
		first_time = TRUE;
		strcpy (extr_rec.co_no, comm_rec.co_no);
		strcpy (extr_rec.jnl_type, " 4");
		strcpy (extr_rec.gl_per, lower);
		extr_rec.run_no = 0L;
		strcpy (extr_rec.int_no, "        ");
		strcpy (extr_rec.ref_no, "               ");
		cc = find_rec (extr, &extr_rec, GTEQ, "r");
		while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
		       	       !strcmp (extr_rec.jnl_type, " 4") &&
	                	strcmp (extr_rec.gl_per,lower) >= 0 && 
	                	strcmp (extr_rec.gl_per,upper) <= 0)
		{
			data_found = TRUE;
			proc_line ();
			cc = find_rec (extr, &extr_rec, NEXT, "r");
		}
		if (data_found) 
		{
			pr_run_tot ();
			pr_typ_tot ("CUSTOMERS INVOICES.");
		}
		data_found = FALSE;

		first_time = TRUE;
		strcpy (extr_rec.co_no, comm_rec.co_no);
		strcpy (extr_rec.jnl_type, " 5");
		strcpy (extr_rec.gl_per, lower);
		extr_rec.run_no = 0L;
		strcpy (extr_rec.int_no, "        ");
		strcpy (extr_rec.ref_no, "               ");
		cc = find_rec (extr, &extr_rec, GTEQ, "r");
		while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
		       	       !strcmp (extr_rec.jnl_type, " 5") &&
	                	strcmp (extr_rec.gl_per,lower) >= 0 && 
	                	strcmp (extr_rec.gl_per,upper) <= 0)
		{
			data_found = TRUE;
			proc_line ();
			cc = find_rec (extr, &extr_rec, NEXT, "r");
		}
		if (data_found) 
		{
			pr_run_tot ();
			pr_typ_tot ("CUSTOMERS CREDIT NOTES.");
		}
		data_found = FALSE;

	}
	if (type [0] == 'S' || type [0] == 'B')
	{
		first_time = TRUE;
		strcpy (extr_rec.co_no, comm_rec.co_no);
		strcpy (extr_rec.jnl_type, " 7");
		strcpy (extr_rec.gl_per, lower);
		extr_rec.run_no = 0L;
		strcpy (extr_rec.int_no, "        ");
		strcpy (extr_rec.ref_no, "               ");
		cc = find_rec (extr, &extr_rec, GTEQ, "r");
		while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
		       	       !strcmp (extr_rec.jnl_type, " 7") &&
	                	strcmp (extr_rec.gl_per,lower) >= 0 && 
	                	strcmp (extr_rec.gl_per,upper) <= 0)
		{
			data_found = TRUE;
			proc_line ();
			cc = find_rec (extr, &extr_rec, NEXT, "r");
		}
		if (data_found) 
		{
			pr_run_tot ();
			pr_typ_tot ("SUPPLIER INVOICES.");
		}
		data_found = FALSE;

		first_time = TRUE;
		strcpy (extr_rec.co_no, comm_rec.co_no);
		strcpy (extr_rec.jnl_type, " 8");
		strcpy (extr_rec.gl_per, lower);
		extr_rec.run_no = 0L;
		strcpy (extr_rec.int_no, "        ");
		strcpy (extr_rec.ref_no, "               ");
		cc = find_rec (extr, &extr_rec, GTEQ, "r");
		while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
		       	       !strcmp (extr_rec.jnl_type, " 8") &&
	                	strcmp (extr_rec.gl_per,lower) >= 0 && 
	                	strcmp (extr_rec.gl_per,upper) <= 0)
		{
			data_found = TRUE;
			proc_line ();
			cc = find_rec (extr, &extr_rec, NEXT, "r");
		}
		if (data_found) 
		{
			pr_run_tot ();
			pr_typ_tot ("SUPPLIERS CREDIT NOTES.");
		}
		data_found = FALSE;
	}
	pr_grp_tot ();
	fflush (fout);
	return (EXIT_SUCCESS);
}
/*=======================
| Print Line items.	|
=======================*/
void
proc_line (
 void)
{
	if (first_time)
		run_no = extr_rec.run_no;

	first_time = FALSE;

	if (extr_rec.run_no != run_no)
		pr_run_tot ();
	
	dsp_process ("Customer ", extr_rec.int_no);
	if (!SUMMARY)
	{
		fprintf (fout,"| %6ld ", 	extr_rec.run_no);
		fprintf (fout,"|     %2.2s    ", extr_rec.gl_per);
		fprintf (fout,"|  %10.10s ", 	DateToString (extr_rec.date));
		fprintf (fout,"|   %-8.8s  ",  extr_rec.int_no);
		fprintf (fout,"| %-15.15s ",     extr_rec.ref_no);
		fprintf (fout,"|%12.2f ", (ADD_VAL) ? extr_rec.sal_val 
						  : extr_rec.sal_val * -1);
		fprintf (fout,"|%12.2f |\n", (ADD_VAL) ? extr_rec.gst_val
						    : extr_rec.gst_val * -1);
	}
	if (ADD_VAL)
	{
		run_sal_val += extr_rec.sal_val;
		run_gst_val += extr_rec.gst_val;
		typ_sal_val += extr_rec.sal_val;
		typ_gst_val += extr_rec.gst_val;
		grd_sal_val += extr_rec.sal_val;
		grd_gst_val += extr_rec.gst_val;
	}
	else
	{
		run_sal_val -= extr_rec.sal_val;
		run_gst_val -= extr_rec.gst_val;
		typ_sal_val -= extr_rec.sal_val;
		typ_gst_val -= extr_rec.gst_val;
		grd_sal_val -= extr_rec.sal_val;
		grd_gst_val -= extr_rec.gst_val;
	}
	run_no = extr_rec.run_no;

	fflush (fout);
}

void
pr_run_tot (
 void)
{
	if (!SUMMARY)
		print_line ();
	fprintf (fout,"| %6ld ", 	run_no);
	fprintf (fout,"| TOTAL FOR RUN");
	fprintf (fout,"           ");
	fprintf (fout,"|             ");
	fprintf (fout,"|                 ");
	fprintf (fout,"|%12.2f ", run_sal_val);
	fprintf (fout,"|%12.2f |\n", run_gst_val);
	run_sal_val = 0.00;
	run_gst_val = 0.00;
	if (!SUMMARY)
		print_line ();
}

void
pr_typ_tot (
 char *	desc)
{
	fprintf (fout,"|        ");
	fprintf (fout,"| TOTAL FOR : ");
	fprintf (fout,"%-25.25s", desc);
	fprintf (fout," |                 ");
	fprintf (fout,"|%12.2f ", typ_sal_val);
	fprintf (fout,"|%12.2f |\n", typ_gst_val);
	typ_sal_val = 0.00;
	typ_gst_val = 0.00;
	print_line ();
}

void
pr_grp_tot (
 void)
{
	fprintf (fout,"|        ");
	fprintf (fout,"| COMPANY TOTAL");
	fprintf (fout,"           ");
	fprintf (fout,"|             ");
	fprintf (fout,"|                 ");
	fprintf (fout,"|%12.2f ", grd_sal_val);
	fprintf (fout,"|%12.2f |\n", grd_gst_val);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		if (GST_RETURN)
			sprintf (err_str, ML (mlMenuMess084));
		else
			sprintf (err_str, ML (mlMenuMess085));

		rv_pr (err_str, (80 - strlen (err_str)) / 2,0,1);
		line_at (1,0,80);
		line_at (8,1,79);
		box (0,4,80,7);

		line_at (19,0,80);
		strcpy (err_str, ML (mlStdMess038));
		print_at (20,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (21,0,err_str,comm_rec.est_no,comm_rec.est_name);
		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*====================================
| Process File by Co/Class/Category. |
====================================*/
int
proc_ret (
 void)
{
	double	tot_a 	= 0.00,
			tot_b 	= 0.00,
			tot_c 	= 0.00,
			tot_d 	= 0.00,
			tot_e 	= 0.00,
			tot_f 	= 0.00;

	char	s_date [11],
			e_date [11];

	long	s_ldate,
			e_ldate;


	int		s_month,
			e_month;
	
	int	no_months;

	strcpy (extr_rec.co_no, comm_rec.co_no);
	strcpy (extr_rec.jnl_type, " 4");
	strcpy (extr_rec.gl_per, lower);
	extr_rec.run_no = 0L;
	strcpy (extr_rec.int_no, "        ");
	strcpy (extr_rec.ref_no, "               ");
	cc = find_rec (extr, &extr_rec, GTEQ, "r");
	while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
	       	       !strcmp (extr_rec.jnl_type, " 4") &&
	               	strcmp (extr_rec.gl_per,lower) >= 0 && 
	               	strcmp (extr_rec.gl_per,upper) <= 0)
	{
		dsp_process ("Customer ", extr_rec.int_no);
		if (extr_rec.gst_val != 0.00)
			tot_a = tot_a + extr_rec.sal_val;

		tot_b = tot_b + extr_rec.gst_val;
		if (extr_rec.gst_val == 0.00)
			tot_c = tot_c + extr_rec.sal_val;

		min_max_date (extr_rec.date);
		
		cc = find_rec (extr, &extr_rec, NEXT, "r");
	}
	strcpy (extr_rec.co_no, comm_rec.co_no);
	strcpy (extr_rec.jnl_type, " 5");
	strcpy (extr_rec.gl_per, lower);
	extr_rec.run_no = 0L;
	strcpy (extr_rec.int_no, "        ");
	strcpy (extr_rec.ref_no, "               ");
	cc = find_rec (extr, &extr_rec, GTEQ, "r");
	while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
	       	       !strcmp (extr_rec.jnl_type, " 5") &&
	               	strcmp (extr_rec.gl_per,lower) >= 0 && 
	               	strcmp (extr_rec.gl_per,upper) <= 0)
	{
		dsp_process ("Customer ", extr_rec.int_no);
		if (extr_rec.gst_val != 0.00)
			tot_a = tot_a - extr_rec.sal_val;

		tot_b = tot_b - extr_rec.gst_val;
		if (extr_rec.gst_val == 0.00)
			tot_c = tot_c - extr_rec.sal_val;

		min_max_date (extr_rec.date);
		cc = find_rec (extr, &extr_rec, NEXT, "r");
	}
	strcpy (extr_rec.co_no, comm_rec.co_no);
	strcpy (extr_rec.jnl_type, " 7");
	strcpy (extr_rec.gl_per, lower);
	extr_rec.run_no = 0L;
	strcpy (extr_rec.int_no, "        ");
	strcpy (extr_rec.ref_no, "               ");
	cc = find_rec (extr, &extr_rec, GTEQ, "r");
	while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
	       	       !strcmp (extr_rec.jnl_type, " 7") &&
	               	strcmp (extr_rec.gl_per,lower) >= 0 && 
	               	strcmp (extr_rec.gl_per,upper) <= 0)
	{
		dsp_process ("Customer ", extr_rec.int_no);
		if (extr_rec.gst_val != 0.00)
			tot_d = tot_d + extr_rec.sal_val;

		tot_e = tot_e + extr_rec.gst_val;
		if (extr_rec.gst_val == 0.00)
			tot_f = tot_f + extr_rec.sal_val;

		min_max_date (extr_rec.date);
		cc = find_rec (extr, &extr_rec, NEXT, "r");
	}
	strcpy (extr_rec.co_no, comm_rec.co_no);
	strcpy (extr_rec.jnl_type, " 8");
	strcpy (extr_rec.gl_per, lower);
	extr_rec.run_no = 0L;
	strcpy (extr_rec.int_no, "        ");
	strcpy (extr_rec.ref_no, "               ");
	cc = find_rec (extr, &extr_rec, GTEQ, "r");
	while (!cc && !strcmp (extr_rec.co_no, comm_rec.co_no) &&
	       	       !strcmp (extr_rec.jnl_type, " 8") &&
	               	strcmp (extr_rec.gl_per,lower) >= 0 && 
	               	strcmp (extr_rec.gl_per,upper) <= 0)
	{
		dsp_process ("Customer ", extr_rec.int_no);
		if (extr_rec.gst_val != 0.00)
			tot_d = tot_d - extr_rec.sal_val;

		tot_e = tot_e - extr_rec.gst_val;
		if (extr_rec.gst_val == 0.00)
			tot_f = tot_f - extr_rec.sal_val;

		min_max_date (extr_rec.date);

		cc = find_rec (extr, &extr_rec, NEXT, "r");
	}
	sprintf (s_date, "%10.10s", DateToString (MonthStart (min_date)));
	sprintf (e_date, "%10.10s", DateToString (MonthEnd (max_date)));
	s_ldate = MonthStart (min_date);
	e_ldate = MonthEnd (max_date);

	DateToDMY (s_ldate, NULL,&s_month,NULL);
	DateToDMY (e_ldate, NULL,&e_month,NULL);
	no_months = (e_month - s_month) + 1;

	fprintf (fout, "For %d month (s) from %s to %s %10.10s %-3.3s NUMBER (%s)\n",
			no_months, s_date, e_date," ", gst_code,
			clip (comr_rec.gst_ird_no));

	fprintf (fout, ".B1\n");

	fprintf (fout, "Total taxable supplies for the period (incl %-3.3s)   %12.2f\n", gst_code, tot_a + tot_c);
	fprintf (fout, ".B1\n");

	fprintf (fout, "Less Zero Rated Supplies                             %12.2f\n", tot_c);
	fprintf (fout, ".B1\n");
		
	fprintf (fout, "Equals                                               %12.2f\n", tot_a);
	fprintf (fout, ".B1\n");
/*
	fprintf (fout, "Divide by %2.2f                                      %12.2f\n", gst_div, twodec (tot_a / gst_div));
*/
	fprintf (fout, "Divide by %2.2f                                      %12.2f\n", gst_div, twodec (tot_b));

	fprintf (fout, ".B1\n");
/*
	fprintf (fout, "            Sum of %-3.3s. charged (Output tax)         %12.2f\n", gst_code, twodec (tot_a / gst_div));
*/
	fprintf (fout, "            Sum of %-3.3s. charged (Output tax)         %12.2f\n", gst_code, twodec (tot_b));
	fprintf (fout, ".B1\n");

	fprintf (fout, "Total taxable supplies received (incl %-3.3s.) but\n", gst_code);
	fprintf (fout, "excluding imported goods                             %12.2f\n", tot_d + tot_e);
	fprintf (fout, ".B1\n");

	fprintf (fout, "Divide by %2.2f Equals                               %12.2f\n", gst_div, twodec ((tot_d + tot_e) / gst_div));
	fprintf (fout, ".B1\n");

	fprintf (fout, "            Total deductions                         %12.2f\n", twodec ((tot_d + tot_e) / gst_div));
	fprintf (fout, ".B1\n");

	fprintf (fout, "            NET %-3.3s. PAYABLE (REFUND)              %12.2f\n", gst_code, twodec ((tot_a - (tot_d + tot_e)) / gst_div));
	fprintf (fout, ".B1\n");

	fflush (fout);
	return (EXIT_SUCCESS);
}

void
min_max_date (
 long date)
{
	if (min_date == 0L)
		min_date = date;

	if (max_date == 0L)
		max_date = date;

	if (min_date > date)
		min_date = date;

	if (max_date < date)
		max_date = date;
}
