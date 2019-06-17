((
line(1,0,1,132)
box(1,0,3,132,17)
line(1,1,8,131)
line(1,1,13,131)
line(1,1,16,131)
line(1,1,19,131)
line(2,0,1,97)
line(2,0,20,132)
box(3,0,3,132,16)
line(3,0,1,132)
line(3,1,8,131)
line(3,1,11,131)
line(3,1,16,131)
line(3,1,21,131)
box(4,0,3,132,13)
line(4,0,1,132)
line(4,1,5,131)
line(4,1,8,131)
line(4,1,10,131)
line(4,1,13,131)
line(4,1,21,131)
))
/*---------------------------------------------------------------------------
| Please add fields above the first )) if you require them to be in effect. |
---------------------------------------------------------------------------*/
LEAVE		debtor				4	2		NOEDIT
LEAVE		curr_code			4	35		DISPLAY
LEAVE		name				4	66		DISPLAY
LEAVE		invoice_no			5	2		NOEDIT
LEAVE		cus_ord_ref			5	66		NOINPUT
LEAVE		dp_no				6	2		DISPLAY
LEAVE		dp_name				6	25		DISPLAY
LEAVE		headOfficeAccount	6	66		DISPLAY
LEAVE		cont_no				7	2		NOEDIT
LEAVE		cont_desc			7	30		DISPLAY
LEAVE		chargeToCustomer	7	66		REQUIRED
LEAVE		chargeToName		7	96		DISPLAY
LEAVE		chargeToHash		7	66		HIDE
LEAVE		cus_addr1			9	2		DISPLAY
LEAVE		cus_addr2			10	2		DISPLAY
LEAVE		cus_addr3			11	2		DISPLAY
LEAVE		cus_addr4			12	2		DISPLAY
LEAVE		del_name			9	66		INPUT
LEAVE		del_addr1			10	66		INPUT
LEAVE		del_addr2			11	66		INPUT
LEAVE		del_addr3			12	66		INPUT
LEAVE		batch_no			14	2		REQUIRED
LEAVE		ord_type			14	66		DISPLAY
LEAVE		pri_type			15	2		DISPLAY
LEAVE		date_raised			15	66		NOINPUT
LEAVE		fix_exch			15	66		HIDE
LEAVE		tax_code			16	2		HIDE
LEAVE		tax_no				16	66		HIDE
LEAVE		sale_code			17	2		NOINPUT
LEAVE		sman_desc			17	66		DISPLAY
LEAVE		area_code			18	2		NOINPUT
LEAVE		area_desc			18	66		DISPLAY
LEAVE		disc_over			20	2		HIDE
LEAVE		prt_price			20	2		NOINPUT
LEAVE		inp_total			20	66		NOINPUT
LEAVE		hide				0	0		DISPLAY
LEAVE		descr				0	0		REQUIRED
LEAVE		sman_code			0	1		HIDE
LEAVE		ord_ref				0	0		HIDE
LEAVE		pack_size			0	0		HIDE
LEAVE		UOM					0	0		REQUIRED
LEAVE		qty_ord				0	0		HIDE
LEAVE		qty_sup				0	0		REQUIRED
LEAVE		cost_price			0	0		HIDE
LEAVE		sale_price			0	0		REQUIRED
LEAVE		disc				0	0		REQUIRED
LEAVE		ser_no				0	0		HIDE
LEAVE		LL					0	0		REQUIRED
LEAVE		extend				0	0		DISPLAY
LEAVE		carrierCode			4	2		REQUIRED
LEAVE		carr_desc			4	66		DISPLAY
LEAVE		deliveryZoneCode	5	2		REQUIRED
LEAVE		deliveryZoneDesc	5	66		DISPLAY
LEAVE		deliveryRequired	6	2		REQUIRED
LEAVE		deliveryDate		6	66		DISPLAY
LEAVE		cons_no				7	2		REQUIRED
LEAVE		no_cartons			7	66		REQUIRED
LEAVE		est_freight			9	2		DISPLAY
LEAVE		tot_kg				9	66		DISPLAY
LEAVE		frei_req			10	2		REQUIRED
LEAVE		freight				10	66		REQUIRED
LEAVE		shipname			12	2		REQUIRED
LEAVE		shipaddr1			13	2		REQUIRED
LEAVE		shipaddr2			14	2		REQUIRED
LEAVE		shipaddr3			15	2		REQUIRED
LEAVE		ship_method			17	2		REQUIRED
LEAVE		spcode1				18	2		REQUIRED
LEAVE		spcode2				19	2		REQUIRED
LEAVE		pay_term			4	2		REQUIRED
LEAVE		sell_terms			6	2		REQUIRED
LEAVE		sell_desc			7	2		DISPLAY
LEAVE		insurance			9	2		REQUIRED
LEAVE		ins_det				9	2		REQUIRED
LEAVE		deposit				11	2		REQUIRED
LEAVE		discount			12	2		REQUIRED
LEAVE		other_1				14	2		REQUIRED
LEAVE		other_2				15	2		REQUIRED
LEAVE		other_3				16	2		REQUIRED
LEAVE		proof				4	20		REQUIRED
