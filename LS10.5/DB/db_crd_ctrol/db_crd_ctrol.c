/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crd_ctrol.c,v 5.4 2002/07/18 06:24:13 scott Exp $
|  Program Name  : (db_crd_ctrol.c) 
|  Program Desc  : (Customer Credit Control Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crd_ctrol.c,v $
| Revision 5.4  2002/07/18 06:24:13  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/25 03:58:07  scott
| Updated for abc_add error on Oracle.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crd_ctrol.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crd_ctrol/db_crd_ctrol.c,v 5.4 2002/07/18 06:24:13 scott Exp $";

#define MAXSCNS		2
#define MAXWIDTH 	140
#define MAXLINES 	500
#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>

	/*
	 * Special fields and flags
	 */
   	int  	newCode 		= FALSE,	
			envVarDbCo 		= 0,
			envVarDbFind 	= 0;

	char	branchNumber [3];

#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuccRecord	cucc_rec;
struct cumrRecord	cumr_rec;

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	char	dbtDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "customerNo",	 4, 25, CHARTYPE,
		"UUUUUU", "          ",
		" ", "", "Customer No.", " ",
		 NE, NO,  JUSTLEFT, "", "", cumr_rec.dbt_no},
	{1, LIN, "name",	 5, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Name  ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.dbt_name},
	{1, LIN, "cont",	 6, 25, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Normal Contact Person ", " ",
		 NA, NO,  JUSTLEFT, "", "", cumr_rec.contact_name},
	{2, TAB, "comment",	MAXLINES, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "                              C  O  M  M  E  N  T                               ", " ",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.comment},
	{2, TAB, "contact",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "  Contact Person.   ", " ",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.con_person},
	{2, TAB, "cont_date",	 0, 1, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.dbtDate, "Contact Date", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&cucc_rec.cont_date},
	{2, TAB, "hold_ref",	 0, 1, CHARTYPE,
		"AAAAAAAA", "          ",
		" ", " ", " Ref. No ", "Input Invoice/Credit or Journal Reference.",
		 NO, NO,  JUSTLEFT, "", "", cucc_rec.hold_ref},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

#include <FindCumr.h>	
/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	SrchCuin 		(char *);
int 	LoadCucc 		(void);
void 	Update 			(void);
int 	heading 		(int);

/*
 * Main Processing Routine. 
 */
int
main (
	int		argc,
	char	*argv [])
{
	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	OpenDB ();
	swide ();

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	/*
	 * Beginning of input control loop . 
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		lcount [1]	= 0;

		/*
		 * Enter screen 1 linear input. 
		 */
		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		/*
		 * Enter screen 2 Tabular input. 
		 */
		if (newCode == TRUE)
		{
			heading (2);
			entry (2);
		}

		if (restart)
			continue;

		/*
		 * Edit screen 1 & 2 & 3  input 
		 */
		edit_all ();
		if (restart)
			continue;

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
	strcpy (local_rec.dbtDate, DateToString (comm_rec.dbt_date));

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) ? "cumr_id_no"
		  					       : "cumr_id_no3");

	open_rec (cucc, cucc_list, CUCC_NO_FIELDS, "cucc_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");
}	

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cucc);
	abc_fclose (cuin);
	abc_dbclose ("data");
}


int
spec_valid (
 int                field)
{
	int	i, this_page;

	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("customerNo"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no,pad_num (cumr_rec.dbt_no));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			errmess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("name");
		DSP_FLD ("cont");

		rv_pr (ML (mlStdMess035),15,12,1);

		newCode = TRUE;

		/*----------------------------------
		| Read all Credit Control Records. |
		----------------------------------*/
		if (LoadCucc ())
		{
			newCode = FALSE;
			entry_exit = 1;
		}
		rv_pr ("                                               ",15,12,0);

		return (EXIT_SUCCESS);
	}
	/*------------------------------------------
	| Validate Customer Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("comment"))
	{
		if (dflt_used && prog_status != ENTRY)
		{
			lcount [2]--;
			this_page = line_cnt / TABLINES;
			for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
			{
				getval (line_cnt + 1);
				putval (line_cnt);
				if (this_page == line_cnt / TABLINES)
					line_display ();
			}
			sprintf (cucc_rec.comment,"%80.80s"," ");
			sprintf (cucc_rec.con_person,"%20.20s"," ");
			cucc_rec.cont_date = 0L;
			sprintf (cucc_rec.hold_ref,"%8.8s"," ");
			putval (line_cnt);
			if (this_page == line_cnt / TABLINES)
				line_display ();
#ifdef GVISION
			scn_display (2);
#endif
			line_cnt = i;
			getval (line_cnt);
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}
	/*----------------------------------
	| Validate Invoice Hold Referance. |
	----------------------------------*/
	if (LCHECK ("hold_ref"))
	{
		if (SRCH_KEY)
		{
			SrchCuin (temp_str);
			return (EXIT_SUCCESS);
		}
	    if (strcmp (cucc_rec.hold_ref, "        ") != 0)
	    {
			cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
			strcpy (cuin_rec.inv_no,cucc_rec.hold_ref);
			cc = find_rec (cuin, &cuin_rec, COMPARISON ,"r");
			if (cc)
			{
				strcpy (cuin_rec.inv_no,zero_pad (cucc_rec.hold_ref,8));
				cc = find_rec (cuin, &cuin_rec, COMPARISON ,"r");
			}
			if (cc)
			{
				sprintf (err_str, ML (mlDbMess125), cucc_rec.hold_ref);
				errmess (err_str);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
	    }
	    return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}	

/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchCuin (
	char	*keyValue)
{
	char	disp_amt [22];
	double	inv_balance;
	
	_work_open (10,0,20);
	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s", keyValue);
	save_rec ("#Invoice ","#     Amount    ");
	cc = find_rec (cuin , &cuin_rec, GTEQ, "r");
	while (!cc && !strncmp (cuin_rec.inv_no, keyValue,strlen (keyValue)) && 
			 (cuin_rec.hhcu_hash == cumr_rec.hhcu_hash))
	{
		inv_balance = cuin_rec.amt - cuin_rec.disc;
		sprintf (disp_amt, "%-14.2f ", DOLLARS (inv_balance));

		cc = save_rec (cuin_rec.inv_no, disp_amt);
		if (cc)
			break;
		cc = find_rec (cuin , &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	sprintf (cuin_rec.inv_no,"%-8.8s", temp_str);
	cc = find_rec (cuin , &cuin_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");
}

int
LoadCucc (void)
{
	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (2);

	init_vars (2);

	lcount [2] = 0;

	/*
	 * Prevents entry if not all lines loaded. 
	 */
	cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cucc_rec.record_no = 0;
	cc = find_rec (cucc, &cucc_rec, GTEQ, "r");
	while (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		/*
		 * Put Value Into Tabular Screen. 
		 */
		putval (lcount [2]++);

		cc = find_rec (cucc, &cucc_rec, NEXT, "r");
	}

	/*
	 * Return to screen 1. 
	 */
	scn_set (1);

	return (lcount [2]);
}

void
Update (void)
{

	clear ();
	rv_pr (ML (mlStdMess035) ,3,5,1);
	
	/*
	 * Set to Tabular Screen (s) to Update Discount Details. 
	 */
	scn_set (2);
	
	cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
	cucc_rec.record_no = 0;
	cc = find_rec (cucc, &cucc_rec, GTEQ, "u");
	while (!cc && cucc_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		cc = abc_delete (cucc);
	  	if (cc)
			file_err (cc, cucc, "DBDELETE");

		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = 0;
		cc = find_rec (cucc, &cucc_rec, GTEQ, "u");
	}
	abc_unlock (cucc);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++) 
	{
		getval (line_cnt);
		cucc_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cucc_rec.record_no = 0L;
		cc = abc_add (cucc, &cucc_rec);
		if (cc)
			file_err (cc, cucc, "DBADD");
	}
}

int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlDbMess126) ,46,0,1);

		line_at (1,0,130);

		if (scn == 1)
			box (0,3,130,3);

		line_at (20,0,130);
		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,	comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039), comm_rec.est_no,	comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else 
		abc_unlock (cumr);

    return (EXIT_SUCCESS);
}
