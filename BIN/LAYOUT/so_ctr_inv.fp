#
#   so_ctr_inv Format file
#
################################################################################
#   
#   $Id: so_ctr_inv.fp,v 5.1 2001/08/06 23:51:07 scott Exp $
#   $Log: so_ctr_inv.fp,v $
#   Revision 5.1  2001/08/06 23:51:07  scott
#   RELEASE 5.0
#
#

options
{
    pitch = 12 
    pagelength = 48 

    pageheader-start = 3
    body-start = 23
    body-end = 36 
    pagetrailer-start = 37 
}

page-header
{
                                                                                                             
                                                         .comr_co_name:40 
                                                           .comr_co_adr1:40  
                                                              .comr_co_adr2:40 
                                                                 .comr_co_adr3:40
                                                                 
                                                              .eval_transaction_type .eval_reprint 
                                                              .eval_line                                                                          
                                                                                                                                    
       Customer No:   .cumr_dbt_no                                                                 
       Customer Name: .cumr_dbt_name:40                                                             Tax Invoice No.: .cohr_inv_no:8 
       Address:                                                     Delivery Address:                                              GST No.: .comr_gst_ird_no  
           .cumr_ch_adr1:40                 .cohr_dl_add1:40                  Invoice Date: .cohr_date_raised
           .cumr_ch_adr2:40                 .cohr_dl_add2:40                      Page No.: .pageno          
           .cumr_ch_adr3:40                 .cohr_dl_add3:40                     Reference: .cohr_cus_ord_ref:20  
         Terms:       .cohr_pay_terms:60 
===============================================================================================================================================================
|    Part No.      |               Description                | Quantity | Quantity |Quantity  |   Price      | UOM  |  %Disc   |   Disc     |     Total      |
|                  |                                          |  Ordered | Supplied |    B/O   |              |      |          |            |                |
===============================================================================================================================================================
}


body-header
{
}

body
{
| .inmr_item_no:16 | .coln_item_desc:40 | .coln_qty_org_ord:-8.2 | .coln_q_order:-8.2 | .eval_line_qty:-8.2 | .coln_sale_price:-12.2 | .inum_uom:4 | .coln_disc_pc:-8.2 | .coln_amt_disc:-10.2 |.eval_line_nett:-15.2 |
}

body-trailer
{
}
page-trailer
{
                                                                                                                     Continued
}

page-trailer-last
{
                                                                                                                                  Sub Total:  .eval_sub_total4:-15.2       
                                                                                                                                  .eval_gst:10     .eval_tax_gst:-12.2
                                                                                                                                  .cumr_curr_code:10  .eval_total4:-15.2                  
}
sons-body-coln
{
                   .sons_desc
}
sons-body-cohr
{
                   .sons_desc
}
misc-chg-body
{
                   .eval_misc_narrative:20                                                                                                             .eval_misc_value:-9.2
}
backorder-body
{
}

space-body
{
}

report-trailer
{

Prepared By                                                                                   Approved By

_______________________________                                                               _______________________________
}
