#
#
#	Detail Stock Valuation
#

options
{
    pitch               = 12
    pagelength          = 51

    pageheader-start    =  2
    body-start          = 10
    body-end            = 50
    pagetrailer-start   = 51
}

page-header
{   
   Page  .pageno  User(.user)										.prog_name
   .pr_no
					COMPANY DETAILED STOCK VALIDATION REPORT
						 .mode_string
						 .coname
						   	
 				              AS AT : .sys_time						
===================================================================================================================================================
|    Item Number     | 		I T E M    D E S C R I P T I O N	|    QUANTITY    |     EXTENDED   |   STANDARD PRICE  |  EXTENDED PRICE   |
==================================================================================================================================================|
}

body
{
|  .itemno:16  |    .description:40	|   .qty:-12.2 | .xtend:-14.2 |  .standard:-16.2 |  .extstand:-16.2 | .symbol
}
