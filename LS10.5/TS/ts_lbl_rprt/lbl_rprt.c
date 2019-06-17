/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: lbl_rprt.c,v 5.2 2001/08/09 09:23:26 scott Exp $
|  Program Name  : (ts_lbl_rprt.c) 
|  Program Desc  : (Flag Labels As Unprinted)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (22/06/92)       |
|---------------------------------------------------------------------|
| $Log: lbl_rprt.c,v $
| Revision 5.2  2001/08/09 09:23:26  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:55:58  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:10  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/28 00:30:35  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to add missing ML calls.
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lbl_rprt.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/TS/ts_lbl_rprt/lbl_rprt.c,v 5.2 2001/08/09 09:23:26 scott Exp $";

#include <pslscr.h>	
#include <hot_keys.h>
#include <getnum.h>
#include <tabdisp.h>
#include <ml_std_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct tslhRecord	tslh_rec;
struct tslbRecord	tslb_rec;
struct tmopRecord	tmop_rec;

	char	*data   		= "data",
			*labelTagFile	= "labelTagFile";

	int		num_in_tab	= 0,
			upd_tslb 	= TRUE;

	long	lsystemDate;

	char	systemDate [11];
	char	localLogName [15];

static int tag_func 	(int, KEY_TAB *);
static int exit_func 	(int, KEY_TAB *);

#ifdef	GVISION
static	KEY_TAB indnt_keys [] = 
{
    { " TAG/UNTAG LINE ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " TAG/UNTAG ALL ",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    { " TAG/UNTAG RANGE ",	'R', tag_func,
	"Tag/Untag Range.",						"A" },
    { NULL,	FN1, exit_func,
	"",								"A" },
    END_KEYS
};
#else
static	KEY_TAB indnt_keys [] = 
{
    { " [T]AG/UNTAG ",	'T', tag_func,
	"Tag/Untag current line.",					"A" },
    { " [^A]ccept All",	CTRL ('A'), tag_func,
	"Tag/Untag All Lines.",						"A" },
    { " TAG [R]ANGE ",	'R', tag_func,
	"Tag/Untag Range.",						"A" },
    { NULL,	FN1, exit_func,
	"",								"A" },
    END_KEYS
};
#endif

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	oprtr [15];
	char	op_desc [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "operator",	 3, 15, CHARTYPE,
		"UUUUUUUUUUUUUU", "          ",
		" ", " ", " Operator   :", " ",
		 YES, NO,  JUSTLEFT, "", "", local_rec.oprtr},
	{1, LIN, "op_desc",	 3, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.op_desc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	SrchTmop 			(char *);
void 	Process 			(void);
void 	Update 				(void);
int 	heading 			(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr = getenv ("LOGNAME");

	if (sptr)
	{
		sprintf (localLogName, "%-14.14s", sptr);
		upshift (localLogName);
	}
	else
		sprintf (localLogName, "%-14.14s", "DEFAULT");

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);
	set_masks ();
	
	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	set_tty ();
	init_scr ();		/*  sets terminal from termcap	*/

	init_vars (1);

	/*-------------------
	| Get default info. |
	-------------------*/
	sprintf (local_rec.oprtr, "%-14.14s", localLogName);
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", local_rec.oprtr);
	cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (local_rec.oprtr, "%-14.14s", " ");
		sprintf (local_rec.op_desc, "%-40.40s", "ALL OPERATORS");
	}
	else
		sprintf (local_rec.op_desc, "%-40.40s",tmop_rec.op_name);

	/*--------------------
	| Main control loop. |
	--------------------*/
	while (!prog_exit)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading ();
		entry (1);
		if (prog_exit || restart)
			continue;

		heading ();
		scn_display (1);
		edit (1);
		if (restart)
			continue;
	
		Process ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
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
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (tslb,  tslb_list, TSLB_NO_FIELDS, "tslb_id_no3");
	open_rec (tslh,  tslh_list, TSLH_NO_FIELDS, "tslh_hhlh_hash");
	open_rec (tmop,  tmop_list, TMOP_NO_FIELDS, "tmop_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (tslb);
	abc_fclose (tslh);
	abc_fclose (tmop);
	abc_dbclose (data);
}

int
spec_valid (
 int    field)
{

	if (LCHECK ("operator"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.oprtr, "%-14.14s", " ");
			sprintf (local_rec.op_desc, "%-40.40s", ML ("ALL OPERATORS"));
			DSP_FLD ("op_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchTmop (temp_str);
			return (EXIT_SUCCESS);
		}
		
		strcpy (tmop_rec.co_no, comm_rec.co_no);
		sprintf (tmop_rec.op_id, "%-14.14s", local_rec.oprtr);
		cc = find_rec (tmop, &tmop_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML ("Operator not found on file "));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.op_desc, "%-40.40s", tmop_rec.op_name);
		DSP_FLD ("op_desc");

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*------------------------------------------
| Search routine for Operator master file. |
------------------------------------------*/
void
SrchTmop (
 char*  key_val)
{
	_work_open (10,0,40);
	save_rec ("#Operator", "#Description");
	strcpy (tmop_rec.co_no, comm_rec.co_no);
	sprintf (tmop_rec.op_id, "%-14.14s", key_val);
	cc = find_rec (tmop, &tmop_rec, GTEQ, "r");
	while (!cc && 
		  !strcmp (tmop_rec.co_no, comm_rec.co_no) &&
	          !strncmp (tmop_rec.op_id, key_val, strlen (key_val)))
	{
		cc = save_rec (tmop_rec.op_id, tmop_rec.op_name);
		if (cc)
			break;

		cc = find_rec (tmop, &tmop_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

void
Process (void)
{
	int	seq_no;
	char	tmp_seq [6];

	heading ();

	seq_no = 1;
	num_in_tab = 0;
	tab_open (labelTagFile, indnt_keys, 2, 0, 14, FALSE);
	tab_add (labelTagFile, 
		"# %-3.3s| %-6.6s| %-40.40s|%-11.11s| %-40.40s| %-9.9s| %-9.9s",
		"NO.", 
		" CUST.", 
		"   CUSTOMER NAME   ",
		"LETTER", 
		"        LETTER NAME",
		"DATE SENT", 
		"TIME SENT");

	strcpy (tslb_rec.co_no, comm_rec.co_no);
	sprintf (tslb_rec.tslb_operator, "%-14.14s", local_rec.oprtr);
	strcpy (tslb_rec.label_prt, "Y");
	tslb_rec.date_sent = 0L;
	tslb_rec.time_sent = 0L;
	tslb_rec.hhcu_hash = 0L;
	cc = find_rec (tslb, &tslb_rec, GTEQ, "r");
	while (!cc && !strcmp (tslb_rec.co_no, comm_rec.co_no))
	{
		if ((strcmp (tslb_rec.tslb_operator, local_rec.oprtr) &&
		     strcmp (local_rec.oprtr, "              ")) ||
		     strcmp (tslb_rec.label_prt, "Y"))
		{
			cc = find_rec (tslb, &tslb_rec, NEXT, "r");
			continue;
		}

		/*------------------------------
		| Look up cust and letter info |
		------------------------------*/
		cumr_rec.hhcu_hash = tslb_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (tslb, &tslb_rec, NEXT, "r");
			continue;
		}

		tslh_rec.hhlh_hash	= tslb_rec.hhlh_hash;
		cc = find_rec (tslh, &tslh_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (tslb, &tslb_rec, NEXT, "r");
			continue;
		}

		/*-------------------
		| Add line to table |
		-------------------*/
		sprintf (tmp_seq, "%03d", seq_no++);
		tab_add (labelTagFile, 
			" %-3.3s  %-6.6s  %-40.40s  %-10.10s  %-40.40s  %-10.10s %-5.5s          %08ld   %08ld   %08ld",
			tmp_seq,
			cumr_rec.dbt_no,
			cumr_rec.dbt_name,
			tslh_rec.let_code,
			tslh_rec.let_desc,
			DateToString (tslb_rec.date_sent),
			ttoa (tslb_rec.time_sent, "NN:NN"),
			cumr_rec.hhcu_hash,
			tslh_rec.hhlh_hash,
			tslb_rec.hhlb_hash);
		num_in_tab++;

		cc = find_rec (tslb, &tslb_rec, NEXT, "r");
	}

	if (num_in_tab > 0)
	{
		tab_scan (labelTagFile);
		if (upd_tslb)
			Update ();
	}
	else
	{
		tab_add (labelTagFile, ML ("  ************  THERE ARE NO LABELS TO BE REPRINTED  ************"));
		tab_display (labelTagFile, TRUE);
		putchar (BELL);
		fflush (stdout);
		sleep (sleepTime);
	}

	tab_close (labelTagFile, TRUE);
}

/*----------------------
| Update tagged lines. |
----------------------*/
void
Update (void)
{
	int	i;
	char	get_buf [200];

	abc_selfield (tslb, "tslb_hhlb_hash");

	/*--------------------------
	| Process all tagged lines |
	--------------------------*/
	for (i = 0; i < num_in_tab; i++)
	{
	    tab_get (labelTagFile, get_buf, EQUAL, i);
	    if (!tagged (get_buf))
			continue;

		tslb_rec.hhlb_hash = atol (get_buf + 158);
	    cc = find_rec (tslb, &tslb_rec, COMPARISON, "u");
	    if (cc)
		{
			abc_unlock (tslb);
			continue;
		}

	    /*--------------------
	    | Update tslb record |
	    --------------------*/
	    strcpy (tslb_rec.label_prt, "N");
	    cc = abc_update (tslb, &tslb_rec);
	    if (cc)
			file_err (cc, tslb, "DBUPDATE");
	}
}

/*-------------------------------------
| Allow user to tag lines for release |
-------------------------------------*/
static int
tag_func (
 int        c,
 KEY_TAB*   psUnused)
{
	int	i;
	int	seq_ok;
	int	st_seq;
	int	end_seq;
	char	get_buf [200];

	if (c == 'T')
		tag_toggle (labelTagFile);
	else
	{
		if (c == 'R')
		{
			/*--------------------------------------------
			| Get range to tag and tag_toggle () them.   |
			--------------------------------------------*/
			print_at (20, 10, ML ("From Sequence :              To Sequence :              "));
			crsr_on ();

			/*----------------------
			| Enter start sequence |
			----------------------*/
			seq_ok = FALSE;
			while (!seq_ok)
			{
				st_seq = getint (26, 20, "NNNNN");
				if (last_char == FN1)
				{
					blank_at (20, 10, 80);
					return (c);
				}

				if (st_seq <= 0)
				{
					print_mess (ML("Start Sequence Must Be Greater Than Zero"));
					sleep (sleepTime);
					clear_mess ();
					continue;
				}

				seq_ok = TRUE;
			}

			/*--------------------
			| Enter end sequence |
			--------------------*/
			seq_ok = FALSE;
			while (!seq_ok)
			{
				end_seq = getint (53, 20, "NNNNN");
				if (last_char == FN1)
				{
					blank_at (20, 10, 80);
					return (c);
				}
		
				if (end_seq < st_seq)
				{
					print_mess (ML ("End Sequence Must Be Greater Than Start Sequence"));
					sleep (sleepTime);
					clear_mess ();
					continue;
				}
				
				seq_ok = TRUE;
			}

			crsr_off ();
			blank_at (20, 10, 80);

			/*-------------------------
			| tag_toggle () the lines. |
			-------------------------*/
			for (i = 0; i < num_in_tab; i++)
			{
				tab_get (labelTagFile, get_buf, EQUAL, i);
				if (atoi (get_buf + 1) >= st_seq &&
				    atoi (get_buf + 1) <= end_seq)
				{
					tag_toggle (labelTagFile);
				}
			}
		}
		else
			tag_all (labelTagFile);
	}

	return (c);
}

/*-------------------------
| Abort without updating. |
-------------------------*/
static int
exit_func (
	int        c,
	KEY_TAB*   psUnused)
{
	upd_tslb = FALSE;

	return (FN16);
}

int
heading (void)
{
	swide ();
	clear ();

	rv_pr (ML (" Flag Telesales Labels For Reprinting "), 50, 0, 1);
	line_at (1,0,132);

	box (0, 2, 132, 1);

	line_at (21,0,132);
	print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	line_cnt = 0;
	scn_write (1);
    return (EXIT_SUCCESS);
}
