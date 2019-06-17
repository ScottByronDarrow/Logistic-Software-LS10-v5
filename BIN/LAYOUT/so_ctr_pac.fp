#
#	so_ctr_pac Format file
#
################################################################################
#
#	$Id: so_ctr_pac.fp,v 5.1 2001/08/06 23:51:08 scott Exp $
#	$Log: so_ctr_pac.fp,v $
#	Revision 5.1  2001/08/06 23:51:08  scott
#	RELEASE 5.0
#	
#

options
{
	pitch = 12 
	pagelength = 51 

	pageheader-start = 6
	body-start = 17 
	body-end = 40 
	pagetrailer-start = 43 
}

page-header
{
                                                P A C K I N G   S L I P

                                                                                                    .eval_reprint
  Customer Address:                             Delivery Address:                                   .comr_inv_date
  .cumr_dbt_name:40      .cohr_dl_name:40            PACKING SLIP  .cohr_inv_no
  .cumr_ch_adr1:40      .cohr_dl_add1:40            STORES ORDER  .sohr_order_no
  .cumr_ch_adr2:40      .cohr_dl_add2:40            .sohr_dt_raised(order) 
  .cumr_ch_adr3:40      .cohr_dl_add3:40            .pageno 

       Customer No: .cumr_dbt_no:20 Reference: .cohr_cus_ord_ref:20   Delivery Instruction: .cohr_din_1:30

  Quantity              Item Description                         Item No              Unit   Outer Discount
                                                                                      Cost    Size

}

body-header
{
}

body
{
.eval_q_order_total:-10.2              .coln_item_desc:40 .inmr_item_no:16 .eval_sale_price:-8.2 .inmr_outer_size:-7.2 .eval_discount:-7.2 
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
                                            .cohr_cons_no:16
                                            .cohr_pay_terms:60
                                            .cohr_din_1:60
                                            .cohr_din_2:60
}
backorder-body
{
.eval_backorder_string1:20    .eval_backorder_quantity:8.2 .eval_backorder_string2:18              .eval_backorder_item_no:16 .eval_backorder_sale_price:-8.2 .eval_backorder_outer_size:-7.2 .eval_backorder_discount:-7.2 
}
sons-body
{
                        .sons_desc
}

space-body
{
}

misc-chg-body
{
}
