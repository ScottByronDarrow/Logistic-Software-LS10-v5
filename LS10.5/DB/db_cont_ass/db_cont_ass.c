/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cont_ass.c,v 5.1 2001/11/22 06:19:13 scott Exp $
|  Program Name  : (db_cont_ass.c)
|  Program Desc  : (Customer Contract Linking Program)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 21/09/93         |
|---------------------------------------------------------------------|
| $Log: db_cont_ass.c,v $
| Revision 5.1  2001/11/22 06:19:13  scott
| Updated to convert to app.schema and perform a general code clean.
|
---------------------------------------------------------------------*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cont_ass.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cont_ass/db_cont_ass.c,v 5.1 2001/11/22 06:19:13 scott Exp $";

#include <pslscr.h>
#include <hot_keys.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>
#include <tabdisp.h>

#define	SINGLE	1
#define	MULTI	0


#include	"schema"

struct cncdRecord	cncd_rec;
struct cnchRecord	cnch_rec;
struct cnclRecord	cncl_rec;
struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;


	char	*data = "data";

	char	branchNo [3],
			prompt [17],
			promptdesc [17];

	int		envDbCo 	= FALSE,
			envDbFind  	= FALSE,
			deletion 	= FALSE,
			byContract	= FALSE,
			noInTab		= 0;

struct	{
	char	dummy [11];
	char	field [7];
	char	field_desc [41];
} local_rec;

static struct var vars [] = 
{
	{1, LIN, "field1",	 4, 15, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", prompt, " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.field},
	{1, LIN, "fielddesc",	 5, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", promptdesc, " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.field_desc},
	{0, LIN, "dummy",	 0, 0, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.dummy},
};

static	int	tag_func (int c, KEY_TAB *psUnused);
static	int	exit_func (int c, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB list_keys [] =
{
   { " TOGGLE ",		'T',		tag_func,
	"Tag Line.",					"A" },
   { " ALL TOGGLE ",			CTRL ('A'), 	tag_func,
	"Tag All Lines.",				"A" },
   { NULL,			FN1, 		exit_func,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		exit_func,
	"Exit and update the database.",				"A" },
   END_KEYS
};
#else
static	KEY_TAB list_keys [] =
{
   { " [T]OGGLE ",		'T',		tag_func,
	"Tag Line.",					"A" },
   { " [^A]LL TOGGLE",			CTRL ('A'), 	tag_func,
	"Tag All Lines.",				"A" },
   { NULL,			FN1, 		exit_func,
	"Exit without update.",						"A" },
   { NULL,			FN16, 		exit_func,
	"Exit and update the database.",				"A" },
   END_KEYS
};
#endif

#include	<FindCumr.h>
/*
 * Local Function Prototypes.
 */
int 	NotMatched 		(void);
int 	heading 		(int);
int 	spec_valid 		(int);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	SrchCnch 		(char *);
void 	ProcessCustomer (void);
void 	TagLine 		(int, int);
void 	ProcessContract (void);
void 	Update 			(void);

int
main (
	int		argc,
	char	*argv [])
{

	char	*sptr;

	if (argc != 2 || 
		 (argv [1][0] != 'd' &&
		 argv [1][0] != 'D' &&
		 argv [1][0] != 'n' &&
		 argv [1][0] != 'N'))
	{
		print_at (0,0, mlDbMess138, argv [0]);
		return (EXIT_FAILURE);
	}

	if (argv [1][0] == 'd' || argv [1][0] == 'D')
		deletion = TRUE;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		argv [0] = sptr + 1;

	if (!strcmp (argv [0], "db_cont_ass"))
		byContract = TRUE;

	if (byContract)
	{
		strcpy (prompt,     " Contract      ");
		strcpy (promptdesc, " Description   ");
	}
	else
	{
		strcpy (prompt,     " Customer No   ");
		strcpy (promptdesc, " Customer Name ");
	}

	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	OpenDB ();

	envDbCo = atoi (get_env ("DB_CO"));
	envDbFind  = atoi (get_env ("DB_FIND"));

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		search_ok  = TRUE;
		init_ok    = FALSE;
		entry_exit = FALSE;
		edit_exit  = FALSE;
		restart    = FALSE;
		prog_exit  = FALSE;
		init_vars (1);

		heading (1);
		entry (1);

		if (restart || prog_exit)
			continue;

		noInTab = 0;

		heading (1);
		scn_display (1);

		if (byContract)
			ProcessContract ();
		else
			ProcessCustomer ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, 
							(envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	open_rec (cncl, cncl_list, CNCL_NO_FIELDS, 
							(!byContract) ? "cncl_id_no2" : "cncl_id_no");
	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cncd, cncd_list, CNCD_NO_FIELDS, "cncd_id_no3");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cnch);
	abc_fclose (cncl);
	abc_fclose (cncd);
	abc_dbclose (data);
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
	if (deletion)
	{
		if (byContract)
			sprintf (err_str, ML (mlDbMess120));
		else
			sprintf (err_str, ML (mlDbMess121));
	}
	else
	{
		if (byContract)
			sprintf (err_str, ML (mlDbMess122));
		else
			sprintf (err_str, ML (mlDbMess123));
	}
	clip (err_str);
	strcat (err_str, " ");
	rv_pr (err_str, (40 - (strlen (err_str) / 2)), 0, 1);

	box (0, 3, 80, 2);		

	line_at (1,0,80);
	line_at (21,0,80);

	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("field1") && !byContract)
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, zero_pad (local_rec.field, 6));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.field_desc, cumr_rec.dbt_name);
		scn_display (1);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("field1") && byContract)
	{
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, local_rec.field);

		if (SRCH_KEY)
		{
			SrchCnch (temp_str);
			return (EXIT_SUCCESS);
		}
		cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (cnch_rec.date_exp < TodaysDate ())
		{
			print_mess (ML (mlDbMess010));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.field_desc, cnch_rec.desc);
		scn_display (1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*===============================
| Search on Contract (cnch)     |
===============================*/
void
SrchCnch (
	char	*key_val)
{
	_work_open (6,0,40);
	save_rec ("#No","#Contract Description");

	cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
	while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no))
	{                        
		if (strncmp (cnch_rec.cont_no, key_val, strlen (key_val)))
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}
		if (cnch_rec.date_exp < TodaysDate ())
		{
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			continue;
		}
		cc = save_rec (cnch_rec.cont_no,cnch_rec.desc);
		if (cc)
				break;
		cc = find_rec (cnch, &cnch_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, cnch, "DBFIND");
}

void
ProcessCustomer (void)
{
	/*------------
	| Open table |
	------------*/
	tab_open ("cnt_lst", list_keys, 8, 6, 9, FALSE);
	tab_add ("cnt_lst", 
		"# %-11.11s  %-40.40s  %-3.3s ",
		"CONT NO",
		"D  E  S  C  R  I  P  T  I  O  N",
		"TAG");

	/*--------------------------
	| if not delete option then
	| loop thru all contracts
	| if already assigned to 
	| customer then auto tag
	---------------------------*/
	if (!deletion)
	{
		abc_selfield (cnch, "cnch_id_no");
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, "      ");

		cc = find_rec (cnch, &cnch_rec, GTEQ, "r");
		while (!cc && !strcmp (cnch_rec.co_no, comm_rec.co_no))
		{
			if (NotMatched ())
				continue;

			if (cnch_rec.date_exp < TodaysDate ())
			{
				cc = find_rec (cnch, &cnch_rec, NEXT, "r");
				continue;
			}
			/*
			 * see whether already assigned to customer
			 */
			cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cncl_rec.hhch_hash = cnch_rec.hhch_hash;
			cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
			tab_add 
			(
				"cnt_lst", 
				" %-11.11s  %-40.40s   %1.1s ",
				cnch_rec.cont_no,
				cnch_rec.desc,
				(cc) ? " " : "A"
			);
			noInTab++;
			cc = find_rec (cnch, &cnch_rec, NEXT, "r");
		}
	}
	else
	{
		/*
		 * deletion - only look up for those already linked
		 */
		abc_selfield (cnch, "cnch_hhch_hash");
		cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cncl_rec.hhch_hash = 0L;
		cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
		while (!cc && cncl_rec.hhcu_hash == cumr_rec.hhcu_hash)
		{
			cc = find_hash (cnch, &cnch_rec, EQUAL, "r", cncl_rec.hhch_hash);
			if (cc)
				file_err (cc, cnch, "DBFIND");

			tab_add ("cnt_lst", 
					" %-11.11s  %-40.40s   %1.1s ",
					cnch_rec.cont_no,
					cnch_rec.desc,
					" "
				  );
			noInTab++;
			cc = find_rec (cncl, &cncl_rec, NEXT, "r");
		}
	}

	if (noInTab == 0)
	{
		tab_add ("cnt_lst", ML ("There Are No Valid Contracts For Selected Customer."));
		tab_display ("cnt_lst", TRUE);
		sleep (sleepTime);
		tab_close ("cnt_lst", TRUE);
		return;
	}
	else
	{
		tab_scan ("cnt_lst");
	}
	
	if (!restart)
		Update ();

	tab_close ("cnt_lst", TRUE);
}


static int
tag_func (
 int                c,
 KEY_TAB*           psUnused)
{
	int	i;
	int	st_line;
	char	get_buf [100];

	st_line = tab_tline ("cnt_lst");

	if (c == 'T')
		TagLine (st_line, SINGLE);
	else
	{
		for (i = st_line; i < noInTab; i++)
			TagLine (i, MULTI);

		tab_display ("cnt_lst", TRUE);
		tab_get ("cnt_lst", get_buf, EQUAL, st_line);
		redraw_keys ("cnt_lst");
	}
	return (c);
}


void
TagLine (
	int		line_no,
	int		type)
{
	char	get_buf [100];
	char	curr_stat [2];
	char	new_stat [2];

	tab_get ("cnt_lst", get_buf, EQUAL, line_no);
	sprintf (curr_stat, "%-1.1s", get_buf + 57);

	if (curr_stat [0] == '*')
		strcpy (new_stat, " ");
	else
	{
		if (curr_stat [0] != 'A')
		{
			strcpy (new_stat, "*");
		}
		else
		{
			if (type == SINGLE)
			{
				print_mess (ML (mlDbMess124));
				sleep (sleepTime);
				clear_mess ();
			}
			strcpy (new_stat, "A");
		}
	}

	tab_update ("cnt_lst", "%-57.57s%-1.1s ", get_buf, new_stat);
}

static int
exit_func (
	int			c,
	KEY_TAB		*psUnused)
{
	if (c == FN1)
		restart = TRUE;
	
	return (c);
}

void
ProcessContract (void)
{
	/*------------
	| Open table |
	------------*/
	tab_open ("cnt_lst", list_keys, 8, 6, 9, FALSE);
	tab_add ("cnt_lst", 
		"# %-11.11s  %-40.40s  %-3.3s ",
		"CUST NO",
		"C  U  S  T  O  M  E  R     N  A  M  E",
		"TAG");

	/*--------------------------
	| if not delete option then
	| loop thru all customer
	| if already assigned to 
	| contract then auto tag
	---------------------------*/
	if (!deletion)
	{
		abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, "      ");
		cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

		while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
		{
			if (strcmp (cumr_rec.est_no, branchNo))
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}
			if (NotMatched ())
				continue;
			/*------------------
			| see whether already 
			| assigned to contract
			--------------------*/
			cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
			cncl_rec.hhch_hash = cnch_rec.hhch_hash;
			cc = find_rec (cncl, &cncl_rec, EQUAL, "r");
			tab_add ("cnt_lst", 
					" %-11.11s  %-40.40s   %1.1s ",
					cumr_rec.dbt_no,
					cumr_rec.dbt_name,
					 (cc) ? " " : "A"
				  );
			noInTab++;
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		}
	}
	else
	{
		/*
		 * deletion - only look up for those already linked
		 */
		abc_selfield (cumr, "cumr_hhcu_hash");
		cncl_rec.hhcu_hash = 0L;
		cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		cc = find_rec (cncl, &cncl_rec, GTEQ, "r");
		while (!cc && cncl_rec.hhch_hash == cnch_rec.hhch_hash)
		{
			cumr_rec.hhcu_hash	=	cncl_rec.hhcu_hash;
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND");

			tab_add ("cnt_lst", 
					" %-11.11s  %-40.40s   %1.1s ",
					cumr_rec.dbt_no,
					cumr_rec.dbt_name,
					" "
				  );
			noInTab++;
			cc = find_rec (cncl, &cncl_rec, NEXT, "r");
		}
	}

	if (noInTab == 0)
	{
		tab_add ("cnt_lst", ML ("There Are No Valid Customers For Selected Contract."));
		tab_display ("cnt_lst", TRUE);
		sleep (sleepTime);
		tab_close ("cnt_lst", TRUE);
		return;
	}
	else
	{
		tab_scan ("cnt_lst");
	}
	
	if (!restart)
		Update ();

	tab_close ("cnt_lst", TRUE);
}

/*
 * this function can be called | by customer (deletion && normal)
 * or by contract (deletion && normal)
 */
void
Update (void)
{
	/*
	 * in all cases will loop thru reading line by line
	 */
	int	count;
	char	get_buf [100];
	char	curr_stat [2];

	abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	abc_selfield (cnch, "cnch_id_no");

	for (count = 0; count < noInTab; count++)
	{
		tab_get ("cnt_lst", get_buf, EQUAL, count);

		sprintf (curr_stat, "%-1.1s", get_buf + 57);
		if (curr_stat [0] != '*')
			continue;

		/*
		 * as the buffer does not hold the necesaary hashes look then up
		 */
		if (byContract)
		{
			cncl_rec.hhch_hash = cnch_rec.hhch_hash;
			/*
			 * look up customer hash
			 */
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNo);
			sprintf (cumr_rec.dbt_no, "%6.6s", get_buf + 1);
			cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND");
			cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
		}
		else
		{
			cncl_rec.hhcu_hash = cumr_rec.hhcu_hash;
			/*
			 * look up Contracts hash
			 */
			strcpy (cnch_rec.co_no, comm_rec.co_no);
			sprintf (cnch_rec.cont_no, "%6.6s", get_buf + 1);
			cc = find_rec (cnch, &cnch_rec, EQUAL, "r");
			if (cc)
				file_err (cc, cnch, "DBFIND");
			cncl_rec.hhch_hash = cnch_rec.hhch_hash;
		}

		/*
		 * if deletion then look up the cncl else add cncl
		 */
		if (deletion)
		{
			cc = find_rec (cncl, &cncl_rec, EQUAL, "u");
			if (cc)
				file_err (cc, cncl, "DBFIND");

			cc = abc_delete (cncl);
			if (cc)
				file_err (cc, cncl, "DBDELETE");
		}
		else
		{
			cc = abc_add (cncl, &cncl_rec);
			if (cc)
				file_err (cc, cncl, "DBADD");
		}
	}
}

int
NotMatched (void)
{
	/*---------------------
	| see if has curr match
	---------------------*/
	if (cnch_rec.exch_type [0] != 'V')
	{
		cncd_rec.hhch_hash = cnch_rec.hhch_hash;
		strcpy (cncd_rec.curr_code, cumr_rec.curr_code);
		cc = find_rec (cncd, &cncd_rec, EQUAL, "r");
		if (cc)
		{
			if (!byContract)
				cc = find_rec (cnch, &cnch_rec, NEXT, "r");
			else
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

