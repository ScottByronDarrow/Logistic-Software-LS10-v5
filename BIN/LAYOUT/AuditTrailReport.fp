#layout file for fptest
options
{
	pitch = 18 
	body-end = 60
	body-start = 0 
	pagelength = 63
}

body
{
+==========================================================================
Supplier Name : .supplier_name
Supplier Type : .supplier_type
Buyer Number  : .buyer_no
Item Number	  : .item_no
Item Quantity : .item_qty
Inv Date	  : .inv_date
Total Sales   : .sales
Total Cost    : .cost
+==========================================================================
}

alt-body
{
}

body-header
{
}

page-trailer
{
}

page-header
{
}

body-trailer
{

}

report-header
{
	Sales Analysis Audit Report

	.date
}

report-trailer
{
}

