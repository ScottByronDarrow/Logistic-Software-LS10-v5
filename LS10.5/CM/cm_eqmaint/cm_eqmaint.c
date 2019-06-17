/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_eqmaint.c,v 5.3 2002/01/10 09:02:10 scott Exp $
|  Program Name  : (cm_eqmaint.c) 
|  Program Desc  : (Plant/Equipement Rate master maintenance)
|---------------------------------------------------------------------|
|  Author        : Anneliese Allen | Date Written  : 25/02/93         |
|---------------------------------------------------------------------|
| $Log: cm_eqmaint.c,v $
| Revision 5.3  2002/01/10 09:02:10  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_eqmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_eqmaint/cm_eqmaint.c,v 5.3 2002/01/10 09:02:10 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cmeqRecord	cmeq_rec;
struct inumRecord	inum_rec;

char	*data	= "data";

int 	newCode = FALSE;
int		envSerialItemsOk;

extern	int		TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	desc [21];
	double	cost;
	double	rate;
	char	ser_no [26];
	long	dop;
	double	cop;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "eq_num",	4, 2, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Plant/Equipment Code  ", " ",
		 NE, NO,  JUSTLEFT, "", "", cmeq_rec.eq_name},
	{1, LIN, "desc",	5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description           ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "unit",	6, 2, CHARTYPE,
		"AAAA", "          ",
		" ", "", "Unit of Measure       ", " ",
		YES, NO,  JUSTLEFT, "", "", inum_rec.uom},
	{1, LIN, "rate",	7, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", "Rate (amount/unit)    ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.rate},
	{1, LIN, "cost",	8, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "", "Cost (amount/unit)    ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.cost},
	{1, LIN, "ser_no",	9, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ","Serial Number         ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.ser_no},
	{1, LIN, "dop",	10, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " (char *)local_rec.lsystemDate", "Date of purchase      ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.dop},
	{1, LIN, "cop",	11, 2, MONEYTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0.00", "Cost of purchase      ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.cop},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO,  JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function prototypes.    
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
int		Update			(void);
int		heading			(int);
int		SrchCmeq		(char *);
int		SrchInum		(char *);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char * argv [])
{
	char	*sptr;

	TruePosition = TRUE;

	SETUP_SCR (vars);

	if (argc < 2)
	{
		print_at (0,0, ML (mlStdMess071), argv [0]);
		return (argc);
	}

	init_scr ();			/*  sets terminal from termcap	*/
	_set_masks (argv [1]);

	/*
	 * open main database files.
	 */
	OpenDB ();

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*
	 * Validate is serial items allowed.
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envSerialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (envSerialItemsOk)
		FLD ("ser_no") 	= YES;
	else
		FLD ("ser_no") 	= ND;

	/*
	 * Beginning of input control loop.
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		newCode 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		set_tty ();

		/*
		 * Enter screen 1 linear input.
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (!newCode)
		{
			strcpy (cmeq_rec.co_no, comm_rec.co_no);
			cc = find_rec (cmeq, &cmeq_rec, COMPARISON, "u");

			heading (1);
			scn_display (1);
		}

		edit_all ();
		if (restart)
		{
			continue;
		}

		/*
		 * Update stock master record. 
		 */
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (cmeq,  cmeq_list, CMEQ_NO_FIELDS, "cmeq_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmeq);
	abc_fclose (inum);

	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	/* 
	 * Check for existance of record
	 */
	if (LCHECK ("eq_num"))
	{
		if (SRCH_KEY)
		{
			SrchCmeq (temp_str);
			return (EXIT_SUCCESS);
		}
		newCode = FALSE;
		strcpy (cmeq_rec.co_no, comm_rec.co_no);
		cc = find_rec (cmeq, &cmeq_rec, EQUAL, "r");
		if (cc)
			newCode = TRUE;
		else
		{
			entry_exit = 1;
			sprintf (local_rec.desc, "%20.20s", cmeq_rec.desc);
			local_rec.rate = cmeq_rec.rate;
			local_rec.cost = cmeq_rec.cost;
			abc_selfield (inum, "inum_hhum_hash");
			inum_rec.hhum_hash	= cmeq_rec.inum_hash;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (cc)
				file_err (cc, inum, "DBFIND");

			DSP_FLD ("desc");
			DSP_FLD ("unit");
			DSP_FLD ("rate");
			DSP_FLD ("cost");
		}
		abc_selfield (inum, "inum_uom");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("desc"))
	{
		strcpy (cmeq_rec.desc, local_rec.desc);
		return (EXIT_SUCCESS);
	}

	/*
	 * Check for valid unit of measure
	 */
	if (LCHECK ("unit"))
	{
		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		else
			cmeq_rec.inum_hash = inum_rec.hhum_hash;
		return (EXIT_SUCCESS);
	}

	/*
	 * Check charge out rate
	 */
	if (LCHECK ("rate"))
	{
		if (local_rec.rate < 0.00)
		{
			errmess (ML (mlCmMess044));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cmeq_rec.rate = local_rec.rate;
		return (EXIT_SUCCESS);
	}

	/*
	 * Check cost
	 */
	if (LCHECK ("cost"))
	{
		if (local_rec.cost < 0.00)
		{
			errmess (ML (mlCmMess045));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		cmeq_rec.cost = local_rec.cost;
		return (EXIT_SUCCESS);
	}

	/*
	 * Check cost
	 */
	if (LCHECK ("ser_no"))
	{
		if (dflt_used)
			sprintf (local_rec.ser_no, "%25.25s", " ");

		strcpy (cmeq_rec.serial, local_rec.ser_no);
		return (EXIT_SUCCESS);
	}

	/*
	 * Check cost 
	 */
	if (LCHECK ("dop"))
	{
		if (dflt_used)
			local_rec.dop = local_rec.lsystemDate;
		cmeq_rec.dop = local_rec.dop;
		return (EXIT_SUCCESS);
	}

	/*
	 * Check cost
	 */
	if (LCHECK ("cop"))
	{
		if (dflt_used)
			local_rec.cop = 0.00;

		cmeq_rec.cop = local_rec.cop;
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * Update All inventory files.
 */
int
Update (void)
{
	clear ();

	/*
	 * Add inventory master record.
	 */
	if (newCode)
	{
		strcpy (cmeq_rec.co_no, comm_rec.co_no);
		strcpy (cmeq_rec.desc, local_rec.desc);
		cmeq_rec.rate = local_rec.rate;
		cmeq_rec.cost = local_rec.cost;
		strcpy (cmeq_rec.serial, local_rec.ser_no);
		cmeq_rec.dop	= local_rec.dop;
		cc = abc_add (cmeq, &cmeq_rec);
		if (cc)
	 		file_err (cc, cmeq, "DBUPDATE");
	}
	else
	{
		cc = abc_update (cmeq, &cmeq_rec);
		if (cc)
	 		file_err (cc, cmeq, "DBUPDATE");
	}
	abc_unlock (cmeq);

	return (EXIT_SUCCESS);
}

/*
 * Heading concerns itself with clearing the screen,painting the 
 * screen overlay in preparation for input.                     
 */
int
heading (
 int	scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCmMess046),24,0,1);
		pr_box_lines (1);

		line_at (1,0,80);
		line_at (21,0,80);

		strcpy (err_str, ML (mlStdMess038));
		print_at (22,0, err_str , comm_rec.co_no, comm_rec.co_name);

		line_cnt = 0;
		scn_write (scn);
	}
	else
		abc_unlock (cmeq);

	return (EXIT_SUCCESS);
}

int
SrchCmeq (
 char *	keyValue)
{
	_work_open (10,0,40);
	save_rec ("#Code","#Plant/Equipment Code Description");

	strcpy (cmeq_rec.co_no, comm_rec.co_no);
	cc = find_rec (cmeq, &cmeq_rec, GTEQ, "r");
	while (!cc &&!strncmp (cmeq_rec.eq_name, keyValue, strlen (keyValue))
		   && !strcmp (cmeq_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmeq_rec.eq_name, cmeq_rec.desc);
		if (cc)
			break;
		cc = find_rec (cmeq, &cmeq_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_SUCCESS);
	strcpy (cmeq_rec.co_no, comm_rec.co_no);
	sprintf (cmeq_rec.eq_name, "%-10.10s", temp_str);
	cc = find_rec (cmeq, &cmeq_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, cmeq, "DBFIND");
	sprintf (local_rec.desc, "%20.20s", cmeq_rec.desc);
	local_rec.rate = cmeq_rec.rate;
	local_rec.cost = cmeq_rec.cost;
	return (EXIT_SUCCESS);
}

int
SrchInum (
	char	*keyValue)
{
	_work_open (4,0,40);
	save_rec ("#UOM","#Description");

	strcpy (inum_rec.uom, "    ");
	strncpy (inum_rec.uom, keyValue, strlen (keyValue));
	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while (!cc &&!strncmp (inum_rec.uom, keyValue, strlen (keyValue)))
	{
		cc = save_rec (inum_rec.uom, inum_rec.desc);
		if (cc)
			break;
		cc = find_rec (inum, &inum_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (EXIT_SUCCESS);

	sprintf (inum_rec.uom, "%-4.4s", temp_str);
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
	 	file_err (cc, inum, "DBFIND");

	return (EXIT_SUCCESS);
}
