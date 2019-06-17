#
#	po_cprint Format file
#
################################################################################
#	$Id: po_cprint.fp,v 5.0 2001/06/19 08:11:50 robert Exp $
#	$Log: po_cprint.fp,v $
#	Revision 5.0  2001/06/19 08:11:50  robert
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:33:03  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 3.6  2001/03/05 10:05:19  cha
#	Ascent specific report
#	
#	Revision 3.4  2001/02/21 03:49:17  scott
#	Updated to make code change to print correct quantity on returns.
#	Purchase orders are quantity order - receipts, returns should only
#	use quantity order.
#	
#	Revision 3.3  2001/02/20 05:16:34  scott
#	Updated to change .fp files
#	
#	Revision 3.2  2001/02/16 04:58:12  scott
#	Updated after testing
#	
#	Revision 3.1  2001/02/16 03:39:36  scott
#	New 'C' version that replaces C++ version that is not database independent
#	
#

options
{
	pitch = 12 
	pagelength = 66 

	pageheader-start = 0
	body-start = 10 
	body-end = 54 
	pagetrailer-start = 55 
}

page-header
{
                  P U R C H A S E   R E T U R N   F O R M
                  ---------------------------------------

Purchase Return No: .pohr_pur_ord_no       .pohr_date_raised                                                     Page .pageno

Supplier No:   .sumr_crd_no                                       RECEIVE FROM
Supplier Name: .sumr_crd_name:40     .comr_co_name:40  Phone:    
Address:       .sumr_adr1:40     .esmr_adr1:40  Fax:      
               .sumr_adr2:40     .esmr_adr2:40  Operator: .pohr_op_id
               .sumr_adr3:40     .esmr_adr3:40 
Fax No:        .sumr_fax_no 
===================================================================================================================================
| Line | Supplier Number  | Supplier Part No | Description                              | Quantity | Unit | Net Cost |  Due Date  |
|------|------------------|------------------|------------------------------------------|----------|------|----------|------------|
}

body-header
{
}

body
{
| .poln_line_no:4 | .inis_sup_part:16 | .inmr_item_no:16 | .inmr_description:40 | .eval_quantity:-8.2 | .inum_uom:4 | .eval_fob_fgn_cst:8.2 | .poln_due_date |
}

body-trailer
{
===================================================================================================================================
}
page-trailer
{
                                                                                 Continued
}

page-trailer-last
{
        .pohr_delin1 
        .pohr_delin2
        .pohr_delin3
}
instruction-body
{
}
inex-desc-body
{
|      |                  |                  | .inex_desc |          |      |          |            |
}
