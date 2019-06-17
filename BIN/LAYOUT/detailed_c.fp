#
#
#	Detailed (by Category)
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

						DETAILED INVENTORY AGEING REPORT by CATEGORY
						FROM Category .startCategory to .endCategory
							As At .systemTime

=========================================================================================================================================
|     LOCATION     |               LOT NUMBER                 | UOM | .m1:2 - .m2:2 MONTHS | .m2:2 - .m3:2 MONTHS | .m3:2 - .m4:2 MONTHS |    .m4:2 ABOVE    |
=========================================================================================================================================
}

body-category-header_1
{
| CATEGORY: .excf_cat_no:11                                                                                                                 |                                         
}

body-item-header
{
| .inmr_item_no:16 | .inmr_description:40 |     |                |                |                |                |
|                  |                                          |     |                |                |                |                |
}

body-unit-item
{
| .inlo_location:10       |   .inlo_lot_no:7                                | .uom:4| .itemUnit1:14 | .itemUnit2:14 | .itemUnit3:14 | .itemUnit4:14 |
}

body
{
| TOTAL PER ITEM                                              | .uom:4| .itemQty1:14 | .itemQty2:14 | .itemQty3:14 | .itemQty4:14 |
-----------------------------------------------------------------------------------------------------------------------------------------
}
