/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( inq_disp.c ) 	                                  |
|  Program Desc  : ( Quotaion Inquiry Display                     )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  qthr, cumr, exsf,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files : N/A  ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Elena Cuaresma  | Date Written  : 20/10/1995       |
|---------------------------------------------------------------------|
|  Date Modified : 28/12/1995      | Modified  by  : Elena B. Cuaresma|
|  Date Modified : 12/09/1997      | Modified  by  : Marnie I. Organo |
|  Date Modified : 20/08/1999      | Modified  by  : Alvin Misalucha  |
|  Date Modified : 28/12/1995      | Modified  by  : Elena B. Cuaresma|
|  Date Modified : 12/09/1997      | Modified  by  : Marnie I. Organo |
|                                                                     |
|  Comments      : 28/12/1995 - Updated to fix the bug in displaying  |
|                :            customers.                              |
|                : 12/09/1997 - Updated for Multilingual Conversion.  |
|                : 20/08/1999 - Converted to ANSI format.             |
|                : 12/09/1997 - Updated for Multilingual Conversion.  |
|                                                                     |
| $Log: inq_disp.c,v $
| Revision 5.2  2001/08/09 08:44:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:38:17  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:12:45  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:33:57  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:18:27  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:30  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:08:54  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  2000/03/03 01:08:40  cam
| Changes for GVision compatibility.  Added conditional code to prevent screen
| being cleared as part of drill down.
| Reorganized data entry to be more logical.
|
| Revision 1.15  1999/11/16 03:29:19  scott
| Updated for warning due to usage of -Wall flags on compiler.
|
| Revision 1.14  1999/09/29 10:12:31  scott
| Updated to be consistant on function names.
|
| Revision 1.13  1999/09/22 05:18:25  scott
| Updated from Ansi project.
|
| Revision 1.12  1999/09/14 04:05:32  scott
| Updated for Ansi.
|
| Revision 1.11  1999/09/13 09:21:24  alvin
| Converted to ANSI format.
|
| Revision 1.10  1999/06/18 06:12:25  scott
| Updated to add log for cvs and remove old style read_comm()
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: inq_disp.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QT/qt_inq_disp/inq_disp.c,v 5.2 2001/08/09 08:44:41 scott Exp $";

#define	X_OFF	20
#define	Y_OFF	3
#include	<std_decs.h>
#include	<pslscr.h>		
#include	<get_lpno.h>
#include 	<qt_status.h>
#include 	<ml_std_mess.h>
#include 	<ml_qt_mess.h>
#include	<dsp_utils.h>

	char	*data = "data",
			*comm = "comm",	
			*ccmr = "ccmr",	
			*comr = "comr",	
	    	*cumr = "cumr",	
	    	*qthr = "qthr",	
	    	*exsf = "exsf";	

	char	head_str[200],
			save_key[21];

	int		save_indx;

	char	branchNo[3];
	int		envDbCo = 0,
			envDbFind	 = 0;


	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list[] = {
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_cc_no"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 7;
	
	struct {
		int	termno;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		char	cc_no[3];
		long	t_dbt_date;
	} comm_rec;

	/*===========================================
	| Cost Centre/Warehouse Master File Record. |
	===========================================*/
	struct dbview ccmr_list[] ={
		{"ccmr_co_no"},
		{"ccmr_est_no"},
		{"ccmr_cc_no"},
		{"ccmr_hhcc_hash"},
		{"ccmr_stat_flag"}
	};

	int ccmr_no_fields = 5;

	struct {
		char	ccmr_co_no[3];
		char	ccmr_est_no[3];
		char	ccmr_cc_no[3];
		long	ccmr_hhcc_hash;
		char	ccmr_stat_flag[2];
	} ccmr_rec;

	/*==================================
	| Company Master File Base Record. |
	==================================*/
	struct dbview comr_list[] ={
		{"comr_co_no"},
		{"comr_co_name"},
		{"comr_co_adr1"},
		{"comr_co_adr2"},
		{"comr_co_adr3"},
		{"comr_stat_flag"},
		{"comr_frt_mweight"},
		{"comr_frt_min_amt"}
	};

	int comr_no_fields = 8;

	struct {
		char	co_no[3];
		char	co_name[41];
		char	co_adr[3][41];
		char	stat_flag[2];
		float	comr_frt_mweight;
		double	comr_frt_min_amt;
	} comr_rec;


	/*========================+
	 | Quotation header file. |
	 +========================*/
#define	QTHR_NO_FIELDS	10

	struct dbview	qthr_list [QTHR_NO_FIELDS] =
	{
		{"qthr_co_no"},
		{"qthr_br_no"},
		{"qthr_quote_no"},
		{"qthr_hhcu_hash"},
		{"qthr_enq_ref"},
		{"qthr_date_create"},
		{"qthr_dt_quote"},
		{"qthr_sman_code"},
		{"qthr_cont_name"},
		{"qthr_status"}
	};

	struct tag_qthrRecord
	{
		char	co_no [3];
		char	br_no [3];
		char	quote_no [9];
		long	hhcu_hash;
		char	enq_ref [21];
		long	date_create;
		long	dt_quote;
		char	sman_code [3];
		char	cont_name [21];
		char	status [3];
	}	qthr_rec;

	/*===================================
	| Customer Master File Base Record. |
	===================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_class_type"},
		{"cumr_curr_code"},
		{"cumr_price_type"},
		{"cumr_ch_adr1"},
		{"cumr_ch_adr2"},
		{"cumr_ch_adr3"},
		{"cumr_ch_adr4"},
		{"cumr_dl_adr1"},
		{"cumr_dl_adr2"},
		{"cumr_dl_adr3"},
		{"cumr_dl_adr4"},
		{"cumr_contact_name"},
		{"cumr_area_code"},
		{"cumr_sman_code"},
		{"cumr_disc_code"},
		{"cumr_tax_code"},
		{"cumr_tax_no"},
		{"cumr_inst_fg1"},
		{"cumr_inst_fg2"},
		{"cumr_inst_fg3"},
		{"cumr_stat_flag"},
		{"cumr_item_codes"},
		{"cumr_crd_prd"}
	};

	int cumr_no_fields = 30;

	struct {
		char	cm_co_no[3];
		char	cm_est_no[3];
		char	cm_dp_no[3];
		char	cm_dbt_no[7];
		long	cm_hhcu_hash;
		char	cm_name[41];
		char	cm_acronym[10];
		char	cm_class_type[4];
		char	cm_curr_code[4];
		char	cm_price_type[2];
		char	cm_ch_adr[4][41];
		char	cm_dl_adr[4][41];
		char	cm_cont_name[21];
		char	cm_area_code[3];
		char	cm_sman_code[3];
		char	cm_disc_code[2];
		char	cm_tax_code[2];
		char	cm_tax_no[16];
		int		cm_inst[3];
		char	cm_stat_flag[2];
		char	cm_item_codes[2];
		char	cm_crd_prd[4];
	} cumr_rec;

	/*=========================
	| External Salesman File. |
	=========================*/
	struct dbview exsf_list[] ={
		{"exsf_co_no"},
		{"exsf_salesman_no"},
		{"exsf_salesman"},
		{"exsf_stat_flag"}
	};

	int exsf_no_fields = 4;

	struct {
		char	co_no[3];
		char	no[3];
		char	desc[41];
		char	stat_flag[2];
	} exsf_rec;

/*============================ 
| Local & Screen Structures. |
============================*/

struct {
	char	dummy[11];
	char	quote_no[9];
	char	end_quote_no[9];
	char	cust_no[7];
	char	qt_no[9];
	char	dbt_no[7];
	char	cust_name[41];
	char	stat_desc[41];
	long	hhcu_hash;
	int	lpno;
} local_rec;


static struct	var vars[] ={

	{1,LIN,"n_quote",3,22, CHARTYPE,
		"UUUUUUUU","          ",
		" ","", "Quote Number    :","Enter Quotation Number.",
		NO,NO, JUSTLEFT,"","",(char *)&local_rec.quote_no},
	{1,LIN,"cust_no",4,22,CHARTYPE,
		"UUUUUU","          ",
		" ","", "Customer Number :","",
		NO,NO, JUSTLEFT,"","",local_rec.cust_no},
	{1,LIN,"cust_name",4,35,CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA","          ",
		" ","", "","",
		NA,NO, JUSTLEFT,"","",local_rec.cust_name},

	{0,LIN,"",0,0,INTTYPE,
		"A","          ",
		" ","", "dummy"," ",
		YES,NO, JUSTRIGHT," "," ",local_rec.dummy}
};

/*=========================== 
| Function prototypes       |
===========================*/
int		main			(int argc, char * argv []);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		(void);
int		spec_valid		(int);
void	show_qthr		(char *);
int		process			(void);
int		heading			(int);
int		inv_enquiry		(char *);
void	run_sdisplay	(char *, char *, char *);

#include 	<FindCumr.h> 

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{

	SETUP_SCR(vars);
	init_scr();			/*  sets terminal from termcap	*/
	set_tty();                      /*  get into raw mode		*/
	clear();
	swide();
	set_masks();			/*  setup print using masks	*/
	init_vars(1);			/*  set default values		*/

	envDbCo  = atoi(get_env("DB_CO"));
	envDbFind	  = atoi(get_env("DB_FIND"));

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	strcpy(branchNo, (!envDbCo) ? " 0" : comm_rec.test_no);

	while (prog_exit == 0)
	{
		FLD ("n_quote") = NO;
		FLD ("cust_no") = NO;
		search_ok = 1;
		entry_exit = 1;
		restart = 0;
		prog_exit = 0; 
		init_vars(1);

		heading(1);
		entry(1);
		
		strcpy(local_rec.qt_no, local_rec.quote_no);
		strcpy(local_rec.dbt_no, local_rec.cust_no);

		if (!restart)
		{
			if (strlen (clip (local_rec.qt_no)) == 0 && 
				strlen (clip (local_rec.dbt_no)) == 0) 
			{
				prog_exit = 1;
			}
			else
			{
				heading(1);
				scn_display(1);

				process();
			}
		}
	} 
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

void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec(qthr, qthr_list, QTHR_NO_FIELDS, "qthr_id_no2");
	open_rec(cumr, cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no"
							       						 : "cumr_id_no3");
	open_rec(exsf, exsf_list, exsf_no_fields, "exsf_id_no");
}

void
CloseDB (void)
{
	abc_fclose(qthr);
	abc_fclose(cumr);
	abc_fclose(exsf);
	abc_dbclose(data);
}

int
spec_valid (
 int	field)
{
	/*---------------------------------
	| Validate Purchase Order Number. |
	---------------------------------*/
	if (LCHECK("n_quote"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.quote_no, "ALL     "); 
			DSP_FLD ("n_quote");
			return (EXIT_SUCCESS);
		}

		FLD ("cust_no") = NA;

		if (SRCH_KEY)
		{
			show_qthr(temp_str);
			strcpy (local_rec.quote_no, qthr_rec.quote_no);
			DSP_FLD ("n_quote");
			return(0);
		}

		strcpy (qthr_rec.quote_no, zero_pad(local_rec.quote_no,8));
		strcpy (local_rec.quote_no, qthr_rec.quote_no); 
		DSP_FLD ("n_quote");

		/*----------------------------
		| Check if order is on file. |
		----------------------------*/
		abc_selfield(qthr,"qthr_id_no2");
		strcpy(qthr_rec.co_no, comm_rec.tco_no);
		strcpy(qthr_rec.br_no, branchNo); 
		if ( find_rec(qthr, &qthr_rec, EQUAL, "r"))
		{
			print_mess(ML(mlStdMess210));
			sleep(2);
			clear_mess();
			return(1);
		}
		return(0);
	}

	/*--------------------
	| Validate Customer no |
	--------------------*/
	if (LCHECK("cust_no")) 
	{
		if (dflt_used || FLD ("cust_no") == NA)
		{
			strcpy (local_rec.cust_no, "ALL   ");
			strcpy (local_rec.cust_name, "Customers"); 
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
			return (EXIT_SUCCESS);
		} 

		FLD ("n_quote") = NA;
		if (!strncmp(local_rec.cust_no, "99998",5))
		{
			DSP_FLD("cust_no");
			return(0);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNo, temp_str);
			return(0);
		}

		strcpy(cumr_rec.cm_co_no,  comm_rec.tco_no);
		strcpy(cumr_rec.cm_est_no, branchNo);
		strcpy(cumr_rec.cm_dbt_no, pad_num(local_rec.cust_no));
		cc = find_rec(cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess(ML(mlStdMess021));
			sleep(2);
			clear_mess();
			return(1);
		}
		strcpy (local_rec.cust_name, cumr_rec.cm_name);
		DSP_FLD("cust_no");
		DSP_FLD("cust_name");
		return(0);
	}
	return(0);
}

/*==========================
| Search for order number. |
==========================*/
void
show_qthr (
 char *	key_val)
{
	char	wk_mask[9];

	work_open();
	save_rec("#Qt No.","#Quote Date| Contact");
	abc_selfield(qthr,"qthr_id_no2");
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, branchNo); 
	sprintf(qthr_rec.quote_no,"%-8.8s",key_val);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strncmp(qthr_rec.quote_no,key_val,strlen(key_val)) && 
		   !strcmp(qthr_rec.co_no, comm_rec.tco_no) && 
		   !strcmp(qthr_rec.br_no, branchNo))
	{
		sprintf(err_str, " %s |%s", DateToString(qthr_rec.dt_quote),
						qthr_rec.cont_name);

		sprintf(wk_mask, "%-8.8s", qthr_rec.quote_no);
		cc = save_rec(wk_mask,err_str);
		if (cc)
				break;

		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
	}
	cc = disp_srch();
	work_close();
	if (cc)
		return;

	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, branchNo);
	sprintf(qthr_rec.quote_no, "%-8.8s", temp_str);
	cc = find_rec(qthr, &qthr_rec, COMPARISON, "r");
	if (cc)
	{
		file_err(cc, qthr, "DBFIND");
	}
	abc_selfield(qthr,"qthr_id_no");
}

int
process (void)
{
	int	i;
	char	tem_line[200];

	save_indx = 0;
	Dsp_prn_open (1 , 6, 12, head_str, comm_rec.tco_no, comm_rec.tco_name, comm_rec.test_no, comm_rec.test_name, (char *) 0, (char *) 0);

	Dsp_saverec(" QUOTE NO. | ENQ. DATE |      REFERENCE      |                SALESMAN                  |                  STATUS                ");
	Dsp_saverec(""); 
	Dsp_saverec("[NEXT] [PREVIOUS] [INPUT/END]");  

	if (!strncmp (local_rec.quote_no, "ALL", 3))
		strcpy (local_rec.quote_no, "        ");

	if (!strncmp (local_rec.cust_no, "ALL",3))
		strcpy (local_rec.cust_no, "      ");
	else
		abc_selfield(cumr, "cumr_hhcu_hash");

	memset (&qthr_rec, 0, sizeof (qthr_rec));
	strcpy(qthr_rec.co_no, comm_rec.tco_no);
	strcpy(qthr_rec.br_no, branchNo); 
	strcpy(qthr_rec.quote_no, local_rec.quote_no);
	cc = find_rec(qthr, &qthr_rec, GTEQ, "r");
	while (!cc && !strcmp(qthr_rec.co_no, comm_rec.tco_no))
	{
		if (strcmp(local_rec.quote_no, "        ") &&
			strcmp (local_rec.quote_no, qthr_rec.quote_no))
		{
			cc = find_rec(qthr, &qthr_rec, NEXT, "r");
			continue;
		}
		if (strcmp(local_rec.cust_no, "      "))
		{
			if (qthr_rec.hhcu_hash != 0L)
			{
				cumr_rec.cm_hhcu_hash = qthr_rec.hhcu_hash;
				cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
				if (cc || strcmp(cumr_rec.cm_dbt_no, local_rec.cust_no))
				{
					cc = find_rec (qthr, &qthr_rec, NEXT, "r");
					continue;
				}
			}
			else
			{
				if (strcmp(local_rec.cust_no, "99998 "))
				{
					cc = find_rec (qthr, &qthr_rec, NEXT, "r");
					continue;
				}
			}
		}
	
		strcpy (exsf_rec.co_no, qthr_rec.co_no);
		strcpy (exsf_rec.no, qthr_rec.sman_code);
		cc = find_rec(exsf, &exsf_rec, COMPARISON, "r");
		
		for (i = 0;strlen(q_status[i]._stat);i++)
		{
			if (!strncmp(qthr_rec.status, q_status[i]._stat,strlen(q_status[i]._stat)))
			{
				sprintf(local_rec.stat_desc,"%-40.40s",q_status[i]._desc);
				break;
			}
		}
		sprintf(tem_line,"  %s ^E%-10.10s ^E %s^E %s ^E %s^E",
			qthr_rec.quote_no,
			DateToString(qthr_rec.date_create),
			qthr_rec.enq_ref,
			exsf_rec.desc,
			local_rec.stat_desc);

		sprintf (save_key, "%04d%-2.2s%-8.8s",
				save_indx++, 
				qthr_rec.br_no,
				qthr_rec.quote_no);

		Dsp_save_fn(tem_line, save_key); 

		cc = find_rec(qthr, &qthr_rec, NEXT, "r");
		continue;
	}
	Dsp_srch_fn(inv_enquiry);
	Dsp_close();
	abc_selfield(cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
	return(0);
}

/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set(scn);
		clear();
	
		move(0,1);
		line(132);

		rv_pr(ML(mlQtMess006),54,0,1);

		box(0,2,132,2);

		line_cnt = 0;
		scn_write(scn);
	}
	return (EXIT_SUCCESS);
}

int
inv_enquiry (
 char *	find_key)
{
	char	_br_no[3];
	char	_quote_no[9];

	sprintf (_br_no, "%-2.2s", find_key+4);
	sprintf (_quote_no, "%-8.8s", find_key+6);
	run_sdisplay ( comm_rec.tco_no, 
					_br_no,
	       	        _quote_no); 
	return (EXIT_SUCCESS);
}

/*=====================================================
| Execute invoice display passing relevent arguments. |
=====================================================*/
void
run_sdisplay (
 char *	_co_no,
 char *	_br_no,
 char *	_quote_no)
{
	int		indx = 1;
	char	run_string [100];
	char	run_string1 [100];

 	sprintf( run_string1, "qt_quote.s");

 	sprintf( run_string, "%-2.2s %-2.2s %-8.8s",
		_co_no, _br_no, _quote_no); 

	box(50,21,40,1);
	/*rv_pr( " PLEASE WAIT, ...... Loading Enquiry. ", 51,22,1); */
	rv_pr(ML(mlStdMess035), 51,22,1); 

	arg[0]      = "qt_quote";
	arg[1] 		= run_string1;
	arg[++indx] = run_string;
	arg[++indx] = (char *)0;

	shell_prog(2); 
#ifndef GVISION
	clear();
	swide();
#endif	/* GVISION */

	if (!strcmp(local_rec.cust_no, "      "))
		strcpy (local_rec.cust_no, "ALL   ");

	if (!strcmp(local_rec.quote_no, "        "))
		strcpy (local_rec.quote_no, "ALL     ");

#ifndef GVISION
	heading(1);
	scn_display(1); 
	Dsp_heading();  
#endif	/* GVISION */
}
