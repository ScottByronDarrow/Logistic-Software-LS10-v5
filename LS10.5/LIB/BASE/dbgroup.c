/*=====================================================================
|  Copyright (C) 1999 - 1999 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : ( dbgroup.c  	)                                 |
|  Program Desc  : ( Customer Statement Grouping Module            )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Date Written  : (27/01/96)      | Author       : Basil Wood        |
|---------------------------------------------------------------------|
|  $Log: dbgroup.c,v $
|  Revision 5.1  2001/08/06 22:40:54  scott
|  RELEASE 5.0
|
|  Revision 5.0  2001/06/19 06:59:15  cha
|  LS10-5.0 New Release as of 19 JUNE 2001
|
|  Revision 4.0  2001/03/09 00:52:36  scott
|  LS10-4.0 New Release as at 10th March 2001
|
|  Revision 3.0  2000/10/12 13:34:19  gerry
|  Revision No. 3 Start
|  <after Rel-10102000>
|
|  Revision 2.0  2000/07/15 07:17:13  gerry
|  Forced revision no. to 2.0 - Rel-15072000
|
|  Revision 1.10  2000/02/16 01:24:27  jonc
|  Oops, missed a few malloc checks the last time around.
|
|  Revision 1.9  2000/02/16 01:03:34  jonc
|  Fixed memory corruption.
|
|  Revision 1.8  1999/11/28 21:56:46  jonc
|  Removed ill-advised define's
|
|  Revision 1.7  1999/11/26 01:01:19  jonc
|  AgePeriod() extended to include true_age parameter.
|
|  Revision 1.6  1999/11/19 03:55:46  scott
|  Updated for removal of old date routines
|
|  Revision 1.5  1999/11/16 23:38:17  scott
|  Updated for comment conflicts with old compilers.
|
|  Revision 1.4  1999/09/30 04:37:31  scott
|  Updated for date routines
|
|  Revision 1.3  1999/09/13 09:36:28  scott
|  Updated for Copyright
|
|  Revision 1.2  1999/09/13 06:20:46  alvin
|  Check-in all ANSI modifications made by Trev.
|
|  Revision 1.1.1.1  1999/06/10 11:56:33  jonc
|  Initial cutover from SCCS.
|
|  Revision 2.7  1998/07/29 22:12:20  jonc
|  FBL 14894. Linking journals appearing on statements for H/O debtors.
|
|  Revision 2.6  1998/01/21 23:20:32  jonc
|  Added minor comments
|
|  Revision 2.5  1997/05/06 23:07:40  jonc
|  Changes to handle schema extensions correctly
|
|  Revision 2.4  1997/05/06 21:56:22  jonc
|  Added - non-crash failure status
|        - error file for failure logs
|
|  Revision 2.3  1997/04/22 22:15:38  jonc
|  Fixed overzealous ageing bias for b/f statements
|
|  Revision 2.2  1997/04/21 03:37:22  jonc
|  Fixes for statement prints
|
|  Revision 2.1  1997/03/13 03:46:09  jonc
|  Copied from v9.0
|
=====================================================================*/

/*
Autoduck tagged comment lines for generation of API documentation:
@doc
@module dbgroup.c | Customer Statement Grouping

	The source file dbgroup.c is identical for version 7 and version 9.
	Version 7 is enabled by defining PSL_VER with a value of 7.

@topic The debtors grouping process. |

	A group refers to those transactions for a customer that are
	linked together, the links being cuhd to cudt via the hhcp_hash
	and the cuin to the cudt via the hhci hash:

    -	There is a 1 to many relationship between the cuhd
		and the cudt;

    -	There is a 1 to many relationship between the cuin
		and the cudt;

	-	The value of the cuhd record should always equal the
		sum of the linked cudt's.

	In the ideal world there will be 1 cuin record paid by 1 cudt
	referenced by 1 cuhd. In reality many cudts (1 or more cuhd)
	will offset a cuin.

	Type 1 cuins = invoice, type 2 = credit note, type 3 = journal

	Type 1 cuhd = cash receipt, type 2 cuhd = linking journal

	cuin, cuhd and cudt records are deleted when the transactions
	balance to zero and the age of the most recent transaction is
	greater than the number of days as specified by the environment
	variable PURGE_MON.

	There are two types of debtors statements being brought forward
	balance, and open item:

	-	Brought forward type produces a balance brought forward
		total from the last statement and only details current
		month transactions.

	-	An open item statement shows all outstanding transactions.

	The debtors group is used in the calculation of ageing and in the
	purge process:

	-	If a customer has an open-item statement type then
		transactions are not purged unless the group totals zero.

	-	If a customer has a balance brought forward statement type
		then the transactions are purged if the invoice balance
		(cuin less cudt's = O). If there are multiple cudt's
		linked to a cuhd then the cuhd total is reduced by the
		value of the deleted cudt's.

	In version 7 there is a column called cuin_child_hash that holds
	the hhcu_hash of the originally transacted customer:

	-	When the invoice (cuin) is created the cuin_hhcu_hash and
		the cuin child_hash are set to the hhcu_hash of the
		transacted customer.

	-	When the update head office function is run (part of
		so_invledup or db_ho_proc) then the cuin_hhcu_hash is
		changed to the hhcu_hash of the head office customer
		(if applicable).

	-	The cumr holds the column cumr_ho_dbt_hash being the hash
		of the head office account.

	In version 9 there is a column called cuin_ho_hash that holds
	the hhcu_hash of the head office customer. Unlike version 7,
	cuin_hhcu_hash points to the customer that incurred the
	transaction.

	When a statement is produced for a head-office customer the
	program is able to provide a subanalysis of the child accounts
	within the account using the cuin_child_hash.

	Most of Logistic's customers are using open item statements.

	Most Logistic customers are not as neat and tidy with their cash
	receipt allocation process with the result being that the number
	of transactions in the groups are getting larger and larger as
	the groups never equal zero and are therefore never purged.

	There is no simple process that the customers can use to display
	the group details and therefore identify which transactions are
	causing the purge process to fail.

	There does not appear to be any consistency in the way that the
	programs handling the debtors grouping work.

	The DPL statement has some extra code which controls the printing
	of transactions on open-item statements. Their purge program has
	been enhanced such that the transactions are flagged when they
	balance to zero, and the statement print program only prints
	unflagged transactions. i.e. the open-item transactions are
	printed until they are effectively paid-off.

	The purge process still does not delete the records until the
	group totals zero.

	Logistic will probably move to the DPL statement/purge logic.
*/

#include	<osdefs.h>
#include	<dbio.h>

#include	<stdio.h>
#include	<std_decs.h>
#include	<math.h>
#include	<assert.h>

/*
 * Jonc,
 * These should be declared once in an include file accessed by all modules.
 */
#define	FALSE	0
#define	TRUE	1

extern char *PNAME;

static int		_StmtGroupDaysAgeing = 0,
				_true_age = 0;

#include	<dbgroup.h>

/*
 *	Database requirements
 */
	/*
	 *	Customer master record
	 */
#define	CUMR_NO_FIELDS	2

	static struct dbview	cumr_list [CUMR_NO_FIELDS] =
	{
		{"cumr_hhcu_hash"},
		{"cumr_ho_dbt_hash"}
	};

	struct tag_cumrRecord
	{
		long	hhcu_hash;
		long	ho_dbt_hash;
	};

	/*======================================+
	 | Customer Payments Header File Record. |
	 +======================================*/
#define	CUHD_NO_FIELDS	10

	static struct dbview	cuhd_list [CUHD_NO_FIELDS] =
	{
		{"cuhd_hhcu_hash"},
		{"cuhd_receipt_no"},
		{"cuhd_type"},
		{"cuhd_hhcp_hash"},
		{"cuhd_narrative"},
		{"cuhd_date_payment"},
		{"cuhd_date_posted"},
		{"cuhd_tot_amt_paid"},
		{"cuhd_disc_given"},
		{"cuhd_stat_flag"}
	};

	static struct tag_cuhdRecord
	{
		long	hhcu_hash;
		char	receipt_no [9];
		char	type [2];
		long	hhcp_hash;
		char	narrative [21];
		Date	date_payment;
		Date	date_posted;
		Money	tot_amt_paid;
		Money	disc_given;
		char	stat_flag [2];
	} _cuhd_rec;

	/*=================================+
	 | Customer Payments Detail Record. |
	 +=================================*/
#define	CUDT_NO_FIELDS	4

	static struct dbview	cudt_list [CUDT_NO_FIELDS] =
	{
		{"cudt_hhcp_hash"},
		{"cudt_hhci_hash"},
		{"cudt_amt_paid_inv"},
		{"cudt_stat_flag"}
	};

	static struct tag_cudtRecord
	{
		long	hhcp_hash;
		long	hhci_hash;
		Money	amt_paid_inv;
		char	stat_flag [2];
	} _cudt_rec;

	/*=================================================+
	 | Customer Invoice Accounting Invoice/Credit file. |
	 +=================================================*/
#define	CUIN_NO_FIELDS	14

	static struct dbview	cuin_list [CUIN_NO_FIELDS] =
	{
		{"cuin_hhcu_hash"},
		{"cuin_hhci_hash"},
		{"cuin_ho_hash"},
		{"cuin_type"},
		{"cuin_est"},
		{"cuin_inv_no"},
		{"cuin_narrative"},
		{"cuin_date_of_inv"},
		{"cuin_date_posted"},
		{"cuin_pay_terms"},
		{"cuin_due_date"},
		{"cuin_disc"},
		{"cuin_amt"},
		{"cuin_stat_flag"}
	};

	static struct tag_cuinRecord
	{
		long	hhcu_hash;
		long	hhci_hash;
		long	ho_hash;
		char	type [2];
		char	est [3];
		char	inv_no [9];
		char	narrative [21];
		Date	date_of_inv;
		Date	date_posted;
		char	pay_terms [4];
		Date	due_date;
		Money	disc;
		Money	amt;
		char	stat_flag [2];
	} _cuin_rec;


static char
	*cumr	= "_gp_cumr",			/* internally referenced names */
	*cuinho	= "_gp_cuinh",
	*cudt	= "_gp_cudt",
	*cuhd	= "_gp_cuhd",
	*cuin2	= "_gp_cuin2";


static Date	_dbt_date;				/* comm_dbt_date */

/*
 *	Statement Grouping
 */

/*
 *	Internal function declarations
 *	Functions return TRUE if successful, unless specified otherwise.
 */

static int
_Compare (
	DGroupItem *src,
	DGroupItem *dst,
	int	isGroupByChild)
{
	assert (src != NULL);
	assert (dst != NULL);
	
	if (isGroupByChild)
	{
		if (src->child_hhcu < dst->child_hhcu)
			return -1;
		if (src->child_hhcu > dst->child_hhcu)
			return 1;
	}

	if (src->date < dst->date)
		return -1;
	if (src->date > dst->date)
		return 1;

	return strcmp (src->doc_no, dst->doc_no);
}

/*=======================================
| Copy ALL entries in the 'live'
| list to the 'free' list.
=======================================*/

static DGroup 		*_pFreeGroupList;
static DGroupItem	*_pFreeGroupItemList;

static DGroupItem *
_AllocGroupItem (
	DG_Source	source,
	long		child_hhcu,
	Date		date,
	long		hhci_hash,
	long		hhcp_hash,
	char		*doc_no,
	DG_Type	type,
	Money		value,
	int			period	)
{
	DGroupItem *pGroupItem;

	if (_pFreeGroupItemList)
	{
		pGroupItem = _pFreeGroupItemList;
		_pFreeGroupItemList = _pFreeGroupItemList->_pNextItem;
	}
	else
	{
		pGroupItem = (DGroupItem *) malloc (sizeof (DGroupItem));
		if (!pGroupItem)
			sys_err ("Error in _AllocGroupItem() during malloc", errno, PNAME);
	}
	memset (pGroupItem, 0, sizeof (DGroupItem));

	pGroupItem->source = source;
	pGroupItem->child_hhcu = child_hhcu;
	pGroupItem->date = date;
	pGroupItem->hhci_hash = hhci_hash;
	pGroupItem->hhcp_hash = hhcp_hash;
	strcpy (pGroupItem->doc_no, doc_no);
	pGroupItem->type = type;
	pGroupItem->value = value;
	pGroupItem->period = period;
    pGroupItem->_pNextItem = NULL;

	return pGroupItem;
}

static DGroup *
_AllocGroup (
	DGroupItem *pGroupItem	)
{
	DGroup	*pGroup;

	if (_pFreeGroupList)
	{
		pGroup = _pFreeGroupList;
		_pFreeGroupList = _pFreeGroupList->_pNextGroup;
	}
	else
		pGroup = (DGroup *) malloc (sizeof (DGroup));
	memset (pGroup, 0, sizeof (DGroup));

    pGroup->_pNextGroup = NULL;
    pGroup->_pFirstItem = pGroupItem;

	return pGroup;
}

static void
_DeallocAllGroupItems (
	DGroupItem	*pGroupItem	)
{
	/*-------------------------------------
	| Add items to the Free Group Item List
	-------------------------------------*/
	while (pGroupItem)
	{
		DGroupItem *titem = pGroupItem;

		pGroupItem = pGroupItem->_pNextItem;

		titem->_pNextItem = _pFreeGroupItemList;
		_pFreeGroupItemList = titem;
	}
}

static void
_DeallocGroupItem (
	DGroupItem	*pGroupItem	)
{
	pGroupItem->_pNextItem = NULL;
	_DeallocAllGroupItems (pGroupItem);
}

static void
_DeallocAllGroups (
	DGroup	*pGroup	)
{
	/*---------------------------------
	| Add groups to the Free Group List
	---------------------------------*/
	while (pGroup)
	{
		DGroup *tgroup = pGroup;

		pGroup = pGroup->_pNextGroup;

		tgroup->_pNextGroup = _pFreeGroupList;
		_pFreeGroupList = tgroup;

		_DeallocAllGroupItems (tgroup->_pFirstItem);
		tgroup->_pFirstItem = NULL;
	}
}

static void
_DeallocGroup (
	DGroup	*pGroup	)
{
	pGroup->_pNextGroup = NULL;
	_DeallocAllGroups (pGroup);
}

static DG_Type
_GetType (
	DG_Source	source,
	char 		type	)
{
	switch (source)
	{
	case DG_cuhd:
		switch (type)
		{
		case '1': return DG_Cheque;
		case '2': return DG_JnlOne;
		}
		break;

	case DG_cuin:
		switch (type)
		{
		case '1': return DG_Invoice;
		case '2': return DG_Credit;
		case '3': return DG_JnlTwo;
		}
		break;
	default:
		break;
	}
	return DG_Invalid;
}

/*=======================================
| Insert a group into a list of groups.
| Based on DPL db_stmtprn List_insert().
=======================================*/
static void
_InsertGroup (
	DGroupSet	*pGroupSet,
 	DGroup		*pGroup	)
{
	DGroup		*prevGroup;
	DGroupItem	*pItem;

	assert (pGroup->_pFirstItem != NULL);
	assert (pGroup->_pNextGroup == NULL);

	/*---------------------------------------
	| empty list
	---------------------------------------*/
	if (!pGroupSet->_pFirstGroup)
	{
		pGroupSet->_pFirstGroup = pGroup;
		return;
	}

	/*---------------------------------------
	| Check for insert at top of list.
	---------------------------------------*/
	pItem = pGroup->_pFirstItem;

	if (_Compare (	pItem,
					pGroupSet->_pFirstGroup->_pFirstItem,
					pGroupSet->_isGroupByChild) < 0)
	{
		pGroup->_pNextGroup = pGroupSet->_pFirstGroup;
		pGroupSet->_pFirstGroup = pGroup;
		return;
	}

	/*---------------------------------------
	| Find point at which to insert record.
	---------------------------------------*/
	for (prevGroup = pGroupSet->_pFirstGroup;
		 prevGroup->_pNextGroup;
		 prevGroup = prevGroup->_pNextGroup)
	{
		if (_Compare (	pItem,
						prevGroup->_pNextGroup->_pFirstItem,
						pGroupSet->_isGroupByChild) < 0)
		{
			/*---------------------------------------
			| Record inserted after previous group
			---------------------------------------*/
			pGroup->_pNextGroup = prevGroup->_pNextGroup;
			prevGroup->_pNextGroup = pGroup;
			return;
		}
	}

	/*---------------------------------------
	| Record inserted at end of list
	---------------------------------------*/
	prevGroup->_pNextGroup = pGroup;
}

static void
_InsertGroupItem (
 	DGroup		*pGroup,
 	DGroupItem	*pItem,
 	int			isGroupByChild	)
{
	DGroupItem	*prevItem;

	assert (pGroup != NULL);
	assert (pItem != NULL);
	assert (pItem->_pNextItem == NULL);

	/*---------------------------------------
	| empty list
	---------------------------------------*/
	if (!pGroup->_pFirstItem)
	{
		pGroup->_pFirstItem = pItem;
		return;
	}

	/*---------------------------------------
	| Check for insert at top of list.
	---------------------------------------*/
	if (_Compare (pItem, pGroup->_pFirstItem, isGroupByChild) < 0)
	{
		pItem->_pNextItem = pGroup->_pFirstItem;
		pGroup->_pFirstItem = pItem;
		return;
	}

	/*---------------------------------------
	| Find point at which to insert record.
	---------------------------------------*/
	for (prevItem = pGroup->_pFirstItem;
		 prevItem->_pNextItem;
		 prevItem = prevItem->_pNextItem)
	{
		if (_Compare (pItem, prevItem->_pNextItem, isGroupByChild) < 0)
		{
			/*---------------------------------------
			| Record inserted after previous group
			---------------------------------------*/
			pItem->_pNextItem = prevItem->_pNextItem;
			prevItem->_pNextItem = pItem;
			return;
		}
	}

	/*---------------------------------------
	| Record inserted at end of list
	---------------------------------------*/
	prevItem->_pNextItem = pItem;
}

/*==================================
| Traverse both lists and insert
| current source node before current
| destination node if it is less.
| Based on DPL db_stmtprn List_merge().
==================================*/
static void
_MergeGroups (
	DGroupSet	*pGroupSet,
	DGroup		*dstGroup,
	DGroup		*srcGroup	)
{
	DGroupItem *src_ptr, *prv_ptr, *dst_ptr;
	int period;
		
	for (period = 0; period < 6; period++)
	{
		dstGroup->gtotal[period] +=	srcGroup->gtotal[period];
		srcGroup->gtotal[period] = 0.0;
	}

	src_ptr = srcGroup->_pFirstItem;
	if (src_ptr)
		srcGroup->_pFirstItem = src_ptr->_pNextItem;

	dst_ptr = dstGroup->_pFirstItem;
	prv_ptr = NULL;

	while (src_ptr && dst_ptr)
	{
		if (_Compare (src_ptr, dst_ptr, pGroupSet->_isGroupByChild) < 0)
		{
			/*-----------------------------
			| Insert src node before dst
			-----------------------------*/
			src_ptr->_pNextItem = dst_ptr;
			dst_ptr = src_ptr;
			if (prv_ptr)
				prv_ptr->_pNextItem = src_ptr;
			else
				dstGroup->_pFirstItem = src_ptr;

			/*-----------------------------
			| Move to next node in src list
			-----------------------------*/
			src_ptr = srcGroup->_pFirstItem;
			if (src_ptr)
				srcGroup->_pFirstItem = src_ptr->_pNextItem;
		}
		else
		{
			/*-----------------------------
			| Move to next node in dst list
			-----------------------------*/
			prv_ptr = dst_ptr;
			dst_ptr = dst_ptr->_pNextItem;

			#ifdef _DEBUG
				if (dst_ptr)
				{
					int list_is_ordered =
							(_Compare (	prv_ptr,
										dst_ptr,
										pGroupSet->_isGroupByChild) <= 0);
					assert (list_is_ordered);
				}
			#endif
		}
	}

	if (!dst_ptr)
	{
    	/*-----------------------------
    	| At end of destination list so
    	| transfer all of source list
    	| to end of destination list
    	------------------------------*/
		if (prv_ptr)
			prv_ptr->_pNextItem = src_ptr;
		else
			dstGroup->_pFirstItem = src_ptr;
		srcGroup->_pFirstItem = NULL;
	}
}

/*=========================
| Return pointer to item
| this invoice belongs to
| or NULL if not found.
=========================*/
static DGroupItem *
_LocateInvoiceItem (
 	DGroupSet	*pGroupSet,
 	long		hhci_hash	)
{
	DGroup 		*pGroup;
	DGroupItem 	*pItem;

	for (pGroup = pGroupSet->_pFirstGroup;
		 pGroup;
		 pGroup = pGroup->_pNextGroup)
	{
		for (pItem = pGroup->_pFirstItem;
			 pItem;
			 pItem = pItem->_pNextItem)
		{
			if (pItem->hhci_hash == hhci_hash)
			{
				return pItem;
			}
		}
	}

	return NULL;
}

/*=====================================
| Remove group this invoice belongs to.
| Return pointer to group or
| NULL if not found.
=====================================*/
static DGroup *
_RemoveInvoiceGroup (
 	DGroupSet	*pGroupSet,
 	long  		hhci_hash	)
{
	DGroup 	*pGroup, *prevGroup;
	DGroupItem *pItem;

	prevGroup = NULL;
	for (pGroup = pGroupSet->_pFirstGroup;
		 pGroup;
		 pGroup = pGroup->_pNextGroup)
	{
		for (pItem = pGroup->_pFirstItem;
			 pItem;
			 pItem = pItem->_pNextItem)
		{
			if (pItem->hhci_hash == hhci_hash)
			{
				/*-----------------------------
				| Found invoice so remove group
				-----------------------------*/
				if (prevGroup)
					prevGroup->_pNextGroup = pGroup->_pNextGroup;
				else
					pGroupSet->_pFirstGroup = pGroup->_pNextGroup;
				pGroup->_pNextGroup = NULL;

				return pGroup;
			}
		}
		prevGroup = pGroup;
	}

	return NULL;
}

static void
_ProcessInvoice (
 	DGroupSet	*pGroupSet,
 	long		hhcu_hash	)
{
	long 		child_hhcu;
	DG_Type		type;
	Money		value;
	int			period;
	DGroup		*pGroup;
	DGroupItem	*pItem;

	child_hhcu = _cuin_rec.hhcu_hash;

	type = _GetType(DG_cuin, *_cuin_rec.type);
	value = -1.0 * (_cuin_rec.amt - _cuin_rec.disc);
	period = AgePeriod 
			(
				_cuin_rec.pay_terms, 
				_cuin_rec.date_of_inv, 
				_dbt_date,
				_cuin_rec.due_date, 
				_StmtGroupDaysAgeing,
				_true_age
			);

	pItem = _AllocGroupItem (	DG_cuin,
								child_hhcu,
								_cuin_rec.date_of_inv,
								_cuin_rec.hhci_hash,
								0L,
								_cuin_rec.inv_no,
								type,
								value,
								period	);
	pGroup = _AllocGroup (pItem);
	_InsertGroup (pGroupSet, pGroup);

	if (period == -1)
	{
		pGroupSet->total[5] -= value;
		pGroup->gtotal[5] -= value;
	}
	else
	{
		pGroupSet->total[period] -= value;
		pGroup->gtotal[period] -= value;
	}
}

static int
_ReadInvoices (
 	DGroupSet	*pGroupSet,
 	long		hhcu_hash	)	/* Head customer (but can also be a child) */
{
	int	err;

	/*------------------------------------
	| Read all the invoice transactions
	| from cuin given head customer hash.
	------------------------------------*/
	memset (&_cuin_rec, 0, sizeof _cuin_rec);
	_cuin_rec.ho_hash = hhcu_hash;
	for (err = find_rec (cuinho, &_cuin_rec, GTEQ, "r");
		 !err && _cuin_rec.ho_hash == hhcu_hash;
		 err = find_rec (cuinho, &_cuin_rec, NEXT, "r"))
	{
		_ProcessInvoice (pGroupSet,	hhcu_hash);
	}

	/*-------------------------------------------
	| Read the child invoice transactions from
	| cuin given head (or child) customer hash.
	-------------------------------------------*/
	memset (&_cuin_rec, 0, sizeof _cuin_rec);
	_cuin_rec.hhcu_hash = hhcu_hash;
	for (err = find_rec (cuin2, &_cuin_rec, GTEQ, "r");
		 !err && _cuin_rec.hhcu_hash == hhcu_hash;
		 err = find_rec (cuin2, &_cuin_rec, NEXT, "r"))
	{
		if (_cuin_rec.hhcu_hash == _cuin_rec.ho_hash)
			continue; /* already processed */

		_ProcessInvoice (pGroupSet,	hhcu_hash);
	}

	return TRUE;
}

/*=======================================
| Read and process ALL available cudt
| records for the current cuhd record.
| Based on DPL db_stmtprn proc_cudt().
=======================================*/
static int
_ReadPaymentDetails (
 DGroupSet	*pGroupSet,
 long		hhcp_hash,
 long *		bad_hhcp,
 long *		bad_hhci)
{
	int			err;
	DG_Type		type;
	long    	curr_child = 0L;
		
	DGroup		*cudt_group = _AllocGroup (NULL);
	DGroupItem	*cudt_item;

    assert (hhcp_hash == _cuhd_rec.hhcp_hash);

	/*----------------------------
	| Get associated detail payment
	| lines into cudt_group.
	----------------------------*/
	for (err = find_hash (cudt, &_cudt_rec, EQUAL, "r", hhcp_hash);
		!err && _cudt_rec.hhcp_hash == hhcp_hash;
		err = find_hash (cudt, &_cudt_rec, NEXT, "r", hhcp_hash))
	{
		DGroupItem	*pItem;
		Money		value;

		pItem = _LocateInvoiceItem (pGroupSet, _cudt_rec.hhci_hash);
		if (!pItem)
		{
			*bad_hhcp = hhcp_hash;
			*bad_hhci = _cudt_rec.hhci_hash;

			return FALSE;
		}
		curr_child = pItem->child_hhcu;
		value = _cudt_rec.amt_paid_inv;
		pItem = _AllocGroupItem (	DG_cudt,
									pItem->child_hhcu,
									pItem->date,
									pItem->hhci_hash,
									hhcp_hash,
									pItem->doc_no,
									pItem->type,
									value,
									pItem->period	);

		pGroupSet->total[pItem->period != -1 ? pItem->period : 5] -= value;

		_InsertGroupItem (cudt_group, pItem, pGroupSet->_isGroupByChild);
    }

	/*-------------------------------
	| If this is an 'APP ' (applied to)
	| cuhd, process the other half.
	-------------------------------*/
	if (atoi (_cuhd_rec.type) == 2 &&
		!strncmp (_cuhd_rec.narrative, "APP ", 4))
	{
		struct tag_cuinRecord cuin2_rec;
		int cc;

		memset (&cuin2_rec, 0, sizeof (struct tag_cuinRecord));
		cuin2_rec.hhcu_hash = _cuhd_rec.hhcu_hash;
		strcpy (cuin2_rec.inv_no, _cuhd_rec.receipt_no);
		cc = find_rec (cuin2, &cuin2_rec, EQUAL, "r");

		if (!cc)
		{
			DGroupItem	*pItem;

			pItem = _LocateInvoiceItem (pGroupSet, cuin2_rec.hhci_hash);
			if (!pItem)
			{
				*bad_hhcp = _cudt_rec.hhcp_hash;
				*bad_hhci = cuin2_rec.hhci_hash;

				return FALSE;
			}

			if (!pGroupSet->_isGroupByChild || curr_child == pItem->child_hhcu)
			{
				Money value;

				value = -1.0 * (_cuhd_rec.tot_amt_paid - _cuhd_rec.disc_given);
				pItem = _AllocGroupItem (	DG_cuin,
											pItem->child_hhcu,
											pItem->date,
											pItem->hhci_hash,
											_cuhd_rec.hhcp_hash,
											pItem->doc_no,
											pItem->type,
											value,
											pItem->period	);

				_InsertGroupItem (	cudt_group,
									pItem,
									pGroupSet->_isGroupByChild);
			}
		}
	}

	/*--------------------------------------
	| Process each item in cudt_group,
	| moving corresponding invoice group to
	| a cuhd group which is created for each
	| child.
	--------------------------------------*/
	type = _GetType(DG_cuhd, *_cuhd_rec.type);
	cudt_item = cudt_group->_pFirstItem;
	while (cudt_item)
	{
		DGroup		*cuhd_group;
		DGroupItem	*cuhd_item;
		int			period;
	
		curr_child = cudt_item->child_hhcu;
		period = AgePeriod 
				( 
					"20A",
					_cuhd_rec.date_payment,
					_dbt_date,
					_dbt_date,
					_StmtGroupDaysAgeing,
					_true_age
				);

		cuhd_item = _AllocGroupItem (	DG_cuhd,
										cudt_item->child_hhcu,
										_cuhd_rec.date_payment,
										0L,
										_cuhd_rec.hhcp_hash,
										_cuhd_rec.receipt_no,
										type,
										0.0,
										period	);

		cuhd_group = _AllocGroup (cuhd_item);

		while (cudt_item &&
				(!pGroupSet->_isGroupByChild ||
				 curr_child == cudt_item->child_hhcu))
		{
			DGroup *pGroup;

			cuhd_item->value += cudt_item->value;

			/* Adjust gtotal[] only if total[] was updated */
            if (cudt_item->source == DG_cudt)
            {
            	int period = cudt_item->period != -1 ? cudt_item->period : 5;
				cuhd_group->gtotal[period] -= cudt_item->value;
			}

			pGroup = _RemoveInvoiceGroup (pGroupSet, cudt_item->hhci_hash);
			if (pGroup)
				_MergeGroups (pGroupSet, cuhd_group, pGroup);
			cudt_item = cudt_item->_pNextItem;
		}
		_InsertGroup (pGroupSet, cuhd_group);
	}

	_DeallocAllGroups (cudt_group);

	return TRUE;
}

static int
_ReadPayments (
 DGroupSet	*pGroupSet,
 long		hhcu_hash,
 long *		bad_hhcp,
 long *		bad_hhci)
{
	/*
	 *	Read in the payment transactions from cuhd/cudt
	 */
	int	err;

	for (err = find_hash (cuhd, &_cuhd_rec, EQUAL, "r", hhcu_hash);
		!err && _cuhd_rec.hhcu_hash == hhcu_hash;
		err = find_hash (cuhd, &_cuhd_rec, NEXT, "r", hhcu_hash))
	{
		/*
		 *	Read all payments, including fwd payments
		 *	'cos if by some odd circumstance, the group resolves
		 *	without the fwd payment and is deleted, we end up
		 *	with a dangling record referencing the fwd payment.
		 */
		if (!_ReadPaymentDetails (pGroupSet,
				_cuhd_rec.hhcp_hash, bad_hhcp, bad_hhci))
		{
			return FALSE;
		}
	}
	return TRUE;
}

/*=======================================
| Process brought forward debtors.
| Based on DPL db_stmtprn proc_bfwd().
| All groups are combined into one group.
=======================================*/
static void
_BringForward (
 	DGroupSet	*pGroupSet	)
{
	DGroupItem	*bfwItem, *pItem;
	DGroup		*bfwGroup, *pGroup;
	Money		bias_0mt, bias_1mt;

	Date    	last_date = pGroupSet->_last_stmt_date;
	Date		eom_date = MonthStart (_dbt_date) - 1;

	/* BFWD defaults to current!!   */
	bfwItem = _AllocGroupItem (	DG_null,
								0L,
								last_date + 1,
								0L,
								0L,
								"      ",
								DG_BFwd,
								0.0,
								0	);

	bfwGroup = _AllocGroup (bfwItem);

	bias_0mt = 0.0;
	bias_1mt = 0.0;

	while (pGroupSet->_pFirstGroup)
	{
		pGroup = pGroupSet->_pFirstGroup;
		pGroupSet->_pFirstGroup = pGroup->_pNextGroup;
		pItem = pGroup->_pFirstItem;
		while (pItem)
		{
			/*
			 *	We're gonna fudge the previous and current month's
			 *	ageing totals to cater for transactions occuring
			 *	in the time period between the close-off date and the
			 *	eom date.
			 */

			/*
			 *	   close			eom
			 *		 v				 v
			 *	-------------------------------------------------> timeline
			 *				^
			 *			pItem->date
			 *
			 *	In this case, we place the item into the current
			 *	month's age-bucket (by physically fudging the numbers)
			 */
			if (last_date < eom_date &&
				pItem->date > last_date && pItem -> date < eom_date)
			{
				bias_0mt += pItem->value;
				bias_1mt -= pItem->value;
			}

			/*
			 *	   					eom				close
			 *		 				 v				  v
			 *	-------------------------------------------------> timeline
			 *								  ^
			 *							 pItem->date
			 *
			 *	In this case, we place the item back into last
			 *	month's age-bucket (by physically fudging the numbers)
			 */
			if (last_date > eom_date && pItem->date > eom_date)
			{
				bias_0mt -= pItem->value;
				bias_1mt += pItem->value;
			}

			/*
			 *	Cull stuff prior to the given B/F date
			 */
			if (pItem->date <= last_date)
			{
				DGroupItem	*curItem;

				bfwItem->value += pItem->value;

				curItem = pItem;
				pItem = pItem->_pNextItem;

				/* delete until current item */
				curItem->_pNextItem = NULL;	/* break chain */
				_DeallocGroupItem (pGroup->_pFirstItem);
				pGroup->_pFirstItem = pItem;
			}
			else
				pItem = pItem->_pNextItem;
		}
		_MergeGroups (pGroupSet, bfwGroup, pGroup);
		_DeallocGroup (pGroup);
	}
	_InsertGroup (pGroupSet, bfwGroup);
	pGroupSet->total[0] -= bias_0mt;
	pGroupSet->total[1] -= bias_1mt;
	bfwGroup->gtotal[0] -= bias_0mt;
	bfwGroup->gtotal[1] -= bias_1mt;
}

/*===============================
| Remove ALL record(s) where
| the nett value is $0.00
| Based on DPL db_stmtprn proc_zero()
===============================*/
static void
_RemoveZeroValues (
 	DGroupSet	*pGroupSet	)
{
	DGroup		*pGroup, *prevGroup, *nextGroup;
	DGroupItem	*pItem, *prevItem, *nextItem;
	
	prevGroup = NULL;
	for (pGroup = pGroupSet->_pFirstGroup; pGroup; pGroup = nextGroup)
	{
		nextGroup = pGroup->_pNextGroup;
		prevItem = NULL;
		for (pItem = pGroup->_pFirstItem; pItem; pItem = nextItem)
		{
			nextItem = pItem->_pNextItem;
			if (!pItem->value)
			{
				if (prevItem)
					prevItem->_pNextItem = pItem->_pNextItem;
				else
					pGroup->_pFirstItem = pItem->_pNextItem;
				_DeallocGroupItem (pItem);
			}
			else
				prevItem = pItem;
		}
		if (!pGroup -> _pFirstItem)
		{
			/*
			 *	Group has no more attachements to it
			 */
			if (prevGroup)
				prevGroup->_pNextGroup = pGroup->_pNextGroup;
			else
				pGroupSet->_pFirstGroup = pGroup->_pNextGroup;
			_DeallocGroup (pGroup);
		}
		else
			prevGroup = pGroup;
	}
}

/*============================================================================
|	External interface
============================================================================*/

static int _isOpenStatementGroups = 0;

/*============================================================================
@func int | OpenStatementGroups |

	Opens cuin (invoices) and cuhd/cudt (payments, credits) tables.

@comm
	Use <f CloseStatementGroups>() to free memory and close tables.
@rdesc
	TRUE if successful.
@ex	Print statements: |

	open_db ();
	read_comm (comm_list, comm_no_fields, &comm_rec);
	ReadDetails ();

	dsp_screen ("PRINTING STATEMENTS", comm_rec.tco_no, comm_rec.tco_name);

	if (OpenStatementGroups (comm_rec.dbt_date))
	{	
		while (scanf ("%ld", &hhcu_hash) != EOF && hhcu_hash)
		{
			process (hhcu_hash);

		}
		CloseStatementGroups();
	}

	close_db ();

	(Example taken from db_stmtprn.c)
*/

int
OpenStatementGroups (
	Date comm_dbt_date )		/*@parm Current Customer Module Date.*/
{
	char	*sptr;

	assert (!_isOpenStatementGroups);
	_isOpenStatementGroups = TRUE;

	abc_alias (cudt, "cudt");		/* successive invocations shouldn't */
	abc_alias (cuhd, "cuhd");		/* be a problem at all */
	abc_alias (cuin2, "cuin");

	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhcp_hash");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcu_hash");
	open_rec (cuin2, cuin_list, CUIN_NO_FIELDS, "cuin_id_no2");

	abc_alias (cumr, "cumr");
	abc_alias (cuinho, "cuin");
	open_rec (cuinho, cuin_list, CUIN_NO_FIELDS, "cuin_ho_id");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");

	_dbt_date = comm_dbt_date;
	
	/*---------------------------------------------------------------
	| Check if ageing is by days overdue or as per standard ageing. |
	---------------------------------------------------------------*/
	sptr = chk_env ("DB_DAYS_AGEING");
	_StmtGroupDaysAgeing = sptr ? atoi (sptr) : 0;

	sptr = chk_env ("DB_TOTAL_AGE");
	_true_age = sptr ? (toupper (*sptr) == 'T') : FALSE;

	return TRUE;
}

/*============================================================================
@func int | CloseStatementGroups |

	Frees memory and closes tables.

@xref
	<f OpenStatementGroups>()
@rdesc
	TRUE if successful.
@ex	See <f OpenStatementGroups>() example. |
*/
int
CloseStatementGroups(void)
{
	assert (_isOpenStatementGroups);
	_isOpenStatementGroups = FALSE;

	while (_pFreeGroupList)
	{
		DGroup *tgroup = _pFreeGroupList;
		_pFreeGroupList = _pFreeGroupList->_pNextGroup;
		free (tgroup);
	}

	while (_pFreeGroupItemList)
	{
		DGroupItem *titem = _pFreeGroupItemList;
		_pFreeGroupItemList = _pFreeGroupItemList->_pNextItem;
		free (titem);
	}

	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (cuin2);
	abc_fclose (cumr);
	abc_fclose (cuinho);

	return TRUE;
}

/*============================================================================
@func int | LoadStatementGroups | Load the set of statement groups.

	Reads in all cuin (invoices) and cuhd/cudt (payments, credits) and
	resolve values relating to a debtor.

@comm
	Stores group and transaction item structures in memory.

	Call <f OpenStatementGroups>() to open files before using.

	Use <f FreeStatementGroups>() to release memory.
@rdesc
	TRUE if successful.
@ex	Print statement for head office debtor: |

	void
	print_stmt()
	{
		DGroup	*pGroup;
		double	bias0,
			bias1;
		int	first_time = TRUE;
		long	curr_child = 0L;

	    if (OPEN_ITEM)
	    {
	    	LoadStatementGroups (	&GroupSet,
	    							cumr_rec.cm_hhcu_hash,
	    							cof_date,
	    							DG_OPENITEM	);
	    }
	    else
	    {
	    	LoadStatementGroups (	&GroupSet,
	    							cumr_rec.cm_hhcu_hash,
	    							cof_date,
	    							DG_BFORWARD);
	    }

		for (pGroup = GetFirstGroup (&GroupSet);
			 pGroup != NULL;
			 pGroup = GetNextGroup (pGroup))
		{
			DGroupItem *pItem = GetFirstGroupItem (pGroup);

			if (pItem->child_hhcu != curr_child)
			{
				curr_child = pItem->child_hhcu;
				prnt_kid (curr_child);
				first_time = FALSE;
			}

			prnt_stmt (pGroup, &first_time);
		}

		if (!first_time)
			prnt_totals ();
			
		FreeStatementGroups (&GroupSet);
	}

	(Example taken from db_stmtprn.c)
*/

int
LoadStatementGroups (
 DGroupSet	*pGroupSet,		/*@parm The set of all groups for debtor.*/
 long		hhcu_hash,		/*@parm Hash of head office debtor.*/
 Date    	last_stmt_date,	/*@parm comr_stmt_date or esmr_stmt_date.*/
 int		options,  		/*@parm Customer Statement Group Options.*/
 FILE *		ferr)			/*@parm File for error output */
	/*
		Customer Statement Group flags:
		@flag DG_PURGE | Default Purge Transaction options.

			_DG_PURGE + _DG_GROUPBYPARENT + _DG_INCLUDEZEROS <nl>

		@flag DG_OPENITEM | Default Open Item Statement options.

			_DG_OPENITEM + _DG_GROUPBYCHILD + _DG_EXCLUDEZEROS <nl>

		@flag DG_BFORWARD | Default Brought Forward Statement options.

			_DG_BFORWARD + _DG_GROUPBYCHILD + _DG_EXCLUDEZEROS <nl>

		@flag _DG_INCLUDEZEROS | Include zero value transaction items.
		@flag _DG_EXCLUDEZEROS | Exclude zero value transaction items.
		@flag _DG_GROUPBYPARENT | Group transaction items by Head Office debtor.
		@flag _DG_GROUPBYCHILD | Group transaction items by Child debtor.
		@flag _DG_OPENITEM | Customer uses open item statements.
		@flag _DG_BFORWARD | Customer uses brought forward statements.
		@flag _DG_PURGE | Purge option selected
	*/
{
	long		bad_hhcp = 0,
				bad_hhci = 0;
	const char *errmsg = "Could not locate invoice for cuhd_hhcp=%ld, hhci=%ld\n";

	int	err;
	struct tag_cumrRecord	_cumr_rec;

	assert (_isOpenStatementGroups);

	memset (pGroupSet, 0, sizeof *pGroupSet);

	pGroupSet->_hhcu_hash = hhcu_hash;
	pGroupSet->_isIncludeZeros = (!(options & _DG_EXCLUDEZEROS));
	pGroupSet->_isGroupByChild = (!(options & _DG_GROUPBYPARENT));
	pGroupSet->_isOpenItem = (!(options & _DG_BFORWARD));
	pGroupSet->_isPurge = ((options & _DG_PURGE) != 0);
	pGroupSet->_last_stmt_date = last_stmt_date;

	_ReadInvoices (pGroupSet, hhcu_hash);
	if (!_ReadPayments (pGroupSet, hhcu_hash, &bad_hhcp, &bad_hhci))
	{
		if (ferr)
		{
			fprintf (ferr, errmsg, bad_hhcp, bad_hhci);
			fflush (ferr);
		}
		return FALSE;
	}

	/*
	 *	If we're a head-office, we need to read in child-debtors' payments
	 *	as well
	 */
	for (err = find_hash (cumr, &_cumr_rec, EQUAL, "r", hhcu_hash);
		!err && _cumr_rec.ho_dbt_hash == hhcu_hash;
		err = find_hash (cumr, &_cumr_rec, NEXT, "r", hhcu_hash))
	{
		if (!_ReadPayments (pGroupSet,
				_cumr_rec.hhcu_hash, &bad_hhcp, &bad_hhci))
		{
			if (ferr)
			{
				fprintf (ferr, errmsg, bad_hhcp, bad_hhci);
				fflush (ferr);
			}
			return FALSE;
		}
	}

	if (!pGroupSet->_isOpenItem)
		_BringForward (pGroupSet);

	if (!pGroupSet->_isIncludeZeros)
		_RemoveZeroValues (pGroupSet);

	/*---------------------
	| subsystem checks
	---------------------*/
#if 0
	{
		DGroup 	*pGroup;
		int 	period;
		Money	total [6];
			
		for (period = 0; period < 6; period++)
			total [period] = 0.00;
		for (pGroup = pGroupSet->_pFirstGroup; pGroup; pGroup = pGroup->_pNextGroup)
		{
			for (period = 0; period < 6; period++)
				total[period] += pGroup->gtotal[period];
		}
		for (period = 0; period < 6; period++)
			assert (!floor (fabs (total[period] - pGroupSet->total[period])));
	}
#endif

	return TRUE;
}

/*============================================================================
@func void | FreeStatementGroups | Free all statement groups in set.

	Release all set structures in memory allocated by
	<f LoadStatementGroups>().

@comm
	Released memory is placed on a free list.
@ex	See <f LoadStatementGroups>() example. |
*/

void
FreeStatementGroups (
 	DGroupSet	*pGroupSet	)	/*@parm The set of groups loaded by
 									<f LoadStatementGroups>().*/
{
	assert (_isOpenStatementGroups);

	_DeallocAllGroups (pGroupSet->_pFirstGroup);
	pGroupSet->_pFirstGroup = NULL;
	memset (pGroupSet, 0, sizeof *pGroupSet);
}

/*============================================================================
@func DGroup * | GetFirstGroup |
@xref
	<f GetNextGroup>()
@rdesc
	Pointer to first group in set.
@ex	See <f LoadStatementGroups>() example. |
*/

DGroup *
GetFirstGroup (
 	DGroupSet	*pGroupSet	)	/*@parm The set of groups loaded by
 									<f LoadStatementGroups>().*/
{
	assert (pGroupSet != NULL);

	return pGroupSet->_pFirstGroup;
}

/*============================================================================
@func DGroup * | GetNextGroup |
@comm
	Call <f GetFirstGroup>() to get a current group before using.
@rdesc
	Pointer to next group in set.
@ex	See <f LoadStatementGroups>() example. |
*/

DGroup *
GetNextGroup (
 	DGroup	*pGroup	)		/*@parm A group returned by
 								<f GetFirstGroup>() or <f GetNextGroup>().*/
{
	assert (pGroup != NULL);

	return pGroup->_pNextGroup;
}

/*============================================================================
@func DGroupItem * | GetFirstGroupItem |
@comm
	Call <f GetFirstGroup> to get a current group before using.
@xref
	<f GetNextGroupItem>()
@rdesc
	Pointer to first item in group.
@ex	Print statement group: |

	void
	prnt_stmt (
		DGroup	*pGroup,
		int	*first_time	)
	{
		DGroupItem	*pItem;
		char	strdate[9];
		double	db_value;

		db_value = DOLLARS (GetGroupValue (pGroup));

		if (*first_time)
		{
			page_no = 0;
			*first_time = FALSE;
		}

		for (pItem = GetFirstGroupItem (pGroup);
			 pItem != NULL;
			 pItem = GetNextGroupItem(pItem))
		{
			page_break();

			strcpy (strdate, DateToString (pItem->date));
			p_line (	(trans_type[pItem->type]._date) ? strdate : " ",
						pItem->type,
						pItem->doc_no,
						DOLLARS (pItem->value),
						DOLLARS (pItem->value),
						db_value,
						GetNextGroupItem (pItem) == NULL,
						pItem->period == -1	);
		}
	}

	(Example taken from db_stmtprn.c)
*/

DGroupItem *
GetFirstGroupItem (
 	DGroup	*pGroup	)		/*@parm A group returned by
 								<f GetFirstGroup>() or <f GetNextGroup>().*/
{
	assert (pGroup != NULL);

	return pGroup->_pFirstItem;
}

/*============================================================================
@func DGroupItem * | GetNextGroupItem |
@comm
	Call <f GetFirstGroupItem> to get a current group item before using.
@rdesc
	Pointer to next item in group.
@ex	See <f GetFirstGroupItem>() example. |
*/

DGroupItem *
GetNextGroupItem (
 	DGroupItem	*pGroupItem	)	/*@parm A group item returned by
 									<f GetFirstGroupItem>() or
 									<f GetNextGroupItem>().*/
{
	assert (pGroupItem != NULL);

	return pGroupItem->_pNextItem;
}

/*============================================================================
@func int | GetGroupCount |
@xref
	<f GetGroupValue>(),
	<f GetLastGroupItemDate>()
@rdesc
	The number of items in group.
@ex	Get number of transactions in group: |

	trn_cnt = GetGroupCount (pGroup);

	(Example taken from db_dpurge.c)
*/

int
GetGroupCount (
 	DGroup	*pGroup	)		/*@parm A group returned by
 								<f GetFirstGroup>() or <f GetNextGroup>().*/
{
	DGroupItem	*pItem;
	int			count = 0;

	assert (pGroup != NULL);
	
	for (pItem = pGroup->_pFirstItem; pItem; pItem = pItem->_pNextItem)
		count++;

	return count;
}

/*============================================================================
@func Money | GetGroupValue |
@xref
	<f GetGroupCount>(),
	<f GetLastGroupItemDate>()
@rdesc
	The sum value of all items in group.
@ex	Get dollar value of group: |

	grp_value = DOLLARS (GetGroupValue (pGroup));

	(Example taken from db_dpurge.c)
*/

Money
GetGroupValue (
 	DGroup	*pGroup	)		/*@parm A group returned by
 								<f GetFirstGroup>() or <f GetNextGroup>().*/
{
	DGroupItem	*pItem;
	Money		value = 0.0;

	assert (pGroup != NULL);

	for (pItem = pGroup->_pFirstItem; pItem; pItem = pItem->_pNextItem)
		value += pItem->value;

	return value;
}

/*============================================================================
@func Date | GetLastTransactionDate |
@xref
	<f GetGroupCount>(),
	<f GetGroupValue>()
@rdesc
	The date of the latest transaction in group.
@ex	Do not purge group if last transaction date was after purge_mon days
	before comm_dbt_date: |

	grp_value = DOLLARS (GetGroupValue (pGroup));
	trn_cnt = GetGroupCount (pGroup);
	if (GetLastTransactionDate (pGroup) >= (comm_rec.tdbt_date - purge_mon))
		grp_value = 1;

	if (!twodec (grp_value))
	{
		proc_purge (pGroup, trn_cnt);
	}

	(Example taken from db_dpurge.c)
*/

Date
GetLastTransactionDate (
 	DGroup	*pGroup	)		/*@parm A group returned by
 								<f GetFirstGroup>() or <f GetNextGroup>().*/
{
	DGroupItem	*pItem;
	Date		date = 0L;

	assert (pGroup != NULL);

	for (pItem = pGroup->_pFirstItem; pItem; pItem = pItem->_pNextItem)
		if (date < pItem->date)
			date = pItem->date;

	return date;
}
