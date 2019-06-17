/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sj_ldmaint.c,v 5.4 2002/07/24 08:39:11 scott Exp $
|  Program Name  : (sj_ldmaint.c)
|  Program Desc  : (Enter job card details)
|---------------------------------------------------------------------|
|  Author        : Lance Whitford  | Date Written  : 21/08/87         |
|---------------------------------------------------------------------|
| $Log: sj_ldmaint.c,v $
| Revision 5.4  2002/07/24 08:39:11  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 05:18:59  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sj_ldmaint.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SJ/sj_ldmaint/sj_ldmaint.c,v 5.4 2002/07/24 08:39:11 scott Exp $";

#define MAXLINES	10
#define TABLINES	10
#define MAXWIDTH	150

#include <ml_sj_mess.h>
#include <ml_std_mess.h>
#include <pslscr.h>
#include <std_decs.h>

#include	"schema"

struct commRecord	comm_rec;
struct sjldRecord	sjld_rec;
struct sjhrRecord	sjhr_rec;
struct sjsrRecord	sjsr_rec;
struct sjlrRecord	sjlr_rec;
struct sjvhRecord	sjvh_rec;
struct cumrRecord	cumr_rec;

struct storeRec {
		char	cli_desc [41];
		char	cli_no [7];
		char	ex_code1 [3];
		char	ex_desc1 [27];
		char	ex_code2 [3];
		char	ex_desc2 [27];
		char	ex_code3 [3];
		char	ex_desc3 [27];
		char	ex_code4 [3];
		char	ex_desc4 [27];
} store [MAXLINES];

	int	clear_box = FALSE;
		
/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11], 
			vehicleCode [4], 
			ex_code [4][3];
	float	km;
	int		ex_cnt;
	long	order_no;
	float   tot_ord, 
			tot_t15, 
			tot_t2, 
			ord, 
			t15, 
			t2, 
			ex_hrs [4];
	double	vh_rates;
	double	ex_rates [4];
} local_rec;

char	systemDate [11];

static	struct	var	vars []	={	

	{1, LIN, "employee", 4, 20, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "Employee Code ", " ", 
		YES, NO, JUSTLEFT, "", "", sjld_rec.emp_code}, 
	{1, LIN, "emp_name", 4, 45, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "-", " ", 
		NA, NO, JUSTLEFT, "", "", sjsr_rec.name}, 
	{1, LIN, "date", 5, 20, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", systemDate, "Date ", " ", 
		YES, NO, JUSTLEFT, "", "", (char *)&sjld_rec.date}, 
	{1, LIN, "tot_ord", 6, 20, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Total Ordinary ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_ord}, 
	{1, LIN, "tot_1.5", 7, 20, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Total T1/2 ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_t15}, 
	{1, LIN, "tot_2", 8, 20, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", "Total Double ", " ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.tot_t2}, 
	{2, TAB, "service_no", MAXLINES, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "", "S.Job#", "Please enter service Job # [SEARCH]", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.order_no}, 
	{2, TAB, "vehicle", 0, 0, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "Veh", "Please enter vehicle [SEARCH] ", 
		NO, NO, JUSTLEFT, "", "", local_rec.vehicleCode}, 
	{2, TAB, "veh_desc", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " Vehicle Description", "", 
		NA, NO, JUSTLEFT, "", "", sjvh_rec.vehicle}, 
	{2, TAB, "Km", 0, 0, FLOATTYPE, 
		"NNNN.NN", "          ", 
		" ", "0", "   Km   ", "Please enter total Km.", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.km}, 
	{2, TAB, "tm_1.0", 0, 1, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", " 1.0xT  ", "Please enter hours at Standard time. ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.ord}, 
	{2, TAB, "tm_1.5", 0, 1, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", " 1.5xT  ", "Please enter hours at time & 1/2.", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.t15}, 
	{2, TAB, "tm_2.0", 0, 1, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "0", " 2.0xT  ", "Please enter hours at Double time. ", 
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.t2}, 
	{2, TAB, "extra_code", 0, 1, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Ext (1)", "Please enter extra hours Code (1). [SEARCH] ", 
		YES, NO, JUSTLEFT, "", "", local_rec.ex_code [0]}, 
	{2, TAB, "extra_hrs", 0, 1, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", "0", "  Hrs  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_hrs [0]}, 
	{2, TAB, "ex_rate", 0, 1, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " ", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_rates [0]}, 
	{2, TAB, "extra_code", 0, 1, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Ext (2)", "Please enter extra hours Code (2). [SEARCH] ", 
		YES, NO, JUSTLEFT, "", "", local_rec.ex_code [1]}, 
	{2, TAB, "extra_hrs", 0, 1, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", "0", "  Hrs  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_hrs [1]}, 
	{2, TAB, "ex_rate", 0, 1, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " ", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_rates [1]}, 
	{2, TAB, "extra_code", 0, 1, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Ext (3)", "Please enter extra hours Code (3). [SEARCH] ", 
		YES, NO, JUSTLEFT, "", "", local_rec.ex_code [2]}, 
	{2, TAB, "extra_hrs", 0, 1, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", "0", "  Hrs  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_hrs [2]}, 
	{2, TAB, "ex_rate", 0, 1, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " ", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_rates [2]}, 
	{2, TAB, "extra_code", 0, 1, CHARTYPE, 
		"UU", "          ", 
		" ", " ", "Ext (4)", "Please enter extra hours Code (4). [SEARCH]. ", 
		YES, NO, JUSTLEFT, "", "", local_rec.ex_code [3]}, 
	{2, TAB, "extra_hrs", 0, 1, FLOATTYPE, 
		"NN.NN", "          ", 
		" ", "0", "  Hrs  ", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_hrs [3]}, 
	{2, TAB, "ex_rate", 0, 1, DOUBLETYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " ", " ", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.ex_rates [3]}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};

/*
 * Function Prototypes   
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	UpdateSjld 		(void);
void 	SrchSjvh 		(char *);
void 	SrchSjsr 		(char *);
void 	SrchSjhr 		(char *);
void 	SrchSjlr 		(char *);
void 	tab_other 		(int);
int 	heading 		(int);
int 	spec_valid 		(int);
int 	CheckTotals 	(void);
int 	Update 			(void);

/*
 * Main Processing Routine 
 */
int
main (
	int		argc, 
	char 	*argv [])
{
	int	i;
	SETUP_SCR (vars);


	tab_row = 8;

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	
	swide ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	strcpy (systemDate, DateToString (TodaysDate ()));

	OpenDB ();

	/*
	 * Beginning of input control loop 
	 */
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
		clear_box 	= FALSE;

		for (i = 0 ; i < MAXLINES; i++)
		{
			sprintf (store [i].cli_desc, "%-40.40s", " ");
			sprintf (store [i].cli_no, "%-6.6s", " ");
			sprintf (store [i].ex_code1, "%-2.2s", " ");
			sprintf (store [i].ex_desc1, "%-26.26s", " ");
			sprintf (store [i].ex_code2, "%-2.2s", " ");
			sprintf (store [i].ex_desc2, "%-26.26s", " ");
			sprintf (store [i].ex_code3, "%-2.2s", " ");
			sprintf (store [i].ex_desc3, "%-26.26s", " ");
			sprintf (store [i].ex_code4, "%-2.2s", " ");
			sprintf (store [i].ex_desc4, "%-26.26s", " ");
		}
		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/*
		 * Enter screen 2 tabular input
		 */
		heading (2);
		entry (2);
		if (restart)
			continue;

		do 
		{
			edit_all ();
			if (restart)
				break;
		} while (CheckTotals ()) ;

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
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (sjld, sjld_list, SJLD_NO_FIELDS, "sjld_id_no");
	open_rec (sjhr, sjhr_list, SJHR_NO_FIELDS, "sjhr_id_no");
	open_rec (sjlr, sjlr_list, SJLR_NO_FIELDS, "sjlr_id_no");
	open_rec (sjsr, sjsr_list, SJSR_NO_FIELDS, "sjsr_id_no");
	open_rec (sjvh, sjvh_list, SJVH_NO_FIELDS, "sjvh_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (sjhr);
	abc_fclose (sjld);
	abc_fclose (sjlr);
	abc_fclose (sjsr);
	abc_fclose (sjvh);
	abc_fclose (cumr);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("employee"))
	{
		if (SRCH_KEY)
		{
			SrchSjsr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjsr_rec.co_no, comm_rec.co_no);
		strcpy (sjsr_rec.est_no, comm_rec.est_no);
		strcpy (sjsr_rec.dp_no, comm_rec.dp_no);
		strcpy (sjsr_rec.code, sjld_rec.emp_code);
		cc = find_rec (sjsr, &sjsr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess053));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (sjlr_rec.co_no, comm_rec.co_no);
		strcpy (sjlr_rec.est_no, comm_rec.est_no);
		strcpy (sjlr_rec.dp_no, comm_rec.dp_no);
		strcpy (sjlr_rec.code, sjsr_rec.lb_rt_code);
		cc = find_rec (sjlr, &sjlr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlSjMess013));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sjld_rec.tm_rate = sjlr_rec.cost_hr;
		sjld_rec.oh_rate = sjlr_rec.ovhd_hr;
		sjld_rec.pr_rate = sjlr_rec.profit_hr;
		display_field (field + 1);
	}

	if (LCHECK ("service_no"))
	{
		if (SRCH_KEY)
		{
			SrchSjhr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sjhr_rec.co_no, comm_rec.co_no);
		strcpy (sjhr_rec.est_no, comm_rec.est_no);
		strcpy (sjhr_rec.dp_no, comm_rec.dp_no);
		sjhr_rec.order_no = local_rec.order_no ;
		cc = find_rec (sjhr, &sjhr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlSjMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (sjhr_rec.status [0] == 'H')
		{
			errmess (ML (mlSjMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (sjhr_rec.status [0] != 'O')
		{
			errmess (ML (mlSjMess005));
			return (EXIT_FAILURE);
		}
		cumr_rec.hhcu_hash = sjhr_rec.chg_client;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].cli_desc, cumr_rec.dbt_name);
		strcpy (store [line_cnt].cli_no, cumr_rec.dbt_no);
		display_field (field + 1);
		tab_other (line_cnt);
	}

	if (LCHECK ("vehicle"))
	{
		if (dflt_used)
		{
			local_rec.vh_rates = 0.00;
			skip_entry = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSjvh (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (sjvh_rec.co_no, comm_rec.co_no);
		strcpy (sjvh_rec.est_no, comm_rec.est_no);
		strcpy (sjvh_rec.code, local_rec.vehicleCode);
		cc = find_rec (sjvh, &sjvh_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess218));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		local_rec.vh_rates = sjvh_rec.rate;
		display_field (field + 1);
	}

	if (LCHECK ("extra_code"))
	{
		/*
		 * skip remaining entries after 1st null extra code 
		 */
		if (dflt_used)
		{
			skip_entry = 24 - field;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSjlr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * calc subscript for extras rate (0 - 3) 
		 */
		local_rec.ex_cnt = (field -13)/3;

		strcpy (sjlr_rec.co_no, comm_rec.co_no);
		strcpy (sjlr_rec.est_no, comm_rec.est_no);
		strcpy (sjlr_rec.dp_no, comm_rec.dp_no);
		strcpy (sjlr_rec.code, local_rec.ex_code [local_rec.ex_cnt]);
		cc = find_rec (sjlr, &sjlr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlSjMess013));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (local_rec.ex_cnt == 0)
		{
			strcpy (store [line_cnt].ex_code1, sjlr_rec.code);
			strcpy (store [line_cnt].ex_desc1, sjlr_rec.descr);
		}
		if (local_rec.ex_cnt == 1)
		{
			strcpy (store [line_cnt].ex_code2, sjlr_rec.code);
			strcpy (store [line_cnt].ex_desc2, sjlr_rec.descr);
		}
		if (local_rec.ex_cnt == 2)
		{
			strcpy (store [line_cnt].ex_code3, sjlr_rec.code);
			strcpy (store [line_cnt].ex_desc3, sjlr_rec.descr);
		}
		if (local_rec.ex_cnt == 3)
		{
			strcpy (store [line_cnt].ex_code4, sjlr_rec.code);
			strcpy (store [line_cnt].ex_desc4, sjlr_rec.descr);
		}
		tab_other (line_cnt);
	}
	return (EXIT_SUCCESS);
}

/*
 * check sum of hourly columns 
 */
int 
CheckTotals (void)
{
	int	i = 0;
	float  	tord = 0.0, 
			tt15 = 0.0, 
			tt2  = 0.0;

	scn_set (2);
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);
	 	tord += local_rec.ord;	
	 	tt15 += local_rec.t15;	
	 	tt2  += local_rec.t2;	
	}

	if (cur_screen != 2)
		scn_set (1);

	if (tord != local_rec.tot_ord)
	{
		sprintf (err_str, "Sum Of Ordinary = %10.2f  Difference = %10.2f, <RETURN> To Continue", tord, local_rec.tot_ord - tord);
		PauseForKey (0, 0, err_str, 0);
		return (EXIT_FAILURE);
	}

	if (tt15 != local_rec.tot_t15)
	{
		sprintf (err_str, "Sum Of Time And A Half = %10.2f  Difference = %10.2f, <RETURN> To Continue", tt15, local_rec.tot_t15 - tt15);
		PauseForKey (0, 0, err_str, 0);
		return (EXIT_FAILURE);
	}
	if (tt2 != local_rec.tot_t2)
	{
		sprintf (err_str, "Sum Of Double Time = %10.2f  Difference = %10.2f, <RETURN> To Continue", tt2, local_rec.tot_t2 - tt2);
		PauseForKey (0, 0, err_str, 0);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int
Update (void)
{
	int	n, 
		i = 0, 
		ex_ptr;
	char	ex_cd [3];

	scn_set (2);

	/*
	 * Post to labour details analysis file 
	 */
	strcpy (sjld_rec.co_no, comm_rec.co_no);
	strcpy (sjld_rec.est_no, comm_rec.est_no);
	strcpy (sjld_rec.dp_no, comm_rec.dp_no);

	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);
		sjld_rec.order_no 		= local_rec.order_no;
		sjld_rec.time 			= local_rec.ord;
		sjld_rec.time_half 		= local_rec.t15;
		sjld_rec.time_double 	= local_rec.t2;
		strcpy (sjld_rec.veh_code, local_rec.vehicleCode);
		sjld_rec.km 	= local_rec.km;
		if (strcmp (local_rec.vehicleCode, "   ") == 0)
			sjld_rec.km_rate = 0.00;
		else
			sjld_rec.km_rate = local_rec.vh_rates;
		sjld_rec.dm 		= 0.0;
		sjld_rec.dm_rate 	= 0.0;
		sjld_rec.hm 		= 0.0;
		sjld_rec.hm_rate 	= 0.0;
		sjld_rec.mm 		= 0.0;
		sjld_rec.mm_rate 	= 0.0;
		sjld_rec.kk 		= 0.0;
		sjld_rec.kk_rate 	= 0.0;

		for (n = 13; n < 24; n += 2)
		{
			ex_ptr = (n - 13) /3;
			strcpy (ex_cd, local_rec.ex_code [ex_ptr]);
			if (strcmp (ex_cd, "  ") == 0)
				break;

			if (strcmp (ex_cd, "DM") == 0)
			{
				sjld_rec.dm = local_rec.ex_hrs [ex_ptr];
				sjld_rec.dm_rate = local_rec.ex_rates [ex_ptr];
			}		
			else if (strcmp (ex_cd, "HM") == 0)
			{
				sjld_rec.hm = local_rec.ex_hrs [ex_ptr];
				sjld_rec.hm_rate = local_rec.ex_rates [ex_ptr];
			}		
			else if (strcmp (ex_cd, "KK") == 0)
			{
				sjld_rec.kk = local_rec.ex_hrs [ex_ptr];
				sjld_rec.kk_rate = local_rec.ex_rates [ex_ptr];
			}		
			else if (strcmp (ex_cd, "MM") == 0)
			{
				sjld_rec.mm = local_rec.ex_hrs [ex_ptr];
				sjld_rec.mm_rate = local_rec.ex_rates [ex_ptr];
			}
		}
		UpdateSjld ();
	}
	return (EXIT_SUCCESS);
}

void
UpdateSjld (void)
{
	cc = abc_add (sjld, &sjld_rec);
	if (cc)
		file_err (cc, sjld, "DBADD");
}

void
SrchSjvh (
 char *keyValue)
{
	_work_open (3, 0, 40);
	save_rec ("#No.", "#Vehicle Description.");
	strcpy (sjvh_rec.co_no, comm_rec.co_no);
	strcpy (sjvh_rec.est_no, comm_rec.est_no);
	sprintf (sjvh_rec.code, "%-3.3s", keyValue);
	cc = find_rec (sjvh, &sjvh_rec, GTEQ, "r");
	while (!cc && !strcmp (sjvh_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sjvh_rec.est_no, comm_rec.est_no) && 
		      !strncmp (sjvh_rec.code, keyValue, strlen (keyValue)))
	{
		cc = save_rec (sjvh_rec.code, sjvh_rec.vehicle);
		if (cc)
			break;
		cc = find_rec (sjvh, &sjvh_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjvh_rec.co_no, comm_rec.co_no);
	strcpy (sjvh_rec.est_no, comm_rec.est_no);
	sprintf (sjvh_rec.code, "%-3.3s", temp_str);
	cc = find_rec (sjvh, &sjvh_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sjvh, "DBFIND");
}

/*==========================================
| Search routine for Employee Master File. |
==========================================*/
void
SrchSjsr (
 char *keyValue)
{
	_work_open (10, 0, 40);
	save_rec ("#Emp Code", "#Employee Name");
	strcpy (sjsr_rec.co_no, comm_rec.co_no);
	strcpy (sjsr_rec.est_no, comm_rec.est_no);
	strcpy (sjsr_rec.dp_no, comm_rec.dp_no);
	sprintf (sjsr_rec.code, "%-10.10s", keyValue);
	cc = find_rec (sjsr, &sjsr_rec, GTEQ, "r");
	while (!cc && !strcmp (sjsr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sjsr_rec.est_no, comm_rec.est_no) && 
		      !strcmp (sjsr_rec.dp_no, comm_rec.dp_no) && 
		      !strncmp (sjsr_rec.code, keyValue, strlen (keyValue))) 
	{
		cc = save_rec (sjsr_rec.code, sjsr_rec.name);
		if (cc)
			break;
		cc = find_rec (sjsr, &sjsr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjsr_rec.co_no, comm_rec.co_no);
	strcpy (sjsr_rec.est_no, comm_rec.est_no);
	strcpy (sjsr_rec.dp_no, comm_rec.dp_no);
	sprintf (sjsr_rec.code, "%-10.10s", temp_str);
	cc = find_rec (sjsr, &sjsr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sjsr, "DBFIND");
}

/*
 * Search routine for Service Header File. 
 */
void
SrchSjhr (
 char *keyValue)
{
	char	order [9];

	_work_open (8, 0, 10);
	save_rec ("#Job No", "#Issued on");
	strcpy (sjhr_rec.co_no, comm_rec.co_no);
	strcpy (sjhr_rec.est_no, comm_rec.est_no);
	strcpy (sjhr_rec.dp_no, comm_rec.dp_no);
	sjhr_rec.order_no = atol (keyValue);
	cc = find_rec (sjhr, &sjhr_rec, GTEQ, "r");
	while (!cc && !strcmp (sjhr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sjhr_rec.est_no, comm_rec.est_no) && 
		      !strcmp (sjhr_rec.dp_no, comm_rec.dp_no))
	{
		sprintf (order, "%8ld", sjhr_rec.order_no);
		if ((strlen (keyValue) == 0 || 
		      !strncmp (order, keyValue, strlen (keyValue))) && 
		      sjhr_rec.status [0] == 'O')
		{
			strcpy (err_str, DateToString (sjhr_rec.issue_date));
			cc = save_rec (order, err_str);
			if (cc)
				break;
		}
		cc = find_rec (sjhr, &sjhr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjhr_rec.co_no, comm_rec.co_no);
	strcpy (sjhr_rec.est_no, comm_rec.est_no);
	strcpy (sjhr_rec.dp_no, comm_rec.dp_no);
	sjhr_rec.order_no = atol (temp_str);
	cc = find_rec (sjhr, &sjhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sjhr, "DBFIND");
}

void
SrchSjlr (
 char *keyValue)
{
	_work_open (2, 0, 40);
	save_rec ("#Lr", "#Labour Rate Description");
	strcpy (sjlr_rec.co_no, comm_rec.co_no);
	strcpy (sjlr_rec.est_no, comm_rec.est_no);
	strcpy (sjlr_rec.dp_no, comm_rec.dp_no);
	sprintf (sjlr_rec.code, "%-2.2s", keyValue);
	cc = find_rec (sjlr, &sjlr_rec, GTEQ, "r");
	while (!cc && !strcmp (sjlr_rec.co_no, comm_rec.co_no) && 
		      !strcmp (sjlr_rec.est_no, comm_rec.est_no) && 
		      !strcmp (sjlr_rec.dp_no, comm_rec.dp_no) && 
		      !strncmp (sjlr_rec.code, keyValue, strlen (keyValue))) 
	{
		cc = save_rec (sjlr_rec.code, sjlr_rec.descr);
		if (cc)
			break;
		cc = find_rec (sjlr, &sjlr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (sjlr_rec.co_no, comm_rec.co_no);
	strcpy (sjlr_rec.est_no, comm_rec.est_no);
	strcpy (sjlr_rec.dp_no, comm_rec.dp_no);
	sprintf (sjlr_rec.code, "%-2.2s", temp_str);
	cc = find_rec (sjlr, &sjlr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sjlr, "DBFIND");
}

/*
 * Display Infor for lines while in edit mode. 
 */
void
tab_other (
 int iline)
{
	crsr_off ();
	if (cur_screen == 2)
	{
		if (!strcmp (store [iline].cli_no, "      "))
		{
			if (!clear_box)
			{
				clear_box = TRUE;
				erase_box (2, 2, 120, 3);
				print_at (3, 4, "%-118.118s", " ");
				print_at (4, 4, "%-118.118s", " ");
				print_at (5, 4, "%-118.118s", " ");
			}
			return;
		}
		clear_box = FALSE;
		box (2, 2, 120, 3);
	
		print_at (3, 4, ML (mlSjMess052), iline + 1, store [iline].cli_no, store [ iline].cli_desc);
		print_at (4, 4, ML (mlSjMess053), store [iline].ex_code1, store [ iline].ex_desc1);
		print_at (4, 40, ML (mlSjMess054), store [iline].ex_code2, store [ iline].ex_desc2);
		print_at (5, 4, ML (mlSjMess055), store [iline].ex_code3, store [ iline].ex_desc3);
		print_at (5, 40, ML (mlSjMess055), store [iline].ex_code4, store [ iline].ex_desc4);
	}
	if (prog_status == ENTRY)
		crsr_on ();
	return;
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
		rv_pr (ML (mlSjMess051), 50, 0, 1);
		line_at (1,0,132);
		if (scn == 1)
			box (0, 3, 132, 5);

		line_at (20,0,132);
		strcpy (err_str, ML (mlStdMess038));
		print_at (21, 0, err_str, comm_rec.co_no, comm_rec.co_name);
		strcpy (err_str, ML (mlStdMess039));
		print_at (22, 0, err_str, comm_rec.est_no, comm_rec.est_name);

		line_at (23,0,132);
	}
	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}
