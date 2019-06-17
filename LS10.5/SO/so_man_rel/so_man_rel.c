/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: so_man_rel.c,v 5.4 2002/07/18 07:18:25 scott Exp $
|  Program Name  : (so_man_rel.c)
|  Program Desc  : (Selected Release Of Manual Sales Orders)
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap    | Date Written  : 09/11/88         |
|---------------------------------------------------------------------|
| $Log: so_man_rel.c,v $
| Revision 5.4  2002/07/18 07:18:25  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/04/30 07:56:49  scott
| Update for new Archive modifications;
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_man_rel.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_man_rel/so_man_rel.c,v 5.4 2002/07/18 07:18:25 scott Exp $";

#define	MAXWIDTH	140
#define	MAXLINES	10
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#define		ONE_STEP	 (updateFlag [0] == 'R')

	int	envDbCo = 0;
	int	envDbFind = 0;
	int	lpno = 0;

	char	findFlag [2];
	char	updateFlag [2];
	char	branchNo [3];

	char	*data = "data";

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct cumrRecord	cumr_rec;
struct sobgRecord	sobg_rec;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	order_no [2][9];
	char	cust_no [7];
	char	cust_name [41];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "st_order_no", MAXLINES, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "ALL   ", "S/O From. ", " Key ALL for all Orders ", 
		YES, NO, JUSTLEFT, "", "", local_rec.order_no [0]}, 
	{1, TAB, "end_order_no", 0, 2, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "ALL   ", "S/O   To. ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.order_no [1]}, 
	{1, TAB, "cust_no", 0, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", " Cust No. ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cust_no}, 
	{1, TAB, "cust_name", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "               Customer Name              ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cust_name}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};


/*
 * Function Declarations 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	PrintALL 		(void);
int  	DeleteLine 		(void);
void 	SrchSohr 		(char *, char *, char *);
void 	Update 			(void);
int  	UpdateSolb 		(long);
void 	DeleteColn 		(long);
int  	heading 		(int);
void 	AddSobg 		(int, char *, long);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc, 
 char	*argv [])
{
	if (argc != 3 && argc != 4)
	{
		print_at (0, 0, mlSoMess723, argv [0]);
		return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sprintf (findFlag, "%-1.1s", argv [1]);
	sprintf (updateFlag, "%-1.1s", argv [2]);

	if (argc == 4)
		lpno = atoi (argv [3]);

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	init_scr ();
	set_tty (); 
	set_masks ();

	while (prog_exit == 0)
	{
		search_ok = 1;
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		init_ok = 1;
		init_vars (1);
		lcount [1] = 0;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		if (lcount [1] != 0)
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
	FinishProgram ();
}

/*
 * Open Database Files. 
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	open_rec (sobg, sobg_list, SOBG_NO_FIELDS, "sobg_id_no");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (sohr);
	abc_fclose (soln);
	abc_fclose (cumr);
	abc_fclose (coln);
	abc_fclose (sobg);
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	char	low_ord [9];
	char	high_ord [9];

	if (LCHECK ("st_order_no"))
	{
		if (dflt_used)
			return (DeleteLine ());

		if (!strcmp (local_rec.order_no [0], "ALL     "))
		{
			PrintALL ();
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.order_no [0], zero_pad (local_rec.order_no [0], 8));

		strcpy (low_ord, "        ");
		strcpy (high_ord, (prog_status == ENTRY) ? "~~~~~~~~" : local_rec.order_no [1]);
		if (SRCH_KEY)
		{
			SrchSohr (low_ord, temp_str, high_ord);
			return (EXIT_SUCCESS);
		}

		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		sprintf (sohr_rec.order_no, local_rec.order_no [0]);
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (cc || sohr_rec.status [0] != findFlag [0])
		{
			if (cc)
				strcpy (err_str, ML (mlStdMess102));
			else
				strcpy (err_str, ML (mlSoMess183));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (sohr_rec.order_no, high_ord) > 0)
		{
			sprintf (err_str, ML (mlSoMess184), local_rec.order_no [0], high_ord);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY && !strcmp (local_rec.order_no [0], local_rec.order_no [1]))
		{
			cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			strcpy (local_rec.cust_no, (cc) ? "      " : cumr_rec.dbt_no);
			sprintf (local_rec.cust_name, "%-40.40s", (cc) ? " " : cumr_rec.dbt_name);
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
		}
		else
		{
			if (prog_status != ENTRY && strcmp (local_rec.order_no [0], local_rec.order_no [1]))
			{
				sprintf (local_rec.cust_no, "      ");
				sprintf (local_rec.cust_name, "%s to %s", local_rec.order_no [0], local_rec.order_no [1]);
				DSP_FLD ("cust_no");
				DSP_FLD ("cust_name");
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("end_order_no"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.order_no [1], local_rec.order_no [0]);
			DSP_FLD ("end_order_no");
		}

		if (!strcmp (local_rec.order_no [1], "ALL     ") || !strcmp (local_rec.order_no [1], "~~~~~~~~"))
		{
			PrintALL ();
			return (EXIT_SUCCESS);
		}
		vars [label ("end_order_no")].required = YES;

		strcpy (local_rec.order_no [1], zero_pad (local_rec.order_no [1], 8));

		strcpy (low_ord, local_rec.order_no [0]);
		strcpy (high_ord, "~~~~~~~~");

		if (SRCH_KEY)
		{
			SrchSohr (low_ord, temp_str, high_ord);
			return (EXIT_SUCCESS);
		}

		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		sprintf (sohr_rec.order_no, local_rec.order_no [1]);
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
		if (cc || sohr_rec.status [0] != findFlag [0])
		{
			if (cc)
				strcpy (err_str, ML (mlStdMess102));
			else
				strcpy (err_str, ML (mlSoMess183));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (strcmp (sohr_rec.order_no, low_ord) < 0)
		{
			sprintf (err_str, ML (mlSoMess185), local_rec.order_no [1], low_ord);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!strcmp (local_rec.order_no [0], local_rec.order_no [1]))
		{
			cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
			strcpy (local_rec.cust_no, (cc) ? "      " : cumr_rec.dbt_no);
			sprintf (local_rec.cust_name, "%-40.40s", (cc) ? " " : cumr_rec.dbt_name);
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
		}
		else
		{
			sprintf (local_rec.cust_no, "      ");
			sprintf (local_rec.cust_name, "%s to %s", local_rec.order_no [0], local_rec.order_no [1]);
			DSP_FLD ("cust_no");
			DSP_FLD ("cust_name");
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
PrintALL (
 void)
{
	sprintf (local_rec.order_no [0], "%-8.8s", "00000000");
	sprintf (local_rec.order_no [1], "%-8.8s", "~~~~~~~~");
	strcpy (local_rec.cust_no, "      ");
	sprintf (local_rec.cust_name, "%-40.40s", ML ("All Orders"));
	vars [label ("end_order_no")].required = NA;
	DSP_FLD ("st_order_no");
	DSP_FLD ("end_order_no");
	DSP_FLD ("cust_no");
	DSP_FLD ("cust_name");
}

int
DeleteLine (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
/*
		print_mess (" Cannot Delete Lines on Entry ");*/
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [1]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	strcpy (local_rec.order_no [0], "        ");
	strcpy (local_rec.order_no [1], "        ");
	strcpy (local_rec.cust_no, "      ");
	sprintf (local_rec.cust_name, "%-40.40s", " ");

	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Search for order number.
 */
void
SrchSohr (
	char	*low_ord, 
	char	*key_val, 
	char	*high_ord)
{
	_work_open (8,0,30);
	save_rec ("#SO No.", "#Customer Order No.   ");

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", key_val);

	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && !strncmp (sohr_rec.order_no, key_val, strlen (key_val)) &&
		      !strcmp (sohr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sohr_rec.br_no, comm_rec.est_no))
	{
		if (strcmp (sohr_rec.order_no, low_ord) >= 0 && 
		     strcmp (sohr_rec.order_no, high_ord) <= 0 && 
		    (sohr_rec.status [0] == findFlag [0]))
		{
			cc = save_rec (sohr_rec.order_no, sohr_rec.cus_ord_ref);
			if (cc)
				break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in sohr during (DBFIND)", cc, PNAME);
}

void  
Update (void)
{
	print_at (0, 0, ML (mlStdMess035));

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);

	for (line_cnt = 0;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt);

		sprintf (sohr_rec.order_no, local_rec.order_no [0]);
		if (!strcmp (local_rec.order_no [0], local_rec.order_no [1]))
			cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
		else
			cc = find_rec (sohr, &sohr_rec, GTEQ, "u");

		while (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no) && 
		  	      !strcmp (sohr_rec.br_no, comm_rec.est_no) && 
			      strcmp (sohr_rec.order_no, local_rec.order_no [1]) <= 0)
		{
			if (sohr_rec.status [0] == findFlag [0])
			{
				if (UpdateSolb (sohr_rec.hhso_hash))
				{
					strcpy (sohr_rec.status, updateFlag);
					strcpy (sohr_rec.stat_flag, updateFlag);
					cc = abc_update (sohr, &sohr_rec);
					if (cc)
						sys_err ("Error in sohr during (DBUPDATE)", cc, PNAME);
					abc_unlock (sohr);

					if (ONE_STEP)
						AddSobg (lpno, (lpno) ? "PA" : "PC", sohr_rec.hhso_hash);
				}
				if (!strcmp (local_rec.order_no [0], local_rec.order_no [1]))
				{
					abc_unlock (sohr);
					break;
				}
			}
			abc_unlock (sohr);
			cc = find_rec (sohr, &sohr_rec, NEXT, "u");
		}
		abc_unlock (sohr);
	}
	abc_unlock (sohr);
}

int
UpdateSolb (
	long	hhsoHash)
{
	int	updateSoln = 0;

	soln_rec.hhso_hash = hhsoHash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.status [0] == findFlag [0])
		{
			DeleteColn (soln_rec.hhsl_hash);
			strcpy (soln_rec.status, updateFlag);
			strcpy (soln_rec.stat_flag, updateFlag);
			cc = abc_update (soln, &soln_rec);
			if (cc)
				sys_err ("Error in soln during (DBUPDATE)", cc, PNAME);
			updateSoln = 1;
		}
		abc_unlock (soln);
		cc = find_rec (soln, &soln_rec, NEXT, "u");
	}
	abc_unlock (soln);

	return (updateSoln);
}

/*
 * Delete line from existing packing slip so new backorder can be released. 
 */
void
DeleteColn (
	long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (coln);
		return;
	}
	if (coln_rec.status [0] == 'P')
		abc_delete (coln);
	else
		abc_unlock (coln);

	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		clear ();

		rv_pr (ML (mlSoMess186), 20, 0, 1);

		line_at (1,0,80);
		line_at (20,0,80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22, 0, err_str, comm_rec.est_no, comm_rec.est_short);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*
 * Add record to background processing file. 
 */
void
AddSobg (
	int _lpno, 
	char *_type_flag, 
	long _hash)
{
	strcpy (sobg_rec.co_no, comm_rec.co_no);
	strcpy (sobg_rec.br_no, comm_rec.est_no);
	strcpy (sobg_rec.type, _type_flag);
	sobg_rec.lpno = _lpno;
	sobg_rec.hash = _hash;

	cc = find_rec (sobg, &sobg_rec, COMPARISON, "r");
	/*
	 * Add the record if an identical one doesn't already exist     
	 */
	if (cc)
	{
		strcpy (sobg_rec.co_no, comm_rec.co_no);
		strcpy (sobg_rec.br_no, comm_rec.est_no);
		strcpy (sobg_rec.type, _type_flag);
		sobg_rec.lpno = _lpno;
		sobg_rec.hash = _hash;

		cc = abc_add (sobg, &sobg_rec);
		if (cc)
			sys_err ("Error in sobg during (DBADD)", cc, PNAME);

	}
}
