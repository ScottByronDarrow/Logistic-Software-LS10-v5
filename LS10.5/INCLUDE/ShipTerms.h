/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: ShipTerms.h,v 5.0 2001/06/19 06:51:24 cha Exp $
-----------------------------------------------------------------------
| $Log: ShipTerms.h,v $
| Revision 5.0  2001/06/19 06:51:24  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/12 13:28:51  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 07:15:35  gerry
| Force revision no. to 2.0 - Rel-15072000
|
| Revision 1.1  2000/07/11 01:33:11  scott
| Added list of valid shipment terms as per INCOTERMS 2000
|
*/
#ifndef	_ShipTerms_h
#define	_ShipTerms_h
	struct
	{

		char	*_shipTermsCode;
		char	*_shipTermsShort;

	} shipTerms [] = {
		{"EXW", "Ex Works"}, 
		{"FCA", "Free Carrier"}, 
		{"FAS", "Free Alongside Ship"}, 
		{"FOB", "Free On Board"}, 
		{"CFR", "Cost and Freight"}, 
		{"CIF", "Cost, Insurance and Freight"}, 
		{"CPT", "Carriage Paid To"},
		{"CIP", "Carriage and Insurance Paid to"}, 
		{"DAF", "Delivery at Frontier"}, 
		{"DES", "Delivery Ex Ship"}, 
		{"DEQ", "Delivery Ex Quay (Duty Paid)"}, 
		{"DDU", "Delivery Duty Unpaid"}, 
		{"DDP", "Delivery Duty Paid"}, 
		{"",""},
	};

#endif	/* _ShipTerms_h */
