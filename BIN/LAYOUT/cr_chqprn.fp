#
#	cr_chqprn Format file
#
################################################################################
#   $Id: cr_chqprn.fp,v 1.1 2000/04/27 21:40:53 cam Exp $
#   $Log: cr_chqprn.fp,v $
#   Revision 1.1  2000/04/27 21:40:53  cam
#   Added Sunlei's mods to the repository.
#
#   Revision 1.4  2000/04/02 10:39:32  ana
#   (02/04/2000) SC2712-16185 Corrected cr_chqprn.fp for pagebreak.
#
#   Revision 1.3  2000/02/24 03:15:49  ana
#   (24/02/2000) Changed pagelength to 66.  51 was never followed. Just placed 66 to declare that it should be 66.
#
#   Revision 1.2  2000/02/23 05:22:35  ana
#   (23/02/2000) SC2459/15930 Corrected printing of the next page.
#
#   Revision 1.1.1.1  1999/08/11 03:06:56  nz
#   DPA v9.10 start
#
#   Revision 1.1.1.1  1999/07/05 23:44:53  sunlei
#   Douglas Australia Ininial Revision
#

options
{
	pitch = 12 
	pagelength = 66 

	pageheader-start = 0
	body-start = 2 
	body-end = 23 
	pagetrailer-start = 24 
}

page-header
{
}

body-header
{
}

body
{
  .eval_transaction_type:8   .eval_invoice_date1:10    .eval_invoice_number1:16 .eval_gross_amount:-12    .eval_amount_paid1:-12 
}

body-trailer
{
}
page-trailer
{

                                                                                CONTINUED..









CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED
CANCELLED          CANCELLED          CANCELLED          CANCELLED          CANCELLED










}

page-trailer-last
{
                                         .eval_cheque_amount:-40



                                                                          .eval_cheque_date




                  .eval_amount_words



            .sumr_crd_name:40                   .eval_cheque_amount 
            .sumr_adr1:40
            .sumr_adr2:40              
            .sumr_adr3:40 
}

