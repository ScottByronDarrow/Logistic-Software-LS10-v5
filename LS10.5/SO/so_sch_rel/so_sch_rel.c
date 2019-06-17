/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_sch_rel.c,v 5.3 2001/12/11 06:05:28 scott Exp $
|  Program Name  : (so_sch_rel.c  )                                 |
|  Program Desc  : (Scheduled Order Release Program.            )   |	
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (06/09/93)       |
|---------------------------------------------------------------------|
| $Log: so_sch_rel.c,v $
| Revision 5.3  2001/12/11 06:05:28  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_sch_rel.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_sch_rel/so_sch_rel.c,v 5.3 2001/12/11 06:05:28 scott Exp $";

#include <pslscr.h>	
#include <hot_keys.h>
#include <getnum.h>
#include <p_terms.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_so_mess.h>
#include <tabdisp.h>

#define	CO_OWNED	(envDbCo == 0)

extern int SR_Y_POS;

FILE	*fout;

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cumrRecord	cumr_rec;
struct cudiRecord	cudi_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct exafRecord	exaf_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct solnRecord	soln3_rec;
struct soktRecord	sokt_rec;

	char	*data   = "data", 
	    	*sohr2  = "sohr2", 
	    	*sohr3  = "sohr3", 
	    	*soln2  = "soln2", 
	    	*soln3  = "soln3";

	int		pipeOpen = FALSE;
	int		doMultiple;
	int		numOrders;
	int		numOrdLines;
	int		envQcApply = FALSE, 
			envSkQcAvl = FALSE;

	FILE	*fsort;

/*--------------------------------------------------------
| Definitions for sohr linked list.                      |
| This linked list is used to put sohr records in the    |
| correct order as an index on sohr for this purpose     |
| is not possible and a sort file would be far too slow. |
|                                                        |
|                  X                                     |
|                  ^                                     |
|                  |                                     |
|              prev|                                     |
| sohrHead----> +---------+                              |
|               |  SOHR   |                              |
|               |  DATA   |                              |
|               +---------+                              |
|                  ^   |next                             |
|                  |   |                                 |
|              prev|   v                                 |
|               +---------+                              |
|               |  SOHR   |                              |
|               |  DATA   |                              |
|               +---------+                              |
|                  ^   |next                             |
|                  |   |                                 |
|              prev|   v                                 |
| sohrTail----> +---------+                              |
|               |  SOHR   |                              |
|               |  DATA   |                              |
|               +---------+                              |
|                      |next                             |
|                      |                                 |
|                      v                                 |
|                      X                                 |
|                                                        |
--------------------------------------------------------*/
struct SOHR_LIST
{
	char	dbtNo [7];
	char	orderNo [9];
	char	dbtName [41];
	long	dtRaised;
	int		noLinesDue;
	long	hhsoHash;
	struct  SOHR_LIST *next;
	struct  SOHR_LIST *prev;
};
#define	SOHR_NULL	((struct SOHR_LIST *)NULL)
struct	SOHR_LIST	*sohrHead = SOHR_NULL;
struct	SOHR_LIST	*sohrTail = SOHR_NULL;

/*--------------------------------------------------------
| Definitions for line and item linked lists.            |
| This linked list is used to keep track of each item    |
| being released at WH level and also for the update     |
| phase when soln's are actually released to status C.   |
|                                                        |
|                  X                                     |
|                  ^                                     |
|                  |                                     |
|              prev|                                     |
| itemHead----> +------+    +------+    +------+         |
|               | ITEM |--->| LINE |--->| LINE |--->X    |
|               | DATA |    | DATA |<---| DATA |         |
|               +------+    +------+    +------+         |
|                 ^  |next                               |
|                 |  |                                   |
|             prev|  v                                   |
| itemTail----> +------+    +------+                     |
|               | ITEM |--->| LINE |--->X                |
|               | DATA |    | DATA |                     |
|               +------+    +------+                     |
|                    |next                               |
|                    |                                   |
|                    v                                   |
|                    X                                   |
|                                                        |
--------------------------------------------------------*/
struct LINE_LIST
{
	long	hhsoHash;
	long	hhslHash;
	float	qtyOrd;
	float	qtyBord;
	struct  LINE_LIST *next;
	struct  LINE_LIST *prev;
};
#define	LINE_NULL	((struct LINE_LIST *)NULL)

/*-----------------------------------
| Definitions for item linked list. |
-----------------------------------*/
struct ITEM_LIST
{
	long	hhbrHash;
	long	hhccHash;
	float	availStk;
	struct  LINE_LIST *lineHead;
	struct  LINE_LIST *lineTail;
	struct  ITEM_LIST *next;
	struct  ITEM_LIST *prev;
};
#define	ITEM_NULL	((struct ITEM_LIST *)NULL)
struct	ITEM_LIST	*itemHead;
struct	ITEM_LIST	*itemTail;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	order_no [9];
	char	systemDate [11];
	long	lsystemDate;
	long	factor;
	char	pterms [4];
	char	old_pterms [4];
	int		lpno;
} local_rec;


/*=======================
| Callback Declarations |
=======================*/
static	int	TagFunc (int c, KEY_TAB *psUnused);
static	int	SelectFunc (int c, KEY_TAB *psUnused);
static	int	ExitFunc (int c, KEY_TAB *psUnused);
/* ===== */
static 	int	TagLine (int c, KEY_TAB *psUnused);
static	int	ShowAddress (int c, KEY_TAB *psUnused);
static	int	ExitLines (int c, KEY_TAB *psUnused);

#ifdef	GVISION
static	KEY_TAB ordKeys [] = 
{
    { " TAG ", 	  'T', 		 TagFunc, 
	  "Tag/Untag Whole Order For Release.", 						"A" }, 
    { " SELECT ", 	  '\r', 	 SelectFunc, 
	  "Examine Order Lines For Release.", 						"A" }, 
    { "", 			  FN1, 		 ExitFunc, 
	  "Exit Without Releasing Orders.", 							"A" }, 
    { "", 			  FN16, 		 ExitFunc, 
	  "Release Orders and Exit.", 								"A" }, 
    END_KEYS
};

static	KEY_TAB linKeys [] = 
{
    { " TAG ", 	  'T', 		 TagLine, 
	  "Tag/Untag A Line For Release.", 							"A" }, 
    { " SHOW ADDRESS ", 	  'S', 		 ShowAddress, 
	  "Display delivery address.", 								"A" }, 
    { "", 			  FN1, 		 ExitLines, 
	  "", 														"A" }, 
    { "", 			  FN16, 	 ExitLines, 
	  "", 														"A" }, 
    END_KEYS
};
#else
static	KEY_TAB ordKeys [] = 
{
    { " [T]AG ", 	  'T', 		 TagFunc, 
	  "Tag/Untag Whole Order For Release.", 						"A" }, 
    { " [RETURN] ", 	  '\r', 	 SelectFunc, 
	  "Examine Order Lines For Release.", 						"A" }, 
    { "", 			  FN1, 		 ExitFunc, 
	  "Exit Without Releasing Orders.", 							"A" }, 
    { "", 			  FN16, 		 ExitFunc, 
	  "Release Orders and Exit.", 								"A" }, 
    END_KEYS
};

static	KEY_TAB linKeys [] = 
{
    { " [T]AG ", 	  'T', 		 TagLine, 
	  "Tag/Untag A Line For Release.", 							"A" }, 
    { " [S]HOW ADDRESS ", 	  'S', 		 ShowAddress, 
	  "Display delivery address.", 								"A" }, 
    { "", 			  FN1, 		 ExitLines, 
	  "", 														"A" }, 
    { "", 			  FN16, 	 ExitLines, 
	  "", 														"A" }, 
    END_KEYS
};
#endif

static	struct	var	vars [] =
{
	{1, LIN, "factor", 	 4, 25, INTTYPE, 
		"NNN", "          ", 
		" ", "0", " Contingency Factor :", " Enter number of contingency days for calculating due lines. ", 
		YES, NO,  JUSTLEFT, "0", "999", (char *)&local_rec.factor}, 
	{1, LIN, "order_no", 	 4, 80, CHARTYPE, 
		"UUUUUUUU", "          ", 
		"0", "", " Sales Order No :", " Enter sales order number to process. ", 
		YES, NO,  JUSTRIGHT, "", "", local_rec.order_no}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include <proc_sobg.h>
#include <RealCommit.h>

/*
 * Function Declarations
 */
float 	AddBackValues 		(long, long);
float 	AddLine 			(long, long, long, long, float, float);
float 	GetAvail 			(long, long);
float 	ProcPhantom 		(long, long);
int  	AddAllLines 		(long, int);
int  	AutoSetList 		(void);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	ClearRelList 		(void);
void 	CloseDB 			(void);
void 	CntSolnDue 			(long, int *, int *);
void 	InsItemNode 		(struct ITEM_LIST *);
void 	InsLineNode 		(struct ITEM_LIST *, struct LINE_LIST *);
void 	InsSohrNode 		(struct SOHR_LIST *);
void 	LogBO 				(float);
void 	OpenDB 				(void);
void 	OpenOutput 			(void);
void 	process 			(void);
void 	ProcSohr 			(long);
void 	ReadSohrs 			(void);
void 	RemoveAllLines 		(long);
void 	RemoveLine 			(long, long, long);
void 	shutdown_prog 		(void);
void 	SrchSohr 			(char *);
void 	UpdateOrders 		(void);
void 	UpdateSohr 			(long, char *);

struct 	ITEM_LIST *ItemInList 	(long, long);
struct 	LINE_LIST *LineInList 	(long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	int		someToRelease;
	char	*sptr;

	if (argc != 3 && argc != 4)
	{
		print_at (0, 0, mlSoMess782, argv [0]);
		print_at (0, 0, mlSoMess783, argv [0]);
		print_at (0, 0, mlSoMess784, argv [0]);
		return (EXIT_FAILURE);
	}

	if (argv [1][0] == 'M')
		doMultiple = TRUE;
	else
		doMultiple = FALSE;

	if (argc == 3)
	{
		if (argv [1][0] == 'A')
		{
			print_at (0, 0, mlSoMess782, argv [0]);
			print_at (1, 0, mlSoMess783, argv [0]);
			print_at (2, 0, mlSoMess784, argv [0]);
			return (EXIT_FAILURE);
		}
		local_rec.lpno = atoi (argv [2]);
	}
	else
	{
		if (argv [1][0] != 'A')
		{
			print_at (0, 0, mlSoMess782, argv [0]);
			print_at (1, 0, mlSoMess783, argv [0]);
			print_at (2, 0, mlSoMess784, argv [0]);
			return (EXIT_FAILURE);
		}
		else
		{
			local_rec.factor = atol (argv [2]);
			local_rec.lpno = atoi (argv [3]);
		}
	}

	/* QC module is active or not. */
	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	/* Whether to include QC qty in available stock. */
	envSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

	SETUP_SCR (vars);

	/*
	 * Setup required parameters
	 */
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();

	if (doMultiple)
		FLD ("order_no") = ND;

	set_tty ();
	init_scr ();
	set_masks ();

	if (argc == 4)
	{
		/*
		 * Automatic release.
		 */
		someToRelease = AutoSetList ();
		if (someToRelease)
			UpdateOrders ();

		if (pipeOpen)
		{	
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
	}
	else
	{
		/*
		 * Single/Multiple release.
		 */
		init_vars (1);
		while (prog_exit == 0)
		{
			/*
			 * Reset control flags
			 */
			entry_exit 	= FALSE;
			edit_exit 	= FALSE;
			prog_exit 	= FALSE;
			restart 	= FALSE;
			search_ok 	= TRUE;
			init_vars (1);		/*  set default values		*/

			/*
			 * Entry screen 1 linear input
			 */
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;

			process ();

			if (pipeOpen)
			{	
				fprintf (fout, ".EOF\n");
				pclose (fout);
			}
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
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files.
 */
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (sohr2, sohr);
	abc_alias (sohr3, sohr);
	abc_alias (soln2, soln);
	abc_alias (soln3, soln);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudi,  cudi_list, CUDI_NO_FIELDS, "cudi_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (sohr,  sohr_list, SOHR_NO_FIELDS, "sohr_id_no4");
	open_rec (sohr2, sohr_list, SOHR_NO_FIELDS, "sohr_id_no2");
	open_rec (sohr3, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (soln,  soln_list, SOLN_NO_FIELDS, "soln_id_no");
	open_rec (soln2, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (soln3, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (cumr);
	abc_fclose (cudi);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (exaf);
	abc_fclose (sohr);
	abc_fclose (sohr2);
	abc_fclose (sohr3);
	abc_fclose (soln);
	abc_fclose (soln2);
	abc_fclose (soln3);
	abc_fclose (soic);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		linesDue;
	int		linesValid;

	/*
	 * Validate Sales Order Number.
	 */
	if (LCHECK ("order_no"))
	{
		if (FLD ("order_no") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Check if order is on file.
		 */
		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		sprintf (sohr_rec.order_no, "%-8.8s", local_rec.order_no);
		cc = find_rec (sohr2, &sohr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess102));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		/*
		 * Check if order is a scheduled order.
		 */
		if (sohr_rec.sch_ord [0] != 'Y')
		{
			print_mess (ML (mlSoMess264));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Check if any lines are left to be releases.
		 */
		CntSolnDue (sohr_rec.hhso_hash, &linesDue, &linesValid);
		if (linesValid == 0)
		{
			print_mess (ML (mlStdMess092));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Search for order number. 
 */
void
SrchSohr (
	char	*keyValue)
{
	int		linesDue;
	int		linesValid;

	_work_open (8, 0, 40);
	save_rec ("#Order No", "#Customer Order Ref.");

	strcpy (sohr_rec.co_no,     comm_rec.co_no);
	strcpy (sohr_rec.br_no,     comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", keyValue);
	cc = find_rec (sohr2, &sohr_rec, GTEQ, "r");
	while (!cc && 
		   !strcmp (sohr_rec.co_no, comm_rec.co_no) && 
		   !strcmp (sohr_rec.br_no, comm_rec.est_no) &&
		   !strncmp (sohr_rec.order_no, keyValue, strlen (keyValue)))
	{
		if (sohr_rec.sch_ord [0] == 'Y')
		{
			CntSolnDue (sohr_rec.hhso_hash, &linesDue, &linesValid);
			if (linesValid != 0)
			{
				cc = save_rec (sohr_rec.order_no, sohr_rec.cus_ord_ref);
				if (cc)
					break;
			}
		}
		cc = find_rec (sohr2, &sohr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no,     comm_rec.co_no);
	strcpy (sohr_rec.br_no,     comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr2, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "sohr", "DBFIND");
}

void
process (void)
{
	struct	SOHR_LIST	*tmpPtr, *delPtr;

	if (doMultiple)
	{
		tab_open ("ordList", ordKeys, 5, 8, 10, FALSE);
		tab_add ("ordList", 
			"#    %-6.6s   |   %-40.40s   |   %-8.8s   |   %-11.11s   |   %-15.15s   ", 
			" CUST.", 
			"       CUSTOMER NAME", 
			"ORDER NO", 
			"DATE RAISED", 
			"NO OF LINES DUE");

		ReadSohrs ();
		if (sohrHead == SOHR_NULL)
		{
			tab_add ("ordList", "  ******  THERE ARE NO SCHEDULED ORDERS TO RELEASE ******");
			tab_display ("ordList", TRUE);
			putchar (BELL);
			fflush (stdout);
			sleep (sleepTime);
			tab_close ("ordList", TRUE);
			return;
		}
		else
		{
			tmpPtr = sohrHead;
			while (tmpPtr != SOHR_NULL)
			{
				tab_add ("ordList", 
					"    %-6.6s   |  %-40.40s    |   %-8.8s   |    %-10.10s   |     %5d            %010ld", 
					tmpPtr->dbtNo, 
					tmpPtr->dbtName, 
					tmpPtr->orderNo, 
					DateToString (tmpPtr->dtRaised), 
					tmpPtr->noLinesDue, 
					tmpPtr->hhsoHash);

				tmpPtr = tmpPtr->next;
			}
		}

		tab_scan ("ordList");
	}
	else
		ProcSohr (sohr_rec.hhso_hash);

	/*
	 * Free list of sohrs.
	 */
	tmpPtr = sohrHead;
	while (tmpPtr != SOHR_NULL)
	{
		delPtr = tmpPtr;
		tmpPtr = tmpPtr->next;

		free (delPtr);
	}
	sohrHead = SOHR_NULL;
	sohrTail = SOHR_NULL;

	if (!restart)
		UpdateOrders ();

	ClearRelList ();

	if (doMultiple)
		tab_close ("ordList", TRUE);
}

/*
 * Read sohrs and do an insertion sort so that
 * records are in debtor/order no.  order.   
 */
void
ReadSohrs (void)
{
	int	linesDue;
	int	linesValid;
	struct	SOHR_LIST	*newSohrPtr;

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	strcpy (sohr_rec.sch_ord, "Y");
	sohr_rec.hhcu_hash = 0L;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
		   		  !strcmp (sohr_rec.br_no, comm_rec.est_no) &&
		   		  sohr_rec.sch_ord [0] == 'Y')
	{
		/*
		 * Check that order has valid lines
		 */
		CntSolnDue (sohr_rec.hhso_hash, &linesDue, &linesValid);
		if (linesValid == 0)
		{
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Get memory for new node in list.
		 */
		newSohrPtr = (struct SOHR_LIST *) malloc (sizeof (struct SOHR_LIST));
		if (newSohrPtr == SOHR_NULL)
			sys_err ("Error in ReadSohrs during MALLOC", errno, PNAME);

		/*
		 * Look up customer.
		 */
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sohr, &sohr_rec, NEXT, "r");
			continue;
		}

		/*
		 * Set data in node.
		 */
		sprintf (newSohrPtr->dbtNo,   "%-6.6s",   cumr_rec.dbt_no);
		sprintf (newSohrPtr->orderNo, "%-8.8s",   sohr_rec.order_no);
		sprintf (newSohrPtr->dbtName, "%-40.40s", cumr_rec.dbt_name);
		newSohrPtr->dtRaised   = sohr_rec.dt_raised;
		newSohrPtr->hhsoHash   = sohr_rec.hhso_hash;
		newSohrPtr->noLinesDue = linesDue;
		newSohrPtr->next       = SOHR_NULL;
		newSohrPtr->prev       = SOHR_NULL;

		/*
		 * Insert Node in list.
		 */
		InsSohrNode (newSohrPtr);

		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
}

/*
 * Count all status G solns that belong to current sohr and are due.
 */
void
CntSolnDue (
	long	hhsoHash, 
	int 	*linesDue, 
	int 	*linesValid)
{
	*linesDue 			= 0;
	*linesValid 		= 0;
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.status [0] == 'G')
		{
			*linesValid += 1;
			if (soln_rec.due_date <= (local_rec.lsystemDate + local_rec.factor))
			{
				*linesDue += 1;
			}
		}
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

static int 
TagFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	int		i;
	int		curLine;
	int		tmpLinesDue;
	int		tmpLinesValid;
	int		lineTagged;
	long	hhsoHash;
	char	getBuf [300];
	char	updBuf [300];
	
	curLine = tab_tline ("ordList");
	tab_get ("ordList", getBuf, EQUAL, curLine);
	hhsoHash = atol (getBuf + 114);

	if (getBuf [2] == '*')
	{
		/*
		 * Untag in tabdisp window.
		 */
		sprintf (updBuf, "    %-127.127s", getBuf + 4);
		tab_update ("ordList", updBuf);

		/*
		 * Untag all lines for this order in release list.
		 */
		RemoveAllLines (hhsoHash);
	}
	else
	{
		/*
		 * Check is lines have been tagged individually.
		 */
		if (getBuf [2] == '+')
		{
			i = prmptmsg (ML (mlSoMess248), "YyNn", 2, 2);
			move (0, 2);
			cl_line ();
			if (i == 'Y' || i == 'y')
			{
				CntSolnDue (hhsoHash, &tmpLinesDue, &tmpLinesValid);
				if (tmpLinesDue < tmpLinesValid)
				{
					/*
					 * Some lines not yet due.
					 */
					i = prmptmsg (ML (mlSoMess249), "YyNn", 2, 2);
					move (0, 2);
					cl_line ();
					if (i == 'Y' || i == 'y')
					{
						/*
						 * Add all status G solns into list.
						 */
						lineTagged = AddAllLines (hhsoHash, TRUE);
					}
					else
					{
						/*
						 * Add all status G solns except those
						 * not yet due into list.            
						 */
						RemoveAllLines (hhsoHash);
						lineTagged = AddAllLines (hhsoHash, FALSE);
					}
					if (lineTagged)
						sprintf (updBuf, "  * %-127.127s", getBuf + 4);
					else
						sprintf (updBuf, "    %-127.127s", getBuf + 4);
					tab_update ("ordList", updBuf);
				}
				else
				{
					/*
					 * All lines due. Add all status G solns into list.
					 */
					RemoveAllLines (hhsoHash);
					lineTagged = AddAllLines (hhsoHash, FALSE);
					if (lineTagged)
						sprintf (updBuf, "  * %-127.127s", getBuf + 4);
					else
						sprintf (updBuf, "    %-127.127s", getBuf + 4);
					tab_update ("ordList", updBuf);
				}
			}
		}
		else
		{
			/*
			 * Check is any lines are not yet due.
			 */
			CntSolnDue (hhsoHash, &tmpLinesDue, &tmpLinesValid);
			if (tmpLinesDue < tmpLinesValid)
			{
				/*
				 * Some lines not yet due.
				 */
				i = prmptmsg (ML (mlSoMess249), "YyNn", 2, 2);
				move (0, 2);
				cl_line ();
				if (i == 'Y' || i == 'y')
				{
					/*
					 * Add all status G solns into list.
					 */
					lineTagged = AddAllLines (hhsoHash, TRUE);
				}
				else
				{
					/*
					 * Add all status G solns except those
					 * not yet due into list.            
					 */
					RemoveAllLines (hhsoHash);
					lineTagged = AddAllLines (hhsoHash, FALSE);
				}
				if (lineTagged)
					sprintf (updBuf, "  * %-127.127s", getBuf + 4);
				else
					sprintf (updBuf, "    %-127.127s", getBuf + 4);
				tab_update ("ordList", updBuf);
			}
			else
			{
				/*
				 * Add all status G solns into list.
				 */
				RemoveAllLines (hhsoHash);
				lineTagged = AddAllLines (hhsoHash, FALSE);
				if (lineTagged)
					sprintf (updBuf, "  * %-127.127s", getBuf + 4);
				else
					sprintf (updBuf, "    %-127.127s", getBuf + 4);
				tab_update ("ordList", updBuf);
			}
		}
	}
	tab_get ("ordList", getBuf, EQUAL, curLine);
    return (EXIT_SUCCESS);
}

int
AddAllLines (
	long	hhsoHash, 
	int		includeAllLines)
{
	int		backOrd;
	int		lineNotDue;
	int		lineAdded = FALSE;

	backOrd = FALSE;
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.status [0] == 'G')
		{
			/*
			 * Only add lines that are not yet due if includeAllLines is TRUE.
			 */
			lineNotDue = (soln_rec.due_date > (local_rec.lsystemDate + local_rec.factor));
			if (!includeAllLines && lineNotDue)
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}

			backOrd = 	AddLine
						(
							soln_rec.hhbr_hash, 
							soln_rec.hhcc_hash, 
							soln_rec.hhsl_hash, 
							soln_rec.hhso_hash, 
							soln_rec.qty_order, 
							soln_rec.qty_bord
						);
			lineAdded = TRUE;
		}

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}

	if (backOrd)
	{
		print_mess (ML (mlSoMess247));
		sleep (sleepTime);
		clear_mess ();
	}

	return (lineAdded);
}

void
RemoveAllLines (
	long	hhsoHash)
{
	soln_rec.hhso_hash = hhsoHash;
	soln_rec.line_no = 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		RemoveLine
		(
			soln_rec.hhbr_hash, 
			soln_rec.hhcc_hash, 
			soln_rec.hhsl_hash
		);
		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}
}

static int 
SelectFunc (
	int 		c, 
	KEY_TAB 	*psUnused)
{
	char	getBuf [300];
	long	hhsoHash;

	tab_get ("ordList", getBuf, CURRENT, 0);
	hhsoHash = atol (getBuf + 114);

	ProcSohr (hhsoHash);
    return (EXIT_SUCCESS);
}

static int 
ExitFunc (
	int 	c, 
	KEY_TAB *psUnused)
{
	if (c == FN1)
		restart = TRUE;
	else
		restart = FALSE;

	return (FN16);
}

int
AutoSetList (
 void)
{
	int		lineAdded;
	int		backOrd;

				
	dsp_screen ("Processing Scheduled Orders", comm_rec.co_no, comm_rec.co_name);
	
	lineAdded = FALSE;
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	strcpy (sohr_rec.sch_ord, "Y");
	sohr_rec.hhcu_hash = 0L;
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
		   !strcmp (sohr_rec.br_no, comm_rec.est_no) &&
		   sohr_rec.sch_ord [0] == 'Y')
	{
		soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
		soln_rec.line_no 	= 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			/*
			 * Check line status.
			 */
			if (soln_rec.status [0] != 'G')
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
		
			/*
			 * Check if line is due.
			 */
			if (soln_rec.due_date > (local_rec.lsystemDate + local_rec.factor))
			{
				cc = find_rec (soln, &soln_rec, NEXT, "r");
				continue;
			}
	
			dsp_process ("Order No : ", sohr_rec.order_no);

			/*
			 * Add line to List.
			 */
			backOrd = 	AddLine
						(
							soln_rec.hhbr_hash, 
							soln_rec.hhcc_hash, 
							soln_rec.hhsl_hash, 
							soln_rec.hhso_hash, 
							soln_rec.qty_order, 
							soln_rec.qty_bord
						);
			lineAdded = TRUE;

			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	return (lineAdded);
}

/*
 * Process individual lines for the sohr chosen.
 */
void
ProcSohr (
	long	hhsoHash)
{
	char	dueLine [4];
	char	inList [2];

	/*
	 * Look up header record.
	 */
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr3, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sohr3, "DBFIND");

	/*
	 * Look up customer.
	 */
	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");
		
	/*
	 * Set up screen and display order header details.
	 */
	cl_box (8, 5, 115, 2);
	print_at (6, 10, ML (mlSoMess251), sohr_rec.order_no);
	print_at (7, 10, ML (mlSoMess252), DateToString (sohr_rec.dt_raised));
	print_at (6, 60, ML (mlSoMess253), cumr_rec.dbt_no);
	print_at (7, 60, ML (mlSoMess254), cumr_rec.dbt_name);

	/*
	 * Open table for order lines.
	 */
	tab_open ("ordLnes", linKeys, 9, 8, 6, FALSE);
	tab_add ("ordLnes", 
		"#    %-2.2s | %-2.2s | %-16.16s|%-40.40s| %-7.7s| %-7.7s|%-10.10s|%-10.10s  ", 
		"BR", 
		"WH", 
		"  ITEM NUMBER", 
		"            ITEM DESCRIPTION", 
		"QTY ORD", 
		"QTY B/O", 
		" DATE DUE ", 
		" LINE DUE ");

	numOrdLines = 0;

	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		/*
		 * Look up item number.
		 */
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		/*
		 * Look up warehouse.
		 */
		ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}

		/*
		 * Check line status.
		 */
		if (soln_rec.status [0] != 'G')
		{
			cc = find_rec (soln, &soln_rec, NEXT, "r");
			continue;
		}
	
		/*
		 * Check if line is due. 
		 */
		strcpy (dueLine, "No ");
		if (soln_rec.due_date <= (local_rec.lsystemDate + local_rec.factor))
			strcpy (dueLine, "Yes");

		/*
		 * Check if line is already in list.
		 */
		strcpy (inList, " ");
		if (LineInList (soln_rec.hhsl_hash) != LINE_NULL)
			strcpy (inList, "*");

		tab_add 
		(
			"ordLnes", 
			"  %-1.1s %2.2s | %-2.2s | %-16.16s|%-40.40s| %7.2f| %7.2f|%-10.10s|   %-3.3s              %010ld", 
			inList, 
			ccmr_rec.est_no, 
			ccmr_rec.cc_no, 
			inmr_rec.item_no, 
			inmr_rec.description, 
			soln_rec.qty_order, 
			soln_rec.qty_bord, 
			DateToString (soln_rec.due_date), 
			dueLine, 
			soln_rec.hhsl_hash
		);
		numOrdLines++;

		cc = find_rec (soln, &soln_rec, NEXT, "r");
	}

	if (numOrdLines > 0)
		tab_scan ("ordLnes");
	else
	{
			tab_add ("ordLnes", "  ******  THERE ARE NO LINES TO RELEASE FROM THIS ORDER ******");
			tab_display ("ordLnes", TRUE);
			putchar (BELL);
			fflush (stdout);
			sleep (sleepTime);
			tab_close ("ordLnes", TRUE);
			return;
	}
}

static int 
TagLine (
	int 	c, 
	KEY_TAB *psUnused)
{
	int		curLine;
	int		backOrd;
	long	hhslHash;
	char	getBuf [300];
	char	updBuf [300];
	
	curLine = tab_tline ("ordLnes");
	tab_get ("ordLnes", getBuf, EQUAL, curLine);
	hhslHash = atol (getBuf + 121);

	soln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (soln2, &soln_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, soln, "DBFIND");

	if (getBuf [2] == '*')
	{
		/*
		 * Untag in tabdisp window.
		 */
		sprintf (updBuf, "    %-127.127s", getBuf + 4);
		tab_update ("ordLnes", updBuf);

		/*
		 * Remove from release list.
		 */
		RemoveLine
		(
			soln_rec.hhbr_hash, 
			soln_rec.hhcc_hash, 
			soln_rec.hhsl_hash
		);
	}
	else
	{
		/*
		 * Tag in tabdisp window.
		 */
		sprintf (updBuf, "  * %-127.127s", getBuf + 4);
		tab_update ("ordLnes", updBuf);

		/*
		 * Insert in release list.
		 */
		backOrd = 	AddLine
					(
						soln_rec.hhbr_hash, 
						soln_rec.hhcc_hash, 
						soln_rec.hhsl_hash, 
						soln_rec.hhso_hash, 
						soln_rec.qty_order, 
						soln_rec.qty_bord
					);
		if (backOrd)
		{
			print_mess (ML (mlSoMess247));
			sleep (sleepTime);
			clear_mess ();
		}
	}
	tab_get ("ordLnes", getBuf, EQUAL, curLine);
    return (EXIT_SUCCESS);
}

static int 
ShowAddress (
 int c, 
 KEY_TAB *psUnused)
{
	int		curLine;
	long	hhslHash;
	char	delName [41];
	char	delAddr [3][41];
	char	getBuf [300];

	curLine = tab_tline ("ordLnes");
	tab_get ("ordLnes", getBuf, EQUAL, curLine);
	hhslHash = atol (getBuf + 121);
	
	soln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (soln2, &soln_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, soln, "DBFIND");

	/*
	 * Look up delivery address.
	 */
	if (soln_rec.del_no == 0)
	{
		sprintf (delName,    "%-40.40s", sohr_rec.del_name);
		sprintf (delAddr [0], "%-40.40s", sohr_rec.del_add1);
		sprintf (delAddr [1], "%-40.40s", sohr_rec.del_add2);
		sprintf (delAddr [2], "%-40.40s", sohr_rec.del_add3);
	}
	else
	{
		cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cudi_rec.del_no = soln_rec.del_no;
		cc = find_rec (cudi, &cudi_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (delName,    "%-40.40s", sohr_rec.del_name);
			sprintf (delAddr [0], "%-40.40s", sohr_rec.del_add1);
			sprintf (delAddr [1], "%-40.40s", sohr_rec.del_add2);
			sprintf (delAddr [2], "%-40.40s", sohr_rec.del_add3);
		}
		else
		{
			sprintf (delName,    "%-40.40s", cudi_rec.name);
			sprintf (delAddr [0], "%-40.40s", cudi_rec.adr1);
			sprintf (delAddr [1], "%-40.40s", cudi_rec.adr2);
			sprintf (delAddr [2], "%-40.40s", cudi_rec.adr3);
		}
	}

	/*
	 * Display delivery address.
	 */
		
	cl_box (30, 13, 65, 4);
	print_at (14, 32, ML (mlSoMess255), delName);
	print_at (15, 32, ML (mlSoMess256), delAddr [0]);
	print_at (16, 32, "                 : %s", delAddr [1]);
	print_at (17, 32, "                 : %s", delAddr [2]);
	PauseForKey (18, 50, ML (mlStdMess042), 0);
		
	tab_display ("ordLnes", TRUE);
	redraw_keys ("ordLnes");

	return (c);
}

static int 
ExitLines (
	int 	c, 
	KEY_TAB *psUnused)
{
	int		i;
	char	getBuf [300];
	char	updBuf [300];

	if (doMultiple)
	{
		/*
		 * If any lines are tagged then we need to   
		 * tag the header line in "ordList" with a +. 
		 */
		for (i = 0; i < numOrdLines; i++)
		{
			tab_get ("ordLnes", getBuf, EQUAL, i);
			if (getBuf [2] == '*')
			{
				tab_get ("ordList", getBuf, CURRENT, 0);
				sprintf (updBuf, "  + %-127.127s", getBuf + 4);
				tab_update ("ordList", updBuf);
				break;
			}
		}
		if (i == numOrdLines)
		{
			tab_get ("ordList", getBuf, CURRENT, 0);
			sprintf (updBuf, "    %-127.127s", getBuf + 4);
			tab_update ("ordList", updBuf);
		}
	}
	
	tab_close ("ordLnes", TRUE);

	if (doMultiple)
	{
		tab_display ("ordList", TRUE);
		redraw_keys ("ordList");
	}

	return (FN16);
}

/*
 * Checks to see if item (at WH level) is already in list. 
 * If so then a pointer to the node is returned.  If not in
 * the list then ITEM_NULL is returned.                    
 */
struct ITEM_LIST *
ItemInList (
	long	hhbrHash, 
	long	hhccHash)
{
	struct ITEM_LIST *tmpPtr;

	tmpPtr = itemHead;
	while (tmpPtr != ITEM_NULL)
	{
		if (tmpPtr->hhbrHash == hhbrHash && tmpPtr->hhccHash == hhccHash)
			return (tmpPtr);

		tmpPtr = tmpPtr->next;
	}

	return (ITEM_NULL);
}

/*
 * Checks to see if an order line is already in list.
 * If so then a pointer to the node is returned.  If 
 * not in the list then LINE_NULL is returned.       
 */
struct LINE_LIST *
LineInList (
	long	hhslHash)
{
	struct ITEM_LIST *tmpItemPtr;
	struct LINE_LIST *tmpLinePtr;

	tmpItemPtr = itemHead;
	while (tmpItemPtr != ITEM_NULL)
	{
		tmpLinePtr = tmpItemPtr->lineHead;
		while (tmpLinePtr != LINE_NULL)
		{
			if (tmpLinePtr->hhslHash == hhslHash)
				return (tmpLinePtr);

			tmpLinePtr = tmpLinePtr->next;
		}

		tmpItemPtr = tmpItemPtr->next;
	}

	return (LINE_NULL);
}
/*
 * Get available stock for item / warehouse.
 */
float 
GetAvail (
	long	hhbrHash, 
	long	hhccHash)
{
	float	availStk;
	float	realCommitted;

	/*
	 * Find inmr record.
	 */
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	availStk = 0.00;

	if (inmr_rec.inmr_class [0] == 'P')	/* PHANTOM */
		availStk = ProcPhantom (hhbrHash, hhccHash);
	else
	{
		incc_rec.hhbr_hash = hhbrHash;
		incc_rec.hhcc_hash = hhccHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		/*
		 * Calculate Actual Qty Committed.
		 */
		realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
											incc_rec.hhcc_hash);

		availStk = 	incc_rec.closing_stock - 
					incc_rec.backorder - 
					incc_rec.committed - 
					realCommitted;

		if (envQcApply && envSkQcAvl)
			availStk -= incc_rec.qc_qty;

		availStk += AddBackValues (incc_rec.hhbr_hash, incc_rec.hhcc_hash);
	}
	return (availStk);
}

float 
ProcPhantom (
	long	hhbrHash, 
	long	hhccHash)
{
	float	min_qty = 0.00, 
			on_hand = 0.00;

	float	realCommitted;

	int	first_time = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash = hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = hhccHash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
	
		/*
		 * Calculate Actual Qty Committed.
		 */
		realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
											incc_rec.hhcc_hash);

		on_hand = 	incc_rec.closing_stock - 
					incc_rec.committed - 
					incc_rec.backorder -
				  	realCommitted;

		if (envQcApply && envSkQcAvl)
			on_hand -= incc_rec.qc_qty;

		on_hand += AddBackValues (incc_rec.hhbr_hash, incc_rec.hhcc_hash);
		
		on_hand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

/*
 * Update tagged lines.
 */
void
UpdateOrders (void)
{
	float	availStk;
	float	qtyOrd;
	struct	ITEM_LIST	*tmpItemPtr;
	struct	LINE_LIST	*tmpLinePtr;

	tmpItemPtr = itemHead;
	while (tmpItemPtr != ITEM_NULL)
	{
		/*
		 * Recalculate available stock for item / warehouse.
		 */
		availStk = GetAvail (tmpItemPtr->hhbrHash, tmpItemPtr->hhccHash);

		/*
		 * Release stock for each line in the list.
		 */
		tmpLinePtr = tmpItemPtr->lineHead;
		while (tmpLinePtr != LINE_NULL)
		{
			soln_rec.hhsl_hash	=	tmpLinePtr->hhslHash;
			cc = find_rec (soln2, &soln_rec, EQUAL, "u");
			if (cc)
			{
				tmpLinePtr = tmpLinePtr->next;
				continue;
			}

			qtyOrd = soln_rec.qty_order + soln_rec.qty_bord;
			if (qtyOrd <= availStk)	
			{
				/*
				 * Can supply the whole line.
				 */
				availStk -= qtyOrd;
				soln_rec.qty_order += soln_rec.qty_bord;
				soln_rec.qty_bord = 0.00;
				strcpy (soln_rec.status, "C");
				strcpy (soln_rec.stat_flag, "M");
			}
			else
			{
				if (availStk <= 0.00)
				{
					/*
					 * Must backorder the whole line.
					 */
					soln_rec.qty_order += soln_rec.qty_bord;
					soln_rec.qty_bord = 0.00;
					if (cumr_rec.bo_flag [0] == 'N')
					{
						LogBO (soln_rec.qty_order);
						soln_rec.qty_order = 0.00;
					}
					else
					{
						if (inmr_rec.bo_flag [0] == 'N')
						{
							LogBO (soln_rec.qty_order);
							soln_rec.qty_order = 0.00;
						}
					}
					strcpy (soln_rec.status,    "B");
					strcpy (soln_rec.stat_flag, "B");
				}
				else
				{
					/*
					 * Partially backorder the line.
					 */
					availStk -= qtyOrd;
					soln_rec.qty_order += soln_rec.qty_bord;
					soln_rec.qty_bord   = soln_rec.qty_order - availStk;
					soln_rec.qty_order  = availStk;
					strcpy (soln_rec.status, "C");
					strcpy (soln_rec.stat_flag, "M");

					if (cumr_rec.bo_flag [0] == 'N')
					{
						LogBO (soln_rec.qty_bord);
						soln_rec.qty_bord = 0.00;
					}
					else
					{
						if (inmr_rec.bo_flag [0] == 'N')
						{
							/*
							 * Item does not allow BO.
							 */
							LogBO (soln_rec.qty_bord);
							soln_rec.qty_bord = 0.00;
						}
						else if (inmr_rec.bo_flag [0] == 'F')
						{
							/*
							 * Item allows full BO only.
							 */
							availStk += soln_rec.qty_order;
							soln_rec.qty_order += soln_rec.qty_bord;
							soln_rec.qty_bord = 0.00;

							strcpy (soln_rec.status,    "B");
							strcpy (soln_rec.stat_flag, "B");
						}
					}
				}
			}

			cc = abc_update (soln2, &soln_rec);
			if (cc)
				file_err (cc, soln, "DBUPDATE");

			if (tmpLinePtr == tmpItemPtr->lineHead && 
				soln_rec.status [0] == 'B')
			{
				UpdateSohr (soln_rec.hhso_hash, "B");
			}
			else
				UpdateSohr (soln_rec.hhso_hash, "C");

			tmpLinePtr = tmpLinePtr->next;
		}

		add_hash
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
			tmpItemPtr->hhbrHash, 
			0L, 
			0L, 
			(double)0.00
		);
		tmpItemPtr = tmpItemPtr->next;
	}
	recalc_sobg ();
}

/*
 * Log zeroised BO quantities to report.
 */
void
LogBO (
	float	qtyZeroed)
{
	char	delName [41];
	char	delAddr [3][41];
	char	wrkDelAddr [200];

	if (!pipeOpen)
		OpenOutput ();
	
	/*
	 * Look up required info.
	 */
	sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
	cc = find_rec (sohr3, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sohr3, "DBFIND");

	cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	/*
	 * Format delivery address.
	 */
	if (soln_rec.del_no == 0)
	{
		sprintf (delName,    "%-40.40s", sohr_rec.del_name);
		sprintf (delAddr [0], "%-40.40s", sohr_rec.del_add1);
		sprintf (delAddr [1], "%-40.40s", sohr_rec.del_add2);
		sprintf (delAddr [2], "%-40.40s", sohr_rec.del_add3);
	}
	else
	{
		cudi_rec.hhcu_hash = cumr_rec.hhcu_hash;
		cudi_rec.del_no = soln_rec.del_no;
		cc = find_rec (cudi, &cudi_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (delName,    "%-40.40s", sohr_rec.del_name);
			sprintf (delAddr [0], "%-40.40s", sohr_rec.del_add1);
			sprintf (delAddr [1], "%-40.40s", sohr_rec.del_add2);
			sprintf (delAddr [2], "%-40.40s", sohr_rec.del_add3);
		}
		else
		{
			sprintf (delName,    "%-40.40s", cudi_rec.name);
			sprintf (delAddr [0], "%-40.40s", cudi_rec.adr1);
			sprintf (delAddr [1], "%-40.40s", cudi_rec.adr2);
			sprintf (delAddr [2], "%-40.40s", cudi_rec.adr3);
		}
	}
	sprintf 
	(
		wrkDelAddr, 
		"%s, %s, %s, %s", 
		clip (delName), 
		clip (delAddr [0]), 
		clip (delAddr [1]), 
		clip (delAddr [2])
	);

	fprintf (fout, "|%-6.6s",    cumr_rec.dbt_no);
	fprintf (fout, "|%-8.8s",    sohr_rec.order_no);
	fprintf (fout, "|%-16.16s",  inmr_rec.item_no);
	fprintf (fout, "|%-40.40s",  inmr_rec.description);
	fprintf (fout, "|%7.2f ",    qtyZeroed);
	fprintf (fout, "|%-60.60s",  wrkDelAddr);
	fprintf (fout, "|%-10.10s|\n", DateToString (soln_rec.due_date));

	fflush (fout);
}

/*
 * Open pipe to pformat for report.
 */
void
OpenOutput (void)
{
	if ((fout = popen ("pformat", "w")) == (FILE *)NULL)
		sys_err ("Error in pformat During POPEN", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",    local_rec.lpno);

	fprintf (fout, ".11\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s - %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E SCHEDULED ORDERS RELEASE - ZEROISED BACKORDERS\n");
	fprintf (fout, ".E AS AT %s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, "=======");
	fprintf (fout, "=========");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	fprintf (fout, "=============================================================");
	fprintf (fout, "============\n");

	fprintf (fout, "|      ");
	fprintf (fout, "|        ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "|B/O QTY ");
	fprintf (fout, "|                                                            ");
	fprintf (fout, "|          |\n");

	fprintf (fout, "| CUST.");
	fprintf (fout, "| ORDER  ");
	fprintf (fout, "|     ITEM       ");
	fprintf (fout, "|            ITEM DESCRIPTION            ");
	fprintf (fout, "|ZEROISED");
	fprintf (fout, "|                    DELIVERY DETAILS                        ");
	fprintf (fout, "| DUE DATE |\n");

	fprintf (fout, "|------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|------------------------------------------------------------");
	fprintf (fout, "|----------|\n");

	fprintf (fout, ".R=======");
	fprintf (fout, "=========");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	fprintf (fout, "=============================================================");
	fprintf (fout, "==========\n");
}

void
UpdateSohr (
	long	hhsoHash, 
	char	*updStatus)
{
	/*
	 * Check if all lines for order are backordered.
	 */
	if (updStatus [0] == 'B')
	{
		soln_rec.hhso_hash = hhsoHash;
		soln_rec.line_no = 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhso_hash == hhsoHash)
		{
			if (soln_rec.status [0] != 'B')
				return;

			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
	}

	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr3, &sohr_rec, COMPARISON, "u");
	if (!cc)
	{
		strcpy (sohr_rec.sohr_new, "N");
		sprintf (sohr_rec.status, "%-1.1s", updStatus);
		strcpy (sohr_rec.stat_flag, (updStatus [0] == 'B') ? "B" : "M");
		cc = abc_update (sohr3, &sohr_rec);
		if (cc)
			file_err (cc, sohr3, "DBUPDATE");
	}
}

void
ClearRelList (void)
{
	struct	ITEM_LIST	*tmpItemPtr, *delItemPtr;
	struct	LINE_LIST	*tmpLinePtr, *delLinePtr;

	tmpItemPtr = itemHead;
	while (tmpItemPtr != ITEM_NULL)
	{
		tmpLinePtr = tmpItemPtr->lineHead;
		while (tmpLinePtr != LINE_NULL)
		{
			delLinePtr = tmpLinePtr;
			tmpLinePtr = tmpLinePtr->next;

			free (delLinePtr);
		}

		delItemPtr = tmpItemPtr;
		tmpItemPtr = tmpItemPtr->next;

		free (delItemPtr);
	}

	itemHead = ITEM_NULL;
	itemTail = ITEM_NULL;
}

/*
 * Insertion routines for linked lists.
 *
 * Insert node into correct position in sohr linked list. 
 * Key is on Customer Number / Order Number.              
 */
void
InsSohrNode (
	struct SOHR_LIST *newSohrPtr)
{
	int		insertPosFnd;
	char	compKey [32];
	char	newKey [32];
	struct SOHR_LIST *tmpPtr;

	sprintf (newKey, "%-6.6s%-8.8s", newSohrPtr->dbtNo, newSohrPtr->orderNo);

	if (sohrHead == SOHR_NULL)
	{
		sohrHead = newSohrPtr;
		sohrTail = newSohrPtr;
	}
	else
	{
		/*
		 * Find position to insert.
		 */
		insertPosFnd = FALSE;
		tmpPtr = sohrHead;
		while (tmpPtr != SOHR_NULL)
		{
			sprintf (compKey, "%-6.6s%-8.8s", tmpPtr->dbtNo, tmpPtr->orderNo);
			/*
			 * Found insert position ?
			 */
			if (strcmp (newKey, compKey) < 0)
			{
				if (tmpPtr == sohrHead)
				{
					/*
					 * Head insert.
					 */
					newSohrPtr->next = tmpPtr;
					tmpPtr->prev     = newSohrPtr;
					sohrHead         = newSohrPtr;
				}
				else
				{
					/*
					 * Middle insert.
					 */
					newSohrPtr->next   = tmpPtr;
					newSohrPtr->prev   = tmpPtr->prev;
					tmpPtr->prev->next = newSohrPtr;
					tmpPtr->prev       = newSohrPtr;
				}

				insertPosFnd = TRUE;
				break;
			}

			tmpPtr = tmpPtr->next;
		}

		/*
		 * Append to tail.
		 */
		if (!insertPosFnd)
		{
			sohrTail->next   = newSohrPtr;
			newSohrPtr->prev = sohrTail;
			sohrTail         = newSohrPtr;
		}
	}
}

/*
 * Insert node into correct position in item linked list. 
 * Key is on hhbr_hash / hhcc_hash.                      
 */
void
InsItemNode (
	struct ITEM_LIST *newItemPtr)
{
	int		insertPosFnd;
	char	compKey [32];
	char	newKey [32];
	struct 	ITEM_LIST 	*tmpPtr;

	sprintf (newKey, "%010ld%010ld",newItemPtr->hhbrHash, newItemPtr->hhccHash);

	if (itemHead == ITEM_NULL)
	{
		itemHead = newItemPtr;
		itemTail = newItemPtr;
	}
	else
	{
		/*
		 * Find position to insert.
		 */
		insertPosFnd = FALSE;
		tmpPtr = itemHead;
		while (tmpPtr != ITEM_NULL)
		{
			sprintf (compKey, "%010ld%010ld",tmpPtr->hhbrHash,tmpPtr->hhccHash);
			/*
			 * Found insert position ?
			 */
			if (strcmp (newKey, compKey) < 0)
			{
				if (tmpPtr == itemHead)
				{
					/*
					 * Head insert.
					 */
					newItemPtr->next = tmpPtr;
					tmpPtr->prev     = newItemPtr;
					itemHead         = newItemPtr;
				}
				else
				{
					/*
					 * Middle insert.
					 */
					newItemPtr->next   = tmpPtr;
					newItemPtr->prev   = tmpPtr->prev;
					tmpPtr->prev->next = newItemPtr;
					tmpPtr->prev       = newItemPtr;
				}

				insertPosFnd = TRUE;
				break;
			}

			tmpPtr = tmpPtr->next;
		}

		/*
		 * Append to tail.
		 */
		if (!insertPosFnd)
		{
			itemTail->next   = newItemPtr;
			newItemPtr->prev = itemTail;
			itemTail         = newItemPtr;
		}
	}
}
/*
 * Append line node to the end of the list for the specified item / warehouse.
 */
void
InsLineNode (
	struct ITEM_LIST *itemPtr, 
	struct LINE_LIST *newLinePtr)
{
	if (itemPtr->lineHead == LINE_NULL)
	{
		itemPtr->lineHead = newLinePtr;
		itemPtr->lineTail = newLinePtr;
	}
	else
	{
		itemPtr->lineTail->next = newLinePtr;
		newLinePtr->prev        = itemPtr->lineTail;
		itemPtr->lineTail       = newLinePtr;
	}
}
/*
 * Add line node into list for the specified item / warehouse
 */
float
AddLine (
	long	hhbrHash, 
	long	hhccHash, 
	long	hhslHash, 
	long	hhsoHash, 
	float	qtyOrd, 
	float	qtyBord)
{
	int		backOrd;
	float	ordQty;
	struct	ITEM_LIST	*itemPtr;
	struct	LINE_LIST	*newLinePtr;

	backOrd = FALSE;
	if (LineInList (hhslHash) != LINE_NULL)
		return (backOrd);

	/*
	 * Insert in release list.
	 */
	itemPtr = ItemInList (hhbrHash, hhccHash);
	if (itemPtr == ITEM_NULL)
	{
		/*
		 * Get memory for new item node in list.
		 */
		itemPtr = (struct ITEM_LIST *) malloc (sizeof (struct ITEM_LIST));
		if (itemPtr == ITEM_NULL)
			sys_err ("Error in AddLine during MALLOC", errno, PNAME);

		/*
		 * Put data in node.
		 */
		itemPtr->hhbrHash = hhbrHash;
		itemPtr->hhccHash = hhccHash;
		itemPtr->availStk = GetAvail (hhbrHash, hhccHash);
		itemPtr->lineHead = LINE_NULL;
		itemPtr->lineTail = LINE_NULL;
		itemPtr->next     = ITEM_NULL;
		itemPtr->prev     = ITEM_NULL;

		/*
		 * Insert node into list.
		 */
		InsItemNode (itemPtr);
	}

	/*
	 * Get memory for new line node in list. 
	 */
	newLinePtr = (struct LINE_LIST *) malloc (sizeof (struct LINE_LIST));
	if (newLinePtr == LINE_NULL)
		sys_err ("Error in AddLine during MALLOC", errno, PNAME);

	/*
	 * Put data in node.
	 */
	newLinePtr->hhslHash = hhslHash;
	newLinePtr->hhsoHash = hhsoHash;
	newLinePtr->qtyOrd   = qtyOrd;
	newLinePtr->qtyBord  = qtyBord;
	newLinePtr->next     = LINE_NULL;
	newLinePtr->prev     = LINE_NULL;

	/*
	 * Append node to list.
	 */
	InsLineNode (itemPtr, newLinePtr);

	/*
	 * Subtract order quantity from available.
	 */
	ordQty = qtyOrd + qtyBord;
	if (ordQty > itemPtr->availStk || itemPtr->availStk < 0.00)
		backOrd = TRUE;

	itemPtr->availStk -= ordQty;

	return (backOrd);
}
/*
 * Remove line node from list for the specified item / warehouse
 */
void
RemoveLine (
	long	hhbrHash, 
	long	hhccHash, 
	long	hhslHash)
{
	struct	ITEM_LIST	*tmpItemPtr;
	struct	LINE_LIST	*tmpLinePtr;

	/*
	 * First find the item node.
	 */
	tmpItemPtr = itemHead;
	while (tmpItemPtr != ITEM_NULL)
	{
		if (tmpItemPtr->hhbrHash == hhbrHash && 
			tmpItemPtr->hhccHash == hhccHash)
		{
			/*
			 * Next find the line node.
			 */
			tmpLinePtr = tmpItemPtr->lineHead;
			while (tmpLinePtr != LINE_NULL)
			{
				if (tmpLinePtr->hhslHash == hhslHash)
					break;

				tmpLinePtr = tmpLinePtr->next;
			}
			if (tmpLinePtr == LINE_NULL)
				return;

			/*
			 * Now free the node after breaking the links.
			 */
			if (tmpLinePtr->next == LINE_NULL && tmpLinePtr->prev == LINE_NULL)
			{
				tmpItemPtr->lineHead = LINE_NULL;
				tmpItemPtr->lineTail = LINE_NULL;
			}
			else
			{
				if (tmpLinePtr->prev != LINE_NULL)
					tmpLinePtr->prev->next = tmpLinePtr->next;
				if (tmpLinePtr->next != LINE_NULL)
					tmpLinePtr->next->prev = tmpLinePtr->prev;
			}
			free (tmpLinePtr);

			break;
		}
		tmpItemPtr = tmpItemPtr->next;
	}
}

/*
 * Screen Headings.
 */
int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		box (0, 3, 132, 1);
		rv_pr (ML (mlSoMess250), (132 - strlen (ML (mlSoMess250))) / 2, 0, 1);
		line_at (1,0,132);
		line_at (21,0,132);

		print_at (22, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

float	
AddBackValues (
	long	hhbrHash, 
	long	hhccHash)
{
	float	addBackQuantity	=	0.00;

	soln3_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (soln3, &soln3_rec, GTEQ, "r");
	while (!cc && soln3_rec.hhbr_hash == hhbrHash)
	{
		if (soln3_rec.status [0] == 'G' && 
			soln3_rec.hhcc_hash == hhccHash)
		{
			addBackQuantity += soln3_rec.qty_order + soln3_rec.qty_bord;
		}

		cc = find_rec (soln3, &soln3_rec, NEXT, "r");
	}
	return (addBackQuantity);
}
