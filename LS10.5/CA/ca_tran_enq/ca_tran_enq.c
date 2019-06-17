/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_tran_enq.c,v 5.2 2002/07/09 09:23:09 scott Exp $
|  Program Name  : (ca_tran_enq.c)
|  Program Desc  : (System Bank Transaction Enquiry)
|---------------------------------------------------------------------|
|  Author        : Alan Rivera .        | Date Written  : 21/06/96    |
|---------------------------------------------------------------------|
| $Log: ca_tran_enq.c,v $
| Revision 5.2  2002/07/09 09:23:09  scott
| Updated to convert to app.schema
|
| Revision 5.1  2002/03/01 01:48:45  scott
| Added app.schema + general clean of code.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_tran_enq.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_tran_enq/ca_tran_enq.c,v 5.2 2002/07/09 09:23:09 scott Exp $";

#define SCN_INIT	1

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <hot_keys.h>   
#include <assert.h>
#include <ml_std_mess.h>
#include <ml_ca_mess.h>
#include	<std_decs.h>
#include	<tabdisp.h>

char	*data   = "data";

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct cbbtRecord	cbbt_rec;

/*
 * Local & Screen Structures 
 */
struct {
	long    lsystemDate;
	Date	startdate;
	Date	enddate;
	char	trantype [2];
	char    trandesc [14];
	char	stat_post [2];
	char	reconciled [2];
	long	start_tranno;
	long	end_tranno;
	char    dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "bankid", 	 4, 19, CHARTYPE, 
		"UUUUU", "      ", 
		" ", "", "Bank ID           ", "Enter Bank Code [SEARCH]", 
		 YES, NO, JUSTLEFT, "", "", crbk_rec.bank_id}, 
	{1, LIN, "bankid_desc", 	 5, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Bank/Branch Name  ", " ", 
		 NA, NO, JUSTLEFT, "", "", crbk_rec.bank_name}, 
	{1, LIN, "acct_no", 	 7, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", "", "Account Number    ", "Enter Account No. or [SEARCH]. ", 
		 NA, NO, JUSTLEFT, "", "", crbk_rec.bank_acct_no}, 
	{1, LIN, "acctname", 	 8, 19, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Account Name      ", " ", 
		 NA, NO, JUSTLEFT, "", "", crbk_rec.acct_name}, 
	{1, LIN, "start_tranno", 	 10, 19, INTTYPE, 
		"NNNNNNNNNNN", "          ", 
		" ", "0", "Start Trans No    ", "Start Transaction Number ", 
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.start_tranno}, 
	{1, LIN, "end_tranno", 	10, 60, INTTYPE, 
		"NNNNNNNNNNN", "          ", 
		" ", "999999999", "End Trans No      ", "End Transaction Number ", 
		 YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.end_tranno}, 
	{1, LIN, "startdate", 	 11, 19, EDATETYPE, 
		"DD/DD/DD", "           ", 
		" ", "00/00/00", "Start Date        ", "Default to 00/00/00 ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.startdate}, 
	{1, LIN, "enddate", 	 11, 60, EDATETYPE, 
		"DD/DD/DD", "           ", 
		" ", "00/00/00", "End Date          ", "Default to End of Month ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.enddate}, 
	{1, LIN, "reconciled", 13, 19, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Reconciled ?      ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.reconciled}, 
/*
	{1, LIN, "stat_post", 14, 19, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Posted ?          ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.stat_post}, 
*/
	{1, LIN, "trantype", 	 14, 19, CHARTYPE, 
		"U", "          ", 
		" ", " ", "Transaction Type  ", "A(dj) C(hr) I(nt) P(etty Cash) D(isbursement) R(eceipt) T(Bank Transfer)" , 
		YES, NO, JUSTLEFT, "ACIPDRT", "", local_rec.trantype}, 
	{1, LIN, "trandesc", 	 14, 35, CHARTYPE, 
		"AAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.trandesc}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * Special fields and flags 
 */

char	*table_fmt =  "%-10.10s %11ld %-40.40s %17.2f %-11.11s   %-1.1s        %-1.1s";
static	int		curr_scn = SCN_INIT;

static int	noInDsp = 0;   /* no. of lines added to table  */  
static int	ExitFunc (int, KEY_PTR);

static  KEY_TAB list_keys [] =
{
   { NULL, 				FN16, 				ExitFunc, 
    "Exit ", 		"A" }, 
   END_KEYS
};

/*
 * Exit Function 
 */
static int
ExitFunc (
	int 	key, 
	KEY_PTR psUnused)
{
	assert (key == FN16);
	curr_scn = SCN_INIT;
	return key;
}

void	ProcessTransactions 	 (void);
void	Initcbbt 				 (void);
void	OpenDB 					 (void);
void	CloseDB 				 (void);
int 	spec_valid 				 (int);
int 	heading 				 (int);
void 	SrchCrbk 				 (char *);
void 	SrchCbbt 				 (char *);

/*
 * Main Processing Routine 
 */

int
main (
 int argc, 
 char * argv [])
{
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);
	OpenDB ();

	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1);

	while (!prog_exit)
	{
		search_ok 	= 	TRUE;
		entry_exit 	= 	FALSE;
		edit_exit 	= 	TRUE;
		prog_exit 	= 	FALSE;
		restart 	= 	FALSE;
		init_ok 	= 	TRUE;

		init_vars (1);

		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input 
		 */
		heading (1);
	    scn_display (1);   
		edit (1);      
		if (restart)
			continue;

    	clear ();
		swide ();
        heading (2);

        if (!restart && !prog_exit)
       	   ProcessTransactions ();
		else
			continue;
	}
	CloseDB (); 
    FinishProgram ();		
	return (EXIT_SUCCESS);
}

/*
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cbbt, cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no2");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_dbclose (data);
}

/*
 * Special Validation  
 */
int
spec_valid (int field)
{
	/*
	 * Validate Bank ID and allow search. 
	 */
	if (LCHECK ("bankid"))
	{
		if (SRCH_KEY)
		{
	 		SrchCrbk (temp_str);
  			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("bankid");
		DSP_FLD ("bankid_desc");
		DSP_FLD ("acct_no");
		DSP_FLD ("acctname");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("start_tranno"))
	{
		if (dflt_used)
		{
			local_rec.start_tranno = 0;
			return (EXIT_SUCCESS);
		}		

		if (SRCH_KEY)
		{
	 		SrchCbbt (temp_str);
			return (EXIT_SUCCESS);
		}

		abc_selfield (cbbt, "cbbt_id_no1");
		strcpy (cbbt_rec.co_no, comm_rec.co_no);
		strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
		cbbt_rec.tran_no = local_rec.start_tranno;
		cc = find_rec (cbbt, &cbbt_rec, EQUAL, "r");
		if (cc)
		{
			abc_selfield (cbbt, "cbbt_id_no2");

			errmess (ML (mlStdMess016));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.start_tranno = cbbt_rec.tran_no; 
		abc_selfield (cbbt, "cbbt_id_no2");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_tranno"))
	{
		if (SRCH_KEY)
		{
			sprintf (temp_str, "%11ld", local_rec.start_tranno);
	 		SrchCbbt (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			local_rec.end_tranno = 999999999;
			if (local_rec.start_tranno > local_rec.end_tranno)
			{
				errmess (ML (mlStdMess006));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}		

		abc_selfield (cbbt, "cbbt_id_no1");
		strcpy (cbbt_rec.co_no, comm_rec.co_no);
		strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
		cbbt_rec.tran_no = local_rec.end_tranno;
		cc = find_rec (cbbt, &cbbt_rec, EQUAL, "r");
		if (cc)
		{
			abc_selfield (cbbt, "cbbt_id_no2");
			errmess (ML (mlStdMess016));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.start_tranno > local_rec.end_tranno)
		{
			abc_selfield (cbbt, "cbbt_id_no2");
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		abc_selfield (cbbt, "cbbt_id_no2");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startdate"))
	{
		if (dflt_used)
			local_rec.startdate = 0l;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("enddate"))
	{
		if (dflt_used)
			local_rec.enddate = MonthEnd (local_rec.lsystemDate);

		DSP_FLD ("enddate");

		if (local_rec.startdate > local_rec.enddate)
		{
			errmess (ML (mlStdMess019));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY && 
				local_rec.startdate > local_rec.enddate)
		{
			errmess (ML (mlStdMess019));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reconciled"))
	{
		if (dflt_used)
			strcpy (local_rec.reconciled, "N");

		return (EXIT_SUCCESS);
	}
/*
	if (LCHECK ("stat_post"))
	{
		if (dflt_used)
			strcpy (local_rec.stat_post, "N");

		return (EXIT_SUCCESS);
	}
*/

	if (LCHECK ("trantype"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.trandesc, "ALL");
			DSP_FLD ("trandesc");
			return (EXIT_SUCCESS);
		}

		switch (local_rec.trantype [0])
		{
		case	'A':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Adjustment"));
			break;

		case	'P':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Petty Cash"));
			break;

		case	'C':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Charge"));
			break;

		case	'I':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Interest"));
			break;
		case	'D':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Disbursement"));
			break;
		case	'R':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Receipt"));
			break;
		case	'T':
			sprintf (local_rec.trandesc, "%-13.13s", ML ("Bank Transfer"));
			break;
		}

		DSP_FLD ("trantype");
		DSP_FLD ("trandesc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===========================
| Display transaction table |
===========================*/
void
ProcessTransactions (void)
{
	noInDsp = 0;

	tab_open ("TabTempFile", list_keys, 6, 10, 12, FALSE);
	tab_add 
	(
		"TabTempFile", 
		"#%-10.10s %-11.11s %-40.40s %-17.17s %-11.11s %-6.6s %-10.10s", 
		"Date    ", 
		"Trans No.  ", 
		"Description                             ", 
		"          Amount", 
		"Trans Type ", 
		"Posted", 
		"Reconciled"
	);

	Initcbbt ();

	if (noInDsp == 0)
	{
		tab_add ("TabTempFile", ML (mlStdMess009));
		tab_display ("TabTempFile", TRUE);
		sleep (sleepTime);
		tab_close ("TabTempFile", TRUE);
		curr_scn =	SCN_INIT;
		restart = TRUE;
		return;
	}
	else
    {
 		tab_scan ("TabTempFile");
	}
	tab_close ("TabTempFile", TRUE);
   	return; 	
}

/*
 * Display Screen Header 
 */
int
heading (int scn)
{
    if (restart)
		return (EXIT_FAILURE);

	if (scn == 1)
		scn_set (scn);   

	clear ();
	switch (scn)
    {
    case 1:
		snorm ();
		rv_pr (ML (mlCaMess068), 25, 0, 1);

		box (0, 3, 80, 11);
		line_at (1, 0, 80);
		line_at (6, 1, 79);
		line_at (9, 1, 79);
		line_at (12, 1, 79);
		line_at (22, 0, 80);
	    scn_write (1);   

        break;
    case 2:
		rv_pr (ML (mlCaMess068), 50, 0, 1);
		line_at (1, 0, 132);

		box (0, 2, 132, 3);

		print_at (3, 3, ML (mlStdMess082), crbk_rec.bank_id);
		print_at (3, 59, " %s", crbk_rec.bank_name);
		print_at (4, 3, ML (mlStdMess114), crbk_rec.bank_acct_no, "");
		print_at (4, 59, " %s", crbk_rec.acct_name);
		print_at (5, 3, ML (mlStdMess112), DateToString (local_rec.startdate));
		print_at (5, 59, ML (mlStdMess113), DateToString (local_rec.enddate));

		line_at (22, 0, 132);

        break;
	}
	return (EXIT_SUCCESS);
}

/*==========================
| Search for Bank ID File. |
==========================*/
void
SrchCrbk (char *keyValue)
{
	_work_open (5, 0, 40);

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", keyValue);
	save_rec ("#Bank", "#Branch Name");
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && 
			!strcmp (crbk_rec.co_no, comm_rec.co_no) &&
			!strncmp (crbk_rec.bank_id, keyValue, strlen (keyValue)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
			break;
		cc = find_rec (crbk, &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	sprintf (crbk_rec.bank_id, "%-5.5s", temp_str);
	cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, crbk, "DBFIND");
}

/*===============================
| Search for Transactions File. |
===============================*/
void
SrchCbbt (char *keyValue)
{
    char    tranno [12];
	_work_open (11,0,40);

	save_rec ("#Tran. No.  #", "#Transaction Description");
	abc_selfield (cbbt, "cbbt_id_no1");

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = atol (keyValue);
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc && 
 			!strcmp (cbbt_rec.co_no, comm_rec.co_no) &&
            !strcmp (cbbt_rec.bank_id, crbk_rec.bank_id)) 
	{
        sprintf (tranno, "%-11ld", cbbt_rec.tran_no);
		cc = save_rec (tranno, cbbt_rec.tran_desc);
		if (cc)
			break;
		cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		abc_selfield (cbbt, "cbbt_id_no2");
		return;
	}

	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
	cbbt_rec.tran_no = atol (temp_str);
	cc = find_rec (cbbt, &cbbt_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cbbt, "DBFIND");

	abc_selfield (cbbt, "cbbt_id_no2");
}

/*
 *  Init cbbt Table  
 */
void
Initcbbt (void)
{
	char    trandesc [14]; 
  
	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, crbk_rec.bank_id);
    cbbt_rec.tran_date = 0L;
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc)  
	{
 		if (!strcmp (cbbt_rec.co_no, comm_rec.co_no) &&
		    !strcmp (cbbt_rec.bank_id, crbk_rec.bank_id))
        {

			if (cbbt_rec.tran_date < local_rec.startdate || 
           		cbbt_rec.tran_date > local_rec.enddate) 
            {
				cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
				continue;
            }

         	if (local_rec.start_tranno != 0 || local_rec.end_tranno != 0)
            {     
    			if (cbbt_rec.tran_no < local_rec.start_tranno ||
             		cbbt_rec.tran_no > local_rec.end_tranno)
                {
				   cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
                   continue;
                }  
			}

/*
			if (strcmp (cbbt_rec.stat_post, local_rec.stat_post)) 
            {  
		   	   cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
               continue;
            }
*/

			if (strcmp (cbbt_rec.reconciled, local_rec.reconciled)) 
            {  
		   	   cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
               continue;
            }

            if (strncmp (local_rec.trantype, " ", 1))
            {
				if (strcmp (cbbt_rec.tran_type, local_rec.trantype)) 
                {  
		    	   cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
                   continue;
                }
			}
			noInDsp++;

			switch (cbbt_rec.tran_type [0])
			{
			case	'A':
				sprintf (trandesc, "%-13.13s", ML ("Adjustment"));
				break;

			case	'P':
				sprintf (trandesc, "%-13.13s", ML ("Petty Cash"));
				break;

			case	'C':
				sprintf (trandesc, "%-13.13s", ML ("Charge"));
				break;

			case	'I':
				sprintf (trandesc, "%-13.13s", ML ("Interest"));
				break;

			case	'D':
				sprintf (trandesc, "%-13.13s", ML ("Disbursement"));
				break;

			case	'R':
				sprintf (trandesc, "%-13.13s", ML ("Receipt"));
				break;

			case	'T':
				sprintf (trandesc, "%-13.13s", ML ("Bank Transfer"));
				break;
			}

			tab_add 
			(
				"TabTempFile", 
				table_fmt, 
				DateToString (cbbt_rec.tran_date),
				cbbt_rec.tran_no, 
				cbbt_rec.tran_desc, 
				DOLLARS (cbbt_rec.tran_amt), 
				trandesc, 
				cbbt_rec.stat_post, 
				cbbt_rec.reconciled
			);
		}
		cc = find_rec (cbbt, &cbbt_rec, NEXT, "r");
    }	
}
