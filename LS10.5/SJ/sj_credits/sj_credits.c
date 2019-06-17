/*====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sj_credits.c,v 5.5 2002/07/19 05:07:35 scott Exp $
|  Program Name  : (sj_credits.c)
|  Program Desc  : (Service job credits entry)
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 08/09/87         |
|---------------------------------------------------------------------|
| $Log: sj_credits.c,v $
| Revision 5.5  2002/07/19 05:07:35  scott
| .
|
| Revision 5.4  2002/07/19 05:01:25  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/07/17 09:57:48  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:17:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:41:14  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_credits.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_credits/sj_credits.c,v 5.5 2002/07/19 05:07:35 scott Exp $";

/*================================
|   Include file dependencies   |
================================*/
#define MAXWIDTH	150

#include <pslscr.h>
#include <GlUtils.h>
#include <get_lpno.h>
#include <ml_sj_mess.h>
#include <ml_std_mess.h>

/*===================================
|   Constants, defines and stuff    |
===================================*/

    /*=====================
    |   Local variables   |
    =====================*/
	int		envDbCo = 0,
			envDbFind = 0;

	char	systemDate [11],
			branchNo [3];

	long	new_hhcu_hash = 0L;

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
struct 
{
	char	dummy [11];
	char	c_client [7];
	char	e_client [7];
	char	chg_client [41];
	char	end_client [41];
	char	order_no [11];
	int	    lp_no;
	char	stat_desc [21];
	char 	acc_desc [26];
	char	gl_acc_no [MAXLEVEL + 1];
	char	job_detail [7][71];
	double	gl_amt;
	double	invoice_chg;
} local_rec;

static struct var vars [] = 
{	
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
	{1, LIN, "comp_date", 8, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Completion Date ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.reqd_date}, 
	{1, LIN, "cost_est", 9, 20, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Cost Estimate ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.cost_estim}, 
	{1, LIN, "invamt", 7, 82, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", "0", "Amt Invoiced ", " ", 
		NA, NO, JUSTRIGHT, "", "", (char *)&sjhr_rec.inv_amt}, 
	{1, LIN, "prebill", 8, 82, DOUBLETYPE, 
		"NNNNNNNN.NN", "          ", 
		" ", (char *)&sjhr_rec.inv_amt, "Credit Amount ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.invoice_chg}, 
	{1, LIN, "inv_date", 9, 82, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Credit Date ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&sjis_rec.date}, 
	{1, LIN, "lp_no", 10, 20, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer No ", " ", 
		YES, NO, JUSTLEFT, "123456789", "", (char *)&local_rec.lp_no}, 
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
		GlMask, "          ", 
		"0", "", GlDesc, " ", 
		YES, NO, JUSTLEFT, "1234567890-*", "", local_rec.gl_acc_no}, 
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

/*===============================
|   local function prototypes   |
===============================*/
void 	shutdown_prog 		(void);
void	OpenDB 				(void);
void 	CloseDB 			(void);
int  	spec_valid 			(int);
int  	InsertLine 			(void);
int  	DeleteLine 			(void);
void 	LoadOrders 			(void);
int  	GetChargeClient 	(void);
int  	GetEndClient 		(void);
void 	GetStatusDesc 		(void);
void 	update 				(void);
void 	SrchSjhr 			(char *);
void 	UpdateSjig 			(void);
int  	GlTotalError 		(void);
int  	heading 			(int);
int  	CheckSjis 			(long);
int  	GlCheckClass 		(void);

#include <FindCumr.h>    

/*=========================
| Main Processing Routine |
==========================*/
int 
main (
 int argc,
 char *argv [])
{
	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	tab_col = 22; 

	init_scr ();
	set_tty (); 
	set_masks ();	

	init_vars (1);

    envDbCo 	= atoi (get_env ("DB_CO"));
    envDbFind 	= atoi (get_env ("DB_FIND"));

    OpenDB ();

	GL_SetMask (GlFormat);
	strcpy (systemDate, DateToString (TodaysDate ()));


	strcpy (branchNo, (!envDbCo) ? " 0" : comm_rec.est_no);

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
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |	
		-------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
        {
			continue;
        }

		/*--------------------------------
		| Enter screen 2 tabular input . |	
		--------------------------------*/
		heading (2);
		entry (2);
		if (restart)
        {
			continue;
        }

		do
		{
			edit_all ();
			if (restart)
            {
				break;
            }
		} while (GlTotalError ()) ;

		if (!restart)
        {
			update ();
        }
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
			/*------------------------
			| Service job Not found. |
			------------------------*/
			errmess (ML (mlSjMess004));
			return (EXIT_FAILURE);
		}

		if (strcmp (sjhr_rec.status,"I"))
		{
			/*------------------------------------------------------
			|Service Job has not been invoiced, credit not allowed |
			------------------------------------------------------*/
			errmess (ML (mlSjMess037));
			return (EXIT_FAILURE);
		}
		abc_selfield (cumr,"cumr_hhcu_hash");
		GetChargeClient ();
		GetEndClient ();
		abc_selfield (cumr, (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
		GetStatusDesc ();

		LoadOrders ();
		return (EXIT_SUCCESS);
	}

	/*=========================================
	| Validate General Ledger Account Number. |
	=========================================*/
	if (LCHECK ("gl_acct"))
	{
		if (last_char == DELLINE)
        {
			return (DeleteLine ());
        }

		if (last_char == INSLINE)
        {
			return (InsertLine ());
        }

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
        {
			return (SearchGlmr (comm_rec.co_no, temp_str, "F*P"));
        }

		strcpy (glmrRec.co_no,comm_rec.co_no);
		sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL,MAXLEVEL,local_rec.gl_acc_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc) 
		{
			/*-------------------------
			| G/L Account not found. |
			-------------------------*/
			errmess (ML (mlStdMess024));
			sleep (sleepTime);		
			return (EXIT_FAILURE);
		}
			
		if (GlCheckClass ())
        {
			return (EXIT_FAILURE);
        }

		strcpy (local_rec.acc_desc, glmrRec.desc);
		DSP_FLD ("gl_desc");
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("end_client"))
	{
		if (vars [label ("end_client")].required == NA)
        {
			return (EXIT_SUCCESS);
        }

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
			/*---------------------
			| Customer Not found. |
			---------------------*/
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.end_client,cumr_rec.dbt_name);
		new_hhcu_hash = cumr_rec.hhcu_hash;
		DSP_FLD ("end_client_name");
	}

	if (LCHECK ("lp_no"))
	{
		if (SRCH_KEY)
		{
			local_rec.lp_no = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lp_no))
		{
			/*-------------------
			| Invalid printer. |
			-------------------*/
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
		/*------------------------------
		| Cannot Insert Lines On Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("gl_acct")].row)
	{
		/*------------------------------------
		| Cannot Insert Line - Table is Full |
		------------------------------------*/
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	for (i = line_cnt,line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
        {
			line_display ();
        }
	}
	lcount [2]++;
	line_cnt = i;

	sprintf (local_rec.gl_acc_no,"%*.*s",MAXLEVEL,MAXLEVEL," ");
	sprintf (local_rec.acc_desc,"%-25.25s"," ");
	local_rec.gl_amt = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
    {
		blank_display ();
    }

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
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
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
        {
			line_display ();
        }
	}

	sprintf (local_rec.gl_acc_no,"%*.*s",MAXLEVEL,MAXLEVEL," ");
	sprintf (local_rec.acc_desc,"%-25.25s"," ");
	local_rec.gl_amt = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
    {
		blank_display ();
    }
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
LoadOrders (
 void)
{
	int	i = 0;

	/*===================
	|  read job details |
	===================*/
	scn_set (1);

	print_at (2,1,"%s\n\r", ML (mlStdMess035));
	fflush (stdout);

	strcpy (sjjd_rec.co_no,comm_rec.co_no);
	strcpy (sjjd_rec.est_no,comm_rec.est_no);
	strcpy (sjjd_rec.dp_no,comm_rec.dp_no);
	sjjd_rec.order_no = sjhr_rec.order_no;
	sjjd_rec.line_no = 0;

	cc = find_rec (sjjd, &sjjd_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (sjjd_rec.co_no,comm_rec.co_no) && 
           !strcmp (sjjd_rec.est_no,comm_rec.est_no) && 
           !strcmp (sjjd_rec.dp_no,comm_rec.dp_no) && 
          (sjjd_rec.order_no == sjhr_rec.order_no))
	{
		strcpy (local_rec.job_detail [i++], sjjd_rec.detail);
		cc = find_rec (sjjd, &sjjd_rec, NEXT, "r");
	}

	scn_display (1);
}

int
GetChargeClient (
 void)
{
	cc = find_hash (cumr, 
                    &cumr_rec, 
                    COMPARISON,
                    "r",
                    sjhr_rec.chg_client);
	if (cc)
	{
		/*---------------------
		| Customer not found. |
		----------------------*/
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
    {
		return (EXIT_SUCCESS);
    }

	cc = find_hash (cumr, 
                    &cumr_rec,
                    COMPARISON, 
                    "r", 
                    sjhr_rec.end_client);
	if (cc)
	{
		errmess (ML (mlStdMess021));
		return (EXIT_FAILURE);
	}

	new_hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (local_rec.e_client, cumr_rec.dbt_no);
	strcpy (local_rec.end_client, cumr_rec.dbt_name);
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
update (
 void)
{
	char	lp_str [3];

	print_at (2,40,"%s\n", ML (mlStdMess035));

    /*=====================
    |   update job header |
	=====================*/
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);

	sjhr_rec.end_client = new_hhcu_hash;
	sjhr_rec.inv_amt -= local_rec.invoice_chg;
    cc = abc_update (sjhr,&sjhr_rec);
    if (cc)
    {
        sys_err ("Error in sjhr During (DBUPDATE)",cc,PNAME);
    }
	
	abc_unlock (sjhr);

	/*==========================
	|   update invoice summary |
	==========================*/
	strcpy (sjis_rec.co_no,comm_rec.co_no);
	strcpy (sjis_rec.est_no,comm_rec.est_no);
	strcpy (sjis_rec.dp_no,comm_rec.dp_no);

	sjis_rec.order_no 		= sjhr_rec.order_no;
	sjis_rec.chg_client 		= sjhr_rec.chg_client;
	sjis_rec.end_client 		= new_hhcu_hash;
	strcpy (sjis_rec.cust_ord_no,sjhr_rec.cust_ord_no);
	sjis_rec.cost_estim 		= 0.00;
	sjis_rec.invoice_cost 	= 0.00;
	sjis_rec.invoice_chg 	= 0.00 - local_rec.invoice_chg;
	sjis_rec.prebill_amt 	= 0.00;
	strcpy (sjis_rec.type,"C");
	strcpy (sjis_rec.status,"G");
	strcpy (sjis_rec.prt_stat,"G");
	sjis_rec.gst_pc = comm_rec.gst_rate;

	/*========================
	|   assign credit number |
	========================*/
	strcpy (esmr_rec.co_no,comm_rec.co_no);
	strcpy (esmr_rec.est_no,comm_rec.est_no);

	cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
	if (cc)
    {
		sys_err ("Error in esmr during (DBFIND)",cc,PNAME);
    }

	/*-----------------------------------------------
	| Check if Crd.Note No. already allocated	|
	| If it has been then skip			|
	-----------------------------------------------*/
	while (CheckSjis (++esmr_rec.nx_crd_nte_no) == 0);

	cc = abc_update (esmr,&esmr_rec);
	if (cc)
    {
		sys_err ("Error in esmr during (DBUPDATE)",cc,PNAME);
    }

	abc_unlock (esmr);

	sprintf (sjis_rec.invno,"%08ld",esmr_rec.nx_crd_nte_no);

	/*-------------------------------------------------
	| Created Credit Note No %s, <RETURN> To Continue |
	-------------------------------------------------*/
	print_at (2,40, ML (mlSjMess038), sjis_rec.invno);
	PauseForKey (2, 40, "\x000", 0);

    cc = abc_add (sjis,&sjis_rec);
    if (cc)
    {
        sys_err ("Error in sjis During (DBADD)",cc,PNAME);
    }

	UpdateSjig ();

	clear ();
	print_at (3,1, "%s\n", ML (mlStdMess035));

	sprintf (lp_str,"%2d",local_rec.lp_no);
	* (arg) = "sj_pbcr_prt";
	* (arg+ (1)) = lp_str;
	* (arg+ (2)) = sjis_rec.invno;
	* (arg+ (3)) = (char *)0;

	shell_prog (3);
}

void
SrchSjhr (
 char *key_val)
{
	char	temp_order [9];
	long	order;

	order = atol (key_val);
	work_open ();
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);
	sjhr_rec.order_no = order;
	save_rec ("#Service Job # ","#Issued On");

	cc = find_rec (sjhr, &sjhr_rec, GTEQ, "r");
	while (!cc && 
           !strcmp (sjhr_rec.co_no,comm_rec.co_no) && 
           !strcmp (sjhr_rec.est_no,comm_rec.est_no) && 
           !strcmp (sjhr_rec.dp_no,comm_rec.dp_no))
    {
		sprintf (temp_order,"%8ld",sjhr_rec.order_no);
		if ((strlen (key_val) == 0 || 
            !strncmp (temp_order, key_val, strlen (key_val))) && 
            sjhr_rec.status [0] == 'I')
		{ 
			strcpy (err_str, DateToString (sjhr_rec.issue_date));
			cc = save_rec (temp_order, err_str);
			if (cc)
            {
				break;
            }
		}

		cc = find_rec (sjhr, &sjhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
    {
		return;
    }

	strcpy (sjhr_rec.co_no, comm_rec.co_no);
	strcpy (sjhr_rec.est_no, comm_rec.est_no);
	strcpy (sjhr_rec.dp_no, comm_rec.dp_no);
	order = atol (temp_str);
	sjhr_rec.order_no = order;

	cc = find_rec (sjhr, &sjhr_rec, COMPARISON, "r");
	if (cc)
    {
		sys_err ("Error in sjhr During (DBFIND)",cc,PNAME);
    }
}

void
UpdateSjig (
 void)
{
	int	i = 0;

	/*===================================
	| Write out sjig records for invoice |
	====================================*/
	strcpy (sjhr_rec.co_no,comm_rec.co_no);
	strcpy (sjhr_rec.est_no,comm_rec.est_no);
	strcpy (sjhr_rec.dp_no,comm_rec.dp_no);

	strcpy (sjig_rec.co_no,sjis_rec.co_no);
	strcpy (sjig_rec.est_no,sjis_rec.est_no);
	strcpy (sjig_rec.dp_no,sjis_rec.dp_no);
	strcpy (sjig_rec.invno,sjis_rec.invno);
	sjig_rec.order_no = sjis_rec.order_no;
	scn_set (2);

	for (i = 0; i < lcount [2] ; i++)
	{
		getval (i);
		sjig_rec.amount = local_rec.gl_amt * -1;
		strcpy (sjig_rec.acc_no,local_rec.gl_acc_no);

		cc = abc_add (sjig,&sjig_rec);
		if (cc)
        {
			sys_err ("Error in sjig During (DBADD)",cc,PNAME);
        }
	}
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
	for (i = 0; i < lcount [2] ; i++)
	{
		getval (i);
		gl_chk_tot += local_rec.gl_amt;
	}
	gl_tot_err = local_rec.invoice_chg - gl_chk_tot;

	if (cur_screen != 2)
    {
		scn_set (1);
    }

	if (gl_tot_err == 0.0)
    {
		return (EXIT_SUCCESS);
    }

	/*------------------------------------------------
	| Sum of GL Allocation = %8.2f Variance = %8.2f, |
	| <RETURN> to continue							 |
	------------------------------------------------*/
	print_at (2,40, ML (mlSjMess035), gl_chk_tot,gl_tot_err);
	PauseForKey (2, 40, "\x000", 0);

    return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
        {
			scn_set (scn);
        }
		clear ();
		/*-------------------------------------------
		| S e r v i c e   C r e d i t s   E n t r y |
		-------------------------------------------*/
		sprintf (err_str, " %s ", ML (mlSjMess036));
		rv_pr (err_str,50,0,1);
		line_at (1,0,132);

		if (scn == 1)
		{
			box (0,3,132,15);
			line_at (11,1,131);
		}
		else
		{
			/*----------------------------
			| Amount To Allocate = %8.2f |
			----------------------------*/
		  	print_at (3,0, ML (mlSjMess010), local_rec.invoice_chg);
		}

		line_at (20,0,132);
		sprintf (err_str, 
                 ML (mlStdMess038),
                 comm_rec.co_no,
                 comm_rec.co_name);
		print_at (21,0, err_str);
		print_at (22,0, 
                  ML (mlStdMess039),
                  comm_rec.est_no,
                  comm_rec.est_name);
		line_at (23,0,132);
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
GlCheckClass (void)
{
    if (glmrRec.glmr_class [2][0] != 'P')
	{
		/*-----------------------------------
		| account is not a posting account |
		-----------------------------------*/
		errmess (ML (mlStdMess025));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);;
}

/* [ end of file ] */
