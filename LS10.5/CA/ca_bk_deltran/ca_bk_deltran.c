/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: ca_bk_deltran.c,v 5.1 2002/02/19 06:06:11 scott Exp $
|  Program Name  : (ca_bk_deltran.cpp)                               |
|  Program Desc  : (Deletion of Obsolete System Bank Transactions)   |
|---------------------------------------------------------------------|
|  Date Written  : 07/03/96        | Author 	 : Edz Monserrate     |
|---------------------------------------------------------------------|
| $Log: ca_bk_deltran.c,v $
| Revision 5.1  2002/02/19 06:06:11  scott
| Updated to convert to app.schema and general clean of code
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ca_bk_deltran.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CA/ca_bk_deltran/ca_bk_deltran.c,v 5.1 2002/02/19 06:06:11 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_ca_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct crbkRecord	crbk_rec;
struct cbbtRecord	cbbt_rec;

static char	*data	  = "data";

/*
 * Local & Screen Structures. 
 */
struct {
	char	co_no       [3];
	char	bank_id     [6];
	char	bank_name   [41];
	char	bank_no     [16];
	char	branch_name [41];
	char	bank_acct   [16];
	char	acct_name   [41];
	char	tran_no     [7];
	char	dummy       [11];
	long	bg_date;
	long	ed_date;
} local_rec;

/*
 * Other Variable Declarations
 */
char	systemDate [11],
		Curr_code [4];

static	struct	var	vars [] =
{
    {1, LIN, "bank_id",	 4, 16, CHARTYPE,
		"UUUUU", "          ",
		" ", " ", "Bank Code", "Enter Bank Code. [SEARCH] available.",
		 YES, NO,  JUSTLEFT, "", "", local_rec.bank_id},
	{1, LIN, "bank_name", 5, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Name", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bank_name},
	{1, LIN, "bank_no",	6, 16, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank No.", "Enter bank number. ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bank_no},
	{1, LIN, "br_name",	 7, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.branch_name},
	{1, LIN, "bk_acno",	8, 16, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", " ", "Account No.", "Enter bank account number [w/ DEFAULT].",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bank_acct},
	{1, LIN, "acct_name", 9, 16, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Name", "Enter bank account name.",
		 NA, NO,  JUSTLEFT, "", "", local_rec.acct_name},
	{1, LIN, "bg_date",	11, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Begin Date", "Enter Begin Date.  Default 1st last month.",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.bg_date},
	{1, LIN, "ed_date",	12, 16, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End Date", "Enter End Date. Default end last month.",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.ed_date},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include    <std_decs.h>
/*
 * Local Function declarations
 */
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	SrchCrbk 		(char *);
void 	ProcTranDetails (void);
int 	spec_valid 		(int);
int		heading 		(int);


/*
 * Main Processing Routine
 */
int      
main (
 int argc,
 char * argv [])
{
	strcpy (systemDate, DateToString (TodaysDate ()));

	SETUP_SCR (vars);
	OpenDB ();

	/*
	 * Set up required parameters 
	 */
	init_scr ();			
	set_tty ();         
	set_masks ();	
	init_vars (1);
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*
		 * Entry screen 1 linear input. 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input. 
		 */
		heading (1);
		scn_display (1);
		edit (1);      
		if (restart)
			continue;

		ProcTranDetails ();
	}
	shutdown_prog ();

    return (EXIT_SUCCESS);
}

/*
 * Function     : shutdown_prog (void)
 * Description  : Program exit sequence.
 * Parameters   : None.
 * Return       : None.
 */
void
shutdown_prog (void)
{
    CloseDB (); 
	FinishProgram ();
}

/*
 * Function     :    OpenDB (void)
 *
 * Description  :    Open Database Files.
 *
 * Parameters   :    None.
 *
 * Return       :    None.
 */
void
OpenDB (void)
{
    abc_dbopen (data);
    read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);

    open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
    open_rec (cbbt, cbbt_list, CBBT_NO_FIELDS, "cbbt_id_no2");
}

/*
 * Function		:    CloseDB (void)
 *
 * Description	:    Close Database Files.
 *
 * Parameters	:    None.
 *
 * Return       :    None.
 */
void
CloseDB (void)
{
	abc_fclose (crbk);
	abc_fclose (cbbt);
	abc_dbclose (data);
}

/*
 * Function     :    spec_valid (int field)
 *
 * Description	:
 *
 * Parameters	:    int field   field to validate.
 *
 * Return       :    0 - Okay, 1 - Error.
 */
int
spec_valid (
	int		field)
{
    /*
	 * Validate Bank ID And Allow Search. 
	 */
	if (LCHECK ("bank_id"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.bank_id, "     "); 
			strcpy (local_rec.bank_name, ML ("All Banks"));
			DSP_FLD ("bank_id"); 
			DSP_FLD ("bank_name");
			return (EXIT_SUCCESS); 
		}

		if (SRCH_KEY)
		{
	 		SrchCrbk (temp_str);
  			return (EXIT_SUCCESS);
		}
		strcpy (crbk_rec.co_no, comm_rec.co_no);
		strcpy (crbk_rec.bank_id, local_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.bank_name, crbk_rec.bank_name);
		strcpy (local_rec.bank_id, crbk_rec.bank_id);
		strcpy (cbbt_rec.bank_id, local_rec.bank_id);
		strcpy (local_rec.bank_no, crbk_rec.bank_no);       
		strcpy (local_rec.branch_name, crbk_rec.branch_name);       
		strcpy (local_rec.bank_acct, crbk_rec.bank_acct_no);
		strcpy (local_rec.acct_name, crbk_rec.acct_name);
		DSP_FLD ("bank_id"); 
		DSP_FLD ("bank_name");
		DSP_FLD ("bank_no");
		DSP_FLD ("br_name");
		DSP_FLD ("bk_acno");
		DSP_FLD ("acct_name");
		entry_exit = 0; 
		return (EXIT_SUCCESS); 
	}


	/*
	 * Validate Begin Date. 
     */
	if (LCHECK ("bg_date"))
	{
		if (dflt_used)
        {
			local_rec.bg_date =	
                MonthStart (MonthStart (StringToDate (systemDate)) - 1L);
        }
			
		if (local_rec.bg_date > StringToDate (systemDate))
		{
			print_mess (ML (mlStdMess086));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("bg_date");
		entry_exit = 0;
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Date. 
	 */
	if (LCHECK ("ed_date"))
	{
		local_rec.ed_date	=	MonthEnd (
            MonthStart (StringToDate (systemDate)) - 1L);
		if (local_rec.ed_date > StringToDate (systemDate) || 
           (local_rec.bg_date > local_rec.ed_date))
		{
			print_mess (ML (mlStdMess013));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("ed_date");
		entry_exit = 0;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Function     :   ProcTranDetails (void)
 *
 * Description	:   Main processing routine to delete bank system 
 *                  transactions.
 *
 * Parameters   :   None.
 *
 * Return       :   None.
 */
void
ProcTranDetails (void)
{
    int    tran_found = FALSE,
           unrecon_found = FALSE;
    int		AnswerPrmpt;

	AnswerPrmpt = prmptmsg ((ML (mlCaMess001)), "YyNn", 1,23);
	move (1,23);
	if (AnswerPrmpt == 'N' || AnswerPrmpt == 'n')
	{	
		clear_mess ();
		restart = TRUE;
		return ;
	}
	
	strcpy (cbbt_rec.co_no, comm_rec.co_no);
	strcpy (cbbt_rec.bank_id, local_rec.bank_id);
	cbbt_rec.tran_date = local_rec.bg_date;
	cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	while (!cc &&
           !strcmp (cbbt_rec.co_no, comm_rec.co_no) &&
          (!strcmp (cbbt_rec.bank_id, local_rec.bank_id) ||
           !strcmp (local_rec.bank_id, "     ")))  
	{
		if (cbbt_rec.tran_date > local_rec.ed_date)
        {
			break;
        }

		tran_found = TRUE;	
		if (cbbt_rec.reconciled [0] == 'N' && unrecon_found == FALSE)
		{
			AnswerPrmpt = prmptmsg ((ML (mlCaMess002)), "YyNn", 1,23);
			move (1,23);
			unrecon_found = TRUE;

			if (AnswerPrmpt == 'N' || AnswerPrmpt == 'n')
			{	
				clear_mess ();
				restart = TRUE;
				return;
			}
		}

		/*
		 * Deletion in progress.
		 */
		print_mess (ML (mlStdMess014));
		sleep (sleepTime);
		clear_mess ();
		cc = abc_delete (cbbt);
		if (cc)
        {
			file_err (cc, cbbt, "DBDELETE"); 
        }
		
		strcpy (cbbt_rec.co_no, comm_rec.co_no);
		strcpy (cbbt_rec.bank_id, local_rec.bank_id);
		cbbt_rec.tran_date = local_rec.bg_date;
		cc = find_rec (cbbt, &cbbt_rec, GTEQ, "r");
	}
	if (tran_found == FALSE)
	{   /*
         * No Records to Delete.
		 */
		print_mess (ML (mlStdMess015));
		sleep (sleepTime);
		clear_mess ();
	}
}


/*
 * Function     :   SrchCrbk (char* key_val)
 *
 * Description  :   Search for Bank Id file.
 *
 * Parameters   :   char* key_val
 *
 * Return       :   None.
 */
void
SrchCrbk (
	char	*keyValue)
{
	_work_open (5,0,40);
	strcpy (crbk_rec.co_no,  comm_rec.co_no);
	sprintf (crbk_rec.bank_id,  "%-5.5s", keyValue);
	save_rec ("#Bank Id", "#Bank Name");
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc &&
           !strcmp (crbk_rec.co_no, comm_rec.co_no) &&
           !strncmp (crbk_rec.bank_id, keyValue, strlen (keyValue)))
	{
		cc = save_rec (crbk_rec.bank_id, crbk_rec.bank_name);
		if (cc)
        {
			break;
        }
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
/*
 * Function     :   heading (int scn)
 *
 * Description  :
 *
 * Parameters   :   int scn
 *
 * Return       :   0 - Ok; 1 - Error
 */
int
heading (
 int scn)
{
    if (restart)
		return (EXIT_FAILURE);
    
	scn_set (scn);    
	clear ();
	rv_pr (ML (mlCaMess033),19,1,1);
	
	box (0,3,80,9);
	line_at (10,1,79);
	line_at (2,1,79);

	strcpy (err_str,ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str,ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no, comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);   
    
    return (EXIT_SUCCESS);
}
