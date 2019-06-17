/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_recon_rep.c,v 5.2 2002/07/17 09:56:56 scott Exp $
|---------------------------------------------------------------------|
|  Program Name  : (ca_recon_rep.c)
|  Program Desc  : (Reconciliation Report)
|---------------------------------------------------------------------|
|  Author        : Alan Rivera .         | Date Written  : 11/07/96   |
|---------------------------------------------------------------------|
| $Log: ca_recon_rep.c,v $
| Revision 5.2  2002/07/17 09:56:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2002/07/09 09:09:04  scott
| Converted to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_recon_rep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_recon_rep/ca_recon_rep.c,v 5.2 2002/07/17 09:56:56 scott Exp $";

#define	STATUS_CONFIRMED	"C"
#define	STATUS_REJECTED		" "

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_ca_mess.h>
#include	<std_decs.h>

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct cbbtRecord	cbbt_rec;
struct cbbsRecord	cbbs_rec;
struct pocrRecord	pocr_rec;

/*============================
| Local & Screen Structures. |
============================*/
struct {
        char    stmt_prd [8];
		char	stmt_prd_desc [10];
		long	prd_end_date;
		Money	amount_clr;
		Money	amount_unclr;
		char	over_draft [12];
		char	over_cleared [15];
		char	status [13];
        int     lpno;

        char    dummy [11];
} local_rec;

static	struct var vars [] = 
{
	{1, LIN, "bank_id",   3, 18, CHARTYPE,
        "UUUUU", "          ",
        " ", "", "Bank Code        ", "Enter Bank Code. [SEARCH] available.  ",
		NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",   3, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "bank_acct_no",    4, 18, CHARTYPE,
        "AAAAAAAAAAAAAAA", "          ",
        " ", "", "Account No       ", "Enter Bank Account No. [SEARCH] available",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
    {1, LIN, "acct_name",  4, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
    {1, LIN, "curcode",  5, 18, CHARTYPE,
        "UUU", "          ",
        " ", "", "Currency Code    ", "",
        NA, NO,  JUSTLEFT, "", "",crbk_rec.curr_code},
    {1, LIN, "curdesc",  5, 35, CHARTYPE,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
        " ", "", "", " ",
        NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
    {1, LIN, "stmt_prd",  7, 18, CHARTYPE,
        "UU/UUUU", "          ",
        " ", "", "Statement Period ", "Enter month and year [MM/YYYY].",
         YES, NO,  JUSTRIGHT, "0,1,2,3,4,5,6,7,8,9","",cbbs_rec.period},
    {1, LIN, "stmt_prd_desc",  7, 35, CHARTYPE,
        "UUUUUUUUU", "          ",
        " ", "", "", " ",
         NA, NO,  JUSTLEFT, "", "", local_rec.stmt_prd_desc},
    {1, LIN, "prd_end_date",  8, 18, EDATETYPE,
        "DD/DD/DD", "          ",
        " ", "", "Period End Date  ", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.prd_end_date},
    {1, LIN, "open_bal",  10, 18, MONEYTYPE,
        "NNNNNN,NNN.NN", "          ",
        " ", "", "Opening Balance  ", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&cbbs_rec.st_bal},
    {1, LIN, "close_bal",  11, 18, MONEYTYPE,
        "NNNNNN,NNN.NN", "          ",
        " ", "", "Closing Balance  ", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&cbbs_rec.end_bal}, 
    {1, LIN, "over_draft",  11, 35, CHARTYPE,
        "AAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTRIGHT, "", "", local_rec.over_draft}, 
    {1, LIN, "amt_clr",  13, 18, MONEYTYPE,
        "NNNNNN,NNN.NN", "          ",
        " ", "", "Amount Cleared   ", "",
         NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.amount_clr},
    {1, LIN, "amt_unclr",  14, 18, MONEYTYPE,
        "NNNNNN,NNN.NN", "          ",
        " ", "", "Amount Uncleared  ", "",
         NA, NO,  JUSTRIGHT, "-9999999.99", "9999999.99", (char *)&local_rec.amount_unclr},
    {1, LIN, "over_cleared",  14, 35, CHARTYPE,
        "AAAAAAAAAAAAA", "          ",
        " ", "", "", "",
         NA, NO,  JUSTRIGHT, "", "", local_rec.over_cleared}, 
	{1, LIN, "lpno",	 16, 18, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer          ", "Printer Number ",
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*==========================
| Special fields and flags |
==========================*/
FILE 	*fout,
		*fsort;

int		edit_mode 		= 0,
		scn 			= 1;

char	*data   = "data";

char    systemDate [11];

void	OpenDB 			(void);
void	CloseDB 		(void);
void 	SrchCrbk 		(char *);
int 	EndReport 		(void);
void 	HeadingOutput 	(void);
void 	DisplayReport 	(void);
void 	HeadingDisplay 	(void);
void 	HeadingDisplay2 (void);
int 	heading 		(int);
void 	ProcessFile 	(void);

/*=========================
| Main Processing Routine |
=========================*/
int
main (
	int argc,
	char *argv [])
{
	SETUP_SCR (vars);
	
	OpenDB ();

	strcpy (systemDate, DateToString (TodaysDate ()));

	/*-----------------------------
	| Set up required parameters  |
	-----------------------------*/
	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1); 

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
		init_vars (1);      

	    /*-----------------------------	
		| Entry screen 1 linear input |
	    -----------------------------*/
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

		dsp_screen ("Printing Reconciled Report",
			comm_rec.co_no,comm_rec.co_name);

		HeadingOutput ();
        DisplayReport ();

        fprintf (fout,".EOF\n");
    	fflush (fout);
    	sort_delete (fsort,"ca_recon_rep");
        EndReport ();
		prog_exit = 1;
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cbbs, cbbs_list, CBBS_NO_FIELDS, "cbbs_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (cbbt, cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no1");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_dbclose (data);
}

/*===================================+
| Special validation of screen fields|
+===================================*/
int
spec_valid (
	int field)
{
	/*-------------------------------------------+
	| Validate Creditor Number And Allow Search. |
	+-------------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (crbk_rec.bank_id, "     "))
			return (EXIT_FAILURE);

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		strcpy (crbk_rec.bank_id, crbk_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
		{
			DSP_FLD ("bank_id");
			DSP_FLD ("bk_name");
			DSP_FLD ("bank_acct_no");
			DSP_FLD ("acct_name");

			strcpy (pocr_rec.co_no, comm_rec.co_no);
			strcpy (pocr_rec.code, crbk_rec.curr_code);
			cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
			if (!cc)
			{
				DSP_FLD ("curcode");
				DSP_FLD ("curdesc");
			}
			return (EXIT_SUCCESS);
		}
	}

	/*---------------------------+
	| Validate Statement Period. |
	+---------------------------*/
	if (LCHECK ("stmt_prd"))
	{
		char 	data_string [8];
		int 	mth;
		int 	yr;
		long	CalcDate;

		strcpy (data_string, cbbs_rec.period);
		mth	=	atoi (data_string);
		yr	=	atoi (data_string + 3);

		if (mth <= 0 || mth >= 13 || yr < 0)
		{
			/*----------------
			| Incorrect date |
			----------------*/
			print_mess (ML ("Please correct date."));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.stmt_prd_desc, "%3.3s", MonthName (mth));
		CalcDate = DMYToDate (1, mth, yr);
		local_rec.prd_end_date	=	MonthEnd (CalcDate);
       
        sprintf (local_rec.stmt_prd, cbbs_rec.period);
		sprintf (local_rec.stmt_prd_desc, "%9.9s", MonthName (mth));

		DSP_FLD ("stmt_prd_desc");
		DSP_FLD ("prd_end_date");

		/*-----------------------------+
		| Check if existing statement. |
		+-----------------------------*/
		strcpy (cbbs_rec.co_no, comm_rec.co_no);
		strcpy (cbbs_rec.bank_id, crbk_rec.bank_id);
		cbbs_rec.end_period = local_rec.prd_end_date;
		cc = find_rec (cbbs, &cbbs_rec, COMPARISON, "r");
		if (!cc && 
			!strcmp (cbbs_rec.co_no, comm_rec.co_no) &&
			!strcmp (cbbs_rec.bank_id, crbk_rec.bank_id) &&
			cbbs_rec.end_period == local_rec.prd_end_date &&
			cbbs_rec.stat [0] == 'R')
		{
			ProcessFile ();

			strcpy (local_rec.status, "Reconciled");

			if (cbbs_rec.end_bal < 0)
			{
				strcpy (local_rec.over_draft, " (Overdraft)");
				DSP_FLD ("over_draft");
			}

			if (local_rec.amount_unclr < 0)
			{
				strcpy (local_rec.over_cleared, " (Over cleared)");
				DSP_FLD ("over_cleared");
			}

			DSP_FLD ("open_bal");
			DSP_FLD ("close_bal");
			DSP_FLD ("amt_clr");
			DSP_FLD ("amt_unclr");

			return (EXIT_SUCCESS);
		}
		else
		{
			print_mess (ML (mlCaMess013));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
	}

	/*-------------
	| Printer No. | 
	-------------*/
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*========================================+
| Search routine for Creditors Bank File. |
+========================================*/
void
SrchCrbk ( 
	char *key_val)
{
	work_open ();
	cc = save_rec ("#Bank Id ","#Bank Name");

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s",key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc &&
			!strcmp (crbk_rec.co_no, comm_rec.co_no) &&
			!strncmp (crbk_rec.bank_id, key_val, strlen (key_val)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();

	if (!restart)
	{
		init_vars (1);   
		return; 
	}

	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc) 
		file_err (cc, crbk, "DBFIND");
}

/*============
| End  Print |
============*/
int
EndReport (void)
{
	pclose (fout);
	return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat","w")) == (FILE *) NULL)
		file_err (errno, "pformat", "POPEN");

	sprintf (err_str, "%-10.10s <%s>", systemDate, PNAME);
	fprintf (fout, ".START%s\n", clip (err_str));
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".6\n");
	fprintf (fout, ".L95\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".C%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".CRECONCILIATION REPORT \n\n");
	fprintf (fout, ".R============");   
	fprintf (fout, "=========");   
	fprintf (fout, "=========================================");
	fprintf (fout, "====================");
	fprintf (fout, "============\n");
	fflush (fout); 
}

/*=================
| Display details |
=================*/
void
DisplayReport (void)
{
	char	*sptr;
    int     lctr;

	Date    trandate;
	char	tranno [7],
			trandesc [41],
			reconflag [2];

	Money	tranamt		=	0.00,
			GrndTotal	=	0.00;

	sptr =  sort_read (fsort);   

    lctr = 25;    

 	HeadingDisplay ();  

	while (sptr != (char *)0)
	{
       	trandate = atol (sptr);
       	sprintf (tranno, "%-6.6s", sptr+12);
       	sprintf (trandesc, "%-40.40s", sptr+19);
		tranamt = atof (sptr+60);
       	sprintf (reconflag, "%1.1s", sptr+71);

		fprintf (fout, "|%-10.10s ", DateToString (trandate));
		fprintf (fout, "| %-6.6s ", tranno);
		fprintf (fout, "| %-40.40s", trandesc);
		fprintf (fout, "|   %12.12s   ", comma_fmt (tranamt,"N,NNN,NNN.NN")); 
		fprintf (fout, "|    %1.1s     |\n", reconflag);
       	GrndTotal += tranamt;
     	lctr += 1;

		if (lctr > 55)
       	{
	   		fprintf (fout,".PA\n");
			HeadingDisplay2 ();
       		lctr = 5;   
       	}

		sptr = sort_read (fsort);
	}
	fprintf (fout,"============");   
	fprintf (fout,"=========");   
	fprintf (fout,"=========================================");
	fprintf (fout,"====================");
	fprintf (fout,"============\n");
	fprintf (fout,"| *** G R A N D     T O T A L  ***                             |");
	fprintf (fout,"   %12.12s  |\n", comma_fmt (GrndTotal,"N,NNN,NNN.NN"));
}

/*========================
| Display report headers |
========================*/
void
HeadingDisplay (void)
{
	fprintf (fout,"   Account Name      :      %-40.40s\n",
			crbk_rec.acct_name);
	fprintf (fout,"   Bank Number       :      %-5.5s\n",
			crbk_rec.bank_id);
	fprintf (fout,"   Bank Acct No      :      %-15.15s\n",
			crbk_rec.bank_acct_no);
	fprintf (fout,"   Currency Code     :      %-3.3s          %40.40s\n\n",
			crbk_rec.curr_code, pocr_rec.description);
	fprintf (fout,"    Opening Balance  :      %12.12s\n",
			comma_fmt (DOLLARS (cbbs_rec.st_bal), "N,NNN,NNN.NN"));
	fprintf (fout,"    Closing Balance  :      %12.12s  %11.11s\n",
			comma_fmt (DOLLARS (cbbs_rec.end_bal), "N,NNN,NNN.NN"),
			local_rec.over_draft);
	fprintf (fout,"    Amount Cleared   :      %12.12s\n",
			comma_fmt (DOLLARS (local_rec.amount_clr),"N,NNN,NNN.NN"));
	fprintf (fout,"    Amount Uncleared :      %12.12s  %14.14s\n",
			comma_fmt (local_rec.amount_unclr,"N,NNN,NNN.NN"),
			local_rec.over_cleared);
	fprintf (fout,"    Status           :      %-12.12s\n",
			local_rec.status);    
	fprintf (fout,"    Statement Period :      %7.7s  %10.10s\n",
			local_rec.stmt_prd, local_rec.stmt_prd_desc);
	fprintf (fout,"    Period End Date  :    %-10.10s\n\n",
			DateToString (local_rec.prd_end_date));

	fprintf (fout,"============");   
	fprintf (fout,"=========");   
	fprintf (fout,"========================================");
	fprintf (fout,"=====================");
	fprintf (fout,"============\n");
 
	fprintf (fout,"| Date      ");   
	fprintf (fout,"| Number ");   
	fprintf (fout,"| Description                             ");
	fprintf (fout,"|  Amount Cleared  ");
	fprintf (fout,"|  Status  |\n");
 
	fprintf (fout,"------------");   
	fprintf (fout,"---------");   
	fprintf (fout,"-----------------------------------------");
	fprintf (fout,"--------------------");
	fprintf (fout,"------------\n");
	fflush (fout);
}

	/*------------------------------
	| Display second report header |
	------------------------------*/
void
HeadingDisplay2 (void)
{
	fprintf (fout,"============");   
	fprintf (fout,"=========");   
	fprintf (fout,"=========================================");
	fprintf (fout,"====================");
	fprintf (fout,"============\n");

	fprintf (fout,"| Date      ");   
	fprintf (fout,"|Number  ");   
	fprintf (fout,"|Description                            ");
	fprintf (fout,"|   Amount Cleared   ");
	fprintf (fout,"|  Status  |\n");
 
 	fprintf (fout,"------------");   
	fprintf (fout,"---------");   
	fprintf (fout,"-----------------------------------------");
	fprintf (fout,"--------------------");
	fprintf (fout,"------------\n");
}

/*=======================
| Display Screen Header |
=======================*/
int
heading (
	int scn
	)
{
	if (restart) 
		return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);    

	clear ();

	rv_pr (ML (mlCaMess050), 29, 0, 1);

	move (0, 1);
	line (80);

	box (0, 2, 80, 14);
	move (1, 6);
	line (79);
	move (1, 15);
	line (79);
	move (0, 20);
	line (80);

	line_cnt = 0;
    scn_write (scn);   
	return (EXIT_SUCCESS);
}

/*=====================================+
| Process Reconciled Bank Transactions |
+=====================================*/
void
ProcessFile (void)
{
	char	tmp_line [75];

	fsort = sort_open ("ca_recon_rep");  

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = 0L;
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc) 
	{
		if (cbbt_rec.period == local_rec.prd_end_date &&
  			!strcmp (cbbt_rec.co_no, comm_rec.co_no) &&  
			!strcmp (cbbt_rec.bank_id, crbk_rec.bank_id) &&
			!strcmp (cbbt_rec.stat_post, "N"))
		{
			sprintf (tmp_line, "%11ld %6ld %-40.40s %10.2f %1.1s\n",
					cbbt_rec.tran_date,				
					cbbt_rec.tran_no,			 
					cbbt_rec.tran_desc,			  
					DOLLARS (cbbt_rec.tran_amt), 
					cbbt_rec.select);

			local_rec.amount_clr += cbbt_rec.tran_amt;

			sort_save (fsort, tmp_line);  
	    }
		cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
	}
    fsort = sort_sort (fsort, "ca_recon_rep"); 
}
