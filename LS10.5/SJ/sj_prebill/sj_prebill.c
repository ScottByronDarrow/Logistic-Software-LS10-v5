/*====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sj_prebill.c,v 5.3 2002/07/17 09:57:50 scott Exp $
|  Program Name  : (sj_prebill.c                                     |
|  Program Desc  : (Service job prebill invoicing            )   |
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 08/09/87         |
|---------------------------------------------------------------------|
| $Log: sj_prebill.c,v $
| Revision 5.3  2002/07/17 09:57:50  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:43  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:41  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_prebill.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_prebill/sj_prebill.c,v 5.3 2002/07/17 09:57:50 scott Exp $";

#define MAXWIDTH	150

#include <pslscr.h>
#include <GlUtils.h>
#include <get_lpno.h>
#include <ml_std_mess.h>
#include <ml_sj_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct sjhrRecord	sjhr_rec;
struct sjjdRecord	sjjd_rec;
struct cumrRecord	cumr_rec;
struct esmrRecord	esmr_rec;
struct sjisRecord	sjis_rec;
struct sjisRecord	sjis2_rec;
struct sjigRecord	sjig_rec;

char	*sjis2	=	"sjis2";
/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	c_client [7];
	char	e_client [7];
	char	chg_client [41];
	char	end_client [41];
	char	prtr_no [3];
	char	stat_desc [21];
	char	gl_acc_no [17];
	char	acc_desc [26];
	char 	job_detail [7][71];
	double	gl_amt;
	double	invoice_chg;
	int		printerNo;
} local_rec;

int	envDbCo = 0,
	envDbFind  = 0;

char	branchNo [3],
	systemDate [11];

long	new_hhcu_hash = 0L;

static	struct	var	vars []	={	

	{1, LIN, "service_no", 4, 20, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "Service Job #", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.order_no}, 
	{1, LIN, "curr_stat", 4, 50, CHARTYPE, 
		"U", "          ", 
		" ", "", "Status ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.status}, 
	{1, LIN, "Stat desc", 4, 82, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.stat_desc}, 
	{1, LIN, "charge_to", 5, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Charge To ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.c_client}, 
	{1, LIN, "charge_to_name", 5, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.chg_client}, 
	{1, LIN, "end_client", 6, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "End Client ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.e_client}, 
	{1, LIN, "end_client_name", 6, 50, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.end_client}, 
	{1, LIN, "cust_order_no", 7, 20, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "", "Cust Order No ", " ", 
		NA, NO, JUSTLEFT, "", "", sjhr_rec.cust_ord_no}, 
	{1, LIN, "invamt", 7, 82, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Amt Invoiced ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.prebill_amt}, 
	{1, LIN, "comp_date", 8, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Completion Date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.reqd_date}, 
	{1, LIN, "prebill", 8, 82, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Amt This Inv ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.invoice_chg}, 
	{1, LIN, "cost_est", 9, 20, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Cost Estimate ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.cost_estim}, 
	{1, LIN, "inv_date", 9, 82, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Invoice Date ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjis_rec.date}, 
	{1, LIN, "printerNo", 10, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No ", " ", 
		YES, NO, JUSTLEFT, "123456789", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "desc", 12, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [0]}, 
	{1, LIN, "desc1", 13, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [1]}, 
	{1, LIN, "desc2", 14, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [2]}, 
	{1, LIN, "desc3", 15, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [3]}, 
	{1, LIN, "desc4", 16, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [4]}, 
	{1, LIN, "desc5", 17, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [5]}, 
	{1, LIN, "desc6", 18, 20, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "            - ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.job_detail [6]}, 
	{2, TAB, "gl_acct", MAXLINES, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAA", "          ", 
		"0", "", " G/L Account ", " ", 
		YES, NO, JUSTLEFT, "1234567890", "", local_rec.gl_acc_no}, 
	{2, TAB, "gl_desc", 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		"0", "", "  G/L Account Description  ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.acc_desc}, 
	{2, TAB, "amount", 0, 0, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", " G/L Amount ", " ", 
		YES, NO, JUSTRIGHT, "-99999999", "99999999", (char *)&local_rec.gl_amt}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};


#include <FindCumr.h>
/*=======================
| Function Declarations |
========================*/
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
int 	InsertLine 		(void);
int 	DeleteLine 		(void);
void 	LoadDetails 	(void);
int 	GetChargeClient (void);
int 	GetEndClient 	(void);
void 	GetStatusDesc 	(void);
void 	Update 			(void);
void 	UpdateSjig 		(void);
int 	GlTotalError 	(void);
void 	SrchSjhr 		(char *);
int 	heading 		(int);
int 	CheckSjis 		(long);
int 	CheckClass 		(char *);


/*=========================
| Main Processing Routine |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	tab_col = 22 ;

	init_scr ();
	set_tty (); 
	set_masks ();
	init_vars (1);

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind 	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	GL_SetMask (GlFormat);

	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);
	strcpy (systemDate, DateToString (TodaysDate ()));

	swide ();

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*--------------------------------
		| Enter screen 2 tabular input . |	
		--------------------------------*/
		heading (2);
		entry (2);
		if (restart)
			continue;

		do
		{
			edit_all ();
			if (restart)
				break;
		} while (GlTotalError ()) ;

		if (!restart)
			Update ();

	}	/* end of input control loop	*/
	shutdown_prog ();
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

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (sjhr, sjhr_list, SJHR_NO_FIELDS, "sjhr_id_no");
	open_rec (sjjd, sjjd_list, SJJD_NO_FIELDS, "sjjd_id_no");
	open_rec (sjig, sjig_list, SJIG_NO_FIELDS, "sjig_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	abc_alias (sjis2,sjis);
	open_rec (sjis, sjis_list, SJIS_NO_FIELDS, "sjis_id_no");
	open_rec (sjis2, sjis_list, SJIS_NO_FIELDS, "sjis_id_no");
	OpenGlmr ();
	
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (sjhr);
	abc_fclose (cumr);
	abc_fclose (sjjd);
	abc_fclose (sjis);
	abc_fclose (sjis2);
	abc_fclose (sjig);
	abc_fclose (esmr);
	GL_Close ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
	if (LCHECK ("service_no"))
	{
		if (SRCH_KEY)
		{
			SrchSjhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sjhr_rec.co_no,comm_rec.co_no);
		strcpy (sjhr_rec.est_no,comm_rec.est_no);
		strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
		cc = find_rec (sjhr,&sjhr_rec,COMPARISON,"r");
		if (cc)
		{
			errmess (ML (mlSjMess004));
			return (EXIT_FAILURE);
		}

		if (strcmp (sjhr_rec.status,"O"))
		{
			errmess (ML (mlSjMess005));
			return (EXIT_FAILURE);
		}
		abc_selfield (cumr,"cumr_hhcu_hash");
		GetChargeClient ();
		GetEndClient ();
		abc_selfield (cumr, (envDbFind == 0) ? "cumr_id_no" : "cumr_id_no3");
		GetStatusDesc ();

		LoadDetails ();

		return (EXIT_SUCCESS);
	}

	/*=========================================
	| Validate General Ledger Account Number. |
	=========================================*/
	if (LCHECK ("gl_acct"))
	{
		if (last_char == DELLINE)
			return (DeleteLine ());

		if (last_char == INSLINE)
			return (InsertLine ());

		/*-------------------------------
		| First character is a '\'	|
		| \D	- delete current line	|
		| \I	- insert before current	|
		-------------------------------*/
		if (local_rec.gl_acc_no [0] == '\\')
		{
			switch (local_rec.gl_acc_no [1])
			{
			case	'D':
			case	'd':
				return (DeleteLine ());
				break;
	
			case	'I':
			case	'i':
				return (InsertLine ());
				break;
	
			default:
				break;
			}
		}
		if (SRCH_KEY) 
			return SearchGlmr (comm_rec.co_no, temp_str, "F*P");

		strcpy (glmrRec.co_no,comm_rec.co_no);
		strcpy (glmrRec.acc_no,local_rec.gl_acc_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);		
			return (EXIT_FAILURE);
		}
			
		if (CheckClass ("G/L"))
			return (EXIT_FAILURE);

		strcpy (local_rec.acc_desc, glmrRec.desc);
		display_field (label ("gl_desc"));
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("end_client"))
	{
		if (vars [label ("end_client")].required == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNo);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.e_client));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.end_client,cumr_rec.dbt_name);
		new_hhcu_hash = cumr_rec.hhcu_hash;
		display_field (label ("end_client_name"));
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			errmess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============
| Insert line. |
==============*/
int
InsertLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("gl_acct")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	for (i = line_cnt,line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	sprintf (local_rec.gl_acc_no,"%-16.16s"," ");
	sprintf (local_rec.acc_desc,"%-25.25s"," ");
	local_rec.gl_amt = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*==============
| Delete line. |
==============*/
int 
DeleteLine (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	sprintf (local_rec.gl_acc_no,"%-16.16s"," ");
	sprintf (local_rec.acc_desc,"%-25.25s"," ");
	local_rec.gl_amt = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
LoadDetails (
 void)
{
	int	i = 0;

	scn_set (1);

	/*===================
	|  read job details |
	===================*/
	strcpy (sjjd_rec.co_no,comm_rec.co_no);
	strcpy (sjjd_rec.est_no,comm_rec.est_no);
	strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
	sjjd_rec.order_no = sjhr_rec.order_no;
	sjjd_rec.line_no = 0;
	cc = find_rec (sjjd,&sjjd_rec,GTEQ,"r");

	while (!cc && !strcmp (sjjd_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sjjd_rec.est_no,comm_rec.est_no) && 
		      !strcmp (sjjd_rec.dp_no,comm_rec.dp_no) && 
		      sjjd_rec.order_no == sjhr_rec.order_no)
	{
		/*=======================
		| get description lines |
		========================*/
		strcpy (local_rec.job_detail [i++],sjjd_rec.detail);
		cc = find_rec (sjjd,&sjjd_rec,NEXT,"r");
	}
	scn_display (1);
}

int
GetChargeClient (
 void)
{
	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sjhr_rec.chg_client);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.c_client,cumr_rec.dbt_no);
	strcpy (local_rec.chg_client,cumr_rec.dbt_name);
	if (strcmp (cumr_rec.class_type,"INT") !=  0)
	{
		strcpy (sjhr_rec.cust_type,"E");
		vars [label ("end_client")].required = NA;
	}
	else
	{
		strcpy (sjhr_rec.cust_type ,"I");
		vars [label ("end_client")].required = YES;
	}
	return (EXIT_SUCCESS);
}

int
GetEndClient (
 void)
{
	if (sjhr_rec.end_client == 0L)
		return (EXIT_SUCCESS);

	cc = find_hash (cumr,&cumr_rec,COMPARISON,"r",sjhr_rec.end_client);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.e_client,cumr_rec.dbt_no);
	strcpy (local_rec.end_client,cumr_rec.dbt_name);
	new_hhcu_hash = cumr_rec.hhcu_hash;
	return (EXIT_SUCCESS);
}

void
GetStatusDesc (
 void)
{
	switch (sjhr_rec.status [0]) 
	{ 
        	case 'O':
			strcpy (local_rec.stat_desc,"Open Service Job");
			break;
        	case 'C':
			strcpy (local_rec.stat_desc,"Closed Service Job");
			break;
        	case 'H':
			strcpy (local_rec.stat_desc,"Service Job On Hold");
			break;
        	case 'I':
			strcpy (local_rec.stat_desc,"Service Job Invoiced");
			break;
	}
}

void
Update (
 void)
{
	print_at (2,40,ML (mlSjMess006));
	fflush (stdout);

	/*=====================
        |   update job header |
	=====================*/
	sjhr_rec.prebill_amt += local_rec.invoice_chg;
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.end_client = new_hhcu_hash;
        cc = abc_update (sjhr,&sjhr_rec);
        if (cc)
        	sys_err ("Error in sjhr During (DBUPDATE)",cc,PNAME);

	/*==========================
        |   update invoice summary |
	==========================*/
	strcpy (sjis_rec.co_no,comm_rec.co_no);
	strcpy (sjis_rec.est_no,comm_rec.est_no);
	strcpy (sjis_rec.dp_no,comm_rec.dp_no);
	sjis_rec.order_no = sjhr_rec.order_no;
	sjis_rec.chg_client = sjhr_rec.chg_client;
	sjis_rec.end_client = new_hhcu_hash;
	strcpy (sjis_rec.cust_ord_no,sjhr_rec.cust_ord_no);
	strcpy (sjis_rec.type,"P");
	strcpy (sjis_rec.status,"G");
	strcpy (sjis_rec.prt_stat,"G");
	sjis_rec.cost_estim = 0;
	sjis_rec.invoice_cost = 0;
	sjis_rec.gst_pc = comm_rec.gst_rate;
	sjis_rec.prebill_amt = sjhr_rec.prebill_amt - sjis_rec.invoice_chg;

	/*=========================
	|   assign invoice number |
	==========================*/
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
	if (cc)
		sys_err ("Error in esmr during (DBFIND)",cc,PNAME);

	/*---------------------------------------
	| Check if Inv.  No Already Allocated	|
	| If it has been then skip		|
	---------------------------------------*/
	while (CheckSjis (++esmr_rec.nx_inv_no) == 0);

	cc = abc_update (esmr,&esmr_rec);
	if (cc)
		sys_err ("Error in esmr during (DBUPDATE)",cc,PNAME);

	sprintf (sjis_rec.invno,"%08ld",esmr_rec.nx_inv_no);

	print_at (2,40,ML (mlSjMess021),sjis_rec.invno);

	cc = abc_add (sjis,&sjis_rec);
	if (cc)
		sys_err ("Error in sjis During (DBADD)",cc,PNAME);

	abc_unlock (esmr);
	UpdateSjig ();

	sprintf (local_rec.prtr_no,"%2d",local_rec.printerNo);

	clear ();
	print_at (0,0,ML (mlSjMess007));
	fflush (stdout);

	* (arg) = "sj_pbcr_prt";
	* (arg+ (1)) = local_rec.prtr_no;
	* (arg+ (2)) = sjis_rec.invno;
	* (arg+ (3)) = (char *)0;
	shell_prog (3);
}

void
UpdateSjig (
 void)
{
	int i = 0;

	/*===================================
	| Write out sjig records for invoice |
	====================================*/
	strcpy (sjig_rec.co_no,comm_rec.co_no);
	strcpy (sjig_rec.est_no,comm_rec.est_no);
	strcpy (sjig_rec.dp_no,comm_rec.dp_no);
	strcpy (sjig_rec.invno,sjis_rec.invno);
	sjig_rec.order_no = sjis_rec.order_no;
	scn_set (2);
	for (i = 0; i < lcount [2] ; i++)
	{
		getval (i);
		sjig_rec.amount = local_rec.gl_amt;
		strcpy (sjig_rec.acc_no,local_rec.gl_acc_no);
		cc = abc_add (sjig,&sjig_rec);
		if (cc)
			sys_err ("Error in sjig During (DBADD)",cc,PNAME);
	}
	scn_set (1);
}

int
GlTotalError (
 void)
{
	/*==========================================
	| Check that GL allocation = invoice amount |
	===========================================*/
	double	gl_chk_tot = 0.0,
		gl_tot_err = 0.0;
	int	i;

	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);
		gl_chk_tot += local_rec.gl_amt;
	}
	gl_tot_err = local_rec.invoice_chg - gl_chk_tot;

	if (cur_screen != 2)
		scn_set (1);

	if (gl_tot_err == 0.00)
		return (EXIT_SUCCESS);

	print_at (2,40,ML (mlSjMess008),gl_chk_tot,gl_tot_err);
	fflush (stdout);
	return (EXIT_FAILURE); 
}

/*=========================================
| Search routine for Service Header File. |
=========================================*/
void
SrchSjhr (
 char *key_val)
{
	char	order_no [7];

	work_open ();
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.order_no = atol (key_val);
	save_rec ("#Service Job","#Issue On");
	cc = find_rec (sjhr, &sjhr_rec, GTEQ, "r");
	while (!cc && !strcmp (sjhr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (sjhr_rec.est_no,comm_rec.est_no) && 
		      !strcmp (sjhr_rec.dp_no,comm_rec.dp_no))
	{
		sprintf (order_no,"%6ld",sjhr_rec.order_no);
		if ((strlen (key_val) == 0 || 
		      !strncmp (order_no,key_val,strlen (key_val))) && 
		      sjhr_rec.status [0] == 'O')
		{
			strcpy (err_str, DateToString (sjhr_rec.issue_date));
			cc = save_rec (order_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (sjhr, &sjhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.order_no = atol (temp_str);
	cc = find_rec (sjhr, &sjhr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in sjhr During (DBFIND)",cc,PNAME);
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
		rv_pr (ML (mlSjMess009),40,0,1);
		move (0,1);
		line (132);

		if (scn == 1)
		{
			box (0,3,132,15);
			move (1,11);
			line (131);
		}
		else
		{
		  	print_at (3,0,ML (mlSjMess010),local_rec.invoice_chg);
		}
		move (0,20);
		line (132);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no,comm_rec.est_name);
		move (0,23);
		line (132);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

int
CheckSjis (
 long inv_no)
{
	strcpy (sjis2_rec.co_no,comm_rec.co_no);
	strcpy (sjis2_rec.est_no,comm_rec.est_no);
	sprintf (sjis2_rec.invno,"%08ld",inv_no);
	cc = find_rec (sjis2,&sjis2_rec,COMPARISON,"r");
	return (cc);
}

int	
CheckClass (
 char	*msg)
{
	if (glmrRec.glmr_class [2][0] != 'P')
	{
		errmess (ML (mlStdMess025));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	
	return (EXIT_SUCCESS);
}

