((
box(1,0,3,132,16);
line(1,1,9,131);
line(1,1,12,131);
line(1,1,15,131);
line(1,1,18,131);
box(3,0,3,132,16)
line(3,0,1,132)
line(3,1,8,131)
line(3,1,11,131)
line(3,1,16,131)
line(3,1,21,131)
))
/*---------------------------------------------------------------------------
| Please add fields above the first )) if you require them to be in effect. |
---------------------------------------------------------------------------*/
LEAVE		debtor				4	2		NOEDIT 
LEAVE		curr_code			4	35		DISPLAY 
LEAVE		name				4	66		DISPLAY 
LEAVE		cus_addr1			5	66		DISPLAY 
LEAVE		cus_addr2			6	66		DISPLAY 
LEAVE		cus_addr3			7	66		DISPLAY 
LEAVE		cus_addr4			8	66		DISPLAY 
LEAVE		headOfficeAccount	10	66		DISPLAY 
LEAVE		ccn_no				5	2		NOEDIT 
LEAVE		app_inv_no			6	2		NOEDIT 
LEAVE		cus_ord_ref			7	2		REQUIRED 
LEAVE		restock_fee			8	2		REQUIRED 
LEAVE		dp_no				10	2		DISPLAY 
LEAVE		dp_name				10	25		DISPLAY 
LEAVE		cont_no				11	2		DISPLAY 
LEAVE		cont_desc			11	30		DISPLAY 
LEAVE		chargeToCustomer	11	66		REQUIRED 
LEAVE		chargeToName		11	96		DISPLAY 
LEAVE		chargeToHash		11	66		HIDE 
LEAVE		batch_no			13	2		REQUIRED 
LEAVE		ord_type			13	66		NOINPUT 
LEAVE		pri_type			14	2		NOINPUT 
LEAVE		date_raised			14	66		REQUIRED 
LEAVE		fix_exch			14	66		HIDE 
LEAVE		tax_code			15	2		HIDE 
LEAVE		tax_no				15	66		HIDE 
LEAVE		sale_code			16	2		REQUIRED 
LEAVE		sale_desc			16	66		DISPLAY 
LEAVE		area_code			17	2		REQUIRED 
LEAVE		area_desc			17	66		DISPLAY 
LEAVE		grn_no				19	2		REQUIRED 
LEAVE		prt_price			19	2		HIDE 
LEAVE		hide				0	0		DISPLAY 
LEAVE		descr				0	0		REQUIRED 
LEAVE		sman_code			0	1		HIDE 
LEAVE		pack_size			0	0		HIDE 
LEAVE		crd_type			0	0		REQUIRED 
LEAVE		uom					0	0		REQUIRED 
LEAVE		qty_sup				0	0		HIDE 
LEAVE		qty_ret				0	0		REQUIRED 
LEAVE		LL					0	0		REQUIRED 
LEAVE		sale_price			0	0		REQUIRED 
LEAVE		disc				0	0		NOINPUT 
LEAVE		ser_no				0	0		HIDE 
LEAVE		extend				0	0		DISPLAY 
LEAVE		carrierCode			4	2		REQUIRED 
LEAVE		carr_desc			4	66		DISPLAY 
LEAVE		deliveryZoneCode	5	2		REQUIRED 
LEAVE		deliveryZoneDesc	5	66		DISPLAY 
LEAVE		deliveryRequired	6	2		REQUIRED 
LEAVE		deliveryDate		6	66		DISPLAY 
LEAVE		cons_no				7	2		REQUIRED 
LEAVE		no_cartons			7	66		REQUIRED 
LEAVE		sos					7	98		REQUIRED 
LEAVE		est_freight			9	2		DISPLAY 
LEAVE		tot_kg				9	66		DISPLAY 
LEAVE		frei_req			10	2		REQUIRED 
LEAVE		freight				10	66		REQUIRED 
LEAVE		shipname			12	2		REQUIRED 
LEAVE		shipaddr1			13	2		REQUIRED 
LEAVE		shipaddr2			14	2		REQUIRED 
LEAVE		shipaddr3			15	2		REQUIRED 
LEAVE		spcode1				18	2		REQUIRED 
LEAVE		spcode2				19	2		REQUIRED 
LEAVE		proof				4	20		REQUIRED 
