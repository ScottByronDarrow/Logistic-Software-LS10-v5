/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_del_inp.c,v 5.7 2002/07/24 08:39:13 scott Exp $
|  Program Name  : (sk_del_inp.c )                                    |
|  Program Des   : (Inventory Master,Branch, Warehouse Delete Input.) |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_del_inp.c,v $
| Revision 5.7  2002/07/24 08:39:13  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/18 07:15:53  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/07/16 02:43:37  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_del_inp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_del_inp/sk_del_inp.c,v 5.7 2002/07/24 08:39:13 scott Exp $";

#define		TABLINES	14
#include <pslscr.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>

#define ERR_OPENING		1
#define ERR_REC			2
#define ERR_PUR			3
#define ERR_ISS			4
#define ERR_ADJ			5
#define ERR_SALES		6
#define ERR_CLOSING		7
#define ERR_ON_HAND		8
#define ERR_WH_RECORDS  9
#define ERR_BR_RECORDS  10
#define ERR_ORDERS  	11
#define ERR_PS_INV  	12
#define ERR_GRIN  		13
#define ERR_PORDERS  	14
#define ERR_TRANS  		15

#define	LIVE_GRIN	 ((pogl_rec.pur_status [0] == 'A' &&  \
                   	   pogl_rec.gl_status [0] == 'D') ||  \
					   pogl_rec.pur_status [0] == 'D')

	/*
	 * Special fields and flags.
	 */
	int		pid,
			wkNo;	

	char	deleteType [2];

	/*
	 * Error messages for user. 
	 */
	char *errors [15] ;

#include	"schema"

struct commRecord	comm_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct cohrRecord	cohr_rec;
struct poglRecord	pogl_rec;
struct polnRecord	poln_rec;
struct itlnRecord	itln_rec;

	/*
	 * Work file record. 
	 */
	struct {
		char	deleteType [2];
		long	hhbrHash;
		long	hhccHash;
	} workRec;

	struct	storeRec {
		char	itemNo	[sizeof	inmr_rec.item_no];
	} store [MAXLINES];


/*
 * Local & Screen Structures . 
 */
struct {
	char 	dummy 		[11];
	char 	prevItemNo	[sizeof inmr_rec.item_no];
	char 	itemNo 		[sizeof inmr_rec.item_no];
} local_rec;

static	struct	var	vars []	={	

	{1, TAB, "itemNo", MAXLINES, 1, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "     Item No     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.itemNo}, 
	{1, TAB, "itemDescription", 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "           Item Description             ", " ", 
		NA, NO, JUSTLEFT, "", "", inmr_rec.description}, 
	{1, TAB, "hhbrHash", 0, 1, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", " ", 
		ND, NO, JUSTLEFT, "", "", (char *)&inmr_rec.hhbr_hash},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, " ", " "}, 
};


/*
 * Function Declarations 
 */
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReadMisc 				(void);
int  	spec_valid 				(int);
int  	CheckInmr 				(void);
int  	CheckIncc 				(void);
int  	CheckOrders 			(void);
int  	CheckPurchaseOrders 	(void);
int  	CheckGrin 				(void);
int  	CheckTransactions 		(void);
int  	Update 					(void);
int  	heading 				(int);
void 	ProcessError 			(int);
void 	initML 					(void);
int		CheckDuplicateItemNo 	(char *, int);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	int	i = 0;

	SETUP_SCR (vars);


	if (argc < 3) 	
	{
		sprintf (err_str, "%s\n", mlSkMess009);
		print_at (0,0, err_str, argv [0]);
		return (EXIT_FAILURE);
	}
	pid = atoi (argv [1]);
	sprintf (deleteType,"%-1.1s",argv [2]);

	initML ();	
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	tab_row	=	3;
	tab_col	=	14;

	OpenDB ();

	ReadMisc ();

	while (prog_exit == 0)	
	{
   		entry_exit	= FALSE;
   		edit_exit	= FALSE;
   		prog_exit	= FALSE;
   		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		lcount [1] 	= 0;

		for (i = 0; i < MAXLINES; i++)
			strcpy (store [i].itemNo, "                ");

		heading (1);
		entry (1);
		if (prog_exit)
			continue;
			
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
		{
			abc_unlock (inmr);
			continue;
		}
		else
		{
			Update ();
			prog_exit = TRUE;
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
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/sk_del%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	if ((cc = RF_OPEN (filename,sizeof (workRec),"w",&wkNo)) != 0) 
		file_err (cc, filename, "WKOPEN");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhbr_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_hhco_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_hhbr_hash");
	open_rec (pogl, pogl_list, POGL_NO_FIELDS, "pogl_hhbr_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhbr_hash");
	open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_hhbr_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (cohr);
	abc_fclose (soln);
	abc_fclose (coln);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (ccmr);
	abc_fclose (pogl);
	abc_fclose (poln);
	abc_fclose (itln);
	abc_dbclose ("data");

	cc = RF_CLOSE (wkNo);
	if (cc) 
		file_err (cc, "Workfile", "WKCLOSE");
}

/*
 * Get common info from commom database file. 
 */
void
ReadMisc (void)
{
	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

int
spec_valid (
	int		field)
{
	int 	invalid = 0;

	/*
	 * Validate Item Number Input. 
	 */
	if (LCHECK ("itemNo"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.itemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
	    	sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (CheckDuplicateItemNo (local_rec.itemNo, line_cnt))
		{
			errmess (ML ("Duplicate item"));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (store [line_cnt].itemNo, local_rec.itemNo);
		DSP_FLD ("itemDescription");

		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",local_rec.itemNo);
		invalid = CheckInmr ();
		if (invalid)
			return (EXIT_FAILURE);

		return (EXIT_SUCCESS);
	}
	return (invalid);
}

int
CheckInmr (void)
{
	switch (deleteType [0])
	{
	case 'M' :
		/*
		 * Check stock on hand. 
		 */
		if (twodec (inmr_rec.on_hand) != 0)
		{
			ProcessError (ERR_ON_HAND);
			return (EXIT_FAILURE);
		}
		cc = CheckOrders ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckPurchaseOrders ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckGrin ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckTransactions ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		/*
		 * Check for warehouse records. 
		 */
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (!cc)
		{
			ProcessError (ERR_WH_RECORDS);
			return (EXIT_FAILURE);
		}
		/*
		 * Check for branch records. 
		 */
		inei_rec.hhbr_hash = inei_rec.hhbr_hash;
		cc = find_rec (inei,&inei_rec,EQUAL,"r");
		if (!cc)
		{
			ProcessError (ERR_BR_RECORDS);
			return (EXIT_FAILURE);
		}
		break;

	case 'B' :
		/*
		 * Check for warehouse records. 
		 */
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,EQUAL,"r");
		if (!cc)
		{
			ProcessError (ERR_WH_RECORDS);
			return (EXIT_FAILURE);
		}
		break;

	case 'W' :
		/*
		 * Check Warehouse Records. 
		 */
		abc_selfield (incc,"incc_id_no");
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;

		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (!cc)
		{
			cc = CheckIncc ();
			if (cc)
				return (EXIT_FAILURE);
		}
		break;

	case 'A' :
		/*
		 * Check stock on hand. 
		 */
		if (inmr_rec.on_hand != 0)
		{
			ProcessError (ERR_ON_HAND);
			return (EXIT_FAILURE);
		}

		/*
		 * Check Warehouse Records. 
		 */
		abc_selfield (incc,"incc_hhbr_hash");
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,GTEQ,"r");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			cc = CheckIncc ();
			if (cc)
				return (EXIT_FAILURE);

			cc = find_rec (incc,&incc_rec,NEXT,"r");
		}
		cc = CheckOrders ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckPurchaseOrders ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckGrin ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
		cc = CheckTransactions ();
		if (cc)
		{
			ProcessError (cc);
			return (EXIT_FAILURE);
		}
	    	break;
	}
	return (EXIT_SUCCESS);
}

int
CheckIncc (void)
{
	if (twodec (incc_rec.opening_stock) != 0.00)
	{
		ProcessError (ERR_OPENING);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.receipts) != 0.00)
	{
		ProcessError (ERR_OPENING);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.pur) != 0.00)
	{
		ProcessError (ERR_PUR);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.issues) != 0.00)
	{
		ProcessError (ERR_ISS);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.adj) != 0.00)
	{
		ProcessError (ERR_ADJ);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.sales) != 0.00)
	{
		ProcessError (ERR_SALES);
		return (EXIT_FAILURE);
	}
	if (twodec (incc_rec.closing_stock) != 0.00)
	{
		ProcessError (ERR_CLOSING);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check for valid Orders. 
 */
int
CheckOrders (void)
{
	soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (soln, &soln_rec, EQUAL, "r");
	if (!cc)
		return (ERR_ORDERS);

	coln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (coln, &coln_rec, EQUAL, "r");
	if (!cc)
	{
		cohr_rec.hhco_hash	=	coln_rec.hhco_hash;
		cc = find_rec (cohr,&cohr_rec,EQUAL,"r");
		if (cc)
			return (EXIT_SUCCESS);

		if (cohr_rec.stat_flag [0] == '9' || cohr_rec.stat_flag [0] == 'D')
			return (EXIT_SUCCESS);

		return (ERR_PS_INV);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check for valid Purchase orders. 
 */
int
CheckPurchaseOrders (void)
{
	poln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (poln,&poln_rec,EQUAL,"r"))
		return (ERR_PORDERS);

	return (EXIT_SUCCESS);
}

/*
 * Check for valid goods receipts. 
 */
int
CheckGrin (void)
{
	pogl_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	cc = find_rec (pogl, &pogl_rec, GTEQ, "r");
	while (!cc && pogl_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		if (!LIVE_GRIN)
			return (ERR_GRIN);

		cc = find_rec (pogl, &pogl_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*
 * Check for valid goods receipts. 
 */
int
CheckTransactions (void)
{
	abc_selfield (itln, "itln_hhbr_hash");

	itln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (itln, &itln_rec, EQUAL, "r"))
		return (ERR_TRANS);

	abc_selfield (itln, "itln_r_hhbr_hash");

	itln_rec.r_hhbr_hash	=	inmr_rec.hhbr_hash;
	if (!find_rec (itln, &itln_rec, EQUAL, "r"))
		return (ERR_TRANS);

	return (EXIT_SUCCESS);
}

int
Update (void)
{
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++) 
	{
		getval (line_cnt);

		if (!strcmp (local_rec.itemNo,"                "))
			continue;

		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",local_rec.itemNo);
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"u");
		if (cc) 
			file_err (cc, inmr, "DBFIND");
			
		strcpy (inmr_rec.stat_flag,"9");
		cc = abc_update (inmr,&inmr_rec);
		if (cc) 
			file_err (cc, inmr, "DBUPDATE");
			
		strcpy (workRec.deleteType,deleteType);
		workRec.hhbrHash = inmr_rec.hhbr_hash;
		workRec.hhccHash = ccmr_rec.hhcc_hash;
		cc = RF_ADD (wkNo, (char *) &workRec);
		if (cc) 
			file_err (cc, "RF_ADD", "WKADD");
				
		strcpy (local_rec.prevItemNo,inmr_rec.item_no);
	}
	abc_unlock (inmr);
	return (0);
}

int
heading (
	int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		switch (deleteType [0])
		{
		case 'M':
			rv_pr (ML (mlSkMess010),20,0,1);
			break;

		case 'B':
			rv_pr (ML (mlSkMess011),20,0,1);
			break;

		case 'W':
			rv_pr (ML (mlSkMess012),20,0,1);
			break;

		case 'A':
			rv_pr (ML (mlSkMess013),20,0,1);
			break;
		}

		print_at (0,50,ML (mlSkMess096),local_rec.prevItemNo); 
		line_at (1,0,80);
		line_at (19,0,80);
		line_at (22,0,80);

		print_at (20,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
ProcessError (
	int		errNo)
{
	errmess (errors [errNo - 1]);
	sleep (sleepTime);
}

void
initML (void)
{
	errors [0] = strdup (ML (mlSkMess690));
	errors [1] = strdup (ML (mlSkMess691));
	errors [2] = strdup (ML (mlSkMess692));
	errors [3] = strdup (ML (mlSkMess693));
	errors [4] = strdup (ML (mlSkMess694));
	errors [5] = strdup (ML (mlSkMess695));
	errors [6] = strdup (ML (mlSkMess696));
	errors [7] = strdup (ML (mlSkMess697));
	errors [8] = strdup (ML (mlSkMess698));
	errors [9] = strdup (ML (mlSkMess699));
	errors [10] = strdup (ML (mlSkMess700));
	errors [11] = strdup (ML (mlSkMess701));
	errors [12] = strdup (ML (mlSkMess702));
	errors [13] = strdup (ML (mlSkMess703));
	errors [14] = strdup (ML (mlSkMess704));
}
/*
 * Check whether Customer has been input before.
 * Return 1 if duplicate					       
 */
int
CheckDuplicateItemNo (
	char	*itemNo,
	int		lineNumber)
{
	int		i;
	int		noItems = (prog_status == ENTRY) ? line_cnt : lcount [1];

	for (i = 0;i < noItems;i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == lineNumber)
			continue;

		/*
		 * cannot be duplicate if not input.
		 */
		if (!strcmp (store [i].itemNo, "                "))
			continue;

		if (!strcmp (store [i].itemNo, itemNo))
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
