/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_stmtprn.i.c,v 5.3 2002/08/14 04:27:07 scott Exp $
|  Program Name  : (db_stmtprn.i.c)
|  Program Desc  : (Input For Printing Selected Statements)
|---------------------------------------------------------------------|
|  Date Written  : (03/04/89)      | Author       : Roger Gibbison.   |
|---------------------------------------------------------------------|
| $Log: db_stmtprn.i.c,v $
| Revision 5.3  2002/08/14 04:27:07  scott
| Updated for Linux Warning
|
| Revision 5.2  2002/07/18 06:24:15  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.1  2001/12/07 03:26:34  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_stmtprn.i.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_stmtprn.i/db_stmtprn.i.c,v 5.3 2002/08/14 04:27:07 scott Exp $";

#define	MAXLINES	10
#define	TABLINES	10
#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_db_mess.h>
#include <arralloc.h>

#define	NUMERIC	1
#define	ACRONYM	2
#define	BY_ACR	(outputOrder == ACRONYM)

#ifdef GVISION
#include <RemoteFile.h>
#include <RemotePipe.h>
#define	popen	Remote_popen
#define	pclose	Remote_pclose
#define	fprintf	Remote_fprintf
#define	fflush	Remote_fflush
#endif	/* GVISION */

FILE	*pout;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*cumr2_balance	=	&cumr2_rec.bo_current;

	char	*cumr2	= "cumr2", 
			*cumr3	= "cumr3", 
			*data	= "data";

	int		envDBStmtCo 		= 0, 
			envDbCo 			= 0, 
			outputOrder 		= ACRONYM, 
			printerNo			= 1, 
			printZeroSortNum	= 0, 
			printPosSortNum		= 0, 
			printNegSortNum		= 0;

	char	branchNo [3], 
			runProgram [31], 
			tempStr [2];

	double	cumrBalance [6];

/*
 *	Structure for dynamic array,  for the stmtRec lines for qsort
 */
struct StmtStruct
{
	char	key [21];
	long	hhcuHash;
}	*stmtRec;
	DArray stmt_details;
	int	stmtCnt = 0;

int		StmtSort			(const	void *,	const void *);

#include	<FindCumr.h>

/*
 * Local & Screen Structures.
 */
struct
{
	char	dummy [11];
	char	customerNoStart [7];
	char	customerAcrStart [10];
	char	customerNoEnd [7];
	char	customerAcrEnd [10];
} local_rec;

static	struct	var	vars [] =
{
	{NUMERIC, TAB, "noCustomerNoStart", 	MAXLINES, 3, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNoStart}, 
	{NUMERIC, TAB, "n_acr_start", 	 0, 3, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", "Start Acronym  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerAcrStart}, 
	{NUMERIC, TAB, "n_dbt_end", 	 0, 3, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "~~~~~~", " End Customer  ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.customerNoEnd}, 
	{NUMERIC, TAB, "n_acr_end", 	 0, 3, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", "~~~~~~~~~", "  End Acronym  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerAcrEnd}, 

	{ACRONYM, TAB, "a_acr_start", 	 MAXLINES, 3, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", " Start Acronym ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.customerAcrStart}, 
	{ACRONYM, TAB, "a_dbt_start", 	0, 3, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerNoStart}, 
	{ACRONYM, TAB, "a_acr_end", 	 0, 3, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", "~~~~~~~~~", " End Acronym   ", " ", 
		YES, NO,  JUSTLEFT, "", "", local_rec.customerAcrEnd}, 
	{ACRONYM, TAB, "a_dbt_end", 	 0, 3, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "~~~~~~",    " End Customer  ", " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.customerNoEnd}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<wild_search.h>

/*
 * Local Function Prototypes.
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	heading 			(int);
int 	spec_valid 			(int);
void 	PrintStatements 	(void);
void 	PrintAllStatements 	(void);
void	PrintAll 			(int, long, char *);
char 	*CheckVariable 		(char *,  char	*);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv [])
{
	char	*sptr;
	int		selected = TRUE;

	SETUP_SCR (vars);

	sptr = chk_env ("DB_CO");
	if (sptr != (char *) 0)
		envDbCo = (* (sptr) == '0') ? TRUE : FALSE;

	sptr = chk_env ("DB_STMT_CO");
	if (sptr != (char *) 0)
		envDBStmtCo = (* (sptr) == '1') ? TRUE : FALSE;

	sptr = chk_env ("DB_STMT_SORT");
	if (sptr != (char *) 0)
	{
		sprintf (tempStr , "%-1.1s", sptr);
		printPosSortNum  = atoi (tempStr);
		sprintf (tempStr , "%-1.1s", sptr + 1);
		printZeroSortNum = atoi (tempStr);
		sprintf (tempStr , "%-1.1s", sptr + 2);
		printNegSortNum  = atoi (tempStr);
	}	
	else
	{
		printPosSortNum  = 1;
		printZeroSortNum = 1;
		printNegSortNum  = 1;
	}

	if (argc != 3 && argc != 4)
	{
		print_at (0, 0, mlDbMess117, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);

	switch (argv [2][0])
	{
	case	'A':
	case	'a':
		outputOrder = ACRONYM;
		break;

	case	'N':
	case	'n':
		outputOrder = NUMERIC;
		break;

	default:
		print_at (0, 0, mlDbMess117, argv [0]);
        return (EXIT_FAILURE);
	}

	if (argc == 4)
	{
		switch (argv [3][0])
		{
		case	'Y':
		case	'y':
			selected = TRUE;
			break;

		case	'N':
		case	'n':
			selected = FALSE;
			break;

		default:
			print_at (0, 0, mlDbMess117, argv [0]);
			print_at (0, 0, mlDbMess118);
            return (EXIT_FAILURE);
		}
	}
	OpenDB ();

	/*
	 * Read common terminal record.
	 */
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (branchNo, (envDbCo) ? " 0" : comm_rec.est_no);

	/*
	 * Open pipe to statement print program.
	 */
	strcpy (runProgram, CheckVariable ("DB_STMTPRN", "db_stmtprn"));
	if ((pout = popen (runProgram, "w")) == 0)
		file_err (errno, runProgram, "POPEN");

	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (outputOrder);

	if (!selected)
	{
		PrintAllStatements ();
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	while (!prog_exit)
	{
		entry_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1); 
		lcount [1] = 0;

		/*
		 * Enter screen 1 linear input.
		 */
		heading (outputOrder);
		entry (outputOrder);
		if (lcount [outputOrder] == 0)
        {
			shutdown_prog ();
            return (EXIT_FAILURE);
        }

		/*
		 * Edit screen 1 input.
		 */
		heading (outputOrder);
		scn_display (outputOrder);
		edit (outputOrder);
		if (!restart)
		{
			PrintStatements ();
			prog_exit = 1;
		}
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
	pclose (pout);
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumr2, cumr);
	abc_alias (cumr3, cumr);

	if (outputOrder == ACRONYM)
		open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDBStmtCo) ? "cumr_id_no4" : "cumr_id_no2");
	else
		open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (envDBStmtCo) ? "cumr_id_no3" : "cumr_id_no");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumr3,cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cumr3);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_dbclose (data);
}

int
heading (
	int		scn)
{
	int		s_size = 80;

	if (restart)
    	return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	rv_pr (ML (mlDbMess115), 27, 0, 1);

	line_at (1, 0, s_size);
	line_at (20, 0, s_size);
	line_at (22, 0, s_size);

	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21, 0, err_str);
	/* reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);

	return (EXIT_SUCCESS);
}

int
spec_valid (
 int                field)
{
	if (LCHECK ("noCustomerNoStart"))
	{
		if (dflt_used)
		{
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNo);
			strcpy (cumr_rec.dbt_no, "      ");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND-First");
			sprintf (local_rec.customerNoStart, "%-6.6s", cumr_rec.dbt_no);
			sprintf (local_rec.customerAcrStart, "%-9.9s", cumr_rec.dbt_acronym);
			DSP_FLD ("noCustomerNoStart");
			DSP_FLD ("n_acr_start");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.customerNoStart));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.customerAcrStart, "%-9.9s", cumr_rec.dbt_acronym);
		DSP_FLD ("noCustomerNoStart");
		DSP_FLD ("n_acr_start");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("n_dbt_end"))
	{
		if (dflt_used)
		{
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNo);
			strcpy (cumr_rec.dbt_no, "~~~~~~");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			if (cc)
				cc = find_rec (cumr, &cumr_rec, LAST, "r");
			else
				cc = find_rec (cumr, &cumr_rec, LT, "r");
			sprintf (local_rec.customerNoEnd, "%-6.6s", cumr_rec.dbt_no);
			sprintf (local_rec.customerAcrEnd, "%-9.9s", cumr_rec.dbt_acronym);
			DSP_FLD ("n_dbt_end");
			DSP_FLD ("n_acr_end");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.customerNoEnd));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.customerAcrEnd, "%-9.9s", cumr_rec.dbt_acronym);
		DSP_FLD ("n_dbt_end");
		DSP_FLD ("n_acr_end");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("a_acr_start"))
	{
		if (dflt_used)
		{
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNo);
			strcpy (cumr_rec.dbt_acronym, "         ");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			if (cc)
				file_err (cc, cumr, "DBFIND-First");
			sprintf (local_rec.customerNoStart, "%-6.6s", cumr_rec.dbt_no);
			sprintf (local_rec.customerAcrStart, "%-9.9s", cumr_rec.dbt_acronym);
			DSP_FLD ("a_dbt_start");
			DSP_FLD ("a_acr_start");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_acronym, local_rec.customerAcrStart);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.customerNoStart, "%-6.6s", cumr_rec.dbt_no);
		DSP_FLD ("a_dbt_start");
		DSP_FLD ("a_acr_start");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("a_acr_end"))
	{
		if (dflt_used)
		{
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNo);
			strcpy (cumr_rec.dbt_acronym, "~~~~~~~~~");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			if (cc)
				cc = find_rec (cumr, &cumr_rec, LAST, "r");
			else
				cc = find_rec (cumr, &cumr_rec, LT, "r");
			sprintf (local_rec.customerNoEnd, "%-6.6s", cumr_rec.dbt_no);
			sprintf (local_rec.customerAcrEnd, "%-9.9s", cumr_rec.dbt_acronym);
			DSP_FLD ("a_dbt_end");
			DSP_FLD ("a_acr_end");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNo, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		strcpy (cumr_rec.dbt_acronym, local_rec.customerAcrEnd);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.customerNoEnd, "%-6.6s", cumr_rec.dbt_no);
		DSP_FLD ("a_dbt_end");
		DSP_FLD ("a_acr_end");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
PrintStatements (void)
{
	int		initDone = FALSE,
			i;

	double	localTotal;

	clear ();

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&stmt_details, &stmtRec,sizeof (struct StmtStruct),10);
	stmtCnt = 0;

	for (line_cnt = 0; line_cnt < lcount [outputOrder]; line_cnt++)
	{
		getval (line_cnt);
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", local_rec.customerNoStart);
		sprintf (cumr_rec.dbt_acronym, "%-9.9s", local_rec.customerAcrStart);

		cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
		while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
		{
			if (!envDBStmtCo)
			{
				if (strcmp (cumr_rec.est_no, branchNo))
					break;
			}

			if (outputOrder == ACRONYM)
			{
				if (strcmp (cumr_rec.dbt_acronym, local_rec.customerAcrEnd) > 0)
					break;
			}

			if (outputOrder != ACRONYM)
			{
				if (strcmp (cumr_rec.dbt_no, local_rec.customerNoEnd) > 0)
					break;
			}
			dsp_process ("Customer : ", cumr_rec.dbt_acronym);

			/*
			 * Exclude child customer.
			 */
			if (cumr_rec.ho_dbt_hash > 0L)
			{
				cc = find_rec (cumr , &cumr_rec, NEXT, "r");
				continue;
			}
			for (i = 0; i < 6; i++)
				cumrBalance [i] = cumr_balance [i];

			cumr2_rec.ho_dbt_hash	=	cumr_rec.hhcu_hash;
			cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
			while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
			{
				for (i = 0; i < 6; i++)
					cumrBalance [i] += cumr2_balance [i];

				cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
			}
	 		localTotal =  cumrBalance [0] + cumrBalance [1] + cumrBalance [2] + 
                       	  cumrBalance [3] + cumrBalance [4] + cumrBalance [5];

			if (localTotal == 0.00 && printZeroSortNum != 0)
			{
				/*
				 * Check the array size before adding new element.
				 */
				if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
					sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

				sprintf 
				(
					stmtRec [stmtCnt].key, 
					"%1d%-9.9s%010ld", 
					 printZeroSortNum, 
	   				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
	   				cumr_rec.hhcu_hash
				);
				stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
				stmtCnt++;
			}

			if (localTotal < 0.00 && printNegSortNum != 0)
			{
				/*
				 * Check the array size before adding new element.
				 */
				if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
					sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

				sprintf 
				(
					stmtRec [stmtCnt].key, 
					"%1d%-9.9s%010ld", 
					printNegSortNum, 
	   				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
	   				cumr_rec.hhcu_hash
				);
				stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
				stmtCnt++;
			}

			if (localTotal > 0.00 && printPosSortNum != 0)
			{
				/*
				 * Check the array size before adding new element.
				 */
				if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
					sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

				sprintf 
				(
					stmtRec [stmtCnt].key, 
					"%1d%-9.9s%010ld", 
					printPosSortNum, 
	   				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
	   				cumr_rec.hhcu_hash
				);
				stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
				stmtCnt++;
			}
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		}
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (stmtRec, stmtCnt, sizeof (struct StmtStruct), StmtSort);

	for (i = 0; i < stmtCnt; i++)
	{
		if (!initDone)
		{
			dsp_screen ("Printing Statements",comm_rec.co_no, comm_rec.co_name);
			PrintAll (printerNo, -1L, "M");
		}

		PrintAll (printerNo, stmtRec [i].hhcuHash, "M");

		initDone = TRUE;

		sprintf (err_str, "%-9.9s", stmtRec [i].key);
		dsp_process ("Customer : ", err_str);

		PrintAll (printerNo, 0L, "M");
	}
	if (!envDbCo)
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (!cc)
		{
			esmr_rec.date_stmt_prn = comm_rec.dbt_date;
			abc_update (esmr, &esmr_rec);
		}
		else
			abc_unlock (esmr);
	}
	else
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (!cc)
		{
			comr_rec.date_stmt_prn = comm_rec.dbt_date;
			abc_update (comr, &comr_rec);
		}
		else
			abc_unlock (comr);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&stmt_details);
}

void
PrintAllStatements (void)
{
	int		initDone = FALSE;
	int		i;
	double	localTotal;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&stmt_details, &stmtRec,sizeof (struct StmtStruct),10);
	stmtCnt = 0;

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%-6.6s", " ");
	sprintf (cumr_rec.dbt_acronym, "%-9.9s", " ");
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*------------------------
		| Exclude child customer. |
		------------------------*/
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr , &cumr_rec, NEXT, "r");
			continue;
		}

		if (!envDBStmtCo)
			if (strcmp (cumr_rec.est_no, branchNo))
				break;

		for (i = 0; i < 6; i++)
			cumrBalance [i] = cumr_balance [i];

		cumr2_rec.ho_dbt_hash  = cumr_rec.hhcu_hash;
		cc = find_rec (cumr2, &cumr2_rec, GTEQ, "r");
		while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
		{
			for (i = 0; i < 6; i++)
				cumrBalance [i] += cumr2_balance [i];

			cc = find_rec (cumr2, &cumr2_rec, NEXT, "r");
		}

	 	localTotal =  	cumrBalance [0] + cumrBalance [1] + cumrBalance [2] + 
                   		cumrBalance [3] + cumrBalance [4] + cumrBalance [5];

		if (localTotal == 0.00 && printZeroSortNum != 0)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
				sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

			sprintf 
			(
				stmtRec [stmtCnt].key, 
				"%1d%-9.9s%010ld", 
				printZeroSortNum, 
				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
				cumr_rec.hhcu_hash
			);
			stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
			stmtCnt++;
		}

		if (localTotal < 0.00 && printNegSortNum != 0)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
				sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

			sprintf 
			(
				stmtRec [stmtCnt].key, 
				"%1d%-9.9s%010ld", 
				printNegSortNum, 
				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
				cumr_rec.hhcu_hash
			);
			stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
			stmtCnt++;
		}

		if (localTotal > 0.00 && printPosSortNum != 0)
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&stmt_details, stmtRec, stmtCnt))
				sys_err ("ArrChkLimit (stmtRec)", ENOMEM, PNAME);

			sprintf 
			(
				stmtRec [stmtCnt].key, 
				"%1d%-9.9s%010ld", 
				printPosSortNum, 
				(BY_ACR) ? cumr_rec.dbt_acronym : cumr_rec.dbt_no, 
				cumr_rec.hhcu_hash
			);
			stmtRec [stmtCnt].hhcuHash	=	cumr_rec.hhcu_hash;
			stmtCnt++;
		}
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	for (i = 0; i < stmtCnt; i++)
	{
		if (!initDone)
		{
			dsp_screen ("Printing Statements",comm_rec.co_no, comm_rec.co_name);
			PrintAll (printerNo, -1L, "M");
		}

		PrintAll (printerNo, stmtRec [i].hhcuHash, "M");

		initDone = TRUE;

	}
	if (!envDbCo)
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (!cc)
		{
			esmr_rec.date_stmt_prn = comm_rec.dbt_date;
			abc_update (esmr, &esmr_rec);
		}
		else
			abc_unlock (esmr);
	}
	else
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (!cc)
		{
			comr_rec.date_stmt_prn = comm_rec.dbt_date;
			abc_update (comr, &comr_rec);
		}
		else
			abc_unlock (comr);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&stmt_details);
}

/*===================================
| Search for cumr (acronym)  file.  |
===================================*/
void
AcronymSearch (
 char*              key_val)
{
	int		valid = 1;
	int		break_out;
	char	type_flag [2];
	char	*sptr = (*key_val == '*') ? (char *)0 : key_val;

	switch (search_key)
	{
	case	FN9:
		strcpy (type_flag, "N");
		break;

	case	FN10:
		strcpy (type_flag, "A");
		break;

	case	FN12:
		strcpy (type_flag, "D");
		break;

	default:
		sprintf (type_flag, "%-1.1s", "A");
		break;
	}

	if (type_flag [0] == 'D' || type_flag [0] == 'd')
		sptr = (char *)0;

	work_open ();

	if (type_flag [0] == 'A' || type_flag [0] == 'a')
		abc_selfield (cumr3, (envDBStmtCo) ? "cumr_id_no4" : "cumr_id_no2");
	else
		abc_selfield (cumr3, (envDBStmtCo) ? "cumr_id_no3" : "cumr_id_no");

	save_rec ("# Acronym ", "# Cust No.  Customer Name.");

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_acronym, "%-9.9s", (sptr != (char *)0) ? sptr : " ");
	sprintf (cumr_rec.dbt_no, "%-6.6s", (sptr != (char *)0) ? sptr : " ");
	
	cc = find_rec (cumr3, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		/*--------------------------------------------
		| If Customer Branch Owned && Correct Branch. |
		--------------------------------------------*/
		if (!envDBStmtCo && strcmp (cumr_rec.est_no, branchNo))
			break;

		switch (type_flag [0])
		{
		case	'A':
		case	'a':
			valid = check_search (cumr_rec.dbt_acronym, key_val, &break_out);
			break;

		case	'D':
		case	'd':
			valid = check_search (cumr_rec.dbt_name, key_val, &break_out);
			break_out = 0;
			break;

		default:
			valid = check_search (cumr_rec.dbt_no, key_val, &break_out);
			break;
		}

		if (valid)
		{
			sprintf (err_str, " (%s) %-40.40s", 
				cumr_rec.dbt_no, 
				cumr_rec.dbt_name);
			cc = save_rec (cumr_rec.dbt_acronym, err_str);
			if (cc)
				break;
		}
		else
		{
			if (break_out)
				break;
		}

		cc = find_rec (cumr3, &cumr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (cumr_rec.dbt_no, "%-6.6s", " ");
		sprintf (cumr_rec.dbt_acronym, "%-9.9s", " ");
		sprintf (cumr_rec.dbt_name, "%-40.40s", " ");
		return;
	}
	
	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%-6.6s", temp_str);
	sprintf (cumr_rec.dbt_acronym, "%-9.9s", temp_str);
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
}

/*
 * check the existence of the environment variable first with company & branch, 
 * then company, and then by itself. If no vble found then return 'prg'
 */
char *
CheckVariable (
	char	*envName, 
	char	*progName)
{
	char	*sptr;
	char	runPrint [41];

	/*
	 * Check Company & Branch
	 */
	sprintf (runPrint, "%s%s%s", envName, comm_rec.co_no, comm_rec.est_no);
	sptr = chk_env (runPrint);
	if (sptr == (char *)0)
	{
		/*
		 * Check Company
		 */
		sprintf (runPrint, "%s%s", envName, comm_rec.co_no);
		sptr = chk_env (runPrint);
		if (sptr == (char *)0)
		{
			strcpy (runPrint, envName);
			sptr = chk_env (runPrint);
			return ((sptr == (char *)0) ? progName : sptr);
		}
		else
			return (sptr);
	}
	else
		return (sptr);
}

void
PrintAll (
	int		printerNo, 
	long	hhcuHash, 
	char 	*mode)
{
	int	c	=	0;
	static int	printTheLot = 0;
	char	*sptr;

	if (!printTheLot)
	{
		sptr = chk_env ("LINE_UP");
		if (sptr != (char *)0)
		{
			c = atoi (sptr);
			printTheLot = !(c);
		}

		fprintf (pout, "%d\n", printerNo);
		fprintf (pout, "%s\n", mode);
		fflush (pout);

		if (printTheLot)
		{
			fprintf (pout, "0\n");
			fflush (pout);
			return;
		}
	}

	do
	{
		fprintf (pout, "%ld\n", hhcuHash);
		fflush (pout);
		if (!printTheLot)
		{
			sleep (sleepTime);
			clear ();
			strcpy (err_str, ML("Reprint Statement (for lineup) <Y/N> ? "));
			c = prmptmsg (err_str, "YyNn", 26, 1);
			if (mode [0] == 'M' && (c == 'N' || c == 'n'))
			{
				fprintf (pout, "0\n");
				fflush (pout);
			}
			clear ();
		}
	} while (!printTheLot && (c == 'Y' || c == 'y'));
	printTheLot = 1;
}

/*
 * Sort statement function.
 */
int 
StmtSort (
	const void *a1, 
	const void *b1)
{
	int	result;
	const struct StmtStruct a = * (const struct StmtStruct *) a1;
	const struct StmtStruct b = * (const struct StmtStruct *) b1;

	result = strcmp (a.key, b.key);

	return (result);
}

