/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_cnt_desc.c,v 5.1 2002/01/10 09:25:00 scott Exp $
|  Program Name  : (cm_cnt_desc.c)
|  Program Desc  : (Maintain ongoing contract description)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (19/03/93)       |
|---------------------------------------------------------------------|
| $Log: cm_cnt_desc.c,v $
| Revision 5.1  2002/01/10 09:25:00  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_cnt_desc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_cnt_desc/cm_cnt_desc.c,v 5.1 2002/01/10 09:25:00 scott Exp $";

#define MAXLINES	1000
#define	TXT_REQD
#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_cm_mess.h>
		
#define	COMPANY	2
#define	BRANCH	1

#define	SCN_HEADER		1
#define	SCN_DESC		2

#include	"schema"

struct commRecord	comm_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;

	char	*data     = "data";

	char	branchNo [3];

/*
 * Local & Screen Structures.
 */
struct {
	char	contractNo [7];
	char	contractDesc [7][71];
	char	extraDesc [71];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{SCN_HEADER, LIN, "contractNo", 4, 17, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Contract No. :", "Enter Contract Number. ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.contractNo},
	{SCN_HEADER, LIN, "desc1", 6, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [0]},
	{SCN_HEADER, LIN, "desc2", 7, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [1]},
	{SCN_HEADER, LIN, "desc3", 8, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [2]},
	{SCN_HEADER, LIN, "desc4", 9, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [3]},
	{SCN_HEADER, LIN, "desc5", 10, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [4]},
	{SCN_HEADER, LIN, "desc6", 11, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [5]},
	{SCN_HEADER, LIN, "desc7", 12, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contractDesc [6]},

	{SCN_DESC, TXT, "",	 14, 2, 0,
		"", "          ",
		"", "", " Ongoing Contract Description ", "",
		 5, 70,  500, "", "", local_rec.extraDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function prototypes.    
 */
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB		 	(void);
int		spec_valid		(int);
void	SrchCmhr		(char *);
int		Update			(void);
int		heading			(int);


/*
 * Main Processing Routine.
 */
int
main (
 int	argc,
 char *	argv [])
{
	int	auto_con_no;
	char	*sptr;

	SETUP_SCR (vars);

	OpenDB ();

	/*
	 * Check contract number level.
	 */
	sptr = chk_env ("CM_AUTO_CON");
	auto_con_no = (sptr == (char *)0) ? COMPANY : atoi (sptr);
	strcpy (branchNo, (auto_con_no == COMPANY) ? " 0" : comm_rec.est_no);

	/*
	 * setup required parameters. 
	 */
	init_scr 	();
	set_tty 	();
	set_masks 	();	

	while (prog_exit == 0)
	{
		search_ok 	= TRUE;
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		init_ok 	= TRUE;
		eoi_ok 		= FALSE;
		restart 	= FALSE;

		init_vars (SCN_HEADER);
		init_vars (SCN_DESC);
		lcount [SCN_DESC] = 0;

		heading (SCN_HEADER);
		entry 	(SCN_HEADER);
		if (prog_exit || restart)
			continue;

		heading 	(SCN_DESC);
		scn_display (SCN_DESC);
		edit 		(SCN_DESC);
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
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (cmhr);
	abc_fclose (cmcd);
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	if (LCHECK ("contractNo"))
	{
		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", zero_pad (local_rec.contractNo,6));
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		scn_set (SCN_DESC);
		lcount [SCN_DESC] = 0;
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		while (!cc &&
		       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		       !strcmp (cmcd_rec.stat_flag, "D"))
		{
			if (cmcd_rec.line_no < 7)
			{
			       sprintf (local_rec.contractDesc [cmcd_rec.line_no],
					"%-70.70s", cmcd_rec.text);
			}
			else
			{
				sprintf (local_rec.extraDesc, "%-70.70s", cmcd_rec.text);
				putval (lcount [SCN_DESC]++);
			}

			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}

		scn_set (SCN_HEADER);
		scn_display (SCN_HEADER);

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCmhr (
 char *	key_val)
{
	_work_open (6,0,40);
	save_rec ("#No", "#Customer Order No.");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", key_val);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, key_val,strlen (key_val)))
	{
		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);

		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

int
Update (void)
{
	int	i;

	/*
	 * Delete old status D lines past line 7.
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 7;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
	while (!cc &&
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       !strcmp (cmcd_rec.stat_flag, "D"))
	{
		cc = abc_delete (cmcd);
		if (cc)
			file_err (cc, cmcd, "DBDELETE");

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 7;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "u");
	}

	/*
	 * Add new cmcd records. 
	 */
	scn_set (SCN_DESC);
	for (i = 0; i < lcount [SCN_DESC]; i++)
	{
		getval (i);

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = i + 7;
		sprintf (cmcd_rec.text, "%-70.70s", local_rec.extraDesc);
		cc = abc_add (cmcd, &cmcd_rec);
		if (cc)
			file_err (cc, cmcd, "DBADD");
	}

	return (EXIT_SUCCESS);
}

/*
 * Display Heading. 
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

		line_at (1,0,80);

		centre_at (0, 80, ML (mlCmMess065));

		/*
		 * Screen 1 box
		 */
		box (2, 3, 72, 9);
		
		line_at (5, 3, 71);

		us_pr (ML (mlCmMess102), (76 - strlen (ML (mlCmMess102))) / 2, 5, 1);

		line_cnt = 0;
		scn_write (scn);

		if (scn == SCN_DESC)
		{
			scn_set 	(SCN_HEADER);
			scn_write 	(SCN_HEADER);
			scn_display (SCN_HEADER);
			scn_set 	(SCN_DESC);
		}
	}
	line_at (21,0,80);

	print_at (22,0, (ML (mlStdMess038)),comm_rec.co_no, comm_rec.co_name);
	return (EXIT_SUCCESS);
}
