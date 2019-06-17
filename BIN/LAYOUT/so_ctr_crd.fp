#
#	so_ctr_crd Format file
#
################################################################################
#	
#	$Id: so_ctr_crd.fp,v 5.1 2001/08/06 23:51:07 scott Exp $
#	$Log: so_ctr_crd.fp,v $
#	Revision 5.1  2001/08/06 23:51:07  scott
#	RELEASE 5.0
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
                                                                                                               Printing Date: .cohr_date_raised
                                                        .comr_co_name                                                                                                       

                                                        .eval_transaction_type
                                                                                                      .eval_transaction_no:15
                                                                                                      .cohr_inv_no:8 .cohr_ps_print_no Page .pageno
                                                                                                      GST NO: .comr_gst_ird_no

                   Customer Address:                             Delivery Address:
                   .cumr_dbt_name:40      .cohr_dl_name:40       Credit  Date: .cohr_date_raised 
                   .cumr_ch_adr1:40      .cohr_dl_add1       Customer No:  .cumr_dbt_no
                   .cumr_ch_adr2:40      .cohr_dl_add2       Reference:    .cohr_cus_ord_ref:20                 
                   .cumr_ch_adr3:40      .cohr_dl_add3 


  Stock                                                     Quantity Quantity Quantity
  .eval_col_title:5                     Item Description                Ordered  Supplied BackOrdered Price UOM   Disc    %Disc      Total

}

body-header
{
}

body
{
  .inmr_item_no:16 .coln_item_desc:40 .coln_qty_org_ord:-8.2 .coln_q_order:-8.2 .eval_line_qty:-8.2 .coln_sale_price:-8.2 .inum_uom:4 .coln_disc_pc:-5.2 .coln_amt_disc:-8.2  .eval_line_nett:-10.2 
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
  .cohr_din_1:40                                                              Sub Total        .eval_sub_total4:-8.2       
  .cohr_din_2:40                                                              .eval_gst:10       .eval_tax_gst:-8.2
  .cohr_din_3:40                                                              .cumr_curr_code:10       .eval_total4:-8.2
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
                   .eval_misc_narrative:20                                                                               .eval_misc_value:-9.2
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
