/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: CurrencyInfo.h,v 5.0 2001/06/19 06:51:15 cha Exp $
-----------------------------------------------------------------------
| $Log: CurrencyInfo.h,v $
| Revision 5.0  2001/06/19 06:51:15  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 00:59:19  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/10 04:54:01  scott
| New Currency structure
|
| Revision 3.1  2001/01/10 03:54:16  scott
| New file for standard structure related to currencies.
|
|
*/
#ifndef	_currencyInfo_h
#define	_currencyInfo_h

	struct
	{
		char	*currencyCode;
		char	*currencyDesc;
		char	*currencyUnit;
	}	currencyInfo []	=	{
		{"ADP",	"Andorran Peseta",						"Peseta"},
		{"AED",	"United Arab Emirates Dirham",			"Dirham"},
		{"AFA",	"Afghani",								"Afghani"},
		{"ALL",	"Lek",									"Lek"},
		{"AMD",	"Dram",									"Dram"},
		{"ANG",	"Netherlands Antilles Guilder",			"Guilder"},
		{"AOK",	"Kwanza",								"Kwanza"},
		{"AON",	"New Kwanza",							"Kwanza"},
		{"ARA",	"Austral",								"Austral"},
		{"ARP",	"Argentinian Peso",						"Peso"},
		{"ARS",	"Argentinian Nuevo Peso",				"Peso"},
		{"ATS",	"Schilling",							"Schilling"},
		{"AUD",	"Australian Dollar",					"Dollar"},
		{"AWG",	"Aruban Guilder",						"Guilder"},
		{"AZM",	"Azerbaijani Manat",					"Manat"},
		{"BAD",	"Bosnian Dinar",						"Dinar"},
		{"BBD",	"Barbados Dollar",						"Dollar"},
		{"BDT",	"Taka",									"Taka"},
		{"BEC",	"Convertible Belgian Franc",			"Franc"},
		{"BEF",	"Belgian Franc (also known as Frank)",	"Franc"},
		{"BEL",	"Financial Belgian Franc",				"Franc"},
		{"BFF",	"Burkina Faso Franc",					"Franc"},
		{"BGL",	"Lev",									"Lev"},
		{"BHD",	"Bahraini Dinar",						"Dinar"},
		{"BIF",	"Burundi Franc",						"Franc"},
		{"BMD",	"Bermudian Dollar",						"Dollar"},
		{"BND",	"Brunei Dollar",						"Dollar"},
		{"BOB",	"Boliviano",							"Peso"},
		{"BOP",	"Bolivian Peso",						"Peso"},
		{"BRC",	"Cruzeiro",								"Peso"},
		{"BRL",	"Real",									"Real"},
		{"BRR",	"Cruzeiro Real",						"Real"},
		{"BSD",	"Bahamian Dollar",						"Dollar"},
		{"BTN",	"Ngultrum",								"Ngultrum"},
		{"BWP",	"Pula",									"Pula"},
		{"BYR",	"Belarussian Rouble",					"Rouble"},
		{"BZD",	"Belize Dollar",						"Dollar"},
		{"CAD",	"Canadian Dollar",						"Dollar"},
		{"CDZ",	"New Zare",								"New Zare"},
		{"CHF",	"Swiss Franc",							"Franc"},
		{"CLF",	"Unidades de Fomento",					"Franc"},
		{"CLP",	"Chilean Peso",							"Peso"},
		{"CNY",	"Yuan Renminbi",						"Renminbi"},
		{"COP",	"Colombian Peso",						"Peso"},
		{"CRC",	"Costa Rican Coln",						"Coln"},
		{"CUP",	"Cuban Peso",							"Peso"},
		{"CVE",	"Escudo Caboverdiano",					"Peso"},
		{"CYP",	"Cypriot Pound",						"Pound"},
		{"CZK",	"Czech Koruna",							"Koruna"},
		{"DEM",	"Deutsche Mark",						"Mark"},
		{"DJF",	"Djibouti Franc",						"Franc"},
		{"DKK",	"Danish Krone",							"Krone"},
		{"DOP",	"Dominican Republic Peso",				"Peso"},
		{"DZD",	"Algerian Dinar",						"Dinar"},
		{"ECS",	"Sucre",								"Sucre"},
		{"EEK",	"Kroon",								"Kroon"},
		{"EGP",	"Egyptian Pound",						"Pound"},
		{"ERN",	"Eritrean Nakfa",						"Nakfa"},
		{"ESP",	"Spanish Peseta",						"Peseta"},
		{"ETB",	"Ethiopian Birr",						"Birr"},
		{"EUR",	"Euro (replacement name for the ECU)",	"Dollar"},
		{"FIM",	"Markka",								"Markka"},
		{"FJD",	"Fiji Dollar",							"Dollar"},
		{"FKP",	"Falkland Pound",						"Pound"},
		{"FRF",	"French Franc",							"Franc"},
		{"GBP",	"Pound Sterling (United Kingdom Pound)","Pound"},
		{"GEL",	"Lari",									"Lari"},
		{"GHC",	"Cedi",									"Cedi"},
		{"GIP",	"Gibraltar Pound",						"Pound"},
		{"GMD",	"Dalasi",								"Dalasi"},
		{"GNS",	"Syli (also known as Guinea Franc)",	"Franc"},
		{"GQE",	"Ekwele",								"Ekwele"},
		{"GRD",	"Greek Drachma",						"Drachma"},
		{"GTQ",	"Quetzal",								"Quetzal"},
		{"GWP",	"Guinea-Bissau Peso",					"Peso"},
		{"GYD",	"Guyana Dollar",						"Dollar"},
		{"HKD",	"Hong Kong Dollar",						"Dollar"},
		{"HNL",	"Lempira",								"Lempira"},
		{"HRD",	"Croatian Dinar",						"Dinar"},
		{"HRK",	"Croatian Kuna",						"Kuna"},
		{"HTG",	"Gourde",								"Gourde"},
		{"HUF",	"Forint",								"Forint"},
		{"IDR",	"Rupiah",								"Rupiah"},
		{"IEP",	"Punt",									"Punt"},
		{"ILS",	"Shekel",								"Shekel"},
		{"INR",	"Indian Rupee",							"Rupee"},
		{"IQD",	"Iraqi Dinar",							"Dinar"},
		{"IRR",	"Iranian Rial",							"Rial"},
		{"ISK",	"Icelandic Krna",						"Krna"},
		{"ITL",	"Italian Lira",							"Lira"},
		{"JMD",	"Jamaican Dollar",						"Dollar"},
		{"JOD",	"Jordanian Dinar",						"Dinar"},
		{"JPY",	"Yen",									"Yen"},
		{"KES",	"Kenyan Shilling",						"Shilling"},
		{"KGS",	"Kyrgyzstani Som",						"Som"},
		{"KHR",	"Riel",									"Riel"},
		{"KMF",	"Comorian Franc",						"Franc"},
		{"KPW",	"Democratic People's Republic of Korean Won",	"Won"},
		{"KRW",	"Republic of Korean Won",				"Won"},
		{"KWD",	"Kuwaiti Dinar",						"Dinar"},
		{"KYD",	"Cayman Islands Dollar",				"Dollar"},
		{"KZT",	"Tenge",								"Tenge"},
		{"LAK",	"Kip",									"Kip"},
		{"LBP",	"Lebanese Pound",						"Pound"},
		{"LKR",	"Sri Lankan Rupee",						"Rupee"},
		{"LRD",	"Liberian Dollar",						"Dollar"},
		{"LSL",	"Loti",									"Loti"},
		{"LSM",	"Maloti",								"Maloti"},
		{"LTL",	"Litas",								"Litas"},
		{"LUF",	"Luxembourg Franc",						"Franc"},
		{"LVL",	"Lat",									"Lat"},
		{"LYD",	"Libyan Dinar",							"Dinar"},
		{"MAD",	"Moroccan Dirham",						"Dirham"},
		{"MDL",	"Moldavian Leu"	,						"Leu"},
		{"MGF",	"Malagasy Franc",						"Franc"},
		{"MKD",	"Macedonian Dinar",						"Dinar"},
		{"MLF",	"Malian Franc",							"Franc"},
		{"MMK",	"Kyat",									"Kyat"},
		{"MNT",	"Tugrik",								"Tugrik"},
		{"MOP",	"Pataca",								"Pataca"},
		{"MRO",	"Ouguiya",								"Kyat"},
		{"MTL",	"Maltese Lira",							"Lira"},
		{"MUR",	"Mauritius Rupee",						"Rupee"},
		{"MVR",	"Rufiyaa",								"Rufiyaa"},
		{"MWK",	"Malawian Kwacha",						"Kwacha"},
		{"MXN",	"Mexican New Peso (replacement for Mexican Peso)",	"Peso"},
		{"MYR",	"Ringgit (also known as Malaysian Dollar)",	"Ringgit"},
		{"MZM",	"Metical",								"Metical"},
		{"NAD",	"Namibian Dollar",						"Dollar"},
		{"NGN",	"Naira",								"Naira"},
		{"NIC",	"Crdoba",								"Crdoba"},
		{"NLG",	"Dutch Guilder",						"Guilder"},
		{"NOK",	"Norwegian Krone",						"Krone"},
		{"NPR",	"Nepalese Rupee",						"Rupee"},
		{"NZD",	"New Zealand Dollar",					"Dollar"},
		{"OMR",	"Omani Rial",							"Rial"},
		{"PAB",	"Balboa",								"Balboa"},
		{"PEI",	"Inti",									"Inti"},
		{"PEN",	"New Sol",								"Sol"},
		{"PGK",	"Kina",									"Kina"},
		{"PHP",	"Philippines Peso",						"Peso"},
		{"PKR",	"Pakistani Rupee",						"Rupee"},
		{"PLN",	"New Zloty",							"Zloty"},
		{"PTE",	"Portuguese Escudo",					"Escudo"},
		{"PYG",	"Guarani",								"Guarani"},
		{"QAR",	"Qatari Riyal",							"Riyal"},
		{"ROL",	"Romanian Leu",							"Leu"},
		{"RUR",	"Russian Federation Rouble",			"Rouble"},
		{"RWF",	"Rwandan Franc",						"Franc"},
		{"SAR",	"Saudi Riyal",							"Riyal"},
		{"SBD",	"Solomon Islands Dollar",				"Dollar"},
		{"SCR",	"Seychelles Rupee",						"Rupee"},
		{"SDD",	"Sudanese Dinar",						"Dinar"},
		{"SDP",	"Sudanese Pound",						"Pound"},
		{"SEK",	"Swedish Krona",						"Krona"},
		{"SGD",	"Singapore Dollar",						"Dollar"},
		{"SHP",	"St Helena Pound",						"Pound"},
		{"SIT",	"Tolar",								"Tolar"},
		{"SKK",	"Slovak Koruna",						"Koruna"},
		{"SLL",	"Leone",								"Leone"},
		{"SOS",	"Somali Shilling",						"Shilling"},
		{"SRG",	"Surinam Guilder",						"Guilder"},
		{"STD",	"Dobra",								"Dobra"},
		{"SUR",	"Union of Soviet Socialist Republics Rouble",	"Rouble"},
		{"SVC",	"El Salvadorian Coln",					"Coln"},
		{"SYP",	"Syrian Pound",							"Pound"},
		{"SZL",	"Lilangeni",							"Lilangeni"},
		{"THB",	"Baht",									"Baht"},
		{"TJR",	"Tajik Rouble",							"Rouble"},
		{"TMM",	"Turkmenistani Manat",					"Manat"},
		{"TND",	"Tunisian Dinar",						"Dinar"},
		{"TOP",	"Pa'anga",								"Pa'anga"},
		{"TPE",	"Timorian Escudo",						"Escudo"},
		{"TRL",	"Turkish Lira",							"Lira"},
		{"TTD",	"Trinidad and Tobago Dollar",			"Dollar"},
		{"TWD",	"Taiwan Dollar",						"Dollar"},
		{"TZS",	"Tanzanian Shilling",					"Shilling"},
		{"UAH",	"Hryvna",								"Hryvna"},
		{"UAK",	"Karbovanet",							"Karbovanet"},
		{"UGS",	"Ugandan Shilling",						"Shilling"},
		{"USD",	"United States Dollar",					"Dollar"},
		{"USN",	"United States Dollar (Next day)",		"Dollar"},
		{"USS",	"United States Dollar (Same day)",		"Dollar"},
		{"UYU",	"Uruguayan New Peso",					"Peso"},
		{"UZS",	"Uzbekistani Som",						"Som"},
		{"VEB",	"Bolivar",								"Bolivar"},
		{"VND",	"Viet Nam Dong",						"Dong"},
		{"VUV",	"Vatu",									"Vatu"},
		{"WST",	"Tala",									"Tala"},
		{"XAF",	"Franc de la Communaut financire africaine","Franc"},
		{"XAU",	"Gold",									"Gold"},
		{"XBA",	"European Composite Unit",				"Dollar"},
		{"XBB",	"European Monetary Unit",				"Dollar"},
		{"XBC",	"European Unit of Account",				"Dollar"},
		{"XBD",	"European Unit of Account",				"Dollar"},
		{"XCD",	"East Caribbean Dollar",				"Dollar"},
		{"XDR",	"IMF Special Drawing Rights",			"Dollar"},
		{"XOF",	"West African Franc",					"Franc"},
		{"XPF",	"Franc des Comptoirs franais du Pacifique",	"Franc"},
		{"YDD",	"South Yemeni Dinar",					"Dinar"},
		{"YER",	"Yemeni Riyal",							"Riyal"},
		{"YUD",	"Yugoslavian New Dinar",				"Dinar"},
		{"ZAL",	"Rand (financial)",						"Rand"},
		{"ZAR",	"Rand",									"Rand"},
		{"ZMK",	"Zambian Kwacha",						"Kwacha"},
		{"ZWD",	"Zimbabwe Dollar",						"Dollar"},
		{"","",""},
		};
#endif	/*	_currencyInfo_h */
