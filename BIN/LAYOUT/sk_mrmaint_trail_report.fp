#
#
#	Audit Report on Stock Master Maintenance
#

options
{
    pitch               = 12
    pagelength          = 65

    pageheader-start    = 10
    body-start          = 25
    body-end            = 60
    pagetrailer-start   = 61
}

page-header
{
						Company: .companyNumber - .companyName
				 		Branch: .branchNumber - .branchName
				 
						STOCK MASTER TRAIL REPORT
			
						As At .systemTime
			
		
			
===========================================================================================================
| Item Number : .itemNumber:16                                                                          |
|													  |
===========================================================================================================
|      DETAILS       |                  OLD                     |                   NEW                   |  
===========================================================================================================
}

body
{
| .details:18 | .oldvalue:40 | .newvalue:40|
}

body-trailer
{
===========================================================================================================
}
