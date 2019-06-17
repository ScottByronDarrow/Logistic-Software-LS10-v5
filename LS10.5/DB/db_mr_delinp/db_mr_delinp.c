/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_mr_delinp.c,v 5.8 2002/07/24 08:38:49 scott Exp $
|  Program Name  : (db_mr_delinp.c)
|  Program Desc  : (Customer Delete Input)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_mr_delinp.c,v $
| Revision 5.8  2002/07/24 08:38:49  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/19 04:01:24  scott
| .
|
| Revision 5.6  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/07/16 01:04:21  scott
| Updated from service calls and general maintenance.
|
| Revision 5.4  2002/06/26 04:34:18  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/06/26 04:26:52  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/02/01 06:40:46  robert
| SC 00750 - fixed memory dump
|
| Revision 5.1  2001/11/26 05:11:43  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_mr_delinp.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_mr_delinp/db_mr_delinp.c,v 5.8 2002/07/24 08:38:49 scott Exp $";

#define		TABLINES	14
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

	/*
	 * Special fields and flags.
	 */
   	int		envDbCo 		 = 0, 
			envDbFind 		 = 0, 
			pid				 = 0, 
			workNo			 = 0;

		char	branchNo 	[3];

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cohrRecord	cohr_rec;
struct sohrRecord	sohr_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;

	char	*cumr2	=	"cumr2";

	/*
	 * Work file record for deletion.
	 */
	struct {
		long	hhcuHash;	
	} customerRec;

	struct	storeRec {
		char	customerNo	[sizeof	cumr_rec.dbt_no];
	} store [MAXLINES];

/*
 * Local & Screen Structures.
 */
struct {
	char 	dummy [11];
	char	customerNo		[sizeof	cumr_rec.dbt_no];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "customerNo", 	MAXLINES, 3, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", " Customer No ", "", 
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNo}, 
	{1, TAB, "customerName", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "             Customer Name              ", "", 
		NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{1, TAB, "customerHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "", "", 
		ND, NO,  JUSTLEFT, "", "", (char *)&cumr_rec.hhcu_hash}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy}, 
};

#include <FindCumr.h>
/*
 * Local Function Prototypes
 */
void 	shutdown_prog 			(void);
void 	CloseDB 				(void);
void 	Update 					(void);
int 	heading 				(int);
int 	OpenDB 					(void);
int 	spec_valid 				(int);
int 	CheckCohr 				(void);
int		CheckDuplicateCustomer 	(char *, int);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc, 
	char	*argv [])
{
	int		i;

	SETUP_SCR (vars);


	tab_row = 3;
	tab_col = 10;

	if (argc < 2)
	{
		print_at (0, 0, mlStdMess046, argv [0]); 
		return (EXIT_FAILURE);
	}

	envDbFind 	= atoi (get_env ("DB_FIND"));
	envDbCo 	= atoi (get_env ("DB_CO"));

	init_scr 	();	
	set_tty 	();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);

	pid = atoi (argv [1]);

	if (OpenDB () == 1)
    {
        return (EXIT_FAILURE);
    }
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	restart = 0;

	while (prog_exit == 0)
	{
		/*  reset control flags  */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/
		lcount [1]	= 0;

		for (i = 0; i < MAXLINES; i++)
			strcpy (store [i].customerNo, "      ");

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence.
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files .
 */
int
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (filename, "%s/WORK/cudl%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	abc_alias (cumr2, cumr);

	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhcu_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhcu_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhcu_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");

	if (RF_OPEN (filename, sizeof (customerRec), "w", &workNo))
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_dbclose ("data");

	RF_CLOSE (workNo);
}

int
spec_valid (
	int		field)
{
	double	balance	=	0.00;

	/*
	 * Validate Customer Number Input And All its Conditions.
	 */
	if (LCHECK ("customerNo")) 
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.customerNo, 6));
		strcpy (cumr_rec.est_no, branchNo);

		/*
		 * Find Customer.
		 */
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "w");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Get Total Balance On Customer Ledger Card.
		 */
		balance =  cumr_rec.bo_current +
		   cumr_rec.bo_per1 +
		   cumr_rec.bo_per2 +
		   cumr_rec.bo_per3 +
		   cumr_rec.bo_per4 +
		   cumr_rec.bo_fwd;

		 /*
		  * Total Balance Is Greater Than Zero.
		  */
		if (balance != 0.0) 
		{
			sprintf (err_str, ML (mlDbMess173), DOLLARS(balance));
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check If Customer is Head Office Customer 
		 */
		cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, COMPARISON, "r");
		if (!cc)
		{
			errmess (ML (mlDbMess174));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check Is Customer Has Invoices On File. 
		 */
		cuin_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cuin, &cuin_rec, EQUAL, "r");
		if (!cc) 
		{
			errmess (ML (mlDbMess175));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		/*
		 * Check Is Customer Has Cheques On File. 
		 */
		cuhd_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
		if (!cc) 
		{
			errmess (ML (mlDbMess176));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check If Customer Has Orders On File. 
		 */
		sohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
		cc = find_rec (sohr, &sohr_rec, EQUAL, "r");
		if (!cc) 
		{
			errmess (ML (mlDbMess177));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (CheckCohr ())
		{
			errmess (ML (mlDbMess178));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (CheckDuplicateCustomer (local_rec.customerNo, line_cnt))
		{
			errmess (ML (mlStdMess096));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].customerNo, local_rec.customerNo);
		DSP_FLD ("customerName");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check for Active invoices.
 */
int
CheckCohr (void)
{
	cohr_rec.hhcu_hash	=	cumr_rec.hhcu_hash;
	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cohr_rec.hhcu_hash)
	{
		if (cohr_rec.stat_flag [0] != 'D' && cohr_rec.stat_flag [0] != '9')
			return (EXIT_FAILURE);

		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Update to work file. 
 */
void
Update (void)
{
	clear ();

	abc_selfield (cumr, "cumr_hhcu_hash");

	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++) 
	{
		getval (line_cnt);
				
		cc = find_rec (cumr, &cumr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		strcpy (cumr_rec.stat_flag, "9");
		cc = abc_update (cumr, &cumr_rec);
		if (cc) 
			file_err (cc, cumr, "DBUPDATE");
			
		customerRec.hhcuHash = cumr_rec.hhcu_hash;
		cc = RF_ADD (workNo, (char *) &customerRec);
		if (cc) 
			file_err (cc, "workNo", "RF_ADD");
	}	/* end of 'for' loop	*/
	abc_selfield (cumr, "cumr_id_no");

	prog_exit = 1;
}

/*
 * Check whether Customer has been input before.
 * Return 1 if duplicate					       
 */
int
CheckDuplicateCustomer (
	char	*customerNo,
	int		lineNumber)
{
	int		i;
	int		noCustomers = (prog_status == ENTRY) ? line_cnt : lcount [1];

	for (i = 0;i < noCustomers;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == lineNumber)
			continue;

		/*
		 * cannot be duplicate if not input.
		 */
		if (!strcmp (store [i].customerNo, "      "))
			continue;

		if (!strcmp (store [i].customerNo, customerNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
int
heading (
	int		scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlDbMess111), 26, 0, 1);
	line_at (1,0,80);
	line_at (20,0,80);
	print_at (21,0,  ML (mlStdMess038),	comm_rec.co_no, comm_rec.co_short);
	print_at (22,0,  ML (mlStdMess039),	comm_rec.est_no,comm_rec.est_short);
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

