/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_lettpro.i.c,v 5.2 2002/07/17 09:57:07 scott Exp $
|  Program Name  : (db_lettpro.i.c)
|  Program Desc  : (Selection for customers Overdue Letters)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/01/87         |
|---------------------------------------------------------------------|
| $Log: db_lettpro.i.c,v $
| Revision 5.2  2002/07/17 09:57:07  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/12/04 00:09:24  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_lettpro.i.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_lettpro.i/db_lettpro.i.c,v 5.2 2002/07/17 09:57:07 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>		
#include <ml_std_mess.h>
#include <ml_db_mess.h>		

	/*
	 * Special fields and flags.
	 */
	                        
	char	directoryName [60];

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	char	*data = "data";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	startCustomerNo [7];
	char	endCustomerNo [7];
	char	customerName [2][41];
	char	back [2];
	char	backDesc [11];
	char	onight [2];
	char	onightDesc [11];
	char	letterNo [2];
	double	letterAmount;
	int		printerNo;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN,  "startCustomerNo",  3,  2,  CHARTYPE,  
		"UUUUUU", "          ",  
		" ", " ",  "Start Customer   ",  " ",  
		YES, NO,  JUSTLEFT,  "",  "",  local_rec.startCustomerNo},  
	{1, LIN,  "startCustomerName",  3,  30,  CHARTYPE,  
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",  
		" ", " ",  " ",  " ",  
		NA, NO,  JUSTLEFT,  "",  "",  local_rec.customerName [0]},  
	{1, LIN,  "endCustomerNo",  4,  2,  CHARTYPE,  
		"UUUUUU", "          ",  
		" ", " ",  "End Customer     ",  " ",  
		YES, NO,  JUSTLEFT,  "",  "",  local_rec.endCustomerNo},  
	{1, LIN,  "endCustomerName",  4,  30,  CHARTYPE,  
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",  
		" ", " ",  " ",  " ",  
		NA, NO,  JUSTLEFT,  "",  "",  local_rec.customerName [1]},  
	{1, LIN,  "printerNo",  5,  2,  INTTYPE,  
		"NN", "          ",  
		" ", "1",  "Printer number   ",  " ",  
		YES, NO,  JUSTRIGHT,  "",  "",  (char *)&local_rec.printerNo},  
	{1, LIN,  "back",  6,  2,  CHARTYPE,  
		"U", "          ",  
		" ", "N","Background       ",  "Y(es or N(o. ",  
		YES, NO,  JUSTRIGHT,  "YN",  "",  local_rec.back},  
	{1, LIN,  "backDesc",  6,  30,  CHARTYPE,  
		"AAAAAAAAAA", "          ",  
		" ", "","",  "",  
		NA, NO,  JUSTLEFT,  "YN",  "",  local_rec.backDesc},  
	{1, LIN,  "onight",  7,  2,  CHARTYPE,  
		"U", "          ",  
		" ", "N",  "Overnight        ",  "Y(es or N(o. ",  
		YES, NO,  JUSTRIGHT,  "YN",  "",  local_rec.onight},  
	{1, LIN,  "onightDesc",  7,  30,  CHARTYPE,  
		"AAAAAAAAAA", "          ",  
		" ", "","",  "",  
		NA, NO,  JUSTLEFT,  "YN",  "",  local_rec.onightDesc},  
	{1, LIN,  "letno",  9,  2,  CHARTYPE,  
		"U", "          ",  
		" ", "0",  "Process Letter No (0-4)          ",  "Enter 0 for All.",  
		YES, NO,  JUSTLEFT,  "01234",  "",  local_rec.letterNo},  
	{1, LIN,  "letamt",  10,  2,  DOUBLETYPE,  
		"NNNNNNNN.NN", "          ",  
		" ", "0.00",  "Process Customers Greater-than   ",  "Enter Amount.",  
		YES, NO,  JUSTLEFT,  "0.00",  "99999999.99",  (char *)&local_rec.letterAmount},  
	{0, LIN,  "",  0,  0,  INTTYPE,  
		"A", "          ",  
		" ", "",  "dummy",  " ",  
		YES, NO,  JUSTRIGHT,  "",  ""},  
};

#include <FindCumr.h>
/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
int 	spec_valid 		(int);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	RunProgram 		(void);
int 	heading 		(int);

	int		envVarDbCo		=	0;
	char	branchNumber [3];
	extern	int	TruePosition;

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv [])
{
	char	*sptr;
	if (argc != 2)
	{
		print_at (0, 0, mlDbMess116, argv [0]);
        return (EXIT_FAILURE);
	}
	sprintf (directoryName, "%-.59s", argv [1]); 

	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	init_scr ();			
	set_tty ();            
	set_masks ();		
	init_vars (1);	

	OpenDB ();

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);	

		/*
		 * Edit screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit) 
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		prog_exit = 1;
		if (restart) 
			continue;
		
		if (!restart) 
			RunProgram ();
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

int
spec_valid (int field)
{
	/*
	 * Validate Start Customer
	 */
	if (LCHECK ("startCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.customerName [0], ML ("Start Customer"));
			DSP_FLD ("startCustomerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber,  temp_str);
			return (EXIT_SUCCESS);
		}  
		
        if (prog_status != ENTRY &&
		    strcmp (local_rec.startCustomerNo, local_rec.endCustomerNo) > 0)
	 	{ 
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

        strcpy (cumr_rec.co_no, comm_rec.co_no);
        strcpy (cumr_rec.est_no, branchNumber);
        strcpy (cumr_rec.dbt_no, pad_num (local_rec.startCustomerNo));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.customerName [0], cumr_rec.dbt_name);
		DSP_FLD ("startCustomerName");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate End Customer
	 */
	if (LCHECK ("endCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.customerName [1], ML ("End Customer"));
			DSP_FLD ("endCustomerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber,  temp_str);
			return (EXIT_SUCCESS);
		}
		
        strcpy (cumr_rec.co_no, comm_rec.co_no);
        strcpy (cumr_rec.est_no, branchNumber);
        strcpy (cumr_rec.dbt_no, pad_num (local_rec.endCustomerNo));
	    cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startCustomerNo, local_rec.endCustomerNo) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.customerName [1], "%-30.30s", cumr_rec.dbt_name);
		DSP_FLD ("endCustomerName");
		return (EXIT_SUCCESS);
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
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Field Selection background option.
	 */
	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, 
					(local_rec.back [0] == 'Y') ? ML ("Y(es") : ML ("N(o"));

		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}
	
	/*
	 * Validate Field Selection overnight option.
	 */
	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, 
				(local_rec.onight [0] == 'Y') ? ML ("Y(es") : ML ("N(o"));

		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data); 

	read_comm (comm_list, COMM_NO_FIELDS,  (char *) &comm_rec);
	open_rec (cumr, cumr_list,  CUMR_NO_FIELDS,  "cumr_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cumr);

	abc_dbclose ("data"); 
}

void
RunProgram (void)
{
	rset_tty ();
	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	/*
	 * Test for Overnight Processing. 
	 */
	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str,
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%s\" \"%d\" \"%s\" \"%.2f\" \"%s\"",
			"db_lettpro", 
			directoryName, 
			local_rec.startCustomerNo, 
			local_rec.endCustomerNo, 
			local_rec.printerNo, 
			local_rec.letterNo, 
			local_rec.letterAmount,
			ML (mlDbMess233)
		);
		SystemExec (err_str, TRUE);
	}
	/*
	 * Test for forground or background.
	 */
	else
	{
		sprintf 
		(
			err_str,
			"\"%s\" \"%s\" \"%s\" \"%s\" \"%d\" \"%s\" \"%.2f\"",
			"db_lettpro", 
			directoryName, 
			local_rec.startCustomerNo, 
			local_rec.endCustomerNo, 
			local_rec.printerNo, 
			local_rec.letterNo, 
			local_rec.letterAmount
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
	CloseDB (); 
	FinishProgram ();
}

/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input                       
 */
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlDbMess107), 20, 0, 1);

		line_at (8,1,79);
		line_at (1,0,80);

		if (scn == 1)
			box (0, 2, 80, 8);

		line_at (20,0,80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

