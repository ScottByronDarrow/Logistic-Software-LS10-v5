/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_wip_upd.c,v 5.3 2002/01/21 04:57:36 scott Exp $
|  Program Name  : (cm_wip_upd.c)
|  Program Desc  : (Contract WIP Status Update)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written  : 02/03/93         |
|---------------------------------------------------------------------|
| $Log: cm_wip_upd.c,v $
| Revision 5.3  2002/01/21 04:57:36  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_wip_upd.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_wip_upd/cm_wip_upd.c,v 5.3 2002/01/21 04:57:36 scott Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>

#define	COMPANY		2
#define	BRANCH		1
#define	USER		0

#include	"schema"

struct commRecord	comm_rec;
struct cmitRecord	cmit_rec;
struct cmitRecord	cmit2_rec;
struct cmcdRecord	cmcd_rec;
struct cmhrRecord	cmhr_rec;
struct cmwsRecord	cmws_rec;
struct cmwsRecord	cmws2_rec;

	char	*cmit2 = "cmit2", 
			*cmws2 = "cmws2", 
			*data  = "data";

	char	cbranchNo [3];

	int	envCmAutoCon;

	extern	int		TruePosition;

/*
 * Local & Screen Structures. 
 */

struct {
		char	dummy [11];
		char	systemDate [11];
		long	lsystemDate;
		long	new_wip_date;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "cont_no", 	 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "", "Contract No.           ", "Enter Contract Number For Maintenance, Full Search Available. ", 
		 NE, NO, JUSTLEFT, "", "", cmhr_rec.cont_no}, 
	{1, LIN, "cont_desc", 	 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description            ", "", 
		 NA, NO, JUSTLEFT, "", "", cmcd_rec.text}, 
	{1, LIN, "old_iss_to", 	7, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", "Current Issue To Code  ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmit_rec.issto}, 
	{1, LIN, "old_iss_name", 	7, 43, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cmit_rec.iss_name}, 
	{1, LIN, "old_wip_st", 	8, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Current WIP Status     ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmhr_rec.wip_status}, 
	{1, LIN, "old_wip_desc", 	8, 43, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cmws_rec.desc}, 
	{1, LIN, "old_wip_date", 	9, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Current WIP Date       ", "", 
		NA, NO, JUSTLEFT, " ", "", (char *)&cmhr_rec.wip_date}, 
	{1, LIN, "new_iss_to", 	11, 2, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", "", "New Issue To Code      ", "Enter Code For New Issue To Person", 
		 YES, NO, JUSTLEFT, "", "", cmit2_rec.issto}, 
	{1, LIN, "new_iss_name", 	11, 43, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cmit2_rec.iss_name}, 
	{1, LIN, "new_wip_st", 	12, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "New WIP Status         ", "Enter New WIP Status", 
		 YES, NO, JUSTLEFT, "", "", cmws2_rec.wp_stat}, 
	{1, LIN, "new_wip_desc", 	12, 43, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", " ", 
		 NA, NO, JUSTLEFT, "", "", cmws2_rec.desc}, 
	{1, LIN, "new_wip_date", 	13, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "New WIP Date           ", "Defalut - Today's Date", 
		YES, NO, JUSTLEFT, " ", "", (char *)&local_rec.new_wip_date}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};


/*
 * Local function prototypes 
 */
int		spec_valid		(int);
int		heading			(int);
void	shutdown_prog	(void);
void	OpenDB			(void);
void	CloseDB			(void);
void	SrchCmhr		(char *);
void	Update			(void);
void	LoadData		(void);
void	SrchCmit2		(char *);
void	SrchCmws2		(char *);


/*
 * Main Processing Routine. 
 */
int
main (
 int	argc, 
 char *	argv [])
{

	char *	sptr = chk_env ("CM_AUTO_CON");

	TruePosition	=	TRUE;

	if (sptr)
		envCmAutoCon = atoi (sptr);
	else
		envCmAutoCon = COMPANY;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	OpenDB ();

	strcpy (cbranchNo, (envCmAutoCon != COMPANY) ? comm_rec.est_no : " 0");

	init_scr 	();
	set_tty 	(); 
	set_masks 	();

	prog_exit 	= FALSE;

	while (!prog_exit)
	{
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		entry_exit	= FALSE;	
		edit_exit	= FALSE;
		prog_exit 	= FALSE;
	
		init_vars (1);
		/*
		 * Enter screen 1 linear input. 
		 */
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
 * Open Database Files. 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cmit2, cmit);

	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmit, cmit_list, CMIT_NO_FIELDS, "cmit_hhit_hash");
	open_rec (cmit2, cmit_list, CMIT_NO_FIELDS, "cmit_id_no");
	open_rec (cmws, cmws_list, CMWS_NO_FIELDS, "cmws_id_no");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
}

/*
 * Close Database Files. 
 */
void
CloseDB (void)
{
	abc_fclose (cmhr);
	abc_fclose (cmcd);
	abc_fclose (cmit);
	abc_fclose (cmit2);
	abc_fclose (cmws);

	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("new_wip_st"))
	{
		if (SRCH_KEY)
		{
			SrchCmws2 (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmws2_rec.co_no, comm_rec.co_no);

		cc = find_rec (cmws, &cmws2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmws, "DBFIND");
		
		DSP_FLD ("new_wip_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_iss_to"))
	{
		if (SRCH_KEY)
		{
			SrchCmit2 (temp_str);
			return (EXIT_SUCCESS);
		}
		else
		{
			strcpy (cmit2_rec.co_no, comm_rec.co_no);
			cc = find_rec (cmit2, &cmit2_rec, EQUAL, "r");
		}
		if (cc)
		{
			print_mess (ML (mlCmMess014));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		DSP_FLD ("new_iss_name");
		return (EXIT_SUCCESS);

	}

	if (LCHECK ("cont_no"))
	{
		strcpy (cmhr_rec.co_no, comm_rec.co_no);

		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.cont_no, zero_pad (cmhr_rec.cont_no, 6));
		strcpy (cmhr_rec.co_no, comm_rec.co_no);	
		strcpy (cmhr_rec.br_no, cbranchNo);	
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		LoadData ();
		scn_display (1);
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


void
SrchCmhr (
 char *	keyValue)
{
	_work_open (6,0,30);
	save_rec ("#No.", "#Customer Order No.");
	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);	
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);
	cc = find_rec (cmhr, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		if (strcmp (cmhr_rec.br_no, cbranchNo))
				break;

		save_rec (cmhr_rec.cont_no, cmhr_rec.cus_ref);
		cc = find_rec (cmhr, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, cbranchNo);
	sprintf (cmhr_rec.cont_no, "%6.6s", temp_str);

	if (envCmAutoCon == BRANCH || envCmAutoCon == USER)
		strcpy (cmhr_rec.br_no, comm_rec.est_no);

	cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
Update (
 void)
{
	strcpy (cmhr_rec.wip_status, cmws2_rec.wp_stat);
	cmhr_rec.wip_date = local_rec.new_wip_date;
	cmhr_rec.it_date = local_rec.lsystemDate;
	cmhr_rec.hhit_hash = cmit2_rec.hhit_hash;

	cc = abc_update (cmhr, &cmhr_rec);
	if (cc)
		file_err (cc, cmhr, "DBUPDATE");
}

void
LoadData (
 void)
{
	strcpy (cmws_rec.co_no, comm_rec.co_no);
	strcpy (cmws_rec.wp_stat, cmhr_rec.wip_status);
	cc = find_rec (cmws, &cmws_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmws, "DBFIND");

	if (cmhr_rec.hhit_hash)
	{
		cmit_rec.hhit_hash	=	cmhr_rec.hhit_hash;
		cc = find_rec (cmit, &cmit_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cmit, "DBFIND");
	}

	cmcd_rec.line_no = 0;
	cmcd_rec.stat_flag [0] = 'D';
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;

	cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cmcd, "DBFIND");
}

void
SrchCmit2 (
	char	*keyValue)
{
	_work_open (10, 0, 40);
	save_rec ("#Issue To", "#Issue To Name");

	strcpy (cmit2_rec.co_no, comm_rec.co_no);
	sprintf (cmit2_rec.issto, "%-10.10s", keyValue);
	cc = find_rec (cmit2, &cmit2_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmit2_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmit2_rec.issto, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmit2_rec.issto, cmit2_rec.iss_name);
		if (cc)
			break;

		cc = find_rec (cmit2, &cmit2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmit2_rec.co_no, comm_rec.co_no);
	sprintf (cmit2_rec.issto, "%-10.10s", temp_str);
	cc = find_rec (cmit2, &cmit2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmit2, "DBFIND");
}

void
SrchCmws2 (
	char	*keyValue)
{
	_work_open (4, 0, 40);
	save_rec ("#Stat", "#WIP Status Description ");

	strcpy (cmws2_rec.co_no, comm_rec.co_no);
	sprintf (cmws2_rec.wp_stat, "%-4.4s", keyValue);
	cc = find_rec (cmws, &cmws2_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmws2_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmws2_rec.wp_stat, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmws2_rec.wp_stat, cmws2_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmws, &cmws2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmws2_rec.co_no, comm_rec.co_no);
	sprintf (cmws2_rec.wp_stat, "%-4.4s", temp_str);
	cc = find_rec (cmws, &cmws2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmws2, "DBFIND");
}
int
heading (
	int	scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	swide ();

	rv_pr (ML (mlCmMess059), 50, 0, 1);

	box (0, 3, 132, 10);
	line_at (1, 0, 132);
	line_at (6, 1, 131);
	line_at (10, 1, 131);
	line_at (21, 1, 131);

	print_at (22, 1, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}
