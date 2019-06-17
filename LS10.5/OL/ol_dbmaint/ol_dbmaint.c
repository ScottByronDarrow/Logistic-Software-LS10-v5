/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( ol_dbmaint.c   )                                 |
|  Program Desc  : ( On Line Customer Maintenance                  )   |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (10/11/95)      | Modified  by : Anneliese Allen   |
|                                                                     |
|    Date        : Call No.   - Comments.                             |
|  (10/11/95)    : ASL 12475  - Initial Conception.                   |
|                :                                                    |
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: ol_dbmaint.c,v $
| Revision 5.2  2001/08/09 09:14:17  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:44  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:46  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:16:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:49:23  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 09:02:22  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.7  1999/11/17 06:40:22  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.6  1999/11/08 08:42:25  scott
| Updated due to warning errors when compiling using -Wall flag.
|
| Revision 1.5  1999/09/29 10:11:24  scott
| Updated to be consistant on function names.
|
| Revision 1.4  1999/09/20 05:51:22  scott
| Updated from Ansi Project.
|
| Revision 1.3  1999/09/10 02:09:33  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.2  1999/06/15 09:39:17  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_dbmaint.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_dbmaint/ol_dbmaint.c,v 5.2 2001/08/09 09:14:17 scott Exp $";

#include <pslscr.h>
#include <minimenu.h>

#define	SLEEP_TIME	3

#define	S_DUMM	 0
#define	S_HEAD	 1

#define	SEL_SAVE	 0
#define	SEL_IGNORE	 1
#define	SEL_RESTRT	-1
#define	SEL_DELETE	 2


	/*=====================+
	 | System Common File. |
	 +=====================*/
#define	COMM_NO_FIELDS	5

	struct dbview	comm_list [COMM_NO_FIELDS] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"}
	};

	struct tag_commRecord
	{
		int		term;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
	}	comm_rec;

	/*===============================+
	 | Valid On Line Cash Customers. |
	 +===============================*/
#define	CUOC_NO_FIELDS	4

	struct dbview	cuoc_list [CUOC_NO_FIELDS] =
	{
		{"cuoc_co_no"},
		{"cuoc_est_no"},
		{"cuoc_hhcu_hash"},
		{"cuoc_type"}
	};

	struct tag_cuocRecord
	{
		char	co_no [3];
		char	est_no [3];
		long	hhcu_hash;
		char	type [2];
	}	cuoc_rec;

	/*===================================+
	 | Customer Master File Base Record. |
	 +===================================*/
#define	CUMR_NO_FIELDS	6

	struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"}
	};

	struct tag_cumrRecord
	{
		char	cm_co_no [3];
		char	cm_est_no [3];
		char	cm_dbt_no [7];
		long	cm_hhcu_hash;
		char	cm_name [41];
		char	cm_acronym [10];
	}	cumr_rec;

	int		newItem;

	char	*data	= "data";
	char	*comm	= "comm";
	char	*cumr	= "cumr";
	char	*cuoc	= "cuoc";

static	struct	var vars[] =
{
	{S_HEAD, LIN, "cust", 2, 15, CHARTYPE,
		"UUUUUU", "          ",
		"", " ", "Customer : ", "",
		 NE, NO,  JUSTLEFT, "", "", cumr_rec.cm_dbt_no},
	{S_HEAD, LIN, "desc",	3, 15, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "             ",
		" ", " ", "Name     : ", "",
		NA, NO,  JUSTLEFT, "", "", cumr_rec.cm_name},
	{S_HEAD, LIN, "type",	4, 15, CHARTYPE,
		"U", "             ",
		" ", "R", "Type     : ", "Cash or cRedit card",
		YES, NO,  JUSTLEFT, "CR", "", cuoc_rec.type},

	{S_DUMM, TAB, "",		0, 0, INTTYPE,
		"A", "          ",
		" ", "", "", " ",
		YES, NO, JUSTRIGHT, " ", " ", NULL}
};

#include	<std_decs.h>
void OpenDB (void);
void CloseDB (void);
int heading (int scn);
int spec_valid (int field);
void UpdateMenu (void);
int DeleteData (void);
void UpdateData (void);
#include <FindCumr.h>

	int		envVarDbCo		=	0;
	char	branchNumber[3];

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int argc,
	char *argv [])
{
	char	*sptr;
	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	  */
	set_tty ();
	set_masks ();

	OpenDB ();
	clear ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.test_no : " 0");

	/*---------------------------------
	| Beginning of input control loop |
	---------------------------------*/
	prog_exit = FALSE;
	while (!prog_exit)
	{
		/*----------------------
		| Reset Control Flags  |
		----------------------*/
		entry_exit = FALSE;
		edit_exit  = FALSE;
		search_ok  = TRUE;
		prog_exit  = FALSE;
		restart    = FALSE;

		init_vars (S_HEAD);

		heading (S_HEAD);
		entry (S_HEAD);

		if (restart || prog_exit)
			continue;

		edit (S_HEAD);

		if (restart)
			continue;

		UpdateMenu ();
	}

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

void
OpenDB (
	void)
{
	abc_dbopen (data);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (cuoc, cuoc_list, CUOC_NO_FIELDS, "cuoc_id_no");
}

void
CloseDB (
	void)
{
	abc_fclose (cumr);
	abc_fclose (cuoc);

	abc_dbclose (data);
}

/*=========================
| Display Screen Heading  |
=========================*/
int
heading (
	int scn)
{
	if (!restart) 
	{
		clear ();

		if (scn != cur_screen)
			scn_set (scn);

		print_at (0, 22, "%R On Line Customer Maintenance ");
		box (0, 1, 80, 3);

		line_at (21, 0, 80);
		print_at (22, 0, "Company : %s  %-25.25s  Branch : %s  %25.25s",
				  comm_rec.tco_no,
				  comm_rec.tco_name,
				  comm_rec.test_no,
				  comm_rec.test_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

int
spec_valid (
	int field)
{
	if (LCHECK ("cust"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.tco_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			print_mess (" Default not available \007");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cumr_rec.cm_co_no, comm_rec.tco_no);
		strcpy (cumr_rec.cm_est_no, branchNumber);
		strcpy (cumr_rec.cm_dbt_no, pad_num (cumr_rec.cm_dbt_no));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess ("Customer is Not on Customer Master File\007");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cuoc_rec.hhcu_hash = cumr_rec.cm_hhcu_hash;
		cc = find_rec (cuoc, &cuoc_rec, EQUAL, "w");
		newItem = cc;
		if (!cc)
		{
			DSP_FLD ("type");
			entry_exit = TRUE;
		}

		DSP_FLD ("desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
UpdateMenu (
	void)
{
	MENUTAB new_menu [] =
	{
		{ " 1. ADD           ",
		  " Add the new record to the database. " },
		{ " 2. SEL_IGNORE        ",
		  " Ignore entry of new record. " },
		{ ENDMENU }
	};

	MENUTAB upd_menu [] =
	{
		{ " 1. UPDATE        ",
		  " Update current record with changes made. " },
		{ " 2. SEL_IGNORE        ",
		  " Ignore changes made to current record. " },
		{ " 3. SEL_DELETE        ",
		  " Delete current record. " },
		{ ENDMENU }
	};

	for (;;)
	{
		mmenu_print ("  UPDATE OPTIONS  ", newItem ? new_menu : upd_menu, 0);
		switch (mmenu_select (newItem ? new_menu : upd_menu))
		{
		case SEL_SAVE :
			UpdateData ();
			return;

		case SEL_IGNORE :
		case SEL_RESTRT : /* Ignore Changes */
			restart = TRUE;
			return;

		case SEL_DELETE :
			if (newItem)
				break;
			cc = DeleteData ();
			if (!cc)
				return;
			else
				break;

		default :
			break;
		}
	}
}

int
DeleteData (
	void)
{
	if (newItem)
		return (EXIT_SUCCESS);

	cc = abc_delete (cuoc);
	if (cc)
		file_err (cc, cuoc, "DBDELETE");

	return (EXIT_SUCCESS);
}

void
UpdateData (
	void)
{
	if (newItem)
	{
		cc = abc_add (cuoc, &cuoc_rec);
		if (cc)
			file_err (cc, cuoc, "DBADD");
	}
	else
	{
		cc = abc_update (cuoc, &cuoc_rec);
		if (cc)
			file_err (cc, cuoc, "DBUPDATE");
	}
}

