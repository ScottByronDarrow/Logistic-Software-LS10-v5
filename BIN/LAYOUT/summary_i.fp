#
#
#	Summary (by Item)
#

options
{
    pitch               = 12
    pagelength          = 65

    pageheader-start    = 10
    body-start          = 21
    body-end            = 60
    pagetrailer-start   = 61
}

page-header
{
						Company No.: .companyNumber - .companyName
							Branch: .branchNumber - .branchName
						Warehouse: .warehouseNumber - .warehouseName

						SUMMARY INVENTORY AGEING REPORT by ITEM
						FROM Item .startItem to .endItem
							As at .systemTime

===================================================================================================================================
|   ITEM NUMBER    |             ITEM DESCRIPTION             | .m1:2 - .m2:2 MONTHS | .m2:2 - .m3:2 MONTHS | .m3:2 - .m4:2 MONTHS |    .m4:2 ABOVE    |
===================================================================================================================================
}

body
{
| .inmr_item_no:16 | .inmr_description:40 | .itemQty1:14 | .itemQty2:14 | .itemQty3:14 | .itemQty4:14 |
}

body-trailer
{
===================================================================================================================================
}