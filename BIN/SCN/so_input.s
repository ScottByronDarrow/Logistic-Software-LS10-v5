LEAVE		orderNo				6	2		DISPLAY 
LEAVE		area_code			LEAVE	LEAVE		REQUIRED 
LEAVE		sale_price			0	0		NOINPUT 
LEAVE		disc				0	0		NOINPUT 
((
line(1,0,1,132)
box(1,0,3,132,17)
line(1,1,8,131)
line(1,1,14,131)
line(1,1,17,131)
line(2,0,1,97)
line(2,0,20,132)
box(3,0,3,132,16)
line(3,0,1,132)
line(3,1,8,131)
line(3,1,11,131)
line(3,1,16,131)
line(3,1,21,131)
box(4,0,3,132,14)
line(4,0,1,132)
line(4,1,6,131)
line(4,1,11,131)
line(4,1,14,131)
line(4,1,21,131)
))
/*---------------------------------------------------------------------------
| Please add fields above the first )) if you require them to be in effect. |
---------------------------------------------------------------------------*/
LEAVE		debtor				4	2		NOEDIT 
LEAVE		curr_code			4	35		DISPLAY 
LEAVE		name				4	66		DISPLAY 
LEAVE		dp_no				5	2		REQUIRED 
LEAVE		dp_name				5	30		DISPLAY 
LEAVE		cust_type			5	66		DISPLAY 
LEAVE		headOfficeAccount	5	102		DISPLAY 
LEAVE		order_no			6	2		NOEDIT 
LEAVE		cus_ord_ref			6	66		REQUIRED 
LEAVE		cont_no				7	2		NOEDIT 
LEAVE		cont_desc			7	66		DISPLAY 
LEAVE		contact_name		9	2		REQUIRED 
LEAVE		contact_phone		9	66		REQUIRED 
LEAVE		cus_addr1			10	2		DISPLAY 
LEAVE		cus_addr2			11	2		DISPLAY 
LEAVE		cus_addr3			12	2		DISPLAY 
LEAVE		cus_addr4			13	2		DISPLAY 
LEAVE		del_name			10	66		INPUT 
LEAVE		del_addr1			11	66		INPUT 
LEAVE		del_addr2			12	66		INPUT 
LEAVE		del_addr3			13	66		INPUT 
LEAVE		dt_raised			15	2		REQUIRED 
LEAVE		freightRequiredFlag	15	66		REQUIRED 
LEAVE		dt_required			16	2		REQUIRED 
LEAVE		sched_ord			16	66		INPUT 
LEAVE		fix_exch			16	66		HIDE 
LEAVE		tax_code			17	2		HIDE 
LEAVE		tax_no				17	66		HIDE 
LEAVE		ord_type			18	2		REQUIRED 
LEAVE		pri_type			19	2		DISPLAY 
LEAVE		sman_code			18	66		REQUIRED 
LEAVE		sman_desc			18	91		DISPLAY 
LEAVE		area_code			19	66		REQUIRED 
LEAVE		area_desc			19	91		DISPLAY 
LEAVE		disc_over			20	2		HIDE 
LEAVE		prt_price			20	2		REQUIRED 
LEAVE		inp_total			20	66		REQUIRED 
LEAVE		line_no				0	0		DISPLAY 
LEAVE		hide				0	0		HIDE 
LEAVE		descr				0	0		NOINPUT 
LEAVE		sale_code			0	1		HIDE 
LEAVE		UOM					0	0		INPUT 
LEAVE		ord_ref				0	0		HIDE 
LEAVE		pack_size			0	0		HIDE 
LEAVE		qty_ord				0	0		REQUIRED 
LEAVE		qty_back			0	0		HIDE 
LEAVE		LL					0	0		REQUIRED 
LEAVE		cost_price			0	0		HIDE 
LEAVE		sale_price			0	0		REQUIRED 
LEAVE		disc				0	0		INPUT 
LEAVE		due_date			0	0		REQUIRED 
LEAVE		ser_no				0	0		HIDE 
LEAVE		del_no				0	0		INPUT 
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
LEAVE		freight				10	66		REQUIRED 
LEAVE		shipname			12	2		REQUIRED 
LEAVE		shipaddr1			13	2		REQUIRED 
LEAVE		shipaddr2			14	2		REQUIRED 
LEAVE		shipaddr3			15	2		REQUIRED 
LEAVE		ship_method			17	2		REQUIRED 
LEAVE		spcode1				18	2		REQUIRED 
LEAVE		spcode2				19	2		REQUIRED 
LEAVE		sos_ok				4	2		REQUIRED 
LEAVE		pay_term			5	2		REQUIRED 
LEAVE		sell_terms			7	2		REQUIRED 
LEAVE		sell_desc			8	2		DISPLAY 
LEAVE		insurance			9	2		REQUIRED 
LEAVE		ins_det				10	2		REQUIRED 
LEAVE		deposit				12	2		REQUIRED 
LEAVE		discount			13	2		REQUIRED 
LEAVE		other_1				15	2		REQUIRED 
LEAVE		other_2				16	2		REQUIRED 
LEAVE		other_3				17	2		REQUIRED 
