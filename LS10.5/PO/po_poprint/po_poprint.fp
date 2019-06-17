#
#   po_poprint Format file
#
################################################################################
#   $Id: po_poprint.fp,v 5.0 2001/06/19 08:11:50 robert Exp $
#   $Log: po_poprint.fp,v $
#   Revision 5.0  2001/06/19 08:11:50  robert
#   LS10-5.0 New Release as of 19 JUNE 2001
#
#   Revision 4.0  2001/03/09 02:33:03  scott
#   LS10-4.0 New Release as at 10th March 2001
#
#   Revision 3.4  2001/03/05 10:04:36  cha
#   Check in for proper compilation
#
#   Revision 3.3  2001/02/20 05:16:35  scott
#   Updated to change .fp files
#   
#   Revision 3.2  2001/02/16 04:58:12  scott
#   Updated after testing
#   
#   Revision 3.1  2001/02/16 03:39:36  scott
#   New 'C' version that replaces C++ version that is not database independent
#   
#

options
{
    pitch = 12 
    pagelength = 40

    pageheader-start = 0
    body-start = 25
    body-end = 35
    
}

page-header
{
         
                                                                     .comr_co_name:40 
                                                                        .comr_co_adr1:40  
                                                                          .comr_co_adr2:40 
                                                                            .comr_co_adr3:40
     
                                                                     P U R C H A S E  O R D E R  
                                                                     ---------------------------
   
        Order Number: .pohr_pur_ord_no:19               PO Date: .pohr_date_raised:10                                              Page .pageno
        Supplier No:   .sumr_crd_no:6                           INVOICE TO:                      DELIVER TO
        Supplier Name: .sumr_crd_name:30   .comr_co_name:30   .esmr_est_name:30  Phone: (65) 8622188
        Address:       .sumr_adr1:30   .comr_co_adr1:30   .esmr_adr1:30  Fax:   (65) 8621800
                       .sumr_adr2:30   .comr_co_adr2:30   .esmr_adr2:30  Ryder-Ascent Contact Person:
                       .sumr_adr3:30   .comr_co_adr3:30   .esmr_adr3:30        .pohr_req_usr:30                        
                                                          Attn:      
                                      
        Credit Terms:   .pohr_term_order:4 - .cred_term:40
        Shipping Terms: .pohr_sup_term_pay:30                                      
        
     ========================================================================================================================================================
     | Line |    Part No        |                Description               |   Quantity   |  UOM   |   Unit     |   Net      |    Extended     |  Delivery  |
     |      |                   |                                          |              |        |   Cost     |   Cost     |      Cost       |    Date    |
     |------|-------------------|------------------------------------------|--------------|--------|------------|------------|-----------------|------------|
}

body-header
{
}

body
{
     | .poln_line_no:4 | .inmr_item_no:16  | .inmr_description:40 |  .eval_quantity:-10.2  | .inum_uom:6 | .eval_fob_line_cost:-10.2 | .eval_fob_fgn_cst:-10.2 | .eval_extended:-15.2 | .poln_due_date |
     |      |                   | .inmr_description2:40 |              |        |            |            |                 |            |
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
                                                                                                                               ________________
                                                                                                           Total Amount: .sumr_curr_code:4  .eval_total_amt:-15.2
         Standard Instruction                                                                                                  ================
                .pohr_stdin1 
                .pohr_stdin2
                .pohr_stdin3
        
         Shipping Instruction
                .pohr_delin1 
                .pohr_delin2
                .pohr_delin3
        
        
        
         __ Chargeable
         __ Not Chargeable
     

         Prepared by:                                Approved by:                                   Vendor Acknowledgment:

         _________________________________________       _________________________________________       _________________________________________
          .pohr_req_usr:40                                                        .sumr_crd_name:30
   
}
instruction-body
{
}
inex-desc-body
{

}
