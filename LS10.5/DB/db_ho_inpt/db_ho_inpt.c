/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_ho_inpt.c,v 5.9 2002/07/24 08:38:48 scott Exp $
|  Program Name  : (db_ho_inpt.c)
|  Program Desc  : (Charge to Head Office Customer (Input/Maint)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 06/03/87         |
|---------------------------------------------------------------------|
| $Log: db_ho_inpt.c,v $
| Revision 5.9  2002/07/24 08:38:48  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/18 06:24:14  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.7  2002/07/09 03:48:18  scott
| S/C 004048 - Deleted lines not displayed correctly on GUI.
|
| Revision 5.6  2002/06/26 04:34:16  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/06/26 04:26:51  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2001/08/09 09:05:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/09 01:36:56  scott
| RELEASE 5.0
|
| Revision 5.2  2001/08/06 23:22:04  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:11  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_ho_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_ho_inpt/db_ho_inpt.c,v 5.9 2002/07/24 08:38:48 scott Exp $";

#define 	MAXWIDTH	150
#define 	MAXLINES	2000
#define		MAXDELETED	 (4 * MAXLINES)
#include <ml_std_mess.h>
#include <ml_db_mess.h>
#include <pslscr.h>

#define	MAX_I(a,b)	 ( (a) < (b) ? (b) : (a))

	/*
	 * Special fields and flags
	 */
   	int  	NewCode		= FALSE,	
			envDbCo		= 0,
			envDbFind	= 0;

	int		numDeleted	=	0;
	long	deletedList [MAXDELETED];

	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct cumrRecord	cumr_ho;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cudtRecord	cudt_rec;

	int		envVarDbMcurr;

struct	storeRec {
	long	hhcuHash;
	int		existing;
} store [MAXLINES];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	childCustomerNo [7];
	char	childCustomerName [41];
	char	childAddress [131];
	long	childHhcuHash;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "dbtrno",	 4, 21, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer No.", " ",
		 NE, NO,  JUSTLEFT, "", "", cumr_ho.dbt_no},
	{1, LIN, "name",	 5, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name  ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.dbt_name},
	{1, LIN, "ch1",	 7, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Charge Address 1", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.ch_adr1},
	{1, LIN, "ch2",	 8, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Charge Address 2", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.ch_adr2},
	{1, LIN, "ch3",	 9, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Charge Address 3", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.ch_adr3},
	{1, LIN, "dl1",	11, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Address 1", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.dl_adr1},
	{1, LIN, "dl2",	12, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Address 2", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.dl_adr2},
	{1, LIN, "dl3",	13, 21, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Delivery Address 3", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_ho.dl_adr3},

	{2, TAB, "ho_dbt_no",	MAXLINES, 4, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", " Customer No. ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.childCustomerNo},
	{2, TAB, "ho_dbt_name",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Customer Name.             ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.childCustomerName},
	{2, TAB, "ho_ch_adr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", " Address                              ", " ",
		 NA, NO,  JUSTLEFT, " ", "", local_rec.childAddress},
	{2, TAB, "childHhcuHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", " ", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.childHhcuHash},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindCumr.h>

/*
 * Local Function Prototypes.
 */
void	shutdown_prog 			 (void);
void 	OpenDB 					 (void);
void 	CloseDB 				 (void);
int 	spec_valid 				 (int);
void 	LoadChildren 			 (void);
int 	DeleteLine 				 (void);
void 	Update 					 (void);
void 	UpdateCustomer 			 (long, long);
void 	tab_other 				 (int);
int 	MayRemoveCustomer 		 (long);
void 	AddToDeletedList 		 (long);
void 	RemoveFromDeletedList 	 (long);
int 	heading 				 (int);

/*
 * Main Processing Routine. 
 */
int
main (
 int                argc,
 char*              argv [])
{
	int		i;
	char	*sptr;

	SETUP_SCR (vars);


	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/

	sptr = chk_env ("DB_CO");
	envDbCo 	= (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("DB_FIND");
	envDbFind 	= (sptr == (char *)0) ? 0 : atoi (sptr);

	OpenDB ();

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	tab_row = 7;

	/*-----------------------------------
	| Beginning of input control loop . |
	-----------------------------------*/
	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		lcount [2]	= 0;
		FLD ("ho_dbt_no") = YES;

		numDeleted = 0;

		for (i = 0; i < MAXLINES; i++)
		{
			store [i].hhcuHash = 0L;
			store [i].existing = FALSE;
		}

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*-------------------------------
		| Load Tabular, set new_code	|
		-------------------------------*/
		LoadChildren ();

		/*-------------------------------
		| Enter screen 2 Tabular input. |
		-------------------------------*/
		if (NewCode == TRUE)
		{
			heading (2);
			entry (2);
		}

		if (restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		/*-----------------
		| Update records. |
		-----------------*/
		Update ();
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence	
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

	abc_alias ("cumr_ho", cumr);
	abc_alias ("cumr2",   cumr);

	open_rec (cumr,cumr_list,CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
													    	: "cumr_id_no3");
	open_rec ("cumr_ho",cumr_list,CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
							       						   	   : "cumr_id_no3");
	open_rec (cuin,cuin_list,CUIN_NO_FIELDS, "cuin_hhcu_hash");
	open_rec (cuhd,cuhd_list,CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cudt,cudt_list,CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec ("cumr2",cumr_list,CUMR_NO_FIELDS, "cumr_hhcu_hash");
}	

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose ("cumr2");
	abc_fclose ("cumr_ho");
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
	int 	i;
	char	tmp_addr [131];

	/*
	 * Validate Customer Number And Allow Search. 
	 */
	if (LCHECK ("dbtrno"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
			
		strcpy (cumr_ho.co_no,comm_rec.co_no);
		strcpy (cumr_ho.est_no, branchNumber);
		strcpy (cumr_ho.dbt_no,pad_num (cumr_ho.dbt_no));
		cc = find_rec ("cumr_ho", &cumr_ho, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (cumr_ho.ho_dbt_hash != 0L)
		{
			print_mess (ML (mlStdMess095));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate Customer in Tabular	
	 */
	if (LCHECK ("ho_dbt_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		/*
		 *	cumr not required	
		 */
		if (dflt_used && prog_status != ENTRY)
			return (DeleteLine ());

		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no,branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (local_rec.childCustomerNo));

		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cumr_rec.hhcu_hash == cumr_ho.hhcu_hash)
		{
			print_mess (ML (mlDbMess071));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (cumr_rec.ho_dbt_hash != 0L && cumr_rec.ho_dbt_hash != cumr_ho.hhcu_hash)
		{
			print_mess (ML (mlStdMess095));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		for (i = 0;i < MAX_I (line_cnt,lcount [2]);i++)
		{
			if (i == line_cnt)
				continue;

			if (cumr_rec.hhcu_hash == store [i].hhcuHash)
			{
				print_mess (ML (mlStdMess096));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		if (envVarDbMcurr &&
		    strcmp (cumr_rec.curr_code, cumr_ho.curr_code))
		{
			print_mess (ML (mlDbMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (envVarDbMcurr &&
		    strcmp (cumr_rec.gl_ctrl_acct, cumr_ho.gl_ctrl_acct))
		{
			print_mess (ML (mlDbMess068));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.childCustomerNo,cumr_rec.dbt_no);
		strcpy (local_rec.childCustomerName,cumr_rec.dbt_name);
		sprintf (tmp_addr,"%s, %s, %s", clip (cumr_rec.ch_adr1),
								        clip (cumr_rec.ch_adr2),
								        clip (cumr_rec.ch_adr3));
		sprintf (local_rec.childAddress,"%-65.65s", tmp_addr);
	
		store [line_cnt].hhcuHash = cumr_rec.hhcu_hash;
		store [line_cnt].existing = FALSE;
		local_rec.childHhcuHash = cumr_rec.hhcu_hash;

		RemoveFromDeletedList (cumr_rec.hhcu_hash);

		DSP_FLD ("ho_dbt_name");
		DSP_FLD ("ho_ch_adr");
	}
	return (EXIT_SUCCESS);
}

void
LoadChildren (void)
{
	char	tmp_addr [131];

	/*
	 * Set screen 2 - for putval. 
	 */
	init_vars (2);
	lcount [2]	= 	0;

	print_at (2,0,ML (mlStdMess035));
	abc_selfield (cumr,"cumr_ho_dbt_hash");

	cumr_rec.ho_dbt_hash = cumr_ho.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && cumr_ho.hhcu_hash == cumr_rec.ho_dbt_hash)
	{
		strcpy (local_rec.childCustomerNo,cumr_rec.dbt_no);
		strcpy (local_rec.childCustomerName,cumr_rec.dbt_name);
		sprintf (tmp_addr,"%s, %s, %s", clip (cumr_rec.ch_adr1),
					       				clip (cumr_rec.ch_adr2),
					       				clip (cumr_rec.ch_adr3));

		sprintf (local_rec.childAddress,"%-65.65s", tmp_addr);

		local_rec.childHhcuHash	 	= cumr_rec.hhcu_hash;
		store [lcount [2]].hhcuHash = cumr_rec.hhcu_hash;
		store [lcount [2]].existing = TRUE;

		putval (lcount [2]++);

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");
	NewCode = (lcount [2] == 0);
}

int
DeleteLine (void)
{
	int		i;
	int		this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	
	AddToDeletedList (local_rec.childHhcuHash);

	lcount [2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt, line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		if (line_cnt >= i)
		{
			memcpy 
			(
				(char *) &store [line_cnt], 
				(char *) &store [line_cnt + 1],
				sizeof (struct storeRec)
			);
			getval (line_cnt + 1);
		}
		else
			getval (line_cnt);

		putval (line_cnt);

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	strcpy (local_rec.childCustomerNo,"      ");
	sprintf (local_rec.childCustomerName,"%-40.40s"," ");
	sprintf (local_rec.childAddress,"%-65.65s"," ");
	local_rec.childHhcuHash = 0L;
	putval (line_cnt);

	memset ((char *) &store [line_cnt], '\0', sizeof (struct storeRec));

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 *	Update cumr records	
 */
void
Update (void)
{
	int		i;
	clear ();
	print_at (2,0,ML (mlStdMess035));

	scn_set (2);

	for (i = 0; i < numDeleted; i++)
		UpdateCustomer (deletedList [i], 0L);
	
	for (i = 0;i < lcount [2];i++)
	{
		getval (i);

		if (local_rec.childHhcuHash != 0L)
			UpdateCustomer (local_rec.childHhcuHash,cumr_ho.hhcu_hash);
	}
}

void
UpdateCustomer (
	long	childHhcuHash,
	long	parentHhcuHash)
{
	abc_selfield (cumr, "cumr_hhcu_hash");

	cumr_rec.hhcu_hash	= childHhcuHash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "u");
	if (cc)
		return;

	cumr_rec.ho_dbt_hash = parentHhcuHash;
	cc = abc_update (cumr, &cumr_rec);
	if (cc)
		file_err (cc, cumr, "DBUPDATE");

	abc_selfield (cumr, (envDbFind) ? "cumr_id_no3" : "cumr_id_no");

	cuin_rec.hhcu_hash = childHhcuHash;
	for (cc = find_rec (cuin, &cuin_rec, GTEQ, "u");
		 !cc && cuin_rec.hhcu_hash == childHhcuHash;
		 cc = find_rec (cuin, &cuin_rec, NEXT, "u"))
	{
		cuin_rec.ho_hash = parentHhcuHash ? parentHhcuHash : childHhcuHash;
		cc = abc_update (cuin, &cuin_rec);
		if (cc)
			file_err (cc, cuin, "DBUPDATE");
	}
	abc_unlock (cuin);
}

void
tab_other (
 int                line_no)
{
	blank_at (5,0,130);
	crsr_off ();

	FLD ("ho_dbt_no") = YES;

	if (store [line_no].existing)
	{
		cumr2_rec.hhcu_hash = store [line_no].hhcuHash;
		cc = find_rec ("cumr2", &cumr2_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "cumr2", "DBFIND");

		if (!MayRemoveCustomer (store [line_no].hhcuHash))
		{
			sprintf (err_str, ML ("NOTE : Child %s (%s) has transactions on file so it cannot be deleted."), cumr2_rec.dbt_no, clip (cumr2_rec.dbt_name));
			rv_pr (err_str, (128 - (int) strlen (err_str)) / 2,5,1);
			FLD ("ho_dbt_no") = NA;
		}
	}
}

int
MayRemoveCustomer (
	long	hhcuHash)
{
	abc_selfield (cuhd, "cuhd_hhcp_hash");
	cuin_rec.hhcu_hash = hhcuHash;
	for (cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
		 !cc && cuin_rec.hhcu_hash == hhcuHash;
		 cc = find_rec (cuin, &cuin_rec, NEXT, "r"))
	{
		long prevHhcpHash = 0L;

		cudt_rec.hhci_hash = cuin_rec.hhci_hash;
		for (cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
			 !cc && cudt_rec.hhci_hash == cuin_rec.hhci_hash;
			 cc = find_rec (cudt, &cudt_rec, NEXT, "r"))
		{
			if (cudt_rec.hhcp_hash == prevHhcpHash)
				continue;

			cuhd_rec.hhcp_hash = cudt_rec.hhcp_hash;
			cc = find_rec (cuhd, &cuhd_rec, EQUAL, "r");
			if (!cc && cuhd_rec.hhcu_hash != hhcuHash)
				return FALSE;

			prevHhcpHash = cudt_rec.hhcp_hash;
		}
	}
    return (TRUE);
}

void
AddToDeletedList (
 long               hash)
{
	deletedList [numDeleted++] = hash;
}

void
RemoveFromDeletedList (
 long               hash)
{
	int i;

	for (i = 0; i < numDeleted; i++)
	{
		if (deletedList [i] == hash)
		{
			int j;

			numDeleted--;
			for (j = i; j < numDeleted; j++)
				deletedList [j] = deletedList [j + 1];
			return;
		}
	}
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
		swide ();
		rv_pr (ML (mlDbMess170),45,0,1);
		line_at (1,0,130);

		if (scn == 1)
		{
			box (0,3,130,10);
			line_at (6,1,129);
			line_at (10,1,129);
		}
		else
		{
			box (0,2,123,1);
			print_at (3,2, ML (mlDbMess169), cumr_ho.dbt_no, cumr_ho.dbt_name);
		}

		line_at (20,0,130);
		print_at (21,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else /* unlock database file (in case of restart in locked record)*/
		abc_unlock (cumr);

    return (EXIT_SUCCESS);
}
