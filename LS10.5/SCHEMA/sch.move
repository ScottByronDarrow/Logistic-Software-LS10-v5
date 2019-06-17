{======================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware
|======================================================================
| $Id: sch.move,v 5.0 2001/06/19 10:17:21 cha Exp $
|  Schema Name  : (sch.move)
|  Schema Desc  : (Movement file. )
|----------------------------------------------------------------------
| $Log: sch.move,v $
| Revision 5.0  2001/06/19 10:17:21  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:01:01  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/02/13 08:33:23  scott
| Updated for length change related to reference.
|
| Revision 3.1  2001/01/25 06:55:35  scott
| Updated to make all schema headers standard.
| Long overdue for some clean up action
|
|=====================================================================}
file	move

field	move_move_hash		type	serial				index	primary
field	move_co_no			type	char	2
field	move_br_no			type	char	2
field	move_wh_no			type	char	2
field	move_hhbr_hash		type	long
field	move_hhcc_hash		type	long
field	move_hhum_hash		type	long
field	move_date_tran		type	edate
field	move_type_tran		type	int
field	move_batch_no		type	char	7
field	move_class			type	char	1
field	move_category		type	char	11
field	move_ref1			type	char	15
field	move_ref2			type	char	15
field	move_qty			type	float
field	move_cost_price		type	money
field	move_sale_price		type	money
field	move_op_id          type	char	14
field	move_time_create    type	char	5
field	move_date_create    type	edate
end
{
              +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
              ! LOGISTIC SCHEMA COMMENTS SECTION. !
              +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	move_co_no					-	Company Number
	move_br_no					-	Branch Number
	move_wh_no					-	Warehouse Number
	move_hhbr_hash				-	Item Hash (link to inmr_hhbr_hash)
	move_hhcc_hash				-	Cost Centre Hash (link to ccmr_hhcc_hash)
	move_hhum_hash				-	UOM Hash (link to inum_hhum_hash)
	move_date_tran				-	Transaction Date.
	move_type_tran				-	Transaction type
								- 1		Stock Balance
								- 2		Stock Receipt
								- 3		Stock Isuue
								- 4		Stock Adjust
								- 5		Stock Purchase
								- 6		Invoice
								- 7		Credit
								- 8		Production Issue
								- 9		Stock Transfer
								- 10	Production Order
								- 11	Stock Write-off
								- 12	Drop Ship Purchase
								- 13	Drop Ship Sale 
	move_batch_no				-	Batch / Lot number
	move_class					-	Item Class
	move_category				-	Item Category
	move_ref1					-	Reference No 1
	move_ref2					-	Reference No 2
	move_qty					-	Quantity of transaction
	move_cost_price				-	Cost Price
	move_sale_price				-	Sale Price.
	move_op_id      			- 	Operator ID.
	move_time_create 			-	Time Created
	move_date_create 			-	Date Created.
}
