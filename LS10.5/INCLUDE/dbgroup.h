/*	$Id: dbgroup.h,v 5.0 2001/06/19 06:51:28 cha Exp $
@doc
@module dbgroup.h | Debtors Statement Grouping include file.
@comm
	Interface to statement grouping.

	-	Members with a leading underscore are for internal use only.
	
	-	Programmer must _NOT_ access these members - Code will break.
	
	-	Include file is identical for Version 7 and Version 9. 

*******************************************************************************
	$Log: dbgroup.h,v $
	Revision 5.0  2001/06/19 06:51:28  cha
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 00:59:23  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.0  2000/10/12 13:28:53  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 07:15:37  gerry
	Force revision no. to 2.0 - Rel-15072000
	
	Revision 1.3  1999/11/15 06:47:05  scott
	Updated for compile problems on AIX
	
	Revision 1.2  1999/09/13 06:13:09  alvin
	Check-in all ANSI-converted include files.
	
	Revision 1.1.1.1  1999/06/10 11:56:33  jonc
	Initial cutover from SCCS.
	
	Revision 2.3  1998/01/21 23:30:00  jonc
	Added support for error-logs
	
	Revision 2.2  1997/03/25 02:04:29  jonc
	Updated C++ comment to C

	Revision 2.1  1997/03/25 01:58:15  jonc
	Update to 9.2, no code changes

	Revision 1.2  1997/03/25 01:51:55  jonc
	Working version for Debtor's Grouping routines

*/
#ifndef	_dbgroup_h
#define	_dbgroup_h

/*
@enum This enumeration lists sources of transaction items.
	
	Use typedef <t DG_Source>.
*/
enum tag_DG_Source
{
	DG_null, 		/*@emem No source (eg brought forward value).*/
	DG_cuin, 		/*@emem Invoice record.*/
	DG_cuhd, 		/*@emem Payment record.*/
	DG_cudt			/*@emem Payment Detail record.*/
};

/*
@type DG_Source | Source of Debtors Statement Group Record. 

    See enum <t tag_DG_Source>.
*/
typedef enum tag_DG_Source DG_Source;

/*
@enum This enumeration lists Debtors Statement Group transaction types. 
	
	Use typedef <t DG_Type>.
*/
enum tag_DG_Type
{
	DG_Cheque, 		/*@emem Payment*/
	DG_JnlOne, 		/*@emem Payment Linking Journal.*/
	DG_Invoice, 	/*@emem Invoice.*/
	DG_Credit, 		/*@emem Invoice Credit.*/
	DG_JnlTwo, 		/*@emem Invoice Journal.*/
	DG_BFwd, 		/*@emem Brought Forward.*/
	DG_Invalid  	/*@emem Invalid transaction.*/
};
                                 
/*
@type DG_Type | Debtors Statement Group Transaction Type. 

    See enum <t tag_DG_Type>.
*/
typedef enum tag_DG_Type DG_Type;

/*
@struct tag_DGroupItem | Debtors Statement Group Item.
@comm 
	One node per cuin or cuhd transaction.
*/
struct tag_DGroupItem
{              
	DG_Source			source;		/*@field Record source.*/
	long				child_hhcu;	/*@field Hash of child debtor (cumr) record.*/
	Date				date;  		/*@field Date of transaction.*/
	long				hhci_hash;	/*@field Hash of invoice (cuin) record.*/
	long				hhcp_hash;	/*@field Hash od payment (cuhd) record.*/
	char				doc_no [7];	/*@field Document number (eg invoice or receipt number).*/
	DG_Type				type;		/*@field Transaction type.*/
	Money				value;		/*@field Value of transaction (Money type).*/
	int					period;		/*@field Period (0 = current, -1 = invalid).*/

/* private: */	
	struct tag_DGroupItem	*_pNextItem;/*@field Do not use.*/
};     

/*
@type DGroupItem | Debtors Statement Group Item.
	
	See struct <t tag_DGroupItem>.
*/
typedef struct tag_DGroupItem DGroupItem;

/*
@struct tag_DGroup | Debtors Statement Group.
@comm 
	One node per group of linked invoice/payment transactions.
*/
struct tag_DGroup
{
	Money				gtotal [6];		/*@field Aged group balances (Money type).*/
	
/* private: */	
	struct tag_DGroupItem	*_pFirstItem;	/*@field Do not use.*/
	struct tag_DGroup		*_pNextGroup;	/*@field Do not use.*/
};                          

/*
@type DGroup | Debtors Statement Group
	
	See struct <t tag_DGroup>.
*/
typedef struct tag_DGroup DGroup;


/*
@struct tag_DGroupSet | The set of all Debtors Statement Groups.
@comm 
	One node per Debtor.
*/
struct tag_DGroupSet
{
	Money				total [6];		/*@field Aged balances (Money type).*/
	
/* private: */	
	long					_hhcu_hash;   	/*@field Do not use.*/
	int						_isIncludeZeros;/*@field Do not use.*/
	int						_isGroupByChild;/*@field Do not use.*/
	int						_isOpenItem;	/*@field Do not use.*/
	int						_isPurge;		/*@field Do not use.*/
	Date    				_last_stmt_date;/*@field Do not use.*/
	struct tag_DGroup		*_pFirstGroup;	/*@field Do not use.*/
};      

/*
@type DGroupSet | The set of all Debtors Statement Groups.
	
	See struct <t tag_DGroupSet>.
*/
typedef struct tag_DGroupSet DGroupSet;

/* LoadStatementGroups() options */
#define _DG_INCLUDEZEROS	0x0001	
#define _DG_EXCLUDEZEROS	0x0002
#define _DG_GROUPBYPARENT	0x0004
#define _DG_GROUPBYCHILD	0x0008
#define _DG_OPENITEM		0x0010
#define _DG_BFORWARD		0x0020
#define _DG_PURGE			0x0040

#define DG_PURGE		(_DG_PURGE | _DG_GROUPBYPARENT | _DG_INCLUDEZEROS)
#define DG_OPENITEM 	(_DG_OPENITEM | _DG_GROUPBYCHILD | _DG_EXCLUDEZEROS)
#define DG_BFORWARD		(_DG_BFORWARD | _DG_GROUPBYCHILD | _DG_EXCLUDEZEROS)

/* Interface functions */

#endif	/*_dbgroup_h*/
