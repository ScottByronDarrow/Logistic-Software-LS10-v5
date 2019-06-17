/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: UomInfo.h,v 5.0 2001/06/19 06:51:26 cha Exp $
-----------------------------------------------------------------------
| $Log: UomInfo.h,v $
| Revision 5.0  2001/06/19 06:51:26  cha
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/02 23:43:04  scott
| Updated for spelling of Hundreds.
|
| Revision 4.0  2001/03/09 00:59:22  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/17 01:44:11  scott
| Updated UOM information
|
| Revision 3.1  2001/01/15 01:35:55  scott
| Added base information file related to UOM's
|
|
*/
#ifndef	_UomInfo_h
#define	_UomInfo_h

	struct
	{
		char	*UomGroup;
		char	*Uom;
		char	*UomDesc;
		float	UomCnv;
	}	uomInfo []	=	{
		{"Units",				"BA",	"Ball", 1},
		{"Units",				"BD",	"Bundle", 1},
		{"Units",				"BF",	"Board Foot", 1},
		{"Units",				"BG",	"Bag", 1},
		{"Units",				"BL",	"Barrel", 1},
		{"Units",				"BR",	"Bar", 1},
		{"Units",				"BT",	"Bottle", 1},
		{"Units",				"CA",	"Cartridge", 1},
		{"Units",				"CE",	"Cone", 1},
		{"Units",				"CL",	"Coil", 1},
		{"Units",				"CN",	"Can", 1},
		{"Units",				"CO",	"Container", 1},
		{"Units",				"CT",	"Carton", 1},
		{"Units",				"CY",	"Cylinder", 1},
		{"Units",				"DR",	"Drum", 1},
		{"Units",				"DZ",	"Dozens", 	12},
		{"Units",				"EA",	"Each", 1},
		{"Units",				"HD",	"Hundreds", 100},
		{"Units",				"JR",	"Jar", 1},
		{"Units",				"KT",	"Kit", 1},
		{"Units",				"PC",	"Piece", 1},
		{"Units",				"PG",	"Package", 1},
		{"Units",				"RM",	"Ream", 1},
		{"Units",				"RO",	"Roll", 1},
		{"Units",				"SA",	"Sack", 1},
		{"Units",				"SE",	"Set", 1},
		{"Units",				"SP",	"Strip", 1},
		{"Units",				"SX",	"Stick", 1},
		{"Units",				"TE",	"Tens", 	10},
		{"Units",				"TH",	"Thousands",1000},
		{"Units",				"TU",	"Tube", 1},
		{"Units",				"UN",	"Unit", 1},
		{"Units",				"VI",	"Vial", 1},
		{"Area",				"cm2",	"square centimeter", 0.01},
		{"Area",				"dam2",	"square area",		 10},
		{"Area",				"dm2",	"square decimeter",	 0.1},
		{"Area",				"hm2",	"square hectometer", 100},
		{"Area",				"km2",	"square kilometer",	 1000000},
		{"Area",				"m2",	"square meter",		 1},
		{"Area",				"mm2",	"square millimeter", 0.001},
		{"Area",				"nm2",	"square nanometer",	 0.000000001},
		{"Area",				"um2",	"square micrometer", 0.000001},
		{"Avoirdupois Mass",	"cwt",	"hundredweight",	 1600},
		{"Avoirdupois Mass",	"dr",	"dram",				 0.0625},
		{"Avoirdupois Mass",	"lb",	"pound",			 16},
		{"Avoirdupois Mass",	"oz",	"ounce",			 1},
		{"Length",				"cm",	"centimeter",		 0.01},
		{"Length",				"dam",	"dekameter",		 10},
		{"Length",				"dm",	"decimeter",		 0.1},
		{"Length",				"hm",	"hectometer",		 100},
		{"Length",				"km",	"kilometer",		 1000},
		{"Length",				"mm",	"millimeter",		 0.001},
		{"Length",				"m",	"meter",			 1},
		{"Length",				"nm",	"nanometer",		 0.000000001},
		{"Length",				"um",	"micrometer",		 0.000001},
		{"Liquid Volume",		"L",	"liter",			 1},
		{"Liquid Volume",		"ML",	"megaliter",		 1000000},
		{"Liquid Volume",		"cL",	"centiliter",		 0.01},
		{"Liquid Volume",		"dL",	"deciliter",		 0.1},
		{"Liquid Volume",		"daL",	"dekaliter",		 10},
		{"Liquid Volume",		"hL",	"hectoliter",		 100},
		{"Liquid Volume",		"kL",	"kiloliter",		 1000},
		{"Liquid Volume",		"mL",	"milliliter",		 0.001},
		{"Liquid Volume",		"nL",	"nanoliter",		 0.000000001},
		{"Liquid Volume",		"uL",	"microliter",		 0.000001},
		{"Mass",				"Mg",	"megagram",			 1000000},
		{"Mass",				"cg",	"centigram",		 0.01},
		{"Mass",				"dag",	"dekagram",			 10},
		{"Mass",				"dg",	"decigram",			 0.1},
		{"Mass",				"g",	"gram",				 1},
		{"Mass",				"hg",	"hectogram",		 100},
		{"Mass",				"kg",	"kilogram",			 1000},
		{"Mass",				"mg",	"milligram",		 0.001},
		{"Mass",				"ng",	"nanogram",			 0.000000001},
		{"Mass",				"t",	"metric ton",		 1000000},
		{"Mass",				"ug",	"microgram",		 0.000001},
		{"US-Area",				"acre",	"acre",				 43560},
		{"US-Area",				"ft2",	"square feet",		 1},
		{"US-Area",				"in2",	"square inch",		 0.006944444},
		{"US-Area",				"mi2",	"square mile",		 27878400},
		{"US-Area",				"sq rd","square rod",		 272.25},
		{"US-Area",				"yd2",	"square yard",		 9},
		{"US-Length",			"ft",	"feet",				 1},
		{"US-Length",			"fur",	"furlong",			 660},
		{"US-Length",			"in",	"inch",				 0.083333333},
		{"US-Length",			"mi",	"mile",				 5280},
		{"US-Length",			"rd",	"rod",				 16.5},
		{"US-Length",			"yd",	"yard",				 3},
		{"US-Volume",			"bu",	"bushel",			 2150.42},
		{"US-Volume",			"in3",	"cubic inch",		 1},
		{"US-Volume",			"pk",	"peck",				 537.605},
		{"US-Volume",			"qt",	"quart",			 67.2006},
		{"US-Volume",			"gal",	"gallon",			 231},
		{"US-Volume",			"pt",	"pint",				 28.875},
		{"Volume",				"cm3",	"cubic centimeter",	 0.01},
		{"Volume",				"dam3",	"cubic dekameter",	 10},
		{"Volume",				"dm3",	"cubic decimeter",	 0.1},
		{"Volume",				"hm3",	"cubic hectometer",	 100},
		{"Volume",				"km3",	"cubic kilometer",	 1000},
		{"Volume",				"m3",	"cubic meter",		 1},
		{"Volume",				"mm3",	"cubic millimeter",	 0.001},
		{"Volume",				"nm3",	"cubic nanometer",	 0.000000001},
		{"Volume",				"um3",	"cubic micrometer",	 0.000001},
		{"", "", "", 0}
		};
#endif	/*	_UomInfo_h */
