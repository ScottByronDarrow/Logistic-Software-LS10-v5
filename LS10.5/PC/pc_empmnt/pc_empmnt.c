/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( pc_empmnt.c )                                    |
|  Program Desc  : ( Employee Maintenance                         )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, prmr, prvr, rgrs,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  prmr, prvr,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Date Written  : (28/07/92)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  Date Modified : (25/11/93)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (08/11/94)      | Modified  by : Aroha Merrilees.  |
|  Date Modified : (04/09/1997)    | Modified  by : Jiggs A Veloz.    |
|                                                                     |
|  Comments      :                                                    |
|  (25/11/93)    : DPL 10099 - assignment of new employee's resources |
|                : Global Mods.                                       |
|  (08/11/94)    : PSL 11527 - fix tagging of default resource        |
|  (04/09/1997)  : SEL Multilingual Conversion. Replaced printf w/    |
|                :     print_at.                             		  |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: pc_empmnt.c,v $
| Revision 5.2  2001/08/09 09:14:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:58  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:31:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 02:50:38  scott
| Updated to for buttons on LS10-GUI
|
| Revision 3.0  2000/10/10 12:16:58  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:03:07  gerry
| forced Revision no start 2.0 Rel-15072000
|
| Revision 1.12  1999/11/12 10:37:45  scott
| Updated due to -wAll flag on compiler and removal of PNAME.
|
| Revision 1.11  1999/10/01 07:48:52  scott
| Updated for standard function calls.
|
| Revision 1.10  1999/09/29 10:11:32  scott
| Updated to be consistant on function names.
|
| Revision 1.9  1999/09/17 08:26:22  scott
| Updated for ttod, datejul, pjuldate, ctime + clean compile.
|
| Revision 1.8  1999/09/13 07:03:12  marlene
| *** empty log message ***
|
| Revision 1.7  1999/09/09 06:12:23  marlene
| *** empty log message ***
|
| Revision 1.6  1999/06/17 07:40:41  scott
| Update for database name and Log file additions required for cvs.
|
=====================================================================*/
#define	CCMAIN

char	*PNAME = "$RCSfile: pc_empmnt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_empmnt/pc_empmnt.c,v 5.2 2001/08/09 09:14:34 scott Exp $";

#include	<pslscr.h>
#include	<hot_keys.h>
#include	<tabdisp.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>

	/*==========================
	| Common Record Structure. |
	==========================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
	};

	int	comm_no_fields = 5;

	struct
	{
		int 	termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
	} comm_rec;

	/*==============================
	| PayRoll employee Master file |
	==============================*/
	struct dbview prmr_list [] =
	{
		{"prmr_hhmr_hash"},
		{"prmr_co_no"},
		{"prmr_br_no"},
		{"prmr_code"},
		{"prmr_hhrs_hash"},
		{"prmr_name"},
	};

	int	prmr_no_fields = 6;

	struct
	{
		long	hhmr_hash;
		char	co_no [3];
		char	br_no [3];
		char	code [9];
		long	hhrs_hash;
		char	name [41];
	} prmr_rec;

	/*==============================
	| PayRoll Valid Resources file |
	==============================*/
	struct dbview prvr_list [] =
	{
		{"prvr_hhmr_hash"},
		{"prvr_hhrs_hash"},
	};

	int	prvr_no_fields = 2;

	struct
	{
		long	hhmr_hash;
		long	hhrs_hash;
	} prvr_rec;

	/*==============================
	| Routing Resource Master file |
	==============================*/
	struct dbview rgrs_list [] =
	{
		{"rgrs_hhrs_hash"},
		{"rgrs_co_no"},
		{"rgrs_br_no"},
		{"rgrs_code"},
		{"rgrs_desc"},
	};

	int	rgrs_no_fields = 5;

	struct
	{
		long	hhrs_hash;
		char	co_no [3];
		char	br_no [3];
		char	code [9];
		char	desc [41];
	} rgrs_rec;

	char	*comm	= "comm",
			*data	= "data",
			*prmr	= "prmr",
			*prvr	= "prvr",
			*rgrs2	= "rgrs2",
			*rgrs	= "rgrs";

	int		new_emp = FALSE;

static	int	tag_func (int iUnused, KEY_TAB *psUnused);
static	int	exit_func (int iUnused, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB rgrs_keys [] =
{
	{	" TAG/UNTAG ",	'T', tag_func,
		"Tag/Untag resource as valid",			"A"	},
	{	NULL,		'\r', tag_func,
		"Tag/Untag resource as valid",			"A"	},
	{	NULL,		FN16, exit_func,
		"Selection of resources complete.",		"A"	},
	END_KEYS
};
#else
static	KEY_TAB rgrs_keys [] =
{
	{	"[T]AG/UNTAG",	'T', tag_func,
		"Tag/Untag resource as valid",			"A"	},
	{	NULL,		'\r', tag_func,
		"Tag/Untag resource as valid",			"A"	},
	{	NULL,		FN16, exit_func,
		"Selection of resources complete.",		"A"	},
	END_KEYS
};
#endif

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	long	dflt_hash;
	char	dflt_code [9];
	char	dflt_desc [41];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "code",		 2, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Employee Code ", " ",
		 NE, NO,  JUSTLEFT, "", "", prmr_rec.code},
	{1, LIN, "name",		 3, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Employee Name ", " ",
		 NO, NO,  JUSTLEFT, "", "", prmr_rec.name},
	{1, LIN, "dflt_code",	 4, 15, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "", "Dflt Resource ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.dflt_code},
	{1, LIN, "dflt_desc",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "              ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.dflt_desc},
	{1, LIN, "dflt_hash",	 0,  0, CHARTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.dflt_hash},
	{0, LIN, "",		 0,  0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================
| function prototypes |
=====================*/

void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int heading (int scn);
int spec_valid (int field);
void update (void);
int edit_vld_rgrs (void);
void add_prvr (void);
void SrchPrmr (char *key_val);
void SrchRgrs (char *key_val);
void save_page (char *key_val);

/*==========================
| Main Processing Routine. |
==========================*/
int 
main (
 int  argc, 
 char *argv[])

{
	SETUP_SCR (vars);

	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (!prog_exit)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
		{
			abc_unlock (prmr);
			continue;
		}

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
		{
			abc_unlock (prmr);
			continue;
		}

		prmr_rec.hhrs_hash = local_rec.dflt_hash;

		if (edit_vld_rgrs ())
		{
			abc_unlock (prmr);
			continue;
		}

		update ();
	}	/* end of input control loop	*/
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	abc_alias (rgrs2, rgrs);

	open_rec (prmr,  prmr_list, prmr_no_fields, "prmr_id_no");
	open_rec (prvr,  prvr_list, prvr_no_fields, "prvr_id_no");
	open_rec (rgrs2, rgrs_list, rgrs_no_fields, "rgrs_hhrs_hash");
	open_rec (rgrs,  rgrs_list, rgrs_no_fields, "rgrs_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (prmr);
	abc_fclose (prvr);
	abc_fclose (rgrs2);
	abc_fclose (rgrs);

	abc_dbclose (data);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		clear ();
		/*------------------------
		|%R Employee Maintenance |
		------------------------*/
		sprintf (err_str, "%s%s", "%R ", ML(mlPcMess087) );
		centre_at (0, 80, err_str );

		box (0, 1, 80, 3);

		move (0, 21);
		line (80);
		print_at(22,0, ML(mlStdMess038),comm_rec.tco_no, comm_rec.tco_name);
		print_at(22,45,ML(mlStdMess039),comm_rec.test_no, comm_rec.test_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_set (scn);
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("code"))
	{
		if (SRCH_KEY)
		{
			SrchPrmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			/*-------------------------
			| Invalid Employee Name...|
			-------------------------*/
			errmess ( ML(mlStdMess053) );
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (prmr_rec.co_no, comm_rec.tco_no);
		strcpy (prmr_rec.br_no, comm_rec.test_no);
		cc = find_rec (prmr, &prmr_rec, COMPARISON, "u");
		if (cc)
		{
			new_emp = TRUE;
			prmr_rec.hhmr_hash = 0L;
		}
		else
		{
			new_emp = FALSE;
			cc = find_hash (rgrs2, &rgrs_rec, EQUAL, "r", prmr_rec.hhrs_hash);
			if (cc)
			{
				local_rec.dflt_hash = 0L;
				strcpy (local_rec.dflt_code, "        ");
				sprintf (local_rec.dflt_desc, "%-40.40s", "No default resource");
			}
			else
			{
				local_rec.dflt_hash = prmr_rec.hhrs_hash;
				strcpy (local_rec.dflt_code, rgrs_rec.code);
				strcpy (local_rec.dflt_desc, rgrs_rec.desc);
			}
			DSP_FLD ("dflt_code");
			DSP_FLD ("dflt_desc");
			entry_exit = TRUE;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_code"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		if (strlen (clip (temp_str)) == 0)
		{
			local_rec.dflt_hash = 0L;
			strcpy (local_rec.dflt_code, "        ");
			sprintf (local_rec.dflt_desc, "%-40.40s", "No default resource");
			DSP_FLD ("dflt_code");
			DSP_FLD ("dflt_desc");
			return (EXIT_SUCCESS);
		}
		strcpy (rgrs_rec.co_no, comm_rec.tco_no);
		strcpy (rgrs_rec.br_no, comm_rec.test_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.dflt_code);
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (cc)
		{
			/*---------------------------
			| No such resource on file!!|
			---------------------------*/
			errmess ( ML(mlPcMess104) ); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		local_rec.dflt_hash = rgrs_rec.hhrs_hash;
		strcpy (local_rec.dflt_code, rgrs_rec.code);
		strcpy (local_rec.dflt_desc, rgrs_rec.desc);
		DSP_FLD ("dflt_code");
		DSP_FLD ("dflt_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
update (
 void)
{
	if (new_emp)
	{
		cc = abc_add (prmr, &prmr_rec);
		if (cc)
			file_err (cc, prmr, "DBADD");
		cc = find_rec (prmr, &prmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, prmr, "DBFIND");
		abc_unlock (prmr);
	}
	else
	{
		cc = abc_update (prmr, &prmr_rec);
		if (cc)
			file_err (cc, prmr, "DBADD");
	}

	add_prvr ();
}

int
edit_vld_rgrs (
 void)
{
	int	total_recs = 0;

	tab_open (rgrs, rgrs_keys, 5, 15, 10, FALSE);
	tab_add (rgrs, "#  Resource Description                             ");

	strcpy (rgrs_rec.co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.br_no, comm_rec.test_no);
	strcpy (rgrs_rec.code, "        ");
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.tco_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.test_no)
	)
	{
		total_recs++;
		prvr_rec.hhmr_hash = prmr_rec.hhmr_hash;
		prvr_rec.hhrs_hash = rgrs_rec.hhrs_hash;
		cc = find_rec (prvr, &prvr_rec, EQUAL, "r");
		if (cc &&
			!strcmp (rgrs_rec.code, local_rec.dflt_code))
			cc = 0;
		tab_add
		(
			rgrs, "%-1.1s %-8.8s %-40.40s %10ld",
			(cc) ? " " : "*",
			rgrs_rec.code,
			rgrs_rec.desc,
			rgrs_rec.hhrs_hash
		);
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}

	if (tab_scan (rgrs))
	{
		tab_close (rgrs, TRUE);
		return (EXIT_FAILURE);
	}
	else
		return(0);
}

/*======================================
| Add payroll valid resources records. |
======================================*/
void
add_prvr (
 void)
{
	char	wrk_str [80];

	prvr_rec.hhmr_hash = prmr_rec.hhmr_hash;
	prvr_rec.hhrs_hash = 0L;
	cc = find_rec (prvr, &prvr_rec, GTEQ, "u");
	while (!cc && prvr_rec.hhmr_hash == prmr_rec.hhmr_hash)
	{
		cc = abc_delete (prvr);
		if (cc)
			file_err (cc, prvr, "DBDELETE");

		prvr_rec.hhmr_hash = prmr_rec.hhmr_hash;
		prvr_rec.hhrs_hash = 0L;
		cc = find_rec (prvr, &prvr_rec, GTEQ, "u");
	}
	abc_unlock (prvr);

	cc = tab_get (rgrs, wrk_str, FIRST, 0);
	while (!cc)
	{
		prvr_rec.hhmr_hash = prmr_rec.hhmr_hash;
		prvr_rec.hhrs_hash =  atol (&wrk_str [52]);
		if (wrk_str [0] == '*')
		{
			cc = abc_add (prvr, &prvr_rec);
			if (cc)
				file_err (cc, prvr, "DBADD");
		}
		cc = tab_get (rgrs, wrk_str, NEXT, 0);
	}
	if (prmr_rec.hhrs_hash != 0L)
	{
		prvr_rec.hhmr_hash = prmr_rec.hhmr_hash;
		prvr_rec.hhrs_hash = prmr_rec.hhrs_hash;

		/*-------------------------------
		| NB: abc_add () errors are	    |
		| INTENTIONALLY ignored here.	|
		-------------------------------*/
		abc_add (prvr, &prvr_rec);
	}

	tab_close (rgrs, TRUE);
}

int
tag_func (
 int      iUnused, 
 KEY_TAB *psUnused)
{
	char	wrk_str [80];

	if (!tab_get (rgrs, wrk_str, EQUAL, tab_tline (rgrs)))
	{
		wrk_str [0] = (wrk_str [0] == '*') ? ' ' : '*';
		tab_update (rgrs, "%s", wrk_str);
	}
	return (EXIT_FAILURE);
}

static	int
exit_func (
 int      iUnused, 
 KEY_TAB *psUnused)
{
	return (FN16);
}

/*=======================
| Search for prmr_code	|
=======================*/
void
SrchPrmr (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Name       ");
	strcpy (prmr_rec.co_no, comm_rec.tco_no);
	strcpy (prmr_rec.br_no, comm_rec.test_no);
	sprintf (prmr_rec.code, "%-8.8s", key_val);
	cc = find_rec (prmr, &prmr_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (prmr_rec.co_no, comm_rec.tco_no) &&
		!strcmp (prmr_rec.br_no, comm_rec.test_no) &&
		!strncmp (prmr_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (prmr_rec.code, prmr_rec.name);
		if (cc)
			break;
		cc = find_rec (prmr, &prmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (prmr_rec.co_no, comm_rec.tco_no);
	strcpy (prmr_rec.br_no, comm_rec.test_no);
	sprintf (prmr_rec.code, "%-8.8s", temp_str);
	cc = find_rec (prmr, &prmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, prmr, "DBFIND");
}

/*=======================
| Search for rgrs_code	|
=======================*/
void
SrchRgrs (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.br_no, comm_rec.test_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	save_rec ("        ", "No default resource                     ");
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.tco_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.test_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.br_no, comm_rec.test_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	if (strlen (clip (temp_str)) == 0)
	{
		rgrs_rec.hhrs_hash = 0L;
		return;
	}
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");
}

/*=======================
| Search for accs_code  |
=======================*/
void
save_page (
 char *key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.br_no, comm_rec.test_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs, &rgrs_rec, GTEQ, "r");
	while
	(
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.tco_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.test_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.co_no, comm_rec.tco_no);
	strcpy (rgrs_rec.br_no, comm_rec.test_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs, "DBFIND");
}
