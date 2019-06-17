#	$Id: db_stmtprn.fp,v 5.0 2001/06/19 08:05:30 robert Exp $
#
#	Example statement format
#	for use with the generic statement print program.
#
################################################################################
#	$Log: db_stmtprn.fp,v $
#	Revision 5.0  2001/06/19 08:05:30  robert
#	LS10-5.0 New Release as of 19 JUNE 2001
#	
#	Revision 4.0  2001/03/09 02:26:01  scott
#	LS10-4.0 New Release as at 10th March 2001
#	
#	Revision 3.0  2000/10/10 12:14:17  gerry
#	Revision No. 3 Start
#	<after Rel-10102000>
#	
#	Revision 2.0  2000/07/15 08:53:04  gerry
#	Forced Revision No. Start 2.0 Rel-15072000
#	
#	Revision 1.1  1999/07/16 00:19:30  jonc
#	Adopted Pinnacle V10 db_stmtprn using format-p.
#	

options
{
    pitch               = 12
    pagelength          = 51

    pageheader-start    = 10
    body-start          = 21
    body-end            = 36
    pagetrailer-start   = 42
}

page-header
{
     .cumr_dbt_no      .comr_dbt_date  .pageno:-2             .cumr_dbt_no                 .comr_dbt_date               .pageno:-2



 .cumr_dbt_name:30        .cumr_dbt_name
 .cumr_ch_adr1:30        .cumr_ch_adr1
 .cumr_ch_adr2:30        .cumr_ch_adr2
 .cumr_ch_adr3:30        .cumr_ch_adr3
 .cumr_ch_adr4:30        .cumr_ch_adr4
}

page-trailer
{
                                                                                     Continued
}

page-trailer-last
{
                 .eval_statement_period_0_amt_abs:-11 .eval_statement_period_0_credit_marker       .eval_statement_overdue_amt_abs:-11 .eval_statement_overdue_credit_marker .eval_statement_period_0_amt_abs:-11 .eval_statement_period_0_credit_marker               .eval_statement_due_amt_abs:-11 .eval_statement_due_amt_credit_marker

                 .eval_statement_period_1_amt_abs:-11 .eval_statement_period_1_credit_marker                                              .cumr_dbt_name:20
                                                                             .cumr_dbt_no
                 .eval_statement_period_2_amt_abs:-11 .eval_statement_period_2_credit_marker                                              .comr_dbt_date
                                                    .eval_statement_due_amt_abs:-11 .eval_statement_due_amt_credit_marker           .pageno:-2
                 .eval_statement_period_3_4_amt_abs:-11 .eval_statement_period_3_4_credit_marker
}

body
{
.eval_line_date .eval_line_tran_type_3 .eval_line_reference:8 .eval_line_amount_abs:-8 .eval_line_credit_marker    .eval_line_date .eval_line_forward_marker  .eval_line_tran_type:7 .eval_line_reference:10 .eval_line_amount_debit:-11 .eval_line_amount_credit:-11 .eval_line_credit_marker
}

body-trailer
{



                                      Forward Invoices Not Yet Due                      .eval_statement_period_fwd_amt
                                      .strm_mesg_tx1:51
                                      .strm_mesg_tx2:51
                                      .strm_mesg_tx3:51
}

################################################################################
#   Child Debtor header & trailers
#
#   These need to be defined, although they may be empty
#
child-debtor-body-header
{
}

child-debtor-body-trailer
{
}

################################################################################
#
#   The report trailer just lists the total
#   number of statements printed and the amount involved
#
report-trailer
{






























    Statements printed  = .eval_print_run_count
    Run total           = .eval_print_run_value
}
