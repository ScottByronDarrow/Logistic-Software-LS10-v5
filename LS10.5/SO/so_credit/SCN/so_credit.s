((
box(1,0,3,132,16);
line(1,1,9,131);
line(1,1,12,131);
line(1,1,15,131);
line(1,1,18,131);
box(3,0,3,132,16);
line(3,1,6,131);
line(3,1,10,131);
line(3,1,15,131);
box(4,0,3,132,1);
))
/*---------------------------------------------------------------------------
| Please add fields above the first )) if you require them to be in effect. |
---------------------------------------------------------------------------*/
LEAVE	debtor				4	2		NOEDIT 
LEAVE	curr_code			4	35		DISPLAY 
LEAVE	name				4	66		DISPLAY 
LEAVE	cus_addr1			5	66		DISPLAY 
LEAVE	cus_addr2			6	66		DISPLAY 
LEAVE	cus_addr3			7	66		DISPLAY 
LEAVE	cus_addr4			8	66		DISPLAY 
LEAVE	ho_dbt				10	66		DISPLAY 
LEAVE	credit_no			5	2		NOEDIT 
LEAVE	app_inv_no			6	2		NOEDIT 
LEAVE	cus_ord_ref			7	2		REQUIRED 
LEAVE	restock_fee			8	2		REQUIRED 
LEAVE	dp_no				10	2		DISPLAY 
LEAVE	dp_name				10	25		DISPLAY 
LEAVE	cont_no				11	2		DISPLAY 
LEAVE	cont_desc			11	30		DISPLAY 
LEAVE	chargeToCustomer	11	66		REQUIRED 
LEAVE	chargeToName		11	96		DISPLAY 
LEAVE	chargeToHash		11	66		HIDE 
LEAVE	batch_no			13	2		REQUIRED 
LEAVE	ord_type			13	66		NOINPUT 
LEAVE	pri_type			14	2		NOINPUT 
LEAVE	date_raised			14	66		REQUIRED 
LEAVE	fix_exch			14	66		HIDE 
LEAVE	tax_code			15	2		HIDE 
LEAVE	tax_no				15	66		HIDE 
LEAVE	sale_code			16	2		REQUIRED 
LEAVE	sale_desc			16	66		DISPLAY 
LEAVE	area_code			17	2		REQUIRED 
LEAVE	area_desc			17	66		DISPLAY 
LEAVE	grn_no				19	2		REQUIRED 
LEAVE	prt_price			19	2		HIDE 
LEAVE	hide				0	0		DISPLAY 
LEAVE	descr				0	0		REQUIRED 
LEAVE	sman_code			0	1		HIDE 
LEAVE	pack_size			0	0		HIDE 
LEAVE	crd_type			0	0		REQUIRED 
LEAVE	UOM					0	0		REQUIRED 
LEAVE	qty_sup				0	0		HIDE 
LEAVE	qty_ret				0	0		REQUIRED 
LEAVE	LL					0	0		REQUIRED 
LEAVE	sale_price			0	0		REQUIRED 
LEAVE	disc				0	0		NOINPUT 
LEAVE	ser_no				0	0		HIDE 
LEAVE	extend				0	0		DISPLAY 
LEAVE	freight				4	2		REQUIRED 
LEAVE	cons_no				4	68		REQUIRED 
LEAVE	sos					5	2		REQUIRED 
LEAVE	pay_term			5	68		REQUIRED 
LEAVE	ship_method			7	2		REQUIRED 
LEAVE	spcode				8	2		REQUIRED 
LEAVE	spcode				9	2		REQUIRED 
LEAVE	sell_terms			11	2		REQUIRED 
LEAVE	sell_desc			11	72		DISPLAY 
LEAVE	insurance			12	2		REQUIRED 
LEAVE	ins_det				12	72		REQUIRED 
LEAVE	deposit				13	2		REQUIRED 
LEAVE	discount			13	72		REQUIRED 
LEAVE	other_1				14	2		REQUIRED 
LEAVE	other_2				14	36		REQUIRED 
LEAVE	other_3				14	72		REQUIRED 
LEAVE	shipname			16	2		REQUIRED 
LEAVE	shipaddr1			17	2		REQUIRED 
LEAVE	shipaddr2			18	2		REQUIRED 
LEAVE	shipaddr3			19	2		REQUIRED 
LEAVE	proof				4	20		REQUIRED 
