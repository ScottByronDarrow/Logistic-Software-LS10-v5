{======================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware
|======================================================================
| $Id: sch.intr,v 5.0 2001/06/19 10:17:20 cha Exp $
|  Schema Name  : (sch.intr)
|  Schema Desc  : (Inventory Transactions File. )
|----------------------------------------------------------------------
| $Log: sch.intr,v $
| Revision 5.0  2001/06/19 10:17:20  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 01:00:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/02/07 00:21:31  scott
| Updated to remove sch.ctsi as should not be standard
| Updated to change intr and inaf reference fields to be 15 chars instead of 10
|
| Revision 3.1  2001/01/25 06:55:33  scott
| Updated to make all schema headers standard.
| Long overdue for some clean up action
|
|=====================================================================}
file	intr

field	intr_co_no				type	char	2
field	intr_br_no				type	char	2
field	intr_hhbr_hash			type	long			index	dups
field	intr_hhcc_hash			type	long
field	intr_hhum_hash			type	long
field	intr_type				type	integer
field	intr_date				type	edate
field	intr_batch_no			type	char	7
field	intr_ref1				type	char	15
field	intr_ref2				type	char	15
field	intr_qty				type	float
field	intr_cost_price			type	money
field	intr_sale_price			type	money
field	intr_stat_flag			type	char	1
field	intr_id_no				type	comp
			intr_co_no,
			intr_br_no										index	dups
field	intr_id_no2				type	comp
			intr_hhbr_hash,
			intr_date										index	dups

end

{
              +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
              ! LOGISTIC SCHEMA COMMENTS SECTION. !
              +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
	intr_co_no			-	Company Number.
	intr_br_no			-	Branch Number.
	intr_hhbr_hash		-	Link to product master file (inmr_hhbr_hash)
	intr_hhcc_hash		-	Link to Warehouse master file (ccmr_hhcc_hash)
	intr_hhum_hash		-	Link to Unit Of Measure file (inum_hhum_hash)
	intr_type			- 1		Stock Balance
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
	intr_date			-	Transaction Date.
	intr_batch_no		-	Batch Number.
	intr_ref1			-	Transaction Ref #1.
	intr_ref2			-	Transaction Ref #2.
	intr_qty			-	Quantity 
	intr_cost_price		-	Cost price.
	intr_sale_price		-	Sale Price.
	intr_stat_flag		-	Status Flag.
}
