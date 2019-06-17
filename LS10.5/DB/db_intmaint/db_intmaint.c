/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_intmaint.c,v 5.1 2001/12/07 03:51:32 scott Exp $
|  Program Name  : (db_intmaint.c)
|  Program Desc  : (Customer Interest Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_intmaint.c,v $
| Revision 5.1  2001/12/07 03:51:32  scott
|
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_intmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_intmaint/db_intmaint.c,v 5.1 2001/12/07 03:51:32 scott Exp $";

#define	MAXSCNS 1
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

int 	envVarDbCo 		= 0,
		envVarDbFind 	= 0;

	char	branchNumber [3];

	double	tot_int = 0.00,
			tot_out = 0.00;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

/*============================ 
| Local & Screen Structures. |
============================*/ 
struct {
	char dummy [11];
	char dbt_no [7];
	float int_percent;
} local_rec;

	char	*data = "data";

static	struct	var	vars []	={	

	{1, LIN, "dbtrno", 4, 23, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Customer No.", " ", 
		NE, NO, JUSTLEFT, "", "", local_rec.dbt_no}, 
	{1, LIN, "name", 5, 23, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Name  ", " ", 
		NA, NO, JUSTLEFT, "", "", cumr_rec.dbt_name}, 
	{1, LIN, "int_rate", 7, 23, FLOATTYPE, 
		"NNN.NNN", "          ", 
		" ", "", "Interest Rate ", " ", 
		NA, NO, JUSTLEFT, "", "", (char *)&local_rec.int_percent}, 
	{1, LIN, "interest", 8, 23, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Interest Y(es) N(o) ", " ", 
		YES, NO, JUSTLEFT, "YN", "", cumr_rec.int_flag}, 
	{1, LIN, "tot_out", 9, 23, MONEYTYPE, 
		"NNNNNNNNNN.NN", "          ", 
		" ", "", "Total Amount Overdue.", " ", 
		NA, NO, JUSTLEFT, "", "", (char *)&tot_out}, 
	{1, LIN, "int_due", 10, 23, MONEYTYPE, 
		"NNNNNNNNNN.NN", "          ", 
		" ", "", "Interest Due.", " ", 
		NA, NO, JUSTLEFT, "", "", (char *)&tot_int}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	Update 				(void);
int 	heading 			(int);
void 	CalculateInterest 	(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	SETUP_SCR (vars);

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/

	prog_exit = FALSE;
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");


	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (!prog_exit )
	{
		restart     = FALSE;
        search_ok   = TRUE;
        init_ok     = TRUE;
        entry_exit  = FALSE;
        edit_exit   = FALSE;
        prog_exit   = FALSE;

		init_vars (1);

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
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

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no"
										: "cumr_id_no3");
	open_rec ("comr", comr_list, COMR_NO_FIELDS, "comr_co_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose ("cumr");
	abc_dbclose ("dbtr");
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*------------------------
	| Validate Customer number |
	------------------------*/
	if (LCHECK ("dbtrno"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		clear_mess ();
		abc_selfield ("cumr", (!envVarDbFind)? "cumr_id_no "  : "cumr_id_no3");
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.dbt_no));
		cc = find_rec ("cumr", &cumr_rec, COMPARISON, "u");
		if (cc) 
		{
			errmess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}

		tot_out = cumr_balance [1] + 
				  cumr_balance [2] +
				  cumr_balance [3] +
				  cumr_balance [4] +
				  cumr_balance [5];

		local_rec.int_percent = comr_rec.int_rate;
		if (cumr_rec.int_flag [0] == 'Y')
			CalculateInterest ();

		display_field (field + 1);
		display_field (field + 2);
		display_field (field + 3);
		entry_exit = 1;
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Interest flag. |
	-------------------------*/
	if (LCHECK ("interest"))
	{
		tot_out = cumr_balance [1] + 
				  cumr_balance [2] +
				  cumr_balance [3] +
				  cumr_balance [4] +
				  cumr_balance [5];

		if (cumr_rec.int_flag [0] == 'Y')
			CalculateInterest ();
		else
			tot_int = 0.00;

		display_field (field + 1);
		display_field (field + 2);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
Update (void)
{
	clear ();
	print_at (21,0,ML (mlStdMess035));
	fflush (stdout);

	cc = abc_update ("cumr", &cumr_rec);
	if (cc)
		file_err (cc, "cumr", "DBUPDATE");
}

int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlDbMess171),20,0,1);

		box (0,3,80,7);

		line_at (1,0,80);
		line_at (6,1,79);
		line_at (20,0,80);
		print_at (21, 0, ML (mlStdMess038),comm_rec.co_no, comm_rec.co_name );
		print_at (22, 0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*=======================================================
| Calculate total Interest due on outstanding balance . |
=======================================================*/
void
CalculateInterest (void)
{
	double	total_bal = 0.00;

	total_bal = cumr_balance [1] + 
				cumr_balance [2] +
				cumr_balance [3] +
				cumr_balance [4] +
				cumr_balance [5];

	if (total_bal == 0.00)
		return;
	
	tot_int = (total_bal / 100) * (double) local_rec.int_percent;
	tot_int /= 12;
	
	if (tot_int <= 0.00)
		tot_int = 0.00;

	return;
}

