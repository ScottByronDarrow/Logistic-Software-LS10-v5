/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_ir_post.c,v 5.3 2001/08/09 09:18:48 scott Exp $
|  Program Name  : (sk_ir_post.c) 
|  Program Desc  : (Post Stock Issue from itln/ithr with status "U")
|                  (Duplicates functions of Stock transfer issues.)
|---------------------------------------------------------------------|
|  Date Written  : (08/03/91)      | Author      : Scott Darrow.      |
|---------------------------------------------------------------------|
| $Log: sk_ir_post.c,v $
| Revision 5.3  2001/08/09 09:18:48  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:07  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:11  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_ir_post.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_ir_post/sk_ir_post.c,v 5.3 2001/08/09 09:18:48 scott Exp $";

#define	VAL_ITLN	 (itln_rec.status [0] == 'U')

#define	BY_COMPANY	 (by_what [0] == 'C')
#define	BY_BRANCH	 (by_what [0] == 'B')
#define	BY_WAREHOUSE (by_what [0] == 'W')
#define	VAL_CO		 (!strcmp (iss_co, comm_rec.co_no))
#define VAL_BR 		 (!strcmp (iss_co, comm_rec.co_no) && \
			 		  !strcmp (iss_br, comm_rec.est_no))
#define VAL_WH 		 (!strcmp (iss_co, comm_rec.co_no) && \
			 		  !strcmp (iss_br, comm_rec.est_no) && \
			 		  !strcmp (iss_wh, comm_rec.cc_no))

#define	FULL_SUPPLY	 (ithr_rec.full_supply [0] == 'Y')

#define	LINES_ALL	-1
#define	LINES_SOME	1
#define	LINES_NONE	0

#include	<pslscr.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

struct RCV_CC_LIST
{
	long	issHhccHash;
	long	recHhccHash;
	char	i_co_no [3];
	char	r_co_no [3];
	char	i_br_no [3];
	char	r_br_no [3];
	char	i_cc_no [3];
	char	r_cc_no [3];
	long	hhitHash;
	int	line_no;
	struct RCV_CC_LIST	*next;
};

#define	RCV_CC_NULL	 ((struct RCV_CC_LIST *) NULL)

struct RCV_CC_LIST	*rcv_cc_head = RCV_CC_NULL;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct sobgRecord	sobg_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct ithrRecord	ithr_rec;
struct ithrRecord	ithr2_rec;
struct ithrRecord	ithr3_rec;
struct itlnRecord	itln_rec;

/*==========
 Table Names
============*/
static char
	*data	= "data",
	*ithr2	= "ithr2",
	*ithr3	= "ithr3";

/*=======
 Globals
=========*/
	char	*by_what;

	char	iss_co [3],
			iss_br [3],
			iss_wh [3];

	char	rec_co [3],
			rec_br [3],
			rec_wh [3];

	char	*curr_user;

/*=======================
| Function Declarations |
=======================*/
double 	FindCostValue 			(char *, long, long, float, float);
int  	CheckItln 				(long);
int  	CopyItln 				(long);
int  	ProcessItln 			(long);
struct 	RCV_CC_LIST *FindIthr 	(void);
void 	CloseDB 				(void);
void 	CreateIthr 				(struct RCV_CC_LIST *);
void 	FreeWhList 				(void);
void 	GetNewDocket 			(char *);
void 	OpenDB 					(void);
void 	SetIssBr 				(long, long);
void 	UpdateItln 				(long);

/*==========================
s| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	struct RCV_CC_LIST	*tmp_list;

	switch (argc)
	{
	case 3	:
		by_what = argv [2];
		if (BY_COMPANY || BY_BRANCH || BY_WAREHOUSE)
			break;

	default	:
		print_at (0,0,mlSkMess626, argv [0]);
		return (EXIT_FAILURE);
	}

	curr_user = getenv ("LOGNAME");

	/*----------------
	 Db initialization
	------------------*/
	OpenDB ();

	init_scr ();			/*  sets terminal from termcap	*/

	dsp_screen ("Consolidating / Creating stock issues.",
					comm_rec.co_no, comm_rec.co_name);
	strcpy (ithr_rec.type, "U");
	ithr_rec.del_no = 0L;
	cc = find_rec (ithr, &ithr_rec, GTEQ, "u");
	while (!cc && ithr_rec.type [0] == 'U')
	{
		if (FULL_SUPPLY)
		{
			/*-----------------------------------------------------
			| Check if any lines still have a non-zero order qty. |
			-----------------------------------------------------*/
			if (CheckItln (ithr_rec.hhit_hash))
			{
				abc_unlock (ithr);
				cc = find_rec (ithr, &ithr_rec, NEXT, "u");
				continue;
			}

			UpdateItln (ithr_rec.hhit_hash);

			abc_unlock (ithr);

			/*------------------------------------------------
			 Get on hash id 'cos we're updating part of the index
			--------------------------------------------------*/
			cc = find_hash (ithr3, &ithr3_rec, EQUAL, "u", ithr_rec.hhit_hash);
			if (cc)
				file_err (cc, ithr3, "DBFIND");

			strcpy (ithr3_rec.type, "M");
			if (ithr3_rec.del_no == 0L)
			{
				GetNewDocket (ithr3_rec.co_no);
				ithr3_rec.del_no = comr_rec.nx_del_no;
			}
			if ((cc = abc_update (ithr3, (char *) &ithr3_rec)))
				file_err (cc, ithr3, "DBUPDATE");

		} 
		else
		{
			if (CopyItln (ithr_rec.hhit_hash) == LINES_ALL)
				strcpy (ithr_rec.stat_flag, "D");
			else
				strcpy (ithr_rec.stat_flag, "0");

			if ((cc = abc_update (ithr, &ithr_rec)))
				file_err (cc, "ithr", "UPDATE");
		}
		cc = find_rec (ithr, &ithr_rec, NEXT, "u");
	}
	abc_unlock (ithr);

	/*-----------------------------------------------
	| This is a rather complex method used to		|
	| remove unwanted ithr records but it should	|
	| guarantee that they are ALL deleted!			|
	-----------------------------------------------*/
	strcpy (ithr_rec.type, "U");
	ithr_rec.del_no = 0L;
	cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
	while (!cc && ithr_rec.type [0] == 'U')
	{
		if (ithr_rec.stat_flag [0] == 'D')
		{
			/* 
			 * Set currency of ithr3, then delete
			 */
			if ((cc = find_hash (ithr3, (char *) &ithr_rec, EQUAL, "u", ithr_rec.hhit_hash)))
				file_err (cc, ithr3, "FIND");
			if ((cc = abc_delete (ithr3)))
				file_err (cc, ithr3, "DELETE");
		}
		cc = find_rec (ithr, &ithr_rec, NEXT, "r");
	}

	tmp_list = rcv_cc_head;
	while (tmp_list)
	{
		if ((!find_hash (ithr3, (char *) &ithr3_rec, EQUAL, "u", tmp_list -> hhitHash)))
		{
			if (ProcessItln (ithr3_rec.hhit_hash))
				strcpy (ithr3_rec.type, "M");
			else
				strcpy (ithr3_rec.type, "U");

			if ((cc = abc_update (ithr3, (char *) &ithr3_rec)))
				file_err (cc, ithr3, "UPDATE");
		}
		tmp_list = tmp_list -> next;
	}

	/*========================
	| Program exit sequence. |
	========================*/
	FreeWhList ();
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open Data Dase Files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (ithr3, ithr);
	abc_alias (ithr2, ithr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ithr,  ithr_list, ITHR_NO_FIELDS, "ithr_id_no3");
	open_rec (ithr2, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
	open_rec (ithr3, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (ithr);
	abc_fclose (ithr2);
	abc_fclose (ithr3);
	abc_fclose (itln);
	abc_fclose (ccmr);
	abc_fclose (comr);
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

double	
FindCostValue (
	char 	*branchNumber, 
	long 	hhbrHash, 
	long 	hhwhHash, 
	float 	onHand, 
	float 	quantityIssue)
{
	double	wkCost = 0;

	switch (inmr_rec.costing_flag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		wkCost = 	FindIneiCosts
					(
						inmr_rec.costing_flag,
						branchNumber,
						hhbrHash
					);
		break;

	case 'F':
		wkCost	=	FindIncfValue 
					(
						hhwhHash, 
						onHand, 
						quantityIssue, 
						TRUE,
						inmr_rec.dec_pt
					);
		break;

	case 'I':
		wkCost	=	FindIncfValue 
					(
						hhwhHash, 
						onHand, 
						quantityIssue, 
						FALSE,
						inmr_rec.dec_pt
					);
		break;

	case 'S':
		wkCost = FindInsfValue (hhwhHash, TRUE);
		break;
	}
	if (wkCost < 0.00)
	{
		wkCost = 	FindIneiCosts
					(
						"L",
						branchNumber,
						hhbrHash
					);
	}
	return (wkCost);
}

/*===============================================
| Consolidate itln records where applicable.	|
| If ALL  consolidated, return LINES_ALL		|
| If some consolidated, return LINES_SOME		|
| If none consolidated, return LINES_NONE		|
===============================================*/
int
CopyItln (
	long	hhitHash)
{
	struct RCV_CC_LIST	*tmp_list;
	int		updatedLine 	= FALSE,
			allUpdated 		= TRUE,
			oldLineNo		= 0;	/* for save/restore */

	itln_rec.hhit_hash	= hhitHash;
	itln_rec.line_no 	= 0;

	cc = find_rec (itln, &itln_rec, GTEQ, "u");
	if (cc || itln_rec.hhit_hash != hhitHash)
	{
		abc_unlock (itln);
		return (LINES_ALL);	/* might as well remove dead ithr */
	}

	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (!VAL_ITLN)
		{
			abc_unlock (itln);
			if (itln_rec.status [0] != 'D')
				allUpdated = FALSE;
			cc = find_rec (itln, &itln_rec, NEXT, "u");
			continue;
		}

		SetIssBr (itln_rec.i_hhcc_hash, itln_rec.r_hhcc_hash);

		if ((BY_COMPANY   && !VAL_CO) ||
		    (BY_BRANCH    && !VAL_BR) ||
		    (BY_WAREHOUSE && !VAL_WH))
		{
			abc_unlock (itln);
			cc = find_rec (itln, &itln_rec, NEXT, "u");
			allUpdated = FALSE;
			continue;
		}
		updatedLine = TRUE;

		oldLineNo = itln_rec.line_no;	/* save */

		/*---------------------
		 Move itln to new ithr
		-----------------------*/
		tmp_list = FindIthr ();
		itln_rec.hhit_hash	= tmp_list -> hhitHash;
		itln_rec.line_no	= tmp_list -> line_no++;
		if ((cc = abc_update (itln, (char *) &itln_rec)))
			file_err (cc, itln, "UPDATE");

		/* restore to old values for find_.. NEXT .. */
		itln_rec.hhit_hash	= hhitHash;
		itln_rec.line_no	= oldLineNo;
		cc = find_rec (itln, &itln_rec, GTEQ, "u");
	}
	abc_unlock (itln);

	if (updatedLine)
	{
		if (allUpdated)
			return (LINES_ALL);
		else
			return (LINES_SOME);
	}
	return (LINES_NONE);
}

/*===============================================
| Update itln lines for full supply transfer.
===============================================*/
void
UpdateItln (
	long	hhitHash)
{
	itln_rec.hhit_hash	= hhitHash;
	itln_rec.line_no 	= 0;
	cc = find_rec (itln, &itln_rec, GTEQ, "u");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		strcpy (itln_rec.status, "M");
		strcpy (itln_rec.stat_flag, "0");
		if ((cc = abc_update (itln, &itln_rec)))
			file_err (cc, itln, "UPDATE");

		cc = find_rec (itln, &itln_rec, NEXT, "u");
	}
	abc_unlock (itln);
}

/*============================================
| Update incc & add transaction to g/l file. |
============================================*/
int
ProcessItln (
	long	hhitHash)
{
	int		updatedLine	=	FALSE;
	char	j_ref [7];

	sprintf (j_ref, "%06ld", ithr_rec.del_no);

	dsp_process ("TRANSFER #", j_ref);

	itln_rec.hhit_hash	= hhitHash;
	itln_rec.line_no 	= 0;

	cc = find_rec (itln, &itln_rec, GTEQ, "u");
	if (cc || itln_rec.hhit_hash != hhitHash)
	{
		abc_unlock (itln);
		return (TRUE);
	}

	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		double	value	=	0.00;

		if (!VAL_ITLN)
		{
			abc_unlock (itln);
			cc = find_rec (itln, &itln_rec, NEXT, "u");
			continue;
		}
		SetIssBr (itln_rec.i_hhcc_hash, itln_rec.r_hhcc_hash);

		if ((BY_COMPANY   && !VAL_CO) ||
		    (BY_BRANCH    && !VAL_BR) ||
		    (BY_WAREHOUSE && !VAL_WH))
		{
			abc_unlock (itln);
			cc = find_rec (itln, &itln_rec, NEXT, "u");
			continue;
		}
		updatedLine = TRUE;

		inmr_rec.hhbr_hash	=	itln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		incc_rec.hhcc_hash = itln_rec.i_hhcc_hash;
		incc_rec.hhbr_hash = itln_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, incc, "DBFIND");

		value	=	FindCostValue 
					(
						iss_br,
						inmr_rec.hhbr_hash,
						incc_rec.hhwh_hash,
						incc_rec.closing_stock,
						itln_rec.qty_order
					);
	
		value = out_cost (value, inmr_rec.outer_size);

		itln_rec.qty_rec	= 0.00;
		itln_rec.cost 		= twodec (value);
		itln_rec.duty 		= 0.00;

		strcpy (itln_rec.status, "M");
		strcpy (itln_rec.stat_flag, "0");
		itln_rec.due_date = TodaysDate ();

		if ((cc = abc_update (itln, &itln_rec)))
			file_err (cc, itln, "DBUPDATE");

		add_hash 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"RC",
			0,
			inmr_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			0,
			0.00
		);
		add_hash 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"RP",
			0,
			inmr_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			0,
			0.00
		);
		cc = find_rec (itln, &itln_rec, NEXT, "u");
	}
	abc_unlock (itln);
	recalc_sobg ();
	return (updatedLine);
}
void
SetIssBr (
	long	issHhccHash, 
	long	recHhccHash)
{
	struct RCV_CC_LIST	*rcv_ptr;

	for (rcv_ptr = rcv_cc_head; rcv_ptr; rcv_ptr = rcv_ptr -> next)
	{
		if (rcv_ptr->issHhccHash == issHhccHash &&
		    rcv_ptr->recHhccHash == recHhccHash)
		{
			strcpy (iss_co, rcv_ptr->i_co_no);
			strcpy (iss_br, rcv_ptr->i_br_no);
			strcpy (iss_wh, rcv_ptr->i_cc_no);
			strcpy (rec_co, rcv_ptr->r_co_no);
			strcpy (rec_br, rcv_ptr->r_br_no);
			strcpy (rec_wh, rcv_ptr->r_cc_no);
			return;
		}
	}

	rcv_ptr = (struct RCV_CC_LIST *) malloc (sizeof (struct RCV_CC_LIST));
	if (!rcv_ptr)
		sys_err ("!malloc", errno, PNAME);

	rcv_ptr -> hhitHash = 0L;
	rcv_ptr -> line_no = 0;
	rcv_ptr -> next = rcv_cc_head;
	rcv_cc_head = rcv_ptr;

	if ((cc = find_hash (ccmr, &ccmr_rec, COMPARISON, "r", issHhccHash)))
		file_err (cc, ccmr, "DBFIND");
	rcv_ptr -> issHhccHash = ccmr_rec.hhcc_hash;

	strcpy (rcv_ptr ->i_co_no, strcpy (iss_co, ccmr_rec.co_no));
	strcpy (rcv_ptr ->i_br_no, strcpy (iss_br, ccmr_rec.est_no));
	strcpy (rcv_ptr ->i_cc_no, strcpy (iss_wh, ccmr_rec.cc_no));

	if ((cc = find_hash (ccmr, &ccmr_rec, COMPARISON, "r", recHhccHash)))
		file_err (cc, ccmr, "DBFIND");
	rcv_ptr -> recHhccHash = ccmr_rec.hhcc_hash;

	strcpy (rcv_ptr -> r_co_no, strcpy (rec_co, ccmr_rec.co_no));
	strcpy (rcv_ptr -> r_br_no, strcpy (rec_br, ccmr_rec.est_no));
	strcpy (rcv_ptr -> r_cc_no, strcpy (rec_wh, ccmr_rec.cc_no));
}

void
FreeWhList (
 void)
{
	struct RCV_CC_LIST	*curr;

	curr = rcv_cc_head;
	while (curr)
	{
		struct RCV_CC_LIST	*tmp_list = curr;

		curr = curr -> next;
		free (tmp_list);
	}
}

struct RCV_CC_LIST	*
FindIthr (
 void)
{
	struct RCV_CC_LIST	*curr;

	for (curr = rcv_cc_head; curr; curr = curr -> next)
	{
		if (curr -> issHhccHash == itln_rec.i_hhcc_hash &&
		    curr -> recHhccHash == itln_rec.r_hhcc_hash)
		{
			if (curr -> hhitHash == 0L)
				CreateIthr (curr);
			return (curr);
		}
	}

	sys_err ("Internal error : FindIthr ()", -1, PNAME);
	return (NULL);
}

void
CreateIthr (
 struct RCV_CC_LIST *tmp_list)
{
	GetNewDocket (tmp_list -> i_co_no);

	strcpy (ithr2_rec.co_no, tmp_list -> i_co_no);
	sprintf (ithr2_rec.op_id, "%-14.14s", curr_user);

	ithr2_rec.date_create = TodaysDate ();
	strcpy (ithr2_rec.time_create, TimeHHMM ());
	strcpy (ithr2_rec.type, "X");
	ithr2_rec.del_no 	= comr_rec.nx_del_no;
	ithr2_rec.iss_sdate = TodaysDate ();
	ithr2_rec.iss_date 	= TodaysDate ();
	ithr2_rec.rec_date 	= 0L;
	strcpy (ithr2_rec.tran_ref, "Consolidated TR");
	strcpy (ithr2_rec.printed, "N");
	strcpy (ithr2_rec.stat_flag, "0");
	cc = abc_add (ithr2, &ithr2_rec);
	if (cc)
		file_err (cc, ithr2, "DBADD");

	cc = find_rec (ithr2, &ithr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ithr2, "DBFIND");

	tmp_list -> hhitHash = ithr2_rec.hhit_hash;
}

/*=====================================================
| Check if any lines still have a non-zero order qty. |
=====================================================*/
int
CheckItln (
	long	hhitHash)
{
	itln_rec.hhit_hash	= hhitHash;
	itln_rec.line_no 	= 0;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (itln_rec.qty_border > 0.00)
			return (TRUE);

		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return (FALSE);
}

void
GetNewDocket (
	char *coNo)
{
	strcpy (comr_rec.co_no, coNo);
	cc = find_rec (comr, &comr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, comr, "DBFIND");
	comr_rec.nx_del_no++;
	cc = abc_update (comr, &comr_rec);
	if (cc)
		file_err (cc, comr, "DBUPDATE");
}
