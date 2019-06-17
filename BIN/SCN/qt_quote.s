Action ( A C L M S )	action				4	22		NOEDIT 
Quote Number.			n_quote				5	22		REQUIRED 
Contract.				cont_no				6	22		NOEDIT 
LEAVE					cont_desc			6	30		DISPLAY 
Customer No.			cust_no				4	75		INPUT 
LEAVE					cust_name			4	88		DISPLAY 
Customer Enq. Ref.		ord_no				5	75		REQUIRED 
Quote Status Code.		qt_stat				6	75		NOEDIT 
LEAVE					qt_stat_desc		6	88		DISPLAY 
Contact name.			con_name			8	22		REQUIRED 
Contact Position.		pos_code			9	22		REQUIRED 
Contact Position Desc.	pos_desc			10	22		DISPLAY 
Salesman Code.			sman				11	22		INPUT 
Salesman Desc.			sman_name			12	22		DISPLAY 
Delivery Name  :		del_name			8	75		INPUT 
Address        :		del_addr1			9	75		INPUT 
               :		del_addr2			10	75		INPUT 
               :		del_addr3			11	75		INPUT 
               :		del_addr4			12	75		INPUT 
First Call Date.		dt_fst_call			14	22		REQUIRED 
Last Call Date.			dt_lst_call			14	75		REQUIRED 
Quotation Date.			dt_quote			14	115		REQUIRED 
Next Follow-up Date.	dt_follow_up		15	22		REQUIRED 
Quote Expiry Date.		exp_date			15	75		REQUIRED 
Total number of Calls.	no_calls			16	22		REQUIRED 
Order to Place:			place_ord			16	75		REQUIRED 
Delivery Date.			del_date			16	115		REQUIRED 
Comments #1:			comm1				18	22		REQUIRED 
Comments #2:			comm2				19	22		REQUIRED 
Comments #3:			comm3				20	22		REQUIRED 
Salutation for Letter.	salute				18	75		INPUT 
Price Type.				pri_type			19	75		DISPLAY 
LEAVE					pri_desc			19	88		DISPLAY 
LEAVE					uom					0	0		INPUT 
LEAVE					qty_avail			0	0		DISPLAY 
LEAVE					qty					0	0		REQUIRED 
LEAVE					cost_price			0	0		REQUIRED 
LEAVE					sale_price			0	0		REQUIRED 
LEAVE					disc				0	0		INPUT 
LEAVE					ser_no				0	0		INPUT 
LEAVE					sub_tot				0	0		INPUT 
S.O Surcharge. (Y/N)	sos_ok				2	18		INPUT 
Payment Terms.			pay_term			3	18		INPUT 
Sell Terms.				sell_terms			4	18		INPUT 
Sell Description.		sell_desc			5	18		DISPLAY 
Carrier Code.			carr_code			7	18		INPUT 
LEAVE					carr_desc			7	44		DISPLAY 
Area code.				carr_area			8	18		INPUT 
LEAVE					ca_desc				8	44		DISPLAY 
Freight Amount.			freight				9	18		INPUT 
Total Kgs.				tot_kg				10	18		DISPLAY 
Reset Prices.			reset_price			11	18		INPUT 
Misc. Charge.			misc_charge1		13	18		REQUIRED 
Misc. Charge.			misc_charge2		14	18		REQUIRED 
Misc. Charge.			misc_charge3		15	18		REQUIRED 
((
line(1,0,1,132)
box(1,0,3,132,17)
line(1,1,7,131)
line(1,1,13,131)
line(1,1,17,131)
line(2,0,1,61)
line(2,61,2,37)
box(2,97,1,1,3)
box(2,60,0,68,4)
line(3,0,1,132)
box(4,0,1,90,14)
line(4,1,6,89)
line(4,1,12,89)
))
